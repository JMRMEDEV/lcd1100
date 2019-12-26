//***************************************************************************
//  File........: lcd1100_lib.c
//  Author(s)...: JMRMEDEV, based on the work by Chiper and demonspells
//  URL(s)......: https://github.com/JMRMEDEV/lcd1100/
//  Device(s)...: PIC18F45K50 (May be adapted to any PIC)
//  Compiler....: CCS
//  Description.: Nokia 1100 LCD controller driver with graphic functions
//  Date........: 25.12.19
//  Version.....: 0.0.2
//***************************************************************************

#ifndef _LCD1100_LIB_C_
#define _LCD1100_LIB_C_

#include "lcd1100_font1.c"
#include "dectobin.c"

// Port pin numbers to which the LCD controller pins are connected
#define SCLK PIN_D4
#define SDA PIN_D5
#define CS PIN_D6
#define RST PIN_D7

// Macros for working with bits
#define ClearBit(reg, bit) reg &= (~(1 << (bit)))
#define SetBit(reg, bit) reg |= (1 << (bit))
#define InvBit(reg, bit) reg ^= 1 << bit

// Macros definitions, utility variables
#define CMD 0
#define DATA 1

#define PIXEL_ON 0
#define PIXEL_OFF 1
#define PIXEL_INV 2

#define FILL_OFF 0
#define FILL_ON 1

#define INV_MODE_ON 0
#define INV_MODE_OFF 1

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
void pix_char(unsigned char x, unsigned char y, unsigned char c);
void lcd_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char pixel_mode);
void lcd_circle(unsigned char x, unsigned char y, unsigned char radius, unsigned char fill, int pixel_mode);
void lcd_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char fill, unsigned char pixel_mode);
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
    lcd_write(CMD, 0xB0); // Set Y-address = 0
    lcd_write(CMD, 0x10); // Set X-address, upper 3 bits
    lcd_write(CMD, 0x00); // Set X-address, lower 4 bits
    lcd_write(CMD, 0xC8); // Mirror Y axis (about X axis) [change 0xC8 for 0xC0 for mirroring]
    lcd_write(CMD, 0xA1); // Invert screen in horizontal axis
    lcd_write(CMD, 0xAC); // Set initial row (R0) of the display
    lcd_write(CMD, 0x07);
    lcd_write(CMD, 0xAF); // Display ON/OFF

    lcd_clear();          // Clear LCD
    lcd_write(CMD, 0xA7); // Invert display
    delay_ms(100);
    lcd_write(CMD, 0xA6); // Normal display (non inverted)
    delay_ms(100);
}

//******************************************************************************
// Screen cleaning
void lcd_clear(void)
{
    unsigned int i;

    lcd_write(CMD, 0x40); // Y = 0
    lcd_write(CMD, 0xB0);
    lcd_write(CMD, 0x10); // X = 0
    lcd_write(CMD, 0x00);

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

    //lcd_write(CMD, (0xB0 | (y & 0x0F))); // Y axis initialisation: 0100 yyyy
    lcd_write(CMD, (0xB0 | (y & 0x0F)));        // Y axis initialisation: 0100 yyyy
    lcd_write(CMD, (0x00 | (x & 0x0F)));        // X axis initialisation: 0000 xxxx ( x3 x2 x1 x0)
    lcd_write(CMD, (0x10 | ((x >> 4) & 0x07))); // X axis initialisation: 0010 0xxx  ( x6 x5 x4)
}

//******************************************************************************
// Set the inversion mode of the entire screen. Data on the screen does not change, only inverted
//  mode: INV_MODE_ON or INV_MODE_OFF
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
    lcd_write(CMD, (0x00 | (x & 0x0F)));         // X address setting: 0000 xxxx - bits (x3 x2 x1 x0)
    lcd_write(CMD, (0x10 | ((x >> 4) & 0x07)));  // X address setting: 0010 0xxx - bits (x6 x5 x4)
}

//******************************************************************************
// Conclusion of a point on the Nokia 1100 LCD screen
//  x: 0..95  horizontal coordinate (counting from the upper left corner)
//	y: 0..64  vertical coordinate
//	pixel_mode: PIXEL_ON  - to enable pixel
//				PIXEL_OFF - to turn off the pixel
//				PIXEL_INV - to invert a pixel
void lcd_pixel(unsigned char x, unsigned char y, unsigned char pixel_mode)
{
    unsigned char temp;

    lcd_gotoxy_pix(x, y);
    temp = lcd_memory[lcd_xcurr][lcd_ycurr];

    switch (pixel_mode)
    {
    case PIXEL_ON:
        SetBit(temp, y % 8); // Turn on the pixel
        break;
    case PIXEL_OFF:
        ClearBit(temp, y % 8); // Turn off the pixel
        break;
    case PIXEL_INV:
        InvBit(temp, y % 8); // Invert pixel
        break;
    }

    lcd_memory[lcd_xcurr][lcd_ycurr] = temp; // transfer the byte to the video buffer
    lcd_write(DATA, temp);                   // We pass the byte to the controller
}

//******************************************************************************
// Display a character on the NOKIA 1100 LCD screen at the specified coordinates in pixels
//  c: character code
//  x: 0..95  horizontal coordinate (counting from the upper left corner)
//	y: 0..64  vertical coordinate
void pix_char(unsigned char x, unsigned char y, unsigned char c)
{
    for (unsigned char j = 0; j < 5; j++)
    {
        dectobin(lcd_Font[c - 0x20][j]);
        for (unsigned char k = 0; k < 7; k++)
        {
            if (bin[k] == 1)
            {
                lcd_pixel(x + j, y + k, PIXEL_ON);
            }
            else
            {
                lcd_pixel(x + j, y + k, PIXEL_OFF);
            }
        }
    }
}

