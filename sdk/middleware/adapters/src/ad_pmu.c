/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup PMU
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_pmu.c
 *
 * @brief PMU adapter API implementation
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configPMU_ADAPTER

#include "sdk_defs.h"
#include <hw_bod.h>
#include <hw_sys.h>
#include <ad_pmu.h>
#include "ad_pmu_internal.h"
#include "hw_usb.h"

#ifdef OS_FREERTOS
#include <sys_power_mgr.h>
#endif

#ifdef CONFIG_USE_BLE
#       include "ble_stack_config.h"
#       include "ble_config.h"
#       include "user_config_defs.h"
#       if USE_BLE_SLEEP
#       include "ad_ble.h"
#       endif
#endif

#define INVALID_DCDC_CONFIG_ENTRY       0xFFFFFFFF
#define AD_PMU_FORCE_VOLTAGE_REQUEST_THRESHOLD  ( 64 )

__RETAINED_RW static uint32_t pmu_dcdc_config_index = INVALID_DCDC_CONFIG_ENTRY;
__RETAINED static ad_pmu_rail_config_t ad_pmu_1v8_rail_config;
__RETAINED static ad_pmu_rail_config_t ad_pmu_1v2_rail_config;
__RETAINED static uint8_t voltage_1v2_forced_count;
__RETAINED static uint8_t dcdc_suspend_cnt;
__RETAINED static bool dcdc_enabled;

#ifdef CONFIG_USE_BLE
#define CMAC_SHARED_POWER_CTRL_REG_CONFIG_MSK                                                   \
        (REG_MSK(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE) |                                    \
         REG_MSK(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE) |                         \
         REG_MSK(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL))
#endif /* CONFIG_USE_BLE */
__RETAINED static uint32_t power_ctrl_reg_default_mode;

#define AD_PMU_ONWAKEUP_VOLTAGE         ( HW_PMU_1V2_VOLTAGE_0V9 )

#ifdef OS_FREERTOS
__RETAINED static OS_MUTEX ad_pmu_mutex;
#endif

#if (dg_configPOWER_1V8_ACTIVE == 1)
__RETAINED_RW static bool pm_1v8_state = true;
#endif

void ad_pmu_set_1v8_state(bool state)
{
#if (dg_configPOWER_1V8_ACTIVE == 1)
        HW_PMU_ERROR_CODE error_code;

#ifdef OS_FREERTOS
        ASSERT_WARNING(ad_pmu_mutex != NULL);

        OS_EVENT_WAIT(ad_pmu_mutex, OS_EVENT_FOREVER);    // Block forever
#endif

        if (state != pm_1v8_state) {

                pm_1v8_state = state;

#ifdef CONFIG_USE_BLE
                GLOBAL_INT_DISABLE();
                while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

                if (state) {
                        error_code = hw_pmu_1v8_onwakeup_enable(ad_pmu_1v8_rail_config.rail_1v8.current);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        if (dg_configPOWER_1V8_SLEEP == 1) {
                                error_code = hw_pmu_1v8_onsleep_enable();
                                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        }
                        if (ad_pmu_1v8_rail_config.use_dcdc) {
                                hw_pmu_1v8_enable_high_efficiency_dcdc();
                        }
#if (dg_configUSE_BOD == 1)
                        hw_bod_activate_channel(BOD_CHANNEL_1V8);
#endif
                } else {
#if (dg_configUSE_BOD == 1)
                        hw_bod_deactivate_channel(BOD_CHANNEL_1V8);
#endif
                        error_code = hw_pmu_1v8_onwakeup_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        if (dg_configPOWER_1V8_SLEEP == 1) {
                                hw_pmu_1v8_onsleep_disable();
                        }
                        if (ad_pmu_1v8_rail_config.use_dcdc) {
                                error_code = hw_pmu_1v8_disable_high_efficiency_dcdc();
                                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        }
                }

#ifdef CONFIG_USE_BLE
                hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
                GLOBAL_INT_RESTORE();
#endif /* CONFIG_USE_BLE */
        }
#ifdef OS_FREERTOS
        OS_EVENT_SIGNAL(ad_pmu_mutex);
#endif
#else
        ASSERT_WARNING(0);
#endif
}

