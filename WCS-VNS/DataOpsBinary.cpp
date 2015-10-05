// DataOpsBinary.cpp
// Binary data code
// Code created on 10/12/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"

extern union ShortSwap *ShtSwp;

short ImportWizGUI::LoadBinary(char *filename, UBYTE *Output, char TestOnly)
{
//double *dbl_test;
//float *flt_test;
FILE *fbonus = NULL, *input = NULL;
UBYTE *buffer = NULL, *ptr;
size_t datasize;
long i, rowsize, truecols, truerows;
BusyWin *BWDO = NULL;
bool gtopobonus = false; //, found_nan, try_again;
short error = 0; // default to no error
short modulo = 0, y;
char bonusname[256], errmsg[256], temppath[256], gtoponame[80];

// if any of these are zero, we don't have enough info yet
if ((Importing->InCols * Importing->InRows * Importing->ValueBytes) == 0)
	return 0;	// no error though

if (Importing->WrapData)
	truecols = Importing->InCols - 1;
else
	truecols = Importing->InCols;
truerows = Importing->InRows;

// see if we need to do a GTOPO30 hack
if (strcmp(Importing->MyIdentity, "GTOPO30") == 0)
	{
	char GTName[32];

	(void)GlobalApp->GetProgDir(bonusname, 254);
	strmfp(temppath, bonusname, "Tools");
	strncpy(GTName, Importing->InFile, sizeof(GTName));
	StripExtension(GTName);
	sprintf(gtoponame, "Bonus%s.raw", GTName);
	strmfp(bonusname, temppath, gtoponame);
	if (fbonus = PROJ_fopen(bonusname, "rb"))
		{
		// we have one of the 27 non-Antarctic tiles, and we found the associated bonus data
		gtopobonus = true;
		truecols = 4800;
		Importing->InCols = 4801;
		truerows = 6000;
		Importing->InRows = 6001;
		modulo = 2;
		if (TestOnly)
			{
			Importing->SBound -= Importing->GridSpaceNS;	// make the bounds reflect the extra row & column
			Importing->EBound += Importing->GridSpaceWE;
			} // !TestOnly
		} // have fbonus
	} // if GTOPO30

datasize = (size_t)(Importing->InCols * Importing->InRows * Importing->ValueBytes);

rowsize = truecols * Importing->ValueBytes;

if ((input = PROJ_fopen(filename, "rb")) == NULL)
	{
	error = 2;
	sprintf(errmsg, "Unable to open file '%s'", filename);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	} // if

if (!(buffer = (UBYTE *)AppMem_Alloc(datasize, 0L)))
	{
	error = 1;
	goto Cleanup;
	} // if

ptr = buffer;
fseek(input, Importing->HdrSize, SEEK_SET); // skip the header
BWDO = new BusyWin("Reading", (unsigned long)Importing->InRows, 'BWDO', 0);
for (i = 0; i < truerows; ++i, ptr += rowsize + modulo)
	{
	if ((fread(ptr, 1, (size_t)rowsize, input)) != (unsigned long)rowsize)
		{
		error = 6;
//		UserMessageOK("Import Wizard", "An error has occurred while reading.");
		goto Cleanup;
		} // if
	if (Importing->WrapData)
		{
		UBYTE *Source, *Dest;

		Source = ptr;
		Dest = ptr + rowsize;
		memcpy((char *)(Dest), (char *)Source, Importing->ValueBytes);
		ptr += Importing->ValueBytes;
		} // if
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			goto Cleanup;
			} // if
		BWDO->Update((unsigned long)i);
		} // if
	} // for

// hack extra data into GTOPO tile if needed
if (gtopobonus)
	{
	ptr = buffer;
	for (y = 0; y < 6000; ++y)
		{
		ptr += rowsize;				// skip data already in memory
		fread(ptr, 2, 1, fbonus);	// read a cell for the extra column
		ptr += modulo;				// advance to the next row
		} // for

	// columns done - read in last row & additional cell
	fread(ptr, 2, 4801, fbonus);
	} // if

// set max / min
if (TestOnly && stricmp(Importing->MyIdentity, "STM") == 0)
	{
	InputData2U = (unsigned short *)buffer;
	// Invert data if moving between platforms
	#ifdef BYTEORDER_LITTLEENDIAN
	if ((Importing->ByteOrder == DEM_DATA_BYTEORDER_HILO) && (Importing->ValueBytes != 1))
		{
		ShtSwp = (union ShortSwap *)InputData2U;
		DataOpsInvert();
		} // if
	#else
	if ((Importing->ByteOrder == DEM_DATA_BYTEORDER_LOHI) && (Importing->ValueBytes != 1))
		{
		ShtSwp = (union ShortSwap *)InputData2U;
		DataOpsInvert();
		} // if
	#endif // BYTEORDER_LITTLEENDIAN
	DataOpsScale();
	InputData2U = NULL;
	} // if

