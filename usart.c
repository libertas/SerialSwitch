#include <avr/io.h>

#include "usart.h"

void sendUSART(unsigned char data)
{
	while (!(UCSRA & (1 << UDRE))) ;
	UDR = data;
}

unsigned char receiveUSART()
{
	while (!(UCSRA & (1 << RXC))) ;
	return UDR;
}

void print(char *s)
{
	while (*s) {
		sendUSART(*s);
		s++;
	}
}
