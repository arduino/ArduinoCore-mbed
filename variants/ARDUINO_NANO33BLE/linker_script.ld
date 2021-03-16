MEMORY
{
  FLASH (rx) : ORIGIN = 0x10000, LENGTH = 0xf0000
  RAM_NVIC (rwx) : ORIGIN = 0x20000000, LENGTH = 0x100
  RAM_CRASH_DATA (rwx) : ORIGIN = (0x20000000 + 0x100), LENGTH = 0x100
  RAM (rwx) : ORIGIN = ((0x20000000 + 0x100) + 0x100), LENGTH = (0x40000 - (0x100 + 0x100))
}
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
ENTRY(Reset_Handler)
SECTIONS
{
    .text :
    {
        KEEP(*(.Vectors))
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
    .sdh_soc_observers :
    {
        PROVIDE(__start_sdh_soc_observers = .);
        KEEP(*(SORT(.sdh_soc_observers*)))
        PROVIDE(__stop_sdh_soc_observers = .);
    } > FLASH
    .sdh_stack_observers :
    {
        PROVIDE(__start_sdh_stack_observers = .);
        KEEP(*(SORT(.sdh_stack_observers*)))
        PROVIDE(__stop_sdh_stack_observers = .);
    } > FLASH
    .sdh_req_observers :
    {
        PROVIDE(__start_sdh_req_observers = .);
        KEEP(*(SORT(.sdh_req_observers*)))
        PROVIDE(__stop_sdh_req_observers = .);
    } > FLASH
    .sdh_state_observers :
    {
        PROVIDE(__start_sdh_state_observers = .);
        KEEP(*(SORT(.sdh_state_observers*)))
        PROVIDE(__stop_sdh_state_observers = .);
    } > FLASH
    .sdh_ble_observers :
    {
        PROVIDE(__start_sdh_ble_observers = .);
        KEEP(*(SORT(.sdh_ble_observers*)))
        PROVIDE(__stop_sdh_ble_observers = .);
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
    .data : AT (__etext)
    {
        __data_start__ = .;
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
        . = ALIGN(8);
        PROVIDE(__start_fs_data = .);
        KEEP(*(.fs_data))
        PROVIDE(__stop_fs_data = .);
        *(.jcr)
        . = ALIGN(8);
        __data_end__ = .;
    } > RAM
    __edata = .;
    .nvictable (NOLOAD) :
    {
      PROVIDE(__start_nvictable = .);
      KEEP(*(.nvictable))
      PROVIDE(__stop_nvictable = .);
    } > RAM_NVIC
    .crash_data_ram :
    {
      . = ALIGN(8);
      __CRASH_DATA_RAM__ = .;
      __CRASH_DATA_RAM_START__ = .;
      KEEP(*(.keep.crash_data_ram))
      *(.m_crash_data_ram)
      . += 0x100;
      . = ALIGN(8);
      __CRASH_DATA_RAM_END__ = .;
    } > RAM_CRASH_DATA
    .noinit (NOLOAD) :
    {
      PROVIDE(__start_noinit = .);
      KEEP(*(.noinit))
      PROVIDE(__stop_noinit = .);
    } > RAM
    .bss :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
    } > RAM
    .heap (NOLOAD):
    {
        __end__ = .;
        end = __end__;
        *(.heap*);
        ASSERT(. <= (ORIGIN(RAM) + LENGTH(RAM) - 0x400), "heap region overflowed into stack");
        . = ORIGIN(RAM) + LENGTH(RAM) - 0x400;
        __HeapLimit = .;
    } > RAM
    PROVIDE(__heap_start = ADDR(.heap));
    PROVIDE(__heap_size = SIZEOF(.heap));
    PROVIDE(__mbed_sbrk_start = ADDR(.heap));
    PROVIDE(__mbed_krbs_start = ADDR(.heap) + SIZEOF(.heap));
    .stack (NOLOAD):
    {
        __StackLimit = .;
        *(.stack*)
        . = ORIGIN(RAM) + LENGTH(RAM);
    } > RAM
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __StackLimit = __StackTop - 0x400;
    PROVIDE(__stack = __StackTop);
}
