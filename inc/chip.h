#ifndef CHIP_H
#include <util/delay.h>
#define CHIP_H

unsigned char SPI_MASTER_WR(unsigned char package);
unsigned char AT89S51_Prog_En(void);
unsigned char AT89S51_Chip_Erase(void);
unsigned int AT89S51_Write_Byte(unsigned int address, unsigned char package);
unsigned char AT89S51_Read_Byte(unsigned int address);

#endif