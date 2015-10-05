// DataOpsBryce.cpp
// Bryce data code
// Code leeched from DEMConvertGUI.cpp on 10/06/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"
#include "Requester.h"
#include "EffectsLib.h"

#ifdef DEBUG // don't let this into release!
//#define RAWBRYCE
#endif

short ImportWizGUI::LoadBryceDEM(char *filename, unsigned short *Output, char TestOnly)
{
// code adapted by F2 from code supplied by Joe Slayton (jslayton@ridgenet.net), author of WILBUR
static UBYTE Plane1[] = { 'P', 'l', 'a', 'n', 'e', ' ', '1' };
static UBYTE esab[] = { 'e', 's', 'a', 'b' };
static UBYTE sig1[4] = { 0xF0, 0x13, 0x2A, 0x01 };	// must keep these sigs
static UBYTE sig2[4] = { 0x18, 0x66, 0xFC, 0x00 };	// the same size!
static UBYTE sig3[4] = { 0xC0, 0xA9, 0xFB, 0x00 };
static UBYTE sig4[4] = { 0xE8, 0x73, 0xE, 0x00 };
//static UBYTE b4sig1[4] = { 0x58, 0xFA, 0x54, 0x01 };
static UBYTE MacID[] = { '3', 'd', 'o', 'b', 'h', 'f', 'l', 'd' };	// these ID's need to be the same size
static UBYTE PC_ID[] = { 'b', 'o', 'd', '3', 'd', 'l', 'f', 'h' };
static UBYTE base[] = { 'b', 'a', 's', 'e' };

double	baselat, baselong;
UBYTE	*buffy;	// my trademark :)
UBYTE	*bite;
char	*cptr;
long	DataOfs;	// data offset
double	delta;
long	DEMstart = 0;
long	filesize;
long	fro;
BOOL	Found1, Found2;
UBYTE	i;
FILE	*input;
BOOL	known = FALSE;
BOOL	MacFile = FALSE;
long	MaxOfs;	// maximum file offset
void	*s;
long	Size = 0;		// DEM size
long	SizeVal = 0;	// where the size value is at

#ifdef RAWBRYCE
FILE	*tempout;
#endif

USHORT	tmp[4096];	// maximum Bryce DEM size
char	tstr[5];
long	to;
USHORT  value, *vptr;
ULONG	value32;
UBYTE	witch = 0;	// which Bryce version
long	y;			// because...

if ((input = PROJ_fopen(filename, "rb")) == NULL)
	{
	UserMessageOK("Import Wizard",
		"Warning\nCan't open file!");
	return(1);
	}

fseek(input, 0L, SEEK_END);
filesize = ftell(input);
rewind(input);

// we're going to read the whole file into memory
if (!(buffy = (UBYTE *)AppMem_Alloc(filesize, 0L)))
	{
	UserMessageOK("Import Wizard", "Warning\nUnable to allocate memory for file.");
	return(1);
	}

if ((unsigned long)filesize != fread(buffy, 1, filesize, input))
	{
	UserMessageOK("Import Wizard", "Warning\nAn error has occurred while reading.");
	return(1);
	}

if ((memcmp(buffy, "CCmF - Universal", 16) != 0)	// pre-Corel Bryce files have this
	&& (memcmp(&buffy[94], "CCmFile::kIdentify4", 19) != 0))	// Corel Bryce 5
	{
	UserMessageOK("Import Wizard",	"Warning\nFile isn't a Bryce format");
	return(99);
	}

// don't search past EOF
MaxOfs = filesize - (long)(sizeof(Plane1) + sizeof(MacID));

DataOfs = 16;	// skip the CCmF string

// Figure out if the file originated on a Mac or a PC.
// See if it looks like a Mac Bryce file here.
while (! known)
	{
	s = memchr(&buffy[DataOfs], MacID[0], MaxOfs - DataOfs);
	if (s != NULL)
		{
		DataOfs = (int)s - int(&buffy[0]);
		if (memcmp(&buffy[DataOfs], MacID, sizeof(Plane1)) == 0)
			{
			MacFile = TRUE;
			known = TRUE;
			break;
			}
		DataOfs++;
		} // if
	else
		{
		DataOfs = 16;	// not found - reset back to start
		break;
		}
	} // while
// If it wasn't a Mac file, see if it looks like a PC Bryce file
if (! known)
	{
	while (! known)
		{
		s = memchr(&buffy[DataOfs], PC_ID[0], MaxOfs - DataOfs);
		if (s != NULL)
			{
			DataOfs = (int)s - int(&buffy[0]);
			if (memcmp(&buffy[DataOfs], PC_ID, sizeof(Plane1)) == 0)
				{
				known = TRUE;
				break;
				}
			DataOfs++;
			} // if
		else
			{
			DataOfs = 16;	// not found - reset back to start
			break;
			}
		} // while
	} // if
// If it's still unknown, bail
if (! known)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Import Wizard - can't determine byte order");
	return 99;	// error, but don't emit another message
	}
	
