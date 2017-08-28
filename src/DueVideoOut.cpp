/**
 * \file VGA.cpp
 * This file implements the low-level functionality for generation of VGA and composite-video waveforms, and includes a
 * signficant amount of inline assembly code, as well as direct register access and manipulation.
 *
 * \warning Working on this file is NOT for the faint of heart.
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
#include "crpal.h"
#include "crntsc.h"



DueVideoOut VGA;





void __attribute__((aligned(64))) TC4_Handler() {
	static video_output_mode_t disp = NO_VIDEO_OUT;
	long dummy =	REG_TC1_SR1;
	int c =			REG_PWM_CCNT2;

	if (!VGA.synced) {
		if ((c <= VGA.xstart + 1) && (c >= VGA.xstart)) {
			REG_PWM_CPRDUPD2 = VGA.xclocks;
			VGA.synced = 1;
		}
	}

	if (disp == VGA_COLOR) {
		REG_DMAC_CTRLA5	= 0x22060000 + (VGA.cw >> 2);
		REG_DMAC_CHER	= 1 << 5;
	}

	if (disp == VGA_MONO) {
		REG_DMAC_CTRLA4	= 0x12030000 + (VGA.monoPixelWordsPerLine >> 1);
		REG_DMAC_CHER	= 1 << 4;

		asm volatile(".rept 10\n\t nop\n\t .endr\n\t");

		REG_PIOA_PDR	= 1 << 26;
	}

	if (VGA._currentVideoOutputMode == VGA_PAL) {
		int p;
		static uint16_t *buf;

		REG_DMAC_SADDR5	= (int)buf;
		REG_DMAC_CTRLA5	= 0x22060000 + 223;
		REG_DMAC_CHER	= 1 << 5;

		int oe = VGA.line & 1;
		buf = (uint16_t*)(((int)VGA.dmabuf) + (oe * 1024));

		asm volatile(
			".rept 18                   \n\t"
			" ldrh r0, [%[cbt]], #2     \n\t"
			" strh r0, [%[dbo]], #2     \n\t"
			".endr                      \n\t"
			:
			:[dbo] " r " (buf + 41), [cbt] " r " (&VGA.cbt[oe][VGA.phase + 11])
			:"r0"
		);

		if (VGA.line < VGA._displayResolutionY) {
			p = VGA.phase + 6;
			if (p >= 30) {
				p -= 30;
			}

			asm volatile(
				" mov r0, #15360                     \n\t"
				" mov r1, #512                       \n\t"

				"1:                                  \n\t"

				".rept 8                             \n\t"
				" ldrb r2, [%[cbl]], #1              \n\t"
				" ldrh r3, [%[crt], r2, lsl #1]      \n\t"
				" strh r3, [%[dbo]], #2              \n\t"
				" adds %[crt], r1                    \n\t"
				".endr                               \n\t"

				" cmp %[crt], %[cre]                 \n\t"
				" it gt                              \n\t"
				" subgt %[crt], r0                   \n\t"
				".rept 8                             \n\t"
				" ldrb r2, [%[cbl]], #1              \n\t"
				" ldrh r3, [%[crt], r2, lsl #1]      \n\t"
				" strh r3, [%[dbo]], #2              \n\t"
				" adds %[crt], r1                    \n\t"
				".endr                               \n\t"

				" cmp %[crt], %[cre]                 \n\t"
				" it gt                              \n\t"
				" subgt %[crt], r0                   \n\t"
				" cmp %[cbl], %[cbe]                 \n\t"
				" bne 1b                             \n\t"

				:
				:[dbo] " r " (buf + 96)
				,[cbl] " r " (VGA.cb + VGA.line * 320)
				,[cbe] " r " (VGA.cb + VGA.line * 320 + 320)
				,[crt] " r " (VGA.crt[oe] + p * 256)
				,[cre] " r " (VGA.crt[oe] + 30 * 256- 1)
				:"r0", "r1", "r2", "r3"
			);

		} else if ((VGA.line == VGA._displayResolutionY) || (VGA.line == VGA._displayResolutionY + 1)) {
			uint32_t *lp = (uint32_t*)(buf + 96);
			for (int i = 0; i < 160; i++) {
				*lp++ = 0x3C3C3C3C;
			}
		} else if ((VGA.line == VGA.ysyncstart) || (VGA.line == VGA.ysyncstart + 1)) {
			uint32_t *lp = (uint32_t*)buf;
			for (int i = 0; i < 16; i++) {
				*lp++ = 0x3C3C3C3C;
			}
			for (int i = 16; i < 223; i++) {
				*lp++ = 0;
			}
		} else if ((VGA.line == VGA.ysyncend) || (VGA.line == VGA.ysyncend + 1)) {
			uint32_t *lp = (uint32_t*)buf;
			for (int i = 0; i < 16; i++) {
				*lp++ = 0;
			}
			for (int i = 16; i < 223; i++) {
				*lp++ = 0x3C3C3C3C;
			}
		}
		VGA.phase += VGA.poff;
		if (VGA.phase >= 30) {
			VGA.phase -= 30;
		}
		VGA.line++;
		if (VGA.line == VGA.ytotal) {
			VGA.line = 0;
		}
		return;


	} else if (VGA._currentVideoOutputMode == VGA_NTSC) {
		int p;
		static uint16_t *buf;

		REG_DMAC_SADDR5	= (int)buf;
		REG_DMAC_CTRLA5	= 0x22060000 + 221;
		REG_DMAC_CHER	= 1 << 5;

		buf = (uint16_t*)(((int)VGA.dmabuf) + ((VGA.line & 1) * 1024));

		asm volatile(
			".rept 18               \n\t"
			" ldrh r0, [%[cbt]], #2 \n\t"
			" strh r0, [%[dbo]], #2 \n\t"
			".endr                  \n\t"
			:
			:[dbo] " r " (buf + 41), [cbt] " r " (&VGA.cbt[0][VGA.phase + 41])
			:"r0"
		);

		if (VGA.line < VGA._displayResolutionY) {
			p = VGA.phase + 88;

			if (p >= 88) {
				p -= 88;
			}

			asm volatile(
				" mov r0, #45056                    \n\t"
				" mov r1, #512                      \n\t"

				"1:                                 \n\t"

				".rept 8                            \n\t"
				" ldrb r2, [%[cbl]], #1             \n\t"
				" ldrh r3, [%[crt], r2, lsl #1]     \n\t"
				" strh r3, [%[dbo]], #2             \n\t"
				" adds %[crt] ,r1                   \n\t"
				".endr                              \n\t"

				" cmp %[crt], %[cre]                \n\t"
				" it gt                             \n\t"
				" subgt %[crt] ,r0                  \n\t"

				".rept 8                            \n\t"
				" ldrb r2, [%[cbl]], #1             \n\t"
				" ldrh r3, [%[crt], r2, lsl #1]     \n\t"
				" strh r3, [%[dbo]], #2             \n\t"
				" adds %[crt] ,r1                   \n\t"
				".endr                              \n\t"

				" cmp %[crt] ,%[cre]                \n\t"
				" it gt                             \n\t"
				" subgt %[crt] ,r0                  \n\t"

				" cmp %[cbl], %[cbe]                \n\t"
				" bne 1b                            \n\t"

				:
				:[dbo] " r " (buf + 88), [cbl] " r " (VGA.cb + VGA.line * 320), [cbe] " r " (VGA.cb + VGA.line * 320 + 320), [crt] " r " (VGA.crt[0] + p * 256), [cre] " r " (VGA.crt[0] + 88 * 256 - 1)
				:"r0", "r1", "r2", "r3"
			);

		} else if ((VGA.line == VGA._displayResolutionY) || (VGA.line == VGA._displayResolutionY + 1)) {
			uint32_t *lp = (uint32_t*)(buf + 88);
			for (int i = 0; i < 160; i++) {
				*lp++ = 0x3C3C3C3C;
			}
		} else if ((VGA.line == VGA.ysyncstart) || (VGA.line == VGA.ysyncstart + 1)) {
			uint32_t *lp = (uint32_t*)buf;
			for (int i = 0; i < 16; i++) {
				*lp++ = 0x3C3C3C3C;
			}
			for (int i = 16; i < 222; i++) {
				*lp++ = 0;
			}
		} else if ((VGA.line == VGA.ysyncend) || (VGA.line == VGA.ysyncend + 1)) {
			uint32_t *lp = (uint32_t*)buf;
			for (int i = 0; i < 16; i++) {
				*lp++ = 0;
			}
			for (int i = 16; i < 222; i++) {
				*lp++ = 0x3C3C3C3C;
			}
		}

		VGA.phase += VGA.poff;
		if (VGA.phase >= 88) {
			VGA.phase -= 88;
		}

		VGA.line++;

		if (VGA.line == VGA.ytotal) {
			VGA.line = 0;
		}


		return;
	}

	if (VGA.line == VGA.ysyncstart) {
		_v_digitalWriteDirect(_video_output_vsync_pin, VGA.vsyncpol);
	}

	if (VGA.line == VGA.ysyncend) {
		_v_digitalWriteDirect(_video_output_vsync_pin, !VGA.vsyncpol);
	}

	VGA.linedouble++;

	if (VGA.linedouble == VGA.yscale) {
		VGA.linedouble = 0;
		VGA.line++;
	}

	if (VGA.line == VGA._displayResolutionY) {
		disp = NO_VIDEO_OUT;
	}

	if (VGA.line == VGA.ytotal) {
		if (VGA._currentVideoOutputMode == VGA_MONO) {
			REG_DMAC_SADDR4 = (uint32_t)VGA.monoPixelBufPtr;
		} else if (VGA._currentVideoOutputMode == VGA_COLOR) {
			REG_DMAC_SADDR5 = (uint32_t)VGA.cb;
		}
		VGA.line = 0;
		disp = VGA._currentVideoOutputMode;
		VGA.framecount++;
	}

}


void __attribute__((aligned(64))) PWM_Handler() {
	long t = (REG_PWM_ISR1);

	if (VGA.linedouble) {
		if (VGA._currentVideoOutputMode == VGA_MONO) {
			REG_DMAC_SADDR4 -= (VGA.monoPixelWordsPerLine << 1);
		} else {
			REG_DMAC_SADDR5 -= (VGA.cw);
		}
	}
	//VGA.debug=REG_TC0_CV1;
	asm volatile("wfe \n\t");
}


void __attribute__((aligned(64))) DMAC_Handler() {
	REG_PIOA_PER	= 1 << 26;
	uint32_t dummy	= REG_DMAC_EBCISR;
}


int DueVideoOut::calculateValidModeline() {
	//try to find a suitable modeline
	for (xscale = 16; xscale > 1; xscale--) {
		for (int yti = 0; yti < 50; yti++) {

			/* ---( Compute pixel clock, using the CPU's frequency as a base )--- */
			pclock = 84000000 / xscale;


			/* ---( Calculate X-axis parameters )--- */
			xtotal = (_displayResoultionX * 5 / 4);

			if (xtotal & 1) {
				xtotal -= 1;
			}

			xsyncstart	= ((10 * _displayResoultionX + 2 * xtotal) / 12);
			xsyncend	= ((5  * _displayResoultionX + 7 * xtotal) / 12);


			/* ---( Calculate Y-axis parameters )--- */
			ytotal = ((_displayResolutionY * 25) / 24) + yti;

			ysyncstart	= ((10 * _displayResolutionY + 2 * ytotal) / 12) + 1;
			ysyncend	= ((8  * _displayResolutionY + 4 * ytotal) / 12) + 1;

			if (ysyncstart <= _displayResolutionY) {
				ysyncstart = _displayResolutionY + 1;
			}

			if (ysyncend <= ysyncstart) {
				ysyncend = ysyncstart + 1;
			}

			lfreq = pclock / xtotal;

			for (yscale = 1; yscale <= 8; yscale++) {
				ffreq	= lfreq / (ytotal * yscale);
				ltot	= _displayResolutionY * yscale;

				if ((lfreq > lfreqmin) && (lfreq < lfreqmax) && (ffreq > ffreqmin) && (ffreq < ffreqmax)) {
					goto foundmode;
				}
			}
		}
	}

	/* ---(LABEL: "foundmode" )--- */
	foundmode:;

	/* ---( Check 'xscale' value for sanity / legality )--- */
	if ((xscale == 1) || (_currentVideoOutputMode == VGA_COLOR && xscale < 6)) {
		/**
		 * A value of \b 1 for \p xscale is \e always illegal. When \p _videoOutputMode is \p VGA_COLOR and \p xscale
		 * is \b 5 or less, an illegal situation occurs, as well.
		 *
		 * If either of these conditions occur, we need to return from the function, and indicate that an error occurred
		 */
		return -1;
	}

	/* calculate timings from modeline data */
	xclocks	= (xtotal * xscale) & ~1;
	xstart	= (xtotal - xsyncend) * xscale - 78;


	if (xstart < 132) {
		/** The value of \p xstart must be \b 132 or greater */
		xstart = 132;
	}

	xsyncwidth = (xsyncend - xsyncstart) * xscale;

	return 0;
}


