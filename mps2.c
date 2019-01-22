#define UART_BASE 0x40200000 // to 0x40200FFF
#define UART_CTRL (UART_BASE + 0x8)
#define UART_BAUDDIV (UART_BASE + 0x010)

void uart_init()
{
	volatile unsigned long *baud = UART_BAUDDIV;
	volatile unsigned long *ctrl = UART_CTRL;
	*baud = 16;
	*ctrl = 1; // Enable
}

void putc(void *p, const char c)
{
	iowrite32(UART_BASE, (unsigned int) c);
}

void enable_timer(void) {

}
