/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup MID_SNC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc.h
 *
 * @brief SNC-Sensor Node Controller Header File
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_H_
#define SNC_H_


#if dg_configUSE_HW_SENSOR_NODE

#include "osal.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC uCode-Block commands bit positions
 *
 */
typedef enum {
        SNC_UCODE_BLOCK_CMD_CM33_CTRL = 0,      /**< CM33 control over a uCode-Block resource command */
        SNC_UCODE_BLOCK_CMD_DISABLE,            /**< Disable-uCode-Block command */
        SNC_UCODE_BLOCK_CMD_MAX
} SNC_UCODE_BLOCK_CMD;

/**
 * \brief SNC uCode-Block status flags bit positions
 *
 */
typedef enum {
        SNC_UCODE_BLOCK_FLAG_SNC_CTRL = 0,      /**< SNC control over a uCode-Block resource flag */
        SNC_UCODE_BLOCK_FLAG_RUNNING,           /**< uCode-Block-is-running flag */
        SNC_UCODE_BLOCK_FLAG_MAX
} SNC_UCODE_BLOCK_FLAG;

#if dg_configUSE_SNC_QUEUES
/**
 * \brief SNC queue type
 *
 */
typedef void* snc_queue_t;

/**
 * \brief SNC queue element size/weight enumeration
 *
 */
typedef enum {
        SNC_QUEUE_ELEMENT_SIZE_BYTE =  1,       /**< Element size is 1 byte                     */
        SNC_QUEUE_ELEMENT_SIZE_HWORD = 2,       /**< Element size is half word ( 2 bytes )      */
        SNC_QUEUE_ELEMENT_SIZE_WORD =  4,       /**< Element size is word ( 4 bytes )           */
} SNC_QUEUE_ELEMENT_SIZE;

/**
 * \brief SNC queue configuration structure
 *
 */
typedef struct {
        uint32_t max_chunk_bytes;               /**< Max number of bytes in chunk                      */
        uint32_t num_of_chunks;                 /**< Max number of chunks                              */
        SNC_QUEUE_ELEMENT_SIZE element_weight;  /**< Number of utilized bytes in a 32-bit word element */
        bool enable_data_timestamp;             /**< Timestamp in header enabled/disabled flag         */
        bool swap_pushed_data_bytes;            /**< Swap CM33 pushed data bytes                       */
        bool swap_popped_data_bytes;            /**< Swap CM33 popped data bytes                       */
} snc_queue_config_t;
#endif /* dg_configUSE_SNC_QUEUES */

#if dg_configUSE_SNC_DEBUGGER
/**
 * \brief Function type controlling an SNC breakpoint group
 *
 */
typedef int (*bkpt_control_func_t)(void);

/**
 * \brief Function type enabling SNC breakpoint control over a SeNIS construct
 *
 */
typedef int (*bkpt_func_t)(bkpt_control_func_t bkpt_control_func);

#if dg_configUSE_HW_SENSOR_NODE_EMU
/**
 * \brief SNC Emulator debugging attributes type
 *
 */
typedef void* snc_emu_dbg_attrs_t;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

/**
 * \brief SNC uCode-Block context structure
 *
 */
typedef struct snc_ucode_context_t {
        uint32_t* ucode;                /**< Pointer to the uCode starting address */
        uint32_t* lp;                   /**< Link pointer to jump back to the previous uCode in the execution flow */


        /**< uCode flags as defined in SNC_UCODE_BLOCK_FLAG enumeration
         * bit0 - Flag indicating SNC control over a uCode-Block resource
         * bit1 - Flag indicating uCode-Block-is-running
         */
        uint32_t flags;

        /**< uCode flags as defined in SNC_UCODE_BLOCK_CMD enumeration
         * bit0 - Command requesting CM33 control over a uCode-Block resource
         * bit1 - Command requesting uCode-Block to be disabled
         */
        uint32_t cmd;

        /**< Flag indicating a pending notification event from SYSCPU (CM33) to the uCode-Block
         * (1: pending event)
         */
        uint32_t CM33_to_SNC_triggered;

        uint32_t ucode_id;              /**< uCode id set when .ucode_create() is called */

#if dg_configUSE_SNC_QUEUES
        /**< Queue for data communication from SNC to SYSCPU (CM33) */
        snc_queue_t SNC_to_CM33_data_queue;

        /**< Queue for data communication from SYSCPU (CM33) to SNC */
        snc_queue_t CM33_to_SNC_data_queue;
#endif /* dg_configUSE_SNC_QUEUES */

#if dg_configUSE_SNC_DEBUGGER
        uint32_t size;                  /**< uCode size in 32bit words */
#if dg_configUSE_HW_SENSOR_NODE_EMU
        snc_emu_dbg_attrs_t dbg_attrs;  /**< SNC emulator debugging attributes of the uCode */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        /**< Function pointer initialized upon uCode definition (i.e. SNC_UCODE_BLOCK_DEF()),
         * used for calling the constructor function which creates the uCode and sets its id
         */
        void (*ucode_create)(uint32_t ucode_id);
        /**< Function pointer initialized upon uCode definition (i.e. SNC_UCODE_BLOCK_DEF()),
         * used for calling the destructor function which deletes the uCode and frees its
         * allocated resources
         */
        void (*ucode_delete)(void);
} snc_ucode_context_t;

