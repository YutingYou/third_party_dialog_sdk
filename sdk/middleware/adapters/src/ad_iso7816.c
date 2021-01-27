/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup ISO7816
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_iso7816.c
 *
 * @brief ISO7816 adapter implementation
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configISO7816_ADAPTER

#include <stdint.h>
#include <hw_iso7816.h>
#include <osal.h>
#include <hw_gpio.h>
#include <hw_sys.h>
#include <resmgmt.h>
#include <sys_bsr.h>
#include <ad_iso7816.h>

#include "interrupts.h"
#include "sys_power_mgr.h"

#define ISO7816_BUS_RESOURCE_ID         (RES_ID_UART3)

#define ISO7816_PIN_NUM                 (3)

#define AD_ISO7816_GET_PORT(value) \
        (value >> HW_GPIO_PIN_BITS)
#define AD_ISO7816_GET_PIN(value) \
        (value & ((1 << HW_GPIO_PIN_BITS) - 1))

#define AD_ISO7816_HANDLE_IS_VALID(x) (((x) == &iso7816_data) &&                                   \
                                       (((ad_iso7816_data_t *)(x))->conf != NULL))

/**
 * \brief ISO7816 adapter (internal) data
 *
 * Data structure of ISO7816 controller
 */
typedef struct {
        const ad_iso7816_controller_conf_t *conf;       /**< ISO7816 controller current configuration */
        OS_TASK  owner;                                 /**< The task which opened the controller */
        OS_EVENT event;                                 /**< Semaphore for async calls  */
        OS_MUTEX busy;                                  /**< Semaphore for thread safety */
        hw_iso7816_config_t hw_init;                    /**< ISO7816 configuration passed to the ISO7816 LLD. */
        ad_iso7816_transact_cb cb_func;                 /**< User function to call after asynchronous
                                                             call finishes */
        void *cb_data;                                  /**< Data to pass to callback function */
        bool clk_paused;
        HW_ISO7816_CLK_STOP clk_stop;                   /**< Clock stop mode supported by the card */
} ad_iso7816_data_t;


typedef struct {
        ad_iso7816_data_t *iso7816;                     /**< Adapter handle */
        size_t rx_len;                                  /**< Received bytes length */
        HW_ISO7816_ERROR status;                        /**< Status of the transaction */
} iso7816_cb_data_t;

/**
 * \brief Array to hold current device for each ISO7816.
 */
__RETAINED static ad_iso7816_data_t iso7816_data;

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */
static void ad_iso7816_gpio_configure(const ad_iso7816_io_conf_t *io_cfg);
static void ad_iso7816_gpio_deconfigure(const ad_iso7816_io_conf_t *io_cfg);
static int ad_iso7816_device_start_session(ad_iso7816_data_t *iso7816);
static void ad_iso7816_device_end_session(ad_iso7816_data_t *iso7816);

void ad_iso7816_init(void)
{
        /* Adapter internal initializations */
        OS_MUTEX_CREATE(iso7816_data.busy);
        OS_EVENT_CREATE(iso7816_data.event);
}

ad_iso7816_handle_t ad_iso7816_open(const ad_iso7816_controller_conf_t *conf)
{
        hw_iso7816_atr_params_t atr;

        /* Check input validity*/
        ad_iso7816_data_t *iso7816 = &iso7816_data;
        OS_ASSERT(iso7816);
        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);

        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_UART3);
        resource_acquire(RES_MASK(ISO7816_BUS_RESOURCE_ID), RES_WAIT_FOREVER);

        /* Update adapter data */
        iso7816->conf  = conf;
        iso7816->owner = OS_GET_CURRENT_TASK();

        pm_sleep_mode_request(pm_mode_idle);

        hw_sys_pd_com_enable();

        /* Set device defaults */
        hw_iso7816_get_default_atr_params(&atr);
        iso7816->clk_stop = atr.clk_stop;
        hw_iso7816_convert_atr_params_to_cfg(&atr, &iso7816->hw_init);

        /* Configure driver*/
        if (ad_iso7816_reconfig(iso7816, (ad_iso7816_driver_conf_t *)conf->drv) == AD_ISO7816_ERROR_NONE) {
                ad_iso7816_gpio_configure(iso7816->conf->io);

                /* Configure driver using conf configuration*/
                if (ad_iso7816_device_start_session(iso7816) == AD_ISO7816_ERROR_NONE) {
                        return iso7816;
                }
        }

        ad_iso7816_close(iso7816, false);
        return NULL;
}

