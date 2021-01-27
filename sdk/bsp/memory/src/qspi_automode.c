/**
 ****************************************************************************************
 *
 * @file qspi_automode.c
 *
 * @brief Access QSPI flash when running in auto mode
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
#include "osal.h"
#include "sys_power_mgr.h"
#include "../system/sys_man/sys_power_mgr_internal.h"
#endif

#include "hw_clk.h"

#include "hw_qspi.h"
#include "qspi_automode.h"
#include "qspi_internal.h"

static bool flash_is_busy(HW_QSPIC_ID id);
static void flash_write_enable(HW_QSPIC_ID id);
static void flash_write_status_register(HW_QSPIC_ID id, uint8_t value);
static void flash_transact(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen, uint8_t *rbuf, uint32_t rlen);
static void flash_write(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen);
static uint8_t flash_read_status_register(HW_QSPIC_ID id);
static void init_hw_qspi(HW_QSPIC_ID id);

/*
 * QSPI controller allows to execute code directly from QSPI flash.
 * When code is executing from flash there is no possibility to reprogram it.
 * To be able to modify flash memory while it is used for code execution it must me assured that
 * during the time needed for erase/write no code is running from flash.
 */

/*
 * Flash specific defines
 */


/*
 * Use QUAD mode for page write.
 *
 * Note: If the flash does not support QUAD mode or it is not connected for QUAD mode set it to 0
 * (single mode).
 */
#ifndef QUAD_MODE
#define QUAD_MODE                       1
#endif

#ifndef ERASE_IN_AUTOMODE
#define ERASE_IN_AUTOMODE               1
#endif

#ifndef FLASH_FORCE_24BIT_ADDRESSING
#define FLASH_FORCE_24BIT_ADDRESSING    0       // Force 24 bit addressing for devices > 128Mbits
#endif

/*
 * WARNING: The Autodetect mode will increase both the code and the used RetRAM size!!!!
 *          Use with extreme caution!!
 */
#if (dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0)
        #if !defined(dg_configFLASH_CONFIG)
        #error Please define dg_configFLASH_CONFIG !!!
        #endif
        #endif

#if (dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0)
        #if !defined(dg_configQSPIC2_DEV_CONFIG)
        #error Please define dg_configQSPIC2_DEV_CONFIG !!!
        #endif
#endif

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)

/**
 * \brief Background operation type
 *
 */
typedef enum {
        BACKGROUND_OP_ERASE,
        BACKGROUND_OP_PROGRAM,
} BACKGROUND_OP_TYPE;

/**
 * \brief Background Flash operation data
 *
 * Structure keeps data related to a background flash operation
 *
 */
typedef struct _pm_qspi_ops {
        OS_TASK handle;                 /**< handle the handle of the task that registers the operation */
        uint32_t addr;                  /**< flash address for the operation */
        const uint8_t *buf;             /**< the buffer containing the data to be written */
        uint32_t *size;                 /**< pointer to the the length of the data in the buffer */
        uint32_t written;               /**< the number of bytes written by the write operation */
        BACKGROUND_OP_TYPE op_type;     /**< Operation type, either erase or program */
        bool suspended;                 /**< true if operation is suspended */
        struct _pm_qspi_ops *next;      /**< pointer to the next operation */
} qspi_ops;
#endif

#if (FLASH_AUTODETECT == 1)
#               include "qspi_gd25le32.h"
#               include "qspi_mx25u3235.h"
#               include "qspi_w25q32fw.h"
#               include "qspi_at25sl321.h"
#               include "psram_aps1604jsq.h"
#               include "psram_aps3204jsq.h"
#               include "psram_aps6404jsq.h"

static const qspi_flash_config_t* flash_config_table[] = {
                &flash_gd25le32_config,
                &flash_mx25u3235_config,
                &flash_w25q32fw_config,
                &flash_at25sl321_config,
                &psram_aps1604jsq_config,
                &psram_aps3204jsq_config,
                &psram_aps6404jsq_config,
};
#else
#       if ((dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0))
#               ifndef dg_configFLASH_HEADER_FILE
#                      error Please define macro dg_configFLASH_HEADER_FILE to the header file name that contains the respective implementation
#               endif
#               include dg_configFLASH_HEADER_FILE
#       endif
#       if ((dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0))
#               ifndef dg_configQSPIC2_DEV_HEADER_FILE
#                       error Please define macro dg_configQSPIC2_DEV_HEADER_FILE to the header file name that contains the respective implementation
#               endif
#              include dg_configQSPIC2_DEV_HEADER_FILE
#       endif
#endif /* FLASH_AUTODETECT */

#if FLASH_AUTODETECT
__RETAINED qspi_flash_config_t flash_config[QSPI_CONTROLLER_SUPPORT_NUM];
#endif

__RETAINED static bool qspi_is_device_present[QSPI_CONTROLLER_SUPPORT_NUM];

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
__RETAINED static qspi_ops *qspi_pending_ops;
static qspi_ops *qspi_active_op;
__RETAINED static OS_MUTEX xSemaphoreQSPI;
#endif

#if FLASH_AUTODETECT
#       if (dg_configUSE_HW_QSPI2 == 1)
#               define QSPI_GET_CONFIG_IDX(id) (id == HW_QSPIC ? 0 : 1)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (idx == 0 ? HW_QSPIC : HW_QSPIC2)
#       else
#               define QSPI_GET_CONFIG_IDX(id) (0)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC)
#       endif
#       define QSPI_GET_DEVICE_PARAM(idx, param) (flash_config[idx].param)
#else
#       if (dg_configUSE_HW_QSPI == 1) && (dg_configUSE_HW_QSPI2 == 0)
#               define QSPI_GET_CONFIG_IDX(id)       (0) // Value is not used. It is defined to suppress errors
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (dg_configFLASH_CONFIG.param)
#       elif (dg_configUSE_HW_QSPI == 0) && (dg_configUSE_HW_QSPI2 == 1)
#               define QSPI_GET_CONFIG_IDX(id)       (1) // Value is not used. It is defined to suppress errors
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC2)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (dg_configQSPIC2_DEV_CONFIG.param)
#       else
#               define QSPI_GET_CONFIG_IDX(id) (id == HW_QSPIC ? 0 : 1)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (idx == 0 ? HW_QSPIC : HW_QSPIC2)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (idx == 0 ? \
                                dg_configFLASH_CONFIG.param : dg_configQSPIC2_DEV_CONFIG.param)
#       endif
#endif

/*
 * Function definitions
 */
/**
 * \brief Get the QSPI controller id from the address of data accessed
 *
 * \param[in] addr The address of data accessed. It may be anywhere in a page.
 * \param[in] size The number of bytes accessed.
 *
 * \return QSPI controller id
 */
__STATIC_FORCEINLINE HW_QSPIC_ID flash_get_addr_id(uint32_t addr, uint32_t size)
{
        ASSERT_WARNING(size > 0);

        ASSERT_WARNING(qspi_is_valid_addr(addr + size - 1));
#if dg_configUSE_HW_QSPI2
        if (addr >= dg_configQSPI2_FLASH_BASE_ADDR) {
                return HW_QSPIC2;
        }
#endif
        return HW_QSPIC;
}

