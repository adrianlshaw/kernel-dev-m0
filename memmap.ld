
MEMORY
{
    rom : ORIGIN = 0x00000000, LENGTH = 0x1FFF 
    ram : ORIGIN = 0x20000000, LENGTH = 0x1FFF 
}

SECTIONS
{
    .text : { *(.text*) } > rom
    .rodata : { *(.rodata*) } > rom
    .bss : { *(.bss*) } > ram
}
