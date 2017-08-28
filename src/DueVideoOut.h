/**
 * \file VGA.h
 * Defines the functionality of the DueVGA Arduino library
 *
 * \version		0.512.1
 *
 * \todo Implement support for drawing bitmap/pixmap graphics to the screen
 * \todo Add additional fonts; particularly a \b larger font
 * \todo Implement our own version of \c printf()
 *
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
 * \li \ref ver0dot512dot1 "Version 0.512.1" - The current \b WIP version (as of 2017-07-05); \b author: Gerad Munsch
 * \li \ref ver0dot404 "Version 0.404"  - Added \b NTSC and \b PAL composite video output, and fixed some timing issues.
 *     (2013-04-04); \b author: stimmer
 */

// Arduino Due VGA Library by stimmer
// v0.404 (4/4/2013)
//
//
// Added NTSC and PAL color composite modes
// Fixed some timing problems
//





#ifndef _VGA_H__
#define _VGA_H__


#include "Arduino.h"
#include "Print.h"


//#define VGA_MONO 1
//#define VGA_MONOCHROME VGA_MONO
//#define VGA_GRAYSCALE VGA_MONO

//#define VGA_COLOR 2
//#define VGA_COLOUR VGA_COLOR
//#define VGA_RGB VGA_COLOR

//#define VGA_NTSC 18

//#define VGA_PAL 34


/**
 * Provides an enumerated type defining the various video output modes which are supported by \b DueVGA
 *
 * \brief Enum defining video output modes
 */
typedef enum {
	VGA_MONO		= 1,	/*!< Defines \b VGA output in \b MONOCHROME mode */
	VGA_MONOCHROME	= 1,	/*!< Defines \b VGA output in \b MONOCHROME mode */
	VGA_GRAYSCALE	= 1,	/*!< Defines \b VGA output in \b MONOCHROME mode */
	VGA_COLOR		= 2,	/*!< Defines \b VGA output in \b COLOR mode */
	VGA_COLOUR		= 2,	/*!< Defines \b VGA output in \b COLOR mode */
	VGA_RGB			= 2,	/*!< Defines \b VGA output in \b COLOR mode */
	VGA_NTSC		= 18,	/*!< Defines \b NTSC composite video output */
	TV_NTSC			= 18,	/*!< Defines \b NTSC composite video output */
	COMPOSITE_NTSC	= 18,	/*!< Defines \b NTSC composite video output */
	VGA_PAL			= 34,	/*!< Defines \b PAL composite video output */
	TV_PAL			= 34,	/*!< Defines \b PAL composite video output */
	COMPOSITE_PAL	= 34,	/*!< Defines \b PAL composite video output */
	NO_VIDEO_OUT	= 0		/*!< Defines that there is \b no video output */
} video_output_mode_t;


extern unsigned char _vga_font8x8 [];


const int _video_output_vsync_pin = 42;		/*! Defines the pin number used for the video output \b VSYNC signal */
const int _video_output_hsync_pin = 43;		/*! Defines the pin number used for the video output \b HSYNC signal */

/**
 * This function provides an optimized, highly-efficient method to toggle the state of a GPIO in \e OUTPUT mode, via the
 * use of an \b inline function, that writes the desired state directly to the port's register.
 *
 * \brief An optimized routine for toggling the state of a digital \e OUTPUT pin
 *
 * \param[in]	pin	The pin number on which to act
 * \param[in]	val	The desired state of the pin:\n
 *					\li \b 0 or \b false - turn the output off (ie: to the \e LOW state)
 *					\li \b 1 or \b true - turn the output on (ie: to the \e HIGH state)
 */
inline void _v_digitalWriteDirect(int pin, boolean val) {
	if (val) {
		g_APinDescription[pin].pPort -> PIO_SODR = g_APinDescription[pin].ulPin;
	} else {
		g_APinDescription[pin].pPort -> PIO_CODR = g_APinDescription[pin].ulPin;
	}
}