int DueVideoOut::allocateVideoMemory() {
	/* ---( VGA - monochrome _currentVideoOutputMode )--- */
	if (_currentVideoOutputMode == VGA_MONO) {
		monoPixelWordsPerLine = ((_displayResoultionX + 31) / 32) * 2 + 2;
		monoPixelBufferSizeInWords = monoPixelWordsPerLine * _displayResolutionY;
		monoPixelBufPtr = (uint16_t*)calloc(monoPixelBufferSizeInWords, 2);

		if (monoPixelBufPtr == 0) {
			return -2;
		}

		monoPixelBufferBitBandingAliasAddrPtr = (uint32_t*)((int(monoPixelBufPtr - 0x20000000) * 32) + 0x22000000);
		monoPixelBufferBitBandingStride = monoPixelWordsPerLine * 16;
	}

	/* ---( VGA - color _currentVideoOutputMode )--- */
	if ((_currentVideoOutputMode & VGA_COLOR)) {
		cw = _displayResoultionX;
		cbsize = cw * _displayResolutionY;
		cb = (uint8_t*)calloc(cbsize, 1);

		if (cb == 0) {
			return -2;
		}
	}

	return 0;
}


void DueVideoOut::freeAllocatedVideoMemory() {
	/* Free pixel buffer (used by monochrome mode) */
	if (monoPixelBufPtr) {
		free(monoPixelBufPtr);
		monoPixelBufPtr = 0;
	}

	/* Free chroma buffer (used by color mode) */
	if (cb) {
		free(cb);
		cb = 0;
	}
}


