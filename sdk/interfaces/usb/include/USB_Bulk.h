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
File    : USB_Bulk.h
Purpose : Public header of the bulk component
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef BULK_H          /* Avoid multiple inclusion */
#define BULK_H

#include "USB_SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, set default values
*
**********************************************************************
*/

#ifndef    USB_BULK_ALLOW_SETUP_REQUESTS
  #define USB_BULK_ALLOW_SETUP_REQUESTS  0
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int USB_BULK_HANDLE;

/*********************************************************************
*
*       USB_BULK_INIT_DATA
*
*  Function description
*    Initialization structure that is needed when adding a BULK interface to emUSB-Device.
*/
typedef struct {
  U8 EPIn;                       // Endpoint for sending data to the host.
  U8 EPOut;                      // Endpoint for receiving data from the host.
} USB_BULK_INIT_DATA;

/*********************************************************************
*
*       USB_BULK_INIT_DATA_EX
*
*  Function description
*    Initialization structure that is needed when adding a BULK interface to emUSB-Device.
*/
typedef struct {
  U16         Flags;             // Reserved for future use, must be 0.
  U8          EPIn;              // Endpoint for sending data to the host.
  U8          EPOut;             // Endpoint for receiving data from the host.
  const char *pInterfaceName;    // Name of the interface.
} USB_BULK_INIT_DATA_EX;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void            USBD_BULK_Init                 (void);
USB_BULK_HANDLE USBD_BULK_Add                  (const USB_BULK_INIT_DATA * pInitData);
USB_BULK_HANDLE USBD_BULK_Add_Ex               (const USB_BULK_INIT_DATA_EX * pInitData);
void            USBD_BULK_SetMSDescInfo        (USB_BULK_HANDLE hInst);
void            USBD_BULK_CancelRead           (USB_BULK_HANDLE hInst);
void            USBD_BULK_CancelWrite          (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesInBuffer  (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesRemToRead (USB_BULK_HANDLE hInst);
unsigned        USBD_BULK_GetNumBytesRemToWrite(USB_BULK_HANDLE hInst);
int             USBD_BULK_Read                 (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes, unsigned Timeout);
int             USBD_BULK_ReadOverlapped       (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes);
int             USBD_BULK_Receive              (USB_BULK_HANDLE hInst, void * pData, unsigned NumBytes, int Timeout);
void            USBD_BULK_SetOnRXHook          (USB_BULK_HANDLE hInst, USB_ON_RX_FUNC * pfOnRx);
void            USBD_BULK_SetOnTXEvent         (USB_BULK_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
void            USBD_BULK_SetOnRXEvent         (USB_BULK_HANDLE hInst, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
int             USBD_BULK_WaitForRX            (USB_BULK_HANDLE hInst, unsigned Timeout);
int             USBD_BULK_WaitForTX            (USB_BULK_HANDLE hInst, unsigned Timeout);
int             USBD_BULK_WaitForTXReady       (USB_BULK_HANDLE hInst, int Timeout);
int             USBD_BULK_Write                (USB_BULK_HANDLE hInst, const void * pData, unsigned NumBytes, int Timeout);
int             USBD_BULK_WriteEx              (USB_BULK_HANDLE hInst, const void * pData, unsigned NumBytes, char Send0PacketIfRequired, int Timeout);
void            USBD_BULK_SetContinuousReadMode(USB_BULK_HANDLE hInst);
int             USBD_BULK_TxIsPending          (USB_BULK_HANDLE hInst);
void            USBD_BULK_SetNumBytesWithout0Packet(U32 Bytes);
void            USBD_BULK_Stall                (USB_BULK_HANDLE hInst);
#if (USB_BULK_ALLOW_SETUP_REQUESTS == 1)
void            USBD_BULK_SetOnVendorRequest(USB_BULK_HANDLE hInst, USB_ON_CLASS_REQUEST * pfOnVendorRequest);
void            USBD_BULK_SetOnSetupRequest(USB_BULK_HANDLE hInst, USB_ON_SETUP * pfOnSetupRequest);
#endif

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define USB_BULK_Init                          USBD_BULK_Init
#define USB_BULK_Add(x)                        USBD_BULK_Add(x)
#define USB_BULK_CancelRead()                  USBD_BULK_CancelRead(0)
#define USB_BULK_CancelWrite()                 USBD_BULK_CancelWrite(0)
#define USB_BULK_GetNumBytesInBuffer()         USBD_BULK_GetNumBytesInBuffer(0)
#define USB_BULK_GetNumBytesRemToRead()        USBD_BULK_GetNumBytesRemToRead(0)
#define USB_BULK_GetNumBytesToWrite()          USBD_BULK_GetNumBytesToWrite(0)
#define USB_BULK_Read(p, n)                    USBD_BULK_Read(0, p, n, 0)
#define USB_BULK_ReadTimed(p, n, t)            USBD_BULK_Read(0, p, n, t)
#define USB_BULK_ReadOverlapped(p, n)          USBD_BULK_ReadOverlapped(0, p, n)
#define USB_BULK_Receive(p, n)                 USBD_BULK_Receive(0, p, n, 0)
#define USB_BULK_ReceiveTimed(p, n, t)         USBD_BULK_Receive(0, p, n, t)
#define USB_BULK_SetOnRXHook(x)                USBD_BULK_SetOnRXHook(0, x)
#define USB_BULK_WaitForRX()                   USBD_BULK_WaitForRX(0, 0)
#define USB_BULK_WaitForTX()                   USBD_BULK_WaitForTX(0, 0)
#define USB_BULK_Write(p, n)                   USBD_BULK_Write(0, p, n, 0)
#define USB_BULK_WriteEx(p, n, s)              USBD_BULK_WriteEx(0, p, n, s, 0)
#define USB_BULK_WriteExTimed(p, n, s, t)      USBD_BULK_WriteEx(0, p, n, s, t)
#define USB_BULK_WriteOverlapped(p, n)         USBD_BULK_Write(0, p, n, -1)
#define USB_BULK_WriteOverlappedEx(p, n, s)    USBD_BULK_WriteEx(0, p, n, s, -1)
#define USB_BULK_WriteTimed(p, n, t)           USBD_BULK_Write(0, p, n, t)
#define USB_BULK_WriteNULLPacket()             USBD_BULK_Write(0, NULL, 0, 0)
#define USB_BULK_StartReadTransfer()           USBD_BULK_Receive(0, NULL, 0, -1)
#define USB_BULK_TxIsPending()                 USBD_BULK_TxIsPending(0)

/*********************************************************************
*  End of Wrapper
**********************************************************************/

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
