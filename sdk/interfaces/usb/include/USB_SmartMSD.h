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
File    : USB_SmartMSD.h
Purpose : USB_SmartMSD API specification
--------- END-OF-HEADER ----------------------------------------------
*/

#ifndef USB_SMARTMSD_H            // Avoid multiple inclusion
#define USB_SMARTMSD_H

#include "Global.h"
#include "USB_MSD.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define USB_SMSD_ATTR_READ_ONLY       (0x01)
#define USB_SMSD_ATTR_HIDDEN          (0x02)
#define USB_SMSD_ATTR_SYSTEM          (0x04)
#define USB_SMSD_ATTR_VOLUME_ID       (0x08)
#define USB_SMSD_ATTR_DIRECTORY       (0x10)
#define USB_SMSD_ATTR_ARCHIVE         (0x20)
#define USB_SMSD_ATTR_LONG_NAME       (USB_SMSD_ATTR_READ_ONLY | USB_SMSD_ATTR_HIDDEN | USB_SMSD_ATTR_SYSTEM | USB_SMSD_ATTR_VOLUME_ID)
#define USB_SMSD_ATTR_LONG_NAME_MASK  (USB_SMSD_ATTR_READ_ONLY | USB_SMSD_ATTR_HIDDEN | USB_SMSD_ATTR_SYSTEM | USB_SMSD_ATTR_VOLUME_ID | USB_SMSD_ATTR_DIRECTORY | USB_SMSD_ATTR_ARCHIVE)

//
// For use in USB_SMSD_CONST_FILE.Flags
//
#define USB_SMSD_FILE_WRITABLE        USB_SMSD_ATTR_READ_ONLY
#define USB_SMSD_FILE_AHEAD           (1<<8)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       USB_SMSD_CONST_FILE
*
*  Description
*    This structure contains information about a constant file which
*    cannot be changed at run time and should be shown inside the
*    SmartMSD volume (e.g. Readme.txt).
*    This structure is a parameter for the USBD_SMSD_AddConstFiles()
*    function.
*
*  Additional information
*    If a file does not occupy complete sectors the remaining bytes of the last sector are
*    automatically filled with 0s on read.
*    If pData is NULL the file is not displayed in the volume. This is useful when the application
*    has certain files which should only be displayed after certain events (e.g. the
*    application displays a Fail.txt when the device is reconnected after an unsuccessful
*    firmware update).
*/
typedef struct {
  const char* sName;            // Pointer to a zero-terminated string containing the filename.
  const U8* pData;              // Pointer to the file data. Can be NULL.
  unsigned FileSize;            // Size of the file. Normally the size of the data pointed to by pData.
  U32 Flags;                    // Can be one of the following items:
                                // * USB_SMSD_FILE_WRITABLE: The file is writable
                                // * USB_SMSD_FILE_AHEAD: File is located at the start of the volume. Normally constant files are allocated at the end of the volume.
} USB_SMSD_CONST_FILE;

/*********************************************************************
*
*       USB_SMSD_DIR_ENTRY_SHORT
*
*  Description
*    Structure used to describe an entry with a short file name.
*    This structure is a member of USB_SMSD_DIR_ENTRY.
*/
typedef struct {
  U8  acFilename[8];        // File name, limited to 8 characters (short file name), padded with spaces (0x20).
  U8  acExt[3];             // File extension, limited to 3 characters (short file name), padded with spaces (0x20).
  U8  DirAttr;              // File attributes. Available attributes are listed below.
  U8  NTRes;                // Reserved for use by Windows NT.
  U8  CrtTimeTenth;         // Millisecond stamp at file creation time. This field actually contains a count of tenths of a second.
  U16 CrtTime;              // Creation time.
  U16 CrtDate;              // Date file was created.
  U16 LstAccDate;           // Last access date. Note that there is no last access time, only a date. This is the date of last read or write.
  U16 FstClusHI;            // High word of this entry?'s first cluster number.
  U16 WrtTime;              // Time of last write.
  U16 WrtDate;              // Date of last write.
  U16 FstClusLO;            // Low word of this entry'?s first cluster number.
  U32 FileSize;             // File size in bytes.
} USB_SMSD_DIR_ENTRY_SHORT;

