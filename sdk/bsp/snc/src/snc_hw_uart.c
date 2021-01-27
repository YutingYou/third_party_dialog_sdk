/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_UART
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_uart.c
 *
 * @brief SNC-Implementation of UART Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_UART

#include "snc_defs.h"
#include "snc_hw_sys.h"
#include "snc_hw_gpio.h"

#include "snc_hw_uart.h"

/* The size of the UART FIFOs */
#define SNC_HW_UART_FIFO_SIZE           ( 16 )
/* The number of characters whose total transfer time is considered as character timeout */
#define SNC_HW_UART_CHAR_TIMEOUT        ( 4 )

#define SNC_UART_GPIO_VALID(port, pin)   ((port != HW_GPIO_PORT_NONE) && (pin != HW_GPIO_PIN_NONE))

/*
 * ENUMERATION, DATA STARUCTURE AND DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/*
 * UART Transmit Holding Register Empty (THRE) FIFO trigger levels
 */
typedef enum {
        SNC_HW_UART_TX_FIFO_TR_LVL_EMPTY = 0,           /* TX FIFO trigger level is FIFO empty */
        SNC_HW_UART_TX_FIFO_TR_LVL_2_CHARS,             /* TX FIFO trigger level is 2 characters in the FIFO */
        SNC_HW_UART_TX_FIFO_TR_LVL_0_25_FULL,           /* TX FIFO trigger level is FIFO 1/4 full */
        SNC_HW_UART_TX_FIFO_TR_LVL_0_5_FULL,            /* TX FIFO trigger level is FIFO 1/2 full */
} SNC_HW_UART_TX_FIFO_TR_LVL;

/*
 * UART Received Data Available FIFO trigger levels
 */
typedef enum {
        SNC_HW_UART_RX_FIFO_TR_LVL_1_CHAR = 0,          /* RX FIFO trigger level is 1 character in the FIFO */
        SNC_HW_UART_RX_FIFO_TR_LVL_0_25_FULL,           /* RX FIFO trigger level is FIFO 1/4 full */
        SNC_HW_UART_RX_FIFO_TR_LVL_0_5_FULL,            /* RX FIFO trigger level is FIFO 1/2 full */
        SNC_HW_UART_RX_FIFO_TR_LVL_2_CHARS_LT_FULL,     /* RX FIFO trigger level is 2 characters less than FIFO full */
} SNC_HW_UART_RX_FIFO_TR_LVL;

/*
 * UART pad latches operation type
 */
typedef enum {
        SNC_UART_PAD_LATCHES_OP_ENABLE,                 /* Enable pad latches */
        SNC_UART_PAD_LATCHES_OP_DISABLE                 /* Disable pad latches */
} SNC_UART_PAD_LATCHES_OP;

/*
 * Array holding the value of UARTx base addresses (HW_UARTx) in order to be used in comparisons
 * throughout the uCode implementations, so that the latter can be configured accordingly, based on
 * the ID of the UART controller instance being accessed
 */
_SNC_RETAINED static HW_UART_ID const_HW_UART[] = { HW_UART1, HW_UART2 };

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_hw_uart_set_src_clk_config(b_ctx_t* b_ctx, HW_UART_ID id, uint32_t clk_en,
        uint32_t clk_sel);
/**
 * \brief Function used in SNC context to configure UART source clock
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 * \param [in] clk_en   (uint32_t: build-time-only value)
 *                      enables or disables the UART clock source (1-Enable, 0-Disable)
 * \param [in] clk_sel  (uint32_t: build-time-only value)
 *                      selects the clock source (1-Div1 , 0-DivN)
 */
#define SNC_hw_uart_set_src_clk_config(id, clk_en, clk_sel)                                     \
        snc_hw_uart_set_src_clk_config(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id),                    \
                                              _SNC_OP_VALUE(uint32_t, clk_en),                  \
                                              _SNC_OP_VALUE(uint32_t, clk_sel))

static void snc_hw_uart_init(b_ctx_t* b_ctx, HW_UART_ID id, const uart_config_ex* cfg);
/**
 * \brief Function used in SNC context to initialize UART controller
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 * \param [in] cfg      (uart_config_ex*: build-time-only value)
 *                      pointer to the UART configuration structure
 */
#define SNC_hw_uart_init(id, cfg)                                                               \
        snc_hw_uart_init(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id),                                  \
                                _SNC_OP_VALUE(const uart_config_ex*, cfg))

static void snc_hw_uart_enable_fifo(b_ctx_t* b_ctx, HW_UART_ID id, bool enable);
/**
 * \brief Function used in SNC context to enable both UART FIFOs
 *
 * Thresholds should be set before, for predictable results.
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 * \param [in] enable   (bool: build-time-only value)
 *                      1 for enable; 0 for disable
 */
#define SNC_hw_uart_enable_fifo(id, enable)                                                     \
        snc_hw_uart_enable_fifo(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id),                           \
                                       _SNC_OP_VALUE(bool, enable))

static void snc_hw_uart_rx_fifo_tr_lvl_setf(b_ctx_t* b_ctx, HW_UART_ID id, uint8_t tr_lvl);
/**
 * \brief Function used in SNC context to set the UART receive FIFO trigger level
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 * \param [in] tr_lvl   (uint8_t: build-time-only value)
 *                      receive FIFO trigger level:
 *                      0 = 1 character in the FIFO,
 *                      1 = FIFO 1/4 full,
 *                      2 = FIFO 1/2 full,
 *                      3 = FIFO 2 less than full
 */
#define SNC_hw_uart_rx_fifo_tr_lvl_setf(id, tr_lvl)                                             \
        snc_hw_uart_rx_fifo_tr_lvl_setf(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id),                   \
                                               _SNC_OP_VALUE(uint8_t, tr_lvl))

static void snc_hw_uart_tx_fifo_tr_lvl_setf(b_ctx_t* b_ctx, HW_UART_ID id, uint8_t tr_lvl);
/**
 * \brief Function used in SNC context to set the UART transmit FIFO trigger level
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 * \param [in] tr_lvl   (uint8_t: build-time-only value)
 *                      transmit FIFO trigger level:
 *                      0 = FIFO empty,
 *                      1 = 2 characters in the FIFO,
 *                      2 = FIFO 1/4 full,
 *                      3 = FIFO 1/2 full
 */
#define SNC_hw_uart_tx_fifo_tr_lvl_setf(id, tr_lvl)                                             \
        snc_hw_uart_tx_fifo_tr_lvl_setf(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id),                   \
                                               _SNC_OP_VALUE(uint8_t, tr_lvl))

static void snc_hw_uart_is_tx_empty(b_ctx_t* b_ctx, HW_UART_ID id);
/**
 * \brief Function used in SNC context to check if UART transmitter is empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if UART transmitter is
 * empty, therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_tx_is_empty)) to decide
 * on an action.
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 */
#define SNC_hw_uart_is_tx_empty(id)                                                             \
        snc_hw_uart_is_tx_empty(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id))

static void snc_hw_uart_is_rx_fifo_not_empty(b_ctx_t* b_ctx, HW_UART_ID id);
/**
 * \brief Function used in SNC context to check if UART receiver FIFO is not empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if UART receiver FIFO is
 * not empty, therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_rx_fifo_is_not_empty))
 * to decide on an action.
 *
 * \param [in] id       (HW_UART_ID: build-time-only value)
 *                      UART controller instance (HW_UART1, HW_UART2, HW_UART3)
 */
#define SNC_hw_uart_is_rx_fifo_not_empty(id)                                                    \
        snc_hw_uart_is_rx_fifo_not_empty(b_ctx, _SNC_OP_VALUE(HW_UART_ID, id))

static void snc_uart_configure_pins(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to configure UART bus pads
 * *
 * \param [in] conf     (const snc_uart_controller_conf_t*: build-time-only value)
 *                      UART controller configuration
 */
#define SNC_uart_configure_pins(conf)                                                           \
        snc_uart_configure_pins(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf))

static void snc_uart_deconfigure_pins(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to de-configure UART bus pads
 * *
 * \param [in] conf     (const snc_uart_controller_conf_t*: build-time-only value)
 *                      UART controller configuration
 */
#define SNC_uart_deconfigure_pins(conf)                                                         \
        snc_uart_deconfigure_pins(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf))

static void snc_uart_pad_latches(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SNC_UART_PAD_LATCHES_OP pad_latches_op);
/**
 * \brief Function used in SNC context to configure UART bus pad latches
 * *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [in] pad_latches_op   (SNC_UART_PAD_LATCHES_OP: build-time-only value)
 *                              UART bus pad latches operation type
 */
#define SNC_uart_pad_latches(conf, pad_latches_op)                                              \
        snc_uart_pad_latches(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf),     \
                                    _SNC_OP_VALUE(SNC_UART_PAD_LATCHES_OP, pad_latches_op))

/*
 * Calculates the UART-transfer time for a specific number of characters
 */
static uint32_t snc_hw_uart_calc_transfer_time(const snc_uart_controller_conf_t* conf,
        uint8_t num_of_chars);

//==================== Controller Acquisition functions ========================

