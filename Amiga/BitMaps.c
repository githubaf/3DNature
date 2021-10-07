/* BitMaps.c (ne gisbitmaps.h 14 Jan 1994 CXH)
** The functions relating to 24-bit maps, creating, saving and closing.
** Original code by Gary R. Huber
*/

#include "WCS.h"

union  MultiByte {
 UBYTE UBt;
 BYTE  Bt;
};

struct CompressData {
 UBYTE  *Data;
 union MultiByte *OutArray;
 long	fHandle;
 long	OutSize,
 	Rows,
	RowBytes,
	OutCtr,
	MaxByteRun,
	TotalOutBytes,
	fh;
};
/*
struct ILBMHeader {
 UBYTE ChunkID[4];
 LONG ChunkSize;
};

struct WcsBitMapHeader {
 USHORT Width, Height;
 SHORT XPos, YPos;
 UBYTE Planes, Masking, Compression, Pad;
 USHORT Transparent;
 UBYTE XAspect, YAspect;
 SHORT PageWidth, PageHeight;
};

struct ZBufferHeader {
 ULONG  Width, Height;
 USHORT VarType, Compression, Sorting, Units;
 float  Min, Max, Bkgrnd, ScaleFactor, ScaleBase;
};
*/
STATIC_FCN short CompressRows(struct CompressData *CD); // used locally only -> static, AF 20.7.2021
STATIC_FCN short FlushOutputBuff(struct CompressData *CD); // used locally only -> static, AF 20.7.2021
STATIC_FCN short LoadZBuf(char *Name, float *ZBuf, struct ZBufferHeader *ZBHdr,
        short Width, short Height); // used locally only -> static, AF 20.7.2021


short openbitmaps(UBYTE **bitmap, long zsize)
{
 short error = 0;
 long i;

 for (i=0; i<3; i++)
  {
  if (!(bitmap[i] = (UBYTE *)get_Memory(zsize, MEMF_CLEAR)))
   {
   error = 1;
   break;
   } /* if out of memory */
  } /* for i=0... */

 return error;

} /* openbitmaps() */

/*********************************************************************/
/* for saving Raw RGB (Sculpt and Interleaved) files */
short savebitmaps(UBYTE **bitmap, long zsize, short renderseg)
{
 char filename[256];
 short error = 0;
 long flag, i, fhred = -2, fhgreen = -2, fhblue = -2,
	 protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE;
 FILE *fred;

 strcpy(filename, ILBMname);

 if (settings.saveIFF == 1)
  {
  UBYTE *ByteRow;
  LONG ByteRowSize;

  strcat(filename, ".RAW");
  if (renderseg == 0 || settings.composite == 0)
   {
   if ((fred = fopen(filename, "wb")) == NULL)
    {
    error = 1;
    goto Cleanup;
    } /* if open fail */
   } /* if no composite or first segment */
  else
   {
   if ((fred = fopen(filename, "ab")) == NULL)
    {
    error = 1;
    goto Cleanup;
    } /* if open fail */
   } /* else composite */

  ByteRowSize = settings.scrnwidth * 3;
  if ((ByteRow = (UBYTE *)get_Memory(ByteRowSize, MEMF_ANY)) != NULL)
   {
   short j = 0;
   UBYTE *RedPtr, *GrnPtr, *BluPtr;

   RedPtr = bitmap[0];
   GrnPtr = bitmap[1];
   BluPtr = bitmap[2];
   for ( ; j<settings.scrnheight; j++)
    {
    for (i=0; i<settings.scrnwidth; i+=3)
     {
     ByteRow[i]     = *RedPtr;
     ByteRow[i + 1] = *GrnPtr;
     ByteRow[i + 2] = *BluPtr;
     RedPtr ++;
     GrnPtr ++;
     BluPtr ++;
     } /* for i=0... */
    if ((fwrite((char *)ByteRow, ByteRowSize, 1, fred)) != 1)
     {
     error = 1;
     break;
     } /* if write fail */
    } /* for j=0... */
   free_Memory(ByteRow, ByteRowSize);
   } /* if row memory */
  else
   {
   for (i=0; i<zsize; i++)
    {
    if (fwrite((char *)(bitmap[0] + i), 1, 1, fred) != 1)
     {
     error = 1;
     break;
     } /* if write fail */
    if (fwrite((char *)(bitmap[1] + i), 1, 1, fred) != 1)
     {
     error = 1;
     break;
     } /* if write fail */
    if (fwrite((char *)(bitmap[2] + i), 1, 1, fred) != 1)
     {
     error = 1;
     break;
     } /* if write fail */
    } /* for i=0... */
   } /* else no row memory */
  } /* if settings.saveIFF == 1, Interleaved Raw */

 else
  {
  if (renderseg == 0 || settings.composite == 0) flag = O_WRONLY | O_CREAT | O_TRUNC ;
  else flag = O_WRONLY | O_CREAT | O_APPEND ;
  strcat(filename, ".red");
  if ((fhred = open(filename, flag, protflags)) == -1)
   {
   error = 1;
   goto Cleanup;
   } /* if no red file */
  strcpy(filename, ILBMname);
  strcat(filename, ".grn");
  if ((fhgreen = open(filename, flag, protflags)) == -1)
   {
   error=1;
   goto Cleanup;
   } /* if no green file */
  strcpy(filename, ILBMname);
  strcat(filename, ".blu");
  if ((fhblue = open(filename, flag, protflags)) == -1)
   {
   error = 1;
   goto Cleanup;
   } /* if no blue file */
  if (write(fhred, (char *)bitmap[0], zsize) != zsize)
   {
   error = 1;
   goto Cleanup;
   } /* if red file write error */
  if (write(fhgreen, (char *)bitmap[1], zsize) != zsize)
   {
   error = 1;
   goto Cleanup;
   } /* if green file write error */
  if (write(fhblue, (char *)bitmap[2], zsize) != zsize)
   {
   error = 1;
   goto Cleanup;
   } /* if blue file write error */
  } /* else no file interleave */

Cleanup:
 if (fred) fclose(fred);
 if (fhred >= 0) close(fhred);
 if (fhgreen >= 0) close(fhgreen);
 if (fhblue >= 0) close(fhblue);

 return (error);

} /* savebitmaps() */

/*********************************************************************/

