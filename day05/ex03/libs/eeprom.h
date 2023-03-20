#ifndef EEPROM_H
# define EEPROM_H

# include <avr/eeprom.h>
# include <stdbool.h>

bool eepromalloc_free(uint16_t id);
bool eepromalloc_write(uint16_t id, void *buffer, uint16_t length);
bool eepromalloc_read(uint16_t id, void *buffer, uint16_t length);

#endif