/**
 * \brief SeNIS base address types
 *
 */
typedef enum {
        SENIS_MEM_BASE_ADDRESS = 0x20000000,
        SENIS_REGS_BASE_ADDRESS = 0x50000000,
} SENIS_BASE_ADDRESS;

/**
 * \brief SeNIS opcodes
 *
 */
typedef enum {
        SENIS_OP_NOP,                   /**< Opcode for NOP command   */
        SENIS_OP_WADAD,                 /**< Opcode for WADAD command */
        SENIS_OP_WADVA,                 /**< Opcode for WADVA command */
        SENIS_OP_TOBRE,                 /**< Opcode for TOBRE command */
        SENIS_OP_RDCBI,                 /**< Opcode for RDCBI command */
        SENIS_OP_RDCGR,                 /**< Opcode for RDCGR command */
        SENIS_OP_COBR,                  /**< Opcode for COBR command  */
        SENIS_OP_INC,                   /**< Opcode for INC command   */
        SENIS_OP_DEL,                   /**< Opcode for DEL command   */
        SENIS_OP_SLP,                   /**< Opcode for SLP command   */
} SENIS_OPCODE_TYPE;

/**
 * \brief SNC flags used by SeNIS main functions
 *
 */
typedef enum {
        SENIS_FLAG_DA1  = (1 << 27),    /**< Direct Addressing mode flag set */
        SENIS_FLAG_InA2 = (1 << 26),    /**< Indirect Addressing mode flag set */
        SENIS_FLAG_REG  = (1 << 19),    /**< Register address flag set (i.e. given address > 0x50000000) */
        SENIS_FLAG_EQ   = (1 <<  0),    /**< Equal flag set */
        SENIS_FLAG_NEQ  = (0 <<  0),    /**< Equal flag not set */
        SENIS_FLAG_GR   = (1 <<  1),    /**< Greater flag set */
        SENIS_FLAG_NGR  = (0 <<  1),    /**< Greater flag not set */
        SENIS_FLAG_INC4 = (1 << 19),    /**< Increment-by-4 mode flag set */
} SENIS_FLAG_TYPE;

/**
 * \brief SNC uCode type
 *
 */
typedef enum {
        /**< uCode used for handling a PDC event sent to SNC when registered to SNC-main-uCode,
         * and exchange data with SYSCPU (CM33) */
        SNC_UCODE_TYPE_UCODE_BLOCK,
        /**< uCode used as function, i.e. implementing a specific operation to be used (called) by other uCodes */
        SNC_UCODE_TYPE_FUNC,
        /**< SNC-main-uCode */
        SNC_UCODE_TYPE_MAIN
} SNC_UCODE_TYPE;

/**
 * \brief Type definition of SNC/CM33 mutexes
 *
 */
typedef struct {
        uint32_t snc_semph;             /**< SNC semaphore variable        */
        uint32_t cm33_semph;            /**< CM33 semaphore variable       */
#ifndef OS_BAREMETAL
        OS_MUTEX os_semph;              /**< OS related semaphore variable */
#endif /* OS_BAREMETAL */
} snc_cm33_mutex_t;

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "SeNIS_macros.h"

#if dg_configUSE_SNC_DEBUGGER

