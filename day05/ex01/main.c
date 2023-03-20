#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

#define UART_BAUDRATE 115200

//Intitialize the UART
void uart_init() {
	//Set baud rate
	UBRR0L = (( F_CPU / 8 / UART_BAUDRATE ) - 1) / 2;

	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);

	//Set frame format: 8data, 1stop bit
	UCSR0C = (0<<USBS0) | (3<<UCSZ00);
}

//Write a character to PC serial port
void uart_tx(char c) {
	//Wait for empty transmit buffer
	while (!( UCSR0A & (1<<UDRE0)));

	//Put data into buffer, sends the data
	UDR0 = c;
}

void uart_printstr(char *str) {
	while (*str) {
		uart_tx(*str);
		str++;
	}
}

void uart_printbyte(uint8_t byte)
{
	for(uint8_t i = 8; i > 0; i--)
		uart_tx(((byte >> (i - 1)) & 1) + '0');
}

#define SCL_FREQUENCY 100000
#define TWI_BITRATE (((F_CPU / SCL_FREQUENCY) - 16) / 2)
#define I2C_EXPANDER_ADDR 0x20

typedef enum e_I2C_MODE {
	I2C_WRITE = 0,
	I2C_READ = 1
} I2C_MODE;

typedef enum e_ack {
	ACK = 1,
	NACK = 0
} ack;

typedef enum e_i2c_expander_port {
	INPUT_PORT_0 = 0,
	INPUT_PORT_1 = 1,
	OUTPUT_PORT_0 = 2,
	OUTPUT_PORT_1 = 3,
	POLARITY_INVERSION_PORT_0 = 4,
	POLARITY_INVERSION_PORT_1 = 5,
	CONFIGURATION_PORT_0 = 6,
	CONFIGURATION_PORT_1 = 7
} i2c_expander_port;

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

void i2c_start(int address, I2C_MODE mode) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	wait_i2c();

	TWDR = (address << 1) | mode;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void i2c_write(unsigned char data) {
	wait_i2c();
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	wait_i2c();
}

uint8_t i2c_read(ack ack) {
	TWCR = (1<<TWINT) | (ack<<TWEA) | (1<<TWEN);
	wait_i2c();
	return TWDR;
}

int8_t i2c_expander_read_port(i2c_expander_port port) {
	i2c_start(I2C_EXPANDER_ADDR, I2C_WRITE);
	i2c_write(port);
	i2c_start(I2C_EXPANDER_ADDR, I2C_READ);
	int8_t data = i2c_read(NACK);
	i2c_stop();
	return data;
}

#define MAGIC_NUMBER 0x21
#define MAGIC_POS 5

int8_t values[4] = {0, 0, 0, 0};
int8_t selected = 0;

void display_value(int idx) {
	PORTB = (((values[idx] & 1) ? 1 : 0) << PB0) | (((values[idx] & 2) ? 1 : 0) << PB1) | (((values[idx] & 4) ? 1 : 0) << PB2) | (((values[idx] & 8) ? 1 : 0) << PB4);
}

void save_value(int idx) {
	eeprom_write_byte((uint8_t *)idx, values[idx]);
}

# define LED_R PD5
# define LED_G PD6
# define LED_B PD3

void display_selected() {
	if (selected == 0) {
		PORTD = (1 << LED_R);
	} else if (selected == 1) {
		PORTD = (1 << LED_G);
	} else if (selected == 2) {
		PORTD = (1 << LED_B);
	} else if (selected == 3) {
		PORTD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);
	}
}

void load_values() {
	uint8_t magic = eeprom_read_byte((uint8_t *)MAGIC_POS);
	if (magic == MAGIC_NUMBER) {
		values[0] = eeprom_read_byte((uint8_t *)0);
		values[1] = eeprom_read_byte((uint8_t *)1);
		values[2] = eeprom_read_byte((uint8_t *)2);
		values[3] = eeprom_read_byte((uint8_t *)3);
		selected = eeprom_read_byte((uint8_t *)4);
	} else {
		eeprom_write_byte((uint8_t *)MAGIC_POS, MAGIC_NUMBER);
		save_value(0);
		save_value(1);
		save_value(2);
		save_value(3);
		eeprom_write_byte((uint8_t *)4, selected);
	}
	display_value(selected);
	display_selected();
}

//Detect interupt on INT0
ISR (INT0_vect)  {
	values[selected]++;
	if (values[selected] > 15) {
		values[selected] = 0;
	}
	display_value(selected);
	save_value(selected);
	_delay_ms(50);
}

int8_t get_sw3_value() {
	int8_t data = i2c_expander_read_port(INPUT_PORT_0);
	int sw3 = data & 0b00000001;
	return !sw3;
}

int main() {
	//Set led as output
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
	DDRD = (1 << LED_R) | (1 << LED_G) | (1 << LED_B);

	//Set button as input
	DDRD |= (0 << PD2) | (0 << PD4);

	//Enable interupt on INT0
	EIMSK = (1 << INT0);
	//Make press (0->1) on INT0 generate an interupt
	EICRA = (1 << ISC01) | (0 << ISC00);

	uart_init();
	load_values();

	i2c_init();

	sei();

	int previouse_sw3 = 0;

	for(;;) {
		if (get_sw3_value() && !previouse_sw3) {
			previouse_sw3 = 1;
			selected++;
			if (selected > 3) {
				selected = 0;
			}
			display_selected();
			eeprom_write_byte((uint8_t *)4, selected);
			display_value(selected);
		}
		if (!get_sw3_value()) {
			previouse_sw3 = 0;
		}
	}
}