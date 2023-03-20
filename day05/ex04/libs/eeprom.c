#include "eeprom.h"
#include "uart.h"

#define EEPROM_MAGIC_START 0x21
#define EEPROM_MAGIC_END 0x24
#define EEPROM_SIZE 1024

typedef struct s_block_info {
	uint16_t id;
	uint16_t length;
	size_t offset;
} block_info;

void read_block(size_t *offset, block_info *info) {
	info->id = eeprom_read_word((uint16_t *)(*offset));
	*offset += 2;
	info->length = eeprom_read_word((uint16_t *)(*offset));
	uart_printstr("Readed length: ");
	uart_printbr_16(info->length);
	uart_printstr(" at offset: ");
	uart_printbr_16(*offset);
	uart_nl();
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
				uart_printstr("Found id: ");
				uart_printbr_16(id);
				uart_nl();
				return true;
			}
			uart_printstr("Not Found id: ");
			uart_printbr_16(id);
			uart_nl();
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
		eeprom_write_byte((uint8_t *)info.offset - 5, 0x0);
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
	uart_printstr("Writing buffer: ");
	uart_printstr(buffer);
	uart_printstr(" | id: ");
	uart_printbr_16(id);
	uart_nl();
	if (find_info(id, &info)) {
		if (info.length == length) {
			uart_printstrnl("Already stored and same in size");
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
				uart_printstrnl("Already stored and pass in size");
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
		uart_printstrnl("Not stored and pass in size");
		eeprom_write_byte((uint8_t *)offset, EEPROM_MAGIC_START);
		offset++;
		eeprom_write_word((uint16_t *)offset, id);
		offset += 2;
		eeprom_write_word((uint16_t *)offset, length);
		uart_printstr("Writed length: ");
		uart_printbr_16(eeprom_read_word((uint16_t *)offset));
		uart_printstr(" | offset: ");
		uart_printbr_8(offset);
		uart_nl();
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
		uart_printstr("Found info: ");
		uart_printbr_16(info.id);
		uart_printstr(" | Length: ");
		uart_printbr_16(info.length);
		uart_printstr(" | id: ");
		uart_printbr_16(id);
		uart_nl();
		if (length <= info.length) {
			size_t offset = info.offset;
			uint8_t *ptr = (uint8_t *)buffer;
			for (uint16_t i = 0; i < length; i++) {
				ptr[i] = eeprom_read_byte((uint8_t *)offset + i);
			}
			return true;
		} else {
			size_t offset = info.offset;
			uint8_t *ptr = (uint8_t *)buffer;
			for (uint16_t i = 0; i < info.length; i++) {
				ptr[i] = eeprom_read_byte((uint8_t *)offset + i);
				// uart_printstr("Readed char: ");
				// uart_printchar(ptr[i]);
				// uart_nl();
			}
			// uart_printstr("Result: ");
			// uart_printstr((char *)buffer);
			// uart_nl();
			return true;
		}
	}
	return false;
}