void DueVideoOut::startVideoDriverInterrupts() {
	/* Set NVIC interrupt priority to 6 for numbered interrupts 0 - 44 */
	for (int i = 0; i < 45; i++) {
		NVIC_SetPriority(IRQn_Type(i), 6);
	}

	NVIC_SetPriority(DMAC_IRQn, 4);		/* Set NVIC interrupt priority to 4 for the DMA controller */
	NVIC_SetPriority(UART_IRQn, 5);		/* Set NVIC interrupt priority to 5 for the UART module */
	NVIC_SetPriority(PWM_IRQn, 3);		/* Set NVIC interrupt priority to 3 for the PWM module */
	NVIC_SetPriority(TC4_IRQn, 1);		/* Set NVIC interrupt priority to 1 for the TC4 (timer/counter 4) module */
	NVIC_SetPriority(UOTGHS_IRQn, 2);	/* Set NVIC interrupt priority to 2 for */

	if (_currentVideoOutputMode == VGA_MONO) {
		NVIC_EnableIRQ(DMAC_IRQn);
	}

	NVIC_EnableIRQ(TC4_IRQn);
	NVIC_EnableIRQ(PWM_IRQn);
}


void DueVideoOut::stopVideoDriverInterrupts() {
	NVIC_DisableIRQ(PWM_IRQn);
	NVIC_DisableIRQ(TC4_IRQn);
	NVIC_DisableIRQ(DMAC_IRQn);
}


