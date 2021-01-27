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
File    : USB.h
Purpose : USB stack API
          Do not modify to allow easy updates !
Literature:
  [1]  Universal Serial Bus Specification Revision 2.0
       \\fileserver\Techinfo\Subject\USB\USB_20\usb_20.pdf
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USB_H     /* Avoid multiple inclusion */
#define USB_H

#include "USB_SEGGER.h"
#include "USB_Conf.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/* USB system version */
#define USB_VERSION  31201UL // Format: Mmmrr Example: 31201UL is 3.12a


/*********************************************************************
*
*       Default values
*
*/
#ifndef   USB_SUPPORT_HIGH_SPEED
  #define USB_SUPPORT_HIGH_SPEED 1
#endif

#ifndef   USB_MAX_PACKET_SIZE
  #if USB_SUPPORT_HIGH_SPEED
    #define USB_MAX_PACKET_SIZE   512u
  #else
    #define USB_MAX_PACKET_SIZE    64u
  #endif
#endif

//
// Maximum packet size defines for respective transfer types.
//
#define USB_HS_BULK_MAX_PACKET_SIZE      512u
#define USB_FS_BULK_MAX_PACKET_SIZE       64u
#define USB_HS_ISO_MAX_PACKET_SIZE      1024u
#define USB_FS_ISO_MAX_PACKET_SIZE      1023u
#define USB_HS_CONTROL_MAX_PACKET_SIZE    64u
#define USB_FS_CONTROL_MAX_PACKET_SIZE    64u
//
// High-speed interrupt endpoints support 1024 bytes as per USB 2.0 spec.
// At the moment this is not supported by the stack, therefore this define is limited to 64.
//
#define USB_HS_INT_MAX_PACKET_SIZE        64u
#define USB_FS_INT_MAX_PACKET_SIZE        64u


#ifndef   USB_DEBUG_LEVEL
  #define USB_DEBUG_LEVEL  0
#endif

#ifndef    USB_DRIVER_OPTIMIZE
  #define  USB_DRIVER_OPTIMIZE
#endif

#define USB_STATUS_ERROR       -1
#define USB_STATUS_EP_HALTED   -3
#define USB_STATUS_EP_BUSY     -4
#define USB_STATUS_TRIAL       -255

#ifndef   USB_SUPPORT_CLASS_REQUESTS
  #define USB_SUPPORT_CLASS_REQUESTS 1
#endif

#ifndef   USB_SUPPORT_VENDOR_REQUESTS
  #define USB_SUPPORT_VENDOR_REQUESTS 1
#endif

#ifndef USB_SUPPORT_STATUS
  #define USB_SUPPORT_STATUS 1
#endif

#ifndef   USB_OTHER_SPEED_DESC
  #define USB_OTHER_SPEED_DESC         USB_SUPPORT_HIGH_SPEED
#endif

#ifndef   USB_SUPPORT_TEST_MODE
  #define USB_SUPPORT_TEST_MODE        0
#endif

#ifndef   USBD_SUPPORT_PROFILE
  #define USBD_SUPPORT_PROFILE  0
#endif

#ifndef     USB_SUPPORT_LOG
  #if       USB_DEBUG_LEVEL > 1
    #define USB_SUPPORT_LOG   1
  #else
    #define USB_SUPPORT_LOG   0
  #endif
#endif
#ifndef     USB_SUPPORT_WARN
  #if       USB_DEBUG_LEVEL > 1
    #define USB_SUPPORT_WARN  1
  #else
    #define USB_SUPPORT_WARN  0
  #endif
#endif

#ifndef USB_V2_V3_MIGRATION_DEVINFO
  #define USB_V2_V3_MIGRATION_DEVINFO   0
#endif
#ifndef USB_V2_V3_MIGRATION_CONFIG
  #define USB_V2_V3_MIGRATION_CONFIG    0
#endif

#ifndef USBD_OS_LAYER_EX
  #define USBD_OS_LAYER_EX       0
#endif

#ifndef USB_SUPPORT_MSD_MTP_COMBINATION
  #define USB_SUPPORT_MSD_MTP_COMBINATION 0
#endif

#ifndef USBD_USE_LEGACY_CACHE_ROUTINES
  #define USBD_USE_LEGACY_CACHE_ROUTINES 0
#endif


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#if USB_DEBUG_LEVEL
  #define USB_PANIC(ErrMsg)      USB_OS_Panic(ErrMsg)
#else
  #define USB_PANIC(ErrMsg)
#endif

#if USB_SUPPORT_LOG
  #define USB_LOG(p) USBD_Logf p
#else
  #define USB_LOG(p)
#endif

#if USB_SUPPORT_WARN
  #define USB_WARN(p) USBD_Warnf p
#else
  #define USB_WARN(p)
#endif

/*********************************************************************
*
*       Transfer types
*/
#define USB_TRANSFER_TYPE_CONTROL   0u     // Refer to [1]:9.6.6, table 9-13
#define USB_TRANSFER_TYPE_ISO       1u
#define USB_TRANSFER_TYPE_BULK      2u
#define USB_TRANSFER_TYPE_INT       3u

/*********************************************************************
*
*       Endpoint direction
*/
#define USB_DIR_IN  1
#define USB_DIR_OUT 0

/*********************************************************************
*
*       Status flags
*/
#define USB_STAT_ATTACHED   (1u << 4)
#define USB_STAT_READY      (1u << 3)      // Set by any bus reset. Is required to go from "Powered" to "Addressed"
#define USB_STAT_ADDRESSED  (1u << 2)
#define USB_STAT_CONFIGURED (1u << 1)
#define USB_STAT_SUSPENDED  (1u << 0)

