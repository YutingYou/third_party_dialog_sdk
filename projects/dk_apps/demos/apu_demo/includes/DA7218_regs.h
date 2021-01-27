/**
 ****************************************************************************************
 *
 * @file DA7218_regs.h
 *
 * @brief DA7217/8 registers file
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef REGS_DA7218_H
#define REGS_DA7218_H

/****************************************************************************
* Module: memoryMap5
* Range.: 0x0..0x1, 0x2
****************************************************************************/

/****************************************************************************
* SYSTEM_ACTIVE_REG, SYSTEM_ACTIVE description
****************************************************************************/

#define SYSTEM_ACTIVE_REG_adr 0x0
#define SYSTEM_ACTIVE_REG_reset 0x00
#define SYSTEM_ACTIVE_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SYSTEM_ACTIVE        : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} SYSTEM_ACTIVE_REG_type ;

#define SYSTEM_ACTIVE_SYSTEM_ACTIVE        0x0001 // Bit 0

/****************************************************************************
* CIF_CTRL_REG, CIF_CTRL description
****************************************************************************/

#define CIF_CTRL_REG_adr 0x1
#define CIF_CTRL_REG_reset 0x00
#define CIF_CTRL_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CIF_I2C_WRITE_MODE   : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} CIF_CTRL_REG_type ;

#define CIF_CTRL_CIF_I2C_WRITE_MODE   0x0001 // Bit 0

/****************************************************************************
* Module: memoryMap6
* Range.: 0x4..0x10, 0xD
****************************************************************************/

/****************************************************************************
* CHIP_ID1_REG, CHIP_ID1 description
****************************************************************************/

#define DA7218CHIP_ID1_REG_adr 0x4
#define DA7218CHIP_ID1_REG_reset 0x23
#define DA7218CHIP_ID1_REG_mask 0xFF

/****************************************************************************
* CHIP_ID2_REG, CHIP_ID2 description
****************************************************************************/

#define DA7218CHIP_ID2_REG_adr 0x5
#define DA7218CHIP_ID2_REG_reset 0x39
#define DA7218CHIP_ID2_REG_mask 0xFF

/****************************************************************************
* SOFT_RESET_REG, SOFT_RESET description
****************************************************************************/

#define SOFT_RESET_REG_adr 0x9
#define SOFT_RESET_REG_reset 0x00
#define SOFT_RESET_REG_mask 0x80

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 7;
    unsigned char CIF_REG_SOFT_RESET   : 1; // Bit 7
  };
} SOFT_RESET_REG_type ;

#define SOFT_RESET_CIF_REG_SOFT_RESET   0x0080 // Bit 7

/****************************************************************************
* CHIP_REVISION_REG, CHIP_REVISION description
****************************************************************************/

#define DA7218CHIP_REVISION_REG_adr 0x6
#define DA7218CHIP_REVISION_REG_reset 0x01
#define DA7218CHIP_REVISION_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CHIP_MINOR           : 4; // Bit 3-0
    unsigned char CHIP_MAJOR           : 4; // Bit 7-4
  };
} DA7218CHIP_REVISION_REG_type ;

#define DA7218CHIP_REVISION_CHIP_MINOR           0x000F // Bit 3-0
#define DA7218CHIP_REVISION_CHIP_MAJOR           0x00F0 // Bit 7-4

/****************************************************************************
* SR_REG, SR description
****************************************************************************/

#define SR_REG_adr 0xB
#define SR_REG_reset 0xAA
#define SR_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SR_ADC               : 4; // Bit 3-0
    unsigned char SR_DAC               : 4; // Bit 7-4
  };
} SR_REG_type ;

#define SR_SR_ADC               0x000F // Bit 3-0
#define SR_SR_DAC               0x00F0 // Bit 7-4

/****************************************************************************
* PC_COUNT_REG, PC_COUNT description
****************************************************************************/

#define PC_COUNT_REG_adr 0xC
#define PC_COUNT_REG_reset 0x02
#define PC_COUNT_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char PC_FREERUN           : 1; // Bit 0
    unsigned char PC_RESYNC_AUTO       : 1; // Bit 1
    unsigned char                      : 6; // Bit 7-2
  };
} PC_COUNT_REG_type ;

#define PC_COUNT_PC_FREERUN           0x0001 // Bit 0
#define PC_COUNT_PC_RESYNC_AUTO       0x0002 // Bit 1

/****************************************************************************
* CIF_TIMEOUT_CTRL_REG, CIF_TIMEOUT_CTRL description
****************************************************************************/

#define CIF_TIMEOUT_CTRL_REG_adr 0x10
#define CIF_TIMEOUT_CTRL_REG_reset 0x01
#define CIF_TIMEOUT_CTRL_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char I2C_TIMEOUT_EN       : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} CIF_TIMEOUT_CTRL_REG_type ;

#define CIF_TIMEOUT_CTRL_I2C_TIMEOUT_EN       0x0001 // Bit 0

/****************************************************************************
* GAIN_RAMP_CTRL_REG, GAIN_RAMP_CTRL description
****************************************************************************/

#define GAIN_RAMP_CTRL_REG_adr 0xD
#define GAIN_RAMP_CTRL_REG_reset 0x00
#define GAIN_RAMP_CTRL_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char GAIN_RAMP_RATE       : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} GAIN_RAMP_CTRL_REG_type ;

#define GAIN_RAMP_CTRL_GAIN_RAMP_RATE       0x0003 // Bit 1-0

/****************************************************************************
* Module: memoryMap31
* Range.: 0x14..0x16, 0x3
****************************************************************************/

/****************************************************************************
* SYSTEM_MODES_INPUT_REG, SYSTEM_MODES_INPUT description
****************************************************************************/

#define SYSTEM_MODES_INPUT_REG_adr 0x14
#define SYSTEM_MODES_INPUT_REG_reset 0x00
#define SYSTEM_MODES_INPUT_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MODE_SUBMIT          : 1; // Bit 0
    unsigned char ADC_MODE             : 7; // Bit 7-1
  };
} SYSTEM_MODES_INPUT_REG_type ;

#define SYSTEM_MODES_INPUT_MODE_SUBMIT          0x0001 // Bit 0
#define SYSTEM_MODES_INPUT_ADC_MODE             0x00FE // Bit 7-1

/****************************************************************************
* SYSTEM_MODES_OUTPUT_REG, SYSTEM_MODES_OUTPUT description
****************************************************************************/

#define SYSTEM_MODES_OUTPUT_REG_adr 0x15
#define SYSTEM_MODES_OUTPUT_REG_reset 0x00
#define SYSTEM_MODES_OUTPUT_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char FIELD1               : 1; // Bit 0
    unsigned char DAC_MODE             : 7; // Bit 7-1
  };
} SYSTEM_MODES_OUTPUT_REG_type ;

#define SYSTEM_MODES_OUTPUT_FIELD1               0x0001 // Bit 0
#define SYSTEM_MODES_OUTPUT_DAC_MODE             0x00FE // Bit 7-1

/****************************************************************************
* SYSTEM_STATUS_REG, SYSTEM_STATUS description
****************************************************************************/

#define SYSTEM_STATUS_REG_adr 0x16
#define SYSTEM_STATUS_REG_reset 0x00
#define SYSTEM_STATUS_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SC1_BUSY             : 1; // Bit 0
    unsigned char SC2_BUSY             : 1; // Bit 1
    unsigned char                      : 6; // Bit 7-2
  };
} SYSTEM_STATUS_REG_type ;

#define SYSTEM_STATUS_SC1_BUSY             0x0001 // Bit 0
#define SYSTEM_STATUS_SC2_BUSY             0x0002 // Bit 1

/****************************************************************************
* Module: memoryMap16
* Range.: 0x18..0x1B, 0x4
****************************************************************************/

/****************************************************************************
* IN_1L_FILTER_CTRL_REG, IN_1L_FILTER_CTRL description
****************************************************************************/

#define IN_1L_FILTER_CTRL_REG_adr 0x18
#define IN_1L_FILTER_CTRL_REG_reset 0x00
#define IN_1L_FILTER_CTRL_REG_mask 0xE0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char         : 5;
    unsigned char IN_1L_RAMP_EN        : 1; // Bit 5
    unsigned char IN_1L_MUTE_EN        : 1; // Bit 6
    unsigned char IN_1L_FILTER_EN      : 1; // Bit 7
  };
} IN_1L_FILTER_CTRL_REG_type ;

#define IN_1L_FILTER_CTRL_IN_1L_RAMP_EN        0x0020 // Bit 5
#define IN_1L_FILTER_CTRL_IN_1L_MUTE_EN        0x0040 // Bit 6
#define IN_1L_FILTER_CTRL_IN_1L_FILTER_EN      0x0080 // Bit 7

/****************************************************************************
* IN_1R_FILTER_CTRL_REG, IN_1R_FILTER_CTRL description
****************************************************************************/

#define IN_1R_FILTER_CTRL_REG_adr 0x19
#define IN_1R_FILTER_CTRL_REG_reset 0x00
#define IN_1R_FILTER_CTRL_REG_mask 0xE0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char         : 5;
    unsigned char IN_1R_RAMP_EN        : 1; // Bit 5
    unsigned char IN_1R_MUTE_EN        : 1; // Bit 6
    unsigned char IN_1R_FILTER_EN      : 1; // Bit 7
  };
} IN_1R_FILTER_CTRL_REG_type ;

#define IN_1R_FILTER_CTRL_IN_1R_RAMP_EN        0x0020 // Bit 5
#define IN_1R_FILTER_CTRL_IN_1R_MUTE_EN        0x0040 // Bit 6
#define IN_1R_FILTER_CTRL_IN_1R_FILTER_EN      0x0080 // Bit 7

/****************************************************************************
* IN_2L_FILTER_CTRL_REG, IN_2L_FILTER_CTRL description
****************************************************************************/

#define IN_2L_FILTER_CTRL_REG_adr 0x1A
#define IN_2L_FILTER_CTRL_REG_reset 0x00
#define IN_2L_FILTER_CTRL_REG_mask 0xE0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 5;
    unsigned char IN_2L_RAMP_EN        : 1; // Bit 5
    unsigned char IN_2L_MUTE_EN        : 1; // Bit 6
    unsigned char IN_2L_FILTER_EN      : 1; // Bit 7
  };
} IN_2L_FILTER_CTRL_REG_type ;

#define IN_2L_FILTER_CTRL_IN_2L_RAMP_EN        0x0020 // Bit 5
#define IN_2L_FILTER_CTRL_IN_2L_MUTE_EN        0x0040 // Bit 6
#define IN_2L_FILTER_CTRL_IN_2L_FILTER_EN      0x0080 // Bit 7

/****************************************************************************
* IN_2R_FILTER_CTRL_REG, IN_2R_FILTER_CTRL description
****************************************************************************/

#define IN_2R_FILTER_CTRL_REG_adr 0x1B
#define IN_2R_FILTER_CTRL_REG_reset 0x00
#define IN_2R_FILTER_CTRL_REG_mask 0xE0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 5;
    unsigned char IN_2R_RAMP_EN        : 1; // Bit 5
    unsigned char IN_2R_MUTE_EN        : 1; // Bit 6
    unsigned char IN_2R_FILTER_EN      : 1; // Bit 7
  };
} IN_2R_FILTER_CTRL_REG_type ;

#define IN_2R_FILTER_CTRL_IN_2R_RAMP_EN        0x0020 // Bit 5
#define IN_2R_FILTER_CTRL_IN_2R_MUTE_EN        0x0040 // Bit 6
#define IN_2R_FILTER_CTRL_IN_2R_FILTER_EN      0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap26
* Range.: 0x20..0x21, 0x2
****************************************************************************/

/****************************************************************************
* OUT_1L_FILTER_CTRL_REG, DAC_L_CTRL description
****************************************************************************/

#define OUT_1L_FILTER_CTRL_REG_adr 0x20
#define OUT_1L_FILTER_CTRL_REG_reset 0x40
#define OUT_1L_FILTER_CTRL_REG_mask 0xF8

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 3;
    unsigned char OUT_1L_BIQ_5STAGE_SEL  : 1; // Bit 3
    unsigned char OUT_1L_SUBRANGE_EN   : 1; // Bit 4
    unsigned char OUT_1L_RAMP_EN       : 1; // Bit 5
    unsigned char OUT_1L_MUTE_EN       : 1; // Bit 6
    unsigned char OUT_1L_FILTER_EN     : 1; // Bit 7
  };
} OUT_1L_FILTER_CTRL_REG_type ;

#define OUT_1L_FILTER_CTRL_OUT_1L_BIQ_5STAGE_SEL  0x0008 // Bit 3
#define OUT_1L_FILTER_CTRL_OUT_1L_SUBRANGE_EN   0x0010 // Bit 4
#define OUT_1L_FILTER_CTRL_OUT_1L_RAMP_EN       0x0020 // Bit 5
#define OUT_1L_FILTER_CTRL_OUT_1L_MUTE_EN       0x0040 // Bit 6
#define OUT_1L_FILTER_CTRL_OUT_1L_FILTER_EN     0x0080 // Bit 7

/****************************************************************************
* OUT_1R_FILTER_CTRL_REG, DAC_R_CTRL description
****************************************************************************/

#define OUT_1R_FILTER_CTRL_REG_adr 0x21
#define OUT_1R_FILTER_CTRL_REG_reset 0x40
#define OUT_1R_FILTER_CTRL_REG_mask 0xF8

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 3;
    unsigned char OUT_1R_BIQ_5STAGE_SEL  : 1; // Bit 3
    unsigned char OUT_1R_SUBRANGE_EN   : 1; // Bit 4
    unsigned char OUT_1R_RAMP_EN       : 1; // Bit 5
    unsigned char OUT_1R_MUTE_EN       : 1; // Bit 6
    unsigned char OUT_1R_FILTER_EN     : 1; // Bit 7
  };
} OUT_1R_FILTER_CTRL_REG_type ;

#define OUT_1R_FILTER_CTRL_OUT_1R_BIQ_5STAGE_SEL  0x0008 // Bit 3
#define OUT_1R_FILTER_CTRL_OUT_1R_SUBRANGE_EN   0x0010 // Bit 4
#define OUT_1R_FILTER_CTRL_OUT_1R_RAMP_EN       0x0020 // Bit 5
#define OUT_1R_FILTER_CTRL_OUT_1R_MUTE_EN       0x0040 // Bit 6
#define OUT_1R_FILTER_CTRL_OUT_1R_FILTER_EN     0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap25
* Range.: 0x24..0x2A, 0x7
****************************************************************************/

/****************************************************************************
* OUT_1_HPF_FILTER_CTRL_REG, OUT_1_HPF_FILTER description
****************************************************************************/

