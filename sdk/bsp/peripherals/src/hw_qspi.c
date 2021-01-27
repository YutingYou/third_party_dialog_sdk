/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup QSPI
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_qspi.c
 *
 * @brief Implementation of the QSPI Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if (dg_configUSE_HW_QSPI) || (dg_configUSE_HW_QSPI2)

#include <stdint.h>
#include "hw_qspi.h"

/*
 * Return 1 if register reg is at the same offset in
 * qspi_regs, QSPIC_Type and QSPIC2_Type, 0 if not
 */
        #define HW_QSPIC_OFFSET(reg) \
                ((offsetof(struct qspi_regs, QSPIC_##reg##_REG) == \
                  offsetof(QSPIC_Type, QSPIC_##reg##_REG)) && \
                 (offsetof(struct qspi_regs, QSPIC_##reg##_REG) == \
                  offsetof(QSPIC2_Type, QSPIC2_##reg##_REG)))

_Static_assert(HW_QSPIC_OFFSET(CTRLBUS) == 1, "Wrong offset for QSPIC register CTRLBUS");
_Static_assert(HW_QSPIC_OFFSET(CTRLMODE) == 1, "Wrong offset for QSPIC register CTRLMODE");
_Static_assert(HW_QSPIC_OFFSET(RECVDATA) == 1, "Wrong offset for QSPIC register RECVDATA");
_Static_assert(HW_QSPIC_OFFSET(BURSTCMDA) == 1, "Wrong offset for QSPIC register BURSTCMDA");
_Static_assert(HW_QSPIC_OFFSET(BURSTCMDB) == 1, "Wrong offset for QSPIC register BURSTCMDB");
_Static_assert(HW_QSPIC_OFFSET(STATUS) == 1, "Wrong offset for QSPIC register STATUS");
_Static_assert(HW_QSPIC_OFFSET(WRITEDATA) == 1, "Wrong offset for QSPIC register WRITEDATA");
_Static_assert(HW_QSPIC_OFFSET(READDATA) == 1, "Wrong offset for QSPIC register READDATA");
_Static_assert(HW_QSPIC_OFFSET(DUMMYDATA) == 1, "Wrong offset for QSPIC register DUMMYDATA");
_Static_assert(HW_QSPIC_OFFSET(ERASECTRL) == 1, "Wrong offset for QSPIC register ERASECTRL");
_Static_assert(HW_QSPIC_OFFSET(ERASECMDA) == 1, "Wrong offset for QSPIC register ERASECMDA");
_Static_assert(HW_QSPIC_OFFSET(ERASECMDB) == 1, "Wrong offset for QSPIC register ERASECMDB");
_Static_assert(HW_QSPIC_OFFSET(BURSTBRK) == 1, "Wrong offset for QSPIC register BURSTBRK");
_Static_assert(HW_QSPIC_OFFSET(STATUSCMD) == 1, "Wrong offset for QSPIC register STATUSCMD");
_Static_assert(HW_QSPIC_OFFSET(CHCKERASE) == 1, "Wrong offset for QSPIC register CHCKERASE");
_Static_assert(HW_QSPIC_OFFSET(GP) == 1, "Wrong offset for QSPIC register GP");

__RETAINED_CODE void hw_qspi_init(HW_QSPIC_ID id, const qspi_config *cfg)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        ASSERT_WARNING(REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN) == 0 ||
                       REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) == 1)
#endif

        hw_qspi_clock_enable(id);
        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_MANUAL);
        hw_qspi_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        hw_qspi_set_pad_direction(id, HW_QSPI_IO2, HW_QSPI_OUTPUT);
        hw_qspi_set_pad_value(id, HW_QSPI_IO2, true);
        hw_qspi_set_pad_direction(id, HW_QSPI_IO3, HW_QSPI_OUTPUT);
        hw_qspi_set_pad_value(id, HW_QSPI_IO3, true);

        if (cfg) {
                hw_qspi_set_address_size(id, cfg->address_size);
                hw_qspi_set_clock_mode(id, cfg->idle_clock);
                hw_qspi_set_read_sampling_edge(id, cfg->sampling_edge);
        }
}


__RETAINED_CODE void hw_qspi_set_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE mode)
{
        switch (mode) {
        case HW_QSPI_BUS_MODE_SINGLE:
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_SINGLE);
                break;
        case HW_QSPI_BUS_MODE_DUAL:
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_DUAL);
                break;
        case HW_QSPI_BUS_MODE_QUAD:
        case HW_QSPI_BUS_MODE_QPI:
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_QUAD);
                hw_qspi_set_pad_direction(id, HW_QSPI_IO2, HW_QSPI_INPUT);
                hw_qspi_set_pad_direction(id, HW_QSPI_IO3, HW_QSPI_INPUT);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

__RETAINED_CODE void hw_qspi_set_access_mode(HW_QSPIC_ID id, HW_QSPI_ACCESS_MODE mode)
{
        switch (mode) {
        case HW_QSPI_ACCESS_MODE_AUTO:
                {
                const uint32_t burst_cmd_a = id->QSPIC_BURSTCMDA_REG,
                               burst_cmd_b = id->QSPIC_BURSTCMDB_REG,
                               status_cmd = id->QSPIC_STATUSCMD_REG,
                               erase_cmd_b = id->QSPIC_ERASECMDB_REG,
                               burstbrk = id->QSPIC_BURSTBRK_REG;
                if (GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_INST_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_ADR_TX_MD ) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_DMY_TX_MD ) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_BURSTCMDA_REG, burst_cmd_a, QSPIC_EXT_TX_MD ) == HW_QSPI_BUS_MODE_QUAD ||

                    GETBITS32(QSPIC, QSPIC_BURSTCMDB_REG, burst_cmd_b, QSPIC_DAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_BURSTCMDB_REG, burst_cmd_b, QSPIC_DAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||

                    GETBITS32(QSPIC, QSPIC_STATUSCMD_REG, status_cmd, QSPIC_RSTAT_RX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_STATUSCMD_REG, status_cmd, QSPIC_RSTAT_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||

                    GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_ERS_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_WEN_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_SUS_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_RES_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||
                    GETBITS32(QSPIC, QSPIC_ERASECMDB_REG, erase_cmd_b, QSPIC_EAD_TX_MD) == HW_QSPI_BUS_MODE_QUAD ||

                    GETBITS32(QSPIC, QSPIC_BURSTBRK_REG, burstbrk, QSPIC_BRK_TX_MD) == HW_QSPI_BUS_MODE_QUAD) {
                        hw_qspi_set_pad_direction(id, HW_QSPI_IO2, HW_QSPI_INPUT);
                        hw_qspi_set_pad_direction(id, HW_QSPI_IO3, HW_QSPI_INPUT);
                }
                }
                HW_QSPIC_REG_SET_BIT(id, CTRLMODE, AUTO_MD);
                break;
        case HW_QSPI_ACCESS_MODE_MANUAL:
                HW_QSPIC_REG_CLR_BIT(id, CTRLMODE, AUTO_MD);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

__RETAINED_CODE void hw_qspi_erase_block(HW_QSPIC_ID id, uint32_t addr)
{
        if (hw_qspi_get_access_mode(id) != HW_QSPI_ACCESS_MODE_AUTO) {
                hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_AUTO);
        }

        // Wait for previous erase to end
        while (hw_qspi_get_erase_status(id) != HW_QSPI_ERS_NO) {
        }

        uint32_t block_sector = addr >> 12;
        switch (hw_qspi_get_address_size(id)) {
        case HW_QSPI_ADDR_SIZE_24:
                ASSERT_WARNING(addr <= 0x00FFFFFF);
                // QSPIC_ERASECTRL_REG/QSPIC2_ERASECTRL_REG bits 23-12 determine the block/sector address bits (23-12)
                block_sector <<= 8;
                break;
        case HW_QSPI_ADDR_SIZE_32:
                ASSERT_WARNING(addr <= 0x01FFFFFF);
                // QSPIC_ERASECTRL_REG/QSPIC2_ERASECTRL_REG bits 23-4 determine the block/sector address bits (31-12)
                break;
        default:
                ASSERT_WARNING(0);
        }
        // Setup erase block page
        HW_QSPIC_REG_SETF(id, ERASECTRL, ERS_ADDR, block_sector);
        // Fire erase
        HW_QSPIC_REG_SET_BIT(id, ERASECTRL, ERASE_EN);
}


static uint32_t hw_qspi_buf_to_word(const uint8 *data)
{
        ASSERT_WARNING(data);
        if ((uint32_t)data & 0x3) {
                return SWAP32(__UNALIGNED_UINT32_READ(data));
        } else {
                return SWAP32(*(uint32_t *)data);
        }
}

__RETAINED_CODE void hw_qspi_write_nonce(HW_QSPIC_ID id, const uint8_t *nonce)
{
        if (id == HW_QSPIC) {
                ASSERT_WARNING(nonce);
                QSPIC->QSPIC_CTR_NONCE_0_3_REG = hw_qspi_buf_to_word(nonce);
                QSPIC->QSPIC_CTR_NONCE_4_7_REG = hw_qspi_buf_to_word(nonce + 4);
        } else {
                ASSERT_WARNING(0);
        }
}

__RETAINED_CODE void hw_qspi_write_key(HW_QSPIC_ID id, const uint8_t *key)
{
        if (id == HW_QSPIC) {
                ASSERT_WARNING(key);
                QSPIC->QSPIC_CTR_KEY_0_3_REG = hw_qspi_buf_to_word(key);
                QSPIC->QSPIC_CTR_KEY_4_7_REG = hw_qspi_buf_to_word(key + 4);
                QSPIC->QSPIC_CTR_KEY_8_11_REG = hw_qspi_buf_to_word(key + 8);
                QSPIC->QSPIC_CTR_KEY_12_15_REG = hw_qspi_buf_to_word(key + 12);
                QSPIC->QSPIC_CTR_KEY_16_19_REG = hw_qspi_buf_to_word(key + 16);
                QSPIC->QSPIC_CTR_KEY_20_23_REG = hw_qspi_buf_to_word(key + 20);
                QSPIC->QSPIC_CTR_KEY_24_27_REG = hw_qspi_buf_to_word(key + 24);
                QSPIC->QSPIC_CTR_KEY_28_31_REG = hw_qspi_buf_to_word(key + 28);
        } else {
                ASSERT_WARNING(0);
        }
}


__STATIC_FORCEINLINE void hw_qspi_set_read_instruction(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        id->QSPIC_BURSTCMDA_REG =
                BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_INST, qspic_set->read_instruction.inst) |
                BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_INST_TX_MD, qspic_set->read_instruction.inst_phase) |
                BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_ADR_TX_MD, qspic_set->read_instruction.addr_phase) |
                BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_DMY_TX_MD, qspic_set->read_instruction.dummy_phase);

        id->QSPIC_BURSTCMDB_REG =
                BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DAT_RX_MD, qspic_set->read_instruction.data_phase) |
                BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_INST_MD, qspic_set->read_instruction.inst_mode);

        hw_qspi_set_dummy_bytes_count(id, qspic_set->read_instruction.dummy_count);
}

