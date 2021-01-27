/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SPI_ADAPTER SPI Adapter
 *
 * \brief Adapter for SPI controller
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_spi.c
 *
 * @brief SPI Adapter implementation
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configSPI_ADAPTER

#include <stdint.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "sys_power_mgr.h"
#include "hw_sys.h"
#include "hw_gpio.h"
#include "sdk_list.h"
#include "sys_bsr.h"


#include <stdarg.h>
#include "ad_spi.h"
#include "hw_spi.h"
#include "platform_devices.h"
#include "resmgmt.h"


typedef struct {
        /**< SPI controller current configuration */
        const ad_spi_controller_conf_t *conf;
        /**< Internal data */
        OS_TASK  owner; /**< The task which opened the controller */
        OS_EVENT event; /**< Semaphore for async calls  */
        OS_MUTEX busy;  /**< Semaphore for thread safety */
} ad_spi_data_t;

__RETAINED static ad_spi_data_t spi1_data;
__RETAINED static ad_spi_data_t spi2_data;

#define AD_SPI_GPIO_VALID(__port, __pin)  ((__port != HW_GPIO_PORT_NONE) && (__pin != HW_GPIO_PIN_NONE))
#define AD_SPI_HANDLE_IS_VALID(__handle) (((__handle == &spi1_data) || (__handle == &spi2_data)) && (((ad_spi_data_t*) __handle)->conf != NULL))

void ad_spi_init(void)
{
        OS_EVENT_CREATE(spi1_data.event);
        OS_MUTEX_CREATE(spi1_data.busy);
        OS_EVENT_CREATE(spi2_data.event);
        OS_MUTEX_CREATE(spi2_data.busy);
}

static int ad_spi_internal_io_config(HW_SPI_ID id, const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state)
{

        uint8 i = 0;
        ad_io_conf_t* gpio = (ad_io_conf_t *) io;

        /* SPI clk should be at least configured*/
        if (!AD_SPI_GPIO_VALID(io->spi_clk.port, io->spi_clk.pin)) {
                return AD_SPI_ERROR_NO_SPI_CLK_PIN;
        }

        for (i = 0; i < 3; i++) {
                if (AD_SPI_GPIO_VALID(gpio->port,gpio->pin)) {
                        hw_gpio_configure_pin_power(gpio->port, gpio->pin, io->voltage_level);
                        if (state == AD_IO_CONF_ON) {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->on.mode,
                                        gpio->on.function, gpio->on.high);
                                hw_gpio_pad_latch_enable(gpio->port, gpio->pin);
                        }
                        else {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->off.mode,
                                        gpio->off.function, gpio->off.high);
                                hw_gpio_pad_latch_disable(gpio->port, gpio->pin);
                        }
                }
                gpio++;
        }

        gpio = (ad_io_conf_t*)io->spi_cs;
        for (i = 0; i < io->cs_cnt; i++) {
                if (AD_SPI_GPIO_VALID(gpio->port,gpio->pin)) {
                        hw_gpio_configure_pin_power(gpio->port, gpio->pin, io->voltage_level);
                        if (state == AD_IO_CONF_ON) {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->on.mode,
                                        gpio->on.function, gpio->on.high);
                                hw_gpio_pad_latch_enable(gpio->port, gpio->pin);
                        }
                        else {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->off.mode,
                                        gpio->off.function, gpio->off.high);
                                hw_gpio_pad_latch_disable(gpio->port, gpio->pin);
                        }
                }
                gpio++;
        }
        return AD_SPI_ERROR_NONE;
}
static void ad_spi_res_acquire(RES_ID id, HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel >=0) {
                resource_acquire(RES_MASK(id) | RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1), RES_WAIT_FOREVER);
        }
        else {
                resource_acquire(RES_MASK(id), RES_WAIT_FOREVER);
        }
}

static void ad_spi_res_release(RES_ID id, HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel >=0) {
                resource_release(RES_MASK(id) | RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1));
        }
        else {
                resource_release(RES_MASK(id));
        }
}