#define OUT_1_HPF_FILTER_CTRL_REG_adr 0x24
#define OUT_1_HPF_FILTER_CTRL_REG_reset 0x80
#define OUT_1_HPF_FILTER_CTRL_REG_mask 0xBF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUT_VOICE_HPF_CORNER  : 3; // Bit 2-0
    unsigned char OUT_VOICE_EN          : 1; // Bit 3
    unsigned char OUT_AUDIO_HPF_CORNER  : 2; // Bit 5-4
    unsigned char _reserved_6_          : 1; // Bit 6
    unsigned char OUT_HPF_EN            : 1; // Bit 7
  };
} OUT_1_HPF_FILTER_CTRL_REG_type ;

#define OUT_1_HPF_FILTER_CTRL_OUT_1_VOICE_HPF_CORNER  0x0007 // Bit 2-0
#define OUT_1_HPF_FILTER_CTRL_OUT_1_VOICE_EN       0x0008 // Bit 3
#define OUT_1_HPF_FILTER_CTRL_OUT_1_AUDIO_HPF_CORNER  0x0030 // Bit 5-4
#define OUT_1_HPF_FILTER_CTRL__reserved_6_         0x0040 // Bit 6
#define OUT_1_HPF_FILTER_CTRL_OUT_1_HPF_EN         0x0080 // Bit 7

/****************************************************************************
* OUT_1_BIQ_5STAGE_CTRL_REG, OUT_1_BIQ_5STAGE_CTRL description
****************************************************************************/

#define OUT_1_BIQ_5STAGE_CTRL_REG_adr 0x28
#define OUT_1_BIQ_5STAGE_CTRL_REG_reset 0x40
#define OUT_1_BIQ_5STAGE_CTRL_REG_mask 0xC0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 6;
    unsigned char OUT_1_BIQ_5STAGE_MUTE_EN  : 1; // Bit 6
    unsigned char OUT_1_BIQ_5STAGE_FILTER_EN  : 1; // Bit 7
  };
} OUT_1_BIQ_5STAGE_CTRL_REG_type ;

#define OUT_1_BIQ_5STAGE_CTRL_OUT_1_BIQ_5STAGE_MUTE_EN  0x0040 // Bit 6
#define OUT_1_BIQ_5STAGE_CTRL_OUT_1_BIQ_5STAGE_FILTER_EN  0x0080 // Bit 7

/****************************************************************************
* OUT_1_EQ_12_FILTER_CTRL_REG, OUT_1_EQ_12_FILTER description
****************************************************************************/

#define OUT_1_EQ_12_FILTER_CTRL_REG_adr 0x25
#define OUT_1_EQ_12_FILTER_CTRL_REG_reset 0x77
#define OUT_1_EQ_12_FILTER_CTRL_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUT_1_EQ_BAND1       : 4; // Bit 3-0
    unsigned char OUT_1_EQ_BAND2       : 4; // Bit 7-4
  };
} OUT_1_EQ_12_FILTER_CTRL_REG_type ;

#define OUT_1_EQ_12_FILTER_CTRL_OUT_1_EQ_BAND1       0x000F // Bit 3-0
#define OUT_1_EQ_12_FILTER_CTRL_OUT_1_EQ_BAND2       0x00F0 // Bit 7-4

/****************************************************************************
* OUT_1_BIQ_5STAGE_DATA_REG, OUT_1_BIQ_5STAGE_DATA description
****************************************************************************/

#define OUT_1_BIQ_5STAGE_DATA_REG_adr 0x29
#define OUT_1_BIQ_5STAGE_DATA_REG_reset 0x00
#define OUT_1_BIQ_5STAGE_DATA_REG_mask 0xFF

/****************************************************************************
* OUT_1_EQ_34_FILTER_CTRL_REG, OUT_1_EQ_34_FILTER description
****************************************************************************/

#define OUT_1_EQ_34_FILTER_CTRL_REG_adr 0x26
#define OUT_1_EQ_34_FILTER_CTRL_REG_reset 0x77
#define OUT_1_EQ_34_FILTER_CTRL_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUT_1_EQ_BAND3       : 4; // Bit 3-0
    unsigned char OUT_1_EQ_BAND4       : 4; // Bit 7-4
  };
} OUT_1_EQ_34_FILTER_CTRL_REG_type ;

#define OUT_1_EQ_34_FILTER_CTRL_OUT_1_EQ_BAND3       0x000F // Bit 3-0
#define OUT_1_EQ_34_FILTER_CTRL_OUT_1_EQ_BAND4       0x00F0 // Bit 7-4

/****************************************************************************
* OUT_1_BIQ_5STAGE_ADDR_REG, OUT_1_BIQ_5STAGE_ADDR description
****************************************************************************/

#define OUT_1_BIQ_5STAGE_ADDR_REG_adr 0x2A
#define OUT_1_BIQ_5STAGE_ADDR_REG_reset 0x00
#define OUT_1_BIQ_5STAGE_ADDR_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUT_1_BIQ_5STAGE_ADDR  : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} OUT_1_BIQ_5STAGE_ADDR_REG_type ;

#define OUT_1_BIQ_5STAGE_ADDR_OUT_1_BIQ_5STAGE_ADDR  0x003F // Bit 5-0

/****************************************************************************
* OUT_1_EQ_5_FILTER_CTRL_REG, OUT_1_EQ_5_FILTER description
****************************************************************************/

#define OUT_1_EQ_5_FILTER_CTRL_REG_adr 0x27
#define OUT_1_EQ_5_FILTER_CTRL_REG_reset 0x07
#define OUT_1_EQ_5_FILTER_CTRL_REG_mask 0x8F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUT_1_EQ_BAND5       : 4; // Bit 3-0
    unsigned char _reserved_4_         : 3; // Bit 6-4
    unsigned char OUT_1_EQ_EN          : 1; // Bit 7
  };
} OUT_1_EQ_5_FILTER_CTRL_REG_type ;

#define OUT_1_EQ_5_FILTER_CTRL_OUT_1_EQ_BAND5       0x000F // Bit 3-0
#define OUT_1_EQ_5_FILTER_CTRL__reserved_4_         0x0070 // Bit 6-4
#define OUT_1_EQ_5_FILTER_CTRL_OUT_1_EQ_EN          0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap23
* Range.: 0x2C..0x2F, 0x4
****************************************************************************/

/****************************************************************************
* MIXIN_1_CTRL_REG, MIXIN_1_CTRL description
****************************************************************************/

#define MIXIN_1_CTRL_REG_adr 0x2C
#define MIXIN_1_CTRL_REG_reset 0x48
#define MIXIN_1_CTRL_REG_mask 0xFB

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXIN_AMP_BIAS       : 2; // Bit 1-0
    unsigned char _reserved_2_         : 1; // Bit 2
    unsigned char MIXIN_1_MIX_SEL      : 1; // Bit 3
    unsigned char MIXIN_1_AMP_ZC_EN    : 1; // Bit 4
    unsigned char MIXIN_1_AMP_RAMP_EN  : 1; // Bit 5
    unsigned char MIXIN_1_AMP_MUTE_EN  : 1; // Bit 6
    unsigned char MIXIN_1_AMP_EN       : 1; // Bit 7
  };
} MIXIN_1_CTRL_REG_type ;

#define MIXIN_1_CTRL_MIXIN_AMP_BIAS       0x0003 // Bit 1-0
#define MIXIN_1_CTRL__reserved_2_         0x0004 // Bit 2
#define MIXIN_1_CTRL_MIXIN_1_MIX_SEL      0x0008 // Bit 3
#define MIXIN_1_CTRL_MIXIN_1_AMP_ZC_EN    0x0010 // Bit 4
#define MIXIN_1_CTRL_MIXIN_1_AMP_RAMP_EN  0x0020 // Bit 5
#define MIXIN_1_CTRL_MIXIN_1_AMP_MUTE_EN  0x0040 // Bit 6
#define MIXIN_1_CTRL_MIXIN_1_AMP_EN       0x0080 // Bit 7

/****************************************************************************
* MIXIN_1_GAIN_REG, MIXIN_1_GAIN description
****************************************************************************/

#define MIXIN_1_GAIN_REG_adr 0x2D
#define MIXIN_1_GAIN_REG_reset 0x03
#define MIXIN_1_GAIN_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXIN_1_AMP_GAIN     : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} MIXIN_1_GAIN_REG_type ;

#define MIXIN_1_GAIN_MIXIN_1_AMP_GAIN     0x000F // Bit 3-0

/****************************************************************************
* MIXIN_2_CTRL_REG, MIXIN_2_CTRL description
****************************************************************************/

#define MIXIN_2_CTRL_REG_adr 0x2E
#define MIXIN_2_CTRL_REG_reset 0x48
#define MIXIN_2_CTRL_REG_mask 0xF8

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 3;
    unsigned char MIXIN_2_MIX_SEL      : 1; // Bit 3
    unsigned char MIXIN_2_AMP_ZC_EN    : 1; // Bit 4
    unsigned char MIXIN_2_AMP_RAMP_EN  : 1; // Bit 5
    unsigned char MIXIN_2_AMP_MUTE_EN  : 1; // Bit 6
    unsigned char MIXIN_2_AMP_EN       : 1; // Bit 7
  };
} MIXIN_2_CTRL_REG_type ;

#define MIXIN_2_CTRL_MIXIN_2_MIX_SEL      0x0008 // Bit 3
#define MIXIN_2_CTRL_MIXIN_2_AMP_ZC_EN    0x0010 // Bit 4
#define MIXIN_2_CTRL_MIXIN_2_AMP_RAMP_EN  0x0020 // Bit 5
#define MIXIN_2_CTRL_MIXIN_2_AMP_MUTE_EN  0x0040 // Bit 6
#define MIXIN_2_CTRL_MIXIN_2_AMP_EN       0x0080 // Bit 7

/****************************************************************************
* MIXIN_2_GAIN_REG, MIXIN_2_GAIN description
****************************************************************************/

#define MIXIN_2_GAIN_REG_adr 0x2F
#define MIXIN_2_GAIN_REG_reset 0x03
#define MIXIN_2_GAIN_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXIN_2_AMP_GAIN     : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} MIXIN_2_GAIN_REG_type ;

#define MIXIN_2_GAIN_MIXIN_2_AMP_GAIN     0x000F // Bit 3-0

/****************************************************************************
* Module: memoryMap2
* Range.: 0x30..0x38, 0x9
****************************************************************************/

/****************************************************************************
* ALC_CTRL1_REG, ALC_CTRL1 description
****************************************************************************/

#define ALC_CTRL1_REG_adr 0x30
#define ALC_CTRL1_REG_reset 0x00
#define ALC_CTRL1_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_EN               : 4; // Bit 3-0
    unsigned char ALC_SYNC_MODE        : 4; // Bit 7-4
  };
} ALC_CTRL1_REG_type ;

#define ALC_CTRL1_ALC_EN               0x000F // Bit 3-0
#define ALC_CTRL1_ALC_SYNC_MODE        0x00F0 // Bit 7-4

/****************************************************************************
* ALC_CTRL2_REG, ALC_CTRL2 description
****************************************************************************/

#define ALC_CTRL2_REG_adr 0x31
#define ALC_CTRL2_REG_reset 0x00
#define ALC_CTRL2_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_ATTACK           : 4; // Bit 3-0
    unsigned char ALC_RELEASE          : 4; // Bit 7-4
  };
} ALC_CTRL2_REG_type ;

#define ALC_CTRL2_ALC_ATTACK           0x000F // Bit 3-0
#define ALC_CTRL2_ALC_RELEASE          0x00F0 // Bit 7-4

/****************************************************************************
* ALC_CTRL3_REG, ALC_CTRL3 description
****************************************************************************/

#define ALC_CTRL3_REG_adr 0x32
#define ALC_CTRL3_REG_reset 0x00
#define ALC_CTRL3_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_HOLD             : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} ALC_CTRL3_REG_type ;

#define ALC_CTRL3_ALC_HOLD             0x000F // Bit 3-0

/****************************************************************************
* ALC_NOISE_REG, ALC_NOISE description
****************************************************************************/

#define ALC_NOISE_REG_adr 0x33
#define ALC_NOISE_REG_reset 0x3F
#define ALC_NOISE_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_NOISE            : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} ALC_NOISE_REG_type ;

#define ALC_NOISE_ALC_NOISE            0x003F // Bit 5-0

/****************************************************************************
* ALC_TARGET_MIN_REG, ALC_TARGET_MIN description
****************************************************************************/

#define ALC_TARGET_MIN_REG_adr 0x34
#define ALC_TARGET_MIN_REG_reset 0x3F
#define ALC_TARGET_MIN_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_THRESHOLD_MIN    : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} ALC_TARGET_MIN_REG_type ;

#define ALC_TARGET_MIN_ALC_THRESHOLD_MIN    0x003F // Bit 5-0

/****************************************************************************
* ALC_TARGET_MAX_REG, ALC_TARGET_MAX description
****************************************************************************/

#define ALC_TARGET_MAX_REG_adr 0x35
#define ALC_TARGET_MAX_REG_reset 0x00
#define ALC_TARGET_MAX_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_THRESHOLD_MAX    : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} ALC_TARGET_MAX_REG_type ;

#define ALC_TARGET_MAX_ALC_THRESHOLD_MAX    0x003F // Bit 5-0

/****************************************************************************
* ALC_GAIN_LIMITS_REG, ALC_GAIN_LIMITS description
****************************************************************************/

#define ALC_GAIN_LIMITS_REG_adr 0x36
#define ALC_GAIN_LIMITS_REG_reset 0xFF
#define ALC_GAIN_LIMITS_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_ATTEN_MAX        : 4; // Bit 3-0
    unsigned char ALC_GAIN_MAX         : 4; // Bit 7-4
  };
} ALC_GAIN_LIMITS_REG_type ;

#define ALC_GAIN_LIMITS_ALC_ATTEN_MAX        0x000F // Bit 3-0
#define ALC_GAIN_LIMITS_ALC_GAIN_MAX         0x00F0 // Bit 7-4

/****************************************************************************
* ALC_ANA_GAIN_LIMITS_REG, ALC_ANA_GAIN_LIMITS description
****************************************************************************/

#define ALC_ANA_GAIN_LIMITS_REG_adr 0x37
#define ALC_ANA_GAIN_LIMITS_REG_reset 0x71
#define ALC_ANA_GAIN_LIMITS_REG_mask 0x77

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_ANA_GAIN_MIN     : 3; // Bit 2-0
    unsigned char _reserved_3_         : 1; // Bit 3
    unsigned char ALC_ANA_GAIN_MAX     : 3; // Bit 6-4
    unsigned char                      : 1; // Bit 7
  };
} ALC_ANA_GAIN_LIMITS_REG_type ;

