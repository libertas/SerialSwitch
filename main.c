// Running on an Atmega 16a
// 16 MHz

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define EEPROM_SIZE 512
#define MAXCMDLEN 100

uint16_t stackTail;
uint16_t dutyH[16], dutyL[16];
uint16_t m[16];
uint16_t status;

char buf[101];
char debugStr[100];

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

void initUSART()
{
	UBRRH = 0;
	UBRRL = 103;		// 9600Hz on 16MHz F_CPU
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
		if (code[2] == 'i')
			for (i = 0; i < 16; i++)
				writeEEPROM(i, code[1]);
		else if (code[2] == 'f')
		{
			if(code[1] == 'a')
				addr = EEPROM_SIZE;
			else
				addr = 16;
			for (i = 0; i < addr; i++)
				writeEEPROM(i, 0xff);
		}
		else if (code[2] == 'r')
			PORTA = ~readEEPROM(code[1]);
		else if (code[1] == 'R') {
			if (sscanf(code + 2, "%d", &addr) == 1) {
				sprintf(buf, "%d.%x\n", addr, readEEPROM(addr));
				print(buf);
			} else {
				for (i = 0; i < EEPROM_SIZE; i++) {
					sprintf(buf, "%d.%x\n", i,
						readEEPROM(i));
					print(buf);
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
			print("Wrong code!\n");
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
	default:
		sprintf(buf, "Unrecognised code:%s\n", code);
		print(buf);
		break;
	}
}

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

	while (1);

	return 0;
}