int ad_iso7816_reconfig(ad_iso7816_handle_t handle, const ad_iso7816_driver_conf_t *conf)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;
        int ret;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        iso7816->hw_init.rx_lvl = conf->rx_lvl;
        iso7816->hw_init.tx_lvl = conf->tx_lvl;
        iso7816->hw_init.rst.port = iso7816->conf->io->rst.port;
        iso7816->hw_init.rst.pin = iso7816->conf->io->rst.pin;

        hw_iso7816_init(&iso7816->hw_init);

        OS_MUTEX_PUT(iso7816->busy);
        return AD_ISO7816_ERROR_NONE;
}

int ad_iso7816_close(ad_iso7816_handle_t handle, bool force)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);
        if (!force && hw_iso7816_is_busy()) {
                OS_MUTEX_PUT(iso7816->busy);
                return AD_ISO7816_ERROR_CONTROLLER_BUSY;
        }

        /* Abort (if any) ongoing transactions */
        ad_iso7816_device_end_session(iso7816);

        ad_iso7816_gpio_deconfigure(iso7816->conf->io);
        hw_iso7816_deinit();
        hw_sys_pd_com_disable();

        resource_release(RES_MASK(ISO7816_BUS_RESOURCE_ID));
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_UART3);

        OS_MUTEX_PUT(iso7816->busy);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_ISO7816_ERROR_NONE;
}

int ad_iso7816_io_config (const ad_iso7816_io_conf_t *io, AD_IO_CONF_STATE state)
{
        /* Perform initial setup of the pins of each device */
        hw_sys_pd_com_enable();

        for (const ad_io_conf_t *cfg = (const ad_io_conf_t *)io;
                cfg < ISO7816_PIN_NUM + (const ad_io_conf_t *)io; cfg++) {
                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                if (state == AD_IO_CONF_OFF) {
                        hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->off.mode, cfg->off.function,
                                cfg->off.high);
                } else {
                        hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->on.mode, cfg->on.function,
                                cfg->on.high);
                }
                hw_gpio_configure_pin_power(cfg->port, cfg->pin, io->voltage_level);
                hw_gpio_pad_latch_enable(cfg->port, cfg->pin);
                hw_gpio_pad_latch_disable(cfg->port, cfg->pin);
        }

        hw_sys_pd_com_disable();

        return AD_ISO7816_ERROR_NONE;
}

static void ad_iso7816_gpio_configure(const ad_iso7816_io_conf_t *io)
{
        for (const ad_io_conf_t *cfg = (const ad_io_conf_t *)io;
                cfg < ISO7816_PIN_NUM + (const ad_io_conf_t *)io; cfg++) {
                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->on.mode, cfg->on.function,
                        cfg->on.high);
                hw_gpio_configure_pin_power(cfg->port, cfg->pin, io->voltage_level);
                hw_gpio_pad_latch_enable(cfg->port, cfg->pin);
        }
}

static void ad_iso7816_gpio_deconfigure(const ad_iso7816_io_conf_t *io)
{
        while (hw_iso7816_is_busy());

        for (const ad_io_conf_t *cfg = (const ad_io_conf_t *)io;
                cfg < ISO7816_PIN_NUM + (const ad_io_conf_t *)io; cfg++) {
                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->off.mode, cfg->off.function,
                        cfg->off.high);
                hw_gpio_pad_latch_disable(cfg->port, cfg->pin);
        }
}

/**
 * \brief Prepare device and card for communication
 *
 * \param[in] iso7816           ISO7816 handle
 *
 * \return 0: success, <0: error code
 */
