/**
 ****************************************************************************************
 *
 * @file DA7218_driver.c
 *
 * @brief Audio codec driver source file.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>

#include "osal.h"
#include "hw_gpio.h"
#include "hw_i2c.h"

#include "DA7218_regs.h"
#include "periph_setup.h"
#include "DA7218_driver.h"

/****************************************************************************************************************
 * Defines
 ****************************************************************************************************************/
#define _DEFAULT_GAIN         (DA7218_MAX_GAIN-4)

/*
 * DA7217/8 audio codec digital audio interface (DAI) configuration and sample rate
 */
#define DAI_32BIT_48KHz         (0)
#define DAI_24BIT_48KHz         (1)
#define DAI_16BIT_48KHz         (2)
#define DAI_16BIT_16KHz         (3)
#define DAI_16BIT_8KHz          (4)

#define _AUDIO_CODE_CONF        (DAI_16BIT_16KHz)

#if (_AUDIO_CODE_CONF == DAI_32BIT_48KHz)
#define SR_REG_VALUE            (0xBB)    //48kHz sample rate for ADC and DAC
#define DAI_CTRL_REG_VALUE      (0xAC)    //DAI Enabled, DAI Channel 1L and 1R enabled, 32bit, I2S
#elif (_AUDIO_CODE_CONF == DAI_24BIT_48KHz)
#define SR_REG_VALUE            (0xBB)    //48kHz sample rate for ADC and DAC
#define DAI_CTRL_REG_VALUE      (0xA8)     //DAI Enabled, DAI Channel 1L and 1R enabled, 24bit, I2S
#elif (_AUDIO_CODE_CONF == DAI_16BIT_48KHz)
#define SR_REG_VALUE            (0xBB)    //48kHz sample rate for ADC and DAC
#define DAI_CTRL_REG_VALUE      (0xA0)     //DAI Enabled, DAI Channel 1L and 1R enabled, 16bit, I2S
#elif (_AUDIO_CODE_CONF == DAI_16BIT_16KHz)
#define SR_REG_VALUE            (0x55)    //16kHz sample rate for ADC and DAC
#define DAI_CTRL_REG_VALUE      (0xA0)     //DAI Enabled, DAI Channel 1L and 1R enabled, 16bit, I2S
#elif (_AUDIO_CODE_CONF == DAI_16BIT_8KHz)
#define SR_REG_VALUE            (0x11)    //8kHz sample rate for ADC and DAC
#define DAI_CTRL_REG_VALUE      (0xA0)     //DAI Enabled, DAI Channel 1L and 1R enabled, 16bit, I2S
#endif

#define DA7218_INSTANCE0_DEVICE_ADDRESS  0x1a  // Depends on DA7218 pin AD state (assume low)

/****************************************************************************************************************
 * Types
 ****************************************************************************************************************/

typedef struct {
  uint8_t Address;
  uint8_t Value;
} TRegisterTable;

/****************************************************************************************************************
 * Constants
 ****************************************************************************************************************/
static const uint8_t _kacGainTable[DA7218_MAX_GAIN+1] =
{
        0x00,   // -83.25dB
        0x29,   // -52.5dB
        0x33,   // -45dB
        0x3D,   // -37.5dB
        0x47,   // -30dB
        0x51,   // -22.5dB
        0x5B,   // -15dB
        0x65,   // -7.5dB
        0x6F,   // 0dB
        0x77    // 6dB
};

