/* Linker script to configure memory regions.
 * Need modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 *   REBOOT.ORIGIN: place for function that moves buffer to beginning of RAM and starts binary
 *   REBOOT.LENGTH: length of reboot area
 */

MEMORY
{
        RAM (rwx)    : ORIGIN = 0x07FC0000, LENGTH = 41K
        BUFFER (rw)  : ORIGIN = 0x07FCA400, LENGTH = 102K
        REBOOT (rwx) : ORIGIN = 0x07FE3C00, LENGTH = 1K
}
