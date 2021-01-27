/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_MEMCTRL Memory Controller
 * \{
 * \brief Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_memctrl.h
 *
 * @brief Definition of API for the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */
#ifndef HW_MEMCTRL_H_
#define HW_MEMCTRL_H_

#include <sdk_defs.h>

/**
 * \brief Priority values for SYSCPU, DMA and CMAC.
 */
typedef enum {
        MEMCTRL_PRIO_DEFAULT = 0,
        MEMCTRL_PRIO_HIGH = 1,
        MEMCTRL_PRIO_HIGHEST = 2
} MEMCTRL_PRIO;

/**
 * \brief Resets memory controller's configuration.
 *
 * Must be used only when both CMAC and SNC masters are disabled.
 */
void hw_memctrl_reset(void);

/**
 * \brief Configures CMAC code, data and shared regions.
 *
 * \param [in] code_base_addr   CMAC code address. CMAC 0x00000000 address is remaped to this value.
 *                              Must be multiple of 1024. The region [code_base_addr, shared_base_addr]
 *                              is not accessible by DMA.
 * \param [in] data_base_addr   CMAC data address. CMAC 0x20000000 address is remaped to this value.
 *                              Must be multiple of 4.
 * \param [in] shared_base_addr CMAC code address. Must be multiple of 1024.
 * \param [in] end_addr         The upper bound of RAM region that CMAC can access. Must end at 1024 byte boundary (10 last bits 0x3FF).
 *                              DMA can only access the RAM region between shared_base_addr and end_addr addresses.
 */
void hw_memctrl_config_cmac_region(uint32_t code_base_addr, uint32_t data_base_addr, uint32_t shared_base_addr, uint32_t end_addr);

/**
 * \brief Configures SNC base address
 *
 * \param [in] snc_base_addr SNC base address. Must be word aligned.
 */
void hw_memctrl_config_snc_region(uint32_t snc_base_addr);

/**
 * \brief Configures RAM access priority for SYSCPU, SNC and DMA.
 *
 * CMAC and MTB have always priority over SYSCPU, SNC and DMA and they cannot
 * operate on the same RAM cell (Since MTB is located at the last RAM cell
 * CMAC should not operate there).
 *
 * When SYSCPU, SNC or DMA request access on the same RAM cell (and CMAC or MTB do not),
 * the PRIO fields determine which master will gain access. For the masters that did not get priority
 * there is an internal counter (with initial value equal to the STALL cycles fields) that
 * decreases by one. When the counter reaches zero, the specific master will gain access
 * regardless of its PRIO for a single cycle and the internal counter will be reset again to the
 * initial STALL value. This is done to avoid starvation of low priority masters.
 *
 * A possible mapping of priorities to priority/stall cycle pair values could be the following:
 * - HIGHEST: PRIO 2, STALL 3
 * - HIGH: PRIO 2, STALL 6
 * - MEDIUM: PRIO 1, STALL 9
 * - LOW: PRIO 0, STALL 12
 * - LOWEST: PRIO 0, STALL 15
 *
 * Configuring two masters with the same stall cycle values should be avoided, since the field
 * was added to differentiate between masters.
 *
 * \param [in] syscpu_prio SYCPU priority
 * \param [in] syscpu_max_stall_cycles SYCPU max stall cycles (1 - 15)
 * \param [in] dma_prio DMA priority
 * \param [in] dma_max_stall_cycles DMA max stall cycles (1 - 15)
 * \param [in] snc_prio SNC priority
 * \param [in] snc_max_stall_cycles SNC max stall cycles (1 - 15)
 */
void hw_memctrl_config_master_priorities(MEMCTRL_PRIO syscpu_prio, uint8_t syscpu_max_stall_cycles,
                                                MEMCTRL_PRIO dma_prio, uint8_t dma_max_stall_cycles,
                                                MEMCTRL_PRIO snc_prio, uint8_t snc_max_stall_cycles);


#endif /* HW_MEMCTRL_H_ */

/**
\}
\}
*/
