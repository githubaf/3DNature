// DataOpsOrdnanceSurvey.cpp
// Code for reading & writing British NTF data
// Written by Frank Weed II on 10/31/01
// Copyright 2001 3D Nature

// Loads these formats:
// Land-Form Panorama DTM from Ordnance Survey
// Land-Form PROFILE DTM from Ordnance Survey

#include "stdafx.h"

#ifdef WCS_BUILD_VNS

#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"

short ImportWizGUI::LoadNTF_DTM(char *filename, float *Output, char TestOnly)
{
FILE *input;
short error = 0;	// default to no error
char buffer[84], errmsg[80];

if ((input = PROJ_fopen(filename, "r")) == NULL)
	{
	error = 2;
	sprintf(errmsg, "Unable to open file '%s'", filename);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	}

// Volume Header Record
fgetline(buffer, 66+2, input);
if (strnicmp(buffer, "01", 2) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Volume Header Record.");
	error = 36;
	goto Cleanup;
	}

// Database Header Record
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "02OS_LANDRANGER_DTM", 19) == 0)
	error = LoadLandFormPanoramaDTM(input, Output, TestOnly);
else if (strnicmp(buffer, "02L-F_PROFILE_DTM", 17) == 0)
	error = LoadLandFormProfileDTM(input, Output, TestOnly);
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Database Header Record.");
	error = 36;
	}

Cleanup:

return error;

} // ImportWizGUI::LoadNTF_DTM

/*===========================================================================*/

short ImportWizGUI::LoadLandFormPanoramaDTM(FILE *input, float *Output, char TestOnly)
{
float *outptr;
long CurCol;
short i, j, first, last;
short error = 0;	// default to no error
char buffer[84];

fgetline(buffer, 80+2, input);	// continuation of DHR

// Data Description Record
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "03GRID_ID", 9) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Data Description Record.");
	error = 36;
	goto Cleanup;
	}
do
	{
	fgetline(buffer, 80+2, input);
	} while (strncmp(buffer, "03", 2) == 0);

// Data Format Record, Grid Header Record
if (strnicmp(buffer, "0450", 4) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Header Record.");
	error = 36;
	goto Cleanup;
	}
do
	{
	fgetline(buffer, 80+2, input);
	} while (strnicmp(buffer, "00", 2) == 0);

// Data Format Record, Grid Data Record
if (strnicmp(buffer, "0451", 4) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Data Record.");
	error = 36;
	goto Cleanup;
	}
do
	{
	fgetline(buffer, 80+2, input);
	} while (strnicmp(buffer, "00", 2) == 0);

if (strnicmp(buffer, "07", 2) != 0)	// Section Header Record
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Section Header Record.");
	error = 36;
	goto Cleanup;
	}
else
	{
	(void)TrimTrailingSpaces(Importing->NameBase);
	Importing->HScale = atoisub(buffer, 21, 30, 1) / 1000.0;
	Importing->VScale = atoisub(buffer, 37, 46, 1) / 1000.0;
	Importing->WBound = atoisub(buffer, 47, 56, 1);
	Importing->EBound = Importing->WBound + 20000;	// technically, (InCols - 1) * xres, but should always be (401 - 1) * 50
	Importing->SBound = atoisub(buffer, 57, 66, 1);
	Importing->NBound = Importing->SBound + 20000;	// similar to above comment
	do
		{
		fgetline(buffer, 80+2, input);
		} while (strnicmp(buffer, "00", 2) == 0);
	}

#ifdef BYTEORDER_LITTLENDIAN
INBYTE_ORDER = DEM_DATA_BYTEORDER_LOHI;
#else
INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
#endif
INVALUE_SIZE = DEM_DATA_VALSIZE_LONG; 
INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;

// Grid Header Record
if (strnicmp(buffer, "50", 2) == 0)	// Grid Header Record
	{
	Importing->InCols = atoisub(buffer, 13, 16, 1);
	Importing->InRows = atoisub(buffer, 17, 20, 1);
	do
		{
		fgetline(buffer, 80+2, input);
		} while (strnicmp(buffer, "00", 2) == 0);
	}
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Header Record.");
	error = 36;
	goto Cleanup;
	}

Importing->IWCoordSys.SetSystemByCode(21);

if (!TestOnly)
	{
	CurCol = 0;
	outptr = Output;
	do
		{
		// Grid Data Record
		if (strnicmp(buffer, "51", 2) == 0)	// Grid Header Record
			{
			fgetline(buffer, 80+2, input);
			// this data record can be hell to parse, so we're gonna "cheat" and hard code the read
			for (j = 0; j < 21; j++)
				{
				fgetline(buffer, 80+2, input);
				for (i = 0, first = 3, last = 6; i < 19; i++, first += 4, last += 4)	// read the first 21 lines of 19 elevs
					{
					*outptr = (float)(atoisub(buffer, first, last, 1) * Importing->VScale);
					outptr++;
					}
				}
			fgetline(buffer, 80+2, input);
			for (i = 0, first = 3, last = 6; i < 2; i++, first += 4, last += 4)	// read the last line with 2 elevs
				{
				*outptr = (float)(atoisub(buffer, first, last, 1) * Importing->VScale);
				outptr++;
				}
			CurCol++;
			fgetline(buffer, 80+2, input);
			} // if "51"
		else
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Header Record.");
			error = 36;
			goto Cleanup;
			}
		} while (CurCol < 401);
	} // !TestOnly

