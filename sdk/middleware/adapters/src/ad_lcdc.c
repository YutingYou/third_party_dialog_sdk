/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup LCD controller
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_lcdc.c
 *
 * @brief LCD controller adapter implementation
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configLCDC_ADAPTER

#include <stdint.h>
#include "hw_lcdc.h"
#include "osal.h"
#include "hw_gpio.h"
#include "hw_sys.h"
#include "resmgmt.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#include "ad_lcdc.h"

/**
 * \brief Compacts port / pin values in a single byte
 */
#define AD_LCDC_COMPACT_PINS(_port, _pin) \
        (_port << HW_GPIO_PIN_BITS) | (_pin & ((1 << HW_GPIO_PIN_BITS) - 1))

/**
 * \brief Returns the port value from the compacted value created with \ref AD_LCDC_COMPACT_PINS
 */
#define AD_LCDC_GET_PORT(value)         (value >> HW_GPIO_PIN_BITS)

/**
 * \brief Returns the pin value from the compacted value created with \ref AD_LCDC_COMPACT_PINS
 */
#define AD_LCDC_GET_PIN(value)          (value & ((1 << HW_GPIO_PIN_BITS) - 1))

/**
 * \brief Checks if the provided handle is valid
 */
#define AD_LCDC_HANDLE_IS_VALID(x) ((((ad_lcdc_data_t *)(x)) == &lcdc_data)                        \
                                 && (((ad_lcdc_data_t *)(x))->conf != NULL))

/**
 * \brief Type of group of pins
 */
typedef enum {
        AD_LCDC_LATCH_TYPE_CTRL_SIG,            //!< Serial and parallel control signal pins
        AD_LCDC_LATCH_TYPE_EXT_CLK,             //!< External clock pins
} AD_LCDC_LATCH_TYPE;

/**
 * \brief LCDC run time data
 *
 * Structure keeps LCD related dynamic data.
 */
typedef struct {
        int16_t disp_offsetx;                   //!< Horizontal offset of first pixel in display's memory (MIPI devices)
        int16_t disp_offsety;                   //!< Vertical offset of first pixel in display's memory (MIPI devices)
        hw_lcdc_frame_t frame;                  //!< Frame dimensions for the partial mode
        bool frame_valid;                       //!< Validity flag for the frame (partial mode)
} ad_lcdc_device_data_t;

/**
 * \brief Continuous update mode data
 *
 * Variables of this type are used to store state needed for the correct configuration of the LCD
 * controller.
 */
typedef struct {
        hw_lcdc_layer_t layer;                  //!< Layer settings
        bool valid;                             //!< Validity flag
} ad_lcdc_continuous_mode_t;

/**
 * \brief LCDC run time data
 *
 * Structure keeps LCD related dynamic data that live as long as configuration is open.
 */
typedef struct {
        const ad_lcdc_controller_conf_t *conf;  //!< LCDC controller current configuration
        ad_lcdc_device_data_t *data;            //!< LCD configuration / state
        ad_lcdc_continuous_mode_t cont_mode;    //!< Continuous mode variables
        OS_TASK owner;                          //!< Task that has acquired this device.
        OS_MUTEX busy;                          //!< Semaphore for thread safety
        OS_EVENT event;                         //!< Event for async calls
        ad_lcdc_user_cb callback;               //!< Callback function to call after transaction ends
        void *callback_data;                    //!< Callback data to pass to \p callback
} ad_lcdc_data_t;

/**
 * \brief Holds current device for LCDC.
 */
__RETAINED static ad_lcdc_data_t lcdc_data;

/**
 * \brief Holds LCD specific data retained after calling \ref ad_lcdc_close function.
 */
__RETAINED static ad_lcdc_device_data_t lcdc_dev_data;

/**
 * \brief Array containing the fixed assignment signal pins
 */
static const uint8_t ad_lcdc_signal_gpios[] = {
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_24),//LCD_TE
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_26),//LCD_VCK
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_27),//LCD_ENB, PLCD_ENAB
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_28),//LCD_VST, PLCD_VSYNC
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_29),//LCD_HCK, PLCD_CLK
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_30),//LCD_HST, PLCD_HSYNC
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_0, HW_GPIO_PIN_31),//LCD_XRST
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_2), //LCD_BLUE0
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_3), //LCD_BLUE1
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_4), //LCD_GREEN0
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_5), //LCD_GREEN1
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_7), //LCD_RED0
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_8), //LCD_RED1
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_13),//LCD_VCK
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_14),//LCD_ENB, PLCD_ENAB
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_15),//LCD_VST, PLCD_VSYNC
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_16),//LCD_HCK, PLCD_CLK
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_17),//LCD_HST, PLCD_HSYNC
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_21),//LCD_XRST
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_22),//LCD_TE
};

