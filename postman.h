#ifndef POSTMAN_H
#define POSTMAN_H
#include <stdint.h>

#define MAILBOX_READ    (unsigned int*)0x2000b880  // Receiving Mail
#define MAILBOX_POLL    (unsigned int*)0x2000b890  // Receive without retrieving
#define MAILBOX_SENDER  (unsigned int*)0x2000b894  // Sender information
#define MAILBOX_STATUS  (unsigned int*)0x2000b898  // Information
#define MAILBOX_CONFIG  (unsigned int*)0x2000b89C  // Settings
#define MAILBOX_WRITE   (unsigned int*)0x2000b8A0  // Send mail

#define POSTMAN_SUCCESS 0
#define POSTMAN_ERR_INVALID_MSG       -1
#define POSTMAN_ERR_INVALID_MAILBOX   -2

int mailbox_read(uint32_t mailbox);
int mailbox_write(uint32_t msg, uint32_t mailbox);

#endif
