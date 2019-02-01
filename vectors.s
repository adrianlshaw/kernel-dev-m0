.syntax unified
.thumb
.thumb_func
.global _start
_start:
stacktop: .word 0x20001FFF
.word main	@ reset 
.word nmi
.word hardfault
.word memfault	@ reserved 
.word busfault	@ reserved 
.word panic     @ Usage fault
.word hang	@ reserved
.word hang	@ reserved
.word hang	@ reserved
.word hang	@ reserved
.word svc
.word hang	@ reserved
.word hang	@ reserved
.word pendsvasm
.word systick	@ SysTick, if implemented
.word irq0
.word irq1

.thumb_func
pendsvasm:
cpsid i
mrs r0, psp

/* Save regs */
subs r0, #16 @ 16 bytes (4 bytes x 4 registers)
stmia r0!,{r4-r7}
mov r4, r8
mov r5, r9
mov r6, r10
mov r7, r11
subs r0, #32 @ 32 because stmia incremented by 16 bytes, and we need to store another 4 registers (+16)
stmia r0, {r4-r7}
/*subs r0, #16 @ bring pointer back to top of stack/*
subs r0, #32 @ bring pointer back to top of stack

/* Save SP */
ldr r2, =current_task
ldr r1, [r2]
str r0, [r1]

/* Next task */
ldr r2, =next_task
ldr r1, [r2]
ldr r0, [r1]
/*ldr r0, [r0] /*problematic jelly */

/* Restore regs */
/*adds r0, #4/* my stuff */
ldmia r0!, {r4-r7}
mov r8, r4
mov r9, r5
mov r10, r6
mov r11, r7
ldmia r0!, {r4-r7}

/* Resume execution */
msr psp,r0
ldr r0, =0xFFFFFFFD
cpsie i
bx r0

.thumb_func
reset:
    bl main
    b hang

.thumb_func
hang: b reset

.thumb_func
nmi:    b .

.thumb_func
hardfault: b reset

.thumb_func
irq0: b .

.thumb_func
irq1: b .
