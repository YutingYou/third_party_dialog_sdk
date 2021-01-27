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
File    : USB_MTP.h
Purpose : Public header of USB MTP (Media Transfer Protocol)
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef MTP_H          /* Avoid multiple inclusion */
#define MTP_H

#include "USB_SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines, configurable
*
**********************************************************************
*/
#ifndef   MTP_USB_CLASS
  #define MTP_USB_CLASS           6       // Class of USB device
#endif

#ifndef   MTP_USB_SUBCLASS
  #define MTP_USB_SUBCLASS        1       // Subclass of USB device
#endif

#ifndef   MTP_USB_PROTOCOL
  #define MTP_USB_PROTOCOL        1       // Protocol of USB device
#endif

#ifndef   MTP_MAX_MUM_STORAGES
  #define MTP_MAX_MUM_STORAGES    4       // Maximum number of storage units
#endif

#ifndef   MTP_SAVE_FILE_INFO
  #define MTP_SAVE_FILE_INFO      0       // Information related to file is stored in object instance. Requires more RAM.
#endif

#ifndef   MTP_MAX_FILE_PATH
  #define MTP_MAX_FILE_PATH       256     // Maximum number of characters in the path of a file or directory
#endif

#ifndef   MTP_SUPPORT_UTF8
  #define MTP_SUPPORT_UTF8        1       // File/directory names are encoded in UTF-8 format
#endif

#ifndef   MTP_SUPPORT_EVENTS
  #define MTP_SUPPORT_EVENTS      1       // Support Events such as object removed/added, new storage added/removed
#endif

#ifndef   MTP_SUPPORT_DEV_PROPERTIES
  #define MTP_SUPPORT_DEV_PROPERTIES   1  // Support Device properties
#endif

#ifndef   USB_MTP_NAME_CASE_SENSITIVE
  #define USB_MTP_NAME_CASE_SENSITIVE  0    // When checking file names and directory name shall the string compare be case sensitiv:
                                            //   0 - case sensitivity is not checked. (default)
                                            //   1 - case sensitivity is checked.
#endif

#ifndef USB_V2_V3_MIGRATION_MTP_INFO
  #define USB_V2_V3_MIGRATION_MTP_INFO   0
#endif

/*********************************************************************
*
*       defines, non-configurable
*
**********************************************************************
*/
#define MTP_NUM_BYTES_FILE_ID     16

#define MTP_FILE_ATTR_WP          0x01
#define MTP_FILE_ATTR_SYSTEM      0x02
#define MTP_FILE_ATTR_HIDDEN      0x04

#define USB_MTP_OBJECT_PROP_ALL_PROPERTIES     0xFFFFFFFFUL

/*********************************************************************
*
*       Typedefs, enums and structs
*
**********************************************************************
*/
/*********************************************************************
*
*       USB_MTP_EVENT
*
*  Description
*    Enum containing the MTP event codes.
*/
typedef enum _USB_MTP_EVENT {
  USB_MTP_EVENT_UNDEFINED = 0x4000,         // This event code is undefined, and is not used
  USB_MTP_EVENT_CANCELTRANSACTION,          // This event is used to initiate the cancellation
                                            // of a transaction over transports which do not have
                                            // their own mechanism for cancelling transactions.
                                            // Currently not used.
  USB_MTP_EVENT_OBJECTADDED,                // This event informs the host about a new object
                                            // that has been added to the storage.
  USB_MTP_EVENT_OBJECTREMOVED,              // Informs the host that an object has been removed.
  USB_MTP_EVENT_STOREADDED,                 // This event indicates that a storage has been added
                                            // to the device. It allows to dynamically show the available storages.
  USB_MTP_EVENT_STOREREMOVED,               // This event indicates that a storage has been removed to the device.
                                            // It allows to dynamically hide the available storages.
  USB_MTP_EVENT_DEVICEPROPCHANGED,          // A property changed on the device has occurred. Currently not used.
  USB_MTP_EVENT_OBJECTINFOCHANGED,          // This event indicates that the information for a particular object
                                            // has changed and that the host should acquire the information once again.
  USB_MTP_EVENT_DEVICEINFOCHANGED,          // This event indicates that the capabilities of the device have
                                            // changed and that the DeviceInfo should be requested again.
                                            // Currently not used.
  USB_MTP_EVENT_REQUESTOBJECTTRANSFER,      // This event can be used by the device to ask the host to initiate
                                            // an file object transfer to him. Currently not used.
  USB_MTP_EVENT_STOREFULL,                  // This event should be sent when a storage becomes full.
  USB_MTP_EVENT_DEVICERESET,                // Notifies the host about an internal reset. Currently not used
  USB_MTP_EVENT_STORAGEINFOCHANGED,         // This event is used when information of a storage changes.
  USB_MTP_EVENT_CAPTURECOMPLETE,            // Informs the host that the previously initiated capture acquire
                                            // is complete. Currently not used.
  USB_MTP_EVENT_UNREPORTEDSTATUS,           // This event may be implemented for certain transports in cases
                                            // where the responder unable to report events to the initiator
                                            // regarding changes in its internal status. Currently not used.
  USB_MTP_EVENT_OBJECTPROPCHANGED = 0xC801, // Informs about a change in the object property of an specific object.
                                            // Currently not used.
  USB_MTP_EVENT_OBJECTPROPDESCCHANGED,      // This event informs that the property description of an object
                                            // property has been changed and needs to be re-acquired.
                                            // Currently not used.
  USB_MTP_EVENT_OBJECTREFERENCESCHANGED     // This event is used to indicate that the references on an object
                                            // have been updated. Currently not used.
} USB_MTP_EVENT;

/*********************************************************************
*
*       USB_MTP_INST_DATA_DRIVER
*
*  Description
*    Structure which stores the parameters passed to the storage driver.
*
*  Additional information
*    pRootDir can specify the root of the file system or any other subdirectory.
*/
typedef struct USB_MTP_INST_DATA_DRIVER {
  const char * pRootDir;          // Path to directory to be used as the root of the storage.
  U8           IsRemovable;
} USB_MTP_INST_DATA_DRIVER;

/*********************************************************************
*
*       USB_MTP_STORAGE_INFO
*
*  Description
*    Structure which stores information about a storage.
*/
typedef struct USB_MTP_STORAGE_INFO {
  U32  NumKBytesTotal;            // Storage capacity in kBytes
  U32  NumKBytesFreeSpace;        // Available free space on storage in kBytes
  U16  FSType;                    // Type of file system as specified by MTP
  U8   IsWriteProtected;          // Set to 1 if the storage medium can not be modified
  U8   IsRemovable;               // Set to 1 if the storage medium can be removed from device
  char DirDelimiter;              // Character which separates the directory/file names in a path
} USB_MTP_STORAGE_INFO;

/*********************************************************************
*
*       USB_MTP_FILE_INFO
*
*  Description
*    Structure which stores information about a file or directory.
*/
typedef struct USB_MTP_FILE_INFO {
  char * pFilePath;               // Full path to file.
  char * pFileName;               // Pointer to beginning of file/directory name in pFilePath.
  U32    FileSize;                // Size of the file in bytes. 0xFFFFFFFF when larger than 4GB.
  U32    CreationTime;            // The time and date when the file was created.
  U32    LastWriteTime;           // The time and date when the file was last modified.
  U8     IsDirectory;             // Set to 1 if the path points to a directory.
  U8     Attributes;              // Bitmask of file attributes (MTP_FILE_ATTR_...).
  U8     acId[MTP_NUM_BYTES_FILE_ID]; // Unique identifier which persists between MTP sessions.
} USB_MTP_FILE_INFO;

