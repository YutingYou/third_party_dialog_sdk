/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_HAPTIC Haptic (LRA/ERM) Driver
 * \{
 * @brief Low Level Software Driver for the Haptic (LRA/ERM) Driver
 *
 * This Low Level Software Driver provides a low level abstraction layer for the
 * Haptic Driver and Controller hardware module (or simply Haptic Driver) used for
 * driving haptic actuators (LRA/ERM).
 *
 * @note Only applicable to DA14697/9 devices of the DA1469x chip family.
 */

/**
 ****************************************************************************************
 *
 * @file hw_haptic.h
 *
 * @brief Definition of API for the Haptic Low Level SW Driver.
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_HAPTIC_H_
#define HW_HAPTIC_H_

#if ((DEVICE_VARIANT == DA14697) || (DEVICE_VARIANT == DA14699))

#if dg_configUSE_HW_ERM || dg_configUSE_HW_LRA

#include "sdk_defs.h"


/**************** MACRO DEFINITIONS****************/


/********* Macros *********/

/**
 * @brief Maximum supported duty cycle
 */
#define HW_HAPTIC_DREF_MAX 0x7FFF

/**
 * @brief Minimum supported duty cycle
 */
#define HW_HAPTIC_DREF_MIN 0x0000

/**
 * @brief Sense resistor (in Ohms)
 */
#define HW_HAPTIC_RS  1

/**
 * @brief H-bridge nominal impedance (in Ohms)
 */
#define HW_HAPTIC_RHBRIDGE  3

/**
 * @brief Number of fractional bits in the chosen unsigned Q fixed-point format
 *
 * (for representing the drive level)
 */
#define HW_HAPTIC_UFIX16_Q    15

/**
 * @brief 1.0 in the chosen Q fixed-point format
 */
#define HW_HAPTIC_UFIX16_ONE  (1 << HW_HAPTIC_UFIX16_Q)

/**
 * @brief 0.5 in the chosen Q fixed-point format
 */
#define HW_HAPTIC_UFIX16_RND  (1 << (HW_HAPTIC_UFIX16_Q - 1))

/**
 * @brief Maximum half period supported by the Haptic Driver
 */
#define HW_HAPTIC_HALF_PERIOD_MAX 0xFF81

/**
 * @brief Minimum half period supported by the Haptic Driver
 */
#define HW_HAPTIC_HALF_PERIOD_MIN 0x0080


/********* Function-like macros *********/

/**
 * @brief Convert actuator voltage to duty cycle
 *
 * Calculates the duty cycle of the drive signal that is equivalent to a specific voltage across
 * the actuator.
 *
 * @param [in] V:     voltage across the actuator to convert (in mV - RMS)
 * @param [in] V_d:   drive voltage amplitude (in mV)
 * @param [in] R_act: actuator impedance (in Ohms)
 *
 * @return duty cycle of the drive signal in unsigned Q1.15 fixed-point format (0-32768
 *         corresponding to 0-100%)
 *
 * @note If the specified voltage across the actuator is too big, the returned duty cycle is
 *       limited to its max allowed value (HW_HAPTIC_DREF_MAX)! This apparently means that there is
 *       a maximum voltage across the actuator that can be applied by the Haptic Driver (for a
 *       given actuator and depending on the battery level).
 */
#define HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(V, V_d, R_act) \
        ((__HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(V, V_d, R_act) < HW_HAPTIC_DREF_MAX) ? __HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(V, V_d, R_act) : HW_HAPTIC_DREF_MAX)

/**
 * @brief Convert frequency to half period
 *
 * @param [in] f: drive frequency (in Hz)
 *
 * @return half period of the square wave drive voltage (in units of 4us)
 */
#define HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(f) ((uint16_t) (125000.0f / (f) + 0.5f))


/********* Internal function-like macros *********/

/**
 * @brief Convert actuator voltage to duty cycle
 *
 * Same as HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE() but without limiting to HW_HAPTIC_DREF_MAX.
 *
 * @note This function-like macro is internal (used by HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE())
 *       and is normally not supposed to be used externally!
 *
 * @sa HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE()
 */
#define __HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(V, V_d, R_act) \
                (((uint64_t) ((R_act) + HW_HAPTIC_RS + HW_HAPTIC_RHBRIDGE) * (V) * HW_HAPTIC_UFIX16_ONE) / (R_act) / (V_d))


/**************** DATA TYPE DEFINITIONS ****************/


/********* Enumerations *********/

/**
 * @brief Haptic Driver state
 *
 * This defines the different drive states of the Haptic Driver (i.e. Active/Idle).
 *
 * @sa hw_haptic_get_state(), hw_haptic_set_state()
 */
typedef enum {
        HW_HAPTIC_STATE_IDLE = 0,   /**< Idle drive state. The drive signal is disabled. The Haptic
                                         Driver preserves its configuration state, but no power
                                         is output to the actuator and no interrupt requests are
                                         generated. */
        HW_HAPTIC_STATE_ACTIVE = 1, /**< Active drive state. The drive signal is enabled. */
} HW_HAPTIC_STATE;

/**
 * @brief Drive polarity
 *
 * This defines the different polarity states of the drive signal.
 *
 * @sa hw_haptic_get_polarity(), hw_haptic_set_polarity()
 */
typedef enum {
        HW_HAPTIC_POLARITY_NORMAL = 0,   /**< Normal polarity */
        HW_HAPTIC_POLARITY_INVERTED = 1, /**< Inverted polarity */
} HW_HAPTIC_POLARITY;

/**
 * @brief Drive level reference type
 *
 * This defines the different reference options for the drive level (i.e. nominal and absolute
 * maximum duty cycle of the actuator in use).
 *
 * @sa hw_haptic_get_drive_level(), hw_haptic_set_drive_level()
 */
typedef enum {
        HW_HAPTIC_DRIVE_LEVEL_REF_NOM_MAX = 0, /**< Actuator's maximum nominal duty cycle  */
        HW_HAPTIC_DRIVE_LEVEL_REF_ABS_MAX = 1, /**< Actuator's maximum absolute duty cycle */
} HW_HAPTIC_DRIVE_LEVEL_REF;

#if dg_configUSE_HW_ERM
/**
 * @brief Haptic Driver output
 *
 * This defines the different polarity options for the ERM drive.
 *
 * @note ERM only
 */
typedef enum {
        HW_HAPTIC_ERM_OUTPUT_HDRVM = 0,        /**< Negative Haptic Driver output (HDRVM output pin) */
        HW_HAPTIC_ERM_OUTPUT_HDRVP = 1,        /**< Positive Haptic Driver output (HDRVP output pin) */
} HW_HAPTIC_ERM_OUTPUT;

#endif /* dg_configUSE_HW_ERM */

/********* Functions *********/

/**
 * @brief Callback function to be fired on every interrupt
 *
 * This defines the type of the function to be called on interrupt (every half cycle, upon
 * capturing the 7th sample) by the Haptic Interrupt Handler.
 *
 * It aims to serve as a hook for performing open or closed-loop haptic processing in order to
 * dynamically update the drive parameters in every half cycle. This mechanism can be used for
 * producing haptic waveform patterns and for ensuring maximum drive efficiency (in terms of power
 * consumption, vibration intensity, start/stop times etc.).
 *
 * The Handler provides the callback function with pointers to the current drive parameters
 * together with the latest eight samples of the actuator's electrical current (i-samples). The
 * callback function can therefore update the drive parametes potentially by taking into account
 * the actuator's real time mechanical response (as obtained by processing/analyzing the
 * i-samples).
 *
 * @param [in] i_data:      i-samples of the current half cycle
 * @param [in] half_period: pointer to the current half_period of the drive signal
 * @param [in] drive_level: pointer to the current drive_level of the drive signal
 * @param [in] hw_state:    pointer to the current hardware state
 *
 * The hardware state is a 16-bit entity comprised of the following fields:
 *
 * Bit   Description
 * 0     instant polarity of the drive signal (first or second half of the current drive cycle)
 * 1     state of the automatic (instant) polarity switching (normal or inverted)
 * 2     validity of the latest eight i-samples (Currently, constantly set to 1 -
 *       reserved for future use)
 * 3-15  reserved
 *
 * @sa haptic_config_t
 */
typedef void (*hw_haptic_interrupt_cb_t)(int16_t *i_data, uint16_t *half_period, uint16_t *drive_level, uint16_t *hw_state);


/********* Structures *********/

/**
 * @brief Haptic Low Level SW Driver configuration
 *
 * This defines the configuration parameter structure for initializing the Haptic Driver.
 *
 * @sa hw_haptic_init()
 */
typedef struct {
        uint16_t duty_cycle_nom_max;            /**< maximum nominal duty cycle of the drive
                                                     signal that is supported by the actuator
                                                     (steady state operation) (in unsigned
                                                     Q1.15 fixed-point format (0-32768
                                                     corresponding to 0-100%)). It should be lower
                                                     or equal to HW_HAPTIC_DREF_MAX and also lower
                                                     or equal to duty_cycle_abs_max. */
        uint16_t duty_cycle_abs_max;            /**< maximum absolute duty cycle of the drive
                                                     signal that is supported by the actuator
                                                     (transient while accelerating/braking)
                                                     (in unsigned Q1.15 fixed-point format
                                                     (0-32768 corresponding to 0-100%)). It should
                                                     be lower or equal to HW_HAPTIC_DREF_MAX. */
        uint16_t half_period;                   /**< half period of the drive signal in case of an
                                                     LRA (should be set according to its
                                                     nominal/initial resonant frequency) - or just
                                                     half period of the interrupt requests in the
                                                     ERM case (optional). If set, a value between
                                                     250 and 5000 (corresponding to a frequency
                                                     range of 25-500Hz) is recommended (allowing
                                                     enough time for the System to service the
                                                     Haptic interrupt requests). */
        hw_haptic_interrupt_cb_t interrupt_cb;  /**< callback function to be fired on interrupt */
#if dg_configUSE_HW_ERM
        HW_HAPTIC_ERM_OUTPUT signal_out;        /**< haptic driver output polarity @note ERM only */
#endif /* dg_configUSE_HW_ERM */
} haptic_config_t;


/****************** FUNCTION DECLARATIONS/DEFINITONS ******************/


/**
 * @brief Initialize Haptic Driver
 *
 * This function applies the necessary initial configuration settings to the Haptic
 * Driver for driving the specific actuator in use.
 *
 * @param [in] cfg: pointer to the actuator configuration parameter structure
 *
 * @note \b cfg should not be NULL.
 */
void hw_haptic_init(const haptic_config_t *cfg);

/**
 * @brief Get Polarity
 *
 * This function reads the polarity of the drive signal.
 *
 * @return polarity (0: normal, 1: inverted)
 */
__STATIC_INLINE HW_HAPTIC_POLARITY hw_haptic_get_polarity(void)
{
#if dg_configUSE_HW_LRA
        return REG_GETF(LRA, LRA_CTRL2_REG, POLARITY);
#else
        return REG_GETF(LRA, LRA_DFT_REG, SWM_MAN);
#endif
}

/**
 * @brief Set Polarity
 *
 * This function sets the polarity of the drive signal.
 *
 * @param [in] polarity: polarity (0: normal, 1: inverted)
 */
__STATIC_INLINE void hw_haptic_set_polarity(HW_HAPTIC_POLARITY polarity)
{
#if dg_configUSE_HW_LRA
        REG_SETF(LRA, LRA_CTRL2_REG, POLARITY, polarity);
#else
        REG_SETF(LRA, LRA_DFT_REG, SWM_MAN, polarity);
#endif
}

/**
 * @brief Get Half Period
 *
 * This function reads the half period of the drive signal.
 *
 * @return half period (in units of 4us)
 *
 * @sa hw_haptic_set_half_period()
 */
__STATIC_INLINE uint16_t hw_haptic_get_half_period(void)
{
        return (uint16_t) REG_GETF(LRA, LRA_CTRL2_REG, HALF_PERIOD);
}

/**
 * @brief Set Half Period
 *
 * This function sets the half period of the drive signal.
 *
 * @param [in] half_period: half period (in units of 4us)
 *
 * @note The actually applied half period is limited by the supported range of the Haptic Driver.
 *
 * @sa hw_haptic_get_half_period()
 */
__STATIC_INLINE void hw_haptic_set_half_period(uint16_t half_period)
{

        if (half_period < HW_HAPTIC_HALF_PERIOD_MIN) {
                half_period = HW_HAPTIC_HALF_PERIOD_MIN;
        } else if (half_period > HW_HAPTIC_HALF_PERIOD_MAX) {
                half_period = HW_HAPTIC_HALF_PERIOD_MAX;
        }
        REG_SETF(LRA, LRA_CTRL2_REG, HALF_PERIOD, half_period);
}

/**
 * @brief Get Drive level
 *
 * This function reads the duty cycle of the drive signal and normalizes it according to the
 * specified drive level reference (i.e. the nominal or absolute maximum duty cycle supported by
 * the specific actuator in use).
 *
 * @param [in] ref: drive level reference type
 *
 * @return drive level (in unsigned Q1.15 fixed-point format (0-32768 corresponding to 0-100%))
 *
 * @note The returned drive level is limited by 100%.
 *
 * @sa hw_haptic_set_drive_level()
 */
uint16_t hw_haptic_get_drive_level(HW_HAPTIC_DRIVE_LEVEL_REF ref);

/**
* @brief Set Drive level
*
* This function sets the duty cycle of the drive signal according to the specified drive level and
* the specified drive level reference (i.e. the drive level is expressed as a percentage of either
* the nominal or the absolute maximum duty cycle limit of the specific actuator in use).
*
* @param [in] drive_level: drive level (in unsigned Q1.15 fixed-point format (0-32768 corresponding
*                          to 0-100%))
* @param [in] ref:         drive level reference type
*
* @note The actually applied drive level is limited by the supported range of both the actuator and
*       the Haptic Driver.
*
* @sa hw_haptic_get_drive_level()
*/
void hw_haptic_set_drive_level(uint16_t drive_level, HW_HAPTIC_DRIVE_LEVEL_REF ref);

/**
 * @brief Set Haptic Driver state
 *
 * This function sets the drive state (i.e. Active/Idle) of the Haptic Driver.
 *
 * @param [in] state: drive state (Active/Idle)
 *
 * @sa hw_haptic_get_state()
 */
__STATIC_INLINE void hw_haptic_set_state(HW_HAPTIC_STATE state)
{
        REG_SETF(LRA, LRA_CTRL1_REG, LRA_EN, state);
}

/**
 * @brief Get Haptic Driver state
 *
 * This function reads the drive state (i.e. Active/Idle) of the Haptic Driver.
 *
 * @return drive state (Active/Idle)
 *
 * @sa hw_haptic_set_state()
 */
__STATIC_INLINE HW_HAPTIC_STATE hw_haptic_get_state(void)
{
        return REG_GETF(LRA, LRA_CTRL1_REG, LRA_EN);
}

/**
 * @brief Shut down Haptic Driver
 *
 * This function deactivates the Haptic Driver completely.
 *
 * @note For re-activating, hw_haptic_init() should be used.
 *
 * @sa hw_haptic_init()
 */
void hw_haptic_shutdown(void);

#endif /* dg_configUSE_HW_ERM || dg_configUSE_HW_LRA */

#endif /* DEVICE_VARIANT */

#endif /* HW_HAPTIC_H_ */

/**
 * \}
 * \}
 * \}
 */