#define ALC_ANA_GAIN_LIMITS_ALC_ANA_GAIN_MIN     0x0007 // Bit 2-0
#define ALC_ANA_GAIN_LIMITS__reserved_3_         0x0008 // Bit 3
#define ALC_ANA_GAIN_LIMITS_ALC_ANA_GAIN_MAX     0x0070 // Bit 6-4

/****************************************************************************
* ALC_ANTICLIP_CTRL_REG, ALC_ANTICLIP_CTRL description
****************************************************************************/

#define ALC_ANTICLIP_CTRL_REG_adr 0x38
#define ALC_ANTICLIP_CTRL_REG_reset 0x00
#define ALC_ANTICLIP_CTRL_REG_mask 0x83

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ALC_ANTICLIP_STEP    : 2; // Bit 1-0
    unsigned char _reserved_2_         : 5; // Bit 6-2
    unsigned char ALC_ANTICLIP_EN      : 1; // Bit 7
  };
} ALC_ANTICLIP_CTRL_REG_type ;

#define ALC_ANTICLIP_CTRL_ALC_ANTICLIP_STEP    0x0003 // Bit 1-0
#define ALC_ANTICLIP_CTRL__reserved_2_         0x007C // Bit 6-2
#define ALC_ANTICLIP_CTRL_ALC_ANTICLIP_EN      0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap1
* Range.: 0x3C..0x40, 0x5
****************************************************************************/

/****************************************************************************
* AGS_ENABLE_REG, AGS_ENABLE description
****************************************************************************/

#define AGS_ENABLE_REG_adr 0x3C
#define AGS_ENABLE_REG_reset 0x00
#define AGS_ENABLE_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char AGS_ENABLE           : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} AGS_ENABLE_REG_type ;

#define AGS_ENABLE_AGS_ENABLE           0x0003 // Bit 1-0

/****************************************************************************
* AGS_ANTICLIP_CTRL_REG, AGS_ANTICLIP_CTRL description
****************************************************************************/

#define AGS_ANTICLIP_CTRL_REG_adr 0x40
#define AGS_ANTICLIP_CTRL_REG_reset 0x00
#define AGS_ANTICLIP_CTRL_REG_mask 0x80

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 7;
    unsigned char AGS_ANTICLIP_EN      : 1; // Bit 7
  };
} AGS_ANTICLIP_CTRL_REG_type ;

#define AGS_ANTICLIP_CTRL_AGS_ANTICLIP_EN      0x0080 // Bit 7

/****************************************************************************
* AGS_TRIGGER_REG, AGS_TRIGGER description
****************************************************************************/

#define AGS_TRIGGER_REG_adr 0x3D
#define AGS_TRIGGER_REG_reset 0x09
#define AGS_TRIGGER_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char AGS_TRIGGER          : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} AGS_TRIGGER_REG_type ;

#define AGS_TRIGGER_AGS_TRIGGER          0x000F // Bit 3-0

/****************************************************************************
* AGS_ATT_MAX_REG, AGS_ATT_MAX description
****************************************************************************/

#define AGS_ATT_MAX_REG_adr 0x3E
#define AGS_ATT_MAX_REG_reset 0x00
#define AGS_ATT_MAX_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char AGS_ATT_MAX          : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} AGS_ATT_MAX_REG_type ;

#define AGS_ATT_MAX_AGS_ATT_MAX          0x0007 // Bit 2-0

/****************************************************************************
* AGS_TIMEOUT_REG, AGS_TIMEOUT description
****************************************************************************/

#define AGS_TIMEOUT_REG_adr 0x3F
#define AGS_TIMEOUT_REG_reset 0x00
#define AGS_TIMEOUT_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char AGS_TIMEOUT_EN       : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} AGS_TIMEOUT_REG_type ;

#define AGS_TIMEOUT_AGS_TIMEOUT_EN       0x0001 // Bit 0

/****************************************************************************
* Module: memoryMap3
* Range.: 0x44..0x48, 0x5
****************************************************************************/

/****************************************************************************
* CALIB_CTRL_REG, CALIB_CTRL description
****************************************************************************/

#define CALIB_CTRL_REG_adr 0x44
#define CALIB_CTRL_REG_reset 0x00
#define CALIB_CTRL_REG_mask 0x0D

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CALIB_OFFSET_EN      : 1; // Bit 0
    unsigned char _reserved_1_         : 1; // Bit 1
    unsigned char CALIB_AUTO_EN        : 1; // Bit 2
    unsigned char CALIB_OVERFLOW       : 1; // Bit 3
    unsigned char                      : 4; // Bit 7-4
  };
} CALIB_CTRL_REG_type ;

#define CALIB_CTRL_CALIB_OFFSET_EN      0x0001 // Bit 0
#define CALIB_CTRL__reserved_1_         0x0002 // Bit 1
#define CALIB_CTRL_CALIB_AUTO_EN        0x0004 // Bit 2
#define CALIB_CTRL_CALIB_OVERFLOW       0x0008 // Bit 3

/****************************************************************************
* CALIB_OFFSET_AUTO_U_2_REG, ALC_OFFSET_AUTO_U_R description
****************************************************************************/

#define CALIB_OFFSET_AUTO_U_2_REG_adr 0x48
#define CALIB_OFFSET_AUTO_U_2_REG_reset 0x00
#define CALIB_OFFSET_AUTO_U_2_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CALIB_OFFSET_AUTO_U_2  : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} CALIB_OFFSET_AUTO_U_2_REG_type ;

#define CALIB_OFFSET_AUTO_U_2_CALIB_OFFSET_AUTO_U_2  0x000F // Bit 3-0

/****************************************************************************
* CALIB_OFFSET_AUTO_M_1_REG, ALC_OFFSET_AUTO_M_L description
****************************************************************************/

#define CALIB_OFFSET_AUTO_M_1_REG_adr 0x45
#define CALIB_OFFSET_AUTO_M_1_REG_reset 0x00
#define CALIB_OFFSET_AUTO_M_1_REG_mask 0xFF

/****************************************************************************
* CALIB_OFFSET_AUTO_U_1_REG, ALC_OFFSET_AUTO_U_L description
****************************************************************************/

#define CALIB_OFFSET_AUTO_U_1_REG_adr 0x46
#define CALIB_OFFSET_AUTO_U_1_REG_reset 0x00
#define CALIB_OFFSET_AUTO_U_1_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CALIB_OFFSET_AUTO_U_1  : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} CALIB_OFFSET_AUTO_U_1_REG_type ;

#define CALIB_OFFSET_AUTO_U_1_CALIB_OFFSET_AUTO_U_1  0x000F // Bit 3-0

/****************************************************************************
* CALIB_OFFSET_AUTO_M_2_REG, ALC_OFFSET_AUTO_M_R description
****************************************************************************/

#define CALIB_OFFSET_AUTO_M_2_REG_adr 0x47
#define CALIB_OFFSET_AUTO_M_2_REG_reset 0x00
#define CALIB_OFFSET_AUTO_M_2_REG_mask 0xFF

/****************************************************************************
* Module: memoryMap13
* Range.: 0x4C..0x4C, 0x1
****************************************************************************/

/****************************************************************************
* ENV_TRACK_CTRL_REG, ENV_TRACK_CTRL description
****************************************************************************/

#define ENV_TRACK_CTRL_REG_adr 0x4C
#define ENV_TRACK_CTRL_REG_reset 0x00
#define ENV_TRACK_CTRL_REG_mask 0x33

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char INTEG_ATTACK         : 2; // Bit 1-0
    unsigned char _reserved_2_         : 2; // Bit 3-2
    unsigned char INTEG_RELEASE        : 2; // Bit 5-4
    unsigned char                      : 2; // Bit 7-6
  };
} ENV_TRACK_CTRL_REG_type ;

#define ENV_TRACK_CTRL_INTEG_ATTACK         0x0003 // Bit 1-0
#define ENV_TRACK_CTRL__reserved_2_         0x000C // Bit 3-2
#define ENV_TRACK_CTRL_INTEG_RELEASE        0x0030 // Bit 5-4

/****************************************************************************
* Module: memoryMap20
* Range.: 0x50..0x51, 0x2
****************************************************************************/

/****************************************************************************
* LVL_DET_CTRL_REG, LVL_DET_CTRL description
****************************************************************************/

#define LVL_DET_CTRL_REG_adr 0x50
#define LVL_DET_CTRL_REG_reset 0x00
#define LVL_DET_CTRL_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char LVL_DET_EN           : 4; // Bit 3-0
    unsigned char                      : 4; // Bit 7-4
  };
} LVL_DET_CTRL_REG_type ;

#define LVL_DET_CTRL_LVL_DET_EN           0x000F // Bit 3-0

/****************************************************************************
* LVL_DET_LEVEL_REG, LVL_DET_LEVEL description
****************************************************************************/

#define LVL_DET_LEVEL_REG_adr 0x51
#define LVL_DET_LEVEL_REG_reset 0x7F
#define LVL_DET_LEVEL_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char LVL_DET_LEVEL        : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} LVL_DET_LEVEL_REG_type ;

#define LVL_DET_LEVEL_LVL_DET_LEVEL        0x007F // Bit 6-0

/****************************************************************************
* Module: memoryMap10
* Range.: 0x54..0x5B, 0x8
****************************************************************************/

/****************************************************************************
* DGS_TRIGGER_REG, DGS_TRIGGER description
****************************************************************************/

#define DGS_TRIGGER_REG_adr 0x54
#define DGS_TRIGGER_REG_reset 0x24
#define DGS_TRIGGER_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_TRIGGER_LVL      : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} DGS_TRIGGER_REG_type ;

#define DGS_TRIGGER_DGS_TRIGGER_LVL      0x003F // Bit 5-0

/****************************************************************************
* DGS_SYNC_DELAY2_REG,
****************************************************************************/

#define DGS_SYNC_DELAY2_REG_adr 0x58
#define DGS_SYNC_DELAY2_REG_reset 0x31
#define DGS_SYNC_DELAY2_REG_mask 0xFF

/****************************************************************************
* DGS_ENABLE_REG, DGS_ENABLE description
****************************************************************************/

#define DGS_ENABLE_REG_adr 0x55
#define DGS_ENABLE_REG_reset 0x00
#define DGS_ENABLE_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_ENABLE           : 2; // Bit 1-0
    unsigned char DGS_DTM_CFG          : 3; // Bit 4-2
    unsigned char                      : 3; // Bit 7-5
  };
} DGS_ENABLE_REG_type ;

#define DGS_ENABLE_DGS_ENABLE           0x0003 // Bit 1-0
#define DGS_ENABLE_DGS_DTM_CFG          0x001C // Bit 4-2

/****************************************************************************
* DGS_SYNC_DELAY3_REG,
****************************************************************************/

#define DGS_SYNC_DELAY3_REG_adr 0x59
#define DGS_SYNC_DELAY3_REG_reset 0x11
#define DGS_SYNC_DELAY3_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_SYNC_DELAY3      : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} DGS_SYNC_DELAY3_REG_type ;

#define DGS_SYNC_DELAY3_DGS_SYNC_DELAY3      0x007F // Bit 6-0

/****************************************************************************
* DGS_RISE_FALL_REG, DGS_RISE_FALL description
****************************************************************************/

#define DGS_RISE_FALL_REG_adr 0x56
#define DGS_RISE_FALL_REG_reset 0x50
#define DGS_RISE_FALL_REG_mask 0x77

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_RISE_COEFF       : 3; // Bit 2-0
    unsigned char _reserved_3_         : 1; // Bit 3
    unsigned char DGS_FALL_COEFF       : 3; // Bit 6-4
    unsigned char                      : 1; // Bit 7
  };
} DGS_RISE_FALL_REG_type ;

#define DGS_RISE_FALL_DGS_RISE_COEFF       0x0007 // Bit 2-0
#define DGS_RISE_FALL__reserved_3_         0x0008 // Bit 3
#define DGS_RISE_FALL_DGS_FALL_COEFF       0x0070 // Bit 6-4

/****************************************************************************
* DGS_LEVELS_REG, DGS_ANTICLIP_LVL description
****************************************************************************/

#define DGS_LEVELS_REG_adr 0x5A
#define DGS_LEVELS_REG_reset 0x01
#define DGS_LEVELS_REG_mask 0xF7

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_ANTICLIP_LVL     : 3; // Bit 2-0
    unsigned char _reserved_3_         : 1; // Bit 3
    unsigned char DGS_SIGNAL_LVL       : 4; // Bit 7-4
  };
} DGS_LEVELS_REG_type ;

#define DGS_LEVELS_DGS_ANTICLIP_LVL     0x0007 // Bit 2-0
#define DGS_LEVELS__reserved_3_         0x0008 // Bit 3
#define DGS_LEVELS_DGS_SIGNAL_LVL       0x00F0 // Bit 7-4

/****************************************************************************
* DGS_SYNC_DELAY_REG, DGS_SYNC_DELAY description
****************************************************************************/

#define DGS_SYNC_DELAY_REG_adr 0x57
#define DGS_SYNC_DELAY_REG_reset 0xA3
#define DGS_SYNC_DELAY_REG_mask 0xFF

/****************************************************************************
* DGS_GAIN_CTRL_REG, DGS_GAIN_CTRL description
****************************************************************************/

#define DGS_GAIN_CTRL_REG_adr 0x5B
#define DGS_GAIN_CTRL_REG_reset 0x74
#define DGS_GAIN_CTRL_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DGS_STEPS            : 5; // Bit 4-0
    unsigned char DGS_RAMP_EN          : 1; // Bit 5
    unsigned char DGS_SUBR_EN          : 1; // Bit 6
    unsigned char                      : 1; // Bit 7
  };
} DGS_GAIN_CTRL_REG_type ;

#define DGS_GAIN_CTRL_DGS_STEPS            0x001F // Bit 4-0
#define DGS_GAIN_CTRL_DGS_RAMP_EN          0x0020 // Bit 5
#define DGS_GAIN_CTRL_DGS_SUBR_EN          0x0040 // Bit 6

/****************************************************************************
* Module: memoryMap29
* Range.: 0x5C..0x8B, 0x30
****************************************************************************/

/****************************************************************************
* DROUTING_OUTDAI_1L_REG, DROUTING_OUTDAI_1L description
****************************************************************************/

