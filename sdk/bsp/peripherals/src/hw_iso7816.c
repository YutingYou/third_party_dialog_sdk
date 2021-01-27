/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup ISO7816
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file hw_iso7816.c
 *
 * @brief Implementation of the ISO7816 Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */
#if dg_configUSE_HW_ISO7816

#include <stdint.h>
#include <string.h>
#include <sdk_defs.h>
#include "hw_iso7816.h"
#include "hw_pd.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define CEILING_FUNC(quotient, divisor)         (((quotient) + ((divisor) - 1UL)) / (divisor))

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Transaction type state
 */
typedef enum {
        TRANSACTION_NONE,               /**< No ongoing transaction */
        TRANSACTION_ATR,                /**< Receiving ATR */
        TRANSACTION_PPS,                /**< Transmitting/Receiving PPS */
        TRANSACTION_T0,                 /**< Transmitting/Receiving T=0 */
        TRANSACTION_T1,                 /**< Transmitting/Receiving T=1 */
} TRASACTION_TYPE;

/**
 * \brief Transaction direction
 */
typedef enum {
        TXN_NONE = 0,                   /**< No transaction ongoing */
        TXN_TX   = 1 << 0,              /**< Receiving */
        TXN_RX   = 1 << 1,              /**< Transmitting */
} TRANSACTION_ACTIVE;

/**
 * \brief APDU format as defined in ISO7816-3:2006, table 13
 */
typedef enum {
        CASE_1,                         /**< Case 1 */
        CASE_2S,                        /**< Case 2S */
        CASE_3S,                        /**< Case 3S */
        CASE_4S,                        /**< Case 4S */
        CASE_2E,                        /**< Case 2E */
        CASE_3E,                        /**< Case 3E */
        CASE_4E,                        /**< Case 4E */
        CASE_INVALID                    /**< Case INVALID */
} APDU_FORMAT;

/**
 * \brief ATR pending actions
 */
typedef enum {
        PEND_RX_CONV  = 1 << 0,         /**< Receive convention character (TS) */
        PEND_RX_TA    = 1 << 1,         /**< Receive Ta byte */
        PEND_RX_TB    = 1 << 2,         /**< Receive Tb byte */
        PEND_RX_TC    = 1 << 3,         /**< Receive Tc byte */
        PEND_RX_TD    = 1 << 4,         /**< Receive Td byte */
        PEND_RX_HIST  = 1 << 5,         /**< Receive Historical bytes */
        PEND_RX_TCK   = 1 << 6,         /**< Receive TCK byte */
        PEND_SET_CONV = 1 << 7,         /**< Set convention and invert ATR bytes if required */
} PEND_ATR_DATA;

/**
 * \brief T=0 pending actions
 */
typedef enum {
        PEND_HEADER      = 1 << 0,      /**< Transmitting header */
        PEND_ACK         = 1 << 1,      /**< Waiting for ACK */
        PEND_DATA        = 1 << 2,      /**< Receiving/Transmitting data */
        PEND_DATA_SINGLE = 1 << 3,      /**< Single byte TX/RX of data */
        PEND_SW1         = 1 << 4,      /**< Waiting for SW1 byte */
} PEND_T0_DATA;

/**
 * \brief T=1 pending actions
 */
typedef enum {
        PEND_NAD  = 1 << 0,             /**< NAD character */
        PEND_PCB  = 1 << 1,             /**< PCB character */
        PEND_LEN  = 1 << 2,             /**< LEN character */
        PEND_INF  = 1 << 3,             /**< INF field (data) */
        PEND_EDC1 = 1 << 4,             /**< EDC1 character */
        PEND_EDC2 = 1 << 5,             /**< EDC2 character */
} PEND_T1_DATA;

/**
 * \brief Interrupt source for character transmission
 */
typedef enum {
        TX_INT_SRC_UART  = 1 << 0,                              /**< IRQ source is the UART signals */
        TX_INT_SRC_TIMER = 1 << 1,                              /**< IRQ source is the ISO7816 timer (mode 1) */
        TX_INT_SRC_ALL   = TX_INT_SRC_UART | TX_INT_SRC_TIMER,  /**< Both sources (Used for disabling) */
} TX_INT_SRC;

/**
 * \brief Command TPDU message structure
 */
typedef struct {
        const uint8_t *data;                                            /**< Data buffer */
        uint16_t length;                                                /**< Length in bytes of data buffer */
        union {
                struct {
                        uint8_t cla;                                    /**< Class (CLA) byte */
                        uint8_t ins;                                    /**< Instruction (INS) byte */
                        uint8_t p1;                                     /**< Parameter (P1) byte */
                        uint8_t p2;                                     /**< Parameter (P2) byte */
                        uint8_t p3;                                     /**< Number of data bytes (P3) byte */
                };
                uint8_t header[HW_ISO7816_APDU_HEADER_BYTE_SIZE];       /**< Header of the TPDU */
        };
} t0_tpdu_cmd_t;


/**
 * \brief Response TPDU message structure
 */
typedef struct {
        uint8_t *data;                  /**< Data buffer */
        uint16_t length;                /**< Length of bytes received */
        uint16_t sw1sw2;                /**< Status bytes */
} t0_tpdu_rsp_t;

/**
 * \brief Transmit block structure
 */
typedef struct {
        const uint8_t *inf;             /**< Buffer containing information field (INF) */
        uint16_t edc;                   /**< Error detection code */
        uint8_t nad;                    /**< Node Address (NAD) byte */
        uint8_t pcb;                    /**< Protocol Control (PCB) byte */
        uint8_t len;                    /**< Length (LEN) byte */
} t1_block_tx_t;

/**
 * \brief Receive block structure
 */
typedef struct {
        uint8_t *inf;                   /**< Buffer containing information field (INF) */
        uint16_t edc;                   /**< Error detection code */
        uint8_t nad;                    /**< Node Address (NAD) byte */
        uint8_t pcb;                    /**< Protocol Control (PCB) byte */
        uint8_t len;                    /**< Length (LEN) byte */
} t1_block_rx_t;

/**
 * \brief T=0 internal protocol parameters
 */
typedef struct {
        uint32_t GT_delay_tick;         /**< Guard time between RX and TX */
        uint32_t WT_tick;               /**< Waiting time in clock ticks, WT = WI * 960 * Fi/f */
        uint16_t GT_etu;                /**< Guard time in ETUs */
} hw_iso7816_t0_internal_t;

/**
 * \brief T=1 internal protocol parameters
 */
typedef struct {
        hw_iso7816_t1_state state;      /**< Protocol state variables */
        uint32_t CGT_delay_tick;        /**< Character guard time between RX and TX */
        uint32_t BWT_tick;              /**< Block waiting time in ticks, BWT = 11etu + 2^BWI * 960 * Fd/f */
        uint32_t BWT_active;            /**< Currently used block waiting time */
        uint32_t CWT_tick;              /**< Character waiting time ticks, CWT =(11 + 2^CWI) * etu */
        uint16_t CGT_etu;               /**< Character guard time in ETUs */
        HW_ISO7816_T1_ERR_DET err_det;  /**< Redundancy detection algorithm, default: LRC */
} hw_iso7816_t1_internal_t;

/**
 * \brief ATR state variables required while the transaction is performed
 */
typedef struct {
        uint8_t *buff;                  /**< Receive buffer */
        PEND_ATR_DATA pending;          /**< Pending actions */
        uint8_t pos;                    /**< Position of buffer */
        uint8_t k;                      /**< Number of historical bytes */
} transaction_state_atr_t;

/**
 * \brief PPS state variables required while the transaction is performed
 */
typedef struct {
        hw_iso7816_pps_params_t *params;                /**< PPS parameters */
        uint8_t buff[HW_ISO7816_PPS_MAX_BYTE_SIZE];     /**< PPS RX/TX buffer */
        uint8_t len;                                    /**< PPS length */
        uint8_t pos;                                    /**< Position of buffer */
        bool pps1_present;                              /**< PPS1 present in PPS request */
        bool pps2_present;                              /**< PPS2 present in PPS request */
        uint8_t pps1;                                   /**< Value of PPS1 in PPS request */
        uint8_t pps2;                                   /**< Value of PPS2 in PPS request */
} transaction_state_pps_t;

/**
 * \brief APDU T=0 state variables required while the transaction is performed
 */
typedef struct {
        t0_tpdu_cmd_t cmd;              /**< Command TPDU */
        t0_tpdu_rsp_t rsp;              /**< Response TPDU */
        uint16_t Ne;                    /**< Expected response length */
        uint16_t Nc;                    /**< Command length */
        uint8_t pos;                    /**< Position of buffer */
        PEND_T0_DATA pending;           /**< Pending actions */
        APDU_FORMAT format;             /**< APDU format */
} transaction_state_t0_t;

/**
 * \brief APDU T=1 state variables required while the transaction is performed
 */
typedef struct {
        t1_block_tx_t tx_block;         /**< TX block */
        t1_block_rx_t rx_block;         /**< RX block */
        uint8_t pos;                    /**< Position of buffer */
        uint8_t retries;                /**< Retries in case of error */
        uint8_t s_rx_byte;              /**< Byte to be received as an answer to an S-block request */
        PEND_T1_DATA pending;           /**< Pending actions */
        bool first_tx_block;            /**< Flag indicating if it is the first block being transmitted */
} transaction_state_t1_t;

/**
 * \brief Transaction state depending on the type (\ref trasaction_type_t)
 */
typedef union {
        transaction_state_atr_t atr;
        transaction_state_pps_t pps;
        transaction_state_t0_t t0;
        transaction_state_t1_t t1;
} transaction_state_t;

/**
 * \brief ISO7816 low level driver internal data
 */
typedef struct {
        hw_iso7816_t0_internal_t t0_intrl;      /**< Internal T=0 transmission parameters */
        hw_iso7816_t1_internal_t t1_intrl;      /**< Internal T=1 transmission parameters */
        uint32_t timer_rem;                     /**< Remaining time in case of value not fitting uint16_t */
        uint32_t baud;                          /**< Baud rate used during card activation */
        void *cb;                               /**< Callback to be called when transaction completes */
        void *used_data;                        /**< User data for the callback */
        const uint8_t *apdu_tx_buff;            /**< Buffer containing the APDU to be transmitted */
        uint8_t *apdu_rx_buff;                  /**< Buffer containing the APDU received */
        size_t apdu_tx_len;                     /**< Length in bytes of the APDU to be transmitted */
        size_t apdu_rx_len;                     /**< Length in bytes of the APDU received */
        transaction_state_t state;              /**< State variables of the corresponding transaction */
        TRASACTION_TYPE type;                   /**< Transaction type ongoing */
        TRANSACTION_ACTIVE txn_mode;            /**< Transaction mode ongoing */
        uint8_t parity_err_retries;             /**< Parity error retries counter */
        uint8_t last_byte;                      /**< Last transmitted byte */
        HW_ISO7816_FIFO_TX_LVL tx_lvl;          /**< Transmit FIFO interrupt level */
        bool tx_fifo_on;                        /**< Flag indicating if FIFOs are currently being used */
        hw_iso7816_pad rst;                     /**< Reset pin */
} ISO7816_Data;

/**
 * \brief ISO7816 low level driver internal data
 *
 * \warning ISO7816 data are not retained. The user must ensure that they are updated after exiting
 * sleep.
 */
static ISO7816_Data iso7816_data;

/**
 * \brief Lookup table of the ISO7816 conversion integer (Fi) values
 */
static const uint16_t hw_iso7816_fi_lut[] = {
        [0b0000] = 372,
        [0b0001] = 372,
        [0b0010] = 558,
        [0b0011] = 744,
        [0b0100] = 1116,
        [0b0101] = 1488,
        [0b0110] = 1860,
        [0b1001] = 512,
        [0b1010] = 768,
        [0b1011] = 1024,
        [0b1100] = 1536,
        [0b1101] = 2048,
};

/**
 * \brief Lookup table of the ISO7816 maximum frequency values supported by card (f(max)) in KHz.
 */
static const uint16_t hw_iso7816_fmax_lut[] = {
        [0b0000] = 4000,
        [0b0001] = 5000,
        [0b0010] = 6000,
        [0b0011] = 8000,
        [0b0100] = 12000,
        [0b0101] = 16000,
        [0b0110] = 20000,
        [0b1001] = 5000,
        [0b1010] = 7500,
        [0b1011] = 10000,
        [0b1100] = 15000,
        [0b1101] = 20000,
};

/**
 * \brief Lookup table of the ISO7816 baud rate adjustment integer (Di) values
 * \note Negative values have a division meaning
 */
static const int8_t hw_iso7816_di_lut[] = {
        [0b0001] = 1,
        [0b0010] = 2,
        [0b0011] = 4,
        [0b0100] = 8,
        [0b0101] = 16,
        [0b0110] = 32,
        [0b0111] = 64,
        [0b1000] = 12,
        [0b1001] = 20,
        [0b1010] = -2,  //!< 1/2
        [0b1011] = -4,  //!< 1/4
        [0b1100] = -8,  //!< 1/8
        [0b1101] = -16, //!< 1/16
        [0b1110] = -32, //!< 1/32
        [0b1111] = -64, //!< 1/64
};

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */

static void hw_iso7816_apdu_t0_cb(HW_ISO7816_ERROR status, const t0_tpdu_cmd_t *cmd,
        const t0_tpdu_rsp_t *rsp);
