/**
 ****************************************************************************************
 *
 * @file sys_audio_mgr.c
 *
 * @brief System Audio manager
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configSYS_AUDIO_MGR

#include <sdk_defs.h>

#include "sys_audio_mgr.h"
#include "osal.h"
#include "hw_apu_src.h"
#include "sys_power_mgr.h"
#include "resmgmt.h"

#define APU_DEFAULT_SAMPLING_RATE 8000
#define APU_DEFAULT_BITS_DEPTH 16
#define APU_DEFAULT_PDM_FREQUENCY 4000000
#define APU_DEFAULT_SRC_CLK 32000000

/*PCM defaults */
#define APU_DEFAULT_OUTPUT_MODE         HW_PCM_DO_OUTPUT_PUSH_PULL
#define APU_DEFAULT_PCM_CYCLE_PER_BIT   HW_PCM_ONE_CYCLE_PER_BIT
#define APU_DEFAULT_FSC_DELAY           HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT

typedef enum {
        DIRECTION_INPUT,
        DIRECTION_OUTPUT
} SYS_AUDIO_MGR_DIRECTION;


typedef struct {
        HW_DMA_CHANNEL dma_channel_number;
        sys_audio_mgr_buffer_ready_cb cb;
        sys_audio_path_t *path;
        sys_audio_mgr_buffer_data_block_t buff_block;
        void *app_ud;
} dma_user_data_t;

static void *internal_data[2];             /**< pointers to internal callback routine data for each
                                               audio channel - internal use only */
static sys_audio_path_t sys_audio_path;

static uint32_t get_sampling_rate(sys_audio_device_t *dev);

bool sys_audio_mgr_default_pdm_data_init(sys_audio_device_t *dev_id, bool stereo, SYS_AUDIO_MGR_MODE mode)
{
        memset(dev_id, 0, sizeof(sys_audio_device_t));

        dev_id->device_type = AUDIO_PDM;
        dev_id->pdm_param.mode = mode;
        dev_id->pdm_param.clk_frequency = APU_DEFAULT_PDM_FREQUENCY;
        dev_id->pdm_param.channel = (stereo ? HW_PDM_CHANNEL_LR : HW_PDM_CHANNEL_R);
        dev_id->pdm_param.in_delay = HW_PDM_DI_NO_DELAY;
        dev_id->pdm_param.out_delay = HW_PDM_DO_NO_DELAY;

        return true;
}

bool sys_audio_mgr_default_pcm_data_init(sys_audio_device_t *dev_id, uint8_t channel_total,
                                              SYS_AUDIO_MGR_MODE mode, SYS_AUDIO_MGR_PCM_FORMATS format)
{
        OS_ASSERT(channel_total <= 8);

        memset(dev_id, 0, sizeof(sys_audio_device_t));

        dev_id->device_type = AUDIO_PCM;
        dev_id->pcm_param.mode = mode;
        dev_id->pcm_param.format = format;
        dev_id->pcm_param.clock = HW_PCM_CLK_DIVN;
        dev_id->pcm_param.sample_rate = APU_DEFAULT_SAMPLING_RATE;

        dev_id->pcm_param.total_channel_num = channel_total;
        dev_id->pcm_param.channel_delay = 0;
        dev_id->pcm_param.bits_depth = APU_DEFAULT_BITS_DEPTH;
        dev_id->pcm_param.enable_dithering = true;
        dev_id->pcm_param.output_mode = APU_DEFAULT_OUTPUT_MODE;
        dev_id->pcm_param.cycle_per_bit = APU_DEFAULT_PCM_CYCLE_PER_BIT;

        /* Note: ignored, except for format==PCM_MODE */
        dev_id->pcm_param.fsc_delay = APU_DEFAULT_FSC_DELAY;

        dev_id->pcm_param.inverted_fsc_polarity = false;
        dev_id->pcm_param.inverted_clk_polarity = true;
        /* Use default fsc_length, remember that fsc length time need to lower than fsc_div time */
        if (channel_total > 1) {
                dev_id->pcm_param.fsc_length = dev_id->pcm_param.total_channel_num / 2;
        } else {
                dev_id->pcm_param.fsc_length = 0;
        }

        return true;
}

