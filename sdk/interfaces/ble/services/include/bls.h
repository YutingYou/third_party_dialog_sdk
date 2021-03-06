/**
 ****************************************************************************************
 *
 * @file bls.h
 *
 * @brief Blood Pressure Service implementation API
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLS_H_
#define BLS_H_

#include <stdbool.h>
#include "ble_service.h"
#include "svc_types.h"

/**
 * \brief Callback function, called when Pressure Measurement indication was changed by the client
 *
 * \param [in] service          service instance
 * \param [in] conn_idx         connection index
 * \param [in] enabled          indication status
 *
 */
typedef void (* bls_measurement_indication_changed_cb_t) (ble_service_t *service, uint16_t conn_idx,
                                                                                     bool enabled);

/**
 * \brief Callback function, called when confirmation to Pressure Measurement indication has been
 * received
 *
 * \param [in] service          service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           indication status
 *
 */
typedef void (* bls_measurement_indication_sent_cb_t) (ble_service_t *service, uint16_t conn_idx,
                                                                                bool status);

/**
 * \brief Callback function, called when Intermediate Cuff Pressure notification was changed by
 * the client
 *
 * \param [in] service          service instance
 * \param [in] conn_idx         connection index
 * \param [in] enabled          notification status
 *
 */
typedef void (* bls_interm_cuff_pressure_notif_changed_cb_t) (ble_service_t *service,
                                                                  uint16_t conn_idx, bool enabled);

/**
 * \brief Callback function, called when a Cuff Pressure notification has been sent to the client
 *
 * \param [in] service          service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           notification status
 *
 */
typedef void (* bls_interm_cuff_pressure_notif_sent_cb_t) (ble_service_t *service,
                                                                uint16_t conn_idx, bool status);

/**
 * \brief BLS application callbacks
 *
 */
typedef struct {
        /** Indication status for Pressure Measurement changed by client */
        bls_measurement_indication_changed_cb_t         meas_indication_changed;
        /** Client response for Pressure Measurement indication */
        bls_measurement_indication_sent_cb_t            meas_indication_sent;
        /** Notification status for Cuff Pressure changed by client */
        bls_interm_cuff_pressure_notif_changed_cb_t     interm_cuff_pressure_notif_changed;
        /** Client response for Cuff Pressure indication */
        bls_interm_cuff_pressure_notif_sent_cb_t        interm_cuff_pressure_notif_sent;
} bls_callbacks_t;

/**
 * \brief BLS Measurement Status fields
 */
#define BLS_MEASUREMENT_STATUS(NAME, VALUE)             ( NAME ## _MASK & (VALUE << NAME ## _POS) )
#define BLS_BODY_MOVEMENT_POS                           ( 0x0000 )
#define BLS_BODY_MOVEMENT_MASK                          ( 0x0001 )
#define BLS_CUFF_FIT_POS                                ( 0x0001 )
#define BLS_CUFF_FIT_MASK                               ( 0x0002 )
#define BLS_IRREGULAR_PULSE_DETECTION_POS               ( 0x0002 )
#define BLS_IRREGULAR_PULSE_DETECTION_MASK              ( 0x0004 )
#define BLS_PULSE_RATE_RANGE_POS                        ( 0x0003 )
#define BLS_PULSE_RATE_RANGE_MASK                       ( 0x0018 )
#define BLS_MEASUREREMENT_POS_DETECTION_POS             ( 0x0005 )
#define BLS_MEASUREREMENT_POS_DETECTION_MASK            ( 0x0020 )

/**
 * \enum bls_body_movement_t
 *
 * \brief Body Movement Detection Flag
 *
 */
typedef enum {
        /** No body movement */
        BLS_BODY_MOVEMENT_NOT_DETECTED  = BLS_MEASUREMENT_STATUS(BLS_BODY_MOVEMENT, 0),
        /** Detect body movement during measurement */
        BLS_BODY_MOVEMENT_DETECTED      = BLS_MEASUREMENT_STATUS(BLS_BODY_MOVEMENT, 1),
} bls_body_movement_t;



/**
 * \enum bls_cuff_fit_t
 *
 * \brief Cuff Fit Detection Flag
 *
 */
typedef enum {
        /** Cuff fits properly */
        BLS_CUFF_FIT_PROPERLY   = BLS_MEASUREMENT_STATUS(BLS_CUFF_FIT, 0),
        /** Cuff too lose */
        BLS_CUFF_FIT_TOO_LOOSE  = BLS_MEASUREMENT_STATUS(BLS_CUFF_FIT, 1),
} bls_cuff_fit_t;


/**
 * \enum bls_irregular_pulse_t
 *
 * \brief Irregular Pulse Detection Flag
 *
 */
typedef enum {
        /** No irregular pulse detected */
        BLS_IRREGULAR_PULSE_NOT_DETECTED        = BLS_MEASUREMENT_STATUS(BLS_CUFF_FIT, 0),
        /** Irregular pulse detected */
        BLS_IRREGULAR_PULSE_DETECTED            = BLS_MEASUREMENT_STATUS(BLS_CUFF_FIT, 1),
} bls_irregular_pulse_t;



/**
 * \enum bls_pulse_rate_range_t
 *
 * \brief Pulse Rate Range Detection Flags
 *
 */
