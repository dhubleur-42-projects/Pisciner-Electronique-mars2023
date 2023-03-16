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
	UCSR0C = (1<<USBS0) | (3<<UCSZ00);

	//Enable interrupts
	UCSR0B |= (1<<RXCIE0);

	//Enable global interrupts
	sei();
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
// char uart_rx(void) {
// 	//Wait for data to be received
// 	while (!(UCSR0A & (1<<RXC0)));

// 	//Get and return received data from buffer
// 	return UDR0;
// }

ISR(USART_RX_vect) {
	char c = UDR0;
	if (c >= 'a' && c <= 'z') {
		c -= 32;
	} else if (c >= 'A' && c <= 'Z') {
		c += 32;
	}
	uart_tx(c);
	uart_printstr("\r\n");
}

int main() {
	uart_init();

	while (1) {
	}

}