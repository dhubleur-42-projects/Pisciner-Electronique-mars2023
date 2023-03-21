#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

# define LED_R PD5
# define LED_G PD6
# define LED_B PD3

void init_rgb() {
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);
	PORTD = 0;

	TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1); 
	TCCR0B = (1 << CS00);
	OCR0A = 0;
	OCR0B = 0;

	TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2B1);
	TCCR2B = (1 << CS20);
	OCR2B = 0;
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
	OCR0B = r;
	OCR0A = g;
	OCR2B = b;
}

int ft_strlen(char *str) {
	int i = 0;
	while (str[i])
		i++;
	return i;
}

int isInHexBase(char c) {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

void toLower(char *buffer) {
	for (int i = 0; i < ft_strlen(buffer); i++) {
		if (buffer[i] >= 'A' && buffer[i] <= 'Z')
			buffer[i] += 32;
	}
}

int isHex(char *buffer) {
	if (ft_strlen(buffer) != 7) {
		uart_printstrln("Invalid hex color (must be 7 characters long)");
		return 0;
	}
	if (buffer[0] != '#') {
		uart_printstrln("Invalid hex color (must start with #)");
		return 0;
	}
	for (int i = 1; i < 7; i++) {
		if (!isInHexBase(buffer[i])) {
			uart_printstrln("Invalid hex color (must be in hex base)");
			return 0;
		}
	}
	toLower(buffer);
	return 1;
}

char *ft_strchr(char *str, char c) {
	for (int i = 0; i < ft_strlen(str); i++) {
		if (str[i] == c)
			return &str[i];
	}
	return NULL;
}

void convert(char *buffer, uint8_t *r, uint8_t *g, uint8_t *b) {
	char base[16] = "0123456789abcdef";
	*r = (ft_strchr(base, buffer[1]) - base) * 16;
	*r += (ft_strchr(base, buffer[2]) - base);

	*g = (ft_strchr(base, buffer[3]) - base) * 16;
	*g += (ft_strchr(base, buffer[4]) - base);

	*b = (ft_strchr(base, buffer[5]) - base) * 16;
	*b += (ft_strchr(base, buffer[6]) - base);
}

int main() {
	uart_init();
	init_rgb();

	char buf[16];
	for(;;) {
		uart_printstr("> ");
		bzero(buf, 16);
		uart_readline(buf, 15, 0);
		if (!isHex(buf))
			continue;
		uint8_t r, g, b;
		convert(buf, &r, &g, &b);
		uart_printstr("Set led to R: ");
		uart_printnbr_u8(r);
		uart_printstr(" G: ");
		uart_printnbr_u8(g);
		uart_printstr(" B: ");
		uart_printnbr_u8(b);
		uart_ln();
		set_rgb(r, g, b);
	}
}