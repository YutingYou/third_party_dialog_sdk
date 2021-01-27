/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup UART
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_uart.c
 *
 * @brief UART adapter implementation
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUART_ADAPTER

#include "ad_uart.h"
#include "sys_power_mgr.h"
#include "sys_bsr.h"

static int ad_uart_gpio_configure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);
static void ad_uart_gpio_deconfigure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);

#define AD_UART_GPIO_IS_VALID(port, pin) ((port < HW_GPIO_PORT_MAX) && (pin < HW_GPIO_PIN_MAX))
#define AD_UART_HANDLE_IS_VALID(handle) (((handle == &ad_uart_dynamic_conf_uart1) || (handle == &ad_uart_dynamic_conf_uart2) || \
                                        (handle == &ad_uart_dynamic_conf_uart3)) && (((ad_uart_data_t *)handle)->ctrl))

static void ad_uart_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                  HW_GPIO_FUNC function, HW_GPIO_POWER power, bool high,
                                  AD_IO_CONF_STATE state, bool is_ext_api);

static int ad_uart_res_acquire(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type, uint32_t timeout);
static void ad_uart_res_release(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type);
static int ad_uart_apply_controller_config(ad_uart_handle_t handle);
static void ad_uart_signal_event_write(void *args, uint16_t transferred);
static void ad_uart_signal_event_read(void *args, uint16_t transferred);
static void ad_uart_signal_event_async_write(void *args, uint16_t transferred);
static void ad_uart_signal_event_async_read(void *args, uint16_t transferred);
static int ad_uart_gpio_config(HW_UART_ID id, const ad_uart_io_conf_t *io,
                               AD_IO_CONF_STATE state, bool is_ext_api);
static bool ad_uart_is_controller_busy(HW_UART_ID id);

static void ad_uart_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                  HW_GPIO_FUNC function, HW_GPIO_POWER power, bool high,
                                  AD_IO_CONF_STATE state, bool is_ext_api)
{
        hw_gpio_configure_pin(port, pin, mode, function, high);
        hw_gpio_configure_pin_power(port, pin, power);
        if (state == AD_IO_CONF_ON) {
                hw_gpio_pad_latch_enable(port, pin);
                /* Public API will leave the pins unlatched.
                 * Internally, pins are latched in ad_uart_open() unlatched in ad_uart_close().
                 */
                if (is_ext_api) {
                        hw_gpio_pad_latch_disable(port, pin);
                }
        } else if (state == AD_IO_CONF_OFF) {
                hw_gpio_pad_latch_enable(port, pin);
                hw_gpio_pad_latch_disable(port, pin);
        } else {
                /* Invalid state */
                OS_ASSERT(0);
        }
}

