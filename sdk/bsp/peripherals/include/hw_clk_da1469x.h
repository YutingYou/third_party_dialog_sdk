/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_CLK HW Clock Driver
\{
\brief Clock Driver
*/

/**
****************************************************************************************
*
* @file hw_clk_da1469x.h
*
* @brief Clock Driver header file.
*
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_CLK_DA1469x_H_
#define HW_CLK_DA1469x_H_


#if dg_configUSE_HW_CLK

#include "sdk_defs.h"
#include "hw_sys.h"

#define HW_CLK_DELAY_OVERHEAD_CYCLES   (72)
#define HW_CLK_CYCLES_PER_DELAY_REP    (4)

/**
 * \brief Convert settling time (in usec) to 256KHz clock cycles
 *
 * \note The 256KHz clock is derived from RC32M divided by 125.
 */
#define XTAL32M_USEC_TO_256K_CYCLES(x)  ((uint16_t)((x * (dg_configRC32M_FREQ/1000000) + 63) / 125 ))

/**
 * \brief Convert XTAL32M Ready IRQ counter cycles to LP clock cycles
 */
#define XTALRDY_CYCLES_TO_LP_CLK_CYCLES(x, lp_freq) ((((uint32_t)(x)) * lp_freq + dg_configRC32M_FREQ_MIN/(125) - 1) / (dg_configRC32M_FREQ_MIN/125))

/**
 * \addtogroup CLOCK_TYPES
 * \{
 */

/**
 * \brief The type of the system clock
 */
typedef enum sys_clk_is_type {
        SYS_CLK_IS_XTAL32M = 0,
        SYS_CLK_IS_RC32,
        SYS_CLK_IS_LP,
        SYS_CLK_IS_PLL,
        SYS_CLK_IS_INVALID
} sys_clk_is_t;

/**
 * \}
 */

/**
 * \brief The type of clock to be calibrated
 */
typedef enum cal_clk_sel_type {
        CALIBRATE_RC32K = 0,
        CALIBRATE_RC32M,
        CALIBRATE_XTAL32K,
        CALIBRATE_RCX,
        CALIBRATE_RCOSC,
} cal_clk_t;

/**
 * \brief The reference clock used for calibration
 */
typedef enum cal_ref_clk_sel_type {
        CALIBRATE_REF_DIVN = 0,
        CALIBRATE_REF_RC32K,
        CALIBRATE_REF_RC32M,
        CALIBRATE_REF_XTAL32K,
        CALIBRATE_REF_RCOSC,
        CALIBRATE_REF_EXT,
} cal_ref_clk_t;

/**
 * \brief The system clock type
 *
 * \note Must only be used with functions cm_sys_clk_init/set()
 */
typedef enum sysclk_type {
        sysclk_RC32    = 0,     //!< RC32
        sysclk_XTAL32M = 2,     //!< 32MHz
        sysclk_PLL96   = 6,     //!< 96MHz
        sysclk_LP      = 255,   //!< not applicable
} sys_clk_t;

/**
 * \brief The CPU clock type (speed)
 *
 */
typedef enum cpu_clk_type {
        cpuclk_2M = 2,          //!< 2 MHz
        cpuclk_4M = 4,          //!< 4 MHz
        cpuclk_8M = 8,          //!< 8 MHz
        cpuclk_16M = 16,        //!< 16 MHz
        cpuclk_32M = 32,        //!< 32 MHz
        cpuclk_96M = 96         //!< 96 MHz
} cpu_clk_t;

/**
 * \brief Check if the RC32M is enabled.
 *
 * \return true if the RC32M is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_rc32_status(void)
{
        return REG_GETF(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Activate the RC32M.
 */
__STATIC_INLINE void hw_clk_enable_rc32(void)
{
        REG_SET_BIT(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Deactivate the RC32M.
 */
__STATIC_FORCEINLINE void hw_clk_disable_rc32(void)
{
        REG_CLR_BIT(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Set the XTAL32M settling time.
 *
 * \param cycles Number of clock cycles
 * \param high_clock If true use 256kHZ clock, false use 32kHz clock
 */
__STATIC_FORCEINLINE void hw_clk_set_xtalm_settling_time(uint8_t cycles, bool high_clock)
{
        uint32_t val = CRG_XTAL->XTALRDY_CTRL_REG;
        REG_SET_FIELD(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CNT, val, cycles);
        REG_SET_FIELD(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CLK_SEL, val, high_clock ? 1 : 0);

        CRG_XTAL->XTALRDY_CTRL_REG = val;
}

/**
 * \brief Get the XTAL32M settling time.
 *
 * \return The number of 256KHz clock cycles required for XTAL32M to settle
 */
__STATIC_FORCEINLINE uint16_t hw_clk_get_xtalm_settling_time(void)
{
        uint32_t val = CRG_XTAL->XTALRDY_CTRL_REG;
        uint16_t cycles = REG_GET_FIELD(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CNT, val);

        if (REG_GET_FIELD(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CLK_SEL, val) == 0) {
                // 32KHz clock cycles. Convert them to 256KHz clock cycles.
                cycles *= 8;
        }
        return cycles;
}

/**
 * \brief Check if the XTAL32M is enabled.
 *
 * \return true if the XTAL32M is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_xtalm_status(void)
{
        return REG_GETF(CRG_XTAL, XTAL32M_STAT1_REG, XTAL32M_STATE) != 0xB;
}

/**
 * \brief Activate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_enable_xtalm(void)
{
        /* Do nothing if XTAL32M is already up and running. */
        if (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)) {
                return;
        }

        // Check if TIM power domain is enabled
        ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));

        // Check the power supply
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE) ||
                       (REG_GETF(DCDC, DCDC_CTRL1_REG, DCDC_ENABLE) &&
                        REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV) &&
                        REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV)))

        /* Enable the XTAL oscillator. */
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_XTAL_ENABLE);
}

/**
 * \brief Deactivate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_disable_xtalm(void)
{
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL1_REG, XTAL32M_XTAL_ENABLE);
}

/**
 * \brief Check if the XTAL32M has settled.
 *
 * \return true if the XTAL32M has settled, else false.
 */
__STATIC_INLINE bool hw_clk_is_xtalm_started(void)
{
        return REG_GETF(CRG_XTAL, XTALRDY_STAT_REG, XTALRDY_COUNT) == 0 && REG_GETF(CRG_XTAL, XTAL32M_STAT1_REG, XTAL32M_STATE) != 0xB;
}

/**
 * \brief Return the clock used as the system clock.
 *
 * \return The type of the system clock
 */
__STATIC_FORCEINLINE sys_clk_is_t hw_clk_get_sysclk(void)
{
        static const uint32_t freq_msk = CRG_TOP_CLK_CTRL_REG_RUNNING_AT_LP_CLK_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_RC32M_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_XTAL32M_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_PLL96M_Msk;

        static __RETAINED_CONST_INIT sys_clk_is_t clocks[] = {
                SYS_CLK_IS_LP,          // 0b000
                SYS_CLK_IS_RC32,        // 0b001
                SYS_CLK_IS_XTAL32M,     // 0b010
                SYS_CLK_IS_INVALID,
                SYS_CLK_IS_PLL          // 0b100
        };

        // drop bit0 to reduce the size of clocks[]
        uint32_t index = (CRG_TOP->CLK_CTRL_REG & freq_msk) >> (CRG_TOP_CLK_CTRL_REG_RUNNING_AT_LP_CLK_Pos + 1);
        ASSERT_WARNING(index <= 4);

        sys_clk_is_t clk = clocks[index];
        ASSERT_WARNING(clk != SYS_CLK_IS_INVALID);
        return clk;
}

/**
 * \brief Check whether the XTAL32K is the Low Power clock.
 *
 * \return true if XTAL32K is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_xtal32k(void)
{
        return REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_XTAL32K);
}

/**
 * \brief Check whether the RC32K is the Low Power clock.
 *
 * \return true if RC32K is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rc32k(void)
{
        return REG_GETF(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RC32K);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rcx(void)
{
        return REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCX);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_external(void)
{
        return REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_EXTERNAL;
}

/**
 * \brief Set RCX as the Low Power clock.
 *
 * \warning The RCX must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rcx(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RCX);
}

/**
 * \brief Set XTAL32K as the Low Power clock.
 *
 * \warning The XTAL32K must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_xtal32k(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_XTAL32K);
}

/**
 * \brief Set an external digital clock as the Low Power clock.
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_ext32k(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_EXTERNAL);
}

/**
 * \brief Enable RC32K.
 */
__STATIC_INLINE void hw_clk_enable_rc32k(void)
{
        REG_SET_BIT(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE);
}

/**
 * \brief Disable RC32K.
 *
 * \warning RC32K must not be the LP clock.
 */
__STATIC_INLINE void hw_clk_disable_rc32k(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RC32K);

        REG_CLR_BIT(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE);
}

