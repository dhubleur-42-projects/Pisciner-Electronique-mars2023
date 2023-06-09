#ifndef I2C_H
# define I2C_H

# include <avr/io.h>
# include <util/delay.h>
# include "uart.h"

# define SCL_FREQUENCY 100000
# define TWI_BITRATE (((F_CPU / SCL_FREQUENCY) - 16) / 2)
# define I2C_EXPANDER_ADDR 0x20
# define AHTR20_ADDR 0x38
# define RTC_ADDR 0b10100010

typedef enum e_I2C_MODE {
	I2C_WRITE = 0,
	I2C_READ = 1
} I2C_MODE;

typedef enum e_ACK {
	ACK = 1,
	NACK = 0
} ACK_TYPE;

typedef enum e_I2C_EXPANDER_PORT {
	INPUT_PORT_0 = 0,
	INPUT_PORT_1 = 1,
	OUTPUT_PORT_0 = 2,
	OUTPUT_PORT_1 = 3,
	POLARITY_INVERSION_PORT_0 = 4,
	POLARITY_INVERSION_PORT_1 = 5,
	CONFIGURATION_PORT_0 = 6,
	CONFIGURATION_PORT_1 = 7
} I2C_EXPANDER_PORT;

void wait_i2c();
void i2c_init(void);
void i2c_start(uint8_t address, I2C_MODE mode);
void i2c_stop(void);
void i2c_write(unsigned char data);
uint8_t i2c_read(ACK_TYPE ack);
void i2c_send_command(uint8_t address, uint8_t command, uint8_t *params, int params_length);
int8_t i2c_expander_read_port(uint8_t address, I2C_EXPANDER_PORT port);
void i2c_expander_write_port(uint8_t address, I2C_EXPANDER_PORT port, uint8_t data);
void i2c_expander_set_pin(uint8_t address, I2C_EXPANDER_PORT port, uint8_t pin, uint8_t value);
void i2c_7segment_write_number(int n, int dot);
void i2c_7segment_write_numbers(uint32_t n);
void i2c_7segment_set_digit(int digit, int n);
void i2c_expander_init();
int i2c_expander_get_pin(uint8_t address, I2C_EXPANDER_PORT port, uint8_t pin);
void i2c_7segment_display(int middle_points);
void i2c_7segment_clear();
void read_temp_sensor(int32_t res[2]);
void i2c_7segment_write_numbers_signed(int32_t n);
void read_rt_clock(uint8_t res[7]);
uint8_t rtc_get_minutes(uint8_t reg[7]);
uint8_t rtc_get_hours(uint8_t reg[7]);
uint8_t rtc_get_day(uint8_t reg[7]);
uint8_t rtc_get_month(uint8_t reg[7]);
uint32_t rtc_get_year(uint8_t reg[7]);
void set_rt_clock(uint8_t secondes, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t month, uint32_t year);

#endif