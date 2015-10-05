// ImageFormatTIFFs.cpp
// TIFF & GeoTIFF routines
// Created 05/09/06 by FW2
// Copyright 3D Nature, LLC 2006

#include "stdafx.h"
#include "ImageFormatConfig.h"
#include "AppMem.h"
#include "Useful.h"
#include "Application.h"
#include "Project.h"
#include "Requester.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "ImageFormatTIFFs.h"
#include "ImageFormatIFF.h"
#include "Raster.h"
#include "RLA.h"
#include "WCSVersion.h"
#include "PostProcessEvent.h"
#include "CoordSys.h"

#ifdef WCS_BUILD_TIFF_SUPPORT

#ifdef WCS_BUILD_GEOTIFF_SUPPORT

class GeoTIFFAccess
	{
	public:
		geokey_t	GeoIndex[GTIF_GeogKeysMax],  ProjIndex[GTIF_ProjKeysMax],  VertIndex[GTIF_VertKeysMax];
		short		GeoValues[GTIF_GeogKeysMax], ProjValues[GTIF_ProjKeysMax], VertValues[GTIF_VertKeysMax];
		double		GeoValuesD[GTIF_GeogKeysMax], ProjValuesD[GTIF_ProjKeysMax], VertValuesD[GTIF_VertKeysMax];
		char		GeoType[GTIF_GeogKeysMax], ProjType[GTIF_ProjKeysMax], VertType[GTIF_VertKeysMax];
		char		GeoExists[GTIF_GeogKeysMax], ProjExists[GTIF_ProjKeysMax], VertExists[GTIF_VertKeysMax];
		int		GeoParamMap[GTIF_GeogKeysMax], ProjParamMap[GTIF_ProjKeysMax], VertParamMap[GTIF_VertKeysMax];
		char GTCitationGeoKeyStr[200], GeogCitationGeoKeyStr[200], PCSCitationGeoKeyStr[200], VerticalCitationGeoKeyStr[200],
		 FullCitationForName[1024];

		GeoTIFFAccess();
		int GetGeoKeys(GTIF *GTIHandle);
		int GetProjKeys(GTIF *GTIHandle);
		int GetVertKeys(GTIF *GTIHandle);
		int GetCiteKeys(GTIF *GTIHandle);
		char *MakeNameFromCites(int MaxLen);
		int FindGeoKey(geokey_t KeyID);
		int FindProjKey(geokey_t KeyID);

	}; // GeoTIFFAccess

/*===========================================================================*/

