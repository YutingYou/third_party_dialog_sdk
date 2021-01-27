/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_SMOTOR SMotor Controller
 * \{
 * \brief SMotor Controller (SMC)
 */

/**
 *****************************************************************************************
 *
 * @file hw_smotor.h
 *
 * @brief Definition of SMotor Controller Low Level Driver API
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_SMOTOR_H_
#define HW_SMOTOR_H_


#if dg_configUSE_HW_SMOTOR

#include "sdk_defs.h"

/*
 *  GLOBAL/CONSTANT VARIABLES DEFINITIONS
 *****************************************************************************************
 */

#define SMOTOR_IRQ_THRESHOLD_DEFAULT 0x08
#define SMOTOR_MOI_DEFAULT 0x100

#define MAX_N_CMDs        31
#define MAX_W_PTR         63
#define MAX_PUN           31
#define MAX_TOD           31
#define WAVETABLE_LEN     63
#define MAX_MOI           0x3FF
#define MAX_CYCLIC_SIZE   0x40
#define MAX_IRQ_THRESHOLD 0x1F
#define PGs_NUM           5

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \enum HW_SMOTOR_PG_IDX
 * \brief SMOTOR PG indices
 *
 */
typedef enum {
        PG0_IDX = 0,    /**< PG number 0 */
        PG1_IDX = 1,    /**< PG number 1 */
        PG2_IDX = 2,    /**< PG number 2 */
        PG3_IDX = 3,    /**< PG number 3 */
        PG4_IDX = 4     /**< PG number 4 */
} HW_SMOTOR_PG_IDX;

/**
 * \enum HW_SMOTOR_OPERATION_MODE
 * \brief SMOTOR FIFO operation mode
 *
 */
typedef enum {
        HW_SMOTOR_NORMAL_FIFO_MODE = 0,         /**< FIFO in run-through mode */
        HW_SMOTOR_CYCLIC_FIFO_MODE = 1          /**< FIFO in cyclic mode */
} HW_SMOTOR_OPERATION_MODE;

/**
 * \enum HW_SMOTOR_PG_START_MODE
 * \brief SMOTOR PG start mode
 *
 */
typedef enum {
        HW_SMOTOR_AUTO_START_MODE = 0,          /**< PG in auto start mode */
        HW_SMOTOR_MANUAL_START_MODE = REG_MSK(SMOTOR, PG0_CTRL_REG, PG_START_MODE)    /**< PG in manual start mode */
} HW_SMOTOR_PG_START_MODE;

/**
 * \enum HW_SMOTOR_PG_MODE
 * \brief SMOTOR PG signals generation mode
 *
 */
typedef enum {
        HW_SMOTOR_PG_FLEX_MODE = 0,             /**< PG signals generation in flex(single) mode */
        HW_SMOTOR_PG_PAIR_MODE = REG_MSK(SMOTOR, PG0_CTRL_REG, PG_MODE)         /**< PG signals generation in pair(mirror) mode */
} HW_SMOTOR_PG_MODE;

/**
 * \enum HW_SMOTOR_SIGNAL
 * \brief SMOTOR PG signals enable/disable
 *
 */
typedef enum {
        HW_SMOTOR_SIGNAL_0_DISABLE = 0,         /**< Disable signal 0 */
        HW_SMOTOR_SIGNAL_1_DISABLE = 0,         /**< Disable signal 1 */
        HW_SMOTOR_SIGNAL_2_DISABLE = 0,         /**< Disable signal 2 */
        HW_SMOTOR_SIGNAL_3_DISABLE = 0,         /**< Disable signal 3 */
        HW_SMOTOR_SIGNAL_0_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, SIG0_EN),      /**< Enable signal 0 */
        HW_SMOTOR_SIGNAL_1_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, SIG1_EN),      /**< Enable signal 1 */
        HW_SMOTOR_SIGNAL_2_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, SIG2_EN),      /**< Enable signal 2 */
        HW_SMOTOR_SIGNAL_3_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, SIG3_EN)       /**< Enable signal 3 */
} HW_SMOTOR_SIGNAL;

/**
 * \enum HW_SMOTOR_SIGNAL_OUTPUT
 * \brief SMOTOR PG signals output mapping/routing
 *
 */
typedef enum {
        HW_SMOTOR_OUTPUT_SIGNAL_0 = 0x0,        /**< Signal mapped to output 0 */
        HW_SMOTOR_OUTPUT_SIGNAL_1 = 0x1,        /**< Signal mapped to output 1 */
        HW_SMOTOR_OUTPUT_SIGNAL_2 = 0x2,        /**< Signal mapped to output 2 */
        HW_SMOTOR_OUTPUT_SIGNAL_3 = 0x3         /**< Signal mapped to output 3 */
} HW_SMOTOR_SIGNAL_OUTPUT;

