#include <avr/io.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

int main() {

	//PB1 (= OC1A) = led
	//TCNT1 = timer

	//Initialize the PB1 pin as an output
	DDRB = (1 << PB1);

	//Initialize the timer
	TCNT1 = 0;

	//Set the timer to CTC mode (Clear Timer on Compare Match)
	TCCR1A = 0;
	TCCR1B = (1 << WGM12);

	//Set the prescaler to 1024 (16MHz / 1024 = 15625Hz)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	//Set the comparing value to blink every 0.5s (15625 / 2 = 7812)
	OCR1A = 7812;
	
	//Setup timer to change OC1A (PB1) state on compare match
	TCCR1A |= (1 << COM1A0);

	while (1) {
		//Do nothing
	}
	
}