bool sys_audio_mgr_default_memory_data_init(sys_audio_device_t *dev_id, bool stereo,
                                    uint32_t buff_len, uint32_t buffer_len_cb,
                                                         sys_audio_mgr_buffer_ready_cb cb, void *ud)
{
        OS_ASSERT(buff_len);
        OS_ASSERT(buffer_len_cb);
        OS_ASSERT(buff_len >= buffer_len_cb);
        OS_ASSERT((buff_len % buffer_len_cb) == 0); //If multi-buffered/circular operation, disallow wrapping

        memset(dev_id, 0, sizeof(sys_audio_device_t));

        dev_id->device_type = AUDIO_MEMORY;
        dev_id->memory_param.dma_channel[0] = HW_DMA_CHANNEL_INVALID;
        dev_id->memory_param.dma_channel[1] = HW_DMA_CHANNEL_INVALID;
        dev_id->memory_param.bits_depth = APU_DEFAULT_BITS_DEPTH;

        dev_id->memory_param.buff_addr[0] = 0;
        dev_id->memory_param.buff_addr[1] = 0;

        dev_id->memory_param.total_buffer_len = buff_len;
        dev_id->memory_param.buffer_len_cb = buffer_len_cb;

        dev_id->memory_param.cb = cb;
        dev_id->memory_param.app_ud = ud;
        dev_id->memory_param.stereo = stereo;
        dev_id->memory_param.sample_rate = APU_DEFAULT_SAMPLING_RATE;

        internal_data[0] = 0;
        internal_data[1] = 0;

        return true;
}

static bool is_src_conversion_required(sys_audio_path_t* path)
{
        uint32_t sample_rate_in;
        uint32_t sample_rate_out;
        /* Return false when:
         * - Memory to PCM with the same sampling rate
         * - PCM to Memory with the same sampling rate */

        if ((path->dev_in->device_type == AUDIO_PDM) || (path->dev_out->device_type == AUDIO_PDM)) {
                return true;
        }

        if ((path->dev_in->device_type == AUDIO_PCM) && (path->dev_out->device_type == AUDIO_PCM)) {
                /* need SRC just to pass-through; PCM->RAM->PCM is separate use case */
                return true;
        }

        /* For test - enable all use case permutations for clarity */
        if (    ((path->dev_in->device_type  == AUDIO_MEMORY) && path->dev_in->memory_param.force_src_use) ||
                ((path->dev_out->device_type == AUDIO_MEMORY) && path->dev_out->memory_param.force_src_use))
        {
                return true;
        }

        sample_rate_in = get_sampling_rate(path->dev_in);
        sample_rate_out = get_sampling_rate(path->dev_out);

        if (sample_rate_in == sample_rate_out) {
                return false;
        }

        return true;
}

static void initialize_pdm_reg(sys_audio_path_t* path, sys_audio_pdm_specific_t *param,
                                                                SYS_AUDIO_MGR_DIRECTION direction)
{
        /* Over-sampling ratio should be at least 64 times the sampling rate to
         * avoid degradation of the audio quality */
        sys_audio_device_t *other_dev = ((direction == DIRECTION_INPUT) ? path->dev_out : path->dev_in);
        if (other_dev->device_type == AUDIO_MEMORY) {
                OS_ASSERT(param->clk_frequency >= (other_dev->memory_param.sample_rate*64));
        }
        if (other_dev->device_type == AUDIO_PCM) {
                if (other_dev->pcm_param.sample_rate < 48000) {
                        /* Maximum SRC bandwidth is 24KHz */
                        OS_ASSERT(param->clk_frequency >= (other_dev->pcm_param.sample_rate * 64));
                } else {
                        OS_ASSERT(param->clk_frequency >= (48000 * 64));
                }
        }

        hw_pdm_request_clk(param->clk_frequency);
        if (direction == DIRECTION_INPUT) {
                hw_pdm_set_input_delay(param->in_delay);
                hw_pdm_set_in_channel_swap(!param->swap_channel);
        } else {
                hw_pdm_set_output_delay(param->out_delay);
                hw_pdm_set_out_channel_swap(param->swap_channel);
        }

        hw_pdm_set_mode(param->mode);
}

__STATIC_INLINE uint16_t calculate_fsc_div(sys_audio_pcm_specific_t *param)
{
        return (param->bits_depth * param->total_channel_num);
}

static void update_pcm_params(sys_audio_pcm_specific_t *param)
{
        param->channel_delay = hw_pcm_get_channel_delay();
        param->cycle_per_bit = hw_pcm_get_clk_per_bit();
        param->fsc_delay = hw_pcm_get_fsc_delay();
        param->fsc_length = hw_pcm_get_fsc_length();
        param->inverted_clk_polarity = hw_pcm_get_clk_polarity();
        param->inverted_fsc_polarity = hw_pcm_get_fsc_polarity();
}

