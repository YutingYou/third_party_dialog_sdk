/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SENSOR_NODE_EMU
 *
 * \brief Sensor Node Controller (SNC) Emulator
 *
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file snc_emu.h
 *
 * @brief Definition of Sensor Node Controller Emulator Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef SENSOR_NODE_EMU_H_
#define SENSOR_NODE_EMU_H_


#if dg_configUSE_HW_SENSOR_NODE & dg_configUSE_HW_SENSOR_NODE_EMU

#include <stdbool.h>
#include <stdint.h>

#include "sdk_defs.h"

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Sensor Node Controller clock frequency division factor
 *
 */
typedef enum {
        HW_SNC_CLK_DIV_1 = 0, /**< always on (XTAL or PLL) */
        HW_SNC_CLK_DIV_2 = 0, /**< always on (XTAL or PLL) */
        HW_SNC_CLK_DIV_4 = 0, /**< always on (XTAL or PLL) */
        HW_SNC_CLK_DIV_8 = 0, /**< always on (XTAL or PLL) */
} HW_SNC_CLK_DIV;

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Sensor Node interrupt callback
 *
 */
typedef void (*hw_snc_interrupt_cb_t)(void);

/**
 * \brief Sensor Node Emulator delay callback
 *
 */
typedef void (*hw_snc_emu_delay_cb_t)(uint8_t);

/**
 * \brief Sensor Node Emulator execution indication callback, returning SNC program counter (PC)
 *        of the SeNIS command to be executed
 *
 */
typedef void (*hw_snc_emu_exec_ind_cb_t)(uint32_t pc);

/**
 * \brief Sensor Node configuration
 *
 * SNC uses the pclk. Therefore, SNC clock frequency depends on hclk, pclk and SNC clock division
 * factors
 *
 */
typedef struct
{
        uint32_t base_address; /**< base address of the SNC ucode */
        HW_SNC_CLK_DIV clk_div; /**< default clock division factor */

        bool sw_ctrl_en; /**< SW-Control mode enabled */
        bool bus_error_detect_en; /**< Bus Error detection enabled */
        bool snc_irq_to_pdc_en; /**< SNC IRQ towards PDC enabled */
        bool snc_irq_to_cm33_en; /**< SNC IRQ towards CM33 enabled */
} hw_snc_config_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//===================== Register Read/Write functions ==========================
/**
 * \brief Write the SNC_EN field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_EN(uint32_t val);

/**
 * \brief Write the SNC_SW_CTRL field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_SW_CTRL(uint32_t val);

/**
 * \brief Write the BUS_ERROR_DETECT_EN field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_BUS_ERROR_DETECT_EN(uint32_t val);

/**
 * \brief Write the SNC_RESET field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_RESET(uint32_t val);

/**
 * \brief Write the SNC_BRANCH_LOOP_INIT field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_BRANCH_LOOP_INIT(uint32_t val);

/**
 * \brief Write the SNC_IRQ_EN field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_EN(uint32_t val);

/**
 * \brief Write the SNC_IRQ_CONFIG field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_CONFIG(uint32_t val);

/**
 * \brief Write the SNC_IRQ_ACK field of the emulated SNC_CTRL_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_ACK(uint32_t val);

/**
 * \brief Write the EQ_FLAG field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_EQ_FLAG(uint32_t val);

/**
 * \brief Write the GR_FLAG field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_GR_FLAG(uint32_t val);

/**
 * \brief Write the SNC_DONE_STATUS field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_SNC_DONE_STATUS(uint32_t val);

/**
 * \brief Write the BUS_ERROR_STATUS field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_BUS_ERROR_STATUS(uint32_t val);

/**
 * \brief Write the HARD_FAULT_STATUS field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_HARD_FAULT_STATUS(uint32_t val);

/**
 * \brief Write the SNC_IS_STOPPED field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_SNC_IS_STOPPED(uint32_t val);

/**
 * \brief Write the SNC_PC_LOADED field of the emulated SNC_STATUS_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_STATUS_REG_setf_SNC_PC_LOADED(uint32_t val);

/**
 * \brief Write the LP_TIMER field of the emulated SNC_LP_TIMER_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_LP_TIMER_REG_setf_LP_TIMER(uint32_t val);

/**
 * \brief Write the PC_REG field of the emulated SNC_PC_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_PC_REG_setf_PC_REG(uint32_t val);

/**
 * \brief Write the R1_REG field of the emulated SNC_R1_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_R1_REG_setf_R1_REG(uint32_t val);

/**
 * \brief Write the R2_REG field of the emulated SNC_R2_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_R2_REG_setf_R2_REG(uint32_t val);

/**
 * \brief Write the TMP1_REG field of the emulated SNC_TMP1_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_TMP1_REG_setf_TMP1_REG(uint32_t val);

/**
 * \brief Write the TMP2_REG field of the emulated SNC_TMP2_REG
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_SETF
 *
 */
