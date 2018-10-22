PLAT=qemu
QFLAGS=qemu-system-arm -d guest_errors -M lm3s811evb -m 8K -nographic -kernel
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
			-Werror \
			-Wall \
			-O2 \
			-nostdlib \
			-nostartfiles \
			-ffreestanding \
			-g \
			-Itinycrypt/lib/include/ \
			-Imath/

LDFLAGS = -Ltinycrypt/lib -Lmath

IMAGE := main

all: $(IMAGE).bin
	arm-none-eabi-readelf -S main.elf

flash-microbit:
	cp $(IMAGE).elf /media/user/MICROBIT/

rust: vectors.o
	rustc main.rs --emit=obj

clean:
	rm -f *.bin *.o *.elf *.list 

libtinycrypt.a:
	cp config.mk ./tinycrypt/
	cd tinycrypt && make

.PHONY: math
math:
	$(AS) $(AFLAGS) math/*.S -o math/math.a

vectors.o:
	$(AS) $(AFLAGS) vectors.s -o vectors.o


main.o: main.c
	rm -f plat.h
	ln -s $(PLAT).c plat.h 
	$(CC) $(CFLAGS) -c main.c -o main.o -Itinycrypt/lib/include/

$(IMAGE).bin : memmap.ld vectors.o  main.o libtinycrypt.a math
	$(LD) $(LDFLAGS) -o $(IMAGE).elf -T memmap.ld vectors.o main.o tinycrypt/lib/libtinycrypt.a math/math.a
	$(OBJDUMP) -D $(IMAGE).elf > main.list
	$(OBJCOPY) $(IMAGE).elf $(IMAGE).bin -O binary

gdb: all
	$(GDB) $(IMAGE) -x gdbfile $(IMAGE).elf

qemu:
	$(QFLAGS) $(IMAGE).elf

qemu-gdb:
	$(QFLAGS) $(IMAGE).elf -S -gdb tcp::1234

ci:
	$(QFLAGS) $(IMAGE).elf

docker-qemu:
	@docker build -t cortexm .
	@docker run --rm -ti cortexm 

debug-qemu:
	-pkill -9 qemu-system-arm
	@tmux new-session 'make qemu-gdb' \; \
		split-window 'sleep 1 && make gdb' \; \
		select-layout even-horizontal

debug-microbit:
	tmux new-session -d -s foo 'pyocd-gdbserver --persist -t nrf51 -bh -r'
	tmux send-keys 'bundle exec thin start' 'C-m'
	tmux rename-window 'Foo'
	tmux select-window -t foo:0
	tmux split-window -h 'sleep 1 && arm-none-eabi-gdb -x gdbfile.board $(IMAGE).elf'
	tmux split-window -v -t 0 'minicom --device=/dev/ttyACM0'
	tmux -2 attach-session -t foo
	tmux select-window -t foo:0
