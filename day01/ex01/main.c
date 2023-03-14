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

	//Set the timer to Fast PWM mode
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12);

	//Set the prescaler to 1024 (16MHz / 1024 = 15625Hz)
	TCCR1B |= (1 << CS12) | (1 << CS10);
	
	//Set the top value to 1s (15625 * 1 = 15625)
	ICR1 = 15625;

	//Set the comparing value to blink after 0.9s (15625 * 0.9 = 14062)
	OCR1A = 14062;

	//Off the PB1 pin when timer reaches the top value
	TCCR1A |= (1 << COM1A1);
	//On the PB1 pin when timer reaches the comparing value
	TCCR1A |= (1 << COM1A0);

	while (1) {
		//Do nothing
	}
	
}