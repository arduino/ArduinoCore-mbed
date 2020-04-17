STACK_SIZE = 1024;
M_CRASH_DATA_RAM_SIZE = 0x100;
MEMORY
{
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
  DTCMRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  RAM (xrw) : ORIGIN = 0x24000000 + 0x298, LENGTH = 512K - 0x298
  RAM_D2 (xrw) : ORIGIN = 0x30000000, LENGTH = 288K
  OPENAMP_RSC_TAB (rwx) : ORIGIN = 0x38000000, LENGTH = 1K
  OPEN_AMP_SHMEM (rwx) : ORIGIN = 0x38000400, LENGTH = 63K
  FLASH (rx) : ORIGIN = 0x8040000, LENGTH = 0xc0000
}
 __OPENAMP_region_start__ = ORIGIN(OPEN_AMP_SHMEM);
 __OPENAMP_region_end__ = ORIGIN(OPEN_AMP_SHMEM) + LENGTH(OPEN_AMP_SHMEM);
_ConfigStart = 0x0801F000;
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
    } > FLASH
    .ConfigData _ConfigStart :
    {
        KEEP(*(.bootloader_version))
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
    .crash_data_ram :
    {
        . = ALIGN(8);
        __CRASH_DATA_RAM__ = .;
        __CRASH_DATA_RAM_START__ = .;
        KEEP(*(.keep.crash_data_ram))
        *(.m_crash_data_ram)
        . += M_CRASH_DATA_RAM_SIZE;
        . = ALIGN(8);
        __CRASH_DATA_RAM_END__ = .;
    } > RAM
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
    .heap (COPY):
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        . = ORIGIN(RAM) + LENGTH(RAM) - STACK_SIZE;
        __HeapLimit = .;
    } > RAM
    .openamp_section (NOLOAD) : {
        . = ABSOLUTE(0x38000000);
        *(.resource_table)
    } >OPENAMP_RSC_TAB AT > FLASH
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > RAM
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    _estack = __StackTop;
    __StackLimit = __StackTop - STACK_SIZE;
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
