/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_CHARGER Hardware Charger
 * \{
 * \brief Hardware charger low level functions
 */

/**
 ****************************************************************************************
 *
 * @file hw_charger_da1469x.h
 *
 * @brief Definition of API for the HW Charger Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_CHARGER_DA1469x_H_
#define HW_CHARGER_DA1469x_H_

#if dg_configUSE_HW_CHARGER

#include "sdk_defs.h"

/************************************** Helper macros *********************************************/

#define __HW_CHARGER_FSM_IRQ_OK(_irq_)                  HW_CHARGER_FSM_IRQ_OK_##_irq_
#define HW_CHARGER_FSM_IRQ_OK_MASK(_irq_)               CHARGER_CHARGER_STATE_IRQ_MASK_REG_##_irq_##_IRQ_EN_Msk
#define HW_CHARGER_FSM_IRQ_OK_ENUM(_irq_)                __HW_CHARGER_FSM_IRQ_OK(_irq_) = HW_CHARGER_FSM_IRQ_OK_MASK(_irq_)

#define __HW_CHARGER_FSM_IRQ_NOK(_irq_)                 HW_CHARGER_FSM_IRQ_NOK_##_irq_
#define HW_CHARGER_FSM_IRQ_NOK_MASK(_irq_)              CHARGER_CHARGER_ERROR_IRQ_MASK_REG_##_irq_##_IRQ_EN_Msk
#define HW_CHARGER_FSM_IRQ_NOK_ENUM(_irq_)              __HW_CHARGER_FSM_IRQ_NOK(_irq_) = HW_CHARGER_FSM_IRQ_NOK_MASK(_irq_)

#define __HW_CHARGER_FSM_IRQ_STAT_OK(_irq_)             HW_CHARGER_FSM_IRQ_STAT_OK_##_irq_
#define HW_CHARGER_FSM_IRQ_STAT_OK_MASK(_irq_)          CHARGER_CHARGER_STATE_IRQ_STATUS_REG_##_irq_##_IRQ_Msk
#define HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(_irq_)          __HW_CHARGER_FSM_IRQ_STAT_OK(_irq_) = HW_CHARGER_FSM_IRQ_STAT_OK_MASK(_irq_)

#define __HW_CHARGER_FSM_IRQ_STAT_NOK(_irq_)            HW_CHARGER_FSM_IRQ_STAT_NOK_##_irq_
#define HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(_irq_)         CHARGER_CHARGER_ERROR_IRQ_STATUS_REG_##_irq_##_IRQ_Msk
#define HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(_irq_)         __HW_CHARGER_FSM_IRQ_STAT_NOK(_irq_) = HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(_irq_)

#define HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(_irq_)         CHARGER_CHARGER_STATE_IRQ_CLR_REG_##_irq_##_IRQ_CLR_Msk
#define HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(_irq_)        CHARGER_CHARGER_ERROR_IRQ_CLR_REG_##_irq_##_IRQ_CLR_Msk

/************************************** Function like macros **************************************/

/**
 * \brief Clear non error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_CLEAR_IRQ_OK(_irq_)                                      \
do {                                                                            \
        REG_SET_BIT(CHARGER, CHARGER_STATE_IRQ_CLR_REG, _irq_##_IRQ_CLR);       \
} while (0)

/**
 * \brief Clear error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_CLEAR_IRQ_NOK(_irq_)                                     \
do {                                                                            \
        REG_SET_BIT(CHARGER, CHARGER_ERROR_IRQ_CLR_REG, _irq_##_IRQ_CLR);       \
} while (0)

/**
 * \brief Enable non error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_ENABLE_IRQ_OK(_irq_)                                     \
do {                                                                            \
        REG_SET_BIT(CHARGER, CHARGER_FSM_IRQ_MASK_REG, _irq_##_IRQ_EN_Msk);     \
} while (0)

/**
 * \brief Disable non error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_DISABLE_IRQ_OK(_irq_)                                    \
do {                                                                            \
        REG_CLR_BIT(CHARGER, CHARGER_FSM_IRQ_MASK_REG, _irq_##_IRQ_EN_Msk);     \
} while (0)

/**
 * \brief Enable error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_ENABLE_IRQ_NOK(_irq_)                                    \
do {                                                                            \
        REG_SET_BIT(CHARGER, CHARGER_ERROR_IRQ_MASK_REG, _irq_##_IRQ_EN_Msk);   \
} while (0)

/**
 * \brief Disable error case IRQ's.
 *
 * \param [in] \_irq\_ IRQ
 *
 */
#define HW_CHARGER_FSM_DISABLE_IRQ_NOK(_irq_)                                   \
do {                                                                            \
        REG_CLR_BIT(CHARGER, CHARGER_ERROR_IRQ_MASK_REG, _irq_##_IRQ_EN_Msk);   \
} while (0)

/************************************** Charger control enumerations ******************************/

/**
 * \enum HW_CHARGER_TBAT_MONITOR_MODE
 * \brief Tbat monitor mode.
 */
typedef enum {
        HW_CHARGER_TBAT_MONITOR_MODE_NON_PERIODIC = 0,  /**< Tbat is checked only once during charger's  powering-up and settling. */
        HW_CHARGER_TBAT_MONITOR_MODE_PERIODIC_FSM_ON,   /**< Periodical check of Tbat. JEITA values are updated. Main HW FSM must be enabled. */
        HW_CHARGER_TBAT_MONITOR_MODE_PERIODIC_FSM_OFF,  /**< Periodical check of Tbat. JEITA values are updated. Enabling main HW FSM is not needed. */
        HW_CHARGER_TBAT_MONITOR_MODE_FREEZE_FSM         /**< Tbat HW FSM is frozen. */
} HW_CHARGER_TBAT_MONITOR_MODE;

/************************************** Voltage enumerations **************************************/

/**
 * \enum HW_CHARGER_V_LEVEL
 * \brief The charge voltage levels.
 */
typedef enum {
        HW_CHARGER_V_LEVEL_2800 = 0,    /**< 2.80V */
        HW_CHARGER_V_LEVEL_2850,        /**< 2.85V */
        HW_CHARGER_V_LEVEL_2900,        /**< 2.90V */
        HW_CHARGER_V_LEVEL_2950,        /**< 2.95V */
        HW_CHARGER_V_LEVEL_3000,        /**< 3.00V */
        HW_CHARGER_V_LEVEL_3050,        /**< 3.05V */
        HW_CHARGER_V_LEVEL_3100,        /**< 3.10V */
        HW_CHARGER_V_LEVEL_3150,        /**< 3.15V */
        HW_CHARGER_V_LEVEL_3200,        /**< 3.20V */
        HW_CHARGER_V_LEVEL_3250,        /**< 3.25V */
        HW_CHARGER_V_LEVEL_3300,        /**< 3.30V */
        HW_CHARGER_V_LEVEL_3350,        /**< 3.35V */
        HW_CHARGER_V_LEVEL_3400,        /**< 3.40V */
        HW_CHARGER_V_LEVEL_3450,        /**< 3.45V */
        HW_CHARGER_V_LEVEL_3500,        /**< 3.50V */
        HW_CHARGER_V_LEVEL_3550,        /**< 3.55V */
        HW_CHARGER_V_LEVEL_3600,        /**< 3.60V */
        HW_CHARGER_V_LEVEL_3650,        /**< 3.65V */
        HW_CHARGER_V_LEVEL_3700,        /**< 3.70V */
        HW_CHARGER_V_LEVEL_3750,        /**< 3.75V */
        HW_CHARGER_V_LEVEL_3800,        /**< 3.80V */
        HW_CHARGER_V_LEVEL_3820,        /**< 3.82V */
        HW_CHARGER_V_LEVEL_3840,        /**< 3.84V */
        HW_CHARGER_V_LEVEL_3860,        /**< 3.86V */
        HW_CHARGER_V_LEVEL_3880,        /**< 3.88V */
        HW_CHARGER_V_LEVEL_3900,        /**< 3.90V */
        HW_CHARGER_V_LEVEL_3920,        /**< 3.92V */
        HW_CHARGER_V_LEVEL_3940,        /**< 3.94V */
        HW_CHARGER_V_LEVEL_3960,        /**< 3.96V */
        HW_CHARGER_V_LEVEL_3980,        /**< 3.98V */
        HW_CHARGER_V_LEVEL_4000,        /**< 4.00V */
        HW_CHARGER_V_LEVEL_4020,        /**< 4.02V */
        HW_CHARGER_V_LEVEL_4040,        /**< 4.04V */
        HW_CHARGER_V_LEVEL_4060,        /**< 4.06V */
        HW_CHARGER_V_LEVEL_4080,        /**< 4.08V */
        HW_CHARGER_V_LEVEL_4100,        /**< 4.10V */
        HW_CHARGER_V_LEVEL_4120,        /**< 4.12V */
        HW_CHARGER_V_LEVEL_4140,        /**< 4.14V */
        HW_CHARGER_V_LEVEL_4160,        /**< 4.16V */
        HW_CHARGER_V_LEVEL_4180,        /**< 4.18V */
        HW_CHARGER_V_LEVEL_4200,        /**< 4.20V */
        HW_CHARGER_V_LEVEL_4220,        /**< 4.22V */
        HW_CHARGER_V_LEVEL_4240,        /**< 4.24V */
        HW_CHARGER_V_LEVEL_4260,        /**< 4.26V */
        HW_CHARGER_V_LEVEL_4280,        /**< 4.28V */
        HW_CHARGER_V_LEVEL_4300,        /**< 4.30V */
        HW_CHARGER_V_LEVEL_4320,        /**< 4.32V */
        HW_CHARGER_V_LEVEL_4340,        /**< 4.34V */
        HW_CHARGER_V_LEVEL_4360,        /**< 4.36V */
        HW_CHARGER_V_LEVEL_4380,        /**< 4.38V */
        HW_CHARGER_V_LEVEL_4400,        /**< 4.40V */
        HW_CHARGER_V_LEVEL_4420,        /**< 4.42V */
        HW_CHARGER_V_LEVEL_4440,        /**< 4.44V */
        HW_CHARGER_V_LEVEL_4460,        /**< 4.46V */
        HW_CHARGER_V_LEVEL_4480,        /**< 4.48V */
        HW_CHARGER_V_LEVEL_4500,        /**< 4.50V */
        HW_CHARGER_V_LEVEL_4520,        /**< 4.52V */
        HW_CHARGER_V_LEVEL_4540,        /**< 4.54V */
        HW_CHARGER_V_LEVEL_4560,        /**< 4.56V */
        HW_CHARGER_V_LEVEL_4580,        /**< 4.58V */
        HW_CHARGER_V_LEVEL_4600,        /**< 4.60V */
        HW_CHARGER_V_LEVEL_4700,        /**< 4.70V */
        HW_CHARGER_V_LEVEL_4800,        /**< 4.80V */
        HW_CHARGER_V_LEVEL_4900         /**< 4.90V */
} HW_CHARGER_V_LEVEL;

/************************************** Current enumerations **************************************/

/** \enum HW_CHARGER_I_EOC_PERCENT_LEVEL
 *  \brief The current percentage level at which the battery is considered charged.
 *
 *  If exceeded by Ibat the HW FSM continues to the end of charge state.
 */
typedef enum {
        HW_CHARGER_I_EOC_PERCENT_LEVEL_4 = 0,   /**<   4.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_5_5,     /**<   5.5 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_7,       /**<   7.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_8_5,     /**<   8.5 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_10,      /**<  10.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_12,      /**<  12.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_14,      /**<  14.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_16,      /**<  16.0 % */

        /* Double range settings */

        HW_CHARGER_I_EOC_PERCENT_LEVEL_8_8,     /**<   8.8 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_12_1,    /**<  12.1 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_15_4,    /**<  15.4 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_18_7,    /**<  18.7 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_22,      /**<  22.0 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_26_4,    /**<  26.4 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_30_8,    /**<  30.8 % */
        HW_CHARGER_I_EOC_PERCENT_LEVEL_35_2     /**<  35.2 % */
} HW_CHARGER_I_EOC_PERCENT_LEVEL;

/** \enum HW_CHARGER_I_PRECHARGE_LEVEL
 *  \brief The charge current level during pre-charging.
 */
typedef enum {
        HW_CHARGER_I_PRECHARGE_LEVEL_0_5 = 0,   /**<  0.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_1_0,       /**<  1.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_1_5,       /**<  1.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_2_0,       /**<  2.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_2_5,       /**<  2.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_3_0,       /**<  3.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_3_5,       /**<  3.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_4_0,       /**<  4.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_4_5,       /**<  4.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_5_0,       /**<  5.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_5_5,       /**<  5.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_6_0,       /**<  6.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_6_5,       /**<  6.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_7_0,       /**<  7.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_7_5,       /**<  7.5mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_8_0,       /**<  8.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_9_0,       /**<  9.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_10,        /**< 10.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_11,        /**< 11.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_12,        /**< 12.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_13,        /**< 13.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_14,        /**< 14.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_15,        /**< 15.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_16,        /**< 16.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_17,        /**< 17.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_18,        /**< 18.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_19,        /**< 19.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_20,        /**< 20.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_21,        /**< 21.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_22,        /**< 22.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_23,        /**< 23.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_24,        /**< 24.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_26,        /**< 26.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_28,        /**< 28.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_30,        /**< 30.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_32,        /**< 32.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_34,        /**< 34.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_36,        /**< 36.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_38,        /**< 38.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_40,        /**< 40.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_42,        /**< 42.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_44,        /**< 44.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_46,        /**< 46.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_48,        /**< 48.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_50,        /**< 50.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_52,        /**< 52.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_54,        /**< 54.0mA */
        HW_CHARGER_I_PRECHARGE_LEVEL_56         /**< 56.0mA */
} HW_CHARGER_I_PRECHARGE_LEVEL;

/**
 * \enum HW_CHARGER_I_LEVEL
 * \brief The charge current levels.
 */
typedef enum {
        HW_CHARGER_I_LEVEL_5 = 0,       /**<   5mA */
        HW_CHARGER_I_LEVEL_10,          /**<  10mA */
        HW_CHARGER_I_LEVEL_15,          /**<  15mA */
        HW_CHARGER_I_LEVEL_20,          /**<  20mA */
        HW_CHARGER_I_LEVEL_25,          /**<  25mA */
        HW_CHARGER_I_LEVEL_30,          /**<  30mA */
        HW_CHARGER_I_LEVEL_35,          /**<  35mA */
        HW_CHARGER_I_LEVEL_40,          /**<  40mA */
        HW_CHARGER_I_LEVEL_45,          /**<  45mA */
        HW_CHARGER_I_LEVEL_50,          /**<  50mA */
        HW_CHARGER_I_LEVEL_55,          /**<  55mA */
        HW_CHARGER_I_LEVEL_60,          /**<  60mA */
        HW_CHARGER_I_LEVEL_65,          /**<  65mA */
        HW_CHARGER_I_LEVEL_70,          /**<  70mA */
        HW_CHARGER_I_LEVEL_75,          /**<  75mA */
        HW_CHARGER_I_LEVEL_80,          /**<  80mA */
        HW_CHARGER_I_LEVEL_90,          /**<  90mA */
        HW_CHARGER_I_LEVEL_100,         /**< 100mA */
        HW_CHARGER_I_LEVEL_110,         /**< 110mA */
        HW_CHARGER_I_LEVEL_120,         /**< 120mA */
        HW_CHARGER_I_LEVEL_130,         /**< 130mA */
        HW_CHARGER_I_LEVEL_140,         /**< 140mA */
        HW_CHARGER_I_LEVEL_150,         /**< 150mA */
        HW_CHARGER_I_LEVEL_160,         /**< 160mA */
        HW_CHARGER_I_LEVEL_170,         /**< 170mA */
        HW_CHARGER_I_LEVEL_180,         /**< 180mA */
        HW_CHARGER_I_LEVEL_190,         /**< 190mA */
        HW_CHARGER_I_LEVEL_200,         /**< 200mA */
        HW_CHARGER_I_LEVEL_210,         /**< 210mA */
        HW_CHARGER_I_LEVEL_220,         /**< 220mA */
        HW_CHARGER_I_LEVEL_230,         /**< 230mA */
        HW_CHARGER_I_LEVEL_240,         /**< 240mA */
        HW_CHARGER_I_LEVEL_260,         /**< 260mA */
        HW_CHARGER_I_LEVEL_280,         /**< 280mA */
        HW_CHARGER_I_LEVEL_300,         /**< 300mA */
        HW_CHARGER_I_LEVEL_320,         /**< 320mA */
        HW_CHARGER_I_LEVEL_340,         /**< 340mA */
        HW_CHARGER_I_LEVEL_360,         /**< 360mA */
        HW_CHARGER_I_LEVEL_380,         /**< 380mA */
        HW_CHARGER_I_LEVEL_400,         /**< 400mA */
        HW_CHARGER_I_LEVEL_420,         /**< 420mA */
        HW_CHARGER_I_LEVEL_440,         /**< 440mA */
        HW_CHARGER_I_LEVEL_460,         /**< 460mA */
        HW_CHARGER_I_LEVEL_480,         /**< 480mA */
        HW_CHARGER_I_LEVEL_500,         /**< 500mA */
        HW_CHARGER_I_LEVEL_520,         /**< 520mA */
        HW_CHARGER_I_LEVEL_540,         /**< 540mA */
        HW_CHARGER_I_LEVEL_560          /**< 560mA */
} HW_CHARGER_I_LEVEL;

/*********************************** Temperature enumerations *************************************/

/**
 * \enum HW_CHARGER_DIE_TEMP_LIMIT
 * \brief The die temperature limit.
 *
 * Charging will be automatically disabled if temperature
 * exceeds the limit and resumed if temperature is few degrees below it.
 */
typedef enum {
        HW_CHARGER_DIE_TEMP_LIMIT_0 = 0,        /**<   0C (mainly for test purposes)*/
        HW_CHARGER_DIE_TEMP_LIMIT_50,           /**<  50C */
        HW_CHARGER_DIE_TEMP_LIMIT_80,           /**<  80C */
        HW_CHARGER_DIE_TEMP_LIMIT_90,           /**<  90C */
        HW_CHARGER_DIE_TEMP_LIMIT_100,          /**< 100C */
        HW_CHARGER_DIE_TEMP_LIMIT_110,          /**< 110C */
        HW_CHARGER_DIE_TEMP_LIMIT_120,          /**< 120C */
        HW_CHARGER_DIE_TEMP_LIMIT_130           /**< 130C */
} HW_CHARGER_DIE_TEMP_LIMIT;

/**
 * \enum HW_CHARGER_BAT_TEMP_LIMIT
 * \brief The battery temperature limit to define the "COLD", "COOL", "NORMAL", "WARM", "HOT" regions.
 *
 */
typedef enum {
        HW_CHARGER_BAT_TEMP_LIMIT_M10 = 0,      /**< -10C */
        HW_CHARGER_BAT_TEMP_LIMIT_M9,           /**<  -9C */
        HW_CHARGER_BAT_TEMP_LIMIT_M8,           /**<  -8C */
        HW_CHARGER_BAT_TEMP_LIMIT_M7,           /**<  -7C */
        HW_CHARGER_BAT_TEMP_LIMIT_M6,           /**<  -6C */
        HW_CHARGER_BAT_TEMP_LIMIT_M5,           /**<  -5C */
        HW_CHARGER_BAT_TEMP_LIMIT_M4,           /**<  -4C */
        HW_CHARGER_BAT_TEMP_LIMIT_M3,           /**<  -3C */
        HW_CHARGER_BAT_TEMP_LIMIT_M2,           /**<  -2C */
        HW_CHARGER_BAT_TEMP_LIMIT_M1,           /**<  -1C */
        HW_CHARGER_BAT_TEMP_LIMIT_0,            /**<   0C */
        HW_CHARGER_BAT_TEMP_LIMIT_1,            /**<   1C */
        HW_CHARGER_BAT_TEMP_LIMIT_2,            /**<   2C */
        HW_CHARGER_BAT_TEMP_LIMIT_3,            /**<   3C */
        HW_CHARGER_BAT_TEMP_LIMIT_4,            /**<   4C */
        HW_CHARGER_BAT_TEMP_LIMIT_5,            /**<   5C */
        HW_CHARGER_BAT_TEMP_LIMIT_6,            /**<   6C */
        HW_CHARGER_BAT_TEMP_LIMIT_7,            /**<   7C */
        HW_CHARGER_BAT_TEMP_LIMIT_8,            /**<   8C */
        HW_CHARGER_BAT_TEMP_LIMIT_9,            /**<   9C */
        HW_CHARGER_BAT_TEMP_LIMIT_10,           /**<  10C */
        HW_CHARGER_BAT_TEMP_LIMIT_11,           /**<  11C */
        HW_CHARGER_BAT_TEMP_LIMIT_12,           /**<  12C */
        HW_CHARGER_BAT_TEMP_LIMIT_13,           /**<  13C */
        HW_CHARGER_BAT_TEMP_LIMIT_14,           /**<  14C */
        HW_CHARGER_BAT_TEMP_LIMIT_15,           /**<  15C */
        HW_CHARGER_BAT_TEMP_LIMIT_16,           /**<  16C */
        HW_CHARGER_BAT_TEMP_LIMIT_17,           /**<  17C */
        HW_CHARGER_BAT_TEMP_LIMIT_18,           /**<  18C */
        HW_CHARGER_BAT_TEMP_LIMIT_19,           /**<  19C */
        HW_CHARGER_BAT_TEMP_LIMIT_20,           /**<  20C */
        HW_CHARGER_BAT_TEMP_LIMIT_21,           /**<  21C */
        HW_CHARGER_BAT_TEMP_LIMIT_22,           /**<  22C */
        HW_CHARGER_BAT_TEMP_LIMIT_23,           /**<  23C */
        HW_CHARGER_BAT_TEMP_LIMIT_24,           /**<  24C */
        HW_CHARGER_BAT_TEMP_LIMIT_25,           /**<  25C */
        HW_CHARGER_BAT_TEMP_LIMIT_26,           /**<  26C */
        HW_CHARGER_BAT_TEMP_LIMIT_27,           /**<  27C */
        HW_CHARGER_BAT_TEMP_LIMIT_28,           /**<  28C */
        HW_CHARGER_BAT_TEMP_LIMIT_29,           /**<  29C */
        HW_CHARGER_BAT_TEMP_LIMIT_30,           /**<  30C */
        HW_CHARGER_BAT_TEMP_LIMIT_31,           /**<  31C */
        HW_CHARGER_BAT_TEMP_LIMIT_32,           /**<  32C */
        HW_CHARGER_BAT_TEMP_LIMIT_33,           /**<  33C */
        HW_CHARGER_BAT_TEMP_LIMIT_34,           /**<  34C */
        HW_CHARGER_BAT_TEMP_LIMIT_35,           /**<  35C */
        HW_CHARGER_BAT_TEMP_LIMIT_36,           /**<  36C */
        HW_CHARGER_BAT_TEMP_LIMIT_37,           /**<  37C */
        HW_CHARGER_BAT_TEMP_LIMIT_38,           /**<  38C */
        HW_CHARGER_BAT_TEMP_LIMIT_39,           /**<  39C */
        HW_CHARGER_BAT_TEMP_LIMIT_40,           /**<  40C */
        HW_CHARGER_BAT_TEMP_LIMIT_41,           /**<  41C */
        HW_CHARGER_BAT_TEMP_LIMIT_42,           /**<  42C */
        HW_CHARGER_BAT_TEMP_LIMIT_43,           /**<  43C */
        HW_CHARGER_BAT_TEMP_LIMIT_44,           /**<  44C */
        HW_CHARGER_BAT_TEMP_LIMIT_45,           /**<  45C */
        HW_CHARGER_BAT_TEMP_LIMIT_46,           /**<  46C */
        HW_CHARGER_BAT_TEMP_LIMIT_47,           /**<  47C */
        HW_CHARGER_BAT_TEMP_LIMIT_48,           /**<  48C */
        HW_CHARGER_BAT_TEMP_LIMIT_49,           /**<  49C */
        HW_CHARGER_BAT_TEMP_LIMIT_50,           /**<  50C */
        HW_CHARGER_BAT_TEMP_LIMIT_51,           /**<  51C */
        HW_CHARGER_BAT_TEMP_LIMIT_52,           /**<  52C */
        HW_CHARGER_BAT_TEMP_LIMIT_53            /**<  53C */
} HW_CHARGER_BAT_TEMP_LIMIT;

/**
 * \enum HW_CHARGER_JEITA_REGION
 * \brief JEITA standard regions.
 *
 *  If battery pack temperature is in the "HOT" region, charging will be stopped. The same will
 *  happen for the case of "COLD" region, unless low temperature operation has been set by
 *  hw_charger_set_bat_low_temp_mode().
 *
 */
typedef enum {
        HW_CHARGER_JEITA_REGION_COLD    = (1 << 0),     /**< Battery temperature below "COLD" level */
        HW_CHARGER_JEITA_REGION_COOL    = (1 << 1),     /**< Battery temperature above "COLD" and below "COOL" level */
        HW_CHARGER_JEITA_REGION_NORMAL  = (1 << 2),     /**< Battery temperature above "COOL" and below "WARM" level */
        HW_CHARGER_JEITA_REGION_WARM    = (1 << 3),     /**< Battery temperature above "WARM" and below "HOT" level */
        HW_CHARGER_JEITA_REGION_HOT     = (1 << 4)      /**< Battery temperature above "HOT" level */
} HW_CHARGER_JEITA_REGION;

/************************************** HW FSM enumerations ***************************************/

/**
 * \enum HW_CHARGER_MAIN_FSM_STATE
 * \brief State of charger's main FSM.
 */
typedef enum {
        HW_CHARGER_MAIN_FSM_STATE_POWER_UP = 0,         /**< Power up state */
        HW_CHARGER_MAIN_FSM_STATE_INIT,                 /**< Initial state */
        HW_CHARGER_MAIN_FSM_STATE_DISABLED,             /**< Disabled state */
        HW_CHARGER_MAIN_FSM_STATE_PRE_CHARGE,           /**< Pre-charge state */
        HW_CHARGER_MAIN_FSM_STATE_CC_CHARGE,            /**< Constant Current state */
        HW_CHARGER_MAIN_FSM_STATE_CV_CHARGE,            /**< Constant Voltage state */
        HW_CHARGER_MAIN_FSM_STATE_END_OF_CHARGE,        /**< End of Charge state */
        HW_CHARGER_MAIN_FSM_STATE_TDIE_PROT,            /**< Die temperature protection state */
        HW_CHARGER_MAIN_FSM_STATE_TBAT_PROT,            /**< Battery temperature protection state */
        HW_CHARGER_MAIN_FSM_STATE_BYPASSED,             /**< Bypass state */
        HW_CHARGER_MAIN_FSM_STATE_ERROR                 /**< Error state */
} HW_CHARGER_MAIN_FSM_STATE;

/**
 * \enum HW_CHARGER_JEITA_FSM_STATE
 * \brief State of charger's JEITA FSM.
 */
typedef enum {
        HW_CHARGER_JEITA_FSM_STATE_IDLE = 0,            /**< Idle state */
        HW_CHARGER_JEITA_FSM_STATE_CHECK_THOT,          /**< Hot state */
        HW_CHARGER_JEITA_FSM_STATE_CHECK_TCOLD,         /**< Cold state */
        HW_CHARGER_JEITA_FSM_STATE_CHECK_TWARM,         /**< Warm state */
        HW_CHARGER_JEITA_FSM_STATE_CHECK_TCOOL,         /**< Cool state */
        HW_CHARGER_JEITA_FSM_STATE_CHECK_TNORMAL,       /**< Normal state */
        HW_CHARGER_JEITA_FSM_STATE_UPDATE_TBAT          /**< Update state */
} HW_CHARGER_JEITA_FSM_STATE;


/************************************** IRQ enumerations ******************************************/

/**
 * \enum HW_CHARGER_FSM_IRQ_OK
 * \brief The masks of the IRQs provided by the HW FSM (non error cases)
 */
typedef enum {
        HW_CHARGER_FSM_IRQ_OK_ENUM(CV_TO_PRECHARGE),            /**< Mask for CV_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(CC_TO_PRECHARGE),            /**< Mask for CC_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(CV_TO_CC),                   /**< Mask for CV_TO_CC */
        HW_CHARGER_FSM_IRQ_OK_ENUM(TBAT_STATUS_UPDATE),         /**< Mask for TBAT_STATUS_UPDATE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(TBAT_PROT_TO_PRECHARGE),     /**< Mask for TBAT_PROT_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(TDIE_PROT_TO_PRECHARGE),     /**< Mask for TDIE_PROT_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(EOC_TO_PRECHARGE),           /**< Mask for EOC_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ENUM(CV_TO_EOC),                  /**< Mask for CV_TO_EOC */
        HW_CHARGER_FSM_IRQ_OK_ENUM(CC_TO_EOC),                  /**< Mask for CC_TO_EOC */
        HW_CHARGER_FSM_IRQ_OK_ENUM(CC_TO_CV),                   /**< Mask for CC_TO_CV */
        HW_CHARGER_FSM_IRQ_OK_ENUM(PRECHARGE_TO_CC),            /**< Mask for PRECHARGE_TO_CC */
        HW_CHARGER_FSM_IRQ_OK_ENUM(DISABLED_TO_PRECHARGE),      /**< Mask for DISABLED_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_OK_ALL =                             /**< Mask for all non error case IRQ's */
                (HW_CHARGER_FSM_IRQ_OK_MASK(CV_TO_PRECHARGE)            |
                 HW_CHARGER_FSM_IRQ_OK_MASK(CC_TO_PRECHARGE)            |
                 HW_CHARGER_FSM_IRQ_OK_MASK(CV_TO_CC)                   |
                 HW_CHARGER_FSM_IRQ_OK_MASK(TBAT_STATUS_UPDATE)         |
                 HW_CHARGER_FSM_IRQ_OK_MASK(TBAT_PROT_TO_PRECHARGE)     |
                 HW_CHARGER_FSM_IRQ_OK_MASK(TDIE_PROT_TO_PRECHARGE)     |
                 HW_CHARGER_FSM_IRQ_OK_MASK(EOC_TO_PRECHARGE)           |
                 HW_CHARGER_FSM_IRQ_OK_MASK(CV_TO_EOC)                  |
                 HW_CHARGER_FSM_IRQ_OK_MASK(CC_TO_EOC)                  |
                 HW_CHARGER_FSM_IRQ_OK_MASK(CC_TO_CV)                   |
                 HW_CHARGER_FSM_IRQ_OK_MASK(PRECHARGE_TO_CC)            |
                 HW_CHARGER_FSM_IRQ_OK_MASK(DISABLED_TO_PRECHARGE))
} HW_CHARGER_FSM_IRQ_OK;

/**
 * \enum HW_CHARGER_FSM_IRQ_NOK
 * \brief The masks of the IRQs provided by the HW FSM (error cases)
 */
typedef enum {
        HW_CHARGER_FSM_IRQ_NOK_ENUM(TBAT_ERROR),                /**< Mask for TBAT_ERROR */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(TDIE_ERROR),                /**< Mask for TDIE_ERROR */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(VBAT_OVP_ERROR),            /**< Mask for VBAT_OVP_ERROR */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(TOTAL_CHARGE_TIMEOUT),      /**< Mask for TOTAL_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(CV_CHARGE_TIMEOUT),         /**< Mask for CV_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(CC_CHARGE_TIMEOUT),         /**< Mask for CC_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_NOK_ENUM(PRECHARGE_TIMEOUT),         /**< Mask for PRECHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_NOK_ALL =                            /**< Mask for all error case IRQ's */
                (HW_CHARGER_FSM_IRQ_NOK_MASK(TBAT_ERROR)                |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(TDIE_ERROR)                |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(VBAT_OVP_ERROR)            |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(TOTAL_CHARGE_TIMEOUT)      |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(CV_CHARGE_TIMEOUT)         |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(CC_CHARGE_TIMEOUT)         |
                 HW_CHARGER_FSM_IRQ_NOK_MASK(PRECHARGE_TIMEOUT))
} HW_CHARGER_FSM_IRQ_NOK;

/**
 * \enum HW_CHARGER_FSM_IRQ_STAT_OK
 * \brief IRQ status bits (non-error cases).
 */
typedef enum {
        HW_CHARGER_FSM_IRQ_STAT_OK_NONE = 0,                            /**< Status for NONE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CV_TO_PRECHARGE),               /**< Status for CV_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CC_TO_PRECHARGE),               /**< Status for CC_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CV_TO_CC),                      /**< Status for CV_TO_CC */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(TBAT_STATUS_UPDATE),            /**< Status for TBAT_STATUS_UPDATE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(TBAT_PROT_TO_PRECHARGE),        /**< Status for TBAT_PROT_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(TDIE_PROT_TO_PRECHARGE),        /**< Status for TDIE_PROT_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(EOC_TO_PRECHARGE),              /**< Status for EOC_TO_PRECHARGE */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CV_TO_EOC),                     /**< Status for CV_TO_EOC */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CC_TO_EOC),                     /**< Status for CC_TO_EOC */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(CC_TO_CV),                      /**< Status for CC_TO_CV */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(PRECHARGE_TO_CC),               /**< Status for PRECHARGE_TO_CC */
        HW_CHARGER_FSM_IRQ_STAT_OK_ENUM(DISABLED_TO_PRECHARGE),         /**< Status for PRECHARGE_TO_CC */
        HW_CHARGER_FSM_IRQ_STAT_OK_ALL =                                /**< Status for all non error case IRQ's */
                (HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CV_TO_PRECHARGE)              |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CC_TO_PRECHARGE)              |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CV_TO_CC)                     |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(TBAT_STATUS_UPDATE)           |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(TBAT_PROT_TO_PRECHARGE)       |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(TDIE_PROT_TO_PRECHARGE)       |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(EOC_TO_PRECHARGE)             |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CV_TO_EOC)                    |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CC_TO_EOC)                    |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(CC_TO_CV)                     |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(PRECHARGE_TO_CC)              |
                 HW_CHARGER_FSM_IRQ_STAT_OK_MASK(DISABLED_TO_PRECHARGE))
} HW_CHARGER_FSM_IRQ_STAT_OK;

