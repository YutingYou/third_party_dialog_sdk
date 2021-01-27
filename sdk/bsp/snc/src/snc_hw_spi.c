/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_SPI
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_spi.c
 *
 * @brief SNC-Implementation of SPI Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_SPI

#include "snc_defs.h"
#include "snc_hw_sys.h"
#include "snc_hw_gpio.h"

#include "snc_hw_spi.h"

#define SNC_SPI_DECONF_PROC(spi_io_conf_ptr)                                                    \
        _SENIS_B_WADVA_D((&GPIO->P0_RESET_DATA_REG +                                            \
                spi_io_conf_ptr->spi_cs[0].port - 2*((uint8_t)spi_io_conf_ptr->spi_cs[0].off.high)), \
                ((1 << spi_io_conf_ptr->spi_cs[0].pin))),                                       \
        _SENIS_B_WADVA_D((&GPIO->P0_00_MODE_REG +                                               \
                (&GPIO->P1_00_MODE_REG - &GPIO->P0_00_MODE_REG)*spi_io_conf_ptr->spi_cs[0].port + \
                spi_io_conf_ptr->spi_cs[0].pin),                                                \
                (spi_io_conf_ptr->spi_cs[0].off.function | spi_io_conf_ptr->spi_cs[0].off.mode)), \
        _SENIS_B_WADVA_D((&CRG_TOP->P0_RESET_PAD_LATCH_REG + 3*spi_io_conf_ptr->spi_cs[0].port), \
                ((1 << spi_io_conf_ptr->spi_cs[0].pin)))

#define SNC_SPI_GPIO_VALID(port, pin)   ((port != HW_GPIO_PORT_NONE) && (pin != HW_GPIO_PIN_NONE))

/*
 * ENUMERATION, DATA STRUCTURE AND DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/*
 * FIFO mode
 */
typedef enum {
        /* Bidirectional mode */
        SNC_HW_SPI_FIFO_RX_TX = HW_SPI_FIFO_RX_TX << REG_POS(SPI, SPI_CTRL_REG, SPI_FIFO_MODE),
        /* Read only mode */
        SNC_HW_SPI_FIFO_RX_ONLY = HW_SPI_FIFO_RX_ONLY << REG_POS(SPI, SPI_CTRL_REG, SPI_FIFO_MODE),
        /* Write only mode */
        SNC_HW_SPI_FIFO_TX_ONLY = HW_SPI_FIFO_TX_ONLY << REG_POS(SPI, SPI_CTRL_REG, SPI_FIFO_MODE),
        /* Backwards compatible mode */
        SNC_HW_SPI_FIFO_NONE = HW_SPI_FIFO_NONE << REG_POS(SPI, SPI_CTRL_REG, SPI_FIFO_MODE),
} SNC_HW_SPI_FIFO;

/*
 * SPI pad latches operation type
 */
typedef enum {
        SNC_SPI_PAD_LATCHES_OP_ENABLE,          /* Enable pad latches */
        SNC_SPI_PAD_LATCHES_OP_DISABLE          /* Disable pad latches */
} SNC_SPI_PAD_LATCHES_OP;

/*
 * Data type for the structure used to hold the previous modes for SPI FIFO and WORD
 */
typedef struct snc_spi_state_t {
        uint32_t word_mode;
        uint32_t fifo_mode;
} snc_spi_state_t;

typedef struct {
        struct {
                struct {
                        uint32_t last_cs_high_off_assign[2];
                        uint32_t last_cs_func_mode_off_assign[2];
                        uint32_t last_cs_pa_latch_disable_assign[2];
                } deconf_proc;
                uint32_t senis_goto[2];
        } deconf_ucode;
} last_cs_t;

/*
 * Data structure used to hold the previous modes for SPI FIFO and WORD
 */
_SNC_RETAINED static snc_spi_state_t snc_spi_state[2];

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_hw_spi_set_src_clk_config(b_ctx_t* b_ctx, HW_SPI_ID id, uint32_t clk_en,
        uint32_t clk_sel);
/**
 * \brief Function used in SNC context to configure SPI source clock
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance to be configured  (HW_SPI1 or HW_SPI2)
 * \param [in] clk_en   (uint32_t: build-time-only value)
 *                      enables or disables the SPI clock source (1-Enable, 0-Disable)
 * \param [in] clk_sel  (uint32_t: build-time-only value)
 *                      selects the clock source (1-Div1 , 0-DivN)
 */
#define SNC_hw_spi_set_src_clk_config(id, clk_en, clk_sel)                                      \
        snc_hw_spi_set_src_clk_config(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id),                      \
                                             _SNC_OP_VALUE(uint32_t, clk_en),                   \
                                             _SNC_OP_VALUE(uint32_t, clk_sel))

