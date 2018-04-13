#include <tinycrypt/sha256.h>
#include "tinyprintf/tinyprintf.c"
#include "main.h"
#include "plat.h"

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

void printregs()
{
	tfp_printf("APSR: 0x%lx\n", get(APSR));
	tfp_printf("IPSR: 0x%lx\n", get(IPSR));
	tfp_printf("EPSR: 0x%lx\n", get(EPSR));
	tfp_printf("PSP: 0x%lx\n", get(PSP));
	tfp_printf("PRIMASK: 0x%lx\n", get(PRIMASK));
	tfp_printf("CONTROL: 0x%lx\n", get(CONTROL));
}

void usagefault()
{
	puts("Usagefault!");
	while(1);
}

void memfault()
{
	puts("\nMemfault!");
	printregs();
	while(1);
}

void systick()
{
	puts("\nSystick!");
}

void pendsv()
{
	puts("\nPendSV!");
}

void hardfault()
{
	puts("Oh fiddlesticks, a hard fault\n");
	printregs();
	reset();
}

void busfault()
{
	puts("Bus fault!\n");
	while(1);
}

void svc()
{
	puts("\nSVC!\n");
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
	puts("\n");
	print_word(1);
	puts("\n");
	print_word(2);
	puts("\n");
	print_word(3);
	puts("\n");

	if ((cpuid >> 24) & 0x41) {
		puts("ARM!\n");
	}

	if ((cpuid & BIT_MASK(24,31))) {
		puts("ARM!\n");
	}
}



int main(void)
{
	init_printf(NULL,putc);
	puts("Hello from adrianlshaw\n");
	puts("Testing supervisor call\n");
	__asm volatile ("SVC #15":::"memory");
	puts("Done\n");

	if (debug_watch_trace()){
		puts("Measuring cycles to perform SHA256\n");
		uint32_t cycle1 = cycles();
		sha();
		uint32_t cycle2 = cycles();
		uint32_t measurement = cycle2 - cycle1;
		puts("Cycles = ");
		//itoa(measurement);
		puts("\n");
	}
	else {
		puts("Not measuring\n");
	}

	if (mpu_exists()) {
		puts("MPU exists!\n");
	}
	else {
		puts ("No MPU\n");
	}

	enable_timer();
	puts("Success\n");
	printregs();
	reset();
	while (1);

}
