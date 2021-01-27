/* Linker script to configure memory regions.
 * Need modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   RetRAM0.ORIGIN: starting address of retained RAM area
 *   RetRAM0.LENGTH: length of retained RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 */

MEMORY
{
        RAM (rwx)    : ORIGIN = 0x00000000, LENGTH = 62K
        RetRAM0 (rwx): ORIGIN = 62K,        LENGTH =  2K
        BUFFER (rw)  : ORIGIN = 0x00010000, LENGTH = 64K
}
