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

void adc_init(int mux0, int mux1, int mux2, int mux3) {
	// Configure ADC to be left justified, use AVCC as reference
	ADMUX |= (1 << ADLAR) | (1 << REFS0);
	if (mux0) {
		ADMUX |= (1 << MUX0);
	} else {
		ADMUX &= ~(1 << MUX0);
	}
	if (mux1) {
		ADMUX |= (1 << MUX1);
	} else {
		ADMUX &= ~(1 << MUX1);
	}
	if (mux2) {
		ADMUX |= (1 << MUX2);
	} else {
		ADMUX &= ~(1 << MUX2);
	}
	if (mux3) {
		ADMUX |= (1 << MUX3);
	} else {
		ADMUX &= ~(1 << MUX3);
	}
	// Enable ADC and set prescaler to 128
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint8_t read_adc() {
	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	// Read ADC value
	return ADCH;
}

int main() {
	init_spi_as_master();

	DDRC &= ~(1 << PC0);
	adc_init(0, 0, 0, 0);

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
		uint8_t adc_value = read_adc();
		if (adc_value < (255 / 3)) {
			apa102_set_all_leds(led_null, led_null, led_null);
		} else if (adc_value < (255 / 3) * 2) {
			apa102_set_all_leds(led_red, led_null, led_null);
		} else if (adc_value < 255) {
			apa102_set_all_leds(led_red, led_red, led_null);
		} else {
			apa102_set_all_leds(led_red, led_red, led_red);
		}
	}
}