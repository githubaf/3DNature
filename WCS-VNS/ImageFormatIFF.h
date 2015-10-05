// ImageFormatIFF.h
// IFF support code
// Built from Bitmaps.h on 12/06/00 by CXH

#include "stdafx.h"

#ifndef WCS_IMAGEFORMATIFF_H
#define WCS_IMAGEFORMATIFF_H

union  MultiByte {
 unsigned char UBt;
 char  Bt;
};

struct CompressData {
 unsigned char  *Data;
 union MultiByte *OutArray;
 FILE *fHandle;
 long	OutSize,
 	Rows,
	RowBytes,
	OutCtr,
	MaxByteRun,
	TotalOutBytes,
	fh;
};

struct ILBMHeader {
	unsigned char ChunkID[4];
	long ChunkSize;
};


// -- if DATATYPES_PICTURECLASS_H has been included, we already have a
// -- perfectly good defn for BitMapHeader, and redefining it pisses off
// -- the compiler, so we`ll just go with the header file version.

# ifndef DATATYPES_PICTURECLASS_H
struct BitMapHeader {
	unsigned short bmh_Width, bmh_Height;
	short bmh_Left, bmh_Top;
	unsigned char bmh_Depth, bmh_Masking, bmh_Compression, Pad;
	unsigned short bmh_Transparent;
	unsigned char bmh_XAspect, bmh_YAspect;
	short bmh_PageWidth, bmh_PageHeight;
};
# endif // DATATYPES_PICTURECLASS_H

struct ZBufferHeader {
	unsigned long Width, Height;
	unsigned short VarType, Compression, Sorting, Units;
	float  Min, Max, Bkgrnd, ScaleFactor, ScaleBase;
};


short CompressRows(struct CompressData *CD);

short FlushOutputBuff(struct CompressData *CD);

short CheckIFF(FILE *fh, struct ILBMHeader *Hdr);

short FindIFFChunk(FILE *fh, struct ILBMHeader *Hdr, char *Chunk);

#endif // WCS_IMAGEFORMATIFF_H