/**
 * \enum HW_CHARGER_FSM_IRQ_STAT_NOK
 * \brief IRQ status bits (error cases).
 */
typedef enum {
        HW_CHARGER_FSM_IRQ_STAT_NOK_NONE = 0,                           /**< Status for NONE */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(TBAT_ERROR),                   /**< Status for TBAT_ERROR */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(TDIE_ERROR),                   /**< Status for TDIE_ERROR */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(VBAT_OVP_ERROR),               /**< Status for VBAT_OVP_ERROR */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(TOTAL_CHARGE_TIMEOUT),         /**< Status for TOTAL_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(CV_CHARGE_TIMEOUT),            /**< Status for CV_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(CC_CHARGE_TIMEOUT),            /**< Status for CC_CHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ENUM(PRECHARGE_TIMEOUT),            /**< Status for PRECHARGE_TIMEOUT */
        HW_CHARGER_FSM_IRQ_STAT_NOK_ALL =                               /**< Status for all error case IRQ's  */
                (HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TBAT_ERROR)           |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TDIE_ERROR)           |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(VBAT_OVP_ERROR)       |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TOTAL_CHARGE_TIMEOUT) |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CV_CHARGE_TIMEOUT)    |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CC_CHARGE_TIMEOUT)    |
                 HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(PRECHARGE_TIMEOUT))
} HW_CHARGER_FSM_IRQ_STAT_NOK;