/**
 * \enum HW_SMOTOR_INTERRUPT
 * \brief SMOTOR interrupt enable/disable
 *
 */
typedef enum {
        HW_SMOTOR_GENSTART_IRQ_DISABLE = 0x00,     /**< Generation start interrupt is disabled */
        HW_SMOTOR_GENEND_IRQ_DISABLE = 0x00,       /**< Generation end interrupt is disabled */
        HW_SMOTOR_FIFO_OVF_IRQ_DISABLE = 0x00,     /**< FIFO overflow interrupt is disabled */
        HW_SMOTOR_FIFO_UNR_IRQ_DISABLE = 0x00,     /**< FIFO underrun interrupt is disabled */
        HW_SMOTOR_THRESHOLD_IRQ_DISABLE = 0x00,    /**< FIFO threshold interrupt is disabled */
        HW_SMOTOR_GENSTART_IRQ_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENSTART_IRQ_EN),   /**< Generation start interrupt is enabled */
        HW_SMOTOR_GENEND_IRQ_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENEND_IRQ_EN),     /**< Generation end interrupt is enabled */
        HW_SMOTOR_FIFO_OVF_IRQ_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_OVF_IRQ_EN),   /**< FIFO overflow interrupt is enabled */
        HW_SMOTOR_FIFO_UNR_IRQ_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_UNR_IRQ_EN),  /**< FIFO underrun interrupt is enabled */
        HW_SMOTOR_THRESHOLD_IRQ_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD_IRQ_EN) /**< FIFO threshold interrupt is enabled */
} HW_SMOTOR_INTERRUPT;

/**
 * \enum HW_SMOTOR_PG_INTERRUPT
 * \brief PG genstart and genend interrupt enable/disable
 *
 */
typedef enum {
        HW_SMOTOR_PG_GENSTART_IRQ_DISABLE = 0x00,         /**< Generation start interrupt is disabled */
        HW_SMOTOR_PG_GENEND_IRQ_DISABLE = 0x00,           /**< Generation end interrupt is disabled */
        HW_SMOTOR_PG_GENSTART_IRQ_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, GENSTART_IRQ_EN),        /**< Generation start interrupt is enabled */
        HW_SMOTOR_PG_GENEND_IRQ_ENABLE = REG_MSK(SMOTOR, PG0_CTRL_REG, GENEND_IRQ_EN)           /**< Generation end interrupt is enabled */
} HW_SMOTOR_PG_INTERRUPT;

/**
 * \enum HW_SMOTOR_TRIGGER
 * \brief SMOTOR trigger enable/disable
 *
 */
typedef enum {
        HW_SMOTOR_MC_LP_CLK_TRIG_DISABLE = 0x00,        /**< LowPower clock trigger is disabled */
        HW_SMOTOR_TRIG_RTC_EVENT_DISABLE = 0x00,        /**< RTC event trigger is disabled */
        HW_SMOTOR_MC_LP_CLK_TRIG_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, MC_LP_CLK_TRIG_EN),    /**< LowPower clock trigger enabled */
        HW_SMOTOR_TRIG_RTC_EVENT_ENABLE = REG_MSK(SMOTOR, SMOTOR_CTRL_REG, TRIG_RTC_EVENT_EN)     /**< RTC event trigger is enabled */
} HW_SMOTOR_TRIGGER;

/**
 * \enum HW_SMOTOR_COMMAND_POP
 * \brief SMOTOR pop command value
 *
 */
typedef enum {
        HW_SMOTOR_POP_COMMAND_FALSE = 0,                /**<  Pop command idle */
        HW_SMOTOR_POP_COMMAND_TRUE = 1                  /**<  Pop command trigger*/
} HW_SMOTOR_COMMAND_POP;

/**
 * \enum HW_SMOTOR_INTERRUPT_HANDLE_ID
 * \brief SMOTOR interrupt type handle ID
 *
 */
typedef enum {
        HW_SMOTOR_GENSTART_IRQ_HANDLE_ID = 0,           /**< Generation start interrupt handle id */
        HW_SMOTOR_GENEND_IRQ_HANDLE_ID = 1,             /**< Generation end interrupt handle id */
        HW_SMOTOR_FIFO_OVF_IRQ_HANDLE_ID = 2,           /**< FIFO overflow interrupt handle id */
        HW_SMOTOR_FIFO_UNR_IRQ_HANDLE_ID = 3,           /**< FIFO underrun interrupt handle id */
        HW_SMOTOR_THRESHOLD_IRQ_HANDLE_ID = 4,          /**< FIFO threshold interrupt handle id */
        HW_SMOTOR_INVALID_IRQ_HANDLE_ID = 0xFF          /**< Invalid interrupt handle id */
} HW_SMOTOR_INTERRUPT_HANDLE_ID;

