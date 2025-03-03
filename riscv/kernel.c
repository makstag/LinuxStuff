#include "common.h"

extern char __bss[], __bss_end[], __stack_top[];

void kernel_main(void)
{
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    PANIC("booted!");
    printf("unreachable here!\n");
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void)
{
    __asm__ __volatile__("mv sp, %[stack_top]\n" // Устанавливаем указатель стека
                         "j kernel_main\n"       // Переходим к функции main ядра
                         :
                         : [stack_top] "r"(__stack_top) // Передаём верхний адрес стека в виде %[stack_top]
    );
}
