// DataOpsImage.cpp
// Image heightfield data code
// Created on 11/12/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"
#include "Requester.h"
#include "ImageInputFormat.h"
#include "ImageFormatIFF.h"
#include "ImageFormatTIFFs.h"
#include "Raster.h"

unsigned long MaxHite8, MaxHiteAdd, MaxHite16, MaxHite24;
unsigned long MinHite8, MinHiteAdd, MinHite16, MinHite24;

short ImportWizGUI::LoadDEMImage(char *filename, unsigned long *Output, char TestOnly, unsigned char valueFmt)
{
#ifdef WCS_BUILD_VNS
GeoRefShell *GRS;
#endif // WCS_BUILD_VNS
unsigned long hite8, hiteadd, hite16, hite24, *hitefield;
long i, j, zippy = 0;
Raster LoadRas;
short error = 1, method = 0;

// F2NOTE: 080501 - Not sure when/why next line was changed to use value passed to function.  Causes nasty crash though :)
Importing->ValueFmt = DEM_DATA_FORMAT_UNSIGNEDINT;
//Importing->ValueFmt = valueFmt;
Importing->ValueBytes = 4;
Importing->AllowRef00 = FALSE;
Importing->AllowPosE = FALSE;
Importing->AskNull = FALSE;

hiteadd = hite16 = hite24 = 0;
MaxHite8 = MaxHiteAdd = MaxHite16 = MaxHite24 = 0;
MinHite8 = MinHiteAdd = MinHite16 = MinHite24 = ULONG_MAX;

hitefield = Output;
//if (LoadRasterImage(filename, 1, Bitmap, 0, 0, 0, &NewWidth, &NewHeight, &NewPlanes))
if (LoadRasterImage(filename, &LoadRas, 0))
	{
#ifdef WCS_BUILD_VNS
	if (GRS = LoadRas.LoaderGeoRefShell)
		{
		Importing->IWCoordSys.Copy(&Importing->IWCoordSys, (CoordSys *)GRS->GetHost());
		Importing->NBound = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetCurValue(0);
		Importing->SBound = GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetCurValue(0);
		Importing->WBound = -GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetCurValue(0);
		Importing->EBound = -GRS->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetCurValue(0);
		} // if GRS
#endif // WCS_BUILD_VNS
	Importing->InCols = LoadRas.Cols;
	Importing->InRows = LoadRas.Rows;
	Importing->Bands = LoadRas.ByteBands;
	if ((Importing->Bands == 3) && (Importing->Flags == ELEV_METHOD_ADD))
		method = 1;
	else if ((Importing->Bands == 3) && (Importing->Flags == ELEV_METHOD_16BIT))
		method = 2;
	else if ((Importing->Bands == 3) && (Importing->Flags == ELEV_METHOD_24BIT))
		method = 3;
	for (i = 0; i < LoadRas.Rows; i++)
		for (j = 0; j < LoadRas.Cols; j++)
			{
			// 8 bit
			hite8 = LoadRas.ByteMap[0][zippy];
			if (Importing->Bands == 3)
				{
				// 24 bit add - R + G + B
				hiteadd = LoadRas.ByteMap[0][zippy] + LoadRas.ByteMap[1][zippy] + LoadRas.ByteMap[2][zippy];
				// 24 bit POV - RG as unsigned 16 bit
				hite16 = LoadRas.ByteMap[0][zippy] * 256 + LoadRas.ByteMap[1][zippy];
				// 24 bit elev - RGB as 24 bit (TruFlite)
				hite24 = LoadRas.ByteMap[0][zippy] * 65536 + LoadRas.ByteMap[1][zippy] * 256 + LoadRas.ByteMap[2][zippy];
				}

			if (TestOnly)
				{
				if (hite8 > MaxHite8)
					MaxHite8 = hite8;
				if (hite8 < MinHite8)
					MinHite8 = hite8;

				if (Importing->Bands == 3)
					{
					if (hiteadd > MaxHiteAdd)
						MaxHiteAdd = hiteadd;
					if (hiteadd < MinHiteAdd)
						MinHiteAdd = hiteadd;

					if (hite16 > MaxHite16)
						MaxHite16 = hite16;
					if (hite16 < MinHite16)
						MinHite16 = hite16;

					if (hite24 > MaxHite24)
						MaxHite24 = hite24;
					if (hite24 < MinHite24)
						MinHite24 = hite24;
					}
				} // if TestOnly
			else
				{
				switch (method)
					{
					default:
					case 0:
						*hitefield++ = hite8;
						break;
					case 1:
						*hitefield++ = hiteadd;
						break;
					case 2:
						*hitefield++ = hite16;
						break;
					case 3:
						*hitefield++ = hite24;
						break;
					} // switch
				} // else
			zippy++;
			} // for
	error = 0;
	} // if
else
	UserMessageOK("Import Wizard: Load Image DEM",
		"Error reading image file!\nOperation terminated.");

// Raster destructor does this for us.
//if (LoadRas.ByteMap[0])
//	AppMem_Free(LoadRas.ByteMap[0], (size_t)(LoadRas.Cols * LoadRas.Rows));
//if (LoadRas.ByteMap[1])
//	AppMem_Free(LoadRas.ByteMap[1], (size_t)(LoadRas.Cols * LoadRas.Rows));
//if (LoadRas.ByteMap[2])
//	AppMem_Free(LoadRas.ByteMap[2], (size_t)(LoadRas.Cols * LoadRas.Rows));


if ((TestOnly) && (Importing->Bands == 1))
	{
	Importing->TestMax = MaxHite8;
	Importing->TestMin = MinHite8;
	}

return error;

} // ImportWizGUI::LoadDEMImage