/**
 * \enum HW_SMOTOR_INTERRUPT_STATUS
 * \brief SMOTOR interrupt status mask
 *
 */
typedef enum {
        HW_SMOTOR_GENSTART_IRQ_FIRED = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, GENSTART_IRQ_STATUS),            /**< Generation start interrupt status mask */
        HW_SMOTOR_GENEND_IRQ_FIRED = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, GENEND_IRQ_STATUS),                /**< Generation end interrupt status mask */
        HW_SMOTOR_FIFO_OVF_IRQ_FIRED = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, FIFO_OVF_IRQ_STATUS),            /**< FIFO overflow interrupt status mask */
        HW_SMOTOR_FIFO_UNR_IRQ_FIRED = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, FIFO_UNR_IRQ_STATUS),            /**< FIFO underrun interrupt status mask */
        HW_SMOTOR_THRESHOLD_IRQ_FIRED = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, THRESHOLD_IRQ_STATUS)           /**< FIFO threshold interrupt status mask */
} HW_SMOTOR_INTERRUPT_STATUS;

/**
 * \enum HW_SMOTOR_PG_BUSY_STATUS
 * \brief SMOTOR PG Busy Status mask
 *
 */
typedef enum {
        HW_SMOTOR_PG0_BUSY = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, PG0_BUSY),              /**< PG_0 busy status mask */
        HW_SMOTOR_PG1_BUSY = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, PG1_BUSY),              /**< PG_1 busy status mask */
        HW_SMOTOR_PG2_BUSY = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, PG2_BUSY),              /**< PG_2 busy status mask */
        HW_SMOTOR_PG3_BUSY = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, PG3_BUSY),              /**< PG_3 busy status mask */
        HW_SMOTOR_PG4_BUSY = REG_MSK(SMOTOR, SMOTOR_STATUS_REG, PG4_BUSY)               /**< PG_4 busy status mask */
} HW_SMOTOR_PG_BUSY_STATUS;

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief SMotor interrupt callback type definition
 *
 */
typedef void (*hw_smotor_interrupt_cb)(uint8_t event);

/**
 * \brief Pattern Generator configuration struct
 *
 */
typedef struct {
        HW_SMOTOR_PG_INTERRUPT genstart_irq_en;         /**< Generation start interrupt enable */
        HW_SMOTOR_PG_INTERRUPT genend_irq_en;           /**< Generation end interrupt enable */
        HW_SMOTOR_PG_START_MODE pg_start_mode;          /**< Pattern Generator start mode*/
        HW_SMOTOR_PG_MODE pg_mode;                      /**< Pattern Generator operation mode */
        HW_SMOTOR_SIGNAL sig0_en;                       /**< Pattern Generator signal 0 enable */
        HW_SMOTOR_SIGNAL sig1_en;                       /**< Pattern Generator signal 1 enable */
        HW_SMOTOR_SIGNAL sig2_en;                       /**< Pattern Generator signal 2 enable */
        HW_SMOTOR_SIGNAL sig3_en;                       /**< Pattern Generator signal 3 enable */
        HW_SMOTOR_SIGNAL_OUTPUT out0_sig;               /**< Pattern Generator signal on output 0 */
        HW_SMOTOR_SIGNAL_OUTPUT out1_sig;               /**< Pattern Generator signal on output 1 */
        HW_SMOTOR_SIGNAL_OUTPUT out2_sig;               /**< Pattern Generator signal on output 2 */
        HW_SMOTOR_SIGNAL_OUTPUT out3_sig;               /**< Pattern Generator signal on output 3 */
} hw_smotor_pg_cfg_t;

/**
 *  \brief SMotor struct for interrupt callbacks
 *
 */
typedef struct {
        hw_smotor_interrupt_cb genstart_cb;             /**< Generation start interrupt callback */
        hw_smotor_interrupt_cb genend_cb;               /**< Generation end interrupt callback */
        hw_smotor_interrupt_cb fifo_ovf_cb;             /**< FIFO overflow interrupt callback */
        hw_smotor_interrupt_cb fifo_unr_cb;             /**< FIFO underrun interrupt callback */
        hw_smotor_interrupt_cb fifo_threshold_cb;       /**< FIFO threshold interrupt callback */
        uint8_t smotor_current_wave_idx;                /**< Current index of wave table */
} hw_smotor_callback_t;

/**
 *  \brief SMotor configuration struct.
 *
 *  Set Operation mode, Enable Triggers/Interrupts and sets operating parameters
 *
 */