/*********************************************************************
*
*       Endpoint read mode
*/
#define USB_READ_MODE_CONTINUOUS    (1u << 0)    // Always accept RX data independent of USB_Read...() calls,
                                                 // as long as there is enough free space in the buffer.
#define USB_READ_MODE_SINGLE_PACKET (1u << 1)    // Obsolete

/*********************************************************************
*
*       Events for callback functions
*/
#define USB_EVENT_DATA_READ         (1u << 0)
#define USB_EVENT_DATA_SEND         (1u << 1)
#define USB_EVENT_DATA_ACKED        (1u << 2)
#define USB_EVENT_READ_COMPLETE     (1u << 3)
#define USB_EVENT_READ_ABORT        (1u << 4)
#define USB_EVENT_WRITE_ABORT       (1u << 5)
#define USB_EVENT_WRITE_COMPLETE    (1u << 6)

/*********************************************************************
*
*       Message types
*
*  Description
*    The same message types are used for log and warning messages.
*    Separate filters can be used for both log and warnings.
*    For details, refer to USBD_SetLogFilter() and USBD_SetWarnFilter()
*    as wells as USBD_AddLogFilter() and USBD_AddWarnFilter() for more
*    information about using the message types.
*
*/
#define USB_MTYPE_INIT          (1UL <<  0)                 // Activates output of messages from the initialization of the stack that should be logged.
#define USB_MTYPE_CORE          (1UL <<  1)                 // Activates output of messages from the core of the stack that should be logged.
#define USB_MTYPE_CONFIG        (1UL <<  2)                 // Activates output of messages from the configuration of the stack.
#define USB_MTYPE_DRIVER        (1UL <<  3)                 // Activates output of messages from the driver that should be logged.
#define USB_MTYPE_ENUMERATION   (1UL <<  4)                 // Activates output of messages from enumeration that should be logged. Note: Since enumeration is handled in an ISR, use this with care as the timing will be changed greatly.
#define USB_MTYPE_CDC           (1UL <<  7)                 // Activates output of messages from CDC module that should be logged when a CDC connection is used.
#define USB_MTYPE_HID           (1UL <<  8)                 // Activates output of messages from HID module that should be logged when a HID connection is used.
#define USB_MTYPE_MSD           (1UL <<  9)                 // Activates output of messages from MSD module that should be logged when a MSD connection is used.
#define USB_MTYPE_MSD_CDROM     (1UL << 10)                 // Activates output of messages from MSD CDROM module that should be logged.
#define USB_MTYPE_MSD_PHY       (1UL << 11)                 // Activates output of messages from MSD Physical layer that should be logged.
#define USB_MTYPE_MTP           (1UL << 12)                 // Activates output of messages from MTP module that should be logged when a MTP connection is used.
#define USB_MTYPE_PRINTER       (1UL << 13)                 // Activates output of messages from Printer module that should be logged when Printer connection is used.
#define USB_MTYPE_RNDIS         (1UL << 14)                 // Activates output of messages from RNDIS module that should be logged when a RNIDS connection is used.
#define USB_MTYPE_SMART_MSD     (1UL << 16)                 // Activates output of messages from Smart-MSD module that should be logged when a SmartMSD connection is used.
#define USB_MTYPE_UVC           (1UL << 17)                 // Activates output of messages from UVC module that should be logged when a UVC connection is used.
#define USB_MTYPE_ECM           (1UL << 18)                 // Activates output of messages from ECM module that should be logged when a ECM connection is used.
#define USB_MTYPE_AUDIO         (1UL << 19)                 // Activates output of messages from Audio module that should be logged when an audio connection is used.


/*********************************************************************
*
*       MS OS relevant defines
*/
#define USB_MSOS_EXT_PROPTYPE_REG_NONE                        (0)   // No value type
#define USB_MSOS_EXT_PROPTYPE_REG_SZ                          (1)   // A NULL-terminated Unicode String (REG_SZ)
#define USB_MSOS_EXT_PROPTYPE_REG_EXPAND_SZ                   (2)   // A NULL-terminated Unicode String that includes environment variables (REG_EXPAND_SZ)

// (with environment variable references)
#define USB_MSOS_EXT_PROPTYPE_REG_BINARY                      (3)   // Free form binary
#define USB_MSOS_EXT_PROPTYPE_REG_DWORD                       (4)   // 32-bit number (LITTLE ENDIAN)
#define USB_MSOS_EXT_PROPTYPE_REG_DWORD_BIG_ENDIAN            (5)   // Leave Definition as defined by MS: 32-bit number (BIG ENDIAN)
#define USB_MSOS_EXT_PROPTYPE_REG_LINK                        (6)   // Symbolic Link (unicode)
#define USB_MSOS_EXT_PROPTYPE_REG_MULTI_SZ                    (7)   // Multiple Unicode strings
#define USB_MSOS_EXT_PROPTYPE_REG_RESOURCE_LIST               (8)   // Resource list in the resource map
#define USB_MSOS_EXT_PROPTYPE_REG_FULL_RESOURCE_DESCRIPTOR    (9)   // Resource list in the hardware description
#define USB_MSOS_EXT_PROPTYPE_REG_RESOURCE_REQUIREMENTS_LIST (10)   // Leave Definition as defined by MS: Microsoft resource requirement list.
#define USB_MSOS_EXT_PROPTYPE_REG_QWORD                      (11)   // 64-bit number

