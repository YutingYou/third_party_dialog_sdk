/**
 ****************************************************************************************
 *
 * @file platform_devices.h
 *
 * @brief LRA haptic configuration.
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_

/*
 * Battery voltage (in mV)
 */
#define V_BAT     3000

/*
 * LRA vibration motor config
 */
#define JHV10L5L00SB      0
#define LVM61530B         1
#define LVM61930B         2
#define G0825001          3

#define LRA_MOTOR       JHV10L5L00SB

#if (LRA_MOTOR == JHV10L5L00SB)
#define LRA_FREQUENCY     170
#define LRA_MIN_FREQ      150
#define LRA_MAX_FREQ      190
#define LRA_MAX_NOM_V     2500    /* Nominal Maximum Voltage of the LRA (in mV - RMS) */
#define LRA_MAX_ABS_V     2750    /* Absolute Maximum Peak Voltage of the LRA (in mV - RMS) */
#define LRA_R             12      /* Impedance of the LRA (in Ohms) */

#elif (LRA_MOTOR == LVM61530B)
#define LRA_FREQUENCY     200
#define LRA_MIN_FREQ      190
#define LRA_MAX_FREQ      210
#define LRA_MAX_NOM_V     2000
#define LRA_MAX_ABS_V     2200
#define LRA_R             11

#elif (LRA_MOTOR == LVM61930B)
#define LRA_FREQUENCY     180
#define LRA_MIN_FREQ      170
#define LRA_MAX_FREQ      190
#define LRA_MAX_NOM_V     2000
#define LRA_MAX_ABS_V     2200
#define LRA_R             15

#else //G0825001
#define LRA_FREQUENCY     240
#define LRA_MIN_FREQ      225
#define LRA_MAX_FREQ      255
#define LRA_MAX_NOM_V     1800
#define LRA_MAX_ABS_V     1980
#define LRA_R             24
#endif


/*
 * Haptics Library config
 */
#define HL_I_DATA_THRESHOLD         1550    /* Threshold for i-data validity */

#define HL_CURVE_FITTER_AMP_THRESH  0.001   /* I-data amplitude threshold for Curve fitter */

#define HL_LRA_AFC_ZETA             0.1     /* AFC damping factor */

#define HL_SMART_DRIVE_APPLY        true    /* Apply SmartDrive algorithm */
#define HL_SMART_DRIVE_UPDATE       false   /* Update SmartDrive algorithm */

#define HL_SMART_DRIVE_PEAK_AMP     0.09    /* Peak expected back EMF amplitude */
#define HL_SMART_DRIVE_TAU          0.2     /* Mechanical time constant */

#define HL_SMART_DRIVE_LAMBDA       0.995   /* RLS forgetting factor */
#define HL_SMART_DRIVE_DELTA        1000    /* RLS matrix initialization parameter */


/*
 * Waveform Memory config
 */
#define WM_TIMEBASE       false             /* Waveform sequence timebase:
                                               false: AD_HAPTIC_WM_SEQUENCE_TIMEBASE * 4,
                                               true: AD_HAPTIC_WM_SEQUENCE_TIMEBASE */


#endif /* PLATFORM_DEVICES_H_ */
