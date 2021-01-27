/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_QSPI QSPI Flash Memory Controller
 * \{
 * \brief QSPI Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_qspi.h
 *
 * @brief Definition of API for the QSPI Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_QSPI_H_
#define HW_QSPI_H_


#if (dg_configUSE_HW_QSPI) || (dg_configUSE_HW_QSPI2)

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Bus mode
 *
 */
typedef enum {
        HW_QSPI_BUS_MODE_SINGLE,   /**< Bus mode in single mode */
        HW_QSPI_BUS_MODE_DUAL,     /**< Bus mode in dual mode */
        HW_QSPI_BUS_MODE_QUAD,     /**< Bus mode in quad mode */
        HW_QSPI_BUS_MODE_QPI       /**< Bus mode in qpi mode */
} HW_QSPI_BUS_MODE;

/**
 * \brief Flash memory address size
 *
 */
typedef enum {
        HW_QSPI_ADDR_SIZE_24,      /**< QSPI flash memory uses 24 bits address */
        HW_QSPI_ADDR_SIZE_32       /**< QSPI flash memory uses 32 bits address */
} HW_QSPI_ADDR_SIZE;

#define HW_QSPI_MAX_ADDR_SIZE 0x2000000

/**
 * \brief Idle clock state
 *
 */
typedef enum {
        HW_QSPI_POL_LOW = 0,    /**< Use Mode 0 for the QSPI_CLK. The QSPI_SCK will be low when QSPI_CS is high (idle) */
        HW_QSPI_POL_HIGH = 1    /**< Use Mode 3 for the QSPI_CLK. The QSPI_SCK will be high when QSPI_CS is high (idle) */
} HW_QSPI_POL;

/**
 * \brief Memory busy status
 *
 */
typedef enum {
        HW_QSPI_BUSY_LOW = 0,   /**< The QSPI memory is busy when the Busy bit is equal to 0 */
        HW_QSPI_BUSY_HIGH = 1   /**< The QSPI memory is busy when the Busy bit is equal to 1 */
} HW_QSPI_BUSY;

/**
 * \brief The progress of sector/block erasing
 *
 */
typedef enum {
        HW_QSPI_ERS_NO = 0,             /**< no erase                           */
        HW_QSPI_ERS_PENDING = 1,        /**< pending erase request              */
        HW_QSPI_ERS_RUNNING = 2,        /**< erase procedure is running         */
        HW_QSPI_ERS_SUSPENDED = 3,      /**< suspended erase procedure          */
        HW_QSPI_ERS_FINISHING = 4       /**< finishing the erase procedure      */
} HW_QSPI_ERS;

/**
 * \brief Type of QSPI_CLK edge for sampling received data
 *
 */
typedef enum {
        HW_QSPI_SAMPLING_EDGE_POSITIVE = 0,        /**< Sample the received data with the positive edge of the QSPI_SCK */
        HW_QSPI_SAMPLING_EDGE_NEGATIVE = 1         /**< Sample the received data with the negative edge of the QSPI_SCK */
} HW_QSPI_SAMPLING_EDGE;

/**
 * \brief QSPI memory access mode
 *
 */
typedef enum {
        HW_QSPI_ACCESS_MODE_MANUAL = 0, /**< Direct register access using the QSPIC register file */
        HW_QSPI_ACCESS_MODE_AUTO = 1    /**< Up to 32 MB memory-mapped access with 3- and 4-byte addressing modes */
} HW_QSPI_ACCESS_MODE;

/**
 * \brief QSPI HREADY mode when accessing the WRITEDATA, READDATA and DUMMYDATA registers
 *
 */
typedef enum {
        HW_QSPI_HREADY_MODE_STALLING = 0,       /**< Add wait states via the HREADY signal  */
        HW_QSPI_HREADY_MODE_FIXED = 1           /**< Don't add wait states via the HREADY signal */
} HW_QSPI_HREADY_MODE;

/**
 * \brief QSPI Instruction mode
 *
 */
typedef enum {
        HW_QSPI_INST_MODE_SEND_ANYTIME = 0,     /**< Transmit instruction at any burst access */
        HW_QSPI_INST_MODE_SEND_ONCE = 1         /**< Transmit instruction only in the first access after the selection of Auto Mode */
} HW_QSPI_INST_MODE;

/**
 * \brief Selected data size of a wrapping burst
 *
 */
typedef enum {
        HW_QSPI_WRAP_SIZE_8BITS = 0,       /**< Byte access (8-bits) */
        HW_QSPI_WRAP_SIZE_16BITS = 1,      /**< Half word access (8-bits) */
        HW_QSPI_WRAP_SIZE_32BITS = 2,      /**< Word access (8-bits) */
} HW_QSPI_WRAP_SIZE;

/**
 * \brief Selected data length of a wrapping burst
 *
 */
typedef enum {
        HW_QSPI_WRAP_LEN_4BEAT = 0,        /**< 4 beat wrapping burst */
        HW_QSPI_WRAP_LEN_8BEAT = 1,        /**< 8 beat wrapping burst */
        HW_QSPI_WRAP_LEN_16BEAT = 2,       /**< 16 beat wrapping burst */
} HW_QSPI_WRAP_LEN;

/**
 * \brief Size of Burst Break Sequence
 *
 */
typedef enum {
        HW_QSPI_BREAK_SEQ_SIZE_1B = 0,     /**< One byte */
        HW_QSPI_BREAK_SEQ_SIZE_2B = 1      /**< Two bytes */
} HW_QSPI_BREAK_SEQ_SIZE;

/**
 * \brief QSPI pad id
 *
 */
typedef enum {
        HW_QSPI_IO2,    /**< In SPI or Dual SPI mode controls the /WP signal.  */
        HW_QSPI_IO3     /**< In SPI or Dual SPI mode controls the /HOLD signal */
} HW_QSPI_PAD;

/**
 * \brief QSPI pad direction
 *
 */
typedef enum {
        HW_QSPI_INPUT = 0,      /**< The QSPI pad is input */
        HW_QSPI_OUTPUT = 1      /**< The QSPI pad is output */
} HW_QSPI_DIRECTION;

/**
 * \brief QSPI pads slew rate control
 *
 */
typedef enum {
        HW_QSPI_SLEW_RATE_0,       /**< xx V/ns (weak) */
        HW_QSPI_SLEW_RATE_1,       /**< xx V/ns */
        HW_QSPI_SLEW_RATE_2,       /**< xx V/ns */
        HW_QSPI_SLEW_RATE_3        /**< xx V/ns (strong) */
} HW_QSPI_SLEW_RATE;

/**
 * \brief QSPI pads drive current
 *
 */
typedef enum {
        HW_QSPI_DRIVE_CURRENT_4,   /**< 4 mA */
        HW_QSPI_DRIVE_CURRENT_8,   /**< 8 mA */
        HW_QSPI_DRIVE_CURRENT_12,  /**< 12 mA */
        HW_QSPI_DRIVE_CURRENT_16,  /**< 16 mA */
} HW_QSPI_DRIVE_CURRENT;

/**
 * \brief QSPI clock divider setting
 *
 */
typedef enum {
        HW_QSPI_DIV_1 = 0,      /**< divide by 1 */
        HW_QSPI_DIV_2 = 1,      /**< divide by 2 */
        HW_QSPI_DIV_4 = 2,      /**< divide by 4 */
        HW_QSPI_DIV_8 = 3       /**< divide by 8 */
} HW_QSPI_DIV;

/**
 * \brief Counter of the delay to read the memory status after an erase or erase resume instruction
 *
 */
typedef enum {
        HW_QSPI_STSDLY_RESSTS = 0,      /**< QSPIC_RESSTS_DLY in STATUSCMD register, counting on the
                                             QSPI clock                                                 */
        HW_QSPI_STSDLY_RESSUS = 1       /**< QSPIC_RESSUS_DLY in ERASECMDB register, counting on the
                                             222 kHz clock for HW_QSPIC and 288 kHz for HW_QSPIC2       */
} HW_QSPI_STSDLY;

/**
 * \brief Type of QSPI_CLK edge producing the QSPI_CS signal
 *
 */
typedef enum {
        HW_QSPI_CS_MODE_RISING = 0,     /**< The QSPI_CS is produced with the rising edge of the QSPI_SCK */
        HW_QSPI_CS_MODE_FALLING = 1     /**< The QSPI_CS is produced with the falling edge of the QSPI_SCK */
} HW_QSPI_CS_MODE;

/**
 * \brief The length of a burst operation for a QSPI RAM device
 *
 */
typedef enum {
        HW_QSPI2_MEMBLEN_0 = 0,     /**< The external memory device is capable to
                                         implement incremental burst of unspecified length */
        HW_QSPI2_MEMBLEN_4 = 1,     /**< The external memory device implements a
                                         wrapping burst of length 4 bytes */
        HW_QSPI2_MEMBLEN_8 = 2,     /**< The external memory device implements a
                                         wrapping burst of length 8 bytes */
        HW_QSPI2_MEMBLEN_16 = 3,     /**< The external memory device implements a
                                         wrapping burst of length 16 bytes */
        HW_QSPI2_MEMBLEN_32 = 4,     /**< The external memory device implements a
                                         wrapping burst of length 32 bytes */
        HW_QSPI2_MEMBLEN_64 = 5,     /**< The external memory device implements a
                                         wrapping burst of length 64 bytes */
} HW_QSPI2_MEMBLEN;

/**
 * \brief QSPI configuration
 *
 */
typedef struct {
        HW_QSPI_ADDR_SIZE address_size;
        HW_QSPI_POL idle_clock;
        HW_QSPI_SAMPLING_EDGE sampling_edge;
} qspi_config;

/**
 * \brief Common QSPIC registers
 */
struct qspi_regs {                      /*!< (@ 0x3x000000) QSPIC Structure                                                     */
  __IO uint32_t  QSPIC_CTRLBUS_REG;     /*!< (@ 0x3x000000) SPI Bus control register for the Manual mode                        */
  __IO uint32_t  QSPIC_CTRLMODE_REG;    /*!< (@ 0x3x000004) Mode Control register                                               */
  __IO uint32_t  QSPIC_RECVDATA_REG;    /*!< (@ 0x3x000008) Received data for the Manual mode                                   */
  __IO uint32_t  QSPIC_BURSTCMDA_REG;   /*!< (@ 0x3x00000C) The way of reading in Auto mode (command register A)                */
  __IO uint32_t  QSPIC_BURSTCMDB_REG;   /*!< (@ 0x3x000010) The way of reading in Auto mode (command register B)                */
  __IO uint32_t  QSPIC_STATUS_REG;      /*!< (@ 0x3x000014) The status register of the QSPI controller                          */
  union {
          __IO uint32_t  data32;
          __IO uint16_t  data16;
          __IO uint8_t   data8;
  } QSPIC_WRITEDATA_REG;                /*!< (@ 0x3x000018) Write data to SPI Bus for the Manual mode                           */
  union {
          __IO uint32_t  data32;
          __IO uint16_t  data16;
          __IO uint8_t   data8;
  } QSPIC_READDATA_REG;                 /*!< (@ 0x3x00001C) Read data from SPI Bus for the Manual mode                          */
  union {
          __IO uint32_t  data32;
          __IO uint16_t  data16;
          __IO uint8_t   data8;
  } QSPIC_DUMMYDATA_REG;                /*!< (@ 0x3x000020) Send dummy clocks to SPI Bus for the Manual mode                    */
  __IO uint32_t  QSPIC_ERASECTRL_REG;   /*!< (@ 0x3x000024) QSPI Erase control register                                         */
  __IO uint32_t  QSPIC_ERASECMDA_REG;   /*!< (@ 0x3x000028) The way of erasing in Auto mode (command register A)                */
  __IO uint32_t  QSPIC_ERASECMDB_REG;   /*!< (@ 0x3x00002C) The way of erasing in Auto mode (command register B)                */
  __IO uint32_t  QSPIC_BURSTBRK_REG;    /*!< (@ 0x3x000030) Read break sequence in Auto mode                                    */
  __IO uint32_t  QSPIC_STATUSCMD_REG;   /*!< (@ 0x3x000034) The way of reading the status of external device in Auto mode       */
  __IO uint32_t  QSPIC_CHCKERASE_REG;   /*!< (@ 0x3x000038) Check erase progress in Auto mode                                   */
  __IO uint32_t  QSPIC_GP_REG;          /*!< (@ 0x3x00003C) QSPI General Purpose control register                               */
};

/**
 * \brief QSPIC instructions
 */
struct qspic_instructions {
        /* Set read instruction struct */
        bool set_read_instruction;              /*!< When true, the read instruction is set and called */
        struct {
                uint8_t inst;                   /*!< Instruction for Incremental Burst or Single read access. This value is
                                                the selected instruction in the cases of incremental burst or single read
                                                access. Also this value is used when a wrapping burst is not supported. */
                HW_QSPI_INST_MODE inst_mode;
                uint8_t dummy_count;            /*!< The number of dummy bytes to send (0..4) */
                HW_QSPI_BUS_MODE inst_phase;    /*!< The mode of the QSPI bus during the instruction phase */
                HW_QSPI_BUS_MODE addr_phase;    /*!< The mode of the QSPI bus during the address phase */
                HW_QSPI_BUS_MODE dummy_phase;   /*!< The mode of the QSPI bus during the dummy phase */
                HW_QSPI_BUS_MODE data_phase;    /*!< The mode of the QSPI bus during the data phase */
        } read_instruction;

        /* Set read status instruction struct */
        bool set_read_status_instruction;       /*!< When true, the read status instruction is set and called */
        struct {
                uint8_t inst;                   /*!< The code value of the read status instruction */
                HW_QSPI_BUS_MODE inst_phase;    /*!< The mode of the QSPI bus during the instruction phase */
                HW_QSPI_BUS_MODE receive_phase; /*!< The mode of the QSPI bus during the receive phase */
                uint8_t busy_pos;               /*!< The position of the Busy bit in the status (7 - 0) */
                HW_QSPI_BUSY busy_val;          /*!< The value of the Busy bit which means that the flash is busy
                                                0 - flash is busy when the Busy bit is equal to 0,
                                                1 - flash is busy when the Busy bit is equal to 1 */
                uint8_t read_delay;             /*!< The minimum time distance between the read status instruction and previous
                                                instructions like erase or erase resume.
                                                0       - don't wait. The controller can read the memory status register
                                                immediately.
                                                1..63 - the controller waits at least this number of QSPI_CLK cycles before
                                                reading the memory status register following the end of a previous erase or erase resume */
                HW_QSPI_STSDLY delay_sel;
        } read_status_instruction;

        /* Set erase instruction struct */
        bool set_erase_instruction;             /*!< When true, the erase instruction is set and called */
        struct {
                uint8_t inst;                   /*!< The code value of the erase instruction */
                HW_QSPI_BUS_MODE inst_phase;    /*!< The mode of the QSPI bus during the instruction phase */
                HW_QSPI_BUS_MODE addr_phase;    /*!< The mode of the QSPI bus during the address phase */
                uint8_t hclk_cycles;            /*!< The number of AMBA AHB hclk cycles (0..15) without memory read requests
                                                before the controller can execute erase or erase resume instructions. */
                uint8_t cs_hi_cycles;           /*!< The minimum number of QSPI bus clock cycles (0..31) that QSPI_CS remains
                                                high after the execution of write enable, erase, erase suspend and erase
                                                resume instructions. */
        } erase_instruction;

        /* Set write enable instruction struct */
        bool set_write_enable_instruction;      /*!< When true, the write enable instruction is set and called */
        struct {
                uint8_t inst;                   /*!< The code value of the write enable instruction */
                HW_QSPI_BUS_MODE inst_phase;    /*!< The mode of the QSPI bus during the instruction phase */
        } write_enable_instruction;

        /* Set wrapping burst instruction struct */
        bool set_wrapping_burst_instruction;    /*!< When true, the wrapping burst instruction is set and called */
        struct {
                uint8_t inst;                   /*!< The code value of the wrapping burst instruction */
                HW_QSPI_WRAP_LEN len;           /*!< The selected length of a wrapping burst */
                HW_QSPI_WRAP_SIZE size;         /*!< The selected data size of a wrapping burst */
        } wrapping_burst_instruction;

        /* Set suspend resume instruction struct */
        bool set_suspend_resume_instruction;    /*!< When true, the suspend/resume instruction is set and called */
        struct {
                uint8_t erase_suspend_inst;     /*!< The code value of the erase suspend instruction */
                HW_QSPI_BUS_MODE suspend_inst_phase;/*!< The mode of the QSPI bus during the instruction phase of the suspend instruction*/
                uint8_t erase_resume_inst;      /*!< The code value of the erase resume instruction */
                HW_QSPI_BUS_MODE resume_inst_phase;/*!< The mode of the QSPI bus during the instruction phase of the resume instruction*/
                uint8_t minimum_delay;           /*!< The minimum time distance between the read status instruction and previous
                                                instructions like erase or erase resume.
                                                0       - don't wait. The controller can read the memory status register
                                                immediately.
                                                1..63 - the controller waits at least this number of QSPI_CLK cycles before
                                                reading the memory status register following the end of a previous erase or erase resume */
        } suspend_resume_instruction;

        /* Set write instruction struct */
        bool set_write_instruction;             /*!< When true, the write instruction is set and called */
        struct {
                uint8_t inst;                   /*!< The code value of the write instruction */
                HW_QSPI_BUS_MODE inst_phase;    /*!< The mode of the QSPI bus during the instruction phase */
                HW_QSPI_BUS_MODE addr_phase;    /*!< The mode of the QSPI bus during the address phase */
                HW_QSPI_BUS_MODE data_phase;    /*!< The mode of the QSPI bus during the data phase */
        } write_instruction;
};

/**
 * \brief QSPI Controller id
 *
 */
typedef struct qspi_regs* HW_QSPIC_ID;
#define HW_QSPIC        ((HW_QSPIC_ID)QSPIC_BASE)
#if dg_configUSE_HW_QSPI2
#define HW_QSPIC2       ((HW_QSPIC_ID)QSPIC2_BASE)
#endif

/**
 * \brief Get the value of a field of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_QSPIC_REG_GETF(id, reg, field) \
        (((id)->QSPIC_##reg##_REG & QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk) >> \
         QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)

/**
 * \brief Set the value of a field of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_QSPIC_REG_SETF(id, reg, field, new_val) \
        (id)->QSPIC_##reg##_REG = (((id)->QSPIC_##reg##_REG & \
                                    ~QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk) | \
                                   (QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk & \
                                    ((new_val) << QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)))

/**
 * \brief Set a bit of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register bit to set
 *
 */
#define HW_QSPIC_REG_SET_BIT(id, reg, field) \
        (id)->QSPIC_##reg##_REG |= (1 << QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)

/**
 * \brief Clear a bit of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register bit to clear
 *
 */
#define HW_QSPIC_REG_CLR_BIT(id, reg, field) \
        (id)->QSPIC_##reg##_REG &= ~QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk

/**
 * \brief Enable CS on QSPI bus
 *
 * Use this in manual mode.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_cs_enable(HW_QSPIC_ID id)
{
        id->QSPIC_CTRLBUS_REG = QSPIC_QSPIC_CTRLBUS_REG_QSPIC_EN_CS_Msk;
}

/**
 * \brief Disable CS on QSPI bus
 *
 * Use this in manual mode.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_cs_disable(HW_QSPIC_ID id)
{
        id->QSPIC_CTRLBUS_REG = QSPIC_QSPIC_CTRLBUS_REG_QSPIC_DIS_CS_Msk;
}

/**
 * \brief Check if SPI Bus is busy
 *
 * \param [in] id QSPI controller id
 *
 * \return false - SPI Bus is idle,
 *         true  - SPI Bus is active. ReadData, WriteData or DummyData activity is in progress
 *
 */
__STATIC_FORCEINLINE bool hw_qspi_is_busy(HW_QSPIC_ID id)
{
        return id->QSPIC_STATUS_REG;
}

/**
 * \brief Generate 32 bits read transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 * \return data read during read transfer
 *
 */
__STATIC_FORCEINLINE uint32_t hw_qspi_read32(HW_QSPIC_ID id)
{
        return id->QSPIC_READDATA_REG.data32;
}

/**
 * \brief Generate 16 bits read transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 * \return data read during read transfer
 *
 */
__STATIC_FORCEINLINE uint16_t hw_qspi_read16(HW_QSPIC_ID id)
{
        return id->QSPIC_READDATA_REG.data16;
}

/**
 * \brief Generate 8 bits read transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 * \return data read during read transfer
 *
 */
__STATIC_FORCEINLINE uint8_t hw_qspi_read8(HW_QSPIC_ID id)
{
        return id->QSPIC_READDATA_REG.data8;
}

/**
 * \brief Generate 32 bits write transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 * \param [in] data data to transfer
 *
 */
__STATIC_FORCEINLINE void hw_qspi_write32(HW_QSPIC_ID id, uint32_t data)
{
        id->QSPIC_WRITEDATA_REG.data32 = data;
}

/**
 * \brief Generate 16 bits write transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 * \param [in] data data to transfer
 *
 */
__STATIC_FORCEINLINE void hw_qspi_write16(HW_QSPIC_ID id, uint16_t data)
{
        id->QSPIC_WRITEDATA_REG.data16 = data;
}

/**
 * \brief Generate 8 bits write transfer on QSPI bus
 *
 * The data is transferred using the selected mode of the SPI bus (SPI, Dual
 * SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 * \param [in] data data to transfer
 *
 */
__STATIC_FORCEINLINE void hw_qspi_write8(HW_QSPIC_ID id, uint8_t data)
{
        id->QSPIC_WRITEDATA_REG.data8 = data;
}

/**
 * \brief Generate clock pulses on the SPI bus for a 32-bit transfer
 *
 * During this activity in the SPI bus, the QSPI_IOx data pads are in hi-z state.
 * Number of pulses depends on selected mode of the SPI bus (SPI, Dual SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_dummy32(HW_QSPIC_ID id)
{
        id->QSPIC_DUMMYDATA_REG.data32 = 0;
}

/**
 * \brief Generate clock pulses on the SPI bus for a 16-bit transfer
 *
 * During this activity in the SPI bus, the QSPI_IOx data pads are in hi-z state.
 * Number of pulses depends on selected mode of the SPI bus (SPI, Dual SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_dummy16(HW_QSPIC_ID id)
{
        id->QSPIC_DUMMYDATA_REG.data16 = 0;
}

/**
 * \brief Generate clock pulses on the SPI bus for an 8-bit transfer
 *
 * During this activity in the SPI bus, the QSPI_IOx data pads are in hi-z state.
 * Number of pulses depends on selected mode of the SPI bus (SPI, Dual SPI, Quad SPI).
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_dummy8(HW_QSPIC_ID id)
{
        id->QSPIC_DUMMYDATA_REG.data8 = 0;
}

/**
 * \brief Specifies the address size that the QSPI memory uses
 *
 * The controller uses 32 of 24 bits for addresses during Auto mode transfer.
 *
 * \param [in] id QSPI controller id
 * \param [in] size selects 32 or 24 bit address size
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_address_size(HW_QSPIC_ID id, HW_QSPI_ADDR_SIZE size)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, USE_32BA, size == HW_QSPI_ADDR_SIZE_32);
}

/**
 * \brief Get the address size that the QSPI memory uses
 *
 * The controller uses 32 or 24 bits for addresses during Auto mode transfer.
 *
 * \param [in] id QSPI controller id
 *
 * \return the currently selected address size
 *
 * \sa hw_qspi_set_address_size
 *
 */
__STATIC_FORCEINLINE HW_QSPI_ADDR_SIZE hw_qspi_get_address_size(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, USE_32BA) ? HW_QSPI_ADDR_SIZE_32 : HW_QSPI_ADDR_SIZE_24;
}

