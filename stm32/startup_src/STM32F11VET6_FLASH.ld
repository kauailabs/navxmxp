
/*
* STM32 Discovery Linker Script
*/

/* Entry Point */
ENTRY(Reset_Handler)

_estack = 0x20020000; /* End of RAM.  Global needed for startup code */
_minimum_heap_size  = 0x4000; /* 16K heap (e.g., for malloc()) */
_minimum_stack_size = 0x2000; /* 8K bytes miniumum stack size */

MEMORY
{
  boot (rx)     : ORIGIN = 0x08000000, LENGTH = 16K
  eeprom (rw)   : ORIGIN = 0x08004000, LENGTH = 16K
  flash (rx)    : ORIGIN = 0x08008000, LENGTH = 480K /* Application code. */
  ram (xrw)     : ORIGIN = 0x20000000, LENGTH = 128K /* data, heap, stack */
}

EXTERN(g_pfnVectors)

SECTIONS
{
  .boot :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    *(.ARM.extab*)
    . = ALIGN(4);
  } >boot
  
   /* .ARM.exidx is sorted, so has to go in its own output section.*/
   .ARM.exidx : 
   {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
   } >boot

  .text :
  {
	. = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    . = ALIGN(4);
  } >flash

    _sidata = .;         /* global needed for startup_stm3210x_md_vl.S */  

  /*now start ram area with initialized data section */
  .data :
  {   
    _sdata = .;        /* global needed for startup_stm3210x_md_vl.S */
    *(.data)          
    *(.data*)         
    . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;
    . = ALIGN(4);
    _edata = .;        /* global needed for startup_stm3210x_md_vl.S */
  } >ram AT>flash

  /* uninitialized data section bss*/
  .bss :
  {
      . = ALIGN(4);
    _sbss = .;         /* global needed for startup_stm3210x_md_vl.S */
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;        /* global needed for startup_stm3210x_md_vl.S */
  } >ram

 
  /* Generates an error if minimum stack size is not left in ram */
  ._check_stack_space :
  {
    . = ALIGN(4);
    _heap_start = .;		/* define a global symbol at heap start */
    . = . + _minimum_heap_size;
    . = . + _minimum_stack_size;
    . = ALIGN(4);
  } >ram

  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) }

  /* Remove debug information from the standard libraries */
  /DISCARD/ : {
      *(.note.GNU-stack)
    libc.a ( * )
    libgcc.a ( * )
    libm.a ( * )
  }
}