void DueVideoOut::startTimers() {
	REG_PIOA_PDR   = 1 << 20;
	REG_PIOA_ABSR |= 1 << 20;

	REG_PMC_PCER1	= 1 << 4;
	REG_PWM_WPCR	= 0x50574dfc;
	REG_PWM_CLK		= 0x00010001;
	REG_PWM_DIS		= 1 << 2;
	REG_PWM_CMR2	= hsyncpol ? 0x0 : 0x200;
	REG_PWM_CPRD2	= xclocks + 1;
	REG_PWM_CDTY2	= xclocks - xsyncwidth;
	REG_PWM_SCM		= 0;
	REG_PWM_IER1	= 1 << 2;
	REG_PWM_ENA		= 1 << 2;

	REG_PMC_PCER0	= 1 << 31;
	REG_TC1_WPMR	= 0x54494D00;
	REG_TC1_CMR1	= 0b00000000000010011100010000000000;
	REG_TC1_RC1		= xclocks / 2;
	REG_TC1_RA1		= 0;
	REG_TC1_CCR1	= 0b101;
	REG_TC1_IER1	= 0b00010000;
	REG_TC1_IDR1	= 0b11101111;
}


void DueVideoOut::stopTimers() {
	REG_TC1_CCR1	= 0b10;
	REG_TC1_IDR1	= 0b00010000;
	REG_PMC_PCDR0	= 1 << 28;

	REG_PWM_DIS		= 1 << 2;
	REG_PWM_IDR1	= 1 << 2;
}


