/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_AUDIO_MANAGER Audio Manager Service
 *
 * \brief Audio Manager Service
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_audio_mgr.h
 *
 * @brief Audio manager API
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_AUDIO_MGR_H_
#define SYS_AUDIO_MGR_H_

#include <stdbool.h>
#include <string.h>
#include "hw_dma.h"
#include "hw_pdm.h"
#include "hw_pcm.h"

#if dg_configSYS_AUDIO_MGR

/**
 \brief Audio unit manager input/output source/sink kind.
 */
typedef enum {
        AUDIO_PCM,             /**< In/out PCM */
        AUDIO_PDM,             /**< In/out PDM */
        AUDIO_MEMORY,          /**< In/out memory (DMA) data */
        AUDIO_INVALID_DEVICE,  /**< Invalid device */
} SYS_AUDIO_MGR_DEVICE;

/**
 \brief Interface mode.
 */
typedef enum {
        MODE_SLAVE = 0,        /**< Interface in slave mode, i.e. clocked externally. */
        MODE_MASTER = 1,       /**< Interface in master mode, i.e. it provides the clock signal. */
} SYS_AUDIO_MGR_MODE;

/* PDM specific configuration */
typedef struct {
        SYS_AUDIO_MGR_MODE mode;         /**< The mode of the interface. Master or slave. */

        uint32_t clk_frequency;          /**< PDM_CLK frequency 62.5 kHz - 4 MHz. It should be noted
                                              that the audio quality degrades when the oversampling
                                              ratio is less than 64. For an 8 kHz sample rate
                                              the minimum recommended PDM clock rate is
                                              64 x 8 kHz = 512 kHz. */

        HW_PDM_CHANNEL_CONFIG channel;   /**< Programmable Left/Right channel for output only */

        HW_PDM_DI_DELAY in_delay;        /**< PDM input delay */

        HW_PDM_DO_DELAY out_delay;       /**< PDM output delay */

        bool swap_channel;               /**< PDM  swap channel this param is important only when
                                              2 channel are on the PDM bus*/
} sys_audio_pdm_specific_t;

typedef enum {
        PCM_MODE,
        I2S_MODE,
        IOM2_MODE,
        TDM_MODE
} SYS_AUDIO_MGR_PCM_FORMATS;

typedef struct {
        SYS_AUDIO_MGR_MODE mode;                          /**< The mode of the interface. Master or slave. */

        SYS_AUDIO_MGR_PCM_FORMATS format;                /**< Interface PCM formats I2S_MODE or PCM_MODE */

        HW_PCM_CLOCK clock;                               /**< Interface clock - DIVN=32MHz, DIV1=sys_clk (only
                                                                 if using 96MHz PLL) */

        uint32_t sample_rate;                             /**< The sample rate of the sample rate converter (Hz).
                                                          The PCM controller is implementing an up-to 192 kHz
                                                          synchronous interface to external audio devices */

        uint8_t channel_delay;                            /**< Select channel delay in range 0-7 */

        uint8_t total_channel_num;                        /**< The total channel number */

        HW_PCM_DO_OUTPUT_MODE output_mode;                /**< PCM DO output mode */

        uint8_t bits_depth;                               /**< The number of bits per channel */

        bool enable_dithering;                            /**< This is used to enable the dithering feature of
                                                                 the sample rate converter. */

        HW_PCM_FSC_DELAY fsc_delay;                       /**< PCM FSC starts one cycle before MSB bit otherwise
                                                                at the same time as MSB bit */

        bool inverted_fsc_polarity;                       /**< The polarity of the fsc signal can be inverted with */

        bool inverted_clk_polarity;                       /**< The polarity of the clk signal can be inverted with */

        HW_PCM_CYCLE_PER_BIT cycle_per_bit;               /**< PCM clock cycles per bit */

        uint8_t fsc_length;                               /**< PCM FSC length (in channels) -> Use only at PCM_MODE,
                                                              other modes calculate this automatically*/
} sys_audio_pcm_specific_t;

typedef struct {
        uint32_t address;
        uint32_t buff_len_total;  /**< The total buffer size in bytes, comprising one/multiple equal size chunks */
        uint32_t buff_len_cb;     /**< Size in bytes of a buffer chunk filled/consumed before each call-back */
        uint32_t buff_len_pos;    /**< At call-back, has start of current chunk being recorded/played */
        uint8_t  channel_num;     /**< For DMA IRQ to determine when all channels are processed and
                                       issue a single application IRQ to process all buffers together */
        uint8_t bits_depth;        /**< The used bits_depth */

        bool     stereo;          /**< Tells DMA IRQ if it actually needs to wait for another channel */
} sys_audio_mgr_buffer_data_block_t;

/**
 * \brief Asynchronous callback function. Execute when new audio data available.
 *
 * \param[in] buff_data_block pointer to the audio buffer data block
 * \param[in] app_ud Application user data
 *
 */
