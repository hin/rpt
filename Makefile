MCU	?= atmega328p
F_CPU	= 16000000
BAUD	?= 57600
PORT	?= /dev/tty.usbmodem1411
PROGRAMMER ?= usbasp

FUSES	= -U hfuse:w:0xd2:m -U lfuse:w:0xff:m
PROGRAM ?= -c $(PROGRAMMER) -P $(PORT) -b $(BAUD)
AVRDUDE = sudo avrdude $(PROGRAM) -p $(MCU) -B 10 -F
CC	= avr-gcc
CFLAGS	= -Wall -Werror -Os -std=c99 -DF_CPU=$(F_CPU) -mmcu=$(MCU) -I. -ffunction-sections -I../src -DAVR
CFLAGS += -DBAUD=${BAUD}

VPATH	=
PROG	= rpt
GENSRCS	= sintable.c morsetab.c
SRCS	= main.c serial.c morse.c $(GENSRCS)
OBJS	= $(SRCS:.c=.o)
DEPS	= $(SRCS:.c=.d)



all build:	$(PROG).hex

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM  $< > $*.d

flash:	all
	$(AVRDUDE) -U flash:w:$(PROG).hex:i

sintable.c:	sintable.py
	python sintable.py > sintable.c

morsetab.c:	morsetab.py
	python morsetab.py > morsetab.c

fuse:
	$(AVRDUDE) $(FUSES)

install: flash fuse

clean:
	rm -f $(PROG).hex $(PROG).elf $(OBJS) $(DEPS) $(GENSRCS)

$(PROG).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG).elf $(OBJS) -lm -Wl,--gc-sections

$(PROG).hex: $(PROG).elf
	avr-objcopy -j .text -j .data -O ihex $(PROG).elf $(PROG).hex
	avr-size $(PROG).elf

-include $(DEPS)