GeoTIFFAccess::GeoTIFFAccess()
{
int Init;

GTCitationGeoKeyStr[0] = GeogCitationGeoKeyStr[0] = PCSCitationGeoKeyStr[0] = VerticalCitationGeoKeyStr[0] = NULL;

for (Init = 0; Init < GTIF_GeogKeysMax; Init++)
	{
	GeoType[Init] = 0;
	GeoExists[Init] = 0;
	GeoValues[Init] = 0;
	GeoValuesD[Init] = 0.0;
	GeoParamMap[Init] = -1;
	} // for

for (Init = 0; Init < GTIF_ProjKeysMax; Init++)
	{
	ProjType[Init] = 0;
	ProjExists[Init] = 0;
	ProjValues[Init] = 0;
	ProjValuesD[Init] = 0.0;
	ProjParamMap[Init] = -1;
	} // for

for (Init = 0; Init < GTIF_VertKeysMax; Init++)
	{
	VertType[Init] = 0;
	VertExists[Init] = 0;
	VertValues[Init] = 0;
	VertValuesD[Init] = 0.0;
	VertParamMap[Init] = -1;
	} // for

GeoIndex[GTIF_GeographicTypeGeoKey] = GeographicTypeGeoKey;
//GeoIndex[GTIF_GeogCitationGeoKey] = GeogCitationGeoKey;
GeoIndex[GTIF_GeogGeodeticDatumGeoKey] = GeogGeodeticDatumGeoKey;
GeoIndex[GTIF_GeogPrimeMeridianGeoKey] = GeogPrimeMeridianGeoKey;
GeoIndex[GTIF_GeogLinearUnitsGeoKey] = GeogLinearUnitsGeoKey;
GeoIndex[GTIF_GeogLinearUnitSizeGeoKey] = GeogLinearUnitSizeGeoKey; GeoType[GTIF_GeogLinearUnitSizeGeoKey] = 1;
GeoIndex[GTIF_GeogAngularUnitsGeoKey] = GeogAngularUnitsGeoKey;
GeoIndex[GTIF_GeogAngularUnitSizeGeoKey] = GeogAngularUnitSizeGeoKey; GeoType[GTIF_GeogAngularUnitSizeGeoKey] = 1;
GeoIndex[GTIF_GeogEllipsoidGeoKey] = GeogEllipsoidGeoKey;
GeoIndex[GTIF_GeogSemiMajorAxisGeoKey] = GeogSemiMajorAxisGeoKey; GeoType[GTIF_GeogSemiMajorAxisGeoKey] = 1;
GeoIndex[GTIF_GeogSemiMinorAxisGeoKey] = GeogSemiMinorAxisGeoKey; GeoType[GTIF_GeogSemiMinorAxisGeoKey] = 1;
GeoIndex[GTIF_GeogInvFlatteningGeoKey] = GeogInvFlatteningGeoKey; GeoType[GTIF_GeogInvFlatteningGeoKey] = 1;
GeoIndex[GTIF_GeogAzimuthUnitsGeoKey] = GeogAzimuthUnitsGeoKey;
GeoIndex[GTIF_GeogPrimeMeridianLongGeoKey] = GeogPrimeMeridianLongGeoKey; GeoType[GTIF_GeogPrimeMeridianLongGeoKey] = 1;

ProjIndex[GTIF_ProjectedCSTypeGeoKey] = ProjectedCSTypeGeoKey;
//ProjIndex[GTIF_PCSCitationGeoKey] = PCSCitationGeoKey;
ProjIndex[GTIF_ProjectionGeoKey] = ProjectionGeoKey;
ProjIndex[GTIF_ProjCoordTransGeoKey] = ProjCoordTransGeoKey;
ProjIndex[GTIF_ProjLinearUnitsGeoKey] = ProjLinearUnitsGeoKey;
ProjIndex[GTIF_ProjLinearUnitSizeGeoKey] = ProjLinearUnitSizeGeoKey; ProjType[GTIF_ProjLinearUnitSizeGeoKey] = 1;
ProjIndex[GTIF_ProjStdParallel1GeoKey] = ProjStdParallel1GeoKey; ProjType[GTIF_ProjStdParallel1GeoKey] = 1;
ProjIndex[GTIF_ProjStdParallel2GeoKey] = ProjStdParallel2GeoKey; ProjType[GTIF_ProjStdParallel2GeoKey] = 1;
ProjIndex[GTIF_ProjNatOriginLongGeoKey] = ProjNatOriginLongGeoKey; ProjType[GTIF_ProjNatOriginLongGeoKey] = 1;
ProjIndex[GTIF_ProjNatOriginLatGeoKey] = ProjNatOriginLatGeoKey; ProjType[GTIF_ProjNatOriginLatGeoKey] = 1;
ProjIndex[GTIF_ProjFalseEastingGeoKey] = ProjFalseEastingGeoKey; ProjType[GTIF_ProjFalseEastingGeoKey] = 1;
ProjIndex[GTIF_ProjFalseNorthingGeoKey] = ProjFalseNorthingGeoKey; ProjType[GTIF_ProjFalseNorthingGeoKey] = 1;
ProjIndex[GTIF_ProjFalseOriginLongGeoKey] = ProjFalseOriginLongGeoKey; ProjType[GTIF_ProjFalseOriginLongGeoKey] = 1;
ProjIndex[GTIF_ProjFalseOriginLatGeoKey] = ProjFalseOriginLatGeoKey; ProjType[GTIF_ProjFalseOriginLatGeoKey] = 1;
ProjIndex[GTIF_ProjFalseOriginEastingGeoKey] = ProjFalseOriginEastingGeoKey; ProjType[GTIF_ProjFalseOriginEastingGeoKey] = 1;
ProjIndex[GTIF_ProjFalseOriginNorthingGeoKey] = ProjFalseOriginNorthingGeoKey; ProjType[GTIF_ProjFalseOriginNorthingGeoKey] = 1;
ProjIndex[GTIF_ProjCenterLongGeoKey] = ProjCenterLongGeoKey; ProjType[GTIF_ProjCenterLongGeoKey] = 1;
ProjIndex[GTIF_ProjCenterLatGeoKey] = ProjCenterLatGeoKey; ProjType[GTIF_ProjCenterLatGeoKey] = 1;
ProjIndex[GTIF_ProjCenterEastingGeoKey] = ProjCenterEastingGeoKey; ProjType[GTIF_ProjCenterEastingGeoKey] = 1;
ProjIndex[GTIF_ProjCenterNorthingGeoKey] = ProjCenterNorthingGeoKey; ProjType[GTIF_ProjCenterNorthingGeoKey] = 1;
ProjIndex[GTIF_ProjScaleAtNatOriginGeoKey] = ProjScaleAtNatOriginGeoKey; ProjType[GTIF_ProjScaleAtNatOriginGeoKey] = 1;
ProjIndex[GTIF_ProjScaleAtCenterGeoKey] = ProjScaleAtCenterGeoKey; ProjType[GTIF_ProjScaleAtCenterGeoKey] = 1;
ProjIndex[GTIF_ProjAzimuthAngleGeoKey] = ProjAzimuthAngleGeoKey; ProjType[GTIF_ProjAzimuthAngleGeoKey] = 1;
ProjIndex[GTIF_ProjStraightVertPoleLongGeoKey] = ProjStraightVertPoleLongGeoKey; ProjType[GTIF_ProjStraightVertPoleLongGeoKey] = 1;
ProjIndex[GTIF_ProjRectifiedGridAngleGeoKey] = ProjRectifiedGridAngleGeoKey; ProjType[GTIF_ProjRectifiedGridAngleGeoKey] = 1;

VertIndex[GTIF_VerticalCSTypeGeoKey] = VerticalCSTypeGeoKey;
//VertIndex[GTIF_VerticalCitationGeoKey] = VerticalCitationGeoKey;
VertIndex[GTIF_VerticalDatumGeoKey] = VerticalDatumGeoKey;
VertIndex[GTIF_VerticalUnitsGeoKey] = VerticalUnitsGeoKey;

// set up parameter mapping tables
// meaningless for GeoKeys (at the moment anyway)
GeoParamMap[GTIF_GeographicTypeGeoKey] = -1;
//GeoParamMap[GTIF_GeogCitationGeoKey] = -1;
GeoParamMap[GTIF_GeogGeodeticDatumGeoKey] = -1;
GeoParamMap[GTIF_GeogPrimeMeridianGeoKey] = -1;
GeoParamMap[GTIF_GeogLinearUnitsGeoKey] = -1;
GeoParamMap[GTIF_GeogLinearUnitSizeGeoKey] = -1;
GeoParamMap[GTIF_GeogAngularUnitsGeoKey] = -1;
GeoParamMap[GTIF_GeogAngularUnitSizeGeoKey] = -1;
GeoParamMap[GTIF_GeogEllipsoidGeoKey] = -1;
GeoParamMap[GTIF_GeogSemiMajorAxisGeoKey] = -1;
GeoParamMap[GTIF_GeogSemiMinorAxisGeoKey] = -1;
GeoParamMap[GTIF_GeogInvFlatteningGeoKey] = -1;
GeoParamMap[GTIF_GeogAzimuthUnitsGeoKey] = -1;
GeoParamMap[GTIF_GeogPrimeMeridianLongGeoKey] = -1;

ProjParamMap[GTIF_ProjectedCSTypeGeoKey] = -1;
//ProjParamMap[GTIF_PCSCitationGeoKey] = -1;
ProjParamMap[GTIF_ProjectionGeoKey] = -1;
ProjParamMap[GTIF_ProjCoordTransGeoKey] = -1;
ProjParamMap[GTIF_ProjLinearUnitsGeoKey] = -1;
ProjParamMap[GTIF_ProjLinearUnitSizeGeoKey] = -1;
ProjParamMap[GTIF_ProjStdParallel1GeoKey] = GeoLib_Lat1ststdparallel;
ProjParamMap[GTIF_ProjStdParallel2GeoKey] = GeoLib_Lat2ndstdparallel;
ProjParamMap[GTIF_ProjNatOriginLongGeoKey] = GeoLib_Loncentralmeridian;
ProjParamMap[GTIF_ProjNatOriginLatGeoKey] = GeoLib_Latprojectionorigin;
ProjParamMap[GTIF_ProjFalseEastingGeoKey] = GeoLib_Falseeasting;
ProjParamMap[GTIF_ProjFalseNorthingGeoKey] = GeoLib_Falsenorthing;
ProjParamMap[GTIF_ProjFalseOriginLongGeoKey] = -1;
ProjParamMap[GTIF_ProjFalseOriginLatGeoKey] = GeoLib_Latprojectionorigin;
ProjParamMap[GTIF_ProjFalseOriginEastingGeoKey] = -1;
ProjParamMap[GTIF_ProjFalseOriginNorthingGeoKey] = -1;
ProjParamMap[GTIF_ProjCenterLongGeoKey] = GeoLib_Loncenterprojection;
ProjParamMap[GTIF_ProjCenterLatGeoKey] = GeoLib_Latcenterprojection;
ProjParamMap[GTIF_ProjCenterEastingGeoKey] = -1;
ProjParamMap[GTIF_ProjCenterNorthingGeoKey] = -1;
ProjParamMap[GTIF_ProjScaleAtNatOriginGeoKey] = GeoLib_Scalefactor;
ProjParamMap[GTIF_ProjScaleAtCenterGeoKey] = 10;
ProjParamMap[GTIF_ProjAzimuthAngleGeoKey] = GeoLib_Azimuthangle;
ProjParamMap[GTIF_ProjStraightVertPoleLongGeoKey] = -1;
ProjParamMap[GTIF_ProjRectifiedGridAngleGeoKey] = -1;

// meaningless for VertKeys (at the moment anyway)
VertParamMap[GTIF_VerticalCSTypeGeoKey] = -1;
//VertParamMap[GTIF_VerticalCitationGeoKey] = -1;
VertParamMap[GTIF_VerticalDatumGeoKey] = -1;
VertParamMap[GTIF_VerticalUnitsGeoKey] = -1;

} // GeoTIFFAccess::GeoTIFFAccess

/*===========================================================================*/

int GeoTIFFAccess::GetCiteKeys(GTIF *GTIHandle)
{
int NumKeys = 0, KeySize;
tagtype_t KeyType;

GTCitationGeoKeyStr[0] = PCSCitationGeoKeyStr[0] = GeogCitationGeoKeyStr[0] = VerticalCitationGeoKeyStr[0] = 0;

if (GTIFKeyInfo(GTIHandle, GTCitationGeoKey, &KeySize, &KeyType))
	{
	GTIFKeyGet(GTIHandle, GTCitationGeoKey, GTCitationGeoKeyStr, 0, 199);
	// sample GeoTIFF files sometimes have erroneous trailing quotes, remove them
	if (GTCitationGeoKeyStr[0])
		{
		if (GTCitationGeoKeyStr[strlen(GTCitationGeoKeyStr) - 1] == '\"')
			{
			GTCitationGeoKeyStr[strlen(GTCitationGeoKeyStr) - 1] = NULL;
			} // if
		} // if
	NumKeys++;
	} // if

if (GTIFKeyInfo(GTIHandle, GeogCitationGeoKey, &KeySize, &KeyType))
	{
	GTIFKeyGet(GTIHandle, GeogCitationGeoKey, GeogCitationGeoKeyStr, 0, 199);
	// sample GeoTIFF files sometimes have erroneous trailing quotes, remove them
	if (GeogCitationGeoKeyStr[0])
		{
		if (GeogCitationGeoKeyStr[strlen(GeogCitationGeoKeyStr) - 1] == '\"')
			{
			GeogCitationGeoKeyStr[strlen(GeogCitationGeoKeyStr) - 1] = NULL;
			} // if
		} // if
	NumKeys++;
	} // if

if (GTIFKeyInfo(GTIHandle, VerticalCitationGeoKey, &KeySize, &KeyType))
	{
	GTIFKeyGet(GTIHandle, VerticalCitationGeoKey, VerticalCitationGeoKeyStr, 0, 199);
	// sample GeoTIFF files sometimes have erroneous trailing quotes, remove them
	if (VerticalCitationGeoKeyStr[0])
		{
		if (VerticalCitationGeoKeyStr[strlen(VerticalCitationGeoKeyStr) - 1] == '\"')
			{
			VerticalCitationGeoKeyStr[strlen(VerticalCitationGeoKeyStr) - 1] = NULL;
			} // if
		} // if
	NumKeys++;
	} // if

if (GTIFKeyInfo(GTIHandle, PCSCitationGeoKey, &KeySize, &KeyType))
	{
	GTIFKeyGet(GTIHandle, PCSCitationGeoKey, PCSCitationGeoKeyStr, 0, 199);
	// sample GeoTIFF files sometimes have erroneous trailing quotes, remove them
	if (PCSCitationGeoKeyStr[0])
		{
		if (PCSCitationGeoKeyStr[strlen(PCSCitationGeoKeyStr) - 1] == '\"')
			{
			PCSCitationGeoKeyStr[strlen(PCSCitationGeoKeyStr) - 1] = NULL;
			} // if
		} // if
	NumKeys++;
	} // if

return(NumKeys);

} // GeoTIFFAccess::GetCiteKeys

