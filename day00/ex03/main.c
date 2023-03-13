#include <avr/io.h>

int main() {

	//PB0 = led
	//PD2 = button

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	//Initialize the PD2 pin as an input
	DDRD = (0 << PD2);

	while (1) {
		if (!(PIND & (1 << PD2))) {
			PORTB = (1 << PB0);
		}
		else {
			PORTB = (0 << PB0);
		}
	}

	return (0);
}