/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_CACHE Cache Controller
 * \{
 * \brief Cache Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache.h
 *
 * @brief Definition of API for the Cache Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_CACHE_H_
#define HW_CACHE_H_


#if dg_configUSE_HW_CACHE

#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Cache Line Sizes
 *
 */
typedef enum {
        HW_CACHE_LINESZ_8_BYTES        =  0,
        HW_CACHE_LINESZ_16_BYTES       =  1,
        HW_CACHE_LINESZ_32_BYTES       =  2,
        HW_CACHE_LINESZ_AS_IS          =  3,
        HW_CACHE_LINESZ_INVALID,
} HW_CACHE_LINESZ;

/**
 * \brief Cache Associativity
 *
 */
typedef enum {
        HW_CACHE_ASSOC_1_WAY           =  0,
        HW_CACHE_ASSOC_2_WAY           =  1,
        HW_CACHE_ASSOC_4_WAY           =  2,
        HW_CACHE_ASSOC_AS_IS           =  3,
        HW_CACHE_ASSOC_INVALID,
} HW_CACHE_ASSOC;

/**
 * \brief Cache RAM Sizes
 *
 * \note Changing this to 16KB would only be necessary for debugging
 *
 */
typedef enum {
        HW_CACHE_RAMSZ_8KB             = 1,
        HW_CACHE_RAMSZ_16KB            = 2,
} HW_CACHE_RAMSZ;

/**
 * \brief Cache Flash Region Sizes
 *
 */
typedef enum {
        HW_CACHE_FLASH_REGION_SZ_8MB  = 2,
        HW_CACHE_FLASH_REGION_SZ_4MB  = 3,
        HW_CACHE_FLASH_REGION_SZ_2MB  = 4,
        HW_CACHE_FLASH_REGION_SZ_1MB  = 5,
        HW_CACHE_FLASH_REGION_SZ_512KB  = 6,
        HW_CACHE_FLASH_REGION_SZ_256KB  = 7
} HW_CACHE_FLASH_REGIONSZ;

typedef uint16_t flash_region_base_t;
typedef uint16_t flash_region_offset_t;

/**
 * \brief Flush the cache contents
 *
 */
__STATIC_INLINE void hw_cache_flush(void)
{
        REG_SET_BIT(CACHE, CACHE_CTRL1_REG, CACHE_FLUSH);
}

/**
 * \brief Set the cacheable memory length
 *
 * \param [in] len The cacheable memory length, in 64KB blocks. The actual cacheable
 *                 memory length will therefore be len * 64KB. A value of 0 disables
 *                 the cache. Valid values: [0, 511]
 */
__STATIC_INLINE void hw_cache_set_len(uint32_t len)
{
        ASSERT_WARNING((len & ~CACHE_CACHE_CTRL2_REG_CACHE_LEN_Msk) == 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN, len);
}

/**
 * \brief Get the cacheable memory length
 *
 * \return The cacheable memory length, in 64KB blocks. The actual cacheable
 *         memory length will therefore be len * 64KB. A value of 0 disables
 *         the cache
 */
__STATIC_INLINE int hw_cache_get_len(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN);
}

/**
 * \brief Enable cache
 *
 * \param [in] len The cacheable memory length, in 64KB blocks. The actual cacheable
 *                 memory length will therefore be len * 64KB. Valid values: [1, 511]
 *
 */
__STATIC_INLINE void hw_cache_enable(uint32_t len)
{
        hw_cache_set_len(len);
}

/**
 * \brief Disable cache
 *
 */
__STATIC_INLINE void hw_cache_disable(void)
{
        hw_cache_set_len(0);
}
/**
 * \brief Configure the cache before reseting Cache Controller (static
 *        configuration, requires a cache reset).
 *
 * \param [in] assoc Cache Associativity
 * \param [in] linesz Cache Line size
 * \param [in] ramsz Cache RAM size to use
 *
 * \note This will disable the cache temporarily. The only reason to use this
 *       function is to change the Cache RAM size, which will probably only be
 *       needed for evaluation purposes.
 *
 */
