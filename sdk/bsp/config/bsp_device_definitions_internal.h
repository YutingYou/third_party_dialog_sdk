/**
 ****************************************************************************************
 *
 * @file bsp_device_definitions_internal.h
 *
 * @brief Board Support Package. Device-Map definitions.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEVICE_DEFINITIONS_INTERNAL_H_
#define BSP_DEVICE_DEFINITIONS_INTERNAL_H_

/*
 *  31           23 22               13       12     11            8 7              0
 *  +--------------+-------------------+------------+---------------+---------------+
 *  | Chip Family  |    Chip Variant    | 0: silicon | chip revision | chip stepping |
 *  |              |                   |            |   |r|r|r|r|   |s|s|s|s|s|s|s|s|
 *  |      |.|.|6|6|x|x|x|x|x|x|x|x|x|x|            |   |D|C|B|A|   |H|G|F|E|D|C|B|A|
 *  |      |.|.|9|8|y|y|y|y|y|y|y|y|y|y|--------------------------------------------|
 *  |      |.|.|z|z|9|8|7|6|5|4|3|2|1|0|  1: FPGA   |          FPGA version         |
 *  |              |                   |            |       (numerical 0-4095)      |
 *  +--------------+-------------------+------------+-------------------------------+
 *
 * The device is described by a 32-bit value with the following bit encoding:
 *
 * [11:0]   HW version, meaning depends on bit 12:
 *         For FPGA (bit 12 is 1):
 *              [11:0] FPGA image version:
 *                      0-4095: numerical version matching single FPGA image version
 *         For silicon (bit 12 is 0):
 *              [ 7:0] Bit field of chip stepping (e.g. xB)
 *                      0: A,
 *                      1: B,
 *                      2: C,
 *                      3: D,
 *                      4: E,
 *                      5: F,
 *                      6: G,
 *                      7: H
 *              [11:8] Bit field of chip revision (e.g. Ax)
 *                      0: A,
 *                      1: B,
 *                      2: C,
 *                      3: D
 *   [12]   Bit for selecting FPGA/silicon chip version
 *              0: silicon,
 *              1: FPGA
 * [22:13]  Bit field that matches one or more chip family variant(s) (up to 10 variants):
 *              0x000: invalid for silicon, used for FPGA only
 *              0x001: variant xy0
 *              0x002: variant xy1
 *              0x004: variant xy2
 *              0x008: variant xy3
 *              0x010: variant xy4
 *              0x020: variant xy5
 *              0x040: variant xy6
 *              0x080: variant xy7
 *              0x100: variant xy9
 *
 *
 * [31:23] Bit field that matches one or more chip family/families (up to 9 families):
 *              0x0001: 68z
 *              0x0002: 69z
 *
 * Example:
 *         DA14683_00 (_DEVICE_FAMILY_680 | _DEVICE_VARIANT_3 | _DEVICE_IC_REV_B | _DEVICE_IC_STEP_B)
 *
 */

/*
 * Internal definitions
 */

/* Make bit field value */
#define _DEVICE_MK_BF_VAL(field, value)         (((1 << value) << _DEVICE_ ## field ## _POS) & _DEVICE_ ## field ## _MASK)
/* Make numerical value */
#define _DEVICE_MK_NUM_VAL(field, value)        (((value) << _DEVICE_ ## field ## _POS) & _DEVICE_ ## field ## _MASK)

/* Device family definitions */
#define _DEVICE_FAMILY_MASK             0xFF800000
#define _DEVICE_FAMILY_POS              23
#define _DEVICE_FAMILY_680              0
#define _DEVICE_FAMILY_690              1
#define _DEVICE_MK_FAMILY(x)            _DEVICE_MK_BF_VAL(FAMILY, _DEVICE_FAMILY_ ## x)

/* Device variants definitions */
#define _DEVICE_VARIANT_MASK            0x007FE000
#define _DEVICE_VARIANT_POS             13
#define _DEVICE_VARIANT_XX0             0
#define _DEVICE_VARIANT_XX1             1
#define _DEVICE_VARIANT_XX2             2
#define _DEVICE_VARIANT_XX3             3
#define _DEVICE_VARIANT_XX4             4
#define _DEVICE_VARIANT_XX5             5
#define _DEVICE_VARIANT_XX6             6
#define _DEVICE_VARIANT_XX7             7
#define _DEVICE_VARIANT_XX8             8
#define _DEVICE_VARIANT_XX9             9
#define _DEVICE_MK_VARIANT(x)           _DEVICE_MK_BF_VAL(VARIANT, _DEVICE_VARIANT_XX ## x)

/* FPGA definitions */
#define _DEVICE_FPGA_MASK               0x00001000
#define _DEVICE_FPGA_POS                12

#define _DEVICE_FPGA_VER_MASK           0x00000FFF
#define _DEVICE_FPGA_VER_POS            0
#define _DEVICE_MK_FPGA_VER(n)          _DEVICE_MK_NUM_VAL(FPGA_VER, (n))

/* Device stepping major subrevisions */
#define _DEVICE_IC_REV_MASK             0x00000F00
#define _DEVICE_IC_REV_POS              8
#define _DEVICE_IC_REV_A                0
#define _DEVICE_IC_REV_B                1
#define _DEVICE_IC_REV_C                2
#define _DEVICE_IC_REV_D                3
#define _DEVICE_MK_IC_REV(x)            _DEVICE_MK_BF_VAL(IC_REV, _DEVICE_IC_REV_ ## x)