typedef void (*sys_audio_mgr_buffer_ready_cb)(sys_audio_mgr_buffer_data_block_t *buff_data_block, void *app_ud);

typedef struct {
        bool force_src_use;                 /**< For testing full range of possibilities, instead of having
                                                 to contrive different in/out sample rates */

        HW_DMA_CHANNEL dma_channel[2];      /**< DMA channel. tab[0] - left channel,
                                                              tab[1] - right channel*/

        uint32_t buff_addr[2];              /**< Data input or output address.
                                                 tab[0] - left channel, tab[1] - right channel*/

        uint32_t total_buffer_len;          /**< Total buffer length in bytes available per channel */

        uint32_t buffer_len_cb;             /**< Number of bytes dma required to execute
                                                 callback routine */

        sys_audio_mgr_buffer_ready_cb cb;   /**< Data buffer package ready callback */

        void *app_ud;                       /**< Application user data that will be passed to callback */

        uint32_t sample_rate;               /**< The sample rate of the sample rate converter (Hz).
                                            The PCM controller is implementing an up-to 192 kHz
                                            synchronous interface to external audio devices */

        bool stereo;                        /**< Selects dual channel operation */

        uint8_t bits_depth;                 /**< The number of bits per channel at a sample */

} sys_audio_memory_specific_t;

typedef struct {
        SYS_AUDIO_MGR_DEVICE device_type; /**< The kind of data device to be used as input or output. */

        union {
                sys_audio_pdm_specific_t pdm_param;
                sys_audio_pcm_specific_t pcm_param;
                sys_audio_memory_specific_t memory_param;
        };
} sys_audio_device_t;

typedef struct {
        sys_audio_device_t *dev_in;      /**< Input device */
        sys_audio_device_t *dev_out;     /**< Output device */
} sys_audio_path_t;

/**
 * \brief Initialize PDM device data structure - it's basic data initialization helper function
 *
 * \param[in,out] dev_id identifier of device that need to be configured
 * \param[in] stereo Select support dual channel operation
 * \param[in] mode the mode of the interface. Master or slave
 *
 * \returns true if the operation successfully, else false
 */
bool sys_audio_mgr_default_pdm_data_init(sys_audio_device_t *dev_id, bool stereo, SYS_AUDIO_MGR_MODE mode);

/**
 * \brief Initialize PCM device data structure - it's basic data initialization helper function
 *
 * \param[in,out] dev_id identifier of device that need to be configured
 * \param[in] channel_total Number of channel supported at in/out interface (left, right).
 * \param[in] mode the mode of the interface. Master or slave
 * \param[in] format PCM interface formats
 *
 * \returns true if the operation successfully, else false
 */
bool sys_audio_mgr_default_pcm_data_init(sys_audio_device_t *dev_id, uint8_t channel_total,
                                 SYS_AUDIO_MGR_MODE mode, SYS_AUDIO_MGR_PCM_FORMATS format);
/**
 * \brief Initialize memory device data structure - it's basic data initialization helper function
 *
 * \param[in,out] dev_id Identifier of device that need to be configured
 * \param[in] stereo Select support for dual buffer/channel operations
 * \param[in] buff_len Number of bytes for data buffer
 * \param[in] buffer_len_cb Data length required to run application callback function
 * \param[in] cb Application callback routine execute when new buffer_len_cb data size arrive
 * \param[in] ud User data pointer
 *
 * \returns true if the operation successfully, else false
 */
bool sys_audio_mgr_default_memory_data_init(sys_audio_device_t *dev_id, bool stereo,
                                    uint32_t buff_len, uint32_t buffer_len_cb,
                                    sys_audio_mgr_buffer_ready_cb cb, void *ud);

/**
 * \brief Open device input to output path - initialize proper devices
 *
 * \param[in] dev_id_in data structure of device connected to audio input bus
 *
 * \param[in] dev_id_out data structure of device connected to audio output bus
 *
 *
 * \return The handle to the audio path that can be used with other functions
 */
sys_audio_path_t* sys_audio_mgr_open(sys_audio_device_t *dev_id_in, sys_audio_device_t *dev_id_out);

/**
 * \brief Close audio path
 *
 * \param[in] path The handle to the audio path
 *
 * \sa sys_audio_mgr_open()
 * \return
 *         \li true in case of closing audio manager with success
 *         \li false in case of closing audio manager failed
 */
bool sys_audio_mgr_close(sys_audio_path_t* path);

/**
 * \brief Audio start function for a path that was previously configured.
 *
 * \param[in] path The handle to the audio path
 * \return
 *           \li true in case of closing audio manager with success
 *           \li false in case of closing audio manager failed
 */
bool sys_audio_mgr_start(sys_audio_path_t* path);

/**
 * \brief Audio stop function.
 *
 * \param[in] path The handle to the audio path
 */
bool sys_audio_mgr_stop(sys_audio_path_t* path);

#endif /* dg_configSYS_AUDIO_MGR */
#endif /* SYS_AUDIO_MGR_H_ */

/**
 * \}
 * \}
 */