/**************************** Charging profile control enumerations *******************************/

/**
 * \enum HW_CHARGER_CTRL
 * \brief Charger control bits.
 */
typedef enum {
        HW_CHARGER_CTRL_ENABLE_DIE_TEMP_PROTECTION                      = (1 << 0),     /**< Enable die temperature protection. */
        HW_CHARGER_CTRL_ENABLE_BAT_TEMP_PROTECTION                      = (1 << 1),     /**< Enable battery temperature protection. */
        HW_CHARGER_CTRL_HALT_CHARGE_TIMERS_ON_TEMP_PROTECTION_STATES    = (1 << 2),     /**< Halt charging timers on temperature protection states. */
        HW_CHARGER_CTRL_ENABLE_BAT_LOW_TEMP                             = (1 << 3),     /**< Enable charging on low battery temperatures. */
        HW_CHARGER_CTRL_RESUME_FROM_DIE_PROTECTION_STATE                = (1 << 4),     /**< Enable resuming from die protection protection state. */
        HW_CHARGER_CTRL_RESUME_FROM_ERROR_STATE                         = (1 << 5),     /**< Enable resuming from error state. */
        HW_CHARGER_CTRL_ENABLE_JEITA_SUPPORT                            = (1 << 6)      /**< Enable JEITA support. */
} HW_CHARGER_CTRL;

