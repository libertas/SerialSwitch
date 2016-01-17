#include <stdint.h>

#define EEPROM_SIZE 512

extern uint16_t stackTail;
extern uint16_t dutyH[16], dutyL[16];
extern uint16_t status;

void writeEEPROM(unsigned int addr, unsigned char data);
unsigned char readEEPROM(unsigned int addr);
void setDuty();
