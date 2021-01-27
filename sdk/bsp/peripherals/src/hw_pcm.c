/**
 ****************************************************************************************
 *
 * @file hw_pcm.c
 *
 * @brief Implementation of the PCM interface Low Level Driver.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_PCM

#include "hw_pcm.h"
#include <hw_clk.h>

#define FDIV_1_2 (0x2)       /* 1/2 */
#define FDIV_1_3 (0x4)       /* 1/3 */
#define FDIV_2_3 (0x6)       /* 2/3 */
#define FDIV_1_4 (0x8)       /* 1/4 */
#define FDIV_3_4 (0xE)       /* 3/4 */
#define FDIV_1_5 (0x10)      /* 1/5 */
#define FDIV_2_5 (0x11)      /* 2/5 */
#define FDIV_5_6 (0x3E)      /* 5/6 */
#define FDIV_1_7 (0x40)      /* 1/7 */
#define FDIV_2_7 (0x41)      /* 2/7 */
#define FDIV_3_7 (0x49)      /* 3/7 */
#define FDIV_4_7 (0x4B)      /* 4/7 */
#define FDIV_5_7 (0x57)      /* 5/7 */
#define FDIV_6_7 (0x5F)      /* 6/7 */
#define FDIV_5_8 (0x8F)      /* 5/8 */
#define FDIV_7_8 (0xFE)      /* 7/8 */
#define FDIV_3_9 (0x111)     /* 3/9 */
#define FDIV_7_9 (0x13F)     /* 7/9 */
#define FDIV_8_9 (0x1FE)     /* 8/9 */

typedef struct {
        uint16_t bit_clock;
        uint16_t div_fractional;
        uint16_t fdiv_fractional;
        uint16_t div_integer_only;
} hw_pcm_divisors_t;

/* integer and fractional clock divisors for XTAL32M and different bit clocks */
static const hw_pcm_divisors_t divisors_xtal32[] = {
        {   64, 500,        0, 500},
        {  128, 250,        0, 250},
        {  192, 166, FDIV_2_3, 160},
        {  256, 125,        0, 125},
        {  320, 100,        0, 100},
        {  384,  83, FDIV_1_3,  80},
        {  448,  71, FDIV_3_7,  50},
        {  512,  62, FDIV_1_2,  50},
        {  640,  50,        0,  50},
        {  768,  41, FDIV_2_3,  40},
        {  896,  35, FDIV_5_7,  25},
        { 1024,  31, FDIV_1_4,  25},
        { 1152,  27, FDIV_7_9,   0},
        { 1280,  25,        0,  25},
        { 1536,  20, FDIV_5_6,  20},
        { 1792,  17, FDIV_6_7,  10},
        { 1920,  16, FDIV_2_3,   0},
        { 2048,  15, FDIV_5_8,  10},
        { 1152,  27, FDIV_7_9,   0},
        { 2304,  13, FDIV_8_9,   0},
        { 2560,  12, FDIV_1_2,  10},
        { 3072,  10, FDIV_2_5,   0},
        { 6144,  5,  FDIV_1_5,   0},
};

/* integer and fractional clock divisors for PLL96 and different bit clocks */
static const hw_pcm_divisors_t divisors_pll96[] = {
        {   64, 1500,        0, 1500},
        {  128,  750,        0,  750},
        {  192,  500,        0,  500},
        {  256,  375,        0,  375},
        {  320,  300,        0,  300},
        {  384,  250,        0,  250},
        {  448,   71, FDIV_2_7,  200},
        {  512,  187, FDIV_1_2,  150},
        {  640,  150,        0,  150},
        {  768,  125,        0,  125},
        {  896,  107, FDIV_1_7,  100},
        { 1024,   93, FDIV_3_4,   75},
        { 1152,   83, FDIV_3_9,   80},
        { 1280,   75,        0,   25},
        { 1536,   62, FDIV_1_2,   50},
        { 1792,   53, FDIV_4_7,   50},
        { 1920,   50,        0,   50},
        { 2048,   46, FDIV_7_8,   25},
        { 1152,   83, FDIV_1_3,   80},
        { 2304,   41, FDIV_2_3,   40},
        { 2560,   37, FDIV_1_2,   30},
        { 3072,   31, FDIV_1_4,   25},
        { 6144,   15, FDIV_5_8,   12},
};

