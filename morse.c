#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>

#include "morse.h"

#define MORSE_PERIOD 450
#define MORSE_QUEUE_LEN 16

char morse_queue[MORSE_QUEUE_LEN];
uint8_t morse_queue_read = 0;
uint8_t morse_queue_write = 0;

void morse_send(char *s) {
    while (*s) {
        morse_queue[morse_queue_write++] = *s++;
        if (morse_queue_write >= MORSE_QUEUE_LEN)
            morse_queue_write = 0;
    }
}

extern const int8_t sintable[];
extern uint8_t morsechar(char c);

uint8_t morse_cur_char = 0;
uint16_t morse_beep = 0;
uint16_t morse_silent = 0;

void morse_tick() {
    if (morse_beep > 0) {
        static uint16_t cntsin;
        cntsin += 13*8;
        if (cntsin >= 1024)
            cntsin -= 1024;
        OCR0B = 128 + (sintable[cntsin]);
        morse_beep --;
        return;
    }

    if (morse_silent > 0) {
//        OCR0B = 128;
        morse_silent --;
        return;
    }

    if (morse_cur_char != 0) {
        if (morse_cur_char & 1) {
            morse_beep = MORSE_PERIOD*3;
        } else {
            morse_beep = MORSE_PERIOD;
        }
        morse_silent = MORSE_PERIOD*1;
        morse_cur_char >>= 1;
        if (morse_cur_char == 1) {
            morse_silent = MORSE_PERIOD*3;
            morse_cur_char = 0;
        }
    } else {
        if (morse_queue_read != morse_queue_write) {
            char c = morse_queue[morse_queue_read];
            morse_queue_read ++;
            if (morse_queue_read >= MORSE_QUEUE_LEN)
                morse_queue_read = 0;
            printf("m >%c< %d\r\n", c, c);
            if (c == ' ') {
                morse_silent = MORSE_PERIOD * 7;
                morse_cur_char = 0;
            } else {
                morse_cur_char = morsechar(c);
            }
        }
    }

    OCR0B = 128;
    return;
}
