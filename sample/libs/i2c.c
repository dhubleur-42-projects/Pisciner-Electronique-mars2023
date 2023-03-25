#include "i2c.h"

void print_i2c_status() {
	int status = TWSR;
	status &= 0xF8;

	switch (status) {
		case 0x08:
			uart_printstr("Start condition has been transmitted\r\n");
			break;
		case 0x10:
			uart_printstr("Repeated start condition has been transmitted\r\n");
			break;
		case 0x18:
			uart_printstr("SLA+W has been transmitted, ACK has been received\r\n");
			break;
		case 0x20:
			uart_printstr("SLA+W has been transmitted, NACK has been received\r\n");
			break;
		case 0x28:
			uart_printstr("Data has been transmitted, ACK has been received\r\n");
			break;
		case 0x30:
			uart_printstr("Data has been transmitted, NACK has been received\r\n");
			break;
		case 0x38:
			uart_printstr("Arbitration lost in SLA+W or data bytes\r\n");
			break;
		case 0x40:
			uart_printstr("SLA+R has been transmitted, ACK has been received\r\n");
			break;
		case 0x48:
			uart_printstr("SLA+R has been transmitted, NACK has been received\r\n");
			break;
		case 0x50:
			uart_printstr("Data has been received, ACK has been returned\r\n");
			break;
		case 0x58:
			uart_printstr("Data has been received, NACK has been returned\r\n");
			break;
		case 0x00:
			uart_printstr("Bus error\r\n");
			break;
		default:
			uart_printstr("Unknown status\r\n");
			break;
	}
}

void wait_i2c() {
	while (!(TWCR & (1<<TWINT)));
}

void i2c_init(void) {
	TWBR = TWI_BITRATE;
	TWSR = 0;
}

void i2c_start(uint8_t address, I2C_MODE mode) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c();

	TWDR = (address << 1) | mode;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

void i2c_start_big_address(uint8_t address, I2C_MODE mode) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c();

	TWDR = address| mode;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void i2c_write(uint8_t data) {
	wait_i2c();
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

uint8_t i2c_read(ACK_TYPE ack) {
	TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN);
	wait_i2c();
	return TWDR;
}

void i2c_send_command(uint8_t address, uint8_t command, uint8_t *params, int params_length) {
	i2c_start(address, I2C_WRITE);
	i2c_write(command);
	for (int i = 0; i < params_length; i++) {
		i2c_write(params[i]);
	}
	i2c_stop();
}

int8_t i2c_expander_read_port(uint8_t address, I2C_EXPANDER_PORT port) {
	i2c_start(address, I2C_WRITE);
	i2c_write(port);
	i2c_start(address, I2C_READ);
	int8_t data = i2c_read(NACK);
	i2c_stop();
	return data;
}

int i2c_expander_get_pin(uint8_t address, I2C_EXPANDER_PORT port, uint8_t pin) {
	int8_t data = i2c_expander_read_port(address, port);
	if (data == -1) {
		return -1;
	}
	return (data >> pin) & 1;
}

volatile uint8_t output_port1_value = 0b11111111;
volatile uint8_t output_port0_value = 0b11111111;

void i2c_expander_write_port(uint8_t address, I2C_EXPANDER_PORT port, uint8_t data) {
	i2c_start(address, I2C_WRITE);
	i2c_write(port);
	i2c_write(data);
	i2c_stop();
	if (port == OUTPUT_PORT_1) {
		output_port1_value = data;
	} else if (port == OUTPUT_PORT_0) {
		output_port0_value = data;
	}
}

void i2c_expander_set_pin(uint8_t address, I2C_EXPANDER_PORT port, uint8_t pin, uint8_t value) {
	if (port == OUTPUT_PORT_1) {
		if (value == 1) {
			output_port1_value |= (1<<pin);
		} else {
			output_port1_value &= ~(1<<pin);
		}
		i2c_expander_write_port(address, port, output_port1_value);
	} else if (port == OUTPUT_PORT_0) {
		if (value == 1) {
			output_port0_value |= (1<<pin);
		} else {
			output_port0_value &= ~(1<<pin);
		}
		i2c_expander_write_port(address, port, output_port0_value);
	}
}

volatile int i2c_7segment[4] = {-1, -1, -1, -1};

void i2c_7segment_display(int middle_points) {
	i2c_7segment_write_number(i2c_7segment[3], !!middle_points);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 7, 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 7, 1);
	i2c_7segment_write_number(i2c_7segment[2], 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 6, 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 6, 1);
	i2c_7segment_write_number(i2c_7segment[1], !!middle_points);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 5, 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 5, 1);
	i2c_7segment_write_number(i2c_7segment[0], 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 4, 0);
	i2c_expander_set_pin(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 4, 1);
}

void i2c_expander_init() {
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_0, 0b00000001);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, CONFIGURATION_PORT_1, 0b00000000);
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_0, 0b11111111);
}

void i2c_7segment_write_number(int n, int dot) {
	uint8_t data = 0b00000000;
	if (dot) {
		data |= 0b10000000;
	}
	if (n == 0) {
		data |= 0b00111111;
	} else if (n == 1) {
		data |= 0b00000110;
	} else if (n == 2) {
		data |= 0b01011011;
	} else if (n == 3) {
		data |= 0b01001111;
	} else if (n == 4) {
		data |= 0b01100110;
	} else if (n == 5) {
		data |= 0b01101101;
	} else if (n == 6) {
		data |= 0b01111101;
	} else if (n == 7) {
		data |= 0b00000111;
	} else if (n == 8) {
		data |= 0b01111111;
	} else if (n == 9) {
		data |= 0b01101111;
	} else if (n == 10) {
		data |= 0b01000000;
	}
	i2c_expander_write_port(I2C_EXPANDER_ADDR, OUTPUT_PORT_1, data);
}