bool ad_pmu_get_1v8_state(void)
{
#if (dg_configPOWER_1V8_ACTIVE == 1)
#ifdef OS_FREERTOS
        ASSERT_WARNING(ad_pmu_mutex != NULL);

        OS_EVENT_WAIT(ad_pmu_mutex, OS_EVENT_FOREVER);    // Block forever
#endif
        bool state = pm_1v8_state;

#ifdef OS_FREERTOS
        OS_EVENT_SIGNAL(ad_pmu_mutex);
#endif
        return state;
#else
        ASSERT_WARNING(0);
        return false;
#endif
}

#ifdef CONFIG_USE_BLE
void cmac_update_power_ctrl_reg_values(uint32_t onsleep_value);
#define CMAC_IS_AWAKE()    ((cmac_dynamic_config_table_ptr != NULL) && (cmac_dynamic_config_table_ptr->maccpu_state == CMAC_STATE_AWAKE))
#endif /* CONFIG_USE_BLE */

static void configure_power_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *cfg)
{
        HW_PMU_ERROR_CODE error_code;

        bool enable_sleep = cfg->enabled && cfg->enabled_on_sleep;

        if (rail == PMU_RAIL_1V8) {
                /* Save the rail configuration. It is used in ad_pmu_set_1v8_state() and ad_pmu_get_1v8_state */
                ad_pmu_1v8_rail_config = *cfg;
        }

        if (rail == PMU_RAIL_1V2) {
                /* Save the rail configuration. Clock manager*/
                ad_pmu_1v2_rail_config = *cfg;
        }

        if (enable_sleep) {
                switch (rail) {
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_onsleep_config(cfg->rail_3v0.current != HW_PMU_3V0_MAX_LOAD_1 ?
                                               HW_PMU_3V0_MAX_LOAD_10 : HW_PMU_3V0_MAX_LOAD_1);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onsleep_enable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onsleep_enable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V2:
                        if (cfg->rail_1v2.current == HW_PMU_1V2_MAX_LOAD_1) {
                                /*
                                 * When current is HW_PMU_1V2_MAX_LOAD_1 then LDO_RET will be used for VDD
                                 * when system wakes-up. In this case, the voltage of LDO_RET must be set
                                 * to 0.9V, otherwise the system will not start properly.
                                 */
                                error_code = hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_SLEEP_VOLTAGE_0V9);
                        } else {
                                /* Enable LDO_CORE first to be able to set LDO_CORE_RET voltage to 0.75V */
                                error_code = hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD_50);
                                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                                error_code = hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_SLEEP_VOLTAGE_0V75);
                        }
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        /*
                         * Set the VDD clamp level below the LDO_CORE_RET level to make sure
                         * that the rail is powered by the LDO.
                         */
                        error_code = hw_pmu_set_vdd_clamp(HW_PMU_VDD_VOLTAGE_706);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V4:
                        // 1V4 rail cannot remain active during sleep
                        ASSERT_WARNING(0);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;

                }
        } else {
                switch (rail) {
                case PMU_RAIL_1V8:
                        /*
                         * If Flash must remain powered during sleep then the 1V8 rail cannot be
                         * turn off. Please set dg_configPOWER_1V8_SLEEP to 1 or call
                         * ad_pmu_configure_rail() to enable 1V8 rail.
                         */
                        ASSERT_WARNING(dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8 ||
                                       dg_configFLASH_POWER_OFF == 1);
                        hw_pmu_1v8_onsleep_disable();
                        break;
                case PMU_RAIL_1V8P:
                        /*
                         * If Flash must remain powered during sleep then the 1V8P rail cannot be
                         * turn off. Please set dg_configPOWER_1V8P_SLEEP to 1 or call
                         * ad_pmu_configure_rail() to enable 1V8P rail.
                         */
                        ASSERT_WARNING((dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8P &&
                                        dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8F) ||
                                        dg_configFLASH_POWER_OFF == 1);

                        error_code = hw_pmu_1v8p_onsleep_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onsleep_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V4:
                        // Nothing to do here. 1V4 rail is always disabled during sleep
                        break;
                case PMU_RAIL_3V0:
                        // 3V0 rail cannot be disabled
                default:
                        ASSERT_WARNING(0);
                        break;
                }
        }

        if (cfg->enabled) {
                switch (rail) {
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_set_voltage(cfg->rail_3v0.voltage);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        error_code = hw_pmu_3v0_onwakeup_config(cfg->rail_3v0.current);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_set_voltage(cfg->rail_1v8.voltage);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        error_code = hw_pmu_1v8_onwakeup_enable(cfg->rail_1v8.current);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onwakeup_enable(cfg->rail_1v8p.current);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onwakeup_set_voltage(cfg->rail_1v2.voltage);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        error_code = hw_pmu_1v2_onwakeup_enable(cfg->rail_1v2.current);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_set_voltage(cfg->rail_1v4.voltage);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        error_code = hw_pmu_1v4_onwakeup_enable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }
        } else {
                switch (rail) {
                case PMU_RAIL_1V8:
                        ASSERT_WARNING(dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8);

                        error_code = hw_pmu_1v8_onwakeup_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8P:
                        ASSERT_WARNING(dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8P &&
                                       dg_configFLASH_CONNECTED_TO != FLASH_CONNECTED_TO_1V8F);

                        error_code = hw_pmu_1v8p_onwakeup_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onwakeup_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_onwakeup_disable();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_3V0:
                        // 3V0 rail cannot be disabled
                default:
                        ASSERT_WARNING(0);
                        break;
                }
        }
