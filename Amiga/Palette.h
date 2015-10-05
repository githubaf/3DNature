// Palette.h
// Stuff to handle the 16, 256 and 16M color hoop-thru-jumping
// Created from scratch on 7/25/95 by Chris "Xenon" Hanson
// Copyright 1995

// name/mode   A16, 256, 16M
// Default/UI:   8,   8,   8
// Grey Range:   8, 128, 256
// Color Pots:   8,   8,   8
// Render:		16,  92, 16M
// Unused:       -   20    -
// 

#ifndef WCS_PALETTE_H
#define WCS_PALETTE_H

#ifdef WIN32
#include <windows.h>
#endif // WIN32

// Name other types we'll be referencing
class GUIContext;
class Fenetre;

// Here's a debugging option you can set in either the Amiga or
// Win versions to range-check the number you pass to the Color-
// Range selection function.

#define WCS_PALETTE_RANGE_CHECKING

// Clarify the basic color manipulation type

#ifdef AMIGA
struct PALWIDG // Used with (Load|Set|Get)RGB32
	{
	unsigned long int peRed, peGreen, peBlue; // Yes, 32-bit unsigned
	}; // PALWIDG
typedef PALWIDG ColPalThingy;

struct ConGlom
	{
	unsigned long int Check;
	ColPalThingy Paints[256];
	unsigned long int Terminator;
	}; // ConGlomerate

#define SHIFT32(a) (a << 24)

#endif // AMIGA
#ifdef WIN32

// No shift needed under Windows
#define SHIFT32(a) (a)

typedef PALETTEENTRY ColPalThingy;
#endif // WIN32

struct SimpleRGBTrio
	{
	unsigned char Red, Green, Blue;
	}; // SimpleRGBTrio, used when setting render colors

// Instantiate the basic drawing-color type -- this is what you specify
// to the various graphics functions if you're not just plotting in
// truecolor mode
#ifdef AMIGA
typedef unsigned char PenThing;
#endif // AMIGA
#ifdef WIN32
typedef HPEN PenThing;

// I Hate Windows. Just so you know.
struct MyLOGPALETTE {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[240];
};
#endif // WIN32


// These are the groups of colors a window can ask for.
enum
	{
	WCS_PALETTE_GROUP_DEFAULT,		// Starts at 0, increases by 1
	WCS_PALETTE_GROUP_COLORPOTS,
	WCS_PALETTE_GROUP_GREY,			// }
	WCS_PALETTE_GROUP_LIGHTGREY,	// } These four are actually the same
	WCS_PALETTE_GROUP_EARTH,		// } range of hardware indices, even in
	WCS_PALETTE_GROUP_PRIMARY,		// } 256 color mode
	WCS_PALETTE_GROUP_RENDER		// ...6
	}; // ColorGroups

class PaletteMeister
	{
	friend class DrawingFenetre;
	friend class GUIFenetre;
	private:
		GUIContext *Display;
		// These are not hardware palette data types, they are once
		// removed through the OS layers. Therefore they are 'symbolic'
		// of the actual palettes. This may not even be used in Amiga
		// 16-color mode.
		// See above to tell what type this thing really is pointing to...
		ColPalThingy *DefaultSym, *ColorSym, *GreySym, *LtGreySym,
		 *EarthSym, *PrimarySym, *RenderSym;
		// This is a handy counter of how many of the above there are.
		#define WCS_PALETTE_NUMGROUPS	7
		#ifdef AMIGA
		// Under WIN32 we have to create and destroy these on the fly,
		// so no arrays of pointers to them hang around...
		PenThing *Thingy[WCS_PALETTE_NUMGROUPS];
		unsigned short MixingBoard4[16];
		struct ConGlom MixingBoard32;
		#endif // AMIGA
		unsigned long int ErrorStatus;
		#ifdef WIN32
		// Windows needs these to throw around
		HPALETTE GiveBack;
		HPALETTE MixingBoard;
		HPEN LastPen;
		#endif // WIN32


		unsigned char RedPart(unsigned short FullPart);
		unsigned char GreenPart(unsigned short FullPart);
		unsigned char BluePart(unsigned short FullPart);

		unsigned char WInstantiate(Fenetre *Fen);
		void CopyToArch(ColPalThingy *InSym, unsigned short *In8, unsigned char ColorDepth);
		void ExpandToArch(ColPalThingy *InSym, unsigned short *In8);
		ColPalThingy *ExpandIfNeeded(unsigned short *In8, unsigned char ColorDepth);
		#ifdef AMIGA
		PenThing *MakePenThingIdx(unsigned char Base, unsigned char Size);
		#endif // AMIGA
		ColPalThingy *ExpandGreyRange(unsigned char Start,
		 unsigned char Step);
	public:
		PaletteMeister(GUIContext *OSDisplay);
		~PaletteMeister();
		inline unsigned long int InquireStatus(void) {return(ErrorStatus);};

		void SelectColors(unsigned char ColorGroup, Fenetre *Target);
		void PickDefColor(unsigned char DefColor, Fenetre *Win);
		void PMSelectRangePen(unsigned char PenDesc, Fenetre *Win, unsigned long int PalIdx = 300);
		void Instantiate(unsigned long int PalIdx, Fenetre *Canvas);
		void SetColorPot(unsigned char PotNum, unsigned char PotRed,
		 unsigned char PotGreen, unsigned char PotBlue);
		void SetRenderColors(struct SimpleRGBTrio *RendGroup);
		unsigned char RangeStep(unsigned char A, unsigned char B,
		 unsigned char Step, unsigned char StepMax);
		#ifdef AMIGA
		PenThing MatchRenderClosestPair(unsigned char R, unsigned char G,
		 unsigned char B, PenThing &Primary, PenThing &Secondary);

		#endif // AMIGA
	}; // PaletteMeister


// Where all the color ranges are located in the palette:
// <<<>>> May need to rearrange these to improve XOR-lines
#define WCS_PALETTE_DEFAULT_BASE	0	// thru 7
#define WCS_PALETTE_DEFAULT_SIZE	8	// thru 7
// These are for 256-color mode
#define WCS_PALETTE_COLOR_BASE		8	// thru 15
#define WCS_PALETTE_COLOR_SIZE		8	// thru 15
#define WCS_PALETTE_GREY_BASE		16	// thru 143
#define WCS_PALETTE_GREY_SIZE		128	// thru 143
#define WCS_PALETTE_RENDER_BASE		144	// thru 235
#define WCS_PALETTE_RENDER_SIZE		92	// thru 235
// This is where the above all fall in 16-color mode
#define WCS_PALETTE_MULTI_BASE		8   // COLOR, GREY and RENDER
#define WCS_PALETTE_MULTI_SIZE		8
// In 256-color mode on the Amiga we leave these unclaimed, since
// we can't get the last 20 under Windows and we're being consistant
// for the sake of ease of programming. Under AmigaDOS v39+ we
// should register these pens as available for other programs'
// windows on our public screen. <<<>>>
#define WCS_PALETTE_UNUSED_BASE		236	// thru 255
#define WCS_PALETTE_UNUSED_SIZE		20	// thru 255


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
