/**
\addtogroup BSP
\{
\addtogroup SYSTEM
\{
\addtogroup TCS_Handling
\{
*/

/**
****************************************************************************************
*
* @file sys_tcs.c
*
* @brief TCS Handler
*
* Copyright (C) 2015-2020 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#include "sys_tcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if (dg_configUSE_HW_GPADC == 1)
#include "hw_gpadc.h"
#endif

#define CS_START_CMD    0xA5A5A5A5
#define CS_SDK_VAL      0x90000000
#define CS_STOP_CMD     0x00000000
#define CS_EMPTY_VAL    0xFFFFFFFF
#define OTP_CS_ADDRESS  0x00000C00

#define EMPTY 0xFF

/* Static allocation for tcs_data */
__RETAINED static uint32_t __tcs_data[CS_MAX_SIZE / 4 + 1];
__RETAINED static uint32_t* tcs_data;

__RETAINED static uint8_t pwmled_tcs_entries;

__RETAINED_RW static sys_tcs_attr_t tcs_attributes[SYS_TCS_GROUP_MAX]= {
        {SYS_TCS_TYPE_RESERVED, SYS_TCS_DOMAIN_NA, EMPTY, 0},           /* GID 0  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_SYS, EMPTY, 0},       /* GID 1  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_COM, EMPTY, 0},       /* GID 2  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_MEM, EMPTY, 0},       /* GID 3  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_TMR, EMPTY, 0},       /* GID 4  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_PER, EMPTY, 0},       /* GID 5  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_RAD, EMPTY, 0},       /* GID 6  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_SYNTH, EMPTY, 0},     /* GID 7  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_RAD, EMPTY, 0},       /* GID 8  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_SYNTH, EMPTY, 0},     /* GID 9  */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_RAD, EMPTY, 0},       /* GID 10 */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_SYNTH, EMPTY, 0},     /* GID 11 */
        {SYS_TCS_TYPE_TRIM_VAL, SYS_TCS_DOMAIN_NA, EMPTY, 3},           /* GID 12 */
        {SYS_TCS_TYPE_TRIM_VAL, SYS_TCS_DOMAIN_NA, EMPTY, 1},           /* GID 13 */
        {SYS_TCS_TYPE_TRIM_VAL, SYS_TCS_DOMAIN_NA, EMPTY, 2},           /* GID 14 */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_COM, EMPTY, 0},       /* GID 15 */
        {SYS_TCS_TYPE_REG_PAIR, SYS_TCS_DOMAIN_PD_COM, EMPTY, 0},       /* GID 16 */
        {SYS_TCS_TYPE_TRIM_VAL, SYS_TCS_DOMAIN_PD_PER, EMPTY, 1},       /* GID 17 */
        {SYS_TCS_TYPE_TRIM_VAL, SYS_TCS_DOMAIN_PD_PER, EMPTY, 1}};      /* GID 18 */


typedef enum {
        TCS_OTP = 0,                    /* CS in OTP */
        TCS_QSPI                        /* CS in Flash */
} SYS_TCS_SOURCE;

static uint32_t fetch_tcs_entry(SYS_TCS_SOURCE source, uint32_t address)
{
        uint32_t cs_value = 0xFFFFFFFF;

        if (source == TCS_OTP) {
                cs_value = *(volatile uint32_t*)(MEMORY_OTP_BASE + OTP_CS_ADDRESS + address);
        } else if (source == TCS_QSPI) {
                cs_value =  *(volatile uint32_t*)(MEMORY_QSPIF_S_BASE + address);
        }

        return cs_value;
}

static void store_tcs(uint32_t address, uint8_t tcs_len, SYS_TCS_SOURCE source, uint16_t *index_table)
{
        int i = 0;
        uint8_t index;
        ASSERT_ERROR(tcs_data);

        uint32_t value = fetch_tcs_entry(source, address);
        SYS_TCS_GID gid = (uint8_t)(value & 0x000000FF);

        if (gid >= SYS_TCS_GROUP_MAX) {
                return;
        }

        if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_TRIM_VAL) {
                //for SYS_TCS_TYPE_TRIM_VAL values, fragmentation is not supported
                // always start at 0
                index_table[gid] = 0;
        }

        while (i < tcs_len) { //4 bytes values
                index = tcs_attributes[gid].start + index_table[gid];
                address += 4;
                tcs_data[index] = fetch_tcs_entry(source, address);
                index_table[gid]++;
                i++;
        }
}