/*********************************************************************
*
*       Types / structures
*/
typedef struct {
  U16  VendorId;        // information used during enumeration
  U16  ProductId;
  char *sVendorName;
  char *sProductName;
  char *sSerialNumber;
} USB_DEVICE_INFO;

typedef struct _USB_INFO_BUFFER USB_INFO_BUFFER;
typedef struct _EP_STAT EP_STAT;

typedef struct {
  U8 bmRequestType;
  U8 bRequest;
  U8 wValueLow;
  U8 wValueHigh;
  U8 wIndexLow;
  U8 wIndexHigh;
  U8 wLengthLow;
  U8 wLengthHigh;
} USB_SETUP_PACKET;

typedef struct _USB_HOOK {
  struct _USB_HOOK * pNext;
  void              (*cb) (void* pContext, U8 NewState);
  void             * pContext;
} USB_HOOK;

typedef struct {
  const U8 * pData;
  U32        NumBytesRem;
} USB_DATA_PART;

typedef union {
  U8         Configuration;
  U8         SetAddressBehavior;
  U8         TestMode;
  U8         RXBehavior;
} USB_IOCTL_PARA;

typedef struct _USB_HW_DRIVER {
  void     (*pfStart)               (void);
  U8       (*pfAllocEP)             (U8 InDir, U8 TransferType);
  void     (*pfUpdateEP)            (EP_STAT * pEPStat);
  void     (*pfEnable)              (void);
  void     (*pfAttach)              (void);
  unsigned (*pfGetMaxPacketSize)    (unsigned EPIndex);
  int      (*pfIsInHighSpeedMode)   (void);
  void     (*pfSetAddress)          (U8  Addr);
  void     (*pfSetClrStallEP)       (unsigned EPIndex, int OnOnff);
  void     (*pfStallEP0)            (void);
  void     (*pfDisableRxInterruptEP)(unsigned EPIndex);
  void     (*pfEnableRxInterruptEP) (unsigned EPIndex, U8 *pData, U32 NumBytesRequested);
  void     (*pfStartTx)             (unsigned EPIndex);
  void     (*pfSendEP)              (unsigned EPIndex, const U8 * p, unsigned NumBytes);
  void     (*pfDisableTx)           (unsigned EPIndex);
  void     (*pfResetEP)             (unsigned EPIndex);
  int      (*pfControl)             (int Cmd, USB_IOCTL_PARA * p);
  int      (*pfDeInit)              (void);
  int      (*pfDetach)              (void);
  U8       (*pfAllocEPEx)           (U8 InDir, U8 TransferType, unsigned MaxPacketSize);
  int      (*pfSendEPEx)            (unsigned EPIndex, unsigned NumParts, const USB_DATA_PART *pParts, unsigned *pNumOfFullPackets);
  void     (*pfInit)                (void);
} USB_HW_DRIVER;

typedef int          USB_ON_CLASS_REQUEST     (const USB_SETUP_PACKET * pSetupPacket);
typedef int          USB_ON_SETUP             (const USB_SETUP_PACKET * pSetupPacket);
typedef void         USB_ADD_FUNC_DESC        (int InterfaceNo, USB_INFO_BUFFER * pInfoBuffer);
typedef void         USB_ADD_EP_FUNC_DESC     (int InterfaceNo, U8 EPAddr, USB_INFO_BUFFER * pInfoBuffer);
typedef void         USB_ON_RX_FUNC           (const U8 * pData, unsigned NumBytes);
typedef void         USB_ISR_HANDLER          (void);
typedef void         USB_DETACH_FUNC          (void);
typedef const char * USB_GET_STRING_FUNC      (int Index);
typedef U16          USB_ON_BCD_VERSION_FUNC  (void);
typedef const char * USB_ON_STRING_REQUEST    (void);
typedef void         USB_DEINIT_FUNC          (void);
typedef void         USB_ON_SET_IF_FUNC       (U16 wIndex, U16 wValue);
typedef void         USB_EVENT_CALLBACK_FUNC  (unsigned Events, void *pContext);
typedef void         USB_ATTACH_FUNC          (void);
typedef void         USB_ENABLE_ISR_FUNC      (USB_ISR_HANDLER * pfISRHandler);
typedef void         USB_INC_DI_FUNC          (void);
typedef void         USB_DEC_RI_FUNC          (void);
typedef void         USB_STATE_CALLBACK_FUNC  (void * pContext, U8 NewState);

typedef struct _USB_EVENT_CALLBACK {
  struct _USB_EVENT_CALLBACK  *pNext;
  USB_EVENT_CALLBACK_FUNC     *pfEventCb;
  void                        *pContext;
} USB_EVENT_CALLBACK;

typedef struct _USB_MS_OS_EXT_PROP {
  U32          PropType;
  const char * sPropName;
  const void * pProp;
  U32          PropSize;
} USB_MS_OS_EXT_PROP;