/**
 * \brief Get read pipe clock delay
 *
 * \param [in] id QSPI controller id
 *
 * \return read pipe clock delay relative to the falling edge of QSPI_SCK
 *
 */
__STATIC_FORCEINLINE uint8_t hw_qspi_get_read_pipe_clock_delay(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, PCLK_MD);
}

/**
 * \brief Set the read pipe clock delay
 *
 * \param [in] id QSPI controller id
 * \param [in] delay read pipe clock delay relative to the falling edge of QSPI_SCK
 *                   value should be in range 0..7
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_pipe_clock_delay(HW_QSPIC_ID id, uint8_t delay)
{
        ASSERT_WARNING(delay < 8);
        // Delay cannot be greater than 2 when VDD_CORE is not 1.2V
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, VDD_LEVEL) == 0x3 || delay <= 2);
        HW_QSPIC_REG_SETF(id, CTRLMODE, PCLK_MD, delay);
}

/**
 * \brief Check if the data read pipe is enabled
 *
 * \param [in] id QSPI controller id
 *
 * \return true if read pipe is enabled, false otherwise
 *
 */
__STATIC_FORCEINLINE bool hw_qspi_is_read_pipe_enabled(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, RPIPE_EN);
}

/**
 * \brief Enable the data read pipe
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_read_pipe_enable(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_SET_BIT(id, CTRLMODE, RPIPE_EN);
}

/**
 * \brief Disable the data read pipe
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_read_pipe_disable(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_CLR_BIT(id, CTRLMODE, RPIPE_EN);
}

/**
 * \brief Get read sampling edge
 *
 * \param [in] id QSPI controller id
 *
 * \return type of QSPI_CLK edge for sampling received data
 *
 */
