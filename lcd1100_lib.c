//***************************************************************************
//  File........: lcd1100_lib.h
//  Author(s)...: JMRMEDEV, based on the work by Chiper and demonspells
//  URL(s)......: https://github.com/JMRMEDEV/lcd1100/
//  Device(s)...: PIC18F45K50 (May be adapted to any PIC)
//  Compiler....: CCS
//  Description.: Nokia 1100 LCD controller driver with graphic functions
//  Data........: 17.12.19
//  Version.....: 0.0.1
//***************************************************************************

#include "lcd1100_font1.c"

// Port pin numbers to which the LCD controller pins are connected
#define SCLK PIN_B4
#define SDA PIN_B5
#define CS PIN_B6
#define RST PIN_B7

// Macros for working with bits
#define ClearBit(reg, bit)       reg &= (~(1<<(bit)))
#define SetBit(reg, bit)         reg |= (1<<(bit))	
#define InvBit(reg, bit)         reg ^= 1<<bit

// Macros definitions, utility variables
#define CMD 0
#define DATA 1

// Pixel display resolution
#define lcd_X_RES 96 // horizontal resolution
#define lcd_Y_RES 68 // vertical resolution

// Video buffer. We work through the buffer, since data cannot be read from the Nokia 1100 controller but for
// graphics mode we need to know the contents of the video memory (9 banks of 96 bytes each)
static unsigned char lcd_memory[lcd_X_RES - 1][(lcd_Y_RES / 8) + 1];

// Current coordinates (pointers) in the video buffer
// lcd_xcurr - in pixels, lcd_ycurr- in banks (lines)
static unsigned char lcd_xcurr, lcd_ycurr;

// Function prototypes
void lcd_init(void);
void lcd_write(int1 cd, unsigned char c);
void lcd_clear(void);
void setx(char x);
void sety(char y);
void gotoxy(char x, char y);
void lcd_inverse(unsigned int1 mode);
void lcd_gotoxy_pix(char x, char y);
void vline(char x, char y, char on);
void line(unsigned char x, unsigned char y, unsigned char y2, unsigned char on);
void print_char(char c);

//******************************************************************************
// Controller initialization
void lcd_init(void)
{
   output_low(CS);
   output_low(RST);
   delay_ms(5); // Wait at least 5ms to install the generator (less than 5 ms may not work)
   output_high(RST);
   lcd_write(CMD, 0x20); // write VOP register
   lcd_write(CMD, 0x90);
   lcd_write(CMD, 0xA4); // All on/normal display
   lcd_write(CMD, 0x2F); // Power control set(charge pump on/oFF)
   lcd_write(CMD, 0x40); // Set start row address = 0
   lcd_write(CMD, 0xb0); // Set Y-address = 0
   lcd_write(CMD, 0x10); // Set X-address, upper 3 bits
   lcd_write(CMD, 0x0);  // Set X-address, lower 4 bits
   lcd_write(CMD, 0xC8); // Mirror Y axis (about X axis)
   lcd_write(CMD, 0xa1); // Invert screen in horizontal axis
   lcd_write(CMD, 0xac); // Set initial row (R0) of the display
   lcd_write(CMD, 0x07);
   lcd_write(CMD, 0xAF); // Display ON/OFF

   lcd_clear();          // Clear LCD
   lcd_write(CMD, 0xa7); // Invert display
   delay_ms(100);
   lcd_write(CMD, 0xa6); // Normal display (non inverted)
   delay_ms(100);
}

//******************************************************************************
// Screen cleaning
void lcd_clear(void)
{
   unsigned int i;
   
   lcd_write(CMD, 0x40); // Y = 0
   lcd_write(CMD, 0xb0);
   lcd_write(CMD, 0x10); // X = 0
   lcd_write(CMD, 0x0);

   lcd_xcurr = 0;
   lcd_ycurr = 0; // Set to 0 the current coordinates in the video buffer

   lcd_write(CMD, 0xAE); // Disable display
   for (i = 0; i < 255; i++)
      lcd_write(DATA, 0x00);
   for (i = 0; i < 255; i++)
      lcd_write(DATA, 0x00);
   for (i = 0; i < 255; i++)
      lcd_write(DATA, 0x00);
   for (i = 0; i < 99; i++)
      lcd_write(DATA, 0x00);
   lcd_write(CMD, 0xAF); // Enable display
}

