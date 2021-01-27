/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_LCD_CONTROLLER LCD Controller Driver
 * \{
 * \brief LCD Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_lcdc.h
 *
 * @brief Definition of API for the LCD Controller Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_LCDC_H
#define HW_LCDC_H


#if dg_configUSE_HW_LCDC

#include <sdk_defs.h>
/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief MIPI Display Bus Interface command type
 */
typedef enum {
        HW_LCDC_MIPI_CMD,        //!< New command to the LCD
        HW_LCDC_MIPI_STORE_BADDR,//!< Store value to the line register
        HW_LCDC_MIPI_DATA,       //!< Additional data to a command
} HW_LCDC_MIPI;

/**
 * \brief LCD controller MIPI configuration type
 */
typedef enum {
        HW_LCDC_MIPI_CFG_TE_DIS       = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_TE_DIS),       //!< Disable sampling of tearing effect signal
        HW_LCDC_MIPI_CFG_FRC_CSX_0    = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_CSX_FORCE),    //!< Force Chip Select output (to 0)
        HW_LCDC_MIPI_CFG_SPI_CSX_V    = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_CSX_FORCE_VAL),//!< Invert Chip Select control
        HW_LCDC_MIPI_CFG_FRC_CSX_1    = HW_LCDC_MIPI_CFG_FRC_CSX_0 | HW_LCDC_MIPI_CFG_SPI_CSX_V,  //!< Force Chip Select output (to 1)
        HW_LCDC_MIPI_CFG_SPI_PAD      = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_PAD),      //!< Enable SPI data padding
        HW_LCDC_MIPI_CFG_RESET        = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_RESX),         //!< Disable reset signal of MIPI
        HW_LCDC_MIPI_CFG_DMA          = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_DMA_EN),       //!< Enable DMA transfer
        HW_LCDC_MIPI_CFG_SPI3         = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI3_EN),      //!< Enable SPI3 interface
        HW_LCDC_MIPI_CFG_SPI4         = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI4_EN),      //!< Enable SPI4 interface
        HW_LCDC_MIPI_CFG_SPI_CPHA     = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_CPHA),     //!< Phase of SPI data
        HW_LCDC_MIPI_CFG_SPI_CPOL     = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_CPOL),     //!< Polarity of SPI data
        HW_LCDC_MIPI_CFG_SPI_JDI      = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_JDI),      //!< Enable line addressing between lines
        HW_LCDC_MIPI_CFG_SPI_HOLD     = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_HOLD),     //!< Enable the hold of commands to bind commands and data
        HW_LCDC_MIPI_CFG_INV_ADDR     = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_INV_ADDR), //!< Enable horizontal line address inversion (MSB to LSB)
        HW_LCDC_MIPI_CFG_INV_DATA     = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_INV_DATA),     //!< Enable bit inversion of data (1s to 0s and vice versa)
        HW_LCDC_MIPI_CFG_JDI_INV_PIX  = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_JDI_INV_PIX),  //!< Enable MSB to LSB inversion for JDI parallel interface
        HW_LCDC_MIPI_CFG_JDI_SOFT_RST = REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_JDI_SOFT_RST), //!< Enable soft reset of JDI timing generation
} HW_LCDC_MIPI_CFG;

/**
 * \brief MIPI Display Command Set
 *
 * \warning Read / Get commands should not be issued since LCD controller does not support reading
 * data from an LCD.
 */