/*********************************************************************
*
*       USB_SMSD_DIR_ENTRY_LONG
*
*  Description
*    Structure used to describe an entry with a long file name.
*    This structure is a member of USB_SMSD_DIR_ENTRY.
*    This is for information only, the read and write callbacks
*    only receive short file names.
*/
typedef struct {
  U8  Ord;             // The order of this entry in the sequence of long dir entries, associated with the short dir entry at the end of the long dir set.
  U8  acName1[10];     // Characters 1-5 of the long-name sub-component in this dir entry.
  U8  Attr;            // Attributes - must be USB_SMSD_ATTR_LONG_NAME.
  U8  Type;            // If zero, indicates a directory entry that is a sub-component of a long name. Other values reserved for future extensions. Non-zero implies other types.
  U8  Chksum;          // Checksum of name in the short dir entry at the end of the long dir set.
  U8  acName2[12];     // Characters 6-11 of the long-name sub-component in this dir entry.
  U16 FstClusLO;       // Must be zero.
  U8  acName3[4];      // Characters 12-13 of the long-name sub-component in this dir entry.
} USB_SMSD_DIR_ENTRY_LONG;

/*********************************************************************
*
*       USB_SMSD_DIR_ENTRY
*
*  Description
*    Union containing references to directory entries. This union
*    is a member of USB_SMSD_FILE_INFO.
*
*  Additional information
*    Check USB_SMSD_DIR_ENTRY_SHORT and USB_SMSD_DIR_ENTRY_LONG
*    for more information.
*/
typedef union {
  USB_SMSD_DIR_ENTRY_SHORT ShortEntry;   // Allows to access the entry as a "short directory entry".
  USB_SMSD_DIR_ENTRY_LONG  LongEntry;    // Allows to access the entry as a "long directory entry".
  U8                       ac[32];       // Allows to write directly to the structure without casting or using the members.
} USB_SMSD_DIR_ENTRY;

/*********************************************************************
*
*       USB_SMSD_FILE_INFO
*
*  Description
*    Structure used in the read and write callbacks.
*
*  Additional information
*   Check USB_SMSD_ON_READ_FUNC, USB_SMSD_ON_WRITE_FUNC and
*   USB_SMSD_DIR_ENTRY_SHORT for more information.
*/
typedef struct {
  const USB_SMSD_DIR_ENTRY_SHORT* pDirEntry;    // Pointer to a USB_SMSD_DIR_ENTRY_SHORT structure.
} USB_SMSD_FILE_INFO;

/*********************************************************************
*
*       USB_SMSD_ON_READ_FUNC
*
*  Description
*    Callback function prototype that is used when calling the
*    USBD_SMSD_SetUserAPI() function.
*
*  Parameters
*    Lun          : Zero-based index for the unit number.
                    Using only one virtual volume, this parameter is 0.
*    pData        : Pointer to a buffer in which the data is stored.
*    Off          : Offset in the file which is read by the host.
*    NumBytes     : Amount of bytes requested by the host.
*    pFile        : Pointer to a USB_SMSD_FILE_INFO structure describing the file.
*
*  Return value
*    == 0:   Success.
*    != 0:   An error occurred.
*/
typedef int    (USB_SMSD_ON_READ_FUNC)(unsigned Lun, U8 * pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO * pFile);

/*********************************************************************
*
*       USB_SMSD_ON_WRITE_FUNC
*
*  Description
*    Callback function prototype that is used when calling the
*    USBD_SMSD_SetUserAPI() function.
*
*  Parameters
*    Lun          : Zero-based index for the unit number.
                    Using only one virtual volume, this parameter is 0.
*    pData        : Pointer to the data to be written (received from the host).
*    Off          : Offset in the file which the host writes.
*    NumBytes     : Amount of bytes to write.
*    pFile        : Pointer to a USB_SMSD_FILE_INFO structure describing the file.
*
*  Return value
*    == 0:   Success.
*    != 0:   An error occurred.
*
*  Additional information
*    Depending on the behavior of the host operating system it is possible that pFile is
*    NULL. In this case we recommend to perform data analysis to recognize the file.
*/
typedef int    (USB_SMSD_ON_WRITE_FUNC)(unsigned Lun, const U8   * pData, U32 Off, U32 NumBytes, const USB_SMSD_FILE_INFO * pFile);

