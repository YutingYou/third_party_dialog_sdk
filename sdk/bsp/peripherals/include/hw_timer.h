/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_TIMER Timer 1/2/3/4 Driver
 * \{
 * \brief Timer
 */

/**
 *****************************************************************************************
 *
 * @file hw_timer.h
 *
 * @brief Definition of API for the Timer, Timer2, Timer3 and Timer4 Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_TIMER_H_
#define HW_TIMER_H_


#if dg_configUSE_HW_TIMER

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>
#include <hw_gpio.h>


/* Timer Base Address */
#define TBA(id)                         ((TIMER_Type *)id)


/**
 * \brief Get the value of a field of a TIMER register.
 *
 * \param [in] id identifies TIMER, TIMER2, TIMER3 or TIMER4
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_TIMER_REG_GETF(id, reg, field) \
        ((TBA(id)->reg & (TIMER_##reg##_##field##_Msk)) >> (TIMER_##reg##_##field##_Pos))

/**
 * \brief Set the value of a field of a TIMER register.
 *
 * \param [in] id identifies TIMER, TIMER2, TIMER3 or TIMER4
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_TIMER_REG_SETF(id, reg, field, val) \
        TBA(id)->reg = ((TBA(id)->reg & ~(TIMER_##reg##_##field##_Msk)) | \
        ((TIMER_##reg##_##field##_Msk) & ((val) << (TIMER_##reg##_##field##_Pos))))

/**
 * \brief Maximum value for timer pre-scaler (5bits).
 *
 */
#define TIMER_MAX_PRESCALER_VAL   TIMER_TIMER_PRESCALER_REG_TIM_PRESCALER_Msk

/**
 * \brief Maximum value for timer reload value (24bits).
 *
 */
#define TIMER_MAX_RELOAD_VAL   TIMER_TIMER_RELOAD_REG_TIM_RELOAD_Msk

/**
 * \brief Maximum value for timer shot phase duration value in oneshot mode (24bits).
 *
 */
#define TIMER_MAX_SHOTWIDTH_VAL   TIMER_TIMER_SHOTWIDTH_REG_TIM_SHOTWIDTH_Msk

/**
 * \brief Maximum value for timer PWM Frequency (16bits).
 *
 */
#define TIMER_MAX_PWM_FREQ_VAL   TIMER_TIMER_PWM_FREQ_REG_TIM_PWM_FREQ_Msk

/**
 * \brief Maximum value for timer PWM duty cycle (16bits).
 *
 */
#define TIMER_MAX_PWM_DC_VAL   TIMER_TIMER_PWM_DC_REG_TIM_PWM_DC_Msk

/**
 * \brief Timer id
 *
 */
#define HW_TIMER          ((void *)TIMER_BASE)
#define HW_TIMER2         ((void *)TIMER2_BASE)
#define HW_TIMER3         ((void *)TIMER3_BASE)
#define HW_TIMER4         ((void *)TIMER4_BASE)
typedef void * HW_TIMER_ID;


/**
 * \brief Mode of operation
 *
 * \note PWM is enabled in both modes.
 *
 */
typedef enum {
        HW_TIMER_MODE_TIMER = 0,       /**< timer/capture mode. Supported by all timers */
        HW_TIMER_MODE_ONESHOT = 1      /**< one-shot mode. Supported only by HW_TIMER and HW_TIMER2 */
} HW_TIMER_MODE;

/**
 * \brief Clock source for timer
 *
 */
typedef enum {
        HW_TIMER_CLK_SRC_INT = 0,      /**< Timer uses the low power clock */
        HW_TIMER_CLK_SRC_EXT = 1       /**< Timer uses the DIVN */
} HW_TIMER_CLK_SRC;

/**
 * \brief Counting direction
 *
 */
typedef enum {
        HW_TIMER_DIR_UP = 0,           /**< Timer counts up (counter is incremented) */
        HW_TIMER_DIR_DOWN = 1          /**< Timer counts down (counter is decremented) */
} HW_TIMER_DIR;

/**
 * \brief Type of triggering events
 *
 */
typedef enum {
        HW_TIMER_TRIGGER_RISING = 0,   /**< Event activated rising edge */
        HW_TIMER_TRIGGER_FALLING = 1   /**< Event activated falling edge */
} HW_TIMER_TRIGGER;

/**
 * \brief One shot mode phases
 *
 */
typedef enum {
        HW_TIMER_ONESHOT_WAIT = 0,     /**< Wait for the event */
        HW_TIMER_ONESHOT_DELAY = 1,    /**< Delay before started */
        HW_TIMER_ONESHOT_STARTED = 2,  /**< Start shot */
        HW_TIMER_ONESHOT_ACTIVE = 3    /**< Shot is active */
} HW_TIMER_ONESHOT;

/**
 * \brief GPIOs for timer trigger
 *
 * In HW_TIMER_MODE_TIMER mode use to select which GPIO will trigger capture time.
 * In HW_TIMER_MODE_ONESHOT mode use to select which GPIO will trigger the programmable pulse.
 * HW_TIMER and HW_TIMER3 are mapped to GPIO Port 0 where HW_TIMER2 and HW_TIMER4 are mapped to GPIO Port 1.
 *
 */

typedef enum {
        HW_TIMER_GPIO_GPIO_NONE = 0,      /**< No GPIO */
        HW_TIMER_GPIO_PIN_0 = 1,      /**< GPIO Pin 0 */
        HW_TIMER_GPIO_PIN_1 = 2,      /**< GPIO Pin 1 */
        HW_TIMER_GPIO_PIN_2 = 3,      /**< GPIO Pin 2 */
        HW_TIMER_GPIO_PIN_3 = 4,      /**< GPIO Pin 3 */
        HW_TIMER_GPIO_PIN_4 = 5,      /**< GPIO Pin 4 */
        HW_TIMER_GPIO_PIN_5 = 6,      /**< GPIO Pin 5 */
        HW_TIMER_GPIO_PIN_6 = 7,      /**< GPIO Pin 6 */
        HW_TIMER_GPIO_PIN_7 = 8,      /**< GPIO Pin 7 */
        HW_TIMER_GPIO_PIN_8 = 9,      /**< GPIO Pin 8 */
        HW_TIMER_GPIO_PIN_9 = 10,     /**< GPIO Pin 9 */
        HW_TIMER_GPIO_PIN_10 = 11,    /**< GPIO Pin 10 */
        HW_TIMER_GPIO_PIN_11 = 12,    /**< GPIO Pin 11 */
        HW_TIMER_GPIO_PIN_12 = 13,    /**< GPIO Pin 12 */
        HW_TIMER_GPIO_PIN_13 = 14,    /**< GPIO Pin 13 */
        HW_TIMER_GPIO_PIN_14 = 15,    /**< GPIO Pin 14 */
        HW_TIMER_GPIO_PIN_15 = 16,    /**< GPIO Pin 15 */
        HW_TIMER_GPIO_PIN_16 = 17,    /**< GPIO Pin 16 */
        HW_TIMER_GPIO_PIN_17 = 18,    /**< GPIO Pin 17 */
        HW_TIMER_GPIO_PIN_18 = 19,    /**< GPIO Pin 18 */
        HW_TIMER_GPIO_PIN_19 = 20,    /**< GPIO Pin 19 */
        HW_TIMER_GPIO_PIN_20 = 21,    /**< GPIO Pin 20 */
        HW_TIMER_GPIO_PIN_21 = 22,    /**< GPIO Pin 21 */
        HW_TIMER_GPIO_PIN_22 = 23,    /**< GPIO Pin 22 */
        HW_TIMER_GPIO_PIN_23 = 24,    /**< GPIO Pin 23 */
        HW_TIMER_GPIO_PIN_24 = 25,    /**< GPIO Pin 24 */
        HW_TIMER_GPIO_PIN_25 = 26,    /**< GPIO Pin 25 */
        HW_TIMER_GPIO_PIN_26 = 27,    /**< GPIO Pin 26 */
        HW_TIMER_GPIO_PIN_27 = 28,    /**< GPIO Pin 27 */
        HW_TIMER_GPIO_PIN_28 = 29,    /**< GPIO Pin 28 */
        HW_TIMER_GPIO_PIN_29 = 30,    /**< GPIO Pin 29 */
        HW_TIMER_GPIO_PIN_30 = 31,    /**< GPIO Pin 30 */
        HW_TIMER_GPIO_PIN_31 = 32,    /**< GPIO Pin 31 */
} HW_TIMER_GPIO;

/**
 * \brief Timer interrupt callback
 *
 */
typedef void (*hw_timer_handler_cb)(void);

/**
 * \brief Timer capture interrupt callback
 *
 * \param [in] event bitmask of capture time event GPIOs. "1" means capture event occurred on GPIO
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *                   | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 */
typedef void (*hw_timer_capture_handler_cb)(uint8_t gpio_event);

/**
 * \brief Timer configuration for timer/capture mode
 *
 * \sa timer_config
 * \sa hw_timer_configure_timer
 *
 */
typedef struct {
        HW_TIMER_DIR        direction;    /**< counting direction */
        uint32_t            reload_val;   /**< reload value */
        bool                free_run;     /**< free-running mode state */

        HW_TIMER_GPIO       gpio1;        /**< 1st GPIO for capture mode */
        HW_TIMER_TRIGGER    trigger1;     /**< 1st GPIO capture trigger */
        HW_TIMER_GPIO       gpio2;        /**< 2nd GPIO for capture mode */
        HW_TIMER_TRIGGER    trigger2;     /**< 2nd GPIO capture trigger */
        HW_TIMER_GPIO       gpio3;        /**< 3rd GPIO for capture mode. Only valid for Timer */
        HW_TIMER_TRIGGER    trigger3;     /**< 3rd GPIO capture trigger. Only valid for Timer  */
        HW_TIMER_GPIO       gpio4;        /**< 4th GPIO for capture mode. Only valid for Timer */
        HW_TIMER_TRIGGER    trigger4;     /**< 4th GPIO capture trigger. Only valid for Timer  */
} timer_config_timer_capture;

/**
 * \brief Timer configuration for oneshot mode
 *
 * \sa timer_config
 * \sa hw_timer_configure_oneshot
 *
 */
typedef struct {
        uint16_t            delay;        /**< delay (ticks) between GPIO event and output pulse */
        uint32_t            shot_width;   /**< width (ticks) of generated pulse */
        HW_TIMER_GPIO       gpio;         /**< GPIO to wait for event */
        HW_TIMER_TRIGGER    trigger;      /**< GPIO trigger */
} timer_config_oneshot;

/**
 * \brief Timer PWM configuration
 *
 * \sa timer_config
 * \sa hw_timer_configure_pwm
 * \sa hw_timer_set_pwm_freq
 * \sa hw_timer_set_pwm_duty_cycle
 */
typedef struct {
        HW_GPIO_PIN     pin;          /**< Defines the pin of the GPIO with PWM function */
        HW_GPIO_PORT    port;         /**< Defines the port of the GPIO with PWM function */
        /**< When true, Timer or Timer2 will keep PWM output on P1_01 or P1_06, respectively, during deep sleep */
        bool            pwm_active_in_sleep;
        /**< Defines the PWM frequency. Timer clock frequency / (frequency + 1) */
        uint16_t        frequency;
        /**< Defines the PWM duty cycle. duty_cycle / ( frequency + 1) */
        uint16_t        duty_cycle;
} timer_config_pwm;

/**
 * \brief Timer configuration
 *
 * Only one of \p timer and \p oneshot configuration can be set since they are stored in the same
 * union (and will overwrite each other). Proper configuration structure is selected depending on
 * timer mode set.
 *
 * \sa timer_config_timer_capture
 * \sa timer_config_oneshot
 * \sa timer_config_pwm
 * \sa hw_timer_configure
 *
 */
typedef struct {
        HW_TIMER_CLK_SRC    clk_src;                    /**< clock source */
        uint8_t             prescaler;                  /**< clock prescaler */

        HW_TIMER_MODE       mode;                       /**< timer/capture mode or oneshot mode */
        union {
                timer_config_timer_capture    timer;    /**< configuration for timer/capture mode */
                timer_config_oneshot          oneshot;  /**< configuration for oneshot mode. Only valid for Timer and Timer2 */
        };
        timer_config_pwm   pwm;                         /**< PWM configuration */
} timer_config;

/**
 * \brief Timer initialization
 *
 * Turn on clock for timer and configure timer. After initialization both timer and its interrupt
 * are disabled. \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 */
void hw_timer_init(HW_TIMER_ID id, const timer_config *cfg);

/**
 * \brief Timer configuration
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing except switching timer to selected mode.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 */
void hw_timer_configure(HW_TIMER_ID id, const timer_config *cfg);

/**
 * \brief Timer configuration for timer/capture mode
 *
 * Shortcut to call appropriate configuration function. This does not switch timer to timer/capture
 * mode, it should be done separately using hw_timer_set_mode().
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 * \note This function will enable the timer clock before loading the registers
 *       and it will leave the timer clock enabled
 *
 * \sa hw_timer_enable_clk
 *
 */
void hw_timer_configure_timer(HW_TIMER_ID id, const timer_config_timer_capture *cfg);

/**
 * \brief Timer configuration for oneshot mode
 *
 * Shortcut to call appropriate configuration function. This does not switch timer to oneshot
 * mode, it should be done separately using hw_timer_set_mode().
 *
 * \param [in] id timer id. Valid values for one shot mode are HW_TIMER and HW_TIMER2
 * \param [in] cfg configuration
 *
 */
void hw_timer_configure_oneshot(HW_TIMER_ID id, const timer_config_oneshot *cfg);

/**
 * \brief Freeze timer
 *
 * \param [in] id timer id
 */
__STATIC_INLINE void hw_timer_freeze(HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM_Msk;
        } else if (id == HW_TIMER2) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk;
        } else if (id == HW_TIMER3) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM3_Msk;
        } else if (id == HW_TIMER4) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM4_Msk;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Unfreeze timer
 *
 * \param [in] id timer id
 */
