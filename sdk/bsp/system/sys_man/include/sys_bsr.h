/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_BSR Busy Status Register Driver Service
 * \{
 * \brief Busy Status Register (BSR) driver
 */

/**
 ****************************************************************************************
 *
 * @file sys_bsr.h
 *
 * @brief Busy Status Register (BSR) driver file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_BSR_H_
#define SYS_BSR_H_


#include "hw_sys.h"

/**
 * \brief Initialize the BSR module.
 */
void sys_sw_bsr_init(void);

/**
 * \brief Acquire exclusive access to a specific peripheral when
 *        it is also used by other masters (SNC or CMAC). This function
 *        will block until access is granted.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be granted.
 *                       Valid range is (0 - 15). Check HW_SYS_BSR_PERIPH_ID.
 */
void sys_sw_bsr_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Checks if exclusive access to a specific peripheral has been acquired
 *        from a given master.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access will be checked.
 *                       Valid range is (0 - BSR_PERIPH_ID_MAX). Check HW_SYS_BSR_PERIPH_ID.
 * \return true if peripheral exclusive access has been acquired from the specific master, else false.
 */
bool sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Releases the exclusive access from a specific peripheral so it
 *        can be also used by other masters (SNC or CMAC).
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be released.
 *                       Valid range is (0 - 15). Check HW_SYS_BSR_PERIPH_ID.
 */
void sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);


#endif /* SYS_BSR_H_ */

/**
 \}
 \}
 */