static void initialize_pcm_reg(sys_audio_pcm_specific_t *param, SYS_AUDIO_MGR_DIRECTION direction)
{
        hw_pcm_config_t config = {0};
        HW_PCM_CLK_GENERATION div = {0};

        config.gpio_output_mode = param->output_mode;

        if (param->mode == MODE_SLAVE) {
                config.pcm_mode = HW_PCM_MODE_SLAVE;
        } else {
                config.pcm_mode = HW_PCM_MODE_MASTER;
        }

        switch (param->format) {
        case PCM_MODE:
                /* Maximum 8 channels of 8bits are supported */
                OS_ASSERT(param->total_channel_num * param->bits_depth / 8 <= 64);

                config.config_mode = HW_PCM_CONFIG_GENERIC_PCM_MODE;
                config.pcm_param.cycle_per_bit = param->cycle_per_bit;

                config.pcm_param.channel_delay = param->channel_delay * param->bits_depth / 8;

                if (param->inverted_fsc_polarity) {
                        config.pcm_param.fsc_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.pcm_param.fsc_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }

                if (param->inverted_clk_polarity) {
                        config.pcm_param.clock_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.pcm_param.clock_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }
                config.pcm_param.fsc_delay = param->fsc_delay;
                config.pcm_param.fsc_div = calculate_fsc_div(param);
                config.pcm_param.fsc_length = param->fsc_length * param->bits_depth / 8;
                break;
        case I2S_MODE:
                OS_ASSERT(param->total_channel_num * param->bits_depth / 8 <= 64);
                ASSERT_WARNING(param->total_channel_num == 2 * param->bits_depth / 8 );

                config.config_mode = HW_PCM_CONFIG_I2S_MODE;
                config.i2s_param.cycle_per_bit = param->cycle_per_bit;
                config.i2s_param.fsc_length = param->bits_depth / 8;
                config.i2s_param.fsc_div = calculate_fsc_div(param);
                break;
        case IOM2_MODE:
                config.config_mode = HW_PCM_CONFIG_IOM_MODE;
                config.iom_param.fsc_div = param->bits_depth * 2;

                if (param->inverted_fsc_polarity) {
                        config.iom_param.fsc_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.iom_param.fsc_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }

                if (param->inverted_clk_polarity) {
                        config.iom_param.clock_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.iom_param.clock_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }
                break;
        case TDM_MODE:
                ASSERT_WARNING(param->fsc_length != 0);
                OS_ASSERT(param->total_channel_num * param->bits_depth <= 64);

                config.config_mode = HW_PCM_CONFIG_TDM_MODE;

                if (param->inverted_fsc_polarity) {
                        config.tdm_param.fsc_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.tdm_param.fsc_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }

                if (param->inverted_clk_polarity) {
                        config.tdm_param.clock_polarity = HW_PCM_CLK_POLARITY_INVERTED;
                } else {
                        config.tdm_param.clock_polarity = HW_PCM_CLK_POLARITY_NORMAL;
                }

                config.tdm_param.cycle_per_bit = param->cycle_per_bit;
                config.tdm_param.channel_delay = param->channel_delay * param->bits_depth / 8;
                config.tdm_param.fsc_length = param->fsc_length * param->bits_depth / 8;
                config.tdm_param.fsc_div = calculate_fsc_div(param);
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        /* When fsc_edge == HW_PCM_FSC_EDGE_RISING_AND_FALLING then the bits of the second
         * channel begin at the falling edge of FSC. In this case the FSC length must be
         * greater or equal to the bit_depth plus the channel offset.
         */

        OS_ASSERT(!(param->fsc_length != 0 && (param->fsc_length < (1 + param->channel_delay)) ));

        hw_pcm_init(&config);

        if (param->enable_dithering) {
                div = HW_PCM_CLK_GEN_FRACTIONAL;
        } else {
                div = HW_PCM_CLK_GEN_INTEGER_ONLY;
        }

        hw_pcm_init_clk_reg(param->clock, param->sample_rate/1000, param->bits_depth, param->total_channel_num,
                                                                                               div);
        /*
         *  Update pcm user's parameters as there are pcm formats (I2S, TDM etc.) which
         *  change some pcm parameters
         */
        update_pcm_params(param);
}

static void dma_transfer_cb(void *user_data, dma_size_t len)
{
        dma_user_data_t *dma_user_data = (dma_user_data_t *)user_data;
        uint32_t next_buff_len_pos;
        uint8_t bus_width;

        /* Calculate index for the range DMA may now be recording to or playing back */
        if (dma_user_data->buff_block.bits_depth > 16) {
                bus_width = 4;
        } else if (dma_user_data->buff_block.bits_depth > 8) {
                bus_width = 2;
        } else {
                bus_width = 1;
        }

        /* Calculate index for the range DMA may now be recording to or playing back in bytes*/
        next_buff_len_pos = dma_user_data->buff_block.buff_len_pos + dma_user_data->buff_block.buff_len_cb;

        dma_user_data->buff_block.buff_len_pos = next_buff_len_pos;

       if (hw_dma_is_channel_active(dma_user_data->dma_channel_number)) {
               uint32_t num_of_transfers = ((next_buff_len_pos +
                                            dma_user_data->buff_block.buff_len_cb) / bus_width) - 1;
               if (num_of_transfers > UINT16_MAX) {
                       num_of_transfers &= UINT16_MAX;
               }
               hw_dma_channel_update_int_ix(dma_user_data->dma_channel_number, num_of_transfers);
       } else if (next_buff_len_pos < dma_user_data->buff_block.buff_len_total) {
               uint32_t len = (dma_user_data->buff_block.buff_len_total - next_buff_len_pos) /
                               bus_width;
               uint16_t num_of_transfers = dma_user_data->buff_block.buff_len_cb / bus_width - 1;
               uint32_t address = dma_user_data->buff_block.address + next_buff_len_pos;
               if (len > UINT16_MAX + 1) {
                       len = UINT16_MAX + 1;
               }
               if (num_of_transfers > UINT16_MAX) {
                       num_of_transfers = UINT16_MAX;
               }
               if (dma_user_data->dma_channel_number % 2 == 0) {
                       hw_dma_channel_update_destination(dma_user_data->dma_channel_number,
                                                         (void *)address,
                                                         len,
                                                         dma_transfer_cb);
               } else {
                       hw_dma_channel_update_source(dma_user_data->dma_channel_number,
                                                    (void *)address,
                                                    len,
                                                    dma_transfer_cb);
               }
               hw_dma_channel_update_int_ix(dma_user_data->dma_channel_number, num_of_transfers);
               hw_dma_channel_enable(dma_user_data->dma_channel_number, HW_DMA_STATE_ENABLED);
       }

        /* We might not have a call-back - if using two audio paths (e.g. PDM->RAM and RAM->PCM)
         * only one path's call-back is required to do any post-capture/pre-playback processing.
         *
         * If doing processing in IRQ context, the app call-back must maintain a read/write index,
         * and use buff_block.buff_len_cb both to update it and specify how much data to process.
         * It should reset it's index to 0 when >= buff_block.buff_len_total.
         *
         * If passing responsibility to a task with notify, the task should maintain a read/write index
         * and compare buff_block.buff_len_pos to calculate quantity to process. This way, multiple
         * IRQs and delayed notification handling will lead to it processing the available range of
         * data, not just a single chunk. It must cope with buff_len_pos having wrapped through 0.
         */
        if (NULL != dma_user_data->cb) {
                dma_user_data->cb( &(dma_user_data->buff_block),
                                     dma_user_data->app_ud);
        }
}

static void initialize_dma_reg(sys_audio_path_t* path, sys_audio_memory_specific_t *param,
                                                                SYS_AUDIO_MGR_DIRECTION direction)
{
        dma_user_data_t *dma_user_data;
        DMA_setup channel_setup;

        /* Setup generic left/right channel data parameter */
        OS_ASSERT(param->bits_depth <= 32);/* Audio support up to two 32-bit channels */

        channel_setup.circular = HW_DMA_MODE_NORMAL;

        if (param->bits_depth > 16) {
                channel_setup.bus_width = HW_DMA_BW_WORD;
                channel_setup.length = param->total_buffer_len/channel_setup.bus_width;
                channel_setup.irq_nr_of_trans = param->buffer_len_cb/channel_setup.bus_width;
        } else if (param->bits_depth > 8) {
                channel_setup.bus_width = HW_DMA_BW_HALFWORD;
                channel_setup.length = param->total_buffer_len/channel_setup.bus_width;
                channel_setup.irq_nr_of_trans = param->buffer_len_cb/channel_setup.bus_width;
        } else {
                channel_setup.bus_width = HW_DMA_BW_BYTE;
                channel_setup.length = param->total_buffer_len;
                channel_setup.irq_nr_of_trans = param->buffer_len_cb;
        }

        if (channel_setup.length > UINT16_MAX + 1) {
                channel_setup.length = UINT16_MAX + 1;
        }

        if (channel_setup.irq_nr_of_trans > UINT16_MAX) {
                channel_setup.irq_nr_of_trans = UINT16_MAX;
        }

        channel_setup.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        channel_setup.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        channel_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        channel_setup.a_inc = (direction == DIRECTION_INPUT) ? HW_DMA_AINC_TRUE : HW_DMA_AINC_FALSE;
        channel_setup.b_inc = (direction == DIRECTION_INPUT) ? HW_DMA_BINC_FALSE : HW_DMA_BINC_TRUE;
        channel_setup.callback = dma_transfer_cb;

        OS_ASSERT(param->buffer_len_cb <= param->total_buffer_len);

        channel_setup.dma_prio = HW_DMA_PRIO_2;
        channel_setup.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        channel_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;

        if (is_src_conversion_required(path)) {
                channel_setup.dma_req_mux = HW_DMA_TRIG_SRC_RXTX;
        } else {
                channel_setup.dma_req_mux = HW_DMA_TRIG_PCM_RXTX;
        }

        /* Setup specific for channel data parameter */
        uint8_t idx = 0;

        /* support up to 2 channel */
         while (idx < 2) {
                if (param->dma_channel[idx] != HW_DMA_CHANNEL_INVALID) {

                        channel_setup.channel_number = param->dma_channel[idx];

                        /* Decide data in/out location */
                        if (direction == DIRECTION_INPUT) {

                                channel_setup.src_address = param->buff_addr[idx];
                                if (is_src_conversion_required(path)) {
                                                                        // Channels 1, 3, 5 or 7 must be used for SRC input
                                        OS_ASSERT(channel_setup.channel_number == HW_DMA_CHANNEL_1 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_3 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_5 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_7);

                                        if (channel_setup.bus_width == HW_DMA_BW_BYTE) {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->SRC1_IN1_REG) + 3) : (uint32_t)((uint32_t)&(APU->SRC1_IN2_REG) + 3);
                                        } else if (channel_setup.bus_width == HW_DMA_BW_HALFWORD) {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->SRC1_IN1_REG) + 2) : (uint32_t)((uint32_t)&(APU->SRC1_IN2_REG) + 2);
                                        } else {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)(&(APU->SRC1_IN1_REG) ) : (uint32_t)(&(APU->SRC1_IN2_REG) );
                                        }
                                } else {
                                        if (channel_setup.bus_width == HW_DMA_BW_BYTE) {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->PCM1_OUT1_REG) + 3) : (uint32_t)((uint32_t)&(APU->PCM1_OUT2_REG) + 3);
                                        } else if (channel_setup.bus_width == HW_DMA_BW_HALFWORD) {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->PCM1_OUT1_REG) + 2) : (uint32_t)((uint32_t)&(APU->PCM1_OUT2_REG) + 2);
                                        } else {
                                                channel_setup.dest_address = (idx == 0) ?
                                                        (uint32_t)(&(APU->PCM1_OUT1_REG) ) : (uint32_t)(&(APU->PCM1_OUT2_REG) );
                                        }
                                }
                        } else {

                                channel_setup.dest_address = param->buff_addr[idx];
                                if (is_src_conversion_required(path)) {
                                               // Channels 0, 2, 4 or 6 must be used for SRC output
                                        OS_ASSERT(channel_setup.channel_number == HW_DMA_CHANNEL_0 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_2 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_4 ||
                                                  channel_setup.channel_number == HW_DMA_CHANNEL_6);

                                        if (channel_setup.bus_width == HW_DMA_BW_BYTE) {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->SRC1_OUT1_REG) + 3) : (uint32_t)((uint32_t)&(APU->SRC1_OUT2_REG) + 3);
                                        } else if (channel_setup.bus_width == HW_DMA_BW_HALFWORD) {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->SRC1_OUT1_REG) + 2) : (uint32_t)((uint32_t)&(APU->SRC1_OUT2_REG) + 2);
                                        } else {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)(&APU->SRC1_OUT1_REG) : (uint32_t)(&APU->SRC1_OUT2_REG);
                                        }
                                } else {
                                        if (channel_setup.bus_width == HW_DMA_BW_BYTE) {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->PCM1_IN1_REG) + 3) : (uint32_t)((uint32_t)&(APU->PCM1_IN2_REG) + 3);
                                        } else if (channel_setup.bus_width == HW_DMA_BW_HALFWORD) {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)((uint32_t)&(APU->PCM1_IN1_REG) + 2) : (uint32_t)((uint32_t)&(APU->PCM1_IN2_REG) + 2);
                                        } else {
                                                channel_setup.src_address = (idx == 0) ?
                                                        (uint32_t)(&APU->PCM1_IN1_REG) : (uint32_t)(&APU->PCM1_IN2_REG);
                                        }
                                }
                        }

                        dma_user_data = OS_MALLOC(sizeof(dma_user_data_t));

                        dma_user_data->dma_channel_number = param->dma_channel[idx];
                        dma_user_data->buff_block.buff_len_total = param->total_buffer_len;
                        dma_user_data->buff_block.buff_len_pos = 0;
                        dma_user_data->buff_block.buff_len_cb =  param->buffer_len_cb;
                        dma_user_data->buff_block.address = param->buff_addr[idx];
                        dma_user_data->cb = param->cb;
                        dma_user_data->path = path;
                        dma_user_data->app_ud = param->app_ud;
                        dma_user_data->buff_block.channel_num = idx;
                        dma_user_data->buff_block.stereo = param->stereo;
                        dma_user_data->buff_block.bits_depth = param->bits_depth;

                        channel_setup.user_data = dma_user_data;
                        internal_data[idx] = dma_user_data;

                        hw_dma_channel_initialization(&channel_setup);
                }
                idx++;
        }
}

