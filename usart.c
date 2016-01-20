#include <avr/io.h>
#include <avr/wdt.h>

#include "usart.h"

void sendUSART(unsigned char data)
{
	while (!(UCSRA & (1 << UDRE)))
		wdt_reset();
	UDR = data;
}

unsigned char receiveUSART()
{
	while (!(UCSRA & (1 << RXC)))
		wdt_reset();
	return UDR;
}

void print(char *s)
{
	while (*s) {
		sendUSART(*s);
		s++;
	}
}
