#define UART_BASE 0x4000C000
#define SYSTICK_CTRL_STATUS  0xE000E010
#define SYSTICK_RELOAD_VALUE 0xE000E014
#define SYSTICK_COUNT_VALUE  0xE000E018

#define	ioread32(addr) ioread32n(addr,0)
#define	iowrite32(addr, value) iowrite32n(addr,0,value)
#define	ioread32n(addr, idx) ( *((volatile uint32_t*)(addr)+idx) )
#define	iowrite32n(addr, idx, value) do { *((volatile uint32_t*)(addr)+idx) = (value); } while (0)

typedef unsigned int	        uint32_t;

int puts(const char *str)
{
	while (*str) {
		*((unsigned volatile int *) UART_BASE) = *str++;
	}
	return 0;
}

void systick()
{
	puts("\nSystick!");
}

void pendsv()
{
	puts("\nPendSV!");
}

void svc()
{
	puts("\nSVC!\n");
}

int main ( void )
{
	puts("Hello from adrianlshaw");	
	__asm volatile ("SVC #15":::"memory");
	puts("Done\n");
	unsigned volatile int timer_val = ioread32(SYSTICK_CTRL_STATUS);
	timer_val = timer_val | 0x4; /* set clock source to core */
	timer_val = timer_val | 0x2; /* enable interrupt generation on NVIC */
	timer_val = timer_val | 0x1; /* enable timer */
	iowrite32(SYSTICK_RELOAD_VALUE, 999999);
	iowrite32(SYSTICK_CTRL_STATUS, timer_val);

	while (1);
}
