#include "pins.h"

int8_t extract_port(PIN pin) {
	switch (pin) {
		case LED_D1:
			return PB0;
		case LED_D2:
			return PB1;
		case LED_D3:
			return PB2;
		case LED_D4:
			return PB4;
		case BUTTON_SW1:
			return PD2;
		case BUTTON_SW2:
			return PD4;
		case LED_RGB_R:
			return PD5;
		case LED_RGB_G:
			return PD6;
		case LED_RGB_B:
			return PD3;
		default:
			return -1;
	}
}

void pin_mode(PIN pin, PIN_MODE mode) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		if (mode == OUTPUT) {
			DDRB |= (1 << extract_port(pin));
		} else {
			DDRB &= ~(1 << extract_port(pin));
		}
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		if (mode == OUTPUT) {
			DDRD |= (1 << extract_port(pin));
		} else {
			DDRD &= ~(1 << extract_port(pin));
		}
	}
}

void pin_set(PIN pin, PIN_STATE state) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		if (state == HIGH) {
			PORTB |= (1 << extract_port(pin));
		} else {
			PORTB &= ~(1 << extract_port(pin));
		}
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		if (state == HIGH) {
			PORTD |= (1 << extract_port(pin));
		} else {
			PORTD &= ~(1 << extract_port(pin));
		}
	}
}

PIN_STATE pin_state(PIN pin) {
	if (pin >= LED_D4 && pin <= LED_D1) {
		return (PINB & (1 << extract_port(pin))) ? HIGH : LOW;
	} else if (pin >= BUTTON_SW1 && pin <= LED_RGB_B) {
		return (PIND & (1 << extract_port(pin))) ? HIGH : LOW;
	}
	return LOW;
}

PIN_STATE button_pressed(PIN pin) {
	if (pin >= BUTTON_SW1 && pin <= BUTTON_SW2) {
		return (PIND & (1 << extract_port(pin))) ? LOW : HIGH;
	}
	return LOW;
}

void adc_init() {
	// Configure ADC to use AVCC as reference
	ADMUX |= (1 << REFS0);
	// Set prescaler to 128
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(PIN pin) {
	//Enable ADC
	ADCSRA |= (1 << ADEN);

	int mux0, mux1, mux2, mux3 = 0;
	if (pin == POTENTIOMETER) {
		mux0 = 0;
		mux1 = 0;
		mux2 = 0;
		mux3 = 0;
	} else if (pin == LDR) {
		mux0 = 1;
		mux1 = 0;
		mux2 = 0;
		mux3 = 0;
	} else if (pin == NTC) {
		mux0 = 0;
		mux1 = 1;
		mux2 = 0;
		mux3 = 0;
	} else {
		return 0;
	}

	// Choose ADC channel
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

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	// Read ADC value
	uint16_t res = ADCL;
	res |= ((ADCH & 0b00000011) << 8);

	// Disable ADC
	ADCSRA &= ~(1 << ADEN);

	return res;
}

uint16_t adc_read_intern_temp() {
	//Enable ADC
	ADCSRA |= (1 << ADEN);

	ADMUX |= (1 << MUX3);
	ADMUX &= ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to finish
	while (ADCSRA & (1 << ADSC));

	// Read ADC value
	uint16_t res = (ADCL | (ADCH << 8)) - 65565;

	// Disable ADC
	ADCSRA &= ~(1 << ADEN);

	return res;
}

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