static void hw_iso7816_apdu_t1_cb(HW_ISO7816_ERROR status, const t1_block_tx_t *tx_block,
        const t1_block_rx_t *rx_block);

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \name                Configuration functions
 *****************************************************************************************
 * \{
 */

uint8_t hw_iso7816_calc_clk_div(uint16_t fmax)
{
        return CEILING_FUNC(dg_configDIVN_FREQ / (1000UL * 2UL), fmax) - 1UL;
}

uint32_t hw_iso7816_calc_baud_rate(uint16_t f, int8_t d, uint16_t fmax)
{
        uint8_t div;
        uint32_t div_int, div_rem, div_fraq;

        div = hw_iso7816_calc_clk_div(fmax);

        /* 1etu = F/D * 1/f = F/D ticks */
        if (d < 0) {
                /* div_int = ((1UL/16UL * SYSCLK) * f * (-d)) / freq */
                div_int = (f * ((uint32_t)-d) * 2UL * (div + 1UL)) / 16UL;
                div_rem = (f * ((uint32_t)-d) * 2UL * (div + 1UL)) - div_int * 16UL;
                div_fraq = 16UL * div_rem / 16UL;
        }
        else {
                /* div_int = ((1UL/16UL * SYSCLK) * f) / (d * freq) */
                div_int = (f * 2UL * (div + 1UL)) / (((uint32_t)d) * 16UL);
                div_rem = (f * 2UL * (div + 1UL)) - div_int * (((uint32_t)d) * 16UL);
                div_fraq = 16UL * div_rem / (((uint32_t)d) * 16UL);
        }
        return (div_int & UINT16_MAX) << 8 | (div_fraq & UINT8_MAX);
}

bool hw_iso7816_set_baud(uint32_t baud)
{
        /* Verify UART not busy */
        if (REG_GETF(UART3, UART3_USR_REG, UART_BUSY))
                return false;

        /* Set bit 8 on LCR in order to access DivisorLatch register */
        REG_SETF(UART3, UART3_LCR_REG, UART_DLAB, 1);

        REG_SETF(UART3, UART3_DLF_REG, UART_DLF, baud);
        REG_SETF(UART3, UART3_RBR_THR_DLL_REG, RBR_THR_DLL, baud >> 8);
        REG_SET_MASKED(UART3, UART3_IER_DLH_REG, 0xFFUL, baud >> 16);

        REG_SETF(UART3, UART3_LCR_REG, UART_DLAB, 0);
        return true;
}

uint32_t hw_iso7816_get_baud(void)
{
        uint32_t baud = 0;

        /* Verify UART not busy */
        if (REG_GETF(UART3, UART3_USR_REG, UART_BUSY))
                return baud;

        /* Set bit 8 on LCR in order to access DivisorLatch register */
        REG_SETF(UART3, UART3_LCR_REG, UART_DLAB, 1);

        baud = REG_GETF(UART3, UART3_DLF_REG, UART_DLF);
        baud |= REG_GETF(UART3, UART3_RBR_THR_DLL_REG, RBR_THR_DLL) << 8;
        baud |= (0xFF & UART3->UART3_IER_DLH_REG) << 16;

        REG_SETF(UART3, UART3_LCR_REG, UART_DLAB, 0);

        return baud;
}

void hw_iso7816_get_default_atr_params(hw_iso7816_atr_params_t *params)
{
        params->convention                 = HW_ISO7816_CONV_UNKNOWN;
        params->pps_params.t               = 0;
        params->pps_params.f               = HW_ISO7816_FD_VALUE;
        params->pps_params.d               = HW_ISO7816_DD_VALUE;
        params->pps_params.fmax            = HW_ISO7816_FMAXD_VALUE;
        params->pps_params.spu.proprietary = false;
        params->pps_params.spu.value       = 0;
        params->n                          = 0;
        params->mode_negotiable            = true;
        params->able_to_change_neg_spec    = false;
        params->implicit_params            = false;
        params->class                      = HW_ISO7816_OPER_CLASS_A;
        params->clk_stop                   = HW_ISO7816_CLOCK_STOP_NOT_SUP;
        params->t0_prot.available          = false;
        params->t0_prot.WI                 = HW_ISO7816_T0_WI_DEFAULT;
        params->t1_prot.available          = false;
        params->t1_prot.CWI                = HW_ISO7816_T1_CWI_DEFAULT;
        params->t1_prot.BWI                = HW_ISO7816_T1_BWI_DEFAULT;
        params->t1_prot.err_det            = HW_ISO7816_T1_ERR_DET_DEFAULT;
        params->t1_prot.state.IFSC         = HW_ISO7816_T1_IFSC_DEFAULT;
        params->t1_prot.state.IFSD         = HW_ISO7816_T1_IFSD_DEFAULT;
        params->t1_prot.state.tx_sn        = 0;
        params->t1_prot.state.rx_sn        = 0;
}

void hw_iso7816_convert_atr_params_to_cfg(const hw_iso7816_atr_params_t *params,
        hw_iso7816_config_t *cfg)
{
        cfg->pps_params = params->pps_params;
        cfg->n = params->n;
        cfg->conv = params->convention;
        cfg->t0_prot = params->t0_prot;
        cfg->t1_prot = params->t1_prot;
}

void hw_iso7816_init(const hw_iso7816_config_t *cfg)
{
        /* Store necessary configurations */
        memset(&iso7816_data, 0, sizeof(iso7816_data));
        iso7816_data.rst = cfg->rst;
        iso7816_data.t1_intrl.state = cfg->t1_prot.state;
        iso7816_data.baud = hw_iso7816_calc_baud_rate(cfg->pps_params.f, cfg->pps_params.d,
                cfg->pps_params.fmax);

        /* Calculate protocol times if required. Calculate T=0 GT anyway since it is used by the PPS
         * exchange also */
        /* GT = (12 + N) * (F/D * 1/f) = (12 + N) * etu*/
        iso7816_data.t0_intrl.GT_etu = cfg->n == 0xFFU ? 12U : 12U + cfg->n;

        if (cfg->t0_prot.available) {
                hw_iso7816_calculate_protocol_times_t0(cfg->n, cfg->pps_params.f, cfg->pps_params.d,
                        &cfg->t0_prot);
        }
        if (cfg->t1_prot.available) {
                hw_iso7816_calculate_protocol_times_t1(cfg->n, cfg->pps_params.f, cfg->pps_params.d,
                        &cfg->t1_prot);
        }

        /* Enable module */
        ASSERT_ERROR(hw_pd_check_com_status());
        CRG_COM->SET_CLK_COM_REG = REG_MSK(CRG_COM, SET_CLK_COM_REG, UART3_ENABLE);
        CRG_COM->RESET_CLK_COM_REG = REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART3_CLK_SEL);

        /* Enable ISO7816 mode */
        REG_SET_BIT(UART3, UART3_CONFIG_REG, ISO7816_ENABLE);

        hw_iso7816_set_clk_div(hw_iso7816_calc_clk_div(cfg->pps_params.fmax));

        /* Configure FIFO */
        hw_iso7816_rx_fifo_tr_lvl_setf(cfg->rx_lvl);
        hw_iso7816_tx_fifo_tr_lvl_setf(cfg->tx_lvl);
        hw_iso7816_enable_fifo(true);
        iso7816_data.tx_lvl = cfg->tx_lvl;
        iso7816_data.tx_fifo_on = false;

        while (!hw_iso7816_set_convention(cfg->conv));

        /* Immediately apply new baud rate in case clock is already running */
        if (REG_GETF(UART3, UART3_CTRL_REG, ISO7816_CLK_EN)) {
                while (!hw_iso7816_set_baud(iso7816_data.baud));
        }
}

void hw_iso7816_deinit(void)
{
        GLOBAL_INT_DISABLE();

        NVIC_DisableIRQ(UART3_IRQn);
        NVIC_ClearPendingIRQ(UART3_IRQn);

        ASSERT_ERROR(hw_pd_check_com_status());

        /* Reset the controller */
        REG_SETF(UART3, UART3_SRR_REG, UART_UR, 1);

        /* Disable clocks */
        CRG_COM->RESET_CLK_COM_REG = CRG_COM_RESET_CLK_COM_REG_UART3_ENABLE_Msk;

        GLOBAL_INT_RESTORE();
}

bool hw_iso7816_set_convention(HW_ISO7816_CONV conv)
{
        uint32_t lcr_reg, config_reg;

        /* Check for invalid convention value */
        ASSERT_WARNING(conv == HW_ISO7816_CONV_UNKNOWN || conv == HW_ISO7816_CONV_DIR
                || conv == HW_ISO7816_CONV_INV)

        /* Verify UART not busy */
        if (REG_GETF(UART3, UART3_USR_REG, UART_BUSY)) {
                return false;
        }

        /* Set parity, stop bits and data length */
        lcr_reg = UART3->UART3_LCR_REG;
        REG_SET_FIELD(UART3, UART3_LCR_REG, UART_DLS, lcr_reg, 3);
        REG_SET_FIELD(UART3, UART3_LCR_REG, UART_STOP, lcr_reg, 1);
        REG_SET_FIELD(UART3, UART3_LCR_REG, UART_PEN, lcr_reg,
                (conv == HW_ISO7816_CONV_DIR || conv == HW_ISO7816_CONV_INV) ? 1UL : 0UL);
        REG_SET_FIELD(UART3, UART3_LCR_REG, UART_EPS, lcr_reg,
                conv == HW_ISO7816_CONV_DIR ? 1UL : 0UL);
        UART3->UART3_LCR_REG = lcr_reg;

        /* Use extra bit as a placeholder for the parity bit since the parity mode is not
         * established yet */
        REG_SETF(UART3, UART3_LCR_EXT, UART_DLS_E, conv == HW_ISO7816_CONV_UNKNOWN ? 1UL : 0UL);

        config_reg = UART3->UART3_CONFIG_REG;
        REG_SET_FIELD(UART3, UART3_CONFIG_REG, ISO7816_ERR_SIG_EN, config_reg,
                (conv == HW_ISO7816_CONV_DIR || conv == HW_ISO7816_CONV_INV) ? 1UL : 0UL);
        REG_SET_FIELD(UART3, UART3_CONFIG_REG, ISO7816_CONVENTION, config_reg,
                conv == HW_ISO7816_CONV_INV ? 1UL : 0UL);
        UART3->UART3_CONFIG_REG = config_reg;

        return true;
}

bool hw_iso7816_is_busy(void)
{
        return iso7816_data.type != TRANSACTION_NONE;
}
/** \} */

/**
 * \name                Activation functions
 *****************************************************************************************
 * \{
 */

void hw_iso7816_cold_reset(void)
{
        hw_gpio_set_inactive(iso7816_data.rst.port, iso7816_data.rst.pin);
        REG_SET_BIT(UART3, UART3_CTRL_REG, ISO7816_CLK_EN);

        hw_iso7816_timer_delay(400);

        while (!hw_iso7816_set_baud(iso7816_data.baud));
        hw_gpio_set_active(iso7816_data.rst.port, iso7816_data.rst.pin);

        hw_iso7816_timer_delay(400);
}

void hw_iso7816_warm_reset(void)
{
        hw_gpio_set_inactive(iso7816_data.rst.port, iso7816_data.rst.pin);

        hw_iso7816_timer_delay(400);

        hw_gpio_set_active(iso7816_data.rst.port, iso7816_data.rst.pin);
}

void hw_iso7816_deactivate(void)
{
        uint32_t ctrl_reg;

        iso7816_data.cb = NULL;
        iso7816_data.type = TRANSACTION_NONE;
        iso7816_data.txn_mode = TXN_NONE;

        /* Disable character transactions */
        while (!hw_iso7816_set_baud(0));

        hw_gpio_set_inactive(iso7816_data.rst.port, iso7816_data.rst.pin);

        ctrl_reg = UART3->UART3_CTRL_REG;
        REG_SET_FIELD(UART3, UART3_CTRL_REG, ISO7816_CLK_LEVEL, ctrl_reg, 0);
        REG_SET_FIELD(UART3, UART3_CTRL_REG, ISO7816_CLK_EN, ctrl_reg, 0);
        UART3->UART3_CTRL_REG = ctrl_reg;
}

void hw_iso7816_clock_stop(HW_ISO7816_CLK_STOP clk_stop)
{
        uint32_t ctrl_reg;

        if (!REG_GETF(UART3, UART3_CTRL_REG, ISO7816_CLK_EN)) {
                return;
        }

        switch (clk_stop) {
        case HW_ISO7816_CLOCK_STOP_LOW:
        case HW_ISO7816_CLOCK_STOP_HIGH:
        case HW_ISO7816_CLOCK_STOP_NO_PREF:
                hw_iso7816_timer_delay(1860);
                ctrl_reg = UART3->UART3_CTRL_REG;
                REG_SET_FIELD(UART3, UART3_CTRL_REG, ISO7816_CLK_LEVEL, ctrl_reg,
                        clk_stop == HW_ISO7816_CLOCK_STOP_HIGH ? 1UL : 0UL);
                REG_SET_FIELD(UART3, UART3_CTRL_REG, ISO7816_CLK_EN, ctrl_reg, 0UL);
                UART3->UART3_CTRL_REG = ctrl_reg;
                break;
        case HW_ISO7816_CLOCK_STOP_NOT_SUP:
        default:
                return;
        }

}

void hw_iso7816_clock_resume(void)
{
        REG_SET_BIT(UART3, UART3_CTRL_REG, ISO7816_CLK_EN);
        hw_iso7816_timer_delay(700);
}
/** \} */

/**
 * \name                Interrupt functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Enable/disable transmit interrupt using the provided interrupt source
 *
 * \param[in] enable            Enable/disable the interrupt source
 * \param[in] src               Select interrupt source to control
 */
__STATIC_INLINE  void hw_iso7816_set_tx_int(bool enable, TX_INT_SRC src)
{
        NVIC_DisableIRQ(UART3_IRQn);
        if (src & TX_INT_SRC_UART) {
                uint32_t ier_dlh_reg = UART3->UART3_IER_DLH_REG;
                REG_SET_FIELD(UART3, UART3_IER_DLH_REG, ETBEI_DLH1, ier_dlh_reg,
                        enable ? 1UL : 0UL);
                REG_SET_FIELD(UART3, UART3_IER_DLH_REG, PTIME_DLH7, ier_dlh_reg,
                        enable ? 1UL : 0UL);
                UART3->UART3_IER_DLH_REG = ier_dlh_reg;
        }
        else if (src & TX_INT_SRC_TIMER) {
                REG_SETF(UART3, UART3_CTRL_REG, ISO7816_TIM_EXPIRED_IRQMASK, enable ? 1UL : 0UL);
        }
        NVIC_EnableIRQ(UART3_IRQn);
}

