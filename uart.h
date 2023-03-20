#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mmio.h"
extern volatile unsigned int __attribute__((aligned(16))) mbox[9];

void uart_init(int raspi);
void uart_putc(unsigned char c);
unsigned char uart_getc();
void uart_puts(const char* str);
void uart_put_int(unsigned int val);
size_t uart_gets(char * buffer, size_t max_size, bool echo);
void uart_put_hex(uint32_t value); 
uint32_t uart_b2str(uint8_t byte, char* message_out, uint32_t offset);
void uart_print_buffer(uint8_t* buffer, uint32_t size);



#endif
