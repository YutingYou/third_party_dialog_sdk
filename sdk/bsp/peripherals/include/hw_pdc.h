/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_PDC PDC Driver
 * \{
 * \brief Power domains Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_pdc.h
 *
 * @brief Definition of API for the Power Domains Controller Low Level Driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_PDC_H_
#define HW_PDC_H_

#if dg_configUSE_HW_PDC

#include <sdk_defs.h>

#define HW_PDC_LUT_SIZE                 (16)
#define HW_PDC_INVALID_LUT_INDEX        (0xFFFF)
#define HW_PDC_UNUSED_LUT_ENTRY_VALUE   (0UL)
#define HW_PDC_FILTER_DONT_CARE         (0xFF)
/**
 * \brief Selects which wakeup source bank is selected as a trigger in a PDC LUT entry.
 */
typedef enum {
        HW_PDC_TRIG_SELECT_P0_GPIO      = 0, /**< Trigger from GPIO Port 0 through WAKEUP block */
        HW_PDC_TRIG_SELECT_P1_GPIO      = 1, /**< Trigger from GPIO Port 1 through WAKEUP block */
        HW_PDC_TRIG_SELECT_PERIPHERAL   = 2, /**< Trigger from peripheral IRQ, table below */
        HW_PDC_TRIG_SELECT_MASTER       = 3, /**< Trigger from master, table below */
} HW_PDC_TRIG_SELECT;

/**
 * \brief Peripheral PDC trigger IDs
*/
typedef enum {
        HW_PDC_PERIPH_TRIG_ID_TIMER             = 0x0, /**< Timer */
        HW_PDC_PERIPH_TRIG_ID_TIMER2            = 0x1, /**< Timer2 */
        HW_PDC_PERIPH_TRIG_ID_TIMER3            = 0x2, /**< Timer3 */
        HW_PDC_PERIPH_TRIG_ID_TIMER4            = 0x3, /**< Timer4 */
        HW_PDC_PERIPH_TRIG_ID_RTC_ALARM         = 0x4, /**< RTC Alarm/Rollover */
        HW_PDC_PERIPH_TRIG_ID_RTC_TIMER         = 0x5, /**< RTC Timer periodic event */
        HW_PDC_PERIPH_TRIG_ID_MAC_TIMER         = 0x6, /**< MAC Timer */
        HW_PDC_PERIPH_TRIG_ID_MOTORCTRL         = 0x7, /**< Motor Controller */
        HW_PDC_PERIPH_TRIG_ID_XTAL32MRDY        = 0x8, /**< XTAL32MRDY_IRQ */
        HW_PDC_PERIPH_TRIG_ID_RFDIAG            = 0x9, /**< RFDIAG_IRQ */
        HW_PDC_PERIPH_TRIG_ID_COMBO             = 0xA, /**< CMAC2SYS_IRQ OR VBUS Present IRQ OR JTAG present OR Debounced IO*/
        HW_PDC_PERIPH_TRIG_ID_SNC               = 0xB, /**< Sensor Node Controller IRQ */
        HW_PDC_PERIPH_TRIG_ID_MASTERONLY        = 0xF, /**< Software trigger only */
} HW_PDC_PERIPH_TRIG_ID;

/**
 * \brief PDC master IDs
 */
typedef enum {
        HW_PDC_MASTER_INVALID   = 0, /**< Invalid master. Signifies an invalid PDC LUT entry. */
        HW_PDC_MASTER_CM33      = 1, /**< ARM Cortex-M33 */
        HW_PDC_MASTER_CMAC      = 2, /**< CMAC */
        HW_PDC_MASTER_SNC       = 3, /**< Sensor Node Controller */
} HW_PDC_MASTER;

/**
 * \brief PDC LUT entry enable bits
 */
typedef enum {
        HW_PDC_LUT_ENTRY_EN_COM  = PDC_PDC_CTRL0_REG_EN_COM_Msk, /**< If set, enables PD_COM for GPIO access. This bit is implied when PDC_MASTER=SNC */
        HW_PDC_LUT_ENTRY_EN_PER  = PDC_PDC_CTRL0_REG_EN_PER_Msk, /**< If set, enables PD_PER */
        HW_PDC_LUT_ENTRY_EN_TMR  = PDC_PDC_CTRL0_REG_EN_TMR_Msk, /**< If set, enables PD_TMR */
        HW_PDC_LUT_ENTRY_EN_XTAL = PDC_PDC_CTRL0_REG_EN_XTAL_Msk, /**< If set, the XTAL32M will be started */
} HW_PDC_LUT_ENTRY_EN;

/**
 * \brief PDC LLD error codes
 */
typedef enum {
        HW_PDC_ERROR_NONE = 0,
        HW_PDC_ERROR_INVALID_LUT_ENTRY,
        HW_PDC_ERROR_INVALID_PARAM
} HW_PDC_ERROR;

/**
 * \brief PDC entry
 */
typedef struct {
        HW_PDC_TRIG_SELECT      trig_select;    /**< triggering types. */
        HW_PDC_PERIPH_TRIG_ID   trig_id;        /**< trigger id. */
        HW_PDC_MASTER           wakeup_master;  /**< wake up master id. */
        HW_PDC_LUT_ENTRY_EN     flags;          /**< PDC LUT entry enable bits .*/
} hw_pdc_entry_t;

/**
 * \brief PDC entries that will be kept after deep sleep
 */
typedef struct {
        uint8_t num;          /**< number of kept entries. */
        hw_pdc_entry_t* keep; /**< keep entry array. */
} hw_pdc_lut_keep_t;

/**
 * \brief Get the mask of a field of a PDC LUT entry.
 *
 * \param [in] field is the PDC LUT entry field to access
 *
 */
#define HW_PDC_LUT_ENTRY_FIELD_MASK(field) \
        (PDC_PDC_CTRL0_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of a PDC LUT entry.
 *
 * \param [in] field is the PDC LUT entry field to access
 *
 */
#define HW_PDC_LUT_ENTRY_FIELD_POS(field) \
        (PDC_PDC_CTRL0_REG_##field##_Pos)

/**
 * \brief Prepare (i.e. shift and mask) a value to be used for a PDC LUT entry field.
 *
 * \param [in] field is the PDC LUT entry field to access
 * \param [in] val is the value to prepare
 *
 */
#define HW_PDC_LUT_ENTRY_FIELD_VAL(field, val) \
        (((val) << HW_PDC_LUT_ENTRY_FIELD_POS(field)) & HW_PDC_LUT_ENTRY_FIELD_MASK(field))

#define HW_PDC_LUT_ENTRY_VAL(trig_select, trig_id, wakeup_master, flags)        \
                (                                                               \
                    HW_PDC_LUT_ENTRY_FIELD_VAL(TRIG_SELECT, trig_select)        \
                  | HW_PDC_LUT_ENTRY_FIELD_VAL(TRIG_ID,     trig_id)            \
                  | HW_PDC_LUT_ENTRY_FIELD_VAL(PDC_MASTER,  wakeup_master)      \
                  | flags                                                       \
                )

/*
 * Shorthand macros
 */
#define HW_PDC_TRIGGER_FROM_PORT0(pin, wakeup_master, flags) \
        HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_P0_GPIO, pin, wakeup_master, flags)

#define HW_PDC_TRIGGER_FROM_PORT1(pin, wakeup_master, flags) \
        HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_P1_GPIO, pin, wakeup_master, flags)

#define HW_PDC_TRIGGER_FROM_PERIPH(peripheral, wakeup_master, flags) \
        HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL, peripheral, wakeup_master, flags)