if (!TestOnly)
	{
	memcpy(Output, buffer, datasize);
	if (stricmp(Importing->MyIdentity, "STM") == 0)
		{
		//if (UserMessageYN("Import Wizard", "Flip the Latitude axis?"))
			DataOpsFlipYAxis(Output, Importing->InCols * 2, Importing->InRows);
		}
	if (stricmp(Importing->MyIdentity, "SURFER") == 0)
		{
		DataOpsFlipYAxis(Output, Importing->InCols * Importing->ValueBytes, Importing->InRows);
		}
/* 3DNAQUICKFIX
	else if ((Importing->ValueFmt == 2) && (Importing->ValueBytes == 4))
		{
		try_again = FALSE;
		do
			{
			found_nan = FALSE;
			flt_test = (float *)Output;
			for (i = 0; i < Importing->InCols * Importing->InRows; i++, flt_test++)
				{
				if (try_again)
					SimpleEndianFlip32F(flt_test, flt_test);
				if (_isnan((double)(*flt_test)))
					{
					found_nan = TRUE;
					break;
					} // if
				} // for
			if (found_nan)
				{
				if (!try_again)	// first time through
					{
					if (UserMessageRETRYCAN("Import Wizard",
						"An invalid number was found.  Shall I swap the byte order and retry, or abort the import?", 0))
						try_again = TRUE;
					else
						error = 50;	// user cancel code
					}
				else	// still bad on 2nd attempt
					{
					UserMessageOK("Import Wizard",
						"An invalid number was still found.  Check your settings.  Aborting.");
					try_again = FALSE;
					error = 50;	// user cancel code
					}
				}
			else	// all valid numbers
				try_again = FALSE;
			} while (try_again);
		}
	else if ((Importing->ValueFmt == 2) && (Importing->ValueBytes == 8))
		{
		try_again = FALSE;
		do
			{
			found_nan = FALSE;
			dbl_test = (double *)Output;
			for (i = 0; i < Importing->InCols * Importing->InRows; i++, dbl_test++)
				{
				if (try_again)
					SimpleEndianFlip64(dbl_test, dbl_test);
				if (_isnan(*dbl_test))
					{
					found_nan = TRUE;
					break;
					}
				}
			if (found_nan)
				{
				if (!try_again)	// first time through
					{
					if (UserMessageRETRYCAN("Import Wizard",
						"An invalid number was found.  Shall I swap the byte order and retry, or abort the import?", 0))
						try_again = TRUE;
					else
						error = 50;	// user cancel code
					}
				else	// still bad on 2nd attempt
					{
					UserMessageOK("Import Wizard",
						"An invalid number was still found.  Check your settings.  Aborting.");
					try_again = FALSE;
					error = 50;	// user cancel code
					}
				}
			else	// all valid numbers
				try_again = FALSE;
			} while (try_again);
		}
*/
	}

Cleanup:
if (BWDO)
	delete BWDO;
if (buffer)
	AppMem_Free(buffer, datasize);
if (input)
	fclose(input);

return error;

} // ImportWizGUI::LoadBinary

/*===========================================================================*/

