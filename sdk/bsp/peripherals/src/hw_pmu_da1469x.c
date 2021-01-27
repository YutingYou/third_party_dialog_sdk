/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup PMU
\{
*/

/**
****************************************************************************************
*
* @file hw_pmu_da1469x.c
*
* @brief Power Manager Unit for DA1469x
*
* Copyright (C) 2015-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_PMU

#include "hw_pmu.h"
#include <string.h>

#ifndef HW_PMU_SANITY_CHECKS_ENABLE
#       if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
#               define HW_PMU_SANITY_CHECKS_ENABLE 1
#       else
#               define HW_PMU_SANITY_CHECKS_ENABLE 0
#       endif
#endif

#define HW_PMU_BOD_THRESHOLD_MARGIN_PERCENT (4)

/* 3V0 LDOs check masks */
#define CHK_LDO_VBAT_RET_MSK  0x1
#define CHK_LDO_VBAT_VBUS_MSK 0x2
#define CHK_VSYS_CLAMP_MSK    0x4

/* 1V2 dependants check masks */
#define CHK_PLL96M_MSK       0x1
#define CHK_UFAST_WAKEUP_MSK 0x2
#define CHK_USB_PHY_MSK      0x4
#define CHK_XTAL32K_MSK      0x10

#define V12_DEPS_MASK_SHIFT 5

/* 3V0 dependants check masks */
#define CHK_LDO_IO            0x1
#define CHK_LDO_IO2           0x2
#define CHK_LDO_IO_RET        0x4
#define CHK_LDO_IO_RET2       0x8
#define CHK_LDO_CORE          0x10
#define CHK_LDO_RADIO         0x20
#define CHK_LRA               0x40
#define CHK_3V0_USB_PHY       0x80
#define CHK_LDO_CORE_RET      0x100

/**
 * \brief PMU State
 *
 */
typedef enum {
        HW_PMU_ACTIVE_STATE = 0,
        HW_PMU_WAKEUP_STATE = 1,
        HW_PMU_SLEEP_STATE  = 2
} HW_PMU_STATE;

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_peripherals(uint16_t mask, HW_PMU_STATE state)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & CHK_LDO_IO) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE)) {
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & CHK_LDO_IO2) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE)) {
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & CHK_LDO_IO_RET) {
                if ((state == HW_PMU_ACTIVE_STATE) | (state == HW_PMU_WAKEUP_STATE)) {
                    if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE)) {
                            return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                    }
                } else if (state == HW_PMU_SLEEP_STATE) {
                    /* Check LDO_IO_RET wakeup state */
                    if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_SLEEP)) {
                            return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                    }
                }
        }

        if (mask & CHK_LDO_IO_RET2) {
            if ((state == HW_PMU_ACTIVE_STATE) | (state == HW_PMU_WAKEUP_STATE)) {
                    if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE)) {
                            return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                    }
            } else if (state == HW_PMU_SLEEP_STATE) {
                    if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_SLEEP)) {
                            return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                    }
            }
        }

        if (mask & CHK_LDO_CORE) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE)) {
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & CHK_LDO_RADIO) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE)) {
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & CHK_LRA) {
                if (REG_GETF(LRA, LRA_CTRL1_REG, LRA_EN)) {
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & CHK_3V0_USB_PHY) {
                if (REG_GETF(USB, USB_MCTRL_REG, USBEN)) {
                        return HW_PMU_ERROR_USB_PHY_ON;
                }
        }

        if (mask & CHK_LDO_CORE_RET) {
                return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
        }

#endif
        return HW_PMU_ERROR_NOERROR;
}

__STATIC_INLINE HW_PMU_ERROR_CODE v30_sanity_check(uint16_t cfg, HW_PMU_STATE state)
{
        int res;

        uint16_t deps_periph = (cfg & 0x1FF);
        res = check_3v0_peripherals(deps_periph, state);

        return res;
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_ldos_active(uint8_t mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & CHK_LDO_VBAT_RET_MSK) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_ACTIVE)) {
                        /* LDO_VBAT_RET is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & CHK_LDO_VBAT_VBUS_MSK) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_MODE) == 0x3) {
                        /* LDO_VBAT/LDO_VBUS is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & CHK_VSYS_CLAMP_MSK) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, CLAMP_3V0_VBAT_ENABLE)) {
                        /* VSYS_CLAMPLDO_VBAT/LDO_VBUS is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_ldos_sleep(uint8_t mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & CHK_LDO_VBAT_RET_MSK) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_SLEEP)) {
                        /* LDO_VBAT_RET is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & CHK_VSYS_CLAMP_MSK) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, CLAMP_3V0_VBAT_ENABLE)) {
                        /* VSYS_CLAMPLDO_VBAT/LDO_VBUS is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v2_peripherals(uint8_t mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & CHK_PLL96M_MSK) {
                if (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M)) {
                        /* PLL96 is powered by 1V4 but the ARM core is powered by 1V2.
                        * The ARM core can only function at 96MHz if the voltage is 1.2V.
                        * For PLL the RUNNING_AT_PLL96M of CLK_CTRL_REG must be checked to allow
                        * setting a voltage lower than 1.2V */
                        return HW_PMU_ERROR_PLL96M_ON;
                }
        }

        if (mask & CHK_USB_PHY_MSK) {
                if (REG_GETF(USB, USB_MCTRL_REG, USBEN)) {
                        return HW_PMU_ERROR_USB_PHY_ON;
                }
        }

        if (mask & CHK_XTAL32K_MSK) {
                if (REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE)) {
                        return HW_PMU_ERROR_XTAL32K_ON;
                }
        }

        if (mask & CHK_UFAST_WAKEUP_MSK) {
                if (REG_GETF(CRG_TOP, PMU_SLEEP_REG , ULTRA_FAST_WAKEUP)) {
                                       return HW_PMU_ERROR_UFAST_WAKEUP_ON;
                }
        }
