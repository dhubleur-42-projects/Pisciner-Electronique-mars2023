#ifndef LEDS_H
# define LEDS_H

# include <avr/io.h>

typedef enum e_pin {
	LED_D4 = 0,
	LED_D3 = 1,
	LED_D2 = 2,
	LED_D1 = 3,
	BUTTON_SW1 = 4,
	BUTTON_SW2 = 5,
	LED_RGB_R = 6,
	LED_RGB_G = 7,
	LED_RGB_B = 8
} PIN;

typedef enum e_pin_mode {
	OUTPUT = 1,
	INPUT = 0
} PIN_MODE;

typedef enum e_pin_state {
	LOW = 0,
	HIGH = 1
} PIN_STATE;

int8_t extract_port(PIN pin);
void pin_mode(PIN pin, PIN_MODE mode);
void pin_set(PIN pin, PIN_STATE state);
PIN_STATE pin_state(PIN pin);
PIN_STATE button_pressed(PIN pin);

#endif