__STATIC_INLINE resource_mask_t dma_resource_mask(HW_DMA_CHANNEL num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0), RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2), RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4), RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6), RES_MASK(RES_ID_DMA_CH7)
        };

        return res_mask[num];
}

static void dma_resource_mng(bool acquire, sys_audio_device_t *dev_id)
{
        uint8_t idx = 0;

        while (idx < 2) {

                if (dev_id->memory_param.dma_channel[idx] != HW_DMA_CHANNEL_INVALID) {

                        if (acquire) {
                                resource_acquire(dma_resource_mask(dev_id->memory_param.dma_channel[idx]), RES_WAIT_FOREVER);

                        } else {
                                resource_release(dma_resource_mask(dev_id->memory_param.dma_channel[idx]));
                        }
                }
                idx++;
        }
}

static void initialize_reg(sys_audio_path_t* path, sys_audio_device_t *dev_id,
                                                                SYS_AUDIO_MGR_DIRECTION direction)
{
        switch (dev_id->device_type) {
        case AUDIO_PDM:
                initialize_pdm_reg(path, &(dev_id->pdm_param), direction);
                break;
        case AUDIO_PCM:
                initialize_pcm_reg(&(dev_id->pcm_param), direction);
                break;
        case AUDIO_MEMORY:
                dma_resource_mng(true, dev_id);
                initialize_dma_reg(path, &(dev_id->memory_param), direction);
                break;
        default :
                OS_ASSERT(0);
                break;
        }
}

