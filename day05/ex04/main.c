#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

int main() {
	uart_init();

	i2c_init();

	sei();


	for(;;) {
		
	}
}