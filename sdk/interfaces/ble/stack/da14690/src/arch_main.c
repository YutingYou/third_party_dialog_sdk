/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Initialization of BLE arhitecture.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE

/*
 * INCLUDES
 ****************************************************************************************
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "co_version.h"
#include "ble_gap.h"
#include "ble_config.h"
#include "ble_stack_config.h"
#include "sdk_defs.h"
#include "rwip_config.h"
#include "rwip.h"
#include "gap_cfg.h"
#include "arch.h"
#include "ke_task.h"
#include "cmac_mailbox.h"
#include "osal.h"
#include "core_cm33.h"
#include "hw_memctrl.h"
#include "co_bt.h"
#include "ad_ble.h"
#include "hw_clk.h"
#include "hw_pd.h"
#include "hw_pdc.h"
#include "platform_nvparam.h"
#include "hw_pmu.h"

#if (dg_configUSE_HW_PDC == 0)
#error "PDC is required for BLE sleep."
#endif

void lib_ble_stack_init(void);
void lib_ble_stack_reset(uint8_t reset_type);
bool cmac_cpu_wakeup(void);

void timer_init(void);
void ble_timer_callback(void* varg);

bool ke_sleep_check(void);

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
extern bool ad_ble_temp_meas_enabled;
#endif

#define KE_GROSSTIMER_MASK      ((uint32_t)0x007FFFFF)
#define ODD_TO_NEXT_EVEN(x)     ((x) & 0x01 ? x+1 : x)

__RETAINED OS_TIMER ble_timer;
__RETAINED uint8_t cmac_system_tcs_length;
__RETAINED uint8_t cmac_synth_tcs_length;
__RETAINED uint8_t cmac_rfcu_tcs_length;

/*
 * This is the placeholder for CMAC code, data and mailbox sections.
 */
extern uint32_t __cmi_section_end__;



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/** Custom message handlers */
__RETAINED struct custom_msg_handler custom_msg_handlers[] =
{
        { 0, 0, NULL}   //it must end with func == NULL so that loop can terminate correctly
};

/** GAP configuration table */
const struct gap_cfg_user_struct gap_cfg_user_var_struct = {
        .GAP_TMR_LECB_DISCONN_TIMEOUT_VAR = 0x0BB8,
        .GAP_APPEARANCE_VAR = 0x0000,
        .GAP_PPCP_CONN_INTV_MAX_VAR = 0x0064,
        .GAP_PPCP_CONN_INTV_MIN_VAR = 0x00c8,
        .GAP_PPCP_SLAVE_LATENCY_VAR = 0x0000,
        .GAP_PPCP_STO_MULT_VAR = 0x07d0,
        .GAP_TMR_LIM_ADV_TIMEOUT_VAR = 0x4650,
        .GAP_TMR_GEN_DISC_SCAN_VAR = 0x0300,
        .GAP_TMR_LIM_DISC_SCAN_VAR = 0x0300,
        .GAP_TMR_PRIV_ADDR_INT_VAR = 0x3A98,
        .GAP_TMR_CONN_PAUSE_CT_VAR = 0x0064,
        .GAP_TMR_CONN_PAUSE_PH_VAR = 0x01F4,
        .GAP_TMR_CONN_PARAM_TIMEOUT_VAR = 0x0BB8,
        .GAP_TMR_LECB_CONN_TIMEOUT_VAR = 0x0BB8,
        .GAP_TMR_SCAN_FAST_PERIOD_VAR = 0x0C00,
        .GAP_TMR_ADV_FAST_PERIOD_VAR = 0x0BB8,
        .GAP_LIM_DISC_SCAN_INT_VAR = 0x0012,
        .GAP_SCAN_FAST_INTV_VAR = 0x0030,
        .GAP_SCAN_FAST_WIND_VAR = 0x0030,
        .GAP_SCAN_SLOW_INTV1_VAR = 0x00CD,
        .GAP_SCAN_SLOW_INTV2_VAR = 0x019A,
        .GAP_SCAN_SLOW_WIND1_VAR = 0x0012,
        .GAP_SCAN_SLOW_WIND2_VAR = 0x0024,
        .GAP_ADV_FAST_INTV1_VAR = 0x0030,
        .GAP_ADV_FAST_INTV2_VAR = 0x0064,
        .GAP_ADV_SLOW_INTV_VAR = 0x00B0,
        .GAP_INIT_CONN_MIN_INTV_VAR = 0x0018,
        .GAP_INIT_CONN_MAX_INTV_VAR = 0x0028,
        .GAP_INQ_SCAN_INTV_VAR = 0x0012,
        .GAP_INQ_SCAN_WIND_VAR = 0x0012,
        .GAP_CONN_SUPERV_TIMEOUT_VAR = 0x07D0,
        .GAP_CONN_MIN_CE_VAR = 0x0000,
        .GAP_CONN_MAX_CE_VAR = 0xFFFF,
        .GAP_CONN_LATENCY_VAR = 0x0000,
        .GAP_MAX_LE_MTU_VAR = defaultBLE_MAX_MTU_SIZE,
        .GAP_DEV_NAME_VAR = "DIALOG SEMI-BLE",
};

