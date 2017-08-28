/**
 * \file graphics.cpp
 * Contains functions to draw a variety of graphical primatives to the display, which are able to be combined to produce
 * increasingly complex shapes/graphics.
 *
 * \brief Contains functions for drawing graphics to the display.
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



void DueVideoOut::clear(int c) {
	if (_currentVideoOutputMode == VGA_MONO) {
		/* ---( VGA -- monochrome output mode )--- */
		for (int y = 0; y < _displayResolutionY; y++) {
			memset(monoPixelBufPtr + y * monoPixelWordsPerLine, (c & 1) ? 0xFF : 0, _displayResoultionX / 8);
		}
	} else if (_currentVideoOutputMode & VGA_COLOR) {
		/* ---( VGA -- color output mode )--- */
		memset(cb, c, cbsize);
	}
}


void DueVideoOut::drawPixel(int x, int y, int c) {
	if ((x < 0) || (x >= _displayResoultionX) || (y < 0) || (y >= _displayResolutionY)) {
		return;
	}

	if (_currentVideoOutputMode == VGA_MONO) {
		/* ---( VGA -- monochrome output mode )--- */
		if (c >= 0) {
			monoPixelBufferBitBandingAliasAddrPtr[y * monoPixelBufferBitBandingStride + (x ^ 15)] = c;
		} else {
			monoPixelBufferBitBandingAliasAddrPtr[y * monoPixelBufferBitBandingStride + (x ^ 15)] ^= c;
		}
	} else if (_currentVideoOutputMode & VGA_COLOR) {
		/* ---( VGA -- color output mode )--- */
		if (c >= 0) {
			cb[y * cw + x] = c;
		} else {
			cb[y * cw + x] ^= -c;
		}
	}
}


template<typename T> int _v_sgn(T val) {
	return (T(0) < val) - (val < T(0));
}


void DueVideoOut::drawLine(int x0, int y0, int x1, int y1, int c) {
	int dx = abs(x1 - x0), dy = abs(y1 - y0), sx = _v_sgn(x1 - x0), sy = _v_sgn(y1 - y0);
	int err = dx - dy;
	if ((x0 != x1) || (y0 != y1)) {
		drawPixel(x1, y1, c);
	}
	do {
		drawPixel(x0, y0, c);
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	} while ((x0 != x1) || (y0 != y1));
}


void DueVideoOut::drawHLine(int y, int x0, int x1, int col) {
	for (int i = x0; i <= x1; i++) {
		drawPixel(i, y, col);
	}
}


void DueVideoOut::drawLinex(int x0, int y0, int x1, int y1, int c) { // Draw line, missing the last point
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = _v_sgn(x1 - x0);
	int sy = _v_sgn(y1 - y0);

	int err = dx - dy;

	while ((x0 != x1) || (y0 != y1)) {
		drawPixel(x0, y0, c);
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}


void DueVideoOut::drawTri(int x0, int y0, int x1, int y1, int x2, int y2, int col) {
	drawLine(x0, y0, x1, y1, col);
	drawLine(x1, y1, x2, y2, col);
	drawLine(x2, y2, x0, y0, col);
}


#define _V_P(x, y)	if (y > = 0 && y < _displayResolutionY) {		\
						if (x < xmin[y]) {			\
							xmin[y] = x;			\
						}							\
						if (x > xmax[y]) {			\
							xmax[y] = x;			\
						}							\
					}


void DueVideoOut::fillTri(int x0, int y0, int x1, int y1, int x2, int y2, int col) {
	/**
	 * \todo this can be done without needing an array
	 */
	short xmin[_displayResolutionY];
	short xmax[_displayResolutionY];

	for (int i = 0; i < _displayResolutionY; i++) {
		xmin[i] = _displayResoultionX;
		xmax[i] = -1;
	}

	int dx, dy, sx, sy, err, x3 = x0, y3 = y0;
	dx = abs(x1 - x0);
	dy = abs(y1 - y0);
	sx = _v_sgn(x1 - x0);
	sy = _v_sgn(y1 - y0);
	err = dx - dy;
	while ((x0 != x1) || (y0 != y1)) {
		_V_P(x0, y0)
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	sx = _v_sgn(x2 - x1);
	sy = _v_sgn(y2 - y1);
	err = dx - dy;
	while ((x1 != x2) || (y1 != y2)) {
		_V_P(x1, y1)
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y1 += sy;
		}
	}
	dx = abs(x3 - x2);
	dy = abs(y3 - y2);
	sx = _v_sgn(x3 - x2);
	sy = _v_sgn(y3 - y2);
	err = dx - dy;
	while ((x2 != x3) || (y2 != y3)) {
		_V_P(x2, y2)
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x2 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y2 += sy;
		}
	}
	for (int i = 0; i < _displayResolutionY; i++) {
		if (xmin[i] <= xmax[i]) {
			drawHLine(i, xmin[i], xmax[i], col);
		}
	}
}


