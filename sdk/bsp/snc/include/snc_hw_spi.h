/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_SPI
 *
 * \brief SPI LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_spi.h
 *
 * @brief SNC-Definition of SPI Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_SPI_H_
#define SNC_HW_SPI_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_SPI

#include "snc_defs.h"
#include "ad.h"
#include "hw_spi.h"

/**
 * \def CONFIG_SNC_SPI_USE_ADVANCED_WRITE
 *
 * \brief Controls whether advanced byte-wise write-only SPI transaction will be enabled
 *
 * If advanced byte-wise write-only SPI transaction (i.e. SNC_spi_write_advanced()) is not to be
 * used, setting this macro to 0 will save retention RAM required for the corresponding
 * implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_USE_ADVANCED_WRITE
#define CONFIG_SNC_SPI_USE_ADVANCED_WRITE       (1)
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

/**
 * \def CONFIG_SNC_SPI_USE_ADVANCED_READ
 *
 * \brief Controls whether advanced byte-wise read-only SPI transaction will be enabled
 *
 * If advanced byte-wise read-only SPI transaction (i.e. SNC_spi_read_advanced()) is not to be
 * used, setting this macro to 0 will save retention RAM required for the corresponding
 * implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_USE_ADVANCED_READ
#define CONFIG_SNC_SPI_USE_ADVANCED_READ        (1)
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

/**
 * \def CONFIG_SNC_SPI_USE_8BIT_WORD_MODE
 *
 * \brief Controls whether SPI 8bit word-size mode will be supported
 *
 * If SPI 8bit word-size mode is not to be used, setting this macro to 0 will save retention RAM
 * required for the corresponding implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_USE_8BIT_WORD_MODE
#define CONFIG_SNC_SPI_USE_8BIT_WORD_MODE       (1)
#endif /* CONFIG_SNC_SPI_USE_8BIT_WORD_MODE */

/**
 * \def CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
 *
 * \brief Controls whether SPI 16bit word-size mode will be supported
 *
 * If SPI 16bit word-size mode is not to be used, setting this macro to 0 will save retention RAM
 * required for the corresponding implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_USE_16BIT_WORD_MODE
#define CONFIG_SNC_SPI_USE_16BIT_WORD_MODE      (1)
#endif /* CONFIG_SNC_SPI_USE_16BIT_WORD_MODE */

/**
 * \def CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
 *
 * \brief Controls whether SPI 32bit word-size mode will be supported
 *
 * If SPI 32bit word-size mode is not to be used, setting this macro to 0 will save retention RAM
 * required for the corresponding implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_USE_32BIT_WORD_MODE
#define CONFIG_SNC_SPI_USE_32BIT_WORD_MODE      (1)
#endif /* CONFIG_SNC_SPI_USE_32BIT_WORD_MODE */

/**
 * \def CONFIG_SNC_SPI_ENABLE_RECONFIG
 *
 * \brief Controls whether SNC_spi_reconfig() will be supported
 *
 * If SNC_spi_reconfig() is not to be used, setting this macro to 0 will save retention RAM
 * required for the corresponding implementation in SNC uCode.
 */
#ifndef CONFIG_SNC_SPI_ENABLE_RECONFIG
#define CONFIG_SNC_SPI_ENABLE_RECONFIG          (1)
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

#if (dg_configSPI_ADAPTER == 0)
/**
 * \brief SPI I/O configuration
 *
 * SPI I/O configuration
 */
typedef struct {
        ad_io_conf_t spi_do;            /**< Configuration of DO signal */
        ad_io_conf_t spi_di;            /**< Configuration of DI signal */
        ad_io_conf_t spi_clk;           /**< Configuration of CLK signal */
        uint8_t cs_cnt;                 /**< Number of configured CS pins */
        const ad_io_conf_t* spi_cs;     /**< Configuration of CS's signals */
        HW_GPIO_POWER voltage_level;    /**< Signals' voltage level */
} snc_spi_io_conf_t;

/**
 * \brief SPI driver configuration
 *
 * Configuration of SPI low level driver
 */
typedef struct {
        spi_config spi;                 /**< Configuration of SPI low level driver */
} snc_spi_driver_conf_t;

/**
 * \brief SPI controller configuration
 *
 * Configuration of SPI controller
 */
typedef struct {
        const HW_SPI_ID id;                     /**< Controller instance */
        const snc_spi_io_conf_t *io;            /**< IO configuration */
        const snc_spi_driver_conf_t *drv;       /**< Driver configuration */
} snc_spi_controller_conf_t;