short SaveZBuf(short zformat, short renderseg, long numpts, UBYTE *ScratchPad,
	float *ZBuf, char *Name, long Rows, long Cols)
{
 char filename[256];
 short error = 0, NewScratch = 0, AppendFile = 0;
 long fhz = -2, i, flag, BodySize, FormSize, OldHeight = 0, OldFormSize = 0,
	OldBodySize = 0, EndBody, ZBODPtr, ZBUFPtr,
	 protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE;
 float zbufmax = -FLT_MAX, zbufmin = FLT_MAX;
 struct ILBMHeader Hdr;
 struct ZBufferHeader ZBHdr;

 if (renderseg == 0 || settings.composite == 0)
  {
  flag = O_WRONLY | O_CREAT | O_TRUNC ;
  }
 else if (zformat == 2 || zformat == 3)
  {
  flag = O_WRONLY | O_CREAT | O_APPEND ;
  }
 else
  {
  AppendFile = 1;
  flag = O_RDWR ;
  }

 strcpy(filename, Name);

 for (i=0; i<numpts; i++)
  {
  if (ZBuf[i] == FLT_MAX) continue;
  if (ZBuf[i] > zbufmax) zbufmax = ZBuf[i];
  if (ZBuf[i] < zbufmin) zbufmin = ZBuf[i];
  } /* for i=0... */
 if (zbufmax < 0.0)
  zbufmax = zbufmin = FLT_MAX;

 if (zformat == 1 || zformat == 3)
  { /* gray scale */
  float incr;

  if (! ScratchPad)
   {
   NewScratch = 1;
   if ((ScratchPad = (UBYTE *)get_Memory(numpts, MEMF_ANY)) == NULL)
    {
    error = 2;
    goto Cleanup;
    } /* if no memory for data convert */
   } /* if no memory passed to function */

  BodySize = numpts;
  incr = (zbufmax - zbufmin) / 256.0;
  for (i=0; i<numpts; i++)
   {
   if (ZBuf[i] >= zbufmax) ScratchPad[i] = 0;
   else ScratchPad[i] = (zbufmax - ZBuf[i]) / incr;
   } /* for i=0... */
  } /* if convert z buffer to gray scale */
 else
  { /* floating point */
  BodySize = numpts * sizeof (float);
  } /* else save z buffer in single precision floating point format */

 if (zformat == 3)
  { /* gray scale array */
  strcat(filename, "GZB");
  if ((fhz = open(filename, flag, protflags)) == -1)
   {
   error = 1;
   goto Cleanup;
   } /* if no z buf file */
  if (write(fhz, (char *)ScratchPad, BodySize) != BodySize)
   {
   error = 1;
   goto Cleanup;
   } /* if z buf file write error */
  } /* if gray scale array */

 else if (zformat == 2)
  { /* float pt array */
  strcat(filename, "FZB");
  if ((fhz = open(filename, flag, protflags)) == -1)
   {
   error = 1;
   goto Cleanup;
   } /* if no z buf file */
  if (write(fhz, (char *)ZBuf, BodySize) != BodySize)
   {
   error = 1;
   goto Cleanup;
   } /* if zbuf file write error */
  } /* float pt array */

 else
  { /* iff */
  if (zformat == 1)
   {
   strcpy(ILBMname, Name);
   strcat(ILBMname, "ZB");
   saveILBM(8, 0, NULL, &ScratchPad, scrnrowzip, renderseg, settings.rendersegs,
	settings.composite, settings.scrnwidth, settings.scrnheight);
   } /* gray scale iff */
  else
   {
   strcat(filename, "ZB");
   if ((fhz = open(filename, flag, protflags)) == -1)
    {
    error = 1;
    goto Cleanup;
    } /* if no z buf file */
   if (AppendFile)
    {
    if (! CheckIFF(fhz, &Hdr))
     {
     error = 1;
     goto Cleanup;
     }
    OldFormSize = Hdr.ChunkSize;
    if (! FindIFFChunk(fhz, &Hdr, "ZBUF"))
     {
     error = 1;
     goto Cleanup;
     }
    ZBUFPtr = tell(fhz) - 8;
    if ((read(fhz, &ZBHdr, 36)) != 36)
     {
     error = 1;
     goto Cleanup;
     }
    if (ZBHdr.Width != settings.scrnwidth || ZBHdr.VarType != 6
	|| ZBHdr.Compression != 0 || ZBHdr.Sorting != 0 || ZBHdr.Units != 3)
     {
     error = 1;
     goto Cleanup;
     }
    OldHeight = ZBHdr.Height;
    if (ZBHdr.Min < zbufmin)
     zbufmin = ZBHdr.Min;
    if (ZBHdr.Max < zbufmax)
     zbufmax = ZBHdr.Max;
    if (! FindIFFChunk(fhz, &Hdr, "ZBOD"))
     {
     error = 1;
     goto Cleanup;
     }
    ZBODPtr = tell(fhz) - 8;
    OldBodySize = Hdr.ChunkSize;
    EndBody = ZBODPtr + 8 + OldBodySize;
    if (lseek(fhz, 0L, 2) != EndBody)
     {
     error = 1;
     goto Cleanup;
     }
    lseek(fhz, 0L, 0);
    } /* if append */
   ZBHdr.Width = Cols == 0 ? settings.scrnwidth: Cols;
   ZBHdr.Height = Rows == 0 ? settings.scrnheight + OldHeight: Rows;
   ZBHdr.VarType = 6;			/* double precision float */
   ZBHdr.Compression = 0;
   ZBHdr.Sorting = 0;			/* low = near */
   ZBHdr.Units = 3;			/* kilometers */
   ZBHdr.Min = zbufmin;
   ZBHdr.Max = zbufmax;
   ZBHdr.Bkgrnd = FLT_MAX;
   ZBHdr.ScaleFactor = 1.0;
   ZBHdr.ScaleBase = 0.0;
   if (AppendFile)
    FormSize = OldFormSize + BodySize;
   else
    FormSize = 12 + 36 + BodySize;
   strncpy(Hdr.ChunkID, "FORM", 4);
   Hdr.ChunkSize = FormSize;
   write(fhz, &Hdr, 8);
   strncpy(Hdr.ChunkID, "ILBM", 4);
   write(fhz, &Hdr, 4);
   if (AppendFile)
    lseek(fhz, ZBUFPtr, 0);
   strncpy(Hdr.ChunkID, "ZBUF", 4);
   Hdr.ChunkSize = 36;
   write(fhz, &Hdr, 8);
   write(fhz, &ZBHdr, 36);
   if (AppendFile)
    lseek(fhz, ZBODPtr, 0);
   strncpy(Hdr.ChunkID, "ZBOD", 4);
   Hdr.ChunkSize = BodySize + OldBodySize;
   write(fhz, &Hdr, 8);
   if (AppendFile)
    lseek(fhz, EndBody, 0);
   if (write(fhz, (char *)ZBuf, BodySize) != BodySize)
    {
    error = 1;
    goto Cleanup;
    } /* if zbuf file write error */
   
   } /* float pt iff */
  } /* iff format */

Cleanup:
 if (fhz >= 0) close(fhz);
 if (ScratchPad && NewScratch)
  {
  free_Memory(ScratchPad, numpts);
  } /* if new scratch space allocated here */

 return (error);

} /* SaveZBuf() */

/*********************************************************************/

void closebitmaps(UBYTE **bitmap,long zsize)
{

 if (bitmap[0]) free_Memory(bitmap[0], zsize);
 if (bitmap[1]) free_Memory(bitmap[1], zsize);
 if (bitmap[2]) free_Memory(bitmap[2], zsize);

} /* closebitmaps() */

/*********************************************************************/

void allocQCmaps(void)
{

 if ((QCmap[0] = (long *)get_Memory(QCmapsize, MEMF_CLEAR)) == NULL ||
	(QCmap[1] = (long *)get_Memory(QCmapsize, MEMF_CLEAR)) == NULL ||
	(QCmap[2] = (long *)get_Memory(QCmapsize, MEMF_CLEAR)) == NULL ||
	(QCcoords[0] = (float *)get_Memory(QCmapsize, MEMF_CLEAR)) == NULL ||
	(QCcoords[1] = (float *)get_Memory(QCmapsize, MEMF_CLEAR)) == NULL)
  {
  render ^= 0x100;
  freeQCmaps();
  } /* if ! QCmap */
 
} /* allocQCmaps() */

/*********************************************************************/

void freeQCmaps(void)
{

 if (QCmap[0]) free_Memory(QCmap[0], QCmapsize);
 QCmap[0] = (long *)NULL;
 if (QCmap[1]) free_Memory(QCmap[1], QCmapsize);
 QCmap[1] = (long *)NULL;
 if (QCmap[2]) free_Memory(QCmap[2], QCmapsize);
 QCmap[2] = (long *)NULL;
 if (QCcoords[0]) free_Memory(QCcoords[0], QCmapsize);
 QCcoords[0] = (float *)NULL;
 if (QCcoords[1]) free_Memory(QCcoords[1], QCmapsize);
 QCcoords[1] = (float *)NULL;

} /* freeQCmaps() */

/*********************************************************************/
/* saveRGB indicates a no of bitplanes */
/* if appending file, only re-write BMHD, BODY and FORM size */

