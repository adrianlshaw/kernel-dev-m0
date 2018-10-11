#define UART_BASE 0x40200000 // to 0x40200FFF
#define UARTDR (UART_BASE + 0x0)
#define UARTRSR (UART_BASE + 0x4)

void uart_init()
{
}

void putc(void *p, const char c)
{
	iowrite32(UARTDR, (unsigned int) c);
}

void enable_timer(void) {

}