/**
 * \brief Enable/disable receive interrupt providing the waiting time to be used
 *
 * \param[in] enable            Enable/disable the interrupt source
 * \param[in] wt                Waiting time in ticks in case of enabling irq
 */
__STATIC_INLINE void hw_iso7816_set_rx_int(bool enable, uint32_t wt)
{
        hw_iso7816_stop_timer();

        NVIC_DisableIRQ(UART3_IRQn);
        if (!enable) {
                hw_iso7816_rx_fifo_flush();
        }
        REG_SETF(UART3, UART3_IER_DLH_REG, ERBFI_DLH0, enable ? 1UL : 0UL);
        REG_SETF(UART3, UART3_CTRL_REG, ISO7816_TIM_EXPIRED_IRQMASK, enable ? 1UL : 0UL);
        NVIC_EnableIRQ(UART3_IRQn);

        if (enable) {
                hw_iso7816_start_timer(wt);
        }
}
/** \} */

/**
 * \name                Timer functions
 *****************************************************************************************
 * \{
 */

void hw_iso7816_start_timer(uint32 cycles)
{
        uint16_t timer_value = (uint16_t)MIN(cycles, UINT16_MAX);

        uint32_t iso7816_tim_reg = UART3->UART3_TIMER_REG;
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, iso7816_tim_reg, 0);
        UART3->UART3_TIMER_REG = iso7816_tim_reg;
        while (hw_iso7816_get_timer_expire_status());

        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_MAX, iso7816_tim_reg, timer_value);
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_MODE, iso7816_tim_reg, 0UL);
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, iso7816_tim_reg, 1UL);
        UART3->UART3_TIMER_REG = iso7816_tim_reg;

        iso7816_data.timer_rem = cycles - timer_value;
}

void hw_iso7816_stop_timer(void)
{
        REG_CLR_BIT(UART3, UART3_TIMER_REG, ISO7816_TIM_EN);
        while (hw_iso7816_get_timer_expire_status());
}

void hw_iso7816_timer_delay(uint16 cycles)
{
        if (!cycles)
                return;
        hw_iso7816_start_timer(cycles);
        while (!hw_iso7816_get_timer_expire_status());
        hw_iso7816_stop_timer();
}

void hw_iso7816_set_guard_time(uint16_t guard_time)
{
        /* Provided value is more than the maximum of the timer */
        ASSERT_WARNING(!((16 * guard_time - 1) & ~UINT16_MAX));

        uint32_t timer_reg = UART3->UART3_TIMER_REG;
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, timer_reg, 0UL);
        UART3->UART3_TIMER_REG = timer_reg;
        while (hw_iso7816_get_timer_expire_status());

        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, timer_reg, guard_time ? 1UL : 0UL);
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_MODE, timer_reg, 1UL);
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_MAX, timer_reg, 16UL * guard_time - 1UL);
        UART3->UART3_TIMER_REG = timer_reg;
}

void hw_iso7816_reset_timer_irq(void)
{
        uint32_t timer_reg = UART3->UART3_TIMER_REG;
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, timer_reg, 0UL);
        UART3->UART3_TIMER_REG = timer_reg;
        while (REG_GETF(UART3, UART3_IRQ_STATUS_REG, ISO7816_TIM_EXPIRED_IRQ));
        REG_SET_FIELD(UART3, UART3_TIMER_REG, ISO7816_TIM_EN, timer_reg, 1UL);
        UART3->UART3_TIMER_REG = timer_reg;
}
/** \} */

/**
 * \name                Protocol exchange functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Perform a reverse search in the Fi and fmax LUTs
 *
 * \param[in] fi                Fi value, as described in ISO7816-3:2006, table 7
 * \param[in] fmax              fmax value, as described in ISO7816-3:2006, table 7
 * \return The LUT position
 */
static uint8_t hw_iso7816_fi_fmax_reverse_lut(uint16_t fi, uint16_t fmax)
{
        size_t ret = 0;
        const size_t Fi_lut_elems = sizeof(hw_iso7816_fi_lut) / sizeof(hw_iso7816_fi_lut[0]);
        const size_t fmax_lut_elems = sizeof(hw_iso7816_fmax_lut) / sizeof(hw_iso7816_fmax_lut[0]);
        const size_t lut_size = MIN(Fi_lut_elems, fmax_lut_elems);

        for (ret = 0; ret < lut_size; ret++) {
                if (fi == hw_iso7816_fi_lut[ret] && fmax == hw_iso7816_fmax_lut[ret]) {
                        break;
                }
        }

        if (ret == lut_size) {
                /* No match, set default values */
                ret = 1;
        }

        return (uint8_t)ret;
}

/**
 * \brief Perform a reverse search in the Di LUT
 *
 * \param[in] di                Di value, as described in ISO7816-3:2006, table 8
 * \return The LUT position
 */
static uint8_t hw_iso7816_di_reverse_lut(int8_t di)
{
        size_t ret = 0;
        const size_t Di_lut_elems = sizeof(hw_iso7816_di_lut) / sizeof(hw_iso7816_di_lut[0]);

        for (ret = 0; ret < Di_lut_elems; ret++) {
                if (di == hw_iso7816_di_lut[ret]) {
                        break;
                }
        }

        if (ret == Di_lut_elems) {
                /* No match, set default values */
                ret = 1;
        }

        return (uint8_t)ret;
}

//===================== ATR functions ==========================================

void hw_iso7816_receive_atr_async(uint8_t *rx_buff, hw_iso7816_atr_callback cb, void *user_data)
{
        if (cb == NULL || rx_buff == NULL)
                return;

        iso7816_data.type = TRANSACTION_ATR;
        iso7816_data.txn_mode = TXN_RX;
        iso7816_data.state.atr.buff = rx_buff;
        iso7816_data.state.atr.pending = PEND_RX_CONV | PEND_RX_TD;

        iso7816_data.cb = cb;
        iso7816_data.used_data = user_data;

        hw_iso7816_set_error_signal_en(true);
        /* Disable FIFO to receive the convention byte (TS) */
        hw_iso7816_enable_fifo(false);

        hw_iso7816_set_rx_int(true, HW_ISO7816_ATR_TS_WT_TICKS);
}

/**
 * \brief Callback function to be called when an ATR transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 */
static void hw_iso7816_atr_cb(HW_ISO7816_RX_ERR status)
{
        hw_iso7816_atr_callback cb = iso7816_data.cb;
        iso7816_data.cb = NULL;

        if (cb) {
                cb(iso7816_data.used_data,
                        status == HW_ISO7816_RX_ERR_TIMEOUT ? HW_ISO7816_ERR_CARD_NO_RESP :
                        status == HW_ISO7816_RX_ERR_OK ? HW_ISO7816_ERR_OK : HW_ISO7816_ERR_CRC,
                        iso7816_data.state.atr.pos);
        }
}

HW_ISO7816_ERROR hw_iso7816_parse_atr(const uint8_t *atr, size_t len,
        hw_iso7816_atr_params_t *params)
{
        size_t pos = 0;
        size_t k = 0;
        size_t i = 1;
        uint8_t T0, Yi, T = 0;
        bool tck_present = false;

        if (params == NULL || len < HW_ISO7816_ATR_MIN_BYTE_SIZE
                || len > HW_ISO7816_ATR_MAX_BYTE_SIZE) {
                return HW_ISO7816_ERR_UNKNOWN;
        }

        /* Set default values to card connection parameters */
        hw_iso7816_get_default_atr_params(params);

        /* Convention */
        params->convention = atr[pos++];

        /* Get T0 */
        T0 = atr[pos++];

        /* K number of bytes is encoded in T0 */
        k = T0 & HW_ISO7816_TDi_VALUE;

        /* Get TAi, TBi, TCi and TDi. TA1, TB1, TC1 and TD1 flags are encoded in Y1 of T0 */
        for (Yi = T0 & HW_ISO7816_Yi_VALUE; Yi; i++) {
                uint8_t TAi, TBi, TCi, TDi = 0;

                if (Yi & HW_ISO7816_TAi_PRSNT) {
                        TAi = atr[pos++];
                        switch (i) {
                        case 1: /* TA1 */
                                params->pps_params.f = hw_iso7816_fi_lut[ISO7816_GET_FIELD(ATR, TA1,
                                        FI_F, TAi)];
                                params->pps_params.fmax = hw_iso7816_fmax_lut[ISO7816_GET_FIELD(ATR,
                                        TA1, FI_F, TAi)];
                                params->pps_params.d = hw_iso7816_di_lut[ISO7816_GET_FIELD(ATR, TA1,
                                        DI, TAi)];
                                break;
                        case 2: /* TA2 */
                                params->mode_negotiable = false;
                                params->able_to_change_neg_spec = !ISO7816_GET_FIELD(ATR, TA2,
                                        CHNG_NEG_ABLE, TAi);
                                params->implicit_params = !!ISO7816_GET_FIELD(ATR, TA2,
                                        IMPLCT_PARAMS, TAi);
                                params->pps_params.t = ISO7816_GET_FIELD(ATR, TA2, T_TYPE, TAi);
                                break;
                        }
                        switch (T) {
                        case 1: /* T = 1 */
                                params->t1_prot.state.IFSC = ISO7816_GET_FIELD(ATR, TA_T1, IFSC,
                                        TAi);
                                break;
                        case 15: /* T = 15 */
                                params->clk_stop = ISO7816_GET_FIELD(ATR, TA_T15, CLK_MODE, TAi);
                                params->class = ISO7816_GET_FIELD(ATR, TA_T15, OPER_CLASS, TAi);
                                break;
                        }
                }

                if (Yi & HW_ISO7816_TBi_PRSNT) {
                        TBi = atr[pos++];
                        switch (T) {
                        case 1: /* T = 1 */
                                params->t1_prot.CWI = ISO7816_GET_FIELD(ATR, TB_T1, CWI, TBi);
                                params->t1_prot.BWI = ISO7816_GET_FIELD(ATR, TB_T1, BWI, TBi);
                                break;
                        case 15: /* T = 15 */
                                params->pps_params.spu.proprietary = !!ISO7816_GET_FIELD(ATR,
                                        TB_T15, SPU_TYPE, TBi);
                                params->pps_params.spu.value = ISO7816_GET_FIELD(ATR, TB_T15,
                                        SPU_VAL, TBi);
                                break;
                        }
                }

                if (Yi & HW_ISO7816_TCi_PRSNT) {
                        TCi = atr[pos++];
                        switch (i) {
                        case 1: /* TC1 */
                                params->n = ISO7816_GET_FIELD(ATR, TC1, N, TCi);
                                break;
                        case 2: /* TC2 */
                                params->t0_prot.WI = ISO7816_GET_FIELD(ATR, TC2, WI, TCi);
                                break;
                        }
                        switch (T) {
                        case 1: /* T = 1 */
                                params->t1_prot.err_det = ISO7816_GET_FIELD(ATR, TC_T1, ERROR_DET,
                                        TCi);
                                break;
                        }
                }

                if (Yi & HW_ISO7816_TDi_PRSNT) {
                        TDi = atr[pos++];

                        /* Check the protocol number */
                        T = TDi & HW_ISO7816_TDi_VALUE;
                        switch (T) {
                        case 0:
                                params->t0_prot.available = true;
                                break;
                        case 1:
                                params->t1_prot.available = true;
                                if (!params->t0_prot.available)
                                        params->pps_params.t = 1;
                                break;
                        }

                        /* Check to see if there is a T!=0 in any TDi */
                        tck_present |= T ? true : false;
                }
                else if (i == 1) { /* TD1 absent */
                        params->t0_prot.available = true;
                        params->pps_params.t = 0;
                }

                Yi = TDi & HW_ISO7816_Yi_VALUE;
        };

        /* Get Historical bytes */
        for (i = 0; i < k; i++)
                params->historical[i] = atr[pos++];

        /* Get TCK byte */
        if (tck_present) {
                uint8_t check = 0;

                pos++;
                for (i = 1; i < pos; i++) {
                        check ^= atr[i];
                }
                if (check)
                        return HW_ISO7816_ERR_CRC;
        }

        if (pos != len)
                return HW_ISO7816_ERR_UNKNOWN;

        return HW_ISO7816_ERR_OK;
}

//===================== PPS functions ==========================================

