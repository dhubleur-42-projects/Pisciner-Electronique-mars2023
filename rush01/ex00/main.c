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
}

volatile int sw1_state = 0;
volatile int sw2_state = 0;
volatile int sw3_state = 0;

volatile int mode = 0;

volatile int mode4_state = 0;

volatile int make_measure = 0;

apa102_led wheel(uint8_t pos) {
	apa102_led led = {0x1f, 0, 0, 0};
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
	return led;
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t unicorn_pos = 0;
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
	if (mode >= 5 && mode <= 10) {
		make_measure = 1;
	}
	if (mode == 11) {
		apa102_set_all_leds(wheel(unicorn_pos), wheel(unicorn_pos + 85), wheel(unicorn_pos + 170));
		unicorn_pos++;
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

void init_measure_timer() {
	TCNT1 = 0;
	TCCR1B = (1 << CS12);
	TCCR1B |= (1 << WGM12);
	OCR1A = 62500;
	TIMSK1 = (1 << OCIE1A);
}

void stop_measure_timer() {
	TCNT1 = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	OCR1A = 0;
}

void init_unicorn() {
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = (F_CPU / 1024) / 255;
	TIMSK1 |= (1 << OCIE1A);
}

void stop_unicorn() {
	TCNT1 = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	OCR1A = 0;
	apa102_set_all_leds(null_led, null_led, null_led);
}

void sw_actions() {
	int readed_sw1 = button_pressed(BUTTON_SW1);
	if (readed_sw1 != sw1_state) {
		sw1_state = readed_sw1;
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 3, !sw1_state);
		if (sw1_state == 1) {
			i2c_7segment_clear();
			mode++;
			if (mode > 11) {
				mode = 0;
			}
			if (mode == 4) {
				init_mode4();
			} 
			if (mode == 5) {
				stop_mode4();
				init_measure_timer();
			}
			if (mode == 11) {
				stop_measure_timer();
				init_unicorn();
			}
			if (mode == 0) {
				stop_unicorn();
			}
		}
	}
	int readed_sw2 = button_pressed(BUTTON_SW2);
	if (readed_sw2 != sw2_state) {
		sw2_state = readed_sw2;
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 2, !sw2_state);
		if (sw2_state == 1) {
			i2c_7segment_clear();
			mode--;
			if (mode < 0) {
				mode = 11;
			}
			if (mode == 11) {
				init_unicorn();
			}
			if (mode == 10) {
				stop_unicorn();
				init_measure_timer();
			}
			if (mode == 4) {
				stop_measure_timer();
				init_mode4();
			} 
			if (mode == 3) {
				stop_mode4();
			}
		}
	}
	int readed_sw3 = !i2c_expander_get_pin(I2C_EXPANDER_ADDR, INPUT_PORT_0, 0);
	if (readed_sw3 != sw3_state) {
		sw3_state = readed_sw3;
		i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 1, !readed_sw3);
	}
}