#define DROUTING_OUTDAI_1L_REG_adr 0x5C
#define DROUTING_OUTDAI_1L_REG_reset 0x01
#define DROUTING_OUTDAI_1L_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char InFilt1LEn           : 1; //bit 0 = Input filter 1 left
    unsigned char InFilt1REn           : 1; //bit 1 = Input filter 1 right
    unsigned char InFilt2LEn           : 1; //bit 2 = Input filter 2 left
    unsigned char InFilt2REn           : 1; //bit 3 = Input filter 2 right
    unsigned char ToneGen              : 1; //bit 4 = Tone generator
    unsigned char DaiLeft              : 1; //bit 5 = DAI 1 input left data
    unsigned char DaiRight             : 1; //bit 6 = DAI 1 input right data
    unsigned char                      : 1; // Bit 7
  };
} DROUTING_OUTDAI_1L_REG_type ;

#define DROUTING_OUTDAI_1L_OUTDAI_1L_SRC        0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTDAI_1L_INFILT_2R_GAIN_REG, DMIX_OUTDAI_1L_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INFILT_2R_GAIN_REG_adr 0x60
#define DMIX_OUTDAI_1L_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INFILT_2R_GAIN_OUTDAI_1L_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DROUTING_OUTDAI_1R_REG, DROUTING_OUTDAI_1R description
****************************************************************************/

#define DROUTING_OUTDAI_1R_REG_adr 0x64
#define DROUTING_OUTDAI_1R_REG_reset 0x04
#define DROUTING_OUTDAI_1R_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char InFilt1LEn           : 1; //bit 0 = Input filter 1 left
    unsigned char InFilt1REn           : 1; //bit 1 = Input filter 1 right
    unsigned char InFilt2LEn           : 1; //bit 2 = Input filter 2 left
    unsigned char InFilt2REn           : 1; //bit 3 = Input filter 2 right
    unsigned char ToneGen              : 1; //bit 4 = Tone generator
    unsigned char DaiLeft              : 1; //bit 5 = DAI 1 input left data
    unsigned char DaiRight             : 1; //bit 6 = DAI 1 input right data
    unsigned char                      : 1; // Bit 7
  };
} DROUTING_OUTDAI_1R_REG_type ;

#define DROUTING_OUTDAI_1R_OUTDAI_1R_SRC        0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTDAI_1R_INFILT_2R_GAIN_REG, DMIX_OUTDAI_1R_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INFILT_2R_GAIN_REG_adr 0x68
#define DMIX_OUTDAI_1R_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INFILT_2R_GAIN_OUTDAI_1R_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DROUTING_OUTFILT_1L_REG, DROUTING_OUTFILT_1L description
****************************************************************************/

#define DROUTING_OUTFILT_1L_REG_adr 0x6C
#define DROUTING_OUTFILT_1L_REG_reset 0x01
#define DROUTING_OUTFILT_1L_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char InFilt1LEn           : 1; //bit 0 = Input filter 1 left
    unsigned char InFilt1REn           : 1; //bit 1 = Input filter 1 right
    unsigned char InFilt2LEn           : 1; //bit 2 = Input filter 2 left
    unsigned char InFilt2REn           : 1; //bit 3 = Input filter 2 right
    unsigned char ToneGen              : 1; //bit 4 = Tone generator
    unsigned char DaiLeft              : 1; //bit 5 = DAI 1 input left data
    unsigned char DaiRight             : 1; //bit 6 = DAI 1 input right data
    unsigned char                      : 1; // Bit 7
  };
} DROUTING_OUTFILT_1L_REG_type ;

#define DROUTING_OUTFILT_1L_OUTFILT_1L_SRC       0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTFILT_1L_INFILT_2R_GAIN_REG, DMIX_OUTFILT_1L_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INFILT_2R_GAIN_REG_adr 0x70
#define DMIX_OUTFILT_1L_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INFILT_2R_GAIN_OUTFILT_1L_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DROUTING_OUTFILT_1R_REG, DROUTING_OUTFILT_1R description
****************************************************************************/

#define DROUTING_OUTFILT_1R_REG_adr 0x74
#define DROUTING_OUTFILT_1R_REG_reset 0x04
#define DROUTING_OUTFILT_1R_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char InFilt1LEn           : 1;//bit 0 = Input filter 1 left
    unsigned char InFilt1REn           : 1;//bit 1 = Input filter 1 right
    unsigned char InFilt2LEn           : 1;//bit 2 = Input filter 2 left
    unsigned char InFilt2REn           : 1;//bit 3 = Input filter 2 right
    unsigned char ToneGen              : 1;//bit 4 = Tone generator
    unsigned char DaiLeft              : 1;//bit 5 = DAI 1 input left data
    unsigned char DaiRight             : 1;//bit 6 = DAI 1 input right data
    unsigned char                      : 1; // Bit 7
  };

} DROUTING_OUTFILT_1R_REG_type ;

#define DROUTING_OUTFILT_1R_OUTFILT_1R_SRC       0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTFILT_1R_INFILT_2R_GAIN_REG, DMIX_OUTFILT_1R_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INFILT_2R_GAIN_REG_adr 0x78
#define DMIX_OUTFILT_1R_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INFILT_2R_GAIN_OUTFILT_1R_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_INFILT_1L_GAIN_REG, DMIX_OUTFILT_1L_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INFILT_1L_GAIN_REG_adr 0x5D
#define DMIX_OUTDAI_1L_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INFILT_1L_GAIN_OUTDAI_1L_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_TONEGEN_GAIN_REG, DMIX_OUTDAI_1L_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_TONEGEN_GAIN_REG_adr 0x61
#define DMIX_OUTDAI_1L_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_TONEGEN_GAIN_OUTDAI_1L_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_INFILT_1L_GAIN_REG, DMIX_OUTDAI_1R_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INFILT_1L_GAIN_REG_adr 0x65
#define DMIX_OUTDAI_1R_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INFILT_1L_GAIN_OUTDAI_1R_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_TONEGEN_GAIN_REG, DMIX_OUTDAI_1R_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_TONEGEN_GAIN_REG_adr 0x69
#define DMIX_OUTDAI_1R_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_TONEGEN_GAIN_OUTDAI_1R_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_INFILT_1L_GAIN_REG, DMIX_OUTFILT_1L_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INFILT_1L_GAIN_REG_adr 0x6D
#define DMIX_OUTFILT_1L_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INFILT_1L_GAIN_OUTFILT_1L_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_TONEGEN_GAIN_REG, DMIX_OUTFILT_1L_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_TONEGEN_GAIN_REG_adr 0x71
#define DMIX_OUTFILT_1L_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_TONEGEN_GAIN_OUTFILT_1L_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_INFILT_1L_GAIN_REG, DMIX_OUTFILT_1R_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INFILT_1L_GAIN_REG_adr 0x75
#define DMIX_OUTFILT_1R_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INFILT_1L_GAIN_OUTFILT_1R_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_TONEGEN_GAIN_REG, DMIX_DAC_R_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_TONEGEN_GAIN_REG_adr 0x79
#define DMIX_OUTFILT_1R_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_TONEGEN_GAIN_OUTFILT_1R_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_INFILT_1R_GAIN_REG, DMIX_OUTDAI_1L_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INFILT_1R_GAIN_REG_adr 0x5E
#define DMIX_OUTDAI_1L_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INFILT_1R_GAIN_OUTDAI_1L_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_INDAI_1L_GAIN_REG, DMIX_OUTDAI_1L_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INDAI_1L_GAIN_REG_adr 0x62
#define DMIX_OUTDAI_1L_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INDAI_1L_GAIN_OUTDAI_1L_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_INFILT_1R_GAIN_REG, DMIX_OUTDAI_1R_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INFILT_1R_GAIN_REG_adr 0x66
#define DMIX_OUTDAI_1R_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INFILT_1R_GAIN_OUTDAI_1R_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_INDAI_1L_GAIN_REG, DMIX_OUTDAI_1R_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INDAI_1L_GAIN_REG_adr 0x6A
#define DMIX_OUTDAI_1R_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INDAI_1L_GAIN_OUTDAI_1R_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_INFILT_1R_GAIN_REG, DMIX_OUTFILT_1L_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INFILT_1R_GAIN_REG_adr 0x6E
#define DMIX_OUTFILT_1L_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INFILT_1R_GAIN_OUTFILT_1L_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG, DMIX_OUTFILT_1L_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG_adr 0x72
#define DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INDAI_1L_GAIN_OUTFILT_1L_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_INFILT_1R_GAIN_REG, DMIX_OUTFILT_1R_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INFILT_1R_GAIN_REG_adr 0x76
#define DMIX_OUTFILT_1R_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INFILT_1R_GAIN_OUTFILT_1R_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_INDAI_1L_GAIN_REG, DMIX_OUTFILT_1R_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INDAI_1L_GAIN_REG_adr 0x7A
#define DMIX_OUTFILT_1R_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INDAI_1L_GAIN_OUTFILT_1R_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_INFILT_2L_GAIN_REG, DMIX_OUTDAI_1L_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INFILT_2L_GAIN_REG_adr 0x5F
#define DMIX_OUTDAI_1L_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INFILT_2L_GAIN_OUTDAI_1L_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1L_INDAI_1R_GAIN_REG, DMIX_OUTDAI_1L_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1L_INDAI_1R_GAIN_REG_adr 0x63
#define DMIX_OUTDAI_1L_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1L_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1L_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1L_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_1L_INDAI_1R_GAIN_OUTDAI_1L_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_INFILT_2L_GAIN_REG, DMIX_OUTDAI_1R_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INFILT_2L_GAIN_REG_adr 0x67
#define DMIX_OUTDAI_1R_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INFILT_2L_GAIN_OUTDAI_1R_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_1R_INDAI_1R_GAIN_REG, DMIX_OUTDAI_1R_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_1R_INDAI_1R_GAIN_REG_adr 0x6B
#define DMIX_OUTDAI_1R_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_1R_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_1R_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_1R_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_1R_INDAI_1R_GAIN_OUTDAI_1R_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_INFILT_2L_GAIN_REG, DMIX_OUTFILT_1L_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INFILT_2L_GAIN_REG_adr 0x6F
#define DMIX_OUTFILT_1L_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INFILT_2L_GAIN_OUTFILT_1L_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1L_INDAI_1R_GAIN_REG, DMIX_OUTFILT_1L_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1L_INDAI_1R_GAIN_REG_adr 0x73
#define DMIX_OUTFILT_1L_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1L_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1L_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1L_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTFILT_1L_INDAI_1R_GAIN_OUTFILT_1L_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_INFILT_2L_GAIN_REG, DMIX_OUTFILT_1R_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INFILT_2L_GAIN_REG_adr 0x77
#define DMIX_OUTFILT_1R_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INFILT_2L_GAIN_OUTFILT_1R_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG, DMIX_OUTFILT_1R_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG_adr 0x7B
#define DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTFILT_1R_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTFILT_1R_INDAI_1R_GAIN_OUTFILT_1R_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DROUTING_OUTDAI_2L_REG, DROUTING_OUTDAI_2L description
****************************************************************************/

#define DROUTING_OUTDAI_2L_REG_adr 0x7C
#define DROUTING_OUTDAI_2L_REG_reset 0x04
#define DROUTING_OUTDAI_2L_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_SRC        : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} DROUTING_OUTDAI_2L_REG_type ;

#define DROUTING_OUTDAI_2L_OUTDAI_2L_SRC        0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTDAI_2L_INFILT_2R_GAIN_REG, DMIX_OUTDAI_2L_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INFILT_2R_GAIN_REG_adr 0x80
#define DMIX_OUTDAI_2L_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INFILT_2R_GAIN_OUTDAI_2L_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DROUTING_OUTDAI_2R_REG, DROUTING_OUTDAI_2R description
****************************************************************************/

#define DROUTING_OUTDAI_2R_REG_adr 0x84
#define DROUTING_OUTDAI_2R_REG_reset 0x08
#define DROUTING_OUTDAI_2R_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_SRC        : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} DROUTING_OUTDAI_2R_REG_type ;

#define DROUTING_OUTDAI_2R_OUTDAI_2R_SRC        0x007F // Bit 6-0

/****************************************************************************
* DMIX_OUTDAI_2R_INFILT_2R_GAIN_REG, DMIX_OUTDAI_2R_INFILT_2R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INFILT_2R_GAIN_REG_adr 0x88
#define DMIX_OUTDAI_2R_INFILT_2R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INFILT_2R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INFILT_2R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INFILT_2R_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INFILT_2R_GAIN_OUTDAI_2R_INFILT_2R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_INFILT_1L_GAIN_REG, DMIX_OUTFILT_2L_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INFILT_1L_GAIN_REG_adr 0x7D
#define DMIX_OUTDAI_2L_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INFILT_1L_GAIN_OUTDAI_2L_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_TONEGEN_GAIN_REG, DMIX_OUTDAI_2L_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_TONEGEN_GAIN_REG_adr 0x81
#define DMIX_OUTDAI_2L_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_TONEGEN_GAIN_OUTDAI_2L_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_INFILT_1L_GAIN_REG, DMIX_OUTFILT_2R_INFILT_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INFILT_1L_GAIN_REG_adr 0x85
#define DMIX_OUTDAI_2R_INFILT_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INFILT_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INFILT_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INFILT_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INFILT_1L_GAIN_OUTDAI_2R_INFILT_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_TONEGEN_GAIN_REG, DMIX_OUTDAI_2R_TONEGEN_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_TONEGEN_GAIN_REG_adr 0x89
#define DMIX_OUTDAI_2R_TONEGEN_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_TONEGEN_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_TONEGEN_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_TONEGEN_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_TONEGEN_GAIN_OUTDAI_2R_TONEGEN_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_INFILT_1R_GAIN_REG, DMIX_OUTDAI_2L_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INFILT_1R_GAIN_REG_adr 0x7E
#define DMIX_OUTDAI_2L_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INFILT_1R_GAIN_OUTDAI_2L_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_INDAI_1L_GAIN_REG, DMIX_OUTDAI_2L_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INDAI_1L_GAIN_REG_adr 0x82
#define DMIX_OUTDAI_2L_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INDAI_1L_GAIN_OUTDAI_2L_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_INFILT_1R_GAIN_REG, DMIX_OUTDAI_2R_INFILT_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INFILT_1R_GAIN_REG_adr 0x86
#define DMIX_OUTDAI_2R_INFILT_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INFILT_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INFILT_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INFILT_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INFILT_1R_GAIN_OUTDAI_2R_INFILT_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_INDAI_1L_GAIN_REG, DMIX_OUTDAI_2R_INDAI_1L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INDAI_1L_GAIN_REG_adr 0x8A
#define DMIX_OUTDAI_2R_INDAI_1L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INDAI_1L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INDAI_1L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INDAI_1L_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INDAI_1L_GAIN_OUTDAI_2R_INDAI_1L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_INFILT_2L_GAIN_REG, DMIX_OUTDAI_2L_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INFILT_2L_GAIN_REG_adr 0x7F
#define DMIX_OUTDAI_2L_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INFILT_2L_GAIN_OUTDAI_2L_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2L_INDAI_1R_GAIN_REG, DMIX_OUTDAI_2L_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2L_INDAI_1R_GAIN_REG_adr 0x83
#define DMIX_OUTDAI_2L_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2L_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2L_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2L_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_2L_INDAI_1R_GAIN_OUTDAI_2L_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_INFILT_2L_GAIN_REG, DMIX_OUTDAI_2R_INFILT_2L_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INFILT_2L_GAIN_REG_adr 0x87
#define DMIX_OUTDAI_2R_INFILT_2L_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INFILT_2L_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INFILT_2L_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INFILT_2L_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INFILT_2L_GAIN_OUTDAI_2R_INFILT_2L_GAIN  0x001F // Bit 4-0

/****************************************************************************
* DMIX_OUTDAI_2R_INDAI_1R_GAIN_REG, DMIX_OUTDAI_2R_INDAI_1R_GAIN description
****************************************************************************/

