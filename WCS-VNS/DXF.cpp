// DXF.cpp
// Too much DXF stuff in too many places. Moved here on Nov 18, 2000 by CXH

#include "stdafx.h"
#include "DXF.h"
#include "Application.h"
#include "Project.h"
#include "AppMem.h"
#include "Database.h"
#include "Joe.h"
#include "Log.h"
#include "Layers.h"
#include "Useful.h"
#include "Palette.h"
#include "Requester.h"
#include "Projection.h"
#include "Types.h"
#include "DB3LayersGUI.h"
#include "DEM.h"
#include "DBEditGUI.h"
#include "EffectsLib.h"
#include "DataOpsDefs.h"
#include "ImportWizGUI.h"

using namespace std;

// VCoords is in DataOpsVectors already
extern union MapProjection VCoords;

// ImportWizGUI::ScanDXF and TestForUTM don't need any merging.

unsigned long ImportWizGUI::ScanDXF(void)
{
double FloatValue;
BusyWin *BWDX = NULL;
FILE *dxffile;
string comment1, comment2, display;
#ifdef DEBUG
unsigned long line = 0;
#endif // DEBUG
long PtsScanned = 0;
long fpos, times = 0, update_interval;
short code, Interested = 0, InterestingPoint = 0, WaitForVertices = 0;
char GroupCode[256], GroupValue[4096];

DXFinfo->MaxVal[0] = DXFinfo->MaxVal[1] = DXFinfo->MaxVal[2] = -1000000000.0;
DXFinfo->MinVal[0] = DXFinfo->MinVal[1] = DXFinfo->MinVal[2] = 1000000000.0;

Importing->Mode = IMWIZ_HORUNITS_LATLON;

dxffile = PROJ_fopen(Importing->LoadName, "r");
if (dxffile == NULL)
	return 0;	// couldn't open file
fseek(dxffile, 0, SEEK_END);
fpos = ftell(dxffile);
rewind(dxffile);
update_interval = fpos / 3840; // sample file contains about 15 bytes/record, * 256 desired updates to the meter
BWDX = new BusyWin("Reading", (ULONG)fpos, 'BWDX', 0);
while (TRUE)	//lint !e716
	{
	if (! fgetline(GroupCode, 256, dxffile))
		break;
	if (! fgetline(GroupValue, 4096, dxffile))
		break;
#ifdef DEBUG
	line += 2;
#endif // DEBUG
	if (BWDX)
		{
		times++;
		if (times > update_interval) // time to update the meter?
			{
			times = 0;
			if (BWDX->CheckAbort())
				{
				delete BWDX;
				return 0;
				}
			fpos = ftell(dxffile);
			BWDX->Update((ULONG)fpos);
			} // if updating
		} // if BWDX

	code = atoi(GroupCode);
	if (code >= 0 && code <= 9)
		{
		if (! Interested)
			{
			if (! strnicmp(GroupValue, "$ACADVER", 8))
				{
				fgetline(GroupCode, 256, dxffile);
				fgetline(GroupValue, 256, dxffile);
				#ifdef DEBUG
					line += 2;
				#endif // DEBUG
				// changed next line from strcmp to strncmp due to apparent bug in fgetline - 1/17/06 FPW2
				if (strncmp(GroupValue, "AC1012", 6) && strncmp(GroupValue, "AC1009", 6))
					{
					UserMessageOK("Import DXF", "DXF file isn't in R13 or R12 format.  File may not import correctly.");
					} // if
				} // if
			if (! strnicmp(GroupValue, "ENTITIES", 8))
				{
				Interested = 1;
				WaitForVertices = 0;
				} // if
			}
		else if (! strnicmp(GroupValue, "ENDSEC", 6))
			{
			Interested = 0;
			InterestingPoint = 0;
			} // if
		else if (! strnicmp(GroupValue, "POLYLINE", 8))
			{
			//InterestingPoint = 0;
			//WaitForVertices = 1;
			InterestingPoint = 1;
			WaitForVertices = 0;
			} // else if
		else if (! strnicmp(GroupValue, "LWPOLYLINE", 10))
			{
			InterestingPoint = 1;
			WaitForVertices = 0;
			} // else if
//		else if (! strnicmp(GroupValue, "VERTEX", 6) || ! strnicmp(GroupValue, "POINT", 5) ||
//			! strnicmp(GroupValue, "INSERT", 6) || ! strnicmp(GroupValue, "3DFACE", 6))
		else if  (! strnicmp(GroupValue, "3DFACE", 6))
			{
			Importing->Flags = DXF_3DFACE;
			if (! WaitForVertices)
				InterestingPoint = 1;
			} // else if
		else if (! strnicmp(GroupValue, "VERTEX", 6) || ! strnicmp(GroupValue, "POINT", 5) ||
			! strnicmp(GroupValue, "LINE", 4) || ! strnicmp(GroupValue, "POLYLINE", 8) ||
			! strnicmp(GroupValue, "LWPOLYLINE", 10))
			{
			if (! WaitForVertices)
				InterestingPoint = 1;
			} // else if
		else if (! strnicmp(GroupValue, "SEQEND", 6))
			{
			InterestingPoint = 0;
			WaitForVertices = 0;
			} // else
		}
	else if (code >= 10 && code <= 19)
		{
		if (InterestingPoint)
			{
			FloatValue = atof(GroupValue);
			if (FloatValue > DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X])
				DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X] = FloatValue;
			if (FloatValue < DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X])
				DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X] = FloatValue;
			PtsScanned ++;
			} // if
		} // if X value
	else if (code >= 20 && code <= 29)
		{
		if (InterestingPoint)
			{
			FloatValue = atof(GroupValue);
			if (FloatValue > DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y])
				DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y] = FloatValue;
			if (FloatValue < DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y])
				DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y] = FloatValue;
			PtsScanned ++;
			} // if
		} // else if Y value
	else if (code >= 30 && code <= 39)
		{
		if (InterestingPoint)
			{
			FloatValue = atof(GroupValue);
			if (FloatValue > DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z])
				DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z] = FloatValue;
			if (FloatValue < DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z])
				DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z] = FloatValue;
			PtsScanned ++;
			} // if
		} // else if Z value
	else if (code == 66)
		{
		WaitForVertices = 0;
		} // else if polyline points follow flag
	else if (code >= 60 && code <= 79)
		{
		}
	else if (code >= 210 && code <= 239)
		{
		}
	else if (code == 999)
		{
		static unsigned long comments = 0;

		comments++;
		if (comments == 1)
			comment1.assign(GroupValue);
		else if (comments == 2)
			comment2.assign(GroupValue);
		else
			{
			// only keep last two comments
			comment1 = comment2;
			comment2.assign(GroupValue);
			} // else
		} // else if 999
	else
		{
		} // else
	} // while TRUE

if (BWDX)
	delete BWDX;

//#ifdef WCS_BUILD_VNS
if (DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z] < DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z])
	{
	// 3D DXF
	sprintf(Importing->FileInfo, "\r\r\n\r\r\nX range: %.1f  -  %.1f\r\r\nY range: %.1f  -  %.1f\r\r\nZ range: %.1f  -  %.1f",
			DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X], DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X],
			DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y], DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y],
			DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z], DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z]);
	} // if
else
	{
	// must be a 2D DXF
	sprintf(Importing->FileInfo, "\r\r\n\r\r\nX range: %.1f  -  %.1f\r\r\nY range: %.1f  -  %.1f",
			DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X], DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X],
			DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y], DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y]);
	} // else
if (comment1.size() != 0)
	{
	display = comment1.substr(0, 200);
	strcat(Importing->FileInfo, "\r\r\n\r\r\nComment: ");
	strcat(Importing->FileInfo, display.c_str());
	};
if (comment2.size() != 0)
	{
	display = comment2.substr(0, 200);
	strcat(Importing->FileInfo, "\r\r\n\r\r\nComment: ");
	strcat(Importing->FileInfo, display.c_str());
	};
//#endif // WCS_BUILD_VNS

if (PtsScanned && DXFinfo->MinVal[0] < 100000000.0 && DXFinfo->MinVal[1] < 100000000.0)
	{
	DXFinfo->RasterInput = 0;
	DXFinfo->OutputStyle = WCS_IMPORTDATA_OUTPUTSTYLE_VECTOR;
	DXFinfo->Resample = 0;
	DXFinfo->ReverseX = 0;
	DXFinfo->ReverseXEnabled = 1;
	DXFinfo->InputFormat = WCS_IMPORTDATA_FORMAT_DXF;
	DXFinfo->FormatEnabled[WCS_IMPORTDATA_FORMAT_UNKNOWN] = 1;
	DXFinfo->FormatEnabled[WCS_IMPORTDATA_FORMAT_DXF] = 1;
	DXFinfo->OutStyleEnabled[WCS_IMPORTDATA_OUTPUTSTYLE_DEM] = 1;
	DXFinfo->OutStyleEnabled[WCS_IMPORTDATA_OUTPUTSTYLE_CONTROL] = 1;
	DXFinfo->OutStyleEnabled[WCS_IMPORTDATA_OUTPUTSTYLE_VECTOR] = 1;
	DXFinfo->InputHUnitsEnabled = 1;
//	DXFinfo->ZeroReference = PreferredReference;
	DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X];
	DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y];
	DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X];
	DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y];

	if (DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y] < 90.0 && DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y] > -90.0)
		{
		DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
#ifdef WCS_BUILD_VNS
		Importing->GridUnits = 2;
#else // WCS_BUILD_VNS
		Importing->GridUnits = IMWIZ_HORUNITS_LATLON;
#endif // WCS_BUIL_VNS
		Importing->HasUTM = FALSE;
		} // if
	else
		{
#ifdef WCS_BUILD_VNS
		Importing->GridUnits = 2;
#else // WCS_BUILD_VNS
		Importing->GridUnits = IMWIZ_HORUNITS_UTM;
#endif // WCS_BUILD_VNS
		Importing->HasUTM = TRUE;
		Importing->Mode = IMWIZ_HORUNITS_UTM;
		} // else

#ifdef FOOBAR
//	else
//		DXFinfo->InputHUnits = GlobalApp->MainProj->Interactive->GetProjHorUnits();
//	DXFinfo->InputVUnits = GlobalApp->MainProj->Interactive->GetProjVertUnits();
	if (! (DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y] < 90.0 && DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y] > -90.0) && TestForUTM(DXFinfo->MaxVal, DXFinfo->MinVal))
		{
		DXFinfo->UTMUnits = UserMessageYN("Import Wizard", "This file appears to be projected in Universal Transverse Mercator coordinates. Is this true?");
		if (DXFinfo->UTMUnits)
			{
			Importing->HasUTM = TRUE;
			sprintf(Str, "%d", GlobalApp->MainProj->Prefs.LastUTMZone);
			if (GetInputString("Enter the UTM Zone number (1-60, negative if southern hemisphere) if you know it.", ":;*/?`#%", Str))
				DXFinfo->UTMZone = atoi(Str);
			else
				DXFinfo->UTMZone = 10;
			GlobalApp->MainProj->Prefs.LastUTMZone = DXFinfo->UTMZone;
			// initialize UTM transform structure and convert the max and min x & y coordinates
			UTMLatLonCoords_Init(&Importing->Coords.UTM, DXFinfo->UTMZone);
			Importing->Coords.UTM.North = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y];
			Importing->Coords.UTM.East = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X];
			UTM_LatLon(&Importing->Coords.UTM);
			DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] = Importing->Coords.UTM.Lon;
			DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] = Importing->Coords.UTM.Lat;
			Importing->Coords.UTM.North = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y];
			Importing->Coords.UTM.East = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X];
			UTM_LatLon(&Importing->Coords.UTM);
			DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] = Importing->Coords.UTM.Lon;
			DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] = Importing->Coords.UTM.Lat;
			if (fabs(DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y]) > 90.0 || 
				fabs(DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y]) > 90.0 || 
				fabs(DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X]) > 360.0 || 
				fabs(DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X]) > 360.0)
				{
				DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X];
				DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y];
				DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X];
				DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y];
				} // if
			else
				{
				DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
				DXFinfo->InputHUnitsEnabled = 0;
//				DXFinfo->ZeroReference = WCS_IMPORTDATA_REFERENCE_IGNORE;
				DXFinfo->ReverseXEnabled = 0;
				} // else
			} // if DXFinfo->UTMUnits
		else
			Importing->WantGridUnits = TRUE;
		} // ifUTM
#endif // FOOBAR

	DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] -= DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X];
	DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] -= DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y];
	DXFinfo->InputRows = DXFinfo->OutputRows = 200;
	DXFinfo->InputCols = DXFinfo->OutputCols = (long)(200 * DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] / DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y]);
	if (DXFinfo->InputCols < 2)
		DXFinfo->InputCols = 2;
	if (fabs(DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z]) > 1000000.0 || fabs(DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z]) > 1000000.0)
		{
		DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z] = 0.0;
		} // if
	DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z] = DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Z] = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z];
	DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z] = DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Z] = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z];
	DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_X] = DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X];
	DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Y] = DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y];
	DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_X] = DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X];
	DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Y] = DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y];
	DXFinfo->GridSpaceNS = DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] / (DXFinfo->InputRows - 1);
	DXFinfo->GridSpaceWE = DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] / (DXFinfo->InputCols - 1);
	DXFinfo->OutputWEMaps = DXFinfo->InputCols / 300;
	DXFinfo->OutputNSMaps = DXFinfo->InputRows / 300;
	strcpy(DXFinfo->OutName, Importing->NameBase);
	(void)StripExtension(DXFinfo->OutName);
	(void)TrimTrailingSpaces(DXFinfo->OutName);
	strcpy(DXFinfo->filename, Importing->LoadName);
	fclose(dxffile);

	Importing->InCols = Importing->OutCols = DXFinfo->InputCols;
	Importing->InRows = Importing->OutRows = DXFinfo->InputRows;
	Importing->EBound = DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X];
	Importing->SBound = DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y];
	Importing->WBound = Importing->EBound + DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X];
	Importing->NBound = Importing->SBound + DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y];
//	Importing->GridSpaceNS = DXFinfo->GridSpaceNS;
//	Importing->GridSpaceWE = DXFinfo->GridSpaceWE;
	DataOpsDimsFromBounds();
	Importing->TestMax = DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z];
	Importing->TestMin = DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z];

#ifdef FOOBAR
	if (Importing->HasUTM)
		Importing->AllowPosE = FALSE;
	else
		Importing->AllowPosE = TRUE;
	if (Importing->WantGridUnits)
		Importing->AllowRef00 = TRUE;
	else
		Importing->AllowRef00 = FALSE;
#endif // FOOBAR

	Importing->AskNull = FALSE;

	return 1;	// ???
	} // if
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "No points found / unsupported entity");
memset(DXFinfo, 0, sizeof (struct ImportData));
return 0;	// ???

} // ImportWizGUI::ScanDXF

/*===========================================================================*/

#ifdef FOOBAR
static BOOL TestForUTM(double *MaxVal, double *MinVal)
{

if (fabs(MaxVal[WCS_IMPORTDATA_COORD_X]) > 10000.0 || fabs(MinVal[WCS_IMPORTDATA_COORD_X]) > 10000.0 || fabs(MaxVal[WCS_IMPORTDATA_COORD_Y]) > 10000.0 || fabs(MinVal[WCS_IMPORTDATA_COORD_Y]) > 10000.0)
	{
	return TRUE;
	} // if

return FALSE;

} // TestForUTM
#endif // FOOBAR

/*===========================================================================*/

// SetObjectLayer has been merged
static void SetObjectLayer(Joe *AverageGuy, char *Layer)
{
LayerEntry *TargetLayer;
long i = 0;

while (Layer[i * WCS_DXF_MAX_LAYERLENGTH] && i < WCS_DXF_MAX_LAYERS)
	{
	if (TargetLayer = GlobalApp->AppDB->DBLayers.MatchMakeLayer(&Layer[i * WCS_DXF_MAX_LAYERLENGTH], 0))
		{
		AverageGuy->AddObjectToLayer(TargetLayer);	// if it fails, tough!
		} // if
	++i;
	} // while

} // SetObjectLayer

/*===========================================================================*/

// SetDXFObjectOneLayer has been merged
static void SetDXFObjectOneLayer(Joe *AverageGuy, char *Layer)
{
LayerEntry *TargetLayer;

if (Layer)
	{
	if (TargetLayer = GlobalApp->AppDB->DBLayers.MatchMakeLayer(Layer, 0))
		{
		AverageGuy->AddObjectToLayer(TargetLayer);	// if it fails, tough!
		} // if
	} // if

} // SetDXFObjectOneLayer

/*===========================================================================*/

