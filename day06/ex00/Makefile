SRCS = main.c libs/i2c.c libs/uart.c libs/eeprom.c
BINARY_FILES=$(SRCS:.c=.out)
LINK_FILE=main.bin
HEX_FILE=main.hex

DEVICE=atmega328p
F_CPU=16000000UL
PORT=/dev/ttyUSB0

FLASH_BAUDRATE=115200
FLASH_PROGRAMMER=arduino

all: flash

%.out: %.c
	avr-gcc -Wall -I libs -Os -D F_CPU=$(F_CPU) -mmcu=$(DEVICE) -c $< -o $@

$(LINK_FILE): $(BINARY_FILES)
	avr-gcc -Wall -I libs -Os -D F_CPU=$(F_CPU) -mmcu=$(DEVICE) $(BINARY_FILES) -o $(LINK_FILE)

${HEX_FILE}: $(LINK_FILE)
	avr-objcopy -O ihex $(LINK_FILE) $(HEX_FILE)

hex: ${HEX_FILE}

flash: hex
	avrdude -p $(DEVICE) -c $(FLASH_PROGRAMMER) -P ${PORT} -b $(FLASH_BAUDRATE) -U flash:w:$(HEX_FILE)

clean:
	rm -f $(BINARY_FILE) $(HEX_FILE)

re: clean all