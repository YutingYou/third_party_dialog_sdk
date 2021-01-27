/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup PDC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_pdc.c
 *
 * @brief Implementation of the Power Domains Controller Low Level Driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_PDC


#include <hw_pdc.h>

__RETAINED_CODE uint32_t hw_pdc_add_entry(uint32_t lut_entry)
{
        uint32_t idx = HW_PDC_INVALID_LUT_INDEX;

        for (int i = 0; i < HW_PDC_LUT_SIZE; ++i) {
                if (*(&PDC->PDC_CTRL0_REG + i) == HW_PDC_UNUSED_LUT_ENTRY_VALUE) {
                        idx = i;
                        *(&PDC->PDC_CTRL0_REG + i) = lut_entry;
                        break;
                }
        }

        return idx;
}

void hw_pdc_write_entry(uint32_t idx, uint32_t value)
{
        ASSERT_ERROR(idx < HW_PDC_LUT_SIZE);

        // In case of invalid value check if LUT idx is pending. If it is, acknowledge it.
        if ((value & HW_PDC_LUT_ENTRY_FIELD_MASK(PDC_MASTER)) == 0 &&
            ((hw_pdc_get_pending() & (1 << idx)) != 0)) {
                hw_pdc_acknowledge(idx);
                // Check if it is the only pending idx. If it is clean pending PDC IRQ.
                if (!hw_pdc_get_pending() && NVIC_GetPendingIRQ(PDC_IRQn)) {
                        NVIC_ClearPendingIRQ(PDC_IRQn);
                }
        }

        *(&PDC->PDC_CTRL0_REG + idx) = value;
}

uint32_t hw_pdc_remove_entry(uint32_t idx)
{
        uint32_t old_value = hw_pdc_read_entry(idx);

        hw_pdc_write_entry(idx, HW_PDC_UNUSED_LUT_ENTRY_VALUE);

        return old_value;
}

void hw_pdc_ack_all_pending_cm33(void)
{
        uint32_t pending = hw_pdc_get_pending_cm33();

        for (int i = 0; i < HW_PDC_LUT_SIZE; ++i) {
                if (pending & (1 << i) ) {
                        hw_pdc_acknowledge(i);
                }
        }
}

void hw_pdc_lut_reset(void)
{
        for (int i = 0; i < HW_PDC_LUT_SIZE; ++i) {
                *(&PDC->PDC_CTRL0_REG + i) = HW_PDC_UNUSED_LUT_ENTRY_VALUE;
                hw_pdc_acknowledge(i);
        }
}

static bool hw_pdc_entry_matches(uint32_t trig_select, uint32_t trig_id,
                           uint32_t wakeup_master, uint32_t flags, uint8_t entry)
{
        uint32_t mask = 0;
        uint32_t pattern = 0;

        mask |= (trig_select == HW_PDC_FILTER_DONT_CARE) ? mask : HW_PDC_LUT_ENTRY_FIELD_MASK(TRIG_SELECT);
        mask |= (trig_id == HW_PDC_FILTER_DONT_CARE) ? mask: HW_PDC_LUT_ENTRY_FIELD_MASK(TRIG_ID);
        mask |= (wakeup_master == HW_PDC_FILTER_DONT_CARE) ? mask : HW_PDC_LUT_ENTRY_FIELD_MASK( PDC_MASTER);
        mask |= ((flags == HW_PDC_FILTER_DONT_CARE) ? mask : flags);

        pattern = HW_PDC_LUT_ENTRY_VAL(trig_select, trig_id, wakeup_master, ((flags == HW_PDC_FILTER_DONT_CARE) ? 0 : flags));

        if ((hw_pdc_read_entry(entry) & mask) == (pattern & mask)) {
                return true;
        }

        return false;
}

uint32_t hw_pdc_find_entry(uint32_t trig_select, uint32_t trig_id,
                           uint32_t wakeup_master, uint32_t flags, uint32_t start)
{
        for (; start < HW_PDC_LUT_SIZE; start++) {
                if (hw_pdc_entry_matches(trig_select, trig_id, wakeup_master, flags, start)) {
                        return start;
                }
        }

        return HW_PDC_INVALID_LUT_INDEX;
}

void hw_pdc_lut_keep(hw_pdc_lut_keep_t* keep)
{
        uint8_t i = 0;
        uint8_t j = 0;
        bool match = false;

        if (keep == NULL) {
                /* keep list is empty */
                hw_pdc_lut_reset();
                return;
        }

        for (i = 0; i < HW_PDC_LUT_SIZE; i++ ) {
                for (j = 0; j < keep->num; j++) {
                        if (hw_pdc_entry_matches(keep->keep[j].trig_select, keep->keep[j].trig_id,
                                keep->keep[j].wakeup_master, keep->keep[j].flags, i)) {
                                match = true;
                                break;
                        }
                }
                /* remove unmatched entry*/
                if (match) {
                        match = false;
                } else {
                        hw_pdc_remove_entry(i);
                }
        }
}

#endif /* dg_configUSE_HW_PDC */
/**
 * \}
 * \}
 * \}
 */
