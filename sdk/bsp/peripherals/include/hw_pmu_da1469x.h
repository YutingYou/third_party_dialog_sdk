/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_PMU Power Manager Driver
\{
\brief Power Manager
*/

/**
****************************************************************************************
*
* @file hw_pmu_da1469x.h
*
* @brief Power Manager header file for DA1469x.
*
* Copyright (C) 2015-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_PMU_DA1469x_H_
#define HW_PMU_DA1469x_H_


#if dg_configUSE_HW_PMU

#include "sdk_defs.h"

/**
 * \brief PMU API Error Codes
 *
 */
typedef enum {
        HW_PMU_ERROR_NOERROR                = 0,    //!< No Error
        HW_PMU_ERROR_INVALID_ARGS           = 1,    //!< Invalid arguments
        HW_PMU_ERROR_NOT_ENOUGH_POWER       = 2,    //!< Current LDO config cannot supply enough power for this config
        HW_PMU_ERROR_PLL96M_ON              = 3,    //!< PLL96M is on
        HW_PMU_ERROR_XTAL32M_ON             = 4,    //!< XTAL32M is on
        HW_PMU_ERROR_USB_PHY_ON             = 5,    //!< USB_PHY is on
        HW_PMU_ERROR_RC32K_ON               = 6,    //!< RC32K is on
        HW_PMU_ERROR_XTAL32K_ON             = 7,    //!< XTAL32K is on
        HW_PMU_ERROR_UFAST_WAKEUP_ON        = 8,    //!< Ultra fast wakeup is on
        HW_PMU_ERROR_ACTION_NOT_POSSIBLE    = 9,    //!< Action not possible to execute
        HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY = 10,   //!< Other loads dependency
        HW_PMU_ERROR_BOD_IS_ACTIVE          = 11,   //!< BOD is active
        HW_PMU_ERROR_BOD_THRESHOLD          = 12,   //!< BOD threshold level
        HW_PMU_ERROR_SLEEP_LDO              = 13,   //!< Sleep LDO configured with voltage below 0.9V
} HW_PMU_ERROR_CODE;

/**
 * \brief PMU API Source type
 *
 * This allows the user to select whether he wants to use a high-efficiency and high-ripple
 * source (DCDC) or low-efficiency and low-ripple source (LDO) for a Power Rail
 */
typedef enum {
        HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE       = 0,   //!< Low ripple source (LDO)
        HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY = 1,   //!< High efficiency (and ripple) source (DCDC)
} HW_PMU_SRC_TYPE;

/**
 * \brief Voltage level options for the 3V0 power rail
 *
 */
typedef enum {
        HW_PMU_3V0_VOLTAGE_2V4  = 0,             //!< 2.40V
        HW_PMU_3V0_VOLTAGE_3V0  = 1,             //!< 3.00V
        HW_PMU_3V0_VOLTAGE_3V3  = 2,             //!< 3.30V
        HW_PMU_3V0_VOLTAGE_3V45 = 3              //!< 3.45V
} HW_PMU_3V0_VOLTAGE;

/**
 * \brief Voltage level options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_VOLTAGE_1V2 = 0,              //!< 1.2V
        HW_PMU_1V8_VOLTAGE_1V8 = 1               //!< 1.8V
} HW_PMU_1V8_VOLTAGE;

/**
 * \brief Voltage level options for the 1V4 power rail
 *
 */
typedef enum {
        HW_PMU_1V4_VOLTAGE_1V20 = 0,              //!< 1.20V
        HW_PMU_1V4_VOLTAGE_1V25 = 1,              //!< 1.25V
        HW_PMU_1V4_VOLTAGE_1V30 = 2,              //!< 1.30V
        HW_PMU_1V4_VOLTAGE_1V35 = 3,              //!< 1.35V
        HW_PMU_1V4_VOLTAGE_1V40 = 4,              //!< 1.40V
        HW_PMU_1V4_VOLTAGE_1V45 = 5,              //!< 1.45V
        HW_PMU_1V4_VOLTAGE_1V50 = 6,              //!< 1.50V
        HW_PMU_1V4_VOLTAGE_1V55 = 7               //!< 1.55V
} HW_PMU_1V4_VOLTAGE;

/**
 * \brief Voltage level options for the 1V2 power rail in active mode
 *
 */
typedef enum {
        HW_PMU_1V2_VOLTAGE_0V9  = 0,             //!< 0.9V
        HW_PMU_1V2_VOLTAGE_1V0  = 1,             //!< 1.0V
        HW_PMU_1V2_VOLTAGE_1V1  = 2,             //!< 1.1V
        HW_PMU_1V2_VOLTAGE_1V2  = 3              //!< 1.2V
} HW_PMU_1V2_VOLTAGE;

/**
 * \brief Voltage level options for the 1V2 power rail in sleep mode
 *
 */
typedef enum {
        HW_PMU_1V2_SLEEP_VOLTAGE_0V75 = 0,       //!< 0.75V
        HW_PMU_1V2_SLEEP_VOLTAGE_0V8  = 1,       //!< 0.80V
        HW_PMU_1V2_SLEEP_VOLTAGE_0V85 = 2,       //!< 0.85V
        HW_PMU_1V2_SLEEP_VOLTAGE_0V9  = 3,       //!< 0.90V
        HW_PMU_1V2_SLEEP_VOLTAGE_0V95 = 4,       //!< 0.95V
        HW_PMU_1V2_SLEEP_VOLTAGE_1V0  = 5,       //!< 1.00V
} HW_PMU_1V2_SLEEP_VOLTAGE;

/**
 * \brief Voltage level options for the 1V2 clamp
 *
 */
typedef enum {
        HW_PMU_VDD_VOLTAGE_1037  = 0,            //!< 1.037V
        HW_PMU_VDD_VOLTAGE_1005  = 1,            //!< 1.005V
        HW_PMU_VDD_VOLTAGE_978   = 2,            //!< 0.978V
        HW_PMU_VDD_VOLTAGE_946   = 3,            //!< 0.946V
        HW_PMU_VDD_VOLTAGE_1120  = 4,            //!< 1.120V
        HW_PMU_VDD_VOLTAGE_1089  = 5,            //!< 1.089V
        HW_PMU_VDD_VOLTAGE_1058  = 6,            //!< 1.058V
        HW_PMU_VDD_VOLTAGE_1030  = 7,            //!< 1.030V
        HW_PMU_VDD_VOLTAGE_952   = 8,            //!< 0.952V
        HW_PMU_VDD_VOLTAGE_918   = 9,            //!< 0.918V
        HW_PMU_VDD_VOLTAGE_889   = 10,           //!< 0.889V
        HW_PMU_VDD_VOLTAGE_861   = 11,           //!< 0.861V
        HW_PMU_VDD_VOLTAGE_862   = 12,           //!< 0.862V
        HW_PMU_VDD_VOLTAGE_828   = 13,           //!< 0.828V
        HW_PMU_VDD_VOLTAGE_798   = 14,           //!< 0.798V
        HW_PMU_VDD_VOLTAGE_706   = 15            //!< 0.706V
} HW_PMU_VDD_CLAMP_VOLTAGE;

/**
 * \brief Maximum load current options for the 3V0 power rail
 *
 */
typedef enum {
        HW_PMU_3V0_MAX_LOAD_1   = 0,             //!< 1mA
        HW_PMU_3V0_MAX_LOAD_10  = 1,             //!< 10mA
        HW_PMU_3V0_MAX_LOAD_150 = 2,             //!< 150mA
} HW_PMU_3V0_MAX_LOAD;

/**
 * \brief Maximum load current options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_MAX_LOAD_10 = 0,              //!< 10mA
        HW_PMU_1V8_MAX_LOAD_50 = 1,              //!< 50mA
} HW_PMU_1V8_MAX_LOAD;

/**
 * \brief Maximum load current options for the 1V2 power rail
 *
 */
typedef enum {
        HW_PMU_1V2_MAX_LOAD_1  = 0,              //!< 1mA
        HW_PMU_1V2_MAX_LOAD_50 = 1,              //!< 50mA
} HW_PMU_1V2_MAX_LOAD;

/**
 * \brief Power rail state (enabled or disabled)
 *
 */
typedef enum {
        POWER_RAIL_DISABLED = 0,
        POWER_RAIL_ENABLED  = 1
} HW_PMU_POWER_RAIL_STATE;

/**
 * \brief 3V0 power rail configuration
 *
 */
typedef struct {
        HW_PMU_3V0_VOLTAGE voltage;
        HW_PMU_3V0_MAX_LOAD current;
} HW_PMU_3V0_RAIL_CONFIG;

/**
 * \brief 1V8 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8_VOLTAGE voltage;
        HW_PMU_1V8_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8_RAIL_CONFIG;

/**
 * \brief 1V8P power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8_VOLTAGE voltage;
        HW_PMU_1V8_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8P_RAIL_CONFIG;

/**
 * \brief 1V4 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V4_VOLTAGE voltage;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V4_RAIL_CONFIG;

/**
 * \brief 1V2 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V2_VOLTAGE voltage;
        HW_PMU_1V2_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V2_RAIL_CONFIG;

/**
 * \brief Configure and enable DC/DC converter
 *
 */
void hw_pmu_dcdc_config(void);

/**
 * \brief Enable DC/DC converter
 *
 */
__STATIC_INLINE void hw_pmu_dcdc_enable(void)
{
        REG_SET_BIT(DCDC, DCDC_CTRL1_REG, DCDC_ENABLE);
}

/**
 * \brief Disable DC/DC converter
 *
 */
__STATIC_INLINE void hw_pmu_dcdc_disable(void)
{
        REG_CLR_BIT(DCDC, DCDC_CTRL1_REG, DCDC_ENABLE);
}

/**
 * \brief Get DC/DC converter status
 *
 * \return true if DC/DC converter is enabled
 */
__STATIC_INLINE bool hw_pmu_dcdc_is_enabled(void)
{
        return REG_GETF(DCDC, DCDC_CTRL1_REG, DCDC_ENABLE) == 1;
}

/**
 * \brief Enable 3V0 clamp
 *
 * This allows the user to enable clamp that can supply 3V0 rail.
 */
void hw_pmu_3v0_clamp_enable(void);

/**
 * \brief Disable 3V0 clamp
 *
 * This allows the user to disable clamp that can supply 3V0 rail.
 */
void hw_pmu_3v0_clamp_disable(void);

/**
 * \brief Set 3V0 rail voltage level
 *
 * This function sets the voltage level of the 3V0 rail.
 * The voltage level setting is applied when the rail is powered by the LDOs.
 * When the rail is powered by clamp the voltage level is always 2.4V.
 *
 * \param[in] voltage The voltage to set the rail to
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters:
 *  HW_PMU_3V0_VOLTAGE_3V0
 *  HW_PMU_3V0_VOLTAGE_3V3
 *  HW_PMU_3V0_VOLTAGE_3V45
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_set_voltage(HW_PMU_3V0_VOLTAGE voltage);

/**
 * \brief Set 3V0 rail wakeup configuration
 *
 * This function sets the 3V0 rail configuration for the wakeup state of the system. This is
 * effective immediately.
 * Depending on the input parameter the appropriate power source will be selected:
 * - High current - Enable LDO_VBAT and LDO_VBUS (Disable LDO_VBAT_RET)
 * - Low current - Enable LDO_VBAT_RET (Disable LDO_VBAT and LDO_VBUS)
 *
 * If other LDOs are active (LDO_1V2, LDO_1V4, LDO_1V8 or LDO_1V8P) and the voltage and
 * current parameters do not fulfill their requirements, then error code
 * HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY will be returned. In this case the new parameters
 * will not be applied.
 *
 * \param[in] max_load The maximum load to be driven by the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters:
 *  max load                  Source
 *  HW_PMU_3V0_MAX_LOAD_1     3V0 VSYS_CLAMP
 *  HW_PMU_3V0_MAX_LOAD_10    3V0 LDO_VBAT_RET
 *  HW_PMU_3V0_MAX_LOAD_150   3V0 LDO_VBAT or LDO_VBUS
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_config(HW_PMU_3V0_MAX_LOAD max_load);

/**
 * \brief Set 3V0 rail sleep configuration
 *
 * This function sets the 3V0 rail configuration for the sleep state of the system.
 * Depending on the input parameter the appropriate power source will be selected:
 * - High voltage  - Enable LDO_VBAT_RET (do not disable clamp)
 * - Low voltage (2,4V) - Enable vsys_clamp (disable LDO_VBAT_RET)
 *
 * If other LDOs are active (LDO_1V2, LDO_1V4, LDO_1V8 or LDO_1V8P) and the voltage and
 * current parameters do not fulfill their requirements, then error code
 * HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY will be returned. In this case the new parameters
 * will not be applied.
 *
 * \param[in] max_load The maximum load to be driven by the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters:
 *  max load                 Source
 *  HW_PMU_3V0_MAX_LOAD_1    3V0 VSYS_CLAMP
 *  HW_PMU_3V0_MAX_LOAD_10   3V0 LDO_VBAT_RET
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_config(HW_PMU_3V0_MAX_LOAD max_load);

/**
 * \brief Set 1V8 rail voltage level
 *
 * This function sets voltage level of the 1V8 rail. This is effective immediately. This voltage
 * is applied to all 1V8 rail states (active, wakeup, sleep).
 *
 * \param[in] voltage The voltage to set the rail to
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the voltage level has been set, or an error code otherwise.

 *  Valid input parameters:
 *  voltage[V]
 *  HW_PMU_1V8_VOLTAGE_1V2
 *  HW_PMU_1V8_VOLTAGE_1V8
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_set_voltage(HW_PMU_1V8_VOLTAGE voltage);

/**
 * \brief Configure DC/DC converter for the 1V8 rail in active state.
 *
 * This function configures but does not enable the DC/DC converter for the 1V8
 * rail in active state.
 */
void hw_pmu_1v8_configure_high_efficiency_dcdc(void);

/**
 * \brief Enable DC/DC converter for 1V8 rail in active state
 *
 * This function enables DC/DC converter for the 1V8 rail in active state.
 * This is effective immediately.
 */
void hw_pmu_1v8_enable_high_efficiency_dcdc(void);

/**
 * \brief Disable DC/DC converter 1V8 rail in active state
 *
 * This function disables 1V8 DC/DC converter power source. If no other DC/DC rails
 * are enabled, then DC/DC converter will be turned off.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_disable_high_efficiency_dcdc(void);

/**
 * \brief Set 1V8 rail wakeup configuration
 *
 * This function sets the 1V8 rail configuration for the wakeup state of the system. This is
 * effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_IO (Disable LDO_IO_RET)
 * - Low current - Enable LDO_IO_RET (Disable LDO_IO)
 * The LDO source cannot be enabled if the power rail that supplies the LDO is off or it
 * cannot provide enough current. In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA             Source
 *  HW_PMU_1V8_MAX_LOAD_10  LDO_IO_RET
 *  HW_PMU_1V8_MAX_LOAD_50  LDO_IO
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8 rail in wakeup state
 *
 * This function disables all 1V8 LDO power sources used for wakeup and active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void);

/**
 * \brief Enable 1V8 rail in sleep state
 *
 * This function enables the 1V8 rail in the sleep state of the system.
 * LDO_IO_RET is the only available power source in sleep state.
 * The LDO_IO_RET source cannot be enabled if the power rail that supplies the LDO is off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(void);

/**
 * \brief Disable 1V8 rail in sleep state
 *
 * This function disables 1V8 LDO_IO_RET that is used for sleep state.
 */
void hw_pmu_1v8_onsleep_disable(void);

/**
 * \brief Configure DC/DC converter for the 1V8P rail in active state.
 *
 * This function configures but does not enable the DC/DC converter for the 1V8P
 * rail in active state.
 */
void hw_pmu_1v8p_configure_high_efficiency_dcdc(void);

/**
 * \brief Enable DC/DC converter for 1V8P rail in active state
 *
 * This function enables DC/DC converter for the 1V8P rail in active state.
 * This is effective immediately.
 */
void hw_pmu_1v8p_enable_high_efficiency_dcdc(void);

/**
 * \brief Disable DC/DC converter 1V8P rail in active state
 *
 * This function disables 1V8P DC/DC converter power source. If no other DC/DC rails
 * are enabled, then DC/DC converter will be turned off.
 * DC/DC converter source cannot be disabled if GPIOs are powered by 1V8P rail and there is no LDO
 * supplying the rail. In this case error code HW_PMU_ERROR_ACTION_NOT_POSSIBLE will be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_disable_high_efficiency_dcdc(void);

/**
 * \brief Set 1V8P rail wakeup configuration
 *
 * This function sets the 1V8P rail configuration for the wakeup state of the system. This is
 * effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_IO2 (Disable LDO_IO_RET2)
 * - Low current - Enable LDO_IO_RET2 (Disable LDO_IO2)
 * The LDO source cannot be enabled if the power rail that supplies the LDO is off or it
 * cannot provide enough current. In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load                 Source
 *  HW_PMU_1V8_MAX_LOAD_10   LDO_IO_RET2
 *  HW_PMU_1V8_MAX_LOAD_50   LDO_IO2
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8P rail in wakeup state
 *
 *  This function disables all 1V8P LDO power sources used for wakeup and active state.
 *  LDO sources cannot be disabled if GPIOs are powered by 1V8P rail. In this case error code
 *  HW_PMU_ERROR_ACTION_NOT_POSSIBLE will be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_disable(void);

/**
 * \brief Set 1V8P rail sleep configuration
 *
 * This function sets the 1V8P rail configuration for the sleep state of the system.
 * LDO_IO2_RET is the only available power source in sleep state.
 * The LDO_IO2_RET source cannot be enabled if the power rail that supplies the LDO is off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *                 error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_enable(void);

/**
 * \brief Disable 1V8P rail in sleep state
 *
 *  This function disables 1V8P LDO_IO2_RET that is used for sleep state.
 *  LDO source cannot be disabled if GPIOs are powered by 1V8P rail. In this case error code
 *  HW_PMU_ERROR_ACTION_NOT_POSSIBLE will be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *                 error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_disable(void);

/**
 * \brief Enable 1V8F rail switch
 *
 * 1V8F rail is powered by 1V8P rail. Make sure that 1V8P rail is properly configured for all
 * states (active, wakeup, sleep).
 */
__STATIC_FORCEINLINE void hw_pmu_1v8f_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, SW_1V8F_ENABLE_FORCE);
}

