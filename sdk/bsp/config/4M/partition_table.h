/**
 ****************************************************************************************
 *
 * @file 4M/partition_table.h
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#define NVMS_PRODUCT_HEADER_PART_START  0x000000
#define NVMS_PRODUCT_HEADER_PART_SIZE   0x002000
#define NVMS_FIRMWARE_PART_START        0x002000        /* Alignment to 512KB is dictated by the default FLASH_REGION_SIZE. */
#define NVMS_FIRMWARE_PART_SIZE         0x07E000

/* +----------------512KB---------------------+ */

#define NVMS_GENERIC_PART_START         0x0E0000
#define NVMS_GENERIC_PART_SIZE          0x020000
#define NVMS_PLATFORM_PARAMS_PART_START 0x100000
#define NVMS_PLATFORM_PARAMS_PART_SIZE  0x0FF000
#define NVMS_PARAM_PART_START           0x1FF000
#define NVMS_PARAM_PART_SIZE            0x001000        /* Recommended location, 4KB before the end of the 1st flash section. */

/* +------------------2MB---------------------+ */

#define NVMS_LOG_PART_START             0x200000
#define NVMS_LOG_PART_SIZE              0x100000
#define NVMS_BIN_PART_START             0x300000
#define NVMS_BIN_PART_SIZE              0x0FF000
#define NVMS_PARTITION_TABLE_START      0x3FF000
#define NVMS_PARTITION_TABLE_SIZE       0x001000        /* Recommended location, 4KB before the end of the flash. */

PARTITION2( NVMS_PRODUCT_HEADER_PART  , 0 )
PARTITION2( NVMS_FIRMWARE_PART        , 0 )
PARTITION2( NVMS_GENERIC_PART         , PARTITION_FLAG_VES )
PARTITION2( NVMS_PLATFORM_PARAMS_PART , PARTITION_FLAG_READ_ONLY )
PARTITION2( NVMS_PARAM_PART           , 0 )
PARTITION2( NVMS_LOG_PART             , 0 )
PARTITION2( NVMS_BIN_PART             , 0 )
PARTITION2( NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )


