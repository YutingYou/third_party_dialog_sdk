/**
 ****************************************************************************************
 *
 * @file ad_sdadc.c
 *
 * @brief SDADC adapter implementation
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configSDADC_ADAPTER

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "ad_sdadc.h"
#include "interrupts.h"
#include "hw_gpio.h"
#include "hw_sdadc.h"
#include "hw_sys.h"
#include "resmgmt.h"
#include "sdk_defs.h"
#include "sdk_list.h"
#include "sys_bsr.h"
#include "sys_power_mgr.h"

typedef enum {
        AD_SDADC_PAD_LATCHES_OP_ENABLE,
        AD_SDADC_PAD_LATCHES_OP_DISABLE
} AD_SDADC_PAD_LATCHES_OP;

typedef struct ad_sdadc_use_input_t {
        bool                            input0;
        bool                            input1;
} ad_sdadc_use_input_t;

/**
 * \brief SDADC adapter (internal) data
 *
 * Data structure of SDADC controller
 *
 */
typedef struct ad_sdadc_data {
        /**< SDADC controller current configuration */
        ad_sdadc_controller_conf_t      *conf;                  /**< Current SDADC configuration */
        ad_sdadc_driver_conf_t          *current_drv;           /**< Current low-level driver configuration */
        ad_sdadc_user_cb                read_cb;                /**< User function to call after asynchronous read finishes */
        void                            *user_data;             /**< User data for callback */
        ad_sdadc_handle_t               handle;                 /**< The handle for the active controller */
        /**< Internal data */
        OS_MUTEX                        busy;                   /**< Semaphore for thread safety */
        bool                            read_in_progress;       /**< Controller is busy reading */
        ad_sdadc_use_input_t            using;                  /**<Flags indicating if input channels are used */
} ad_sdadc_data;

__RETAINED static ad_sdadc_data dynamic_data;

#define AD_SDADC_HANDLE_IS_INVALID(_h_)      ( _h_ == NULL || dynamic_data.handle != _h_ )

static void ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP pad_latches_op, const ad_sdadc_io_conf_t *io_conf)
{
        if (io_conf == NULL) {
                return;
        }

        if (pad_latches_op == AD_SDADC_PAD_LATCHES_OP_ENABLE) {
                if (dynamic_data.using.input0) {
                        hw_gpio_pad_latch_enable(io_conf->input0.port, io_conf->input0.pin);
                }
                if (dynamic_data.using.input1) {
                        hw_gpio_pad_latch_enable(io_conf->input1.port, io_conf->input1.pin);
                }
        } else {
                if (dynamic_data.using.input0) {
                        hw_gpio_pad_latch_disable(io_conf->input0.port, io_conf->input0.pin);
                }
                if (dynamic_data.using.input1) {
                        hw_gpio_pad_latch_disable(io_conf->input1.port, io_conf->input1.pin);
                }
        }
}

static void ad_sdadc_gpio_io_config(const ad_io_conf_t *gpio, AD_IO_CONF_STATE state)
{
        if (gpio == NULL) {
                return;
        }
        if (state == AD_IO_CONF_ON) {
                if AD_GPIO_PIN_PORT_VALID(gpio) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->on.mode, gpio->on.function);
                }
                if (gpio->on.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        } else {
                if AD_GPIO_PIN_PORT_VALID(gpio) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->off.mode, gpio->off.function);
                }
                if (gpio->off.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        }
}

int ad_sdadc_io_config(HW_SDADC_ID id, const ad_sdadc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        if (id != HW_SDADC) {
                return AD_SDADC_ERROR_ID_INVALID;
        }
        if (io == NULL) {
                return AD_SDADC_ERROR_IO_CONF_INVALID;
        }

        OS_MUTEX_GET(dynamic_data.busy, OS_MUTEX_FOREVER);

        hw_sys_pd_com_enable();
        ad_sdadc_gpio_io_config(&io->input0, state);
        ad_sdadc_gpio_io_config(&io->input1, state);
        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_ENABLE, io);
        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_DISABLE, io);
        hw_sys_pd_com_disable();

        OS_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}

void ad_sdadc_init(void)
{
        dynamic_data.conf = NULL;
        OS_MUTEX_CREATE(dynamic_data.busy);
}

__STATIC_INLINE bool ad_sdadc_acquire(uint32_t timeout)
{
        if (resource_acquire(RES_MASK(RES_ID_SDADC), timeout)) {
                return true;
        }

        OS_ASSERT(0);
        return false;
}

__STATIC_INLINE void ad_sdadc_release(void)
{
        resource_release(RES_MASK(RES_ID_SDADC));
}

