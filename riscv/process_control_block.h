#ifndef __PROCESS_CONTROL_BLOCK_H__
#define __PROCESS_CONTROL_BLOCK_H__

#include "common.h"

#define PROCS_MAX 8 // Максимальное число процессов.

#define PROC_UNUSED 0   // Неиспользуемый процесс.
#define PROC_RUNNABLE 1 // Готовый к выполнению процесс.

struct process
{
    int pid;             // ID процесса.
    int state;           // Состояние процесса: PROC_UNUSED или PROC_RUNNABLE
    vaddr_t sp;          // Указатель стека.
    uint8_t stack[8192]; // Стек ядра.
};

#endif /* __PROCESS_CONTROL_BLOCK_H__ */