/*===========================================================================*/

char *GeoTIFFAccess::MakeNameFromCites(int MaxLen)
{
FullCitationForName[0] = NULL;
int ALen, BLen, CLen, DLen, Cited = 0, Remain = 0;

MaxLen -= 1; // we account for NULL in measurements here so we don't later

ALen = (int)strlen(GTCitationGeoKeyStr);
BLen = (int)strlen(GeogCitationGeoKeyStr);
CLen = (int)strlen(PCSCitationGeoKeyStr);
DLen = (int)strlen(VerticalCitationGeoKeyStr);

//strncat(FullCitationForName, "GeoTIFF: ", MaxLen);
FullCitationForName[MaxLen - 1] = NULL;

Remain = MaxLen - (int)strlen(FullCitationForName);
if (ALen && Remain > 0)
	{
	strncat(FullCitationForName, GTCitationGeoKeyStr, Remain);
	FullCitationForName[MaxLen] = NULL;
	Remain = MaxLen - (int)strlen(FullCitationForName);
	if (Remain > 0 && (BLen || CLen || DLen)) strcat(FullCitationForName, " ");
	Remain = MaxLen - (int)strlen(FullCitationForName);
	} // if

if (BLen && Remain > 0)
	{
	strncat(FullCitationForName, GeogCitationGeoKeyStr, Remain);
	FullCitationForName[MaxLen] = NULL;
	Remain = MaxLen - (int)strlen(FullCitationForName);
	if (Remain > 0 && (CLen || DLen)) strcat(FullCitationForName, " ");
	Remain = MaxLen - (int)strlen(FullCitationForName);
	} // if

if (CLen && Remain > 0)
	{
	strncat(FullCitationForName, PCSCitationGeoKeyStr, Remain);
	FullCitationForName[MaxLen] = NULL;
	Remain = MaxLen - (int)strlen(FullCitationForName);
	if (Remain > 0 && DLen) strcat(FullCitationForName, " ");
	Remain = MaxLen - (int)strlen(FullCitationForName);
	} // if

if (DLen && Remain > 0)
	{
	strncat(FullCitationForName, PCSCitationGeoKeyStr, Remain);
	FullCitationForName[MaxLen] = NULL;
	Remain = MaxLen - (int)strlen(FullCitationForName);
	} // if

return(FullCitationForName);
} // GeoTIFFAccess::MakeNameFromCites

/*===========================================================================*/

int GeoTIFFAccess::GetGeoKeys(GTIF *GTIHandle)
{
int NumKeys = 0, KeyLoop;

for (KeyLoop = 0; KeyLoop < GTIF_GeogKeysMax; KeyLoop++)
	{
	if (GeoType[KeyLoop])
		{
		NumKeys += (GeoExists[KeyLoop] = GTIFKeyGet(GTIHandle, GeoIndex[KeyLoop], &GeoValuesD[KeyLoop], 0, 1));
		} // if
	else
		{
		NumKeys += (GeoExists[KeyLoop] = GTIFKeyGet(GTIHandle, GeoIndex[KeyLoop], &GeoValues[KeyLoop], 0, 1));
		} // else
	} // for

return(NumKeys);
} // GeoTIFFAccess::GetGeoKeys

/*===========================================================================*/

int GeoTIFFAccess::GetProjKeys(GTIF *GTIHandle)
{
int NumKeys = 0, KeyLoop;

for (KeyLoop = 0; KeyLoop < GTIF_ProjKeysMax; KeyLoop++)
	{
	if (ProjType[KeyLoop])
		{
		NumKeys += (ProjExists[KeyLoop] = GTIFKeyGet(GTIHandle, ProjIndex[KeyLoop], &ProjValuesD[KeyLoop], 0, 1));
		} // if
	else
		{
		NumKeys += (ProjExists[KeyLoop] = GTIFKeyGet(GTIHandle, ProjIndex[KeyLoop], &ProjValues[KeyLoop], 0, 1));
		} // else
	} // for

return(NumKeys);
} // GeoTIFFAccess::GetProjKeys

/*===========================================================================*/

int GeoTIFFAccess::GetVertKeys(GTIF *GTIHandle)
{
int NumKeys = 0, KeyLoop;

for (KeyLoop = 0; KeyLoop < GTIF_VertKeysMax; KeyLoop++)
	{
	if (VertType[KeyLoop])
		{
		NumKeys += (VertExists[KeyLoop] = GTIFKeyGet(GTIHandle, VertIndex[KeyLoop], &VertValuesD[KeyLoop], 0, 1));
		} // if
	else
		{
		NumKeys += (VertExists[KeyLoop] = GTIFKeyGet(GTIHandle, VertIndex[KeyLoop], &VertValues[KeyLoop], 0, 1));
		} // else
	} // for

return(NumKeys);
} // GeoTIFFAccess::GetVertKeys

/*===========================================================================*/

int GeoTIFFAccess::FindGeoKey(geokey_t KeyID)
{
int NumKey = -1, KeyLoop;

for (KeyLoop = 0; KeyLoop < GTIF_GeogKeysMax; KeyLoop++)
	{
	if (GeoExists[KeyLoop] && GeoIndex[KeyLoop] == KeyID)
		{
		NumKey = KeyLoop;
		break;
		} // if
	} // for

return(NumKey);
} // GeoTIFFAccess::FindGeoKey

/*===========================================================================*/

int GeoTIFFAccess::FindProjKey(geokey_t KeyID)
{
int NumKey = -1, KeyLoop;

for (KeyLoop = 0; KeyLoop < GTIF_ProjKeysMax; KeyLoop++)
	{
	if (ProjExists[KeyLoop] && ProjIndex[KeyLoop] == KeyID)
		{
		NumKey = KeyLoop;
		break;
		} // if
	} // for

return(NumKey);
} // GeoTIFFAccess::FindProjKey

#endif // WCS_BUILD_GEOTIFF_SUPPORT

#endif // WCS_BUILD_TIFF_SUPPORT

/*===========================================================================*/

#ifdef WCS_BUILD_TIFF_SUPPORT

static char emsg[1024], TIFFEBuf[2048];

// translate CSV filenames into full pathnames
static char TIFFCSVFileNameBuffer[1000];
const char *TIFFCSVFileNameHook(const char *CSVName)
{
char temppath[512];
strmfp(temppath, GlobalApp->GetProgDir(), "Tools");
strmfp(TIFFCSVFileNameBuffer, temppath, CSVName);

return(TIFFCSVFileNameBuffer);
} // TIFFCSVFileNameHook

// First attempt at simple TIFF loader. I think we can be more efficient than this.

#ifdef WCS_BUILD_GEOTIFF_SUPPORT

int IsValidGeoTIFF(GTIF *geotiff)
{
int gtVersions[3], gtKeys, valid = 0;

GTIFDirectoryInfo(geotiff, &gtVersions[0], &gtKeys);
if (gtKeys)
	{
	valid = 1;
	} // if

return valid;

} // ValidGeoTIFF

/*===========================================================================*/

