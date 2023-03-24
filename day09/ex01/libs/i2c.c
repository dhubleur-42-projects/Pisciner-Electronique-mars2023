#include "i2c.h"

void print_i2c_status() {
	int status = TWSR;
	status &= 0xF8;

	switch (status) {
		case 0x08:
			uart_printstr("Start condition has been transmitted\r\n");
			break;
		case 0x10:
			uart_printstr("Repeated start condition has been transmitted\r\n");
			break;
		case 0x18:
			uart_printstr("SLA+W has been transmitted, ACK has been received\r\n");
			break;
		case 0x20:
			uart_printstr("SLA+W has been transmitted, NACK has been received\r\n");
			break;
		case 0x28:
			uart_printstr("Data has been transmitted, ACK has been received\r\n");
			break;
		case 0x30:
			uart_printstr("Data has been transmitted, NACK has been received\r\n");
			break;
		case 0x38:
			uart_printstr("Arbitration lost in SLA+W or data bytes\r\n");
			break;
		case 0x40:
			uart_printstr("SLA+R has been transmitted, ACK has been received\r\n");
			break;
		case 0x48:
			uart_printstr("SLA+R has been transmitted, NACK has been received\r\n");
			break;
		case 0x50:
			uart_printstr("Data has been received, ACK has been returned\r\n");
			break;
		case 0x58:
			uart_printstr("Data has been received, NACK has been returned\r\n");
			break;
		case 0x00:
			uart_printstr("Bus error\r\n");
			break;
		default:
			uart_printstr("Unknown status\r\n");
			break;
	}
}

void wait_i2c() {
	while (!(TWCR & (1<<TWINT)));
}

void i2c_init(void) {
	TWBR = TWI_BITRATE;
	TWSR = 0;
}

void i2c_start(uint8_t address, I2C_MODE mode) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c();

	TWDR = (address << 1) | mode;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void i2c_write(uint8_t data) {
	wait_i2c();
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

uint8_t i2c_read(ACK_TYPE ack) {
	TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN);
	wait_i2c();
	return TWDR;
}

void i2c_send_command(uint8_t address, uint8_t command, uint8_t *params, int params_length) {
	i2c_start(address, I2C_WRITE);
	i2c_write(command);
	for (int i = 0; i < params_length; i++) {
		i2c_write(params[i]);
	}
	i2c_stop();
}

int8_t i2c_expander_read_port(uint8_t address, I2C_EXPANDER_PORT port) {
	i2c_start(address, I2C_WRITE);
	i2c_write(port);
	i2c_start(address, I2C_READ);
	int8_t data = i2c_read(NACK);
	i2c_stop();
	return data;
}

void i2c_expander_write_port(uint8_t address, I2C_EXPANDER_PORT port, uint8_t data) {
	i2c_start(address, I2C_WRITE);
	i2c_write(port);
	i2c_write(data);
	i2c_stop();
}