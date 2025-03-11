#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "common.h"

// Базовый виртуальный адрес образа приложения. Должен соответствовать стартовому адресу, определённому в `user.ld`.
#define USER_BASE 0x1000000
#define PROC_EXITED 2
#define SSTATUS_SPIE (1 << 5)
#define SCAUSE_ECALL 8
#define SATP_SV32 (1u << 31)
#define PAGE_V (1 << 0) // бит "Valid" (запись активна)
#define PAGE_R (1 << 1) // Доступна для чтения
#define PAGE_W (1 << 2) // Доступна для записи
#define PAGE_X (1 << 3) // Исполняемая
#define PAGE_U (1 << 4) // Пользователь (доступна в режиме пользователя)

#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR 0x10001000
#define VIRTIO_REG_MAGIC 0x00
#define VIRTIO_REG_VERSION 0x04
#define VIRTIO_REG_DEVICE_ID 0x08
#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM 0x38
#define VIRTIO_REG_QUEUE_ALIGN 0x3c
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
#define VIRTIO_REG_QUEUE_NOTIFY 0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK 8
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define FILES_MAX 2
#define DISK_MAX_SIZE align_up(sizeof(struct file) * FILES_MAX, SECTOR_SIZE)
#define SSTATUS_SUM  (1 << 18)

struct tar_header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
    char data[]; // Массив, указывающий на область данных после заголовка
                 // (изменчивый член массива)
} __attribute__((packed));

struct file
{
    bool in_use;     // Указывает, используется ли эта запись файла
    char name[100];  // Имя файла
    char data[1024]; // Содержимое файла
    size_t size;     // Размер файла
};

// Запись области дескрипторов virtqueue.
struct virtq_desc
{
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

// Avail Ring.
struct virtq_avail
{
    uint16_t flags;
    uint16_t index;
    uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Запись Used Ring.
struct virtq_used_elem
{
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

// Used Ring.
struct virtq_used
{
    uint16_t flags;
    uint16_t index;
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue.
struct virtio_virtq
{
    struct virtq_desc descs[VIRTQ_ENTRY_NUM];
    struct virtq_avail avail;
    struct virtq_used used __attribute__((aligned(PAGE_SIZE)));
    int queue_index;
    volatile uint16_t* used_index;
    uint16_t last_used_index;
} __attribute__((packed));

// Запрос virtio-blk.
struct virtio_blk_req
{
    // Первый дескриптор: только чтение с устройства
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;

    // Второй дескриптор: возможна запись устройством, если это операция чтения (VIRTQ_DESC_F_WRITE)
    uint8_t data[512];

    // Третий дескриптор: возможна запись устройством (VIRTQ_DESC_F_WRITE)
    uint8_t status;
} __attribute__((packed));

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
