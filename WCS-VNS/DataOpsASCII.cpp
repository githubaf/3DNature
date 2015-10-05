// DataOpsASCII.cpp
// ASCII data code
// Code created on 11/30/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
#include "AppMem.h"

short ImportWizGUI::LoadASCII_DEM(char *filename, float *Output, char TestOnly)
{
short error = 1; // default to error
FILE *input = NULL;
long i, j;
long InputValues, ct;
char InputChar, InValue[64];
BusyWin *BWDO = NULL;
unsigned long red = 0;

Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
Importing->ValueBytes = 4;
Importing->AllowPosE = FALSE;
Importing->AskNull = TRUE;

// the PRJ file may have set a different scale
if (!TestOnly)
	{
	Importing->NBound *= Importing->HScale;
	Importing->SBound *= Importing->HScale;
	Importing->EBound *= Importing->HScale;
	Importing->WBound *= Importing->HScale;
	} // if !TestOnly

if ((input = PROJ_fopen(filename, "rb")) == NULL)
	{
	UserMessageOK("Import Wizard: ", "Warning\nCan't open file!");
	goto Cleanup;
	}

if (TestOnly)
	return 0;

if (! Importing->WrapData)
	InputValues = INPUT_ROWS * INPUT_COLS;
else
	InputValues = INPUT_ROWS * (INPUT_COLS - 1);

fseek(input, Importing->HdrSize, SEEK_SET);
BWDO = new BusyWin("Reading", (ULONG)INPUT_ROWS, 'BWDO', 0);
for (ct=0; ct<InputValues; ct++)
	{
	ReadMore:
	if ((InputChar = (char)fgetc(input)) == EOF)
		{
		error = 3;
		goto Cleanup;
		}
	if (InputChar < 45 || InputChar > 57)
		goto ReadMore;
	InValue[0] = InputChar;
	j = 0;
	while (j < 63 && ((InValue[j] > 44 && InValue[j] < 58) || (tolower(InValue[j]) == 'e')) && InValue[j] != EOF)
		{
		j ++;
		InValue[j] = (char)fgetc(input);
		} // while
	if (j >= 63)
		{
		error = 3;
		goto Cleanup;
		}
	InValue[j] = '\0';

	InputData4F[ct] = (FLOAT)(atof(InValue));

	if ((ct % INPUT_COLS) == 0)		// if we've read a row of data
		{
		red++;
		if (BWDO)
			{
			if (BWDO->CheckAbort())
				{
				error = 50;
				goto Cleanup;
				}
			BWDO->Update(red);
			} // if
		} // if
	} // for ct=0...

if (Importing->WrapData)
	{
	ULONG Source, Dest, InputRowSize, FullRowSize;

	InputRowSize = (ULONG)((INPUT_COLS - 1) * Importing->ValueBytes);
	FullRowSize = InputRowSize + Importing->ValueBytes;
	for (i = INPUT_ROWS - 1; i >= 0; i--)
		{
		Source = (ULONG)InputData4F + i * InputRowSize;
		Dest = (ULONG)InputData4F + i * FullRowSize;
		if (i > 0)
			memmove((char *)Dest, (char *)Source, InputRowSize);
		memcpy((char *)(Dest + InputRowSize), (char *)Dest, Importing->ValueBytes);
		} // for i
	} // if wrap longitude

// invert file if it is stored SE corner to NW
//if (INPUT_FORMAT == DEM_DATA2_INPUT_ASCII && InquiriesAllowed && UserMessageYN("Data Ops: Convert DEM", "Is this Mexican DEM data (stored from East to West)?", 1))
/***
if (INPUT_FORMAT == DEM_DATA2_INPUT_ASCII && UserMessageYN("Import Wizard: Load ASCII DEM",
	"Is this Mexican DEM data (stored from East to West)?", 1))
	{
	long DataPts, cnt;
	char *LowPtr, *HighPtr;

	DataPts = INPUT_ROWS * INPUT_COLS;
	LowPtr = (char *)InputData4F;
	HighPtr = LowPtr + (DataPts - 1) * Importing->ValueBytes;
	DataPts /= 2;
	for (cnt=0; cnt<DataPts; cnt++, LowPtr+=Importing->ValueBytes, HighPtr-=Importing->ValueBytes)
		{
		swmem(LowPtr, HighPtr, Importing->ValueBytes);
		} // for cnt
	} // if invert data
***/
error = 0;

Cleanup:
if (BWDO)
	delete BWDO;
if (input)
	fclose(input);
return error;

} // ImportWizGUI::LoadASCII_DEM

/*===========================================================================*/

