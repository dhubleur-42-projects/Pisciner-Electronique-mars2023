#include <avr/io.h>

#ifndef F_CPU
# define F_CPU 1000000UL
#endif

int main() {

	//PB0 (Pin0 of PORTB) = led

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	//Set the PB0 pin to HIGH
	PORTB = (1 << PB0);

	return (0);
}