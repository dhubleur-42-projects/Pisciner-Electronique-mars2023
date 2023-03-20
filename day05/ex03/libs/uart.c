#include "uart.h"

//Intitialize the UART
void uart_init() {
	//Set baud rate
	UBRR0L = (( F_CPU / 8 / UART_BAUDRATE ) - 1) / 2;

	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	//Set frame format: 8data, 1stop bit
	UCSR0C = (0<<USBS0) | (3<<UCSZ00);
}

//Write a character to PC serial port
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

void uart_printbyte(uint8_t byte)
{
	for(uint8_t i = 8; i > 0; i--)
		uart_tx(((byte >> (i - 1)) & 1) + '0');
}

void uart_nl() {
	uart_printstr("\r\n");
}

void uart_printstrnl(char *str) {
	uart_printstr(str);
	uart_nl();
}

void uart_printbr(int n) {
	if (n < 0) {
		uart_tx('-');
		n = -n;
	}

	if (n / 10) {
		uart_printbr(n / 10);
	}

	uart_tx(n % 10 + '0');
}