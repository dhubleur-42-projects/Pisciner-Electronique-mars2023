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

void adc_init(int mux0, int mux1, int mux2, int mux3) {
	// Configure ADC to be left justified, use AVCC as reference
	ADMUX |= (1 << ADLAR) | (1 << REFS0);
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
	// Enable ADC and set prescaler to 128
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint8_t read_adc() {
	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	// Read ADC value
	return ADCH;
}
 
int main() {
	uart_init();

	// Set PC0 as input
	DDRC &= ~(1 << PC0);

	adc_init(0, 0, 0, 0);

	for(;;) {
		uart_print_byte_hexa(read_adc());
		uart_ln();

		_delay_ms(20);
	}
}