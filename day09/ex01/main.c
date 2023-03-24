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

int8_t get_sw3_value() {
	int8_t data = i2c_expander_read_port(I2C_EXPANDER_ADDR, INPUT_PORT_0);
	int sw3 = data & 0b00000001;
	return !sw3;
}

void display_value(int value) {
	uint8_t data = 0b11111111;
	if (value & 1) {
		data &= ~(1 << 1);
	}
	if (value & 2) {
		data &= ~(1 << 2);
	}
	if (value & 4) {
		data &= ~(1 << 3);
	}
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, data);
}

int main() {
	i2c_init();
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_0, 0b11110001);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11111111);
	
	int value = 0;
	int previouse_sw3 = 0;
	
	for(;;) {
		if (get_sw3_value() && !previouse_sw3) {
			previouse_sw3 = 1;
			value++;
			if (value > 7) {
				value = 0;
			}
			display_value(value);
		}
		if (!get_sw3_value()) {
			previouse_sw3 = 0;
		}
	}
}