/**
 * \brief Macros used in SNC context to enable and control breakpoints and step-by-step debugging
 *
 * If a breakpoint is set next to SNC_BKPT_GROUP() macro, then all SNC breakpoints and step-by-step
 * debugging regions defined for that group (e.g. foo_group), using SNC_BKPT(foo_group) and
 * SNC_STEP_BY_STEP_[BEGIN/END](foo_group), respectively, are enabled, resulting in both SNC and
 * SYSCPU (CM33) halting when the SNC execution flow reaches SNC breakpoints.
 *
 * Default SNC breakpoint group (i.e. SNC_BKPT_GROUP_DFLT()) shall be always defined in order to
 * enable and control SNC breakpoints and step-by-step debugging regions defined by the SNC_BKPT()
 * and SNC_STEP_BY_STEP_[BEGIN/END]() macros when no group is used as argument.
 *
 * Specifically, for step-by-step debugging use:
 *  - "step-return":    to show the current SeNIS construct command being executed in a uCode, and
 *  - "resume":         to move to the next SeNIS construct command
 * And for debugging over SNC breakpoints use:
 *  - "resume":         to move to the next SeNIS construct command
 *
 * Example usage:
 * \code{.c}
 * SNC_BKPT_GROUP_DFLT();               // Definition of snc_bkpt_group_dflt default breakpoint group
 * SNC_BKPT_GROUP(bkpt_group1);         // Definition of bkpt_group1 breakpoint group
 * SNC_BKPT_GROUP(bkpt_group2);         // Definition of bkpt_group2 breakpoint group
 * \endcode
 *
 * \note Define here SNC breakpoint groups (i.e. SNC_BKPT_GROUP()), so that they can be applied
 *       to all SNC uCodes.
 *
 * \sa SNC_BKPT
 * \sa SNC_STEP_BY_STEP_BEGIN
 * \sa SNC_STEP_BY_STEP_END
 *
 */
SNC_BKPT_GROUP_DFLT();
// SNC_BKPT_GROUP(foo_bkpt_group);

#if dg_configUSE_HW_SENSOR_NODE_EMU
/**
 * \brief Macro used in SNC Emulator context to enable and control step-by-step debugging
 *
 * If a breakpoint is set next to this macro, when SNC Emulator is used, then step-by-step
 * debugging is enabled over SeNIS construct commands.
 *
 * Specifically, use:
 *  - "step-return":    to show the current SeNIS construct command being executed in a uCode, and
 *  - "resume":         to move to the next SeNIS construct command
 *
 */
SNC_BKPT_GROUP_EMU();
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== uCode declaration/definition macros =====================

/**
 * \brief Macro used in SYSCPU context, returning the SNC context of a given uCode
 *
 * \param [in] name     the name of the uCode
 *
 * \return the address of the uCode context
 *
 * Example usage:
 * \code{.c}
 * SNC_UCODE_BLOCK_DECL(myUcodeBlockExample);
 *
 * void foo_func(void)
 * {
 *         ...
 *         snc_ucode_context_t* ctx = SNC_UCODE_CTX(myUcodeBlockExample);
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_UCODE_CTX(name)                                                                     \
        (_SNC_UCODE_GET_CTX(name)())

/**
 * \brief Macro used in SYSCPU context, returning the type of a given uCode
 *
 * \param [in] name     the name of the uCode
 *
 * \return the type of the uCode (i.e. SNC_UCODE_TYPE)
 *
 * Example usage:
 * \code{.c}
 * SNC_UCODE_BLOCK_DECL(myUcodeBlockExample);
 *
 * void foo_func(void)
 * {
 *         ...
 *         SNC_UCODE_TYPE type = SNC_UCODE_TYPE(myUcodeBlockExample);
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_UCODE_TYPE(name)                                                                    \
        (_SNC_UCODE_GET_TYPE(name)())

/**
 * \brief Macro used in SYSCPU context to declare a uCode-Block
 *
 * \param [in] name     the name of the uCode-Block
 *
 * Example usage:
 * \code{.c}
 * SNC_UCODE_BLOCK_DECL(myUcodeBlockExample);
 *
 * void foo_func(void)
 * {
 *         ...
 *         snc_ucode_context_t* ctx = SNC_UCODE_CTX(myUcodeBlockExample);
 *         SNC_UCODE_TYPE type = SNC_UCODE_TYPE(myUcodeBlockExample);
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_UCODE_CTX
 * \sa SNC_UCODE_TYPE
 *
 */
#define SNC_UCODE_BLOCK_DECL(name)                                                              \
        _SNC_FUNC_DECL(ucode, name, void)