#define DMIX_OUTDAI_2R_INDAI_1R_GAIN_REG_adr 0x8B
#define DMIX_OUTDAI_2R_INDAI_1R_GAIN_REG_reset 0x1C
#define DMIX_OUTDAI_2R_INDAI_1R_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OUTDAI_2R_INDAI_1R_GAIN  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} DMIX_OUTDAI_2R_INDAI_1R_GAIN_REG_type ;

#define DMIX_OUTDAI_2R_INDAI_1R_GAIN_OUTDAI_2R_INDAI_1R_GAIN  0x001F // Bit 4-0

/****************************************************************************
* Module: memoryMap9
* Range.: 0x8C..0x90, 0x5
****************************************************************************/

/****************************************************************************
* DAI_CTRL_REG, DAI_CTRL description
****************************************************************************/

#define DAI_CTRL_REG_adr 0x8C
#define DAI_CTRL_REG_reset 0x28
#define DAI_CTRL_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAI_FORMAT           : 2; // Bit 1-0
    unsigned char DAI_WORD_LENGTH      : 2; // Bit 3-2
    unsigned char DAI_CH_NUM           : 3; // Bit 6-4
    unsigned char DAI_EN               : 1; // Bit 7
  };
} DAI_CTRL_REG_type ;

#define DAI_CTRL_DAI_FORMAT           0x0003 // Bit 1-0
#define DAI_CTRL_DAI_WORD_LENGTH      0x000C // Bit 3-2
#define DAI_CTRL_DAI_CH_NUM           0x0070 // Bit 6-4
#define DAI_CTRL_DAI_EN               0x0080 // Bit 7

/****************************************************************************
* DAI_CLK_MODE_REG, DAI_CLK_MODE description
****************************************************************************/

#define DAI_CLK_MODE_REG_adr 0x90
#define DAI_CLK_MODE_REG_reset 0x01
#define DAI_CLK_MODE_REG_mask 0x9F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAI_BCLKS_PER_WCLK   : 2; // Bit 1-0
    unsigned char DAI_CLK_POL          : 1; // Bit 2
    unsigned char DAI_WCLK_POL         : 1; // Bit 3
    unsigned char DAI_WCLK_TRI_STATE   : 1; // Bit 4
    unsigned char _reserved_5_         : 2; // Bit 6-5
    unsigned char DAI_CLK_EN           : 1; // Bit 7
  };
} DAI_CLK_MODE_REG_type ;

#define DAI_CLK_MODE_DAI_BCLKS_PER_WCLK   0x0003 // Bit 1-0
#define DAI_CLK_MODE_DAI_CLK_POL          0x0004 // Bit 2
#define DAI_CLK_MODE_DAI_WCLK_POL         0x0008 // Bit 3
#define DAI_CLK_MODE_DAI_WCLK_TRI_STATE   0x0010 // Bit 4
#define DAI_CLK_MODE__reserved_5_         0x0060 // Bit 6-5
#define DAI_CLK_MODE_DAI_CLK_EN           0x0080 // Bit 7

/****************************************************************************
* DAI_TDM_CTRL_REG,
****************************************************************************/

#define DAI_TDM_CTRL_REG_adr 0x8D
#define DAI_TDM_CTRL_REG_reset 0x40
#define DAI_TDM_CTRL_REG_mask 0xCF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAI_TDM_CH_EN        : 4; // Bit 3-0
    unsigned char _reserved_4_         : 2; // Bit 5-4
    unsigned char DAI_OE               : 1; // Bit 6
    unsigned char DAI_TDM_MODE_EN      : 1; // Bit 7
  };
} DAI_TDM_CTRL_REG_type ;

#define DAI_TDM_CTRL_DAI_TDM_CH_EN        0x000F // Bit 3-0
#define DAI_TDM_CTRL__reserved_4_         0x0030 // Bit 5-4
#define DAI_TDM_CTRL_DAI_OE               0x0040 // Bit 6
#define DAI_TDM_CTRL_DAI_TDM_MODE_EN      0x0080 // Bit 7

/****************************************************************************
* DAI_OFFSET_LOWER_REG, DAI_OFFSET_LOWER description
****************************************************************************/

#define DAI_OFFSET_LOWER_REG_adr 0x8E
#define DAI_OFFSET_LOWER_REG_reset 0x00
#define DAI_OFFSET_LOWER_REG_mask 0xFF

/****************************************************************************
* DAI_OFFSET_UPPER_REG, DAI_OFFSET_UPPER description
****************************************************************************/

#define DAI_OFFSET_UPPER_REG_adr 0x8F
#define DAI_OFFSET_UPPER_REG_reset 0x00
#define DAI_OFFSET_UPPER_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAI_OFFSET_UPPER     : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} DAI_OFFSET_UPPER_REG_type ;

#define DAI_OFFSET_UPPER_DAI_OFFSET_UPPER     0x0007 // Bit 2-0

/****************************************************************************
* Module: memoryMap27
* Range.: 0x91..0x98, 0x8
****************************************************************************/

/****************************************************************************
* PLL_CTRL_REG, PLL_CTRL description
****************************************************************************/

#define PLL_CTRL_REG_adr 0x91
#define PLL_CTRL_REG_reset 0x04
#define PLL_CTRL_REG_mask 0xD7

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char PLL_INDIV            : 3; // Bit 2-0
    unsigned char _reserved_3_         : 1; // Bit 3
    unsigned char PLL_MCLK_SQR_EN      : 1; // Bit 4
    unsigned char _reserved_5_         : 1; // Bit 5
    unsigned char PLL_MODE             : 2; // Bit 7-6
  };
} PLL_CTRL_REG_type ;

#define PLL_CTRL_PLL_INDIV            0x0007 // Bit 2-0
#define PLL_CTRL__reserved_3_         0x0008 // Bit 3
#define PLL_CTRL_PLL_MCLK_SQR_EN      0x0010 // Bit 4
#define PLL_CTRL__reserved_5_         0x0020 // Bit 5
#define PLL_CTRL_PLL_MODE             0x00C0 // Bit 7-6

/****************************************************************************
* PLL_FRAC_TOP_REG, PLL_FRAC_TOP description
****************************************************************************/

#define PLL_FRAC_TOP_REG_adr 0x92
#define PLL_FRAC_TOP_REG_reset 0x00
#define PLL_FRAC_TOP_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char PLL_FBDIV_FRAC_TOP   : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} PLL_FRAC_TOP_REG_type ;

#define PLL_FRAC_TOP_PLL_FBDIV_FRAC_TOP   0x001F // Bit 4-0

/****************************************************************************
* PLL_FRAC_BOT_REG, PLL_FRAC_BOT description
****************************************************************************/

#define PLL_FRAC_BOT_REG_adr 0x93
#define PLL_FRAC_BOT_REG_reset 0x00
#define PLL_FRAC_BOT_REG_mask 0xFF

/****************************************************************************
* PLL_INTEGER_REG, PLL_INTEGER description
****************************************************************************/

#define PLL_INTEGER_REG_adr 0x94
#define PLL_INTEGER_REG_reset 0x20
#define PLL_INTEGER_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char PLL_FBDIV_INTEGER    : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} PLL_INTEGER_REG_type ;

#define PLL_INTEGER_PLL_FBDIV_INTEGER    0x007F // Bit 6-0

/****************************************************************************
* PLL_STATUS_REG, PLL_STATUS description
****************************************************************************/

#define PLL_STATUS_REG_adr 0x95
#define PLL_STATUS_REG_reset 0x00
#define PLL_STATUS_REG_mask 0xFF

/****************************************************************************
* PLL_REFOSC_CAL_REG, PLL_REFOSC_CAL description
****************************************************************************/

#define PLL_REFOSC_CAL_REG_adr 0x98
#define PLL_REFOSC_CAL_REG_reset 0x00
#define PLL_REFOSC_CAL_REG_mask 0xDF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char PLL_REFOSC_CAL_CTRL  : 5; // Bit 4-0
    unsigned char _reserved_5_         : 1; // Bit 5
    unsigned char PLL_REFOSC_CAL_START  : 1; // Bit 6
    unsigned char PLL_REFOSC_CAL_EN    : 1; // Bit 7
  };
} PLL_REFOSC_CAL_REG_type ;

#define PLL_REFOSC_CAL_PLL_REFOSC_CAL_CTRL  0x001F // Bit 4-0
#define PLL_REFOSC_CAL__reserved_5_         0x0020 // Bit 5
#define PLL_REFOSC_CAL_PLL_REFOSC_CAL_START  0x0040 // Bit 6
#define PLL_REFOSC_CAL_PLL_REFOSC_CAL_EN    0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap8
* Range.: 0x9C..0x9F, 0x4
****************************************************************************/

/****************************************************************************
* DAC_NG_CTRL_REG, DAC_NG_CTRL description
****************************************************************************/

#define DAC_NG_CTRL_REG_adr 0x9C
#define DAC_NG_CTRL_REG_reset 0x00
#define DAC_NG_CTRL_REG_mask 0x80

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 7;
    unsigned char DAC_NG_EN            : 1; // Bit 7
  };
} DAC_NG_CTRL_REG_type ;

#define DAC_NG_CTRL_DAC_NG_EN            0x0080 // Bit 7

/****************************************************************************
* DAC_NG_SETUP_TIME_REG, DAC_NG_SETUP_TIME description
****************************************************************************/

#define DAC_NG_SETUP_TIME_REG_adr 0x9D
#define DAC_NG_SETUP_TIME_REG_reset 0x00
#define DAC_NG_SETUP_TIME_REG_mask 0x0F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAC_NG_SETUP_TIME    : 2; // Bit 1-0
    unsigned char DAC_NG_RAMPUP_RATE   : 1; // Bit 2
    unsigned char DAC_NG_RAMPDN_RATE   : 1; // Bit 3
    unsigned char                      : 4; // Bit 7-4
  };
} DAC_NG_SETUP_TIME_REG_type ;

#define DAC_NG_SETUP_TIME_DAC_NG_SETUP_TIME    0x0003 // Bit 1-0
#define DAC_NG_SETUP_TIME_DAC_NG_RAMPUP_RATE   0x0004 // Bit 2
#define DAC_NG_SETUP_TIME_DAC_NG_RAMPDN_RATE   0x0008 // Bit 3

/****************************************************************************
* DAC_NG_OFF_THRESH_REG, DAC_NG_OFF_THRESH description
****************************************************************************/

#define DAC_NG_OFF_THRESH_REG_adr 0x9E
#define DAC_NG_OFF_THRESH_REG_reset 0x00
#define DAC_NG_OFF_THRESH_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAC_NG_OFF_THRESHOLD  : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} DAC_NG_OFF_THRESH_REG_type ;

#define DAC_NG_OFF_THRESH_DAC_NG_OFF_THRESHOLD  0x0007 // Bit 2-0

/****************************************************************************
* DAC_NG_ON_THRESH_REG, DAC_NG_ON_THRESH description
****************************************************************************/

#define DAC_NG_ON_THRESH_REG_adr 0x9F
#define DAC_NG_ON_THRESH_REG_reset 0x00
#define DAC_NG_ON_THRESH_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DAC_NG_ON_THRESHOLD  : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} DAC_NG_ON_THRESH_REG_type ;

#define DAC_NG_ON_THRESH_DAC_NG_ON_THRESHOLD  0x0007 // Bit 2-0

/****************************************************************************
* Module: memoryMap32
* Range.: 0xA0..0xA8, 0x9
****************************************************************************/

/****************************************************************************
* TONE_GEN_CFG1_REG, TONE_GEN_CFG1 description
****************************************************************************/

#define TONE_GEN_CFG1_REG_adr 0xA0
#define TONE_GEN_CFG1_REG_reset 0x00
#define TONE_GEN_CFG1_REG_mask 0x9F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DTMF_REG             : 4; // Bit 3-0
    unsigned char DTMF_EN              : 1; // Bit 4
    unsigned char _reserved_5_         : 2; // Bit 6-5
    unsigned char START_STOPN          : 1; // Bit 7
  };
} TONE_GEN_CFG1_REG_type ;

#define TONE_GEN_CFG1_DTMF_REG             0x000F // Bit 3-0
#define TONE_GEN_CFG1_DTMF_EN              0x0010 // Bit 4
#define TONE_GEN_CFG1__reserved_5_         0x0060 // Bit 6-5
#define TONE_GEN_CFG1_START_STOPN          0x0080 // Bit 7

/****************************************************************************
* TONE_GEN_CFG2_REG, TONE_GEN_CFG2 description
****************************************************************************/

#define TONE_GEN_CFG2_REG_adr 0xA1
#define TONE_GEN_CFG2_REG_reset 0x00
#define TONE_GEN_CFG2_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SWG_SEL              : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} TONE_GEN_CFG2_REG_type ;

#define TONE_GEN_CFG2_SWG_SEL              0x0003 // Bit 1-0

/****************************************************************************
* TONE_GEN_FREQ1_L_REG, TONE_GEN_FREQ1_L description
****************************************************************************/

#define TONE_GEN_FREQ1_L_REG_adr 0xA2
#define TONE_GEN_FREQ1_L_REG_reset 0x55
#define TONE_GEN_FREQ1_L_REG_mask 0xFF

/****************************************************************************
* TONE_GEN_FREQ1_U_REG, TONE_GEN_FREQ1_U description
****************************************************************************/

#define TONE_GEN_FREQ1_U_REG_adr 0xA3
#define TONE_GEN_FREQ1_U_REG_reset 0x15
#define TONE_GEN_FREQ1_U_REG_mask 0xFF

