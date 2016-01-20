// Running on an Atmega 16a
// 16 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "cmd.h"
#include "eeprom.h"
#include "it.h"
#include "usart.h"


char debugStr[100];


int main()
{
	uint8_t i;

	DDRA = 0xff;
	DDRC = 0xff;
	PORTA = 0x00;
	PORTC = 0x00;

	for (i = 0; i < 8; i++) {
		PORTA = PORTA >> 1;
		PORTA |= ((readEEPROM(i) - '0') & 0x01) << 7;
	}

	for (i = 8; i < 16; i++) {
		PORTC = PORTC >> 1;
		PORTC |= ((readEEPROM(i) - '0') & 0x01) << 7;
	}

	for (stackTail = EEPROM_SIZE - 1; readEEPROM(stackTail) != 0xff; stackTail--);

	status = ((uint16_t) PORTC) << 8 | (uint16_t) PORTA;

	initUSART();

	setDuty();

	initTimer0();

	initTimer2();

	sei();

	print("\nEntering the main loop\n");
	while (1);

	return 0;
}
