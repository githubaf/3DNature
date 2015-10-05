// FontImage.h
// Header for rendering font images
// Built from scratch on 03/12/02 by Frank Weed II
// Copyright 2002 3D Nature, LLC. All rights reserved.

#ifndef WCS_FONTIMAGE_H
#define WCS_FONTIMAGE_H

enum FIJustify
	{
	WCS_FONTIMAGE_JUSTIFY_CENTER,
	WCS_FONTIMAGE_JUSTIFY_LEFT,
	WCS_FONTIMAGE_JUSTIFY_RIGHT,
	WCS_FONTIMAGE_JUSTIFY_NONE
	}; // FIJustify

enum FIMode
	{
	WCS_FONTIMAGE_DRAWMODE_NORMAL
	}; // FIMode

enum FIBuffer
	{
	FI_RED,
	FI_GREEN,
	FI_BLUE,
	FI_ALPHA,
	FI_EDGE
	}; // FIbuffer

// This class handles acquiring (loading and decompressing) and releasing the built-in raster font
class InternalRasterFont
	{
	public:
		int InternalFontWidth, InternalFontHeight;
		unsigned char *FontDataPointer;
		unsigned char TinyFont;

		InternalRasterFont(unsigned char Tiny = 0);
		~InternalRasterFont();
	}; // InternalRasterFont

class FontImage
	{
	private:
		USHORT chwidth, chheight, fwidth, fheight;
		ULONG  rwidth, rheight;
		long rmax;
		UBYTE  leftx[256], widthx[256];
		UBYTE  *font, *red, *grn, *blu, *alf, *edge;
	public:
		FontImage();
		long CharWidth(unsigned char ch);
		void DebugDump(void);
		bool Dump(char *filename, FIBuffer raster);	// call DumpKill after you're done with the temp file that this creates
		bool DumpKill(char *filename);
		long OutlineText(UBYTE edging);
		long PrintText(ULONG x, ULONG y, char *string, UBYTE r, UBYTE g, UBYTE b, FIJustify justification, FIMode drawmode,
			char leading, UBYTE kerning, UBYTE edging, PostProcText *PPT = NULL);
		long SetFont(UBYTE *fontimage, USHORT fontwidth, USHORT fontheight);
		long SetOutput(ULONG owidth, ULONG oheight, UBYTE *rband, UBYTE *gband, UBYTE *bband, UBYTE *aband = NULL, UBYTE *eband = NULL);
		long TextWidth(char *str, UBYTE kerning);
		~FontImage();
	}; // FontImage

void NeHeUpper(char *str);
void strcat_NeHeUpper(char *dest, const char *src, bool italics);

#endif // WCS_FONTIMAGE_H
