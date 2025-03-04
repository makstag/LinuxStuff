#include "kernel.h"
#include "process_control_block.h"

extern char __bss[], __bss_end[], __stack_top[];
extern char __free_ram[], __free_ram_end[];

struct process procs[PROCS_MAX]; // Все блоки управления процессами.
struct process* proc_a;
struct process* proc_b;

struct process* create_process(uint32_t pc)
{
    // Поиск блока управления неиспользуемого процесса.
    struct process* proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++)
    {
        if (procs[i].state == PROC_UNUSED)
        {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");

    // Запись в стек значений регистров, сохранённых вызываемым кодом. Эти значения будут восстановлены при первом
    // переключении контекста функцией switch_context.
    uint32_t* sp = (uint32_t*)&proc->stack[sizeof(proc->stack)];
    *--sp = 0;            // s11
    *--sp = 0;            // s10
    *--sp = 0;            // s9
    *--sp = 0;            // s8
    *--sp = 0;            // s7
    *--sp = 0;            // s6
    *--sp = 0;            // s5
    *--sp = 0;            // s4
    *--sp = 0;            // s3
    *--sp = 0;            // s2
    *--sp = 0;            // s1
    *--sp = 0;            // s0
    *--sp = (uint32_t)pc; // ra

    // Инициализация полей.
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint32_t)sp;
    return proc;
}

__attribute__((naked)) void switch_context(uint32_t* prev_sp, uint32_t* next_sp)
{
    __asm__ __volatile__(
        // Запись сохранённых вызываемым кодом значений регистров в стек текущего процесса.
        "addi sp, sp, -13 * 4\n" // Аллокация пространства стека для 13 4-байтовых регистров.
        "sw ra,  0  * 4(sp)\n"   // Запись только сохранённых вызываемым кодом значений регистров.
        "sw s0,  1  * 4(sp)\n"
        "sw s1,  2  * 4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Переключение указателя стека.
        "sw sp, (a0)\n" // *prev_sp = sp;
        "lw sp, (a1)\n" // Переключение указателя стека (sp) сюда.

        // Восстановление сохранённых вызываемым кодом значений регистров из стека следующего процесса.
        "lw ra,  0  * 4(sp)\n" // Восстановление только сохранённых вызываемым кодом значений регистров.
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n" // Извлекли из стека 13 4-байтовых регистров.
        "ret\n");
}

void delay(void)
{
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}

void proc_a_entry(void)
{
    printf("starting process A\n");
    while (1)
    {
        putchar('A');
        switch_context(&proc_a->sp, &proc_b->sp);
        delay();
    }
}

void proc_b_entry(void)
{
    printf("starting process B\n");
    while (1)
    {
        putchar('B');
        switch_context(&proc_b->sp, &proc_a->sp);
        delay();
    }
}

__attribute__((naked)) __attribute__((aligned(4))) void kernel_entry(void)
{
    __asm__ __volatile__("csrw sscratch, sp\n"
                         "addi sp, sp, -4 * 31\n"
                         "sw ra,  4 * 0(sp)\n"
                         "sw gp,  4 * 1(sp)\n"
                         "sw tp,  4 * 2(sp)\n"
                         "sw t0,  4 * 3(sp)\n"
                         "sw t1,  4 * 4(sp)\n"
                         "sw t2,  4 * 5(sp)\n"
                         "sw t3,  4 * 6(sp)\n"
                         "sw t4,  4 * 7(sp)\n"
                         "sw t5,  4 * 8(sp)\n"
                         "sw t6,  4 * 9(sp)\n"
                         "sw a0,  4 * 10(sp)\n"
                         "sw a1,  4 * 11(sp)\n"
                         "sw a2,  4 * 12(sp)\n"
                         "sw a3,  4 * 13(sp)\n"
                         "sw a4,  4 * 14(sp)\n"
                         "sw a5,  4 * 15(sp)\n"
                         "sw a6,  4 * 16(sp)\n"
                         "sw a7,  4 * 17(sp)\n"
                         "sw s0,  4 * 18(sp)\n"
                         "sw s1,  4 * 19(sp)\n"
                         "sw s2,  4 * 20(sp)\n"
                         "sw s3,  4 * 21(sp)\n"
                         "sw s4,  4 * 22(sp)\n"
                         "sw s5,  4 * 23(sp)\n"
                         "sw s6,  4 * 24(sp)\n"
                         "sw s7,  4 * 25(sp)\n"
                         "sw s8,  4 * 26(sp)\n"
                         "sw s9,  4 * 27(sp)\n"
                         "sw s10, 4 * 28(sp)\n"
                         "sw s11, 4 * 29(sp)\n"

                         "csrr a0, sscratch\n"
                         "sw a0, 4 * 30(sp)\n"

                         "mv a0, sp\n"
                         "call handle_trap\n"

                         "lw ra,  4 * 0(sp)\n"
                         "lw gp,  4 * 1(sp)\n"
                         "lw tp,  4 * 2(sp)\n"
                         "lw t0,  4 * 3(sp)\n"
                         "lw t1,  4 * 4(sp)\n"
                         "lw t2,  4 * 5(sp)\n"
                         "lw t3,  4 * 6(sp)\n"
                         "lw t4,  4 * 7(sp)\n"
                         "lw t5,  4 * 8(sp)\n"
                         "lw t6,  4 * 9(sp)\n"
                         "lw a0,  4 * 10(sp)\n"
                         "lw a1,  4 * 11(sp)\n"
                         "lw a2,  4 * 12(sp)\n"
                         "lw a3,  4 * 13(sp)\n"
                         "lw a4,  4 * 14(sp)\n"
                         "lw a5,  4 * 15(sp)\n"
                         "lw a6,  4 * 16(sp)\n"
                         "lw a7,  4 * 17(sp)\n"
                         "lw s0,  4 * 18(sp)\n"
                         "lw s1,  4 * 19(sp)\n"
                         "lw s2,  4 * 20(sp)\n"
                         "lw s3,  4 * 21(sp)\n"
                         "lw s4,  4 * 22(sp)\n"
                         "lw s5,  4 * 23(sp)\n"
                         "lw s6,  4 * 24(sp)\n"
                         "lw s7,  4 * 25(sp)\n"
                         "lw s8,  4 * 26(sp)\n"
                         "lw s9,  4 * 27(sp)\n"
                         "lw s10, 4 * 28(sp)\n"
                         "lw s11, 4 * 29(sp)\n"
                         "lw sp,  4 * 30(sp)\n"
                         "sret\n");
}

void handle_trap(struct trap_frame* f)
{
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}

void kernel_main(void)
{
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    WRITE_CSR(stvec, (uint32_t)kernel_entry);

    proc_a = create_process((uint32_t)proc_a_entry);
    proc_b = create_process((uint32_t)proc_b_entry);
    proc_a_entry();

    PANIC("unreachable here!");
}
__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void)
{
    __asm__ __volatile__("mv sp, %[stack_top]\n" // Устанавливаем указатель стека
                         "j kernel_main\n"       // Переходим к функции main ядра
                         :
                         : [stack_top] "r"(__stack_top) // Передаём верхний адрес стека в виде %[stack_top]
    );
}