/**
 * \class Vga
 * This class provides the implementation of the display driver for the Arduino Due hardware. There are a variety of
 * output methods available.
 *
 * \brief Implements the display driver for the Arduino Due
 */
class DueVideoOut : public Print {
public:
	/**
	 * Starts the \b DueVGA driver, in \b VGA mode. If one of the color modes (ie: the \p VGA_COLOR enumerated value) is
	 * not included as a parameter, the driver will start in \p VGA_MONO mode by default.
	 *
	 * \brief Starts the video output driver in VGA mode, in either mono or color mode
	 *
	 * \note The maxiumum \e practical resolution for \p VGA_MONO mode is 800x600; for \p VGA_COLOR mode, 320x240
	 * \warning The absolute \b maximum Y-resolution in \p VGA_COLOR mode is 380; anything greater will fail.
	 * \todo Add a page about the VGA output, its limitations, etc..; create a link to said page.
	 *
	 * \todo Provide a unified \c begin() function
	 *
	 * \param[in]	targetResX		Sets the desired target VGA \b X resolution (width) in pixels.
	 * \param[in]	targetResY		Sets the desired target VGA \b Y resolution (height) in pixels.
	 * \param[in]	targetVideoMode	\b OPTIONAL -- Sets the VGA output mode:\n
	 *								\li \p VGA_MONO (\e default)
	 *								\li \p VGA_COLOR
	 */
	int begin(int targetResX, int targetResY, video_output_mode_t targetVideoMode = VGA_MONO);

	/**
	 * Starts the \b DueVGA driver, in \b PAL output mode.
	 *
	 * \brief Start video output in PAL mode
	 *
	 * \note PAL output has a preset (non-configurable) resolution of 320x240
	 *
	 * \todo Provide a unified \c begin() function
	 */
	int beginPAL();

	/**
	 * Starts the \b DueVGA driver, in \b NTSC output mode.
	 *
	 * \brief Start video output in NTSC mode
	 *
	 * \note NTSC output has a preset (non-configurable) resolution of 320x200
	 *
	 * \todo Provide a unified \c begin() function
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


	void moveCursor(int destColumn, int destLine);


	void setPrintWindow(int winLeftEdge, int winTopEdge, int width, int height);


	void unsetPrintWindow() {
		textWindowWidthInCharColsX = textFontWidthX;
		textWindowHeightInCharRowsY = textFontHeightY;
		textWindowPosLeftEdgeX = 0;
		textWindowPosTopEdgeY = 0;
	}


	void clearPrintWindow();


	void scrollPrintWindow();


	void setInk(int i) {
		textWindowFontColor = i;
	}


	void setPaper(int p) {
		textWindowBackgroundColor = p;
	}


	virtual size_t write(const uint8_t *buffer, size_t size);


	virtual size_t write(uint8_t c);


	void waitBeam() {
		while ((*(volatile int *)&line) < _displayResolutionY);
	}


	void waitSync() {
		while ((*(volatile int *)&line) >= _displayResolutionY);
		while ((*(volatile int *)&line) < _displayResolutionY);
	}




	// modeline
	int pclock; // must divide 84000000


	int xsyncstart;


	int xsyncend;


	int xtotal;


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


	/**
	 * \name VGAMonoPixelData
	 * Variables and methods which are used by the \p VGA_MONO video output mode
	 */

	/**
	 * @{
	 */

	/**
	 * A pointer to the pixel buffer's memory address. The pixel buffer is comprised of an array of 16-bit unsigned
	 * integer values, the size of which will be stored in \p pbsize
	 *
	 * \brief Pixel buffer memory address
	 */
	uint16_t *monoPixelBufPtr;

	/**
	 * The amount of \e words (16-bit values) per output line.
	 * \note An extra 2 words are added as \e spare words, which are used for blanking. The data stored in these 2 extra
	 *       words \b must always be \b 0x0000
	 *
	 *  \brief Qty of \e words per line
	 */
	int monoPixelWordsPerLine;