#endif
        return HW_PMU_ERROR_NOERROR;
}

__STATIC_INLINE HW_PMU_ERROR_CODE v12_sanity_check(uint16_t cfg)
{
        int res;

        // LDOs that 1V2 rail depends on are stored at bits starting at V12_DEPS_MASK_SHIFT of cfg
        // Keep only these bits in deps_srcs
        uint8_t deps_srcs = (uint8_t)((cfg >> V12_DEPS_MASK_SHIFT) & ((1 << (8 - V12_DEPS_MASK_SHIFT)) - 1));
        if (deps_srcs != 0) {
                /* Not DCDC, so LDO */
                res = check_3v0_ldos_active(deps_srcs);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
        }

        uint8_t deps_periph = (uint8_t)(cfg & ((1 << V12_DEPS_MASK_SHIFT) - 1));
        res = check_1v2_peripherals(deps_periph);

        return res;
}

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
static int hw_pmu_get_bod_threshold_level_mv(int bod_lvl) {

        int level;

        /* calculate threshold level in milivolts */
        level = 1200*(bod_lvl+1)/192;

        /* add margin */
        level += (HW_PMU_BOD_THRESHOLD_MARGIN_PERCENT*level)/100;

        return level;
}
#endif

static bool is_all_dcdc_rails_off(void)
{
        /* Check 1V8 rail */
        if (REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_HV) ||
            REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_LV) ||
            REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_HV) ||
            REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_LV) ||
            REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV) ||
            REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV) ||
            REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_HV) ||
            REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_LV) ) {
                return false;
        }

        return true;
}

static void check_and_disable_dcdc(void)
{
        if (is_all_dcdc_rails_off()) {
                hw_pmu_dcdc_disable();
        }
}

static void dcdc_config(void)
{
        if (!hw_pmu_dcdc_is_enabled()) {
             hw_pmu_dcdc_config();
        }
}

void hw_pmu_dcdc_config(void)
{
        hw_pmu_dcdc_enable();
}

#define V30_SRC_VSYS_CLAMP    0
#define V30_SRC_LDO_VBAT_RET  1
#define V30_SRC_LDO_VBAT_VBUS 2

static void v30_ldo_vbat_ret_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_ACTIVE);
}

static void v30_ldo_vbat_vbus_disable(void)
{
        REG_SETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_MODE, 0);
}

static void v30_ldo_vbat_ret_active()
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_ACTIVE);
}

static void v30_ldo_vbat_vbus_active()
{
        REG_SETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_MODE, 0x3);
}

void hw_pmu_3v0_clamp_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_3V0_VBAT_ENABLE);
}

void hw_pmu_3v0_clamp_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_3V0_VBAT_ENABLE);
}

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
/* array indexed with HW_PMU_3V0_VOLTAGE */
static const int v30_voltage_mv[] = { 2400, 3000, 3300, 3450 };

static bool check_3v0_bod_threshold(HW_PMU_3V0_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V30_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                                  BOD_LVL_CTRL0_REG, BOD_LVL_V30));
                if (v30_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }

        return false;
}
#endif

static HW_PMU_3V0_VOLTAGE get_ldo_vbat_vbus_voltage(void)
{
        uint8_t vol;
        HW_PMU_3V0_VOLTAGE voltage = 0;

        vol = REG_GETF(CRG_TOP, POWER_CTRL_REG, V30_LEVEL);

        switch (vol) {
        case 0:
                voltage = HW_PMU_3V0_VOLTAGE_3V0;
                break;
        case 1:
                voltage = HW_PMU_3V0_VOLTAGE_3V45;
                break;
        case 2:
        case 3:
                voltage = HW_PMU_3V0_VOLTAGE_3V3;
                break;
        default:
                ASSERT_ERROR(0);
        }

        return voltage;
}

/* 3V0 Active/Wakeup
  max_load   idx   srcs
  1   (0) |   0   |  0
  10  (1) |   1   |  1
  150 (2) |   2   |  2
 */
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
static uint16_t chk_3v0_deps_wakeup[] = {
        /* 0  */ CHK_LDO_IO | CHK_LDO_IO2 | CHK_LDO_IO_RET | CHK_LDO_IO_RET2 | CHK_LDO_CORE | CHK_LDO_RADIO | CHK_LRA,
        /* 1  */ CHK_LDO_IO | CHK_LDO_IO2 | CHK_LDO_CORE | CHK_LDO_RADIO | CHK_LRA,
        /* 2  */ 0x0,
};
#endif

static uint8_t v30_srcs_wakeup[] = {
        /* 0  */ V30_SRC_VSYS_CLAMP,
        /* 1  */ V30_SRC_LDO_VBAT_RET,
        /* 2  */ V30_SRC_LDO_VBAT_VBUS,
};