__STATIC_FORCEINLINE uint32_t flash_get_zero_based_addr(uint32_t addr)
{
#if dg_configUSE_HW_QSPI2
        if (addr >= dg_configQSPI2_FLASH_BASE_ADDR) {
                return addr - dg_configQSPI2_FLASH_BASE_ADDR;
        }
#endif
        return addr;
}


__RETAINED_CODE bool qspi_is_valid_addr(uint32_t addr)
{
#if dg_configUSE_HW_QSPI2
        if (addr >= (dg_configQSPI2_FLASH_BASE_ADDR + HW_QSPI_MAX_ADDR_SIZE)) {
                return false;
        }

        if (addr >= dg_configQSPI2_FLASH_BASE_ADDR) {
                if (qspi_is_device_present[QSPI_GET_CONFIG_IDX(HW_QSPIC2)]) {
                        return (((addr - dg_configQSPI2_FLASH_BASE_ADDR)* 8) <=
                                QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(HW_QSPIC2),memory_size));
                } else {
                        return false;
                }
        }
#endif
        if (qspi_is_device_present[QSPI_GET_CONFIG_IDX(HW_QSPIC)] == false || addr >= HW_QSPI_MAX_ADDR_SIZE) {
                return false;
        }

        return (addr * 8) <= QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(HW_QSPIC), memory_size);
}

uint32_t qspi_get_device_size(HW_QSPIC_ID id)
{
        return (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), memory_size) / 8);
}

bool qspi_get_config(HW_QSPIC_ID id, uint8_t *manufacturer_id, uint8_t *device_type,
                                                                                uint8_t *density)
{
        if (!qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)]) {
                return false;
        }

        *manufacturer_id = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), manufacturer_id);
        *device_type = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), device_type);
        *density = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), device_density);

        return true;
}

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)

/**
 * \brief Register a program or erase QSPI operation to be executed in background
 *
 * \param [in]    handle the handle of the task that registers the operation
 * \param [in]    addr starting offset
 * \param [in]    buf the buffer containing the data to be written
 * \param [inout] size pointer to the length of the data in the buffer
 * \param [out]   operation pointer to a structure allocated in this function that must be freed later
 *                by the caller of this function
 *
 * \returns true if the operation was registered successfully, else false
 */
static bool register_qspi_operation(OS_TASK handle, uint32_t addr, const uint8_t *buf, uint32_t *size,
        void **operation)
{
        qspi_ops *op;

        if (xSemaphoreQSPI == NULL) {
                return false;
        }

        op = (qspi_ops *)OS_MALLOC(sizeof(qspi_ops));
        ASSERT_ERROR(op != NULL);

        *operation = (void *)op;

        OS_EVENT_WAIT(xSemaphoreQSPI, OS_EVENT_FOREVER);    // Block forever

        op->handle = handle;
        op->addr = addr;
        op->buf = buf;
        op->size = size;
        op->written = 0;
        if (buf == NULL) {
                op->op_type = BACKGROUND_OP_ERASE;
        } else {
                op->op_type = BACKGROUND_OP_PROGRAM;
        }
        op->suspended = false;
        op->next = NULL;

        if (qspi_pending_ops) {
                qspi_ops *p = qspi_pending_ops;

                while (p->next != NULL) {
                        p = p->next;
                }

                p->next = op;
        } else {
                qspi_pending_ops = op;
        }

        OS_EVENT_SIGNAL(xSemaphoreQSPI);

        return true;
}

/**
 * \brief Monitor the progress of an ongoing program QSPI operation
 *
 * \param[out] bool true if the operation is still in progress else false
 *
 * \returns true if an interrupt is pending, else false
 */
__RETAINED_CODE static bool process_qspi_program_finish(bool *in_progress)
{
        bool pending_irq = false;
        qspi_ops *op;

        op = qspi_pending_ops;

        HW_QSPIC_ID id = flash_get_addr_id(op->addr, *op->size);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL);

        do {
#if ( defined(__CORE_CM33_H_DEPENDANT) || defined(__CORE_CM3_H_DEPENDANT) )
                if ((NVIC->ISER[0] & NVIC->ISPR[0]) || (NVIC->ISER[1] & NVIC->ISPR[1]))
#elif ( defined(__CORE_CM0PLUS_H_DEPENDANT) || defined(__CORE_CM0_H_DEPENDANT) )
                if (NVIC->ISER[0] & NVIC->ISPR[0])
#endif
                {
                        pending_irq = true;

                        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL_IRQ);
                }

                *in_progress = qspi_check_program_erase_in_progress(id);

        } while (!pending_irq && *in_progress);

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL_IRQ);
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG_WL);

        return pending_irq;
}

/**
 * \brief Issue (or continue execution of a) program QSPI operation
 *
 * \returns true if the operation was completed, else false (i.e. an interrupt is pending)
 */
__RETAINED_CODE static bool process_qspi_program(void)
{
        uint32_t *p_size;
        qspi_ops *op;
        bool pending_irq;
        bool write_completed = false;

        op = qspi_pending_ops;
        p_size = op->size;

        do {
                bool in_progress;

                op->written += qspi_int_program_page_manual_mode(op->addr + op->written,
                                        op->buf + op->written, *p_size - op->written);

                /* Check if the operation has finished or if an interrupt
                 * is pending.
                 */
                pending_irq = process_qspi_program_finish(&in_progress);
                if (!in_progress && (op->written == *p_size)) {
                        // Notify the waiting task without delay
                        write_completed = true;
                }
        } while (!pending_irq && (op->written < *p_size));

        return write_completed;
}

#endif /* dg_configDISABLE_BACKGROUND_FLASH_OPS */

void qspi_operations_init(void)
{
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        xSemaphoreQSPI = xSemaphoreCreateMutex();         // Create Mutex
        ASSERT_WARNING(xSemaphoreQSPI != NULL);
#endif
}

__RETAINED_CODE bool qspi_process_operations(void)
{
        bool write_completed = false;

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        qspi_active_op = qspi_pending_ops;

        if (qspi_active_op != NULL) {
                HW_QSPIC_ID id = flash_get_addr_id(qspi_active_op->addr, *qspi_active_op->size);

                if (qspi_active_op->suspended) {
                        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_RESUME);

                        qspi_resume(id);
                        hw_clk_delay_usec(FLASH_RES_DELAY); // Guard time until RESUME can be issued!
                        qspi_active_op->suspended = false;

                        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_RESUME);

                        if (qspi_active_op->op_type == BACKGROUND_OP_PROGRAM) {
                                bool pending_irq, in_progress;

                                pending_irq = process_qspi_program_finish(&in_progress);
                                if (!in_progress) {
                                        if (qspi_active_op->written == *qspi_active_op->size) {
                                                // Notify the waiting task without delay
                                                write_completed = true;
                                        }

                                        if (!pending_irq && !write_completed) {
                                                // More data to write...
                                                write_completed = process_qspi_program();
                                        }
                                }
                        }
                } else {
                        if (qspi_active_op->op_type == BACKGROUND_OP_ERASE) {
                                DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);

                                qspi_int_erase_sector_manual_mode(qspi_active_op->addr);

                                DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);
                        } else {                        // program
                                qspi_int_activate_command_entry_mode(id);
                                write_completed = process_qspi_program();
                        }
                }
        }
#endif /* dg_configDISABLE_BACKGROUND_FLASH_OPS */

        return write_completed;
}