typedef struct {
        HW_SMOTOR_OPERATION_MODE operation_mode;        /**< SMotor operation mode */
        uint8_t cyclic_size;                            /**< FIFO cyclic mode size */
        uint8_t moi;                                    /**< Idle time interval after a wave ends */
        HW_SMOTOR_INTERRUPT genstart_irq_en;            /**< Generation start interrupt enable */
        HW_SMOTOR_INTERRUPT genend_irq_en;              /**< Generation end interrupt enable */
        HW_SMOTOR_INTERRUPT fifo_overflow_irq_en;       /**< FIFO overflow interrupt enable */
        HW_SMOTOR_INTERRUPT fifo_underrun_irq_en;       /**< FIFO underrun interrupt enable */
        HW_SMOTOR_INTERRUPT threshold_irq_en;           /**< FIFO threshold interrupt enable */
        uint8_t threshold;                              /**< FIFO threshold level */
        HW_SMOTOR_TRIGGER sleep_clk_trigger_en;         /**< SMotor LowPower clock trigger */
        HW_SMOTOR_TRIGGER rtc_trigger_en;               /**< SMotor RTC trigger */
        hw_smotor_pg_cfg_t *pg_cfg[PGs_NUM];            /**< PG configuration pointers */
} hw_smotor_cfg_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Push a new command in FIFO
 *
 *\param [in] PG_ID      Defines which PG the current command is for
 *\param [in] PG_SIG     Defines which signal (out of the 4) is the command addressing.
 *\param [in] N_CMDs     Defines the additional commands that should be pushed to PG along with
 *                       the current one
 *\param [in] W_PTR      A pointer to the waves Table, defining the wave the PG will start generate.
 *
 */
__STATIC_INLINE void hw_smotor_push_command(uint8_t PG_ID, uint8_t PG_SIG, uint8_t N_CMDs,
        uint8_t W_PTR)
{
        uint16_t newCommand;
        ASSERT_WARNING(N_CMDs < MAX_N_CMDs);
        ASSERT_WARNING(W_PTR < MAX_W_PTR);

        newCommand = ((PG_ID & 0x07) << 13) | ((PG_SIG & 0x3) << 11) | ((N_CMDs & 0x1F) << 6)
                | (W_PTR & 0x3F);
        REG_SETF(SMOTOR, SMOTOR_CMD_FIFO_REG, SMOTOR_CMD_FIFO, newCommand);
}
/**
 * \brief Set command FIFO Operation Mode. There are two options : Normal and Cyclic.
 *
 * \param [in] mode       HW_SMOTOR_NORMAL_FIFO_MODE for Normal, HW_SMOTOR_CYCLIC_FIFO_MODE for Cyclic mode.
 *
 * \note When switching from Cyclic to Normal (and vice versa), Read/Write pointers are reset.
 *
 */
__STATIC_INLINE void hw_smotor_set_command_fifo_mode(HW_SMOTOR_OPERATION_MODE mode)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, CYCLIC_MODE, mode);
}
/**
 * \brief Get command FIFO Operation Mode.
 *
 * \return HW_SMOTOR_OPERATION_MODE     HW_SMOTOR_NORMAL_FIFO_MODE for Normal, HW_SMOTOR_CYCLIC_FIFO_MODE for Cyclic mode
 *
 */
__STATIC_INLINE HW_SMOTOR_OPERATION_MODE hw_smotor_get_command_fifo_mode(void)
{
        return REG_GETF(SMOTOR, SMOTOR_CTRL_REG, CYCLIC_MODE);
}
/**
 * \brief Reset Command FIFO Read/Write pointers (Both return to zero values).
 *
 * \note Reads the current FIFO operation mode and performs a toggle operation to reset pointers,
 * then returns to previous FIFO operation mode.
 *
 */
void hw_smotor_fifo_pointers_reset(void);

/**
 * \brief Get depth of Cyclic FIFO (valid only when in Cyclic Mode)
 *
 * \return uint8_t       cyclic size of FIFO
 *
 */
__STATIC_INLINE uint8_t hw_smotor_get_cyclic_size(void)
{
        return REG_GETF(SMOTOR, SMOTOR_CTRL_REG, CYCLIC_SIZE);
}
/**
 * \brief Set depth of Cyclic FIFO (valid only when in Cyclic Mode)
 *
 * \param [in] cyclic_size       Depth of the cyclic buffer (6 bits)
 *
 */
__STATIC_INLINE void hw_smotor_set_cyclic_size(uint8_t cyclic_size)
{
        ASSERT_WARNING(cyclic_size < MAX_CYCLIC_SIZE);
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, CYCLIC_SIZE, cyclic_size);
}
/**
 * \brief This function will pop a command from the command FIFO to the pattern generators.
 *
 */