/*===========================================================================*/

short ImportWizGUI::LoadWCS_ZBuffer(char *filename, char *Output, char TestOnly)
{
struct ILBMHeader Hdr;
struct ZBufferHeader ZBHdr;
FILE *fZ = NULL;
long InputDataSize, Ptr = 0L;
short error = 0;

Importing->AllowPosE = FALSE;
Importing->AskNull = FALSE;

if ((fZ = PROJ_fopen(filename, "rb")) == NULL)
	{
	return 2;	// can't open file
	}

if (CheckIFF(fZ, &Hdr))
	{
	if (FindIFFChunk(fZ, &Hdr, "ZBUF"))
		{
		if (fread(&ZBHdr, 1, sizeof (struct ZBufferHeader), fZ) == sizeof (struct ZBufferHeader))
			{
			if (FindIFFChunk(fZ, &Hdr, "ZBOD"))
				{
				Ptr = ftell(fZ);
				} // if
			} // if
		} // if
	} // if
if (Ptr)
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

	INPUT_HEADER = Ptr;
	INPUT_ROWS = (long)ZBHdr.Height;
	INPUT_COLS = (long)ZBHdr.Width;
	switch (ZBHdr.VarType)
		{
		default: break;
		case 0:	// BYTE
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_BYTE;
			INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT;
			break;
			}
		case 1:	// UBYTE
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_BYTE;
			INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
			break;
			}
		case 2:	// SHORT
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT;
			INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT;
			break;
			}
		case 3:	// USHORT
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT;
			INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
			break;
			}
		case 4:	// LONG
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
			INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT;
			break;
			}
		case 5:	// ULONG
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
			INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
			break;
			}
		case 6:	// FLOAT
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
			INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
			break;
			}
		case 7:	// DOUBLE
			{
			INVALUE_SIZE = DEM_DATA_VALSIZE_DOUBLE;
			INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
			break;
			}
		} // switch

	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
	READ_ORDER = DEM_DATA_READORDER_ROWS;
	switch (ZBHdr.Units)
		{
		case 0:
			{
			ELEV_UNITS = DEM_DATA_UNITS_OTHER;
			break;
			} // dimensionless
		case 1:
			{
			ELEV_UNITS = DEM_DATA_UNITS_MILLIM;
			break;
			} // millimeters
		default:
		case 2:
			{
			ELEV_UNITS = DEM_DATA_UNITS_METERS;
			break;
			} // meters
		case 3:
			{
			ELEV_UNITS = DEM_DATA_UNITS_KILOM;
			break;
			} // kilometers
		case 4:
			{
			ELEV_UNITS = DEM_DATA_UNITS_INCHES;
			break;
			} // inches
		case 5:
			{
			ELEV_UNITS = DEM_DATA_UNITS_FEET;
			break;
			} // feet
		case 6:
			{
			ELEV_UNITS = DEM_DATA_UNITS_YARDS;
			break;
			} // yards
		case 7:
			{
			ELEV_UNITS = DEM_DATA_UNITS_MILES;
			break;
			} // miles
		case 8:
			{
			ELEV_UNITS = DEM_DATA_UNITS_OTHER;
			break;
			} // light years
		case 9:
			{
			ELEV_UNITS = DEM_DATA_UNITS_OTHER;
			break;
			} // undefined
		} // switch

	OUTVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
	OUTVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
	Importing->TestMin = ZBHdr.Min;
	Importing->TestMax = ZBHdr.Max;
	} // if
else
	{
	error = 19;	// not a WCS ZBuffer
	goto zback;
	}

if (! TestOnly)
	{
	InputDataSize = INPUT_ROWS * INPUT_COLS * INVALUE_SIZE;
	if (! Importing->WrapData)
		{
		if ((fread(Output, 1, InputDataSize, fZ)) != (unsigned)InputDataSize)
			{
			error = 6;	// read file error
			goto zback;
			} // if read fail
		} // if no wrap longitude
	else
		{
		ULONG Source, Dest, InputRowSize, FullRowSize;
		long i;

		if ((fread(Output, 1, (InputDataSize - INPUT_ROWS * INVALUE_SIZE), fZ))
			!= (unsigned)(InputDataSize - INPUT_ROWS * INVALUE_SIZE))
			{
			error = 6;	// read file error
			goto zback;
			} // if read fail
		InputRowSize = (INPUT_COLS - 1) * INVALUE_SIZE;
		FullRowSize = InputRowSize + INVALUE_SIZE;
		for (i = INPUT_ROWS - 1; i >= 0; i--)
			{
			Source = (ULONG)Output + i * InputRowSize;
			Dest = (ULONG)Output + i * FullRowSize;
			if (i > 0)
				memmove((char *)Dest, (char *)Source, InputRowSize);
			memcpy((char *)(Dest + InputRowSize), (char *)Dest, INVALUE_SIZE);
			} // for i
		} // else wrap longitude
//	if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_COLORBMP)
//		strcat(Importing->NameBase, ".bmp");
//	else if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_COLORIFF)
//		strcat(Importing->NameBase, ".iff");
//	else if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_COLORTGA)
//		strcat(Importing->NameBase, ".tga");
//	else if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_GRAYIFF)
//		strcat(Importing->NameBase, ".iff");
	}