if (MacFile)
	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
else
	INBYTE_ORDER = DEM_DATA_BYTEORDER_LOHI;

if (MacFile)
	{
	// test for Bryce 5
	if (memcmp(&buffy[94], "CCmFile::kIdentify4", 19) == 0)
		witch = 5;

	// I can't tell the diff between B4 & B3D yet, Macs may not set file extension, so set default to 4
	witch = 4;	// continue processing to make sure it looks good

	// Ok, here's what we're gonna do: look for "Plane 1" then "esab", if that fails, look for
	// "esab" + a marker, and if that fails, look for "3dobhlfd"
	Found1 = Found2 = FALSE;
	// look for the "PLANE 1" string
	DataOfs = 16;	// skip the CCmF string
	while (!Found1 && (DataOfs < MaxOfs))
		{
		s = memchr(&buffy[DataOfs], Plane1[0], MaxOfs - DataOfs);
		if (s != NULL)
			{
			DataOfs = (int)s - int(&buffy[0]);
			if (memcmp(&buffy[DataOfs], Plane1, sizeof(Plane1)) == 0)
				{
				Found1 = TRUE;
				DataOfs += 7;
				break;
				}
			DataOfs++;
			} // if
		else
			{
			DataOfs = 16;	// not found - reset back to start
			break;
			}
		} // while

	// look for the "base" string
	while (!Found2 && (DataOfs < MaxOfs))
		{
		s = memchr(&buffy[DataOfs], base[0], MaxOfs - DataOfs);
		if (s != NULL)
			{
			DataOfs = (int)s - int(&buffy[0]);
			if (memcmp(&buffy[DataOfs], base, sizeof(esab)) == 0)
				{
				if (Found1)
					{
					if ((witch == 4) || (witch == 5))
						{
						SizeVal = DataOfs + 1524;
						DEMstart = DataOfs + 1528;
						} // if
					else
						{
						SizeVal = DataOfs + 876;
						DEMstart = DataOfs + 880;
						witch = 3;
						}
					}
				Found2 = TRUE;
				break;
				}
			else
				DataOfs++;
			} // if
		else
			{
			DataOfs += 4;	// position for the next search
			break;
			} // else
		} // while

	if (!Found1 && Found2)	// found "base", look for markers
		{
		if ((memcmp(&buffy[DataOfs], sig1, sizeof(sig1)) == 0) ||
			(memcmp(&buffy[DataOfs], sig2, sizeof(sig2)) == 0) ||
			(memcmp(&buffy[DataOfs], sig3, sizeof(sig3)) == 0))
			{
			Found1 = TRUE;
			SizeVal = DataOfs + 872;
			DEMstart = DataOfs + 876;
			}
		else if (memcmp(&buffy[DataOfs], sig4, sizeof(sig4)) == 0)
			{
			Found1 = TRUE;
			SizeVal = DataOfs + 244;	// or something else
			DEMstart = DataOfs + 624;
			} // else if
		} // if

	if (!(Found1 && Found2))	// failed both sets of tests
		{ // Try Bryce 2
		DataOfs = 16;
		Found2 = FALSE;
		while (!Found2 && DataOfs < MaxOfs)
			{
			s = memchr(&buffy[DataOfs], MacID[0], MaxOfs - DataOfs);
			if (s != NULL) // found '3', check rest of string
				{
				DataOfs = (int) s - (int) &buffy[0];
				if (!(Found2 = (memcmp(&buffy[DataOfs], MacID, sizeof(MacID))) == 0))
					DataOfs++;
				} // if
			else
				{
				DataOfs = 0;
				break;
				} // else
			} // while
		Found2 = TRUE;
		SizeVal = DataOfs + 876;
		DEMstart = DataOfs + 4;
		INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;	// always Mac order for V2?
		witch = 2;
		} // else

	switch (witch)	// retrieve the DEM size
		{
		case 5:
		case 4:
		case 3:
			value32 = *((ULONG *)&buffy[SizeVal]);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(value32, &value32);
			#endif
			Size = (long)value32;
			switch (Size)
				{
				case 16: case 32: case 64: case 128: case 256: case 512: case 1024: case 2048: case 4096:
					break;
				default:
					Size = 0;
				} // switch
			break;
		case 2:
			DataOfs += 97;
			bite = (UBYTE *)&buffy[DataOfs];
			switch (*bite)
				{
				case 0x20 : Size =   16; break;
				case 0x30 : Size =   32; break;
				case 0x40 : Size =   64; break;
				case 0x50 : Size =  128; break;
				case 0x60 : Size =  256; break;
				case 0x70 : Size =  512; break;
				case 0x80 : Size = 1024; break;
				default   : Size =    0;
				} // switch
			DEMstart = DataOfs + 720 - 97;	// 720 away from "3dobhlfd"
			break;
		default:
			break;
		}
	} // if MacFile
