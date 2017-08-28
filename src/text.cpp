/**
 * \file text.cpp
 * Implements the functions for drawing text to the display.
 *
 * \author		Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * \author		stimmer <stimmylove@gmail.com>
 * \date		2013-2017
 *
 * \copyright \parblock
 * This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with this library;
 * if not, write to the
 *   Free Software Foundation, Inc.
 *   51 Franklin St
 *   Fifth Floor
 *   Boston, MA 02110-1301
 *   USA
 * \endparblock
 */


#include "DueVideoOut.h"


void DueVideoOut::drawText(char *text, int x, int y, int c, int b, int dir) {
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


void DueVideoOut::moveCursor(int destColumn, int destLine) {
	textCursorPosColumnX = destColumn;
	if (textCursorPosColumnX < 0) {
		textCursorPosColumnX = 0;
	}
	if (textCursorPosColumnX >= textWindowWidthInCharColsX) {
		textCursorPosColumnX = textWindowWidthInCharColsX - 1;
	}
	textCursorPosRowY = destLine;
	if (textCursorPosRowY < 0) {
		textCursorPosRowY = 0;
	}
	if (textCursorPosRowY >= textWindowHeightInCharRowsY) {
		textCursorPosRowY = textWindowHeightInCharRowsY - 1;
	}
}


void DueVideoOut::setPrintWindow(int winLeftEdge, int winTopEdge, int width, int height) {

	/*
	 * Check that the desired left/top edges of the window are non-negative, and that they do not exceed the maximum
	 * allowed text width and text height. If they fall outside of this allowed range, adjust the location of the
	 * edge(s) accordingly.
	 */
	if (winLeftEdge < 0) {
		/* If value is negative, set it to 0 */
		winLeftEdge = 0;
	}

	if (winLeftEdge >= textFontWidthX) {
		/* If value is too large, set it to the maximum text width, minus one */
		winLeftEdge = textFontWidthX - 1;
	}

	if (winTopEdge < 0) {
		/* If value is negative, set it to 0 */
		winTopEdge = 0;
	}

	if (winTopEdge >= textFontHeightY) {
		/* If the value is too large, set it to the maximum text height, minus one */
		winTopEdge = textFontHeightY - 1;
	}


	/*
	 * Check that the window's left/top edges plus the desired width/height don't exceed the maximum allowed text width
	 * and text height. If they do exceed this value, reduce the width/height accordingly.
	 */
	if (winLeftEdge + width > textFontWidthX) {
		width = textFontWidthX - winLeftEdge;
	}

	if (winTopEdge + height > textFontHeightY) {
		height = textFontHeightY - winTopEdge;
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
	textWindowPosLeftEdgeX = winLeftEdge;
	textWindowPosTopEdgeY = winTopEdge;
	textWindowWidthInCharColsX = width;
	textWindowHeightInCharRowsY = height;
	textCursorPosColumnX = 0;
	textCursorPosRowY = 0;
}


void DueVideoOut::clearPrintWindow() {
	fillRect(textWindowPosLeftEdgeX * 8, textWindowPosTopEdgeY * 8, (textWindowPosLeftEdgeX + textWindowWidthInCharColsX) * 8 - 1, (textWindowPosTopEdgeY + textWindowHeightInCharRowsY) * 8 - 1, textWindowBackgroundColor);
	textCursorPosColumnX = 0;
	textCursorPosRowY = 0;
}


void DueVideoOut::scrollPrintWindow() {
	if (textWindowWidthInCharColsX == textFontWidthX && textWindowHeightInCharRowsY == textFontHeightY) { // fast version where no text window
		if (_currentVideoOutputMode == VGA_MONO) {
			/* ---( VGA -- monochrome _currentVideoOutputMode )--- */
			uint16_t *a = monoPixelBufPtr;
			uint16_t *b = a + 8 * monoPixelWordsPerLine;
			memmove((uint8_t *) a, (uint8_t *) b, 2 * monoPixelWordsPerLine * (_displayResolutionY - 8));
			for (int i = 0; i < 8; i++) {
				memset((uint8_t * )(a + monoPixelWordsPerLine * (_displayResolutionY - 8 + i)), (textWindowFontColor & 1) ? 0 : 255, 2 * (monoPixelWordsPerLine - 2));
			}
		} else if (_currentVideoOutputMode & VGA_COLOR) {
			/* ---( VGA -- color _currentVideoOutputMode )--- */
			uint8_t *a = cb, *b = cb + 8 * cw;
			memmove(a, b, cw * (_displayResolutionY - 8));
			memset(a + cw * (_displayResolutionY - 8), textWindowBackgroundColor, cw * 8);
		}
	} else {
		scroll(textWindowPosLeftEdgeX * 8, textWindowPosTopEdgeY * 8, textWindowWidthInCharColsX * 8, textWindowHeightInCharRowsY * 8, 0, -8, textWindowBackgroundColor);
	}

	textCursorPosColumnX = 0;
	textCursorPosRowY = textWindowHeightInCharRowsY - 1;
}


size_t DueVideoOut::write(uint8_t c) {
	if (c == 13) {
		textCursorPosColumnX = 0;
		return (1);
	}
	if (c == 10) {
		textCursorPosColumnX = 0;
		textCursorPosRowY++;
		if (textCursorPosRowY == textWindowHeightInCharRowsY) {
			scrollPrintWindow();
		}
		return 1;
	}
	if (c == 9) {
		write(32);
		for (int r = 1; r <= 8; r++) {
			if (textCursorPosColumnX & 7) {
				write(32);
			} else {
				return r;
			}
		}
	}

	if (_currentVideoOutputMode == VGA_MONO) {
		uint8_t *a = (uint8_t *) monoPixelBufPtr + (textWindowPosTopEdgeY + textCursorPosRowY) * 16 * monoPixelWordsPerLine + ((textWindowPosLeftEdgeX + textCursorPosColumnX) ^ 1);
		for (int j = 0; j < 8; j++) {
			*a = _vga_font8x8[8 * c + j] ^ ((textWindowFontColor & 1) ? 0 : 255);
			a += monoPixelWordsPerLine * 2;
		}
	} else if (_currentVideoOutputMode & VGA_COLOR) {
		uint8_t *a = (uint8_t *) cb + (textWindowPosTopEdgeY + textCursorPosRowY) * 8 * cw + (textWindowPosLeftEdgeX + textCursorPosColumnX) * 8;
		for (int j = 0; j < 8; j++) {
			for (int i = 0; i < 8; i++) {
				a[i] = (_vga_font8x8[8 * c + j] & (128 >> i)) ? textWindowFontColor : textWindowBackgroundColor;
			}
			a += cw;
		}
	}

	textCursorPosColumnX++;
	if (textCursorPosColumnX == textWindowWidthInCharColsX) {
		textCursorPosColumnX = 0;
		textCursorPosRowY++;
		if (textCursorPosRowY == textWindowHeightInCharRowsY) {
			scrollPrintWindow();
		}
	}

}


size_t DueVideoOut::write(const uint8_t *buffer, size_t size) {
	int rv = 0;
	for (int i = 0; i < size; i++) {
		rv += write(buffer[i]);
	}
	return rv;
}
