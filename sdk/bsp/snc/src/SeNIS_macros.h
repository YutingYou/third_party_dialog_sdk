/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SENIS
 *
 * \brief SNC-Sensor Node Instruction Set (SeNIS) -based framework macros
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file SeNIS_macros.h
 *
 * @brief SNC-Macro definition for Sensor Node Instruction Set (SeNIS)
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SENIS_MACROS_H_
#define SENIS_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#include <stdint.h>
#include <string.h>

#ifndef OS_BAREMETAL
#include "osal.h"
#endif /* OS_BAREMETAL */

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC Function-uCode context structure
 */
typedef struct snc_func_context_t {
        uint32_t* ucode;                /**< Pointer to the uCode starting address */
        uint32_t* lp;                   /**< Link pointer to jump back to the previous
                                             uCode in the execution flow */

        uint32_t numOfConns;            /**< Number of connections/dependencies from other uCodes */

#if dg_configUSE_SNC_DEBUGGER
        uint32_t size;                  /**< uCode size in 32bit words */
#if dg_configUSE_HW_SENSOR_NODE_EMU
        snc_emu_dbg_attrs_t dbg_attrs;  /**< SNC emulator debugging attributes of the uCode */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        /**< Function pointer initialized upon uCode definition (i.e. SNC_FUNC_DEF()),
         * used for calling the constructor function which creates the uCode
         */
        void (*ucode_create)(void);
        /**< Function pointer initialized upon uCode definition (i.e. SNC_FUNC_DEF()),
         * used for calling the destructor function which deletes the uCode and frees its
         * allocated resources
         */
        void (*ucode_delete)(void);
} snc_func_context_t;

/**
 * \brief SNC context structure
 */
typedef struct snc_context_t {
        /** < Number of requests by uCodes for preventing SNC from going to sleep. */
        uint32_t prevent_slp_cnt;

#if dg_configUSE_SNC_DEBUGGER
        bkpt_func_t snc_bkpt;                   /**< Function enabling SNC breakpoint control (i.e. SNC breakpoint indication) */
        bkpt_control_func_t snc_bkpt_control;   /**< Function controlling the group of the indicated SNC breakpoint (i.e. .snc_bkpt) */
#endif /* dg_configUSE_SNC_DEBUGGER */

#if dg_configUSE_HW_SENSOR_NODE_EMU
        uint32_t snc_emu;                       /**< SNC emulator mode indication when SYSCPU (CM33) is notified by SNC */
#if dg_configUSE_SNC_DEBUGGER
        snc_emu_dbg_attrs_t snc_emu_dbg_list;   /**< Pointer to the first element in the list
                                                     including debugging attributes of the uCodes */
#endif /* dg_configUSE_SNC_DEBUGGER */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

        uint32_t snc_active;                    /**< SNC flag indicating that SNC is active (1) or halted (0) */
        uint32_t snc_halt;                      /**< SNC flag indicating request for SNC halt */

        /**< Bitmap of uCode IDs indicating SNC to CM33 pending notification events.
         * Multiple events may have been triggered
         */
        uint32_t snc_to_CM33_trigger;
} snc_context_t;

extern snc_context_t snc_context;

extern uint32_t snc_const[];

extern uint32_t senis_RDCBI_bit_pos_array[];
extern uint32_t senis_TOBRE_bit_mask_array[];

#if dg_configUSE_SNC_DEBUGGER
#if dg_configUSE_HW_SENSOR_NODE_EMU
/**
 * \brief SNC emulator step-by-step debugging attributes structure
 *
 */
typedef struct snc_emu_sbs_attrs_t {
        uint32_t* ucode_start_address;          /**< Pointer to the uCode starting address */
        uint32_t ucode_size;                    /**< uCode size in 32bit words */
        struct snc_emu_sbs_attrs_t* next;       /**< Pointer to the next structure referring to a
                                                     different uCode */
        bkpt_func_t sbs_funcs[0];               /**< Functions implementing step-by-step debugging */
} snc_emu_sbs_attrs_t;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

/**
 * \brief SeNIS operand type
 */
typedef enum {
        SENIS_OPER_TYPE_ADDRESS_IA,             /**< Indirect addressing operand type */
        SENIS_OPER_TYPE_ADDRESS_DA,             /**< Direct addressing operand type */
        SENIS_OPER_TYPE_VALUE,                  /**< Value operand type */
} SENIS_OPER_TYPE;

/**
 * \brief SeNIS shift type
 */
typedef enum {
        SENIS_SHIFT_TYPE_RIGHT,                 /**< Right-shift type */
        SENIS_SHIFT_TYPE_LEFT,                  /**< Left-shift type */
} SENIS_SHIFT_TYPE;

/**
 * \brief uCode building context structure
 */
typedef struct b_ctx_t {
        SNC_UCODE_TYPE ucode_type;              /**< The uCode type, i.e. uCode-Block, Function-uCode, SNC-main-uCode */
        void* ucode_this_ctx;                   /**< The current uCode, context */

        uint32_t creating;                      /**< Flag indicating the uCode is being created (1) or deleted (0) */

        uint32_t header_len;                    /**< The length of the uCode header in 32bit words */
        uint32_t body_len;                      /**< The length of the uCode body in 32bit words */
        uint32_t footer_len;                    /**< The length of the uCode footer in 32bit words */

        uint32_t l_upd;                         /**< Flag indicating uCode label updating phase (-1: updating is enabled) */
        uint32_t l_index;                       /**< Index value counting 32bit words in the allocated for the labels memory space */
        uint32_t* labels;                       /**< Pointer to the allocated for the labels memory space */
        uint32_t labels_len;                    /**< Length of the allocated for the labels memory space in 32bit words */

        uint32_t upd;                           /**< Flag indicating uCode body static space footer updating phase (-1: updating is enabled) */
        uint32_t index;                         /**< Index value counting 32bit words in the allocated for the uCode body memory space */

        uint32_t fs_index;                      /**< Index value counting 32bit words in the allocated memory space used for the static space footer */
        uint32_t ft_index;                      /**< Index value counting 32bit words in the allocated memory space used for the temporary space footer */

        uint32_t len;                           /**< Total length of the allocated for the uCode memory space in 32bit words */

        uint32_t* ucode;                        /**< Pointer to the uCode body starting position */
        uint32_t** lp;                          /**< Pointer to the link pointer defined for the uCode in its context */

        uint32_t* senis_break_addr;             /**< The current address in the uCode body to jump to when a SENIS_break is to be called */
        uint32_t* senis_continue_addr;          /**< The current address in the uCode body to jump to when a SENIS_continue is to be called */

#if dg_configUSE_SNC_DEBUGGER
        uint32_t debug_en;                      /**< Flag indicating whether SNC debugging is enabled (1: enabled, 0: disabled) */

        bkpt_control_func_t bkpt_sbs_control_func;      /**< Pointer to function controlling an SNC breakpoint group (NULL: disabled) */
#if dg_configUSE_HW_SENSOR_NODE_EMU
        bkpt_func_t* bkpt_emu_sbs_funcs;        /**< Pointer to the current uCode functions implementing
                                                     step-by-step debugging in SNC Emulator context */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
} b_ctx_t;

