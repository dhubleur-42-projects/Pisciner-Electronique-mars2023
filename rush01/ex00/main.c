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

volatile apa102_led null_led = {0, 0, 0, 0};

void init() {
	i2c_init();
	i2c_expander_init();
	uart_init();
	adc_init();
	init_spi_as_master();
	apa102_set_all_leds(null_led, null_led, null_led);
	pin_mode(LED_D1, OUTPUT);
	pin_set(LED_D1, LOW);
	pin_mode(LED_D2, OUTPUT);
	pin_set(LED_D2, LOW);
	pin_mode(LED_D3, OUTPUT);
	pin_set(LED_D3, LOW);
	pin_mode(LED_D4, OUTPUT);
	pin_set(LED_D4, LOW);
	pin_mode(LED_RGB_B, OUTPUT);
	pin_mode(LED_RGB_G, OUTPUT);
	pin_mode(LED_RGB_R, OUTPUT);
	pin_mode(BUTTON_SW1, INPUT);
	pin_mode(BUTTON_SW2, INPUT);

	sei();
}

void start() {
	pin_set(LED_D1, HIGH);
	pin_set(LED_D2, HIGH);
	pin_set(LED_D3, HIGH);
	pin_set(LED_D4, HIGH);
	_delay_ms(3000);
	pin_set(LED_D1, LOW);
	pin_set(LED_D2, LOW);
	pin_set(LED_D3, LOW);
	pin_set(LED_D4, LOW);
	uart_printstrln("Board started");
}

volatile int sw1_state = 0;
volatile int sw2_state = 0;
volatile int sw3_state = 0;

volatile int mode = 0;

volatile int mode4_state = 0;

ISR(TIMER1_COMPA_vect) {
	if(mode == 4) {
		mode4_state++;
		if (mode4_state > 3) {
			mode4_state = 1;
		}
		if (mode4_state == 1) {
			pin_set(LED_RGB_R, HIGH);
			pin_set(LED_RGB_G, LOW);
			pin_set(LED_RGB_B, LOW);
			apa102_led led = {0x03, 255, 0, 0};
			apa102_set_all_leds(led, led, led);
		} else if (mode4_state == 2) {
			pin_set(LED_RGB_R, LOW);
			pin_set(LED_RGB_G, HIGH);
			pin_set(LED_RGB_B, LOW);
			apa102_led led = {0x03, 0, 255, 0};
			apa102_set_all_leds(led, led, led);
		} else if (mode4_state == 3) {
			pin_set(LED_RGB_R, LOW);
			pin_set(LED_RGB_G, LOW);
			pin_set(LED_RGB_B, HIGH);
			apa102_led led = {0x03, 0, 0, 255};
			apa102_set_all_leds(led, led, led);
		}
	}
}

void init_mode4() {
	i2c_7segment_set_digit(0, 10);
	i2c_7segment_set_digit(1, 4);
	i2c_7segment_set_digit(2, 2);
	i2c_7segment_set_digit(3, 10);

	pin_set(LED_RGB_R, HIGH);
	pin_set(LED_RGB_G, LOW);
	pin_set(LED_RGB_B, LOW);
	apa102_led led = {0x03, 255, 0, 0};
	apa102_set_all_leds(led, led, led);

	//setting timer 1 to 1s
	TCNT1 = 0;
	TCCR1B = (1 << CS12);
	TCCR1B |= (1 << WGM12);
	OCR1A = 62500;
	TIMSK1 = (1 << OCIE1A);
}

void stop_mode4() {
	TCNT1 = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	OCR1A = 0;
	pin_set(LED_RGB_R, LOW);
	pin_set(LED_RGB_G, LOW);
	pin_set(LED_RGB_B, LOW);
	apa102_set_all_leds(null_led, null_led, null_led);
}

void sw_actions() {
	int readed_sw1 = button_pressed(BUTTON_SW1);
	if (readed_sw1 != sw1_state) {
		sw1_state = readed_sw1;
		if (sw1_state == 1) {
			uart_printstrln("SW1 pressed");
		} else {
			uart_printstrln("SW1 released");
		}
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 3, !sw1_state);
		if (sw1_state == 1) {
			i2c_7segment_clear();
			mode++;
			if (mode > 9) {
				mode = 0;
			}
			if (mode == 4) {
				init_mode4();
			} 
			if (mode == 5) {
				stop_mode4();
			}
			uart_printstr("Mode: ");
			uart_printnbr(mode);
			uart_ln();
		}
	}
	int readed_sw2 = button_pressed(BUTTON_SW2);
	if (readed_sw2 != sw2_state) {
		sw2_state = readed_sw2;
		if (sw2_state == 1) {
			uart_printstrln("SW2 pressed");
		} else {
			uart_printstrln("SW2 released");
		}
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 2, !sw2_state);
		if (sw2_state == 1) {
			i2c_7segment_clear();
			mode--;
			if (mode < 0) {
				mode = 9;
			}
			if (mode == 4) {
				init_mode4();
			} 
			if (mode == 3) {
				stop_mode4();
			}
			uart_printstr("Mode: ");
			uart_printnbr(mode);
			uart_ln();
		}
	}
	int readed_sw3 = !i2c_expander_get_pin(I2C_EXPANDER_ADDR, INPUT_PORT_0, 0);
	if (readed_sw3 != sw3_state) {
		sw3_state = readed_sw3;
		if (sw3_state == 1) {
			uart_printstrln("SW3 pressed");
		} else {
			uart_printstrln("SW3 released");
		}
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 1, !readed_sw3);
	}
}

void display_mode() {
	pin_set(LED_D1, (mode & 1) ? HIGH : LOW);
	pin_set(LED_D2, (mode & 2) ? HIGH : LOW);
	pin_set(LED_D3, (mode & 4) ? HIGH : LOW);
	pin_set(LED_D4, (mode & 8) ? HIGH : LOW);
}

void adc_actions() {
	if (mode == 0) {
		i2c_7segment_write_numbers(adc_read(POTENTIOMETER));
	} else if (mode == 1) {
		i2c_7segment_write_numbers(adc_read(LDR));
	} else if (mode == 2) {
		i2c_7segment_write_numbers(adc_read(NTC));
	} else if (mode == 3) {
		i2c_7segment_write_numbers(adc_read_intern_temp());
	}
}

int main() {
	init();
	uart_printstrln("Board initialized");
	// start();
	for(int i = 0;;i++) {
		sw_actions();
		adc_actions();

		display_mode();
		i2c_7segment_display();
	}
}