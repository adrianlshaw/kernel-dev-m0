#include <tinycrypt/sha256.h>
#include "main.h"
#include "plat.h"

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
	// Typecast src and dest addresses to (char *)
	char *csrc = (char *)src;
	char *cdest = (char *)dest;

	// Copy contents of src[] to dest[]
	for (int i=0; i<n; i++) {
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
		putc(*str++);
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
	puts("\nMemfault!");
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
	while (1);
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

void flash2()
{
	unsigned int ra = 0;
	#define ROW1 12
	#define COL9 13
	iowrite32(GPIO_DIRSET, (1 << ROW1) | (1 << COL9));
	iowrite32(GPIO_OUTCLR, 1 << COL9); // set column to low
	iowrite32(GPIO_OUTSET, 1 << ROW1); // set row to high
	//while(1){}
    	//iowrite32(GPIO_OUTCLR,1<<ROW1);
}


int main(void)
{
	flash2();
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
	while (1);

}