/**
 * \brief Pointer to a function wrapping a SeNIS block i.e. if/else/while/do-while block
 */
typedef void (*senis_blk_t)(b_ctx_t* b_ctx);

/**
 * \brief Pointer to a function wrapping an if-else clause SeNIS block
 */
typedef void (*senis_ifelse_blk_t)(b_ctx_t* b_ctx, senis_blk_t* else_blk);

/**
 * \brief Pointer to a function wrapping a while clause SeNIS block
 */
typedef senis_blk_t senis_while_blk_t;

/**
 * \brief Pointer to a function wrapping a do-while clause SeNIS block
 */
typedef senis_blk_t senis_dowhile_blk_t;

/**
 * \brief Enumeration value used for defining empty-operand SeNIS commands which are to be overwritten
 */
enum {
        SNC_PLACE_HOLDER
};

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_sys.h"
#include "snc_debug.h"

/*
 * MACRO DEFINITIONS
 *****************************************************************************************
 */

#define ___SNC_APPEND_LINE_NO(x, line_no)       x##line_no
#define __SNC_APPEND_LINE_NO(x, line_no)        ___SNC_APPEND_LINE_NO(x, line_no)
#define _SNC_APPEND_LINE_NO(x)                  __SNC_APPEND_LINE_NO(x, __LINE__)

#define _SNC_UCODE_INDX         b_ctx->index
#define _SNC_UCODE_UPD          b_ctx->upd&b_ctx->index++

/**
 * \brief Macro printing a message indicating wrong type of operand
 */
#define _SNC_OP_TYPE_ERROR(error)                                                               \
        SNC_OP_TYPE_ERROR                                                                       \
        "Variable Type error" type error is not recognised                                      \
        A variable 'var' must be defined as                                                     \
              'da(var)': for direct addressing                                                  \
              'ia(var)': for indirect addressing                                                \
                  'var': for value                                                              \
        __FILE__: __LINE__                                                                      \
        :

/**
 * \brief Macro printing a message indicating wrong number of arguments in a macro definition
 */
#define _SNC_NUM_ARGS_ERROR(numOfArgs)                                                          \
        SNC_NUM_ARGS_ERROR                                                                      \
        "Syntax error" number of arguments numOfArgs is wrong                                   \
        __FILE__: __LINE__                                                                      \
        :

/**
 * \brief Macro implementing different macro flavors based on the number of their arguments
 */
#define __CHOOSE_FLAVOR(_1, _2 , _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _FLAVOR, ...) _FLAVOR

/**
 * \brief Macro abstracting [name]_get_context() function of a uCode
 */
#define _SNC_UCODE_GET_CTX(name)                name##_##get_context

/**
 * \brief Macro abstracting [name]_get_type() function of a uCode
 */
#define _SNC_UCODE_GET_TYPE(name)               name##_##get_type

/**
 * \brief Macro declaring a uCode through its [name]_get_context() and [name]_get_type() functions.
 */
#define _SNC_FUNC_DECL(ucode_type, name, ...)                                                   \
        snc_##ucode_type##_context_t* _SNC_UCODE_GET_CTX(name)(void);                           \
        SNC_UCODE_TYPE _SNC_UCODE_GET_TYPE(name)(void)

/**
 * \brief Macro adding a static space in the execution context of a uCode and defining
 *        a pointer initialized with the starting address of it.
 *
 * A static space is added at the end of the uCode (i.e. the static space footer),
 * and it can be addressed through the defined pointer (i.e. st_type* st_name) which is
 * initialized with the starting address of it.
 *
 * \param [in] st_type          type of the elements addressed by the created pointer
 * \param [in] st_name          name of the static space being created and
 *                              the pointer to its starting address
 * \param [in] st_size          size of the created space
 * \param [in] ... st_content   the content of the created space
 *
 */
#define _SNC_STATIC(st_type, st_name, st_size, ...)                                             \
        st_type* st_name = (void*)&b_ctx->ucode[b_ctx->fs_index];                               \
        if (b_ctx->upd) {                                                                       \
                st_type local_st_name[((uint32_t)st_size) / sizeof(st_type)] = {__VA_ARGS__};   \
                memcpy(st_name, local_st_name, sizeof(local_st_name));                          \
        }                                                                                       \
        b_ctx->fs_index += (((uint32_t)(st_size)) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)

/**
 * \brief Macro adding a temporary/local space in the execution context of a uCode and defining
 *        a pointer initialized with the starting address of it.
 *
 * A temporary/local space is added at the end of the uCode (i.e. the footer), and it can be
 * addressed through the defined pointer (i.e. tmp_type* tmp_name) which is initialized with the
 * starting address of it.
 *
 * \param [in] tmp_type         type of the elements in the created temporary/local space
 * \param [in] tmp_name         name of the temporary space being created and
 *                              the pointer to its starting address
 * \param [in] tmp_size         size of the created space
 *
 * \sa _SNC_TMP_RMV
 *
 */
#define _SNC_TMP_ADD(tmp_type, tmp_name, tmp_size)                                              \
        tmp_type* tmp_name = (void*)&b_ctx->ucode[b_ctx->ft_index];                             \
        b_ctx->ft_index += (((uint32_t)(tmp_size)) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t); \
        if (b_ctx->ft_index > b_ctx->len) b_ctx->len = b_ctx->ft_index

/**
 * \brief Macro removing the previously created temporary space in the execution context of
 *        a uCode.
 *
 * \param [in] tmp_name         name of the temporary space being removed
 *
 * \sa _SNC_TMP_ADD
 *
 */
#define _SNC_TMP_RMV(tmp_name)                                                                  \
        b_ctx->ft_index = b_ctx->body_len + (uint32_t)((uint32_t*)tmp_name - (uint32_t*)&b_ctx->ucode[b_ctx->body_len])

//==================== SeNIS uCode/function definition macros ==================

#define _SNC_UCODE_TYPE_ucode                   SNC_UCODE_TYPE_UCODE_BLOCK
#define _SNC_UCODE_TYPE_func                    SNC_UCODE_TYPE_FUNC
#define _SNC_UCODE_TYPE_main                    SNC_UCODE_TYPE_MAIN

#define _SNC_UCODE_CREATE_ARGS_ucode(...)       __VA_ARGS__
#define _SNC_UCODE_CREATE_ARGS_func(...)        void
#define _SNC_UCODE_CREATE_ARGS_main(...)        void

#define _SNC_UCODE_CREATE_VAL_ucode(...)        __VA_ARGS__
#define _SNC_UCODE_CREATE_VAL_func(...)         0
#define _SNC_UCODE_CREATE_VAL_main(...)         0