static void snc_hw_spi_toggle_enable(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to enable or disable SPI controller
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance to be toggled (enabled/disabled) (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_toggle_enable(id)                                                            \
        snc_hw_spi_toggle_enable(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_toggle_reset(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to reset or clear reset status of the SPI bus
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance the reset status of which
 *                      is to be toggled (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_toggle_reset(id)                                                             \
        snc_hw_spi_toggle_reset(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_init(b_ctx_t* b_ctx, HW_SPI_ID id, const spi_config* cfg);
/**
 * \brief Function used in SNC context to initialize SPI controller
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance (HW_SPI1 or HW_SPI2)
 * \param [in] cfg      (spi_config*: build-time-only value)
 *                      pointer to the SPI configuration structure
 */
#define SNC_hw_spi_init(id, cfg)                                                                \
        snc_hw_spi_init(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id),                                    \
                               _SNC_OP_VALUE(const spi_config*, cfg))

static void snc_hw_spi_is_tx_fifo_empty(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to check if SPI TX FIFO is empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if SPI TX FIFO is empty,
 * therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_spi_tx_fifo_is_empty)) to decide on
 * an action.
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_is_tx_fifo_empty(id)                                                          \
        snc_hw_spi_is_tx_fifo_empty(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_is_busy(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to check if SPI bus is busy
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if SPI bus is busy,
 * therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_spi_is_busy)) to decide on
 * an action.
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_is_busy(id)                                                                  \
        snc_hw_spi_is_busy(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_wait_while_busy(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to wait while SPI bus is busy
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_wait_while_busy(id)                                                          \
        snc_hw_spi_wait_while_busy(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_is_rx_fifo_empty(b_ctx_t* b_ctx, HW_SPI_ID id);
/**
 * \brief Function used in SNC context to check if SPI RX FIFO is empty
 *
 * When this function is called, EQ_FLAG in SNC_STATUS REG of SNC is set if SPI RX FIFO is empty,
 * therefore it can be followed by a SENIS_cobr_eq(l(gothere_if_spi_rx_fifo_is_empty)) to decide on
 * an action.
 *
 * \param [in] id       (HW_SPI_ID: build-time-only value)
 *                      SPI controller instance (HW_SPI1 or HW_SPI2)
 */
#define SNC_hw_spi_is_rx_fifo_empty(id)                                                         \
        snc_hw_spi_is_rx_fifo_empty(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id))

static void snc_hw_spi_change_fifo_mode(b_ctx_t* b_ctx, HW_SPI_ID id,
        SENIS_OPER_TYPE fifo_mode_type, uint32_t* fifo_mode);
/**
 * \brief Function used in SNC context to change the SPI FIFO mode
 *
 * \param [in] id               (HW_SPI_ID: build-time-only value)
 *                              SPI controller instance (HW_SPI1 or HW_SPI2)
 * \param [in] fifo_mode        (SNC_HW_SPI_FIFO: use da() or ia() or build-time-only value)
 *                              SPI FIFO mode
 */
#define SNC_hw_spi_change_fifo_mode(id, fifo_mode)                                              \
        snc_hw_spi_change_fifo_mode(b_ctx, _SNC_OP_VALUE(HW_SPI_ID, id), _SNC_OP(fifo_mode))

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
static void snc_spi_set_last_pin_conf(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        bool deconf_prev_cs);
/**
 * \brief Function used in SNC context to reset and update last SPI chip select (CS) pad
 * *
 * \param [in] conf             (const snc_spi_controller_conf_t*: build-time-only value)
 *                              SPI controller configuration
 * \param [in] deconf_prev_cs   (bool: build-time-only value)
 *                              Indication that the previously selected CS pad must be de-configured
 */
#define SNC_spi_set_last_pin_conf(conf, deconf_prev_cs)                                         \
        snc_spi_set_last_pin_conf(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf), \
                                  _SNC_OP_VALUE(bool, deconf_prev_cs))
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

static void snc_spi_configure_pins(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to configure SPI bus pads
 * *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 */
#define SNC_spi_configure_pins(conf)                                                            \
        snc_spi_configure_pins(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

static void snc_spi_deconfigure_pins(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
/**
 * \brief Function used in SNC context to de-configure SPI bus pads
 * *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 */
#define SNC_spi_deconfigure_pins(conf)                                                          \
        snc_spi_deconfigure_pins(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

static void snc_spi_pad_latches(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SNC_SPI_PAD_LATCHES_OP pad_latches_op);
/**
 * \brief Function used in SNC context to configure SPI bus pad latches
 * *
 * \param [in] conf             (const snc_spi_controller_conf_t*: build-time-only value)
 *                              SPI controller configuration
 * \param [in] pad_latches_op   (SNC_SPI_PAD_LATCHES_OP: build-time-only value)
 *                              SPI bus pad latches operation type
 */
#define SNC_spi_pad_latches(conf, pad_latches_op)                                               \
        snc_spi_pad_latches(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),       \
                                   _SNC_OP_VALUE(SNC_SPI_PAD_LATCHES_OP, pad_latches_op))

//==================== Controller Acquisition functions ========================

void snc_spi_open(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_SPI1 || conf->id == HW_SPI2);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        const spi_config* spi_hw_cfg = &conf->drv->spi;
        uint32_t perif_id = (conf->id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2;

        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_9BIT);
        ASSERT_WARNING(spi_hw_cfg->smn_role != HW_SPI_MODE_SLAVE);
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */

        // SPI clock should be at least configured
        ASSERT_WARNING(SNC_SPI_GPIO_VALID(conf->io->spi_clk.port, conf->io->spi_clk.pin));

        // SPI controller instance acquisition
        SNC_hw_sys_bsr_acquire(perif_id);

        // SPI controller configuration:
        // SPI controller instance initialization:
        // Configure SPI source clock
        SNC_hw_spi_set_src_clk_config(conf->id, 1, 0);
        // Initialize SPI controller registers and status
        SNC_hw_spi_init(conf->id, spi_hw_cfg);

        // Enable SPI controller instance
        SNC_hw_spi_toggle_enable(conf->id);

        // SPI bus pads initialization
        SNC_spi_configure_pins(conf);

        // Enable SPI bus pad latches
        SNC_spi_pad_latches(conf, SNC_SPI_PAD_LATCHES_OP_ENABLE);
}

void snc_spi_close(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        uint32_t perif_id = (conf->id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2;

        ASSERT_WARNING(b_ctx);

        // while (hw_spi_is_busy(conf->id));
        SNC_hw_spi_wait_while_busy(conf->id);

        // Disable SPI controller instance
        SNC_hw_spi_toggle_enable(conf->id);
        SNC_hw_spi_set_src_clk_config(conf->id, 0, 0);

        // SPI bus pads restored to the off state
        SNC_spi_deconfigure_pins(conf);

        // Disable SPI BUS pad latches
        SNC_spi_pad_latches(conf, SNC_SPI_PAD_LATCHES_OP_DISABLE);

        // Release SPI controller instance
        SNC_hw_sys_bsr_release(perif_id);
}

//==================== Configuration functions =================================

static void snc_hw_spi_set_src_clk_config(b_ctx_t* b_ctx, HW_SPI_ID id, uint32_t clk_en,
        uint32_t clk_sel)
{
        SENIS_wadva(da(&CRG_COM->RESET_CLK_COM_REG),
                (id == HW_SPI1) ?
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, SPI_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, SPI_CLK_SEL)) :
                (REG_MSK(CRG_COM, RESET_CLK_COM_REG, SPI2_ENABLE) |
                 REG_MSK(CRG_COM, RESET_CLK_COM_REG, SPI2_CLK_SEL)));
        SENIS_wadva(da(&CRG_COM->SET_CLK_COM_REG),
                (id == HW_SPI1) ?
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, SPI_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, SPI_CLK_SEL)) :
                (clk_en << REG_POS(CRG_COM, SET_CLK_COM_REG, SPI2_ENABLE) |
                 clk_sel << REG_POS(CRG_COM, SET_CLK_COM_REG, SPI2_CLK_SEL)));
}

static void snc_hw_spi_toggle_enable(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_tobre(da(&SBA(id)->SPI_CTRL_REG), REG_MSK(SPI, SPI_CTRL_REG, SPI_ON));
}

static void snc_hw_spi_toggle_reset(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_tobre(da(&SBA(id)->SPI_CTRL_REG), REG_MSK(SPI, SPI_CTRL_REG, SPI_RST));
}

static void snc_hw_spi_init(b_ctx_t* b_ctx, HW_SPI_ID id, const spi_config* cfg)
{
        SENIS_wadva(da(&SBA(id)->SPI_CTRL_REG),
                ((cfg->word_mode << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk) |
                ((cfg->smn_role << SPI_SPI_CTRL_REG_SPI_SMN_Pos) & SPI_SPI_CTRL_REG_SPI_SMN_Msk) |
                ((cfg->polarity_mode << SPI_SPI_CTRL_REG_SPI_POL_Pos) & SPI_SPI_CTRL_REG_SPI_POL_Msk) |
                ((cfg->phase_mode << SPI_SPI_CTRL_REG_SPI_PHA_Pos) & SPI_SPI_CTRL_REG_SPI_PHA_Msk) |
                ((cfg->mint_mode << SPI_SPI_CTRL_REG_SPI_MINT_Pos) & SPI_SPI_CTRL_REG_SPI_MINT_Msk) |
                ((cfg->xtal_freq << SPI_SPI_CTRL_REG_SPI_CLK_Pos) & SPI_SPI_CTRL_REG_SPI_CLK_Msk) |
                ((cfg->fifo_mode << SPI_SPI_CTRL_REG_SPI_FIFO_MODE_Pos) & SPI_SPI_CTRL_REG_SPI_FIFO_MODE_Msk));
        SENIS_wadva(da(&snc_spi_state[id == HW_SPI2].word_mode),
                ((cfg->word_mode << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));
        SENIS_wadva(da(&snc_spi_state[id == HW_SPI2].fifo_mode),
                ((cfg->fifo_mode << SPI_SPI_CTRL_REG_SPI_FIFO_MODE_Pos) & SPI_SPI_CTRL_REG_SPI_FIFO_MODE_Msk));
}

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
static last_cs_t last_cs[2] = { 0 };

SNC_FUNC_DECL(snc_spi_set_last_pin_conf_ucode, last_cs_t* spi_last_cs, uint32_t* last_cs_pin_deconf);

static void snc_spi_set_last_pin_conf(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        bool deconf_prev_cs)
{
        const snc_spi_io_conf_t* io_cfg = conf->io;

        if (deconf_prev_cs) {
                SENIS_labels(pt_last_cs_pin_configured);

                SENIS_assign(da(&last_cs[conf->id == HW_SPI2].deconf_ucode.senis_goto[0]),
                        _SENIS_B_RDCBI_D(&snc_const[1], 0));
                SENIS_assign(da(&last_cs[conf->id == HW_SPI2].deconf_ucode.senis_goto[1]),
                        _SENIS_B_COBR_EQ_D(_SNC_OP_DIRECT_ADDRESS(l(pt_last_cs_pin_configured))));
                SENIS_goto(da(&last_cs[conf->id == HW_SPI2]));
                SENIS_label(pt_last_cs_pin_configured);
        }

        _SNC_STATIC(uint32_t, last_cs_pin_deconf,
                sizeof(last_cs[0].deconf_ucode.deconf_proc), SNC_SPI_DECONF_PROC(io_cfg));
        if (!SNC_SPI_GPIO_VALID(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin)) {
                if (b_ctx->upd) {
                        memset(last_cs_pin_deconf, 0,
                                sizeof(last_cs[0].deconf_ucode.deconf_proc) * sizeof(uint32_t));
                }
        }

        senis_call(b_ctx, SNC_UCODE_CTX(snc_spi_set_last_pin_conf_ucode),
                2 * 2, _SNC_OP(&last_cs[conf->id == HW_SPI2]), _SNC_OP(last_cs_pin_deconf));
}

SNC_FUNC_DEF(snc_spi_set_last_pin_conf_ucode, last_cs_t* spi_last_cs, uint32_t* last_cs_pin_deconf)
{
        SENIS_copy(ia(&SNC_ARG(spi_last_cs)), ia(&SNC_ARG(last_cs_pin_deconf)),
                ((sizeof(last_cs[0].deconf_ucode.deconf_proc))/sizeof(uint32_t)));
}
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

static void snc_spi_configure_pins(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        const snc_spi_io_conf_t* io_cfg = conf->io;
        uint32_t func_offset =
                (conf->id == HW_SPI1) ? 0 : HW_GPIO_FUNC_SPI2_DI - HW_GPIO_FUNC_SPI_DI;

        // Configure DO
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_do.port, io_cfg->spi_do.pin)) {
                SNC_hw_gpio_set_pin_function(io_cfg->spi_do.port, io_cfg->spi_do.pin,
                        io_cfg->spi_do.on.mode, HW_GPIO_FUNC_SPI_DO + func_offset);
        }

        // Configure DI
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_di.port, io_cfg->spi_di.pin)) {
                SNC_hw_gpio_set_pin_function(io_cfg->spi_di.port, io_cfg->spi_di.pin,
                        io_cfg->spi_di.on.mode, HW_GPIO_FUNC_SPI_DI + func_offset);
        }

        // Configure CLK
        SNC_hw_gpio_set_pin_function(io_cfg->spi_clk.port, io_cfg->spi_clk.pin,
                io_cfg->spi_clk.on.mode, HW_GPIO_FUNC_SPI_CLK + func_offset);

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
        // Update the last configuration of the SPI chip select (CS) pad
        SNC_spi_set_last_pin_conf(conf, false);
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

        // Configure CS
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin,
                        io_cfg->spi_cs[0].on.mode, HW_GPIO_FUNC_SPI_EN + func_offset,
                        io_cfg->spi_cs[0].on.high);
        }
}

