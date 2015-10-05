// DataOpsUSGS.cpp
// Code for the USGS ASCII files
// Adapted from DEMExtractGUI.cpp on 10/15/99 by Frank Weed II
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "DataOpsDefs.h"
#include "AppMem.h"
#include "Project.h"
#include "Application.h"
#include "Projection.h"
#include "Requester.h"
#include "Database.h"
#include "Notify.h"
#include "Joe.h"
#include "DEM.h"

#ifdef DEBUG
// USGS_SEEKING will enable file position reporting - handy for debugging
#define USGS_SEEKING

// this will save out the raw UTM data read from a USGS 30 meter file as a WCS DEM
// at the correct lat lon but without resampling to NS grid or patching missing data
//#define SAVE_RAW_UTM
#endif

static BOOL HaveSWCorner;

/*===========================================================================*/

/***
void ImportWizGUI::CreateUSGSDBGroup(char *Name)
{

if (Importing->FileParent = new (Name) Joe)
	{
	Importing->FileParent->SetFlags(WCS_JOEFLAG_HASKIDS);
	DBHost->AddJoe(Importing->FileParent, WCS_DATABASE_STATIC);
	} // if

} // ImportWizGUI::CreateUSGSDBGroup
***/

/*===========================================================================*/

/***
// attempt to read Lat/Lon from the first filler field in Data Element 1, Logical Record A
// since they're sometimes being stuffed in there (CDED)
short ImportWizGUI::GetSWCornerFromFiller(struct USGS_DEMHeader *USGSHdr)
{
char sign;
double d, m, s;
double selat, swlon;

// try to read 3 values, and make sure something useful was actually read
if ((sscanf(&USGSHdr->FileName[109], "%c%3lf%2lf%6lf", &sign, &d, &m, &s) != 4) || (d == 0))
	return 0;
swlon = d + m/60 + s/3600;
if (sign == '-')
	swlon = -swlon;

if ((sscanf(&USGSHdr->FileName[123], "%c%2lf%2lf%6lf", &sign, &d, &m, &s) != 4) || (d == 0))
	return 0;
selat = d + m/60 + s/3600;
if (sign == '-')
	selat = -selat;

Importing->SBound = selat;
Importing->WBound = -swlon;	// since their negative is our positive

HaveSWCorner = TRUE;

return 1;

} // ImportWizGUI::GetSWCornerFromFiller
***/

/*===========================================================================*/

short ImportWizGUI::GetSECornerFromUser(void)
{
char FloatStr[32], MsgStr[120];

if (Importing->TestOnly)
	return 1;	// don't ask the first time thru

FloatStr[0] = 0;
sprintf(MsgStr, "Please enter the Latitude (N-S) of the SouthEast corner for the DEM '%s'", Importing->InFile);

// ask for lat/lon values to use
if (GetInputString(MsgStr, "", FloatStr))
	{
	Importing->SBound = atof(FloatStr);
	FloatStr[0] = 0;
	sprintf(MsgStr, "Please enter the Longitude (E-W) of the SouthEast corner for the DEM '%s'", Importing->InFile);
	if (GetInputString(MsgStr, "", FloatStr))
		{
		Importing->EBound = atof(FloatStr);
		return (1);
		} // if
	return (0);
	} // if

return (0);

} // ImportWizGUI::GetSECornerFromUser

/*===========================================================================*/

short ImportWizGUI::Match250FileName(char *InputName, char *OutBaseName)
{
FILE *fMatch;
char filename[256], temppath[256];
char MatchName[64];
char ReadBuf[80];

if (InputName[0])
	{
	GlobalApp->GetProgDir(filename, 254);
	strmfp(temppath, filename, "Tools");
	strmfp(filename, temppath, "DEM250Files.txt");
	if ((fMatch = PROJ_fopen(filename, "r")) || (fMatch = PROJ_fopen("WCSProjects:DEM250Files.txt", "r")))
		{
		if (InputName[strlen(InputName) - 1] == '.')
			InputName[strlen(InputName) - 1] = 0;
		while (fscanf(fMatch, "%s", MatchName) == 1)
			{
			if (! strnicmp(InputName, MatchName, strlen(MatchName)))
				{
				fscanf(fMatch, "%s", OutBaseName);
//				fscanf(fMatch, "%le", &DEMExtract->SELat);
//				fscanf(fMatch, "%le", &DEMExtract->SELon);
				fscanf(fMatch, "%le", &Importing->SBound);
				fscanf(fMatch, "%le", &Importing->EBound);
				Importing->NBound = Importing->SBound + 1.0;
				Importing->WBound = Importing->EBound + 1.0;
				DataOpsDimsFromBounds();
				if (OutBaseName[0])
					{
					fclose(fMatch);
					return (1);
					} // if
				break;
				} // if match found
			if (! fgetline(ReadBuf, 80, fMatch))
				break;
			} // while
		fclose(fMatch);
		} // if
	} // if

return (0);

} // ImportWizGUI::Match250FileName

/*===========================================================================*/

void ImportWizGUI::CloseUSGSDBGroup(void)
{

if (Importing->FileParent)
	{
	if (Importing->LastJoe)
		DBHost->BoundUpTree(Importing->LastJoe);
	Importing->LastJoe = NULL;
	Importing->FileParent = NULL;
	} // if

} // ImportWizGUI::CloseUSGSDBGroup

/*===========================================================================*/

static double MaxEast[2], MinEast[2], MaxNorth[2], MinNorth[2];
static double UTMColInt, UTMRowInt;
static double MaxLat, MinLat, MaxLon, MinLon;
static char *MsgHdr = "Import Wizard: USGS ASCII DEM";

/*===========================================================================*/

short ImportWizGUI::GetUTMBounds(struct DEMInfo *Info)
{

MaxEast[0] 	= -FLT_MAX;
MinEast[0] 	= FLT_MAX;
MaxNorth[0] = -FLT_MAX;
MinNorth[0] = FLT_MAX;
MaxLon 	= -FLT_MAX;
MinLon 	= FLT_MAX;
MaxLat 	= -FLT_MAX;
MinLat 	= FLT_MAX;

if (Importing->UTMZone != 0)
	{
	Importing->UTMZone = Info->Zone;
	UTMColInt = Info->Res[0];
	UTMRowInt = Info->Res[1];
	} // if
else if (Info->Zone != Importing->UTMZone)
	{
	UserMessageOK(MsgHdr,
		"7.5 Minute DEMs do not all lie within same UTM Zone!\nOperation terminated.");
	return 0;
	} // else if

// Corner order is defined as SWC first, then clockwise around polygon
// UTM[0] is SW, UTM[1] is NW, UTM[2] is NE, UTM[3] is SE
if (Info->UTM[2][0] > MaxEast[0])
	{
	MaxEast[0] = Info->UTM[2][0];
	MaxEast[1] = Info->UTM[2][1];
	}
if (Info->UTM[3][0] > MaxEast[0])
	{
	MaxEast[0] = Info->UTM[3][0];
	MaxEast[1] = Info->UTM[3][1];
	}

if (Info->UTM[0][0] < MinEast[0])
	{
	MinEast[0] = Info->UTM[0][0];
	MinEast[1] = Info->UTM[0][1];
	}
if (Info->UTM[1][0] < MinEast[0])
	{
	MinEast[0] = Info->UTM[1][0];
	MinEast[1] = Info->UTM[1][1];
	}

if (Info->UTM[1][1] > MaxNorth[0])
	{
	MaxNorth[0] = Info->UTM[1][1];
	MaxNorth[1] = Info->UTM[1][0];
	}
if (Info->UTM[2][1] > MaxNorth[0])
	{
	MaxNorth[0] = Info->UTM[2][1];
	MaxNorth[1] = Info->UTM[2][0];
	}

if (Info->UTM[0][1] < MinNorth[0])
	{
	MinNorth[0] = Info->UTM[0][1];
	MinNorth[1] = Info->UTM[0][0];
	}
if (Info->UTM[3][1] < MinNorth[0])
	{
	MinNorth[0] = Info->UTM[3][1];
	MinNorth[1] = Info->UTM[3][0];
	}

if (MaxNorth[0] > RUTMMaxNorthing)
	RUTMMaxNorthing = MaxNorth[0];
if (MinNorth[0] < RUTMMinNorthing)
	RUTMMinNorthing = MinNorth[0];
if (MaxEast[0] > RUTMMaxEasting)
	RUTMMaxEasting = MaxEast[0];
if (MinEast[0] < RUTMMinEasting)
	RUTMMinEasting = MinEast[0];

if (Info->LL[0][0] > MaxLon)
	MaxLon = Info->LL[0][0];
if (Info->LL[1][0] > MaxLon)
	MaxLon = Info->LL[1][0];

if (Info->LL[2][0] < MinLon)
	MinLon = Info->LL[2][0];
if (Info->LL[3][0] < MinLon)
	MinLon = Info->LL[3][0];

if (Info->LL[1][1] > MaxLat)
	MaxLat = Info->LL[1][1];
if (Info->LL[2][1] > MaxLat)
	MaxLat = Info->LL[2][1];

if (Info->LL[0][1] < MinLat)
	MinLat = Info->LL[0][1];
if (Info->LL[3][1] < MinLat)
	MinLat = Info->LL[3][1];

return 1;

} // ImportWizGUI::GetUTMBounds