HW_PMU_ERROR_CODE hw_pmu_3v0_set_voltage(HW_PMU_3V0_VOLTAGE voltage)
{

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (voltage == HW_PMU_3V0_VOLTAGE_2V4) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        if (check_3v0_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }
#endif

        // Set 3V0 rail voltage reference to the bandgap output
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_3V0_REF);

        // Level setting for 3V0 rail
        switch (voltage) {
        case HW_PMU_3V0_VOLTAGE_3V0:
                REG_SETF(CRG_TOP, POWER_CTRL_REG, V30_LEVEL, 0x0);
                break;
        case HW_PMU_3V0_VOLTAGE_3V3:
                REG_SETF(CRG_TOP, POWER_CTRL_REG, V30_LEVEL, 0x2);
                break;
        case HW_PMU_3V0_VOLTAGE_3V45:
                REG_SETF(CRG_TOP, POWER_CTRL_REG, V30_LEVEL, 0x1);
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_config(HW_PMU_3V0_MAX_LOAD max_load)
{
        uint8_t idx = max_load;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        int res = 0;
        HW_PMU_3V0_VOLTAGE voltage;

        if (idx >= (sizeof(chk_3v0_deps_wakeup)/sizeof(chk_3v0_deps_wakeup[0]))) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        voltage = (max_load == HW_PMU_3V0_MAX_LOAD_1) ? HW_PMU_3V0_VOLTAGE_2V4 : get_ldo_vbat_vbus_voltage();
        if (check_3v0_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }

        res = v30_sanity_check(chk_3v0_deps_wakeup[idx], HW_PMU_WAKEUP_STATE);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        switch (v30_srcs_wakeup[idx]) {
        case V30_SRC_LDO_VBAT_RET:
                v30_ldo_vbat_ret_active();
                /* Disable other LDO */
                v30_ldo_vbat_vbus_disable();
                hw_pmu_3v0_clamp_disable();
                break;
        case V30_SRC_LDO_VBAT_VBUS:
                v30_ldo_vbat_vbus_active();
                /* Disable other LDO */
                v30_ldo_vbat_ret_disable();
                hw_pmu_3v0_clamp_disable();
                break;
        case V30_SRC_VSYS_CLAMP:
                hw_pmu_3v0_clamp_enable();
                /* Disable other LDO */
                v30_ldo_vbat_ret_disable();
                v30_ldo_vbat_vbus_disable();
                break;
        default:
                ASSERT_ERROR(0);
        }

        return HW_PMU_ERROR_NOERROR;
}

/* 3V0 Sleep
  max_load   idx   srcs
  1   (0) |   0   |  0
  10  (1) |   1   |  1
  */

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
static uint16_t chk_3v0_deps_sleep[] = {
        /* 0  */ CHK_LDO_IO_RET | CHK_LDO_IO_RET2,
        /* 1  */ 0x0,
};
#endif

static uint8_t v30_srcs_sleep[] = {
        /* 0  */ V30_SRC_VSYS_CLAMP,
        /* 1  */ V30_SRC_LDO_VBAT_RET,
};

static void v30_ldo_vbat_ret_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_SLEEP);
}

static void v30_ldo_vbat_ret_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_SLEEP);
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_config(HW_PMU_3V0_MAX_LOAD max_load)
{
        uint8_t idx = max_load;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        int res;
        HW_PMU_3V0_VOLTAGE voltage;

        if (idx >= (sizeof(chk_3v0_deps_sleep)/sizeof(chk_3v0_deps_sleep[0]))) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        voltage = (max_load == HW_PMU_3V0_MAX_LOAD_1) ? HW_PMU_3V0_VOLTAGE_2V4 : get_ldo_vbat_vbus_voltage();
        if (check_3v0_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }

        res = v30_sanity_check(chk_3v0_deps_sleep[idx], HW_PMU_SLEEP_STATE);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        switch (v30_srcs_sleep[idx]) {
        case V30_SRC_VSYS_CLAMP:
                hw_pmu_3v0_clamp_enable();
                /* Disable other LDO at sleep*/
                v30_ldo_vbat_ret_sleep_disable();
                break;
        case V30_SRC_LDO_VBAT_RET:
                v30_ldo_vbat_ret_sleep_enable();
                break;
        default:
                ASSERT_ERROR(0);
        }

        return HW_PMU_ERROR_NOERROR;
}

static void v18_ldo_io_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE);
}

static void v18_ldo_io_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE);
}

static void v18_ldo_io_ret_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE);
}

static void v18_ldo_io_ret_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE);
}

static void v18_dcdc_disable(void)
{
        uint32_t reg;

        reg = DCDC->DCDC_V18_REG;
        REG_SET_FIELD(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_HV, reg, 0);
        REG_SET_FIELD(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_LV, reg, 0);
        DCDC->DCDC_V18_REG = reg;

        check_and_disable_dcdc();
}

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
/* array indexed with HW_PMU_1V8_VOLTAGE */
static const int v18_voltage_mv[] = { 1200, 1800 };

static bool check_1v8_bod_threshold(HW_PMU_1V8_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                                   BOD_LVL_CTRL0_REG, BOD_LVL_V18));
                if (v18_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }

        return false;
}
#endif

HW_PMU_ERROR_CODE hw_pmu_1v8_set_voltage(HW_PMU_1V8_VOLTAGE voltage)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (check_1v8_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }
#endif
        if (voltage == HW_PMU_1V8_VOLTAGE_1V2) {
                REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, V18_LEVEL);
        } else {
                REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, V18_LEVEL);
        }

        return HW_PMU_ERROR_NOERROR;
}

