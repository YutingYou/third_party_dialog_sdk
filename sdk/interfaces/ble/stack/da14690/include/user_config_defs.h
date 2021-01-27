/**
 ****************************************************************************************
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef USER_CONFIG_DEFS_H
#define USER_CONFIG_DEFS_H

#include <stdint.h>
#include <stdbool.h>


typedef struct cmac_config_ {
        bool     wait_on_main;                  // When this flag is set to true, CMAC will wait on main() entry until the flag becomes false.
        uint8_t  ble_bd_address[6];             // The BLE Device Address
        uint8_t  rf_calibration_delay;          // The maximum delay allowed for RF calibration (in multiples of 100 msec)
        uint8_t  lp_clock_freq;                 // LP clock type:
                                                //      0 = 32768Hz XTAL
                                                //      1 = 32000Hz XTAL
                                                //      2 = RCX
                                                // Default: 32768Hz
        uint16_t lp_clock_drift;                // Device SCA setting, Default: 500

        uint16_t ble_rx_buffer_size;            // BLE Rx data buffer size, Default: 262 bytes
        uint16_t ble_tx_buffer_size;            // BLE Tx data buffer size, Defalut: 262 bytes
        bool     ble_length_exchange_needed;    // Flag to control Length Exchange, Default: true
        uint16_t ble_chnl_assess_timer;         // Channel Assessment Timer duration (5s - 
                                                // Multiple of 10ms), Default: 500, Private
        uint8_t  ble_chnl_reassess_timer;       // Channel Reassessment Timer duration (Multiple of
                                                // Channel Assessment Timer duration), Default: 8, Private
        int8_t   ble_chnl_assess_min_rssi;      // BLE Chnl Assess alg, Min RSSI, Default: -60 (dBm)
        uint16_t ble_chnl_assess_nb_pkt;        // # of packets to receive for statistics, Default: 20, Private
        uint16_t ble_chnl_assess_nb_bad_pkt;    // # of bad packets needed to remove a channel, Default: 10, Private
        uint8_t  system_tcs_length;             // Number of valid entries in the table
        uint8_t  synth_tcs_length;              // Number of valid entries in the table
        uint8_t  rfcu_tcs_length;               // Number of valid entries in the table
        uint8_t  initial_tx_power_lvl;          // The initial TX power level index used in Advertising and Data channels
        uint8_t  ble_dup_filter_max;            // Maximum number of devices for the duplicate filtering list
        bool     ble_dup_filter_found;          // Unknown devices are treated as "found" (be in the 
                                                // duplicate filter buffer) when the buffer is full, 
                                                // if true, Default: true, Private
        bool     use_high_performance_1m;       // Enable 1M High Performance mode
        bool     use_high_performance_2m;       // Enable 2M High Performance mode

        int8_t   golden_range_low;              // RSSI "Golden Range" lower value (dBm)
        int8_t   golden_range_up;               // RSSI "Golden Range" upper value (dBm)
        int8_t   golden_range_pref;             // Preferred RSSI value inside "Golden Range" (dBm)
        uint8_t  pcle_min_tx_pwr_idx;           // Min Tx Power index used in PCLE feature
        uint8_t  pcle_max_tx_pwr_idx;           // Max Tx Power index used in PCLE feature

} cmac_configuration_table_t;

extern volatile cmac_configuration_table_t cmac_config_table;

typedef enum {
    CMAC_STATE_DISABLED,        // Not yet started
    CMAC_STATE_DEEPSLEEPING,    // Deep sleeping or entering deep sleep
    CMAC_STATE_AWAKE,           // Awake
} CMAC_STATE;

typedef struct cmac_dynamic_config_ {
        bool     sleep_enable;                  // Flag to control sleep, Default: false
        bool     ble_host_irq_after_event;      // Flag to control IRQ after BLE event, Default: false
        bool     ble_skip_conn_latency;         // Flag to control Conn Latency, Default: false
        uint8_t  bw_reservation_slave;          // Number of slots reserved for a slave conn event
                                                // Minimum allowed value is 3 slots. Warning: no
                                                // checks are performed! Setting a value lower than
                                                // 3 will result in unpredictable behavior.
        uint32_t rcx_period;                    // RCX period in usec as a 12.20 fixed point number.
        uint32_t rcx_clock_hz_acc;              // RCX frequency in Hz as a 29.3 fixed point number.
        uint16_t wakeup_time;                   // The total wake up time in LP clock cycles including
                                                // the HW FSM wake-up time plus
                                                // the XTAL32M settling time.
        bool        first_rfcu_enable;
        uint8_t     pwr_level;
        uint8_t     femonly_fine_atten;
        uint8_t     femonly_fine_atten_disabled;
        uint8_t     coarse_atten;
        uint8_t     rfio_tx_dcf_val;
        uint8_t     rfio_rx_dcf_val;
        uint8_t     rfio_tx_dcf_pref_val;
        uint32_t    tx_0dbm_2ndharm_trim;
        uint32_t    tx_6dbm_2ndharm_trim;

        uint32_t    power_ctrl_reg_onwakeup_value;  // The value that should be applied in POWER_CTRL_REG on wakeup
        uint32_t    power_ctrl_reg_onsleep_value;   // The value that should be applied in POWER_CTRL_REG on sleep

        uint8_t     maccpu_state;                   // The current state of MAC CPU (type of CMAC_STATE)

        /*
         * ble_advertising_permutation: The permutation index to take effect next time advertising
         * begins.  Its value will be propagated to adv_perm_sel at the beginning of the next
         * advertising cycle, so as to not violate the standard by broadcasting more than one PDU in
         * each channel.
         */
        uint8_t     ble_advertising_permutation;

} cmac_dynamic_configuration_table_t;