__STATIC_FORCEINLINE void hw_qspi_set_read_status_instruction(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        ASSERT_WARNING(qspic_set->read_status_instruction.busy_pos < 8);
        ASSERT_WARNING(qspic_set->read_status_instruction.read_delay < 64);
        id->QSPIC_STATUSCMD_REG =
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_BUSY_VAL, qspic_set->read_status_instruction.busy_val) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_BUSY_POS , qspic_set->read_status_instruction.busy_pos) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_RX_MD, qspic_set->read_status_instruction.receive_phase) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_TX_MD, qspic_set->read_status_instruction.inst_phase) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_INST, qspic_set->read_status_instruction.inst) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_STSDLY_SEL, qspic_set->read_status_instruction.delay_sel) |
                BITS32(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RESSTS_DLY, qspic_set->read_status_instruction.read_delay);
}

__STATIC_FORCEINLINE void hw_qspi_set_erase_instruction(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        ASSERT_WARNING(qspic_set->erase_instruction.hclk_cycles < 16);
        ASSERT_WARNING(qspic_set->erase_instruction.cs_hi_cycles < 32);

        HW_QSPIC_REG_SETF(id, ERASECMDA, ERS_INST, qspic_set->erase_instruction.inst);
        id->QSPIC_ERASECMDB_REG = (id->QSPIC_ERASECMDB_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_TX_MD) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_EAD_TX_MD) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERSRES_HLD) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_CS_HI))) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_TX_MD, qspic_set->erase_instruction.inst_phase) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_EAD_TX_MD, qspic_set->erase_instruction.addr_phase) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERSRES_HLD, qspic_set->erase_instruction.hclk_cycles) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_CS_HI, qspic_set->erase_instruction.cs_hi_cycles);
}

