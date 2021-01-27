/**
 ****************************************************************************************
 *
 * @file ad_pmu_internal.h
 *
 * @brief PMU internal adapter API - Should be excluded from documentation
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_PMU_INTERNAL_H_
#define AD_PMU_INTERNAL_H_

#if dg_configPMU_ADAPTER

#include "hw_pmu.h"

/**
 * \brief Suspend DC/DC converter operation
 *
 * If DC/DC converter is enabled, then disable it without changing the configuration
 */
void ad_pmu_dcdc_suspend(void);

/**
 * \brief Resume DC/DC converter operation
 *
 * If DC/DC converter operation is suspended, enable it if there is still at least one rail
 * configured to be powered by the DC/DC converter
 */
void ad_pmu_dcdc_resume(void);


/**
 * \brief Requests the system to force the 1V2 rail voltage to a level
 *
 * The 1V2 rail voltage is requested to temporarily be set to the defined value. LDO_CORE is
 * enabled if required, since LDO_CORE_RET voltage cannot be set to a level greater than 0.9V.
 * The system rail configuration is not changed. It will be restored when
 * ad_pmu_1v2_force_max_voltage_release() is called equal times ad_pmu_1v2_force_max_voltage_request()
 * has been called.
 *
 * \warning The PM Adapter API user MUST ensure that any request is matched by the respective release.
 *          Otherwise the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_request(void);

/**
 * \brief Restore the 1V2 rail configuration. It terminates a matching request.
 *
 * The 1V2 rail is restored to the configuration that was applied before calling
 * ad_pmu_1v2_force_max_voltage_request().
 *
 * \warning This function MUST be called always to terminate a matching ad_pmu_1v2_force_max_voltage_request().
 *          If called alone the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_release(void);

/**
 * \brief Prepare for sleep. Configure 1V2 rail at 0.9V if enabled in sleep
 */
void ad_pmu_prepare_for_sleep(void);

/**
 * \brief Restore for system wake-up. Restore 1V2 rail configuration
 */
void ad_pmu_restore_for_wake_up(void);

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_INTERNAL_H_ */
