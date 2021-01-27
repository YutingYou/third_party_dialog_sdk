/**
 ****************************************************************************************
 *
 * @file waveform_memory.c
 *
 * @brief Definition of the haptic waveform memory data.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * Waveform memory data
 */
const char wm_data[] = {
        0x03,      // num_snp
        0x03,      // num_seq
        0x09, 0x0B, 0x0D,  // snippet end ptrs
        0x0F, 0x14, 0x1E,  // sequence end ptrs
        0xBC, 0x10,                    // snp_1
        0x1C, 0x10,                    // snp_2
        0x1C, 0x50,                    // snp_3
        0x31, 0x11,                    // seq_0
        0x1A, 0x12, 0x88, 0x13, 0x88,  // seq_1
        0x0A, 0xC8, 0x18, 0xA0, 0x0A, 0xD8, 0x0A, 0xD8, 0x18, 0xA0,  // seq_2
};