void hw_snc_SNC_TMP2_REG_setf_TMP2_REG(uint32_t val);

/**
 * \brief Read the SNC_EN field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_EN(void);

/**
 * \brief Read the SNC_SW_CTRL field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_SW_CTRL(void);

/**
 * \brief Read the BUS_ERROR_DETECT_EN field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_BUS_ERROR_DETECT_EN(void);

/**
 * \brief Read the SNC_RESET field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_RESET(void);

/**
 * \brief Read the SNC_BRANCH_LOOP_INIT field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_BRANCH_LOOP_INIT(void);

/**
 * \brief Read the SNC_IRQ_EN field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_EN(void);

/**
 * \brief Read the SNC_IRQ_CONFIG field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_CONFIG(void);

/**
 * \brief Read the SNC_IRQ_ACK field of the emulated SNC_CTRL_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_ACK(void);

/**
 * \brief Read the EQ_FLAG field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_EQ_FLAG(void);

/**
 * \brief Read the GR_FLAG field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_GR_FLAG(void);

/**
 * \brief Read the SNC_DONE_STATUS field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_DONE_STATUS(void);

/**
 * \brief Read the BUS_ERROR_STATUS field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_BUS_ERROR_STATUS(void);

/**
 * \brief Read the HARD_FAULT_STATUS field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_HARD_FAULT_STATUS(void);

/**
 * \brief Read the SNC_IS_STOPPED field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_IS_STOPPED(void);

/**
 * \brief Read the SNC_PC_LOADED field of the emulated SNC_STATUS_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_PC_LOADED(void);

/**
 * \brief Read the LP_TIMER field of the emulated SNC_LP_TIMER_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_LP_TIMER_REG_getf_LP_TIMER(void);

/**
 * \brief Read the PC_REG field of the emulated SNC_PC_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_PC_REG_getf_PC_REG(void);

/**
 * \brief Read the R1_REG field of the emulated SNC_R1_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_R1_REG_getf_R1_REG(void);

/**
 * \brief Read the R2_REG field of the emulated SNC_R2_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_R2_REG_getf_R2_REG(void);

/**
 * \brief Read the TMP1_REG field of the emulated SNC_TMP1_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_TMP1_REG_getf_TMP1_REG(void);

/**
 * \brief Read the TMP2_REG field of the emulated SNC_TMP2_REG
 * \return uint32_t      the read value
 *
 * \sa HW_SNC_REG_GETF
 *
 */
uint32_t hw_snc_SNC_TMP2_REG_getf_TMP2_REG(void);

/**
 * \brief Write a value to an SNC register field
 *
 * \param [in] reg      the SNC register
 * \param [in] field    the SNC register field
 * \param [in] val      value to be written
 *
 * \sa HW_SNC_REG_GETF
 *
 */
#define HW_SNC_REG_SETF(reg, field, val) \
        hw_snc_##reg##_setf_##field(val)

/**
 * \brief Get the value of an SNC register field
 *
 * \param [in] reg      the SNC register
 * \param [in] field    the SNC register field
 *
 * \sa HW_SNC_REG_SETF
 *
 */
#define HW_SNC_REG_GETF(reg, field) \
        hw_snc_##reg##_getf_##field()

//==================== Initialization function =================================

/**
 * \brief SNC initialization
 *
 * Initializes SNC.
 *
 * \note After initialization, if SW-Control mode is enabled, SNC is disabled.
 *       Enable SNC by calling hw_snc_manual_enable().
 *
 * \param [in] cfg      pointer to SNC configuration struct
 *
 * \sa hw_snc_manual_enable
 *
 */
void hw_snc_init(const hw_snc_config_t *cfg);

//==================== Control functions =======================================

