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

//Timer1 overflow interrupt
ISR(TIMER1_COMPA_vect) {
	uart_printstr("Hello World!\r\n");
}

int main() {
	uart_init();
	
	//Initialize the timer
	TCNT1 = 0;

	//Set the timer to CTC mode
	TCCR1B = (1 << WGM12);

	//Enable timer1 overflow interrupt
	TIMSK1 = (1<<OCIE1A);

	//Set the timer1 prescaler to 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	//Set the timer1 to 2s
	OCR1A = (F_CPU / 1024 - 1) * 2;

	//Enable global interrupts
	sei();

	while (1) {
		//Do nothing
	}

}