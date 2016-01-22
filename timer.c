#include <avr/io.h>


void initTimer0()
{
	TCCR0 = (1 << CS02) | (1 << CS00);	// CLKio / 1024
	TCNT0 = 0;
	TIMSK |= _BV(TOIE0);
}

void initTimer2()
{
	TCCR2 = (1 << CS22) | (1 << CS21) | (1 << CS20);	// CLKio / 1024
	TCNT2 = 0;
	TIMSK |= _BV(TOIE2);
}
