/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup SYS
\{
*/

/**
****************************************************************************************
*
* @file hw_sys_da1469x.c
*
* @brief System Driver
*
* Copyright (C) 2017-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_SYS

#include <stdint.h>
#include "hw_cpm.h"
#include "hw_clk.h"
#include "hw_pd.h"
#include "hw_sys.h"
#include "hw_gpio.h"
#include "sys_tcs_da1469x.h"

#define SW_CURSOR_GPIO                  *(SW_CURSOR_PORT == 0 ? \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P0_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P0_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P0_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P0_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P0_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P0_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P0_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P0_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P0_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P0_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P0_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P0_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P0_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P0_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P0_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P0_15_MODE_REG) : \
                                                (SW_CURSOR_PIN == 16 ? &(GPIO->P0_16_MODE_REG) : \
                                                (SW_CURSOR_PIN == 17 ? &(GPIO->P0_17_MODE_REG) : \
                                                (SW_CURSOR_PIN == 18 ? &(GPIO->P0_18_MODE_REG) : \
                                                (SW_CURSOR_PIN == 19 ? &(GPIO->P0_19_MODE_REG) : \
                                                (SW_CURSOR_PIN == 20 ? &(GPIO->P0_20_MODE_REG) : \
                                                (SW_CURSOR_PIN == 21 ? &(GPIO->P0_21_MODE_REG) : \
                                                (SW_CURSOR_PIN == 22 ? &(GPIO->P0_22_MODE_REG) : \
                                                (SW_CURSOR_PIN == 23 ? &(GPIO->P0_23_MODE_REG) : \
                                                (SW_CURSOR_PIN == 24 ? &(GPIO->P0_24_MODE_REG) : \
                                                (SW_CURSOR_PIN == 25 ? &(GPIO->P0_25_MODE_REG) : \
                                                (SW_CURSOR_PIN == 26 ? &(GPIO->P0_26_MODE_REG) : \
                                                (SW_CURSOR_PIN == 27 ? &(GPIO->P0_27_MODE_REG) : \
                                                (SW_CURSOR_PIN == 28 ? &(GPIO->P0_28_MODE_REG) : \
                                                (SW_CURSOR_PIN == 29 ? &(GPIO->P0_29_MODE_REG) : \
                                                (SW_CURSOR_PIN == 30 ? &(GPIO->P0_30_MODE_REG) : \
                                                                       &(GPIO->P0_31_MODE_REG)))))))))))))))))))))))))))))))) \
                                                : \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P1_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P1_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P1_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P1_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P1_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P1_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P1_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P1_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P1_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P1_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P1_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P1_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P1_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P1_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P1_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P1_15_MODE_REG) : \
                                                (SW_CURSOR_PIN == 16 ? &(GPIO->P1_16_MODE_REG) : \
                                                (SW_CURSOR_PIN == 17 ? &(GPIO->P1_17_MODE_REG) : \
                                                (SW_CURSOR_PIN == 18 ? &(GPIO->P1_18_MODE_REG) : \
                                                (SW_CURSOR_PIN == 19 ? &(GPIO->P1_19_MODE_REG) : \
                                                (SW_CURSOR_PIN == 20 ? &(GPIO->P1_20_MODE_REG) : \
                                                (SW_CURSOR_PIN == 21 ? &(GPIO->P1_21_MODE_REG) : \
                                                                       &(GPIO->P1_22_MODE_REG))))))))))))))))))))))))

#define SW_CURSOR_SET                   *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_SET_DATA_REG) :    \
                                                                &(GPIO->P1_SET_DATA_REG))

#define SW_CURSOR_RESET                 *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_RESET_DATA_REG) :  \
                                                                &(GPIO->P1_RESET_DATA_REG))

#define SW_CURSOR_SET_PAD_LATCH         *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_SET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_SET_PAD_LATCH_REG))

#define SW_CURSOR_RESET_PAD_LATCH       *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_RESET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_RESET_PAD_LATCH_REG))

#define NUM_OF_REG_CONFIG_ENTRIES       5

/**
 * \brief The number of register addresses to check if they are configured in CS
 *
 * \sa hw_sys_apply_default_values
 */
#define NUM_OF_REG_ADDR_IN_CS           2

/**
 * \brief VCO tune voltage low and high levels
 */
#define TUNE_VOLTAGE_LOW_LEVEL          125
#define TUNE_VOLTAGE_HIGH_LEVEL         550

/**
 * \brief VCO current trimming
 */
__RETAINED static uint16_t pll_min_current;

/*
 * Indicates which master has currently access to a certain peripheral.
 * hw_sys_sw_bsr[periph_id] == BSR_MASTER_X: Indicates that master X has currently access to that peripheral.
 * This allows for more efficient calculations on SNC side.
 */
__RETAINED uint32_t hw_sys_sw_bsr[BSR_PERIPH_ID_MAX];
__RETAINED static uint8_t hw_sys_sw_bsr_cnt[BSR_PERIPH_ID_MAX];

__RETAINED static hw_sys_reg_config_t hw_sys_reg_config[NUM_OF_REG_CONFIG_ENTRIES];
__RETAINED static uint32_t hw_sys_reg_num_of_config_entries;

__RETAINED uint32_t hw_sys_pd_com_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_periph_acquire_cnt;

void hw_sys_set_preferred_values(HW_PD pd)
{
        switch (pd) {
        case HW_PD_AON:
                /* PD_AON. Apply if no CS value is set by the booter */
                if (CRG_TOP->PMU_TRIM_REG == 0x8800) {
                        CRG_TOP->PMU_TRIM_REG = DEFAULT_PMU_TRIM_REG;
                }

                REG_SET_MASKED(CRG_TOP, BANDGAP_REG, 0x00001000, 0x00001020);
                CRG_TOP->BIAS_VREF_SEL_REG = 0x000000CA;
                REG_SET_MASKED(CRG_TOP, BOD_LVL_CTRL0_REG, 0x0003FFFF, 0x041E6EF4);
                break;
        case HW_PD_SYS:
                REG_SET_MASKED(CHARGER, CHARGER_CTRL_REG, 0x00000C00, 0x003F6A78);
                REG_SET_MASKED(CHARGER, CHARGER_PWR_UP_TIMER_REG, 0x000003FF, 0x00000002);
                break;

        case HW_PD_TMR:
                REG_SET_MASKED(CRG_XTAL, CLK_FREQ_TRIM_REG, 0x3FF00000, 0x000AFD70);
                REG_SET_MASKED(CRG_XTAL, TRIM_CTRL_REG,     0x000000C0, 0x00000562);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL0_REG, 0x03C38002, 0x0801E6B6);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL1_REG, 0x007FFF00, 0x7500A1A4);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL2_REG, 0x00000FFF, 0x001E45C4);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL3_REG, 0x40000000, 0x40096255);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL4_REG, 0x00C00000, 0x00C00000);
                REG_SET_MASKED(CRG_XTAL, XTALRDY_CTRL_REG,  0x000000FF, 0x00000180);
                break;
        default:
                ASSERT_ERROR(pd < HW_PD_MAX);
                break;
        }
}

