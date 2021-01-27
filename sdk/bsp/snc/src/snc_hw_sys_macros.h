/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SYS
 *
 * \brief System LLD macros for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_sys_macros.h
 *
 * @brief SNC-Definition of Sensor Node Controller system Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_SYS_MACROS_H_
#define SNC_HW_SYS_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== SNC critical section definition =========================

void snc_critical_section_enter(b_ctx_t* b_ctx);
#define _SNC_ENTER_CRITICAL_SECTION()                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        snc_critical_section_enter(b_ctx)

void snc_critical_section_leave(b_ctx_t* b_ctx);
#define _SNC_LEAVE_CRITICAL_SECTION()                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        snc_critical_section_leave(b_ctx)

//==================== SNC mutex manipulation macros ===========================

void snc_mutex_lock(b_ctx_t* b_ctx, snc_cm33_mutex_t* mutex);
#define _SNC_MUTEX_LOCK(mutex)                                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        snc_mutex_lock(b_ctx, _SNC_OP_VALUE(snc_cm33_mutex_t*, mutex))

void snc_mutex_unlock(b_ctx_t* b_ctx, snc_cm33_mutex_t* mutex);
#define _SNC_MUTEX_UNLOCK(mutex)                                                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_mutex_unlock(b_ctx, _SNC_OP_VALUE(snc_cm33_mutex_t*, mutex))

//==================== SNC uCode resources access manipulation macros ==========

void snc_ucode_rsrc_acquire(b_ctx_t* b_ctx, SNC_UCODE_TYPE ucode_type,
        snc_ucode_context_t* ucode_ctx);
#define _SNC_UCODE_RSRC_ACQUIRE(name)                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        snc_ucode_rsrc_acquire(b_ctx, SNC_UCODE_TYPE(name), SNC_UCODE_CTX(name))

void snc_ucode_rsrc_release(b_ctx_t* b_ctx, SNC_UCODE_TYPE ucode_type,
        snc_ucode_context_t* ucode_ctx);
#define _SNC_UCODE_RSRC_RELEASE(name)                                                           \
        SNC_STEP_BY_STEP();                                                                     \
        snc_ucode_rsrc_release(b_ctx, SNC_UCODE_TYPE(name), SNC_UCODE_CTX(name))

//==================== SNC uCodes exchanging events with SYSCPU macros =========

void snc_cm33_notify(b_ctx_t* b_ctx);
#define _SNC_CM33_NOTIFY()                                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_cm33_notify(b_ctx)

void snc_check_cm33_notif_pending(b_ctx_t* b_ctx, SENIS_OPER_TYPE is_pending_type,
        uint32_t* is_pending);
#define _SNC_CHECK_CM33_NOTIF_PENDING(is_pending)                                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_check_cm33_notif_pending(b_ctx, _SNC_OP_ADDRESS(is_pending))

//==================== SNC-main-uCode control functions ========================

void snc_hw_sys_init(b_ctx_t* b_ctx);
#define _SNC_hw_sys_init()                                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_init(b_ctx)

void snc_hw_sys_prevent_sleep(b_ctx_t* b_ctx);
#define _SNC_PREVENT_SLEEP()                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_prevent_sleep(b_ctx)

void snc_hw_sys_allow_sleep(b_ctx_t* b_ctx);
#define _SNC_ALLOW_SLEEP()                                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_allow_sleep(b_ctx)

//==================== Peripheral Acquisition functions ========================

void snc_hw_sys_bsr_try_acquire(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id,
        SENIS_OPER_TYPE is_acquired_type, uint32_t* is_acquired);
#define _SNC_hw_sys_bsr_try_acquire(perif_id, is_acquired)                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_bsr_try_acquire(b_ctx, _SNC_OP_VALUE(HW_SYS_BSR_PERIPH_ID, perif_id),        \
                                          _SNC_OP_ADDRESS(is_acquired))

void snc_hw_sys_bsr_acquire(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id);
#define _SNC_hw_sys_bsr_acquire(perif_id)                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_bsr_acquire(b_ctx, _SNC_OP_VALUE(HW_SYS_BSR_PERIPH_ID, perif_id))

void snc_hw_sys_bsr_release(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id);
#define _SNC_hw_sys_bsr_release(perif_id)                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_bsr_release(b_ctx, _SNC_OP_VALUE(HW_SYS_BSR_PERIPH_ID, perif_id))

//==================== PDC event entry acknowledgment functions ================

void snc_hw_sys_pdc_acknowledge(b_ctx_t* b_ctx, SENIS_OPER_TYPE idx_type, uint32_t* idx);
#define _SNC_hw_sys_pdc_acknowledge(idx)                                                        \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_pdc_acknowledge(b_ctx, _SNC_OP(idx))