static int ad_iso7816_device_start_session(ad_iso7816_data_t *iso7816)
{
        hw_iso7816_atr_params_t atr;
        int ret = AD_ISO7816_ERROR_NONE;

        /* Set device defaults */
        hw_iso7816_get_default_atr_params(&atr);

        if (!iso7816->conf->drv->manual_control) {
                size_t atr_len;
                uint8_t buf[HW_ISO7816_ATR_MAX_BYTE_SIZE];

                atr_len = ad_iso7816_activate(iso7816, buf);
                /* Check if activation failed */
                if (!atr_len) {
                        ret = AD_ISO7816_ERROR_OTHER;
                }

                if (iso7816->conf->drv->opt_speed && atr_len) {
                        HW_ISO7816_ERROR status;

                        status = hw_iso7816_parse_atr(buf, atr_len, &atr);

                        if (status != HW_ISO7816_ERR_OK) {
                                ret = AD_ISO7816_ERROR_OTHER;
                        }
                        else if (atr.mode_negotiable
                                && (atr.pps_params.fmax > iso7816->hw_init.pps_params.fmax
                                        || atr.pps_params.f < iso7816->hw_init.pps_params.f
                                        || atr.pps_params.d > iso7816->hw_init.pps_params.d)) {
                                hw_iso7816_pps_params_t pps = {
                                        .t = atr.pps_params.t,
                                        .f = atr.pps_params.f,
                                        .d = atr.pps_params.d,
                                        .fmax = atr.pps_params.fmax,
                                };

                                status = ad_iso7816_exchange_pps(iso7816, &pps, true, false);
                                /* Check if PPS failed */
                                if (status != HW_ISO7816_ERR_OK
                                        && status != HW_ISO7816_ERR_PPS_PART_ACPT) {
                                        ret = AD_ISO7816_ERROR_OTHER;
                                }
                        }
                }
        }

        return ret;
}

/**
 * \brief End communication session deactivating the device if not in manual mode
 *
 * \param[in] iso7816           ISO7816 handle
 */
static void ad_iso7816_device_end_session(ad_iso7816_data_t *iso7816)
{
        if (!iso7816->conf->drv->manual_control) {
                ad_iso7816_deactivate(iso7816);
        }
}

int ad_iso7816_stop_clock(ad_iso7816_handle_t handle)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        if (iso7816->clk_stop != HW_ISO7816_CLOCK_STOP_NOT_SUP
                && !iso7816->clk_paused) {
                iso7816->clk_paused = true;
                hw_iso7816_clock_stop(iso7816->clk_stop);
        }

        OS_MUTEX_PUT(iso7816->busy);

        return AD_ISO7816_ERROR_NONE;
}

int ad_iso7816_resume_clock(ad_iso7816_handle_t handle)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        if (iso7816->clk_stop != HW_ISO7816_CLOCK_STOP_NOT_SUP
                && iso7816->clk_paused) {
                iso7816->clk_paused = false;
                ad_iso7816_gpio_configure(handle);
                hw_iso7816_clock_resume();
        }

        OS_MUTEX_PUT(iso7816->busy);

        return AD_ISO7816_ERROR_NONE;
}

static void ad_iso7816_atr_event(void *p, HW_ISO7816_ERROR status, size_t len)
{
        iso7816_cb_data_t *cb_data = (iso7816_cb_data_t *)p;
        cb_data->rx_len = len;
        cb_data->status = status;
        OS_EVENT_SIGNAL_FROM_ISR(cb_data->iso7816->event);
}