GeoRefShell* ReadTIFFGeoref(ArcWorldInfo &awi, GeoRefShell *LoaderGeoRefShell, GTIF *GTI, const long cols, const long rows, const int prjGood, const int TFWValid, const int GTIValid, bool askUnits)
{
double UnitMult = 1.0;
CoordSys *TempCS;
int FlipLon = 0;

if (! LoaderGeoRefShell)
	LoaderGeoRefShell = new GeoRefShell;
if (LoaderGeoRefShell)
	{
	double TempNorth = 1.0, TempSouth = 0.0, TempEast = 1.0, TempWest = 0.0;
	short TempShort;

	LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_EDGES; // GeoTIFF apparently stores coords this way for image-type data
	if (TFWValid && !GTIValid)
		{ // we prefer GeoTIFF to TFW, but we'll take TFW if no GeoTIFF
		int IsUTM = 1;

		TempEast = (awi.originX + ((cols - 1) * awi.cellSizeX));
		TempSouth = (awi.originY + ((rows - 1)* awi.cellSizeY));

		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(TempNorth = awi.originY);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(TempWest = awi.originX);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(TempEast);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(TempSouth);
		
		if (! prjGood)
			{
			// check to see if all bounds are within geographic limits -- unlikely for a UTM/stateplane
			if (TempNorth <= 90.0 && TempSouth >= -90.0 && fabs(TempEast) <= 360.0 && fabs(TempWest) <= 360.0)
				{
				// probably a WCS geographic
				IsUTM = 0;
				} // if
			// Set up a coordsys if needed
			if (!LoaderGeoRefShell->Host)
				LoaderGeoRefShell->Host = new CoordSys();
			if (TempCS = (CoordSys *)LoaderGeoRefShell->Host)
				{
				if (IsUTM)
					{ // take the default CoordSys
					PlanetOpt *DefPlanetOpt;
					if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
						{
						if (DefPlanetOpt->Coords)
							TempCS->Copy(TempCS, DefPlanetOpt->Coords);
						}
					} // if
				else
					{
					// if geographic, we ought to negate the sign of the east and west bounds
					// World file stores GIS PosEast notation
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
					LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
					} // else
				} // if
			} // if
		else
			{
			TempCS = (CoordSys *)LoaderGeoRefShell->Host;
			if (TempCS && TempCS->GetGeographic()) // are we GIS-style geographic notation
				{
				// if geographic, we ought to negate the sign of the east and west bounds
				// World file stores GIS PosEast notation
				LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
				LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
				} // if
			} // else
		if (askUnits)
			{
			double tVal;
			unsigned char result;

			result = UserMessageCustom("World File Units", "\n\nPlease set the units for this world file.",
				"Meters", "Feet", "US Survey Feet", 1);
			switch (result)
				{
				default:
				case 1:
					//UnitMult = 1.0;  --  already set to this by default
					break;
				case 0:
					UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
					break;
				case 2:
					UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
					break;
				} // switch
			tVal = LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(tVal * UnitMult);
			tVal = LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(tVal * UnitMult);
			tVal = LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(tVal * UnitMult);
			tVal = LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(tVal * UnitMult);
			} // if
		} // if
	else if (GTIValid)
		{
		if (GTIFKeyGet(GTI, GTRasterTypeGeoKey, &TempShort, 0, 1))
			{
			// Although this seems backwards, this is correct
			// xref: http://support.esri.com/index.cfm?fa=knowledgebase.techarticles.articleShow&d=19491
			if (TempShort == 1) // RasterPixelIsArea
				{
				LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_EDGES;
				} // if
			else if (TempShort == 2) // RasterPixelIsPoint
				{
				LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS;
				} // if
			} // if
		if (! LoaderGeoRefShell->Host)
			LoaderGeoRefShell->Host = new CoordSys();
		if (TempCS = (CoordSys *)LoaderGeoRefShell->Host)
			{
			double TempX, TempY;
			short ProjType;

			if (GTIFKeyGet(GTI, GTModelTypeGeoKey, &ProjType, 0, 1))
				{
				GeoTIFFAccess GTA;
				GTIFDefn GTD;

				TempCS->SetSystem(0L); // identify System as custom so as not to mislead
				if (GTA.GetCiteKeys(GTI))
					{
					TempCS->SetUniqueName(GlobalApp->AppEffects, GTA.MakeNameFromCites(WCS_EFFECT_MAXNAMELENGTH));
					} // if
				else
					{
					TempCS->SetUniqueName(GlobalApp->AppEffects, "Unnamed GeoTIFF CoordSys");
					} // else
				// get full Proj/Geo keysets to try to aquire units of length
				if (GTA.GetGeoKeys(GTI))
					{
					int KeySlot;
					if ((KeySlot = GTA.FindGeoKey(GeogLinearUnitsGeoKey)) != -1)
						{
						switch (GTA.GeoValues[KeySlot])
							{
							case Linear_Foot:
								{
								UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
								break;
								} // 
							case Linear_Foot_US_Survey:
								{
								UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
								break;
								} //
							// anything else is either meters, or we don't support it so it's meters now. ;)
							} // switch
						} // if
					} // if
				if (GTA.GetProjKeys(GTI))
					{
					int KeySlot;
					if ((KeySlot = GTA.FindProjKey(ProjLinearUnitsGeoKey)) != -1)
						{
						switch (GTA.ProjValues[KeySlot])
							{
							case Linear_Foot:
								{
								UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
								break;
								} // 
							case Linear_Foot_US_Survey:
								{
								UnitMult = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
								break;
								} //
							// anything else is either meters, or we don't support it so it's meters now. ;)
							} // switch
						} // if
					} // if
				if (ProjType == 1) // ModelTypeProjected
					{
					if (GTA.GetProjKeys(GTI))
						{
						int ProjParam;
						GTIFGetDefn(GTI, &GTD);

						// fetch and stash units multiplier
						if (GTD.UOMLengthInMeters != 1.0)
							{ // use this one if it says anything
							UnitMult = GTD.UOMLengthInMeters; // Previously thought this needed to be 1/ inversed, but it appears not: One UOMLength = UOMLengthInMeters meters.
							} // if


						// some of this will be redundent, as we will configure information
						// from finest granularity to coarsest, so that if the later coarse
						// idents fail, at least some info will be passed, and if the later
						// coarse idents suceed, they will supercede the earlier custom setup.

						// set ellipsoid numerically first, in case later "by code" fails
						if (GTD.SemiMajor > 0) TempCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(GTD.SemiMajor);
						if (GTD.SemiMinor > 0) TempCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(GTD.SemiMinor);
						// set ellipsoid first, in case Datum by code fails
						TempCS->Datum.Ellipse.SetEllipsoidByEPSGCode(GTD.Ellipsoid);
						TempCS->Datum.SetDatumByEPSGCode(GTD.Datum);
						// try to set projection method
						TempCS->Method.SetMethodByGeoTIFFCode(GTD.CTProjection);

						// Set projection method params here in case full-system ID below fails or we're custom
						for (ProjParam = 0; ProjParam < GTIF_ProjKeysMax; ProjParam++)
							{
							if (GTA.ProjExists[ProjParam] && GTA.ProjParamMap[ProjParam] != -1)
								{
								int SlotNum;
								SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GTA.ProjParamMap[ProjParam]);
								if (SlotNum == -1) // failed to find slot, try alternate slot for certain cases
									{
									if (ProjParam == GTIF_ProjNatOriginLongGeoKey)
										{ // try GCTPC "Lon Center Projection"
										SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GeoLib_Loncenterprojection);
										} // if
									else if (ProjParam == GTIF_ProjNatOriginLatGeoKey)
										{ // try GCTPC "Lat Center Projection"
										SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GeoLib_Latcenterprojection);
										if (SlotNum == -1)
											{ // try GCTPC "Lat of True Scale"
											SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GeoLib_Lattruescale);
											} // if
										} // else if
									else if (ProjParam == GTIF_ProjCenterLatGeoKey)
										{ // try GCTPC "Lat Projection Origin"
										SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GeoLib_Latprojectionorigin);
										} // else if
									else if (ProjParam == GTIF_ProjCenterLongGeoKey)
										{
										// GeoLib_Loncentralmeridian
										SlotNum = TempCS->Method.FindParamAnimParSlotByGCTPID(GeoLib_Loncentralmeridian);
										} // 
									} // if
								if (SlotNum != -1) // did we find a matching slot?
									{
									if (GTA.ProjType[ProjParam] == 1)
										{
										// most of the params below we don't know how to parse anyway, but I'm being complete...
										if ((ProjParam == GTIF_ProjNatOriginLongGeoKey) || (ProjParam == GTIF_ProjFalseOriginLongGeoKey) ||
										 (ProjParam == GTIF_ProjCenterLongGeoKey) || (ProjParam == GTIF_ProjStraightVertPoleLongGeoKey))
											{
											// it's a longitude, flip it
											TempCS->Method.AnimPar[SlotNum].SetValue(-GTA.ProjValuesD[ProjParam]);
											} // if
										else
											{
											TempCS->Method.AnimPar[SlotNum].SetValue(GTA.ProjValuesD[ProjParam]);
											} // else
										} // if
									else
										{
										TempCS->Method.AnimPar[SlotNum].SetValue((double)GTA.ProjValues[ProjParam]);
										} // else
									} // if
								} // if
							} // for

						// can we got more specific with a complete identified System?
						if ((GTD.ProjCode >= Proj_UTM_zone_1N && GTD.ProjCode <= Proj_UTM_zone_60S) // UTM of some sort
						 || (GTD.ProjCode >= 16200 && GTD.ProjCode <= 16332)) // Gauss-Kruger of some sort
							{
							if (GTD.ProjCode >= Proj_UTM_zone_1N && GTD.ProjCode <= Proj_UTM_zone_60S) // UTM of some sort
								{
								switch (GTD.Datum)
									{
									case 6267: TempCS->SetSystemByCode(2); break; // UTM NAD27
									case 6269: TempCS->SetSystemByCode(3); break; // UTM NAD83
									case 6230: TempCS->SetSystemByCode(9); break; // UTM ED50
									case 6326: TempCS->SetSystemByCode(10); break; // UTM WGS84
									} // Datum
								} // if UTM
							if (GTD.PCS >= 28402 && GTD.PCS <= 28432) // Gauss-Kruger of some sort with odd false easting
								{
								int ZoneCheck;
								char ZoneShorthand[12];

								// EPSG only seems to define north Zones from 2 thru 32
								TempCS->SetSystemByCode(19); // G-K
								ZoneCheck = GTD.ProjCode - 28400; // 28401 = Zone 1
								sprintf(ZoneShorthand, "%dN", ZoneCheck);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // if G-K
							if (GTD.ProjCode >= 16201 && GTD.ProjCode <= 16232) // Gauss-Kruger of some sort with odd false easting
								{
								int ZoneCheck;
								char ZoneShorthand[12];

								// EPSG only seems to define north Zones from 2 thru 32
								TempCS->SetSystemByCode(19); // G-K
								ZoneCheck = GTD.ProjCode - 16200; // 16201 = Zone 1
								sprintf(ZoneShorthand, "%dN", ZoneCheck);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // if G-K
							else if (GTD.ProjCode >= 16301 && GTD.ProjCode <= 16332) // Gauss-Kruger of some sort
								{
								int ZoneCheck;
								char ZoneShorthand[12];

								// EPSG only seems to define north Zones from 2 thru 32
								TempCS->SetSystemByCode(6); // G-K
								ZoneCheck = GTD.ProjCode - 16300; // 16301 = Zone 1
								sprintf(ZoneShorthand, "%dN", ZoneCheck);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // if G-K
							if (GTD.PCS >= 28462 && GTD.PCS <= 28492) // Gauss-Kruger of some sort
								{
								int ZoneCheck;
								char ZoneShorthand[12];

								// EPSG only seems to define north Zones from 2 thru 32
								TempCS->SetSystemByCode(6); // G-K
								ZoneCheck = GTD.ProjCode - 28460; // 28461 = Zone 1
								sprintf(ZoneShorthand, "%dN", ZoneCheck);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // if G-K
							// Set UTM Zone
							if (GTD.MapSys == MapSys_UTM_North)
								{
								char ZoneShorthand[12];
								sprintf(ZoneShorthand, "%dN", GTD.Zone);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // if
							else if (GTD.MapSys == MapSys_UTM_South)
								{
								char ZoneShorthand[12];
								sprintf(ZoneShorthand, "%dS", GTD.Zone);
								TempCS->SetZoneByShort(ZoneShorthand);
								} // else
							} // UTM
						else if (GTD.ProjCode >= Proj_Alabama_CS27_East && GTD.ProjCode <= Proj_Puerto_Rico_Virgin_Is) // US State Plane of some sort
							{
							switch (GTD.Datum)
								{
								case 6267: TempCS->SetSystemByCode(4); break; // US State Plane NAD27
								case 6269: TempCS->SetSystemByCode(5); break; // US State Plane NAD83
								} // Datum
							// Set State/Zone
							if ((GTD.MapSys == MapSys_State_Plane_27) || (GTD.MapSys == MapSys_State_Plane_83))
								{
								// we've already set the datum up, so we only need to
								// determine which state zone to use, and need to ignore whether it is NAD27 or NAD83

								// special issues:
								// Montana
								// NAD27 N:12501 C:12502 S:12503
								// NAD83 ALL:12530
								// Nebraska
								// NAD27 N:12601 S:12602
								// NAD83 ALL:12630
								// South Carolina
								// NAD27 N:13901 S:13902
								// NAD83 ALL:13930
								// Purto Rico / Virgin Islands
								// NAD27 PR:15201
								// NAD83 PR/VI:15230

								switch (GTD.ProjCode)
									{
									case 12501: // Montana NAD27 N
									case 12502: // Montana NAD27 C
									case 12503: // Montana NAD27 S
									case 12601: // Nebraska NAD27 N
									case 12602: // Nebraska NAD27 S
									case 13901: // South Carolina NAD27 N
									case 13902: // South Carolina NAD27 S
									case 15201: // Puerto Rico, NAD27
										{
										// make sure we're SP-NAD27
										// This will use the StatePlaneZones.dbf table
										TempCS->SetSystemByCode(4);
										TempCS->SetStatePlaneZoneByEPSG27(GTD.ProjCode);
										break;
										} // 
									case 12530: // Montana NAD83 All
									case 12630: // Nebraska NAD83 All
									case 13930: // South Carolina NAD83 All
									case 15230: // Puerto Rico/ Virgin Islands NAD83
										{
										// make sure we're SP-NAD83
										// This will use the StatePlaneZones83.dbf table
										TempCS->SetSystemByCode(5);
										TempCS->SetStatePlaneZoneByEPSG27(GTD.ProjCode);
										break;
										} // 
									default:
										{
										// if EPSG Zone code hundreds digit > 3, it's a NAD83
										// code and we should subtract 30 to get the NAD27
										// code that our Zone table is indexed by.
										//NAD27SPZone = GTD.ProjCode;
										// Zonecheck tricks not necessary now that I've realized
										// we have a different SP Zone table for NAD83...
										//ZoneCheck = GTD.ProjCode % 100;
										//if (ZoneCheck >= 30)
										//	{
										//	NAD27SPZone -= 30;
										//	} // if
										TempCS->SetStatePlaneZoneByEPSG27(GTD.ProjCode);
										break;
										} // all the normal ones
									} // switch
								} // if
							} // State Plane
						else if (GTD.ProjCode == 16061) // Polar Stereographic N
							{
							TempCS->SetSystemByCode(7);
							TempCS->SetZone("North");
							} // Polar Stereographic N
						else if (GTD.ProjCode == 16161) // Polar Stereographic S
							{
							TempCS->SetSystemByCode(7);
							TempCS->SetZone("South");
							} // Polar Stereographic S
						} // if
					} // if
				else if (ProjType == 3) // ModelTypeGeocentric
					{
					// What is this? ECEF? Anyone use this?
					} // else if
				else // ModelTypeGeographic (2) or assume geographic
					{
					// new CoordSys defaults to geographic
					if (GTA.GetGeoKeys(GTI))
						{
						FlipLon = 1;
						GTIFGetDefn(GTI, &GTD);

						// set ellipsoid numerically first, in case later "by code" fails
						TempCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(GTD.SemiMajor);
						TempCS->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(GTD.SemiMajor);
						// set ellipsoid first, in case Datum by code fails
						TempCS->Datum.Ellipse.SetEllipsoidByEPSGCode(GTD.Ellipsoid);
						TempCS->Datum.SetDatumByEPSGCode(GTD.Datum);

						// can we got more specific with a complete identified System?
						switch (GTD.Datum)
							{
							case 6267: TempCS->SetSystemByCode(13); break; // Geo NAD27
							case 6269: TempCS->SetSystemByCode(11); break; // Geo NAD83
							case 6326: TempCS->SetSystemByCode(12); break; // Geo WGS84
							} // Datum
						} // if
					} // else
				} // if

			TempX = TempY = 0.0;
			if (GTIFImageToPCS(GTI, &TempX, &TempY))
				{
				TempNorth = TempY;
				TempWest = TempX;
				} // if
			TempX = cols;
			TempY = rows;
			if (GTIFImageToPCS(GTI, &TempX, &TempY))
				{
				TempSouth = TempY;
				TempEast = TempX;
				} // if
			if (FlipLon)
				{
				TempWest *= -1.0;
				TempEast *= -1.0;
				} // if
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(TempNorth * UnitMult);
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(TempWest * UnitMult);
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(TempEast * UnitMult);
			LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(TempSouth * UnitMult);
			} // if
		} // else
	} // if

return LoaderGeoRefShell;

} // ReadTIFFGeoref

