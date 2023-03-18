#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

#define UART_BAUDRATE 115200

#define AHTR20_ADDR 0x38

void uart_init() {
	//Set baud rate
	UBRR0L = (( F_CPU / 8 / UART_BAUDRATE ) - 1) / 2;

	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	//Set frame format: 8data, 1stop bit
	UCSR0C = (0<<USBS0) | (3<<UCSZ00);

	//Enable interrupts
	UCSR0B |= (1<<RXCIE0);

	//Enable global interrupts
	sei();
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
		default:
			uart_printstr("Unknown status\r\n");
			break;
	}
}

void wait_i2c_send() {
	while (!(TWCR & (1<<TWINT)));
}

//Init I2C to communicate with AHT20 (U3)
//Frequency: 100kHz
void i2c_init(void) {
	TWBR = 0x0C;
	TWCR = 0;
}

//Start I2C communication in writing mode
void i2c_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c_send();
	print_i2c_status();

	TWDR = AHTR20_ADDR;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c_send();
	print_i2c_status();
}

//Interrupt I2C communication
void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

int main() {
	uart_init();
	i2c_init();
	i2c_start();
	i2c_stop();
}