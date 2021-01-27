/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_I2C
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_i2c.c
 *
 * @brief SNC-Implementation of I2C Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_I2C

#include "snc_defs.h"
#include "snc_hw_sys.h"
#include "snc_hw_gpio.h"

#include "snc_hw_i2c.h"

#define SNC_I2C_DECONF_PROC(i2c_io_conf_ptr)                                                    \
        _SENIS_B_WADVA_D((&GPIO->P0_RESET_DATA_REG +                                            \
                i2c_io_conf_ptr->scl.port - 2*((uint8_t)i2c_io_conf_ptr->scl.off.high)),        \
                ((1 << i2c_io_conf_ptr->scl.pin))),                                             \
        _SENIS_B_WADVA_D((&GPIO->P0_00_MODE_REG +                                               \
                (&GPIO->P1_00_MODE_REG - &GPIO->P0_00_MODE_REG)*i2c_io_conf_ptr->scl.port +     \
                i2c_io_conf_ptr->scl.pin),                                                      \
                (i2c_io_conf_ptr->scl.off.function | i2c_io_conf_ptr->scl.off.mode)),           \
        _SENIS_B_WADVA_D((&GPIO->P0_RESET_DATA_REG +                                            \
                i2c_io_conf_ptr->sda.port - 2*((uint8_t)i2c_io_conf_ptr->sda.off.high)),        \
                ((1 << i2c_io_conf_ptr->sda.pin))),                                             \
        _SENIS_B_WADVA_D((&GPIO->P0_00_MODE_REG +                                               \
                (&GPIO->P1_00_MODE_REG - &GPIO->P0_00_MODE_REG)*i2c_io_conf_ptr->sda.port +     \
                i2c_io_conf_ptr->sda.pin),                                                      \
                (i2c_io_conf_ptr->sda.off.function | i2c_io_conf_ptr->sda.off.mode))

/*
 * ENUMERATION, DATA STRUCTURE AND DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/*
 * I2C access flags bit positions
 */
typedef enum {
        SNC_HW_I2C_FLAG_ADD_STOP_BIT_POS,       /* Add stop condition after write */
        SNC_HW_I2C_FLAG_ADD_RESTART_BIT_POS,    /* Add restart condition after write and before read */
} SNC_HW_I2C_ACCESS_FLAGS_BIT_POS;

/*
 * I2C pad latches operation type
 */
typedef enum {
        SNC_I2C_PAD_LATCHES_OP_ENABLE,          /* Enable pad latches */
        SNC_I2C_PAD_LATCHES_OP_DISABLE          /* Disable pad latches */
} SNC_I2C_PAD_LATCHES_OP;

/*
 * Internal SNC I2C driver configuration
 */
typedef struct {
        /* I2C connection parameter settings, i.e. bus speed, mode of operation and addressing mode,
         * merged to 32bit word value to be applied to I2C Control Register (I2C_CON_REG or I2C2_CON_REG)
         */
        uint32_t ctrl_params;
        uint32_t address;                       /* Target slave address in master mode or controller address in slave mode */
        uint32_t clk_hcnt_ss_hs;                /* I2C clock (SCL) standard/high speed high count */
        uint32_t clk_lcnt_ss_hs;                /* I2C clock (SCL) standard/high speed low count */
        uint32_t clk_hcnt_fs;                   /* I2C clock (SCL) fast speed high count */
        uint32_t clk_lcnt_fs;                   /* I2C clock (SCL) fast speed low count */
} snc_i2c_config_t;

/*
 * Compact SNC I2C I/O configuration
 */
typedef union {
        struct {
                uint32_t func   : GPIO_P0_00_MODE_REG_PUPD_Pos;
                uint32_t mode   : GPIO_P0_00_MODE_REG_PPOD_Pos - GPIO_P0_00_MODE_REG_PUPD_Pos + 1;
                uint32_t high   : 1;
                uint32_t pin    : 5;
                uint32_t port   : 5;
        };
        uint32_t io_conf;
} snc_i2c_io_conf_compact_t;

typedef struct {
        snc_i2c_io_conf_compact_t scl_pad_off;
        snc_i2c_io_conf_compact_t sda_pad_off;

        struct {
                struct {
                        uint32_t last_scl_high_off_assign[2];
                        uint32_t last_scl_func_mode_off_assign[2];
                        uint32_t last_sda_high_off_assign[2];
                        uint32_t last_sda_func_mode_off_assign[2];
                } deconf_proc;
                uint32_t senis_goto[2];
        } deconf_ucode;
} last_io_t;

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_hw_i2c_set_src_clk_config(b_ctx_t* b_ctx, HW_I2C_ID id, uint32_t clk_en,
        uint32_t clk_sel);
/**
 * \brief Function used in SNC context to configure I2C source clock
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance to be configured (HW_I2C1 or HW_I2C2)
 * \param [in] clk_en   (uint32_t: build-time-only value)
 *                      enables or disables the I2C clock source (1-Enable, 0-Disable)
 * \param [in] clk_sel  (uint32_t: build-time-only value)
 *                      selects the clock source (0-Div1, 1-DivN)
 */
#define SNC_hw_i2c_set_src_clk_config(id, clk_en, clk_sel)                                      \
        snc_hw_i2c_set_src_clk_config(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id),                      \
                                             _SNC_OP_VALUE(uint32_t, clk_en),                   \
                                             _SNC_OP_VALUE(uint32_t, clk_sel))