void snc_hw_sys_get_pdc_pending(b_ctx_t* b_ctx, SENIS_OPER_TYPE pending_type, uint32_t* pending);
#define _SNC_hw_sys_get_pdc_pending(pending)                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_get_pdc_pending(b_ctx, _SNC_OP_ADDRESS(pending))

//==================== Clearing peripheral events functions ====================

void snc_hw_sys_clear_wkup_status(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t status);
#define _SNC_hw_sys_clear_wkup_status(port, status)                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_clear_wkup_status(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                  \
                                            _SNC_OP_VALUE(uint32_t, status))

void snc_hw_sys_clear_timer_gpio_event(b_ctx_t* b_ctx, uint32_t mask);
#define _SNC_hw_sys_clear_timer_gpio_event(mask)                                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_clear_timer_gpio_event(b_ctx, _SNC_OP_VALUE(uint32_t, mask))

void snc_hw_sys_clear_timer_interrupt(b_ctx_t* b_ctx, HW_TIMER_ID id);
#define _SNC_hw_sys_clear_timer_interrupt(id)                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_clear_timer_interrupt(b_ctx, _SNC_OP_VALUE(HW_TIMER_ID, id))

void snc_hw_sys_get_rtc_event_flags(b_ctx_t* b_ctx, SENIS_OPER_TYPE flags_type, uint32_t* flags);
#define _SNC_hw_sys_get_rtc_event_flags(flags)                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_get_rtc_event_flags(b_ctx, _SNC_OP_ADDRESS(flags))

void snc_hw_sys_clear_rtc_pdc_event(b_ctx_t* b_ctx);
#define _SNC_hw_sys_clear_rtc_pdc_event()                                                       \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_clear_rtc_pdc_event(b_ctx)

//==================== RTC Time/Calendar Getter functions ======================

void snc_hw_sys_rtc_get_time_bcd(b_ctx_t* b_ctx, SENIS_OPER_TYPE time_type, uint32_t* time);
#define _SNC_hw_sys_rtc_get_time_bcd(time)                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_rtc_get_time_bcd(b_ctx, _SNC_OP_ADDRESS(time))

void snc_hw_sys_rtc_get_clndr_bcd(b_ctx_t* b_ctx, SENIS_OPER_TYPE clndr_type, uint32_t* clndr);
#define _SNC_hw_sys_rtc_get_clndr_bcd(clndr)                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_rtc_get_clndr_bcd(b_ctx, _SNC_OP_ADDRESS(clndr))

//==================== Timer Capture Getter functions ==========================

void snc_hw_sys_timer_get_capture1(b_ctx_t* b_ctx, HW_TIMER_ID id, SENIS_OPER_TYPE cap_type,
        uint32_t* cap);
#define _SNC_hw_sys_timer_get_capture1(id, cap)                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_timer_get_capture1(b_ctx, id, _SNC_OP_ADDRESS(cap))

void snc_hw_sys_timer_get_capture2(b_ctx_t* b_ctx, HW_TIMER_ID id, SENIS_OPER_TYPE cap_type,
        uint32_t* cap);
#define _SNC_hw_sys_timer_get_capture2(id, cap)                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_timer_get_capture2(b_ctx, id, _SNC_OP_ADDRESS(cap))

void snc_hw_sys_timer_get_capture3(b_ctx_t* b_ctx, SENIS_OPER_TYPE cap_type, uint32_t* cap);
#define _SNC_hw_sys_timer_get_capture3(cap)                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_timer_get_capture3(b_ctx, _SNC_OP_ADDRESS(cap))

void snc_hw_sys_timer_get_capture4(b_ctx_t* b_ctx, SENIS_OPER_TYPE cap_type, uint32_t* cap);
#define _SNC_hw_sys_timer_get_capture4(cap)                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_timer_get_capture4(b_ctx, _SNC_OP_ADDRESS(cap))

//==================== System Timer Getter functions ===========================

#if (defined(OS_FREERTOS) && dg_configUSE_HW_TIMER)
void snc_hw_sys_timer_get_uptime_ticks(b_ctx_t* b_ctx, SENIS_OPER_TYPE uptime_ticks_type,
        uint32_t* uptime_ticks);
#define _SNC_hw_sys_timer_get_uptime_ticks(uptime_ticks)                                        \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_timer_get_uptime_ticks(b_ctx, _SNC_OP_ADDRESS(uptime_ticks))
#endif /* defined(OS_FREERTOS) && dg_configUSE_HW_TIMER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_SYS_MACROS_H_ */

/**
 * \}
 * \}
 */