/**
 * \brief Set RC32K as the Low Power clock.
 *
 * \warning The RC32K must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rc32k(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RC32K);
}

/**
 * \brief Configure RCX. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_rcx(void)
{
        // Reset values for CLK_RCX_REG register should be used
}

/**
 * \brief Enable RCX but does not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_rcx(void)
{
        REG_SET_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

/**
 * \brief Disable RCX.
 *
 * \warning RCX must not be the LP clock
 */
__STATIC_INLINE void hw_clk_disable_rcx(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RCX);

        REG_CLR_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

/**
 * \brief Configure XTAL32K. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_xtal32k(void)
{
        // Configure xtal.
        uint32_t reg = CRG_TOP->CLK_XTAL32K_REG;
        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_CUR, reg, 5);
        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_RBIAS, reg, 3);

        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_DISABLE_AMPREG, reg, dg_configEXT_LP_IS_DIGITAL);

        CRG_TOP->CLK_XTAL32K_REG = reg;
}

/**
 * \brief Enable XTAL32K but do not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_xtal32k(void)
{
        REG_SET_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
}

/**
 * \brief Disable XTAL32K.
 *
 * \warning XTAL32K must not be the LP clock.
 */
__STATIC_INLINE void hw_clk_disable_xtal32k(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_XTAL32K);
        REG_CLR_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
}

/**
 * \brief Check the status of a requested calibration.
 *
 * \return true if the calibration has finished (or never run) else false.
 */
__STATIC_INLINE bool hw_clk_calibration_finished(void)
{
        return REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START) == 0;
}

/**
 * \brief Start calibration of a clock.
 *
 * \param[in] clk_type The clock to be calibrated. Must be enabled.
 * \param[in] clk_ref_type The reference clock to USE.
 * \param[in] cycles The number of cycles of the to-be-calibrated clock to be measured using the
 *            reference clock.
 *
 * \warning If clk_ref_type == CALIBRATE_REF_EXT, the clk_type is not used. Instead, the value
 *          returned by hw_clk_get_calibration_data() is the number of clock cycles of DIVN counted
 *          during a single pulse of the EXT clock source used.
 *          The EXT clock source must be applied to a pin with a PID equal to HW_GPIO_FUNC_UART_RX
 */
void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles);

/**
 * \brief Return the calibration results.
 *
 * \return The number of cycles of the reference clock corresponding to the programmed
 * (in hw_clk_start_calibration() cycles param) cycles of the clock to be calibrated.
 * In the special case of EXTernal calibration, this function returns the number of cycles of DIVN
 * that correspond to one positive pulse of the EXT source applied.
 */
uint32_t hw_clk_get_calibration_data(void);

/**
 * \brief Set System clock.
 *
 * \param[in] mode The new system clock.
 *
 * \note System clock switch to PLL is only allowed when current system clock is XTAL32M.
 * System clock switch from PLL is only allowed when new system clock is XTAL32M.
 */
__STATIC_INLINE void hw_clk_set_sysclk(sys_clk_is_t mode)
{
        /* Make sure a valid sys clock is requested */
        ASSERT_WARNING(mode <= SYS_CLK_IS_PLL);

        /* Switch to PLL is only allowed when current system clock is XTAL32M */
        ASSERT_WARNING(mode != SYS_CLK_IS_PLL ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)  ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M))

        /* Switch to PLL is only allowed when HDIV and PDIV are 0 */
        ASSERT_WARNING(mode != SYS_CLK_IS_PLL || (hw_clk_get_hclk_div() == ahb_div1 &&  hw_clk_get_pclk_div() == apb_div1));

        /* Switch from PLL is only allowed when new system clock is XTAL32M */
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M) ||
                mode == SYS_CLK_IS_XTAL32M  ||
                mode == SYS_CLK_IS_PLL);

        if (mode == SYS_CLK_IS_XTAL32M && REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RC32M)) {
                /* XTAL32M may have been enabled by the PDC. Check the power supply. */
                ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE) ||
                               (REG_GETF(DCDC, DCDC_CTRL1_REG, DCDC_ENABLE) &&
                                REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_HV) &&
                                REG_GETF(DCDC, DCDC_V14_REG, DCDC_V14_ENABLE_LV)))

                REG_SET_BIT(CRG_TOP, CLK_SWITCH2XTAL_REG, SWITCH2XTAL);
        }
        else {
                GLOBAL_INT_DISABLE();
                REG_SETF(CRG_TOP, CLK_CTRL_REG, SYS_CLK_SEL, mode);
                GLOBAL_INT_RESTORE();
        }

        /* Wait until the switch is done! */
        switch (mode) {
        case SYS_CLK_IS_XTAL32M:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)) {
                }
                return;

        case SYS_CLK_IS_RC32:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RC32M)) {
                }
                return;

        case SYS_CLK_IS_LP:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_LP_CLK)) {
                }
                return;

        case SYS_CLK_IS_PLL:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M)) {
                }
                return;
        default:
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Enable the PLL.
 */
