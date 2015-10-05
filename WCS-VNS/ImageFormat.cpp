// ImageFormat.cpp
// output image formats
// Created from scratch on 11/29/99 by Chris "Xenon" Hanson"
// Copyright 1999 by 3DNature. All rights reserved.

#include "stdafx.h"
#include "ImageFormat.h"
#include "Useful.h"
#include "Project.h"
#include "ImageOutputEvent.h"
#include "Application.h"
#include "requester.h"
#include "AppMem.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "RLA.h"
#include "ImageFormatIFF.h"
#include "Illustrator.h"
#include "Exports.h"
#include "DEM.h"
#include "WCSVersion.h"
#include "PixelManager.h"
#include "zlib.h"
#include "EXIF.h"

extern EXIFtool gEXIF;

#ifdef WCS_BUILD_JPEG_SUPPORT
#include <setjmp.h>
extern "C" {
#include "jpeglib.h"
} // extern C
#endif // WCS_BUILD_JPEG_SUPPORT

#ifdef WCS_BUILD_PNG_SUPPORT
extern "C" {
#include "png.h"
} // extern C
#endif // WCS_BUILD_PNG_SUPPORT

#ifdef WCS_BUILD_TIFF_SUPPORT
extern "C" {
#include "tiffio.h"
} // extern C
#include "geotiff.h"
#include "xtiffio.h"
#ifdef WCS_BUILD_GEOTIFF_SUPPORT
#include "geo_normalize.h"
#include "geovalues.h"
#endif // WCS_BUILD_GEOTIFF_SUPPORT
#endif // WCS_BUILD_TIFF_SUPPORT

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
char WorldFileName[1024];
#endif // WCS_BUILD_WORLDFILE_SUPPORT

/*===========================================================================*/

void ImageFormat::ConstructClear(void)
{
int Clear;

fHandle = NULL;
TransferBuf = NULL;
CompletePath = NULL;

for (Clear = 0; Clear < WCS_IMAGE_FORMAT_CHAR_CHANNELS_MAX; Clear++)
	{
	CharChannelNode[Clear] = NULL;
	CharChannelData[Clear] = NULL;
	} // for

for (Clear = 0; Clear < WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX; Clear++)
	{
	FloatChannelNode[Clear] = NULL;
	FloatChannelData[Clear] = NULL;
	} // for

for (Clear = 0; Clear < WCS_IMAGE_FORMAT_SHORT_CHANNELS_MAX; Clear++)
	{
	ShortChannelNode[Clear] = NULL;
	ShortChannelData[Clear] = NULL;
	} // for

} // ImageFormat::ConstructClear

/*===========================================================================*/

ImageFormat::ImageFormat()
{
ConstructClear();
} // ImageFormat::ImageFormat

/*===========================================================================*/

ImageFormat::~ImageFormat()
{
if (fHandle) fclose(fHandle);
fHandle = NULL;
} // ImageFormat::~ImageFormat()

/*===========================================================================*/

void ImageFormat::SetCompleteOutputPath(char *OutPath)
{
CompletePath = OutPath;
} // ImageFormat::SetCompleteOutputPath

/*===========================================================================*/

char *ImageFormat::GetCompleteOutputPath(void)
{
if (IOE)
	{
	return(IOE->GetCompleteOutputPath());
	} // if
else
	{
	return(CompletePath);
	} // else
} // ImageFormat::GetCompleteOutputPath

char *ImageFormat::GetName(void)
{
if (IOE)
	{
	return((char *)IOE->PAF.GetName());
	} // if
else
	{
	return(CompletePath);
	} // else
} // ImageFormat::GetName

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatIFF::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
unsigned char *BuffPtr;
unsigned char *ZEight = NULL;
long BMHDsize = 20, BODYsize = 0, FORMsize, FormSizePtr, BodySizePtr, TransferBufferSize, xscan, scanrow, PixCt;
struct CompressData CD;
short DummyZeroShort = 0, aspect, width = 0, height = 0, pp, scrRowBytes = 0, color, pixel;
unsigned char AllIsWell = 0, DummyZeroChar = 0, nplanes = 0, compression = 1, *scrRow, OneColor, power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};
char tt[5], Channels = 0;
//unsigned short Padded = 0;

CD.OutArray = NULL;

if (IOE)
	{
	if (!strcmp(IOE->Codec, "Run length compress"))
		{
		compression = 1;
		} // if
	else
		{
		compression = 0;
		} // else
	} // if

if (ZAsGrey)
	{
	if (FloatChannelNode[0] = Buffers->FindBufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT))
		{
		if (ZEight = (unsigned char *)AppMem_Alloc(BufWidth, NULL))
			{
			// Get extrema for float channels
			FCDMax[0] = -FLT_MAX;
			FCDMin[0] = FLT_MAX;
			AllIsWell = 1;
			Channels = 1;
			for (scanrow = 0; AllIsWell && (scanrow < BufHeight); scanrow++)
				{
				if (FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT))
					{
					for (xscan = 0; xscan < BufWidth; xscan++)
						{
						float FVal = FloatChannelData[0][xscan];
						if (FVal != FLT_MAX)
							{
							if (FVal > FCDMax[0]) FCDMax[0] = FVal;
							if (FVal < FCDMin[0]) FCDMin[0] = FVal;
							} // if
						} // for
					} // if
				else
					{
					AllIsWell = 0;
					break;
					} // else
				} // for
			} // if
		} // if
	} // if
else
	{
	// we know we'll need RGB bands so look for them specifically
	if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
		CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
	else
		CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
	if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
		CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
	else
		CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
	if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
		CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
	else
		CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
	ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
	ShortChannelData[0] = NULL;
	if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
		{
		AllIsWell = 1;
		Channels = 3;
		} // if
	else if (CharChannelNode[0])
		{
		AllIsWell = 1;
		Channels = 1;
		} // else
	} // else

if (AllIsWell)
	{
	AllIsWell = 0;
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		nplanes = 8 * Channels;
		width 	= (short)BufWidth;
		height 	= (short)BufHeight;
		scrRowBytes 	= (short)(2 * ((BufWidth + 15) / 16));
		BODYsize 	= scrRowBytes * BufHeight * nplanes;
		AllIsWell = 1;
		} // if
	} // if

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	AllIsWell = 0;
	if (1)
		{
		// Write FORM header
		/* Form size will vary depending on whether there is a CMAP & CAMG chunk */
		FORMsize = BMHDsize + BODYsize + /* labelfieldsize */ (2 /* (chunks) */ * 8 + 4);

		strcpy(tt, "FORM");
		fwrite(tt, 1, 4, fHandle);
		FormSizePtr = ftell(fHandle);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32U(FORMsize, (unsigned long int *)tt);
		#else // BYTEORDER_LITTLEENDIAN
		memcpy(tt, &FORMsize, 4);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(tt, 1, 4, fHandle);
		strcpy(tt, "ILBM");
		if (fwrite(tt, 1, 4, fHandle) == 4)
			{
			strcpy(tt, "BMHD");
			fwrite(tt, 1, 4, fHandle);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(BMHDsize, (unsigned long int *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &BMHDsize, 4);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 4, fHandle);

			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(width, (short *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &width, 2);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(height, (short *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &height, 2);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			DummyZeroShort = 0;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(DummyZeroShort, (short *)tt); // xpos
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &DummyZeroShort, 2); // xpos
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			DummyZeroShort = 0;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(DummyZeroShort, (short *)tt); // ypos
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &DummyZeroShort, 2); // ypos
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			fwrite(&nplanes, 1, 1, fHandle);
			fwrite(&DummyZeroChar, 1, 1, fHandle); // masking
			fwrite(&compression, 1, 1, fHandle);
			fwrite(&DummyZeroChar, 1, 1, fHandle); // pad1
			// We don't append anymore
			//TransPtr = ftell(fHandle);

			DummyZeroShort = 0;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(DummyZeroShort, (short *)tt); // TransparentColor
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &DummyZeroShort, 2); // TransparentColor
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			aspect = 0xa0b; // hmm.
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(aspect, (short *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &aspect, 2);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(width, (short *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &width, 2);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite(tt, 1, 2, fHandle);

			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(height, (short *)tt);
			#else // BYTEORDER_LITTLEENDIAN
			memcpy(tt, &height, 2);
			#endif // BYTEORDER_LITTLEENDIAN
			if (fwrite(tt, 1, 2, fHandle) == 2)
				{
				AllIsWell = 1;
				} // if
			} // if
		} // if
	// Now write BODY
	if (AllIsWell)
		{
		AllIsWell = 0;
		strcpy(tt, "BODY");
		fwrite(tt, 1, 4, fHandle);
		BodySizePtr = ftell(fHandle);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32U(BODYsize, (unsigned long int *)tt);
		#else // BYTEORDER_LITTLEENDIAN
		memcpy(tt, &BODYsize, 4);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(tt, 1, 4, fHandle);

		if (compression)
			{
			// limit added 2/13/03 GRH
			CD.OutSize = min(1048576, BODYsize);
			CD.OutSize = max(scrRowBytes + 1, CD.OutSize);
			//CD.OutSize = BODYsize;

			while((CD.OutArray = (union MultiByte *)AppMem_Alloc(CD.OutSize, NULL, "IFF Saver", 1)) == NULL)
				{
				CD.OutSize /= 2;
				if (CD.OutSize < scrRowBytes + 1)
					{
					break;
					} // if
				} // if buffer size still greater than 0 - IT BETTER BE !!!
			if (CD.OutArray)
				{
				CD.Rows = nplanes;
				CD.RowBytes = scrRowBytes;
				CD.OutCtr = 0;
				CD.MaxByteRun = 127;
				CD.TotalOutBytes = 0;
				CD.fHandle = fHandle;
				AllIsWell = 1;
				} // if
			} /* if compression */
		else
			{
			AllIsWell = 1;
			} // else

		if (AllIsWell)
			{
			float incr;
			TransferBufferSize = scrRowBytes * 3 * 8;
			if ((TransferBuf = (unsigned char *)AppMem_Alloc(TransferBufferSize, APPMEM_CLEAR, "Save IFF")) != NULL)
				{
				CD.Data = TransferBuf;
				incr = 1.0f;
				if (ZAsGrey)
					{
					incr = (float)((FCDMax[0] - FCDMin[0]) / 256.0);
					if (incr == 0.0f) incr = 1.0f; // prevent div-by-zero
					} // if
				// height used to be rr, but we can reuse it
				for (height = 0; height < BufHeight; height++)
					{
					BuffPtr = TransferBuf;
					memset(TransferBuf, 0, TransferBufferSize);
					if (ZAsGrey && (FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(height, WCS_RASTER_BANDSET_FLOAT)))
						{
						// Transfer and compact Z-buffer to ZEight buffer
						for (xscan = 0; xscan < BufWidth; xscan++)
							{
							if (FloatChannelData[0][xscan] >= FCDMax[0])
								ZEight[xscan] = 0;
							else
								ZEight[xscan] = (unsigned char)((FCDMax[0] - FloatChannelData[0][xscan]) / incr);
							} // for
						} // if

					if (ShortChannelNode[0])
						ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(height, WCS_RASTER_BANDSET_SHORT);
					for (color = 0; color < Channels; color ++)
						{
						if (ZAsGrey)
							{
							CharChannelData[color] = ZEight;
							} // if
						else
							{
							CharChannelData[color] = (unsigned char *)CharChannelNode[color]->GetDataLine(height, WCS_RASTER_BANDSET_BYTE);
							} // else
						for (pp=0; pp<8; pp++)
							{
							scrRow = CharChannelData[color];
							PixCt = 0;
							for (short inbyte=0; inbyte < scrRowBytes; inbyte++)
								{
								for (pixel=7; pixel>=0; pixel--)
									{
									// test to make sure we're fetching real pixels, not IFF word-pad pixels, otherwise we skip or-ing in bits and they just remain 0
									if (PixCt < BufWidth) // don't want to read-overrun our input data blocks
										{
										OneColor = *scrRow; // fetch 8-bit non-HDR color gun value
										if (ShortChannelData[0] && ShortChannelData[0][PixCt]) // fetch 16-bit HDR exponent, if available
											rPixelFragment::ExtractClippedExponentialColor(OneColor, ShortChannelData[0][PixCt], color); // alter OneColor using 16-bit exponent if necessary
										if (OneColor & power2[pp]) // if the bit we're fetching within OneColor is non-zero
											*BuffPtr += (1 << pixel); // "or" the corresponding bit in the output BuffPtr to 1
										} // if
									scrRow ++;
									PixCt ++;
									} // for pixel=0... 
								BuffPtr ++;
								} // for byte=0... 
							} // for pp=0... 
						} // for color=0... 
					if (compression)
						{
						if (CompressRows(&CD) > 0)
							{
							AllIsWell = 0;
							break;
							} // if write error 
						} // if compression       
					else
						{
						if (fwrite(TransferBuf, 1, TransferBufferSize, fHandle) != (unsigned)TransferBufferSize)
							{
							break;
							} // if 
						} // if no compression 
					} // for height=0...
				if (height == BufHeight)
					{
					AllIsWell = 1;
					if (compression && CD.OutCtr > 0)
						{
						if (FlushOutputBuff(&CD) > 0)
							{
							AllIsWell = 0;
							} // if write error
						CD.TotalOutBytes += CD.OutCtr;
						} // if some data remaining in output buffer
					} // if

				AppMem_Free(TransferBuf, TransferBufferSize);
				TransferBuf = NULL;
				} // if row buffer allocated

			if (AllIsWell)
				{
				if (compression && (CD.TotalOutBytes % 2))
					{
					fwrite(&DummyZeroChar, 1, 1, fHandle); // pad1
					CD.TotalOutBytes ++;
					//Padded = 1;
					} // if odd number of output bytes

				if (compression)
					{
					fseek(fHandle, FormSizePtr, SEEK_SET);
					FORMsize -= BODYsize;
					FORMsize += CD.TotalOutBytes;
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32U(FORMsize, (unsigned long int *)tt);
					#else // BYTEORDER_LITTLEENDIAN
					memcpy(tt, &FORMsize, 4);
					#endif // BYTEORDER_LITTLEENDIAN
					fwrite(tt, 1, 4, fHandle);

					fseek(fHandle, BodySizePtr, SEEK_SET);
					BODYsize = CD.TotalOutBytes;
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32U(BODYsize, (unsigned long int *)tt);
					#else // BYTEORDER_LITTLEENDIAN
					memcpy(tt, &BODYsize, 4);
					#endif // BYTEORDER_LITTLEENDIAN
					fwrite(tt, 1, 4, fHandle);

/*
					// we don't append anymore...
					// Use Transparent Color in BMHD for notice of pad byte for later appending
					if (Padded)
						{
						fseek(fHandle, TransPtr, SEEK_SET);
						#ifdef BYTEORDER_LITTLEENDIAN
						SimpleEndianFlip16U(Padded, (unsigned short *)tt);
						#else // BYTEORDER_LITTLEENDIAN
						memcpy(tt, &Padded, 2);
						#endif // BYTEORDER_LITTLEENDIAN
						fwrite(tt, 1, 2, fHandle);
						} // if
*/
					} // if compression
				} // if
			} // if
		} // if
	} // if

if (ZEight)
	{
	AppMem_Free(ZEight, BufWidth);
	ZEight = NULL;
	} // if

if (fHandle)
	fclose(fHandle);
if (CD.OutArray)
	AppMem_Free(CD.OutArray, CD.OutSize);
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return (0);
#endif // !DEMO
} // ImageFormatIFF::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatBMP::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow;
unsigned char AllIsWell = 0, BMP_ID_B, BMP_ID_M, SampRGB[3];
long BMP_Width, BMP_Height, BMP_HRes, BMP_VRes;
unsigned long BMP_Size, BMP_Offset, BMP_Compression, BMP_BMapSize,
 BMP_ColorsUsed, BMP_ColorsImport;
unsigned short BMP_Planes, BMP_Bits;
long sizepos = 0, filelen = 0;
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
int WriteWorldFile = 0;
FILE *WF = NULL;
double N, S, W, E, pX, pY;

