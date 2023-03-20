#include "eeprom.h"

#define EEPROM_MAGIC_START 0x21
#define EEPROM_MAGIC_END 0x24
#define EEPROM_SIZE 1024

typedef struct s_block_info {
	uint16_t id;
	uint16_t length;
	size_t offset;
} block_info;

void read_block(size_t *offset, block_info *info) {
	info->id = eeprom_read_word((uint16_t *)*offset);
	*offset += 2;
	info->length = eeprom_read_word((uint16_t *)*offset);
	*offset += 2;
	info->offset = *offset;
	*offset += info->length;
}

bool find_info(uint16_t id, block_info *info) {
	uint8_t first = eeprom_read_byte((uint8_t *)0);
	if (first != EEPROM_MAGIC_START && first != EEPROM_MAGIC_END) {
		eeprom_write_byte((uint8_t *)0, EEPROM_MAGIC_END);
		return false;
	}
	size_t offset = 0;
	while (offset < EEPROM_SIZE) {
		uint8_t magic = eeprom_read_byte((uint8_t *)offset);
		if (magic == EEPROM_MAGIC_START) {
			offset++;
			read_block(&offset, info);
			if (info->id == id) {
				return true;
			}
		} else if (magic == EEPROM_MAGIC_END)
			return false;
		else
			offset++;
	}
	return false;
}

bool is_available_space(size_t start_offset, uint16_t length) {
	for (size_t i = 0; i < length; i++) {
		if (start_offset + i >= EEPROM_SIZE) {
			return false;
		}
		if (eeprom_read_byte((uint8_t *)start_offset + i) == EEPROM_MAGIC_START) {
			return false;
		}
	}
	return true;
}

bool eepromalloc_free(uint16_t id) {
	block_info info;
	if (find_info(id, &info)) {
		eeprom_write_word((uint16_t *)info.offset - 4, 0x0);
		return true;
	}
	return false;
}

size_t find_end() {
	size_t offset = 0;
	while (offset < EEPROM_SIZE) {
		uint8_t magic = eeprom_read_byte((uint8_t *)offset);
		if (magic == EEPROM_MAGIC_START) {
			offset++;
			block_info info;
			read_block(&offset, &info);
		} else if (magic == EEPROM_MAGIC_END) {
			return offset;
		} else {
			offset++;
		}
	}
	return 0;
}

bool eepromalloc_write(uint16_t id, void *buffer, uint16_t length) {
	block_info info;
	if (find_info(id, &info)) {
		if (info.length == length) {
			size_t offset = info.offset;
			uint8_t *ptr = (uint8_t *)buffer;
			for (uint16_t i = 0; i < length; i++) {
				if (eeprom_read_byte((uint8_t *)offset + i) != ptr[i]) {
					eeprom_write_byte((uint8_t *)offset + i, ptr[i]);
				}
			}
			return true;
		} else {
			if (length < info.length || is_available_space(info.offset, length)) {
				eeprom_write_word((uint16_t *)info.offset - 2, length);
				size_t offset = info.offset;
				uint8_t *ptr = (uint8_t *)buffer;
				for (uint16_t i = 0; i < length; i++) {
					if (eeprom_read_byte((uint8_t *)offset + i) != ptr[i]) {
						eeprom_write_byte((uint8_t *)offset + i, ptr[i]);
					}
				}
				return true;
			} else {
				eepromalloc_free(id);
			}
		}
	}
	size_t offset = find_end();
	if (offset + length + 6 < EEPROM_SIZE) {
		eeprom_write_byte((uint8_t *)offset, EEPROM_MAGIC_START);
		offset++;
		eeprom_write_word((uint16_t *)offset, id);
		offset += 2;
		eeprom_write_word((uint16_t *)offset, length);
		offset += 2;
		uint8_t *ptr = (uint8_t *)buffer;
		for (uint16_t i = 0; i < length; i++) {
			eeprom_write_byte((uint8_t *)offset + i, ptr[i]);
		}
		offset += length;
		eeprom_write_byte((uint8_t *)offset, EEPROM_MAGIC_END);
		return true;
	} else {
		return false;
	}
}

bool eepromalloc_read(uint16_t id, void *buffer, uint16_t length) {
	block_info info;
	if (find_info(id, &info)) {
		if (length <= info.length) {
			size_t offset = info.offset;
			uint8_t *ptr = (uint8_t *)buffer;
			for (uint16_t i = 0; i < length; i++) {
				ptr[i] = eeprom_read_byte((uint8_t *)offset + i);
			}
			return true;
		}
	}
	return false;
}