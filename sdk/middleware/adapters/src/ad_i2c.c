/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup I2C
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_i2c.c
 *
 * @brief I2C device access API implementation
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configI2C_ADAPTER


#include <stdarg.h>
#include "ad_i2c.h"
#include "hw_i2c.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "sys_power_mgr.h"

#include "hw_sys.h"
#include "sdk_list.h"
#include "sys_bsr.h"

typedef enum {
        AD_I2C_PAD_LATCHES_OP_ENABLE,
        AD_I2C_PAD_LATCHES_OP_DISABLE,
} AD_I2C_PAD_LATCHES_OP;

/**
 * \brief I2C adapter (internal) dynamic data
 *
 * Dynamic data structure of I2C controller
 *
 */
typedef struct {
        /**< I2C controller current configuration */
        const ad_i2c_controller_conf_t *conf;
        /**< Internal data */
        OS_TASK owner; /**< The task which opened the controller */
        ad_i2c_driver_conf_t *current_drv;
#if dg_configI2C_ADAPTER_SLAVE_SUPPORT
        i2c_slave_state_data_t slave_data;
#endif /* dg_configI2C_ADAPTER_SLAVE_SUPPORT */
}ad_i2c_dynamic_data_t;

/**
 * \brief I2C adapter (internal) static data
 *
 * Static data structure of I2C controller
 *
 */
typedef struct {
        OS_EVENT event; /**< Semaphore for blocking calls  */
        OS_MUTEX busy; /**< Semaphore for thread safety */
}ad_i2c_static_data_t;

__RETAINED static ad_i2c_static_data_t i2c_static_data;
__RETAINED static ad_i2c_static_data_t i2c2_static_data;
static ad_i2c_dynamic_data_t i2c_dynamic_data;
static ad_i2c_dynamic_data_t i2c2_dynamic_data;

#define AD_I2C_HANDLE_IS_VALID(x) ((((x) == &i2c_dynamic_data) || ((x) == &i2c2_dynamic_data)) && \
                                   (((ad_i2c_dynamic_data_t *)(x))->conf != NULL))

const static ad_i2c_static_data_t* ad_i2c_get_static_data_by_hw_id(HW_I2C_ID id)
{
        return ((id == HW_I2C1) ? &i2c_static_data : &i2c2_static_data);
}

const ad_i2c_dynamic_data_t* ad_i2c_get_handle_by_hw_id(HW_I2C_ID id)
{
        return ((id == HW_I2C1) ? &i2c_dynamic_data : &i2c2_dynamic_data);
}

__RETAINED_RW static ad_i2c_io_conf_t i2c_last_io_config = {
        .scl = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C_SCL,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .sda = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C_SDA,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .voltage_level = HW_GPIO_POWER_V33
};
__RETAINED_RW static ad_i2c_io_conf_t i2c2_last_io_config = {
        .scl = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C2_SCL,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .sda = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C2_SDA,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .voltage_level = HW_GPIO_POWER_V33
};

static void ad_i2c_pad_latches(AD_I2C_PAD_LATCHES_OP pad_latches_op,
                               const ad_i2c_controller_conf_t *conf)
{
        const ad_i2c_io_conf_t *io = conf->io;

        if (pad_latches_op == AD_I2C_PAD_LATCHES_OP_ENABLE) {
                hw_gpio_pad_latch_enable(io->scl.port, io->scl.pin);
                hw_gpio_pad_latch_enable(io->sda.port, io->sda.pin);
        } else {
                hw_gpio_pad_latch_disable(io->scl.port, io->scl.pin);
                hw_gpio_pad_latch_disable(io->sda.port, io->sda.pin);
        }
}

