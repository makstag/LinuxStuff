#ifndef __KERNEL_H__
#define __KERNEL_H__

#define PANIC(fmt, ...)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                                          \
        while (1)                                                                                                      \
            ;                                                                                                          \
    } while (0)

struct sbiret
{
    long error;
    long value;
};

#endif /* __KERNEL_H__ */
