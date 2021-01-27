/*
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

#include <stdint.h>

#define NVPARAM_PARAM_VALUE(TAG, TYPE, ...) \
        TYPE param_ ## TAG[] __attribute__((section("section_" #TAG))) __attribute__((aligned(1))) = { __VA_ARGS__ }; \
        uint16_t param_ ## TAG ## _size __attribute__((section("section_" #TAG "_size"))) __attribute__((aligned(1))) = sizeof(param_ ##TAG);

#if (dg_configNVPARAM_APP_AREA == 1)
#include "app_nvparam_values.h"
#else
#include "platform_nvparam_values.h"
#endif

#define NVPARAM_AREA(NAME, PARTITION, OFFSET)

#define NVPARAM_PARAM(TAG, OFFSET, LENGTH) \
                char sizeofcheck_ ## TAG[LENGTH - sizeof(param_ ## TAG)];

#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH) \
                char sizeofcheck_ ## TAG[LENGTH - sizeof(param_ ## TAG) - 2];

#define NVPARAM_AREA_END()

// define this so preprocessor does not try to include ad_nvparam_defs.h
#define AD_NVPARAM_DEFS_H_

#if (dg_configNVPARAM_APP_AREA == 1)
#include "app_nvparam.h"
#else
#include "platform_nvparam.h"
#endif

// dummy main
int main() { }