//******************************************************************************
// Display a character on the NOKIA 1100 LCD screen at the current location
//  c: character code
void print_char(unsigned char c)
{
    for (unsigned char i = 0; i < 5; i++)
    {
        lcd_write(DATA, lcd_Font[c - 0x20][i]);
    }
    lcd_write(DATA, 0x00);
}

// Display a string of characters on the Nokia 1100 LCD screen at its current location, if the line exists
// behind the screen in the current line, the remainder is transferred to the next line.
//  message: pointer to a string of characters. 0x00 - a sign of the end of the line.
void lcd_print(char *message)
{
    while (*message)
        print_char(*message++); // The end of the line is indicated by zero
}

//******************************************************************************
// Line output to the Nokia 1100 LCD screen
//  x1, x2: 0..95  horizontal coordinate (counting from the upper left corner)
//	y1, y2: 0..64  vertical coordinate
//	pixel_mode: 0  - to enable the pixel
//				1 - tu turn off the pixel
//				2 - to invert a pixel
void lcd_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char pixel_mode)
{
    int dy, dx;
    signed char addx = 1, addy = 1;
    signed int P, diff;

    unsigned char i = 0;

    dx = abs((signed char)(x2 - x1));
    dy = abs((signed char)(y2 - y1));

    if (x1 > x2)
        addx = -1;
    if (y1 > y2)
        addy = -1;

    if (dx >= dy)
    {
        dy *= 2;
        P = dy - dx;

        diff = P - dx;

        for (; i <= dx; ++i)
        {
            lcd_pixel(x1, y1, pixel_mode);

            if (P < 0)
            {
                P += dy;
                x1 += addx;
            }
            else
            {
                P += diff;
                x1 += addx;
                y1 += addy;
            }
        }
    }
    else
    {
        dx *= 2;
        P = dx - dy;
        diff = P - dy;

        for (; i <= dy; ++i)
        {
            lcd_pixel(x1, y1, pixel_mode);

            if (P < 0)
            {
                P += dx;
                y1 += addy;
            }
            else
            {
                P += diff;
                x1 += addx;
                y1 += addy;
            }
        }
    }
}

//******************************************************************************
// Display the circle on the Nokia 1100 LCD screen
//  x: 0..95  coordinate of the circle (counting from the upper left corner)
//	y: 0..64  vertical coordinate
//  radius:   circle radius
//  fill:		FILL_OFF  - no fill circle
//				FILL_ON	  - with fill
//	pixel_mode: PIXEL_ON  - to enable the pixel
//				PIXEL_OFF - to turn off the pixel
//				PIXEL_INV - to invert a pixel
void lcd_circle(unsigned char x, unsigned char y, unsigned char radius, unsigned char fill, int pixel_mode)
{
    signed char a, b, P;

    a = 0;
    b = radius;
    P = 1 - radius;

    do
    {
        if (fill)
        {
            lcd_line(x - a, y + b, x + a, y + b, pixel_mode);
            lcd_line(x - a, y - b, x + a, y - b, pixel_mode);
            lcd_line(x - b, y + a, x + b, y + a, pixel_mode);
            lcd_line(x - b, y - a, x + b, y - a, pixel_mode);
        }
        else
        {
            lcd_pixel(a + x, b + y, pixel_mode);
            lcd_pixel(b + x, a + y, pixel_mode);
            lcd_pixel(x - a, b + y, pixel_mode);
            lcd_pixel(x - b, a + y, pixel_mode);
            lcd_pixel(b + x, y - a, pixel_mode);
            lcd_pixel(a + x, y - b, pixel_mode);
            lcd_pixel(x - a, y - b, pixel_mode);
            lcd_pixel(x - b, y - a, pixel_mode);
        }

        if (P < 0)
            P += 3 + 2 * a++;
        else
            P += 5 + 2 * (a++ - b--);
    } while (a <= b);
}

//******************************************************************************
// Display a rectangle on the Nokia 1100 LCD screen
//  x1, x2: 0..95  horizontal coordinate (counting from the upper left corner)
//	y1, y2: 0..64  vertical coordinate
//	pixel_mode: PIXEL_ON  - to enable the pixel
//				PIXEL_OFF - to turn off the pixel
//				PIXEL_INV - to invert a pixel
void lcd_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char fill, unsigned char pixel_mode)
{
    if (fill)
    { // With fill
        unsigned char i, xmin, xmax, ymin, ymax;

        if (x1 < x2)
        {
            xmin = x1;
            xmax = x2;
        } // Determine the minimum and maximum coordinate in X
        else
        {
            xmin = x2;
            xmax = x1;
        }

        if (y1 < y2)
        {
            ymin = y1;
            ymax = y2;
        } // Determine the minimum and maximum coordinate in Y
        else
        {
            ymin = y2;
            ymax = y1;
        }

        for (; xmin <= xmax; ++xmin)
        {
            for (i = ymin; i <= ymax; ++i)
                lcd_pixel(xmin, i, pixel_mode);
        }
    }
    else // No fill
    {
        lcd_line(x1, y1, x2, y1, pixel_mode); // Draw the sides of the rectangle
        lcd_line(x1, y2, x2, y2, pixel_mode);
        lcd_line(x1, y1 + 1, x1, y2 - 1, pixel_mode);
        lcd_line(x2, y1 + 1, x2, y2 - 1, pixel_mode);
    }
}

#endif /* _LCD1100_LIB_C_ */