void hw_pmu_1v8_configure_high_efficiency_dcdc(void)
{
        /* DCDC supply rail to 1,8V and max load to 50mA */
        uint32_t reg;

        /* Enable DCDC IRQ mask, although in most cases it's not used. */
        REG_SET_BIT(DCDC, DCDC_IRQ_MASK_REG, DCDC_V18_TIMEOUT_IRQ_MASK);

        reg = DCDC->DCDC_V18_REG;

        /* Set default battery level to high battery voltage */
        REG_SET_FIELD(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_HV, reg, 1);
        REG_SET_FIELD(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_LV, reg, 0);
        DCDC->DCDC_V18_REG = reg;
}

void hw_pmu_1v8_enable_high_efficiency_dcdc(void)
{
        hw_pmu_1v8_configure_high_efficiency_dcdc();
        dcdc_config();
}

HW_PMU_ERROR_CODE hw_pmu_1v8_disable_high_efficiency_dcdc(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (!REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_HV) &&
            !REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_LV)) {
                /* 1V8 rails is already disabled  */
                return HW_PMU_ERROR_NOERROR;
        }

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif
        /* Switch off rail DCDC */
        v18_dcdc_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        int res = HW_PMU_ERROR_NOERROR;

        if (max_load == HW_PMU_1V8_MAX_LOAD_50) {
                /* 50mA Max load */
                res = check_3v0_ldos_active(CHK_LDO_VBAT_VBUS_MSK);
                if (res == HW_PMU_ERROR_NOERROR) {
                        v18_ldo_io_active_enable();
                        /* Disable other LDO LDO_IO_RET */
                        v18_ldo_io_ret_disable();
                }
        } else {
                /* 10mA Max load */
                res = check_3v0_ldos_active(CHK_LDO_VBAT_VBUS_MSK | CHK_LDO_VBAT_RET_MSK);
                if (res == HW_PMU_ERROR_NOERROR) {
                        v18_ldo_io_ret_active_enable();
                        /* Disable other LDO LDO_IO */
                        v18_ldo_io_disable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif
        /* Switch off rail LDOs */
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE);
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE);

        return HW_PMU_ERROR_NOERROR;
}

static void v18_ldo_io_ret_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_SLEEP);
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(void)
{
        int res = HW_PMU_ERROR_NOERROR;

        /* 10mA Max load */
        if ((res = check_3v0_ldos_sleep(CHK_LDO_VBAT_RET_MSK)) == HW_PMU_ERROR_NOERROR) {
                v18_ldo_io_ret_sleep_enable();
        }

        return res;
}

void hw_pmu_1v8_onsleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_SLEEP);
}

static void v18p_ldo_io_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE);
}

static void v18p_ldo_io_ret_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE);
}

static void v18p_ldo_io_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE);
}

static void v18p_ldo_io_ret_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE);
}

static void v18p_dcdc_disable(void)
{
        uint32_t reg;

        reg = DCDC->DCDC_V18P_REG;
        REG_SET_FIELD(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_HV, reg, 0);
        REG_SET_FIELD(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_LV, reg, 0);
        DCDC->DCDC_V18P_REG = reg;

        check_and_disable_dcdc();
}

void hw_pmu_1v8p_configure_high_efficiency_dcdc(void)
{
        uint32_t reg;

        /* Enable DCDC IRQ mask, although in most cases it's not used. */
        REG_SET_BIT(DCDC, DCDC_IRQ_MASK_REG, DCDC_V18P_TIMEOUT_IRQ_MASK);

        reg = DCDC->DCDC_V18P_REG;

        /* Set default battery level to high battery voltage */
        REG_SET_FIELD(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_HV, reg, 1);
        REG_SET_FIELD(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_LV, reg, 0);
        DCDC->DCDC_V18P_REG = reg;
}

