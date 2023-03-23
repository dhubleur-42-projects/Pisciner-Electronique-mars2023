#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"
#include "pins.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

typedef struct s_apa102_led {
	uint8_t brightness;
	uint8_t r;
	uint8_t g;
	uint8_t b;
} apa102_led;

volatile apa102_led led1 = {0, 0, 0, 0};
volatile apa102_led led2 = {0, 0, 0, 0};
volatile apa102_led led3 = {0, 0, 0, 0};

void init_spi_as_master() {
	// Set MOSI, SCK and SS as output
	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2);
	// Enable SPI, Master, set clock rate fck/16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void spi_transmit(uint8_t data) {
	// Start transmission
	SPDR = data;
	// Wait for transmission complete
	while(!(SPSR & (1 << SPIF)));
}

void apa102_start_frame() {
	spi_transmit(0x00);
	spi_transmit(0x00);
	spi_transmit(0x00);
	spi_transmit(0x00);
}

void apa102_set_led(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b) {
	spi_transmit(0xe0+brightness);
	spi_transmit(b);
	spi_transmit(g);
	spi_transmit(r);
}

void apa102_end_frame() {
	spi_transmit(0xff);
	spi_transmit(0xff);
	spi_transmit(0xff);
	spi_transmit(0xff);
}

void apa102_set_all_leds(apa102_led led1, apa102_led led2, apa102_led led3) {
	apa102_start_frame();
	apa102_set_led(led1.brightness, led1.r, led1.g, led1.b);
	apa102_set_led(led2.brightness, led2.r, led2.g, led2.b);
	apa102_set_led(led3.brightness, led3.r, led3.g, led3.b);
	apa102_end_frame();
}

int isInHexBase(char c) {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int ft_strlen(char *str) {
	int i = 0;
	while (str[i])
		i++;
	return i;
}

int ft_strcmp(char *str1, char *str2) {
	int i = 0;
	while (str1[i] && str2[i] && str1[i] == str2[i])
		i++;
	return str1[i] - str2[i];
}

char *ft_strchr(char *str, char c) {
	for (int i = 0; i < ft_strlen(str); i++) {
		if (str[i] == c)
			return &str[i];
	}
	return NULL;
}

void wheel(uint8_t pos) {
	apa102_led led = {0x03, 0, 0, 0};
	pos = 255 - pos;
	if (pos < 85) {
		led.r = 255 - pos * 3;
		led.g = 0;
		led.b = pos * 3;
	} else if (pos < 170) {
		pos = pos - 85;
		led.r = 0;
		led.g = pos * 3;
		led.b = 255 - pos * 3;
	} else {
		pos = pos - 170;
		led.r = pos * 3;
		led.g = 255 - pos * 3;
		led.b = 0;
	}
	apa102_set_all_leds(led, led, led);
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t pos = 0;
	wheel(pos);
	pos++;
}

void start_rainbow() {
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = (F_CPU / 1024) / 255;
	TIMSK1 |= (1 << OCIE1A);
}

void stop_rainbow() {
	TCCR1B &= ~((1 << WGM12) | (1 << CS12) | (1 << CS10));
	TIMSK1 &= ~(1 << OCIE1A);
}

int isFormat(char buffer[16], uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *led) {
	if (buffer[0] != '#')
		return 0;
	for (int i = 1; i < 7; i++) {
		if (!isInHexBase(buffer[i]))
			return 0;
	}
	char base[16] = "0123456789abcdef";
	*r = (ft_strchr(base, buffer[1]) - base) * 16;
	*r += (ft_strchr(base, buffer[2]) - base);
	*g = (ft_strchr(base, buffer[3]) - base) * 16;
	*g += (ft_strchr(base, buffer[4]) - base);
	*b = (ft_strchr(base, buffer[5]) - base) * 16;
	*b += (ft_strchr(base, buffer[6]) - base);
	if (buffer[7] != 'D')
		return 0;
	if (buffer[8] != '6' && buffer[8] != '7' && buffer[8] != '8')
		return 0;
	*led = buffer[8] - '0';
	return 1;
}

void parse(char buffer[16]) {
	if (ft_strlen(buffer) == 9) {
		uint8_t r, g, b, led;
		if (isFormat(buffer, &r, &g, &b, &led)) {
			stop_rainbow();
			if (led == 6) {
				if (r == 0 && g == 0 && b == 0) {
					led1.brightness = 0;
					led1.r = 0;
					led1.g = 0;
					led1.b = 0;
				} else {
					led1.brightness = 0x03;
					led1.r = r;
					led1.g = g;
					led1.b = b;
				}
			} else if (led == 7) {
				if (r == 0 && g == 0 && b == 0) {
					led2.brightness = 0;
					led2.r = 0;
					led2.g = 0;
					led2.b = 0;
				} else {
					led2.brightness = 0x03;
					led2.r = r;
					led2.g = g;
					led2.b = b;
				}
			} else if (led == 8) {
				if (r == 0 && g == 0 && b == 0) {
					led3.brightness = 0;
					led3.r = 0;
					led3.g = 0;
					led3.b = 0;
				} else {
					led3.brightness = 0x03;
					led3.r = r;
					led3.g = g;
					led3.b = b;
				}
			}
			apa102_set_all_leds(led1, led2, led3);
		} else {
			uart_printstrln("Invalid command");
		}
	} else if (ft_strlen(buffer) == 12) {
		if (ft_strcmp(buffer, "#FULLRAINBOW") == 0) {
			start_rainbow();
			led1.brightness = 0x0;
			led1.r = 0;
			led1.g = 0;
			led1.b = 0;
			led2.brightness = 0x0;
			led2.r = 0;
			led2.g = 0;
			led3.brightness = 0x0;
			led3.r = 0;
			led3.g = 0;
			led3.b = 0;
		} else {
			uart_printstrln("Invalid command");
		}
	} else {
		uart_printstrln("Invalid command");
	}
}

int main() {
	init_spi_as_master();
	uart_init();

	apa102_led led_null = {0x00, 0x00, 0x00, 0x0};
	apa102_set_all_leds(led_null, led_null, led_null);

	sei();

	char buf[16];
	for(;;) {
		uart_printstr("> ");
		bzero(buf, 16);
		uart_readline(buf, 15, 0);
		parse(buf);
	}
}