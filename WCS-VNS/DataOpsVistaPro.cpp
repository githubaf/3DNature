// DataOpsVistaPro.cpp
// VistaPro data code
// Code leeched from DEMConvertGUI.cpp on 10/11/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"
#include "Requester.h"

//extern WCSApp *GlobalApp;

/***

  The VistaPro DEM format is a variant on the Amiga ByteRun1 compression.
  The elevations are stored in the form of deltas from a base elevation.
  If the delta is too big, a new base elevation is set.
  The actual elevation data starts at byte #2048.  The first 2K of the file
  stores all of Vista's settings.
  The southern raster is the first row read

 ***/

/*===========================================================================*/

short ImportWizGUI::LoadVistaDEM(char *filename, short *Output, char TestOnly)
{
double HiLat, HiLon;
float testmax = -FLT_MIN, testmin = FLT_MAX;
FILE *fDEM;
long Compression, DataPts, HdrType, i, j, ndx, size1, size2, tmpndx;
USHORT count;
short error = 0, elev;
short *DataPtr;
unsigned char id[32], name[32];
SBYTE  *buffy = NULL, *tmp = NULL, delta;

if ((fDEM = PROJ_fopen(filename, "rb")) == NULL)
	{
	return (2);
	} // if file open error

if (TestOnly)
	{
	if (fread(id, 1, 32, fDEM) != 32)
		{
		error = 6;	// error reading source file
		goto Cleanup;
		}
	id[31] = 0;
	if (strcmp((char *)id, "Vista DEM File"))
		{
		UserMessageOK("Import Wizard",
			"Warning\nFile is not a VistaPro DEM file.");
		error = 50;
		goto Cleanup;
		} // if no file id match
	if (fread(name, 1, 32, fDEM) != 32)
		{
		error = 6;	// error reading source file
		goto Cleanup;
		}
	name[31] = 0;	// terminate the name string
	fseek(fDEM, 128, SEEK_SET);
	if ((fread(&Compression, 1, 4, fDEM) != 4) || (fread(&HdrType, 1, 4, fDEM) != 4) ||
		(fread(&size1, 1, 4, fDEM) != 4) || (fread(&size2, 1, 4, fDEM) != 4) || (size1 != size2))
		{
		error = 6;
		goto Cleanup;
		}

	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(Compression, &Compression);
	SimpleEndianFlip32S(HdrType, &HdrType);
	SimpleEndianFlip32S(size1, &size1);
	SimpleEndianFlip32S(size2, &size2);
	#endif // BYTEORDER_LITTLEENDIAN

	if (! Compression)
		{
		UserMessageOK("Import Wizard:",
			"Warning\nFile is not recognised as a compressed Vista file and cannot be imported.");
		error = 1;
		goto Cleanup;
		}
	//lint -save -e527
	if (size1 == 0)
		size1 = size2 = 258;	// Fix to handle files Adam found - presumably version 1 format
	switch (size1)
		{
		default:
			UserMessageOK("Import Wizard:",
				"Invalid size found");
			error = 50;
			goto Cleanup;
			break;
		case 258:	// Small
			HiLat = 0.069428;
			HiLon = 180.069428;
			break;
		case 514:	// Large
			HiLat = 0.138856;
			HiLon = 180.138856;
			break;
		case 1026:	// Huge
			HiLat = 0.276901;
			HiLon = 180.276901;
			break;
		case 2050:	// Mega
			HiLat = 0.553802;
			HiLon = 180.553802;
			break;
		} // switch
	//lint -restore
	DataPts = size1;
	if (name[0])	// if there's a name stored, use it
		strcpy(OUTPUT_NAMEBASE, (char *)name);
	INPUT_ROWS = DataPts;
	INPUT_COLS = DataPts;
	OUTPUT_ROWS = DataPts;
	OUTPUT_COLS = DataPts;
	OUTPUT_HILAT = HiLat;
	OUTPUT_LOLAT = 0.0f;
	OUTPUT_HILON = HiLon;
	OUTPUT_LOLON = 180.0f;
//#ifdef WCS_BUILD_VNS
	OUTPUT_HILON = - OUTPUT_HILON;
	OUTPUT_LOLON = - OUTPUT_LOLON;
//#endif // WCS_BUILD_VNS
	INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT; 
	INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT; 
	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO; 
	READ_ORDER = DEM_DATA_READORDER_ROWS; 
	ROWS_EQUAL = DEM_DATA_ROW_LAT; 
	GRID_UNITS = DEM_DATA_UNITS_METERS;
	ELEV_UNITS = DEM_DATA_UNITS_METERS;
	Importing->AllowPosE = TRUE;
	Importing->AskNull = FALSE;
	return 0;
	}

fseek(fDEM, 128, SEEK_SET);
if ((fread((char *)&Compression, 4, 1, fDEM) != 1) || (fread((char *)&HdrType, 4, 1, fDEM) != 1))
	{
	error = 6;
	goto Cleanup;
	}

#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32S(Compression, &Compression);
SimpleEndianFlip32S(HdrType, &HdrType);
#endif // BYTEORDER_LITTLEENDIAN

if (! Compression)
	{
	error = 7;
	goto Cleanup;
	}
buffy = (SBYTE *)AppMem_Alloc((unsigned int)Importing->InCols * 2, APPMEM_CLEAR);
tmp = (SBYTE *)AppMem_Alloc((unsigned int)Importing->InCols * 2, APPMEM_CLEAR);
if (! buffy || ! tmp)
	{
	error = 1;
	goto Cleanup;
	}

DataPtr = &Output[(Importing->InRows - 1) * Importing->InCols];

fseek(fDEM, 2048, SEEK_SET);
for (i = 0; i < Importing->InRows; i++)
	{
	if ((fread(&count, 2, 1, fDEM)) != 1)
		{
		error = 6;
		goto Cleanup;
		}
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(count, &count);
	#endif // BYTEORDER_LITTLEENDIAN
	if ((fread(buffy, count, 1, fDEM)) != 1)
		{
		error = 6;
		goto Cleanup;
		}
	ndx = 0;
	tmpndx = 0;
	do
		{
		if (buffy[ndx] <= 0)	// replicate code
			{
			delta = buffy[ndx + 1];	// value to replicate
			for (j = 0; j < (1 - buffy[ndx]); j++)	// replicate (-code + 1) times
				tmp[tmpndx++] = delta;
			ndx += 2;	// advance to next code
			} // if
		else					// literal run
			{
			for (j = 0; j < (buffy[ndx] + 1); j++)	// copy (code + 1) bytes
				{
				delta = buffy[ndx + j + 1];
				tmp[tmpndx++] = delta;
				} // for j
			ndx += buffy[ndx] + 2;	// advance to next set
			} // else
		} while (ndx < count);
	ndx = 0;
	elev = (char)tmp[0] * 256 + (UBYTE)tmp[1];
	DataPtr[ndx++] = elev;
	if (elev < testmin)
		testmin = elev;
	if (elev > testmax)
		testmax = elev;
	for (j = 2; ndx < Importing->InRows; j++)
		{
		delta = tmp[j];
		if (delta == -128)	// new base elevation
			{
			elev = (char)tmp[j + 1] * 256 + (UBYTE)tmp[j + 2];
			j += 2;
			} // if
		else
			elev += delta;
		DataPtr[ndx++] = elev;
		if (elev < testmin)
			testmin = elev;
		if (elev > testmax)
			testmax = elev;
		} // for j
	DataPtr -= Importing->InRows;
	} // for i

if (i < Importing->InRows)
	error = 6;

Importing->TestMax = testmax;
Importing->TestMin = testmin;

Cleanup:

fclose(fDEM);
if (tmp)
	AppMem_Free(tmp, (unsigned int)Importing->InCols * 2);
if (buffy)
	AppMem_Free(buffy, (unsigned int)Importing->InCols * 2);

return (error);

} // LoadVistaDEM()