static int ad_sdadc_check_apply_controller_conf(const ad_sdadc_controller_conf_t *conf, AD_IO_CONF_STATE onoff, ad_sdadc_use_input_t *input_channels)
{
        if (conf == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        if (conf->drv == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        if (onoff == AD_IO_CONF_OFF && conf->io) {
                /* I/O is set OFF */
                ad_sdadc_io_config(conf->id, conf->io, AD_IO_CONF_OFF);
                return AD_SDADC_ERROR_NONE;
        }

        if (conf->drv->inp != HW_SDADC_INP_VBAT) {
                /* check inp for validity */
                switch (conf->drv->inp) {
                case HW_SDADC_IN_ADC0_P1_09:
                case HW_SDADC_IN_ADC1_P0_25:
                case HW_SDADC_IN_ADC2_P0_08:
                case HW_SDADC_IN_ADC3_P0_09:
                case HW_SDADC_IN_ADC4_P1_14:
                case HW_SDADC_IN_ADC5_P1_20:
                case HW_SDADC_IN_ADC6_P1_21:
                case HW_SDADC_IN_ADC7_P1_22:
                        input_channels->input0 = true;
                        break;
                default:
                        return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
                }

                /* check inn for validity - if 2 pins are used */
                if (conf->drv->input_mode == HW_SDADC_INPUT_MODE_DIFFERENTIAL) {
                        switch (conf->drv->inn) {
                        case HW_SDADC_IN_ADC0_P1_09:
                        case HW_SDADC_IN_ADC1_P0_25:
                        case HW_SDADC_IN_ADC2_P0_08:
                        case HW_SDADC_IN_ADC3_P0_09:
                        case HW_SDADC_IN_ADC4_P1_14:
                        case HW_SDADC_IN_ADC5_P1_20:
                        case HW_SDADC_IN_ADC6_P1_21:
                        case HW_SDADC_IN_ADC7_P1_22:
                                input_channels->input1 = true;
                                break;
                        default:
                                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
                        }
                }
        }

        /* reference voltage validity */
        switch (conf->drv->vref_selection) {
        case HW_SDADC_VREF_INTERNAL:
                /* force the correct value for V reference when internal Vref is used */
                OS_ASSERT(conf->drv->vref_voltage == HW_SDADC_VREF_VOLTAGE_INTERNAL);
                break;
        case HW_SDADC_VREF_EXTERNAL:
                OS_ASSERT(HW_SDADC_VREF_IN_RANGE(conf->drv->vref_voltage));
                break;
        default:
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        hw_sdadc_reset();
        hw_sdadc_configure(conf->drv);

        if (conf->drv->inp == HW_SDADC_INP_VBAT) {
                /* VBAT input does not need I/O configuration */
                return AD_SDADC_ERROR_NONE;
        }

        return ad_sdadc_io_config(HW_SDADC, conf->io, AD_IO_CONF_ON);
}

int ad_sdadc_reconfig(ad_sdadc_handle_t handle, const ad_sdadc_driver_conf_t *drv)
{
        int ret;
        ad_sdadc_controller_conf_t new_controller_conf;

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                OS_ASSERT(0);
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        if (drv == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        OS_MUTEX_GET(dynamic_data.busy, OS_MUTEX_FOREVER);

        if (dynamic_data.conf == NULL) {
                /* use ad_sdadc_open instead */
                ret = AD_SDADC_ERROR_DRIVER_UNINITIALIZED;
                goto out_of_here;
        }

        if (dynamic_data.read_in_progress) {
                ret = AD_SDADC_ERROR_READ_IN_PROGRESS;
                goto out_of_here;
        }

        if (dynamic_data.current_drv->input_mode != drv->input_mode) {
                /* use ad_sdadc_open instead */
                ret = AD_SDADC_ERROR_DRIVER_MODE_INVALID;
                goto out_of_here;
        }

        if (dynamic_data.current_drv->inp != drv->inp ||
            ((dynamic_data.current_drv->input_mode == HW_SDADC_INPUT_MODE_DIFFERENTIAL) && (dynamic_data.current_drv->inn != drv->inn)) ) {
                /* can't change input source with reconfig */
                ret = AD_SDADC_ERROR_DRIVER_INPUT_INVALID;
                goto out_of_here;
        }

        /* keep the same io configuration */
        new_controller_conf.io = dynamic_data.conf->io;
        /* refresh the driver configuration */
        new_controller_conf.drv = drv;

        if ((ret = ad_sdadc_check_apply_controller_conf(&new_controller_conf, AD_IO_CONF_ON, &dynamic_data.using)) < 0) {
                goto out_of_here;
        }
        dynamic_data.current_drv = (ad_sdadc_driver_conf_t *) drv;

        ret = AD_SDADC_ERROR_NONE;

out_of_here:
        OS_MUTEX_PUT(dynamic_data.busy);
        return ret;
}

ad_sdadc_handle_t ad_sdadc_open(const ad_sdadc_controller_conf_t *conf)
{
        if (conf == NULL) {
                return NULL;
        }

        if (dynamic_data.conf != NULL) {
                /* use ad_sdadc_reconfig instead */
                return NULL;
        }

        pm_sleep_mode_request(pm_mode_idle);

        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_SDADC);
        ad_sdadc_acquire(RES_WAIT_FOREVER);

        hw_sys_pd_com_enable();

        if (ad_sdadc_check_apply_controller_conf(conf, AD_IO_CONF_ON, &dynamic_data.using) < 0) {
                ad_sdadc_release();
                hw_sys_pd_com_disable();
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_SDADC);
                pm_sleep_mode_release(pm_mode_idle);
                dynamic_data.using.input0 = false;
                dynamic_data.using.input1 = false;
                return NULL;
        }

        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_ENABLE, conf->io);
        /* The value of the handle has no specific meaning, it's just an arbitrary number. */
        dynamic_data.handle              = (ad_sdadc_handle_t) conf;
        dynamic_data.conf                = (ad_sdadc_controller_conf_t *) conf;
        dynamic_data.current_drv         = (ad_sdadc_driver_conf_t *) conf->drv;
        dynamic_data.read_cb             = NULL;
        dynamic_data.user_data           = NULL;
        dynamic_data.read_in_progress    = false;

        return dynamic_data.handle;
}

int ad_sdadc_close(ad_sdadc_handle_t handle, bool forced)
{
        int off_configuration_valid;

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                OS_ASSERT(0);
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        OS_ENTER_CRITICAL_SECTION();

        if (dynamic_data.read_in_progress) {
                if (forced) {
                        hw_sdadc_clear_interrupt();
                        hw_sdadc_deinit();
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_SDADC_ERROR_READ_IN_PROGRESS;
                }
        }

        off_configuration_valid =
                        ad_sdadc_check_apply_controller_conf(dynamic_data.conf, AD_IO_CONF_OFF, &dynamic_data.using);
        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_DISABLE, dynamic_data.conf->io);
        hw_sdadc_reset();
        hw_sdadc_disable();

        OS_MUTEX keep = dynamic_data.busy;
        memset(&dynamic_data, 0, sizeof(ad_sdadc_data));
        dynamic_data.busy = keep;

        OS_LEAVE_CRITICAL_SECTION();

        hw_sys_pd_com_disable();
        ad_sdadc_release();
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_SDADC);
        pm_sleep_mode_release(pm_mode_idle);

        return off_configuration_valid;
}

