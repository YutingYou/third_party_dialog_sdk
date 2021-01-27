/**
 ****************************************************************************************
 *
 * @file sys_charger_da1469x.c
 *
 * @brief System Charger
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup SYS_CHARGER
 \{
 */

#if (dg_configUSE_SYS_CHARGER == 1)

#ifndef SYS_CHARGER_MAX_QUEUE_SIZE
#define SYS_CHARGER_MAX_QUEUE_SIZE      (16)
#endif

#include "sys_charger.h"
#include "sys_usb.h"
#include "sys_usb_da1469x_internal.h"
#include "osal.h"
#include "hw_usb_charger.h"
#include "hw_pmu.h"

/************************************** Private definitions ***************************************/

#define SYS_CHARGER_SW_FSM_DCD_DEBOUNCE_PERIOD         10       /* 100ms */
#define SYS_CHARGER_SW_FSM_DCD_TIMEOUT                 60       /* 600ms */
#define SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN     5       /*  50ms */
#define SYS_CHARGER_SW_FSM_10ms_SAFE_READOUT_MARGIN     1       /*  10ms */
#define SYS_CHARGER_20ms_SAFE_READOUT_MARGIN           20       /*  20ms */

/************************************** Private types *********************************************/

/*
 * List of exchanged messages between tasks / ISR's
 */
typedef enum {
        SYS_CHARGER_MSG_VBUS_UNKNOWN            = 0,            /* VBUS state is unknown */
        SYS_CHARGER_MSG_VBUS_ATTACH             = (1 << 1),     /* VBUS is attached  */
        SYS_CHARGER_MSG_VBUS_DETACH             = (1 << 2),     /* VBUS is detached */
        SYS_CHARGER_MSG_DCD_TRUE                = (1 << 3),     /* Data contact is detected */
        SYS_CHARGER_MSG_DCD_FALSE               = (1 << 4),     /* Data contact is not detected */

        SYS_CHARGER_MSG_START_SW_FSM            = (1 << 5),     /* Start the SW FSM */
        SYS_CHARGER_MSG_STOP_SW_FSM             = (1 << 6),     /* Stop the SW FSM */

        SYS_CHARGER_MSG_KICK_SW_FSM             = (1 << 7),     /* Kick the 10ms counter of the SW FSM */

        SYS_CHARGER_MSG_USB_ENUMERATED          = (1 << 8),     /* Enumeration is completed */
        SYS_CHARGER_MSG_USB_SUSPENDED           = (1 << 9),     /* USB is suspended */
        SYS_CHARGER_MSG_USB_RESUMED             = (1 << 10),    /* USB is resumed */
} SYS_CHARGER_MSG_STAT;

/*
 * List of SW FSM states
 */
typedef enum {
        SYS_CHARGER_SW_FSM_STATE_IDLE = 0,              /* Idle, SW FSM is suspended. */
        SYS_CHARGER_SW_FSM_STATE_ATTACHED,              /* Attached */
        SYS_CHARGER_SW_FSM_STATE_DCD,                   /* Data Contact Detection */
        SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION,     /* Primary contact detection SDP or DCP / CDP */
        SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION,   /* Secondary contact detection DCP or CDP */
        SYS_CHARGER_SW_FSM_STATE_SDP                    /* SDP */
} SYS_CHARGER_SW_FSM_STATE;

/************************************** Forward Declarations **************************************/

static void sys_charger_shallow_copy_configuration(const sys_charger_configuration_t* conf);
static void sys_charger_kick_sw_fsm_timer_cb(OS_TIMER timer);
static void sys_charger_hw_fsm_ok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_OK status);
static void sys_charger_hw_fsm_nok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_NOK status);
static void sys_charger_ok_task(void* pvParameters);
static void sys_charger_nok_task(void* pvParameters);
static void sys_charger_kick_sw_fsm_task(void* pvParameters);
static void sys_charger_program_hw_fsm(void);
static void sys_charger_start_hw_fsm(void);
static void sys_charger_stop_hw_fsm(void);

/************************************** Private variables *****************************************/

/************************************** OS handlers ***********************************************/

__RETAINED static OS_TASK sys_charger_kick_sw_fsm_task_h;
__RETAINED static OS_TASK sys_charger_ok_task_h;
__RETAINED static OS_TASK sys_charger_nok_task_h;
__RETAINED static OS_TIMER sys_charger_kick_sw_fsm_timer_h;
__RETAINED static OS_QUEUE sys_charger_ok_task_msg_queue;

