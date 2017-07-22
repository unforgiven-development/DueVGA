/**
 * \file VGA.h
 * \brief Defines the functionality of the DueVGA Arduino library
 *
 * \version		0.512.1
 *
 *

 *
 *
 * \todo		Implement support for drawing bitmap/pixmap graphics to the screen
 * \todo		Add additional fonts; particularly a \b larger font
 * \todo		Implement our own version of \c printf()
 *
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

/**
 * \page tasks Task List
 * This page is dedicated to holding the task list for the project. Tasks include bugs, enhancements, and anything else
 * that may need done to improve to project's overall quality and functionality.
 *
 * \tableofcontents
 *
 * \section buglist Bugs
 *
 * \subsection knownbugs Known and Confirmed Bugs
 *
 * \subsubsection bugsnoclass Unclassified
 *
 * \bug Sync timing code is incorrect
 * \bug Functions that draw to the screen need optimized for speed
 * \bug Is known to perform poorly in conjunction with the native USB peripheral
 * \bug "White line / missing first pixel in mono modes"
 *
 * \subsubsection bugshighpri High Priority Bugs
 *
 * \subsubsection bugsmedpri Medium Priority Bugs
 *
 * \subsubsection bugslowpri Low Priority Bugs
 *
 * \subsection unconfbugs Unconfirmed Bugs
 *
 * \subsection recentlyresolvedbugs Recently Resolved Bugs
 * This section will be populated with recently resolved bugs. At major version
 * releases, this list will be copied to the \ref prjchangelog "Changelog"; this
 * list will be purged as this occurs.
 *
 *
 */

/**
 * \page prjchangelog Project Changelog
 * As the project's development continues onwards, major revisions and changes will be documented here.
 *
 * \version 0.512.1
 *
 * \tableofcontents
 *
 * \section briefcl Brief Changelog
 * For people who don't care about the details...
 *
 * \li \ref ver0dot512dot1 "Version 0.512.1" - The current \b WIP version (as of 2017-07-05); author: Gerad Munsch
 * \li \ref

// Arduino Due VGA Library by stimmer
// v0.404 (4/4/2013)
//
//
// Added NTSC and PAL color composite modes
// Fixed some timing problems
//


/*

*/


#ifndef _VGA_H__
#define _VGA_H__


#include "Arduino.h"
#include "Print.h"


#define VGA_MONO 1
#define VGA_MONOCHROME VGA_MONO
#define VGA_GRAYSCALE VGA_MONO

#define VGA_COLOR 2
#define VGA_COLOR VGA_COLOR
#define VGA_RGB VGA_COLOR

#define VGA_NTSC 18

#define VGA_PAL 34


extern unsigned char _vga_font8x8 [];


const int _v_vsync = 42;
const int _v_hsync = 43;


inline void _v_digitalWriteDirect(int pin, boolean val) {
	if (val) {
		g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
	} else {
		g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
	}
}



class Vga : public Print {

public:
	/**
	 * Starts the \b DueVGA driver, in \b VGA mode. If a color mode isn't included as a parameter, the driver will start
	 * in \p VGA_MONO mode by default.
	 *
	 * \note The maxiumum resolution for \p VGA_MONO mode is 800x600; and for \p VGA_COLOR mode, 320x240
	 * \todo Add a page about the VGA output, its limitations, etc..; create a link to said page.
	 *
	 * \param[in]	x	Sets the VGA \p x resolution (width) in pixels.
	 * \param[in]	y	Sets the VGA \p y resolution (height) in pixels.
	 * \param[in]	m	\b OPTIONAL - Sets the VGA color mode: \p VGA_MONO (the default), or \p VGA_COLOR
	 */
	int begin(int x, int y, int m = VGA_MONO);

	/**
	 * Starts the \b DueVGA driver, in \b PAL output mode.
	 *
	 * \brief Start video output in PAL mode
	 *
	 * \note PAL output has a maximum resolution of 320x240
	 */
	int beginPAL();

	/**
	 * Starts the \b DueVGA driver, in \b NTSC output mode.
	 *
	 * \brief Start video output in NTSC mode
	 *
	 * \note NTSC output has a maximum resolution of 320x200
	 */
	int beginNTSC();


	void end();



	void clear(int c = 0);


	void drawPixel(int x, int y, int col);


	void drawLine(int x0, int y0, int x1, int y1, int col);


	void drawLinex(int x0, int y0, int x1, int y1, int col);


	void drawHLine(int y, int x0, int x1, int col);


	void drawTri(int x0, int y0, int x1, int y1, int x2, int y2, int col);


	void fillTri(int x0, int y0, int x1, int y1, int x2, int y2, int col);


	void drawRect(int x0, int y0, int x1, int y1, int col);