/**
 * \brief Disable 1V8F rail switch
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *                 error code otherwise.
 */
__STATIC_FORCEINLINE HW_PMU_ERROR_CODE hw_pmu_1v8f_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, SW_1V8F_ENABLE_FORCE);

        return HW_PMU_ERROR_NOERROR;
}

/**
 * \brief Set 1V4 rail voltage level
 *
 * This function sets the voltage level of the 1V4 rail. This is effective immediately.
 *
 * \param[in] voltage The voltage to set the rail to. This voltage is applied to all 1V4 rail
 *            states (active, wakeup).
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail voltage has been set, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  HW_PMU_1V4_VOLTAGE_1V20
 *  HW_PMU_1V4_VOLTAGE_1V25
 *  HW_PMU_1V4_VOLTAGE_1V30
 *  HW_PMU_1V4_VOLTAGE_1V35
 *  HW_PMU_1V4_VOLTAGE_1V40
 *  HW_PMU_1V4_VOLTAGE_1V45
 *  HW_PMU_1V4_VOLTAGE_1V50
 *  HW_PMU_1V4_VOLTAGE_1V55
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_set_voltage(HW_PMU_1V4_VOLTAGE voltage);

/**
 * \brief Configure DC/DC converter for the 1V4 rail in active state.
 *
 * This function configures but does not enable the DC/DC converter for the 1V4
 * rail in active state.
 */
