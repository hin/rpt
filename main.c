#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "serial.h"
#include "morse.h"

#define RX_SIGNAL_PORT PIND
#define RX_SIGNAL_PIN 3
#define TX_PTT_PORT PORTD
#define TX_PTT_PIN 2

static inline uint8_t rx_signal() {
    return (RX_SIGNAL_PORT & (1 << RX_SIGNAL_PIN)) != 0;
}

uint8_t ptt_status;
void ptt(uint8_t on) {
    if (ptt_status != on) {
        printf("ptt %d -> %d\r\n", ptt_status, on);
        ptt_status = on;
    }
    if (ptt_status) {
        TX_PTT_PORT |= 1 << TX_PTT_PIN;
    } else {
        TX_PTT_PORT &= ~(1 << TX_PTT_PIN);
    }
}

uint8_t hang_timer = 0;
void rpt_tick() {
    if (rx_signal())
        hang_timer = 5;
    if (hang_timer > 0) {
        ptt(1);
    } else {
        ptt(0);
    }
}

void tick_1hz() {
    if (hang_timer > 0)
        hang_timer --;
}

void timer0_init() {
    TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
    TCCR0B = (0<<WGM02) | (1<<CS00);
    OCR0A = 127;
    OCR0B = 0;
    TIMSK0 = (1 << TOIE0);
}

static uint16_t tick_1000hz;
ISR(TIMER0_OVF_vect) {
    static uint16_t cnt_1000hz = 0;
    cnt_1000hz += 1000;
    if (++cnt_1000hz > 62500) {
        //PORTB ^= 2;
        cnt_1000hz -= 62500;
        tick_1000hz++;
    }

    static uint16_t cnt_1hz = 0;
    cnt_1hz += 1;
    if (cnt_1hz > 62500) {
        //PORTB ^= 2;
        cnt_1hz -= 62500;
        tick_1hz();
    }

    static uint16_t cnt_100hz = 0;
    cnt_100hz += 100;
    if (++cnt_100hz > 62500) {
        //PORTB ^= 2;
        cnt_100hz -= 62500;
    }

    morse_tick();

    rpt_tick();
}

void init(void) {
    serial_init();
    timer0_init();
    sei();
}

int main(void) {
    //int count = 0;

    _delay_ms(1);
    init();
    printf("Start!\r\n");

//    DDRB = 0b00000110;
//    DDRD = 0b01111100;
    DDRD |= TX_PTT_PIN;

    while(1) {
        _delay_ms(100);
        printf("rx=%d hang=%d\r\n", rx_signal(), hang_timer);
    }

    return 0;
}
