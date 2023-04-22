#ifndef PROGRAM_H

#define PROGRAM_H

#define HAL_OK 0
#define HAL_Prog_En_ERROR 1
#define HAL_Verify_ERROR 2

unsigned char AT89S51_Program_All(void);
unsigned char AT89S51_Program_write(void);
unsigned char AT89S51_Program_read(void);
unsigned char AT89S51_Program_erase(void);

#endif