static void snc_spi_deconfigure_pins(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        const snc_spi_io_conf_t* io_cfg = conf->io;

        // De-configure DO
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_do.port, io_cfg->spi_do.pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->spi_do.port, io_cfg->spi_do.pin,
                        io_cfg->spi_do.off.mode, io_cfg->spi_do.off.function, io_cfg->spi_do.off.high);
        }

        // De-configure DI
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_di.port, io_cfg->spi_di.pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->spi_di.port, io_cfg->spi_di.pin,
                        io_cfg->spi_di.off.mode, io_cfg->spi_di.off.function, io_cfg->spi_di.off.high);
        }

        // De-configure CLK
        SNC_hw_gpio_configure_pin(io_cfg->spi_clk.port, io_cfg->spi_clk.pin,
                io_cfg->spi_clk.off.mode, io_cfg->spi_clk.off.function, io_cfg->spi_clk.off.high);

        // De-configure CS
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin,
                        io_cfg->spi_cs[0].off.mode, io_cfg->spi_cs[0].off.function,
                        io_cfg->spi_cs[0].off.high);
        }
}

static void snc_spi_pad_latches(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SNC_SPI_PAD_LATCHES_OP pad_latches_op)
{
        const snc_spi_io_conf_t* io_cfg = conf->io;
        uint32_t pads_stat[HW_GPIO_NUM_PORTS] = { 0 };

        if (SNC_SPI_GPIO_VALID(io_cfg->spi_do.port, io_cfg->spi_do.pin)) {
                pads_stat[io_cfg->spi_do.port] |= (1 << io_cfg->spi_do.pin);
        }
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_di.port, io_cfg->spi_di.pin)) {
                pads_stat[io_cfg->spi_di.port] |= (1 << io_cfg->spi_di.pin);
        }
        pads_stat[io_cfg->spi_clk.port] |= (1 << io_cfg->spi_clk.pin);
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin)) {
                pads_stat[io_cfg->spi_cs[0].port] |= (1 << io_cfg->spi_cs[0].pin);
        }
        for (HW_GPIO_PORT port = HW_GPIO_PORT_0; port < HW_GPIO_NUM_PORTS; port++) {
                if (pads_stat[port]) {
                        if (pad_latches_op == SNC_SPI_PAD_LATCHES_OP_ENABLE) {
                                SNC_hw_gpio_pads_latch_enable(port, pads_stat[port]);
                        } else {
                                SNC_hw_gpio_pads_latch_disable(port, pads_stat[port]);
                        }
                }
        }
}

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
void snc_spi_reconfig(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);
        ASSERT_WARNING(conf->id == HW_SPI1 || conf->id == HW_SPI2);
        ASSERT_WARNING(conf->io);
        ASSERT_WARNING(conf->drv);

        const snc_spi_io_conf_t* io_cfg = conf->io;
        const spi_config* spi_hw_cfg = &conf->drv->spi;
        uint32_t func_offset =
                (conf->id == HW_SPI1) ? 0 : HW_GPIO_FUNC_SPI2_DI - HW_GPIO_FUNC_SPI_DI;

        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_9BIT);
        ASSERT_WARNING(spi_hw_cfg->smn_role != HW_SPI_MODE_SLAVE);
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
        ASSERT_WARNING(spi_hw_cfg->word_mode != HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */

        // Reset and update the last configuration of the SPI chip select (CS) pad
        SNC_spi_set_last_pin_conf(conf, true);

        // SPI controller instance initialization:
        // Configure SPI source clock
        SNC_hw_spi_set_src_clk_config(conf->id, 1, 0);
        // Initialize SPI controller registers and status
        SNC_hw_spi_init(conf->id, spi_hw_cfg);

        // Enable SPI controller instance
        SNC_hw_spi_toggle_enable(conf->id);

        // Configure CS
        if (SNC_SPI_GPIO_VALID(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin)) {
                SNC_hw_gpio_configure_pin(io_cfg->spi_cs[0].port, io_cfg->spi_cs[0].pin,
                        io_cfg->spi_cs[0].on.mode, HW_GPIO_FUNC_SPI_EN + func_offset,
                        io_cfg->spi_cs[0].on.high);
                // Enable latch for cs pad
                SNC_hw_gpio_pad_latch_enable(conf->io->spi_cs[0].port, conf->io->spi_cs[0].pin);
        }
}
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

//==================== Status Acquisition functions ============================

static void snc_hw_spi_is_tx_fifo_empty(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_rdcbi(da(&SBA(id)->SPI_CTRL_REG), REG_POS(SPI, SPI_CTRL_REG, SPI_TX_FIFO_EMPTY));
}

static void snc_hw_spi_is_busy(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_rdcbi(da(&SBA(id)->SPI_CTRL_REG), REG_POS(SPI, SPI_CTRL_REG, SPI_BUSY));
}

static void snc_hw_spi_wait_while_busy(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_labels(pt_check_spi_busy);

        SENIS_label(pt_check_spi_busy);
        SNC_hw_spi_is_busy(id);
        SENIS_cobr_eq(l(pt_check_spi_busy));
}

static void snc_hw_spi_is_rx_fifo_empty(b_ctx_t* b_ctx, HW_SPI_ID id)
{
        SENIS_rdcbi(da(&SBA(id)->SPI_CTRL_REG), REG_POS(SPI, SPI_CTRL_REG, SPI_RX_FIFO_EMPTY));
}

//==================== CS handling functions ===================================

void snc_spi_activate_cs(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);

        const spi_config* spi_hw_cfg = &conf->drv->spi;

        SNC_hw_gpio_set_inactive(spi_hw_cfg->cs_pad.port, spi_hw_cfg->cs_pad.pin);
}

void snc_spi_deactivate_cs(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(conf);

        const spi_config* spi_hw_cfg = &conf->drv->spi;

        SNC_hw_gpio_set_active(spi_hw_cfg->cs_pad.port, spi_hw_cfg->cs_pad.pin);
}

