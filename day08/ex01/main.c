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

int main() {
	init_spi_as_master();

	spi_transmit(0x32);

	apa102_led led_red = {0x03, 0xff, 0x00, 0x00};
	apa102_led led_green = {0x03, 0x00, 0xff, 0x00};
	apa102_led led_blue = {0x03, 0x00, 0x00, 0xff};
	apa102_led led_yellow = {0x03, 0xff, 0xff, 0x00};
	apa102_led led_cyan = {0x03, 0x00, 0xff, 0xff};
	apa102_led led_magenta = {0x03, 0xff, 0x00, 0xff};
	apa102_led led_white = {0x03, 0xff, 0xff, 0xff};
	apa102_led led_null = {0x00, 0x00, 0x00, 0x0};

	for(;;) {
		apa102_set_all_leds(led_red, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_green, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_blue, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_yellow, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_cyan, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_magenta, led_null, led_null);
		_delay_ms(1000);
		apa102_set_all_leds(led_white, led_null, led_null);
		_delay_ms(1000);
	}
}