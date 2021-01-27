/**
 ****************************************************************************************
 *
 * @file audio_task.h
 *
 * @brief Audio task
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef INCLUDES_AUDIO_TASK_H_
#define INCLUDES_AUDIO_TASK_H_

#include "osal.h"
#include "sys_audio_mgr.h"

#define DEMO_PDM_MIC                    (0)
#define DEMO_PDM_RECORD_PLAYBACK        (0)
#define DEMO_PCM_MIC                    (0)
#define DEMO_PCM_RECORD_PLAYBACK        (1)

#if (DEMO_PDM_MIC + DEMO_PDM_RECORD_PLAYBACK + DEMO_PCM_MIC + DEMO_PCM_RECORD_PLAYBACK > 1)
#error "Only one audio demo should be selected!!!"
#endif

#if DEMO_PDM_MIC || DEMO_PDM_RECORD_PLAYBACK
#define PDM_FREQ_2MHZ    (2000000)       /* Frequency of PDM mic at 2MHz */
#endif

#define PCM_SAMPLE_RATE_16KHZ  (16000)   /* Sample rate of PCM at 16 Khz */
#define PCM_SAMPLE_RATE_8KHZ   (8000)    /* Sample rate of PCM at 8 Khz */
#define PCM_BIT_DEPTH_16      (16)      /* Bit depth of PCM at 16 */

#if DEMO_PDM_RECORD_PLAYBACK || DEMO_PCM_RECORD_PLAYBACK
#include <ad_nvms.h>

#define NOTIF_TASK_TEST_DONE          (1 << 1)
#define DEMO_CHANNEL_DATA_BUF_BASIC_SIZE (1024)
#define DEMO_CHANNEL_DATA_BUF_TOTAL_SIZE (DEMO_CHANNEL_DATA_BUF_BASIC_SIZE * 64 * 3)
#define DEMO_CHANNEL_DATA_BUF_CB_SIZE    (DEMO_CHANNEL_DATA_BUF_BASIC_SIZE * 64)
#endif

#define PRINTF_RECORDED_CHANNELS(ch) (ch == HW_PDM_CHANNEL_R ? "R" : \
                                       ch == HW_PDM_CHANNEL_L ? "L" : \
                                       ch == HW_PDM_CHANNEL_LR ? "L and R" : "none")

typedef enum {
        INPUT_DEVICE,
        OUTPUT_DEVICE
} device_direction_t;

typedef struct {
        void *audio_dev;
        OS_TASK audio_task;
#if DEMO_PDM_RECORD_PLAYBACK || DEMO_PCM_RECORD_PLAYBACK
        uint32_t available_to_read;
#endif
        sys_audio_device_t dev_in;
        sys_audio_device_t dev_out;
} context_demo_apu_t;

extern context_demo_apu_t context_demo_apu;

void printf_settings(sys_audio_device_t *dev, device_direction_t dir);
void audio_task(void *pvParameters);

#endif /* INCLUDES_AUDIO_TASK_H_ */
