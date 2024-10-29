MEMORY
{
  FLASH (rx) : ORIGIN = 0x8040000, LENGTH = CM4_BINARY_START - 0x8040000
  DTCMRAM (rwx) : ORIGIN = 0x20000000 + (((166 * 4) + 7) & 0xFFFFFFF8), LENGTH = 128K - (((166 * 4) + 7) & 0xFFFFFFF8)
  RAM (xrw) : ORIGIN = 0x24000000, LENGTH = 0x80000
  RAM_D2 (xrw) : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3 (xrw) : ORIGIN = 0x38000000, LENGTH = 64K
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
}
__OPENAMP_region_start__  = 0x38000400;
__OPENAMP_region_end__ = 0x38000400 + LENGTH(RAM_D3) - 1K;
ENTRY(Reset_Handler)
SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        KEEP(*(.init))
        KEEP(*(.fini))
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)
        *(.rodata*)
        KEEP(*(.eh_frame*))
        *ltrans0*.o(.rodata*)
        *ltrans1*.o(.rodata*)
        *ltrans2*.o(.rodata*)
        *ltrans3*.o(.rodata*)
        *ltrans4*.o(.rodata*)
        *lib*.o(.rodata*)
    } > FLASH
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;
    __etext = .;
    _sidata = .;
    .data : AT (__etext)
    {
        __data_start__ = .;
        _sdata = .;
        *(vtable)
        *(.data*)
        . = ALIGN(8);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(8);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(8);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        KEEP(*(.jcr*))
        . = ALIGN(8);
        __data_end__ = .;
        _edata = .;
    } > RAM
    .uninitialized (NOLOAD):
    {
        . = ALIGN(32);
        __uninitialized_start = .;
        *(.uninitialized)
        KEEP(*(.keep.uninitialized))
        . = ALIGN(32);
        __uninitialized_end = .;
    } > RAM
    .bss :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
        _ebss = .;
    } > RAM
    .pdm_section 0x3800FC00 (NOLOAD):  {
 				*(.pdm_buffer)
 				} > RAM_D3
 					_dtcm_lma = __etext + SIZEOF(.data);
 					.dtcm : AT(_dtcm_lma) {
 					_sdtcm = .;
 					*(.dtcm*)
 					_edtcm = .;
 					} > DTCMRAM
    .heap (COPY):
    {
        __end__ = .;
        PROVIDE(end = .);
        *(.heap*)
        . = ORIGIN(RAM) + LENGTH(RAM) - 0x400;
        __HeapLimit = .;
    } > RAM
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > RAM
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    _estack = __StackTop;
    __StackLimit = __StackTop - 0x400;
    PROVIDE(__stack = __StackTop);
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
    .lwip_sec (NOLOAD) : {
        . = ABSOLUTE(0x30040000);
        *(.RxDecripSection)
        . = ABSOLUTE(0x30040100);
        *(.TxDecripSection)
        . = ABSOLUTE(0x30040400);
        *(.RxArraySection)
        . = ABSOLUTE(0x30044000);
        *(.ethusbram)
    } >RAM_D2 AT> FLASH
}