/*
 * BLE stack variables
 */
extern char use_h4tl;                               // 0=GTL auto, 1=HCI auto, 8=GTL fix, 9=HCI fix
extern const struct gap_cfg_user_struct gap_cfg_user_var_struct;  // GAP configuration table
extern struct gap_cfg_user_struct *gap_cfg_user;    // Pointer to GAP configuration table

/// Variable storing the reason of platform reset
static uint32_t reset_reason __RETAINED; /* = RESET_NO_ERROR */

/*
 * BLE adapter/stack API hooks and variables
 */
extern bool ble_stack_initialized;
void ad_ble_stack_flow_on(void);
bool ad_ble_stack_flow_off(void);

const struct rwip_eif_api external_api = {
        .flow_off = ad_ble_stack_flow_off,
        .flow_on = ad_ble_stack_flow_on,
        .read = ad_ble_stack_read,
        .write = ad_ble_stack_write
};

/*
 * Last POWER_CTRL_REG on-wakeup/on-sleep values applied for CMAC
 */
__RETAINED static struct {
        uint32_t onwakeup_value;
        uint32_t onsleep_value;
} power_ctrl_reg_values;

/**
 ****************************************************************************************
 * @brief Updates the POWER_CTRL_REG on-wakeup/on-sleep values to be applied by CMAC
          accordingly.
 ****************************************************************************************
 */
void cmac_update_power_ctrl_reg_values(uint32_t onsleep_value)
{
        uint32_t onwakeup_value = onsleep_value;

        /* Workaround for "Errata issue 303": RF Variable Vref decreases with low VDDD */
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE, onwakeup_value, 1);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE, onwakeup_value, 0);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL, onwakeup_value, HW_PMU_1V2_VOLTAGE_1V2);

        GLOBAL_INT_DISABLE();

        power_ctrl_reg_values.onwakeup_value = onwakeup_value;
        power_ctrl_reg_values.onsleep_value = onsleep_value;

        if (cmac_dynamic_config_table_ptr) {
                cmac_dynamic_config_table_ptr->power_ctrl_reg_onwakeup_value = onwakeup_value;
                cmac_dynamic_config_table_ptr->power_ctrl_reg_onsleep_value = onsleep_value;
        }

        GLOBAL_INT_RESTORE();
}

/**
 ****************************************************************************************
 * @brief Retrieves the code and end base address of CMAC FW and sets up the memory
 *        controller accordingly.
 *
 * @Note This function gets called during execution of lib_ble_stack_init() and
 *       lib_ble_stack_reset() functions.
 ****************************************************************************************
 */
void cmac_mem_ctrl_setup(uint32_t *cmac_code_base_addr, uint32_t *cmac_end_base_addr)
{
    extern uint32_t cmi_fw_dst_addr;

    *cmac_code_base_addr = (uint32_t) &cmi_fw_dst_addr;
    *cmac_end_base_addr = (uint32_t) &__cmi_section_end__;
}

/**
 ****************************************************************************************
 * @brief Configures CMAC parameters.
 *
 * @Note This function gets called during execution of lib_ble_stack_init() and
 *       lib_ble_stack_reset() functions, at a point where CMAC is ready to start executing
 *       its main() function. CMAC execution will resume when this function returns.
 ****************************************************************************************
 */
