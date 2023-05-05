# Name: Makefile
# Project: custom-class example
# Author: Christian Starkjohann
# Creation Date: 2008-04-07
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

ifeq ($(OS),Windows_NT)
    # Windows
    RM := del /Q
    FIXPATH = $(subst /,\,$1)
else
    # Unix-based
    RM := rm -f
    FIXPATH = $1
endif

DEVICE  = atmega328p
F_CPU   = 12000000
FUSE_L  = 0xdf
FUSE_H  = 0xd9
CONF = C:/AVR/avrdude/avrdude.conf
AVRDUDE = avrdude -C$(CONF) -v -V -p$(DEVICE) -cstk500v1 -PCOM3 -b19200

# CFLAGS  = -Iusbdrv -I. -Iinc
CFLAGS = -Iusbdrv -I. -Iinc -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(DEVICE)
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o main.o src/atmega328_51_serial.o

# COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

####################### Fuse values for  ATMega328 ###########################
# ATMega328 FUSE_L (Fuse low byte):
# 0xdf = 1 1 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ CKOUT (if 0: Clock output enabled)
#        +-------------------- CKDIV8 (if 0: divide by 8)
# ATMega328 FUSE_H (Fuse high byte):
# 0xd9 = 1 1 0 1   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (if 0: watchdog always on)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ DWEN (debug wire enable)
#        +-------------------- RSTDISBL (reset pin is enabled)
##############################################################################

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"

re: clean flash

hex: main.hex

program: flash fuse

fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

clean:
	$(RM) main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf $(wildcard *.o) .\usbdrv\usbdrv.o .\usbdrv\usbdrvasm.o .\usbdrv\oddebug.o src\chip.o src\param.o src\program.o

# Generic rule for compiling C files:
.c.o:
	avr-gcc $(CFLAGS) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	avr-gcc $(CFLAGS) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	avr-gcc $(CFLAGS) -S $< -o $@

main.elf: $(OBJECTS)	# usbdrv dependency only needed because we copy it
	avr-gcc $(CFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	$(RM) main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex

# debugging targets:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