	void fillRect(int x0, int y0, int x1, int y1, int col);


	void drawCircle(int x, int y, int r, int col);


	void fillCircle(int x, int y, int r, int col);


	void drawEllipse(int x0, int y0, int x1, int y1, int col);


	void fillEllipse(int x0, int y0, int x1, int y1, int col);


	void drawText(char *text, int x, int y, int fgcol, int bgcol = -256, int dir = 0);


	void scroll(int x, int y, int w, int h, int dx, int dy, int col = 0);

	void moveCursor(int column, int line);


	void setPrintWindow(int left, int top, int width, int height);


	void unsetPrintWindow() { tww = tw;twh = th;twx = twy = 0; }


	void clearPrintWindow();


	void scrollPrintWindow();


	void setInk(int i) { ink = i; }


	void setPaper(int p) { paper = p; }


	virtual size_t write(const uint8_t *buffer, size_t size);


	virtual size_t write(uint8_t c);

	void waitBeam() {
		while ((*(volatile int *)&line) < ysize);
	}


	void waitSync() {
		while ((*(volatile int *)&line) >= ysize);
		while ((*(volatile int *)&line) < ysize);
	}


	int up; // whether we are running or not

	// modeline
	int pclock; // must divide 84000000


	int xsize;


	int xsyncstart;


	int xsyncend;


	int xtotal;


	int ysize;


	int ysyncstart;


	int ysyncend;


	int ytotal;


	bool vsyncpol;


	bool hsyncpol;

	//PAL
	const uint16_t *cbt[2];


	const uint16_t *crt[2];


	uint16_t *dmabuf;


	int phase;


	int poff;

	// various display parameters
	int mode;


	int line;


	int linedouble;


	int synced;


	int framecount;


	int xclocks;


	int xstart;


	int xsyncwidth;


	int xscale;


	int yscale;


	int lfreq;


	int ffreq;


	int ltot;


	int debug;


	int lfreqmin;


	int lfreqmax;


	int ffreqmin;


	int ffreqmax;


	void setMonitorFreqRange(int hmin, int hmax, int vmin, int vmax) {
		lfreqmin = hmin;
		lfreqmax = hmax;
		ffreqmin = vmin;
		ffreqmax = vmax;
	}


	void setSyncPolarity(bool h, bool v) {
		hsyncpol = h;
		vsyncpol = v;
	}


	uint16_t *pb;		// Pixel buffer memory address


	int pw;				// Count of words from one line to the next (aka stride or pitch)


	int pbsize;			// Total size of pixel buffer (note these sizes are 16-bit words)


	uint32_t *pbb;		// Pixel buffer bit-banding alias address (read the datasheet p75)


	int pbw;			// Pixel buffer bit-banding stride (in 32-bit words)

	// To help understand usage of these, look at the following functions:


	void putPPixelFast(int x, int y, int c) {
		pbb[y * pbw + (x ^ 15)] = c;
	}



	int getPPixelFast(int x, int y) {
		return pbb[y * pbw + (x ^ 15)];
	}


	uint8_t *cb;		// Color buffer memory address


	int cw;				// Color buffer stride, in bytes


	int cbsize;			// Size of color buffer in bytes



	void putCPixelFast(int x, int y, int c) {
		cb[y * cw + x] = c;
	}



	int getCPixelFast(int x, int y) {
		return cb[y * cw + x];
	}


	/**
	 * \defgroup textwindows Text Windows
	 * This group collects information about important functions and variables used the "Text Windows" feature. Also, to
	 * ease discovery of information in the documentation, some functions/variables may be included in multiple/various
	 * "sub-groups".
	 */

	/**
	 * \defgroup textwinvars Text Windows - Variables
	 * \ingroup textwindows
	 * Variables used by the "Text Windows" feature.
	 *
	 * @{
	 */

	/**
	 * The text window cursor's position.
	 * \var tx The text cursor's position (column)
	 * \var ty The text cursor's position (row)
	 */
	int tx;
	int ty;

	/**
	 * The text window cursor's position.
	 * \var
	 * \var
	 */
	int tw;
	int th;				// Text width / height

	/**
	 * The text window cursor's position.
	 * \var
	 * \var
	 * \var
	 * \var
	 */
	int twx;
	int twy;
	int tww;
	int twh;			// Text window

	/**
	 * The text window cursor's position.
	 * \var
	 * \var
	 */
	int ink;
	int paper;			// Text colors

	/**
	*/


private:
	int calcmodeline();
	int allocvideomem();
	void freevideomem();
	void startinterrupts();
	void stopinterrupts();
	void starttimers();
	void stoptimers();
	void startmono();
	void stopmono();
	void startcolor();
	void stopcolor();
	void dmapri();

};

extern Vga VGA;


#endif
