/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_OTPC
 * \{
 * \brief OTP Memory Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_otpc_da1469x.h
 *
 * @brief Definition of API for the OTP Controller driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef HW_OTPC_DA1469x_H_
#define HW_OTPC_DA1469x_H_


#if dg_configUSE_HW_OTPC

#include <sdk_defs.h>

/**
 * \brief Get the mask of a field of an OTPC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_OTPC_REG_FIELD_MASK(reg, field) \
        (OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Msk)

/**
 * \brief Get the bit position of a field of an OTPC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_OTPC_REG_FIELD_POS(reg, field) \
        (OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Pos)

/**
 * \brief Prepare (i.e. shift and mask) a value to be used for an OTPC register field.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 * \param [in] val is the value to prepare
 *
 */
#define HW_OTPC_FIELD_VAL(reg, field, val) \
        (((val) << HW_OTPC_REG_FIELD_POS(reg, field)) & HW_OTPC_REG_FIELD_MASK(reg, field))

/**
 * \brief Get the value of a field of an OTPC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_OTPC_REG_GETF(reg, field) \
        ((OTPC->OTPC_##reg##_REG & (OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Msk)) >> (OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Pos))

/**
 * \brief Set the value of a field of an OTPC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_OTPC_REG_SETF(reg, field, new_val) \
        OTPC->OTPC_##reg##_REG = ((OTPC->OTPC_##reg##_REG & ~(OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Msk)) | \
                ((OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Msk) & ((new_val) << (OTPC_OTPC_##reg##_REG_##OTPC_##reg##_##field##_Pos))))


/**
 * \brief OTP Controller mode
 */
typedef enum {
        HW_OTPC_MODE_PDOWN      = 0,    /**< OTP cell and LDO are inactive*/
        HW_OTPC_MODE_DSTBY      = 1,    /**< OTP cell is powered on LDO is inactive*/
        HW_OTPC_MODE_STBY       = 2,    /**< OTP cell and LDO are powered on, chip select is deactivated*/
        HW_OTPC_MODE_READ       = 3,    /**< OTP cell can be read*/
        HW_OTPC_MODE_PROG       = 4,    /**< OTP cell can be programmed*/
        HW_OTPC_MODE_PVFY       = 5,    /**< OTP cell can be read in PVFY margin read mode*/
        HW_OTPC_MODE_RINI       = 6     /**< OTP cell can be read in RINI margin read mode*/
} HW_OTPC_MODE;


#define HW_OTP_CELL_NUM                 (0x400) /* Max number of OTP cells, each cell size is 4 bytes*/
#define HW_OTP_MAX_PAYLOAD_ENTRIES      (8)     /* Max number of OTP Payload entries */
#define HW_OTP_USER_DATA_KEY_SIZE       (0x20)  /* Size of User Data Encryption Key */

/**
 * \brief Word inside cell to program/read
 *
 * Cell contents in memory starts with low word (i.e. to program/read both words in cell at once,
 * HW_OTPC_WORD_LOW should be used for addressing).
 *
 */
typedef enum {
        HW_OTPC_WORD_LOW = 0,
        HW_OTPC_WORD_HIGH = 1
} HW_OTPC_WORD;


/**
 * \brief System clock frequency in MHz
 *
 */
typedef enum {
        HW_OTPC_CLK_FREQ_16MHz      = 0,
        HW_OTPC_CLK_FREQ_32MHz      = 1,
        HW_OTPC_CLK_FREQ_48MHz      = 2,
        HW_OTPC_CLK_FREQ_96MHz      = 3
} HW_OTPC_CLK_FREQ;

/*
 * Reset values of OTPC registers
 */
#define OTPC_TIM1_REG_RESET                  (0x0999101f)
#define OTPC_TIM2_REG_RESET                  (0xa4040409)


/**
 * \brief check OTPC_STAT_MRDY bit .
 *
 */
__STATIC_INLINE void hw_otpc_wait_mode_change(void)
{
        while (!REG_GETF(OTPC, OTPC_STAT_REG, OTPC_STAT_MRDY))
                ;
}

/**
 * \brief check OTPC_STAT_PRDY bit.
 *
 */
__STATIC_INLINE void hw_otpc_wait_while_busy_programming(void)
{
        while (!REG_GETF(OTPC, OTPC_STAT_REG, OTPC_STAT_PRDY))
                ;
}

/**
 * \brief check OTPC_STAT_PBUF_EMPTY bit.
 *
 */
__STATIC_INLINE void hw_otpc_wait_while_programming_buffer_is_full(void)
{
        while (!REG_GETF(OTPC, OTPC_STAT_REG, OTPC_STAT_PBUF_EMPTY))
                ;
}

/**
 * \brief Initialize the OTP Controller.
 *
 */
__STATIC_INLINE void hw_otpc_init(void)
{
        GLOBAL_INT_DISABLE();

        /*
         * Enable OTPC clock
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE, 0x1);

        REG_SETF(OTPC, OTPC_MODE_REG, OTPC_MODE_MODE, HW_OTPC_MODE_DSTBY);
        hw_otpc_wait_mode_change();

        OTPC->OTPC_TIM1_REG = OTPC_TIM1_REG_RESET;
        OTPC->OTPC_TIM2_REG = OTPC_TIM2_REG_RESET;

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Close the OTP Controller.
 *
 */
__STATIC_INLINE void hw_otpc_close(void)
{
        /*
         * Disable OTPC clock
         */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE, 0x0);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the OTP Controller is active.
 *
 * \return 1 if it is active, else 0.
 *
 */
__STATIC_FORCEINLINE bool hw_otpc_is_active(void)
{
        /*
         * Check if the OTPC clock is enabled
         */
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE);
}

/**
 * \brief Moves the OTPC in new mode
 *
 * \param [in] mode the new mode
 *
 */
__STATIC_INLINE void hw_otpc_enter_mode (HW_OTPC_MODE mode)
{
        volatile HW_OTPC_MODE current_mode;

        /*change mode only if new mode is different than the old one*/
        current_mode = REG_GETF(OTPC, OTPC_MODE_REG, OTPC_MODE_MODE);
        if (mode != current_mode)
        {
                REG_SETF(OTPC, OTPC_MODE_REG, OTPC_MODE_MODE, mode);
                hw_otpc_wait_mode_change();

        }
}

/**
 * \brief Program OTP with a word
 *
 * \param [in] wdata          the data to be programmed
 *
 * \param [in] cell_offset   The offset of cell to be writtent in 32 bit words
 *
 */
__STATIC_INLINE void hw_otpc_word_prog(uint32_t wdata, uint32_t cell_offset )
{
        /*Check if we are in program mode, if not enter */
        hw_otpc_enter_mode(HW_OTPC_MODE_PROG);
        OTPC->OTPC_PWORD_REG = wdata;
        OTPC->OTPC_PADDR_REG = cell_offset;
        hw_otpc_wait_while_busy_programming();
}

/**
 * \brief Set the access speed of the OTP Controller based on the system clock.
 *
 * \details Switch from PLL to XTAL : call function after clock switch with high_clk_speed == false
 *          Switch from XTAL to PLL : call function before clock switch with high_clk_speed == true
 *
 * \param [in] clk_speed The frequency of the system clock: 16, 32, 48 and 96.
 *
 * \warning The OTP clock must have been enabled (OTP_ENABLE == 1).
 *          (Note: the hw_otpc_set_speed() must be called only once when the PLL is used or
 *          during each clock switch when both PLL and XTAL16 are used, since the register bits it
 *          modifies are retained.)
 *
 */
void hw_otpc_set_speed(HW_OTPC_CLK_FREQ clk_speed);

/**
 * \brief Moves the OTPC in power down state
 */
__RETAINED_CODE void hw_otpc_disable(void);

/**
 * \brief read  a word from OTP
 *
 * \param [in] cell_offset   The offset of cell to be read in 32 bit words
 *
 * \return otp cell value
 *
 */
uint32_t hw_otpc_word_read( uint32_t cell_offset );

/**
 * \brief Program specific bits in OTP
 *
 * \param [in] wdata    the data to be programmed
 *
 * \param [in] mask     the mask of the bit field
 *
 * \param [in] pos      the position (offset) of the bitfield
 *
 * \param [in] cell_offset   The offset of cell to be written in 32 bit words
 *
 */
void hw_otpc_bits_prog(uint32_t wdata, const uint32_t mask, const uint32_t pos, uint32_t cell_offset );

/**
 * \brief Program specific bits in OTP with verification
 *
 * \param [in] wdata    the data to be programmed
 *
 * \param [in] mask     the mask of the bit field
 *
 * \param [in] pos      the position (offset) of the bitfield
 *
 * \param [in] cell_offset   The offset of cell to be written in 32 bit words
 *
 * \return cell true if success or false in fail
 *
 */
bool hw_otpc_bits_prog_and_verify(uint32_t wdata, const uint32_t mask, const uint32_t pos, uint32_t cell_offset );

/**
 * \brief Program OTP with a block of data.
 *
 * \param [in] p_data        pointer to the data to be programmed
 *
 * \param [in] cell_offset   The offset of cell to be written in 32 bit words
 *
 * \param [in] num_of_words  Number of words to be written
 *
 */
void hw_otpc_prog(uint32_t *p_data, uint32_t cell_offset, uint32_t num_of_words);


/**
 * \brief Program OTP with a block of data with verify.
 *
 * \param [in] p_data        pointer to the data to be programmed
 *
 * \param [in] cell_offset   The offset of cell to be written in 32 bit words
 *
 * \param [in] num_of_words  Number of words to be written
 *
 * \return cell true if success or false in fail
 *
 * \warning The comparison is in a word by word basis while writing. On the first fail the function exits.
 *
 */
bool hw_otpc_prog_and_verify(uint32_t *p_data, uint32_t cell_offset, uint32_t num_of_words);

/**
 * \brief read  an OTP block
 *
 * \param [in] p_data        pointer to the data to be read
 *
 * \param [in] cell_offset   The offset of cell to be read in 32 bit words
 *
 * \param [in] num_of_words  Number of words to be read
 *
 */
void hw_otpc_read( uint32_t *p_data, uint32_t cell_offset, uint32_t num_of_words);

/**
 * \brief Get cell memory address
 *
 * Returns mapped memory address for given cell
 *
 * \param [in] cell_offset cell offset
 *
 * \return cell memory address
 *
 */
__STATIC_INLINE void *hw_otpc_cell_to_mem(uint32_t cell_offset)
{
        return (void *) (MEMORY_OTP_BASE + (cell_offset << 2)); // cell size is 4 bytes
}

/**
 * \brief Translate OTP address to cell offset
 *
 * \param [in] address       Address of OTP memory cell
 *
 * \return cell offset
 *
 * \warning If given address is not at beginning of cell then memory cell containing given address
 *          will be returned
 *
 */
uint32_t hw_otpc_address_to_cell_offset(uint32_t address);


#endif /* dg_configUSE_HW_OTPC */


#endif /* HW_OTPC_DA1469x_H_ */

/**
\}
\}
 */