static uint32_t get_sampling_rate(sys_audio_device_t *dev)
{
        uint32_t ret = 0;

        switch (dev->device_type) {
        case AUDIO_PDM:
                break;
        case AUDIO_PCM:
                ret = dev->pcm_param.sample_rate;
                break;
        case AUDIO_MEMORY:
                ret = dev->memory_param.sample_rate;
                break;
        default :
                OS_ASSERT(0);
                break;
        }

        return ret;
}

static void validate_config(sys_audio_device_t *dev_id_in, sys_audio_device_t *dev_id_out)
{
        uint8_t byte_depth;
        sys_audio_device_t *pcm_dev;
        sys_audio_device_t *mem_dev;

        if ( dev_id_in->device_type == AUDIO_PCM && dev_id_out->device_type == AUDIO_MEMORY) {
                pcm_dev = dev_id_in;
                mem_dev = dev_id_out;
        } else if (dev_id_in->device_type == AUDIO_MEMORY && dev_id_out->device_type == AUDIO_PCM) {
                pcm_dev = dev_id_out;
                mem_dev = dev_id_in;
        } else {
                return;
        }

        byte_depth = (pcm_dev->pcm_param.bits_depth / 8);

        switch (pcm_dev->pcm_param.format) {
        case PCM_MODE:
                /* PCM mode - mono */
               if ((pcm_dev->pcm_param.fsc_length == 0)) {
                       OS_ASSERT(!( (mem_dev->memory_param.stereo == false) &&
                               (pcm_dev->pcm_param.total_channel_num * byte_depth <
                                       byte_depth * (1 + pcm_dev->pcm_param.channel_delay))
                                       ));

                       /* PCM mode - stereo */
                       OS_ASSERT(!((mem_dev->memory_param.stereo == true) &&
                               (pcm_dev->pcm_param.total_channel_num * byte_depth <
                                       4 + byte_depth * (1 + pcm_dev->pcm_param.channel_delay))
                                       ));
               }

               break;
       case I2S_MODE:
               /* I2S mode */
               OS_ASSERT(!(mem_dev->memory_param.stereo == false));

               OS_ASSERT(!(pcm_dev->pcm_param.channel_delay > 0));

               OS_ASSERT(pcm_dev->pcm_param.total_channel_num <= 8);
               break;
       case TDM_MODE:
               /* TDM mode */
               OS_ASSERT(!(mem_dev->memory_param.stereo == false));

               OS_ASSERT(!(pcm_dev->pcm_param.fsc_length == 0));

               OS_ASSERT(!((pcm_dev->pcm_param.fsc_length != 0) &&
                           (pcm_dev->memory_param.stereo == true) &&
                           (pcm_dev->pcm_param.total_channel_num * byte_depth <
                                   2 * byte_depth * (1 + pcm_dev->pcm_param.channel_delay))
                                   ));
               break;
       case IOM2_MODE:
               /* IOM mode */

               OS_ASSERT(!(pcm_dev->pcm_param.fsc_length != 0));

               OS_ASSERT(!(pcm_dev->pcm_param.channel_delay > 0));

               if (dev_id_in->device_type == AUDIO_PCM) {
                       OS_ASSERT(!((pcm_dev->pcm_param.format == IOM2_MODE) &&
                               pcm_dev->pcm_param.total_channel_num !=
                                       2 * (mem_dev->memory_param.stereo ? 2 : 1)
                                       ));
               } else {
                       OS_ASSERT(!(mem_dev->memory_param.stereo == true));
                       OS_ASSERT(!(pcm_dev->pcm_param.total_channel_num != 1));
               }
               break;
       default:
               break;
        }

        return;
}

