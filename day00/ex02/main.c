#include <avr/io.h>

#ifndef F_CPU
# define F_CPU 1000000UL
#endif

void wait_ms(uint16_t ms) {
	for (uint16_t i = 0; i < ms; i++) {
		// Compute how many cycles do we need to wait for 1ms
		uint16_t cycles = F_CPU / 1000;		
		for (uint16_t j = 0; j < cycles; j++) {
			asm volatile ("nop");
		}
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