__STATIC_INLINE void hw_smotor_trigger(void)
{
        REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, POP_CMD, HW_SMOTOR_POP_COMMAND_TRUE);
}
/**
 * \brief Get value of FIFO read pointer
 *
 */
__STATIC_INLINE uint8_t hw_smotor_get_read_pointer(void)
{
        return REG_GETF(SMOTOR, SMOTOR_CMD_READ_PTR_REG, SMOTOR_CMD_READ_PTR);
}

/**
 * \brief Get value of FIFO write pointer
 *
 */
__STATIC_INLINE uint8_t hw_smotor_get_write_pointer(void)
{
        return REG_GETF(SMOTOR, SMOTOR_CMD_WRITE_PTR_REG, SMOTOR_CMD_WRITE_PTR);
}

/**
 * \brief Set value of FIFO write pointer
 *
 * \param [in] write_pointer_value      The write pointer to set
 *
 */
__STATIC_INLINE void hw_smotor_set_write_pointer(uint8_t write_pointer_value)
{
        REG_SETF(SMOTOR, SMOTOR_CMD_WRITE_PTR_REG, SMOTOR_CMD_WRITE_PTR, write_pointer_value);
}

/**
 * \brief Get command placed on a specific FIFO position
 *
 * \param [in] index    FIFO position to read the command
 *
 * \return uint16_t     Command in index position
 *
 */
__STATIC_INLINE uint16_t hw_smotor_get_fifo_command(uint8_t index)
{
        ASSERT_WARNING(index <= MAX_N_CMDs);
        return *(((uint16_t *)&SMOTOR->CMD_TABLE_BASE) + index);
}

/**
 * \brief Set the Idle time of (every) PG after generating a waveform.
 *
 * \param [in] moi_value        MOI (MOtor Idle) time indicates the amount of slots where nothing happens
 *
 */
__STATIC_INLINE void hw_smotor_set_moi(uint16_t moi_value)
{
        ASSERT_WARNING(moi_value < MAX_MOI);
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_MOI, moi_value);
}

/**
 * \brief Read MOI (MOtor Idle) time interval
 *
 * \return uint16_t     MOI value
 */
__STATIC_INLINE uint16_t hw_smotor_get_moi(void)
{
        return REG_GETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_MOI);
}

/**
 * \brief Enable Pattern Generation Start Interrupt requests
 *
 */
__STATIC_INLINE void hw_smotor_genstart_irq_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENSTART_IRQ_EN, 1);
}

/**
 * \brief Disable Pattern Generation Start Interrupt requests
 *
 */
__STATIC_INLINE void hw_smotor_genstart_irq_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENSTART_IRQ_EN, 0);
}

/**
 * \brief Enable Pattern Generation End Interrupt requests
 *
 */
__STATIC_INLINE void hw_smotor_genend_irq_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENEND_IRQ_EN, 1);
}

/**
 * \brief Disable Pattern Generation End Interrupt requests
 *
 */
__STATIC_INLINE void hw_smotor_genend_irq_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENEND_IRQ_EN, 0);
}

/**
 * \brief Enable FIFO Underrun Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_fifo_unr_irq_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_UNR_IRQ_EN, 1);
}

/**
 * \brief Disable FIFO Underrun Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_fifo_unr_irq_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_UNR_IRQ_EN, 0);
}

/**
 * \brief Enable FIFO Overflow Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_fifo_ovf_irq_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_OVF_IRQ_EN, 1);
}

/**
 * \brief Disable FIFO Overflow Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_fifo_ovf_irq_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_OVF_IRQ_EN, 0);
}

/**
 * \brief Enable FIFO Threshold Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_threshold_irq_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD_IRQ_EN, 1);
}

/**
 * \brief Disable FIFO Threshold Interrupt
 *
 */
__STATIC_INLINE void hw_smotor_threshold_irq_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD_IRQ_EN, 0);
}

/**
 * \brief Determines the amount of commands reside in FIFO, below which the motor controller will
 *  generate an interrupt request
 *
 * \param [in] IRQ_THRESHOLD    When Read pointer - Write pointer < IRQ_THRESHOLD, an interrupt is
 *                              generated
 *
 */
__STATIC_INLINE void hw_smotor_set_irq_threshold(uint8_t IRQ_THRESHOLD)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD, IRQ_THRESHOLD);
}

/**
 * \brief Enable RTC Event trigger
 *
 */
__STATIC_INLINE void hw_smotor_rtc_trig_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, TRIG_RTC_EVENT_EN, 1);
}
/**
 * \brief Disable RTC Event trigger
 *
 */
__STATIC_INLINE void hw_smotor_rtc_trig_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, TRIG_RTC_EVENT_EN, 0);
}

