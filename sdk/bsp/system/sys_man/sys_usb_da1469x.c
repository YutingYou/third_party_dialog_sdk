/**
 ****************************************************************************************
 *
 * @file sys_usb_da1469x.c
 *
 * @brief System USB
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
 \addtogroup SYS_USB
 \{
 */

#if (dg_configUSE_SYS_USB == 1)

#include "sys_usb.h"
#include "sys_usb_da1469x_internal.h"
#include <osal.h>
#include <sys_power_mgr.h>
#include <sys_clock_mgr.h>
#include <hw_gpio.h>
#include <../adapters/src/ad_pmu_internal.h>

/************************************** Private definitions ***************************************/

#define SYS_USB_20ms_SAFE_READOUT_MARGIN           20       /*  20ms */

/************************************** Forward Declarations **************************************/

static void sys_usb_task(void* pvParameters);
static void sys_usb_process_attach(void);
static void sys_usb_process_detach(void);
static void sys_usb_process_vbus_event(void);

#if (dg_configUSE_USB_ENUMERATION == 1)
static void sys_usb_assert_usb_data_pin_conf(void);
static void sys_usb_idle_on_suspend(bool set_idle);
extern void set_sdk_callbacks_1469x();
extern void set_emusb_1469x_driver();
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

/************************************** OS handlers ***********************************************/

__RETAINED static OS_TASK sys_usb_task_h;

/************************************** Housekeeping variables ************************************/

__RETAINED static bool sys_usb_is_process_attach_completed;
#if (dg_configUSE_USB_ENUMERATION == 1)
__RETAINED static bool sys_usb_is_pll96_activated;
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

/************************************** Private types *********************************************/

/*
 * List of exchanged messages between tasks / ISR's
 */
typedef enum {
        SYS_USB_TASK_MSG_VBUS_UNKNOWN     = 0,           /* VBUS state is unknown */
        SYS_USB_TASK_MSG_VBUS_RISE        = (1 << 1),    /* VBUS is attached  */
        SYS_USB_TASK_MSG_VBUS_FALL        = (1 << 2),    /* VBUS is detached */
        SYS_USB_TASK_MSG_USB_RESET        = (1 << 3),    /* USB RESET detected */
        SYS_USB_TASK_MSG_USB_RESUME       = (1 << 4),    /* USB RESUME detected */
        SYS_USB_TASK_MSG_SYS_CLOCK_RESET  = (1 << 5),    /* restore the system clock*/
        SYS_USB_TASK_MSG_SYS_CLOCK_PLL    = (1 << 6),    /* set the system clock to PLL96*/
} SYS_USB_TASK_MSG_STAT;

/************************************** ISR callbacks *********************************************/

static void sys_usb_vbus_isr_cb(HW_USB_VBUS_IRQ_STAT status)
{
        if (status & HW_USB_VBUS_IRQ_STAT_RISE) {
                OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, SYS_USB_TASK_MSG_VBUS_RISE, OS_NOTIFY_SET_BITS);
        } else if (status & HW_USB_VBUS_IRQ_STAT_FALL) {
                OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, SYS_USB_TASK_MSG_VBUS_FALL, OS_NOTIFY_SET_BITS);
        }
}

__UNUSED static void sys_usb_usb_isr_cb(uint32_t status)
{
        hw_usb_interrupt_handler(status);
}
/************************************** Processing Tasks ******************************************/

static void sys_usb_task(void* pvParameters)
{
        uint32_t        notif;
        OS_BASE_TYPE    ret;

        /* Initialize VBUS IRQ mask assuming that VBUS is not present
         * to start at a known state.
         */
        hw_usb_program_vbus_irq_on_rising();
        hw_usb_program_vbus_irq_on_falling();

        /* Check if VBUS is actually present in case USB was attached
         * before this task started.
         */
        if (hw_usb_is_powered_by_vbus()) {
                /* VBUS IRQ has been missed (since it was not enabled when USB was attached).
                 * A fake interrupt is triggered to let the VBUS IRQ handler place the system in
                 * the correct state.
                 */
                NVIC_SetPendingIRQ(VBUS_IRQn);
        }

        while (true) {
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                if (ret == OS_TASK_NOTIFY_FAIL) {
                        continue;
                }

                if ((notif & SYS_USB_TASK_MSG_VBUS_RISE) ||
                    (notif & SYS_USB_TASK_MSG_VBUS_FALL)) {
                        sys_usb_process_vbus_event();
                }

#if (dg_configUSE_USB_ENUMERATION == 1)
                if (notif & SYS_USB_TASK_MSG_SYS_CLOCK_RESET) {
                        if ( sys_usb_is_pll96_activated ) {
                                cm_sys_clk_set(dg_configDEFAULT_CLK);
                                sys_usb_is_pll96_activated = false;
                        }
                }

                /* clock set to PLL96 MUST complete before handling usb events */
                if (notif & SYS_USB_TASK_MSG_SYS_CLOCK_PLL) {
                        if ( ! sys_usb_is_pll96_activated ) {
                                cm_sys_clk_set(sysclk_PLL96);
                                sys_usb_is_pll96_activated = true;
                        }
                }

                if (notif & SYS_USB_TASK_MSG_USB_RESUME) {
                        OS_ASSERT(sys_usb_is_pll96_activated == true);
                        hw_usb_resume_event();
                        //Host has exited suspend state, go from idle to active
                        sys_usb_idle_on_suspend(false);
                        sys_usb_int_charger_hook_resume_event();
                }

                if (notif & SYS_USB_TASK_MSG_USB_RESET) {
                        OS_ASSERT(sys_usb_is_pll96_activated == true);

                        if (hw_usb_is_suspended()) {
                                //Host has exited suspend state, go from idle to active
                                sys_usb_idle_on_suspend(false);
                                hw_usb_set_suspended(false);
                        }
                        hw_usb_reset_event();

                }
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */
        }
}

