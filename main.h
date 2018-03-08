#define UART_BASE 0x40002000 // Microbit

#define DWT_CTRL        0xE0001000
#define DWT_CYCCNT      0xE0001004

#define SYSTICK_CTRL_STATUS  0xE000E010
#define SYSTICK_RELOAD_VALUE 0xE000E014
#define SYSTICK_COUNT_VALUE  0xE000E018
#define SYSTICK_CALIB_VALUE  0xE000E01C

#define GPIOBASE 0x50000000
#define GPIO_OUTSET (GPIOBASE+0x508)
#define GPIO_OUTCLR (GPIOBASE+0x50C)
#define GPIO_DIRSET (GPIOBASE+0x518)

#define MPU_TYPE 0xE000ED90

#define CPUID 0x410CC200

#define ioread32(addr) ioread32n(addr,0)
#define iowrite32(addr, value) iowrite32n(addr,0,value)
#define ioread32n(addr, idx) ( *((volatile uint32_t*)(addr)+idx) )
#define iowrite32n(addr, idx, value) do { *((volatile uint32_t*)(addr)+idx) = (value); } while (0)

#define BIT_MASK(a, b) (((unsigned) -1 >> (31 - (b))) & ~((1U << (a)) - 1))
