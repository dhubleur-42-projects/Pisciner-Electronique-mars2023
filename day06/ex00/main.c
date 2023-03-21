#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

# define LED_R PD5
# define LED_G PD6
# define LED_B PD3

int main() {
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);
	
	for(;;) {
		PORTD = (1 << LED_R);
		_delay_ms(1000);
		PORTD = (1 << LED_G);
		_delay_ms(1000);
		PORTD = (1 << LED_B);
		_delay_ms(1000);
	}
}