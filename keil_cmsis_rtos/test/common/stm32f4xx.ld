/*******************************************************************************
  WEOS - Wrapper for embedded operating systems

  Copyright (c) 2013, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/* A linker script for the STM32F4xx.

   The following symbols are provided by this script:
   __etext               ... The start of the initialized data in the FLASH (the source for copying).
   __data_start__        ... The start of the initialized data section in the RAM (the target for copying).
   __data_end__          ... The end of the initialized data section in the RAM.
   __bss_start__         ... The start of the uninitialized data section in the RAM.
   __bss_end__           ... The end of the uninitialized data section in the RAM.
   __end__
   end
   __heap_start__        ... The start of the heap.
   __heap_end__          ... The end of the heap.
   __stack_limit__       ... The end of the stack.
   __stack_top__         ... The top of the stack.
   __stack               ... The initial stack pointer.
*/

/* Define the available memory regions. */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH =  128K
}

/* The entry point. */
ENTRY(Reset_Handler)

/* Map the object file sections to the memory regions. */
SECTIONS
{
    /* The .text section contains the program code. This section has to
       be placed in the FLASH. */
    .text :
    {
        /* The ISR vector table has to be placed at the very beginning of
           the FLASH. */
        KEEP(*(.isr_vector))

        *(.text*)

        /* Static constructors and destructors. */
        KEEP(*(.init))
        KEEP(*(.fini))

        /* Constructors (.ctors). */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* Destructors (.dtors). */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        /* Read-only data. */
        *(.rodata*)

        /* Exception-handling frames. */
        KEEP(*(.eh_frame*))
    } > FLASH

    /* Exception unwinding table. */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    /* Exception table indices. */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    /* Remember the end of the text section. The startup script knows that
       this is the start of the .data section in the FLASH. */
    __etext = .;

    /* The .data section contains the initialized data. For the program, this
       section is placed in the RAM. However, the linker puts the initial
       values in the FLASH (at address __etext). The startup script has to
       copy the FLASH to the RAM. */
    .data : AT (__etext)
    {
        /* Remember the beginning of the .data section. The startup script
           needs this address to initialized the section.  */
        . = ALIGN(4);
        __data_start__ = .;

        *(vtable)
        *(.data*)

        /* preinit data */
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        /* init data */
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        /* finit data */
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        /* .jcr sections record Java classes which need to be registered
           at startup. */
        KEEP(*(.jcr*))

        /* Remember the end of the .data section. The startup script needs this
           address to initialized the section. */
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM

    /* The .bss section contains the uninitialized data. */
    .bss :
    {
        . = ALIGN(4);
        /* Store the start of .bss. The startup script needs this address to
           zero the section. */
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        /* Store the end of .bss. The startup script needs this address to
           zero the section. */
        __bss_end__ = .;
    } > RAM

    /* Reserve RAM memory for the heap. The heap grows upward in the RAM. */
    .heap (COPY):
    {
        __end__ = .;
        end = __end__;
        __heap_start__ = __end__;
        *(.heap*)
        /* Store the end of the heap. */
        __heap_end__ = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > RAM

    /* Set the stack top to the end of the RAM. The stack grows downward, so
       the limit is the end of the RAM minus the size of the stack. */
    __stack_top__ = ORIGIN(RAM) + LENGTH(RAM);
    __stack_limit__ = __stack_top__ - SIZEOF(.stack_dummy);

    /* Define a symbol for the initial stack pointer. */
    PROVIDE(__stack = __stack_top__);

    /* Make sure that the heap does not intersect the stack. */
    ASSERT(__stack_limit__ >= __heap_end__, "The heap collides with the stack.")
}