void hw_pmu_1v4_configure_high_efficiency_dcdc(void);

/**
 * \brief Enable DC/DC converter for the 1V4 rail in active state.
 *
 * This function enables DC/DC converter for the 1V4 rail in active state.
 * This is effective immediately.
 */
void hw_pmu_1v4_enable_high_efficiency_dcdc(void);

/**
 * \brief Disable DC/DC converter for the 1V4 rail in active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the DC/DC converter has been disabled,
 *         or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_disable_high_efficiency_dcdc(void);

/**
 * \brief Enable 1V4 rail for wakeup
 *
 * This function enables the 1V4 rail for the wakeup state of the system.
 * This is effective immediately.
 * The LDO source cannot be enabled if the power rail that supplies the LDO is off or it
 * cannot provide enough current. In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *         error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_enable(void);

/**
 * \brief Disable 1V4 rail in wakeup state
 *
 * This function disables 1V4 LDO power source used for wakeup and active state.
 * LDO source cannot be disabled if hardware blocks are powered by 1V4 rail. In this case the
 * appropriate error code will be returned:
 *  - HW_PMU_ERROR_ACTION_NOT_POSSIBLE: Radio is enabled
 *  - HW_PMU_ERROR_PLL96M_ON: PLL is enabled and used as system clock
 *  - HW_PMU_ERROR_XTAL32M_ON: XTAL32M is enabled
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_disable(void);

/**
 * \brief Set 1V2 rail voltage level
 *
 * This function sets the wakeup voltage level of the 1V2 rail. This is effective immediately.
 * This voltage is applied to active and wakeup 1V2 rail states.
 *
 * \param[in] voltage The voltage to set the rail to.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail voltage has been set, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  voltage[V]
 *  HW_PMU_1V2_VOLTAGE_0V9
 *  HW_PMU_1V2_VOLTAGE_1V0
 *  HW_PMU_1V2_VOLTAGE_1V1
 *  HW_PMU_1V2_VOLTAGE_1V2
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_set_voltage(HW_PMU_1V2_VOLTAGE voltage);

/**
 * \brief Configure DC/DC converter for the 1V2 rail in active state.
 *
 * This function configures but does not enable the DC/DC converter for the 1V2
 * rail in active state.
 */