sys_audio_path_t* sys_audio_mgr_open(sys_audio_device_t *dev_id_in, sys_audio_device_t *dev_id_out)
{
        sys_audio_path_t* path;

        if (!dev_id_in || !dev_id_out) {
                return NULL;
        }

        pm_sleep_mode_request(pm_mode_idle);

        OS_ASSERT(!((dev_id_in->device_type == AUDIO_INVALID_DEVICE) || (dev_id_out->device_type == AUDIO_INVALID_DEVICE)));
        OS_ASSERT(!((dev_id_in->device_type == dev_id_out->device_type) && (dev_id_out->device_type == AUDIO_PDM)));

        /* When fsc_edge == HW_PCM_FSC_EDGE_RISING and two channels are used then
         * the bits_depth must be 32bits because there is no way to define when the
         * bits of the first channel end and where the bits of the second channel begin.
         */
        validate_config(dev_id_in, dev_id_out);

        /* Acquire APU */
        resource_acquire(RES_MASK(RES_ID_APU), RES_WAIT_FOREVER);

        path = &sys_audio_path;

        path->dev_in = dev_id_in;
        path->dev_out = dev_id_out;

        initialize_reg(path, dev_id_in, DIRECTION_INPUT);
        initialize_reg(path, dev_id_out, DIRECTION_OUTPUT);

        /*
        0 = SRC fifo is used to store samples from memory to SRC
        1 = SRC fifo is used to store sample from SRC to memory
        */

        if (is_src_conversion_required(path)) {

                /* Initialize src */
                /* Set src clk at kHz divide Hz by 1000 */
                uint16_t src_clk = APU_DEFAULT_SRC_CLK/1000;

                /* Interfaces with sample rate (PCM/MEMORY)
                 * initialize the fsc and iir setting in apu_src
                 */
                uint32_t in_sample_rate = get_sampling_rate(dev_id_in);
                uint32_t out_sample_rate = get_sampling_rate(dev_id_out);

                hw_apu_src_init(src_clk, in_sample_rate, out_sample_rate);

                /* Only in case that interface is memory it is used the manual mode */
                if ((dev_id_in->device_type != AUDIO_MEMORY)) {
                        hw_apu_src_set_automode(HW_APU_SRC_IN);
                } else {
                        hw_apu_src_set_manual_mode(HW_APU_SRC_IN);
                }

                if ((dev_id_out->device_type != AUDIO_MEMORY)) {
                        hw_apu_src_set_automode(HW_APU_SRC_OUT);
                } else {
                        hw_apu_src_set_manual_mode(HW_APU_SRC_OUT);
                }

                hw_apu_src_clear_flow_error();

                /* Select the input */
                hw_apu_src_select_input((HW_APU_SRC_SELECTION)(dev_id_in->device_type));

                /* Enable the SRC FIFO and set direction. FIFO cannot be enabled in stereo mode */
                if (dev_id_in->device_type == AUDIO_MEMORY && dev_id_in->memory_param.stereo == false) {
                        hw_apu_src_enable_fifo(HW_APU_SRC_IN);
                } else if (dev_id_out->device_type == AUDIO_MEMORY && dev_id_out->memory_param.stereo == false) {
                        hw_apu_src_enable_fifo(HW_APU_SRC_OUT);
                } else {
                        hw_apu_src_disable_fifo();
                }

                /* i. Select the output */
                switch (dev_id_out->device_type) {
                case AUDIO_PCM:
                        hw_pdm_set_output_channel_config(HW_PDM_CHANNEL_NONE);
                        hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_SRC_OUT);
                        break;
                case AUDIO_PDM:
                        hw_pdm_set_output_channel_config(dev_id_out->pdm_param.channel);
                        hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_OFF);
                        break;
                case AUDIO_MEMORY:
                        hw_pdm_set_output_channel_config(HW_PDM_CHANNEL_NONE);
                        hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_OFF);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }
        } else if (dev_id_out->device_type == AUDIO_PCM) {
                /* When SRC is disabled (i.e. input cannot be PDM, PCM->PCM also forces SRC, so source device must be memory),
                 * and data should be output to PCM, the PCM data will be presented to PCM_OUT_REG.
                 */
                hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_PCM_OUT_REG);
        }

        return path;
}

