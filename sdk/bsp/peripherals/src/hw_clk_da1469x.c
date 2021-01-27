/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup CLK
\{
*/

/**
****************************************************************************************
*
* @file hw_clk_da1469x.c
*
* @brief Clock Driver
*
* Copyright (C) 2015-2020 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_CLK

#define USE_FAST_STARTUP        0

#include <stdint.h>
#include "hw_clk.h"

#if USE_FAST_STARTUP
typedef struct
{
        uint16_t temp_refpt;            // reference point for temperature compensation

        uint16_t base_drive_cycles;     // base drive cycles at room xtal32m_temp_refpt
} xtalm_otp_t;

__RETAINED static xtalm_otp_t xtalm_otp_values;
#endif
/*
 * Function definitions
 */

__RETAINED_CODE void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles)
{
        uint32_t val = 0;

        /* Must be disabled */
        ASSERT_WARNING(!REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START));

        ANAMISC_BIF->CLK_REF_CNT_REG = cycles;                      // # of cal clock cycles

        if (clk_ref_type == CALIBRATE_REF_EXT) {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, EXT_CNT_EN_SEL, val, 1);
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, 0); /* DivN to be calibrated */
        } else {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, clk_ref_type);
        }
        REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CLK_SEL, val, clk_type);
        ANAMISC_BIF->CLK_REF_SEL_REG = val;

        REG_SET_BIT(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START);
}

uint32_t hw_clk_get_calibration_data(void)
{
        /* Wait until it's finished */
        while (REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START)) {
        }

        return ANAMISC_BIF->CLK_REF_VAL_REG;
}

#define CLK_DELAY_SANITY_CHECKS
#pragma GCC push_options
#pragma GCC optimize ("O3")

void hw_clk_delay_usec(uint32_t usec)
{
        static const uint32_t DIVIDER = 1000000;

#ifdef CLK_DELAY_SANITY_CHECKS
        ASSERT_WARNING((dg_configXTAL32M_FREQ % DIVIDER) == 0);
        ASSERT_WARNING((dg_configPLL96M_FREQ % DIVIDER) == 0);
        ASSERT_WARNING((HW_CLK_DELAY_OVERHEAD_CYCLES % HW_CLK_CYCLES_PER_DELAY_REP) == 0);
#endif

        static const uint8_t OVERHEAD_REPS = HW_CLK_DELAY_OVERHEAD_CYCLES / HW_CLK_CYCLES_PER_DELAY_REP;
        static volatile uint32_t sys_freq_table[] = {
                dg_configXTAL32M_FREQ/DIVIDER,  // SYS_CLK_IS_XTAL32M
                dg_configXTAL32M_FREQ/DIVIDER,  // SYS_CLK_IS_RC32: Use XTAL32M frequency. This is the maximum frequency of RC32M
                                                // therefore it is guaranteed that the delay will be greater or equal to the
                                                // requested period.
                0,                              // SYS_CLK_IS_LP is not supported
                dg_configPLL96M_FREQ/DIVIDER    // SYS_CLK_IS_PLL
        };

        const uint32_t cycles_per_usec = sys_freq_table[hw_clk_get_sysclk()] >> hw_clk_get_hclk_div(),
                       reps = cycles_per_usec * usec / HW_CLK_CYCLES_PER_DELAY_REP;

#ifdef CLK_DELAY_SANITY_CHECKS
        ASSERT_WARNING(usec <= 0xFFFFFFFF/cycles_per_usec);     // The requested delay is greater than the maximum delay this function can achieve
        ASSERT_WARNING(reps > OVERHEAD_REPS);                   // The requested delay is smaller than the minimum delay this function can achieve.
#endif

        if (reps <= OVERHEAD_REPS) {
                return;
        }

        asm volatile(
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "loop:  nop                             \n"
                "       subs %[reps], %[reps], #1       \n"
                "       bne loop                        \n"
                :                                       // outputs
                : [reps] "r" (reps - OVERHEAD_REPS)     // inputs
                :                                       // clobbers
        );
}

#pragma GCC pop_options
#if USE_FAST_STARTUP
static void xtal32m_readOTP(void)
{
        xtalm_otp_values.temp_refpt = 100;                              // reference point for temperature compensation
        xtalm_otp_values.base_drive_cycles = 175;                       // base drive cycles at room xtal32m_temp_refpt
}
#endif