__STATIC_FORCEINLINE void hw_qspi_set_write_enable_instruction(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        HW_QSPIC_REG_SETF(id, ERASECMDA, WEN_INST, qspic_set->write_enable_instruction.inst);
        HW_QSPIC_REG_SETF(id, ERASECMDB, WEN_TX_MD, qspic_set->write_enable_instruction.inst_phase);
}

__STATIC_FORCEINLINE void hw_qspi_set_wrapping_burst_instruction(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        HW_QSPIC_REG_SETF(id, BURSTCMDA, INST_WB, qspic_set->wrapping_burst_instruction.inst);
        id->QSPIC_BURSTCMDB_REG = (id->QSPIC_BURSTCMDB_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_SIZE) |
                                     REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_LEN))) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_SIZE, qspic_set->wrapping_burst_instruction.size) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_LEN, qspic_set->wrapping_burst_instruction.len) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_WRAP_MD, true);
}

__STATIC_FORCEINLINE void hw_qspi_set_suspend_resume_instructions(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        ASSERT_WARNING(qspic_set->suspend_resume_instruction.minimum_delay < 64);

        id->QSPIC_ERASECMDA_REG = (id->QSPIC_ERASECMDA_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_ERASECMDA_REG, QSPIC_SUS_INST) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDA_REG, QSPIC_RES_INST))) |
                                  BITS32(QSPIC, QSPIC_ERASECMDA_REG, QSPIC_SUS_INST, qspic_set->suspend_resume_instruction.erase_suspend_inst) |
                                  BITS32(QSPIC, QSPIC_ERASECMDA_REG, QSPIC_RES_INST, qspic_set->suspend_resume_instruction.erase_resume_inst);
        id->QSPIC_ERASECMDB_REG = (id->QSPIC_ERASECMDB_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_SUS_TX_MD) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RES_TX_MD) |
                                     REG_MSK(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RESSUS_DLY))) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_SUS_TX_MD, qspic_set->suspend_resume_instruction.suspend_inst_phase) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RES_TX_MD, qspic_set->suspend_resume_instruction.resume_inst_phase) |
                                  BITS32(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RESSUS_DLY, qspic_set->suspend_resume_instruction.minimum_delay);
}
#if dg_configUSE_HW_QSPI2
__STATIC_FORCEINLINE void hw_qspi_set_write_instruction(struct qspic_instructions *qspic_set)
{
        QSPIC2->QSPIC2_AWRITECMD_REG =
                        BITS32(QSPIC2, QSPIC2_AWRITECMD_REG, QSPIC_WR_INST, qspic_set->write_instruction.inst) |
                        BITS32(QSPIC2, QSPIC2_AWRITECMD_REG, QSPIC_WR_INST_TX_MD, qspic_set->write_instruction.inst_phase) |
                        BITS32(QSPIC2, QSPIC2_AWRITECMD_REG, QSPIC_WR_ADR_TX_MD, qspic_set->write_instruction.addr_phase) |
                        BITS32(QSPIC2, QSPIC2_AWRITECMD_REG, QSPIC_WR_DAT_TX_MD, qspic_set->write_instruction.data_phase);
}
#endif
__RETAINED_CODE void hw_qspi_set_instructions(HW_QSPIC_ID id, struct qspic_instructions *qspic_set)
{
        if (qspic_set->set_read_status_instruction) {
                hw_qspi_set_read_status_instruction(id, qspic_set);
        }
        if (qspic_set->set_read_instruction) {
                hw_qspi_set_read_instruction(id, qspic_set);
        }
        if (qspic_set->set_erase_instruction) {
                hw_qspi_set_erase_instruction(id, qspic_set);
        }
        if (qspic_set->set_write_enable_instruction) {
                hw_qspi_set_write_enable_instruction(id, qspic_set);
        }
        if (qspic_set->set_wrapping_burst_instruction) {
                hw_qspi_set_wrapping_burst_instruction(id, qspic_set);
        }
        if (qspic_set->set_suspend_resume_instruction) {
                hw_qspi_set_suspend_resume_instructions(id, qspic_set);
        }
#if dg_configUSE_HW_QSPI2
        if (id == HW_QSPIC2) {
                if (qspic_set->set_write_instruction) {
                        hw_qspi_set_write_instruction(qspic_set);
                }
        }
#endif
}

#endif /* dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2 */
/**
 * \}
 * \}
 * \}
 */
