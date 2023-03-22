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

#define TOS 0

void adc_init() {
	// Configure ADC to use AVCC as reference
	ADMUX |= (1 << REFS0);
	// Set prescaler to 128
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(int mux3, int mux2, int mux1, int mux0) {
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

	// Read ADC value (10 bits)
	uint16_t res = ADCL;
	res |= ((ADCH & 0b00000011) << 8);

	// Disable ADC
	ADCSRA &= ~(1 << ADEN);

	return res;
}

uint16_t adc_read_chip_temperature() {
	//Enable ADC
	ADCSRA |= (1 << ADEN);

	// Choose ADC channel
	ADMUX |= (1 << MUX3);
	ADMUX &= ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	uint16_t res = ADCH << 8;
	res |= ADCL;
	res -= TOS;
	// Disable ADC
	ADCSRA &= ~(1 << ADEN);

	return res;
}
 
int main() {
	uart_init();

	adc_init();

	for(;;) {
		uart_printnbr(adc_read_chip_temperature());
		uart_ln();
		_delay_ms(100);
	}
}