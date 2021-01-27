/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_ISO7816 ISO7816 Driver
 * \{
 * \brief ISO7816 Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_iso7816.h
 *
 * @brief Definition of API for the ISO7816 Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_ISO7816_H
#define HW_ISO7816_H


#if dg_configUSE_HW_ISO7816

#include <stdint.h>
#include <sdk_defs.h>
#include "hw_gpio.h"

#define HW_ISO7816_PAR_ERR_RETRIES           (4UL)    /* Parity error retry attempts */

#define HW_ISO7816_ATR_MAX_BYTE_SIZE         (33UL)
#define HW_ISO7816_ATR_MIN_BYTE_SIZE         (2UL)    /* TS and T0  */
#define HW_ISO7816_ATR_MAX_HIST_BYTE_SIZE    (15UL)   /* Maximum historical bytes */
#define HW_ISO7816_ATR_TS_WT_TICKS           (40000UL)/* Wait time of TS of ATR */
#define HW_ISO7816_ATR_WT_TICKS              (9600UL * HW_ISO7816_FD_VALUE / HW_ISO7816_DD_VALUE) /* Wait time of rest of ATR bytes */

#define HW_ISO7816_ATR_TA1_FI_F_Pos          (4UL)    /*!< ATR_TA1: Fi and f (Bit 4)                                   */
#define HW_ISO7816_ATR_TA1_FI_F_Msk          (0xF0UL) /*!< ATR_TA1: Fi and f (Bitfield-Mask: 0xf)                      */

#define HW_ISO7816_ATR_TA1_DI_Pos            (0UL)    /*!< ATR_TA1: Di (Bit 0)                                         */
#define HW_ISO7816_ATR_TA1_DI_Msk            (0xFUL)  /*!< ATR_TA1: Di (Bitfield-Mask: 0xf)                            */

#define HW_ISO7816_ATR_TC1_N_Pos             (0UL)    /*!< ATR_TC1: Extra Guard Time Integer (Bit 0)                   */
#define HW_ISO7816_ATR_TC1_N_Msk             (0xFFUL) /*!< ATR_TC1: Extra Guard Time Integer (Bitfield-Mask: 0xff)     */

#define HW_ISO7816_ATR_TA2_T_TYPE_Pos        (0UL)    /*!< ATR_TA2: T Type (Bit 0)                                     */
#define HW_ISO7816_ATR_TA2_T_TYPE_Msk        (0xFUL)  /*!< ATR_TA2: T Type (Bitfield-Mask: 0xf)                        */

#define HW_ISO7816_ATR_TA2_IMPLCT_PARAMS_Pos (4UL)    /*!< ATR_TA2: Implicit Parameters (Bit 4)                        */
#define HW_ISO7816_ATR_TA2_IMPLCT_PARAMS_Msk (0x10UL) /*!< ATR_TA2: Implicit Parameters (Bitfield-Mask: 0x1)           */

#define HW_ISO7816_ATR_TA2_CHNG_NEG_ABLE_Pos (7UL)    /*!< ATR_TA2: Able to change Negotiable / Specific Mode (Bit 7)  */
#define HW_ISO7816_ATR_TA2_CHNG_NEG_ABLE_Msk (0x80UL) /*!< ATR_TA2: Able to change Negotiable / Specific Mode (Bitfield-Mask: 0x1) */

#define HW_ISO7816_ATR_TC2_WI_Pos            (0UL)    /*!< ATR_TC2: Waiting Time Integer (Bit 0)                       */
#define HW_ISO7816_ATR_TC2_WI_Msk            (0xFFUL) /*!< ATR_TC2: Waiting Time Integer (Bitfield-Mask: 0xff)         */

#define HW_ISO7816_ATR_TA_T1_IFSC_Pos        (0UL)    /*!< ATR_TA_T1: Information Field Size of Card (Bit 0)           */
#define HW_ISO7816_ATR_TA_T1_IFSC_Msk        (0xFFUL) /*!< ATR_TA_T1: Information Field Size of Card (Bitfield-Mask: 0xff) */

#define HW_ISO7816_ATR_TB_T1_CWI_Pos         (0UL)    /*!< ATR_TA_T1: CWI (Bit 0)                                      */
#define HW_ISO7816_ATR_TB_T1_CWI_Msk         (0xFUL)  /*!< ATR_TA_T1: CWI (Bitfield-Mask: 0xf)                         */

#define HW_ISO7816_ATR_TB_T1_BWI_Pos         (4UL)    /*!< ATR_TB_T1: BWI (Bit 4)                                      */
#define HW_ISO7816_ATR_TB_T1_BWI_Msk         (0xF0UL) /*!< ATR_TB_T1: BWI (Bitfield-Mask: 0xf)                         */

#define HW_ISO7816_ATR_TC_T1_ERROR_DET_Pos   (0UL)    /*!< ATR_TC_T1: Error detection (Bit 0)                          */
#define HW_ISO7816_ATR_TC_T1_ERROR_DET_Msk   (0x1UL)  /*!< ATR_TC_T1: Error detection (Bitfield-Mask: 0x1)             */

#define HW_ISO7816_ATR_TA_T15_OPER_CLASS_Pos (0UL)    /*!< ATR_TA_T15: Class of Operation (Bit 0)                      */
#define HW_ISO7816_ATR_TA_T15_OPER_CLASS_Msk (0x3FUL) /*!< ATR_TA_T15: Class of Operation (Bitfield-Mask: 0x3f)        */

#define HW_ISO7816_ATR_TA_T15_CLK_MODE_Pos   (6UL)    /*!< ATR_TA_T15: Clock Mode (Bit 6)                              */
#define HW_ISO7816_ATR_TA_T15_CLK_MODE_Msk   (0xC0UL) /*!< ATR_TA_T15: Clock Mode (Bitfield-Mask: 0x3)                 */

#define HW_ISO7816_ATR_TB_T15_SPU_VAL_Pos    (0UL)    /*!< ATR_TB_T15: SPU Value (Bit 0)                               */
#define HW_ISO7816_ATR_TB_T15_SPU_VAL_Msk    (0x7FUL) /*!< ATR_TB_T15: SPU Value (Bitfield-Mask: 0x7f)                 */

#define HW_ISO7816_ATR_TB_T15_SPU_TYPE_Pos   (7UL)    /*!< ATR_TB_T15: SPU Standard / Proprietary (Bit 7)              */
#define HW_ISO7816_ATR_TB_T15_SPU_TYPE_Msk   (0x80UL) /*!< ATR_TB_T15: SPU Standard / Proprietary (Bitfield-Mask: 0x1) */

#define HW_ISO7816_PPS_MAX_BYTE_SIZE         (6UL)
#define HW_ISO7816_PPSS_CHAR                 (0xFFUL)
#define HW_ISO7816_PPS_GT_DELAY_TICKS        (HW_ISO7816_FD_VALUE / HW_ISO7816_DD_VALUE) /* GT after RX, 12 - 11 = 1 etu */
#define HW_ISO7816_PPS_WT_TICKS              (9600UL * HW_ISO7816_FD_VALUE / HW_ISO7816_DD_VALUE) /* Wait time of PPS bytes */

#define HW_ISO7816_PPS_PPS0_PPS3_PRSNT_Pos   (6UL)    /*!< PPS_PPS0: PPS3 Present (Bit 6)                              */
#define HW_ISO7816_PPS_PPS0_PPS3_PRSNT_Msk   (0x40UL) /*!< PPS_PPS0: PPS3 Present (Bitfield-Mask: 0x1)                 */

#define HW_ISO7816_PPS_PPS0_PPS2_PRSNT_Pos   (5UL)    /*!< PPS_PPS0: PPS2 Present (Bit 5)                              */
#define HW_ISO7816_PPS_PPS0_PPS2_PRSNT_Msk   (0x20UL) /*!< PPS_PPS0: PPS2 Present (Bitfield-Mask: 0x1)                 */

#define HW_ISO7816_PPS_PPS0_PPS1_PRSNT_Pos   (4UL)    /*!< PPS_PPS0: PPS1 Present (Bit 4)                              */
#define HW_ISO7816_PPS_PPS0_PPS1_PRSNT_Msk   (0x10UL) /*!< PPS_PPS0: PPS1 Present (Bitfield-Mask: 0x1)                 */

#define HW_ISO7816_PPS_PPS0_T_Pos            (0UL)    /*!< PPS_PPS0: T Type (Bit 0)                                    */
#define HW_ISO7816_PPS_PPS0_T_Msk            (0xFUL)  /*!< PPS_PPS0: T Type (Bitfield-Mask: 0xf)                       */

#define HW_ISO7816_PPS_PPS1_FI_F_Pos         (4UL)    /*!< PPS_PPS1: Fi and f (Bit 4)                                  */
#define HW_ISO7816_PPS_PPS1_FI_F_Msk         (0xF0UL) /*!< PPS_PPS1: Fi and f (Bitfield-Mask: 0xf)                     */

#define HW_ISO7816_PPS_PPS1_DI_Pos           (0UL)    /*!< PPS_PPS1: Di (Bit 0)                                        */
#define HW_ISO7816_PPS_PPS1_DI_Msk           (0xFUL)  /*!< PPS_PPS1: Di (Bitfield-Mask: 0xf)                           */

#define HW_ISO7816_PPS_PPS2_SPU_VAL_Pos      (0UL)    /*!< PPS_PPS2: SPU Value (Bit 0)                                 */
#define HW_ISO7816_PPS_PPS2_SPU_VAL_Msk      (0x7FUL) /*!< PPS_PPS2: SPU Value (Bitfield-Mask: 0x7f)                   */

#define HW_ISO7816_PPS_PPS2_SPU_TYPE_Pos     (7UL)    /*!< PPS_PPS2: SPU Standard / Proprietary (Bit 7)                */
#define HW_ISO7816_PPS_PPS2_SPU_TYPE_Msk     (0x80UL) /*!< PPS_PPS2: SPU Standard / Proprietary (Bitfield-Mask: 0x1)   */

#define ISO7816_GET_FIELD(base, byte, field, var) \
        ((var & (HW_ISO7816_ ## base ## _ ## byte ## _ ## field ## _Msk)) >> \
                (HW_ISO7816_ ## base ## _ ## byte ## _ ## field ## _Pos))

#define ISO7816_SET_FIELD(base, byte, field, var, val) \
        var = ((var & ~((HW_ISO7816_ ## base ## _ ## byte ## _ ## field ## _Msk))) | \
               ((val << (HW_ISO7816_ ## base ## _ ## byte ## _ ## field ## _Pos)) & \
                        (HW_ISO7816_ ## base ## _ ## byte ## _ ## field ## _Msk)))

#define HW_ISO7816_APDU_HEADER_BYTE_SIZE     (5UL)
#define HW_ISO7816_APDU_HEADER_CLA_POS       (0UL) /* Invalid values FF */
#define HW_ISO7816_APDU_HEADER_INS_POS       (1UL) /* Invalid values 6X and 9X */
#define HW_ISO7816_APDU_HEADER_P1_POS        (2UL)
#define HW_ISO7816_APDU_HEADER_P2_POS        (3UL)
#define HW_ISO7816_APDU_HEADER_P3_POS        (4UL)

#define HW_ISO7816_PROC_BYTE_NULL            (0x60UL)
#define HW_ISO7816_PROC_BYTE_SW1_1           (0x60UL)
#define HW_ISO7816_PROC_BYTE_SW1_2           (0x90UL)
#define HW_ISO7816_PROC_BYTE_SW1_MASK        (0xF0UL)
#define HW_ISO7816_PROC_BYTE_INS_XOR_VAL     (0xFFUL)

#define HW_ISO7816_SW1SW2_RESP_SW1_MASK      (0xFF00UL)
#define HW_ISO7816_SW1SW2_RESP_SW2_MASK      (0x00FFUL)

#define HW_ISO7816_T0_GET_RESP_INS           (0xC0UL)
#define HW_ISO7816_T0_ENVELOPE_INS           (0xC2UL)
#define HW_ISO7816_T0_WI_DEFAULT             (10UL)

#define HW_ISO7816_T1_PROLOGUE_BYTE_SIZE     (3UL)
#define HW_ISO7816_T1_INF_MAX_BYTE_SIZE      (254UL)
#define HW_ISO7816_T1_RETRIES_MAX            (2UL)
#define HW_ISO7816_T1_RESYNC_MAX             (3UL)
#define HW_ISO7816_T1_CWI_DEFAULT            (13UL)
#define HW_ISO7816_T1_BWI_DEFAULT            (4UL)
#define HW_ISO7816_T1_IFSC_DEFAULT           (32UL)
#define HW_ISO7816_T1_IFSD_DEFAULT           (32UL)
#define HW_ISO7816_T1_ERR_DET_DEFAULT        (HW_ISO7816_T1_ERR_DET_LRC)

#define HW_ISO7816_FD_VALUE                  (372UL)
#define HW_ISO7816_DD_VALUE                  (1UL)
#define HW_ISO7816_FMAXD_VALUE               (5000UL)

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief SW1SW2 status codes
 */
typedef enum {
        HW_ISO7816_SW1SW2_INVALID        = 0x6000,      /**< Invalid status code */
        HW_ISO7816_SW1SW2_RESP_GET_RESP  = 0x6100,      /**< Process completed normally, SW2 bytes available */
        HW_ISO7816_SW1SW2_RESP_WARN1     = 0x6200,      /**< Process completed with warning */
        HW_ISO7816_SW1SW2_RESP_WARN2     = 0x6300,      /**< Process completed with warning */
        HW_ISO7816_SW1SW2_RESP_LEN_ERR   = 0x6700,      /**< Process aborted due to wrong length */
        HW_ISO7816_SW1SW2_RESP_LEN_RETRY = 0x6C00,      /**< Process aborted, SW2 bytes available */
        HW_ISO7816_SW1SW2_RESP_INV_INS   = 0x6D00,      /**< Process aborted, invalid instruction code */
        HW_ISO7816_SW1SW2_RESP_SUCCESS   = 0x9000,      /**< Process completed normally */
} HW_ISO7816_SW1SW2;

/**
 * \brief UART interrupt Identification codes
 */
typedef enum {
        HW_ISO7816_UART_INT_MODEM_STAT         = 0,     /**< Modem status */
        HW_ISO7816_UART_INT_NO_INT_PEND        = 1,     /**< No interrupt pending */
        HW_ISO7816_UART_INT_THR_EMPTY          = 2,     /**< Transmit holding register empty */
        HW_ISO7816_UART_INT_RECEIVED_AVAILABLE = 4,     /**< Received data available */
        HW_ISO7816_UART_INT_RECEIVE_LINE_STAT  = 6,     /**< Receiver line status */
        HW_ISO7816_UART_INT_BUSY_DETECTED      = 7,     /**< Busy detect indication */
        HW_ISO7816_UART_INT_TIMEOUT            = 12,    /**< Character timeout indication */
} HW_ISO7816_UART_INT;

/**
 * \brief UART interrupt Identification codes
 */
typedef enum {
        HW_ISO7816_STATUS_TIM_EXPIRED  = 1 << 0,        /**< ISO7816 timer expired */
        HW_ISO7816_STATUS_ERR_TX_TIME  = 1 << 1,        /**< TX value error indication sampled */
        HW_ISO7816_STATUS_ERR_TX_VALUE = 1 << 2,        /**< TX value error indication detected */
} HW_ISO7816_STATUS_INT;


/**
 * \brief ISO7816 character convention
 */
typedef enum {
        HW_ISO7816_CONV_UNKNOWN = 0x00,                 /**< Convention undefined/unknown */
        HW_ISO7816_CONV_DIR     = 0x3B,                 /**< Convention direct */
        HW_ISO7816_CONV_INV     = 0x3F,                 /**< Convention inverse */
} HW_ISO7816_CONV;

/**
 * \brief ATR's TDi byte mask
 */
typedef enum {
        HW_ISO7816_TDi_VALUE = 0b00001111,              /**< TDi value */
        HW_ISO7816_Yi_VALUE  = 0b11110000,              /**< Yi value */
        HW_ISO7816_TAi_PRSNT = 0b00010000,              /**< TAi present */
        HW_ISO7816_TBi_PRSNT = 0b00100000,              /**< TBi present */
        HW_ISO7816_TCi_PRSNT = 0b01000000,              /**< TCi present */
        HW_ISO7816_TDi_PRSNT = 0b10000000,              /**< TDi present */
} HW_ISO7816_TDI_MSK;

/**
 * \brief ISO7816 clock stop mode
 */
typedef enum {
        HW_ISO7816_CLOCK_STOP_NOT_SUP = 0b00,           /**< Clock stop not supported by card */
        HW_ISO7816_CLOCK_STOP_LOW     = 0b01,           /**< Clock low when stopped */
        HW_ISO7816_CLOCK_STOP_HIGH    = 0b10,           /**< Clock high when stopped */
        HW_ISO7816_CLOCK_STOP_NO_PREF = 0b11,           /**< No preference of clock state when stopped */
} HW_ISO7816_CLK_STOP;

/**
 * \brief Card operation class
 */
typedef enum {
        HW_ISO7816_OPER_CLASS_A     = 0b000001,         /**< Operation class A */
        HW_ISO7816_OPER_CLASS_B     = 0b000010,         /**< Operation class B */
        HW_ISO7816_OPER_CLASS_C     = 0b000100,         /**< Operation class C */
        HW_ISO7816_OPER_CLASS_A_B   = 0b000011,         /**< Operation classes A and B */
        HW_ISO7816_OPER_CLASS_B_C   = 0b000110,         /**< Operation classes B and C */
        HW_ISO7816_OPER_CLASS_A_B_C = 0b000111,         /**< Operation classes A,B and C */
} HW_ISO7816_OPER_CLASS;

/**
 * \brief T=1 Redundancy detection algorithm
 */
typedef enum {
        HW_ISO7816_T1_ERR_DET_LRC = 0b0,                /**< Longitudinal redundancy code */
        HW_ISO7816_T1_ERR_DET_CRC = 0b1,                /**< Cyclic redundancy code */
} HW_ISO7816_T1_ERR_DET;

/**
 * \brief ISO7816 receive status bits
 */
typedef enum {
        HW_ISO7816_RX_ERR_OK = 0,                           /**< No error */
        HW_ISO7816_RX_ERR_PARITY,                       /**< Parity error */
        HW_ISO7816_RX_ERR_TIMEOUT,                      /**< Timeout error */
} HW_ISO7816_RX_ERR;

/**
 * \brief ISO7816 error status
 */
typedef enum {
        HW_ISO7816_ERR_OK = 0,                              /**< No error */
        HW_ISO7816_ERR_CRC,                             /**< CRC or parity error */
        HW_ISO7816_ERR_PPS,                             /**< Error in PPS exchange */
        HW_ISO7816_ERR_PPS_PART_ACPT,                   /**< PPS is partially accepted */
        HW_ISO7816_ERR_CARD_NO_RESP,                    /**< Card is unresponsive (RX timed-out) */
        HW_ISO7816_ERR_CARD_ABORTED,                    /**< Card aborted transaction */
        HW_ISO7816_ERR_UNKNOWN,                         /**< Generic error */
} HW_ISO7816_ERROR;

/**
 * \brief ISO7816 receiver FIFO interrupt level
 */
typedef enum {
        HW_ISO7816_FIFO_RX_LVL_ONE_CHAR  = 0,           /**< Interrupt produced when a single character in FIFO */
        HW_ISO7816_FIFO_RX_LVL_QRTR_FULL = 1,           /**< Interrupt produced when 1/4 of FIFO is full/ */
        HW_ISO7816_FIFO_RX_LVL_HALF_FULL = 2,           /**< Interrupt produced when 1/2 of FIFO is full */
        HW_ISO7816_FIFO_RX_LVL_TWO_EMPTY = 3,           /**< Interrupt produced when 2 characters are empty */
} HW_ISO7816_FIFO_RX_LVL;

/**
 * \brief ISO7816 transmitter FIFO interrupt level
 */
typedef enum {
        HW_ISO7816_FIFO_TX_LVL_EMPTY     = 0,           /**< Interrupt produced when FIFO is empty */
        HW_ISO7816_FIFO_TX_LVL_TWO_CHARS = 1,           /**< Interrupt produced when 2 characters are in FIFO */
        HW_ISO7816_FIFO_TX_LVL_QRTR_FULL = 2,           /**< Interrupt produced when 1/4 of FIFO is full */
        HW_ISO7816_FIFO_TX_LVL_HALF_FULL = 3,           /**< Interrupt produced when 1/2 of FIFO is full */
} HW_ISO7816_FIFO_TX_LVL;

/**
 * \brief Protocol control byte type
 */
typedef enum {
        HW_ISO7816_I_PCB,                               /**< Information block */
        HW_ISO7816_R_PCB,                               /**< Receive ready block */
        HW_ISO7816_S_PCB,                               /**< Supervisory block */
        HW_ISO7816_INV_PCB,                             /**< Invalid block type */
} HW_ISO7816_PCB_BLOCK;

/**
 * \brief Information block protocol control byte masks
 */
typedef enum {
        HW_ISO7816_I_PCB_MSK_N = 0b01000000,            /**< Send-sequence number (N(S)) */
        HW_ISO7816_I_PCB_MSK_M = 0b00100000,            /**< More data bit (M-bit) */
} HW_ISO7816_I_PCB_MSK;

/**
 * \brief Receive ready block protocol control byte masks
 */
typedef enum {
        HW_ISO7816_R_PCB_MSK_N   = 0b00010000,          /**< Error indication bit */
        HW_ISO7816_R_PCB_MSK_ERR = 0b00001111,          /**< Error code nibble */
} HW_ISO7816_R_PCB_MSK;

/**
 * \brief Receive ready block protocol control byte values
 */
typedef enum {
        HW_ISO7816_R_PCB_ERR_NO_ERROR = 0b10000000,     /**< Error free acknowledgment */
        HW_ISO7816_R_PCB_ERR_CRC      = 0b10000001,     /**< Redundancy or parity error */
        HW_ISO7816_R_PCB_ERR_OTHER    = 0b10000010,     /**< Other errors */
} HW_ISO7816_R_PCB_VAL;

/**
 * \brief Supervisory block protocol control byte values
 */
typedef enum {
        HW_ISO7816_S_PCB_RESYNCH_REQ  = 0b11000000,     /**< RESYNCH request */
        HW_ISO7816_S_PCB_RESYNCH_RESP = 0b11100000,     /**< RESYNCH response */
        HW_ISO7816_S_PCB_IFS_REQ      = 0b11000001,     /**< IFS request */
        HW_ISO7816_S_PCB_IFS_RESP     = 0b11100001,     /**< IFS response */
        HW_ISO7816_S_PCB_ABORT_REQ    = 0b11000010,     /**< ABORT request */
        HW_ISO7816_S_PCB_ABORT_RESP   = 0b11100010,     /**< ABORT response */
        HW_ISO7816_S_PCB_WTX_REQ      = 0b11000011,     /**< WTX request */
        HW_ISO7816_S_PCB_WTX_RESP     = 0b11100011,     /**< WTX response */
} HW_ISO7816_S_PCB_VAL;

/**
 * \brief ISO7816 pin definition
 */
typedef struct {
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
} hw_iso7816_pad;

/**
 * \brief Standard or Proprietary Use contact (SPU)
 */
typedef struct {
        bool proprietary;                               /**< Standard or proprietary use */
        uint8_t value;                                  /**< Value of SPU */
} hw_iso7816_spu_t;

/**
 * \brief Protocol and Parameters Selection (PPS) parameters
 */
typedef struct {
        uint8_t t;                                      /**< Selected protocol (T=0 or T=1) */
        int8_t d;                                       /**< Baud rate adjustment integer */
        uint16_t f;                                     /**< Clock rate conversion integer */
        uint16_t fmax;                                  /**< Maximum frequency of the clock signal
                                                             provided to the card (KHz) */
        hw_iso7816_spu_t spu;                           /**< Standard or proprietary use contact */
} hw_iso7816_pps_params_t;

/**
 * \brief T=0 protocol parameters
 */
typedef struct {
        bool available;                                 /**< Protocol supported by card */
        uint8_t WI;                                     /**< Waiting time integer, default: 10 */
} hw_iso7816_t0_prot_t;

/**
 * \brief ISO7816 T=1 protocol state
 */
typedef struct {
        uint8_t IFSC;                                   /**< Maximum information field size of card,
                                                             default: 32 */
        uint8_t IFSD;                                   /**< Maximum information field size of
                                                             interface, default: 32 */
        bool tx_sn;                                     /**< TX sequence number */
        bool rx_sn;                                     /**< RX sequence number */
} hw_iso7816_t1_state;

/**
 * \brief T=1 protocol parameters
 */
typedef struct {
        bool available;
        uint8_t CWI;                                    /**< Character waiting time integer,
                                                             default: 13, range: 0-15 */
        uint8_t BWI;                                    /**< Block waiting time integer, default: 4,
                                                             range: 0-9 */
        HW_ISO7816_T1_ERR_DET err_det;                  /**< Redundancy detection algorithm,
                                                             default: LRC */
        hw_iso7816_t1_state state;                      /**< Protocol state variables */
} hw_iso7816_t1_prot_t;

/**
 * \brief ISO7816 card communication parameters
 */
typedef struct {
        hw_iso7816_pps_params_t pps_params;                     /**< PPS parameters structure */
        HW_ISO7816_CONV convention;                             /**< Character convention used by card */
        uint8_t n;                                              /**< Extra guard time integer, default: 0,
                                                                     range: 0-254 */
        bool mode_negotiable;                                   /**< Card in specific or negotiable mode */
        bool able_to_change_neg_spec;                           /**< Ability to change between negotiable/specific mode
                                                                     \sa ISO7816-3:2006 specification, ATR:TA2:Bit8 */
        bool implicit_params;                                   /**< Defines the use of implicit F and D values,
                                                                     \sa ISO7816-3:2006 specification, ATR:TA2:Bit5 */
        HW_ISO7816_OPER_CLASS class;                            /**< Card operation class, default: Class A */
        HW_ISO7816_CLK_STOP clk_stop;                           /**< Card clock stop mode, default: Not supported */
        hw_iso7816_t0_prot_t t0_prot;                           /**< T=0 transmission protocol parameters */
        hw_iso7816_t1_prot_t t1_prot;                           /**< T=1 transmission protocol parameters */
        uint8_t historical[HW_ISO7816_ATR_MAX_HIST_BYTE_SIZE];  /**< Historical bytes */
} hw_iso7816_atr_params_t;

/**
 * \brief ISO7816 configuration
 */
typedef struct {
        hw_iso7816_pad rst;                             /**< Reset pin */
        hw_iso7816_pps_params_t pps_params;             /**< PPS parameters structure */
        uint8_t n;                                      /**< Extra guard time integer, default: 0, range: 0-254 */
        HW_ISO7816_FIFO_TX_LVL tx_lvl;                  /**< Transmit FIFO interrupt trigger level (applicable to
                                                             T=1 transactions only) */
        HW_ISO7816_FIFO_RX_LVL rx_lvl;                  /**< Receive FIFO interrupt trigger level */
        HW_ISO7816_CONV conv;                           /**< Character convention used */
        hw_iso7816_t0_prot_t t0_prot;                   /**< T=0 transmission parameters */
        hw_iso7816_t1_prot_t t1_prot;                   /**< T=1 transmission parameters */
} hw_iso7816_config_t;

/*
 * API FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 * \name                Callback function definitions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Callback function type to be called when ATR reception is complete
 *
 * \param[in] user_data         User data passed at call of function hw_iso7816_receive_atr_async()
 * \param[in] status            Operation status
 *      \arg \c HW_ISO7816_ERR_CARD_NO_RESP ATR was not received, card unresponsive
 *      \arg \c HW_ISO7816_ERR_OK           ATR was successfully received
 * \param[in] len               Length of the ATR received
 */
typedef void (*hw_iso7816_atr_callback)(void *user_data, HW_ISO7816_ERROR status, size_t len);

/**
 * \brief Callback function type to be called when PPS transaction is complete
 *
 * \param[in] user_data         User data passed at call of function hw_iso7816_exchange_pps_async()
 * \param[in] status            Operation status
 *      \arg \c HW_ISO7816_ERR_PPS                    Error in PPS exchange procedure
 *      \arg \c HW_ISO7816_ERR_CRC                    PCK verification failed
 *      \arg \c HW_ISO7816_ERR_CARD_NO_RESP           Response was not received, card unresponsive
 *      \arg \c HW_ISO7816_ERR_PPS_PARTIALLY_ACCEPTED PPS successful, card proposed different parameters
 *      \arg \c HW_ISO7816_ERR_OK                     PPS successful, card accepted parameters
 */
typedef void (*hw_iso7816_pps_callback)(void *user_data, HW_ISO7816_ERROR status);

/**
 * \brief Callback function type to be called when APDU (T=0 or T=1) transaction is complete
 *
 * \param[in] user_data         User data passed at call of functions
 *                              hw_iso7816_apdu_transaction_t0_async(),
 *                              hw_iso7816_apdu_transaction_t1_async() and
 *                              hw_iso7816_supervisory_transaction_t1_async()
 * \param[in] status            Operation status
 *      \arg \c HW_ISO7816_ERR_CRC          CRC or parity error
 *      \arg \c HW_ISO7816_ERR_CARD_NO_RESP Response was not received, card unresponsive
 *      \arg \c HW_ISO7816_ERR_CARD_ABORTED Operation aborted by card
 *      \arg \c HW_ISO7816_ERR_UNKNOWN      Operation completed unsuccessfully
 *      \arg \c HW_ISO7816_ERR_OK           Operation completed successfully
 * \param[in] sw1sw2            Status code as provided by card. In case of error in communication
 *                              the \c HW_ISO7816_SW1SW2_INVALID value is returned.
 * \param[in] len               Length of the response APDU
 */
typedef void (*hw_iso7816_transact_callback)(void *user_data, HW_ISO7816_ERROR status,
        HW_ISO7816_SW1SW2 sw1sw2, size_t len);
/** \} */

/**
 * \name                Register manipulation functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set the clock divider
 *
 * Produced clock frequency is clk_freq = sclk / (2 * (ISO7816_CLK_DIV + 1)
 *
 * \param[in] div               Clock divisor to be set
 */
__STATIC_INLINE void hw_iso7816_set_clk_div(uint8_t div)
{
        REG_SETF(UART3, UART3_CTRL_REG, ISO7816_CLK_DIV, div);
}

/**
 * \brief Get the clock divider
 *
 * \return The clock divider
 */
__STATIC_INLINE uint8_t hw_iso7816_get_clk_div(void)
{
        return REG_GETF(UART3, UART3_CTRL_REG, ISO7816_CLK_DIV);
}

/**
 * \brief Get timer operation status
 *
 * \return If timer is enabled
 * \retval true Timer is running
 * \retval false Timer is not running
 */
__STATIC_INLINE bool hw_iso7816_get_timer_status(void)
{
        return REG_GETF(UART3, UART3_CTRL_REG, ISO7816_CLK_EN);
}

/**
 * \brief Get timer expiration status
 *
 * \return If timer has expired
 * \retval true Timer has expired
 * \retval false Timer has not expired
 */
__STATIC_INLINE bool hw_iso7816_get_timer_expire_status(void)
{
        return REG_GETF(UART3, UART3_IRQ_STATUS_REG, ISO7816_TIM_EXPIRED_IRQ);
}

/**
 * \brief Set the error signal enable
 *
 * \param[in] enable            True to enable and false to disable the error signal generation
 */
__STATIC_INLINE void hw_iso7816_set_error_signal_en(bool enable)
{
        REG_SETF(UART3, UART3_CONFIG_REG, ISO7816_ERR_SIG_EN, enable ? 1UL : 0UL);
}

/**
 * \brief Set the automatic guard time
 *
 * \param[in] enable            True to enable and false to disable the automatic guard time
 */
__STATIC_INLINE void hw_iso7816_set_auto_gt(bool enable)
{
        REG_SETF(UART3, UART3_CTRL_REG, ISO7816_AUTO_GT, enable ? 1UL : 0UL);
        REG_SETF(UART3, UART3_MCR_REG, UART_AFCE, enable ? 1UL : 0UL);
}

/**
 * \brief Get the error signal enable
 *
 * \return If error signal generation is enabled
 * \retval true Error signal generation enabled
 * \retval false Error signal generation disabled
 */
__STATIC_INLINE bool hw_iso7816_get_error_signal_en(void)
{
        return REG_GETF(UART3, UART3_CONFIG_REG, ISO7816_ERR_SIG_EN) ? true : false;
}

/**
 * \brief Enable both FIFOs
 *
 * Thresholds should be set before for predictable results.
 */
__STATIC_INLINE void hw_iso7816_enable_fifo(bool enable)
{
        REG_SETF(UART3, UART3_SFE_REG, UART_SHADOW_FIFO_ENABLE, enable ? 1UL : 0UL);
}

/**
 * \brief Set the receive FIFO trigger level at which the Received Data Available Interrupt is
 * generated
 *
 * \param [in] lvl              The receive FIFO trigger level, \sa HW_ISO7816_FIFO_RX_LVL
 */
__STATIC_INLINE void hw_iso7816_rx_fifo_tr_lvl_setf(HW_ISO7816_FIFO_RX_LVL lvl)
{
        REG_SETF(UART3, UART3_SRT_REG, UART_SHADOW_RCVR_TRIGGER, lvl);
}

/**
 * \brief Set the transmit FIFO trigger level at which the Transmit Holding Register Empty (THRE)
 * Interrupt is generated
 *
 * \param [in] lvl              The transmit FIFO trigger level, \sa HW_ISO7816_FIFO_TX_LVL
 */
__STATIC_INLINE void hw_iso7816_tx_fifo_tr_lvl_setf(HW_ISO7816_FIFO_TX_LVL lvl)
{
        REG_SETF(UART3, UART3_STET_REG, UART_SHADOW_TX_EMPTY_TRIGGER, lvl);
}

/**
 * \brief Reset transmit FIFO
 */
__STATIC_INLINE void hw_iso7816_tx_fifo_flush(void)
{
        UART3->UART3_SRR_REG = REG_MSK(UART3, UART3_SRR_REG, UART_XFR);
}

/**
 * \brief Reset receive FIFO
 */
__STATIC_INLINE void hw_iso7816_rx_fifo_flush(void)
{
        UART3->UART3_SRR_REG = REG_MSK(UART3, UART3_SRR_REG, UART_RFR);
}

/**
 * \brief Get the UART Interrupt ID
 *
 * \return interrupt type
 */
__STATIC_INLINE HW_ISO7816_UART_INT hw_iso7816_get_uart_interrupt_id(void)
{
        return (HW_ISO7816_UART_INT) REG_GETF(UART3, UART3_IIR_FCR_REG, IIR_FCR) & 0xFUL;
}

/**
 * \brief Get the ISO7816 Interrupt ID
 *
 * \return interrupt type
 */
__STATIC_INLINE HW_ISO7816_STATUS_INT hw_iso7816_get_status_interrupt_id(void)
{
        uint32_t ctrl_reg = UART3->UART3_CTRL_REG;
        return (HW_ISO7816_STATUS_INT) UART3->UART3_IRQ_STATUS_REG & (ctrl_reg >> 8) & 0x7UL;
}

/**
 * \brief Read receive buffer register
 *
 * \return the read byte
 */
__STATIC_INLINE uint8_t hw_iso7816_rxdata_getf()
{
        return REG_GETF(UART3, UART3_RBR_THR_DLL_REG, RBR_THR_DLL);
}

/**
 * \brief Write byte to the transmit holding register
 *
 * \param [in] data byte to be written
 */
__STATIC_INLINE void hw_iso7816_txdata_setf(uint8_t data)
{
        uint32_t uart3_rbr_thr_dll_reg = 0;
        REG_SET_FIELD(UART3, UART3_RBR_THR_DLL_REG, RBR_THR_DLL, uart3_rbr_thr_dll_reg, data);
        UART3->UART3_RBR_THR_DLL_REG = uart3_rbr_thr_dll_reg;
}

/**
 * \brief Check if receive FIFO is not empty
 *
 * \return true if FIFO is not empty
 */
__STATIC_INLINE bool hw_iso7816_receive_fifo_not_empty(void)
{
        return REG_GETF(UART3, UART3_USR_REG, UART_RFNE) != 0;
}

/**
 * \brief Check if transmit FIFO is not full
 *
 * \return true if FIFO is full
 */
__STATIC_INLINE bool hw_iso7816_transmit_fifo_not_full(void)
{
        return REG_GETF(UART3, UART3_USR_REG, UART_TFNF) != 0;
}

/**
 * \brief Check if transmit FIFO is empty
 *
 * \return true if FIFO empty
 */
__STATIC_INLINE bool hw_iso7816_transmit_fifo_empty()
{
        return REG_GETF(UART3, UART3_USR_REG, UART_TFE) != 0;
}

/**
 * \brief Get the value of the Transmit Holding Register Empty bit
 *
 * \return the Transmit Holding Register Empty bit value
 */
__STATIC_INLINE uint8_t hw_iso7816_thr_empty_getf(void)
{
        return REG_GETF(UART3, UART3_LSR_REG, UART_THRE);
}
/** \} */

/**
 * \name                Configuration functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Calculate the clock divisor using the fmax
 *
 * \param[in] fmax              fmax as it is provided by the card (in KHz)
 * \return Clock divisor
 */
uint8_t hw_iso7816_calc_clk_div(uint16_t fmax);

/**
 * \brief Calculate UART's baud rate using the card parameters
 *
 * \param[in] f                 Clock rate conversion integer
 * \param[in] d                 Baud rate adjustment integer
 * \param[in] fmax              Maximum frequency of the clock signal provided to the card
 * \return Calculated baud rate in the form DLH(8 bits), DLL(8 bits), DLF(8 bits)
 */
uint32_t hw_iso7816_calc_baud_rate(uint16_t f, int8_t d, uint16_t fmax);

/**
 * \brief Set the baud rate parameters
 *
 * \param[in] baud              Baud rate value in the form DLH(8 bits), DLL(8 bits), DLF(8 bits)
 *
 * \return If baud rate was applied
 * \retval true Baud rate was successfully applied
 * \retval false UART was busy and baud rate was not applied
 */
bool hw_iso7816_set_baud(uint32_t baud);

/**
 * \brief Get the baud rate parameters
 *
 * \return Baud rate value in the form DLH(8 bits), DLL(8 bits), DLF(8 bits)
 */
uint32_t hw_iso7816_get_baud(void);

/**
 * \brief Get default parameters of the ATR parameters
 *
 * \param[out] params           ATR parameters
 */
void hw_iso7816_get_default_atr_params(hw_iso7816_atr_params_t *params);

/**
 * \brief Fill the configuration structure using the ATR parameters
 *
 * \param[in]  params           ATR parameters
 * \param[out] cfg              Configuration structure
 *
 * \note Only part of the configuration can be filled with ATR parameters
 */
void hw_iso7816_convert_atr_params_to_cfg(const hw_iso7816_atr_params_t *params,
        hw_iso7816_config_t *cfg);

/**
 * \brief Initialize ISO7816 module and low level driver
 *
 * \param[in] cfg               Configuration to be applied
 */
void hw_iso7816_init(const hw_iso7816_config_t *cfg);

/**
 * \brief Disables ISO7816 controller
 */
void hw_iso7816_deinit(void);

/**
 * \brief Set the character convention
 *
 * \param[in] conv              Convention to be set
 *
 * \return If operation completed successfully
 * \retval true  Convention was set successfully
 * \retval false Convention was not set (UART busy)
 */
bool hw_iso7816_set_convention(HW_ISO7816_CONV conv);

/**
 * \brief Get the busy state of the ISO7816 module
 *
 * \return If a transaction is pending
 * \retval true A transaction is in progress
 * \retval false There is no transaction in progress
 */
bool hw_iso7816_is_busy(void);
/** \} */

/**
 * \name                Activation functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Perform a cold reset of the card as described in ISO7816-3:2006, chapter 6.2.2
 */
void hw_iso7816_cold_reset(void);

/**
 * \brief Perform a warm reset of the card as described in ISO7816-3:2006, chapter 6.2.3
 */
void hw_iso7816_warm_reset(void);

/**
 * \brief Perform a deactivation of the card as described in ISO7816-3:2006, chapter 6.4
 */
void hw_iso7816_deactivate(void);

/**
 * \brief Stop clock produced by interface if supported by the card (ISO7816-3:2006, chapter 6.3.2)
 *
 * \param[in] clk_stop          Clock stop mode, as provided by card in the ATR, global iface TA15
 */
void hw_iso7816_clock_stop(HW_ISO7816_CLK_STOP clk_stop);

/**
 * \brief Resume clock operation, when stopped with hw_iso7816_clock_stop (ISO7816-3:2006, chapter
 * 6.3.2)
 */
void hw_iso7816_clock_resume(void);
/** \} */

/**
 * \name                Timer functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set ISO7816 timer to count specified clock ticks
 *
 * The duration of a tick is configured by \ref hw_iso7816_set_clk_div() and matches the output
 * clock of the module.
 *
 * \param[in] cycles            Ticks to count
 */
void hw_iso7816_start_timer(uint32 cycles);

/**
 * \brief Stop ISO7816 timer and clear corresponding interrupt flag
 */
void hw_iso7816_stop_timer(void);

/**
 * \brief Use ISO7816 timer to produce a delay of specified clock ticks.
 *
 * \sa hw_iso7816_start_timer()
 *
 * \param[in] cycles            Ticks to wait
 */
void hw_iso7816_timer_delay(uint16 cycles);

/**
 * \brief Set guard time between transmitted characters
 *
 * Function has to be called each time a new TX session begins to setup guard time
 *
 * \param[in] guard_time        Guard time in ETUs (Maximum value: 0x1000)
 */
void hw_iso7816_set_guard_time(uint16_t guard_time);

/**
 * \brief Function to reset the ISO7816 timer IRQ status bit, when using timer in mode 1: guard time
 *
 * \note Call only when timer is expired (ISO7816_TIM_EXPIRED_IRQ bit is set), otherwise TIM_MAX has
 * to be set
 */
void hw_iso7816_reset_timer_irq(void);
/** \} */

/**
 * \name                Protocol exchange functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Receive Answer To Reset (ATR) from card after a cold or warm reset
 *
 * \param[out] rx_buff          Buffer to place the ATR bytes with size at least
 *                              HW_ISO7816_ATR_MAX_BYTE_SIZE
 * \param[in] cb                Callback to be called upon completion (from interrupt)
 * \param[in] user_data         Parameter passed to callback function
 *
 * \warning RX buffer must be valid until callback function is called.
 */
void hw_iso7816_receive_atr_async(uint8_t *rx_buff, hw_iso7816_atr_callback cb, void *user_data);

/**
 * \brief Parse received ATR and fill the corresponding fields of the card communication parameters
 *
 * \param[in]  atr              Buffer containing the ATR
 * \param[in]  len              Length of ATR in bytes
 * \param[out] params           Card communication parameters
 * \return Status of operation
 * \retval HW_ISO7816_ERR_UNKNOWN An error occurred during parsing the ATR
 * \retval HW_ISO7816_ERR_CRC     The TCK verification failed
 * \retval HW_ISO7816_ERR_OK      ATR was parsed successfully
 */
HW_ISO7816_ERROR hw_iso7816_parse_atr(const uint8_t *atr, size_t len,
        hw_iso7816_atr_params_t *params);

/**
 * \brief Sends Protocol and Parameters Selection (PPS) request to the card and receives the card
 * response
 *
 * Function updates the PPS parameters as well as the card communication parameters with the
 * exchanged parameters and updates the timings if necessary. After a successful PPS, a warm reset,
 * a card re-initialization or a deactivation may be necessary.
 *
 * Parameters are grouped into PPS bytes and they are the following:
 *
 *  PPS byte  |  Parameters  |  Mandatory
 * ---------- | ------------ | -------------------
 *  PPS0      |  T           |  Yes
 *  PPS1      |  f, fmax, d  |  No
 *  PPS2      |  SPU         |  No
 *
 * The non mandatory bytes and the corresponding parameters are omitted if the \p pps1 or \p pps2
 * parameters are set to false.
 *
 * \param[in,out] params        PPS parameters to request
 * \param[in]     pps1          True if PPS request will include PPS1 byte (speed parameters)
 * \param[in]     pps2          True if PPS request will include PPS2 byte (SPU parameters)
 * \param[in]     cb            Callback to be called upon completion (from interrupt)
 * \param[in]     user_data     Parameter passed to callback function
 *
 * \warning PPS parameters input must be valid until callback function is called.
 */
void hw_iso7816_exchange_pps_async(hw_iso7816_pps_params_t *params, bool pps1, bool pps2,
        hw_iso7816_pps_callback cb, void *user_data);
/** \} */

/**
 * \name                T=0 Protocol functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Calculate the waiting and guard times of T=0 using the provided settings
 *
 * \param[in] n                 Extra guard time integer
 * \param[in] f                 Clock rate conversion integer
 * \param[in] d                 Baud rate adjustment integer
 * \param[in] t0_prot           T=0 protocol parameters
 */
void hw_iso7816_calculate_protocol_times_t0(uint8_t n, uint16_t f, int8_t d,
        const hw_iso7816_t0_prot_t *t0_prot);

/**
 * \brief Perform a single APDU transaction over T=0 transmission protocol.
 *
 * \param[in]  tx_buff          Buffer containing the APDU to transmit
 * \param[in]  tx_len           Length in bytes of the APDU to transmit
 * \param[out] rx_buff          Buffer to receive the response APDU with size at least the maximum
 *                              expected response APDU
 * \param[in]  cb               Callback to be called upon completion (from interrupt)
 * \param[in]  user_data        Parameter passed to callback function
 *
 * \warning TX and RX buffer must be valid until callback function is called.
 */
void hw_iso7816_apdu_transact_t0_async(const uint8_t *tx_buff, size_t tx_len, uint8_t *rx_buff,
        hw_iso7816_transact_callback cb, void *user_data);
/** \} */

/**
 * \name                T=1 Protocol functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Retrieve the current values of the T=1 protocol state variables.
 *
 * \param[out] state            State variables of T=1 protocol
 */
void hw_iso7816_get_state_t1(hw_iso7816_t1_state *state);

/**
 * \brief Calculate the waiting and guard times of T=1 using the provided settings
 *
 * \param[in] n                 Extra guard time integer
 * \param[in] f                 Clock rate conversion integer
 * \param[in] d                 Baud rate adjustment integer
 * \param[in] t1_prot           T=1 protocol parameters
 */
void hw_iso7816_calculate_protocol_times_t1(uint8_t n, uint16_t f, int8_t d,
        const hw_iso7816_t1_prot_t *t1_prot);

/**
 * \brief Perform a single APDU transaction over T=1 transmission protocol.
 *
 * \param[in]  tx_buff          Buffer containing the APDU to transmit
 * \param[in]  tx_len           Length in bytes of the APDU to transmit
 * \param[out] rx_buff          Buffer to receive the response APDU with size at least the maximum
 *                              expected response APDU
 * \param[in]  cb               Callback to be called upon completion (from interrupt)
 * \param[in]  user_data        Parameter passed to callback function
 *
 * \warning TX and RX buffer must be valid until callback function is called.
 */
void hw_iso7816_apdu_transact_t1_async(const uint8_t *tx_buff, size_t tx_len, uint8_t *rx_buff,
        hw_iso7816_transact_callback cb, void *user_data);

/**
 * \brief Transmit a T=1 supervisory block of the provided type and value (if applicable)
 *
 * \param[in] type              Type of supervisory block
 * \param[in] value             Value of the supervisory block data (if applicable)
 * \param[in] cb                Callback to be called upon completion (from interrupt)
 * \param[in] user_data         Parameter passed to callback function
 */
void hw_iso7816_supervisory_transact_t1_async(HW_ISO7816_S_PCB_VAL type, uint8_t *value,
        hw_iso7816_transact_callback cb, void *user_data);
/** \} */

#endif /* dg_configUSE_HW_ISO7816 */


#endif /* HW_ISO7816_H */
/**
 * \}
 * \}
 * \}
 */