if (IOE)
	{
	if (strstr(IOE->Codec, "No"))
		{
		WriteWorldFile = 0;
		} // if
	else // max
		{
		WriteWorldFile = 1;
		} // else
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

// we know we'll need RGB bands so look for them specifically
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		// Write new WINBMPFILEHEADER
		BMP_ID_B   = 0x42; // B
		BMP_ID_M   = 0x4d; // M
		BMP_Size   = 0;    // Ignored in uncompressed BMP

		fwrite(&BMP_ID_B, 1, 1, fHandle);
		fwrite(&BMP_ID_M, 1, 1, fHandle);

		// so we can fseek back later to rewrite, for brain-damaged BMP readers
		// that can't handle size=0 uncompressed files
		sizepos = ftell(fHandle);

		fwrite(&BMP_Size, 1, 4, fHandle); // Size
		fwrite(&BMP_Size, 1, 4, fHandle); // Reserved

		BMP_Offset = 54; // Offset constant
		#ifndef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32U(BMP_Offset, &BMP_Offset);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(&BMP_Offset, 1, 4, fHandle); // Offset to bitmap data


		// Write new WIN3XBITMAPHEADER
		BMP_Size = 40;
		BMP_Width = BufWidth;
		BMP_Height = 0; // For now.
		BMP_Planes = 1;
		BMP_Bits = 24;
		BMP_Compression = 0;
		BMP_BMapSize = 0; // Meaning uncompressed
		BMP_HRes = 2834;	// 2834.65 pixels-per-meter is 72 dots per inch.
		BMP_VRes = 2834;	// 2834.65 pixels-per-meter is 72 dots per inch.
		BMP_ColorsUsed = 0;
		BMP_ColorsImport = 0;

		#ifndef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32U(BMP_Size, &BMP_Size);
		SimpleEndianFlip32S(BMP_Width, &BMP_Width);
		// Height is 0, pointless to flip...
		//SimpleEndianFlip32S(BMP_Height, &BMP_Height);
		SimpleEndianFlip16U(BMP_Planes, &BMP_Planes);
		SimpleEndianFlip16U(BMP_Bits, &BMP_Bits);
		// no need to flip compression...
		SimpleEndianFlip32U(BMP_BMapSize, &BMP_BMapSize);
		SimpleEndianFlip32S(BMP_HRes, &BMP_HRes);
		SimpleEndianFlip32S(BMP_VRes, &BMP_VRes);
		// no need to flip colorsused or colorsimport
		#endif // BYTEORDER_LITTLEENDIAN

		fwrite(&BMP_Size, 1, 4, fHandle); // Intel Byte-order
		fwrite(&BMP_Width, 1, 4, fHandle);
		fwrite(&BMP_Height, 1, 4, fHandle);
		fwrite(&BMP_Planes, 1, 2, fHandle);
		fwrite(&BMP_Bits, 1, 2, fHandle);
		fwrite(&BMP_Compression, 1, 4, fHandle);
		fwrite(&BMP_BMapSize, 1, 4, fHandle);
		fwrite(&BMP_HRes, 1, 4, fHandle);
		fwrite(&BMP_VRes, 1, 4, fHandle);
		fwrite(&BMP_ColorsUsed, 1, 4, fHandle);
		if (fwrite(&BMP_ColorsImport, 1, 4, fHandle) == 4)
			{
			AllIsWell = 1;
			} // if
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if 

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	// Clear status bit for next stage of operation
	AllIsWell = 0;

	if (TransferBuf = (unsigned char *)
	 AppMem_Alloc(ROUNDUP((BufWidth * 3), 4), APPMEM_CLEAR, "BMP Saver"))
		{
		// Write each line
		for (scanrow = BufHeight - 1; scanrow >= 0; scanrow--)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Interleave data into BGR format
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[2];
					TransferBuf[xout++] = SampRGB[1];
					TransferBuf[xout++] = SampRGB[0];
					} // for
				// write the interleaved data
				if (fwrite(TransferBuf, 1, ROUNDUP((BufWidth * 3), 4), fHandle) != (unsigned)ROUNDUP((BufWidth * 3), 4))
					{
					break;
					} // if
				} // if
			else
				{
				break;
				} // else
			} // for
		if (scanrow < 0)
			{
			fseek(fHandle, 22, SEEK_SET);
			BMP_Height += BufHeight;
			#ifndef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32S(BMP_Height, &BMP_Height);
			#endif // BYTEORDER_LITTLEENDIAN
			if (fwrite(&BMP_Height, 1, 4, fHandle) == 4)
				{
				AllIsWell = 1;
				} // if
			} // if
		} // if
	} // if file opened ok

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
if (AllIsWell && IOE && WriteWorldFile && RBounds)
	{
	// Filename.bpw
	FILE *PRJ = NULL;
	IOE->PAF.GetFramePathAndName(WorldFileName, ".bpw", Frame, 1000, IOE->AutoDigits);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			/*
			// Acquire boundaries of raster
			// center of UL (NW) corner pixel
			Corner.ScrnXYZ[0] = 0.5;
			Corner.ScrnXYZ[1] = 0.5;
			Corner.ScrnXYZ[2] = 1.0;
			UnprojectSys = RHost->DefCoords;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				UnprojectSys = RHost->Cam->Coords;
				} // if
			#endif // WCS_BUILD_VNS

			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				N = Corner.xyz[1]; 
				W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = Corner.Lat; 
				W = -Corner.Lon; // WorldFile uses GIS pos=east notation
				} // else

			// center of UL+1 pixel
			Corner.ScrnXYZ[0] = 1.5;
			Corner.ScrnXYZ[1] = 1.5;
			Corner.ScrnXYZ[2] = 1.0;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				pY = Corner.xyz[1] - N; 
				pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				pY = Corner.Lat - N; 
				pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
				} // else
			*/
			fprintf(WF, "%.15f\n", pX); // x dim
			fprintf(WF, "0.000\n"); // rot row
			fprintf(WF, "0.000\n"); // rot col
			fprintf(WF, "%.15f\n", -pY); // y dim
			fprintf(WF, "%f\n", W); // ul x = West
			fprintf(WF, "%f\n", N); // ul y = North
			fclose(WF);

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		} // if
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

// rewrite Size field
fseek(fHandle, 0, SEEK_END);
filelen = ftell(fHandle);
if (sizepos && filelen)
	{
	fseek(fHandle, sizepos, SEEK_SET);
	BMP_Size = filelen;
	#ifndef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(BMP_Size, &BMP_Size);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite(&BMP_Size, 1, 4, fHandle); // Size
	} // if

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, ROUNDUP((BufWidth * 3), 4));
	TransferBuf = NULL;
	} // if

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatBMP::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatBT::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long scanrow, scancol;
float *TransferF;
double N = 1.0, S = 0.0, E = 0.0, W = 1.0;
bool IsProjected = false;

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

#ifdef WCS_BUILD_VNS
CoordSys *ExportCS = NULL;

if (RBounds && (ExportCS = RBounds->FetchCoordSys()) != 0)
	{
	if (ExportCS->Method.GCTPMethod == 0)
		{
		IsProjected = false;
		} // if
	else
		{
		IsProjected = true;
		} // else
	} // if
#endif // WCS_BUILD_VNS

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufHeight, APPMEM_CLEAR, "BT Saver"))
		{
		TransferF = (float *)TransferBuf;
		if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
			{
			// write header
			WriteBT_Hdr(fHandle, BufWidth, BufHeight, IsProjected);
			for (scancol = 0; scancol < BufWidth; scancol++)
				{
				for (scanrow = 0; scanrow < BufHeight; scanrow++)
					{
					// fetch scanline
					FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
					TransferF[BufHeight - (scanrow + 1)] = FloatChannelData[0][scancol];
					} // for
				// write line
				WriteBT_Line(fHandle, BufHeight, TransferF);
				} // for
			// Acquire boundaries of raster
			if (RBounds)
				{
				RBounds->FetchBoundsCentersGIS(N, S, W, E);
				/*
				// center of UL (NW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = (float)Corner.Lat; 
				W = (float)-Corner.Lon; // BT uses GIS pos=east notation

				// center of LR (SE) corner pixel
				Corner.ScrnXYZ[0] = (double)BufWidth  - 0.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				S = (float)Corner.Lat; 
				E = (float)-Corner.Lon; // BT uses GIS pos=east notation
				*/
				} // if
			// close file
			FinishBT_File(fHandle, N, S, E, W);

#ifdef WCS_BUILD_VNS
			if (IsProjected)
				{
				FILE *PRJ = NULL;
				// Now try to write a PRJ file along with it
				IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
				if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
					{
					RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
					fclose(PRJ);
					PRJ = NULL;
					} // if
				} // if
#endif // WCS_BUILD_VNS

			} // if
		AppMem_Free(TransferBuf, sizeof(float) * BufHeight);
		TransferBuf = NULL;
		} // if
	} // if

if (fHandle != NULL)
	fclose(fHandle);

/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatBT::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatWCSELEV::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long scanrow, scancol;
float *TransferF, *SavePtr;
Joe *TempJoe = NULL;
JoeCoordSys *MyAttrib = NULL;
double N = 1.0, S = 0.0, E = 0.0, W = 1.0;
double LatStep, LonStep;
VertexDEM Corner;
DEM TempDEM;

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufHeight, APPMEM_CLEAR, "WCS ELEV Saver"))
		{
		TransferF = (float *)TransferBuf;
		if (1)
			{
			// Plug in defaults in case something goes wrong.
			LatStep = 1.0 / (double)(BufHeight - 1);
			LonStep = 1.0 / (double)(BufWidth - 1);
			// Acquire boundaries of raster and LatStep/LonStep
			if (RBounds)
				{
				RBounds->FetchBoundsCentersWCS(N, S, W, E);
				RBounds->FetchCellSizeWCS(LatStep, LonStep);

				/*
				// center of UL (NW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = (float)Corner.Lat; 
				W = (float)Corner.Lon; // WCS uses pos=west notation

				// center of UL+1 (NW) corner pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				LatStep = fabs(N - (float)Corner.Lat);
				LonStep = fabs(W - (float)Corner.Lon);

				// center of LR (SE) corner pixel
				Corner.ScrnXYZ[0] = (double)BufWidth  - 0.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				S = (float)Corner.Lat; 
				E = (float)Corner.Lon; // WCS uses pos=west notation
				*/
				} // if

			// Write WCS .elev DEM file
			TempDEM.pLonEntries = BufWidth;
			TempDEM.pLatEntries = BufHeight;

			TempDEM.pLatStep = LatStep;
			TempDEM.pLonStep = LonStep;
			TempDEM.pNorthWest.Lon = W;
			TempDEM.pSouthEast.Lat = S;
			TempDEM.pSouthEast.Lon = E;
			TempDEM.pNorthWest.Lat = N;
			TempDEM.pElScale = ELSCALE_METERS;
			TempDEM.pElDatum = 0.0;
			TempDEM.PrecalculateCommonFactors();

			if ((TempDEM.RawMap = (float *)AppMem_Alloc(TempDEM.MapSize() * sizeof(float), 0)) != NULL)
				{
				// prepare DEM array in memory
				SavePtr = TempDEM.RawMap;
				for (scancol = 0; scancol < BufWidth; scancol++)
					{
					for (scanrow = 0; scanrow < BufHeight; scanrow++)
						{
						// fetch scanline
						FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
						TransferF[BufHeight - (scanrow + 1)] = FloatChannelData[0][scancol];
						} // for
					// do finished line
					memcpy(SavePtr, TransferF, BufHeight * sizeof(float));
					SavePtr = &SavePtr[BufHeight];
					} // for

				if (RBounds)
					{
					if (RBounds->FetchCoordSys())
						{
						// need to create a Joe, set MoJoe, create a JoeCoordSys and attach CoordSys
						if (TempJoe = new (NULL) Joe())
							{
							if (MyAttrib = new JoeCoordSys)
								{
								MyAttrib->Coord = RBounds->FetchCoordSys();
								TempJoe->AddAttribute(MyAttrib);
								} // if
							TempDEM.MoJoe = TempJoe;
							} // if
						} // if
					TempDEM.SetNullValue(RBounds->NullValue);
					TempDEM.SetNullReject(RBounds->NullReject);
					} // if
				if (TempDEM.SaveDEM(GetCompleteOutputPath(), NULL))
					{
					// nothing to do here anymore
					} // if saved OK
				else
					{
					UserMessageOK(APP_TLA" DEM Saver ", "Error saving DEM..");
					} // else
				if (TempJoe)
					{
					if (MyAttrib)
						{
						TempJoe->RemoveSpecificAttributeNoNotify(MyAttrib);
						delete MyAttrib;
						} // if
					delete TempJoe;
					} // if
				TempDEM.FreeRawElevs();
				} // if
			} // if
		AppMem_Free(TransferBuf, sizeof(float) * BufHeight);
		TransferBuf = NULL;
		} // if
	} // if


/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatWCSELEV::StartFrame

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_IMAGEFORMATBIL_SUPPORT
int ImageFormatBIL::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long scanrow, scancol;
double N = 1.0, S, W = -1.0, E, pX, pY;
VertexDEM Corner;
FILE *hdrHandle = NULL, *blwHandle = NULL;

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufHeight, APPMEM_CLEAR, "BIL Saver"))
		{
		if (fHandle = PROJ_fopen(IOE->PrepCompleteOutputPathSuffix(Frame, ".bil"), "wb+"))
			{
			FILE *PRJ = NULL;

			pX = 1.0 / (BufWidth - 1);	// backup
			pY = 1.0 / (BufHeight - 1);	// backup
			if (RBounds)
				{
				RBounds->FetchBoundsCentersGIS(N, S, W, E);
				RBounds->FetchCellSizeGIS(pY, pX);
				/*
				// Acquire boundaries of raster
				// center of UL (NW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				UnprojectSys = RHost->DefCoords;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					UnprojectSys = RHost->Cam->Coords;
					} // if
				#endif // WCS_BUILD_VNS

				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					N = Corner.xyz[1]; 
					W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					N = Corner.Lat; 
					W = -Corner.Lon; // WorldFile uses GIS pos=east notation
					} // else

				// center of UL+1 pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					pY = Corner.xyz[1] - N; 
					pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					pY = Corner.Lat - N; 
					pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
					} // else
				*/
				} // if

			for (scanrow = 0; scanrow < BufHeight; scanrow++)
				{
				// fetch scanline
				FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
				// write line
				for (scancol = 0; scancol < BufWidth; scancol++)
					{
					signed short SSI;
					SSI = (signed short)FloatChannelData[0][scancol];
					fwrite(&SSI, 1, 2, fHandle);
					} // for
				} // for

			if (hdrHandle = PROJ_fopen(IOE->PrepCompleteOutputPathSuffix(Frame, ".hdr"), "wb+"))
				{
				// write header
				#ifdef BYTEORDER_LITTLEENDIAN
				fprintf(hdrHandle, "BYTEORDER I\n");
				#else // !BYTEORDER_LITTLEENDIAN
				fprintf(hdrHandle, "BYTEORDER M\n");
				#endif // BYTEORDER_LITTLEENDIAN
				fprintf(hdrHandle, "LAYOUT BIL\n");
				fprintf(hdrHandle, "NROWS %d\n", BufHeight);
				fprintf(hdrHandle, "NCOLS %d\n", BufWidth);
				fprintf(hdrHandle, "NBANDS 1\n");
				fprintf(hdrHandle, "NBITS 16\n");
				fprintf(hdrHandle, "BANDROWBYTES %d\n", BufWidth * 2);
				fprintf(hdrHandle, "TOTALROWBYTES %d\n", BufWidth * 2);
				fprintf(hdrHandle, "BANDGAPBYTES 0\n");
				} // if

			if (blwHandle = PROJ_fopen(IOE->PrepCompleteOutputPathSuffix(Frame, ".blw"), "wb+"))
				{
				fprintf(blwHandle, "%.15f\n", pX); // x dim
				fprintf(blwHandle, "0.000\n"); // rot row
				fprintf(blwHandle, "0.000\n"); // rot col
				fprintf(blwHandle, "%.15f\n", -pY); // y dim
				fprintf(blwHandle, "%f\n", W); // ul x = West
				fprintf(blwHandle, "%f\n", N); // ul y = North
		
				// Now try to write a PRJ file along with it
				IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
				if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
					{
					RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
					fclose(PRJ);
					PRJ = NULL;
					} // if
				} // if

			} // if
		AppMem_Free(TransferBuf, sizeof(float) * BufHeight);
		TransferBuf = NULL;
		} // if
	} // if

if (fHandle != NULL)
	fclose(fHandle);

if (hdrHandle != NULL)
	fclose(hdrHandle);
hdrHandle = NULL;

if (blwHandle != NULL)
	fclose(blwHandle);
blwHandle = NULL;

/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatBIL::StartFrame
#endif // WCS_BUILD_IMAGEFORMATBIL_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT
int ImageFormatGRIDFLOAT::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long scanrow, scancol;
double N = 1.0f, S = 0.0f, E = 0.0f, W = -1.0f, pX, pY;
VertexDEM Corner;
double FileCellSize = 1.0;
FILE *hdrHandle = NULL;

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufHeight, APPMEM_CLEAR, "GRIDFLOAT Saver"))
		{
		if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
			{
			FILE *PRJ = NULL;

			FileCellSize = 1.0 / (BufWidth - 1);	// backup
			// Acquire boundaries of raster
			if (RBounds)
				{
				RBounds->FetchBoundsCentersGIS(N, S, W, E);
				RBounds->FetchCellSizeGIS(pY, pX);
				FileCellSize = pX;
				/*
				// Acquire boundaries of raster
				// center of UL (NW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				UnprojectSys = RHost->DefCoords;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					UnprojectSys = RHost->Cam->Coords;
					} // if
				#endif // WCS_BUILD_VNS

				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					N = Corner.xyz[1]; 
					W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					N = Corner.Lat; 
					W = -Corner.Lon; // WorldFile uses GIS pos=east notation
					} // else

				// center of UL+1 pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					pY = Corner.xyz[1] - N; 
					pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					pY = Corner.Lat - N; 
					pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
					} // else

				FileCellSize = pX;

				// center of LL (SW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				S = (float)Corner.Lat; 
				W = (float)-Corner.Lon; // ARCASCII uses GIS pos=east notation

				// Determine Cell Size
				// center of LL+1 (SW) corner pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				LatStep = fabs(S - (float)Corner.Lat);
				LonStep = fabs(W - (float)-Corner.Lon);
				*/
				} // if

			for (scanrow = 0; scanrow < BufHeight; scanrow++)
				{
				// fetch scanline
				FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
				// write line
				for (scancol = 0; scancol < BufWidth; scancol++)
					{
					fwrite(&FloatChannelData[0][scancol], 1, 4, fHandle);
					} // for
				} // for

			if (hdrHandle = PROJ_fopen(IOE->PrepCompleteOutputPathSuffix(Frame, ".hdr"), "wb+"))
				{
				// write header
				fprintf(hdrHandle, "ncols %d\n", BufWidth);
				fprintf(hdrHandle, "nrows %d\n", BufHeight);
				fprintf(hdrHandle, "cellsize %f\n", FileCellSize);
				fprintf(hdrHandle, "xllcorner %f\n", W);
				fprintf(hdrHandle, "yllcorner %f\n", S);
				fprintf(hdrHandle, "nodata_value %f\n", -9999.0f);
				#ifdef BYTEORDER_LITTLEENDIAN
				fprintf(hdrHandle, "byteorder LSBFIRST\n");
				#else // !BYTEORDER_LITTLEENDIAN
				fprintf(hdrHandle, "byteorder MSBFIRST\n");
				#endif // BYTEORDER_LITTLEENDIAN
				} // if

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		AppMem_Free(TransferBuf, sizeof(float) * BufHeight);
		TransferBuf = NULL;
		} // if
	} // if

if (fHandle != NULL)
	fclose(fHandle);

if (hdrHandle != NULL)
	fclose(hdrHandle);
hdrHandle = NULL;

