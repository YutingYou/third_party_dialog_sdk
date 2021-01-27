/**
 ****************************************************************************************
 *
 * @file Xtal_TRIM_da1469x.c
 *
 * @brief Xtal trim for DA1469x
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


/*
 inputs:
 auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin)
 Xtal = 32MHz
 TRIM-limits are dependent of the XTAL
 port_number: input of the 300 msec signal for XAL-calibration
 e.g. 0 = P0_0-P0_31 or P1_0-P1_22 (00-31 or 100-122)
 outputs:
 return (-1)     // -1 = square pulse outside boundaries
 return (-2)     // -2 = no square pulse detected
 return (-3)     // -3 = failed to write otp value
 return (-4)     // -4 = wrong input of port_number
 return (-5)     // -5 = wrong input of XTAL_select
 return (TRIM)   // TRIM-value is returned
 */

#include <stdio.h>
#include <hw_gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "Xtal_TRIM.h"

/*
 *  Macro definitions
 */

// Status codes
#define PULSE_OUT_OF_RANGE_ERROR        (-1)            // Pulse found in the pulse pin assigned GPIO was out of acceptable range
#define NO_PULSE_ERROR                  (-2)            // No pulse found, or pulse > 740 ms (measure_pulse aborts)
#define WRITING_VAL_TO_OTP_ERROR        (-3)            // Failed to write value in OTP.
#define INVALID_GPIO_ERROR              (-4)            // Wrong GPIO configuration.
#define WRONG_XTAL_SOURCE_ERROR         (-5)            // Incorrect pulse detected.

// General parameters
#define TEMP_OFFSET                     0               // 9.6 = 1 ppm (32M)
#define ACCURACY_STEP                   9               // using the SYSTICK: accuracy is 9 clocks
#define DELAY_1MSEC                                      1777                            // delay x * 1 msec
#define PPM_1                           10              // 1.04 ppm (9.6M)
#define PPM_2                           20              // 2.08 ppm (9.632M)

#define PPM_BOUNDARY                    96              // 96 = 10 ppm (9M6) at 32 MHz

#define NB_TRIM_DATA                    11              // max amount of itterations

// XTAL_32M specific 69x
#define XTAL32M                         9600000         // 300 msec  TRIM = 252 (ideal 32M * 0.3 = 9.6M)
#define TRIM_MIN_32M                    10              // minimum TRIM value (limits 4 - 12 pF)
#define TRIM_MAX_32M                    700             // maximum TRIM value

// Boundaries 69x
volatile uint32_t border_1;                             // TRIM_MIN            // for linearization
#define border_2            200  //700
#define border_3            350 //1150
#define border_4            500 //1600
volatile uint32_t border_5;                             // TRIM_MAX

/*
 * Variables
 */
int32_t cnt_output_temp;                                // temp ccc1
volatile uint32_t TRIM_MIN;
volatile uint32_t TRIM_MAX;
volatile uint32_t IDEAL_CNT;                            // ideal value of XTAL
bool PulseError = false;                                // __585__ // Error_Flag when no pulses arriving.
int32_t calc_output;
uint8_t Q;                                              // quadrant number
uint8_t Clock_Read_count = 0;
volatile uint32_t Fr[NB_TRIM_DATA];                     // input TRIM value output measured counter value (appr 9.6M counts)
volatile uint32_t Trim[NB_TRIM_DATA];
volatile uint32_t C[NB_TRIM_DATA];
volatile uint32_t actual_trimming_value;

/*
 * Debug parameters
 */
#if AUTO_XTAL_TEST_DBG_EN

#endif

/*
 * Function Declaration
 */
void delay(uint32_t dd);
void Setting_Trim(uint32_t Trim_Value);
long Clock_Read(uint8_t port, uint8_t pin);
//int Overall_calculation(uint8_t port_number);
int linearization(int C, int Cmin, int Cmax, int Tmin, int Tmax);
int16_t auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin);

// *** delay routine x * 1 msec / is controlled by 32M XTAL
void delay(uint32_t dd)
{
        uint32_t j, jj;

        dd = 2 * dd;                                    // for 32 MHz Xtal

        jj = dd * DELAY_1MSEC;

        for (j = 1; j <= jj; j++)
                {
                __NOP();
                __NOP();
        }
}

// *** boundary testing new TRIM-value and storing in CLK_FREQ_TRIM_REG
void Setting_Trim(uint32_t Trim_Value)                  // program new Trim_Value
{
        if (Trim_Value < TRIM_MIN && Trim_Value != 0)
                Trim_Value = TRIM_MIN;
        if (Trim_Value > TRIM_MAX)
                Trim_Value = TRIM_MAX;

        REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, Trim_Value); //Trim_Value;

        delay(2);                                       // delay = 2msec
}

/**
 ******************************************************************************************
 * @brief Measures the high duration of an externally applied square pulse in system ticks.
 *
 * @param[in] port_number    GPIO where the square pulse is applied on.
 * @return   Zero if no external pulse is detected on the given GPIO.
 *           Otherwise the pulse high duration in system ticks is returned.
 ******************************************************************************************
 */
