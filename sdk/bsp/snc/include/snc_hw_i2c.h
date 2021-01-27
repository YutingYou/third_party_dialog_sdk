/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_I2C
 *
 * \brief I2C LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_i2c.h
 *
 * @brief SNC-Definition of I2C Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_I2C_H_
#define SNC_HW_I2C_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_I2C

#include "snc_defs.h"
#include "ad.h"
#include "hw_i2c.h"
#include "hw_dma.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief I2C access flags
 *
 */
typedef enum {
        SNC_HW_I2C_FLAG_NONE,                       /**< No STOP or RESTART conditions will be generated */
        SNC_HW_I2C_FLAG_ADD_STOP        = (1 << 0), /**< STOP condition will be generated at the end of the transaction */
        SNC_HW_I2C_FLAG_ADD_RESTART     = (1 << 1), /**< RESTART condition will be generated at the beginning of the transaction */
} SNC_HW_I2C_ACCESS_FLAGS;

#if (dg_configI2C_ADAPTER == 0)
#ifndef I2C_DEFAULT_CLK_CFG
        #define I2C_DEFAULT_CLK_CFG .i2c.clock_cfg = { 0, 0, 0, 0, 0, 0 }
#endif /* I2C_DEFAULT_CLK_CFG */

/**
 * \brief I2C I/O configuration
 *
 * I2C I/O configuration
 */
typedef struct {
        ad_io_conf_t scl;               /**< Configuration of SCL signal */
        ad_io_conf_t sda;               /**< Configuration of SDA signal */
        HW_GPIO_POWER voltage_level;    /**< Signals' voltage level */
} snc_i2c_io_conf_t;

/**
 * \brief I2C driver configuration
 *
 * Configuration of I2C low level driver
 */
typedef struct {
        i2c_config i2c;                 /**< Configuration of I2C low level driver */
        HW_DMA_CHANNEL dma_channel;     /**< DMA channel */
} snc_i2c_driver_conf_t;

/**
 * \brief I2C controller configuration
 *
 * Configuration of I2C controller
 */
typedef struct {
        const HW_I2C_ID id;                     /**< Controller instance */
        const snc_i2c_io_conf_t *io;            /**< I/O configuration */
        const snc_i2c_driver_conf_t *drv;       /**< Driver configuration */
} snc_i2c_controller_conf_t;

#else
#include "ad_i2c.h"

/**
 * \brief I2C I/O configuration
 *
 * I2C I/O configuration
 */
typedef ad_i2c_io_conf_t snc_i2c_io_conf_t;

/**
 * \brief I2C driver configuration
 *
 * Configuration of I2C low level driver
 */
typedef ad_i2c_driver_conf_t snc_i2c_driver_conf_t;

/**
 * \brief I2C controller configuration
 *
 * Configuration of I2C controller
 */
typedef ad_i2c_controller_conf_t snc_i2c_controller_conf_t;

#endif /* dg_configI2C_ADAPTER */

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_i2c_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== Controller Acquisition functions ========================

/**
 * \brief Function used in SNC context to open I2C controller
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 *
 * \sa SNC_i2c_close
 */
#define SNC_i2c_open(conf)                                                                      \
        _SNC_i2c_open(conf)

/**
 * \brief Function used in SNC context to close I2C controller
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 *
 * \sa SNC_i2c_open
 */
#define SNC_i2c_close(conf)                                                                     \
        _SNC_i2c_close(conf)

//==================== Controller Configuration functions ======================

/**
 * \brief Function used in SNC context to reconfigure I2C controller
 *
 * This function will apply a new I2C driver configuration implied by the given I2C controller
 * configuration.
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 *
 * \sa SNC_i2c_open
 * \sa SNC_i2c_close
 */
#define SNC_i2c_reconfig(conf)                                                                  \
        _SNC_i2c_reconfig(conf)


//==================== Status Acquisition functions ============================

/**
 * \brief Function used in SNC context to poll for ACK on I2C bus before starting any write
 *        transfer to a slave device
 *
 * This is a BLOCKING function that performs multiple write transfers until the slave device
 * responds with ACK.
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 */
#define SNC_i2c_poll_for_ack(conf)                                                              \
        _SNC_i2c_poll_for_ack(conf)