__STATIC_INLINE void hw_cache_reset_config(HW_CACHE_ASSOC assoc, HW_CACHE_LINESZ linesz,
                                           HW_CACHE_RAMSZ ramsz)
{
        int cache_len = hw_cache_get_len();
        uint32_t val;

        /* Disable the cache */
        hw_cache_disable();

        /* Update the configuration */
        val = *(volatile uint32_t *)0x100C0024;
        RAW_SET_FIELD(val, 0xcUL, linesz);
        RAW_SET_FIELD(val, 0x3UL, assoc);
        RAW_SET_FIELD(val, 0x70UL, ramsz);
        *(volatile uint32_t *)0x100C0024 = val;

        /* reset the cache controller to apply new configuration */
        RAW_SETF(0x100C0024, 0x80UL, 1);
        RAW_SETF(0x100C0024, 0x80UL, 0);

        /* Re-enable the cache */
        hw_cache_enable(cache_len);
}

/**
 * \brief Set the cache line size (runtime)
 *
 * \param [in] linesz Cache Line size
 *
 * \note If linesz is set to HW_CACHE_LINESZ_8_BYTES, the cache
 *       will be flushed in order for the setting to take
 *       effect. This cache flushing will need around 0x200
 *       clock cycles to be completed.
 */
__STATIC_INLINE void hw_cache_set_linesz(HW_CACHE_LINESZ linesz)
{
        CACHE->CACHE_LNSIZECFG_REG = linesz;

        if (linesz == HW_CACHE_LINESZ_8_BYTES) {
                REG_SET_BIT(CACHE, CACHE_CTRL1_REG, CACHE_FLUSH);
        }
}

/**
 * \brief Get the cache line size (runtime)
 *
 * \return Cache Line size
 *
 */
__STATIC_INLINE HW_CACHE_LINESZ hw_cache_get_linesz(void)
{
        return CACHE->CACHE_LNSIZECFG_REG;
}

/**
 * \brief Set the cache associativity (runtime)
 *
 * \param [in] assoc Cache Associativity
 *
 */
__STATIC_INLINE void hw_cache_set_assoc(HW_CACHE_ASSOC assoc)
{
        CACHE->CACHE_ASSOCCFG_REG = assoc;
}

/**
 * \brief Get the cache associativity (runtime)
 *
 * \return Cache Associativity
 *
 */
__STATIC_INLINE HW_CACHE_ASSOC hw_cache_get_assoc(void)
{
        return CACHE->CACHE_ASSOCCFG_REG;
}

/**
 * \brief Configure the cache
 *
 * \param [in] assoc  Cache Associativity
 * \param [in] linesz Cache Line size
 * \param [in] len    Length of QSPI FLASH cacheable memory
 *
 */
