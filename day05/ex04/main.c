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

volatile uint16_t global_id = 0;

typedef enum e_cmd {
	CMD_READ,
	CMD_WRITE,
	CMD_FORGET,
	CMD_UNKNOWN
} CMD;

void bzero(char *s, int n) {
	for(int i = 0; i < n; i++) {
		s[i] = 0;
	}
}

int strcmp(char *str1, char *str2) {
	int i = 0;
	while (str1[i] && str2[i] && str1[i] == str2[i])
		i++;
	return str1[i] - str2[i];
}

void ft_strcpy(char *dest, char *src) {
	int i = 0;
	while (src[i]) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = 0;
}

int ft_strlen(char *str) {
	int i = 0;
	while (str[i]) {
		i++;
	}
	return i;
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

int read_key(char key[256], char value[256]) {
	uart_printstr("Global id (read_key): ");
	uart_printbr_16(global_id);
	uart_nl();
	char read_key[256];
	for (uint16_t i = 1; i < global_id; i += 2) {
		bzero(read_key, 256);
		uart_printstr("Test read: ");
		uart_printbr_16(i);
		uart_nl();
		if (eepromalloc_read(i, read_key, 256)) {
			uart_printstr("Read: ");
			uart_printstr(read_key);
			uart_printstr(" at id: ");
			uart_printbr_16(i);
			uart_nl();
			uart_printstr("Compare: |");
			uart_printstr(read_key);
			uart_printstr("| with |");
			uart_printstr(key);
			uart_printstr("|");
			uart_nl();
			if (strcmp(read_key, key) == 0) {
				uart_printstr("Found: ");
				eepromalloc_read(i + 1, value, 256);
				uart_printstrnl(value);
				return i + 1;
			}
			uart_printstrnl("Not equal");
		}
	}
	uart_printstrnl("READ KEY NOT FOUND");
	return 0;
}

void read_command(char key[256]) {
	uart_printstr("Read: ");
	uart_printstr(key);
	uart_nl();
	uart_printstr("Global id (read_command): ");
	uart_printbr_16(global_id);
	uart_nl();
	char value[256];
	bzero(value, 256);
	if (read_key(key, value)) {
		uart_printstr(value);
		uart_nl();
	} else {
		uart_printstr(key);
		uart_printstrnl(" not found");
	}
}

void write_command(char key[256], char value[256]) {
	char already_value[256];
	int stored;
	if ((stored = read_key(key, already_value)) == 0) {
		if (!eepromalloc_write(++global_id, key, (int16_t)ft_strlen(key)) || !eepromalloc_write(++global_id, value, (int16_t)ft_strlen(value))) {
			uart_printstrnl("Error: eeprom full");
			return;
		}
		uart_printstr("Value stored at id: ");
		uart_printbr_16(global_id);
		uart_nl();
	} else {
		if (!eepromalloc_write(stored, value, (int16_t)ft_strlen(value))) {
			uart_printstrnl("Error: eeprom full");
			return;
		}
		uart_printstrnl("Value updated");
	}
}

void forget_command(char key[256]) {
	uart_printstr("Forget: ");
	uart_printstr(key);
	uart_nl();
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
	CMD cmd = CMD_UNKNOWN;
	char arg1[256];
	char arg2[256];
	while (get_arg(buffer, arg, i)) {
		if (arg[0] == 0) {
			uart_printstrnl("Error: empty argument");
			return;
		}
		if (i == 0) {
			if (strcmp(arg, "READ") == 0) {
				cmd = CMD_READ;
			} else if (strcmp(arg, "WRITE") == 0) {
				cmd = CMD_WRITE;
			} else if (strcmp(arg, "FORGET") == 0) {
				cmd = CMD_FORGET;
			} else {
				uart_printstrnl("Error: unknown command");
				return;
			}
		} else {
			if (i == 1) {
				ft_strcpy(arg1, arg);
			} else if (i == 2) {
				ft_strcpy(arg2, arg);
			} else {
				uart_printstrnl("Error: too many arguments");
				return;
			}
		}
		i++;
	}
	if (cmd == CMD_UNKNOWN) {
		uart_printstrnl("Error: unknown command");
		return;
	}
	if (cmd == CMD_READ && i != 2) {
		uart_printstrnl("Error in command: READ <key>");
		return;
	}
	if (cmd == CMD_WRITE && i != 3) {
		uart_printstrnl("Error in command: WRITE <key> <value>");
		return;
	}
	if (cmd == CMD_FORGET && i != 2) {
		uart_printstrnl("Error in command: FORGET <key>");
		return;
	}
	if (cmd == CMD_READ) {
		read_command(arg1);
	} else if (cmd == CMD_WRITE) {
		write_command(arg1, arg2);
	} else if (cmd == CMD_FORGET) {
		forget_command(arg1);
	}
}

int main() {
	uart_init();

	i2c_init();

	for (int i = 0; i < 100; i++) {
		eepromalloc_free(i);
	}

	// if (!eepromalloc_read(0, &global_id, 2)) {
		global_id = 0;
		// eepromalloc_write(0, (int16_t *)&global_id, 2);
	// }

	char buffer[256];

	for(;;) {
		uart_printstr("Global id (main): ");
		uart_printbr_16(global_id);
		uart_nl();
		uart_printstr("> ");
		bzero(buffer, 256);
		uart_readline(buffer, 256, 0);
		treat_command(buffer);
	}
}