static int ad_uart_gpio_config(HW_UART_ID id, const ad_uart_io_conf_t *io, AD_IO_CONF_STATE state, bool is_ext_api)
{
        OS_ASSERT(state < 2);

        ad_pin_conf_t ad_pin_conf_tx[] = {io->tx.off, io->tx.on};
        ad_pin_conf_t ad_pin_conf_rx[] = {io->rx.off, io->rx.on};
        ad_pin_conf_t ad_pin_conf_ctsn[] = {io->ctsn.off, io->ctsn.on};
        ad_pin_conf_t ad_pin_conf_rtsn[] = {io->rtsn.off, io->rtsn.on};

        HW_GPIO_PORT tx_port = io->tx.port;
        HW_GPIO_PIN tx_pin = io->tx.pin;
        HW_GPIO_PORT rx_port = io->rx.port;
        HW_GPIO_PIN rx_pin = io->rx.pin;
        HW_GPIO_PORT ctsn_port = io->ctsn.port;
        HW_GPIO_PIN ctsn_pin = io->ctsn.pin;
        HW_GPIO_PORT rtsn_port = io->rtsn.port;
        HW_GPIO_PIN rtsn_pin = io->rtsn.pin;
        HW_GPIO_POWER voltage_level = io->voltage_level;


        if (AD_UART_GPIO_IS_VALID(tx_port,  tx_pin)) {
                ad_uart_configure_pin(tx_port, tx_pin,
                                        ad_pin_conf_tx[state].mode,
                                        ad_pin_conf_tx[state].function,
                                        voltage_level, ad_pin_conf_tx[state].high, state, is_ext_api);
        }
        if (AD_UART_GPIO_IS_VALID(rx_port, rx_pin)) {
                ad_uart_configure_pin(rx_port, rx_pin,
                                        ad_pin_conf_rx[state].mode,
                                        ad_pin_conf_rx[state].function,
                                        voltage_level, ad_pin_conf_rx[state].high, state, is_ext_api);
        }
        if (id == HW_UART2 || id == HW_UART3) {
                if (AD_UART_GPIO_IS_VALID(ctsn_port, ctsn_pin)) {
                        ad_uart_configure_pin(ctsn_port, ctsn_pin,
                                              ad_pin_conf_ctsn[state].mode,
                                              ad_pin_conf_ctsn[state].function,
                                              voltage_level, ad_pin_conf_ctsn[state].high, state, is_ext_api);
                }
                if (AD_UART_GPIO_IS_VALID(rtsn_port, rtsn_pin)) {
                        ad_uart_configure_pin(rtsn_port, rtsn_pin,
                                              ad_pin_conf_rtsn[state].mode,
                                              ad_pin_conf_rtsn[state].function,
                                              voltage_level, ad_pin_conf_rtsn[state].high, state, is_ext_api);
                }
        }
        return AD_UART_ERROR_NONE;
}

static bool ad_uart_is_controller_busy(HW_UART_ID id)
{
        return (hw_uart_tx_in_progress(id) || hw_uart_is_busy(id) || !hw_uart_transmit_empty(id));
}


static int ad_uart_gpio_configure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        const ad_uart_io_conf_t *io = ad_uart_ctrl_conf->io;
        HW_UART_ID id = ad_uart_ctrl_conf->id;
        bool auto_flow_control = ad_uart_ctrl_conf->drv->hw_conf.auto_flow_control;
        HW_GPIO_PORT ctsn_port = io->ctsn.port;
        HW_GPIO_PIN ctsn_pin = io->ctsn.pin;
        HW_GPIO_PORT rtsn_port = io->rtsn.port;
        HW_GPIO_PIN rtsn_pin = io->rtsn.pin;

        int ret = AD_UART_ERROR_NONE;

        /* Sanity checks */

        if (id == HW_UART1) {
                if (auto_flow_control) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }

        } else if ((id == HW_UART2) || (id == HW_UART3)) {
                if (auto_flow_control && !AD_UART_GPIO_IS_VALID(ctsn_port, ctsn_pin)) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }
                if (auto_flow_control && !AD_UART_GPIO_IS_VALID(rtsn_port, rtsn_pin)) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }
        }
        else {
                OS_ASSERT(0);
                ret = AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        ad_uart_gpio_config(id, io, AD_IO_CONF_ON, false);

        return ret;
}

static void ad_uart_gpio_deconfigure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        const ad_uart_io_conf_t *io = ad_uart_ctrl_conf->io;
        HW_UART_ID id = ad_uart_ctrl_conf->id;

        ad_uart_gpio_config(id, io, AD_IO_CONF_OFF, false);
}




/**
 * \brief UART device data run time data
 *
 * Structure keeps runtime data related to UART.
 *
 */
