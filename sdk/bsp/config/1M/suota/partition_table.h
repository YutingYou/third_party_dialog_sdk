/**
 ****************************************************************************************
 *
 * @file 1M/suota/partition_table.h
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#define NVMS_PRODUCT_HEADER_PART_START  0x000000
#define NVMS_PRODUCT_HEADER_PART_SIZE   0x002000
#define NVMS_FW_EXEC_PART_START         0x002000
#define NVMS_FW_EXEC_PART_SIZE          0x060000
#define NVMS_LOG_PART_START             0x062000
#define NVMS_LOG_PART_SIZE              0x00B000
#define NVMS_PLATFORM_PARAMS_PART_START 0x06D000
#define NVMS_PLATFORM_PARAMS_PART_SIZE  0x003000
#define NVMS_PARAM_PART_START           0x070000
#define NVMS_PARAM_PART_SIZE            0x00F000
#define NVMS_PARTITION_TABLE_START      0x07F000
#define NVMS_PARTITION_TABLE_SIZE       0x001000
#define NVMS_FW_UPDATE_PART_START       0x080000
#define NVMS_FW_UPDATE_PART_SIZE        0x060000
#define NVMS_GENERIC_PART_START         0x0E0000
#define NVMS_GENERIC_PART_SIZE          0x020000

PARTITION2( NVMS_PRODUCT_HEADER_PART  , 0 )
PARTITION2( NVMS_FW_EXEC_PART         , 0 )
PARTITION2( NVMS_LOG_PART             , 0 )
PARTITION2( NVMS_PLATFORM_PARAMS_PART , PARTITION_FLAG_READ_ONLY )
PARTITION2( NVMS_PARAM_PART           , 0 )
PARTITION2( NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )
PARTITION2( NVMS_FW_UPDATE_PART       , 0 )
PARTITION2( NVMS_GENERIC_PART         , PARTITION_FLAG_VES )

