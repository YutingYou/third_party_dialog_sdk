/**
 ****************************************************************************************
 *
 * @file jump_table.c
 *
 * @brief Heaps and configuration table setup.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE

/*
 * INCLUDES
 ****************************************************************************************
 */
#include <string.h>
#include "co_version.h"
#include "ble_stack_config.h"
#include "sdk_defs.h"
#include "cmsis_compiler.h"
#include "co_version.h"
#include "rwip.h"
#include "arch.h"
#include "user_config_defs.h"
#include "gapc.h"

#ifndef dg_cfgCMAC_WDOG_VAL
#define dg_cfgCMAC_WDOG_VAL                     (1000)
#endif

#define RWIP_HEAP_DB_SIZE_JT                    dg_configBLE_STACK_DB_HEAP_SIZE //IN BYTES

/*
 * Needed macros to calculate heap size (copied from ble-stack not exported file)
 */
#define RWIP_HEAP_HEADER             (12 / sizeof(uint32_t))
#define RWIP_CALC_HEAP_LEN(len)      ((((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)) + RWIP_HEAP_HEADER)

/* Attribute Database Heap */
uint32_t rwip_heap_db_ret[RWIP_CALC_HEAP_LEN(RWIP_HEAP_DB_SIZE_JT)] __RETAINED;
const uint32_t rwip_heap_db_size_jt = RWIP_HEAP_DB_SIZE_JT;

#endif /* CONFIG_USE_BLE */
