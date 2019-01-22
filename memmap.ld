
MEMORY
{
    rom : ORIGIN = 0x10000000, LENGTH = 512K
    ram : ORIGIN = 0x20000000, LENGTH = 512K
}

SECTIONS
{
    .text : { *(.text*) } > rom
    .rodata : { *(.rodata*) } > rom
    .bss : { *(.bss*) } > ram
    __StackTop = ORIGIN(rom) + LENGTH(rom);
}