__STATIC_FORCEINLINE HW_QSPI_SAMPLING_EDGE hw_qspi_get_read_sampling_edge(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, RXD_NEG) ? HW_QSPI_SAMPLING_EDGE_NEGATIVE : HW_QSPI_SAMPLING_EDGE_POSITIVE;
}

/**
 * \brief Set read sampling edge
 *
 * \param [in] id QSPI controller id
 * \param [in] edge sets whether read samples are taken on rising or falling edge
 *                  of QSPI_SCK
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_sampling_edge(HW_QSPIC_ID id, HW_QSPI_SAMPLING_EDGE edge)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, RXD_NEG, edge == HW_QSPI_SAMPLING_EDGE_NEGATIVE);
}

/**
 * \brief Check if hready signal is used
 *
 * \param [in] id QSPI controller id
 *
 * \return HW_QSPI_HREADY_MODE_STALLING - wait states are added via hready signal during
 *                                        access to QSPIC_WRITEDATA, QSPIC_READDATA and
 *                                        QSPIC_DUMMYDATA registers.
 *         HW_QSPI_HREADY_MODE_FIXED    - wait states are not added via hready signal
 *                                        during access to QSPIC_WRITEDATA, QSPIC_READDATA
 *                                        and QSPIC_DUMMYDATA registers. In this case read
 *                                        QSPI_STATUS register to check the end of
 *                                        activity at the SPI bus
 *
 */