typedef struct _USB_MTP_STRING {
  U32 NumBytes;
  char * sString;
} USB_MTP_STRING;

/*********************************************************************
*
*       Codes for object and device properties
*/
typedef enum _USB_MTP_OBJECT_PROPERTIES {
  /*lint --e{621} MISRA C:2012 Rule 5.1, required */
  USB_MTP_OBJECT_PROP_STORAGE_ID                            = 0xDC01,
  USB_MTP_OBJECT_PROP_OBJECT_FORMAT                         = 0xDC02,
  USB_MTP_OBJECT_PROP_PROTECTION_STATUS                     = 0xDC03,
  USB_MTP_OBJECT_PROP_OBJECT_SIZE                           = 0xDC04,
  USB_MTP_OBJECT_PROP_ASSOCIATION_TYPE                      = 0xDC05,
  USB_MTP_OBJECT_PROP_ASSOCIATION_DESC                      = 0xDC06,
  USB_MTP_OBJECT_PROP_OBJECT_FILE_NAME                      = 0xDC07,
  USB_MTP_OBJECT_PROP_DATE_CREATED                          = 0xDC08,
  USB_MTP_OBJECT_PROP_DATE_MODIFIED                         = 0xDC09,
  USB_MTP_OBJECT_PROP_KEYWORDS                              = 0xDC0A,
  USB_MTP_OBJECT_PROP_PARENT_OBJECT                         = 0xDC0B,
  USB_MTP_OBJECT_PROP_ALLOWED_FOLDER_CONTENTS               = 0xDC0C,
  USB_MTP_OBJECT_PROP_HIDDEN                                = 0xDC0D,
  USB_MTP_OBJECT_PROP_SYSTEM_OBJECT                         = 0xDC0E,
  USB_MTP_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER   = 0xDC41,
  USB_MTP_OBJECT_PROP_SYNCID                                = 0xDC42,
  USB_MTP_OBJECT_PROP_PROPERTY_BAG                          = 0xDC43,
  USB_MTP_OBJECT_PROP_NAME                                  = 0xDC44,
  USB_MTP_OBJECT_PROP_CREATED_BY                            = 0xDC45,
  USB_MTP_OBJECT_PROP_ARTIST                                = 0xDC46,
  USB_MTP_OBJECT_PROP_DATE_AUTHORED                         = 0xDC47,
  USB_MTP_OBJECT_PROP_DESCRIPTION                           = 0xDC48,
  USB_MTP_OBJECT_PROP_URL_REFERENCE                         = 0xDC49,
  USB_MTP_OBJECT_PROP_LANGUAGELOCALE                        = 0xDC4A,
  USB_MTP_OBJECT_PROP_COPYRIGHT_INFORMATION                 = 0xDC4B,
  USB_MTP_OBJECT_PROP_SOURCE                                = 0xDC4C,
  USB_MTP_OBJECT_PROP_ORIGIN_LOCATION                       = 0xDC4D,
  USB_MTP_OBJECT_PROP_DATE_ADDED                            = 0xDC4E,
  USB_MTP_OBJECT_PROP_NON_CONSUMABLE                        = 0xDC4F,
  USB_MTP_OBJECT_PROP_CORRUPTUNPLAYABLE                     = 0xDC50,
  USB_MTP_OBJECT_PROP_PRODUCERSERIALNUMBER                  = 0xDC51,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_FORMAT          = 0xDC81,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_SIZE            = 0xDC82,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_HEIGHT          = 0xDC83,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_WIDTH           = 0xDC84,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_DURATION        = 0xDC85,
  USB_MTP_OBJECT_PROP_REPRESENTATIVE_SAMPLE_DATA            = 0xDC86,
  USB_MTP_OBJECT_PROP_WIDTH                                 = 0xDC87,
  USB_MTP_OBJECT_PROP_HEIGHT                                = 0xDC88,
  USB_MTP_OBJECT_PROP_DURATION                              = 0xDC89,
  USB_MTP_OBJECT_PROP_RATING                                = 0xDC8A,
  USB_MTP_OBJECT_PROP_TRACK                                 = 0xDC8B,
  USB_MTP_OBJECT_PROP_GENRE                                 = 0xDC8C,
  USB_MTP_OBJECT_PROP_CREDITS                               = 0xDC8D,
  USB_MTP_OBJECT_PROP_LYRICS                                = 0xDC8E,
  USB_MTP_OBJECT_PROP_SUBSCRIPTION_CONTENT_ID               = 0xDC8F,
  USB_MTP_OBJECT_PROP_PRODUCED_BY                           = 0xDC90,
  USB_MTP_OBJECT_PROP_USE_COUNT                             = 0xDC91,
  USB_MTP_OBJECT_PROP_SKIP_COUNT                            = 0xDC92,
  USB_MTP_OBJECT_PROP_LAST_ACCESSED                         = 0xDC93,
  USB_MTP_OBJECT_PROP_PARENTAL_RATING                       = 0xDC94,
  USB_MTP_OBJECT_PROP_META_GENRE                            = 0xDC95,
  USB_MTP_OBJECT_PROP_COMPOSER                              = 0xDC96,
  USB_MTP_OBJECT_PROP_EFFECTIVE_RATING                      = 0xDC97,
  USB_MTP_OBJECT_PROP_SUBTITLE                              = 0xDC98,
  USB_MTP_OBJECT_PROP_ORIGINAL_RELEASE_DATE                 = 0xDC99,
  USB_MTP_OBJECT_PROP_ALBUM_NAME                            = 0xDC9A,
  USB_MTP_OBJECT_PROP_ALBUM_ARTIST                          = 0xDC9B,
  USB_MTP_OBJECT_PROP_MOOD                                  = 0xDC9C,
  USB_MTP_OBJECT_PROP_DRM_STATUS                            = 0xDC9D,
  USB_MTP_OBJECT_PROP_SUB_DESCRIPTION                       = 0xDC9E,
  USB_MTP_OBJECT_PROP_IS_CROPPED                            = 0xDCD1,
  USB_MTP_OBJECT_PROP_IS_COLOUR_CORRECTED                   = 0xDCD2,
  USB_MTP_OBJECT_PROP_IMAGE_BIT_DEPTH                       = 0xDCD3,
  USB_MTP_OBJECT_PROP_FNUMBER                               = 0xDCD4,
  USB_MTP_OBJECT_PROP_EXPOSURE_TIME                         = 0xDCD5,
  USB_MTP_OBJECT_PROP_EXPOSURE_INDEX                        = 0xDCD6,
  USB_MTP_OBJECT_PROP_TOTAL_BITRATE                         = 0xDE91,
  USB_MTP_OBJECT_PROP_BITRATE_TYPE                          = 0xDE92,
  USB_MTP_OBJECT_PROP_SAMPLE_RATE                           = 0xDE93,
  USB_MTP_OBJECT_PROP_NUMBER_OF_CHANNELS                    = 0xDE94,
  USB_MTP_OBJECT_PROP_AUDIO_BITDEPTH                        = 0xDE95,
  USB_MTP_OBJECT_PROP_SCAN_TYPE                             = 0xDE97,
  USB_MTP_OBJECT_PROP_AUDIO_WAVE_CODEC                      = 0xDE99,
  USB_MTP_OBJECT_PROP_AUDIO_BITRATE                         = 0xDE9A,
  USB_MTP_OBJECT_PROP_VIDEO_FOURCC_CODEC                    = 0xDE9B,
  USB_MTP_OBJECT_PROP_VIDEO_BITRATE                         = 0xDE9C,
  USB_MTP_OBJECT_PROP_FRAMES_PER_THOUSAND_SECONDS           = 0xDE9D,
  USB_MTP_OBJECT_PROP_KEYFRAME_DISTANCE                     = 0xDE9E,
  USB_MTP_OBJECT_PROP_BUFFER_SIZE                           = 0xDE9F,
  USB_MTP_OBJECT_PROP_ENCODING_QUALITY                      = 0xDEA0,
  USB_MTP_OBJECT_PROP_ENCODING_PROFILE                      = 0xDEA1,
  USB_MTP_OBJECT_PROP_DISPLAY_NAME                          = 0xDCE0,
  USB_MTP_OBJECT_PROP_BODY_TEXT                             = 0xDCE1,
  USB_MTP_OBJECT_PROP_SUBJECT                               = 0xDCE2,
  USB_MTP_OBJECT_PROP_PRIORITY                              = 0xDCE3,
  USB_MTP_OBJECT_PROP_GIVEN_NAME                            = 0xDD00,
  USB_MTP_OBJECT_PROP_MIDDLE_NAMES                          = 0xDD01,
  USB_MTP_OBJECT_PROP_FAMILY_NAME                           = 0xDD02,
  USB_MTP_OBJECT_PROP_PREFIX                                = 0xDD03,
  USB_MTP_OBJECT_PROP_SUFFIX                                = 0xDD04,
  USB_MTP_OBJECT_PROP_PHONETIC_GIVEN_NAME                   = 0xDD05,
  USB_MTP_OBJECT_PROP_PHONETIC_FAMILY_NAME                  = 0xDD06,
  USB_MTP_OBJECT_PROP_EMAIL_PRIMARY                         = 0xDD07,
  USB_MTP_OBJECT_PROP_EMAIL_PERSONAL_1                      = 0xDD08,
  USB_MTP_OBJECT_PROP_EMAIL_PERSONAL_2                      = 0xDD09,
  USB_MTP_OBJECT_PROP_EMAIL_BUSINESS_1                      = 0xDD0A,
  USB_MTP_OBJECT_PROP_EMAIL_BUSINESS_2                      = 0xDD0B,
  USB_MTP_OBJECT_PROP_EMAIL_OTHERS                          = 0xDD0C,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_PRIMARY                  = 0xDD0D,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_PERSONAL                 = 0xDD0E,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_PERSONAL_2               = 0xDD0F,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_BUSINESS                 = 0xDD10,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_BUSINESS_2               = 0xDD11,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_MOBILE                   = 0xDD12,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_MOBILE_2                 = 0xDD13,
  USB_MTP_OBJECT_PROP_FAX_NUMBER_PRIMARY                    = 0xDD14,
  USB_MTP_OBJECT_PROP_FAX_NUMBER_PERSONAL                   = 0xDD15,
  USB_MTP_OBJECT_PROP_FAX_NUMBER_BUSINESS                   = 0xDD16,
  USB_MTP_OBJECT_PROP_PAGER_NUMBER                          = 0xDD17,
  USB_MTP_OBJECT_PROP_PHONE_NUMBER_OTHERS                   = 0xDD18,
  USB_MTP_OBJECT_PROP_PRIMARY_WEB_ADDRESS                   = 0xDD19,
  USB_MTP_OBJECT_PROP_PERSONAL_WEB_ADDRESS                  = 0xDD1A,
  USB_MTP_OBJECT_PROP_BUSINESS_WEB_ADDRESS                  = 0xDD1B,
  USB_MTP_OBJECT_PROP_INSTANT_MESSENGER_ADDRESS             = 0xDD1C,
  USB_MTP_OBJECT_PROP_INSTANT_MESSENGER_ADDRESS_2           = 0xDD1D,
  USB_MTP_OBJECT_PROP_INSTANT_MESSENGER_ADDRESS_3           = 0xDD1E,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_FULL          = 0xDD1F,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_LINE_1        = 0xDD20,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_LINE_2        = 0xDD21,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_CITY          = 0xDD22,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_REGION        = 0xDD23,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_POSTAL_CODE   = 0xDD24,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_PERSONAL_COUNTRY       = 0xDD25,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_FULL          = 0xDD26,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_LINE_1        = 0xDD27,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_LINE_2        = 0xDD28,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_CITY          = 0xDD29,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_REGION        = 0xDD2A,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE   = 0xDD2B,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_BUSINESS_COUNTRY       = 0xDD2C,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_FULL             = 0xDD2D,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_LINE_1           = 0xDD2E,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_LINE_2           = 0xDD2F,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_CITY             = 0xDD30,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_REGION           = 0xDD31,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_POSTAL_CODE      = 0xDD32,
  USB_MTP_OBJECT_PROP_POSTAL_ADDRESS_OTHER_COUNTRY          = 0xDD33,
  USB_MTP_OBJECT_PROP_ORGANIZATION_NAME                     = 0xDD34,
  USB_MTP_OBJECT_PROP_PHONETIC_ORGANIZATION_NAME            = 0xDD35,
  USB_MTP_OBJECT_PROP_ROLE                                  = 0xDD36,
  USB_MTP_OBJECT_PROP_BIRTHDATE                             = 0xDD37,
  USB_MTP_OBJECT_PROP_MESSAGE_TO                            = 0xDD40,
  USB_MTP_OBJECT_PROP_MESSAGE_CC                            = 0xDD41,
  USB_MTP_OBJECT_PROP_MESSAGE_BCC                           = 0xDD42,
  USB_MTP_OBJECT_PROP_MESSAGE_READ                          = 0xDD43,
  USB_MTP_OBJECT_PROP_MESSAGE_RECEIVED_TIME                 = 0xDD44,
  USB_MTP_OBJECT_PROP_MESSAGE_SENDER                        = 0xDD45,
  USB_MTP_OBJECT_PROP_ACTIVITY_BEGIN_TIME                   = 0xDD50,
  USB_MTP_OBJECT_PROP_ACTIVITY_END_TIME                     = 0xDD51,
  USB_MTP_OBJECT_PROP_ACTIVITY_LOCATION                     = 0xDD52,
  USB_MTP_OBJECT_PROP_ACTIVITY_REQUIRED_ATTENDEES           = 0xDD54,
  USB_MTP_OBJECT_PROP_ACTIVITY_OPTIONAL_ATTENDEES           = 0xDD55,
  USB_MTP_OBJECT_PROP_ACTIVITY_RESOURCES                    = 0xDD56,
  USB_MTP_OBJECT_PROP_ACTIVITY_ACCEPTED                     = 0xDD57,
  USB_MTP_OBJECT_PROP_OWNER                                 = 0xDD5D,
  USB_MTP_OBJECT_PROP_EDITOR                                = 0xDD5E,
  USB_MTP_OBJECT_PROP_WEBMASTER                             = 0xDD5F,
  USB_MTP_OBJECT_PROP_URL_SOURCE                            = 0xDD60,
  USB_MTP_OBJECT_PROP_URL_DESTINATION                       = 0xDD61,
  USB_MTP_OBJECT_PROP_TIME_BOOKMARK                         = 0xDD62,
  USB_MTP_OBJECT_PROP_OBJECT_BOOKMARK                       = 0xDD63,
  USB_MTP_OBJECT_PROP_BYTE_BOOKMARK                         = 0xDD64,
  USB_MTP_OBJECT_PROP_LAST_BUILD_DATE                       = 0xDD70,
  USB_MTP_OBJECT_PROP_TIME_TO_LIVE                          = 0xDD71,
  USB_MTP_OBJECT_PROP_MEDIA_GUID                            = 0xDD72
} USB_MTP_OBJECT_PROPERTIES;                                /*lint !e621 MISRA C:2012 Rule 5.1, required */