/*===========================================================================*/

int ReadWorldFile(ArcWorldInfo &awi, const char *filename)
{
FILE *worldFileHandle = NULL;
int wfLen, valid = 0;
char worldFileName[1024];

strcpy(worldFileName, filename);
wfLen = (int)strlen(worldFileName);
if ((wfLen > 4) && (worldFileName[wfLen - 4] == '.'))
	{
	// modify TLA extension to World File notation automagically
	worldFileName[wfLen - 2] = worldFileName[wfLen - 1];
	worldFileName[wfLen - 1] = 'w';
	if (worldFileHandle = PROJ_fopen(worldFileName, "r"))
		{
		fscanf(worldFileHandle, "%lf", &awi.cellSizeX); // real Easting cellsize
		fscanf(worldFileHandle, "%lf", &awi.rowRot);	// row rotation (unused)
		fscanf(worldFileHandle, "%lf", &awi.colRot);	// column rotation (unused)
		fscanf(worldFileHandle, "%lf", &awi.cellSizeY); // real Northing cellsize (negated) 
		fscanf(worldFileHandle, "%lf", &awi.originX);	// real UL Easting
		if (fscanf(worldFileHandle, "%lf", &awi.originY) == 1)	// real UL Northing
			valid = 1;
		fclose(worldFileHandle);
		worldFileHandle = NULL;
		} // if
	} // if
/***
   ArcView docs say world files are supposed to be w appended to extension (ie: file.tiff -> file.tiffw), with the
   above strategy only used on 8.3 systems
else if
	{
	} // else if
***/

return valid;

} // ReadWorldFile