__STATIC_INLINE void hw_timer_unfreeze(HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM_Msk;
        } else if (id == HW_TIMER2) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM2_Msk;
        } else if (id == HW_TIMER3) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM3_Msk;
        } else if (id == HW_TIMER4) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM4_Msk;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Check if timer is frozen
 *
 * \param [in] id timer id
 *
 * \return true if it is frozen else false
 *
 */
__STATIC_INLINE bool hw_timer_frozen(HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM_Msk);
        } else if (id == HW_TIMER2) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk);
        } else if (id == HW_TIMER3) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM3_Msk);
        } else if (id == HW_TIMER4) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM4_Msk);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Set clock source of the timer
 *
 * \param [in] id timer id
 * \param [in] clk clock source of the timer, external or internal
 *
 */
__STATIC_INLINE void hw_timer_set_clk(HW_TIMER_ID id, HW_TIMER_CLK_SRC clk)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_SYS_CLK_EN, clk);
}

/**
 * \brief Set timer clock prescaler
 *
 * Actual timer frequency is \p timer_freq = \p freq_clock / (\p value + 1)
 *
 * \param [in] id timer id
 * \param [in] value prescaler. 5 bits long, shall not be greater than 0x1f
 *
 */
__STATIC_INLINE void hw_timer_set_prescaler(HW_TIMER_ID id, uint8_t value)
{
        ASSERT_WARNING(TIMER_MAX_PRESCALER_VAL >= value);
        TBA(id)->TIMER_PRESCALER_REG = value;
}