/**
 * \brief Charging profile.
 */
typedef struct {
        HW_CHARGER_CTRL                 ctrl_flags;                     /**< Control flags for influencing charging profile. */

        HW_CHARGER_TBAT_MONITOR_MODE    tbat_monitor_mode;              /**< Monitor mode for battery temperature. */

        /* IRQ parameters */

        HW_CHARGER_FSM_IRQ_OK           irq_ok_mask;                    /**< IRQ's the charging profile is interested in (non error cases). */
        HW_CHARGER_FSM_IRQ_NOK          irq_nok_mask;                   /**< IRQ's the charging profile is interested in (error cases). */

        /* Voltage parameters */

        HW_CHARGER_V_LEVEL              ovp_level;                      /**< Over voltage level. */
        HW_CHARGER_V_LEVEL              replenish_v_level;              /**< Replenish voltage level. */
        HW_CHARGER_V_LEVEL              precharged_v_thr;               /**< Pre-charged voltage threshold. */
        HW_CHARGER_V_LEVEL              cv_level;                       /**< Constant Voltage level. */

        HW_CHARGER_V_LEVEL              jeita_ovp_warm_level;           /**< Over voltage level set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_V_LEVEL              jeita_ovp_cool_level;           /**< Over voltage level set by JEITA FSM if Tbat found in cool zone. */
        HW_CHARGER_V_LEVEL              jeita_replenish_v_warm_level;   /**< Replenish voltage level set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_V_LEVEL              jeita_replenish_v_cool_level;   /**< Replenish voltage level set by JEITA FSM if Tbat found in cool zone. */
        HW_CHARGER_V_LEVEL              jeita_precharged_v_warm_thr;    /**< Pre-charged voltage threshold set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_V_LEVEL              jeita_precharged_v_cool_thr;    /**< Pre-charged voltage threshold set by JEITA FSM if Tbat found in cool zone. */
        HW_CHARGER_V_LEVEL              jeita_cv_warm_level;            /**< Constant Voltage level set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_V_LEVEL              jeita_cv_cool_level;            /**< Constant Voltage level set by JEITA FSM if Tbat found in cool zone. */

        /* Current parameters */

        HW_CHARGER_I_EOC_PERCENT_LEVEL  eoc_i_thr;                      /**< End of charge current threshold. */
        HW_CHARGER_I_PRECHARGE_LEVEL    precharge_cc_level;             /**< Pre-charged constant current level. */
        HW_CHARGER_I_LEVEL              cc_level;                       /**< Constant Current level. */

        HW_CHARGER_I_PRECHARGE_LEVEL    jeita_precharge_cc_warm_level;  /**< Pre-charged constant current level set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_I_PRECHARGE_LEVEL    jeita_precharge_cc_cool_level;  /**< Pre-charged constant current level set by JEITA FSM if Tbat found in cool zone. */
        HW_CHARGER_I_LEVEL              jeita_cc_warm_level;            /**< Constant Current level set by JEITA FSM if Tbat found in warm zone. */
        HW_CHARGER_I_LEVEL              jeita_cc_cool_level;            /**< Constant Current level set by JEITA FSM if Tbat found in cool zone. */

        /* Temperature parameters */

        HW_CHARGER_DIE_TEMP_LIMIT       die_temp_limit;                 /**< Die's temperature limit. */

        HW_CHARGER_BAT_TEMP_LIMIT       bat_temp_hot_limit;             /**< Tbat hot limit. */
        HW_CHARGER_BAT_TEMP_LIMIT       bat_temp_warm_limit;            /**< Tbat warm limit. */
        HW_CHARGER_BAT_TEMP_LIMIT       bat_temp_cool_limit;            /**< Tbat cool limit. */
        HW_CHARGER_BAT_TEMP_LIMIT       bat_temp_cold_limit;            /**< Tbat cold limit. */

        /* Charging timeout parameters */

        uint16_t                        max_precharge_timeout;          /**< Timeout for Pre-charge state (in secs). */
        uint16_t                        max_cc_charge_timeout;          /**< Timeout for Constant Current state (in secs). */
        uint16_t                        max_cv_charge_timeout;          /**< Timeout for Constant Voltage state (in secs). */
        uint16_t                        max_total_charge_timeout;       /**< Timeout for total charging (in secs). */
} hw_charger_charging_profile_t;