/**
 * \brief Array containing the fixed assignment external clock pins
 */
static const uint8_t ad_lcdc_ext_gpios[] = {
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_10),//LCD_VCOM, LCD_FRP, LCD_EXTCOMIN
        AD_LCDC_COMPACT_PINS(HW_GPIO_PORT_1, HW_GPIO_PIN_11),//LCD_XFRP
};

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */
static void ad_lcdc_gpio_configure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type);
static void ad_lcdc_gpio_deconfigure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type);
static void ad_lcdc_wait_cs(const ad_lcdc_io_conf_t *io_cfg);

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void ad_lcdc_init(void)
{
        /* Adapter internal initializations */
        OS_MUTEX_CREATE(lcdc_data.busy);
        OS_EVENT_CREATE(lcdc_data.event);
}

ad_lcdc_handle_t ad_lcdc_open(const ad_lcdc_controller_conf_t *conf)
{
        ad_lcdc_data_t *lcdc = &lcdc_data;

        /* Check input validity*/
        OS_ASSERT(conf);
        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);

        /* Acquire resources */
        resource_acquire(RES_MASK(RES_ID_LCDC), RES_WAIT_FOREVER);

        /* Update adapter data */
        lcdc->conf = conf;
        lcdc->owner = OS_GET_CURRENT_TASK();

        pm_sleep_mode_request(pm_mode_idle);

        if (ad_lcdc_reconfig(lcdc, conf->drv) == AD_LCDC_ERROR_NONE) {
                ad_lcdc_gpio_configure(conf->io, AD_LCDC_LATCH_TYPE_CTRL_SIG);

                return lcdc;
        }

        ad_lcdc_close(lcdc, false);
        return NULL;
}

int ad_lcdc_reconfig(ad_lcdc_handle_t handle, const ad_lcdc_driver_conf_t *conf)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        HW_LCDC_STATUS hw_status;
        int ret = AD_LCDC_ERROR_UNKNOWN;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        /* If LCDC driver is already configured, check if the new configuration could be applied */
        if (lcdc->conf->drv) {
                if (lcdc->conf->drv->hw_init.phy_type != conf->hw_init.phy_type) {
                        OS_ASSERT(0);
                        ret = AD_LCDC_ERROR_DRIVER_CONF_INVALID;
                        goto end;
                }
        }

        if (conf->hw_init.iface_freq & HW_LCDC_CLK_PLL_BIT) {
                cm_sys_clk_set_status_t clk_status = cm_sys_clk_set(sysclk_PLL96);
                OS_ASSERT(clk_status == cm_sysclk_success);
                if (clk_status != cm_sysclk_success) {
                        ret = AD_LCDC_ERROR_PLL_NOT_SET;
                        goto end;
                }
        }

        hw_status = hw_lcdc_init(&conf->hw_init);
        OS_ASSERT(hw_status == HW_LCDC_OK);
        if (hw_status != HW_LCDC_OK) {
                goto restore_clock;
        }

        lcdc->data = &lcdc_dev_data;

        hw_lcdc_set_timing(&conf->display);
        if (conf->hw_init.phy_type == HW_LCDC_PHY_JDI_PARALLEL
                || conf->hw_init.phy_type == HW_LCDC_PHY_CLASSIC_PARALLEL) {
                hw_lcdc_jdi_parallel_set_timings(&conf->par_timings);
        }

        /* Restore the partial update mode */
        if (lcdc->data->frame_valid) {
                hw_lcdc_set_update_region(&lcdc->data->frame);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;

restore_clock:
        cm_sys_clk_set(sysclk_XTAL32M);

end:
        OS_MUTEX_PUT(lcdc->busy);

        return ret;
}

