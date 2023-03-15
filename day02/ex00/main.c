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

//Detect interupt on INT0
ISR (INT0_vect)  {
	PORTB ^= (1 << PB0);
	wait_ms(20);
}

int main() {
	//PB0 = led
	//PD2 (= INT0) = button

	//Set led as output
	DDRB = (1 << PB0);

	//Set button as input
	DDRD = (0 << PD2);

	//Enable interupt on INT0
	EIMSK = (1 << INT0);

	//Make press (0->1) on INT0 generate an interupt
	EICRA = (1 << ISC01) | (0 << ISC00);

	//Enable global interupt
	sei();

	while (1) {
	}
}