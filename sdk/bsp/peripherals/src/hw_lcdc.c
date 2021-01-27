/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup LCD_CONTROLLER
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file hw_lcdc.c
 *
 * @brief Implementation of the LCD Controller Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#if dg_configUSE_HW_LCDC

#include <sdk_defs.h>
#include "hw_lcdc.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define LCDC_MAGIC              (0x87452365)

#define LCDC_WORD_BITS          (32)
#define MIN_PART_UPDATE_WIDTH   (4)

#define JDI_SERIAL_CFG_DEFAULT  ( HW_LCDC_MIPI_CFG_RESET    | \
                                  HW_LCDC_MIPI_CFG_DMA      | \
                                  HW_LCDC_MIPI_CFG_SPI_JDI  | \
                                  HW_LCDC_MIPI_CFG_SPI_HOLD | \
                                  HW_LCDC_MIPI_CFG_TE_DIS   )

#define SPI_PHY_CONFIG          ( HW_LCDC_MIPI_CFG_SPI_CSX_V | \
                                  HW_LCDC_MIPI_CFG_SPI_CPHA  | \
                                  HW_LCDC_MIPI_CFG_SPI_CPOL  )

#define CEILING_FUNC(quotient, divisor)         (((quotient) + ((divisor) - 1)) / (divisor))

/**
 * \brief LCD Controller low level driver internal data
 */
typedef struct {
        hw_lcdc_callback        cb;             //!< User callback function
        void                   *cb_data;        //!< User callback data
        hw_lcdc_frame_t         active_area;    //!< Active area of the LCD that is updated
        HW_LCDC_MIPI_CFG        config;         //!< Active configuration
        HW_LCDC_PHY             phy;            //!< Physical connection type
        HW_LCDC_JDIS_CMD        jdis_update_cmd;//!< JDI/Sharp update/refresh command
} LCDC_Data;

/**
 * \brief LCD Controller low level driver internal data
 *
 * \warning LCD Controller data are not retained. The user must ensure that they are updated after
 * exiting sleep.
 */
static LCDC_Data lcdc_data;

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 * \name                Register functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Set display resolution
 *
 * \param[in] x                 Resolution X in pixels
 * \param[in] y                 Resolution Y in pixels
 */
__STATIC_INLINE void set_resolution(uint16_t x, uint16_t y)
{
        uint32_t lcdc_resxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_RESXY_REG, LCDC_RES_X, lcdc_resxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_RESXY_REG, LCDC_RES_Y, lcdc_resxy_reg, y);
        LCDC->LCDC_RESXY_REG = lcdc_resxy_reg;
}

/**
 * \brief Get display resolution
 *
 * \param[out] x                Resolution X in pixels
 * \param[out] y                Resolution Y in pixels
 */
__STATIC_INLINE void get_resolution(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_resxy_reg = LCDC->LCDC_RESXY_REG;
        *x = REG_GET_FIELD(LCDC, LCDC_RESXY_REG, LCDC_RES_X, lcdc_resxy_reg);
        *y = REG_GET_FIELD(LCDC, LCDC_RESXY_REG, LCDC_RES_Y, lcdc_resxy_reg);
}

/**
 * \brief Set front porch settings
 *
 * \param[in] x                 Front porch X (lines)
 * \param[in] y                 Front porch Y (pixel clocks)
 */
__STATIC_INLINE void set_front_porch(uint16_t x, uint16_t y)
{
        uint32_t lcdc_frontporchxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_FRONTPORCHXY_REG, LCDC_FPORCH_X, lcdc_frontporchxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_FRONTPORCHXY_REG, LCDC_FPORCH_Y, lcdc_frontporchxy_reg, y);
        LCDC->LCDC_FRONTPORCHXY_REG = lcdc_frontporchxy_reg;
}

/**
 * \brief Get front porch settings
 *
 * \param[out] x                Front porch X (lines)
 * \param[out] y                Front porch Y (pixel clocks)
 */
__STATIC_INLINE void get_front_porch(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_frontporchxy_reg = LCDC->LCDC_FRONTPORCHXY_REG;
        *x = REG_GET_FIELD(LCDC, LCDC_FRONTPORCHXY_REG, LCDC_FPORCH_X, lcdc_frontporchxy_reg);
        *y = REG_GET_FIELD(LCDC, LCDC_FRONTPORCHXY_REG, LCDC_FPORCH_Y, lcdc_frontporchxy_reg);
}

/**
 * \brief Set blanking period
 *
 * \param[in] x                 Blanking period X (VSYNC lines)
 * \param[in] y                 Blanking period Y (HSYNC pulse length)
 */
__STATIC_INLINE void set_blanking(uint16_t x, uint16_t y)
{
        uint32_t lcdc_blankingxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_BLANKINGXY_REG, LCDC_BLANKING_X, lcdc_blankingxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_BLANKINGXY_REG, LCDC_BLANKING_Y, lcdc_blankingxy_reg, y);
        LCDC->LCDC_BLANKINGXY_REG = lcdc_blankingxy_reg;
}

/**
 * \brief Get blanking period
 *
 * \param[out] x                Blanking period X (VSYNC lines)
 * \param[out] y                Blanking period Y (HSYNC pulse length)
 */