/**
 * \brief Macro used in SYSCPU context to define a uCode-Block
 *
 * \param [in] name     the name of the uCode-Block
 *
 * Example usage:
 * \code{.c}
 * // list of global variables
 * uint32_t var1, var2;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeBlockExample)
 * {
 *         // A sequence of SeNIS construct sentences constituting the uCode implementation
 *         // e.g.
 *         //   SENIS_labels(label0, label1);
 *         //   ...
 *         //   SENIS_assign(da(&var1), da(&var2));
 *         //   ...
 *         //   SENIS_label(label0);
 *         //
 *         //   SENIS_if(da(&var1), GT, 0) {
 *         //           SENIS_assign(da(&var1), 0);
 *         //           ...
 *         //   }
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_UCODE_BLOCK_DEF(name)                                                               \
        _SNC_FUNC_DEF(ucode, name)

//==================== Functions used for SYSCPU (CM33) to SNC uCodes communication =

/**
 * \brief Function used in SYSCPU context to enter critical section
 *
 * This allows to enter critical section, preventing SNC uCodes from accessing a shared resource.
 *
 * Example usage:
 * \code{.c}
 * uint32_t foo_var = 0;                        // Definition of the shared variable foo_var
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_ENTER_CRITICAL_SECTION();        // Enter critical section in SNC context
 *         SENIS_assign(da(&foo_var), 0);       // Access foo_var in SNC context
 *         SNC_LEAVE_CRITICAL_SECTION();        // Leave critical section in SNC context
 *         ...
 * )
 *
 * void foo_func_in_CM33_context()              // Function called in CM33 context,
 *                                              // accessing shared variable foo_var
 * {
 *         ...
 *         snc_enter_SNC_critical_section();    // Enter critical section in CM33 context
 *         foo_var = 1;                         // Access foo_var in CM33 context
 *         snc_leave_SNC_critical_section();    // Leave critical section in CM33 context
 *         ...
 * }
 * \endcode
 *
 * \sa snc_leave_SNC_critical_section
 * \sa SNC_ENTER_CRITICAL_SECTION
 * \sa SNC_LEAVE_CRITICAL_SECTION
 *
 */
void snc_enter_SNC_critical_section(void);

/**
 * \brief Function used in SYSCPU context to leave critical section
 *
 * This restores access to a shared resource, allowing SNC uCodes to access it.
 *
 * Example usage:
 * \code{.c}
 * uint32_t foo_var = 0;                        // Definition of the shared variable foo_var
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_ENTER_CRITICAL_SECTION();        // Enter critical section in SNC context
 *         SENIS_assign(da(&foo_var), 0);       // Access foo_var in SNC context
 *         SNC_LEAVE_CRITICAL_SECTION();        // Leave critical section in SNC context
 *         ...
 * )
 *
 * void foo_func_in_CM33_context()              // Function called in CM33 context,
 *                                              // accessing shared variable foo_var
 * {
 *         ...
 *         snc_enter_SNC_critical_section();    // Enter critical section in CM33 context
 *         foo_var = 1;                         // Access foo_var in CM33 context
 *         snc_leave_SNC_critical_section();    // Leave critical section in CM33 context
 *         ...
 * }
 * \endcode
 *
 * \sa snc_enter_SNC_critical_section
 * \sa SNC_ENTER_CRITICAL_SECTION
 * \sa SNC_LEAVE_CRITICAL_SECTION
 *
 */
void snc_leave_SNC_critical_section(void);

/**
 * \brief Function used in SYSCPU context to create/initialize an SNC/CM33 mutex
 *
 * \param [in] mutex    pointer to the variable used as the mutex to be created
 *
 * \sa snc_mutex_SNC_delete
 * \sa snc_mutex_SNC_lock
 * \sa snc_mutex_SNC_unlock
 *
 */
void snc_mutex_SNC_create(snc_cm33_mutex_t* mutex);

/**
 * \brief Function used in SYSCPU context to delete an SNC/CM33 mutex
 *
 * \param [in] mutex    pointer to the variable used as the mutex to be deleted
 *
 * \sa snc_mutex_SNC_create
 * \sa snc_mutex_SNC_lock
 * \sa snc_mutex_SNC_unlock
 *
 */
void snc_mutex_SNC_delete(snc_cm33_mutex_t* mutex);

/**
 * \brief Function used in SYSCPU context to acquire a mutex
 *
 * The mutex is created using snc_mutex_SNC_create() function.
 *
 * \param [in] mutex    pointer to the variable used as the mutex to lock
 *
 * \sa snc_mutex_SNC_create
 * \sa snc_mutex_SNC_delete
 * \sa snc_mutex_SNC_unlock
 *
 */
void snc_mutex_SNC_lock(snc_cm33_mutex_t* mutex);

/**
 * \brief Function used in SYSCPU context to release a mutex
 *
 * The mutex is created using snc_mutex_SNC_create() function.
 * The snc_mutex_SNC_lock() function should be called before calling this function.
 *
 * \param [in] mutex    pointer to the variable used as the mutex to unlock
 *
 * \sa snc_mutex_SNC_create
 * \sa snc_mutex_SNC_delete
 * \sa snc_mutex_SNC_lock
 *
 */
void snc_mutex_SNC_unlock(snc_cm33_mutex_t* mutex);

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_H_ */

/**
 * \}
 * \}
 */