//==================== FIFO control functions ==================================

SNC_FUNC_DECL(snc_hw_spi1_change_fifo_mode_ucode, uint32_t fifo_mode);
SNC_FUNC_DECL(snc_hw_spi2_change_fifo_mode_ucode, uint32_t fifo_mode);

static void snc_hw_spi_change_fifo_mode(b_ctx_t* b_ctx, HW_SPI_ID id,
        SENIS_OPER_TYPE fifo_mode_type, uint32_t* fifo_mode)
{
        senis_call(b_ctx,
                ((id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_hw_spi1_change_fifo_mode_ucode) :
                        SNC_UCODE_CTX(snc_hw_spi2_change_fifo_mode_ucode)),
                2 * 1, fifo_mode_type, fifo_mode);
}

static void snc_hw_spi_change_fifo_mode_ucode_impl(b_ctx_t* b_ctx, HW_SPI_ID id,
        uint32_t* p_fifo_mode)
{
        SENIS_labels(pt_change_fifo_mode, pt_exit);

        // if (snc_spi_state[id == HW_SPI2].fifo_mode != fifo_mode) {
        SENIS_rdcgr(da(&snc_spi_state[id == HW_SPI2].fifo_mode), da(p_fifo_mode));
        SENIS_cobr_gr(l(pt_change_fifo_mode));
        SENIS_rdcgr(da(p_fifo_mode), da(&snc_spi_state[id == HW_SPI2].fifo_mode));
        SENIS_cobr_gr(l(pt_change_fifo_mode));
        SENIS_goto(l(pt_exit));
        SENIS_label(pt_change_fifo_mode); {
                // hw_spi_enable(id, 0);
                SNC_hw_spi_toggle_enable(id);

                // HW_SPI_REG_SETF(SPI, SPI_CTRL_REG, SPI_FIFO_MODE, fifo_mode);
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG), da(&snc_spi_state[id == HW_SPI2].fifo_mode));
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG), da(p_fifo_mode));
                SENIS_assign(da(&snc_spi_state[id == HW_SPI2].fifo_mode), da(p_fifo_mode));

                // hw_spi_enable(id, 1);
                SNC_hw_spi_toggle_enable(id);
                SNC_hw_spi_toggle_reset(id);
                SNC_hw_spi_toggle_reset(id);
        }
        // } //if

        SENIS_label(pt_exit);
}

SNC_FUNC_DEF(snc_hw_spi1_change_fifo_mode_ucode, uint32_t fifo_mode)
{
        snc_hw_spi_change_fifo_mode_ucode_impl(b_ctx, HW_SPI1, &SNC_ARG(fifo_mode));
}

SNC_FUNC_DEF(snc_hw_spi2_change_fifo_mode_ucode, uint32_t fifo_mode)
{
        snc_hw_spi_change_fifo_mode_ucode_impl(b_ctx, HW_SPI2, &SNC_ARG(fifo_mode));
}

//==================== Data Read/Write functions ===============================

static void snc_spi_transfer_write_ucode_impl(b_ctx_t* b_ctx, HW_SPI_ID id,
        uint32_t* p_out_word_mode, uint32_t*** p_out_buf_ind, uint32_t* p_out_len,
        uint32_t* p_cnt_words)
{
        SENIS_labels(
#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
#if CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
                pt_spi_word_is_lt_32bit,
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                pt_spi_word_is_set,
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */
                pt_check_tx_cnt, pt_check_tx_fifo_empty, ph_tx_cnt_inc,
                pt_write
#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
                , ph_tx_reload_count, pt_tx_complete,
                pt_return
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */
        );

        // hw_spi_change_fifo_mode(id, HW_SPI_FIFO_TX_ONLY);
        SNC_hw_spi_change_fifo_mode(id, SNC_HW_SPI_FIFO_TX_ONLY);

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
        // Static variables in SNC context
#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
        _SNC_STATIC(uint32_t, const_spi_word_16bit, sizeof(uint32_t), HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
        _SNC_STATIC(uint32_t, const_spi_word_32bit, sizeof(uint32_t), HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

        // Non-static local variables in SNC context
        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, tx_cnt, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, cur_spi_word_mode, sizeof(uint32_t));

        SENIS_assign(da(&temp_out_buf[0]), ia(p_out_buf_ind));

        // tx_cnt = (uint32_t)(-out_len - 1);
        SENIS_wadad(da(tx_cnt), da(p_out_len));
        SENIS_tobre(da(tx_cnt), 0xFFFFFFFF);

        // Use "+ 1" step for increasing tx_cnt in the commands located
        // at ph_tx_cnt_inc label
        SENIS_assign(l(ph_tx_cnt_inc), _SENIS_B_INC_D(tx_cnt));
        SENIS_assign(l(ph_tx_cnt_inc) + 1, _SENIS_B_NOP());

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
        // if (!cnt_words) {
        SENIS_rdcbi(da(p_cnt_words), 0);
        SENIS_cobr_eq(l(pt_check_tx_cnt)); {
                // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, 0);
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                        da(&snc_spi_state[id == HW_SPI2].word_mode));

                // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                SENIS_assign(da(cur_spi_word_mode),
                        ((HW_SPI_WORD_8BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                // Perform a "TOBRE" to tx_cnt with 0x0 in the command located
                // at ph_tx_reload_count label
                SENIS_assign(l(ph_tx_reload_count) + 1, 0x0);

#if CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
                // if (out_word_mode == HW_SPI_WORD_32BIT) {
                SENIS_rdcgr(da(const_spi_word_32bit), da(p_out_word_mode));
                SENIS_cobr_gr(l(pt_spi_word_is_lt_32bit)); {
                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_32BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));
                        // Use "+ 4" step for increasing tx_cnt in the commands located
                        // at ph_tx_cnt_inc label
                        SENIS_assign(l(ph_tx_cnt_inc), _SENIS_B_INC4_D(tx_cnt));
                        // Perform a "TOBRE" to tx_cnt with 0x3 in the command located
                        // at ph_tx_reload_count label
                        SENIS_assign(l(ph_tx_reload_count) + 1, 0x3);
                }
                // }
#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
                SENIS_goto(l(pt_spi_word_is_set));
                // else
#endif /* #if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
                SENIS_label(pt_spi_word_is_lt_32bit);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
                // if (out_word_mode == HW_SPI_WORD_16BIT) {
                SENIS_rdcgr(da(const_spi_word_16bit), da(p_out_word_mode));
                SENIS_cobr_gr(l(pt_spi_word_is_set)); {
                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_16BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));
                        // Use "+ 2" step for increasing tx_cnt in the commands located
                        // at ph_tx_cnt_inc label
                        SENIS_assign(l(ph_tx_cnt_inc) + 1, _SENIS_B_INC_D(tx_cnt));
                        // Perform a "TOBRE" to tx_cnt with 0x1 in the command located
                        // at ph_tx_reload_count label
                        SENIS_assign(l(ph_tx_reload_count) + 1, 0x1);
                }
                // }
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
                SENIS_label(pt_spi_word_is_set);

                // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, cur_spi_word_mode);
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG), da(cur_spi_word_mode));
        }
        // } //if
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

        // while (tx_cnt > "SPI word size") {
        SENIS_goto(l(pt_check_tx_cnt)); {
                // if (hw_spi_is_tx_fifo_empty(id)) {
                SENIS_label(pt_check_tx_fifo_empty); {
                        // continue;
                        SNC_hw_spi_is_tx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_write));
                        SENIS_goto(l(pt_check_tx_fifo_empty));
                }
                // } //if

                SENIS_label(pt_write);

                // hw_spi_write_word(id, out_buf, wordsize);
                SENIS_wadad(da(&SBA(id)->SPI_RX_TX_REG), ia(&temp_out_buf[0]));
                // out_buf++;
                SENIS_inc4(da(&temp_out_buf[0]));

                SENIS_label(pt_check_tx_cnt);
                // tx_cnt += N; // where N = 1, 2 or 4, depending on the SPI word size mode
                SENIS_label(ph_tx_cnt_inc);
                senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                //
                SENIS_rdcbi(da(tx_cnt), 31);
                SENIS_cobr_eq(l(pt_check_tx_fifo_empty));
        }
        // } //while

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
        // if (!cnt_words) {
        SENIS_rdcbi(da(p_cnt_words), 0);
        SENIS_cobr_eq(l(pt_return)); {
                // tx_cnt ^= M; // where M = 0x0, 0x1 or 0x3, depending on the SPI word size mode
                //              // indicating the remaining bytes to be written using 8bit word size mode
                SENIS_label(ph_tx_reload_count);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(tx_cnt)), SNC_PLACE_HOLDER);

                // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_8BIT);
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG), da(cur_spi_word_mode));

                // if (tx_cnt > 0) {
                SENIS_rdcgr(da(&snc_const[1]), da(tx_cnt));
                SENIS_cobr_gr(l(pt_tx_complete)); {
                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_8BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // Use "+ 1" step for increasing tx_cnt in the commands located
                        // at ph_tx_cnt_inc label
                        SENIS_assign(l(ph_tx_cnt_inc), _SENIS_B_INC_D(tx_cnt));
                        SENIS_assign(l(ph_tx_cnt_inc) + 1, _SENIS_B_NOP());

                        // Perform a "TOBRE" to tx_cnt with 0x0 in the command located
                        // at ph_tx_reload_count label
                        SENIS_assign(l(ph_tx_reload_count) + 1, 0x0);

                        // tx_cnt = (uint32_t)(-tx_cnt - 1);
                        SENIS_tobre(da(tx_cnt), 0xFFFFFFFF);

                        // while (hw_spi_is_busy(id));
                        SNC_hw_spi_wait_while_busy(id);

                        // goto pt_check_tx_cnt;
                        SENIS_goto(l(pt_check_tx_cnt));
                }
                SENIS_label(pt_tx_complete);
                // }

                // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, word_mode);
                SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                        da(&snc_spi_state[id == HW_SPI2].word_mode));
        }
        // } //if

        SENIS_label(pt_return);
