// Palette.cpp
// Code to do that palette stuff for the PaletteMeister object
// created from scratch on 8/4/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Palette.h"
#include "Fenetre.h"
#include "GUI.h"

// Static RGB data for 8-color palettes. In 256 color mode, the
// default palette is still 8 colors, the pure-grey palettes are
// synthesized without using these, and the color-range palettes
// created by stretching these over 128 gradient colors.

// Therefore the grey ones are not necessary (and are ifdef'ed out)
// in the Windows version, since it can't do 16-color mode.

// These are still used many places
unsigned short Default8[8] =
	{
	0x089b, // WCS_PALETTE_DEF_BLUGRY
	0x0000, // WCS_PALETTE_DEF_BLACK
	0x0ddd, // WCS_PALETTE_DEF_WHITE
	0x0b10, // WCS_PALETTE_DEF_RED
	0x0348, // WCS_PALETTE_DEF_DKBLUE
	0x0392, // WCS_PALETTE_DEF_GREEN
	0x037c, // WCS_PALETTE_DEF_LTBLUE
	0x0dd2  // WCS_PALETTE_DEF_YELLOW
	}; // DefaultRange


void PaletteMeister::CopyToArch(ColPalThingy *InSym, unsigned short *In8, unsigned char ColorDepth)
{
// Copies out of 8-entry 8-bit color tables into 8-entry architecture
// dependent Color symbol tables.

int CopyLoop;

//if(ColorDepth == 8)
	{
	for(CopyLoop = 0; CopyLoop < 8; CopyLoop++)
		{
		InSym[CopyLoop].peRed   = SHIFT32(RedPart(In8[CopyLoop]));
		InSym[CopyLoop].peGreen = SHIFT32(GreenPart(In8[CopyLoop]));
		InSym[CopyLoop].peBlue  = SHIFT32(BluePart(In8[CopyLoop]));
		#ifdef _WIN32
		InSym[CopyLoop].peFlags = PC_RESERVED;
		#endif // _WIN32
		} // for
	} // if

} // PaletteMeister::CopyToArch


PaletteMeister::PaletteMeister(GUIContext *OSDisplay)
{
#ifdef _WIN32
MyLOGPALETTE *Blank;
int BlankOut;
#endif // _WIN32

ErrorStatus = NULL;
DefaultSym = NULL;

if(!(Display = OSDisplay))
	{
	ErrorStatus = WCS_ERROR_PALETTE_NEW;
	return;
	} // if


// Windows needs a palette structure dynamically created to manipulate
// colors.
#ifdef _WIN32
MixingBoard = NULL;
if(Blank = new MyLOGPALETTE)
	{
	Blank->palVersion = 0x300;
    Blank->palNumEntries = 240;
    for(BlankOut = 0; BlankOut < 240; BlankOut++)
		{
		Blank->palPalEntry[BlankOut].peRed    = 
		 Blank->palPalEntry[BlankOut].peGreen = 
		 Blank->palPalEntry[BlankOut].peBlue  = 0;
		Blank->palPalEntry[BlankOut].peFlags  = PC_RESERVED;
		} // for
	if((MixingBoard = CreatePalette((struct tagLOGPALETTE*)Blank)) == NULL)
		{
		ErrorStatus = WCS_ERROR_PALETTE_NEW;
		} // if
	delete Blank;
	Blank = NULL;
	} // if
#endif // _WIN32

// Create palette data structures on the fly for device capabilities

// Default colors are common
if(DefaultSym = new ColPalThingy[8])
	{
	CopyToArch(DefaultSym, Default8, Display->ColorDepth);
	} // if
else
	{
	ErrorStatus = WCS_ERROR_PALETTE_NEW;
	return;
	} // else

#ifdef _WIN32
// Hopefully cacheing all the OTHER freakin' pens will
// keep Windows from losing them all the time...
for(BlankOut = 0; BlankOut < 8; BlankOut++)
	{
	WinPenCacheDef[BlankOut] = NULL;
	} // for
#endif // _WIN32

// If anything went wrong, it'll set ErrorStatus, and some of the *Syms
// will be NULL.

} // PaletteMeister:PaletteMeister

PaletteMeister::~PaletteMeister()
{
// Operator delete[] checks for NULLs, so we don't have to.
delete [] DefaultSym;
DefaultSym = NULL;


#ifdef _WIN32
for(int BlankOut = 0; BlankOut < 8; BlankOut++)
	{
	DeleteObject(WinPenCacheDef[BlankOut]);
	WinPenCacheDef[BlankOut] = NULL;
	} // for

if(MixingBoard)
	{
	DeleteObject(MixingBoard);
	MixingBoard = NULL;
	} // if
#endif // _WIN32

} // PaletteMeister::~PaletteMeister

