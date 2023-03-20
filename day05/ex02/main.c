#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

# define I2C_EXPANDER_ADDR 0x20

int8_t values[4] = {0, 0, 0, 0};
int8_t selected = 0;

void display_value() {
	PORTB = (((values[selected] & 1) ? 1 : 0) << PB0) | (((values[selected] & 2) ? 1 : 0) << PB1) | (((values[selected] & 4) ? 1 : 0) << PB2) | (((values[selected] & 8) ? 1 : 0) << PB4);
}

void save_values() {
	safe_eeprom_write(values, 1, 4);
	safe_eeprom_write(&selected, 6, 1);
}

void load_values() {
	if(!safe_eeprom_read(values, 1, 4)) {
		values[0] = 0;
		values[1] = 0;
		values[2] = 0;
		values[3] = 0;
		safe_eeprom_write(values, 1, 4);
	}
	if(!safe_eeprom_read(&selected, 6, 1)) {
		selected = 0;
		safe_eeprom_write(&selected, 6, 1);
	}
}

# define LED_R PD5
# define LED_G PD6
# define LED_B PD3

void display_selected() {
	if (selected == 0) {
		PORTD = (1 << LED_R);
	} else if (selected == 1) {
		PORTD = (1 << LED_G);
	} else if (selected == 2) {
		PORTD = (1 << LED_B);
	} else if (selected == 3) {
		PORTD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);
	}
}

//Detect interupt on INT0
ISR (INT0_vect)  {
	values[selected]++;
	if (values[selected] > 15) {
		values[selected] = 0;
	}
	display_value();
	save_values();
	_delay_ms(50);
}

int8_t get_sw3_value() {
	int8_t data = i2c_expander_read_port(I2C_EXPANDER_ADDR, INPUT_PORT_0);
	int sw3 = data & 0b00000001;
	return !sw3;
}

int main() {
	//Set led as output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);

	//Set button as input
	DDRD |= (0 << PD2) | (0 << PD4);

	//Enable interupt on INT0
	EIMSK = (1 << INT0);
	//Make press (0->1) on INT0 generate an interupt
	EICRA = (1 << ISC01) | (0 << ISC00);

	uart_init();
	load_values();

	i2c_init();

	sei();

	int previouse_sw3 = 0;

	for(;;) {
		if (get_sw3_value() && !previouse_sw3) {
			previouse_sw3 = 1;
			selected++;
			if (selected > 3) {
				selected = 0;
			}
			save_values();
		}
		if (!get_sw3_value()) {
			previouse_sw3 = 0;
		}
		display_selected();
		display_value();
	}
}