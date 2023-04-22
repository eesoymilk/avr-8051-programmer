#include "chip.h"
#include "param.h"

unsigned char AT89S51_Read_Byte(unsigned int address)
{
	unsigned char spi_r_buf;
	SPI_MASTER_WR(0x20);
		_delay_us(10);
	SPI_MASTER_WR(address >> 8);
		_delay_us(10);
	SPI_MASTER_WR(address & 0x00ff);
		_delay_us(10);
	spi_r_buf = SPI_MASTER_WR(0x00);
		_delay_us(10);	

	return spi_r_buf;		//return reading address by 89s51
}

unsigned char AT89S51_Prog_En(void)
{
	unsigned char spi_r_buf;
	SPI_MASTER_WR(0xac);
		_delay_us(10);	
	SPI_MASTER_WR(0x53);
		_delay_us(10);	
	SPI_MASTER_WR(0x00);
		_delay_us(10);	
	spi_r_buf = SPI_MASTER_WR(0x11);
		_delay_us(10);		

	return spi_r_buf;
}


unsigned char AT89S51_Chip_Erase(void)
{
	unsigned char spi_r_buf;
	SPI_MASTER_WR(0xac);
		_delay_us(10);	
	SPI_MASTER_WR(0x80);
		_delay_us(10);	
	SPI_MASTER_WR(0x12);
		_delay_us(10);	
	spi_r_buf = SPI_MASTER_WR(0x13);
		_delay_us(10);		

	return spi_r_buf;
}
	

unsigned char SPI_MASTER_WR(unsigned char package)
{
	unsigned char spi_cnt = 0;
	unsigned char spi_r_buf = 0;
	
	AT8051_SCK_WR(0);
		_delay_us(10);	
	
	for(spi_cnt = 0; spi_cnt < 8; spi_cnt ++)
	{
		AT8051_MOSI_WR(((package & 0x80) == 0x80) ? 1 : 0);
		package <<= 1;
			_delay_us(10);	
		
		AT8051_SCK_WR(1);
			_delay_us(10);	
		
		spi_r_buf <<= 1;
		spi_r_buf = (AT8051_MISO_VAL() == 1) ? (spi_r_buf |0x01) : spi_r_buf;
		
		AT8051_SCK_WR(0);
			_delay_us(10);	
	}
	
	return spi_r_buf;
}

unsigned int AT89S51_Write_Byte(unsigned int address, unsigned char package)
{
	unsigned int spi_r_buf1;
	unsigned char spi_r_buf2;
	SPI_MASTER_WR(0x40);
		_delay_us(10);
	SPI_MASTER_WR(address >> 8);
		_delay_us(10);
	spi_r_buf1 = SPI_MASTER_WR(address & 0x00ff);
		_delay_us(10);
	spi_r_buf2 = SPI_MASTER_WR(package);
		_delay_us(10);	
	
	
	spi_r_buf1 = (spi_r_buf1 << 8) | spi_r_buf2;

	return spi_r_buf1;		//return writing address by 89s51
}