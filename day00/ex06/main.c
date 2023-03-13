#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

int get_led_pin(int led) {
	switch (led) {
		case 1:
			return PB0;
		case 2:
			return PB1;
		case 3:
			return PB2;
		case 4:
			return PB4;
		default:
			return -1;
	}
}

int main() {

	//PB0 = led 1 
	//PB1 = led 2 
	//PB2 = led 3 
	//PB4 = led 4

	//Initialize the PB0/1/2/4 pin as an output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

	int current = 0;
	int direction = 1;

	while (1) {
		PORTB = (1 << get_led_pin(current));
		current += direction;
		if (current == 4) {
			direction = -1;
		} else if (current == 1) {
			direction = 1;
		}
		_delay_ms(100);
	}

	return (0);
}