/*===========================================================================*/

static double FirstCol, FirstRow, LLColInt, LLRowInt;
static long LLCols, LLRows, LLSize, UTMCols, UTMRows, UTMSize;
static float *LLData, *UTMData;
static struct USGS_DEMHeader USGSHdr;
static struct USGS_DEMProfileHeader USGSProfHdr;
static struct UTMLatLonCoords Convert;
static struct DEMInfo Info;
static FILE *DEMFile = NULL;

/*===========================================================================*/

// Given the UTM bounding box (SW/NW/NE/SE), figure out what the bounds of the postings will be
void ImportWizGUI::CalcUTMCorners(struct DEMInfo *UTMInfo, double *minx, double *maxx, double *miny, double *maxy)
{
double leftx, nwx, swx, xres, yres;
double xmax,xmin,ymax,ymin;	// for the local computations

swx = UTMInfo->UTM[0][0];
nwx = UTMInfo->UTM[1][0];
xres = UTMInfo->Res[0];
yres = UTMInfo->Res[1];		// should be same as xres
// assign leftx the minimum of SW X (those W of CM) or NW X (those E of CM)
if (swx <= nwx)
	leftx = swx;
else
	leftx = nwx;

if (leftx <= 500000)	// West of Central Meridian?
	{
	xmin = WCS_ceil(UTMInfo->UTM[0][0] / xres) * xres;		// sw x
	xmax = WCS_floor(UTMInfo->UTM[2][0] / xres) * xres;		// ne x
	ymin = WCS_ceil(UTMInfo->UTM[3][1] / yres) * yres;		// se y
	ymax = WCS_floor(UTMInfo->UTM[1][1] / yres) * yres;		// nw y
	} // if
else					// East of CM
	{
	xmin = WCS_ceil(UTMInfo->UTM[1][0] / xres) * xres;		// nw x
	xmax = WCS_floor(UTMInfo->UTM[3][0] / xres) * xres;		// se x
	ymin = WCS_ceil(UTMInfo->UTM[0][1] / yres) * yres;		// sw y
	ymax = WCS_floor(UTMInfo->UTM[2][1] / yres) * yres;		// ne y
	} // else

// now, adjust our region bounds if needed
if (xmin < *minx)
	*minx = xmin;
if (xmax > *maxx)
	*maxx = xmax;
if (ymin < *miny)
	*miny = ymin;
if (ymax > *maxy)
	*maxy = ymax;

} // ImportWizGUI::CalcUTMCorners

/*===========================================================================*/