__STATIC_INLINE void get_blanking(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_blankingxy_reg = LCDC->LCDC_BLANKINGXY_REG;
        *x = REG_GET_FIELD(LCDC, LCDC_BLANKINGXY_REG, LCDC_BLANKING_X, lcdc_blankingxy_reg);
        *y = REG_GET_FIELD(LCDC, LCDC_BLANKINGXY_REG, LCDC_BLANKING_Y, lcdc_blankingxy_reg);
}

/**
 * \brief Set back porch settings
 *
 * \param[in] x                 Back porch X (lines)
 * \param[in] y                 Back porch Y (pixel clocks)
 */
__STATIC_INLINE void set_back_porch(uint16_t x, uint16_t y)
{
        uint32_t lcdc_backporchxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_BACKPORCHXY_REG, LCDC_BPORCH_X, lcdc_backporchxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_BACKPORCHXY_REG, LCDC_BPORCH_Y, lcdc_backporchxy_reg, y);
        LCDC->LCDC_BACKPORCHXY_REG = lcdc_backporchxy_reg;
}

/**
 * \brief Get back porch settings
 *
 * \param[out] x                Back porch X (lines)
 * \param[out] y                Back porch Y (pixel clocks)
 */
__STATIC_INLINE void get_back_porch(uint16_t *x, uint16_t *y)
{
        uint32_t lcdc_backporchxy_reg = LCDC->LCDC_BACKPORCHXY_REG;
        *x = REG_GET_FIELD(LCDC, LCDC_BACKPORCHXY_REG, LCDC_BPORCH_X, lcdc_backporchxy_reg);
        *y = REG_GET_FIELD(LCDC, LCDC_BACKPORCHXY_REG, LCDC_BPORCH_Y, lcdc_backporchxy_reg);
}

/**
 * \brief Set layer mode settings
 *
 * \param[in] enable            Enable/disable layer
 * \param[in] color             Color mode
 */
__STATIC_INLINE void set_layer_mode(bool enable, HW_LCDC_LAYER_COLOR_MODE color)
{
        uint32_t lcdc_layer0_mode_reg = LCDC->LCDC_LAYER0_MODE_REG;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_MODE_REG, LCDC_L0_COLOUR_MODE, lcdc_layer0_mode_reg, color);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_MODE_REG, LCDC_L0_EN, lcdc_layer0_mode_reg, enable ? 1 : 0);
        LCDC->LCDC_LAYER0_MODE_REG = lcdc_layer0_mode_reg;
}

/**
 * \brief Set layer start (offset in pixels)
 *
 * \param[in] x                 Start X in pixels
 * \param[in] y                 Start Y in pixels
 *
 * \warning Register will preserve the previous value when read until a write to register
 * LCDC_MODE_REG is performed (\sa hw_lcdc_set_mode())
 */
__STATIC_INLINE void set_layer_start(int16_t x, int16_t y)
{
        uint32_t lcdc_layer0_startxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_STARTXY_REG, LCDC_L0_START_X, lcdc_layer0_startxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_STARTXY_REG, LCDC_L0_START_Y, lcdc_layer0_startxy_reg, y);
        LCDC->LCDC_LAYER0_STARTXY_REG = lcdc_layer0_startxy_reg;
}

/**
 * \brief Set layer size in pixels
 *
 * \param[in] x                 Size X in pixels
 * \param[in] y                 Size Y in pixels
 *
 * \warning Register will preserve the previous value when read until a write to register
 * LCDC_MODE_REG is performed (\sa hw_lcdc_set_mode())
 */
__STATIC_INLINE void set_layer_size(uint16_t x, uint16_t y)
{
        uint32_t lcdc_layer0_sizexy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_SIZEXY_REG, LCDC_L0_SIZE_X, lcdc_layer0_sizexy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_SIZEXY_REG, LCDC_L0_SIZE_Y, lcdc_layer0_sizexy_reg, y);
        LCDC->LCDC_LAYER0_SIZEXY_REG = lcdc_layer0_sizexy_reg;
}

/**
 * \brief Set layer base address
 *
 * \param[in] addr              Base address
 *
 * \warning Register will preserve the previous value when read until a write to register
 * LCDC_MODE_REG is performed (\sa hw_lcdc_set_mode())
 */
__STATIC_INLINE void set_layer_base_addr(uint32_t addr)
{
        REG_SETF(LCDC, LCDC_LAYER0_BASEADDR_REG, LCDC_L0_FB_ADDR, addr);
}

/**
 * \brief Set layer stride (distance from line to line in bytes)
 *
 * \param[in] stride            Distance in bytes between consecutive lines
 */
__STATIC_INLINE void set_layer_stride(int16_t stride)
{
        REG_SETF(LCDC, LCDC_LAYER0_STRIDE_REG, LCDC_L0_STRIDE, stride);
}

/**
 * \brief Set layer resolution in pixels
 *
 * \param[in] x                 Resolution X in pixels
 * \param[in] y                 Resolution Y in pixels
 *
 * \warning Register will preserve the previous value when read until a write to register
 * LCDC_MODE_REG is performed (\sa hw_lcdc_set_mode())
 */
__STATIC_INLINE void set_layer_resolution(uint16_t x, uint16_t y)
{
        uint32_t lcdc_layer0_resxy_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_RESXY_REG, LCDC_L0_RES_X, lcdc_layer0_resxy_reg, x);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_RESXY_REG, LCDC_L0_RES_Y, lcdc_layer0_resxy_reg, y);
        LCDC->LCDC_LAYER0_RESXY_REG = lcdc_layer0_resxy_reg;
}

