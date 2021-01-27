/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup PD Power Domain Driver
\{
\brief Power Domain Driver
*/

/**
****************************************************************************************
*
* @file hw_pd.h
*
* @brief Power Domain Driver header file.
*
* Copyright (C) 2015-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/
#ifndef HW_PD_H_
#define HW_PD_H_


#if dg_configUSE_HW_PD

#include "sdk_defs.h"

/**
 * \enum HW_PD
 * \brief Hardware power domains.
 *
 */
typedef enum {
        HW_PD_AON = 0,      /**< Aon power domain */
        HW_PD_SYS,          /**< System power domain */
        HW_PD_COM,          /**< Communication power domain */
        HW_PD_MEM,          /**< Memory power domain */
        HW_PD_TMR,          /**< Timers power domain */
        HW_PD_PER,          /**< Peripherals power domain */
        HW_PD_RAD,          /**< Radio power domain */
        HW_PD_SYNTH,        /**< Synth power domain */
        HW_PD_MAX           /**< Power domain max*/
} HW_PD;

/**
 * \brief Power up the Peripherals Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_periph(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_UP)) == 0);
}

/**
 * \brief Power down the Peripheral Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_periph(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Peripheral Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_periph(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Peripherals Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_periph_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_UP) == 1;
}

/**
 * \brief Power up the Radio Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_UP)) == 0);
}

/**
 * \brief Power down the Radio Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_INLINE void hw_pd_power_down_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Radio Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_rad(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Radio Power Domain.
 *
 * \return 0, if it is powered down and 1 if it is powered up.
 *
 */
__STATIC_INLINE bool hw_pd_check_rad_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_UP) == 1;
}


/**
 * \brief Power up the Communications Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, COM_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, COM_IS_UP)) == 0);
}

/**
 * \brief Power down the Communications Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, COM_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Communications Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_com(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, COM_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Communications Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_com_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1;
}

/**
 * \brief Power up the Timers Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_UP)) == 0);
}

/**
 * \brief Power down the Timers Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Timers Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_tim(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Timers Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_tim_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 1;
}


#endif /* dg_configUSE_HW_PD */


#endif /* HW_PD_H_ */

/**
\}
\}
\}
*/