#endif /* #if CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

        SENIS_assign(ia(p_out_buf_ind), da(&temp_out_buf[0]));

        // while (hw_spi_is_busy(id));
        SNC_hw_spi_wait_while_busy(id);

        _SNC_TMP_RMV(cur_spi_word_mode);
        _SNC_TMP_RMV(tx_cnt);
        _SNC_TMP_RMV(temp_out_buf);
}

SNC_FUNC_DEF(snc_spi1_transfer_write_ucode, uint32_t out_word_mode,
        uint32_t** out_buf_ind, uint32_t out_len, uint32_t cnt_words)
{
        snc_spi_transfer_write_ucode_impl(b_ctx, HW_SPI1, &SNC_ARG(out_word_mode),
                &SNC_ARG(out_buf_ind), &SNC_ARG(out_len), &SNC_ARG(cnt_words));
}

SNC_FUNC_DEF(snc_spi2_transfer_write_ucode, uint32_t out_word_mode,
        uint32_t** out_buf_ind, uint32_t out_len, uint32_t cnt_words)
{
        snc_spi_transfer_write_ucode_impl(b_ctx, HW_SPI2, &SNC_ARG(out_word_mode),
                &SNC_ARG(out_buf_ind), &SNC_ARG(out_len), &SNC_ARG(cnt_words));
}

