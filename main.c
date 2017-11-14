#define UART_BASE 0x4000C000

int puts(const char *str)
{
	while (*str) {
		*((unsigned volatile int *) UART_BASE) = *str++;
	}
	return 0;
}

int main ( void )
{
	puts("Hello from adrianlshaw");	
	while (1);
}
