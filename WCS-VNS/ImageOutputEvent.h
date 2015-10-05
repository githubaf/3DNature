// ImageOutputEvent.h
// header for the output event processor
// Created from scratch on 8/26/99 by Gary R. Huber with ideas from Chris "Xenon" Hanson"
// Copyright 1999 by Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMAGEOUTPUTEVENT_H
#define WCS_IMAGEOUTPUTEVENT_H

#include "PathAndfile.h"
#include "Raster.h"

class BufferNode;	// in Raster.h and Raster.cpp
class Project;
class ImageFormat;
class RasterBounds;

#define WCS_MAX_IMAGEOUTBUFFERS		12
#define WCS_MAX_FILETYPELENGTH		24
#define WCS_MAX_CODECLENGTH			48

// policy is to always append to the end of this list.
// Never move items, reuse slots, or otherwise change
// the order of this list.
enum
	{
	WCS_IMAGEFILEFORMAT_IFF = 0,
	WCS_IMAGEFILEFORMAT_TGA,
	WCS_IMAGEFILEFORMAT_BMP,
	WCS_IMAGEFILEFORMAT_PICT,
	WCS_IMAGEFILEFORMAT_RLA,
	WCS_IMAGEFILEFORMAT_RAW,
	WCS_IMAGEFILEFORMAT_RAWI,
	WCS_IMAGEFILEFORMAT_IFFZBUF,
	WCS_IMAGEFILEFORMAT_ZBGREYIFF,
	WCS_IMAGEFILEFORMAT_ZBFLOAT,
	WCS_IMAGEFILEFORMAT_ZBCHAR,
	WCS_IMAGEFILEFORMAT_AVI,
	WCS_IMAGEFILEFORMAT_QT,
	WCS_IMAGEFILEFORMAT_AI,
	WCS_IMAGEFILEFORMAT_BT,
	WCS_IMAGEFILEFORMAT_RGBE,
	WCS_IMAGEFILEFORMAT_FLX,
	WCS_IMAGEFILEFORMAT_WCSELEV,
	WCS_IMAGEFILEFORMAT_JPEG,
	WCS_IMAGEFILEFORMAT_TIFF,
	WCS_IMAGEFILEFORMAT_PNG,
	WCS_IMAGEFILEFORMAT_ECW,
	WCS_IMAGEFILEFORMAT_ECWDEM,
	WCS_IMAGEFILEFORMAT_MRSID,
	WCS_IMAGEFILEFORMAT_BIL,
	WCS_IMAGEFILEFORMAT_GRIDFLOAT,
	WCS_IMAGEFILEFORMAT_ARCASCII,
	WCS_IMAGEFILEFORMAT_STL,
	WCS_IMAGEFILEFORMAT_SGIRGB,
	WCS_IMAGEFILEFORMAT_JP2,
	WCS_IMAGEFILEFORMAT_JP2DEM,
	WCS_IMAGEFILEFORMAT_LAST_DONT_MOVE
	}; // Image file format types

struct ImageFileFormat
	{
	char *Name, *Extensions, *DefaultExt, *Buffers, *DefaultBuffers, *RequiredBuffers, *Codecs, OptionsAvailable;
	char PlanOnly;
	}; // struct ImageFileType

class ImageSaverLibrary
	{
	private:
		static char NameBuf[WCS_MAX_BUFFERNODE_NAMELEN];

		static long GetFormatIndex(char *Format);
		static char *GetNextEntry(char *Search, char *Current);
		static int MatchEntry(char *Search, char *Current);

	public:
		// Since the only data invoked is in the ImageFileFormat array we might as well
		// make everything static so you don't need to create one of these to use it. 
		// No reason we should need more than one even if we're multi-threaded.
		static char *GetNextFileFormat(char *Current);
		static char *GetNextExtension(char *Format, char *Current);
		static char *GetDefaultExtension(char *Format);
		static char *GetNextBuffer(char *Format, char *Current);
		static char *GetNextDefaultBuffer(char *Format, char *Current);
		static char *GetNextRequiredBuffer(char *Format, char *Current);
		static char *GetNextCodec(char *Format, char *Current);
		static char AdvancedOptionsAvailable(char *Current);
		static int GetIsBufferDefault(char *Format, char *Current);
		static int GetIsBufferRequired(char *Format, char *Current);
		static char *GetFormatFromExtension(char *Extension);
		static char GetPlanOnly(char *Format);
		static void StripImageExtension(char *FileName);

	}; // class ImageSaverLibrary

class ImageOutputEvent
	{
	private:

	public:
		PathAndFile PAF;
		char Enabled, AutoExtension, AutoDigits, BeforePost, FileType[WCS_MAX_FILETYPELENGTH], Codec[WCS_MAX_CODECLENGTH], 
			OutBuffers[WCS_MAX_IMAGEOUTBUFFERS][WCS_MAX_BUFFERNODE_NAMELEN], BufType[WCS_MAX_IMAGEOUTBUFFERS],
			CompleteOutputPath[1024];
		BufferNode *BufNodes[WCS_MAX_IMAGEOUTBUFFERS];

		ImageOutputEvent *Next;

		ImageFormat *Format;
		RenderOpt *SaveOpts;

		ImageOutputEvent();
		~ImageOutputEvent();
		void Copy(ImageOutputEvent *CopyTo, ImageOutputEvent *CopyFrom);
		ImageOutputEvent *SaveBufferQuery(char *QueryStr);
		ImageOutputEvent *SaveEnabledBufferQuery(char *QueryStr);
		int InitSequence(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight);
		int StartVectorFrame(long BufWidth, long BufHeight, long Frame);
		int EndVectorFrame(void);
		int SaveImage(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame, RenderOpt *Options);
		int EndSequence(void);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		char *PrepCompleteOutputPath(long Frame);
		char *PrepCompleteOutputPathSuffix(long Frame, char *Suffix);
		char *GetCompleteOutputPath(void) {return(CompleteOutputPath);};
		void SetDataRange(float MaxValue, float MinValue);

	}; // class ImageOutputEvent

#define WCS_PARAM_DONE				0xffff0000

#define WCS_IMAGEOUTPUTEVENT_FILETYPE		0x00210000
#define WCS_IMAGEOUTPUTEVENT_CODEC			0x00220000
#define WCS_IMAGEOUTPUTEVENT_ENABLED		0x00230000
#define WCS_IMAGEOUTPUTEVENT_AUTOEXTENSION	0x00240000
#define WCS_IMAGEOUTPUTEVENT_IMAGEOUTBUFFER	0x00250000
#define WCS_IMAGEOUTPUTEVENT_AUTODIGITS		0x00260000
#define WCS_IMAGEOUTPUTEVENT_BEFOREPOST		0x00270000
#define WCS_IMAGEOUTPUTEVENT_FILENAME		0x00310000

#endif // WCS_IMAGEOUTPUTEVENT_H