else
	{
	// test for Bryce 5
	if (memcmp(&buffy[94], "CCmFile::kIdentify4", 19) == 0)
		witch = 5;

	// I can't tell the diff between B4 & B3D yet, so use file extension to hint
	if (cptr = strrchr(filename, '.'))
		{
		strncpy(tstr, cptr, 4);
		tstr[4] = NULL;
		for (i = 0; i < strlen(tstr); i++)
			tstr[i] = (char)tolower(tstr[i]);
		if (!strcmp(tstr, ".br4"))
			witch = 4;	// continue processing to make sure it looks good
		}


	// Ok, here's what we're gonna do: look for "Plane 1" then "esab", if that fails, look for
	// "esab" + a marker, and if that fails, look for "3dobhlfd"
	Found1 = Found2 = FALSE;
	// look for the "PLANE 1" string
	DataOfs = 16;	// skip the CCmF string
	while (!Found1 && (DataOfs < MaxOfs))
		{
		s = memchr(&buffy[DataOfs], Plane1[0], MaxOfs - DataOfs);
		if (s != NULL)
			{
			DataOfs = (int)s - int(&buffy[0]);
			if (memcmp(&buffy[DataOfs], Plane1, sizeof(Plane1)) == 0)
				{
				Found1 = TRUE;
				DataOfs += 7;
				break;
				}
			DataOfs++;
			} // if
		else
			{
			DataOfs = 16;	// not found - reset back to start
			break;
			}
		} // while

	// look for the "esab" string
	while (!Found2 && (DataOfs < MaxOfs))
		{
		s = memchr(&buffy[DataOfs], esab[0], MaxOfs - DataOfs);
		if (s != NULL)
			{
			DataOfs = (int)s - int(&buffy[0]);
			if (memcmp(&buffy[DataOfs], esab, sizeof(esab)) == 0)
				{
				if (Found1)
					{
					if ((witch == 4) || (witch == 5))
						{
						SizeVal = DataOfs + 1524;
						DEMstart = DataOfs + 1528;
						} // if
					else
						{
						SizeVal = DataOfs + 876;
						DEMstart = DataOfs + 880;
						witch = 3;
						}
					}
				Found2 = TRUE;
				break;
				}
			else
				DataOfs++;
			} // if
		else
			{
			DataOfs += 4;	// position for the next search
			break;
			} // else
		} // while

	if (!Found1 && Found2)	// found "esab", look for markers
		{
		if ((memcmp(&buffy[DataOfs], sig1, sizeof(sig1)) == 0) ||
			(memcmp(&buffy[DataOfs], sig2, sizeof(sig2)) == 0) ||
			(memcmp(&buffy[DataOfs], sig3, sizeof(sig3)) == 0))
			{
			Found1 = TRUE;
			SizeVal = DataOfs + 872;
			DEMstart = DataOfs + 876;
			}
		else if (memcmp(&buffy[DataOfs], sig4, sizeof(sig4)) == 0)
			{
			Found1 = TRUE;
			SizeVal = DataOfs + 244;	// or something else
			DEMstart = DataOfs + 624;
			} // else if
		} // if

	if (!(Found1 && Found2))	// failed both sets of tests
		{ // Try Bryce 2
		DataOfs = 16;
		Found2 = FALSE;
		while (!Found2 && DataOfs < MaxOfs)
			{
			s = memchr(&buffy[DataOfs], MacID[0], MaxOfs - DataOfs);
			if (s != NULL) // found '3', check rest of string
				{
				DataOfs = (int) s - (int) &buffy[0];
				if (!(Found2 = (memcmp(&buffy[DataOfs], MacID, sizeof(MacID))) == 0))
					DataOfs++;
				} // if
			else
				{
				DataOfs = 0;
				break;
				} // else
			} // while
		Found2 = TRUE;
		SizeVal = DataOfs + 876;
		DEMstart = DataOfs + 4;
		INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;	// always Mac order for V2?
		witch = 2;
		} // else

	switch (witch)	// retrieve the DEM size
		{
		case 5:
		case 4:
		case 3:
			value32 = *((ULONG *)&buffy[SizeVal]);
			#ifdef BYTEORDER_BIGENDIAN
			SimpleEndianFlip32U(value32, &value32);
			#endif
			Size = (long)value32;
			switch (Size)
				{
				case 16: case 32: case 64: case 128: case 256: case 512: case 1024: case 2048: case 4096:
					break;
				default:
					Size = 0;
				} // switch
			break;
		case 2:
			DataOfs += 97;
			bite = (UBYTE *)&buffy[DataOfs];
			switch (*bite)
				{
				case 0x20 : Size =   16; break;
				case 0x30 : Size =   32; break;
				case 0x40 : Size =   64; break;
				case 0x50 : Size =  128; break;
				case 0x60 : Size =  256; break;
				case 0x70 : Size =  512; break;
				case 0x80 : Size = 1024; break;
				default   : Size =    0;
				} // switch
			DEMstart = DataOfs + 720 - 97;	// 720 away from "3dobhlfd"
			break;
		default:
			break;
		}
	} // else !MacFile