void cmac_config_table_setup(void)
{
        cmac_configuration_table_t tmp_cfg_tbl;
        bool sleep_nvparam = false;

        ASSERT_ERROR( (dg_configBLE_PCLE_MIN_TX_PWR_IDX >= GAP_TX_POWER_MINUS_26_dBm && dg_configBLE_PCLE_MAX_TX_PWR_IDX <= GAP_TX_POWER_MAX) &&
                      (dg_configBLE_INITIAL_TX_POWER >= dg_configBLE_PCLE_MIN_TX_PWR_IDX && dg_configBLE_INITIAL_TX_POWER <= dg_configBLE_PCLE_MAX_TX_PWR_IDX) );

        /* Initialize CMAC configuration table */
        cmac_config_table_ptr->ble_length_exchange_needed = dg_configBLE_DATA_LENGTH_REQ_UPON_CONN;
        cmac_config_table_ptr->ble_rx_buffer_size         = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_RX_MAX + 11);
        cmac_config_table_ptr->ble_tx_buffer_size         = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_TX_MAX + 11);
        cmac_config_table_ptr->initial_tx_power_lvl       = dg_configBLE_INITIAL_TX_POWER;
        cmac_config_table_ptr->use_high_performance_1m    = dg_configBLE_USE_HIGH_PERFORMANCE_1M;
        cmac_config_table_ptr->use_high_performance_2m    = dg_configBLE_USE_HIGH_PERFORMANCE_2M;
        cmac_config_table_ptr->ble_dup_filter_max         = dg_configBLE_DUPLICATE_FILTER_MAX;
        cmac_config_table_ptr->golden_range_low           = dg_configBLE_GOLDEN_RANGE_LOW;
        cmac_config_table_ptr->golden_range_up            = dg_configBLE_GOLDEN_RANGE_UP;
        cmac_config_table_ptr->golden_range_pref          = dg_configBLE_GOLDEN_RANGE_PREF;
        cmac_config_table_ptr->pcle_min_tx_pwr_idx        = dg_configBLE_PCLE_MIN_TX_PWR_IDX;
        cmac_config_table_ptr->pcle_max_tx_pwr_idx        = dg_configBLE_PCLE_MAX_TX_PWR_IDX;

        cmac_system_tcs_length = cmac_config_table_ptr->system_tcs_length;
        cmac_synth_tcs_length  = cmac_config_table_ptr->synth_tcs_length;
        cmac_rfcu_tcs_length   = cmac_config_table_ptr->rfcu_tcs_length;

        cmac_config_table_ptr->system_tcs_length = 0;
        cmac_config_table_ptr->synth_tcs_length  = 0;
        cmac_config_table_ptr->rfcu_tcs_length   = 0;

        // LP clock type (frequency)
#if dg_configUSE_LP_CLK == LP_CLK_32768
        cmac_config_table_ptr->lp_clock_freq    = 0; //  32768Hz LP clock
#elif dg_configUSE_LP_CLK == LP_CLK_32000
        cmac_config_table_ptr->lp_clock_freq    = 1; //  32000Hz LP clock
#elif dg_configUSE_LP_CLK == LP_CLK_RCX
        cmac_config_table_ptr->lp_clock_freq    = 2; //  RCX
#else
        #error "The selected dg_configUSE_LP_CLK option is not supported by CMAC"
#endif

        /* Write already fetched public BD address to CMAC configuration table */
        ad_ble_get_public_address(cmac_config_table_ptr->ble_bd_address);

        /* Update POWER_CTRL_REG values */
        ASSERT_ERROR(power_ctrl_reg_values.onwakeup_value != 0
                && power_ctrl_reg_values.onsleep_value != 0);

        GLOBAL_INT_DISABLE();

        cmac_dynamic_config_table_ptr->power_ctrl_reg_onwakeup_value =
                power_ctrl_reg_values.onwakeup_value;
        cmac_dynamic_config_table_ptr->power_ctrl_reg_onsleep_value =
                power_ctrl_reg_values.onsleep_value;

        GLOBAL_INT_RESTORE();

        /*
         * Check NVPARAM for valid configuration values and write to proper CMAC configuration table
         */

        /* Static Configuration */
#if dg_configUSE_LP_CLK == LP_CLK_RCX
#if dg_configLP_CLK_DRIFT != 500
#error "500 PPM is the only valid option for dg_configLP_CLK_DRIFT when RCX is the low power clock."
#endif
        cmac_config_table_ptr->lp_clock_drift = dg_configLP_CLK_DRIFT;
