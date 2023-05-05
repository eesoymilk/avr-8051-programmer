#pragma once

#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#ifndef uchar
#define uchar unsigned char
#endif

void AT8051WriteSCK(bool input);
void AT8051WriteMOSI(bool input);
uchar AT8051ReadMISO();
void AT8051WriteRST(bool input);

uchar AT8051ProgrammingEnable();
uchar AT8951ChipErase();
uchar AT8951ReadByte(unsigned int address);
void AT8951WriteByte(unsigned int address, uchar data);
void AT8951WriteOctet(unsigned int address, uchar* data);

uchar AT8051SPITransfer(uchar data);

void AT8051SPIInit();
void AT8051SPIRelease();

void ATMega328SPIInit();
void ATMega328SPIRelease();