__RETAINED_CODE  void qspi_process_completed_operations(void)
{
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        qspi_ops *op;

        op = qspi_pending_ops;

        if (op && (op->suspended == false)) {
                DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_TASK_NOTIFY);

                if (((op->op_type == BACKGROUND_OP_ERASE) && (op->written == 1))
                        || ((op->op_type == BACKGROUND_OP_PROGRAM) && (op->written == *(op->size)))) {
                        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_TASK_NOTIFY);
                        // The QSPI operation has been completed
                        if ((op->op_type == BACKGROUND_OP_PROGRAM)  && (op->written != 0)) {
                                *(op->size) = op->written;
                        }
                        qspi_pending_ops = op->next;
                        OS_TASK_RESUME(op->handle);
                        // Calling portYIELD() here is needless (and an error...)
                        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_TASK_NOTIFY);
                }
        }
#endif
}

bool qspi_is_op_pending(void)
{
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        return qspi_pending_ops != NULL;
#else
        return false;
#endif
}

/**
 * \brief Set bus mode to single or QUAD mode.
 *
 * \param[in] id QSPI controller id
 * \param[in] mode Can be single (HW_QSPI_BUS_MODE_SINGLE) or quad (HW_QSPI_BUS_MODE_QUAD) mode.
 *
 * \note DUAL mode page program so is not supported by this function.
 */
__STATIC_FORCEINLINE void flash_set_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE mode)
{
#if dg_configUSE_HW_QSPI && dg_configUSE_HW_QSPI2
        ASSERT_WARNING((id == HW_QSPIC) || (id == HW_QSPIC2));
#elif dg_configUSE_HW_QSPI
        ASSERT_WARNING(id == HW_QSPIC);
#else
        ASSERT_WARNING(id == HW_QSPIC2);
#endif

        if (mode == HW_QSPI_BUS_MODE_SINGLE) {
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_SINGLE);
                id->QSPIC_CTRLMODE_REG |=
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_DAT, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_DAT, 1);
        } else {
#if QUAD_MODE
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_QUAD);
                id->QSPIC_CTRLMODE_REG &=
                        ~(BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, 1) |
                          BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, 1));
#endif
        }
}

/**
 * \brief Set device in QPI mode
 *
 * \param[in] id QSPI controller id
 *
 */
__RETAINED_CODE static void flash_enter_qpi_mode(HW_QSPIC_ID id)
{
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), qpi_mode)) {
                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), enter_qpi_opcode));
                hw_qspi_cs_disable(id);
                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
        }
}

/**
 * \brief Set the mode of the QSPI controller (manual or auto)
 *
 * \param[in] id QSPI controller id
 * \param[in] automode True for auto and false for manual mode setting.
 */
__STATIC_FORCEINLINE void flash_set_automode(HW_QSPIC_ID id, bool automode)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, AUTO_MD, automode);
}

/**
 * \brief Write to the Flash the contents of a buffer
 *
 * \param[in] id QSPI controller id
 * \param[in] wbuf Pointer to the beginning of the buffer
 * \param[in] wlen The number of bytes to be written
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__RETAINED_CODE static void flash_write(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen)
{
        uint32_t i;

        hw_qspi_cs_enable(id);

        for (i = 0; i < wlen; ++i) {
                hw_qspi_write8(id, wbuf[i]);
        }

        hw_qspi_cs_disable(id);
}

/**
 * \brief Write an arbitrary number of bytes to the Flash and then read an arbitrary number of bytes
 *       from the Flash in one transaction
 *
 * \param[in] id QSPI controller id
 * \param[in] wbuf Pointer to the beginning of the buffer that contains the data to be written
 * \param[in] wlen The number of bytes to be written
 * \param[in] rbuf Pointer to the beginning of the buffer than the read data are stored
 * \param[in] rlen The number of bytes to be read
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__RETAINED_CODE static void flash_transact(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen,
                                                                        uint8_t *rbuf, uint32_t rlen)
{
        uint32_t i;

        hw_qspi_cs_enable(id);

        for (i = 0; i < wlen; ++i) {
                hw_qspi_write8(id, wbuf[i]);
        }

        for (i = 0; i < rlen; ++i) {
                rbuf[i] = hw_qspi_read8(id);
        }

        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static bool flash_erase_program_in_progress(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd;

        cmd = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), read_erase_progress_opcode);

        flash_transact(id, &cmd, 1, &status, 1);

        return ((status & (1 << QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_in_progress_bit))) != 0)
                        == QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_in_progress_bit_high_level);
}

__RETAINED_CODE static bool flash_is_busy(HW_QSPIC_ID id)
{
        return (flash_read_status_register(id) & FLASH_STATUS_BUSY_MASK) != 0;
}

/**
 * \brief Exit from continuous mode.
 *
 * \param[in] id QSPI controller id
 */
__RETAINED_CODE static void flash_reset_continuous_mode(HW_QSPIC_ID id, HW_QSPI_BREAK_SEQ_SIZE break_seq_size)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, CMD_EXIT_CONTINUOUS_MODE);
        if (break_seq_size == HW_QSPI_BREAK_SEQ_SIZE_2B) {
                hw_qspi_write8(id, CMD_EXIT_CONTINUOUS_MODE);
        }
        hw_qspi_cs_disable(id);
}

/**
 * \brief Get Device ID when Flash is not in Power Down mode
 *
 * \param[in] id QSPI controller id
 * \return uint8_t The Device ID of the Flash
 *
 * \note The function blocks until the Flash executes the command.
 */
__RETAINED_CODE __UNUSED static uint8_t flash_get_device_id(HW_QSPIC_ID id)
{
        uint8_t device_id;

        hw_qspi_cs_enable(id);
        hw_qspi_write32(id, CMD_RELEASE_POWER_DOWN);
        device_id = hw_qspi_read8(id);
        hw_qspi_cs_disable(id);

        while (flash_is_busy(id));

        return device_id;
}

/**
 * \brief Set WEL (Write Enable Latch) bit of the Status Register of the Flash
 * \details The WEL bit must be set prior to every Page Program, Quad Page Program, Sector Erase,
 *       Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers
 *       instruction. In the case of Write Status Register command, any status bits will be written
 *       as non-volatile bits.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function blocks until the Flash has processed the command and it will be repeated if,
 *       for any reason, the command was not successfully executed by the Flash.
 */
__RETAINED_CODE static void flash_write_enable(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { CMD_WRITE_ENABLE };

        do {
                flash_write(id, cmd, 1);
                /* Verify */
                do {
                        status = flash_read_status_register(id);
                } while (status & FLASH_STATUS_BUSY_MASK);
        } while (!(status & FLASH_STATUS_WEL_MASK));
}

/**
 * \brief Read the Status Register 1 of the Flash
 *
 * \param[in] id QSPI controller id
 *
 * \return uint8_t The value of the Status Register 1 of the Flash.
 */