typedef enum _USB_MTP_DEVICE_PROPERTIES {
  USB_MTP_DEVICE_PROP_UNDEFINED                      = 0x5000,
  USB_MTP_DEVICE_PROP_BATTERY_LEVEL                  = 0x5001,
  USB_MTP_DEVICE_PROP_FUNCTIONAL_MODE                = 0x5002,
  USB_MTP_DEVICE_PROP_IMAGE_SIZE                     = 0x5003,
  USB_MTP_DEVICE_PROP_COMPRESSION_SETTING            = 0x5004,
  USB_MTP_DEVICE_PROP_WHITE_BALANCE                  = 0x5005,
  USB_MTP_DEVICE_PROP_RGB_GAIN                       = 0x5006,
  USB_MTP_DEVICE_PROP_F_NUMBER                       = 0x5007,
  USB_MTP_DEVICE_PROP_FOCAL_LENGTH                   = 0x5008,
  USB_MTP_DEVICE_PROP_FOCUS_DISTANCE                 = 0x5009,
  USB_MTP_DEVICE_PROP_FOCUS_MODE                     = 0x500A,
  USB_MTP_DEVICE_PROP_EXPOSURE_METERING_MODE         = 0x500B,
  USB_MTP_DEVICE_PROP_FLASH_MODE                     = 0x500C,
  USB_MTP_DEVICE_PROP_EXPOSURE_TIME                  = 0x500D,
  USB_MTP_DEVICE_PROP_EXPOSURE_PROGRAM_MODE          = 0x500E,
  USB_MTP_DEVICE_PROP_EXPOSURE_INDEX                 = 0x500F,
  USB_MTP_DEVICE_PROP_EXPOSURE_BIAS_COMPENSATION     = 0x5010,
  USB_MTP_DEVICE_PROP_DATETIME                       = 0x5011,
  USB_MTP_DEVICE_PROP_CAPTURE_DELAY                  = 0x5012,
  USB_MTP_DEVICE_PROP_STILL_CAPTURE_MODE             = 0x5013,
  USB_MTP_DEVICE_PROP_CONTRAST                       = 0x5014,
  USB_MTP_DEVICE_PROP_SHARPNESS                      = 0x5015,
  USB_MTP_DEVICE_PROP_DIGITAL_ZOOM                   = 0x5016,
  USB_MTP_DEVICE_PROP_EFFECT_MODE                    = 0x5017,
  USB_MTP_DEVICE_PROP_BURST_NUMBER                   = 0x5018,
  USB_MTP_DEVICE_PROP_BURST_INTERVAL                 = 0x5019,
  USB_MTP_DEVICE_PROP_TIMELAPSE_NUMBER               = 0x501A,
  USB_MTP_DEVICE_PROP_TIMELAPSE_INTERVAL             = 0x501B,
  USB_MTP_DEVICE_PROP_FOCUS_METERING_MODE            = 0x501C,
  USB_MTP_DEVICE_PROP_UPLOAD_URL                     = 0x501D,
  USB_MTP_DEVICE_PROP_ARTIST                         = 0x501E,
  USB_MTP_DEVICE_PROP_COPYRIGHT_INFO                 = 0x501F,
  USB_MTP_DEVICE_PROP_SYNCHRONIZATION_PARTNER        = 0xD401,
  USB_MTP_DEVICE_PROP_DEVICE_FRIENDLY_NAME           = 0xD402,
  USB_MTP_DEVICE_PROP_VOLUME                         = 0xD403,
  USB_MTP_DEVICE_PROP_SUPPORTEDFORMATSORDERED        = 0xD404,
  USB_MTP_DEVICE_PROP_DEVICEICON                     = 0xD405,
  USB_MTP_DEVICE_PROP_PLAYBACK_RATE                  = 0xD410,
  USB_MTP_DEVICE_PROP_PLAYBACK_OBJECT                = 0xD411,
  USB_MTP_DEVICE_PROP_PLAYBACK_CONTAINER             = 0xD412,
  USB_MTP_DEVICE_PROP_SESSION_INITIATOR_VERSION_INFO = 0xD406,
  USB_MTP_DEVICE_PROP_PERCEIVED_DEVICE_TYPE          = 0xD407
} USB_MTP_DEVICE_PROPERTIES;