ad_spi_handle_t ad_spi_open(const ad_spi_controller_conf_t *conf)
{
        OS_ASSERT(conf);
        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);
        ad_spi_data_t *spi = ((conf->id == HW_SPI1) ? &spi1_data : &spi2_data);
        OS_ASSERT(spi);

        pm_sleep_mode_request(pm_mode_idle);

        RES_ID res_id = ((conf->id == HW_SPI1) ? RES_ID_SPI1 : RES_ID_SPI2);
        uint8_t dma_channel = conf->drv->spi.rx_dma_channel;
        OS_ASSERT(dma_channel < HW_DMA_CHANNEL_INVALID)

        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU,
                (conf->id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2);

        ad_spi_res_acquire(res_id, dma_channel);

        hw_sys_pd_com_enable();

        if (ad_spi_internal_io_config(conf->id, conf->io, AD_IO_CONF_ON) != AD_SPI_ERROR_NONE) {
                ad_spi_res_release(res_id, dma_channel);
                hw_sys_pd_com_disable();
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU,
                (conf->id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        spi->owner = OS_GET_CURRENT_TASK();

        OS_ENTER_CRITICAL_SECTION();
        hw_spi_enable(conf->id, 1);
        OS_LEAVE_CRITICAL_SECTION();

        spi->conf = conf;

        if (ad_spi_reconfig(spi, conf->drv) != AD_SPI_ERROR_NONE) {
                spi->conf = NULL;
                ad_spi_res_release(res_id, dma_channel);
                hw_sys_pd_com_disable();
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU,
                (conf->id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }
        return spi;
}

int ad_spi_reconfig(ad_spi_handle_t handle, const ad_spi_driver_conf_t *drv_conf)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        OS_ASSERT(drv_conf);
        OS_ASSERT(spi->conf->drv);

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);

        if (hw_spi_is_occupied(spi->conf->id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        if (spi->conf->drv->spi.rx_dma_channel != drv_conf->spi.rx_dma_channel) {
                ASSERT_WARNING(0);
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_CONFIG_DMA_CHANNEL_INVALID;
        }

        if (spi->conf->drv->spi.tx_dma_channel != drv_conf->spi.tx_dma_channel) {
                ASSERT_WARNING(0);
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_CONFIG_DMA_CHANNEL_INVALID;
        }

        if (spi->conf->drv->spi.smn_role != drv_conf->spi.smn_role) {
                ASSERT_WARNING(0);
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_CONFIG_SPI_ROLE_INVALID;
        }

          /* check if the CS pin is the configured one */
        if (spi->conf->drv->spi.smn_role == HW_SPI_MODE_MASTER) {
                uint8_t cs_cnt = spi->conf->io->cs_cnt;
                const ad_io_conf_t* cs_ports = spi->conf->io->spi_cs;
                uint8 cs_configured = 0;
                uint8_t i = 0;

                for (i = 0; i< cs_cnt; i++) {
                        cs_configured = ((cs_ports->port == drv_conf->spi.cs_pad.port) &&
                                (cs_ports->pin == drv_conf->spi.cs_pad.pin));
                        if (cs_configured ) {
                                break;
                        }
                        cs_ports++;
                }

                if (!cs_configured) {
                        ASSERT_WARNING(0);
                        OS_MUTEX_PUT(spi->busy);
                        return AD_SPI_ERROR_CONFIG_SPI_CS_INVALID;
                }
        }

        hw_spi_init(spi->conf->id, &drv_conf->spi);

        OS_MUTEX_PUT(spi->busy);

        return AD_SPI_ERROR_NONE;
}

int ad_spi_close(ad_spi_handle_t handle, bool force)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        HW_SPI_ID id = spi->conf->id;
        RES_ID res_id = ((id == HW_SPI1) ? RES_ID_SPI1 : RES_ID_SPI2);
        int8_t dma_channel = spi->conf->drv->spi.rx_dma_channel;
        /* check for ongoing transactions */

        if (!force && hw_spi_is_occupied(id)) {
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_deinit(id);

        ad_spi_internal_io_config(spi->conf->id,spi->conf->io, AD_IO_CONF_OFF);
        hw_sys_pd_com_disable();
        spi->owner = NULL;
        spi->conf = NULL;

        ad_spi_res_release(res_id, dma_channel);

        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU,
                (id == HW_SPI1) ? BSR_PERIPH_ID_SPI1 : BSR_PERIPH_ID_SPI2);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_SPI_ERROR_NONE;
}

HW_SPI_ID ad_spi_get_hw_spi_id(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;
        return spi->conf->id;
}

static void ad_spi_wait_event( void *p, uint16_t transferred)
{

        ad_spi_data_t *spi = (ad_spi_data_t *) p;
        OS_EVENT_SIGNAL_FROM_ISR(spi->event);
}

void ad_spi_activate_cs(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        const HW_SPI_ID id = spi->conf->id;
        /* The task must own the controller */
        OS_ASSERT(id);

        if (!hw_spi_is_slave(id)) {
                hw_spi_set_cs_low(id);
        }
}

void ad_spi_deactivate_cs(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        const HW_SPI_ID id = spi->conf->id;
        /* The task must own the controller */
        OS_ASSERT(id);

        if (!hw_spi_is_slave(id)) {
                hw_spi_set_cs_high(id);
        }
}

void ad_spi_deactivate_cs_when_spi_done(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        const HW_SPI_ID id = spi->conf->id;
        /* The task must own the controller */
        OS_ASSERT(id);
        hw_spi_wait_while_busy(id);

        ad_spi_deactivate_cs(handle);
}

int ad_spi_write(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen)
{

        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);

        if (hw_spi_is_occupied(id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }
        hw_spi_writeread_buf(id, wbuf, NULL, wlen, ad_spi_wait_event, spi);

        OS_EVENT_WAIT(spi->event, OS_EVENT_FOREVER);

        OS_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_read(ad_spi_handle_t handle, uint8_t *rbuf, size_t rlen)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);

        if (hw_spi_is_occupied(id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_read_buf(id, rbuf, rlen, ad_spi_wait_event, spi);

        OS_EVENT_WAIT(spi->event, OS_EVENT_FOREVER);

        OS_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

#if CONFIG_SPI_USE_ASYNC_TRANSACTIONS

int ad_spi_write_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen,
                                                ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);

        if (hw_spi_is_occupied(id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_write_buf(id, wbuf, wlen, cb, user_data);
        OS_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_read_async(ad_spi_handle_t handle, const uint8_t *rbuf, size_t rlen,
                                                ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);


        if (hw_spi_is_occupied(id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_read_buf(id, (uint8_t *)rbuf, (uint16_t)rlen, cb, user_data);
        OS_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_write_read_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen,
                         uint8_t *rbuf, size_t rlen, ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        OS_MUTEX_GET(spi->busy, OS_MUTEX_FOREVER);


        if (hw_spi_is_occupied(id)) {
                OS_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_writeread_buf(id, wbuf, rbuf, wlen, cb, user_data);
        OS_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}
#endif /* CONFIG_SPI_USE_ASYNC_TRANSACTIONS */


int ad_spi_io_config(HW_SPI_ID id, const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state)
{
        uint8 i = 0;
        ad_io_conf_t* gpio = (ad_io_conf_t *) io;

        /* SPI clk should be at least configured*/
        if (!AD_SPI_GPIO_VALID(io->spi_clk.port, io->spi_clk.pin)) {
                return AD_SPI_ERROR_NO_SPI_CLK_PIN;
        }
        hw_sys_pd_com_enable();
        for (i = 0; i < 3; i++) {
                if (AD_SPI_GPIO_VALID(gpio->port,gpio->pin)) {
                        hw_gpio_configure_pin_power(gpio->port, gpio->pin, io->voltage_level);
                        if (state == AD_IO_CONF_ON) {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->on.mode,
                                        gpio->on.function, gpio->on.high);
                        }
                        else {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->off.mode,
                                        gpio->off.function, gpio->off.high);

                        }

                        hw_gpio_pad_latch_enable(gpio->port, gpio->pin);
                        hw_gpio_pad_latch_disable(gpio->port, gpio->pin);
                }
                gpio++;
        }

        gpio =(ad_io_conf_t*)io->spi_cs;
        for (i = 0; i < io->cs_cnt; i++) {
                if (AD_SPI_GPIO_VALID(gpio->port,gpio->pin)) {
                        hw_gpio_configure_pin_power(gpio->port, gpio->pin, io->voltage_level);
                        if (state == AD_IO_CONF_ON) {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->on.mode,
                                        gpio->on.function, gpio->on.high);
                        }
                        else {
                                hw_gpio_configure_pin(gpio->port,gpio->pin, gpio->off.mode,
                                        gpio->off.function, gpio->off.high);
                        }


                        hw_gpio_pad_latch_enable(gpio->port, gpio->pin);
                        hw_gpio_pad_latch_disable(gpio->port, gpio->pin);
                }
                gpio++;
        }
        hw_sys_pd_com_disable();
        return AD_SPI_ERROR_NONE;
}

ADAPTER_INIT(ad_spi_adapter, ad_spi_init);

#endif /* dg_configSPI_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