/*********************************************************************
*
*       USB_ADD_EP_INFO
*
*  Description
*    Structure used by USBD_AddEPEx() when adding an endpoint.
*
*  Additional information
*    The Interval parameter specifies the frequency in which the endpoint should be
*    polled for information by the host. It must be specified in units of 125 us.
*
*    Depending on the actual speed of the device during enumeration, the USB stack
*    converts the interval to the correct value required for the endpoint descriptor
*    according to the USB specification (into milliseconds for low/full speed, into 125 us
*    for high speed).
*
*    For endpoints of type USB_TRANSFER_TYPE_BULK the value is ignored and should be set to 0.
*/
typedef struct _USB_ADD_EP_INFO {
  U8        Flags;            // Reserved. Must be zero.
  U8        InDir;            // Specifies the direction of the desired endpoint.
                              // * USB_DIR_IN
                              // * USB_DIR_OUT
  U8        TransferType;     // Specifies the transfer type of the endpoint.
                              // The following values are allowed:
                              // * USB_TRANSFER_TYPE_BULK
                              // * USB_TRANSFER_TYPE_ISO
                              // * USB_TRANSFER_TYPE_INT
  U16       Interval;         // Specifies the interval measured in units of 125us (microframes).
                              // This value should be zero for a bulk endpoint.
  unsigned  MaxPacketSize;    // Maximum packet size for the endpoint.
} USB_ADD_EP_INFO;


/*********************************************************************
*
*       Public API functions
*
*/
unsigned USBD_AddEP                     (U8 InDir, U8 TransferType, U16 Interval, U8 * pBuffer, unsigned BufferSize);
unsigned USBD_AddEPEx                   (const USB_ADD_EP_INFO * pInfo, U8 * pBuffer, unsigned BufferSize);
void     USBD_DeInit                    (void);
void     USBD_EnableIAD                 (void);
unsigned USBD_GetState                  (void);
void     USBD_Init                      (void);
char     USBD_IsConfigured              (void);
void     USBD_SetMaxPower               (unsigned MaxPower);
void     USBD_Start                     (void);
void     USBD_Stop                      (void);
int      USBD_RegisterSCHook            (USB_HOOK * pHook, USB_STATE_CALLBACK_FUNC *pfStateCb, void * pContext);
int      USBD_UnregisterSCHook          (USB_HOOK * pHook);

int      USBD_Read                      (unsigned EPOut, void* pData, unsigned NumBytesReq, unsigned Timeout);
int      USBD_Receive                   (unsigned EPOut, void* pData, unsigned NumBytesReq, int Timeout);
int      USBD_ReadOverlapped            (unsigned EPOut, void* pData, unsigned NumBytesReq);
int      USBD_Write                     (unsigned EPIndex, const void* pData, unsigned NumBytes, char Send0PacketIfRequired, int ms);
int      USBD_WriteOverlapped           (unsigned EPIndex, const void* pData, unsigned NumBytes, char Send0PacketIfRequired);
void     USBD_CancelIO                  (unsigned EPIndex);
unsigned USBD_GetNumBytesInBuffer       (unsigned EPIndex);
unsigned USBD_GetNumBytesRemToWrite     (unsigned EPIndex);
unsigned USBD_GetNumBytesRemToRead      (unsigned EPIndex);
void     USBD_SetOnRXHookEP             (unsigned EPIndex, USB_ON_RX_FUNC * pfOnRx);
void     USBD_SetClrStallEP             (unsigned EPIndex, int OnOff);
void     USBD_StallEP                   (unsigned EPIndex);
int      USBD_WaitForEndOfTransfer      (unsigned EPIndex, unsigned Timeout);
int      USBD_WaitForTXReady            (unsigned EPIndex, int Timeout);
unsigned USBD_GetReadMode               (unsigned EPIndex);
void     USBD_SetReadMode               (unsigned EPIndex, unsigned Mode);
void     USBD_SetOnEvent                (unsigned EPIndex, USB_EVENT_CALLBACK *pEventCb, USB_EVENT_CALLBACK_FUNC *pfEventCb, void *pContext);
void     USBD_RemoveOnEvent             (unsigned EPIndex, const USB_EVENT_CALLBACK *pEventCb);
int      USBD_IsNULLPacketRequired      (unsigned EPIndex);

void     USBD_SetClassRequestHook       (unsigned InterfaceNum, USB_ON_CLASS_REQUEST * pfOnClassRequest);
void     USBD_SetVendorRequestHook      (unsigned InterfaceNum, USB_ON_CLASS_REQUEST * pfOnVendorRequest);
void     USBD_SetOnSetupHook            (unsigned InterfaceNum, USB_ON_SETUP         * pfOnSetup);
void     USBD_SetOnRxEP0                (USB_ON_RX_FUNC       * pfOnRx);
void     USBD_SetDetachFunc             (USB_DETACH_FUNC * pfDetach);
void     USBD_SetGetStringFunc          (USB_GET_STRING_FUNC * pfOnGetString);
void     USBD_SetOnBCDVersionFunc       (USB_ON_BCD_VERSION_FUNC * pfOnBCDVersion);
void     USBD_SetDeInitUserFunc         (USB_DEINIT_FUNC * pfDeInit);
void     USBD_SetOnSetInterfaceFunc     (USB_ON_SET_IF_FUNC * pfOnSetInterface);

void     USBD_DoRemoteWakeup            (void);
void     USBD_SetIsSelfPowered          (U8 IsSelfPowered);
void     USBD_SetAllowRemoteWakeUp      (U8 AllowRemoteWakeup);
int      USBD_TxIsPending               (unsigned EPIndex);

unsigned USBD_GetMaxPacketSize          (unsigned EPIndex);
unsigned USBD_GetInternalBufferSize     (unsigned EPIndex);

void     USBD_SetMSDescInfo             (U8 InterfaceNum, const char * sCompatibleID, const char * sSubCompatibleID, const USB_MS_OS_EXT_PROP * pProperties, U32 NumProperties);
void     USBD_SetMSVendorCode           (U8 VendorCode);
void     USBD_MSOSD_Init                (void);