void hw_iso7816_exchange_pps_async(hw_iso7816_pps_params_t *params, bool pps1, bool pps2,
        hw_iso7816_pps_callback cb, void *user_data)
{
        uint8_t pck = 0;

        if (!cb || !params)
                return;

        iso7816_data.type = TRANSACTION_PPS;
        iso7816_data.txn_mode = TXN_TX;
        iso7816_data.state.pps.params = params;
        iso7816_data.state.pps.len = 0;
        iso7816_data.state.pps.pos = 0;
        iso7816_data.cb = cb;
        iso7816_data.used_data = user_data;

        { /* PPSS Character */
                uint8_t ppss = HW_ISO7816_PPSS_CHAR;
                iso7816_data.state.pps.buff[iso7816_data.state.pps.len++] = ppss;
                pck ^= ppss;
        }

        { /* PPS0 character */
                uint8_t pps0 = 0;
                iso7816_data.state.pps.pps1_present = pps1;
                iso7816_data.state.pps.pps2_present = pps2;
                ISO7816_SET_FIELD(PPS, PPS0, T, pps0, params->t);
                ISO7816_SET_FIELD(PPS, PPS0, PPS1_PRSNT, pps0, pps1);
                ISO7816_SET_FIELD(PPS, PPS0, PPS2_PRSNT, pps0, pps2);
                ISO7816_SET_FIELD(PPS, PPS0, PPS3_PRSNT, pps0, 0);
                iso7816_data.state.pps.buff[iso7816_data.state.pps.len++] = pps0;
                pck ^= pps0;
        }

        /* PPS1 character */
        if (pps1) {
                uint8_t pps1_val = 0;
                ISO7816_SET_FIELD(PPS, PPS1, FI_F, pps1_val,
                        hw_iso7816_fi_fmax_reverse_lut(params->f, params->fmax));
                ISO7816_SET_FIELD(PPS, PPS1, DI, pps1_val, hw_iso7816_di_reverse_lut(params->d));
                iso7816_data.state.pps.buff[iso7816_data.state.pps.len++] = pps1_val;
                iso7816_data.state.pps.pps1 = pps1_val;
                pck ^= pps1_val;
        }

        /* PPS2 character */
        if (pps2) {
                uint8_t pps2_val = 0;
                ISO7816_SET_FIELD(PPS, PPS2, SPU_VAL, pps2_val, params->spu.value);
                ISO7816_SET_FIELD(PPS, PPS2, SPU_TYPE, pps2_val, params->spu.proprietary);
                iso7816_data.state.pps.buff[iso7816_data.state.pps.len++] = pps2_val;
                iso7816_data.state.pps.pps2 = pps2_val;
                pck ^= pps2_val;
        }

        { /* PCK character */
                iso7816_data.state.pps.buff[iso7816_data.state.pps.len++] = pck;
        }

        hw_iso7816_set_error_signal_en(true);

        hw_iso7816_set_tx_int(true, TX_INT_SRC_TIMER);

        hw_iso7816_start_timer(HW_ISO7816_PPS_GT_DELAY_TICKS);
}

/**
 * \brief Callback function to be called when a PPS transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 */
static void hw_iso7816_pps_cb(HW_ISO7816_RX_ERR status)
{
        HW_ISO7816_ERROR ret = HW_ISO7816_ERR_OK;
        hw_iso7816_pps_callback cb;
        uint8_t pos = 0, len = 0;
        uint8_t pps1_req, pps2_req, crc = 0;
        uint8_t *pps_resp;
        bool pps1_req_present, pps2_req_present, pps1_resp_present, pps2_resp_present;
        hw_iso7816_pps_params_t *params;

        switch (status) {
        case HW_ISO7816_RX_ERR_OK:
                len = iso7816_data.state.pps.pos;
                pps_resp = iso7816_data.state.pps.buff;
                pps1_req_present = iso7816_data.state.pps.pps1_present;
                pps2_req_present = iso7816_data.state.pps.pps2_present;
                pps1_req = iso7816_data.state.pps.pps1;
                pps2_req = iso7816_data.state.pps.pps2;
                params = iso7816_data.state.pps.params;

                for (pos = 0; pos < len; pos++) {
                        crc ^= pps_resp[pos];
                }
                pos = 0;

                if (crc) {
                        ret = HW_ISO7816_ERR_CRC;
                        break;
                }

                { /* PPSS Character */
                        uint8_t ppss_resp = pps_resp[pos];
                        if (pos++ == len) {
                                ret = HW_ISO7816_ERR_CARD_NO_RESP;
                                break;
                        }
                        if (ppss_resp != HW_ISO7816_PPSS_CHAR) {
                                ret = HW_ISO7816_ERR_PPS;
                                break;
                        }
                }
                { /* PPS0 Character */
                        uint8_t pps0_resp = pps_resp[pos];
                        if (pos++ == len) {
                                ret = HW_ISO7816_ERR_CARD_NO_RESP;
                                break;
                        }
                        if (ISO7816_GET_FIELD(PPS, PPS0, T, pps0_resp) != params->t) {
                                ret = HW_ISO7816_ERR_PPS;
                                break;
                        }
                        pps1_resp_present = ISO7816_GET_FIELD(PPS, PPS0, PPS1_PRSNT, pps0_resp);
                        pps2_resp_present = ISO7816_GET_FIELD(PPS, PPS0, PPS2_PRSNT, pps0_resp);
                        if ((pps1_resp_present & ~pps1_req_present)
                                || (pps2_resp_present & ~pps2_req_present)
                                || ISO7816_GET_FIELD(PPS, PPS0, PPS3_PRSNT, pps0_resp)) {
                                ret = HW_ISO7816_ERR_PPS;
                                break;
                        }
                }
                /* PPS1 character */
                if (pps1_resp_present) {
                        uint8_t pps1_resp = pps_resp[pos];
                        if (pos++ == len) {
                                ret = HW_ISO7816_ERR_CARD_NO_RESP;
                                break;
                        }
                        if (pps1_req != pps1_resp) {
                                ret = HW_ISO7816_ERR_PPS;
                                break;
                        }
                }
                else if (pps1_req_present) {
                        /* Set default values Fd and Dd*/
                        params->f = HW_ISO7816_FD_VALUE;
                        params->d = HW_ISO7816_DD_VALUE;
                        ret = HW_ISO7816_ERR_PPS_PART_ACPT;
                }
                /* PPS2 character */
                if (pps2_resp_present) {
                        uint8_t pps2_resp = pps_resp[pos];
                        if (pos++ == len) {
                                ret = HW_ISO7816_ERR_CARD_NO_RESP;
                                break;
                        }
                        if (pps2_req != pps2_resp) {
                                ret = HW_ISO7816_ERR_PPS;
                                break;
                        }
                }
                else if (pps2_req_present) {
                        /* Card does not use SPU */
                        params->spu.proprietary = false;
                        params->spu.value = 0;
                        ret = HW_ISO7816_ERR_PPS_PART_ACPT;
                }
                break;
        case HW_ISO7816_RX_ERR_PARITY:
                ret = HW_ISO7816_ERR_CRC;
                break;
        case HW_ISO7816_RX_ERR_TIMEOUT:
                ret = HW_ISO7816_ERR_CARD_NO_RESP;
                break;
        default:
                ret = HW_ISO7816_ERR_UNKNOWN;
        }

        cb = iso7816_data.cb;
        iso7816_data.cb = NULL;
        if (cb) {
                cb(iso7816_data.used_data, ret);
        }
}
/** \} */

/**
 * \brief Detect the APDU's format
 *
 * \param[in] tx_buff           Buffer containing the APDU
 * \param[in] tx_len            Length of APDU in bytes
 * \return The format of the APDU
 */
static APDU_FORMAT hw_iso7816_apdu_type(const uint8_t *tx_buff, size_t tx_len)
{
        if (tx_len == 4U) {
                return CASE_1;
        } else if (tx_len == 5U) {
                return CASE_2S;
        } else if (tx_buff[4] != 0U && tx_len == (5U + tx_buff[4])) {
                return CASE_3S;
        } else if (tx_buff[4] != 0U && tx_len == (6U + tx_buff[4])) {
                return CASE_4S;
        } else if (tx_buff[4] == 0U && tx_len == 7U) {
                return CASE_2E;
        } else if (tx_buff[4] == 0U
                && tx_len == (7U + (((unsigned)tx_buff[5] << 8) | tx_buff[6]))) {
                return CASE_3E;
        } else if (tx_buff[4] == 0U
                && tx_len == (9U + (((unsigned)tx_buff[5] << 8) | tx_buff[6]))) {
                return CASE_4E;
        } else {
                return CASE_INVALID;
        }
}

/**
 * \brief Perform a CRC/LRC calculation
 *
 * \param[in|out] edc           Error Detection Code value
 * \param[in]     data          Byte to insert in calculation
 */
static void hw_iso7816_update_edc(uint16_t *edc, uint8_t data)
{
        if (iso7816_data.t1_intrl.err_det == HW_ISO7816_T1_ERR_DET_CRC) {
                uint16_t tmp = (uint16_t)(data << 8);

                for (int i = 0; i < 8; i++) {
                        if (((*edc) ^ tmp) & 0x8000) {
                                (*edc) <<= 1;
                                (*edc) ^= (uint16_t)0x1021; /* X^12 + X^5 + 1 */
                        }
                        else {
                                (*edc) <<= 1;
                        }
                        tmp <<= 1;
                }
        }
        else {
                (*edc) ^= (uint16_t)data;
        }
}

/**
 * \name                T=0 Protocol functions
 *****************************************************************************************
 * \{
 */

void hw_iso7816_calculate_protocol_times_t0(uint8_t n, uint16_t f, int8_t d,
        const hw_iso7816_t0_prot_t *t0_prot)
{
        /* GT = (12 + N) * (F/D * 1/f) = (12 + N) * etu*/
        iso7816_data.t0_intrl.GT_etu = n == 0xFFU ? 12U : 12U + n;

        /* WT = WI * 960 * Fi/f = (WI * 960 * D)etu = WI * 960 * Fi ticks */
        iso7816_data.t0_intrl.WT_tick = t0_prot->WI * 960U * f;

        if (d > 0) {
                iso7816_data.t0_intrl.GT_delay_tick = (iso7816_data.t0_intrl.GT_etu - 10U) * f
                        / ((unsigned)d);
        }
        else { /* Negative values have a division meaning */
                iso7816_data.t0_intrl.GT_delay_tick = (iso7816_data.t0_intrl.GT_etu - 10U) * f
                        * ((unsigned)-d);
        }
}

/**
 * \brief Transmit and receive a TPDU over the T=0 transmission protocol
 *
 * \param[in] cmd               Command TPDU
 * \param[in] rsp               Response TPDU
 */
static void hw_iso7816_tpdu_transact_t0_async(t0_tpdu_cmd_t *cmd, t0_tpdu_rsp_t *rsp)
{
        iso7816_data.type = TRANSACTION_T0;
        iso7816_data.txn_mode = TXN_TX;
        iso7816_data.state.t0.cmd = *cmd;
        iso7816_data.state.t0.rsp = *rsp;

        iso7816_data.state.t0.pending = PEND_HEADER | PEND_ACK | PEND_DATA | PEND_SW1;
        iso7816_data.state.t0.pos = 0;

        hw_iso7816_set_error_signal_en(true);

        // Interrupt driven
        hw_iso7816_set_tx_int(true, TX_INT_SRC_TIMER);

        hw_iso7816_start_timer(iso7816_data.t0_intrl.GT_delay_tick);
}

/**
 * \brief Callback function to be called when a T=0 TPDU transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 */
static void hw_iso7816_tpdu_t0_cb(HW_ISO7816_RX_ERR status)
{
        HW_ISO7816_ERROR error;

        switch (status) {
        case HW_ISO7816_RX_ERR_OK:
                error = HW_ISO7816_ERR_OK;
                break;
        case HW_ISO7816_RX_ERR_TIMEOUT:
                error = HW_ISO7816_ERR_CARD_NO_RESP;
                break;
        case HW_ISO7816_RX_ERR_PARITY:
                error = HW_ISO7816_ERR_CRC;
                break;
        default:
                error = HW_ISO7816_ERR_UNKNOWN;
        }

        hw_iso7816_apdu_t0_cb(error, &iso7816_data.state.t0.cmd, &iso7816_data.state.t0.rsp);
}

void hw_iso7816_apdu_transact_t0_async(const uint8_t *tx_buff, size_t tx_len, uint8_t *rx_buff,
        hw_iso7816_transact_callback cb, void *user_data)
{
        APDU_FORMAT format;
        t0_tpdu_cmd_t cmd;
        t0_tpdu_rsp_t rsp;
        uint16_t Ne = 0, Nc = 0;

        if (!cb)
                return;
        iso7816_data.apdu_tx_buff = tx_buff;
        iso7816_data.apdu_tx_len = tx_len;
        iso7816_data.apdu_rx_buff = rx_buff;
        iso7816_data.apdu_rx_len = 0;
        iso7816_data.cb = cb;
        iso7816_data.used_data = user_data;

        cmd.cla = tx_buff[HW_ISO7816_APDU_HEADER_CLA_POS];
        cmd.p1 = tx_buff[HW_ISO7816_APDU_HEADER_P1_POS];
        cmd.p2 = tx_buff[HW_ISO7816_APDU_HEADER_P2_POS];

        format = hw_iso7816_apdu_type(tx_buff, tx_len);
        switch (format) {
        case CASE_1:
                cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                cmd.p3 = 0;
                cmd.data = NULL;
                cmd.length = cmd.p3;
                rsp.data = NULL;
                rsp.length = 0;
                break;
        case CASE_2S:
                cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                cmd.p3 = tx_buff[HW_ISO7816_APDU_HEADER_P3_POS];
                cmd.data = NULL;
                cmd.length = 0;

                Ne = cmd.p3;
                Ne += Ne ? 0U : (UINT8_MAX + 1U); /* 0 -> 256 */
                rsp.data = rx_buff;
                rsp.length = Ne;
                break;
        case CASE_3S:
                cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                cmd.p3 = tx_buff[HW_ISO7816_APDU_HEADER_P3_POS];
                cmd.data = &tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 1];
                cmd.length = cmd.p3;
                rsp.data = NULL;
                rsp.length = 0;
                break;
        case CASE_4S:
                cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                cmd.p3 = tx_buff[HW_ISO7816_APDU_HEADER_P3_POS];
                cmd.data = &tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 1];
                cmd.length = cmd.p3;

                Ne = tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + cmd.p3 + 1];
                Ne += Ne ? 0U : (UINT8_MAX + 1U); /* 0 -> 256 */
                rsp.data = NULL;
                rsp.length = 0;
                break;
        case CASE_2E:
                Ne = (tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 1] << 8)
                        | tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 2];
                Ne += Ne ? 0U : (UINT16_MAX + 1U); /* 0 -> 65536 */
                cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                cmd.data = NULL;
                cmd.length = 0;
                if (Ne <= (UINT8_MAX + 1U)) {
                        /* Compact Le length in a single byte, case 2E.1 */
                        cmd.p3 = Ne & 0xFFU;
                        rsp.data = rx_buff;
                        rsp.length = Ne;
                        format = CASE_2S;
                }
                else {
                        /* Ne > 256, case 2E.2 */
                        cmd.p3 = 0x00;
                        rsp.data = rx_buff;
                        rsp.length = UINT8_MAX + 1;
                }
                break;
        case CASE_3E:
                Nc = (tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 1] << 8)
                        | tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 2];
                if (Nc < (UINT8_MAX + 1)) {
                        /* Compact Lc length in a single byte, case 3E.1 */
                        cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                        cmd.p3 = Nc & 0xFFU;
                        cmd.data = &tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 3];
                }
                else {
                        /* Nc > 255, fragmentation (ENVELOPE) is required, case 3E.2*/
                        cmd.ins = HW_ISO7816_T0_ENVELOPE_INS;
                        cmd.p1 = 0;
                        cmd.p2 = 0;
                        cmd.p3 = UINT8_MAX;
                        cmd.data = tx_buff;
                }
                cmd.length = cmd.p3;
                rsp.data = NULL;
                rsp.length = 0;

                Nc -= cmd.p3;
                break;
        case CASE_4E:
                Nc = (tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 1] << 8)
                        | tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 2];
                Ne = (tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + Nc + 1] << 8)
                        | tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + Nc + 2];
                Ne += Ne ? 0U : (UINT16_MAX + 1U); /* 0 -> 65536 */
                if (Nc < (UINT8_MAX + 1)) {
                        /* Compact Lc length in a single byte, case 4E.1 */
                        cmd.ins = tx_buff[HW_ISO7816_APDU_HEADER_INS_POS];
                        cmd.p3 = Nc & 0xFFU;
                        cmd.data = &tx_buff[HW_ISO7816_APDU_HEADER_P3_POS + 3];
                }
                else {
                        /* Nc > 255, fragmentation (ENVELOPE) is required, case 4E.2*/
                        cmd.ins = HW_ISO7816_T0_ENVELOPE_INS;
                        cmd.p1 = 0;
                        cmd.p2 = 0;
                        cmd.p3 = UINT8_MAX;
                        cmd.data = tx_buff;
                }
                cmd.length = cmd.p3;
                rsp.data = NULL;
                rsp.length = 0;

                Nc -= cmd.p3;
                break;
        case CASE_INVALID:
                iso7816_data.cb = NULL;
                if (cb) {
                        cb(iso7816_data.used_data, HW_ISO7816_ERR_UNKNOWN,
                                HW_ISO7816_SW1SW2_INVALID, 0);
                }
        }

        iso7816_data.state.t0.Ne = Ne;
        iso7816_data.state.t0.Nc = Nc;
        iso7816_data.state.t0.format = format;

        hw_iso7816_tpdu_transact_t0_async(&cmd, &rsp);
}