typedef enum _USB_MTP_OPERATION_CODES {
  /*lint --e{621} MISRA C:2012 Rule 5.1, required */
  USB_MTP_OPERATION_GET_DEVICE_INFO              = 0x1001,
  USB_MTP_OPERATION_OPEN_SESSION                 = 0x1002,
  USB_MTP_OPERATION_CLOSE_SESSION                = 0x1003,
  USB_MTP_OPERATION_GET_STORAGE_IDS              = 0x1004,
  USB_MTP_OPERATION_GET_STORAGE_INFO             = 0x1005,
  USB_MTP_OPERATION_GET_NUM_OBJECTS              = 0x1006,
  USB_MTP_OPERATION_GET_OBJECT_HANDLES           = 0x1007,
  USB_MTP_OPERATION_GET_OBJECT_INFO              = 0x1008,
  USB_MTP_OPERATION_GET_OBJECT                   = 0x1009,
  USB_MTP_OPERATION_DELETE_OBJECT                = 0x100B,
  USB_MTP_OPERATION_SEND_OBJECT_INFO             = 0x100C,
  USB_MTP_OPERATION_SEND_OBJECT                  = 0x100D,
  USB_MTP_OPERATION_FORMAT_STORE                 = 0x100F,
  USB_MTP_OPERATION_RESET_DEVICE                 = 0x1010,
  USB_MTP_OPERATION_GET_DEVICE_PROP_DESC         = 0x1014,
  USB_MTP_OPERATION_GET_DEVICE_PROP_VALUE        = 0x1015,
  USB_MTP_OPERATION_SET_DEVICE_PROP_VALUE        = 0x1016,
  USB_MTP_OPERATION_RESET_DEVICE_PROP_VALUE      = 0x1017,
  USB_MTP_OPERATION_TERMINATE_OPEN_CAPTURE       = 0x1018,
  USB_MTP_OPERATION_MOVE_OBJECT                  = 0x1019,
  USB_MTP_OPERATION_COPY_OBJECT                  = 0x101A,
  USB_MTP_OPERATION_GET_PARTIAL_OBJECT           = 0x101B,
  USB_MTP_OPERATION_INITIATE_OPEN_CAPTURE        = 0x101C,
  USB_MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED   = 0x9801,
  USB_MTP_OPERATION_GET_OBJECT_PROP_DESC         = 0x9802,
  USB_MTP_OPERATION_GET_OBJECT_PROP_VALUE        = 0x9803,
  USB_MTP_OPERATION_SET_OBJECT_PROP_VALUE        = 0x9804,
  USB_MTP_OPERATION_GET_OBJECT_PROPLIST          = 0x9805,
  USB_MTP_OPERATION_GETSERVICEIDS                = 0x9301,
  USB_MTP_OPERATION_GETSERVICEINFO               = 0x9302,
  USB_MTP_OPERATION_GETSERVICECAPABILITIES       = 0x9303,
  USB_MTP_OPERATION_GETSERVICEPROPDESC           = 0x9304,
  USB_MTP_OPERATION_GETSERVICEPROPLIST           = 0x9305,
  USB_MTP_OPERATION_SETSERVICEPROPLIST           = 0x9306,
  USB_MTP_OPERATION_UPDATEOBJECTPROPLIST         = 0x9307,
  USB_MTP_OPERATION_DELETEOBJECTPROPLIST         = 0x9308,
  USB_MTP_OPERATION_DELETESERVICEPROPLIST        = 0x9309,
  USB_MTP_OPERATION_GETFORMATCAPABILITIES        = 0x930A,

  USB_MTP_OPERATION_ANDROID_BEGIN_EDIT_OBJECT    = 0x95C4, //Must be called before using SendPartialObject and TruncateObject
  USB_MTP_OPERATION_ANDROID_END_EDIT_OBJECT      = 0x95C5, // Called to commit changes made by SendPartialObject and TruncateObject
  USB_MTP_OPERATION_ANDROID_GET_PARTIAL_OBJECT64 = 0x95C1, // Same as GetPartialObject, but with 64 bit offset
  USB_MTP_OPERATION_ANDROID_SEND_PARTIAL_OBJECT  = 0x95C2, //Same as GetPartialObject64, but copying host to device
  USB_MTP_OPERATION_ANDROID_TRUNCATE_OBJECT      = 0x95C3, // Truncates file to 64 bit length
  USB_MTP_OPERATION_SEGGER_WRITE                 = 0x9750, // Custom vendor command, transfers arbitary data, data size specified by first command parameter; Host writes, device reads.
  USB_MTP_OPERATION_SEGGER_READ                  = 0x9751  // Custom vendor command, transfers arbitary data, data size specified by first command parameter; Host reads, device writes.
} USB_MTP_OPERATION_CODES;                       /*lint !e621 MISRA C:2012 Rule 5.1, required */