/****************************************************************************
* TONE_GEN_FREQ2_L_REG, TONE_GEN_FREQ2_L description
****************************************************************************/

#define TONE_GEN_FREQ2_L_REG_adr 0xA4
#define TONE_GEN_FREQ2_L_REG_reset 0x00
#define TONE_GEN_FREQ2_L_REG_mask 0xFF

/****************************************************************************
* TONE_GEN_FREQ2_U_REG, TONE_GEN_FREQ2_U description
****************************************************************************/

#define TONE_GEN_FREQ2_U_REG_adr 0xA5
#define TONE_GEN_FREQ2_U_REG_reset 0x40
#define TONE_GEN_FREQ2_U_REG_mask 0xFF

/****************************************************************************
* TONE_GEN_CYCLES_REG, TONE_GEN_CYCLES description
****************************************************************************/

#define TONE_GEN_CYCLES_REG_adr 0xA6
#define TONE_GEN_CYCLES_REG_reset 0x00
#define TONE_GEN_CYCLES_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char BEEP_CYCLES          : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} TONE_GEN_CYCLES_REG_type ;

#define TONE_GEN_CYCLES_BEEP_CYCLES          0x0007 // Bit 2-0

/****************************************************************************
* TONE_GEN_ON_PER_REG, TONE_GEN_ON_PER description
****************************************************************************/

#define TONE_GEN_ON_PER_REG_adr 0xA7
#define TONE_GEN_ON_PER_REG_reset 0x02
#define TONE_GEN_ON_PER_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char BEEP_ON_PER          : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} TONE_GEN_ON_PER_REG_type ;

#define TONE_GEN_ON_PER_BEEP_ON_PER          0x003F // Bit 5-0

/****************************************************************************
* TONE_GEN_OFF_PER_REG, TONE_GEN_OFF_PER description
****************************************************************************/

#define TONE_GEN_OFF_PER_REG_adr 0xA8
#define TONE_GEN_OFF_PER_REG_reset 0x01
#define TONE_GEN_OFF_PER_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char BEEP_OFF_PER         : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} TONE_GEN_OFF_PER_REG_type ;

#define TONE_GEN_OFF_PER_BEEP_OFF_PER         0x003F // Bit 5-0

/****************************************************************************
* Module: memoryMap4
* Range.: 0xAC..0xAE, 0x3
****************************************************************************/

/****************************************************************************
* CP_CTRL_REG, CP_CTRL description
****************************************************************************/

#define CP_CTRL_REG_adr 0xAC
#define CP_CTRL_REG_reset 0x60
#define CP_CTRL_REG_mask 0xFC

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 2;
    unsigned char CP_MOD               : 2; // Bit 3-2
    unsigned char CP_MCHANGE           : 2; // Bit 5-4
    unsigned char CP_SMALL_SWITCH_FREQ_EN  : 1; // Bit 6
    unsigned char CP_EN                : 1; // Bit 7
  };
} CP_CTRL_REG_type ;

#define CP_CTRL_CP_MOD               0x000C // Bit 3-2
#define CP_CTRL_CP_MCHANGE           0x0030 // Bit 5-4
#define CP_CTRL_CP_SMALL_SWITCH_FREQ_EN  0x0040 // Bit 6
#define CP_CTRL_CP_EN                0x0080 // Bit 7

/****************************************************************************
* CP_DELAY_REG, CP_DELAY description
****************************************************************************/

#define CP_DELAY_REG_adr 0xAD
#define CP_DELAY_REG_reset 0x11
#define CP_DELAY_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CP_FCONTROL          : 3; // Bit 2-0
    unsigned char CP_TAU_DELAY         : 3; // Bit 5-3
    unsigned char                      : 2; // Bit 7-6
  };
} CP_DELAY_REG_type ;

#define CP_DELAY_CP_FCONTROL          0x0007 // Bit 2-0
#define CP_DELAY_CP_TAU_DELAY         0x0038 // Bit 5-3

/****************************************************************************
* CP_VOL_THRESHOLD1_REG, CP_VOL_THRESHOLD1 description
****************************************************************************/

#define CP_VOL_THRESHOLD1_REG_adr 0xAE
#define CP_VOL_THRESHOLD1_REG_reset 0x0E
#define CP_VOL_THRESHOLD1_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char CP_THRESH_VDD2       : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} CP_VOL_THRESHOLD1_REG_type ;

#define CP_VOL_THRESHOLD1_CP_THRESH_VDD2       0x003F // Bit 5-0

/****************************************************************************
* Module: memoryMap21
* Range.: 0xB4..0xBB, 0x8
****************************************************************************/

/****************************************************************************
* MIC_1_CTRL_REG, MIC_1_CTRL description
****************************************************************************/

#define MIC_1_CTRL_REG_adr 0xB4
#define MIC_1_CTRL_REG_reset 0x40
#define MIC_1_CTRL_REG_mask 0xF3

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIC_AMP_BIAS         : 2; // Bit 1-0
    unsigned char _reserved_2_         : 2; // Bit 3-2
    unsigned char MIC_1_AMP_ZC_EN      : 1; // Bit 4
    unsigned char FIELD2               : 1; // Bit 5
    unsigned char MIC_1_AMP_MUTE_EN    : 1; // Bit 6
    unsigned char MIC_1_AMP_EN         : 1; // Bit 7
  };
} MIC_1_CTRL_REG_type ;

#define MIC_1_CTRL_MIC_AMP_BIAS         0x0003 // Bit 1-0
#define MIC_1_CTRL__reserved_2_         0x000C // Bit 3-2
#define MIC_1_CTRL_MIC_1_AMP_ZC_EN      0x0010 // Bit 4
#define MIC_1_CTRL_FIELD2               0x0020 // Bit 5
#define MIC_1_CTRL_MIC_1_AMP_MUTE_EN    0x0040 // Bit 6
#define MIC_1_CTRL_MIC_1_AMP_EN         0x0080 // Bit 7

/****************************************************************************
* MIC_2_CTRL_REG, MIC_2_CTRL description
****************************************************************************/

#define MIC_2_CTRL_REG_adr 0xB8
#define MIC_2_CTRL_REG_reset 0x40
#define MIC_2_CTRL_REG_mask 0xF0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 4;
    unsigned char MIC_2_AMP_ZC_EN      : 1; // Bit 4
    unsigned char FIELD2               : 1; // Bit 5
    unsigned char MIC_2_AMP_MUTE_EN    : 1; // Bit 6
    unsigned char MIC_2_AMP_EN         : 1; // Bit 7
  };
} MIC_2_CTRL_REG_type ;

#define MIC_2_CTRL_MIC_2_AMP_ZC_EN      0x0010 // Bit 4
#define MIC_2_CTRL_FIELD2               0x0020 // Bit 5
#define MIC_2_CTRL_MIC_2_AMP_MUTE_EN    0x0040 // Bit 6
#define MIC_2_CTRL_MIC_2_AMP_EN         0x0080 // Bit 7

/****************************************************************************
* MIC_1_GAIN_REG, MIC_1_GAIN description
****************************************************************************/

#define MIC_1_GAIN_REG_adr 0xB5
#define MIC_1_GAIN_REG_reset 0x01
#define MIC_1_GAIN_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIC_1_AMP_GAIN       : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} MIC_1_GAIN_REG_type ;

#define MIC_1_GAIN_MIC_1_AMP_GAIN       0x0007 // Bit 2-0

/****************************************************************************
* MIC_2_GAIN_REG, MIC_2_GAIN description
****************************************************************************/

#define MIC_2_GAIN_REG_adr 0xB9
#define MIC_2_GAIN_REG_reset 0x01
#define MIC_2_GAIN_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIC_2_AMP_GAIN       : 3; // Bit 2-0
    unsigned char                      : 5; // Bit 7-3
  };
} MIC_2_GAIN_REG_type ;

#define MIC_2_GAIN_MIC_2_AMP_GAIN       0x0007 // Bit 2-0

/****************************************************************************
* MIC_1_SELECT_REG, MIC_1_SELECT description
****************************************************************************/

#define MIC_1_SELECT_REG_adr 0xB7
#define MIC_1_SELECT_REG_reset 0x00
#define MIC_1_SELECT_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIC_1_AMP_IN_SEL     : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} MIC_1_SELECT_REG_type ;

#define MIC_1_SELECT_MIC_1_AMP_IN_SEL     0x0003 // Bit 1-0

/****************************************************************************
* MIC_2_SELECT_REG, MIC_2_SELECT description
****************************************************************************/

#define MIC_2_SELECT_REG_adr 0xBB
#define MIC_2_SELECT_REG_reset 0x00
#define MIC_2_SELECT_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIC_2_AMP_IN_SEL     : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} MIC_2_SELECT_REG_type ;

#define MIC_2_SELECT_MIC_2_AMP_IN_SEL     0x0003 // Bit 1-0

/****************************************************************************
* Module: memoryMap17
* Range.: 0xBC..0xBD, 0x2
****************************************************************************/

/****************************************************************************
* IN_1_HPF_FILTER_CTRL_REG, IN_1_FILTER_CTRL description
****************************************************************************/

#define IN_1_HPF_FILTER_CTRL_REG_adr 0xBC
#define IN_1_HPF_FILTER_CTRL_REG_reset 0x80
#define IN_1_HPF_FILTER_CTRL_REG_mask 0xBF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_VOICE_HPF_CORNER  : 3; // Bit 2-0
    unsigned char IN_VOICE_EN          : 1; // Bit 3
    unsigned char IN_AUDIO_HPF_CORNER  : 2; // Bit 5-4
    unsigned char _reserved_6_         : 1; // Bit 6
    unsigned char IN_HPF_EN            : 1; // Bit 7
  };
} IN_1_HPF_FILTER_CTRL_REG_type ;

#define IN_1_HPF_FILTER_CTRL_IN_1_VOICE_HPF_CORNER  0x0007 // Bit 2-0
#define IN_1_HPF_FILTER_CTRL_IN_1_VOICE_EN        0x0008 // Bit 3
#define IN_1_HPF_FILTER_CTRL_IN_1_AUDIO_HPF_CORNER  0x0030 // Bit 5-4
#define IN_1_HPF_FILTER_CTRL__reserved_6_         0x0040 // Bit 6
#define IN_1_HPF_FILTER_CTRL_IN_1_HPF_EN          0x0080 // Bit 7

/****************************************************************************
* IN_2_HPF_FILTER_CTRL_REG, IN_2_FILTER_CTRL description
****************************************************************************/

#define IN_2_HPF_FILTER_CTRL_REG_adr 0xBD
#define IN_2_HPF_FILTER_CTRL_REG_reset 0x80
#define IN_2_HPF_FILTER_CTRL_REG_mask 0xBF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_2_VOICE_HPF_CORNER  : 3; // Bit 2-0
    unsigned char IN_2_VOICE_EN        : 1; // Bit 3
    unsigned char IN_2_AUDIO_HPF_CORNER  : 2; // Bit 5-4
    unsigned char _reserved_6_         : 1; // Bit 6
    unsigned char IN_2_HPF_EN          : 1; // Bit 7
  };
} IN_2_HPF_FILTER_CTRL_REG_type ;

#define IN_2_HPF_FILTER_CTRL_IN_2_VOICE_HPF_CORNER  0x0007 // Bit 2-0
#define IN_2_HPF_FILTER_CTRL_IN_2_VOICE_EN        0x0008 // Bit 3
#define IN_2_HPF_FILTER_CTRL_IN_2_AUDIO_HPF_CORNER  0x0030 // Bit 5-4
#define IN_2_HPF_FILTER_CTRL__reserved_6_         0x0040 // Bit 6
#define IN_2_HPF_FILTER_CTRL_IN_2_HPF_EN          0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap0
* Range.: 0xC0..0xC2, 0x3
****************************************************************************/

/****************************************************************************
* ADC_1_CTRL_REG, ADC_1L_CTRL description
****************************************************************************/

#define ADC_1_CTRL_REG_adr 0xC0
#define ADC_1_CTRL_REG_reset 0x07
#define ADC_1_CTRL_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ADC_1_DWA_EN         : 1; // Bit 0
    unsigned char ADC_1_DWABBL_EN      : 1; // Bit 1
    unsigned char ADC_1_AAF_EN         : 1; // Bit 2
    unsigned char                      : 5; // Bit 7-3
  };
} ADC_1_CTRL_REG_type ;

#define ADC_1_CTRL_ADC_1_DWA_EN         0x0001 // Bit 0
#define ADC_1_CTRL_ADC_1_DWABBL_EN      0x0002 // Bit 1
#define ADC_1_CTRL_ADC_1_AAF_EN         0x0004 // Bit 2

/****************************************************************************
* ADC_2_CTRL_REG, ADC_2L_CTRL description
****************************************************************************/

#define ADC_2_CTRL_REG_adr 0xC1
#define ADC_2_CTRL_REG_reset 0x07
#define ADC_2_CTRL_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ADC_2_DWA_EN         : 1; // Bit 0
    unsigned char ADC_2_DWABBL_EN      : 1; // Bit 1
    unsigned char ADC_2_AAF_EN         : 1; // Bit 2
    unsigned char                      : 5; // Bit 7-3
  };
} ADC_2_CTRL_REG_type ;

#define ADC_2_CTRL_ADC_2_DWA_EN         0x0001 // Bit 0
#define ADC_2_CTRL_ADC_2_DWABBL_EN      0x0002 // Bit 1
#define ADC_2_CTRL_ADC_2_AAF_EN         0x0004 // Bit 2

/****************************************************************************
* ADC_MODE_REG, ADC_MODE description
****************************************************************************/

#define ADC_MODE_REG_adr 0xC2
#define ADC_MODE_REG_reset 0x00
#define ADC_MODE_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char ADC_LP_MODE          : 1; // Bit 0
    unsigned char ADC_LVLDET_MODE      : 1; // Bit 1
    unsigned char ADC_LVLDET_AUTO_EXIT  : 1; // Bit 2
    unsigned char                      : 5; // Bit 7-3
  };
} ADC_MODE_REG_type ;

#define ADC_MODE_ADC_LP_MODE          0x0001 // Bit 0
#define ADC_MODE_ADC_LVLDET_MODE      0x0002 // Bit 1
#define ADC_MODE_ADC_LVLDET_AUTO_EXIT  0x0004 // Bit 2

/****************************************************************************
* Module: memoryMap24
* Range.: 0xCC..0xCF, 0x4
****************************************************************************/

/****************************************************************************
* MIXOUT_L_CTRL_REG, MIXOUT_L_CTRL description
****************************************************************************/