long Clock_Read(uint8_t port, uint8_t pin) // testing block wave via input e.g. P0_6 ... port can be selected
{
        int32_t cnt_output = 0;
        //uint8_t port_number_10, port_number_1;                // 10th and 1th e.g. 2 and 3 => port P2_3
        uint32_t shift_bit;
        uint32_t datareg;
        volatile uint32_t tick_counter = 0;

        Clock_Read_count++;                                     // counting the loops

        shift_bit = (1 << pin);

        switch (port)
        {
        case 0:
                datareg = (uint32_t)(&(GPIO->P0_DATA_REG));
                break;
        case 1:
                datareg = (uint32_t)(&(GPIO->P1_DATA_REG));
                break;
        default:
                return 0;
        }

        /* configure systick timer */
        SysTick->LOAD = 0xFFFFFF;
        SysTick->VAL = 0;
        SysTick->CTRL |= 0x04;                                  // use processor-clock

        /* during counting, no interrupts should appear */
        GLOBAL_INT_DISABLE()
                ;                                               // disable interrupts

                tick_counter = MEASURE_PULSE(datareg, shift_bit);
                SysTick->CTRL &= ~(0x01);                       // stop systick timer ... bit 0: ENABLE

                GLOBAL_INT_RESTORE();                           // enable interrupts

        cnt_output = 0xFFFFFF - tick_counter;

        PulseError = false;
        if (cnt_output == 0xFFFFFF)
                {
                PulseError = true;
        }

        /* test ### */
        cnt_output_temp = cnt_output;
        calc_output = cnt_output;

        return cnt_output;

}

// *** calculate new TRIM via linear line algorithm
int linearization(int C, int Cmin, int Cmax, int Tmin, int Tmax)
{
        int T;
        volatile int32_t temp;
        /* C = counter at 9.6M (ideal value) */

        temp = ((C - Cmin) * (Tmax - Tmin)) / (Cmax - Cmin);
        T = (int)Tmax - temp;

        return T;
}

/* main function is start of auto-calibration */

int16_t auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        int32_t Trim_low;                               // left side value e.g. 350
        int32_t Trim_hi;                                // right side value e.g 500

        int32_t C_max, C_min;

        int32_t Trim_next = 350;                        // start with first value
        int8_t loop = 1;                                // loop 1 ... max 10

        int32_t temp, temp0;                            // temporary value

        IDEAL_CNT = XTAL32M - 4 + TEMP_OFFSET;

        border_1 = TRIM_MIN_32M;                        // MIN and MAX vakues of 32M Xtal
        border_5 = TRIM_MAX_32M;

        TRIM_MIN = border_1;
        TRIM_MAX = border_5;

        /* using p00 as input for 300ms */

        hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO);

        Trim_next = 350;

        // ** 0e Setting_Trim & temp = Clock_Read()
        Setting_Trim(Trim_next);                        // start e.g. Trim at 350

        // ** 1e Clock_Read
        temp0 = Clock_Read(port, pin);
        temp = temp0;

        // ** 2e set Trim at Tmax or Tmin
        if (temp > XTAL32M) {
                Setting_Trim(TRIM_MAX_32M);             // start e.g. Trim at 700
                Trim_next = 700;
        }
        else {
                Setting_Trim(TRIM_MIN_32M);             // start e.g. Trim at 10
                Trim_next = 10;
        }
        // ** 3e Clock Read at Trim
        temp = Clock_Read(port, pin); // at Tmin (at C_max) or Tmax (at C_min)

        loop = 0;

        do
        {
                loop++;                                 // max amount of loops

                // ** 4e if abs(temp - C_ideal) <= in spec => break
                if ((temp > XTAL32M) && (temp - XTAL32M) <= PPM_1) // XTAL32M = C_ideal (9.6M at 300ms)
                        break;  // out of while
                if ((temp < XTAL32M) && (XTAL32M - temp) <= PPM_1) // XTAL32M = C_ideal (9.6M at 300ms)
                        break;  // out of while

                // ** 5e if (temp < C_ideal)
                if (temp > XTAL32M)
                {
                        Trim_hi = 350;                  // right side graph
                        Trim_low = Trim_next;
                        C_min = temp0;                  // at 350
                        C_max = temp;                   // at < 350
                }
                else
                {
                        Trim_hi = Trim_next;            // left side graph
                        Trim_low = 350;
                        C_min = temp;                   // at > 350
                        C_max = temp0;                  // at 350
                }

                // **   6e      Trim_next = sub linearization
                Trim_next = linearization(XTAL32M, C_min, C_max, Trim_low, Trim_hi);

                // ** 7e Trim = Trim_next
                Setting_Trim(Trim_next);

                // ** 8e temp = Clock_Read()
                temp = Clock_Read(port, pin);
        }
        while (loop < 10);

        // ** check square pulse
        if (PulseError == true)                         // no square pulse detected
        {
                Setting_Trim(0);                        // if no square pulse was detected, then TRIM = 0
                return NO_PULSE_ERROR;                  // no square pulse detected
        }

        if (calc_output < (IDEAL_CNT - PPM_1))
                {
                Setting_Trim(0);                        // if no square pulse was detected, then TRIM = 0
                return PULSE_OUT_OF_RANGE_ERROR;        // no square pulse detected
        }

        actual_trimming_value = REG_GETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM); // read TRIM register

        if (actual_trimming_value <= TRIM_MIN && actual_trimming_value > 0)
                return PULSE_OUT_OF_RANGE_ERROR;        // TRIM-value = 10  out of spec!
        if (actual_trimming_value >= TRIM_MAX)
                return PULSE_OUT_OF_RANGE_ERROR;        // {32M} TRIM-value = 700 out of spec!

        return (actual_trimming_value);                 // actual TRIM-value is send back
}

/*
 * Debug function
 */
#if AUTO_XTAL_TEST_DBG_EN
void TRIM_test (int S1, int S2) // measuring Arrays in TRIM-values
{
        volatile signed int i, j;
        static int ff[2050];

        for (i=S1; i<=S2; i++)
        {
                REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, i);

                ff[i] = Clock_Read(23);
                j = 9600000 - ff[i];
                __NOP();
        }

}
#endif

