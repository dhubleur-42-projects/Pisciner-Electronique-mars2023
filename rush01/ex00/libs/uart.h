#ifndef UART_H
# define UART_H

# include <avr/io.h>
# include <avr/interrupt.h>

# define UART_BAUDRATE 115200

# define UART_RX_BUFFER_SIZE 100


typedef void (*readline_callback)(char *);

void uart_init();
void uart_printchar(char c);
void uart_printstr(char *str);
void uart_printbyte(uint8_t byte);
void uart_ln();
void uart_printstrln(char *str);
void uart_printnbr(int n);
void uart_printnbr_u8(uint8_t n);
void uart_printnbr_8(int8_t n);
void uart_printnbr_u16(uint16_t n);
void uart_printnbr_16(int16_t n);
void uart_printnbr_u32(uint32_t n);
void uart_printnbr_32(int32_t n);
void uart_print_byte_hexa(uint8_t n);
char uart_readchar(void);
void uart_readline(char *buffer, int size, char echo);
void bzero(char *s, int n);
void start_uart_async_readline(readline_callback callback, char echo);

#endif