void hw_pcm_init_clk_reg(HW_PCM_CLOCK clock, uint8_t sample_rate, uint8_t bit_num, uint8_t channel_num,
                                                                          HW_PCM_CLK_GENERATION div)
{
        const hw_pcm_divisors_t *pdiv;
        uint8_t i;
        uint16_t bit_clock;
        uint8_t bit_clock_num_max =  sizeof(divisors_xtal32) / sizeof(divisors_xtal32[0]);

        /* verify sample rate is supported */
        ASSERT_WARNING((sample_rate == 8)  || (sample_rate == 16) ||
                     (sample_rate == 32) || (sample_rate == 48) || (sample_rate == 96));

        /* verify bits number is supported */
        ASSERT_WARNING((bit_num == 8) || (bit_num == 16) || (bit_num == 24) || (bit_num == 32));

        /* verify total number of channels */
        ASSERT_WARNING((channel_num != 0) && (channel_num <= 8));

        if (clock == HW_PCM_CLK_DIV1) {
                /*  DIV1 clock is used so make sure system clock is set to PLL96 */
                ASSERT_WARNING(hw_clk_get_sysclk() == SYS_CLK_IS_PLL);
                REG_SET_BIT(CRG_PER, PCM_DIV_REG, PCM_SRC_SEL);
                pdiv = divisors_pll96;
        } else {
                /* DIVN clock used (XTAL32M) */
                REG_CLR_BIT(CRG_PER, PCM_DIV_REG, PCM_SRC_SEL);
                /* integer only option for sample rate 48ksps and 96ksps is not supported */
                if (div == HW_PCM_CLK_GEN_INTEGER_ONLY) {
                        ASSERT_WARNING(sample_rate != 48 && sample_rate != 96);
                }
                pdiv = divisors_xtal32;
        }

        /* is the bit clock supported? */
        bit_clock = sample_rate * bit_num * channel_num;
        for (i = 0; i < bit_clock_num_max; i++) {
                if (pdiv[i].bit_clock == bit_clock) {
                        break;
                }
        }

        ASSERT_WARNING(i != bit_clock_num_max);

        if (div == HW_PCM_CLK_GEN_FRACTIONAL) {
                REG_SETF(CRG_PER, PCM_DIV_REG, PCM_DIV, pdiv[i].div_fractional);
                REG_SETF(CRG_PER, PCM_FDIV_REG, PCM_FDIV, pdiv[i].fdiv_fractional);
        } else {
                /* HW_PCM_CLK_GEN_INTEGER_ONLY */
                ASSERT_WARNING(((hw_clk_get_sysclk() == SYS_CLK_IS_PLL ?
                                         dg_configPLL96M_FREQ : dg_configXTAL32M_FREQ) /
                                         pdiv[i].div_integer_only % sample_rate ) == 0);
                REG_SETF(CRG_PER, PCM_DIV_REG, PCM_DIV, pdiv[i].div_integer_only);
                REG_SETF(CRG_PER, PCM_FDIV_REG, PCM_FDIV, 0);
        }
}

static void hw_pcm_init_generic_pcm(hw_pcm_config_generic_pcm_t *config)
{
        /* Set channel delay in multiples of 8 bits */
        hw_pcm_set_channel_delay(config->channel_delay);

        /* Set the number of clock cycles per data bit */
        hw_pcm_set_clk_per_bit(config->cycle_per_bit);

        /* Set polarity of PCM FSC */
        hw_pcm_set_fsc_polarity(config->fsc_polarity);

        /* Set polarity of PCM CLK */
        hw_pcm_set_clk_polarity(config->clock_polarity);

        hw_pcm_set_fsc_delay(config->fsc_delay);

        /* FSC length */
        hw_pcm_set_fsc_length(config->fsc_length);

        /* Set PCM edge */
        hw_pcm_set_fsc_edge((config->fsc_length == 0) ? HW_PCM_FSC_EDGE_RISING :
                                                                HW_PCM_FSC_EDGE_RISING_AND_FALLING);

        /* For 2 clock cycles per bit fsc_div must be even */
        ASSERT_WARNING((config->cycle_per_bit == HW_PCM_ONE_CYCLE_PER_BIT) ||
                    ((config->cycle_per_bit == HW_PCM_TWO_CYCLE_PER_BIT) &&
                    ((config->fsc_div % 2) == 0)));

        /* Set PCM FSC divider */
        hw_pcm_set_fsc_div(config->fsc_div);

        /* Generic PCM configuration done successfully */
}

