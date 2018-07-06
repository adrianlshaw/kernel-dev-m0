#include <tinycrypt/sha256.h>
#include "tinyprintf/tinyprintf.c"
#include "main.h"
#include "plat.h"
#include "task.h"

void __attribute__((naked)) panic(void);
void printregs(void);

void reset(void)
{
#define SYSRESETREQ 0x4
	uint32_t resetval = (0x05FA << 16);
	resetval = (resetval | SYSRESETREQ);

	iowrite32(0xE000ED0C, resetval); /* Write to the VECTKEY field */
	for(;;);
}

uint32_t debug_watch_trace(void)
{
	return ioread32(DWT_CTRL);
}

inline uint32_t cycles(void)
{
	return ioread32(DWT_CYCCNT);
}

void memcpy(void *dest, void *src, size_t n)
{
	int i;
	// Typecast src and dest addresses to (char *)
	char *csrc = (char *)src;
	char *cdest = (char *)dest;

	// Copy contents of src[] to dest[]
	for (i=0; i<n; i++) {
		cdest[i] = csrc[i];
	}
}

void memset(void *b, int c, int len)
{
	unsigned char *p = b;
	while(len > 0)
	{
		*p = c;
		p++;
		len--;
	}
}

int puts(const char *str)
{
	while (*str) {
		//*((unsigned volatile int *) UART_BASE) = *str++;
		putc(NULL, *str++);
	}
	return 0;
}


void usagefault()
{
	puts("Usagefault!");
	while(1);
}

void memfault()
{
	puts("\r\nMemfault!");
	printregs();
	while(1);
}

static task_t current_task; // Must use static for it to automatically appear in .data section
static task_t *next_task;

void systick()
{
	puts("\r\nSystick!\r\n");
	next_task = &current_task;
	// Set PENDSVSET bit in the NVIC ICSR register
	*((uint32_t volatile *)0xE000ED04) = 0x10000000;
}

/* NVIC automatically saves R0-R3, R12, LR, PC, XPSR on a new frame atop the PSP */
/* The rest have to be done in software */
void __attribute__((naked)) pendsv(void)
{
	puts("\r\nPendSV!"); // Remove?

	/* Save rest of registers to the stack */
	asm volatile (
	"cpsid i\n"
	"ldr r1,%[CURRENTSP]\n" // load current TCB into R1
	"mrs r0, psp\n" // Get stack pointer
	"str r0, [r1]\n" // store stack pointer in TCB
	"sub r0, r0, #32\n" // 32 not possible in thumb-16 mode 
	"mov r1, r4\n" // Shift r4-r10 into r1-r7 so we can put these on the stack
	"mov r2, r5\n"
	"mov r3, r6\n"
	"mov r4, r7\n"
	"mov r5, r8\n"
	"mov r6, r9\n"
	"mov r7, r10\n"
	"stmia r0!, {r1-r7}\n" // can only store to memory from lower registers
	"ldr r4, [%[NEXTSP]]\n" // Get stack pointer of new task
	"mov r0, r4\n"
	"isb\n"
	"msr psp, r4\n" // Load stack pointer of new task
	"sub r0, r0, #32\n" // Should this be 16?
	"ldmia r0!, {r1-r7}\n" // get saved register values 
	"mov r10, r7\n" // shift registers back
	"mov r9, r6\n"
	"mov r8, r5\n"
	"mov r7, r4\n"
	"mov r6, r3\n"
	"mov r5, r2\n"
	"mov r4, r1\n"
	"ldr r0, =0xfffffffd\n" // Exception return to unpriv thread mode
	"isb\n"
	"cpsie i\n"
	"bx r0\n" // make it so
	:: [CURRENTSP] "r" (current_task.sp), [NEXTSP] "r" (next_task->sp)
	);
	// 16 registers altogether are saved on stack (8 bw HW, 8 by SW) 
}

void hardfault(void)
{
	puts("Oh fiddlesticks, a hard fault\r\n");
	printregs();
}

void __attribute__((naked)) task_test() {
	puts ("Task started\n");
	init_printf(NULL,putc);
	while(1) { 
		asm volatile ("wfi\nisb\n");
		puts ("And I'm back\n");
		tfp_printf("Priv level = %lu\n", get(CONTROL));
		__asm volatile ("SVC #0":::"memory");
	}
}