static void ad_i2c_configure_pins(HW_I2C_ID id, const ad_i2c_io_conf_t *config)
{
        HW_GPIO_FUNC scl_function = ((id == HW_I2C1) ? HW_GPIO_FUNC_I2C_SCL :
                                                       HW_GPIO_FUNC_I2C2_SCL);
        HW_GPIO_FUNC sda_function = ((id == HW_I2C1) ? HW_GPIO_FUNC_I2C_SDA :
                                                       HW_GPIO_FUNC_I2C2_SDA);
        /* Configure SCL */
        hw_gpio_set_pin_function(config->scl.port, config->scl.pin, HW_GPIO_MODE_OUTPUT,
                                 scl_function);
        hw_gpio_configure_pin_power(config->scl.port, config->scl.pin, config->voltage_level);
        /* Configure SDA */
        hw_gpio_set_pin_function(config->sda.port, config->sda.pin, HW_GPIO_MODE_OUTPUT,
                                 sda_function);
        hw_gpio_configure_pin_power(config->sda.port, config->sda.pin, config->voltage_level);

}

static void ad_i2c_deconfigure_pin(const ad_io_conf_t *io, HW_GPIO_POWER voltage_level)
{
        hw_gpio_set_pin_function(io->port, io->pin, io->off.mode, io->off.function);
        if (io->off.mode == HW_GPIO_MODE_OUTPUT || io->off.mode == HW_GPIO_MODE_OUTPUT_PUSH_PULL ||
            io->off.mode == HW_GPIO_MODE_OUTPUT_OPEN_DRAIN) {
                if (io->off.high) {
                        hw_gpio_set_active(io->port, io->pin);
                } else {
                        hw_gpio_set_inactive(io->port, io->pin);
                }
        }
        hw_gpio_configure_pin_power(io->port, io->pin, voltage_level);
}

int ad_i2c_io_config(HW_I2C_ID id, const ad_i2c_io_conf_t *io_config, AD_IO_CONF_STATE state)
{
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)ad_i2c_get_handle_by_hw_id(id);
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        hw_sys_pd_com_enable();
        if (state == AD_IO_CONF_ON) {
                ad_i2c_configure_pins(id, io_config);
        } else {
                ad_i2c_deconfigure_pin(&(io_config->scl), io_config->voltage_level);
                ad_i2c_deconfigure_pin(&(io_config->sda), io_config->voltage_level);
        }
        hw_gpio_pad_latch_enable(io_config->scl.port, io_config->scl.pin);
        hw_gpio_pad_latch_disable(io_config->scl.port, io_config->scl.pin);
        hw_gpio_pad_latch_enable(io_config->sda.port, io_config->sda.pin);
        hw_gpio_pad_latch_disable(io_config->sda.port, io_config->sda.pin);
        hw_sys_pd_com_disable();
        OS_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

HW_I2C_ID ad_i2c_get_hw_i2c_id(const ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        return i2c->conf->id;
}

static bool ad_i2c_controller_is_busy(HW_I2C_ID id)
{
        return (hw_i2c_is_occupied(id) || hw_i2c_controler_is_busy(id) ||
                !hw_i2c_is_tx_fifo_empty(id) || hw_i2c_is_rx_fifo_not_empty(id));
}

int ad_i2c_reconfig(ad_i2c_handle_t p, const ad_i2c_driver_conf_t *config)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const HW_I2C_ID id = ad_i2c_get_hw_i2c_id(i2c);
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(id);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c is in use */
        if (ad_i2c_controller_is_busy(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        OS_ASSERT(config);

        /* If I2C driver is already configured, check if the new configuration could be applied */
        if (i2c->current_drv) {
                if (i2c->current_drv->i2c.mode != config->i2c.mode &&
                    i2c->current_drv->dma_channel != config->dma_channel) {
                        ASSERT_WARNING(0);
                        OS_MUTEX_PUT(i2c_static->busy);
                        return AD_I2C_ERROR_DRIVER_CONF_INVALID;
                }
        }

        hw_i2c_init(id, &(config->i2c));

#if dg_configI2C_ADAPTER_SLAVE_SUPPORT
        if (config->i2c.mode == HW_I2C_MODE_SLAVE) {
                if (!(i2c->slave_data.state & AD_I2C_SLAVE_STATE_INIT)) {
                        goto after_enabling;
                }
        }
        hw_i2c_enable(id);
after_enabling:
#else
        hw_i2c_enable(id);
#endif
        hw_i2c_reset_abort_source(id);
        hw_i2c_reset_int_all(id);
        i2c->current_drv = (ad_i2c_driver_conf_t *)config;
        OS_MUTEX_PUT(i2c_static->busy);

        return AD_I2C_ERROR_NONE;
}

__STATIC_INLINE resource_mask_t dma_resource_mask(HW_DMA_CHANNEL num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0), RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2), RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4), RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6), RES_MASK(RES_ID_DMA_CH7)
        };

        return res_mask[num];
}