#ifdef _WIN32
void PaletteMeister::InitPenCache(void)
{
int BlankOut;
COLORREF PixelCol;

for(BlankOut = 0; BlankOut < 8; BlankOut++)
	{
	PixelCol = RGB(DefaultSym[BlankOut].peRed, DefaultSym[BlankOut].peGreen, DefaultSym[BlankOut].peBlue);
	WinPenCacheDef[BlankOut] = CreatePen(PS_SOLID, 1, PixelCol);
	} // for
	
} // PaletteMeister::InitPenCache
#endif // _WIN32


void PaletteMeister::PickDefColor(unsigned char DefColor, Fenetre *Win)
{
#ifdef _WIN32
HPEN *WinPenCache;
HPEN NewPen = NULL;
#endif // _WIN32
ColPalThingy *Symbol;
DrawingFenetre *Sketch;

Sketch = (DrawingFenetre *)Win;

#ifdef _WIN32

if(!WinPenCacheDef[0])
	{
	InitPenCache();
	} // if

#endif // _WIN32

#ifdef _WIN32
NewPen = NULL;

// case WCS_PALETTE_GROUP_DEFAULT:
Symbol = DefaultSym;
WinPenCache = WinPenCacheDef;
Sketch->PixelCol = RGB(Symbol[DefColor].peRed, Symbol[DefColor].peGreen, Symbol[DefColor].peBlue);
NewPen = WinPenCache[DefColor];

if(NewPen)
	{
	Win->InstallColorHook(NewPen);
	} // if
return;

#endif // _WIN32
} // PaletteMeister::PickDefColor


void PaletteMeister::Instantiate(Fenetre *Canvas)
{
void *Symbol;

#ifdef _WIN32

if(!WinPenCacheDef[0])
	{
	InitPenCache();
	} // if
#endif // _WIN32


Symbol = DefaultSym;
Canvas->RangeBase = WCS_PALETTE_DEFAULT_BASE;
Canvas->RangeSizeCheck = WCS_PALETTE_DEFAULT_SIZE;
#ifdef _WIN32
if(SetPaletteEntries(MixingBoard, Canvas->RangeBase, Canvas->RangeSizeCheck, (ColPalThingy *)Symbol))
	{
	WInstantiate(Canvas);
	} // if
#endif // _WIN32
} // PaletteMeister::Instantiate

#ifdef _WIN32
unsigned char PaletteMeister::WInstantiate(Fenetre *Fen)
{
HDC ColorShuffle;
unsigned char RealStat = 0;

if(ColorShuffle = Fen->GetNativeDC())
	{
	UnrealizeObject(MixingBoard);
	if(SelectPalette(ColorShuffle, MixingBoard, FALSE))
		{
		RealStat = RealizePalette(ColorShuffle);
		} // if
	Fen->ReleaseNativeDC();
	ColorShuffle = NULL;
	} // if

// If we're a double-buffered DrawingFenetre, we need to do the
// Backup DC as well.
if(ColorShuffle = Fen->GetBackupDC())
	{
	// We haven't changed anything, don't bother Unrealizing...
	//UnrealizeObject(MixingBoard);
	if(SelectPalette(ColorShuffle, MixingBoard, FALSE))
		{
		// Ignore return value...
		RealizePalette(ColorShuffle);
		} // if
	// Backup DC is persistant, no need to release...
	//Fen->ReleaseNativeDC();
	ColorShuffle = NULL;
	} // if

return(RealStat);

} // PaletteMeister::WInstantiate

#endif // _WIN32

unsigned char RedPart(unsigned short FullPart)
{
unsigned char Foo;

Foo  = (unsigned char)((FullPart & 0x0f00) >> 4);
Foo |= (Foo >> 4);
return(Foo);
} // RedPart

unsigned char GreenPart(unsigned short FullPart)
{
unsigned char Foo;

Foo  = (unsigned char)((FullPart & 0x00f0));
Foo |= (Foo >> 4);
return(Foo);
} // GreenPart

unsigned char BluePart(unsigned short FullPart)
{
unsigned char Foo;

Foo  = (unsigned char)((FullPart & 0x000f));
Foo |= (Foo << 4);
return(Foo);
} // BluePart
