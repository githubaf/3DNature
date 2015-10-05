// Palette.h
// Stuff to handle the 16, 256 and 16M color hoop-thru-jumping
// Created from scratch on 7/25/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_PALETTE_H
#define WCS_PALETTE_H

// Where all the color ranges are located in the palette:
#define WCS_PALETTE_DEFAULT_BASE	0	// thru 7
#define WCS_PALETTE_DEFAULT_SIZE	8	// thru 7
// These are for 256-color mode
#define WCS_PALETTE_COLOR_BASE		8	// thru 15
#define WCS_PALETTE_COLOR_SIZE		16	// thru 15

// Name other types we'll be referencing
class GUIContext;
class Fenetre;

// Here's a debugging option you can set in either the Amiga or
// Win versions to range-check the number you pass to the Color-
// Range selection function.

#define WCS_PALETTE_RANGE_CHECKING

// Clarify the basic color manipulation type

#ifdef _WIN32

// No shift needed under Windows
#define SHIFT32(a) (a)
#define UNSHIFT32(a) (a)

typedef PALETTEENTRY ColPalThingy;
#endif // _WIN32

// Instantiate the basic drawing-color type -- this is what you specify
// to the various graphics functions if you're not just plotting in
// truecolor mode
#ifdef _WIN32
typedef HPEN PenThing;

// I Hate Windows. Just so you know.
struct MyLOGPALETTE {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[240];
};
#endif // _WIN32

unsigned char RedPart(unsigned short FullPart);
unsigned char GreenPart(unsigned short FullPart);
unsigned char BluePart(unsigned short FullPart);

class PaletteMeister
	{
	friend class DrawingFenetre;
	friend class GUIFenetre;
	private:
		GUIContext *Display;
		// These are not hardware palette data types, they are once
		// removed through the OS layers. Therefore they are 'symbolic'
		// of the actual palettes.

		// See above to tell what type this thing really is pointing to...
		ColPalThingy *DefaultSym;

		#ifdef _WIN32
		// Windows needs this to throw around
		HPALETTE MixingBoard;

		HPEN WinPenCacheDef[8];
		#endif // _WIN32

		unsigned long ErrorStatus;

		#ifdef _WIN32
		void InitPenCache(void);
		unsigned char WInstantiate(Fenetre *Fen);
		#endif // _WIN32
		void CopyToArch(ColPalThingy *InSym, unsigned short *In8, unsigned char ColorDepth);
	public:
		PaletteMeister(GUIContext *OSDisplay);
		~PaletteMeister();
		inline unsigned long InquireStatus(void) {return(ErrorStatus);};

		void PickDefColor(unsigned char DefColor, Fenetre *Win);
		void Instantiate(Fenetre *Canvas);
	}; // PaletteMeister

// Symbolic names for the color found in the default set below. Handy.
enum
	{
	WCS_PALETTE_DEF_BLUGRY,
	WCS_PALETTE_DEF_BLACK,
	WCS_PALETTE_DEF_WHITE,
	WCS_PALETTE_DEF_RED,
	WCS_PALETTE_DEF_DKBLUE,
	WCS_PALETTE_DEF_GREEN,
	WCS_PALETTE_DEF_LTBLUE,
	WCS_PALETTE_DEF_YELLOW
	}; // HandyDefColors

// I've removed the palette arrays from here (Palette.h) and put them
// into palette.cpp so I wouldn't have to mess with the EXTERN/multi
// include problem. They're not needed outside the PaletteMeister
// object private scope anyway.

enum // These are the errors that the PaletteMeister can post
	{
	WCS_ERROR_PALETTE_NEW
	}; // WCS_ERROR_PALETTE

#endif // WCS_PALETTE_H