__STATIC_INLINE void hw_cache_config(HW_CACHE_ASSOC assoc, HW_CACHE_LINESZ linesz, uint32_t len)
{
        uint32_t configured_len = hw_cache_get_len();
        uint32_t configured_assoc = hw_cache_get_assoc();
        uint32_t configured_linesz = hw_cache_get_linesz();

        if ((configured_assoc == assoc || configured_assoc == HW_CACHE_ASSOC_AS_IS) &&
            (configured_linesz == linesz || configured_linesz == HW_CACHE_LINESZ_AS_IS) &&
            configured_len == len) {
                /* The configuration is the same. Do not reapply it */
                return;
        }

        ASSERT_ERROR(assoc < HW_CACHE_ASSOC_INVALID && linesz < HW_CACHE_LINESZ_INVALID);

        /* cache_len shouldn't set any bits that do not fit in CACHE_CTRL2_REG.CACHE_LEN */
        ASSERT_ERROR((len & CACHE_CACHE_CTRL2_REG_CACHE_LEN_Msk) == len);

        GLOBAL_INT_DISABLE();

        /* Disable cache */
        hw_cache_disable();

        if ( assoc != HW_CACHE_ASSOC_AS_IS) {
                /* override the set associativity setting */
                hw_cache_set_assoc(assoc);
        }

        if (linesz != HW_CACHE_LINESZ_AS_IS) {
                /* override the cache line setting */
                hw_cache_set_linesz(linesz);
        }

        /* flush cache */
        hw_cache_flush();

        hw_cache_enable(len);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the cache MRM interrupt threshold for misses
 *
 * Defines the threshold (in misses) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_misses_thres(uint32_t thres)
{
        CACHE->CACHE_MRM_MISSES_THRES_REG = thres;
}

/**
 * \brief Get the cache MRM interrupt threshold for misses
 *
 * \return The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_misses_thres(void)
{
        return CACHE->CACHE_MRM_MISSES_THRES_REG;
}

/**
 * \brief Set the cache MRM interrupt threshold for hits
 *
 * Defines the threshold (in hits) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits_thres(uint32_t thres)
{
        CACHE->CACHE_MRM_HITS_THRES_REG = thres;
}

/**
 * \brief Get the cache MRM interrupt threshold for hits
 *
 * \return The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits_thres(void)
{
        return CACHE->CACHE_MRM_HITS_THRES_REG;
}

/**
 * \brief Get the cache MRM misses threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of misses reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_cache_mrm_get_misses_thres_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Clear the cache MRM misses threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_misses_thres_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Get the cache MRM hits threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of hits reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_cache_mrm_get_hits_thres_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Clear the cache MRM hits threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_hits_thres_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Set the cache MRM monitoring time interval
 *
 * Defines the time interval for the monitoring in 32 MHz clock cycles.
 * Must be an 19-bit value max. When this time is reached, an interrupt
 * will be generated.
 * A value of 0 disables interrupt generation
 *
 * \param [in] tint Monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_tint(uint32_t tint)
{
        ASSERT_WARNING((tint & ~CACHE_CACHE_MRM_TINT_REG_MRM_TINT_Msk) == 0)
                CACHE->CACHE_MRM_TINT_REG = tint;
}

/**
 * \brief Get the cache MRM monitoring time interval
 *
 * \return The monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_tint(void)
{
        return CACHE->CACHE_MRM_TINT_REG & CACHE_CACHE_MRM_TINT_REG_MRM_TINT_Msk;
}

/**
 * \brief Get the cache MRM timer interval IRQ status
 *
 * \return True if an interrupt has been generated because the time
 *         interval counter reached the end (time interval != 0).
 *
 */
__STATIC_INLINE bool hw_cache_mrm_get_tint_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Clear the cache MRM timer interval IRQ status
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_tint_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Start MRM counters
 *
 * \note If Timer interval is not set to 0 using
 *       hw_cache_mrm_set_tint, the timer interval will count down to 0.
 *       When zero is reached, an interrupt will be generated, and the
 *       counters will be disabled automatically.
 */
__STATIC_INLINE void hw_cache_mrm_start_counters(void)
{
        REG_SET_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Freeze MRM counters
 *
 */
__STATIC_INLINE void hw_cache_mrm_freeze_counters(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Get the cache MRM misses number
 *
 * \return The number of cache misses
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_misses(void)
{
        return CACHE->CACHE_MRM_MISSES_REG;
}

/**
 * \brief Set the cache MRM cache misses number
 *
 * This is primarily intended for clearing the misses number
 *
 * \param[in] misses The number of cache misses
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_misses(uint32_t misses)
{
        CACHE->CACHE_MRM_MISSES_REG = misses;
}

/**
 * \brief Get the cache MRM cache hits number
 *
 * \return The number of cache hits
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits(void)
{
        return CACHE->CACHE_MRM_HITS_REG;
}

/**
 * \brief Set the cache MRM cache hits number
 *
 * This is primarily intended for clearing the hits number
 *
 * \param[in] hits The number of cache hits
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits(uint32_t hits)
{
        CACHE->CACHE_MRM_HITS_REG = hits;
}

/**
 * \brief Set the flash region base
 *
 * \param [in] base The Flash region base corresponds to the flash address bits
 *             [31:16]. Bits [31:25] should be fixed to '0xb' and bits [17:16]
 *             should be fixed to '0x0'. Therefore, valid values are from 0x1600 to
 *             0x17fc.
 *
 * \note This should be aligned to the region size value (hw_cache_flash_set_region_size()).
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_base(flash_region_base_t base)
{
        ASSERT_WARNING((base & 0xfe03) == 0x1600);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE, base);
}

/**
 * \brief Get the flash region size
 *
 * \return The Flash region size to use with the cache
 */
__STATIC_INLINE flash_region_base_t hw_cache_flash_get_region_base(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE);
}

/**
 * \brief Set the flash region offset
 *
 * This value (expressed in words) is added to flash region base
 * (see hw_cache_flash_set/get_region_base()) to configure the flash area to
 * remap to 0x0 and cache.
 *
 * \param [in] offset Flash region offset in 32-bit words. Max: 0xFFF
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_offset(flash_region_offset_t offset)
{
        ASSERT_WARNING(offset < 0x1000);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET, offset);
}

/**
 * \brief Get the flash region offset
 *
 * \return The Flash region size to use with the cache (in 32-bit words)
 */
__STATIC_INLINE flash_region_offset_t hw_cache_flash_get_region_offset(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET);
}

/**
 * \brief Set the flash region size
 *
 * \param [in] sz The Flash region size to use with the cache
 *
 * This is the size after flash region base (see hw_cache_flash_set_region_base()) plus
 * flash region offset (see hw_cache_flash_set_region_offset()) that will be cached.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_size(HW_CACHE_FLASH_REGIONSZ sz)
{
        ASSERT_WARNING(sz >= HW_CACHE_FLASH_REGION_SZ_8MB && sz <= HW_CACHE_FLASH_REGION_SZ_256KB);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE, sz);
}

/**
 * \brief Get the flash region size
 *
 * \return The Flash region size to use with the cache
 */
__STATIC_INLINE HW_CACHE_FLASH_REGIONSZ hw_cache_flash_get_region_size(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE);
}