void hw_pmu_1v8p_enable_high_efficiency_dcdc(void)
{
        hw_pmu_1v8p_configure_high_efficiency_dcdc();
        dcdc_config();
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_disable_high_efficiency_dcdc(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (!REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_HV) &&
            !REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_LV)) {
                /* 1V8P dcdc rails is already disabled,
                 * LDO could be disabled only by HW_PMU_1V8p_onwakeup_disable */
                return HW_PMU_ERROR_NOERROR;
        }

        if (!REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE) &&
                /* Check if GPIOs are off on 1v8rail  - P0_x port output is powered by VDD1V8P rail*/
                ( REG_GETF(GPIO, P0_PADPWR_CTRL_REG, P0_OUT_CTRL) ||
                REG_GETF(GPIO, P1_PADPWR_CTRL_REG, P1_OUT_CTRL))) {
                /* Cannot disable V18p rail - GPIO use it */
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        v18p_dcdc_disable();

        /* Switch off rail LDOs by HW_PMU_1V8p_onwakeup_disable and HW_PMU_1V8p_onsleep_disable*/

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        int res = HW_PMU_ERROR_NOERROR;

        if (max_load == HW_PMU_1V8_MAX_LOAD_50) {
                /* 50mA Max load */
                res = check_3v0_ldos_active(CHK_LDO_VBAT_VBUS_MSK);
                if (res == HW_PMU_ERROR_NOERROR) {
                        v18p_ldo_io_active_enable();
                        /* Disable other LDO LDO_IO_RET2 */
                        v18p_ldo_io_ret_disable();
                }
        } else {
                /* 10mA Max load */
                res = check_3v0_ldos_active(CHK_LDO_VBAT_VBUS_MSK | CHK_LDO_VBAT_RET_MSK);
                if (res == HW_PMU_ERROR_NOERROR) {
                        v18p_ldo_io_ret_active_enable();
                        /* Disable other LDO LDO_IO2 */
                        v18p_ldo_io_disable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif
        /* Check if GPIOs are off on V18P rail */
        if ( REG_GETF(GPIO, P0_PADPWR_CTRL_REG, P0_OUT_CTRL) ||
             REG_GETF(GPIO, P1_PADPWR_CTRL_REG, P1_OUT_CTRL)) {
                /* Cannot disable V18P rail - GPIO use it */
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        /* Switch off rail LDOs */
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE);
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE);

        return HW_PMU_ERROR_NOERROR;
}

static void v18p_ldo_io_ret_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_SLEEP);
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_enable(void)
{
        int res = HW_PMU_ERROR_NOERROR;

        /* 10mA Max load */
        if ((res = check_3v0_ldos_sleep(CHK_LDO_VBAT_RET_MSK)) == HW_PMU_ERROR_NOERROR) {
                v18p_ldo_io_ret_sleep_enable();
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_disable(void)
{
        /* Check if GPIOs are off on V18P rail */
        if ( REG_GETF(GPIO, P0_PADPWR_CTRL_REG, P0_OUT_CTRL) ||
             REG_GETF(GPIO, P1_PADPWR_CTRL_REG, P1_OUT_CTRL)) {
                /* Cannot disable V18P rail - GPIO use it */
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_SLEEP);

        return HW_PMU_ERROR_NOERROR;
}

void hw_pmu_1v4_configure_high_efficiency_dcdc(void)
{
        /* DCDC supply rail to 1V4 and max load to 20mA */
        uint32_t reg;

        /* Enable DCDC IRQ mask, although in most cases it's not used. */
        REG_SET_BIT(DCDC, DCDC_IRQ_MASK_REG, DCDC_V14_TIMEOUT_IRQ_MASK);

        reg = DCDC->DCDC_V14_REG;

        /* Set default battery level to high battery voltage */
        REG_SET_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV, reg, 1);
        REG_SET_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV, reg, 1);
        DCDC->DCDC_V14_REG = reg;
}

void hw_pmu_1v4_enable_high_efficiency_dcdc(void)
{
        hw_pmu_1v4_configure_high_efficiency_dcdc();
        dcdc_config();
}

static void v14_dcdc_disable(void)
{
        uint32_t reg;

        reg = DCDC->DCDC_V14_REG;
        REG_SET_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV, reg, 0);
        REG_SET_FIELD(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV, reg, 0);
        DCDC->DCDC_V14_REG = reg;

        check_and_disable_dcdc();
}

static HW_PMU_ERROR_CODE v14_ldo_radio_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE);

        return HW_PMU_ERROR_NOERROR;
}

static void v14_ldo_radio_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE);
}

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
/* array indexed with HW_PMU_1V4_VOLTAGE */
static const int v14_voltage_mv[] = { 1200, 1250, 1300, 1350, 1400, 1450, 1500, 1550 };

static bool check_1v4_bod_threshold(HW_PMU_1V4_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                                  BOD_LVL_CTRL2_REG, BOD_LVL_V14));
                if (v14_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }
        return false;
}
#endif

HW_PMU_ERROR_CODE hw_pmu_1v4_set_voltage(HW_PMU_1V4_VOLTAGE voltage)
{
        ASSERT_WARNING(voltage <= HW_PMU_1V4_VOLTAGE_1V55);

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (check_1v4_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }
#endif

        REG_SETF(CRG_TOP, POWER_CTRL_REG, V14_LEVEL, voltage);

        return HW_PMU_ERROR_NOERROR;
}

static HW_PMU_ERROR_CODE v14_check_active_dependants(void)
{
        /* Check that PLL96M is off (USB is implied, since it needs PLL96M) */
        if (REG_GETF(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN)) {
                return HW_PMU_ERROR_PLL96M_ON;
        }

        /* Check that XTAL32M is off */
        if (REG_GETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_XTAL_ENABLE)) {
                return HW_PMU_ERROR_XTAL32M_ON;
        }

        /* Check that radio is off */
        if (REG_GETF(CRG_TOP, CLK_RADIO_REG, RFCU_ENABLE)) {
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_disable_high_efficiency_dcdc(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        if (!REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV) &&
            !REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV)) {
                /* 1V4 DCDC is already disabled */
                return HW_PMU_ERROR_NOERROR;
        }

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        if (!REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE)) {
                res = v14_check_active_dependants();
                if (res != HW_PMU_ERROR_NOERROR) {
                        /* Active dependency and LDO on 1V4 is disabled */
                        return res;
                }
        }
