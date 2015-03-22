#ifndef _SERIAL_H_INCLUDED_
#define _SERIAL_H_INCLUDED_

#include <stdio.h>

#ifndef TX_BUF_LEN
#define TX_BUF_LEN 80
#endif
#ifndef RX_BUF_LEN
#define RX_BUF_LEN 80
#endif

void serial_init(void);
int serial_sendchar(char c, FILE *f);
int serial_read(FILE *f);

#endif /* !_SERIAL_H_INCLUDED_ */