/*********************************************************************
*
*       Response codes
*/
typedef enum _USB_MTP_RESPONSE_CODES {
  /*lint --e{621} MISRA C:2012 Rule 5.1, required */
  USB_MTP_RESPONSE_OK                                     = 0x2001,
  USB_MTP_RESPONSE_GENERAL_ERROR                          = 0x2002,
  USB_MTP_RESPONSE_PARAMETER_NOT_SUPPORTED                = 0x2006,
  USB_MTP_RESPONSE_INCOMPLETE_TRANSFER                    = 0x2007,
  USB_MTP_RESPONSE_INVALID_STORAGE_ID                     = 0x2008,
  USB_MTP_RESPONSE_INVALID_OBJECT_HANDLE                  = 0x2009,
  USB_MTP_RESPONSE_DEVICEPROP_NOT_SUPPORTED               = 0x200A,
  USB_MTP_RESPONSE_STORE_FULL                             = 0x200C,
  USB_MTP_RESPONSE_ACCESS_DENIED                          = 0x200F,
  USB_MTP_RESPONSE_STORE_NOT_AVAILABLE                    = 0x2013,
  USB_MTP_RESPONSE_SPECIFICATION_BY_FORMAT_NOT_SUPPORTED  = 0x2014,
  USB_MTP_RESPONSE_NO_VALID_OBJECT_INFO                   = 0x2015,
  USB_MTP_RESPONSE_DEVICE_BUSY                            = 0x2019,
  USB_MTP_RESPONSE_INVALID_PARENT_OBJECT                  = 0x201A,
  USB_MTP_RESPONSE_INVALID_PARAMETER                      = 0x201D,
  USB_MTP_RESPONSE_SESSION_ALREADY_OPEN                   = 0x201E,
  USB_MTP_RESPONSE_TRANSACTION_CANCELLED                  = 0x201F,
  USB_MTP_RESPONSE_INVALID_OBJECT_PROP_CODE               = 0xA801,
  USB_MTP_RESPONSE_SPECIFICATION_BY_GROUP_UNSUPPORTED     = 0xA807,
  USB_MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED              = 0xA80A
} USB_MTP_RESPONSE_CODES;                                 /*lint !e621 MISRA C:2012 Rule 5.1, required */

/*********************************************************************
*
*       Object format codes
*/
typedef enum _USB_MTP_OBJECT_FORMAT {
  /*lint --e{621} MISRA C:2012 Rule 5.1, required */
  USB_MTP_OBJECT_FORMAT_UNDEFINED             = 0x3000,  //Undefined object
  USB_MTP_OBJECT_FORMAT_ASSOCIATION           = 0x3001,  //Association (for example, a folder)
  USB_MTP_OBJECT_FORMAT_SCRIPT                = 0x3002,  //Device model-specific script
  USB_MTP_OBJECT_FORMAT_EXECUTABLE            = 0x3003,  //Device model-specific binary executable
  USB_MTP_OBJECT_FORMAT_TEXT                  = 0x3004,  //Text file
  USB_MTP_OBJECT_FORMAT_HTML                  = 0x3005,  //Hypertext Markup Language file (text)
  USB_MTP_OBJECT_FORMAT_DPOF                  = 0x3006,  //Digital Print Order Format file (text)
  USB_MTP_OBJECT_FORMAT_AIFF                  = 0x3007,  //Audio clip
  USB_MTP_OBJECT_FORMAT_WAV                   = 0x3008,  //Audio clip
  USB_MTP_OBJECT_FORMAT_MP3                   = 0x3009,  //Audio clip
  USB_MTP_OBJECT_FORMAT_AVI                   = 0x300A,  //Video clip
  USB_MTP_OBJECT_FORMAT_MPEG                  = 0x300B,  //Video clip
  USB_MTP_OBJECT_FORMAT_ASF                   = 0x300C,  //Microsoft Advanced Streaming Format (video)
  USB_MTP_OBJECT_FORMAT_DEFINED               = 0x3800,  //Unknown image object
  USB_MTP_OBJECT_FORMAT_EXIF_JPEG             = 0x3801,  //Exchangeable File Format, JEIDA standard
  USB_MTP_OBJECT_FORMAT_TIFF_EP               = 0x3802,  //Tag Image File Format for Electronic Photography
  USB_MTP_OBJECT_FORMAT_FLASHPIX              = 0x3803,  //Structured Storage Image Format
  USB_MTP_OBJECT_FORMAT_BMP                   = 0x3804,  //Microsoft Windows Bitmap file
  USB_MTP_OBJECT_FORMAT_CIFF                  = 0x3805,  //Canon Camera Image File Format
  USB_MTP_OBJECT_FORMAT_UNDEFINED_RESERVED0   = 0x3806,  //Reserved
  USB_MTP_OBJECT_FORMAT_GIF                   = 0x3807,  //Graphics Interchange Format
  USB_MTP_OBJECT_FORMAT_JFIF                  = 0x3808,  //JPEG File Interchange Format
  USB_MTP_OBJECT_FORMAT_CD                    = 0x3809,  //PhotoCD Image Pac
  USB_MTP_OBJECT_FORMAT_PICT                  = 0x380A,  //Quickdraw Image Format
  USB_MTP_OBJECT_FORMAT_PNG                   = 0x380B,  //Portable Network Graphics
  USB_MTP_OBJECT_FORMAT_UNDEFINED_RESERVED1   = 0x380C,  //Reserved
  USB_MTP_OBJECT_FORMAT_TIFF                  = 0x380D,  //Tag Image File Format
  USB_MTP_OBJECT_FORMAT_TIFF_IT               = 0x380E,  //Tag Image File Format for Information Technology (graphic arts)
  USB_MTP_OBJECT_FORMAT_JP2                   = 0x380F,  //JPEG2000 Baseline File Format
  USB_MTP_OBJECT_FORMAT_JPX                   = 0x3810,  //JPEG2000 Extended File Format
  USB_MTP_OBJECT_FORMAT_UNDEFINED_FIRMWARE    = 0xB802,  //
  USB_MTP_OBJECT_FORMAT_WINDOWS_IMAGE_FORMAT  = 0xB881,  //
  USB_MTP_OBJECT_FORMAT_UNDEFINED_AUDIO       = 0xB900,  //
  USB_MTP_OBJECT_FORMAT_WMA                   = 0xB901,  //
  USB_MTP_OBJECT_FORMAT_OGG                   = 0xB902,  //
  USB_MTP_OBJECT_FORMAT_AAC                   = 0xB903,  //
  USB_MTP_OBJECT_FORMAT_AUDIBLE               = 0xB904,  //
  USB_MTP_OBJECT_FORMAT_FLAC                  = 0xB906,  //
  USB_MTP_OBJECT_FORMAT_UNDEFINED_VIDEO       = 0xB980,  //
  USB_MTP_OBJECT_FORMAT_WMV                   = 0xB981,  //
  USB_MTP_OBJECT_FORMAT_MP4_CONTAINER         = 0xB982,  //ISO 14496-1
  USB_MTP_OBJECT_FORMAT_MP2                   = 0xB983,  //
  USB_MTP_OBJECT_FORMAT_3GP_CONTAINER         = 0xB984   //3GPP file format. http://www.3gpp.org/ftp/Specs/html-info/26244.htm
} USB_MTP_OBJECT_FORMAT;                      /*lint !e621 MISRA C:2012 Rule 5.1, required */