typedef enum {
        HW_LCDC_MIPI_DCS_NOP                   = 0x00,//!< No Operation
        HW_LCDC_MIPI_DCS_SOFT_RESET            = 0x01,//!< Software Reset
        HW_LCDC_MIPI_DCS_GET_RED_CHANNEL       = 0x06,//!< Get the red component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_GREEN_CHANNEL     = 0x07,//!< Get the green component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_BLUE_CHANNEL      = 0x08,//!< Get the blue component of the pixel at (0, 0).
        HW_LCDC_MIPI_DCS_GET_POWER_MODE        = 0x0A,//!< Get the current power mode.
        HW_LCDC_MIPI_DCS_GET_ADDRESS_MODE      = 0x0B,/*!< Get the data order for transfers from the Host
                                                           to the display module and from the frame memory
                                                           to the display device. */
        HW_LCDC_MIPI_DCS_GET_PIXEL_FORMAT      = 0x0C,//!< Get the current pixel format.
        HW_LCDC_MIPI_DCS_GET_DISPLAY_MODE      = 0x0D,//!< Get the current display mode from the peripheral.
        HW_LCDC_MIPI_DCS_GET_SIGNAL_MODE       = 0x0E,//!< Get display module signaling mode.
        HW_LCDC_MIPI_DCS_GET_DIAGNOSTIC_RESULT = 0x0F,//!< Get Peripheral Self-Diagnostic Result
        HW_LCDC_MIPI_DCS_ENTER_SLEEP_MODE      = 0x10,//!< Power for the display panel is off.
        HW_LCDC_MIPI_DCS_EXIT_SLEEP_MODE       = 0x11,//!< Power for the display panel is on.
        HW_LCDC_MIPI_DCS_ENTER_PARTIAL_MODE    = 0x12,//!< Part of the display area is used for image display.
        HW_LCDC_MIPI_DCS_ENTER_NORMAL_MODE     = 0x13,//!< The whole display area is used for image display.
        HW_LCDC_MIPI_DCS_EXIT_INVERT_MODE      = 0x20,//!< Displayed image colors are not inverted.
        HW_LCDC_MIPI_DCS_ENTER_INVERT_MODE     = 0x21,//!< Displayed image colors are inverted.
        HW_LCDC_MIPI_DCS_SET_GAMMA_CURVE       = 0x26,//!< Selects the gamma curve used by the display device.
        HW_LCDC_MIPI_DCS_SET_DISPLAY_OFF       = 0x28,//!< Blanks the display device.
        HW_LCDC_MIPI_DCS_SET_DISPLAY_ON        = 0x29,//!< Show the image on the display device.
        HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS    = 0x2A,//!< Set the column extent.
        HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS      = 0x2B,//!< Set the page extent.
        HW_LCDC_MIPI_DCS_WRITE_MEMORY_START    = 0x2C,/*!< Transfer image data from the Host Processor to the
                                                           peripheral starting at the location provided by
                                                           HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS and
                                                           HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS. */
        HW_LCDC_MIPI_DCS_WRITE_LUT             = 0x2D,//!< Fills the peripheral look-up table with the provided data.
        HW_LCDC_MIPI_DCS_READ_MEMORY_START     = 0x2E,/*!< Transfer image data from the peripheral to the Host
                                                           Processor interface starting at the location provided
                                                           by HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS and
                                                           HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS. */
        HW_LCDC_MIPI_DCS_SET_PARTIAL_ROWS      = 0x30,/*!< Defines the number of rows in the partial display area
                                                           on the display device. */
        HW_LCDC_MIPI_DCS_SET_PARTIAL_COLUMNS   = 0x31,/*!< Defines the number of columns in the partial display
                                                           area on the display device. */
        HW_LCDC_MIPI_DCS_SET_SCROLL_AREA       = 0x33,/*!< Defines the vertical scrolling and fixed area on
                                                           display device. */
        HW_LCDC_MIPI_DCS_SET_TEAR_OFF          = 0x34,/*!< Synchronization information is not sent from the display
                                                           module to the host processor. */
        HW_LCDC_MIPI_DCS_SET_TEAR_ON           = 0x35,/*!< Synchronization information is sent from the display
                                                           module to the host processor at the start of VFP. */
        HW_LCDC_MIPI_DCS_SET_ADDRESS_MODE      = 0x36,/*!< Set the data order for transfers from the Host to the
                                                           display module and from the frame memory to the display
                                                           device. */
        HW_LCDC_MIPI_DCS_SET_SCROLL_START      = 0x37,//!< Defines the vertical scrolling starting point.
        HW_LCDC_MIPI_DCS_EXIT_IDLE_MODE        = 0x38,//!< Full color depth is used on the display panel.
        HW_LCDC_MIPI_DCS_ENTER_IDLE_MODE       = 0x39,//!< Reduced color depth is used on the display panel.
        HW_LCDC_MIPI_DCS_SET_PIXEL_FORMAT      = 0x3A,//!< Defines how many bits per pixel are used in the interface.
        HW_LCDC_MIPI_DCS_WRITE_MEMORY_CONTINUE = 0x3C,/*!< Transfer image information from the Host Processor interface
                                                           to the peripheral from the last written location. */
        HW_LCDC_MIPI_DCS_SET_3D_CONTROL        = 0x3D,//!< 3D is used on the display panel
        HW_LCDC_MIPI_DCS_READ_MEMORY_CONTINUE  = 0x3E,/*!< Read image data from the peripheral continuing after the
                                                           last HW_LCDC_MIPI_DCS_READ_MEMORY_CONTINUE or
                                                           HW_LCDC_MIPI_DCS_READ_MEMORY_START. */
        HW_LCDC_MIPI_DCS_GET_3D_CONTROL        = 0x3F,//!< Get display module 3D mode
        HW_LCDC_MIPI_DCS_SET_VSYNC_TIMING      = 0x40,//!< Set VSYNC timing
        HW_LCDC_MIPI_DCS_SET_TEAR_SCANLINE     = 0x44,/*!< Synchronization information is sent from the display module
                                                           to the host processor when the display device refresh
                                                           reaches the provided scan line. */
        HW_LCDC_MIPI_DCS_GET_SCANLINE          = 0x45,//!< Get the current scan line.
        HW_LCDC_MIPI_DCS_READ_DDB_START        = 0xA1,//!< Read the DDB from the provided location.
        HW_LCDC_MIPI_DCS_READ_DDB_CONTINUE     = 0xA8,//!< Continue reading the DDB from the last read location.
} HW_LCDC_MIPI_DCS;

/**
 * \brief Output color mode/format of the LCD controller
 */
typedef enum {
        /** 0 0 R G B R' G' B' */
        HW_LCDC_OCM_8RGB111_1 = 0x06,
        /** R G B 0 R' G' B' 0 */
        HW_LCDC_OCM_8RGB111_2 = 0x07,
        /** R G B  R' G' B' ... */
        HW_LCDC_OCM_RGB111    = 0x08,
        /** D D' D'' ... */
        HW_LCDC_OCM_L1        = 0x09,
        /** R[2-0]G[2-0]B[1-0] */
        HW_LCDC_OCM_8RGB332   = 0x10,
        /** R[3-0]G[3-0] - B[3-0]R'[3-0] - G'[3-0]B'[3-0] */
        HW_LCDC_OCM_8RGB444   = 0x11,
        /** R[4-0]G[5-3] - G[2-0]B[4-0] */
        HW_LCDC_OCM_8RGB565   = 0x12,
        /** R[5-0]00 - G[5-0]00 - B[5-0]00 */
        HW_LCDC_OCM_8RGB666   = 0x13,
        /** R[7-0] - G[7-0] - B[7-0] */
        HW_LCDC_OCM_8RGB888   = 0x14,

        /**
         * JDI parallel only
         *
         * R1 line: R[1]  R''[1]  ... R[0]  R''[0]  ...
         * R2 line: R'[1] R'''[1] ... R'[0] R'''[0] ...
         * G1 line: G[1]  G''[1]  ... G[0]  G''[0]  ...
         * G2 line: G'[1] G'''[1] ... G'[0] G'''[0] ...
         * B1 line: B[1]  B''[1]  ... B[0]  B''[0]  ...
         * B2 line: B'[1] B'''[1] ... B'[0] B'''[0] ...
         */
        HW_LCDC_OCM_RGB222    = 0x0a,

} HW_LCDC_OUTPUT_COLOR_MODE;

/**
 * \brief LCD controller mode configuration type
 */
