#ifndef UART_H
# define UART_H

# include <avr/io.h>

# define UART_BAUDRATE 115200

void uart_init();
void uart_printchar(char c);
void uart_printstr(char *str);
void uart_printbyte(uint8_t byte);
void uart_nl();
void uart_printstrnl(char *str);
void uart_printbr_8(int8_t n);
void uart_printbr_16(uint16_t n);
void uart_printbr_32(int32_t n);
char uart_readchar(void);
void uart_readline(char *buffer, int size, char echo);

#endif