short saveILBM(short saveRGB, short AskFile, struct RastPort *RPort,
	UBYTE **bitmap, long *scrnrowzip, short renderseg, short segments,
	short concat, short scrnwidth, short scrnheight)
{
 UBYTE red, green, blue, pad1 = 0, masking = 0, nplanes, compression = 1;
 UBYTE power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};
 short error = 0, regTemp, aspect = 0xa0b, temp, width, height, WriteHeight,
	rr, pp, xpos = 0, ypos = 0, transparentColor = 0, chunks, labelfieldsize,
	scrRowBytes, RowBytes, AppendFile = 0, Colors;
 USHORT Padded = 0, OldPad = 0;
 ULONG VPmode;
 char tt[5], filename[255];
 UBYTE *scrRow, *cbuf = NULL; 
 long protflags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE;
 long fHandle, EndBody;
 long BMHDsize = 20,
      CMAPsize = DEPTH * DEPTH * 3,
      CAMGsize = 4,
      BODYsize,
      FORMsize,
      kk,
      FormSizePtr,
      BodySizePtr,
      BODYPtr,
      BMHDPtr,
      TransPtr,
      OldFormSize = 0,
      OldBodySize = 0;
 PLANEPTR *Planes;
 struct CompressData CD;
 struct ILBMHeader Hdr;
 struct WcsBitMapHeader BMHdr;
 struct BusyWindow *BWIM = NULL;

 if (! saveRGB)
  {
  if (AskFile)
   {
   if (! getfilename(1, "IFF File Path/Name", graphpath, graphname))
    return (1);
   }
  strmfp(filename, graphpath, graphname);

  if (AskFile)
   {
   if ((fHandle = open(filename, O_RDONLY, 0)) != -1)
    {
    close(fHandle);
    if (! User_Message((CONST_STRPTR)graphname,
            (CONST_STRPTR)"File already exists!\nOverwrite it?", (CONST_STRPTR)"OK|CANCEL", (CONST_STRPTR)"oc"))
     return(1);
    } /* if open succeeds */
   } /* if not ask name */
  } /* if ! 24 bit */
 else
  {
  strcpy(filename, ILBMname);
  } /* else 24 bit, name provided */

 if (saveRGB && concat && segments > 1 && renderseg > 0)
  {
  if ((fHandle = open(filename, O_RDWR, 0)) == -1)
   {
   User_Message((CONST_STRPTR)filename,
           (CONST_STRPTR)"Can't open image file for output!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   error=1;
   goto Scleanup;
   } /* if open fail */
  AppendFile = 1;
  if (! CheckIFF(fHandle, &Hdr))
   {
   error = 1;
   goto Scleanup;
   } /* not IFF file */
  OldFormSize = Hdr.ChunkSize;
  if (! FindIFFChunk(fHandle, &Hdr, "BMHD"))
   {
   error = 1;
   goto Scleanup;
   } /* not IFF file */
  BMHDPtr = tell(fHandle) - 8;
  if ((read(fHandle, (char *)&BMHdr, sizeof (struct WcsBitMapHeader))) !=
	 sizeof (struct WcsBitMapHeader))
   {
   error = 1;
   goto Scleanup;
   } /* read error */
  if (BMHdr.Width != scrnwidth || BMHdr.Compression != compression)
   {
   error = 1;
   goto Scleanup;
   } /* read error */
  if (! FindIFFChunk(fHandle, &Hdr, "BODY"))
   {
   error = 1;
   goto Scleanup;
   } /* read error */
  OldBodySize = Hdr.ChunkSize;
  BODYPtr = tell(fHandle) - 8;
  EndBody = lseek(fHandle, OldBodySize, 1);
  if (EndBody != lseek(fHandle, 0L, 2))
   {
   error = 1;
   goto Scleanup;
   } /* read error */
  if (BMHdr.Transparent == 1)
   {
   OldPad = 1;
   EndBody --;
   }
  lseek(fHandle, 0L, 0);
  } /* if need to append existing file */
 else
  {
  if ((fHandle = open(filename, O_CREAT | O_TRUNC | O_RDWR, protflags)) == -1)
   {
   User_Message((CONST_STRPTR)filename,
           (CONST_STRPTR)"Can't open image file for output!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   error=1;
   goto Scleanup;
   } /* if open fail */
  } /* else */

 if (saveRGB)
  {
  chunks 	= 2;
  nplanes 	= saveRGB;
  width 	= scrnwidth;
  height 	= scrnheight;
  scrRowBytes 	= 2 * ((scrnwidth + 15) / 16);
  RowBytes 	= scrRowBytes;		/* not actually used I think */
  VPmode 	= MODES;
  BODYsize 	= scrRowBytes * scrnheight * nplanes + OldBodySize;
  Colors 	= nplanes == 24 ? 3: 1;
  } /* if RGB file */
 else if (RPort)
  {
  chunks 	= 4;
  nplanes 	= DEPTH;
  width 	= scrnwidth  /*(short)RPort->RP_User*/;
  height 	= RPort->BitMap->Rows;
  scrRowBytes 	= 2 * ((scrnwidth + 15) / 16) /* RPort->BitMap->BytesPerRow*/;
  RowBytes	= RPort->BitMap->BytesPerRow;
  VPmode 	= HIRES | LACE;
  BODYsize 	= scrRowBytes * height * DEPTH;
  Planes 	= RPort->BitMap->Planes;
  } /* if rastport passed to saveILBM() */
 else
  {
  chunks 	= 4;
  nplanes 	= DEPTH;
  width 	= WCSScrn->ViewPort.DWidth;
  height 	= WCSScrn->ViewPort.DHeight;
  scrRowBytes 	= WCSScrn->RastPort.BitMap->BytesPerRow;
  RowBytes	= scrRowBytes;
  VPmode 	= GetVPModeID(&WCSScrn->ViewPort);
  BODYsize 	= scrRowBytes * height * DEPTH;
  Planes 	= WCSScrn->RastPort.BitMap->Planes;
  } /* else not RGB */
 WriteHeight = height;

 if (AppendFile)
  {
  height += BMHdr.Height;
  } /* if appending existing file */

 if ((cbuf = (char *)get_Memory(scrnwidth + 16, MEMF_CLEAR)) == NULL)
  {
  error = 1;
  goto Scleanup;
  } /* if no buffer memory */

/* Form size will vary depending on whether there is a CMAP & CAMG chunk */
 labelfieldsize = chunks * 8 + 4;
 if (AppendFile)
  {
  FORMsize = OldFormSize - OldBodySize + BODYsize;
  }
 else
  { 
  FORMsize = BMHDsize + BODYsize + labelfieldsize;
  if (! saveRGB) FORMsize += (CMAPsize + CAMGsize);
  }

 strcpy(tt, "FORM");
 write(fHandle, tt, 			4);
 FormSizePtr = tell(fHandle);
 write(fHandle, &FORMsize, 		4);
 strcpy(tt, "ILBM");
 if (write(fHandle, tt, 		4) != 4)
  {
  error = 1;
  goto Scleanup;
  } /* if FORM write fail */
 if (AppendFile)
  lseek(fHandle, BMHDPtr, 0);
 strcpy(tt, "BMHD");
 write(fHandle, tt, 			4);
 write(fHandle, &BMHDsize, 		4);
 write(fHandle, &width, 		2);
 write(fHandle, &height, 		2);
 write(fHandle, &xpos, 			2);
 write(fHandle, &ypos, 			2);
 write(fHandle, &nplanes, 		1);
 write(fHandle, &masking, 		1);
 write(fHandle, &compression, 		1);
 write(fHandle, &pad1, 			1);
 TransPtr = tell(fHandle);
 write(fHandle, &transparentColor, 	2);
 write(fHandle, &aspect, 		2);
 write(fHandle, &width, 		2);
 if (write(fHandle, &height, 		2) != 2)
  {
  error = 1;
  goto Scleanup;
  } /* if BMHD write fail */

/* skip CMAP if 24 bit image or else overflow of cbuf buffer may occur */
 if (! saveRGB && ! AppendFile)
  {
  strcpy(tt,"CMAP");
  write(fHandle, tt,			4);
  write(fHandle, &CMAPsize,		4);

  for (kk=0; kk<DEPTH*DEPTH; kk++)
   {
   regTemp			= GetRGB4(WCSScrn->ViewPort.ColorMap, kk);
   temp				= (regTemp & 0xf00);
   red				= temp / 16;
   *(cbuf + (kk * 3))		= red;
   temp				= (regTemp &0xf0);
   green			= temp;
   *(cbuf + (kk * 3) +1)	= green;
   temp				= (regTemp & 0xf) * 16;
   blue				= temp; 
   *(cbuf + (kk * 3) + 2)	= blue;
   } /* for kk=0... */

  if (write(fHandle, cbuf,		CMAPsize) != CMAPsize)
   {
   error = 1;
   goto Scleanup;
   } /* if CMAP write error */
  } /* if not 24 bit */

 if (! saveRGB && ! AppendFile)
  {
  strcpy(tt, "CAMG");
  write(fHandle, tt,			4);
  write(fHandle, &CAMGsize,		4);
  if (write(fHandle, &VPmode,		4) != 4)
   {
   error = 1;
   goto Scleanup;
   } /* if CAMG write error */
  }

 if (AppendFile)
  lseek(fHandle, BODYPtr, 0);
 strcpy(tt, "BODY");
 write(fHandle, tt,			4);
 BodySizePtr = tell(fHandle);
 write(fHandle, &BODYsize,		4);

 if (AppendFile)
  {
  lseek(fHandle, EndBody, 0);
  }

 if (compression)
  {
  CD.OutSize = BODYsize;
RepeatMemGrab:
  if ((CD.OutArray = (union MultiByte *)get_Memory(CD.OutSize, MEMF_ANY)) == NULL)
   {
   CD.OutSize /= 2;
   if (CD.OutSize > 0) goto RepeatMemGrab;
   error = 1;
   goto Scleanup;
   } /* if buffer size still greater than 0 - IT BETTER BE !!! */
  CD.Rows = nplanes;
  CD.RowBytes = scrRowBytes;
  CD.OutCtr = 0;
  CD.MaxByteRun = 127;
  CD.TotalOutBytes = 0;
  CD.fHandle = fHandle;
  } /* if compression */

 BWIM = BusyWin_New("Saving Image", WriteHeight, 0, MakeID('B','W','I','M'));

 if (saveRGB)
  {
  UBYTE mask;
  short color, pixel;
  long byte;
  UBYTE *RowBuffer, *BuffPtr;
  LONG RowBufferSize;

  RowBufferSize = scrRowBytes * 3 * 8;
  if ((RowBuffer = (UBYTE *)get_Memory(RowBufferSize, MEMF_ANY)) != NULL)
   {
   CD.Data = RowBuffer;

   for (rr=0; rr<WriteHeight; rr++)
    {
    BuffPtr = RowBuffer;
    memset(RowBuffer, 0, RowBufferSize);

    for (color=0; color<Colors; color ++)
     {
     memcpy (cbuf, (bitmap[color] + scrnrowzip[rr]), scrnwidth);
     for (pp=0; pp<8; pp++)
      {
      mask = power2[pp];
      scrRow = cbuf;
      for (byte=0; byte<scrRowBytes; byte++)
       {
       for (pixel=7; pixel>=0; pixel--)
        {
        if (*scrRow & mask) *BuffPtr += (1 << pixel);
        scrRow ++;
        } /* for pixel=0... */
       BuffPtr ++;
       } /* for byte=0... */
      } /* for pp=0... */
     } /* for color=0... */
    if (compression)
     {
     if ((error = CompressRows(&CD)) > 0)
      {
      free_Memory(RowBuffer, RowBufferSize);
      goto Scleanup;
      } /* if write error */
     } /* if compression */      
    else
     {
     if (write(fHandle, RowBuffer, 		RowBufferSize) != RowBufferSize)
      {
      error = 1;
      free_Memory(RowBuffer, RowBufferSize);
      goto Scleanup;
      } /* if */
     } /* if no compression */
    BusyWin_Update(BWIM, rr + 1);
    } /* for rr=0... */
   if (compression && CD.OutCtr > 0)
    {
    if ((error = FlushOutputBuff(&CD)) > 0)
     {
     goto Scleanup;
     } /* if write error */
    CD.TotalOutBytes += CD.OutCtr;
    } /* if some data remaining in output buffer */

   free_Memory(RowBuffer, RowBufferSize);
   } /* if row buffer allocated */

  else
   {
   if ((RowBuffer = get_Memory(scrRowBytes, MEMF_CLEAR)) != NULL)
    {
    for (rr=0; rr<WriteHeight; rr++)
     {
     for (color=0; color<Colors; color ++)
      {
      memcpy(cbuf, (bitmap[color] + scrnrowzip[rr]), scrnwidth);
      for (pp=0; pp<8; pp++)
       {
       mask = power2[pp];
       scrRow = cbuf;
       memset(RowBuffer, 0, scrRowBytes);
       for (byte=0; byte<scrRowBytes; byte++)
        {
        for (pixel=7; pixel>=0; pixel--)
         {
         if (*scrRow & mask) *(RowBuffer + byte) += (1 << pixel);
         scrRow ++;
         } /* for pixel=0... */
        } /* for byte=0... */
       if (write(fHandle, RowBuffer, 		scrRowBytes) != scrRowBytes)
        {
        error = 1;
        free_Memory(RowBuffer, scrRowBytes);
        goto Scleanup;
        } /* if */
       } /* for pp=0... */
      } /* for color=0... */
     BusyWin_Update(BWIM, rr + 1);
     } /* for rr=0... */
    free_Memory(RowBuffer, scrRowBytes);
    } /* if memory allocated for row buffer */
   else
    {
    error = 1;
    goto Scleanup;
    } /* else can't corner turn, no memory  */
   } /* else not enough memory for row buffer */
  } /* if 24 or 8 bit */

 else
  { /* not 24 or 8 bit */
  CD.Rows = 1;
  for (rr=0; rr<WriteHeight; rr++)
   {
   for (pp=0; pp<DEPTH; pp++)
    {
    scrRow = Planes[pp] + (rr * RowBytes);

    CD.Data = scrRow;
    if (compression)
     {
     if ((error = CompressRows(&CD)) > 0)
      {
      goto Scleanup;
      } /* if write error */
     } /* if byte run 1 compression */
    else
     {
     if (write(fHandle, scrRow, scrRowBytes) != scrRowBytes)
      {   
      error = 1;
      goto Scleanup;
      } /* if BODY write error */
     } /* else no compression */
    } /* for pp=0... */
   BusyWin_Update(BWIM, rr + 1);
   } /* for rr=0... */

  if (compression && CD.OutCtr > 0)
   {
   if ((error = FlushOutputBuff(&CD)) > 0)
    {
    goto Scleanup;
    } /* if write error */
   CD.TotalOutBytes += CD.OutCtr;
   } /* if some data remaining in output buffer */

  } /* else not 24 or 8 bit */

 if ((CD.TotalOutBytes + OldBodySize - OldPad) % 2)
  {
  write(fHandle, &pad1, 			1);
  CD.TotalOutBytes ++;
  Padded = 1;
  } /* if odd number of output bytes */

 if (compression || AppendFile)
  {
  lseek(fHandle, FormSizePtr, 0);
  FORMsize -= BODYsize;
  FORMsize += (CD.TotalOutBytes + OldBodySize - OldPad);
  write(fHandle, &FORMsize,	 		4);
  lseek(fHandle, BodySizePtr, 0);
  BODYsize = CD.TotalOutBytes + OldBodySize - OldPad;
  write(fHandle, &BODYsize,	 		4);
/* Use Transparent Color in BMHD for notice of pad byte for later appending */
  if (Padded)
   {
   lseek(fHandle, TransPtr, 0);
   write(fHandle, &Padded, 2);
   }
  } /* if compression */

Scleanup:
 if (BWIM)
  BusyWin_Del(BWIM);
 if (fHandle >= 0) close(fHandle);
 if (CD.OutArray) free_Memory(CD.OutArray, CD.OutSize);
 if (cbuf) free_Memory(cbuf, scrnwidth + 16);
 if (error)
  {
  if (AskFile)
   User_Message((CONST_STRPTR)filename, (CONST_STRPTR)"Error saving image!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_WRITE_FAIL, filename);
  } /* if error */

 return (error);

} /* saveILBM() */

/***********************************************************************/

void ModFileName(char *tempfile, short renderseg, short imagearray,
	 short imagenum)
{
 char framestr[32], suffix[5] = {0};   // suffix[5] !, room for 4 chars, e.g ".grn" + terminating null byte
 short i, j, namelen, firstdigit = -1, lastdigit = -1, diglen = 0,
	digstrlen, suffixlen;

 namelen = strlen(tempfile);
 for (i=0; i<namelen; i++)
  {
  if (isdigit(tempfile[i]))
   {
   firstdigit = i;
   for (i++; i<namelen; i++)
    {
    if (! isdigit(tempfile[i]))
     {
     break;
     } /* if end of number found */
    } /* for i<namelen */
   lastdigit = i;
   diglen = i - firstdigit;
   } /* if first digit found */
  } /* for i=0... */

 if (settings.rendersegs > 1)
  {
  if (settings.rendersegs <= 26)
   {
   sprintf(suffix, "%c", 65 + renderseg);
   } /* if suffix z or less */
  else
   {
   sprintf(suffix, "%c%c", (65 + renderseg / 26), (65 + renderseg % 26));
   } /* else */
  } /* if rendering in segments */

 if (diglen > 0)
  {
  sprintf(framestr, "%d", frame);
  digstrlen = strlen(framestr);
  strcat(framestr, suffix);
  suffixlen = strlen(suffix);
  for (i=firstdigit; i<lastdigit-digstrlen; i++)
   {
   tempfile[i] = '0';
   }
  for (i=lastdigit-digstrlen, j=0; i<lastdigit+suffixlen; i++, j++)
   {
   tempfile[i] = framestr[j];
   } /* for i=... */
  } /* if digit in name */

 if (imagearray)
  {
  namelen = strlen(tempfile);
  if (! stricmp(&tempfile[namelen - 4], ".red"))
   {
   if (imagenum)
    {
    tempfile[namelen - 4] = '\0';
    switch (imagenum)
     {
     case 1:
      {
      strcpy(suffix, ".grn");
      break;
      } /* case 2 */
     case 2:
      {
      strcpy(suffix, ".blu");
      break;
      } /* case 3 */
     } /* switch imagenum */
    strcat(tempfile, suffix);
    } /* if */
   } /* if separate red, green and blue files - otherwise don't change name */
  } /* if array of images must be RGB files */

} /* ModFileName() */

/***********************************************************************/

STATIC_FCN short CompressRows(struct CompressData *CD) // used locally only -> static, AF 20.7.2021
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
   while ((newmode = CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1]) == mode)
    {
    byte ++;
    writebytes ++;
    if (writebytes == CD->MaxByteRun)
     {
     if (mode)
      {
      newmode = CD->Data[RowBase + byte] != CD->Data[RowBase + byte + 1];
      if (! newmode && byte != lastbyte) writebytes --;
      } /* if current mode = 1 */
     else
      {
      newmode = CD->Data[RowBase + byte + 1] != CD->Data[RowBase + byte + 2];
      } /* else current mode = 0 */
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
     CD->OutArray[CD->OutCtr].Bt = writebytes - 1;
     memcpy(&CD->OutArray[CD->OutCtr + 1].UBt, &CD->Data[RowBase + startbyte], writebytes);
     CD->OutCtr += (writebytes + 1);
     } /* if some data to write */
    } /* if mode == 1 values unequal */
   else
    {
    CD->OutArray[CD->OutCtr].Bt = -writebytes;
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
    } /* if buffer almost full */

   forcebreak = 0;
   if (! mode && ! newmode) writebytes = 0;
   else
    {
    mode = newmode;
    writebytes = 1 - mode;
    } /* else */
   } /* for byte<lastbyte */

  if (error) break;
  if (CD->OutCtr > CD->OutSize - CD->MaxByteRun - 2)
   {
   if ((error = FlushOutputBuff(CD)) > 0) break;
   CD->TotalOutBytes += CD->OutCtr;
   CD->OutCtr = 0;
   } /* if buffer almost full */

  RowBase += CD->RowBytes;
  } /* for i=0... rows of data */

 return (error);

} /* main() */

