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

#define ID_INTERVAL 600
#define ID_PTTON_DELAY 1
#define ID_SEND_TIME 5
#define ID_PTTOFF_DELAY 1

char *callsign = "SM0UTY/R";
uint16_t id_timer;
uint8_t courtesy_timer;

enum {
    IdState_Idle,
    IdState_PttOn,
    IdState_SendCallsign,
    IdState_Wait
} id_state;

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
    static uint8_t prev_rx = 0;
    uint8_t rx = rx_signal();

    if (rx)
        hang_timer = 15;

    if ((rx == 1) && (prev_rx == 0))
        courtesy_timer = 2;

    if ((rx == 0) && (prev_rx == 1) && (id_state == IdState_Idle)) {
        if (courtesy_timer == 0) {
            morse_send("K");
            courtesy_timer = 2;
        }
    }

    if ((hang_timer > 0) || (id_state != IdState_Idle)) {
        ptt(1);
    } else {
        ptt(0);
    }

    prev_rx = rx;
}

void tick_1hz() {
    if (hang_timer > 0)
        hang_timer --;

    if (courtesy_timer > 0)
        courtesy_timer --;

    if (id_timer > 0)
        id_timer --;
    else {
        switch(id_state) {
            case IdState_Idle:
                printf("ptton\r\n");
                id_state = IdState_PttOn;
                id_timer = ID_PTTON_DELAY;
                break;
            case IdState_PttOn:
                if (!rx_signal()) {
                    printf("callsign\r\n");
                    morse_send(callsign);
                    id_state = IdState_SendCallsign;
                    id_timer = ID_SEND_TIME;
                }
                break;
            case IdState_SendCallsign:
                printf("wait\r\n");
                id_state = IdState_Wait;
                id_timer = ID_PTTOFF_DELAY;
                break;
            case IdState_Wait:
                printf("idle\r\n");
                id_state = IdState_Idle;
                id_timer = ID_INTERVAL;
                break;
        }
    }
}

void timer0_init() {
    TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
    TCCR0B = (0<<WGM02) | (1<<CS01);
    OCR0A = 127;
    OCR0B = 0;
    TIMSK0 = (1 << TOIE0);
}

static uint16_t tick_1000hz;
ISR(TIMER0_OVF_vect) {
    static uint16_t cnt_1000hz = 0;
    cnt_1000hz += 7812;
    if (++cnt_1000hz > 62500) {
        //PORTB ^= 2;
        cnt_1000hz -= 62500;
        tick_1000hz++;
    }

    static uint16_t cnt_1hz = 0;
    cnt_1hz += 1;
    if (cnt_1hz > 7812) {
        //PORTB ^= 2;
        cnt_1hz -= 7812;
        tick_1hz();
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

    id_timer = ID_INTERVAL;
    id_state = IdState_Idle;

    //DDRB = 0b00000110;
    DDRD = 0b01100000;
    DDRD |= TX_PTT_PIN;

    while(1) {
        _delay_ms(100);
        printf("rx=%d hang=%d id=%d c=%d\r\n", rx_signal(), hang_timer, id_timer, courtesy_timer);
    }

    return 0;
}