static void sys_usb_process_vbus_event(void)
{
        OS_DELAY_MS(SYS_USB_20ms_SAFE_READOUT_MARGIN);

        while (true) {
                if (CRG_TOP->ANA_STATUS_REG & REG_MSK(CRG_TOP, ANA_STATUS_REG, VBUS_AVAILABLE)) {
                        // VBUS_AVAILABLE == 1
                        if (hw_usb_is_powered_by_vbus()) {
                                sys_usb_process_attach();
                                break;
                        }
                } else {
                        // VBUS_AVAILABLE == 0
                        sys_usb_process_detach();
                        break;
                }
                // inconclusive, retry
                OS_DELAY_MS(10);
        }
}

static void sys_usb_process_attach(void)
{
        if (sys_usb_is_process_attach_completed) {
                return;
        }

        /* Safe guard to properly call hw_usb_is_powered_by_vbus() */


        /* sys_usb_process_attach() and sys_usb_process_detach() work in a pipeline
         * fashion. That is the two processes will never interfere with each other.  This
         * is guaranteed by sys_usb_vbus_task(). As a result, there will be no race
         * threat for using sys_usb_is_process_attach_completed
         */
        sys_usb_is_process_attach_completed = true;

        pm_sleep_mode_request(pm_mode_active);
        ad_pmu_dcdc_suspend();

        sys_usb_ext_hook_attach();

#if (dg_configUSE_USB_ENUMERATION == 1) || (dg_configUSE_SYS_CHARGER == 1)
        hw_usb_enable_usb_interrupt(sys_usb_usb_isr_cb);
#endif

#if (dg_configUSE_USB_ENUMERATION == 1)
        sys_usb_assert_usb_data_pin_conf();
#endif

        hw_usb_enable_usb_pads_without_pullup();

#if (dg_configUSE_SYS_CHARGER == 1)
        hw_usb_program_usb_irq();
        sys_usb_int_charger_hook_attach();
#else
        sys_usb_finalize_attach();
#endif
}

static void sys_usb_process_detach(void)
{
#if (dg_configUSE_USB_ENUMERATION == 1)
        bool is_suspended = hw_usb_is_suspended();
#endif

        /* Do not proceed in detaching process in case of an uncompleted attach cycle */
        if (sys_usb_is_process_attach_completed) {
                sys_usb_is_process_attach_completed = false;
                hw_usb_disable_usb_pads();

                ad_pmu_dcdc_resume();

#if (dg_configUSE_USB_ENUMERATION == 1)
                if (is_suspended) {
                        sys_usb_idle_on_suspend(false);
                        hw_usb_set_suspended(false);

#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
                                OS_ENTER_CRITICAL_SECTION();
                                hw_usb_enable_irqs_on_resume();
                                OS_LEAVE_CRITICAL_SECTION();
#endif
                }
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */
                pm_sleep_mode_release(pm_mode_active);

#if (dg_configUSE_SYS_CHARGER == 1)
                sys_usb_int_charger_hook_detach();
#endif

#if (dg_configUSE_USB_ENUMERATION == 1)
                hw_usb_bus_detach();
                REG_SETF(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC, 0);
                USB->USB_MCTRL_REG = 0;
                OS_TASK_NOTIFY(sys_usb_task_h, SYS_USB_TASK_MSG_SYS_CLOCK_RESET, OS_NOTIFY_SET_BITS);
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

                hw_usb_disable_usb_interrupt();
                sys_usb_ext_hook_detach();
        }
}

void sys_usb_init(void)
{
        sys_usb_is_process_attach_completed = false;

        hw_usb_enable_vbus_interrupt(sys_usb_vbus_isr_cb);
#if (dg_configUSE_USB_ENUMERATION == 1)
        set_sdk_callbacks_1469x();
        set_emusb_1469x_driver();
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */
        OS_TASK_CREATE("VBUS",                          /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                       sys_usb_task,                    /* The function that implements the task. */
                       NULL,                            /* The parameter passed to the task. */
                       configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                       OS_TASK_PRIORITY_HIGHEST - 2,    /* The priority assigned to the task. */
                       sys_usb_task_h);                 /* The task handle */

        OS_ASSERT(sys_usb_task_h);
}