void snc_uart_open(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_UART1 || conf->id == HW_UART2 || conf->id == HW_UART3);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        const uart_config_ex* uart_hw_cfg = &conf->drv->hw_conf;
        bool auto_flow_control = conf->drv->hw_conf.auto_flow_control;
        uint32_t perif_id = (conf->id == HW_UART1) ? BSR_PERIPH_ID_UART1 :
                            ((conf->id == HW_UART2) ? BSR_PERIPH_ID_UART2 : BSR_PERIPH_ID_UART3);

        ASSERT_WARNING((conf->id == HW_UART1 && !auto_flow_control) ||
                ((conf->id == HW_UART2 || conf->id == HW_UART3) &&
                        (!auto_flow_control ||
                                (SNC_UART_GPIO_VALID(conf->io->ctsn.port, conf->io->ctsn.pin) &&
                                 SNC_UART_GPIO_VALID(conf->io->rtsn.port, conf->io->rtsn.pin))))
        );

        // UART controller instance acquisition
        SNC_hw_sys_bsr_acquire(perif_id);

        // UART controller configuration:
        // UART controller instance initialization:
        // Configure UART source clock
        SNC_hw_uart_set_src_clk_config(conf->id, 1, 0);
        // Initialize UART controller registers and status
        SNC_hw_uart_init(conf->id, uart_hw_cfg);

        // UART bus pads initialization
        SNC_uart_configure_pins(conf);

        /* Enable UART bus pad latches */
        SNC_uart_pad_latches(conf, SNC_UART_PAD_LATCHES_OP_ENABLE);
}

void snc_uart_close(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        uint32_t perif_id = (conf->id == HW_UART1) ? BSR_PERIPH_ID_UART1 :
                            ((conf->id == HW_UART2) ? BSR_PERIPH_ID_UART2 : BSR_PERIPH_ID_UART3);

        ASSERT_WARNING(b_ctx);

        // Wait until TX shift register and TX FIFO are both empty
        SNC_uart_wait_write_pending(conf);

        // Reset UART
        SENIS_wadva(da(&UBA(conf->id)->UART2_SRR_REG), 1 << REG_POS(UART2, UART2_SRR_REG, UART_UR));

        // Disable UART controller source clock
        SNC_hw_uart_set_src_clk_config(conf->id, 0, 0);

        // UART bus pads restored to the off state
        SNC_uart_deconfigure_pins(conf);

        // Disable UART BUS pad latches
        SNC_uart_pad_latches(conf, SNC_UART_PAD_LATCHES_OP_DISABLE);

        // Release UART controller instance
        SNC_hw_sys_bsr_release(perif_id);
}

//==================== Configuration functions =================================

static void snc_hw_uart_set_src_clk_config(b_ctx_t* b_ctx, HW_UART_ID id, uint32_t clk_en,
        uint32_t clk_sel)
{
        SENIS_wadva(da(&CRG_COM->RESET_CLK_COM_REG),
                (id == HW_UART1) ?
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART_ENABLE)) :
                ((id == HW_UART2) ?
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART2_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART2_CLK_SEL)) :
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART3_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, UART3_CLK_SEL))));
        SENIS_wadva(da(&CRG_COM->SET_CLK_COM_REG),
                (id == HW_UART1) ?
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, UART_ENABLE)) :
                ((id == HW_UART2) ?
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, UART2_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, UART2_CLK_SEL)) :
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, UART3_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, UART3_CLK_SEL))));
}

static void snc_hw_uart_init(b_ctx_t* b_ctx, HW_UART_ID id, const uart_config_ex* cfg)
{
        // Reset UART
        SENIS_wadva(da(&UBA(id)->UART2_SRR_REG), 1 << REG_POS(UART2, UART2_SRR_REG, UART_UR));

        // Setup UART FIFOs accordingly
        if (cfg->use_fifo) {
                SNC_hw_uart_enable_fifo(id, true);
                SNC_hw_uart_rx_fifo_tr_lvl_setf(id, cfg->rx_fifo_tr_lvl);
                SNC_hw_uart_tx_fifo_tr_lvl_setf(id, cfg->tx_fifo_tr_lvl);
        }

        // Set Auto flow control
        if (cfg->auto_flow_control) {
                SENIS_wadva(da(&UBA(id)->UART2_MCR_REG),
                        (1 << REG_POS(UART2, UART2_MCR_REG, UART_AFCE)) |
                        (1 << REG_POS(UART2, UART2_MCR_REG, UART_RTS)));
        }

        // Set Divisor Latch Access Bit in LCR register to access DLL & DLH registers
        SENIS_wadva(da(&UBA(id)->UART2_LCR_REG), 1 << REG_POS(UART2, UART2_LCR_REG, UART_DLAB));
        // Set fraction byte of baud rate
        SENIS_wadva(da(&UBA(id)->UART2_DLF_REG), 0xFF & cfg->baud_rate);
        // Set low byte of baud rate
        SENIS_wadva(da(&UBA(id)->UART2_RBR_THR_DLL_REG), 0xFF & (cfg->baud_rate >> 8));
        // Set high byte of baud rate
        SENIS_wadva(da(&UBA(id)->UART2_IER_DLH_REG), 0xFF & (cfg->baud_rate >> 16));
        // Reset Divisor Latch Access Bit in LCR register, and set Parity, Data and Stop Bits
        SENIS_wadva(da(&UBA(id)->UART2_LCR_REG),
                0 << REG_POS(UART2, UART2_LCR_REG, UART_DLAB)                   |
                (cfg->parity << REG_POS(UART2, UART2_LCR_REG, UART_PEN))        |
                (cfg->data << REG_POS(UART2, UART2_LCR_REG, UART_DLS))          |
                (cfg->stop << REG_POS(UART2, UART2_LCR_REG, UART_STOP)));
}

static void snc_hw_uart_enable_fifo(b_ctx_t* b_ctx, HW_UART_ID id, bool enable)
{
        SENIS_wadva(da(&UBA(id)->UART2_SFE_REG),
                enable << REG_POS(UART2, UART2_SFE_REG, UART_SHADOW_FIFO_ENABLE));
}

static void snc_hw_uart_rx_fifo_tr_lvl_setf(b_ctx_t* b_ctx, HW_UART_ID id, uint8_t tr_lvl)
{
        SENIS_wadva(da(&UBA(id)->UART2_SRT_REG),
                tr_lvl << REG_POS(UART2, UART2_SRT_REG, UART_SHADOW_RCVR_TRIGGER));
}

static void snc_hw_uart_tx_fifo_tr_lvl_setf(b_ctx_t* b_ctx, HW_UART_ID id, uint8_t tr_lvl)
{
        SENIS_wadva(da(&UBA(id)->UART2_STET_REG),
                tr_lvl << REG_POS(UART2, UART2_STET_REG, UART_SHADOW_TX_EMPTY_TRIGGER));
}

static void snc_uart_configure_pins(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        const snc_uart_io_conf_t* io_cfg = conf->io;
        uint32_t func_offset =
                (conf->id == HW_UART1) ? 0 :
                ((conf->id == HW_UART2) ? HW_GPIO_FUNC_UART2_RX - HW_GPIO_FUNC_UART_RX:
                        HW_GPIO_FUNC_UART3_RX - HW_GPIO_FUNC_UART_RX);

        // Configure RX
        if (SNC_UART_GPIO_VALID(io_cfg->rx.port, io_cfg->rx.pin)) {
                SNC_hw_gpio_set_pin_function(io_cfg->rx.port, io_cfg->rx.pin,
                        io_cfg->rx.on.mode, HW_GPIO_FUNC_UART_RX + func_offset);
        }

        // Configure TX
        if (SNC_UART_GPIO_VALID(io_cfg->tx.port, io_cfg->tx.pin)) {
                SNC_hw_gpio_set_pin_function(io_cfg->tx.port, io_cfg->tx.pin,
                        io_cfg->tx.on.mode, HW_GPIO_FUNC_UART_TX + func_offset);
        }

        if ((conf->id == HW_UART2 || conf->id == HW_UART3)) {
                func_offset =
                        (conf->id == HW_UART2) ? 0 : HW_GPIO_FUNC_UART3_CTSN - HW_GPIO_FUNC_UART2_CTSN;

                // Configure CTSN
                if (SNC_UART_GPIO_VALID(io_cfg->ctsn.port, io_cfg->ctsn.pin)) {
                        SNC_hw_gpio_set_pin_function(io_cfg->ctsn.port, io_cfg->ctsn.pin,
                                io_cfg->ctsn.on.mode, HW_GPIO_FUNC_UART2_CTSN + func_offset);
                }

                // Configure RTSN
                if (SNC_UART_GPIO_VALID(io_cfg->rtsn.port, io_cfg->rtsn.pin)) {
                        SNC_hw_gpio_set_pin_function(io_cfg->rtsn.port, io_cfg->rtsn.pin,
                                io_cfg->rtsn.on.mode, HW_GPIO_FUNC_UART2_RTSN + func_offset);
                }
        }
}