#define HW_PDC_TRIGGER_FROM_MASTER(wakeup_master, flags) \
        HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_MASTER, HW_PDC_PERIPH_TRIG_ID_MASTERONLY, \
                                wakeup_master, flags)

/**
 * \brief Read value from specific PDC LUT index
 *
 * \param [in] idx      LUT index to read from. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 *
 * \return value at given PDC LUT index
 */
__STATIC_INLINE uint32_t hw_pdc_read_entry(uint32_t idx)
{
        ASSERT_ERROR(idx < HW_PDC_LUT_SIZE);

        return *(&PDC->PDC_CTRL0_REG + idx);
}

/**
 * \brief Add a PDC LUT entry dynamically
 *
 * Scans all LUT entries until it finds an unused one. A LUT entry shall be considered unused if it equals zero.
 *
 * \param [in]  lut_entry value for the LUT entry
 *
 * \return      LUT index of the new entry if an unused entry was found
 *              HW_PDC_INVALID_LUT_INDEX otherwise.
 */
__RETAINED_CODE uint32_t hw_pdc_add_entry(uint32_t lut_entry);

/**
 * \brief Remove a dynamically added PDC LUT entry
 *
 * Zero shall be written in the LUT entry at the given index.
 *
 * \param [in] idx      the index of the LUT entry to remove. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 *
 * \return      the old LUT entry value
 */
