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
File    : USB_MSD_MTP.h
Purpose : Public header of USB MSD/MTP combination feature.
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_MSD_MTP_H_          /* Avoid multiple inclusion */
#define USB_MSD_MTP_H_

#include "../src/USB_Private.h"
#if USB_SUPPORT_MSD_MTP_COMBINATION > 0
#include <stdio.h>
#include "USB_SEGGER.h"
#include "USB_MSD_Private.h"
#include "USB_MTP_Private.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif


/*********************************************************************
*
*       defines, non-configurable
*
**********************************************************************
*/
#define USBD_MSD_MTP_MODE_NOT_INITED  (0ul)
#define USBD_MSD_MTP_MODE_MSD         (1ul)
#define USBD_MSD_MTP_MODE_MTP         (2ul)

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

int   USBD_MSD_MTP_Add      (const USB_MSD_INIT_DATA * pMSDInitData, const USB_MTP_INIT_DATA * pMTPInitData);
int   USBD_MSD_MTP_GetMode  (void);
void  USBD_MSD_MTP_Task     (void);
#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif
#endif                 // USB_SUPPORT_MSD_MTP_COMBINATION > 0
#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/

