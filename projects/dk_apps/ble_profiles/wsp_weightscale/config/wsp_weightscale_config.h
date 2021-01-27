/**
 ****************************************************************************************
 *
 * @file wsp_weightscale_config.h
 *
 * @brief Application configuration
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef WSP_WEIGHTSCALE_CONFIG_H_
#define WSP_WEIGHTSCALE_CONFIG_H_

#define CFG_MULTIPLE_CLIENTS    (0)     // allow couple of users to connect
#define CFG_UDS_MAX_USERS       (3)     // max number of users in database for UDS
#define CFG_MAX_MEAS_TO_STORE   (25)    // max number of measurements to store for each client who
                                        // is registered and have a proper consent, according to
                                        // specification server should store at least 25 data meas

/* Maximum valid value for user consent */
#define CFG_UDS_MAX_VALID_USER_CONSENT  (9999)

/* Debug functionality - attach new connection to already existing user */
#define CFG_ATTACH_CONN_TO_USER (0)

/*
 * Port and pin which are used to trigger perform notification action
 */
#       define CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PORT      (HW_GPIO_PORT_0)
#       define CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PIN       (HW_GPIO_PIN_6)
#endif /* WSP_WEIGHTSCALE_CONFIG_H_ */
