/**
 ****************************************************************************************
 *
 * @file platform_gap_version.h
 *
 * @brief GAP Version header file
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */



#ifndef _DA14690_GAP_VERSION_H_
#define _DA14690_GAP_VERSION_H_

// Version has the form Major.minor.release
/// Major version. This value is coming from the assigned numbers and corresponds to the
/// version of specification that the host is following.
/// For more information, go to https://www.bluetooth.org/Technical/AssignedNumbers/link_manager.htm
#define BLE_HOST_VERSION_MAJ       8  // From Assigned Numbers

/// Minor version. This value is incremented when new features are added to the host
#define BLE_HOST_VERSION_MIN       2

/// Release number. This value is incremented for every delivery done to the validation
/// team prior to release to customer
#define BLE_HOST_VERSION_REL       14

#endif // _DA14690_GAP_VERSION_H_
