/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_OTPC
 * \{
 * \brief OTP Memory Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_otpc.h
 *
 * @brief Definition of API for the OTP Controller driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_OTPC_H_
#define HW_OTPC_H_


#include <sdk_defs.h>
/**
 * \brief System clock frequency in MHz
 *
 */
typedef enum {
        HW_OTPC_SYS_CLK_FREQ_1 = 0,
        HW_OTPC_SYS_CLK_FREQ_2 = 1,
        HW_OTPC_SYS_CLK_FREQ_3 = 2,
        HW_OTPC_SYS_CLK_FREQ_4 = 3,
        HW_OTPC_SYS_CLK_FREQ_6 = 4,
        HW_OTPC_SYS_CLK_FREQ_8 = 5,
        HW_OTPC_SYS_CLK_FREQ_12 = 6,
        HW_OTPC_SYS_CLK_FREQ_16 = 7,
        HW_OTPC_SYS_CLK_FREQ_24 = 8,
        HW_OTPC_SYS_CLK_FREQ_32 = 9,
        HW_OTPC_SYS_CLK_FREQ_48 = 10
} HW_OTPC_SYS_CLK_FREQ;

/**
 * \brief Convert system clock frequency expressed in MHz to equivalent HW_OTPC_SYS_CLK_FREQ value
 *
 * \sa HW_OTPC_SYS_CLK_FREQ
 */
__RETAINED_CODE HW_OTPC_SYS_CLK_FREQ hw_otpc_convert_sys_clk_mhz(uint32_t clk_freq);

#include "hw_otpc_da1469x.h"


#endif /* HW_OTPC_H_ */

/**
\}
\}
\}
*/