/**
 * \brief Set timer reload value
 *
 * \note This changes the same register value as hw_timer_set_oneshot_delay() since both parameters
 * share the same register (value is interpreted differently depending on timer mode).
 *
 * \param [in] id timer id
 * \param [in] value reload value. 24 bits long, shall not be greater than 0xffffff
 *
 * \sa hw_timer_set_oneshot_delay
 *
 */
__STATIC_INLINE void hw_timer_set_reload(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_RELOAD_VAL >= value);
        TBA(id)->TIMER_RELOAD_REG = value;
}

/**
 * \brief Set pulse delay in oneshot mode
 *
 * \note This changes the same register value as hw_timer_set_reload() since both parameters share
 * the same register (value is interpreted differently depending on timer mode).
 *
 * \param [in] id timer id
 * \param [in] delay delay (ticks). 24 bits long, shall not be greater than 0xffffff
 *
 * \sa hw_timer_set_reload
 *
 */
__STATIC_INLINE void hw_timer_set_oneshot_delay(HW_TIMER_ID id, uint32_t delay)
{
        ASSERT_WARNING(TIMER_MAX_RELOAD_VAL >= delay);
        TBA(id)->TIMER_RELOAD_REG = delay;
}

/**
 * \brief Set shot width
 *
 * This applies only to one-shot mode.
 *
 * \param [in] id timer id
 * \param [in] duration shot phase duration. 24 bits long, shall not be greater than 0xffffff
 *
 */