#ifdef CONFIG_USE_BLE
        if (rail == PMU_RAIL_1V2) {
                power_ctrl_reg_default_mode = CRG_TOP->POWER_CTRL_REG;
        }
        else {
                power_ctrl_reg_default_mode =
                        (power_ctrl_reg_default_mode & CMAC_SHARED_POWER_CTRL_REG_CONFIG_MSK) |
                        (CRG_TOP->POWER_CTRL_REG & (~CMAC_SHARED_POWER_CTRL_REG_CONFIG_MSK));
        }
        cmac_update_power_ctrl_reg_values(power_ctrl_reg_default_mode);
#else
        power_ctrl_reg_default_mode = CRG_TOP->POWER_CTRL_REG;
#endif /* CONFIG_USE_BLE */
        /*
         * Configure DC/DC converter
         */
        if (cfg->enabled && cfg->use_dcdc) {
                switch (rail) {
                case PMU_RAIL_1V8:
                        hw_pmu_1v8_configure_high_efficiency_dcdc();
                        break;
                case PMU_RAIL_1V8P:
                        hw_pmu_1v8p_configure_high_efficiency_dcdc();
                        break;
                case PMU_RAIL_1V2:
                        hw_pmu_1v2_configure_high_efficiency_dcdc();
                        break;
                case PMU_RAIL_1V4:
                        hw_pmu_1v4_configure_high_efficiency_dcdc();
                        break;
                case PMU_RAIL_3V0:
                        // 3V0 rail is not powered by DC/DC converter
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                if (dcdc_suspend_cnt == 0) {
                        /* Do not not enable DC/DC if VBUS is the power source */
                        if ((!hw_usb_is_powered_by_vbus())) {
                                hw_pmu_dcdc_config();
                                dcdc_enabled = true;
                        }
                }
        } else {
                switch (rail) {
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_disable_high_efficiency_dcdc();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_disable_high_efficiency_dcdc();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_disable_high_efficiency_dcdc();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_disable_high_efficiency_dcdc();
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        break;
                case PMU_RAIL_3V0:
                        // 3V0 rail is not powered by DC/DC converter
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                /*
                 * Check if DC/DC remains enabled after disabling the rail supply
                 */
                dcdc_enabled = hw_pmu_dcdc_is_enabled();
        }
}

int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config)
{
        int ret = 0;

        ASSERT_WARNING(rail < NUM_OF_PMU_RAILS);

        ASSERT_WARNING(rail != PMU_RAIL_1V8  || (dg_configPOWER_1V8_ACTIVE == 2  && dg_configPOWER_1V8_SLEEP == 2));
        ASSERT_WARNING(rail != PMU_RAIL_1V8P || (dg_configPOWER_1V8P_ACTIVE == 2 && dg_configPOWER_1V8P_SLEEP == 2));

#ifdef OS_FREERTOS
        ASSERT_WARNING(ad_pmu_mutex != NULL);

        OS_EVENT_WAIT(ad_pmu_mutex, OS_EVENT_FOREVER);    // Block forever
#endif

#ifdef CONFIG_USE_BLE
        GLOBAL_INT_DISABLE();
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
        if (rail == PMU_RAIL_1V2 && CMAC_IS_AWAKE()) {
                ret = -1;
                goto AD_PMU_RET;
        }
#endif /* CONFIG_USE_BLE */

        if (dg_configUSE_BOD == 1) {
                /* temporarily deactivate BOD to configure the LDOs */
                hw_bod_deactivate();
        }

        if (rail != PMU_RAIL_3V0 &&                                     // 3V0 rail is not powered by DC/DC converter
                pmu_dcdc_config_index != INVALID_DCDC_CONFIG_ENTRY) {   // sys_reg configuration is used
                /* Get register indexes in configuration table */
                uint8_t dcdc_ctrl_reg_index = pmu_dcdc_config_index +
                                offsetof(pmu_dcdc_config_t, ctrl1)/sizeof(hw_sys_reg_config_t);
                uint8_t dcdc_rail_reg_index = pmu_dcdc_config_index + rail +
                                offsetof(pmu_dcdc_config_t, rail)/sizeof(hw_sys_reg_config_t);

                /* Save DC/DC rail register address */
                __IO uint32 *rail_reg_addr = hw_sys_reg_get_config(dcdc_rail_reg_index)->addr;

                /*
                 * Invalidate register address to prevent other masters from applying
                 * the old configuration
                 */
                hw_sys_reg_modify_config(dcdc_ctrl_reg_index, &DCDC->DCDC_STATUS1_REG, 0);
                hw_sys_reg_modify_config(dcdc_rail_reg_index, &DCDC->DCDC_STATUS1_REG, 0);

#if (USE_BLE_SLEEP == 1)
                /* Update CMAC configuration */
                ad_ble_sys_tcs_config();
#endif

                configure_power_rail(rail, config);

                /* Update pmu_dcdc_config values */
                hw_sys_reg_modify_config(dcdc_ctrl_reg_index, &DCDC->DCDC_CTRL1_REG, DCDC->DCDC_CTRL1_REG);
                hw_sys_reg_modify_config(dcdc_rail_reg_index, rail_reg_addr, *rail_reg_addr);

#if (USE_BLE_SLEEP == 1)
                /* Update CMAC configuration */
                ad_ble_sys_tcs_config();
#endif
        } else {
                configure_power_rail(rail, config);
        }

#if (dg_configUSE_BOD == 1)
        hw_bod_configure();
#endif

#ifdef CONFIG_USE_BLE
AD_PMU_RET:
        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
        GLOBAL_INT_RESTORE();
#endif /* CONFIG_USE_BLE */

#ifdef OS_FREERTOS
        OS_EVENT_SIGNAL(ad_pmu_mutex);
#endif

        return ret;
}