/**
 * \brief Set layer horizontal offset in pixels
 *
 * \param[in] offset            Offset X in pixels
 * \param[in] level             FIFO pre-fetch level as defined in \sa HW_LCDC_FIFO_PREFETCH_LVL
 *
 * \warning Register will preserve the previous value when read until a write to register
 * LCDC_MODE_REG is performed (\sa hw_lcdc_set_mode())
 */
__STATIC_INLINE void set_layer_offset_dma_prefetch(int16_t offset, HW_LCDC_FIFO_PREFETCH_LVL level)
{
        uint32_t lcdc_layer0_offsetx_reg = 0;
        REG_SET_FIELD(LCDC, LCDC_LAYER0_OFFSETX_REG, LCDC_L0_OFFSETX, lcdc_layer0_offsetx_reg, offset);
        REG_SET_FIELD(LCDC, LCDC_LAYER0_OFFSETX_REG, LCDC_L0_DMA_PREFETCH, lcdc_layer0_offsetx_reg, level);
        LCDC->LCDC_LAYER0_OFFSETX_REG = lcdc_layer0_offsetx_reg;
}
/** \} */

/**
 * \name                Display controller functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Return the size in bytes of each display mode
 *
 * \param[in] format            Format (color) mode
 *
 * \return Size in bytes of the mode
 */
__STATIC_INLINE uint8_t format_size(HW_LCDC_LAYER_COLOR_MODE format)
{
        switch (format & 0xff) {
        case HW_LCDC_LCM_RGBA8888:
        case HW_LCDC_LCM_ARGB8888:
        case HW_LCDC_LCM_ABGR8888:
        case HW_LCDC_LCM_BGRA8888:
                return 4;
        case HW_LCDC_LCM_RGBA5551:
        case HW_LCDC_LCM_RGB565:
                return 2;
        case HW_LCDC_LCM_RGB332:
        case HW_LCDC_LCM_L8:
        case HW_LCDC_LCM_L1:
        case HW_LCDC_LCM_L4:
                return 1;
        default:
                return 0;
        }
}

/**
 * \brief Return the size in bits of each display mode
 *
 * \param[in] format            Format (color) mode
 *
 * \return Size in bits of the mode
 */
__STATIC_INLINE uint8_t format_size_bits(HW_LCDC_LAYER_COLOR_MODE format)
{
        switch (format & 0xff) {
        case HW_LCDC_LCM_RGBA8888:
        case HW_LCDC_LCM_ARGB8888:
        case HW_LCDC_LCM_ABGR8888:
        case HW_LCDC_LCM_BGRA8888:
                return 32;
        case HW_LCDC_LCM_RGBA5551:
        case HW_LCDC_LCM_RGB565:
                return 16;
        case HW_LCDC_LCM_RGB332:
        case HW_LCDC_LCM_L8:
                return 8;
        case HW_LCDC_LCM_L4:
                return 4;
        case HW_LCDC_LCM_L1:
                return 1;
        default:
                return 0;
        }
}

/**
 * \brief Reverse a byte MSB to LSB wise
 *
 * \param[in] val               Byte to be reversed
 *
 * \return The reversed byte
 */
__STATIC_INLINE uint8_t byte_reverse(uint8_t val)
{
        return __RBIT(val) >> 24;
}

uint32_t hw_lcdc_stride_size(HW_LCDC_LAYER_COLOR_MODE format, uint16_t width)
{
        int stride;
        switch (format) {
        case HW_LCDC_LCM_L1:
                stride = CEILING_FUNC(width, 8);
                break;
        case HW_LCDC_LCM_L4:
                stride = CEILING_FUNC(width, 2);
                break;
        default:
                stride = width * format_size(format);
                break;
        }
        return stride;
}