__STATIC_INLINE void hw_timer_set_shot_width(HW_TIMER_ID id, uint32_t duration)
{
        ASSERT_WARNING(TIMER_MAX_SHOTWIDTH_VAL >= duration);
        TBA(id)->TIMER_SHOTWIDTH_REG = duration;
}

/**
 * \brief Turn on free run mode of the timer
 *
 * This mode is valid only when timer is counting up
 *
 * \param [in] id timer id
 * \param [in] enable if it is '1' timer does not zero when it reaches the reload value.
 */
__STATIC_INLINE void hw_timer_set_freerun(HW_TIMER_ID id, bool enable)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_FREE_RUN_MODE_EN, enable);
}

/**
 * \brief Set a type of the edge which triggers event1
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 */
__STATIC_INLINE void hw_timer_set_event1_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN1_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event2
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 */
__STATIC_INLINE void hw_timer_set_event2_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN2_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event3
 *
 * \param [in] edge type of edge, rising or falling
 *
 * \note Valid only for Timer
 *
 */
__STATIC_INLINE void hw_timer_set_event3_trigger(HW_TIMER_TRIGGER edge)
{
        REG_SETF(TIMER, TIMER_CTRL_REG, TIM_IN3_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event4
 *
 * \param [in] edge type of edge, rising or falling
 *
 * \note Valid only for Timer
 *
 */
__STATIC_INLINE void hw_timer_set_event4_trigger(HW_TIMER_TRIGGER edge)
{
        REG_SETF(TIMER, TIMER_CTRL_REG, TIM_IN4_EVENT_FALL_EN, edge);
}

/**
 * \brief Select which time capture event GPIOs will create a capture IRQ
 *
 * \param [in] mask bitmask of capture time event GPIOs. Set "1" to enable capture interrupt on GPIOx event
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         IRQ_EN    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Valid only for Timer
 *
 */
__STATIC_INLINE void hw_timer_set_gpio_event_int(uint8_t mask)
{
        uint32_t tmp = TIMER->TIMER_CTRL_REG;
        REG_SET_FIELD(TIMER, TIMER_CTRL_REG, TIM_CAP_GPIO1_IRQ_EN, tmp, (mask & 0x1));
        REG_SET_FIELD(TIMER, TIMER_CTRL_REG, TIM_CAP_GPIO2_IRQ_EN, tmp, ((mask & 0x2) >> 1));
        REG_SET_FIELD(TIMER, TIMER_CTRL_REG, TIM_CAP_GPIO3_IRQ_EN, tmp, ((mask & 0x4) >> 2));
        REG_SET_FIELD(TIMER, TIMER_CTRL_REG, TIM_CAP_GPIO4_IRQ_EN, tmp, ((mask & 0x8) >> 3));
        TIMER->TIMER_CTRL_REG = tmp;
}


/**
 * \brief Set a GPIO input which triggers event1
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 */
__STATIC_INLINE void hw_timer_set_event1_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        TBA(id)->TIMER_GPIO1_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event2
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 */
__STATIC_INLINE void hw_timer_set_event2_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        TBA(id)->TIMER_GPIO2_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event3
 *
 * \param [in] gpio GPIO input
 *
 * \note Valid only for Timer
 *
 */
__STATIC_INLINE void hw_timer_set_event3_gpio(HW_TIMER_GPIO gpio)
{
        TIMER->TIMER_GPIO3_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event4
 *
 * \param [in] gpio GPIO input
 *
 * \note Valid only for Timer
 *
 */
__STATIC_INLINE void hw_timer_set_event4_gpio(HW_TIMER_GPIO gpio)
{
        TIMER->TIMER_GPIO4_CONF_REG = gpio;
}

/**
 * \brief Get clock source of the timer
 *
 * \param [in] id timer id
 * \return clock source
 *
 */
__STATIC_INLINE HW_TIMER_CLK_SRC hw_timer_get_clk(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_SYS_CLK_EN);
}

/**
 * \brief Get timer clock prescaler
 *
 * Actual timer frequency is \p timer_freq = \p freq_clock / (\p retval + 1)
 *
 * \param [in] id timer id
 *
 * \return prescaler value
 *
 * \sa hw_timer_get_prescaler_val
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_prescaler(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_PRESCALER_REG;
}

/**
 * \brief Get timer reload value
 *
 * \param [in] id timer id
 *
 * \return reload value
 *
 * \sa hw_timer_set_reload
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_reload(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_RELOAD_REG;
}

/**
 * \brief Get pulse delay in oneshot mode
 *
 * \param [in] id timer id
 *
 * \return delay (ticks)
 *
 * \sa hw_timer_get_oneshot_delay
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_oneshot_delay(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_RELOAD_REG;
}

/**
 * \brief Get shot width
 *
 * This applies only to one-shot mode.
 *
 * \param [in] id timer id
 *
 * \return shot width value
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_shot_width(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_SHOTWIDTH_REG;
}

/**
 * \brief Get free-running mode state
 *
 * \param [in] id timer id
 *
 * \return free-running mode state
 *
 */
__STATIC_INLINE bool hw_timer_get_freerun(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_FREE_RUN_MODE_EN);
}

/**
 * \brief Get a type of the edge which triggers event1
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event1_trigger(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN1_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event2
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event2_trigger(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN2_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event3. Valid only for Timer
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event3_trigger(void)
{
        return REG_GETF(TIMER, TIMER_CTRL_REG, TIM_IN3_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event4. Valid only for Timer
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event4_trigger(void)
{
        return REG_GETF(TIMER, TIMER_CTRL_REG, TIM_IN4_EVENT_FALL_EN);
}
/**
 * \brief Get a GPIO input which triggers event1.
 *
 * \param [in] id timer id
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event1_gpio(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_GPIO1_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event2
 *
 * \param [in] id timer id
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event2_gpio(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_GPIO2_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event3. Valid only for Timer
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event3_gpio(void)
{
        return TIMER->TIMER_GPIO3_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event4. Valid only for Timer
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event4_gpio(void)
{
        return TIMER->TIMER_GPIO4_CONF_REG;
}

/**
 * \brief Get the capture time for event on GPIO1
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO1
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture1(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_CAPTURE_GPIO1_REG;
}

/**
 * \brief Get the capture time for event on GPIO2
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO2
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture2(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_CAPTURE_GPIO2_REG;
}


/**
 * \brief Get the capture time for event on GPIO3. Valid only for Timer
 *
 * \return time for event on GPIO3
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture3(void)
{
        return TIMER->TIMER_CAPTURE_GPIO3_REG;
}

/**
 * \brief Get the capture time for event on GPIO4. Valid only for Timer
 *
 * \return time for event on GPIO4
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture4(void)
{
        return TIMER->TIMER_CAPTURE_GPIO4_REG;
}

/**
 * \brief Set the direction of timer counting
 *
 * \param [in] id timer id
 *
 * \param [in] dir counting direction of the timer, up or down
 *
 */
__STATIC_INLINE void hw_timer_set_direction(HW_TIMER_ID id, HW_TIMER_DIR dir)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_COUNT_DOWN_EN, dir);
}