typedef void (*ucode_build_func_t)(b_ctx_t* b_ctx, void* args);

void snc_ucode_create(SNC_UCODE_TYPE ucode_type, void* context, uint32_t args_size,
        ucode_build_func_t ucode_build, uint32_t ucode_id);

void snc_ucode_delete(SNC_UCODE_TYPE ucode_type, void* context, uint32_t args_size,
        ucode_build_func_t ucode_build);

#define __SNC_FUNC_DEF0a(ucode_type, name, ...)                                                 \
        struct name##_args_t {

#define __SNC_FUNC_DEF0b(ucode_type, name, ...)                                                 \
        };                                                                                      \
        static void name(b_ctx_t* b_ctx, struct name##_args_t* args);                           \
        static void name##_create(_SNC_UCODE_CREATE_ARGS_##ucode_type(uint32_t ucode_id));      \
        _SNC_RETAINED static snc_##ucode_type##_context_t name##_context;                       \
        static void name##_delete(void) {                                                       \
                return snc_ucode_delete(_SNC_UCODE_TYPE_##ucode_type, &name##_context,          \
                                        sizeof(struct name##_args_t), (ucode_build_func_t)name); \
        }                                                                                       \
        _SNC_RETAINED static snc_##ucode_type##_context_t name##_context;                       \
        snc_##ucode_type##_context_t* _SNC_UCODE_GET_CTX(name)(void) {                          \
                name##_context.ucode_create = name##_create;                                    \
                name##_context.ucode_delete = name##_delete;                                    \
                return &name##_context;                                                         \
        }                                                                                       \
        SNC_UCODE_TYPE _SNC_UCODE_GET_TYPE(name)(void) {                                        \
                return _SNC_UCODE_TYPE_##ucode_type;                                            \
        }                                                                                       \
        static void name##_create(_SNC_UCODE_CREATE_ARGS_##ucode_type(uint32_t ucode_id)) {     \
                return snc_ucode_create(_SNC_UCODE_TYPE_##ucode_type, &name##_context,          \
                                        sizeof(struct name##_args_t), (ucode_build_func_t)name, \
                                        _SNC_UCODE_CREATE_VAL_##ucode_type(ucode_id));          \
        }                                                                                       \
        static void name(b_ctx_t* b_ctx, struct name##_args_t* args)

#define __SNC_FUNC_DEF1(ucode_type, name)                                                       \
                /* no args */
#define __SNC_FUNC_DEF2(ucode_type, name, arg)                                                  \
                arg;
#define __SNC_FUNC_DEF3(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF2(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF4(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF3(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF5(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF4(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF6(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF5(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF7(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF6(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF8(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF7(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF9(ucode_type, name, arg, ...)                                             \
                arg;                                                                            \
        __SNC_FUNC_DEF8(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF10(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF9(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF11(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF10(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF12(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF11(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF13(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF12(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF14(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF13(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF15(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF14(ucode_type, name, __VA_ARGS__)
#define __SNC_FUNC_DEF16(ucode_type, name, arg, ...)                                            \
                arg;                                                                            \
        __SNC_FUNC_DEF15(ucode_type, name, __VA_ARGS__)

/**
 * \brief SNC uCode definition as function with configurable number of attributes.
 *
 * \param [in] ucode_type       the type of the uCode
 * \param [in] name             the name of the uCode
 * \param [in] ... args         (optional) comma separated list of arguments
 *
 * \sa SNC_UCODE_BLOCK_DECL
 * \sa SNC_FUNC_DECL
 * \sa SNC_MAIN_DECL
 *
 */
#define _SNC_FUNC_DEF(ucode_type, name, ...)                                                    \
        __SNC_FUNC_DEF0a(ucode_type, name, ##__VA_ARGS__)                                       \
        __CHOOSE_FLAVOR(name, ##__VA_ARGS__, __SNC_FUNC_DEF16,  __SNC_FUNC_DEF15,               \
                                             __SNC_FUNC_DEF14,  __SNC_FUNC_DEF13,               \
                                             __SNC_FUNC_DEF12,  __SNC_FUNC_DEF11,               \
                                             __SNC_FUNC_DEF10,  __SNC_FUNC_DEF9,                \
                                             __SNC_FUNC_DEF8,   __SNC_FUNC_DEF7,                \
                                             __SNC_FUNC_DEF6,   __SNC_FUNC_DEF5,                \
                                             __SNC_FUNC_DEF4,   __SNC_FUNC_DEF3,                \
                                             __SNC_FUNC_DEF2,   __SNC_FUNC_DEF1                 \
                       )(ucode_type, name, ##__VA_ARGS__)                                       \
        __SNC_FUNC_DEF0b(ucode_type, name, ##__VA_ARGS__)

/*
 * MACRO FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== SeNIS in-Function-uCode -related variables reference functions =

#define _SNC_ARG(var)                                                                           \
        args->var

#define _SNC_UCODE(index)                                                                       \
        b_ctx->ucode[index]

//==================== SeNIS operands in-code configuration functions ==========

#define _SENIS_RDCBI_BIT_POS(bit_pos)                                                           \
        (((bit_pos) & 0x1F) << 23)

#define _SENIS_COBR_LOOP_CNT(cnt)                                                               \
        ((((cnt) & 0x7F)) << 20)

#define __SENIS_1ST_OPERAND(op1, op2)                                                           \
        op1

#define __SENIS_2ND_OPERAND(op1, op2)                                                           \
        op2

#define _SENIS_1ST_OPERAND(...)                                                                 \
        __SENIS_1ST_OPERAND(__VA_ARGS__)

#define _SENIS_2ND_OPERAND(...)                                                                 \
        __SENIS_2ND_OPERAND(__VA_ARGS__)

//==================== Macros used for determining the address type of operands in a SeNIS construct =

#define _SNC_ADDR_IS_REG(addr)  (((uint32_t)(addr) & SENIS_REGS_BASE_ADDRESS) != 0)

#define _SNC_ADDR_IS_MEM(addr)  (((uint32_t)(addr) & SENIS_MEM_BASE_ADDRESS) != 0)

#define _SNC_ADDR_SET_MODE(addr)                                                                \
        ((_SNC_ADDR_IS_REG(addr)) ? SENIS_FLAG_REG : 0)

#define _SNC_ADDR_SET_VALUE(addr)                                                               \
        (((uint32_t)(addr) & 0x7FFFF))

//==================== Macros used for determining the number of operands in a SeNIS construct =

#define ___SNC_OP_TYPE_DA_ONLY_D(op)                                                            \
        op

#define ___SNC_OP_TYPE_DA_ONLY_I(op)                                                            \
        _SNC_OP_TYPE_ERROR( 1)

#define ___SNC_OP_TYPE_IA_ONLY_D(op)                                                            \
        _SNC_OP_TYPE_ERROR( 1)

#define ___SNC_OP_TYPE_IA_ONLY_I(op)                                                            \
        op

#define __SNC_OP_TYPE_DIRECT_ADDRESS_ONLY(op_addr_type, op)                                     \
        ___SNC_OP_TYPE_DA_ONLY_##op_addr_type(op)

#define __SNC_OP_TYPE_INDIRECT_ADDRESS_ONLY(op_addr_type, op)                                   \
        ___SNC_OP_TYPE_IA_ONLY_##op_addr_type(op)

#define __SNC_OP_TYPE_ADDRESS(op_addr_type, op)                                                 \
        SENIS_OPER_TYPE_ADDRESS_##op_addr_type##A, op

#define __SNC_OP_TYPE_VALUE                                                                     \
        SENIS_OPER_TYPE_VALUE,

#define __SNC_OP_TYPE_VALUE_ONLY

#define __SNC_OP_TYPE_IS_ADDRESS(...)                                                           \
        1, 2
#define __SNC_OP_TYPE_IS_VALUE(...)                                                             \
        1

#define __SNC_OP_TYPE(...)                                                                      \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_OP_TYPE_IS_ADDRESS, __SNC_OP_TYPE_IS_VALUE           \
                       )(__VA_ARGS__)

#define ___SNC_OP(...)                                                                          \
        __SNC_OP_TYPE(__VA_ARGS__) __SNC_OP_TYPE

#define __SENIS_OP_TYPE_SEL_ADDRESS_OR_VALUE(...)                                               \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_OP_TYPE_ADDRESS , __SNC_OP_TYPE_VALUE                \
                       )

#define __SENIS_OP_TYPE_SEL_ADDRESS_ONLY(...)                                                   \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_OP_TYPE_ADDRESS , _SNC_OP_TYPE_ERROR( 1)             \
                       )

#define __SENIS_OP_TYPE_SEL_VALUE_ONLY(...)                                                     \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     _SNC_OP_TYPE_ERROR( 2), __SNC_OP_TYPE_VALUE_ONLY           \
                       )

#define __SENIS_OP_TYPE_SEL_DIRECT_ADDRESS_ONLY(...)                                            \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_OP_TYPE_DIRECT_ADDRESS_ONLY, _SNC_OP_TYPE_ERROR( 1)  \
                       )

#define __SENIS_OP_TYPE_SEL_INDIRECT_ADDRESS_ONLY(...)                                          \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_OP_TYPE_INDIRECT_ADDRESS_ONLY, _SNC_OP_TYPE_ERROR( 1) \
                       )

#define __SNC_CAST_OP_TYPE_ADDRESS(...)                                                         \
        __VA_ARGS__

#define __SNC_CAST_OP_TYPE_VALUE(...)                                                           \
        (uint32_t*)(__VA_ARGS__)

#define __SNC_CAST_OP_TYPE_VALUE_ONLY(type, ...)                                                \
        (type)(__VA_ARGS__)

#define __SENIS_CAST_OP_TYPE_SEL_ADDRESS_OR_VALUE(...)                                          \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_CAST_OP_TYPE_ADDRESS, __SNC_CAST_OP_TYPE_VALUE       \
                       )

#define __SENIS_CAST_OP_TYPE_SEL_ADDRESS_ONLY(...)                                              \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     __SNC_CAST_OP_TYPE_ADDRESS, _SNC_OP_TYPE_ERROR( 1)         \
                       )

#define __SENIS_CAST_OP_TYPE_SEL_VALUE_ONLY(...)                                                \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_OP_TYPE_ERROR(16), _SNC_OP_TYPE_ERROR(15),            \
                                     _SNC_OP_TYPE_ERROR(14), _SNC_OP_TYPE_ERROR(13),            \
                                     _SNC_OP_TYPE_ERROR(12), _SNC_OP_TYPE_ERROR(11),            \
                                     _SNC_OP_TYPE_ERROR(10), _SNC_OP_TYPE_ERROR( 9),            \
                                     _SNC_OP_TYPE_ERROR( 8), _SNC_OP_TYPE_ERROR( 7),            \
                                     _SNC_OP_TYPE_ERROR( 6), _SNC_OP_TYPE_ERROR( 5),            \
                                     _SNC_OP_TYPE_ERROR( 4), _SNC_OP_TYPE_ERROR( 3),            \
                                     _SNC_OP_TYPE_ERROR( 2), __SNC_CAST_OP_TYPE_VALUE_ONLY      \
                       )

#define __SNC_OP(opt_type_mode, op_cast_type_sel, op_type_sel)                                  \
        __SENIS_OP_TYPE_SEL_##opt_type_mode(___SNC_OP op_type_sel) op_cast_type_sel

#define _SNC_OP(...)                                                                            \
        __SNC_OP(ADDRESS_OR_VALUE, __SENIS_CAST_OP_TYPE_SEL_##ADDRESS_OR_VALUE(___SNC_OP __VA_ARGS__)(__VA_ARGS__), __VA_ARGS__)

#define _SNC_OP_ADDRESS(...)                                                                    \
        __SNC_OP(ADDRESS_ONLY, __SENIS_CAST_OP_TYPE_SEL_##ADDRESS_ONLY(___SNC_OP __VA_ARGS__)(__VA_ARGS__), __VA_ARGS__)

#define _SNC_OP_DIRECT_ADDRESS(...)                                                             \
        __SNC_OP(DIRECT_ADDRESS_ONLY, __SENIS_CAST_OP_TYPE_SEL_##ADDRESS_ONLY(___SNC_OP __VA_ARGS__)(__VA_ARGS__), __VA_ARGS__)

#define _SNC_OP_INDIRECT_ADDRESS(...)                                                           \
        __SNC_OP(INDIRECT_ADDRESS_ONLY, __SENIS_CAST_OP_TYPE_SEL_##ADDRESS_ONLY(___SNC_OP __VA_ARGS__)(__VA_ARGS__), __VA_ARGS__)

#define _SNC_OP_VALUE(type, ...)                                                                \
        __SNC_OP(VALUE_ONLY, __SENIS_CAST_OP_TYPE_SEL_##VALUE_ONLY(___SNC_OP __VA_ARGS__)(type, __VA_ARGS__), __VA_ARGS__)

//==================== SeNIS main commands (bare) ==============================

#define _SENIS_B_NOP()                                                                          \
        (uint32_t)((SENIS_OP_NOP  <<28))

#define _SENIS_B_WADAD_ID(dst_ind_addr, src_addr)                                               \
        (uint32_t)((SENIS_OP_WADAD<<28) | (_SNC_ADDR_SET_MODE(dst_ind_addr)) | (_SNC_ADDR_SET_VALUE(dst_ind_addr))), \
        (uint32_t)(                       (_SNC_ADDR_SET_MODE(src_addr)) | (_SNC_ADDR_SET_VALUE(src_addr)))

#define _SENIS_B_WADAD_II(dst_ind_addr, src_ind_addr)                                           \
        (uint32_t)((SENIS_OP_WADAD<<28) | (_SNC_ADDR_SET_MODE(dst_ind_addr)) | (_SNC_ADDR_SET_VALUE(dst_ind_addr)) | SENIS_FLAG_InA2), \
        (uint32_t)(                       (_SNC_ADDR_SET_MODE(src_ind_addr)) | (_SNC_ADDR_SET_VALUE(src_ind_addr)))

#define _SENIS_B_WADAD_DD(dst_addr, src_addr)                                                   \
        (uint32_t)((SENIS_OP_WADAD<<28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)) | SENIS_FLAG_DA1), \
        (uint32_t)(                       (_SNC_ADDR_SET_MODE(src_addr)) | (_SNC_ADDR_SET_VALUE(src_addr)))

#define _SENIS_B_WADAD_DI(dst_addr, src_ind_addr)                                               \
        (uint32_t)((SENIS_OP_WADAD<<28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)) | SENIS_FLAG_DA1 | SENIS_FLAG_InA2), \
        (uint32_t)(                       (_SNC_ADDR_SET_MODE(src_ind_addr)) | (_SNC_ADDR_SET_VALUE(src_ind_addr)))

#define _SENIS_B_WADVA_I(dst_ind_addr, value)                                                   \
        (uint32_t)((SENIS_OP_WADVA<<28) | (_SNC_ADDR_SET_MODE(dst_ind_addr)) | (_SNC_ADDR_SET_VALUE(dst_ind_addr))), \
        (uint32_t)(                       value)

#define _SENIS_B_WADVA_D(dst_addr, value)                                                       \
        (uint32_t)((SENIS_OP_WADVA<<28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)) | SENIS_FLAG_DA1), \
        (uint32_t)(                       value)

#define _SENIS_B_TOBRE_D(dst_addr, bit_mask)                                                    \
        (uint32_t)((SENIS_OP_TOBRE<<28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr))), \
        (uint32_t)(                       bit_mask)

#define _SENIS_B_RDCBI_D(addr, bit_pos)                                                         \
        (uint32_t)((SENIS_OP_RDCBI<<28) | (_SNC_ADDR_SET_MODE(addr)) | (_SNC_ADDR_SET_VALUE(addr)) | _SENIS_RDCBI_BIT_POS(bit_pos))

#define _SENIS_B_RDCGR_DD(addr, gt_addr)                                                        \
        (uint32_t)((SENIS_OP_RDCGR<<28) | (_SNC_ADDR_SET_MODE(addr)) | (_SNC_ADDR_SET_VALUE(addr))), \
        (uint32_t)(                       (_SNC_ADDR_SET_MODE(gt_addr)) | (_SNC_ADDR_SET_VALUE(gt_addr)))

#define _SENIS_B_COBR_EQ_D(addr)                                                                \
        (uint32_t)((SENIS_OP_COBR <<28) | (_SNC_ADDR_SET_MODE(addr)) | (_SNC_ADDR_SET_VALUE(addr)) | ((0x0A) << 20))

#define _SENIS_B_COBR_EQ_I(ind_addr)                                                            \
        (uint32_t)((SENIS_OP_COBR <<28) | (_SNC_ADDR_SET_MODE(ind_addr)) | (_SNC_ADDR_SET_VALUE(ind_addr)) | ((0x1A) << 20))

#define _SENIS_B_COBR_GR_D(addr)                                                                \
        (uint32_t)((SENIS_OP_COBR <<28) | (_SNC_ADDR_SET_VALUE(addr)) | ((0x05) << 20))

#define _SENIS_B_COBR_GR_I(ind_addr)                                                            \
        (uint32_t)((SENIS_OP_COBR <<28) | (_SNC_ADDR_SET_VALUE(ind_addr)) | ((0x15) << 20))

#define _SENIS_B_COBR_LOOP_D(addr, cnt)                                                         \
        (uint32_t)((SENIS_OP_COBR <<28) | (_SNC_ADDR_SET_VALUE(addr)) | ((0x80 | ((cnt) & 0x7F)) << 20))

#define _SENIS_B_INC_D(addr)                                                                    \
        (uint32_t)((SENIS_OP_INC  <<28) | (_SNC_ADDR_SET_VALUE(addr)))

#define _SENIS_B_INC4_D(addr)                                                                   \
        (uint32_t)((SENIS_OP_INC  <<28) | (_SNC_ADDR_SET_VALUE(addr)) | SENIS_FLAG_INC4)

#define _SENIS_B_DEL(value)                                                                     \
        (uint32_t)((SENIS_OP_DEL  <<28) + ((value) & 0xFF))

#define _SENIS_B_SLP()                                                                          \
        (uint32_t)((SENIS_OP_SLP  <<28))

//==================== Calling an SNC-function -related macros =================

#define __SNC_FUNC_ARGS0(func_name)                                                             \
                0
#define __SNC_FUNC_ARGS1(func_name, arg)                                                        \
                _SNC_OP(arg)
#define __SNC_FUNC_ARGS2(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS1(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS3(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS2(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS4(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS3(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS5(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS4(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS6(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS5(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS7(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS6(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS8(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS7(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS9(func_name, arg, ...)                                                   \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS8(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS10(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS9(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS11(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS10(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS12(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS11(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS13(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS12(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS14(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS13(func_name, __VA_ARGS__)
#define __SNC_FUNC_ARGS15(func_name, arg, ...)                                                  \
                _SNC_OP(arg),                                                                   \
                __SNC_FUNC_ARGS14(func_name, __VA_ARGS__)

#define _SNC_FUNC_ARGS(func_name, ...)                                                          \
        __CHOOSE_FLAVOR(func_name, ##__VA_ARGS__,                                               \
                                     __SNC_FUNC_ARGS15, __SNC_FUNC_ARGS14,                      \
                                     __SNC_FUNC_ARGS13, __SNC_FUNC_ARGS12,                      \
                                     __SNC_FUNC_ARGS11, __SNC_FUNC_ARGS10,                      \
                                     __SNC_FUNC_ARGS9,  __SNC_FUNC_ARGS8,                       \
                                     __SNC_FUNC_ARGS7,  __SNC_FUNC_ARGS6,                       \
                                     __SNC_FUNC_ARGS5,  __SNC_FUNC_ARGS4,                       \
                                     __SNC_FUNC_ARGS3,  __SNC_FUNC_ARGS2,                       \
                                     __SNC_FUNC_ARGS1,  __SNC_FUNC_ARGS0                        \
                       )(func_name, ##__VA_ARGS__)

#define __SNC_FUNC_NUM_OF_ARGS(func_name, ...)                                                  \
        __CHOOSE_FLAVOR(func_name, ##__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

void senis_call(b_ctx_t* b_ctx, snc_func_context_t* ucode_ctx, uint32_t num_of_args, ...);
#define _SENIS_call(func_name, ...)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        senis_call(b_ctx, SNC_UCODE_CTX(func_name),                                             \
                          2 * __SNC_FUNC_NUM_OF_ARGS(func_name, ##__VA_ARGS__),                 \
                          _SNC_FUNC_ARGS(func_name, ##__VA_ARGS__))

//==================== Returning from an SNC-function -related macros ==========

void senis_return(b_ctx_t* b_ctx);
#define _SENIS_return                                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        senis_return(b_ctx)

//==================== Label definition function ===============================

#define _SENIS_labels(...)                                                                      \
        enum __senis_custom_labels_enum {__VA_ARGS__ , __senis_custom_labels_enum_end};         \
        uint32_t* __senis_custom_labels = &b_ctx->labels[b_ctx->l_upd&b_ctx->l_index];          \
        b_ctx->l_index += __senis_custom_labels_enum_end;

#define _SENIS_label(label)                                                                     \
        __senis_custom_labels[b_ctx->l_upd&label] = _SNC_UCODE_INDX

//==================== SeNIS main functions ====================================

//==================== SeNIS NOP ===============================================

void senis_nop(b_ctx_t* b_ctx);
#define _SENIS_nop                                                                              \
        SNC_STEP_BY_STEP();                                                                     \
        senis_nop(b_ctx)

//==================== SeNIS WADAD =============================================

void senis_wadad(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                 SENIS_OPER_TYPE src_addr_type, uint32_t* src_addr);
#define _SENIS_wadad(dst_addr, src_addr)                                                        \
        SNC_STEP_BY_STEP();                                                                     \
        senis_wadad(b_ctx, _SNC_OP_ADDRESS(dst_addr), _SNC_OP_ADDRESS(src_addr))

//==================== SeNIS WADVA =============================================

void senis_wadva(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                 uint32_t value);
#define _SENIS_wadva(dst_addr, value)                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        senis_wadva(b_ctx, _SNC_OP_ADDRESS(dst_addr), _SNC_OP_VALUE(uint32_t, value))

//==================== SeNIS TOBRE =============================================

void senis_tobre(b_ctx_t* b_ctx, uint32_t* dst_addr, uint32_t bit_mask_value);
#define _SENIS_tobre(dst_addr, bit_mask_value)                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(dst_addr), _SNC_OP_VALUE(uint32_t, bit_mask_value))

//==================== SeNIS RDCBI =============================================

void senis_rdcbi(b_ctx_t* b_ctx, uint32_t* addr, uint32_t bit_pos_value);
#define _SENIS_rdcbi(addr, bit_pos_value)                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(addr), _SNC_OP_VALUE(uint32_t, bit_pos_value))

//==================== SeNIS RDCGR =============================================

void senis_rdcgr(b_ctx_t* b_ctx, uint32_t* addr, uint32_t* gt_addr);
#define _SENIS_rdcgr(addr, gt_addr)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        senis_rdcgr(b_ctx, _SNC_OP_DIRECT_ADDRESS(addr), _SNC_OP_DIRECT_ADDRESS(gt_addr))

//==================== SeNIS COBR_EQ ===========================================

void senis_cobr_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr);
#define _SENIS_cobr_eq(addr)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_cobr_eq(b_ctx, _SNC_OP_ADDRESS(addr))

//==================== SeNIS COBR_GR ===========================================

void senis_cobr_gr(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr);
#define _SENIS_cobr_gr(addr)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_cobr_gr(b_ctx, _SNC_OP_ADDRESS(addr))

//==================== SeNIS COBR_LOOP =========================================

void senis_cobr_loop(b_ctx_t* b_ctx, uint32_t* addr, uint8_t cnt_value);
#define _SENIS_cobr_loop(addr, cnt_value)                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        senis_cobr_loop(b_ctx, _SNC_OP_DIRECT_ADDRESS(addr), _SNC_OP_VALUE(uint8_t, cnt_value))

//==================== SeNIS INC1 ==============================================

void senis_inc1(b_ctx_t* b_ctx, uint32_t* dst_addr);
#define _SENIS_inc1(dst_addr)                                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(dst_addr))

//==================== SeNIS INC4 ==============================================

void senis_inc4(b_ctx_t* b_ctx, uint32_t* dst_addr);
#define _SENIS_inc4(dst_addr)                                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(dst_addr))

//==================== SeNIS DEL ===============================================

void senis_del(b_ctx_t* b_ctx, uint8_t delay_value);
#define _SENIS_del(delay_value)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        senis_del(b_ctx, _SNC_OP_VALUE(uint8_t, delay_value))

//==================== SeNIS SLP ===============================================

void senis_slp(b_ctx_t* b_ctx);
#define _SENIS_slp                                                                              \
        SNC_STEP_BY_STEP();                                                                     \
        senis_slp(b_ctx)

//==================== SeNIS main functions extensions =========================

//==================== SeNIS RDCGR_Z ===========================================

void senis_rdcgr_z(b_ctx_t* b_ctx, uint32_t* addr);
#define _SENIS_rdcgr_z(addr)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_rdcgr_z(b_ctx, _SNC_OP_DIRECT_ADDRESS(addr))

//==================== SeNIS COBR_LOOP_RESET ===================================

void senis_cobr_loop_reset(b_ctx_t* b_ctx);
#define _SENIS_cobr_loop_reset()                                                                \
        SNC_STEP_BY_STEP();                                                                     \
        senis_cobr_loop_reset(b_ctx)

//==================== SeNIS DEL_LP_CLK ========================================

void senis_del_lp_clk(b_ctx_t* b_ctx, uint32_t delay_ticks);
#define _SENIS_del_lp_clk(delay_ticks)                                                          \
        SNC_STEP_BY_STEP();                                                                     \
        senis_del_lp_clk(b_ctx, _SNC_OP_VALUE(uint32_t, delay_ticks))

//==================== SeNIS DEL_MS ============================================

void senis_del_ms(b_ctx_t* b_ctx, uint32_t delay_ms);
#define _SENIS_del_ms(delay_ms)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        senis_del_ms(b_ctx, _SNC_OP_VALUE(uint32_t, delay_ms))

//==================== SeNIS extension for c-like clauses ======================

uint32_t* senis_add_clause_op(b_ctx_t* b_ctx, SENIS_OPER_TYPE op_type, uint32_t* op);

void senis_rmv_clause_op(b_ctx_t* b_ctx, SENIS_OPER_TYPE clause_op_type, uint32_t* clause_op);

//==================== SeNIS ASSIGN ============================================

void senis_assign(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_assign(op1, op2)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        senis_assign(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

//==================== SeNIS XOR ===============================================

void senis_xor(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_xor(op1, op2)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_xor(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

//==================== SeNIS IF-ELSE ===========================================

void senis_if_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk);
void senis_if_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                   senis_ifelse_blk_t ifelse_blk);
void senis_if_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk);
void senis_if_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                   senis_ifelse_blk_t ifelse_blk);
void senis_if_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk);
void senis_if_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                  senis_ifelse_blk_t ifelse_blk);
void senis_if_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                  senis_ifelse_blk_t ifelse_blk);
void senis_if_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                   senis_ifelse_blk_t ifelse_blk);

void senis_if_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   senis_ifelse_blk_t ifelse_blk);
void senis_if_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   senis_ifelse_blk_t ifelse_blk);

void senis_if_true(b_ctx_t* b_ctx, senis_ifelse_blk_t ifelse_blk);
void senis_if_false(b_ctx_t* b_ctx, senis_ifelse_blk_t ifelse_blk);

#define __SENIS_IF_LT(op1, op2, ifelse_blk)                                                     \
        senis_if_lt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_LTEQ(op1, op2, ifelse_blk)                                                   \
        senis_if_lteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_GT(op1, op2, ifelse_blk)                                                     \
        senis_if_gt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_GTEQ(op1, op2, ifelse_blk)                                                   \
        senis_if_gteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_EQ(op1, op2, ifelse_blk)                                                     \
        senis_if_eq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_NEQ(op1, op2, ifelse_blk)                                                    \
        senis_if_neq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), ifelse_blk)
#define __SENIS_IF_BIT(op1, op2, ifelse_blk)                                                    \
        senis_if_bit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), ifelse_blk)
#define __SENIS_IF_NBIT(op1, op2, ifelse_blk)                                                   \
        senis_if_nbit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), ifelse_blk)

#define _SENIS_IF_COND(ifelse_clause_func, op1, cond, op2)                                      \
        auto void ifelse_clause_func(b_ctx_t* b_ctx, senis_blk_t* else_blk_p);                  \
        {                                                                                       \
                SENIS_COND_TYPE dummy_cond __UNUSED = cond;                                     \
                __SENIS_IF_##cond(op1, op2, ifelse_clause_func);                                \
        }                                                                                       \
        void ifelse_clause_func(b_ctx_t* b_ctx, senis_blk_t* else_blk_p)

#define _SENIS_IF(ifelse_clause_func, op)                                                       \
        auto void ifelse_clause_func(b_ctx_t* b_ctx, senis_blk_t* else_blk_p);                  \
        {                                                                                       \
                __SENIS_IF_GT(op, 0, ifelse_clause_func);                                       \
        }                                                                                       \
        void ifelse_clause_func(b_ctx_t* b_ctx, senis_blk_t* else_blk_p)

#define _SENIS_if(...)                                                                          \
        SNC_STEP_BY_STEP();                                                                     \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_NUM_ARGS_ERROR(16), _SNC_NUM_ARGS_ERROR(15),          \
                                     _SNC_NUM_ARGS_ERROR(14), _SNC_NUM_ARGS_ERROR(13),          \
                                     _SNC_NUM_ARGS_ERROR(12), _SNC_NUM_ARGS_ERROR(11),          \
                                     _SNC_NUM_ARGS_ERROR(10), _SNC_NUM_ARGS_ERROR( 9),          \
                                     _SNC_NUM_ARGS_ERROR( 8), _SNC_NUM_ARGS_ERROR( 7),          \
                                     _SNC_NUM_ARGS_ERROR( 6), _SNC_NUM_ARGS_ERROR( 5),          \
                                     _SNC_NUM_ARGS_ERROR( 4), _SENIS_IF_COND,                   \
                                     _SNC_NUM_ARGS_ERROR( 2), _SENIS_IF                         \
                       )(_SNC_APPEND_LINE_NO(ifelse_clause_func), __VA_ARGS__)

#define _SENIS_else                                                                             \
        auto void else_blk(b_ctx_t* b_ctx);                                                     \
        if (else_blk_p) *else_blk_p = else_blk;                                                 \
        void else_blk(b_ctx_t* b_ctx)

//==================== SeNIS WHILE =============================================

void senis_while_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk);
void senis_while_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_while_blk_t while_blk);
void senis_while_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk);
void senis_while_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_while_blk_t while_blk);
void senis_while_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk);
void senis_while_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                     SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                     senis_while_blk_t while_blk);
void senis_while_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                     senis_while_blk_t while_blk);
void senis_while_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                      senis_while_blk_t while_blk);

void senis_while_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      senis_while_blk_t while_blk);
void senis_while_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      senis_while_blk_t while_blk);

void senis_while_true(b_ctx_t* b_ctx, senis_while_blk_t while_blk);

#define __SENIS_WHILE_LT(op1, op2, while_blk)                                                   \
        senis_while_lt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_LTEQ(op1, op2, while_blk)                                                 \
        senis_while_lteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_GT(op1, op2, while_blk)                                                   \
        senis_while_gt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_GTEQ(op1, op2, while_blk)                                                 \
        senis_while_gteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_EQ(op1, op2, while_blk)                                                   \
        senis_while_eq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_NEQ(op1, op2, while_blk)                                                  \
        senis_while_neq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), while_blk)
#define __SENIS_WHILE_BIT(op1, op2, while_blk)                                                  \
        senis_while_bit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), while_blk)
#define __SENIS_WHILE_NBIT(op1, op2, while_blk)                                                 \
        senis_while_nbit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), while_blk)

#define _SENIS_WHILE_COND(while_clause_func, op1, cond, op2)                                    \
        auto void while_clause_func(b_ctx_t* b_ctx);                                            \
        {                                                                                       \
                SENIS_COND_TYPE dummy_cond __UNUSED = cond;                                     \
                __SENIS_WHILE_##cond(op1, op2, while_clause_func);                              \
        }                                                                                       \
        void while_clause_func(b_ctx_t* b_ctx)

#define _SENIS_WHILE(while_clause_func, op)                                                     \
        auto void while_clause_func(b_ctx_t* b_ctx);                                            \
        {                                                                                       \
                __SENIS_WHILE_GT(op, 0, while_clause_func);                                     \
        }                                                                                       \
        void while_clause_func(b_ctx_t* b_ctx)

#define _SENIS_while(...)                                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_NUM_ARGS_ERROR(16), _SNC_NUM_ARGS_ERROR(15),          \
                                     _SNC_NUM_ARGS_ERROR(14), _SNC_NUM_ARGS_ERROR(13),          \
                                     _SNC_NUM_ARGS_ERROR(12), _SNC_NUM_ARGS_ERROR(11),          \
                                     _SNC_NUM_ARGS_ERROR(10), _SNC_NUM_ARGS_ERROR( 9),          \
                                     _SNC_NUM_ARGS_ERROR( 8), _SNC_NUM_ARGS_ERROR( 7),          \
                                     _SNC_NUM_ARGS_ERROR( 6), _SNC_NUM_ARGS_ERROR( 5),          \
                                     _SNC_NUM_ARGS_ERROR( 4), _SENIS_WHILE_COND,                \
                                     _SNC_NUM_ARGS_ERROR( 2), _SENIS_WHILE                      \
                       )(_SNC_APPEND_LINE_NO(while_clause_func), __VA_ARGS__)

//==================== SeNIS DOWHILE ===========================================

void senis_dowhile_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                        senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                       SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                       senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                       senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                        senis_dowhile_blk_t dowhile_blk);

void senis_dowhile_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        senis_dowhile_blk_t dowhile_blk);

void senis_dowhile_true(b_ctx_t* b_ctx, senis_dowhile_blk_t dowhile_blk);
void senis_dowhile_false(b_ctx_t* b_ctx, senis_dowhile_blk_t dowhile_blk);

#define __SENIS_DOWHILE_LT(op1, op2, dowhile_blk)                                               \
        senis_dowhile_lt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_LTEQ(op1, op2, dowhile_blk)                                             \
        senis_dowhile_lteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_GT(op1, op2, dowhile_blk)                                               \
        senis_dowhile_gt(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_GTEQ(op1, op2, dowhile_blk)                                             \
        senis_dowhile_gteq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_EQ(op1, op2, dowhile_blk)                                               \
        senis_dowhile_eq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_NEQ(op1, op2, dowhile_blk)                                              \
        senis_dowhile_neq(b_ctx, _SNC_OP(op1), _SNC_OP(op2), dowhile_blk)
#define __SENIS_DOWHILE_BIT(op1, op2, dowhile_blk)                                              \
        senis_dowhile_bit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), dowhile_blk)
#define __SENIS_DOWHILE_NBIT(op1, op2, dowhile_blk)                                             \
        senis_dowhile_nbit(b_ctx, _SNC_OP(op1), _SNC_OP_VALUE(uint32_t, op2), dowhile_blk)

#define _SENIS_DOWHILE_COND(dowhile_clause_func, op1, cond, op2)                                \
        auto void dowhile_clause_func(b_ctx_t* b_ctx);                                          \
        {                                                                                       \
                SENIS_COND_TYPE dummy_cond __UNUSED = cond;                                     \
                __SENIS_DOWHILE_##cond(op1, op2, dowhile_clause_func);                          \
        }                                                                                       \
        void dowhile_clause_func(b_ctx_t* b_ctx)

#define _SENIS_DOWHILE(dowhile_clause_func, op)                                                 \
        auto void dowhile_clause_func(b_ctx_t* b_ctx);                                          \
        {                                                                                       \
                __SENIS_DOWHILE_GT(op, 0, dowhile_clause_func);                                 \
        }                                                                                       \
        void dowhile_clause_func(b_ctx_t* b_ctx)

#define _SENIS_dowhile(...)                                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        __CHOOSE_FLAVOR(__VA_ARGS__, _SNC_NUM_ARGS_ERROR(16), _SNC_NUM_ARGS_ERROR(15),          \
                                     _SNC_NUM_ARGS_ERROR(14), _SNC_NUM_ARGS_ERROR(13),          \
                                     _SNC_NUM_ARGS_ERROR(12), _SNC_NUM_ARGS_ERROR(11),          \
                                     _SNC_NUM_ARGS_ERROR(10), _SNC_NUM_ARGS_ERROR( 9),          \
                                     _SNC_NUM_ARGS_ERROR( 8), _SNC_NUM_ARGS_ERROR( 7),          \
                                     _SNC_NUM_ARGS_ERROR( 6), _SNC_NUM_ARGS_ERROR( 5),          \
                                     _SNC_NUM_ARGS_ERROR( 4), _SENIS_DOWHILE_COND,              \
                                     _SNC_NUM_ARGS_ERROR( 2), _SENIS_DOWHILE                    \
                       )(_SNC_APPEND_LINE_NO(dowhile_clause_func), __VA_ARGS__)

//==================== SeNIS GOTO ==============================================

void senis_goto(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr);
#define _SENIS_goto(addr)                                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        senis_goto(b_ctx, _SNC_OP_ADDRESS(addr))

//==================== SeNIS ADD ===============================================

void senis_add(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_add(op1, op2)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_add(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

//==================== SeNIS SUB ===============================================

void senis_sub(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_sub(op1, op2)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_sub(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

//==================== SeNIS RSHIFT ============================================

void senis_rshift(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_rshift(op1, op2)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        senis_rshift(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

void senis_rshift_masked(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                         SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                         SENIS_OPER_TYPE bit_msk_len_type, uint32_t* bit_msk_len);
#define _SENIS_rshift_masked(op1, op2, bit_msk_len)                                             \
        SNC_STEP_BY_STEP();                                                                     \
        senis_rshift_masked(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2), _SNC_OP(bit_msk_len))

//==================== SeNIS LSHIFT ============================================

void senis_lshift(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2);
#define _SENIS_lshift(op1, op2)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        senis_lshift(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2))

void senis_lshift_masked(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                         SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                         SENIS_OPER_TYPE bit_msk_len_type, uint32_t* bit_msk_len);
#define _SENIS_lshift_masked(op1, op2, bit_msk_len)                                             \
        SNC_STEP_BY_STEP();                                                                     \
        senis_lshift_masked(b_ctx, _SNC_OP_ADDRESS(op1), _SNC_OP(op2), _SNC_OP(bit_msk_len))

//==================== SeNIS BREAK =============================================

void senis_break(b_ctx_t* b_ctx);
#define _SENIS_break                                                                            \
        SNC_STEP_BY_STEP();                                                                     \
        senis_break(b_ctx)

//==================== SeNIS CONTINUE ==========================================

void senis_continue(b_ctx_t* b_ctx);
#define _SENIS_continue                                                                         \
        SNC_STEP_BY_STEP();                                                                     \
        senis_continue(b_ctx)

//==================== SeNIS COPY ==============================================

void senis_copy(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                SENIS_OPER_TYPE src_addr_type, uint32_t* src_addr,
                                SENIS_OPER_TYPE len_type, uint32_t* len);
#define _SENIS_copy(dst_addr, src_addr, len)                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        senis_copy(b_ctx, _SNC_OP_ADDRESS(dst_addr), _SNC_OP_ADDRESS(src_addr), _SNC_OP(len))

//==================== SeNIS COMPARE ===========================================

void senis_compare(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr,
                                   SENIS_OPER_TYPE cmp_addr_type, uint32_t* cmp_addr,
                                   SENIS_OPER_TYPE len_type, uint32_t* len,
                                   SENIS_OPER_TYPE rtn_addr_type, uint32_t* rtn_addr);
#define _SENIS_compare(addr, cmp_addr, len, rtn_addr)                                           \
        SNC_STEP_BY_STEP();                                                                     \
        senis_compare(b_ctx, _SNC_OP_ADDRESS(addr), _SNC_OP_ADDRESS(cmp_addr),                  \
                             _SNC_OP(len), _SNC_OP_ADDRESS(rtn_addr))

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SENIS_MACROS_H_ */

/**
 * \}
 * \}
 */