void display_mode() {
	pin_set(LED_D1, (mode & 1) ? HIGH : LOW);
	pin_set(LED_D2, (mode & 2) ? HIGH : LOW);
	pin_set(LED_D3, (mode & 4) ? HIGH : LOW);
	pin_set(LED_D4, (mode & 8) ? HIGH : LOW);
	if (mode != 4 && mode != 11) {
		apa102_set_all_leds(null_led, null_led, null_led);
	}
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

void temp_actions() {
	if (make_measure && mode >= 5 && mode <= 7) {
		make_measure = 0;
		int32_t res[2] = {0, 0};
		read_temp_sensor(res);
		if (res[0] > -10) {
			if (mode == 5) {
				i2c_7segment_write_numbers_signed(res[0]);
			} else if (mode == 6) {
				int32_t as_farenheit = (res[0] * 9 / 5) + 32;
				i2c_7segment_write_numbers_signed(as_farenheit);
			} else if (mode == 7) {
				i2c_7segment_write_numbers_signed(res[1] / 10);
			}
		}
	}
}

int parse_int8_to_char(char *buffer, int from, char delimiter, uint8_t *data) {
	*data = 0;
	while (buffer[from] != '\0' && buffer[from] != delimiter) {
		if (buffer[from] < '0' || buffer[from] > '9') {
			return -1;
		} 
		*data = *data * 10 + (buffer[from] - '0');
		from++;
	}
	if (buffer[from] == '\0' && delimiter != '\0') {
		return -1;
	}
	return from + 1;
}

int parse_int32_to_char(char *buffer, int from, char delimiter, uint32_t *data) {
	*data = 0;
	while (buffer[from] != '\0' && buffer[from] != delimiter) {
		if (buffer[from] < '0' || buffer[from] > '9') {
			return -1;
		} 
		*data = *data * 10 + (buffer[from] - '0');
		from++;
	}
	if (buffer[from] == '\0' && delimiter != '\0') {
		return -1;
	}
	return from + 1;
}

uint8_t secondes, minutes, hours, day, month;
uint32_t year;
int change_date = 0;

int parse(char *buffer) {
	int from = 0;
	from = parse_int8_to_char(buffer, from, '/', &day);
	if (from == -1 || day > 31 || day < 1) {
		uart_printstrln("Invalid date format");
		return -0;
	}
	from = parse_int8_to_char(buffer, from, '/', &month);
	if (from == -1 || month > 12 || month < 1) {
		uart_printstrln("Invalid date format");
		return -0;
	}
	from = parse_int32_to_char(buffer, from, ' ', &year);
	if (from == -1 || year > 2099 || year < 2000) {
		uart_printstrln("Invalid date format");
		return -0;
	}
	from = parse_int8_to_char(buffer, from, ':', &hours);
	if (from == -1 || hours > 23 || hours < 0) {
		uart_printstrln("Invalid time format");
		return -0;
	}
	from = parse_int8_to_char(buffer, from, ':', &minutes);
	if (from == -1 || minutes > 59 || minutes < 0) {
		uart_printstrln("Invalid time format");
		return -0;
	}
	from = parse_int8_to_char(buffer, from, '\0', &secondes);
	if (from == -1 || secondes > 59 || secondes < 0) {
		uart_printstrln("Invalid time format");
		return -0;
	}
	return 0;
}

void uart_callback(char buffer[UART_RX_BUFFER_SIZE + 1]) {
	if (parse(buffer) == 0) {
		change_date = 1;
	}
	uart_printstr("> ");
}


void rtc_actions() {
	if (make_measure && mode >= 8 && mode <= 10) {
		make_measure = 0;
		uint8_t res[7] = {0, 0, 0, 0, 0, 0, 0};
		read_rt_clock(res);
		if (mode == 8) {
			uint8_t hours = rtc_get_hours(res);
			uint8_t minutes = rtc_get_minutes(res);
			i2c_7segment_set_digit(0, hours / 10);
			i2c_7segment_set_digit(1, hours % 10);
			i2c_7segment_set_digit(2, minutes / 10);
			i2c_7segment_set_digit(3, minutes % 10);
		} else if (mode == 9) {
			uint8_t month = rtc_get_month(res);
			uint8_t day = rtc_get_day(res);
			i2c_7segment_set_digit(0, day / 10);
			i2c_7segment_set_digit(1, day % 10);
			i2c_7segment_set_digit(2, month / 10);
			i2c_7segment_set_digit(3, month % 10);
		} else if (mode == 10) {
			uint32_t year = rtc_get_year(res);
			i2c_7segment_write_numbers(year);
		}
	}
	if (change_date) {
		change_date = 0;
		set_rt_clock(secondes, minutes, hours, day, month, year);
	}
}

int main() {
	init();
	start();
	start_uart_async_readline(uart_callback, 0);
	uart_printstr("> ");
	for(int i = 0;;i++) {
		sw_actions();
		adc_actions();
		temp_actions();
		rtc_actions();
		display_mode();
		i2c_7segment_display(mode == 8 || mode == 9);
	}
}