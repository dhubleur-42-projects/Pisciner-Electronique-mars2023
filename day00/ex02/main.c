#include <avr/io.h>
#include <util/delay.h>

int main() {

	//PB0 (Pin0 of PORTB) = led

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	while (1) {
		//Switch the PB0 state
		PORTB ^= (1 << PB0);
		_delay_ms(500);
	}

	return (0);
}