static bool close_audio_memory(sys_audio_memory_specific_t *memory)
{
        if (!memory) {
                return(false);
        }

        if (internal_data[0]) {
                OS_FREE(internal_data[0]);
        }
        if (internal_data[1]) {
                OS_FREE(internal_data[1]);
        }

        return(true);
}

static bool audio_mgr_close_device(sys_audio_device_t *dev)
{
        if (!dev) {
                return (false);
        }

        if (dev->device_type == AUDIO_MEMORY) {
                return(close_audio_memory(&dev->memory_param));
        }

        return(true);
}

bool sys_audio_mgr_close(sys_audio_path_t* path)
{
        bool ret;

        ret = audio_mgr_close_device(path->dev_in);

        if (ret) {
                ret = audio_mgr_close_device(path->dev_out);
        }

        /* Release APU */
        resource_release(RES_MASK(RES_ID_APU));

        pm_sleep_mode_release(pm_mode_idle);

        return(ret);
}

/**
 * \brief Start audio device input or output - helper function
 *
 * \param [in] dev input or output device
 * \return
 *         \li true in case of closing audio manager with success
 *         \li false in case of closing audio manager failed
 *
 */
static bool start_device(sys_audio_device_t *dev)
{
        bool ret = true;
        uint8_t i;

        switch (dev->device_type) {
        case AUDIO_PDM:
                if (dev->pdm_param.mode == MODE_MASTER) {
                        hw_pdm_enable();
                        ret = hw_pdm_get_status();
                }
                break;
        case AUDIO_PCM:
                hw_pcm_enable();
                ret = hw_pcm_is_enabled();
                break;
        case AUDIO_MEMORY:
                for (i = 0; i < 2; i++) {
                        if (dev->memory_param.dma_channel[i] != HW_DMA_CHANNEL_INVALID) {
                                hw_dma_channel_enable(dev->memory_param.dma_channel[i],
                                        HW_DMA_STATE_ENABLED);
                        }
                }
                break;
        default :
                OS_ASSERT(0);
                break;
        }

        return(ret);
}