/**
 * \brief Fine tuning parameters.
 */
typedef struct {
        uint16_t        vbat_comparator_settling_time;          /**< Settling time for Vbat comparator (in usec). */
        uint16_t        ovp_comparator_settling_time;           /**< Settling time for over voltage comparator (in usec). */
        uint16_t        tdie_comparator_settling_time;          /**< Settling time for Tdie comparator (in usec). */
        uint16_t        tbat_comparator_settling_time;          /**< Settling time for Tbat comparator(in usec) */
        uint16_t        tbat_hot_comparator_settling_time;      /**< Settling time for Tbat hot comparator (in usec). */
        uint16_t        tbat_monitoring_time;                   /**< JEITA FSM monitoring interval (in msec). */
        uint16_t        charger_powering_up_time;               /**< Settling time for powering up (in msec). */
        uint16_t        eoc_interval_check_threshold;           /**< End of Charge interval check (in usec). */
} hw_charger_fine_tuning_settings_t ;

/**
 * \brief Charger's callback for non error cases.
 *
 * \param [in] status: IRQ status.
 *
 */
typedef void (*hw_charger_fsm_ok_cb_t)(HW_CHARGER_FSM_IRQ_STAT_OK status);

/**
 * \brief Charger's callback for error cases.
 *
 * \param [in] status: IRQ status.
 *
 */
typedef void (*hw_charger_fsm_nok_cb_t)(HW_CHARGER_FSM_IRQ_STAT_NOK status);

/************************************** Charger's clock parameters ********************************/

/**
 * \brief Set charger's clock mode.
 *
 * When set it enables the clock source for charger's timers.
 *
 * \param [in] mode: Charger's clock mode.
 *
 */
__STATIC_INLINE void hw_charger_set_clock_mode(bool mode)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_SYS, CLK_SYS_REG , CLK_CHG_EN, mode);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get charger's clock mode.
 *
 * \return true: Charger's clock source is enabled.
 * \return false: Charger's clock source is disabled.
 *
 * \note Function has to be called with closed interrupts for thread safeness.
 */
__STATIC_INLINE bool hw_charger_get_clock_mode(void)
{
        return REG_GETF(CRG_SYS, CLK_SYS_REG , CLK_CHG_EN);
}

/************************************** Charger's control parameters ******************************/


/**
 * \brief Set charger's JEITA support mode.
 *
 * When set it allows the handling of charging parameters (voltage, current ) in different temperature zones
 * with finer granularity.
 *
 * \param [in] mode: True enable JEITA support false to disable it.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_support_mode(bool mode)
{
        if (mode && !REG_GETF(CHARGER, CHARGER_CTRL_REG, TBAT_PROT_ENABLE)) {
                /* No point to have JEITA without Tbat protection. */
                ASSERT_WARNING(0);
        }
        REG_SETF(CHARGER, CHARGER_CTRL_REG, JEITA_SUPPORT_DISABLED, !(mode));
}

/**
 * \brief Get charger's JEITA support mode.
 *
 * \return true: JEITA support is enabled.
 * \return false: JEITA support is disabled.
 *
 */
__STATIC_INLINE bool hw_charger_get_jeita_support_mode(void)
{
        return !(REG_GETF(CHARGER, CHARGER_CTRL_REG, JEITA_SUPPORT_DISABLED));
}

/**
 * \brief Set charger's mode for monitoring the battery temperature.
 *
 * \param [in] mode: Charger's monitor mode.
 *
 * \see hw_charger_set_bat_temp_monitor_periodicity_ms()
 *
 */
__STATIC_INLINE void hw_charger_set_bat_temp_monitor_mode(HW_CHARGER_TBAT_MONITOR_MODE mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, TBAT_MONITOR_MODE, mode);
}

/**
 * \brief Get charger's mode for monitoring the battery temperature.
 *
 * \return Charger's monitor mode.
 *
 */
__STATIC_INLINE HW_CHARGER_TBAT_MONITOR_MODE hw_charger_get_bat_temp_monitor_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, TBAT_MONITOR_MODE);
}

/**
 * \brief Set charger's timer's behavior as soon as HW FSM has moved to battery/die temperature protection state.
 *
 * \param [in] mode: True to halt charge related timers on battery/die temperature protection state, false to let them running.
 *
 */
__STATIC_INLINE void hw_charger_halt_timers_on_temp_protection_states(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, CHARGE_TIMERS_HALT_ENABLE, mode);
}

/**
 * \brief Get charger's timer's behavior as soon HW FSM has moved to battery/die protection state.
 *
 * \return true: Halt charge related timers when HW FSM moves to battery/die temperature protection state.
 * \return false: Let charge related timers running when HW FSM moves to battery/die temperature protection state.
 *
 */
__STATIC_INLINE bool hw_charger_get_halt_timers_on_temp_protection_states(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGE_TIMERS_HALT_ENABLE);
}

/**
 * \brief Set charger's resume mode.
 *
 * When set the Charger's FSM returns back to "charging" mode by moving into disabled (default) state.
 *
 * \param [in] mode: True to resume from Error state, false to disable resuming.
 *
 */
__STATIC_INLINE void hw_charger_set_resume_mode(bool mode)
{
        /* Resume mode has no effect if bypass mode is set */
        ASSERT_WARNING(REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGER_BYPASS) == false);
        REG_SETF(CHARGER, CHARGER_CTRL_REG, CHARGER_RESUME, mode);
}

/**
 * \brief Get charger's resume mode.
 *
 * \return true: Resuming from Error state is enabled.
 * \return false: Resuming from Error state is disabled.
 *
 */
__STATIC_INLINE bool hw_charger_get_resume_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGER_RESUME);
}

/**
 * \brief Set charger's bypass mode.
 *
 * When set HW FSM is bypassed and SW is responsible for controlling the charger's state transitions.
 *
 * \param [in] mode: True to let SW control the charger, false to let HW control the charger.
 *
 */
__STATIC_INLINE void hw_charger_set_bypass_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, CHARGER_BYPASS, mode);
}

/**
 * \brief Get charger's bypass mode.
 *
 * \return true: Jump to Bypass state and charger can be controlled by SW.
 * \return false: HW controls the charger.
 *
 */
__STATIC_INLINE bool hw_charger_get_bypass_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGER_BYPASS);
}

/**
 * \brief Set charger's die temperature protection mode.
 *
 * When set, HW FSM will move to die-protection state, disabling charging at the same time if the limit
 * set by hw_charger_set_die_temp_protection_limit() exceeded.
 *
 * \param [in] mode: True to enable charger's die temperature protection, false to disable it.
 *
 */
__STATIC_INLINE void hw_charger_set_die_temp_protection_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, TDIE_PROT_ENABLE, mode);
}

/**
 * \brief Get charger's die temperature protection mode.
 *
 * \return true: Charger's die temperature protection is enabled.
 * \return false: Charger's die temperature protection is disabled.
 *
 */
__STATIC_INLINE bool hw_charger_get_die_temp_protection_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, TDIE_PROT_ENABLE);
}

/**
 * \brief Set charger's operation at low temperature mode.
 *
 * When set, charging the battery at low temperature (even the battery temperature pack reaches the "COLD" region) is allowed.
 * Consequently, the FSM continues charging and no battery temperature error event is generated.
 *
 * \param [in] mode: True to allow operate at low temperature, false to disable charger at low temperature.
 *
 */
__STATIC_INLINE void hw_charger_set_bat_low_temp_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, NTC_LOW_DISABLE, mode);
}

/**
 * \brief Get charger's operation at low temperature.
 *
 * \return true: Charger is allowed to operate at low temperature ("COLD" region).
 * \return false: Charger will get disabled at low temperature ("COLD" region).
 *
 */
__STATIC_INLINE bool hw_charger_get_bat_low_temp_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, NTC_LOW_DISABLE);
}

/**
 * \brief Set charger's battery temperature protection mode.
 *
 * When set, HW FSM will move to battery temperature protection state, if battery's temperature found
 * in the "COLD" or "HOT" region. Charging at the same time is disabled.
 *
 * \param [in] mode: True to enable battery temperature protection, false to disable it.
 *
 */
__STATIC_INLINE void hw_charger_set_bat_temp_protection_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, TBAT_PROT_ENABLE, mode);
}

/**
 * \brief Get charger's battery temperature protection mode.
 *
 * \return true: Battery temperature protection is enabled.
 * \return false: Battery temperature protection is disabled.
 *
 */
__STATIC_INLINE bool hw_charger_get_bat_temp_protection_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, TBAT_PROT_ENABLE);
}

/**
 * \brief Set charger's resuming behavior as soon HW FSM has moved to die temperature protection state.
 *
 * \param [in] mode: True to resume charging given die temperature is below limit, false otherwise.
 *
 */
__STATIC_INLINE void hw_charger_set_resume_behavior_on_die_temp_protection_state(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, TDIE_ERROR_RESUME, mode);
}

/**
 * \brief Get charger's resuming behavior as soon HW FSM has moved to die temperature protection state
 *
 * \return true: HW FSM will move to pre-charge state as soon as die temperature is below limit.
 * \return false: HW FSM will stuck in die temperature protection state even if die temperature is below limit.
 *
 */
__STATIC_INLINE bool hw_charger_get_resume_behavior_on_die_temp_protection_state(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, TDIE_ERROR_RESUME);
}

/**
 * \brief Set charger's HW FSM operating mode.
 *
 * When set, charger's FSM is enabled. FSM's state can move from DISABLED to the actual charging
 * states, starting from pre-charge state.
 *
 * \param [in] mode: True to enable HW FSM, false to disable it.
 *
 */
__STATIC_INLINE void hw_charger_set_fsm_operating_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, CHARGE_START, mode);
}

/**
 * \brief Get charger's HW FSM starting status.
 *
 * \return true: HW FSM is enabled.
 * \return false: HW FSM is disabled.
 *
 */
__STATIC_INLINE bool hw_charger_get_fsm_operating_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGE_START);
}

/**
 * \brief Set charger's analog circuitry operating mode.
 *
 *  Set charger's power up mode.
 *
 * \param [in] mode: True to enable charger's analog circuitry (power up), false to disable it (power down).
 *
 */
__STATIC_INLINE void hw_charger_set_analog_circuitry_operating_mode(bool mode)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, CHARGER_ENABLE, mode);
}

