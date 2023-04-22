#include <avr/io.h>
#include <util/delay.h>
#include "param.h"
#include "stdio.h"
#include "program.h"

unsigned char erase_flag;
unsigned char write_flag;
unsigned char read_flag;

int main(void)
{
    _delay_us(10);	

    DDRB |= (1<<PB2) | (1<<PB3) | (1<<PB5);     
    DDRB &= ~(1<<PB4);                          

    DDRC |= (1<<PC0) | (1<<PC1); 

    PORTC &= ~(1<<PC0);
    PORTC &= ~(1<<PC1);

    PORTB |= (1<<PB2);
    PORTB |= (1<<PB3);
    PORTB &= ~(1<<PB5);

    _delay_ms(1000);
	erase_flag = AT89S51_Program_erase();

	if(erase_flag == HAL_OK)
    {
        PORTC |= (1<<PB1);
    }
	
	else if(erase_flag == HAL_Prog_En_ERROR)
    {
        while(1){
            PORTC |= (1<<PB0);
            _delay_ms(500);
            PORTC &= ~(1<<PB0);
            _delay_ms(500);
        }
    }
    
	_delay_ms(1000);
	write_flag = AT89S51_Program_write();
	if(write_flag == HAL_OK)
	{	
        PORTC |= (1<<PB1);
	}
	else if(write_flag == HAL_Prog_En_ERROR)
	{
        while(1){
            PORTC |= (1<<PB0);
            _delay_ms(500);
            PORTC &= ~(1<<PB0);
            _delay_ms(500);
        }
	}    

	_delay_ms(1000);
	read_flag=AT89S51_Program_read();
	if(read_flag==HAL_OK)
	{	
        PORTC |= (1<<PB1);
	}
	else if(read_flag==HAL_Prog_En_ERROR)
	{
        while(1){
            PORTC |= (1<<PB0);
            _delay_ms(500);
            PORTC &= ~(1<<PB0);
            _delay_ms(500);
        }
	}
	else if(read_flag==HAL_Verify_ERROR)
	{
        while(1){
            PORTC |= (1<<PB0);
            PORTC |= (1<<PB1);
            _delay_ms(500);
            PORTC &= ~(1<<PB0);
            PORTC &= ~(1<<PB1);
            _delay_ms(500);
        }
	}

    RELEASE_PORTB(); 

    return 0;
}