/************************************** Housekeeping variables ************************************/

__RETAINED static const sys_charger_configuration_t* sys_charger_configuration;

static void sys_charger_kick_sw_fsm_timer_cb(OS_TIMER timer)
{
        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_KICK_SW_FSM, OS_NOTIFY_SET_BITS);
}

/************************************** ISR callbacks *********************************************/

static void sys_charger_hw_fsm_ok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_OK status)
{
        HW_CHARGER_MAIN_FSM_STATE fsm_state;

        hw_charger_clear_ok_irq();
        fsm_state = hw_charger_get_main_fsm_state();

        OS_ASSERT(OS_QUEUE_OK == OS_QUEUE_PUT_FROM_ISR(sys_charger_ok_task_msg_queue, &fsm_state));
}

static void sys_charger_hw_fsm_nok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_NOK status)
{
        hw_charger_clear_nok_irq();

        /* Process only the events the charging profile is interested in. */
        status &= hw_charger_get_nok_irq_mask();

        OS_TASK_NOTIFY_FROM_ISR(sys_charger_nok_task_h, status, OS_NOTIFY_SET_BITS);
}

/************************************** sys_usb hooks *********************************************/

void sys_usb_int_charger_hook_attach(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_VBUS);

        /* Verify that the Workaround for "Errata issue 281": Charger Detect Circuit erroneous if V30 setting not at 3.3V
         * has been applied. */
        HW_PMU_3V0_RAIL_CONFIG rail_config;
        OS_ASSERT(hw_pmu_get_3v0_onwakeup_config(&rail_config) == POWER_RAIL_ENABLED);
        OS_ASSERT(rail_config.voltage == HW_PMU_3V0_VOLTAGE_3V3);

        hw_usb_charger_start_contact_detection();

        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_START_SW_FSM, OS_NOTIFY_SET_BITS);
}

void sys_usb_int_charger_hook_detach(void)
{
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_VBUS);

        sys_charger_stop_hw_fsm();

        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_STOP_SW_FSM, OS_NOTIFY_SET_BITS);
}

void sys_usb_int_charger_hook_ch_event(void)
{
        SYS_CHARGER_MSG_STAT value;

        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT);

        uint32_t usb_charger_stat = hw_usb_charger_get_charger_status();

        if (hw_usb_charger_has_data_pin_contact_detected(usb_charger_stat)) {
                value = SYS_CHARGER_MSG_DCD_TRUE;

        } else {
                value = SYS_CHARGER_MSG_DCD_FALSE;
        }

        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, value, OS_NOTIFY_SET_BITS);

        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT)
}

void sys_usb_int_charger_hook_suspend_event(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_SUS);
        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_SUSPENDED, OS_NOTIFY_SET_BITS);
}

void sys_usb_int_charger_hook_resume_event(void)
{
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_SUS);
        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_RESUMED, OS_NOTIFY_SET_BITS);
}

void sys_usb_charger_enumeration_done(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_ENUM_DONE);
        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_ENUMERATED, OS_NOTIFY_SET_BITS);
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_ENUM_DONE);
}

/************************************** Processing Tasks ******************************************/

static void sys_charger_ok_task(void* pvParameters)
{
        HW_CHARGER_MAIN_FSM_STATE fsm_state;
        OS_BASE_TYPE              res;

                do {
                        res = OS_QUEUE_GET(sys_charger_ok_task_msg_queue, &fsm_state, portMAX_DELAY);

                        if (res == OS_QUEUE_EMPTY) {
                                continue;
                        }

                        switch (fsm_state) {

                        case HW_CHARGER_MAIN_FSM_STATE_POWER_UP:
                        case HW_CHARGER_MAIN_FSM_STATE_INIT:
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_DISABLED:
                                sys_charger_ext_hook_hw_fsm_disabled();
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_PRE_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_PRE_CH);
                                sys_charger_ext_hook_precharging();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_PRE_CH);
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_CC_CHARGE:
                        case HW_CHARGER_MAIN_FSM_STATE_CV_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH);
                                sys_charger_ext_hook_charging();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH);
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_END_OF_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_EOC);
                                sys_charger_ext_hook_charged();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_EOC);
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_TDIE_PROT:
                        case HW_CHARGER_MAIN_FSM_STATE_TBAT_PROT:
                        case HW_CHARGER_MAIN_FSM_STATE_BYPASSED:
                        case HW_CHARGER_MAIN_FSM_STATE_ERROR:
                                break;
                        default:
                                /* We should not reach here. */
                                OS_ASSERT(0);
                        }
                } while (1);
}

