# Name: Makefile
# Project: custom-class example
# Author: Christian Starkjohann
# Creation Date: 2008-04-07
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

DEVICE  = atmega328p
FUSE_L  = 0xd2
FUSE_H  = 0xd9
AVRDUDE = avrdude -C C:/AVR/avrdude/avrdude.conf -v -V -p $(DEVICE) -c stk500v1 -P COM3 -b 19200

COMPILE = avr-gcc -Wall -Os -mmcu=$(DEVICE)

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

hex: blink.hex

program: flash fuse

fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

flash: blink.hex
	$(AVRDUDE) -U flash:w:blink.hex:i

clean:
	del /Q /F blink.hex blink.out

# Generic rule for compiling C files:
blink.out:
	$(COMPILE) blink.c -o blink.out

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

blink.hex: blink.out
	avr-objcopy -j .text -j .data -O ihex blink.out blink.hex
	avr-size blink.hex

cpp:
	$(COMPILE) -E main.c