/*===========================================================================*/

#endif // WCS_BUILD_GEOTIFF_SUPPORT

/*===========================================================================*/

short Raster::LoadTIFF(char *Name, short SupressWng, ImageCacheControl *ICC)
{
CoordSys *TempCS = NULL;
ArcWorldInfo awi;
unsigned char *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf, *ABuf;
unsigned long int WorkRow, InScan, PixelCol;
TIFF *tif;
TIFFRGBAImage img;
size_t npixels;
uint32* raster = NULL;
unsigned char *Alpha = NULL;
int PrjGood = 0;
uint32 TileWidth, TileHeight, StripRowsPerStrip;
tstrip_t NumStripsInFile;
int LoadingSubTile = 0;
long int LoadOffsetX = 0, LoadOffsetY = 0;
#ifdef WCS_IMAGE_MANAGEMENT
char PerfectTileMatch = 0;
#endif // WCS_IMAGE_MANAGEMENT
unsigned char Success = 0;
unsigned char DoGrey = 0, DoAlpha = 0, AlphaFailed = 0;
char IsTiled = 0;

if (ICC)
	{
	#ifdef WCS_IMAGE_MANAGEMENT
	ICC->QueryOnlyFlags = NULL; // clear all flags
	ICC->QueryOnlyFlags = WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE | WCS_BITMAPS_IMAGECAPABILITY_HASNATIVETILES; // even stripped reports as short wide tiles
	LoadingSubTile = ICC->LoadingSubTile;
	#else // !WCS_IMAGE_MANAGEMENT
	ICC->QueryOnlyFlags = NULL; // clear all flags
	#endif // !WCS_IMAGE_MANAGEMENT
	} // if

#ifdef WCS_BUILD_GEOTIFF_SUPPORT
char WorldFileName[1024];
FILE *WorldFileHandle = NULL;
GTIF *GTI;
int GTIValid = 0, WFValid = 0, WFLen;

if (!LoadingSubTile)
	{
	SetCSVFilenameHook(TIFFCSVFileNameHook);

	WFValid = ReadWorldFile(awi, Name);

	// Now try for PRJ file
	strcpy(WorldFileName, Name);
	WFLen = (int)strlen(WorldFileName);
	if ((WFLen > 4) && (WorldFileName[WFLen - 4] == '.'))
		{
		// modify TLA extension to .PRJ File
		WorldFileName[WFLen - 3] = 'p';
		WorldFileName[WFLen - 2] = 'r';
		WorldFileName[WFLen - 1] = 'j';
		if (WorldFileHandle = PROJ_fopen(WorldFileName, "r"))
			{
			if (!LoaderGeoRefShell) LoaderGeoRefShell = new GeoRefShell;
			if (LoaderGeoRefShell)
				{
				if (! LoaderGeoRefShell->Host)
					LoaderGeoRefShell->Host = new CoordSys();
				if (TempCS = (CoordSys *)LoaderGeoRefShell->Host)
					{
					CSLoadInfo *CSLI = NULL;

					if (CSLI = TempCS->LoadFromArcPrj(WorldFileHandle))
						{
						PrjGood = 1;
						delete CSLI;
						CSLI = NULL;
						} // if
					} // if
				} // if
			fclose(WorldFileHandle);
			WorldFileHandle = NULL;
			} // if
		} // if
	} // if

if (tif = XTIFFOpen(GlobalApp->MainProj->MungPath(Name), "rC"))
	{
	GTI = GTIFNew(tif);
	GTIValid = IsValidGeoTIFF(GTI);

#else // !WCS_BUILD_GEOTIFF_SUPPORT
if (tif = TIFFOpen(GlobalApp->MainProj->MungPath(Name), "rC"))
	{
#endif // !WCS_BUILD_GEOTIFF_SUPPORT
	if (TIFFRGBAImageBegin(&img, tif, 0, emsg))
		{
		if (img.samplesperpixel == 4)
			{
			DoAlpha = 1;
			} // if
		if (img.samplesperpixel == 1)
			{
			if (!img.redcmap)
				{
				DoGrey = 1;
				} // if
			} // if
		NumStripsInFile = TIFFNumberOfTiles(tif);
		if (IsTiled = (char)TIFFIsTiled(tif))
			{
			TIFFGetField(tif, TIFFTAG_TILEWIDTH, &TileWidth);
			TIFFGetField(tif, TIFFTAG_TILELENGTH, &TileHeight);
			npixels = TileWidth * TileHeight;
			if (!(ICC && ICC->LoadingSubTile))
				{ // don't mess with these while we're subtiling
				NativeTileWidth  = TileWidth;
				NativeTileHeight = TileHeight;
				} // if
#ifdef WCS_IMAGE_MANAGEMENT
			else
				{
				if ((TileWidth == ICC->LoadWidth) && (TileHeight == ICC->LoadHeight))
					{
					PerfectTileMatch = 1;
					} // if
				} // if
#endif // WCS_IMAGE_MANAGEMENT
			} // if
		else
			{
			if (TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &StripRowsPerStrip))
				{
				npixels = img.width * StripRowsPerStrip;
				if (!(ICC && ICC->LoadingSubTile))
					{ // don't mess with these while we're subtiling
					NativeTileWidth  = img.width;
					NativeTileHeight = StripRowsPerStrip;
					} // if
				} // if
			else
				{
				StripRowsPerStrip = img.height;
				npixels = img.width * StripRowsPerStrip;
				if (!(ICC && ICC->LoadingSubTile))
					{ // don't mess with these while we're subtiling
					NativeTileWidth  = img.width;
					NativeTileHeight = StripRowsPerStrip;
					} // if
				} // else
			} // else
	#ifdef WCS_IMAGE_MANAGEMENT
		if (ICC)
			{
			if (ICC->LoadingSubTile)
				{ // loading a subtile, don't fiddle with settings
				Rows = ICC->LoadHeight;
				Cols = ICC->LoadWidth;
				LoadOffsetX = ICC->LoadXOri;
				LoadOffsetY = ICC->LoadYOri;
				} // if
			else
				{ // loading a whole file, pre-set settings
				unsigned long int AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_THRESH; // use higher AutoEnable threshold by default
				if(WFValid || GTIValid) AutoEnableThreshold = WCS_RASTER_TILECACHE_AUTOENABLE_GEO_THRESH; // use lower threshold if we know it's georeferenced
				if ((img.width >= AutoEnableThreshold) || (img.height >= AutoEnableThreshold))
					{
					// autoswitch on
					ICC->ImageManagementEnable = 1;
					Cols = img.width; // need to set these here if we skip loading real image below
					Rows = img.height;
					} // if
				} // else
			} // if
	#endif // WCS_IMAGE_MANAGEMENT
		raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
		if (raster != NULL)
			{
			#ifdef WCS_IMAGE_MANAGEMENT
			if (!(ICC && ICC->ImageManagementEnable)) // skip loading the real image if we've autoswitched IM on
			#else // WCS_IMAGE_MANAGEMENT
			if (1)
			#endif // !WCS_IMAGE_MANAGEMENT
				{
				// process loaded raster data...

			#ifdef WCS_IMAGE_MANAGEMENT
				if (!(ICC && ICC->LoadingSubTile))
			#endif // WCS_IMAGE_MANAGEMENT
					{
					// already set above, if under IM
					Cols = img.width;
					Rows = img.height;
					} // if
				ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TIFF Loader Bitmaps");
				if (!DoGrey)
					{
					ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TIFF Loader Bitmaps");
					ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TIFF Loader Bitmaps");
					} // if
				if (DoAlpha && (Alpha == NULL))
					{
					if (!(Alpha = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "TIFF Loader Alpha Bitmaps")))
						{
						AlphaFailed = 1;
						} // if
					} // if

				if (ByteMap[0] && (DoGrey || (ByteMap[1] && ByteMap[2])) && !AlphaFailed) // everything ok?
					{
					// Clear bitmaps
					memset(ByteMap[0], 0, Cols * Rows);
					if (ByteMap[1]) memset(ByteMap[1], 0, Cols * Rows);
					if (ByteMap[2]) memset(ByteMap[2], 0, Cols * Rows);
					if (DoAlpha && Alpha)
						{
						memset(Alpha, 0, Cols * Rows);
						} // if

					if (IsTiled)
						{
						int CurTile, CurTileULX, CurTileULY, DecodeInRow, DecodeOutRow;

						for (CurTile = CurTileULX = CurTileULY = 0; CurTile < (signed)NumStripsInFile; CurTile++)
							{
							#ifdef WCS_IMAGE_MANAGEMENT
							// do we even need this tile?
							if (ICC && ICC->LoadingSubTile)
								{
								if (PerfectTileMatch)
									{ // cut directly to the chase
									CurTileULX = ICC->LoadXOri;
									CurTileULY = ICC->LoadYOri;
									} // if
								else
									{
									if (!(((unsigned)(CurTileULY + TileHeight) <= (unsigned)LoadOffsetY) || (CurTileULY > LoadOffsetY + Rows) ||
									 (CurTileULX > LoadOffsetX + Cols) || ((unsigned)(CurTileULX + TileWidth) <= (unsigned)LoadOffsetX)))
										{
										// not sure if we need to do anything here
										} // if
									else
										{ // don't need it at all, skip it
										CurTileULX += TileWidth;
										if (CurTileULX >= (signed)img.width)
											{
											CurTileULY += TileHeight;
											CurTileULX = 0;
											} // if
										continue;
										} // else
									} // else
								} // if
							#endif // WCS_IMAGE_MANAGEMENT

							// load in next tile
							TIFFReadRGBATile(tif, CurTileULX, CurTileULY, raster);


							for (DecodeOutRow = CurTileULY + TileHeight - 1, DecodeInRow = 0;
							 DecodeOutRow >= CurTileULY; DecodeInRow++, DecodeOutRow--)
								{
								// decode it
							#ifdef WCS_IMAGE_MANAGEMENT
								if (ICC && ICC->LoadingSubTile)
									{
									// does this line somehow coincide with our output tile?
									if (((DecodeOutRow >= LoadOffsetY) && (DecodeOutRow < LoadOffsetY + Rows)) &&
									 !((CurTileULX > LoadOffsetX + Cols) || ((unsigned)(CurTileULX + TileWidth) < (unsigned)LoadOffsetX)))
										{ // calculate proper offsets
										RBuf = &ByteMap[0][(DecodeOutRow - LoadOffsetY) * Cols];
										if (ByteMap[1]) GBuf = &ByteMap[1][(DecodeOutRow - LoadOffsetY) * Cols];
										if (ByteMap[2]) BBuf = &ByteMap[2][(DecodeOutRow - LoadOffsetY) * Cols];
										if (DoAlpha) ABuf = &(Alpha)[(DecodeOutRow - LoadOffsetY) * Cols];
										} // if
									else
										{ // don't need this input line at all, skip it
										continue;
										} // else
									} // if
								else
							#endif // WCS_IMAGE_MANAGEMENT
									{ // normal loading
									if (DecodeOutRow < (signed)img.height)
										{
										RBuf = &ByteMap[0][DecodeOutRow * Cols];
										if (ByteMap[1]) GBuf = &ByteMap[1][DecodeOutRow * Cols];
										if (ByteMap[2]) BBuf = &ByteMap[2][DecodeOutRow * Cols];
										if (DoAlpha) ABuf = &(Alpha)[DecodeOutRow * Cols];
										} // if
									else
										{
										// row from this tile is off end of image
										continue;
										} // else
									} // else
								// raster is a 32-bit pointer, so we don't multiply by 4 in the subscript
								//InterleaveBuf = (unsigned char *)&raster[((img.height - 1) - WorkRow) * img.width];
								InterleaveBuf = (unsigned char *)&raster[DecodeInRow * TileWidth];
								for (InScan = 0, PixelCol = CurTileULX; PixelCol < CurTileULX + TileWidth; PixelCol++)
									{
									#ifdef WCS_IMAGE_MANAGEMENT
									int RightThresh;
									RightThresh = min((signed)img.width, LoadOffsetX + Cols);
									if (PixelCol < (unsigned)RightThresh && DecodeOutRow < (int)img.height && DecodeOutRow >= LoadOffsetY && DecodeOutRow < LoadOffsetY + Rows)
									#else // !WCS_IMAGE_MANAGEMENT
									if (PixelCol < img.width && DecodeOutRow < (int)img.height)
									#endif // !WCS_IMAGE_MANAGEMENT
										{
/*
										if ((&RBuf[PixelCol - LoadOffsetX] >= &ByteMap[0][Rows * Cols]) || (&RBuf[PixelCol - LoadOffsetX] < &ByteMap[0][0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&GBuf[PixelCol - LoadOffsetX] >= &ByteMap[1][Rows * Cols]) || (&GBuf[PixelCol - LoadOffsetX] < &ByteMap[1][0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&BBuf[PixelCol - LoadOffsetX] >= &ByteMap[2][Rows * Cols]) || (&BBuf[PixelCol - LoadOffsetX] < &ByteMap[2][0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&InterleaveBuf[InScan] >= (unsigned char *)&raster[npixels * sizeof (uint32)]) || (&InterleaveBuf[InScan] < (unsigned char *)&raster[0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&InterleaveBuf[InScan + 1] >= (unsigned char *)&raster[npixels * sizeof (uint32)]) || (&InterleaveBuf[InScan + 1] < (unsigned char *)&raster[0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&InterleaveBuf[InScan + 2] >= (unsigned char *)&raster[npixels * sizeof (uint32)]) || (&InterleaveBuf[InScan + 2] < (unsigned char *)&raster[0]))
											{
											PixelCol = 0; // fault!
											} // if
										if ((&InterleaveBuf[InScan + 3] >= (unsigned char *)&raster[npixels * sizeof (uint32)]) || (&InterleaveBuf[InScan + 3] < (unsigned char *)&raster[0]))
											{
											PixelCol = 0; // fault!
											} // if
*/
										#ifdef BYTEORDER_LITTLEENDIAN
										RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										if (!DoGrey)
											{
 											GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											} // if
										else
											{
											InScan++; InScan++;
											} // else
										if (DoAlpha) ABuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										else InScan++;
										#endif // BYTEORDER_LITTENDIAN
										#ifdef BYTEORDER_BIGENDIAN
										if (DoAlpha) ABuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										else InScan++;
										if (!DoGrey)
											{
											BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											} // if
										else
											{
											InScan++; InScan++;
											} // else
										RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										#endif // BYTEORDER_BIGENDIAN
										} // if
									else
										{ // tile pixel outside of actual image
										InScan += 4; // skip RGB and A
										} // else
									} // for
								} // for
							CurTileULX += TileWidth;
							if (CurTileULX >= (signed)img.width)
								{
								CurTileULY += TileHeight;
								CurTileULX = 0;
								} // if

							#ifdef WCS_IMAGE_MANAGEMENT
							if (PerfectTileMatch)
								{ // end it now, prematurely, as we have the one and only one tile we need
								CurTile = (signed)NumStripsInFile;
								} // if
							#endif // WCS_IMAGE_MANAGEMENT
							} // for
						} // if
					else
						{ // strips
						int CurStrip = 0, DecodeInRow, DecodeOutRow;
						for (WorkRow = 0; WorkRow < img.height; WorkRow += StripRowsPerStrip)
							{
							int Status;

							#ifdef WCS_IMAGE_MANAGEMENT
							// do we even need this strip?

							if (ICC && ICC->LoadingSubTile)
								{
								if ((WorkRow + StripRowsPerStrip <= (unsigned)(LoadOffsetY)) || (WorkRow >= (unsigned)(LoadOffsetY + Rows)))
									{ // don't need it at all, skip it
									CurStrip++;
									continue;
									} // if
								} // if

							#endif // WCS_IMAGE_MANAGEMENT

							// load in a strip
							Status = TIFFReadRGBAStrip(tif, CurStrip * StripRowsPerStrip, raster);
							CurStrip++;

							// and decode it
							for (DecodeOutRow = min(img.height - 1, WorkRow + (StripRowsPerStrip - 1)), DecodeInRow = 0;
							 DecodeOutRow >= (signed)WorkRow; DecodeInRow++, DecodeOutRow--)
								{
							#ifdef WCS_IMAGE_MANAGEMENT
								if (ICC && ICC->LoadingSubTile)
									{
									if ((DecodeOutRow >= LoadOffsetY) && (DecodeOutRow < LoadOffsetY + Rows))
										{
										RBuf = &ByteMap[0][(DecodeOutRow - LoadOffsetY) * Cols];
										if (ByteMap[1]) GBuf = &ByteMap[1][(DecodeOutRow - LoadOffsetY) * Cols];
										if (ByteMap[2]) BBuf = &ByteMap[2][(DecodeOutRow - LoadOffsetY) * Cols];
										if (DoAlpha) ABuf = &(Alpha)[(DecodeOutRow - LoadOffsetY) * Cols];
										} // if
									else
										{ // line out of range of tile being loaded
										continue;
										} // else
									} // if
								else
							#endif // WCS_IMAGE_MANAGEMENT
									{ // normal loading, no tricky offsets
									RBuf = &ByteMap[0][DecodeOutRow * Cols];
									if (ByteMap[1]) GBuf = &ByteMap[1][DecodeOutRow * Cols];
									if (ByteMap[2]) BBuf = &ByteMap[2][DecodeOutRow * Cols];
									if (DoAlpha) ABuf = &(Alpha)[DecodeOutRow * Cols];
									} // else
								// raster is a 32-bit pointer, so we don't multiply by 4 in the subscript
								//InterleaveBuf = (unsigned char *)&raster[((img.height - 1) - WorkRow) * img.width];
								InterleaveBuf = (unsigned char *)&raster[DecodeInRow * img.width];
								for (InScan = PixelCol = 0; PixelCol < img.width; PixelCol++)
									{
									#ifdef WCS_IMAGE_MANAGEMENT
									if ((PixelCol < (unsigned)(LoadOffsetX + Cols)) && (PixelCol >= (unsigned)LoadOffsetX))
									#else // !WCS_IMAGE_MANAGEMENT
									if (1) // always within range in absence of Image Management
									#endif // !WCS_IMAGE_MANAGEMENT
										{
										// debug trap
/*
										if (&RBuf[PixelCol - LoadOffsetX] > &ByteMap[0][Rows * Cols])
											{
											PixelCol = 0; // fault!
											} // if
*/
										#ifdef BYTEORDER_LITTLEENDIAN
										RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										if (!DoGrey)
											{
											GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											} // if
										else
											{
											InScan++; InScan++;
											} // else
										if (DoAlpha) ABuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										else InScan++;
										#endif // BYTEORDER_LITTLEENDIAN
										#ifdef BYTEORDER_BIGENDIAN
										if (DoAlpha) ABuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										else InScan++;
										if (!DoGrey)
											{
											BBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											GBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
											} // if
										else
											{
											InScan++; InScan++;
											} // else
										RBuf[PixelCol - LoadOffsetX] = InterleaveBuf[InScan++];
										#endif // BYTEORDER_BIGENDIAN
										} // if (should we write this pixel)
									else
										{ // pixel outside of required area
										InScan += 4; // skip RGB and A
										} // else

									} // for
								} // for
							} // for
						} // else
					Success = 1;
					} // if

				} // if
			else
				{ // we deferred loading the whole image, just for a thumbnail
				Success = 1;
				} // else
			_TIFFfree(raster);
			} // if
		TIFFRGBAImageEnd(&img);
		} // if
	else
		{
		//TIFFError(GlobalApp->MainProj->MungPath(Name), emsg);
		sprintf(TIFFEBuf, GlobalApp->MainProj->MungPath(Name), emsg);
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, TIFFEBuf);
		} // else

