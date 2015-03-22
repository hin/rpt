#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/setbaud.h>

#include <util/delay.h>
#include <stdio.h>

#include "serial.h"

static FILE serial = FDEV_SETUP_STREAM(serial_sendchar,
				       serial_read,
				       _FDEV_SETUP_RW);

volatile char tx_buf[TX_BUF_LEN];
volatile uint8_t tx_buf_head = 0;
volatile uint8_t tx_buf_tail = 0;

volatile char rx_buf[TX_BUF_LEN];
volatile uint8_t rx_buf_head = 0;
volatile uint8_t rx_buf_tail = 0;
volatile uint16_t rx_buf_drop = 0;

void serial_init(void) {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~_BV(U2X0);
#endif

  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

  /* enable interrupt on receive */
  UCSR0B |= 1<<RXCIE0;

  stdout = &serial;
  stdin = &serial;
}

int serial_sendchar(char c, FILE *f) {
  uint8_t next_head = tx_buf_head + 1;
  if (next_head == TX_BUF_LEN)
    next_head = 0;

  /* Wait until there's room in the buffer */
  while (next_head == tx_buf_tail)
    /* wait */;

  tx_buf[tx_buf_head] = c;
  tx_buf_head = next_head;

  /* Enable interrupts on empty send buffer */
  UCSR0B |= (1<<UDRIE0);

  return 0;
}

ISR(USART_UDRE_vect) {
  uint8_t tail = tx_buf_tail;
  UDR0 = tx_buf[tail];
  tail++;
  if (tail == TX_BUF_LEN)
    tail = 0;
  tx_buf_tail = tail;

  /* If the buffer is empty we should disable interrupt */
  if (tail == tx_buf_head)
    UCSR0B &= ~(1 << UDRIE0);
}

int serial_read(FILE *f) {
  if (rx_buf_head == rx_buf_tail)
    return EOF;
  else {
    char data = rx_buf[rx_buf_tail];
    rx_buf_tail++;
    if (rx_buf_tail == RX_BUF_LEN)
      rx_buf_tail = 0;

    return data;
  }
}

ISR(USART_RX_vect) {
  char data = UDR0;
  uint8_t next_head = rx_buf_head + 1;
  if (next_head == RX_BUF_LEN)
    next_head = 0;

  if (next_head != rx_buf_tail) {
    rx_buf[rx_buf_head] = data;
    rx_buf_head = next_head;
  } else {
    rx_buf_drop++;
  }
}