static void snc_hw_i2c_enable(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to enable I2C controller
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance to be enabled (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_enable(id)                                                                   \
        snc_hw_i2c_enable(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_disable(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to disable I2C controller
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance to be disabled (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_disable(id)                                                                  \
        snc_hw_i2c_disable(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_is_master_busy(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to check if I2C controller is busy when operating in
 *        master mode
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if I2C controller is busy,
 * therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_master_is_busy)) to decide on
 * an action.
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_is_master_busy(id)                                                           \
        snc_hw_i2c_is_master_busy(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_is_tx_fifo_empty(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to check if I2C controller TX FIFO is empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if I2C controller TX FIFO
 * is empty, therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_tx_fifo_is_empty))
 * to decide on an action.
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 *
 */
#define SNC_hw_i2c_is_tx_fifo_empty(id)                                                         \
        snc_hw_i2c_is_tx_fifo_empty(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_is_tx_fifo_not_full(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to check if I2C controller TX FIFO is NOT full
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if I2C controller TX FIFO
 * is NOT full, therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_tx_fifo_is_not_full))
 * to decide on an action.
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_is_tx_fifo_not_full(id)                                                      \
        snc_hw_i2c_is_tx_fifo_not_full(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_is_rx_fifo_not_empty(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to check if I2C controller RX FIFO is NOT empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if I2C controller RX FIFO
 * is NOT empty, therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_rx_fifo_is_not_empty))
 * to decide on an action.
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_is_rx_fifo_not_empty(id)                                                     \
        snc_hw_i2c_is_rx_fifo_not_empty(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_is_abort_status(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to check if TX_ABORT interrupt has been triggered
 *
 * When this function is called, GR_FLAG in SNC_STATUS REG of SNC is set if there is an I2C transfer
 * abort, therefore it can be followed by a SENIS_cobr_gr(l(gothere_if_there_is_an_abort_status))
 * to decide on an action.
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_is_abort_status(id)                                                          \
        snc_hw_i2c_is_abort_status(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_hw_i2c_reset_abort_source(b_ctx_t* b_ctx, HW_I2C_ID id);
/**
 * \brief Function used in SNC context to reset TX_ABORT interrupt
 *
 * \param [in] id       (HW_I2C_ID: build-time-only value)
 *                      I2C controller instance (HW_I2C1 or HW_I2C2)
 */
#define SNC_hw_i2c_reset_abort_source(id)                                                       \
        snc_hw_i2c_reset_abort_source(b_ctx, _SNC_OP_VALUE(HW_I2C_ID, id))

static void snc_i2c_set_last_pin_conf(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to reset and update last I2C bus pads
 * *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 */
#define SNC_i2c_set_last_pin_conf(conf)                                                         \
        snc_i2c_set_last_pin_conf(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

static void snc_i2c_configure_pins(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to configure I2C bus pads
 * *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 */
#define SNC_i2c_configure_pins(conf)                                                            \
        snc_i2c_configure_pins(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

static void snc_i2c_pad_latches(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SNC_I2C_PAD_LATCHES_OP pad_latches_op);
/**
 * \brief Function used in SNC context to configure I2C bus pad latches
 * *
 * \param [in] conf             (const snc_i2c_controller_conf_t*: build-time-only value)
 *                              I2C controller configuration
 * \param [in] pad_latches_op   (SNC_I2C_PAD_LATCHES_OP: build-time-only value)
 *                              I2C bus pad latches operation type
 */
#define SNC_i2c_pad_latches(conf, pad_latches_op)                                               \
        snc_i2c_pad_latches(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf),       \
                                   _SNC_OP_VALUE(SNC_I2C_PAD_LATCHES_OP, pad_latches_op))

//==================== Controller Acquisition functions ========================

void snc_i2c_open(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_I2C1 || conf->id == HW_I2C2);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        uint32_t perif_id = (conf->id == HW_I2C1) ? BSR_PERIPH_ID_I2C1 : BSR_PERIPH_ID_I2C2;

        // I2C controller instance acquisition
        SNC_hw_sys_bsr_acquire(perif_id);

        // I2C controller configuration
        SNC_i2c_reconfig(conf);

        // I2C bus pads initialization
        SNC_i2c_configure_pins(conf);

        // Enable I2C bus pad latches
        SNC_i2c_pad_latches(conf, SNC_I2C_PAD_LATCHES_OP_ENABLE);
}

void snc_i2c_close(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        uint32_t perif_id = (conf->id == HW_I2C1) ? BSR_PERIPH_ID_I2C1 : BSR_PERIPH_ID_I2C2;

        ASSERT_WARNING(b_ctx);

        // Disable I2C controller instance
        SNC_hw_i2c_disable(conf->id);
        SNC_hw_i2c_set_src_clk_config(conf->id, 0, 0);

        // Disable I2C BUS pad latches
        SNC_i2c_pad_latches(conf, SNC_I2C_PAD_LATCHES_OP_DISABLE);

        // Release I2C controller instance
        SNC_hw_sys_bsr_release(perif_id);
}

//==================== Configuration functions =================================

static void snc_hw_i2c_set_src_clk_config(b_ctx_t* b_ctx, HW_I2C_ID id, uint32_t clk_en,
        uint32_t clk_sel)
{
        SENIS_wadva(da(&CRG_COM->RESET_CLK_COM_REG),
                (id == HW_I2C1) ?
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, I2C_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, I2C_CLK_SEL)) :
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, I2C2_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, I2C2_CLK_SEL)));
        SENIS_wadva(da(&CRG_COM->SET_CLK_COM_REG),
                (id == HW_I2C1) ?
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, I2C_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, I2C_CLK_SEL)) :
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, I2C2_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, I2C2_CLK_SEL)));
}

static void snc_hw_i2c_enable(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_wadva(da(&IBA(id)->I2C_ENABLE_REG), 1);
}

static void snc_hw_i2c_disable(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_wadva(da(&IBA(id)->I2C_ENABLE_REG), 0);
}
_SNC_RETAINED static last_io_t last_io[2] = { 0 };

SNC_FUNC_DECL(snc_i2c_set_last_pin_conf_ucode, last_io_t* i2c_last_io, uint32_t* last_pins_deconf);

static void snc_i2c_set_last_pin_conf(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        const snc_i2c_io_conf_t* io_cfg = conf->io;
        snc_i2c_io_conf_compact_t scl_pad_off;
        snc_i2c_io_conf_compact_t sda_pad_off;

        ASSERT_WARNING(b_ctx);

        SENIS_labels(pt_last_pins_configured);

        SENIS_assign(da(&last_io[conf->id == HW_I2C2].deconf_ucode.senis_goto[0]),
                _SENIS_B_RDCBI_D(&snc_const[1], 0));
        SENIS_assign(da(&last_io[conf->id == HW_I2C2].deconf_ucode.senis_goto[1]),
                _SENIS_B_COBR_EQ_D(_SNC_OP_DIRECT_ADDRESS(l(pt_last_pins_configured))));
        SENIS_goto(da(&last_io[conf->id == HW_I2C2]));
        SENIS_label(pt_last_pins_configured);

        scl_pad_off.func = io_cfg->scl.off.function;
        scl_pad_off.mode = io_cfg->scl.off.mode;
        scl_pad_off.high = io_cfg->scl.off.high;
        scl_pad_off.pin  = io_cfg->scl.pin;
        scl_pad_off.port = io_cfg->scl.port;

        sda_pad_off.func = io_cfg->sda.off.function;
        sda_pad_off.mode = io_cfg->sda.off.mode;
        sda_pad_off.high = io_cfg->sda.off.high;
        sda_pad_off.pin  = io_cfg->sda.pin;
        sda_pad_off.port = io_cfg->sda.port;

        _SNC_STATIC(uint32_t, last_pins_deconf,
                2*sizeof(snc_i2c_io_conf_compact_t) + sizeof(last_io[0].deconf_ucode.deconf_proc),
                scl_pad_off.io_conf, sda_pad_off.io_conf, SNC_I2C_DECONF_PROC(io_cfg));

        senis_call(b_ctx, SNC_UCODE_CTX(snc_i2c_set_last_pin_conf_ucode),
                2 * 2, _SNC_OP(&last_io[conf->id == HW_I2C2]), _SNC_OP(last_pins_deconf));
}

SNC_FUNC_DEF(snc_i2c_set_last_pin_conf_ucode, last_io_t* i2c_last_io, uint32_t* last_pins_deconf)
{
        SENIS_copy(ia(&SNC_ARG(i2c_last_io)), ia(&SNC_ARG(last_pins_deconf)),
                ((2*sizeof(snc_i2c_io_conf_compact_t) +
                  sizeof(last_io[0].deconf_ucode.deconf_proc))/sizeof(uint32_t)));
}

static void snc_i2c_configure_pins(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        const snc_i2c_io_conf_t* io_cfg = conf->io;
        uint32_t func_offset =
                (conf->id == HW_I2C1) ? 0 : HW_GPIO_FUNC_I2C2_SCL - HW_GPIO_FUNC_I2C_SCL;


        // Reset and update the last configuration of the I2C pads
        SNC_i2c_set_last_pin_conf(conf);

        // Configure SCL
        SNC_hw_gpio_set_pin_function(io_cfg->scl.port, io_cfg->scl.pin,
                HW_GPIO_MODE_OUTPUT_OPEN_DRAIN, HW_GPIO_FUNC_I2C_SCL + func_offset);

        // Configure SDA
        SNC_hw_gpio_set_pin_function(io_cfg->sda.port, io_cfg->sda.pin,
                HW_GPIO_MODE_OUTPUT_OPEN_DRAIN, HW_GPIO_FUNC_I2C_SDA + func_offset);
}

static void snc_i2c_pad_latches(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SNC_I2C_PAD_LATCHES_OP pad_latches_op)
{
        const snc_i2c_io_conf_t* io_cfg = conf->io;
        uint32_t pads_stat[HW_GPIO_NUM_PORTS] = { 0 };

        pads_stat[io_cfg->scl.port] |= (1 << io_cfg->scl.pin);
        pads_stat[io_cfg->sda.port] |= (1 << io_cfg->sda.pin);
        for (HW_GPIO_PORT port = HW_GPIO_PORT_0; port < HW_GPIO_NUM_PORTS; port++) {
                if (pads_stat[port]) {
                        if (pad_latches_op == SNC_I2C_PAD_LATCHES_OP_ENABLE) {
                                SNC_hw_gpio_pads_latch_enable(port, pads_stat[port]);
                        } else {
                                SNC_hw_gpio_pads_latch_disable(port, pads_stat[port]);
                        }
                }
        }
}

SNC_FUNC_DECL(snc_i2c1_reconfig_ucode, snc_i2c_config_t* i2c_cfg);
SNC_FUNC_DECL(snc_i2c2_reconfig_ucode, snc_i2c_config_t* i2c_cfg);

void snc_i2c_reconfig(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_I2C1 || conf->id == HW_I2C2);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        const i2c_config* i2c_hw_cfg = &conf->drv->i2c;

        uint32_t clk_hcnt_ss_hs = -1;
        uint32_t clk_lcnt_ss_hs = -1;
        uint32_t clk_hcnt_fs = -1;
        uint32_t clk_lcnt_fs = -1;

        switch (i2c_hw_cfg->speed) {
        case HW_I2C_SPEED_STANDARD:
                if (!i2c_hw_cfg->clock_cfg.ss_hcnt && !i2c_hw_cfg->clock_cfg.ss_lcnt) {
                        clk_hcnt_ss_hs = 0x90;
                        clk_lcnt_ss_hs = 0x9E;
                } else {
                        clk_hcnt_ss_hs = i2c_hw_cfg->clock_cfg.ss_hcnt;
                        clk_lcnt_ss_hs = i2c_hw_cfg->clock_cfg.ss_lcnt;
                }
                break;
        case HW_I2C_SPEED_HIGH:
                if (!i2c_hw_cfg->clock_cfg.hs_hcnt && !i2c_hw_cfg->clock_cfg.hs_lcnt) {
                        clk_hcnt_ss_hs = 0x06;
                        clk_lcnt_ss_hs = 0x10;
                } else {
                        clk_hcnt_ss_hs = i2c_hw_cfg->clock_cfg.hs_hcnt;
                        clk_lcnt_ss_hs = i2c_hw_cfg->clock_cfg.hs_lcnt;
                }
                /* NO BREAK */
        case HW_I2C_SPEED_FAST:
                if (!i2c_hw_cfg->clock_cfg.fs_hcnt && !i2c_hw_cfg->clock_cfg.fs_lcnt) {
                        clk_hcnt_fs = 0x10;
                        clk_lcnt_fs = 0x2E;
                } else {
                        clk_hcnt_fs = i2c_hw_cfg->clock_cfg.fs_hcnt;
                        clk_lcnt_fs = i2c_hw_cfg->clock_cfg.fs_lcnt;
                }
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        _SNC_STATIC(snc_i2c_config_t, snc_i2c_hw_cfg, sizeof(snc_i2c_config_t), {
                .ctrl_params =
                        ((i2c_hw_cfg->speed + 1) << REG_POS(I2C, I2C_CON_REG, I2C_SPEED)) |
                        (1 << REG_POS(I2C, I2C_CON_REG, I2C_RESTART_EN)) |
                        ((i2c_hw_cfg->mode == HW_I2C_MODE_MASTER) << REG_POS(I2C, I2C_CON_REG, I2C_MASTER_MODE)) |
                        ((i2c_hw_cfg->mode == HW_I2C_MODE_MASTER) << REG_POS(I2C, I2C_CON_REG, I2C_SLAVE_DISABLE)) |
                        (i2c_hw_cfg->addr_mode << REG_POS(I2C, I2C_CON_REG, I2C_10BITADDR_MASTER)) |
                        (i2c_hw_cfg->addr_mode << REG_POS(I2C, I2C_CON_REG, I2C_10BITADDR_SLAVE)),
                .address = i2c_hw_cfg->address,
                .clk_hcnt_ss_hs = clk_hcnt_ss_hs,
                .clk_lcnt_ss_hs = clk_lcnt_ss_hs,
                .clk_hcnt_fs = clk_hcnt_fs,
                .clk_lcnt_fs = clk_lcnt_fs
        });

        senis_call(b_ctx,
                ((conf->id == HW_I2C1) ?
                        SNC_UCODE_CTX(snc_i2c1_reconfig_ucode) :
                        SNC_UCODE_CTX(snc_i2c2_reconfig_ucode)),
                2 * 1, _SNC_OP(snc_i2c_hw_cfg));
}

static void snc_i2c_reconfig_ucode_impl(b_ctx_t* b_ctx, HW_I2C_ID id,
        snc_i2c_config_t** p_i2c_cfg)
{
        // I2C controller instance initialization:
        // Configure I2C source clock
        SNC_hw_i2c_set_src_clk_config(id, 1, 0);
        // Disable I2C controller instance
        SNC_hw_i2c_disable(id);
        // Set the control register
        SENIS_wadad(da(&IBA(id)->I2C_CON_REG), ia(p_i2c_cfg));
        // Set the target address
        SENIS_inc4(da(p_i2c_cfg));
        SENIS_wadad(da(&IBA(id)->I2C_TAR_REG), ia(p_i2c_cfg));
        // Disable I2C interrupts
        SENIS_wadva(da(&IBA(id)->I2C_INTR_MASK_REG), 0x0000);
        // Set SCL values
        SENIS_inc4(da(p_i2c_cfg));
        SENIS_wadad(da(&IBA(id)->I2C_SS_SCL_HCNT_REG), ia(p_i2c_cfg)); // clk_hcnt_ss_hs
        SENIS_wadad(da(&IBA(id)->I2C_HS_SCL_HCNT_REG), ia(p_i2c_cfg)); // clk_hcnt_ss_hs
        SENIS_inc4(da(p_i2c_cfg));
        SENIS_wadad(da(&IBA(id)->I2C_SS_SCL_LCNT_REG), ia(p_i2c_cfg)); // clk_lcnt_ss_hs
        SENIS_wadad(da(&IBA(id)->I2C_HS_SCL_LCNT_REG), ia(p_i2c_cfg)); // clk_lcnt_ss_hs
        SENIS_inc4(da(p_i2c_cfg));
        SENIS_wadad(da(&IBA(id)->I2C_FS_SCL_HCNT_REG), ia(p_i2c_cfg)); // clk_hcnt_fs
        SENIS_inc4(da(p_i2c_cfg));
        SENIS_wadad(da(&IBA(id)->I2C_FS_SCL_LCNT_REG), ia(p_i2c_cfg)); // clk_lcnt_fs

        // Enable I2C controller instance
        SNC_hw_i2c_enable(id);

        // Clear TX_ABORT interrupt status and unlock TX FIFO
        SNC_hw_i2c_reset_abort_source(id);
}

SNC_FUNC_DEF(snc_i2c1_reconfig_ucode, snc_i2c_config_t* i2c_cfg)
{
        snc_i2c_reconfig_ucode_impl(b_ctx, HW_I2C1, &SNC_ARG(i2c_cfg));
}

SNC_FUNC_DEF(snc_i2c2_reconfig_ucode, snc_i2c_config_t* i2c_cfg)
{
        snc_i2c_reconfig_ucode_impl(b_ctx, HW_I2C2, &SNC_ARG(i2c_cfg));
}

//==================== Status Acquisition functions ============================

void snc_i2c_poll_for_ack(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);

        SENIS_labels(poll_for_ack);

        SENIS_label(poll_for_ack); {
                SNC_hw_i2c_reset_abort_source(conf->id);
                SNC_i2c_write(conf, da(&snc_const[0]), 1, SNC_HW_I2C_FLAG_ADD_STOP);
                SENIS_rdcbi(da(&IBA(conf->id)->I2C_TX_ABRT_SOURCE_REG),
                        (conf->drv->i2c.addr_mode == HW_I2C_ADDRESSING_10B));
                SENIS_cobr_eq(l(poll_for_ack));
        }
}

void snc_i2c_check_abort_source(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SENIS_OPER_TYPE abort_source_type, uint32_t* abort_source)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(abort_source);
        ASSERT_WARNING((abort_source_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(abort_source)));

        senis_assign(b_ctx, abort_source_type, abort_source,
                _SNC_OP(da(&IBA(conf->id)->I2C_TX_ABRT_SOURCE_REG)));

        SNC_hw_i2c_reset_abort_source(conf->id);
}