/**
 * \brief Enable Divided LP Clock trigger (divider is configurable)
 *
 */
__STATIC_INLINE void hw_smotor_lp_clk_trig_enable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, MC_LP_CLK_TRIG_EN, 1);
}

/**
 * \brief Disable Divided LP Clock trigger (divider is configurable)
 *
 */
__STATIC_INLINE void hw_smotor_lp_clk_trig_disable(void)
{
        REG_SETF(SMOTOR, SMOTOR_CTRL_REG, MC_LP_CLK_TRIG_EN, 0);
}
/**
 * \brief Enable IRQ from PGx GENSTART
 *
 * \param [in] pg_idx         id of the PG to enable generation start interrupt
 *
 */
__STATIC_INLINE void hw_smotor_pg_genstart_irq_enable(HW_SMOTOR_PG_IDX pg_idx)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, GENSTART_IRQ_EN, *hw_smotor_pg_reg, 1);
}
/**
 * \brief Disable IRQ from PGx GENSTART
 *
 * \param [in] pg_idx         id of the PG to disable generation start interrupt
 *
 */
__STATIC_INLINE void hw_smotor_pg_genstart_irq_disable(HW_SMOTOR_PG_IDX pg_idx)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, GENSTART_IRQ_EN, *hw_smotor_pg_reg, 0);
}

/**
 * \brief Enable IRQ from PGx GENEND
 *
 * \param [in] pg_idx         id of the PG to enable generation end interrupt
 *
 */
__STATIC_INLINE void hw_smotor_pg_genend_irq_enable(HW_SMOTOR_PG_IDX pg_idx)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, GENEND_IRQ_EN, *hw_smotor_pg_reg, 1);
}
/**
 * \brief Disable IRQ from PGx GENEND
 *
 * \param [in] pg_idx         id of the PG to enable generation end interrupt
 *
 */
__STATIC_INLINE void hw_smotor_pg_genend_irq_disable(HW_SMOTOR_PG_IDX pg_idx)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, GENEND_IRQ_EN, *hw_smotor_pg_reg, 0);
}

/**
 * \brief Enable all Interrupt sources for SMotor Controller
 *
 */
__STATIC_INLINE void hw_smotor_global_irq_enable(void)
{
        uint32_t smotor_ctrl_reg_val = SMOTOR->SMOTOR_CTRL_REG;
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_UNR_IRQ_EN, smotor_ctrl_reg_val, 1);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_OVF_IRQ_EN, smotor_ctrl_reg_val, 1);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD_IRQ_EN, smotor_ctrl_reg_val, 1);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENSTART_IRQ_EN, smotor_ctrl_reg_val, 1);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENEND_IRQ_EN, smotor_ctrl_reg_val, 1);
        SMOTOR->SMOTOR_CTRL_REG = smotor_ctrl_reg_val;
}
/**
 * \brief Disable all Interrupt sources for SMotor Controller
 *
 */
__STATIC_INLINE void hw_smotor_global_irq_disable(void)
{
        uint32_t smotor_ctrl_reg_val = SMOTOR->SMOTOR_CTRL_REG;
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, TRIG_RTC_EVENT_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, MC_LP_CLK_TRIG_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_UNR_IRQ_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_FIFO_OVF_IRQ_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_THRESHOLD_IRQ_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENSTART_IRQ_EN, smotor_ctrl_reg_val, 0);
        REG_SET_FIELD(SMOTOR, SMOTOR_CTRL_REG, SMOTOR_GENEND_IRQ_EN, smotor_ctrl_reg_val, 0);
        SMOTOR->SMOTOR_CTRL_REG = smotor_ctrl_reg_val;
}

/**
 * \brief Read SMotor Status Register containing IRQ Status of all possible interrupt sources
 *
 * \return SMotor Status Register value
 *
 */
__STATIC_INLINE uint32_t hw_smotor_get_status_reg(void)
{
        return SMOTOR->SMOTOR_STATUS_REG;
}

/**
 * \brief Read GENSTART Interrupt Status from SMOTOR_STATUS_REG
 *
 * \return true if triggered, else false
 *
 */
__STATIC_INLINE bool hw_smotor_get_genstart_irq_status(void)
{
        return REG_GETF(SMOTOR, SMOTOR_STATUS_REG, GENSTART_IRQ_STATUS);
}

/**
 * \brief Read GENEND Interrupt Status from SMOTOR_STATUS_REG
 *
 * \return true if triggered, else false
 *
 */
__STATIC_INLINE bool hw_smotor_get_genend_irq_status(void)
{
        return REG_GETF(SMOTOR, SMOTOR_STATUS_REG, GENSTART_IRQ_STATUS);
}