/**
 * \brief Get charger's analog circuitry operating mode.
 *
 * \return true: Charger's analog circuitry powered up. HW FSM in power up state.
 * \return false: Charger's analog circuitry is powered down. HW FSM in power down state.
 *
 */
__STATIC_INLINE  bool hw_charger_get_analog_circuitry_operating_mode(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, CHARGER_ENABLE);
}

/************************************** Charger's status information ******************************/

/**
 * \brief Get the state of charger's JEITA FSM.
 *
 * \return State of charger's JEITA FSM.
 *
 */
__STATIC_INLINE  HW_CHARGER_JEITA_FSM_STATE hw_charger_get_jeita_fsm_state(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, CHARGER_JEITA_STATE);
}

/**
 * \brief Get the state of charger's main FSM.
 *
 * \return State of charger's main FSM.
 *
 */
__STATIC_INLINE  HW_CHARGER_MAIN_FSM_STATE hw_charger_get_main_fsm_state(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, CHARGER_STATE);
}

/**
 * \brief Check if die's temperature protection limit exceeded.
 *
 * \return true: Die's temperature protection limit exceeded. Charging is disabled. HW_FSM will move die-temperature protection state.
 * \return false: Die's temperature is below protection limit. This is the normal operation.
 *
 * \see hw_charger_set_die_temp_protection_mode()
 *
 */
__STATIC_INLINE  bool hw_charger_is_die_temp_protection_limit_exceeded(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, TDIE_COMP_OUT);
}

/**
 * \brief Get JEITA operating region.
 *
 * \return JEITA operating region.
 *
 */
__STATIC_INLINE HW_CHARGER_JEITA_REGION hw_charger_get_jeita_operating_region(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, TBAT_STATUS);
}

/**
 * \brief Check if end of charge has been reached.
 *
 * The actual charge current less than 10% of that set by hw_charger_set_const_current_level()
 *
 * \return true: End of charge has been reached.
 * \return false: End of charge hasn't been reached.
 *
 */
__STATIC_INLINE  bool hw_charger_is_eoc_reached(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, END_OF_CHARGE);
}

/**
 * \brief Check if charger operates in constant voltage mode.
 *
 * \return true: Charger operates in constant voltage mode.
 * \return false: Charger doesn't operate in constant voltage mode or is off
 *
 */
__STATIC_INLINE  bool hw_charger_is_const_voltage_mode_on(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, CHARGER_CV_MODE);
}

/**
 * \brief Check if charger operates in constant current mode.
 *
 * \return true: Charger operates in constant current mode.
 * \return false: Charger doesn't operate in constant current mode or is off
 *
 */
__STATIC_INLINE  bool hw_charger_is_const_current_mode_on(void)
{
        return REG_GETF(CHARGER, CHARGER_STATUS_REG, CHARGER_CC_MODE);
}

/************************************** Charger's voltage parameters ******************************/

/**
 * \brief Set the over voltage protection level.
 *
 *
 * \param [in] ovp_level: Over voltage protection level.
 *
 */
__STATIC_INLINE void hw_charger_set_ovp_level(HW_CHARGER_V_LEVEL ovp_level)
{
        REG_SETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_OVP, ovp_level);
}

/**
 * \brief Get the over voltage protection level.
 *
 * \return Over voltage protection level.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_ovp_level(void)
{
        return REG_GETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_OVP);
}

/**
 * \brief Set the replenish level.
 *
 * This is voltage level that VBAT can drop below V_CHARGE, before battery charging
 * starts again
 *
 * \param [in] replenish_level: Replenish level.
 *
 */
__STATIC_INLINE void hw_charger_set_replenish_level(HW_CHARGER_V_LEVEL replenish_level)
{
        REG_SETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_REPLENISH, replenish_level);
}

/**
 * \brief Get the replenish level.
 *
 * \return Replenish level.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_replenish_level(void)
{
        return REG_GETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_REPLENISH);
}

/**
 * \brief Set the pre-charge voltage threshold.
 *
 * This is voltage level at which the battery is considered pre-charged and the
 * HW FSM will move to constant current state.
 *
 *
 * \param [in] precharged_v_thr: Pre-charge voltage threshold.
 *
 */
__STATIC_INLINE void hw_charger_set_precharged_voltage_threshold(HW_CHARGER_V_LEVEL precharged_v_thr)
{
        REG_SETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_PRECHARGE, precharged_v_thr);
}

/**
 * \brief Get the pre-charge voltage threshold.
 *
 * \return Pre-charge voltage threshold.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_precharged_voltage_threshold(void)
{
        return REG_GETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_PRECHARGE);
}

/**
 * \brief Set the voltage level in constant voltage mode.
 *
 * \param [in] charge_voltage: The nominal constant voltage level.
 *
 */
__STATIC_INLINE void hw_charger_set_const_voltage_level(HW_CHARGER_V_LEVEL charge_voltage)
{
        REG_SETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_CHARGE, charge_voltage);
}

/**
 * \brief Get the voltage level in constant voltage mode.
 *
 * \return Voltage level in constant voltage mode.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_const_voltage_level(void)
{
        return REG_GETF(CHARGER, CHARGER_VOLTAGE_PARAM_REG, V_CHARGE);
}

/**
 * \brief Set the voltage level in constant voltage mode in the "WARM" region.
 *
 * \param [in] charge_voltage: The nominal constant voltage level in the "WARM" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_const_voltage_level(HW_CHARGER_V_LEVEL charge_voltage)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_CHARGE_REG , V_CHARGE_TWARM, charge_voltage);
}

/**
 * \brief Get the voltage level in constant voltage mode in the "WARM" region.
 *
 * \return Voltage level in constant voltage mode in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_warm_const_voltage_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_CHARGE_REG, V_CHARGE_TWARM);
}

/**
 * \brief Set the voltage level in constant voltage mode in the "COOL" region.
 *
 * \param [in] charge_voltage: The nominal constant voltage level in the "COOL" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_const_voltage_level(HW_CHARGER_V_LEVEL charge_voltage)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_CHARGE_REG, V_CHARGE_TCOOL, charge_voltage);
}

/**
 * \brief Get the voltage level in constant voltage mode in the "COOL" region.
 *
 * \return Voltage level in constant voltage mode in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_cool_const_voltage_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_CHARGE_REG, V_CHARGE_TCOOL);
}

/**
 * \brief Set the pre-charge voltage threshold in the "WARM" region.
 *
 * This is voltage level at which the battery is considered pre-charged and the
 * HW FSM will move to constant current state.
 *
 * \param [in] precharged_v_thr: Pre-charge voltage threshold in the "WARM" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_precharged_voltage_threshold(HW_CHARGER_V_LEVEL precharged_v_thr)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_PRECHARGE_REG, V_PRECHARGE_TWARM, precharged_v_thr);
}

/**
 * \brief \brief Get the pre-charge voltage threshold in the "WARM" region.
 *
 * \return Pre-charge voltage threshold in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_warm_precharged_voltage_threshold(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_PRECHARGE_REG, V_PRECHARGE_TWARM);
}

/**
 * \brief Set the pre-charge voltage threshold in the "COOL" region.
 *
 * This is voltage level at which the battery is considered pre-charged and the
 * HW FSM will move to constant current state.
 *
 * \param [in] precharged_v_thr: Pre-charge voltage threshold in the "COOL" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_precharged_voltage_threshold(HW_CHARGER_V_LEVEL precharged_v_thr)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_PRECHARGE_REG, V_PRECHARGE_TCOOL, precharged_v_thr);
}

/**
 * \brief \brief Get the pre-charge voltage threshold in the "COOL" region.
 *
 * \return Pre-charge voltage threshold in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_cool_precharged_voltage_threshold(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_PRECHARGE_REG, V_PRECHARGE_TCOOL);
}


/**
 * \brief Set the replenish level in the "WARM" region.
 *
 * This is voltage level that VBAT can drop below V_CHARGE, before battery charging of the battery
 * starts again
 *
 * \param [in] replenish_level: Replenish level in the "WARM" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_replenish_level(HW_CHARGER_V_LEVEL replenish_level)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_REPLENISH_REG, V_REPLENISH_TWARM, replenish_level);
}

/**
 * \brief Get the replenish level in the "WARM" region.
 *
 * \return Replenish level in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_warm_replenish_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_REPLENISH_REG, V_REPLENISH_TWARM);
}

/**
 * \brief Set the replenish level in the "COOL" region.
 *
 * This is voltage level that VBAT can drop below V_CHARGE, before battery charging of the battery
 * starts again
 *
 * \param [in] replenish_level: Replenish level in the "COOL" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_replenish_level(HW_CHARGER_V_LEVEL replenish_level)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_REPLENISH_REG, V_REPLENISH_TCOOL, replenish_level);
}

/**
 * \brief Get the replenish level in the "COOL" region.
 *
 * \return Replenish level in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_cool_replenish_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_REPLENISH_REG, V_REPLENISH_TCOOL);
}

/**
 * \brief Set the over voltage protection level in the "WARM" region.
 *
 * \param [in] ovp_level: Over voltage protection level in the "WARM" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_ovp_level(HW_CHARGER_V_LEVEL ovp_level)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_OVP_REG, V_OVP_TWARM, ovp_level);
}

/**
 * \brief Get the over voltage protection level in the "WARM" region.
 *
 * \return Over voltage protection level in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_warm_ovp_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_OVP_REG, V_OVP_TWARM);
}

/**
 * \brief Set the over voltage protection level in the "COOL" region.
 *
 * \param [in] ovp_level: Over voltage protection level in the "COOL" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_ovp_level(HW_CHARGER_V_LEVEL ovp_level)
{
        REG_SETF(CHARGER, CHARGER_JEITA_V_OVP_REG, V_OVP_TCOOL, ovp_level);
}

/**
 * \brief Get the over voltage protection level in the "COOL" region.
 *
 * \return Over voltage protection level in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_V_LEVEL hw_charger_get_jeita_cool_ovp_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_V_OVP_REG, V_OVP_TCOOL);
}

/************************************** Charger's current parameters ******************************/

/**
 * \brief Set the end of charge current threshold.
 *
 * This is the current level at which the battery is considered charged and the
 * HW FSM will move to the end-of-charge state.
 *
 *
 * \param [in] eoc_i_thr: End of charge current threshold.
 *
 */
