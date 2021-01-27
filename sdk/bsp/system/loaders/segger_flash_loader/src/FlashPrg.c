/*********************************************************************
*               (c) SEGGER Microcontroller GmbH & Co. KG             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode template
--------  END-OF-HEADER  ---------------------------------------------
*/
#include "FlashOS.h"
#include "qspi_automode.h"
#include "hw_qspi.h"
#include "hw_watchdog.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************/


/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// We use this dummy variable to make sure that the PrgData
// section is present in the output elf-file as this section
// is mandatory in current versions of the J-Link DLL 
//
static volatile int _Dummy;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/



/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
__attribute__((section("PrgCode"))) int Init(U32 Addr, U32 Freq, U32 Func) {
  (void)Addr;
  (void)Freq;
  (void)Func;

  // run the initialization function
  hw_watchdog_freeze();
  return((int)qspi_automode_init());


}

/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*
*  Parameters
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
__attribute__((section("PrgCode"))) int UnInit(U32 Func) {
  (void)Func;

  // Uninit code
  hw_qspi_set_access_mode(HW_QSPIC, HW_QSPI_ACCESS_MODE_AUTO);

  return(0);
}
/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Check the flash is blank
*
*  Parameters
*    Addr: Address of the sector to be erased
*
*  Return value
*    0 O.K.
*    1 Error
*/

__attribute__((section("PrgCode"))) int BlankCheck(U32 Addr, U32 Size, U8 Pat)
{
        U8 tmp;

        for (int i = 0; i < Size; i++) {
                qspi_automode_read(i, &tmp, 1);
                if ( tmp != Pat) {
                        return (1);
                }
        }

        return (0);
}


/*********************************************************************
*
*       EraseChip
*
*  Function description
*    Erases all sectors.
*
*  Parameters
*
*
*  Return value
*    0 O.K.
*    1 Error
*/

__attribute__((section("PrgCode"))) int EraseChip   (void) {

        qspi_automode_erase_chip();
        return(0);

}

/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    Addr: Address of the sector to be erased
*
*  Return value
*    0 O.K.
*    1 Error
*/

__attribute__((section("PrgCode"))) int EraseSector(U32 SectorAddr) {
  //
  // Erase sector code
  //
  qspi_automode_erase_flash_sector(SectorAddr);

  return(0);
}

/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr: Destination address
*    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
*    pSrcBuff: Point to the source buffer
*
*  Return value 
*    0 O.K.
*    1 Error
*/
__attribute__((section("PrgCode"))) int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {
        U32 tmp;
        U8 *curPtr = pSrcBuff;

        while(NumBytes > 0) {
                tmp = qspi_automode_write_flash_page(DestAddr,curPtr,NumBytes);
                curPtr += tmp;
                DestAddr += tmp;
                NumBytes -= tmp;
        }
  return (0);
}

__attribute__((section("PrgCode"))) U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff) {
        U32 r;
        U8 tmp;
        int count = 0;

        r = Addr + NumBytes;

        do {
                qspi_automode_read(Addr+count, &tmp, 1);
                if ( tmp != pBuff[count]) {
                        r = (unsigned long)(Addr+count);
                        break;
                }
                count++;
        } while (--NumBytes);

        return r;
}
