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

uint8_t led = 1;
uint8_t color = 1;
volatile apa102_led led1 = {0x03, 0, 0, 0};
volatile apa102_led led2 = {0x03, 0, 0, 0};
volatile apa102_led led3 = {0x03, 0, 0, 0};

int sw2ButtonPressDetection() {
	static int state = 0;
	int buttonState = !(PIND & (1 << PD2));
	if (state == 0 && buttonState == 1) {
		state = 1;
		_delay_ms(20);
		return (1);
	}
	else if (state == 1 && buttonState == 0) {
		state = 0;
		_delay_ms(20);
	}
	return (0);
}

int sw3ButtonPressDetection() {
	static int state = 0;
	int buttonState = !(PIND & (1 << PD4));
	if (state == 0 && buttonState == 1) {
		state = 1;
		_delay_ms(20);
		return (1);
	}
	else if (state == 1 && buttonState == 0) {
		state = 0;
		_delay_ms(20);
	}
	return (0);
}

int main() {
	init_spi_as_master();

	DDRC &= ~(1 << PC0);
	adc_init(0, 0, 0, 0);

	pin_mode(LED_D1, OUTPUT);
	pin_mode(LED_D2, OUTPUT);
	pin_mode(LED_D3, OUTPUT);
	pin_set(LED_D1, HIGH);
	pin_set(LED_D2, LOW);
	pin_set(LED_D3, LOW);

	DDRD |= (0 << PD2) | (0 << PD4);

	apa102_led currentLed = {0x03, 0, 0, 0};

	for(;;) {
		if (sw2ButtonPressDetection()) {
			color++;
			if (color > 3) {
				color = 1;
			}
		}
		if (sw3ButtonPressDetection()) {
			if (led == 1) {
				led1 = currentLed;
				currentLed = led2;
				led = 2;
				pin_set(LED_D1, LOW);
				pin_set(LED_D2, HIGH);
			} else if (led == 2) {
				led2 = currentLed;
				currentLed = led3;
				led = 3;
				pin_set(LED_D2, LOW);
				pin_set(LED_D3, HIGH);
			} else if (led == 3) {
				led3 = currentLed;
				currentLed = led1;
				led = 1;
				pin_set(LED_D3, LOW);
				pin_set(LED_D1, HIGH);
			}
		}
		uint8_t adcValue = read_adc();
		if (color == 1) {
			currentLed.r = adcValue;
		} else if (color == 2) {
			currentLed.g = adcValue;
		} else if (color == 3) {
			currentLed.b = adcValue;
		}
		if (led == 1) {
			apa102_set_all_leds(currentLed, led2, led3);
		} else if (led == 2) {
			apa102_set_all_leds(led1, currentLed, led3);
		} else if (led == 3) {
			apa102_set_all_leds(led1, led2, currentLed);
		}
	}
}