	/**
	 * The size of the pixel buffer.
	 * \note The sizes are 16-bit words.
	 *
	 * \brief Total size of pixel buffer
	 */
	int monoPixelBufferSizeInWords;

	/**
	 * Pointer to the monochrome pixel buffer's bit-banding alias address.
	 *
	 * \note For technical details, see page 66-68 in the \b SAM3X8E datasheet
	 */
	uint32_t *monoPixelBufferBitBandingAliasAddrPtr;

	/**
	 * The monochrome pixel buffer's bit-banding \e stride (in 32-bit "double words")
	 *
	 * \note The value of this indicates the amount of pixels per line.
	 * \todo Verify the accuracy of the note for this variable.
	 */
	int monoPixelBufferBitBandingStride;

	/* To help understand usage of these, look at the following functions: */


	void putPPixelFast(int x, int y, int c) {
		monoPixelBufferBitBandingAliasAddrPtr[y * monoPixelBufferBitBandingStride + (x ^ 15)] = c;
	}



	int getPPixelFast(int x, int y) {
		return monoPixelBufferBitBandingAliasAddrPtr[y * monoPixelBufferBitBandingStride + (x ^ 15)];
	}


	/**
	 * @}
	 */


	/**
	 * \name VGAColorPixelData
	 * Variables and methods which are used by the \p VGA_COLOR video output mode
	 */

	/**
	 * @{
	 */


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
	 * @}
	 */

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

	int textCursorPosColumnX;				/*! The text cursor's position (X-axis / "column") */
	int textCursorPosRowY;				/*! The text cursor's position (Y-axis / "row") */

	int textFontWidthX;				/*! Text width (X-axis / "columns") */
	int textFontHeightY;				/*! Text height (Y-axis / "rows") */

	int textWindowPosLeftEdgeX;			/*! The text window's position (X-axis / left edge of window) */
	int textWindowPosTopEdgeY;			/*! The text window's position (Y-axis / top edge of window) */
	int textWindowWidthInCharColsX;			/*! The text window's width in columns (1 column = 8 pixels) */
	int textWindowHeightInCharRowsY;			/*! The text window's height in rows (1 row = 8 pixels) */

	int textWindowFontColor;			/*! The color of the text within the text window */
	int textWindowBackgroundColor;			/*! The color of the background of the text window */

	/**
	 * @}
	*/

protected:

	/**
	 * The X-axis display resolution in pixels. Allowed values vary by video output mode.
	 *
	 * \brief The X-axis resolution
	 */
	int _displayResoultionX;


	/**
	 * The Y-axis display resolution in pixels. Allowed values vary by video output mode.
	 *
	 * \brief The Y-axis resolution
	 */
	int _displayResolutionY;

private:
	/**
	 * This variable holds the operational state of the driver; this indicates whether the driver is currently running,
	 * or if it is idle.
	 *
	 * \brief The operational state of the driver
	 */
	static uint8_t _isDriverRunning;

	/**
	 * This variable holds the current video output mode which the driver is currently using to display video output.
	 * The value is stored as a type of the enumeration \c video_output_mode_t -- allowing for relatively easy lookup or
	 * comparison in code structures.
	 *
	 * \brief The current video output mode
	 */
	static video_output_mode_t _currentVideoOutputMode;


	/**
	 * Attempt to calculate a valid modeline for video output.
	 *
	 * \return Indicates whether a modeline was able to be calculated for the given configuration.
	 * \retval 0	The operation was successful.
	 * \retval -1	The subroutine was unable to calculate a valid modeline.
	 */
	int calculateValidModeline();



	int allocateVideoMemory();


	void freeAllocatedVideoMemory();


	void startVideoDriverInterrupts();


	void stopVideoDriverInterrupts();


	void startTimers();


	void stopTimers();


	void startVgaOutputMono();


	void stopVgaOutputMono();


	void startVgaOutputColor();


	void stopVgaOutputColor();


	void reconfigureDmaPriority();

};

extern DueVideoOut VGA;


#endif