int16_t hw_clk_xtalm_update_rdy_cnt(void)
{
        int16_t xtalrdy_stat = 0;
        /*
         */
        uint8_t xtalrdy_cnt = 0;

        if (REG_GETF(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CLK_SEL) == 0) {
                xtalrdy_cnt = REG_GETF(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CNT);
                xtalrdy_stat = 3 - REG_GETF(CRG_XTAL, XTALRDY_STAT_REG, XTALRDY_STAT);
                xtalrdy_cnt += xtalrdy_stat;
                REG_SETF(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CNT, xtalrdy_cnt);
        }
        return xtalrdy_stat;
}

void hw_clk_xtalm_compensate_amp(void)
{
#if USE_FAST_STARTUP
        uint16_t T_drive, T_drive_lsb;
        uint8_t N;
        // perform amplitude compensation
        if (REG_GETF(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_RCOSC_XTAL_DRIVE) == 1) {
                uint16_t divisor = (0x8 << (0x7 - REG_GETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDRIVE)));

                T_drive = (xtalm_otp_values.base_drive_cycles * xtal32m_adcread()) / xtalm_otp_values.temp_refpt;
                N = T_drive / divisor;
                T_drive_lsb = T_drive - divisor * N;

                REG_SETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDRIVE_LSB, T_drive_lsb);
                REG_SETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_DRIVE_CYCLES, N+1);
        }
#endif
}

void hw_clk_xtalm_configure(void)
{
#if USE_FAST_STARTUP
        uint8_t settling_time;
        uint8_t cxcomp_phi_trim;

        xtal32m_readOTP();

        REG_SETF(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_CORE_CUR_SET, 3);                          // gmopt cur set.

        uint32_t reg = CRG_XTAL->XTAL32M_CTRL1_REG;
        REG_SET_FIELD(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_RCOSC_SYNC_DELAY_TRIM, reg, 0);      // synchronization delay trim
        REG_SET_FIELD(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDISCHARGE,    reg, 0);      // discharge time in drive sequence
        REG_SET_FIELD(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TSETTLE,       reg, 5);      // required settling time
        settling_time = 6;
        REG_SET_FIELD(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDRIVE,        reg, settling_time);      // unit drive length
        CRG_XTAL->XTAL32M_CTRL1_REG = reg;

        REG_SETF(CRG_XTAL, XTAL32M_CTRL2_REG, XTAL32M_RCOSC_TRIM_SNS, 118);                     // sensitivity of rcosc

        // sets TSETTLE to half of TDRIVE
        REG_SETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TSETTLE, settling_time);

        uint16_t T_drive, T_drive_lsb;
        uint8_t N;

        uint16_t divisor = (0x8 << (0x7 - REG_GETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDRIVE)));

        // equate FASTBOOT drive-cycles
        T_drive = xtalm_otp_values.base_drive_cycles;
        N = T_drive / divisor;
        T_drive_lsb = T_drive - divisor * N;

        REG_SETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_STARTUP_TDRIVE_LSB, T_drive_lsb);
        REG_SETF(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_DRIVE_CYCLES, N + 1);

        // fast startup mode
        REG_SETF(CRG_XTAL, TRIM_CTRL_REG, XTAL_TRIM_SELECT, 0x2);                       // always use direct trimming (disable legacy)

        // ~10us for ldo settling and 2 cycles for xtal voltage settling. IRQ handler also takes some time.
        // hclk divider needs to be set to 0
        uint16_t xtalrdy_cnt = T_drive/83 + 3 + 2;

        // Setup IRQ:
        REG_SET_BIT(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CLK_SEL);                     // use 256kHz clock
        REG_SETF(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CNT, xtalrdy_cnt);
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_RCOSC_CALIBRATE);
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_CXCOMP_ENABLE);
#else
        // Configure OSF BOOST
        uint8_t cxcomp_phi_trim = 0;
        uint8_t cxcomp_trim_cap = REG_GETF(CRG_XTAL, XTAL32M_CTRL2_REG, XTAL32M_CXCOMP_TRIM_CAP);

        // set phi compensation
        if (cxcomp_trim_cap < 37) {
                cxcomp_phi_trim = 3;
        } else {
                if (cxcomp_trim_cap < 123)
                        cxcomp_phi_trim = 2;
                else {
                        if (cxcomp_trim_cap < 170) {
                                cxcomp_phi_trim = 1;
                        }
                        else {
                                cxcomp_phi_trim = 0;
                        }
                }
        }

        REG_SETF(CRG_XTAL, XTAL32M_CTRL2_REG, XTAL32M_CXCOMP_PHI_TRIM, cxcomp_phi_trim);
#endif
}

#endif /* dg_configUSE_HW_CLK */


/**
\}
\}
\}
*/
