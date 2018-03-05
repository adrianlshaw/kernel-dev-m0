#include <tinycrypt/sha256.h>
#define UART_BASE 0x40002000 // Microbit
//#define UART_BASE 0x4000C000 //QEMU

#define DWT_CTRL	0xE0001000
#define DWT_CYCCNT	0xE0001004

#define SYSTICK_CTRL_STATUS  0xE000E010
#define SYSTICK_RELOAD_VALUE 0xE000E014
#define SYSTICK_COUNT_VALUE  0xE000E018
#define SYSTICK_CALIB_VALUE  0xE000E01C

#define MPU_TYPE 0xE000ED90

#define CPUID 0x410CC200

#define	ioread32(addr) ioread32n(addr,0)
#define	iowrite32(addr, value) iowrite32n(addr,0,value)
#define	ioread32n(addr, idx) ( *((volatile uint32_t*)(addr)+idx) )
#define	iowrite32n(addr, idx, value) do { *((volatile uint32_t*)(addr)+idx) = (value); } while (0)

#define BIT_MASK(a, b) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))


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
		*((unsigned volatile int *) UART_BASE) = *str++;
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

void enable_timer(void)
{
	unsigned volatile int timer_val = ioread32(SYSTICK_CTRL_STATUS);
	unsigned volatile int tenms = ioread32(SYSTICK_CALIB_VALUE);
	tenms = tenms & 0x17;
	tenms = tenms; /* get 10ms */
	timer_val = timer_val | 0x4; /* set clock source to core */
	timer_val = timer_val | 0x2; /* enable interrupt generation on NVIC */
	timer_val = timer_val | 0x1; /* enable timer */
	iowrite32(SYSTICK_RELOAD_VALUE, tenms * 500000);
	iowrite32(SYSTICK_CTRL_STATUS, timer_val);
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
	while (1);
}
