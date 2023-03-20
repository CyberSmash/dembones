#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "postman.h"
#include "mmio.h"
#include "uart.h"

#pragma pack(push)
#pragma pack(4)
typedef struct _frame_buffer_info_t
{
    int physical_width;
    int physical_height;
    int virtual_width;
    int virtual_height;
    int gpu_pitch;
    int bit_depth;
    int x;
    int y;
    int gpu_pointer;
    int gpu_size;
} __attribute__((aligned (16)))  frame_buffer_info_t;
#pragma pack(pop)


frame_buffer_info_t frame_buffer_info = {
    .physical_width = 1024,
    .physical_height = 768,
    .virtual_width = 1024,
    .virtual_height = 768,
    .gpu_pitch = 0,
    .bit_depth = 16,
    .x = 0,
    .y = 0,
    .gpu_pointer = 0,
    .gpu_size = 0,
};


 

 
void memset(void * buff, unsigned char val, size_t buff_size)
{
	unsigned char* buff_ptr = (unsigned char*) buff;
	for (size_t i = 0; i < buff_size; i++)
	{
		buff_ptr[i] = val;
	}
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
 
#ifdef AARCH64
// arguments for AArch64
void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
#else
// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
#endif
{
	// initialize UART for Raspi2
	uart_init(2);
	uart_puts("Hello, kernel World!\r\n");
 	char buff[80];
    uart_puts("Writing things.\n");
    uart_put_int((uint32_t)&frame_buffer_info);
    uart_puts("\n");
    uart_put_hex((uint32_t)&frame_buffer_info);
    uart_puts("\n");
    uart_print_buffer(&frame_buffer_info, sizeof(frame_buffer_info));
    int mailbox_write_result = mailbox_write((int)(&frame_buffer_info + 0x40000000), 0x1); 
    uart_put_hex((uint32_t)mailbox_write_result);
    uart_puts("\n");
    if (mailbox_write_result < 0)
    {
        uart_puts("Write error! : ");
    }
    if (mailbox_write_result == POSTMAN_ERR_INVALID_MAILBOX)
    {
        uart_puts("Invalid mailbox.\n");
    }
    if (mailbox_write_result == POSTMAN_ERR_INVALID_MSG)
    {
        uart_puts("Invalid message.\n");
    }
    uart_puts("Written.\n");
    uint32_t setup_response = mailbox_read(0x1);
    uart_puts("\nRead things.\n");
   
    uart_put_int(setup_response);
    uart_puts("\n");
    if (setup_response != 0)
    {
        uart_puts("Invalid request to frame buffer");
    }
    else
    {
        uart_puts("Valid request to frame buffer");
    }

	do
	{
		memset(buff, 0, sizeof(buff));
		//uart_putc(uart_getc());
		uart_gets(buff, sizeof(buff), true);
		uart_puts(buff);
	} while (1);
}