// IncreaseCopyPointAllocation has been merged
VectorPoint *IncreaseCopyPointAllocation(VectorPoint *OldPoints, unsigned long &NumPts, VectorPoint *&CurrentPoint)
{
VectorPoint *NewPoints, *PLink;
long NewNumPoints;

NewNumPoints = 2000;	// (OldPoints && NumPts > 0) ? NumPts * 2: 2000;

if (OldPoints)
	{
	for (PLink = OldPoints; PLink->Next; PLink = PLink->Next)
		;
	} // if
else
	NumPts = 0;
if (NewPoints = GlobalApp->AppDB->MasterPoint.Allocate(NewNumPoints))
	{
	if (OldPoints)
		{
		PLink->Next = NewPoints;
		/*
		for (PLinkA = OldPoints, PLinkB = NewPoints; PLinkA; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
			{
			PLinkB->Latitude = PLinkA->Latitude;
			PLinkB->Longitude = PLinkA->Longitude;
			PLinkB->Elevation = PLinkA->Elevation;
			if (PLinkA == CurrentPoint)
				CurrentPoint = PLinkB;
			} // for
		DBHost->MasterPoint.DeAllocate(OldPoints);
		*/
		} // if
	else
		{
		CurrentPoint = NewPoints;
		OldPoints = NewPoints;
		} // else assign CurrentPoint to base of chain
	NumPts += NewNumPoints;
	} // if
else
	{
	CurrentPoint = NULL;
	if (OldPoints)
		GlobalApp->AppDB->MasterPoint.DeAllocate(OldPoints);
	OldPoints = NULL;
	} // else failure

return (OldPoints);

} // IncreaseCopyPointAllocation

/*===========================================================================*/

// IncreaseCopyPolygonAllocation has been merged
DXFImportPolygon *IncreaseCopyPolygonAllocation(DXFImportPolygon *OldPolys, unsigned long &NumPolys)
{
DXFImportPolygon *NewPolys;
long NewNumPolys;

NewNumPolys = (OldPolys && NumPolys > 0) ? NumPolys * 2: 2000;

if (NewPolys = (DXFImportPolygon *)AppMem_Alloc(NewNumPolys * sizeof (struct DXFImportPolygon), APPMEM_CLEAR))
	{
	if (OldPolys)
		{
		memcpy(NewPolys, OldPolys, NumPolys * sizeof (struct DXFImportPolygon));
		AppMem_Free(OldPolys, NumPolys * sizeof (struct DXFImportPolygon));
		} // if
	NumPolys = NewNumPolys;
	} // if
else
	{
	if (OldPolys)
		AppMem_Free(OldPolys, NumPolys * sizeof (struct DXFImportPolygon));
	} // else failure

return (NewPolys);

} // IncreaseCopyPolygonAllocation

/*===========================================================================*/

// SetDXFObject has been merged
Joe *SetDXFObject(char *Name, long Pairs, char Style, unsigned char Color, VectorPoint *Points, ImportWizGUI *IWG)
{
extern struct DXF_Pens dxfpens;
long Point;
float MyNorth, MySouth, MyEast, MyWest;
Joe *AverageGuy;
VectorPoint *PLinkA, *PLinkB;
char RGB[8][3] = {(char)136, (char)153, (char)187,  (char)0, (char)0, (char)0,  (char)221, (char)221, (char)221,  (char)187, (char)31, (char)0,  (char)51, (char)68, (char)136,
					(char)51, (char)153, (char)34,  (char)51, (char)119, (char)204,  (char)221, (char)221, (char)34};
char DoBounds = TRUE;	// if Gary's the caller {since I don't know what he wants}, or IWG under WCS

#ifdef WCS_BUILD_VNS
if (IWG)
	DoBounds = FALSE;	// IWG under VNS
#endif // WCS_BUILD_VNS

if (AverageGuy = new (Name) Joe)
	{
	if (AverageGuy->Points(GlobalApp->AppDB->MasterPoint.Allocate(Pairs + Joe::GetFirstRealPtNum())))
		{
		AverageGuy->NumPoints(Pairs + Joe::GetFirstRealPtNum());
		if (DoBounds)
			{
			MyNorth = -90.0f;
			MySouth = 90.0f;
			MyEast = 360.0f;
			MyWest = -360.0f;
			} // if
		#ifdef WCS_JOE_LABELPOINTEXISTS
		PLinkA = AverageGuy->Points();
		PLinkA->Latitude = Points->Latitude;
		PLinkA->Longitude = Points->Longitude;
		PLinkA->Elevation = Points->Elevation;
		#endif // WCS_JOE_LABELPOINTEXISTS
		for (Point = 0, PLinkA = AverageGuy->GetFirstRealPoint(), PLinkB = Points; Point < Pairs && PLinkB; Point ++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
			{
			PLinkA->Latitude = PLinkB->Latitude;
			PLinkA->Longitude = PLinkB->Longitude;
			PLinkA->Elevation = PLinkB->Elevation;
			if (DoBounds)
				{
				if (PLinkA->Latitude > MyNorth)
					{
					MyNorth = (float)PLinkA->Latitude;
					} // if
				if (PLinkA->Latitude < MySouth)
					{
					MySouth = (float)PLinkA->Latitude;
					} // if
				if (PLinkA->Longitude > MyWest)
					{
					MyWest = (float)PLinkA->Longitude;
					} // if
				if (PLinkA->Longitude < MyEast)
					{
					MyEast = (float)PLinkA->Longitude;
					} // if
				} // if DoBounds
			if (IWG)
				{
				IWG->cpstats.n++;
				if (PLinkA->Latitude > IWG->cpstats.maxY)
					IWG->cpstats.maxY = PLinkA->Latitude;
				if (PLinkA->Latitude < IWG->cpstats.minY)
					IWG->cpstats.minY = PLinkA->Latitude;
				IWG->cpstats.sumY += PLinkA->Latitude;
				IWG->cpstats.sumY2 += PLinkA->Latitude * PLinkA->Latitude;
				if (PLinkA->Longitude > IWG->cpstats.maxX)
					IWG->cpstats.maxX = PLinkA->Longitude;
				if (PLinkA->Longitude < IWG->cpstats.minX)
					IWG->cpstats.minX = PLinkA->Longitude;
				IWG->cpstats.sumX += PLinkA->Longitude;
				IWG->cpstats.sumX2 += PLinkA->Longitude * PLinkA->Longitude;
				} // if
			} // for

		if (DoBounds)
			{
			AverageGuy->NWLat = MyNorth;
			AverageGuy->NWLon = MyWest;
			AverageGuy->SELat = MySouth;
			AverageGuy->SELon = MyEast;
			} // if

		AverageGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
		if (IWG)
			{
			if (! IWG->Importing->DBRenderFlag)
				AverageGuy->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
			} // if
		AverageGuy->SetLineStyle(Style);
		AverageGuy->SetLineWidth((unsigned char)1);
		AverageGuy->SetRGB(dxfpens.r[Color], dxfpens.g[Color], dxfpens.b[Color]);
		#ifdef WCS_BUILD_VNS
		if (IWG)
			{
			if (IWG->NewCoordSys)
				AverageGuy->AddEffect(IWG->NewCoordSys, -1);
			AverageGuy->RecheckBounds();
			} // if
		#endif // WCS_BUILD_VNS
		} // if
	} // if

return (AverageGuy);

} // SetDXFObject()

/*===========================================================================*/

// PrepnSaveDXFObject has been merged
Joe *PrepnSaveDXFObject(Joe *FileParent, Joe *AfterGuy, ImportHook *IH, struct ImportData *ScaleHook,
	VectorPoint *Points, char *ObjName, char *ObjLayer, unsigned long NumPts, float DefaultElev, short ElevDataValid,
	short RefSys, char ConnectPoints, short ElevUnits, short ReverseLon, double *MaxBounds, ImportWizGUI *IWG,
	struct DXFLayerPens *LayerPens, short PenNum)
{
unsigned long SkipIt, Pt, RPt, ReducedNumPts, PtsToSave = 0, PtsSaved = 0;
float ElevScale;
VectorPoint *PLink, *PLinkA, *PLinkB, *ReducedPoints = NULL;
Joe *AverageGuy;
unsigned char DrawPen;
struct DXFLayerPens *LayerPtr;
BOOL matched;

if (Points && NumPts > 0)
	{
	if (! ElevDataValid)
		{
		for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
			{
			PLink->Elevation = DefaultElev;
			} // for i=0... 
		} // if
	if (ElevUnits)
		{
		ElevScale = (float)(ELSCALE_FEET / ELSCALE_METERS);	// (km / ft) / (km / m) = m / ft
		for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
			{
			PLink->Elevation *= ElevScale;
			} // for
		} // if
#ifdef WCS_BUILD_VNS
	if (ReverseLon)
		{
		for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
			{
			PLink->Longitude = -PLink->Longitude;
			} // for Pt
		} // if ReverseLon
#else // WCS_BUILD_VNS
	if (RefSys == 1)
		{
		for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
			{
			VCoords.UTM.East = PLink->Longitude;
			VCoords.UTM.North = PLink->Latitude;
			UTM_LatLon(&VCoords.UTM);
			PLink->Longitude = VCoords.UTM.Lon;
			PLink->Latitude = VCoords.UTM.Lat;
			} // for
		} // if UTM
	else if (ReverseLon && ! ScaleHook)
		{
		for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
			{
			PLink->Longitude = -PLink->Longitude;
			} // for i=0... 
		} // else if
#endif // WCS_BUILD_VNS
	if (IH)
		{
		if (ReducedPoints = GlobalApp->AppDB->MasterPoint.Allocate(NumPts))
			{
			PLinkB = ReducedPoints;
			ReducedNumPts = 0;
			for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
				{
				SkipIt = 0;
				for (PLinkA = ReducedPoints, RPt = 0; RPt < ReducedNumPts; PLinkA = PLinkA->Next, RPt ++)
					{
					if (PLink->Latitude == PLinkA->Latitude && PLink->Longitude == PLinkA->Longitude)
						{
						SkipIt = 1;
						break;
						} // if
					} // for
				if (! SkipIt)
					{
					IH->Lat = PLinkB->Latitude = PLink->Latitude;
					IH->Lon = PLinkB->Longitude = PLink->Longitude;
					IH->El = PLinkB->Elevation = PLink->Elevation;
					PLinkB = PLinkB->Next;
					ReducedNumPts ++;
					IH->Connected = Pt > 0 ? ConnectPoints: 0;
					IH->Invoke(IH);
					} // if
				} // for
			GlobalApp->AppDB->MasterPoint.DeAllocate(ReducedPoints);
			ReducedPoints = NULL;
			} // if
		} // if
	else if (FileParent && MaxBounds)
		{
		if (ScaleHook)
			{
			// <<<>>> if control points - remove duplicates
			if (ScaleHook->OutputStyle == WCS_IMPORTDATA_OUTPUTSTYLE_CONTROL ||
				ScaleHook->OutputStyle == WCS_IMPORTDATA_OUTPUTSTYLE_DEM)
				{
				if (ReducedPoints = GlobalApp->AppDB->MasterPoint.Allocate(NumPts))
					{
					PLinkB = ReducedPoints;
					ReducedNumPts = 0;
					for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
						{
						SkipIt = 0;
						for (PLinkA = ReducedPoints, RPt = 0; RPt < ReducedNumPts; PLinkA = PLinkA->Next, RPt ++)
							{
							if (PLink->Latitude == PLinkA->Latitude && PLink->Longitude == PLinkA->Longitude)
								{
								SkipIt = 1;
								break;
								} // if
							} // for
						if (! SkipIt)
							{
							PLinkB->Latitude = PLink->Latitude;
							PLinkB->Longitude = PLink->Longitude;
							PLinkB->Elevation = PLink->Elevation;
							PLinkB = PLinkB->Next;
							ReducedNumPts ++;
							} // if
						} // for
					Points = ReducedPoints;
					NumPts = ReducedNumPts;
					} // if
				} // if
			for (Pt = 0, PLink = Points; Pt < NumPts && PLink; Pt ++, PLink = PLink->Next)
				{
				if (ScaleHook->ScaleLatFunc)
					ScaleHook->ScaleLatFunc(ScaleHook, PLink->Latitude);
				if (ScaleHook->ScaleLonFunc)
					ScaleHook->ScaleLonFunc(ScaleHook, PLink->Longitude);
				if (ScaleHook->ScaleElevFunc)
					ScaleHook->ScaleElevFunc(ScaleHook, PLink->Elevation);
				} // for
			} // if
		if (! ObjName[0])
			{
			if (ObjLayer[0])
				strcpy(ObjName, ObjLayer);
			else
				strcpy(ObjName, "Unnamed");
			} // if
		while (PtsSaved < NumPts)
			{
			for (Pt = 0; Pt < PtsToSave; Pt ++, Points = Points->Next)
				;	// does nothing on first loop with PtsToSave = 0
			if (NumPts - PtsSaved >= WCS_DXF_MAX_OBJPTS - 1)
				{
				PtsToSave = WCS_DXF_MAX_OBJPTS - 1;
				} // if
			else
				{
				PtsToSave = NumPts - PtsSaved;
				} // else
			if (PenNum < 0)
				DrawPen = 2;
			else if (PenNum == 256)	// color by Layer
				{
				DrawPen = 2;	// set a default
				LayerPtr = LayerPens;
				matched = FALSE;
				while (LayerPtr && !matched)
					{
					if (strcmp(ObjName, LayerPtr->LayerName) != 0)
						LayerPtr = LayerPtr->Next;
					else
						{
						matched = TRUE;
						if (LayerPtr->Pen < 0)
							DrawPen = 2;
						else
							DrawPen = (unsigned char)LayerPtr->Pen;
						}
					}
				}
			else	// use pen color
				DrawPen = (unsigned char)PenNum;
			if (AverageGuy = SetDXFObject(ObjName, PtsToSave, ConnectPoints ? 4: 0, DrawPen, Points, IWG))
				{
				FileParent->AddChild(AverageGuy, AfterGuy);
				AfterGuy = AverageGuy;
				if (ObjLayer)
					SetDXFObjectOneLayer(AverageGuy, ObjLayer);
//				if (ScaleHook && (ScaleHook->OutputStyle == WCS_IMPORTDATA_OUTPUTSTYLE_CONTROL ||
//					ScaleHook->OutputStyle == WCS_IMPORTDATA_OUTPUTSTYLE_DEM))
				if (IWG)
					{
					if ((IWG->Importing->LoadAs == LAS_DEM) || (IWG->Importing->LoadAs == LAS_CP))
						AverageGuy->SetFlags(WCS_JOEFLAG_ISCONTROL);
					} // IWG
				if (AverageGuy->NWLat > MaxBounds[0])
					{
					MaxBounds[0] = AverageGuy->NWLat;
					} // if
				if (AverageGuy->SELat < MaxBounds[1])
					{
					MaxBounds[1] = AverageGuy->SELat;
					} // if
				if (AverageGuy->SELon < MaxBounds[2])
					{
					MaxBounds[2] = AverageGuy->SELon;
					} // if
				if (AverageGuy->NWLon > MaxBounds[3])
					{
					MaxBounds[3] = AverageGuy->NWLon;
					} // if
				} // if object created
			PtsSaved += PtsToSave;
			} // while more points to save
		} // else no input hook
	} // if

if (ReducedPoints)
	GlobalApp->AppDB->MasterPoint.DeAllocate(ReducedPoints);

return (AfterGuy);

} // PrepnSaveDXFObject

/*===========================================================================*/

unsigned long int ImportGISDXF(FILE *fDXF, Joe *FileParent, ImportHook *IH, struct ImportData *ScaleHook, Object3DEffect *Hook3D, ImportWizGUI *IWG)
{
double RefElev, RefLat, RefLon, Scale;
double MaxBounds[4];
BusyWin *Position = NULL;
DXFLayerPens *HeadLayer = NULL, *LayerColors = NULL;
Joe *AfterGuy = NULL;
unsigned long int PtCt, Ct, NumPtsAllocated, Num3DPolysAllocated, ObjsImported, Num3DPolysRead, PolylineFlags, PolyvertFlags;
short GroupCode, EntitiesFound = 0, Terminate = 1, DontRead = 0, MemError = 0, ReadyToRead, ReadingVertices, ElevRead,
	ElevUnits, WestNeg, RefSys, UTMZone, BailOut = 0, PolyfaceMesh, PolyfaceVertex, PolyfaceFace, PointHolding, Point3DHolding;
float DefaultElev = 0.0f;
VectorPoint *Points = NULL, *CurrentPoint = NULL, *PrevPoint = NULL, TempVtx, Temp3DVtx[4];
struct DXFImportPolygon *Polygons = NULL, TempPoly;
BOOL InLayer, ReducePts;
short aci = 256;	// AutoCAD Color Index
char Str[4096], ObjName[256], ObjLayer[256];

// initialize
if (IWG)
	IWG->ResetCPStats();
ObjName[0] = 0;
ObjLayer[0] = 0;
PtCt = NumPtsAllocated = 0;
ObjsImported = 0;
Num3DPolysAllocated = 0;
Num3DPolysRead = 0;
Point3DHolding = 0;
memset(&TempPoly, 0, sizeof (struct DXFImportPolygon));

if (IWG)
	{
	if ((IWG->Importing->Flags & DXF_3DFACE) && (IWG->Importing->LoadAs == LAS_CP))
		ReducePts = TRUE;
	} // if

RefLat = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
RefLon = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
RefElev = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);

// finish the initialization of DXFinfo
if (IWG)
	{
#ifdef WCS_BUILD_VNS
	if (IWG->Importing->IWCoordSys.Method.GCTPMethod == 0)
		{
		if (IWG->Importing->PosEast)
			ScaleHook->ReverseX = TRUE;
		else
			ScaleHook->ReverseX = FALSE;
		}
	else
		{
		if (IWG->Importing->PosEast)
			ScaleHook->ReverseX = FALSE;
		else
			ScaleHook->ReverseX = TRUE;
		}
#else // WCS_BUILD_VNS
	ScaleHook->ReverseX = IWG->Importing->PosEast;
#endif // WCS_BUILD_VNS
	if (IWG->Importing->Reference == 1) // X,Y
		{
		IWG->DXFinfo->ZeroReference = WCS_IMPORTDATA_REFERENCE_LOWXY;
		IWG->DXFinfo->ScaleLatFunc = ScaleLatitude;
		IWG->DXFinfo->ScaleLonFunc = ScaleLongitude;
	///	IWG->DXFinfo->ScaleElevFunc = ScaleElevation;
		IWG->DXFinfo->OutputLowLat = RefLat;
		IWG->DXFinfo->OutputLowLon = RefLon;
	///	IWG->DXFinfo->OutputHighElev =
	///		RefElev + DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z] - IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z];
	///	IWG->DXFinfo->OutputLowElev = RefElev;
		IWG->DXFinfo->OutputXScale = IWG->DXFinfo->OutputYScale = 1.0;
	///	IWG->DXFinfo->OutputElScale = 1.0;
		}
	else if (IWG->Importing->Reference == 2) // 0,0
		{
		IWG->DXFinfo->ZeroReference = WCS_IMPORTDATA_REFERENCE_ZERO;
	///	IWG->DXFinfo->OutputLowElev = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z];
	///	IWG->DXFinfo->OutputHighElev = IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z];
	//	IWG->DXFinfo->OutputLowElev = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z] * Scale;
	//	IWG->DXFinfo->OutputHighElev = IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z] * Scale;
	///	IWG->DXFinfo->OutputVUnits = DEM_DATA_UNITS_METERS;
		}
	else
		{
		IWG->DXFinfo->ZeroReference = WCS_IMPORTDATA_REFERENCE_IGNORE;
	///	IWG->DXFinfo->OutputLowElev = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z];
	///	IWG->DXFinfo->OutputHighElev = IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z];
	//	IWG->DXFinfo->OutputLowElev = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z] * Scale;
	//	IWG->DXFinfo->OutputHighElev = IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z] * Scale;
	///	IWG->DXFinfo->OutputVUnits = DEM_DATA_UNITS_METERS;
		}

	// set the elev ranges
	IWG->DXFinfo->OutputLowElev = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z] * IWG->Importing->VScale;
	IWG->DXFinfo->OutputHighElev = IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z] * IWG->Importing->VScale;
	IWG->DXFinfo->OutputVUnits = DEM_DATA_UNITS_METERS;
	if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_LOWXY)
		{
		IWG->DXFinfo->OutputHighElev = RefElev + IWG->DXFinfo->OutputHighElev - IWG->DXFinfo->OutputLowElev;
		IWG->DXFinfo->OutputLowElev = RefElev;
		} // else if
	else if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_ZERO)
		{
		IWG->DXFinfo->OutputLowElev += RefElev;
		IWG->DXFinfo->OutputHighElev += RefElev;
		} // else