/* Enable DCDC taking into account the BOD configuration.
 * To be used during VBUS attaching / detaching procedure.
 * Not to be called directly.
 * Callers should determine first the origin of the power source (VBUS or VBAT).
 */
static void enable_dcdc_safe(bool enable)
{
        if (enable) {
                hw_pmu_dcdc_enable();
                /* Switching from VBUS to VBAT. BOD at VBAT channel should be restored. */
                if (dg_configUSE_BOD == 1) {
                        REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN);
                }
        } else {
                /* Switching from VBAT to VBUS. BOD at VBAT channel should be canceled. */
                if (dg_configUSE_BOD == 1) {
                        REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN);
                }
                hw_pmu_dcdc_disable();
        }
}

static void enable_dcdc(bool enable)
{
#ifdef OS_FREERTOS
        ASSERT_WARNING(ad_pmu_mutex != NULL);

        OS_EVENT_WAIT(ad_pmu_mutex, OS_EVENT_FOREVER);    // Block forever
#endif

        bool force_enable = false;
        bool force_disable = false;
        if (enable == false) {
                force_disable = (dcdc_suspend_cnt == 0);
                dcdc_suspend_cnt++;

                /* Max reference counter value is 126 */
                ASSERT_WARNING(dcdc_suspend_cnt < 127);
        } else {
                if (dcdc_suspend_cnt > 0) {
                        dcdc_suspend_cnt--;
                }
                force_enable = (dcdc_suspend_cnt == 0);
        }

        if (dcdc_enabled && (force_enable || force_disable)) {
                if (pmu_dcdc_config_index != INVALID_DCDC_CONFIG_ENTRY) {
                        /* Get register index in configuration table */
                        uint8_t dcdc_ctrl_reg_index = pmu_dcdc_config_index +
                                        offsetof(pmu_dcdc_config_t, ctrl1)/sizeof(hw_sys_reg_config_t);

                        /*
                         * Invalidate register address to prevent other masters from applying
                         * the old configuration
                         */
                        hw_sys_reg_modify_config(dcdc_ctrl_reg_index, &DCDC->DCDC_STATUS1_REG, 0);

#if (USE_BLE_SLEEP == 1)
                        /* Update CMAC configuration */
                        ad_ble_sys_tcs_config();
#endif

                        enable_dcdc_safe(force_enable);

                        /* Update pmu_dcdc_config values */
                        hw_sys_reg_modify_config(dcdc_ctrl_reg_index, &DCDC->DCDC_CTRL1_REG, DCDC->DCDC_CTRL1_REG);

#if (USE_BLE_SLEEP == 1)
                        /* Update CMAC configuration */
                        ad_ble_sys_tcs_config();
#endif
                } else {
                        enable_dcdc_safe(force_enable);
                }
        }

#ifdef OS_FREERTOS
        OS_EVENT_SIGNAL(ad_pmu_mutex);
#endif
}