/*********************************************************************
*
*       USB_SMSD_MEM_ALLOC
*
*  Description
*    Function prototype that is used when memory is being allocated
*    by the SmartMSD module.
*
*  Parameters
*    Size       : Size of the required memory in bytes.
*
*  Return value
*    Pointer to the allocated memory or NULL.
*/
typedef void * (USB_SMSD_MEM_ALLOC)(U32 Size);
/*********************************************************************
*
*       USB_SMSD_MEM_FREE
*
*  Description
*    Function prototype that is used when memory is being freed
*    by the SmartMSD module.
*
*  Parameters
*    p     : Pointer to a memory block which was previously
*            allocated by USB_SMSD_MEM_ALLOC.
*/
typedef void   (USB_SMSD_MEM_FREE)(void * p);

/*********************************************************************
*
*       USB_SMSD_CONST_FILE
*
*  Description
*    This structure contains the function pointers for user provided functions.
*    This structure is a parameter for the USBD_SMSD_SetUserAPI() function.
*/
typedef struct _USB_SMSD_USER_FUNC_API {
  USB_SMSD_ON_READ_FUNC  * pfOnReadSector;             // Pointer to a callback function of type USB_SMSD_ON_READ_FUNC which
                                                       // is called when a sector is read from the host.
                                                       // This function is mandatory and can not be NULL.
  USB_SMSD_ON_WRITE_FUNC * pfOnWriteSector;            // Pointer to a callback function of type USB_SMSD_ON_WRITE_FUNC which
                                                       // is called when a sector is written from the host.
                                                       // This function is mandatory and can not be NULL.
  USB_SMSD_MEM_ALLOC     * pfMemAlloc;                 // Pointer to a user provided alloc function of type USB_SMSD_MEM_ALLOC.
                                                       // If this pointer is NULL the internal alloc function is called.
                                                       // If no memory block is assigned USB_PANIC is called.
  USB_SMSD_MEM_FREE      * pfMemFree;                  // Pointer to a user provided free function of type USB_SMSD_MEM_FREE.
                                                       // If this pointer is NULL the internal free function is called.
} USB_SMSD_USER_FUNC_API;

/*********************************************************************
*
*       Const data
*
**********************************************************************
*/
extern const USB_MSD_STORAGE_API USB_MSD_StorageSMSD;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
//
// Configuration functions that can be called within the USB_SMSD_X_Config function
//
void USBD_SMSD_AssignMemory          (U32 * p, U32 NumBytes);
void USBD_SMSD_SetUserAPI            (const USB_SMSD_USER_FUNC_API            * pUserFunc);
void USBD_SMSD_SetNumRootDirSectors  (unsigned Lun, unsigned                    NumRootDirSectors);
int  USBD_SMSD_SetVolumeInfo         (unsigned Lun, const char                * sVolumeName, const USB_MSD_LUN_INFO * pLunInfo);
int  USBD_SMSD_AddConstFiles         (unsigned Lun, const USB_SMSD_CONST_FILE * paConstFile, unsigned NumFiles);    // Add list of predefined files. such as Readme.txt, ...
void USBD_SMSD_SetNumSectors         (unsigned Lun, unsigned                    NumSectors);
void USBD_SMSD_SetSectorsPerCluster  (unsigned Lun, unsigned                    SectorsPerCluster);

void USBD_SMSD_Add                   (void);
void USBD_SMSD_Init                  (void);
void USBD_SMSD_ReInit                (void);
void USBD_SMSD_DeInit                (void);

void USB_SMSD_X_Config               (void);    // Has to be defined by user

/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/

#define USB_SMSD_AssignMemory           USBD_SMSD_AssignMemory
#define USB_SMSD_SetUserAPI             USBD_SMSD_SetUserAPI
#define USB_SMSD_SetNumRootDirSectors   USBD_SMSD_SetNumRootDirSectors
#define USB_SMSD_SetVolumeInfo          USBD_SMSD_SetVolumeInfo
#define USB_SMSD_AddConstFiles          USBD_SMSD_AddConstFiles
#define USB_SMSD_SetNumSectors          USBD_SMSD_SetNumSectors
#define USB_SMSD_SetSectorsPerCluster   USBD_SMSD_SetSectorsPerCluster

#define USB_SMSD_Init                   USBD_SMSD_Add
#define USB_SMSD_ReInit                 USBD_SMSD_ReInit
#define USB_SMSD_DeInit                 USBD_SMSD_DeInit

/*********************************************************************
*  End of Wrapper
**********************************************************************/

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
