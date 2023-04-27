#ifndef PARAM_H
#include <avr/io.h>
#define PARAM_H

int AT8051_MISO_VAL(void);

void AT8051_MOSI_WR(int);
void AT8051_SCK_WR(int);
void AT8051_RST_WR(int);
void RELEASE_PORTB(void); 

#endif
