/**
 * \file text.cpp
 * Implements the functions for drawing text to the display.
 *
 * \author		Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * \author		stimmer <stimmylove@gmail.com>
 * \date		2013-2017
 * \copyright	This library is free software; you can redistribute it and/or
 *				modify it under the terms of the GNU Lesser General Public
 *				License as published by the Free Software Foundation; either
 *				version 2.1 of the License, or (at your option) any later version.
 *
 *				This library is distributed in the hope that it will be useful,
 *				but WITHOUT ANY WARRANTY; without even the implied warranty of
 *				MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *				See the GNU Lesser General Public License for more details.
 *
 *				You should have received a copy of the GNU Lesser General Public
 *				License along with this library; if not, write to the Free Software
 *				Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 *
 *
 */


#include "VGA.h"


void Vga::drawText(char *text, int x, int y, int c, int b, int dir) {
	uint8_t t;

	while (t = (uint8_t)*text++) {
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++) {
				switch (dir) {
					case -1:
					case 0:
						if (_vga_font8x8[8 * t + j] & (128 >> i)) {
							drawPixel(x + i, y + j, c);
						} else if (b != -256) {
							drawPixel(x + i, y + j, b);
						}
						break;
					case 1:
						if (_vga_font8x8[8 * t + j] & (128 >> i)) {
							drawPixel(x + j, y - i, c);
						} else if (b != -256) {
							drawPixel(x + j, y - i, b);
						}
						break;
					case 2:
						if (_vga_font8x8[8 * t + j] & (128 >> i)) {
							drawPixel(x - i, y - j, c);
						} else if (b != -256) {
							drawPixel(x - i, y - j, b);
						}
						break;
					case 3:
						if (_vga_font8x8[8 * t + j] & (128 >> i)) {
							drawPixel(x - j, y + i, c);
						} else if (b != -256) {
							drawPixel(x - j, y + i, b);
						}
						break;
					default:
						break;
				}
			}
		}

		if (dir == 0) {
			x += 8;
		} else if (dir == 1) {
			y -= 8;
		} else if (dir == 2) {
			x -= 8;
		} else {
			y += 8;
		}
	}
}


void Vga::moveCursor(int column, int line) {
	tx = column;
	if (tx < 0) {
		tx = 0;
	}
	if (tx >= tww) {
		tx = tww - 1;
	}
	ty = line;
	if (ty < 0) {
		ty = 0;
	}
	if (ty >= twh) {
		ty = twh - 1;
	}
}


void Vga::setPrintWindow(int left, int top, int width, int height) {

	/*
	 * Check that the desired left/top edges of the window are non-negative, and that they do not exceed the maximum
	 * allowed text width and text height. If they fall outside of this allowed range, adjust the location of the
	 * edge(s) accordingly.
	 */
	if (left < 0) {
		/* If value is negative, set it to 0 */
		left = 0;
	}

	if (left >= tw) {
		/* If value is too large, set it to the maximum text width, minus one */
		left = tw - 1;
	}

	if (top < 0) {
		/* If value is negative, set it to 0 */
		top = 0;
	}

	if (top >= th) {
		/* If the value is too large, set it to the maximum text height, minus one */
		top = th - 1;
	}


	/*
	 * Check that the window's left/top edges plus the desired width/height don't exceed the maximum allowed text width
	 * and text height. If they do exceed this value, reduce the width/height accordingly.
	 */
	if (left + width > tw) {
		width = tw - left;
	}

	if (top + height > th) {
		height = th - top;
	}

	/*
	 * Ensure that the dimensions of the window will be greater than zero.
	 *
	 * As a window with one of the dimensions being zero would not be drawn, we can just return from the function
	 * immediately if either of the values are zero.
	 */
	if (width <= 0 || height <= 0) {
		return;
	}


	/*
	 * Set the text window's left edge, top edge, width, and height. Additionally, set the text cursor's initial
	 * position to (0, 0) (ie: the top left corner).
	 */
	twx = left;
	twy = top;
	tww = width;
	twh = height;
	tx = 0;
	ty = 0;
}


void Vga::clearPrintWindow() {
	fillRect(twx * 8, twy * 8, (twx + tww) * 8 - 1, (twy + twh) * 8 - 1, paper);
	tx = 0;
	ty = 0;
}


void Vga::scrollPrintWindow() {
	if (tww == tw && twh == th) { // fast version where no text window
		if (videoOutputMode == VGA_MONO) {
			/* ---( VGA -- monochrome videoOutputMode )--- */
			uint16_t *a = pb;
			uint16_t *b = a + 8 * pw;
			memmove((uint8_t *) a, (uint8_t *) b, 2 * pw * (ysize - 8));
			for (int i = 0; i < 8; i++) {
				memset((uint8_t * )(a + pw * (ysize - 8 + i)), (ink & 1) ? 0 : 255, 2 * (pw - 2));
			}
		} else if (videoOutputMode & VGA_COLOR) {
			/* ---( VGA -- color videoOutputMode )--- */
			uint8_t *a = cb, *b = cb + 8 * cw;
			memmove(a, b, cw * (ysize - 8));
			memset(a + cw * (ysize - 8), paper, cw * 8);
		}
	} else {
		scroll(twx * 8, twy * 8, tww * 8, twh * 8, 0, -8, paper);
	}

	tx = 0;
	ty = twh - 1;
}


size_t Vga::write(uint8_t c) {
	if (c == 13) {
		tx = 0;
		return (1);
	}
	if (c == 10) {
		tx = 0;
		ty++;
		if (ty == twh) {
			scrollPrintWindow();
		}
		return 1;
	}
	if (c == 9) {
		write(32);
		for (int r = 1; r <= 8; r++) {
			if (tx & 7) {
				write(32);
			} else {
				return r;
			}
		}
	}

	if (videoOutputMode == VGA_MONO) {
		uint8_t *a = (uint8_t *) pb + (twy + ty) * 16 * pw + ((twx + tx) ^ 1);
		for (int j = 0; j < 8; j++) {
			*a = _vga_font8x8[8 * c + j] ^ ((ink & 1) ? 0 : 255);
			a += pw * 2;
		}
	} else if (videoOutputMode & VGA_COLOR) {
		uint8_t *a = (uint8_t *) cb + (twy + ty) * 8 * cw + (twx + tx) * 8;
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++) {
				a[i] = (_vga_font8x8[8 * c + j] & (128 >> i)) ? ink : paper;
			}
			a += cw;
		}
	}

	tx++;
	if (tx == tww) {
		tx = 0;
		ty++;
		if (ty == twh) {
			scrollPrintWindow();
		}
	}

}


size_t Vga::write(const uint8_t *buffer, size_t size) {
	int rv = 0;
	for (int i = 0; i < size; i++) {
		rv += write(buffer[i]);
	}
	return rv;
}
