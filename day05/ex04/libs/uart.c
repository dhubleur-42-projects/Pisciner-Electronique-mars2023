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
void uart_printchar(char c) {
	//Wait for empty transmit buffer
	while (!( UCSR0A & (1<<UDRE0)));

	//Put data into buffer, sends the data
	UDR0 = c;
}

void uart_printstr(char *str) {
	while (*str) {
		uart_printchar(*str);
		str++;
	}
}

void uart_printbyte(uint8_t byte)
{
	for(uint8_t i = 8; i > 0; i--)
		uart_printchar(((byte >> (i - 1)) & 1) + '0');
}

void uart_nl() {
	uart_printstr("\r\n");
}

void uart_printstrnl(char *str) {
	uart_printstr(str);
	uart_nl();
}

void uart_printbr_8(int8_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printbr_8(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printbr_16(uint16_t n) {
	if (n / 10) {
		uart_printbr_16(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printbr_32(int32_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printbr_32(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

//Read a character from PC serial port
char uart_readchar(void) {
	//Wait for data to be received
	while (!(UCSR0A & (1<<RXC0)));

	//Get and return received data from buffer
	return UDR0;
}

void uart_readline(char *buffer, int size, char echo) {
	int len = 0;
	while (1) {
		char c = uart_readchar();
		
		//Enter
		if (c == '\r') {
			if (len == 0)
				continue;
			uart_printstr("\r\n");
			return;
		}

		//Backspace
		if (c == 0x7f) {
			if (len > 0) {
				len -= 1;
				buffer[len] = 0;
				uart_printstr("\b \b");
			}
			continue;
		}

		//Other
		if (len < size) {
			buffer[len] = c;
			len += 1;
		} else {
			continue;
		}

		//Echo
		if (echo == 0)
			uart_printchar(c);
		else
			uart_printchar(echo);
	}
}