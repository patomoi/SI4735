// ----------------------------------------------------------------------
// Seven Segment font for Tiny4kOLED library.
// 14x24 resolution.
// Charset: '.', '-', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
// Empty space character as a '/'
// v1.0
// ----------------------------------------------------------------------
// By Goshante
// 02.2024
// http://github.com/goshante
// ----------------------------------------------------------------------


#include <avr/pgmspace.h>


const uint8_t ssd1306xled_font14x24sevenSeg [] PROGMEM = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x10,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x40,0xE0,0xE0,0x40,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0xFC,0xFA,0xF6,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0xEF,0xC7,0x83,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0xC7,0xEF,0x00,
	0x00,0x7F,0xBF,0xDF,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF8,0xFC,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0xC7,0xEF,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x3F,0x7F,0x00,
	0x00,0x00,0x02,0x06,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0xE0,0xD0,0xB8,0x38,0x38,0x38,0x38,0x38,0x38,0x3B,0x17,0x0F,0x00,
	0x00,0x7F,0xBF,0xDF,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xC0,0x80,0x00,0x00,
	0x00,0x00,0x02,0x06,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0x00,0x10,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0xBB,0xD7,0xEF,0x00,
	0x00,0x00,0x80,0xC0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00,
	0x00,0xFC,0xF8,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0xF8,0xFC,0x00,
	0x00,0x0F,0x17,0x3B,0x38,0x38,0x38,0x38,0x38,0x38,0xBB,0xD7,0xEF,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x3F,0x7F,0x00,
	0x00,0xFC,0xFA,0xF6,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x06,0x02,0x00,0x00,
	0x00,0x0F,0x17,0x3B,0x38,0x38,0x38,0x38,0x38,0x38,0xB8,0xD0,0xE0,0x00,
	0x00,0x00,0x80,0xC0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00,
	0x00,0xFC,0xFA,0xF6,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x06,0x02,0x00,0x00,
	0x00,0xEF,0xD7,0xBB,0x38,0x38,0x38,0x38,0x38,0x38,0xB8,0xD0,0xE0,0x00,
	0x00,0x7F,0xBF,0xDF,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00,
	0x00,0x00,0x02,0x06,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0xC7,0xEF,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x3F,0x7F,0x00,
	0x00,0xFC,0xFA,0xF6,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0xEF,0xD7,0xBB,0x38,0x38,0x38,0x38,0x38,0x38,0xBB,0xD7,0xEF,0x00,
	0x00,0x7F,0xBF,0xDF,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00,
	0x00,0xFC,0xFA,0xF6,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0xF6,0xFA,0xFC,0x00,
	0x00,0x0F,0x17,0x3B,0x38,0x38,0x38,0x38,0x38,0x38,0xBB,0xD7,0xEF,0x00,
	0x00,0x00,0x80,0xC0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xDF,0xBF,0x7F,0x00
};

const DCfont TinyOLED4kfont14x24sevenSeg = 
{
	(uint8_t*)ssd1306xled_font14x24sevenSeg,
	14, //Character width in pixels
	3, //Character height in pages (8 pixels)
	45, 57 //ASCII extents
};

//For backwards compatibility
#define FONT14X24SEVENSEG (&TinyOLED4kfont14x24sevenSeg)