HW_LCDC_STATUS hw_lcdc_init(const hw_lcdc_config_t *cfg)
{
        HW_LCDC_MODE mode = HW_LCDC_MODE_DISABLE;
        uint32_t clk_sys_reg;

        if (!cfg) {
                return HW_LCDC_ERR;
        }

        lcdc_data.phy = cfg->phy_type;

        if (cfg->phy_type == HW_LCDC_PHY_NONE) {
                /* Ensure that block is not busy */
                while (hw_lcdc_is_busy());

                NVIC_DisableIRQ(LCD_CONTROLLER_IRQn);
                NVIC_ClearPendingIRQ(LCD_CONTROLLER_IRQn);

                GLOBAL_INT_DISABLE();
                REG_SET_BIT(CRG_SYS, CLK_SYS_REG, LCD_RESET_REQ);
                clk_sys_reg = CRG_SYS->CLK_SYS_REG;
                REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_RESET_REQ, clk_sys_reg, 0);
                REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL, clk_sys_reg, 0);
                REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_ENABLE, clk_sys_reg, 0);
                CRG_SYS->CLK_SYS_REG = clk_sys_reg;
                GLOBAL_INT_RESTORE();
                return HW_LCDC_OK;
        }

        GLOBAL_INT_DISABLE();
        clk_sys_reg = CRG_SYS->CLK_SYS_REG;
        REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_CLK_SEL, clk_sys_reg,
                (cfg->iface_freq & HW_LCDC_CLK_PLL_BIT) ? 1 : 0);
        REG_SET_FIELD(CRG_SYS, CLK_SYS_REG, LCD_ENABLE, clk_sys_reg, 1);
        CRG_SYS->CLK_SYS_REG = clk_sys_reg;
        GLOBAL_INT_RESTORE();

        if (hw_lcdc_get_id() != LCDC_MAGIC) {
                GLOBAL_INT_DISABLE();
                REG_CLR_BIT(CRG_SYS, CLK_SYS_REG, LCD_ENABLE);
                GLOBAL_INT_RESTORE();
                return HW_LCDC_ERR;
        }

        hw_lcdc_set_mode(HW_LCDC_MODE_DISABLE);

        REG_SETF(LCDC, LCDC_INTERRUPT_REG, LCDC_IRQ_TRIGGER_SEL, 1);
        hw_lcdc_enable_vsync_irq(false);
        NVIC_EnableIRQ(LCD_CONTROLLER_IRQn);

        hw_lcdc_set_iface_clk(
                ((cfg->phy_type == HW_LCDC_PHY_CLASSIC_PARALLEL ?
                        cfg->iface_freq : (cfg->iface_freq / 2)) - 1) & HW_LCDC_CLK_DIV_MSK);

        switch (cfg->phy_type) {
        case HW_LCDC_PHY_MIPI_SPI3:
                lcdc_data.config = HW_LCDC_MIPI_CFG_RESET | HW_LCDC_MIPI_CFG_SPI3
                        | HW_LCDC_MIPI_CFG_DMA | HW_LCDC_MIPI_CFG_TE_DIS;
                break;
        case HW_LCDC_PHY_MIPI_SPI4:
                lcdc_data.config = HW_LCDC_MIPI_CFG_RESET | HW_LCDC_MIPI_CFG_SPI4
                        | HW_LCDC_MIPI_CFG_DMA | HW_LCDC_MIPI_CFG_TE_DIS;
                break;
        case HW_LCDC_PHY_JDI_SPI:
                lcdc_data.config = JDI_SERIAL_CFG_DEFAULT | SPI_PHY_CONFIG | HW_LCDC_MIPI_CFG_SPI4;
                switch (cfg->format) {
                case HW_LCDC_OCM_8RGB111_2:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_4BIT;
                        break;
                case HW_LCDC_OCM_RGB111:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_NATIVE;
                        break;
                case HW_LCDC_OCM_L1:
                default:
                        lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_1BIT;
                        break;
                }
                break;
        case HW_LCDC_PHY_SHARP_SPI:
                lcdc_data.config = JDI_SERIAL_CFG_DEFAULT | SPI_PHY_CONFIG | HW_LCDC_MIPI_CFG_SPI4
                        | HW_LCDC_MIPI_CFG_INV_ADDR;
                lcdc_data.jdis_update_cmd = HW_LCDC_JDIS_CMD_UPDATE_NATIVE;
                break;
        case HW_LCDC_PHY_JDI_PARALLEL:
                hw_lcdc_mipi_cfg_out(HW_LCDC_MIPI_CFG_JDI_SOFT_RST);
                lcdc_data.config = HW_LCDC_MIPI_CFG_DMA | HW_LCDC_MIPI_CFG_RESET
                        | HW_LCDC_MIPI_CFG_TE_DIS;
                mode = HW_LCDC_MODE_JDIMIP | HW_LCDC_MODE_SCANDOUBLE;
                break;
        case HW_LCDC_PHY_CLASSIC_PARALLEL:
                hw_lcdc_set_parallel_iface(HW_LCDC_PAR_IF_CLASSIC);
                lcdc_data.config = HW_LCDC_MIPI_CFG_DMA | HW_LCDC_MIPI_CFG_RESET
                        | HW_LCDC_MIPI_CFG_TE_DIS;
                break;
        case HW_LCDC_PHY_CUSTOM:
                lcdc_data.config = 0;
                break;
        default:
                return HW_LCDC_ERR;
        }

        lcdc_data.cb = NULL;

        /* Modify predefined settings using the configuration parameters */
        lcdc_data.config ^= cfg->cfg_extra_flags;

        hw_lcdc_mipi_cfg_out(lcdc_data.config | cfg->format);
        hw_lcdc_set_mode(mode ^ cfg->mode);

        return HW_LCDC_OK;
}

void hw_lcdc_set_timing(const hw_lcdc_display_t *params)
{
        uint16_t dc_fpx, dc_fpy, dc_blx, dc_bly, dc_bpx, dc_bpy;
        uint16_t resy = params->resy;

        if (lcdc_data.phy == HW_LCDC_PHY_JDI_PARALLEL) {
                resy *= 2;
        }
        else if (lcdc_data.phy == HW_LCDC_PHY_JDI_SPI || lcdc_data.phy == HW_LCDC_PHY_SHARP_SPI) {
                /* Add an extra line at the end to produce the required dummy bytes */
                ++resy;
        }

        dc_fpx = params->resx + params->fpx;
        dc_fpy = resy + params->fpy;
        dc_blx = dc_fpx + params->blx;
        dc_bly = dc_fpy + params->bly;
        dc_bpx = dc_blx + params->bpx;
        dc_bpy = dc_bly + params->bpy;

        lcdc_data.active_area.startx = 0;
        lcdc_data.active_area.starty = 0;
        lcdc_data.active_area.endx = params->resx - 1;
        lcdc_data.active_area.endy = params->resy - 1;

        set_resolution(params->resx, resy);
        set_front_porch(dc_fpx, dc_fpy);
        set_blanking(dc_blx, dc_bly);
        set_back_porch(dc_bpx, dc_bpy);
}