void DueVideoOut::startVgaOutputMono() {
	for (int i = 34; i <= 41; i++) {
		pinMode(i, INPUT);
	}

	REG_PMC_PCER1    = 1 << 7;
	REG_DMAC_WPMR    = DMAC_WPMR_WPKEY(0x444d4143);
	REG_DMAC_EN      = 1;
	REG_DMAC_GCFG    = 0x00;
	REG_DMAC_EBCIER  = 1 << 4;
	REG_DMAC_SADDR4  = (uint32_t) VGA.monoPixelBufPtr;
	REG_DMAC_DADDR4  = (uint32_t) &REG_SPI0_TDR;
	REG_DMAC_DSCR4   = 0;
	REG_DMAC_CTRLB4  = 0x20310000;
	REG_DMAC_CFG4    = 0x01412210;

	REG_PIOA_PDR     = (1 << 25) | (1 << 27) | (1 << 28);
	REG_PIOA_PER     = 1 << 26;
	REG_PIOA_ABSR   &= ~((1 << 25) | (1 << 27) | (1 << 28));
	REG_PMC_PCER0    = 1 << 24;
	REG_SPI0_WPMR    = 0x53504900;
	REG_SPI0_CR      = 0x1;
	REG_SPI0_MR      = 0x00000011;
	SPI0->SPI_CSR[0] = 0x00000080 + (xscale << 8);

}


void DueVideoOut::stopVgaOutputMono() {
	REG_DMAC_CHDR   = 1 << 4;

	//while(REG_DMAC_CHSR&(1<<4));
	REG_DMAC_EBCIDR = 1 << 4;
	REG_SPI0_CR     = 0x0;
	REG_PIOA_PER    = 1 << 26;
}