/***********************************************************************/

STATIC_FCN short FlushOutputBuff(struct CompressData *CD) // used locally only -> static, AF 20.7.2021
{

 if (write (CD->fHandle, CD->OutArray, CD->OutCtr) != CD->OutCtr)
  return (1);

 return (0);

} /* FlushOutputBuff() */

/***********************************************************************/

short LoadImage(char *Name, short ColorImage, UBYTE **bitmap,
	short Width, short Height, short SupressWng,
	short *NewWidth, short *NewHeight, short *NewPlanes)
{
 UBYTE power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};
 short copyred = 0, namelen, error = 0, pp, rr, color, pixel;
 long byte, x, fh, RowDataSize, InputDataSize, BytesRead, RowSize;
 union MultiByte *InputData = NULL, *InputDataPtr;
 UBYTE *RowData = NULL, *RowDataPtr, *BuffPtr, *BitmapPtr;
 struct ILBMHeader Hdr;
 struct WcsBitMapHeader BMHdr;
 struct BusyWindow *BWIM = NULL;

RepeatOpen:
 if ((fh = open(Name, O_RDONLY, 0)) == -1)
  {
  namelen = strlen(Name);
  if (namelen > 1)
  if (Name[namelen - 1] == ' ')
   {
   Name[namelen - 1] = 0;
   goto RepeatOpen;
   } /* if trailing blank */
  error = 1;
  goto Cleanup;
  }

 if (CheckIFF(fh, &Hdr))
  { 
  if (FindIFFChunk(fh, &Hdr, "BMHD"))
   {
   if ((read(fh, (char *)&BMHdr, sizeof (struct WcsBitMapHeader))) !=
	 sizeof (struct WcsBitMapHeader))
    {
    close(fh);
    error = 2;
    goto Cleanup;
    } /* read error */
   if ((ColorImage && BMHdr.Planes != 24 && bitmap[0])
	 || (! ColorImage && BMHdr.Planes != 8 && bitmap[0]) ||
	(BMHdr.Planes != 8 && BMHdr.Planes != 24))
    {
    close(fh);
    error = 4;
    goto Cleanup;
    } /* wrong number of bit planes */
   else if (! bitmap[0])
    {
/* bitmaps not allocated - do so now! */
    Width = BMHdr.Width;
    Height = BMHdr.Height;
    *NewWidth = Width;
    *NewHeight = Height;
    if ((bitmap[0] = (UBYTE *)get_Memory(Width * Height, MEMF_ANY)))
     {
     *NewPlanes = 8;
     if (BMHdr.Planes == 24)
      {
      *NewPlanes = 24;
      if ((bitmap[1] = (UBYTE *)get_Memory(Width * Height, MEMF_ANY)))
       {
       if ((bitmap[2] = (UBYTE *)get_Memory(Width * Height, MEMF_ANY)) == NULL)
        error = 5;
       } /* if */
      else
       error = 5;
      } /* if planes = 24 */
     } /* if */
    else
     error = 5;
    if (error)
     {
     close(fh);
     goto Cleanup;
     } /* if */
    } /* else need to allocate bitmaps */
   if (BMHdr.Width != Width || BMHdr.Height != Height)
    {
    close(fh);
    error = 3;
    goto Cleanup;
    } /* wrong image size */

   if (FindIFFChunk(fh, &Hdr, "BODY"))
    {
    InputDataSize = Hdr.ChunkSize;
    if ((InputData = (union MultiByte *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
     {
     close(fh);
     error = 5;
     goto Cleanup;
     } /* if out of memory */
    RowSize = 2 * ((Width + 15) / 16);
    RowDataSize = RowSize * BMHdr.Planes;
    if ((RowData = (UBYTE *)get_Memory(RowDataSize, MEMF_CLEAR)) == NULL)
     {
     close(fh);
     error = 5;
     goto Cleanup;
     } /* if out of memory */

    if ((read(fh, (char *)InputData, InputDataSize)) != InputDataSize)
     {
     close(fh);
     error = 2;
     goto Cleanup;
     } /* if read error */
    close(fh);

/* clear bitmaps */
    for (color=0; color<BMHdr.Planes/8; color++)
     memset(bitmap[color], 0, Width * Height);

   BWIM = BusyWin_New("Loading Image", Height, 0, MakeID('B','W','I','M'));

/* read the data */
    InputDataPtr = InputData;
    for (rr=0; rr<Height; rr++)
     {
     RowDataPtr = RowData;
     if (BMHdr.Compression)
      {
      for (color=0; color<BMHdr.Planes/8; color++)
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
          } /* if literal run */
         else
          {
          memset(RowDataPtr, InputDataPtr[1].UBt, 1 - InputDataPtr[0].Bt);
          BytesRead += (1 - InputDataPtr[0].Bt);
          RowDataPtr += (1 - InputDataPtr[0].Bt);
          InputDataPtr += 2;
          } /* else replicate run */
         } /* while */
        if (BytesRead > RowSize)
         {
         error = 2;
         goto Cleanup;
         } /* if compression error */
        } /* for pp=0... */
       } /* for color=0... */
      } /* if compressed data */
     else
      {
      memcpy(RowDataPtr, InputDataPtr, RowDataSize);
      InputDataPtr += RowDataSize;
      } /* else not compressed */
/* do corner turn for all planes, one row */
     BuffPtr = RowData;
     for (color=0; color<BMHdr.Planes/8; color++)
      {
      BitmapPtr = bitmap[color] + rr * Width;
      for (pp=0; pp<8; pp++)
       {
       for (byte=0,x=0; byte<RowSize; byte++, BuffPtr++)
        {
        for (pixel=7; pixel>=0 && x<Width; pixel--, x++)
         {
         if (*BuffPtr & power2[pixel]) BitmapPtr[x] += (1 << pp);
         } /* for pixel=7... */
        } /* for byte=0... */
       } /* for pp=0... */
      } /* for color=0... */
     BusyWin_Update(BWIM, rr + 1);
     } /* for rr=0... */
    } /* if BODY found */
   else
    {
    close (fh);
    error = 2;
    goto Cleanup;
    } /* else no BODY */
   } /* if BMHD found */
  else
   {
   close (fh);
   error = 2;
   goto Cleanup;
   } /* else no BMHD */
  } /* if IFF file */

 else
  { /* not IFF file */
  BWIM = BusyWin_New("Loading Image", Height, 0, MakeID('B','W','I','M'));

  InputDataSize = Width * Height;
  if (lseek(fh, 0L, 2) != InputDataSize)
   {
   close(fh);
   error = 3;
   goto Cleanup;
   } /* wrong file size */
  lseek(fh, 0L, 0);
  if (! ColorImage)
   {
   if (read(fh, (char *)(*bitmap), InputDataSize) != InputDataSize)
    {
    close(fh);
    error = 2;
    goto Cleanup;
    } /* if read error */
   close(fh);
   } /* if gray image */
  else
   {
   if (read(fh, (char *)bitmap[0], InputDataSize) != InputDataSize)
    {
    close(fh);
    error = 2;
    goto Cleanup;
    } /* if read error */
   close(fh);
   namelen = strlen(Name);
   if (! stricmp(&Name[namelen - 4], ".red"))
    {
    Name[namelen - 4] = 0;
    strcat(Name, ".grn");
    if ((fh = open(Name, O_RDONLY, 0)) == -1)
     {
     copyred = 1;
     }
    else if (read(fh, (char *)bitmap[1], InputDataSize) != InputDataSize)
     {
     close(fh);
     error = 2;
     goto Cleanup;
     }
    else
     {
     close(fh);
     Name[namelen - 4] = 0;
     strcat(Name, ".blu");
     if ((fh = open(Name, O_RDONLY, 0)) == -1)
      {
      copyred = 1;
      }
     else if (read(fh, (char *)bitmap[2], InputDataSize) != InputDataSize)
      {
      close(fh);
      error = 2;
      goto Cleanup;
      }
     else
      {
      close(fh);
      } /* else read blue OK */
     } /* else read green OK */
    } /* if filename ends in ".red" */
   else
    copyred = 1;
   if (copyred)
    {
    memcpy(bitmap[1], bitmap[0], InputDataSize);
    memcpy(bitmap[2], bitmap[0], InputDataSize);
    } /* else copy red data */

   } /* else color image */
  } /* Raw file ? */

