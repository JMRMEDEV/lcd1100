#include <18F45K50.h> 
#device PIC18F45K50
#use delay(crystal=8M, clock=32M) 
#fuses HSM 
#FUSES NOWDT, NOPROTECT,NOLVP, MCLR
#include "lcd1100_lib.c"
#include "scrollingscreen.c"

void main(void) {

    lcd_init();
    movescreen();
    
}

