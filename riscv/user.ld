ENTRY(start)

SECTIONS {
    . = 0x1000000;

    /* машинный код */
    .text :{
        KEEP(*(.text.start));
        *(.text .text.*);
    }

    /* данные только для чтения */
    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    /* данные с изначальными значениями */
    .data : ALIGN(4) {
        *(.data .data.*);
    }

    /* данные, которые при запуске приложения должны заполняться нулями */
    .bss : ALIGN(4) {
        *(.bss .bss.* .sbss .sbss.*);

        . = ALIGN(16);
        . += 64 * 1024; /* 64KB */
        __stack_top = .;

       ASSERT(. < 0x1800000, "too large executable");
    }
}