static void snc_uart_deconfigure_pins(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        const snc_uart_io_conf_t* io_cfg = conf->io;

        // De-configure RX
        if (SNC_UART_GPIO_VALID(io_cfg->rx.port, io_cfg->rx.pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->rx.port, io_cfg->rx.pin,
                        io_cfg->rx.off.mode, io_cfg->rx.off.function, io_cfg->rx.off.high);
        }

        // De-configure TX
        if (SNC_UART_GPIO_VALID(io_cfg->tx.port, io_cfg->tx.pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->tx.port, io_cfg->tx.pin,
                        io_cfg->tx.off.mode, io_cfg->tx.off.function, io_cfg->tx.off.high);
        }

        if ((conf->id == HW_UART2 || conf->id == HW_UART3)) {
                // De-configure CTSN
                if (SNC_UART_GPIO_VALID(io_cfg->ctsn.port, io_cfg->ctsn.pin)) {
                        SNC_hw_gpio_configure_pin(io_cfg->ctsn.port, io_cfg->ctsn.pin,
                                io_cfg->ctsn.off.mode, io_cfg->ctsn.off.function, io_cfg->ctsn.off.high);
                }

                // De-configure RTSN
                if (SNC_UART_GPIO_VALID(io_cfg->rtsn.port, io_cfg->rtsn.pin)) {
                        SNC_hw_gpio_configure_pin(io_cfg->rtsn.port, io_cfg->rtsn.pin,
                                io_cfg->rtsn.off.mode, io_cfg->rtsn.off.function, io_cfg->rtsn.off.high);
                }
        }
}

static void snc_uart_pad_latches(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SNC_UART_PAD_LATCHES_OP pad_latches_op)
{
        const snc_uart_io_conf_t* io_cfg = conf->io;
        uint32_t pads_stat[HW_GPIO_NUM_PORTS] = { 0 };

        if (SNC_UART_GPIO_VALID(io_cfg->rx.port, io_cfg->rx.pin)) {
                pads_stat[io_cfg->rx.port] |= (1 << io_cfg->rx.pin);
        }
        if (SNC_UART_GPIO_VALID(io_cfg->tx.port, io_cfg->tx.pin)) {
                pads_stat[io_cfg->tx.port] |= (1 << io_cfg->tx.pin);
        }
        if ((conf->id == HW_UART2 || conf->id == HW_UART3) && conf->drv->hw_conf.auto_flow_control) {
                if (SNC_UART_GPIO_VALID(io_cfg->ctsn.port, io_cfg->ctsn.pin)) {
                        pads_stat[io_cfg->ctsn.port] |= (1 << io_cfg->ctsn.pin);
                }
                if (SNC_UART_GPIO_VALID(io_cfg->rtsn.port, io_cfg->rtsn.pin)) {
                        pads_stat[io_cfg->rtsn.port] |= (1 << io_cfg->rtsn.pin);
                }
        }
        for (HW_GPIO_PORT port = HW_GPIO_PORT_0; port < HW_GPIO_NUM_PORTS; port++) {
                if (pads_stat[port]) {
                        if (pad_latches_op == SNC_UART_PAD_LATCHES_OP_ENABLE) {
                                SNC_hw_gpio_pads_latch_enable(port, pads_stat[port]);
                        } else {
                                SNC_hw_gpio_pads_latch_disable(port, pads_stat[port]);
                        }
                }
        }
}

void snc_uart_reconfig(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_UART1 || conf->id == HW_UART2 || conf->id == HW_UART3);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        const uart_config_ex* uart_hw_cfg = &conf->drv->hw_conf;
        bool auto_flow_control = conf->drv->hw_conf.auto_flow_control;

        ASSERT_WARNING((conf->id == HW_UART1 && !auto_flow_control) ||
                ((conf->id == HW_UART2 || conf->id == HW_UART3) &&
                        (!auto_flow_control ||
                                (SNC_UART_GPIO_VALID(conf->io->ctsn.port, conf->io->ctsn.pin) &&
                                 SNC_UART_GPIO_VALID(conf->io->rtsn.port, conf->io->rtsn.pin))))
        );

        // UART controller instance initialization:
        // Configure UART source clock
        SNC_hw_uart_set_src_clk_config(conf->id, 1, 0);
        // Initialize UART controller registers and status
        SNC_hw_uart_init(conf->id, uart_hw_cfg);
}

static uint32_t snc_hw_uart_calc_transfer_time(const snc_uart_controller_conf_t* conf,
        uint8_t num_of_chars)
{
        const uart_config_ex* uart_hw_cfg = &conf->drv->hw_conf;
        uint32_t tf_time;

        tf_time = (1 + (uart_hw_cfg->data + 5) + (uart_hw_cfg->parity > 0)) * 10;
        tf_time +=
                ((uart_hw_cfg->data == 0 && uart_hw_cfg->stop == HW_UART_STOPBITS_2)
                        ? (15) : ((uart_hw_cfg->stop + 1) * 10));
        tf_time *= 1000000;
        switch (uart_hw_cfg->baud_rate) {
        case HW_UART_BAUDRATE_1000000:
                tf_time /= 1000000;
                break;
        case HW_UART_BAUDRATE_500000:
                tf_time /= 500000;
                break;
        case HW_UART_BAUDRATE_230400:
                tf_time /= 230400;
                break;
        case HW_UART_BAUDRATE_115200:
                tf_time /= 115200;
                break;
        case HW_UART_BAUDRATE_57600:
                tf_time /= 57600;
                break;
        case HW_UART_BAUDRATE_38400:
                tf_time /= 38400;
                break;
        case HW_UART_BAUDRATE_28800:
                tf_time /= 28800;
                break;
        case HW_UART_BAUDRATE_19200:
                tf_time /= 19200;
                break;
        case HW_UART_BAUDRATE_14400:
                tf_time /= 14400;
                break;
        case HW_UART_BAUDRATE_9600:
                tf_time /= 9600;
                break;
        case HW_UART_BAUDRATE_4800:
                tf_time /= 4800;
                break;
        default:
                ASSERT_ERROR(0);
        }
        tf_time *= num_of_chars;
        tf_time = (tf_time < 1510) ? 0 :
                ((tf_time - 1510) * (dg_configXTAL32K_FREQ / 32) / ((1000000 / 32) * 10)) + 1;

        return tf_time;
}

//==================== Status Acquisition functions ============================

static void snc_hw_uart_is_tx_empty(b_ctx_t* b_ctx, HW_UART_ID id)
{
        SENIS_rdcbi(da(&UBA(id)->UART2_LSR_REG), REG_POS(UART2, UART2_LSR_REG, UART_TEMT));
}

static void snc_hw_uart_is_rx_fifo_not_empty(b_ctx_t* b_ctx, HW_UART_ID id)
{
        SENIS_rdcbi(da(&UBA(id)->UART2_USR_REG), REG_POS(UART2, UART2_USR_REG, UART_RFNE));
}

void snc_uart_is_write_pending(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE pending_type, uint32_t* pending)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(pending_type < SENIS_OPER_TYPE_VALUE);

        SENIS_labels(pt_return);

        senis_assign(b_ctx, pending_type, pending, _SNC_OP(0));
        SNC_hw_uart_is_tx_empty(conf->id);
        SENIS_cobr_eq(l(pt_return)); {
                senis_assign(b_ctx, pending_type, pending, _SNC_OP(1));
        }
        SENIS_label(pt_return);
}

SNC_FUNC_DECL(snc_uart_wait_write_pending_ucode, HW_UART_ID id, uint32_t wait_delay_senis_cmd,
        uint32_t nofifo_used);

void snc_uart_wait_write_pending(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        uint32_t tx_wait_delay;

        ASSERT_WARNING(b_ctx);

        // Wait until TX shift register and TX FIFO are both empty
        tx_wait_delay = _SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1));

        senis_call(b_ctx, SNC_UCODE_CTX(snc_uart_wait_write_pending_ucode), 2 * 3,
                        _SNC_OP(conf->id), _SNC_OP(tx_wait_delay),
                        _SNC_OP(conf->drv->hw_conf.use_fifo == 0));
}