/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatGRIDFLOAT::StartFrame
#endif // WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT
int ImageFormatARCASCII::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long scanrow, scancol;
double N = 1.0f, S = 0.0f, E = 0.0f, W = -1.0f, pX, pY;
VertexDEM Corner;
double FileCellSize = 1.0;

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufHeight, APPMEM_CLEAR, "ARCASCII Saver"))
		{
		if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "w+"))
			{
			FILE *PRJ = NULL;
			FileCellSize = 1.0 / (BufWidth - 1);	// backup
			// Acquire boundaries of raster
			if (RBounds)
				{
				RBounds->FetchBoundsCentersGIS(N, S, W, E);
				RBounds->FetchCellSizeGIS(pY, pX);
				FileCellSize = pX;
				/*
				// Acquire boundaries of raster
				// center of UL (NW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				UnprojectSys = RHost->DefCoords;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					UnprojectSys = RHost->Cam->Coords;
					} // if
				#endif // WCS_BUILD_VNS

				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					N = Corner.xyz[1]; 
					W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					N = Corner.Lat; 
					W = -Corner.Lon; // WorldFile uses GIS pos=east notation
					} // else

				// center of UL+1 pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					pY = Corner.xyz[1] - N; 
					pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					pY = Corner.Lat - N; 
					pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
					} // else

				FileCellSize = pX;

				// ARCASCII wants SOUTH-West corner...
				// Acquire boundaries of raster
				// center of LL (SW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = BufHeight - 0.5;
				Corner.ScrnXYZ[2] = 1.0;

				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
					S = Corner.xyz[1]; 
					} // if
				else
				#endif // WCS_BUILD_VNS
					{
					RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
					S = Corner.Lat; 
					} // else
				*/
				/*
				// center of LL (SW) corner pixel
				Corner.ScrnXYZ[0] = 0.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 0.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				S = (float)Corner.Lat; 
				W = (float)-Corner.Lon; // ARCASCII uses GIS pos=east notation

				// Determine Cell Size
				// center of LL+1 (SW) corner pixel
				Corner.ScrnXYZ[0] = 1.5;
				Corner.ScrnXYZ[1] = (double)BufHeight - 1.5;
				Corner.ScrnXYZ[2] = 1.0;
				RHost->Cam->UnProjectVertexDEM(RHost->DefCoords, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				LatStep = fabs(S - (float)Corner.Lat);
				LonStep = fabs(W - (float)-Corner.Lon);

				FileCellSize = LonStep;
				*/
				} // if

			// write header
			fprintf(fHandle, "ncols %d\n", BufWidth);
			fprintf(fHandle, "nrows %d\n", BufHeight);
			fprintf(fHandle, "cellsize %f\n", FileCellSize);
			fprintf(fHandle, "xllcorner %f\n", W);
			fprintf(fHandle, "yllcorner %f\n", S);
			fprintf(fHandle, "nodata_value %f\n", -9999.0f);
			for (scanrow = 0; scanrow < BufHeight; scanrow++)
				{
				// fetch scanline
				FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
				// write line
				for (scancol = 0; scancol < BufWidth; scancol++)
					{
					fprintf(fHandle, "%f ", FloatChannelData[0][scancol]);
					} // for
				fprintf(fHandle, "\n");
				} // for

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		AppMem_Free(TransferBuf, sizeof(float) * BufHeight);
		TransferBuf = NULL;
		} // if
	} // if

if (fHandle != NULL)
	fclose(fHandle);

/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatARCASCII::StartFrame
#endif // WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_IMAGEFORMATSTL_SUPPORT
// currently we're only expecting to generate STL's for projected planimetrics with Eastings & Northings
int ImageFormatSTL::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
double N, S, W, E, dX, dY;
double xpos, ypos;
double xmax, ymax, xmin, ymin, xrange, yrange, scalefactor;
float xout, yout, zout;
VertexDEM Corner1, Corner2;
long scanrow, scancol, ASCIImode;
unsigned short zero = 0;

if (IOE)
	{
	if (strstr(IOE->Codec, "ASCII"))
		ASCIImode = 1;
	else
		ASCIImode = 0;
	} // if IOE

// we know we'll need elev band so look for it specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	if (TransferBuf = (unsigned char *)AppMem_Alloc(sizeof(float) * BufWidth, APPMEM_CLEAR, "STL Saver"))
		{
		if (ASCIImode)
			fHandle = PROJ_fopen(GetCompleteOutputPath(), "w+");
		else
			fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+");
		if (fHandle)
			{
			// Acquire boundaries of raster
			if (RBounds)
				{
				RBounds->FetchBoundsCentersGIS(N, S, W, E);
				RBounds->FetchCellSizeGIS(dY, dX);
				xmin = W;
				ymax = N;
				xmax = E;
				ymin = S;
				xrange = xmax - xmin;
				yrange = ymax - ymin;
				// auto scale to fit build envelope
				// here we scale to fit maximum of 250mm per side (ie: 3D Systems SLA 250 rapid prototyper)
				if (xrange > yrange)
					scalefactor = 250.0 / xrange;
				else
					scalefactor = 250.0 / yrange;
				// compute cell size
				dX = xrange / BufWidth;
				dY = yrange / BufHeight;
				/*
				// Acquire boundaries of raster
				// UL (NW) cell center
				Corner1.ScrnXYZ[0] = 0.5;
				Corner1.ScrnXYZ[1] = 0.5;
				Corner1.ScrnXYZ[2] = 1.0;
				UnprojectSys = RHost->DefCoords;
				// LR (SE) cell center
				Corner2.ScrnXYZ[0] = RHost->Width - 0.5;
				Corner2.ScrnXYZ[1] = RHost->Height - 0.5;
				Corner2.ScrnXYZ[2] = 1.0;
				#ifdef WCS_BUILD_VNS
				if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
					{
					UnprojectSys = RHost->Cam->Coords;
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner1, RHost->EarthLatScaleMeters);
					RHost->Cam->UnProjectProjectedVertexDEM(&Corner2, RHost->EarthLatScaleMeters);
					// get bounds
					xmin = Corner1.xyz[0];
					ymax = Corner1.xyz[1];
					xmax = Corner2.xyz[0];
					ymin = Corner2.xyz[1];
					xmin = W;
					ymax = N;
					xmax = E;
					ymin = S;
					xrange = xmax - xmin;
					yrange = ymax - ymin;
					// auto scale to fit build envelope
					// here we scale to fit maximum of 250mm per side (ie: 3D Systems SLA 250 rapid prototyper)
					if (xrange > yrange)
						scalefactor = 250.0 / xrange;
					else
						scalefactor = 250.0 / yrange;
					// compute cell size
					dX = xrange / BufWidth;
					dY = yrange / BufHeight;
					} // if
				#endif // WCS_BUILD_VNS
				*/
				} // if RBounds

			if (ASCIImode)
				fprintf(fHandle, "solid\n");
			else
				{
				unsigned long facets = (unsigned long)((BufWidth - 1) * (BufHeight - 1) * 2);
				char Header[80];
				char String[] = "STL generated by 3D Nature's RTX product";
				memset(Header, 0, 80);
				memcpy(Header, String, sizeof(String));
				fwrite(Header, 1, 80, fHandle);
				Put32U(LITTLE_END_DATA, facets, fHandle);
				} // else

			ypos = Corner1.xyz[1];
			// we probably really should scan the elevs once to get the proper range too
			// for now, we just assume that the scalefactor will ensure a reasonable height too
			for (scanrow = 1; scanrow < BufHeight; scanrow++, ypos -= dY)
				{
				xpos = Corner1.xyz[0];
				// fetch scanline
				FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow - 1, WCS_RASTER_BANDSET_FLOAT);
				FloatChannelData[1] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);
				// write triangles for the two lines we have
				for (scancol = 1; scancol < BufWidth; scancol++, xpos += dX)
					{
					long backcol = scancol - 1;
					// triangles are counterclockwise on the outer surface
					// triangle #1 with verts 0 1 2, verts 0 & 2 on scancol, vert 1 on backcol:
					// 1 - 0 -> FloatChannelData[0]
					//   \ |
					//     2 -> FloatChannelData[1]
					if (ASCIImode)
						{
						fprintf(fHandle, "  facet normal 0.0 0.0 1.0\n");
						fprintf(fHandle, "    outer loop\n");
						} // if
					else
						{
						xout = 0.0f; yout = 0.0f, zout = 1.0f;
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						}
					// vert 0
					xout = (float)((xpos +dX - xmin) * scalefactor);
					yout = (float)((ypos - ymin) * scalefactor);
					zout = (float)(FloatChannelData[0][scancol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					// vert 1
					xout = (float)((xpos - xmin) * scalefactor);
					yout = (float)((ypos - ymin) * scalefactor);
					zout = (float)(FloatChannelData[0][backcol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					// vert 2
					xout = (float)((xpos + dX - xmin) * scalefactor);
					yout = (float)((ypos - dY - ymin) * scalefactor);
					zout = (float)(FloatChannelData[1][scancol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					if (ASCIImode)
						{
						fprintf(fHandle, "    end loop\n");
						fprintf(fHandle, "  endfacet\n");
						} // if
					else
						{
						Put16U(LITTLE_END_DATA, zero, fHandle);
						}
					// triangle #2 with verts 3 4 5, vert 3 on scancol, verts 4 & 5 on backcol:
					// 4     -> FloatChannelData[0]
					// | \								//
					// 5 - 3 -> FloatChannelData[1]
					if (ASCIImode)
						{
						fprintf(fHandle, "  facet normal 0.0 0.0 1.0\n");
						fprintf(fHandle, "    outer loop\n");
						} // if
					else
						{
						xout = 0.0f; yout = 0.0f, zout = 1.0f;
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						}
					// vert 3
					xout = (float)((xpos + dX - xmin) * scalefactor);
					yout = (float)((ypos - dY - ymin) * scalefactor);
					zout = (float)(FloatChannelData[1][scancol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					// vert 4
					xout = (float)((xpos - xmin) * scalefactor);
					yout = (float)((ypos - ymin) * scalefactor);
					zout = (float)(FloatChannelData[0][backcol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					// vert 5
					xout = (float)((xpos - xmin) * scalefactor);
					yout = (float)((ypos - dY - ymin) * scalefactor);
					zout = (float)(FloatChannelData[1][backcol] * scalefactor);
					if (ASCIImode)
						{
						fprintf(fHandle, "      vertex %f %f %f\n", xout, yout, zout);
						}
					else
						{
						Put32F(LITTLE_END_DATA, &xout, fHandle);
						Put32F(LITTLE_END_DATA, &yout, fHandle);
						Put32F(LITTLE_END_DATA, &zout, fHandle);
						} // else
					if (ASCIImode)
						{
						fprintf(fHandle, "    end loop\n");
						fprintf(fHandle, "  endfacet\n");
						} // if
					else
						{
						Put16U(LITTLE_END_DATA, zero, fHandle);
						}
					} // for
				} // for
			if (ASCIImode)
				{
				fprintf(fHandle, "endsolid\n");
				}

			} // if fHandle
		AppMem_Free(TransferBuf, sizeof(float) * BufWidth);
		TransferBuf = NULL;
		} // if TransferBuf
	} // if FloatChannelNode[0]

if (fHandle != NULL)
	fclose(fHandle);

/*
if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if
*/
return(0);
#endif // !DEMO

} // ImageFormatSTL::StartFrame
#endif // WCS_BUILD_IMAGEFORMATSTL_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatTGA::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow /*, ExtOffset*/;
unsigned char AllIsWell = 0, TGA_IDLen, TGA_CMType, TGA_IMType, TGA_PixDepth, TGA_ImgDesc, Channels, AlphaOk = 1, SampRGB[3];
unsigned short TGA_Width, TGA_Height;

// we know we'll need RGB bands so look for them specifically
Channels = 3;
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if
else if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		// Write new Targa header
		TGA_IDLen  = 32; // We stuff our filename here
		TGA_CMType = 0;  // No colormap
		TGA_IMType = 2;  // Truecolor Image, 24 bit uncompressed

		fwrite(&TGA_IDLen, 1, 1, fHandle);
		fwrite(&TGA_CMType, 1, 1, fHandle);
		fwrite(&TGA_IMType, 1, 1, fHandle);

		// Write 5 empty bytes for colormap spec
		memset(TGA_Dummy, 0, 5);
		fwrite(TGA_Dummy, 1, 5, fHandle);

		// Write X and Y origin
		TGA_Width = TGA_Height = 0;
		fwrite(&TGA_Width, 1, 2, fHandle); // really X-Origin
		fwrite(&TGA_Height, 1, 2, fHandle); // really Y-Origin

		// Write Width and Height. Leave Height at 0, we'll
		// come back and rewrite it when we're done writing data.
		TGA_Width = (unsigned short)BufWidth;
		TGA_Height = 0;
		#ifndef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(TGA_Width, &TGA_Width);
		SimpleEndianFlip16U(TGA_Height, &TGA_Height);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(&TGA_Width, 1, 2, fHandle); // Intel Byte-order
		fwrite(&TGA_Height, 1, 2, fHandle); // Intel Byte-order

		// Write PixDepth
		TGA_PixDepth = (Channels == 4) ? 32 : 24; // bits of data, 8r8g8b plus 8a, potentially
		TGA_ImgDesc  = (Channels == 4) ? 0x28 : 0x20; // Origin at TopLeft, need to indicate number of alpha bits
		fwrite(&TGA_PixDepth, 1, 1, fHandle);
		fwrite(&TGA_ImgDesc, 1, 1, fHandle);

		// Now try to write 32 bytes worth of ImageID, built
		// from our filename
		TGA_ID[0] = 0;
		strncpy(TGA_ID, GetName(), 32);
		TGA_ID[32] = NULL;
		// Append 32 spaces for padding
		strcat(TGA_ID, "                                ");
		if (fwrite(TGA_ID, 1, 32, fHandle) == 32)
			{
			AllIsWell = 1;
			} // if
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if
else
	{
	// you should never get to this message if everything was done correctly in render init
	UserMessageOK("Image Save", "Unable to save image file because not all required buffers have been rendered.");
	} // else

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	// Clear status bit for next stage of operation
	AllIsWell = 0;

	if (TransferBuf = (unsigned char *)
	 AppMem_Alloc(BufWidth * Channels, APPMEM_CLEAR, "TGA Saver"))
		{
		// Write each line
		for (scanrow = 0; scanrow < BufHeight; scanrow++)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);

			if (CharChannelNode[3])
				{
				CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				} // if
			else
				{
				AlphaOk = 1;
				} // else

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2] && AlphaOk)
				{
				// Interleave data into (A)BGR format
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[2];
					TransferBuf[xout++] = SampRGB[1];
					TransferBuf[xout++] = SampRGB[0];
					if (CharChannelData[3])
						{
						TransferBuf[xout++] = CharChannelData[3][xscan];
						} // if
					} // for
				// write the interleaved data
				if (fwrite(TransferBuf, 1, BufWidth * Channels, fHandle) != (unsigned)(BufWidth * Channels))
					{
					break;
					} // if
				} // if
			else
				{
				break;
				} // else
			} // for
		if (scanrow == BufHeight)
			{
			// Possibly write extension area here...
			//ExtOffset = ftell(fHandle);

			// Write Targa Footer
			scanrow = 0;
			fwrite(&scanrow, 1, 4, fHandle); // Extension offset (Unused)
			fwrite(&scanrow, 1, 4, fHandle); // Developer directory offset
			if (fwrite("TRUEVISION-XFILE.", 1, 18, fHandle) == 18)
				{
				// Go back and rewrite Height field
				fseek(fHandle, 14, SEEK_SET);
				TGA_Height += (unsigned short)BufHeight;
				#ifndef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip16U(TGA_Height, &TGA_Height);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite(&TGA_Height, 1, 2, fHandle); // Intel Byte-order
				AllIsWell = 1;
				} // if
			} // if
		} // if

	} // if file opened ok

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth * Channels);
	TransferBuf = NULL;
	} // if

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO

} // ImageFormatTGA::StartFrame

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_SGIRGB_SUPPORT
int ImageFormatSGIRGB::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{ // SGI RGB format is big-endian
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow /*, ExtOffset*/;
unsigned char AllIsWell = 0, Channels, SampRGB[3];

// we know we'll need RGB bands so look for them specifically
Channels = 3;
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if
else if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		// Write new SGIRGB header

		struct _SGIHeader
			{
			short Magic;          // Identification number (474)
			char Storage;         // Compression flag
			char Bpc;             // Bytes per pixel
			unsigned short Dimension;       // Number of image dimensions
			unsigned short XSize;           // Width of image in pixels
			unsigned short YSize;           // Height of image in pixels
			unsigned short ZSize;           // Number of bit planes
			long PixMin;          // Smallest pixel value
			long PixMax;          // Largest pixel value
			char Dummy1[4];       // Not used
			char ImageName[80];   // Name of image
			long ColorMap;        // Format of pixel data
			char Dummy2[404];     // Not used
			} SGIHEAD;

		SGIHEAD.Magic = 474;
		SGIHEAD.Storage = 0; // uncompresed
		SGIHEAD.Bpc = 1; // one byte per channel
		SGIHEAD.Dimension = 3; // multi-channel, number of channels indicated by ZSize
		SGIHEAD.XSize = (unsigned short)BufWidth;
		SGIHEAD.YSize = (unsigned short)BufHeight;
		SGIHEAD.ZSize = Channels; // RGB: 3, RGBA: 4
		SGIHEAD.PixMin = 0; // Or so we claim
		SGIHEAD.PixMax = 255; // Or so we claim
		SGIHEAD.Dummy1[0] = SGIHEAD.Dummy1[1] = SGIHEAD.Dummy1[2] = SGIHEAD.Dummy1[3] = 0; // Dummy NULL
		memset(SGIHEAD.ImageName, 0, 80);
		strncpy(SGIHEAD.ImageName, GetName(), 79);
		SGIHEAD.ImageName[79] = NULL;
		SGIHEAD.ColorMap = 0; // according to SGI's docs, not GFF, which says 1
		memset(SGIHEAD.Dummy2, 0, 404);

		// write the above
		PutB16S(SGIHEAD.Magic, fHandle);
		PutB8S(SGIHEAD.Storage, fHandle);
		PutB8S(SGIHEAD.Bpc, fHandle);
		PutB16U(SGIHEAD.Dimension, fHandle);
		PutB16U(SGIHEAD.XSize, fHandle);
		PutB16U(SGIHEAD.YSize, fHandle);
		PutB16U(SGIHEAD.ZSize, fHandle);
		PutB32S(SGIHEAD.PixMin, fHandle);
		PutB32S(SGIHEAD.PixMax, fHandle);
		PutB8S(SGIHEAD.Dummy1[0], fHandle); PutB8S(SGIHEAD.Dummy1[1], fHandle); PutB8S(SGIHEAD.Dummy1[2], fHandle); PutB8S(SGIHEAD.Dummy1[3], fHandle);
		fwrite(SGIHEAD.ImageName, 1, 80, fHandle);
		PutB32S(SGIHEAD.ColorMap, fHandle);

		if (fwrite(SGIHEAD.Dummy2, 1, 404, fHandle) == 404)
			{
			AllIsWell = 1;
			} // if
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if
else
	{
	// you should never get to this message if everything was done correctly in render init
	UserMessageOK("Image Save", "Unable to save image file because not all required buffers have been rendered.");
	} // else

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	// Set status bit for next stage of operation
	AllIsWell = 1;

	if (TransferBuf = (unsigned char *) AppMem_Alloc(BufWidth, APPMEM_CLEAR, "SGIRGB Saver"))
		{
		// write all scanlines of plane 1, followed by all scanlines of 2, 3 and possibly 4

		// Plane 1: Red
		for (scanrow = BufHeight - 1; scanrow >= 0; scanrow--)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
			// Write each line
			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Convert data into TransferBuf
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					// need all three channels to handle exponent
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[0];
					} // for
				// write the data
				if (fwrite(TransferBuf, 1, BufWidth, fHandle) != (unsigned)BufWidth)
					{
					AllIsWell = 0;
					break;
					} // if
				} // if
			else
				{
				AllIsWell = 0;
				break;
				} // else
			} // for

		// Plane 2: Green
		for (scanrow = BufHeight - 1; scanrow >= 0; scanrow--)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
			// Write each line
			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Convert data into TransferBuf
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					// need all three channels to handle exponent
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[1];
					} // for
				// write the data
				if (fwrite(TransferBuf, 1, BufWidth, fHandle) != (unsigned)BufWidth)
					{
					AllIsWell = 0;
					break;
					} // if
				} // if
			else
				{
				AllIsWell = 0;
				break;
				} // else
			} // for

		// Plane 3: Blue
		for (scanrow = BufHeight - 1; scanrow >= 0; scanrow--)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
			// Write each line
			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Convert data into TransferBuf
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					// need all three channels to handle exponent
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[2];
					} // for
				// write the data
				if (fwrite(TransferBuf, 1, BufWidth, fHandle) != (unsigned)BufWidth)
					{
					AllIsWell = 0;
					break;
					} // if
				} // if
			else
				{
				AllIsWell = 0;
				break;
				} // else
			} // for

		// Plane 4: Alpha
		if (CharChannelNode[3] && Channels == 4)
			{
			AllIsWell = 0;
			for (scanrow = BufHeight - 1; scanrow >= 0; scanrow--)
				{
				CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				// Write each line
				if (CharChannelData[3])
					{
					// Copy alpha into TransferBuf
					for (xout = xscan = 0; xscan < BufWidth; xscan++)
						{
						// need all three channels to handle exponent
						TransferBuf[xout++] = CharChannelData[3][xscan];
						} // for
					// write the data
					if (fwrite(TransferBuf, 1, BufWidth, fHandle) != (unsigned)BufWidth)
						{
						AllIsWell = 0;
						break;
						} // if
					} // if
				else
					{
					AllIsWell = 0;
					break;
					} // else
				} // for
			AllIsWell = 1;
			} // if
		} // if
	} // if file opened ok

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth);
	TransferBuf = NULL;
	} // if

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO

} // ImageFormatSGIRGB::StartFrame

