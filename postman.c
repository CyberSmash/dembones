#include "postman.h"
#include <stdint.h>
#include "uart.h"

int mailbox_read(uint32_t mailbox)
{
    uint32_t status = 0;
    uint32_t val = 0;
    uint32_t status_mailbox = 0;
    if (mailbox > 0x0F)
    {
        return POSTMAN_ERR_INVALID_MAILBOX;
    }

    do
    {

        status = *(MAILBOX_STATUS);
        if ((status & 0x40000000) != 0)
        {
            continue;
        }

        val = *(MAILBOX_READ);
        status_mailbox = val & 0x0000000F;

        if (status_mailbox != mailbox)
        {
            uart_puts("Read message from address: ");
            uart_put_hex(val);
            uart_puts("\n Epecting: ");
            uart_put_hex(mailbox);
            uart_puts("\n");
            continue;
        }

        break;

    } while ( 1 );

    return val & 0xFFFFFFF0;

}

int mailbox_write(uint32_t msg, uint32_t mailbox)
{
    uint32_t status = 0;
    uint32_t to_write = 0;
    if ((msg & 0x0000000F) != 0)
    {
        return POSTMAN_ERR_INVALID_MSG;
    }
    
    if (mailbox > 0x0F) 
    {
        return POSTMAN_ERR_INVALID_MAILBOX;
    }

    uart_puts("Writing to mailbox: ");
    uart_put_int(mailbox);
    uart_puts("\n");

    to_write = msg + mailbox;

    while ((*(MAILBOX_STATUS) & 0x80000000) != 0)
    {
        // Loop
    }
    
    *(MAILBOX_WRITE) = to_write;
    return POSTMAN_SUCCESS;
}
