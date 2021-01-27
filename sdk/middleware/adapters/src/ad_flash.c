/**
 ****************************************************************************************
 *
 * @file ad_flash.c
 *
 * @brief Flash adapter implementation
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configFLASH_ADAPTER

#include <string.h>
#include <osal.h>
#include <ad_flash.h>
#include <qspi_automode.h>
#ifndef OS_BAREMETAL
#include <sys_power_mgr.h>
#endif
#include "hw_cache.h"

/**
 * Enable/Disable run-time checks for possible cache incoherence:
 *  - 1 to enable
 *  - 0 to disable
 */
#define DETECT_CACHE_INCOHERENCE_DANGER         0

#define FLASH_PAGE_SIZE   0x0100

/*
 * In case that user tries to write data to flash passing as source QSPI mapped flash address
 * data must be first copied to RAM as during write QSPI controller can't read flash.
 * To achieve this, small on stack buffer will be used to copy data to RAM first.
 * This define specifies how much stack can be used for this purpose.
 */
#define ON_STACK_BUFFER_SIZE 16

__RETAINED static bool initialized;
#ifndef OS_BAREMETAL
__RETAINED static OS_MUTEX flash_mutex;
#endif
__RETAINED static uint32_t no_cache_flush_base;
__RETAINED static uint32_t no_cache_flush_end;

__STATIC_INLINE bool is_flash_addr_cached(uint32_t addr)
{
        uint32_t cache_len;
        uint32_t cache_base;

        if (REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0) != 2) {
                return false;
        }

        /*
         * Cacheable area is N * 64KB
         *
         * N == 0 --> no caching
         */
        cache_len = hw_cache_get_len();

        cache_base = REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE) << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
        cache_base -= MEMORY_QSPIF_BASE;


        return ((addr >= cache_base) && (addr < cache_base + (cache_len << 16)));
}

__STATIC_INLINE bool triggers_cache_flushing(uint32_t base, uint32_t size)
{
        if ((base >= no_cache_flush_base) && ((base + size) <= no_cache_flush_end))
                return false;
        else
                return true;
}

void ad_flash_init(void)
{
        if (!initialized) {
                initialized = true;
#ifndef OS_BAREMETAL
                OS_MUTEX_CREATE(flash_mutex);
                OS_ASSERT(flash_mutex);
#endif

                ad_flash_lock();
                if (dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH) {
                        qspi_automode_flash_power_up();
                }
                no_cache_flush_base = AD_FLASH_ALWAYS_FLUSH_CACHE;
                no_cache_flush_end = 0;

                ad_flash_unlock();
        }
}

size_t ad_flash_read(uint32_t addr, uint8_t *buf, size_t len)
{
        size_t readed;
        ASSERT_WARNING(buf);

#if DETECT_CACHE_INCOHERENCE_DANGER
        /* An address within the cacheable area and a read space excluded
         * from being flushed (see ad_flash_skip_cache_flushing()) create a condition
         * for potential cache incoherence.
         */
        OS_ASSERT(!is_flash_addr_cached(addr) || triggers_cache_flushing(addr, len));
#endif
        ad_flash_lock();
        readed = qspi_automode_read(addr, buf, len);
        ad_flash_unlock();
        return readed;
}

__STATIC_INLINE bool qspi_address(const void *buf)
{
        if (IS_QSPIF_ADDRESS((uint32_t) buf)) {
                return true;
        }
        else if (IS_QSPIF_S_ADDRESS((uint32_t) buf)) {
                return true;
        }
#if dg_configUSE_HW_QSPI2
        else if (((uint32_t) buf >= MEMORY_QSPIR_BASE) && ((uint32_t) buf < MEMORY_QSPIR_END)) {
                return true;
        }
#endif


        return IS_REMAPPED_ADDRESS(buf) && (REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0) == 2);
}

