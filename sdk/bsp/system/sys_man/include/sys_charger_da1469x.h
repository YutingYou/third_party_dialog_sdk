/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_CHARGER System Charger
 *
 * \brief System Charger
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_charger_da1469x.h
 *
 * @brief System Charger header file.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_CHARGER == 1)

#ifndef SYS_CHARGER_DA1469x_H_
#define SYS_CHARGER_DA1469x_H_

#include "sdk_defs.h"
#include "hw_charger.h"


/**
 * \brief System charging configuration.
 */
typedef struct {
        hw_charger_charging_profile_t hw_charging_profile;      /**< Profile for programming the HW FSM. */
} sys_charger_configuration_t;

/**
 * \brief Initialize sys_charger service.
 *
 * \param [in] conf: Charging configuration.
 */
void sys_charger_init(const sys_charger_configuration_t* conf);

/******************** Weak functions to be overridden, if needed, by the application code *********/

/**
 * \brief Notification hook to be called when HW FSM is disabled.
 *
 */
__WEAK void sys_charger_ext_hook_hw_fsm_disabled(void);

/**
 * \brief Notification hook to be called in pre-charging state.
 *
 */
__WEAK void sys_charger_ext_hook_precharging(void);

/**
 * \brief Notification hook to be called in charging state.
 *
 */
__WEAK void sys_charger_ext_hook_charging(void);

/**
 * \brief Notification hook to be called when the end of charge (EoC) is reached.
 *
 */
__WEAK void sys_charger_ext_hook_charged(void);

/**
 * \brief Notification hook to be called when TBAT exceeds its defined limits.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_tbat_error(void);

/**
 * \brief Notification hook to be called when TDIE exceeds its defined limits.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_tdie_error(void);

/**
 * \brief Notification hook to be called when over-voltage occurs.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_ovp_error(void);

/**
 * \brief Notification hook to be called when timeout occurs during the total
 *        charging cycle.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_total_charge_timeout(void);

/**
 * \brief Notification hook to be called when timeout occurs during the constant voltage
 *        period.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_cv_charge_timeout(void);

/**
 * \brief Notification hook to be called when timeout occurs during the constant current
 *        period.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_cc_charge_timeout(void);

/**
 * \brief Notification hook to be called when timeout occurs during the pre-charging
 *        period.
 * \see hw_charger_charging_profile_t
 */
__WEAK void sys_charger_ext_hook_pre_charge_timeout(void);


#endif /* SYS_CHARGER_DA1469x_H_ */

#endif /* (dg_configUSE_SYS_CHARGER == 1) */

/**
 \}
 \}
 */