uint32_t hw_pdc_remove_entry(uint32_t idx);

/**
 * \brief Get all PDC LUT entries pending for any master
 */
__STATIC_INLINE uint32_t hw_pdc_get_pending(void)
{
        return PDC->PDC_PENDING_REG;
}

/**
 * \brief Get all PDC LUT entries pending for CM33
 */
__STATIC_INLINE uint32_t hw_pdc_get_pending_cm33(void)
{
        return PDC->PDC_PENDING_CM33_REG;
}

/**
 * \brief Get all PDC LUT entries pending for CMAC
 */
__STATIC_INLINE uint32_t hw_pdc_get_pending_cmac(void)
{
        return PDC->PDC_PENDING_CMAC_REG;
}

/**
 * \brief Get all PDC LUT entries pending for Sensor Node Controller
 */
__STATIC_INLINE uint32_t hw_pdc_get_pending_snc(void)
{
        return PDC->PDC_PENDING_SNC_REG;
}

/**
 * \brief Acknowledge a PDC LUT entry
 *
 * \param [in] idx      the index of the LUT entry to acknowledge. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 */
__STATIC_INLINE void hw_pdc_acknowledge(uint32_t idx)
{
        ASSERT_ERROR(idx < HW_PDC_LUT_SIZE);

        PDC->PDC_ACKNOWLEDGE_REG= idx;
}

/**
 * \brief Write a value in specific PDC LUT index
 *
 * \param [in] idx      LUT index to write at. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 * \param [in] value    value to be written
 *
 */
void hw_pdc_write_entry(uint32_t idx, uint32_t value);

/**
 * \brief Set a PDC LUT entry as pending
 *
 * \param [in] idx      the index of the PDC LUT entry. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 *
 * \return     HW_PDC_ERROR_NONE if no error occurred, else error code.
 *
 * \sa HW_PDC_ERROR
 */
__STATIC_INLINE HW_PDC_ERROR hw_pdc_set_pending(uint32_t idx)
{
        if (idx >= HW_PDC_LUT_SIZE) {
                return HW_PDC_ERROR_INVALID_PARAM;
        }

        if ((hw_pdc_read_entry(idx) & HW_PDC_LUT_ENTRY_FIELD_MASK(PDC_MASTER)) == 0) {
                return HW_PDC_ERROR_INVALID_LUT_ENTRY;
        }

        PDC->PDC_SET_PENDING_REG = idx;

        return HW_PDC_ERROR_NONE;
}

/**
 * \brief Check if a PDC LUT entry is pending
 *
 * \param [in] idx      the index of the PDC LUT entry. Valid range: Range 0 - (HW_PDC_LUT_SIZE-1)
 */
__STATIC_FORCEINLINE bool hw_pdc_is_pending(uint32_t idx)
{
        ASSERT_ERROR(idx < HW_PDC_LUT_SIZE);

        return !!(PDC->PDC_PENDING_REG & (1 << idx));
}

/**
 * \brief Acknowledge all PDC LUT entries pending for CM33
 */
void hw_pdc_ack_all_pending_cm33(void);

/**
 * \brief Reset PDC Lookup table
 *
 * Invalidates all PDC lookup table entries
 */
void hw_pdc_lut_reset(void);

/**
 * \brief Keep only the selected PDC Lookup table entries
 *
 * Invalidates all PDC lookup table entries except those set to be kept
 *
 *\param [in] keep      pointer to an array with the LUT entries that will be kept
 *
 */
void hw_pdc_lut_keep(hw_pdc_lut_keep_t* keep);

/**
 * \brief Get the first PDC LUT entry index matching specific criteria
 *
 *\param [in] trig_select       LUT entry triggering type GPIO/peripheral
 *\param [in] trig_id           LUT entry Pin_ID/Periph_ID
 *\param [in] wakeup_master     LUT entry wake up Master_ID
 *\param [in] flags             LUT entry action(s)
 *\param [in] start             Starting PDC entry search point
 *
 *\return                       LUT index of the entry matching to above criteria,
 *                              HW_PDC_INVALID_LUT_INDEX otherwise
 *
 */
uint32_t hw_pdc_find_entry(uint32_t trig_select, uint32_t trig_id,
                             uint32_t wakeup_master, uint32_t flags, uint32_t start);

#endif /* dg_configUSE_HW_PDC */
#endif /* HW_PDC_H_ */

/**
 * \}
 * \}
 * \}
 */