typedef enum {
        HW_LCDC_MODE_DISABLE    = 0,                                               //!< Disable mode
        HW_LCDC_MODE_ENABLE     = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_MODE_EN),      //!< Enable continuous mode
        HW_LCDC_MODE_NEG_V      = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_VSYNC_POL),    //!< Negative VSYNC polarity
        HW_LCDC_MODE_NEG_H      = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_HSYNC_POL),    //!< Negative HSYNC polarity
        HW_LCDC_MODE_NEG_DE     = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_DE_POL),       //!< Negative DE polarity
        HW_LCDC_MODE_SINGLEV    = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_VSYNC_SCPL),   //!< VSYNC for a single cycle per line
        HW_LCDC_MODE_INVPIXCLK  = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_PIXCLKOUT_POL),//!< Pixel clock out polarity
        HW_LCDC_MODE_BLANK      = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_FORCE_BLANK),  //!< Force output to blank
        HW_LCDC_MODE_ONE_FRAME  = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_SFRAME_UPD),   //!< Single frame update
        HW_LCDC_MODE_FORMAT_CLK = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_PIXCLKOUT_SEL),//!< Select pixel clock source
        HW_LCDC_MODE_MIPI_OFF   = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_MIPI_OFF),     //!< MIPI off
        HW_LCDC_MODE_OUTP_OFF   = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_FORM_OFF),     //!< Formating off
        HW_LCDC_MODE_SCANDOUBLE = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_DSCAN),        //!< Enable double horizontal scan
        HW_LCDC_MODE_TESTMODE   = REG_MSK(LCDC, LCDC_MODE_REG, LCDC_TMODE),        //!< Enable test mode

        HW_LCDC_MODE_P_RGB3     = 0 << REG_POS(LCDC, LCDC_MODE_REG, LCDC_OUT_MODE),//!< Parallel RGB
        HW_LCDC_MODE_JDIMIP     = 8 << REG_POS(LCDC, LCDC_MODE_REG, LCDC_OUT_MODE),//!< JDI MIP
} HW_LCDC_MODE;

/**
 * \brief Layer color format/mode
 * \note Alpha values are ignored
 */
typedef enum {
        HW_LCDC_LCM_RGBA5551 = 0x01,//!< R[4-0]G[4-0]B[4-0]A0
        HW_LCDC_LCM_ABGR8888 = 0x02,//!< R[7-0]G[7-0]B[7-0]A[7-0]
        HW_LCDC_LCM_RGB332   = 0x04,//!< R[2-0]G[2-0]B[1-0]
        HW_LCDC_LCM_RGB565   = 0x05,//!< R[4-0]G[5-0]B[4-0]
        HW_LCDC_LCM_BGRA8888 = 0x06,//!< A[7-0]R[7-0]G[7-0]B[7-0]
        HW_LCDC_LCM_L8       = 0x07,//!< L[7-0] - Grayscale
        HW_LCDC_LCM_L1       = 0x08,//!< L - Black and white, 1 = black, 0 = white
        HW_LCDC_LCM_L4       = 0x09,//!< L[3-0] - Grayscale
        HW_LCDC_LCM_RGBA8888 = 0x0d,//!< A[7-0]B[7-0]G[7-0]R[7-0]
        HW_LCDC_LCM_ARGB8888 = 0x0e,//!< B[7-0]G[7-0]R[7-0]A[7-0]
} HW_LCDC_LAYER_COLOR_MODE;

/**
 * \brief Status of operation
 */
typedef enum {
        HW_LCDC_OK, //!< Operation completed successfully
        HW_LCDC_ERR,//!< Operation encountered an error
} HW_LCDC_STATUS;

/**
 * \brief Physical connection type enumeration
 */
typedef enum {
        HW_LCDC_PHY_NONE,            //!< No physical connection
        HW_LCDC_PHY_MIPI_SPI3,       //!< SPI connection with 3 wires (DCX as an extra bit)
        HW_LCDC_PHY_MIPI_SPI4,       //!< SPI connection with 4 wires (DCX as an extra line)
        HW_LCDC_PHY_JDI_SPI,         //!< JDI serial connection
        HW_LCDC_PHY_SHARP_SPI,       //!< Sharp serial connection
        HW_LCDC_PHY_JDI_PARALLEL,    //!< JDI parallel connection
        HW_LCDC_PHY_CLASSIC_PARALLEL,//!< Classic parallel connection
        HW_LCDC_PHY_CUSTOM,          //!< Custom connection parameters, \sa cfg_extra_flags of lcdc_config
} HW_LCDC_PHY;

/**
 * \brief Clock source of the LCD controller module
 */
typedef enum {
        HW_LCDC_CLK_SRC_DIVN,//!< Clock source is DIVN clock (steady at 32MHz)
        HW_LCDC_CLK_SRC_SYS, //!< Clock source is system clock
} HW_LCDC_CLK_SRC;

/**
 * \def HW_LCDC_CLK_DIV_MSK
 *
 * \brief Macro to define the interface (secondary) clock divider mask
 */
#define HW_LCDC_CLK_DIV_MSK             (0x1F)

/**
 * \def HW_LCDC_CLK_PLL_BIT
 *
 * \brief Macro to define the bit that indicates if system PLL clock is required to achieve the
 * required frequency
 */
#define HW_LCDC_CLK_PLL_BIT             (1 << 31)

/**
 * \def HW_LCDC_DIV
 *
 * \brief Macro to calculate the LCDC divider to produce a frequency using the provided source clock
 *
 * Due to physical restrictions only the following ranges of frequencies are valid:
 *
 *  Output (interface) clock       |  Source DIVN       |  Source PLL96M
 * ------------------------------- | ------------------ | ------------------
 *  Classic Parallel               |  32 MHz - 1 MHz    |  96 MHz - 3 MHz
 *  JDI Parallel (divided by hck)  |  16 MHz - 0.5 MHz  |  48 MHz - 1.5 MHz
 *  Serial                         |  16 MHz - 0.5 MHz  |  48 MHz - 1.5 MHz
 *
 *  In case of JDI parallel and serial interfaces the divider's value is automatically adapted
 *  (divided by 2) to produce the correct frequency
 *
 * \param[in] hz                Frequency in Hz
 *
 * \note If the requested frequency is not supported (i.e. there is no LCDC divider to produce the
 * exact frequency) the next available frequency will be selected.
 *
 * \warning The application or the adapter (if used) is responsible to turn on the PLL if needed and
 * maintain it as long as needed.
 */
