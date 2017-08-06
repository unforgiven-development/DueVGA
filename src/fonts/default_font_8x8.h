/**
 * \file default_font_8x8.h
 * This file defines the default font used by the video driver. Each character (glyph) is 8x8 pixels in dimension.
 *
 * \author	Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * \date	2017
 * \version	0.1
 *
 * \todo Add a font definition to this file
 */

#ifndef _FONTS_DEFAULT_FONT_8X8_H__
#define _FONTS_DEFAULT_FONT_8X8_H__


unsigned char _font_default_8x8[256][8] = {

/*
 * Character Defintion
 * Character Number: 0
 * Character: [NULL]
 * Notes: This is the "NULL" character
 */
{
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00
}

};



#endif /* _FONTS_DEFAULT_FONT_8X8_H__ */
