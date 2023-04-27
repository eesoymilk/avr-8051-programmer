#include "program.h"
#include "hex_file.h"
#include "chip.h"
#include "param.h"



//-----------------------------self use functions-----------------------------//
//----------------------------------------------------------------------------//


void CASE_SETTING_IO(void)
{
	//Step1: Setting IO for Program Mode//
	AT8051_MOSI_WR(0);			//Setting output 0
	AT8051_SCK_WR(0);				//Setting output 0
		_delay_ms(1);
	AT8051_RST_WR(1);				//Going to reset mode
		_delay_ms(10);
}

void CASE_RELEASE_IO(void)
{
	//Step4: Release IO for AT89S51 can start to work//
	AT8051_RST_WR(0);
		_delay_ms(100);	
	AT8051_MOSI_WR(1);
	AT8051_SCK_WR(1);
		_delay_ms(1000);	
}

unsigned char CASE_PROG_EN(void)
{
	//unsigned char spr_r_buf;
	//spr_r_buf = 
	AT89S51_Prog_En();
	// if(spr_r_buf != 0x69)
	// {
	// 	CASE_RELEASE_IO();
	// 	return HAL_Prog_En_ERROR;
	// }
	return HAL_OK;
}

void CASE_ERASE_FLASH(void)
{
	//though AT89S51_Chip_Erase has return value, I think we can ignore it
	AT89S51_Chip_Erase();
		_delay_ms(1000);
}

void CASE_WRITE(unsigned char* case_hex_file, int program_cnt)
{	
	//Step4: Write Chip Flash Area//
	for(int wr_tmp_cnt = 0; wr_tmp_cnt < case_hex_size; wr_tmp_cnt++)
	{
		AT89S51_Write_Byte(program_cnt + wr_tmp_cnt, case_hex_file[wr_tmp_cnt]);
		_delay_ms(1);
	}
	program_cnt += 8;
}

// unsigned char CASE_READ(unsigned char* case_hex_file)
// {
// 	unsigned char spr_r_buf;
// 	unsigned int program_cnt = 0;
// 	//Step3: Read and Verify Chip Flash Area//
// 	for(program_cnt = 0; program_cnt < case_hex_size; program_cnt ++)
// 	{
// 		spr_r_buf = AT89S51_Read_Byte(program_cnt);
// 		_delay_ms(1);	
// 		if(spr_r_buf != case_hex_file[program_cnt])
// 		{
// 			CASE_RELEASE_IO();
// 			return HAL_Verify_ERROR;
// 		}
// 	}
// 	return HAL_OK;
// }


// unsigned char AT89S51_Program_read(void)
// {
// 	unsigned char spr_r_buf;	
// 	unsigned int program_cnt = 0;	
	
// 	//Step1: Setting IO for Program Mode//
// 	AT8051_MOSI_WR(0);			//Setting output 0
// 	AT8051_SCK_WR(0);				//Setting output 0
// 		_delay_ms(1);
// 	AT8051_RST_WR(1);				//Going to reset mode
// 		_delay_ms(10);
	
// 	//Step2: Programming Enable//
// 	spr_r_buf = AT89S51_Prog_En();
// 	if(spr_r_buf != 0x69)
// 	{
// 		AT8051_RST_WR(0);
// 			_delay_ms(100);	
// 		AT8051_MOSI_WR(1);
// 		AT8051_SCK_WR(1);
// 			_delay_ms(1000);	
// 		return HAL_Prog_En_ERROR;
// 	}
	
// 	//Step3: Read and Verify Chip Flash Area//
// 	for(program_cnt = 0; program_cnt < hex_size; program_cnt ++)
// 	{
// 		spr_r_buf = AT89S51_Read_Byte(program_cnt);
// 		_delay_ms(1);	
// 		if(spr_r_buf != hex_file[program_cnt])
// 		{
// 			AT8051_RST_WR(0);
// 				_delay_ms(100);	
// 			AT8051_MOSI_WR(1);
// 			AT8051_SCK_WR(1);
// 				_delay_ms(1000);	
		
// 			return HAL_Verify_ERROR;
// 		}
// 	}
	
	
// 	//Step4: Release IO for AT89S51 can start to work//
// 	AT8051_RST_WR(0);
// 		_delay_ms(100);	
// 	AT8051_MOSI_WR(1);
// 	AT8051_SCK_WR(1);
// 		_delay_ms(1000);	
// 	return HAL_OK;
// }

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

// unsigned char AT89S51_Program_write(void)
// {
// 	unsigned char spr_r_buf;	
// 	unsigned int program_cnt = 0;	
	
// 	//Step1: Setting IO for Program Mode//
// 	AT8051_MOSI_WR(0);			//Setting output 0
// 	AT8051_SCK_WR(0);				//Setting output 0
// 		_delay_ms(1);
// 	AT8051_RST_WR(1);				//Going to reset mode
// 		_delay_ms(10);
	
// 	//Step2: Programming Enable//
// 	spr_r_buf = AT89S51_Prog_En();
// 	if(spr_r_buf != 0x69)
// 	{
// 		AT8051_RST_WR(0);
// 			_delay_ms(100);	
// 		AT8051_MOSI_WR(1);
// 		AT8051_SCK_WR(1);
// 			_delay_ms(1000);	
// 		return HAL_Prog_En_ERROR;
// 	}
	
// 	//Step3: Erase Chip Flash Area//
// 	AT89S51_Chip_Erase();
// 		_delay_ms(1000);
	
// 	//Step4: Write Chip Flash Area//
// 	for(program_cnt = 0; program_cnt < hex_size; program_cnt ++)
// 	{
// 		AT89S51_Write_Byte(program_cnt, hex_file[program_cnt]);
// 		_delay_ms(1);
// 	}
	
// 	//Step6: Release IO for AT89S51 can start to work//
// 	AT8051_RST_WR(0);
// 		_delay_ms(100);	
// 	AT8051_MOSI_WR(1);
// 	AT8051_SCK_WR(1);
// 		_delay_ms(1000);	
// 	return HAL_OK;
// }

