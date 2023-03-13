#include <avr/io.h>

int main() {

	//PB0 (located in register B) = led

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	//Set the PB0 pin to HIGH
	PORTB = (1 << PB0);

	return (0);
}