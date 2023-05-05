#include "atmega328_51_serial.h"

void AT8051WriteSCK(bool input)
{
    PORTB = input ? (PORTB | (1 << PB5)) : (PORTB & ~(1 << PB5));
}

void AT8051WriteMOSI(bool input)
{
    PORTB = input ? (PORTB | (1 << PB3)) : (PORTB & ~(1 << PB3));
}

uchar AT8051ReadMISO()
{
    return PINB & (1 << PB4) ? 1 : 0;
}

void AT8051WriteRST(bool input)
{
    PORTB = input ? (PORTB | (1 << PB2)) : (PORTB & ~(1 << PB2));
}

uchar AT8051ProgrammingEnable(void)
{
    uchar output;

    AT8051SPITransfer(0xac);           // 1010_1100
    _delay_us(10);
    AT8051SPITransfer(0x53);           // 0101_0011
    _delay_us(10);
    AT8051SPITransfer(0x00);           // don't care
    _delay_us(10);
    output = AT8051SPITransfer(0x11);  // don't care
    _delay_us(10);

    return output;
}

uchar AT8951ChipErase(void)
{
    uchar output;
    AT8051SPITransfer(0xac);
    _delay_us(10);
    AT8051SPITransfer(0x80);
    _delay_us(10);
    AT8051SPITransfer(0x12);
    _delay_us(10);
    output = AT8051SPITransfer(0x13);
    _delay_us(10);

    _delay_ms(500);

    return output;
}

uchar AT8951ReadByte(unsigned int address)
{
    uchar output;
    AT8051SPITransfer(0x20);
    _delay_us(10);
    AT8051SPITransfer(address >> 8);
    _delay_us(10);
    AT8051SPITransfer(address & 0x00ff);
    _delay_us(10);
    output = AT8051SPITransfer(0x00);
    _delay_us(10);

    return output;
}

void AT8951WriteByte(unsigned int address, uchar data)
{
    uchar output;
    AT8051SPITransfer(0x40);
    _delay_us(10);
    AT8051SPITransfer(address >> 8);
    _delay_us(10);
    AT8051SPITransfer(address & 0x00ff);
    _delay_us(10);
    AT8051SPITransfer(data);
    _delay_us(10);
}

void AT8951WriteOctet(unsigned int address, uchar* data)
{
    for (int i = 0; i < 8; i++) {
        AT8951WriteByte(address + i, data[i]);
        _delay_ms(1);
    }
}

uchar AT8051SPITransfer(uchar data)
{
    uchar i = 0, output = 0;

    AT8051WriteSCK(0);
    _delay_us(10);

    for (i = 0; i < 8; i++) {
        AT8051WriteMOSI((data >> 7) & 0x01);
        data <<= 1;
        _delay_us(10);

        AT8051WriteSCK(1);
        _delay_us(10);

        output = output << 1 | AT8051ReadMISO();

        AT8051WriteSCK(0);
        _delay_us(10);
    }

    return output;
}

void AT8051SPIInit()
{
    // After Reset signal is high,
    // SCK should be low for at least 64 system clocks before
    // it goes high to clock in the enable data bytes.
    // In our case, 64 system clocks for a 12 MHz clock would take
    // approximately 5.33 us. We halt for 10 ms just to be SUPER safe.
    AT8051WriteMOSI(0);
    AT8051WriteSCK(0);
    AT8051WriteRST(1);
    _delay_ms(10);
}

void AT8051SPIRelease()
{
    AT8051WriteMOSI(1);
    AT8051WriteSCK(1);
    AT8051WriteRST(0);
}

void ATMega328SPIInit()
{
    // Set PB2 (RST), PB3 (MOSI), PB5 (SCK) as output ports
    // and PB4 (MISO) as input port
    DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5);
    DDRB &= ~(1 << PB4);
}

void ATMega328SPIRelease()
{
    // Release PB2 (RST), PB3 (MOSI), PB5 (SCK)
    DDRB &= ~((1 << PB2) | (1 << PB3) | (1 << PB5));
}
