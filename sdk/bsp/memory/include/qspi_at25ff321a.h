/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file qspi_at25ff321a.h
 *
 * @brief Driver for flash ADESTO AT25FF321A
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _QSPI_ADESTO_AT25FF321A_H_
#define _QSPI_ADESTO_AT25FF321A_H_

#include "qspi_common.h"

/************************************************* JEDEC ID INFO *************************************************/

/**
 * \brief The Flash type JEDEC ID
 *
 * This is the first byte returned by the 0x9F command
 */
#define ADESTO_ID                                       0x1F

/**
 * \brief The Flash type JEDEC ID
 *
 * This is the second byte returned by the 0x9F command
 */
#define AT25FF321A_TYPE                                 0x47

/**
 * \brief The Flash density JEDEC ID
 *
 * This is the third byte returned by the 0x9F command
 */
#define AT25FF321A_SIZE                                 0x08

/**************************************************** TIMINGS ****************************************************/

/* Flash power up/down timings in usec - cannot have less than 3us delay */
#define AT25FF321A_POWER_DOWN_DELAY_US                  3
#define AT25FF321A_RELEASE_POWER_DOWN_DELAY_US          35
#define AT25FF321A_POWER_UP_DELAY_US                    60000

/************************************************ Opcodes SECTION ************************************************/

/* Status Register Opcodes */
#define AT25FF321A_VOLATILE_WRITE_ENABLE_OPCODE         0x50
#define AT25FF321A_READ_SR2_REGISTER_OPCODE             0x35
#define AT25FF321A_WRITE_SR2_REGISTER_OPCODE            0x31
#define AT25FF321A_INDIRECT_READ_STATUS_REG_X_OPCODE    0x65
#define AT25FF321A_INDIRECT_WRITE_STATUS_REG_X_OPCODE   0x71

/* Suspend/Resume Opcodes */
#define AT25FF321A_ERASE_PROGRAM_SUSPEND_OPCODE         0x75
#define AT25FF321A_ERASE_PROGRAM_RESUME_OPCODE          0x7A

/* Quad Read Opcode */
#define AT25FF321A_FAST_READ_QUAD_OPCODE                0xEB

/* Quad Page Program */
#define AT25FF321A_QUAD_PAGE_PROGRAM_OPCODE             0x32

/********************************************** DRIVER GENERIC INFO **********************************************/

/* Quad Enable Bit Position - Status Register */
#define AT25FF321A_SR2_QE_BIT_POS                       (1)
#define AT25FF321A_SR2_QE_MASK                          (1 << AT25FF321A_SR2_QE_BIT_POS)

/* Erase/Program Suspend Bit Positions - Status Register */
#define AT25FF321A_SR2_EPSUS_BIT_POS                    (7)
#define AT25FF321A_SR2_EPSUS_MASK                       (1 << AT25FF321A_SR2_EPSUS_BIT_POS)

/* XiP Bit */
#define AT25FF321A_SR4_XIP_BIT_POS                      (3)
#define AT25FF321A_SR4_XIP_MASK                         (1 << AT25FF321A_SR4_XIP_BIT_POS)

/* PDM Bit */
#define AT25FF321A_SR4_PDM_BIT_POS                      (7)
#define AT25FF321A_SR4_PDM_MASK                         (1 << AT25FF321A_SR4_PDM_BIT_POS)

/* Dummy Cycles B0 position on Status Register 5*/
#define AT25FF321A_SR5_DC0_BIT_POS                      (4)

/* Extra/Mode Byte used for entering continuous read mode */
#define AT25FF321A_EXTRA_BYTE_M7_M0                     0xA5

/************************************************* TYPEDEF TYPES *************************************************/
/**
 * \brief Flash Status Registers
 *
 */
typedef enum {
        AT25FF321A_STATUS_REG_1 = 0x01,
        AT25FF321A_STATUS_REG_2 = 0x02,
        AT25FF321A_STATUS_REG_3 = 0x03,
        AT25FF321A_STATUS_REG_4 = 0x04,
        AT25FF321A_STATUS_REG_5 = 0x05,
} AT25FF321A_STATUS_REG;

/**
 * \brief Dummy Cycles Bit value set of Status Register 5 for EBh at each Operating Frequency according to the Datasheet.
 *
 */
typedef enum {
        AT25FF321A_OP_FREQ_50_EB = 1,
        AT25FF321A_OP_FREQ_104_EB = 2,
} AT25FF321A_OP_FREQ_EB;

/************************************************** GLOBAL VARS **************************************************/
/*
 * Variable that holds the number of Dummy Bytes needed (starting value should reflect 32 MHz operating Frequency).
 * Must be __RETAINED_RW to be initialized to anything other than zero.
 */