static void snc_hw_i2c_is_master_busy(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcbi(da(&IBA(id)->I2C_STATUS_REG), REG_POS(I2C, I2C_STATUS_REG, MST_ACTIVITY));
}

static void snc_hw_i2c_is_tx_fifo_empty(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcbi(da(&IBA(id)->I2C_STATUS_REG), REG_POS(I2C, I2C_STATUS_REG, TFE));
}

static void snc_hw_i2c_is_tx_fifo_not_full(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcbi(da(&IBA(id)->I2C_STATUS_REG), REG_POS(I2C, I2C_STATUS_REG, TFNF));
}

static void snc_hw_i2c_is_rx_fifo_not_empty(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcbi(da(&IBA(id)->I2C_STATUS_REG), REG_POS(I2C, I2C_STATUS_REG, RFNE));
}

static void snc_hw_i2c_is_abort_status(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcgr_z(da(&IBA(id)->I2C_TX_ABRT_SOURCE_REG));
}

static void snc_hw_i2c_reset_abort_source(b_ctx_t* b_ctx, HW_I2C_ID id)
{
        SENIS_rdcbi(da(&IBA(id)->I2C_CLR_TX_ABRT_REG), 0);
}

//==================== Data Read/Write functions ===============================

static void snc_i2c_transfer_write_ucode_impl(b_ctx_t* b_ctx, HW_I2C_ID id,
        uint32_t*** p_out_buf_ind, uint32_t* p_out_len, uint32_t* p_flags)
{
        SENIS_labels(
                while_len_start, check_tx_fifo_not_full, add_stop_mask, write_byte,
                check_len, check_tx_fifo_empty, check_master_busy
        );

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));

        SENIS_assign(da(&temp_out_buf[0]), ia(p_out_buf_ind));

        // Add needed temporary variable
        // Used as a helper variable for writing I2C data
        _SNC_TMP_ADD(uint32_t, temp_val, sizeof(uint32_t));

        // Init local variables

        // XOR and inc by one the length variable in order to obtain its negative equivalent
        // Now each inc1 of this variable corresponds to the (out_len--) expression
        SENIS_tobre(da(p_out_len), 0xFFFFFFFF);
        SENIS_inc1(da(p_out_len));

        // while(check_len) {
        SENIS_goto(l(check_len));

        SENIS_label(while_len_start);

        //         temp_val = *out_buf;
        SENIS_wadad(da(temp_val), ia(&temp_out_buf[0]));
        //         if(--out_len == 0) {
        SENIS_inc1(da(p_out_len));
        SENIS_rdcbi(da(p_out_len), 31);
        SENIS_cobr_eq(l(check_tx_fifo_not_full));
        //                 if(flags & SNC_HW_I2C_FLAG_ADD_STOP) {
        //                         add_stop_mask;
        //                 }

        SENIS_rdcbi(da(p_flags), SNC_HW_I2C_FLAG_ADD_STOP_BIT_POS);
        SENIS_cobr_eq(l(add_stop_mask));
        //         } else {

        //                 while(!hw_i2c_tx_fifo_not_full(id));
        //                 write_byte;
        //         }

        SENIS_label(check_tx_fifo_not_full);

        SNC_hw_i2c_is_tx_fifo_not_full(id);
        SENIS_cobr_eq(l(write_byte));
        SENIS_goto(l(check_tx_fifo_not_full));

        SENIS_label(add_stop_mask);

        //         temp_val ^=I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk;
        SENIS_tobre(da(temp_val), I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk);
        SENIS_goto(l(check_tx_fifo_not_full));

        SENIS_label(write_byte);

        //         IBA(id)->I2C_DATA_CMD_REG = temp_val;
        SENIS_wadad(da(&IBA(id)->I2C_DATA_CMD_REG), da(temp_val));
        SENIS_inc4(da(&temp_out_buf[0]));

        SENIS_label(check_len);

        // } end of  while(out_len)
        SENIS_rdcbi(da(p_out_len), 31);
        SENIS_cobr_eq(l(while_len_start));

        // if(hw_i2c_abort_status(id)) {return;}
        SNC_hw_i2c_is_abort_status(id);
        SENIS_cobr_gr(l(check_master_busy));

        // if(flags & SNC_HW_I2C_FLAG_WAIT_FOR_STOP) {
        SENIS_rdcbi(da(p_flags), SNC_HW_I2C_FLAG_ADD_STOP_BIT_POS);
        SENIS_cobr_eq(l(check_tx_fifo_empty));
        SENIS_goto(l(check_master_busy));

        //         while(!hw_i2c_tx_fifo_empty(id));
        SENIS_label(check_tx_fifo_empty);

        SNC_hw_i2c_is_tx_fifo_empty(id);
        SENIS_cobr_eq(l(check_master_busy));
        SENIS_goto(l(check_tx_fifo_empty));

        //         while(hw_i2c_master_is_busy(id));
        SENIS_label(check_master_busy);

        SNC_hw_i2c_is_master_busy(id);
        SENIS_cobr_eq(l(check_master_busy));
        // } end of if (flags & SNC_HW_I2C_FLAG_WAIT_FOR_STOP)

        _SNC_TMP_RMV(temp_val);

        SENIS_assign(ia(p_out_buf_ind), da(&temp_out_buf[0]));

        _SNC_TMP_RMV(temp_out_buf);
}