void hw_lcdc_set_update_region(hw_lcdc_frame_t *frame)
{
        int16_t modx, mody;
        uint16_t fpx, fpy;
        uint16_t blx, bly;
        uint16_t bpx, bpy;
        uint16_t resx, resy;
        uint16_t width = frame->endx - frame->startx + 1;

        ASSERT_ERROR(frame->endx >= frame->startx);
        ASSERT_ERROR(frame->endy >= frame->starty);

        if (lcdc_data.phy == HW_LCDC_PHY_JDI_PARALLEL) {
                REG_SETF(LCDC, LCDC_JDI_ENB_START_HLINE_REG, LCDC_JDI_ENB_START_HLINE,
                        2 * (frame->starty + 1));
                REG_SETF(LCDC, LCDC_JDI_ENB_END_HLINE_REG, LCDC_JDI_ENB_END_HLINE,
                        2 * (frame->endy + 1) + 1);
                /* No need to change timing of the LCD, just the ENB signal */
                return;
        }

        get_resolution(&resx, &resy);
        get_front_porch(&fpx, &fpy);
        get_blanking(&blx, &bly);
        get_back_porch(&bpx, &bpy);

        /* Workaround for "Errata issue 297": LCDC: Wrong Partial Refresh */
        /* If columns are less than 4, increase the update area */
        /* Firstly check how much can be increased on the left */
        if (width < MIN_PART_UPDATE_WIDTH) {
                uint16_t dec_startx = MIN(MIN_PART_UPDATE_WIDTH - width, frame->startx);
                frame->startx -= dec_startx;
                width += dec_startx;
        }
        /* If increase on the left not sufficient (too close to the border), increase the rest on
         * the right. No need to perform a limit check since we have reached the left border of the
         * screen */
        if (width < MIN_PART_UPDATE_WIDTH) {
                frame->endx += MIN_PART_UPDATE_WIDTH - width;
        }

        modx = frame->endx - lcdc_data.active_area.endx
                - (frame->startx - lcdc_data.active_area.startx);
        mody = frame->endy - lcdc_data.active_area.endy
                - (frame->starty - lcdc_data.active_area.starty);

        set_resolution(resx + modx, resy + mody);
        set_front_porch(fpx + modx, fpy + mody);
        set_blanking(blx + modx, bly + mody);
        set_back_porch(bpx + modx, bpy + mody);

        lcdc_data.active_area = *frame;
}

bool hw_lcdc_set_layer(bool enable, const hw_lcdc_layer_t *layer)
{

        if (enable) {
                uint16_t disp_resx, disp_resy;
                uint8_t pixels_per_word = LCDC_WORD_BITS / format_size_bits(layer->format);
                int16_t stride = layer->stride;
                uint32_t addr = black_orca_phy_addr(layer->baseaddr);
                uint32_t resx = layer->resx;
                uint32_t resy = layer->resy;
                int16_t sx = layer->startx - lcdc_data.active_area.startx;
                int16_t sy = layer->starty - lcdc_data.active_area.starty;
                uint32_t szx, szy;
                int16_t offset = 0;
                int endx, endy;

                if (lcdc_data.phy == HW_LCDC_PHY_JDI_PARALLEL) { /* Perform parallel only calculations */
                        if (sy >= 0) {
                                sy *= 2;
                                resy *= 2;
                        }
                        else {
                                resy = resy * 2 + sy;
                        }
                }

                szx = resx;
                szy = resy;
                endx = sx + resx;
                endy = sy + resy;

                get_resolution(&disp_resx, &disp_resy);

                if (stride == 0) {
                        stride = hw_lcdc_stride_size(layer->format, layer->resx);
                }

                if ((int)sx >= (int)disp_resx
                        || (int)sy >= (int)disp_resy
                        || endx <= 0
                        || endy <= 0) {
                        set_layer_mode(false, 0);
                        return false;
                }

                if (endx > (int)disp_resx) {
                        endx = disp_resx;
                }
                if (endy > (int)disp_resy) {
                        endy = disp_resy;
                }

                if (sy < 0) {
                        addr -= sy * stride;
                        szy = endy;
                        resy = endy;
                        sy = 0;
                }
                else {
                        szy = endy - sy;
                        resy = endy - sy;
                }

                if (sx > 0) {
                        szx = endx - sx;
                        if (endx >= (int)disp_resx) {
                                resx = ((((disp_resx - sx) - 1) / pixels_per_word) + 1)
                                        * pixels_per_word;
                        }
                        else {
                                resx = szx;
                        }
                }
                else {
                        //offset is negative
                        offset = (sx % pixels_per_word);
                        addr += (-sx) / pixels_per_word * 4;
                        sx = 0;
                        szx = endx;
                        resx = CEILING_FUNC(szx + (-offset), pixels_per_word) * pixels_per_word;
                }

                ASSERT_ERROR((addr & 0x3) == 0); /* Ensure base address is word aligned */
                ASSERT_ERROR((stride & 0x3) == 0); /* Ensure stride has a proper length */

                set_layer_base_addr(addr);
                set_layer_start(sx, sy);
                set_layer_offset_dma_prefetch(offset, layer->dma_prefetch_lvl);
                set_layer_size(szx, szy);
                set_layer_resolution(resx, resy);
                /* Permit change of stride only if continuous mode is off */
                if (!REG_GETF(LCDC, LCDC_MODE_REG, LCDC_MODE_EN)) {
                        set_layer_stride(stride);
                }
        }
        set_layer_mode(enable, layer->format);
        return enable;
}