static const TRegisterTable _ktCodecInit[] =
{
        {SYSTEM_ACTIVE_REG_adr,         0x01 },    //Enable ACTIVE mode
        {SOFT_RESET_REG_adr,            0x80 },    //RESET
        { 0xFE, 50 },   // DELAY 50

        {SYSTEM_ACTIVE_REG_adr,         0x01 },    //Enable ACTIVE mode
        {PC_COUNT_REG_adr,              0x02},     //PC synced

        //{CIF_CTRL_REG_adr,              0x01 },    //Enable repeat mode register access (register address and data is sent for each write)

        { IO_CTRL_REG_adr,              0x01},  //IO Level 1.5-2.5V
        { LDO_CTRL_REG_adr,             0x80},  //Digital LDO Enabled, 1.05V
        { REFERENCES_REG_adr,           0x08 },    //Enable Master BIAS and VMID (VMID and Bandgap enabled on BIAS_EN)

        {PLL_CTRL_REG_adr,              0x00 },    //PLL Disabled
        {PLL_INTEGER_REG_adr,           0x00 },
        {PLL_FRAC_TOP_REG_adr,          0x00 },
        {PLL_FRAC_BOT_REG_adr,          0x00 },
        {DAI_CTRL_REG_adr,              0x00 },

        //STEPS8-10= PLL DIVIDER FOR MCLK
        //-->0x18,0x9374;
        //MCLK is 32MHz, so pll_integer=0x18 FRAC_TOP=0x93 FRAC_BOT=0x74
        {PLL_FRAC_TOP_REG_adr,          0x12 },    //PLL FRAC_TOP=0x93
        {PLL_FRAC_BOT_REG_adr,          0x6E },    //PLL FRAC_BOT=0x74
        {PLL_INTEGER_REG_adr,           0x12 },    //PLL INTEGER=0x18  - PLL Set-up for 32MHz MCLK
        //STEP12= PLL ENABLE
        //-->write_reg(0x91, 0x43);
        //Normal=PLL enabled, the system clk is fixed multiple of MCLK
        {PLL_CTRL_REG_adr,              0x83 },    //PLL Enabled,

        {SR_REG_adr,                    SR_REG_VALUE },
        {DAI_CTRL_REG_adr,              DAI_CTRL_REG_VALUE },
        //{DAI_CLK_MODE_REG_adr,          0x01 },    //Slave Mode DAI

        //{DAI_OFFSET_LOWER_REG_adr,      0x00 },    //No Offset on DAI (lower)
        //{DAI_OFFSET_UPPER_REG_adr,      0x00 },    //No Offset on DAI (upper)

        { CP_CTRL_REG_adr,              0xF0},  //CP Enabled, Signal Magnitude tracking mode
        { CP_DELAY_REG_adr,             0x35},  //CP Delay 64ms
        { CP_VOL_THRESHOLD1_REG_adr,    0x08},  //CP Volume threshold = 0x36

        { DROUTING_OUTFILT_1L_REG_adr,  0x40},  //OUTFILT_1L_SOURCE = DAI INPUT L ----MONO (Get from R)
        { DROUTING_OUTFILT_1R_REG_adr,  0x40},  //OUTFILT_1R_SOURCE = DAI INPUT R

        //{ DMIX_OUTFILT_1L_INDAI_1L_GAIN_REG_adr, 0x1C},  //DAI INPUT L GAIN = 0dB
        //{ DMIX_OUTFILT_1R_INDAI_1R_GAIN_REG_adr, 0x1C},  //DAI INPUT L GAIN = 0dB

        { MIXOUT_L_GAIN_REG_adr,        0x03},  //MIXOUT_L GAIN = 0dB
        { MIXOUT_R_GAIN_REG_adr,        0x03},  //MIXOUT_R GAIN = 0dB


#if DEMO_PCM_RECORD_PLAYBACK || DEMO_PCM_MIC
        //Tx Path
        { MICBIAS_EN_REG_adr,           0x01},  //enable MIC1 bias
        { MIC_1_CTRL_REG_adr,           0x80},  //enable MIC1
        { MIC_1_GAIN_REG_adr,           0x06},  //set MIC1 gain
        { MIC_1_SELECT_REG_adr,         0x01},  //Enable MIC1_P
        { MIXIN_1_CTRL_REG_adr,         0x88},  //Enable mixer
        { IN_1L_FILTER_CTRL_REG_adr,    0xA0},  //Enable filter
        { DROUTING_OUTDAI_1L_REG_adr,   0x01},  //Select MIC1
        { DROUTING_OUTDAI_1R_REG_adr,   0x01},  //Select MIC1
#endif

        { OUT_1L_FILTER_CTRL_REG_adr,   0x80},  //Enable OUT_1L_FILTER (also enables DACREF)
        { OUT_1R_FILTER_CTRL_REG_adr,   0x80},  //Enable OUT_1R_FILTER

        { MIXOUT_L_CTRL_REG_adr,        0x80},  //MIXOUT_L Enable
        { MIXOUT_R_CTRL_REG_adr,        0x80},  //MIXOUT_R Enable

        { HP_L_CTRL_REG_adr,            0xE8},  //HP_L Enable, Muted, Ramped
        { HP_R_CTRL_REG_adr,            0xE8},  //HP_R Enable, Muted, Ramped

        { HP_L_GAIN_REG_adr,            0x3B},  //0dB Gain
        { HP_R_GAIN_REG_adr,            0x3B},  //0dB Gain

        //{ DGS_LEVELS_REG_adr,           0x00},  //DGS SIGNAL and ANTICLIP level to 0dB
        //{ DGS_TRIGGER_REG_adr,          0x27},  //Enable DGS

        { HP_L_CTRL_REG_adr,            0xA8},  //HP_L Enable, Un-muted, Ramped
        { HP_R_CTRL_REG_adr,            0xA8},  //HP_R Enable, Un-muted, Ramped

        {0xFF, 0x00 }
};
/****************************************************************************************************************
 * Global Data
 ****************************************************************************************************************/

