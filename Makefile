
CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
GDB = $(CROSS_COMPILE)gdb
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

# Cortex-M0/M0+: thumbv6m-none-eabi
# Cortex M3: thumbv7m-none-eabi
# Cortex M4/M7 no FPU: thumbv7em-none-eabi
# Coretex M4/M7 with FPU: thumbv7em-none-eabihf

AFLAGS = --warn --fatal-warnings -mcpu=cortex-m0

CFLAGS =	-mcpu=cortex-m0 \
			-mthumb \
			-Wall \
			-Werror \
			-O2 \
			-nostdlib \
			-nostartfiles \
			-ffreestanding \
			-g 

IMAGE := main

all : $(IMAGE).bin

rust:
	rustc main.rs --emit=obj

clean:
	rm -f *.bin *.o *.elf *.list 

vectors.o :
	$(AS) $(AFLAGS) vectors.s -o vectors.o

main.o : main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

$(IMAGE).bin : memmap.ld vectors.o main.o
	$(LD) -o $(IMAGE).elf -T memmap.ld vectors.o main.o
	$(OBJDUMP) -D $(IMAGE).elf > main.list
	$(OBJCOPY) $(IMAGE).elf $(IMAGE).bin -O binary

gdb: all
	$(GDB) $(IMAGE) -x gdbfile ./$(IMAGE).elf

qemu:
	@qemu-system-arm -S -gdb tcp::1234 -M lm3s811evb -m 8K -nographic -kernel $(IMAGE).bin

docker-qemu:
	@docker run -ti cortexm 

test:
	-pkill -9 qemu-system-arm
	@tmux new-session 'make qemu' \; \
		split-window 'sleep 1 && make gdb' \; \
		select-layout even-horizontal
