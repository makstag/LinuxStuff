#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "common.h"

// Базовый виртуальный адрес образа приложения. Должен соответствовать стартовому адресу, определённому в `user.ld`.
#define USER_BASE 0x1000000

#define SSTATUS_SPIE (1 << 5)

#define SATP_SV32 (1u << 31)
#define PAGE_V (1 << 0) // бит "Valid" (запись активна)
#define PAGE_R (1 << 1) // Доступна для чтения
#define PAGE_W (1 << 2) // Доступна для записи
#define PAGE_X (1 << 3) // Исполняемая
#define PAGE_U (1 << 4) // Пользователь (доступна в режиме пользователя)

#define PANIC(fmt, ...)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                                          \
        while (1)                                                                                                      \
            ;                                                                                                          \
    } while (0)

#define READ_CSR(reg)                                                                                                  \
    ({                                                                                                                 \
        unsigned long __tmp;                                                                                           \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                                                          \
        __tmp;                                                                                                         \
    })

#define WRITE_CSR(reg, value)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        uint32_t __tmp = (value);                                                                                      \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                                                        \
    } while (0)

#endif /* __KERNEL_H__ */