/**
 * \brief Configure the cached flash region
 *
 * This API configures the cacheable flash region. This starts at an address with the 16-upper
 * bits being equal to the 'base' parameter. Additionally, another 4 * 'offset' bytes (since the
 * 'offset' param is expressed in words) are added to the address. Therefore, the start address for
 * the flash image to be cached is at (base << 16) | (offset << 2).
 * The size of the cached flash area is defined by the 'sz' argument. Note, however, that the 'base'
 * parameter (without the offset) must be aligned to the size indicated by the 'sz' param.
 *
 * Cache Length (see hw_cache_set_cache_len()) must have been configured accordingly. Please note
 * that since cache length is 64-bit aligned, it must cover the entire size from 'base', not including
 * the offset in the beginning. So, in order to take into account the last 'offset' bytes, it may need
 * an additional 64KB region.
 *
 * This is an alternative API to hw_cache_flash_set_region_base()/_size(). It automatically
 * configures the entire flash region in one call.
 * At the same time, it provides some additional sanity checks to the ones performed by the
 * individual API functions, like making sure that the region base address is properly aligned
 * according to the selected region size.
 *
 * \param [in] base The Flash region base corresponds to the flash address bits
 *             [31:16]. Bits [31:25] should be fixed to '0xb' and bits [17:16]
 *             should be fixed to '0x0'. Therefore, valid values are from 0x1600 to
 *             0x17fc. This address should be 'size'-param aligned.
 * \param [in] sz The Flash region size to use with the cache
 * \param [in] offset Flash region offset in 32-bit words. Max: 0xFFF
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_configure_region(flash_region_base_t base, HW_CACHE_FLASH_REGIONSZ sz,
        flash_region_offset_t offset)
{
        ASSERT_WARNING((base & ( (1 << (9-sz)) - 1)) == 0);

        hw_cache_flash_set_region_base(base);
        hw_cache_flash_set_region_size(sz);
        hw_cache_flash_set_region_offset(offset);
}

/**
 * \brief Application defined callback for the MRM interrupt.
 *
 * \note The application defined callback should be declared as  __RETAINED_CODE.
 *
 */
typedef void (*hw_cache_cb_t)(void);

/**
 * \brief Enable the MRM interrupt generation
 *
 * The application should define its own callback. The latter is registered
 * and then invoked when the MRM interrupt is generated.
 *
 * \param [in] cb Callback defined by the application.
 *
 */
void hw_cache_mrm_enable_interrupt(hw_cache_cb_t cb);

/**
 * \brief Disable the MRM interrupt generation
 *
 * \note The application defined called is unregistered.
 *
 */
void hw_cache_mrm_disable_interrupt(void);

#endif /* dg_configUSE_HW_CACHE */


#endif /* HW_CACHE_H_ */

/**
 * \}
 * \}
 */
