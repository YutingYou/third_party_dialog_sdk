/**
 ****************************************************************************************
 *
 * @file ams_config.h
 *
 * @brief Application configuration
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AMS_CONFIG_H_
#define AMS_CONFIG_H_

/*
 * Non-zero value enables extended printouts from application, like more detailed information about
 * remote commands and entity updates
 */
#ifndef CFG_VERBOSE_LOG
#       define CFG_VERBOSE_LOG (0)
#endif

/*
 * Both K1 and CTS use P1.6 on DA1468x boards - we need to define CFG_USER_BUTTON_* wrappers.
 */

/*
 * User button port configuration
 */
#ifndef CFG_USER_BUTTON_PORT
#               define CFG_USER_BUTTON_PORT (KEY1_PORT)
#endif

/*
 * User button pin configuration
 */
#ifndef CFG_USER_BUTTON_PIN
#               define CFG_USER_BUTTON_PIN (KEY1_PIN)
#endif

#endif /* AMS_CONFIG_H_ */