void hw_pmu_1v2_configure_high_efficiency_dcdc(void);

/**
 * \brief Enable DC/DC converter for the 1V2 rail in active state.
 *
 * This function enables DC/DC converter for the 1V2 rail in active state.
 * This is effective immediately.
 */
void hw_pmu_1v2_enable_high_efficiency_dcdc(void);

/**
 * \brief Disable DC/DC converter for the 1V2 rail in active state.
 *
 * This function disables 1V2 DC/DC converter power source. If no other DC/DC rails
 * are enabled, then DC/DC converter will be turned off.
 * LDO power source can only be disabled by executing hw_pmu_1v2_onwakeup_disable().
 * DC/DC converter source cannot be disabled if hardware blocks are powered by 1V2 rail and there
 * is no LDO supplying the rail. In this case the appropriate error code will be returned:
 *  - HW_PMU_ERROR_PLL96M_ON: PLL96M is enabled and used as system clock
 *  - HW_PMU_ERROR_USB_PHY_ON: USB_PHY is enabled
 *  - HW_PMU_ERROR_XTAL32K_ON: XTAL32K is enabled
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_disable_high_efficiency_dcdc(void);

/**
 * \brief Set 1V2 clamp voltage
 *
 * \param[in] voltage The voltage to set the rail to
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters:
 *  voltage[mV]: 1037, 1005, 978, 946, 1120, 1089, 1058, 1030,
 *               952, 918, 889, 861, 862, 828, 798, 770
 */