int ad_lcdc_close(ad_lcdc_handle_t handle, bool force)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_config_t cfg = {
                .phy_type = HW_LCDC_PHY_NONE,
        };

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        if (!force) {
                if (hw_lcdc_is_busy()) {
                        OS_MUTEX_PUT(lcdc->busy);
                        return AD_LCDC_ERROR_CONTROLLER_BUSY;
                }

                ad_lcdc_wait_cs(lcdc->conf->io);
        }

        ad_lcdc_gpio_deconfigure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_CTRL_SIG);

        hw_lcdc_init(&cfg);

        if ((lcdc->conf->drv->hw_init.iface_freq & HW_LCDC_CLK_PLL_BIT) && lcdc->conf != NULL) {
                /* There is no need to check if clock actually changed */
                cm_sys_clk_set(sysclk_RC32);
        }

        /* Update adapter data */
        lcdc_data.conf = NULL;
        lcdc_data.owner = NULL;

        resource_release(RES_MASK(RES_ID_LCDC));

        OS_MUTEX_PUT(lcdc->busy);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_io_config(const ad_lcdc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        /* Perform initial setup of the pins of each device */
        hw_sys_pd_com_enable();

        for (const ad_io_conf_t *cfg = io->io_list; cfg < io->io_cnt + io->io_list; cfg++) {
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

        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Function checks to find if a CS is configured and if found it waits until it is de-asserted
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 */
static void ad_lcdc_wait_cs(const ad_lcdc_io_conf_t *io_cfg)
{
        const ad_io_conf_t *cfg;

        /* Ensure that transfer has been completed. IRQ does not ensure that since it is
         * called before the end of the transfer.  */
        for (cfg = io_cfg->io_list; cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {
                uint8_t pin = AD_LCDC_COMPACT_PINS(cfg->port, cfg->pin);
                if (cfg->on.function == HW_GPIO_FUNC_LCD_SPI_EN) {
                        break;
                }
        }
        if (cfg != io_cfg->io_cnt + io_cfg->io_list) {
                bool state = !!(hw_lcdc_get_mipi_cfg() & HW_LCDC_MIPI_CFG_SPI_CSX_V);
                while (hw_gpio_get_pin_status(cfg->port, cfg->pin) == state);
        }
}

/**
 * \brief Helper function that returns the pin list and its size associated to the provided type
 *
 * \param [out] pin_list        Pin list pointer
 * \param [in]  type            Type of pins to return
 *
 * \return >0 size of list, 0: error
 */
static size_t ad_lcdc_get_pins(const uint8_t **pin_list, AD_LCDC_LATCH_TYPE type)
{
        switch (type) {
        case AD_LCDC_LATCH_TYPE_CTRL_SIG:
                *pin_list = ad_lcdc_signal_gpios;
                return sizeof(ad_lcdc_signal_gpios);
        case AD_LCDC_LATCH_TYPE_EXT_CLK:
                *pin_list = ad_lcdc_ext_gpios;
                return sizeof(ad_lcdc_ext_gpios);
        default:
                return 0;
        }
}

/**
 * \brief Configure corresponding group of pins to be ready for use
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 * \param [in] type             Type of pins to configure
 */
static void ad_lcdc_gpio_configure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type)
{
        const uint8_t *pin_list;
        size_t pin_list_size;

        if ((pin_list_size = ad_lcdc_get_pins(&pin_list, type)) == 0) {
                return;
        }

        hw_sys_pd_com_enable();

        for (const ad_io_conf_t *cfg = io_cfg->io_list;
                cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {
                uint8_t pin = AD_LCDC_COMPACT_PINS(cfg->port, cfg->pin);

                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                if (cfg->on.function == HW_GPIO_FUNC_LCD) {
                        int i;
                        for (i = 0; i < pin_list_size; ++i) {
                                if (pin == pin_list[i]) {
                                        break;
                                }
                        }
                        if (i == pin_list_size) {
                                continue;
                        }
                } else if (cfg->on.function < HW_GPIO_FUNC_LCD_SPI_DC
                        || cfg->on.function > HW_GPIO_FUNC_LCD_SPI_EN
                        || type != AD_LCDC_LATCH_TYPE_CTRL_SIG) {
                        continue;
                }

                hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->on.mode, cfg->on.function,
                        cfg->on.high);
                hw_gpio_configure_pin_power(cfg->port, cfg->pin, io_cfg->voltage_level);
                hw_gpio_pad_latch_enable(cfg->port, cfg->pin);
        }
}

/**
 * \brief Configure corresponding group of pins to be ready for sleep
 *
 * \param [in] io_cfg           LCD controller GPIO configuration
 * \param [in] type             Type of pins to configure
 */
static void ad_lcdc_gpio_deconfigure(const ad_lcdc_io_conf_t *io_cfg, AD_LCDC_LATCH_TYPE type)
{
        const uint8_t *pin_list;
        size_t pin_list_size;

        if ((pin_list_size = ad_lcdc_get_pins(&pin_list, type)) == 0) {
                return;
        }

        for (const ad_io_conf_t *cfg = io_cfg->io_list;
                cfg < io_cfg->io_cnt + io_cfg->io_list; cfg++) {
                uint8_t pin = AD_LCDC_COMPACT_PINS(cfg->port, cfg->pin);

                if (cfg->port == HW_GPIO_PORT_NONE || cfg->pin == HW_GPIO_PIN_NONE) {
                        continue;
                }
                if (cfg->on.function == HW_GPIO_FUNC_LCD) {
                        int i;
                        for (i = 0; i < pin_list_size; ++i) {
                                if (pin == pin_list[i]) {
                                        break;
                                }
                        }
                        if (i == pin_list_size) {
                                continue;
                        }
                } else if (cfg->on.function < HW_GPIO_FUNC_LCD_SPI_DC
                        || cfg->on.function > HW_GPIO_FUNC_LCD_SPI_EN
                        || type != AD_LCDC_LATCH_TYPE_CTRL_SIG) {
                        continue;
                }

                hw_gpio_configure_pin(cfg->port, cfg->pin, cfg->off.mode, cfg->off.function,
                        cfg->off.high);
                hw_gpio_pad_latch_disable(cfg->port, cfg->pin);
        }

        hw_sys_pd_com_disable();
}

/**
 * \brief Set state of the provided pin
 *
 * \param [in] io               LCD controller GPIO configuration
 * \param [in] pin_index        Pin to configure as provided by \ref AD_LCDC_COMPACT_PINS
 * \param [in] state            State of the pin
 */
static void ad_lcdc_set_gpio_state(const ad_lcdc_io_conf_t *io, uint8_t pin_index, bool state)
{
        HW_GPIO_PORT port = AD_LCDC_GET_PORT(pin_index);
        HW_GPIO_PIN pin = AD_LCDC_GET_PIN(pin_index);

        if (port == HW_GPIO_PORT_NONE || pin == HW_GPIO_PIN_NONE) {
                return;
        }

        for (const ad_io_conf_t *cfg = io->io_list; cfg < io->io_cnt + io->io_list; cfg++) {
                if (port == cfg->port && pin == cfg->pin) {
                        hw_gpio_configure_pin(port, pin, cfg->on.mode, cfg->on.function, state);
                        hw_gpio_configure_pin_power(port, pin, io->voltage_level);
                        hw_gpio_pad_latch_enable(port, pin);
                        hw_gpio_pad_latch_disable(port, pin);
                }
        }
}

int ad_lcdc_execute_cmds(ad_lcdc_handle_t handle, const uint8_t *cmds, size_t len)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        for (size_t index = 0; len > index; ++index) {
                switch (cmds[index]) {
                case LCDC_TAG_DELAY_US:
                {
                        uint16_t delay;
                        delay = cmds[++index];
                        delay |= cmds[++index] << 8;
                        hw_clk_delay_usec(delay);
                        break;
                }
                case LCDC_TAG_DELAY_MS:
                {
                        uint16_t delay;
                        delay = cmds[++index];
                        delay |= cmds[++index] << 8;
                        OS_DELAY_MS(delay);
                        break;
                }
                case LCDC_TAG_GPIO_SET_ACTIVE:
                        ad_lcdc_set_gpio_state(lcdc->conf->io, cmds[++index], true);
                        break;
                case LCDC_TAG_GPIO_SET_INACTIVE:
                        ad_lcdc_set_gpio_state(lcdc->conf->io, cmds[++index], false);
                        break;
                case LCDC_TAG_MIPI_CMD:
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, cmds[++index]);
                        break;
                case LCDC_TAG_MIPI_PARAM:
                        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, cmds[++index]);
                        break;
                case LCDC_TAG_JDI_CMD:
                        hw_lcdc_jdi_serial_cmd_send(cmds[++index]);
                        break;
                case LCDC_TAG_EXT_CLK:
                        ad_lcdc_set_external_clock(lcdc, cmds[++index]);
                        break;
                default:
                        OS_ASSERT(0);
                }
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_set_external_clock(ad_lcdc_handle_t handle, bool enable)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        if ((REG_GETF(CRG_COM, CLK_COM_REG, LCD_EXT_CLK_SEL) != HW_LCDC_EXT_CLK_OFF) != enable) {
                if (enable) {
                        ad_lcdc_gpio_configure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_EXT_CLK);
                } else {
                        ad_lcdc_gpio_deconfigure(lcdc->conf->io, AD_LCDC_LATCH_TYPE_EXT_CLK);
                }
                hw_lcdc_set_external_clk(enable ? lcdc->conf->drv->ext_clk : HW_LCDC_EXT_CLK_OFF);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_set_partial_update(ad_lcdc_handle_t handle, hw_lcdc_frame_t *frame)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        hw_lcdc_set_update_region(frame);

        if (lcdc->conf->drv->hw_init.phy_type == HW_LCDC_PHY_MIPI_SPI3
                || lcdc->conf->drv->hw_init.phy_type == HW_LCDC_PHY_MIPI_SPI4) {
                if (lcdc->data->disp_offsetx || lcdc->data->disp_offsety) {
                        hw_lcdc_frame_t mipi_frame = {
                                .startx = frame->startx + lcdc->data->disp_offsetx,
                                .starty = frame->starty + lcdc->data->disp_offsety,
                                .endx = frame->endx + lcdc->data->disp_offsetx,
                                .endy = frame->endy + lcdc->data->disp_offsety,
                        };

                        hw_lcdc_mipi_set_position(&mipi_frame);
                } else {
                        hw_lcdc_mipi_set_position(frame);
                }
        }

        lcdc->data->frame_valid = !((frame->startx == 0) && (frame->starty == 0)
                && (frame->endx == lcdc->conf->drv->display.resx - 1)
                && (frame->endy == lcdc->conf->drv->display.resy - 1));

        lcdc->data->frame = *frame;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_exit_partial_update(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        hw_lcdc_frame_t frame = {
                .startx = 0,
                .starty = 0,
                .endx = lcdc->conf->drv->display.resx - 1,
                .endy = lcdc->conf->drv->display.resy - 1,
        };

        return ad_lcdc_set_partial_update(lcdc, &frame);
}

int ad_lcdc_set_display_offset(ad_lcdc_handle_t handle, int16_t offsetx, int16_t offsety)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        lcdc->data->disp_offsetx = offsetx;
        lcdc->data->disp_offsety = offsety;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

/**
 * \brief Setup callback parameters and trigger a single frame transmission
 *
 * \param[in] cb                Callback to call (from ISR context)
 * \param[in] ud                User data to pass to \p cb
 */
static void ad_lcdc_send_one_frame(ad_lcdc_user_cb cb, void *ud)
{
        hw_lcdc_set_callback(cb, ud);
        hw_lcdc_send_one_frame();
        hw_lcdc_enable_vsync_irq(true);
}

/**
 * \brief Enable tearing effect detection and setup corresponding callback.
 *
 * \param[in] polarity          TE detection polarity
 * \param[in] cb                Callback to call (from ISR context)
 * \param[in] ud                User data to pass to \p cb
 */
static void ad_lcdc_enable_tearing(HW_LCDC_TE polarity, ad_lcdc_user_cb cb, void *ud)
{
        hw_lcdc_set_callback(cb, ud);
        hw_lcdc_set_tearing_effect(true, polarity);
        hw_lcdc_enable_tearing_effect_irq(true);
}

/**
 * \brief Callback function, called when the transmission in blocking mode is complete.
 *
 * \param[in] handle            Handle returned from ad_lcdc_open()
 */
static void ad_lcdc_wait_event(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_enable_vsync_irq(false);
        OS_EVENT_SIGNAL_FROM_ISR(lcdc->event);
}

/**
 * \brief Callback function, called on TE signal detection to initiate frame transmission.
 *
 * \note Function is used when in blocking mode transmission.
 *
 * \param[in] handle            Handle returned from ad_lcdc_open()
 */
static void ad_lcdc_tearing_callback(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_enable_vsync_irq(false);
        hw_lcdc_enable_tearing_effect_irq(false);
        hw_lcdc_set_tearing_effect(false, lcdc->conf->drv->te_polarity);

        ad_lcdc_send_one_frame(ad_lcdc_wait_event, lcdc);
}

int ad_lcdc_draw_screen(ad_lcdc_handle_t handle, const hw_lcdc_layer_t *layer, OS_TICK_TIME timeout)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        OS_BASE_TYPE ret;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        hw_lcdc_set_layer(true, layer);

        if (lcdc->conf->drv->te_enable) {
                ad_lcdc_enable_tearing(lcdc->conf->drv->te_polarity, ad_lcdc_tearing_callback, lcdc);
        }
        else {
                ad_lcdc_send_one_frame(ad_lcdc_wait_event, lcdc);
        }

        ret = OS_EVENT_WAIT(lcdc->event, timeout);

        OS_MUTEX_PUT(lcdc->busy);

        return ret == OS_EVENT_SIGNALED ? AD_LCDC_ERROR_NONE : AD_LCDC_ERROR_TIMEOUT;
}