void hw_sys_setup_sw_cursor(void)
{
        if (dg_configUSE_SW_CURSOR == 1) {
                hw_sys_pd_com_enable();
                hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
                hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_sys_pd_com_disable();
        }
}

void hw_sys_trigger_sw_cursor(void)
{
        if (dg_configUSE_SW_CURSOR == 1) {
                hw_sys_pd_com_enable();
                hw_gpio_configure_pin(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, true);
                hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_clk_delay_usec(50);
                hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
                hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_sys_pd_com_disable();
        }
}

void hw_sys_assert_trigger_gpio(void)
{
        if (EXCEPTION_DEBUG == 1) {
                hw_pd_power_up_com();
                if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                        hw_clk_configure_ext32k_pins();
                }
                hw_gpio_pad_latch_enable_all();

                DBG_SET_HIGH(EXCEPTION_DEBUG, EXCEPTIONDBG);
        }
}


bool hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos)
{
        ASSERT_ERROR((hw_bsr_master_id & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_WARNING((pos % 2) == 0);
        ASSERT_WARNING(pos < 31);

        MEMCTRL->BUSY_SET_REG = hw_bsr_master_id << pos;
        if (((MEMCTRL->BUSY_STAT_REG >> pos) & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id) {
                return true;
        } else {
                return false;
        }
}

static HW_BSR_MASTER_ID hw_sys_sw_bsr_to_hw_bsr(SW_BSR_MASTER_ID sw_bsr_master_id) {
        switch (sw_bsr_master_id) {
        case SW_BSR_MASTER_SNC:
                return HW_BSR_MASTER_SNC;
                break;
        case SW_BSR_MASTER_SYSCPU:
                return HW_BSR_MASTER_SYSCPU;
                break;
        case SW_BSR_MASTER_CMAC:
                return HW_BSR_MASTER_CMAC;
                break;
        default:
                ASSERT_ERROR(0);
                return HW_BSR_MASTER_NONE;
                break;
        };
}

void hw_sys_hw_bsr_unlock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos)
{
        ASSERT_ERROR((hw_bsr_master_id & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_ERROR(((MEMCTRL->BUSY_STAT_REG >> pos) & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_WARNING((pos % 2) == 0);
        ASSERT_WARNING(pos < 31);

        MEMCTRL->BUSY_RESET_REG = hw_bsr_master_id << pos;
}

void hw_sys_sw_bsr_init(void)
{
        int i;

        for (i = 0; i < sizeof(hw_sys_sw_bsr) / sizeof(hw_sys_sw_bsr[0]); i++) {
                hw_sys_sw_bsr[i] = SW_BSR_MASTER_NONE;
                hw_sys_sw_bsr_cnt[i] = 0;
        }

        MEMCTRL->BUSY_RESET_REG = MEMCTRL->BUSY_STAT_REG;
}

bool hw_sys_sw_bsr_try_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;
        bool acquired = false;

        ASSERT_ERROR (periph_id < BSR_PERIPH_ID_MAX);

        /*
         * Since hw_sys_hw_bsr_try_lock() / hw_sys_hw_bsr_unlock() sequence
         * can be executed from different contexts, the caller must
         * either disable interrupts or use a mutex to protect against
         * the following scenario:
         * -  Context1: hw_sys_hw_bsr_try_lock()
         * -  Context2: hw_sys_hw_bsr_try_lock()
         * -  Context2: hw_sys_hw_bsr_unlock()
         * -  Context1: HW BSR has been released before calling hw_sys_hw_bsr_unlock()
         */

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        if ((hw_sys_sw_bsr[periph_id] == SW_BSR_MASTER_NONE) || (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id)) {
                /* Update SW BSR */
                hw_sys_sw_bsr[periph_id] = sw_bsr_master_id;
                hw_sys_sw_bsr_cnt[periph_id]++;
                acquired = true;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

bool hw_sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;
        bool acquired = false;

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        if (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id) {
                acquired = true;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

void hw_sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        ASSERT_ERROR (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id);
        ASSERT_ERROR (hw_sys_sw_bsr_cnt[periph_id]);

        if (--hw_sys_sw_bsr_cnt[periph_id] == 0) {
                hw_sys_sw_bsr[periph_id] = SW_BSR_MASTER_NONE;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);
}

__RETAINED_CODE void hw_sys_pd_com_enable(void)
{
        GLOBAL_INT_DISABLE();
        ASSERT_ERROR((!hw_sys_pd_com_acquire_cnt) || !REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));
        ASSERT_ERROR((hw_sys_pd_com_acquire_cnt) || REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));
        if (++hw_sys_pd_com_acquire_cnt == 1) {
                hw_pd_power_up_com();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_com_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_com_acquire_cnt);
        if (--hw_sys_pd_com_acquire_cnt == 0) {
                hw_pd_power_down_com();
        }
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void hw_sys_pd_periph_enable(void)
{
        GLOBAL_INT_DISABLE();
        if (++hw_sys_pd_periph_acquire_cnt == 1) {
                hw_pd_power_up_periph();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_periph_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_periph_acquire_cnt);
        if (--hw_sys_pd_periph_acquire_cnt == 0) {
                hw_pd_power_down_periph();
        }
        GLOBAL_INT_RESTORE();
}

uint32_t hw_sys_reg_add_config(hw_sys_reg_config_t *config, uint32_t num_of_entries)
{
       ASSERT_ERROR(hw_sys_reg_num_of_config_entries + num_of_entries <= NUM_OF_REG_CONFIG_ENTRIES);

       uint32_t ret = hw_sys_reg_num_of_config_entries;

       for (uint32_t i = 0; i < num_of_entries; i++) {
               hw_sys_reg_config[hw_sys_reg_num_of_config_entries + i] = config[i];
       }
       hw_sys_reg_num_of_config_entries += num_of_entries;

       return ret;
}

hw_sys_reg_config_t *hw_sys_reg_get_config(uint32_t index)
{
        ASSERT_WARNING(index == 0 || index < hw_sys_reg_num_of_config_entries);

        return &hw_sys_reg_config[index];
}

void hw_sys_reg_modify_config(uint32_t index, __IO uint32_t *addr, uint32_t value)
{
        ASSERT_ERROR(index < hw_sys_reg_num_of_config_entries);

        hw_sys_reg_config[index].value = value;

        // Address must be written after value to prevent race condition with other hosts
        hw_sys_reg_config[index].addr = addr;
}

uint32_t *hw_sys_reg_get_num_of_config_entries(void)
{
        return &hw_sys_reg_num_of_config_entries;
}

__RETAINED_CODE void hw_sys_reg_apply_config(void)
{
        uint32_t *p = (uint32_t *)&hw_sys_reg_config;

        while (p < (uint32_t *)&hw_sys_reg_config + 2 * hw_sys_reg_num_of_config_entries) {
                *(uint32_t *)(*p) = *(p+1);
                p += 2;
        }
}

void hw_sys_apply_default_values(void)
{
        const uint32_t reg_in_cs[NUM_OF_REG_ADDR_IN_CS] = {
                (uint32_t)&CRG_XTAL->CLK_FREQ_TRIM_REG,
                (uint32_t)&CRG_XTAL->XTAL32M_CTRL0_REG
        };

        bool is_reg_trimmed[NUM_OF_REG_ADDR_IN_CS] = {false};

        REG_SETF(CRG_XTAL, XTALRDY_CTRL_REG, XTALRDY_CLK_SEL, 0);

        // Check for plain register entries
        if (sys_tcs_reg_pairs_in_cs(reg_in_cs, NUM_OF_REG_ADDR_IN_CS, is_reg_trimmed)) {
                return;
        }

        if (!is_reg_trimmed[0]) {
                REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, dg_configDEFAULT_CLK_FREQ_TRIM_REG__XTAL32M_TRIM__VALUE);
        }

        if (!is_reg_trimmed[1]) {
                REG_SETF(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_CXCOMP_ENABLE,
                        dg_configDEFAULT_XTAL32M_CTRL0_REG__XTAL32M_CXCOMP_ENABLE__VALUE);
        }
}

#include "hw_gpadc.h"
static void hw_sys_pll_locktime_tune(void)
{
        uint8_t cnt = 0;
        uint16_t corrected_band;
        uint16_t found_band;
        uint16_t tune_voltage_found_band;


        /* GPADC is on the peripherals power domain so
         * PD_PER must be powered up*/
        ASSERT_WARNING(hw_pd_check_periph_status());
        hw_sys_pd_com_enable();

        /* Initialize GPADC */
        gpadc_config cfg = {
                .chopping = true,
                .input_mode = 1,
                .input = 15,
                .sample_time = 2,
                .oversampling = 2
        };

        hw_gpadc_init(&cfg);
        hw_gpadc_set_ldo_constant_current(true);
        hw_gpadc_set_ldo_dynamic_current(true);
        hw_gpadc_set_ldo_delay(0x28);
        hw_gpadc_enable();

        /* LDO_CORE voltage must be set to 1.2V prior to enabling PLL */
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL) == 3);

        /*  Enable DXTAL for the system PLL */
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_DXTAL_SYSPLL_ENABLE);

        /* Prepare 1.2 V testbus for VCO tune voltage measurement */
        *(volatile uint32_t *)0x50020B2C &= ~0x1UL;
        *(volatile uint32_t *)0x50020B2C |= 0x40000UL;
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_TEST_VCTR);

        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE); // Start PLL LDO

        hw_clk_delay_usec(20); // Wait for the XTAL LDO to settle

        ASSERT_ERROR(REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, LDO_PLL_OK));

        /*
         * Start the PLL
         */
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_SEL_MIN_CUR_INT); // Use internal VCO current setting
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN); // Start the SYSPLL

        /* Wait for calibration to finish, but wait no longer than 100 us */

        while (cnt < 25) {
                if (REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_CALIBRATION_END)) {
                        break;
                }
                hw_clk_delay_usec(4);
                cnt++;
        }

        cnt = 0;
        /* Wait for the PLL to lock, but wait no longer than 100 us */
        while (cnt < 25) {
                if (REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE)) {
                        break;
                }
                hw_clk_delay_usec(4);
                cnt++;
        }

        /* Measure VCO tune voltage */
        found_band = REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_BEST_MIN_CUR);
        corrected_band = found_band;

        hw_gpadc_adc_measure();
        tune_voltage_found_band =  hw_gpadc_get_raw_value() >> 6;

        /* Check if VCO tune voltage lies within the low and high levels of the PLL comparators */
        if (tune_voltage_found_band < TUNE_VOLTAGE_LOW_LEVEL) {
                corrected_band = 0x20 | found_band >> 1;
        } else if (tune_voltage_found_band > TUNE_VOLTAGE_HIGH_LEVEL) {
                corrected_band = 0x3F & found_band << 1;
        }

        pll_min_current = corrected_band;

        if (corrected_band != found_band) {
                /* First tell the PLL to get the VCO current from PLL_MIN_CURRENT instead
                 * the internally determined value by the calibration algorithm.
                 *
                 * Then switch to the correct VCO current.
                 */
                REG_CLR_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_SEL_MIN_CUR_INT);
                REG_SETF(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_MIN_CURRENT, corrected_band);
        }

        /* Disable testmode */
        REG_CLR_BIT(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_TEST_VCTR);
        *(volatile uint32_t *)0x50020B2C &= ~0x40000UL;
        *(volatile uint32_t *)0x50020B2C |= 0x1UL;

        /* Turn off PLL. */
        REG_CLR_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN);
        /* LDO PLL disable. */
        REG_CLR_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE);
        /*  Disable DXTAL for the system PLL */
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL0_REG, XTAL32M_DXTAL_SYSPLL_ENABLE);

        hw_sys_pd_com_disable();

        hw_gpadc_reset();
        hw_gpadc_disable();
}

void hw_sys_pll_calculate_min_current(void)
{
        uint32_t* values = NULL;
        uint8_t size = 0;
        uint32_t reg = (uint32_t)&CRG_XTAL->PLL_SYS_CTRL3_REG;

        sys_tcs_get_reg_pairs(SYS_TCS_GROUP_PD_TMR, &values, &size);

        if (values) {
                while (size > 0) {
                        /* If PLL_SYS_CTRL3_REG entry exists in OTP it has already been applied
                        * from sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_TMR) in SystemInit.
                        */
                        if (*values == reg) {
                                pll_min_current = REG_GETF(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_MIN_CURRENT);
                                return;
                        }
                        size-=2;
                        values+=2;
                }
        }

        hw_sys_pll_locktime_tune();
}

void hw_sys_pll_set_min_current(void)
{
        /* PLL must use the VCO current from PLL_MIN_CURRENT instead of the internally
         * determined value by the calibration algorithm. */
        REG_CLR_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_SEL_MIN_CUR_INT);
        REG_SETF(CRG_XTAL, PLL_SYS_CTRL3_REG, PLL_MIN_CURRENT, pll_min_current);
}
#endif /* dg_configUSE_HW_SYS */

/**
\}
\}
\}
*/