void ad_pmu_dcdc_suspend(void)
{
        enable_dcdc(false);
}

void ad_pmu_dcdc_resume(void)
{
        enable_dcdc(true);
}

static void update_1v2_voltage(void)
{
        HW_PMU_ERROR_CODE error_code;

#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        if (voltage_1v2_forced_count) {
#ifdef CONFIG_USE_BLE
                if (!CMAC_IS_AWAKE()) {
#endif /* CONFIG_USE_BLE */
                        if (ad_pmu_1v2_rail_config.enabled == false) {
                                HW_PMU_1V2_RAIL_CONFIG rail_config;
                                hw_pmu_get_1v2_onwakeup_config(&rail_config);

                                /*
                                 * PMU adapter has not been used to configure the 1V2 rail.
                                 * Save the LDO state and voltage state to be able to restore
                                 * them in ad_pmu_1v2_force_max_voltage_release()
                                 */
                                ad_pmu_1v2_rail_config.rail_1v2.voltage = rail_config.voltage;
                                ad_pmu_1v2_rail_config.rail_1v2.current = rail_config.current;
                        }

                        /*
                         * Make sure that LDO_CORE_RET is not used. This is always needed even
                         * if DC/DC is currently used, in case the DC/DC is disabled and 1V2
                         * rail supply is switch to LDO.
                         */
                        error_code = hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD_50);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                        /* Set the voltage */
                        error_code = hw_pmu_1v2_onwakeup_set_voltage(HW_PMU_1V2_VOLTAGE_1V2);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
#ifdef CONFIG_USE_BLE
                }

                cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_CTRL_REG);
