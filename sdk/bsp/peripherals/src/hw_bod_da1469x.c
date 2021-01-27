/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup BOD
\{
*/

/**
****************************************************************************************
*
* @file hw_bod_da1469x.c
*
* @brief BOD LLD
*
* Copyright (C) 2017-2020 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_BOD

#include "hw_bod.h"
#include "hw_pmu.h"
#include "hw_usb.h"

void hw_bod_activate_before_sleep(void)
{
        uint32_t val = CRG_TOP->BOD_CTRL_REG;

        HW_PMU_1V8_RAIL_CONFIG v18_rail_config;
        if (hw_pmu_get_1v8_onsleep_config(&v18_rail_config) == POWER_RAIL_DISABLED) {
                REG_CLR_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN, val);
        }

        HW_PMU_1V8P_RAIL_CONFIG v18p_rail_config;
        if (hw_pmu_get_1v8p_onsleep_config(&v18p_rail_config) == POWER_RAIL_DISABLED) {
                REG_CLR_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN, val);
                REG_CLR_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN, val);
        }

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                HW_PMU_1V2_SLEEP_VOLTAGE v12_rail_sleep_voltage;
                if (hw_pmu_get_1v2_onsleep_config(&v12_rail_sleep_voltage) == POWER_RAIL_ENABLED) {
                        uint16_t voltage;

                        switch (v12_rail_sleep_voltage) {
                        // Use a switch instead of a lookup table to be consistent with the LLD
                        case HW_PMU_1V2_SLEEP_VOLTAGE_0V75:
                                voltage = 750;
                                break;
                        case HW_PMU_1V2_SLEEP_VOLTAGE_0V8:
                                voltage = 800;
                                break;
                        case HW_PMU_1V2_SLEEP_VOLTAGE_0V85:
                                voltage = 850;
                                break;
                        case HW_PMU_1V2_SLEEP_VOLTAGE_0V9:
                                voltage = 900;
                                break;
                        case HW_PMU_1V2_SLEEP_VOLTAGE_0V95:
                                voltage = 950;
                                break;
                        case HW_PMU_1V2_SLEEP_VOLTAGE_1V0:
                                voltage = 1000;
                                break;
                        default:
                                ASSERT_WARNING(0);
                                return;
                        }

                        ASSERT_WARNING(voltage > hw_bod_get_channel_voltage_level(BOD_CHANNEL_VDD_SLEEP)*108/100);
                }
                else {
                        // Check clamp voltage

                        uint16_t voltage;

                        switch (REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_CLAMP_LEVEL)) {
                        // Use a switch instead of a lookup table to be consistent with the LLD                                                case HW_PMU_VDD_VOLTAGE_1037:
                        case HW_PMU_VDD_VOLTAGE_1037:
                                voltage = 1037;
                                break;
                        case HW_PMU_VDD_VOLTAGE_1005:
                                voltage = 1005;
                                break;
                        case HW_PMU_VDD_VOLTAGE_978:
                                voltage = 978;
                                break;
                        case HW_PMU_VDD_VOLTAGE_946:
                                voltage = 946;
                                break;
                        case HW_PMU_VDD_VOLTAGE_1120:
                                voltage = 1120;
                                break;
                        case HW_PMU_VDD_VOLTAGE_1089:
                                voltage = 1089;
                                break;
                        case HW_PMU_VDD_VOLTAGE_1058:
                                voltage = 1058;
                                break;
                        case HW_PMU_VDD_VOLTAGE_1030:
                                voltage = 1030;
                                break;
                        case HW_PMU_VDD_VOLTAGE_952:
                                voltage = 952;
                                break;
                        case HW_PMU_VDD_VOLTAGE_918:
                                voltage = 918;
                                break;
                        case HW_PMU_VDD_VOLTAGE_889:
                                voltage = 889;
                                break;
                        case HW_PMU_VDD_VOLTAGE_861:
                                voltage = 861;
                                break;
                        case HW_PMU_VDD_VOLTAGE_862:
                                voltage = 862;
                                break;
                        case HW_PMU_VDD_VOLTAGE_828:
                                voltage = 828;
                                break;
                        case HW_PMU_VDD_VOLTAGE_798:
                                voltage = 798;
                                break;
                        case HW_PMU_VDD_VOLTAGE_706:
                                voltage = 706;
                                break;
                        default:
                                ASSERT_WARNING(0);
                                return;
                        }

                        ASSERT_WARNING(voltage > hw_bod_get_channel_voltage_level(BOD_CHANNEL_VDD_SLEEP)*108/100);

                }
        }

        REG_CLR_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN, val);

        CRG_TOP->BOD_CTRL_REG = val;
}

static void hw_bod_activate_on_wup(void)
{
       uint32_t val = 0;

       HW_PMU_1V2_RAIL_CONFIG v12_rail_config;
       if (hw_pmu_get_1v2_onwakeup_config(&v12_rail_config) == POWER_RAIL_ENABLED) {
               REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN, val, 1);
       }

       HW_PMU_1V8_RAIL_CONFIG v18_rail_config;
       if (hw_pmu_get_1v8_onwakeup_config(&v18_rail_config) == POWER_RAIL_ENABLED) {
               REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN, val, 1);
       }

       HW_PMU_1V8P_RAIL_CONFIG v18p_rail_config;
       if (hw_pmu_get_1v8p_onwakeup_config(&v18p_rail_config) == POWER_RAIL_ENABLED) {
               REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN, val, 1);
       }

       HW_PMU_1V4_RAIL_CONFIG v14_rail_config;
       if (hw_pmu_get_1v4_onwakeup_config(&v14_rail_config) == POWER_RAIL_ENABLED) {
               REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN, val, 1);
       }

       REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_V30_EN, val, 1);

       /* If the power source is VBUS then no point to enable BOD on VBAT channel. */
       if (!hw_usb_is_powered_by_vbus()) {
               /* Power source is VBAT, so BOD at VBAT channel should be enabled. */
               REG_SET_FIELD(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN, val, 1);
       }

       CRG_TOP->BOD_CTRL_REG = val;
}

void hw_bod_configure(void)
{
        REG_SETF(CRG_TOP, BOD_CTRL_REG, BOD_CLK_DIV, 0); // Reset value

        hw_bod_activate_on_wup();

        /* Generate Reset on a BOD event */
        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V14_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VDD_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V30_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_RST_EN);
        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, mask);
}

#endif /* dg_configUSE_BOD */


/**
\}
\}
\}
*/