/* The calculated size in bytes, that is the number
 * of entries * sizeof(int)
 */
static void get_size_of_cs(SYS_TCS_SOURCE source, uint32_t address, uint16_t *size, uint16_t *size_table)
{
        ASSERT_ERROR(size != NULL);

        uint32_t value = 0;
        uint8_t total = 0;
        uint8_t tcs_len = 0;
        SYS_TCS_GID gid;

        *size = 0;

        while (address < CS_MAX_SIZE) {
                value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MEMCTRL_BASE + 0x100) { // address - value pair
                        address += 0x4;
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        tcs_len = (value & 0x0000FF00) >> 8;
                        gid = (uint8_t)(value & 0x000000FF);
                        address += tcs_len*4; //skip next tcs values.

                        if (gid >= SYS_TCS_GROUP_MAX) {
                                address += 0x4; //skip this entry
                                continue;
                        }

                        if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_TRIM_VAL) {
                                //check that SYS_TCS_TYPE_TRIM_VAL values have the expected length
                                ASSERT_ERROR(tcs_len == sys_tcs_get_size(gid));

                                if (size_table[gid] == 0) {
                                        *size += 4 * tcs_len; //size should be in bytes
                                        size_table[gid] = tcs_len;
                                }
                        } else {
                                //check that SYS_TCS_TYPE_REG_PAIR values are of an even number
                                if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_REG_PAIR) {
                                        ASSERT_ERROR(size_table[gid] % 2 == 0);
                                }

                                *size += 4 * tcs_len; //size should be in bytes
                                size_table[gid] += tcs_len;
                        }
                }
                address += 0x4; //advance address by 4 bytes
        }

        //convert sizes to offsets in the tcs table and
        //set the sizes of the GID attributes
        for (gid = 0; gid < SYS_TCS_GROUP_MAX; gid++) {
                if (size_table[gid] != 0) {
                        tcs_attributes[gid].start = total;
                        total += size_table[gid];
                }
                tcs_attributes[gid].size = size_table[gid];
        }
}

static void store_cs_attributes(SYS_TCS_SOURCE source, uint32_t address, uint16_t *index_table)
{
        uint32_t value = 0;
        uint8_t tcs_len = 0;

        while (address < CS_MAX_SIZE) {
                value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MEMCTRL_BASE + 0x100) { // address - value pair
                        address += 0x4;
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        tcs_len = (value & 0x00000FF00) >> 8;
                        store_tcs(address, tcs_len, source, index_table);
                        address += tcs_len*4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
        }
}

void sys_tcs_get_trim_values_from_cs(void)
{
        uint32_t address = 0;
        uint32_t value = 0;
        uint16_t size = 0;
        SYS_TCS_SOURCE source = TCS_OTP;
        uint16_t size_table[SYS_TCS_GROUP_MAX] = { 0 };

#if (dg_configUSE_SYS_TCS == 0)
        return;
#endif
        //locate start of CS in OTP
        value = fetch_tcs_entry(source, address);
        if (value != CS_START_CMD) {
                //No start command found, try to locate CS in flash
                source = TCS_QSPI;
                value = fetch_tcs_entry(source, address);
                if (value != CS_START_CMD) {
                        return; // no CS found;
                }

        }

        address += 0x4;

        get_size_of_cs(source, address, &size, size_table);

        /* The calculated size in bytes, taking into account that one entry is 4 bytes */
        ASSERT_ERROR(size < CS_MAX_SIZE);

        /* Static allocation for tcs_data */
        tcs_data = __tcs_data;

        ASSERT_ERROR(tcs_data != NULL);

        //reuse the sizes table as index table
        memset(size_table, 0 , sizeof(size_table));
        store_cs_attributes(source, address, size_table);

#ifdef dg_configM33_USES_LEDS
        // sort entries for PD_PER
        // move PWMLED_CTRL_REG entries at the end
        // keep count of those entries
        uint8_t start = tcs_attributes[SYS_TCS_GROUP_PD_PER].start;
        size = tcs_attributes[SYS_TCS_GROUP_PD_PER].size;
        uint8_t index, j;
        pwmled_tcs_entries = 0;
        for (index = 0; index < size - pwmled_tcs_entries - 1; index += 2) {
                if (tcs_data[start + index] == (uint32_t)&PWMLED->PWMLED_CTRL_REG) {
                        pwmled_tcs_entries += 2;
                        for (j = 2; j < size - index - 1; j += 2) {
                                if (tcs_data[start + size - j] != (uint32_t)&PWMLED->PWMLED_CTRL_REG) {
                                        // swap reg pairs
                                       address = tcs_data[start + size -j];
                                       value = tcs_data[start + size -j + 1];
                                       tcs_data[start + size - j] = tcs_data[start + index];
                                       tcs_data[start + size - j + 1] = tcs_data[start + index + 1];
                                       tcs_data[start + index] = address;
                                       tcs_data[start + index + 1] = value;
                                       break;
                                }
                        }
                }
        }
#endif
}

