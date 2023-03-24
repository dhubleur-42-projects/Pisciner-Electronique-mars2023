#ifndef I2C_H
# define I2C_H

# include <avr/io.h>
# include "uart.h"

# define SCL_FREQUENCY 100000
# define TWI_BITRATE (((F_CPU / SCL_FREQUENCY) - 16) / 2)

typedef enum e_I2C_MODE {
	I2C_WRITE = 0,
	I2C_READ = 1
} I2C_MODE;

typedef enum e_ACK {
	ACK = 1,
	NACK = 0
} ACK_TYPE;

typedef enum e_I2C_EXPANDER_PORT {
	INPUT_PORT_0 = 0,
	INPUT_PORT_1 = 1,
	OUTPUT_PORT_0 = 2,
	OUTPUT_PORT_1 = 3,
	POLARITY_INVERSION_PORT_0 = 4,
	POLARITY_INVERSION_PORT_1 = 5,
	CONFIGURATION_PORT_0 = 6,
	CONFIGURATION_PORT_1 = 7
} I2C_EXPANDER_PORT;

void wait_i2c();
void i2c_init(void);
void i2c_start(uint8_t address, I2C_MODE mode);
void i2c_stop(void);
void i2c_write(unsigned char data);
uint8_t i2c_read(ACK_TYPE ack);
void i2c_send_command(uint8_t address, uint8_t command, uint8_t *params, int params_length);
int8_t i2c_expander_read_port(uint8_t address, I2C_EXPANDER_PORT port);
void i2c_expander_write_port(uint8_t address, I2C_EXPANDER_PORT port, uint8_t data);
void i2c_expander_set_pin(uint8_t address, I2C_EXPANDER_PORT port, uint8_t pin, uint8_t value);

#endif