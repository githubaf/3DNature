// ImageOutputEvent.cpp
// Code for the output event processor
// Created from scratch on 8/26/99 by Gary R. Huber with ideas from Chris "Xenon" Hanson"
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Raster.h"
#include "Useful.h"
#include "Requester.h"
#include "ImageFormat.h"
#include "Script.h"
#include "FeatureConfig.h"

char ImageSaverLibrary::NameBuf[WCS_MAX_BUFFERNODE_NAMELEN];

// Raster.cpp has list of available buffer types
struct ImageFileFormat FileFormats[] =
	{
	{"IFF", ".iff\1.iff24\1.iff8", ".iff", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Run length compress\1Uncompressed", 0, 0},
	{"Targa", ".tga", ".tga", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
	{"BMP", ".bmp", ".bmp", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "No World File\1With BMP World File (.bpw)", 0, 0},
#else // !WCS_BUILD_WORLDFILE_SUPPORT
	{"BMP", ".bmp", ".bmp", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#endif // !WCS_BUILD_WORLDFILE_SUPPORT
	{"PICT", ".pct\1.pic\1.pict", ".pct", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
	{"RLA", ".rla", ".rla", "RED\1GREEN\1BLUE\1ANTIALIAS\1ZBUF\1OBJECT\1SURFACE NORMAL X\1SURFACE NORMAL Y\1SURFACE NORMAL Z\1LATITUDE\1LONGITUDE", "RED\1GREEN\1BLUE\1ANTIALIAS\1ZBUF", "RED\1GREEN\1BLUE", "", 0, 0},
	{"Raw", ".raw", ".raw", "RED\1GREEN\1BLUE\1ANTIALIAS\1ZBUF\1ELEVATION\1RELATIVE ELEVATION\1SLOPE\1ASPECT\1ILLUMINATION\1LATITUDE\1LONGITUDE\1OBJECT\1REFLECTION", "RED\1GREEN\1BLUE\1ZBUF", "", "Full Channel Precision\1Scale to 8-bit", 0, 0},
	{"Raw Interleaved", ".raw", ".raw", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
	{"IFF ZBUF", ".zb", ".zb", "ZBUF", "ZBUF", "ZBUF", "Uncompressed\1ZLIB Compressed", 0, 0},
	{"Z-Buffer Gray IFF", "_gzb.iff", "_gzb.iff", "ZBUF", "ZBUF", "ZBUF", "", 0, 0},
	{"Illustrator", ".ai", ".ai", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Linework only\1Linework linked to next saver", 0, 0},
	{"BinaryTerrain", ".bt", ".bt", "ELEVATION", "ELEVATION", "ELEVATION", "", 0, 0},
	{"WCS DEM (.elev)", ".elev", ".elev", "ELEVATION", "ELEVATION", "ELEVATION", "", 0, 1},
#ifdef WCS_BUILD_HDR_SUPPORT
	{"Radiance RGBE HDR", ".hdr", ".hdr", "RED\1GREEN\1BLUE\1RGB EXPONENT", "RED\1GREEN\1BLUE\1RGB EXPONENT", "RED\1GREEN\1BLUE\1RGB EXPONENT", "", 0, 0},
#endif // WCS_BUILD_HDR_SUPPORT
#ifdef WCS_BUILD_JPEG_SUPPORT
	{"JPEG", ".jpg\1.jpeg", ".jpg", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#endif // WCS_BUILD_JPEG_SUPPORT
#ifdef WCS_BUILD_TIFF_SUPPORT
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
	{"TIFF", ".tif\1.tiff", ".tif", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "No World File\1With TIFF World File (.tfw)", 0, 0},
#else // !WCS_BUILD_WORLDFILE_SUPPORT
	{"TIFF", ".tif\1.tiff", ".tif", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#endif // !WCS_BUILD_WORLDFILE_SUPPORT
#endif // WCS_BUILD_TIFF_SUPPORT

#ifdef WCS_BUILD_ECW_SUPPORT
	{"ERMapper ECW", ".ecw", ".ecw", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#endif // WCS_BUILD_ECW_SUPPORT
#if defined WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_SX2
	{"ERMapper ECW DEM", ".ecw", ".ecw", "ELEVATION", "ELEVATION", "ELEVATION", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#endif // WCS_BUILD_ECW_SUPPORT && WCS_BUILD_SX2
#ifdef WCS_BUILD_ECW_SUPPORT
#ifdef WCS_BUILD_JP2_SUPPORT // we get JP2 from ECW 3.x and later
	{"JPEG 2000", ".jp2", ".jp2", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
	{"Lossless JPEG 2000", ".jp2", ".jp2", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#ifdef WCS_BUILD_SX2
	{"JPEG 2000 DEM", ".jp2", ".jp2", "ELEVATION", "ELEVATION", "ELEVATION", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
	{"Lossless JPEG 2000 DEM", ".jp2", ".jp2", "ELEVATION", "ELEVATION", "ELEVATION", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#endif // WCS_BUILD_SX2
#endif // WCS_BUILD_JP2_SUPPORT
#endif // WCS_BUILD_ECW_SUPPORT
#ifdef WCS_BUILD_MRSID__WRITE_SUPPORT
	{"MrSID", ".sid", ".sid", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Maximum Quality\1High Quality\1Med Quality\1Low Quality", 0, 0},
#endif // WCS_BUILD_MRSID__WRITE_SUPPORT


#ifdef WCS_BUILD_PNG_SUPPORT
	{"PNG", ".png", ".png", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "Compress Less\1Compress Normal\1Compress More\1Compress Max", 0, 0},
#endif // WCS_BUILD_PNG_SUPPORT
#ifdef WCS_BUILD_AVI_SUPPORT
	{"AVI", ".avi", ".avi", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#endif // WCS_BUILD_AVI_SUPPORT
#ifdef WCS_BUILD_QUICKTIME_SUPPORT
	{"Quicktime MOV", ".mov", ".mov", "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#endif // WCS_BUILD_QUICKTIME_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATBIL_SUPPORT
	{"Arc GRID BIL", ".bil", ".bil", "ELEVATION", "ELEVATION", "ELEVATION", "", 0, 1},
#endif // WCS_BUILD_IMAGEFORMATBIL_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT
	{"Arc GRIDFLOAT", ".flt", ".flt", "ELEVATION", "ELEVATION", "ELEVATION", "", 0, 1},
#endif // WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT
	{"Arc ASCII DEM", ".asc", ".asc", "ELEVATION", "ELEVATION", "ELEVATION", "", 0, 1},
#endif // WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT

//#ifdef WCS_BUILD_IMAGEFORMATSTL_SUPPORT
//	{"Stereolithography", ".stl", ".stl", "ELEVATION", "ELEVATION", "ELEVATION", "ASCII\1Binary", 0, 1},
//#endif // WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT

#ifdef WCS_BUILD_SGIRGB_SUPPORT
	{"SGI RGB", ".rgb\1.rgba\1.sgi", ".rgb",  "RED\1GREEN\1BLUE\1ANTIALIAS", "RED\1GREEN\1BLUE", "RED\1GREEN\1BLUE", "", 0, 0},
#endif // WCS_BUILD_SGIRGB_SUPPORT

	{"", "", "", "", "", "", "", 0, 0}
	}; // ImageSaverLibrary::FileFormats

long ImageSaverLibrary::GetFormatIndex(char *Format)
{
long Index;

if (Format)
	{
	// find name match
	for (Index = 0; FileFormats[Index].Name[0]; Index ++)
		{
		if (! stricmp(Format, FileFormats[Index].Name))
			{
			return (Index);
			} // if found match
		} // for
	// no match found
	return (-1);
	} // if

// return first entry
return (0);

} // ImageSaverLibrary::GetFormatIndex

/*===========================================================================*/

char *ImageSaverLibrary::GetNextFileFormat(char *Format)
{
long Index;

if (! Format)
	{
	return (FileFormats[0].Name);
	} // if

// search for match and return next one
if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index + 1].Name[0])
		return (FileFormats[Index + 1].Name);
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextFileFormat

/*===========================================================================*/

char *ImageSaverLibrary::GetNextExtension(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index].Extensions)
		{
		return (GetNextEntry(FileFormats[Index].Extensions, Current));
		} // if
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextExtension

/*===========================================================================*/

char *ImageSaverLibrary::GetDefaultExtension(char *Format)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	return (FileFormats[Index].DefaultExt);
	} // if

return (""); // returning NULL is very dangerous to those that strcat it blindly

} // ImageSaverLibrary::GetDefaultExtension

/*===========================================================================*/

char ImageSaverLibrary::GetPlanOnly(char *Format)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	return (FileFormats[Index].PlanOnly);
	} // if

return (NULL);

} // ImageSaverLibrary::GetPlanOnly

/*===========================================================================*/

char *ImageSaverLibrary::GetNextBuffer(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index].Buffers)
		{
		return (GetNextEntry(FileFormats[Index].Buffers, Current));
		} // if
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextBuffer

/*===========================================================================*/

char *ImageSaverLibrary::GetNextDefaultBuffer(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index].DefaultBuffers)
		{
		return (GetNextEntry(FileFormats[Index].DefaultBuffers, Current));
		} // if
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextDefaultBuffer

/*===========================================================================*/

char *ImageSaverLibrary::GetNextRequiredBuffer(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index].RequiredBuffers)
		{
		return (GetNextEntry(FileFormats[Index].RequiredBuffers, Current));
		} // if
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextRequiredBuffer

/*===========================================================================*/

char *ImageSaverLibrary::GetNextCodec(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	if (FileFormats[Index].Codecs)
		{
		return (GetNextEntry(FileFormats[Index].Codecs, Current));
		} // if
	} // if

return (NULL);

} // ImageSaverLibrary::GetNextCodec

/*===========================================================================*/

char ImageSaverLibrary::AdvancedOptionsAvailable(char *Format)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	return (FileFormats[Index].OptionsAvailable);
	} // if

return (NULL);

} // ImageSaverLibrary::AdvancedOptionsAvailable

/*===========================================================================*/

int ImageSaverLibrary::GetIsBufferDefault(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	return (MatchEntry(FileFormats[Index].DefaultBuffers, Current));
	} // if

return (0);

} // ImageSaverLibrary::GetIsBufferDefault

/*===========================================================================*/

int ImageSaverLibrary::GetIsBufferRequired(char *Format, char *Current)
{
long Index;

if ((Index = GetFormatIndex(Format)) >= 0)
	{
	return (MatchEntry(FileFormats[Index].RequiredBuffers, Current));
	} // if

return (0);

} // ImageSaverLibrary::GetIsBufferRequired

/*===========================================================================*/

char *ImageSaverLibrary::GetFormatFromExtension(char *Extension)
{
char *Format = NULL, *TestExtension, HasDot;

HasDot = Extension[0] == '.';

while (Format = GetNextFileFormat(Format))
	{
	TestExtension = NULL;
	while (TestExtension = GetNextExtension(Format, TestExtension))
		{
		if (HasDot)
			{
			if (! stricmp(Extension, TestExtension))
				return (Format);
			} // if
		else
			{
			if (! stricmp(Extension, &TestExtension[1]))
				return (Format);
			} // else
		} // while
	} // while

return (NULL);

} // ImageSaverLibrary::GetFormatFromExtension

/*===========================================================================*/

char *ImageSaverLibrary::GetNextEntry(char *Search, char *Current)
{
char *FirstChar, *FoundOne, FoundMatch;

// searches for 1-delimited string segments

FoundMatch = (Current ? 0: 1);

for (FirstChar = FoundOne = Search; ; FoundOne ++)
	{
	if (*FoundOne == 1 || *FoundOne == 0)
		{
		if (FoundOne != FirstChar)
			{
			if (FoundMatch)
				{
				strncpy(NameBuf, FirstChar, FoundOne - FirstChar);
				NameBuf[FoundOne - FirstChar] = 0;
				return (NameBuf);
				} // if
			if (! strnicmp(FirstChar, Current, FoundOne - FirstChar) && strlen(Current) == (unsigned int)(FoundOne - FirstChar))
				{
				FoundMatch = 1;
				} // if
			FirstChar = FoundOne + 1;
			} // if
		else
			break;
		} // if found a delimiter
	if (! *FoundOne)
		break;
	} // for

// failed to find another
return (NULL);

} // ImageSaverLibrary::GetNextEntry

/*===========================================================================*/

int ImageSaverLibrary::MatchEntry(char *Search, char *Current)
{
char *FirstChar, *FoundOne;

// searches for 1-delimited string segments

for (FirstChar = FoundOne = Search; ; FoundOne ++)
	{
	if (*FoundOne == 1 || *FoundOne == 0)
		{
		if (FoundOne != FirstChar)
			{
			if (! strnicmp(FirstChar, Current, FoundOne - FirstChar) && strlen(Current) == (unsigned int)(FoundOne - FirstChar))
				{
				return (1);
				} // if
			FirstChar = FoundOne + 1;
			} // if
		else
			break;
		} // if found a delimiter
	if (! *FoundOne)
		break;
	} // for

// failed to find match
return (0);

} // ImageSaverLibrary::MatchEntry

/*===========================================================================*/

void ImageSaverLibrary::StripImageExtension(char *FileName)
{
char *Format = NULL, *TestExtension, *Extension;

if (Extension = FindFileExtension(FileName))
	{
	while (Format = GetNextFileFormat(Format))
		{
		TestExtension = NULL;
		while (TestExtension = GetNextExtension(Format, TestExtension))
			{
			if (! stricmp(Extension, &TestExtension[1]))
				StripExtension(FileName);
			} // while
		} // while
	} // if

} // ImageSaverLibrary::StripImageExtension

/*===========================================================================*/
/*===========================================================================*/

ImageOutputEvent::ImageOutputEvent()
{
long Ct;

Enabled = AutoExtension = 1;
FileType[0] = Codec[0] = 0;
Format = 0;
SaveOpts = NULL;
AutoDigits = 5;
BeforePost = 0;

for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
	{
	OutBuffers[Ct][0] = 0;
	BufNodes[Ct] = NULL;
	} // for

Next = NULL;

} // ImageOutputEvent::ImageOutputEvent

/*===========================================================================*/

ImageOutputEvent::~ImageOutputEvent()
{

if(Format)
	{
	delete Format;
	Format = NULL;
	} // if

} // ImageOutputEvent::~ImageOutputEvent

/*===========================================================================*/

void ImageOutputEvent::Copy(ImageOutputEvent *CopyTo, ImageOutputEvent *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
	strcpy(CopyTo->OutBuffers[Ct], CopyFrom->OutBuffers[Ct]);

strcpy(CopyTo->FileType, CopyFrom->FileType);
strcpy(CopyTo->Codec, CopyFrom->Codec);

PAF.Copy(&CopyTo->PAF, &CopyFrom->PAF);
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->AutoExtension = CopyFrom->AutoExtension;
CopyTo->Format = CopyFrom->Format;
CopyTo->SaveOpts = CopyFrom->SaveOpts;
CopyTo->AutoDigits = CopyFrom->AutoDigits;
CopyTo->BeforePost = CopyFrom->BeforePost;

} // ImageOutputEvent::Copy

/*===========================================================================*/

unsigned long int ImageOutputEvent::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
short BufferNumber = 0, MaxBuffers = WCS_MAX_IMAGEOUTBUFFERS;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					default:
						break;
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_IMAGEOUTPUTEVENT_FILETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)FileType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_CODEC:
						{
						BytesRead = ReadBlock(ffile, (char *)Codec, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_AUTOEXTENSION:
						{
						BytesRead = ReadBlock(ffile, (char *)&AutoExtension, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_AUTODIGITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&AutoDigits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_BEFOREPOST:
						{
						BytesRead = ReadBlock(ffile, (char *)&BeforePost, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_IMAGEOUTBUFFER:
						{
						if (BufferNumber <= MaxBuffers)
							BytesRead = ReadBlock(ffile, (char *)OutBuffers[BufferNumber ++], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_IMAGEOUTPUTEVENT_FILENAME:
						{
						BytesRead = PAF.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // ImageOutputEvent::Load

/*===========================================================================*/

unsigned long int ImageOutputEvent::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_FILETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FileType) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FileType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_CODEC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Codec) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Codec)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_AUTOEXTENSION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AutoExtension)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_AUTODIGITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AutoDigits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_BEFOREPOST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BeforePost)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGEOUTPUTEVENT_IMAGEOUTBUFFER, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(OutBuffers[Ct]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)OutBuffers[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

ItemTag = WCS_IMAGEOUTPUTEVENT_FILENAME + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PAF.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if file path saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // ImageOutputEvent::Save

/*===========================================================================*/

ImageOutputEvent *ImageOutputEvent::SaveBufferQuery(char *QueryStr)
{
int Ct;
ImageOutputEvent *CurEvent = this;

while (CurEvent)
	{
	for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
		{
		if (! strnicmp(CurEvent->OutBuffers[Ct], QueryStr, WCS_MAX_BUFFERNODE_NAMELEN))
			return (CurEvent);
		// exponent buffer should be saved whenever RGB is saved
		if (! strnicmp(QueryStr, "RGB EXPONENT", WCS_MAX_BUFFERNODE_NAMELEN))
			{
			if (! strnicmp(CurEvent->OutBuffers[Ct], "RED", WCS_MAX_BUFFERNODE_NAMELEN))
				return (CurEvent);
			if (! strnicmp(CurEvent->OutBuffers[Ct], "GREEN", WCS_MAX_BUFFERNODE_NAMELEN))
				return (CurEvent);
			if (! strnicmp(CurEvent->OutBuffers[Ct], "BLUE", WCS_MAX_BUFFERNODE_NAMELEN))
				return (CurEvent);
			} // if
		} // for
	CurEvent = CurEvent->Next;
	} // while

return (NULL);

} // ImageOutputEvent::SaveBufferQuery

/*===========================================================================*/

// find out which channels are enabled for a file type
ImageOutputEvent *ImageOutputEvent::SaveEnabledBufferQuery(char *QueryStr)
{
int Ct;

for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
	{
	if (! strnicmp(OutBuffers[Ct], QueryStr, WCS_MAX_BUFFERNODE_NAMELEN))
		return (this);
	} // for

return (NULL);

} // ImageOutputEvent::SaveEnabledBufferQuery

/*===========================================================================*/

char *ImageOutputEvent::PrepCompleteOutputPath(long Frame)
{
if(Format)
	{
	// Create a filename
	PAF.GetFramePathAndName(CompleteOutputPath, AutoExtension ? ImageSaverLibrary::GetDefaultExtension(FileType) : NULL, Frame, 1000, AutoDigits);
	return(CompleteOutputPath);
	} // if
return (NULL);

} // ImageOutputEvent::PrepCompleteOutputPath

/*===========================================================================*/

char *ImageOutputEvent::PrepCompleteOutputPathSuffix(long Frame, char *Suffix)
{
if(Format)
	{
	// Create a filename
	PAF.GetFramePathAndName(CompleteOutputPath, Suffix, Frame, 1000, AutoDigits);
	return(CompleteOutputPath);
	} // if
return (NULL);

} // ImageOutputEvent::PrepCompleteOutputPathSuffix

/*===========================================================================*/

// allocates ImageFormat or returns 0 for failure to find the correct file type
int ImageOutputEvent::InitSequence(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight)
{
// Allocate the format-specific objects here
if (! stricmp(FileType, "IFF"))
	{
	Format = new ImageFormatIFF;
	} // TAGRA
else if (! stricmp(FileType, "Targa"))
	{
	Format = new ImageFormatTGA;
	} // TGA
else if (! stricmp(FileType, "BMP"))
	{
	Format = new ImageFormatBMP;
	} // BMP
//else if (! stricmp(FileType, "Radiance RGBE HDR"))
#ifdef WCS_BUILD_HDR_SUPPORT
else if (! strnicmp(FileType, "Radiance", strlen("Radiance")))
	{
	Format = new ImageFormatRGBE;
	} // BMP
else if (! stricmp(FileType, "FLX"))
	{
	Format = new ImageFormatFLX;
	} // BMP
#endif // WCS_BUILD_HDR_SUPPORT

#ifdef WCS_BUILD_SGIRGB_SUPPORT
else if (! stricmp(FileType, "SGI RGB"))
	{
	Format = new ImageFormatSGIRGB;
	} // SGIRGB
#endif // WCS_BUILD_SGIRGB_SUPPORT

#ifdef WCS_BUILD_ECW_SUPPORT
else if (! stricmp(FileType, "ERMapper ECW"))
	{
	Format = new ImageFormatECW;
	} // ECW
#endif // WCS_BUILD_ECW_SUPPORT

#ifdef WCS_BUILD_ECW_SUPPORT
#ifdef WCS_BUILD_JP2_SUPPORT // we get JPEG2000 from ECW 3.x and later
else if (! stricmp(FileType, "JPEG 2000"))
	{
	Format = new ImageFormatECW;
	strcpy(Codec, "High Quality");
	} // JP2
else if (! stricmp(FileType, "Lossless JPEG 2000"))
	{
	Format = new ImageFormatECW;
	strcpy(Codec, "Lossless");
	} // JP2

#if defined WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_SX2 && defined WCS_BUILD_ECWDEM_SUPPORT
else if (! stricmp(FileType, "JPEG 2000 DEM"))
	{
	Format = new ImageFormatECWDEM;
	((ImageFormatECWDEM *)Format)->SetJP2(1);
	strcpy(Codec, "High Quality");
	} // JP2DEM
else if (! stricmp(FileType, "Lossless JPEG 2000 DEM"))
	{
	Format = new ImageFormatECWDEM;
	((ImageFormatECWDEM *)Format)->SetJP2(1);
	strcpy(Codec, "Lossless");
	} // JP2DEM
#endif // WCS_BUILD_ECW_SUPPORT && WCS_BUILD_SX2 && WCS_BUILD_ECWDEM_SUPPORT


#endif // WCS_BUILD_JP2_SUPPORT
#endif // WCS_BUILD_ECW_SUPPORT

#if defined WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_SX2 && defined WCS_BUILD_ECWDEM_SUPPORT
else if (! stricmp(FileType, "ERMapper ECW DEM"))
	{
	Format = new ImageFormatECWDEM;
	} // ECWDEM
#endif // WCS_BUILD_ECW_SUPPORT && WCS_BUILD_SX2 && WCS_BUILD_ECWDEM_SUPPORT

#ifdef WCS_BUILD_JPEG_SUPPORT
else if (! stricmp(FileType, "JPEG"))
	{
	Format = new ImageFormatJPEG;
	} // JPEG
#endif // WCS_BUILD_JPEG_SUPPORT

#ifdef WCS_BUILD_TIFF_SUPPORT
else if (! stricmp(FileType, "TIFF"))
	{
	Format = new ImageFormatTIFF;
	} // TIFF
#endif // WCS_BUILD_TIFF_SUPPORT

#ifdef WCS_BUILD_PNG_SUPPORT
else if (! stricmp(FileType, "PNG"))
	{
	Format = new ImageFormatPNG;
	} // PNG
#endif // WCS_BUILD_PNG_SUPPORT

#ifdef WCS_BUILD_AVI_SUPPORT
else if (! stricmp(FileType, "AVI"))
	{
	Format = new ImageFormatAVI;
	} // AVI
#endif // WCS_BUILD_AVI_SUPPORT

#ifdef WCS_BUILD_QUICKTIME_SUPPORT
else if (! stricmp(FileType, "Quicktime MOV"))
	{
	Format = new ImageFormatQT;
	} // Quicktime
#endif // WCS_BUILD_QUICKTIME_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATBIL_SUPPORT
else if (! stricmp(FileType, "Arc GRID BIL"))
	{
	Format = new ImageFormatBIL;
	} // BT
#endif // WCS_BUILD_IMAGEFORMATBIL_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT
else if (! stricmp(FileType, "Arc GRIDFLOAT"))
	{
	Format = new ImageFormatGRIDFLOAT;
	} // BT
#endif // WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT
else if (! stricmp(FileType, "Arc ASCII DEM"))
	{
	Format = new ImageFormatARCASCII;
	} // BT
#endif // WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT

//#ifdef WCS_BUILD_IMAGEFORMATSTL_SUPPORT
//else if (! stricmp(FileType, "Stereolithography"))
//	{
//	Format = new ImageFormatSTL;
//	} // BT
//#endif // WCS_BUILD_IMAGEFORMATSTL_SUPPORT

else if (! stricmp(FileType, "BinaryTerrain"))
	{
	Format = new ImageFormatBT;
	} // BT
else if (! strnicmp(FileType, "WCS DEM", strlen("WCS DEM")))
	{
	Format = new ImageFormatWCSELEV;
	} // WCS ELEV
else if (! stricmp(FileType, "PICT"))
	{
	Format = new ImageFormatPICT;
	} // PICT
else if (! stricmp(FileType, "Illustrator"))
	{
	Format = new ImageFormatAI;
	} // PICT
else if (! stricmp(FileType, "RLA"))
	{
	Format = new ImageFormatRLA;
	} // RLA
else if (! stricmp(FileType, "Raw"))
	{
	if(Format = new ImageFormatRAW)
		{
		if(!strcmp(Codec, "Scale to 8-bit"))
			{
			((ImageFormatRAW *)Format)->SetForceByte(1);
			} // if
		else
			{
			((ImageFormatRAW *)Format)->SetForceByte(0);
			} // else
		} // if
	} // 
else if (! stricmp(FileType, "Raw Interleaved"))
	{
	if(Format = new ImageFormatRAW)
		{
		((ImageFormatRAW *)Format)->SetInterleave(1);
		} // if
	} // 
else if (! stricmp(FileType, "IFF ZBUF"))
	{
	Format = new ImageFormatZBUF;
	} // 
else if (! stricmp(FileType, "Z-Buffer Gray IFF"))
	{
	if(Format = new ImageFormatIFF)
		{
		((ImageFormatIFF *)Format)->SetZAsGrey(1);
		} // if
	} // else if



/*
else if (! stricmp(FileType, "XXX"))
	{
	Format = new ImageFormatXXX;
	} // 
*/

if(Format)
	{
	Format->SetIOE(this);
	// we need to check saveopts here because it's null when we're
	// called at the end of a render to write WCSLastRender.iff
	// we can live without the framerate/animtime vals there.
	if(SaveOpts)
		{
		Format->StartAnim(RBounds, Buffers, BufWidth, BufHeight,
		 SaveOpts->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].GetCurValue(0),
		 SaveOpts->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].GetCurValue(0) -
		  SaveOpts->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].GetCurValue(0));
		} // if
	else
		{ // must be saving WCSLastRender.iff
		Format->StartAnim(RBounds, Buffers, BufWidth, BufHeight, 0.0, 0.0);
		} // else
	return (1);
	} // if

return(0);
} // ImageOutputEvent::InitSequence

/*===========================================================================*/

int ImageOutputEvent::EndSequence(void)
{

if(Format)
	{
	Format->EndAnim();
	// free the format-specific stuff here.
	delete Format;
	Format = NULL;
	} // if

return(0);
} // ImageOutputEvent::EndSequence

/*===========================================================================*/

// Before calling this be sure to call PrepToSave on each BufferNode that will be needed.
// Buffers is a pointer to a chain of Buffers some of which contain the image data needed by the savers.
// BufWidth and Height are in pixel elements. Frame will be needed to create a file name.
int ImageOutputEvent::SaveImage(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame, RenderOpt *Options)
{
int Success = 1;

SaveOpts = Options;

if(Format)
	{
	// Create a filename
	PAF.GetFramePathAndName(CompleteOutputPath, AutoExtension ? ImageSaverLibrary::GetDefaultExtension(FileType) : NULL, Frame, 1000, AutoDigits);
	Success = !(Format->StartFrame(RBounds, Buffers, BufWidth, BufHeight, Frame));
	if(Success)
		{
		GlobalApp->SuperScript->NetSendString("OUTPUT=\"");
		GlobalApp->SuperScript->NetSendString(Format->GetCompleteOutputPath());
		GlobalApp->SuperScript->NetSendString("\"\r\n");
		} // if
	} // if

SaveOpts = NULL;

return (Success);

} // ImageOutputEvent::SaveImage

/*===========================================================================*/
// this is called only once per frame no matter how many segments or whatever are rendered
int ImageOutputEvent::StartVectorFrame(long BufWidth, long BufHeight, long Frame)
{
int Success = 1;

if(Format)
	{
	// Create a filename
	PAF.GetFramePathAndName(CompleteOutputPath, AutoExtension ? ImageSaverLibrary::GetDefaultExtension(FileType) : NULL, Frame, 1000, AutoDigits);
	Success = !(Format->StartVectorFrame(BufWidth, BufHeight, Frame));
	} // if

return (Success);

} // ImageOutputEvent::StartVectorFrame

/*===========================================================================*/
// this is called only once per frame
int ImageOutputEvent::EndVectorFrame(void)
{
int Success = 1;

if(Format)
	{
	Success = !(Format->EndVectorFrame());
	} // if

return (Success);

} // ImageOutputEvent::EndVectorFrame

/*===========================================================================*/

void ImageOutputEvent::SetDataRange(float MaxValue, float MinValue)
{

if (Format)
	Format->SetDataRange(MaxValue, MinValue);

} // ImageOutputEvent::SetDataRange