void DueVideoOut::startVgaOutputColor() {
	REG_PMC_PCER1 = 1 << 7;
	REG_DMAC_WPMR = DMAC_WPMR_WPKEY(0x444d4143);
	REG_DMAC_EN = 1;
	REG_DMAC_GCFG = 0x00;
	REG_DMAC_SADDR5 = (uint32_t)VGA.cb;
	REG_DMAC_DADDR5 = (uint32_t)0x60000000;
	REG_DMAC_DSCR5 = 0;
	REG_DMAC_CTRLB5 = 0x20000000;
	REG_DMAC_CFG5 = 0x10012200;

	REG_PMC_PCER0 = 1 << 9;
	REG_PIOC_PDR = 0b1111111100;
	REG_PIOC_ABSR &= ~0b1111111100;
	REG_SMC_WPCR = 0x534d4300;
	REG_SMC_SETUP0 = 0x00000000;
	REG_SMC_PULSE0 = 0X00000101;
	REG_SMC_CYCLE0 = xscale;
	REG_SMC_TIMINGS0 = 0;
	REG_SMC_MODE0 = 0x00000000;

}


void DueVideoOut::stopVgaOutputColor() {
	REG_PMC_PCDR0 = 1 << 9;
}


void DueVideoOut::reconfigureDmaPriority() {
	// this code puts DMA priority above CPU.
	MATRIX->MATRIX_WPMR = 0x4D415400;

	for (int i = 0; i < 6; i++) {
		MATRIX->MATRIX_MCFG[i] = 1;
	}

	MATRIX->MATRIX_MCFG[4] = 0;
	for (int i = 0; i < 8; i++) {
		MATRIX->MATRIX_SCFG[i] = 0x01000008;
	}

	MATRIX->MATRIX_SCFG[6]	= 0x011200FF;
	MATRIX->MATRIX_PRAS0	= 0x00020100;
	MATRIX->MATRIX_PRAS1	= 0x00020100;
	MATRIX->MATRIX_PRAS2	= 0x00000000;
	MATRIX->MATRIX_PRAS3	= 0x00000003;
	MATRIX->MATRIX_PRAS4	= 0x00000000;
	MATRIX->MATRIX_PRAS5	= 0x00000000;
	MATRIX->MATRIX_PRAS6	= 0x00030000;
	MATRIX->MATRIX_PRAS7	= 0x00030000;
	MATRIX->MATRIX_PRAS8	= 0x00000100;
}


int DueVideoOut::begin(int targetResX, int targetResY, video_output_mode_t targetVideoMode) {
	if (_isDriverRunning) {
		VGA.end();
	}

	if (targetVideoMode != VGA_MONO && targetVideoMode != VGA_COLOR) {
		return -4;
	}

	if (targetVideoMode == VGA_COLOR && targetResY > 380) {
		return -3;
	}

	if (lfreqmin == 0) {
		lfreqmin = 27000;
		lfreqmax = 83000;
		ffreqmin = 57;
		ffreqmax = 70;
	}

	_displayResoultionX = targetResX;
	_displayResolutionY = targetResY;
	_currentVideoOutputMode  = targetVideoMode;

	textWindowWidthInCharColsX = textFontWidthX = _displayResoultionX / 8;
	textWindowHeightInCharRowsY = textFontHeightY = _displayResolutionY / 8;

	textWindowPosLeftEdgeX   = 0;
	textWindowPosTopEdgeY   = 0;
	textCursorPosColumnX    = 0;
	textCursorPosRowY    = 0;
	textWindowFontColor   = 255;
	textWindowBackgroundColor = 0;

	synced     = 0;
	framecount = 0;
	line       = 0;
	linedouble = 0;

	int retVal;		/* Holds the return value from the mode line calculation and video memory allocation functions */

	retVal = calculateValidModeline();
	if (retVal) {
		return retVal;
	}

	retVal = allocateVideoMemory();
	if (retVal) {
		return retVal;
	}

	reconfigureDmaPriority();

	/* Set the HSYNC and VSYNC pins to OUTPUT _currentVideoOutputMode */
	pinMode(_video_output_hsync_pin, OUTPUT);
	pinMode(_video_output_vsync_pin, OUTPUT);

	/* Start the timers used by the driver */
	startTimers();

	/* Start the video output in the appropriate chromatic _currentVideoOutputMode */
	if (_currentVideoOutputMode == VGA_MONO) {
		/* Start the video output in monochrome _currentVideoOutputMode */
		startVgaOutputMono();
	} else if (_currentVideoOutputMode == VGA_COLOR) {
		/* Start the video output in color _currentVideoOutputMode */
		startVgaOutputColor();
	}

	/* Start the interrupts used by the driver */
	startVideoDriverInterrupts();

	/* Store the state of the driver, and return successfully */
	_isDriverRunning = 1;
	return 0;
}