#endif // 


#ifdef WCS_BUILD_HDR_SUPPORT


// RGBE code should bear this notice:
// posted to http://www.graphics.cornell.edu/~bjw/
// written by Bruce Walter  (bjw@graphics.cornell.edu)  5/26/95
// based on code written by Greg Ward


/* standard conversion from float pixels to rgbe pixels */
/* note: you can remove the "inline"s if your compiler complains about it */
static inline void float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
float v;
int e;

v = red;
if (green > v)
	v = green;
if (blue > v)
	v = blue;
if (v < 1e-32)
	{
	rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	}
else
	{
	v = (float)(frexp(v,&e) * 256.0/v);
	rgbe[0] = (unsigned char)(red * v);
	rgbe[1] = (unsigned char)(green * v);
	rgbe[2] = (unsigned char)(blue * v);
	rgbe[3] = (unsigned char)(e + 128);
	} // else

} // float2rgbe

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatRGBE::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow;
unsigned char AllIsWell = 0;
unsigned char rgbe[4];
unsigned long SampRGB[3];
// we know we'll need RGB bands so look for them specifically

// until we actually have something in those buffers, we'll use the CHAR buffers
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2] && ShortChannelNode[0])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		// The #? is to identify file type, the programtype is optional.
		//fprintf(fHandle,"#?%s %s\n",APP_TLA,ExtVersion);
		//fprintf(fHandle,"#?%s\n",APP_TLA);
		fprintf(fHandle,"#?RADIANCE\n");
		//fprintf(fHandle,"GAMMA=%g\n",1.0);
		//fprintf(fHandle,"EXPOSURE=%g\n",1.0);
		fprintf(fHandle,"FORMAT=32-bit_rle_rgbe\n\n");
		fprintf(fHandle, "-Y %d +X %d\n", BufHeight, BufWidth);
		AllIsWell = 1;
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if
else
	{
	// you should never get to this message if everything was done correctly in render init
	UserMessageOK("Image Save", "Unable to save image file because not all required buffers have been rendered.");
	} // else


// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	if (TransferBuf = (unsigned char *) AppMem_Alloc(BufWidth * 4, APPMEM_CLEAR, "RGBE Saver"))
		{
		// Write each line
		for (scanrow = 0; scanrow < BufHeight; scanrow++)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2] && ShortChannelData[0])
				{
				// Convert data into RGBE format
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractUnclippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					float2rgbe(rgbe, (float)(SampRGB[0])/255.0f, (float)(SampRGB[1])/255.0f,(float)(SampRGB[2])/255.0f);
					TransferBuf[xout++] = rgbe[0]; // R
					TransferBuf[xout++] = rgbe[1]; // G
					TransferBuf[xout++] = rgbe[2]; // B
					TransferBuf[xout++] = rgbe[3]; // E
					} // for
				// write the RGBE data
				if (fwrite(TransferBuf, 1, BufWidth * 4, fHandle) != (unsigned)(BufWidth * 4))
					{
					AllIsWell = 0;
					break;
					} // if
				} // if
			else
				{
				AllIsWell = 0;
				break;
				} // else
			} // for
		} // if
	else
		{
		AllIsWell = 0;
		} // else
	} // if file opened ok

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth * 4);
	TransferBuf = NULL;
	} // if

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatRGBE::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatFLX::StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
return(1);
#endif // !DEMO
} // ImageFormatFLX::StartAnim

/*===========================================================================*/

int ImageFormatFLX::EndAnim(void)
{
return(1);
} // ImageFormatFLX::EndAnim

/*===========================================================================*/

int ImageFormatFLX::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
return(0);
#endif // !DEMO
} // ImageFormatFLX::StartFrame

#endif // WCS_BUILD_HDR_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_JPEG_SUPPORT

/*
 * Here's the routine that will replace the standard error_exit method:
 */

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_JPEGSAVER_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/*===========================================================================*/

int ImageFormatJPEG::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
unsigned char AllIsWell = 0, SampRGB[3];
struct jpeg_compress_struct cinfo;
struct my_error_mgr jerr;
int row_stride = 0;			/* physical row width in buffer */
long xout, xscan;
int Quality = 100;
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
int WriteWorldFile = 0;
FILE *WF = NULL;
double N, S, W, E, pX, pY;
VertexDEM Corner;

if (IOE)
	{
	if (strstr(IOE->Codec, "With"))
		{
		WriteWorldFile = 1;
		} // if
	else // max
		{
		WriteWorldFile = 0;
		} // else
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (IOE)
	{
	if (!strcmp(IOE->Codec, "Low Quality"))
		{
		Quality = 25;
		} // if
	else if (!strcmp(IOE->Codec, "Med Quality"))
		{
		Quality = 50;
		} // if
	else if (!strcmp(IOE->Codec, "High Quality"))
		{
		Quality = 75;
		} // if
	else // max
		{
		Quality = 100;
		} // else
	} // if

// init JPEG library object
cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);
jerr.pub.error_exit = my_JPEGSAVER_error_exit;

// Establish the setjmp return context for my_error_exit to use.
if (setjmp(jerr.setjmp_buffer))
	{
	// If we get here, the JPEG code has signaled an error.
	// We need to clean up the JPEG object, close the input file, and return.
	if (TransferBuf && row_stride)
		{
		AppMem_Free(TransferBuf, row_stride);
		TransferBuf = NULL;
		} // if
	jpeg_destroy_compress(&cinfo);
	if (fHandle != NULL)
		fclose(fHandle);

	return(0);
	} // if

jpeg_create_compress(&cinfo);

// we know we'll need RGB bands so look for them specifically
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		AllIsWell = 1;
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if 

row_stride = BufWidth * 3;	/* JSAMPLEs per row in image_buffer */
// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	AllIsWell = 0;

	if (TransferBuf = (unsigned char *)
	 AppMem_Alloc(row_stride, APPMEM_CLEAR, "JPEG Saver"))
		{
		// initialize filestream output module
		jpeg_stdio_dest(&cinfo, fHandle);

		cinfo.image_width = BufWidth; 	/* image width and height, in pixels */
		cinfo.image_height = BufHeight;
		cinfo.input_components = 3;	/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality (&cinfo, Quality, 1);
		cinfo.dct_method = JDCT_FLOAT;

		jpeg_start_compress(&cinfo, TRUE);

		while (cinfo.next_scanline < cinfo.image_height)
			{
			//row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
			// fetch and interleave data in RGB order into Transferbuf
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(cinfo.next_scanline, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(cinfo.next_scanline, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(cinfo.next_scanline, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(cinfo.next_scanline, WCS_RASTER_BANDSET_SHORT);

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Interleave data into RGB order
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[0];
					TransferBuf[xout++] = SampRGB[1];
					TransferBuf[xout++] = SampRGB[2];
					} // for
				if (jpeg_write_scanlines(&cinfo, &TransferBuf, 1) != 1)
					{
					jpeg_abort_compress(&cinfo);
					break;
					} // if
				} // if
			else
				{
				jpeg_abort_compress(&cinfo);
				break;
				} // else
			} // while

		if (cinfo.next_scanline == cinfo.image_height)
			{
			jpeg_finish_compress(&cinfo);
			AllIsWell = 1;
			} // if
		} // if
	} // if

jpeg_destroy_compress(&cinfo);

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
if (AllIsWell && IOE && WriteWorldFile && RBounds)
	{
	// Filename.bpw
	FILE *PRJ = NULL;
	IOE->PAF.GetFramePathAndName(WorldFileName, ".jgw", Frame, 1000, IOE->AutoDigits);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			/*
			// Acquire boundaries of raster
			// center of UL (NW) corner pixel
			Corner.ScrnXYZ[0] = 0.5;
			Corner.ScrnXYZ[1] = 0.5;
			Corner.ScrnXYZ[2] = 1.0;
			UnprojectSys = RHost->DefCoords;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				UnprojectSys = RHost->Cam->Coords;
				} // if
			#endif // WCS_BUILD_VNS

			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				N = Corner.xyz[1]; 
				W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = Corner.Lat; 
				W = -Corner.Lon; // WorldFile uses GIS pos=east notation
				} // else

			// center of UL+1 pixel
			Corner.ScrnXYZ[0] = 1.5;
			Corner.ScrnXYZ[1] = 1.5;
			Corner.ScrnXYZ[2] = 1.0;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				pY = Corner.xyz[1] - N; 
				pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				pY = Corner.Lat - N; 
				pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
				} // else
			*/
			fprintf(WF, "%.15f\n", pX); // x dim
			fprintf(WF, "0.000\n"); // rot row
			fprintf(WF, "0.000\n"); // rot col
			fprintf(WF, "%.15f\n", -pY); // y dim
			fprintf(WF, "%f\n", W); // ul x = West
			fprintf(WF, "%f\n", N); // ul y = North
			fclose(WF);

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		} // if
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, row_stride);
	TransferBuf = NULL;
	} // if

if (fHandle != NULL)
	fclose(fHandle);

gEXIF.SaveEXIF(GetCompleteOutputPath());

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatJPEG::StartFrame

#endif // WCS_BUILD_JPEG_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_TIFF_SUPPORT
int ImageFormatTIFF::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow;
unsigned char AllIsWell = 0, Channels = 3, SampRGB[3];
TIFF *tiffh;
tsize_t linebytes;
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
int WriteWorldFile = 0;
double N, S, W, E, pX, pY;
VertexDEM Corner;