/**
 * \brief Read FIFO Overflow Interrupt Status from SMOTOR_STATUS_REG
 *
 * \return true if triggered, else false
 *
 */
__STATIC_INLINE bool hw_smotor_get_fifo_ovf_irq_status(void)
{
        return REG_GETF(SMOTOR, SMOTOR_STATUS_REG, FIFO_OVF_IRQ_STATUS);
}

/**
 * \brief Read FIFO Underrun Interrupt Status from SMOTOR_STATUS_REG
 *
 * \return true if triggered, else false
 *
 */
__STATIC_INLINE bool hw_smotor_get_fifo_unr_irq_status(void)
{
        return REG_GETF(SMOTOR, SMOTOR_STATUS_REG, FIFO_UNR_IRQ_STATUS);
}

/**
 * \brief Read FIFO Threshold Interrupt Status from SMOTOR_STATUS_REG
 *
 * \return true if triggered, else false
 *
 */
__STATIC_INLINE bool hw_smotor_get_threshold_irq_status(void)
{
        return REG_GETF(SMOTOR, SMOTOR_STATUS_REG, THRESHOLD_IRQ_STATUS);
}

/**
 * \brief Clear genstart interrupt status on SMOTOR_IRQ_CLEAR_REG
 *
 */
__STATIC_INLINE void hw_smotor_clear_genstart_irq_status(void)
{
        REG_SETF(SMOTOR, SMOTOR_IRQ_CLEAR_REG, GENSTART_IRQ_CLEAR, 1);
}

/**
 * \brief Clear genend interrupt status on SMOTOR_IRQ_CLEAR_REG
 *
 */
__STATIC_INLINE void hw_smotor_clear_genend_irq_status(void)
{
        REG_SETF(SMOTOR, SMOTOR_IRQ_CLEAR_REG, GENEND_IRQ_CLEAR, 1);
}

/**
 * \brief Clear overflow interrupt status on SMOTOR_IRQ_CLEAR_REG
 *
 */
__STATIC_INLINE void hw_smotor_clear_fifo_ovf_irq_status(void)
{
        REG_SETF(SMOTOR, SMOTOR_IRQ_CLEAR_REG, FIFO_OVF_IRQ_CLEAR, 1);
}

/**
 * \brief Clear underrun interrupt status on SMOTOR_IRQ_CLEAR_REG
 *
 */
__STATIC_INLINE void hw_smotor_clear_fifo_urn_irq_status(void)
{
        REG_SETF(SMOTOR, SMOTOR_IRQ_CLEAR_REG, FIFO_UNR_IRQ_CLEAR, 1);
}

/**
 * \brief Clear threshold interrupt status on SMOTOR_IRQ_CLEAR_REG
 *
 */
__STATIC_INLINE void hw_smotor_clear_threshold_irq_status(void)
{
        REG_SETF(SMOTOR, SMOTOR_IRQ_CLEAR_REG, THRESHOLD_IRQ_CLEAR, 1);
}

/**
 * \brief Set PGx signals generation mode. Signals can be generated in single or pair mode.
 *
 * \param [in]  pg_idx          index of PG to set signals generation mode
 * \param [in]  pg_mode         Mode for signals : 0 for Flex (single), 1 for Pair (mirror).
 *
 */
__STATIC_INLINE void hw_smotor_set_pg_operation_mode(HW_SMOTOR_PG_IDX pg_idx, bool pg_mode)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);
        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, PG_MODE, *hw_smotor_pg_reg, pg_mode);
}

/**
 * \brief  Set Pattern Generator start mode
 *
 * \param [in]  pg_idx          index of PG to set start mode
 * \param [in]  mode            0 - Auto, 1 - Manual
 *
 */
__STATIC_INLINE void hw_smotor_set_pg_start_mode(HW_SMOTOR_PG_IDX pg_idx, bool mode)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);
        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, PG_START_MODE, *hw_smotor_pg_reg, mode);
}

/**
 * \brief Enable Pattern Generator signals
 *
 * \param [in] pg_idx           index of PG to enable signals
 * \param [in] Sig0_En          1 for enable, 0 for disable signal 0
 * \param [in] Sig1_En          1 for enable, 0 for disable signal 1
 * \param [in] Sig2_En          1 for enable, 0 for disable signal 2
 * \param [in] Sig3_En          1 for enable, 0 for disable signal 3
 */
__STATIC_INLINE void hw_smotor_pg_signals_enable(HW_SMOTOR_PG_IDX pg_idx, bool Sig0_En,
        bool Sig1_En, bool Sig2_En, bool Sig3_En)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, SIG0_EN, *hw_smotor_pg_reg, Sig0_En);
        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, SIG1_EN, *hw_smotor_pg_reg, Sig1_En);
        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, SIG2_EN, *hw_smotor_pg_reg, Sig2_En);
        REG_SET_FIELD(SMOTOR, PG0_CTRL_REG, SIG3_EN, *hw_smotor_pg_reg, Sig3_En);
}