//******************************************************************************
// write (CMD or DATA) transfer to the LCD controller
//  mode: CMD - pass the command
//		  DATA - transfer data
//  c: value of transmitted byte
void lcd_write(int1 cd, unsigned char c)
{
   output_low(CS);
   output_low(SCLK);

   if (cd == 1)
   {
      lcd_memory[lcd_xcurr][lcd_ycurr] = c; // Write data to the video buffer

      lcd_xcurr++; // Update the coordinates in the video buffer

      if (lcd_xcurr > 95)
      {
         lcd_xcurr = 0;
         lcd_ycurr++;
      }

      if (lcd_ycurr > 8)
         lcd_ycurr = 0;
      output_high(SDA);
   }
   else
   {
      output_low(SDA);
   }

   output_low(CS);
   output_low(SCLK);
   output_bit(SDA, cd);
   output_high(SCLK);

   for (unsigned char i = 0; i < 8; i++)
   {
      output_low(SCLK);
      if ((c & 0x80))
         output_high(SDA);
      else
         output_low(SDA);
      output_high(SCLK);
      c <<= 1;
      delay_us(34);
   }
   output_high(CS);
}

//******************************************************************************
// Sets the cursor to the desired position. The countdown begins in the
// upper left corner. Horizontal 16 familiarity, vertical - 8
//  x: 0..15
//  y: 0..7
void gotoxy(char x, char y)
{
   x = x * 6; // We pass from the coordinate in familiarity to the coordinates in pixels

   lcd_xcurr = x;
   lcd_ycurr = y;

   lcd_write(CMD, (0xB0 | (y & 0x0F)));        // Y axis initialisation: 0100 yyyy
   lcd_write(CMD, (0x00 | (x & 0x0F)));        // X axis initialisation: 0000 xxxx ( x3 x2 x1 x0)
   lcd_write(CMD, (0x10 | ((x >> 4) & 0x07))); // X axis initialisation: 0010 0xxx  ( x6 x5 x4)
}

//******************************************************************************
// Set the inversion mode of the entire screen. Data on the screen does not change, only inverted
//  mode: 1 or 0
void lcd_inverse(unsigned char mode)
{
   if (mode)
      lcd_write(CMD, 0xA6);
   else
      lcd_write(CMD, 0xA7);
}

//******************************************************************************
// Sets the cursor in pixels. The countdown begins in the upper
// left corner. 96 pixels horizontally, 65 pixels vertically
//  x: 0..95
//  y: 0..64
void lcd_gotoxy_pix(char x, char y)
{
   lcd_xcurr = x;
   lcd_ycurr = y / 8;

   lcd_write(CMD, (0xB0 | (lcd_ycurr & 0x0F))); // Y address setting: 0100 yyyy
   lcd_write(CMD, (0x00 | (x & 0x0F)));          // X address setting: 0000 xxxx - bits (x3 x2 x1 x0)
   lcd_write(CMD, (0x10 | ((x >> 4) & 0x07)));   // X address setting: 0010 0xxx - bits (x6 x5 x4)
}

void lcd_pixel(unsigned char x,unsigned char y, int pixel_mode)
{
	unsigned char temp;

	lcd_gotoxy_pix(x,y);        
	temp=lcd_memory[lcd_xcurr][lcd_ycurr];

	switch(pixel_mode)
	{
    	case 0:
        	SetBit(temp, y%8);			// Turn on the pixel
			break;
    	case 1:
     		ClearBit(temp, y%8);		// Turn off the pixel
			break;
    	case 2:
     		InvBit(temp, y%8);			// Invert pixel
			break;
	}
	
	lcd_memory[lcd_xcurr][lcd_ycurr] = temp; // transfer the byte to the video buffer
	lcd_write(DATA,temp); // We pass the byte to the controller
}

void print_char(unsigned char c)
{
   for (unsigned char i = 0; i < 5; i++)
   {
      lcd_write(DATA, lcd_Font[c - 0x20][i]);
   }
   lcd_write(DATA, 0x00);
}