__STATIC_FORCEINLINE HW_QSPI_HREADY_MODE hw_qspi_get_hready_mode(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, HRDY_MD) ? HW_QSPI_HREADY_MODE_FIXED : HW_QSPI_HREADY_MODE_STALLING;
}

/**
 * \brief Enable/disable adding wait states during register access
 *
 * \param [in] id QSPI controller id
 * \param [in] mode sets whether wait states are added via hready signal during access to
 *                  QSPIC_WRITEDATA, QSPIC_READDATA and QSPIC_DUMMYDATA registers.
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_hready_mode(HW_QSPIC_ID id, HW_QSPI_HREADY_MODE mode)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, HRDY_MD, mode == HW_QSPI_HREADY_MODE_FIXED);
}

/**
 * \brief Get clock mode
 *
 * \param [in] id QSPI controller id
 *
 * \return HW_QSPI_POL_LOW  - SPI mode 0 is used for QSPI_CLK;
 *                            the QSPI_SCK is low when QSPI_CS is high (idle),
 *         HW_QSPI_POL_HIGH - SPI mode 3 is used for QSPI_CLK;
 *                            the QSPI_SCK is high when QSPI_CS is high (idle)
 *
 */
__STATIC_FORCEINLINE HW_QSPI_POL hw_qspi_get_clock_mode(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, CLK_MD) ? HW_QSPI_POL_HIGH : HW_QSPI_POL_LOW;
}