#define HW_LCDC_DIV(hz)                                                                            \
        ((!((dg_configDIVN_FREQ) % (hz))) ? ((dg_configDIVN_FREQ / (hz))) :                        \
                                            (((dg_configPLL96M_FREQ / (hz))) | HW_LCDC_CLK_PLL_BIT))

/**
 * \brief LCD interface frequency
 *
 * It controls the interface clock divisor (\sa hw_lcdc_set_iface_clk()) and the requirement of a
 * PLL system clock by setting the flag bit LCDC_CLK_PLL_BIT.
 *
 * \note Custom values can also be entered to produce frequencies between the predefined ones. To
 * set such a frequency, the format (divisor | flag) has to be followed.
 *
 * \warning The application or the adapter (if used) is responsible to turn on the PLL if needed and
 * maintain it as long as needed.
 */
typedef enum {
        LCDC_FREQ_48MHz  = HW_LCDC_DIV(48000000),//!< LCD interface frequency at 48MHz (PLL on)
        LCDC_FREQ_24MHz  = HW_LCDC_DIV(24000000),//!< LCD interface frequency at 24MHz (PLL on)
        LCDC_FREQ_16MHz  = HW_LCDC_DIV(16000000),//!< LCD interface frequency at 16MHz
        LCDC_FREQ_12MHz  = HW_LCDC_DIV(12000000),//!< LCD interface frequency at 12MHz (PLL on)
        LCDC_FREQ_9_6MHz = HW_LCDC_DIV( 9600000),//!< LCD interface frequency at 9,6MHz (PLL on)
        LCDC_FREQ_8MHz   = HW_LCDC_DIV( 8000000),//!< LCD interface frequency at 8MHz
        LCDC_FREQ_6MHz   = HW_LCDC_DIV( 6000000),//!< LCD interface frequency at 6MHz (PLL on)
        LCDC_FREQ_4_8MHz = HW_LCDC_DIV( 4800000),//!< LCD interface frequency at 4,8MHz (PLL on)
        LCDC_FREQ_4MHz   = HW_LCDC_DIV( 4000000),//!< LCD interface frequency at 4MHz
        LCDC_FREQ_3_2MHz = HW_LCDC_DIV( 3200000),//!< LCD interface frequency at 3,2MHz
        LCDC_FREQ_3MHz   = HW_LCDC_DIV( 3000000),//!< LCD interface frequency at 3MHz (PLL on)
        LCDC_FREQ_2MHz   = HW_LCDC_DIV( 2000000),//!< LCD interface frequency at 2MHz
        LCDC_FREQ_1_6MHz = HW_LCDC_DIV( 1600000),//!< LCD interface frequency at 1,6MHz
        LCDC_FREQ_1MHz   = HW_LCDC_DIV( 1000000),//!< LCD interface frequency at 1MHz
        LCDC_FREQ_0_8MHz = HW_LCDC_DIV(  800000),//!< LCD interface frequency at 800Hz
        LCDC_FREQ_0_5MHz = HW_LCDC_DIV(  500000),//!< LCD interface frequency at 500Hz
} HW_LCDC_FREQ;

/**
 * \brief Parallel connection type
 */
typedef enum {
        HW_LCDC_PAR_IF_JDI     = 0,//!< JDI parallel connection
        HW_LCDC_PAR_IF_CLASSIC = 1,//!< Classic parallel connection
} HW_LCDC_PAR_IF;

/**
 * \brief Tearing effect detection level
 */
typedef enum {
        HW_LCDC_TE_LOW  = 0,//!< Detected low TE signal
        HW_LCDC_TE_HIGH = 1,//!< Detected high TE signal
} HW_LCDC_TE;

/**
 * \brief Layer FIFO input threshold
 */
typedef enum {
        HW_LCDC_FIFO_THR_HALF         = 0x00,//!< DMA is triggered when FIFO is below half (Default)
        HW_LCDC_FIFO_THR_2_BURST_SIZE = 0x01,//!< DMA is triggered when FIFO can fit at least 2 bursts
        HW_LCDC_FIFO_THR_4_BURST_SIZE = 0x02,//!< DMA is triggered when FIFO can fit at least 4 bursts
        HW_LCDC_FIFO_THR_8_BURST_SIZE = 0x03,//!< DMA is triggered when FIFO can fit at least 8 bursts
} HW_LCDC_FIFO_THR;

/**
 * \brief Layer burst length in beats
 */
typedef enum {
        HW_LCDC_BURST_LEN_DEFAULT  = 0x0,//!< Default burst length (16 beats)
        HW_LCDC_BURST_LEN_2_BEATS  = 0x1,//!< 2 beats burst length
        HW_LCDC_BURST_LEN_4_BEATS  = 0x2,//!< 4 beats burst length
        HW_LCDC_BURST_LEN_8_BEATS  = 0x3,//!< 8 beats burst length
        HW_LCDC_BURST_LEN_16_BEATS = 0x4,//!< 16 beats burst length
} HW_LCDC_BURST_LEN;

/**
 * \brief DMA pre-fetch level
 *
 * LCD controller waits until at least the specified amount of data have been received in FIFO
 * before the transmission of the frame starts.
 */