void hw_lcdc_set_stride(const hw_lcdc_layer_t *layer)
{
        int16_t stride = layer->stride;

        if (!REG_GETF(LCDC, LCDC_MODE_REG, LCDC_MODE_EN)) {
                return;
        }

        if (stride == 0) {
                stride = hw_lcdc_stride_size(layer->format, layer->resx);
        }

        ASSERT_ERROR((stride & 0x3) == 0); /* Ensure stride has a proper length */

        set_layer_stride(stride);
}

void hw_lcdc_set_scs(HW_LCDC_SCS_CFG state)
{
        HW_LCDC_MIPI_CFG cfg = LCDC->LCDC_DBIB_CFG_REG & (~HW_LCDC_MIPI_CFG_FRC_CSX_1);
        switch (state) {
        case HW_LCDC_SCS_AUTO:
                hw_lcdc_mipi_cfg_out(cfg);
                break;
        case HW_LCDC_SCS_AUTO_INV:
                hw_lcdc_mipi_cfg_out(cfg | HW_LCDC_MIPI_CFG_SPI_CSX_V);
                break;
        case HW_LCDC_SCS_HIGH:
                hw_lcdc_mipi_cfg_out(cfg | HW_LCDC_MIPI_CFG_FRC_CSX_1);
                break;
        case HW_LCDC_SCS_LOW:
                hw_lcdc_mipi_cfg_out(cfg | HW_LCDC_MIPI_CFG_FRC_CSX_0);
                break;
        }
}

void hw_lcdc_set_hold(bool enable)
{
        uint32_t lcdc_dbib_cfg_reg = LCDC->LCDC_DBIB_CFG_REG;
        REG_SET_FIELD(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_HOLD, lcdc_dbib_cfg_reg,
                enable ? 1 : 0);
        hw_lcdc_mipi_cfg_out(lcdc_dbib_cfg_reg);
}

void hw_lcdc_set_tearing_effect(bool enable, HW_LCDC_TE polarity)
{
        uint32_t lcdc_dbib_cfg_reg = LCDC->LCDC_DBIB_CFG_REG;

        REG_SETF(LCDC, LCDC_GPIO_REG, LCDC_TE_INV, polarity == HW_LCDC_TE_LOW ? 0 : 1);

        REG_SET_FIELD(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_TE_DIS, lcdc_dbib_cfg_reg, enable ? 0 : 1);
        hw_lcdc_mipi_cfg_out(lcdc_dbib_cfg_reg);
}

void hw_lcdc_send_one_frame(void)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG;
        switch (lcdc_data.phy) {
        case HW_LCDC_PHY_MIPI_SPI3:
        case HW_LCDC_PHY_MIPI_SPI4:
                hw_lcdc_mipi_start_frame_transfer();
                break;
        case HW_LCDC_PHY_JDI_SPI:
        {
                uint8_t starty = lcdc_data.active_area.starty + 1;

                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, lcdc_data.jdis_update_cmd);
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_STORE_BADDR, starty);
                break;
        }
        case HW_LCDC_PHY_SHARP_SPI:
        {
                uint8_t starty = lcdc_data.active_area.starty + 1;

                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, lcdc_data.jdis_update_cmd);
                hw_lcdc_mipi_cmd(HW_LCDC_MIPI_STORE_BADDR, byte_reverse(starty));
                break;
        }
        default:
                break;
        }
        hw_lcdc_set_mode(mode | HW_LCDC_MODE_ONE_FRAME);
}

void hw_lcdc_set_continuous_mode(bool enable)
{
        HW_LCDC_MODE mode = LCDC->LCDC_MODE_REG;

        if (!enable || (lcdc_data.phy != HW_LCDC_PHY_JDI_PARALLEL
                && lcdc_data.phy != HW_LCDC_PHY_CLASSIC_PARALLEL)) {

                hw_lcdc_set_mode(mode & ~HW_LCDC_MODE_ENABLE);
        }
        else {
                hw_lcdc_set_mode(mode | HW_LCDC_MODE_ENABLE);

                /* Block until transmission of the first frame starts */
                while (!REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_STAT_VSYNC));
                while (REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_STAT_VSYNC));
        }
}
/** \} */

/**
 * \name                MIPI functions
 *****************************************************************************************
 * \{
 */