short ImportWizGUI::LoadXYZ_WYXZ_CP(FILE *phyle, char TestOnly, bool LoadLinks)
{
char msg[80];
double dX, dY, dZ;
#ifndef WCS_BUILD_VNS
double LonDir = 1.0;
#endif // WCS_BUILD_VNS
long LinkIt = 0;
long fpos, times = 0, update_interval;
BusyWin *BWCP = NULL;
int count;
char *text;
char buffer[255], str1[51], str2[51], str3[51], str4[51], str5[51];
#ifdef WCS_BUILD_VNS
char FlipSignX = FALSE;
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_VNS
if (Importing->IWCoordSys.GetGeographic() && Importing->PosEast)
	FlipSignX = TRUE;
#endif // WCS_BUILD_VNS

ResetCPStats();

Importing->AllowRef00 = FALSE;

if (! CurDat)
	CurDat = TC;

fseek(phyle, 0, SEEK_END);
fpos = ftell(phyle);
rewind(phyle);

#ifndef WCS_BUILD_VNS
if (!TestOnly && Importing->HasUTM)
	UTMLatLonCoords_Init(&Importing->Coords.UTM, Importing->UTMZone);

// If Lat/Lon XYZ, ask which direction is positive longitude
if (!TestOnly && (Importing->Mode == 0) && (stricmp(Importing->MyIdentity, "XYZ_FILE") == 0))
	{
	if (UserMessageYN("Import Wizard:",	"Is East Longitude positive?", 0))
		LonDir = -1.0;
	}
#endif // WCS_BUILD_VNS

update_interval = fpos / 15360; // figure 60 chars per line * 256 desired updates to the meter
BWCP = new BusyWin("Reading", (ULONG)fpos, 'BWCP', 0);
//while (fscanf(phyle, "%le%le%le", &dX, &dY, &dZ) != EOF)
while (fgetline(buffer, 255, phyle))
	{
	if (LoadLinks) // WCS format - all we expect is spaces as separators
		count = sscanf(buffer, "%le%le%le%d", &dX, &dY, &dZ, &LinkIt);
	else
		{
		// all this work to accept commas: [^ ,\t] reads to separator, [ ,\t] reads separator
		text = buffer;
		while (isspace(*text))
			text++;
		count = sscanf(text, "%[^ ,\t]%[ ,\t]%[^ ,\t]%[ ,\t]%s", str1, str2, str3, str4, str5);
		dX = atof(str1); dY = atof(str3); dZ = atof(str5);
		}
//	if (LoadLinks)
//		sscanf(buffer, "%d", &LinkIt);

	if (BWCP)
		{
		times++;
		if (times > update_interval) // time to update the meter?
			{
			times = 0;
			if (BWCP->CheckAbort())
				{
				break;	// ???
				}
			fpos = ftell(phyle);
			BWCP->Update((ULONG)fpos);
			} // if updating
		} // if BWCP

	if (TestOnly)
		{
		if ((fabs(dX) > 100000.0) || (fabs(dY) > 100000.0))
			{
			Importing->HasUTM = TRUE;
			Importing->Mode = 1;
			} // if
		} // if TestOnly
	else
		{
		if (CurDat->nextdat = new ControlPointDatum())
			{
			CurDat = CurDat->nextdat;
#ifdef WCS_BUILD_VNS
			if (FlipSignX)
				dX = -dX;
			dX *= Importing->HScale;
			dY *= Importing->HScale;
#else // WCS_BUILD_VNS
			switch (Importing->Mode)
				{
				default:
				case 0:	// Lat/Lon
					dX *= LonDir;
					break;
				case 1:	// UTM
					Importing->Coords.UTM.East = dX;
					Importing->Coords.UTM.North = dY;
					UTM_LatLon(&Importing->Coords.UTM);
					dX = Importing->Coords.UTM.Lon;
					dY = Importing->Coords.UTM.Lat;
					break;
				case 2: // Arbitrary
					// convert dX & dY into Lat/Lon
					break;
				}
#endif // WCS_BUILD_VNS
			CurDat->values[0] = dX;
			CurDat->values[1] = dY;
			CurDat->values[2] = dZ * Importing->VScale;
			CurDat->LinkToLast = (char)LinkIt;
			ControlPts++;
			if (dX > cpstats.maxX)
				cpstats.maxX = dX;
			if (dX < cpstats.minX)
				cpstats.minX = dX;
			if (dY > cpstats.maxY)
				cpstats.maxY = dY;
			if (dY < cpstats.minY)
				cpstats.minY = dY;
			cpstats.n++;
			cpstats.sumX += dX;
			cpstats.sumX2 += dX * dX;
			cpstats.sumY += dY;
			cpstats.sumY2 += dY * dY;
			} // if new datum created 
		else
			{
			fclose(phyle);
			if (BWCP)
				delete BWCP;
			return 18;	// else out of memory
			} // else
		} // else TestOnly
	} // while 

if (BWCP)
	delete BWCP;

fclose(phyle);

if (TestOnly)
	return 0;

sprintf(msg, "Loaded %1d Control Points", ControlPts);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
//DoDrawPoints(FALSE);
return CPSave();

} // ImportWizGUI::LoadXYZ_WYXZ_CP

