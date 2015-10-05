// DataOpsTerragen.cpp
// Terragen data code
// Code created on 10/13/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"

short ImportWizGUI::LoadTerragen(char *filename, float *Output, char TestOnly)
{
double maxval = -DBL_MAX, minval = DBL_MAX;
FILE *input = NULL;
UBYTE *ibuffer = NULL, *obuffer = NULL;
float *outval;
long datasize;
short i,j;
short *inval;
short BaseHeight, HeightScale;
short error = 1; // default to error

Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
Importing->ValueBytes = 4;
//#ifdef WCS_BUILD_VNS
Importing->AllowPosE = TRUE;
//#else // WCS_BUILD_VNS
//Importing->AllowPosE = FALSE;
//#endif // WCS_BUILD_VNS
Importing->AskNull = FALSE;
Importing->CoordSysWarn = FALSE;

datasize = Importing->InCols * Importing->InRows * 2;

if ((input = PROJ_fopen(filename, "rb")) == NULL)
	{
	UserMessageOK("Import: ", "Warning\nCan't open file!");
	goto Cleanup;
	}

ibuffer = (UBYTE *)AppMem_Alloc((unsigned int)datasize, 0L);
obuffer = (UBYTE *)AppMem_Alloc((unsigned int)datasize * 2, 0L);
if (!ibuffer || !obuffer)
	{
	UserMessageOK("Import: ", "Warning\nUnable to allocate memory for file.");
	goto Cleanup;
	}

fseek(input, Importing->HdrSize, SEEK_SET); // skip the header
(void)fread(&HeightScale, 1, 2, input);
#ifdef BYTEORDER_BIGENDIAN
SimpleEndianFlip16S(HeightScale, &HeightScale);
#endif // BYTEORDER_BIGENDIAN
(void)fread(&BaseHeight, 1, 2, input);
#ifdef BYTEORDER_BIGENDIAN
SimpleEndianFlip16S(BaseHeight, &BaseHeight);
#endif // BYTEORDER_BIGENDIAN
if ((unsigned long)datasize != fread(ibuffer, 1, (unsigned int)datasize, input))
	{
	UserMessageOK("Import: ", "Warning\nAn error has occurred while reading.");
	goto Cleanup;
	}

// convert the encoded elevations: absolute altitude = BaseHeight + Elevation * HeightScale / 65536. 
inval = (short *)ibuffer;
outval = (float *)obuffer;
for (i = 0; i < Importing->InRows; i++)
	for (j = 0; j < Importing->InCols; j++)
		{
		#ifdef BYTEORDER_BIGENDIAN
		SimpleEndianFlip16S(*inval, inval);
		#endif
		*outval = (float)(BaseHeight + *inval++ *  HeightScale / 65536.0);
		if (*outval > maxval)
			maxval = *outval;
		if (*outval < minval)
			minval = *outval;
		outval++;
		}

Importing->TestMax = maxval;
Importing->TestMin = minval;

DataOpsFlipYAxis(obuffer, Importing->InCols * 4, Importing->InRows);

if (!TestOnly)
	memcpy(Output, obuffer, (unsigned int)datasize * 2);

error = 0; // success

Cleanup:
if (obuffer)
	AppMem_Free(obuffer, (unsigned int)datasize * 2);
if (ibuffer)
	AppMem_Free(ibuffer, (unsigned int)datasize);
if (input)
	fclose(input);

return error;

} // ImportWizGUI::LoadTerragen