if (IOE)
	{
	if (strstr(IOE->Codec, "No"))
		{
		WriteWorldFile = 0;
		} // if
	else if (strstr(IOE->Codec, "WorldWind"))
		{
		WriteWorldFile = 2;
		} // if
	else // anything else
		{
		WriteWorldFile = 1;
		} // else
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

// we know we'll need RGB bands so look for them specifically
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if
else if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
#ifdef WCS_BUILD_VNS
	if (tiffh = XTIFFOpen(GlobalApp->MainProj->MungPath(GetCompleteOutputPath()), "w"))
#else // !WCS_BUILD_VNS
	if (tiffh = TIFFOpen(GlobalApp->MainProj->MungPath(GetCompleteOutputPath()), "w"))
#endif // !WCS_BUILD_VNS
		{
		AllIsWell = 1;
		} // if open ok
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if 

// Check and see if file opened successfully above
if ((tiffh != NULL) && (AllIsWell))
	{
	// Clear status bit for next stage of operation
	AllIsWell = 0;

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
	if (WriteWorldFile && RBounds && RBounds->FetchCoordSys())
		{
		CoordSys *GTCS = RBounds->FetchCoordSys();
		CoordSys TestCS;
		long CSMatch = 0, DoDatum = 0, DoEllipsoid = 0, DoPrimeMeridian = 0;
		// GeoTIFF writing inspired by AlphaPixel PixelSense, with permission
		GTIF	*gtif; // GeoKey-level descriptor
		gtif = GTIFNew(tiffh);

		// Insert the proper GeoTIFF keys here
		GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsArea);

		// Units & Cites
		if (GTCS->GetGeographic())
			{ // geographic of some sort
			GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic);
			GTIFKeySet(gtif, GeogAngularUnitsGeoKey, TYPE_SHORT, 1, Angular_Degree);
			GTIFKeySet(gtif, GeogCitationGeoKey, TYPE_ASCII, (int)strlen(GTCS->GetName()), GTCS->GetName());
			} // if
		else
			{ // projected
			GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
			GTIFKeySet(gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1, Linear_Meter);
			GTIFKeySet(gtif, PCSCitationGeoKey, TYPE_ASCII, (int)strlen(GTCS->GetName()), GTCS->GetName());
			} // else

		// encode and embed projection data
		if (GTCS->GetGeographic())
			{
			// tests ordered by expected frequency
			TestCS.SetSystemByCode(11);	// Geo NAD 83
			if (TestCS.GTEquals(GTCS))
				{
				GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_NAD83);
				CSMatch = 1;
				} // if
			if (! CSMatch)
				{
				TestCS.SetSystemByCode(12);	// Geo WGS 84
				if (TestCS.GTEquals(GTCS))
					{
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_WGS_84);
					CSMatch = 1;
					} // if
				} // if
			if (! CSMatch)
				{
				TestCS.SetSystemByCode(13);	// Geo NAD 27
				if (TestCS.GTEquals(GTCS))
					{
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_NAD27);
					CSMatch = 1;
					} // if
				} // if
			if (! CSMatch)
				{
				TestCS.SetSystemByCode(18);	// Geo ED 50
				if (TestCS.GTEquals(GTCS))
					{
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_ED50);
					CSMatch = 1;
					} // if
				} // if
			if (! CSMatch)
				{
				// Dump the hard way
				// Set GCS
				DoDatum = 1;
				} // if
			} // if Geo
		else
			{
			// tests ordered by expected frequency
			if (strcmp(GTCS->ProjSysName, "UTM - NAD 27") == 0)	// UTM NAD 27
				{
				// see if zone in GeoTIFF range 3N to 22N (even if North) for standard def
				if ((GTCS->ZoneID >= 4) && (GTCS->ZoneID <= 42) && ((GTCS->ZoneID & 0x1) == 0))
					{
					TestCS.SetSystemByCode(2);	// UTM NAD 27
					TestCS.SetZoneByCode(GTCS->ZoneID);
					if (TestCS.GTEquals(GTCS))
						{
						unsigned long ZoneNum = (unsigned long)(GTCS->ZoneID + 2) >> 1;

						GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 26700 + ZoneNum);
						CSMatch = 1;
						} // if
					} // if
				} // if
			else if (strcmp(GTCS->ProjSysName, "UTM - NAD 83") == 0)
				{
				// see if zone in GeoTIFF range 3N to 22N (even if North) for standard def
				if ((GTCS->ZoneID >= 4) && (GTCS->ZoneID <= 42) && ((GTCS->ZoneID & 0x1) == 0))
					{
					TestCS.SetSystemByCode(3);	// UTM NAD 83
					TestCS.SetZoneByCode(GTCS->ZoneID);
					if (TestCS.GTEquals(GTCS))
						{
						unsigned long ZoneNum = (unsigned long)(GTCS->ZoneID + 2) >> 1;

						GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 26900 + ZoneNum);
						CSMatch = 1;
						} // if
					} // if
				} // if
			else if (strcmp(GTCS->ProjSysName, "UTM - WGS 84") == 0)
				{
				// all zones exist in standard def
				TestCS.SetSystemByCode(10);	// UTM WGS 84
				TestCS.SetZoneByCode(GTCS->ZoneID);
				if (TestCS.GTEquals(GTCS))
					{
					unsigned long ZoneNum;
					
					if (GTCS->ZoneID & 0x1)
						{ // South
						ZoneNum = (unsigned long)(GTCS->ZoneID - 1) >> 1;
						GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 32700 + ZoneNum);
						} // if
					else
						{ // North
						ZoneNum = (unsigned long)(GTCS->ZoneID + 2) >> 1;
						GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 32600 + ZoneNum);
						} // else
					CSMatch = 1;
					} // if
				} // else if
			else if (strcmp(GTCS->ProjSysName, "UTM - ED 50") == 0)
				{
				// see if zone in GeoTIFF range 28N to 38N (even if North) for standard def
				if ((GTCS->ZoneID >= 54) && (GTCS->ZoneID <= 74) && ((GTCS->ZoneID & 0x1) == 0))
					{
					TestCS.SetSystemByCode(9);	// UTM ED 50
					TestCS.SetZoneByCode(GTCS->ZoneID);
					if (TestCS.GTEquals(GTCS))
						{
						unsigned long ZoneNum = (unsigned long)(GTCS->ZoneID + 2) >> 1;

						GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, 23000 + ZoneNum);
						CSMatch = 1;
						} // if
					} // if
				} // else if
			else if ((strcmp(GTCS->ProjSysName, "US State Plane - NAD 83") == 0) || (strcmp(GTCS->ProjSysName, "US State Plane - NAD 27") == 0))
				{
				unsigned short EPSGCode;

				EPSGCode = GTCS->GetEPSGStatePlaneCode();
				if (EPSGCode > 0)
					{
					GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, EPSGCode);
					CSMatch = 1;
					} // if
				} // else if
			else if (strcmp(GTCS->ProjSysName, "UK National Grid") == 0)
				{
				GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, PCS_British_National_Grid);
				CSMatch = 1;
				DoDatum = 1;
				} // else if
			/***
			else if (strcmp(GTCS->ProjSysName, "New Zealand Map Grid") == 0)
				{
				} // else if
			***/
			else if (strcmp(GTCS->ProjSysName, "Universal Polar Stereographic") == 0)
				{
				ProjectionMethod *PM = &GTCS->Method;

				GTIFKeySet(gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, CT_PolarStereographic);
				GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
				GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
				GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
				GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
				CSMatch = 1;
				DoDatum = 1;
				} // else if
			if (! CSMatch)
				{
				const char *DN = GTCS->Datum.DatumName;
				unsigned short CTCode;

				// Dump the hard way
				// Set PCS
				// Set Projection
				// Set Projection Method (GeoTIFF Coordinate Transform)
				CTCode = GTCS->Method.GetGeoTIFFMethodCode();
				if (CTCode == 0)
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "GeoTIFF err: The Projection Method used is unsupported.");
				else
					{
					ProjectionMethod *PM = &GTCS->Method;

					GTIFKeySet(gtif, ProjectionGeoKey, TYPE_SHORT, 1, KvUserDefined);
					GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, KvUserDefined);
					GTIFKeySet(gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, CTCode);
					switch (CTCode)
						{
						default:
							break;
						// GeoTIFF name (VNS name)
						case 1:	// TransverseMercator (Transverse Mercator)
							GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLongGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[4].GetCurValue());
							break; // keys match GeoTIFF sample
						case 2: // TransvMercator_Modified_Alaska (Modified Stereographic Conformal)
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							break;
						case 3: // ObliqueMercator (Hotine Oblique Mercator, B)
							GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjAzimuthAngleGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[4].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[5].GetCurValue());
							break; // keys match GeoTIFF sample
						case 7: // Mercator (Mercator)
							GTIFKeySet(gtif, ProjNatOriginLongGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey , TYPE_DOUBLE, 1, 1.0);
							break; // keys match GeoTIFF sample
						case 8: // LambertConfConic_2SP (Lambert Conformal Conic)
							GTIFKeySet(gtif, ProjStdParallel1GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjStdParallel2GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseOriginLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseOriginLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[4].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[5].GetCurValue());
							break; // keys match GeoTIFF sample
						case 10: // LambertAzimEqualArea (Lambert Azimuthal Equal Area)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 11: // AlbersEqualArea (Albers Conical Equal Area)
							GTIFKeySet(gtif, ProjStdParallel1GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjStdParallel2GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[4].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[5].GetCurValue());
							break; // keys match GeoTIFF sample
						case 12: // AzimuthalEquidistant (Azimuthal Equidistant)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 13: // EquidistantConic (Equidistant Conic, B)
							GTIFKeySet(gtif, ProjStdParallel1GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjStdParallel2GeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[4].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[5].GetCurValue());
							break; // keys match GeoTIFF sample
						case 14: // Stereographic (Stereographic)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 15: // PolarStereographic (Polar Stereographic)
							GTIFKeySet(gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, 1.0);
							break; // keys match GeoTIFF sample
						case 17: // Equirectangular (Equirectangular)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 19: // Gnomonic (Gnomonic)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 20: // MillerCylindrical (Miller Cylindrical)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							break; // keys match GeoTIFF sample
						case 21: // Orthographic (Orthographic)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 22: // Polyconic (Polyconic)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjCenterLatGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							break; // keys match GeoTIFF sample
						case 23: // Robinson (Robinson)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							break; // keys match GeoTIFF sample
						case 24: // Sinusoidal (Sinusoidal)
							GTIFKeySet(gtif, ProjCenterLongGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							break; // keys match GeoTIFF sample
						case 25: // VanDerGrinten (Van der Grinten)
							GTIFKeySet(gtif, ProjNatOriginLongGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[0].GetCurValue());
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey , TYPE_DOUBLE, 1, PM->AnimPar[1].GetCurValue());
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[2].GetCurValue());
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, PM->AnimPar[3].GetCurValue());
							GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey , TYPE_DOUBLE, 1, 1.0);
							break; // keys match GeoTIFF sample
						case 26: // NewZealandMapGrid (New Zealand Map Grid)
							GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, -41.0);
							GTIFKeySet(gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE, 1, 173.0);
							GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, 2510000.0);
							GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, 6023150.0);
							break; // keys match GeoTIFF sample
						} // switch
					} // else
				// Set GCS
				if (strcmp(DN, "North American 1927 - Continental US") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_NAD27);
				else if (strcmp(DN, "North American 1983 - Continental US") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_NAD83);
				else if (strcmp(DN, "European 1950 - Mean") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_ED50);
				else if (strcmp(DN, "World Geodetic System 1984") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_WGS_84);
				else if (strcmp(DN, "World Geodetic System 1972") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_WGS_72);
				else if (strcmp(DN, "Australian Geodetic Datum 1984") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_AGD84);
				else if (strcmp(DN, "Australian Geodetic Datum 1966") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_AGD66);
				else if (strcmp(DN, "Pulkovo 1942") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_Pulkovo_1942);
				else if (strcmp(DN, "Monte Mario") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_Monte_Mario);
				else if (strcmp(DN, "New Zealand Geodetic Datum 1949") == 0)
					GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_GD49); // a guess
				else if (strcmp(DN, "Ordnance Survey Great Britain 1936") == 0) // several datums have this name
					{
					if ((GTCS->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].GetCurValue() == 371.0) &&
						(GTCS->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].GetCurValue() == -112.0) &&
						(GTCS->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].GetCurValue() == 434.0))
						GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_OSGB_1936);
					else
						DoDatum = 1;
					} // else if
				else
					DoDatum = 1;
				} // if !CSMatch
			} // else Projected

		if (DoDatum)
			{
			unsigned short DatumID;

			// Assumes name is "Custom" if any mods to datum values or ellipsoid are made
			DatumID = GTCS->Datum.GetEPSGDatumCodeFromName(GTCS->Datum.DatumName);
			if (strcmp(GTCS->Datum.DatumName, "Ordnance Survey Great Britain 1936") == 0)
				{
				// assume all OS36 equate to this datum key unless we find out otherwise - can't find a way to encode deltas in GeoTIFF
				GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT, 1, 6277);
				DoEllipsoid = 1;
				} // else if
			else if (DatumID > 0)
				{
				GTIFKeySet(gtif, GeogGeodeticDatumGeoKey, TYPE_SHORT, 1, DatumID);
				} // if
			else
				DoEllipsoid = 1;
			} // if DoDatum

		if (DoEllipsoid)
			{
			double smajor, sminor, inv_flat;
			unsigned short EllipsoidID;

			// Assumes name is "Custom" if any params are non-standard
			EllipsoidID = GTCS->Datum.Ellipse.GetEPSGEllipsoidCodeFromName(GTCS->Datum.Ellipse.EllipsoidName);
			if (EllipsoidID > 0)
				{
				GTIFKeySet(gtif, GeogEllipsoidGeoKey, TYPE_SHORT, 1, EllipsoidID);
				} // if
			else
				{
				smajor = GTCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].GetCurValue();
				sminor = GTCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetCurValue();
				if (smajor != sminor)
					inv_flat = smajor / (smajor - sminor); // 1/f = a/(a-b)
				else
					{
					inv_flat = DBL_MAX;	// GeoAPI convention (Positive Infinity)
					GTIFKeySet(gtif, GeogEllipsoidGeoKey, TYPE_SHORT, 1, 32767);
					} // else
				GTIFKeySet(gtif, GeogSemiMajorAxisGeoKey, TYPE_DOUBLE, 1, smajor);
				GTIFKeySet(gtif, GeogSemiMinorAxisGeoKey, TYPE_DOUBLE, 1, sminor);
				GTIFKeySet(gtif, GeogInvFlatteningGeoKey, TYPE_DOUBLE, 1, inv_flat);
				DoPrimeMeridian = 1;
				} // else
			} // if DoEllipsoid

		if (DoPrimeMeridian)
			{
			GTIFKeySet(gtif, GeogPrimeMeridianGeoKey , TYPE_SHORT, 1, PM_Greenwich);
			} // if DoPrimeMeridian

		// any other stuff

		// write the keys to the file
		GTIFWriteKeys(gtif); // need to do this here, not after TIFFSetField code below


		// Scale and Georef
		// NOTE: PixelIsArea requires bounds to be edges
		// xref: http://support.esri.com/index.cfm?fa=knowledgebase.techarticles.articleShow&d=19491
		if (RBounds->FetchBoundsEdgesGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
			{
			double PixelScale[3];
			double tiePoints[6]  = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

			PixelScale[0] = pX;
			PixelScale[1] = pY;
			PixelScale[2] = 0.0; // z -- not used
			TIFFSetField(tiffh, TIFFTAG_GEOPIXELSCALE, 3, PixelScale);

			tiePoints[3] = W;
			tiePoints[4] = N;
			TIFFSetField(tiffh, TIFFTAG_GEOTIEPOINTS, 6, tiePoints);
			} // if

		} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

	TIFFSetField (tiffh, TIFFTAG_XRESOLUTION, 72.0f);  // set the DPI of the image
	TIFFSetField(tiffh, TIFFTAG_YRESOLUTION, 72.0f);    // set the DPI of the image
	TIFFSetField (tiffh, TIFFTAG_IMAGEWIDTH, BufWidth);  // set the width of the image
	TIFFSetField(tiffh, TIFFTAG_IMAGELENGTH, BufHeight);    // set the height of the image
	TIFFSetField(tiffh, TIFFTAG_SAMPLESPERPIXEL, Channels);   // set number of channels per pixel
	TIFFSetField(tiffh, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
	TIFFSetField(tiffh, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
	//   Some other essential fields to set that you do not have to understand for now.
	TIFFSetField(tiffh, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tiffh, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	linebytes = Channels * BufWidth;

	TIFFSetField(tiffh, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiffh, (unsigned long)-1));

	//if (TransferBuf = (unsigned char *)AppMem_Alloc(BufWidth * 3, APPMEM_CLEAR, "TIFF Saver"))
	tsize_t TIFFScanlineSizeTemp = TIFFScanlineSize(tiffh);
	if (TransferBuf = (unsigned char *)_TIFFmalloc(max(TIFFScanlineSizeTemp, linebytes)))
		{
		// Write each line
		for (scanrow = 0; scanrow < BufHeight; scanrow++)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
			if (Channels == 4)
				{
				CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				} // if

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Interleave data into RGB format
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
					TransferBuf[xout++] = SampRGB[0];
					TransferBuf[xout++] = SampRGB[1];
					TransferBuf[xout++] = SampRGB[2];
					// Alpha
					if (Channels == 4)
						{
						TransferBuf[xout++] = CharChannelData[3][xscan];
						} // if
					} // for
				// write the interleaved data
				if (TIFFWriteScanline(tiffh, TransferBuf, scanrow, 0) < 0)
					{
					// error
					break;
					} // if
				} // if
			else
				{
				break;
				} // else
			} // for
		if (scanrow == BufHeight)
			{
			AllIsWell = 1;
			} // if
		} // if
	} // if file opened ok

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
if (AllIsWell && IOE && WriteWorldFile == 1 && RBounds)
	{
	FILE *WF = NULL;
	FILE *PRJ = NULL;
	// Filename.tfw
	IOE->PAF.GetFramePathAndName(WorldFileName, ".tfw", Frame, 1000, IOE->AutoDigits);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			fprintf(WF, "%.15f\n", pX); // x dim
			fprintf(WF, "0.000\n"); // rot row
			fprintf(WF, "0.000\n"); // rot col
			fprintf(WF, "%.15f\n", -pY); // y dim
			fprintf(WF, "%f\n", W); // ul x = West
			fprintf(WF, "%f\n", N); // ul y = North
			fclose(WF);
			
			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		} // if
	} // if

// WorldWind overlays.txt pseudo-worldfile
if (AllIsWell && IOE && WriteWorldFile == 2 && RBounds)
	{
	PathAndFile OverlayPAF;
	FILE *WF = NULL;
	// Filename.tfw
	OverlayPAF.SetPath(IOE->PAF.GetPath());
	OverlayPAF.SetName("overlays");
	OverlayPAF.GetFramePathAndName(WorldFileName, ".txt", 0, 1000, 0);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			fprintf(WF, "%s\t%s\t", IOE->PAF.GetName(), GlobalApp->MainProj->MungPath(GetCompleteOutputPath()));
			fprintf(WF, "%.15f\t%.15f\t", N, W); // UL
			fprintf(WF, "%.15f\t%.15f\t", N, E); // UR
			fprintf(WF, "%.15f\t%.15f\t", S, W); // LL
			fprintf(WF, "%.15f\t%.15f\t", S, E); // LR
			fprintf(WF, "%.15f\t%.15f\t", (N + S) * .5, (W + E) * .5); // MM
			fprintf(WF, "%.15f\t%.15f\t", (N + S) * .5, E); // MR
			fprintf(WF, "255\n"); // MR
			fclose(WF);
			} // if
		} // if
	} // if


#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (TransferBuf)
	{
	//AppMem_Free(TransferBuf, BufWidth * 3);
    _TIFFfree(TransferBuf);
	TransferBuf = NULL;
	} // if
if (tiffh != NULL)
	{
#ifdef WCS_BUILD_VNS
	XTIFFClose(tiffh);
#else // !WCS_BUILD_VNS
	TIFFClose(tiffh);
#endif // !WCS_BUILD_VNS
	} // if

gEXIF.SaveEXIF(GetCompleteOutputPath());

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatTIFF::StartFrame

#endif // WCS_BUILD_TIFF_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_BUILD_PNG_SUPPORT
int ImageFormatPNG::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow;
unsigned char AllIsWell = 0, Channels = 3, SampRGB[3];
unsigned char UnPreMultAlpha = 0;
png_structp png_ptr = NULL;
png_infop info_ptr = NULL;
int CompressLevel = Z_DEFAULT_COMPRESSION;
time_t Now;
png_text pngtext[3];
png_time PNow;
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
int WriteWorldFile = 0;
FILE *WF = NULL;
double N, S, W, E, pX, pY;
VertexDEM Corner;

if (IOE)
	{
	if (strstr(IOE->Codec, "With"))
		{
		WriteWorldFile = 1;
		} // if
	else // max
		{
		WriteWorldFile = 0;
		} // else
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (IOE)
	{
	if (strstr(IOE->Codec, "Less"))
		{
		CompressLevel = 3;
		} // if
	else if (strstr(IOE->Codec, "Normal"))
		{
		CompressLevel = 5;
		} // if
	else if (strstr(IOE->Codec, "More"))
		{
		CompressLevel = 7;
		} // if
	else // max
		{
		CompressLevel = Z_BEST_COMPRESSION; // 9
		} // else
	} // if


// we know we'll need RGB bands so look for them specifically
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if
else if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	Channels = 4;
	} // if

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
		{
		if (png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL))
			{
			if (info_ptr = png_create_info_struct(png_ptr))
				{
				AllIsWell = 1;
				} // if
			} // if
		else
			{
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			UserMessageOK(GetCompleteOutputPath(),
				"Can't initialize PNG saver!\nOperation terminated.");
			} // else
		} /* if open ok */
	else
		{
		UserMessageOK(GetCompleteOutputPath(),
			"Can't open image file for output!\nOperation terminated.");
		} // else
	} // if 