void start_task(task_t task) {
	memcpy(&current_task, 0, sizeof(task_t));
	current_task.entry_point = task_test;
	
	asm volatile (
	"mov r0, %0\n" /* Entry point */
	"mov r1, %1\n" /* Process Stack Pointer */
	"msr psp, r1\n" /* Set process stack */
	"mov r2, #0x3\n" /* 0x3 is unprivileged mode and use PSP */
	"msr control, r2\n" /* Enable */
	"isb\n" /* Really enable */
	"mov pc, r0\n"
	:: "r" (task.entry_point), "r" (task.sp)
	);
}

static uint32_t panic_regs[16] = { 0 };

void __attribute__((naked)) panic(void) 
{
	asm volatile (
	"mov r0, %[pregs]\n"
	"stmia r0!, {r1-r7}\n"
	"mov r1, r8\n"
	"mov r2, r9\n"
	"mov r3, r10\n"
	"mov r4, r11\n"
	"mov r5, r12\n"
	"mov r6, lr\n"
	"stmia r0!, {r1-r6}\n"
	"bl printregs\n"
	:: [pregs] "r" (panic_regs)
	);
}	

void printregs(void)
{
	int i;
	/* Print general regs */
	for (i=0; i<12; i++) {
		tfp_printf("r%d: 0x%lx\r\n", i+1, panic_regs[i]);
	}
	tfp_printf("LR: 0x%lx\r\n", panic_regs[12]);

	tfp_printf("APSR: 0x%lx\r\n", get(APSR));
	tfp_printf("IPSR: 0x%lx\r\n", get(IPSR));
	tfp_printf("EPSR: 0x%lx\r\n", get(EPSR));
	tfp_printf("PSP: 0x%lx\r\n", get(PSP));
	tfp_printf("PRIMASK: 0x%lx\r\n", get(PRIMASK));
	tfp_printf("CONTROL: 0x%lx\r\n", get(CONTROL));
	reset();
}

void busfault()
{
	puts("Bus fault!\r\n");
	panic();
	while(1);
}

void svc()
{
	puts("\r\nSVC handled!\r\n");
}

void sha()
{
	struct tc_sha256_state_struct s;
	uint8_t digest[32] = { 0 };
	const char *m = "abc";

	tc_sha256_init(&s);
	tc_sha256_update(&s, (const uint8_t *) m, 3);
	tc_sha256_final(digest, &s);
}

uint32_t mpu_exists()
{
	return ioread32(MPU_TYPE);
}

void print_word(uint32_t word)
{
	int i;
	for (i=31; i >= 0; i--) {

		if (BIT_MASK(i,i) & word) {
			puts("1");			
		} else {
			puts("0");
		}
	}
}

void decode_cpuid(void)
{
	uint32_t cpuid = ioread32(CPUID);

	print_word(cpuid);
	puts("\r\n");
	print_word(1);
	puts("\r\n");
	print_word(2);
	puts("\r\n");
	print_word(3);
	puts("\r\n");

	if ((cpuid >> 24) & 0x41) {
		puts("ARM!\r\n");
	}

	if ((cpuid & BIT_MASK(24,31))) {
		puts("ARM!\r\n");
	}
}

int main(void)
{
	uart_init();
	init_printf(NULL,putc);
	puts("Hello from adrianlshaw\r\n");
	tfp_printf("Priv level = %lu\n", get(CONTROL));
	//__asm volatile ("SVC #15":::"memory");
#if 0
	if (debug_watch_trace()){
		puts("Measuring cycles to perform SHA256\r\n");
		uint32_t cycle1 = cycles();
		sha();
		uint32_t cycle2 = cycles();
		uint32_t measurement = cycle2 - cycle1;
		tfp_printf("Cycles = %ld\n", measurement);
		puts("\r\n");
	}
	else {
		puts("Not measuring\r\n");
	}
#endif
	if (mpu_exists()) {
		puts("MPU exists!\r\n");
	}
	else {
		puts ("No MPU\r\n");
	}

	enable_timer();
	puts("Success\r\n");
	current_task.sp = 0x20001FFF - 1000;
	start_task(current_task);
	/* Should not reach here */
	panic();
	while(1);
}