#define MIXOUT_L_CTRL_REG_adr 0xCC
#define MIXOUT_L_CTRL_REG_reset 0x00
#define MIXOUT_L_CTRL_REG_mask 0x83

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXOUT_AMP_BIAS      : 2; // Bit 1-0
    unsigned char _reserved_2_         : 5; // Bit 6-2
    unsigned char MIXOUT_L_AMP_EN      : 1; // Bit 7
  };
} MIXOUT_L_CTRL_REG_type ;

#define MIXOUT_L_CTRL_MIXOUT_AMP_BIAS      0x0003 // Bit 1-0
#define MIXOUT_L_CTRL__reserved_2_         0x007C // Bit 6-2
#define MIXOUT_L_CTRL_MIXOUT_L_AMP_EN      0x0080 // Bit 7

/****************************************************************************
* MIXOUT_L_GAIN_REG, MIXOUT_L_GAIN description
****************************************************************************/

#define MIXOUT_L_GAIN_REG_adr 0xCD
#define MIXOUT_L_GAIN_REG_reset 0x03
#define MIXOUT_L_GAIN_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXOUT_L_AMP_GAIN    : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} MIXOUT_L_GAIN_REG_type ;

#define MIXOUT_L_GAIN_MIXOUT_L_AMP_GAIN    0x0003 // Bit 1-0

/****************************************************************************
* MIXOUT_R_CTRL_REG, MIXOUT_R_CTRL description
****************************************************************************/

#define MIXOUT_R_CTRL_REG_adr 0xCE
#define MIXOUT_R_CTRL_REG_reset 0x00
#define MIXOUT_R_CTRL_REG_mask 0x80

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 7;
    unsigned char MIXOUT_R_AMP_EN      : 1; // Bit 7
  };
} MIXOUT_R_CTRL_REG_type ;

#define MIXOUT_R_CTRL_MIXOUT_R_AMP_EN      0x0080 // Bit 7

/****************************************************************************
* MIXOUT_R_GAIN_REG, MIXOUT_R_GAIN description
****************************************************************************/

#define MIXOUT_R_GAIN_REG_adr 0xCF
#define MIXOUT_R_GAIN_REG_reset 0x03
#define MIXOUT_R_GAIN_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MIXOUT_R_AMP_GAIN    : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} MIXOUT_R_GAIN_REG_type ;

#define MIXOUT_R_GAIN_MIXOUT_R_AMP_GAIN    0x0003 // Bit 1-0

/****************************************************************************
* Module: memoryMap14
* Range.: 0xD0..0xD7, 0x8
****************************************************************************/

/****************************************************************************
* HP_L_CTRL_REG, HP_L_CTRL description
****************************************************************************/

#define HP_L_CTRL_REG_adr 0xD0
#define HP_L_CTRL_REG_reset 0x40
#define HP_L_CTRL_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HP_AMP_BIAS          : 2; // Bit 1-0
    unsigned char HP_L_AMP_MIN_GAIN_EN  : 1; // Bit 2
    unsigned char HP_L_AMP_OE          : 1; // Bit 3
    unsigned char HP_L_AMP_ZC_EN       : 1; // Bit 4
    unsigned char HP_L_AMP_RAMP_EN     : 1; // Bit 5
    unsigned char HP_L_AMP_MUTE_EN     : 1; // Bit 6
    unsigned char HP_L_AMP_EN          : 1; // Bit 7
  };
} HP_L_CTRL_REG_type ;

#define HP_L_CTRL_HP_AMP_BIAS          0x0003 // Bit 1-0
#define HP_L_CTRL_HP_L_AMP_MIN_GAIN_EN  0x0004 // Bit 2
#define HP_L_CTRL_HP_L_AMP_OE          0x0008 // Bit 3
#define HP_L_CTRL_HP_L_AMP_ZC_EN       0x0010 // Bit 4
#define HP_L_CTRL_HP_L_AMP_RAMP_EN     0x0020 // Bit 5
#define HP_L_CTRL_HP_L_AMP_MUTE_EN     0x0040 // Bit 6
#define HP_L_CTRL_HP_L_AMP_EN          0x0080 // Bit 7

/****************************************************************************
* HP_L_GAIN_REG, HP_L_GAIN description
****************************************************************************/

#define HP_L_GAIN_REG_adr 0xD1
#define HP_L_GAIN_REG_reset 0x3B
#define HP_L_GAIN_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HP_L_AMP_GAIN        : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} HP_L_GAIN_REG_type ;

#define HP_L_GAIN_HP_L_AMP_GAIN        0x003F // Bit 5-0

/****************************************************************************
* HP_R_CTRL_REG, HP_R_CTRL description
****************************************************************************/

#define HP_R_CTRL_REG_adr 0xD2
#define HP_R_CTRL_REG_reset 0x40
#define HP_R_CTRL_REG_mask 0xFC

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 2;
    unsigned char HP_R_AMP_MIN_GAIN_EN  : 1; // Bit 2
    unsigned char HP_R_AMP_OE          : 1; // Bit 3
    unsigned char HP_R_AMP_ZC_EN       : 1; // Bit 4
    unsigned char HP_R_AMP_RAMP_EN     : 1; // Bit 5
    unsigned char HP_R_AMP_MUTE_EN     : 1; // Bit 6
    unsigned char HP_R_AMP_EN          : 1; // Bit 7
  };
} HP_R_CTRL_REG_type ;

#define HP_R_CTRL_HP_R_AMP_MIN_GAIN_EN  0x0004 // Bit 2
#define HP_R_CTRL_HP_R_AMP_OE          0x0008 // Bit 3
#define HP_R_CTRL_HP_R_AMP_ZC_EN       0x0010 // Bit 4
#define HP_R_CTRL_HP_R_AMP_RAMP_EN     0x0020 // Bit 5
#define HP_R_CTRL_HP_R_AMP_MUTE_EN     0x0040 // Bit 6
#define HP_R_CTRL_HP_R_AMP_EN          0x0080 // Bit 7

/****************************************************************************
* HP_R_GAIN_REG, HP_R_GAIN description
****************************************************************************/

#define HP_R_GAIN_REG_adr 0xD3
#define HP_R_GAIN_REG_reset 0x3B
#define HP_R_GAIN_REG_mask 0x3F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HP_R_AMP_GAIN        : 6; // Bit 5-0
    unsigned char                      : 2; // Bit 7-6
  };
} HP_R_GAIN_REG_type ;

#define HP_R_GAIN_HP_R_AMP_GAIN        0x003F // Bit 5-0

/****************************************************************************
* HP_SNGL_CTRL_REG, HP_CTRL description
****************************************************************************/

#define HP_SNGL_CTRL_REG_adr 0xD4
#define HP_SNGL_CTRL_REG_reset 0x00
#define HP_SNGL_CTRL_REG_mask 0xC7

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HP_AMP_STEREO_DETECT_STATUS  : 1; // Bit 0
    unsigned char HPL_AMP_LOAD_DETECT_STATUS  : 1; // Bit 1
    unsigned char HPR_AMP_LOAD_DETECT_STATUS  : 1; // Bit 2
    unsigned char _reserved_3_         : 3; // Bit 5-3
    unsigned char HP_AMP_LOAD_DETECT_EN  : 1; // Bit 6
    unsigned char HP_AMP_STEREO_DETECT_EN  : 1; // Bit 7
  };
} HP_SNGL_CTRL_REG_type ;

#define HP_SNGL_CTRL_HP_AMP_STEREO_DETECT_STATUS  0x0001 // Bit 0
#define HP_SNGL_CTRL_HPL_AMP_LOAD_DETECT_STATUS  0x0002 // Bit 1
#define HP_SNGL_CTRL_HPR_AMP_LOAD_DETECT_STATUS  0x0004 // Bit 2
#define HP_SNGL_CTRL__reserved_3_         0x0038 // Bit 5-3
#define HP_SNGL_CTRL_HP_AMP_LOAD_DETECT_EN  0x0040 // Bit 6
#define HP_SNGL_CTRL_HP_AMP_STEREO_DETECT_EN  0x0080 // Bit 7

/****************************************************************************
* HP_DIFF_CTRL_REG, HP_DIFF_CTRL description
****************************************************************************/

#define HP_DIFF_CTRL_REG_adr 0xD5
#define HP_DIFF_CTRL_REG_reset 0x00
#define HP_DIFF_CTRL_REG_mask 0x11

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HP_AMP_DIFF_MODE_EN  : 1; // Bit 0
    unsigned char _reserved_1_         : 3; // Bit 3-1
    unsigned char HP_AMP_SINGLE_SUPPLY_EN  : 1; // Bit 4
    unsigned char                      : 3; // Bit 7-5
  };
} HP_DIFF_CTRL_REG_type ;

#define HP_DIFF_CTRL_HP_AMP_DIFF_MODE_EN  0x0001 // Bit 0
#define HP_DIFF_CTRL__reserved_1_         0x000E // Bit 3-1
#define HP_DIFF_CTRL_HP_AMP_SINGLE_SUPPLY_EN  0x0010 // Bit 4

/****************************************************************************
* HP_DIFF_UNLOCK_REG, HP_DIFF_UNLOCK description
****************************************************************************/

#define HP_DIFF_UNLOCK_REG_adr 0xD7
#define HP_DIFF_UNLOCK_REG_reset 0xC3
#define HP_DIFF_UNLOCK_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char FIELD0               : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} HP_DIFF_UNLOCK_REG_type ;

#define HP_DIFF_UNLOCK_FIELD0               0x0001 // Bit 0

/****************************************************************************
* Module: memoryMap15
* Range.: 0xD8..0xDA, 0x3
****************************************************************************/

/****************************************************************************
* HPLDET_JACK_REG, HPLDET_JACK description
****************************************************************************/

#define HPLDET_JACK_REG_adr 0xD8
#define HPLDET_JACK_REG_reset 0x0B
#define HPLDET_JACK_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HPLDET_JACK_RATE     : 3; // Bit 2-0
    unsigned char HPLDET_JACK_DEBOUNCE  : 2; // Bit 4-3
    unsigned char HPLDET_JACK_THR      : 2; // Bit 6-5
    unsigned char HPLDET_JACK_EN       : 1; // Bit 7
  };
} HPLDET_JACK_REG_type ;

#define HPLDET_JACK_HPLDET_JACK_RATE     0x0007 // Bit 2-0
#define HPLDET_JACK_HPLDET_JACK_DEBOUNCE  0x0018 // Bit 4-3
#define HPLDET_JACK_HPLDET_JACK_THR      0x0060 // Bit 6-5
#define HPLDET_JACK_HPLDET_JACK_EN       0x0080 // Bit 7

/****************************************************************************
* HPLDET_CTRL_REG, HPLDET_JACK description
****************************************************************************/

#define HPLDET_CTRL_REG_adr 0xD9
#define HPLDET_CTRL_REG_reset 0x00
#define HPLDET_CTRL_REG_mask 0x83

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char HPLDET_COMP_INV      : 1; // Bit 0
    unsigned char HPLDET_HYST_EN       : 1; // Bit 1
    unsigned char _reserved_2_         : 5; // Bit 6-2
    unsigned char HPLDET_DISCHARGE_EN  : 1; // Bit 7
  };
} HPLDET_CTRL_REG_type ;

#define HPLDET_CTRL_HPLDET_COMP_INV      0x0001 // Bit 0
#define HPLDET_CTRL_HPLDET_HYST_EN       0x0002 // Bit 1
#define HPLDET_CTRL__reserved_2_         0x007C // Bit 6-2
#define HPLDET_CTRL_HPLDET_DISCHARGE_EN  0x0080 // Bit 7

/****************************************************************************
* HPLDET_TEST_REG, HPLDET_TEST description
****************************************************************************/

#define HPLDET_TEST_REG_adr 0xDA
#define HPLDET_TEST_REG_reset 0x00
#define HPLDET_TEST_REG_mask 0x10

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 4;
    unsigned char HPLDET_COMP_STS      : 1; // Bit 4
    unsigned char                      : 3; // Bit 7-5
  };
} HPLDET_TEST_REG_type ;

#define HPLDET_TEST_HPLDET_COMP_STS      0x0010 // Bit 4

/****************************************************************************
* Module: memoryMap28
* Range.: 0xDC..0xDC, 0x1
****************************************************************************/

/****************************************************************************
* REFERENCES_REG, REFERENCES description
****************************************************************************/

#define REFERENCES_REG_adr 0xDC
#define REFERENCES_REG_reset 0x08
#define REFERENCES_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char BIAS_LEVEL           : 2; // Bit 1-0
    unsigned char VREFBUF_BIAS         : 1; // Bit 2
    unsigned char BIAS_EN              : 1; // Bit 3
    unsigned char VMID_FAST_CHARGE     : 1; // Bit 4
    unsigned char                      : 3; // Bit 7-5
  };
} REFERENCES_REG_type ;

#define REFERENCES_BIAS_LEVEL           0x0003 // Bit 1-0
#define REFERENCES_VREFBUF_BIAS         0x0004 // Bit 2
#define REFERENCES_BIAS_EN              0x0008 // Bit 3
#define REFERENCES_VMID_FAST_CHARGE     0x0010 // Bit 4

/****************************************************************************
* Module: memoryMap19
* Range.: 0xE0..0xE1, 0x2
****************************************************************************/

/****************************************************************************
* IO_CTRL_REG, IO_CTRL description
****************************************************************************/

#define IO_CTRL_REG_adr 0xE0
#define IO_CTRL_REG_reset 0x00
#define IO_CTRL_REG_mask 0x01

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IO_VOLTAGE_LEVEL     : 1; // Bit 0
    unsigned char                      : 7; // Bit 7-1
  };
} IO_CTRL_REG_type ;

#define IO_CTRL_IO_VOLTAGE_LEVEL     0x0001 // Bit 0

/****************************************************************************
* LDO_CTRL_REG, LDO_CTRL description
****************************************************************************/

#define LDO_CTRL_REG_adr 0xE1
#define LDO_CTRL_REG_reset 0x00
#define LDO_CTRL_REG_mask 0xB0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 4;
    unsigned char LDO_LEVEL_SELECT     : 2; // Bit 5-4
    unsigned char _reserved_6_         : 1; // Bit 6
    unsigned char LDO_EN               : 1; // Bit 7
  };
} LDO_CTRL_REG_type ;

#define LDO_CTRL_LDO_LEVEL_SELECT     0x0030 // Bit 5-4
#define LDO_CTRL__reserved_6_         0x0040 // Bit 6
#define LDO_CTRL_LDO_EN               0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap30
* Range.: 0xE4..0xEB, 0x8
****************************************************************************/