/**
 * \brief Set timer mode
 *
 * \param [in] id timer id
 * \param [in] mode timer mode: '1' One shot, '0' counter
 *
 */
__STATIC_INLINE void hw_timer_set_mode(HW_TIMER_ID id, HW_TIMER_MODE mode)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_ONESHOT_MODE_EN, mode);
}

/**
 * \brief Return timer mode
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE HW_TIMER_MODE hw_timer_get_mode(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_ONESHOT_MODE_EN);
}

/**
 * \brief Get the tick count of the timer
 *
 * \param [in] id timer id
 *
 * \return current value of the timer ticks
 *
 * \sa hw_timer_get_prescaler_val
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_count(HW_TIMER_ID id)
{

        return TBA(id)->TIMER_TIMER_VAL_REG;
}

/**
 * \brief Get the current phase of the one shot mode
 *
 * \param [in] id timer id
 *
 * \return current phase of the one shot mode
 *
 */
__STATIC_INLINE HW_TIMER_ONESHOT hw_timer_get_oneshot_phase(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_ONESHOT_PHASE);
}

/**
 * \brief Get the current state of Event input 1 (IN1)
 *
 * \param [in] id timer id
 *
 * \return current logic level of IN1
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio1_state(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN1_STATE);
}

/**
 * \brief Get the time capture event GPIOs pending events.
 *
 * This function can be used to read the pending status of the capture
 * time event GPIOs (GPIO1 to GPIO4)
 *
 * \return event bitmap of GPIOs. When "1" GPIO event is pending.
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         Event     | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Only valid for Timer
 *
 */
