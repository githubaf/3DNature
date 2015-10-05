// ImageFormatIFF.cpp
// IFF support code
// Built from Bitmaps.cpp on 12/06/00 by CXH

#include "stdafx.h"
#include "ImageFormatIFF.h"
#include "Useful.h"
#include "Raster.h"
#include "Project.h"
#include "Application.h"
#include "AppMem.h"
#include "Log.h"
#include "zlib.h"

extern WCSApp *GlobalApp;

short CheckIFF(FILE *fh, struct ILBMHeader *Hdr)
{

if ((fread((char *)Hdr, 1, sizeof (struct ILBMHeader), fh)) == sizeof (struct ILBMHeader))
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(Hdr->ChunkSize, &Hdr->ChunkSize);
	#endif // BYTEORDER_LITTLEENDIAN
	if (! strncmp((char *)&Hdr->ChunkID[0], "FORM", 4))
		{
		if ((fread((char *)&Hdr->ChunkID[0], 1, 4, fh)) == 4)
			{
			if (! strncmp((char *)&Hdr->ChunkID[0], "ILBM", 4))
				{
				return (1);
				} // if ILBM found
			} // if read OK
		} // if FORM found
	} // if header read

return (0);

} // CheckIFF

/*===========================================================================*/

short FindIFFChunk(FILE *fh, struct ILBMHeader *Hdr, char *Chunk)
{
long readsize;
short error = 0;

// fread()ing a whole structure could be very bad!
while ((readsize = (long)fread((char *)Hdr, 1, sizeof (struct ILBMHeader), fh)) == sizeof (struct ILBMHeader))
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(Hdr->ChunkSize, &Hdr->ChunkSize);
	#endif // BYTEORDER_LITTLEENDIAN
	if (! strncmp((char *)&Hdr->ChunkID[0], Chunk, 4))
		break;
	if ((fseek(fh, Hdr->ChunkSize, SEEK_CUR)) != 0)
		{
		error = 1;
		break;
		} // if file size error
	} // until bit map header found
if (readsize != sizeof (struct ILBMHeader)) error = 1;

if (error) return (0);
return (1);

} // FindIFFChunk

/*===========================================================================*/

short CompressRows(struct CompressData *CD)
{
long lastbyte, startbyte, byte, writebytes, RowBase;
short i, mode, newmode, forcebreak, error = 0;

lastbyte = CD->RowBytes - 1;
RowBase = 0;

for (i=0; i<CD->Rows; i++)
  {
  forcebreak = 0;
  startbyte = byte = 0;
  mode = CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1];
  writebytes = 1;
  byte ++;

  while (byte<lastbyte)
   {
   while ((newmode = (CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1])) == mode) //lint !e731
    {
    byte ++;
    writebytes ++;
    if (writebytes == CD->MaxByteRun)
     {
	 if (byte != lastbyte)
	  {
	  newmode = 1;
	  } // if
	 /* this is what it used to be - large files sometines compressed wrong
     if (mode)
      {
      newmode = CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1];
      if (! newmode && byte != lastbyte) writebytes --;
      } // if current mode = 1 
     else
      {
      newmode = CD->Data[RowBase + byte + 1] != CD->Data[RowBase + byte + 2];
      } // else current mode = 0 
	  */
     forcebreak = 1;
     break;
     } /* if time to write some data */
    if (byte == lastbyte)
     {
     forcebreak = 1;
     break;
     } /* if time to write some data */
    } /* while no change in mode */
   if (mode)
    {
    if (! forcebreak)
     {
     if (byte + 2 > lastbyte)
      {
      byte ++;
      writebytes ++;
      forcebreak = 1;
      }
     else if (CD->Data[RowBase + byte + 1] != CD->Data[RowBase + byte + 2])
      {
      byte ++;
      writebytes ++;
      if (writebytes == CD->MaxByteRun)
       {
       newmode = CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1];
       if (! newmode && byte != lastbyte) writebytes --;
       forcebreak = 1;
       } /* if time to write some data */
      else continue;
      } /* if only two consecutive values equal continue 'for' loop */
     } /* if not end of row or write buffer full */
    if (forcebreak)
     {
     writebytes ++;
     } /* if forced break */
    if (writebytes)
     {
	 // limit to CD->OutSize added 2/13/03 GRH, bounds checking added same date
	 if (CD->OutCtr + writebytes + 1 > CD->OutSize)
		{
		error = 1;
		break;
		}
     CD->OutArray[CD->OutCtr].Bt = (char)(writebytes - 1);
     memcpy(&CD->OutArray[CD->OutCtr + 1].UBt, &CD->Data[RowBase + startbyte], writebytes); //lint !e669
     CD->OutCtr += (writebytes + 1);
     } /* if some data to write */
    } /* if mode == 1 values unequal */
   else
    {
    CD->OutArray[CD->OutCtr].Bt = (char)-writebytes;
    CD->OutArray[CD->OutCtr + 1].UBt = CD->Data[RowBase + startbyte];
    CD->OutCtr += 2;
    } /* else mode == 0 values equal */

   if (newmode)
    {
    byte ++;
    startbyte = byte;
    }
   else
    {
    startbyte = byte;
    byte ++;
    if (! mode) startbyte ++;
    }

   if (byte == lastbyte)
    {
    CD->OutArray[CD->OutCtr].Bt = 0;
    CD->OutArray[CD->OutCtr + 1].UBt = CD->Data[RowBase + lastbyte];
    CD->OutCtr += 2;
    break;
    } /* if last byte in row */

   if (CD->OutCtr > CD->OutSize - CD->MaxByteRun - 2)
    {
    if ((error = FlushOutputBuff(CD)) > 0) break;
    CD->TotalOutBytes += CD->OutCtr;
    CD->OutCtr = 0;
    } // if buffer almost full

   forcebreak = 0;
   if (! mode && ! newmode) writebytes = 0;
   else
    {
    mode = newmode;
    writebytes = 1 - mode;
    } // else
   } // for

  if (error) break;
  if (CD->OutCtr > CD->OutSize - CD->MaxByteRun - 2)
   {
   if ((error = FlushOutputBuff(CD)) > 0) break;
   CD->TotalOutBytes += CD->OutCtr;
   CD->OutCtr = 0;
   } // if buffer almost full

  RowBase += CD->RowBytes;
  } // for i

 return (error);

} // CompressRows