#else
#include "ad_spi.h"

/**
 * \brief SPI I/O configuration
 *
 * SPI I/O configuration
 */
typedef ad_spi_io_conf_t snc_spi_io_conf_t;

/**
 * \brief SPI driver configuration
 *
 * Configuration of SPI low level driver
 */
typedef ad_spi_driver_conf_t snc_spi_driver_conf_t;

/**
 * \brief SPI controller configuration
 *
 * Configuration of SPI controller
 */
typedef ad_spi_controller_conf_t snc_spi_controller_conf_t;

#endif /* dg_configSPI_ADAPTER */

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_spi_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== Controller Acquisition functions ========================

/**
 * \brief Function used in SNC context to open SPI controller
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 *
 * \sa SNC_spi_close
 */
#define SNC_spi_open(conf)                                                                      \
        _SNC_spi_open(conf)

/**
 * \brief Function used in SNC context to close SPI controller
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 *
 * \sa SNC_spi_open
 */
#define SNC_spi_close(conf)                                                                     \
        _SNC_spi_close(conf)

//==================== Controller Configuration functions ======================

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
/**
 * \brief Function used in SNC context to reconfigure SPI controller
 *
 * This function will apply a new SPI driver configuration implied by the given SPI controller
 * configuration.
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 */
#define SNC_spi_reconfig(conf)                                                                  \
        _SNC_spi_reconfig(conf)
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */


//==================== CS handling functions ===================================

/**
 * \brief Function used in SNC context to activate chip select for a specific device
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 */
#define SNC_spi_activate_cs(conf)                                                               \
        _SNC_spi_activate_cs(conf)

/**
 * \brief Function used in SNC context to deactivate chip select for a specific device
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 */
#define SNC_spi_deactivate_cs(conf)                                                             \
        _SNC_spi_deactivate_cs(conf)

//==================== Data Read/Write functions ===============================

/**
 * \brief Function used in SNC context to perform a typical word-wise blocking write-only SPI
 *        transaction using the internal FIFO
 *
 * This function performs a typical word-wise write-only transaction on SPI bus using the internal
 * FIFO. The contents of out_buf address are sent over SPI bus.
 *
 * This function is blocking. It waits until transaction is completed.
 *
 * Two modes are supported based on the addressing mode of out_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(command)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&command_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_spi_controller_config_t _SPI_CONF1 = {HW_SPI1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const SPI_CONF1 = &_SPI_CONF1;
 * uint32_t command[2] = {0x0, 0x1};
 * uint32_t* command_ind = &command[1];
 *
 * {
 *         ...
 *         SNC_spi_open(SPI_CONF1);
 *         SNC_spi_activate_cs(SPI_CONF1);
 *
 *         // Assuming configured word size mode is HW_SPI_WORD_8BIT, the following statements
 *         // send over SPI bus one byte stored to a 32bit word of command buffer
 *         // (1-byte each - the first)
 *         SNC_spi_write(SPI_CONF1, da(&command[0]), 1);
 *         SNC_spi_write(SPI_CONF1, ia(&command_ind), 1);
 *
 *         SNC_spi_deactivate_cs(SPI_CONF1);
 *         SNC_spi_close(SPI_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 * \param [in] out_buf  (uint32_t*: use da() or ia())
 *                      buffer containing the data to be sent over the SPI bus
 * \param [in] out_len  (uint32_t: use da() or ia() or build-time-only value)
 *                      number of data words to be sent over the SPI bus,
 *                      where 1 word of data is stored to each 32bit word of out_buf
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 * \sa SNC_spi_activate_cs
 * \sa SNC_spi_deactivate_cs
 */
#define SNC_spi_write(conf, out_buf, out_len)                                                   \
        _SNC_spi_write(conf, out_buf, out_len)

