/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup POWER_MANAGER Power Manager Service
 *
 * \brief Power Manager
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_power_mgr.h
 *
 * @brief Power Manager header file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_POWER_MGR_H_
#define SYS_POWER_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#ifdef OS_FREERTOS
#include "osal.h"
#endif

typedef enum {
        pm_mode_active = 0,
        pm_mode_idle,
        pm_mode_extended_sleep,
        pm_mode_deep_sleep,
        pm_mode_hibernation,
        pm_mode_sleep_max,
} sleep_mode_t;

typedef enum {
        pm_sys_wakeup_mode_slow,
        pm_sys_wakeup_mode_fast,
        pm_sys_wakeup_mode_ultra_fast,
} sys_wakeup_mode_t;


#ifdef OS_FREERTOS

typedef void (*periph_init_cb)(void);
typedef int32_t pm_id_t;

typedef struct _adptCB {
        bool (*ad_prepare_for_sleep)(void);
        void (*ad_sleep_canceled)(void);
        void (*ad_wake_up_ind)(bool);
        void (*ad_xtalm_ready_ind)(void);
        uint8_t ad_sleep_preparation_time;
} adapter_call_backs_t;


/*
 * Variables declarations
 */


/**
 * Initialization tree node.
 */
typedef void (*comp_init_func)(void *);

typedef struct comp_init_tree {
        comp_init_func init_fun;                     /**< Initialization function */
        void *init_arg;                              /**< Argument for init_fun */
        const struct comp_init_tree * const *depend; /**< List of nodes this node depends on */
} comp_init_tree_t;

/**
 * \brief Component initialization declaration
 *
 * This macro declares component that depends on arbitrary number of other components.
 *
 * param [in] _comp Name of component, this name can be used in other component declarations
 *                  dependency.
 * param [in] _init Function to call during initialization of component.
 * param [in] _deps NULL terminated array of components that should be initialized before.
 */
#define COMPONENT_INIT_WITH_DEPS(_comp, _init, _init_arg, _deps, _sect) \
        __USED \
        const comp_init_tree_t _comp = { _init, (void *)_init_arg, _deps }; \
        __USED \
        const comp_init_tree_t *const _comp##_ptr __attribute__((section (#_sect "_init_section"))) = &_comp; \

/**
 * \brief bus initialization declaration
 *
 * param [in] _id bus id
 * param [in] _init initialization function
 * param [in] _init_arg argument to pass to _init function during bus initialization
 */
#define BUS_INIT(_id, _init, _init_arg) \
        COMPONENT_INIT_WITH_DEPS(_id, (comp_init_func)_init, _init_arg, NULL, bus)

/**
 * \brief device initialization declaration
 *
 * param [in] _id device id
 * param [in] _init initialization function
 * param [in] _init_arg argument to pass to _init function during device initialization
 */
#define DEVICE_INIT(_id, _init, _init_arg) \
        COMPONENT_INIT_WITH_DEPS(_id, (comp_init_func)_init, _init_arg, NULL, device)

#define ADAPTER_INIT_WITH_DEPS(_adapter, _init, _deps) \
        COMPONENT_INIT_WITH_DEPS(_adapter, (comp_init_func)_init, NULL, _deps, adapter)

/**
 * \brief Adapter initialization declaration
 *
 * This macro declares adapter that does not depend on any other adapters. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 */
#define ADAPTER_INIT(_adapter, _init) \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, NULL)

/**
 * \brief Declaration of adapter with one dependency
 *
 * This macro declares adapter that depends on other adapter. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 * param [in] _dep1 Adapter that must be initialized before.
 */
#define ADAPTER_INIT_DEP1(_adapter, _init, _dep1) \
        extern const comp_init_tree_t _dep1; \
        __USED \
        const comp_init_tree_t *const _adapter##_dep[2] = { &_dep1, NULL }; \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, _adapter##_dep)

/**
 * \brief Declaration of adapter that depends on other two
 *
 * This macro declares adapter that depends on two other adapters. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 * param [in] _dep1 Adapter that must be initialized before.
 * param [in] _dep2 Adapter that must be initialized before.
 *
 * \note: Order of dependencies is undefined, if there is dependency between _dep2 and _dep1
 *        it should be specified in respective adapter declaration.
 */
#define ADAPTER_INIT_DEP2(_adapter, _init, _dep1, _dep2) \
        extern const comp_init_tree_t _dep1; \
        extern const comp_init_tree_t _dep2; \
        __USED \
        const comp_init_tree_t *_adapter##_dep[3] = { &_dep1, &_dep2, NULL }; \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, _adapter##_dep)

/*
 * Function declarations
 */

/**
 * \brief Initialize the system after power-up.
 *
 * \param[in] peripherals_initialization Callback to an application function that handles
 *            the initialization of the GPIOs and the peripherals.
 *
 * \warning This function will change when the initialization of the GPIOs and the peripherals is
 *          moved to the Adapters (or wherever it is decided).
 */
void pm_system_init(periph_init_cb peripherals_initialization);

/**
 * \brief Wait for the debugger to detach if sleep is used.
 *
 * \param[in] mode The sleep mode of the application. It must be different than pm_mode_active if
 *            the application intends to use sleep.
 */
void pm_wait_debugger_detach(sleep_mode_t mode);