__STATIC_INLINE void hw_charger_set_eoc_current_threshold(HW_CHARGER_I_EOC_PERCENT_LEVEL eoc_i_thr)
{
        /* Check if the threshold value belongs to the double range threshold settings. */
        if (eoc_i_thr < HW_CHARGER_I_EOC_PERCENT_LEVEL_8_8) {
                REG_SETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_END_OF_CHARGE, eoc_i_thr);
        } else {
                REG_SET_BIT(CHARGER, CHARGER_CURRENT_PARAM_REG, I_EOC_DOUBLE_RANGE);
                REG_SETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_END_OF_CHARGE,
                         eoc_i_thr - HW_CHARGER_I_EOC_PERCENT_LEVEL_8_8);
        }
}

/**
 * \brief Get end of charge current threshold.
 *
 * \return End of charge current threshold.
 *
 */
__STATIC_INLINE HW_CHARGER_I_EOC_PERCENT_LEVEL hw_charger_get_eoc_current_threshold(void)
{
        if (REG_GETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_EOC_DOUBLE_RANGE)) {
                return (REG_GETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_END_OF_CHARGE) +
                        HW_CHARGER_I_EOC_PERCENT_LEVEL_8_8);
        } else {
                return REG_GETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_END_OF_CHARGE);
        }
}

/**
 * \brief Set the current level for the pre-charge state.
 *
 * \param [in] charge_current: The nominal constant current level.
 *
 */
__STATIC_INLINE void hw_charger_set_precharge_const_current_level(HW_CHARGER_I_PRECHARGE_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_PRECHARGE, charge_current);
}

/**
 * \brief Get the current level for the pre-charge state.
 *
 * \return Current level in constant current mode.
 *
 */
__STATIC_INLINE HW_CHARGER_I_PRECHARGE_LEVEL hw_charger_get_precharge_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_PRECHARGE);
}

/**
 * \brief Set the current level in constant current mode.
 *
 * \param [in] charge_current: The nominal constant current level.
 *
 */
__STATIC_INLINE void hw_charger_set_const_current_level(HW_CHARGER_I_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_CHARGE, charge_current);
}

/**
 * \brief Get the current level in constant current mode.
 *
 * \return Current level in constant current mode.
 *
 */
__STATIC_INLINE HW_CHARGER_I_LEVEL hw_charger_get_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_CURRENT_PARAM_REG, I_CHARGE);
}


/**
 * \brief Set the current level in constant current mode in the "WARM" region.
 *
 * \param [in] charge_current: The nominal constant current level in the "WARM" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_const_current_level(HW_CHARGER_I_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_CHARGE_TWARM, charge_current);
}

/**
 * \brief Get the current level in constant current mode in the "WARM" region.
 *
 * \return Current level in constant current mode in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_I_LEVEL hw_charger_get_jeita_warm_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_CHARGE_TWARM);
}

/**
 * \brief Set the current level in constant current mode in the "COOL" region.
 *
 * \param [in] charge_current: The nominal constant current level in the "COOL" region.
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_const_current_level(HW_CHARGER_I_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_CHARGE_TCOOL, charge_current);
}

/**
 * \brief Get the current level in constant current mode in the "COOL" region.
 *
 * \return Current level in constant current mode in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_I_LEVEL hw_charger_get_jeita_cool_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_CHARGE_TCOOL);
}

/**
 * \brief Set the current level for the pre-charge state in the "WARM" region.
 *
 * \param [in] charge_current: The nominal constant current level in the "WARM" region.
 *
 */

__STATIC_INLINE void hw_charger_set_jeita_warm_precharge_const_current_level(HW_CHARGER_I_PRECHARGE_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_PRECHARGE_TWARM, charge_current);
}

/**
 * \brief Get the current level for the pre-charge state in the "WARM" region.
 *
 * \return The nominal constant current level in the "WARM" region.
 *
 */
__STATIC_INLINE HW_CHARGER_I_PRECHARGE_LEVEL hw_charger_get_jeita_warm_precharge_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_PRECHARGE_TWARM);
}

/**
 * \brief Set the current level for the pre-charge state in the "COOL" region.
 *
 * \param [in] charge_current: The nominal constant current level in the "COOL" region.
 *
 */

__STATIC_INLINE void hw_charger_set_jeita_cool_precharge_const_current_level(HW_CHARGER_I_PRECHARGE_LEVEL charge_current)
{
        REG_SETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_PRECHARGE_TCOOL, charge_current);
}

/**
 * \brief Get the current level for the pre-charge state in the "COOL" region.
 *
 * \return The nominal constant current level in the "COOL" region.
 *
 */
__STATIC_INLINE HW_CHARGER_I_PRECHARGE_LEVEL hw_charger_get_jeita_cool_precharge_const_current_level(void)
{
        return REG_GETF(CHARGER, CHARGER_JEITA_CURRENT_REG, I_PRECHARGE_TCOOL);
}

#if (dg_configUSE_USB_ENUMERATION == 1)
/**
 * \brief Function to convert charging level to miliAmps.
 *
 * This function is necessary to convert the charging
 * levels to actual mA values. This is necessary in cases
 * where USB enumeration is performed.
 */

uint16_t hw_charger_i_level_to_miliamp(HW_CHARGER_I_LEVEL level);
#endif

/************************************** Charger's temperature parameters **************************/

/**
 * \brief Set die's temperature protection limit.
 *
 * \param [in] die_temp_limit: The die's temperature protection limit.
 *
 * \see hw_charger_set_die_temp_protection()
 *
 */
__STATIC_INLINE void hw_charger_set_die_temp_protection_limit(HW_CHARGER_DIE_TEMP_LIMIT die_temp_limit)
{
        ASSERT_WARNING(die_temp_limit > HW_CHARGER_DIE_TEMP_LIMIT_0);

        REG_SETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TDIE_MAX, die_temp_limit);
}

/**
 * \brief Get die's temperature protection limit.
 *
 * \return Die's temperature protection limit.
 *
 */
__STATIC_INLINE HW_CHARGER_DIE_TEMP_LIMIT hw_charger_get_die_temp_protection_limit(void)
{
        return REG_GETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TDIE_MAX);
}

/**
 * \brief Set battery's hot temperature limit.
 *
 * \param [in] bat_temp_limit: The battery's hot temperature limit.
 *
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_hot_temp_limit(HW_CHARGER_BAT_TEMP_LIMIT bat_temp_limit)
{
        REG_SETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_HOT, bat_temp_limit);
}

/**
 * \brief Get battery's hot temperature limit.
 *
 * \return Battery's hot temperature limit.
 *
 */
__STATIC_INLINE HW_CHARGER_BAT_TEMP_LIMIT hw_charger_get_jeita_hot_temp_limit(void)
{
        return REG_GETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_HOT);
}

/**
 * \brief Set battery's warm temperature limit.
 *
 * \param [in] bat_temp_limit: The battery's warm temperature limit.
 *
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_warm_temp_limit(HW_CHARGER_BAT_TEMP_LIMIT bat_temp_limit)
{
        REG_SETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_WARM, bat_temp_limit);
}

/**
 * \brief Get battery's warm temperature limit.
 *
 * \return Battery's warm temperature limit.
 *
 */
__STATIC_INLINE HW_CHARGER_BAT_TEMP_LIMIT hw_charger_get_jeita_warm_temp_limit(void)
{
        return REG_GETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_WARM);
}

/**
 * \brief Set battery's cool temperature limit.
 *
 * \param [in] bat_temp_limit: The battery's cool temperature limit.
 *
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cool_temp_limit(HW_CHARGER_BAT_TEMP_LIMIT bat_temp_limit)
{
        REG_SETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_COOL, bat_temp_limit);
}

/**
 * \brief Get battery's cool temperature limit.
 *
 * \return Battery's cool temperature limit.
 *
 */
__STATIC_INLINE HW_CHARGER_BAT_TEMP_LIMIT hw_charger_get_jeita_cool_temp_limit(void)
{
        return REG_GETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_COOL);
}

/**
 * \brief Set battery's cold temperature limit.
 *
 * \param [in] bat_temp_limit: The battery's cold temperature limit.
 *
 *
 */
__STATIC_INLINE void hw_charger_set_jeita_cold_temp_limit(HW_CHARGER_BAT_TEMP_LIMIT bat_temp_limit)
{
        REG_SETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_COLD, bat_temp_limit);
}

/**
 * \brief Get battery's cold temperature limit.
 *
 * \return Battery's cold temperature limit.
 *
 */
__STATIC_INLINE HW_CHARGER_BAT_TEMP_LIMIT hw_charger_get_jeita_cold_temp_limit(void)
{
        return REG_GETF(CHARGER, CHARGER_TEMPSET_PARAM_REG, TBAT_COLD);
}

/************************************** Charger's fine tune parameters ****************************/

/**
 * \brief Set battery's voltage comparator settling time.
 *
 * \param [in] time: Settling time in usec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_vbat_comparator_settling_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_VBAT_COMP_TIMER_REG, VBAT_COMP_SETTLING, time);
}

/**
 * \brief Set battery's over-voltage comparator settling time.
 *
 * \param [in] time: Settling time in usec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_ovp_comparator_settling_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_VOVP_COMP_TIMER_REG, VBAT_OVP_COMP_SETTLING, time);
}

/**
 * \brief Set die's temperature comparator settling time.
 *
 * \param [in] time: Settling time in usec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_tdie_comparator_settling_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_TDIE_COMP_TIMER_REG, TDIE_COMP_SETTLING, time);
}

/**
 * \brief Set battery's temperature comparator settling time.
 *
 * \param [in] time: Settling time in usec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_tbat_comparator_settling_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_TBAT_COMP_TIMER_REG, TBAT_COMP_SETTLING, time);
}

/**
 * \brief Set battery's hot temperature comparator settling time.
 *
 * \param [in] time: Settling time in usec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_tbat_hot_comparator_settling_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_THOT_COMP_TIMER_REG, THOT_COMP_SETTLING, time);
}

/**
 * \brief Set the periodicity for monitoring the temperature of the battery.
 *
 * \param [in] time: The periodicity in msec (10bit value).
 *
 *
 */
__STATIC_INLINE void hw_charger_set_tbat_monitoring_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_TBAT_MON_TIMER_REG, TBAT_MON_INTERVAL, time);
}

/**
 * \brief Get the periodicity for monitoring the temperature of the battery.
 *
 * \return The periodicity in msec.
 *
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_tbat_monitoring_time(void)
{
        return REG_GETF(CHARGER, CHARGER_TBAT_MON_TIMER_REG, TBAT_MON_INTERVAL);
}

/**
 * \brief Set charger's power up settling time.
 *
 * \param [in] time: Settling time in msec (10bit value).
 *
 */
