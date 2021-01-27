/*
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

/* Linker script to configure memory regions.
 * May need to be modified for a specific board.
 *   ROM.ORIGIN: starting address of read-only RAM area
 *   ROM.LENGTH: length of read-only RAM area
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 */

/*
 * The size of the interrupt vector table
 */
#define IVT_AREA_OVERHEAD               0x200

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        #define CODE_BASE_ADDRESS               (0x0)   /* Remapped address will be 0x800000 */
        #define CODE_SZ                         (CODE_SIZE)

        MEMORY
        {
                /* CODE and RAM are merged into a single RAM section */
                RAM (rx) : ORIGIN = CODE_BASE_ADDRESS, LENGTH = CODE_SZ
        }
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        #define CODE_BASE_ADDRESS               (0x0)   /* Remapped address will be at any offset in
                                                                QSPI (0x16000000) according to the image header  */
        #define CODE_SZ                         (CODE_SIZE)
        #define RAM_BASE_ADDRESS                (0x20000000 + IVT_AREA_OVERHEAD)
        #define RAM_SZ                          (RAM_SIZE - IVT_AREA_OVERHEAD)

        MEMORY
        {
                ROM (rx) : ORIGIN = CODE_BASE_ADDRESS, LENGTH = CODE_SZ
                RAM (rw) : ORIGIN = RAM_BASE_ADDRESS,  LENGTH = RAM_SZ
        }
#else
        #error "Unknown code location type..."
#endif