static void sys_charger_nok_task(void* pvParameters)
{
        HW_CHARGER_FSM_IRQ_STAT_NOK status;

        do {
                OS_TASK_NOTIFY_WAIT(0x0,
                                    OS_TASK_NOTIFY_ALL_BITS,
                                    (uint32_t *)&status,
                                    OS_TASK_NOTIFY_FOREVER);

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TBAT_ERROR)) {
                        sys_charger_ext_hook_tbat_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TDIE_ERROR)) {
                        sys_charger_ext_hook_tdie_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(VBAT_OVP_ERROR)) {
                        sys_charger_ext_hook_ovp_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TOTAL_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_total_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CV_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_cv_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CC_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_cc_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(PRECHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_pre_charge_timeout();
                }

        } while (1);
}

static void sys_charger_kick_sw_fsm_task(void* pvParameters)
{
        SYS_CHARGER_SW_FSM_STATE state;

        uint32_t ulNotifiedValue;
        uint32_t tick_cntr = 0;         /* 10ms per tick */
        uint32_t dcd_cntr = 0;          /* data contact detection counter */
        bool dcd_result = false;        /* data contact detection result */
        state = SYS_CHARGER_SW_FSM_STATE_IDLE;

        do {
                OS_TASK_NOTIFY_WAIT(0x0,
                                    OS_TASK_NOTIFY_ALL_BITS,
                                    &ulNotifiedValue,
                                    OS_TASK_NOTIFY_FOREVER);

                if (ulNotifiedValue & SYS_CHARGER_MSG_KICK_SW_FSM) {
                        tick_cntr++;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_START_SW_FSM) {
                        tick_cntr = 0;
                        dcd_result = false;
                        state = SYS_CHARGER_SW_FSM_STATE_ATTACHED;
                        OS_TIMER_RESET(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_STOP_SW_FSM) {
                        state = SYS_CHARGER_SW_FSM_STATE_IDLE;

                        OS_TIMER_STOP(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_DCD_TRUE) {
                        dcd_result = true;
                        dcd_cntr = tick_cntr;
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_DCD_FALSE) {
                        dcd_result = false;
                        dcd_cntr = UINT32_MAX;
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_ENUMERATED) {
                        /* Enumeration is done, we are good to apply the requested current level. */
                        hw_charger_set_const_current_level(sys_charger_configuration->hw_charging_profile.cc_level);
                        continue;

                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_SUSPENDED) {
                        sys_charger_stop_hw_fsm();
                        continue;

                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_RESUMED) {
                        sys_charger_start_hw_fsm();
                        continue;
                }

                switch (state) {

                case SYS_CHARGER_SW_FSM_STATE_ATTACHED:
                        state = SYS_CHARGER_SW_FSM_STATE_DCD;
                        break;
                case SYS_CHARGER_SW_FSM_STATE_DCD:
                        if ((dcd_result && (tick_cntr > (dcd_cntr + SYS_CHARGER_SW_FSM_DCD_DEBOUNCE_PERIOD))) ||
                             (tick_cntr >  SYS_CHARGER_SW_FSM_DCD_TIMEOUT)) {
                                hw_usb_program_usb_cancel_irq();
                                hw_usb_charger_start_primary_detection();
                                state = SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION;
                                tick_cntr = 0;
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN) {
                                tick_cntr = 0;
                                HW_USB_CHARGER_PRIMARY_CONN_TYPE prim_con_type = hw_usb_charger_get_primary_detection_result();
                                /* CDP and DCP result are mapped on the same value. So only a single
                                 * comparison would be enough. Added here for clarity reasons only.
                                 */
                                if ((prim_con_type == HW_USB_CHARGER_PRIMARY_CONN_TYPE_CDP) ||
                                    (prim_con_type == HW_USB_CHARGER_PRIMARY_CONN_TYPE_DCP)) {

                                        hw_usb_charger_start_secondary_detection();
                                        state = SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION;
                                } else {
                                        hw_usb_charger_disable_detection();
                                        state = SYS_CHARGER_SW_FSM_STATE_SDP;
                                }
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN) {
                                tick_cntr = 0;
                                HW_USB_CHARGER_SECONDARY_CONN_TYPE sec_con_type = hw_usb_charger_get_secondary_detection_result();
                                if (sec_con_type == HW_USB_CHARGER_SECONDARY_CONN_TYPE_CDP) { /* min 1500 mA */
                                        /* Ready for enumeration if needed so. */
                                        sys_usb_finalize_attach();

                                } else if (sec_con_type == HW_USB_CHARGER_SECONDARY_CONN_TYPE_DCP) { /* min 500 mA */
                                        hw_usb_charger_set_dp_high();
                                } else {
                                        /* We should not reach here. */
                                        OS_ASSERT(0);
                                }
                                hw_usb_charger_disable_detection();
                                state = SYS_CHARGER_SW_FSM_STATE_IDLE;
                                sys_charger_program_hw_fsm();
                                sys_charger_start_hw_fsm();
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_SDP:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_10ms_SAFE_READOUT_MARGIN) {
                                HW_CHARGER_I_LEVEL cc_level;
                                sys_charger_program_hw_fsm();
                                cc_level = hw_charger_get_const_current_level();
                                /* Override the programmed CC level if needed.
                                 * JEITA CC values for warm/cool should be lower anyway by spec.
                                 */
                                if (cc_level >= HW_CHARGER_I_LEVEL_100) {
                                        hw_charger_set_const_current_level(HW_CHARGER_I_LEVEL_90);
                                }

                                /* Should be appeared as connected to be able to draw 100mA.
                                 * For up to 500mA enumeration is expected to update the new CC level.
                                 */

                                /* Ready for enumeration if needed so. */
                                sys_usb_finalize_attach();

                                state = SYS_CHARGER_SW_FSM_STATE_IDLE;
                                sys_charger_start_hw_fsm();
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_IDLE:
                        OS_TIMER_STOP(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        break;

                } /* switch */
        } while (1);
}

/************************************** Helper Functions ******************************************/

static void sys_charger_shallow_copy_configuration(const sys_charger_configuration_t* conf)
{
        sys_charger_configuration = conf;
}

static void sys_charger_program_hw_fsm(void)
{
        hw_charger_program_charging_profile(&sys_charger_configuration->hw_charging_profile);
}

static void sys_charger_start_hw_fsm(void)
{
        hw_charger_enable_fsm_ok_interrupt(sys_charger_hw_fsm_ok_isr_cb);
        hw_charger_enable_fsm_nok_interrupt(sys_charger_hw_fsm_nok_isr_cb);
        hw_charger_set_clock_mode(true);
        hw_charger_set_analog_circuitry_operating_mode(true);
        hw_charger_set_fsm_operating_mode(true);
}

static void sys_charger_stop_hw_fsm(void)
{
        hw_charger_disable_fsm_ok_interrupt();
        hw_charger_disable_fsm_nok_interrupt();
        hw_charger_set_analog_circuitry_operating_mode(false);
        hw_charger_set_fsm_operating_mode(false);
}

void sys_charger_init(const sys_charger_configuration_t* conf)
{
        OS_ASSERT(conf);

        sys_charger_shallow_copy_configuration(conf);

        OS_TASK_CREATE("CH_OK",                         /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_ok_task,            /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 2,   /* The priority assigned to the task. */
                        sys_charger_ok_task_h);         /* The task handle */

        OS_ASSERT(sys_charger_ok_task_h);

        OS_QUEUE_CREATE(sys_charger_ok_task_msg_queue, sizeof(HW_CHARGER_MAIN_FSM_STATE), SYS_CHARGER_MAX_QUEUE_SIZE);
        ASSERT_ERROR(sys_charger_ok_task_msg_queue != NULL);

        OS_TASK_CREATE("CH_NOK",                        /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_nok_task,           /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 2,   /* The priority assigned to the task. */
                        sys_charger_nok_task_h);        /* The task handle */

        OS_ASSERT(sys_charger_nok_task_h);

        OS_TASK_CREATE("SW_FSM",                        /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_kick_sw_fsm_task,   /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                         /* The number of bytes to allocate to the
                                                            stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 3,    /* The priority assigned to the task. */
                        sys_charger_kick_sw_fsm_task_h); /* The task handle */

        OS_ASSERT(sys_charger_kick_sw_fsm_task_h);

        sys_charger_kick_sw_fsm_timer_h =
                OS_TIMER_CREATE("SW_FSM_TIM",
                OS_MS_2_TICKS(10),                      /* Expire after 10 msec */
                OS_TIMER_SUCCESS,                       /* Run repeatedly */
                (void *) 0,
                sys_charger_kick_sw_fsm_timer_cb);

        OS_ASSERT(sys_charger_kick_sw_fsm_timer_h != NULL);
}

#endif /* (dg_configUSE_SYS_CHARGER == 1) */
/**
 \}
 \}
 \}
 */