Cleanup:
 if (BWIM)
  BusyWin_Del(BWIM);
 if (InputData)  free_Memory(InputData, InputDataSize);
 if (RowData) free_Memory(RowData, RowDataSize);

 switch (error)
  {
  case 1:
   {
   if (! SupressWng) Log(WNG_OPEN_FAIL, Name);
   break;
   }
  case 2:
   {
   Log(ERR_READ_FAIL, Name);
   break;
   }
  case 3:
   {
   Log(ERR_READ_FAIL, (CONST_STRPTR)"Wrong image size");
   break;
   }
  case 4:
   {
   Log(ERR_READ_FAIL, (CONST_STRPTR)"Wrong image depth");
   break;
   }
  case 5:
   { /* logged in memory module */
   break;
   } /* memory fail */
  } /* switch error */

 if (error) return (0);
 return (1);

} /* LoadImage() */

/***********************************************************************/

STATIC_FCN short LoadZBuf(char *Name, float *ZBuf, struct ZBufferHeader *ZBHdr,
	short Width, short Height) // used locally only -> static, AF 20.7.2021
{
 short success = 0;
 long fh, ReadSize;
 struct ILBMHeader Hdr;

 ReadSize = Width * Height * sizeof (float);

 if ((fh = open(Name, O_RDONLY, 0)) != -1)
  {
  if (CheckIFF(fh, &Hdr))
   {
   if (FindIFFChunk(fh, &Hdr, "ZBUF"))
    {
    if (read(fh, (char *)ZBHdr, sizeof (struct ZBufferHeader)) == sizeof (struct ZBufferHeader))
     {
     if (FindIFFChunk(fh, &Hdr, "ZBOD"))
      {
      if (ZBHdr->VarType == 6)
       {
       if ((read(fh, (char *)ZBuf, ReadSize)) == ReadSize)
        {
        success = 1;
	} /* if data read OK */
       } /* if single precision floating point */
      } /* if ZBOD chunk found */
     } /* if header read OK */
    } /* if ZBUF chunk found */
   } /* if IFF file */
  else
   {
   if (lseek(fh, 0L, 2) == ReadSize)
    {
    lseek(fh, 0L, 0);
    if ((read(fh, (char *)ZBuf, ReadSize)) == ReadSize)
     {
     success = 1;
     }
    } /* if file size OK for single precision floating point */
   } /* else not IFF file */
  close (fh);
  } /* if file opened OK */
 else
  {
  Log(ERR_OPEN_FAIL, Name);
  } /* else open fail */

 return (success);

} /* LoadZBuf() */

