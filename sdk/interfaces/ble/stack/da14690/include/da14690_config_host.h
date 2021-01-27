/*
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

//CFG_NVDS CFG_EMB CFG_HOST CFG_APP  CFG_GTL CFG_BLE nCFG_HCI_UART nCFG_EXT_DB  nCFG_DBG_MEM nCFG_DBG_FLASH  nCFG_DBG_NVDS  nCFG_DBG_STACK_PROF  CFG_RF_RIPPLE nCFG_PERIPHERAL  CFG_ALLROLES  CFG_CON=6  CFG_SECURITY_ON  CFG_ATTC CFG_ATTS  nCFG_PRF_PXPM nCFG_PRF_PXPR nCFG_PRF_FMPL nCFG_PRF_FMPT nCFG_PRF_HTPT  nCFG_PRF_HTPC  nCFG_PRF_DISC nCFG_PRF_DISS  nCFG_PRF_BLPC  nCFG_PRF_BLPS  nCFG_PRF_HRPC nCFG_PRF_HRPS  CFG_PRF_ACCEL  POWER_OFF_SLEEP  RIPPLE_ID=16 CFG_BLECORE_11 CFG_SLEEP RADIO_580 CFG_DEEP_SLEEP  __NO_EMBEDDED_ASM nCFG_APP_SEC


#define F_2M
#define F_CH2
#define F_PCLE
#define CFG_NVDS
#define nCFG_EMB
#define CFG_HOST
#define nCFG_APP
#define CFG_GTL
#define CFG_BLE
#define CFG_HCI_UART
#define CFG_EXT_DB
#define nCFG_DBG_MEM
#define CFG_DBG_FLASH
#define nCFG_DBG_NVDS
#define nCFG_DBG_STACK_PROF
#define CFG_RF_RIPPLE
#define nCFG_PERIPHERAL
#define CFG_ALLROLES            1
#if ALTER_DEV >= 0x20
#define CFG_CON                 2
#else
#define CFG_CON                 8
#endif
#define CFG_SECURITY_ON         1
#define CFG_ATTC
#define CFG_ATTS

#define nCFG_PRF
#define nCFG_NB_PRF             10

#define CFG_DBG
#define DA_DBG_TASK

//#define CFG_WLAN_COEX
//#define CFG_WLAN_COEX_TEST
//#define CFG_BLE_TESTER

#define CFG_H4TL                1


//#define CFG_PRF_ACCEL           1
#define POWER_OFF_SLEEP
#ifndef ALTER_DEV
#define RIPPLE_ID               49
#else
#define RIPPLE_ID               19
#endif
#define CFG_BLECORE_11
#define CFG_SLEEP

#if ALTER_DEV >= 0x20
#define RADIO_680               1
#else
#define RADIO_RIPPLE            1
#endif

#define CFG_DEEP_SLEEP
#define __NO_EMBEDDED_ASM
#define nCFG_APP_SEC
#define CFG_CHNL_ASSESS
#define CFG_DBG_NVDS
#define CFG_DBG_MEM

#define BLE_CONNECTION_MAX_USER CFG_CON

#define nCFG_PRF_BASC           1
#define nCFG_PRF_BASS           1
#define nCFG_PRF_DISC           1
#define nCFG_PRF_DISS           1

#define nCFG_PRF_ANPC           1
#define nCFG_PRF_ANPS           1
#define nCFG_PRF_BLPC           1
#define nCFG_PRF_BLPS           1
#define nCFG_PRF_CSCPC          1
#define nCFG_PRF_CSCPS          1
#define nCFG_PRF_FMPL           1
#define nCFG_PRF_FMPT           1
#define nCFG_PRF_GLPC           1
#define nCFG_PRF_GLPS           1
#define nCFG_PRF_HOGPD          1
#define nCFG_PRF_HOGPBH         1
#define nCFG_PRF_HOGPRH         1
#define nCFG_PRF_HRPC           1
#define nCFG_PRF_HRPS           1
#define nCFG_PRF_HTPT           1
#define nCFG_PRF_HTPC           1
#define nCFG_PRF_PASPC          1
#define nCFG_PRF_PASPS          1
#define nCFG_PRF_PXPM           1
#define nCFG_PRF_PXPR           1
#define nCFG_PRF_RSCPC          1
#define nCFG_PRF_RSCPS          1
#define nCFG_PRF_SCPPC          1
#define nCFG_PRF_SCPPS          1
#define nCFG_PRF_TIPC           1
#define nCFG_PRF_TIPS           1

#define nCFG_PRF_CPPC           1
#define nCFG_PRF_CPPS           1

#define nSEC_TEST               1
#define DA14690                 1

#define uECC_OPTIMIZATION_LEVEL 1
#define default_RNG_defined     1
#define uECC_ENABLE_VLI_API     1
//#define RAM_BUILD
