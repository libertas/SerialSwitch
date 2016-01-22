#include <avr/io.h>
#include <avr/wdt.h>

#include "usart.h"


static FILE mystdout = FDEV_SETUP_STREAM(myfputc, NULL,_FDEV_SETUP_WRITE);

void initUSART()
{
	UCSRA |= (1 << U2X);
	UBRRH = 0;
	UBRRL = 207;		// 9600Hz on 16MHz F_CPU
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (3 << UCSZ0);

	stdout = &mystdout;
}

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

char myfputc(char ch, FILE *f)
{
	sendUSART(ch);
	return ch;
}
