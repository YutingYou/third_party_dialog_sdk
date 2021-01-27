/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_GPADC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpadc.c
 *
 * @brief SNC-Implementation of GPADC Low Level Driver
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPADC

#include "snc_defs.h"
#include "snc_hw_sys.h"
#include "snc_hw_gpadc.h"
#include "ad_gpadc.h"
#include "hw_gpadc.h"

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_hw_gpadc_init(b_ctx_t* b_ctx, const gpadc_config* cfg);
/**
 * \brief Function used in SNC context to initialize the GPADC peripheral
 *
 * \param [in] cfg              (gpadc_config*: build-time-only value)
 *                              pointer to the GPADC Low-Level Driver configuration structure
 */
#define SNC_hw_gpadc_init(cfg)                                                                  \
        snc_hw_gpadc_init(b_ctx, _SNC_OP_VALUE(const gpadc_config*, cfg))

static void snc_hw_gpadc_disable(b_ctx_t* b_ctx);
/**
 * \brief Function used in SNC context to disable the GPADC peripheral
 *
 */
#define SNC_hw_gpadc_disable()                                                                  \
        snc_hw_gpadc_disable(b_ctx)


static void snc_hw_gpadc_in_progress(b_ctx_t* b_ctx);
/**
 * \brief Function used in SNC context to check if conversion is in progress
 *
 */
#define SNC_hw_gpadc_in_progress()                                                                  \
        snc_hw_gpadc_in_progress(b_ctx)
//==================== Configuration functions =================================

static void snc_hw_gpadc_init(b_ctx_t* b_ctx, const gpadc_config* cfg)
{
        SENIS_wadva(da(&GPADC->GP_ADC_CTRL_REG),
                (((cfg->temp_sensor & 0x04) >> 2)
                                      << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_EN)) |
                ((cfg->temp_sensor << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_SEL))
                                      & REG_MSK(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_SEL)) |
                (cfg->input << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_SEL)) |
                (cfg->clock << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_CLK_SEL)) |
                (cfg->input_mode << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE)) |
                (cfg->chopping << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_CHOP)) |
                (1 << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN)));

        SENIS_wadva(da(&GPADC->GP_ADC_CTRL2_REG),
                (cfg->sample_time << REG_POS(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME)) |
                (cfg->input_attenuator << REG_POS(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN3X)) |
                (cfg->oversampling << REG_POS(GPADC, GP_ADC_CTRL2_REG, GP_ADC_CONV_NRS)));

        SENIS_wadva(da(&GPADC->GP_ADC_CTRL3_REG),
                (cfg->interval << REG_POS(GPADC, GP_ADC_CTRL3_REG, GP_ADC_INTERVAL)) |
                (0x40 << REG_POS(GPADC, GP_ADC_CTRL3_REG, GP_ADC_EN_DEL)));
}

static void snc_hw_gpadc_disable(b_ctx_t* b_ctx)
{
        SENIS_wadva(da(&GPADC->GP_ADC_CTRL_REG), 0);
}

static void snc_hw_gpadc_in_progress(b_ctx_t* b_ctx)
{
        SENIS_labels(check_rdy);

        SENIS_label(check_rdy);

        SENIS_rdcbi(da(&GPADC->GP_ADC_CTRL_REG), REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_START));

        SENIS_cobr_eq(l(check_rdy));
}

//==================== Peripheral Acquisition functions ========================

void snc_gpadc_open(b_ctx_t* b_ctx, const snc_gpadc_config* conf)
{
        ASSERT_WARNING(b_ctx);
        // GPADC peripheral Initialization / Acquisition
        SNC_hw_sys_bsr_acquire(BSR_PERIPH_ID_GPADC);

        /* Implementing the following DS note:
         * Before making any changes to the ADC settings,
         * Continuous mode must be disabled by setting bit GP_ADC_CONT to 0,
         * while waiting until bit GP_ADC_START = 0.
         */

        SENIS_labels(gp_adc_cont_mode_disable, gp_adc_init);

        SENIS_rdcbi(da(&GPADC->GP_ADC_CTRL_REG), REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT));
        SENIS_cobr_eq(l(gp_adc_cont_mode_disable));
        SENIS_goto(l(gp_adc_init));

        SENIS_label(gp_adc_cont_mode_disable);
        SENIS_xor(da(&GPADC->GP_ADC_CTRL_REG), 1 << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT));

        SNC_hw_gpadc_in_progress();

        SENIS_label(gp_adc_init);

        SNC_hw_gpadc_init(conf->drv);
}

void snc_gpadc_close(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);
        SNC_hw_gpadc_disable();
        SNC_hw_sys_bsr_release(BSR_PERIPH_ID_GPADC);
}

//==================== Peripheral Access functions =============================

void snc_gpadc_read(b_ctx_t* b_ctx, SENIS_OPER_TYPE value_type, uint32_t* value)
{
        ASSERT_WARNING(b_ctx);

        SENIS_xor(da(&GPADC->GP_ADC_CTRL_REG), 1 << REG_POS(GPADC, GP_ADC_CTRL_REG, GP_ADC_START));

        SNC_hw_gpadc_in_progress();
        senis_assign(b_ctx, value_type, value, _SNC_OP(da(&GPADC->GP_ADC_RESULT_REG)));
}

#endif /* dg_configUSE_SNC_HW_GPADC */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