int ad_iso7816_activate(ad_iso7816_handle_t handle, uint8_t *atr)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;
        hw_iso7816_config_t *hw_init = &iso7816->hw_init;
        hw_iso7816_atr_params_t atr_params;
        iso7816_cb_data_t cb_data = { .iso7816 = iso7816, .rx_len = 0, .status =
                HW_ISO7816_ERR_UNKNOWN };
        HW_ISO7816_ERROR status = HW_ISO7816_ERR_UNKNOWN;
        uint8_t buf[HW_ISO7816_ATR_MAX_BYTE_SIZE];

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        if (atr == NULL) {
                atr = buf;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        hw_iso7816_cold_reset();

        hw_iso7816_receive_atr_async(atr, ad_iso7816_atr_event, &cb_data);

        OS_EVENT_WAIT(iso7816->event, OS_EVENT_FOREVER);

        if (cb_data.status == HW_ISO7816_ERR_OK) {
                status = hw_iso7816_parse_atr(atr, cb_data.rx_len, &atr_params);
        }

        if (status == HW_ISO7816_ERR_OK) {
                iso7816->clk_stop = atr_params.clk_stop;

                if (!atr_params.mode_negotiable) {
                        hw_iso7816_convert_atr_params_to_cfg(&atr_params, hw_init);
                        hw_iso7816_init(hw_init);
                }
                else {
                        /* Change only protocol parameters and initialize protocol(s) */
                        hw_init->pps_params.t = atr_params.pps_params.t;
                        hw_init->pps_params.spu = atr_params.pps_params.spu;
                        hw_init->n = atr_params.n;
                        hw_init->conv = atr_params.convention;
                        hw_init->t0_prot = atr_params.t0_prot;
                        hw_init->t1_prot = atr_params.t1_prot;

                        if (hw_init->pps_params.t == 0 && atr_params.t0_prot.available) {
                                hw_iso7816_calculate_protocol_times_t0(hw_init->n,
                                        hw_init->pps_params.f, hw_init->pps_params.d,
                                        &atr_params.t0_prot);
                        }
                        else if (iso7816->hw_init.pps_params.t == 1
                                && atr_params.t1_prot.available) {
                                hw_iso7816_calculate_protocol_times_t1(hw_init->n,
                                        hw_init->pps_params.f, hw_init->pps_params.d,
                                        &atr_params.t1_prot);
                        }
                }
        }

        OS_MUTEX_PUT(iso7816->busy);

        return (status != HW_ISO7816_ERR_OK) ? 0 : cb_data.rx_len;
}

int ad_iso7816_warm_reset(ad_iso7816_handle_t handle)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        hw_iso7816_warm_reset();

        OS_MUTEX_PUT(iso7816->busy);

        return AD_ISO7816_ERROR_NONE;
}

int ad_iso7816_deactivate(ad_iso7816_handle_t handle)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        hw_iso7816_deactivate();

        iso7816_data.cb_func = NULL;

        OS_MUTEX_PUT(iso7816->busy);

        return AD_ISO7816_ERROR_NONE;
}

static void ad_iso7816_pps_event(void *p, HW_ISO7816_ERROR status)
{
        iso7816_cb_data_t *cb_data = (iso7816_cb_data_t *)p;
        cb_data->status = status;
        OS_EVENT_SIGNAL_FROM_ISR(cb_data->iso7816->event);
}

int ad_iso7816_exchange_pps(ad_iso7816_handle_t handle, hw_iso7816_pps_params_t *params,
        bool pps1, bool pps2)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;
        iso7816_cb_data_t cb_data = { .iso7816 = iso7816, .status = HW_ISO7816_ERR_UNKNOWN };

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        hw_iso7816_exchange_pps_async(params, pps1, pps2, ad_iso7816_pps_event, &cb_data);

        OS_EVENT_WAIT(iso7816->event, OS_EVENT_FOREVER);

        if (cb_data.status == HW_ISO7816_ERR_OK
                || cb_data.status == HW_ISO7816_ERR_PPS_PART_ACPT) {
                bool reinit_needed = false;

                if (pps1) {
                        iso7816->hw_init.pps_params.f = params->f;
                        iso7816->hw_init.pps_params.fmax = params->fmax;
                        iso7816->hw_init.pps_params.d = params->d;
                        reinit_needed = true;
                }

                if (pps2) {
                        iso7816->hw_init.pps_params.spu = params->spu;
                }

                if (reinit_needed) {
                        hw_iso7816_init(&iso7816->hw_init);
                }
        }

        OS_MUTEX_PUT(iso7816->busy);

        return cb_data.status;
}

static void ad_iso7816_transact_event(void *p, HW_ISO7816_ERROR status, HW_ISO7816_SW1SW2 sw1sw2,
        size_t len)
{
        iso7816_cb_data_t *cb_data = (iso7816_cb_data_t *)p;

        (void)sw1sw2; /* To suppress compiler warning */

        cb_data->rx_len = len;
        cb_data->status = status;
        OS_EVENT_SIGNAL_FROM_ISR(cb_data->iso7816->event);
}