/**
 * \brief Callback function to be called when a T=0 apdu transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 * \param[in] cmd               Command of the transaction
 * \param[in] rsp               Response of the transaction
 */
static void hw_iso7816_apdu_t0_cb(HW_ISO7816_ERROR status, const t0_tpdu_cmd_t *cmd,
        const t0_tpdu_rsp_t *rsp)
{
        uint16_t sw1sw2 = HW_ISO7816_SW1SW2_INVALID;
        bool proc_cmpl = false;
        t0_tpdu_cmd_t new_cmd = {
                .cla = cmd->cla,
                .ins = 0,
                .p1 = 0,
                .p2 = 0,
                .p3 = 0,
                .data = NULL,
                .length = 0,
        };
        t0_tpdu_rsp_t new_rsp = {
                .data = NULL,
                .length = 0,
                .sw1sw2 = HW_ISO7816_SW1SW2_INVALID,
        };

        if (status == HW_ISO7816_ERR_OK) {
                uint8_t *rx_buff = iso7816_data.apdu_rx_buff;
                const size_t rx_pos = iso7816_data.apdu_rx_len + rsp->length;
                const uint16_t Ne = iso7816_data.state.t0.Ne - rsp->length;
                uint16_t Nc = iso7816_data.state.t0.Nc;
                APDU_FORMAT format = iso7816_data.state.t0.format;

                sw1sw2 = rsp->sw1sw2;

                switch (sw1sw2) { /* Check exact values */
                case HW_ISO7816_SW1SW2_RESP_SUCCESS:
                        switch (format) {
                        case CASE_4S:
                                /* Completed successfully, call GET_RESPONSE (4S.2) */
                                new_cmd.ins = HW_ISO7816_T0_GET_RESP_INS;
                                new_cmd.p1 = 0;
                                new_cmd.p2 = 0;
                                new_cmd.p3 = Ne & 0xFFU;
                                new_cmd.data = NULL;
                                new_cmd.length = 0;

                                new_rsp.data = rx_buff;
                                new_rsp.length = Ne;
                                format = CASE_2S;
                                break;
                        case CASE_3E:
                                /* Envelope supported, case 3E.2b */
                                if (Nc) {
                                        new_cmd.ins = cmd->ins;
                                        new_cmd.p1 = cmd->p1;
                                        new_cmd.p2 = cmd->p2;
                                        new_cmd.p3 = MIN(UINT8_MAX, Nc);
                                        new_cmd.data += new_cmd.p3;
                                        new_cmd.length = new_cmd.p3;

                                        new_rsp.data = NULL;
                                        new_rsp.length = 0;

                                        Nc -= new_cmd.p3;
                                }
                                else {
                                        proc_cmpl = true;
                                }
                                break;
                        case CASE_4E:
                                if (Nc) {
                                        new_cmd.ins = cmd->ins;
                                        new_cmd.p1 = cmd->p1;
                                        new_cmd.p2 = cmd->p2;
                                        new_cmd.p3 = MIN(UINT8_MAX, Nc);
                                        new_cmd.data += new_cmd.p3;
                                        new_cmd.length = new_cmd.p3;

                                        new_rsp.data = NULL;
                                        new_rsp.length = 0;

                                        Nc -= new_cmd.p3;
                                }
                                else {
                                        new_cmd.ins = HW_ISO7816_T0_GET_RESP_INS;
                                        new_cmd.p1 = 0;
                                        new_cmd.p2 = 0;
                                        new_cmd.data = NULL;
                                        new_cmd.length = 0;
                                        new_rsp.data = rx_buff;

                                        if (Ne <= (UINT8_MAX + 1)) {
                                                new_cmd.p3 = Ne & 0xFFU;
                                                new_rsp.length = Ne;
                                                format = CASE_2S;
                                        }
                                        else {
                                                new_cmd.p3 = 0x00;
                                                new_rsp.length = UINT8_MAX + 1;
                                                format = CASE_2E;
                                        }
                                }
                                break;
                        default:
                                proc_cmpl = true;
                        }
                        break;
                case HW_ISO7816_SW1SW2_RESP_LEN_ERR:
                        /* Process aborted (2S.2 and 2E.2a) */
                        proc_cmpl = true;
                        break;
                case HW_ISO7816_SW1SW2_RESP_INV_INS:
                        /* Envelope not supported (3E.2a) */
                        proc_cmpl = true;
                        break;
                default:
                        switch (sw1sw2 & HW_ISO7816_SW1SW2_RESP_SW1_MASK) { /* Check SW1 byte */
                        case HW_ISO7816_SW1SW2_RESP_GET_RESP:
                                switch (format) {
                                case CASE_4S:
                                        /* Completed with info added, call GET_RESPONSE (4S.3) */
                                        new_rsp.data = rx_buff;
                                        new_rsp.length = sw1sw2 & HW_ISO7816_SW1SW2_RESP_SW2_MASK;
                                        new_rsp.length += new_rsp.length ? 0U : (UINT8_MAX + 1U);
                                        new_rsp.length = MIN(new_rsp.length, Ne);
                                        format = CASE_2S;

                                        new_cmd.ins = HW_ISO7816_T0_GET_RESP_INS;
                                        new_cmd.p1 = 0;
                                        new_cmd.p2 = 0;
                                        new_cmd.p3 = new_rsp.length & 0xFFU;
                                        new_cmd.data = NULL;
                                        new_cmd.length = 0;
                                        break;
                                case CASE_2E:
                                case CASE_4E:
                                        /* Completed, call GET_RESPONSE (2E.2d, 4E.1c) */
                                        if (Ne) {
                                                new_rsp.data = &rx_buff[rx_pos];
                                                new_rsp.length = sw1sw2
                                                        & HW_ISO7816_SW1SW2_RESP_SW2_MASK;
                                                new_rsp.length +=
                                                        new_rsp.length ? 0U : (UINT8_MAX + 1U);
                                                new_rsp.length = MIN(new_rsp.length, Ne);

                                                new_cmd.ins = HW_ISO7816_T0_GET_RESP_INS;
                                                new_cmd.p1 = 0;
                                                new_cmd.p2 = 0;
                                                new_cmd.p3 = new_rsp.length & 0xFFU;
                                                new_cmd.data = NULL;
                                                new_cmd.length = 0;
                                        }
                                        else {
                                                proc_cmpl = true;
                                        }
                                        break;
                                default:
                                        break;
                                }
                                break;
                        case HW_ISO7816_SW1SW2_RESP_WARN1:
                                /* Case 4S.4, 4E.1d */
                                proc_cmpl = true;
                                break;
                        case HW_ISO7816_SW1SW2_RESP_WARN2:
                                /* Case 4S.4, 4E.1d */
                                proc_cmpl = true;
                                break;
                        case HW_ISO7816_SW1SW2_RESP_LEN_RETRY:
                                /* Process aborted (2S.3 and 2E.2b) */
                                new_cmd.ins = cmd->ins;
                                new_cmd.p1 = cmd->p1;
                                new_cmd.p2 = cmd->p2;
                                new_cmd.p3 = sw1sw2 & HW_ISO7816_SW1SW2_RESP_SW2_MASK;
                                new_cmd.data = cmd->data;
                                new_cmd.length = cmd->length;

                                new_rsp.data = rsp->data;
                                new_rsp.length = new_cmd.p3;
                                new_rsp.length += new_rsp.length ? 0U : (UINT8_MAX + 1U); /* 0 -> 256 */
                                format = CASE_2S;
                                break;
                        default:
                                /* Cases 1, 2S.4, 3S, 4S.1, 4S.4, 3E.1, 4E.1a, 4E.1d */
                                proc_cmpl = true;
                        }
                }

                if (!proc_cmpl) {
                        /* Prepare next block if required */
                        iso7816_data.state.t0.Ne = Ne;
                        iso7816_data.state.t0.Nc = Nc;
                        iso7816_data.state.t0.format = format;
                        iso7816_data.apdu_rx_len = rx_pos;
                        hw_iso7816_tpdu_transact_t0_async(&new_cmd, &new_rsp);
                }
                else {
                        if (sw1sw2 != HW_ISO7816_SW1SW2_INVALID) {
                                /* Append SW1SW2 */
                                rx_buff[rx_pos] = (sw1sw2 & HW_ISO7816_SW1SW2_RESP_SW1_MASK) >> 8;
                                rx_buff[rx_pos + 1] = sw1sw2 & HW_ISO7816_SW1SW2_RESP_SW2_MASK;

                                iso7816_data.apdu_rx_len = rx_pos + 2U;
                        }
                }
        }

        /* Process completed or an error occurred */
        if (proc_cmpl || status != HW_ISO7816_ERR_OK) {
                hw_iso7816_transact_callback cb = iso7816_data.cb;
                iso7816_data.cb = NULL;
                if (cb) {
                        cb(iso7816_data.used_data, status, sw1sw2, iso7816_data.apdu_rx_len);
                }
        }
}

/** \} */

/**
 * \name                T=1 Protocol functions
 *****************************************************************************************
 * \{
 */
void hw_iso7816_calculate_protocol_times_t1(uint8_t n, uint16_t f, int8_t d,
        const hw_iso7816_t1_prot_t *t1_prot)
{
        /* GT = (12 + N) * (F/D * 1/f) = (12 + N) * etu*/
        iso7816_data.t1_intrl.CGT_etu = n == 0xFFU ? 11U : 12U + n;

        /* CWT = (11 + 2^CWI)etu = (11 + 2^CWI) * F/D ticks */
        /* BWT = 11etu + 2^BWI * 960 * Fd/f = 11 * F/D + (2^BWI * 960 * Fd) ticks */
        if (d > 0) {
                iso7816_data.t1_intrl.CGT_delay_tick = (iso7816_data.t1_intrl.CGT_etu - 10U) * f
                        / ((unsigned)d);
                iso7816_data.t1_intrl.CWT_tick = (11U + (1U << t1_prot->CWI)) * f / ((unsigned)d);
                iso7816_data.t1_intrl.BWT_tick = 11U * f / ((unsigned)d)
                        + (1U << t1_prot->BWI) * 960U * HW_ISO7816_FD_VALUE;
        }
        else { /* Negative values have a division meaning */
                uint32_t etu_ticks = f * ((unsigned)-d);
                iso7816_data.t1_intrl.CGT_delay_tick = (iso7816_data.t1_intrl.CGT_etu - 10U)
                        * etu_ticks;
                iso7816_data.t1_intrl.CWT_tick = (11U + (1U << t1_prot->CWI)) * etu_ticks;
                iso7816_data.t1_intrl.BWT_tick = 11U * etu_ticks
                        + (1U << t1_prot->BWI) * 960U * HW_ISO7816_FD_VALUE;
        }
        iso7816_data.t1_intrl.BWT_active = iso7816_data.t1_intrl.BWT_tick;
}

void hw_iso7816_get_state_t1(hw_iso7816_t1_state *state)
{
        *state = iso7816_data.t1_intrl.state;
}

/**
 * \brief Transmit and receive a block over the T=1 transmission protocol
 *
 * \param[in] tx_block          Transmit block
 * \param[in] rx_buff           Receive buffer
 */