short ImportWizGUI::LoadSurfer(char *filename, float *Output, char TestOnly)
{
double dblval, dlo, dhi;
FILE *input = NULL;
long fpos, longval, tagid, tagsize;
short error = 0, nx, ny;
char data[4];

if ((input = PROJ_fopen(filename, "rb")) == NULL)
	{
	error = 2;
	goto Cleanup;
	} // if

if (fread(data, 1, 4, input) != 4)
	{
	error = 6;	// read error
	goto Cleanup;
	} // if

if ((strncmp(data, "DSAA", 4) != 0) && (strncmp(data, "DSBB", 4) != 0) && (strncmp(data, "DSRB", 4) != 0))
	{
	error = 6;	// read error
	goto Cleanup;
	} // if

if (data[2] == 'A')			// Version 5 & prior
	Importing->Flags = IMWIZ_SURFER_ASCII;
else if (data[2] == 'B')	// Version 6
	Importing->Flags = IMWIZ_SURFER_BINARY;
else						// Version 7 & later
	Importing->Flags = IMWIZ_SURFER_TAGGED;

switch (Importing->Flags)
	{
	default:
	case IMWIZ_SURFER_ASCII:	// Version 5 & prior
		Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
		Importing->ValueBytes = 4;
		break;
	case IMWIZ_SURFER_BINARY:	// Version 6
		Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
		Importing->ValueBytes = 4;
		if ((fread(&nx, 2, 1, input) != 1) || (fread(&ny, 2, 1, input) != 1))
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		Importing->InCols = nx;
		Importing->InRows = ny;
		if ((fread(&dlo, 8, 1, input) != 1) || (fread(&dhi, 8, 1, input) != 1))
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		Importing->West = dlo;
		Importing->East = dhi;
		if ((fread(&dlo, 8, 1, input) != 1) || (fread(&dhi, 8, 1, input) != 1))
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		Importing->South = dlo;
		Importing->North = dhi;
		if ((fread(&dlo, 8, 1, input) != 1) || (fread(&dhi, 8, 1, input) != 1))
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		Importing->TestMin = dlo;
		Importing->TestMax = dhi;
		fpos = ftell(input);
		Importing->HdrSize = fpos;
		Importing->Flags = IMWIZ_SURFER_BINARY;
		Importing->HasNulls = TRUE;
		Importing->NullVal = 1.7014100091878e+038;	// near FLT_MAX, but not quite - don't know where they get this from
		Importing->UseCeiling = TRUE;
		Importing->Ceiling = 1.7014100091878e+038;	// can't get accurate enough NULL value
		if (Importing->InRows != 1)
			Importing->GridSpaceNS = (Importing->North - Importing->South) / (Importing->InRows - 1);
		if (Importing->InCols != 1)
			Importing->GridSpaceWE = fabs((Importing->East - Importing->West) / (Importing->InCols - 1));
		break;
	case IMWIZ_SURFER_TAGGED:	// Version 7 & later
		error = 6;	// set read error return code in case any problems are encountered
		Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
		Importing->ValueBytes = 8;
		if (fread(&tagsize, 4, 1, input) != 1)
			goto Cleanup;
		fpos = ftell(input);
		if (fread(&longval, 4, 1, input) != 1)	// get version number
			goto Cleanup;
		fseek(input, fpos + tagsize, SEEK_SET);
		if (fread(&tagid, 4, 1, input) != 1)
			goto Cleanup;
		while ((tagid != 0x44495247) && !feof(input))	// "GRID"
			{
			if (fread(&tagsize, 4, 1, input) != 1)
				goto Cleanup;
			fpos = ftell(input);
			fseek(input, fpos + tagsize, SEEK_SET);
			} // while
		if (tagid == 0x44495247)	// "GRID"
			{
			if (fread(&tagsize, 4, 1, input) != 1)
				goto Cleanup;
			fpos = ftell(input);
			if (fread(&longval, 4, 1, input) != 1)	// nRow
				goto Cleanup;
			Importing->InRows = longval;
			if (fread(&longval, 4, 1, input) != 1)	// nCol
				goto Cleanup;
			Importing->InCols = longval;
			if (fread(&dblval, 8, 1, input) != 1)	// xLL
				goto Cleanup;
			Importing->West = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// yLL
				goto Cleanup;
			Importing->South = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// xSize
				goto Cleanup;
			Importing->GridSpaceWE = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// ySize
				goto Cleanup;
			Importing->GridSpaceNS = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// zMin
				goto Cleanup;
			Importing->TestMin = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// zMax
				goto Cleanup;
			Importing->TestMax = dblval;
			if (fread(&dblval, 8, 1, input) != 1)	// Rotation
				goto Cleanup;
			// rotation value not currently used in Surfer, probably wouldn't be supported by us anyways
			if (fread(&dblval, 8, 1, input) != 1)	// BlankValue
				goto Cleanup;
			error = 0;	// clear error flag
			Importing->HasNulls = TRUE;
			Importing->AskNull = TRUE;
			Importing->NullVal = dblval;
			} // if GRID
		break;
	} // switch

Importing->AllowPosE = TRUE;
Importing->NullMethod = IMWIZ_NULL_GE;

Importing->SBound = Importing->South;
Importing->NBound = Importing->SBound + Importing->GridSpaceNS * (Importing->InRows - 1);
Importing->WBound = Importing->West;
Importing->EBound = Importing->WBound + Importing->GridSpaceWE * (Importing->InCols - 1);

if (TestOnly)
	{
	sprintf(Importing->FileInfo, "\r\r\n\r\r\nX range: %.1f  -  %.1f\r\r\nY range: %.1f  -  %.1f\r\r\nZ range: %.1f  -  %.1f",
		Importing->WBound, Importing->EBound, Importing->NBound, Importing->SBound, Importing->TestMin, Importing->TestMax);
	return 0;
	} // if

switch (Importing->Flags)
	{
	default:
	case IMWIZ_SURFER_ASCII:	// Version 5 & prior
		break;
	case IMWIZ_SURFER_BINARY:	// Version 6
		break;
	case IMWIZ_SURFER_TAGGED:	// Version 7 & higher
		if (fread(&tagid, 4, 1, input) != 1)
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		if (fread(&tagsize, 4, 1, input) != 1)
			{
			error = 6;	// read error
			goto Cleanup;
			} // if
		fpos = ftell(input);
		switch (tagid)
			{
			default:
				break;
			case 0x41544144:	// "DATA"
				Importing->HdrSize = ftell(input);
				LoadBinary(filename, (unsigned char *)Output, TestOnly);
				break;
			} // switch
		fseek(input, fpos + tagsize, SEEK_SET);
		break;
	} // switch

Cleanup:
if (input)
	fclose(input);

return error;

} // ImportWizGUI::LoadSurfer