#else
        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.lp_clock_drift, 2,
                                        NVPARAM_BLE_PLATFORM_LPCLK_DRIFT, NVPARAM_OFFSET_BLE_PLATFORM_LPCLK_DRIFT)) {
                cmac_config_table_ptr->lp_clock_drift = tmp_cfg_tbl.lp_clock_drift;
        } else {
                cmac_config_table_ptr->lp_clock_drift = dg_configLP_CLK_DRIFT;
        }
#endif

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_timer, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_TIMER_DUR)) {
                cmac_config_table_ptr->ble_chnl_assess_timer = tmp_cfg_tbl.ble_chnl_assess_timer;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_reassess_timer, 1,
                                        NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CRA_TIMER_DUR)) {
                cmac_config_table_ptr->ble_chnl_reassess_timer = tmp_cfg_tbl.ble_chnl_reassess_timer;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_min_rssi, 1,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_MIN_RSSI)) {
                cmac_config_table_ptr->ble_chnl_assess_min_rssi = tmp_cfg_tbl.ble_chnl_assess_min_rssi;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_nb_pkt, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_PKT)) {
                cmac_config_table_ptr->ble_chnl_assess_nb_pkt = tmp_cfg_tbl.ble_chnl_assess_nb_pkt;
        }
        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_nb_bad_pkt, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_BAD_PKT)) {
                cmac_config_table_ptr->ble_chnl_assess_nb_bad_pkt = tmp_cfg_tbl.ble_chnl_assess_nb_bad_pkt;
        }

        /* Dynamic Configuration */
        if (ad_ble_read_nvms_param((uint8_t *) &sleep_nvparam, 1,
                                        NVPARAM_BLE_PLATFORM_SLEEP_ENABLE, NVPARAM_OFFSET_BLE_PLATFORM_SLEEP_ENABLE)) {
                cmac_dynamic_config_table_ptr->sleep_enable = (bool) sleep_nvparam;
                ad_ble_stay_active(!sleep_nvparam);
        }

}

/**
 ****************************************************************************************
 * @brief Initializes the BLE stack
 *
 * @Note cmac_mem_ctrl_setup() and cmac_config_table_setup() will get called while
 *       executing lib_ble_stack_init() to configure the memory controller and the
 *       CMAC parameters.
 ****************************************************************************************
 */
void ble_stack_init(void)
{
        /* Make sure that LP clock is enabled */
        uint8_t lp_clk_sel = REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL);
        switch (lp_clk_sel)
        {
        case LP_CLK_IS_RC32K: /* RC32K */
                ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE));
                break;
        case LP_CLK_IS_RCX: /* RCX */
                ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE));
                break;
        case LP_CLK_IS_XTAL32K: /* XTAL32K through the oscillator with an external Crystal. */
                ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_XTAL32K_REG , XTAL32K_ENABLE));
                break;
        case LP_CLK_IS_EXTERNAL: /* XTAL32K through an external square wave generator */
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        /* Initialize BLE timer */
        timer_init();

        /* Initialize BLE stack (Controller and Host) */
        lib_ble_stack_init();
}

/**
 ****************************************************************************************
 * @brief Resets the BLE controller
 *
 * @Note cmac_mem_ctrl_setup() and cmac_config_table_setup() will get called while
 *       executing lib_ble_stack_reset() to configure the memory controller and the
 *       CMAC parameters.
 ****************************************************************************************
 */
void ble_controller_reset(void)
{
        bool sleep_enable = cmac_dynamic_config_table_ptr->sleep_enable;

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
        /* Disable temperature monitoring for calibration if enabled */
        if (ad_ble_temp_meas_enabled) {
                sys_temp_meas_disable();
                ad_ble_temp_meas_enabled = false;
        }
#endif

        /* Reset the controller */
        lib_ble_stack_reset(0);

        /* Restore CMAC sleep enable value */
        cmac_dynamic_config_table_ptr->sleep_enable = sleep_enable;
}

/**
 * @brief       Wake up CMAC
 *
 * @details     If CMAC CPU is sleeping, then it will be woken up.
 *              if CMAC CPU is on its way to enter sleep mode (i.e. it's past the
 *              sleep decision point but has not entered sleep mode yet), then it
 *              will not enter sleep mode.
 *
 * @return      bool
 *
 * @retval      false, if the CMAC was not sleeping when this function was called
 * @retval      true,  if the CMAC was sleeping when this function was called
 *
 */