unsigned USBD_GetUSBAddr                (void);
void     USBD_WriteEP0FromISR           (const void* pData, unsigned NumBytes, char Send0PacketIfRequired);

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define USB_AddDriver                          USBD_AddDriver
#define USB_AddEP                              USBD_AddEP
#define USB_EnableIAD                          USBD_EnableIAD
#define USB_GetState                           USBD_GetState
#define USB_Init                               USBD_Init
#define USB_IsConfigured                       USBD_IsConfigured
#define USB_SetMaxPower                        USBD_SetMaxPower
#define USB_Start                              USBD_Start
#define USB_Stop                               USBD_Stop
#define USB_RegisterSCHook                     USBD_RegisterSCHook
#define USB_UnregisterSCHook                   USBD_UnregisterSCHook
#define USB_DeInit                             USBD_DeInit

#define USB_CancelIO                           USBD_CancelIO
#define USB_GetNumBytesInBuffer                USBD_GetNumBytesInBuffer

int     USB_ReadEP                             (unsigned EPIndex, void * pData, unsigned NumBytesReq);
int     USB_ReadEPOverlapped                   (unsigned EPIndex, void * pData, unsigned NumBytesReq);
int     USB_ReadEPTimed                        (unsigned EPIndex, void * pData, unsigned NumBytesReq, unsigned ms);
int     USB_ReceiveEP                          (unsigned EPIndex, void * pData, unsigned NumBytesReq);
int     USB_ReceiveEPTimed                     (unsigned EPIndex, void * pData, unsigned NumBytesReq, unsigned ms);
#define USB_WriteEP(h, p, n, s)                USBD_Write(h, p, n, s, 0)
#define USB_WriteEPOverlapped(h, p, n, s)      USBD_WriteOverlapped(h, p, n, s)
#define USB_WriteEPTimed(h, p, n, s, t)        USBD_Write(h, p, n, s, t)
void    USB_StartReadTransfer                  (unsigned EPIndex);
int     USB_IsStartReadTransferActive          (unsigned EPIndex);

#define USB_SetOnRXHookEP                      USBD_SetOnRXHookEP
#define USB_SetClrStallEP                      USBD_SetClrStallEP
#define USB_StallEP                            USBD_StallEP
#define USB_WaitForEndOfTransfer               USBD_WaitForEndOfTransfer

#define USB_SetClassRequestHook                USBD_SetClassRequestHook
#define USB_SetVendorRequestHook               USBD_SetVendorRequestHook
#define USB_SetOnSetupHook                     USBD_SetOnSetupHook
#define USB_SetOnRxEP0                         USBD_SetOnRxEP0
#define USB_SetDetachFunc                      USBD_SetDetachFunc
#define USB_SetGetStringFunc                   USBD_SetGetStringFunc
#define USB_SetOnBCDVersionFunc                USBD_SetOnBCDVersionFunc
#define USB_SetDeInitUserFunc                  USBD_SetDeInitUserFunc

#define USB_DoRemoteWakeup                     USBD_DoRemoteWakeup
#define USB_SetIsSelfPowered                   USBD_SetIsSelfPowered
#define USB_SetAllowRemoteWakeUp               USBD_SetAllowRemoteWakeUp
#define USB_TxIsPending                        USBD_TxIsPending
#define USB_GetMaxPacketSize                   USBD_GetMaxPacketSize
#define USB_GetInternalBufferSize              USBD_GetInternalBufferSize

/*********************************************************************
*
*       Kernel interface routines      (also for polled mode without kernel)
*
*/
void     USB_OS_Init                   (void);
void     USB_OS_Delay                  (int ms);
void     USB_OS_DecRI                  (void);
U32      USB_OS_GetTickCnt             (void);
void     USB_OS_IncDI                  (void);
void     USB_OS_Panic                  (const char *pErrMsg);
#if USBD_OS_LAYER_EX > 0
void     USB_OS_Signal                 (unsigned EPIndex, unsigned TransactCnt);
void     USB_OS_Wait                   (unsigned EPIndex, unsigned TransactCnt);
int      USB_OS_WaitTimed              (unsigned EPIndex, unsigned ms, unsigned TransactCnt);
void     USB_OS_DeInit                 (void);
#else
void     USB_OS_Signal                 (unsigned EPIndex);
void     USB_OS_Wait                   (unsigned EPIndex);
int      USB_OS_WaitTimed              (unsigned EPIndex, unsigned ms);
#endif

/*********************************************************************
*
*       Log/Warn functions
*
**********************************************************************
*/
void USBD_SetLogFilter                  (U32 FilterMask);
void USBD_SetWarnFilter                 (U32 FilterMask);
void USBD_AddLogFilter                  (U32 FilterMask);
void USBD_AddWarnFilter                 (U32 FilterMask);
void USBD_Logf                          (U32 Type,       const char * sFormat, ...);
void USBD_Warnf                         (U32 Type,       const char * sFormat, ...);
void USBD_Logf_Application              (const char * sFormat, ...);

//
// obsolete function, do not use these functions.
// They are only for compatibility reasons here.
// Use instead USBD_* functions.
//
#define USB_SetLogFilter(FilterMask)                               USBD_SetLogFilter(FilterMask)
#define USB_SetWarnFilter(FilterMask)                              USBD_SetWarnFilter(FilterMask)
#define USB_AddLogFilter(FilterMask)                               USBD_AddLogFilter(FilterMask)
#define USB_AddWarnFilter(FilterMask)                              USBD_AddWarnFilter(FilterMask)

