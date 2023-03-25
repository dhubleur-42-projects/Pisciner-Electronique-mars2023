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

void uart_ln() {
	uart_printstr("\r\n");
}

void uart_printstrln(char *str) {
	uart_printstr(str);
	uart_ln();
}

void uart_printnbr(int n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_u8(uint8_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_u8(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_8(int8_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_8(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_u16(uint16_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_u16(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_16(int16_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_16(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_u32(uint32_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_u32(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_printnbr_32(int32_t n) {
	if (n < 0) {
		uart_printchar('-');
		n = -n;
	}

	if (n / 10) {
		uart_printnbr_16(n / 10);
	}

	uart_printchar(n % 10 + '0');
}

void uart_print_byte_hexa(uint8_t n) {
	char base[] = "0123456789ABCDEF";
	uart_printchar(base[n >> 4]);
	uart_printchar(base[n & 0x0F]);
}

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

void bzero(char *s, int n) {
	for (int i = 0; i < n; i++) {
		s[i] = 0;
	}
}

char buffer[UART_RX_BUFFER_SIZE + 1];
int buffer_index = 0;
readline_callback callback = 0;
char readline_echo = 0;

ISR(USART_RX_vect) {
	char c = UDR0;
	//Enter
		if (c == '\r') {
			if (buffer_index == 0)
				return;
			uart_printstr("\r\n");
			if (callback != 0) {
				callback(buffer);
				UCSR0B &= ~(1<<RXCIE0);
			}
			return;
		}

		//Backspace
		if (c == 0x7f) {
			if (buffer_index > 0) {
				buffer_index -= 1;
				buffer[buffer_index] = 0;
				uart_printstr("\b \b");
				return;
			}
		}

		//Other
		if (buffer_index < UART_RX_BUFFER_SIZE) {
			buffer[buffer_index] = c;
			buffer_index += 1;
		} else {
			return;
		}

		//Echo
		if (readline_echo == 0)
			uart_printchar(c);
		else
			uart_printchar(readline_echo);
}

void start_uart_async_readline(readline_callback callback, char echo) {
	bzero(buffer, 100);
	buffer_index = 0;
	UCSR0B |= (1<<RXCIE0);
	readline_echo = echo;
}