static void snc_spi_transfer_read_ucode_impl(b_ctx_t* b_ctx, HW_SPI_ID id,
        uint32_t* p_in_word_mode, uint32_t*** p_in_buf_ind, uint32_t* p_in_len,
        uint32_t* p_cnt_words)
{
        SENIS_labels(
#if CONFIG_SNC_SPI_USE_8BIT_WORD_MODE
                pt_lt_4tx4rx_words_read,
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                pt_4tx4rx_word_mode_is_set,
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
                pt_4tx4rx_check_rx_cnt, pt_4tx4rx_read,
                pt_4tx4rx_check_rx_fifo_not_empty1, pt_4tx4rx_check_rx_fifo_not_empty2,
                pt_4tx4rx_check_rx_fifo_not_empty3, pt_4tx4rx_check_rx_fifo_not_empty4,
                ph_4tx4rx_rx_reload_count,
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */

#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                pt_2tx2rx_set_cnt_words, pt_2tx2rx_word_mode_is_set,
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
                pt_2tx2rx_check_rx_cnt, pt_2tx2rx_read, ph_2tx2rx_rx_cnt_inc,
                pt_2tx2rx_check_rx_fifo_not_empty1, pt_2tx2rx_check_rx_fifo_not_empty2,
                ph_2tx2rx_rx_reload_count,

                pt_1tx1rx_words_read,
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */

#if CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                pt_1tx1rx_set_cnt_words, pt_1tx1rx_word_mode_is_set,
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                pt_1tx1rx_check_rx_cnt, pt_1tx1rx_read_tx, pt_1tx1rx_read_rx, ph_1tx1rx_rx_cnt_inc,
                pt_1tx1rx_check_rx_fifo_not_empty,

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                ph_1tx1rx_rx_reload_count, pt_1tx1rx_reload_count,
                pt_1tx1rx_rx_complete,
                pt_return,
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

                pt_exit
        );

        // if (in_len == 0) return;
        SENIS_rdcgr(da(&snc_const[1]), da(p_in_len));
        SENIS_cobr_gr(l(pt_exit));

        // hw_spi_change_fifo_mode(id, HW_SPI_FIFO_RX_TX);
        SNC_hw_spi_change_fifo_mode(id, SNC_HW_SPI_FIFO_RX_TX);

        // Static variables in SNC context
#if CONFIG_SNC_SPI_USE_8BIT_WORD_MODE
        _SNC_STATIC(uint32_t, const_spi_word_8bit, sizeof(uint32_t), HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
        _SNC_STATIC(uint32_t, const_spi_word_16bit, sizeof(uint32_t), HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */

        // Non-static local variables in SNC context
        _SNC_TMP_ADD(uint32_t, cur_spi_word_mode, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, rx_cnt, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, word_pending_to_read, sizeof(uint32_t));

        SENIS_wadad(da(&temp_in_buf[0]), ia(p_in_buf_ind));

        // rx_cnt = in_len;
        SENIS_wadad(da(rx_cnt), da(p_in_len));

        // Initially set rx_cnt to increase by 1 (i.e. counting 1 word at a time) in 1tx1rx read mode
        SENIS_assign(l(ph_1tx1rx_rx_cnt_inc), _SENIS_B_INC_D(rx_cnt));
        SENIS_assign(l(ph_1tx1rx_rx_cnt_inc) + 1, _SENIS_B_NOP());

        // word_pending_to_read = 0;
        SENIS_assign(da(word_pending_to_read), 0);

#if CONFIG_SNC_SPI_USE_8BIT_WORD_MODE
        // if (in_word_mode <= HW_SPI_WORD_8BIT) {
        SENIS_rdcgr(da(p_in_word_mode), da(const_spi_word_8bit));
        SENIS_cobr_gr(l(pt_lt_4tx4rx_words_read)); {
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                // if (!cnt_words) {
                SENIS_rdcbi(da(p_cnt_words), 0);
                SENIS_cobr_eq(l(pt_4tx4rx_word_mode_is_set));
                { // Change SPI word size mode
                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_8BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                da(&snc_spi_state[id == HW_SPI2].word_mode));

                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_8BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // Perform a "TOBRE" to rx_cnt with 0x0 in the command located
                        // at ph_1tx1rx_rx_reload_count label
                        SENIS_assign(l(ph_1tx1rx_rx_reload_count) + 1, 0x0 + 0x1);
                }
                SENIS_label(pt_4tx4rx_word_mode_is_set);
                // } //if
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

                // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                // word_pending_to_read = 1;
                SENIS_assign(da(word_pending_to_read), 1);

                // rx_cnt = (uint32_t)(-in_len - 1);
                SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);
                // rx_cnt += 1;
                SENIS_inc1(da(rx_cnt));

                // Perform a "TOBRE" to rx_cnt with 0x3 in the command located
                // at ph_4tx4rx_rx_reload_count label (in order to count 8bit words)
                SENIS_assign(l(ph_4tx4rx_rx_reload_count) + 1, 0xFFFFFFFF ^ 0x3);

                // while (rx_cnt > 0) {
                SENIS_goto(l(pt_4tx4rx_check_rx_cnt));
                SENIS_label(pt_4tx4rx_read); {
                        // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                        // while (hw_spi_is_rx_fifo_empty(id));
                        SENIS_label(pt_4tx4rx_check_rx_fifo_not_empty1);
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_4tx4rx_check_rx_fifo_not_empty1));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        // while (hw_spi_is_rx_fifo_empty(id));
                        SENIS_label(pt_4tx4rx_check_rx_fifo_not_empty2);
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_4tx4rx_check_rx_fifo_not_empty2));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        // while (hw_spi_is_rx_fifo_empty(id));
                        SENIS_label(pt_4tx4rx_check_rx_fifo_not_empty3);
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_4tx4rx_check_rx_fifo_not_empty3));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        // while (hw_spi_is_rx_fifo_empty(id));
                        SENIS_label(pt_4tx4rx_check_rx_fifo_not_empty4);
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_4tx4rx_check_rx_fifo_not_empty4));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        SENIS_label(pt_4tx4rx_check_rx_cnt);
                        // rx_cnt += 4;
                        SENIS_inc4(da(rx_cnt));
                        //
                        SENIS_rdcbi(da(rx_cnt), 31);
                        SENIS_cobr_eq(l(pt_4tx4rx_read));
                }
                // } //while

                SENIS_label(ph_4tx4rx_rx_reload_count);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(rx_cnt)), SNC_PLACE_HOLDER);

                // goto pt_1tx1rx_check_rx_cnt:
                SENIS_goto(l(pt_1tx1rx_check_rx_cnt));
        }
        // else
        SENIS_label(pt_lt_4tx4rx_words_read);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
        // if (in_word_mode <= HW_SPI_WORD_16BIT) {
        SENIS_rdcgr(da(p_in_word_mode), da(const_spi_word_16bit));
        SENIS_cobr_gr(l(pt_1tx1rx_words_read)); {
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                // if (!cnt_words) {
                SENIS_rdcbi(da(p_cnt_words), 0);
                SENIS_cobr_eq(l(pt_2tx2rx_set_cnt_words));
                { // Change SPI word size mode
                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_8BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                da(&snc_spi_state[id == HW_SPI2].word_mode));

                        // if (in_len < 2) goto pt_1tx1rx_reload_count;
                        SENIS_rdcgr(da(&snc_const[2]), da(p_in_len));
                        SENIS_cobr_gr(l(pt_1tx1rx_reload_count));

                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_16BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // Set rx_cnt to increase by 2 (i.e. counting 2 bytes at a time) in 1tx1rx read mode
                        SENIS_assign(l(ph_1tx1rx_rx_cnt_inc) + 1, _SENIS_B_INC_D(rx_cnt));

                        // Perform a "TOBRE" to rx_cnt with 0x1 in the command located
                        // at ph_1tx1rx_rx_reload_count label
                        SENIS_assign(l(ph_1tx1rx_rx_reload_count) + 1, 0x1 + 0x2);

                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_16BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                ((HW_SPI_WORD_16BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // rx_cnt = (uint32_t)(-in_len - 1);
                        SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);
                        // rc_cnt += 2;
                        SENIS_inc1(da(rx_cnt));
                        SENIS_inc1(da(rx_cnt));

                        // Set rx_cnt to increase by 4 (i.e. counting 4 bytes at a time)
                        SENIS_assign(l(ph_2tx2rx_rx_cnt_inc), _SENIS_B_INC4_D(rx_cnt));
                        SENIS_assign(l(ph_2tx2rx_rx_cnt_inc) + 1, _SENIS_B_NOP());

                        // Perform a "TOBRE" to rx_cnt with 0x3 in the command located
                        // at ph_2tx2rx_rx_reload_count label (in order to count bytes)
                        SENIS_assign(l(ph_2tx2rx_rx_reload_count) + 1, 0xFFFFFFFF ^ 0x3);
                } SENIS_goto(l(pt_2tx2rx_word_mode_is_set));
                // } else {
                SENIS_label(pt_2tx2rx_set_cnt_words);
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
                {
                        // rx_cnt = (uint32_t)(-in_len - 1);
                        SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);
                        // rc_cnt += 1;
                        SENIS_inc1(da(rx_cnt));

                        // Set rx_cnt to increase by 2 (i.e. counting 2 words at a time)
                        SENIS_assign(l(ph_2tx2rx_rx_cnt_inc), _SENIS_B_INC_D(rx_cnt));
                        SENIS_assign(l(ph_2tx2rx_rx_cnt_inc) + 1, _SENIS_B_INC_D(rx_cnt));

                        // Perform a "TOBRE" to rx_cnt with 0x1 in the command located
                        // at ph_2tx2rx_rx_reload_count label (in order to count 16bit words)
                        SENIS_assign(l(ph_2tx2rx_rx_reload_count) + 1, 0xFFFFFFFF ^ 0x1);
                }
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                SENIS_label(pt_2tx2rx_word_mode_is_set);
                // } //if-else
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

                // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                // word_pending_to_read = 1;
                SENIS_assign(da(word_pending_to_read), 1);

                // while (rx_cnt > 0) {
                SENIS_goto(l(pt_2tx2rx_check_rx_cnt));
                SENIS_label(pt_2tx2rx_read); {
                        // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                        SENIS_label(pt_2tx2rx_check_rx_fifo_not_empty1);
                        // while (hw_spi_is_rx_fifo_empty(id));
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_2tx2rx_check_rx_fifo_not_empty1));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        SENIS_label(pt_2tx2rx_check_rx_fifo_not_empty2);
                        // while (hw_spi_is_rx_fifo_empty(id));
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_2tx2rx_check_rx_fifo_not_empty2));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        SENIS_label(pt_2tx2rx_check_rx_cnt);
                        // rx_cnt += N; // where N = 2 or 4, depending on the RX counter mode
                        SENIS_label(ph_2tx2rx_rx_cnt_inc);
                        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                        //
                        SENIS_rdcbi(da(rx_cnt), 31);
                        SENIS_cobr_eq(l(pt_2tx2rx_read));
                }
                // } //while

                SENIS_label(ph_2tx2rx_rx_reload_count);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(rx_cnt)), SNC_PLACE_HOLDER);

                // goto pt_1tx1rx_check_rx_cnt;
                SENIS_goto(l(pt_1tx1rx_check_rx_cnt));
        }
        // else {
        SENIS_label(pt_1tx1rx_words_read);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
        {
#if CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                // if (!cnt_words) {
                SENIS_rdcbi(da(p_cnt_words), 0);
                SENIS_cobr_eq(l(pt_1tx1rx_set_cnt_words));
                { // Change SPI word size mode
                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_8BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                da(&snc_spi_state[id == HW_SPI2].word_mode));

                        // if (in_len < 4) goto pt_1tx1rx_reload_count;
                        SENIS_rdcgr(da(&snc_const[4]), da(p_in_len));
                        SENIS_cobr_gr(l(pt_1tx1rx_reload_count));

                        // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                        SENIS_assign(da(cur_spi_word_mode),
                                ((HW_SPI_WORD_32BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // Set rx_cnt to increase by 4 (i.e. counting 4 bytes at a time) in 1tx1rx read mode
                        SENIS_assign(l(ph_1tx1rx_rx_cnt_inc), _SENIS_B_INC4_D(rx_cnt));

                        // Perform a "TOBRE" to rx_cnt with 0x3 in the command located
                        // at ph_1tx1rx_rx_reload_count label
                        SENIS_assign(l(ph_1tx1rx_rx_reload_count) + 1, 0x3 + 0x4);

                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_32BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                ((HW_SPI_WORD_32BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                        // rx_cnt = (uint32_t)(-in_len - 1);
                        SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);
                        // rc_cnt += 4;
                        SENIS_inc4(da(rx_cnt));
                } SENIS_goto(l(pt_1tx1rx_word_mode_is_set));
                // } else {
                SENIS_label(pt_1tx1rx_set_cnt_words);
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
                {
                        // rx_cnt = (uint32_t)(-in_len - 1);
                        SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);
                        // rc_cnt += 1;
                        SENIS_inc1(da(rx_cnt));
                }
#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                SENIS_label(pt_1tx1rx_word_mode_is_set);
                // } //if-else
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

                // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                // word_pending_to_read = 1;
                SENIS_assign(da(word_pending_to_read), 1);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */

                // while (rx_cnt > 0) {
                SENIS_goto(l(pt_1tx1rx_check_rx_cnt));
                SENIS_label(pt_1tx1rx_read_tx); {
                        // uint32_t hw_spi_transfer_read_dummy = 0xFFFFFFFF;
                        // hw_spi_write_word(id, (uint8_t*)&hw_spi_transfer_read_dummy, wordsize);
                        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

                        // pt_1tx1rx_read_rx:
                        SENIS_label(pt_1tx1rx_read_rx);

                        SENIS_label(pt_1tx1rx_check_rx_fifo_not_empty);
                        // while (hw_spi_is_rx_fifo_empty(id));
                        SNC_hw_spi_is_rx_fifo_empty(id);
                        SENIS_cobr_eq(l(pt_1tx1rx_check_rx_fifo_not_empty));

                        // hw_spi_read_word(id, in_buf, wordsize);
                        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
                        // in_buf++;
                        SENIS_inc4(da(&temp_in_buf[0]));

                        // pt_1tx1rx_check_rx_cnt:
                        SENIS_label(pt_1tx1rx_check_rx_cnt);
                        // rx_cnt += N; // where N = 1, 2 or 4, depending on the RX counter mode
                        SENIS_label(ph_1tx1rx_rx_cnt_inc);
                        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)));
                        //
                        SENIS_rdcbi(da(rx_cnt), 31);
                        SENIS_cobr_eq(l(pt_1tx1rx_read_tx));
                }
                // } //while

                // if (word_pending_to_read) {
                //         word_pending_to_read = 0;
                //         goto pt_1tx1rx_read_rx; // Read the pending word
                SENIS_rdcgr_z(da(word_pending_to_read));
                SENIS_assign(da(word_pending_to_read), 0);
                SENIS_cobr_gr(l(pt_1tx1rx_read_rx));
                // } //if

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
                // if (!cnt_words) {
                SENIS_rdcbi(da(p_cnt_words), 0);
                SENIS_cobr_eq(l(pt_return)); {
                        // rx_cnt ^= M; // where M = 0x0, 0x1 or 0x3, depending on the SPI word size mode
                        //              // indicating the remaining bytes to be read using 8bit word size mode
                        SENIS_label(ph_1tx1rx_rx_reload_count);
                        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(rx_cnt)), SNC_PLACE_HOLDER);

                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, HW_SPI_WORD_8BIT);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG), da(cur_spi_word_mode));

                        // if (rx_cnt > 0) {
                        SENIS_rdcgr(da(&snc_const[1]), da(rx_cnt));
                        SENIS_cobr_gr(l(pt_1tx1rx_rx_complete)); {
                                // pt_reload_count:
                                SENIS_label(pt_1tx1rx_reload_count);

                                // Prepare "TOBRE" value for setting SPI word size mode value in SPI_CTRL_REG
                                SENIS_assign(da(cur_spi_word_mode),
                                        ((HW_SPI_WORD_8BIT << SPI_SPI_CTRL_REG_SPI_WORD_Pos) & SPI_SPI_CTRL_REG_SPI_WORD_Msk));

                                // Set rx_cnt to increase by 1 (i.e. counting 1 byte at a time) in 1tx1rx read mode
                                SENIS_assign(l(ph_1tx1rx_rx_cnt_inc), _SENIS_B_INC_D(rx_cnt));
                                SENIS_assign(l(ph_1tx1rx_rx_cnt_inc) + 1, _SENIS_B_NOP());

                                // Perform a "TOBRE" to rx_cnt with 0x0 in the command located
                                // at ph_1tx1rx_rx_reload_count label
                                SENIS_assign(l(ph_1tx1rx_rx_reload_count) + 1, 0x0);

                                // rx_cnt = (uint32_t)(-rx_cnt - 1);
                                SENIS_tobre(da(rx_cnt), 0xFFFFFFFF);

                                // while (hw_spi_is_busy(id));
                                SNC_hw_spi_wait_while_busy(id);

                                // goto pt_1tx1rx_check_rx_cnt;
                                SENIS_goto(l(pt_1tx1rx_check_rx_cnt));
                        }
                        SENIS_label(pt_1tx1rx_rx_complete);
                        // }

                        // HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_WORD, word_mode);
                        SENIS_xor(da(&SBA(id)->SPI_CTRL_REG),
                                da(&snc_spi_state[id == HW_SPI2].word_mode));
                }
                // } //if
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */
        }
        // } //if-else

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
        SENIS_label(pt_return);
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

        SENIS_assign(ia(p_in_buf_ind), da(&temp_in_buf[0]));

        SENIS_label(pt_exit);

        _SNC_TMP_RMV(word_pending_to_read);
        _SNC_TMP_RMV(rx_cnt);
        _SNC_TMP_RMV(temp_in_buf);
        _SNC_TMP_RMV(cur_spi_word_mode);
}

