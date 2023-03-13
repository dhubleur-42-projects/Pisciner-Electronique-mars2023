#include <avr/io.h>
#include <util/delay.h>

void wait_ms(int ms) {
	for (int i = 0; i < ms; i++) {
		_delay_ms(1);
	}
}

int plusButtonPressDetection() {
	static int state = 0;
	int buttonState = !(PIND & (1 << PD2));
	if (state == 0 && buttonState == 1) {
		state = 1;
		_delay_ms(20);
		return (1);
	}
	else if (state == 1 && buttonState == 0) {
		state = 0;
		_delay_ms(20);
	}
	return (0);
}

int minusButtonPressDetection() {
	static int state = 0;
	int buttonState = !(PIND & (1 << PD4));
	if (state == 0 && buttonState == 1) {
		state = 1;
		_delay_ms(20);
		return (1);
	}
	else if (state == 1 && buttonState == 0) {
		state = 0;
		_delay_ms(20);
	}
	return (0);
}

int main() {

	//PB0 = led 1 
	//PB1 = led 2 
	//PB2 = led 4 
	//PB4 = led 4
	//PD2 = button +
	//PD4 = button -

	//Initialize the PB0/1/2/4 pin as an output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

	//Initialize the PD2/3 pin as an input
	DDRD = (0 << PD2) | (0 << PD4);

	int value = 0;

	while (1) {
		int changed = 0;
		if (plusButtonPressDetection()) {
			value++;
			if (value > 15) {
				value = 0;
			}
			changed = 1;
		}
		if (minusButtonPressDetection()) {
			value--;
			if (value < 0) {
				value = 15;
			}
			changed = 1;
		}
		if(changed) {
			PORTB = (((value & 1) ? 1 : 0) << PB0) | (((value & 2) ? 1 : 0) << PB1) | (((value & 4) ? 1 : 0) << PB2) | (((value & 8) ? 1 : 0) << PB4);
		}
	}

	return (0);
}