/**
 * \brief Enable manually SNC
 *
 * When SNC is enabled manually, SW-Control mode is enabled for SNC and it starts execution of the
 * registered ucode-program, the address of which is equal to the SNC base address,
 * bypassing control by PDC
 *
 * \note If SNC is disabled/stopped when enabling SNC, it starts the execution of the targeted
 *       program from the memory address where the Program Counter (PC) points to, otherwise,
 *       if SNC has completed execution, first it resets the PC value to the value of the SNC
 *       base address, and then starts the execution of the targeted program from the "beginning"
 *
 * \sa hw_snc_set_base_address
 * \sa hw_snc_manual_disable
 * \sa hw_snc_has_completed_execution
 *
 */
void hw_snc_manual_enable(void);

/**
 * \brief Disable manually SNC
 *
 * When SNC is disabled manually, SW-Control mode is enabled for SNC and it stops execution of the
 * registered ucode-program, the address of which is equal to the SNC base address,
 * bypassing control by PDC
 *
 * \note When SNC is disabled, it will first complete the last on-going command before being
 *       stopped/halted
 *
 * \sa hw_snc_set_base_address
 * \sa hw_snc_manual_enable
 *
 */
void hw_snc_manual_disable(void);

/**
 * \brief Enable SNC being controlled by PDC
 *
 * When SNC is controlled by PDC, SW-Control mode is disabled for SNC. This means that SNC is
 * enabled or disabled based on the triggered PDC entries registered for SNC as target master
 *
 */
void hw_snc_pdc_ctrl_enable(void);

/**
 * \brief Sets the function to emulate the delay in low power clock ticks implied by SENIS_DEL
 *        command when running on the SNC emulator
 *
 * \param [in] cb       pointer function implementing a delay in low power clock ticks
 *
 */
void hw_snc_set_emu_delay_func(hw_snc_emu_delay_cb_t cb);

/**
 * \brief Sets the function to trigger an execution indication function when running on
 *        the SNC emulator, return the SNC program counter (PC) of the SeNIS command to be executed
 *
 * \param [in] cb       pointer to function being called when a SeNIS command is executed
 *
 */
void hw_snc_set_emu_exec_indication(hw_snc_emu_exec_ind_cb_t cb);

/**
 * \brief Executes SNC emulator
 *
 */
void hw_snc_emu_run(void);

/**
 * \brief Reset SNC
 *
 * \note When SNC is reset, its program counter is set back to the programmed base address.
 *
 * \warning Resetting SNC may interrupt its regular execution and any command currently being
 *          executed may be abnormally terminated
 *
 */
__STATIC_INLINE void hw_snc_reset(void)
{
        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_RESET, 1);
}

//==================== Configuration functions =================================

/**
 * \brief Enable bus error detection
 *
 * It enables the detection of system bus errors that may occur in case a non-mapped address is
 * used by SNC, when performing a register access. Whether a bus error has been produced
 * or not can be checked using hw_snc_hasBusError() function
 *
 * \sa hw_snc_hasBusError
 *
 */
__STATIC_INLINE void hw_snc_bus_error_detection_enable(void)
{
        HW_SNC_REG_SETF(SNC_CTRL_REG, BUS_ERROR_DETECT_EN, 1);
}

/**
 * \brief Disable bus error detection
 *
 * It disables the detection of system bus errors that may occur while SNC is running
 *
 */
__STATIC_INLINE void hw_snc_bus_error_detection_disable(void)
{
        HW_SNC_REG_SETF(SNC_CTRL_REG, BUS_ERROR_DETECT_EN, 0);
}

/**
 * \brief Sets the SNC base address
 *
 * It sets the SNC base address, which is the memory address from where SNC starts execution of
 * SeNIS commands (i.e. the address of the target ucode-program)
 *
 * \param [in] base_address     the new SNC base address
 *
 */
void hw_snc_set_base_address(uint32_t base_address);

/**
 * \brief Sets the SNC program counter (PC)
 *
 * It sets the SNC program counter, which is the memory address from where SNC will start
 * execution of the target programm's SeNIS commands after manually enabling it by calling
 * hw_snc_manual_enable()
 *
 * \param [in] pc       the new SNC program counter
 *
 * \note Program counter can be set only when SNC has been previously manually disabled
 *       (i.e. using hw_snc_manual_disable()), otherwise calling the particular function has
 *       no effect
 *
 * \sa hw_snc_manual_enable
 * \sa hw_snc_manual_disable
 *
 */