void hw_lcdc_mipi_cmd(HW_LCDC_MIPI type, HW_LCDC_MIPI_DCS value)
{
        uint32_t lcdc_dbib_cmd_reg = 0;
        bool cmd_bit = (type == HW_LCDC_MIPI_CMD || type == HW_LCDC_MIPI_STORE_BADDR) ? 1 : 0;

        /* Workaround for "Errata issue 285": LCD_SPI_DC not aligned correctly with the LCD_SPI_CLK */
        /* If SPI4 and not using hold and type changes wait until FIFO is empty. In any other case
         * wait until there is an empty space in FIFO */
        if (((LCDC->LCDC_DBIB_CFG_REG
                & (REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI_HOLD)
                        | REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI4_EN)))
                != REG_MSK(LCDC, LCDC_DBIB_CFG_REG, LCDC_DBIB_SPI4_EN))
                || (REG_GETF(LCDC, LCDC_DBIB_CMD_REG, LCDC_DBIB_CMD_SEND) == cmd_bit)) {
                while (REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_DBIB_CMD_FIFO_FULL));
        } else {
                while (REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_DBIB_CMD_PENDING));
        }

        REG_SET_FIELD(LCDC, LCDC_DBIB_CMD_REG, LCDC_DBIB_CMD_SEND, lcdc_dbib_cmd_reg, cmd_bit);

        REG_SET_FIELD(LCDC, LCDC_DBIB_CMD_REG, LCDC_DBIB_CMD_STORE, lcdc_dbib_cmd_reg,
                type == HW_LCDC_MIPI_STORE_BADDR ? 1 : 0);

        REG_SET_FIELD(LCDC, LCDC_DBIB_CMD_REG, LCDC_DBIB_CMD_VAL, lcdc_dbib_cmd_reg, value);

        LCDC->LCDC_DBIB_CMD_REG = lcdc_dbib_cmd_reg;
}

void hw_lcdc_mipi_cfg_out(HW_LCDC_MIPI_CFG cfg)
{
        /* Make sure command queue is not full */
        while (REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_DBIB_CMD_FIFO_FULL));

        LCDC->LCDC_DBIB_CFG_REG = cfg;
}

void hw_lcdc_mipi_enable(void)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_EXIT_SLEEP_MODE);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_DISPLAY_ON);
}

void hw_lcdc_mipi_disable(void)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_DISPLAY_OFF);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_ENTER_SLEEP_MODE);
}

void hw_lcdc_mipi_set_mode(uint8_t mode)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_PIXEL_FORMAT);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, mode);
}

void hw_lcdc_mipi_set_position(const hw_lcdc_frame_t *frame)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_COLUMN_ADDRESS);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->startx >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->startx & 0xFF);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endx >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endx & 0xFF);

        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_PAGE_ADDRESS);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->starty >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->starty & 0xFF);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endy >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endy & 0xFF);
}

void hw_lcdc_mipi_set_partial_mode(const hw_lcdc_frame_t *frame)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_PARTIAL_COLUMNS);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->startx >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->startx & 0xFF);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endx >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endx & 0xFF);

        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_SET_PARTIAL_ROWS);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->starty >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->starty & 0xFF);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endy >> 8);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_DATA, frame->endy & 0xFF);

        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_ENTER_PARTIAL_MODE);
}

void hw_lcdc_mipi_start_frame_transfer(void)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_MIPI_DCS_WRITE_MEMORY_START);
}
/** \} */

/**
 * \name                JDI / Sharp functions
 *****************************************************************************************
 * \{
 */
/**
 * \brief Release and restore HOLD flag
 *
 * Release the HOLD flag that binds commands and data to enable command transmission and restore it
 * if it was previously used.
 */
__STATIC_INLINE void jdi_serial_cmd_release(void)
{
        hw_lcdc_set_hold(false);
        while (REG_GETF(LCDC, LCDC_STATUS_REG, LCDC_DBIB_CMD_PENDING)); //make sure command is sent

        if (lcdc_data.config & HW_LCDC_MIPI_CFG_SPI_HOLD) {
                hw_lcdc_set_hold(true);
        }
}

void hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD cmd)
{
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, cmd);
        hw_lcdc_mipi_cmd(HW_LCDC_MIPI_CMD, HW_LCDC_JDIS_CMD_NOP);
        jdi_serial_cmd_release();
}

void hw_lcdc_jdi_serial_clear(void)
{
        hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD_CLEAR);
}

void hw_lcdc_jdi_serial_blink_off(void)
{
        hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD_BLINKOFF);
}

void hw_lcdc_jdi_serial_blink_inv_colors(void)
{
        hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD_BLINKINVERT);
}

void hw_lcdc_jdi_serial_blink_white(void)
{
        hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD_BLINKWHITE);
}

void hw_lcdc_jdi_serial_blink_black(void)
{
        hw_lcdc_jdi_serial_cmd_send(HW_LCDC_JDIS_CMD_BLINKBLACK);
}