static void hw_iso7816_block_transact_t1_async(t1_block_tx_t *tx_block, uint8_t *rx_buff)
{
        uint16_t edc;

        if (iso7816_data.t1_intrl.err_det == HW_ISO7816_T1_ERR_DET_CRC) {
                edc = UINT16_MAX;
                iso7816_data.state.t1.pending |= PEND_EDC2;
        }
        else {
                edc = 0U;
        }

        hw_iso7816_update_edc(&edc, tx_block->nad);
        hw_iso7816_update_edc(&edc, tx_block->pcb);
        hw_iso7816_update_edc(&edc, tx_block->len);
        for (int i = 0; i < tx_block->len; i++) {
                hw_iso7816_update_edc(&edc, tx_block->inf[i]);
        }

        iso7816_data.type = TRANSACTION_T1;
        iso7816_data.txn_mode = TXN_TX;
        iso7816_data.state.t1.pos = 0U;
        iso7816_data.state.t1.pending = PEND_NAD | PEND_PCB | PEND_LEN | PEND_INF | PEND_EDC1;

        iso7816_data.state.t1.tx_block = *tx_block;
        iso7816_data.state.t1.tx_block.edc = edc;

        iso7816_data.state.t1.rx_block.nad = 0U;
        iso7816_data.state.t1.rx_block.pcb = 0U;
        iso7816_data.state.t1.rx_block.len = 0U;
        iso7816_data.state.t1.rx_block.inf = rx_buff;
        iso7816_data.state.t1.rx_block.edc = 0U;

        /* Enable FIFO usage */
        iso7816_data.tx_fifo_on = true;

        hw_iso7816_set_error_signal_en(false);
        hw_iso7816_set_auto_gt(true);

        hw_iso7816_start_timer(iso7816_data.t1_intrl.CGT_delay_tick);

        /* Interrupt driven. We use the timer to initially count a guard time period from the
         * last received byte and then change to the UART IRQ */
        hw_iso7816_set_tx_int(true, TX_INT_SRC_TIMER);
}

/**
 * \brief Callback function to be called when a block transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 */
static void hw_iso7816_block_t1_cb(HW_ISO7816_RX_ERR status)
{
        HW_ISO7816_ERROR error;
        t1_block_tx_t *tx_block = &iso7816_data.state.t1.tx_block;
        t1_block_rx_t *rx_block = &iso7816_data.state.t1.rx_block;
        uint16_t edc;

        hw_iso7816_set_auto_gt(false);

        switch (status) {
        case HW_ISO7816_RX_ERR_OK:
                /* Verify RX block */
                if (iso7816_data.t1_intrl.err_det == HW_ISO7816_T1_ERR_DET_CRC) {
                        edc = UINT16_MAX;
                }
                else {
                        edc = 0;
                }

                hw_iso7816_update_edc(&edc, rx_block->nad);
                hw_iso7816_update_edc(&edc, rx_block->pcb);
                hw_iso7816_update_edc(&edc, rx_block->len);
                for (int i = 0; i < rx_block->len; i++) {
                        hw_iso7816_update_edc(&edc, rx_block->inf[i]);
                }
                if (rx_block->edc != edc) {
                        error = HW_ISO7816_ERR_CRC;
                }
                else {
                        error = HW_ISO7816_ERR_OK;
                }
                break;
        case HW_ISO7816_RX_ERR_PARITY:
                error = HW_ISO7816_ERR_CRC;
                break;
        case HW_ISO7816_RX_ERR_TIMEOUT:
                error = HW_ISO7816_ERR_CARD_NO_RESP;
                break;
        default:
                error = HW_ISO7816_ERR_UNKNOWN;
        }

        hw_iso7816_apdu_t1_cb(error, tx_block, rx_block);
}

/**
 * \brief Verify and prepare the T=1 block to be transmitted
 *
 * \param[in]     tx_type       Type of the block
 * \param[in]     inf           INF field of the block
 * \param[in]     len           Length of the INF field
 * \param[in|out] tx_block      TX block
 *
 * \return Type of the block
 */
static HW_ISO7816_PCB_BLOCK hw_iso7816_prepare_tx_block_t1(HW_ISO7816_PCB_BLOCK tx_type,
        const uint8_t *inf, size_t len, t1_block_tx_t *tx_block)
{
        hw_iso7816_t1_state *state = &iso7816_data.t1_intrl.state;
        switch (tx_type) {
        case HW_ISO7816_I_PCB:
                /* Prepare I block, set N(S) bit */
                tx_block->pcb = state->tx_sn ? HW_ISO7816_I_PCB_MSK_N : 0U;
                if (len > state->IFSC) {
                        /* Chaining is needed */
                        tx_block->pcb |= HW_ISO7816_I_PCB_MSK_M;
                        tx_block->len = state->IFSC;
                }
                else {
                        tx_block->len = len;
                }
                tx_block->inf = inf;
                break;
        case HW_ISO7816_R_PCB:
                tx_block->pcb |= state->rx_sn ? HW_ISO7816_R_PCB_MSK_N : 0U;
                tx_block->len = 0U;
                tx_block->inf = NULL;
                break;
        case HW_ISO7816_S_PCB:
                switch (tx_block->pcb) {
                case HW_ISO7816_S_PCB_RESYNCH_REQ:
                case HW_ISO7816_S_PCB_ABORT_REQ:
                case HW_ISO7816_S_PCB_ABORT_RESP:
                        tx_block->len = 0U;
                        tx_block->inf = NULL;
                        break;
                case HW_ISO7816_S_PCB_IFS_REQ:
                case HW_ISO7816_S_PCB_IFS_RESP:
                case HW_ISO7816_S_PCB_WTX_RESP:
                        tx_block->len = 1U;
                        tx_block->inf = inf;
                        break;
                default:
                        tx_type = HW_ISO7816_INV_PCB;
                }
                break;
        default:
                tx_type = HW_ISO7816_INV_PCB;
        }
        return tx_type;
}

void hw_iso7816_apdu_transact_t1_async(const uint8_t *tx_buff, size_t tx_len, uint8_t *rx_buff,
        hw_iso7816_transact_callback cb, void *user_data)
{
        t1_block_tx_t tx_block = { 0 };

        if (!cb)
                return;
        iso7816_data.apdu_tx_buff = tx_buff;
        iso7816_data.apdu_tx_len = tx_len;
        iso7816_data.apdu_rx_buff = rx_buff;
        iso7816_data.apdu_rx_len = 0U;
        iso7816_data.state.t1.first_tx_block = true;
        iso7816_data.cb = cb;
        iso7816_data.used_data = user_data;

        if (hw_iso7816_prepare_tx_block_t1(HW_ISO7816_I_PCB, tx_buff, tx_len, &tx_block)
                == HW_ISO7816_INV_PCB) {
                iso7816_data.cb = NULL;
                cb(iso7816_data.used_data, HW_ISO7816_ERR_UNKNOWN, HW_ISO7816_SW1SW2_INVALID, 0);
                return;
        }

        /* Send/Receive  block */
        hw_iso7816_block_transact_t1_async(&tx_block, rx_buff);
}

void hw_iso7816_supervisory_transact_t1_async(HW_ISO7816_S_PCB_VAL type, uint8_t *value,
        hw_iso7816_transact_callback cb, void *user_data)
{
        t1_block_tx_t tx_block = { .pcb = type };

        if (!cb)
                return;
        iso7816_data.apdu_tx_buff = value;
        iso7816_data.apdu_tx_len = sizeof(*value);
        iso7816_data.apdu_rx_buff = &iso7816_data.state.t1.s_rx_byte;
        iso7816_data.apdu_rx_len = 0;
        iso7816_data.state.t1.first_tx_block = true;
        iso7816_data.cb = cb;
        iso7816_data.used_data = user_data;

        if (hw_iso7816_prepare_tx_block_t1(HW_ISO7816_S_PCB, value, sizeof(*value), &tx_block)
                == HW_ISO7816_INV_PCB) {
                iso7816_data.cb = NULL;
                cb(iso7816_data.used_data, HW_ISO7816_ERR_UNKNOWN, HW_ISO7816_SW1SW2_INVALID, 0);
                return;
        }

        /* Send/Receive  block */
        hw_iso7816_block_transact_t1_async(&tx_block, &iso7816_data.state.t1.s_rx_byte);
}

/**
 * \brief Callback function to be called when a T=1 APDU transaction is complete
 *
 * \param[in] status            Status of the completed transaction
 * \param[in] tx_block          Block transmitted
 * \param[in] rx_block          Block received
 */
static void hw_iso7816_apdu_t1_cb(HW_ISO7816_ERROR status, const t1_block_tx_t *tx_block,
        const t1_block_rx_t *rx_block)
{
        t1_block_tx_t new_tx_block = *tx_block;
        HW_ISO7816_PCB_BLOCK rx_type, tx_type;
        uint16_t sw1sw2 = HW_ISO7816_SW1SW2_INVALID;
        uint8_t *rx_inf = rx_block->inf;
        hw_iso7816_t1_state *state = &iso7816_data.t1_intrl.state;
        bool sn;

        iso7816_data.t1_intrl.BWT_active = iso7816_data.t1_intrl.BWT_tick;

        /* Determine types of RX and TX block */
        tx_type =
                ~tx_block->pcb & 0x80 ?
                        HW_ISO7816_I_PCB :
                        (~tx_block->pcb & 0x40 ? HW_ISO7816_R_PCB : HW_ISO7816_S_PCB);
        if (status != HW_ISO7816_ERR_OK) {
                rx_type = HW_ISO7816_INV_PCB;
        }
        else {
                rx_type =
                        ~rx_block->pcb & 0x80 ?
                                HW_ISO7816_I_PCB :
                                (~rx_block->pcb & 0x40 ? HW_ISO7816_R_PCB : HW_ISO7816_S_PCB);
        }

        switch (rx_type) {
        case HW_ISO7816_I_PCB:
                sn = rx_block->pcb & HW_ISO7816_I_PCB_MSK_N ? true : false;
                if (sn == state->rx_sn) {
                        state->rx_sn ^= true;
                        iso7816_data.apdu_rx_len += rx_block->len;
                        rx_inf += rx_block->len;

                        if (tx_type == HW_ISO7816_I_PCB) {
                                iso7816_data.apdu_tx_buff += tx_block->len;
                                iso7816_data.apdu_tx_len -= tx_block->len;
                                iso7816_data.state.t1.first_tx_block = false;
                                state->tx_sn ^= true;
                        }

                        if (rx_block->pcb & HW_ISO7816_I_PCB_MSK_M) {
                                /* M bit set, ack packet */
                                new_tx_block.pcb = HW_ISO7816_R_PCB_ERR_NO_ERROR;
                                tx_type = HW_ISO7816_R_PCB;
                        }
                        else if (rx_block->len) {
                                /* Completed */
                                tx_type = HW_ISO7816_INV_PCB;
                                sw1sw2 = rx_block->inf[rx_block->len - 1];
                                sw1sw2 |= rx_block->inf[rx_block->len - 2] << 8;
                        }
                        else {
                                /* Transmit forced ACK */
                                tx_type = HW_ISO7816_I_PCB;
                        }
                }
                else {
                        /* Sequence error */
                        new_tx_block.pcb = HW_ISO7816_R_PCB_ERR_OTHER;
                        tx_type = HW_ISO7816_R_PCB;
                        if (iso7816_data.state.t1.retries++ == HW_ISO7816_T1_RETRIES_MAX) {
                                tx_type = HW_ISO7816_INV_PCB;
                                status = HW_ISO7816_ERR_UNKNOWN;
                                break;
                        }
                }
                break;
        case HW_ISO7816_R_PCB:
                sn = rx_block->pcb & HW_ISO7816_R_PCB_MSK_N ? true : false;
                switch (tx_type) {
                case HW_ISO7816_I_PCB:
                        if ((sn != state->tx_sn) && !(rx_block->pcb & HW_ISO7816_R_PCB_MSK_ERR)) {
                                        iso7816_data.apdu_tx_buff += tx_block->len;
                                        iso7816_data.apdu_tx_len -= tx_block->len;
                                        iso7816_data.state.t1.first_tx_block = false;
                                        state->tx_sn ^= true;
                                        tx_type =
                                                iso7816_data.apdu_tx_len ?
                                                                           HW_ISO7816_I_PCB :
                                                                           HW_ISO7816_INV_PCB;
                        } else {
                                /* Retry with same N(S) */
                                if (iso7816_data.state.t1.retries++ == HW_ISO7816_T1_RETRIES_MAX) {
                                        new_tx_block.pcb = HW_ISO7816_S_PCB_RESYNCH_REQ;
                                        tx_type = HW_ISO7816_S_PCB;
                                }
                        }

                        break;
                case HW_ISO7816_S_PCB:
                        if ((sn == state->tx_sn) && !(rx_block->pcb & HW_ISO7816_R_PCB_MSK_ERR)) {
                                if (tx_block->pcb == HW_ISO7816_S_PCB_ABORT_RESP) {
                                        tx_type = HW_ISO7816_INV_PCB;
                                        status = HW_ISO7816_ERR_CARD_ABORTED;
                                }
                        } else {
                                new_tx_block.pcb = HW_ISO7816_S_PCB_RESYNCH_REQ;
                                tx_type = HW_ISO7816_S_PCB;
                                if (iso7816_data.state.t1.retries++ == HW_ISO7816_T1_RETRIES_MAX) {
                                        tx_type = HW_ISO7816_INV_PCB;
                                }
                        }
                        break;
                }
                break;
        case HW_ISO7816_S_PCB:
                if (tx_type == HW_ISO7816_S_PCB && rx_block->pcb != (tx_block->pcb | 0x20)) {
                        if (iso7816_data.state.t1.retries++ < HW_ISO7816_T1_RETRIES_MAX) {
                                tx_type = HW_ISO7816_S_PCB;
                        }
                        else {
                                tx_type = HW_ISO7816_INV_PCB;
                                status = HW_ISO7816_ERR_UNKNOWN;
                        }
                        break;
                }
                switch (rx_block->pcb) {
                case HW_ISO7816_S_PCB_RESYNCH_RESP:
                        state->tx_sn = false;
                        state->rx_sn = false;
                        tx_type = iso7816_data.apdu_tx_len ? HW_ISO7816_I_PCB : HW_ISO7816_INV_PCB;
                        if (tx_type == HW_ISO7816_INV_PCB) {
                                status = HW_ISO7816_ERR_UNKNOWN;
                        }
                        break;
                case HW_ISO7816_S_PCB_IFS_REQ:
                        state->IFSC = rx_block->inf[0];
                        new_tx_block.pcb = HW_ISO7816_S_PCB_IFS_RESP;
                        tx_type = HW_ISO7816_S_PCB;
                        break;
                case HW_ISO7816_S_PCB_IFS_RESP:
                        state->IFSD = rx_block->inf[0];
                        tx_type = HW_ISO7816_INV_PCB;
                        break;
                case HW_ISO7816_S_PCB_ABORT_REQ:
                        new_tx_block.pcb = HW_ISO7816_S_PCB_ABORT_RESP;
                        tx_type = HW_ISO7816_S_PCB;
                        /* Set TX length to 0 to abort transmission */
                        iso7816_data.apdu_tx_len = 0U;
                        break;
                case HW_ISO7816_S_PCB_ABORT_RESP:
                        tx_type = HW_ISO7816_INV_PCB;
                        break;
                case HW_ISO7816_S_PCB_WTX_REQ:
                        iso7816_data.t1_intrl.BWT_active = iso7816_data.t1_intrl.BWT_tick
                                * rx_block->inf[0];
                        new_tx_block.pcb = HW_ISO7816_S_PCB_WTX_RESP;
                        tx_type = HW_ISO7816_S_PCB;
                        break;
                case HW_ISO7816_S_PCB_WTX_RESP:
                        tx_type = HW_ISO7816_INV_PCB;
                        break;
                }
                break;
        case HW_ISO7816_INV_PCB:
                if (iso7816_data.state.t1.retries < HW_ISO7816_T1_RETRIES_MAX) {
                        if (tx_type != HW_ISO7816_S_PCB) {
                                new_tx_block.pcb =
                                        status != HW_ISO7816_ERR_CRC ?
                                                                       HW_ISO7816_R_PCB_ERR_CRC :
                                                                       HW_ISO7816_R_PCB_ERR_OTHER;
                                tx_type = HW_ISO7816_R_PCB;
                        }
                }
                else if (!iso7816_data.state.t1.first_tx_block
                        && (iso7816_data.state.t1.retries
                                < HW_ISO7816_T1_RETRIES_MAX + HW_ISO7816_T1_RESYNC_MAX)) {
                        new_tx_block.pcb = HW_ISO7816_S_PCB_RESYNCH_REQ;
                        tx_type = HW_ISO7816_S_PCB;
                }
                else {
                        tx_type = HW_ISO7816_INV_PCB;
                        status = HW_ISO7816_ERR_UNKNOWN;
                }
                iso7816_data.state.t1.retries++;
                break;
        }

        /* Prepare next block if required */
        if (tx_type != HW_ISO7816_INV_PCB) {
                const uint8_t *inf =
                        tx_type == HW_ISO7816_I_PCB ? iso7816_data.apdu_tx_buff : rx_block->inf;
                new_tx_block.nad = rx_block->nad;

                hw_iso7816_prepare_tx_block_t1(tx_type, inf, iso7816_data.apdu_tx_len,
                        &new_tx_block);
                hw_iso7816_block_transact_t1_async(&new_tx_block, rx_inf);
        }
        else {
                hw_iso7816_transact_callback cb = iso7816_data.cb;
                iso7816_data.cb = NULL;
                if (cb) {
                        cb(iso7816_data.used_data, status, sw1sw2, iso7816_data.apdu_rx_len);
                }
        }
}