/**
 * \brief Function used in SNC context to perform a typical word-wise blocking read-only SPI
 *        transaction using the internal FIFO
 *
 * This function performs a typical word-wise read-only transaction on SPI bus using the internal
 * FIFO. The data received over SPI bus are stored to in_buf address.
 *
 * This function is blocking. It waits until transaction is completed.
 *
 * Two modes are supported based on the addressing mode of in_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(response)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&response_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_spi_controller_config_t _SPI_CONF1 = {HW_SPI1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const SPI_CONF1 = &_SPI_CONF1;
 * uint32_t response[10];
 * uint32_t* response_ind = &response[0];
 *
 * {
 *         ...
 *         SNC_spi_open(SPI_CONF1);
 *         SNC_spi_activate_cs(SPI_CONF1);
 *
 *         // Assuming configured word size mode is HW_SPI_WORD_8BIT, the following statements
 *         // read 10 bytes over SPI bus and store them to the 32-bit aligned response buffer words
 *         // (1-byte each - the first)
 *         SNC_spi_read(SPI_CONF1, da(response), sizeof(response)/sizeof(uint32_t));
 *         SNC_spi_read(SPI_CONF1, ia(&response_ind), sizeof(response)/sizeof(uint32_t));
 *
 *         SNC_spi_deactivate_cs(SPI_CONF1);
 *         SNC_spi_close(SPI_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 * \param [out] in_buf  (uint32_t*: use da() or ia())
 *                      buffer for incoming data
 * \param [in] in_len   (uint32_t: use da() or ia() or build-time-only value)
 *                      number of data words to be read over the SPI bus,
 *                      where 1 word of data is stored to each 32bit word of in_buf
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 * \sa SNC_spi_activate_cs
 * \sa SNC_spi_deactivate_cs
 */
#define SNC_spi_read(conf, in_buf, in_len)                                                      \
        _SNC_spi_read(conf, in_buf, in_len)

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
/**
 * \brief Function used in SNC context to perform an advanced byte-wise blocking write-only SPI
 *        transaction using the internal FIFO and configurable SPI word size mode
 *
 * This function performs an advanced byte-wise write-only transaction on SPI bus using the internal
 * FIFO. The word size mode being used for the SPI transaction is given as argument.
 * The contents of out_buf address are sent over SPI bus.
 *
 * This function is blocking. It waits until transaction is completed.
 *
 * Two modes are supported based on the addressing mode of out_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(command)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&command_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * If the size of data words in bytes is not a multiple of the size in bytes implied by the given
 * SPI word size mode, then SPI word size mode changes to HW_SPI_WORD_8BIT for the remaining bytes.
 *
 * Example usage:
 * \code{.c}
 * const snc_spi_controller_config_t _SPI_CONF1 = {HW_SPI1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const SPI_CONF1 = &_SPI_CONF1;
 * uint32_t command[2] = {0x0, 0x1};
 * uint32_t* command_ind = &command[1];
 *
 * {
 *         ...
 *         SNC_spi_open(SPI_CONF1);
 *         SNC_spi_activate_cs(SPI_CONF1);
 *
 *         // The following statements send over SPI bus one byte stored to a 32bit word
 *         // of command buffer
 *         SNC_spi_write_advanced(SPI_CONF1, HW_SPI_WORD_32BIT, da(&command[0]), 1);
 *         SNC_spi_write_advanced(SPI_CONF1, HW_SPI_WORD_32BIT, ia(&command_ind), 1);
 *
 *         SNC_spi_deactivate_cs(SPI_CONF1);
 *         SNC_spi_close(SPI_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf             (const snc_spi_controller_conf_t*: build-time-only value)
 *                              SPI controller configuration
 * \param [in] out_word_mode    (uint32_t: use da() or ia() or build-time-only value)
 *                              the SPI word size mode to be used when writing (HW_SPI_WORD)
 * \param [in] out_buf          (uint32_t*: use da() or ia())
 *                              buffer containing the data to be sent over the SPI bus
 * \param [in] out_len          (uint32_t: use da() or ia() or build-time-only value)
 *                              size of data words in bytes to be sent over the SPI bus,
 *                              where 1 word of data is stored to each 32bit word of out_buf
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 * \sa SNC_spi_activate_cs
 * \sa SNC_spi_deactivate_cs
 */