void ad_i2c_bus_acquire(HW_I2C_ID bus_id, HW_DMA_CHANNEL dma_channel)
{
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : RES_ID_I2C2;

        resource_acquire(RES_MASK(res_id), RES_WAIT_FOREVER);

        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                ASSERT_WARNING(dma_channel < HW_DMA_CHANNEL_INVALID - 1);
                resource_acquire(dma_resource_mask(dma_channel) |
                                 dma_resource_mask(dma_channel + 1), RES_WAIT_FOREVER);
        }
}

void ad_i2c_bus_release(HW_I2C_ID bus_id, HW_DMA_CHANNEL dma_channel)
{
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : RES_ID_I2C2;

        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                ASSERT_WARNING(dma_channel < HW_DMA_CHANNEL_INVALID - 1);
                resource_release(dma_resource_mask(dma_channel) |
                                 dma_resource_mask(dma_channel + 1));
        }
        resource_release(RES_MASK(res_id));
}

typedef struct {
        ad_i2c_dynamic_data_t *i2c;
        uint16_t abort_source;
        bool success;
} i2c_cb_data_t;

static void ad_i2c_transaction_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        i2c_cb_data_t *data = (i2c_cb_data_t *) cb_data;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(id);
        uint16_t abort_source = hw_i2c_get_abort_source(id);

        if (!success && (abort_source == HW_I2C_ABORT_NONE)) {
                abort_source = HW_I2C_ABORT_SW_ERROR;
        }
        data->success = success;
        data->abort_source = abort_source;
        if (in_interrupt()) {
                OS_EVENT_SIGNAL_FROM_ISR(i2c_static->event);
        } else {
                OS_EVENT_SIGNAL(i2c_static->event);
        }
}