SNC_FUNC_DEF(snc_i2c1_transfer_write_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t flags)
{
        snc_i2c_transfer_write_ucode_impl(b_ctx, HW_I2C1, &SNC_ARG(out_buf_ind), &SNC_ARG(out_len),
                &SNC_ARG(flags));
}

SNC_FUNC_DEF(snc_i2c2_transfer_write_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t flags)
{
        snc_i2c_transfer_write_ucode_impl(b_ctx, HW_I2C2, &SNC_ARG(out_buf_ind), &SNC_ARG(out_len),
                &SNC_ARG(flags));
}

static void snc_i2c_transfer_read_ucode_impl(b_ctx_t* b_ctx, HW_I2C_ID id,
        uint32_t*** p_in_buf_ind, uint32_t* p_in_len, uint32_t* p_flags)
{
        SENIS_labels(
                read_while_begin, check_read_tx_fifo_not_full, prepare_read_transfer,
                clear_restart_mask, check_rx_fifo_not_empty, rr_in_len_is_set, check_rr_len,
                trigger_read_op, clear_stop_mask, check_rr_len_on_exit,
                check_in_length, check_abort_after_read, check_in_len_on_exit, read_byte,
                r_check_master_busy
        );

        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        SENIS_assign(da(&temp_in_buf[0]), ia(p_in_buf_ind));

        // Add needed temporary variables
        // Used as a helper variable for writing/reading I2C data
        _SNC_TMP_ADD(uint32_t, temp_val, sizeof(uint32_t));
        // Used as supplementary counters to in_len
        _SNC_TMP_ADD(uint32_t, rr_in_len, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, rr_in_len2, sizeof(uint32_t));
        // A first read byte flag
        _SNC_TMP_ADD(uint32_t, is_not_first_byte, sizeof(uint32_t));

        // Check abort status
        SNC_hw_i2c_is_abort_status(id);
        SENIS_cobr_gr(l(r_check_master_busy));

        // Init local variables

        // XOR and inc by one the length variables in order to obtain their negative equivalent
        // Now each inc1 of these variables corresponds to the (in_len--) expression
        SENIS_tobre(da(p_in_len), 0xFFFFFFFF);
        SENIS_inc1(da(p_in_len));

        // Copy the in_len to rr_in_len variable
        SENIS_wadad(da(rr_in_len2), da(p_in_len));

        // Set the is not first byte flag to false
        SENIS_wadva(da(is_not_first_byte), 0);

        // while(in_len) {
        SENIS_goto(l(check_in_length));

        SENIS_label(read_while_begin);

        //         while(hw_i2c_tx_fifo_not_full(id) && rr_in_len--) {
        //                 temp_val = I2C_I2C_DATA_CMD_REG_I2C_CMD_Msk          |
        //                            I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk         |
        //                            I2C_I2C_DATA_CMD_REG_I2C_RESTART_Msk;
        //

        _SNC_STATIC(uint32_t, const_minus_32, sizeof(uint32_t), ((uint32_t)-32));

        SENIS_rdcgr(da(&snc_const[1]), da(rr_in_len2));
        SENIS_cobr_gr(l(check_in_len_on_exit));

        SENIS_wadad(da(rr_in_len), da(rr_in_len2));
        SENIS_rdcgr(da(rr_in_len), da(const_minus_32));
        SENIS_cobr_gr(l(rr_in_len_is_set));
        SENIS_wadva(da(rr_in_len), ((uint32_t)-32));

        SENIS_label(rr_in_len_is_set);

        SENIS_goto(l(check_rr_len));

        SENIS_label(check_read_tx_fifo_not_full);

        SNC_hw_i2c_is_tx_fifo_not_full(id);
        SENIS_cobr_eq(l(prepare_read_transfer));
        SENIS_goto(l(check_in_len_on_exit));

        SENIS_label(prepare_read_transfer);

        SENIS_wadva(da(temp_val), I2C_I2C_DATA_CMD_REG_I2C_CMD_Msk              |
                                  I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk             |
                                  I2C_I2C_DATA_CMD_REG_I2C_RESTART_Msk);

        //                 if(is_not_first_byte) {
        //                         // clear the restart mask
        //                         temp_val ^= I2C_I2C_DATA_CMD_REG_I2C_RESTART_Msk;
        //                 }
        SENIS_rdcbi(da(is_not_first_byte), 0);
        SENIS_cobr_eq(l(clear_restart_mask));
        SENIS_inc1(da(is_not_first_byte));
        SENIS_rdcbi(da(p_flags), SNC_HW_I2C_FLAG_ADD_RESTART_BIT_POS);
        SENIS_cobr_eq(l(check_rr_len_on_exit));

        SENIS_label(clear_restart_mask);

        SENIS_tobre(da(temp_val), I2C_I2C_DATA_CMD_REG_I2C_RESTART_Msk);

        //                 if(rr_in_len) {
        //                         // clear the stop mask
        //                         temp_val ^= I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk;
        //                 }
        SENIS_label(check_rr_len_on_exit);

        SENIS_inc1(da(rr_in_len));
        SENIS_inc1(da(rr_in_len2));
        SENIS_rdcbi(da(rr_in_len), 31);
        SENIS_cobr_eq(l(clear_stop_mask));
        SENIS_rdcbi(da(p_flags), SNC_HW_I2C_FLAG_ADD_STOP_BIT_POS);
        SENIS_cobr_eq(l(trigger_read_op));

        SENIS_label(clear_stop_mask);

        SENIS_tobre(da(temp_val), I2C_I2C_DATA_CMD_REG_I2C_STOP_Msk);

        //                 IBA(id)->I2C_DATA_CMD_REG = temp_val;
        SENIS_label(trigger_read_op);

        SENIS_wadad(da(&IBA(id)->I2C_DATA_CMD_REG), da(temp_val));

        //         } end of while(hw_i2c_tx_fifo_not_full(id) && rr_in_len--)
        SENIS_label(check_rr_len);

        SENIS_rdcbi(da(rr_in_len), 31);
        SENIS_cobr_eq(l(check_read_tx_fifo_not_full));
        SENIS_goto(l(check_in_len_on_exit));

        //         while(in_len && hw_i2c_rx_fifo_not_empty(id)) {
        //                 *in_buf = IBA(id)->I2C_DATA_CMD_REG;
        //                 in_buf += 4;
        //                 in_len--;
        //         } end of while(in_len && hw_i2c_rx_fifo_not_empty(id))
        SENIS_label(check_rx_fifo_not_empty);

        SNC_hw_i2c_is_rx_fifo_not_empty(id);
        SENIS_cobr_eq(l(read_byte));
        SENIS_goto(l(check_abort_after_read));

        SENIS_label(read_byte);

        SENIS_wadad(ia(&temp_in_buf[0]), da(&IBA(id)->I2C_DATA_CMD_REG));
        SENIS_inc4(da(&temp_in_buf[0]));
        SENIS_inc1(da(p_in_len));

        // } end of while(in_len)
        SENIS_label(check_in_len_on_exit);

        SENIS_rdcbi(da(p_in_len), 31);
        SENIS_cobr_eq(l(check_rx_fifo_not_empty));

        // if(hw_i2c_get_abort_source(id)) {
        //         return;
        // }
        SENIS_label(check_abort_after_read);

        SNC_hw_i2c_is_abort_status(id);
        SENIS_cobr_gr(l(r_check_master_busy));

        SENIS_label(check_in_length);

        SENIS_rdcbi(da(p_in_len), 31);
        SENIS_cobr_eq(l(read_while_begin));

        // while(hw_i2c_master_is_busy(id));
        SENIS_label(r_check_master_busy);

        SNC_hw_i2c_is_master_busy(id);
        SENIS_cobr_eq(l(r_check_master_busy));

        // Remove the previously added temporary variables
        _SNC_TMP_RMV(is_not_first_byte);
        _SNC_TMP_RMV(rr_in_len2);
        _SNC_TMP_RMV(rr_in_len);
        _SNC_TMP_RMV(temp_val);

        SENIS_assign(ia(p_in_buf_ind), da(&temp_in_buf[0]));

        _SNC_TMP_RMV(temp_in_buf);
}