SNC_FUNC_DEF(snc_uart_wait_write_pending_ucode, HW_UART_ID id, uint32_t wait_delay_senis_cmd,
        uint32_t nofifo_used)
{
        SENIS_labels(
                pt_hw_uart2_cfg, pt_hw_uart3_cfg,
                ph_ucode_cfg_UART2_TFL_REG, ph_ucode_cfg_UART2_LSR_REG,
                pt_hw_uart_cfg_done,

                pt_wait_tx_one_begin, pt_wait_tx_one_cond,
                pt_wait_tx_empty, ph_tx_check_delay, pt_tx_is_empty
        );

        // uCode configuration based on the implied UART bus ID
        {
                // Check which UART bus ID is implied
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART1 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart2_cfg));
                { // HW_UART1 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG),
                                _SENIS_1ST_OPERAND(_SENIS_B_RDCGR_DD(&UBA(HW_UART1)->UART2_TFL_REG,
                                        &snc_const[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG),
                                _SENIS_B_RDCBI_D(&UBA(HW_UART1)->UART2_LSR_REG,
                                        REG_POS(UART2, UART2_LSR_REG, UART_TEMT)));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart2_cfg);
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART2 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart3_cfg));
                { // HW_UART2 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG),
                                _SENIS_1ST_OPERAND(_SENIS_B_RDCGR_DD(&UBA(HW_UART2)->UART2_TFL_REG,
                                        &snc_const[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG),
                                _SENIS_B_RDCBI_D(&UBA(HW_UART2)->UART2_LSR_REG,
                                        REG_POS(UART2, UART2_LSR_REG, UART_TEMT)));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart3_cfg);
                { // HW_UART3 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG),
                                _SENIS_1ST_OPERAND(_SENIS_B_RDCGR_DD(&UBA(HW_UART3)->UART2_TFL_REG,
                                        &snc_const[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG),
                                _SENIS_B_RDCBI_D(&UBA(HW_UART3)->UART2_LSR_REG,
                                        REG_POS(UART2, UART2_LSR_REG, UART_TEMT)));
                }

                SENIS_label(pt_hw_uart_cfg_done);
        }

        SENIS_rdcbi(da(&SNC_ARG(nofifo_used)), 0);
        SENIS_cobr_eq(l(pt_wait_tx_empty)); {
                SENIS_assign(l(pt_wait_tx_one_begin), da(&SNC_ARG(wait_delay_senis_cmd)));
                SENIS_goto(l(pt_wait_tx_one_cond));
                SENIS_label(pt_wait_tx_one_begin);
                senis_del(b_ctx, SNC_PLACE_HOLDER);

                SENIS_label(pt_wait_tx_one_cond);

                SENIS_label(ph_ucode_cfg_UART2_TFL_REG);
                senis_rdcgr(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)),
                        _SNC_OP_DIRECT_ADDRESS(da(&snc_const[0])));
                SENIS_cobr_gr(l(pt_wait_tx_one_begin));
        }

        SENIS_label(pt_wait_tx_empty);
        SENIS_label(ph_ucode_cfg_UART2_LSR_REG);
        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)),
                REG_POS(UART2, UART2_LSR_REG, UART_TEMT));
        SENIS_cobr_eq(l(pt_tx_is_empty));
        SENIS_label(ph_tx_check_delay);
        SENIS_goto(l(pt_wait_tx_empty));
        SENIS_label(pt_tx_is_empty);
}

//==================== Control functions =======================================

void snc_uart_abort_read(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);

        // Reset RX FIFO
        SENIS_assign(da(&UBA(conf->id)->UART2_SRR_REG),
                1 << REG_POS(UART2, UART2_SRR_REG, UART_RFR));
}

void snc_uart_set_flow_on(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf->id != HW_UART1);

        // Restore HW flow control
        SENIS_wadva(da(&UBA(conf->id)->UART2_MCR_REG),
                (1 << REG_POS(UART2, UART2_MCR_REG, UART_AFCE)) |
                (1 << REG_POS(UART2, UART2_MCR_REG, UART_RTS)));
}

void snc_uart_set_flow_off(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE chars_avail_type, uint32_t* chars_avail, bool forced)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf->id != HW_UART1);
        ASSERT_WARNING((chars_avail_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(chars_avail)));

        SENIS_labels(pt_rx_fifo_is_not_empty, pt_return);

        /* Stop flow, it should tell host to stop sending data */
        SENIS_wadva(da(&UBA(conf->id)->UART2_MCR_REG),
                (1 << REG_POS(UART2, UART2_MCR_REG, UART_AFCE)) |
                (0 << REG_POS(UART2, UART2_MCR_REG, UART_RTS)));

        /*
         * Wait for at least 1 character duration to ensure host has not started a transmission
         * at the same time
         */
        SENIS_del(_SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1) + 1));

        /* Check if data has been received during wait time */
        SNC_hw_uart_is_rx_fifo_not_empty(conf->id);
        SENIS_cobr_eq(l(pt_rx_fifo_is_not_empty));
        senis_assign(b_ctx, chars_avail_type, chars_avail, _SNC_OP(0));
        SENIS_goto(l(pt_return));
        SENIS_label(pt_rx_fifo_is_not_empty); {
                senis_assign(b_ctx, chars_avail_type, chars_avail, _SNC_OP(1));
                if (!forced) {
                        SENIS_wadva(da(&UBA(conf->id)->UART2_MCR_REG),
                                (1 << REG_POS(UART2, UART2_MCR_REG, UART_AFCE)) |
                                (1 << REG_POS(UART2, UART2_MCR_REG, UART_RTS)));
                }
        }

        SENIS_label(pt_return);
}

//==================== Data Read/Write functions ===============================

SNC_FUNC_DECL(snc_uart_write_ucode, HW_UART_ID id, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t tx_tr_lvl_del_cmd, uint32_t tx_tr_lvl_del_rep, uint32_t tx_wait_del_cmd);

void snc_uart_write(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
        SENIS_OPER_TYPE out_len_type, uint32_t* out_len)
{
        const uart_config_ex* uart_hw_cfg = &conf->drv->hw_conf;
        uint32_t tx_tr_lvl_del;
        uint32_t tx_tr_lvl_del_rep;
        uint32_t tx_wait_del;
        uint32_t** applied_out_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(out_buf);
        ASSERT_WARNING((out_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(out_buf)));

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));

        if (out_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_out_buf[0]), out_buf);
                applied_out_buf_ind = &temp_out_buf[0];
        } else {
                SNC_ASSERT(da(out_buf));
                applied_out_buf_ind = (uint32_t**)out_buf;
        }

        // Calculate transaction timing parameters
        if (uart_hw_cfg->use_fifo) {
                switch (uart_hw_cfg->tx_fifo_tr_lvl) {
                case SNC_HW_UART_TX_FIFO_TR_LVL_EMPTY:
                        tx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE;
                        break;
                case SNC_HW_UART_TX_FIFO_TR_LVL_2_CHARS:
                        tx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE - 2;
                        break;
                case SNC_HW_UART_TX_FIFO_TR_LVL_0_25_FULL:
                        tx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE * 3 / 4;
                        break;
                case SNC_HW_UART_TX_FIFO_TR_LVL_0_5_FULL:
                        tx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE * 1 / 2;
                        break;
                }

                tx_tr_lvl_del = snc_hw_uart_calc_transfer_time(conf, tx_tr_lvl_del_rep);
                tx_wait_del = tx_tr_lvl_del / tx_tr_lvl_del_rep;
                if ((tx_tr_lvl_del / tx_tr_lvl_del_rep) > 0 && tx_tr_lvl_del > 255) {
                        tx_tr_lvl_del = _SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1));
                } else {
                        tx_tr_lvl_del_rep = (tx_tr_lvl_del > 0);
                        tx_tr_lvl_del = _SENIS_B_DEL(tx_tr_lvl_del);
                }
        } else {
                tx_tr_lvl_del_rep = -1;
                tx_tr_lvl_del = _SENIS_B_NOP();
        }
        tx_wait_del = _SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1));

        senis_call(b_ctx, SNC_UCODE_CTX(snc_uart_write_ucode), 2 * 6,
                        _SNC_OP(conf->id), _SNC_OP(applied_out_buf_ind),
                        out_len_type, out_len, _SNC_OP(tx_tr_lvl_del), _SNC_OP(tx_tr_lvl_del_rep),
                        _SNC_OP(tx_wait_del));

        _SNC_TMP_RMV(temp_out_buf);
}

