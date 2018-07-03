#include <tinycrypt/sha256.h>
#include "tinyprintf/tinyprintf.c"
#include "main.h"
#include "plat.h"
#include "task.h"

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

void systick()
{
	puts("\r\nSystick!\r\n");
}

void pendsv()
{
	puts("\r\nPendSV!");
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
	}
}

static task_t current_task;

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
	puts("Testing supervisor call\r\n");
	__asm volatile ("SVC #15":::"memory");
	puts("Done\r\n");

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