if (Channels == 4 && strstr(IOE->Codec, "NonPremultAlpha"))
	{
	UnPreMultAlpha = 1;
	} // if

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	// Clear status bit for next stage of operation
	AllIsWell = 0;

	// set up error handling
	if (setjmp(png_jmpbuf(png_ptr)))
		{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fHandle);
		if (TransferBuf)
			{
			AppMem_Free(TransferBuf, BufWidth * Channels);
			TransferBuf = NULL;
			} // if
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
		return (0);
		}

    png_init_io(png_ptr, fHandle);
	png_set_compression_level(png_ptr, CompressLevel);
	png_set_IHDR(png_ptr, info_ptr, BufWidth, BufHeight, 8, (Channels == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB),
	 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	//png_set_gAMA(png_ptr, info_ptr, 1.0);
    (void)time(&Now);
	png_convert_from_time_t(&PNow, Now);
	png_set_tIME(png_ptr, info_ptr, &PNow);
	pngtext[0].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[0].key  = "Title";
	pngtext[0].text = "";
	if (IOE)
		{
		pngtext[0].text = (char *)IOE->PAF.GetName();
		} // if
	pngtext[0].text_length = strlen(pngtext[0].text);

	pngtext[1].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[1].key  = "Author";
	pngtext[1].text = GlobalApp->MainProj->UserName;
	pngtext[1].text_length = strlen(pngtext[1].text);

	pngtext[2].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[2].key  = "Software";
	pngtext[2].text = APP_TITLE " V" APP_VERS;
	pngtext[2].text_length = strlen(pngtext[2].text);

	png_set_text(png_ptr, info_ptr, pngtext, 3);

	png_write_info(png_ptr, info_ptr);


	if (TransferBuf = (unsigned char *)AppMem_Alloc(BufWidth * Channels, APPMEM_CLEAR, "PNG Saver"))
		{
		// Write each line
		for (scanrow = 0; scanrow < BufHeight; scanrow++)
			{
			CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
			if (ShortChannelNode[0])
				ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
			if (Channels == 4)
				{
				CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				} // if

			if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
				{
				// Interleave data into RGB format
				for (xout = xscan = 0; xscan < BufWidth; xscan++)
					{
					SampRGB[0] = CharChannelData[0][xscan];
					SampRGB[1] = CharChannelData[1][xscan];
					SampRGB[2] = CharChannelData[2][xscan];
					if (ShortChannelData[0] && ShortChannelData[0][xscan])
						rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);

					// Un-premult the RGB via the Alpha value
					if (UnPreMultAlpha)
						{
						double InvAlphaScale;
						unsigned char AlphaValue;
						
						AlphaValue = CharChannelData[3][xscan];
						
						if (AlphaValue > 0 && AlphaValue < 255)
							{
							InvAlphaScale = 255.0 / (double)(AlphaValue + 1);
							SampRGB[0] = (unsigned char)(InvAlphaScale * (double)SampRGB[0]);
							SampRGB[1] = (unsigned char)(InvAlphaScale * (double)SampRGB[1]);
							SampRGB[2] = (unsigned char)(InvAlphaScale * (double)SampRGB[2]);
							} // if
						} // if


					TransferBuf[xout++] = SampRGB[0];
					TransferBuf[xout++] = SampRGB[1];
					TransferBuf[xout++] = SampRGB[2];
					// Alpha
					if (Channels == 4)
						{
						TransferBuf[xout++] = CharChannelData[3][xscan];
						} // if
					} // for
				// write the interleaved data
				png_write_row(png_ptr, TransferBuf);
				} // if
			else
				{
				break;
				} // else
			} // for
		if (scanrow == BufHeight)
			{
		    png_write_end(png_ptr, NULL);
			AllIsWell = 1;
			} // if
		} // if
	} // if file opened ok

if (png_ptr && info_ptr)
	png_destroy_write_struct(&png_ptr, &info_ptr);

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
if (AllIsWell && IOE && WriteWorldFile && RBounds)
	{
	// Filename.bpw
	FILE *PRJ = NULL;

	IOE->PAF.GetFramePathAndName(WorldFileName, ".pgw", Frame, 1000, IOE->AutoDigits);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			/*
			// Acquire boundaries of raster
			// center of UL (NW) corner pixel
			Corner.ScrnXYZ[0] = 0.5;
			Corner.ScrnXYZ[1] = 0.5;
			Corner.ScrnXYZ[2] = 1.0;
			UnprojectSys = RHost->DefCoords;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				UnprojectSys = RHost->Cam->Coords;
				} // if
			#endif // WCS_BUILD_VNS

			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				N = Corner.xyz[1]; 
				W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = Corner.Lat; 
				W = -Corner.Lon; // WorldFile uses GIS pos=east notation
				} // else

			// center of UL+1 pixel
			Corner.ScrnXYZ[0] = 1.5;
			Corner.ScrnXYZ[1] = 1.5;
			Corner.ScrnXYZ[2] = 1.0;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				pY = Corner.xyz[1] - N; 
				pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				pY = Corner.Lat - N; 
				pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
				} // else
			*/
			fprintf(WF, "%.15f\n", pX); // x dim
			fprintf(WF, "0.000\n"); // rot row
			fprintf(WF, "0.000\n"); // rot col
			fprintf(WF, "%.15f\n", -pY); // y dim
			fprintf(WF, "%f\n", W); // ul x = West
			fprintf(WF, "%f\n", N); // ul y = North
			fclose(WF);

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		} // if
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth * Channels);
	TransferBuf = NULL;
	} // if
if (fHandle != NULL)
	fclose(fHandle);

gEXIF.SaveEXIF(GetCompleteOutputPath());

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
		
} // ImageFormatPNG::StartFrame

#endif // WCS_BUILD_PNG_SUPPORT

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatPICT::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
char Channels;
unsigned char *MBuf = NULL;

PICTdata SaveDat;
ImSaverLocal ISL;
unsigned char Success = 1;

SaveDat.NewMode  = 1;
SaveDat.d_width  = BufWidth;
SaveDat.d_height = BufHeight;

Channels = 3;
SaveDat.DoAlpha = 0;
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	SaveDat.DoAlpha = 1;
	Channels = 4;
	} // if
else if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	SaveDat.DoAlpha = 1;
	Channels = 4;
	} // if

// Nasty cast to shoehorn new pointers into old structure
SaveDat.Rbuf = (unsigned char *)CharChannelNode[0];
SaveDat.Gbuf = (unsigned char *)CharChannelNode[1];
SaveDat.Bbuf = (unsigned char *)CharChannelNode[2];
SaveDat.Abuf = (unsigned char *)CharChannelNode[3];

if (MBuf = (unsigned char *)AppMem_Alloc(Channels * BufWidth, NULL))
	{
	SaveDat.d_LineBuf = MBuf;
	SaveDat.d_LineBufSize = Channels * BufWidth;
	ISL.filename = GetCompleteOutputPath();
	ISL.type = PICT_IMG_RGB24;
	ISL.result = PICT_IPSTAT_OK;
	ISL.priv_data = &SaveDat;
	ISL.sendData = PICTHostSendData;
	ISL.monitor = NULL;

	PICTMainSaver(&ISL);
	if (ISL.result == PICT_IPSTAT_OK) Success = 0;
	AppMem_Free(MBuf, Channels * BufWidth);
	MBuf = NULL;
	} // if

return(Success);
#endif // DEMO

} // ImageFormatPICT::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatRAW::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long RawBand, xout, xscan, scanrow;
unsigned char AllIsWell = 0, ChannelBufferSize = 0 /*, AlphaOk = 1 */;
float /* *FTransferBuf, */ incr;
BufferNode *RedChan, *GrnChan, *BluChan;
#ifdef WCS_BUILD_WORLDFILE_SUPPORT
int WriteWorldFile = 0;
FILE *WF = NULL;
double N, S, W, E, pX, pY;
VertexDEM Corner;

if (IOE)
	{
	if (strstr(IOE->Codec, "With"))
		{
		WriteWorldFile = 1;
		} // if
	else // max
		{
		WriteWorldFile = 0;
		} // else
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (!IOE) return(1); // Dunno what to do without an ImageOutputEvent to tell us about channels...

// identify RGB buffers for later comparison
RedChan = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
GrnChan = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
BluChan = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);

// Find nodes for all bands, note max band size
for (RawBand = 0; RawBand < WCS_MAX_IMAGEOUTBUFFERS; RawBand++)
	{
	if (IOE->OutBuffers[RawBand][0])
		{
		if (CharChannelNode[RawBand]  = Buffers->FindBufferNode(IOE->OutBuffers[RawBand], WCS_RASTER_BANDSET_BYTE))
			{
			if (Interleave)
				{
				ChannelBufferSize += sizeof(char);
				} // if
			else
				{
				ChannelBufferSize = max(ChannelBufferSize, sizeof(char));
				} // else
			} // if
		if (FloatChannelNode[RawBand] = Buffers->FindBufferNode(IOE->OutBuffers[RawBand], WCS_RASTER_BANDSET_FLOAT))
			{
			if (Interleave)
				{
				// We don't support interleaved float channels, so don't add them
				//ChannelBufferSize += sizeof(float);
				} // if
			else
				{
				ChannelBufferSize = max(ChannelBufferSize, sizeof(float));
				} // else
			} // if
		} // if
	else
		{
		break;
		} // else
	} // for

ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (ForceByte)
	{
	// Get extrema for float channels
	for (RawBand = 0; RawBand < WCS_MAX_IMAGEOUTBUFFERS; RawBand++)
		{
		FCDMax[RawBand] = -FLT_MAX;
		FCDMin[RawBand] = FLT_MAX;
		if (FloatChannelNode[RawBand])
			{
			AllIsWell = 1;
			for (scanrow = 0; AllIsWell && (scanrow < BufHeight); scanrow++)
				{
				if (FloatChannelData[RawBand] = (float *)FloatChannelNode[RawBand]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT))
					{
					for (xscan = 0; xscan < BufWidth; xscan++)
						{
						float FVal = FloatChannelData[RawBand][xscan];
						if (FVal != FLT_MAX)
							{
							if (FVal > FCDMax[RawBand]) FCDMax[RawBand] = FVal;
							if (FVal < FCDMin[RawBand]) FCDMin[RawBand] = FVal;
							} // if
						} // for
					} // if
				else
					{
					AllIsWell = 0;
					break;
					} // else
				} // for
			} // if
		} // for
	} // if

AllIsWell = 0;
if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb+"))
	{
	AllIsWell = 1;
	} /* if open ok */
else
	{
	UserMessageOK(GetCompleteOutputPath(),
		"Can't open image file for output!\nOperation terminated.");
	} // else