/**
 * \brief Set clock mode
 *
 * \param [in] id QSPI controller id
 * \param [in] mode HW_SPI_POL_LOW  - SPI mode 0 will be used for the QSPI_CLK;
 *                                    The QSPI_SCK will be low when QSPI_CS is high (idle),
 *                  HW_SPI_POL_HIGH - SPI mode 3 will be used for QSPI_CLK;
 *                                    The QSPI_SCK will be high when QSPI_CS is high (idle)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_clock_mode(HW_QSPIC_ID id, HW_QSPI_POL mode)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, CLK_MD, mode == HW_QSPI_POL_HIGH);
}

/**
 * \brief Set pad direction
 *
 * \param [in] id QSPI controller id
 * \param [in] pad QSPI pad id
 * \param [in] direction input/output in SPI or Dual SPI mode.
 *                       When the Auto Mode is selected and the QUAD SPI is used,
 *                       set this to input
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_pad_direction(HW_QSPIC_ID id, HW_QSPI_PAD pad, HW_QSPI_DIRECTION direction)
{
        if (pad == HW_QSPI_IO2) {
                HW_QSPIC_REG_SETF(id, CTRLMODE, IO2_OEN, direction == HW_QSPI_OUTPUT);
        } else if (pad == HW_QSPI_IO3) {
                HW_QSPIC_REG_SETF(id, CTRLMODE, IO3_OEN, direction == HW_QSPI_OUTPUT);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Set pad value
 *
 * \param [in] id QSPI controller id
 * \param [in] pad QSPI pad id
 * \param [in] dat The value of the pad if its direction is HW_QSPI_OUTPUT
 *
 * \sa hw_qspi_set_pad_direction
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_pad_value(HW_QSPIC_ID id, HW_QSPI_PAD pad, bool dat)
{
        if (pad == HW_QSPI_IO2) {
                HW_QSPIC_REG_SETF(id, CTRLMODE, IO2_DAT, dat);
        } else if (pad == HW_QSPI_IO3) {
                HW_QSPIC_REG_SETF(id, CTRLMODE, IO3_DAT, dat);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Get pad value
 *
 * \param [in] id QSPI controller id
 * \param [in] pad QSPI pad id
 *
 * \return The value of the pad
 *
 */