__STATIC_INLINE void hw_charger_set_charger_powering_up_time(uint32_t time)
{
        REG_SETF(CHARGER, CHARGER_PWR_UP_TIMER_REG, CHARGER_PWR_UP_SETTLING, time);
}

/**
 * \brief Set charger's end of charge threshold check.
 *
 * \param [in] threshold: In usec.
 *
 */
__STATIC_INLINE void hw_charger_set_eoc_interval_check_threshold(uint32_t threshold)
{
        REG_SETF(CHARGER, CHARGER_CTRL_REG, EOC_INTERVAL_CHECK_THRES, threshold);
}

/**
 * \brief Get charger's end of charge threshold check.
 *
 * \return The threshold in usec.
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_eoc_interval_check_threshold(void)
{
        return REG_GETF(CHARGER, CHARGER_CTRL_REG, EOC_INTERVAL_CHECK_THRES);
}

/************************************** Charging timeout parameters ******************************/

/**
 * \brief Set the max timeout for the pre charging state.
 *
 * If timer expires HW FSM moves to error state.
 *
 * \see hw_charger_set_resume_mode()
 *
 * \param [in] timeout: The max timeout in seconds (15bit value).
 *
 *
 */
__STATIC_INLINE void hw_charger_set_max_precharging_timeout(uint32_t timeout)
{
        REG_SETF(CHARGER, CHARGER_PRE_CHARGE_TIMER_REG, MAX_PRE_CHARGE_TIME, timeout);
}

/**
 * \brief Get the max timeout for the pre charging state.
 *
 * \return The max timeout in seconds.
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_max_precharging_timeout(void)
{
        return REG_GETF(CHARGER, CHARGER_PRE_CHARGE_TIMER_REG, MAX_PRE_CHARGE_TIME);
}

/**
 * \brief Set the max timeout for the constant current mode.
 *
 * If timer expires HW FSM moves to error state.
 *
 * \see hw_charger_set_resume_mode()
 *
 * \param [in] timeout: The max timeout in seconds (15bit value).
 *
 *
 */
__STATIC_INLINE void hw_charger_set_max_cc_charging_timeout(uint32_t timeout)
{
        REG_SETF(CHARGER, CHARGER_CC_CHARGE_TIMER_REG, MAX_CC_CHARGE_TIME, timeout);
}

/**
 * \brief Get the max timeout for the constant current mode.
 *
 * \return The max timeout in seconds.
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_max_cc_charging_timeout(void)
{
        return REG_GETF(CHARGER, CHARGER_CC_CHARGE_TIMER_REG, MAX_CC_CHARGE_TIME);
}

/**
 * \brief Set the max timeout for the constant voltage mode.
 *
 * If timer expires HW FSM moves to error state.
 *
 * \see hw_charger_set_resume_mode()
 *
 * \param [in] timeout: The max timeout in seconds (15bit value).
 *
 *
 */
__STATIC_INLINE void hw_charger_set_max_cv_charging_timeout(uint32_t timeout)
{
        REG_SETF(CHARGER, CHARGER_CV_CHARGE_TIMER_REG, MAX_CV_CHARGE_TIME, timeout);
}

/**
 * \brief Get the max timeout for the constant voltage mode.
 *
 * \return The max timeout in seconds.
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_max_cv_charging_timeout(void)
{
        return REG_GETF(CHARGER, CHARGER_CV_CHARGE_TIMER_REG, MAX_CV_CHARGE_TIME);
}

/**
 * \brief Set the max timeout for charging (as soon as the HW FSM starts running).
 *
 * If timer expires HW FSM moves to error state.
 *
 * \see hw_charger_set_resume_mode()
 *
 * \param [in] timeout: The max timeout in seconds (16bit value).
 *
 *
 */
__STATIC_INLINE void hw_charger_set_max_total_charging_timeout(uint32_t timeout)
{
        REG_SETF(CHARGER, CHARGER_TOTAL_CHARGE_TIMER_REG, MAX_TOTAL_CHARGE_TIME, timeout);
}

/**
 * \brief Get the max timeout for charging.
 *
 * \return The max timeout in seconds.
 *
 */
__STATIC_INLINE uint32_t hw_charger_get_max_total_charging_timeout(void)
{
        return REG_GETF(CHARGER, CHARGER_TOTAL_CHARGE_TIMER_REG, MAX_TOTAL_CHARGE_TIME);
}



/************************************** IRQ handling **********************************************/

/**
 * \brief Get the status register (non error cases).
 *
 * \return Status register.
 *
 */
__STATIC_INLINE HW_CHARGER_FSM_IRQ_STAT_OK hw_charger_get_ok_irq_status(void)
{
        return CHARGER->CHARGER_STATE_IRQ_STATUS_REG;
}

/**
 * \brief Get the status register (error cases).
 *
 * \return Status register.
 *
 */
__STATIC_INLINE HW_CHARGER_FSM_IRQ_STAT_NOK hw_charger_get_nok_irq_status(void)
{
        return CHARGER->CHARGER_ERROR_IRQ_STATUS_REG;
}

/**
 * \brief Clear IRQ's (non error cases).
 *
 */
__STATIC_INLINE void hw_charger_clear_ok_irq(void)
{
        CHARGER->CHARGER_STATE_IRQ_CLR_REG = (HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CV_TO_PRECHARGE)         |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CC_TO_PRECHARGE)         |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CV_TO_CC)                |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(TBAT_STATUS_UPDATE)      |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(TBAT_PROT_TO_PRECHARGE)  |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(TDIE_PROT_TO_PRECHARGE)  |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(EOC_TO_PRECHARGE)        |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CV_TO_EOC)               |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CC_TO_EOC)               |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(CC_TO_CV)                |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(PRECHARGE_TO_CC)         |
                                              HW_CHARGER_FSM_CLEAR_IRQ_OK_MASK(DISABLED_TO_PRECHARGE));
}

/**
 * \brief Clear IRQ's (error cases).
 *
 */
__STATIC_INLINE void hw_charger_clear_nok_irq(void)
{
        CHARGER->CHARGER_ERROR_IRQ_CLR_REG = (HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(TBAT_ERROR)             |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(TDIE_ERROR)             |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(VBAT_OVP_ERROR)         |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(TOTAL_CHARGE_TIMEOUT)   |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(CV_CHARGE_TIMEOUT)      |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(CC_CHARGE_TIMEOUT)      |
                                              HW_CHARGER_FSM_CLEAR_IRQ_NOK_MASK(PRECHARGE_TIMEOUT));
}

/**
 * \brief Set IRQ mask (non error cases).
 *
 * Set the events that will trigger an IRQ.
 *
 * \param [in] value: IRQ mask.
 *
 * \warning It's recommended not to enable CC-to-CV and/or CV-to-CC notifications
 *          since HW FSM may oscillate between the two states.
 *
 */
__STATIC_INLINE void hw_charger_set_ok_irq_mask(HW_CHARGER_FSM_IRQ_OK value)
{
        /* Workaround for "Errata issue 302": Charger CC-CV Comparator Hysteresis. */

        /* CC-to-CV comparator has low hysteresis. As a result, the transition from one state */
        /* to another is not smooth, since the HW FSM oscillates between the two states. */
        /* The proposed SW workaround is to suppress the notifications from CC-to-CV and CV-to-CC transitions. */
        ASSERT_WARNING((value & (HW_CHARGER_FSM_IRQ_OK_MASK(CV_TO_CC)    |
                                HW_CHARGER_FSM_IRQ_OK_MASK(CC_TO_CV))) == 0);
        REG_SET_MASKED(CHARGER, CHARGER_STATE_IRQ_MASK_REG, HW_CHARGER_FSM_IRQ_OK_ALL, value);
}

/**
 * \brief Get IRQ mask (non error cases).
 *
 * \return The events that will trigger an IRQ.
 *
 */
__STATIC_INLINE HW_CHARGER_FSM_IRQ_OK hw_charger_get_ok_irq_mask(void)
{
        return CHARGER->CHARGER_STATE_IRQ_MASK_REG;
}

/**
 * \brief Set IRQ mask (error cases).
 *
 * Set the events that will trigger an IRQ.
 *
 * \param [in] value: IRQ mask.
 *
 */
__STATIC_INLINE void hw_charger_set_nok_irq_mask(HW_CHARGER_FSM_IRQ_NOK value)
{
        REG_SET_MASKED(CHARGER, CHARGER_ERROR_IRQ_MASK_REG, HW_CHARGER_FSM_IRQ_NOK_ALL, value);
}

/**
 * \brief Get IRQ mask (error cases).
 *
 * \return The events that will trigger an IRQ.
 *
 */
__STATIC_INLINE HW_CHARGER_FSM_IRQ_NOK hw_charger_get_nok_irq_mask(void)
{
        return CHARGER->CHARGER_ERROR_IRQ_MASK_REG;
}

/**
 * \brief Enable interrupt in NVIC (non error cases).
 *
 * \param [in] cb: Callback function that will be called when the interrupt line is asserted.
 *
 */
void hw_charger_enable_fsm_ok_interrupt(hw_charger_fsm_ok_cb_t cb);

/**
 * \brief Disable interrupt in NVIC (non error cases).
 *
 */
void hw_charger_disable_fsm_ok_interrupt(void);

/**
 * \brief Enable interrupt in NVIC (error cases).
 *
 * \param [in] cb: Callback function that will be called when the interrupt line is asserted.
 *
 */
void hw_charger_enable_fsm_nok_interrupt(hw_charger_fsm_nok_cb_t cb);

/**
 * \brief Disable interrupt in NVIC (error cases).
 *
 */
void hw_charger_disable_fsm_nok_interrupt(void);

/**
 * \brief Program HW FSM according to the charging profile.
 *
 * \param [in] prof: Charging profile.
 *
 */
void hw_charger_program_charging_profile(const hw_charger_charging_profile_t *prof);

/**
 * \brief Program HW FSM according to the fine tuning settings.
 *
 * \param [in] settings: Fine tune settings.
 *
 */
void hw_charger_program_fine_tuning_settings(const hw_charger_fine_tuning_settings_t *settings);


#endif /* dg_configUSE_HW_CHARGER */

#endif /* HW_CHARGER_DA1469x_H_ */

/**
\}
\}
*/