SNC_FUNC_DEF(snc_uart_write_ucode, HW_UART_ID id, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t tx_tr_lvl_del_cmd, uint32_t tx_tr_lvl_del_rep, uint32_t tx_wait_del_cmd)
{
        SENIS_labels(
                pt_hw_uart2_cfg, pt_hw_uart3_cfg,
                ph_ucode_cfg_UART2_LSR_REG,
                ph_ucode_cfg_UART2_USR_REG,
                ph_ucode_cfg_UART2_TFL_REG,
                ph_ucode_cfg_UART2_RBR_THR_DLL_REG_1,
                ph_ucode_cfg_UART2_RBR_THR_DLL_REG_2,
                pt_hw_uart_cfg_done,

                pt_tx_fifo,
                pt_check_tx_chars_remaining,
                pt_while_tx_multi,
                pt_check_tx_fifo_status,
                pt_wait_tx_tr_lvl, ph_tx_tr_lvl_delay,
                pt_while_tx_step_begin, pt_while_tx_step_cond,
                ph_tx_wait_delay,
                pt_while_tx_one_begin, pt_while_tx_one_cond,
                ph_wait_tfnf_thre_cond,
                pt_write_char,
                pt_return
        );

        // uCode configuration based on the implied UART bus ID
        {
                // Check which UART bus ID is implied
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART1 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart2_cfg));
                { // HW_UART1 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART1)->UART2_LSR_REG,
                                                REG_POS(UART2, UART2_LSR_REG, UART_THRE)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_USR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART1)->UART2_USR_REG,
                                                REG_POS(UART2, UART2_USR_REG, UART_TFNF)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(tx_cnt_step_compl,
                                        &UBA(HW_UART1)->UART2_TFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_1),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART1)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_2),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART1)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart2_cfg);
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART2 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart3_cfg));
                { // HW_UART2 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART2)->UART2_LSR_REG,
                                                REG_POS(UART2, UART2_LSR_REG, UART_THRE)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_USR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART2)->UART2_USR_REG,
                                                REG_POS(UART2, UART2_USR_REG, UART_TFNF)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(tx_cnt_step_compl,
                                        &UBA(HW_UART2)->UART2_TFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_1),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART2)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_2),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART2)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart3_cfg);
                { // HW_UART3 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART3)->UART2_LSR_REG,
                                                REG_POS(UART2, UART2_LSR_REG, UART_THRE)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_USR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADVA_D(l(ph_wait_tfnf_thre_cond),
                                        _SENIS_B_RDCBI_D(&UBA(HW_UART3)->UART2_USR_REG,
                                                REG_POS(UART2, UART2_USR_REG, UART_TFNF)))));
                        SENIS_assign(l(ph_ucode_cfg_UART2_TFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(tx_cnt_step_compl,
                                        &UBA(HW_UART3)->UART2_TFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_1),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART3)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_2),
                                _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(&UBA(HW_UART3)->UART2_RBR_THR_DLL_REG,
                                        &temp_out_buf[0])));
                }

                SENIS_label(pt_hw_uart_cfg_done);
        }

        _SNC_STATIC(uint32_t, const_0xFFFFFFFF, sizeof(uint32_t), 0xFFFFFFFF);
        _SNC_STATIC(uint32_t, const_uart_fifo_size_1compl, sizeof(uint32_t),
                SNC_HW_UART_FIFO_SIZE ^ 0xFFFFFFFF);
        _SNC_STATIC(uint32_t, const_uart_fifo_size_2compl, sizeof(uint32_t),
                (SNC_HW_UART_FIFO_SIZE ^ 0xFFFFFFFF) + 1);

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, tx_cnt_compl, sizeof(uint32_t));

        SENIS_assign(da(&temp_out_buf[0]), ia(&SNC_ARG(out_buf_ind)));

        // Use the 1's-compl. value representation of the TX counter (for efficiency)
        SENIS_wadad(da(tx_cnt_compl), da(&SNC_ARG(out_len)));
        SENIS_tobre(da(tx_cnt_compl), 0xFFFFFFFF);

        // Check if the TX FIFO is enabled, and configure the uCode accordingly
        SENIS_rdcgr(da(const_0xFFFFFFFF), da(&SNC_ARG(tx_tr_lvl_del_rep)));
        SENIS_cobr_gr(l(pt_tx_fifo));
        { // The TX FIFO is not enabled
                // Set the SeNIS command condition for checking THR-empty bit
                // while writing one character at a time to the Transmit Holding Register (THR)
                SENIS_label(ph_ucode_cfg_UART2_LSR_REG);
                senis_wadva(b_ctx, _SNC_OP(l(ph_wait_tfnf_thre_cond)), SNC_PLACE_HOLDER);

                // Branch to "1-character-transfer" mode case
                SENIS_goto(l(pt_check_tx_chars_remaining));
        }

        // The TX FIFO is enabled
        SENIS_label(pt_tx_fifo);

        // Set the SeNIS command condition for checking TX-FIFO-not-Full bit
        // while writing one character at a time to the TX FIFO
        SENIS_label(ph_ucode_cfg_UART2_USR_REG);
        senis_wadva(b_ctx, _SNC_OP(l(ph_wait_tfnf_thre_cond)), SNC_PLACE_HOLDER);

        // If the number of the remaining characters is less than the TX FIFO size,
        // branch to "1-character-transfer" mode case
        SENIS_rdcgr(da(tx_cnt_compl), da(const_uart_fifo_size_1compl));
        SENIS_cobr_gr(l(pt_check_tx_chars_remaining));

        // "Multiple-characters-transfer" mode case //

        // Set the delay between successive steps of writing multiple characters to the TX FIFO
        SENIS_assign(l(ph_tx_tr_lvl_delay), da(&SNC_ARG(tx_tr_lvl_del_cmd)));

        // Write multiple characters to the TX FIFO while the number of the remaining characters
        // is more than or equal to the TX FIFO size (no "trigger-level" delay is added the
        // first time, because it is assumed that the TX FIFO is not full)
        SENIS_goto(l(pt_check_tx_fifo_status));
        SENIS_label(pt_while_tx_multi); {
                _SNC_TMP_ADD(uint32_t, tr_lvl_del_rep_cnt, sizeof(uint32_t));

                SENIS_assign(da(tr_lvl_del_rep_cnt), 1);

                // Perform a delay proportional to the given TX trigger level
                SENIS_label(pt_wait_tx_tr_lvl);
                SENIS_rdcgr(da(tr_lvl_del_rep_cnt), da(&SNC_ARG(tx_tr_lvl_del_rep)));
                SENIS_cobr_gr(l(pt_check_tx_fifo_status)); {
                        SENIS_label(ph_tx_tr_lvl_delay);
                        senis_del(b_ctx, SNC_PLACE_HOLDER);

                        SENIS_inc1(da(tr_lvl_del_rep_cnt));
                        SENIS_goto(l(pt_wait_tx_tr_lvl));
                }

                _SNC_TMP_RMV(tr_lvl_del_rep_cnt);

                _SNC_TMP_ADD(uint32_t, tx_cnt_step_compl, sizeof(uint32_t));

                // Get the number of characters that can be written to the TX FIFO
                // (use 2's complement for efficiency)
                SENIS_label(pt_check_tx_fifo_status); {
                        SENIS_label(ph_ucode_cfg_UART2_TFL_REG);
                        senis_wadad(b_ctx, _SNC_OP(da(tx_cnt_step_compl)), _SNC_OP(da(SNC_PLACE_HOLDER)));
                        SENIS_tobre(da(tx_cnt_step_compl), (SNC_HW_UART_FIFO_SIZE - 1) ^ 0xFFFFFFFF);
                }

                // If the TX FIFO is full wait for a TX trigger level delay
                SENIS_rdcgr(da(const_uart_fifo_size_2compl), da(tx_cnt_step_compl));
                SENIS_cobr_gr(l(pt_while_tx_multi));

                // Write characters to the TX FIFO
                SENIS_goto(l(pt_while_tx_step_cond));
                SENIS_label(pt_while_tx_step_begin); {
                        SENIS_label(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_1);
                        senis_wadad(b_ctx, _SNC_OP(da(SNC_PLACE_HOLDER)), _SNC_OP(ia(&temp_out_buf[0])));
                        SENIS_inc4(da(&temp_out_buf[0]));
                        SENIS_inc1(da(tx_cnt_step_compl));
                        SENIS_inc1(da(tx_cnt_compl));

                        SENIS_label(pt_while_tx_step_cond);
                        SENIS_rdcbi(da(tx_cnt_step_compl), 31);
                        SENIS_cobr_eq(l(pt_while_tx_step_begin));
                }

                _SNC_TMP_RMV(tx_cnt_step_compl);

                // Check if the number of the remaining characters to write is still more than or
                // equal to the TX FIFO size in order to continue writing multiple characters at
                // a time
                SENIS_rdcgr(da(const_uart_fifo_size_2compl), da(tx_cnt_compl));
                SENIS_cobr_gr(l(pt_while_tx_multi));
        }

        // Check if there are characters remaining to be written. If not, return.
        SENIS_label(pt_check_tx_chars_remaining); {
                SENIS_inc1(da(tx_cnt_compl)); // Use 2's complement value for the counter (for efficiency)

                SENIS_rdcgr(da(&snc_const[1]), da(tx_cnt_compl));
                SENIS_cobr_gr(l(pt_return));
        }

        // "One-character-transfer" mode case //

        // Set the delay for checking TX-FIFO-not-Full/THR-empty bit
        // while writing one character at a time
        SENIS_assign(l(ph_tx_wait_delay), da(&SNC_ARG(tx_wait_del_cmd)));

        // Write one character at a time while there are still characters to write
        SENIS_goto(l(pt_while_tx_one_cond));
        SENIS_label(pt_while_tx_one_begin); {
                // Wait TX-FIFO-not-Full/THR-empty bit to be set
                SENIS_label(ph_wait_tfnf_thre_cond);
                SENIS_rdcbi(da(SNC_PLACE_HOLDER), SNC_PLACE_HOLDER);
                SENIS_cobr_eq(l(pt_write_char)); {
                        SENIS_label(ph_tx_wait_delay);
                        senis_del(b_ctx, SNC_PLACE_HOLDER);

                        SENIS_goto(l(pt_while_tx_one_begin));
                }
                // Write a character
                SENIS_label(pt_write_char); {
                        SENIS_label(ph_ucode_cfg_UART2_RBR_THR_DLL_REG_2);
                        senis_wadad(b_ctx, _SNC_OP(da(SNC_PLACE_HOLDER)), _SNC_OP(ia(&temp_out_buf[0])));
                        SENIS_inc4(da(&temp_out_buf[0]));
                        SENIS_inc1(da(tx_cnt_compl));
                }

                // Check if there are characters remaining to write, so that a delay can be added
                // between successive one-character TX transfers
                SENIS_label(pt_while_tx_one_cond);

                SENIS_rdcgr_z(da(tx_cnt_compl));
                SENIS_cobr_gr(l(pt_while_tx_one_begin));
        }

        SENIS_label(pt_return);

        SENIS_assign(ia(&SNC_ARG(out_buf_ind)), da(&temp_out_buf[0]));

        _SNC_TMP_RMV(tx_cnt_compl);
        _SNC_TMP_RMV(temp_out_buf);
}

