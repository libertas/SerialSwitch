#include <avr/interrupt.h>
#include <avr/io.h>

#include "cmd.h"
#include "eeprom.h"
#include "it.h"
#include "timer.h"
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
		g_seconds++;

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
				printf("time:%ld\n", g_seconds);
				printf("Auto running command:");
				printf(cmd);
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
		printf("\nReceived:\n");
		for (i = 0; i < MAXCMDLEN; i++) {
			sendUSART(codeUSART[i]);
			if (codeUSART[i] == '\0')
				break;
		}
		printf("\n");
	}

	runCmd(codeUSART);
}