int DueVideoOut::beginPAL()  {
	_currentVideoOutputMode = COMPOSITE_PAL;

	/* Composite PAL video has a static screen geometry of 320x240 */
	_displayResoultionX = 320;
	_displayResolutionY = 240;


	/* Given screen geometry, the maximum dimensions of a text window are 40x30 characters */
	textWindowWidthInCharColsX = 40;
	textWindowHeightInCharRowsY = 30;

	textWindowPosLeftEdgeX = 0;
	textWindowPosTopEdgeY = 0;
	textCursorPosColumnX  = 0;
	textCursorPosRowY  = 0;

	textWindowFontColor = 255;
	textWindowBackgroundColor = 0;

	synced = 0;
	framecount = 0;

	xscale = 12;
	yscale = 1;

	xtotal     = 448;
	xsyncstart = 335;
	xsyncend   = 368;

	ytotal     = 312;
	ysyncstart = 270;
	ysyncend   = 272;

	lfreq = 15625;
	pclock = 7000000;
	ltot = 262;
	xclocks = 5376;
	xstart = 126;
	xsyncwidth = 394;

	line = 0;
	linedouble = 0;

	phase = 0;
	poff  = 28;

	int r;
	dmabuf = (uint16_t*)malloc(2048);

	crt[0] = (const uint16_t*)cretab;
	crt[1] = (const uint16_t*)crotab;
	cbt[0] = cbetab;
	cbt[1] = cbotab;

	r = allocateVideoMemory();

	if (r) {
		return r;
	}


	/* Set the HSYNC and VSYNC pins to OUTPUT _currentVideoOutputMode */
	pinMode(_video_output_hsync_pin, OUTPUT);
	pinMode(_video_output_vsync_pin, OUTPUT);

	startTimers();

	reconfigureDmaPriority();

	REG_PMC_PCER1 = 1 << 7;
	REG_DMAC_WPMR = DMAC_WPMR_WPKEY(0x444d4143);
	REG_DMAC_EN = 1;
	REG_DMAC_GCFG = 0x00;
	REG_DMAC_SADDR5 = (uint32_t) dmabuf;
	REG_DMAC_DADDR5 = (uint32_t) 0x60000000;
	REG_DMAC_DSCR5 = 0;
	REG_DMAC_CTRLB5 = 0x20000000;
	REG_DMAC_CFG5 = 0x10702200;

	REG_PMC_PCER0 = 1 << 9;
	REG_PIOC_PDR = 0b1111111100;
	REG_PIOC_ABSR &= ~0b1111111100;
	REG_SMC_WPCR = 0x534d4300;
	REG_SMC_SETUP0 = 0x00000000;
	REG_SMC_PULSE0 = 0X00000101;
	REG_SMC_CYCLE0 = 6;
	REG_SMC_TIMINGS0 = 0;
	REG_SMC_MODE0 = 0x00000000;

	startVideoDriverInterrupts();
	_isDriverRunning = 1;
	return 0;

}


