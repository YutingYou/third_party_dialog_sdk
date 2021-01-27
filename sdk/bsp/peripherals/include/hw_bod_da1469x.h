/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_BOD BOD driver
\{
\brief DA1469x BOD LLD
*/

/**
****************************************************************************************
*
* @file hw_bod_da1469x.h
*
* @brief BOD LLD header file for DA1469x.
*
* Copyright (C) 2015-2020 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_BOD_DA1469x_H_
#define HW_BOD_DA1469x_H_


#include "sdk_defs.h"

#if dg_configUSE_BOD

/**
 * \brief The BOD channel name
 */
typedef enum {
        BOD_CHANNEL_VBAT      = 3,
        BOD_CHANNEL_3V0       = 4,
        BOD_CHANNEL_1V8       = 5,
        BOD_CHANNEL_1V8P      = 6,
        BOD_CHANNEL_VDD       = 7,
        BOD_CHANNEL_1V8F      = 8,
        BOD_CHANNEL_1V4       = 9,
        BOD_CHANNEL_VDD_SLEEP = 10,
} bod_channel_t;

/**
 * \brief Activate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_activate_channel(bod_channel_t channel)
{
        // BOD_CHANNEL_VDD_SLEEP is not used for activating VDD channel.
        // Use BOD_CHANNEL_VDD instead
        ASSERT_WARNING(channel >= BOD_CHANNEL_VBAT && channel < BOD_CHANNEL_VDD_SLEEP);
        CRG_TOP->BOD_CTRL_REG |= (1 << channel);
}

/**
 * \brief Deactivate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_deactivate_channel(bod_channel_t channel)
{
        // BOD_CHANNEL_VDD_SLEEP is not used for deactivating VDD channel.
        // Use BOD_CHANNEL_VDD instead
        ASSERT_WARNING(channel >= BOD_CHANNEL_VBAT && channel < BOD_CHANNEL_VDD_SLEEP);
        CRG_TOP->BOD_CTRL_REG &= ~(1 << channel);
}

/**
 * \brief Set BOD channel voltage level.
 *
 * Valid voltage level depends on the BOD channel:
 *     BOD_CHANNEL_VBAT: 10..4800mV
 *     BOD_CHANNEL_V30:  7..3200mV
 *     BOD_CHANNEL_V18:  7..3200mV
 *     BOD_CHANNEL_V18P: 7..3200mV
 *     BOD_CHANNEL_V18F: 7..3200mV
 *     BOD_CHANNEL_V14:  7..3200mV
 *     BOD_CHANNEL_VDD:  7..1600mV
 *     BOD_CHANNEL_VDD_SLEEP: 7..1600mV
 *
 * These levels cannot necessarily be achieved using the internal LDOs
 *
 * \param[in] channel BOD channel
 * \param[in] voltage level in mV
 *
 */
__STATIC_INLINE void hw_bod_set_channel_voltage_level(bod_channel_t channel, uint16_t voltage)
{
        uint32_t level;

        switch (channel) {
        case BOD_CHANNEL_VBAT:
                level = voltage*192/1800 - 1;
                break;
        default:
                level = voltage*192/1200 - 1;
                break;
        }

        switch (channel) {
        case BOD_CHANNEL_VBAT: // Max 4.8V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_VBAT_Msk >> CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_VBAT_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_VBAT, level);
                break;
        case BOD_CHANNEL_3V0: // Max 3.2V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_V30_Msk >> CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_V30_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_V30, level);
                break;
        case BOD_CHANNEL_1V8: // Max 3.2V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_V18_Msk >> CRG_TOP_BOD_LVL_CTRL0_REG_BOD_LVL_V18_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_V18, level);
                break;
        case BOD_CHANNEL_1V8P: // Max 3.2V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_V18P_Msk >> CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_V18P_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_V18P, level);
                break;
        case BOD_CHANNEL_VDD: // Max 1.6V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_VDD_ON_Msk >> CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_VDD_ON_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_VDD_ON, level);
                break;
        case BOD_CHANNEL_VDD_SLEEP: // Max 1.6V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_VDD_RET_Msk >> CRG_TOP_BOD_LVL_CTRL1_REG_BOD_LVL_VDD_RET_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_VDD_RET, level);
                break;
        case BOD_CHANNEL_1V8F: // Max 3.2V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL2_REG_BOD_LVL_V18F_Msk >> CRG_TOP_BOD_LVL_CTRL2_REG_BOD_LVL_V18F_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL2_REG, BOD_LVL_V18F, level);
                break;
        case BOD_CHANNEL_1V4: // Max 3.2V
                ASSERT_WARNING(level <= CRG_TOP_BOD_LVL_CTRL2_REG_BOD_LVL_V14_Msk >> CRG_TOP_BOD_LVL_CTRL2_REG_BOD_LVL_V14_Pos);
                REG_SETF(CRG_TOP, BOD_LVL_CTRL2_REG, BOD_LVL_V14, level);
                break;
        }
}

/**
 * \brief Get BOD channel voltage level.
 *
 * \param[in] channel BOD channel
 *
 * \return voltage level in mV
 *
 */
__STATIC_INLINE uint16_t hw_bod_get_channel_voltage_level(bod_channel_t channel)
{
        uint32_t level;

        switch (channel) {
        case BOD_CHANNEL_VBAT:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_VBAT);
                break;
        case BOD_CHANNEL_3V0:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_V30);
                break;
        case BOD_CHANNEL_1V8:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL0_REG, BOD_LVL_V18);
                break;
        case BOD_CHANNEL_1V8P:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_V18P);
                break;
        case BOD_CHANNEL_VDD:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_VDD_ON);
                break;
        case BOD_CHANNEL_VDD_SLEEP:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL1_REG, BOD_LVL_VDD_RET);
                break;
        case BOD_CHANNEL_1V8F:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL2_REG, BOD_LVL_V18F);
                break;
        case BOD_CHANNEL_1V4:
                level = REG_GETF(CRG_TOP, BOD_LVL_CTRL2_REG, BOD_LVL_V14);
                break;
        }

        switch (channel) {
        case BOD_CHANNEL_VBAT:
                return (level+1)*1800/192;
        default:
                return (level+1)*1200/192;
        }
}

/**
 * \brief Configure BOD.
 *
 */
void hw_bod_configure(void);

/**
 * \brief Activate BOD after wake-up.
 *
 * \deprecated API no longer supported, use hw_bod_configure() instead.
 *
 */
DEPRECATED_MSG("API no longer supported, use hw_bod_configure() instead.")
__STATIC_INLINE void hw_bod_activate_on_wakeup(void)
{
        hw_bod_configure();
}

/**
 * \brief Activate BOD  before sleep.
 *
 */
__RETAINED_CODE void hw_bod_activate_before_sleep(void);

#endif /* dg_configUSE_BOD */

/**
 * \brief Deactivate BOD.
 *
 */
__STATIC_FORCEINLINE void hw_bod_deactivate(void)
{
        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VDD_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V30_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_EN);
        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, 0);
}


#endif /* HW_BOD_DA1469x_H_ */

/**
\}
\}
*/