#endif /* CONFIG_USE_BLE */
        } else {
#ifdef CONFIG_USE_BLE
                if (!CMAC_IS_AWAKE()) {
#endif /* CONFIG_USE_BLE */
                        if (ad_pmu_1v2_rail_config.enabled) {
                                /* Restore rail configuration */
                                CRG_TOP->POWER_CTRL_REG = power_ctrl_reg_default_mode
                                        & (~(REG_MSK(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE)
                                                & REG_MSK(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE)));
                                CRG_TOP->POWER_CTRL_REG = power_ctrl_reg_default_mode;
#ifdef CONFIG_USE_BLE
                                cmac_update_power_ctrl_reg_values(power_ctrl_reg_default_mode);
#endif /* CONFIG_USE_BLE */
                        } else {
                                HW_PMU_ERROR_CODE error_code;

                                /* Restore the voltage */
                                error_code = hw_pmu_1v2_onwakeup_set_voltage(ad_pmu_1v2_rail_config.rail_1v2.voltage);
                                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                                /* Restore LDO used for wakeup */
                                error_code = hw_pmu_1v2_onwakeup_enable(ad_pmu_1v2_rail_config.rail_1v2.current);
                                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
#ifdef CONFIG_USE_BLE
                                cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_CTRL_REG);
#endif /* CONFIG_USE_BLE */
                        }
#ifdef CONFIG_USE_BLE
                } else {
                        cmac_update_power_ctrl_reg_values(power_ctrl_reg_default_mode);
                }
#endif /* CONFIG_USE_BLE */
        }

#ifdef CONFIG_USE_BLE
        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
}

void ad_pmu_1v2_force_max_voltage_request(void)
{
#ifdef CONFIG_USE_BLE
        GLOBAL_INT_DISABLE();
#endif /* CONFIG_USE_BLE */

        ASSERT_ERROR(voltage_1v2_forced_count < AD_PMU_FORCE_VOLTAGE_REQUEST_THRESHOLD);

        voltage_1v2_forced_count++;

        if (voltage_1v2_forced_count == 1) {
                update_1v2_voltage();
        }

#ifdef CONFIG_USE_BLE
        GLOBAL_INT_RESTORE();
#endif /* CONFIG_USE_BLE */
}

void ad_pmu_1v2_force_max_voltage_release(void)
{
#ifdef CONFIG_USE_BLE
        GLOBAL_INT_DISABLE();
#endif /* CONFIG_USE_BLE */

        ASSERT_ERROR(voltage_1v2_forced_count != 0);

        voltage_1v2_forced_count--;

        if (voltage_1v2_forced_count == 0) {
                update_1v2_voltage();
        }

#ifdef CONFIG_USE_BLE
        GLOBAL_INT_RESTORE();
#endif /* CONFIG_USE_BLE */
}

void ad_pmu_prepare_for_sleep(void)
{
        HW_PMU_ERROR_CODE error_code;

        if (ad_pmu_1v2_rail_config.enabled && ad_pmu_1v2_rail_config.enabled_on_sleep) {
#ifdef CONFIG_USE_BLE
                uint32_t applied_power_reg_val;

                while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);

                if (!CMAC_IS_AWAKE()) {
#endif /* CONFIG_USE_BLE */
                        /* Set voltage */
                        error_code = hw_pmu_1v2_onwakeup_set_voltage(AD_PMU_ONWAKEUP_VOLTAGE);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
#ifdef CONFIG_USE_BLE
                        applied_power_reg_val = CRG_TOP->POWER_CTRL_REG;
                } else {
                        applied_power_reg_val = CRG_TOP->POWER_CTRL_REG;
                        /* Set voltage */
                        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL,
                                applied_power_reg_val, AD_PMU_ONWAKEUP_VOLTAGE);
                }

                cmac_update_power_ctrl_reg_values(applied_power_reg_val);

                hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        }
}

void ad_pmu_restore_for_wake_up(void)
{
        update_1v2_voltage();
}

#ifdef OS_FREERTOS