#endif
        /* We could disable dcdc only when no active dependency or if 1V4 LDO is enabled */
        v14_dcdc_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_enable(void)
{
        int res = HW_PMU_ERROR_NOERROR;

        res = check_3v0_ldos_active(CHK_LDO_VBAT_VBUS_MSK);
        if (res == HW_PMU_ERROR_NOERROR) {
                res = v14_ldo_radio_enable();
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_disable(void)
{
        HW_PMU_ERROR_CODE res;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        res = v14_check_active_dependants();
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }

        /* Program 1V4 to be off on wakeup and on active */
        v14_ldo_radio_disable();

        return HW_PMU_ERROR_NOERROR;
}

static void v12_dcdc_disable(void)
{
        uint32_t reg;

        reg = DCDC->DCDC_VDD_REG;
        REG_SET_FIELD(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_HV, reg, 0);
        REG_SET_FIELD(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_LV, reg, 0);
        DCDC->DCDC_VDD_REG = reg;

        check_and_disable_dcdc();
}

static void v12_ldo_core_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE);
}

static void v12_ldo_core_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE);
}

static HW_PMU_ERROR_CODE v12_set_core_ret_voltage(HW_PMU_1V2_SLEEP_VOLTAGE voltage)
{
        if (voltage > HW_PMU_1V2_SLEEP_VOLTAGE_1V0) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        REG_SETF(CRG_TOP, POWER_CTRL_REG, VDD_SLEEP_LEVEL, voltage);

        return HW_PMU_ERROR_NOERROR;
}

static void v12_ldo_core_ret_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE);
}

static void v12_ldo_core_ret_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE);
}

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
/* array indexed with HW_PMU_1V2_VOLTAGE */
static const int v12_voltage_mv[] = { 900, 1000, 1100, 1200 };
/* array indexed with HW_PMU_1V2_SLEEP_VOLTAGE */
static const int v12_sleep_voltage_mv[] = { 750, 800, 850, 900, 950, 1000 };
/* array indexed with HW_PMU_VDD_CLAMP_VOLTAGE */
static const int v12_clamp_voltage_mv[] = { 1037, 1005, 978, 946, 1120, 1089, 1058, 952, 918,
                                            889, 861, 862, 828, 798, 706 };

static bool check_1v2_bod_threshold(HW_PMU_1V2_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                               BOD_LVL_CTRL1_REG, BOD_LVL_VDD_ON));
                if (v12_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }

        return false;
}

static bool check_1v2_bod_threshold_sleep(HW_PMU_1V2_SLEEP_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                              BOD_LVL_CTRL1_REG, BOD_LVL_VDD_RET));
                if (v12_sleep_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }

        return false;
}

static bool check_1v2_bod_threshold_clamp(HW_PMU_VDD_CLAMP_VOLTAGE voltage)
{
        int bod_threshold_level;

        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                bod_threshold_level = hw_pmu_get_bod_threshold_level_mv(REG_GETF(CRG_TOP,
                                                              BOD_LVL_CTRL1_REG, BOD_LVL_VDD_RET));
                if (v12_clamp_voltage_mv[voltage] < bod_threshold_level) {
                        return true;
                }
        }

        return false;
}
#endif

HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_set_voltage(HW_PMU_1V2_VOLTAGE voltage)
{
        if (voltage > HW_PMU_1V2_VOLTAGE_1V2) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        if (check_1v2_bod_threshold(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }

        if (voltage != HW_PMU_1V2_VOLTAGE_1V2) {
                /* if USB / PLL is active switch to the other voltage is not allowed */
                res = check_1v2_peripherals(CHK_PLL96M_MSK | CHK_USB_PHY_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
        }
        if (voltage != HW_PMU_1V2_VOLTAGE_0V9 &&
                REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE) == 0) {
                /*
                 * LDO_CORE_RET maximum voltage is 0.9V. Voltage levels > 0.9V can only be set
                 * when LDO_CORE is used for wake-up. Return an error if the requested wake-up
                 * voltage is > 0.9V and the system is not configured to wake-up with LDO_CORE.
                 */
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }
#endif

        uint32_t reg1 = QSPIC->QSPIC_CTRLMODE_REG;
        uint32_t reg2 = QSPIC2->QSPIC2_CTRLMODE_REG;

        // QSPIC readpipe cannot be greater than 2 when VDD_CORE is not 1.2V
        ASSERT_WARNING(voltage == HW_PMU_1V2_VOLTAGE_1V2 ||
                ((REG_GET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_RPIPE_EN, reg1) == 0 ||
                  REG_GET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_PCLK_MD, reg1) <= 2) &&
                 (REG_GET_FIELD(QSPIC2, QSPIC2_CTRLMODE_REG, QSPIC_RPIPE_EN, reg2) == 0 ||
                  REG_GET_FIELD(QSPIC2, QSPIC2_CTRLMODE_REG, QSPIC_PCLK_MD, reg2) <= 2)));

        REG_SETF(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL, voltage);

        return HW_PMU_ERROR_NOERROR;
}

void hw_pmu_1v2_configure_high_efficiency_dcdc(void)
{
        uint32_t reg;

        /* Enable DCDC IRQ mask, although in most cases it's not used. */
        REG_SET_BIT(DCDC, DCDC_IRQ_MASK_REG, DCDC_VDD_TIMEOUT_IRQ_MASK);

        reg = DCDC->DCDC_VDD_REG;

        /* Set default battery level to high battery voltage */
        REG_SET_FIELD(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_HV, reg, 1);
        REG_SET_FIELD(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_LV, reg, 1);
        DCDC->DCDC_VDD_REG = reg;
}

void hw_pmu_1v2_enable_high_efficiency_dcdc(void)
{
        hw_pmu_1v2_configure_high_efficiency_dcdc();
        dcdc_config();
}

