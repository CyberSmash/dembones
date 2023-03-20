
#include "uart.h"
// A Mailbox message with set clock rate of PL011 to 3MHz tag
volatile unsigned int  __attribute__((aligned(16))) mbox[9] = {
    9*4, 0, 0x38002, 12, 8, 2, 3000000, 0 ,0
};


// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}
 

void uart_init(int raspi)
{
	mmio_init(raspi);
 
	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.
 
	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);
 
	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
 
	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);
 
	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);
 
	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// Baud = 115200.
 
	// For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
	// Set it to 3Mhz so that we can consistently set the baud rate
	if (raspi >= 3) {
		// UART_CLOCK = 30000000;
		unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
		// wait until we can talk to the VC
		while ( mmio_read(MBOX_STATUS) & 0x80000000 ) { }
		// send our message to property channel and wait for the response
		mmio_write(MBOX_WRITE, r);
		while ( (mmio_read(MBOX_STATUS) & 0x40000000) || mmio_read(MBOX_READ) != r ) { }
	}
 
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}
 
unsigned char uart_getc()
{
    // Wait for UART to have received something.
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}
 
void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}

void uart_put_int(unsigned int val)
{
    char message_out[512] = {0};
    uint32_t num_digits = 0;
    uint32_t val2 = val;
    // Get the number of digits.
    do
    {
        ++num_digits;
        val2 /= 10;
    } while (val2 > 0);    

    for ( int i = num_digits - 1; i >= 0; i--)
    {
        message_out[i] = (val % 10) + 0x30;
        val /= 10;
    }


    uart_puts(message_out);
}

unsigned char dec_hex_lookup[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};


void uart_put_hex(uint32_t value) 
{
    char message_out[512] = {0};
    uint32_t count = 2;
    uint32_t num_digits = 0;
    uint32_t value2 = value;

    message_out[0] = '0';
    message_out[1] = 'x';

    do {
        num_digits++;
        value2 /= 16;
    } while (value2 > 0);

    count += num_digits - 1;

    do {
        uint32_t dec_val = value % 16;
        message_out[count] = dec_hex_lookup[dec_val];
        value /= 16;
        count--;
    } while(value > 0 && count <= 511);

    uart_puts(message_out);
}

uint32_t uart_b2str(uint8_t byte, char* message_out, uint32_t offset)
{
    if (message_out == NULL)
        return 0;
    
    int count = offset + 1;
    for (int i = 0; i < 2 && count >= offset; i++)
    {
        message_out[count] = dec_hex_lookup[byte % 16];
        byte /= 16;
        count--;
    }
    return 2;
}

void uart_print_buffer(uint8_t* buffer, uint32_t size)
{
    char byte[2];
    for (uint32_t i = 0; i < size; i++)
    {
        if (i % 16 == 0 && i != 0)
        {
            uart_putc('\n');
        }
        uart_b2str(buffer[i], byte, 0);
        uart_puts(byte);
        uart_putc(' ');
    }
}

size_t uart_gets(char * buffer, size_t max_size, bool echo)
{
	unsigned char b = '\0';
	size_t bytes_read = 0;
	do
	{
		b = uart_getc();
		if (b == '\r')
		{
			buffer[bytes_read] = '\r';
			buffer[bytes_read++] = '\n';
			break;
		}
		buffer[bytes_read] = b;
			
		if (echo)
			uart_putc(b);

		bytes_read++;
	} while (bytes_read < max_size);
	uart_puts("\r\nInput recv'd\r\n");		
	return bytes_read;
}