/*===========================================================================*/

short FlushOutputBuff(struct CompressData *CD)
{
short rVal = 0;

if (fwrite (CD->OutArray, 1, CD->OutCtr, CD->fHandle) != (unsigned)CD->OutCtr)
	rVal = 1;

return (rVal);

} // FlushOutputBuff

/*===========================================================================*/

short Raster::LoadIFFILBM(char *Name, short SupressWng, ImageCacheControl *ICC)
{
//short copyred = 0;
short namelen, error = 0, pp, rr, color, pixel;
long byte, x, RowDataSize, InputDataSize, BytesRead, RowSize;
union MultiByte *InputData = NULL, *InputDataPtr;
unsigned char *RowData = NULL, *RowDataPtr, *BuffPtr, *BitmapPtr;
struct ILBMHeader Hdr;
struct BitMapHeader BMHdr;
FILE *fh;
unsigned char power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};

if(ICC) ICC->QueryOnlyFlags = NULL; // clear all flags

RepeatOpen:
if ((fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/)) == NULL)
	{
	namelen = (short)strlen(Name);
	if (namelen > 1)
		{
		if (Name[namelen - 1] == ' ')
			{
			Name[namelen - 1] = 0;
			goto RepeatOpen;
			} // if trailing blank
		} // if
	error = 1; // open fail
	goto Cleanup;
	} // if