void USBD_SetWarnFunc                   (void (*pfWarn)(const char *s));
void USBD_SetLogFunc                    (void (*pfLog) (const char *s));

/*********************************************************************
*
*       USB configuration functions, to be called in USB_X_Config()
*
*/
void USBD_AddDriver                     (const USB_HW_DRIVER * pDriver);
void USBD_SetAttachFunc                 (USB_ATTACH_FUNC *pfAttach);
void USBD_SetISRMgmFuncs                (USB_ENABLE_ISR_FUNC *pfEnableISR, USB_INC_DI_FUNC *pfIncDI, USB_DEC_RI_FUNC *pfDecRI);
void USBD_SetDeviceInfo                 (const USB_DEVICE_INFO *pDeviceInfo);
void USBD_SetCacheConfig                (const SEGGER_CACHE_CONFIG *pConfig, unsigned ConfSize);

/*********************************************************************
*
*       Function that has to be supplied by the customer
*
*/
void USBD_X_Config                      (void);
void USBD_X_EnableInterrupt             (void);   // optional function, activate with USBD_OS_USE_USBD_X_INTERRUPT
void USBD_X_DisableInterrupt            (void);   // optional function, activate with USBD_OS_USE_USBD_X_INTERRUPT

/*********************************************************************
*
*       Template Functions that can be used for outputting the warn
*       and log messages.
*       They are used in all samples provided with emUSB-Device.
*       These functions are used with the sample log and warning
*       implementation located under Sample\TermIO
*       Warn and log output can be individually set to other function
*       by using the functions:
*         USBD_SetWarnFunc              ()
*         USBD_SetLogFunc               ()
*
*/
void USB_X_Warn                        (const char * s);
void USB_X_Log                         (const char * s);

/*********************************************************************
*
*       Profiling instrumentation functions
*/
void USBD_SYSVIEW_Init(void);

/*********************************************************************
*
*       Utility functions
*/
U16      USBD_GetU16BE                  (const U8 * p);
U16      USBD_GetU16LE                  (const U8 * p);
U32      USBD_GetU32BE                  (const U8 * p);
U32      USBD_GetU32LE                  (const U8 * p);
void     USBD_StoreU16BE                (U8 * p, unsigned v);
void     USBD_StoreU16LE                (U8 * p, unsigned v);
void     USBD_StoreU32LE                (U8 * p, U32 v);
void     USBD_StoreU32BE                (U8 * p, U32 v);
U32      USBD_SwapU32                   (U32 v);

/*********************************************************************
*
*       Functions necessary for easy migration from emUSB V2 to V3
*
*/
#if USB_V2_V3_MIGRATION_DEVINFO > 0
const char * USB_GetVendorName         (void);
const char * USB_GetProductName        (void);
const char * USB_GetSerialNumber       (void);
U16          USB_GetVendorId           (void);
U16          USB_GetProductId          (void);
#endif
#if USB_V2_V3_MIGRATION_CONFIG > 0
void         USB_X_AddDriver           (void);
void         USB_X_HWAttach            (void);
void         USB_X_EnableISR           (USB_ISR_HANDLER * pfISRHandler);
#endif

/*********************************************************************
*
*       Individual driver configuration functions
*
*/
void USB_DRIVER_LPC17xx_ConfigAddr    (U32 BaseAddr);
void USB_DRIVER_LPC18xx_ConfigAddr    (U32 BaseAddr);
void USB_DRIVER_LPC43xx_ConfigAddr    (U32 BaseAddr);
void USB_DRIVER_P1020_ConfigAddr      (U32 BaseAddr);
void USB_DRIVER_RX_ConfigAddr         (U32 BaseAddr);
void USB_DRIVER_RZ_ConfigAddr         (U32 BaseAddr);
void USB_DRIVER_R8A66597_ConfigAddr   (U32 BaseAddr);
void USB_DRIVER_SH726A_ConfigAddr     (U32 BaseAddr);
void USB_DRIVER_KinetisEHCI_ConfigAddr(U32 BaseAddr);

void USB_DRIVER_STM32F7xxHS_ConfigPHY(U8 UsePHY);
void USB_DRIVER_STM32F4xxHS_ConfigPHY(U8 UsePHY);

/*********************************************************************
*
*       Compatibility Macros for configuring the base address
*
*/
#define USB_DRIVER_RX62N_ConfigAddr(x) USB_DRIVER_RX_ConfigAddr(x)
#define USB_DRIVER_RX63N_ConfigAddr(x) USB_DRIVER_RX_ConfigAddr(x)
#define USB_DRIVER_RX64M_ConfigAddr(x) USB_DRIVER_RX_ConfigAddr(x)
#define USB_DRIVER_RX65N_ConfigAddr(x) USB_DRIVER_RX_ConfigAddr(x)
#define USB_DRIVER_RX71M_ConfigAddr(x) USB_DRIVER_RX_ConfigAddr(x)

