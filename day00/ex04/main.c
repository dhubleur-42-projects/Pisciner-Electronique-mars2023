#include <avr/io.h>
#include <util/delay.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

int main() {

	//PB0 = led
	//PD2 = button

	//Initialize the PB0 pin as an output
	DDRB = (1 << PB0);

	//Initialize the PD2 pin as an input
	DDRD = (0 << PD2);

	int buttonState = 0;

	while (1) {
		int readState = !(PIND & (1 << PD2));
		if(buttonState == 0 && readState == 1) {
			PORTB ^= (1 << PB0);
			buttonState = 1;
			_delay_ms(20);
		}
		else if(buttonState == 1 && readState == 0) {
			buttonState = 0;
			_delay_ms(20);
		}
	}

	return (0);
}