//	if (IWG->Importing->GridUnits == IMWIZ_HORUNITS_UTM)
	if (IWG->Importing->Mode == IMWIZ_HORUNITS_UTM)
		{
		// initialize UTM transform structure and convert the max and min x & y coordinates
		IWG->DXFinfo->UTMUnits = 1;
		IWG->DXFinfo->UTMZone = IWG->Importing->UTMZone;
		IWG->DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
		IWG->DXFinfo->OutputVUnits = 1;
		IWG->DXFinfo->ReverseX = 0;
		UTMLatLonCoords_Init(&IWG->Importing->Coords.UTM, IWG->DXFinfo->UTMZone);
		IWG->Importing->Coords.UTM.North = IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y];
		IWG->Importing->Coords.UTM.East = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X];
		UTM_LatLon(&IWG->Importing->Coords.UTM);
		IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] = IWG->Importing->Coords.UTM.Lon;
		IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] = IWG->Importing->Coords.UTM.Lat;
		IWG->Importing->Coords.UTM.North = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y];
		IWG->Importing->Coords.UTM.East = IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X];
		UTM_LatLon(&IWG->Importing->Coords.UTM);
		IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] = IWG->Importing->Coords.UTM.Lon;
		IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] = IWG->Importing->Coords.UTM.Lat;
		if (fabs(IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y]) > 90.0 || 
			fabs(IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y]) > 90.0 || 
			fabs(IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X]) > 360.0 || 
			fabs(IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X]) > 360.0)
			{
			IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] = IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_X];
			IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] = IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Y];
			IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_X];
			IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Y];
			} // if
		else
			{
			IWG->DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
			IWG->DXFinfo->InputHUnitsEnabled = 0;
	//				IWG->DXFinfo->ZeroReference = WCS_IMPORTDATA_REFERENCE_IGNORE;
			IWG->DXFinfo->ReverseXEnabled = 0;
			} // else
		IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] -= IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] -= IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y];
		IWG->DXFinfo->InputRows = IWG->DXFinfo->OutputRows = 200;
		IWG->DXFinfo->InputCols = IWG->DXFinfo->OutputCols = (long)(200 * IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] / IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y]);
		if (IWG->DXFinfo->InputCols < 2)
			IWG->DXFinfo->InputCols = 2;
		if (fabs(IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z]) > 1000000.0 || fabs(IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z]) > 1000000.0)
			{
			IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z] = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z] = 0.0;
			} // if
		IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Z] = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Z] = IWG->DXFinfo->MaxVal[WCS_IMPORTDATA_COORD_Z];
		IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Z] = IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Z] = IWG->DXFinfo->MinVal[WCS_IMPORTDATA_COORD_Z];
		IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_X] = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Y] = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y];
		IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_X] = IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Y] = IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y];
		IWG->DXFinfo->GridSpaceNS = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] / (IWG->DXFinfo->InputRows - 1);
		IWG->DXFinfo->GridSpaceWE = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] / (IWG->DXFinfo->InputCols - 1);
		IWG->DXFinfo->OutputWEMaps = IWG->DXFinfo->InputCols / 300;
		IWG->DXFinfo->OutputNSMaps = IWG->DXFinfo->InputRows / 300;
		goto Skippy;
		}
#ifdef WCS_BUILD_VNS
	else if (IWG->Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES)
#else // WCS_BUILD_VNS
	else if (IWG->DXFinfo->InputHUnits == WCS_IMPORTDATA_HUNITS_DEGREES)
#endif // WCS_BUILD_VNS
		{
		IWG->DXFinfo->OutputHighLon = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_X] + IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->OutputHighLat = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Y] + IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Y];
		IWG->DXFinfo->OutputLowLon = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->OutputLowLat = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Y];
		if (IWG->DXFinfo->ReverseX)
			{
			swmem(&IWG->DXFinfo->OutputLowLon, &IWG->DXFinfo->OutputHighLon, sizeof(double));
			IWG->DXFinfo->OutputLowLon = -IWG->DXFinfo->OutputLowLon;
			IWG->DXFinfo->OutputHighLon = -IWG->DXFinfo->OutputHighLon;
			} // if
		if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_LOWXY)
			{
			if (IWG->DXFinfo->ReverseX)
				{
				IWG->DXFinfo->OutputLowLon = RefLon - (IWG->DXFinfo->OutputHighLon - IWG->DXFinfo->OutputLowLon);
				IWG->DXFinfo->OutputHighLon = RefLon;
				} // if
			else
				{
				IWG->DXFinfo->OutputHighLon = RefLon + IWG->DXFinfo->OutputHighLon - IWG->DXFinfo->OutputLowLon;
				IWG->DXFinfo->OutputLowLon = RefLon;
				} // else
			IWG->DXFinfo->OutputHighLat = RefLat + IWG->DXFinfo->OutputHighLat - IWG->DXFinfo->OutputLowLat;
			IWG->DXFinfo->OutputLowLat = RefLat;
			} // if
		else if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_ZERO)
			{
			IWG->DXFinfo->OutputLowLon += RefLon;
			IWG->DXFinfo->OutputHighLon += RefLon;
			IWG->DXFinfo->OutputLowLat += RefLat;
			IWG->DXFinfo->OutputHighLat += RefLat;
			} // else if
		} // WCS_IMPORTDATA_HUNITS_DEGREES
	else // DXF in linear units
		{
#ifdef WCS_BUILD_VNS
		/***
		// km / km/deg = deg/km * km = deg/km
		Scale = (IWG->Importing->HScale / 1000) /
			LatScale(IWG->Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue);
		***/
		Scale = IWG->Importing->HScale;
#else // WCS_BUILD_VNS
		switch (IWG->DXFinfo->InputHUnits)
			{
			case WCS_IMPORTDATA_HUNITS_KILOMETERS:
				{
				Scale = ELSCALE_KILOM / EARTHLATSCALE;	// km/km / km/deg = deg/km * km/km = deg/km
				break;
				} // WCS_IMPORTDATA_HUNITS_KILOMETERS
			default:
			case WCS_IMPORTDATA_HUNITS_METERS:
				{
				Scale = ELSCALE_METERS / EARTHLATSCALE;	// km/m / km/deg = deg/km * km/m = deg/m
				break;
				} // WCS_IMPORTDATA_HUNITS_METERS
			case WCS_IMPORTDATA_HUNITS_DECIMETERS:
				{
				Scale = ELSCALE_DECIM / EARTHLATSCALE;	// km/dm / km/deg = deg/km * km/dm = deg/dm
				break;
				} // WCS_IMPORTDATA_HUNITS_DECIMETERS
			case WCS_IMPORTDATA_HUNITS_CENTIMETERS:
				{
				Scale = ELSCALE_CENTIM / EARTHLATSCALE;	// km/cm / km/deg = deg/km * km/cm = deg/cm
				break;
				} // WCS_IMPORTDATA_HUNITS_CENTIMETERS
			case WCS_IMPORTDATA_HUNITS_MILES:
				{
				Scale = ELSCALE_MILES / EARTHLATSCALE;	// km/mi / km/deg = deg/km * km/mi = deg/mi
				break;
				} // WCS_IMPORTDATA_HUNITS_MILES
			case WCS_IMPORTDATA_HUNITS_FEET:
				{
				Scale = ELSCALE_FEET / EARTHLATSCALE;	// km/ft / km/deg = deg/km * km/ft = deg/ft
				break;
				} // WCS_IMPORTDATA_HUNITS_FEET
			case WCS_IMPORTDATA_HUNITS_SURVEY_FEET:
				{
				Scale = 3.0480092E-004 / EARTHLATSCALE;	// km/ft / km/deg = deg/km * km/ft = deg/ft
				break;
				} // WCS_IMPORTDATA_HUNITS_SURVEY_FEET
			case WCS_IMPORTDATA_HUNITS_INCHES:
				{
				Scale = ELSCALE_INCHES / EARTHLATSCALE;	// km/in / km/deg = deg/km * km/in = deg/in
				break;
				} // WCS_IMPORTDATA_HUNITS_INCHES
			} // switch
#endif // WCS_BUILD_VNS
		IWG->DXFinfo->OutputLowLat = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_Y] * Scale;
		IWG->DXFinfo->OutputHighLat = IWG->DXFinfo->OutputLowLat + IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_Y] * Scale;
		if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_LOWXY)
			{
			IWG->DXFinfo->OutputHighLat = RefLat + IWG->DXFinfo->OutputHighLat - IWG->DXFinfo->OutputLowLat;
			IWG->DXFinfo->OutputLowLat = RefLat;
			} // if
		else if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_ZERO)
			{
			IWG->DXFinfo->OutputLowLat += RefLat;
			IWG->DXFinfo->OutputHighLat += RefLat;
			} // else if

#ifndef WCS_BUILD_VNS
		Scale /= (cos(PiOver180 * ((IWG->DXFinfo->OutputHighLat + IWG->DXFinfo->OutputLowLat) / 2.0)));
#endif // WCS_BUILD_VNS
		IWG->DXFinfo->OutputLowLon = IWG->DXFinfo->OutputLow[WCS_IMPORTDATA_COORD_X] * Scale;
		IWG->DXFinfo->OutputHighLon = IWG->DXFinfo->OutputLowLon + IWG->DXFinfo->OutputHigh[WCS_IMPORTDATA_COORD_X] * Scale;
		if (IWG->DXFinfo->ReverseX)
			{
			swmem(&IWG->DXFinfo->OutputLowLon, &IWG->DXFinfo->OutputHighLon, sizeof(double));
			IWG->DXFinfo->OutputLowLon = -IWG->DXFinfo->OutputLowLon;
			IWG->DXFinfo->OutputHighLon = -IWG->DXFinfo->OutputHighLon;
			} // if
		if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_LOWXY)
			{
			if (IWG->DXFinfo->ReverseX)
				{
				IWG->DXFinfo->OutputLowLon = RefLon - (IWG->DXFinfo->OutputHighLon - IWG->DXFinfo->OutputLowLon);
				IWG->DXFinfo->OutputHighLon = RefLon;
				} // if
			else
				{
				IWG->DXFinfo->OutputHighLon = RefLon + IWG->DXFinfo->OutputHighLon - IWG->DXFinfo->OutputLowLon;
				IWG->DXFinfo->OutputLowLon = RefLon;
				} // else
			} // if
		else if (IWG->DXFinfo->ZeroReference == WCS_IMPORTDATA_REFERENCE_ZERO)
			{
			IWG->DXFinfo->OutputLowLon += RefLon;
			IWG->DXFinfo->OutputHighLon += RefLon;
			} // else if
		} // User defined horizontal units

	// longitude scaling and reversal
	if (IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] != IWG->DXFinfo->OutputLowLon || IWG->DXFinfo->ReverseX ||
		IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_X] + IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X] != IWG->DXFinfo->OutputHighLon)
		{
		IWG->DXFinfo->InputXRange = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_X];
		IWG->DXFinfo->OutputXRange = IWG->DXFinfo->OutputHighLon - IWG->DXFinfo->OutputLowLon;
		if (IWG->DXFinfo->InputXRange != 0.0)
			{
			IWG->DXFinfo->OutputXScale = IWG->DXFinfo->OutputXRange / IWG->DXFinfo->InputXRange;
			if (IWG->DXFinfo->ReverseX)
				{
				IWG->DXFinfo->OutputXScale = -IWG->DXFinfo->OutputXScale;
				// swap high and low so that scaling arithmetic operates on high longitude value
				swmem(&IWG->DXFinfo->OutputLowLon, &IWG->DXFinfo->OutputHighLon, sizeof (double));
				} // if
			IWG->DXFinfo->ScaleLonFunc = ScaleLongitude;
			} // if
		else
			IWG->DXFinfo->ScaleLonFunc = NULL;
		} // if
	else
		IWG->DXFinfo->ScaleLonFunc = NULL;

	// latitude scaling
	if (IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] != IWG->DXFinfo->OutputLowLat ||
		IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Y] + IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y] != IWG->DXFinfo->OutputHighLat)
		{
		IWG->DXFinfo->InputYRange = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Y];
		IWG->DXFinfo->OutputYRange = IWG->DXFinfo->OutputHighLat - IWG->DXFinfo->OutputLowLat;
		if (IWG->DXFinfo->InputYRange != 0.0)
			{
			IWG->DXFinfo->OutputYScale = IWG->DXFinfo->OutputYRange / IWG->DXFinfo->InputYRange;
			IWG->DXFinfo->ScaleLatFunc = ScaleLatitude;
			} // if
		else
			IWG->DXFinfo->ScaleLatFunc = NULL;
		} // if
	else
		IWG->DXFinfo->ScaleLatFunc = NULL;

	if (IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Z] != IWG->DXFinfo->OutputLowElev ||
		IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Z] != IWG->DXFinfo->OutputHighElev)
		{
		IWG->DXFinfo->InputElRange = IWG->DXFinfo->InputHigh[WCS_IMPORTDATA_COORD_Z] - IWG->DXFinfo->InputLow[WCS_IMPORTDATA_COORD_Z];
		IWG->DXFinfo->OutputElRange = IWG->DXFinfo->OutputHighElev - IWG->DXFinfo->OutputLowElev;
		if (IWG->DXFinfo->InputElRange != 0.0)
			{
			IWG->DXFinfo->OutputElScale = IWG->DXFinfo->OutputElRange / IWG->DXFinfo->InputElRange;
			IWG->DXFinfo->ScaleElevFunc = ScaleElevation;
			} // if
		else
			IWG->DXFinfo->ScaleElevFunc = NULL;
		} // if
	else
		IWG->DXFinfo->ScaleElevFunc = NULL;

	IWG->DXFinfo->OutputWEMaps = IWG->DXFinfo->OutputCols > 300 ? IWG->DXFinfo->OutputCols / 300: 1;
	IWG->DXFinfo->OutputNSMaps = IWG->DXFinfo->OutputRows > 300 ? IWG->DXFinfo->OutputRows / 300: 1;
	} // if