/****************************************************************************
* SIDETONE_CTRL_REG, SIDETONE_CTRL description
****************************************************************************/

#define SIDETONE_CTRL_REG_adr 0xE4
#define SIDETONE_CTRL_REG_reset 0x40
#define SIDETONE_CTRL_REG_mask 0xC0

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 6;
    unsigned char SIDETONE_MUTE_EN     : 1; // Bit 6
    unsigned char SIDETONE_FILTER_EN   : 1; // Bit 7
  };
} SIDETONE_CTRL_REG_type ;

#define SIDETONE_CTRL_SIDETONE_MUTE_EN     0x0040 // Bit 6
#define SIDETONE_CTRL_SIDETONE_FILTER_EN   0x0080 // Bit 7

/****************************************************************************
* DROUTING_ST_OUTFILT_1L_REG, DROUTING_SIDETONE_OUTFILT_1R description
****************************************************************************/

#define DROUTING_ST_OUTFILT_1L_REG_adr 0xE8
#define DROUTING_ST_OUTFILT_1L_REG_reset 0x01
#define DROUTING_ST_OUTFILT_1L_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OutFilter1LSrc       :1; //bit 0 = Output filter 1l
    unsigned char OutFilter1RSrc       :1; //bit 1 = Output filter 1r
    unsigned char SideToneFileterSrc   :1; //bit 2 = SideTone
    unsigned char                      : 5; // Bit 7-3
  };
} DROUTING_ST_OUTFILT_1L_REG_type ;

#define DROUTING_ST_OUTFILT_1L_OUTFILT_ST_1L_SRC    0x0007 // Bit 2-0

/****************************************************************************
* SIDETONE_IN_SELECT_REG, SIDETONE_IN_SELECT description
****************************************************************************/

#define SIDETONE_IN_SELECT_REG_adr 0xE5
#define SIDETONE_IN_SELECT_REG_reset 0x00
#define SIDETONE_IN_SELECT_REG_mask 0x03

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SIDETONE_IN_SELECT   : 2; // Bit 1-0
    unsigned char                      : 6; // Bit 7-2
  };
} SIDETONE_IN_SELECT_REG_type ;

#define SIDE_TONE_SELCET_ADC_1L 0
#define SIDE_TONE_SELCET_ADC_1R 1
#define SIDE_TONE_SELCET_ADC_2L 2
#define SIDE_TONE_SELCET_ADC_2R 3

#define SIDETONE_IN_SELECT_SIDETONE_IN_SELECT   0x0003 // Bit 1-0

/****************************************************************************
* DROUTING_ST_OUTFILT_1R_REG, DROUTING_SIDETONE_OUTFILT_1R description
****************************************************************************/

#define DROUTING_ST_OUTFILT_1R_REG_adr 0xE9
#define DROUTING_ST_OUTFILT_1R_REG_reset 0x02
#define DROUTING_ST_OUTFILT_1R_REG_mask 0x07

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char OutFilter1LSrc     :1; //bit 0 = Output filter 1l
    unsigned char OutFilter1RSrc     :1; //bit 1 = Output filter 1r
    unsigned char SideToneFileterSrc :1; //bit 2 = SideTone
    unsigned char                    : 5; // Bit 7-3
  };
} DROUTING_ST_OUTFILT_1R_REG_type ;

#define DROUTING_ST_OUTFILT_1R_OUTFILT_ST_1R_SRC    0x0007 // Bit 2-0

/****************************************************************************
* SIDETONE_GAIN_REG, SIDETONE_GAIN description
****************************************************************************/

#define SIDETONE_GAIN_REG_adr 0xE6
#define SIDETONE_GAIN_REG_reset 0x1C
#define SIDETONE_GAIN_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SIDETONE_GAIN        : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} SIDETONE_GAIN_REG_type ;

#define SIDETONE_GAIN_SIDETONE_GAIN        0x001F // Bit 4-0

/****************************************************************************
* SIDETONE_BIQ_3STAGE_DATA_REG, BBQ_DATA description
****************************************************************************/

#define SIDETONE_BIQ_3STAGE_DATA_REG_adr 0xEA
#define SIDETONE_BIQ_3STAGE_DATA_REG_reset 0x00
#define SIDETONE_BIQ_3STAGE_DATA_REG_mask 0xFF

/****************************************************************************
* SIDETONE_BIQ_3STAGE_ADDR_REG, BBQ_ADDR description
****************************************************************************/

#define SIDETONE_BIQ_3STAGE_ADDR_REG_adr 0xEB
#define SIDETONE_BIQ_3STAGE_ADDR_REG_reset 0x00
#define SIDETONE_BIQ_3STAGE_ADDR_REG_mask 0x1F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char SIDETONE_BIQ_3STAGE_ADDR  : 5; // Bit 4-0
    unsigned char                      : 3; // Bit 7-5
  };
} SIDETONE_BIQ_3STAGE_ADDR_REG_type ;

#define SIDETONE_BIQ_3STAGE_ADDR_SIDETONE_BIQ_3STAGE_ADDR  0x001F // Bit 4-0

/****************************************************************************
* Module: memoryMap18
* Range.: 0xEC..0xEE, 0x3
****************************************************************************/

/****************************************************************************
* EVENT_STATUS_REG, EVENT_STATUS description
****************************************************************************/

#define EVENT_STATUS_REG_adr 0xEC
#define EVENT_STATUS_REG_reset 0x00
#define EVENT_STATUS_REG_mask 0x80

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char          : 7;
    unsigned char HPLDET_JACK_STS      : 1; // Bit 7
  };
} EVENT_STATUS_REG_type ;

#define EVENT_STATUS_HPLDET_JACK_STS      0x0080 // Bit 7

/****************************************************************************
* EVENT_REG, EVENT description
****************************************************************************/

#define EVENT_REG_adr 0xED
#define EVENT_REG_reset 0x00
#define EVENT_REG_mask 0x81

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char LVL_DET_EVENT        : 1; // Bit 0
    unsigned char _reserved_1_         : 6; // Bit 6-1
    unsigned char HPLDET_JACK_EVENT    : 1; // Bit 7
  };
} EVENT_REG_type ;

#define EVENT_LVL_DET_EVENT        0x0001 // Bit 0
#define EVENT__reserved_1_         0x007E // Bit 6-1
#define EVENT_HPLDET_JACK_EVENT    0x0080 // Bit 7

/****************************************************************************
* EVENT_MASK_REG, EVENT_MASK description
****************************************************************************/

#define EVENT_MASK_REG_adr 0xEE
#define EVENT_MASK_REG_reset 0x00
#define EVENT_MASK_REG_mask 0x81

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char LVL_DET_EVENT_MSK    : 1; // Bit 0
    unsigned char _reserved_1_         : 6; // Bit 6-1
    unsigned char HPLDET_JACK_EVENT_IRQ_MSK  : 1; // Bit 7
  };
} EVENT_MASK_REG_type ;

#define EVENT_MASK_LVL_DET_EVENT_MSK    0x0001 // Bit 0
#define EVENT_MASK__reserved_1_         0x007E // Bit 6-1
#define EVENT_MASK_HPLDET_JACK_EVENT_IRQ_MSK  0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap12
* Range.: 0xF0..0xF1, 0x2
****************************************************************************/

/****************************************************************************
* DMIC_1_CTRL_REG, MIC_CONFIG description
****************************************************************************/

#define DMIC_1_CTRL_REG_adr 0xF0
#define DMIC_1_CTRL_REG_reset 0x00
#define DMIC_1_CTRL_REG_mask 0xC7

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DMIC_1_DATA_SEL      : 1; // Bit 0
    unsigned char DMIC_1_SAMPLEPHASE   : 1; // Bit 1
    unsigned char DMIC_1_CLK_RATE      : 1; // Bit 2
    unsigned char _reserved_3_         : 3; // Bit 5-3
    unsigned char DMIC_1L_EN           : 1; // Bit 6
    unsigned char DMIC_1R_EN           : 1; // Bit 7
  };
} DMIC_1_CTRL_REG_type ;

#define DMIC_1_CTRL_DMIC_1_DATA_SEL      0x0001 // Bit 0
#define DMIC_1_CTRL_DMIC_1_SAMPLEPHASE   0x0002 // Bit 1
#define DMIC_1_CTRL_DMIC_1_CLK_RATE      0x0004 // Bit 2
#define DMIC_1_CTRL__reserved_3_         0x0038 // Bit 5-3
#define DMIC_1_CTRL_DMIC_1L_EN           0x0040 // Bit 6
#define DMIC_1_CTRL_DMIC_1R_EN           0x0080 // Bit 7

/****************************************************************************
* DMIC_2_CTRL_REG, MIC_CONFIG description
****************************************************************************/

#define DMIC_2_CTRL_REG_adr 0xF1
#define DMIC_2_CTRL_REG_reset 0x00
#define DMIC_2_CTRL_REG_mask 0xC7

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char DMIC_2_DATA_SEL      : 1; // Bit 0
    unsigned char DMIC_2_SAMPLEPHASE   : 1; // Bit 1
    unsigned char DMIC_2_CLK_RATE      : 1; // Bit 2
    unsigned char _reserved_3_         : 3; // Bit 5-3
    unsigned char DMIC_2L_EN           : 1; // Bit 6
    unsigned char DMIC_2R_EN           : 1; // Bit 7
  };
} DMIC_2_CTRL_REG_type ;

#define DMIC_2_CTRL_DMIC_2_DATA_SEL      0x0001 // Bit 0
#define DMIC_2_CTRL_DMIC_2_SAMPLEPHASE   0x0002 // Bit 1
#define DMIC_2_CTRL_DMIC_2_CLK_RATE      0x0004 // Bit 2
#define DMIC_2_CTRL__reserved_3_         0x0038 // Bit 5-3
#define DMIC_2_CTRL_DMIC_2L_EN           0x0040 // Bit 6
#define DMIC_2_CTRL_DMIC_2R_EN           0x0080 // Bit 7

/****************************************************************************
* Module: memoryMap11
* Range.: 0xF4..0xF9, 0x6
****************************************************************************/

/****************************************************************************
* IN_1L_GAIN_REG, IN_1L_GAIN description
****************************************************************************/

#define IN_1L_GAIN_REG_adr 0xF4
#define IN_1L_GAIN_REG_reset 0x6F
#define IN_1L_GAIN_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_1L_DIGITAL_GAIN   : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} IN_1L_GAIN_REG_type ;

#define IN_1L_GAIN_IN_1L_DIGITAL_GAIN   0x007F // Bit 6-0

/****************************************************************************
* OUT_1L_GAIN_REG, OUT_1L_GAIN description
****************************************************************************/

#define OUT_1L_GAIN_REG_adr 0xF8
#define OUT_1L_GAIN_REG_reset 0x6F
#define OUT_1L_GAIN_REG_mask 0xFF

/****************************************************************************
* IN_1R_GAIN_REG, IN_1R_GAIN description
****************************************************************************/

#define IN_1R_GAIN_REG_adr 0xF5
#define IN_1R_GAIN_REG_reset 0x6F
#define IN_1R_GAIN_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_1R_DIGITAL_GAIN   : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} IN_1R_GAIN_REG_type ;

#define IN_1R_GAIN_IN_1R_DIGITAL_GAIN   0x007F // Bit 6-0

/****************************************************************************
* OUT_1R_GAIN_REG, OUT_1R_GAIN description
****************************************************************************/

#define OUT_1R_GAIN_REG_adr 0xF9
#define OUT_1R_GAIN_REG_reset 0x6F
#define OUT_1R_GAIN_REG_mask 0xFF

/****************************************************************************
* IN_2L_GAIN_REG, IN_2L_GAIN description
****************************************************************************/

#define IN_2L_GAIN_REG_adr 0xF6
#define IN_2L_GAIN_REG_reset 0x6F
#define IN_2L_GAIN_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_2L_DIGITAL_GAIN   : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} IN_2L_GAIN_REG_type ;

#define IN_2L_GAIN_IN_2L_DIGITAL_GAIN   0x007F // Bit 6-0

/****************************************************************************
* IN_2R_GAIN_REG, IN_2R_GAIN description
****************************************************************************/

#define IN_2R_GAIN_REG_adr 0xF7
#define IN_2R_GAIN_REG_reset 0x6F
#define IN_2R_GAIN_REG_mask 0x7F

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char IN_2R_DIGITAL_GAIN   : 7; // Bit 6-0
    unsigned char                      : 1; // Bit 7
  };
} IN_2R_GAIN_REG_type ;

#define IN_2R_GAIN_IN_2R_DIGITAL_GAIN   0x007F // Bit 6-0

/****************************************************************************
* Module: memoryMap22
* Range.: 0xFC..0xFD, 0x2
****************************************************************************/

/****************************************************************************
* MICBIAS_CTRL_REG, MICBIAS_CTRL description
****************************************************************************/

#define MICBIAS_CTRL_REG_adr 0xFC
#define MICBIAS_CTRL_REG_reset 0x00
#define MICBIAS_CTRL_REG_mask 0xFF

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MICBIAS_1_LEVEL      : 3; // Bit 2-0
    unsigned char MICBIAS_1_LP_MODE    : 1; // Bit 3
    unsigned char MICBIAS_2_LEVEL      : 3; // Bit 6-4
    unsigned char MICBIAS_2_LP_MODE    : 1; // Bit 7
  };
} MICBIAS_CTRL_REG_type ;

#define MICBIAS_CTRL_MICBIAS_1_LEVEL      0x0007 // Bit 2-0
#define MICBIAS_CTRL_MICBIAS_1_LP_MODE    0x0008 // Bit 3
#define MICBIAS_CTRL_MICBIAS_2_LEVEL      0x0070 // Bit 6-4
#define MICBIAS_CTRL_MICBIAS_2_LP_MODE    0x0080 // Bit 7

/****************************************************************************
* MICBIAS_EN_REG, MICBIAS_EN description
****************************************************************************/

#define MICBIAS_EN_REG_adr 0xFD
#define MICBIAS_EN_REG_reset 0x00
#define MICBIAS_EN_REG_mask 0x11

typedef union
{
  unsigned char Value;
  struct
  {
    unsigned char MICBIAS_1_EN         : 1; // Bit 0
    unsigned char _reserved_1_         : 3; // Bit 3-1
    unsigned char MICBIAS_2_EN         : 1; // Bit 4
    unsigned char                      : 3; // Bit 7-5
  };
} MICBIAS_EN_REG_type ;

#define MICBIAS_EN_MICBIAS_1_EN         0x0001 // Bit 0
#define MICBIAS_EN__reserved_1_         0x000E // Bit 3-1
#define MICBIAS_EN_MICBIAS_2_EN         0x0010 // Bit 4

#endif