HW_PMU_ERROR_CODE hw_pmu_set_vdd_clamp(HW_PMU_VDD_CLAMP_VOLTAGE voltage);

/**
 * \brief Set 1V2 rail wakeup configuration
 *
 * This function sets the 1V2 rail configuration for the wakeup state of the system. This is
 * effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_CORE (Disable ldo_core_ret)
 * - Low current - Enable LDO_CORE_RET (Disable ldo_core)
 * The LDO source cannot be enabled if the power rail that supplies the LDO is off or it
 * cannot provide enough current. In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  HW_PMU_1V2_MAX_LOAD_1
 *  HW_PMU_1V2_MAX_LOAD_50
 *
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD max_load);

/**
 * \brief Disable 1V2 rail in wakeup state
 *
 * This function disables all 1V2 LDO power sources used for wakeup and active state.
 * LDO sources cannot be disabled if hardware blocks are powered by 1V2 rail. In this case the
 * appropriate error code will be returned:
 *  - HW_PMU_ERROR_PLL96M_ON: PLL96M is enabled and used as system clock
 *  - HW_PMU_ERROR_USB_PHY_ON: USB_PHY is enabled
 *  - HW_PMU_ERROR_XTAL32K_ON: XTAL32K is enabled
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_disable(void);

/**
 * \brief Set 1V2 rail sleep configuration
 *
 * This function sets the 1V2 rail configuration for the sleep state of the system.
 * Ldo_core_ret is the only available power source in sleep state.
 * The ldo_core_ret source cannot be enabled if the power rail that supplies the LDO is off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * \param[in] voltage The voltage to set the rail to. This voltage is applied to 1V2 rail sleep
 *            state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters:
 *  HW_PMU_1V2_SLEEP_VOLTAGE_0V75
 *  HW_PMU_1V2_SLEEP_VOLTAGE_0V8
 *  HW_PMU_1V2_SLEEP_VOLTAGE_0V85
 *  HW_PMU_1V2_SLEEP_VOLTAGE_0V9
 *  HW_PMU_1V2_SLEEP_VOLTAGE_0V95
 *  HW_PMU_1V2_SLEEP_VOLTAGE_1V0
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_SLEEP_VOLTAGE voltage);

/**
 * \brief Disable 1V2 rail in sleep state
 *
 * This function disables 1V2 Ldo_core_ret that is used for sleep state.
 * LDO source cannot be disabled if hardware blocks are powered by 1V2 rail. In this case the
 * appropriate error code will be returned:
 *  - HW_PMU_ERROR_XTAL32K_ON: XTAL32K is enabled
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *                 error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_disable(void);

/**
 * \brief Get 3V0 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 3V0 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onwakeup_config(HW_PMU_3V0_RAIL_CONFIG *rail_config);

/**
 * \brief Get 3V0 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 3V0 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onsleep_config(HW_PMU_3V0_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8P rail active configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_active_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8P rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onwakeup_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V8P rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onsleep_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V4 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V4 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_active_config(HW_PMU_1V4_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V4 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V4 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onwakeup_config(HW_PMU_1V4_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V2 rail active configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V2 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_active_config(HW_PMU_1V2_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V2 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the 1V2 rail has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onwakeup_config(HW_PMU_1V2_RAIL_CONFIG *rail_config);

/**
 * \brief Get 1V2 rail sleep configuration
 *
 * \param[out] rail_voltage The rail voltage
 *
 * \return POWER_RAIL_ENABLED if the rail 1V2 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onsleep_config(HW_PMU_1V2_SLEEP_VOLTAGE *rail_voltage);

#endif /* dg_configUSE_HW_PMU */


#endif /* HW_PMU_DA1469x_H_ */

/**
\}
\}
*/
