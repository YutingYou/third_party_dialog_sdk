/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup GPADC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.c
 *
 * @brief GPADC adapter implementation
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configGPADC_ADAPTER == 1)

#include <stdarg.h>
#include <stdint.h>
#include "ad_gpadc.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "hw_gpadc.h"
#include "sdk_defs.h"
#include "hw_gpio.h"
#include "hw_sys.h"
#include "sys_bsr.h"

#define AD_GPADC_ASSERT_HANDLE_VALID(__handle)                          \
        OS_ASSERT(__handle == dynamic_data.handle && __handle != NULL); \
        if (__handle != dynamic_data.handle || __handle == NULL) {      \
                return AD_GPADC_ERROR_HANDLE_INVALID;                   \
        }

typedef enum {
        AD_GPADC_PAD_LATCHES_OP_ENABLE,
        AD_GPADC_PAD_LATCHES_OP_DISABLE,
} AD_GPADC_PAD_LATCHES_OP;

/**
 * \brief GPADC adapter (internal) data
 *
 * Data structure of GPADC controller
 *
 */
typedef struct ad_gpadc_data {
        /**< GPADC controller current configuration */
        ad_gpadc_controller_conf_t      *conf;                  /**< Current GPADC configuration */
        ad_gpadc_user_cb                read_cb;                /**< User function to call after asynchronous read finishes */
        void                            *user_data;             /**< User data for callback */
        ad_gpadc_handle_t               handle;                 /**< The handle for the active controller */
        /**< Internal data */
        OS_TASK                         owner;                  /**< The task which opened the controller */
        OS_EVENT                        event;                  /**< Semaphore for thread safety */
        OS_MUTEX                        busy;                   /**< Semaphore for thread safety */
        int8_t                          gpadc_acquire_count;    /**< This keeps track of number of calls to ad_gpadc_acquire() */
        bool                            read_in_progress;       /**< Number of source_acquire calls */
        bool                            latch_input0;           /**< flag to indicate if input 0 needs latching */
        bool                            latch_input1;           /**< flag to indicate if input 1 needs latching */
} ad_gpadc_data;

__RETAINED static ad_gpadc_data dynamic_data;

#include <sys_power_mgr.h>

static void ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP pad_latches_op, const ad_gpadc_io_conf_t * io, bool use_mutex)
{
        if (io == NULL) {
                return;
        }

        if (use_mutex) {
                OS_MUTEX_GET(dynamic_data.busy, OS_MUTEX_FOREVER);
        }

        if (pad_latches_op == AD_GPADC_PAD_LATCHES_OP_ENABLE) {
                if (dynamic_data.latch_input0) {
                        hw_gpio_pad_latch_enable(io->input0.port, io->input0.pin);
                }

                if (dynamic_data.latch_input1) {
                        hw_gpio_pad_latch_enable(io->input1.port, io->input1.pin);
                }
        } else {
                if (dynamic_data.latch_input0) {
                        hw_gpio_pad_latch_disable(io->input0.port, io->input0.pin);
                }

                if (dynamic_data.latch_input1) {
                        hw_gpio_pad_latch_disable(io->input1.port, io->input1.pin);
                }
        }

        if (use_mutex) {
                OS_MUTEX_PUT(dynamic_data.busy);
        }
}

static void ad_gpadc_gpio_io_config (const ad_io_conf_t * gpio, AD_IO_CONF_STATE state)
{
        if (gpio == NULL) {
                return;
        }

        if (state == AD_IO_CONF_ON) {
                if (gpio->pin != HW_GPIO_PIN_NONE && gpio->port != HW_GPIO_PORT_NONE) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->on.mode, gpio->on.function);
                }

                if (gpio->on.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        } else {
                if (gpio->pin != HW_GPIO_PIN_NONE && gpio->port != HW_GPIO_PORT_NONE) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->off.mode, gpio->off.function);
                }

                if (gpio->off.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        }
}

int ad_gpadc_io_config (const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        if (io == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        hw_sys_pd_com_enable();
        ad_gpadc_gpio_io_config(&io->input0, state);
        ad_gpadc_gpio_io_config(&io->input1, state);
        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_ENABLE, io, true);
        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_DISABLE, io, true);
        hw_sys_pd_com_disable();
        return AD_GPADC_ERROR_NONE;
}

void ad_gpadc_init(void)
{
        dynamic_data.conf = NULL;
        OS_EVENT_CREATE(dynamic_data.event);
        OS_MUTEX_CREATE(dynamic_data.busy);
}


