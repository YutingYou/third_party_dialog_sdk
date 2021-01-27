/**
 ****************************************************************************************
 *
 * @file peripheral_setup.h
 *
 * @brief Peripherals setup header file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PERIPHERAL_SETUP_H_
#define PERIPHERAL_SETUP_H_


/**
 * Power Enable (PE) pins
 */
#define ACC_POWER_ENABLE_PORT           ( HW_GPIO_PORT_1 )
#define ACC_POWER_ENABLE_PIN            ( HW_GPIO_PIN_20 )

#define BH1750_POWER_ENABLE_PORT        ( HW_GPIO_PORT_0 )
#define BH1750_POWER_ENABLE_PIN         ( HW_GPIO_PIN_12 )

/**
 * SPI 1 configuration
 */
#define SPI1_DO_GPIO_PORT               ( HW_GPIO_PORT_0 )
#define SPI1_DO_GPIO_PIN                ( HW_GPIO_PIN_26 )

#define SPI1_DI_GPIO_PORT               ( HW_GPIO_PORT_0 )
#define SPI1_DI_GPIO_PIN                ( HW_GPIO_PIN_24 )

#define SPI1_CLK_GPIO_PORT              ( HW_GPIO_PORT_0 )
#define SPI1_CLK_GPIO_PIN               ( HW_GPIO_PIN_21 )

/**
 * I2C 1 configuration
 */
#define I2C1_SCL_PORT                   ( HW_GPIO_PORT_0 )
#define I2C1_SCL_PIN                    ( HW_GPIO_PIN_30 )

#define I2C1_SDA_PORT                   ( HW_GPIO_PORT_0 )
#define I2C1_SDA_PIN                    ( HW_GPIO_PIN_31 )

/**
 * EEPROM (24FC256) configuration
 */
#define EEPROM_24FC256_ADDRESS          ( 0x50 )

#define EEPROM_24FC256_I2C_SCL_PORT     ( I2C1_SCL_PORT )
#define EEPROM_24FC256_I2C_SCL_PIN      ( I2C1_SCL_PIN  )

#define EEPROM_24FC256_I2C_SDA_PORT     ( I2C1_SDA_PORT )
#define EEPROM_24FC256_I2C_SDA_PIN      ( I2C1_SDA_PIN  )

/**
 * Analog Devices (ADXL362) Sensor configuration
 */
#define ADXL362_SPI_CS_PORT             ( HW_GPIO_PORT_0 )
#define ADXL362_SPI_CS_PIN              ( HW_GPIO_PIN_18 )

#define ADXL362_INT_1_PORT              ( HW_GPIO_PORT_0 )
#define ADXL362_INT_1_PIN               ( HW_GPIO_PIN_17 )

#define ADXL362_INT_2_PORT              ( HW_GPIO_PORT_0 )
#define ADXL362_INT_2_PIN               ( HW_GPIO_PIN_25 )

/**
 * ROHM BH-1750 Ambient Light Sensor configuration
 */
#define BH1750_I2C_ADDRESS              ( 0x23 )

#define BH1750_I2C_SCL_PORT             ( I2C1_SCL_PORT )
#define BH1750_I2C_SCL_PIN              ( I2C1_SCL_PIN  )

#define BH1750_I2C_SDA_PORT             ( I2C1_SDA_PORT )
#define BH1750_I2C_SDA_PIN              ( I2C1_SDA_PIN  )


#endif /* PERIPHERAL_SETUP_H_ */
