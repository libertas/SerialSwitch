#include <stdio.h>


void initUSART();
void sendUSART(unsigned char data);
unsigned char receiveUSART();

char myfputc(char ch, FILE *f);