void hw_lcdc_jdi_parallel_set_timings(const hw_lcdc_jdi_parallel_timings_t *jdip_timings)
{
        uint16_t resx, resy;
        uint16_t fx_blanking, fy_blanking;
        uint16_t bx_blanking, by_blanking;
        uint32_t hck_width;
        uint32_t xrst_width;
        uint32_t vst_width, vst_delay;
        uint32_t hst_width, hst_delay;
        uint32_t vck_delay;
        uint32_t enb_start_hline, enb_end_hline;
        uint32_t enb_start_clk, enb_width_clk;

        resx = jdip_timings->resx / 2;
        resy = jdip_timings->resy * 2;
        fx_blanking = jdip_timings->fxb;
        bx_blanking = jdip_timings->bxb;
        fy_blanking = jdip_timings->fyb;
        by_blanking = jdip_timings->byb;
        hck_width = jdip_timings->hck_width;
        xrst_width = jdip_timings->xrst_width
                * (hck_width * (((resx + fx_blanking + bx_blanking) / 2) * hck_width));
        vst_width = (resx + fx_blanking + bx_blanking) * hck_width;
        vst_delay = (((resx + fx_blanking + bx_blanking) / 2) * hck_width);
        vck_delay = vst_delay + ((hck_width * ((resx + fx_blanking + bx_blanking) / 2)));
        hst_width = hck_width / 2;
        hst_delay = hck_width;
        enb_start_hline = (jdip_timings->starty + 1) * 2;
        enb_end_hline = (jdip_timings->endy + 1) * 2 + 1;
        enb_start_clk = (resx + fx_blanking + bx_blanking) / 4;
        enb_width_clk = 2 * (resx + fx_blanking + bx_blanking) / 4;

        REG_SETF(LCDC, LCDC_JDI_RESXY_REG, LCDC_JDI_RES_X, resx);
        REG_SETF(LCDC, LCDC_JDI_RESXY_REG, LCDC_JDI_RES_Y, resy);

        /* In HCK half periods */
        REG_SETF(LCDC, LCDC_JDI_FBX_BLANKING_REG, LCDC_JDI_BXBLANKING, bx_blanking);
        REG_SETF(LCDC, LCDC_JDI_FBX_BLANKING_REG, LCDC_JDI_FXBLANKING, fx_blanking);

        /* In VCK half periods */
        REG_SETF(LCDC, LCDC_JDI_FBY_BLANKING_REG, LCDC_JDI_BYBLANKING, by_blanking);
        REG_SETF(LCDC, LCDC_JDI_FBY_BLANKING_REG, LCDC_JDI_FYBLANKING, fy_blanking);

        /* Number of the format clock cycles for the half period */
        REG_SETF(LCDC, LCDC_JDI_HCK_WIDTH_REG, LCDC_JDI_HCK_WIDTH, hck_width);
        /* Number of the format clock cycles of the XRST width */
        REG_SETF(LCDC, LCDC_JDI_XRST_WIDTH_REG, LCDC_JDI_XRST_WIDTH, xrst_width);
        /* Number of the format clock cycles of the VST width */
        REG_SETF(LCDC, LCDC_JDI_VST_WIDTH_REG, LCDC_JDI_VST_WIDTH, vst_width);
        /* Number of the format clock cycles of the HST width */
        REG_SETF(LCDC, LCDC_JDI_HST_WIDTH_REG, LCDC_JDI_HST_WIDTH, hst_width * 2);

        /* Number of the format clock cycles of the XRST-to-VST delay */
        REG_SETF(LCDC, LCDC_JDI_VST_DELAY_REG, LCDC_JDI_VST_DELAY, vst_delay);
        /* Number of the format clock cycles of the XRST-to-VCK delay */
        REG_SETF(LCDC, LCDC_JDI_VCK_DELAY_REG, LCDC_JDI_VCK_DELAY, vck_delay);
        /* Number of the format clock cycles of the VCK-to-HST delay */
        REG_SETF(LCDC, LCDC_JDI_HST_DELAY_REG, LCDC_JDI_HST_DELAY, hst_delay * 2);

        REG_SETF(LCDC, LCDC_JDI_ENB_START_HLINE_REG, LCDC_JDI_ENB_START_HLINE, enb_start_hline);
        REG_SETF(LCDC, LCDC_JDI_ENB_END_HLINE_REG, LCDC_JDI_ENB_END_HLINE, enb_end_hline);

        /* Number of the HCK half periods of the VCK-to-ENB delay */
        REG_SETF(LCDC, LCDC_JDI_ENB_START_CLK_REG, LCDC_JDI_ENB_START_CLK, enb_start_clk);
        /* Number of the HCK half periods of the ENB (high) width */
        REG_SETF(LCDC, LCDC_JDI_ENB_WIDTH_CLK_REG, LCDC_JDI_ENB_WIDTH_CLK, enb_width_clk);
}
/** \} */

/**
 * \name                Interrupt functions
 *****************************************************************************************
 * \{
 */

void hw_lcdc_set_callback(hw_lcdc_callback cb, void *user_data)
{
        lcdc_data.cb = cb;
        lcdc_data.cb_data = user_data;
}

/**
 * \brief LCD Controller Interrupt Handler
 *
 */
void LCD_Controller_Handler(void)
{
        hw_lcdc_callback cb;
        void *cb_data;

        SEGGER_SYSTEMVIEW_ISR_ENTER();

        cb = lcdc_data.cb;
        cb_data = lcdc_data.cb_data;
        if (cb) {
                /* Do not clear interrupt callback in case of continuous refresh */
                if (!REG_GETF(LCDC, LCDC_MODE_REG, LCDC_MODE_EN)) {
                        lcdc_data.cb = NULL;
                        lcdc_data.cb_data = NULL;
                }
                cb(cb_data);
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
/** \} */

#endif /* dg_configUSE_HW_LCDC */
/**
 * \}
 * \}
 * \}
 */