if (CheckIFF(fh, &Hdr))
	{ 
	if (FindIFFChunk(fh, &Hdr, "BMHD"))
		{
		if ((fread((char *)&BMHdr, 1, sizeof (struct BitMapHeader), fh)) != sizeof (struct BitMapHeader))
			{
			fclose(fh);
			error = 2; // read fail
			goto Cleanup;
			} // read error

		// Endian adjust
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(BMHdr.bmh_Width, &BMHdr.bmh_Width);
		SimpleEndianFlip16U(BMHdr.bmh_Height, &BMHdr.bmh_Height);
		SimpleEndianFlip16S(BMHdr.bmh_Left, &BMHdr.bmh_Left);
		SimpleEndianFlip16S(BMHdr.bmh_Top, &BMHdr.bmh_Top);
		SimpleEndianFlip16U(BMHdr.bmh_Transparent, &BMHdr.bmh_Transparent);
		SimpleEndianFlip16S(BMHdr.bmh_PageWidth, &BMHdr.bmh_PageWidth);
		SimpleEndianFlip16S(BMHdr.bmh_PageHeight, &BMHdr.bmh_PageHeight);
		#endif // BYTEORDER_LITTLEENDIAN

		if (BMHdr.bmh_Depth != 8 && BMHdr.bmh_Depth != 24)
			{
			fclose(fh);
			error = 4; // wrong depth
			goto Cleanup;
			} // wrong number of bit planes

		// Allocate bitmaps
		Cols = BMHdr.bmh_Width;
		Rows = BMHdr.bmh_Height;
		if (ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "IFF Loader Bitmaps"))
			{
			ByteBands = 1;
			if (BMHdr.bmh_Depth == 24)
				{
				ByteBands = 3;
				if (ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "IFF Loader Bitmaps"))
					{
					if ((ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "IFF Loader Bitmaps")) == NULL)
					error = 5; // memory
					} // if
				else
					{
					error = 5; // memory
					} // else
				} // if planes = 24
			} // if
		else
			{
			error = 5; // memory
			} // else
		if (error)
			{
			fclose(fh);
			goto Cleanup;
			} // if

		if (FindIFFChunk(fh, &Hdr, "BODY"))
			{
			InputDataSize = Hdr.ChunkSize;
			if (InputDataSize <= 0 || ((InputData = (union MultiByte *)AppMem_Alloc(InputDataSize, NULL, "IFF Loader")) == NULL))
				{
				fclose(fh);
				error = 5; // silent out of memory
				goto Cleanup;
				} // if out of memory
			RowSize = 2 * ((Cols + 15) / 16);
			RowDataSize = RowSize * BMHdr.bmh_Depth;
			if ((RowData = (unsigned char *)AppMem_Alloc(RowDataSize, APPMEM_CLEAR, "IFF Loader")) == NULL)
				{
				fclose(fh);
				error = 5; // silent out of memory
				goto Cleanup;
				} // if out of memory

			if ((fread((char *)InputData, 1, InputDataSize, fh)) != (unsigned)InputDataSize)
				{
				fclose(fh);
				error = 2; // READ FAIL
				goto Cleanup;
				} // if read error
			fclose(fh);

			// clear bitmaps
			for (color=0; color<BMHdr.bmh_Depth/8; color++)
				{
				memset(ByteMap[color], 0, Cols * Rows);
				} // for

			// read the data
			InputDataPtr = InputData;
			for (rr=0; rr<Rows; rr++)
				{
				RowDataPtr = RowData;
				if (BMHdr.bmh_Compression)
					{
					for (color=0; color<ByteBands; color++)
						{
						for (pp=0; pp<8; pp++)
							{
							BytesRead = 0;
							while (BytesRead < RowSize)
								{
								if (InputDataPtr[0].Bt >= 0)
									{
									memcpy(RowDataPtr, &InputDataPtr[1].UBt, 1 + InputDataPtr[0].Bt);
									BytesRead += (1 + InputDataPtr[0].Bt);
									RowDataPtr += (1 + InputDataPtr[0].Bt);
									InputDataPtr += (2 + InputDataPtr[0].Bt);
									} // if literal run
								else
									{
									memset(RowDataPtr, InputDataPtr[1].UBt, 1 - InputDataPtr[0].Bt);
									BytesRead += (1 - InputDataPtr[0].Bt);
									RowDataPtr += (1 - InputDataPtr[0].Bt);
									InputDataPtr += 2;
									} // else replicate run
								} // while
							if (BytesRead > RowSize)
								{
								error = 2; // READ FAIL
								goto Cleanup;
								} // if compression error
							} // for pp=0...
						} // for color=0...
					} // if compressed data
				else
					{
					memcpy(RowDataPtr, InputDataPtr, RowDataSize);
					InputDataPtr += RowDataSize;
					} // else not compressed
				// do corner turn for all planes, one row
				BuffPtr = RowData;
				for (color=0; color<ByteBands; color++)
					{
					BitmapPtr = ByteMap[color] + rr * Cols;
					for (pp=0; pp<8; pp++)
						{
						for (byte=0,x=0; byte<RowSize; byte++, BuffPtr++)
							{
							for (pixel=7; pixel>=0 && x<Cols; pixel--, x++)
								{
								if (*BuffPtr & power2[pixel])
									BitmapPtr[x] += (1 << pp);
								} // for pixel=7...
							} // for byte=0...
						} // for pp=0...
					} // for color=0...
				} // for rr=0...
			} // if BODY found
		else
			{
			fclose (fh);
			error = 2; // READ FAIL
			goto Cleanup;
			} // else no BODY
		} // if BMHD found
	else
		{
		fclose (fh);
		error = 2;  // READ FAIL
		goto Cleanup;
		} // else no BMHD
	} // if IFF file

