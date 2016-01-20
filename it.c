#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#include "cmd.h"
#include "eeprom.h"
#include "it.h"
#include "usart.h"

uint16_t m[16];


ISR(TIMER0_OVF_vect)
{
	static uint16_t t;
	static uint16_t seconds;
	static uint16_t p = EEPROM_SIZE - 1;
	uint16_t time, i;
	char cmd[100];

	t++;
	if (t > F_CPU / 256 / 1024) {
		t = 0;
		seconds++;
		time =
		    ((uint16_t) readEEPROM(p)) << 8 |
		    readEEPROM(p - 1);
		if (time != 0xffff) {
			if (seconds >= time) {
				p -= 2;
				for (i = 0;; i++, p--) {
					cmd[i] = readEEPROM(p);
					if (cmd[i] == '\n' || cmd[i] == 19) {
					    cmd[i + 1] = 0;
						p--;
						break;
					}
				}
				print("Auto running command:");
				print(cmd);
				runCmd(cmd);
				seconds = 0;
			}
		}
	}
}

ISR(TIMER2_OVF_vect)
{
	static uint16_t t;
	static uint8_t sec;
	uint16_t i;

	t++;
	if (t > F_CPU / 256 / 1024) {
		t = 0;
		sec++;
		if (sec >= 60) {
			sec = 0;
			for (i = 0; i < 16; i++) {
				if (m[i] > 0) {
					m[i]--;
				} else {
					if (i < 8) {
						PORTA ^= 1 << i;
					} else {
						PORTC ^= 1 << (i - 8);
					}

					if (status & 1 << i) {
						m[i] = dutyL[i];
					} else {
						m[i] = dutyH[i];
					}
					status ^= 1 << i;
				}
			}
		}
	}
}

ISR(USART_RXC_vect)
{
	char codeUSART[MAXCMDLEN];
	uint8_t i;

	for (i = 0; i < MAXCMDLEN; i++) {
		codeUSART[i] = receiveUSART();
		if (codeUSART[i] == 19 || codeUSART[i] == '\n') {
			codeUSART[i] = '\0';
			break;
		}
	}

	if (codeUSART[0]) {
		print("\nReceived:\n");
		for (i = 0; i < MAXCMDLEN; i++) {
			sendUSART(codeUSART[i]);
			if (codeUSART[i] == '\0')
				break;
		}
		print("\n");
	}

	runCmd(codeUSART);
}

void initUSART()
{
	// UCSRA |= (1 << U2X);
	UBRRH = 0;
	UBRRL = 52;		// 9600Hz on 8MHz F_CPU
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

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