SNC_FUNC_DEF(snc_i2c1_transfer_read_ucode, uint32_t** in_buf_ind, uint32_t in_len,
        uint32_t flags)
{
        snc_i2c_transfer_read_ucode_impl(b_ctx, HW_I2C1, &SNC_ARG(in_buf_ind), &SNC_ARG(in_len),
                &SNC_ARG(flags));
}

SNC_FUNC_DEF(snc_i2c2_transfer_read_ucode, uint32_t** in_buf_ind, uint32_t in_len,
        uint32_t flags)
{
        snc_i2c_transfer_read_ucode_impl(b_ctx, HW_I2C2, &SNC_ARG(in_buf_ind), &SNC_ARG(in_len),
                &SNC_ARG(flags));
}

void snc_i2c_write(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
        SENIS_OPER_TYPE out_len_type, uint32_t* out_len,
        uint32_t flags)
{
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

        senis_call(b_ctx,
                ((conf->id == HW_I2C1) ?
                        SNC_UCODE_CTX(snc_i2c1_transfer_write_ucode) :
                        SNC_UCODE_CTX(snc_i2c2_transfer_write_ucode)),
                2 * 3, _SNC_OP(applied_out_buf_ind), out_len_type, out_len, _SNC_OP(flags));

        _SNC_TMP_RMV(temp_out_buf);
}