bool ble_force_wakeup(void)
{
#if (USE_BLE_SLEEP == 1)
        bool retval;

        retval = cmac_cpu_wakeup();

        return retval;
#else
        return true;
#endif /* (USE_BLE_SLEEP == 1) */
}

__WEAK void ble_controller_error(void)
{
        ASSERT_ERROR(0);
}

void sys_cmac_on_error_handler(void)
{
        ble_controller_error();
}

#ifdef RAM_BUILD
void platform_reset_sdk(uint32_t error)
{
        reset_reason = error;

        ASSERT_ERROR(0);
}
#endif

int dia_rand_func(void)
{
        return rand();
}

void dia_srand_func (unsigned int seed)
{
        srand(seed);
}

void ble_platform_initialization(void)
{
        use_h4tl = 0;   // 0 = GTL auto
        gap_cfg_user = (struct gap_cfg_user_struct *) &gap_cfg_user_var_struct;
}

/**
 * @brief       Check if the BLE stack has pending actions.
 *
 * @return      bool
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> false, if the BLE stack has pending actions
 *                  <li> true, if the BLE stack has finished
 *              </ul>
 *
 */
bool ble_block(void)
{
        bool result = false;

        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (ke_sleep_check() && ble_stack_initialized) {
                result = true;
        }

        return result;
}

/*
 * @brief Initialize BLE timer
 */
void timer_init(void)
{
        ble_timer = OS_TIMER_CREATE("ble_tmr", 100 /* Dummy */, 0, 0, ble_timer_callback);
        OS_ASSERT(ble_timer);
}

/*
 * @brief Retrieve current time.
 *
 * @return  current time in 10ms step.
 */
uint32_t timer_get_time(void)
{
        uint32_t current_time_10ms;

        current_time_10ms = (OS_TICKS_2_MS(OS_GET_TICK_COUNT()) / 10) & KE_GROSSTIMER_MASK;

        return current_time_10ms;
}

/*
 * @brief Sets the absolute expiration time for the first timer in queue.
 *
 * @param to The absolute expiration time in units of 10ms
 */
void timer_set_timeout(uint32_t to)
{
        OS_BASE_TYPE ret;
        uint32_t current_time_10ms, timeout_10ms;

        to = to & KE_GROSSTIMER_MASK;
        current_time_10ms = timer_get_time() & KE_GROSSTIMER_MASK;
        timeout_10ms = (to - current_time_10ms) & KE_GROSSTIMER_MASK;

        if (!timeout_10ms) {
                timeout_10ms++;
        }

        OS_ASSERT(ble_timer);

        if (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
                ret = OS_TIMER_CHANGE_PERIOD_FROM_ISR(ble_timer, OS_TIME_TO_TICKS(timeout_10ms * 10));
        } else {
                ret = OS_TIMER_CHANGE_PERIOD(ble_timer, OS_TIME_TO_TICKS(timeout_10ms * 10), OS_TIMER_FOREVER);
        }

        OS_ASSERT(ret == OS_TIMER_SUCCESS);
}

/*
 * @brief Enables or disables the timer
 */
void timer_enable(bool enable)
{
        OS_BASE_TYPE ret;

        OS_ASSERT(ble_timer);

        if (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) {
                if (enable) {
                        ret = OS_TIMER_START_FROM_ISR(ble_timer);
                } else {
                        ret = OS_TIMER_STOP_FROM_ISR(ble_timer);
                }
        } else {
                if (enable) {
                        ret = OS_TIMER_START(ble_timer, OS_TIMER_FOREVER);
                } else {
                        ret = OS_TIMER_STOP(ble_timer, OS_TIMER_FOREVER);
                }
        }

        OS_ASSERT(ret == OS_TIMER_SUCCESS);
}

void sys_temp_meas_enable(void)
{
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
        ad_ble_task_notify(mainBIT_TEMP_MONITOR_ENABLE);
#endif
}

void sys_temp_meas_disable(void)
{
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
        ad_ble_task_notify(mainBIT_TEMP_MONITOR_DISABLE);
#endif
}

#endif /* CONFIG_USE_BLE */