uint8_t sys_tcs_get_size(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return tcs_attributes[gid].size;
}

SYS_TCS_TYPE sys_tcs_get_value_type(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return tcs_attributes[gid].value_type;
}

void sys_tcs_get_custom_values(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_TRIM_VAL);

        if (size) {
                if (tcs_attributes[gid].start == EMPTY) {
                      *size = 0;
                } else {
                      *size = tcs_attributes[gid].size;
                }
        }

        if (values) {
                if (tcs_data == NULL) {
                        *values = NULL;
                } else {
                        *values = &tcs_data[tcs_attributes[gid].start];
                }
        }
}

void sys_tcs_apply_custom_values(SYS_TCS_GID gid, sys_tcs_custom_values_cb cb, void *user_data)
{
        uint32_t* values = NULL;
        uint8_t size = 0;
        if (cb) {
                sys_tcs_get_custom_values(gid, &values, &size);
                if (size != 0) {
                        cb(gid, user_data, values, size);
                }
        }
}

void sys_tcs_get_reg_pairs(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (size) {
                *size = tcs_attributes[gid].size;
        }

        if (tcs_data == NULL) {
                *values = NULL;
        } else {
                *values = &tcs_data[tcs_attributes[gid].start];
        }
}

__RETAINED_CODE void sys_tcs_apply_reg_pairs(SYS_TCS_GID gid)
{
        if (tcs_data == NULL) {
                return;
        }
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        uint8_t start = tcs_attributes[gid].start;
        int size = (int)tcs_attributes[gid].size;

#if (dg_configM33_USES_LEDS == 0)
        /* if LEDs are used from SNC do not apply PWMLED_CTRL_REG TCS value
         * SNC will do it
         */
        if (gid == SYS_TCS_GROUP_PD_PER) {
                size -= pwmled_tcs_entries;
        }
#endif
        while (size > 0) {
                *(uint32_t *)tcs_data[start] = tcs_data[start+1];
                size -= 2;
                start += 2;
        }
}

void sys_tcs_apply_entries(SYS_TCS_DOMAIN domain, sys_tcs_custom_values_cb cb,  void *user_data)
{
        SYS_TCS_GID gid;

        for (gid = 0; gid < SYS_TCS_GROUP_MAX; gid++) {
                if (tcs_attributes[gid].start != EMPTY) {
                        if (tcs_attributes[gid].power_domain == domain) {
                                if (tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR) {
                                        sys_tcs_apply_reg_pairs(gid);
                                } else if (tcs_attributes[gid].value_type == SYS_TCS_TYPE_TRIM_VAL) {
                                        sys_tcs_apply_custom_values(gid, cb, user_data);
                                }
                        }
                }
        }
}