void i2c_7segment_write_numbers(uint32_t n) {
	if (n < 0) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = 0;
		i2c_7segment[3] = 0;
	} else if (n < 10) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = 0;
		i2c_7segment[3] = n;
	} else if (n < 100) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = 0;
		i2c_7segment[2] = n / 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 1000) {
		i2c_7segment[0] = 0;
		i2c_7segment[1] = n / 100;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 10000) {
		i2c_7segment[0] = n / 1000;
		i2c_7segment[1] = (n / 100) % 10;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	}
}

void i2c_7segment_write_numbers_signed(int32_t n) {
	if (n < -999) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = -1;
		i2c_7segment[3] = -1;
	} else if (n < -99) {
		i2c_7segment[0] = 10;
		i2c_7segment[1] = -n / 100;
		i2c_7segment[2] = (-n / 10) % 10;
		i2c_7segment[3] = -n % 10;
	} else if (n < -9) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = 10;
		i2c_7segment[2] = -n / 10;
		i2c_7segment[3] = -n % 10;
	} else if (n < 0) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = 10;
		i2c_7segment[3] = -n;
	} else if (n < 10) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = -1;
		i2c_7segment[3] = n;
	} else if (n < 100) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = -1;
		i2c_7segment[2] = n / 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 1000) {
		i2c_7segment[0] = -1;
		i2c_7segment[1] = n / 100;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	} else if (n < 10000) {
		i2c_7segment[0] = n / 1000;
		i2c_7segment[1] = (n / 100) % 10;
		i2c_7segment[2] = (n / 10) % 10;
		i2c_7segment[3] = n % 10;
	}
}

void i2c_7segment_set_digit(int digit, int n) {
	i2c_7segment[digit] = n;
}

void i2c_7segment_clear() {
	i2c_7segment[0] = -1;
	i2c_7segment[1] = -1;
	i2c_7segment[2] = -1;
	i2c_7segment[3] = -1;
}

void read_temp_sensor(int32_t res[2]) {
	i2c_start(AHTR20_ADDR, I2C_WRITE);

	i2c_write(0xac);
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();

	_delay_ms(80);

	i2c_start(AHTR20_ADDR, I2C_READ);

	uint8_t buffer[7];

	uint8_t read_byte = i2c_read(1);
	while (read_byte & 0b10000000) {
		_delay_ms(80);
		read_byte = i2c_read(1);
	}
	buffer[0] = read_byte;
	
	for (int i = 0; i < 6; i++) {
		buffer[i + 1] = i2c_read(i != 5);
	}
	i2c_stop();
	int32_t humidity = ((int32_t)buffer[1] << 12) | ((int32_t)buffer[2] << 4) | ((int32_t)buffer[3] & 0b11110000 >> 4);
	int32_t temperature = (((int32_t)buffer[3] & 0b00001111) << 16) | ((int32_t)buffer[4] << 8) | (int32_t)buffer[5];

	temperature = (temperature * 25 >> 17) - 50;
	humidity = (humidity * 250) >> 18;
	
	res[0] = temperature;
	res[1] = humidity;
}

void read_rt_clock(uint8_t res[7]) {
	i2c_start_big_address(RTC_ADDR, I2C_WRITE);
	i2c_write(0x02);
	i2c_start_big_address(RTC_ADDR, I2C_READ);
	for (int i = 0; i < 7; i++) {
		res[i] = i2c_read(i != 6);
	}
	i2c_stop();
}

uint8_t dec_to_bcd(uint8_t value) {
	uint8_t ten = value / 10;
	uint8_t unit = value % 10;
	return (ten << 4) | unit;
}

uint8_t rtc_get_minutes(uint8_t reg[7]) {
	uint8_t ten = (reg[1] & 0b01110000) >> 4;
	uint8_t unit = reg[1] & 0b00001111;
	return ten * 10 + unit;
}

uint8_t rtc_get_hours(uint8_t reg[7]) {
	uint8_t ten = (reg[2] & 0b00110000) >> 4;
	uint8_t unit = reg[2] & 0b00001111;
	return ten * 10 + unit;
}

uint8_t rtc_get_day(uint8_t reg[7]) {
	uint8_t ten = (reg[3] & 0b00110000) >> 4;
	uint8_t unit = reg[3] & 0b00001111;
	return ten * 10 + unit;
}

uint8_t rtc_get_month(uint8_t reg[7]) {
	return reg[5] & 0b00011111;
}

uint32_t rtc_get_year(uint8_t reg[7]) {
	uint8_t ten = (reg[6] & 0b11110000) >> 4;
	uint8_t unit = reg[6] & 0b00001111;
	return 2000 + ten * 10 + unit;
}

void set_rt_clock(uint8_t secondes, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t month, uint32_t year) {
	i2c_start_big_address(RTC_ADDR, I2C_WRITE);
	i2c_write(0x02);
	i2c_write(dec_to_bcd(secondes));
	i2c_write(dec_to_bcd(minutes));
	i2c_write(dec_to_bcd(hours));
	i2c_write(dec_to_bcd(day));
	i2c_write(dec_to_bcd(0));
	i2c_write(dec_to_bcd(month));
	i2c_write(dec_to_bcd(year % 100));
	i2c_stop();
}