const adapter_call_backs_t ad_pmu_pm_call_backs = {
        /*
         * No need to prepare the DCDC for sleep. This will be done by PDC.
         * No need to configure BOD. PDC will switch off the channels of the rails that will
         * remain without power during sleep.
         */
        .ad_prepare_for_sleep = NULL,
        .ad_sleep_canceled = NULL,

        /*
         * BOD setup for active mode is not required. PDC will reactivate the BOD channels
         * that were turned off during sleep. DC/DC configuration is applied by power manager
         * immediately after wake-up to optimize power consumption.
         */
        .ad_wake_up_ind = NULL,
        .ad_xtalm_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

/**
 * \brief Initialize adapter
 *
 */
static void ad_pmu_init(void)
{
        ad_pmu_rail_config_t rail_config;
        HW_PMU_ERROR_CODE error_code;

        pm_register_adapter(&ad_pmu_pm_call_backs);

        ad_pmu_mutex = xSemaphoreCreateMutex();         // Create Mutex
        ASSERT_WARNING(ad_pmu_mutex != NULL);

        if (dg_configUSE_BOD == 1) {
                // temporarily deactivate BOD to configure the LDOs
                hw_bod_deactivate();
        }

        /*
         * 3V0 rail configuration
         */

        rail_config.enabled  = true;
        rail_config.enabled_on_sleep  = true;
#if (dg_configUSE_SYS_CHARGER == 1)
        /* Workaround for "Errata issue 281": Charger Detect Circuit erroneous if V30 setting not at 3.3V */
        rail_config.rail_3v0.voltage = HW_PMU_3V0_VOLTAGE_3V3;
#else
        rail_config.rail_3v0.voltage = HW_PMU_3V0_VOLTAGE_3V0;
#endif
        rail_config.rail_3v0.current = HW_PMU_3V0_MAX_LOAD_150;
        rail_config.use_dcdc = false;
        configure_power_rail(PMU_RAIL_3V0, &rail_config);

        bool rail_1v4_dcdc_en, rail_1v2_dcdc_en, rail_1v8_dcdc_en, rail_1v8p_dcdc_en;

        rail_1v4_dcdc_en = dg_configUSE_DCDC == 1;
        rail_1v2_dcdc_en = dg_configUSE_DCDC == 1;

        if (dg_configPOWER_1V8_ACTIVE != 2) {
                rail_1v8_dcdc_en = (dg_configPOWER_1V8_ACTIVE == 1) && (dg_configUSE_DCDC == 1);
        } else {
                rail_1v8_dcdc_en = true;        // Default value
        }

        if (dg_configPOWER_1V8P_ACTIVE != 2) {
                rail_1v8p_dcdc_en = (dg_configPOWER_1V8P_ACTIVE == 1) && (dg_configUSE_DCDC == 1);
        } else {
                rail_1v8p_dcdc_en = true;       // Default value
        }

       /*
        * By default DC/DC converter will supply power to all rails. Disable the unused rails to
        * avoid glitches in the supply current when enabling the DC/DC converter.
        */
        if (rail_1v4_dcdc_en == false) {
                error_code = hw_pmu_1v4_disable_high_efficiency_dcdc();
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        if (rail_1v2_dcdc_en == false) {
                error_code = hw_pmu_1v2_disable_high_efficiency_dcdc();
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        if (rail_1v8_dcdc_en == false) {
                error_code = hw_pmu_1v8_disable_high_efficiency_dcdc();
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        if (rail_1v8p_dcdc_en == false) {
                error_code = hw_pmu_1v8p_disable_high_efficiency_dcdc();
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

       /*
        * 1V8 rail configuration
        */
        if (dg_configPOWER_1V8_ACTIVE != 2) {
                rail_config.enabled  = (dg_configPOWER_1V8_ACTIVE == 1);
        } else {
                rail_config.enabled = true;            // Default value
        }

        rail_config.use_dcdc = rail_1v8_dcdc_en;

        if (dg_configPOWER_1V8_SLEEP != 2) {
                rail_config.enabled_on_sleep = rail_config.enabled && (dg_configPOWER_1V8_SLEEP == 1);
        } else {
                rail_config.enabled_on_sleep = rail_config.enabled; // Default value
        }

        rail_config.rail_1v8.voltage = HW_PMU_1V8_VOLTAGE_1V8;
        rail_config.rail_1v8.current = HW_PMU_1V8_MAX_LOAD_10;
        configure_power_rail(PMU_RAIL_1V8, &rail_config);

        /*
         * 1V8F switch configuration
         * NOTE: V18F switch should not be used, V18F and V18P should be shorted on PCB
         * NOTE: "Errata issue 294": V18F Resistance too high
         */
        hw_pmu_1v8f_enable();

       /*
        * 1V8P rail configuration
        */
        if (dg_configPOWER_1V8P_ACTIVE != 2) {
                rail_config.enabled  = (dg_configPOWER_1V8P_ACTIVE == 1);
        } else {
                rail_config.enabled = true;            // Default value
        }

        rail_config.use_dcdc = rail_1v8p_dcdc_en;

        if (dg_configPOWER_1V8P_SLEEP != 2) {
                rail_config.enabled_on_sleep = rail_config.enabled && (dg_configPOWER_1V8P_SLEEP == 1);
        } else {
                rail_config.enabled_on_sleep = rail_config.enabled; // Default value
        }

        /* Allow power intensive flash operations with LDO. */

        rail_config.rail_1v8p.current = HW_PMU_1V8_MAX_LOAD_50;
        configure_power_rail(PMU_RAIL_1V8P, &rail_config);

       /*
        * 1V2 rail configuration
        */
#if (dg_configUSE_BOD == 1)
        hw_bod_set_channel_voltage_level(BOD_CHANNEL_VDD, 700);
        hw_bod_set_channel_voltage_level(BOD_CHANNEL_VDD_SLEEP, 700);
#endif

        rail_config.enabled_on_sleep = true;
        rail_config.enabled = true;
        rail_config.use_dcdc = rail_1v2_dcdc_en;
        rail_config.rail_1v2.voltage = (hw_clk_get_sysclk() == SYS_CLK_IS_PLL) ?
                HW_PMU_1V2_VOLTAGE_1V2 : HW_PMU_1V2_VOLTAGE_0V9;
        rail_config.rail_1v2.current = HW_PMU_1V2_MAX_LOAD_50;
        configure_power_rail(PMU_RAIL_1V2, &rail_config);

       /*
        * 1V4 rail configuration
        */
        rail_config.enabled = true;
        rail_config.enabled_on_sleep  = false;  // V14 rail cannot be enabled during sleep
        rail_config.use_dcdc = rail_1v4_dcdc_en;
        rail_config.rail_1v4.voltage = HW_PMU_1V4_VOLTAGE_1V40;
        configure_power_rail(PMU_RAIL_1V4, &rail_config);

#if (dg_configUSE_BOD == 1)
        hw_bod_configure();
#endif

        // Add DC/DC register configuration in system and CMAC tables
        pmu_dcdc_config_t dcdc_config;

        dcdc_config.ctrl1.addr  = &DCDC->DCDC_CTRL1_REG;
        dcdc_config.ctrl1.value = DCDC->DCDC_CTRL1_REG;
        dcdc_config.rail[PMU_RAIL_1V4].addr   = &DCDC->DCDC_V14_REG;
        dcdc_config.rail[PMU_RAIL_1V4].value  = DCDC->DCDC_V14_REG;
        dcdc_config.rail[PMU_RAIL_1V2].addr   = &DCDC->DCDC_VDD_REG;
        dcdc_config.rail[PMU_RAIL_1V2].value  = DCDC->DCDC_VDD_REG;
        dcdc_config.rail[PMU_RAIL_1V8].addr   = &DCDC->DCDC_V18_REG;
        dcdc_config.rail[PMU_RAIL_1V8].value  = DCDC->DCDC_V18_REG;
        dcdc_config.rail[PMU_RAIL_1V8P].addr  = &DCDC->DCDC_V18P_REG;
        dcdc_config.rail[PMU_RAIL_1V8P].value = DCDC->DCDC_V18P_REG;

        pmu_dcdc_config_index = hw_sys_reg_add_config((hw_sys_reg_config_t *)&dcdc_config,
                                                       sizeof(pmu_dcdc_config_t)/sizeof(hw_sys_reg_config_t));

#if (USE_BLE_SLEEP == 1)
        // Update CMAC configuration
        ad_ble_sys_tcs_config();
#endif
}

ADAPTER_INIT(ad_pmu_adapter, ad_pmu_init);

#endif /* OS_FREERTOS */

#endif /* dg_configPMU_ADAPTER */


/**
 * \}
 * \}
 * \}
 */