/*********************************************************************
*
*       USB_MTP_STORAGE_INIT
*
*  Description
*    Initializes the storage medium.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath    : Pointer to a USB_MTP_INST_DATA_DRIVER structure that contains all information
*                   that is necessary for the driver initialization. For detailed information
*                   about the USB_MTP_INST_DATA_DRIVER structure, refer to USB_MTP_INST_DATA_DRIVER.
*
*  Additional information
*    This function is called when the storage driver is added to emUSB-Device-MTP.
*    It is the first function of the storage driver to be called.
*/
typedef void (USB_MTP_STORAGE_INIT)                   (U8 Unit, const USB_MTP_INST_DATA_DRIVER * pDriverData);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_INFO
*
*  Description
*    Returns information about storage medium such as capacity and available free space.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pStorageInfo : Pointer to a USB_MTP_STORAGE_INFO structure. For detailed information about
*                   the USB_MTP_STORAGE_INFO structure, refer to USB_MTP_STORAGE_INFO.
*
*  Additional information
*    Typically, this function is called immediately after the device is connected
*    to USB host when the USB host requests information about the available storage mediums.
*/
typedef void (USB_MTP_STORAGE_GET_INFO)               (U8 Unit, USB_MTP_STORAGE_INFO * pStorageInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_FIND_FIRST_FILE
*
*  Description
*    Returns information about the first file in a specified directory.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pDirPath     : Full path to the directory to be searched.
*    pFileInfo    : [OUT] Information about the file/directory found.
*
*  Return value
*    == 0: File/directory found
*    == 1: No more files/directories found
*     < 0: An error occurred
*
*  Additional information
*    The "." and ".." directory entries which are relevant only for the file system should be skipped.
*/
typedef int  (USB_MTP_STORAGE_FIND_FIRST_FILE)        (U8 Unit, const char * pDirPath, USB_MTP_FILE_INFO * pFileInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_FIND_NEXT_FILE
*
*  Description
*    Moves to next file and returns information about it.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pFileInfo    : [OUT] Information about the file/directory found.
*
*  Return value
*    == 0: File/directory found
*    == 1: No more files/directories found
*     < 0: An error occurred
*
*  Additional information
*    The "." and ".." directory entries which are relevant only for the file system should be skipped.
*/
typedef int  (USB_MTP_STORAGE_FIND_NEXT_FILE)         (U8 Unit, USB_MTP_FILE_INFO * pFileInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_OPEN_FILE
*
*  Description
*    Opens a file for reading.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath    : [IN] Full path to file.
*
*  Return value
*    == 0: File opened
*    != 0: An error occurred
*
*  Additional information
*    This function is called at the beginning of a file read operation.
*    It is followed by one or more calls to USB_MTP_STORAGE_READ_FROM_FILE.
*    At the end of data transfer the MTP module closes the file by calling USB_MTP_STORAGE_CLOSE_FILE.
*    If the file does not exists an error should be returned.
*    The MTP module opens only one file at a time.
*/
typedef int  (USB_MTP_STORAGE_OPEN_FILE)              (U8 Unit, const char * pFilePath);

/*********************************************************************
*
*       USB_MTP_STORAGE_CREATE_FILE
*
*  Description
*    Opens a file for writing.
*
*  Parameters
*    Unit         : Logical unit number. Specifies for which storage medium the function is called.
*    pDirPath     : [IN] Full path to directory where the file should be created.
*    pFileInfo    : [IN] Information about the file to be created. pFileName points to the name of the file.
*                   [OUT] pFilePath points to full path of created file, pFileName points to the beginning of file name in pFilePath.
*
*  Return value
*    == 0: File created and opened
*    != 0: An error occurred
*
*  Additional information
*    This function is called at the beginning of a file write operation.
*    The name of the file is specified in the pFileName filed of pFileInfo.
*    If the file exists it should be truncated to zero length.
*    When a file is created, the call to USB_MTP_STORAGE_CREATE_FILE is followed
*    by one or more calls to USB_MTP_STORAGE_WRITE_TO_FILE.
*    If CreationTime and LastWriteTime in pFileInfo are not zero,
*    these should be used instead of the time stamps generated by the file system.
*/
typedef int  (USB_MTP_STORAGE_CREATE_FILE)            (U8 Unit, const char * pDirPath, USB_MTP_FILE_INFO * pFileInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_READ_FROM_FILE
*
*  Description
*    Reads data from the current file.
*
*  Parameters
*    Unit     : Logical unit number. Specifies for which storage medium the function is called.
*    Off      : Byte offset where to read from.
*    pData    : [OUT] Data read from file.
*    NumBytes : Number of bytes to read from file.
*
*  Return value
*    == 0: Data read from file
*    != 0: An error occurred
*
*  Additional information
*    The function reads data from the file opened by USB_MTP_STORAGE_OPEN_FILE.
*/
typedef int  (USB_MTP_STORAGE_READ_FROM_FILE)         (U8 Unit, U32 Off, void * pData, U32 NumBytes);

/*********************************************************************
*
*       USB_MTP_STORAGE_WRITE_TO_FILE
*
*  Description
*    Writes data to current file.
*
*  Parameters
*    Unit     : Logical unit number. Specifies for which storage medium the function is called.
*    Off      : Byte offset where to read from.
*    pData    : [IN] Data to write to file.
*    NumBytes : Number of bytes to write to file.
*
*  Return value
*    == 0: Data written to file
*    != 0: An error occurred
*
*  Additional information
*    The function writes data to file opened by USB_MTP_STORAGE_CREATE_FILE.
*/
typedef int  (USB_MTP_STORAGE_WRITE_TO_FILE)          (U8 Unit, U32 Off, const void * pData, U32 NumBytes);

/*********************************************************************
*
*       USB_MTP_STORAGE_CLOSE_FILE
*
*  Description
*    Closes the current file.
*
*  Parameters
*    Unit     : Logical unit number. Specifies for which storage medium the function is called.
*
*  Return value
*    == 0: File closed.
*    != 0: An error occurred
*
*  Additional information
*    The function closes the file opened by USB_MTP_STORAGE_CREATE_FILE or USB_MTP_STORAGE_OPEN_FILE.
*/
typedef int  (USB_MTP_STORAGE_CLOSE_FILE)             (U8 Unit);

/*********************************************************************
*
*       USB_MTP_STORAGE_REMOVE_FILE
*
*  Description
*    Removes a file/directory from the storage medium.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file/directory to be removed
*
*  Return value
*    == 0: File removed.
*    != 0: An error occurred
*/
typedef int  (USB_MTP_STORAGE_REMOVE_FILE)            (U8 Unit, const char * pFilePath);

/*********************************************************************
*
*       USB_MTP_STORAGE_CREATE_DIR
*
*  Description
*    Creates a directory on the storage medium.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pDirPath   : [IN] Full path to directory where the directory should be created.
*    pFileInfo  : [IN] Information about the directory to be created. pFileName points to the directory name.
*                 [OUT] pFilePath points to full path of directory, pFileName points to the beginning of directory name in pFilePath
*
*  Return value
*    == 0: Directory created.
*    != 0: An error occurred
*
*  Additional information
*    If CreationTime and LastWriteTime in pFileInfo are not available,
*    zero should be used instead of the time stamps generated by the file system.
*/
typedef int  (USB_MTP_STORAGE_CREATE_DIR)             (U8 Unit, const char * pDirPath, USB_MTP_FILE_INFO * pFileInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_REMOVE_DIR
*
*  Description
*    Removes a directory and its contents from the storage medium.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pDirPath   : [IN] Full path to directory to be removed.
*
*  Return value
*    == 0: Directory removed.
*    != 0: An error occurred
*
*  Additional information
*    The function should remove the directory and the entire file tree under it.
*/
typedef int  (USB_MTP_STORAGE_REMOVE_DIR)             (U8 Unit, const char * pDirPath);

/*********************************************************************
*
*       USB_MTP_STORAGE_FORMAT
*
*  Description
*    Initializes the storage medium.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*
*  Return value
*    == 0: Storage medium initialized.
*    != 0: An error occurred
*
*  Additional information
*    The file system layer has to differentiate between two cases,
*    one where the MTP root directory is the same as the root directory
*    of the file system and one where it is only a subdirectory of
*    the file system. If pRootDir which was configured in the call
*    to USB_MTP_STORAGE_INIT, points to a subdirectory of the file system,
*    the storage medium should not be formatted.
*    Instead, all the files and directories underneath pRootDir should be removed.
*/
typedef int  (USB_MTP_STORAGE_FORMAT)                 (U8 Unit);

/*********************************************************************
*
*       USB_MTP_STORAGE_RENAME_FILE
*
*  Description
*    Changes the name of a file or directory.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFileInfo  : Pointer to a USB_MTP_FILE_INFO structure.
*                 [IN] Information about the file/directory to be renamed.
*                 pFilePath member points to the full path and pFileName points to the new name.
*                 [OUT] pFilePath member points to full path of file/directory with the new name,
*                 pFileName points to the beginning of file/directory name in pFilePath.
*                 The other structure fields should also be filled.
*
*  Return value
*    == 0: File/directory renamed.
*    != 0: An error occurred
*
*  Additional information
*    Only the name of the file/directory should be changed.
*    The path to parent directory should remain the same.
*/
typedef int  (USB_MTP_STORAGE_RENAME_FILE)            (U8 Unit, USB_MTP_FILE_INFO * pFileInfo);

/*********************************************************************
*
*       USB_MTP_STORAGE_DEINIT
*
*  Description
*    De-initializes the storage medium.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*
*  Additional information
*    Typically called when the application calls USBD_Stop()
*    to de-initialize emUSB-Device.
*/
typedef void (USB_MTP_STORAGE_DEINIT)                 (U8 Unit);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_FILE_ATTRIBUTES
*
*  Description
*    Returns the attributes of a file or directory.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    pMask      : [OUT] The bitmask of the attributes.
*
*  Return value
*    == 0: Information returned.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*    For the list of supported attributes refer to USB_MTP_FILE_INFO.
*/
typedef int  (USB_MTP_STORAGE_GET_FILE_ATTRIBUTES)    (U8 Unit, const char * pFilePath, U8 * pMask);

/*********************************************************************
*
*       USB_MTP_STORAGE_MODIFY_FILE_ATTRIBUTES
*
*  Description
*    Sets and clears file attributes.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    SetMask    : The bitmask of the attributes which should be set.
*    ClrMask    : The bitmask of the attributes which should be cleared.
*
*  Return value
*    == 0: Attributes modified.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*    For the list of supported attributes refer to USB_MTP_FILE_INFO.
*/
typedef int  (USB_MTP_STORAGE_MODIFY_FILE_ATTRIBUTES) (U8 Unit, const char * pFilePath, U8 SetMask, U8 ClrMask);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_FILE_CREATION_TIME
*
*  Description
*    Returns the creation time of file or directory.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    pTime      : [OUT] The creation time.
*
*  Return value
*    == 0: Creation time returned.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*    For the list of supported attributes refer to USB_MTP_FILE_INFO.
*/
typedef int  (USB_MTP_STORAGE_GET_FILE_CREATION_TIME) (U8 Unit, const char * pFilePath, U32 * pTime);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_FILELAST_WRITE_TIME
*
*  Description
*    Returns the time when the file or directory was last modified.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    pTime      : [OUT] The modification time.
*
*  Return value
*    == 0: Modification time returned.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*    For the list of supported attributes refer to USB_MTP_FILE_INFO.
*/
typedef int  (USB_MTP_STORAGE_GET_FILELAST_WRITE_TIME)(U8 Unit, const char * pFilePath, U32 * pTime);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_FILE_ID
*
*  Description
*    Returns an ID which uniquely identifies the file or directory.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    pId        : [OUT] The unique ID of file or directory.
*                 Should point to a byte array MTP_NUM_BYTES_FILE_ID large.
*
*  Return value
*    == 0: ID returned.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*/
typedef int  (USB_MTP_STORAGE_GET_FILE_ID)            (U8 Unit, const char * pFilePath, U8  * pId);

/*********************************************************************
*
*       USB_MTP_STORAGE_GET_FILE_SIZE
*
*  Description
*    Returns the size of a file in bytes.
*
*  Parameters
*    Unit       : Logical unit number. Specifies for which storage medium the function is called.
*    pFilePath  : Full path to file or directory (0-terminated string).
*    pFileSize  : [OUT] The size of file in bytes.
*
*  Return value
*    == 0: Size of file returned.
*    != 0: An error occurred
*
*  Additional information
*    This function is called only when the compile time switch MTP_SAVE_FILE_INFO is set to 0.
*/
typedef int  (USB_MTP_STORAGE_GET_FILE_SIZE)          (U8 Unit, const char * pFilePath, U32 * pFileSize);


/*********************************************************************
*
*       USB_MTP_STORAGE_API
*
*  Description
*    Structure that contains callbacks to the storage driver.
*/
typedef struct USB_MTP_STORAGE_API {
  USB_MTP_STORAGE_INIT                    * pfInit;                 // Initializes the storage medium.
  USB_MTP_STORAGE_GET_INFO                * pfGetInfo;              // Returns information about the storage medium such as storage capacity
                                                                    // and the available free space.
  USB_MTP_STORAGE_FIND_FIRST_FILE         * pfFindFirstFile;        // Returns information about the first file in a given directory.
  USB_MTP_STORAGE_FIND_NEXT_FILE          * pfFindNextFile;         // Moves to next file and returns information about it.
  USB_MTP_STORAGE_OPEN_FILE               * pfOpenFile;             // Opens an existing file.
  USB_MTP_STORAGE_CREATE_FILE             * pfCreateFile;           // Creates a new file.
  USB_MTP_STORAGE_READ_FROM_FILE          * pfReadFromFile;         // Reads data from the current file.
  USB_MTP_STORAGE_WRITE_TO_FILE           * pfWriteToFile;          // Writes data to current file.
  USB_MTP_STORAGE_CLOSE_FILE              * pfCloseFile;            // Closes the current file.
  USB_MTP_STORAGE_REMOVE_FILE             * pfRemoveFile;           // Removes a file from storage medium.
  USB_MTP_STORAGE_CREATE_DIR              * pfCreateDir;            // Creates a new directory.
  USB_MTP_STORAGE_REMOVE_DIR              * pfRemoveDir;            // Removes a directory from storage medium.
  USB_MTP_STORAGE_FORMAT                  * pfFormat;               // Formats the storage.
  USB_MTP_STORAGE_RENAME_FILE             * pfRenameFile;           // Changes the name of a file or directory.
  USB_MTP_STORAGE_DEINIT                  * pfDeInit;               // De-initializes the storage medium.
  USB_MTP_STORAGE_GET_FILE_ATTRIBUTES     * pfGetFileAttributes;    // Reads the attributes of a file or directory.
  USB_MTP_STORAGE_MODIFY_FILE_ATTRIBUTES  * pfModifyFileAttributes; // Changes the attributes of a file or directory.
  USB_MTP_STORAGE_GET_FILE_CREATION_TIME  * pfGetFileCreationTime;  // Returns the creation time of a file or directory.
  USB_MTP_STORAGE_GET_FILELAST_WRITE_TIME * pfGetFileLastWriteTime; // Returns the time of the last modification made to a file or directory.
  USB_MTP_STORAGE_GET_FILE_ID             * pfGetFileId;            // Returns the unique ID of a file or directory.
  USB_MTP_STORAGE_GET_FILE_SIZE           * pfGetFileSize;          // Returns the size of a file in bytes.
} USB_MTP_STORAGE_API;

/*********************************************************************
*
*       USB_MTP_INST_DATA
*
*  Description
*    Structure which stores the parameters of storage driver.
*
*  Additional information
*    The MTP device returns the sDescription string in the Storage
*    Description parameter and the sVolumeId in the Volume Identifier
*    of the StorageInfo dataset. For more information, refer to MTP specification.
*/
typedef struct USB_MTP_INST_DATA {
  const USB_MTP_STORAGE_API * pAPI;         // Pointer to a structure that holds the storage device driver API.
  const char                * sDescription; // Human-readable string which identifies the storage.
                                            // This string is displayed in Nautilus/Windows Explorer/etc.
  const char                * sVolumeId;    // Unique volume identifier
  USB_MTP_INST_DATA_DRIVER    DriverData;   // Driver data that are passed to the storage driver.
                                            // Refer to USB_MTP_INST_DATA_DRIVER for detailed information
                                            // about how to initialize this structure. This field must be up
                                            // to 256 characters long but only the first 128 are significant
                                            // and these must be unique for all storages of an MTP device.
} USB_MTP_INST_DATA;

/*********************************************************************
*
*       USB_MTP_INFO
*
*  Description
*    Structure that is used when initialising the MTP module.
*/
typedef struct USB_MTP_INFO {
  const char * pManufacturer; // Name of the device manufacturer.
  const char * pModel;        // Model name of the MTP device.
  const char * pDeviceVersion;// Version of the MTP device.
  const char * pSerialNumber; // Serial number of the MTP device.
                              // The serial number should contain exactly
                              // 32 hexadecimal characters.
                              // It must be unique among devices sharing
                              // the same model name and device version
                              // strings. The MTP device returns this
                              // string in the Serial Number field of
                              // the DeviceInfo dataset. For more
                              // information, refer to MTP specification.
} USB_MTP_INFO;

/*********************************************************************
*
*       USB_MTP_INIT_DATA
*
*  Description
*    Structure which stores the parameters of the MTP interface.
*
*  Additional information
*    This structure holds the endpoints that should be used with the
*    MTP interface. Refer to USBD_AddEP() for more information about
*    how to add an endpoint.
*
*    The number of bytes in the pDataBuffer should be a multiple of USB
*    maximum packet size. The number of bytes in the object list depends
*    on the number of files/directories on the storage medium. An object
*    is assigned to each file/directory when the USB host requests
*    the object information for the first time.
*/
typedef struct USB_MTP_INIT_DATA {
  U8     EPIn;                // Endpoint for receiving data from host.
  U8     EPOut;               // Endpoint for sending data to host.
  U8     EPInt;               // Endpoint for sending events to host.
  void * pObjectList;         // Pointer to a memory region where the list of MTP objects is stored.
  U32    NumBytesObjectList;  // Number of bytes allocated for the object list.
  void * pDataBuffer;         // Pointer to a memory region to be used as communication buffer.
  U32    NumBytesDataBuffer;  // Number of bytes allocated for the data buffer.
  USB_MTP_INFO * pMTPInfo;    // Pointer to a USB_MTP_INFO structure. Filling this structure is mandatory.
  //
  // The following fields are used internally by the MTP component.
  //
  U8     InterfaceNum;
  U32    NumBytesAllocated;
  U32    NumObjects;
} USB_MTP_INIT_DATA;

/*********************************************************************
*
*       USB_MTP_STORAGE_HANDLE
*/
typedef U32 USB_MTP_STORAGE_HANDLE;


/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void                    USBD_MTP_Init             (void);
int                     USBD_MTP_Add              (const USB_MTP_INIT_DATA * pInitData);
USB_MTP_STORAGE_HANDLE  USBD_MTP_AddStorage       (const USB_MTP_INST_DATA * pInstData);
void                    USBD_MTP_Task             (void);
void                    USBD_MTP_SendEvent        (USB_MTP_STORAGE_HANDLE hStorage, USB_MTP_EVENT Event, void * pPara);
/*********************************************************************
*
*       Wrapper for emUSB V2 migration
*
**********************************************************************
*/
#define  USB_MTP_Init              USBD_MTP_Init
#define  USB_MTP_Add               USBD_MTP_Add
#define  USB_MTP_AddStorage        USBD_MTP_AddStorage
#define  USB_MTP_Task              USBD_MTP_Task
#define  USB_MTP_SendEvent         USBD_MTP_SendEvent

/*********************************************************************
*
*       Functions defined in the application
*
**********************************************************************
*/
#if USB_V2_V3_MIGRATION_MTP_INFO > 0
const char * USB_MTP_GetManufacturer (void);
const char * USB_MTP_GetModel        (void);
const char * USB_MTP_GetDeviceVersion(void);
const char * USB_MTP_GetSerialNumber (void);
#endif
/*********************************************************************
*
*       Storage drivers
*
**********************************************************************
*/
extern const USB_MTP_STORAGE_API USB_MTP_StorageFS;

/*********************************************************************
*
*       Custom commands
*
**********************************************************************
*/
#ifdef USB_MTP_SUPPORT_SEGGER_CMD
typedef void USB_MTP_SEGGER_WRITE(U32 TransactionId, U32 NumBytesRemaining, U32 * pNumBytesToSend, U8 ** pData);
typedef void USB_MTP_SEGGER_READ(U32 TransactionId, U32 NumBytesRemaining, U32 NumBytesInBuffer, U8 * pData);
void USBD_MTP_SetReadCallback  (USB_MTP_SEGGER_READ * pfRead);
void USBD_MTP_SetWriteCallback (USB_MTP_SEGGER_WRITE * pfWrite);
#endif

#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/