void sys_usb_finalize_attach(void)
{
#if (dg_configUSE_USB_ENUMERATION == 1)
        hw_usb_init();
        GPREG->USBPAD_REG = 0;

        //   Power up USB hardware.
        USB->USB_MCTRL_REG = USB_USB_MCTRL_REG_USBEN_Msk;
        USB->USB_MCTRL_REG = USB_USB_MCTRL_REG_USBEN_Msk | USB_USB_MCTRL_REG_USB_NAT_Msk;

        // Now that everything is ready, announce device presence to the USB host.
        hw_usb_bus_attach();
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

        sys_usb_ext_hook_begin_enumeration();
}

/**
 * \brief USB Interrupt handling function.
 *
 */
void hw_usb_interrupt_handler(uint32_t status)
{
        __UNUSED uint16_t maev = status;

#if (dg_configUSE_USB_ENUMERATION == 1)

        if (maev & USB_USB_MAEV_REG_USB_ALT_Msk) {
                uint8_t altev;

                altev = USB->USB_ALTEV_REG;
                altev &= USB->USB_ALTMSK_REG;

                if (altev & USB_USB_ALTEV_REG_USB_SD3_Msk) {
                        ud_stat.sd3++;

                        sys_usb_int_charger_hook_suspend_event();

                        hw_usb_sd3_event();

                        //Host is suspended. Need to reduce power consumption. Go from active to idle
                        sys_usb_idle_on_suspend(true);

                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, SYS_USB_TASK_MSG_SYS_CLOCK_RESET, OS_NOTIFY_SET_BITS);
                }

                if (altev & USB_USB_ALTEV_REG_USB_SD5_Msk) {
                        ud_stat.sd5++;
                        hw_usb_sd5_event();
                }

                if (altev & USB_USB_ALTEV_REG_USB_RESET_Msk) {
                        ud_stat.reset++;
                        REG_CLR_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);
                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h,
                                                SYS_USB_TASK_MSG_SYS_CLOCK_PLL | SYS_USB_TASK_MSG_USB_RESET,
                                                OS_NOTIFY_SET_BITS);
                }

                if (altev & USB_USB_ALTEV_REG_USB_RESUME_Msk) {
                        ud_stat.resume++;
                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h,
                                                SYS_USB_TASK_MSG_USB_RESUME | SYS_USB_TASK_MSG_SYS_CLOCK_RESET,
                                                OS_NOTIFY_SET_BITS);
                }
        }

        if (maev & USB_USB_MAEV_REG_USB_FRAME_Msk) {
                hw_usb_frame_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_NAK_Msk) {
                ud_stat.nak++;
                hw_usb_nak_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_TX_EV_Msk) {
                ud_stat.tx_ev++;
                hw_usb_tx_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_RX_EV_Msk) {
                ud_stat.rx_ev++;
                hw_usb_rx_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_EP0_NAK_Msk) {
                ud_stat.nak0++;
                hw_usb_nak_event_ep0();
        }

        if (maev & USB_USB_MAEV_REG_USB_EP0_TX_Msk) {
                ud_stat.tx_ev0++;
                hw_usb_tx_ep(0);
        }

        if (maev & USB_USB_MAEV_REG_USB_EP0_RX_Msk) {
                ud_stat.rx_ev0++;
                hw_usb_rx_ep0();
        }
#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

#if (dg_configUSE_SYS_CHARGER == 1)
        if (maev & REG_MSK(USB, USB_MAEV_REG, USB_CH_EV)) {
                sys_usb_int_charger_hook_ch_event();
        }
#endif
}

/************************************** Helper Functions ******************************************/

#if (dg_configUSE_USB_ENUMERATION == 1)
static void sys_usb_assert_usb_data_pin_conf(void)
{
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC func;

        hw_gpio_get_pin_function(HW_GPIO_PORT_0, HW_GPIO_PIN_14, &mode, &func);

        OS_ASSERT(mode == HW_GPIO_MODE_INPUT);
        OS_ASSERT(func == HW_GPIO_FUNC_USB);

        hw_gpio_get_pin_function(HW_GPIO_PORT_0, HW_GPIO_PIN_15, &mode, &func);

        OS_ASSERT(mode == HW_GPIO_MODE_INPUT);
        OS_ASSERT(func == HW_GPIO_FUNC_USB);
}

static void sys_usb_idle_on_suspend(bool set_idle)
{
#if (dg_configUSB_SUSPEND_MODE != USB_SUSPEND_MODE_NONE)
        if (set_idle) {
                pm_sleep_mode_request(pm_mode_idle);
                pm_sleep_mode_release(pm_mode_active);
        } else {
                pm_sleep_mode_request(pm_mode_active);
                pm_sleep_mode_release(pm_mode_idle);
        }
#endif
}

#endif /* (dg_configUSE_USB_ENUMERATION == 1) */

#endif /* dg_configUSE_SYS_USB */
/**
 \}
 \}
 \}
 */
