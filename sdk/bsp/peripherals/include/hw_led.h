/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_LED LED Driver
 * \{
 * \brief LED Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_led.h
 *
 * @brief Definition of API for the LED Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_LED_H_
#define HW_LED_H_


#include <sdk_defs.h>


/**
 * \brief Enable or disable LED1
 *
 * \param [in] state true to enable LED1, or false to disable it
 *
 */
__STATIC_INLINE void hw_led_enable_led1(bool state)
{
        REG_SETF(PWMLED, PWMLED_CTRL_REG, LED1_EN, state);
}

/**
 * \brief Enable or disable LED2
 *
 * \param [in] state true to enable LED2, or false to disable it
 *
 */
__STATIC_INLINE void hw_led_enable_led2(bool state)
{
        REG_SETF(PWMLED, PWMLED_CTRL_REG, LED2_EN, state);
}

/**
 * \brief Led's PWM duty cycle configuration.
 *
 */
typedef struct {
        int hw_led_pwm_start;
        int hw_led_pwm_end;
} hw_led_pwm_duty_cycle_t;


/**
 * \brief Set the PWM duty cycle for led1.
 *
 * This function sets the PWM duty cycle configuration for led1.
 * \see hw_led_pwm_set_frequency()
 *
 * \param [in] duty_cycle Duty cycle configuration.
 *
 * \warning If the start of duty cycle is larger than PWM's frequency or the end of duty cycle is
 *          equal to start then PWM's output remains low.
 * \warning If the end of duty cycle is larger than PWM's frequency and the start of duty cycle is
 *          not larger than PWM's frequency then PWM's output remains high.
 *
 */
__STATIC_INLINE void hw_led1_pwm_set_duty_cycle(const hw_led_pwm_duty_cycle_t* duty_cycle)
{
         REG_SETF(PWMLED, PWMLED_DUTY_CYCLE_LED1_REG, LED1_PWM_START_CYCLE, duty_cycle->hw_led_pwm_start);
         REG_SETF(PWMLED, PWMLED_DUTY_CYCLE_LED1_REG, LED1_PWM_END_CYCLE, duty_cycle->hw_led_pwm_end);
}

/**
 * \brief Get the PWM duty cycle for led1.
 *
 * This function gets the PWM duty cycle configuration for led1.
 *
 * \param [out] duty_cycle Duty cycle configuration.
 *
 */
__STATIC_INLINE void hw_led1_pwm_get_duty_cycle(hw_led_pwm_duty_cycle_t* duty_cycle)
{
        duty_cycle->hw_led_pwm_start = REG_GETF(PWMLED, PWMLED_DUTY_CYCLE_LED1_REG, LED1_PWM_START_CYCLE);
        duty_cycle->hw_led_pwm_end = REG_GETF(PWMLED, PWMLED_DUTY_CYCLE_LED1_REG, LED1_PWM_END_CYCLE);
}

/**
 * \brief Set the PWM duty cycle for led2.
 *
 * This function sets the PWM duty cycle configuration for led2.
 * \see hw_led_pwm_set_frequency()
 *
 * \param [in] duty_cycle Duty cycle configuration.
 *
 * \warning If the start of duty cycle is larger than PWM's frequency or the end of duty cycle is
 *          equal to start then PWM's output remains low.
 * \warning If the end of duty cycle is larger than PWM's frequency and the start of duty cycle is
 *          not larger than PWM's frequency then PWM's output remains high.
 *
 */
__STATIC_INLINE void hw_led2_pwm_set_duty_cycle(const hw_led_pwm_duty_cycle_t* duty_cycle)
{
        REG_SETF(PWMLED, PWMLED_DUTY_CYCLE_LED2_REG, LED2_PWM_START_CYCLE, duty_cycle->hw_led_pwm_start);
        REG_SETF(PWMLED, PWMLED_DUTY_CYCLE_LED2_REG, LED2_PWM_END_CYCLE, duty_cycle->hw_led_pwm_end);
}

/**
 * \brief Get the PWM duty cycle for led2.
 *
 * This function gets the PWM duty cycle configuration for led2.
 *
 * \param [out] duty_cycle Duty cycle configuration.
 *
 */
__STATIC_INLINE void hw_led2_pwm_get_duty_cycle(hw_led_pwm_duty_cycle_t* duty_cycle)
{
        duty_cycle->hw_led_pwm_start = REG_GETF(PWMLED, PWMLED_DUTY_CYCLE_LED2_REG, LED2_PWM_START_CYCLE);
        duty_cycle->hw_led_pwm_end = REG_GETF(PWMLED, PWMLED_DUTY_CYCLE_LED2_REG, LED2_PWM_END_CYCLE);
}

/**
 * \brief Set led's PWM frequency.
 *
 * This function sets the led's PWM frequency. Low power clock is the clock source.
 * Example with pwm_freq 16.
 *
 * \verbatim
 * tick=  0          pwm_freq-1
 *        |<-- period -->|
 *    #1  HHHH............
 *    #2  ......HHHH......
 *    #3  ............HHHH
 *    #4  HH............HH
 *
 *    #1 start= 0, end= 4
 *    #2 start= 6, end=10
 *    #3 start=12, end=16 (or end = 0, both have the same result)
 *    #4 start=14, end= 2
 * \endverbatim
 *
 * \param [in]  pwm_freq PWM's frequency
 *
 */
__STATIC_INLINE void hw_led_pwm_set_frequency(int pwm_freq)
{
        REG_SETF(PWMLED, PWMLED_FREQUENCY_REG, LED_PWM_FREQUENCY, pwm_freq);
}

/**
 * \brief Get led's PWM frequency.
 *
 * This function gets PWM's frequency
 *
 * \return PWM's frequency
 *
 */
__STATIC_INLINE int hw_led_pwm_get_frequency(void)
{
        return REG_GETF(PWMLED, PWMLED_FREQUENCY_REG, LED_PWM_FREQUENCY);
}

/**
 * \brief Get led1's state.
 *
 * This function gets led1's state
 *
 * \return 1: Led1 is enabled.
 * \return 0: Led1 is disabled.
 *
 */
__STATIC_INLINE bool hw_led_get_led1_state(void)
{
       return  REG_GETF(PWMLED, PWMLED_CTRL_REG, LED1_EN);
}

/**
 * \brief Get led2's state.
 *
 * This function gets led1's state
 *
 * \return 1: Led2 is enabled.
 * \return 0: Led2 is disabled.
 *
 */
__STATIC_INLINE bool hw_led_get_led2_state(void)
{
       return  REG_GETF(PWMLED, PWMLED_CTRL_REG, LED2_EN);
}

/**
 * \brief Get the PWM's trim value.
 *
 * This function gets PWM's trim value.
 *
 * \return trim value.
 *
 */
__STATIC_INLINE int hw_led_get_pwm_trim_bits(void)
{
        return REG_GETF(PWMLED, PWMLED_CTRL_REG, LED_TRIM);
}

/**
 * \brief Set the PWM's SW blocking state.
 *
 * This function sets PWM's SW blocking state.
 *
 * \param [in] state This is PWM's SW blocking state. If set, software blocks/pauses PWM.
 *
 */
__STATIC_INLINE void hw_led_set_pwm_sw_pause(bool state)
{
        REG_SETF(PWMLED, PWMLED_CTRL_REG, SW_PAUSE_EN, state);
}

/**
 * \brief Get the PWM's SW blocking state.
 *
 * This function gets PWM's SW blocking state.
 *
 * \return 1: PWM is blocked by SW.
 * \return 0: PWM is not block by HW.
 *
 */
__STATIC_INLINE bool hw_led_get_pwm_sw_pause(void)
{
        return REG_GETF(PWMLED, PWMLED_CTRL_REG, SW_PAUSE_EN);
}

/**
 * \brief Set PWM's state.
 *
 * This function sets PWM's states.
 *
 * \param [in] state True to enable PWM, false to disable PWM.
 *
 */
__STATIC_INLINE void hw_led_set_pwm_state(bool state)
{
        REG_SETF(PWMLED, PWMLED_CTRL_REG, PWM_ENABLE, state);
}

/**
 * \brief Get PWM's state.
 *
 * This function sets PWM's states.
 *
 * \return 1: PWM is enabled.
 * \return 0: PWM is disabled.
 *
 */
__STATIC_INLINE bool hw_led_get_pwm_state(void)
{
        return REG_GETF(PWMLED, PWMLED_CTRL_REG, PWM_ENABLE);
}



#endif /* HW_LED_H_ */

/**
 * \}
 * \}
 */
