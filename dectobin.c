//***************************************************************************
//  File........: dectobin.c
//  Author(s)...: JMRMEDEV
//  URL(s)......: https://github.com/JMRMEDEV/lcd1100/
//  Device(s)...: PIC18F45K50 (May be adapted to any PIC)
//  Compiler....: CCS
//  Description.: Decimal to array of bits (MSB at the last position) converter
//  Date........: 25.12.19 
//  Version.....: 0.0.1
//***************************************************************************

int1 bin[7];

void initbinary(unsigned char size);
void dectobin(unsigned char dec);

void initbinary(unsigned char size) {
    if (size <= 0) {
        return;
    }
    for (unsigned char i = 0; i < size; ++i) {
        bin[i] = 0;
    }
}

void dectobin(unsigned char dec) {
    int pos = 0;
    initbinary(7);
    if (dec == 0) {
        return;
    }
    if (dec == 1) {
        bin[1] = 1;
        return;
    }
    unsigned char quotient;
    unsigned char remainder;

    do {
        quotient = dec / 2;
        remainder = dec % 2;
        bin[pos] = remainder;
        if (quotient < 2) {
            bin[pos + 1] = quotient;
        }
        dec = quotient;
        ++pos;
    } while (quotient >= 2);
}