#include "eeprom.h"

#define EEPROM_MAGIC 0x42

bool safe_eeprom_read(void *buffer, size_t offset, size_t length) {
	if (offset < 1) {
		return false;
	}
	size_t magic = offset - 1;
	if (eeprom_read_byte((uint8_t *)magic) != EEPROM_MAGIC) {
		return false;
	}
	uint8_t *ptr = (uint8_t *)buffer;
	for (size_t i = 0; i < length; i++) {
		ptr[i] = eeprom_read_byte((uint8_t *)offset + i);
	}
	return true;
}

void write_buffer(void *buffer, size_t offset, size_t length) {
	uint8_t *ptr = (uint8_t *)buffer;
	for (size_t i = 0; i < length; i++) {
		eeprom_write_byte((uint8_t *)offset + i, ptr[i]);
	}
}

bool safe_eeprom_write(void *buffer, size_t offset, size_t length) {
	if (offset < 1) {
		return false;
	}
	size_t magic = offset - 1;
	if (eeprom_read_byte((uint8_t *)magic) != EEPROM_MAGIC) {
		eeprom_write_byte((uint8_t *)magic, EEPROM_MAGIC);
		write_buffer(buffer, offset, length);
	} else {
		uint8_t *ptr = (uint8_t *)buffer;
		for (size_t i = 0; i < length; i++) {
			if (eeprom_read_byte((uint8_t *)offset + i) != ((uint8_t *)buffer)[i]) {
				eeprom_write_byte((uint8_t *)offset + i, ptr[i]);
			}
		}
	}
	return true;
}