__RETAINED_CODE static uint8_t flash_read_status_register(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { CMD_READ_STATUS_REGISTER };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

/**
 * \brief Write the Status Register 1 of the Flash
 *
 * \param[in] id QSPI controller id
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE __UNUSED static void flash_write_status_register(HW_QSPIC_ID id, uint8_t value)
{
        uint8_t cmd[2] = { CMD_WRITE_STATUS_REGISTER, value };

        flash_write(id, cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

/**
 * \brief Fast copy of a buffer to a FIFO
 * \details Implementation of a fast copy of the contents of a buffer to a FIFO in assembly. All
 *        addresses are word aligned.
 *
 * \param[in] start Pointer to the beginning of the buffer
 * \param[in] end Pointer to the end of the buffer
 * \param[in] Pointer to the FIFO
 *
 * \warning No validity checks are made! It is the responsibility of the caller to make sure that
 *        sane values are passed to this function.
 */
__STATIC_FORCEINLINE void fast_write_to_fifo32(uint32_t start, uint32_t end, uint32_t dest)
{
        asm volatile(   "copy:                                  \n"
                        "       ldmia %[start]!, {r3}           \n"
                        "       str r3, [%[dest]]               \n"
                        "       cmp %[start], %[end]            \n"
                        "       blt copy                        \n"
                        :
                        :                                                         /* output */
                        [start] "l" (start), [end] "r" (end), [dest] "l" (dest) : /* inputs (%0, %1, %2) */
                        "r3");                                              /* registers that are destroyed */
}

/**
 * \brief Write data (up to 1 page) to Flash
 *
 * \param[in] addr The address of the Flash where the data will be written. It may be anywhere in a
 *        page.
 * \param[in] buf Pointer to the beginning of the buffer that contains the data to be written.
 * \param[in] size The number of bytes to be written.
 *
 * \return The number of bytes written.
 *
 * \warning The boundary of the page where addr belongs to, will not be crossed! The caller should
 *        issue another flash_write_page() call in order to write the remaining data to the next
 *        page.
 */
__RETAINED_CODE static uint32_t flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        uint32_t i = 0;

        uint32_t odd = ((uint32_t) buf) & 3;
        uint32_t size_aligned32;
        uint32_t tmp;
        HW_QSPIC_ID id = flash_get_addr_id(addr, size);
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        addr = flash_get_zero_based_addr(addr);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        flash_write_enable(id);

        /* Reduce max write size, that can reduce interrupt latency time */
        if (size > dg_configFLASH_MAX_WRITE_SIZE) {
                size = dg_configFLASH_MAX_WRITE_SIZE;
        }

        /* Make sure write will not cross page boundary */
        tmp = 256 - (addr & 0xFF);
        if (size > tmp) {
                size = tmp;
        }

        hw_qspi_cs_enable(id);

        if (QSPI_GET_DEVICE_PARAM(idx, qpi_mode)) { // QPI mode
                // Must already be in QUAD mode
                ASSERT_WARNING(QUAD_MODE == 1);

                if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                        hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_qpi_program_opcode));
                        hw_qspi_write32(id, __REV(addr));
                } else {
                       hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx,
                                          page_qpi_program_opcode) | (__REV(addr) & 0xFFFFFF00));
                }
        } else {
                if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                        hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode));
#if QUAD_MODE
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == true) {
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
                        }
#endif
                        hw_qspi_write32(id, __REV(addr));

#if QUAD_MODE
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == false) {
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
                        }
#endif
                }
                else {
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == true) {
                                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode));
#if QUAD_MODE
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
                                hw_qspi_write8(id, (addr >> 16) & 0xFF);
                                hw_qspi_write16(id, (uint16_t)__REV16(addr));
                        } else {
                               hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode) |
                                                                       (__REV(addr) & 0xFFFFFF00));
#if QUAD_MODE
                               flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
                        }
                }
        }

        if (odd) {
                odd = 4 - odd;
                for (i = 0; i < odd && i < size; ++i) {
                        hw_qspi_write8(id, buf[i]);
                }
        }

        size_aligned32 = ((size - i) & ~0x3);

        if (size_aligned32) {
                fast_write_to_fifo32((uint32_t)(buf + i), (uint32_t)(buf + i + size_aligned32),
                        (uint32_t)&(id->QSPIC_WRITEDATA_REG));
                i += size_aligned32;
        }

        for (; i < size; i++) {
                hw_qspi_write8(id, buf[i]);
        }

        hw_qspi_cs_disable(id);

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

#if QUAD_MODE
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), qpi_mode) == false) { // QUAD mode
                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        }
#endif

        return i;
}

#if !ERASE_IN_AUTOMODE
/**
 * \brief Erase a sector of the Flash
 *
 * \param[in] addr The address of the sector to be erased.
 *
 * \note This function blocks until the Flash has processed the command.
 */
__RETAINED_CODE __UNUSED static void flash_erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        addr = flash_get_zero_based_addr(addr);

        flash_write_enable(id);

        if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, erase_opcode));
                hw_qspi_write32(id, __REV(addr));
                hw_qspi_cs_disable(id);
        }
        else {
                hw_qspi_cs_enable(id);
                hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx, erase_opcode) |
                                                        (__REV(addr) & 0xFFFFFF00));
                hw_qspi_cs_disable(id);
        }

        /* Wait for the Flash to process the command */
        while (flash_erase_program_in_progress(id));
}
#endif

/**
 * \brief Check if the Flash can accept commands
 *
 * \param[in] id QSPI controller id
 * \return bool True if the Flash is not busy else false.
 *
 */
__RETAINED_CODE static bool flash_writable(HW_QSPIC_ID id)
{
        bool writable;

        /*
         * From now on QSPI may not be available, turn off interrupts.
         */
        GLOBAL_INT_DISABLE();

        /*
         * Turn on command entry mode.
         */
        qspi_int_activate_command_entry_mode(id);

        /*
         * Check if flash is ready.
         */
        writable = !(flash_is_busy(id));

        /*
         * Restore auto mode.
         */
        qspi_int_deactivate_command_entry_mode(id);

        /*
         * Let other code to be executed including QSPI one.
         */
        GLOBAL_INT_RESTORE();

        return writable;
}

__RETAINED_CODE void qspi_int_activate_command_entry_mode(HW_QSPIC_ID id)
{
        /*
         * Turn off auto mode to allow write.
         */
        flash_set_automode(id, false);

        /*
         * Switch to single mode for command entry.
         */
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

        /*
         * Exit continuous mode, after this the flash will interpret commands again.
         */
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), send_once) != 0) {
                flash_reset_continuous_mode(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), break_seq_size));
        }
}

__RETAINED_CODE void qspi_int_deactivate_command_entry_mode(HW_QSPIC_ID id)
{
        flash_enter_qpi_mode(id);

#if QUAD_MODE
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
        flash_set_automode(id, true);
}

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
__RETAINED_CODE void qspi_int_erase_sector_manual_mode(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);
        addr = flash_get_zero_based_addr(addr);
        /*
         * Turn on command entry mode.
         */
        qspi_int_activate_command_entry_mode(id);

        /*
         * Issue the erase sector command.
         */
        flash_write_enable(id);

        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), address_size) == HW_QSPI_ADDR_SIZE_32) {
                uint8_t cmd[] = { QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_opcode), addr >> 24, addr >> 16, addr >>  8, addr };
                flash_write(id, cmd, 5);
        }
        else {
                uint8_t cmd[] = { QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_opcode), addr >> 16, addr >>  8, addr };
                flash_write(id, cmd, 4);
        }

        /*
         * Flash stays in manual mode.
         */
}

__RETAINED_CODE uint32_t qspi_int_program_page_manual_mode(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        uint32_t written = flash_write_page(addr, buf, size);

        /*
         * Flash stays in manual mode.
         */

        return written;
}

__RETAINED_CODE bool qspi_check_program_erase_in_progress(HW_QSPIC_ID id)
{
        ASSERT_WARNING(qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)]);
        return flash_is_busy(id);
}