typedef struct {
        OS_EVENT event_write;                   /* Event used for synchronization in accessing UART controller for sending data. */
        OS_EVENT event_read;                    /* Event used for synchronization in accessing UART controller for receiving data. */

        struct {
                OS_TASK owner;                  /* Task that acquired this resource */
                int8_t acquire_count;
        } res_states[AD_UART_RES_TYPES];        /* This keeps track of number of acquisitions for this resource */

        int8_t open_count;                      /* Reference counter incremented in ad_uart_open(), decremented in ad_uart_close()*/
        ad_uart_user_cb read_cb;                /* User function to call after asynchronous read finishes */
        ad_uart_user_cb write_cb;               /* User function to call after asynchronous write finishes */
        void *read_cb_data;                     /* Data to pass to read_cb */
        void *write_cb_data;                    /* Data to pass to write_cb */
        uint8_t cts_pin;                        /* Packed GPIO port and pin for CTS */
        uint16_t read_cnt;                      /* Number of bytes read in last async read */
#if dg_configUART_RX_CIRCULAR_DMA
        bool use_rx_circular_dma;               /* true if UART is using circular DMA on RX */
        void *read_cb_ptr;                      /* original pointer passed to read, used only with circular DMA */
#endif
        const ad_uart_controller_conf_t *ctrl;  /* Pointer at the controller structure passed in ad_uart_open() */
} ad_uart_data_t;

typedef struct {
        ad_uart_data_t *ad_uart_data;
        uint16_t transferred;
} ad_uart_cb_data_t;

static ad_uart_data_t ad_uart_dynamic_conf_uart1;
static ad_uart_data_t ad_uart_dynamic_conf_uart2;
static ad_uart_data_t ad_uart_dynamic_conf_uart3;

__STATIC_INLINE resource_mask_t dma_resource_mask(int num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0),
                RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2),
                RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4),
                RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6),
                RES_MASK(RES_ID_DMA_CH7),
        };
        return res_mask[num];
}

static int ad_uart_res_acquire(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type, uint32_t timeout)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        OS_TASK *owner = &ad_uart_data->res_states[res_type].owner;
        int8_t *acquire_count = &ad_uart_data->res_states[res_type].acquire_count;
        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (timeout == RES_WAIT_FOREVER) {

                if (current_task == *owner) {
                        (*acquire_count)++;
                        return AD_UART_ERROR_NONE;
                }
        }

        uint32_t resource_mask = 0;

        switch (res_type) {
        case AD_UART_RES_TYPE_CONFIG:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_CONFIG : (id == HW_UART2 ? RES_ID_UART2_CONFIG : RES_ID_UART3_CONFIG));
                break;
        case AD_UART_RES_TYPE_WRITE:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_WRITE : (id == HW_UART2 ? RES_ID_UART2_WRITE : RES_ID_UART3_WRITE));
                break;
        case AD_UART_RES_TYPE_READ:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_READ : (id == HW_UART2 ? RES_ID_UART2_READ : RES_ID_UART3_READ));
                break;
        default:
                /* Invalid argument. */
                OS_ASSERT(0);

        }

        if (resource_acquire(resource_mask, timeout)) {
                *owner = current_task;
                (*acquire_count)++;
                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }

}

static void ad_uart_res_release(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type)
{

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        OS_TASK *owner = &ad_uart_data->res_states[res_type].owner;
        int8_t *acquire_count = &ad_uart_data->res_states[res_type].acquire_count;

        /* A device release can only happen from the same task that owns it, or from an ISR */
        OS_ASSERT(in_interrupt() ||
                (OS_GET_CURRENT_TASK() == *owner));

        if (--(*acquire_count) == 0) {
                *owner = NULL;

                uint32_t resource_mask = 0;

                switch (res_type) {
                case AD_UART_RES_TYPE_CONFIG:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_CONFIG : (id == HW_UART2 ? RES_ID_UART2_CONFIG : RES_ID_UART3_CONFIG));
                break;
        case AD_UART_RES_TYPE_WRITE:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_WRITE : (id == HW_UART2 ? RES_ID_UART2_WRITE : RES_ID_UART3_WRITE));
                break;
        case AD_UART_RES_TYPE_READ:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_READ : (id == HW_UART2 ? RES_ID_UART2_READ : RES_ID_UART3_READ));
                break;
                default:
                        /* Invalid argument. */
                        OS_ASSERT(0);
                }
                resource_release(resource_mask);
        }
}

