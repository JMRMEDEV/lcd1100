# lcd1100_lib

## A C library for interfacing with the Nokia 1100 graphic LCD for PIC

Have you been driving yourself crazy trying to find a library meant for interfacing with the common Nokia 1100 graphic LCD without success? Have you searched once and once again with no results? Have you found some libraries for this purpose but they are not updated since some years? Have you found some library but is not for Michrochip's PIC? 

If your answer to any of these questions was "yes", then you have found a Holly Grail right here.

**lcd1100_lib** is a **library** written in C language meant for interfacing the Nokia 1100 "65.96" pixels graphic LCD through a Michrochip's PIC Microcontroller. This library is based on the work previously done by "Chiper" and "demonspells" available at https://digitalchip.ru/podklyuchenie-displeya-ot-nokia-1100-chast-2 and https://www.ccsinfo.com/forum/viewtopic.php?t=44928 respectively. It takes the best of both libraries for getting as many functions as possible such as setting a desired position for the cursor for writing characters in character sizes or pixels, drawing circles, lines, pixels and rectangles at fixed positions on the screen, writing characters in wide mode for titles and more.

### All commented

As it has been a headache to understand the way the controller (PCF8814) included in this LCD and the way of interfacing with it, I tooked most of the comments written by Chiper and properly translated them to english and some more added by myself.

### Requirements

-MPLABX or any IDE thought for working with the CCS compiler.
-The files contained in this library: lib1100_lcd.c (contains the main functionality and definitions of the library, here you should adapt the defined pins used to your own design and the configuration directives to your own PIC), lcd1100_font1.c (contains the definition of characters following the standard ASCII), dectobin.c (contains a function meant to translate the ASCII code into bit arrays for drawing characters at fixed coordinates in pixels) and main.c (for testing all the files).

### Use

Just add the "lib1100.c" to your main file and acces to all the available functions.

### Functions

The available functions for working with the LCD available in this library are:

// Controller initialization
void lcd_init(void);
// write (CMD or DATA) transfer to the LCD controller
void lcd_write(int1 cd, unsigned char c);
// Screen cleaning
void lcd_clear(void);
// Sets the cursor to the desired position. The countdown begins in the
// upper left corner. Horizontal 16 familiarity, vertical - 8
void gotoxy(char x, char y);
// Set the inversion mode of the entire screen. Data on the screen does not change, only inverted
void lcd_inverse(unsigned int1 mode);
// Sets the cursor in pixels. The countdown begins in the upper
// left corner. 96 pixels horizontally, 65 pixels vertically
void lcd_gotoxy_pix(char x, char y);
// Conclusion of a point on the Nokia 1100 LCD screen
//  x: 0..95  horizontal coordinate (counting from the upper left corner)
//	y: 0..64  vertical coordinate
void lcd_pixel(unsigned char x, unsigned char y, unsigned char pixel_mode)
// Display a character on the NOKIA 1100 LCD screen at the specified coordinates in pixels
void pix_char(unsigned char x, unsigned char y, unsigned char c);
// Line output to the Nokia 1100 LCD screen
void lcd_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char pixel_mode);
// Display a circle on the Nokia 1100 LCD screen
void lcd_circle(unsigned char x, unsigned char y, unsigned char radius, unsigned char fill, int pixel_mode);
// Display a rectangle on the Nokia 1100 LCD screen
void lcd_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char fill, unsigned char pixel_mode);
// Display a character on the NOKIA 1100 LCD screen at the current location
void print_char(char c);

### Made With Love

It was a titanic task to understand the whole problem and adapting the whole libraries to my specific needs. However, I did this with the attempt to also help others with my same struggles. As it is obvious there are probably many ways to improve this code, its functionality and performance. All the polite comments are welcome.