/****************************************************************************************************************
 * Local Data
 ****************************************************************************************************************/
static i2c_config _I2cConfig;

static int _iUsedLevel;

/****************************************************************************************************************
 * Local functions prototypes
 ****************************************************************************************************************/

/****************************************************************************************************************
 * Local functions
 ****************************************************************************************************************/

/****************************************************************************************************************
 * I2C
 ****************************************************************************************************************/
static void CmdInitI2c(void)
{

        /** I2C clock (SCL) settings, refer to datasheet for details. Set to 0 for default values to be used. */
        _I2cConfig.clock_cfg.ss_hcnt = 0;       /**< standard speed I2C clock (SCL) high count */
        _I2cConfig.clock_cfg.ss_lcnt = 0;       /**< standard speed I2C clock (SCL) low count  */
        _I2cConfig.clock_cfg.fs_hcnt = 0;       /**< fast speed I2C clock (SCL) high count     */
        _I2cConfig.clock_cfg.fs_lcnt = 0;       /**< fast speed I2C clock (SCL) low count      */
        _I2cConfig.clock_cfg.hs_hcnt = 0;       /**< high speed I2C clock (SCL) high count     */
        _I2cConfig.clock_cfg.hs_lcnt = 0;       /**< high speed I2C clock (SCL) low count      */

        _I2cConfig.speed = HW_I2C_SPEED_STANDARD;             /**< bus speed */
        _I2cConfig.mode = HW_I2C_MODE_MASTER;                 /**< mode of operation */
        _I2cConfig.addr_mode = HW_I2C_ADDRESSING_7B;          /**< addressing mode */
        _I2cConfig.address = DA7218_INSTANCE0_DEVICE_ADDRESS; /**< target slave address in master mode or controller address in slave mode */
        _I2cConfig.event_cb = 0;             /**< slave event callback (only valid in slave mode) */

        /* Configure the I2C GPIOs. */
        hw_gpio_pad_latch_enable(I2C_SCL_PORT, I2C_SCL_PIN);
        hw_gpio_pad_latch_enable(I2C_SDA_PORT, I2C_SDA_PIN);

        hw_gpio_set_pin_function(I2C_SCL_PORT, I2C_SCL_PIN, HW_GPIO_MODE_INPUT,
                                 HW_GPIO_FUNC_I2C_SCL);
        hw_gpio_set_pin_function(I2C_SDA_PORT, I2C_SDA_PIN, HW_GPIO_MODE_INPUT,
                                 HW_GPIO_FUNC_I2C_SDA);
        hw_gpio_configure_pin_power(I2C_SCL_PORT, I2C_SCL_PIN, _GPIO_LVL);
        hw_gpio_configure_pin_power(I2C_SDA_PORT, I2C_SDA_PIN, _GPIO_LVL);

        /* Initialize the I2C interface. */
        hw_i2c_init(HW_I2C1, &_I2cConfig);

        hw_i2c_enable( HW_I2C1);
}

