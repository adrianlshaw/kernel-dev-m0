#include "main.h"

#define UART_BASE 0x4000C000

void putc(const char c)
{
	iowrite32(UART_BASE, (unsigned int) c);
}

void uart_init() 
{

}

/* This will enable the in-built SysTick */
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

void flash(void)
{
	// unimplemented
}