__STATIC_INLINE void hw_snc_set_pc(uint32_t pc)
{
        ASSERT_ERROR(IS_SYSRAM_ADDRESS(black_orca_phy_addr(pc)));
        ASSERT_ERROR(pc % 4 == 0);
        if (HW_SNC_REG_GETF(SNC_STATUS_REG, SNC_IS_STOPPED)) {
                HW_SNC_REG_SETF(SNC_PC_REG, PC_REG, (pc >> 2));
                while (!HW_SNC_REG_GETF(SNC_STATUS_REG, SNC_PC_LOADED));
        }
}

/**
 * \brief Set the SNC clock division factor
 *
 * \note SNC clock frequency depends also on both hclk frequency (HCLK_DIV) and
 *       pclk frequency (PCLK_DIV), thus being given by
 *       (XTAL or PLL)/(HCLK_DIV * PCLK_DIV * SNC_DIV)
 *
 * \param [in] clk_div  the selected SNC clock division factor
 *
 */
__STATIC_INLINE void hw_snc_set_clock_div(HW_SNC_CLK_DIV clk_div)
{
        // always operate on 32MHz
}

/**
 * \brief Enable SNC interrupt (where IRQ line is routed towards CM33)
 *
 * The interrupt handler registered with hw_snc_register_int() function is called when SNC
 * interrupt (to CM33) event occurs
 *
 * \sa hw_snc_register_int
 *
 */
void hw_snc_interrupt_enable(void);

/**
 * \brief Disable SNC interrupt (where IRQ line is routed towards CM33)
 *
 */
void hw_snc_interrupt_disable(void);

/**
 * \brief Register an interrupt callback handler for the SNC interrupt
 * (where IRQ line is routed towards CM33) and enable the SNC NVIC IRQ
 *
 * \param [in] handler  function pointer to call when SNC interrupt (to CM33) event occurs
 *
 */
void hw_snc_register_int(hw_snc_interrupt_cb_t handler);

/**
 * \brief Unregister the interrupt callback handler (set it to NULL) for the SNC interrupt
 * (where IRQ line is routed towards CM33) and disable the SNC NVIC IRQ
 *
 */
void hw_snc_unregister_int(void);

/**
 * \brief Enable SNC to Power Domains Controller (PDC) event
 *
 * It enables SNC to PDC event (where IRQ line is routed towards PDC)
 *
 */
void hw_snc_pdc_event_enable(void);

/**
 * \brief Disable SNC to Power Domains Controller (PDC) event
 *
 * It disables SNC to PDC event (where IRQ line is routed towards PDC)
 *
 */
void hw_snc_pdc_event_disable(void);

//==================== State Acquisition functions =============================

/**
 * \brief Check if SNC is enabled
 *
 * It returns true if SNC is enabled. If SW-Control mode is disabled, then SNC is always
 * enabled/controlled by PDC
 *
 * \return true  - if SNC is enabled
 *         false - if SNC is disabled
 *
 * \sa hw_snc_manual_enable
 * \sa hw_snc_manual_disable
 * \sa hw_snc_pdc_ctrl_enable
 *
 */
__STATIC_INLINE bool hw_snc_is_enabled(void)
{
        return !HW_SNC_REG_GETF(SNC_CTRL_REG, SNC_SW_CTRL) || HW_SNC_REG_GETF(SNC_CTRL_REG, SNC_EN);
}

/**
 * \brief Check if SNC SW-Control mode is enabled
 *
 * It returns true if SNC SW-Control is enabled, which means that SNC is (manually) controlled by
 * the host, otherwise it is controlled by PDC.
 *
 * \return true  - if SW-Control is enabled
 *         false - if SW_Control is disabled
 */
__STATIC_INLINE bool hw_snc_is_sw_control_enabled(void)
{
        return HW_SNC_REG_GETF(SNC_CTRL_REG, SNC_SW_CTRL);
}

/**
 * \brief Check if SNC bus error detection is enabled
 *
 * \return true  - if bus error detection is enabled
 *         false - if bus error detection is disabled
 */
__STATIC_INLINE bool hw_snc_is_bus_error_detection_enabled(void)
{
        return HW_SNC_REG_GETF(SNC_CTRL_REG, BUS_ERROR_DETECT_EN);
}

/**
 * \brief Get SNC base address
 *
 * \return SNC base address
 */
