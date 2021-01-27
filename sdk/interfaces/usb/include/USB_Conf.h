/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2018     SEGGER Microcontroller GmbH              *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device * USB Device stack for embedded applications    *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product for in-house use.         *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Device version: V3.12a                                 *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              Dialog Semiconductor BV, Het Zuiderkruis 53, 5215 MV S-Hertogenbosch, The Netherlands
Licensed SEGGER software: emUSB-Device
License number:           USBD-00327
License model:            Buyout SRC [Buyout Source Code License], signed on 8th August, 2016 and Amendment No. 1, signed on 26th September, 2017
Licensed product:         Any
Licensed platform:        D2320, D2522
Licensed number of seats: -
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2016-08-10 - 2018-12-31
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File    : USB_Conf.h
Purpose : Config file. Modify to reflect your configuration
--------  END-OF-HEADER  ---------------------------------------------
*/


#ifndef USB_CONF_H           /* Avoid multiple inclusion */
#define USB_CONF_H

// #ifdef USB_IS_HIGH_SPEED
//   #define USB_SUPPORT_HIGH_SPEED           1
// #else
//   #define USB_SUPPORT_HIGH_SPEED           0
// #endif

#define USB_MSD_MAX_UNIT                  2

#ifdef DEBUG
  #if DEBUG
    #define USB_DEBUG_LEVEL        2   // Debug level: 1: Support "Panic" checks, 2: Support warn & log
  #endif
#endif

//
// Configure profiling support.
//
#if defined(SUPPORT_PROFILE) && (SUPPORT_PROFILE)
  #ifndef   USBD_SUPPORT_PROFILE
    #define USBD_SUPPORT_PROFILE           1                   // Define as 1 to enable profiling via SystemView.
  #endif
#endif

#define     USB_MAX_NUM_IAD        2

//
// IAR ARM compiler related macros
//
#ifdef __ICCARM__
  #if ((__TID__ >> 4) & 0x0F) < 6   // For any ARM CPU core < v7, we will use optimized routines
    #include "USB_SEGGER.h"
    #define USB_MEMCPY(pDest, pSrc, NumBytes) SEGGER_ARM_memcpy(pDest, pSrc, NumBytes)    // Speed optimization: Our memcpy is much faster!
  #else
//    #include "SEGGER.h"
//    #define USB_MEMCPY(pDest, pSrc, NumBytes) SEGGER_memcpy(pDest, pSrc, NumBytes)    // Speed optimization: Our memcpy is much faster!
  #endif
#endif

#define USB_V2_V3_MIGRATION_DEVINFO      0
#define USB_V2_V3_MIGRATION_CONFIG       0
#define USB_V2_V3_MIGRATION_MSD_LUN_INFO 0
#define USB_V2_V3_MIGRATION_MTP_INFO     0

#define USBD_OS_LAYER_EX                 1      // Use extended OS layer

#define USBD_USE_LEGACY_CACHE_ROUTINES   0

#endif     /* Avoid multiple inclusion */

/*************************** End of file ****************************/
