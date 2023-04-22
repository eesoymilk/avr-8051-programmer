#include "program.h"
#include "hex_file.h"
#include "chip.h"
#include "param.h"


unsigned char AT89S51_Program_read(void)
{
	unsigned char spr_r_buf;	
	unsigned int program_cnt = 0;	
	
	//Step1: Setting IO for Program Mode//
	AT8051_MOSI_WR(0);			//Setting output 0
	AT8051_SCK_WR(0);				//Setting output 0
		_delay_ms(1);
	AT8051_RST_WR(1);				//Going to reset mode
		_delay_ms(10);
	
	//Step2: Programming Enable//
	spr_r_buf = AT89S51_Prog_En();
	if(spr_r_buf != 0x69)
	{
		AT8051_RST_WR(0);
			_delay_ms(100);	
		AT8051_MOSI_WR(1);
		AT8051_SCK_WR(1);
			_delay_ms(1000);	
		return HAL_Prog_En_ERROR;
	}
	
	//Step3: Read and Verify Chip Flash Area//
	for(program_cnt = 0; program_cnt < hex_size; program_cnt ++)
	{
		spr_r_buf = AT89S51_Read_Byte(program_cnt);
		_delay_ms(1);	
		if(spr_r_buf != hex_file[program_cnt])
		{
			AT8051_RST_WR(0);
				_delay_ms(100);	
			AT8051_MOSI_WR(1);
			AT8051_SCK_WR(1);
				_delay_ms(1000);	
		
			return HAL_Verify_ERROR;
		}
	}
	
	
	//Step4: Release IO for AT89S51 can start to work//
	AT8051_RST_WR(0);
		_delay_ms(100);	
	AT8051_MOSI_WR(1);
	AT8051_SCK_WR(1);
		_delay_ms(1000);	
	return HAL_OK;
}

unsigned char AT89S51_Program_erase(void)
{
	unsigned char spr_r_buf;	
	//unsigned int program_cnt = 0;	
	
	//Step1: Setting IO for Program Mode//
	AT8051_MOSI_WR(0);			//Setting output 0
	AT8051_SCK_WR(0);				//Setting output 0
		_delay_ms(1);
	AT8051_RST_WR(1);				//Going to reset mode
		_delay_ms(10);
	
	//Step2: Programming Enable//
	spr_r_buf = AT89S51_Prog_En();
	if(spr_r_buf != 0x69)
	{
		AT8051_RST_WR(0);
			_delay_ms(100);	
		AT8051_MOSI_WR(1);
		AT8051_SCK_WR(1);
			_delay_ms(1000);	
		return HAL_Prog_En_ERROR;
	}
	//Step3: Erase Chip Flash Area//
	AT89S51_Chip_Erase();
		_delay_ms(1000);
	
	//Step4: Release IO for AT89S51 can start to work//
	AT8051_RST_WR(0);
		_delay_ms(100);	
	AT8051_MOSI_WR(1);
	AT8051_SCK_WR(1);
		_delay_ms(1000);	
	return HAL_OK;
}

unsigned char AT89S51_Program_write(void)
{
	unsigned char spr_r_buf;	
	unsigned int program_cnt = 0;	
	
	//Step1: Setting IO for Program Mode//
	AT8051_MOSI_WR(0);			//Setting output 0
	AT8051_SCK_WR(0);				//Setting output 0
		_delay_ms(1);
	AT8051_RST_WR(1);				//Going to reset mode
		_delay_ms(10);
	
	//Step2: Programming Enable//
	spr_r_buf = AT89S51_Prog_En();
	if(spr_r_buf != 0x69)
	{
		AT8051_RST_WR(0);
			_delay_ms(100);	
		AT8051_MOSI_WR(1);
		AT8051_SCK_WR(1);
			_delay_ms(1000);	
		return HAL_Prog_En_ERROR;
	}
	
	//Step3: Erase Chip Flash Area//
	AT89S51_Chip_Erase();
		_delay_ms(1000);
	
	//Step4: Write Chip Flash Area//
	for(program_cnt = 0; program_cnt < hex_size; program_cnt ++)
	{
		AT89S51_Write_Byte(program_cnt, hex_file[program_cnt]);
		_delay_ms(1);
		
	}
	
	//Step6: Release IO for AT89S51 can start to work//
	AT8051_RST_WR(0);
		_delay_ms(100);	
	AT8051_MOSI_WR(1);
	AT8051_SCK_WR(1);
		_delay_ms(1000);	
	return HAL_OK;
}