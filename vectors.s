.cpu cortex-m0
.thumb

.thumb_func
.global _start
_start:
stacktop: .word __StackTop 
.word main	@ reset 
.word nmi
.word hardfault
.word memfault	@ reserved 
.word busfault	@ reserved 
.word usagefault@ reserved
.word hang	@ reserved
.word hang	@ reserved
.word hang	@ reserved
.word hang	@ reserved
.word svc
.word hang	@ reserved
.word hang	@ reserved
.word pendsv
.word systick	@ SysTick, if implemented
.word irq0
.word irq1

.thumb_func
reset:
    bl main
    b hang

.thumb_func
hang: b .

.thumb_func
nmi:    b .

.thumb_func
hardfault: b .

.thumb_func
irq0: b .

.thumb_func
irq1: b .