Cleanup:
if (InputData) AppMem_Free(InputData, InputDataSize);
if (RowData) AppMem_Free(RowData, RowDataSize);

switch (error)
	{
	case 1:
		{
		if (! SupressWng) GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, Name);
		break;
		} // OPEN FAIL
	case 2:
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Name);
		break;
		} // READ FAIL
	case 4:
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Only 8-bit and 24-bit IFF supported.");
		break;
		} // Invalid Depth
	case 5:
		{ // logged in memory module
		break;
		} // memory fail
	} // switch error

if (error)
	{
	return (0);
	} // if

ByteBandSize = Rows * Cols;
AlphaAvailable = 0;
AlphaEnabled = 0;

return (1);

} // Raster::LoadIFFILBM

/*===========================================================================*/

#ifdef OLD_ZB_LOADER // instead of commenting out

short LoadZBuf(char *Name, float *ZBuf, struct ZBufferHeader *ZBHdr,
	short Width, short Height)
{
 short success = 0;
 long ReadSize;
 FILE *fh;
 struct ILBMHeader Hdr;

 ReadSize = Width * Height * sizeof (float);

 if ((fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/)) != NULL)
  {
  if (CheckIFF(fh, &Hdr))
   {
   if (FindIFFChunk(fh, &Hdr, "ZBUF"))
    {
    if (fread((char *)ZBHdr, 1, sizeof (struct ZBufferHeader), fh) == sizeof (struct ZBufferHeader))
     {
#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(ZBHdr->Width, &ZBHdr->Width);
	SimpleEndianFlip32U(ZBHdr->Height, &ZBHdr->Height);

	SimpleEndianFlip16U(ZBHdr->VarType, &ZBHdr->VarType);
	SimpleEndianFlip16U(ZBHdr->Compression, &ZBHdr->Compression);
	SimpleEndianFlip16U(ZBHdr->Sorting, &ZBHdr->Sorting);
	SimpleEndianFlip16U(ZBHdr->Units, &ZBHdr->Units);

	SimpleEndianFlip32F(&ZBHdr->Min, &ZBHdr->Min);
	SimpleEndianFlip32F(&ZBHdr->Max, &ZBHdr->Max);
	SimpleEndianFlip32F(&ZBHdr->Bkgrnd, &ZBHdr->Bkgrnd);
	SimpleEndianFlip32F(&ZBHdr->ScaleFactor, &ZBHdr->ScaleFactor);
	SimpleEndianFlip32F(&ZBHdr->ScaleBase, &ZBHdr->ScaleBase);

#endif // BYTEORDER_LITTLEENDIAN
     if (FindIFFChunk(fh, &Hdr, "ZBOD"))
      {
      if (ZBHdr->VarType == 6)
       {
       if ((fread((char *)ZBuf, 1, ReadSize, fh)) == (unsigned)ReadSize)
        {
#ifdef BYTEORDER_LITTLEENDIAN
		{
		int FlipLoop;
		for(FlipLoop = 0; FlipLoop < (Width * Height); FlipLoop++)
			{
			SimpleEndianFlip32F(&ZBuf[FlipLoop], &ZBuf[FlipLoop]);
			} // for
		} // scope
#endif // BYTEORDER_LITTLEENDIAN
        success = 1;
	} /* if data read OK */
       } /* if single precision floating point */
      } /* if ZBOD chunk found */
     } /* if header read OK */
    } /* if ZBUF chunk found */
   } /* if IFF file */
  else
   {
   fseek(fh, 0L, SEEK_END);
   if (ftell(fh) == ReadSize)
    {
    fseek(fh, 0L, SEEK_SET);
    if ((fread((char *)ZBuf, 1, ReadSize, fh)) == (unsigned)ReadSize)
     {
#ifdef BYTEORDER_LITTLEENDIAN
	{
	int FlipLoop;
	for(FlipLoop = 0; FlipLoop < (Width * Height); FlipLoop++)
		{
		SimpleEndianFlip32F(&ZBuf[FlipLoop], &ZBuf[FlipLoop]);
		} // for
	} // scope
#endif // BYTEORDER_LITTLEENDIAN
     success = 1;
     }
    } /* if file size OK for single precision floating point */
   } /* else not IFF file */
  fclose (fh);
  } /* if file opened OK */
 else
  {
  GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, Name);
  } /* else open fail */

 return (success);

} /* LoadZBuf() */