typedef enum {
        HW_LCDC_FIFO_PREFETCH_LVL_DISABLED  = 0x00,//!< No wait, controller starts immediately sending data
        HW_LCDC_FIFO_PREFETCH_LVL_44_BYTES  = 0x01,//!< Wait until at least 44 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_84_BYTES  = 0x02,//!< Wait until at least 84 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_116_BYTES = 0x03,//!< Wait until at least 116 bytes have been received
        HW_LCDC_FIFO_PREFETCH_LVL_108_BYTES = 0x04,//!< Wait until at least 108 bytes have been received
} HW_LCDC_FIFO_PREFETCH_LVL;

/**
 * \brief Chip select mode of operation
 *
 * In auto modes, chip select is handled automatically by the LCD controller
 */
typedef enum {
        HW_LCDC_SCS_AUTO,    //!< Chip select is low when enabled
        HW_LCDC_SCS_AUTO_INV,//!< Chip select is high when enabled
        HW_LCDC_SCS_HIGH,    //!< Chip select is forced to high
        HW_LCDC_SCS_LOW,     //!< Chip select is forced to low
} HW_LCDC_SCS_CFG;

/**
 * \brief JDI/Sharp serial commands enumeration
 *
 * \note Each LCD may adopt only a part of the functionality and the respective commands. Please,
 * reference to the specific LCD documentation for the supported commands.
 */
typedef enum {
        HW_LCDC_JDIS_CMD_NOP           = 0x00,//!< No operation
        HW_LCDC_JDIS_CMD_BLINKOFF      = 0x00,//!< Stop LCD blinking
        HW_LCDC_JDIS_CMD_BLINKBLACK    = 0x10,//!< Blink display with black color
        HW_LCDC_JDIS_CMD_BLINKWHITE    = 0x18,//!< Blink display with white color
        HW_LCDC_JDIS_CMD_BLINKINVERT   = 0x14,//!< Blink display with inverted colors
        HW_LCDC_JDIS_CMD_CLEAR         = 0x20,//!< Clear display memory
        HW_LCDC_JDIS_CMD_UPDATE_NATIVE = 0x80,//!< Update display in native color mode
        HW_LCDC_JDIS_CMD_UPDATE_1BIT   = 0x88,//!< Update display in 1 bit color mode (b&w)
        HW_LCDC_JDIS_CMD_UPDATE_4BIT   = 0x90,//!< Update display in 4 bit color mode
} HW_LCDC_JDIS_CMD;

/**
 * \brief LCD external clock frequency
 */
typedef enum {
        HW_LCDC_EXT_CLK_1HZ,   //!< Clock frequency at 1Hz
        HW_LCDC_EXT_CLK_62_5HZ,//!< Clock frequency at 62.5Hz
        HW_LCDC_EXT_CLK_125HZ, //!< Clock frequency at 125Hz
        HW_LCDC_EXT_CLK_OFF,   //!< Clock is off (Default)
} HW_LCDC_EXT_CLK;

/**
 * \brief Structure that holds a frame's dimensions
 */
typedef struct {
        uint16_t startx, starty;//!< Start row/column of the frame
        uint16_t endx, endy;    //!< End row/column of the frame
} hw_lcdc_frame_t;

/**
 * \brief Structure that holds the JDI parallel timings information
 */
typedef struct {
        uint32_t starty, endy;//!< Start/end line of the display to be updated
        uint16_t resx, resy;  //!< Horizontal/vertical resolution of the display
        uint16_t fxb, fyb;    //!< Front horizontal/vertical blanking
        uint16_t bxb, byb;    //!< Back horizontal/vertical blanking
        uint32_t hck_width;   //!< HCK half period in interface clock cycles (\sa hw_lcdc_set_iface_clk())
        uint32_t xrst_width;  //!< XRST width in half horizontal line clock cycles
} hw_lcdc_jdi_parallel_timings_t;

/**
 * \brief Structure that holds the display timing parameters in pixels
 */
typedef struct {
        uint16_t resx, resy;//!< Horizontal/vertical resolution of the screen
        uint16_t fpx, fpy;  //!< Horizontal/vertical front porch
        uint16_t bpx, bpy;  //!< Horizontal/vertical back porch
        uint16_t blx, bly;  //!< Horizontal/vertical blanking
} hw_lcdc_display_t;

/**
 * \brief Structure that holds the layer parameters (input of the LCD controller)
 */
typedef struct {
        uint32_t baseaddr;                         //!< Base address where the input frame resides in memory
        int32_t  stride;                           //!< Line to line distance in bytes of frame in memory
        int16_t  startx, starty;                   /*!< Horizontal/vertical coordinates of the top-left corner
                                                        of the layer. (0,0) is the top-left corner of the screen */
        uint16_t resx, resy;                       //!< Resolution of layer in pixels
        HW_LCDC_LAYER_COLOR_MODE format;           //!< Color mode format of the layer, \sa HW_LCDC_LAYER_COLOR_MODE
        HW_LCDC_FIFO_PREFETCH_LVL dma_prefetch_lvl;//!< DMA pre-fetch level as defined in
} hw_lcdc_layer_t;

/**
 * \brief LCD Controller configuration
 */
typedef struct
{
        HW_LCDC_PHY phy_type;            //!< Physical connection type as defined in \sa HW_LCDC_PHY
        HW_LCDC_OUTPUT_COLOR_MODE format;//!< Output color mode/format of the LCD controller as in \sa HW_LCDC_OUTPUT_COLOR_MODE*/
        HW_LCDC_MIPI_CFG cfg_extra_flags;//!< Extra configuration flags to be applied in register LCDC_DBIB_CFG_REG, \sa hw_lcdc_mipi_cfg_out()
        HW_LCDC_MODE mode;               //!< Mode configuration flags (\sa HW_LCDC_MODE)
        HW_LCDC_FREQ iface_freq;         //!< Frequency of the interface as provided by \sa HW_LCDC_FREQ or \sa HW_LCDC_DIV
} hw_lcdc_config_t;

/**
 * \brief Callback function to be called when an interrupt event occurs
 *
 * \param[in] user_data         User defined data to be passed, \sa hw_lcdc_set_callback()
 */
typedef void (*hw_lcdc_callback)(void *user_data);