static int ad_uart_apply_controller_config(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        const uart_config_ex *drv = &ad_uart_data->ctrl->drv->hw_conf;
        __UNUSED bool *use_rx_circular_dma;
        int ret;

        ad_uart_res_acquire(handle, AD_UART_RES_TYPE_CONFIG, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                ad_uart_res_release(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        if (hw_uart_init_ex(id, drv) == HW_UART_CONFIG_ERR_NOERR) {
                ret = AD_UART_ERROR_NONE;
        } else {
                OS_ASSERT(0);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

#if dg_configUART_RX_CIRCULAR_DMA

        use_rx_circular_dma = &ad_uart_data->use_rx_circular_dma;
        /*
         * If circular DMA or RX is enabled on UART, we automatically use it in adapter. However,
         * it can be enabled separately for each UART so we need to check this and configure adapter
         * in runtime appropriately.
         */

        if (((id == HW_UART1) && (dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE > 0)) ||
                ((id == HW_UART2) && (dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE > 0))) {
                *use_rx_circular_dma = true;
                hw_uart_enable_rx_circular_dma(id);
        }
        else if ((id == HW_UART3) && (dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE > 0)) {
                *use_rx_circular_dma = true;
                hw_uart_enable_rx_circular_dma(id);
        }

#endif
               ad_uart_res_release(handle, AD_UART_RES_TYPE_CONFIG);
               return ret;
}


ad_uart_handle_t ad_uart_open(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        OS_ASSERT(ad_uart_ctrl_conf &&
                  ad_uart_ctrl_conf->drv && ad_uart_ctrl_conf->io && ad_uart_ctrl_conf->id);

        uint32_t resource_mask = 0;
        ad_uart_data_t *ret = NULL;
        HW_UART_ID id = ad_uart_ctrl_conf->id;
        bool use_dma = ad_uart_ctrl_conf->drv->hw_conf.use_dma;
        HW_DMA_CHANNEL rx_dma_channel = ad_uart_ctrl_conf->drv->hw_conf.rx_dma_channel;
        HW_DMA_CHANNEL tx_dma_channel = ad_uart_ctrl_conf->drv->hw_conf.tx_dma_channel;

        pm_sleep_mode_request(pm_mode_idle);

        /* Start acquiring resources */

        /* Arbitrate on multiple masters and tasks . */
        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, id == HW_UART1 ? BSR_PERIPH_ID_UART1 : (id == HW_UART2 ? BSR_PERIPH_ID_UART2 : BSR_PERIPH_ID_UART3));

        /* Arbitrate on multiple tasks. */

        resource_mask |= 1 << (id == HW_UART1 ? RES_ID_UART1 : (id == HW_UART2 ? RES_ID_UART2 : RES_ID_UART3));

        if (use_dma) {
                resource_mask |= dma_resource_mask(rx_dma_channel);
                resource_mask |= dma_resource_mask(tx_dma_channel);
        }

        resource_acquire(resource_mask, RES_WAIT_FOREVER);


        hw_sys_pd_com_enable();

        /* Apply I/O configuration. */
        ad_uart_gpio_configure(ad_uart_ctrl_conf);

        /* Handle dynamic data */

        if (id == HW_UART1) {
                OS_EVENT_CREATE(ad_uart_dynamic_conf_uart1.event_write);
                OS_EVENT_CREATE(ad_uart_dynamic_conf_uart1.event_read);
                /* Bind controller configuration to the dynamic data. */
                ad_uart_dynamic_conf_uart1.ctrl = ad_uart_ctrl_conf;
                OS_ASSERT(ad_uart_dynamic_conf_uart1.open_count++ == 0)
                ret =  &ad_uart_dynamic_conf_uart1;
        } else if (id == HW_UART2) {
                OS_EVENT_CREATE(ad_uart_dynamic_conf_uart2.event_write);
                OS_EVENT_CREATE(ad_uart_dynamic_conf_uart2.event_read);
                /* Bind controller configuration to the dynamic data. */
                ad_uart_dynamic_conf_uart2.ctrl = ad_uart_ctrl_conf;
                OS_ASSERT(ad_uart_dynamic_conf_uart2.open_count++ == 0)
                ret =  &ad_uart_dynamic_conf_uart2;
        }
        else {
               OS_EVENT_CREATE(ad_uart_dynamic_conf_uart3.event_write);
               OS_EVENT_CREATE(ad_uart_dynamic_conf_uart3.event_read);
               /* Bind controller configuration to the dynamic data. */
               ad_uart_dynamic_conf_uart3.ctrl = ad_uart_ctrl_conf;
               OS_ASSERT(ad_uart_dynamic_conf_uart3.open_count++ == 0)
               ret =  &ad_uart_dynamic_conf_uart3;
       }

        /* Apply configuration */
        if (ad_uart_apply_controller_config(ret)) {
                /* Apply I/O de-configuration. */
                ad_uart_gpio_deconfigure(ad_uart_ctrl_conf);
                hw_sys_pd_com_disable();

                resource_release(resource_mask);

                /* Arbitrate on multiple masters. */
                sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, id == HW_UART1 ? BSR_PERIPH_ID_UART1 :
                                                         (id == HW_UART2 ? BSR_PERIPH_ID_UART2 :
                                                         BSR_PERIPH_ID_UART3));
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        return ret;
}

int ad_uart_close(ad_uart_handle_t handle, bool force)

{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;

        HW_UART_ID id = ad_uart_data->ctrl->id;
        uint32_t resource_mask = 0;
        bool use_dma = ad_uart_data->ctrl->drv->hw_conf.use_dma;
        HW_DMA_CHANNEL rx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.rx_dma_channel;
        HW_DMA_CHANNEL tx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.tx_dma_channel;

        OS_ENTER_CRITICAL_SECTION();

        if (!force) {
                if (ad_uart_is_controller_busy(id)) {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_UART_ERROR_CONTROLLER_BUSY;
                }
        } else {
                hw_uart_abort_receive(id);
        }

        hw_uart_deinit(id);

        OS_LEAVE_CRITICAL_SECTION();

        const ad_uart_controller_conf_t *ad_uart_ctrl_conf = ad_uart_data->ctrl;
        /* Apply I/O de-configuration. */
        ad_uart_gpio_deconfigure(ad_uart_ctrl_conf);
        /* Handle dynamic data */

        if (id == HW_UART1) {
                OS_ASSERT(ad_uart_dynamic_conf_uart1.open_count-- == 1);
                /* Unbind controller configuration from dynamic data. */
                ad_uart_dynamic_conf_uart1.ctrl = NULL;
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart1.event_write);
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart1.event_read);
        } else if (id == HW_UART2) {
                OS_ASSERT(ad_uart_dynamic_conf_uart2.open_count-- == 1);
                /* Unbind controller configuration from dynamic data. */
                ad_uart_dynamic_conf_uart2.ctrl = NULL;
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart2.event_write);
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart2.event_read);
        }
        else {
                OS_ASSERT(ad_uart_dynamic_conf_uart3.open_count-- == 1);
                /* Unbind controller configuration from dynamic data. */
                ad_uart_dynamic_conf_uart3.ctrl = NULL;
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart3.event_write);
                OS_EVENT_DELETE(ad_uart_dynamic_conf_uart3.event_read);
       }

        /* From now on dynamic data are invalidated. */


        hw_sys_pd_com_disable();


        /* Start releasing resources. */

        resource_mask |= 1 << (id == HW_UART1 ? RES_ID_UART1 : (id == HW_UART2 ? RES_ID_UART2 : RES_ID_UART3));

        if (use_dma) {
                resource_mask |= dma_resource_mask(rx_dma_channel);
                resource_mask |= dma_resource_mask(tx_dma_channel);
        }

        resource_release(resource_mask);

        /* Arbitrate on multiple masters. */
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, id == HW_UART1 ? BSR_PERIPH_ID_UART1 : (id == HW_UART2 ? BSR_PERIPH_ID_UART2 : BSR_PERIPH_ID_UART3));

        pm_sleep_mode_release(pm_mode_idle);

        return AD_UART_ERROR_NONE;
}