__RETAINED_CODE void qspi_resume(HW_QSPIC_ID id)
{
        ASSERT_WARNING(qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)]);

        uint8_t cmd = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_resume_opcode);
        is_suspended_cb_t is_suspended = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), is_suspended);

        do {
                /*
                 * Turn on command entry mode.
                 */
                qspi_int_activate_command_entry_mode(id);

                /*
                 * Check if suspended.
                 */
                if (is_suspended(id) == false) {
                        break;
                }

                /*
                 * Wait for flash to become ready again.
                 */
                do {
                        /*
                         * Resume action.
                         */
                        flash_write(id, &cmd, 1);

                        /*
                         * Check if SUS bit is cleared.
                         */
                }  while (is_suspended(id));
        } while (0);

        /*
         * Flash stays in manual mode.
         */
}
#endif

__RETAINED_CODE void qspi_check_and_suspend_operations(void)
{
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        if (qspi_active_op != NULL) {
                DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SUSPEND);

                HW_QSPIC_ID id = flash_get_addr_id(qspi_active_op->addr, *qspi_active_op->size);
                uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);

                uint8_t cmd = QSPI_GET_DEVICE_PARAM(idx, erase_suspend_opcode);
                HW_QSPI_ACCESS_MODE am = hw_qspi_get_access_mode(id);

                if (am == HW_QSPI_ACCESS_MODE_AUTO) {
                        /*
                         * Turn on command entry mode.
                         */
                        qspi_int_activate_command_entry_mode(id);
                }

                /*
                 * Suspend action.
                 */
                DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);

                /*
                 * Check if an operation is ongoing.
                 */
                while (flash_erase_program_in_progress(id)) {
                        flash_write(id, &cmd, 1);
                }

                hw_clk_delay_usec(FLASH_SUS_DELAY);  // Wait for SUS bit to be updated

                DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);

                if (QSPI_GET_DEVICE_PARAM(idx, is_suspended)(id) == true) {
                        qspi_active_op->suspended = true;
                } else {
                        if (qspi_active_op->op_type == BACKGROUND_OP_ERASE) {
                                qspi_active_op->written = 1;
                        }
                        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);
                }

                /*
                 * Restore auto mode.
                 */
                qspi_int_deactivate_command_entry_mode(id);

                DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SUSPEND);
        }
#endif
}


#if (ERASE_IN_AUTOMODE == 1)
/**
 * \brief Erase sector
 *
 * \details This function will execute a Flash sector erase operation. The operation will either be
 *        carried out immediately (dg_configDISABLE_BACKGROUND_FLASH_OPS is set to 1) or it will be
 *        deferred to be executed in the background when the system becomes idle (when
 *        dg_configDISABLE_BACKGROUND_FLASH_OPS is set to 0, default value). In the latter case, the
 *        caller will block until the registered erase operation is executed.
 *
 * \param[in] addr The address of the sector to be erased.
 */
__RETAINED_CODE static void qspi_erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);

        uint32_t zero_base_addr = flash_get_zero_based_addr(addr);
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)

        OS_TASK handle;
        void *op;
        uint32_t size = FLASH_SECTOR_SIZE;

        handle = OS_GET_CURRENT_TASK();

        if (register_qspi_operation(handle, addr, NULL, &size, &op)) {
                /* Block until erase is completed */
                OS_TASK_SUSPEND(handle);
                OS_FREE(op);
        }
        else {
                /* The PM has not started yet... */

#endif
                hw_qspi_erase_block(id, zero_base_addr);
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        }
#endif
}
#endif /* (ERASE_IN_AUTOMODE == 1)  */

/**
 * \brief Erase a sector of the Flash
 *
 * \details The time and the way that the operation will be carried out depends on the following
 *        settings:
 *        ERASE_IN_AUTOMODE = 0: the command is issued immediately in manual mode
 *        ERASE_IN_AUTOMODE = 1:
 *              dg_configDISABLE_BACKGROUND_FLASH_OPS = 0: the operation is executed manually in the
 *                      background when the system becomes idle
 *              dg_configDISABLE_BACKGROUND_FLASH_OPS = 1: the operation is executed automatically
 *                      by the QSPI controller.
 *
 * \param[in] addr The address of the sector to be erased.
 */
__RETAINED_CODE static void erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);
#if ERASE_IN_AUTOMODE
        /*
         * Erase sector in automode
         */
        qspi_erase_sector(addr);

        /*
         * Wait for erase to finish
         */
        while (hw_qspi_get_erase_status(id) != HW_QSPI_ERS_NO) {
        }
#else
        /*
         * From now on QSPI may not be available, turn off interrupts.
         */
        GLOBAL_INT_DISABLE();

        /*
         * Turn off auto mode to allow write.
         */
        flash_set_automode(id, false);

        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

        /*
         * Exit continuous mode, after this the flash will interpret commands again.
         */
        flash_reset_continuous_mode(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), break_seq_size));

        flash_enter_qpi_mode(id);

        /*
         * Execute erase command.
         */
        flash_erase_sector(addr);

        /*
         * Restore auto mode.
         */
        qspi_int_deactivate_command_entry_mode(id);

        /*
         * Let other code to be executed including QSPI one.
         */
        GLOBAL_INT_RESTORE();
#endif
}

__RETAINED_CODE static uint32_t write_page(HW_QSPIC_ID id, uint32_t addr, const uint8_t *buf, uint32_t size)
{
        uint32_t written;

        /*
        * From now on QSPI may not be available, turn off interrupts.
        */
        GLOBAL_INT_DISABLE();

        /*
        * Turn on command entry mode.
        */
        qspi_int_activate_command_entry_mode(id);

        /*
        * Write data into the page of the Flash.
        */
        written = flash_write_page(addr, buf, size);

        /* Wait for the Flash to process the command */
        while (flash_erase_program_in_progress(id));

        /*
        * Restore auto mode.
        */
        qspi_int_deactivate_command_entry_mode(id);

        /*
        * Let other code to be executed including QSPI one.
        */
        GLOBAL_INT_RESTORE();

        return written;
}

uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        ASSERT_WARNING(size > 0);

        uint32_t written;

        HW_QSPIC_ID id = flash_get_addr_id(addr, size);

        while (!flash_writable(id)) {
        }

#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        OS_TASK handle;
        void *op;

        handle = OS_GET_CURRENT_TASK();

        /* Register a background operation to program this sector */
        if (register_qspi_operation(handle, addr, buf, &size, &op)) {
                /* Block until sector programming is completed */
                OS_TASK_SUSPEND(handle);
                OS_FREE(op);
                written = size;
        } else {
#endif
                written = write_page(id, addr, buf, size);
#if defined(OS_FREERTOS) && (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
        }
#endif

        return written;
}

void qspi_automode_erase_flash_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);

        while (!flash_writable(id)) {
        }

        erase_sector(addr);
}

void qspi_automode_erase_chip(void)
{
        uint8_t idx = 0;
        HW_QSPIC_ID id;

        do {
                if (qspi_is_device_present[idx] == false) {
                        idx++;
                        continue;
                }

                id = QSPI_GET_CONFIG_BASE_REG(idx);
                qspi_int_activate_command_entry_mode(id);

                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, CMD_WRITE_ENABLE);
                hw_qspi_cs_disable(id);

                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, CMD_CHIP_ERASE);
                hw_qspi_cs_disable(id);

                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, CMD_READ_STATUS_REGISTER);
                while (hw_qspi_read8(id) & FLASH_STATUS_BUSY_MASK);
                hw_qspi_cs_disable(id);

                qspi_int_deactivate_command_entry_mode(id);
                idx++;
        } while (idx < QSPI_CONTROLLER_SUPPORT_NUM);
}

uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
        memcpy(buf, qspi_automode_addr(addr), len);
        return len;
}

const void *qspi_automode_addr(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, 1);
        addr = flash_get_zero_based_addr(addr);

        if (id == HW_QSPIC) {
                /* Always access QSPI from QSPI_S bus to make sure that access to
                * all flash regions is possible */
                return (const void *) (MEMORY_QSPIF_S_BASE + addr);
        }
#if dg_configUSE_HW_QSPI2
        return (const void *) (MEMORY_QSPIR_BASE + addr);
#else
        ASSERT_WARNING(0);
        return (const void *) (MEMORY_QSPIF_BASE + addr);
#endif
}

__RETAINED_CODE void qspi_automode_flash_power_up(void)
{
        /* Interrupts must be turned off since the flash goes in manual mode, and
         * code (e.g. for an ISR) cannot be fetched from flash during this time
         */
        GLOBAL_INT_DISABLE();

        uint8_t idx;

#if (dg_configUSE_HW_QSPI == 1)
        if (hw_qspi_is_init_enabled(HW_QSPIC) == false) {
                hw_qspi_clock_enable(HW_QSPIC);
        }
#endif

#if (dg_configUSE_HW_QSPI2 == 1)
        // Reinitialize QSPIC2
        init_hw_qspi(HW_QSPIC2);
        qspi_int_configure(HW_QSPIC2);
#endif

        for (idx = 0; idx < QSPI_CONTROLLER_SUPPORT_NUM; idx++) {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

                if (qspi_is_device_present[idx]) {
                        if (QSPI_GET_DEVICE_PARAM(idx, is_ram)) {
                                qspi_int_deactivate_command_entry_mode(id);
                        } else {
                                if (dg_configFLASH_POWER_DOWN == 1) {
                                        hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                power_down_delay));
                                        /*
                                         * Do not call qspi_int_activate_command_entry_mode().
                                         * This function will try to send break sequence to the
                                         * QSPI Flash which is in power-down mode.
                                         */
                                        flash_set_automode(id, false);
                                        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

                                        hw_qspi_cs_enable(id);
                                        hw_qspi_write8(id, CMD_RELEASE_POWER_DOWN);
                                        hw_qspi_cs_disable(id);
                                        qspi_int_deactivate_command_entry_mode(id);
                                        hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                release_power_down_delay));
                                } else if (hw_qspi_is_init_enabled(HW_QSPIC) == false) {
                                        /*
                                         * Flash is never initialized by the QSPI controller so
                                         * execute the initialization
                                         *
                                         * Note: If flash is initialized by the QSPI controller, it
                                         * will power up (and consume power) every time the system wakes up.
                                         * In case system wakes up by a master which does not use QSPI (e.g SNC),
                                         * power will be consumed for no reason.
                                         *
                                         */
                                        if (dg_configFLASH_POWER_OFF == 1) {
                                                hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                        power_up_delay));

                                                qspi_int_activate_command_entry_mode(id);
                                                QSPI_GET_DEVICE_PARAM(idx, initialize)(id);
                                                qspi_int_deactivate_command_entry_mode(id);
                                        } else {
                                                qspi_int_deactivate_command_entry_mode(id);
                                        }
                                }
                        }
                }
        }

        /*
         * The flash is in auto mode again. Re-enable the interrupts
         */
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void qspi_automode_flash_power_down(void)
{
        for (uint8_t idx = 0; idx < QSPI_CONTROLLER_SUPPORT_NUM; idx++) {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

                if (qspi_is_device_present[idx] &&
                    (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), is_ram) == false)) {
                        /*
                         * Set QSPIC to single mode, disable QSPIC auto mode and
                         * disable flash device's continuous mode (some flash devices
                         * cannot enter standby correctly if continuous read mode is
                         * enabled).
                         */
                        qspi_int_activate_command_entry_mode(id);
                        if (dg_configFLASH_POWER_DOWN == 1) {
                                hw_qspi_cs_enable(id);
                                hw_qspi_write8(id, CMD_ENTER_POWER_DOWN);
                                hw_qspi_cs_disable(id);
                        }
                }
        }

#if (dg_configUSE_HW_QSPI == 1)
        if (hw_qspi_is_init_enabled(HW_QSPIC) == false) {
                // Disable QSPI clock to save power
                hw_qspi_clock_disable(HW_QSPIC);
        }
#endif
}

__RETAINED_CODE void qspi_int_reset_device(HW_QSPIC_ID id)
{
        /*
         * If we initialize rst_cmd during declaration (e.g uint8_t rst_cmd[2] = { 0x66, 0x99 };),
         * compiler adds it to .rodata (flash). We need to run qspi_int_reset_device() from RAM
         * so we declare it without initialization and fill it later.
         */
        uint8_t rst_cmd[2];
        uint8_t power_up_cmd = CMD_RELEASE_POWER_DOWN;

        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        flash_write(id, &power_up_cmd, 1);

        // Reset device to get is out of QPI mode
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);

        // reset continuous mode using both one and two break bytes to cover all cases
        flash_reset_continuous_mode(id, HW_QSPI_BREAK_SEQ_SIZE_2B);
        flash_reset_continuous_mode(id, HW_QSPI_BREAK_SEQ_SIZE_1B);

        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        rst_cmd[0] = 0x66;
        rst_cmd[1] = 0x99;
        flash_write(id, &rst_cmd[0], 1);
        flash_write(id, &rst_cmd[1], 1);

        hw_clk_delay_usec(FLASH_RST_DELAY);
}

__RETAINED_CODE bool qspi_read_flash_jedec_id(HW_QSPIC_ID id, uint8_t *manufacturer_id,
                                                        uint8_t *device_type, uint8_t *density)
{
        uint8_t cmd[] = { CMD_READ_JEDEC_ID, 0, 0, 0 };
        uint8_t buffer[3];
        bool found = false;

        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_MANUAL);

        qspi_int_reset_device(id);

        // Try to read JEDEC standard device ID
        flash_transact(id, cmd, 1, buffer, 3);
        found = buffer[0] != 0xFF && buffer[0] != 0;

        if (found == false) {
                // JEDEC standard device ID reading failed. Try alternative method
                flash_transact(id, cmd, 4, buffer, 3);
                found = buffer[0] != 0xFF && buffer[0] != 0;
        }

        if (found) {
                *manufacturer_id = buffer[0];
                *device_type = buffer[1];
                *density = buffer[2];
        }

        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_AUTO);

        return found;
}

static void flash_automode_prepare_qfis_code(void)
{
        volatile uint32_t *p;
        int i;

        const qspi_ucode_t *ucode = qspi_automode_get_ucode(HW_QSPIC);

        if (!ucode) {
                return;
        }

        p = (volatile uint32_t *)&(QSPIC->QSPIC_UCODE_START);

        for (i = 0; i < ucode->size/sizeof(uint32_t); i++) {
                *p = ucode->code[i];
                p++;
                /* Max 16 ucodes are allowed on wakeup */
                ASSERT_WARNING(i < 16);
        }

        /* zero out trailing words */
        for ( ; i < 16; i++, p++ ) {
                *p = 0;
        }

        /* Must have reached the end of ucodes area here */
        ASSERT_WARNING(p == &(QSPIC->QSPIC_UCODE_START) + 16);
}

