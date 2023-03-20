#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

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

#define MAGIC_NUMBER 0x42

int8_t value = 0;

void display_value() {
	PORTB = (((value & 1) ? 1 : 0) << PB0) | (((value & 2) ? 1 : 0) << PB1) | (((value & 4) ? 1 : 0) << PB2) | (((value & 8) ? 1 : 0) << PB4);
}

void save_value() {
	eeprom_write_byte((uint8_t *)0, value);
}

void load_value() {
	uint8_t magic = eeprom_read_byte((uint8_t *)1);
	if (magic == MAGIC_NUMBER) {
		value = eeprom_read_byte((uint8_t *)0);
	} else {
		eeprom_write_byte((uint8_t *)1, MAGIC_NUMBER);
		value = 0;
		save_value();
	}
	display_value();
}

//Detect interupt on INT0
ISR (INT0_vect)  {
	value++;
	if (value > 15) {
		value = 0;
	}
	display_value();
	save_value();
	_delay_ms(50);
}

int main() {
	//Set led as output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

	//Set button as input
	DDRD = (0 << PD2) | (0 << PD4);

	//Enable interupt on INT0
	EIMSK = (1 << INT0);
	//Make press (0->1) on INT0 generate an interupt
	EICRA = (1 << ISC01) | (0 << ISC00);

	uart_init();
	load_value();

	sei();

	for(;;);
}