int ad_uart_reconfig(ad_uart_handle_t handle, const ad_uart_driver_conf_t *ad_drv)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        HW_DMA_CHANNEL current_tx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.tx_dma_channel;
        HW_DMA_CHANNEL current_rx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.rx_dma_channel;
        const uart_config_ex *drv = &ad_drv->hw_conf;
        HW_DMA_CHANNEL new_tx_dma_channel = drv->tx_dma_channel;
        HW_DMA_CHANNEL new_rx_dma_channel = drv->rx_dma_channel;

        int ret;

        /* Sanity checks */

        if (new_tx_dma_channel != current_tx_dma_channel) {
                OS_ASSERT(0);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        if (new_rx_dma_channel != current_rx_dma_channel) {
                OS_ASSERT(0);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        ad_uart_res_acquire(handle, AD_UART_RES_TYPE_CONFIG, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                ad_uart_res_release(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        if (ad_uart_is_controller_busy(id)) {
                OS_ASSERT(0);
                ad_uart_res_release(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_CONTROLLER_BUSY;
        }

        if (hw_uart_init_ex(id, drv) == HW_UART_CONFIG_ERR_NOERR) {
                ret = AD_UART_ERROR_NONE;
        } else {
                OS_ASSERT(0);
                ret = AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        ad_uart_res_release(handle, AD_UART_RES_TYPE_CONFIG);

        return ret;
}

static void ad_uart_signal_event_write(void *args, uint16_t transferred)
{
        ad_uart_cb_data_t *cb_data = (ad_uart_cb_data_t *) args;
        cb_data->transferred = transferred;
        OS_EVENT_SIGNAL_FROM_ISR(cb_data->ad_uart_data->event_write);
}

int ad_uart_write(ad_uart_handle_t handle, const char *wbuf, size_t wlen)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        ad_uart_cb_data_t cb_data = {ad_uart_data, 0};

        ad_uart_res_acquire(handle, AD_UART_RES_TYPE_WRITE, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                ad_uart_res_release(handle, AD_UART_RES_TYPE_WRITE);
                OS_ASSERT(0);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        hw_uart_send(id, (const uint8_t *) wbuf, wlen, ad_uart_signal_event_write, &cb_data);
        OS_EVENT_WAIT(ad_uart_data->event_write, RES_WAIT_FOREVER);

        ad_uart_res_release(handle, AD_UART_RES_TYPE_WRITE);

        return AD_UART_ERROR_NONE;
}


int ad_uart_complete_async_read(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        /* Force callback */
        return hw_uart_abort_receive(id);
}

static void ad_uart_signal_event_read(void *args, uint16_t transferred)
{
        ad_uart_cb_data_t *cb_data = (ad_uart_cb_data_t *) args;
        cb_data->transferred = transferred;
        /* The callback might get called directly by:
         * - ad_uart_complete_async_read()
         * - hw_uart_receive() in case the data are available on the circular buffer
         * In all of the cases not in an interrupt context.
         */
        if (in_interrupt()) {
                OS_EVENT_SIGNAL_FROM_ISR(cb_data->ad_uart_data->event_read);
        } else {
                OS_EVENT_SIGNAL(cb_data->ad_uart_data->event_read);
        }
}

int ad_uart_read(ad_uart_handle_t handle, char *rbuf, size_t rlen, OS_TICK_TIME timeout)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        ad_uart_cb_data_t cb_data = {ad_uart_data, 0};

        ad_uart_res_acquire(handle, AD_UART_RES_TYPE_READ, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                ad_uart_res_release(handle, AD_UART_RES_TYPE_READ);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        /* If there is a pending read event clear it out.
         * This may occur while waiting to take the event_read semaphore and
         * the configured timeout expires.
         */
        OS_EVENT_CHECK(ad_uart_data->event_read);

        hw_uart_receive(id, (uint8_t *) rbuf, rlen, ad_uart_signal_event_read, &cb_data);
        /* Wait for receiving the read event */
        OS_EVENT_WAIT(ad_uart_data->event_read, timeout);
        /* Needs to be called to cover the following cases:
         * 1. A circular DMA is used, the data from the circular
         *     buffer will be copied to the application buffer.
         * 2. A timeout occurs and we need to abort gracefully.
         */
        ad_uart_complete_async_read(handle);

        ad_uart_res_release(handle, AD_UART_RES_TYPE_READ);

        return cb_data.transferred;
}

static void ad_uart_signal_event_async_write(void *args, uint16_t transferred)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *) args;
        ad_uart_data->write_cb(ad_uart_data->write_cb_data, transferred);
        ad_uart_res_release(ad_uart_data, AD_UART_RES_TYPE_WRITE);
}

int ad_uart_write_async(ad_uart_handle_t handle, const char *wbuf, size_t wlen,
                           ad_uart_user_cb cb, void *user_data)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;


        if (!ad_uart_res_acquire(handle, AD_UART_RES_TYPE_WRITE, 0)) {

                /* Check ad_uart_close() for being faster */
                if (!ad_uart_data->open_count) {
                        OS_ASSERT(0);
                        ad_uart_res_release(handle, AD_UART_RES_TYPE_WRITE);
                        return AD_UART_ERROR_DEVICE_CLOSED;
                }

                ad_uart_data->write_cb = cb;
                ad_uart_data->write_cb_data = user_data;

                hw_uart_send(id, (const uint8_t *) wbuf, wlen, ad_uart_signal_event_async_write, ad_uart_data);

                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }
}

static void ad_uart_signal_event_async_read(void *args, uint16_t transferred)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *) args;
        ad_uart_data->read_cb(ad_uart_data->read_cb_data, transferred);
        ad_uart_res_release(ad_uart_data, AD_UART_RES_TYPE_READ);
}