SNC_FUNC_DECL(snc_uart_read_check_error_ucode, HW_UART_ID id, uint32_t** in_buf_ind,
        uint32_t in_len, uint32_t wait_time, uint32_t* rcv_len, uint32_t* error,
        uint32_t** error_char, uint32_t rx_tr_lvl_del_cmd, uint32_t rx_tr_lvl_del_rep,
        uint32_t rx_wait_del_cmd, uint32_t rx_timeout_del_cmd, uint32_t rx_timeout_del_rep);

void snc_uart_read(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
        SENIS_OPER_TYPE wait_time_type, uint32_t* wait_time,
        SENIS_OPER_TYPE rcv_len_type, uint32_t* rcv_len)
{
        snc_uart_read_check_error(b_ctx, conf, in_buf_type, in_buf, in_len_type, in_len,
                wait_time_type, wait_time, rcv_len_type, rcv_len,
                _SNC_OP(da(NULL)), _SNC_OP(da(NULL)));
}

void snc_uart_read_check_error(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
        SENIS_OPER_TYPE wait_time_type, uint32_t* wait_time,
        SENIS_OPER_TYPE rcv_len_type, uint32_t* rcv_len,
        SENIS_OPER_TYPE error_type, uint32_t* error,
        SENIS_OPER_TYPE error_char_type, uint32_t* error_char)
{
        const uart_config_ex* uart_hw_cfg = &conf->drv->hw_conf;
        uint32_t rx_tr_lvl_del;
        uint32_t rx_tr_lvl_del_rep;
        uint32_t rx_wait_del;
        uint32_t rx_timeout_del;
        uint32_t rx_timeout_del_rep;
        uint32_t** applied_in_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(in_buf);
        ASSERT_WARNING((in_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(in_buf)));
        ASSERT_WARNING((rcv_len_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(rcv_len)));
        ASSERT_WARNING((error_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(error)));
        ASSERT_WARNING((error_char_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(error_char)));

        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        if (in_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_in_buf[0]), in_buf);
                applied_in_buf_ind = &temp_in_buf[0];
        } else {
                SNC_ASSERT(da(in_buf));
                applied_in_buf_ind = (uint32_t**)in_buf;
        }

        if (rcv_len_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                SNC_ASSERT(da(rcv_len));
        }
        if (error_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                SNC_ASSERT(da(error));
        }
        if (error_char_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                SNC_ASSERT(da(error_char));
        }

        // Calculate transaction timing parameters
        if (uart_hw_cfg->use_fifo) {
                switch (uart_hw_cfg->rx_fifo_tr_lvl) {
                case SNC_HW_UART_RX_FIFO_TR_LVL_1_CHAR:
                        rx_tr_lvl_del_rep = 1;
                        break;
                case SNC_HW_UART_RX_FIFO_TR_LVL_0_25_FULL:
                        rx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE * 1 / 4;
                        break;
                case SNC_HW_UART_RX_FIFO_TR_LVL_0_5_FULL:
                        rx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE * 1 / 2;
                        break;
                case SNC_HW_UART_RX_FIFO_TR_LVL_2_CHARS_LT_FULL:
                        rx_tr_lvl_del_rep = SNC_HW_UART_FIFO_SIZE - 2;
                        break;
                }

                rx_tr_lvl_del = snc_hw_uart_calc_transfer_time(conf, rx_tr_lvl_del_rep);
                if ((rx_tr_lvl_del / rx_tr_lvl_del_rep) > 0 && rx_tr_lvl_del > 255) {
                        rx_tr_lvl_del = _SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1));
                } else {
                        rx_tr_lvl_del_rep = (rx_tr_lvl_del > 0);
                        rx_tr_lvl_del = _SENIS_B_DEL(rx_tr_lvl_del);
                }
        } else {
                rx_tr_lvl_del_rep = -1;
                rx_tr_lvl_del = _SENIS_B_NOP();
        }
        rx_wait_del = _SENIS_B_DEL(snc_hw_uart_calc_transfer_time(conf, 1));
        rx_timeout_del_rep = 1;
        uint32_t i = 1;
        do {
                rx_timeout_del = snc_hw_uart_calc_transfer_time(conf, i);
                if (rx_timeout_del > 0) {
                        rx_timeout_del_rep = SNC_HW_UART_CHAR_TIMEOUT / i;
                        break;
                }
                i *= 2;
        } while (i <= SNC_HW_UART_CHAR_TIMEOUT);
        if (rx_timeout_del == 0) {
                rx_timeout_del_rep++;
        }
        rx_timeout_del = _SENIS_B_DEL(rx_timeout_del);

        senis_call(b_ctx, SNC_UCODE_CTX(snc_uart_read_check_error_ucode), 2 * 12,
                        _SNC_OP(conf->id), _SNC_OP(applied_in_buf_ind),
                        in_len_type, in_len, wait_time_type, wait_time,
                        rcv_len_type + 1, rcv_len, error_type + 1, error,
                        error_char_type + 1, error_char,
                        _SNC_OP(rx_tr_lvl_del), _SNC_OP(rx_tr_lvl_del_rep),
                        _SNC_OP(rx_wait_del), _SNC_OP(rx_timeout_del), _SNC_OP(rx_timeout_del_rep));

        _SNC_TMP_RMV(temp_in_buf);
}