/*********************************************************************
*
*       Available target USB drivers
*
*/
#define USB_Driver_AtmelCAP9            USB_Driver_Atmel_CAP9
#define USB_Driver_AtmelSAM3U           USB_Driver_Atmel_SAM3U
#define USB_Driver_AtmelRM9200          USB_Driver_Atmel_RM9200
#define USB_Driver_AtmelSAM7A3          USB_Driver_Atmel_SAM7A3
#define USB_Driver_AtmelSAM7S           USB_Driver_Atmel_SAM7S
#define USB_Driver_AtmelSAM7SE          USB_Driver_Atmel_SAM7SE
#define USB_Driver_AtmelSAM7X           USB_Driver_Atmel_SAM7X
#define USB_Driver_AtmelSAM9260         USB_Driver_Atmel_SAM9260
#define USB_Driver_AtmelSAM9261         USB_Driver_Atmel_SAM9261
#define USB_Driver_AtmelSAM9263         USB_Driver_Atmel_SAM9263
#define USB_Driver_AtmelSAM9G45         USB_Driver_Atmel_SAM9G45
#define USB_Driver_AtmelSAM9G20         USB_Driver_Atmel_SAM9G20
#define USB_Driver_AtmelSAM9Rx64        USB_Driver_Atmel_SAM9Rx64
#define USB_Driver_AtmelSAM9XE          USB_Driver_Atmel_SAM9XE
#define USB_Driver_NXPLPC13xx           USB_Driver_NXP_LPC13xx
#define USB_Driver_NXPLPC17xx           USB_Driver_NXP_LPC17xx
#define USB_Driver_NXPLPC214x           USB_Driver_NXP_LPC214x
#define USB_Driver_NXPLPC23xx           USB_Driver_NXP_LPC23xx
#define USB_Driver_NXPLPC24xx           USB_Driver_NXP_LPC24xx
#define USB_Driver_NXPLPC288x           USB_Driver_NXP_LPC288x
#define USB_Driver_NXPLPC318x           USB_Driver_NXP_LPC318x
#define USB_Driver_NXPLPC313x           USB_Driver_NXP_LPC313x
#define USB_Driver_OKI69Q62             USB_Driver_OKI_69Q62
#define USB_Driver_SharpLH79524         USB_Driver_Sharp_LH79524
#define USB_Driver_SharpLH7A40x         USB_Driver_Sharp_LH7A40x
#define USB_Driver_STSTM32              USB_Driver_ST_STM32
#define USB_Driver_STSTM32F107          USB_Driver_ST_STM32F107
#define USB_Driver_STSTR71x             USB_Driver_ST_STR71x
#define USB_Driver_STSTR750             USB_Driver_ST_STR750
#define USB_Driver_STSTR91x             USB_Driver_ST_STR91x
#define USB_Driver_H8SX1668R            USB_Driver_Renesas_H8SX1668R
#define USB_Driver_H8S2472              USB_Driver_Renesas_H8S2472
#define USB_Driver_TMPA910              USB_Driver_Toshiba_TMPA910
#define USB_Driver_TMPA900              USB_Driver_Toshiba_TMPA900
#define USB_Driver_SH7203               USB_Driver_Renesas_SH7203
#define USB_Driver_SH7216               USB_Driver_Renesas_SH7216
#define USB_Driver_SH7286               USB_Driver_Renesas_SH7286
#define USB_Driver_Renesas_RX62N        USB_Driver_Renesas_RX
#define USB_Driver_Renesas_RX63N        USB_Driver_Renesas_RX
#define USB_Driver_Renesas_RX64M        USB_Driver_Renesas_RX
#define USB_Driver_Renesas_RX65N        USB_Driver_Renesas_RX
#define USB_Driver_Renesas_RX71M        USB_Driver_Renesas_RX
#define USB_Driver_Renesas_SH7269       USB_Driver_Renesas_SH7268
#define USB_Driver_Atmel_SAM9X35        USB_Driver_Atmel_SAM9X25
#define USB_Driver_Atmel_SAMA5Dx        USB_Driver_Atmel_SAMA5D3x
#define USB_Driver_Microchip_PIC32      USB_Driver_Microchip_PIC32MX
#define USB_Driver_ST_STM32             USB_Driver_ST_STM32x32
#define USB_Driver_ST_STM32F3xx6        USB_Driver_ST_STM32x16
#define USB_Driver_ST_STM32F3xx8        USB_Driver_ST_STM32x16
#define USB_Driver_ST_STM32F3xxB        USB_Driver_ST_STM32x32
#define USB_Driver_ST_STM32F3xxC        USB_Driver_ST_STM32x32
#define USB_Driver_ST_STM32F3xxD        USB_Driver_ST_STM32x16
#define USB_Driver_ST_STM32F3xxE        USB_Driver_ST_STM32x16
#define USB_Driver_ST_STM32F0           USB_Driver_ST_STM32x16
#define USB_Driver_Freescale_K40        USB_Driver_Freescale_KHCI
#define USB_Driver_Freescale_K60        USB_Driver_Freescale_KHCI
#define USB_Driver_Freescale_K70        USB_Driver_Freescale_KHCI
#define USB_Driver_Renesas_SynergyS3_FS USB_Driver_Renesas_SynergyFS
#define USB_Driver_Renesas_SynergyS5_FS USB_Driver_Renesas_SynergyFS
#define USB_Driver_Renesas_SynergyS7_FS USB_Driver_Renesas_SynergyFS
#define USB_Driver_Renesas_RX200        USB_Driver_Renesas_RX100
#define USB_Driver_SiLabs_EFM32GG11     USB_Driver_EM_EFM32GG990

