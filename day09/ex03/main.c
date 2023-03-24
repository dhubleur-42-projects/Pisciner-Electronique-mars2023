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

void reset_selected() {
	i2c_7segment_write_number(-1, 0);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11111111);
}

volatile int displaying = 0;

ISR(TIMER0_COMPA_vect) {
	//Disable interrupt
	TIMSK0 &= ~(1 << OCIE0A);
	reset_selected();
	if (displaying == 0) {
		i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b01111111);
		i2c_7segment_write_number(i2c_7segment[3], 0);
	} else if (displaying == 1) {
		i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b10111111);
		i2c_7segment_write_number(i2c_7segment[2], 0);
	} else if (displaying == 2) {
		i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11011111);
		i2c_7segment_write_number(i2c_7segment[1], 0);
	} else if (displaying == 3) {
		i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11101111);
		i2c_7segment_write_number(i2c_7segment[0], 0);
	}
	displaying++;
	if (displaying == 4) {
		displaying = 0;
	}
	//Enable interrupt
	TIMSK0 |= (1 << OCIE0A);
}

void i2c_7segment_init() {
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_0, 0b00000001);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_1, 0b00000000);
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS00) | (1 << CS02);
	OCR0A = 0x2C;
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
	displaying = 0;
	if (n < 0) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = -1;
		i2c_7segment[3] = -1;
	} else if (n < 10) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = -1;
		i2c_7segment[3] = n;
	} else if (n < 100) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = n / 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 1000) {
		i2c_7segment[0] = -1;
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

int main() {
	i2c_init();
	i2c_7segment_init();

	sei();
	
	for(;;) {
		for (int i = 0; i < 10; i ++) {
			i2c_7segment_write_numbers(i);
			_delay_ms(700);
		}
	}
}