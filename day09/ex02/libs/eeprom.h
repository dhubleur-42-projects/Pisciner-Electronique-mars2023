#ifndef EEPROM_H
# define EEPROM_H

# include <avr/eeprom.h>
# include <stdbool.h>

bool safe_eeprom_read(void *buffer, size_t offset, size_t length);
bool safe_eeprom_write(void *buffer, size_t offset, size_t length);

#endif