Cleanup:

return error;

} // ImportWizGUI::LoadLandFormPanoramaDTM

/*===========================================================================*/

short ImportWizGUI::LoadLandFormProfileDTM(FILE *input, float *Output, char TestOnly)
{
float *outptr;
long CurCol;
short error = 0;	// default to no error
short i, j, first, last;
char buffer[84], *sptr;

fgetline(buffer, 80+2, input);	// continuation of DHR

// Data Description Record
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "03GRID_REF", 10) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Data Description Record.");
	error = 36;
	goto Cleanup;
	}
fgetline(buffer, 80+2, input);
while (strncmp(buffer, "03", 2) == 0)
	{
	if (strncmp(buffer+2, "N_COLUMNS", 9) == 0)
		{
		if ((sptr = strchr(buffer, '\\')) && (sptr++) && (sptr = strchr(sptr, '\\')))
			Importing->InCols = atoisub(sptr, 1, 8, 0);
		else
			error = 36;
		}
	else if (strncmp(buffer+2, "N_ROWS", 6) == 0)
		{
		if ((sptr = strchr(buffer, '\\')) && (sptr++) && (sptr = strchr(sptr, '\\')))
			Importing->InRows = atoisub(sptr, 1, 8, 0);
		else
			error = 36;
		}
	else if (strncmp(buffer+2, "COL_INTRVL", 10) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else if (strncmp(buffer+2, "ROW_INTRVL", 10) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else if (strncmp(buffer+2, "SURV_METH", 9) == 0)
		{
		}
	else if (strncmp(buffer+2, "SURVEY", 6) == 0)
		{
		}
	else if (strncmp(buffer+2, "CHAN_METH", 9) == 0)
		{
		}
	else if (strncmp(buffer+2, "CHANGE", 6) == 0)
		{
		}
	else if (strncmp(buffer+2, "GRID_ID", 7) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else if (strncmp(buffer+2, "N_Z_COORD", 9) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else if (strncmp(buffer+2, "AREA_AMND", 9) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else if (strncmp(buffer+2, "HO_UNIT_CT", 10) == 0)
		{
		fgetline(buffer, 80+2, input);
		}
	else
		error = 36;
	if (error)
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Data Description Record.");
		goto Cleanup;
		}
	fgetline(buffer, 80+2, input);
	}

// Data Format Record
if (strnicmp(buffer, "04", 2) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Data Format Record.");
	error = 36;
	goto Cleanup;
	}

do
	{
	if (strnicmp(buffer+2, "50", 2) == 0)	// Grid Header Record
		{
		do
			{
			fgetline(buffer, 80+2, input);
			} while (strnicmp(buffer, "00", 2) == 0);
		}
	else if (strnicmp(buffer + 2, "51", 2) == 0)	// Grid Data Record
		{
		fgetline(buffer, 80+2, input);
		}
	else
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An unknown record type was encountered.");
		error = 36;
		goto Cleanup;
		}
	} while (strncmp(buffer, "04", 2) == 0);

Importing->IWCoordSys.SetSystemByCode(21);

// Section Header Record
if (strnicmp(buffer, "07", 2) == 0)	// Section Header Record
	{
	strncpy(Importing->NameBase, buffer + 2, 10);
	Importing->NameBase[10] = 0;
	(void)TrimTrailingSpaces(Importing->NameBase);
	Importing->HScale = atoisub(buffer, 21, 30, 1) / 1000.0;
	Importing->VScale = atoisub(buffer, 37, 46, 1) / 1000.0;
	Importing->WBound = atoisub(buffer, 47, 56, 1);
	Importing->EBound = Importing->WBound + 5000;	// technically, (InCols - 1) * xres, but should always be (501 - 1) * 10
	Importing->SBound = atoisub(buffer, 57, 66, 1);
	Importing->NBound = Importing->SBound + 5000;	// similar to above comment
	do
		{
		fgetline(buffer, 80+2, input);
		} while (strnicmp(buffer, "00", 2) == 0);
	}
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Section Header Record.");
	error = 36;
	goto Cleanup;
	}

#ifdef BYTEORDER_LITTLENDIAN
INBYTE_ORDER = DEM_DATA_BYTEORDER_LOHI;
#else
INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
#endif
INVALUE_SIZE = DEM_DATA_VALSIZE_LONG; 
INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;

if (!TestOnly)
	{
	// Grid Header Record
	if (strnicmp(buffer, "50", 2) == 0)	// Grid Header Record
		{
		// what are we really supposed to do with X_COORD & Y_COORD???
		fgetline(buffer, 80+2, input);
		}
	else
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Header Record.");
		error = 36;
		goto Cleanup;
		}

	// Grid Data Record
	CurCol = 0;
	outptr = Output;
	if (strnicmp(buffer, "51", 2) == 0)	// Grid Header Record
		{
		do
			{
			// this data record can be hell to parse, so we're gonna "cheat" and hard code the read
			for (i = 0, first = 19, last = 23; i < 12; i++, first += 5, last += 5)	// scan the first line
				{
				*outptr = (float)(atoisub(buffer, first, last, 1) * Importing->VScale);
				outptr++;
				}
			for (j = 0; j < 32; j++)
				{
				fgetline(buffer, 80+2, input);
				for (i = 0, first = 3, last = 7; i < 15; i++, first += 5, last += 5)	// scan the middle lines
					{
					*outptr = (float)(atoisub(buffer, first, last, 1) * Importing->VScale);
					outptr++;
					}
				}
			fgetline(buffer, 80+2, input);
			for (i = 0, first = 3, last = 7; i < 9; i++, first += 5, last += 5)	// scan the last line
				{
				*outptr = (float)(atoisub(buffer, first, last, 1) * Importing->VScale);
				outptr++;
				}
			CurCol++;
			fgetline(buffer, 80+2, input);
			} while (CurCol < 501);
		} // if "51"
	else
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Unexpected record found before Grid Header Record.");
		error = 36;
		goto Cleanup;
		}
	}