__STATIC_INLINE uint8_t hw_timer_get_gpio_event_pending(void)
{
        return ((TIMER->TIMER_STATUS_REG &
                (TIMER_TIMER_STATUS_REG_TIM_GPIO1_EVENT_PENDING_Msk |
                 TIMER_TIMER_STATUS_REG_TIM_GPIO2_EVENT_PENDING_Msk |
                 TIMER_TIMER_STATUS_REG_TIM_GPIO3_EVENT_PENDING_Msk |
                 TIMER_TIMER_STATUS_REG_TIM_GPIO4_EVENT_PENDING_Msk)) >> 4);
}

/**
 * \brief Get the current state of Event input 2 (IN2)
 *
 * \param [in] id timer id
 *
 * \return current logic level of IN2
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio2_state(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN2_STATE);
}

/**
 * \brief Get the current prescaler counter value
 *
 * This is value of internal counter used for prescaling. It can be used to have finer granularity
 * when reading timer value.
 *
 * For reading current setting of prescaler, see hw_timer_get_prescaler().
 *
 * \param [in] id timer id
 *
 * \return current prescaler counter value
 *
 * \sa hw_timer_get_count
 * \sa hw_timer_get_prescaler
 *
 */
__STATIC_INLINE uint16_t hw_timer_get_prescaler_val(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_PRESCALER_VAL_REG;
}

