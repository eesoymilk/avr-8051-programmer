#include "param.h"

// PB2 <-> RST, PB3 <-> P1_5, PB4 <-> P1_6, PB5 <-> P1_7

int AT8051_MISO_VAL(void) { 
    if (PINB & (1<<PB4)) {
        return 1;
    }
    return 0;    
}

void AT8051_MOSI_WR(int input) { input == 1 ? (PORTB |= (1 << PB3)) : (PORTB &= ~(1 << PB3)); }
void AT8051_SCK_WR(int input)  { input == 1 ? (PORTB |= (1 << PB5)) : (PORTB &= ~(1 << PB5)); }
void AT8051_RST_WR(int input)  { input == 1 ? (PORTB |= (1 << PB2)) : (PORTB &= ~(1 << PB2)); }

void RELEASE_PORTB(void){
    DDRB &= ~(1<<PB0 | 1<<PB1 | 1<<PB2 | 1<<PB3 | 1<<PB4 | 1<<PB5 | 1<<PB6 | 1<<PB7);
}