Cleanup:

return error;

} // ImportWizGUI::LoadLandFormProfileDTM

/*===========================================================================*/

short ImportWizGUI::LoadNTF_Meridian2(char *filename, float *Output, char TestOnly)
{
FILE *input;
short error = 0;	// default to no error
char buffer[84], errmsg[80];

if ((input = PROJ_fopen(filename, "r")) == NULL)
	{
	error = 2;
	sprintf(errmsg, "Unable to open file '%s'", filename);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
	goto Cleanup;
	}

// Volume Header Record
fgetline(buffer, 66+2, input);
if (strnicmp(buffer, "01ORDNANCE SURVEY", 17) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Volume Header Record.");
	error = 36;
	goto Cleanup;
	}

// Database Header Record
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "02Meridian_02.00", 16) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Database Header Record.");
	error = 36;
	}
// Database Header Record - continued
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "00Meridian_02.00", 16) == 0)
	error = 0;
//	error = LoadNTFMeridian2Vect(input);
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Database Header Record.");
	error = 36;
	}

Cleanup:

return error;

} // ImportWizGUI::LoadNTF_Meridian2

/*===========================================================================*/

short ImportWizGUI::LoadNTFMeridian2DTM(FILE *input, float *Output, char TestOnly)
{

return 0;

} // ImportWizGUI::LoadNTFMeridian2DTM

/*===========================================================================*/

unsigned long int ImportWizGUI::ImportMeridian2(FILE *input, Joe *Level)
{
unsigned long int NumLoaded = 0;
unsigned short i, numcoords, xcoord, ycoord;
short error = 0;	// default to no error
unsigned char code, firstx, lastx, firsty, lasty;
char buffer[82];

// Volume Header Record
fgetline(buffer, 66+2, input);
if (strnicmp(buffer, "01ORDNANCE SURVEY", 17) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Volume Header Record.");
	error = 36;
	goto Cleanup;
	}

// Database Header Record
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "02Meridian_02.00", 16) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Database Header Record.");
	error = 36;
	}
// Database Header Record - continued
fgetline(buffer, 80+2, input);
if (strnicmp(buffer, "00Meridian_02.00", 16) != 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "An error was encountered in the Database Header Record.");
	error = 36;
	}

fgetline(buffer, 80+2, input);
if (Level)
	{
	while (strncmp(buffer, "99", 2) != 0)
		{
		code = atoisub(buffer, 0, 1, 0);
		switch (code)
			{
			case 05:
				break;
			case 21:	// two dimensional geometry record
				if (buffer[8] == '1')			// point record
					{
					xcoord = atoisub(buffer, 14, 18, 1);
					ycoord = atoisub(buffer, 19, 23, 1);
					}
				else if (buffer[8] == '2')		// line record
					{
					numcoords = atoisub(buffer, 10, 13, 1);
					firstx = 14; lastx = 18;
					firsty = 19; lasty = 23;
					for (i = 0; i < numcoords; i++, firstx += 11, lastx += 11, firsty += 11, lasty += 11)
						{
						xcoord = atoisub(buffer, firstx, lastx, 1);
						ycoord = atoisub(buffer, firsty, lasty, 1);
						}
					}
				break;
			case 23:
				break;
			case 14:
				break;
			case 40:
				break;
			case 90:
				break;
			} // switch
		fgetline(buffer, 80+2, input);
		} // while
	}
else
	return 1;	// fake that we loaded something

Cleanup:

if (error)
	return 0;
else
	return NumLoaded;

} // ImportWizGUI::ImportMeridian2

#endif // WCS_BUILD_VNS
