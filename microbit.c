#include "main.h"

#define GPIOBASE 0x50000000
#define GPIO_DIRSET (GPIOBASE+0x518)

#define UART_BASE	0x40002000
#define UART_STARTTX    (UART_BASE+0x008)
#define UART_ENABLE     (UART_BASE+0x500)
#define UART_PSELTXD    (UART_BASE+0x50C)
#define UART_TXD        (UART_BASE+0x51C)
#define UART_BAUDRATE   (UART_BASE+0x524)
#define UART_CONFIG     (UART_BASE+0x56C)

#define UART_EVENT_BASE (UART_BASE+0x100) 
#define UART_TXDRDY 	(UART_EVENT_BASE+0x11C) 

static int initialised = 0;

void putc(char c)
{
	if (!initialised) {
		iowrite32(GPIO_DIRSET,1<<24);
		iowrite32(UART_BAUDRATE,0x01D7E000);
		iowrite32(UART_ENABLE,4);
		iowrite32(UART_STARTTX,1);
		initialised = 1;
	}

	/* A UART transmission sequence is started by triggering the STARTTX task. Bytes are transmitted by writing
	 * to the TXD register. When a byte has been successfully transmitted the UART will generate a TXDRDY
	 * event after which a new byte can be written to the TXD register. A UART transmission sequence is stopped
	 * immediately by triggering the STOPTX task. */

	iowrite32(UART_TXD, c);

	while (ioread32(UART_TXDRDY) == 0) {
		continue;
	}
	iowrite32(UART_TXDRDY,0);
#if 0
	while ((ioread32(UART_EVENT_BASE) | BIT_MASK(28,31))) {

	}
#endif
}

void enable_timer()
{

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