static bool  stop_device(sys_audio_path_t* path, sys_audio_device_t *dev,  SYS_AUDIO_MGR_DIRECTION direction)
{
        bool ret = false;
        uint8_t i;

        switch (dev->device_type) {
        case AUDIO_PDM:
                hw_pdm_disable();

                if (dev->pdm_param.mode == MODE_MASTER) {
                        ret = hw_pdm_get_status();
                }
                break;
        case AUDIO_PCM:
                hw_pcm_disable();
                ret = hw_pcm_is_enabled();
                break;
        case AUDIO_MEMORY:
                for ( i = 0; i < 2; i++) {
                        if (dev->memory_param.dma_channel[i] != HW_DMA_CHANNEL_INVALID) {
                                hw_dma_channel_enable(dev->memory_param.dma_channel[i],
                                        HW_DMA_STATE_DISABLED);
                        }
                }
                dma_resource_mng(false, dev);
                break;
        case AUDIO_INVALID_DEVICE:
                break;
        default :
                OS_ASSERT(0);
                break;
        }

        return(!ret);
}


bool sys_audio_mgr_start(sys_audio_path_t* path)
{
        bool ret;

        if (!path) {
                return (false);
        }

        ret = start_device(path->dev_in);

        if (ret) {
                ret = start_device(path->dev_out);
        }

        if (ret) {
                if (is_src_conversion_required(path)) {
                        hw_apu_src_enable();
                        ret = hw_apu_src_get_status();
                }
        }

        return(ret);

}

bool sys_audio_mgr_stop(sys_audio_path_t* path)
{
        bool ret;

        if (!path) {
                return (false);
        }

        ret = stop_device(path, path->dev_in, DIRECTION_INPUT);

        if (ret) {
                if (is_src_conversion_required(path)) {
                        hw_apu_src_disable();
                        ret = !hw_apu_src_get_status();
                }
        }

        if (ret) {
                ret = stop_device(path, path->dev_out, DIRECTION_OUTPUT);
        }

        return(ret);
}

#endif /* #if dg_configSYS_AUDIO_MGR */
