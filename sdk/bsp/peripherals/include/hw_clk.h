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
* @file hw_clk.h
*
* @brief Clock Driver header file.
*
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_CLK_H_
#define HW_CLK_H_


#if dg_configUSE_HW_CLK

#include "sdk_defs.h"

/**
 * \addtogroup CLOCK_TYPES
 * \brief Clock types
 * \note These macros must only be used with hw_clk_set_sysclk() and hw_clk_set_lp_clk().
 * \{
 */

/**
 * \brief The type of the LP clock
 */
typedef enum lp_clk_is_type {
        LP_CLK_IS_RC32K = 0,
        LP_CLK_IS_RCX,
        LP_CLK_IS_XTAL32K,
        LP_CLK_IS_EXTERNAL,
        LP_CLK_IS_INVALID
} lp_clk_is_t;

/**
 * \}
 */

/**
 * \brief The AMBA High-Performance Bus (AHB) clock divider
 */
typedef enum ahbdiv_type {
        ahb_div1 = 0,           //!< Divide by 1
        ahb_div2,               //!< Divide by 2
        ahb_div4,               //!< Divide by 4
        ahb_div8,               //!< Divide by 8
        ahb_div16,              //!< Divide by 16
} ahb_div_t;

/**
 * \brief The AMBA Peripheral Bus (APB) clock divider
 */
typedef enum apbdiv_type {
        apb_div1 = 0,           //!< Divide by 1
        apb_div2,               //!< Divide by 2
        apb_div4,               //!< Divide by 4
        apb_div8,               //!< Divide by 8
} apb_div_t;

/**
 * \brief Get the divider of the AMBA High Speed Bus.
 *
 * \return The AMBA High Speed Bus divider
 */
__STATIC_FORCEINLINE ahb_div_t hw_clk_get_hclk_div(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV);
}

/**
 * \brief Get the divider of the AMBA Peripheral Bus.
 *
 * \return The AMBA Peripheral Bus divider
 */
__STATIC_FORCEINLINE apb_div_t hw_clk_get_pclk_div(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, PCLK_DIV);
}

        #include "hw_clk_da1469x.h"

/**
 * \brief Set the divider of the AMBA High Speed Bus.
 *
 * \param div The AMBA High Speed Bus divider
 */
__STATIC_FORCEINLINE void hw_clk_set_hclk_div(ahb_div_t div)
{
        ASSERT_WARNING(div <= ahb_div16);

        // When system clock is PLL only ahb_div1 is allowed
        ASSERT_WARNING(div == ahb_div1 || hw_clk_get_sysclk() != SYS_CLK_IS_PLL);

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, div);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the divider of the AMBA Peripheral Bus.
 *
 * \param div The AMBA Peripheral Bus divider
 */
__STATIC_FORCEINLINE void hw_clk_set_pclk_div(apb_div_t div)
{
        ASSERT_WARNING(div <= apb_div8);

        // When system clock is PLL only apb_div1 is allowed
        ASSERT_WARNING(div == apb_div1 || hw_clk_get_sysclk() != SYS_CLK_IS_PLL);

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, PCLK_DIV, div);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set Low Power clock.
 *
 * \param[in] mode The new low power clock.
 */
__STATIC_INLINE void hw_clk_set_lpclk(lp_clk_is_t mode)
{
        GLOBAL_INT_DISABLE();
        switch (mode) {
        case LP_CLK_IS_RC32K:
                hw_clk_lp_set_rc32k();
                break;
        case LP_CLK_IS_RCX:
                hw_clk_lp_set_rcx();
                break;
        case LP_CLK_IS_XTAL32K:
                hw_clk_lp_set_xtal32k();
                break;
        case LP_CLK_IS_EXTERNAL:
                hw_clk_lp_set_ext32k();
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check whether a clock is the Low Power clock.
 *
 * \param[in] clk The clock to check.
 *
 * \return true if clk is the Low Power clock, else false.
 */
__STATIC_INLINE bool hw_clk_lpclk_is(lp_clk_is_t clk)
{
        switch (clk) {
        case LP_CLK_IS_RC32K:
                return hw_clk_lp_is_rc32k();
        case LP_CLK_IS_RCX:
                return hw_clk_lp_is_rcx();
        case LP_CLK_IS_XTAL32K:
                return hw_clk_lp_is_xtal32k();
        case LP_CLK_IS_EXTERNAL:
                return hw_clk_lp_is_external();
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
                return false;
        }
}

/**
 * \brief Return the clock used as the Low Power clock.
 *
 * \return The type of the Low Power clock
 *
 */
__STATIC_INLINE lp_clk_is_t hw_clk_get_lpclk(void)
{
        lp_clk_is_t lp_clk;
        for (lp_clk = 0; lp_clk < LP_CLK_IS_INVALID; lp_clk++) {
                if (hw_clk_lpclk_is(lp_clk)) {
                        return lp_clk;
                }
        }
        ASSERT_WARNING(0);
        return LP_CLK_IS_INVALID;
}

/**
 * \brief Configure a Low Power clock. This must be done only once since the register is retained.
 *
 * \param[in] clk The clock to configure.
 */
__STATIC_INLINE void hw_clk_configure_lpclk(lp_clk_is_t clk)
{
        switch (clk) {
        case LP_CLK_IS_RC32K:
                // Nothing to do for RC32K
                return;
        case LP_CLK_IS_RCX:
                hw_clk_configure_rcx();
                return;
        case LP_CLK_IS_XTAL32K:
                hw_clk_configure_xtal32k();
                return;
        case LP_CLK_IS_EXTERNAL:
                // Nothing to do for external LP clock
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Activate a Low Power clock.
 *
 * \param[in] clk The clock to activate.
 */
__STATIC_INLINE void hw_clk_enable_lpclk(lp_clk_is_t clk)
{
        switch (clk) {
        case LP_CLK_IS_RC32K:
                hw_clk_enable_rc32k();
                return;
        case LP_CLK_IS_RCX:
                hw_clk_enable_rcx();
                return;
        case LP_CLK_IS_XTAL32K:
                hw_clk_enable_xtal32k();
                return;
        case LP_CLK_IS_EXTERNAL:
                // Nothing to do for external LP clock
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate a Low Power clock.
 *
 * \param[in] clk The clock to deactivate.
 */
__STATIC_INLINE void hw_clk_disable_lpclk(lp_clk_is_t clk)
{
        switch (clk) {
        case LP_CLK_IS_RC32K:
                hw_clk_disable_rc32k();
                return;
        case LP_CLK_IS_RCX:
                hw_clk_disable_rcx();
                return;
        case LP_CLK_IS_XTAL32K:
                hw_clk_disable_xtal32k();
                return;
        case LP_CLK_IS_EXTERNAL:
                // Nothing to do for external LP clock
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Add delay of N usecs.
 *
 * \param[in] usec The number of usecs to wait for.
 *
 * \return void
 *
 * \warning The minimum delay is HW_CLK_DELAY_OVERHEAD_CYCLES in system clock ticks.
 *      The resolution is HW_CLK_CYCLES_PER_DELAY_REP in system clock ticks.
 *      The system clock tick is calculated after applying the AHB divider.
 */
__RETAINED_CODE void hw_clk_delay_usec(uint32_t usec);
#endif /* dg_configUSE_HW_CLK */
#endif /* HW_CLK_H_ */

/**
\}
\}
*/