/*
 * API FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 * \name                Register functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set the mode register with corresponding value(s)
 *
 * \param[in] mode              Flag(s) to be set
 *
 * \sa HW_LCDC_MODE
 */
__STATIC_INLINE void hw_lcdc_set_mode(HW_LCDC_MODE mode)
{
        LCDC->LCDC_MODE_REG = mode;
}

/**
 * \brief Set clock divider of the LCDC which controls the internal pixel pipeline clock
 *
 * \note Source clock of this divider is the format pipeline clock (\sa hw_lcdc_set_pixel_clk()). The
 * period of the generated clock is defined as : LCDC_CLK_DIV x period_of_format_clk. A zero value
 * gives division by one.
 *
 * \note Preferably set this divider to 1 (maximum frequency) which is also the default value
 *
 * \param[in] div               Clock divider
 */
__STATIC_INLINE void hw_lcdc_set_pixel_clk(uint8_t div)
{
        REG_SETF(LCDC, LCDC_CLKCTRL_REG, LCDC_CLK_DIV, div);
}

/**
 * \brief Set (secondary) clock divider of LCDC which controls the interface/format clock
 *
 * \note Source clock of this divider is the main clock of LCD controller. The period of the
 * generated clock is defined as : (LCDC_SEC_CLK_DIV + 1) x period_of_main_clock.
 *
 * \note Output clock of the serial interfaces is further divided by 2
 *
 * \param[in] div               Clock divider
 */
__STATIC_INLINE void hw_lcdc_set_iface_clk(uint8_t div)
{
        REG_SETF(LCDC, LCDC_CLKCTRL_REG, LCDC_SEC_CLK_DIV, div);
}

/**
 * \brief Configures the frequency of the external clock produced for the LCD internal refresh.
 *
 * \param[in] div               Clock divider as defined in \sa HW_LCDC_EXT_CLK
 */
__STATIC_INLINE void hw_lcdc_set_external_clk(HW_LCDC_EXT_CLK div)
{
        REG_SETF(CRG_COM, RESET_CLK_COM_REG, LCD_EXT_CLK_SEL, UINT32_MAX);
        REG_SETF(CRG_COM, SET_CLK_COM_REG, LCD_EXT_CLK_SEL, div);
}

/**
 * \brief Set layer FIFO parameters
 *
 * FIFO threshold controls at which threshold a DMA request is triggered and FIFO burst length
 * controls the amount of data DMA will try to fetch.
 *
 * \param[in] fifo_thr          FIFO input threshold (\sa HW_LCDC_FIFO_THR)
 * \param[in] burst_len         Burst length (\sa HW_LCDC_BURST_LEN)
 */
__STATIC_INLINE void set_layer_fifo_params(HW_LCDC_FIFO_THR fifo_thr, HW_LCDC_BURST_LEN burst_len)
{
        uint32_t lcdc_layer0_stride_reg = LCDC->LCDC_LAYER0_STRIDE_REG;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_STRIDE_REG, LCDC_L0_FIFO_THR, lcdc_layer0_stride_reg, fifo_thr);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_STRIDE_REG, LCDC_L0_BURST_LEN, lcdc_layer0_stride_reg, burst_len);
        LCDC->LCDC_LAYER0_STRIDE_REG = lcdc_layer0_stride_reg;
}

/**
 * \brief Set display background color
 *
 * \param[in] red               Red color used as background
 * \param[in] green             Green color used as background
 * \param[in] blue              Blue color used as background
 * \param[in] alpha             Alpha used as background
 */
__STATIC_INLINE void hw_lcdc_set_bg_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
        uint32_t lcdc_bgcolor_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_BGCOLOR_REG, LCDC_BG_RED, lcdc_bgcolor_reg, red);
        REG_SET_FIELD(LCDC, LCDC_BGCOLOR_REG, LCDC_BG_GREEN, lcdc_bgcolor_reg, green);
        REG_SET_FIELD(LCDC, LCDC_BGCOLOR_REG, LCDC_BG_BLUE, lcdc_bgcolor_reg, blue);
        REG_SET_FIELD(LCDC, LCDC_BGCOLOR_REG, LCDC_BG_ALPHA, lcdc_bgcolor_reg, alpha);
        LCDC->LCDC_BGCOLOR_REG = lcdc_bgcolor_reg;
}

/**
 * \brief Set parallel interface type
 *
 * \param[in] iface             Selected interface type
 *      \arg \c HW_LCDC_PAR_IF_JDI     JDI interface signals
 *      \arg \c HW_LCDC_PAR_IF_CLASSIC Classic parallel interface signals
 */
__STATIC_INLINE void hw_lcdc_set_parallel_iface(HW_LCDC_PAR_IF iface)
{
        REG_SETF(LCDC, LCDC_GPIO_REG, LCDC_PARIF_SEL, iface = HW_LCDC_PAR_IF_JDI ? 0 : 1);
}

/**
 * \brief Returns the sticky underflow status bit and clears it before exiting
 *
 * An underflow can occur if an LCDC DMA transaction has been initiated and required layer data is
 * not available in the required rate. Possible causes may be that bus/memory are either slow or
 * occupied by another master. Condition can be affected by the DMA level of the layer
 * (\sa hw_lcdc_set_layer_offset_dma_prefetch)
 *
 * \note Any write access to register LCDC_INTERRUPT_REG will clear status. As a result function
 * \ref hw_lcdc_get_sticky_underflow_status() must be called before.
 *
 * \return If an underflow has occurred
 * \retval true  Underflow has occurred
 * \retval false Underflow has not occurred
 */
__STATIC_INLINE bool hw_lcdc_get_sticky_underflow_status(void)
{
        bool underflow = REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_STICKY_UNDERFLOW) ? true : false;
        if (underflow) {
                /* Clean sticky bit by writing the interrupt register */
                uint32_t lcdc_interrupt_reg = LCDC->LCDC_INTERRUPT_REG;
                LCDC->LCDC_INTERRUPT_REG = lcdc_interrupt_reg;
        }
        return underflow;
}