int ad_iso7816_apdu_transact(ad_iso7816_handle_t handle, const uint8_t *tx_apdu, size_t tx_len,
        uint8_t *rx_apdu, size_t *rx_len)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;
        iso7816_cb_data_t cb_data = {
                .iso7816 = iso7816,
                .rx_len = 0,
                .status = HW_ISO7816_ERR_UNKNOWN,
        };

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        if (iso7816->hw_init.pps_params.t == 0) {
                hw_iso7816_apdu_transact_t0_async(tx_apdu, tx_len, rx_apdu,
                        ad_iso7816_transact_event, &cb_data);
        }
        else if (iso7816->hw_init.pps_params.t == 1) {
                hw_iso7816_apdu_transact_t1_async(tx_apdu, tx_len, rx_apdu,
                        ad_iso7816_transact_event, &cb_data);
        }
        else {
                /* Only T=0 and T=1 are supported. Please, check for corruption in parameters */
                OS_ASSERT(0);
        }

        OS_EVENT_WAIT(iso7816->event, OS_EVENT_FOREVER);

        *rx_len = cb_data.rx_len;

        OS_MUTEX_PUT(iso7816->busy);

        return cb_data.status;
}

int ad_iso7816_supervisory_transact_t1(ad_iso7816_handle_t handle, HW_ISO7816_S_PCB_VAL type,
        uint8_t value)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;
        iso7816_cb_data_t cb_data = {
                .iso7816 = iso7816,
                .rx_len = 0,
                .status = HW_ISO7816_ERR_UNKNOWN,
        };

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);

        /* Only T=1 is supported */
        OS_ASSERT(iso7816->hw_init.pps_params.t == 1);

        hw_iso7816_supervisory_transact_t1_async(type, &value, ad_iso7816_transact_event, &cb_data);

        OS_EVENT_WAIT(iso7816->event, OS_EVENT_FOREVER);

        OS_MUTEX_PUT(iso7816->busy);

        return cb_data.status;
}

static void ad_iso7816_apdu_async_event(void *p, HW_ISO7816_ERROR status, HW_ISO7816_SW1SW2 sw1sw2,
        size_t len)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)p;
        ad_iso7816_transact_cb cb = iso7816->cb_func;
        void *user_data = iso7816->cb_data;

        iso7816->cb_func = NULL;
        iso7816->cb_data = NULL;

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        cb(user_data, status, sw1sw2, len);
}

int ad_iso7816_apdu_transact_async(ad_iso7816_handle_t handle, const uint8_t *tx_apdu, size_t tx_len,
        uint8_t *rx_apdu, ad_iso7816_transact_cb cb, void *user_data)
{
        ad_iso7816_data_t *iso7816 = (ad_iso7816_data_t *)handle;

        if (!AD_ISO7816_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(iso7816->busy, OS_MUTEX_FOREVER);
        /* Check if ISO7816 HW driver is already in use */
        if (hw_iso7816_is_busy()) {
                OS_MUTEX_PUT(iso7816->busy);
                OS_ASSERT(0);
                return AD_ISO7816_ERROR_CONTROLLER_BUSY;
        }

        iso7816->cb_func = cb;
        iso7816->cb_data = user_data;

        if (iso7816->hw_init.pps_params.t == 0) {
                hw_iso7816_apdu_transact_t0_async(tx_apdu, tx_len, rx_apdu,
                        ad_iso7816_apdu_async_event, iso7816);
        }
        else if (iso7816->hw_init.pps_params.t == 1) {
                hw_iso7816_apdu_transact_t1_async(tx_apdu, tx_len, rx_apdu,
                        ad_iso7816_apdu_async_event, iso7816);
        }
        else {
                /* Only T=0 and T=1 are supported. Please, check for corruption in parameters */
                OS_ASSERT(0);
        }

        OS_MUTEX_PUT(iso7816->busy);

        return AD_ISO7816_ERROR_NONE;
}

ADAPTER_INIT(ad_iso7816_adapter, ad_iso7816_init)

#endif /* dg_configISO7816_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
