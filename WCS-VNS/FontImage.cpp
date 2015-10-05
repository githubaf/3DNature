// FontImage.cpp
// Code for rendering font images
// Built from scratch on 03/12/02 by Frank Weed II
// Copyright 2002 3D Nature, LLC. All rights reserved.

#include "stdafx.h"

class PostProcText;

#include "Application.h"
#include "GUI.h"
#include "Effectslib.h"
#include "FontImage.h"
#include "PostProcessEvent.h"
#include "AppMem.h"
#include "zlib.h"
#include "resource.h"
#include "Project.h"

extern WCSApp *GlobalApp;

/*===========================================================================*/

FontImage::FontImage()
{

} // FontImage::FontImage

/*===========================================================================*/

FontImage::~FontImage()
{

} // FontImage::~FontImage

/*===========================================================================*/

long FontImage::CharWidth(unsigned char ch)
{
unsigned char NeHe = ch;

if (ch < 32)
	return 0;

NeHe -= 32;
return widthx[NeHe];

} // FontImage::CharWidth

/*===========================================================================*/

// quick & dirty raster dump to C drive
void FontImage::DebugDump(void)
{
FILE *out;
ULONG rsize = rwidth * rheight;
char fname[256], tmp[64];

sprintf(tmp, "_%dx%d.raw", rwidth, rheight);

PROJ_mkdir("C:/_Debug");

if (red)
	{
	sprintf(fname, "C:/_Debug/red");
	strcat(fname, tmp);
	if (out = fopen(fname, "wb"))
		{
		fwrite(red, 1, rsize, out);
		fclose(out);
		} // if
	} // if

if (grn)
	{
	sprintf(fname, "C:/_Debug/grn");
	strcat(fname, tmp);
	if (out = fopen(fname, "wb"))
		{
		fwrite(grn, 1, rsize, out);
		fclose(out);
		} // if
	} // if

if (blu)
	{
	sprintf(fname, "C:/_Debug/blu");
	strcat(fname, tmp);
	if (out = fopen(fname, "wb"))
		{
		fwrite(blu, 1, rsize, out);
		fclose(out);
		} // if
	} // if

if (alf)
	{
	sprintf(fname, "C:/_Debug/alf");
	strcat(fname, tmp);
	if (out = fopen(fname, "wb"))
		{
		fwrite(alf, 1, rsize, out);
		fclose(out);
		} // if
	} // if

if (edge)
	{
	sprintf(fname, "C:/_Debug/edge");
	strcat(fname, tmp);
	if (out = fopen(fname, "wb"))
		{
		fwrite(edge, 1, rsize, out);
		fclose(out);
		} // if
	} // if

} // FontImage::DebugDump

/*===========================================================================*/

bool FontImage::Dump(char *filename, FIBuffer raster)
{
FILE *daFile;
UBYTE *which;
ULONG rsize = rwidth * rheight;
bool success = false;

if (filename && (daFile = fopen(filename, "wb")))
	{
	if (raster == FI_RED)
		which = red;
	else if (raster == FI_GREEN)
		which = grn;
	else if (raster == FI_BLUE)
		which = blu;
	else if (raster == FI_ALPHA)
		which = alf;
	else
		which = edge;

	fwrite(which, 1, rsize, daFile);
	fclose(daFile);

	success = true;
	} // if

return(success);

} // FontImage::Dump

/*===========================================================================*/

bool FontImage::DumpKill(char *filename)
{
bool success = false;

if (filename)
	{
	remove(filename);	// no idea what return codes to this are
	success = true;
	} // if

return(success);

} // FontImage::DumpKill

/*===========================================================================*/