void snc_i2c_read(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
        uint32_t flags)
{
        uint32_t** applied_in_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(in_buf);
        ASSERT_WARNING((in_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(in_buf)));

        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        if (in_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_in_buf[0]), in_buf);
                applied_in_buf_ind = &temp_in_buf[0];
        } else {
                SNC_ASSERT(da(in_buf));
                applied_in_buf_ind = (uint32_t**)in_buf;
        }

        senis_call(b_ctx,
                ((conf->id == HW_I2C1) ?
                        SNC_UCODE_CTX(snc_i2c1_transfer_read_ucode) :
                        SNC_UCODE_CTX(snc_i2c2_transfer_read_ucode)),
                2 * 3, _SNC_OP(applied_in_buf_ind), in_len_type, in_len, _SNC_OP(flags));

        _SNC_TMP_RMV(temp_in_buf);
}

#if (dg_configI2C_ADAPTER == 1)
void snc_i2c_sync_with_snc_last_pin_conf(HW_I2C_ID id, ad_i2c_io_conf_t* last_io_cfg)
{
        ASSERT_WARNING(last_io_cfg);

        if (last_io[id == HW_I2C2].deconf_ucode.senis_goto[0]) {
                last_io_cfg->scl.port = last_io[id == HW_I2C2].scl_pad_off.port;
                last_io_cfg->scl.pin = last_io[id == HW_I2C2].scl_pad_off.pin;
                last_io_cfg->scl.off.mode = last_io[id == HW_I2C2].scl_pad_off.mode;
                last_io_cfg->scl.off.function = last_io[id == HW_I2C2].scl_pad_off.func;
                last_io_cfg->scl.off.high = last_io[id == HW_I2C2].scl_pad_off.high;

                last_io_cfg->sda.port = last_io[id == HW_I2C2].sda_pad_off.port;
                last_io_cfg->sda.pin = last_io[id == HW_I2C2].sda_pad_off.pin;
                last_io_cfg->sda.off.mode = last_io[id == HW_I2C2].sda_pad_off.mode;
                last_io_cfg->sda.off.function = last_io[id == HW_I2C2].sda_pad_off.func;
                last_io_cfg->sda.off.high = last_io[id == HW_I2C2].sda_pad_off.high;
        }
}
void snc_i2c_update_snc_last_pin_conf(HW_I2C_ID id, ad_i2c_io_conf_t* last_io_cfg)
{
        ASSERT_WARNING(last_io_cfg);

        uint32_t snc_last_io_ucode_instance[
                sizeof(last_io[0].deconf_ucode.deconf_proc)/sizeof(uint32_t) + 1] =
                { SNC_I2C_DECONF_PROC(last_io_cfg) , 0 };

        memcpy(&last_io[id == HW_I2C2].deconf_ucode, snc_last_io_ucode_instance,
                sizeof(snc_last_io_ucode_instance));
}
#endif /* dg_configI2C_ADAPTER */

#endif /* dg_configUSE_SNC_HW_I2C */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
