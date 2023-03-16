#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

#define UART_BAUDRATE 115200

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

//Read a character from PC serial port
char uart_rx(void) {
	//Wait for data to be received
	while (!(UCSR0A & (1<<RXC0)));

	//Get and return received data from buffer
	return UDR0;
}

void bzero(char *str, int len) {
	for (int i = 0; i < len; i++)
		str[i] = 0;
}

int strcmp(char *str1, char *str2) {
	int i = 0;
	while (str1[i] && str2[i] && str1[i] == str2[i])
		i++;
	return str1[i] - str2[i];
}

void ask_user(char *prompt, char buffer[32], char echo) {
	int len = 0;
	uart_printstr(prompt);
	while (1) {
		char c = uart_rx();
		
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
		if (len < 31) {
			buffer[len] = c;
			len += 1;
		} else {
			continue;
		}

		//Echo
		if (echo == 0)
			uart_tx(c);
		else
			uart_tx(echo);
	}
}

void led_init() {
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
}

void led_blink() {
	for (int i = 0; i < 5; i++) {
		PORTB = (1 << PB0);
		_delay_ms(50);
		PORTB = (1 << PB1);
		_delay_ms(50);
		PORTB = (1 << PB2);
		_delay_ms(50);
		PORTB = (1 << PB4);
		_delay_ms(50);
	}
	PORTB = 0;
}

int main() {
	uart_init();
	led_init();

	for(;;) {
		char username[32];
		bzero(username, 32);
		char password[32];
		bzero(password, 32);

		uart_printstr("Login:\r\n");

		ask_user("\tUsername: ", username, 0);
		ask_user("\tPassword: ", password, '*');

		if (strcmp(username, "dhubleur") == 0 && strcmp(password, "1234567890") == 0) {
			uart_printstr("Hello ");
			uart_printstr(username);
			uart_printstr("!\r\n");
			led_blink();
			break;
		} else {
			uart_printstr("Invalid username or password!\r\n");
		}
	}

}