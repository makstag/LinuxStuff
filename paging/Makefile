CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -mcmodel=kernel -fno-pic -Wall -Wextra -Werror \
	-pedantic -std=c23 -Wframe-larger-than=1024 -Wstack-usage=1024 -Wno-unknown-warning-option $(if $(DEBUG), -DDEBUG)
LFLAGS := -lc -z max-page-size=0x1000

INC := ./inc
SRC := ./src

CSOURCES := $(wildcard $(SRC)/*.c)
COBJECTS := $(CSOURCES:.c=.o)

SSOURCES := $(wildcard $(SRC)/*.s)
SOBJECTS := $(SSOURCES:.s=.o)

OBJ := $(COBJECTS) $(SOBJECTS)

all: kernel

kernel: $(OBJ) link.ld
	ld $(LFLAGS) -T link.ld -o $@ $(OBJ)

$(SOBJECTS): %.o: %.s
	cc nasm -D__ASM_FILE__ -I$(INC) -g -c $< -o $@

$(COBJECTS): %.o: %.c
	cc $(CFLAGS) -I$(INC) -g -c $< -o $@

.PHONY: clean
clean:
	rm -f kernel $(OBJ)