extern const USB_HW_DRIVER USB_Driver_Dummy;
extern const USB_HW_DRIVER USB_Driver_Atmel_AT32UC3x;
extern const USB_HW_DRIVER USB_Driver_Atmel_CAP9;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM3U;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM3X;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM3S;
extern const USB_HW_DRIVER USB_Driver_Atmel_RM9200;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM7A3;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM7S;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM7SE;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM7X;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9260;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9261;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9263;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9G45;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9G20;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9Rx64;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9XE;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAM9X25;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAMA5D2x;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAMA5D3x;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAMA5D4x;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAMV7;
extern const USB_HW_DRIVER USB_Driver_Atmel_SAMD21;
extern const USB_HW_DRIVER USB_Driver_EM_EFM32GG990;
extern const USB_HW_DRIVER USB_Driver_Freescale_iMX25x;
extern const USB_HW_DRIVER USB_Driver_Freescale_KHCI;
extern const USB_HW_DRIVER USB_Driver_Freescale_KinetisEHCI;
extern const USB_HW_DRIVER USB_Driver_Freescale_iMX28x;
extern const USB_HW_DRIVER USB_Driver_Freescale_MCF227x;
extern const USB_HW_DRIVER USB_Driver_Freescale_MCF225x;
extern const USB_HW_DRIVER USB_Driver_Freescale_MCF51JMx;
extern const USB_HW_DRIVER USB_Driver_Freescale_Vybrid;
extern const USB_HW_DRIVER USB_Driver_Freescale_P1020;
extern const USB_HW_DRIVER USB_Driver_Cypress_MB9BFxxx;
extern const USB_HW_DRIVER USB_Driver_Infineon_XMC45xx;
extern const USB_HW_DRIVER USB_Driver_Maxim_MAX3590;
extern const USB_HW_DRIVER USB_Driver_Microchip_PIC32MX;
extern const USB_HW_DRIVER USB_Driver_NEC_70F376x;
extern const USB_HW_DRIVER USB_Driver_NEC_70F3765;
extern const USB_HW_DRIVER USB_Driver_NEC_uPD720150;
extern const USB_HW_DRIVER USB_Driver_NEC_78F102x;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC13xx;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC17xx;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC18xx;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC214x;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC23xx;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC24xx;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC288x;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC318x;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC313x;
extern const USB_HW_DRIVER USB_Driver_NXP_LPC43xx;
extern const USB_HW_DRIVER USB_Driver_OKI_69Q62;
extern const USB_HW_DRIVER USB_Driver_Renesas_H8SX1668R;
extern const USB_HW_DRIVER USB_Driver_Renesas_H8S2472;
extern const USB_HW_DRIVER USB_Driver_Renesas_RL78;
extern const USB_HW_DRIVER USB_Driver_Renesas_RZ;
extern const USB_HW_DRIVER USB_Driver_Renesas_RZG1;
extern const USB_HW_DRIVER USB_Driver_Renesas_RX;
extern const USB_HW_DRIVER USB_Driver_Renesas_RX64M_USBA;
extern const USB_HW_DRIVER USB_Driver_Renesas_RX71M_HS;
extern const USB_HW_DRIVER USB_Driver_Renesas_RX100;
extern const USB_HW_DRIVER USB_Driver_Renesas_SH7203;
extern const USB_HW_DRIVER USB_Driver_Renesas_SH7216;
extern const USB_HW_DRIVER USB_Driver_Renesas_SH7268;
extern const USB_HW_DRIVER USB_Driver_Renesas_SH7286;
extern const USB_HW_DRIVER USB_Driver_Renesas_SH726A;
extern const USB_HW_DRIVER USB_Driver_Renesas_SynergyS1;
extern const USB_HW_DRIVER USB_Driver_Renesas_SynergyFS;
extern const USB_HW_DRIVER USB_Driver_Renesas_SynergyHS;
extern const USB_HW_DRIVER USB_Driver_Renesas_uPD70F351x;
extern const USB_HW_DRIVER USB_Driver_Renesas_R8A66597;
extern const USB_HW_DRIVER USB_Driver_Sharp_LH79524;
extern const USB_HW_DRIVER USB_Driver_Sharp_LH7A40x;
extern const USB_HW_DRIVER USB_Driver_ST_STM32x32;
extern const USB_HW_DRIVER USB_Driver_ST_STM32x16;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F107;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F4xxFS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F4xxHS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F4xxHS_inFS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F7xxFS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32F7xxHS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32H7xxFS;
extern const USB_HW_DRIVER USB_Driver_ST_STM32L4xx;
extern const USB_HW_DRIVER USB_Driver_ST_STR71x;
extern const USB_HW_DRIVER USB_Driver_ST_STR750;
extern const USB_HW_DRIVER USB_Driver_ST_STR91x;
extern const USB_HW_DRIVER USB_Driver_TI_AM335x;
extern const USB_HW_DRIVER USB_Driver_TI_AM335xDMA;
extern const USB_HW_DRIVER USB_Driver_TI_LM3S9B9x;
extern const USB_HW_DRIVER USB_Driver_TI_MSP430;
extern const USB_HW_DRIVER USB_Driver_TI_OMAP_L138;
extern const USB_HW_DRIVER USB_Driver_TI_TM4Cxx;
extern const USB_HW_DRIVER USB_Driver_Toshiba_TMPM369;
extern const USB_HW_DRIVER USB_Driver_Toshiba_TMPA900;
extern const USB_HW_DRIVER USB_Driver_Toshiba_TMPA910;
extern const USB_HW_DRIVER USB_Driver_Xilinx_Zynq7010;
extern const USB_HW_DRIVER USB_Driver_DialogSemi_DA1468x;
extern const USB_HW_DRIVER USB_Driver_DialogSemi_DA1469x;

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif
