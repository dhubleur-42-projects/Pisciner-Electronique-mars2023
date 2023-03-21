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

void set_rgb(int r, int g, int b) {
	PORTD = (r << LED_R) | (g << LED_G) | (b << LED_B);
}

int main() {
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);

	for(;;) {
		set_rgb(1, 0, 0);
		_delay_ms(1000);
		set_rgb(0, 1, 0);
		_delay_ms(1000);
		set_rgb(0, 0, 1);
		_delay_ms(1000);
		set_rgb(1, 1, 0);
		_delay_ms(1000);
		set_rgb(0, 1, 1);
		_delay_ms(1000);
		set_rgb(1, 0, 1);
		_delay_ms(1000);
		set_rgb(1, 1, 1);
		_delay_ms(1000);
	}
}