void DueVideoOut::drawRect(int x0, int y0, int x1, int y1, int col) {
	drawLinex(x0, y0, x0, y1, col);
	drawLinex(x0, y1, x1, y1, col);
	drawLinex(x1, y1, x1, y0, col);
	drawLinex(x1, y0, x0, y0, col);
}


void DueVideoOut::fillRect(int x0, int y0, int x1, int y1, int col) {
	int xa = min(min(_displayResoultionX - 1, x0), x1), xb = max(max(0, x0), x1);
	for (int y = min(min(_displayResolutionY - 1, y0), y1); y <= max(max(0, y0), y1); y++) {
		drawHLine(y, xa, xb, col);
	}
}


// These circle and ellipse functions taken from
// http://members.chello.at/~easyfilter/bresenham.html
// by Zingl Alois
void DueVideoOut::drawCircle(int xm, int ym, int r, int col) {
	int x = -r, y = 0, err = 2 - 2 * r;                /* bottom left to top right */
	do {
		drawPixel(xm - x, ym + y, col);                        /*   I. Quadrant +x +y */
		drawPixel(xm - y, ym - x, col);                        /*  II. Quadrant -x +y */
		drawPixel(xm + x, ym - y, col);                        /* III. Quadrant -x -y */
		drawPixel(xm + y, ym + x, col);                        /*  IV. Quadrant +x -y */
		r = err;
		if (r <= y) {
			err += ++y * 2 + 1;
		}                             /* e_xy+e_y < 0 */
		if (r > x || err > y)                  /* e_xy+e_x > 0 or no 2nd y-step */
			err += ++x * 2 + 1;                                     /* -> x-step now */
	} while (x < 0);
}


void DueVideoOut::fillCircle(int xm, int ym, int r, int col) {
	short xmin[_displayResolutionY];
	short xmax[_displayResolutionY];

	for (int i = 0; i < _displayResolutionY; i++) {
		xmin[i] = _displayResoultionX;
		xmax[i] = -1;
	}

	/* Bottom left to top right */
	int x = -r;
	int y = 0;
	int err = 2 - 2 * r;

	do {
		_V_P(xm - x, ym + y);						/*   I. Quadrant +x +y */
		_V_P(xm - y, ym - x);						/*  II. Quadrant -x +y */
		_V_P(xm + x, ym - y);						/* III. Quadrant -x -y */
		_V_P(xm + y, ym + x);						/*  IV. Quadrant +x -y */
		r = err;

		/* e_xy + e_y < 0 */
		if (r <= y) {
			err += ++y * 2 + 1;
		}

		/* e_xy + e_x > 0 or no 2nd Y-step */
		if (r > x || err > y) {
			err += ++x * 2 + 1;						/* -> x-step now */
		}
	} while (x < 0);

	for (int i = 0; i < _displayResolutionY; i++) {
		if (xmin[i] <= xmax[i]) {
			drawHLine(i, xmin[i], xmax[i], col);
		}
	}
}


void DueVideoOut::drawEllipse(int x0, int y0, int x1, int y1, int col) {                              /* rectangular parameter enclosing the ellipse */
	long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;                 /* diameter */
	double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;           /* error increment */
	double err = dx + dy + b1 * a * a, e2;                          /* error of 1.step */

	if (x0 > x1) {
		x0 = x1;
		x1 += a;
	}        /* if called with swapped points */
	if (y0 > y1) {
		y0 = y1;
	}                                  /* .. exchange them */
	y0 += (b + 1) / 2;
	y1 = y0 - b1;                               /* starting pixel */
	a = 8 * a * a;
	b1 = 8 * b * b;

	do {
		drawPixel(x1, y0, col);                                  /*   I. Quadrant */
		drawPixel(x0, y0, col);                                  /*  II. Quadrant */
		drawPixel(x0, y1, col);                                  /* III. Quadrant */
		drawPixel(x1, y1, col);                                  /*  IV. Quadrant */
		e2 = 2 * err;
		if (e2 <= dy) {
			y0++;
			y1--;
			err += dy += a;
		}                 /* y step */
		if (e2 >= dx || 2 * err > dy) {
			x0++;
			x1--;
			err += dx += b1;
		}  /* x step */
	} while (x0 <= x1);

	while (y0 - y1 <= b) {                /* too early stop of flat ellipses a=1 */
		drawPixel(x0 - 1, y0, col);                    /* -> finish tip of ellipse */
		drawPixel(x1 + 1, y0++, col);
		drawPixel(x0 - 1, y1, col);
		drawPixel(x1 + 1, y1--, col);
	}
}