static void ad_gpadc_acquire(void)
{
        bool acquired __UNUSED;

        acquired = ad_gpadc_acquire_to(RES_WAIT_FOREVER);
        OS_ASSERT(acquired);
}

bool ad_gpadc_acquire_to(uint32_t timeout)
{
        uint32_t ret = false;

        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (current_task == dynamic_data.owner) {
                dynamic_data.gpadc_acquire_count++;
                return true;
        }

        if (resource_acquire(RES_MASK(RES_ID_GPADC), timeout)) {
                dynamic_data.owner = current_task;
                dynamic_data.gpadc_acquire_count++;
                ret = true;
        }

        return ret;
}

static void ad_gpadc_release(void)
{
        /* Make sure that gpadc is acquired at least once before attempting to release it */
        OS_ASSERT(dynamic_data.gpadc_acquire_count > 0);

        if (--dynamic_data.gpadc_acquire_count == 0) {
                /* A device release can only happen from the same task that owns it, or from an ISR */
                OS_ASSERT(in_interrupt() || (OS_GET_CURRENT_TASK() == dynamic_data.owner));
                dynamic_data.owner = NULL;
                resource_release(RES_MASK(RES_ID_GPADC));
        }
}

static int ad_gpadc_check_and_apply_config(const ad_gpadc_controller_conf_t *conf, AD_IO_CONF_STATE onoff)
{
        if (!conf) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        if (onoff == AD_IO_CONF_ON) {
                if (conf->drv->input_mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                        switch (conf->drv->input) {
                        case HW_GPADC_INPUT_SE_P1_09:
                        case HW_GPADC_INPUT_SE_P0_25:
                        case HW_GPADC_INPUT_SE_P0_08:
                        case HW_GPADC_INPUT_SE_P0_09:
                        case HW_GPADC_INPUT_SE_P1_13:
                        case HW_GPADC_INPUT_SE_P1_12:
                        case HW_GPADC_INPUT_SE_P1_18:
                        case HW_GPADC_INPUT_SE_P1_19:
                                if (!conf->io) {
                                        return AD_GPADC_ERROR_CONFIG_INVALID;
                                }
                                dynamic_data.latch_input0 = true;
                                dynamic_data.latch_input1 = false;
                                ad_gpadc_gpio_io_config(&conf->io->input0, onoff);
                                break;
                        case HW_GPADC_INPUT_SE_VDD:
                        case HW_GPADC_INPUT_SE_V30_1:
                        case HW_GPADC_INPUT_SE_V30_2:
                        case HW_GPADC_INPUT_SE_VSSA:
                        case HW_GPADC_INPUT_SE_VBAT:
                        case HW_GPADC_INPUT_SE_TEMPSENS:
                                dynamic_data.latch_input0 = false;
                                dynamic_data.latch_input1 = false;
                                break;
                        default:
                                return AD_GPADC_ERROR_CONFIG_INVALID;
                        }
                } else { /* HW_GPADC_INPUT_MODE_DIFFERENTIAL - all combinations are valid */
                        if (!conf->io) {
                                return AD_GPADC_ERROR_CONFIG_INVALID;
                        }
                        dynamic_data.latch_input0 = true;
                        dynamic_data.latch_input1 = true;
                        ad_gpadc_gpio_io_config(&conf->io->input0, onoff);
                        ad_gpadc_gpio_io_config(&conf->io->input1, onoff);
                }
        }

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_reconfig(ad_gpadc_handle_t handle, const ad_gpadc_driver_conf_t *drv)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        if (drv == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        if (dynamic_data.conf == NULL) {
                //use ad_gpadc_open instead
                return AD_GPADC_ERROR_ADAPTER_NOT_OPEN;
        }

        if (dynamic_data.conf->drv->input != drv->input) {
                //can't change input with reconfig
                return AD_GPADC_ERROR_CHANGE_NOT_ALLOWED;
        }

        if (dynamic_data.read_in_progress) {
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        ad_gpadc_acquire();
        dynamic_data.conf->drv = (ad_gpadc_driver_conf_t *) drv;

        hw_gpadc_reset();
        hw_gpadc_configure(dynamic_data.conf->drv);

        ad_gpadc_release();

        return AD_GPADC_ERROR_NONE;
}

ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE) ||
                REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE));
        if (conf == NULL) {
                return NULL;
        }

        /* The driver configuration is mandatory */
        if (conf->drv == NULL) {
                return NULL;
        }

        pm_sleep_mode_request(pm_mode_idle);

        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_GPADC);

        ad_gpadc_acquire();

        if (dynamic_data.conf != NULL) {
                //use ad_gpadc_reconfig instead
                ad_gpadc_release();
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_GPADC);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }
        hw_sys_pd_periph_enable();
        hw_sys_pd_com_enable();

        if (ad_gpadc_check_and_apply_config(conf, AD_IO_CONF_ON) < 0) {
                ad_gpadc_release();
                hw_sys_pd_com_disable();
                hw_sys_pd_periph_disable();
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_GPADC);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        hw_gpadc_reset();
        hw_gpadc_configure((gpadc_config*) conf->drv);

        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_ENABLE, conf->io, true);

        dynamic_data.conf        = (ad_gpadc_controller_conf_t *) conf;
        dynamic_data.conf->io    = (ad_gpadc_io_conf_t *) conf->io;
        dynamic_data.conf->drv   = (ad_gpadc_driver_conf_t *) conf->drv;
        dynamic_data.handle      = (ad_gpadc_handle_t *) conf;

        return dynamic_data.handle;
}

