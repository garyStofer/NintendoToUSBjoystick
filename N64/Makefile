

#####   USE managed build in AVRSTUDIO5 instead 

## FUSE setting for ATMEGA88P  0xF9,0xDD,0xDF


# Name: Makefile
# Project: HIDKeys
# Author: Christian Starkjohann
# Creation Date: 2006-02-02
# Tabsize: 4
# Copyright: (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile,v 1.2 2007/10/04 00:44:14 raph Exp $

UISP = uisp -dprog=stk500 -dpart=atmega8 -dserial=/dev/avr
COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega88 -DF_CPU=12000000L #-DDEBUG_LEVEL=1
COMMON_OBJS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o n64.o reportdesc.o

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o gamecube.o devdesc.o n64.o reportdesc.o


# symbolic targets:
all:	gc_n64_usb.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@


clean:
	rm -f gc_n64_usb.hex gc_n64_usb.lst gc_n64_usb.obj gc_n64_usb.cof gc_n64_usb.list gc_n64_usb.map gc_n64_usb.eep.hex gc_n64_usb.bin *.o usbdrv/*.o gc_n64_usb.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
gc_n64_usb.bin:	$(COMMON_OBJS)	gamecube.o devdesc.o
	$(COMPILE) -o gc_n64_usb.bin $(OBJECTS) -Wl,-Map=gc_n64_usb.map

gc_n64_usb.hex:	gc_n64_usb.bin
	rm -f gc_n64_usb.hex gc_n64_usb.eep.hex
	avr-objcopy -j .text -j .data -O ihex gc_n64_usb.bin gc_n64_usb.hex
	./checksize gc_n64_usb.bin

flash:	all
	#$(UISP) --erase --upload --verify if=gc_n64_usb.hex
	$(UISP) --erase --upload if=gc_n64_usb.hex

flash_usb: all
	sudo avrdude -p m8 -P usb -c avrispmkII -Uflash:w:gc_n64_usb.hex -B 1.0

# Fuse high byte:
# 0xc9 = 1 1 0 0   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse low byte:
# 0x9f = 1 0 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)
fuse:
	$(UISP) --wr_fuse_h=0xc9 --wr_fuse_l=0x9f



