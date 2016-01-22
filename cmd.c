#include <avr/io.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <string.h>

#include "usart.h"
#include "cmd.h"
#include "eeprom.h"

char buf[100];

void runCmd(char code[])
{
	uint16_t i;
	uint16_t addr;
	uint16_t time, time1;
	char port, pin;
	addr = code[1] - '0';
	switch (code[0]) {
	case 'A':		// control port a
		if (code[2] - '0') {
			writeEEPROM(addr, '1');
			PORTA |= 1 << addr;
			sprintf(buf, "UA%c65535/0", code[1]);
			status |= 1 << addr;
		} else {
			writeEEPROM(addr, '0');
			PORTA &= ~(1 << addr);
			sprintf(buf, "UA%c0/65535", code[1]);
			status &= ~(1 << addr);
		}
		runCmd(buf);
		break;
	case 'C':		// control port c
		if (code[2] - '0') {
			writeEEPROM(8 + addr, '1');
			PORTC |= 1 << addr;
			sprintf(buf, "UC%c65535/0", code[1]);
			status |= 1 << (8 + addr);
		} else {
			writeEEPROM(8 + addr, '0');
			PORTC &= ~(1 << addr);
			sprintf(buf, "UC%c0/65535", code[1]);
			status ^= ~(1 << (8 + addr));
		}
		runCmd(buf);
		break;
	case 'E':		// set eeprom data
		if (code[2] == 'i') {
			for (i = 0; i < 16; i++) {
				wdt_reset();
				writeEEPROM(i, code[1]);
			}
		} else if (code[2] == 'f')
		{
			if(code[1] == 'a')
				addr = EEPROM_SIZE;
			else
				addr = 16;
			for (i = 0; i < addr; i++) {
				wdt_reset();
				writeEEPROM(i, 0xff);
			}
		} else if (code[2] == 'r') {
			PORTA = ~readEEPROM(code[1]);
		} else if (code[1] == 'R') {
			if (sscanf(code + 2, "%d", &addr) == 1) {
				sprintf(buf, "%d.%x\n", addr, readEEPROM(addr));
				printf(buf);
			} else {
				for (i = 0; i < EEPROM_SIZE; i++) {
					sprintf(buf, "%d.%x\n", i,
						readEEPROM(i));
					printf(buf);
				}
			}
		} else
			writeEEPROM(code[1], code[2]);
		break;
	case 'T':		// Timer
		if (sscanf(code, "T%d%s", &time, code) == 2) {
			writeEEPROM(stackTail--, (uint8_t) (time >> 8));
			writeEEPROM(stackTail--, (uint8_t) time);
			for (i = 0; i < strlen(code); i++) {
				writeEEPROM(stackTail--, code[i]);
			}
			writeEEPROM(stackTail--, '\n');
		} else {
			printf("Wrong code!\n");
			return;
		}
		break;
	case 'U':		// Duty of each pin
		if (sscanf(code, "U%c%c%d/%d", &port, &pin, &time, &time1) == 4) {
			if (port == 'A')
				addr = 16;
			else if (port == 'C')
				addr = 48;
			else {
				printf("Wrong port\n");
				return;
			}
			if (pin - '0' < 10 && pin - '0' >= 0)
				addr += (pin - '0') * 4;
			writeEEPROM(addr, time >> 8);
			writeEEPROM(addr + 1, time);
			writeEEPROM(addr + 2, time1 >> 8);
			writeEEPROM(addr + 3, time1);
			setDuty();
		}
		break;
	case 'R':		// Reboot
		printf("REBOOTING\n");
		wdt_enable(WDTO_15MS);
		while (1);
		break;
	case 'F':		// Print CPU frequency
		sprintf(buf, "F_CPU: %ld Hz\n", F_CPU);
		printf(buf);
		break;
	default:
		sprintf(buf, "Unrecognized code:%s\n", code);
		printf(buf);
		break;
	}
}
