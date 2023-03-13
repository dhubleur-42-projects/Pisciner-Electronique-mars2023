#include <avr/io.h>
#include <util/delay.h>

void wait_ms(uint16_t ms) {
	for (uint16_t i = 0; i < ms; i++) {
		_delay_ms(1);
	}
}

int main() {

	//PB0 (Pin0 of PORTB) = led

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	while (1) {
		//Switch the PB0 state
		PORTB ^= (1 << PB0);
		wait_ms(500);
	}

	return (0);
}