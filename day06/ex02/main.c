#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

# define LED_R PD5
# define LED_G PD6
# define LED_B PD3

void init_rgb() {
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);
	PORTD = 0;

	TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1); 
	TCCR0B = (1 << CS00);
	OCR0A = 0;
	OCR0B = 0;

	TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2B1);
	TCCR2B = (1 << CS20);
	OCR2B = 0;
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
	OCR0B = r;
	OCR0A = g;
	OCR2B = b;
}

void wheel(uint8_t pos) {
	pos = 255 - pos;
	if (pos < 85) {
		set_rgb(255 - pos * 3, 0, pos * 3);
	} else if (pos < 170) {
		pos = pos - 85;
		set_rgb(0, pos * 3, 255 - pos * 3);
	} else {
		pos = pos - 170;
		set_rgb(pos * 3, 255 - pos * 3, 0);
	}
}

int main() {
	uart_init();
	init_rgb();
	sei();
	int pos = 0;
	for(;;) {
		wheel(pos);
		pos++;
		_delay_ms(10);
	}
}