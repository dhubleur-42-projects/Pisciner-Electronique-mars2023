#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"
#include "pins.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

void adc_init() {
	// Configure ADC to be left justified, use AVCC as reference
	ADMUX |= (1 << ADLAR) | (1 << REFS0);
	// Set prescaler to 128
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint8_t adc_read(int mux3, int mux2, int mux1, int mux0) {
	//Enable ADC
	ADCSRA |= (1 << ADEN);

	// Choose ADC channel
	if (mux0) {
		ADMUX |= (1 << MUX0);
	} else {
		ADMUX &= ~(1 << MUX0);
	}
	if (mux1) {
		ADMUX |= (1 << MUX1);
	} else {
		ADMUX &= ~(1 << MUX1);
	}
	if (mux2) {
		ADMUX |= (1 << MUX2);
	} else {
		ADMUX &= ~(1 << MUX2);
	}
	if (mux3) {
		ADMUX |= (1 << MUX3);
	} else {
		ADMUX &= ~(1 << MUX3);
	}

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	// Read ADC value
	uint8_t res = ADCH;

	// Disable ADC
	ADCSRA &= ~(1 << ADEN);

	return res;
}

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
	DDRC &= ~(1 << PC0);
	pin_mode(LED_D1, OUTPUT);
	pin_mode(LED_D2, OUTPUT);
	pin_mode(LED_D3, OUTPUT);
	pin_mode(LED_D4, OUTPUT);

	init_rgb();
	adc_init();

	for(;;) {
		uint8_t value = adc_read(0, 0, 0, 0);
		wheel(value);

		if (value >= (255 / 4)) {
			pin_set(LED_D1, HIGH);
		} else {
			pin_set(LED_D1, LOW);
		}

		if (value >= (255 / 2)) {
			pin_set(LED_D2, HIGH);
		} else {
			pin_set(LED_D2, LOW);
		}

		if (value >= (255 / 4 * 3)) {
			pin_set(LED_D3, HIGH);
		} else {
			pin_set(LED_D3, LOW);
		}

		if (value == 255) {
			pin_set(LED_D4, HIGH);
		} else {
			pin_set(LED_D4, LOW);
		}

		_delay_ms(20);
	}
}