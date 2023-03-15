#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

float volatile ratio = 0.9;
float volatile direction = 0.01;

void set_ratio() {
	OCR1A = (F_CPU / 1024 - 1) * ratio;
}

//Interrupt when timer0 reach top
ISR (TIMER0_OVF_vect) {
	ratio += direction;
	if (ratio == 0) {
		direction = 0.01;
	}
	if (ratio == 1) {
		direction = -0.01;
	}
	set_ratio();
}

int main() {
	//PB1 (= OC1A) = led
	//TCNT1 = main timer
	//TCNT0 = timer to change ratio

	//Initialize the PB1 pin as an output
	DDRB = (1 << PB1);
	DDRB = (1 << PB2);

	//Initialize the timers
	TCNT1 = 0;
	TCNT0 = 0;

	//Set the timer1 to Fast PWM mode
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12);

	//Set the timer0 to interrupt and clear on overflow
	TIFR0  = (1 << TOV0);
	TIMSK0 = (1 << TOIE0);

	//Set the timer1 prescaler to 1024 (16MHz / 1024 = 15625Hz)
	TCCR1B |= (1 << CS12) | (1 << CS10);

	//Set the timer0 prescalar to 1024 (16MHz / 1024 = 15625Hz)
	TCCR0B |= (1 << CS02) | (1 << CS00);
	
	//Set the timer1 top value to 1s (15625 * 1 = 15625)
	ICR1 = F_CPU / 1024 - 1;

	//Set the timer0 top value to 0.01s (15625 * 0.01 = 156)
	TCNT0 = (F_CPU / 1024 - 1) * 0.01;

	set_ratio();

	//On the PB1 pin when timer1 reaches the comparing value
	//Off the PB1 pin when timer1 is on bottom
	TCCR1A |= (1 << COM1A1);
	TCCR1A |= (1 << COM1A0);

	//Enable global interrupts
	sei();

	while (1) {
		//Do nothing
	}
}