long FontImage::SetFont(UBYTE *fontimage, USHORT fontwidth, USHORT fontheight)
{
unsigned long  fx, fy, fzip;
unsigned short i;
unsigned char  *fpixel, maxx, minx, space, xpos, ypos;

// need a pointer, and dimensions modulo 16
if ((fontimage == NULL) || (fontwidth % 16) || (fontheight % 16))
	return 0;	// failed

font = fontimage;
fwidth = fontwidth;
chwidth = fontwidth / 16;
fheight = fontheight;
chheight = fontheight / 16;

// grab width of 'n' {#78} first, and use that as the size of the space character
fx = (78 % 16) * chwidth;
fy = (78 / 16) * chheight;
minx = 255;
maxx = 0;
for (ypos = 0; ypos < chheight; ypos++)
	{
	fzip = (fy + ypos) * fwidth + fx;
	fpixel = &font[fzip];
	for (xpos = 0; xpos < chwidth; xpos++, fpixel++)
		{
		if (*fpixel)	// is the font pixel set?
			{
			if (xpos < minx)
				minx = xpos;
			if (xpos > maxx)
				maxx = xpos;
			}
		} // for x
	} // for y
if (maxx > minx)
	space = maxx - minx;
else
	space = chwidth / 2;	// n character is blank in this font image!

// run thru the font image, figuring out which columns hold the first & last set pixels (to appoximate kerning)
for (i = 0; i < 256; i++)
	{
	fx = (i % 16) * chwidth;
	fy = (i / 16) * chheight;
	minx = 255;
	maxx = 0;
	for (ypos = 0; ypos < chheight; ypos++)
		{
		fzip = (fy + ypos) * fwidth + fx;
		fpixel = &font[fzip];
		for (xpos = 0; xpos < chwidth; xpos++, fpixel++)
			{
			if (*fpixel)	// is the font pixel set?
				{
				if (xpos < minx)
					minx = xpos;
				if (xpos > maxx)
					maxx = xpos;
				}
			} // for x
		} // for y
	// if we scanned a completely blank font box (ie: space), just set minx to 0, and maxx width to the size we calculated for the space character
	if (maxx < minx)
		{
		minx = 0;
		maxx = space;
		}
	leftx[i] = minx;
	widthx[i] = maxx - minx + 1;
	} // for i

return 1;	// success

} // FontImage::SetFont

/*===========================================================================*/

long FontImage::SetOutput(ULONG owidth, ULONG oheight, UBYTE *rband, UBYTE *gband, UBYTE *bband, UBYTE *aband, UBYTE *eband)
{

if ((owidth > 0) && (oheight > 0) && (rband || gband || bband))
	{
	rwidth = owidth;
	rheight = oheight;
	rmax = owidth * oheight;
	red = rband;
	grn = gband;
	blu = bband;
	alf = aband;
	edge = eband;
	return 1;	// success
	}

return 0;	// failure

} // FontImage::SetOutput

/*===========================================================================*/

long FontImage::OutlineText(UBYTE edging)
{
unsigned char *aptr, *eptr, *eptr2;
unsigned long xpos, ypos, zip;
short xx, yy;

if (edge == NULL)
	return 0;	// failed - no outline raster available!

//DebugDump();

// pass 1 - put an outline in the edge channel
for (ypos = edging; ypos < (rheight - edging); ypos++)
	{
	zip = ypos * rwidth;
	aptr = &alf[zip] + edging;
	eptr = &edge[zip] + edging;
	for (xpos = edging; xpos < (rwidth - edging); xpos++, aptr++, eptr++)
		{
		// if there's something in the alpha channel, copy it around the box
		if (*aptr)
			{
			for (yy = -edging; yy <= edging; yy++)
				{
				eptr2 = eptr + yy * rwidth;
				for (xx = -edging; xx <= edging; xx++)
					{
					if (*aptr > *(eptr2 + xx))
						*(eptr2 + xx) = *aptr;
					} // for xx
				} // for yy
			} // if
		} // for x
	} // for y

//DebugDump();

// pass 2 - make sure that the sum of the alpha & edge channels are always <= 255
for (ypos = edging; ypos < (rheight - edging); ypos++)
	{
	zip = ypos * rwidth;
	aptr = &alf[zip] + edging;
	eptr = &edge[zip] + edging;
	for (xpos = edging; xpos < (rwidth - edging); xpos++, aptr++, eptr++)
		{
		// if there's something in the alpha channel, make the edging channel the complement of the alpha channel
		if (*aptr)
			*eptr = 255 - *aptr;
		} // for xpos
	} // for ypos

//DebugDump();

return 1;	// success

} // FontImage::OutlineText