// Check and see if file opened successfully above
if ((fHandle != NULL) && (AllIsWell))
	{
	FILE *fPRC;
	char fnamePRC[WCS_PATHANDFILE_PATH_LEN + WCS_PATHANDFILE_NAME_LEN + 4];

	if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("GenPRC"))
		{
		// Write Project Ref Coords
		strcpy(fnamePRC, IOE->PAF.GetPath());
		strcat(fnamePRC, IOE->PAF.GetName());
		strcat(fnamePRC, ".prc");
		if (fPRC = PROJ_fopen(fnamePRC, "wb"))
			{
			fwrite(&GlobalApp->MainProj->Interactive->VE.ProjRefCoord[1], sizeof(double), 1, fPRC);	// Lat
			fwrite(&GlobalApp->MainProj->Interactive->VE.ProjRefCoord[0], sizeof(double), 1, fPRC); // Lon
			fwrite(&GlobalApp->MainProj->Interactive->VE.ProjRefCoord[2], sizeof(double), 1, fPRC);	// Elev
			fclose(fPRC);
			} // if
		} // if

	// Set status bit for next stage of operation --> F2_NOTE: If we're in here, it's already set!
	//AllIsWell = 1;

	if (TransferBuf = (unsigned char *)AppMem_Alloc(BufWidth * ChannelBufferSize, APPMEM_CLEAR, "RAW Saver"))
		{
		//FTransferBuf = (float *)TransferBuf;
		if (Interleave)
			{
			// Write data by scanline, interleaved by channel (assuming that's useful)
			for (scanrow = 0; AllIsWell && (scanrow < BufHeight); scanrow++)
				{
				// First, get a dataline for each channel
				for (RawBand = 0; AllIsWell && (RawBand < WCS_MAX_IMAGEOUTBUFFERS); RawBand++)
					{
					if (CharChannelNode[RawBand])
						{
						CharChannelData[RawBand] = (unsigned char *)CharChannelNode[RawBand]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
						} // if
					else if (FloatChannelNode[RawBand])
						{ // Ignore, it's just not sane to write interleaved float channels
						} // if
					else
						{
						break;
						} // else
					} // for

				if (ShortChannelNode[0])
					ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);

				// Now interleave them into Transferbuf
				for (xout = 0, xscan = 0; xscan < BufWidth; xscan++)
					{
					for (RawBand = 0; AllIsWell && (RawBand < WCS_MAX_IMAGEOUTBUFFERS); RawBand++)
						{
						if (CharChannelData[RawBand])
							{
							TransferBuf[xout] = CharChannelData[RawBand][xscan];
							// perform HDR clamping if necessary
							if (ShortChannelData[0])
								{
								if (CharChannelNode[RawBand] == RedChan)
									{
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xout], ShortChannelData[0][xscan], 0); // 0=red
									} // if
								else if (CharChannelNode[RawBand] == GrnChan)
									{
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xout], ShortChannelData[0][xscan], 1); // 1=green
									} // if
								else if (CharChannelNode[RawBand] == BluChan)
									{
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xout], ShortChannelData[0][xscan], 2); // 2=blue
									} // if
								} // if
							xout++;
							} // if
						else if (FloatChannelNode[RawBand])
							{ // Ignore, it's just not sane to write interleaved float channels
							} // if
						else
							{
							break;
							} // else
						} // for
					} // for

				// write the data
				if (fwrite(TransferBuf, 1, BufWidth * ChannelBufferSize, fHandle) != (unsigned)(BufWidth * ChannelBufferSize))
					{
					AllIsWell = 0;
					break;
					} // if
				} // for
			/*
							// Interleave data
							for (xout = xscan = 0; xscan < BufWidth; xscan++)
								{
								TransferBuf[xout++] = CharChannelData[2][xscan];
								TransferBuf[xout++] = CharChannelData[1][xscan];
								TransferBuf[xout++] = CharChannelData[0][xscan];
								if (CharChannelData[3])
									{
									TransferBuf[xout++] = CharChannelData[3][xscan];
									} // if
								} // for
			*/
			} // if Interleave
		else
			{
			// write each channel
			for (RawBand = 0; AllIsWell && (RawBand < WCS_MAX_IMAGEOUTBUFFERS); RawBand++)
				{
				if (CharChannelNode[RawBand])
					{
					for (scanrow = 0; AllIsWell && (scanrow < BufHeight); scanrow++)
						{
						if (CharChannelData[RawBand] = (unsigned char *)CharChannelNode[RawBand]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE))
							{
							if (ShortChannelNode[0])
								ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
							if (ShortChannelData[0] && (CharChannelNode[RawBand] == RedChan))
								{ // HDR clamp red into TransferBuf
								for (xscan = 0; xscan < BufWidth; xscan++)
									{
									TransferBuf[xscan] = CharChannelData[RawBand][xscan];
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xscan], ShortChannelData[0][xscan], 0); // 0=red
									} // for
								if (fwrite(TransferBuf, 1, BufWidth * sizeof(char), fHandle) != (unsigned)(BufWidth * sizeof(char)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // if
							else if (ShortChannelData[0] && (CharChannelNode[RawBand] == GrnChan))
								{ // HDR clamp green into TransferBuf
								for (xscan = 0; xscan < BufWidth; xscan++)
									{
									TransferBuf[xscan] = CharChannelData[RawBand][xscan];
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xscan], ShortChannelData[0][xscan], 1); // 1=green
									} // for
								if (fwrite(TransferBuf, 1, BufWidth * sizeof(char), fHandle) != (unsigned)(BufWidth * sizeof(char)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // if
							else if (ShortChannelData[0] && (CharChannelNode[RawBand] == BluChan))
								{ // HDR clamp blue into TransferBuf
								for (xscan = 0; xscan < BufWidth; xscan++)
									{
									TransferBuf[xscan] = CharChannelData[RawBand][xscan];
									if (ShortChannelData[0][xscan])
										rPixelFragment::ExtractClippedExponentialColor(TransferBuf[xscan], ShortChannelData[0][xscan], 2); // 2=blue
									} // for
								if (fwrite(TransferBuf, 1, BufWidth * sizeof(char), fHandle) != (unsigned)(BufWidth * sizeof(char)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // if
							else
								{
								// write the data without HDR clipping
								if (fwrite(CharChannelData[RawBand], 1, BufWidth * sizeof(char), fHandle) != (unsigned)(BufWidth * sizeof(char)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // else
							} // if
						else
							{
							AllIsWell = 0;
							break;
							} // else
						} // for
					} // if
				else if (FloatChannelNode[RawBand])
					{
					incr = 1.0f;
					if (ForceByte)
						{
						incr = (float)((FCDMax[RawBand] - FCDMin[RawBand]) / 256.0);
						if (incr == 0.0f) incr = 1.0f; // prevent div-by-zero
						} // if
					for (scanrow = 0; AllIsWell && (scanrow < BufHeight); scanrow++)
						{
						if (FloatChannelData[RawBand] = (float *)FloatChannelNode[RawBand]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT))
							{
							// RAW data is endian-native, so no need to flip.
							if (ForceByte)
								{
								// Compress to 8-bit
								for (xscan = 0; xscan < BufWidth; xscan++)
									{
									if (FloatChannelData[RawBand][xscan] >= FCDMax[RawBand])
										TransferBuf[xscan] = 0;
									else
										TransferBuf[xscan] = (unsigned char)((FloatChannelData[RawBand][xscan] - FCDMin[RawBand]) / incr);
									} // for
								// write the data
								if (fwrite(TransferBuf, 1, BufWidth * sizeof(char), fHandle) != (unsigned)(BufWidth * sizeof(char)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // if
							else
								{
								// write the data
								if (fwrite(FloatChannelData[RawBand], 1, BufWidth * sizeof(float), fHandle) != (unsigned)(BufWidth * sizeof(float)))
									{
									AllIsWell = 0;
									break;
									} // if
								} // else
							} // if
						else
							{
							AllIsWell = 0;
							break;
							} // else
						} // for
					} // if
				else
					{
					break;
					} // else
				} // for
			} // else not interleaved
		} // if

	} // if file opened ok

#ifdef WCS_BUILD_WORLDFILE_SUPPORT
if (AllIsWell && IOE && WriteWorldFile && RBounds)
	{
	// Filename.bpw
	FILE *PRJ = NULL;

	IOE->PAF.GetFramePathAndName(WorldFileName, ".rww", Frame, 1000, IOE->AutoDigits);

	// Acquire boundaries of raster
	if (RBounds->FetchBoundsCentersGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
		{
		if (WF = PROJ_fopen(WorldFileName, "w")) // text mode, methinks
			{
			/*
			// Acquire boundaries of raster
			// center of UL (NW) corner pixel
			Corner.ScrnXYZ[0] = 0.5;
			Corner.ScrnXYZ[1] = 0.5;
			Corner.ScrnXYZ[2] = 1.0;
			UnprojectSys = RHost->DefCoords;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				UnprojectSys = RHost->Cam->Coords;
				} // if
			#endif // WCS_BUILD_VNS
	
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				N = Corner.xyz[1]; 
				W = UnprojectSys->Geographic ? -Corner.xyz[0]: Corner.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				N = Corner.Lat; 
				W = -Corner.Lon; // WorldFile uses GIS pos=east notation
				} // else

			// center of UL+1 pixel
			Corner.ScrnXYZ[0] = 1.5;
			Corner.ScrnXYZ[1] = 1.5;
			Corner.ScrnXYZ[2] = 1.0;
			#ifdef WCS_BUILD_VNS
			if (RHost->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && RHost->Cam->Projected && RHost->Cam->Coords)
				{
				RHost->Cam->UnProjectProjectedVertexDEM(&Corner, RHost->EarthLatScaleMeters);
				pY = Corner.xyz[1] - N; 
				pX = UnprojectSys->Geographic ? -Corner.xyz[0] - W: Corner.xyz[0] - W; // WorldFile uses GIS pos=east notation for 'Easting'
				} // if
			else
			#endif // WCS_BUILD_VNS
				{
				RHost->Cam->UnProjectVertexDEM(UnprojectSys, &Corner, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
				pY = Corner.Lat - N; 
				pX = -Corner.Lon - W; // WorldFile uses GIS pos=east notation
				} // else
			*/
			fprintf(WF, "%.15f\n", pX); // x dim
			fprintf(WF, "0.000\n"); // rot row
			fprintf(WF, "0.000\n"); // rot col
			fprintf(WF, "%.15f\n", -pY); // y dim
			fprintf(WF, "%f\n", W); // ul x = West
			fprintf(WF, "%f\n", N); // ul y = North
			fclose(WF);

			// Now try to write a PRJ file along with it
			IOE->PAF.GetFramePathAndName(WorldFileName, ".prj", Frame, 1000, IOE->AutoDigits);
			if (RBounds && RBounds->FetchCoordSys() && (PRJ = PROJ_fopen(WorldFileName, "w"))) // text mode, methinks
				{
				RBounds->FetchCoordSys()->SaveToArcPrj(PRJ);
				fclose(PRJ);
				PRJ = NULL;
				} // if
			} // if
		} // if
	} // if
#endif // WCS_BUILD_WORLDFILE_SUPPORT

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth * ChannelBufferSize);
	TransferBuf = NULL;
	} // if

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // DEMO

} // ImageFormatRAW::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatZBUF::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
unsigned char AllIsWell = 0, compression = 0;
long scanrow, i, BodySize;
float zbufmax = -FLT_MAX, zbufmin = FLT_MAX, *ZTransfer;
struct ILBMHeader Hdr;
struct ZBufferHeader ZBHdr;
unsigned long int TBufSize = 0;
unsigned char *ZLIBout = NULL;
long ZBODMarker = 0;

if (IOE)
	{
	if (!strcmp(IOE->Codec, "ZLIB Compressed"))
		{
		compression = 1;
		} // if
	else
		{
		compression = 0;
		} // else
	} // if

if (compression)
	{
	// need a full buffer for compression
	TBufSize = BufWidth * BufHeight * sizeof(float);
	TransferBuf = (unsigned char *)AppMem_Alloc(TBufSize, APPMEM_CLEAR, "ZBUF Saver");
	ZTransfer = (float *)TransferBuf;
	ZLIBout = (unsigned char *)AppMem_Alloc(TBufSize, APPMEM_CLEAR, "ZBUF Saver");
	if (!ZLIBout)
		{ // compression buffer unavailable, fall back to uncompressed
		compression = 0;
		} // if
	} // if
else
	{
	TBufSize = BufWidth * sizeof(float);
	TransferBuf = (unsigned char *)AppMem_Alloc(TBufSize, APPMEM_CLEAR, "ZBUF Saver");
	ZTransfer = (float *)TransferBuf;
	} // else

if (ZTransfer && (FloatChannelNode[0] = Buffers->FindBufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT)))
	{
	for (scanrow = 0; scanrow < BufHeight; scanrow++)
		{
		FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);

		for (i=0; i<BufWidth; i++)
			{
			if (FloatChannelData[0][i] == FLT_MAX)
				continue;
			if (FloatChannelData[0][i] > zbufmax)
				zbufmax = FloatChannelData[0][i];
			if (FloatChannelData[0][i] < zbufmin)
				zbufmin = FloatChannelData[0][i];
			} /* for i=0... */
		} // for
	if (zbufmax < 0.0)
		zbufmax = zbufmin = FLT_MAX;

	BodySize = (BufWidth * BufHeight) * sizeof(float);
	if (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb"))
		{
		ZBHdr.Width = BufWidth;
		ZBHdr.Height = BufHeight;
		ZBHdr.VarType = 6;			/* single precision float */
		ZBHdr.Compression = compression; // 1=zlib
		ZBHdr.Sorting = 0;			/* low = near */
		ZBHdr.Units = 2;			/* 1=mm, 2=meters, 3=kilometers, 4=in, 5=ft, 6=yds, 7=miles, 8=lightyears, 100=undefined */
		ZBHdr.Min = zbufmin;
		ZBHdr.Max = zbufmax;
		ZBHdr.Bkgrnd = FLT_MAX;
		ZBHdr.ScaleFactor = 1.0f;
		ZBHdr.ScaleBase = 0.0f;
		strncpy((char *)&Hdr.ChunkID[0], "FORM", 4);
		Hdr.ChunkSize = 12 + 36 + BodySize;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32S(Hdr.ChunkSize, &Hdr.ChunkSize);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(&Hdr, 1, 8, fHandle);
		strncpy((char *)&Hdr.ChunkID[0], "ILBM", 4);
		fwrite(&Hdr, 1, 4, fHandle);
		strncpy((char *)&Hdr.ChunkID[0], "ZBUF", 4);
		Hdr.ChunkSize = 36;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32S(Hdr.ChunkSize, &Hdr.ChunkSize);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(&Hdr, 1, 8, fHandle);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32U(ZBHdr.Width, &ZBHdr.Width);
		SimpleEndianFlip32U(ZBHdr.Height, &ZBHdr.Height);

		SimpleEndianFlip16U(ZBHdr.VarType, &ZBHdr.VarType);
		SimpleEndianFlip16U(ZBHdr.Compression, &ZBHdr.Compression);
		SimpleEndianFlip16U(ZBHdr.Sorting, &ZBHdr.Sorting);
		SimpleEndianFlip16U(ZBHdr.Units, &ZBHdr.Units);

		SimpleEndianFlip32F(&ZBHdr.Min, &ZBHdr.Min);
		SimpleEndianFlip32F(&ZBHdr.Max, &ZBHdr.Max);
		SimpleEndianFlip32F(&ZBHdr.Bkgrnd, &ZBHdr.Bkgrnd);
		SimpleEndianFlip32F(&ZBHdr.ScaleFactor, &ZBHdr.ScaleFactor);
		SimpleEndianFlip32F(&ZBHdr.ScaleBase, &ZBHdr.ScaleBase);

		#endif // BYTEORDER_LITTLEENDIAN
		fwrite(&ZBHdr, 1, 36, fHandle);
		strncpy((char *)&Hdr.ChunkID[0], "ZBOD", 4);
		Hdr.ChunkSize = BodySize;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32S(Hdr.ChunkSize, &Hdr.ChunkSize);
		#endif // BYTEORDER_LITTLEENDIAN
		if (compression)
			{ // mark spot for later rewite of body size
			ZBODMarker = ftell(fHandle);
			} // if
		fwrite(&Hdr, 1, 8, fHandle);

		int ZLIBIdx = 0;
		for (scanrow = 0; scanrow < BufHeight; scanrow++)
			{
			FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_FLOAT);

			if (compression)
				{
				for (i=0; i<BufWidth; i++)
					{
					#ifdef BYTEORDER_LITTLEENDIAN
					// flip the Z value
					SimpleEndianFlip32F(&FloatChannelData[0][i], &ZTransfer[ZLIBIdx++]);
					#else // !BYTEORDER_LITTLEENDIAN
					// copy the Z value
					ZTransfer[ZLIBIdx++] = FloatChannelData[0][i];
					#endif // !BYTEORDER_LITTLEENDIAN
					} /* for i=0... */
				} // if
			else
				{
				for (i=0; i<BufWidth; i++)
					{
					#ifdef BYTEORDER_LITTLEENDIAN
					// flip the Z value
					SimpleEndianFlip32F(&FloatChannelData[0][i], &ZTransfer[i]);
					#endif // BYTEORDER_LITTLEENDIAN
					} /* for i=0... */
				if (fwrite((char *)ZTransfer, 1, BufWidth * sizeof(float), fHandle) == (unsigned)(BufWidth * sizeof(float)))
					{
					AllIsWell = 1;
					} // if
				} // else
			} // for
		if (compression)
			{
			unsigned long int CompressedSize = TBufSize;
			if (compress(ZLIBout, &CompressedSize, TransferBuf, TBufSize) == Z_OK)
				{
				if (fwrite((char *)ZLIBout, 1, CompressedSize, fHandle) == CompressedSize)
					{
					AllIsWell = 1;
					} // if
				} // if

			// go back and rewite ZBOD chunk header with new chunk size
			Hdr.ChunkSize = CompressedSize;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32S(Hdr.ChunkSize, &Hdr.ChunkSize);
			#endif // BYTEORDER_LITTLEENDIAN
			fseek(fHandle, ZBODMarker, SEEK_SET);
			fwrite(&Hdr, 1, 8, fHandle);
			} // if


		} // if file open
	} // if z channel

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, TBufSize);
	} // if
if (ZLIBout)
	{
	AppMem_Free(ZLIBout, TBufSize);
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatZBUF::StartFrame

/*===========================================================================*/
/*===========================================================================*/

int ImageFormatZPIC::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
return(0);
#endif // !DEMO

} // ImageFormatZPIC::StartFrame

/*===========================================================================*/

// This is a BigEndian file format
int ImageFormatRLA::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
unsigned char Channels;
BOOL AllIsWell, SaveAA, /* SaveElev, */ SaveLat, SaveLon, SaveNormX, SaveNormY, SaveNormZ, SaveObj, SaveZ;
RLAHeader rlahdr;
ULONG WCSgbuffers = 0;
float *zline = NULL;
RasterAnimHost **ObjectID = NULL;
char CompName[128], UserName[128];
DWORD sighs;
LPTSTR lpBuffer;
LPDWORD nSize;
unsigned char *ClampedRedBuf = NULL, *ClampedGrnBuf = NULL, *ClampedBluBuf = NULL;

AllIsWell = SaveAA = /* SaveElev = */ SaveLat = SaveLon = SaveNormX = SaveNormY = SaveNormZ = SaveObj = SaveZ = FALSE;

// we know we'll need RGB bands so look for them specifically
CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;
Channels = 3;

// look for the other possible channels
if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
	SaveAA = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("ZBUF"))
	{
	FloatChannelNode[0] = Buffers->FindBufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_Z;
	SaveZ = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("LATITUDE"))
	{
	FloatChannelNode[2] = Buffers->FindBufferNode("LATITUDE", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_UV;
	SaveLat = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("LONGITUDE"))
	{
	FloatChannelNode[3] = Buffers->FindBufferNode("LONGITUDE", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_UV;
	SaveLon = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("OBJECT"))
	{
	ObjectID = (RasterAnimHost **)Buffers->FindBuffer("OBJECT", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_NODE_ID;
	SaveObj = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("SURFACE NORMAL X"))
	{
	FloatChannelNode[5] = Buffers->FindBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_NORMAL;
	SaveNormX = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("SURFACE NORMAL Y"))
	{
	FloatChannelNode[6] = Buffers->FindBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_NORMAL;
	SaveNormY = TRUE;
	Channels++;
	}
if (IOE && IOE->SaveEnabledBufferQuery("SURFACE NORMAL Z"))
	{
	FloatChannelNode[7] = Buffers->FindBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT);
	WCSgbuffers |= BMM_CHAN_NORMAL;
	SaveNormZ = TRUE;
	Channels++;
	}

if ((BufWidth < USHRT_MAX) && (BufHeight < USHRT_MAX))
	{
	AllIsWell = TRUE;
	}

if (AllIsWell && (!(TransferBuf = (UBYTE *)AppMem_Alloc((ULONG)BufWidth * sizeof(float), APPMEM_CLEAR))))
	{
	AllIsWell = FALSE;
	}

// need to change the sign on the z-buffer for MAX - (buffer also used for some other channels!!!)
if (AllIsWell && (!(zline = (float *)AppMem_Alloc((ULONG)BufWidth * sizeof(float), APPMEM_CLEAR))))
	{
	AllIsWell = FALSE;
	}

if (AllIsWell && (fHandle = PROJ_fopen(GetCompleteOutputPath(), "wb")))
	{
	long datapos, ytable;	// where the next scanline is writing to, where the seek position is stored
	USHORT runlen;
	UBYTE *uptr;
	long allbands;
	UBYTE *rlabuf;
	USHORT x,y;
	#ifdef BYTEORDER_LITTLEENDIAN
	long tmp;
	USHORT tmp16;
	#endif // BYTEORDER_LITTLEENDIAN

	// The overall size of the image.
	rlahdr.window.left = 0;
	rlahdr.window.right = (USHORT)(BufWidth - 1);
	rlahdr.window.bottom = 0;
	rlahdr.window.top = (USHORT)(BufHeight - 1);

	// The size of the active (non-zero) portion of the image.
	// <Assume> entire window is non black.
	rlahdr.active_window.left = 0;
	rlahdr.active_window.right = (USHORT)(BufWidth - 1);
	rlahdr.active_window.bottom = 0;
	rlahdr.active_window.top = (USHORT)(BufHeight - 1);

	// Animation frame number
	rlahdr.frame = (short)Frame + 1;

	// Number of image channels.
	rlahdr.num_chan = 3;		// Our friends R, G, and B

	// Number of matte (alpha) channels (>1 means multispectral mattes).
	if (SaveAA)
		rlahdr.num_matte = 1;
	else
		rlahdr.num_matte = 0;

	// Number of auxiliary data channels (G-Buffer types) ???
	rlahdr.num_aux = Channels - 3;

	// Version number.
	rlahdr.revision = RLA_MAGIC;

//	float g = OutputGamma();
	float g = (float)2.2;	// Set recommended default value
	sprintf(rlahdr.gamma, "%15f", g); 

	// Chromaticities of red, green and blue primaries and the white point.
	// Put in the suggested, NTSC, defaults.
	sprintf(rlahdr.red_pri,   "%7.4f %7.4f", 0.670, 0.080);	// [24]
	sprintf(rlahdr.green_pri, "%7.4f %7.4f", 0.210, 0.710);	// [24]
	sprintf(rlahdr.blue_pri,  "%7.4f %7.4f", 0.140, 0.330);	// [24]
	sprintf(rlahdr.white_pt,  "%7.4f %7.4f", 0.310, 0.316);	// [24]

	// User-specified job number (optional).
	rlahdr.job_num = 12345;

	// Filename used to open the output file (optional).
	strncpy(rlahdr.name, GetCompleteOutputPath(), 127);		// [128]

	// Description of file contents (optional).
	//strncpy(rlahdr.desc, UserData.desc, 127);				// [128]
	strncpy(rlahdr.desc, "Another great "APP_TLA" rendering", 127);

	// Program creating file (optional).
	/* But very desirable */
	MakeRLAProgString(rlahdr.program, WCSgbuffers, 0);

	// Machine on which file was created (optional).
	lpBuffer = &CompName[0];
	sighs = 127;
	nSize = &sighs;
	if (!GetComputerName(lpBuffer, nSize))
		strcpy(CompName, "My Computer");
	strncpy(rlahdr.machine, CompName, 31);					// [32]

	// User name of creator (optional).
	//strncpy(rlahdr.user, UserData.user, 31);
	lpBuffer = &UserName[0];
	sighs = 127;
	nSize = &sighs;
	strcpy(UserName, GlobalApp->MainProj->UserName);
	strncpy(rlahdr.user, UserName, 31);						// [32]

	// Date  of creation (optional).
	struct tm *newtime;
	time_t thetime;
	char datestr[26];
	char date[20];
	(void)time(&thetime);
	newtime = localtime(&thetime);
	sprintf(datestr, "%s", asctime(newtime));
	strncpy(date, &datestr[4], 7);			// copy MMM DD and space from datestr
	strncpy(&date[6], &datestr[19], 5);		// copy space and YY from datestr
	strncpy(&date[11], &datestr[10],6);		// copy space hh:mm from datestr
	date[19] = 0;							// terminate the string
	strncpy(rlahdr.date, date, 19);

	// Name of aspect ratio.
	/* We could output a LOT of standards here! */
	strncpy(rlahdr.aspect, "", 23);				// [24]

	// Aspect ratio of image (width / height)
	/* This isn't correct!!! */
	sprintf(rlahdr.aspect_ratio, "%.5f", (float)BufWidth / (float)BufHeight);	// [8]

	// Color space Can be one of rgb/xyz/sampled/raw.
	strncpy(rlahdr.chan, "rgb", 31);				// [32]

	// Flag to indicate image was rendered in fields.
	/* Implemented Jun 18, 2001, CHX */
	rlahdr.field = 0;
	// this data is no longer available since there is no more Renderer
	//if (RHost)
	//	{
	//	rlahdr.field = RHost->Cam->FieldRender;
	//	} // if

	// Rendering time (optional).
	/* Implemented Jun 18, 2001, CXH */
	strncpy(rlahdr.time, "N/A", 11);			// [12]
	// this data is no longer available since there is no more Renderer
	//if (RHost && RHost->StartSecs)
	//	{
	//	time_t NowSecs, Elapsed;
	//	unsigned char ElapHrs, ElapMin, ElapSec;

	//	GetTime(NowSecs);
	//	Elapsed = NowSecs - RHost->StartSecs;
	//	ElapSec = (unsigned char)(Elapsed % 60);
	//	ElapMin = (unsigned char)((Elapsed / 60) % 60);
	//	ElapHrs = (unsigned char)(Elapsed / 3600);

	//	sprintf(rlahdr.time, "%02d:%02d:%02d", ElapHrs, ElapMin, ElapSec);
	//	} // if

	// Filter used to post-process the image (optional).
	strncpy(rlahdr.filter, "", 31);				// [32]

	// Bit precision of data.
	/* Change to 16 bit channels when renderer rewritten {scale data} */
	rlahdr.chan_bits = rlahdr.matte_bits = 8;
	rlahdr.aux_bits = 8;	/* Presumably only relevant to original RLA since G-buffers have various depths */

	// Type of data (0=integer, 4=float).
	rlahdr.storage_type = 0;	// RGB
	rlahdr.matte_type = 0;		// Alpha
	rlahdr.aux_type = 0;		/* Presumably only relevant to original RLA since G-buffers have various types */
								/* NOTE: It looks like the G-buffers are always stored as byte channels,
								         regardless of the underlying data type */

	// Kind of auxiliary data. Can be either "range" or "depth".
	/* Can somebody tell me what the difference is? */
	strncpy(rlahdr.aux, "depth", 31);				// [32]

	// Unused - must be zeros.
	memset(rlahdr.space, 0, 36);			// [36]

	// Offset of next image in file.
	/* We may want to put a swatch/thumbnail after the image */
	rlahdr.next = 0;

	#ifdef BYTEORDER_LITTLEENDIAN
	SwapRLAHdrBytes(rlahdr);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((UBYTE *)&rlahdr, sizeof(RLAHeader), 1, fHandle);

	ytable = sizeof(RLAHeader);
	datapos = ytable + (ULONG)BufHeight * sizeof(long);

	if (ShortChannelNode[0])
		{
		if (ClampedRedBuf = (unsigned char *)AppMem_Alloc(BufWidth * 3, APPMEM_CLEAR))
			{
			ClampedGrnBuf = &ClampedRedBuf[BufWidth];
			ClampedBluBuf = &ClampedRedBuf[BufWidth + BufWidth];
			} // if
		} // if

	// Another Y-flipped file format apparently!
	for (y = 0; y < BufHeight; y++)
		{
		// for HDR clamping
		long int nCell;
		unsigned char TempRGB[3];

		// seek to the scanline index for this line
		fseek(fHandle, ytable + ((BufHeight - 1) - y) * 4, SEEK_SET);
		// write out seek position for data for this scanline
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32S(datapos, &tmp);
		fwrite((UBYTE *)&tmp, sizeof(long), 1, fHandle);
		#else
		fwrite(&datapos, sizeof(long), 1, fHandle);
		#endif
		fseek(fHandle, datapos, SEEK_SET);	// jump to ytable[y] to write scanline
		// write out all bands in order for each scanline
		rlabuf = &TransferBuf[0];
		// fetch all three RGB bands now for possible HDR clamping
		CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);
		CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);
		CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);

		// prep for HDR clamping
		if (ShortChannelNode[0])
			ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine((long)y, WCS_RASTER_BANDSET_SHORT);
		else
			ShortChannelData[0] = NULL;

		if (ShortChannelNode[0] && ShortChannelData[0])
			{
			for (nCell = 0; nCell < BufWidth; nCell++)
				{
				TempRGB[0] = CharChannelData[0][nCell];
				TempRGB[1] = CharChannelData[1][nCell];
				TempRGB[2] = CharChannelData[2][nCell];
				if (ShortChannelData[0] && ShortChannelData[0][nCell])
					rPixelFragment::ExtractClippedExponentialColors(TempRGB, ShortChannelData[0][nCell]);
				ClampedRedBuf[nCell] = TempRGB[0];
				ClampedGrnBuf[nCell] = TempRGB[1];
				ClampedBluBuf[nCell] = TempRGB[2];
				} // for
			} // if

		if (ShortChannelData[0])
			runlen = rla_encode(ClampedRedBuf, rlabuf, (int)BufWidth, (int)1);
		else
			runlen = rla_encode(CharChannelData[0], rlabuf, (int)BufWidth, (int)1);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(runlen, &tmp16);
		fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
		#else
		fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
		#endif
		fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
		allbands = 2 + runlen;	// length + run data
		rlabuf = &TransferBuf[0];
		// done above
		//CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);
		if (ShortChannelData[0])
			runlen = rla_encode(ClampedGrnBuf, rlabuf, (int)BufWidth, (int)1);
		else
			runlen = rla_encode(CharChannelData[1], rlabuf, (int)BufWidth, (int)1);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(runlen, &tmp16);
		fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
		#else
		fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
		#endif
		fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
		allbands += 2 + runlen;
		rlabuf = &TransferBuf[0];
		// done above
		//CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);
		if (ShortChannelData[0])
			runlen = rla_encode(ClampedBluBuf, rlabuf, (int)BufWidth, (int)1);
		else
			runlen = rla_encode(CharChannelData[2], rlabuf, (int)BufWidth, (int)1);
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(runlen, &tmp16);
		fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
		#else
		fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
		#endif
		fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
		allbands += 2 + runlen;

		// Matte (alpha) channel
		if (SaveAA)
			{
			rlabuf = &TransferBuf[0];
			CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine((long)y, WCS_RASTER_BANDSET_BYTE);
			runlen = rla_encode(CharChannelData[3], rlabuf, (int)BufWidth, (int)1);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			}

		// Z Buffer
		if (SaveZ)
			{
			rlabuf = &TransferBuf[0];
			FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			for (x = 0; x < BufWidth; x++)
				{
				zline[x] = -FloatChannelData[0][x];
				}
			uptr = (UBYTE *)zline;
			// encode each byte of float as separate bands, using stride
			runlen = rla_encode(uptr + 3, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)zline;
			runlen = rla_encode(uptr + 2, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)zline;
			runlen = rla_encode(uptr + 1, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)zline;
			runlen = rla_encode(uptr + 0, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			}

		// Max Video Post - Material Effects channel # (MAX uses 1..15 only, Kinetix Paint & Effect can handle 0..255)
		// Paint* says it can be 16 bit?!?

		// Object ID
		if (SaveObj) /* save our object number */
			{
			USHORT *objnum = (USHORT *)(&zline[0]);	// borrow this storage area
			ULONG  PixZip;
//			FloatChannelData[4] = (float *)FloatChannelNode[4]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
//			for (x = 0; x < BufWidth; x++)
//				objnum[x] = (USHORT)(((ULONG)(FloatChannelData[4][x])) % 65536);
			PixZip = y * BufWidth;
			for (x = 0; x < BufWidth; x++)
				objnum[x] = (USHORT)(ObjectID[PixZip++]); //lint !e507
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)objnum;	// cast away
			runlen = rla_encode(uptr + 1, rlabuf, (int)BufWidth, (int)2);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)objnum;
			runlen = rla_encode(uptr + 0, rlabuf, (int)BufWidth, (int)2);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			}

		// UV texture mapping coords
		if (SaveLat || SaveLon) /* we're putting the Lat / Long into these */
			{
			// store latitude in U
			if (SaveLat)
				FloatChannelData[2] = (float *)FloatChannelNode[2]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			else
				{
				FloatChannelData[2] = zline;	// borrow this storage area
				for (x = 0; x < BufWidth; x++)
					zline[x] = 0.0f;
				}
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[2];
			runlen = rla_encode(uptr + 3, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[2];
			runlen = rla_encode(uptr + 2, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[2];
			runlen = rla_encode(uptr + 1, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[2];
			runlen = rla_encode(uptr + 0, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;

			// now store longitude in V
			if (SaveLat)
				FloatChannelData[3] = (float *)FloatChannelNode[3]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			else
				{
				FloatChannelData[3] = zline;	// borrow this storage area
				for (x = 0; x < BufWidth; x++)
					zline[x] = 0.0f;
				}
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[3];
			runlen = rla_encode(uptr + 3, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[3];
			runlen = rla_encode(uptr + 2, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[3];
			runlen = rla_encode(uptr + 1, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)FloatChannelData[3];
			runlen = rla_encode(uptr + 0, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			}

		// Normals
		if (SaveNormX || SaveNormY || SaveNormZ)
			{
			// Note from Matthew Sorrels (Pontari Productions):
			// Max's Normals(for relighting) are compressed into a single ULONG from
			// the 3 normalized XYZ floats.  Basically it's the high 10 bits of the
			// normalized value jammed into the ULONG using (X<<20|Y<<10|Z).
			ULONG *norms = (ULONG *)(&zline[0]);	// borrow this storage area
			ULONG nx, ny, nz;

			nx = ny = nz = 0;
			if (SaveNormX)
				FloatChannelData[5] = (float *)FloatChannelNode[5]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			if (SaveNormY)
				FloatChannelData[6] = (float *)FloatChannelNode[6]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			if (SaveNormZ)
				FloatChannelData[7] = (float *)FloatChannelNode[7]->GetDataLine((long)y, WCS_RASTER_BANDSET_FLOAT);
			for (x = 0; x < BufWidth; x++)
				{
				if (SaveNormZ)	// normal value range = -1.0 .. 1.0
					nz = ((ULONG)(FloatChannelData[7][x] * 511) + 511) & 0x3FF;			// pack Z in the 10 least sig bits
				if (SaveNormY)
					ny = ((ULONG)((FloatChannelData[6][x] * 511) + 511) & 0x3FF) << 10;	// pack Y in the next 10 bits up
				if (SaveNormX)
					nx = ((ULONG)((FloatChannelData[5][x] * 511) + 511) & 0x3FF) << 20;	// pack X in the next 10 bits up
				norms[x] = nx | ny | nz;
				}
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)norms;
			runlen = rla_encode(uptr + 3, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)norms;
			runlen = rla_encode(uptr + 2, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)norms;
			runlen = rla_encode(uptr + 1, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			rlabuf = &TransferBuf[0];
			uptr = (UBYTE *)norms;
			runlen = rla_encode(uptr + 0, rlabuf, (int)BufWidth, (int)4);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(runlen, &tmp16);
			fwrite((UBYTE *)&tmp16, sizeof(USHORT), 1, fHandle);
			#else
			fwrite((UBYTE *)&runlen, sizeof(USHORT), 1, fHandle);
			#endif
			fwrite(TransferBuf, sizeof(UBYTE), runlen, fHandle);
			allbands += 2 + runlen;
			}

		// Unclamped RGB values???
		//if (WCSgbuffers & BMM_CHAN_REALPIX)

		//  Z-Buffer coverage (A-Buffer?)
		// if (WCSgbuffers & BMM_CHAN_COVERAGE)

		// Background channel -	The image that is behind a rendered object {ie: Z-Buffer cache?}
		// if (WCSgbuffers & BMM_CHAN_BG) /* used for transparency? */

		datapos += allbands;
		} // for y

	if (ClampedRedBuf)
		{
		AppMem_Free(ClampedRedBuf, BufWidth * 3);
		ClampedRedBuf = NULL;
		} // if


	if (ferror(fHandle))
		AllIsWell = FALSE;

	} // if (AllIsWell && fHandle)

if (fHandle)
	{
	fclose(fHandle);
	fHandle = NULL;
	} // if

if (zline)
	{
	AppMem_Free(zline, BufWidth * sizeof(float));
	} // if

if (TransferBuf)
	{
	AppMem_Free(TransferBuf, BufWidth * sizeof(float));
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO
} // ImageFormatRLA::StartFrame

/*===========================================================================*/

// A taste of the new generation...

#ifdef WCS_BUILD_AVI_SUPPORT
int ImageFormatAVI::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
long xout, xscan, scanrow, outrow;
unsigned char AllIsWell = 0, SampRGB[3];
BYTE *bits;

bits = (BYTE *)bmi->bmiColors; 

// we know we'll need RGB bands so look for them specifically
CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	for (outrow = 0, scanrow = BufHeight - 1; scanrow >= 0; scanrow--, outrow++)
		{
		CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
		CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
		CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
		if (ShortChannelNode[0])
			ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);

		TransferBuf = bits + (outrow * rowbytes);

		if (CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
			{
			// Interleave data into AVI's BGR format
			for (xout = xscan = 0; xscan < BufWidth; xscan++)
				{
				SampRGB[0] = CharChannelData[0][xscan];
				SampRGB[1] = CharChannelData[1][xscan];
				SampRGB[2] = CharChannelData[2][xscan];
				if (ShortChannelData[0] && ShortChannelData[0][xscan])
					rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
				TransferBuf[xout++] = SampRGB[2];
				TransferBuf[xout++] = SampRGB[1];
				TransferBuf[xout++] = SampRGB[0];
				} // for
			} // if
		else
			{
			break;
			} // else
		} // for
	if (scanrow < 0)
		{
		// write
		// Set format if on first frame
		if (!FramesWritten)
			{
			//AVIStreamSetFormat(pcomp, 0, bmi, bmi->bmiHeader.biSize);
			} // if
		// write image
		AVIStreamWrite(pcomp, FramesWritten, 1, bits, bmi->bmiHeader.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
		FramesWritten++;
		AllIsWell = 1;
		} // if
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);
#endif // !DEMO

} // ImageFormatAVI::StartFrame

/*===========================================================================*/

static char AnimOutputPath[1024];

/*
======================================================================
AVIalloc_bmi()

Allocate memory for a Windows device-independent bitmap.

Makes space for a 24-bit image in CF_DIB format.  If rowbytes isn't
NULL, it's set to the number of bytes per scanline (DIB scanlines are
longword-aligned).  The BITMAPINFOHEADER is initialized for a 24-bit
DIB, and the memory is returned as a pointer to BITMAPINFO.
====================================================================== */

static BITMAPINFO *AVIalloc_bmi(int width, int height, int *rowbytes, int *bmisize)
{
BITMAPINFO *bmi;
int rb;

rb = (((unsigned)width * 3 + 3 ) >> 2 ) << 2;
if (rowbytes)
	*rowbytes = rb;
bmi = (BITMAPINFO *)AppMem_Alloc(*bmisize = sizeof(BITMAPINFOHEADER) + rb * height, NULL);
if (!bmi)
	return NULL;

bmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
bmi->bmiHeader.biWidth         = width;
bmi->bmiHeader.biHeight        = height;
bmi->bmiHeader.biPlanes        = 1;
bmi->bmiHeader.biBitCount      = 24;
bmi->bmiHeader.biCompression   = BI_RGB;
bmi->bmiHeader.biSizeImage     = rb * height;
bmi->bmiHeader.biXPelsPerMeter = 3780;
bmi->bmiHeader.biYPelsPerMeter = 3780;
bmi->bmiHeader.biClrUsed       = 0;
bmi->bmiHeader.biClrImportant  = 0;

return bmi;

} // AVIalloc_bmi

/*===========================================================================*/

int ImageFormatAVI::StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO

if (!IOE) return(0);
AVICOMPRESSOPTIONS *ACO;

if (!(bmi = AVIalloc_bmi(BufWidth, BufHeight, &rowbytes, &BMISize)))
	{
	return(0);
	} // if


IOE->PAF.GetFramePathAndName(AnimOutputPath, IOE->AutoExtension ? ImageSaverLibrary::GetDefaultExtension(IOE->FileType) : NULL, 0, 1000, 0);

// remove file if it already exists, because Windows won't.
if (fHandle = PROJ_fopen(AnimOutputPath, "rb"))
	{
	fclose(fHandle);
	PROJ_remove(AnimOutputPath);
	} // if

pfile = NULL;
AVIFileInit();          // opens AVIFile library
LibRef++;

ICCompressorChoose((HWND)GlobalApp->WinSys->GetRoot(), ICMF_CHOOSE_ALLCOMPRESSORS, NULL, NULL, &cv, APP_SHORTTITLE " AVI Options" );

if (!AVIFileOpen(&pfile, GlobalApp->MainProj->MungPath(AnimOutputPath), OF_CREATE | OF_WRITE, NULL))
	{
	memset(&avisi,0,sizeof(AVISTREAMINFO));
	
	avisi.fccType    = streamtypeVIDEO;
	avisi.fccHandler = cv.fccHandler;
	avisi.dwScale    = 1;
	avisi.dwRate     = (DWORD)FrameRate;
	avisi.dwSuggestedBufferSize = bmi->bmiHeader.biSizeImage;
	//avisi.dwQuality  = 7500;
	//avisi.dwLength   = (DWORD)(AnimTime * FrameRate);

	SetRect(&avisi.rcFrame, 0, 0, BufWidth, BufHeight);

	if (AVIFileCreateStream(pfile, &pavi, &avisi) == AVIERR_OK)
		{
		memset(&compOptions, 0, sizeof(AVICOMPRESSOPTIONS));

		ACO = &compOptions;
		AVISaveOptions((HWND)GlobalApp->WinSys->GetRoot(), ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE,
		 1, &pavi, /*(AVICOMPRESSOPTIONS **)*/ &ACO);
		
		//compOptions.dwFlags         = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;
		//compOptions.fccType         = streamtypeVIDEO;
		//compOptions.fccHandler      = cv.fccHandler;
		//compOptions.dwQuality       = cv.lQ;
		//compOptions.dwKeyFrameEvery = lKeys;

		if (AVIMakeCompressedStream(&pcomp, pavi, &compOptions, NULL) == AVIERR_OK)
			{
			// Use this cleanup code:
			// AVIStreamRelease(pcomp);
			AVIStreamSetFormat(pcomp, 0, bmi, bmi->bmiHeader.biSize);
			return(1);
			} // if
		AVIStreamRelease(pavi);
		} // if
	AVIFileRelease(pfile);
	} // if

if (LibRef > 0)
	{
	AVIFileExit();          // releases AVIFile library 
	LibRef--;
	} // if

return(0);

#endif // !DEMO
} // ImageFormatAVI::StartAnim

/*===========================================================================*/

int ImageFormatAVI::EndAnim(void)
{
if (bmi)
	{
	AppMem_Free(bmi, BMISize);
	bmi = NULL;
	BMISize = 0;
	} // if

if (pavi)
	{
	AVIStreamRelease(pavi);
	} // if
if (pcomp)
	{
	AVIStreamRelease(pcomp);
	} // if
if (pfile)
	{
	AVIFileRelease(pfile);
	} // if
if (LibRef > 0)
	{
	AVIFileExit();          // releases AVIFile library 
	LibRef--;
	} // if
return(0);

} // ImageFormatAVI::EndAnim
#endif // WCS_BUILD_AVI_SUPPORT

/*===========================================================================*/

int ImageFormatAI::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{

return(0);

} // ImageFormatAI::StartFrame

/*===========================================================================*/

int ImageFormatAI::StartVectorFrame(long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
IllustratorInit(WCS_ILLUSTRATOR_DEFAULT_DPI, WCS_ILLUSTRATOR_APPROX_RASTER_DPI, BufWidth, BufHeight, Frame);
return(0);
#endif // !DEMO

} // ImageFormatAI::StartVectorFrame

/*===========================================================================*/

int ImageFormatAI::EndVectorFrame(void)
{

IllustratorEnd();
return(0);

} // ImageFormatAI::EndVectorFrame

/*===========================================================================*/

#ifdef WCS_BUILD_MRSID_WRITE__SUPPORT
int ImageFormatMRSID::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
return(0);
#endif // !DEMO

} // ImageFormatMRSID::StartFrame
#endif // WCS_BUILD_MRSID_WRITE__SUPPORT