/**
 * \brief Selects which signal of the PGx is routed to each output.
 *
 * \param [in] pg_idx           index of PG to route signals to outputs
 * \param [in] Out0_Sig         PGx signal mapped to Output 0
 * \param [in] Out1_Sig         PGx signal mapped to Output 1
 * \param [in] Out2_Sig         PGx signal mapped to Output 2
 * \param [in] Out3_Sig         PGx signal mapped to Output 3
 */
__STATIC_INLINE void hw_smotor_pg_signal_outputs(HW_SMOTOR_PG_IDX pg_idx, uint8_t Out0_Sig,
        uint8_t Out1_Sig, uint8_t Out2_Sig, uint8_t Out3_Sig)
{
        volatile uint32 *hw_smotor_pg_reg =
                REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);

        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, OUT0_SIG, *hw_smotor_pg_reg, Out0_Sig);
        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, OUT1_SIG, *hw_smotor_pg_reg, Out1_Sig);
        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, OUT2_SIG, *hw_smotor_pg_reg, Out2_Sig);
        REG_SET_FIELD(SMOTOR, PG1_CTRL_REG, OUT3_SIG, *hw_smotor_pg_reg, Out3_Sig);
}

/**
 * \brief Call of this function will start the pattern generator in manual start mode.
 *
 * \param [in] pg_id            index of PG to start manually
 *
 */
__STATIC_INLINE void hw_smotor_pg_start(HW_SMOTOR_PG_IDX pg_id)
{
        switch (pg_id) {
        case PG0_IDX:
                REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, PG0_START, 1);
                break;
        case PG1_IDX:
                REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, PG1_START, 1);
                break;
        case PG2_IDX:
                REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, PG2_START, 1);
                break;
        case PG3_IDX:
                REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, PG3_START, 1);
                break;
        case PG4_IDX:
                REG_SETF(SMOTOR, SMOTOR_TRIGGER_REG, PG4_START, 1);
                break;
        default:
                ASSERT_ERROR(0);
        }
}

/**
 * \brief Get busy_motor signal, indicating that resource is busy.
 *
 * \return 1 if SMotor is busy, else 0
 *
 */
__STATIC_INLINE uint8_t hw_smotor_get_busy_status(void)
{
        return REG_GETF(MEMCTRL, BUSY_STAT_REG, BUSY_MOTOR);
}

/**
 * \brief Get pg_busy signal, indicating that resource is busy.
 *
 * \param [in] pg_id    index of PG
 *
 * \return true if PG with id : pg_id is busy; else false.
 *
 */
bool hw_smotor_get_pg_busy_status(HW_SMOTOR_PG_IDX pg_id);

/**
 * \brief Adds waveform in Wave Table
 *
 * \param [in] *wave  pointer to the waveform array to be added
 *
 * \return index where the current waveform has been stored when wave is added, 0xFF when Wave Table is full
 */
uint8_t hw_smotor_add_wave(const uint8_t *wave);

/**
 * \brief Resets Wave Table index. ("erases" waveforms from Wave Table)
 *
 * \note Necessary when Wave Table needs to be re-configured
 *
 */
void hw_smotor_rst_wave_idx(void);

/**
 * \brief Returns current Wave Table index
 *
 * \return uint8_t the index where any valid wave can be added
 * \note Necessary when Wave Table needs to be re-configured
 *
 */
 uint8_t hw_smotor_get_wave_idx(void);

/**
 * \brief Register interrupt callback functions
 *
 * \param [in] handler          callback function to be registered
 * \param [in] handle_type      type of interrupt for the callback function
 *
 */
void hw_smotor_register_intr(hw_smotor_interrupt_cb handler,
        HW_SMOTOR_INTERRUPT_HANDLE_ID handle_type);

/**
 * \brief Unregister interrupt callback functions
 *
 * \param[in] handle_type       type of interrupt to unregister callback function
 *
 */
void hw_smotor_unregister_intr(HW_SMOTOR_INTERRUPT_HANDLE_ID handle_type);

/**
 * \brief Initializes SMotor Controller
 *
 * \note Configures active PG's, then sets SMotor Controller generic configuration
 *
 * \param [in] cfg      pointer to SMotor Controller configuration struct
 *
 */
void hw_smotor_initialization(const hw_smotor_cfg_t *cfg);

#endif /* dg_configUSE_HW_SMOTOR */


#endif /* HW_SMOTOR_H_ */

/**
 * \}
 * \}
 */