HW_PMU_ERROR_CODE hw_pmu_1v2_disable_high_efficiency_dcdc(void)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        if (!REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_LV) &&
            !REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_HV)) {
                /* 1V2 DCDC is already disabled */
                return res;
        }

        if (!REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE)) {
                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE)) {
                        // XTAL32K and RC32K can be powered by LDO_CORE_RET
                        res = check_1v2_peripherals(CHK_USB_PHY_MSK | CHK_PLL96M_MSK);
                }
                else {
                        res = check_1v2_peripherals(CHK_USB_PHY_MSK | CHK_XTAL32K_MSK |
                                                    CHK_PLL96M_MSK);
                }

                if (res != HW_PMU_ERROR_NOERROR) {
                        /* Active dependency and LDO core on 1V2 is disabled */
                        return res;
                }
        }

        /* We could disable dcdc only when no active dependency or if 1V2 LDO core is enabled */
        v12_dcdc_disable();

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_set_vdd_clamp(HW_PMU_VDD_CLAMP_VOLTAGE voltage)
{
        if (voltage > HW_PMU_VDD_VOLTAGE_706) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        /* Allow VDD Clamp level to be set lower than the BOD threshold when LDO_CORE_RET is enabled */
        if (check_1v2_bod_threshold_clamp(voltage) &&
                REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP) == 0) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }
#endif

        REG_SETF(CRG_TOP, POWER_CTRL_REG, VDD_CLAMP_LEVEL, voltage);

        return HW_PMU_ERROR_NOERROR;
}

#define V12_SRC_LDO_CORE_RET 0
#define V12_SRC_LDO_CORE     1

/* 1V2 Wakeup
 max_load  idx   srcs
 1  (0)  |  0  |  0
 50 (1)  |  1  |  1

*/
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
static uint16_t chk_1v2_deps_wakeup[] = {
        /* 0  */ CHK_PLL96M_MSK | CHK_USB_PHY_MSK |
                        (CHK_LDO_VBAT_RET_MSK << V12_DEPS_MASK_SHIFT) |
                        (CHK_LDO_VBAT_VBUS_MSK << V12_DEPS_MASK_SHIFT) |
                        (CHK_VSYS_CLAMP_MSK  << V12_DEPS_MASK_SHIFT) ,
        /* 1  */ CHK_UFAST_WAKEUP_MSK |
                        (CHK_LDO_VBAT_VBUS_MSK << V12_DEPS_MASK_SHIFT) ,
};
#endif

static uint8_t v12_srcs_wakeup[] = {
        /* 0  */ V12_SRC_LDO_CORE_RET,
        /* 1  */ V12_SRC_LDO_CORE,
};

static HW_PMU_1V2_SLEEP_VOLTAGE get_1v2_ldo_core_ret_voltage(void)
{
        return (HW_PMU_1V2_SLEEP_VOLTAGE)REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_SLEEP_LEVEL);
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD max_load)
{
        uint8_t idx = max_load;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        int res;
        if (idx >= (sizeof(chk_1v2_deps_wakeup)/sizeof(chk_1v2_deps_wakeup[0]))) {
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        res = v12_sanity_check(chk_1v2_deps_wakeup[idx]);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        switch (v12_srcs_wakeup[idx]) {
        case V12_SRC_LDO_CORE_RET:
                if (get_1v2_ldo_core_ret_voltage() < HW_PMU_1V2_SLEEP_VOLTAGE_0V9) {
                        return HW_PMU_ERROR_SLEEP_LDO;
                }
                v12_ldo_core_ret_enable();
                /* Disabled other LDO*/
                v12_ldo_core_disable();
                break;
        case V12_SRC_LDO_CORE:
                v12_ldo_core_enable();
                /* Disabled other LDO*/
                v12_ldo_core_ret_disable();
                break;
        default:
                ASSERT_ERROR(0);
        }

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_disable(void)
{
        HW_PMU_ERROR_CODE res;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        res = check_1v2_peripherals(CHK_PLL96M_MSK | CHK_USB_PHY_MSK | CHK_XTAL32K_MSK);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }

        /* Disable 1V2 for wakeup */
        v12_ldo_core_disable();
        v12_ldo_core_ret_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_SLEEP_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (voltage < HW_PMU_1V2_SLEEP_VOLTAGE_0V9) {
                res = check_1v2_peripherals(CHK_UFAST_WAKEUP_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }

                if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE)) {
                        return HW_PMU_ERROR_SLEEP_LDO;
                }
        }

        if (check_1v2_bod_threshold_sleep(voltage)) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }

        res = check_3v0_ldos_sleep(CHK_LDO_VBAT_RET_MSK | CHK_VSYS_CLAMP_MSK);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        res = v12_set_core_ret_voltage(voltage);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }

        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP);

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        /* Do not disable LDO_CORE_RET if VDD Clamp level is lower than the BOD threshold */
        if (check_1v2_bod_threshold_clamp(REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_CLAMP_LEVEL))) {
                return HW_PMU_ERROR_BOD_THRESHOLD;
        }

        res = check_1v2_peripherals( CHK_XTAL32K_MSK);
        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP);

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onwakeup_config(HW_PMU_3V0_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_3V0_RAIL_CONFIG));

        rail_config->voltage = get_ldo_vbat_vbus_voltage();

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_MODE) == 0x3) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_3V0_MAX_LOAD_150;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_3V0_MAX_LOAD_10;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onsleep_config(HW_PMU_3V0_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_3V0_RAIL_CONFIG));

        rail_config->voltage = get_ldo_vbat_vbus_voltage();
        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_3V0_RET_ENABLE_SLEEP)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_3V0_MAX_LOAD_10;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, CLAMP_3V0_VBAT_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_3V0_MAX_LOAD_1;
                rail_config->voltage = HW_PMU_3V0_VOLTAGE_2V4;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, V18_LEVEL)) {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;
        } else {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V2;
        }

        if (hw_pmu_dcdc_is_enabled() &&
            (REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_HV) ||
             REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_ENABLE_LV))) {

                /* First check DCDC config as a HIGH EFFICIENCY rail */
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
                if (!REG_GETF(DCDC, DCDC_V18_REG, DCDC_V18_CUR_LIM_MAX_HV)) {
                        rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                } else {
                        rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                }
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, V18_LEVEL)) {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;
        } else {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V2;
        }

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, V18_LEVEL)) {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;
        } else {
                rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V2;
        }

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8_RET_ENABLE_SLEEP)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_active_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8P_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;

        if (hw_pmu_dcdc_is_enabled() &&
            (REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_HV) ||
             REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_ENABLE_LV))) {
                /* First check DCDC config as a HIGH EFFICIENCY rail */
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
                if (!REG_GETF(DCDC, DCDC_V18P_REG, DCDC_V18P_CUR_LIM_MAX_HV)) {
                        rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                } else {
                        rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                }
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onwakeup_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8P_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onsleep_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V8P_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_SLEEP)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

