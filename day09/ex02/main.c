#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"
#include "pins.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

# define I2C_EXPANDER_ADDR 0x20

int main() {
	i2c_init();
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_0, 0b00000001);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_1, 0b00000000);

	//Select CA4
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b01111111);
	//Write 2
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_1, 0b01011011);

	
	for(;;) {
	}
}