__RETAINED_CODE static void init_hw_qspi(HW_QSPIC_ID id)
{
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);

        const qspi_config qspi_cfg = {
                QSPI_GET_DEVICE_PARAM(idx, address_size),
                HW_QSPI_POL_LOW,
                HW_QSPI_SAMPLING_EDGE_POSITIVE
          };

        hw_qspi_cs_enable(id);
        hw_qspi_cs_disable(id);
        hw_qspi_init(id, &qspi_cfg);
        hw_qspi_set_div(id, HW_QSPI_DIV_1);

        hw_qspi_set_read_pipe_clock_delay(id, REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M) ? 7 : 2 );
        hw_qspi_read_pipe_enable(id);
}

__RETAINED_CODE void qspi_int_configure(HW_QSPIC_ID id)
{
        struct qspic_instructions qspi_init_config;
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        HW_QSPI_BUS_MODE mode;

        if (!qspi_is_device_present[idx]) {
                return;
        }

        mode = QSPI_GET_DEVICE_PARAM(idx, qpi_mode) ? HW_QSPI_BUS_MODE_QUAD : HW_QSPI_BUS_MODE_SINGLE;

        ASSERT_WARNING(QSPI_GET_DEVICE_PARAM(idx, fast_read_opcode)); // fast_read_opcode not set

        qspi_init_config.set_read_instruction = false;
        qspi_init_config.read_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, fast_read_opcode);
        qspi_init_config.read_instruction.inst_mode = QSPI_GET_DEVICE_PARAM(idx, send_once);
        qspi_init_config.read_instruction.dummy_count = QSPI_GET_DEVICE_PARAM(idx, get_dummy_bytes)(id);
        qspi_init_config.read_instruction.inst_phase = mode;
        qspi_init_config.read_instruction.addr_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.read_instruction.dummy_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.read_instruction.data_phase = HW_QSPI_BUS_MODE_QUAD;
        /* Setup instruction that will be used to periodically check erase operation status.
         * Check LSB which is 1 when erase is in progress. */
        qspi_init_config.set_read_status_instruction = true;
        qspi_init_config.read_status_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, read_erase_progress_opcode);
        qspi_init_config.read_status_instruction.inst_phase = mode;
        qspi_init_config.read_status_instruction.receive_phase = mode;
        qspi_init_config.read_status_instruction.busy_pos = QSPI_GET_DEVICE_PARAM(idx, erase_in_progress_bit);
        qspi_init_config.read_status_instruction.busy_val = QSPI_GET_DEVICE_PARAM(idx, erase_in_progress_bit_high_level) ? 1 : 0;
        qspi_init_config.read_status_instruction.read_delay = 20;
        qspi_init_config.read_status_instruction.delay_sel = 0;
        /* Setup erase instruction that will be sent by QSPI controller to erase sector in automode. */
        qspi_init_config.set_erase_instruction = true;
        qspi_init_config.erase_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, erase_opcode);
        qspi_init_config.erase_instruction.inst_phase = mode;
        qspi_init_config.erase_instruction.addr_phase = mode;
        qspi_init_config.erase_instruction.hclk_cycles = 15;
        qspi_init_config.erase_instruction.cs_hi_cycles = 5;
        /* QSPI controller must send write enable before erase, this sets it up. */
        qspi_init_config.set_write_enable_instruction = true;
        qspi_init_config.write_enable_instruction.inst = CMD_WRITE_ENABLE;
        qspi_init_config.write_enable_instruction.inst_phase = mode;
        /* Setup instruction pair that will temporarily suspend erase operation to allow read. */
        qspi_init_config.set_suspend_resume_instruction = true;
        qspi_init_config.suspend_resume_instruction.erase_suspend_inst = QSPI_GET_DEVICE_PARAM(idx, erase_suspend_opcode);
        qspi_init_config.suspend_resume_instruction.suspend_inst_phase = mode;
        qspi_init_config.suspend_resume_instruction.erase_resume_inst = QSPI_GET_DEVICE_PARAM(idx, erase_resume_opcode);
        qspi_init_config.suspend_resume_instruction.resume_inst_phase = mode;
        qspi_init_config.suspend_resume_instruction.minimum_delay = 7;

        qspi_init_config.set_write_instruction = true;
        qspi_init_config.write_instruction.inst = CMD_WRITE_QUAD;
        qspi_init_config.write_instruction.inst_phase = mode;
        qspi_init_config.write_instruction.addr_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.write_instruction.data_phase = HW_QSPI_BUS_MODE_QUAD;

        qspi_init_config.set_wrapping_burst_instruction = false;

        if (QSPI_GET_DEVICE_PARAM(idx, is_ram) == false) {
                hw_qspi_set_instructions(id, &qspi_init_config);
                /*
                 * This sequence is necessary if flash is working in continuous read mode, when instruction
                 * is not sent on every read access just address. Sending 0xFFFF will exit this mode.
                 * This sequence is sent only when QSPI is working in automode and decides to send one of
                 * instructions above.
                 * If flash is working in DUAL bus mode sequence should be 0xFFFF and size should be
                 * HW_QSPI_BREAK_SEQ_SIZE_2B.
                 */
                 hw_qspi_burst_break_sequence_enable(id,
                                                 0xFFFF, HW_QSPI_BUS_MODE_SINGLE,
                                                 QSPI_GET_DEVICE_PARAM(idx, break_seq_size), 0);
        }

//         qspi_int_activate_command_entry_mode(id);

        qspi_init_config.set_read_instruction = true;
        hw_qspi_set_instructions(id, &qspi_init_config);

#if (dg_configUSE_HW_QSPI2 == 1)
         if (QSPI_GET_DEVICE_PARAM(idx, is_ram)) {
                 hw_qspi_set_instructions(HW_QSPIC2, &qspi_init_config);
                 hw_qspi_set_sram_mode(HW_QSPIC2, true);
                 hw_qspi_set_cs_mode(HW_QSPIC2, 0);
                 if (QSPI_GET_DEVICE_PARAM(idx, cs_high_t_en)) {
                         hw_qspi_enable_tCEM(HW_QSPIC2);
                         hw_qspi_set_tCEM(HW_QSPIC2, QSPI_GET_DEVICE_PARAM(idx, cs_high_t));
                 }
                 hw_qspi_set_burst_length(HW_QSPIC2, QSPI_GET_DEVICE_PARAM(idx, burst_len));
         }
#endif

         hw_qspi_set_extra_byte(id, QSPI_GET_DEVICE_PARAM(idx, extra_byte), HW_QSPI_BUS_MODE_QUAD, 0);
         hw_qspi_set_address_size(id, QSPI_GET_DEVICE_PARAM(idx, address_size));

         hw_qspi_set_min_cs_high(id, 0);
}

__RETAINED_CODE bool qspi_automode_init(void)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        ASSERT_WARNING(REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN) == 0 ||
                       REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) == 1)
