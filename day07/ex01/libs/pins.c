#include "pins.h"

int8_t extract_port(PIN pin) {
	switch (pin) {
		case LED_D1:
			return PB0;
		case LED_D2:
			return PB1;
		case LED_D3:
			return PB2;
		case LED_D4:
			return PB3;
		case BUTTON_SW1:
			return PD2;
		case BUTTON_SW2:
			return PD4;
		case LED_RGB_R:
			return PD5;
		case LED_RGB_G:
			return PD6;
		case LED_RGB_B:
			return PD3;
		default:
			return -1;
	}
}

void pin_mode(PIN pin, PIN_MODE mode) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		if (mode == OUTPUT) {
			DDRB |= (1 << extract_port(pin));
		} else {
			DDRB &= ~(1 << extract_port(pin));
		}
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		if (mode == OUTPUT) {
			DDRD |= (1 << extract_port(pin));
		} else {
			DDRD &= ~(1 << extract_port(pin));
		}
	}
}

void pin_set(PIN pin, PIN_STATE state) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		if (state == HIGH) {
			PORTB |= (1 << extract_port(pin));
		} else {
			PORTB &= ~(1 << extract_port(pin));
		}
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		if (state == HIGH) {
			PORTD |= (1 << extract_port(pin));
		} else {
			PORTD &= ~(1 << extract_port(pin));
		}
	}
}

PIN_STATE pin_state(PIN pin) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		return (PINB & (1 << extract_port(pin))) ? HIGH : LOW;
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		return (PIND & (1 << extract_port(pin))) ? HIGH : LOW;
	}
	return LOW;
}

PIN_STATE button_pressed(PIN pin) {
	if (pin >= BUTTON_SW1 && pin <= BUTTON_SW2) {
		return (PIND & (1 << extract_port(pin))) ? LOW : HIGH;
	}
	return LOW;
}