__STATIC_FORCEINLINE void hw_clk_pll_sys_on(void)
{
        GLOBAL_INT_DISABLE();

        /* Workaround for "Errata issue 299": XTAL32M oscillator: Track and Hold timing */
        while (!hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_PLL_ENABLE_POS));

        /* LDO_CORE voltage must be set to 1.2V prior to enabling PLL */
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL) == 3);


        /*  Enable DXTAL for the system PLL */
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_DXTAL_SYSPLL_ENABLE);

        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_PLL_ENABLE_POS);

        /* LDO PLL enable. */
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE);

        /* Configure system PLL. */
        //    REG_SETF(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_SEL_R_DIV_TEST, 1); /* Default/reset value. */

        /* Program N-divider and DEL_SEL. */
        //    REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_SEL_MIN_CUR_INT);  // Last review date: Feb 15, 2016 - 12:25:47

        /* Check the status of the PLL LDO before enabling it! */
        while (!REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, LDO_PLL_OK));

        /* Now turn on PLL. */
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN);

        /* Added only for debugging purposes. */
        ASSERT_WARNING(REG_GETF(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_DXTAL_SYSPLL_ENABLE));

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the PLL.
 *
 * \warning The System clock must have been set to XTAL32M before calling this function!
 */
__STATIC_FORCEINLINE void hw_clk_pll_sys_off(void)
{
        GLOBAL_INT_DISABLE();

        // The PLL is not the system clk.
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M));

        uint32_t val = CRG_XTAL->PLL_SYS_CTRL1_REG;

        /* Turn off PLL. */
        REG_CLR_FIELD(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN, val);
        /* LDO PLL disable. */
        REG_CLR_FIELD(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE, val);

        CRG_XTAL->PLL_SYS_CTRL1_REG = val;

        while (!hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_PLL_ENABLE_POS));

        /*  Disable DXTAL for the system PLL */
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_DXTAL_SYSPLL_ENABLE);

        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_PLL_ENABLE_POS);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the PLL is enabled.
 *
 * \return true if the PLL is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_pll_status(void)
{
        return REG_GETF(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN);
}

/**
 * \brief Check if the PLL is on and has locked.
 *
 * \return true if the PLL has locked, else false.
 */
__STATIC_INLINE bool hw_clk_is_pll_locked(void)
{
        return REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE);
}

/**
 * \brief Activate a System clock.
 *
 * \param[in] clk The clock to activate.
 */
__STATIC_INLINE void hw_clk_enable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_enable_xtalm();
                return;
        case SYS_CLK_IS_RC32:
                hw_clk_enable_rc32();
                return;
        case SYS_CLK_IS_PLL:
                hw_clk_pll_sys_on();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate a System clock.
 *
 * \param[in] clk The clock to deactivate.
 */
__STATIC_INLINE void hw_clk_disable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_disable_xtalm();
                return;
        case SYS_CLK_IS_RC32:
                hw_clk_disable_rc32();
                return;
        case SYS_CLK_IS_PLL:
                hw_clk_pll_sys_off();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Check if a System clock is enabled.
 *
 * \return true if the System clock is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_is_enabled_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                return hw_clk_check_xtalm_status();
        case SYS_CLK_IS_RC32:
                return hw_clk_check_rc32_status();
        case SYS_CLK_IS_PLL:
                return hw_clk_check_pll_status();
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
                return false;
        }
}

/**
 * \brief Configure pin to connect an external digital clock.
 */
__STATIC_INLINE void hw_clk_configure_ext32k_pins(void)
{
        GPIO-> P0_23_MODE_REG = 0;
}

/**
 * \brief Configure XTAL32M.
 */
void hw_clk_xtalm_configure(void);

/**
 * \brief Perform XTAL32M RCOSC amplitude temperature compensation.
 */
void hw_clk_xtalm_compensate_amp(void);

/**
 * \brief Update XTAL32M Ready IRQ counter.
 *
 * \return The difference between the new and the old XTAL32M Ready IRQ counter
 *         in cycles of 32KHz clocks.
 */
int16_t hw_clk_xtalm_update_rdy_cnt(void);

#endif /* dg_configUSE_HW_CLK */


#endif /* HW_CLK_DA1469x_H_ */

/**
\}
\}
*/