/*===========================================================================*/

short ImportWizGUI::LoadXYZ_CP(char *filename, char TestOnly)
{
FILE *fXYZ;

if (fXYZ = PROJ_fopen(filename, "r"))
	return LoadXYZ_WYXZ_CP(fXYZ, TestOnly, false);
else
	return 2;	// file open fail

} // ImportWizGUI::LoadXYZ_CP

/*===========================================================================*/

short ImportWizGUI::LoadWXYZ_CP(char *filename, char TestOnly)
{

FILE *fWXYZ;

if (fWXYZ = PROJ_fopen(filename, "r"))
	return LoadXYZ_WYXZ_CP(fWXYZ, TestOnly, true);
else
	return 2;	// file open fail

} // ImportWizGUI::LoadWXYZ_CP

/*===========================================================================*/

void ImportWizGUI::ResetCPStats(void)
{

cpstats.n = 0;
cpstats.sumX = cpstats.sumX2 = cpstats.sumY = cpstats.sumY2 = 0;
cpstats.maxX = cpstats.maxY = -DBL_MAX;
cpstats.minX = cpstats.minY = DBL_MAX;

} // ImportWizGUI::ResetCPStats

/*===========================================================================*/

short ImportWizGUI::LoadTIN(char *filename, char TestOnly)
{
double X1, X2, X3, Y1, Y2, Y3, Z1, Z2, Z3;
FILE *fTIN;
BusyWin *BWDO = NULL;
long fpos, times = 0, update_interval;
short triangles = 0;
int count;
char buffer[255], ch, sx1[40], sy1[40], sz1[40], sx2[40], sy2[40], sz2[40], sx3[40], sy3[40], sz3[40];
BOOL FlipSignX = FALSE;

if (!(fTIN = PROJ_fopen(filename, "r")))
	return 2;	// file open failed

#ifdef WCS_BUILD_VNS
if (Importing->IWCoordSys.GetGeographic() && Importing->PosEast)
	FlipSignX = TRUE;
#endif // WCS_BUILD_VNS

fseek(fTIN, 0, SEEK_END);
fpos = ftell(fTIN);
rewind(fTIN);
update_interval = fpos / 26880; // figure 105 chars per line * 256 desired updates to the meter
BWDO = new BusyWin("Reading", (ULONG)fpos, 'BWDO', 0);

while (fgetline(buffer, 255, fTIN))
	{
	count = sscanf(buffer, "%c %s %s %s %s %s %s %s %s %s", &ch, sx1, sy1, sz1, sx2, sy2, sz2, sx3, sy3, sz3);
	if ((ch != 't') || (count != 10))
		return 6;	// error reading file
	triangles++;
	X1 = atof(sx1); Y1 = atof(sy1); Z1 = atof(sz1);
	X2 = atof(sx2); Y2 = atof(sy2); Z2 = atof(sz2);
	X3 = atof(sx3); Y3 = atof(sy3); Z3 = atof(sz3);
#ifdef WCS_BUILD_VNS
	if (FlipSignX)
		{
		X1 = -X1;
		X2 = -X2;
		X3 = -X3;
		}
	X1 *= Importing->HScale;
	X2 *= Importing->HScale;
	X3 *= Importing->HScale;
	Y1 *= Importing->HScale;
	Y2 *= Importing->HScale;
	Y3 *= Importing->HScale;
	Z1 *= Importing->VScale;
	Z2 *= Importing->VScale;
	Z3 *= Importing->VScale;
#endif // WCS_BUILD_VNS

	if (!TestOnly)
		{
		if (CurDat->nextdat = new ControlPointDatum())
			{
			CurDat = CurDat->nextdat;
			CurDat->values[0] = X1;
			CurDat->values[1] = Y1;
			CurDat->values[2] = Z1;
			}
		if (CurDat->nextdat = new ControlPointDatum())
			{
			CurDat = CurDat->nextdat;
			CurDat->values[0] = X2;
			CurDat->values[1] = Y2;
			CurDat->values[2] = Z2;
			}
		if (CurDat->nextdat = new ControlPointDatum())
			{
			CurDat = CurDat->nextdat;
			CurDat->values[0] = X3;
			CurDat->values[1] = Y3;
			CurDat->values[2] = Z3;
			}
		} // if !TestOnly

	if (BWDO)
		{
		times++;
		if (times > update_interval) // time to update the meter?
			{
			times = 0;
			if (BWDO->CheckAbort())
				{
				break;	// ???
				}
			fpos = ftell(fTIN);
			BWDO->Update((ULONG)fpos);
			} // if updating
		} // if BWDO

	} // while

if (BWDO)
	delete BWDO;

fclose(fTIN);

if (TestOnly)
	triangles = 0;

return triangles;

} // ImportWizGUI::LoadTIN

/*===========================================================================*/