#endif
        uint8_t qspi_control_idx = 0;

        /*
         * If application does not run from FLASH we need to setup qspi first
         * (Normally this is done from booter, but on FPGA there is no booter
         * for RAM projects)
         */
        if ((dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH)) {
                init_hw_qspi(HW_QSPIC);
                hw_qspi_set_access_mode(HW_QSPIC, HW_QSPI_ACCESS_MODE_AUTO);
        } else {
                /*
                 * The booter may have enabled the readpipe. The QSPIC will not work properly if VDD_CORE is 0.9V and
                 * the PCLK_MD is 7. Reset the readpipe to the appropriate value.
                 */
                hw_qspi_set_read_pipe_clock_delay(HW_QSPIC, REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL96M) ? 7 : 2 );
                hw_qspi_read_pipe_enable(HW_QSPIC);
        }

#if (dg_configUSE_HW_QSPI2 == 1)
        // Initialize QSPIC2
        init_hw_qspi(HW_QSPIC2);
        hw_qspi_set_access_mode(HW_QSPIC2, HW_QSPI_ACCESS_MODE_AUTO);
#endif

#if FLASH_AUTODETECT
        bool autodetect[2] = { false, false };
#endif

#if FLASH1_AUTODETECT
        autodetect[0] = true;
#endif
#if QSPIC2_DEV_AUTODETECT
        autodetect[1] = true;
#endif

        do {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(qspi_control_idx);

#if __DBG_QSPI_ENABLED
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_DIV, 3);
#       if (dg_configUSE_HW_QSPI2 == 1)
                REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI2_DIV, 3);
#       endif
#endif

#if (FLASH_AUTODETECT) || (dg_configFLASH_CONFIG_VERIFY)
                uint8_t device_type;
                uint8_t device_density;
                uint8_t manufacturer_id;
#endif
#if (FLASH_AUTODETECT)
                const qspi_flash_config_t *flash_config_init = NULL;

                if (autodetect[qspi_control_idx]) {
                        qspi_is_device_present[qspi_control_idx] = qspi_read_flash_jedec_id(id, &manufacturer_id, &device_type, &device_density);

                        if (qspi_is_device_present[qspi_control_idx] == false) {
                                qspi_control_idx++;
                                continue;
                        }

                        for (uint8_t i=0; i< sizeof(flash_config_table)/sizeof(qspi_flash_config_t*); i++) {
                                if ( (flash_config_table[i]->manufacturer_id == manufacturer_id) &&
                                        (flash_config_table[i]->device_type == device_type) &&
                                        (flash_config_table[i]->device_density == device_density)) {
                                        flash_config_init = flash_config_table[i];
                                        break;
                                }
                        }

                        if (!flash_config_init) {
                                /*
                                 * QSPI controller doesn't have any known device connected.
                                 * Mark QSPI device as not existing
                                 */
                                qspi_is_device_present[qspi_control_idx] = false;
                        }
                }
                else
                {
                        if (id == HW_QSPIC) {
                                qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI == 1);
#if (dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0)
                                flash_config_init = &dg_configFLASH_CONFIG;
#endif
                        }
                        else {
                                qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI2 == 1);
#if (dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0)
                                flash_config_init = &dg_configQSPIC2_DEV_CONFIG;
#endif
                        }
                }
#else
#if dg_configFLASH_CONFIG_VERIFY
                if (qspi_read_flash_jedec_id(id, &manufacturer_id, &device_type, &device_density) &&
                        (manufacturer_id == QSPI_GET_DEVICE_PARAM(qspi_control_idx, manufacturer_id)) &&
                        (device_type == QSPI_GET_DEVICE_PARAM(qspi_control_idx, device_type)) &&
                        (device_density == QSPI_GET_DEVICE_PARAM(qspi_control_idx, device_density)))
                {
                        qspi_is_device_present[qspi_control_idx] = true;
                } else {
                        qspi_is_device_present[qspi_control_idx] = false;
                }
#else
                if (id == HW_QSPIC) {
                        qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI == 1);
                } else {
                        qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI2 == 1);
                }
#endif
#endif
                if (qspi_is_device_present[qspi_control_idx] == false) {
                        qspi_control_idx++;
                        continue;
                }


#if FLASH_AUTODETECT
                ASSERT_WARNING(flash_config_init != NULL);
                /*
                 * Copy the selected flash struct from flash into retram
                 */
                memcpy(&flash_config[qspi_control_idx], flash_config_init, sizeof(qspi_flash_config_t));
#endif
                // Only QSPIC2 supports QSPI RAM devices
                ASSERT_WARNING(qspi_control_idx != 0 || QSPI_GET_DEVICE_PARAM(qspi_control_idx, is_ram) == false);

                // Copy the structure to RAM to use it while in command entry mode

                qspi_int_activate_command_entry_mode(id);

                QSPI_GET_DEVICE_PARAM(qspi_control_idx, initialize)(id);

                qspi_int_configure(id);

                qspi_int_deactivate_command_entry_mode(id);

                qspi_control_idx++;
        } while (qspi_control_idx < QSPI_CONTROLLER_SUPPORT_NUM);

        if ((dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) && hw_qspi_is_init_enabled(HW_QSPIC)) {
                flash_automode_prepare_qfis_code();
        }

#if (dg_configUSE_HW_QSPI == 1)
        // Disable QSPIC1 clock is not used
        if (qspi_is_device_present[0] == false) {
                hw_qspi_clock_disable(HW_QSPIC);
        }
#endif

#if (dg_configUSE_HW_QSPI2 == 1)
        // Disable QSPIC2 clock is not used
        if (qspi_is_device_present[1] == false) {
                hw_qspi_clock_disable(HW_QSPIC2);
        }
#endif
        return true;
}

__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk)
{
        uint8_t idx = 0;

        do {
                if (qspi_is_device_present[idx] == false) {
                        idx++;
                        continue;
                }

                /* Some of the sys_clk_cfg() implementations put the flash in command entry mode, where the flash is
                 * not available for code execution. We must make sure that no interrupt (that may cause a cache miss)
                 * hits during this time.
                 */
                GLOBAL_INT_DISABLE();
                QSPI_GET_DEVICE_PARAM(idx, sys_clk_cfg)(QSPI_GET_CONFIG_BASE_REG(idx), sys_clk);
                GLOBAL_INT_RESTORE();
                hw_qspi_set_read_pipe_clock_delay( QSPI_GET_CONFIG_BASE_REG(idx),
                                                      sys_clk == sysclk_PLL96 ? 7 : 2 );
                hw_qspi_set_min_cs_high(QSPI_GET_CONFIG_BASE_REG(idx),
                                                                sys_clk == sysclk_PLL96 ? 1 : 0);
                idx++;
        } while (idx < QSPI_CONTROLLER_SUPPORT_NUM);
}

const qspi_ucode_t *qspi_automode_get_ucode(HW_QSPIC_ID id)
{
        if (!qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)]) {
                return NULL;
        }

#if FLASH1_AUTODETECT
        return &(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), ucode_wakeup));
#else
        if (QSPI_GET_CONFIG_IDX(id) == 0) {
#if (dg_configUSE_HW_QSPI == 1)
                return &dg_configFLASH_CONFIG.ucode_wakeup;
#endif
        }
        else {
#if (dg_configUSE_HW_QSPI2 == 1)
                return &dg_configQSPIC2_DEV_CONFIG.ucode_wakeup;
#endif
        }
#endif
        ASSERT_WARNING(0);
        return NULL;
}
