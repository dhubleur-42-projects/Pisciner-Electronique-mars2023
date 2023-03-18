#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

#define UART_BAUDRATE 115200

#define AHTR20_ADDR 0x38

#define SCL_FREQUENCY 100000
#define TWI_BITRATE (((F_CPU / SCL_FREQUENCY) - 16) / 2)

typedef enum e_I2C_MODE {
	I2C_WRITE = 0,
	I2C_READ = 1
} I2C_MODE;

void uart_init() {
	//Set baud rate
	UBRR0L = (( F_CPU / 8 / UART_BAUDRATE ) - 1) / 2;

	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	//Set frame format: 8data, 1stop bit
	UCSR0C = (0<<USBS0) | (3<<UCSZ00);

	//Enable interrupts
	UCSR0B |= (1<<RXCIE0);
}

void uart_tx(char c) {
	//Wait for empty transmit buffer
	while (!( UCSR0A & (1<<UDRE0)));

	//Put data into buffer, sends the data
	UDR0 = c;
}

void uart_printstr(char *str) {
	while (*str) {
		uart_tx(*str);
		str++;
	}
}

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

void i2c_start(I2C_MODE mode) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c();

	TWDR = AHTR20_ADDR << 1 | mode;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

void i2c_write(unsigned char data) {
	wait_i2c();
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

uint8_t i2c_read(int ack) {
	TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN);
	wait_i2c();
	return TWDR;
}

void print_hex_value(uint8_t c) {
	char hex[] = "0123456789ABCDEF";
	uart_tx(hex[c >> 4]);
	uart_tx(hex[c & 0x0F]);
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void read_temp_sensor() {
	i2c_start(I2C_WRITE);

	i2c_write(0xac);
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();

	_delay_ms(80);

	i2c_start(I2C_READ);

	uint8_t read_byte = i2c_read(1);
	while (read_byte & 0b10000000) {
		_delay_ms(80);
		read_byte = i2c_read(1);
	}
	print_hex_value(read_byte);
	uart_tx(' ');
	
	for (int i = 0; i < 6; i++) {
		print_hex_value(i2c_read(i != 5));
		uart_tx(' ');
	}
	i2c_stop();

	uart_printstr("\r\n");
}

int main() {
	uart_init();
	i2c_init();
	
	while (1) {
		read_temp_sensor();
		_delay_ms(1000);
	}
}