static void ad_sdadc_get_voltage_cb(void)
{
        hw_sdadc_clear_interrupt();
        int val = hw_sdadc_result_reg_to_millivolt(dynamic_data.current_drv);
        dynamic_data.read_in_progress = false;
        dynamic_data.read_cb(dynamic_data.user_data, val);
        dynamic_data.read_cb = NULL;
}

int ad_sdadc_read_async(ad_sdadc_handle_t handle, ad_sdadc_user_cb cb, void *user_data)
{
        if (!cb) {
                return AD_SDADC_ERROR_CB_INVALID;
        }

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                OS_ASSERT(0);
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(dynamic_data.busy, OS_MUTEX_FOREVER);

        if (hw_sdadc_in_progress()) {
                OS_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_READ_IN_PROGRESS;
        }

        if (dynamic_data.read_in_progress) {
                OS_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_READ_IN_PROGRESS;
        }
        dynamic_data.read_cb = cb;
        dynamic_data.user_data = user_data;
        dynamic_data.read_in_progress = true;

        hw_sdadc_register_interrupt(ad_sdadc_get_voltage_cb);

        /* Start actual conversion */
        hw_sdadc_start();

        OS_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}


int ad_sdadc_read(ad_sdadc_handle_t handle, int32_t *value)
{
        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                OS_ASSERT(0);
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(dynamic_data.busy, OS_MUTEX_FOREVER);

        if (dynamic_data.read_in_progress) {
                OS_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_READ_IN_PROGRESS;
        }
        dynamic_data.read_in_progress = true;
        hw_sdadc_unregister_interrupt();
        *value = hw_sdadc_get_voltage(dynamic_data.current_drv);
        dynamic_data.read_in_progress = false;

        OS_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}

ADAPTER_INIT(ad_sdadc_adapter, ad_sdadc_init);

#endif /* dg_configSDADC_ADAPTER */
