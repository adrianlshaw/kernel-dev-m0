#include "main.h"

#define GPIOBASE 0x50000000
#define GPIO_DIRSET (GPIOBASE+0x518)
#define GPIO_OUTSET (GPIOBASE+0x508)
#define GPIO_OUTCLR (GPIOBASE+0x50C)

#define UART_BASE		0x40002000
#define UART_STARTTX    (UART_BASE+0x008)
#define UART_ENABLE     (UART_BASE+0x500)
#define UART_PSELTXD    (UART_BASE+0x50C)
#define UART_TXD        (UART_BASE+0x51C)
#define UART_BAUDRATE   (UART_BASE+0x524)
#define UART_CONFIG     (UART_BASE+0x56C)
#define UART_TXDRDY 	(UART_BASE+0x11C) 

static int initialised = 0;

static void uart_init (void)
{
	iowrite32(GPIO_DIRSET,1<<24);
	iowrite32(UART_BAUDRATE,0x01D7E000); //115200
	iowrite32(UART_PSELTXD,24);
	iowrite32(UART_ENABLE,4);
	iowrite32(UART_STARTTX,1);
	initialised = 1;
}

void putc(char c)
{
	if (!initialised) {
		uart_init();	
	}

	iowrite32(UART_TXDRDY,0);
	iowrite32(UART_TXD,(unsigned int) c);

	while(ioread32(UART_TXDRDY) == 0) {
		continue;
	}

	iowrite32(UART_TXDRDY,0);
}

/* Will send interrupts to */
void enable_timer()
{
	// unimplemented
#define TIMER_BASE 0x40008000
#define TIMER_CLEAR (TIMER_BASE + 0x00C)
#define TIMER_SHUTDOWN (TIMER_BASE + 0x010)
#define TIMER_CAPTURE0 (TIMER_BASE + 0x040)
#define TIMER_BITMODE (TIMER_BASE + 0x508)
#define TIMER_COMPARE0 (TIMER_BASE + 0x140)
#define TIMER_PRESCALAR (TIMER_BASE + 0x510)
#define TIMER_INTENSET (TIMER_BASE + 0x304)
#define TIMER_REGISTER0 (TIMER_BASE + 0x540)
#define TIMER_MODE (TIMER_BASE + 0x504)
#define MODE_TIMER 0
#define BITMODE 1 // 8 bits width

	iowrite32(TIMER_MODE, MODE_TIMER); // timer mode
	iowrite32(TIMER_BITMODE, BITMODE); // 8 bits 
	iowrite32(TIMER_PRESCALAR, 4); // 0 - 9
	iowrite32(TIMER_INTENSET, BIT_MASK(16,16)); // enable interrupts for timer0
	iowrite32(TIMER_REGISTER0, 101);

	iowrite32(TIMER_BASE, 0xF);
}