/**
 * \brief Sets the wake-up mode of the system.
 *
 * The wake-up mode specifies if the system will wait upon wake-up for the system clock to
 * be switched to the appropriate clock source before continuing with code execution.
 * If system clock is RC32M then the wake-up mode has no effect since the system always
 * wake-up using RC32M as system clock.
 *
 * \param[in] wait_for_xtalm When false, the system will start executing code immediately
 *                           after wake-up.
 *                           When true, upon wake-up the system will wait for the system
 *                           clock to be switched to the appropriate clock source before
 *                           continuing with code execution.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_set_wakeup_mode(bool wait_for_xtalm);

/**
 * \brief Returns the wake-up mode of the system (whether the OS will be resumed with RC or XTAL).
 *
 * \return The wake-up mode.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
bool pm_get_wakeup_mode(void);

/**
 * \brief Sets the generic sleep mode of the system. The sleep mode can be temporarily bypassed using
 *        the pm_sleep_mode_request function. In all cases, the priority of the sleep mode (considering 'active'
 *        as the highest and 'hibernation' the lowest priority) defines which will be the current system sleep mode.
 *
 * \param[in] mode the sleep mode to be set
 *
 * \return the previous sleep mode
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 * \sa pm_sleep_mode_request
 *
 */
sleep_mode_t pm_sleep_mode_set(sleep_mode_t mode);

/**
 * \brief Returns the sleep mode of the system.
 *
 * \return The sleep mode used.
 *
 */
sleep_mode_t pm_sleep_mode_get(void);

/**
 * \brief Requests the system to apply the desired sleep mode.
 *        The request is evaluated by the Power Manager and the desired mode is applied, following a strict
 *        priority between the requested modes. A higher-priority sleep mode (eg. pm_mode_active) is never affected by
 *        calls to switch the system to idle or any lower mode and so on and so forth. The same priority rules are applied
 *        to pm_sleep_mode_set calls.
 *
 * \param[in] mode the desired sleep mode
 *
 * \warning The PM API user MUST ensure that any request is matched by the respective release.
 *          Otherwise the system will reach an error-state!
 *
 */
void pm_sleep_mode_request(sleep_mode_t mode);

/**
 * \brief Restores the sleep mode of the system. It terminates a matching request.
 *        Under conditions the system will return to the sleep mode it was before the request was called \sa pm_sleep_mode_request.
 *
 * \param[in] mode the sleep mode the system was requested to go to, when the matching pm_sleep_mode_request was called.
 *            This is NOT the sleep mode the system will go to.
 *
 * \warning This function MUST be called always to terminate a matching pm_sleep_mode_request.
 *          If called alone the system will reach an error-state!
 *
 */
void pm_sleep_mode_release(sleep_mode_t mode);

/**
 * \brief Registers an Adapter to the Power Manager.
 *
 * \param[in] cb The pointer to the set of the call-back functions of this Adapter.
 *
 * \return pm_id_t The registered Adapter's ID in the Power Manager state table.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
pm_id_t pm_register_adapter(const adapter_call_backs_t *cb);

/**
 * \brief Unregisters an Adapter with a specific ID from the Power Manager.
 *
 * \param[in] id The ID of the Adapter in the Power Manager.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_unregister_adapter(pm_id_t id);


/**
 * \brief Called by an Adapter to ask the PM not to go to sleep for some short period.
 *
 * \param[in] id The ID of the Adapter.
 * \param[in] time_in_LP_cycles The offset from the current system time, in (non-prescaled) Low
 *            Power clock cycles, until when the caller requests the system to stay active.
 *
 * \warning Called from Interrupt Context! Must be called with ALL interrupts disabled!
 *
 */
void pm_defer_sleep_for(pm_id_t id, uint32_t time_in_LP_cycles);

/*
 * \brief Put the system to idle or sleep or block in a WFI() waiting for the next tick, if neither
 *        idle nor sleep is possible.
 *
 * \detail Puts the system to idle or sleep, if possible. If an exit-from-idle or a wake-up is
 *         needed, it programs the Timer1 to generate an interrupt after the specified idle or sleep
 *         period. Else, the system stays forever in idle or sleep mode.
 *         If neither idle nor sleep is possible, it blocks in a WFI() call waiting for the next
 *         (already programmed) OS tick to hit.
 *
 * \param[in] low_power_periods The number of (prescaled) low power clock periods the OS will be
 *            idle. If it is 0 then the OS indicates that it can block forever waiting for an
 *            external event. If the system goes to sleep, then it can wake up only from an external
 *            event in this case.
 *
 * \warning Must be called with interrupts disabled!
 *
 */
__RETAINED_CODE void pm_sleep_enter(uint32_t low_power_periods);

#endif /* OS_FREERTOS */


/*
 * \brief Block in a WFI() waiting for the next tick.
 *
 * \detail Blocks in a WFI() call waiting for the next (already programmed) OS tick to hit.
 *
 */
__RETAINED_CODE void pm_execute_wfi(void);

/**
 * \brief Sets the wake-up mode of the system.
 *
 * \param[in] mode This is the new wake-up mode.
 *
 */
void pm_set_sys_wakeup_mode(sys_wakeup_mode_t mode);

/**
 * \brief Returns the wake-up mode of the system.
 *
 * \return The wake-up mode used.
 *
 */
sys_wakeup_mode_t pm_get_sys_wakeup_mode(void);

/**
 * \brief Returns the number of LP clock cycles needed for wake-up.
 *
 * The number of cycles depend on the wake-up mode set by pm_set_sys_wakeup_mode().
 *
 * \return The number of LP clock cycles needed for wake-up.
 *
 */
uint8_t pm_get_sys_wakeup_cycles(void);

/**
 * \brief Prepare system for sleep
 *
 * \param[in] sleep_mode The sleep mode.
 */
__RETAINED_CODE void pm_prepare_sleep(sleep_mode_t sleep_mode);

/**
 * \brief Perform initialization after wake-up.
 *
 * \note This function is called before interrupts are enabled.
 */
__RETAINED_CODE void pm_resume_from_sleep(void);


#endif /* SYS_POWER_MGR_H_ */

/**
 \}
 \}
 */
