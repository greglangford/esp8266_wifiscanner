CC = xtensa-lx106-elf-gcc
CFLAGS = -I./include -mlongcalls -DICACHE_FLASH
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

main-0x00000.bin: main
	esptool.py elf2image $^

main: main.o uart.o
main.o: main.c

uart: uart.o
uart.o: uart.c
