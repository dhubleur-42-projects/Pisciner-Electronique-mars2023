#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "i2c.h"
#include "eeprom.h"

#ifndef F_CPU
# define F_CPU 16000000UL
#endif

void bzero(char *s, int n) {
	for(int i = 0; i < n; i++) {
		s[i] = 0;
	}
}

bool get_arg(char buffer[256], char arg[256], int arg_to_found) {
	int i = 0;
	while(buffer[i] == ' ' && buffer[i] != 0) {
		i++;
	}
	if (buffer[i] == 0) {
		return false;
	}
	for (int j = 0; j < arg_to_found; j++) {
		bool in_quote = false;
		if (buffer[i] == '"') {
			in_quote = true;
			i++;
		}
		while(((!in_quote && buffer[i] != ' ') || (in_quote && buffer[i] != '"')) && buffer[i] != 0) {
			i++;
		}
		if (buffer[i] == 0) {
			return false;
		}
		if (in_quote) {
			i++;
		}
		while(buffer[i] == ' ' && buffer[i] != 0) {
			i++;
		}
		if (buffer[i] == 0) {
			return false;
		}
	}
	int j = 0;
	bool in_quote = false;
	if (buffer[i] == '"') {
		in_quote = true;
		i++;
	}
	while(((!in_quote && buffer[i] != ' ') || (in_quote && buffer[i] != '"')) && buffer[i] != 0) {
		arg[j] = buffer[i];
		i++;
		j++;
	}
	if (in_quote) {
		i++;
	}
	arg[j] = 0;
	return true;
}

void treat_command(char buffer[256]) {
	bool quote = false;
	for (int i = 0; buffer[i] != 0; i++) {
		if (buffer[i] == '"') {
			quote = !quote;
		}
	}
	if (quote) {
		uart_printstrnl("Error: missing quote");
		return;
	}
	char arg[256];
	int i = 0;
	while (get_arg(buffer, arg, i)) {
		uart_printstrnl(arg);
		i++;
	}
}

int main() {
	uart_init();

	i2c_init();

	sei();

	char buffer[256];

	for(;;) {
		uart_printstr("> ");
		bzero(buffer, 256);
		uart_readline(buffer, 256, 0);
		treat_command(buffer);
	}
}