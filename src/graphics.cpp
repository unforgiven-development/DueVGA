/**
 * \file graphics.cpp
 * \brief Contains functions for drawing graphics to the display.
 *
 * Contains functions to draw a variety of graphical primatives to the display, which are able to be combined to produce
 * increasingly complex shapes/graphics.
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
 */




#include "VGA.h"

void Vga::clear(int c) {
	if (videoOutputMode == VGA_MONO) {
		/* ---( VGA -- monochrome videoOutputMode )--- */
		for (int y = 0; y < ysize; y++) {
			memset(pb + y * pw, (c & 1) ? 0xFF : 0, xsize / 8);
		}
	} else if (videoOutputMode & VGA_COLOR) {
		/* ---( VGA -- color videoOutputMode )--- */
		memset(cb, c, cbsize);
	}
}


void Vga::drawPixel(int x, int y, int c) {
	if ((x < 0) || (x >= xsize) || (y < 0) || (y >= ysize)) {
		return;
	}

	if (videoOutputMode == VGA_MONO) {
		/* ---( VGA -- monochrome videoOutputMode )--- */
		if (c >= 0) {
			pbb[y * pbw + (x ^ 15)] = c;
		} else {
			pbb[y * pbw + (x ^ 15)] ^= c;
		}
	} else if (videoOutputMode & VGA_COLOR) {
		/* ---( VGA -- color videoOutputMode )--- */
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


void Vga::drawLine(int x0, int y0, int x1, int y1, int c) {
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


void Vga::drawHLine(int y, int x0, int x1, int col) {
	for (int i = x0; i <= x1; i++) {
		drawPixel(i, y, col);
	}
}


void Vga::drawLinex(int x0, int y0, int x1, int y1, int c) { // Draw line, missing the last point
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


void Vga::drawTri(int x0, int y0, int x1, int y1, int x2, int y2, int col) {
	drawLine(x0, y0, x1, y1, col);
	drawLine(x1, y1, x2, y2, col);
	drawLine(x2, y2, x0, y0, col);
}


#define _V_P(x, y)	if (y > = 0 && y < ysize) {		\
						if (x < xmin[y]) {			\
							xmin[y] = x;			\
						}							\
						if (x > xmax[y]) {			\
							xmax[y] = x;			\
						}							\
					}


void Vga::fillTri(int x0, int y0, int x1, int y1, int x2, int y2, int col) {
	/**
	 * \todo this can be done without needing an array
	 */
	short xmin[ysize];
	short xmax[ysize];

	for (int i = 0; i < ysize; i++) {
		xmin[i] = xsize;
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
	for (int i = 0; i < ysize; i++) {
		if (xmin[i] <= xmax[i]) {
			drawHLine(i, xmin[i], xmax[i], col);
		}
	}
}


void Vga::drawRect(int x0, int y0, int x1, int y1, int col) {
	drawLinex(x0, y0, x0, y1, col);
	drawLinex(x0, y1, x1, y1, col);
	drawLinex(x1, y1, x1, y0, col);
	drawLinex(x1, y0, x0, y0, col);
}


void Vga::fillRect(int x0, int y0, int x1, int y1, int col) {
	int xa = min(min(xsize - 1, x0), x1), xb = max(max(0, x0), x1);
	for (int y = min(min(ysize - 1, y0), y1); y <= max(max(0, y0), y1); y++) {
		drawHLine(y, xa, xb, col);
	}
}


// These circle and ellipse functions taken from
// http://members.chello.at/~easyfilter/bresenham.html
// by Zingl Alois
void Vga::drawCircle(int xm, int ym, int r, int col) {
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


void Vga::fillCircle(int xm, int ym, int r, int col) {
	short xmin[ysize], xmax[ysize];
	for (int i = 0; i < ysize; i++) {
		xmin[i] = xsize;
		xmax[i] = -1;
	}
	int x = -r, y = 0, err = 2 - 2 * r;                /* bottom left to top right */
	do {
		_V_P(xm - x, ym + y);                        /*   I. Quadrant +x +y */
		_V_P(xm - y, ym - x);                        /*  II. Quadrant -x +y */
		_V_P(xm + x, ym - y);                        /* III. Quadrant -x -y */
		_V_P(xm + y, ym + x);                        /*  IV. Quadrant +x -y */
		r = err;
		if (r <= y) {
			err += ++y * 2 + 1;
		}                             /* e_xy+e_y < 0 */
		if (r > x || err > y)                  /* e_xy+e_x > 0 or no 2nd y-step */
			err += ++x * 2 + 1;                                     /* -> x-step now */
	} while (x < 0);
	for (int i = 0; i < ysize; i++) {
		if (xmin[i] <= xmax[i]) {
			drawHLine(i, xmin[i], xmax[i], col);
		}
	}
}


void Vga::drawEllipse(int x0, int y0, int x1, int y1, int col) {                              /* rectangular parameter enclosing the ellipse */
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


void Vga::fillEllipse(int x0, int y0, int x1, int y1, int col) {                              /* rectangular parameter enclosing the ellipse */
	long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;                 /* diameter */
	double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;           /* error increment */
	double err = dx + dy + b1 * a * a, e2;                          /* error of 1.step */
	short xmin[ysize], xmax[ysize];
	for (int i = 0; i < ysize; i++) {
		xmin[i] = xsize;
		xmax[i] = -1;
	}

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
		_V_P(x1, y0);                                  /*   I. Quadrant */
		_V_P(x0, y0);                                  /*  II. Quadrant */
		_V_P(x0, y1);                                  /* III. Quadrant */
		_V_P(x1, y1);                                  /*  IV. Quadrant */
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
		_V_P(x0 - 1, y0);                    /* -> finish tip of ellipse */
		_V_P(x1 + 1, y0++);
		_V_P(x0 - 1, y1);
		_V_P(x1 + 1, y1--);
	}

	for (int i = 0; i < ysize; i++)
		if (xmin[i] <= xmax[i])
			drawHLine(i, xmin[i], xmax[i], col);
}


void Vga::scroll(int x, int y, int w, int h, int dx, int dy, int col) {
	if (videoOutputMode & VGA_COLOR) {
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
	} else if (videoOutputMode == VGA_MONO) {
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