extern volatile cmac_dynamic_configuration_table_t cmac_dynamic_config_table;

#if (CMAC_CPU)
#define cmac_config_table_ptr (&cmac_config_table)
#define cmac_dynamic_config_table_ptr (&cmac_dynamic_config_table)
#else
extern cmac_configuration_table_t *cmac_config_table_ptr;
extern cmac_dynamic_configuration_table_t *cmac_dynamic_config_table_ptr;
#endif

typedef struct cmac_info_ {
        uint32_t ble_conn_evt_counter[BLE_CONNECTION_MAX_USER];
        uint32_t ble_conn_evt_counter_non_apfm[BLE_CONNECTION_MAX_USER];

        uint32_t ble_adv_evt_counter;
        uint32_t ble_adv_evt_counter_non_apfm;
} cmac_info_table_t;

extern cmac_info_table_t cmac_info_table;

struct _tcs_entry {
        uint32_t *register_p;
        uint32_t value;
};

#if !(CMAC_CPU)
extern cmac_info_table_t *cmac_info_table_ptr;
extern struct _tcs_entry *cmac_sys_tcs_table_ptr;
extern struct _tcs_entry *cmac_synth_tcs_table_ptr;
extern struct _tcs_entry *cmac_rfcu_tcs_table_ptr;
extern void *cmac_nmi_context_ptr;
#endif

extern uint32_t rwip_heap_non_ret[];
extern uint32_t rwip_heap_msg_ret[];
extern uint32_t rwip_heap_db_ret[];
extern uint32_t rwip_heap_env_ret[];

extern const uint32_t rwip_heap_non_ret_size_jt;
extern const uint32_t rwip_heap_env_size_jt;
extern const uint32_t rwip_heap_db_size_jt;
extern const uint32_t rwip_heap_msg_size_jt;

extern const uint32_t user_llm_adv_interval_min;

#define SYSTEM_TCS_NO_OF_ENTRIES                (8)
#define SYNTH_TCS_NO_OF_ENTRIES                 (6)
#define RFCU_TCS_NO_OF_ENTRIES                  (8)


#define rom_cfg_table           cmac_config_table_ptr;

#include "co_version.h"

#endif //USER_CONFIG_DEFS_H