// instead of commenting out
#endif // OLD_ZB_LOADER


/*===========================================================================*/


short Raster::LoadIFFZBUF(char *Name, short SupressWng)
{
unsigned char power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};
//short copyred = 0;
short namelen, error = 0;
struct ILBMHeader Hdr;
struct ZBufferHeader ZBHdr;
long ReadSize;
FILE *fh;

RepeatOpen:
if ((fh = PROJ_fopen(Name, "rb"/*IOFLAG_BINARY | IOFLAG_RDONLY*/)) == NULL)
	{
	namelen = (short)strlen(Name);
	if (namelen > 1)
		{
		if (Name[namelen - 1] == ' ')
			{
			Name[namelen - 1] = 0;
			goto RepeatOpen;
			} // if trailing blank
		} // if
	error = 1; // open fail
	} // if

if (fh && CheckIFF(fh, &Hdr))
	{
	if (FindIFFChunk(fh, &Hdr, "ZBUF"))
		{
		if (fread((char *)&ZBHdr, 1, sizeof (struct ZBufferHeader), fh) == sizeof (struct ZBufferHeader))
			{
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


			// Allocate bitmaps
			Cols = ZBHdr.Width;
			Rows = ZBHdr.Height;

			ReadSize = Cols * Rows * sizeof (float);

			if (FloatMap[0] = (float *)AppMem_Alloc(ReadSize, 0, "ZBUF Loader Bitmaps"))
				{
				FloatBands = 1;

				if (FindIFFChunk(fh, &Hdr, "ZBOD"))
					{
					if (ZBHdr.VarType == 6)
						{
						char ReadOk = 0;
						if(ZBHdr.Compression == 1)
							{
							unsigned char *DecompressBuf;
							unsigned long CompressedReadSize;

							// load data in ZBOD chunk and decompress with zlib
							CompressedReadSize = Hdr.ChunkSize;
							if(DecompressBuf = (unsigned char *)AppMem_Alloc(CompressedReadSize, 0, "ZBUF Loader Decompression"))
								{
								if((fread((char *)DecompressBuf, 1, CompressedReadSize, fh)) == (unsigned)CompressedReadSize)
									{
									unsigned long int OutputSize;
									OutputSize = ReadSize;
									if(uncompress((unsigned char *)FloatMap[0], &OutputSize, DecompressBuf, CompressedReadSize) == Z_OK)
										{
										if(OutputSize == (unsigned)ReadSize)
											{
											ReadOk = 1;
											} // if
										} // if
									} // if
								AppMem_Free(DecompressBuf);
								DecompressBuf = NULL;
								} // if
							} // if
						else
							{
							if((fread((char *)FloatMap[0], 1, ReadSize, fh)) == (unsigned)ReadSize)
								{
								ReadOk = 1;
								} // if
							} // else
						if (ReadOk)
							{
							#ifdef BYTEORDER_LITTLEENDIAN
								{
								int FlipLoop;
								for(FlipLoop = 0; FlipLoop < (Cols * Rows); FlipLoop++)
									{
									SimpleEndianFlip32F(&FloatMap[0][FlipLoop], &FloatMap[0][FlipLoop]);
									} // for
								} // scope
							#endif // BYTEORDER_LITTLEENDIAN
							} /* if data read OK */
						} /* if single precision floating point */
					} /* if ZBOD chunk found */
				else
					{
					error = 2; // READ FAIL
					} // else no BODY
				} // if
			else
				{
				error = 5; // memory
				} // else
			} // if
		else
			{
			error = 2;  // READ FAIL
			} // else
		} // if ZBUF found
	else
		{
		error = 2;  // READ FAIL
		} // else no ZBUF
	} // if IFF file

if(fh)
	{
	fclose (fh);
	fh = NULL;
	} // if

switch (error)
	{
	case 1:
		{
		if (! SupressWng) GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, Name);
		break;
		} // OPEN FAIL
	case 2:
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Name);
		break;
		} // READ FAIL
	case 5:
		{ // logged in memory module
		break;
		} // memory fail
	default:
		break;
	} // switch error

if (error)
	{
	return (0);
	} // if

ByteBandSize = Rows * Cols;
FloatBandSize = Rows * Cols;
AlphaAvailable = 0;
AlphaEnabled = 0;

return (1);

} // Raster::LoadIFFZBUF