/** \} */

/**
 * \name                Interrupt handling functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Produce an inverted and reversed (MSB to LSB wise) byte
 *
 * \param[in] byte              Byte to invert
 *
 * \return The inverted byte's value
 */
__STATIC_INLINE uint8_t hw_iso7816_invert_byte(uint8_t byte)
{
        return ((uint8_t)(__RBIT(byte) >> 24)) ^ UINT8_MAX;
}

/**
 * \brief Called when a transmission is complete. Disables TX interrupts and enables RX interrupts
 */
__STATIC_INLINE void hw_iso7816_irq_stop_transmit(void)
{
        // Disable TX interrupts and enable RX interrupts
        hw_iso7816_set_tx_int(false, TX_INT_SRC_ALL);
        switch (iso7816_data.type) {
        case TRANSACTION_PPS:
                hw_iso7816_set_rx_int(true, HW_ISO7816_PPS_WT_TICKS);
                break;
        case TRANSACTION_T0:
                hw_iso7816_set_rx_int(true, iso7816_data.t0_intrl.WT_tick);
                break;
        case TRANSACTION_T1:
                hw_iso7816_set_rx_int(true, iso7816_data.t1_intrl.BWT_active);
                break;
        default:
                break;
        }
}

/**
 * \brief Called when a reception is complete. Disables RX interrupts and either enables TX
 * interrupts or calls corresponding completion callback.
 *
 * \param[in] status            Reason that the reception was stopped
 */
__STATIC_INLINE void hw_iso7816_irq_stop_receive(HW_ISO7816_RX_ERR status)
{
        hw_iso7816_set_rx_int(false, 0);
        switch (iso7816_data.type) {
        case TRANSACTION_ATR:
                iso7816_data.type = TRANSACTION_NONE;
                hw_iso7816_atr_cb(status);
                break;
        case TRANSACTION_PPS:
                iso7816_data.type = TRANSACTION_NONE;
                hw_iso7816_pps_cb(status);
                break;
        case TRANSACTION_T0:
                if (iso7816_data.txn_mode & TXN_TX) {
                        hw_iso7816_set_tx_int(true, TX_INT_SRC_TIMER);

                        hw_iso7816_start_timer(iso7816_data.t0_intrl.GT_delay_tick);
                }
                else {
                        iso7816_data.type = TRANSACTION_NONE;
                        hw_iso7816_tpdu_t0_cb(status);
                }
                break;
        case TRANSACTION_T1:
                iso7816_data.type = TRANSACTION_NONE;
                hw_iso7816_block_t1_cb(status);
                break;
        default:
                break;
        }
}

/**
 * \brief Provides character to be transmitted when in PPS exchange procedure
 *
 * \return Character to be transmitted
 */
__STATIC_INLINE uint8_t hw_iso7816_tx_pps(void)
{
        transaction_state_pps_t *state = &iso7816_data.state.pps;
        uint8_t tx_byte = 0;

        tx_byte = state->buff[state->pos++];

        if (state->pos >= state->len) {
                iso7816_data.txn_mode |= TXN_RX;
                state->pos = 0;
        }

        return tx_byte;
}

/**
 * \brief Provides character to be transmitted when in T=0 TPDU exchange procedure
 *
 * \return Character to be transmitted
 */
__STATIC_INLINE uint8_t hw_iso7816_tx_t0(void)
{
        transaction_state_t0_t *state = &iso7816_data.state.t0;
        uint8_t tx_byte = 0;

        if (state->pending & PEND_HEADER) {
                tx_byte = state->cmd.header[state->pos++];
                if (state->pos >= HW_ISO7816_APDU_HEADER_BYTE_SIZE) {
                        state->pending &= ~PEND_HEADER;
                        iso7816_data.txn_mode |= TXN_RX;
                }
        }
        else if (state->pending & PEND_DATA_SINGLE) {
                tx_byte = state->cmd.data[state->pos++];
                state->pending &= ~PEND_DATA_SINGLE;
                if (state->pos >= state->cmd.length) {
                        state->pending &= ~PEND_DATA;
                        iso7816_data.txn_mode |= TXN_RX;
                }
        }
        else if (state->pending & PEND_DATA) {
                tx_byte = state->cmd.data[state->pos++];
                if (state->pos >= state->cmd.length) {
                        state->pending &= ~PEND_DATA;
                        iso7816_data.txn_mode |= TXN_RX;
                }
        }

        return tx_byte;
}

/**
 * \brief Provides character to be transmitted when in T=1 block exchange procedure
 *
 * \return Character to be transmitted
 */
__STATIC_INLINE uint8_t hw_iso7816_tx_t1(void)
{
        transaction_state_t1_t *state = &iso7816_data.state.t1;
        uint8_t tx_byte = 0;
        bool use_edc2 = state->pending & PEND_EDC2;

        if (state->pending & PEND_NAD) {
                tx_byte = state->tx_block.nad;
                state->pending &= ~PEND_NAD;
        }
        else if (state->pending & PEND_PCB) {
                tx_byte = state->tx_block.pcb;
                state->pending &= ~PEND_PCB;
        }
        else if (state->pending & PEND_LEN) {
                tx_byte = state->tx_block.len;
                state->pending &= ~PEND_LEN;
                if (!state->tx_block.len) {
                        state->pending &= ~PEND_INF;
                }
        }
        else if (state->pending & PEND_INF) {
                tx_byte = state->tx_block.inf[state->pos++];
                if (state->pos >= state->tx_block.len) {
                        state->pending &= ~PEND_INF;
                }
        }
        else if (state->pending & PEND_EDC1) {
                tx_byte = (uint8_t)state->tx_block.edc;
                state->pending &= ~PEND_EDC1;
        }
        else if (state->pending & PEND_EDC2) {
                tx_byte = (uint8_t)(state->tx_block.edc >> 8);
                state->pending &= ~PEND_EDC2;
        }

        if (!state->pending) {
                iso7816_data.txn_mode = TXN_RX;
                state->pending = PEND_NAD | PEND_PCB | PEND_LEN | PEND_INF | PEND_EDC1
                        | (use_edc2 ? PEND_EDC2 : 0);
        }

        return tx_byte;
}

/**
 * \brief Function handling the TX interrupt
 *
 * \param[in] retry             Indicates if a re-transmission of the last character is required
 */
__STATIC_INLINE void hw_iso7816_tx_isr(bool retry)
{
        bool error_signal_en = hw_iso7816_get_error_signal_en();

        while (iso7816_data.txn_mode & TXN_TX) {
                uint8_t tx_byte;
                if (iso7816_data.tx_fifo_on) {
                        if (!hw_iso7816_transmit_fifo_not_full()) {
                                break;
                        }
                }
                else if (!hw_iso7816_thr_empty_getf()) {
                        break;
                }
                if (retry) {
                        if (iso7816_data.parity_err_retries < HW_ISO7816_PAR_ERR_RETRIES) {
                                tx_byte = iso7816_data.last_byte;
                                iso7816_data.parity_err_retries++;
                        }
                        else {
                                /* Terminate procedure, call callback */
                                iso7816_data.parity_err_retries = 0;
                                iso7816_data.txn_mode = TXN_NONE;
                                hw_iso7816_set_tx_int(false, TX_INT_SRC_ALL);
                                hw_iso7816_irq_stop_receive(HW_ISO7816_RX_ERR_PARITY);
                                break;
                        }
                }
                else {
                        switch (iso7816_data.type) {
                        case TRANSACTION_PPS:
                                tx_byte = hw_iso7816_tx_pps();
                                break;
                        case TRANSACTION_T0:
                                tx_byte = hw_iso7816_tx_t0();
                                break;
                        case TRANSACTION_T1:
                                tx_byte = hw_iso7816_tx_t1();
                                break;
                        default:
                                iso7816_data.txn_mode &= ~TXN_TX;
                                continue;
                        }
                        iso7816_data.last_byte = tx_byte;
                        iso7816_data.parity_err_retries = 0;
                }
                hw_iso7816_txdata_setf(tx_byte);

                if (error_signal_en)
                        break;
        }

        /* Transmission is complete only if error signal is not enabled, otherwise it will
         * be completed when STATUS_TIM_EXPIRED IRQ is triggered */
        if (!error_signal_en) {
                if (~iso7816_data.txn_mode & TXN_TX) {
                        if (!hw_iso7816_transmit_fifo_empty()) {
                              /* Set the TX FIFO level to hit when FIFO is completely empty in order
                               * to start counting the wait time */
                                hw_iso7816_tx_fifo_tr_lvl_setf(HW_ISO7816_FIFO_TX_LVL_EMPTY);
                        }
                        else {
                                /* Last byte has started being transmitted, restore TX FIFO level to
                                 * user preference */
                                hw_iso7816_tx_fifo_tr_lvl_setf(iso7816_data.tx_lvl);
                                hw_iso7816_irq_stop_transmit();
                        }
                }
        }
}

/**
 * \brief Handles received character when in ATR reception mode
 *
 * \param[in] rx_byte           Byte received
 *
 * \return Maximum waiting time of the next expected character
 */