zback:
if (fZ)
	fclose(fZ);

return error;

} // ImportWizGUI::LoadWCS_ZBuffer

/*===========================================================================*/

bool ImportWizGUI::IsTIFFDEM(char *filename)
{
TIFF *tif;
uint16 bitsPerSample, sampleFormat, samplesPerPixel;
bool status = false;

if (tif = XTIFFOpen(GlobalApp->MainProj->MungPath(filename), "rC"))
	{
	if (TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample) &&
		TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat) &&
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel))
		status = true;
	if (status && (bitsPerSample != 32) && (sampleFormat != 3) && (samplesPerPixel != 1))		// 32 bit floating point (USGS SDDS)
		status = false;
	else if (status && (bitsPerSample != 16) && (sampleFormat != 2) && (samplesPerPixel != 1))	// 16 bit signed (CGIAR-CSI)
		status = false;
	else if (status && (bitsPerSample != 16) && (sampleFormat != 1) && (samplesPerPixel != 1))	// 16 bit unsigned
		status = false;
	XTIFFClose(tif);
	} // if

return status;

} // ImportWizGUI::IsTIFFDEM

/*===========================================================================*/

short ImportWizGUI::LoadTIFFDEM(char *filename, float *output, char testOnly)
{
ArcWorldInfo awi;
TIFF *tif;
GTIF *gtif;
GeoRefShell *grs = NULL;
char *dem = (char *)output;
int gtifValid = 0, tfwValid = 0;
short status = 0;

if (tif = XTIFFOpen(GlobalApp->MainProj->MungPath(filename), "rC"))
	{
	uint32* bc;
	tdata_t buf;
	tstrip_t strip;
	uint32 iLength, iWidth, rowsPerStrip, stripsize;
	uint16 bitsPerSample, sampleFormat, samplesPerPixel;

	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);
	TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &bc);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &iLength);
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &iWidth);

	if (gtif = GTIFNew(tif))
		gtifValid = IsValidGeoTIFF(gtif);

	if (tfwValid || gtifValid)
		grs = ReadTIFFGeoref(awi, NULL, gtif, iWidth, iLength, 0, tfwValid, gtifValid);

	Importing->InCols = iWidth;
	Importing->InRows = iLength;
	Importing->InFormat = DEM_DATA2_INPUT_IFF;
	if ((bitsPerSample == 32) && (sampleFormat == 3))
		{
		Importing->ValueBytes = 4;
		Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
		} // if
	else if ((bitsPerSample == 16) && (sampleFormat == 2))
		{
		Importing->ValueBytes = 2;
		Importing->ValueFmt = DEM_DATA_FORMAT_SIGNEDINT;
		} // else if
	else if ((bitsPerSample == 16) && (sampleFormat == 1))
		{
		Importing->ValueBytes = 2;
		Importing->ValueFmt = DEM_DATA_FORMAT_UNSIGNEDINT;
		} // else if
	if (grs)
		{
		Importing->NBound = grs->GeoReg.AnimPar[0].CurValue;
		Importing->SBound = grs->GeoReg.AnimPar[1].CurValue;
		if (1)
			{
			Importing->WBound = -grs->GeoReg.AnimPar[2].CurValue;
			Importing->EBound = -grs->GeoReg.AnimPar[3].CurValue;
			} // if
		else
			{
			} // else
		Importing->IWCoordSys.Copy(&Importing->IWCoordSys, (CoordSys *)grs->Host);
		Importing->CoordSysWarn = 0;
		//Importing->express = 1;
		delete grs;
		} // if

	if (! testOnly)
		{
		stripsize = bc[0];
		buf = _TIFFmalloc(stripsize);
		for (strip = 0; strip < TIFFNumberOfStrips(tif); strip++)
			{
			if (bc[strip] > stripsize)
				{
				buf = _TIFFrealloc(buf, bc[strip]);
				stripsize = bc[strip];
				} // if
			TIFFReadRawStrip(tif, strip, buf, bc[strip]);
			memcpy((void *)dem, buf, bc[strip]);
			dem += stripsize;
			} // for
		//status = 0;
		_TIFFfree(buf);
		} // if
	GTIFFree(gtif);
	XTIFFClose(tif);

	} // if

return status;

} // ImportWizGUI::LoadTIFF32F
