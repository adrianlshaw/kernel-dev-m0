PLAT=qemu
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

AFLAGS = --warn --fatal-warnings -mthumb-interwork -mcpu=cortex-m0

CFLAGS =		-mcpu=cortex-m0 \
			-mthumb \
			-mthumb-interwork \
			-Wall \
			-O2 \
			-nostdlib \
			-nostartfiles \
			-ffreestanding \
			-g \
			-Itinycrypt/lib/include/

LDFLAGS = -Ltinycrypt/lib

IMAGE := main

all: $(IMAGE).bin

flash-microbit:
	cp $(IMAGE).elf /media/user/MICROBIT/

rust:
	rustc main.rs --emit=obj

clean:
	rm -f *.bin *.o *.elf *.list 

libtinycrypt.a:
	cp config.mk ./tinycrypt/
	cd tinycrypt && make

vectors.o:
	$(AS) $(AFLAGS) vectors.s -o vectors.o

main.o: main.c
	rm plat.h
	ln -s $(PLAT).c plat.h 
	$(CC) $(CFLAGS) -c main.c -o main.o -Itinycrypt/lib/include/

$(IMAGE).bin : memmap.ld vectors.o main.o libtinycrypt.a
	$(LD) $(LDFLAGS) -o $(IMAGE).elf -T memmap.ld vectors.o main.o tinycrypt/lib/libtinycrypt.a
	$(OBJDUMP) -D $(IMAGE).elf > main.list
	$(OBJCOPY) $(IMAGE).elf $(IMAGE).bin -O binary

gdb: all
	$(GDB) $(IMAGE) -x gdbfile $(IMAGE).elf

debug-qemu:
	@qemu-system-arm -S -gdb tcp::1234 -d guest_errors -M lm3s811evb -m 8K -nographic -kernel $(IMAGE).elf

ci:
	@qemu-system-arm -d guest_errors -M lm3s811evb -m 8K -nographic -kernel $(IMAGE).elf

debug-microbit:
	@tmux new-session -s foo 'pyocd-gdbserver --persist -t nrf51 -bh -r' \; \
		split-window 'sleep 1 && arm-none-eabi-gdb -x gdbfile.board $(IMAGE).elf' \; \
		select-layout even-horizontal\; \
		select-window -t foo:0\; split-window -h 'minicom --device /ttyACM0';\

docker-qemu:
	@docker run -ti cortexm 

test:
	-pkill -9 qemu-system-arm
	@tmux new-session 'make debug-qemu' \; \
		split-window 'sleep 1 && make gdb' \; \
		select-layout even-horizontal