int ad_uart_read_async(ad_uart_handle_t handle, char *rbuf, size_t rlen, ad_uart_user_cb cb,
                         void *user_data)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        if (!ad_uart_res_acquire(handle,  AD_UART_RES_TYPE_READ, 0)) {

                /* Check ad_uart_close() for being faster */
                if (!ad_uart_data->open_count) {
                        OS_ASSERT(0);
                        ad_uart_res_release(handle, AD_UART_RES_TYPE_READ);
                        return AD_UART_ERROR_DEVICE_CLOSED;
                }

                ad_uart_data->read_cb = cb;
                ad_uart_data->read_cb_data = user_data;

#if dg_configUART_RX_CIRCULAR_DMA

                ad_uart_data->read_cb_ptr = rbuf;
#endif

                hw_uart_receive(id, (uint8_t *) rbuf, rlen, ad_uart_signal_event_async_read, ad_uart_data);

                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }
}

HW_UART_ID ad_uart_get_hw_uart_id(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        return id;
}

int ad_uart_io_config(HW_UART_ID id, const ad_uart_io_conf_t *io, AD_IO_CONF_STATE state)
{
        int ret;
        hw_sys_pd_com_enable();
        ret = ad_uart_gpio_config(id, io, state, true);
        hw_sys_pd_com_disable();

        return ret;
}


void ad_uart_init(void)
{
}

ADAPTER_INIT(ad_uart_adapter, ad_uart_init)

#endif /* dg_configUART_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