int ad_i2c_write(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;

        ASSERT_WARNING( ( condition_flags &
                         ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }
        if (condition_flags & HW_I2C_F_ADD_STOP) {
                condition_flags |= HW_I2C_F_WAIT_FOR_STOP;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        hw_i2c_write_buffer_async(i2c->conf->id, wbuf, wlen, ad_i2c_transaction_cb,
                                  (void *)&transaction_data, condition_flags);

        OS_EVENT_WAIT(i2c_static->event, OS_EVENT_FOREVER);
        OS_MUTEX_PUT(i2c_static->busy);

        return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
}

int ad_i2c_read(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;
        ASSERT_WARNING( (condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID || rlen <= 1) {
                hw_i2c_read_buffer_async(i2c->conf->id, rbuf, rlen, ad_i2c_transaction_cb,
                                         (void *)&transaction_data, condition_flags);
        } else {
                hw_i2c_read_buffer_dma(i2c->conf->id, i2c->conf->drv->dma_channel,
                                       rbuf, rlen, ad_i2c_transaction_cb, (void *)&transaction_data,
                                       condition_flags);
        }

        OS_EVENT_WAIT(i2c_static->event, OS_EVENT_FOREVER);
        OS_MUTEX_PUT(i2c_static->busy);

        return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
}

int ad_i2c_write_read(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;

        ASSERT_WARNING( ( condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        hw_i2c_write_then_read_async(i2c->conf->id, wbuf, wlen, rbuf, rlen, ad_i2c_transaction_cb,
                                     (void *)&transaction_data, condition_flags);

        OS_EVENT_WAIT(i2c_static->event, OS_EVENT_FOREVER);
        OS_MUTEX_PUT(i2c_static->busy);

        return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
}


#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
/* define maximum number of supported independent i2c devices */
#define I2C_TRANSACT_DEV_NUM 2

typedef struct {
        ad_i2c_dynamic_data_t *i2c;
        void *app_user_data;
        ad_i2c_user_cb app_cb;
} I2C_TRANSACT_TYPE;

static I2C_TRANSACT_TYPE i2c_transact_data[I2C_TRANSACT_DEV_NUM];

static void ad_i2c_async_cb(HW_I2C_ID id, void *user_data, uint16_t transferred, bool success)
{
        I2C_TRANSACT_TYPE *cb_transact_data = user_data;

        cb_transact_data->i2c = NULL;

        if (success) {
                cb_transact_data->app_cb(cb_transact_data->app_user_data, HW_I2C_ABORT_NONE);
        } else {
                cb_transact_data->app_cb(cb_transact_data->app_user_data, HW_I2C_ABORT_SW_ERROR);
        }
}

static I2C_TRANSACT_TYPE *get_transact_data(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        uint8_t idx = 0;

        while (idx < I2C_TRANSACT_DEV_NUM) {
                if (i2c_transact_data[idx].i2c == NULL) {
                        return &i2c_transact_data[idx];
                }

                if (i2c_transact_data[idx].i2c == i2c) {
                        return NULL;
                }

                idx++;
        }

        return NULL;
}

int ad_i2c_write_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen,
                       ad_i2c_user_cb cb, void *user_data, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( ( condition_flags &
                         ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        if (condition_flags & HW_I2C_F_ADD_STOP) {
                condition_flags |= HW_I2C_F_WAIT_FOR_STOP;
        }

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

        hw_i2c_write_buffer_async(i2c->conf->id, wbuf, wlen, ad_i2c_async_cb, cb_transact_data,
                                  condition_flags);
        OS_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_read_async(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, ad_i2c_user_cb cb,
                      void *user_data, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( ( condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

        if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID || rlen <= 1) {
                hw_i2c_read_buffer_async(i2c->conf->id, rbuf, rlen, ad_i2c_async_cb,
                                         cb_transact_data, condition_flags);
        } else {
                hw_i2c_read_buffer_dma(i2c->conf->id, i2c->conf->drv->dma_channel, rbuf,
                                       rlen, ad_i2c_async_cb, cb_transact_data, condition_flags);
        }
        OS_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_write_read_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen,
                            uint8_t *rbuf, size_t rlen, ad_i2c_user_cb cb, void *user_data,
                            uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( (condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                OS_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

        hw_i2c_write_then_read_async(i2c->conf->id, wbuf, wlen, rbuf, rlen,
                                     ad_i2c_async_cb, (void *)cb_transact_data, condition_flags);
        OS_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}
#endif /* CONFIG_I2C_USE_ASYNC_TRANSACTIONS */

#if dg_configI2C_ADAPTER_SLAVE_SUPPORT

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event);

static void ad_i2c_slave_sent_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)cb_data;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        slave->state &= ~AD_I2C_SLAVE_STATE_WRITE_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_READ_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks && NULL != slave->event_callbacks->data_sent) {
                slave->event_callbacks->data_sent((ad_i2c_dynamic_data_t *) i2c, len, success,
                                                  slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void ad_i2c_slave_received_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *) cb_data;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        slave->state &= ~AD_I2C_SLAVE_STATE_READ_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_WRITE_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks &&
                NULL != slave->event_callbacks->data_received) {
                slave->event_callbacks->data_received((ad_i2c_handle_t)i2c, len, success,
                                                      slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void i2c_slave_send(HW_I2C_ID id, ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        /*
         * Master initiated read, if user already prepared buffer, send it.
         * If not notify user that read request is pending.
         */
        if (slave->output_buffer && slave->output_buffer_len) {
                hw_i2c_write_buffer_async(id, slave->output_buffer, slave->output_buffer_len,
                                          ad_i2c_slave_sent_cb, i2c, 0);
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->read_request != NULL) {
                        slave->event_callbacks->read_request(i2c, slave->user_data);
                }
        }
}

/*
 * Master initiated write, if user already prepared buffer, use it.
 * If not notify user that master started write.
 */
static void i2c_slave_receive(HW_I2C_ID id, ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        if (slave->input_buffer && slave->input_buffer_len) {
                if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID ||
                    slave->input_buffer_len <= 1) {
                        hw_i2c_read_buffer_async(id, slave->input_buffer, slave->input_buffer_len,
                                                 ad_i2c_slave_received_cb, i2c, 0);
                } else {
                        hw_i2c_register_slave_dma_read_callback(id);
                }
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->data_ready != NULL) {
                        slave->event_callbacks->data_ready((ad_i2c_handle_t)i2c, slave->user_data);
                }
        }
}

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event)
{
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)ad_i2c_get_handle_by_hw_id(id);

        switch (event) {
        case HW_I2C_EVENT_READ_REQUEST:
                i2c_slave_send(id, i2c);
                break;
        case HW_I2C_EVENT_DATA_READY:
                i2c_slave_receive(id, i2c);
                break;
        case HW_I2C_EVENT_TX_ABORT:
        case HW_I2C_EVENT_RX_OVERFLOW:
        case HW_I2C_EVENT_INVALID:
                break;
        default:
                ASSERT_WARNING(0);
        }
}

int ad_i2c_start_slave(ad_i2c_handle_t p, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata,
                       uint16_t rlen, const i2c_dev_slave_event_callbacks_t *events,
                       void *user_data)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        slave->state = AD_I2C_SLAVE_STATE_INIT;

        /* Enable I2C controller */
        hw_i2c_enable(i2c->conf->id);

        slave->event_callbacks = events;
        slave->user_data = user_data;
        slave->output_buffer = wdata;
        slave->output_buffer_len = wlen;
        slave->input_buffer = rdata;
        slave->input_buffer_len = rlen;
        if (wdata != NULL && wlen) {
                slave->state |= AD_I2C_SLAVE_STATE_WRITE_PENDING;
        }
        if (rdata != NULL && rlen > 0) {
                slave->state |= AD_I2C_SLAVE_STATE_READ_PENDING;
                if (i2c->conf->drv->dma_channel < HW_DMA_CHANNEL_INVALID) {
                        /* When DMA is used for the Rx it is better to set it up here (rather than
                         * in the read request handler) so that the slave is more responsive. */
                        hw_i2c_prepare_dma(i2c->conf->id, i2c->conf->drv->dma_channel,
                                           (uint16_t *) rdata, rlen, HW_I2C_DMA_TRANSFER_SLAVE_READ,
                                           ad_i2c_slave_received_cb, i2c, HW_I2C_F_NONE);
                        hw_i2c_dma_start(i2c->conf->id);
                }
        }
        hw_i2c_set_slave_callback(i2c->conf->id, ad_i2c_slave_cb);
        OS_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_stop_slave(ad_i2c_handle_t p)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);

        OS_EVENT_CREATE(slave->operation_done_event);

        if (hw_i2c_is_slave_busy(i2c->conf->id)) {
                OS_EVENT_WAIT(slave->operation_done_event, OS_EVENT_FOREVER);
                /* Wait for flushing TX fifo */
                while (!hw_i2c_is_tx_fifo_empty(i2c->conf->id));
                while (hw_i2c_is_slave_busy(i2c->conf->id));
        }

        OS_EVENT_DELETE(slave->operation_done_event);
        slave->event_callbacks = NULL;
        slave->user_data = NULL;
        slave->output_buffer = NULL;
        slave->output_buffer_len = 0;
        slave->input_buffer = NULL;
        slave->input_buffer_len = 0;
        slave->state = AD_I2C_SLAVE_STATE_STOPPED;
        slave->operation_done_event = NULL;
        hw_i2c_set_slave_callback(i2c->conf->id, NULL);

        hw_i2c_disable(i2c->conf->id);

        OS_MUTEX_PUT(i2c_static->busy);

        return AD_I2C_ERROR_NONE;
}

void ad_i2c_clear_read_slave(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        while (hw_i2c_is_rx_fifo_not_empty(i2c->conf->id)) {
                hw_i2c_read_byte(i2c->conf->id);
        }
        OS_MUTEX_PUT(i2c_static->busy);
}


#endif /* dg_configI2C_ADAPTER_SLAVE_SUPPORT */

void ad_i2c_wait_while_master_busy(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        OS_MUTEX_GET(i2c_static->busy, OS_MUTEX_FOREVER);
        while (hw_i2c_is_master_busy(i2c->conf->id));
        OS_MUTEX_PUT(i2c_static->busy);
}

ad_i2c_handle_t ad_i2c_open(const ad_i2c_controller_conf_t *ctrl_config)
{
        OS_ASSERT(ctrl_config);
        ad_i2c_dynamic_data_t *i2c = ((ctrl_config->id == HW_I2C1) ? &i2c_dynamic_data : &i2c2_dynamic_data);
        ASSERT_WARNING(i2c);
        pm_sleep_mode_request(pm_mode_idle);

        ad_i2c_io_conf_t *last = ((ctrl_config->id == HW_I2C1) ? &i2c_last_io_config :
                                                                 &i2c2_last_io_config);
        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU,
                           ctrl_config->id == HW_I2C1 ? BSR_PERIPH_ID_I2C1 : BSR_PERIPH_ID_I2C2);

        ASSERT_WARNING(ctrl_config->drv);
        ad_i2c_bus_acquire(ctrl_config->id, ctrl_config->drv->dma_channel);

        ASSERT_WARNING(ctrl_config->io);
        hw_sys_pd_com_enable();
        ad_i2c_configure_pins(ctrl_config->id, ctrl_config->io);
#if (dg_configUSE_HW_SENSOR_NODE && dg_configUSE_SNC_HW_I2C)
        void snc_i2c_sync_with_snc_last_pin_conf(HW_I2C_ID id, ad_i2c_io_conf_t* last_io_cfg);
        snc_i2c_sync_with_snc_last_pin_conf(ctrl_config->id, last);
#endif /* dg_configUSE_HW_SENSOR_NODE && dg_configUSE_SNC_HW_I2C */
        if (last->scl.port < HW_GPIO_PORT_MAX && last->scl.pin < HW_GPIO_PIN_MAX &&
            last->sda.port < HW_GPIO_PORT_MAX && last->sda.pin < HW_GPIO_PIN_MAX) {
                if (last->scl.port != ctrl_config->io->scl.port ||
                    last->scl.pin != ctrl_config->io->scl.pin) {
                        ad_i2c_deconfigure_pin(&(last->scl), last->voltage_level);
                        hw_gpio_pad_latch_enable(last->scl.port, last->scl.pin);
                        hw_gpio_pad_latch_disable(last->scl.port, last->scl.pin);
                }
                if (last->sda.port != ctrl_config->io->sda.port ||
                    last->sda.pin != ctrl_config->io->sda.pin) {
                        ad_i2c_deconfigure_pin(&(last->sda), last->voltage_level);
                        hw_gpio_pad_latch_enable(last->sda.port, last->sda.pin);
                        hw_gpio_pad_latch_disable(last->sda.port, last->sda.pin);
                }
        }
        ad_i2c_pad_latches(AD_I2C_PAD_LATCHES_OP_ENABLE, ctrl_config);

        i2c->conf = ctrl_config;
        ASSERT_WARNING(ctrl_config->drv);
        /* Configure I2C controller driver */
        if (ad_i2c_reconfig(i2c, ctrl_config->drv) < 0) {
                *last = *(i2c->conf->io);
                ad_i2c_pad_latches(AD_I2C_PAD_LATCHES_OP_DISABLE, i2c->conf);
                hw_sys_pd_com_disable();
                /* Driver configuration failed, release bus and */
                ad_i2c_bus_release(ctrl_config->id, ctrl_config->drv->dma_channel);
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU,
                                   ctrl_config->id == HW_I2C1 ? BSR_PERIPH_ID_I2C1 :
                                                                BSR_PERIPH_ID_I2C2);
                i2c->conf = NULL;
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }
        i2c->current_drv = (ad_i2c_driver_conf_t *)ctrl_config->drv;
        i2c->owner = OS_GET_CURRENT_TASK();

        return i2c;
}

