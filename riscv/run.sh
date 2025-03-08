#!/bin/sh
set -xue

# Путь QEMU
QEMU=qemu-system-riscv32

# Путь к clang и его флагам
OBJCOPY=llvm-objcopy
CC=clang  # Для Ubuntu: используйте CC=clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib"

# Сборка ядра
# $CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
#     kernel.c common.c

# Запуск QEMU
# $QEMU -machine virt -bios default -nographic -serial mon:stdio --no-reboot \
#	-kernel kernel.elf

# Сборка оболочки (приложения)
$CC $CFLAGS -Wl,-Tuser.ld -Wl,-Map=shell.map -o shell.elf shell.c user.c common.c
$OBJCOPY --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin
$OBJCOPY -Ibinary -Oelf32-littleriscv shell.bin shell.bin.o

# Сборка ядра
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
    kernel.c common.c shell.bin.o
