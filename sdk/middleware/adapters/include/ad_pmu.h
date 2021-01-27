/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup PMU_ADAPTER PMU Adapter
 *
 * \brief Power Management Unit adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_pmu.h
 *
 * @brief PMU adapter API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_PMU_H_
#define AD_PMU_H_

#if dg_configPMU_ADAPTER

#include <hw_pmu.h>
#include <hw_sys.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Rail selection
 *
 */
typedef enum {
        PMU_RAIL_1V4,           //!< 1V4 rail
        PMU_RAIL_1V2,           //!< 1V2 rail
        PMU_RAIL_1V8,           //!< 1V8 rail
        PMU_RAIL_1V8P,          //!< 1V8P rail
        PMU_RAIL_3V0,           //!< 3V0 rail
        NUM_OF_PMU_RAILS
} AD_PMU_RAIL;

/**
 * \brief Rail configuration
 *
 */
typedef struct {
        bool enabled;                                   //!< true if rail is enabled in active state
        bool enabled_on_sleep;                          //!< true if rail is enabled in sleep state
        union {
                struct {
                        HW_PMU_3V0_VOLTAGE voltage;     //!< 3V0 rail voltage configuration
                        HW_PMU_3V0_MAX_LOAD current;    //!< 3V0 rail current configuration
                } rail_3v0;                             //!< 3V0 rail voltage and current configuration

                struct {
                        HW_PMU_1V2_VOLTAGE voltage;     //!< 1V2 rail voltage configuration
                        HW_PMU_1V2_MAX_LOAD current;    //!< 1V2 rail current configuration
                } rail_1v2;                             //!< 1V2 rail voltage and current configuration

                struct {
                        HW_PMU_1V4_VOLTAGE voltage;     //!< 1V4 rail voltage configuration
                } rail_1v4;                             //!< 1V4 rail voltage configuration

                struct {
                        HW_PMU_1V8_VOLTAGE voltage;     //!< 1V8 rail voltage configuration
                        HW_PMU_1V8_MAX_LOAD current;    //!< 1V8 rail current configuration
                } rail_1v8;                             //!< 1V8 rail voltage and current configuration

                struct {
                        HW_PMU_1V8_MAX_LOAD current;    //!< 1V8 rail current configuration
                } rail_1v8p;                            //!< 1V8 rail current configuration
        };                                              //!< rail voltage and current configuration
        bool use_dcdc;                                  //!< true if rail is powered by the DC
} ad_pmu_rail_config_t;

#define PMU_NUM_OF_DCDC_RAILS   4

/**
 * \brief DCDC configuration
 *
 */
typedef struct {
        hw_sys_reg_config_t rail[PMU_NUM_OF_DCDC_RAILS];
        hw_sys_reg_config_t ctrl1; //!< DCDC_CTRL1_REG configuration
} pmu_dcdc_config_t;

/**
 * \brief Sets the state of the 1v8 rail.
 *
 * \param[in] state If true, the 1v8 rail state is set according to dg_configPOWER_1V8_ACTIVE
 *                  and dg_configPOWER_1V8_SLEEP macros.
 *                  If false, the 1v8 rail is turned off.
 * \note If dg_configPOWER_1V8_ACTIVE or dg_configPOWER_1V8_SLEEP is equal to 2 then the
 * state of the rail is not affected by this function
 * This function should only be called by the power manager. Application should call function
 * pm_set_1v8_state() instead.
 *
 */
void ad_pmu_set_1v8_state(bool state);

/**
 * \brief Gets the state of the 1v8 rail
 *
 * \return false if the 1v8 rail is off or not controlled by dg_configPOWER_1V8_ACTIVE and
 *         dg_configPOWER_1V8_SLEEP macros.
 *         true if it is controlled by dg_configPOWER_1V8_ACTIVE and dg_configPOWER_1V8_SLEEP
 *         macros and the rail is powered.
 *
 * \note This function should only be called by the power manager. Application should call function
 * pm_get_1v8_state() instead.
 */
bool ad_pmu_get_1v8_state(void);

/**
 * \brief Configure a power rail
 *
 * \return 0 if the rail is configured, >0 otherwise
 *
 * \param[in] rail the rail to configure
 * \param[in] config a pointer to the structure holding the rail configuration
 */
int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_H_ */

/**
 * \}
 * \}
 */