#define SNC_spi_write_advanced(conf, out_word_mode, out_buf, out_len)                           \
        _SNC_spi_write_advanced(conf, out_word_mode, out_buf, out_len)
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
/**
 * \brief Function used in SNC context to perform an advanced byte-wise blocking read-only SPI
 *        transaction using the internal FIFO and configurable SPI word size mode
 *
 * This function performs an advanced byte-wise read-only transaction on SPI bus using the internal
 * FIFO. The word size mode being used for the SPI transaction is given as argument. The data
 * received over SPI bus are stored to in_buf address.
 *
 * This function is blocking. It waits until transaction is completed.
 *
 * Two modes are supported based on the addressing mode of in_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(response)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&response_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * If the size of data words in bytes is not a multiple of the size in bytes implied by the given
 * SPI word size mode, then SPI word size mode changes to HW_SPI_WORD_8BIT for the remaining bytes.
 *
 * Example usage:
 * \code{.c}
 * const snc_spi_controller_config_t _SPI_CONF1 = {HW_SPI1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const SPI_CONF1 = &_SPI_CONF1;
 * uint32_t response[10];
 * uint32_t* response_ind = &response[0];
 *
 * {
 *         ...
 *         SNC_spi_open(SPI_CONF1);
 *         SNC_spi_activate_cs(SPI_CONF1);
 *
 *         // Reading 40 bytes over SPI bus and storing them to the 32bit-aligned response buffer
 *         // words (4-bytes each in Big-Endian)
 *         SNC_spi_read_advanced(SPI_CONF1, HW_SPI_WORD_32BIT, ia(&response_ind),
 *                 sizeof(response));
 *
 *         SNC_spi_deactivate_cs(SPI_CONF1);
 *         SNC_spi_close(SPI_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf             (const snc_spi_controller_conf_t*: build-time-only value)
 *                              SPI controller configuration
 * \param [in] in_word_mode     (uint32_t: use da() or ia() or build-time-only value)
 *                              the SPI word size mode to be used when reading (HW_SPI_WORD)
 * \param [out] in_buf          (uint32_t*: use da() or ia())
 *                              buffer for incoming data
 * \param [in] in_len           (uint32_t: use da() or ia() or build-time-only value)
 *                              size of data words in bytes to be read over the SPI bus,
 *                              where 1 word of data is stored to each 32bit word of in_buf
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 * \sa SNC_spi_activate_cs
 * \sa SNC_spi_deactivate_cs
 */
#define SNC_spi_read_advanced(conf, in_word_mode, in_buf, in_len)                               \
        _SNC_spi_read_advanced(conf, in_word_mode, in_buf, in_len)
#endif /*CONFIG_SNC_SPI_USE_ADVANCED_READ */

/**
 * \brief Function used in SNC context to perform a typical word-wise blocking write-read SPI
 *        transaction WITHOUT using the internal FIFOs
 *
 * This function performs a typical word-size write-read transaction on SPI bus without using the
 * internal FIFOs The contents of out_buf address are sent over SPI bus, and the data received over
 * SPI bus are stored to in_buf address.
 *
 * This function is blocking. It waits until transaction is completed.
 *
 * Two modes are supported based on the addressing mode of out_buf and in_buf addresses being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(command)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&response_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_spi_controller_config_t _SPI_CONF1 = {HW_SPI1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const SPI_CONF1 = &_SPI_CONF1;
 * uint32_t command[1] = {0x0};
 * uint32_t response[10];
 * uint32_t* response_ind = &response[0];
 *
 * {
 *         ...
 *         SNC_spi_open(SPI_CONF1);
 *         SNC_spi_activate_cs(SPI_CONF1);
 *
 *         // Assuming configured word size mode is HW_SPI_WORD_8BIT, the following statement
 *         // writes one byte and reads 10 bytes over SPI bus, stored to 32bit words of
 *         // command (1-byte each - the first) and response (1-byte each - the first) buffers,
 *         // respectively
 *         SNC_spi_writeread_buf(SPI_CONF1, da(command), sizeof(command)/sizeof(uint32_t),
 *                                          ia(&response_ind), sizeof(response)/sizeof(uint32_t));
 *
 *         SNC_spi_deactivate_cs(SPI_CONF1);
 *         SNC_spi_close(SPI_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf     (const snc_spi_controller_conf_t*: build-time-only value)
 *                      SPI controller configuration
 * \param [in] out_buf  (uint32_t*: use da() or ia())
 *                      buffer containing the data to be sent over the SPI bus
 * \param [in] out_len  (uint32_t: use da() or ia() or build-time-only value)
 *                      size of data in 32bit words to be sent over the SPI bus,
 *                      where 1 word of data is stored to each 32bit word of out_buf
 * \param [out] in_buf  (uint32_t*: use da() or ia())
 *                      buffer for incoming data
 * \param [in] in_len   (uint32_t: use da() or ia() or build-time-only value)
 *                      number of data words to be read over the SPI bus,
 *                      where 1 word of data is stored to each 32bit word of in_buf
 *
 * \sa SNC_spi_open
 * \sa SNC_spi_close
 * \sa SNC_spi_activate_cs
 * \sa SNC_spi_deactivate_cs
 */
#define SNC_spi_writeread_buf(conf, out_buf, out_len, in_buf, in_len)                           \
        _SNC_spi_writeread_buf(conf, out_buf, out_len, in_buf, in_len)

#endif /* dg_configUSE_SNC_HW_SPI */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_SPI_H_ */

/**
 * \}
 * \}
 */