__STATIC_FORCEINLINE bool hw_qspi_get_pad_value(HW_QSPIC_ID id, HW_QSPI_PAD pad)
{
        if (pad == HW_QSPI_IO2) {
                return HW_QSPIC_REG_GETF(id, CTRLMODE, IO2_DAT);
        } else if (pad == HW_QSPI_IO3) {
                return HW_QSPIC_REG_GETF(id, CTRLMODE, IO3_DAT);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Select QSPI bus mode
 *
 * Use this in manual mode.
 *
 * \note Selecting QUAD mode will automatically configure pins IO2 and IO3 to input.
 *
 * \param [in] id QSPI controller id
 * \param [in] mode selects single/dual/quad mode for QSPI bus
 *
 */
__RETAINED_CODE void hw_qspi_set_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE mode);

/**
 * \brief Select access mode on QSPI
 *
 * \note Selecting auto mode when any command previously configured has chosen
 * QUAD mode for any phase will automatically configure pins IO2 and IO3 to input.
 *
 * \param [in] id QSPI controller id
 * \param [in] mode HW_QSPI_ACCESS_MODE_AUTO auto mode selected,
 *                  HW_QSPI_ACCESS_MODE_MANUAL manual mode selected
 *
 * \sa hw_qspi_set_read_instruction
 * \sa hw_qspi_set_read_status_instruction
 * \sa hw_qspi_set_suspend_resume_instructions
 * \sa hw_qspi_set_write_enable_instruction
 * \sa hw_qspi_set_extra_byte
 * \sa hw_qspi_set_erase_instruction
 * \sa hw_qspi_set_break_sequence
 *
 */
__RETAINED_CODE void hw_qspi_set_access_mode(HW_QSPIC_ID id, HW_QSPI_ACCESS_MODE mode);

/**
 * \brief Read access mode
 *
 * \param [in] id QSPI controller id
 *
 * \return HW_QSPI_ACCESS_MODE_AUTO auto mode is selected,
 *         HW_QSPI_ACCESS_MODE_MANUAL manual mode is selected
 *
 */
__STATIC_FORCEINLINE HW_QSPI_ACCESS_MODE hw_qspi_get_access_mode(HW_QSPIC_ID id)
{
        return HW_QSPIC_REG_GETF(id, CTRLMODE, AUTO_MD) ? HW_QSPI_ACCESS_MODE_AUTO : HW_QSPI_ACCESS_MODE_MANUAL;
}

/**
 * \brief Set the number of dummy bytes sent when a read instruction is executed
 *
 * \param [in] id QSPI controller id
 * \param [in] count number of dummy bytes to send (0..4)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_dummy_bytes_count(HW_QSPIC_ID id, uint8_t count)
{
        ASSERT_WARNING(count < 5);
        if (count == 3) {
                HW_QSPIC_REG_SET_BIT(id, BURSTCMDB, DMY_FORCE);
        } else {
                id->QSPIC_BURSTCMDB_REG = (id->QSPIC_BURSTCMDB_REG &
                                           ~(REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_FORCE) |
                                             REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM))) |
                                          BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM, count==4 ? 3 : count);
        }
}

/**
 * \brief Set an extra byte to use with read instructions
 *
 * \param [in] id QSPI controller id
 * \param [in] extra_byte an extra byte transferred after the address asking memory to
 *                        stay in continuous read mode or wait for a normal instruction
 *                        after CS goes inactive
 * \param [in] bus_mode the mode of the SPI bus during the extra byte phase.
 * \param [in] half_disable_out true  - disable (hi-z) output during the transmission
 *                                      of bits [3:0] of extra byte
 *                              false - transmit the complete extra byte
 *
 * \sa hw_qspi_set_read_instruction
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_extra_byte(
        HW_QSPIC_ID id, uint8_t extra_byte, HW_QSPI_BUS_MODE bus_mode, bool half_disable_out)
{
        id->QSPIC_BURSTCMDA_REG = (id->QSPIC_BURSTCMDA_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_BYTE) |
                                     REG_MSK(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_TX_MD))) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_BYTE, extra_byte) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_TX_MD, bus_mode);

        id->QSPIC_BURSTCMDB_REG = (id->QSPIC_BURSTCMDB_REG &
                                   ~(REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_BYTE_EN) |
                                     REG_MSK(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_HF_DS))) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_BYTE_EN, 1) |
                                  BITS32(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_HF_DS, half_disable_out);
}

/**
 * \brief Set the number of clocks that CS stays high
 *
 * \param [in] id QSPI controller id
 * \param [in] clock_count between the transmission of two different instructions to the
 *                         memory, the QSPI bus stays in idle state (QSPI_CS high)
 *                         for at least this number of SPI clock cycles (0..7). See QSPIC_ERS_CS_HI
 *                         in QSPIC_ERASECMDB_REG/QSPIC2_ERASECMDB_REG for an exception
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_min_cs_high(HW_QSPIC_ID id, uint8_t clock_count)
{
        ASSERT_WARNING(clock_count < 8);
        HW_QSPIC_REG_SETF(id, BURSTCMDB, CS_HIGH_MIN, clock_count);
}

/**
 * \brief Get erase status
 *
 * \param [in] id QSPI controller id
 *
 * \return the progress of sector/block erasing
 *
 */
__STATIC_FORCEINLINE HW_QSPI_ERS hw_qspi_get_erase_status(HW_QSPIC_ID id)
{
        id->QSPIC_CHCKERASE_REG = 0;
        return HW_QSPIC_REG_GETF(id, ERASECTRL, ERS_STATE);
}

/**
 * \brief Erase block/sector of flash memory
 *
 * For this function to work, erase instructions must be set up
 * with hw_qspi_set_erase_instruction.
 * The user should call hw_qspi_get_erase_status to check if the block was erased.
 *
 * \note The user should call hw_qspi_get_erase_status and receive status 0, for
 * the QSPI controller to switch to manual mode.
 *
 * \param [in] id QSPI controller id
 * \param [in] addr memory address of the block/sector to be erased
 *                  For 24-bit addressing, bits [23:12] are the block/sector
 *                  address bits. Bits [11:0] are ignored by the controller.
 *                  For 32-bit addressing, bits [31:12] are the block/sector
 *                  address bits. Bits [11:0] are ignored by the controller
 *
 * \sa hw_qspi_set_erase_instruction
 * \sa hw_qspi_get_erase_status
 * \sa hw_qspi_set_access_mode
 *
 */
__RETAINED_CODE void hw_qspi_erase_block(HW_QSPIC_ID id, uint32_t addr);

/**
 * \brief Enable burst break sequence
 *
 * \param [in] id QSPI controller id
 * \param [in] sequence the command applied to the memory device to abandon the continuous read mode
 * \param [in] mode mode of the QSPI Bus during the transmission of the burst break sequence
 * \param [in] size size of Burst Break Sequence
 * \param [in] dis_out disable output during the transmission of the second half (sequence[3:0]).
 *                     Setting this bit is only useful if size = HW_QSPI_BREAK_SEQ_SIZE_2B.
 *                     false - drive the QSPI bus during the transmission of sequence[3:0],
 *                     true  - leave the QSPI bus in Hi-Z during the transmission of sequence[3:0]
 *
 */
__STATIC_FORCEINLINE void hw_qspi_burst_break_sequence_enable(
        HW_QSPIC_ID id, uint16_t sequence, HW_QSPI_BUS_MODE mode, HW_QSPI_BREAK_SEQ_SIZE size, bool dis_out)
{
        id->QSPIC_BURSTBRK_REG =
                BITS32(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_SEC_HF_DS, dis_out) |
                BITS32(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_SZ, size) |
                BITS32(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_TX_MD, mode) |
                BITS32(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_EN, true) |
                BITS32(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_WRD, sequence);
}

/**
 * \brief Disable burst break sequence
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_burst_break_sequence_disable(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_CLR_BIT(id, BURSTBRK, BRK_EN);
}

/**
 * \brief Set configuration of QSPI pads
 *
 * \param [in] id QSPI controller id
 * \param [in] rate QSPI pads slew rate control
 * \param [in] current QSPI pads drive current
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_pads(
        HW_QSPIC_ID id, HW_QSPI_SLEW_RATE rate, HW_QSPI_DRIVE_CURRENT current)
{
        id->QSPIC_GP_REG =
                BITS32(QSPIC, QSPIC_GP_REG, QSPIC_PADS_SLEW, rate) |
                BITS32(QSPIC, QSPIC_GP_REG, QSPIC_PADS_DRV, current);
}

/**
 * \brief QSPI controller initialization
 *
 * This function will enable QSPI controller in manual mode.
 *
 * \param [in] id QSPI controller id
 * \param [in] cfg pointer to QSPI configuration
 *
 * \note This function doesn't change QSPI_DIV
 *
 */
__RETAINED_CODE void hw_qspi_init(HW_QSPIC_ID id, const qspi_config *cfg);

/**
 * \brief Enable QSPI controller clock
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_clock_enable(HW_QSPIC_ID id)
{
        GLOBAL_INT_DISABLE();
        if (id == HW_QSPIC) {
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE);
#if dg_configUSE_HW_QSPI2
        } else if (id == HW_QSPIC2) {
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, QSPI2_ENABLE);
#endif
        } else {
                ASSERT_WARNING(0);
        }
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable QSPI controller clock
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_clock_disable(HW_QSPIC_ID id)
{
        GLOBAL_INT_DISABLE();
        if (id == HW_QSPIC) {
                REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE);
#if dg_configUSE_HW_QSPI2
        } else if (id == HW_QSPIC2) {
                REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, QSPI2_ENABLE);
#endif
        } else {
                ASSERT_WARNING(0);
        }
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the QSPI clock divider
 *
 * \param [in] id QSPI controller id
 * \param [in] div QSPI clock divider
 *
 * \sa HW_QSPI_DIV
 */
__STATIC_FORCEINLINE void hw_qspi_set_div(HW_QSPIC_ID id, HW_QSPI_DIV div)
{
        GLOBAL_INT_DISABLE();

        uint32_t reg = CRG_TOP->CLK_AMBA_REG;
        if (id == HW_QSPIC) {
                // Set the divider
                REG_SET_FIELD(CRG_TOP, CLK_AMBA_REG, QSPI_DIV, reg, div);
#if dg_configUSE_HW_QSPI2
        } else if (id == HW_QSPIC2) {
                // Set the divider
                REG_SET_FIELD(CRG_TOP, CLK_AMBA_REG, QSPI2_DIV, reg, div);
#endif
        } else {
                ASSERT_WARNING(0);
        }
        CRG_TOP->CLK_AMBA_REG = reg;

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get the QSPI clock divider
 *
 * \param [in] id QSPI controller id
 *
 * \return the QSPI clock divider setting
 *
 * \sa HW_QSPI_DIV
 */
__STATIC_FORCEINLINE HW_QSPI_DIV hw_qspi_get_div(HW_QSPIC_ID id)
{
        if (id == HW_QSPIC) {
                return REG_GETF(CRG_TOP, CLK_AMBA_REG, QSPI_DIV);
#if dg_configUSE_HW_QSPI2
        } else if (id == HW_QSPIC2) {
                return REG_GETF(CRG_TOP, CLK_AMBA_REG, QSPI2_DIV);
#endif
        } else {
                ASSERT_WARNING(0);
        }
        return 0;
}

/**
 * \brief Enable QSPI initialization after wake-up.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_enable_init(HW_QSPIC_ID id)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        ASSERT_WARNING(REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN) == 0 ||
                       REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) == 1)
#endif

        if (id == HW_QSPIC) {
                GLOBAL_INT_DISABLE();
                REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, QSPI_INIT);
                GLOBAL_INT_RESTORE();
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Disable QSPI initialization after wake-up.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_disable_init(HW_QSPIC_ID id)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        ASSERT_WARNING(REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN) == 0 ||
                       REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) == 1)
#endif

        if (id == HW_QSPIC) {
                GLOBAL_INT_DISABLE();
                REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, QSPI_INIT);
                GLOBAL_INT_RESTORE();
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Get QSPI initialization after wake-up state.
 *
 * \param [in] id QSPI controller id
 *
 * \return true is QSPI initialization is enabled
 *
 * \note The function applies only for HW_QSPIC
 */
__STATIC_FORCEINLINE bool hw_qspi_is_init_enabled(HW_QSPIC_ID id)
{
        ASSERT_WARNING(id == HW_QSPIC);
        return REG_GETF(CRG_TOP, SYS_CTRL_REG, QSPI_INIT) == 1;
}


/**
 * \brief Defines the behavior of the controller when the internal buffer is full
 *        and there are more data to be retrieved for the current burst.
 *        It has meaning only for reads in auto mode.
 *
 * \param [in] id QSPI controller id
 * \param [in] enable
 *        false - The access to the flash device is not terminated when the internal
 *                buffer has no empty space. In this case the QSPI_SCK clock is blocked
 *                until space is freed in the internal buffer.
 *        true  - The access to the flash device is terminated when the internal buffer
 *                has no empty space. A new access to the flash device will be initiated
 *                when addresses are requested that are not present in the internal
 *                buffer.
 *
 * \note In both cases the access to the flash device is terminated when there is no
 *       read request.
 *
 */
__STATIC_FORCEINLINE void hw_qspi_buf_lim(HW_QSPIC_ID id, bool enable)
{
        if (id == HW_QSPIC) {
                REG_SETF(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_BUF_LIM_EN, enable);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Write the NONCE value for the AES-CTR decryption
 *
 * The QSPI controller decrypts Flash contents on-the-fly using AES-CTR. AES-CTR uses a
 * 16-byte counter block (CTRB). The first 8 bytes of CTRB consist of the NONCE while the
 * other 8-bytes are produced automatically by the hardware.
 *
 * \param [in] id QSPI controller id
 * \param [in] nonce Pointer to the buffer containing the 8 NONCE bytes
 */
__RETAINED_CODE void hw_qspi_write_nonce(HW_QSPIC_ID id, const uint8_t *nonce);

/**
 * \brief Write the key for the AES-CTR decryption
 *
 * The QSPI controller decrypts Flash contents on-the-fly using AES-CTR. This function
 * is used to store the key to the QSPI controller.
 *
 * \param [in] id QSPI controller id
 * \param [in] key Pointer to the buffer containing the 32 key bytes
 */
__RETAINED_CODE void hw_qspi_write_key(HW_QSPIC_ID id, const uint8_t *key);

/**
 * \brief Set the address range for on-the-fly decryption
 *
 * The QSPI controller decrypts Flash contents on-the-fly using AES-CTR. This function is used
 * to define the address range in which the QSPI controller should decrypt the contents. The start
 * and end address of the range must be aligned to 1KByte (1024 bytes) blocks. This function will adjust
 * their values into the closest 1KByte, saddr is rounded down and eaddr is rounded up.
 * The address values must be absolute Flash addresses and not offsets within the QSPI address range of the bus.
 *
 * \param [in] id QSPI controller id
 * \param [in] saddr Start address of the decryption area in the Flash
 * \param [in] eaddr End address of the decryption area in the Flash
 */
__STATIC_FORCEINLINE void hw_qspi_set_decryption_address_range(HW_QSPIC_ID id, uint32_t saddr, uint32_t eaddr)
{
        if (id == HW_QSPIC) {
        /* End address must always be larger than start address */
                ASSERT_ERROR(eaddr > saddr);

                if (eaddr & 0x3FF) {
                        eaddr += 0x400;
                }
                QSPIC->QSPIC_CTR_SADDR_REG = saddr & 0xFFFFFC00;
                QSPIC->QSPIC_CTR_EADDR_REG = eaddr & 0xFFFFFC00;
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Enable the AES-CTR decryption
 *
 * The QSPI controller decrypts Flash contents on-the-fly using AES-CTR. This function
 * is used to enable this feature.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_enable_decryption(HW_QSPIC_ID id)
{
        if (id == HW_QSPIC) {
                REG_SET_BIT(QSPIC, QSPIC_CTR_CTRL_REG, QSPIC_CTR_EN);
                __ISB();
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Disable the AES-CTR decryption
 *
 * The QSPI controller decrypts Flash contents on-the-fly using AES-CTR. This function
 * is used to disable this feature.
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_disable_decryption(HW_QSPIC_ID id)
{
       if (id == HW_QSPIC) {
                REG_CLR_BIT(QSPIC, QSPIC_CTR_CTRL_REG, QSPIC_CTR_EN);
                __ISB();
       } else {
                ASSERT_WARNING(0);
       }
}


/**
 * \brief Controls translation of burst accesses from the AMBA bus to the QSPI2 bus
 *
 * \param [in] id QSPI controller id
 * \param [in] enable
 *        false - The controller translates a burst access on the AMBA bus to a burst
 *                access on the QSPI bus. This results in a minimum command/address
 *                phases, but the QSPI_CS is low for as long as the access occurs,
 *        true  - The controller will split a burst access on the AMBA bus into single
 *                accesses on the QSPI bus. This results in a separate read command to
 *                the FLASH memory for each data required. A 4-beat word incremental
 *                AMBA access will be split into 4 different sequences of reading
 *                (command/address/extra clock/read data). QSPI_CS will be high only
 *                while a QSPI access occurs. This results in lower power dissipation
 *                compared to when QSPIC_FORCENSEQ_EN=0 at the cost of performance
 *
 */
__STATIC_FORCEINLINE void hw_qspi_force_nseq(HW_QSPIC_ID id, bool enable)
{
#if dg_configUSE_HW_QSPI2
        if (id == HW_QSPIC2) {
                REG_SETF(QSPIC2, QSPIC2_CTRLMODE_REG, QSPIC_FORCENSEQ_EN, enable);
        } else {
                ASSERT_WARNING(0);
        }
#endif
}

#if dg_configUSE_HW_QSPI2
/*
 * QSPIC2 FUNCTIONS
 *****************************************************************************************
 */

/**
 * \brief Set the CS mode (QSPIC_CS_MD)
 *
 * \param [in] id QSPI controller id
 * \param [in] edge HW_QSPI_CS_MODE_FALLING The QSPI_CS is produced with the falling edge of the QSPI_SCK
 *                  HW_QSPI_CS_MODE_RISING The QSPI_CS is produced with the rising edge of the QSPI_SCK.
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_cs_mode(HW_QSPIC_ID id, HW_QSPI_CS_MODE edge)
{
        if (id == HW_QSPIC2) {
                REG_SETF(QSPIC2, QSPIC2_CTRLMODE_REG, QSPIC_CS_MD, edge);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Enable SRAM mode (QSPIC2)
 *
 * \param [in] id QSPI controller id
 * \param [in] enable true Enable SRAM mode (SRAM memory is connected)
 *                    false Disable SRAM mode (FLASH memory is connected)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_sram_mode(HW_QSPIC_ID id, bool enable)
{
        if (id == HW_QSPIC2) {
                REG_SETF(QSPIC2, QSPIC2_CTRLMODE_REG, QSPIC_SRAM_EN, enable);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Enable the control of the maximum time tCEM (QSPIC2)
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_enable_tCEM(HW_QSPIC_ID id)
{
        if (id == HW_QSPIC2) {
                REG_SET_BIT(QSPIC2, QSPIC2_MEMBLEN_REG, QSPIC_T_CEM_EN);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Set the maximum time tCEM (QSPIC2)
 *
 * \param [in] id QSPI controller id
 * \param [in] value The maximum allowed time tCEM for which QSPIC_CS can stay active (in us)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_tCEM(HW_QSPIC_ID id, uint16_t value)
{
        if (id == HW_QSPIC2) {
                REG_SETF(QSPIC2, QSPIC2_MEMBLEN_REG, QSPIC_T_CEM_CC, value);
        } else {
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Set the length of a burst operation (QSPIC2)
 *
 * \param [in] id QSPI controller id
 * \param [in] value The length of the wrapping burst that the external memory device
 *                   is capable to implement.
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_burst_length(HW_QSPIC_ID id, HW_QSPI2_MEMBLEN value)
{
        if (id == HW_QSPIC2) {
                REG_SETF(QSPIC2, QSPIC2_MEMBLEN_REG, QSPIC_MEMBLEN, value);
        } else {
                ASSERT_WARNING(0);
        }
}
#endif

/**
 * \brief Set QSPIC instructions
 *
 * \param [in] id QSPI controller id
 * \param [in] qspic_set QSPIC instructions configuration
 *
 */
__RETAINED_CODE void hw_qspi_set_instructions(HW_QSPIC_ID id, struct qspic_instructions *qspic_set);
#endif /* dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2 */


#endif /* HW_QSPI_H_ */

/**
 * \}
 * \}
 */
