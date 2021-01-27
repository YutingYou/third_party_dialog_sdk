/*
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */


#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "cmsis_compiler.h"

#define __IRQ           __attribute__((interrupt))
#define __FIQ           __attribute__((interrupt))
#define __BLEIRQ

// define size of an empty array (used to declare structure with an array size not defined)
#define __ARRAY_EMPTY

#endif // _COMPILER_H_
