// DataOpsDTED.cpp
// Code for DTED files
// Adapted from DEMConvertGUI.cpp on 12/10/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Useful.h"
#include "Project.h"

short ImportWizGUI::LoadDTED(char *filename, short *Output, char TestOnly)
{
char Sentinel[16], test[16];
double Coord;
short error = 0, *DataPtr, PtCt[2];
long row, col, Rows, Cols, ColSize, StartPt = 0;
FILE *fDEM;
size_t len;

Importing->AllowRef00 = FALSE;
Importing->AllowPosE = FALSE;
Importing->AskNull = FALSE;
Importing->HasNulls = TRUE;
Importing->NullVal = -32767;
INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT;		// value size
INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT; // value format
INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;		// value byte order
READ_ORDER = DEM_DATA_READORDER_ROWS;		// read order
ROWS_EQUAL = DEM_DATA_ROW_LAT;				// rows equal
ELEV_UNITS = DEM_DATA_UNITS_METERS;			// data units meters

if ((fDEM = PROJ_fopen(filename, "rb")) == NULL)
	{
	return (2);
	} // if file open error

fseek(fDEM, 160, SEEK_SET);
if (fread((char *)Sentinel, 3, 1, fDEM) != 1)
	return 6;	// read error
Sentinel[3] = 0;
if (strcmp(Sentinel, "DSI"))
	{
	StartPt = -30000;
	fseek(fDEM, 0L, SEEK_SET);
	for (col = 0; col < 20000; col ++)
		{
		if (fread((char *)Sentinel, 1, 1, fDEM) != 1)
			return 6;	// read error
		if (Sentinel[0] == 'D')
			{
			if (fread((char *)Sentinel, 1, 1, fDEM) != 1)
				return 6;	// read error
			if (Sentinel[0] == 'S')
				{
				if (fread((char *)Sentinel, 1, 1, fDEM) != 1)
					return 6;	// read error
				if (Sentinel[0] == 'I')
					{
					StartPt = ftell(fDEM) - 163;
					break;
					} // if
				} // if
			} // if
		} // for col
	} // if
if (StartPt <= -30000)
	{
	error = 15;
	goto EndLoad;
	} // if can't find DSI record

fseek(fDEM, StartPt + 364, SEEK_SET);
if (fread(test, 1, 7, fDEM) != 7)
	return 6;	// read error
if (TestOnly)	// if users changes values, use theirs instead
	{
	test[7] = 0;
	Coord = DegMinSecToDegrees(test);
	if (test[6] == 'S')
		{
		Coord = -Coord;
		}
	OUTPUT_LOLAT = Coord;
	}

if (fread(test, 1, 8, fDEM) != 8)
	return 6;	// read error
if (TestOnly)
	{
	test[8] = 0;
	Coord = DegMinSecToDegrees(test);
//	#ifdef WCS_BUILD_VNS
	if (test[7] == 'W')
//	#else
//	if (test[7] == 'E')
//	#endif // WCS_BUILD_VNS
		{
		Coord = -Coord;
		}
	OUTPUT_HILON = Coord;
	}

if (fread(test, 1, 7, fDEM) != 7)
	return 6;	// read error
if (TestOnly)
	{
	test[7] = 0;
	Coord = DegMinSecToDegrees(test);
	if (test[6] == 'S')
		{
		Coord = -Coord;
		}
	OUTPUT_HILAT = Coord;
	}

fseek(fDEM, StartPt + 401, SEEK_SET);
if (fread(test, 1, 8, fDEM) != 8)
	return 6;	// read error
if (TestOnly)
	{
	test[8] = 0;
	Coord = DegMinSecToDegrees(test);
//	#ifdef WCS_BUILD_VNS
	if (test[7] == 'W')
//	#else // WCS_BUILD_VNS
//	if (test[7] == 'E')
//	#endif // WCS_BUILD_VNS
		{
		Coord = -Coord;
		}
	OUTPUT_LOLON = Coord;
	}

fseek(fDEM, StartPt + 441, SEEK_SET);
if (fread((char *)Sentinel, 4, 1, fDEM) != 1)
	return 6;	// read error
Sentinel[4] = 0;
Rows = atoi(Sentinel);
//Importing->InRows = Rows;

if (fread((char *)Sentinel, 4, 1, fDEM) != 1)
	return 6;	// read error
Cols = atoi(Sentinel);
ColSize = Rows * sizeof (short);
//Importing->InCols = Cols;
Importing->InRows = Cols;
Importing->InCols = Rows;

#ifdef WCS_BUILD_VNS
Importing->IWCoordSys.SetSystem("Geographic - WGS 84");
Importing->CoordSysWarn = FALSE;
Importing->BoundsType = VNS_BOUNDSTYPE_DEGREES,
Importing->BoundTypeLocked = TRUE;
#endif

if (TestOnly)
	{
	if (strlen(Importing->NameBase) == 3)	// probably ndd or sdd then
		{
		len = strlen(Importing->InDir);
		if (len > 6)
			{
			// see if it's in a 'eddd' or 'wddd' directory then
			if ((Importing->InDir[len - 6] == '\\') && ((Importing->InDir[len - 5] == 'e') || (Importing->InDir[len - 5] == 'w')) &&
				(Importing->InDir[len - 4] >= '0') && (Importing->InDir[len - 4] <= '9') &&
				(Importing->InDir[len - 3] >= '0') && (Importing->InDir[len - 3] <= '9') &&
				(Importing->InDir[len - 2] >= '0') && (Importing->InDir[len - 2] <= '9'))
				{
				strncpy(Importing->OutName, &Importing->InDir[len - 5], 4);
				Importing->OutName[4] = 0;
				strcat(Importing->OutName, Importing->NameBase);
				strcpy(Importing->NameBase, Importing->OutName);
				}
			}
		}
	return 0;
	}

fseek(fDEM, StartPt + 3508, SEEK_SET);
DataPtr = Output;

for (col = 0; col < Cols; col ++)
	{
	if ((fread((char *)Sentinel, 4, 1, fDEM) != 1) || (fread((char *)&PtCt[0], 4, 1, fDEM) != 1))
		return 6;	// read error
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16S(PtCt[0], &PtCt[0]);
	#endif // BYTEORDER_LITTLEENDIAN
	if (PtCt[0] != col)
		{
		error = 6;	// read error
		break;
		}
	if ((fread((char *)DataPtr, ColSize, 1, fDEM) != 1) || (fread((char *)Sentinel, 4, 1, fDEM) != 1))
		return 6;	// read error
	for (row = 0; row < Rows; row ++)
		{
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16S(DataPtr[row], &DataPtr[row]);
		#endif // BYTEORDER_LITTLEENDIAN
		// numbers are stored as 15 bit signed (ones complement), so change to twos complement if negative
		if (DataPtr[row] & 0x8000)
			DataPtr[row] = -(DataPtr[row] & 0x7fff);
		} // for row
	DataPtr += Rows;
	} // for col

EndLoad:
fclose(fDEM);

return (error);

} // ImportWizGUI::LoadDTED()