int ad_i2c_close(ad_i2c_handle_t p, bool force)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        HW_I2C_ID id = i2c->conf->id;
        HW_DMA_CHANNEL dma_channel = i2c->conf->drv->dma_channel;

        OS_ENTER_CRITICAL_SECTION();
        if (!force) {
                if (ad_i2c_controller_is_busy(i2c->conf->id)) {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_I2C_ERROR_CONTROLLER_BUSY;
                }
        } else {
                /* Abort ongoing transactions */
                if (hw_i2c_is_master(i2c->conf->id) && ad_i2c_controller_is_busy(i2c->conf->id)) {
                        hw_i2c_master_abort_transfer(id);
                        while (HW_I2C_REG_GETF(id, I2C_ENABLE, I2C_ABORT));
                }
                hw_i2c_unregister_int(id);
        }

        ad_i2c_io_conf_t *last_io = ((id == HW_I2C1) ? &i2c_last_io_config : &i2c2_last_io_config);
        /* Wait until any ongoing transaction is finished */
        hw_i2c_deinit(id);
        if (i2c->conf->drv->dma_channel < HW_DMA_CHANNEL_INVALID - 1) {
                hw_dma_channel_stop(i2c->conf->drv->dma_channel);
                hw_dma_channel_stop(i2c->conf->drv->dma_channel + 1);
        }
        /* Don't deconfigure pins, just keep the last configuration */
        *last_io = *(i2c->conf->io);
