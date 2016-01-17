#include <avr/io.h>

#include "it.h"
#include "eeprom.h"

uint16_t stackTail;
uint16_t dutyH[16], dutyL[16];
uint16_t status;


/*
    EEPROM Usage:
        0  - 7 : Status of Port A
		8  - 15: Status of Port C
		16 - 80:
				4 Bytes make a unit
				Bit 0 and bit 1 is the time of high voltage
				Bit 2 and bit 2 is the time of low voltage
		X - 511: Commands will be executed at boot
*/

void writeEEPROM(unsigned int addr, unsigned char data)
{
	while (EECR & (1 << EEWE)) ;
	EEAR = addr;
	EEDR = data;
	EECR |= 1 << EEMWE;
	EECR |= 1 << EEWE;
}

unsigned char readEEPROM(unsigned int addr)
{
	while (EECR & (1 << EEWE)) ;
	EEAR = addr;
	EECR |= 1 << EERE;
	return EEDR;
}

void setDuty()
{
	uint8_t i;
	for (i = 0 + 16; i < 64 + 16; i += 4) {
		dutyH[(i - 16) / 4] =
		    ((uint16_t) readEEPROM(i)) << 8 | readEEPROM(i + 1);
		dutyL[(i - 16) / 4] =
		    ((uint16_t) readEEPROM(i + 2)) << 8 | readEEPROM(i + 3);
	}

	for (i = 0; i < 16; i++) {
		if (status & (1 << i))
			m[i] = dutyH[i];
		else
			m[i] = dutyL[i];
	}
}