/**
 * \brief Callback function, called when the transmission in async mode is complete.
 *
 * \param[in] handle            Handle returned from ad_lcdc_open()
 */
static void ad_lcdc_async_callback(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        ad_lcdc_user_cb cb = lcdc->callback;
        void *user_data = lcdc->callback_data;

        lcdc->callback = NULL;
        lcdc->callback_data = NULL;

        hw_lcdc_enable_vsync_irq(false);

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        cb(user_data);
}

/**
 * \brief Callback function, called on TE signal detection to initiate frame transmission.
 *
 * \note Function is used when in async mode transmission.
 *
 * \param[in] handle            Handle returned from ad_lcdc_open()
 */
static void ad_lcdc_tearing_async_callback(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        hw_lcdc_enable_vsync_irq(false);
        hw_lcdc_enable_tearing_effect_irq(false);
        hw_lcdc_set_tearing_effect(false, lcdc->conf->drv->te_polarity);

        ad_lcdc_send_one_frame(ad_lcdc_async_callback, lcdc);
}

int ad_lcdc_draw_screen_async(ad_lcdc_handle_t handle, const hw_lcdc_layer_t *layer,
        ad_lcdc_user_cb cb, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);
        /* Check if LCDC HW driver is already in use */
        if (hw_lcdc_is_busy()) {
                OS_MUTEX_PUT(lcdc->busy);
                OS_ASSERT(0);
                return AD_LCDC_ERROR_CONTROLLER_BUSY;
        }

        lcdc->callback = cb;
        lcdc->callback_data = user_data;

        hw_lcdc_set_layer(true, layer);

        if (lcdc->conf->drv->te_enable) {
                ad_lcdc_enable_tearing(lcdc->conf->drv->te_polarity, ad_lcdc_tearing_async_callback,
                        lcdc);
        }
        else {
                ad_lcdc_send_one_frame(ad_lcdc_async_callback, lcdc);
        }

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