/*===========================================================================*/

long FontImage::PrintText(ULONG x, ULONG y, char *string, UBYTE r, UBYTE g, UBYTE b, FIJustify justification, FIMode drawmode,
						  char leading, UBYTE kerning, UBYTE edging, PostProcText *PPT)
{
long cx = 0, dx, sx, sy, outzip;
ULONG fx, fy, fzip;
USHORT xpos, ypos;
long pwidth;	// width in pixels for this string
unsigned char *fpixel;
unsigned char *ch = (unsigned char *)string, NeHe;
UBYTE RealR, RealG, RealB;

pwidth = TextWidth(string, kerning);
// bail if somethings wacky
//if (pwidth > (long)rwidth)
//	return 0;
switch (justification)
	{
	case WCS_FONTIMAGE_JUSTIFY_CENTER:
		dx = ((long)rwidth - pwidth) / 2 + 1;
		break;
	case WCS_FONTIMAGE_JUSTIFY_LEFT:
		dx = 1 + edging;
		break;
	case WCS_FONTIMAGE_JUSTIFY_RIGHT:
		dx = (long)rwidth - (pwidth + edging + 1);
		break;
	case WCS_FONTIMAGE_JUSTIFY_NONE:
	default:
		break;
	} // switch

while (*ch != 0)
	{
	/*** F2 NOTE: This conditional probably isn't needed any more ***/
	if (*ch < 32)	// keep from blowing up - we won't print the first 32 control chars
		{
		ch++;
		continue;
		} // if
	NeHe = *ch - 32;	// convert from ASCII to NeHe mappings
	fx = (NeHe % 16) * chwidth;
	fy = (NeHe / 16) * chheight;
	for (ypos = 0; ypos < chheight; ypos++)
		{
		if (justification == WCS_FONTIMAGE_JUSTIFY_NONE)
			{
			sx = x + cx + 1;	// one pixel border around entire "frame"
			sy = y + ypos + 1;
			} // if
		else
			{
			sx = x + cx + dx + 1;
			sy = y + ypos + 1;
			} // else
		// see if this x is plotable
		if ((sx >= 0) && (sx < (long)rwidth))
			{
			outzip = sy * rwidth + sx;
			fzip = (fy + ypos) * fwidth + fx;
			fpixel = &font[fzip];
			fpixel += leftx[NeHe];
			for (xpos = 0; xpos < widthx[NeHe]; xpos++, fpixel++, outzip++)
				{
				RealR = r;
				RealG = g;
				RealB = b;
				if(PPT)
					{
					PPT->FetchPixelColor(xpos, ypos, RealR, RealG, RealB);
					} // if
				// in raster?
				if ((outzip > 0) && (outzip < rmax) && ((sx + xpos) < (long)rwidth))
					{
					if (alf)
						{
						alf[outzip] = *fpixel;
						if (red)
							red[outzip] = RealR;
						if (grn)
							grn[outzip] = RealG;
						if (blu)
							blu[outzip] = RealB;
						} // if
					else
						{
						// may be writing onto existing image
						if (red)
							red[outzip] = (((unsigned int)*fpixel * (unsigned int)RealR) / (unsigned int)255) + (((unsigned int)(255 - *fpixel) * (unsigned int)red[outzip]) / (unsigned int)255);
						if (grn)
							grn[outzip] = (((unsigned int)*fpixel * (unsigned int)RealG) / (unsigned int)255) + (((unsigned int)(255 - *fpixel) * (unsigned int)grn[outzip]) / (unsigned int)255);
						if (blu)
							blu[outzip] = (((unsigned int)*fpixel * (unsigned int)RealB) / (unsigned int)255) + (((unsigned int)(255 - *fpixel) * (unsigned int)blu[outzip]) / (unsigned int)255);
						} // else
					} // if
				} // for x
			} // if
		} // for y
	// advance to next x position
	cx += widthx[NeHe] + kerning + 1;	// 1 pixel kerning is automatic
	ch++;
	} // while

return 1;

} // FontImage::PrintText