/***********************************************************************/

short CheckIFF(long fh, struct ILBMHeader *Hdr)
{

 if ((read(fh, (char *)Hdr, sizeof (struct ILBMHeader))) == sizeof (struct ILBMHeader))
  {
  if (! strncmp(Hdr->ChunkID, "FORM", 4))
   {
   if ((read(fh, (char *)Hdr->ChunkID, 4)) == 4)
    {
    if (! strncmp(Hdr->ChunkID, "ILBM", 4))
     {
     return (1);
     } /* if ILBM found */
    } /* if read OK */
   } /* if FORM found */
  } /* if header read */

 return (0);

} /* CheckIFF() */

/***********************************************************************/

short FindIFFChunk(long fh, struct ILBMHeader *Hdr, char *Chunk)
{
 short error = 0;
 long readsize;

 while ((readsize = read(fh, (char *)Hdr, sizeof (struct ILBMHeader))) ==
	sizeof (struct ILBMHeader))
  {
  if (! strncmp(Hdr->ChunkID, Chunk, 4))
   break;
  if ((lseek(fh, Hdr->ChunkSize, 1)) == -1)
   {
   error = 1;
   break;
   } /* if file size error */
  } /* until bit map header found */
 if (readsize != sizeof (struct ILBMHeader)) error = 1;

 if (error) return (0);
 return (1);

} /* FindIFFChunk() */