SNC_FUNC_DEF(snc_spi1_transfer_read_ucode, uint32_t in_word_mode,
        uint32_t** in_buf_ind, uint32_t in_len, uint32_t cnt_words)
{
        snc_spi_transfer_read_ucode_impl(b_ctx, HW_SPI1, &SNC_ARG(in_word_mode),
                &SNC_ARG(in_buf_ind), &SNC_ARG(in_len), &SNC_ARG(cnt_words));
}

SNC_FUNC_DEF(snc_spi2_transfer_read_ucode, uint32_t in_word_mode,
        uint32_t** in_buf_ind, uint32_t in_len, uint32_t cnt_words)
{
        snc_spi_transfer_read_ucode_impl(b_ctx, HW_SPI2, &SNC_ARG(in_word_mode),
                &SNC_ARG(in_buf_ind), &SNC_ARG(in_len), &SNC_ARG(cnt_words));
}

void snc_spi_write(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
        SENIS_OPER_TYPE out_len_type, uint32_t* out_len)
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
                ((conf->id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_spi1_transfer_write_ucode) :
                        SNC_UCODE_CTX(snc_spi2_transfer_write_ucode)),
                2 * 4, _SNC_OP(da(&snc_spi_state[conf->id == HW_SPI2].word_mode)),
                _SNC_OP(applied_out_buf_ind), out_len_type, out_len, _SNC_OP(true));

        _SNC_TMP_RMV(temp_out_buf);
}

