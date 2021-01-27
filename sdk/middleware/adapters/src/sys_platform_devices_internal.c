/**
 ****************************************************************************************
 *
 * @file sys_platform_devices_internal.c
 *
 * @brief Configuration of devices connected to board
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configGPADC_ADAPTER == 1)

#include "sys_platform_devices_internal.h"

/*
 * Define sources connected to GPADC
 */
const ad_gpadc_driver_conf_t temp_sensor_radio_driver_internal = {
        .clock                  = HW_GPADC_CLOCK_INTERNAL,
        .input_mode             = HW_GPADC_INPUT_MODE_SINGLE_ENDED,
        .input                  = HW_GPADC_INPUT_SE_TEMPSENS,
        .temp_sensor            = HW_GPADC_TEMP_SENSOR_NEAR_RADIO,
        .sample_time            = 2,
        .chopping               = false,
        .oversampling           = HW_GPADC_OVERSAMPLING_64_SAMPLES,
        .input_attenuator       = HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2,
};
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
const ad_gpadc_driver_conf_t temp_sensor_bandgap_driver_internal = {
        .clock                  = HW_GPADC_CLOCK_INTERNAL,
        .input_mode             = HW_GPADC_INPUT_MODE_SINGLE_ENDED,
        .input                  = HW_GPADC_INPUT_SE_TEMPSENS,
        .temp_sensor            = HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP,
        .sample_time            = 2,
        .chopping               = false,
        .oversampling           = HW_GPADC_OVERSAMPLING_64_SAMPLES,
        .input_attenuator       = HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2,
};
const ad_gpadc_driver_conf_t battery_level_driver_internal = {
        .clock                  = HW_GPADC_CLOCK_INTERNAL,
        .input_mode             = HW_GPADC_INPUT_MODE_SINGLE_ENDED,
        .input                  = HW_GPADC_INPUT_SE_VBAT,
        .sample_time            = 2,
        .chopping               = false,
        .oversampling           = HW_GPADC_OVERSAMPLING_4_SAMPLES,
        .input_attenuator       = HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2,
};

const ad_gpadc_controller_conf_t TEMP_SENSOR_BANDGAP_INTERNAL = {
        HW_GPADC_1,
        NULL,
        &temp_sensor_bandgap_driver_internal
};

const ad_gpadc_controller_conf_t BATTERY_LEVEL_INTERNAL = {
        HW_GPADC_1,
        NULL,
        &battery_level_driver_internal
};
#endif /* dg_configUSE_LP_CLK == LP_CLK_RCX */
const ad_gpadc_controller_conf_t TEMP_SENSOR_RADIO_INTERNAL = {
        HW_GPADC_1,
        NULL,
        &temp_sensor_radio_driver_internal
};

#endif /* dg_configGPADC_ADAPTER */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1))

#include "ad_uart.h"

const ad_uart_io_conf_t sys_platform_console_io_conf = {
        /* Rx UART2 */
        .rx = {
                .port = SER1_RX_PORT,
                .pin = SER1_RX_PIN,
                /* On */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
        },
        /* Tx UART2 */
        .tx = {
                .port = SER1_TX_PORT,
                .pin = SER1_TX_PIN,
                /* On */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
        },
        /* RTSN */
        .rtsn = {
                .port = SER1_RTS_PORT,
                .pin = SER1_RTS_PIN,
                /* On */
                {
                        .mode = SER1_RTS_MODE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RTS_MODE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
        },
        /* CTSN */
        .ctsn = {
                .port = SER1_CTS_PORT,
                .pin = SER1_CTS_PIN,
                /* On */
                {
                        .mode = SER1_CTS_MODE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_CTS_MODE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
        },
        /* Voltage Level */
        .voltage_level = HW_GPIO_POWER_V33,
};

const ad_uart_driver_conf_t sys_platform_console_uart_driver_conf = {
        {

                .baud_rate = HW_UART_BAUDRATE_115200,
                .data = HW_UART_DATABITS_8,
                .parity = HW_UART_PARITY_NONE,
                .stop = HW_UART_STOPBITS_1,
                .auto_flow_control = 1,
                .use_fifo = 1,
                .use_dma = 1,
                .tx_dma_channel = HW_DMA_CHANNEL_3,
                .rx_dma_channel = HW_DMA_CHANNEL_2,
                .tx_fifo_tr_lvl = 0,
                .rx_fifo_tr_lvl = 0,
        }
};

const ad_uart_controller_conf_t sys_platform_console_controller_conf = {
        .id = SER1_UART,
        .io = &sys_platform_console_io_conf,
        .drv = &sys_platform_console_uart_driver_conf,
};
#endif /* ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1)) */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_DGTL == 1))
#include "ad_uart.h"
#include "dgtl_config.h"

const ad_uart_io_conf_t sys_platform_dgtl_io_conf = {
        /* Rx UART2 */
        .rx = {
                .port = SER1_RX_PORT,
                .pin = SER1_RX_PIN,
                /* On */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
        },
        /* Tx UART2 */
        .tx = {
                .port = SER1_TX_PORT,
                .pin = SER1_TX_PIN,
                /* On */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
        },
        /* RTSN */
        .rtsn = {
                .port = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_PORT : HW_GPIO_PORT_NONE,
                .pin = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_PIN : HW_GPIO_PIN_NONE,
                /* On */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
        },
        /* CTSN */
        .ctsn = {
                .port = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_PORT : HW_GPIO_PORT_NONE,
                .pin = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_PIN : HW_GPIO_PIN_NONE,
                /* On */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
        },
        /* Voltage Level */
        .voltage_level = HW_GPIO_POWER_V33,
};

const ad_uart_driver_conf_t sys_platform_dgtl_uart_driver_conf = {
        {

                .baud_rate = HW_UART_BAUDRATE_115200,
                .data = HW_UART_DATABITS_8,
                .parity = HW_UART_PARITY_NONE,
                .stop = HW_UART_STOPBITS_1,
                .auto_flow_control = DGTL_AUTO_FLOW_CONTROL,
                .use_fifo = 1,
                .use_dma = 1,
                .tx_dma_channel = HW_DMA_CHANNEL_3,
                .rx_dma_channel = HW_DMA_CHANNEL_2,
                .tx_fifo_tr_lvl = 0,
                .rx_fifo_tr_lvl = 0,
        }
};

const ad_uart_controller_conf_t sys_platform_dgtl_controller_conf = {
        .id = SER1_UART,
        .io = &sys_platform_dgtl_io_conf,
        .drv = &sys_platform_dgtl_uart_driver_conf,
};
#endif /* (dg_configUART_ADAPTER == 1) && (dg_configUSE_DGTL == 1) */

