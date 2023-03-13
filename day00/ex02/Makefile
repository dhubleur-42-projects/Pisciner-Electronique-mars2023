FILE_NAME=main
SRC_FILE=$(FILE_NAME).c
BINARY_FILE=$(FILE_NAME).bin
HEX_FILE=$(FILE_NAME).hex

DEVICE=atmega328p
F_CPU=1000000UL
PORT=/dev/ttyUSB0

FLASH_BAUDRATE=115200
FLASH_PROGRAMMER=arduino

all: hex flash

hex:
	avr-gcc -Wall -Os -D F_CPU=$(F_CPU) -mmcu=$(DEVICE) $(SRC_FILE) -o $(BINARY_FILE)
	avr-objcopy -O ihex $(BINARY_FILE) $(HEX_FILE)

flash:
	avrdude -p $(DEVICE) -c $(FLASH_PROGRAMMER) -P ${PORT} -b $(FLASH_BAUDRATE) -U flash:w:$(HEX_FILE)

clean:
	rm -f $(BINARY_FILE) $(HEX_FILE)