/**
 * \brief Detect if LCD controller is active / inactive
 *
 * \return If LCDC is busy
 * \retval true  LCD controller is active
 * \retval false LCD controller is idle
 */
__STATIC_INLINE bool hw_lcdc_is_busy(void)
{
        uint32_t lcdc_status_reg = LCDC->LCDC_STATUS_REG;
        bool busy = false;
        busy |= REG_GET_FIELD(LCDC, LCDC_STATUS_REG, LCDC_FRAMEGEN_BUSY, lcdc_status_reg) ?
                                                                                            true :
                                                                                            false;
        busy |= REG_GET_FIELD(LCDC, LCDC_STATUS_REG, LCDC_DBIB_CMD_FIFO_EMPTY_N, lcdc_status_reg) ?
                true : false;
        return busy;
}

/**
 * \brief Get LCD Controller ID
 *
 * \return ID of the LCD controller
 */
__STATIC_INLINE uint32_t hw_lcdc_get_id(void)
{
        return REG_GETF(LCDC, LCDC_IDREG_REG, LCDC_ID);
}

/** \} */

/**
 * \name                Display controller functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Calculate the minimum stride size required for the provided parameters
 *
 * \param[in] format            Format (color) mode
 * \param[in] width             Width of the display
 *
 * \return Stride size in bytes of the mode
 */
uint32_t hw_lcdc_stride_size(HW_LCDC_LAYER_COLOR_MODE format, uint16_t width);

/**
 * \brief Initializes the LCD Controller module and driver
 *
 * \param[in] cfg               Configuration parameters
 *
 * \return Operation status
 * \retval HW_LCDC_ERR An error occurred
 * \retval HW_LCDC_OK  Operation completed successfully
 */
HW_LCDC_STATUS hw_lcdc_init(const hw_lcdc_config_t *cfg);

/**
 * \brief Sets displaying timing parameters
 *
 * \param[in] params            Display parameters
 */
void hw_lcdc_set_timing(const hw_lcdc_display_t *params);

/**
 * \brief Set the update region of the screen (screen must support partial update)
 *
 * If provided parameters are not valid, they are modified accordingly.
 *
 * \param[in,out] frame         Frame parameters
 */
void hw_lcdc_set_update_region(hw_lcdc_frame_t *frame);

/**
 * \brief Set layer parameters.
 *
 * Enable the layer and set attributes to it. LCD controller uses a layer structure to overlay
 * the background which is a simple color defined by the background color \sa hw_lcdc_set_bg_color().
 * The layer can be disabled (only background will be displayed) or can be placed anywhere in the
 * horizontal plane of the LCD. It can be placed even outside (partially or not) of the visual
 * boundaries of the LCD. Layer is capable of displaying any sized image that can be described with
 * the \ref hw_lcdc_layer_t structure.
 *
 * \param[in] enable            Enable / disable display of the layer
 * \param[in] layer             Layer attributes structure
 *
 * \return If layer is enabled and has a part inside the LCD frame.
 * \retval true  Layer is enabled
 * \retval false Layer is not enabled or not visible
 *
 * \warning Function will NOT change the stride parameter if continuous mode is on. In such case
 * function \ref hw_lcdc_set_stride() must be used to set stride at next frame end event.
 */
bool hw_lcdc_set_layer(bool enable, const hw_lcdc_layer_t *layer);

/**
 * \brief Set layer stride if it was not permitted by \ref hw_lcdc_set_layer() function
 *
 * This function should be used only in case of continuous mode update, when the change of stride is
 * not permitted by \ref hw_lcdc_set_layer() function.
 *
 * \warning It MUST be called at the frame end event that follows the call of
 * \ref hw_lcdc_set_layer() function
 *
 * \note It uses parameters of the layer settings to calculate stride in case that it is not
 * explicitly set and it does not affect any other layer parameters.
 *
 * \param[in] layer             Layer attributes structure
 */
void hw_lcdc_set_stride(const hw_lcdc_layer_t *layer);

/**
 * \brief Set chip select pin configuration
 *
 * \note In most use cases, chip select does not need to be configured, it is automatically done by
 * function \sa hw_lcdc_init()
 *
 * \param[in] state             Chip select configuration as defined in \sa HW_LCDC_SCS_CFG
 */
void hw_lcdc_set_scs(HW_LCDC_SCS_CFG state);

/**
 * \brief Sets hold flag to bind commands and data
 *
 * \param[in] enable            Enable/disable commands binding with data
 */
void hw_lcdc_set_hold(bool enable);

/**
 * \brief Set the tearing effect detection state
 *
 * \param[in] enable            Enable/disable the tearing effect detection
 * \param[in] polarity          TE level to detect
 */
void hw_lcdc_set_tearing_effect(bool enable, HW_LCDC_TE polarity);

/**
 * \brief Performs a single frame update to the screen using the configured physical interface
 */
void hw_lcdc_send_one_frame(void);

/**
 * \brief Enables the continuous update of the LCD controller.
 *
 * \note Only parallel LCDs (HW_LCDC_PHY_JDI_PARALLEL) support continuous mode update.
 *
 * \param[in] enable            Enable / disable the continuous update
 */
void hw_lcdc_set_continuous_mode(bool enable);
/** \} */

/**
 * \name                MIPI functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Send command or data to MIPI Interface
 *
 * \param[in] type              Type of instruction as defined in \sa HW_LCDC_MIPI
 * \param[in] value             Command or data as defined in (but not limited to) \sa HW_LCDC_MIPI_DCS
 */
void hw_lcdc_mipi_cmd(HW_LCDC_MIPI type, HW_LCDC_MIPI_DCS value);

/**
 * \brief Set the configuration register parameters
 *
 * \param[in] cfg               Configuration flag(s) as defined in \sa HW_LCDC_MIPI_CFG
 */
void hw_lcdc_mipi_cfg_out(HW_LCDC_MIPI_CFG cfg);