// if bad data, bail
// <<< LINT: Found2 always evaluates to FALSE, may want to check later >>>
if ((!Found2) || (DataOfs >= (filesize - (2 * Size * Size))) || (Size == 0)) //lint !e774
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unrecognized Bryce file - aborting");
	return(99);
	} // if

INPUT_ROWS = Size;
INPUT_COLS = Size;
INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT; 
READ_ORDER = DEM_DATA_READORDER_ROWS; 
ROWS_EQUAL = DEM_DATA_ROW_LAT;

if (TestOnly)	// set up some quick defaults
	{
	GRID_UNITS = DEM_DATA_UNITS_METERS;
	delta = 30.0;	// 30m postings
	ELEV_UNITS = DEM_DATA_UNITS_METERS;
	Importing->GridSpaceNS = Importing->GridSpaceWE = delta;
	baselat = OUTPUT_LOLAT;
	if (Importing->PosEast)
		baselong = Importing->WBound;
	else
		baselong = Importing->EBound;
/***
#ifdef WCS_BUILD_VNS
	baselong = OUTPUT_HILON;
#else // WCS_BUILD_VNS
	baselong = OUTPUT_LOLON;
#endif // WCS_BUILD_VNS
***/
	delta = ConvertMetersToDeg(delta, baselat, IWGlobeRad);
	OUTPUT_HILAT = baselat + (delta * (Size - 1));
	if (Importing->PosEast)
		Importing->EBound = baselong + delta * (Size - 1);
	else
		Importing->WBound = baselong + delta * (Size - 1);
/***
#ifdef WCS_BUILD_VNS
	OUTPUT_LOLON = baselong + (delta * (Size - 1));
#else // WCS_BUILD_VNS
	OUTPUT_HILON = baselong + (delta * (Size - 1));
#endif // WCS_BUILD_VNS
***/
	DataOpsDimsFromBounds();
	INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
	Importing->AllowPosE = TRUE;
	Importing->AskNull = FALSE;
	vptr = (USHORT *)&buffy[DEMstart];
	for (y = 0; y < (Size * Size); y++, vptr++)
		{
		value = *vptr;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(value, &value);
		#endif
		if (value > Importing->TestMax)
			Importing->TestMax = value;
		if (value < Importing->TestMin)
			Importing->TestMin = value;
		}
	} // if TestOnly

DataOfs = DEMstart;

if (!TestOnly)
	{
#ifdef RAWBRYCE
	if (tempout = fopen("E:/BryceBefore.raw", "wb")) // works on my HD, you may want to change this
		{
		fwrite(&buffy[DEMstart], 1, 2 * Size * Size, tempout);
		fclose(tempout);
		}
#endif // RAWBRYCE

	for (y = 0; y < (Size / 2); y++)		// we need to flip vertically
		{
		to = Size - (y + 1);				// which row we want
		to *= Size * (long)sizeof(short);	// row start
		to += DEMstart;
		fro = Size * (long)sizeof(short) * y;
		fro += DEMstart;
		memcpy(tmp, &buffy[to], Size * (long)sizeof(short));
		memcpy(&buffy[to], &buffy[fro], Size * (long)sizeof(short));
		memcpy(&buffy[fro], tmp, Size * (long)sizeof(short));
		} // for
	memcpy(Output, &buffy[DEMstart], Size * Size * (long)sizeof(short));

#ifdef RAWBRYCE
	if (tempout = fopen("E:/BryceAfter.raw", "wb"))
		{
		fwrite(&buffy[DEMstart], 1, 2 * Size * Size, tempout);
		fclose(tempout);
		}
#endif // RAWBRYCE
	} // if !TestOnly

AppMem_Free(buffy, filesize);

return(0);

} // LoadBryceDEM