__STATIC_INLINE uint32_t hw_iso7816_rx_atr(uint8_t rx_byte)
{
        transaction_state_atr_t *state = &iso7816_data.state.atr;
        uint32_t waiting_time = HW_ISO7816_ATR_WT_TICKS;

        if (state->pending & PEND_SET_CONV) {
                /* Try to set convention */
                uint8_t ts = state->buff[0];
                if (hw_iso7816_set_convention(ts)) {
                        state->pending &= ~PEND_SET_CONV;
                        /* Convert also any already received bytes */
                        if (ts == HW_ISO7816_CONV_INV) {
                                for (size_t i = 1; i < state->pos; ++i) {
                                        state->buff[i] = hw_iso7816_invert_byte(state->buff[i]);
                                }
                        }
                }
        }

        if (state->pending & PEND_RX_CONV) {
                uint8_t ts;
                state->pending &= ~PEND_RX_CONV;
                /* Get TS byte and try to set convention accordingly */
                ts = rx_byte == HW_ISO7816_CONV_DIR ? rx_byte : hw_iso7816_invert_byte(rx_byte);
                if (!hw_iso7816_set_convention(ts)) {
                        state->pending |= PEND_SET_CONV;
                }
                /* Enable FIFO for the rest of the bytes */
                hw_iso7816_enable_fifo(true);
                state->pos = 0;
                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_TA) {
                state->pending &= ~PEND_RX_TA;
                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_TB) {
                state->pending &= ~PEND_RX_TB;
                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_TC) {
                state->pending &= ~PEND_RX_TC;
                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_TD) {
                state->pending &= ~PEND_RX_TD;
                if (state->pos == 1) {
                        state->k = rx_byte & HW_ISO7816_TDi_VALUE;
                        if (state->k) {
                                state->pending |= PEND_RX_HIST;
                        }
                } else if (rx_byte & HW_ISO7816_TDi_VALUE) {
                        state->pending |= PEND_RX_TCK;
                }

                state->pending |= rx_byte & HW_ISO7816_TAi_PRSNT ? PEND_RX_TA : 0;
                state->pending |= rx_byte & HW_ISO7816_TBi_PRSNT ? PEND_RX_TB : 0;
                state->pending |= rx_byte & HW_ISO7816_TCi_PRSNT ? PEND_RX_TC : 0;
                state->pending |= rx_byte & HW_ISO7816_TDi_PRSNT ? PEND_RX_TD : 0;

                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_HIST) {
                if (--state->k == 0) {
                        state->pending &= ~PEND_RX_HIST;
                }
                state->buff[state->pos++] = rx_byte;
        }
        else if (state->pending & PEND_RX_TCK) {
                state->pending &= ~PEND_RX_TCK;
                state->buff[state->pos++] = rx_byte;
        }

        if (state->pos >= HW_ISO7816_ATR_MAX_BYTE_SIZE || !state->pending) {
                state->pending = 0;
                iso7816_data.txn_mode = TXN_NONE;
                waiting_time = 0;
        }
        return waiting_time;
}

/**
 * \brief Handles received character when in PPS transaction mode
 *
 * \param[in] rx_byte           Byte received
 *
 * \return Maximum waiting time of the next expected character
 */
__STATIC_INLINE uint32_t hw_iso7816_rx_pps(uint8_t rx_byte)
{
        transaction_state_pps_t *state = &iso7816_data.state.pps;
        uint32_t waiting_time = HW_ISO7816_PPS_WT_TICKS;

        if (state->pos < state->len) {
                state->buff[state->pos++] = rx_byte;
                if (state->pos == state->len) {
                        iso7816_data.txn_mode = TXN_NONE;
                        waiting_time = 0;
                }
        }
        return waiting_time;
}

/**
 * \brief Handles received character when in T=0 TPDU transaction mode
 *
 * \param[in] rx_byte           Byte received
 *
 * \return Maximum waiting time of the next expected character
 */
__STATIC_INLINE uint32_t hw_iso7816_rx_t0(uint8_t rx_byte)
{
        transaction_state_t0_t *state = &iso7816_data.state.t0;
        uint32_t waiting_time = iso7816_data.t0_intrl.WT_tick;

        if (~state->pending & PEND_SW1) {
                /* SW1 has been received, receive SW2 */
                state->rsp.sw1sw2 |= rx_byte;
                iso7816_data.txn_mode = TXN_NONE;
                waiting_time = 0;
        }
        else if ((~state->pending & PEND_ACK) && (state->pending & PEND_DATA)) {
                /* An ACK has been received, receive remaining bytes */
                if (state->Ne) {
                        state->rsp.data[state->pos++] = rx_byte;
                        state->Ne--;
                }
                state->rsp.length--;
                if (!state->rsp.length) {
                        state->pending &= ~PEND_DATA;
                        state->rsp.length = state->pos;
                }
                else if (state->pending & PEND_DATA_SINGLE) {
                        state->pending &= ~PEND_DATA_SINGLE;
                        state->pending |= PEND_ACK;
                }
        } /* A procedure byte has been received */
        else if (rx_byte == HW_ISO7816_PROC_BYTE_NULL) {
                /* A NULL byte has been received, no action is needed */
        }
        else if (((rx_byte & HW_ISO7816_PROC_BYTE_SW1_MASK) == HW_ISO7816_PROC_BYTE_SW1_1)
                || ((rx_byte & HW_ISO7816_PROC_BYTE_SW1_MASK) == HW_ISO7816_PROC_BYTE_SW1_2)) {
                /* A SW1 byte has been received, receive the SW2 byte */
                state->rsp.sw1sw2 = (uint16_t)(rx_byte << 8);

                if (state->pending & PEND_DATA) {
                        /* In this case no data has been received, but an exception occurred */
                        state->rsp.length = 0;
                }

                state->pending = 0;
        }
        else if ((rx_byte == state->cmd.ins)
                || (rx_byte == (state->cmd.ins ^ HW_ISO7816_PROC_BYTE_INS_XOR_VAL))) {
                /* An ACK byte has been received */
                state->pending &= ~PEND_ACK;
                state->pos = 0;
                if (state->cmd.length) {
                        /* Stop reception and start transmission of data */
                        iso7816_data.txn_mode = TXN_TX;
                        waiting_time = 0;
                }
                if (rx_byte == (state->cmd.ins ^ HW_ISO7816_PROC_BYTE_INS_XOR_VAL)) {
                        state->pending |= PEND_DATA_SINGLE;
                }
        }
        return waiting_time;
}

/**
 * \brief Handles received character when in T=1 block transaction mode
 *
 * \param[in] rx_byte           Byte received
 *
 * \return Maximum waiting time of the next expected character
 */
__STATIC_INLINE uint32_t hw_iso7816_rx_t1(uint8_t rx_byte)
{
        transaction_state_t1_t *state = &iso7816_data.state.t1;
        uint32_t waiting_time = iso7816_data.t1_intrl.CWT_tick;

        if (state->pending & PEND_NAD) {
                state->rx_block.nad = rx_byte;
                state->pending &= ~PEND_NAD;
        }
        else if (state->pending & PEND_PCB) {
                state->rx_block.pcb = rx_byte;
                state->pending &= ~PEND_PCB;
        }
        else if (state->pending & PEND_LEN) {
                state->rx_block.len = rx_byte;
                state->pending &= ~PEND_LEN;
                state->pos = 0;
                if (!state->rx_block.len) {
                        state->pending &= ~PEND_INF;
                }
        }
        else if (state->pending & PEND_INF) {
                state->rx_block.inf[state->pos++] = rx_byte;
                if (state->pos >= state->rx_block.len) {
                        state->pending &= ~PEND_INF;
                }
        }
        else if (state->pending & PEND_EDC1) {
                state->rx_block.edc = rx_byte;
                state->pending &= ~PEND_EDC1;
        }
        else if (state->pending & PEND_EDC2) {
                state->rx_block.edc |= rx_byte << 8;
                state->pending &= ~PEND_EDC2;
        }
        if (!state->pending) {
                iso7816_data.txn_mode = TXN_NONE;
                waiting_time = 0;
        }
        return waiting_time;
}

/**
 * \brief Function handling the RX interrupt
 */
__STATIC_INLINE void hw_iso7816_rx_isr()
{
        HW_ISO7816_RX_ERR status = HW_ISO7816_RX_ERR_OK;
        uint32_t waiting_time = 0;

        while (iso7816_data.txn_mode & TXN_RX) {
                uint8_t rx_byte;
                uint32_t lsr_reg = UART3->UART3_LSR_REG;

                if (!REG_GET_FIELD(UART3, UART3_LSR_REG, UART_DR, lsr_reg)) {
                        break;
                }

                rx_byte = hw_iso7816_rxdata_getf();

                status = REG_GET_FIELD(UART3, UART3_LSR_REG, UART_PE, lsr_reg) ?
                        HW_ISO7816_RX_ERR_PARITY : HW_ISO7816_RX_ERR_OK;
                ASSERT_WARNING(!REG_GET_FIELD(UART3, UART3_LSR_REG, UART_FE, lsr_reg));

                /* Handle bytes with receive error */
                if (status != HW_ISO7816_RX_ERR_OK) {
                        /* If error signal is enabled, transmitter should repeat character.
                         * Thus, we omit the erroneous one. Else, we stop reception */

                        iso7816_data.parity_err_retries++;
                        if ((iso7816_data.parity_err_retries > HW_ISO7816_PAR_ERR_RETRIES)
                                || !hw_iso7816_get_error_signal_en()) {
                                /* Error detected, stop reception */
                                iso7816_data.parity_err_retries = 0;
                                iso7816_data.txn_mode &= ~TXN_RX;
                        }
                        continue;
                }
                iso7816_data.parity_err_retries = 0;
                switch (iso7816_data.type) {
                case TRANSACTION_ATR:
                        waiting_time = hw_iso7816_rx_atr(rx_byte);
                        break;
                case TRANSACTION_PPS:
                        waiting_time = hw_iso7816_rx_pps(rx_byte);
                        break;
                case TRANSACTION_T0:
                        waiting_time = hw_iso7816_rx_t0(rx_byte);
                        break;
                case TRANSACTION_T1:
                        waiting_time = hw_iso7816_rx_t1(rx_byte);
                        break;
                default:
                        break;
                }
        }

        // Everything read?
        if (iso7816_data.txn_mode & TXN_RX) {
                if (waiting_time) {
                        hw_iso7816_start_timer(waiting_time);
                }
        }
        else {
                // Disable RX interrupts, fire callback if present
                hw_iso7816_irq_stop_receive(status);
        }
}

/**
 * \brief Function handling the ISO7816 timer expiration
 */
__STATIC_INLINE void hw_iso7816_timer_expired_isr(void)
{
        bool parity_err = false;
        bool call_tx_isr_handler = false;

        if (!REG_GETF(UART3, UART3_TIMER_REG, ISO7816_TIM_MODE)) {
                /* Timer mode */
                hw_iso7816_stop_timer(); /* To clear timer irq bit */
                if (iso7816_data.timer_rem) {
                        hw_iso7816_start_timer(iso7816_data.timer_rem);
                }
                else {
                        /* Call callback and disable timer irq */
                        if (iso7816_data.txn_mode & TXN_RX) {
                                if (!hw_iso7816_receive_fifo_not_empty()) {
                                        iso7816_data.txn_mode &= ~TXN_RX;
                                        hw_iso7816_irq_stop_receive(HW_ISO7816_RX_ERR_TIMEOUT);
                                }
                        }
                        else if (iso7816_data.txn_mode & TXN_TX) {
                                /* Start TX */
                                switch (iso7816_data.type) {
                                case TRANSACTION_PPS:
                                case TRANSACTION_T0:
                                        hw_iso7816_set_guard_time(iso7816_data.t0_intrl.GT_etu);
                                        call_tx_isr_handler = true;
                                        break;
                                case TRANSACTION_T1:
                                        hw_iso7816_set_tx_int(false, TX_INT_SRC_TIMER);
                                        hw_iso7816_set_guard_time(iso7816_data.t1_intrl.CGT_etu);
                                        hw_iso7816_set_tx_int(true, TX_INT_SRC_UART);
                                        break;
                                default:
                                        break;
                                }
                        }
                }
        }
        else {
                uint32_t irq_status_reg;

                hw_iso7816_reset_timer_irq();

                irq_status_reg = UART3->UART3_IRQ_STATUS_REG;
                /* Ensure TX_TIME_IRQ has been triggered */
                ASSERT_WARNING(REG_GET_FIELD(UART3, UART3_IRQ_STATUS_REG, ISO7816_ERR_TX_TIME_IRQ,
                        irq_status_reg));

                parity_err = REG_GET_FIELD(UART3, UART3_IRQ_STATUS_REG, ISO7816_ERR_TX_VALUE_IRQ,
                        irq_status_reg) ? true : false;

                /* Clear corresponding IRQs */
                UART3->UART3_IRQ_STATUS_REG =
                        REG_MSK(UART3, UART3_IRQ_STATUS_REG, ISO7816_ERR_TX_VALUE_IRQ)
                                || REG_MSK(UART3, UART3_IRQ_STATUS_REG, ISO7816_ERR_TX_TIME_IRQ);

                if ((iso7816_data.txn_mode & TXN_RX) && !parity_err) {
                        iso7816_data.txn_mode &= ~TXN_TX;
                        hw_iso7816_irq_stop_transmit();
                }
                else {
                        call_tx_isr_handler = true;
                }
        }
        if (call_tx_isr_handler) {
                hw_iso7816_tx_isr(parity_err);
        }
}

/**
 * \brief UART3 Interrupt Handler
 *
 */
void UART3_Handler(void)
{
        HW_ISO7816_UART_INT uart_int_id;
        HW_ISO7816_STATUS_INT status_int_id;

        SEGGER_SYSTEMVIEW_ISR_ENTER();

        for (bool int_pend = true; int_pend;) {
                uart_int_id = hw_iso7816_get_uart_interrupt_id();
                switch (uart_int_id) {
                case HW_ISO7816_UART_INT_TIMEOUT:
                case HW_ISO7816_UART_INT_RECEIVED_AVAILABLE:
                        hw_iso7816_rx_isr();
                        break;
                case HW_ISO7816_UART_INT_MODEM_STAT:
                        break;
                case HW_ISO7816_UART_INT_NO_INT_PEND:
                        break;
                case HW_ISO7816_UART_INT_THR_EMPTY:
                        hw_iso7816_tx_isr(false);
                        break;
                case HW_ISO7816_UART_INT_RECEIVE_LINE_STAT:
                        break;
                case HW_ISO7816_UART_INT_BUSY_DETECTED:
                        /*
                         * Stop here means that timing rules for access divisor latch were not
                         * followed. See description of register RBR_THR_DLL.
                         */
                        __BKPT(0);
                        break;
                }

                status_int_id = hw_iso7816_get_status_interrupt_id();
                if (status_int_id & HW_ISO7816_STATUS_TIM_EXPIRED) {
                        /* Timer has expired */
                        hw_iso7816_timer_expired_isr();
                }

                int_pend = (uart_int_id != HW_ISO7816_UART_INT_NO_INT_PEND || status_int_id);
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/** \} */

#endif /* dg_configUSE_HW_ISO7816 */

/**
 * \}
 * \}
 * \}
 */