typedef enum {
        /** Pulse rate is within the range */
        BLS_PULSE_RATE_RANGE_WITHIN =   BLS_MEASUREMENT_STATUS(BLS_PULSE_RATE_RANGE, 0),
        /** Pulse rate exceeds upper limit */
        BLS_PULSE_RATE_RANGE_EXCEEDS =  BLS_MEASUREMENT_STATUS(BLS_PULSE_RATE_RANGE, 1),
        /** Pulse rate is less than lower limit */
        BLS_PULSE_RATE_RANGE_LESS =     BLS_MEASUREMENT_STATUS(BLS_PULSE_RATE_RANGE, 2),
} bls_pulse_rate_range_t;
/**
 * \enum bls_measurement_pos_detection_t
 *
 * \brief Measurement Position Detection Flag
 *
 */
typedef enum {
        /** Proper measurement position */
        BLS_MEASUREREMENT_POS_DETECTION_PROPER          = BLS_MEASUREMENT_STATUS(BLS_MEASUREREMENT_POS_DETECTION, 0),
        /** Improper measurement position */
        BLS_MEASUREREMENT_POS_DETECTION_IMPROPER        = BLS_MEASUREMENT_STATUS(BLS_MEASUREREMENT_POS_DETECTION, 1),
} bls_measurement_pos_detection_t;

/**
 * \enum bls_pressure_unit_t
 *
 * \brief Blood Pressure Service Pressure Unit
 *
 */
typedef enum {
        BLS_PRESSURE_UNIT_MMHG                  = 0x00, /**< Blood pressure in mmHg */
        BLS_PRESSURE_UNIT_KPA                   = 0x01, /**< Blood pressure in kPa */
} bls_pressure_unit_t;

/**
 * \brief Blood Pressure Measurement
 *
 */
typedef struct {
        bls_pressure_unit_t      unit;                           /**< Blood pressure unit (mmHg or kPa) */
        svc_ieee11073_float_t    pressure_systolic;              /**< Blood pressure value - Systolic */
        svc_ieee11073_float_t    pressure_diastolic;             /**< Blood pressure value - Diastolic */
        svc_ieee11073_float_t    pressure_map;                   /**< Blood pressure value - Mean Arterial Pressure */
        svc_date_time_t          time_stamp;                     /**< Time of measurement */
        svc_ieee11073_float_t    pulse_rate;                     /**< Pulse rate */
        uint8_t                  user_id;                        /**< User ID */
        uint16_t                 measurement_status;             /**< Measurement status */
        bool                     time_stamp_present         : 1; /**< True if Time Stamp value is present */
        bool                     pulse_rate_present         : 1; /**< True if Pulse Rate value is present */
        bool                     user_id_present            : 1; /**< True if User ID value is present */
        bool                     measurement_status_present : 1; /**< True if Measurement status is present */
} bls_measurement_t;

/**
 * \enum bls_feature_t
 *
 * \brief Blood Pressure Feature supported features
 *
 */
typedef enum {
        BLS_FEATURE_BODY_MOVEMENT_DETECTION       = 0x01, /**< Body Movement Detection Support */
        BLS_FEATURE_CUFF_FIT_DETECTION            = 0x02, /**< Cuff Fit Detection Support */
        BLS_FEATURE_IRREGULAR_PULSE_DETECTION     = 0x04, /**< Irregular Pulse Detection Support */
        BLS_FEATURE_PULSE_RATE_RANGE_DETECTION    = 0x08, /**< Pulse Rate Range Detection Support */
        BLS_FEATURE_MEASUREMENT_POS_DETECTION     = 0x10, /**< Measurement Position Detection Support*/
        BLS_FEATURE_MULTIPLE_BOND                 = 0x20, /**< Multiple Bond Support */
} bls_feature_t;

/**
 * \enum bls_supported_char_t
 *
 * \brief Support optional characteristics
 *
 */
typedef enum {
        /**< Support Intermediate Cuff Pressure Characteristic */
        BLS_SUPPORTED_CHAR_INTERM_CUFF_PRESSURE = 0x01,
} bls_supported_char_t;

/**
 * \brief Blood Pressure Service config used during initialization of service
 *
 */
typedef struct {
        bls_feature_t           feature_supp;           /**< BLS supported features */
        bls_supported_char_t    supported_char;         /**< BLS supported optional characteristics */
} bls_config_t;

/**
 * \brief Initialize Body Pressure Service instance
 *
 * This function initializes Body Pressure Service with a given set of flags.
 *
 * \param [in] config           service configuration
 * \param [in] bls_config       Blood Pressure Service specific configuration
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *bls_init(const ble_service_config_t *config, const bls_config_t *bls_config,
                                                                        const bls_callbacks_t *cb);

/**
 * \brief Indicate blood pressure measurement to client
 *
 * Send indication of the Blood Pressure Measurement characteristic value to a client (if such
 * indications have been previously enabled).
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] measurement      blood pressure measurement
 *
 * \return True if the indication has been sent successfully, false otherwise
 *
 */
bool bls_indicate_pressure_measurement(ble_service_t *svc, uint16_t conn_idx,
                                                             const bls_measurement_t *measurement);

/**
 * \brief Notify intermediate cuff pressure measurement to client
 *
 * Send notification of the Intermediate Cuff Pressure characteristic value to a client (if such
 * notifications have been previously enabled).
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] measurement      intermediate blood pressure measurement
 *
 * \return True if the notification has been sent successfully, false otherwise
 *
 */
bool bls_notify_intermediate_cuff_pressure(ble_service_t *svc, uint16_t conn_idx,
                                                             const bls_measurement_t *measurement);

#endif /* BLS_H_ */
