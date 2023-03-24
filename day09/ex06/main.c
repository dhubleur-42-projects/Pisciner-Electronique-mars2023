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

# define I2C_EXPANDER_ADDR 0x20

volatile int i2c_7segment[4] = {-1, -1, -1, -1};

void i2c_7segment_write_number(int n, int dot);

ISR(TIMER0_COMPA_vect) {
	//Disable interrupt
	TIMSK0 &= ~(1 << OCIE0A);
	i2c_7segment_write_number(-1, 0);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b01111111);
	i2c_7segment_write_number(i2c_7segment[3], 0);
	i2c_7segment_write_number(-1, 0);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b10111111);
	i2c_7segment_write_number(i2c_7segment[2], 0);
	i2c_7segment_write_number(-1, 0);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11011111);
	i2c_7segment_write_number(i2c_7segment[1], 0);
	i2c_7segment_write_number(-1, 0);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11101111);
	i2c_7segment_write_number(i2c_7segment[0], 0);
	//Enable interrupt
	TIMSK0 |= (1 << OCIE0A);
}

void i2c_7segment_init() {
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_0, 0b00000001);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_1, 0b00000000);
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS00) | (1 << CS02);
	OCR0A = 0x3C;
	TIMSK0 |= (1 << OCIE0A);
}

void i2c_7segment_write_number(int n, int dot) {
	uint8_t data = 0b00000000;
	if (dot) {
		data |= 0b10000000;
	}
	if (n == 0) {
		data |= 0b00111111;
	} else if (n == 1) {
		data |= 0b00000110;
	} else if (n == 2) {
		data |= 0b01011011;
	} else if (n == 3) {
		data |= 0b01001111;
	} else if (n == 4) {
		data |= 0b01100110;
	} else if (n == 5) {
		data |= 0b01101101;
	} else if (n == 6) {
		data |= 0b01111101;
	} else if (n == 7) {
		data |= 0b00000111;
	} else if (n == 8) {
		data |= 0b01111111;
	} else if (n == 9) {
		data |= 0b01101111;
	}
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_1, data);
}

void i2c_7segment_write_numbers(int16_t n) {
	if (n < 0) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = 0;
		i2c_7segment[3] = 0;
	} else if (n < 10) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = 0;
		i2c_7segment[3] = n;
	} else if (n < 100) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = n / 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 1000) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = n / 100;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 10000) {
		i2c_7segment[0] = n / 1000;
		i2c_7segment[1] = (n / 100) % 10;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	}
}

void i2c_7segment_set_digit(int digit, int n) {
	i2c_7segment[digit] = n;
}

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

int main() {
	i2c_init();
	i2c_7segment_init();
	adc_init();

	sei();

	for(;;) {
		i2c_7segment_write_numbers(adc_read(0, 0, 0, 0));
	}
}