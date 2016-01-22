// Running on an Atmega 16a

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdio.h>

#include "cmd.h"
#include "eeprom.h"
#include "it.h"
#include "timer.h"
#include "usart.h"


int main()
{
	uint8_t i;

	wdt_enable(WDTO_60MS);

	for (i = 0; i < 8; i++) {
		PORTA = PORTA >> 1;
		PORTA |= ((readEEPROM(i) - '0') & 0x01) << 7;
	}

	for (i = 8; i < 16; i++) {
		PORTC = PORTC >> 1;
		PORTC |= ((readEEPROM(i) - '0') & 0x01) << 7;
	}

	DDRA = 0xff;
	DDRC = 0xff;

	for (stackTail = EEPROM_SIZE - 1; readEEPROM(stackTail) != 0xff; stackTail--);

	status = ((uint16_t) PORTC) << 8 | (uint16_t) PORTA;

	initUSART();

	setDuty();

	initTimer0();

	initTimer2();

	sei();

	printf("\nEntering the main loop\n");
	while (1) {
		wdt_reset();
	}

	return 0;
}