/**
 * \brief Register an interrupt handler.
 *
 * \param [in] id timer id
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 *
 */
void hw_timer_register_int(HW_TIMER_ID id, hw_timer_handler_cb handler);

/**
 * \brief Unregister an interrupt handler
 *
 * \param [in] id timer id
 *
 */
void hw_timer_unregister_int(HW_TIMER_ID id);

/**
 * \brief Register an interrupt handler for GPIO triggered Timer Capture interrupt.
 *
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 * \param [in] gpio_mask bitmask of capture time event GPIOs. Set "1" to enable capture interrupt on GPIOx event
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         IRQ_EN    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 */
void hw_timer_register_capture_int(hw_timer_capture_handler_cb handler, uint8_t gpio_mask);

/**
 * \brief Unregister an interrupt handler for GPIO triggered Timer Capture interrupt
 *
 */
void hw_timer_unregister_capture_int(void);

/**
 * \brief Enable the timer
 *
 * \param [in] id timer id
 *
 * \note Assuming the timer clock is enabled, which is done during timer initialisation (hw_timer_init),
 *       the timer will start running immediately after the execution of this function.
 *
 */
__STATIC_INLINE void hw_timer_enable(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EN, 1);
}

/**
 * \brief Disable the timer
 *
 * \param [in] id timer id
 *
 * \note This function will disable the timer and timer clock
 *
 */
