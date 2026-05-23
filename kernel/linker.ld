ENTRY(_start)

SECTIONS {
    . = 0xffffffff80000000;

    .text   : { *(.text*)   }
    . = ALIGN(0x1000);      /* align to 4KB page boundary */
    .rodata : { *(.rodata*) }
    . = ALIGN(0x1000);      /* align to 4KB page boundary */
    .data   : { *(EXCLUDE_FILE(*loop.o) .data*) }
    . = ALIGN(0x1000);      /* align to 4KB page boundary */
    .bss    : {
        __bss_start = .;
        *(.bss*)
        __bss_end = .;
    }
    .user_bins : {
        user_loop_start = .;
        KEEP(*loop.o(.data))
        user_loop_end = .;
    }
}