void DueVideoOut::fillEllipse(int x0, int y0, int x1, int y1, int col) {                              /* rectangular parameter enclosing the ellipse */
	/* Ellipse dimensions ("diameter") */
	long a = abs(x1 - x0);
	long b = abs(y1 - y0);
	long b1 = b & 1;

	/* Error increment */
	double dx = 4 * (1.0 - a) * b * b;
	double dy = 4 * (b1 + 1) * a * a;

	/* Error of 1 step */
	double err = dx + dy + b1 * a * a;
	double e2;

	short xmin[_displayResolutionY];
	short xmax[_displayResolutionY];


	for (int i = 0; i < _displayResolutionY; i++) {
		xmin[i] = _displayResoultionX;
		xmax[i] = -1;
	}

	/* if called with swapped points... */
	if (x0 > x1) {
		x0 = x1;
		x1 += a;
	}

	/* ...then exchange them */
	if (y0 > y1) {
		y0 = y1;
	}


	y0 += (b + 1) / 2;
	y1 = y0 - b1;                               /* starting pixel */
	a = 8 * a * a;
	b1 = 8 * b * b;


	do {
		_V_P(x1, y0);						/*   I. Quadrant */
		_V_P(x0, y0);						/*  II. Quadrant */
		_V_P(x0, y1);						/* III. Quadrant */
		_V_P(x1, y1);						/*  IV. Quadrant */
		e2 = 2 * err;

		/* Y step */
		if (e2 <= dy) {
			y0++;
			y1--;
			err += dy += a;
		}

		/* X step */
		if (e2 >= dx || 2 * err > dy) {
			x0++;
			x1--;
			err += dx += b1;
		}
	} while (x0 <= x1);

	/* Too early stop of flat ellipses (a = 1) */
	while (y0 - y1 <= b) {
		_V_P(x0 - 1, y0);					/* -> finish tip of ellipse */
		_V_P(x1 + 1, y0++);
		_V_P(x0 - 1, y1);
		_V_P(x1 + 1, y1--);
	}

	for (int i = 0; i < _displayResolutionY; i++) {
		if (xmin[i] <= xmax[i]) {
			drawHLine(i, xmin[i], xmax[i], col);
		}
	}
}


void DueVideoOut::scroll(int x, int y, int w, int h, int dx, int dy, int col) {
	if (_currentVideoOutputMode & VGA_COLOR) {
		if (dy <= 0) {
			if (dx <= 0) {
				for (int i = x; i < x + w + dx; i++) {
					for (int j = y; j < y + h + dy; j++) {
						putCPixelFast(i, j, getCPixelFast(i - dx, j - dy));
					}
				}
			} else {
				for (int i = x + w - 1; i >= x + dx; i--) {
					for (int j = y; j < y + h + dy; j++) {
						putCPixelFast(i, j, getCPixelFast(i - dx, j - dy));
					}
				}
			}
		} else {
			if (dx <= 0) {
				for (int i = x; i < x + w + dx; i++) {
					for (int j = y + h - 1; j >= y + dy; j--) {
						putCPixelFast(i, j, getCPixelFast(i - dx, j - dy));
					}
				}
			} else {
				for (int i = x + w - 1; i >= x + dx; i--) {
					for (int j = y + h - 1; j >= y + dy; j--) {
						putCPixelFast(i, j, getCPixelFast(i - dx, j - dy));
					}
				}
			}
		}
	} else if (_currentVideoOutputMode == VGA_MONO) {
		if (dy <= 0) {
			if (dx <= 0) {
				for (int i = x; i < x + w + dx; i++) {
					for (int j = y; j < y + h + dy; j++) {
						putPPixelFast(i, j, getPPixelFast(i - dx, j - dy));
					}
				}
			} else {
				for (int i = x + w - 1; i >= x + dx; i--) {
					for (int j = y; j < y + h + dy; j++) {
						putPPixelFast(i, j, getPPixelFast(i - dx, j - dy));
					}
				}
			}
		} else {
			if (dx <= 0) {
				for (int i = x; i < x + w + dx; i++) {
					for (int j = y + h - 1; j >= y + dy; j--) {
						putPPixelFast(i, j, getPPixelFast(i - dx, j - dy));
					}
				}
			} else {
				for (int i = x + w - 1; i >= x + dx; i--) {
					for (int j = y + h - 1; j >= y + dy; j--) {
						putPPixelFast(i, j, getPPixelFast(i - dx, j - dy));
					}
				}
			}
		}
	}
	if (col == -256) {
		return;
	}
	if (dy > 0) {
		fillRect(x, y, x + w - 1, y + dy - 1, col);
	} else if (dy < 0) {
		fillRect(x, y + h + dy, x + w - 1, y + h - 1, col);
	}
	if (dx > 0) {
		fillRect(x, y, x + dx - 1, y + h - 1, col);
	} else if (dx < 0) {
		fillRect(x + w + dx, y, x + w - 1, y + h - 1, col);
	}
}