__STATIC_INLINE void hw_timer_disable(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EN, 0);
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 0);

}

/**
 * \brief Enable the timer clock
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_enable_clk(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 1);
}

/**
 * \brief Disable the timer clock
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_disable_clk(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 0);
}

/**
 * \brief Timer PWM configuration
 *
 * Shortcut to call appropriate configuration function.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 * \note PWM will not be enabled if either frequency or duty cycle are 0
 * \note PD COM should be enabled in order to configure PWM output PIN
 *
 */
void hw_timer_configure_pwm(HW_TIMER_ID id, const timer_config_pwm *cfg);

/**
 * \brief Set PWM frequency prescaler
 *
 * Actual PWM frequency is \p pwm_freq = \p timer_freq / (\p value + 1)
 *
 * \param [in] id timer id
 * \param [in] value PWM frequency defined as above
 *
 * \sa hw_timer_set_prescaler
 *
 */
__STATIC_INLINE void hw_timer_set_pwm_freq(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_PWM_FREQ_VAL >= value);
        TBA(id)->TIMER_PWM_FREQ_REG = value;
}

/**
 * \brief Set PWM duty cycle
 *
 * Actualy PWM duty cycle is \p pwm_dc = \p value / (\p pwm_freq + 1)
 *
 * \param [in] id timer id
 * \param [in] value PWM duty cycle defined as above
 *
 * \sa hw_timer_set_pwm_freq
 *
 */
__STATIC_INLINE void hw_timer_set_pwm_duty_cycle(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_PWM_DC_VAL >= value);
        TBA(id)->TIMER_PWM_DC_REG = value;
}

/**
 * \brief Get PWM frequency
 *
 * Actual PWM frequency is \p pwm_freq = \p timer_freq / (\p retval + 1)
 *
 * \param [in] id timer id
 *
 * \return PWM frequency as defined above
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_pwm_freq(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_PWM_FREQ_REG;
}

/**
 * \brief Get PWM duty cycle
 *
 * Actualy PWM duty cycle is \p pwm_dc = \p retval / (\p pwm_freq + 1)
 *
 * \param [in] id timer id
 *
 * \return PWM duty cycle as defined above
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_pwm_duty_cycle(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_PWM_DC_REG;
}


/**
 * \brief Clear capture time GPIO event.
 *
 * \param [in] mask bitmask of capture time event GPIOs. Set "1" to clear event
 * \parblock
 *         Bit:          |   3   |  2    |  1    |   0   |
 *                       +-------+-------+-------+-------+
 *         event GPIO    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                       +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Only valid for Timer
 *
 */
__STATIC_INLINE void hw_timer_clear_gpio_event(uint8_t mask)
{
        TIMER->TIMER_CLEAR_GPIO_EVENT_REG = mask;
}


/**
 * \brief Clear timer interrupt.
 *
 * Writing any value clears the interrupt
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_clear_interrupt(HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                TIMER->TIMER_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER2) {
                TIMER2->TIMER2_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER3) {
                TIMER3->TIMER3_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER4) {
                TIMER4->TIMER4_CLEAR_IRQ_REG = 0;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

#endif /* dg_configUSE_HW_TIMER */


#endif /* HW_TIMER_H_ */
/**
 * \}
 * \}
 */