void sys_tcs_get_gid_per_domain(SYS_TCS_DOMAIN domain, uint8_t *gids, uint8_t size)
{
        int gid;
        uint8_t i = 0;

        for (gid = 0; gid < SYS_TCS_GROUP_MAX && i < size ; gid++) {
                if ((tcs_attributes[gid].start != EMPTY) &&
                    (tcs_attributes[gid].power_domain == domain)) {
                        gids[i++] = gid;
                }
        }
        //mark the end of the gids array with SYS_TCS_GROUP_RESERVED since it is not used.
        gids[i] = SYS_TCS_GROUP_RESERVED;
}

uint32_t *sys_tcs_snc_get_reg_pair(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (tcs_attributes[gid].start == EMPTY) {
                /* If there are no valid TCS values SNC will be aware of it from the size*/
                return (uint32_t*)1; //NO valid address
        }
        return &tcs_data[tcs_attributes[gid].start];
}

uint32_t sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

#if dg_configM33_USES_LEDS
        return ((tcs_attributes[gid].size - pwmled_tcs_entries) / 2);
#else
        return tcs_attributes[gid].size / 2;
#endif
}

void sys_tcs_custom_values_system_cb(SYS_TCS_GID gid, void *user_data, uint32_t *val, uint8_t len)
{
#if (dg_configUSE_HW_GPADC == 1)
        int16_t val_hi = (*val & 0xFFFF0000) >> 16;
        int16_t val_lo = *val & 0xFFFF;
#endif
        ASSERT_ERROR(val);
        switch (gid) {
#if (dg_configUSE_HW_GPADC == 1)
        case SYS_TCS_GROUP_GP_ADC_SINGLE_MODE:
                hw_gpadc_store_se_gain_error(hw_gpadc_calculate_single_ended_gain_error(val_lo, val_hi));
                hw_gpadc_store_se_offset_error(hw_gpadc_calculate_single_ended_offset_error(val_lo, val_hi));
                break;
        case SYS_TCS_GROUP_GP_ADC_DIFF_MODE:
                hw_gpadc_store_diff_gain_error(hw_gpadc_calculate_differential_gain_error(val_lo, val_hi));
                hw_gpadc_store_diff_offset_error(hw_gpadc_calculate_differential_offset_error(val_lo, val_hi));
                break;
#endif
        default:
                break;
        }
}

bool sys_tcs_reg_pairs_in_cs(const uint32_t *reg_address, uint8_t num, bool *trimmed_reg)
{
        uint32_t address = 0;
        uint32_t value = 0;
        uint8_t i = 0;

        if (num == 0) {
                return false;
        }

        while (address < CS_MAX_SIZE) {
                value = fetch_tcs_entry(TCS_OTP, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MEMCTRL_BASE + 0x100) { // address - value pair
                        for (i = 0; i < num; i++) {
                                if (value == reg_address[i]) {
                                        trimmed_reg[i] = true;
                                        break;
                                }
                        }
                        address += 0x4;
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        address += ((value & 0x00000FF00) >> 8)*4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
        }

        for (i = 0; i < num; i++) {
                if (trimmed_reg[i] == false) {
                        return false;
                }
        }
        return true;
}

#if (dg_configUSE_HW_SDADC == 1)
#include "hw_sdadc.h"
void hw_sdadc_get_trimmed_values(uint8_t mode, int16_t *gain, int16_t *offs)
{
        uint32_t *values;
        uint8_t size;

        switch (mode) {
        case HW_SDADC_INPUT_MODE_SINGLE_ENDED:
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_SD_ADC_SINGLE_MODE, &values, &size);
                break;
        case HW_SDADC_INPUT_MODE_DIFFERENTIAL:
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_SD_ADC_DIFF_MODE, &values, &size);
                break;
        default:
                values = NULL;
                break;
        }
        ASSERT_ERROR(values != NULL);

        for (int i = 0; i < size; i+=2) {
                if ((uint32_t *)values[i] == &SDADC->SDADC_GAIN_CORR_REG) {
                        *gain = values[i+1] & REG_MSK(SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR);
                } else if ((uint32_t *)values[i] == &SDADC->SDADC_OFFS_CORR_REG) {
                        *offs = values[i+1] & REG_MSK(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR);
                }
        }
}
#endif /* dg_configUSE_HW_SDADC */

/**
\}
\}
\}
*/