// F2 - 03/19/02
#ifdef WCS_BUILD_VNS
IWG->DXFinfo->ScaleLatFunc = NULL;
IWG->DXFinfo->ScaleLonFunc = NULL;
IWG->DXFinfo->ScaleElevFunc = NULL;
#endif // WCS_BUILD_VNS

Skippy:
if (ScaleHook->UTMUnits)
	{
	RefSys = 1;
	UTMZone = ScaleHook->UTMZone;
	UTMLatLonCoords_Init(&VCoords.UTM, UTMZone);
	WestNeg = 0;
	} // if
else
	{
	RefSys = 0;
	WestNeg = ScaleHook->ReverseX;
	} // else
ElevUnits = 0;

MaxBounds[0] = -90.0;
MaxBounds[1] = 90.0;
MaxBounds[2] = 360.0;
MaxBounds[3] = -360.0;

fseek(fDXF, 0L, SEEK_END);
Position = new BusyWin ("Import DXF", ftell(fDXF), 'IDLG', 0);
fseek(fDXF, 0L, SEEK_SET);

if (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint))
	{
	// scan file for entities section
	while (! EntitiesFound && (fscanf(fDXF, "%hd", &GroupCode) != EOF))
		{
		//fscanf(fDXF, "%s", Str);
		fgetline(Str, 4096, fDXF);	// eat CR/LF
		fgetline(Str, 4096, fDXF);
		if (GroupCode == 0)
			InLayer = FALSE;
		if (InLayer || ((GroupCode == 0) && (stricmp(Str, "LAYER") == 0)))
			{
			InLayer = TRUE;
			switch (GroupCode)
				{
				case 2:
					if (HeadLayer == NULL)
						{
						HeadLayer = (struct DXFLayerPens *)AppMem_Alloc(sizeof(DXFLayerPens), APPMEM_CLEAR);
						LayerColors = HeadLayer;
						}
					else
						{
						LayerColors->Next = (struct DXFLayerPens *)AppMem_Alloc(sizeof(DXFLayerPens), APPMEM_CLEAR);
						LayerColors = LayerColors->Next;
						}
					strcpy(LayerColors->LayerName, Str);
					break;
				case 62:
					LayerColors->Pen = (short)atoi(Str);
					break;
				default:
					break;
				} // switch GroupCode
			} // if InLayer
		if (! stricmp(Str, "ENTITIES"))
			{
			EntitiesFound = 1;
			} // if Entities
		if (Position)
			{
			Position->Update(ftell(fDXF));
			BailOut += Position->CheckAbort();
			} // if
		} // while not entities section and not end of file

	// read entities
	if (EntitiesFound)
		{
		Str[0] = 0;
		while (! BailOut && CurrentPoint && Terminate != EOF && (DontRead || ((fscanf(fDXF, "%hd", &GroupCode) != EOF) && (fscanf(fDXF, "%s", Str) != EOF))))
			{
			DontRead = 0;
			switch (GroupCode)
				{
				case 0:
					if (! stricmp(Str, "POINT"))
						{
						CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
						CurrentPoint->Elevation = 0.0f;
						ElevRead = 1;
						while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
							{
							switch (GroupCode)
								{
								case 0:
									{
									PtCt ++;
									// if new entity is not a point go ahead and save the points and start a new entity
									if (stricmp(Str, "POINT"))
										{
										DontRead = 1;
										if (PtCt > 0)
											{
											// <<<>>> finish old object and start new one
											if (! Hook3D)
												{
												AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
													Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
													RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
												ObjsImported ++;
												} // if
											} // if
										PtCt = 0;
										CurrentPoint = Points;
										} // if
									else
										{
										if (PtCt >= NumPtsAllocated)
											{
											if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
												{
												Terminate = EOF;
												MemError = 1;
												break;	// break here since CurrentPoint will be set to NULL
												} // if no new points
											} // if need new points
										CurrentPoint = CurrentPoint->Next;
										if (Position)
											{
											Position->Update(ftell(fDXF));
											BailOut += Position->CheckAbort();
											} // if
										} // else continue reading points
									break;
									} // if new entity
								/*** I think somebody told me this can't happen
								case 1:
									{
									if (strcmp(Str, ObjName))
										{
										if (PtCt > 0)
											{
											// <<<>>> finish old object and start new one
											if (! Hook3D)
												{
												AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
													Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
													RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
												ObjsImported ++;
												} // if
											} // if
										CurrentPoint = Points;
										PtCt = 0;
										strcpy(ObjName, Str);
										} // if
									break;
									} // name
								***/
								case 8:
									{
									if (strcmp(Str, ObjLayer))
										{
										if (PtCt > 0)
											{
											// <<<>>> finish old object and start new one
											if (! Hook3D)
												{
												AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
													Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
													RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
												ObjsImported ++;
												} // if
											} // if
										CurrentPoint = Points;
										PtCt = 0;
										strcpy(ObjLayer, Str);
										strcpy(ObjName, Str);	// added since case 1 can't happen
										} // if
									break;
									} // layer
								case 10:
									{
									CurrentPoint->Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // X
								case 20:
									{
									CurrentPoint->Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // Y
								case 30:
								case 38:
									{
									if (IWG)
										{
										CurrentPoint->Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										CurrentPoint->Elevation = (float)(atof(Str));
										} // else
									break;
									} // Z
								case 62:
									aci = (short)atoi(Str);
									break;
								default:
									break;
								} // switch GroupCode
							} // while not EOF
						} // if POINT
					else if (! stricmp(Str, "LINE"))
						{
						CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
						CurrentPoint->Next->Longitude = CurrentPoint->Next->Latitude = 0.0;
						CurrentPoint->Elevation = 0.0f;
						CurrentPoint->Next->Elevation = 0.0f;
						ElevRead = 1;
						while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
							{
							switch (GroupCode)
								{
								case 0:
									{
									// <<<>>> finish old object and start new one
									if (! Hook3D)
										{
										AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
											Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
											RefSys, 1, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
										ObjsImported ++;
										} // if
									CurrentPoint = Points;
									PtCt = 0;
									DontRead = 1;
									break;
									} // if new entity
								case 1:
									{
									if (strcmp(Str, ObjName))
										{
										strcpy(ObjName, Str);
										} // if
									break;
									} // name
								case 8:
									{
									if (strcmp(Str, ObjLayer))
										{
										strcpy(ObjLayer, Str);
										} // if
									break;
									} // layer
								case 10:
									{
									PtCt = 1;
									CurrentPoint->Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // X
								case 20:
									{
									CurrentPoint->Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // Y
								case 30:
									{
									ElevRead = 1;
									if (IWG)
										{
										CurrentPoint->Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										CurrentPoint->Elevation = (float)(atof(Str));
										} // else
									break;
									} // Z
								case 38:
									{
									if (IWG)
										{
										CurrentPoint->Next->Elevation = CurrentPoint->Elevation =
											(float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										CurrentPoint->Next->Elevation = CurrentPoint->Elevation =
											(float)(atof(Str));
										} // else
									break;
									} // Z
								case 62:	// color number
									// to fix: 0 = BYBLOCK, 1..255 = pen number, never entering this section = BYLAYER
									aci = (short)atoi(Str);
									break;	// case 62
								case 11:
									{
									PtCt  = 2;
									CurrentPoint->Next->Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // X
								case 21:
									{
									CurrentPoint->Next->Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // Y
								case 31:
									{
									if (IWG)
										{
										CurrentPoint->Next->Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										CurrentPoint->Next->Elevation = (float)(atof(Str));
										} // else
									break;
									} // Z
								default:
									break;
								} // switch GroupCode
							} // while not EOF
						} // if LINE
					else if (! stricmp(Str, "POLYLINE"))
						{
						unsigned long Ignore = 0;
						CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
						CurrentPoint->Elevation = 0.0f;
						ReadingVertices = 0;
						ReadyToRead = 0;
						ElevRead = 0;
						PolyvertFlags = PolylineFlags = 0;
						PolyfaceVertex = PolyfaceFace = 0;
						PolyfaceMesh = 0;
						PointHolding = 0;
						PolyvertFlags = 0;
						DefaultElev = 0.0f;
						TempVtx.Longitude = TempVtx.Latitude = 0.0;
						TempVtx.Elevation = 0.0f;
						ObjLayer[0] = ObjName[0] = 0;
						while (! BailOut && ! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
							{
							switch (GroupCode)
								{
								case 0:
									{
									if (PointHolding)
										{
										if (! (PolyvertFlags & 16))
											{
											if (ReadyToRead)
												{
												if (! PolyfaceMesh || PolyfaceVertex)
													{
													CurrentPoint->Longitude = TempVtx.Longitude;
													CurrentPoint->Latitude = TempVtx.Latitude;
													if (ElevRead)
														CurrentPoint->Elevation = TempVtx.Elevation;
													else
														CurrentPoint->Elevation = DefaultElev;
													PtCt ++;
													if (PtCt >= NumPtsAllocated)
														{
														if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
															{
															Terminate = EOF;
															MemError = 1;
															break;	// break here since CurrentPoint will be set to NULL
															} // if no new points
														} // if need new points
													CurrentPoint = CurrentPoint->Next;
													} // if
												else if (Polygons && PolyfaceMesh && PolyfaceFace)
													{
													strncpy(Polygons[Num3DPolysRead].MaterialStr, ObjLayer, WCS_DXF_MAX_MATERIAL_NAMELEN);
													Polygons[Num3DPolysRead].MaterialStr[WCS_DXF_MAX_MATERIAL_NAMELEN - 1] = 0;
													Polygons[Num3DPolysRead].Index[0] = TempPoly.Index[0];
													Polygons[Num3DPolysRead].Index[1] = TempPoly.Index[1];
													Polygons[Num3DPolysRead].Index[2] = TempPoly.Index[2];
													Polygons[Num3DPolysRead].Index[3] = TempPoly.Index[3];
													TempPoly.Index[0] = TempPoly.Index[1] = TempPoly.Index[2] = TempPoly.Index[3] = 0;
													Num3DPolysRead ++;
													} // if
												} // if
											} // if not a control point vertex
										TempVtx.Longitude = TempVtx.Latitude = 0.0;
										TempVtx.Elevation = 0.0f;
										} // if
									PointHolding = 0;
									if (! stricmp(Str, "SEQEND"))
										{
										// <<<>>> finish old object and start new one
										if (! Hook3D)
											{
											if (! Ignore)
												{
												AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
													Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
													RefSys, 1, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
												ObjsImported ++;
												} // if
											} // if
										else if (Num3DPolysRead > 0)
											{
											ObjsImported = Hook3D->ProcessDXFInput(Polygons, Points, Num3DPolysRead, PtCt, 0);	// 0 indicates points don't need to be searched for redundancy
											BailOut = 1; // we're only reading one object per file for now
											} // else must be treated as a 3D object
										CurrentPoint = Points;
										PtCt = 0;
										Num3DPolysRead = 0;
										DontRead = 1;
										} // if done reading vertices
									else if (ReadyToRead)
										{
										if (! stricmp(Str, "VERTEX") || ! stricmp(Str, "INSERT"))
											{
											PointHolding = 1;
											ReadingVertices = 1;
											} // else if
										} // else if
									ElevRead = 0;
									PolyvertFlags = 0;
									PolyfaceVertex = PolyfaceFace = 0;
									Ignore = 0;
									break;
									} // if new entity
								case 1:
									{
									strcpy(ObjName, Str);
									break;
									} // name
								case 8:
									{
									strcpy(ObjLayer, Str);
									break;
									} // layer
								case 10:
									{
									TempVtx.Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // X
								case 20:
									{
									TempVtx.Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // Y
								case 30:
								case 38:
									{
									if (ReadingVertices)
										{
										ElevRead = 1;
										if (IWG)
											{
											TempVtx.Elevation = (float)(atof(Str) * IWG->Importing->VScale);
											} // if
										else
											{
											TempVtx.Elevation = (float)(atof(Str));
											} // else
										} // if
									else
										{
										if (IWG)
											{
											DefaultElev = (float)(atof(Str) * IWG->Importing->VScale);
											} // if
										else
											{
											DefaultElev = (float)(atof(Str));
											} // else
										} // else
									break;
									} // Z
								case 62:	// color number
									// to fix: 0 = BYBLOCK, 1..255 = pen number, never entering this section = BYLAYER
									aci = (short)atoi(Str);
									break;	// case 62
								case 66:
									{
									ReadyToRead = 1;
									break;
									} // Points Follow Flag
								case 70:
									{
									if (ReadingVertices)
										{
										PolyvertFlags = atoi(Str);
										PolyfaceFace = (PolyfaceMesh && (PolyvertFlags & 128));
										PolyfaceVertex = (PolyfaceFace && (PolyvertFlags & 64));
										if (PolyfaceVertex)
											PolyfaceFace = 0;
										if (PolyfaceFace && Hook3D)
											{
											if (Num3DPolysRead >= Num3DPolysAllocated)
												{
												if (! (Polygons = IncreaseCopyPolygonAllocation(Polygons, Num3DPolysAllocated)))
													{
													Terminate = EOF;
													MemError = 1;
													break;	// break here since we're doomed anyway
													} // if no new points
												} // if
											} // if this is a polyface Face
										} // if this is a vertex flag
									else
										{
										PolylineFlags = atoi(Str);
										PolyfaceMesh = (short)(PolylineFlags & 64);
										} // else this is a polyline header flag
									break;
									} // vertex Flag
								case 71:
									{
									TempPoly.Index[0] = abs(atoi(Str));	// we don't care about edge visibility (neg values)
									break;
									} // vertex index
								case 72:
									{
									TempPoly.Index[1] = abs(atoi(Str));
									break;
									} // vertex index
								case 73:
									{
									TempPoly.Index[2] = abs(atoi(Str));
									break;
									} // vertex index
								case 74:
									{
									TempPoly.Index[3] = abs(atoi(Str));
									break;
									} // vertex index
								case 210:
								case 220:
								case 230:
									{
									Ignore = 1;
									break;
									} // extrusions
								default:
									break;
								} // switch group code
							} // while not EOF
						} // if POLYLINE
					else if (! stricmp(Str, "LWPOLYLINE"))
						{
						CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
						CurrentPoint->Elevation = 0.0f;
						ReadingVertices = 1;
						ReadyToRead = 1;
						ElevRead = 0;
						PolyvertFlags = PolylineFlags = 0;
						PolyfaceVertex = PolyfaceFace = 0;
						PolyfaceMesh = 0;
						PointHolding = 1;
						PolyvertFlags = 0;
						DefaultElev = 0.0f;
						TempVtx.Longitude = TempVtx.Latitude = 0.0;
						TempVtx.Elevation = 0.0f;
						ObjLayer[0] = ObjName[0] = 0;
						while (! BailOut && ! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
							{
							switch (GroupCode)
								{
								case 0:
									{
									// <<<>>> finish old object and start new one
									AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
										Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
										RefSys, 1, ElevUnits, WestNeg, MaxBounds, IWG, HeadLayer, aci);
									ObjsImported ++;
									CurrentPoint = Points;
									PtCt = 0;
									Num3DPolysRead = 0;
									DontRead = 1;
									ElevRead = 0;
									PolyvertFlags = 0;
									PolyfaceVertex = PolyfaceFace = 0;
									break;
									} // if new entity
								case 1:
									{
									strcpy(ObjName, Str);
									break;
									} // name
								case 8:
									{
									strcpy(ObjLayer, Str);
									break;
									} // layer
								case 10:
									{
									TempVtx.Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // X
								case 20:
									{
									TempVtx.Latitude = atof(Str) * IWG->Importing->HScale;
									CurrentPoint->Longitude = TempVtx.Longitude;
									CurrentPoint->Latitude = TempVtx.Latitude;
									if (ElevRead)
										CurrentPoint->Elevation = TempVtx.Elevation;
									else
										CurrentPoint->Elevation = DefaultElev;
									PtCt ++;
									if (PtCt >= NumPtsAllocated)
										{
										if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
											{
											Terminate = EOF;
											MemError = 1;
											break;	// break here since CurrentPoint will be set to NULL
											} // if no new points
										} // if need new points
									CurrentPoint = CurrentPoint->Next;
									TempVtx.Longitude = TempVtx.Latitude = 0.0; // Elevation is constant for this entity
									break;
									} // Y
								case 30:
								case 38:
									{
									ElevRead = 1;
									if (IWG)
										{
										TempVtx.Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										TempVtx.Elevation = (float)(atof(Str));
										} // else
									break;
									} // Z
								case 62:	// color number
									// to fix: 0 = BYBLOCK, 1..255 = pen number, never entering this section = BYLAYER
									aci = (short)atoi(Str);
									break;	// case 62
								case 70:
									PolylineFlags = atoi(Str);
									PolyfaceMesh = (short)(PolylineFlags & 64);
									break; // 70: vertex Flag
								default:
									break;
								} // switch group code
							} // while not EOF
						} // if LWPOLYLINE
					else if (! stricmp(Str, "3DFACE"))
						{
						// <<<>>> the BOOL ReducePts will be set to TRUE if you need to do your magic
						Temp3DVtx[0].Longitude = Temp3DVtx[1].Longitude = Temp3DVtx[2].Longitude = Temp3DVtx[3].Longitude = 0.0;
						Temp3DVtx[0].Latitude = Temp3DVtx[1].Latitude = Temp3DVtx[2].Latitude = Temp3DVtx[3].Latitude = 0.0;
						Temp3DVtx[0].Elevation = Temp3DVtx[1].Elevation = Temp3DVtx[2].Elevation = Temp3DVtx[3].Elevation = 0.0f;
						Point3DHolding = 1;
						ObjLayer[0] = 0;
						ElevRead = FALSE;
						while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
							{
							switch (GroupCode)
								{
								case 0:
									{
									if (Point3DHolding)
										{
										if (PtCt + 4 >= NumPtsAllocated)
											{
											if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
												{
												Terminate = EOF;
												MemError = 1;
												break;	// break here since CurrentPoint will be set to NULL
												} // if no new points
											} // if need new points
										CurrentPoint->Longitude = Temp3DVtx[0].Longitude;
										CurrentPoint->Latitude = Temp3DVtx[0].Latitude;
										CurrentPoint->Elevation = Temp3DVtx[0].Elevation;
										PtCt ++;
										if (Hook3D)
											{
											if (Num3DPolysRead >= Num3DPolysAllocated)
												{
												if (! (Polygons = IncreaseCopyPolygonAllocation(Polygons, Num3DPolysAllocated)))
													{
													Terminate = EOF;
													MemError = 1;
													break;	// break here since we're doomed anyway
													} // if no new points
												} // if
											strncpy(Polygons[Num3DPolysRead].MaterialStr, ObjLayer, WCS_DXF_MAX_MATERIAL_NAMELEN);
											Polygons[Num3DPolysRead].MaterialStr[WCS_DXF_MAX_MATERIAL_NAMELEN - 1] = 0;
											Polygons[Num3DPolysRead].Index[0] = PtCt;	// index is one-based
											} // if
										PrevPoint = CurrentPoint;
										CurrentPoint = CurrentPoint->Next;
										for (Ct = 1; Ct < 4; Ct ++)
											{
											if (Temp3DVtx[Ct].Longitude != PrevPoint->Longitude || 
												Temp3DVtx[Ct].Latitude != PrevPoint->Latitude || 
												Temp3DVtx[Ct].Elevation != PrevPoint->Elevation)
												{
												CurrentPoint->Longitude = Temp3DVtx[Ct].Longitude;
												CurrentPoint->Latitude = Temp3DVtx[Ct].Latitude;
												CurrentPoint->Elevation = Temp3DVtx[Ct].Elevation;
												PtCt ++;
												if (Hook3D)
													{
													Polygons[Num3DPolysRead].Index[Ct] = PtCt;	// index is one-based
													} // if
												PrevPoint = CurrentPoint;
												CurrentPoint = CurrentPoint->Next;
												} // if
											else
												break;
											} // for
										if (Hook3D)
											Num3DPolysRead ++;
										} // if
									Point3DHolding = 0;
									if (stricmp(Str, "3DFACE"))
										{
										if (PtCt > 0)
											{
											// <<<>>> finish old object and start new one
											if (! Hook3D)
												{
												AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
													Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
													RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG);
												ObjsImported ++;
												} // if
											else if (Num3DPolysRead > 0)
												{
												ObjsImported = Hook3D->ProcessDXFInput(Polygons, Points, Num3DPolysRead, PtCt, 1);	// 0 indicates points don't need to be searched for redundancy
												BailOut = 1; // we're only reading one object per file for now
												} // else must be treated as a 3D object
											Num3DPolysRead = 0;
											} // if
										PtCt = 0;
										CurrentPoint = Points;
										DontRead = 1;
										} // if
									else
										{
										Temp3DVtx[0].Longitude = Temp3DVtx[1].Longitude = Temp3DVtx[2].Longitude = Temp3DVtx[3].Longitude = 0.0;
										Temp3DVtx[0].Latitude = Temp3DVtx[1].Latitude = Temp3DVtx[2].Latitude = Temp3DVtx[3].Latitude = 0.0;
										Temp3DVtx[0].Elevation = Temp3DVtx[1].Elevation = Temp3DVtx[2].Elevation = Temp3DVtx[3].Elevation = 0.0f;
										Point3DHolding = 1;
										ObjLayer[0] = 0;
										if (Position)
											{
											Position->Update(ftell(fDXF));
											BailOut += Position->CheckAbort();
											} // if
										} // else
									break;
									} // 0
								case 8:
									{
									strcpy(ObjLayer, Str);
									break;
									} // layer
								case 10:
									{
									Temp3DVtx[0].Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 10
								case 11:
									{
									Temp3DVtx[1].Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 11
								case 12:
									{
									Temp3DVtx[2].Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 12
								case 13:
									{
									Temp3DVtx[3].Longitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 13
								case 20:
									{
									Temp3DVtx[0].Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 20
								case 21:
									{
									Temp3DVtx[1].Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 21
								case 22:
									{
									Temp3DVtx[2].Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 22
								case 23:
									{
									Temp3DVtx[3].Latitude = atof(Str) * IWG->Importing->HScale;
									break;
									} // 23
								case 30:
									{
									if (IWG)
										{
										Temp3DVtx[0].Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										Temp3DVtx[0].Elevation = (float)(atof(Str));
										} // else
									ElevRead = TRUE;
									break;
									} // 30
								case 31:
									{
									if (IWG)
										{
										Temp3DVtx[1].Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										Temp3DVtx[1].Elevation = (float)(atof(Str));
										} // else
									ElevRead = TRUE;
									break;
									} // 31
								case 32:
									{
									if (IWG)
										{
										Temp3DVtx[2].Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										Temp3DVtx[2].Elevation = (float)(atof(Str));
										} // else
									ElevRead = TRUE;
									break;
									} // 32
								case 33:
									{
									if (IWG)
										{
										Temp3DVtx[3].Elevation = (float)(atof(Str) * IWG->Importing->VScale);
										} // if
									else
										{
										Temp3DVtx[3].Elevation = (float)(atof(Str));
										} // else
									ElevRead = TRUE;
									break;
									} // 33
								default:
									break;
								} // switch
							} // while
						} // if 3DFACE
					else if (! stricmp(Str, "ENDSEC"))
						{
						CurrentPoint = NULL;
						} // if ENDSEC end of entities section
					else if (! stricmp(Str, "EOF"))
						{
						CurrentPoint = NULL;
						} // if EOF end of file - shouldn't be hit before end of entities but... who knows?
					break;	// case 0
				case 62:	// Color number
					aci = (short)atoi(Str);
					break;	// case 62
				case 999:	// Comments
					break;	// case 999
				} // GroupCode
			if (Position)
				{
				Position->Update(ftell(fDXF));
				BailOut += Position->CheckAbort();
				} // if
			} // while not end of file
		} // if entities section
	GlobalApp->AppDB->MasterPoint.DeAllocate(Points);
	if (Polygons)
		AppMem_Free(Polygons, Num3DPolysAllocated * sizeof (struct DXFImportPolygon));
	if (FileParent && ObjsImported)
		{
		FileParent->NWLat = MaxBounds[0];
		FileParent->SELat = MaxBounds[1];
		FileParent->SELon = MaxBounds[2];
		FileParent->NWLon = MaxBounds[3];
		GlobalApp->AppDB->BoundUpTree(FileParent);
		} // if
	} // if Points allocated
else
	MemError = 1;

if (Position)
	{
	delete Position;
	Position = NULL;
	} // if

if (Terminate == EOF)
	{
	if (MemError)
		UserMessageOK("Import DXF", "Out of memory! Not all entities read correctly.");
	else
		UserMessageOK("Import DXF", "Premature end of file. Not all entities read correctly.");
	} // if file read error

while (HeadLayer)
	{
	LayerColors = HeadLayer;
	HeadLayer = LayerColors->Next;
	AppMem_Free(LayerColors, sizeof(DXFLayerPens));
	}
LayerColors = NULL;

return (ObjsImported);

} // ImportGISDXF

/*===========================================================================*/

unsigned long int Import3DObjDXF(FILE *fDXF, Joe *FileParent, ImportHook *IH, Object3DEffect *Hook3D)
{
double MaxBounds[4];
Joe *AfterGuy = NULL;
BusyWin *Position = NULL;
float DefaultElev = 0.0f;
unsigned long int PtCt, Ct, NumPtsAllocated, Num3DPolysAllocated, ObjsImported, Num3DPolysRead, PolylineFlags, PolyvertFlags;
short GroupCode, EntitiesFound = 0, Terminate = 1, DontRead = 0, MemError = 0, ReadyToRead, ReadingVertices, ElevRead,
	ElevUnits, WestNeg, RefSys, BailOut = 0, PolyfaceMesh, PolyfaceVertex, PolyfaceFace, PointHolding, Point3DHolding;
VectorPoint *Points = NULL, *CurrentPoint = NULL, *PrevPoint = NULL, TempVtx, Temp3DVtx[4];
struct DXFImportPolygon *Polygons = NULL, TempPoly;
char Str[256], ObjName[256], ObjLayer[256];

struct ImportData *ScaleHook = NULL;
ImportWizGUI *IWG = NULL;

// initialize
ObjName[0] = 0;
ObjLayer[0] = 0;
PtCt = NumPtsAllocated = 0;
ObjsImported = 0;
Num3DPolysAllocated = 0;
Num3DPolysRead = 0;
Point3DHolding = 0;
memset(&TempPoly, 0, sizeof (struct DXFImportPolygon));

if (Hook3D)
	{
	RefSys = 0;
	ElevUnits = 0;
	WestNeg = 0;
	} // if

MaxBounds[0] = -90.0;
MaxBounds[1] = 90.0;
MaxBounds[2] = 360.0;
MaxBounds[3] = -360.0;

fseek(fDXF, 0L, SEEK_END);
Position = new BusyWin ("Import DXF", ftell(fDXF), 'IDLG', 0);
fseek(fDXF, 0L, SEEK_SET);

if (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint))
	{
	// scan file for entities section
	while (! EntitiesFound && (fscanf(fDXF, "%hd", &GroupCode) != EOF))
		{
		fscanf(fDXF, "%s", Str);
		if (! stricmp(Str, "ENTITIES"))
			{
			EntitiesFound = 1;
			} // if
		if (Position)
			{
			Position->Update(ftell(fDXF));
			BailOut += Position->CheckAbort();
			} // if
		} // while not entities section and not end of file

	// read entities
	if (EntitiesFound)
		{
		Str[0] = 0;
		while (! BailOut && CurrentPoint && Terminate != EOF && (DontRead || ((fscanf(fDXF, "%hd", &GroupCode) != EOF) && (fscanf(fDXF, "%s", Str) != EOF))))
			{
			DontRead = 0;
			if (GroupCode == 0)
				{
				if (! stricmp(Str, "POINT"))
					{
					CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
					CurrentPoint->Elevation = 0.0f;
					ElevRead = 1;
					while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
						{
						switch (GroupCode)
							{
							case 0:
								{
								PtCt ++;
								// if new entity is not a point go ahead and save the points and start a new entity
								if (stricmp(Str, "POINT"))
									{
									DontRead = 1;
									if (PtCt > 0)
										{
										// <<<>>> finish old object and start new one
										if (! Hook3D)
											{
											AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
												Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
												RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG);
											ObjsImported ++;
											} // if
										} // if
									PtCt = 0;
									CurrentPoint = Points;
									} // if
								else
									{
									if (PtCt >= NumPtsAllocated)
										{
										if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
											{
											Terminate = EOF;
											MemError = 1;
											break;	// break here since CurrentPoint will be set to NULL
											} // if no new points
										} // if need new points
									CurrentPoint = CurrentPoint->Next;
									if (Position)
										{
										Position->Update(ftell(fDXF));
										BailOut += Position->CheckAbort();
										} // if
									} // else continue reading points
								break;
								} // if new entity
							case 1:
								{
								if (strcmp(Str, ObjName))
									{
									if (PtCt > 0)
										{
										// <<<>>> finish old object and start new one
										if (! Hook3D)
											{
											AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
												Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
												RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG);
											ObjsImported ++;
											} // if
										} // if
									CurrentPoint = Points;
									PtCt = 0;
									strcpy(ObjName, Str);
									} // if
								break;
								} // name
							case 8:
								{
								if (strcmp(Str, ObjLayer))
									{
									if (PtCt > 0)
										{
										// <<<>>> finish old object and start new one
										if (! Hook3D)
											{
											AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
												Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
												RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG);
											ObjsImported ++;
											} // if
										} // if
									CurrentPoint = Points;
									PtCt = 0;
									strcpy(ObjLayer, Str);
									} // if
								break;
								} // layer
							case 10:
								{
								CurrentPoint->Longitude = atof(Str);
								break;
								} // X
							case 20:
								{
								CurrentPoint->Latitude = atof(Str);
								break;
								} // Y
							case 30:
							case 38:
								{
								CurrentPoint->Elevation = (float)atof(Str);
								break;
								} // Z
							} // switch group code
						} // while not EOF
					} // if POINT
				else if (! stricmp(Str, "LINE"))
					{
					CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
					CurrentPoint->Next->Longitude = CurrentPoint->Next->Latitude = 0.0;
					CurrentPoint->Elevation = 0.0f;
					CurrentPoint->Next->Elevation = 0.0f;
					ElevRead = 1;
					while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
						{
						switch (GroupCode)
							{
							case 0:
								{
								// <<<>>> finish old object and start new one
								if (! Hook3D)
									{
									AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
										Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
										RefSys, 1, ElevUnits, WestNeg, MaxBounds, IWG);
									ObjsImported ++;
									} // if
								CurrentPoint = Points;
								PtCt = 0;
								DontRead = 1;
								break;
								} // if new entity
							case 1:
								{
								if (strcmp(Str, ObjName))
									{
									strcpy(ObjName, Str);
									} // if
								break;
								} // name
							case 8:
								{
								if (strcmp(Str, ObjLayer))
									{
									strcpy(ObjLayer, Str);
									} // if
								break;
								} // layer
							case 10:
								{
								PtCt = 1;
								CurrentPoint->Longitude = atof(Str);
								break;
								} // X
							case 20:
								{
								CurrentPoint->Latitude = atof(Str);
								break;
								} // Y
							case 30:
								{
								ElevRead = 1;
								CurrentPoint->Elevation = (float)atof(Str);
								break;
								} // Z
							case 38:
								{
								CurrentPoint->Next->Elevation = CurrentPoint->Elevation = (float)atof(Str);
								break;
								} // Z
							case 11:
								{
								PtCt  = 2;
								CurrentPoint->Next->Longitude = atof(Str);
								break;
								} // X
							case 21:
								{
								CurrentPoint->Next->Latitude = atof(Str);
								break;
								} // Y
							case 31:
								{
								CurrentPoint->Next->Elevation = (float)atof(Str);
								break;
								} // Z
							} // switch group code
						} // while not EOF
					} // if LINE
				else if (! stricmp(Str, "POLYLINE"))
					{
					CurrentPoint->Longitude = CurrentPoint->Latitude = 0.0;
					CurrentPoint->Elevation = 0.0f;
					ReadingVertices = 0;
					ReadyToRead = 0;
					ElevRead = 0;
					PolyvertFlags = PolylineFlags = 0;
					PolyfaceVertex = PolyfaceFace = 0;
					PolyfaceMesh = 0;
					PointHolding = 0;
					DefaultElev = 0.0f;
					TempVtx.Longitude = TempVtx.Latitude = 0.0;
					TempVtx.Elevation = 0.0f;
					ObjLayer[0] = ObjName[0] = 0;
					while (! BailOut && ! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
						{
						switch (GroupCode)
							{
							case 0:
								{
								if (PointHolding)
									{
									if (ReadyToRead)
										{
										if (! PolyfaceMesh || PolyfaceVertex)
											{
											CurrentPoint->Longitude = TempVtx.Longitude;
											CurrentPoint->Latitude = TempVtx.Latitude;
											if (ElevRead)
												CurrentPoint->Elevation = TempVtx.Elevation;
											else
												CurrentPoint->Elevation = DefaultElev;
											PtCt ++;
											if (PtCt >= NumPtsAllocated)
												{
												if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
													{
													Terminate = EOF;
													MemError = 1;
													break;	// break here since CurrentPoint will be set to NULL
													} // if no new points
												} // if need new points
											CurrentPoint = CurrentPoint->Next;
											} // if
										else if (Polygons && PolyfaceMesh && PolyfaceFace)
											{
											strncpy(Polygons[Num3DPolysRead].MaterialStr, ObjLayer, WCS_DXF_MAX_MATERIAL_NAMELEN);
											Polygons[Num3DPolysRead].MaterialStr[WCS_DXF_MAX_MATERIAL_NAMELEN - 1] = 0;
											Polygons[Num3DPolysRead].Index[0] = TempPoly.Index[0];
											Polygons[Num3DPolysRead].Index[1] = TempPoly.Index[1];
											Polygons[Num3DPolysRead].Index[2] = TempPoly.Index[2];
											Polygons[Num3DPolysRead].Index[3] = TempPoly.Index[3];
											TempPoly.Index[0] = TempPoly.Index[1] = TempPoly.Index[2] = TempPoly.Index[3] = 0;
											Num3DPolysRead ++;
											} // if
										} // if
									TempVtx.Longitude = TempVtx.Latitude = 0.0;
									TempVtx.Elevation = 0.0f;
									} // if
								PointHolding = 0;
								if (! stricmp(Str, "SEQEND"))
									{
									// <<<>>> finish old object and start new one
									if (! Hook3D)
										{
										AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
											Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
											RefSys, 1, ElevUnits, WestNeg, MaxBounds, IWG);
										ObjsImported ++;
										} // if
									else if (Num3DPolysRead > 0)
										{
										ObjsImported = Hook3D->ProcessDXFInput(Polygons, Points, Num3DPolysRead, PtCt, 0);	// 0 indicates points don't need to be searched for redundancy
										BailOut = 1; // we're only reading one object per file for now
										} // else must be treated as a 3D object
									CurrentPoint = Points;
									PtCt = 0;
									Num3DPolysRead = 0;
									DontRead = 1;
									} // if done reading vertices
								else if (ReadyToRead)
									{
									if (! stricmp(Str, "VERTEX") || ! stricmp(Str, "INSERT"))
										{
										PointHolding = 1;
										ReadingVertices = 1;
										} // else if
									} // else if
								ElevRead = 0;
								PolyvertFlags = 0;
								PolyfaceVertex = PolyfaceFace = 0;
								break;
								} // if new entity
							case 1:
								{
								strcpy(ObjName, Str);
								break;
								} // name
							case 8:
								{
								strcpy(ObjLayer, Str);
								break;
								} // layer
							case 10:
								{
								TempVtx.Longitude = atof(Str);
								break;
								} // X
							case 20:
								{
								TempVtx.Latitude = atof(Str);
								break;
								} // Y
							case 30:
							case 38:
								{
								if (ReadingVertices)
									{
									ElevRead = 1;
									TempVtx.Elevation = (float)atof(Str);
									} // if
								else
									DefaultElev = (float)atof(Str);
								break;
								} // Z
							case 66:
								{
								ReadyToRead = 1;
								break;
								} // Points Follow Flag
							case 70:
								{
								if (ReadingVertices)
									{
									PolyvertFlags = atoi(Str);
									PolyfaceFace = (PolyfaceMesh && (PolyvertFlags & 128));
									PolyfaceVertex = (PolyfaceFace && (PolyvertFlags & 64));
									if (PolyfaceVertex)
										PolyfaceFace = 0;
									if (PolyfaceFace && Hook3D)
										{
										if (Num3DPolysRead >= Num3DPolysAllocated)
											{
											if (! (Polygons = IncreaseCopyPolygonAllocation(Polygons, Num3DPolysAllocated)))
												{
												Terminate = EOF;
												MemError = 1;
												break;	// break here since we're doomed anyway
												} // if no new points
											} // if
										} // if this is a polyface Face
									} // if this is a vertex flag
								else
									{
									PolylineFlags = atoi(Str);
									PolyfaceMesh = (short)(PolylineFlags & 64);
									} // else this is a polyline header flag
								break;
								} // Points Follow Flag
							case 71:
								{
								TempPoly.Index[0] = abs(atoi(Str));	// we don't care about edge visibility (neg values)
								break;
								} // vertex index
							case 72:
								{
								TempPoly.Index[1] = abs(atoi(Str));
								break;
								} // vertex index
							case 73:
								{
								TempPoly.Index[2] = abs(atoi(Str));
								break;
								} // vertex index
							case 74:
								{
								TempPoly.Index[3] = abs(atoi(Str));

								break;
								} // vertex index
							} // switch group code
						} // while not EOF
					} // if POLYLINE
				else if (! stricmp(Str, "3DFACE"))
					{
					Temp3DVtx[0].Longitude = Temp3DVtx[1].Longitude = Temp3DVtx[2].Longitude = Temp3DVtx[3].Longitude = 0.0;
					Temp3DVtx[0].Latitude = Temp3DVtx[1].Latitude = Temp3DVtx[2].Latitude = Temp3DVtx[3].Latitude = 0.0;
					Temp3DVtx[0].Elevation = Temp3DVtx[1].Elevation = Temp3DVtx[2].Elevation = Temp3DVtx[3].Elevation = 0.0f;
					Point3DHolding = 1;
					ObjLayer[0] = 0;
					while (! DontRead && ((Terminate = fscanf(fDXF, "%hd", &GroupCode)) != EOF) && ((Terminate = fscanf(fDXF, "%s", Str)) != EOF))
						{
						switch (GroupCode)
							{
							case 0:
								{
								if (Point3DHolding)
									{
									if (PtCt + 4 >= NumPtsAllocated)
										{
										if (! (Points = IncreaseCopyPointAllocation(Points, NumPtsAllocated, CurrentPoint)))
											{
											Terminate = EOF;
											MemError = 1;
											break;	// break here since CurrentPoint will be set to NULL
											} // if no new points
										} // if need new points
									CurrentPoint->Longitude = Temp3DVtx[0].Longitude;
									CurrentPoint->Latitude = Temp3DVtx[0].Latitude;
									CurrentPoint->Elevation = Temp3DVtx[0].Elevation;
									PtCt ++;
									if (Hook3D)
										{
										if (Num3DPolysRead >= Num3DPolysAllocated)
											{
											if (! (Polygons = IncreaseCopyPolygonAllocation(Polygons, Num3DPolysAllocated)))
												{
												Terminate = EOF;
												MemError = 1;
												break;	// break here since we're doomed anyway
												} // if no new points
											} // if
										strncpy(Polygons[Num3DPolysRead].MaterialStr, ObjLayer, WCS_DXF_MAX_MATERIAL_NAMELEN);
										Polygons[Num3DPolysRead].MaterialStr[WCS_DXF_MAX_MATERIAL_NAMELEN - 1] = 0;
										Polygons[Num3DPolysRead].Index[0] = PtCt;	// index is one-based
										} // if
									PrevPoint = CurrentPoint;
									CurrentPoint = CurrentPoint->Next;
									for (Ct = 1; Ct < 4; Ct ++)
										{
										if (Temp3DVtx[Ct].Longitude != PrevPoint->Longitude || 
											Temp3DVtx[Ct].Latitude != PrevPoint->Latitude || 
											Temp3DVtx[Ct].Elevation != PrevPoint->Elevation)
											{
											CurrentPoint->Longitude = Temp3DVtx[Ct].Longitude;
											CurrentPoint->Latitude = Temp3DVtx[Ct].Latitude;
											CurrentPoint->Elevation = Temp3DVtx[Ct].Elevation;
											PtCt ++;
											if (Hook3D)
												{
												Polygons[Num3DPolysRead].Index[Ct] = PtCt;	// index is one-based
												} // if
											PrevPoint = CurrentPoint;
											CurrentPoint = CurrentPoint->Next;
											} // if
										else
											break;
										} // for
									if (Hook3D)
										Num3DPolysRead ++;
									} // if
								Point3DHolding = 0;
								if (! stricmp(Str, "ENDSEC"))	// end of entities section
									{
									if (PtCt > 0)
										{
										// Finish old object
										if (! Hook3D)
											{
											AfterGuy = PrepnSaveDXFObject(FileParent, AfterGuy, IH, ScaleHook,
												Points, ObjName, ObjLayer, PtCt, DefaultElev, ElevRead,
												RefSys, 0, ElevUnits, WestNeg, MaxBounds, IWG);
											ObjsImported ++;
											} // if
										else if (Num3DPolysRead > 0)
											{
											ObjsImported = Hook3D->ProcessDXFInput(Polygons, Points, Num3DPolysRead, PtCt, 1);	// 0 indicates points don't need to be searched for redundancy
											BailOut = 1; // we're only reading one object per file for now
											} // else must be treated as a 3D object
										Num3DPolysRead = 0;
										} // if
									PtCt = 0;
									CurrentPoint = Points;
									DontRead = 1;
									} // if
								else if (stricmp(Str, "3DFACE"))
									{
									// don't think we need do anything, we're going to ignore this thing
									} // if
								else
									{
									Temp3DVtx[0].Longitude = Temp3DVtx[1].Longitude = Temp3DVtx[2].Longitude = Temp3DVtx[3].Longitude = 0.0;
									Temp3DVtx[0].Latitude = Temp3DVtx[1].Latitude = Temp3DVtx[2].Latitude = Temp3DVtx[3].Latitude = 0.0;
									Temp3DVtx[0].Elevation = Temp3DVtx[1].Elevation = Temp3DVtx[2].Elevation = Temp3DVtx[3].Elevation = 0.0f;
									Point3DHolding = 1;
									ObjLayer[0] = 0;
									if (Position)
										{
										Position->Update(ftell(fDXF));
										BailOut += Position->CheckAbort();
										} // if
									} // else
								break;
								} // 0
							case 8:
								{
								strcpy(ObjLayer, Str);
								break;
								} // layer
							case 10:
								{
								Temp3DVtx[0].Longitude = atof(Str);
								break;
								} // 10
							case 11:
								{
								Temp3DVtx[1].Longitude = atof(Str);
								break;
								} // 11
							case 12:
								{
								Temp3DVtx[2].Longitude = atof(Str);
								break;
								} // 12
							case 13:
								{
								Temp3DVtx[3].Longitude = atof(Str);
								break;
								} // 13
							case 20:
								{
								Temp3DVtx[0].Latitude = atof(Str);
								break;
								} // 20
							case 21:
								{
								Temp3DVtx[1].Latitude = atof(Str);
								break;
								} // 21
							case 22:
								{
								Temp3DVtx[2].Latitude = atof(Str);
								break;
								} // 22
							case 23:
								{
								Temp3DVtx[3].Latitude = atof(Str);
								break;
								} // 23
							case 30:
								{
								Temp3DVtx[0].Elevation = (float)atof(Str);
								break;
								} // 30
							case 31:
								{
								Temp3DVtx[1].Elevation = (float)atof(Str);
								break;
								} // 31
							case 32:
								{
								Temp3DVtx[2].Elevation = (float)atof(Str);
								break;
								} // 32
							case 33:
								{
								Temp3DVtx[3].Elevation = (float)atof(Str);
								break;
								} // 33
							} // switch
						} // while
					} // if 3DFACE
				else if (! stricmp(Str, "ENDSEC"))
					{
					CurrentPoint = NULL;
					} // if ENDSEC end of entities section
				else if (! stricmp(Str, "EOF"))
					{
					CurrentPoint = NULL;
					} // if EOF end of file - shouldn't be hit before end of entities but... who knows?
				} // if
			if (Position)
				{
				Position->Update(ftell(fDXF));
				BailOut += Position->CheckAbort();
				} // if
			} // while not end of file
		} // if entities section
	GlobalApp->AppDB->MasterPoint.DeAllocate(Points);
	if (Polygons)
		AppMem_Free(Polygons, Num3DPolysAllocated * sizeof (struct DXFImportPolygon));
	if (FileParent && ObjsImported)
		{
		FileParent->NWLat = MaxBounds[0];
		FileParent->SELat = MaxBounds[1];
		FileParent->SELon = MaxBounds[2];
		FileParent->NWLon = MaxBounds[3];
		GlobalApp->AppDB->BoundUpTree(FileParent);
		} // if
	} // if Points allocated
else
	MemError = 1;

if (Position)
	{
	delete Position;
	Position = NULL;
	} // if

if (Terminate == EOF)
	{
	if (MemError)
		UserMessageOK("Import 3DObj DXF", "Out of memory! Not all entities read correctly.");
	else
		UserMessageOK("Import 3DObj DXF", "Premature end of file. Not all entities read correctly.");
	} // if file read error

return (ObjsImported);

} // Import3DObjDXF

/*===========================================================================*/

int Database::ExportDXF(Project *Proj, JoeApproveHook *JAH, CoordSys *OutCoords, double exportXYScale, double exportZScale, int FlipLon)
{
//double NClamp, SClamp, EClamp, WClamp;
double MaxVal[3], MinVal[3], NWLat, NWLon, SELat, SELon, PtLat, PtLon;
BusyWin *BWDX = NULL;
CoordSys *MyCoords;
FILE *fOutFile;
Joe *CurrentJoe;
JoeCoordSys *MyAttr;
JoeDEM *MyDEM;
LayerEntry *CurrentLayer;
VectorPoint *PLink;
int Success = 1;
DEM Topo;
float MaxEl, MinEl, SampEl, SampElLat, SampElOne, SampElLatOne;
long ObjectsFound = 0;
unsigned long PtCt, Lr, Lc;
//double time1, time2, totaltime;
//char timemsg[80];
VertexDEM Vert;
VertexData VTD;
int NumWrite = 0;
#ifdef WCS_VIEW_TERRAFFECTORS
PlanetOpt *DefPO;
PolygonData PGD;
RenderData Rend(NULL);
int PreviewTFX = 1; // allow for future disabling in GUI
char OutPath[256], OutName[64], Ptrn[32], filename[256];

DefPO = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL);

// Setup for terraffector eval
Rend.Interactive = GlobalApp->MainProj->Interactive;
Rend.PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
Rend.EarthLatScaleMeters = LatScale(Rend.PlanetRad);
Rend.EffectsBase = GlobalApp->AppEffects;
Rend.ElevDatum = DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
Rend.Exageration = DefPO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
Rend.DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Rend.TexRefLon = Rend.Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
Rend.TexRefLat = Rend.Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
Rend.TexRefElev = Rend.Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
Rend.RefLonScaleMeters = Rend.EarthLatScaleMeters * cos(Rend.TexRefLat * PiOver180);
Rend.TexData.MetersPerDegLat = Rend.EarthLatScaleMeters;
Rend.TexData.Datum = Rend.ElevDatum;
Rend.TexData.Exageration = Rend.Exageration;
Rend.ExagerateElevLines = DefPO->EcoExageration;
Rend.DBase = this;

#endif // WCS_VIEW_TERRAFFECTORS

//time1 = GetSystemTimeFP();
MaxVal[0] = MaxVal[1] = MaxVal[2] = -FLT_MAX;
MinVal[0] = MinVal[1] = MinVal[2] = FLT_MAX;

strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("dxf"));
if (Proj->dirname[0])
	strcpy(OutPath, Proj->dirname);
else if (Proj->projectpath[0])
	strcpy(OutPath, Proj->projectpath);
else
	strcpy(OutPath, "WCSProjects:");
if (Proj->projectname[0])
	strcpy(OutName, Proj->projectname);
else
	strcpy(OutName, "NoName");
(void)StripExtension(OutName);
strcat(OutName, ".dxf");

if (GetFileNamePtrn(1, "Export DXF", OutPath, OutName, Ptrn, 64))
	{
	strmfp(filename, OutPath, OutName);
	if (fOutFile = PROJ_fopen(filename, "w"))
		{
		strcpy(Proj->dbasepath, OutPath);
		strcpy(Proj->dbasename, OutName);
		(void)StripExtension(Proj->dbasename);
		ResetGeoClip();
		// first determine the object extents for the header
		for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
			{
			if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				JAH->ApproveMe = CurrentJoe;
				if (JAH->Approve(JAH))
					{
					NumWrite ++;
					if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (MyDEM = (JoeDEM *)CurrentJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
							{
							NumWrite += 100;
							if (MyDEM->MaxEl > MaxVal[2])
								MaxVal[2] = MyDEM->MaxEl;
							if (MyDEM->MinEl < MinVal[2])
								MinVal[2] = MyDEM->MinEl;
							if (CurrentJoe->NWLat > MaxVal[1])
								MaxVal[1] = CurrentJoe->NWLat;
							if (CurrentJoe->SELat < MinVal[1])
								MinVal[1] = CurrentJoe->SELat;
							if (CurrentJoe->NWLon > MaxVal[0])
								MaxVal[0] = CurrentJoe->NWLon;
							if (CurrentJoe->SELon < MinVal[0])
								MinVal[0] = CurrentJoe->SELon;
							ObjectsFound ++;
							} // if
						} // if
					else
						{
						CurrentJoe->GetElevRange(MaxEl, MinEl);
						CurrentJoe->GetTrueBounds(NWLat, NWLon, SELat, SELon);

#if WCS_BUILD_VNS
						VertexDEM vert;
						bool convert = OutCoords && !OutCoords->GetGeographic();

						// To be technically correct, we should project all four corners at least, but this should be
						// close enough for what we're trying to accomplish (getting reasonable bounds so AutoCAD can
						// center the display on the export).
						vert.Lat = NWLat;
						vert.Lon = NWLon;
						vert.Elev = MinEl;
						if (convert)
							{
							OutCoords->DefDegToProj(&vert);
							vert.Lon = vert.xyz[0] * exportXYScale;
							vert.Lat = vert.xyz[1] * exportXYScale;
							vert.Elev = (float)(vert.xyz[2] * exportZScale);
							} // if

						if (vert.Elev > MaxVal[2])
							MaxVal[2] = MaxEl;
						if (vert.Elev < MinVal[2])
							MinVal[2] = MinEl;
						if (vert.Lat > MaxVal[1])
							MaxVal[1] = vert.Lat;
						if (vert.Lat < MinVal[1])
							MinVal[1] = vert.Lat;
						if (vert.Lon > MaxVal[0])
							MaxVal[0] = vert.Lon;
						if (vert.Lon < MinVal[0])
							MinVal[0] = vert.Lon;

						vert.Lat = SELat;
						vert.Lon = SELon;
						vert.Elev = MaxEl;
						if (convert)
							{
							OutCoords->DefDegToProj(&vert);
							vert.Lon = vert.xyz[0] * exportXYScale;
							vert.Lat = vert.xyz[1] * exportXYScale;
							vert.Elev = (float)(vert.xyz[2] * exportZScale);
							} // if

						if (vert.Elev > MaxVal[2])
							MaxVal[2] = MaxEl;
						if (vert.Elev < MinVal[2])
							MinVal[2] = MinEl;
						if (vert.Lat > MaxVal[1])
							MaxVal[1] = vert.Lat;
						if (vert.Lat < MinVal[1])
							MinVal[1] = vert.Lat;
						if (vert.Lon > MaxVal[0])
							MaxVal[0] = vert.Lon;
						if (vert.Lon < MinVal[0])
							MinVal[0] = vert.Lon;
#else // WCS_BUILD_VNS
						if (MaxEl > MaxVal[2])
							MaxVal[2] = MaxEl;
						if (MinEl < MinVal[2])
							MinVal[2] = MinEl;
						if (NWLat > MaxVal[1])
							MaxVal[1] = NWLat;
						if (SELat < MinVal[1])
							MinVal[1] = SELat;
						if (NWLon > MaxVal[0])
							MaxVal[0] = NWLon;
						if (SELon < MinVal[0])
							MinVal[0] = SELon;
#endif // WCS_BUILD_VNS
						ObjectsFound ++;
						} // else
					} // if
				} // if
			} // for
		if (ObjectsFound)
			{
			BWDX = new BusyWin("Writing", NumWrite, 'BWDX', 0);
			NumWrite = 0;
			if (FlipLon)
				{
				MaxVal[0] *= -1.0;
				MinVal[0] *= -1.0;
				Swap64(MaxVal[0], MinVal[0]);
				}
			// header stuff
			if (PrintDXFSection(fOutFile, "HEADER"))
				{
				// identify the DXF output as being R12
				PrintDXFTextValue(fOutFile, 9, "$ACADVER");
				PrintDXFTextValue(fOutFile, 1, "AC1009");
				// put in the extents
				PrintDXFTextValue(fOutFile, 9, "$EXTMIN");
				PrintDXFFltValue(fOutFile, 10, MinVal[0]);
				PrintDXFFltValue(fOutFile, 20, MinVal[1]);
				PrintDXFFltValue(fOutFile, 30, MinVal[2]);
				PrintDXFTextValue(fOutFile, 9, "$EXTMAX");
				PrintDXFFltValue(fOutFile, 10, MaxVal[0]);
				PrintDXFFltValue(fOutFile, 20, MaxVal[1]);
				PrintDXFFltValue(fOutFile, 30, MaxVal[2]);
				PrintDXFTextValue(fOutFile, 9, "$LIMMIN");
				PrintDXFFltValue(fOutFile, 10, MinVal[0]);
				PrintDXFFltValue(fOutFile, 20, MinVal[1]);
				PrintDXFTextValue(fOutFile, 9, "$LIMMAX");
				PrintDXFFltValue(fOutFile, 10, MaxVal[0]);
				PrintDXFFltValue(fOutFile, 20, MaxVal[1]);
				PrintDXFEndSec(fOutFile);
				// table stuff
				if (PrintDXFSection(fOutFile, "TABLES"))
					{
					PrintDXFTableType(fOutFile, "LTYPE");
					PrintDXFIntValue(fOutFile, 70, 1);
					PrintDXFTextValue(fOutFile, 0, "LTYPE");
					PrintDXFTextValue(fOutFile, 2, "CONTINUOUS");
					PrintDXFIntValue(fOutFile, 70, 64);
					PrintDXFTextValue(fOutFile, 3, "Solid Line");
					PrintDXFIntValue(fOutFile, 72, 65);
					PrintDXFIntValue(fOutFile, 73, 0);
					PrintDXFFltValue(fOutFile, 40, 0.0);
					/* example from Onyx Tree file
					70
					1
					0
					LTYPE
					2
					CONTINUOUS
					70
					64
					3
					Solid Line
					72
					65
					73
					0
					40
					0.0
					*/
					PrintDXFEndTab(fOutFile);
					// layer stuff
					PrintDXFTableType(fOutFile, "LAYER");
					PrintDXFIntValue(fOutFile, 70, 12);
					for (CurrentLayer = DBLayers.FirstEntry(); CurrentLayer; CurrentLayer = DBLayers.NextEntry(CurrentLayer))
						{
						if (!CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
							{
							for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
								{
								if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
									{
									JAH->ApproveMe = CurrentJoe;
									if (JAH->Approve(JAH))
										{
										if (CurrentJoe->MatchEntryToStub(CurrentLayer))
											{
											PrintDXFTextValue(fOutFile, 0, "LAYER");
											PrintDXFTextValue(fOutFile, 2, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
											//PrintDXFTextValue(fOutFile, 2, (char *)CurrentLayer->GetName());
											PrintDXFIntValue(fOutFile, 70, 64);
											PrintDXFIntValue(fOutFile, 62, 10);
											PrintDXFTextValue(fOutFile, 6, "CONTINUOUS");
											/*  example from Onyx Tree file incl layer table header flags
											70
											12
											0
											LAYER
											2
											Name
											70
											64
											62
											10
											6
											CONTINUOUS
											*/
											// don't need to test any more Joes for this layer
											break;
											} // if object is in current layer
										} // if approved for output
									} // if not group object
								} // for each object in database
							} // if not an attribute
						} // for each layer in database
					PrintDXFEndTab(fOutFile);
					PrintDXFEndSec(fOutFile);
					// entities stuff
					if (PrintDXFSection(fOutFile, "ENTITIES"))
						{
						for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
							{
							if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
								{
								JAH->ApproveMe = CurrentJoe;
								if (JAH->Approve(JAH))
									{
									NumWrite++;
									BWDX->Update(NumWrite);
									if (BWDX->CheckAbort())
										{
										break; // bail out of entities loop, but still write end of file
										}
									if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
										{
										if (Topo.AttemptLoadDEM(CurrentJoe, 1, Proj))
											{
											// I believe TransferToVerticesLatLon is safe to do in non-Coordinate-System-aware builds
											// Ensure that the point remains within the DEM after whatever precision errors occur during coord transforms
											// so that VertexDataPoint will find a valid elevation
											Topo.TransferToVerticesLatLon(TRUE);
											// precision errors cause FindAndLoadDEM to flush too often
											// This problem is believed fixed by ensuring the vertices fall within 
											// the DEM bounds in TransferToVerticesLatLon()
											//NClamp = Topo.Northest();
											//SClamp = Topo.Southest();
											//EClamp = Topo.Eastest();
											//WClamp = Topo.Westest();
											switch (JAH->ExportDEMsAs)
												{
												case WCS_DBEXPORT_DEMSAS_POINTS:
													{
													for (PtCt = 0, Lr = 0; Lr < Topo.LonEntries(); Lr ++)
														{
														int Ratio;
														Ratio = (int)(((double)Lr / (double)Topo.LonEntries()) * 100.0);
														BWDX->Update(NumWrite + Ratio);
														if (BWDX->CheckAbort())
															{
															break; // bail out of DEM loop, full abort on next CheckAbort() call.
															}
														//PtLon = Topo.Westest() - Lr * Topo.LonStep();
														for (Lc = 0; Lc < Topo.LatEntries(); Lc ++, PtCt ++)
															{
															int gotValue;

															PtLon = Topo.Vertices[PtCt].Lon;
															PtLat = Topo.Vertices[PtCt].Lat;
															//PtLat = Topo.Southest() + Lc * Topo.LatStep();
															//PrintDXFNameOrLayer(fOutFile, CurrentJoe);
															VTD.Lat = PtLat;
															VTD.Lon = PtLon;
															#ifdef WCS_VIEW_TERRAFFECTORS
															//if (VTD.Lat < SClamp)
															//	VTD.Lat = SClamp;
															//if (VTD.Lat > NClamp)
															//	VTD.Lat = NClamp;
															//if (VTD.Lon < EClamp)
															//	VTD.Lon = EClamp;
															//if (VTD.Lon > WClamp)
															//	VTD.Lon = WClamp;
															gotValue = GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
															 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
															SampEl = (float)UnCalcExag(VTD.Elev, DefPO);
															#else // !WCS_VIEW_TERRAFFECTORS
															SampEl = (float)Topo.Vertices[PtCt].Elev;
															gotValue = 1;
															#endif // !WCS_VIEW_TERRAFFECTORS
															if (gotValue)
																{
																PrintDXFTextValue(fOutFile, 0, "POINT");
																PrintDXFTextValue(fOutFile, 8, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
																if (! PrintDXFTriple(fOutFile, 0, (FlipLon ? -PtLon : PtLon), PtLat, SampEl, OutCoords, exportXYScale, exportZScale))
																	{
																	Success = 0;
																	goto FileError;
																	} // if error
																} // if gotValue
															} // for Lc
														} // for Lr
													break;
													} // WCS_DBEXPORT_DEMSAS_POINTS
												case WCS_DBEXPORT_DEMSAS_3DFACES:
													{
													for (PtCt = 0, Lr = 0; Lr < Topo.LonEntries() - 1; Lr ++)
														{
														int Ratio;
														Ratio = (int)(((double)Lr / (double)Topo.LonEntries()) * 100.0);
														BWDX->Update(NumWrite + Ratio);
														if (BWDX->CheckAbort())
															{
															break; // bail out of DEM loop, full abort on next CheckAbort() call.
															}
														//PtLon = Topo.Westest() - Lr * Topo.LonStep();
														for (Lc = 0; Lc < Topo.LatEntries() - 1; Lc ++, PtCt ++)
															{
															PtLon = Topo.Vertices[PtCt].Lon;
															PtLat = Topo.Vertices[PtCt].Lat;
															//PtLat = Topo.Southest() + Lc * Topo.LatStep();
															PrintDXFTextValue(fOutFile, 0, "3DFACE");
															//PrintDXFTextValue(fOutFile, 1, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
															//PrintDXFNameOrLayer(fOutFile, CurrentJoe);
															PrintDXFTextValue(fOutFile, 8, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
															VTD.Lat = PtLat;
															VTD.Lon = PtLon;
															#ifdef WCS_VIEW_TERRAFFECTORS
															GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
															 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
															SampEl = (float)UnCalcExag(VTD.Elev, DefPO);
															#else // !WCS_VIEW_TERRAFFECTORS
															SampEl = (float)Topo.Vertices[PtCt].Elev;
															#endif // !WCS_VIEW_TERRAFFECTORS
															PrintDXFTriple(fOutFile, 0, (FlipLon ? -PtLon : PtLon), PtLat, SampEl, OutCoords, exportXYScale, exportZScale);
															//PrintDXFTriple(fOutFile, 0, PtLon, PtLat, SampEl);

															VTD.Lat = PtLat;
															VTD.Lon = Topo.Vertices[PtCt + Topo.LatEntries()].Lon;
															//VTD.Lon = PtLon - Topo.LonStep();
															#ifdef WCS_VIEW_TERRAFFECTORS
															GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
															 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
															SampElLat = (float)UnCalcExag(VTD.Elev, DefPO);
															#else // !WCS_VIEW_TERRAFFECTORS
															SampElLat = (float)Topo.Vertices[PtCt + Topo.LatEntries()].Elev;
															#endif // !WCS_VIEW_TERRAFFECTORS
															PrintDXFTriple(fOutFile, 1, (FlipLon ? -VTD.Lon : VTD.Lon), PtLat, SampElLat, OutCoords, exportXYScale, exportZScale);
															//PrintDXFTriple(fOutFile, 1, VTD.Lon, PtLat, SampElLat);

															VTD.Lat = Topo.Vertices[PtCt + 1].Lat;
															//VTD.Lat = PtLat + Topo.LatStep();
															VTD.Lon = PtLon;
															#ifdef WCS_VIEW_TERRAFFECTORS
															GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
															 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
															SampElOne = (float)UnCalcExag(VTD.Elev, DefPO);
															#else // !WCS_VIEW_TERRAFFECTORS
															SampElOne = (float)Topo.Vertices[PtCt + 1].Elev;
															#endif // !WCS_VIEW_TERRAFFECTORS
															PrintDXFTriple(fOutFile, 2, (FlipLon ? -PtLon : PtLon), VTD.Lat, SampElOne, OutCoords, exportXYScale, exportZScale);
															PrintDXFTriple(fOutFile, 3, (FlipLon ? -PtLon : PtLon), VTD.Lat, SampElOne, OutCoords, exportXYScale, exportZScale);
															//PrintDXFTriple(fOutFile, 2, PtLon, VTD.Lat, SampElOne);
															//PrintDXFTriple(fOutFile, 3, PtLon, VTD.Lat, SampElOne);
															PrintDXFTextValue(fOutFile, 0, "3DFACE");
															//PrintDXFTextValue(fOutFile, 1, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
															//PrintDXFNameOrLayer(fOutFile, CurrentJoe);
															PrintDXFTextValue(fOutFile, 8, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));

															VTD.Lat = Topo.Vertices[PtCt + 1].Lat;
															VTD.Lon = Topo.Vertices[PtCt + Topo.LatEntries()].Lon;
															//VTD.Lat = PtLat + Topo.LatStep();
															//VTD.Lon = PtLon - Topo.LonStep();
															#ifdef WCS_VIEW_TERRAFFECTORS
															GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
															 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
															SampElLatOne = (float)UnCalcExag(VTD.Elev, DefPO);
															#else // !WCS_VIEW_TERRAFFECTORS
															SampElLatOne = (float)Topo.Vertices[PtCt + 1 + Topo.LatEntries()].Elev;
															#endif // !WCS_VIEW_TERRAFFECTORS
															//PrintDXFTriple(fOutFile, 0, PtLon - Topo.LonStep(), PtLat + Topo.LatStep(), SampElLatOne);
															//PrintDXFTriple(fOutFile, 1, PtLon, PtLat + Topo.LatStep(), SampElOne);
															//PrintDXFTriple(fOutFile, 2, PtLon - Topo.LonStep(), PtLat, SampElLat);
															//if (! PrintDXFTriple(fOutFile, 3, PtLon - Topo.LonStep(), PtLat, SampElLat))
															//PrintDXFTriple(fOutFile, 0, VTD.Lon, VTD.Lat, SampElLatOne);
															//PrintDXFTriple(fOutFile, 1, PtLon, VTD.Lat, SampElOne);
															//PrintDXFTriple(fOutFile, 2, VTD.Lon, PtLat, SampElLat);
															//if (! PrintDXFTriple(fOutFile, 3, VTD.Lon, PtLat, SampElLat))
															PrintDXFTriple(fOutFile, 0, (FlipLon ? -VTD.Lon : VTD.Lon), VTD.Lat, SampElLatOne, OutCoords, exportXYScale, exportZScale);
															PrintDXFTriple(fOutFile, 1, (FlipLon ? -PtLon : PtLon), VTD.Lat, SampElOne, OutCoords, exportXYScale, exportZScale);
															PrintDXFTriple(fOutFile, 2, (FlipLon ? -VTD.Lon : VTD.Lon), PtLat, SampElLat, OutCoords, exportXYScale, exportZScale);
															if (! PrintDXFTriple(fOutFile, 3, (FlipLon ? -VTD.Lon : VTD.Lon), PtLat, SampElLat, OutCoords, exportXYScale, exportZScale))
																{
																Success = 0;
																goto FileError;
																} // if error
															} // for Lc
														PtCt ++;
														} // for Lr
													break;
													} // WCS_DBEXPORT_DEMSAS_3DFACES
												case WCS_DBEXPORT_DEMSAS_POLYLINE:
												case WCS_DBEXPORT_DEMSAS_POLYGONMESH:
												case WCS_DBEXPORT_DEMSAS_POLYFACEMESH:
													{
													PrintDXFTextValue(fOutFile, 0, "POLYLINE");
													PrintDXFTextValue(fOutFile, 8, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
													//PrintDXFNameOrLayer(fOutFile, CurrentJoe);
													PrintDXFIntValue(fOutFile, 70, (JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYFACEMESH ? 64: (JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYGONMESH ? 16: 0)));
													if (JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYFACEMESH || JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYGONMESH)
														{
														PrintDXFIntValue(fOutFile, 71, Topo.LatEntries());
														PrintDXFIntValue(fOutFile, 72, Topo.LonEntries());
														} // if polygon or polyface mesh
													PrintDXFIntValue(fOutFile, 66, 1);	// points follow
													if (JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYGONMESH)
														{
														// <<<>>> large degree of uncertainty as to if this is correct
														for (PtCt = 0, Lr = 0; Lr < Topo.LonEntries(); Lr ++)
															{
															int Ratio;
															Ratio = (int)(((double)Lr / (double)Topo.LonEntries()) * 100.0);
															BWDX->Update(NumWrite + Ratio);
															if (BWDX->CheckAbort())
																{
																break; // bail out of DEM loop, full abort on next CheckAbort() call.
																}
															//PtLon = Topo.Westest() - Lr * Topo.LonStep();
															for (Lc = 0; Lc < Topo.LatEntries(); Lc ++, PtCt ++)
																{
																PtLon = Topo.Vertices[PtCt].Lon;
																PtLat = Topo.Vertices[PtCt].Lat;
																//PtLat = Topo.Southest() + Lc * Topo.LatStep();
																PrintDXFTextValue(fOutFile, 0, "VERTEX");
																PrintDXFNameOrLayer(fOutFile, CurrentJoe);
																PrintDXFIntValue(fOutFile, 70, 64);
																VTD.Lat = PtLat;
																VTD.Lon = PtLon;
																#ifdef WCS_VIEW_TERRAFFECTORS
																GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
																 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
																SampEl = (float)UnCalcExag(VTD.Elev, DefPO);
																#else // !WCS_VIEW_TERRAFFECTORS
																SampEl = (float)Topo.Vertices[PtCt].Elev;
																#endif // !WCS_VIEW_TERRAFFECTORS
																//if (! PrintDXFTriple(fOutFile, 0, PtLon, PtLat, SampEl))
																if (! PrintDXFTriple(fOutFile, 0, (FlipLon ? -PtLon : PtLon), PtLat, SampEl, OutCoords, exportXYScale, exportZScale))
																	{
																	Success = 0;
																	goto FileError;
																	} // if error
																} // for Lc
															} // for Lr
														} // if polygon or polyface mesh
													else if (JAH->ExportDEMsAs == WCS_DBEXPORT_DEMSAS_POLYFACEMESH)
														{
														for (PtCt = 0, Lr = 0; Lr < Topo.LonEntries(); Lr ++)
															{
															//PtLon = Topo.Westest() - Lr * Topo.LonStep();
															for (Lc = 0; Lc < Topo.LatEntries(); Lc ++, PtCt ++)
																{
																PtLon = Topo.Vertices[PtCt].Lon;
																PtLat = Topo.Vertices[PtCt].Lat;
																//PtLat = Topo.Southest() + Lc * Topo.LatStep();
																PrintDXFTextValue(fOutFile, 0, "VERTEX");
																PrintDXFNameOrLayer(fOutFile, CurrentJoe);
																PrintDXFIntValue(fOutFile, 70, 128 + 64);
																VTD.Lat = PtLat;
																VTD.Lon = PtLon;
																#ifdef WCS_VIEW_TERRAFFECTORS
																GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
																 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
																SampEl = (float)UnCalcExag(VTD.Elev, DefPO);
																#else // !WCS_VIEW_TERRAFFECTORS
																SampEl = (float)Topo.Vertices[PtCt].Elev;
																#endif // !WCS_VIEW_TERRAFFECTORS
																//if (! PrintDXFTriple(fOutFile, 0, PtLon, PtLat, SampEl))
																if (! PrintDXFTriple(fOutFile, 0, (FlipLon ? -PtLon : PtLon), PtLat, SampEl, OutCoords, exportXYScale, exportZScale))
																	{
																	Success = 0;
																	goto FileError;
																	} // if error
																} // for Lc
															} // for Lr
														for (PtCt = 1, Lr = 0; Lr < Topo.LonEntries() - 1; Lr ++)	// PtCt starts at 1 so that indices start at 1
															{
															int Ratio;
															Ratio = (int)(((double)Lr / (double)Topo.LonEntries()) * 100.0);
															BWDX->Update(NumWrite + Ratio);
															if (BWDX->CheckAbort())
																{
																break; // bail out of DEM loop, full abort on next CheckAbort() call.
																}
															for (Lc = 0; Lc < Topo.LatEntries() - 1; Lc ++, PtCt ++)
																{
																PrintDXFTextValue(fOutFile, 0, "VERTEX");
																PrintDXFNameOrLayer(fOutFile, CurrentJoe);
																PrintDXFIntValue(fOutFile, 70, 128);
																PrintDXFTriple(fOutFile, 0, 0.0, 0.0, 0.0f, NULL, exportXYScale, exportZScale);
																PrintDXFIntValue(fOutFile, 71, PtCt);
																PrintDXFIntValue(fOutFile, 72, PtCt + Topo.LatEntries());
																PrintDXFIntValue(fOutFile, 73, PtCt + 1);
																PrintDXFTextValue(fOutFile, 0, "VERTEX");
																PrintDXFNameOrLayer(fOutFile, CurrentJoe);
																PrintDXFIntValue(fOutFile, 70, 128);
																PrintDXFTriple(fOutFile, 0, 0.0, 0.0, 0.0f, NULL, exportXYScale, exportZScale);
																PrintDXFIntValue(fOutFile, 71, PtCt + 1 + Topo.LatEntries());
																PrintDXFIntValue(fOutFile, 72, PtCt + 1);
																if (!PrintDXFIntValue(fOutFile, 73, PtCt + Topo.LatEntries()))
																	{
																	Success = 0;
																	goto FileError;
																	} // if error
																} // for Lc
															PtCt ++;
															} // for Lr
														} // if polyface mesh
													else
														{
														for (PtCt = 0, Lr = 0; Lr < Topo.LonEntries(); Lr ++)
															{
															int Ratio;
															Ratio = (int)(((double)Lr / (double)Topo.LonEntries()) * 100.0);
															BWDX->Update(NumWrite + Ratio);
															if (BWDX->CheckAbort())
																{
																break; // bail out of DEM loop, full abort on next CheckAbort() call.
																}
															//PtLon = Topo.Westest() - Lr * Topo.LonStep();
															for (Lc = 0; Lc < Topo.LatEntries(); Lc ++, PtCt ++)
																{
																PtLon = Topo.Vertices[PtCt].Lon;
																PtLat = Topo.Vertices[PtCt].Lat;
																//PtLat = Topo.Southest() + Lc * Topo.LatStep();
																PrintDXFTextValue(fOutFile, 0, "VERTEX");
																PrintDXFNameOrLayer(fOutFile, CurrentJoe);
																#ifdef WCS_VIEW_TERRAFFECTORS
																VTD.Lat = PtLat;
																VTD.Lon = PtLon;
																GlobalApp->MainProj->Interactive->VertexDataPoint(&Rend, &VTD, &PGD,
																 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
																SampEl = (float)UnCalcExag(VTD.Elev, DefPO);
																#else // !WCS_VIEW_TERRAFFECTORS
																SampEl = (float)Topo.Vertices[PtCt].Elev;
																#endif // !WCS_VIEW_TERRAFFECTORS
																//if (! PrintDXFTriple(fOutFile, 0, PtLon, PtLat, SampEl))
																if (! PrintDXFTriple(fOutFile, 0, (FlipLon ? -PtLon : PtLon), PtLat, SampEl, OutCoords, exportXYScale, exportZScale))
																	{
																	Success = 0;
																	goto FileError;
																	} // if error
																} // for Lc
															} // for Lr
														} // if polygon or polyface mesh
													PrintDXFSeqEnd(fOutFile);
													break;
													} // WCS_DBEXPORT_DEMSAS_POLYLINE
												} // switch
											Topo.FreeRawElevs();
											NumWrite += 100; // Indicate DEM full written
											} // if DEM loaded
										} // if DEM
									else
										{
										if (CurrentJoe->GetFirstRealPoint() && CurrentJoe->GetNumRealPoints() > 0)
											{
											// Convert non-geographic coords into geographic.

											// identify if there is a coordsys attached to this object
											if (MyAttr = (JoeCoordSys *)CurrentJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
												MyCoords = MyAttr->Coord;
											else
												MyCoords = NULL;

											if (CurrentJoe->GetLineStyle() >= 4)
												{
												int gc70 = 8;	// group code 70
												PrintDXFTextValue(fOutFile, 0, "POLYLINE");
												// AutoCAD chokes if this is here
//												PrintDXFTextValue(fOutFile, 1, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
												PrintDXFNameOrLayer(fOutFile, CurrentJoe);
												PrintDXFIntValue(fOutFile, 66, 1);
												PrintDXFTriple(fOutFile, 0, 0.0, 0.0, 0.0f, NULL, 1.0, 1.0);	// we DON'T want to project our 0,0,0 triplet
												PrintDXFIntValue(fOutFile, 70, gc70);	// write 8 here, 32 the other times
												gc70 = 32;
												for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
													{
													PrintDXFTextValue(fOutFile, 0, "VERTEX");
													PrintDXFNameOrLayer(fOutFile, CurrentJoe);
													PLink->ProjToDefDeg(MyCoords, &Vert);
													if (FlipLon)
														PrintDXFTriple(fOutFile, 0, -Vert.Lon, Vert.Lat, (float)Vert.Elev, OutCoords, exportXYScale, exportZScale);
													else
														PrintDXFTriple(fOutFile, 0, Vert.Lon, Vert.Lat, (float)Vert.Elev, OutCoords, exportXYScale, exportZScale);
													PrintDXFIntValue(fOutFile, 70, gc70);
													} // for
												PrintDXFSeqEnd(fOutFile);
												} // if points exist
											else
												{
												for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
													{
													PrintDXFTextValue(fOutFile, 0, "POINT");
													PrintDXFTextValue(fOutFile, 1, (char *)(CurrentJoe->Name() ? CurrentJoe->Name(): (CurrentJoe->FileName() ? CurrentJoe->FileName(): "No Name")));
													PrintDXFNameOrLayer(fOutFile, CurrentJoe);
													PLink->ProjToDefDeg(MyCoords, &Vert);
													if (FlipLon)
														PrintDXFTriple(fOutFile, 0, -Vert.Lon, Vert.Lat, (float)Vert.Elev, OutCoords, exportXYScale, exportZScale);
													else
														PrintDXFTriple(fOutFile, 0, Vert.Lon, Vert.Lat, (float)Vert.Elev, OutCoords, exportXYScale, exportZScale);
													} // for
												} // else point style
											} // if points exist
										} // else
									} // if
								} // if not group
							} // for Current
						PrintDXFEndSec(fOutFile);
						if (! PrintDXFEndFile(fOutFile))
							{
							Success = 0;
							goto FileError;
							} // else
						} // if entities
					else
						{
						Success = 0;
						goto FileError;
						} // else
					} // if tables
				else
					{
					Success = 0;
					goto FileError;
					} // else
				} // if header
			else
				{
				Success = 0;
				goto FileError;
				} // else
			} // if objects found
		else
			{
			UserMessageOK("Export DXF", "No objects were found!\nOperation terminated.");
			Success = 0;
			} // else
		FileError:
		if (! Success)
			{
			UserMessageOK("Export DXF", "An error occurred while writing to file!\nNot all entities were saved correctly.");
			} // if
		fclose(fOutFile);
		} // if
	else
		{
		UserMessageOK("Export DXF", "Error opening file for output!\nOperation terminated.");
		Success = 0;
		} // else
	} // if
else
	Success = 0;

//time2 = GetSystemTimeFP();
//totaltime = time2 - time1;
//sprintf(timemsg, "Export Time = %lf seconds.", totaltime);
//GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, timemsg);

if (BWDX)
	delete BWDX;

BWDX = NULL;

return (Success);

} //Database::ExportDXF

/*===========================================================================*/

// dest must be at least 256 characters
void CreateValidDXFString(char *dest, char *src)
{
char c, *from = src, *to = dest;
short i = 0;	// output counter

do
	{
	c = *from++;
	if (c == 0) // end of string
		{
		*to = 0;
		return;
		}
	if (c < 32)	// a control character
		{
		if ((i + 2) < 255)	// if we won't overflow the destination
			{
			*to++ = '^';	// control character becomes caret + alpha letter
			*to++ = c + 0x40;
			i += 2;
			}
		else
			{
			*to = 0;		// that's all that we can fit there
			return;
			}
		} // if control char
	else if (c == ' ')		// DXF docs say nothing about why this is rejected
		{
		c = '_';
		*to++ = c;
		i++;
		}
	else if (c == '.')		// DXF docs say nothing about why this is rejected
		{
		c = '_';
		*to++ = c;
		i++;
		}
	else 
		{
		*to++ = c;
		i++;
		}
	} while (i < 255);
*to = 0;	// terminate the string

}

/*===========================================================================*/

// NOTE: AutoCAD layer names can apparently only be alphanumeric + any of "$_-"
// Layer names also need to be 31 chars or less!
void Database::PrintDXFNameOrLayer(FILE *fFile, Joe *Current)
{
LayerStub *Stub;

for (Stub = Current->FirstLayer(); Stub; Stub = Stub->NextLayerInObject())
	{
	if (Stub->MyLayer() && !((Stub->MyLayer())->TestFlags(WCS_LAYER_ISATTRIBUTE)))
		{
		PrintDXFTextValue(fFile, 8, (char *)Stub->MyLayer()->GetName());
		return;
		} // if
	} // for Stub
PrintDXFTextValue(fFile, 8, (char *)(Current->Name() ? Current->Name(): (Current->FileName() ? Current->FileName(): "NoName")));

} // Database::PrintDXFNameOrLayer

/*===========================================================================*/

int PrintDXFSection(FILE *fFile, char *Section)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "SECTION") < 0)
	return (0);
if (fprintf(fFile, "%3d\n", 2) < 0)
	return (0);
if (fprintf(fFile, "%s\n", Section) < 0)
	return (0);

return (1);

} // PrintDXFSection

/*===========================================================================*/

int PrintDXFTableType(FILE *fFile, char *TableType)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "TABLE") < 0)
	return (0);
if (fprintf(fFile, "%3d\n", 2) < 0)
	return (0);
if (fprintf(fFile, "%s\n", TableType) < 0)
	return (0);

return (1);

} // PrintDXFTableType

/*===========================================================================*/

int PrintDXFIntValue(FILE *fFile, int GroupCode, int Value)
{

if (fprintf(fFile, "%3d\n", GroupCode) < 0)
	return (0);
if (fprintf(fFile, "%d\n", Value) < 0)
	return (0);

return (1);

} // PrintDXFIntValue

/*===========================================================================*/

int PrintDXFFltValue(FILE *fFile, int GroupCode, double Value)
{

if (fprintf(fFile, "%3d\n", GroupCode) < 0)
	return (0);
if (fprintf(fFile, "%1.1f\n", Value) < 0)
	return (0);

return (1);

} // PrintDXFFltValue

/*===========================================================================*/

int PrintDXFTextValue(FILE *fFile, int GroupCode, char *Text)
{
char ValidStr[256];

CreateValidDXFString(ValidStr, Text);
if (fprintf(fFile, "%3d\n", GroupCode) < 0)
	return (0);
if (fprintf(fFile, "%s\n", ValidStr) < 0)
	return (0);

return (1);

} // PrintDXFTextValue

/*===========================================================================*/

int PrintDXFHdrTriple(FILE *fFile, char *Variable, int VtxNum, double *XYZ)
{
short Ct;

if (fprintf(fFile, "%3d\n", 9) < 0)
	return (0);
if (fprintf(fFile, "%s\n", Variable) < 0)
	return (0);
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (fprintf(fFile, "%3d\n", 10 + VtxNum + 10 * Ct) < 0)
		return (0);
	if (fprintf(fFile, "%1.12le\n", XYZ[Ct]) < 0)
		return (0);
	} // for

return (1);

} // PrintDXFSection

/*===========================================================================*/

int PrintDXFTriple(FILE *fFile, int VtxNum, double X, double Y, float Z, CoordSys *OutCoords, double XYScale, double ZScale)
{
VertexDEM Vert;

if (OutCoords)
	{
	Vert.Lat = Y;
	Vert.Lon = X;
	Vert.Elev = Z;
	OutCoords->DefDegToProj(&Vert);
	X = Vert.xyz[0] * XYScale;
	Y = Vert.xyz[1] * XYScale;
	Z = (float)(Vert.xyz[2] * ZScale);
	} // if

if (fprintf(fFile, "%3d\n", 10 + VtxNum) < 0)
	return (0);
if (fprintf(fFile, "%1.12le\n", X) < 0)
	return (0);
if (fprintf(fFile, "%3d\n", 20 + VtxNum) < 0)
	return (0);
if (fprintf(fFile, "%1.12le\n", Y) < 0)
	return (0);
if (fprintf(fFile, "%3d\n", 30 + VtxNum) < 0)
	return (0);
if (fprintf(fFile, "%f\n", Z) < 0)
	return (0);

return (1);

} // PrintDXFTriple

/*===========================================================================*/

int PrintDXFEndSec(FILE *fFile)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "ENDSEC") < 0)
	return (0);

return (1);

} // PrintDXFEndSec

/*===========================================================================*/

int PrintDXFEndTab(FILE *fFile)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "ENDTAB") < 0)
	return (0);

return (1);

} // PrintDXFEndTab

/*===========================================================================*/

int PrintDXFSeqEnd(FILE *fFile)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "SEQEND") < 0)
	return (0);

return (1);

} // PrintDXFSeqEnd

/*===========================================================================*/

int PrintDXFEndFile(FILE *fFile)
{

if (fprintf(fFile, "%3d\n", 0) < 0)
	return (0);
if (fprintf(fFile, "%s\n", "EOF") < 0)
	return (0);

return (1);

} // PrintDXFEndFile

/*===========================================================================*/