static size_t ad_flash_write_from_qspi(uint32_t addr, const uint8_t *qspi_buf, size_t size)
{
        size_t offset = 0;
        uint8_t buf[ON_STACK_BUFFER_SIZE];

        /* Get the FLASH offset of qspi_buf */
        if (IS_QSPIF_ADDRESS((uint32_t) qspi_buf)) {
                qspi_buf -= MEMORY_QSPIF_BASE;
        } else if (IS_QSPIF_S_ADDRESS((uint32_t) qspi_buf)) {
                qspi_buf -= MEMORY_QSPIF_S_BASE;
        } else {
                uint32_t flash_region_base_offset;
                flash_region_base_offset = REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE) << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
                flash_region_base_offset += REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET) << 2;
                flash_region_base_offset -= MEMORY_QSPIF_BASE;
                qspi_buf += flash_region_base_offset;
        }

        ASSERT_WARNING(IS_REMAPPED_ADDRESS(qspi_buf));

        /* Get automode address of qspi_buf */
        qspi_buf = (const uint8_t*) qspi_automode_addr((uint32_t) qspi_buf);

        /*
         * qspi_automode_write_flash_page can't write data if source address points to QSPI mapped
         * memory. To write data from QSPI flash, it will be first copied to small on stack
         * buffer. This buffer can be accessed when QSPI controller is working in manual mode.
         */
        while (offset < size) {
                size_t chunk = sizeof(buf) > size - offset ? size - offset: sizeof(buf);
                memcpy(buf, qspi_buf + offset, chunk);

                size_t written = qspi_automode_write_flash_page(addr + offset, buf, chunk);
                offset += written;
        }
        return offset;
}

size_t ad_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
        size_t written;
        size_t offset = 0;
        bool buf_from_flash = qspi_address(buf);

        ASSERT_WARNING(buf);

        ad_flash_lock();

        while (offset < size) {
                /*
                 * If buf is QSPI Flash memory, use function that will copy source data to RAM
                 * first.
                 */
                if (buf_from_flash) {
                        written = ad_flash_write_from_qspi(addr + offset, buf + offset,
                                                                                size - offset);
                } else {
                        /*
                         * Try write everything, lower driver will reduce this value to accommodate
                         * page boundary and and maximum write size limitation
                         */
                        written = qspi_automode_write_flash_page(addr + offset, buf + offset,
                                                                                size - offset);
                }
                offset += written;
        }

        if (is_flash_addr_cached(addr & ~(AD_FLASH_SECTOR_SIZE - 1))
                        &&  triggers_cache_flushing(addr, size)) {
                hw_cache_flush();
        }

        ad_flash_unlock();

        return size;
}

bool ad_flash_erase_region(uint32_t addr, size_t size)
{
        uint32_t flash_offset = addr & ~(AD_FLASH_SECTOR_SIZE - 1);

        ad_flash_lock();

        while (flash_offset < addr + size) {
                qspi_automode_erase_flash_sector(flash_offset);
                flash_offset += AD_FLASH_SECTOR_SIZE;
        }

        if (is_flash_addr_cached(flash_offset)  &&  triggers_cache_flushing(addr, size)) {
                hw_cache_flush();
        }

        ad_flash_unlock();

        return true;
}

bool ad_flash_chip_erase(void)
{
        ad_flash_lock();

        qspi_automode_erase_chip();

        ad_flash_unlock();

        return true;
}

int ad_flash_update_possible(uint32_t addr, const uint8_t *data_to_write, size_t size)
{
        int i;
        int same;
        const uint8_t *old = qspi_automode_addr(addr);

        ASSERT_WARNING(data_to_write);

        /* Check if new data is same as old one, in which case no write will be needed */
        for (i = 0; i < size && old[i] == data_to_write[i]; ++i) {
        }

        /* This much did not change */
        same = i;

        /* Check if new data can be stored by clearing bits only */
        for (; i < size ; ++i) {
                if ((old[i] & data_to_write[i]) != data_to_write[i])
                        /*
                         * Found byte that needs to have at least one bit set and it was cleared,
                         * erase will be needed.
                         */
                        return -1;
        }
        return same;
}

size_t ad_flash_erase_size(void)
{
        return AD_FLASH_SECTOR_SIZE;
}

void ad_flash_lock(void)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_GET(flash_mutex, OS_MUTEX_FOREVER);
#endif
}

void ad_flash_unlock(void)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(flash_mutex);
#endif
}

void ad_flash_skip_cache_flushing(uint32_t base, uint32_t size)
{
        ASSERT_WARNING((base == AD_FLASH_ALWAYS_FLUSH_CACHE) || IS_REMAPPED_ADDRESS(base));

        no_cache_flush_base = base;
        no_cache_flush_end = base + size;
        if (no_cache_flush_end < no_cache_flush_base)
                no_cache_flush_end = 0;
}

#ifndef OS_BAREMETAL
ADAPTER_INIT(ad_flash_adapter, ad_flash_init)
#endif

#endif