SNC_FUNC_DEF(snc_uart_read_check_error_ucode, HW_UART_ID id, uint32_t** in_buf_ind,
        uint32_t in_len, uint32_t wait_time, uint32_t* rcv_len, uint32_t* error,
        uint32_t** error_char, uint32_t rx_tr_lvl_del_cmd, uint32_t rx_tr_lvl_del_rep,
        uint32_t rx_wait_del_cmd, uint32_t rx_timeout_del_cmd, uint32_t rx_timeout_del_rep)
{
        SENIS_labels(
                pt_hw_uart2_cfg, pt_hw_uart3_cfg,
                ph_ucode_cfg_UART2_RFL_REG,
                ph_ucode_cfg_UART2_LSR_REG,
                ph_ucode_cfg_UART2_RBR_THR_DLL_REG,
                pt_hw_uart_cfg_done,

                pt_while_rx_first_begin, pt_while_rx_first_end,
                pt_read_first_char, pt_check_char_reveived,
                pt_rx_fifo, pt_rx_nofifo,
                pt_check_rx_chars_remaining,
                pt_while_rx_multi, pt_check_rx_fifo_status, pt_check_rx_multi_timeout,
                pt_while_rx_multi_cond,
                pt_read_chars, pt_read_one_char, pt_one_char_read,
                pt_error_produced, pt_rx_error_is_set, pt_no_error_produced,
                pt_wait_rx_tr_lvl, ph_rx_fifo_step_delay,
                pt_while_rx,
                pt_rx_one, pt_while_rx_one_begin, pt_read_char, pt_check_rx_one_timeout,
                pt_while_rx_one_cond,
                ph_rx_char_transfer_del,
                pt_return,
                pt_update_error, pt_update_in_buf,
                pt_rx_timeout, ph_char_timeout_del, pt_while_rx_timeout_begin,
                pt_while_rx_timeout_cond
        );

        // uCode configuration based on the implied UART bus ID
        {
                // Check which UART bus ID is implied
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART1 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart2_cfg));
                { // HW_UART1 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_RFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rx_cnt_step_compl,
                                        &UBA(HW_UART1)->UART2_RFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rcv_error,
                                        &UBA(HW_UART1)->UART2_LSR_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_ID(&temp_in_buf[0],
                                        &UBA(HW_UART1)->UART2_RBR_THR_DLL_REG)));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart2_cfg);
                SENIS_rdcgr(da(&SNC_ARG(id)), da(&const_HW_UART[HW_UART2 == HW_UART2]));
                SENIS_cobr_gr(l(pt_hw_uart3_cfg));
                { // HW_UART2 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_RFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rx_cnt_step_compl,
                                        &UBA(HW_UART2)->UART2_RFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rcv_error,
                                        &UBA(HW_UART2)->UART2_LSR_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_ID(&temp_in_buf[0],
                                        &UBA(HW_UART2)->UART2_RBR_THR_DLL_REG)));
                }
                SENIS_goto(l(pt_hw_uart_cfg_done));
                SENIS_label(pt_hw_uart3_cfg);
                { // HW_UART3 is implied
                        SENIS_assign(l(ph_ucode_cfg_UART2_RFL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rx_cnt_step_compl,
                                        &UBA(HW_UART3)->UART2_RFL_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_LSR_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_DD(rcv_error,
                                        &UBA(HW_UART3)->UART2_LSR_REG)));
                        SENIS_assign(l(ph_ucode_cfg_UART2_RBR_THR_DLL_REG)+1,
                                _SENIS_2ND_OPERAND(_SENIS_B_WADAD_ID(&temp_in_buf[0],
                                        &UBA(HW_UART3)->UART2_RBR_THR_DLL_REG)));
                }

                SENIS_label(pt_hw_uart_cfg_done);
        }

        _SNC_STATIC(uint32_t, const_0xFFFFFFFF, sizeof(uint32_t), 0xFFFFFFFF);
        _SNC_STATIC(uint32_t, const_uart_fifo_size, sizeof(uint32_t), SNC_HW_UART_FIFO_SIZE);
        _SNC_STATIC(uint32_t, const_uart_fifo_size_2compl, sizeof(uint32_t),
                (SNC_HW_UART_FIFO_SIZE ^ 0xFFFFFFFF) + 1);

        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, rx_cnt_compl, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, rx_cnt_step_compl, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, rcv_cnt, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, rcv_error, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, char_timeout_cnt, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t*, p_while_rx_rtn, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, p_rx_timeout_rtn, sizeof(uint32_t*));

        // Initialize the counter for received characters
        SENIS_assign(da(rcv_cnt), 0);

        // Initialize error indication for the received characters
        SENIS_assign(da(rcv_error), 0);

        // If the number of characters to read is "0", return
        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(in_len)));
        SENIS_cobr_gr(l(pt_return));

        SENIS_assign(da(&temp_in_buf[0]), ia(&SNC_ARG(in_buf_ind)));

        // Use the 1's-compl. value representation of the RX counter (for efficiency)
        SENIS_assign(da(rx_cnt_compl), da(&SNC_ARG(in_len)));
        SENIS_tobre(da(rx_cnt_compl), 0xFFFFFFFF);

        // Wait for the first character to be received within the given wait-time period
        {
                _SNC_TMP_ADD(uint32_t, wait_time_cnt, sizeof(uint32_t));

                SENIS_assign(da(wait_time_cnt), 0);

                SENIS_goto(l(pt_read_first_char));
                SENIS_label(pt_while_rx_first_begin); {
                        // Increase by one the wait-time low power clock tick count
                        SENIS_inc1(da(wait_time_cnt));

                        // Perform a SeNIS delay of 1 low power clock tick
                        SENIS_del(1);

                        // Read a character (if any received)
                        SENIS_label(pt_read_first_char); {
                                SENIS_assign(da(rx_cnt_step_compl), 0);
                                SENIS_assign(da(p_while_rx_rtn),
                                        _SNC_OP_DIRECT_ADDRESS(l(pt_check_char_reveived)));
                                SENIS_goto(l(pt_while_rx));
                        }

                        // Check if a character has been received, and break the wait-loop, accordingly
                        SENIS_label(pt_check_char_reveived); {
                                SENIS_rdcgr_z(da(rcv_cnt));
                                SENIS_cobr_gr(l(pt_while_rx_first_end));
                        }

                        // Check if wait time has passed. If yes, return, otherwise continue the
                        // wait-loop
                        SENIS_rdcgr(da(&SNC_ARG(wait_time)), da(wait_time_cnt));
                        SENIS_cobr_gr(l(pt_while_rx_first_begin));
                        SENIS_goto(l(pt_return));
                }
                SENIS_label(pt_while_rx_first_end);

                _SNC_TMP_RMV(wait_time_cnt);
        }

        // Check if the RX FIFO is enabled, and proceed to the appropriate transfer mode accordingly
        SENIS_rdcgr(da(const_0xFFFFFFFF), da(&SNC_ARG(rx_tr_lvl_del_rep)));
        SENIS_cobr_gr(l(pt_rx_fifo));
        { // The RX FIFO is not enabled
                // Branch to "1-character-transfer" mode case
                SENIS_goto(l(pt_check_rx_chars_remaining));
        }

        // The RX FIFO is enabled
        SENIS_label(pt_rx_fifo);

        // If the number of the remaining characters is less than the RX FIFO size,
        // branch to "1-character-transfer" mode case
        SENIS_rdcgr(da(const_uart_fifo_size), da(&SNC_ARG(in_len)));
        SENIS_cobr_gr(l(pt_check_rx_chars_remaining));

        // "Multiple-characters-transfer" mode case //

        // Set the delay between successive steps of reading characters from the RX FIFO
        SENIS_assign(l(ph_rx_fifo_step_delay), da(&SNC_ARG(rx_tr_lvl_del_cmd)));

        // Read multiple characters from the RX FIFO while the number of the remaining characters
        // is more than or equal to the RX FIFO size (no "trigger-level" delay is added the
        // first time, because the RX FIFO may be full)
        SENIS_goto(l(pt_check_rx_fifo_status));
        SENIS_label(pt_while_rx_multi); {
                _SNC_TMP_ADD(uint32_t, tr_lvl_del_abort, sizeof(uint32_t));
                _SNC_TMP_ADD(uint32_t, tr_lvl_del_rep_cnt, sizeof(uint32_t));

                // Check if the RX trigger level delay can be aborted
                SENIS_rdcbi(da(tr_lvl_del_abort), 0);
                SENIS_cobr_eq(l(pt_check_rx_fifo_status));

                SENIS_assign(da(tr_lvl_del_rep_cnt), 1);

                // Perform a delay proportional to the given RX trigger level
                SENIS_label(pt_wait_rx_tr_lvl);
                SENIS_rdcgr(da(tr_lvl_del_rep_cnt), da(&SNC_ARG(rx_tr_lvl_del_rep)));
                SENIS_cobr_gr(l(pt_check_rx_fifo_status)); {
                        SENIS_label(ph_rx_fifo_step_delay);
                        senis_del(b_ctx, SNC_PLACE_HOLDER);

                        SENIS_inc1(da(tr_lvl_del_rep_cnt));
                        SENIS_goto(l(pt_wait_rx_tr_lvl));
                }

                _SNC_TMP_RMV(tr_lvl_del_rep_cnt);

                // Get the number of characters that can be read from the RX FIFO
                // (use 2's complement for efficiency)
                SENIS_label(pt_check_rx_fifo_status); {
                        SENIS_label(ph_ucode_cfg_UART2_RFL_REG);
                        senis_wadad(b_ctx, _SNC_OP(da(rx_cnt_step_compl)), _SNC_OP(da(SNC_PLACE_HOLDER)));
                        SENIS_tobre(da(rx_cnt_step_compl), 0xFFFFFFFF);
                        SENIS_inc1(da(rx_cnt_step_compl));
                }

                // Read characters from the RX FIFO
                SENIS_assign(da(p_while_rx_rtn), _SNC_OP_DIRECT_ADDRESS(l(pt_check_rx_multi_timeout)));
                SENIS_goto(l(pt_while_rx));

                // Set "abort-RX-trigger-level-delay" indication
                SENIS_assign(da(tr_lvl_del_abort), 1);

                // Check if there is a character timeout. If not, continue with reading the
                // the remaining characters, otherwise return.
                SENIS_label(pt_check_rx_multi_timeout); {
                        SENIS_assign(da(p_rx_timeout_rtn),
                                _SNC_OP_DIRECT_ADDRESS(l(pt_while_rx_multi_cond)));
                        SENIS_rdcgr_z(da(char_timeout_cnt));
                        SENIS_cobr_gr(l(pt_rx_timeout));
                }

                // Clear "abort-trigger-level-delay" indication if character timeout detection
                // initiation has been enabled
                SENIS_assign(da(tr_lvl_del_abort), 0);

                // Check if the number of the remaining characters to read is still more than or
                // equal to the RX FIFO size in order to continue reading multiple characters at
                // a time
                SENIS_label(pt_while_rx_multi_cond);

                SENIS_rdcgr(da(const_uart_fifo_size_2compl), da(rx_cnt_compl));
                SENIS_cobr_gr(l(pt_while_rx_multi));

                _SNC_TMP_RMV(tr_lvl_del_abort);
        }

        // Check if there are characters remaining to be read. If not, return.
        SENIS_label(pt_check_rx_chars_remaining); {
                SENIS_rdcgr(da(&SNC_ARG(in_len)), da(rcv_cnt));
                SENIS_cobr_gr(l(pt_rx_one));
                SENIS_goto(l(pt_return));
        }

        // "One-character-transfer" mode case //

        SENIS_label(pt_rx_one); {
                // Set the delay for checking data-ready bit while reading one character at a time
                // from the RX FIFO
                SENIS_assign(l(ph_rx_char_transfer_del), da(&SNC_ARG(rx_wait_del_cmd)));

                // Read one character at a time while there are still characters to read
                SENIS_goto(l(pt_read_char));
                SENIS_label(pt_while_rx_one_begin); {
                        // Perform a delay proportional to the transfer time of a character
                        SENIS_label(ph_rx_char_transfer_del);
                        senis_del(b_ctx, SNC_PLACE_HOLDER);

                        // Read a character (if any received)
                        SENIS_label(pt_read_char); {
                                SENIS_assign(da(rx_cnt_step_compl), 0);
                                SENIS_assign(da(p_while_rx_rtn),
                                        _SNC_OP_DIRECT_ADDRESS(l(pt_check_rx_one_timeout)));
                                SENIS_goto(l(pt_while_rx));
                        }

                        // Check if there is a character timeout. If not, continue with reading the
                        // the remaining characters, otherwise return.
                        SENIS_label(pt_check_rx_one_timeout); {
                                SENIS_assign(da(p_rx_timeout_rtn),
                                        _SNC_OP_DIRECT_ADDRESS(l(pt_while_rx_one_cond)));
                                SENIS_rdcgr_z(da(char_timeout_cnt));
                                SENIS_cobr_gr(l(pt_rx_timeout));
                        }

                        // Check if there are characters remaining to read, so that a delay can be
                        // added between successive one-character RX transfers
                        SENIS_label(pt_while_rx_one_cond);

                        SENIS_rdcgr(da(&SNC_ARG(in_len)), da(rcv_cnt));
                        SENIS_cobr_gr(l(pt_while_rx_one_begin));
                }
        }

        SENIS_label(pt_return);

        // Set the returned received characters number
        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(rcv_len)));
        SENIS_cobr_gr(l(pt_update_error)); {
                SENIS_assign(ia(&SNC_ARG(rcv_len)), da(rcv_cnt));
        }

        // Set the returned error value
        SENIS_label(pt_update_error);
        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(error)));
        SENIS_cobr_gr(l(pt_update_in_buf)); {
                SENIS_assign(ia(&SNC_ARG(error)), da(rcv_error));
        }

        SENIS_label(pt_update_in_buf);
        SENIS_assign(ia(&SNC_ARG(in_buf_ind)), da(&temp_in_buf[0]));

        SENIS_return;

        // ===="Local function"====

        // Read a number of characters from the UART as indicated by the 2's complement value of
        // rx_cnt_step_compl. If '0' number of characters is requested, then a character will be
        // read if "data-ready" bit is set.
        SENIS_label(pt_while_rx); {
                // Get the status of the transaction
                SENIS_label(ph_ucode_cfg_UART2_LSR_REG);
                senis_wadad(b_ctx, _SNC_OP(da(rcv_error)), _SNC_OP(da(SNC_PLACE_HOLDER)));

                // Check if the number of characters to be read is greater than '0'
                SENIS_rdcgr_z(da(rx_cnt_step_compl));
                SENIS_cobr_gr(l(pt_read_chars));
                { // '0' number of characters is requested
                        // Check if the receiver contains at least one character
                        SENIS_rdcbi(da(rcv_error), REG_POS(UART2, UART2_LSR_REG, UART_DR));
                        SENIS_cobr_eq(l(pt_read_one_char));
                        { // The receiver does not contain any characters
                                // If not already initiated (i.e. *char_timeout_cnt > 0), initiate
                                // character timeout detection ((i.e. *char_timeout_cnt = 1)
                                // and branch back from where the particular "local function" has
                                // been called
                                SENIS_rdcgr_z(da(char_timeout_cnt));
                                SENIS_cobr_gr(ia(p_while_rx_rtn));
                                SENIS_assign(da(char_timeout_cnt), 1);
                                SENIS_goto(ia(p_while_rx_rtn));
                        }

                        // The receiver contains at least one character
                        SENIS_label(pt_read_one_char);
                        SENIS_assign(da(rx_cnt_step_compl), (uint32_t)-1);
                }

                SENIS_label(pt_read_chars);
                { // The receiver contains at least one character
                        // Clear character timeout indication (i.e. *char_timeout_cnt = 0)
                        SENIS_assign(da(char_timeout_cnt), 0);

                        // Read a character
                        SENIS_label(ph_ucode_cfg_UART2_RBR_THR_DLL_REG);
                        senis_wadad(b_ctx, _SNC_OP(ia(&temp_in_buf[0])), _SNC_OP(da(SNC_PLACE_HOLDER)));

                        // Increase the counters accordingly
                        SENIS_inc1(da(rcv_cnt));
                        SENIS_inc1(da(rx_cnt_compl));

                        // Check if error-checking for the received characters is enabled
                        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(error)));
                        SENIS_cobr_gr(l(pt_one_char_read)); {
                                // Check if an error has been produced
                                SENIS_rdcbi(da(rcv_error), REG_POS(UART2, UART2_LSR_REG, UART_OE));
                                SENIS_cobr_eq(l(pt_error_produced));
                                SENIS_rdcbi(da(rcv_error), REG_POS(UART2, UART2_LSR_REG, UART_PE));
                                SENIS_cobr_eq(l(pt_error_produced));
                                SENIS_rdcbi(da(rcv_error), REG_POS(UART2, UART2_LSR_REG, UART_FE));
                                SENIS_cobr_eq(l(pt_error_produced));
                                SENIS_rdcbi(da(rcv_error), REG_POS(UART2, UART2_LSR_REG, UART_BI));
                                SENIS_cobr_eq(l(pt_error_produced));
                                SENIS_goto(l(pt_no_error_produced));
                                SENIS_label(pt_error_produced);
                                { // An error has been produced
                                        _SNC_TMP_ADD(uint32_t*, rcv_err_char_addr, sizeof(uint32_t*));

                                        // Set "RX-error" indication
                                        SENIS_rdcbi(da(rcv_error), SNC_HW_UART_ERROR_RX);
                                        SENIS_cobr_eq(l(pt_rx_error_is_set)); {
                                                SENIS_tobre(da(rcv_error), 1 << SNC_HW_UART_ERROR_RX);
                                        }
                                        SENIS_label(pt_rx_error_is_set);

                                        // Get the address where it is stored the character which
                                        // produced the error
                                        SENIS_assign(da(rcv_err_char_addr), da(&temp_in_buf[0]));

                                        // Update the pointer to the input buffer, so that it can
                                        // point to the next 32bit word aligned character
                                        SENIS_inc4(da(&temp_in_buf[0]));

                                        // Set the returned address where it is stored the character
                                        // which produced the error
                                        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(error_char)));
                                        SENIS_cobr_gr(l(pt_return));
                                        SENIS_assign(ia(&SNC_ARG(error_char)), da(rcv_err_char_addr));

                                        _SNC_TMP_RMV(rcv_err_char_addr);

                                        SENIS_goto(l(pt_return));
                                }

                                // No error has been produced
                                SENIS_label(pt_no_error_produced);
                                SENIS_assign(da(rcv_error), 0);
                        }

                        SENIS_label(pt_one_char_read);

                        // Update the counter of the requested characters, as well as the pointer
                        // to the input buffer, so that it can point to the next 32bit word
                        // aligned character
                        SENIS_inc4(da(&temp_in_buf[0]));
                        SENIS_inc1(da(rx_cnt_step_compl));
                }

                // Check if all requested characters have been read, and break the loop accordingly
                SENIS_rdcgr_z(da(rx_cnt_step_compl));
                SENIS_cobr_gr(l(pt_while_rx));

                // Branch back from where the particular "local function" has been called
                SENIS_goto(ia(p_while_rx_rtn));
        }

        // ========================

        // ===="Local function"====

        SENIS_label(pt_rx_timeout); {
                SENIS_label(pt_while_rx_timeout_begin); {
                        // Set the delay required for identifying character timeout
                        SENIS_assign(l(ph_char_timeout_del), da(&SNC_ARG(rx_timeout_del_cmd)));

                        // Perform a delay proportional to the transfer time of a character
                        SENIS_label(ph_char_timeout_del);
                        senis_del(b_ctx, SNC_PLACE_HOLDER);

                        // Increase by 1 low power clock tick the delay, so that it can be
                        // greater than the duration of a character transfer
                        SENIS_inc1(l(ph_char_timeout_del));

                        // Read a character (if any received)
                        {
                                SENIS_assign(da(rx_cnt_step_compl), 0);
                                SENIS_assign(da(p_while_rx_rtn),
                                        _SNC_OP_DIRECT_ADDRESS(l(pt_while_rx_timeout_cond)));
                                SENIS_goto(l(pt_while_rx));
                        }

                        // Check if a character has been received. If yes, branch back from where
                        // the particular "local function" has been called.
                        SENIS_label(pt_while_rx_timeout_cond);
                        SENIS_rdcgr(da(&snc_const[1]), da(char_timeout_cnt));
                        SENIS_cobr_gr(ia(p_rx_timeout_rtn));

                        // Check if a character timeout has been detected. If yes, return.
                        SENIS_inc1(da(char_timeout_cnt));
                        SENIS_rdcgr(da(char_timeout_cnt), da(&SNC_ARG(rx_timeout_del_rep)));
                        SENIS_cobr_gr(l(pt_return));

                        SENIS_goto(l(pt_while_rx_timeout_begin));
                }
        }

        // ========================

        _SNC_TMP_RMV(p_rx_timeout_rtn);
        _SNC_TMP_RMV(p_while_rx_rtn);
        _SNC_TMP_RMV(char_timeout_cnt);
        _SNC_TMP_RMV(rcv_error);
        _SNC_TMP_RMV(rcv_cnt);
        _SNC_TMP_RMV(rx_cnt_step_compl);
        _SNC_TMP_RMV(rx_cnt_compl);
        _SNC_TMP_RMV(temp_in_buf);
}

#endif /* dg_configUSE_SNC_HW_UART */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
