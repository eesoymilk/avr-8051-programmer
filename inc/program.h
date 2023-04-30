#ifndef PROGRAM_H

#define PROGRAM_H

#define HAL_OK 0
#define HAL_Prog_En_ERROR 1
#define HAL_Verify_ERROR 2
#define case_hex_size 8

unsigned char AT89S51_Program_All(void);
unsigned char AT89S51_Program_write(void);
unsigned char AT89S51_Program_read(void);
unsigned char AT89S51_Program_erase(void);
void CASE_SETTING_IO(void);
void CASE_RELEASE_IO(void);
unsigned char CASE_PROG_EN(void);
void CASE_ERASE_FLASH(void);
void CASE_WRITE(unsigned char* case_hex_file, int program_cnt);
unsigned char CASE_READ(unsigned char* case_hex_file);

#endif