// Extract 7.5 minute (10/30 meter) DEM
short ImportWizGUI::ExtractUTM_DEM(void)
{
double min_elev = DBL_MAX;
#ifdef WCS_BUILD_VNS
double xres, yres;	// should always be same
#endif // WCS_BUILD_VNS
double maxx, maxy, minx, miny;
double ElevDatum, ElevFactor, roundval;
BusyWin *BWRE = NULL;
long i, j, kutm, tmp;
short error = 0;
#ifdef EXTRUDING
float Elev;
double Slope;
short k, ColSign, RowSign;
long CornerPtX, CornerPtY, MapCtrX, MapCtrY, ColQuit, RowQuit;
#endif // EXTRUDING
float *MapPtr;
DEM Topo;
long Columns, LastCol = -1;
char elevbase[64];
long Col, Row, Rows;
//long i, j, Lr, Rows, Columns, lowrow, lowcol, MapRowSize,
//	Row, Col, LastCol = -1, MapCtrX, MapCtrY, CornerPtX, CornerPtY, RowQuit, ColQuit;
class Pier1 *ThisUTM;
char ReadMsg[60];
long fpos;
struct DEMInfo *UTMInfo;
long GaussCol, GaussRow;
#ifndef WCS_BUILD_VNS
long VoidCol, VoidRow;
#endif // !WCS_BUILD_VNS

if ((UTMInfo = (struct DEMInfo *)AppMem_Alloc
	(Num2Load * sizeof (struct DEMInfo), APPMEM_CLEAR)) == NULL)
	{
	UserMessageOK(MsgHdr, "Out of memory allocating DEM Info Header! Operation terminated.");
	goto EndPhase1;
	} // if

maxx = maxy = -FLT_MAX;
minx = miny = FLT_MAX;
kutm = 0;
UTMInfo[0].Code = Info.Code;
UTMInfo[0].HUnits = Info.HUnits;
UTMInfo[0].VUnits = Info.VUnits;
UTMInfo[0].Res[0] = Info.Res[0];
UTMInfo[0].Res[1] = Info.Res[1];
UTMInfo[0].Res[2] = Info.Res[2];
UTMInfo[0].UTM[0][0] = Info.UTM[0][0];
UTMInfo[0].UTM[0][1] = Info.UTM[0][1];
UTMInfo[0].UTM[1][0] = Info.UTM[1][0];
UTMInfo[0].UTM[1][1] = Info.UTM[1][1];
UTMInfo[0].UTM[2][0] = Info.UTM[2][0];
UTMInfo[0].UTM[2][1] = Info.UTM[2][1];
UTMInfo[0].UTM[3][0] = Info.UTM[3][0];
UTMInfo[0].UTM[3][1] = Info.UTM[3][1];
UTMInfo[0].LL[0][0] = Info.LL[0][0];
UTMInfo[0].LL[0][1] = Info.LL[0][1];
UTMInfo[0].LL[1][0] = Info.LL[1][0];
UTMInfo[0].LL[1][1] = Info.LL[1][1];
UTMInfo[0].LL[2][0] = Info.LL[2][0];
UTMInfo[0].LL[2][1] = Info.LL[2][1];
UTMInfo[0].LL[3][0] = Info.LL[3][0];
UTMInfo[0].LL[3][1] = Info.LL[3][1];

ThisUTM = Importing;

/*** Arc records the DEM bounds based on the center of the posting, instead of the posting coverage.
See note from Zack regarding this.  Need to adjust bounds by 1/2 grid spacing in each direction to compensate ***/
if (Importing->Signal == ARC_VARIANT)
	{
	MinEast[0] -= UTMColInt / 2.0;
	MaxEast[0] += UTMColInt / 2.0;
	MinNorth[0] -= UTMRowInt / 2.0;
	MaxNorth[0] += UTMRowInt / 2.0;
	} // if

tmp = 1 + (long)(RUTMMinEasting / UTMColInt);
FirstCol = tmp * UTMColInt;
UTMCols = 1 + (long)(fabs((RUTMMaxEasting - FirstCol) / UTMColInt)); 
tmp = 1 + (long)(RUTMMinNorthing / UTMRowInt);
FirstRow = tmp * UTMRowInt;
UTMRows = 1 + (long)(fabs((RUTMMaxNorthing - FirstRow) / UTMRowInt));

// derive the output intervals from the input intervals
LLRowInt = UTMRowInt / (1000.0 * EARTHLATSCALE);

LLColInt = UTMColInt / (1000.0 * (EARTHLATSCALE * cos(PiOver180 * (RUTMBoundN + RUTMBoundS) / 2.0)));

LLRows = (long)(fabs((RUTMBoundN - RUTMBoundS) / LLRowInt));
LLCols = (long)(fabs((RUTMBoundW - RUTMBoundE) / LLColInt));

Importing->InRows = LLRows;
Importing->InCols = LLCols;
Importing->OutRows = LLRows;
Importing->OutCols = LLCols;

// recompute the output intervals based on the total spread of data
LLRowInt = (RUTMBoundN - RUTMBoundS) / (LLRows - 1);
LLColInt = (RUTMBoundW - RUTMBoundE) / (LLCols - 1);

SetGridSpacing();

if (Importing->TestOnly)	// We have all the GeoRef info
	return 0;

UTMSize = UTMRows * UTMCols * sizeof (float);
LLSize = LLRows * LLCols * sizeof (float);

UTMData = (float *)AppMem_Alloc(UTMSize, 0);
LLData = (float *)AppMem_Alloc(LLSize, APPMEM_CLEAR);

if (! UTMData || ! LLData)
	{
	UserMessageOK(MsgHdr, "Out of memory allocating DEM Arrays! Operation terminated.");
	goto EndPhase1;
	} // if

USGSHdr.voidvalue = -32767;
USGSHdr.fillvalue = -32767;
MapPtr = UTMData;
for (i = 0; i < UTMCols; i++)
	{
	for (j = 0; j < UTMRows; j++, MapPtr++)
		*MapPtr = (float)USGSHdr.voidvalue;
	} // for i

fpos = ftell(DEMFile);

do
	{
	CalcUTMCorners(UTMInfo, &minx, &maxx, &miny, &maxy);
	if (atof(USGSHdr.ElMin) < min_elev)
		min_elev = atof(USGSHdr.ElMin);
	Columns = atoi(USGSHdr.Columns);
	LastCol = -1;
	if (UTMInfo[kutm].VUnits == 1)
		ElevFactor = ELSCALE_FEET / ELSCALE_METERS;
	else
		ElevFactor = 1.0;
	ElevFactor *= UTMInfo[kutm].Res[2];
	Importing->VScale = ElevFactor;

	sprintf(ReadMsg, "Reading '%s'", ThisUTM->InFile);
	BWRE = new BusyWin(ReadMsg, Columns, 'BWRE', 0);

	if (ThisUTM->Signal == ARC_VARIANT)
		fseek(DEMFile, ThisUTM->Flags, SEEK_SET);		// I probably shouldn't store the seek pos in Flags, but...
	else
		// Changes to handle LT4X files - USGS added more fields!
		{
		char TempPadBuf[30], IsPad = 1, PadSearch;

		fseek(DEMFile, 1000, SEEK_SET);
		fread(TempPadBuf, 1, 23, DEMFile);
		for (PadSearch = 0; PadSearch < 23; PadSearch++)
			{
			if (TempPadBuf[PadSearch] != ' ')
				{
				IsPad = 0;
				break;
				} // if
			} // if
		if (IsPad)
			{
			fseek(DEMFile, 1024, SEEK_SET);
			} // if
		else
			{
			fseek(DEMFile, 893, SEEK_SET);
			} // else
		} // temp scope

	for (i = 0; i < Columns; i++)
		{
		if (! Read_USGSProfHeader(DEMFile, &USGSProfHdr))
			{
			fclose(DEMFile);
			UserMessageOK(MsgHdr, "Can't read DEM profile header!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			error = 6;
			break;
			} // if read header failed
		fpos = ftell(DEMFile);
		if (atoi(USGSProfHdr.Column) != i + 1)
			{
			UserMessageOK(MsgHdr, "Error reading DEM profile header!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			error = 6;
			break;
			} // if read header failed

		Rows = atoi(USGSProfHdr.ProfRows);

//		Col = (long)((FCvt(ProfHdr.Coords[0]) +.1 - FirstCol)
//		Col = (long)((FCvt(ProfHdr.Coords[0]) +.5 - FirstCol)
//		Col = (long)((FCvt(ProfHdr.Coords[0]) +.0000001 - FirstCol)
		roundval = UTMRowInt / 2;	// round to nearest posting interval
		Col = (long)((FCvt(USGSProfHdr.Coords[0]) + roundval - FirstCol) / UTMColInt);
		if (LastCol >= 0 && Col != LastCol + 1)
			{
			Col = LastCol + 1;
			} // if
		LastCol = Col;
//		Row = (long)((FCvt(ProfHdr.Coords[1]) +.5 - FirstRow)
//		Row = (long)((FCvt(ProfHdr.Coords[1]) +.0000001 - FirstRow)
		roundval = UTMRowInt / 2;
		Row = (long)((FCvt(USGSProfHdr.Coords[1]) + roundval - FirstRow) / UTMRowInt);
		if (Row > UTMRows)	// SDTS2DEM occasionally writes profiles with invalid origins!
			{
			UserMessageOK(MsgHdr, "Invalid DEM profile Origin!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			error = 6;
			break;
			}
		ElevDatum = (double)(FCvt(USGSProfHdr.ElDatum));
		if (UTMInfo[kutm].VUnits == 1)
			ElevDatum *= (ELSCALE_FEET / ELSCALE_METERS);

		if (Row < 0 || Col < 0)
			{
			UserMessageOK(MsgHdr, "Error reading DEM profile header!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			error = 6;
			break;
			}
		// avoids a crash if Col & Row are too big
		if ((Col * UTMRows + Row) < (UTMCols * UTMRows))
			{
			double ElMax, ElMin;
			ElMax = atof(USGSProfHdr.ElMax);// / ElevFactor;
			ElMin = atof(USGSProfHdr.ElMin);// / ElevFactor;
			Read_USGSDEMProfile(DEMFile, &UTMData[Col * UTMRows + Row], (short)Rows, ElevFactor, ElevDatum, ElMax, ElMin);
			} // if

		if (BWRE)
			{
			BWRE->Update(i + 1);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if
		} // for i
	fclose (DEMFile);

	// select the next UTM DEM
	do
		{
		ThisUTM = ThisUTM->Next;
		} while (ThisUTM && !ThisUTM->HasUTM);

	if (ThisUTM)
		{
		kutm++;
		fclose(DEMFile);
		if ((DEMFile = PROJ_fopen(ThisUTM->LoadName, "rb")) == NULL)
			{
			UserMessageOK(MsgHdr, "Warning\nCan't open file!");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, ThisUTM->LoadName);
			goto EndPhase1;
			} // if

		if (! Read_USGSHeader(DEMFile, &USGSHdr))
			{
			UserMessageOK(MsgHdr, "Can't read DEM file header!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, ThisUTM->LoadName);
			goto EndPhase1;
			} // if read header failed

		// Read X, Y, Z resolution (Units of measure come from GrUnits for X & Y, ElUnits for Z)
		for (i=0; i<3; i++)
			UTMInfo[kutm].Res[i] = FCvt(USGSHdr.Resolution[i]);

		UTMLatLonCoords_Init(&Convert, ThisUTM->UTMZone);
		// Convert Northings & Eastings to Lat & Lon
		for (i = 0; i < 4; i++)
			{
			UTMInfo[kutm].UTM[i][0] = FCvt(USGSHdr.Coords[i][0]);
			UTMInfo[kutm].UTM[i][1] = FCvt(USGSHdr.Coords[i][1]);
			Convert.East = UTMInfo[kutm].UTM[i][0];
			Convert.North = UTMInfo[kutm].UTM[i][1];
			UTM_LatLon(&Convert);
			UTMInfo[kutm].LL[i][0] = Convert.Lon;
			UTMInfo[kutm].LL[i][1] = Convert.Lat;
			} // for
		ThisUTM->NBound = UTMInfo[kutm].LL[0][1];	// Set initial Bounds to an edge
		ThisUTM->SBound = UTMInfo[kutm].LL[0][1];
		ThisUTM->WBound = UTMInfo[kutm].LL[0][0];
		ThisUTM->EBound = UTMInfo[kutm].LL[0][0];
		for (i = 1; i < 4; i++)
			{
			if (UTMInfo[kutm].LL[i][1] > ThisUTM->NBound)
				ThisUTM->NBound = UTMInfo[kutm].LL[i][1];
			if (UTMInfo[kutm].LL[i][1] < ThisUTM->SBound)
				ThisUTM->SBound = UTMInfo[kutm].LL[i][1];
			if (UTMInfo[kutm].LL[i][0] > ThisUTM->WBound)
				ThisUTM->WBound = UTMInfo[kutm].LL[i][0];
			if (UTMInfo[kutm].LL[i][0] < ThisUTM->EBound)
				ThisUTM->EBound = UTMInfo[kutm].LL[i][0];
			} // for
		UTMInfo[kutm].Code = (short)atoi(USGSHdr.RefSysCode);
		UTMInfo[kutm].Zone = (short)atoi(USGSHdr.Zone);
		UTMInfo[kutm].HUnits = (short)atoi(USGSHdr.GrUnits);
		UTMInfo[kutm].VUnits = (short)atoi(USGSHdr.ElUnits);

		CalcUTMCorners(&UTMInfo[kutm], &minx, &maxx, &miny, &maxy);

		if (! GetUTMBounds(&UTMInfo[kutm]))
			goto EndPhase1;

		if ((UTMInfo[kutm].Code != 0) && (UTMInfo[kutm].Code != 1))
			{
			if (UTMInfo[kutm].Code == 2)
				UserMessageOK(MsgHdr, "Unsupported planimetric system (State plane).  Only Geographic & UTM supported.");
			else
				UserMessageOK(MsgHdr, "Unsupported planimetric system.  Only Geographic & UTM supported.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->NameBase);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Not a UTM or Geographic referenced file");
			goto EndPhase1;
			} // if
		} // if ThisUTM

	if (BWRE) delete BWRE;
	BWRE = NULL;

	} while (ThisUTM);	// keep processing as long as we have another UTM DEM

if (error)
	goto EndPhase1;


// Scan for missing data cells in the inner region.  Use Guassian weighted filter of valid cells to fix.
// Since we're going to fix this in situ, "fixed" data will be written with a bias so we can distinguish it
// from unfixed data.  After all cells are fixed, a 2nd pass to remove the bias is made.
BWRE = new BusyWin("Blank Patch 1a", UTMRows - 4, 'BWRE', 0);
for (GaussRow = 2; GaussRow < (UTMRows - 2); GaussRow++)
	{
	for (GaussCol = 2; GaussCol < (UTMCols - 2); GaussCol++)
		{
		// look for void or fill
		if (*(UTMData + GaussCol * UTMRows + GaussRow) == -32767.0f)
			{
			*(UTMData + GaussCol * UTMRows + GaussRow) =
				GaussFix(UTMData, UTMRows, GaussCol, GaussRow);
			} // if
		} // for GaussCol

	if (BWRE)
		{
		BWRE->Update(GaussRow - 2);
		if (BWRE->CheckAbort())
			{
			error = 50;
			break;
			} // if
		} // if

	} // for GaussRow

if (BWRE) delete BWRE;
BWRE = NULL;

// now remove the bias
BWRE = new BusyWin("Blank Patch 1b", UTMRows - 4, 'BWRE', 0);
for (GaussRow = 2; GaussRow < (UTMRows - 2); GaussRow++)
	{
	for (GaussCol = 2; GaussCol < (UTMCols - 2); GaussCol++)
		{
		if (*(UTMData + GaussCol * UTMRows + GaussRow) < -32767.0f) // modified data
			{
			*(UTMData + GaussCol * UTMRows + GaussRow) =
				-40000.0f - *(UTMData + GaussCol * UTMRows + GaussRow);
			} // if
		} // for GaussCol

	if (BWRE)
		{
		BWRE->Update(GaussRow - 2);
		if (BWRE->CheckAbort())
			{
			error = 50;
			break;
			} // if
		} // if

	} // for GaussRow

if (BWRE) delete BWRE;
BWRE = NULL;


#ifndef WCS_BUILD_VNS
#ifndef EXTRUDING

	// change voids to minimum elevation
	BWRE = new BusyWin("Void to minimum", UTMRows, 'BWRE', 0);
	for (VoidRow = 0; VoidRow < UTMRows; VoidRow++)
		{
		for (VoidCol = 0; VoidCol < UTMCols; VoidCol++)
			{
			if (*(UTMData + VoidCol * UTMRows + VoidRow) <= -32767.0f) // void data
				{
				*(UTMData + VoidCol * UTMRows + VoidRow) = (float)(min_elev * ElevFactor);
				} // if
			} // for VoidCol

		if (BWRE)
		  	{
			BWRE->Update(VoidRow);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if

		} // for VoidRow

	if (BWRE) delete BWRE;
	BWRE = NULL;

	if (error)
		goto EndPhase1;

#endif // !EXTRUDING
#endif // ! WCS_BUILD_VNS
#ifdef EXTRUDING

// fill any missing cells with adjacent elevations (by "extrusion")
BWRE = new BusyWin("Blank Patch 2", 4, 'BWRE', 0);

MapCtrX = UTMCols / 2;  
MapCtrY = UTMRows / 2;  

for (k=0; k<4; k++)
	{
	switch (k)
		{
		case 0:
			{
			CornerPtX	= 0;
			CornerPtY	= 0;
			RowSign	= -1;
			ColSign	= -1;
			RowQuit	= -1;
			ColQuit	= -1;
			break;
			} // SouthWest
		case 1:
			{
			CornerPtX 	= 0;
			CornerPtY	= UTMRows - 1;
			RowSign	= +1;
			ColSign	= -1;
			RowQuit	= UTMRows;
			ColQuit	= -1;
			break;
			} // NorthWest
		case 2:
			{
			CornerPtX 	= UTMCols - 1;
			CornerPtY	= UTMRows - 1;
			RowSign	= +1;
			ColSign	= +1;
			RowQuit	= UTMRows;
			ColQuit	= UTMCols;
			break;
			} // NorthEast
		case 3:
			{
			CornerPtX 	= UTMCols - 1;
			CornerPtY	= 0;
			RowSign	= -1;
			ColSign	= +1;
			RowQuit	= -1;
			ColQuit	= UTMCols;
			break;
			} // SouthEast
		default:
			break;
		} // switch

	Slope	= ((double)CornerPtY - MapCtrY) / ((double)CornerPtX - MapCtrX);

	Elev = UTMData[MapCtrX * UTMRows + MapCtrY];
	for (Row = MapCtrY, i = 0; Row != RowQuit; Row += RowSign, i += RowSign)
		{
		Col = (long)(MapCtrX + i / Slope);
		MapPtr = UTMData + Col * UTMRows + Row;
		while (Col != ColQuit)
			{
			if ((*MapPtr == (float)USGSHdr.voidvalue) || (*MapPtr == (float)USGSHdr.fillvalue))
				{
				*MapPtr = Elev;
				Col += ColSign;
				MapPtr += (ColSign * UTMRows);
				} // if
			else
				{
				Elev = *MapPtr;
				Col += ColSign;
				MapPtr += (ColSign * UTMRows);
				} // else
			} // while 
		} // for Row=...

	Elev = UTMData[MapCtrX * UTMRows + MapCtrY];
	for (Col = MapCtrX, i = 0; Col != ColQuit; Col += ColSign, i += ColSign)
		{
		Row = (long)(MapCtrY + i * Slope);
		MapPtr = UTMData + Col * UTMRows + Row;
		while (Row != RowQuit)
			{
			if ((*MapPtr == (float)USGSHdr.voidvalue) || (*MapPtr == (float)USGSHdr.fillvalue))
				{
				*MapPtr = Elev;
				Row += RowSign;
				MapPtr += RowSign;
				} // if
			else
				{
				Elev = *MapPtr;
				Row += RowSign;
				MapPtr += RowSign;
				} // else
			} // while 
		} // for Row=...

	if (BWRE)
		{
		BWRE->Update(k + 1);
		if (BWRE->CheckAbort())
			{
			error = 50;
			break;
			} // if
		} // if

	} // for k=0...

if (BWRE) delete BWRE;
BWRE = NULL;

if (error)
	goto EndPhase1;

#endif // EXTRUDING

#ifndef WCS_BUILD_VNS
// Resample UTM grid into Lat/Lon grid
BWRE = new BusyWin("Resample", LLCols, 'BWRE', 0);

Convert.Lon = RUTMBoundW;
for (i = 0; i < LLCols; i++, Convert.Lon -= LLColInt)
	{
	Convert.Lat = RUTMBoundS;
	for (j = 0; j < LLRows; j++, Convert.Lat += LLRowInt)
		{
		LatLon_UTM(&Convert, Importing->UTMZone);

		LLData[i * LLRows + j] = (float)(Point_Extract
			(Convert.East, Convert.North, FirstCol, FirstRow, UTMColInt, UTMRowInt, UTMData, UTMRows, UTMCols));
		} // for j
	if (BWRE)
		{
		BWRE->Update(i + 1);
		if (BWRE->CheckAbort())
			{
			error = 50;
			break;
			} // if
		} // if
	} // for i

if (BWRE) delete BWRE;
BWRE = NULL;

if (error)
	goto EndPhase1;

// 7.5 Minute file save
Topo.pLatEntries 	= LLRows;
Topo.pLonEntries 	= LLCols;
Topo.pLatStep 	= LLRowInt;
Topo.pLonStep 	= LLColInt;
Topo.pSouthEast.Lat = RUTMBoundS;
Topo.pNorthWest.Lon = RUTMBoundW;
Topo.pNorthWest.Lat = Topo.pSouthEast.Lat + (Topo.pLatEntries - 1) * Topo.pLatStep;
Topo.pSouthEast.Lon = Topo.pNorthWest.Lon - (Topo.pLonEntries - 1) * Topo.pLonStep;
Topo.pElScale = ELSCALE_METERS;
Topo.pElDatum = 0.0;
Topo.PrecalculateCommonFactors();

strncpy(elevbase, Importing->NameBase, 60);
elevbase[60] = 0;
(void)TrimTrailingSpaces(elevbase);

error = USGSDEMFile_Save(elevbase, &Topo, LLData, LLSize);
//error = DBAddUSGS_UTMDEM(elevbase, &Topo, LLData, LLSize);
#else // !WCS_BUILD_VNS
Importing = HeadPier;
Topo.pLatEntries 	= UTMRows;
Topo.pLonEntries 	= UTMCols;
Topo.pSouthEast.Lat	= miny;
Topo.pNorthWest.Lon	= minx;
Topo.pSouthEast.Lon	= maxx;
Topo.pNorthWest.Lat	= maxy;
xres = UTMInfo[0].Res[0];
yres = UTMInfo[0].Res[1];	// I really don't know why these would be different
Topo.pLatStep 	= yres;
Topo.pLonStep 	= -xres;
Topo.pElScale = ELSCALE_METERS;
Topo.pElDatum = 0.0;
Topo.PrecalculateCommonFactors();
Topo.SetNullReject(1);
Topo.SetNullValue((float)USGSHdr.voidvalue);

strncpy(elevbase, Importing->NameBase, 60);
elevbase[60] = 0;
(void)TrimTrailingSpaces(elevbase);

error = USGSDEMFile_Save(elevbase, &Topo, UTMData, UTMSize);
#endif // !WCS_BULD_VNS

EndPhase1:

if (UTMInfo)
	AppMem_Free(UTMInfo, Num2Load * sizeof (struct DEMInfo));
if (BWRE) delete BWRE;
BWRE = NULL;
if (UTMData)
	AppMem_Free(UTMData, UTMSize);
if (LLData)
	AppMem_Free(LLData, LLSize);
UTMData = LLData = NULL;
UTMSize = LLSize = 0;

if (error)
	goto EndExtract;

EndExtract:

if (Importing->FileParent)
	CloseUSGSDBGroup();

return error;

} // ImportWizGUI::ExtractUTM_DEM

/*===========================================================================*/

// Originally only for 1 Degree, hopefully accepts all four geographic types correctly now
short ImportWizGUI::ExtractGeo_DEM(void)
{
double ElevDatum, ElevFactor, LatSize, LonSize;
float *MapPtr = NULL, *TmpPtr = NULL;
BusyWin *BWRE;
char *dot;
long Columns, LastCol = -1, MapRowSize, Rows;
long lowcol, lowrow, Lr;
long i, j;
short error = 0;
DEM Topo;
size_t slen;
char elevbase[64], ExtChar[2], ReadMsg[60];

if (Importing->Signal == ARC_VARIANT)
	fseek(DEMFile, Importing->Flags, SEEK_SET);		// I probably shouldn't store the seek pos in Flags, but...
else
	fseek(DEMFile, 1024, SEEK_SET);	// make sure we're positioned correctly for the first profile

if (! Read_USGSProfHeader(DEMFile, &USGSProfHdr))
	{
	fclose(DEMFile);
	UserMessageOK(MsgHdr, "Can't read DEM profile header!\nOperation terminated.");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
	return 1;
	} /* if read header failed */
Rows = atoi(USGSProfHdr.ProfRows);
Columns = atoi(USGSHdr.Columns);

ElevDatum = (double)(FCvt(USGSProfHdr.ElDatum));
if (Info.VUnits == 1)
	{
	ElevFactor = ELSCALE_FEET / ELSCALE_METERS;
	ElevDatum *= (ELSCALE_FEET / ELSCALE_METERS);
	} // if
else
	{
	ElevFactor = 1.0;
	} // else
ElevFactor *= Info.Res[2];
Importing->VScale = ElevFactor;

Topo.pLonEntries = 1 + (Columns - 1) / 4;
Topo.pLatEntries = 1 + ((Rows - 1) / 4);
Topo.pElScale = ELSCALE_METERS;
Topo.pElDatum = 0.0;

/***
if ((Info.Res[0] == 3) && (Info.Res[1] == 2))		// Alaska 15'
	{
	LatSize = 0.25;
	if (Importing->SBound < 59.0)			// cell sizes vary by latitude
		LonSize = 20.0 / 60.0;				// convert minutes to degrees
	else if (Importing->SBound < 62.0)
		LonSize = 22.5 / 60.0;
	else if (Importing->SBound < 68.0)
		LonSize = 30.0 / 60.0;
	else
		LonSize = 36.0 / 60.0;
	}
else if ((Info.Res[0] == 2) && (Info.Res[1] == 1))	// Alaska 7.5'
	{
	LatSize = 0.125;
	if (Importing->SBound < 59.0)
		LonSize = 10.0 / 60.0;
	else if (Importing->SBound < 62.0)
		LonSize = 11.25 / 60.0;
	else if (Importing->SBound < 68.0)
		LonSize = 15.0 / 60.0;
	else
		LonSize = 18.0 / 60.0;
	}
else if ((Info.Res[0] == 1) && (Info.Res[1] == 1))	// future 7.5' series
	{
	LatSize = 0.125;
	LonSize = 0.125;
	}
else if ((Info.Res[0] == 2) && (Info.Res[2] == 2))	// 30'
	{
	LatSize = 0.5;
	LonSize = 0.5;
	}
else												// default back to 1 degree
	{
	LatSize = 1.0;
	LonSize = 1.0;
	}
***/
LonSize = Info.Res[0] * (Columns - 1) / 3600;
LatSize = Info.Res[1] * (Rows - 1) / 3600;

Topo.pLatStep 	= LatSize / (Rows - 1);
Topo.pLonStep 	= LonSize / (Columns - 1);
strncpy(elevbase, Importing->InFile, 60);
elevbase[60] = 0;
(void)TrimTrailingSpaces(elevbase);
if (dot = FindFileExtension(elevbase))
	*dot = 0;
slen = strlen(elevbase);
if ((slen > 1) && (elevbase[slen - 1] == '.'))
	elevbase[slen - 1] = 0;
strcpy(Importing->NameBase, elevbase);

HaveSWCorner = FALSE;
Importing->WBound = -atof(USGSHdr.Coords[0][0]) / 3600.0;	// coord in arc-seconds
Importing->SBound = atof(USGSHdr.Coords[0][1]) / 3600.0;
if ((Importing->WBound != 0.0) && (Importing->SBound != 0.0))
	{
	HaveSWCorner = TRUE;
	Importing->EBound = -atof(USGSHdr.Coords[2][0]) / 3600.0;
	Importing->NBound = atof(USGSHdr.Coords[2][1]) / 3600.0;
	} // if
else
	{
	if (! Match250FileName(Importing->NameBase, Importing->NameBase) &&
//	! GetSWCornerFromFiller(&USGSHdr) &&
	! GetSECornerFromUser())
		{
		fclose(DEMFile);
		return 50;	// user cancelled
		} // no georef

	Importing->NBound = Importing->SBound + LatSize;
	if (HaveSWCorner)
		Importing->EBound = Importing->WBound - LonSize;
	else
		Importing->WBound = Importing->EBound + LonSize;
	} // else

Importing->InRows = Rows;
Importing->InCols = Columns;

SetGridSpacing();

if (Importing->TestOnly)	// We have all the GeoRef info
	return 0;

UTMSize = Rows * Columns * sizeof (float);
if ((UTMData = (float *)AppMem_Alloc(UTMSize, 0)) == NULL)
	{
	fclose(DEMFile);
	UserMessageOK(MsgHdr, "Out of memory allocating temporary buffer! Operation terminated.");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
	return 1;
	} // if read header failed

sprintf(ReadMsg, "Reading '%s'", Importing->InFile);
BWRE = new BusyWin(ReadMsg, Columns, 'BWRE', 0);
for (i = 0; i < Columns; i++)
	{
	double ElMax, ElMin;
	ElMax = atof(USGSProfHdr.ElMax); // / ElevFactor;
	ElMin = atof(USGSProfHdr.ElMin); // / ElevFactor;
	TmpPtr = UTMData + i * Rows;
	if (! Read_USGSDEMProfile(DEMFile, TmpPtr, (short)Rows, ElevFactor, ElevDatum, ElMax, ElMin))
		{
		UserMessageOK(MsgHdr, "Error reading DEM profile!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
		if (BWRE) delete BWRE;
		return 1;
		} // if not read error
	if (i < Columns - 1)
		{
		if (! Read_USGSProfHeader(DEMFile, &USGSProfHdr))
			{
			UserMessageOK(MsgHdr, "Error reading DEM profile header!\nOperation terminated.");
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			if (BWRE) delete BWRE;
			return 1;
			} // if not read error
		if (atoi(USGSProfHdr.ProfRows) != Rows)
			{
			// MapMart DEMs may fail here.  This is because they may have variable length profiles in their geographic
			// DEMs, which is not allowed in the standard USGS DEMs.
			UserMessageOK(MsgHdr, "Improper DEM profile length!\nOperation terminated.");
				GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
			if (BWRE) delete BWRE;
			return 1;
			} // if unequal row lengths
		ElevDatum = (double)(FCvt(USGSProfHdr.ElDatum));
		if (Info.VUnits == 1)
			{
			ElevDatum *= (ELSCALE_FEET / ELSCALE_METERS);
			} // if
		} // if not last column
	if (BWRE)
		{
		BWRE->Update(i + 1);
		if (BWRE->CheckAbort())
			{
//				error = 50;
//				break;
			delete BWRE;
			return 1;
			} // if
		} // if
	} // for i
if (BWRE) delete BWRE;
BWRE = NULL;

fclose(DEMFile);

if (error)
	{
	if (UTMData) AppMem_Free(UTMData, UTMSize);
//			break;
//			return 1;
	return 1;
	} // if error reading DEM

// allocate small array to copy data to
LLCols 	= Topo.pLonEntries;
LLRows 	= Topo.pLatEntries;
LLSize 	= LLCols * LLRows * sizeof (float);
MapRowSize 		= LLRows * sizeof (float);
if ((LLData = (float *)AppMem_Alloc(LLSize, 0)) == NULL)
	{
	UserMessageOK(MsgHdr, "Out of memory allocating map buffer! Operation terminated.");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->InFile);
	if (UTMData)
		AppMem_Free(UTMData, UTMSize);
//		break;
	return 1;
	} // if read header failed

/*** F2 Note - should we use less tiles for DEM's that aren't 1 degree? ***/
// copy data one quad at a time and save
ExtChar[1] = 0;
for (i = 0; i < 4; i++)
	{
	for (j = 0; j < 4; j++)
		{
		lowcol = i * (LLCols - 1);
		lowrow = j * (LLRows - 1);
		MapPtr = LLData;
		TmpPtr = UTMData + lowcol * Rows + lowrow;

		for (Lr = 0; Lr < LLCols; Lr++)
			{
			memcpy(MapPtr, TmpPtr, MapRowSize);
			MapPtr += LLRows;
			TmpPtr += Rows;
			} // for Lr=0...

		// set file name and lat/long parameters

		Topo.pSouthEast.Lat = Importing->SBound + lowrow * Topo.pLatStep;
		Topo.pNorthWest.Lon = Importing->EBound + ((Columns - 1) - lowcol) * Topo.pLonStep;
		Topo.pNorthWest.Lat = Topo.pSouthEast.Lat + (Topo.pLatEntries - 1) * Topo.pLatStep;
		Topo.pSouthEast.Lon = Topo.pNorthWest.Lon - (Topo.pLonEntries - 1) * Topo.pLonStep;
		Topo.PrecalculateCommonFactors();

		strncpy(elevbase, Importing->NameBase, 60);
		elevbase[60] = 0;
		(void)TrimTrailingSpaces(elevbase);
		strcat(elevbase, ".");
		ExtChar[0] = (char)(65 + i * 4 + j);
		strcat(elevbase, ExtChar);

		error = USGSDEMFile_Save(elevbase, &Topo, LLData, LLSize);

		} // for j
	if (error)
//			break;
		return 1;

	} // for i

if (UTMData)
	AppMem_Free(UTMData, UTMSize);
if (LLData)
	AppMem_Free(LLData, LLSize);

if (error)
	{
	UserMessageOK(MsgHdr, "Error creating output file!\nOperation terminated.");
//		break;
	return 1;
	} // if file output error

CloseUSGSDBGroup();

return 0;

} // ImportWizGUI::ExtractGeo_DEM

/*===========================================================================*/

short ImportWizGUI::DBAddUSGS_UTMDEM(char *BaseName, DEM *Topo, float *MapArray, long MapSize)
{
float *destptr, *srcptr, *TileData;
long cols, rows, colmod, rowmod, firstcol, lastcol, firstrow, lastrow;
long ndx, ndx2, TileSize;
short error, i, j, ii, jj;
extern short DupRow;
char RGBComponent[5] = {0,0,0,0,0};

DupRow = 1;
Importing = HeadPier;
OUTPUT_COLMAPS = (short)((Topo->pLonEntries - 1) / 300.0 + 1.0);
OUTPUT_ROWMAPS = (short)((Topo->pLatEntries - 1) / 300.0 + 1.0);

OUTPUT_COLS = Topo->pLonEntries;
OUTPUT_ROWS = Topo->pLatEntries;
ELEV_UNITS = DEM_DATA_UNITS_METERS;

for (j = 0; j < OUTPUT_COLMAPS; j++)
	{
	for (i = 0; i < OUTPUT_ROWMAPS; i++)
		{
		cols = (long)Topo->pLonEntries / OUTPUT_COLMAPS;
		colmod = (long)Topo->pLonEntries % (OUTPUT_COLMAPS * cols);
		if (colmod != 0)
			cols++;
		if (j != (OUTPUT_COLMAPS - 1))
			firstcol = cols * j;
		else
			firstcol = Topo->pLonEntries - cols;
		lastcol = firstcol + cols - 1;

		rows = (long)Topo->pLatEntries / OUTPUT_ROWMAPS;
		rowmod = (long)Topo->pLatEntries % (OUTPUT_ROWMAPS * rows);
		if (rowmod != 0)
			rows++;
		if (i != (OUTPUT_ROWMAPS - 1))
			firstrow = rows * i;
		else
			firstrow = Topo->pLatEntries - rows;
		lastrow = firstrow + rows - 1;

		TileSize = cols * rows * sizeof(float);
		if ((TileData = (float *)AppMem_Alloc(TileSize, 0)) == NULL)
			{
			return 1;
			} // if
		destptr = TileData;
		srcptr = MapArray;
		OUTPUT_HILAT = RUTMBoundN;
		OUTPUT_LOLAT = RUTMBoundS;
		OUTPUT_HILON = RUTMBoundW;
		OUTPUT_LOLON = RUTMBoundE;

		for (jj = 0; jj < rows; jj++)
			{
			for (ii = 0; ii < cols; ii++)
				{
				ndx = (firstcol + ii) * Topo->pLatEntries + firstrow + jj;
				ndx2 = ii * rows + jj;
				destptr[ndx2] = srcptr[ndx];
				} // for
			} // for

		ARNIE_SWAP(OUTPUT_COLMAPS, OUTPUT_ROWMAPS);
		error = DataOpsSaveOutput(TileData, TileSize, j, i, rows, cols, rows, cols, RGBComponent);
		ARNIE_SWAP(OUTPUT_COLMAPS, OUTPUT_ROWMAPS);
		AppMem_Free(TileData, TileSize);
		} // for
	} // for

Topo->FreeRawElevs();

return (0);

} // ImportWizGUI::DBAddUSGS_UTMDEM

/*===========================================================================*/

short ImportWizGUI::USGSDEMFile_Save(char *BaseName, DEM *Topo, float *MapArray, long MapSize)
{
Joe *Clip, *Added = NULL;
LayerEntry *LE = NULL;
long Ct, MaxCt = (long)Topo->MapSize();
short Found = 0;
//JoeDEM *MyDEM;
//NotifyTag ChangeEvent[2];
char filename[256], FileBase[256];//, NameStr[256];

strcpy(FileBase, BaseName);
strcat(FileBase, ".elev");
strmfp(filename, GlobalApp->MainProj->dirname, FileBase);

if (Topo->AllocRawMap())
	{
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		Topo->RawMap[Ct] = MapArray[Ct];
		} // for
	} // if

#ifdef WCS_BUILD_VNS
if (Importing->HasNulls)
	{
	Topo->SetNullReject(1);
	Topo->SetNullValue((float)Importing->NullVal);
	} // if
#endif // WCS_BUILD_VNS
Topo->FindElMaxMin();	// added by GRH 8/6/01

#ifdef WCS_BUILD_VNS
Clip = DBHost->AddDEMToDatabase("Import Wizard", BaseName, Topo, NewCoordSys, ProjHost, EffectsHost);	// added by GRH 8/6/01
#else // WCS_BUILD_VNS
Clip = DBHost->AddDEMToDatabase("Import Wizard", BaseName, Topo, NULL, ProjHost, EffectsHost);	// added by GRH 8/6/01
#endif // WCS_BUILD_VNS

if (! Topo->SaveDEM(filename, GlobalApp->StatusLog))
	{
	Topo->FreeRawElevs();
	return (5);
	} // if
Topo->FreeRawElevs();
GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DEM_SAVE, FileBase);

return (0);

} // ImportWizGUI::DEMFile_Save()

/*===========================================================================*/

// Logical Record Type A
short ImportWizGUI::Read_USGSHeader(FILE *DEM, struct USGS_DEMHeader *Hdr)
{
bool HaveSomething = false, ShortRec = false;
bool CDED = false;
short i, j, k, namelen, val;
unsigned char startch = 0;
char RecordTypeA[1028];

memset(RecordTypeA, 0, 1028);	// clear the array
if (!fgetline(RecordTypeA, 1025, DEM))	// we want to read a 1K block, so need to ask for n+1
	return 0;

// copy the original 16 data elements - all offsets are 1 less than the starting byte listed in the USGS booklet (0 vs 1 based numbering)
strncpy(Hdr->FileName, &RecordTypeA[0], 144);
strncpy(Hdr->LevelCode, &RecordTypeA[144], 6);
strncpy(Hdr->ElPattern, &RecordTypeA[150], 6);
strncpy(Hdr->RefSysCode, &RecordTypeA[156], 6);
strncpy(Hdr->Zone, &RecordTypeA[162], 6);
for (i = 0, k = 0; i < 15; i++, k += 24)
	strncpy(Hdr->ProjPar[i], &RecordTypeA[168 + k], 24);
strncpy(Hdr->GrUnits, &RecordTypeA[528], 6);
strncpy(Hdr->ElUnits, &RecordTypeA[534], 6);
strncpy(Hdr->PolySides, &RecordTypeA[540], 6);
for (i = 0, k = 0; i < 4; i++)
	for (j = 0; j < 2; j++, k += 24)
		strncpy(Hdr->Coords[i][j], &RecordTypeA[546 + k], 24);
strncpy(Hdr->ElMin, &RecordTypeA[738], 24);
strncpy(Hdr->ElMax, &RecordTypeA[762], 24);
strncpy(Hdr->AxisRot, &RecordTypeA[786], 24);
strncpy(Hdr->Accuracy, &RecordTypeA[810], 6);
for (i = 0, k = 0; i < 3; i++, k += 12)
	strncpy(Hdr->Resolution[i], &RecordTypeA[816 + k], 12);
strncpy(Hdr->Rows, &RecordTypeA[852], 6);
strncpy(Hdr->Columns, &RecordTypeA[858], 6);

// now we want the datums, but we have 3 possibilities to deal with for the datum fields:
//	1) They're blank (these fields were undefined in old format)
//	2) They're set (new format DEMs)
//	3) They're missing due to the ESRI LATTICEDEM short record mess
// So, we'll scan for a short record
for (i = 865; i < 892; i++)
	{
	if (RecordTypeA[i] == 0x0D)
		{
		ShortRec = true;
		break;
		} // if
	} // for

if (!ShortRec)
	{
	strncpy(Hdr->VDatum, &RecordTypeA[888], 2);
	strncpy(Hdr->HDatum, &RecordTypeA[890], 2);
	} // if

// try to detect CDED data - xref CDED Product Specs Edition 1.0 & 2.0
if (!ShortRec)
	{
	if ((strnicmp(&RecordTypeA[140], "NTDB", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "BC  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "MB  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "NB  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "NL  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "NS  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "NT  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "NU  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "ON  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "PE  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "AB  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "QC  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "SK  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "YT  ", 4) == 0) ||
		(strnicmp(&RecordTypeA[140], "MULT", 4) == 0))
		{
		CDED = true;	// may want to do more in the future if CDED is detected
		Importing->HasNulls = TRUE;
		Importing->NullVal = -32767.0;
		} // if
	} // if

/***
if ((! fgetline(Hdr->FileName,	 145, DEM)) ||
	(! fgetline(Hdr->LevelCode,   7, DEM)) ||
	(! fgetline(Hdr->ElPattern,   7, DEM)) ||
	(! fgetline(Hdr->RefSysCode,  7, DEM)) ||
	(! fgetline(Hdr->Zone,		   7, DEM)))
	return(0);
for (i=0; i<15; i++)
	{
	if (! fgetline(Hdr->ProjPar[i], 25, DEM))
		return(0);
	} // for i=0...
if ((! fgetline(Hdr->GrUnits,	 7, DEM)) ||
	(! fgetline(Hdr->ElUnits,	 7, DEM)) ||
	(! fgetline(Hdr->PolySides, 7, DEM)))
	return(0);
for (i=0; i<4; i++)
	{
	for (j=0; j<2; j++)
		{
		if (! fgetline(Hdr->Coords[i][j],	 25, DEM))
			return(0);
		} // for j=0...
	} // for i=0...
if ((! fgetline(Hdr->ElMin,   25, DEM)) ||
	(! fgetline(Hdr->ElMax,   25, DEM)) ||
	(! fgetline(Hdr->AxisRot, 25, DEM)) ||
	(! fgetline(Hdr->Accuracy, 7, DEM)))
	return(0);
for (i=0; i<3; i++)
	{
	if (! fgetline(Hdr->Resolution[i],	13, DEM))
		return(0);
	} // for i=0...
if ((! fgetline(Hdr->Rows,		7,	DEM)) ||
	(! fgetline(Hdr->Columns,	7,	DEM)))
	return (0);
***/
// we've read all 16 fields of the old format at this point

if (atoi(Hdr->RefSysCode) == 1)
	{
	Importing->HasUTM = TRUE;
	Importing->UTMZone = (short)atoi(Hdr->Zone);
	} // if

if (Importing->TestOnly)
	{
	// copy DEM TITLE up to comma, 79 chars max
	namelen = (short)strcspn(Hdr->FileName, ",");
	if (namelen > 79)
		namelen = 79;
	// since some idiots put the path & other garbage in the file name, try to correct that
	for (unsigned char ii = 0; ii < 79; ii++)
		{
		if (Hdr->FileName[ii] == '\\')
			startch = ii + 1;
		else if (Hdr->FileName[ii] == ' ')
			{
			if (HaveSomething)	// don't terminate the string if it starts with blanks (stupid Canadians!)
				{
				// if we get double spaces, terminate the string and break outta here
				if (ii < 79)
					{
					if (Hdr->FileName[ii + 1] == ' ')
						{
						Hdr->FileName[ii] = 0;
						break;
						} // if
					} // if
				} // if
			} // else if
		else
			{
			if ((!HaveSomething) && (startch == 0))
				startch = ii;	// will skip leading spaces, but not mess with startch if already adjusted
			HaveSomething = true;
			}
		} // for
	strncpy(Importing->NameBase, &Hdr->FileName[startch], namelen - startch);
	Importing->NameBase[namelen] = 0;
	} // if

Importing->TestMin = atof(Hdr->ElMin);
Importing->TestMax = atof(Hdr->ElMax);
val = (short)atoi(Hdr->ElUnits);
if (val == 1)
	Importing->ElevUnits = DEM_DATA_UNITS_FEET;
else
	Importing->ElevUnits = DEM_DATA_UNITS_METERS;
return (1);

} // ImportWizGUI::Read_USGSHeader()

/*===========================================================================*/

// Logical Record Type B
short ImportWizGUI::Read_USGSProfHeader(FILE *DEM, struct USGS_DEMProfileHeader *ProfHdr)
{
#ifdef USGS_SEEKING
long fpos;
#endif
//short value = 0;

#ifdef USGS_SEEKING
fpos = ftell(DEM);
#endif

if (fscanf(DEM, "%6c", ProfHdr->Row) != 1)
	return 0;
ProfHdr->Row[6] = 0;

if (fscanf(DEM, "%6c", ProfHdr->Column) != 1)
	return 0;
ProfHdr->Column[6] = 0;

if (fscanf(DEM, "%6c", ProfHdr->ProfRows) != 1)
	return 0;
ProfHdr->ProfRows[6] = 0;

if (fscanf(DEM, "%6c", ProfHdr->ProfCols) != 1)
	return 0;
ProfHdr->ProfCols[6] = 0;

if (fscanf(DEM, "%24c", ProfHdr->Coords[0]) != 1)
	return 0;
ProfHdr->Coords[0][24] = 0;

if (fscanf(DEM, "%24c", ProfHdr->Coords[1]) != 1)
	return 0;
ProfHdr->Coords[1][24] = 0;

if (fscanf(DEM, "%24c", ProfHdr->ElDatum) != 1)
	return 0;
ProfHdr->ElDatum[24] = 0;

if (fscanf(DEM, "%24c", ProfHdr->ElMin) != 1)
	return 0;
ProfHdr->ElMin[24] = 0;

if (fscanf(DEM, "%24c", ProfHdr->ElMax) != 1)
	return 0;
ProfHdr->ElMax[24] = 0;

return (1);

/*** original code
while (! value)
	{
	if (! fscanf(DEM, " %7s", ProfHdr->Row))
		return (0);
	value = (short)atoi(ProfHdr->Row);
	}
fscanf(DEM, " %7s", ProfHdr->Column);
fscanf(DEM, " %7s", ProfHdr->ProfRows);
fscanf(DEM, " %7s", ProfHdr->ProfCols);
fscanf(DEM, " %25s", ProfHdr->Coords[0]);
fscanf(DEM, " %25s", ProfHdr->Coords[1]);
fscanf(DEM, " %25s", ProfHdr->ElDatum);
fscanf(DEM, " %25s", ProfHdr->ElMin);
if (fscanf(DEM, " %25s", ProfHdr->ElMax) < 1)
	return (0);

return (1);

***/

} // ImportWizGUI::Read_USGSProfHeader

/*===========================================================================*/

short ImportWizGUI::Read_USGSDEMProfile(FILE *DEM, float *ProfPtr, short ProfItems, double ElevFactor, double ElevDatum, double ElMax, double ElMin)
{
double value;
#ifdef USGS_SEEKING
long fpos;
#endif
long have = 144, need;	// we've already read 144 characters in this record when we processed the profile header
short count = 0, error = 0, i, skip = 146;	// only 146 elevations can be read before we need to skip 4 characters
char text[8];

text[6] = 0;

for (i = 0; i < ProfItems; i++)
	{
#ifdef USGS_SEEKING
	fpos = ftell(DEM);
#endif
	if ((fscanf(DEM, "%6c", text)) != 1)
		{
		error = 1;
		break;
		} // if
	value = atof(text);
	/***
	if ((value >= ElMin) && (value <= ElMax))
		*ProfPtr++ = (float)(ElevDatum + (value * ElevFactor));
	else
		*ProfPtr++ = -32767.0f;
	***/
	if (value < -30000.0)
		*ProfPtr++ = -32767.0f;
	else
		*ProfPtr++ = (float)(ElevDatum + (value * ElevFactor));
	count++;
	have += 6;
	if (count == skip)
		{
		int ch;
		skip = 170;	// all subsequent reads in the record
		count = 0;
		// now handle CR/LF pair or read 4 characters to put us back on a 1024 byte boundary
		ch = fgetc(DEM);
		if (ch == 0x0d)
			{
			ch = fgetc(DEM);	// assuming CR/LF pair for now
			have += 2;
			if (i == ProfItems - 1)
				have = 1024;
			} // if
		else
			{
			fscanf(DEM, "%3c", text);
			have += 4;
			} // else
		} // if
	} // for i

// data is aligned to 1024 byte blocks.  Need to reposition ourselves for next record
if ((have % 1024) != 0)
	{
	need = 1024 - (have % 1024);
	while (need)
		{
		int ch;
		ch = fgetc(DEM);
		need--;
		if (ch == 0x0d)
			{
			ch = fgetc(DEM);	// assuming CR/LF pair for now
			have = 1024;
			need = 0;
			} // if
		} // while
	} // if

/*** original code
for (i = 0; i < ProfItems; i++)
	{
#ifdef USGS_SEEKING
	fpos = ftell(DEM);
#endif
	if ((fscanf(DEM, "%d", &value)) != 1)
		{
#ifdef USGS_SEEKING
		fpos = ftell(DEM);
#endif
		if ((fscanf(DEM, "%d", &value)) != 1)
			{
#ifdef USGS_SEEKING
			fpos = ftell(DEM);
#endif
			if ((fscanf(DEM, "%d", &value)) != 1)
				{
#ifdef USGS_SEEKING
				fpos = ftell(DEM);
#endif
				error = 1;
				break;
				} // if - keep trying until a value is read
			} // if - keep trying until a value is read
		} // if - keep trying until a value is read
	*ProfPtr = (float)(ElevDatum + (value * ElevFactor));
	ProfPtr ++;
	} // for i=0...
***/

if (error)
	return (0);

return (1);

} // ImportWizGUI::Read_USGSDEMProfile

/*===========================================================================*/

short ImportWizGUI::LoadUSGS_DEM(char *filename, float *Output, char TestOnly)
{
short error = 1, i;
BOOL  FoundUTM = FALSE, ZoneMixErr = FALSE;
#ifdef WCS_BUILD_VNS
long zonenum;	// gets mangled
char zmsg[256];
#endif // WCS_BUILD_VNS

do
	{
	if ((DEMFile = PROJ_fopen(Importing->LoadName, "rb")) == NULL)
		{
		UserMessageOK(MsgHdr, "Warning\nCan't open file!");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->LoadName);
		goto Cleanup;
		}

	// set up an automatically allocated 16k read buffer
	setvbuf(DEMFile, NULL, _IOFBF, 16384);

	Importing->AllowRef00 = FALSE;
	Importing->AllowPosE = FALSE;
	Importing->AskNull = FALSE;
	Importing->TestOnly = TestOnly;

	if (! Read_USGSHeader(DEMFile, &USGSHdr))
		{
		UserMessageOK(MsgHdr, "Can't read DEM file header!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->LoadName);
		goto Cleanup;
		} // if read header failed

	// Read X, Y, Z resolution (Units of measure come from GrUnits for X & Y, ElUnits for Z)
	for (i = 0; i < 3; i++)
		Info.Res[i] = FCvt(USGSHdr.Resolution[i]);

	Info.Code = (short)atoi(USGSHdr.RefSysCode);
	Info.Zone = (short)atoi(USGSHdr.Zone);
	Info.HUnits = (short)atoi(USGSHdr.GrUnits);
	Info.VUnits = (short)atoi(USGSHdr.ElUnits);

#ifdef WCS_BUILD_VNS
		switch (atoi(USGSHdr.HDatum))
			{
			default:
				if (! Importing->HasUTM)
					{
					// default for geographic data
					Importing->IWCoordSys.SetSystemByCode(0);			// Custom
					Importing->IWCoordSys.Method.SetMethodByCode(6);	// Geographic
					Importing->IWCoordSys.Datum.SetDatumByCode(350);	// WGS 72
					break;
					}
				// fall through to NAD 27 as default for UTM
				//lint -fallthrough
			case 1:	// NAD 27
				if (RUTMZone == 0)
					RUTMZone = Info.Zone;
				else if (RUTMZone != Info.Zone)
					{
					error = 33;	// different UTM zones
					ZoneMixErr = TRUE;
					}
				sprintf(zmsg, "%s = Zone %d", Importing->LoadName, Info.Zone);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, zmsg);
				if (Info.Code == 0)
					Importing->IWCoordSys.SetSystem("Geographic - NAD 27");
				else
					Importing->IWCoordSys.SetSystem("UTM - NAD 27");
				zonenum = Info.Zone;
				UTMZoneNum2DBCode(&zonenum);
				Importing->IWCoordSys.SetZoneByCode(zonenum);
				break;
			case 2:	// WGS 72
				break;
			case 3: // WGS 84
				break;
			case 4:	// NAD 83
				if (RUTMZone == 0)
					RUTMZone = Info.Zone;
				else if (RUTMZone != Info.Zone)
					{
					error = 33;	// different UTM zones
					ZoneMixErr = TRUE;
					}
				sprintf(zmsg, "%s = Zone %d", Importing->LoadName, Info.Zone);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, zmsg);
				if (Info.Code == 0)
					Importing->IWCoordSys.SetSystem("Geographic - NAD 83");
				else
					Importing->IWCoordSys.SetSystem("UTM - NAD 83");
				zonenum = Info.Zone;
				UTMZoneNum2DBCode(&zonenum);
				Importing->IWCoordSys.SetZoneByCode(zonenum);
				break;
			case 5: // Old Hawaii Datum
				break;
			case 6: // Puerto Rico Datum
				break;
			case 7: // NAD 83 Provisional
				break;
			} // switch HDatum
#endif // WCS_BUILD_VNS

	if (Importing->HasUTM)
		{
		UTMLatLonCoords_Init(&Convert, Importing->UTMZone);
		// Convert Northings & Eastings to Lat & Lon
		for (i = 0; i < 4; i++)
			{
			Info.UTM[i][0] = FCvt(USGSHdr.Coords[i][0]);
			Info.UTM[i][1] = FCvt(USGSHdr.Coords[i][1]);
			Convert.East = Info.UTM[i][0];
			Convert.North = Info.UTM[i][1];
			UTM_LatLon(&Convert);
			Info.LL[i][0] = Convert.Lon;
			Info.LL[i][1] = Convert.Lat;
			} // for
		Importing->NBound = Info.LL[0][1];	// Set initial Bounds to an edge
		Importing->SBound = Info.LL[0][1];
		Importing->WBound = Info.LL[0][0];
		Importing->EBound = Info.LL[0][0];
		for (i = 1; i < 4; i++)
			{
			if (Info.LL[i][1] > Importing->NBound)
				Importing->NBound = Info.LL[i][1];
			if (Info.LL[i][1] < Importing->SBound)
				Importing->SBound = Info.LL[i][1];
			if (Info.LL[i][0] > Importing->WBound)
				Importing->WBound = Info.LL[i][0];
			if (Info.LL[i][0] < Importing->EBound)
				Importing->EBound = Info.LL[i][0];
			} // for
		if (! GetUTMBounds(&Info))
			goto Cleanup;
		} // if

/*** F2 NOTE: Fix for VNS ***/
	if ((Info.Code != 0) && (Info.Code != 1))
		{
		if (Info.Code == 2)
			UserMessageOK(MsgHdr, "Unsupported planimetric system (State plane).  Only Geographic & UTM supported.");
		else
			UserMessageOK(MsgHdr, "Unsupported planimetric system.  Only Geographic & UTM supported.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Importing->NameBase);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Not a UTM or Geographic referenced file");
		goto Cleanup;
		} // if

	if (!TestOnly)
		{
		if (Importing->HasUTM && FoundUTM)
			;	// we've already done what we needed to do
		else if (Importing->HasUTM)
			{
			FoundUTM = TRUE;
			if ((error = ExtractUTM_DEM()) != 0)	// extract all the UTM DEMs together
				goto Cleanup;
			}
		else if ((error = ExtractGeo_DEM()) != 0)	// extract the Geographic DEMs
			goto Cleanup;
		Importing = Importing->Next;
		} // !TestOnly

	fclose(DEMFile);
	DEMFile = NULL;

	} while (!TestOnly && Importing);

if (! ZoneMixErr)
	error = 0;

if (! TestOnly)
	{
	Importing = HeadPier; // so we clean up
//	if (Importing->FileParent)
//		CloseUSGSDBGroup();
	} // if

Cleanup:
if (DEMFile)
	fclose(DEMFile);
return error;

} // ImportWizGUI::Load_USGSDEM