#if (dg_configUSE_HW_SENSOR_NODE && dg_configUSE_SNC_HW_I2C)
        void snc_i2c_update_snc_last_pin_conf(HW_I2C_ID id, ad_i2c_io_conf_t* last_io_cfg);
        snc_i2c_update_snc_last_pin_conf(id, last_io);
#endif
        ad_i2c_pad_latches(AD_I2C_PAD_LATCHES_OP_DISABLE, i2c->conf);
        hw_sys_pd_com_disable();
        i2c->owner = NULL;
        i2c->current_drv = NULL;
        i2c->conf = NULL;

        OS_LEAVE_CRITICAL_SECTION();

        ad_i2c_bus_release(id, dma_channel);
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, id == HW_I2C1 ? BSR_PERIPH_ID_I2C1 :
                                                                 BSR_PERIPH_ID_I2C2);
        pm_sleep_mode_release(pm_mode_idle);

        return AD_I2C_ERROR_NONE;
}

void ad_i2c_init(void)
{
        OS_MUTEX_CREATE(i2c_static_data.busy);
        OS_EVENT_CREATE(i2c_static_data.event);
        OS_MUTEX_CREATE(i2c2_static_data.busy);
        OS_EVENT_CREATE(i2c2_static_data.event);
}

ADAPTER_INIT(ad_i2c_adapter, ad_i2c_init);

#endif

/**
 * \}
 * \}
 * \}
 */