static void CmdWriteI2c(uint8_t addr, const uint8_t *data, uint16_t len)
{
        HW_I2C_ABORT_SOURCE abrt_code;

        hw_i2c_write_byte(HW_I2C1, addr);
        hw_i2c_write_buffer_sync(HW_I2C1, data, len, &abrt_code, HW_I2C_F_WAIT_FOR_STOP);
        switch (abrt_code)
        {
        case HW_I2C_ABORT_NONE:
                break;
        case HW_I2C_ABORT_7B_ADDR_NO_ACK:
        case HW_I2C_ABORT_10B_ADDR1_NO_ACK:
        case HW_I2C_ABORT_10B_ADDR2_NO_ACK:
                break;
        case HW_I2C_ABORT_TX_DATA_NO_ACK:
                break;
        case HW_I2C_ABORT_GENERAL_CALL_NO_ACK:
        case HW_I2C_ABORT_GENERAL_CALL_READ:
        case HW_I2C_ABORT_START_BYTE_ACK:
        case HW_I2C_ABORT_10B_READ_NO_RESTART:
        case HW_I2C_ABORT_MASTER_DISABLED:
        case HW_I2C_ABORT_ARBITRATION_LOST:
        case HW_I2C_ABORT_SLAVE_FLUSH_TX_FIFO:
        case HW_I2C_ABORT_SLAVE_ARBITRATION_LOST:
        case HW_I2C_ABORT_SLAVE_IN_TX:
        case HW_I2C_ABORT_SW_ERROR:
                break;

        }
}

/****************************************************************************************************************
 * CODEC
 ****************************************************************************************************************/
void DA7218_Init(void)
{
        int i=0;
        CmdInitI2c();
        while (_ktCodecInit[i].Address != 0xFF)
        {
                if (_ktCodecInit[i].Address == 0xFE)
                {       // Delay function
                        OS_DELAY(OS_MS_2_TICKS(_ktCodecInit[i].Value));
                }
                else
                {
                        CmdWriteI2c(_ktCodecInit[i].Address, &(_ktCodecInit[i].Value), 1);
                }
                i++;
        }

        DA7218_SetGain(DA7218_MAX_GAIN);
}

void DA7218_Enable(void)
{
        uint8_t ucData;

        ucData = 0x01;
        CmdWriteI2c(SYSTEM_ACTIVE_REG_adr, &ucData, 1);

        ucData = PLL_FRAC_BOT_REG_adr; //PLL FRAC_TOP=0x93
        CmdWriteI2c(PLL_FRAC_TOP_REG_adr, &ucData, 1);
        ucData = 0x74; //PLL FRAC_BOT=0x74
        CmdWriteI2c(PLL_FRAC_BOT_REG_adr, &ucData, 1);
        ucData = 0x18; //PLL INTEGER=0x18  - PLL Set-up for 32MHz MCLK
        CmdWriteI2c(PLL_INTEGER_REG_adr, &ucData, 1);

}

void DA7218_Disable(void)
{
        uint8_t ucData;

        ucData = 0x00;
        CmdWriteI2c(SYSTEM_ACTIVE_REG_adr, &ucData, 1);
}

/****************************************************************************************************************
 * API
 ****************************************************************************************************************/
void DA7218_SetGain(int iLevel)
{
        if (iLevel > DA7218_MAX_GAIN)
        {
                _iUsedLevel =  DA7218_MAX_GAIN;
        }
        else
        {
                _iUsedLevel = iLevel;
        }
        CmdWriteI2c(0xF8, &(_kacGainTable[_iUsedLevel]), 1); //OUT_1L_DIGITAL_GAIN GAIN = 0dB
        CmdWriteI2c(0xF9, &(_kacGainTable[_iUsedLevel]), 1); //OUT_1R_DIGITAL_GAIN GAIN = 0dB

}

int DA7218_GetGain(void)
{
        return(_iUsedLevel);
}