int ad_gpadc_close(ad_gpadc_handle_t handle, bool force)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        OS_ENTER_CRITICAL_SECTION();
        if (dynamic_data.read_in_progress) {
                if (force) {
                        hw_gpadc_unregister_interrupt();
                        hw_gpadc_clear_interrupt();
                        dynamic_data.read_in_progress = false;
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
                }
        }
        OS_LEAVE_CRITICAL_SECTION();

        hw_gpadc_reset();
        hw_gpadc_disable();
        ad_gpadc_check_and_apply_config(dynamic_data.conf, AD_IO_CONF_OFF);

        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_DISABLE, (const ad_gpadc_io_conf_t *) dynamic_data.conf->io, true);

        dynamic_data.conf = NULL;
        dynamic_data.handle = NULL;

        hw_sys_pd_com_disable();
        hw_sys_pd_periph_disable();

        ad_gpadc_release();

        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_GPADC);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_GPADC_ERROR_NONE;
}

static void ad_gpadc_cb(void)
{
        int val;

        if (!dynamic_data.read_in_progress) {
                return;
        }

        val = hw_gpadc_get_value();

        hw_gpadc_clear_interrupt();

        dynamic_data.read_in_progress = false;

        ad_gpadc_release();

        dynamic_data.read_cb(dynamic_data.user_data, val);
}

int ad_gpadc_read_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        if (!read_async_cb) {
                return AD_GPADC_ERROR_OTHER;
        }

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ad_gpadc_acquire();

        if (dynamic_data.read_in_progress) {
                ad_gpadc_release();
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.read_in_progress = true;


        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        hw_gpadc_register_interrupt(ad_gpadc_cb);

        /* Start actual conversion */
        hw_gpadc_start();

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_read(ad_gpadc_handle_t handle, uint16_t *value)
{
        bool acquired __UNUSED;
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_to(dynamic_data.conf, value, RES_WAIT_FOREVER);
        OS_ASSERT(ret == AD_GPADC_ERROR_NONE);

        return ret;
}

int ad_gpadc_read_raw(ad_gpadc_handle_t handle, uint16_t *value)
{
        bool acquired __UNUSED;
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_raw_to(dynamic_data.conf, value, RES_WAIT_FOREVER);
        OS_ASSERT(ret == AD_GPADC_ERROR_NONE);

        return ret;
}

static int ad_gpadc_read_internal_to(ad_gpadc_controller_conf_t *conf, uint16_t *value,
        uint32_t timeout, uint16_t (*hw_gpadc_measurement_func)(void))
{
        ASSERT_WARNING(hw_gpadc_measurement_func);

        int ret = AD_GPADC_ERROR_TIMEOUT;

        if (ad_gpadc_acquire_to(timeout)) {
                hw_gpadc_unregister_interrupt();

                hw_gpadc_adc_measure();

                *value = hw_gpadc_measurement_func();
                ad_gpadc_release();
                ret = AD_GPADC_ERROR_NONE;
        }

        return ret;
}

int ad_gpadc_read_raw_to(ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout)
{
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, timeout, hw_gpadc_get_raw_value);

        return ret;
}

int ad_gpadc_read_to(ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout)
{
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, timeout, hw_gpadc_get_value);

        return ret;
}

uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv)
{
        return 0xFFFF >> (6 - MIN(6, drv->oversampling));
}

ADAPTER_INIT(ad_gpadc_adapter, ad_gpadc_init);

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