/**
 * \brief Function used in SNC context to check for abort source (if any) causing I2C transaction
 *        to stop
 *
 * This function can be used in order to retrieve the source that has caused I2C transaction to
 * be aborted. If the transaction was successful, then no abort source is indicated
 * (i.e. HW_I2C_ABORT_NONE). Abort source indication is cleared accordingly when using this macro.
 *
 * \param [in] conf             (const snc_i2c_controller_conf_t*: build-time-only value)
 *                              I2C controller configuration
 * \param [out] abort_source    (uint32_t*: use da() or ia())
 *                              abort source bitmask (HW_I2C_ABORT_SOURCE)
 */
#define SNC_i2c_check_abort_source(conf, abort_source)                                          \
        _SNC_i2c_check_abort_source(conf, abort_source)

//==================== Data Read/Write functions ===============================

/**
 * \brief Function used in SNC context to perform a blocking write-only I2C transaction
 *
 * This function performs a write-only transaction on I2C bus using the internal FIFO.
 * The contents of out_buf address are sent over I2C bus.
 *
 * This function is blocking. It waits until transaction is completed.
 * STOP and RESTART conditions will be generated according to flags parameter value.
 *
 * Two modes are supported based on the addressing mode of out_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(command)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&command_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_i2c_controller_config_t _I2C_CONF1 = {HW_I2C1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const I2C_CONF1 = &_I2C_CONF1;
 * uint32_t command[1] = {0x20};
 * uint32_t* command_ind = &command[0];
 *
 * {
 *         ...
 *         SNC_i2c_open(I2C_CONF1);
 *         SNC_i2c_write(I2C_CONF1, ia(&command_ind), sizeof(command)/sizeof(uint32_t),
 *                                  SNC_HW_I2C_FLAG_ADD_STOP);
 *         SNC_i2c_close(I2C_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 * \param [in] out_buf  (uint32_t*: use da() or ia())
 *                      buffer containing the data to be sent over the I2C bus
 * \param [in] out_len  (uint32_t: use da() or ia() or build-time-only value)
 *                      size of data in bytes to be sent over the I2C bus,
 *                      where 1 byte of data is stored to each 32bit word of out_buf
 * \param [in] flags    (SNC_HW_I2C_ACCESS_FLAGS: build-time-only value)
 *                      I2C access flags for this transfer (SNC_HW_I2C_ACCESS_FLAGS)
 *
 * \sa SNC_i2c_open
 * \sa SNC_i2c_close
 */
#define SNC_i2c_write(conf, out_buf, out_len, flags)                                            \
        _SNC_i2c_write(conf, out_buf, out_len, flags)

/**
 * \brief Function used in SNC context to perform a blocking read-only I2C transaction
 *
 * This function performs a read-only transaction on I2C bus using the internal FIFO.
 * The data received over I2C bus are stored to in_buf address.
 *
 * This function is blocking. It waits until transaction is completed.
 * STOP and RESTART conditions will be generated according to flags parameter value.
 *
 * Two modes are supported based on the addressing mode of in_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(response)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&response_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_i2c_controller_config_t _I2C_CONF1 = {HW_I2C1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const I2C_CONF1 = &_I2C_CONF1;
 * uint32_t response[2] = {0};
 * uint32_t* response_ind = &response[0];
 *
 * {
 *         ...
 *         SNC_i2c_open(I2C_CONF1);
 *         SNC_i2c_read(I2C_CONF1, ia(&response_ind), sizeof(response)/sizeof(uint32_t),
 *                                 SNC_HW_I2C_FLAG_ADD_STOP);
 *         SNC_i2c_close(I2C_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf     (const snc_i2c_controller_conf_t*: build-time-only value)
 *                      I2C controller configuration
 * \param [out] in_buf  (uint32_t*: use da() or ia())
 *                      buffer for incoming data
 * \param [in] in_len   (uint32_t: use da() or ia() or build-time-only value)
 *                      size of data in bytes to be read over the I2C bus,
 *                      where 1 byte of data is stored to each 32bit word of in_buf
 * \param [in] flags    (SNC_HW_I2C_ACCESS_FLAGS: build-time-only value)
 *                      I2C access flags for this transfer (SNC_HW_I2C_ACCESS_FLAGS)
 *
* \sa SNC_i2c_open
* \sa SNC_i2c_close
 */
#define SNC_i2c_read(conf, in_buf, in_len, flags)                                               \
        _SNC_i2c_read(conf, in_buf, in_len, flags)

#endif /* dg_configUSE_SNC_HW_I2C */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_I2C_H_ */

/**
 * \}
 * \}
 */