/* Device stepping minor subrevisions */
#define _DEVICE_IC_STEP_MASK            0x000000FF
#define _DEVICE_IC_STEP_POS             0
#define _DEVICE_IC_STEP_A               0
#define _DEVICE_IC_STEP_B               1
#define _DEVICE_IC_STEP_C               2
#define _DEVICE_IC_STEP_D               3
#define _DEVICE_IC_STEP_E               4
#define _DEVICE_IC_STEP_F               5
#define _DEVICE_IC_STEP_G               6
#define _DEVICE_IC_STEP_H               7
#define _DEVICE_MK_IC_STEP(y)           _DEVICE_MK_BF_VAL(IC_STEP, _DEVICE_IC_STEP_ ## y)
#define _DEVICE_MK_IC_VER(x, y)         (_DEVICE_MK_IC_REV(x) | _DEVICE_MK_IC_STEP(y))

#define _DEVICE_MASK                    (_DEVICE_FAMILY_MASK | _DEVICE_VARIANT_MASK | _DEVICE_FPGA_MASK)

/*
 *  680 device family.
 */

/* Variants */
#define DA14680                         (_DEVICE_MK_FAMILY(680) | _DEVICE_MK_VARIANT(0))
#define DA14681                         (_DEVICE_MK_FAMILY(680) | _DEVICE_MK_VARIANT(1))
#define DA14682                         (_DEVICE_MK_FAMILY(680) | _DEVICE_MK_VARIANT(2))
#define DA14683                         (_DEVICE_MK_FAMILY(680) | _DEVICE_MK_VARIANT(3))
/* FPGA Device */
#define DA1468X_FPGA                    (_DEVICE_MK_FAMILY(680) | _DEVICE_FPGA_MASK)
/* Family Wildcard */
#define DA1468X                         (_DEVICE_MK_FAMILY(680))

/*
 * 690 device family.
 */

/* Variants */
#define DA14691                         (_DEVICE_MK_FAMILY(690) | _DEVICE_MK_VARIANT(1))
#define DA14693                         (_DEVICE_MK_FAMILY(690) | _DEVICE_MK_VARIANT(3))
#define DA14695                         (_DEVICE_MK_FAMILY(690) | _DEVICE_MK_VARIANT(5))
#define DA14697                         (_DEVICE_MK_FAMILY(690) | _DEVICE_MK_VARIANT(7))
#define DA14699                         (_DEVICE_MK_FAMILY(690) | _DEVICE_MK_VARIANT(9))
/* FPGA Device */
#define D2522                           (_DEVICE_MK_FAMILY(690) | _DEVICE_FPGA_MASK)
/* Family Wildcard */
#define DA1469X                         (_DEVICE_MK_FAMILY(690))

 /*
 * The supported chip revisions used for runtime checks.
 */
#define DEVICE_IC_REV_A                 _DEVICE_IC_REV_A
#define DEVICE_IC_REV_B                 _DEVICE_IC_REV_B

/*
 * The supported chip steppings used for runtime checks.
 */
#define DEVICE_IC_STEP_A                _DEVICE_IC_STEP_A
#define DEVICE_IC_STEP_B                _DEVICE_IC_STEP_B
#define DEVICE_IC_STEP_C                _DEVICE_IC_STEP_C
#define DEVICE_IC_STEP_D                _DEVICE_IC_STEP_D
#define DEVICE_IC_STEP_E                _DEVICE_IC_STEP_E
#define DEVICE_IC_STEP_F                _DEVICE_IC_STEP_F
#define DEVICE_IC_STEP_G                _DEVICE_IC_STEP_G
#define DEVICE_IC_STEP_H                _DEVICE_IC_STEP_H

/*
 * Generic Revision and Step Macros.
 */
#define DEVICE_REV_A                    (_DEVICE_MK_IC_REV(A))
#define DEVICE_REV_B                    (_DEVICE_MK_IC_REV(B))

#define DEVICE_VER_AB                   (_DEVICE_MK_IC_VER(A, B))
#define DEVICE_VER_AE                   (_DEVICE_MK_IC_VER(A, E))
#define DEVICE_VER_BB                   (_DEVICE_MK_IC_VER(B, B))

/*
 * A generic FPGA check, available for any device family.
 */
#define DEVICE_FPGA                     ((dg_configDEVICE & _DEVICE_FPGA_MASK) == _DEVICE_FPGA_MASK)

/*
 * Macros checking against specific device characteristics.
 * Examples:
 * #if (DEVICE_FAMILY == DA1468X)
 * #if ((DEVICE_VARIANT == DA14695) || (DEVICE_VARIANT == DA14699))
 * #if (DEVICE_REVISION == DEVICE_REV_B)
 * #if ((DEVICE_VERSION == DEVICE_VER_AB) || (DEVICE_VERSION == DEVICE_VER_AE))
 */
#define DEVICE_FAMILY                   (dg_configDEVICE & _DEVICE_FAMILY_MASK)
#define DEVICE_VARIANT                  (dg_configDEVICE & (_DEVICE_FAMILY_MASK | _DEVICE_VARIANT_MASK))
#define DEVICE_REVISION                 (dg_configDEVICE & _DEVICE_IC_REV_MASK)
#define DEVICE_VERSION                  (dg_configDEVICE & (_DEVICE_IC_REV_MASK | _DEVICE_IC_STEP_MASK))

#endif /* BSP_DEVICE_DEFINITIONS_INTERNAL_H_ */