static void continuous_mode_callback(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;
        ad_lcdc_user_cb cb = lcdc->callback;
        void *user_data = lcdc->callback_data;

        portDISABLE_INTERRUPTS();
        if (lcdc->cont_mode.valid) {
                hw_lcdc_set_stride(&lcdc->cont_mode.layer);
                lcdc->cont_mode.valid = false;
        }
        portENABLE_INTERRUPTS();

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        cb(user_data);
}

int ad_lcdc_continuous_update_start(ad_lcdc_handle_t handle, ad_lcdc_user_cb cb, void *user_data)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        /* Continuous mode cannot use tearing effect */
        OS_ASSERT(!lcdc->conf->drv->te_enable);

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        lcdc->callback = cb;
        lcdc->callback_data = user_data;

        hw_lcdc_set_layer(false, NULL);
        hw_lcdc_set_callback(continuous_mode_callback, lcdc);

        hw_lcdc_set_continuous_mode(true);
        hw_lcdc_enable_frame_end_irq(true);

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_continuous_update_stop(ad_lcdc_handle_t handle)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_MUTEX_GET(lcdc->busy, OS_MUTEX_FOREVER);

        hw_lcdc_enable_frame_end_irq(false);
        hw_lcdc_set_continuous_mode(false);

        hw_lcdc_set_callback(NULL, NULL);

        lcdc->callback = NULL;
        lcdc->callback_data = NULL;

        OS_MUTEX_PUT(lcdc->busy);

        return AD_LCDC_ERROR_NONE;
}

int ad_lcdc_continuous_update_draw(ad_lcdc_handle_t handle, const hw_lcdc_layer_t *layer)
{
        ad_lcdc_data_t *lcdc = (ad_lcdc_data_t *)handle;

        if (!AD_LCDC_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_LCDC_ERROR_HANDLE_INVALID;
        }

        OS_ENTER_CRITICAL_SECTION();
        lcdc->cont_mode.layer = *layer;
        lcdc->cont_mode.valid = true;
        OS_LEAVE_CRITICAL_SECTION();

        hw_lcdc_set_layer(true, layer);

        return AD_LCDC_ERROR_NONE;
}

ADAPTER_INIT(ad_lcdc_adapter, ad_lcdc_init);

#endif /* dg_configLCDC_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
