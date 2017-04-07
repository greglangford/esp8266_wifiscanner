CC = xtensa-lx106-elf-gcc
CFLAGS = -I./include -mlongcalls -DICACHE_FLASH
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

CFILES := $(wildcard *.c)
OFILES := $(CFILES:%.c=%.o)

FIRMWARE := ./firmware
BINARY := elfimage

.PHONY: main convert clean

main: clean elfimage convert flash

test: main console

clean:
	rm -rf *.o $(FIRMWARE)/$(BINARY) $(FIRMWARE)/*.bin

elfimage: $(OFILES)
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $(FIRMWARE)/$(BINARY) $(OFILES)

$(OFILES): $(CFILES)

convert: elfimage
	esptool.py elf2image --output $(FIRMWARE)/ $(FIRMWARE)/$(BINARY)

flash:
	esptool.py write_flash 0x00000 $(FIRMWARE)/0x00000.bin 0x10000 $(FIRMWARE)/0x10000.bin

console:
	picocom /dev/ttyUSB0 -b 115200
