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
	// Configure ADC to use AVCC as reference
	ADMUX |= (1 << REFS0);
	// Set prescaler to 128
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(int mux0, int mux1, int mux2, int mux3) {
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
 
int main() {
	uart_init();

	DDRC &= ~(1 << PC0);
	DDRC &= ~(1 << PC1);
	DDRC &= ~(1 << PC2);

	adc_init();

	for(;;) {
		uart_printnbr_u16(adc_read(0, 0, 0, 0));
		uart_printstr(", ");

		uart_printnbr_u16(adc_read(0, 0, 0, 1));
		uart_printstr(", ");

		uart_printnbr_u16(adc_read(0, 0, 1, 0));
		uart_ln();

		_delay_ms(20);
	}
}