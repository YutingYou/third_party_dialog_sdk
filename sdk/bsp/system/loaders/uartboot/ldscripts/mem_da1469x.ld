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
        RAM (rwx)    : ORIGIN = 0x0,    LENGTH = 48K
        /* Buffer length could be increased up to 447KB for DA14695, DA14697 and DA14699 devices */
        BUFFER (rw)  : ORIGIN = 48K,    LENGTH = 335K
        REBOOT (rwx) : ORIGIN = 383K,   LENGTH = 1K
}