void snc_spi_read(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len)
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
                ((conf->id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_spi1_transfer_read_ucode) :
                        SNC_UCODE_CTX(snc_spi2_transfer_read_ucode)),
                2 * 4, _SNC_OP(da(&snc_spi_state[conf->id == HW_SPI2].word_mode)),
                _SNC_OP(applied_in_buf_ind), in_len_type, in_len, _SNC_OP(true));

        _SNC_TMP_RMV(temp_in_buf);
}

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
void snc_spi_write_advanced(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SENIS_OPER_TYPE out_word_mode_type, uint32_t* out_word_mode,
        SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
        SENIS_OPER_TYPE out_len_type, uint32_t* out_len)
{
        uint32_t** applied_out_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(out_buf);
        ASSERT_WARNING((out_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(out_buf)));

        switch (out_word_mode_type) {
        case SENIS_OPER_TYPE_ADDRESS_IA:
                SNC_ASSERT(da(out_word_mode));
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(out_word_mode), NEQ, HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(out_word_mode), NEQ, HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(out_word_mode), NEQ, HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        case SENIS_OPER_TYPE_ADDRESS_DA:
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                SNC_ASSERT(da(out_word_mode), NEQ, HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                SNC_ASSERT(da(out_word_mode), NEQ, HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                SNC_ASSERT(da(out_word_mode), NEQ, HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        case SENIS_OPER_TYPE_VALUE:
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)out_word_mode != HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)out_word_mode != HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)out_word_mode != HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        default:
                ASSERT_WARNING(0);
        }

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));

        if (out_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_out_buf[0]), out_buf);
                applied_out_buf_ind = &temp_out_buf[0];
        } else {
                SNC_ASSERT(da(out_buf));
                applied_out_buf_ind = (uint32_t**)out_buf;
        }

        senis_call(b_ctx,
                ((conf->id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_spi1_transfer_write_ucode) :
                        SNC_UCODE_CTX(snc_spi2_transfer_write_ucode)),
                2 * 4, out_word_mode_type, out_word_mode, _SNC_OP(applied_out_buf_ind),
                out_len_type, out_len, _SNC_OP(false));

        _SNC_TMP_RMV(temp_out_buf);
}
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
void snc_spi_read_advanced(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SENIS_OPER_TYPE in_word_mode_type, uint32_t* in_word_mode,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len)
{
        uint32_t** applied_in_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(in_buf);
        ASSERT_WARNING((in_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(in_buf)));

        switch (in_word_mode_type) {
        case SENIS_OPER_TYPE_ADDRESS_IA:
                SNC_ASSERT(da(in_word_mode));
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(in_word_mode), NEQ, HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(in_word_mode), NEQ, HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                SNC_ASSERT(ia(in_word_mode), NEQ, HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        case SENIS_OPER_TYPE_ADDRESS_DA:
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                SNC_ASSERT(da(in_word_mode), NEQ, HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                SNC_ASSERT(da(in_word_mode), NEQ, HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                SNC_ASSERT(da(in_word_mode), NEQ, HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        case SENIS_OPER_TYPE_VALUE:
#if (CONFIG_SNC_SPI_USE_8BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)in_word_mode != HW_SPI_WORD_8BIT);
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_16BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)in_word_mode != HW_SPI_WORD_16BIT);
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */
#if (CONFIG_SNC_SPI_USE_32BIT_WORD_MODE == 0)
                ASSERT_WARNING((uint32_t)in_word_mode != HW_SPI_WORD_32BIT);
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */
                break;
        default:
                ASSERT_WARNING(0);
        }

        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        if (in_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_in_buf[0]), in_buf);
                applied_in_buf_ind = &temp_in_buf[0];
        } else {
                SNC_ASSERT(da(in_buf));
                applied_in_buf_ind = (uint32_t**)in_buf;
        }

        senis_call(b_ctx,
                ((conf->id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_spi1_transfer_read_ucode) :
                        SNC_UCODE_CTX(snc_spi2_transfer_read_ucode)),
                2 * 4, in_word_mode_type, in_word_mode, _SNC_OP(applied_in_buf_ind),
                in_len_type, in_len, _SNC_OP(false));

        _SNC_TMP_RMV(temp_in_buf);
}
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

SNC_FUNC_DECL(snc_spi1_writeread_buf_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t** in_buf_ind, uint32_t in_len);
SNC_FUNC_DECL(snc_spi2_writeread_buf_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t** in_buf_ind, uint32_t in_len);

void snc_spi_writeread_buf(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
        SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
        SENIS_OPER_TYPE out_len_type, uint32_t* out_len,
        SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
        SENIS_OPER_TYPE in_len_type, uint32_t* in_len)
{
        uint32_t** applied_out_buf_ind;
        uint32_t** applied_in_buf_ind;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((out_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(out_buf)));
        ASSERT_WARNING((in_buf_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(in_buf)));

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        if (out_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_out_buf[0]), out_buf);
                applied_out_buf_ind = &temp_out_buf[0];
        } else {
                applied_out_buf_ind = (uint32_t**)out_buf;
        }

        if (in_buf_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                SENIS_assign(da(&temp_in_buf[0]), in_buf);
                applied_in_buf_ind = &temp_in_buf[0];
        } else {
                applied_in_buf_ind = (uint32_t**)in_buf;
        }

        senis_call(b_ctx,
                ((conf->id == HW_SPI1) ?
                        SNC_UCODE_CTX(snc_spi1_writeread_buf_ucode) :
                        SNC_UCODE_CTX(snc_spi2_writeread_buf_ucode)),
                2 * 4, _SNC_OP(applied_out_buf_ind), out_len_type, out_len,
                _SNC_OP(applied_in_buf_ind), in_len_type, in_len);

        _SNC_TMP_RMV(temp_in_buf);
        _SNC_TMP_RMV(temp_out_buf);
}

static void snc_spi_writeread_buf_ucode_impl(b_ctx_t* b_ctx, HW_SPI_ID id,
        uint32_t*** p_out_buf_ind, uint32_t* p_out_len, uint32_t*** p_in_buf_ind, uint32_t* p_in_len)
{
        SENIS_labels(
                blk1_for_begin, blk1_for_cond, blk1_for_end,
                blk2_while_begin, blk2_while_end,
                blk3_for_begin, blk3_for_cond, blk3_for_end,
                blk4_while_begin, blk4_while_end
        );

        _SNC_TMP_ADD(uint32_t*, temp_out_buf, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_in_buf, sizeof(uint32_t*));

        SENIS_assign(da(&temp_out_buf[0]), ia(p_out_buf_ind));
        SENIS_assign(da(&temp_in_buf[0]), ia(p_in_buf_ind));

        //write
        SENIS_tobre(da(p_out_len), 0xFFFFFFFF);

        SENIS_goto(l(blk1_for_cond));

        SENIS_label(blk1_for_begin);

        SENIS_wadad(da(&SBA(id)->SPI_RX_TX_REG), ia(&temp_out_buf[0]));

        SENIS_label(blk2_while_begin);

        SENIS_rdcbi(da(&SBA(id)->SPI_CTRL_REG), 13);
        SENIS_cobr_eq(l(blk2_while_end));
        SENIS_goto(l(blk2_while_begin));

        SENIS_label(blk2_while_end);

        SENIS_wadva(da(&SBA(id)->SPI_CLEAR_INT_REG), 1);
        SENIS_inc4(da(&temp_out_buf[0]));

        SENIS_label(blk1_for_cond);

        SENIS_inc1(da(p_out_len));

        SENIS_rdcgr_z(da(p_out_len));
        SENIS_cobr_gr(l(blk1_for_begin));

        SENIS_label(blk1_for_end);

        //read
        SENIS_tobre(da(p_in_len), 0xFFFFFFFF);

        SENIS_goto(l(blk3_for_cond));

        SENIS_label(blk3_for_begin);

        SENIS_wadva(da(&SBA(id)->SPI_RX_TX_REG), 0xFFFFFFFF);

        SENIS_label(blk4_while_begin);

        SENIS_rdcbi(da(&SBA(id)->SPI_CTRL_REG), 13);
        SENIS_cobr_eq(l(blk4_while_end));
        SENIS_goto(l(blk4_while_begin));

        SENIS_label(blk4_while_end);

        SENIS_wadva(da(&SBA(id)->SPI_CLEAR_INT_REG), 1);
        SENIS_wadad(ia(&temp_in_buf[0]), da(&SBA(id)->SPI_RX_TX_REG));
        SENIS_inc4(da(&temp_in_buf[0]));

        SENIS_label(blk3_for_cond);

        SENIS_inc1(da(p_in_len));

        SENIS_rdcgr_z(da(p_in_len));
        SENIS_cobr_gr(l(blk3_for_begin));

        SENIS_label(blk3_for_end);

        SENIS_assign(ia(p_out_buf_ind), da(&temp_out_buf[0]));
        SENIS_assign(ia(p_in_buf_ind), da(&temp_in_buf[0]));

        _SNC_TMP_RMV(temp_in_buf);
        _SNC_TMP_RMV(temp_out_buf);
}

SNC_FUNC_DEF(snc_spi1_writeread_buf_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t** in_buf_ind, uint32_t in_len)
{
        snc_spi_writeread_buf_ucode_impl(b_ctx, HW_SPI1, &SNC_ARG(out_buf_ind),
                &SNC_ARG(out_len), &SNC_ARG(in_buf_ind), &SNC_ARG(in_len));
}

SNC_FUNC_DEF(snc_spi2_writeread_buf_ucode, uint32_t** out_buf_ind, uint32_t out_len,
        uint32_t** in_buf_ind, uint32_t in_len)
{
        snc_spi_writeread_buf_ucode_impl(b_ctx, HW_SPI2, &SNC_ARG(out_buf_ind),
                &SNC_ARG(out_len), &SNC_ARG(in_buf_ind), &SNC_ARG(in_len));
}

#endif /* dg_configUSE_SNC_HW_SPI */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