int DueVideoOut::beginNTSC() {
	_currentVideoOutputMode = COMPOSITE_NTSC;

	/* NTSC _currentVideoOutputMode uses a static screen geometry of 320x200 */
	_displayResoultionX = 320;
	_displayResolutionY = 200;


	textWindowWidthInCharColsX = 40;	/* The maximum text window width in NTSC _currentVideoOutputMode is 40 */
	textWindowHeightInCharRowsY = 25;	/* The maximum text window height in NTSC _currentVideoOutputMode is 25 */

	textWindowPosLeftEdgeX = 0;
	textWindowPosTopEdgeY = 0;
	textCursorPosColumnX  = 0;
	textCursorPosRowY  = 0;

	textWindowFontColor = 255;
	textWindowBackgroundColor = 0;

	synced = 0;
	framecount = 0;

	xscale = 12;
	yscale = 1;

	xtotal = 444;
	xsyncstart = 335;
	xsyncend = 368;
	ytotal = 262;
	ysyncstart = 230;
	ysyncend = 236;

	lfreq = 15778;
	pclock = 7000000;
	ltot = 262;
	xclocks = 5328;
	xstart = 130;
	xsyncwidth = 394;
	line = linedouble = 0;
	phase = 0;
	poff = 8;

	int r;

	dmabuf = (uint16_t *)malloc(2048);
	crt[0] = (const uint16_t *)crtab;
	cbt[0] = cbtab;


	r = allocateVideoMemory();

	if (r) {
		return r;
	}

	pinMode(_video_output_hsync_pin, OUTPUT);
	pinMode(_video_output_vsync_pin, OUTPUT);

	startTimers();

	reconfigureDmaPriority();

	REG_PMC_PCER     = 1 << 7;
	REG_DMAC_WPMR    = DMAC_WPMR_WPKEY(0x444d4143);
	REG_DMAC_EN      = 1;
	REG_DMAC_GCFG    = 0x00;
	REG_DMAC_SADDR5  = (uint32_t)dmabuf;
	REG_DMAC_DADDR5  = (uint32_t)0x60000000;
	REG_DMAC_DSCR5   = 0;
	REG_DMAC_CTRLB5  = 0x20000000;
	REG_DMAC_CFG5    = 0x10702200;

	REG_PMC_PCER0    = 1 << 9;
	REG_PIOC_PDR     = 0b1111111100;
	REG_PIOC_ABSR   &= ~0b1111111100;
	REG_SMC_WPCR     = 0x534d4300;
	REG_SMC_SETUP0   = 0x00000000;
	REG_SMC_PULSE0   = 0X00000101;
	REG_SMC_CYCLE0   = 6;
	REG_SMC_TIMINGS0 = 0;
	REG_SMC_MODE0    = 0x00000000;

	startVideoDriverInterrupts();

	_isDriverRunning = 1;

	return 0;
}


void DueVideoOut::end() {
	/* Ensure the driver is actually running before we try to stop it... */
	if (!_isDriverRunning) {
		/* If the driver isn't running, we can return from the function immediately */
		return;
	}

	_isDriverRunning = 0;	/* Set the driver's state to stopped */

	stopVideoDriverInterrupts();

	if (_currentVideoOutputMode == VGA_MONO) {
		stopVgaOutputMono();
	} else if (_currentVideoOutputMode & VGA_COLOR) {
		stopVgaOutputColor();
	}

	stopTimers();

	/* Restore the HSYNC and VSYNC pins to a safe default _currentVideoOutputMode ('INPUT' _currentVideoOutputMode is generally Hi-Z) */
	pinMode(_video_output_hsync_pin, INPUT);
	pinMode(_video_output_vsync_pin, INPUT);

	/* ---( Free video memory and other resources )--- */
	freeAllocatedVideoMemory();

	if ((_currentVideoOutputMode == VGA_NTSC) || (_currentVideoOutputMode == VGA_PAL)) {
		free(dmabuf);
	}

	pclock     = 0;
	_displayResoultionX      = 0;
	xsyncstart = 0;
	xsyncend   = 0;
	xtotal     = 0;
	_displayResolutionY      = 0;
	ysyncstart = 0;
	ysyncend   = 0;
	ytotal     = 0;

	_currentVideoOutputMode = NO_VIDEO_OUT;


	line       = 0;
	linedouble = 0;
	synced     = 0;
	xclocks    = 0;
	xstart     = 0;
	xsyncwidth = 0;
	xscale     = 0;
	yscale     = 0;
	lfreq      = 0;
	ffreq      = 0;
	ltot       = 0;

	return;
}