/*===========================================================================*/

long FontImage::TextWidth(char *str, UBYTE kerning)
{
unsigned char *ch = (unsigned char *)str;
long width = 0, numchars = 0;
unsigned char NeHe;

while (*ch != 0)
	{
	if (*ch < 32)
		{
		ch++;
		continue;
		}
	NeHe = *ch - 32;
	width += widthx[NeHe];	// add the characters width
	numchars++;
	ch++;
	} // while

if (numchars != 0)
	width += (numchars - 1) * (kerning + 1);	// add kerning between all chars

return width;

} // FontImage::TextWidth

/*===========================================================================*/
/*===========================================================================*/

// convert a ASCII string to a shifted-ASCII string (char + 128)
// used to switch between char sets in FontImage
void NeHeUpper(char *str)
{
unsigned char *ch = (unsigned char *)str;

while (*ch)
	{
	*ch = *ch + 128;
	ch++;
	}

} // NeHeUpper

/*===========================================================================*/

void strcat_NeHeUpper(char *dest, const char *src, bool italics)
{
unsigned char *ch = (unsigned char *)src;

while (*dest)
	dest++;

while (*ch)
	{
	if (italics)
		*dest++ = *ch + 128;
	else
		*dest++ = *ch;
	ch++;
	} // while

*dest = 0;

} // NeHeUpper

/*===========================================================================*/

InternalRasterFont::InternalRasterFont(unsigned char Tiny)
{
void *ResourceLock = NULL;
unsigned char *ResourceBlock = NULL;
uLongf destLen;
uLong sourceLen = 0;
HRSRC  ResHandle = NULL;
WORD FontID;

TinyFont = Tiny;

// Change these if you change the built-in font.

if(Tiny)
	{
	InternalFontWidth = 256;
	InternalFontHeight = 256;
	FontID = IDR_GZFONTDATA2;
	} // if
else
	{
	InternalFontWidth = 1024;
	InternalFontHeight = 1024;
	FontID = IDR_GZFONTDATA1;
	} // else
destLen = InternalFontHeight * InternalFontWidth;

FontDataPointer = NULL;

// load and decompress font into buffer
if(FontDataPointer = (unsigned char *)AppMem_Alloc(InternalFontWidth * InternalFontHeight, APPMEM_CLEAR))
	{
	// fetch compressed data block
	if(ResourceLock = LockResource(LoadResource((HINSTANCE)GlobalApp->WinSys->Instance(),
	 ResHandle = FindResource((HINSTANCE)GlobalApp->WinSys->Instance(), MAKEINTRESOURCE(FontID), "GZFONTDATA"))))
		{
		sourceLen = SizeofResource((HINSTANCE)GlobalApp->WinSys->Instance(), ResHandle);
		ResourceBlock = (unsigned char *)ResourceLock;

		// decompress it
		uncompress(FontDataPointer, &destLen, ResourceBlock, sourceLen);
		// clean up for exit
		ResourceBlock = NULL; // don't access via this either
		FreeResource(ResourceLock);
		ResourceLock = NULL;
		return; // success
		} // if
	AppMem_Free(FontDataPointer, InternalFontWidth * InternalFontHeight);
	FontDataPointer = NULL;
	} // if

} // InternalRasterFont::InternalRasterFont

/*===========================================================================*/

InternalRasterFont::~InternalRasterFont()
{

if(FontDataPointer)
	{ // free font
	AppMem_Free(FontDataPointer, InternalFontWidth * InternalFontHeight);
	FontDataPointer = NULL;
	} // if

} // InternalRasterFont::~InternalRasterFont

/*===========================================================================*/