#ifdef WCS_BUILD_GEOTIFF_SUPPORT
	// prepare georeferencing (and projection/datum if avail) info
	// to pass back to loader
	if (Success && (WFValid || GTIValid))
		LoaderGeoRefShell = ReadTIFFGeoref(awi, LoaderGeoRefShell, GTI, Cols, Rows, PrjGood, WFValid, GTIValid, true);

	if (GTI)
		GTIFFree(GTI);
	GTI = NULL;
	XTIFFClose(tif);
#else // !WCS_BUILD_GEOTIFF_SUPPORT
	TIFFClose(tif);
#endif // !WCS_BUILD_GEOTIFF_SUPPORT
	tif = NULL;
	} // if

if (!Success)
	{
	if (ByteMap[0]) AppMem_Free(ByteMap[0], Cols * Rows); ByteMap[0] = NULL;
	if (ByteMap[1]) AppMem_Free(ByteMap[1], Cols * Rows); ByteMap[1] = NULL;
	if (ByteMap[2]) AppMem_Free(ByteMap[2], Cols * Rows); ByteMap[2] = NULL;
	if (Alpha) AppMem_Free(Alpha, Cols * Rows); Alpha = NULL;
	} // if

if (Success)
	{
	//*NewPlanes = 24;
	if (DoGrey)
		{
		ByteBands = 1;
		} // if
	else
		{
		ByteBands = 3;
		} // else
	ByteBandSize = Rows * Cols;
	if (DoAlpha && Alpha)
		{
		AlphaAvailable = 1;
		if (AlphaEnabled)
			{
			CopyAlphaToCoverage(Alpha);
			} // if
		AppMem_Free(Alpha, Cols * Rows);
		Alpha = NULL;
		} // if
	else
		{
		AlphaAvailable = 0;
		AlphaEnabled = 0;
		} // else
	} // if

return(Success);

} // Raster::LoadTIFF()


// uncomment these and link with the memtrack version
// of Libtiff (which has these functions commented out)
// to make libtiff use our memory subsystem
// for leak tracking

/*
tdata_t
_TIFFmalloc(tsize_t s)
{
	return ((tdata_t)AppMem_Alloc(s, 0, NULL));
}

void
_TIFFfree(tdata_t p)
{
	AppMem_Free(p, 0);
	return;
}

tdata_t
_TIFFrealloc(tdata_t p, tsize_t s)
{
	void* pvTmp;
	//if ((pvTmp = GlobalReAlloc(p, s, 0)) == NULL) {
		{
		if ((pvTmp = AppMem_Alloc(s, 0, NULL)) != NULL) {
			CopyMemory(pvTmp, p, AppMem_QueryRealSize(p));
			AppMem_Free(p, 0);
		}
	}
	return ((tdata_t)pvTmp);
}
*/

#endif // WCS_BUILD_TIFF_SUPPORT