uint32_t hw_snc_get_base_address(void);

/**
 * \brief Get SNC program counter (PC)
 *
 * \return SNC program counter (PC)
 */
__STATIC_INLINE uint32_t hw_snc_get_pc(void)
{
        return (uint32_t)((HW_SNC_REG_GETF(SNC_PC_REG, PC_REG) << 2) | 0x20000000);
}

/**
 * \brief Get the SNC clock division factor
 *
 * \return the current SNC clock division factor
 *
 */
__STATIC_INLINE HW_SNC_CLK_DIV hw_snc_get_clock_div(void)
{
        return REG_GETF(CRG_COM, CLK_COM_REG, SNC_DIV);
}

/**
 * \brief Check if SNC has finished the target program's execution
 *
 * It returns true if SNC has finished the target program's execution, by executing a SLP command.
 *
 * \note This function must be used when SW-Control mode is enabled. SNC execution becomes
 *       pending when SNC is either disabled and (re-)enabled again (i.e. calling
 *       hw_snc_manual_disable() and then hw_snc_manual_enable()) or reset (i.e. calling
 *       hw_snc_reset())
 *
 * \return true  - if SNC execution is completed
 *         false - if SNC execution is pending
 *
 * \sa hw_snc_manual_enable
 * \sa hw_snc_manual_disable
 * \sa hw_snc_reset
 *
 */
__STATIC_INLINE bool hw_snc_has_completed_execution(void)
{
        return HW_SNC_REG_GETF(SNC_STATUS_REG, SNC_DONE_STATUS);
}

/**
 * \brief Check if an SNC bus error has been produced during the target program's execution
 *
 * It returns true if SNC execution has produced a bus error (i.e. a non-mapped address has been
 * used by SNC during the target program's execution).
 *
 * \return true  - if an SNC bus error has been produced
 *         false - if no SNC bus error has been produced
 *
 * \note SNC bus error status is reset when SNC is re-initialized, by starting again from its base
 *       address (i.e. either being disabled and (re-)enabled again, by calling
 *       hw_snc_manual_disable() and then hw_snc_manual_enable(), or reset, by calling
 *       hw_snc_reset())
 *
 * \sa hw_snc_manual_enable
 * \sa hw_snc_manual_disable
 * \sa hw_snc_reset
 *
 */
__STATIC_INLINE bool hw_snc_has_bus_error(void)
{
        return HW_SNC_REG_GETF(SNC_STATUS_REG, BUS_ERROR_STATUS);
}

/**
 * \brief Check if an SNC hard fault has been produced during the target program's execution
 *
 * It returns true if SNC execution has produced a hard fault or opcode error (i.e. an invalid
 * instruction opcode has been used by SNC during the target program's execution).
 *
 * \return true  - if an SNC hard fault has been produced
 *         false - if no SNC hard fault has been produced
 *
 * \note SNC hard fault status is reset when SNC is re-initialized, by starting again from its base
 *       address (i.e. either being disabled and (re-)enabled again, by calling
 *       hw_snc_manual_disable() and then hw_snc_manual_enable(), or reset, by calling
 *       hw_snc_reset())
 *
 * \sa hw_snc_manual_enable
 * \sa hw_snc_manual_disable
 * \sa hw_snc_reset
 *
 */
__STATIC_INLINE bool hw_snc_has_hard_fault(void)
{
        return HW_SNC_REG_GETF(SNC_STATUS_REG, HARD_FAULT_STATUS);
}

/**
 * \brief Checks if target program's execution by SNC has been stopped
 *
 * It returns true if SNC execution is stopped and its FSM is halted (i.e. when disabling SNC,
 * by calling hw_snc_manual_disable()).
 *
 * \return true  - if SNC is stopped/halted
 *         false - if SNC is not stopped (i.e. either currently executing a target program or
 *                                        currently being idle, having executed a SLP command)
 *
 * \sa hw_snc_manual_disable
 *
 */
__STATIC_INLINE bool hw_snc_is_stopped(void)
{
        return HW_SNC_REG_GETF(SNC_STATUS_REG, SNC_IS_STOPPED);
}

#endif /* dg_configUSE_HW_SENSOR_NODE & dg_configUSE_HW_SENSOR_NODE_EMU */


#endif /* SENSOR_NODE_EMU_H_ */

/**
 * \}
 * \}
 */