__RETAINED_RW uint8_t at25ff321a_dummy_bytes = AT25FF321A_OP_FREQ_50_EB;

/************************************************* uCode SECTION *************************************************/

/*
 * This section is OBSOLETE in DA1469x Family products however it is maintained here for reference reasons as
 * this part is mandatory for DA14683 products.
 */

#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */

const uint32_t at25ff321a_ucode_wakeup[] = {
        0x11000001 | (((uint16_t)(AT25FF321A_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        0xFFFF0000,
};
#elif (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
const uint32_t at25ff321a_ucode_wakeup[] = {
        0xAB000009 | (((uint16_t)(AT25FF321A_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
};
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
const uint32_t at25ff321a_ucode_wakeup[] = {
        0xFF000025,
        0x00FFFFFF,
};

const uint32_t at25ff321a_ucode_wakeup_32bit_addressing[] = {
        0xFF000045,
        0xFFFFFFFF,
        0x00FFFFFF,
};

#endif

/********************************************** FUNCTION PROTOTYPES **********************************************/
/**
 * \brief Set the Quad Enable Bit of the Status Register if it is not already set.
 *
 * \param[in] id QSPI controller id
 */
__RETAINED_CODE static void flash_at25ff321a_enable_quad_mode(HW_QSPIC_ID id);

/**
 * \brief Set the XiP Bit of the Status Register 4 if it is not already set.
 *
 * \param[in] id QSPI controller id
 */
__RETAINED_CODE static void flash_at25ff321a_enable_XiP_and_DPD_mode(HW_QSPIC_ID id);

/**
 * \brief This function returns the dummy bytes required when utilizing Fast Read Quad.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function will return the number of dummy bytes required while Fast Read Quad is activated. In some QSPI Flash devices,
 *       it is possible that the number of dummy bytes required can vary depending on the operating speed.
 */
__RETAINED_CODE static uint8_t flash_at25ff321a_get_dummy_bytes(HW_QSPIC_ID id);

/**
 * \brief Initialize AT25FF321A QSPI Flash
 *
 * \param[in] id QSPI controller id
 *
 * \note This function will perform all the required actions in order for AT25FF321A QSPI Flash to
 *       be properly initialized.
 */
__RETAINED_CODE static void flash_at25ff321a_initialize(HW_QSPIC_ID id);

/**
 * \brief This function returns true if an erase or program operation is already suspended.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function reads the status register and checks the erase/program suspend bits. If either of them is set then it returns true
 *       since a program or erase operation is currently suspended.
 */
__RETAINED_CODE static bool flash_at25ff321a_is_suspended(HW_QSPIC_ID id);

/**
 * \brief Read the Status Register 2 of the Flash
 *
 * \param[in] id QSPI controller id
 *
 * \return uint8_t The value of the Status Register 2 of the Flash.
 */
__RETAINED_CODE static uint8_t flash_at25ff321a_read_status_register_2(HW_QSPIC_ID id);

/**
 * \brief Indirectly read the targetted Status Register of the Flash.
 *
 * \param[in] id QSPI controller id
 * \param[in] reg uint8_t reg value 1 to 5
 *
 * \return uint8_t The value of the targetted Status Register of the Flash.
 */
__RETAINED_CODE static uint8_t flash_at25ff321a_read_status_register_indirect(HW_QSPIC_ID id, uint8_t reg);

/**
 * \brief Sets the number of Dummy Cycles for AT25FF321A.
 *
 * \param[in] id QSPI controller id
 * \param[in] set_non_volatile bool set_non_volatile
 * \param[in] d_cycles AT25FF321A_OP_FREQ_EB d_cycles

 *
 * \note This function sets up bits[6:4] of Status Register 5 that correspond to the number of Dummy Cycles needed.
 *       The number of Dummy Cycles include the cycles needed to send the Extra Byte[M7-M0] and depend on the Operating Frequency.
 *       If set_non_volatile is false the volatile version of the register is set, otherwise the non volatile
 *       version of the register is set instead.
 */
__RETAINED_CODE static void flash_at25ff321a_set_dummy_cycles(HW_QSPIC_ID id, bool set_non_volatile, AT25FF321A_OP_FREQ_EB d_cycles);

/**
 * \brief Sets the initial number of Dummy Cycles for AT25FF321A.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function reads Status Register 5 and checks if the correct number of expected dummy cycles are set on the QSPI Flash for 32MHz Operating Frequency
 *       and if not then sets them.
 */
__RETAINED_CODE static void flash_at25ff321a_set_initial_boot_state(HW_QSPIC_ID id);

/**
 * \brief Hook function called when the clock is changed
 *
 * \param[in] id QSPI controller id
 *
 * \param[in] sys_clk system clock source to be switched to
 *
 * \note This function will be called prior to switching from one clock source to another. This function allows doing all
 *       the required tasks before the clock source is switched. Typical use could be setting up the proper QSPI divider
 *       to ensure that the device will continue to operate even after the clock source will be switched. E.g. Flash that supports
 *       up to 80 MHz operating speed will need to have a divider of 2 if PLL96 is picked as the source.
 */
__RETAINED_CODE static void flash_at25ff321a_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);

/**
 * \brief Set WEL (Write Enable Latch) bit of the Volatile version of the Status Register of the Flash
 * \details The WEL bit must be set prior to writing to the volatile versions of the Status Registers.
 *
 * \param[in] id QSPI controller id
 *
 */
__RETAINED_CODE static void flash_at25ff321a_volatile_write_enable(HW_QSPIC_ID id);

/**
 * \brief Write the Status Register 2 of the Flash
 *
 * \param[in] id QSPI controller id
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE static void flash_at25ff321a_write_status_register_2(HW_QSPIC_ID id, uint8_t value);

/**
 * \brief Indirectly write the targetted Status Register of the Flash
 *
 * \param[in] id QSPI controller id
 * \param[in] reg uint8_t reg value 1 to 5
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE static void flash_at25ff321a_write_status_register_indirect(HW_QSPIC_ID id, uint8_t reg, uint8_t value);


/************************************************* CONFIG OBJECT *************************************************/

/**
 * \brief This struct configures the system for the specific flash
 *
 * \note This struct MUST be const for this to work. Therefore, assignments must
 *       not change (must be read-only)
 */
static const qspi_flash_config_t flash_at25ff321a_config = {
        /* JEDEC Bytes 9Fh */
        .manufacturer_id                  = ADESTO_ID,
        .device_type                      = AT25FF321A_TYPE,
        .device_density                   = AT25FF321A_SIZE,

        /* Flash Info */
        .memory_size                      = MEMORY_SIZE_32Mb,
        .address_size                     = HW_QSPI_ADDR_SIZE_24,
        .is_ram                           = false,
        .qpi_mode                         = false,

        /* Callbacks */
        .is_suspended                     = flash_at25ff321a_is_suspended,
        .initialize                       = flash_at25ff321a_initialize,
        .sys_clk_cfg                      = flash_at25ff321a_sys_clock_cfg,
        .get_dummy_bytes                  = flash_at25ff321a_get_dummy_bytes,

        /* Read */
        .fast_read_opcode                 = AT25FF321A_FAST_READ_QUAD_OPCODE,
        .send_once                        = 1,
        .extra_byte                       = AT25FF321A_EXTRA_BYTE_M7_M0,
        .break_seq_size                   = HW_QSPI_BREAK_SEQ_SIZE_1B,

        /* Page Program */
        .page_program_opcode              = AT25FF321A_QUAD_PAGE_PROGRAM_OPCODE,
        .quad_page_program_address        = false,

        /* Sector Erase */
        .erase_opcode                     = CMD_SECTOR_ERASE,
        .read_erase_progress_opcode       = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit            = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,

        /* Program/Erase Suspend/Resume */
        .erase_suspend_opcode             = AT25FF321A_ERASE_PROGRAM_SUSPEND_OPCODE,
        .erase_resume_opcode              = AT25FF321A_ERASE_PROGRAM_RESUME_OPCODE,

        /* Timings */
        .power_down_delay                 = AT25FF321A_POWER_DOWN_DELAY_US,
        .release_power_down_delay         = AT25FF321A_RELEASE_POWER_DOWN_DELAY_US,
        .power_up_delay                   = AT25FF321A_POWER_UP_DELAY_US,
};

/********************************************* FUNCTION  DEFINITIONS *********************************************/

__RETAINED_CODE static void flash_at25ff321a_enable_quad_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_at25ff321a_read_status_register_2(id);
        if (!(status & AT25FF321A_SR2_QE_MASK)) {
                flash_write_enable(id);
                flash_at25ff321a_write_status_register_2(id, status | AT25FF321A_SR2_QE_MASK);
        }
}

__RETAINED_CODE static void flash_at25ff321a_enable_XiP_and_DPD_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ volatile uint8_t status;
        uint8_t set_SR4;

        status = flash_at25ff321a_read_status_register_indirect(id, AT25FF321A_STATUS_REG_4);

        set_SR4 = status & (AT25FF321A_SR4_XIP_MASK | AT25FF321A_SR4_PDM_MASK);

        if (!set_SR4) {
                set_SR4 = status | (AT25FF321A_SR4_XIP_MASK | AT25FF321A_SR4_PDM_MASK);
                flash_write_enable(id);
                flash_at25ff321a_write_status_register_indirect(id, AT25FF321A_STATUS_REG_4, set_SR4);
        }

}

__RETAINED_CODE static uint8_t flash_at25ff321a_get_dummy_bytes(HW_QSPIC_ID id)
{
        return at25ff321a_dummy_bytes;
}

__RETAINED_CODE static void flash_at25ff321a_initialize(HW_QSPIC_ID id)
{
        /* Set QE Bit if it is not set */
        flash_at25ff321a_enable_quad_mode(id);

        /* Set XiP Bit if it is not set */
        flash_at25ff321a_enable_XiP_and_DPD_mode(id);

        /* Set the  dummy Cycles that the device will be expecting upon initialization */
        flash_at25ff321a_set_initial_boot_state(id);
}

__RETAINED_CODE static bool flash_at25ff321a_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_at25ff321a_read_status_register_2(id);
        return ((status & AT25FF321A_SR2_EPSUS_MASK) != 0);
}

__RETAINED_CODE static uint8_t flash_at25ff321a_read_status_register_2(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { AT25FF321A_READ_SR2_REGISTER_OPCODE };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

__RETAINED_CODE static uint8_t flash_at25ff321a_read_status_register_indirect(HW_QSPIC_ID id, AT25FF321A_STATUS_REG reg)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { AT25FF321A_INDIRECT_READ_STATUS_REG_X_OPCODE, reg, 0x00 };

        flash_transact(id, cmd, 3, &status, 1);

        return status;
}

__RETAINED_CODE static void flash_at25ff321a_set_dummy_cycles(HW_QSPIC_ID id, bool set_non_volatile, AT25FF321A_OP_FREQ_EB d_cycles)
{
        uint8_t status;
        uint8_t dc_mask;
        uint8_t written_value;
        uint8_t dc_bit_clear_mask = 0x8F;

        if (d_cycles > 4) {
                d_cycles = 4;
        }

        status = flash_at25ff321a_read_status_register_indirect(id, AT25FF321A_STATUS_REG_5);

        if (set_non_volatile) {
                flash_write_enable(id);
        } else {
                flash_at25ff321a_volatile_write_enable(id);
        }

        dc_mask = (d_cycles << AT25FF321A_SR5_DC0_BIT_POS);
        written_value = ((status & dc_bit_clear_mask) | dc_mask);

        flash_at25ff321a_write_status_register_indirect(id, AT25FF321A_STATUS_REG_5, written_value);

        /* The encoding of the dummy cycles for this QSPI Flash corresponds to the actual number of dummy bytes that need to be sent */
        hw_qspi_set_dummy_bytes_count(id, d_cycles);
        at25ff321a_dummy_bytes = d_cycles;

}

__RETAINED_CODE static void flash_at25ff321a_set_initial_boot_state(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_at25ff321a_read_status_register_indirect(id, AT25FF321A_STATUS_REG_5);

        if (AT25FF321A_OP_FREQ_50_EB != ((status >> AT25FF321A_SR5_DC0_BIT_POS) & 0x07)) {
                flash_at25ff321a_set_dummy_cycles(id, true, AT25FF321A_OP_FREQ_50_EB);
        }
}

__RETAINED_CODE static void flash_at25ff321a_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        qspi_int_activate_command_entry_mode(id);
        if (sysclk_PLL96 == sys_clk) {
                flash_at25ff321a_set_dummy_cycles(id, false, AT25FF321A_OP_FREQ_104_EB);
        } else {
                flash_at25ff321a_set_dummy_cycles(id, false, AT25FF321A_OP_FREQ_50_EB);
        }
        qspi_int_deactivate_command_entry_mode(id);
}

__RETAINED_CODE static void flash_at25ff321a_volatile_write_enable(HW_QSPIC_ID id)
{
        uint8_t cmd[] = { AT25FF321A_VOLATILE_WRITE_ENABLE_OPCODE };


        flash_write(id, cmd, 1);
}

__RETAINED_CODE static void flash_at25ff321a_write_status_register_2(HW_QSPIC_ID id, uint8_t value)
{
        uint8_t cmd[2] = { AT25FF321A_WRITE_SR2_REGISTER_OPCODE, value };

        flash_write(id, cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

__RETAINED_CODE static void flash_at25ff321a_write_status_register_indirect(HW_QSPIC_ID id, AT25FF321A_STATUS_REG reg, uint8_t value )
{
        uint8_t cmd[3] = { AT25FF321A_INDIRECT_WRITE_STATUS_REG_X_OPCODE, reg, value };

        flash_write(id, cmd, 3);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

#endif /* _QSPI_ADESTO_AT25FF321A_H_ */
/**
 * \}
 * \}
 */
