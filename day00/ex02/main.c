#include <avr/io.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

void wait_ms(unsigned long ms) {
	unsigned long cycles = F_CPU / 1000 * ms / 11; // 11 is approx. the number of CPU cycles per loop
	for (unsigned long i = 0; i < cycles; i++) {
		__asm__ __volatile__ ("nop");
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