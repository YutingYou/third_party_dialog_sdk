MEMORY
{
  UNPLACED_SECTIONS (wx) : ORIGIN = 0x100000000, LENGTH = 0
  RAM (wx) : ORIGIN = 0x07fd0000, LENGTH = 0x00004000
}


SECTIONS
{
  __RAM_segment_start__ = 0x07fd0000;
  __RAM_segment_end__ = 0x07fd4000;
  __RAM_segment_size__ = 0x00004000;

  __HEAPSIZE__ = 256;
  __STACKSIZE_ABT__ = 0;
  __STACKSIZE_FIQ__ = 0;
  __STACKSIZE_IRQ__ = 0;
  __STACKSIZE_PROCESS__ = 0;
  __STACKSIZE_SVC__ = 0;
  __STACKSIZE_UND__ = 0;
  __STACKSIZE__ = 256;

  __PrgCode_load_start__ = ALIGN(__RAM_segment_start__ , 4);
  PrgCode ALIGN(__RAM_segment_start__ , 4) : AT(ALIGN(__RAM_segment_start__ , 4))
  {
    __PrgCode_start__ = .;
    KEEP(*(PrgCode PrgCode.*))
    *(.init*)
    *(.fini*)
    *(.text*)
    *(text_retained)
    *(.text_retained*)
  }
  __PrgCode_end__ = __PrgCode_start__ + SIZEOF(PrgCode);
  __PrgCode_size__ = SIZEOF(PrgCode);
  __PrgCode_load_end__ = __PrgCode_end__;

  . = ASSERT(__PrgCode_start__ == __PrgCode_end__ || (__PrgCode_end__ >= __RAM_segment_start__ && __PrgCode_end__ <= __RAM_segment_end__) , "error: PrgCode is too large to fit in RAM memory segment");

  __PrgData_load_start__ = ALIGN(__PrgCode_end__ , 4);
  PrgData ALIGN(__PrgCode_end__ , 4) : AT(ALIGN(__PrgCode_end__ , 4))
  {
    __PrgData_start__ = .;
    KEEP(*(PrgData PrgData.*))
    *(.data*)
    *(.rodata*)
    *(.bss*)
    *(retention_mem_init)
    *(retention_mem_zi)
  }
  __PrgData_end__ = __PrgData_start__ + SIZEOF(PrgData);
  __PrgData_size__ = SIZEOF(PrgData);
  __PrgData_load_end__ = __PrgData_end__;

  . = ASSERT(__PrgData_start__ == __PrgData_end__ || (__PrgData_end__ >= __RAM_segment_start__ && __PrgData_end__ <= __RAM_segment_end__) , "error: PrgData is too large to fit in RAM memory segment");

  __DevDscr_load_start__ = ALIGN(__PrgData_end__ , 4);
  DevDscr ALIGN(__PrgData_end__ , 4) : AT(ALIGN(__PrgData_end__ , 4))
  {
    __DevDscr_start__ = .;
    KEEP(*(DevDscr DevDscr.*))
  }
  __DevDscr_end__ = __DevDscr_start__ + SIZEOF(DevDscr);
  __DevDscr_size__ = SIZEOF(DevDscr);
  __DevDscr_load_end__ = __DevDscr_end__;

  __RAM_segment_used_end__ = ALIGN(__PrgData_end__ , 4) + SIZEOF(DevDscr);
  __RAM_segment_used_size__ = __RAM_segment_used_end__ - __RAM_segment_start__;

  . = ASSERT(__DevDscr_start__ == __DevDscr_end__ || (__DevDscr_end__ >= __RAM_segment_start__ && __DevDscr_end__ <= __RAM_segment_end__) , "error: DevDscr is too large to fit in RAM memory segment");

}