/**
 * \brief Get the configuration register parameters
 *
 * \return Configuration flag(s) as defined in \sa HW_LCDC_MIPI_CFG
 */
__STATIC_INLINE HW_LCDC_MIPI_CFG hw_lcdc_get_mipi_cfg(void)
{
        return LCDC->LCDC_DBIB_CFG_REG;
}

/**
 * \brief Enable display by turning display on and exiting sleep mode
 */
void hw_lcdc_mipi_enable(void);

/**
 * \brief Disable display by entering sleep mode and turning display off
 */
void hw_lcdc_mipi_disable(void);

/**
 * \brief Set the color mode (pixel format) to the LCD display
 *
 * \param[in] mode              Color mode (format)
 */
void hw_lcdc_mipi_set_mode(uint8_t mode);

/**
 * \brief Set screen dimensions that are updated by the controller
 *
 * \warning Controller timings must be set accordingly to match the dimensions provided to screen.
 * \sa hw_lcdc_set_timing()
 *
 * \param[in] frame             Frame parameters
 */
void hw_lcdc_mipi_set_position(const hw_lcdc_frame_t *frame);

/**
 * \brief Set partial mode of the screen (disables screen portion)
 *
 * \param[in] frame             Frame parameters
 */
void hw_lcdc_mipi_set_partial_mode(const hw_lcdc_frame_t *frame);

/**
 * \brief Inform screen that a new frame buffer will be transmitted. \sa hw_lcdc_set_mode().
 */
void hw_lcdc_mipi_start_frame_transfer(void);
/** \} */

/**
 * \name                JDI / Sharp functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief JDI serial / Sharp specific function to send a command to the LCD
 *
 * \param[in] cmd               Command byte to be sent
 */
void hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD cmd);
/**
 * \brief JDI serial / Sharp specific function to send a command to clear LCD contents
 */
void hw_lcdc_jdi_serial_clear(void);

/**
 * \brief JDI serial / Sharp specific function to exit blinking mode (if supported by LCD)
 */
void hw_lcdc_jdi_serial_blink_off(void);

/**
 * \brief JDI serial / Sharp specific function to start blinking with inverted colors (if supported by LCD)
 */
void hw_lcdc_jdi_serial_blink_inv_colors(void);

/**
 * \brief JDI serial / Sharp specific function to start blinking with white color (if supported by LCD)
 */
void hw_lcdc_jdi_serial_blink_white(void);

/**
 * \brief JDI serial / Sharp specific function to start blinking with black color (if supported by LCD)
 */
void hw_lcdc_jdi_serial_blink_black(void);

/**
 * \brief JDI parallel specific function to set exact timings of produced control signals
 */
void hw_lcdc_jdi_parallel_set_timings(const hw_lcdc_jdi_parallel_timings_t *jdip_timings);
/** \} */

/**
 * \name                Interrupt functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set callback function to be called upon an interrupt event
 *
 * \param[in] cb                Callback function to be called
 * \param[in] user_data         Parameters to be passed to callback function
 */
void hw_lcdc_set_callback(hw_lcdc_callback cb, void *user_data);

/**
 * \brief Enable/disable the VSync interrupt
 *
 * \param[in] enable            Enable/disable VSync
 *
 * \note VSYNC and Tearing Effect interrupts are enabled with LCDC_VSYNC_IRQ_EN. To enable tearing
 * effect detection, bit LCDC_DBIB_CFG_REG[LCDC_DBIB_TE_DIS] must be set.
 * \sa hw_lcdc_set_tearing_effect()
 */
__STATIC_INLINE void hw_lcdc_enable_vsync_irq(bool enable)
{
        REG_SETF(LCDC, LCDC_INTERRUPT_REG, LCDC_VSYNC_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the HSync interrupt
 *
 * \param[in] enable            Enable/disable HSync
 */
__STATIC_INLINE void hw_lcdc_enable_hsync_irq(bool enable)
{
        REG_SETF(LCDC, LCDC_INTERRUPT_REG, LCDC_HSYNC_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the "frame end" interrupt
 *
 * \param[in] enable            Enable/disable frame end
 */
__STATIC_INLINE void hw_lcdc_enable_frame_end_irq(bool enable)
{
        REG_SETF(LCDC, LCDC_INTERRUPT_REG, LCDC_FRAME_END_IRQ_EN, enable ? 1 : 0);
}

/**
 * \brief Enable/disable the tearing effect interrupt
 *
 * \param[in] enable            Enable/disable tearing effect
 *
 * \note To enable tearing effect detection, bit LCDC_DBIB_CFG_REG[LCDC_DBIB_TE_DIS] must be set.
 * \sa hw_lcdc_set_tearing_effect()
 */
__STATIC_INLINE void hw_lcdc_enable_tearing_effect_irq(bool enable)
{
        REG_SETF(LCDC, LCDC_INTERRUPT_REG, LCDC_TE_IRQ_EN, enable ? 1 : 0);
}
/** \} */


/**
 * \name                State functions
 *****************************************************************************************
 * \{
 */

/**
 * \brief Check if the LCD interface is active.
 *
 * \return true, if it is active, else false
 *
 */
__STATIC_INLINE bool hw_lcdc_is_active(void)
{
        return (REG_GETF(CRG_SYS, CLK_SYS_REG, LCD_ENABLE) == 1);
}

/**
 * \brief Check if the LCD interface is active and clocked by div1 clock.
 *
 * \return true, if it is active and clocked by div1 clock, else false
 *
 */
__STATIC_INLINE bool hw_lcdc_clk_is_div1(void)
{
        uint32_t val = CRG_SYS->CLK_SYS_REG;

        return ((val & REG_MSK(CRG_SYS, CLK_SYS_REG, LCD_ENABLE)) &&
                        (val & REG_MSK(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL)));
}

/** \} */

#endif /* dg_configUSE_HW_LCDC */


#endif /* HW_LCDC_H */

/**
 * \}
 * \}
 * \}
 */