/***********************************************************************/

#define BKGRND_NULL	0
#define BKGRND_REPLACE	1
#define BKGRND_MERGE	2
#define BG_FORMAT_RAW	0
#define BG_FORMAT_IFF	1

short MergeZBufBack(short renderseg, short Width, short Height, struct Window *win)
{
 UBYTE power2[8] = {1, 2, 4, 8, 16, 32, 64, 128};
 char tempfile[32], filename[256];
 short error = 0, ZBufLoaded = 0, BkGrndLoaded = 0, BGFormat = 1, Operation,
	pp, color, pixel, MergePts, DummyShort;
 float *ZBufAlt = NULL;
 BYTE ByteRun;
 UBYTE ReadSize, *RowData = NULL, *BkGrnd[3] = {NULL, NULL, NULL}, *BuffPtr,
	 *BitmapPtr;
 long x, y, ZBufSize, BkGrndSize, zip, colval, BGzip, ZBzip, fhZ = 0,
	 fhBR = 0, fhBG = 0, fhBB = 0, RowSize, BytesRead, RowDataSize, byte;
 struct ILBMHeader Hdr;
 struct WcsBitMapHeader BMHdr;
 struct ZBufferHeader ZBHdr;
 struct BusyWindow *BWDE;

 BkGrndSize = Width * Height;
 ZBufSize = BkGrndSize * sizeof (float);

 BWDE = BusyWin_New("Background", Height, 0, MakeID('B','W','D','E'));

 if (settings.zbuffer)
  {
  strcpy(tempfile, zbufferfile);
  ModFileName(tempfile, renderseg, 0, 0);
  strmfp(filename, zbufferpath, tempfile);
  if ((ZBufAlt = (float *)get_Memory(ZBufSize, MEMF_ANY)) != NULL)
   {
   if (! LoadZBuf(filename, ZBufAlt, &ZBHdr, Width, Height))
    {
    error = 1;
    goto EndMerge;
    } /* if error loading z buffer */
   ZBufLoaded = 1;
   } /* if full memory allocated */
  else
   {
   ZBufSize = Width * sizeof (float);
   if ((ZBufAlt = (float *)get_Memory(ZBufSize, MEMF_ANY)) == NULL)
    {
    error = 2;
    goto EndMerge;
    } /* if out of memory */
/* open z buffer file, if IFF find ZBOD chunk */
   if ((fhZ = open(filename, O_RDONLY, 0)) == -1)
    {
    error = 3;
    goto EndMerge;
    } /* if file open error */
   if (CheckIFF(fhZ, &Hdr))
    {
    if (FindIFFChunk(fhZ, &Hdr, "ZBUF"))
     {
     read(fhZ, (char *)&ZBHdr, sizeof (struct ZBufferHeader));
     if (ZBHdr.VarType == 6)
      {
      if (! FindIFFChunk(fhZ, &Hdr, "ZBOD"))
       {
       error = 5;
       goto EndMerge;
       } /* if can't find ZBOD chunk */
      }
     else
      {
      error = 4;
      goto EndMerge;
      } /* if wrong variable type */
     } /* if found ZBUF chunk */
    else
     {
     error = 6;
     goto EndMerge;
     } /* else can't find ZBUF chunk */
    } /* if IFF file */
   else
    {
    if (lseek(fhZ, 0L, 2) != Width * Height * sizeof (float))
     {
     error = 7;
     goto EndMerge;
     } /* if wrong size file */
    lseek(fhZ, 0L, 0);
    } /* else not IFF file */
   } /* if no complete z buffer */
  } /* if pre-load z buffer */

 if (settings.background)
  {
  strcpy(tempfile, backgroundfile);
  ModFileName(tempfile, renderseg, 0, 0);
  strmfp(filename, backgroundpath, tempfile);
  if (! openbitmaps(BkGrnd, BkGrndSize))
   {
   if (! LoadImage(filename, 1, BkGrnd, Width, Height, 1,
	 &DummyShort, &DummyShort, &DummyShort))
    {
    error = 8;
    goto EndMerge;
    } /* if error loading background image */
   BkGrndLoaded = 1;
   } /* if bitmaps allocated */
  else
   {
   BkGrndSize = Width;
   if (! openbitmaps(BkGrnd, BkGrndSize))
    {
    error = 9;
    goto EndMerge;
    } /* if out of memory */
/* open background file, read header, if IFF find BODY chunk */
   if ((fhBR = open(filename, O_RDONLY, 0)) == -1)
    {
    error = 10;
    goto EndMerge;
    } /* if file open error */
   if (CheckIFF(fhBR, &Hdr))
    {
    if (FindIFFChunk(fhBR, &Hdr, "BMHD"))
     {
     read(fhBR, (char *)&BMHdr, sizeof (struct WcsBitMapHeader));
     if (BMHdr.Width != Width || BMHdr.Height != Height || BMHdr.Planes != 24)
      {
      error = 11;
      goto EndMerge;
      } /* if wrong size */
     if (! FindIFFChunk(fhBR, &Hdr, "BODY"))
      {
      error = 12;
      goto EndMerge;
      } /* if can't find BODY chunk */
     RowDataSize = Width * 24;
     RowSize = (Width + 7) / 8;
     if ((RowData = (UBYTE *)get_Memory(RowDataSize, MEMF_CLEAR)) == NULL)
      {
      error = 9;
      goto EndMerge;
      } /* if out of memory */
     BGFormat = BG_FORMAT_IFF;
     } /* if found BMHD chunk */
    else
     {
     error = 13;
     goto EndMerge;
     } /* else can't find BMHD chunk */
    } /* if IFF file */
   else
    {
    if (lseek(fhBR, 0L, 2) != Width * Height)
     {
     error = 11;
     goto EndMerge;
     } /* if wrong size */
    lseek(fhBR, 0L, 0);
    strcpy(tempfile, backgroundfile);
    ModFileName(tempfile, renderseg, 1, 1);
    strmfp(filename, backgroundpath, tempfile);
    if ((fhBG = open(filename, O_RDONLY, 0)) == -1)
     {
     error = 10;
     goto EndMerge;
     } /* if file open error */
    if (lseek(fhBG, 0L, 2) != Width * Height)
     {
     error = 11;
     goto EndMerge;
     } /* if wrong size */
    lseek(fhBG, 0L, 0);
    strcpy(tempfile, backgroundfile);
    ModFileName(tempfile, renderseg, 1, 2);
    strmfp(filename, backgroundpath, tempfile);
    if ((fhBB = open(filename, O_RDONLY, 0)) == -1)
     {
     error = 10;
     goto EndMerge;
     } /* if file open error */
    if (lseek(fhBB, 0L, 2) != Width * Height)
     {
     error = 11;
     goto EndMerge;
     } /* if wrong size */
    lseek(fhBB, 0L, 0);
    BGFormat = BG_FORMAT_RAW;
    } /* else not IFF */
   } /* if no complete background */
  } /* if use background image */

 zip = BGzip = ZBzip = 0;
 for (y=0; y<Height; y++)
  {
  if (! BkGrndLoaded && settings.background)
   {
   if (BGFormat == BG_FORMAT_IFF)
    {
    if (BMHdr.Compression)
     {
     BytesRead = 0;
     while (BytesRead < RowSize)
      {
      if ((read(fhBR, (char *)&ByteRun, 1)) != 1)
       {
       error = 11;
       break;
       } /* if read error */
      ReadSize = abs(ByteRun + 1);
      if (BytesRead + ReadSize > RowSize)
       {
       error = 14;
       break;
       } /* if compression error */
      if (ByteRun >= 0)
       {
       if ((read(fhBR, (char *)&RowData[BytesRead], ReadSize)) != ReadSize)
        {
        error = 11;
        break;
	} /* if read error */
       } /* if literal run */
      else
       {
       if ((read(fhBR, (char *)&RowData[BytesRead], 1)) != 1)
        {
        error = 11;
        break;
	} /* if read error */
       for (x=1; x<=-ByteRun; x++)
        {
        RowData[BytesRead + x] = RowData[BytesRead];
	}
       } /* else replicate run */
      BytesRead += ReadSize;
      } /* while */
     } /* if compressed IFF */
    else
     {
     if ((read(fhBR, (char *)RowData, RowDataSize)) != RowDataSize)
      {
      error = 11;
      break;
      } /* if read error */
     } /* else uncompressed IFF */
/* corner turn one row */
    BuffPtr = RowData;
    for (color=0; color<3; color++)
     {
     BitmapPtr = BkGrnd[color];
     for (pp=0; pp<8; pp++)
      {
      for (byte=0,x=0; byte<RowSize; byte++, BuffPtr++)
       {
       for (pixel=7; pixel>=0 && x<Width; pixel--, x++)
        {
        if (*BuffPtr & power2[pixel]) BitmapPtr[x] += (1 << pp);
        } /* for pixel=7... */
       } /* for byte=0... */
      } /* for pp=0... */
     } /* for color=0... */
    } /* if IFF background file */
   else
    {
    if ((read(fhBR, (char *)BkGrnd[0], BkGrndSize)) != BkGrndSize
	|| (read(fhBG, (char *)BkGrnd[1], BkGrndSize)) != BkGrndSize
	|| (read(fhBB, (char *)BkGrnd[2], BkGrndSize)) != BkGrndSize)
     {
     error = 11;
     break;
     } /* if read error */
    } /* else raw background file */
   BGzip = 0;
   }
  if (! ZBufLoaded && settings.zbuffer)
   {
   if ((read(fhZ, (char *)ZBufAlt, ZBufSize)) != ZBufSize)
    {
    error = 7;
    break;
    } /* if read error */
   ZBzip = 0;
   }
  for (x=0; x<Width; x++, zip++, BGzip++, ZBzip++)
   {
   Operation = BKGRND_NULL;
   if (settings.zbuffer)
    {
    if (! bytemap[zip] || ZBufAlt[ZBzip] < zbuf[zip])
     {
     Operation = BKGRND_REPLACE;
     zbuf[zip] = ZBufAlt[ZBzip];
     bytemap[zip] = 0;
     } /* if just replace */
    else if (bytemap[zip] < 100 && zbuf[zip] < ZBufAlt[ZBzip])
     {
     Operation = BKGRND_MERGE;
     } /* if need to merge */ 
    } /* if zbuffer */
   else
    {
    if (bytemap[zip] < 100)
     {
     if (! bytemap[zip])
      {
      Operation = BKGRND_REPLACE;
      } /* else no bitmap value, just replace */
     else
      {
      Operation = BKGRND_MERGE;
      } /* if a value present */
     } /* if need to merge */ 
    } /* else background only */

   if (settings.background)
    {
    switch (Operation)
     {
     case BKGRND_REPLACE:
      {
      *(bitmap[0] + zip) = *(BkGrnd[0] + BGzip);
      *(bitmap[1] + zip) = *(BkGrnd[1] + BGzip);
      *(bitmap[2] + zip) = *(BkGrnd[2] + BGzip);
      MergePts = 100;
      break;
      } /* replace */
     case BKGRND_MERGE:
      {
      MergePts = 100 - bytemap[zip];
      colval = (*(bitmap[0] + zip) * bytemap[zip] +
	 *(BkGrnd[0] + BGzip) * MergePts) / 100;
      *(bitmap[0] + zip) = (UBYTE)colval;
      colval = (*(bitmap[1] + zip) * bytemap[zip] +
	 *(BkGrnd[1] + BGzip) * MergePts) / 100;
      *(bitmap[1] + zip) = (UBYTE)colval;
      colval = (*(bitmap[2] + zip) * bytemap[zip] +
	 *(BkGrnd[2] + BGzip) * MergePts) / 100;
      *(bitmap[2] + zip) = (UBYTE)colval;
      break;
      } /* merge */
     } /* switch */
    if (((render & 0x11) == 0x11) && Operation)
     {
     ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
     } /* if */
    } /* if backround */
   } /* for x=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   }
  BusyWin_Update(BWDE, y + 1);
  } /* for y=0... */

 if (BWDE) BusyWin_Del(BWDE);
 BWDE = NULL;

 if (error) goto EndMerge;

 if (! settings.background)
  {
  makesky(renderseg, win);
  }

EndMerge:
 if (BWDE) BusyWin_Del(BWDE);
 if (fhZ > 0) close (fhZ);
 if (fhBR > 0) close (fhBR);
 if (fhBG > 0) close (fhBG);
 if (fhBB > 0) close (fhBB);
 if (BkGrnd[0]) free_Memory(BkGrnd[0], BkGrndSize);
 if (BkGrnd[1]) free_Memory(BkGrnd[1], BkGrndSize);
 if (BkGrnd[2]) free_Memory(BkGrnd[2], BkGrndSize);
 if (ZBufAlt) free_Memory(ZBufAlt, ZBufSize);

 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error loading Z Buffer!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 2:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Out of memory merging Z Buffer!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 3:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error opening Z Buffer file for input!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 4:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Z Buffer file!\nNot single precision floating point.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 5:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Z Buffer file!\nNo ZBOD chunk.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 6:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Z Buffer file!\nNo ZBUF chunk.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 7:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Z Buffer file!\nWrong Size.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 8:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error loading background image!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 9:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Out of memory merging background!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 10:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error opening Background file for input!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 11:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Background file!\nWrong Size.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 12:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Background file!\nNo BODY Chunk.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 13:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Background file!\nNo BMHD Chunk.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 14:
   {
   User_Message((CONST_STRPTR)"Render Module",
           (CONST_STRPTR)"Error reading Background file!\nCompression error.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  } /* switch */

 return (error);

} /* MergeZBufBack() */

/***********************************************************************/

short InterlaceFields(char *Name, short Width, short Height, short Dominance)
{
 short namelen, color, error = 0;
 long y, fh;
 UBYTE *RowPtr;

 namelen = strlen(Name);
 strcat(Name, ".red");

 for (color=0; color<3; color++)
  {
  if ((fh = open(Name, O_RDONLY, 0)) == -1)
   {
   error = 1;
   break;
   } /* if open fail */

  if (lseek(fh, 0L, 2) != Width * Height)
   {
   close (fh);
   error = 2;
   break;
   } /* if wrong size */

  lseek(fh, (long)(Dominance * Width), 0);
  RowPtr = bitmap[color] + Dominance * Width;	/* added Dominance 7/23/95 */
  for (y=0; y<Height; y+=2)
   {
   if ((read(fh, RowPtr, Width)) != Width)
    {
    error = 2;
    break;
    } /* if read error */
   lseek(fh, (long)Width, 1);
   RowPtr += (Width * 2);
   } /* for y=0... */

  close (fh);
  if (error) break;

  remove (Name);
  Name[namelen] = 0;
  if (color == 0)
   {
   strcat(Name, ".grn");
   }
  else if (color == 1)
   {
   strcat(Name, ".blu");
   }
  } /* for color=0... */

 return (error);

} /* InterlaceFields() */