static HW_PMU_1V4_VOLTAGE get_ldo_radio_voltage(void)
{
        return (HW_PMU_1V4_VOLTAGE)REG_GETF(CRG_TOP, POWER_CTRL_REG, V14_LEVEL);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_active_config(HW_PMU_1V4_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V4_RAIL_CONFIG));

        rail_config->voltage = get_ldo_radio_voltage();

        if (hw_pmu_dcdc_is_enabled() &&
            (REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV) ||
             REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV))) {
                /* First check DCDC config as a HIGH EFFICIENCY rail */
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onwakeup_config(HW_PMU_1V4_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, 0, sizeof(HW_PMU_1V4_RAIL_CONFIG));

        rail_config->voltage = get_ldo_radio_voltage();

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

static HW_PMU_1V2_VOLTAGE get_1v2_active_voltage(void)
{
        uint8_t voltage = REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL);
        switch (voltage) {
        case 0:
                return HW_PMU_1V2_VOLTAGE_0V9;
        case 1:
                return HW_PMU_1V2_VOLTAGE_1V0;
        case 2:
                return HW_PMU_1V2_VOLTAGE_1V1;
        case 3:
                return HW_PMU_1V2_VOLTAGE_1V2;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_active_config(HW_PMU_1V2_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;
        HW_PMU_1V2_SLEEP_VOLTAGE sleep_voltage;

        memset(rail_config, 0, sizeof(HW_PMU_1V2_RAIL_CONFIG));

        if (hw_pmu_dcdc_is_enabled() &&
            (REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_HV) ||
             REG_GETF(DCDC, DCDC_VDD_REG, DCDC_VDD_ENABLE_LV))) {
                /* First check DCDC config as a HIGH EFFICIENCY rail */
                r_state = POWER_RAIL_ENABLED;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_50;
                rail_config->voltage = get_1v2_active_voltage();
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
                rail_config->voltage = get_1v2_active_voltage();
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_1;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
                sleep_voltage = get_1v2_ldo_core_ret_voltage();

                if (sleep_voltage == HW_PMU_1V2_SLEEP_VOLTAGE_0V9) {
                        rail_config->voltage = HW_PMU_1V2_VOLTAGE_0V9;
                } else if (sleep_voltage == HW_PMU_1V2_SLEEP_VOLTAGE_1V0) {
                        rail_config->voltage = HW_PMU_1V2_VOLTAGE_1V0;
                } else {
                        ASSERT_WARNING(0);
                }
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onwakeup_config(HW_PMU_1V2_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;
        HW_PMU_1V2_SLEEP_VOLTAGE sleep_voltage;

        memset(rail_config, 0, sizeof(HW_PMU_1V2_RAIL_CONFIG));

        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_50;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
                rail_config->voltage = get_1v2_active_voltage();
        } else if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE)) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_1;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;

                sleep_voltage = get_1v2_ldo_core_ret_voltage();

                if (sleep_voltage == HW_PMU_1V2_SLEEP_VOLTAGE_0V9) {
                        rail_config->voltage = HW_PMU_1V2_VOLTAGE_0V9;
                } else if (sleep_voltage == HW_PMU_1V2_SLEEP_VOLTAGE_1V0) {
                        rail_config->voltage = HW_PMU_1V2_VOLTAGE_1V0;
                } else {
                        ASSERT_WARNING(0);
                }
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onsleep_config(HW_PMU_1V2_SLEEP_VOLTAGE *rail_votlage)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;


        if (REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP)) {
                r_state = POWER_RAIL_ENABLED;
                *rail_votlage = get_1v2_ldo_core_ret_voltage();
        }
        else {
                *rail_votlage = 0;
        }

        return r_state;
}

#endif /* dg_configUSE_HW_PMU */
/**
\}
\}
\}
*/
