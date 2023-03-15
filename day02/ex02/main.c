#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

volatile int pressed = 0;

void wait_ms(int ms) {
	for (int i = 0; i < ms; i++) {
		_delay_ms(1);
	}
}

int volatile value = 0;

void set_value() {
	PORTB = (((value & 1) ? 1 : 0) << PB0) | (((value & 2) ? 1 : 0) << PB1) | (((value & 4) ? 1 : 0) << PB2) | (((value & 8) ? 1 : 0) << PB4);
}

//Detect interupt on INT0
ISR (INT0_vect)  {
	value++;
	if (value > 15) {
		value = 0;
	}
	set_value();
	wait_ms(50);
}

int main() {
	//PB0 = led 1 
	//PB1 = led 2 
	//PB2 = led 4 
	//PB4 = led 4
	//PD2 (= INT0) = button +
	//PD4 = button -

	//Set led as output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

	//Set button as input
	DDRD = (0 << PD2) | (0 << PD4);

	//Enable interupt on INT0
	EIMSK = (1 << INT0);
	//Make press (0->1) on INT0 generate an interupt
	EICRA = (1 << ISC01) | (0 << ISC00);

	//Detect interupt on PD4
	PCICR = (1 << PCIE2);
	PCMSK2 = (1 << PCINT20);

	//Enable global interupt
	sei();

	while (1) {
	}
}