static void hw_pcm_init_i2s(hw_pcm_config_i2s_mode_t *config)
{
        hw_pcm_set_channel_delay(0);

        /* Set PCM edge */
        hw_pcm_set_fsc_edge(HW_PCM_FSC_EDGE_RISING_AND_FALLING);

        /* Set the number of clock cycles per data bit */
        hw_pcm_set_clk_per_bit(config->cycle_per_bit);

        /* Set polarity of PCM FSC */
        hw_pcm_set_fsc_polarity(HW_PCM_FSC_POLARITY_NORMAL);

        /* Set polarity of PCM CLK */
        hw_pcm_set_clk_polarity(HW_PCM_CLK_POLARITY_INVERTED);

        hw_pcm_set_fsc_delay(HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT);

        /* FSC length */
        hw_pcm_set_fsc_length(config->fsc_length);

        /* Set PCM FSC divider */
        hw_pcm_set_fsc_div(config->fsc_div);
}

static void hw_pcm_set_init_tdm(hw_pcm_config_tdm_mode_t *config)
{
        hw_pcm_set_channel_delay(config->channel_delay);

        /* Set PCM edge */
        hw_pcm_set_fsc_edge(HW_PCM_FSC_EDGE_RISING_AND_FALLING);

        /* Set the number of clock cycles per data bit */
        hw_pcm_set_clk_per_bit(config->cycle_per_bit);

        /* Set polarity of PCM FSC */
        hw_pcm_set_fsc_polarity(config->fsc_polarity);

        /* Set polarity of PCM CLK */
        hw_pcm_set_clk_polarity(config->clock_polarity);

        hw_pcm_set_fsc_delay(HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT);

        /* FSC length */
        hw_pcm_set_fsc_length(config->fsc_length);

        /* For 2 clock cycles per bit fsc_div must be even */
        ASSERT_WARNING((config->cycle_per_bit == HW_PCM_ONE_CYCLE_PER_BIT) ||
                    ((config->cycle_per_bit == HW_PCM_TWO_CYCLE_PER_BIT) &&
                    ((config->fsc_div % 2) == 0)));

        /* Set PCM FSC divider */
        hw_pcm_set_fsc_div(config->fsc_div);
}

static void hw_pcm_set_init_iom(hw_pcm_config_iom_mode_t *config)
{
        hw_pcm_set_channel_delay(0);

        /* Set PCM edge */
        hw_pcm_set_fsc_edge(HW_PCM_FSC_EDGE_RISING);

        /* Set the number of clock cycles per data bit */
        hw_pcm_set_clk_per_bit(HW_PCM_TWO_CYCLE_PER_BIT);

        /* Set polarity of PCM FSC */
        hw_pcm_set_fsc_polarity(config->fsc_polarity);

        /* Set polarity of PCM CLK */
        hw_pcm_set_clk_polarity(config->clock_polarity);

        hw_pcm_set_fsc_delay(HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT);

        /* FSC length */
        hw_pcm_set_fsc_length(0);

        /* For 2 clock cycles per bit fsc_div must be even */
        ASSERT_WARNING((config->fsc_div % 2) == 0);

        /* Set PCM Frame synchronization divider */
        hw_pcm_set_fsc_div(config->fsc_div);
}

void hw_pcm_init(hw_pcm_config_t *config)
{
        /* Disable PCM */
        hw_pcm_disable();

        /* Write zero value to output registers to force the unused channels to zero */
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_1, 0);
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_2, 0);

        hw_pcm_set_output_mode(config->gpio_output_mode);

        /* Set PCM in Master Mode */
        hw_pcm_set_mode(config->pcm_mode);

        switch (config->config_mode) {
        case HW_PCM_CONFIG_GENERIC_PCM_MODE:
                hw_pcm_init_generic_pcm(&config->pcm_param);
                break;
        case HW_PCM_CONFIG_I2S_MODE:
                hw_pcm_init_i2s(&config->i2s_param);
                break;
        case HW_PCM_CONFIG_TDM_MODE:
                hw_pcm_set_init_tdm(&config->tdm_param);
                break;
        case HW_PCM_CONFIG_IOM_MODE:
                hw_pcm_set_init_iom(&config->iom_param);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

#endif /* dg_configUSE_HW_PCM */
