// CoordSys.cpp
// For managing Coordinate Systems
// Built from scratch on 12/28/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "CoordSysEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Toolbar.h"
#include "DragnDropListGUI.h"
#include "TableListGUI.h"
#include "CoordSys.h"
#include "Lists.h"

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

ProjectionSystemTable CoordSys::ProjSysTable;
EllipsoidTable GeoEllipsoid::EllipseTable;
DatumTable GeoDatum::DatmTable;
ProjectionMethodTable ProjectionMethod::MethodTable;
ProjectionParameterTable ProjectionMethod::ParamTable;

// 0 = non-numeric, 1 = double, 2 = int
char CSLoadNumericMap[] =
	{
	0, // WCS_COORDSYS_SLOT_UNIDENTIFIED,
	0, // WCS_COORDSYS_SLOT_PROJECTION,
	0, // WCS_COORDSYS_SLOT_DATUM,
	0, // WCS_COORDSYS_SLOT_UNITS,
	0, // WCS_COORDSYS_SLOT_ZUNITS,
	0, // WCS_COORDSYS_SLOT_SPHEROID,
	1, // WCS_COORDSYS_SLOT_XSHIFT,
	1, // WCS_COORDSYS_SLOT_YSHIFT,
	2, // WCS_COORDSYS_SLOT_ZONE,
	2, // WCS_COORDSYS_SLOT_FIPSZONE,
	2, // WCS_COORDSYS_SLOT_QUADRANT,
	2, // WCS_COORDSYS_SLOT_FLIP,
	0, // WCS_COORDSYS_SLOT_PARAMETERS, (dummy, really)
	0, // WCS_COORDSYS_SLOT_MAX
	}; // CSLoadNumericMap

// in case of subset match, put shorter version first
// IE, the word ZUNITS contains the word UNITS, so UNITS must come first
// This tabel must match the order of the other enums and map(s), but otherwise
// can be reordered as necessary

char *CSLoadNameMap[] =
	{ // all upper case
	"", // WCS_COORDSYS_SLOT_UNIDENTIFIED,
	"PROJECTION", // WCS_COORDSYS_SLOT_PROJECTION,
	"DATUM", // WCS_COORDSYS_SLOT_DATUM,
	"UNITS UNIT", // WCS_COORDSYS_SLOT_UNITS,
	"ZUNITS", // WCS_COORDSYS_SLOT_ZUNITS,
	"SPHEROID", // WCS_COORDSYS_SLOT_SPHEROID,
	"XSHIFT", // WCS_COORDSYS_SLOT_XSHIFT,
	"YSHIFT", // WCS_COORDSYS_SLOT_YSHIFT,
	"ZONE", // WCS_COORDSYS_SLOT_ZONE,
	"FIPSZONE", // WCS_COORDSYS_SLOT_FIPSZONE,
	"QUADRANT", // WCS_COORDSYS_SLOT_QUADRANT,
	"FLIP", // WCS_COORDSYS_SLOT_FLIP,
	"PARAMETERS", // WCS_COORDSYS_SLOT_PARAMETERS, (dummy, really)
	"", // WCS_COORDSYS_SLOT_MAX
	}; // CSLoadNameMap

/***
__m128d sincos2(double val)
{
__m128d sc = _mm_set_sd(val);

#if defined _WIN32 && !defined __GNUC__ && !defined MAC

__asm
	{
	fld		val					// load val
	fsincos						// atomic sine/cosine computation
	mov		edx, cos_val		// load cos_val pointer
	fstp	qword ptr [edx]		// store cosine via pointer
	mov		edx, sin_val		// load sin_val pointer
	fstp	qword ptr [edx]		// store sine via pointer
	}

#else // if defined _WIN32 && !defined MAC

*sin_val = sin(val);
*cos_val = cos(val);

#endif // if defined _WIN32 && !defined MAC

} // sincos2
***/

/*===========================================================================*/

ProjectionMethod::ProjectionMethod(void)
: RasterAnimHost(NULL)
{

SetDefaults();

} // ProjectionMethod::ProjectionMethod

/*===========================================================================*/

ProjectionMethod::ProjectionMethod(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // ProjectionMethod::ProjectionMethod

/*===========================================================================*/

void ProjectionMethod::SetDefaults(void)
{
double EffectDefault = 0.0;
double RangeDefaults[3] = {FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults);
	ParamID[Ct] = 0;
	ParamName[Ct][0] = 0;
	} // for

SetMethod(0L);

} // ProjectionMethod::SetDefaults

/*===========================================================================*/

void ProjectionMethod::Copy(ProjectionMethod *CopyTo, ProjectionMethod *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	CopyTo->ParamID[Ct] = CopyFrom->ParamID[Ct];
	strcpy(CopyTo->ParamName[Ct], CopyFrom->ParamName[Ct]);
	} // for
strcpy(CopyTo->MethodName, CopyFrom->MethodName);
CopyTo->MethodID = CopyFrom->MethodID;
CopyTo->GCTPMethod = CopyFrom->GCTPMethod;

} // ProjectionMethod::Copy

/*===========================================================================*/

int ProjectionMethod::Equals(ProjectionMethod *MatchMe)
{
long Ct;

if (strcmp(MethodName, MatchMe->MethodName))
	return (0);
if (MethodID != MatchMe->MethodID)
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (ParamID[Ct] != MatchMe->ParamID[Ct])
		return (0);
	if (ParamID[Ct] >= 0)
		{
		if (strcmp(ParamName[Ct], MatchMe->ParamName[Ct]))
			return (0);
		if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
			return (0);
		} // if
	} // for

return (1);

} // ProjectionMethod::Equals

/*===========================================================================*/

// for use by GeoTIFF writing routine
int ProjectionMethod::GTEquals(ProjectionMethod *MatchMe)
{
long Ct;

if (MethodID != MatchMe->MethodID)
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (ParamID[Ct] != MatchMe->ParamID[Ct])
		return (0);
	if (ParamID[Ct] >= 0)
		{
		if (strcmp(ParamName[Ct], MatchMe->ParamName[Ct]))
			return (0);
		if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
			return (0);
		} // if
	} // for

return (1);

} // ProjectionMethod::GTEquals

/*===========================================================================*/

unsigned short ProjectionMethod::GetGeoTIFFMethodCode(void)
{
long Code;

MethodTable.FetchFieldValueLong(MethodID, "GTIFF_CODE", Code);
return ((unsigned short)Code);

} //  ProjectionMethod::GetGeoTIFFMethodCode

/*===========================================================================*/

void ProjectionMethod::SetMethod(char *NewName)
{
long NewID;

if ((NewID = MethodTable.FindRecordByFieldStr("NAME", NewName)) < 0)
	NewID = 0;

SetMethod(NewID);

} // ProjectionMethod::SetMethod

/*===========================================================================*/

void ProjectionMethod::SetMethod2(char *NewName)
{
long NewID;

if ((NewID = MethodTable.FindRecordByFieldStr("PRJ_NAME", NewName)) < 0)
	NewID = 0;

SetMethod(NewID);

} // ProjectionMethod::SetMethod2

/*===========================================================================*/

bool ProjectionMethod::ValidMethod(char *NewName)
{
long NewID;
bool rval = true;

NewID = MethodTable.FindRecordByFieldStr("NAME", NewName);
if (NewID < 0)
	rval = false;

return(rval);

} // ProjectionMethod::ValidMethod

/*===========================================================================*/

bool ProjectionMethod::ValidMethod2(char *NewName)
{
long NewID;
bool rval = true;

NewID = MethodTable.FindRecordByFieldStr("PRJ_NAME", NewName);
if (NewID < 0)
	rval = false;

return(rval);

} // ProjectionMethod::ValidMethod2

/*===========================================================================*/

void ProjectionMethod::SetMethodByCode(long NewCode)
{
long NewID;

if ((NewID = MethodTable.FindRecordByFieldLong("CODE", NewCode)) < 0)
	NewID = 0;

SetMethod(NewID);

} // ProjectionMethod::SetMethodByCode

/*===========================================================================*/

void ProjectionMethod::SetMethodByEPSGCode(long NewCode)
{
long NewID;
char EPSGCodeText[10];

sprintf(EPSGCodeText, "%1d", NewCode); // gary recommends %1d, who am I to disagree?

if ((NewID = MethodTable.FindRecordByFieldStr("EPSG_CODE", EPSGCodeText)) < 0)
	NewID = 0;

SetMethod(NewID);

} // ProjectionMethod::SetMethodByEPSGCode

/*===========================================================================*/

void ProjectionMethod::SetMethodByGeoTIFFCode(long NewCode)
{
long NewID;

if ((NewID = MethodTable.FindRecordByFieldLong("GTIFF_CODE", NewCode)) < 0)
	NewID = 0;

SetMethod(NewID);

} // ProjectionMethod::SetMethodByGeoTIFFCode

/*===========================================================================*/

int ProjectionMethod::SetMethodByPrjName(char *PrjName)
{
long NewID;

// try to do a full string match first, look for a truncated match if it failed
if ((NewID = MethodTable.FindRecordByFieldStr("PRJ_NAME", PrjName)) < 0)
	{
	if ((NewID = MethodTable.FindRecordByFieldStrTrunc("PRJ_NAME", PrjName)) < 0)
		NewID = 0;
	}

SetMethod(NewID);

return(!(NewID == 0));
} // ProjectionMethod::SetMethodByPrjName

/*===========================================================================*/


void ProjectionMethod::SetMethod(long NewID)
{
long ParamNumber, Ct, LoopCt, TempID;
char Str[256], BaseName[64];

if (MethodTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	strncpy(MethodName, Str, WCS_EFFECT_MAXNAMELENGTH);
	MethodName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	MethodID = NewID;
	strcpy(BaseName, "GCTP_CODE");
	if (MethodTable.FetchFieldValueLong(NewID, BaseName, ParamNumber))
		GCTPMethod = ParamNumber;
	else
		GCTPMethod = 0;
	for (Ct = LoopCt = 0; LoopCt < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; LoopCt ++)
		{
		sprintf(BaseName, "PARAM%d", LoopCt + 1);
		if (MethodTable.FetchFieldValueLong(NewID, BaseName, ParamNumber))
			{
			ParamID[Ct] = ParamNumber;
			if ((TempID = ParamTable.FindRecordByFieldLong("CODE", ParamID[Ct])) >= 0)
				{
				if (ParamTable.FetchFieldValueStr(TempID, "NAME", Str, 256))
					{
					strncpy(ParamName[Ct], Str, WCS_EFFECT_MAXNAMELENGTH);
					ParamName[Ct][WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
					AnimPar[Ct].SetValue(0.0);
					} // if
				} // if
			else
				ParamName[Ct][0] = 0;
			Ct ++;
			} // if
		} // for
	for ( ; Ct < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; Ct ++)
		{
		ParamID[Ct] = -1;
		ParamName[Ct][0] = 0;
		} // for
	} // if
else
	{
	// set it to geographic
	MethodID = 0;
	GCTPMethod = 0;
	for ( Ct = 0; Ct < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; Ct ++)
		{
		ParamID[Ct] = -1;
		ParamName[Ct][0] = 0;
		} // for
	} // for

SetParamMetrics();

} // ProjectionMethod::SetMethod

/*===========================================================================*/

void ProjectionMethod::SetMethodParams(long NewID)
{
long ParamNumber, Ct, LoopCt, TempID;
char Str[256], BaseName[64];

if (MethodTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	for (Ct = LoopCt = 0; LoopCt < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; LoopCt ++)
		{
		sprintf(BaseName, "PARAM%d", LoopCt + 1);
		if (MethodTable.FetchFieldValueLong(NewID, BaseName, ParamNumber))
			{
			ParamID[Ct] = ParamNumber;
			if ((TempID = ParamTable.FindRecordByFieldLong("CODE", ParamID[Ct])) >= 0)
				{
				if (ParamTable.FetchFieldValueStr(TempID, "NAME", Str, 256))
					{
					strncpy(ParamName[Ct], Str, WCS_EFFECT_MAXNAMELENGTH);
					ParamName[Ct][WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
					} // if
				} // if
			else
				ParamName[Ct][0] = 0;
			Ct ++;
			} // if
		} // for
	for ( ; Ct < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; Ct ++)
		{
		ParamID[Ct] = -1;
		ParamName[Ct][0] = 0;
		} // for
	} // if
else
	{
	// set it to geographic
	for ( Ct = 0; Ct < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; Ct ++)
		{
		ParamID[Ct] = -1;
		ParamName[Ct][0] = 0;
		} // for
	} // for

SetParamMetrics();

} // ProjectionMethod::SetMethodParams

/*===========================================================================*/

void ProjectionMethod::SetParamMetrics(void)
{
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; Ct ++)
	{
	if (ParamID[Ct] >= 0)
		{
		switch (ParamID[Ct])
			{
			case 10:	// Scale factor
			case 22:	// Period of revolution
			case 23:	// Landsat norhern ratio
			case 24:	// End of path flag
			case 25:	// Landsat satellite number
			case 26:	// Landsat path number
			case 27:	// Shape parameter m
			case 28:	// Shape parameter n
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
				break;
				} // 
			case 13:	// Height perspective point
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
				break;
				} // 
			case 6:		// False easting
			case 7:		// False northing
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
				break;
				} // 
			case 18:	// Azimuth angle
			case 20:	// Inclination of orbit
			case 29:	// Rotation angle
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
				break;
				} // 
			case 1:		// Lat std parallel
			case 2:		// Lat 1st std parallel
			case 3:		// Lat 2nd std parallel
			case 5:		// Lat projection origin
			case 8:		// Lat true scale
			case 12:	// Lat center projection
			case 16:	// Lat 1st point
			case 17:	// Lat 2nd point
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
				break;
				} // 
			case 4:		// Lon central meridian
			case 9:		// Lon below pole
			case 11:	// Lon center projection
			case 14:	// Lon 1st point
			case 15:	// Lon 2nd point
			case 19:	// Lon azimuth point
			case 21:	// Lon ascending orbit
				{
				AnimPar[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
				break;
				} // 
			} // switch
		} // if
	} // for

} // ProjectionMethod::SetParamMetrics

/*===========================================================================*/

long ProjectionMethod::IdentifyMethodIDFromName(char *ProjMethodName)
{
long NewID, NewCode;

if ((NewID = MethodTable.FindRecordByFieldStr("NAME", ProjMethodName)) >= 0)
	{
	if (MethodTable.FetchFieldValueLong(NewID, "GCTP_CODE", NewCode))
		GCTPMethod = NewCode;
	else
		GCTPMethod = 0;
	return (NewID);
	} // if

return (0);

} // ProjectionMethod::IdentifyMethodIDFromName

/*===========================================================================*/

int ProjectionMethod::FindParamAnimParSlotByGCTPID(int GCTPID)
{
int ParamScan;

for (ParamScan = 0; ParamScan < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamScan++)
	{
	if (ParamID[ParamScan] == GCTPID)
		{
		return(ParamScan);
		} // if
	} // for

return(-1);
} // ProjectionMethod::FindParamAnimParSlotByGCTPID

/*===========================================================================*/

ULONG ProjectionMethod::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_METHODNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MethodName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						MethodID = IdentifyMethodIDFromName(MethodName);
						SetMethodParams(MethodID);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM1:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM1].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM2:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM3:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM3].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM4:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM4].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM5:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM5].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM6:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM6].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM7:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM7].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PROJMETHOD_PARAM8:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM8].Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // ProjectionMethod::Load

/*===========================================================================*/

unsigned long ProjectionMethod::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_PROJMETHOD_NUMANIMPAR] = {WCS_EFFECTS_PROJMETHOD_PARAM1,
																	WCS_EFFECTS_PROJMETHOD_PARAM2,
																	WCS_EFFECTS_PROJMETHOD_PARAM3,
																	WCS_EFFECTS_PROJMETHOD_PARAM4,
																	WCS_EFFECTS_PROJMETHOD_PARAM5,
																	WCS_EFFECTS_PROJMETHOD_PARAM6,
																	WCS_EFFECTS_PROJMETHOD_PARAM7,
																	WCS_EFFECTS_PROJMETHOD_PARAM8};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PROJMETHOD_METHODNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(MethodName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MethodName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ProjectionMethod::Save

/*===========================================================================*/

char *ProjectionMethod::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		{
		if (ParamID[Ct] >= 0)
			{
			return (ParamName[Ct]);
			} // if
		} // if
	} // for

return ("");

} // ProjectionMethod::GetCritterName

/*===========================================================================*/

int ProjectionMethod::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // ProjectionMethod::SetToTime

/*===========================================================================*/

int ProjectionMethod::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (ParamID[Ct] >= 0 && GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // ProjectionMethod::GetRAHostAnimated

/*===========================================================================*/

long ProjectionMethod::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (ParamID[Ct] >= 0 && GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // ProjectionMethod::GetKeyFrameRange

/*===========================================================================*/

char ProjectionMethod::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);

return (0);

} // ProjectionMethod::GetRAHostDropOK

/*===========================================================================*/

int ProjectionMethod::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (ProjectionMethod *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (ProjectionMethod *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // ProjectionMethod::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long ProjectionMethod::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

if (GCTPMethod)
	Mask &= (WCS_RAHOST_ICONTYPE_COORDSYS | WCS_RAHOST_FLAGBIT_CHILDREN |
		WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);
else
	Mask &= (WCS_RAHOST_ICONTYPE_COORDSYS | 
		WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // ProjectionMethod::GetRAFlags

/*===========================================================================*/

void ProjectionMethod::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = MethodName;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_TERRAFFECTOR];
	Prop->Ext = "prm";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // ProjectionMethod::GetRAHostProperties

/*===========================================================================*/

int ProjectionMethod::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(MethodName, Prop->Name, WCS_EFFECT_MAXNAMELENGTH);
	MethodName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // ProjectionMethod::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *ProjectionMethod::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found && ParamID[Ct] >= 0)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // ProjectionMethod::GetRAHostChild

/*===========================================================================*/

int ProjectionMethod::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	} // if


return (0);

} // ProjectionMethod::GetAffiliates

/*===========================================================================*/

int ProjectionMethod::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // ProjectionMethod::GetPopClassFlags

/*===========================================================================*/

int ProjectionMethod::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // ProjectionMethod::AddSRAHBasePopMenus

/*===========================================================================*/

int ProjectionMethod::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // ProjectionMethod::HandleSRAHPopMenuSelection

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int ProjectionMethod::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ProjectionMethod *CurrentMethod = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	// set some global pointers so that things know what libraries to link to
	GlobalApp->LoadToEffectsLib = LoadToEffects;

	while (BytesRead && Success)
		{
		// read block descriptor tag from file 
		if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
			WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
			{
			TotalRead += BytesRead;
			ReadBuf[8] = 0;
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&Size,
				WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				if (! strnicmp(ReadBuf, "DEMBnds", 8))
					{
					if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
						OldBoundsLoaded = 1;
					} // if DEMBnds
				else if (! strnicmp(ReadBuf, "ProjMeth", 8))
					{
					if (CurrentMethod = new ProjectionMethod(NULL))
						{
						if ((BytesRead = CurrentMethod->Load(ffile, Size, ByteFlip)) == Size)
							Success = 1;	// we got our man
						}
					} // if Strata
				else if (! fseek(ffile, Size, SEEK_CUR))
					BytesRead = Size;
				TotalRead += BytesRead;
				if (BytesRead != Size)
					{
					Success = 0;
					break;
					} // if error
				} // if size block read 
			else
				break;
			} // if tag block read 
		else
			break;
		} // while 
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentMethod)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	Copy(this, CurrentMethod);
	delete CurrentMethod;
	} // if

if (LoadToEffects)
	delete LoadToEffects;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

return (Success);

} // ProjectionMethod::LoadObject

/*===========================================================================*/

int ProjectionMethod::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Projection Method
strcpy(StrBuf, "ProjMeth");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Projection Method saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // ProjectionMethod::SaveObject

/*===========================================================================*/
/*===========================================================================*/

GeoDatum::GeoDatum(void)
: RasterAnimHost(NULL), Ellipse(this)
{

SetDefaults();

} // GeoDatum::GeoDatum

/*===========================================================================*/

GeoDatum::GeoDatum(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost), Ellipse(this)
{

SetDefaults();

} // GeoDatum::GeoDatum

/*===========================================================================*/

void GeoDatum::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_GEODATUM_NUMANIMPAR] = {0.0, 0.0, 0.0};
double RangeDefaults[WCS_EFFECTS_GEODATUM_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);

SetDatum(1);

} // GeoDatum::SetDefaults

/*===========================================================================*/

void GeoDatum::Copy(GeoDatum *CopyTo, GeoDatum *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
strcpy(CopyTo->DatumName, CopyFrom->DatumName);
CopyTo->Ellipse.Copy(&CopyTo->Ellipse, &CopyFrom->Ellipse);
CopyTo->DatumID = CopyFrom->DatumID;

} // GeoDatum::Copy

/*===========================================================================*/

int GeoDatum::Equals(GeoDatum *MatchMe)
{
long Ct;

if (strcmp(DatumName, MatchMe->DatumName))
	return (0);
if (DatumID != MatchMe->DatumID)
	return (0);
if (! Ellipse.Equals(&MatchMe->Ellipse))
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
		return (0);
	} // for

return (1);

} // GeoDatum::Equals

/*===========================================================================*/

// for use by GeoTIFF writing routine
int GeoDatum::GTEquals(GeoDatum *MatchMe)
{
long Ct;

if (DatumID != MatchMe->DatumID)
	return (0);
if (! Ellipse.GTEquals(&MatchMe->Ellipse))
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
		return (0);
	} // for

return (1);

} // GeoDatum::GTEquals

/*===========================================================================*/

void GeoDatum::SetDatum(char *NewName)
{
long NewID;

if ((NewID = DatmTable.FindRecordByFieldStr("NAME", NewName)) < 0)
	NewID = 0;

SetDatum(NewID);

} // GeoDatum::SetDatum

/*===========================================================================*/

void GeoDatum::SetDatumByCode(long NewCode)
{
long NewID;

if ((NewID = DatmTable.FindRecordByFieldLong("CODE", NewCode)) < 0)
	NewID = 0;

SetDatum(NewID);

} // GeoDatum::SetDatumByCode

/*===========================================================================*/

void GeoDatum::SetDatumByEPSGCode(long NewCode)
{
long NewID;
char EPSGCodeText[10];

sprintf(EPSGCodeText, "%1d", NewCode); // gary recommends %1d, who am I to disagree?

if ((NewID = DatmTable.FindRecordByFieldStr("EPSG_CODE", EPSGCodeText)) < 0)
	NewID = 0;

SetDatum(NewID);

} // GeoDatum::SetDatumByEPSGCode

/*===========================================================================*/

int GeoDatum::SetDatumByERMName(char *ERMName)
{
long NewID;

if ((NewID = DatmTable.FindRecordByFieldStr("ERM_NAME", ERMName)) < 0)
	NewID = 0;

SetDatum(NewID);

return(!(NewID == 0));
} // GeoDatum::SetDatumByERMName

/*===========================================================================*/

int GeoDatum::SetDatumByPrjName(char *PrjName)
{
long NewID;

if ((NewID = DatmTable.FindRecordByFieldStr("PRJ_NAME", PrjName)) < 0)
	NewID = 0;

SetDatum(NewID);

return(!(NewID == 0));
} // GeoDatum::SetDatumByPrjName

/*===========================================================================*/


void GeoDatum::SetDatum(long NewID)
{
double DeltaX, DeltaY, DeltaZ;
long NewEllipseID;
char Str[256];

if (DatmTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	strncpy(DatumName, Str, WCS_EFFECT_MAXNAMELENGTH);
	DatumName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	DatumID = NewID;
	if (DatmTable.FetchFieldValueDbl(NewID, "DELTA_X", DeltaX))
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].SetValue(DeltaX);
	else if (NewID != 0)
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].SetValue(0.0);
	if (DatmTable.FetchFieldValueDbl(NewID, "DELTA_Y", DeltaY))
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].SetValue(DeltaY);
	else if (NewID != 0)
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].SetValue(0.0);
	if (DatmTable.FetchFieldValueDbl(NewID, "DELTA_Z", DeltaZ))
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].SetValue(DeltaZ);
	else if (NewID != 0)
		AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].SetValue(0.0);
	if (DatmTable.FetchFieldValueLong(NewID, "ELLIPSOID", NewEllipseID))
		{
		if ((NewEllipseID = Ellipse.EllipseTable.FindRecordByFieldLong("CODE", NewEllipseID)) >= 0)
			Ellipse.SetEllipsoid(NewEllipseID);
		else if (NewID != 0)
			Ellipse.SetEllipsoid(0L);
		} // if
	else if (NewID != 0)
		Ellipse.SetEllipsoid(0L);
	} // if
else
	// set it to custom
	DatumID = 0;

} // GeoDatum::SetDatum

/*===========================================================================*/

long GeoDatum::IdentifyDatumIDFromName(char *GeoDatumName)
{
long NewID;

if ((NewID = DatmTable.FindRecordByFieldStr("NAME", GeoDatumName)) >= 0)
	return (NewID);

return (0);

} // GeoDatum::IdentifyDatumIDFromName

/*===========================================================================*/

unsigned short GeoDatum::GetEPSGDatumCodeFromName(char *GeoDatumName)
{
long NewID, EPSG_Code = 0;

if ((NewID = DatmTable.FindRecordByFieldStr("NAME", GeoDatumName)) > 0)
	{
	DatmTable.FetchFieldValueLong(NewID, "EPSG_CODE", EPSG_Code);
	} // if

return ((unsigned short)EPSG_Code);

} // GeoDatum::IdentifyDatumIDFromName

/*===========================================================================*/

ULONG GeoDatum::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_GEODATUM_DATUMNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)DatumName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						DatumID = IdentifyDatumIDFromName(DatumName);
						break;
						}
					case WCS_EFFECTS_GEODATUM_DELTAX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEODATUM_DELTAY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEODATUM_DELTAZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEODATUM_ELLIPSOID:
						{
						BytesRead = Ellipse.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // GeoDatum::Load

/*===========================================================================*/

unsigned long GeoDatum::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_GEODATUM_NUMANIMPAR] = {WCS_EFFECTS_GEODATUM_DELTAX,
																	WCS_EFFECTS_GEODATUM_DELTAY,
																	WCS_EFFECTS_GEODATUM_DELTAZ};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GEODATUM_DATUMNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DatumName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)DatumName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_EFFECTS_GEODATUM_ELLIPSOID + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Ellipse.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if geo datum saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GeoDatum::Save

/*===========================================================================*/

char *GeoDatumCritterNames[WCS_EFFECTS_GEODATUM_NUMANIMPAR] = {"Delta X", "Delta Y", "Delta Z"};

char *GeoDatum::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (GeoDatumCritterNames[Ct]);
	} // for

return ("");

} // GeoDatum::GetCritterName

/*===========================================================================*/

int GeoDatum::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (Ellipse.SetToTime(Time))
	Found = 1;

return (Found);

} // GeoDatum::SetToTime

/*===========================================================================*/

int GeoDatum::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
if (Ellipse.GetRAHostAnimated())
	return (1);

return (0);

} // GeoDatum::GetRAHostAnimated

/*===========================================================================*/

long GeoDatum::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
if (Ellipse.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // GeoDatum::GetKeyFrameRange

/*===========================================================================*/

char GeoDatum::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_GEOELLIPSOID)
	return (1);

return (0);

} // GeoDatum::GetRAHostDropOK

/*===========================================================================*/

int GeoDatum::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (GeoDatum *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (GeoDatum *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_GEOELLIPSOID)
	{
	Success = Ellipse.ProcessRAHostDragDrop(DropSource);
	} // else if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // GeoDatum::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long GeoDatum::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_COORDSYS | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // GeoDatum::GetRAFlags

/*===========================================================================*/

void GeoDatum::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = DatumName;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_TERRAFFECTOR];
	Prop->Ext = "gdt";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // GeoDatum::GetRAHostProperties

/*===========================================================================*/

int GeoDatum::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(DatumName, Prop->Name, WCS_EFFECT_MAXNAMELENGTH);
	DatumName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // GeoDatum::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *GeoDatum::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
if (Found)
	return (&Ellipse);

return (NULL);

} // GeoDatum::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *GeoDatum::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX))
	return (GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY))
	return (GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ))
	return (GetAnimPtr(WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX));

return (NULL);

} // GeoDatum::GetNextGroupSibling

/*===========================================================================*/

int GeoDatum::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	} // if


return (0);

} // GeoDatum::GetAffiliates

/*===========================================================================*/

int GeoDatum::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // GeoDatum::GetPopClassFlags

/*===========================================================================*/

int GeoDatum::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // GeoDatum::AddSRAHBasePopMenus

/*===========================================================================*/

int GeoDatum::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // GeoDatum::HandleSRAHPopMenuSelection

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int GeoDatum::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
GeoDatum *CurrentDatum = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	// set some global pointers so that things know what libraries to link to
	GlobalApp->LoadToEffectsLib = LoadToEffects;

	while (BytesRead && Success)
		{
		// read block descriptor tag from file 
		if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
			WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
			{
			TotalRead += BytesRead;
			ReadBuf[8] = 0;
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&Size,
				WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				if (! strnicmp(ReadBuf, "DEMBnds", 8))
					{
					if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
						OldBoundsLoaded = 1;
					} // if DEMBnds
				else if (! strnicmp(ReadBuf, "GeoDatum", 8))
					{
					if (CurrentDatum = new GeoDatum(NULL))
						{
						if ((BytesRead = CurrentDatum->Load(ffile, Size, ByteFlip)) == Size)
							Success = 1;	// we got our man
						}
					} // if Strata
				else if (! fseek(ffile, Size, SEEK_CUR))
					BytesRead = Size;
				TotalRead += BytesRead;
				if (BytesRead != Size)
					{
					Success = 0;
					break;
					} // if error
				} // if size block read 
			else
				break;
			} // if tag block read 
		else
			break;
		} // while 
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentDatum)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	Copy(this, CurrentDatum);
	delete CurrentDatum;
	} // if

if (LoadToEffects)
	delete LoadToEffects;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

return (Success);

} // GeoDatum::LoadObject

/*===========================================================================*/

int GeoDatum::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// GeoDatum
strcpy(StrBuf, "GeoDatum");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if GeoDatum saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // GeoDatum::SaveObject

/*===========================================================================*/
/*===========================================================================*/

GeoEllipsoid::GeoEllipsoid(void)
: RasterAnimHost(NULL)
{

SetDefaults();

} // GeoEllipsoid::GeoEllipsoid

/*===========================================================================*/

GeoEllipsoid::GeoEllipsoid(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // GeoEllipsoid::GeoEllipsoid

/*===========================================================================*/

void GeoEllipsoid::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR] = {EffectsLib::CelestialPresetRadius[3], EffectsLib::CelestialPresetRadius[3]};
double RangeDefaults[WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR][3] = {FLT_MAX, 1.0, 1.0,
															FLT_MAX, 1.0, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);

SetEllipsoid(1);

} // GeoEllipsoid::SetDefaults

/*===========================================================================*/

void GeoEllipsoid::Copy(GeoEllipsoid *CopyTo, GeoEllipsoid *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
strcpy(CopyTo->EllipsoidName, CopyFrom->EllipsoidName);
CopyTo->EllipsoidID = CopyFrom->EllipsoidID;

} // GeoEllipsoid::Copy

/*===========================================================================*/

int GeoEllipsoid::Equals(GeoEllipsoid *MatchMe)
{
long Ct;

if (strcmp(EllipsoidName, MatchMe->EllipsoidName))
	return (0);
if (EllipsoidID != MatchMe->EllipsoidID)
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
		return (0);
	} // for

return (1);

} // GeoEllipsoid::Equals

/*===========================================================================*/

// for use by GeoTIFF writing routine
int GeoEllipsoid::GTEquals(GeoEllipsoid *MatchMe)
{
long Ct;

if (EllipsoidID != MatchMe->EllipsoidID)
	return (0);
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (AnimPar[Ct].CurValue != MatchMe->AnimPar[Ct].CurValue)
		return (0);
	} // for

return (1);

} // GeoEllipsoid::GTEquals

/*===========================================================================*/

void GeoEllipsoid::SetEllipsoid(char *NewName)
{
long NewID;

if ((NewID = EllipseTable.FindRecordByFieldStr("NAME", NewName)) < 0)
	NewID = 0;

SetEllipsoid(NewID);

} // GeoEllipsoid::SetEllipsoid

/*===========================================================================*/

void GeoEllipsoid::SetEllipsoidByCode(long NewCode)
{
long NewID;

if ((NewID = EllipseTable.FindRecordByFieldLong("CODE", NewCode)) < 0)
	NewID = 0;

SetEllipsoid(NewID);

} // GeoEllipsoid::SetEllipsoidByCode

/*===========================================================================*/

void GeoEllipsoid::SetEllipsoidByEPSGCode(long NewCode)
{
long NewID;
char EPSGCodeText[10];

sprintf(EPSGCodeText, "%1d", NewCode); // gary recommends %1d, who am I to disagree?
if ((NewID = EllipseTable.FindRecordByFieldStr("EPSG_CODE", EPSGCodeText)) < 0)
	NewID = 0;

SetEllipsoid(NewID);

} // GeoEllipsoid::SetEllipsoidByEPSGCode

/*===========================================================================*/

int GeoEllipsoid::SetEllipsoidByPrjName(char *PrjName)
{
long NewID;

if ((NewID = EllipseTable.FindRecordByFieldStr("PRJ_NAME", PrjName)) < 0)
	NewID = 0;

SetEllipsoid(NewID);

return(!(NewID == 0));
} // GeoEllipsoid::SetEllipsoidByEPSGCode

/*===========================================================================*/


void GeoEllipsoid::SetEllipsoid(long NewID)
{
double SemiMajor, Flattening, SemiMinor;
char Str[256];

if (EllipseTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	strncpy(EllipsoidName, Str, WCS_EFFECT_MAXNAMELENGTH);
	EllipsoidName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	EllipsoidID = NewID;
	if (EllipseTable.FetchFieldValueDbl(NewID, "MAJOR_AXIS", SemiMajor))
		{
		if (EllipseTable.FetchFieldValueDbl(NewID, "FLATTENING", Flattening))
			{
			if (Flattening > 0.0)
				SemiMinor = SemiMajor - SemiMajor / Flattening;
			else
				SemiMinor = SemiMajor;
			AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(SemiMajor);
			AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(SemiMinor);
			} // if
		} // if
	} // if
else
	// set it to custom
	EllipsoidID = 0;

} // GeoEllipsoid::SetEllipsoid

/*===========================================================================*/

long GeoEllipsoid::IdentifyEllipsoidIDFromName(char *GeoEllipseName)
{
long NewID;

if ((NewID = EllipseTable.FindRecordByFieldStr("NAME", GeoEllipseName)) >= 0)
	return (NewID);

return (0);

} // GeoEllipsoid::IdentifyEllipsoidIDFromName

/*===========================================================================*/

unsigned short GeoEllipsoid::GetEPSGEllipsoidCodeFromName(char *GeoEllipseName)
{
long NewID, EPSG_Code = 0;

if ((NewID = EllipseTable.FindRecordByFieldStr("NAME", GeoEllipseName)) > 0)
	{
	EllipseTable.FetchFieldValueLong(NewID, "EPSG_CODE", EPSG_Code);
	} // if

return ((unsigned short)EPSG_Code);

} // GeoEllipsoid::GetEPSGEllipsoidCodeFromName

/*===========================================================================*/

ULONG GeoEllipsoid::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_GEOELLIPSOID_ELLIPSENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)EllipsoidName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						EllipsoidID = IdentifyEllipsoidIDFromName(EllipsoidName);
						break;
						}
					case WCS_EFFECTS_GEOELLIPSOID_SEMIMAJOR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEOELLIPSOID_SEMIMINOR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // GeoEllipsoid::Load

/*===========================================================================*/

unsigned long GeoEllipsoid::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR] = {WCS_EFFECTS_GEOELLIPSOID_SEMIMAJOR, WCS_EFFECTS_GEOELLIPSOID_SEMIMINOR};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GEOELLIPSOID_ELLIPSENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(EllipsoidName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)EllipsoidName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // GeoEllipsoid::Save

/*===========================================================================*/

char *GeoEllipsoidCritterNames[WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR] = {"Semi-major Axis", "Semi-minor Axis"};

char *GeoEllipsoid::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (GeoEllipsoidCritterNames[Ct]);
	} // for

return ("");

} // GeoEllipsoid::GetCritterName

/*===========================================================================*/

int GeoEllipsoid::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // GeoEllipsoid::SetToTime

/*===========================================================================*/

int GeoEllipsoid::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // GeoEllipsoid::GetRAHostAnimated

/*===========================================================================*/

long GeoEllipsoid::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // GeoEllipsoid::GetKeyFrameRange

/*===========================================================================*/

char GeoEllipsoid::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);

return (0);

} // GeoEllipsoid::GetRAHostDropOK

/*===========================================================================*/

int GeoEllipsoid::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (GeoEllipsoid *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (GeoEllipsoid *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // GeoEllipsoid::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long GeoEllipsoid::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_COORDSYS | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // GeoEllipsoid::GetRAFlags

/*===========================================================================*/

void GeoEllipsoid::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = EllipsoidName;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_TERRAFFECTOR];
	Prop->Ext = "gel";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // GeoEllipsoid::GetRAHostProperties

/*===========================================================================*/

int GeoEllipsoid::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(EllipsoidName, Prop->Name, WCS_EFFECT_MAXNAMELENGTH);
	EllipsoidName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // GeoEllipsoid::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *GeoEllipsoid::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // GeoEllipsoid::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *GeoEllipsoid::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR))
	return (GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR))
	return (GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR));

return (NULL);

} // GeoEllipsoid::GetNextGroupSibling

/*===========================================================================*/

int GeoEllipsoid::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	} // if


return (0);

} // GeoEllipsoid::GetAffiliates

/*===========================================================================*/

int GeoEllipsoid::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // GeoEllipsoid::GetPopClassFlags

/*===========================================================================*/

int GeoEllipsoid::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // GeoEllipsoid::AddSRAHBasePopMenus

/*===========================================================================*/

int GeoEllipsoid::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // GeoEllipsoid::HandleSRAHPopMenuSelection

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int GeoEllipsoid::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
GeoEllipsoid *CurrentEllipse = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	// set some global pointers so that things know what libraries to link to
	GlobalApp->LoadToEffectsLib = LoadToEffects;

	while (BytesRead && Success)
		{
		// read block descriptor tag from file 
		if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
			WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
			{
			TotalRead += BytesRead;
			ReadBuf[8] = 0;
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&Size,
				WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				if (! strnicmp(ReadBuf, "DEMBnds", 8))
					{
					if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
						OldBoundsLoaded = 1;
					} // if DEMBnds
				else if (! strnicmp(ReadBuf, "GeoEllip", 8))
					{
					if (CurrentEllipse = new GeoEllipsoid(NULL))
						{
						if ((BytesRead = CurrentEllipse->Load(ffile, Size, ByteFlip)) == Size)
							Success = 1;	// we got our man
						}
					} // if Strata
				else if (! fseek(ffile, Size, SEEK_CUR))
					BytesRead = Size;
				TotalRead += BytesRead;
				if (BytesRead != Size)
					{
					Success = 0;
					break;
					} // if error
				} // if size block read 
			else
				break;
			} // if tag block read 
		else
			break;
		} // while 
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentEllipse)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	Copy(this, CurrentEllipse);
	delete CurrentEllipse;
	} // if

if (LoadToEffects)
	delete LoadToEffects;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

return (Success);

} // GeoEllipsoid::LoadObject

/*===========================================================================*/

int GeoEllipsoid::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// GeoEllipsoid
strcpy(StrBuf, "GeoEllip");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if GeoEllipsoid saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // GeoEllipsoid::SaveObject

/*===========================================================================*/
/*===========================================================================*/

CoordSys::CoordSys()
: GeneralEffect(NULL), Method(this), Datum(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_COORDSYS;
SetDefaults();

} // CoordSys::CoordSys

/*===========================================================================*/

CoordSys::CoordSys(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), Method(this), Datum(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_COORDSYS;
SetDefaults();

} // CoordSys::CoordSys

/*===========================================================================*/

CoordSys::CoordSys(RasterAnimHost *RAHost, EffectsLib *Library, CoordSys *Proto)
: GeneralEffect(RAHost), Method(this), Datum(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_COORDSYS;
Prev = Library->LastCoord;
if (Library->LastCoord)
	{
	Library->LastCoord->Next = this;
	Library->LastCoord = this;
	} // if
else
	{
	Library->Coord = Library->LastCoord = this;
	} // else
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	if (Proto->Name[0])
		strncpy(NameBase, Proto->Name, WCS_EFFECT_MAXNAMELENGTH - 1);
	else if (Name[0])
		strcpy(NameBase, Name);
	else
		strcpy(NameBase, "Coordinate System");
	NameBase[WCS_EFFECT_MAXNAMELENGTH - 1] = NULL; // ensure NULL-terminated after strncpy
	} // if
else if (Name[0])
	{
	strcpy(NameBase, Name);
	} // else
else
	{
	strcpy(NameBase, "Coordinate System");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // CoordSys::CoordSys

/*===========================================================================*/

CoordSys::~CoordSys()
{
ImageList *NextImage;
RasterAttribute *MyAttr;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->COS && GlobalApp->GUIWins->COS->GetActive() == this)
		{
		delete GlobalApp->GUIWins->COS;
		GlobalApp->GUIWins->COS = NULL;
		} // if
	if (GlobalApp->GUIWins->TBG && GlobalApp->GUIWins->TBG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->TBG;
		GlobalApp->GUIWins->TBG = NULL;
		} // if
	} // if

while (Images)
	{
	NextImage = Images->Next;
	if (Images->Me->GetRaster())
		{
		if ((MyAttr = Images->Me->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			MyAttr->GetShell()->SetHostNotify(NULL);
			} // if
		} // if
	Images = NextImage;
	} // if

} // CoordSys::~CoordSys

/*===========================================================================*/

void CoordSys::SetDefaults(void)
{

Initialized = 0;
ProjSysName[0] = 0;
ZoneName[0] = 0;
ProjSysID = ZoneID = 0;
Images = NULL;
DeltaX = DeltaY = DeltaZ = EccSq = EccPrimeSq = 0.0;
ErrorCode = 0;
Geographic = 0;		// used by renderer
memset(ToidValues, 0, sizeof ToidValues);

CachedLat = CachedLon = -FLT_MAX;

#ifdef WCS_BUILD_VNS
SetSystem(5);
#else
SetSystem(1);
#endif // WCS_BUILD_VNS

} // CoordSys::SetDefaults

/*===========================================================================*/

void CoordSys::Copy(CoordSys *CopyTo, CoordSys *CopyFrom)
{

strcpy(CopyTo->ProjSysName, CopyFrom->ProjSysName);
strcpy(CopyTo->ZoneName, CopyFrom->ZoneName);
CopyTo->ProjSysID = CopyFrom->ProjSysID;
CopyTo->ZoneID = CopyFrom->ZoneID;
CopyTo->Geographic = CopyFrom->Geographic;
SetZoneFile();
CopyTo->Method.Copy(&CopyTo->Method, &CopyFrom->Method);
CopyTo->Datum.Copy(&CopyTo->Datum, &CopyFrom->Datum);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);
CopyTo->Initialized = 0;

} // CoordSys::Copy

/*===========================================================================*/

int CoordSys::Equals(CoordSys *MatchMe)
{

if (strcmp(ProjSysName, MatchMe->ProjSysName))
	return (0);
if (strcmp(ZoneName, MatchMe->ZoneName))
	return (0);
if (ProjSysID != MatchMe->ProjSysID)
	return (0);
if (ZoneID != MatchMe->ZoneID)
	return (0);
if (! Method.Equals(&MatchMe->Method))
	return (0);
if (! Datum.Equals(&MatchMe->Datum))
	return (0);

return (1);

} // CoordSys::Equals

/*===========================================================================*/

// for use by GeoTIFF writing routine
int CoordSys::GTEquals(CoordSys *MatchMe)
{

if (ProjSysID != MatchMe->ProjSysID)
	return (0);
if (ZoneID != MatchMe->ZoneID)
	return (0);
if (! Method.GTEquals(&MatchMe->Method))
	return (0);
if (! Datum.GTEquals(&MatchMe->Datum))
	return (0);

return (1);

} // CoordSys::GTEquals

/*===========================================================================*/

unsigned short CoordSys::GetEPSGStatePlaneCode(void)
{
long NewID, Code = 0;

if (ZoneName)
	{
	NewID = ZoneTable.FindRecordByFieldStr("NAME", ZoneName);
	if (NewID > 0)
		ZoneTable.FetchFieldValueLong(NewID, "EPSG_PCS", Code);
	} // if

return ((unsigned short)Code);

} // CoordSys::GetEPSGStatePlaneCode

/*===========================================================================*/

void CoordSys::SetSystem(char *NewName)
{
long NewID;

if ((NewID = ProjSysTable.FindRecordByFieldStr("NAME", NewName)) < 0)
	NewID = 0;

SetSystem(NewID);

} // CoordSys::SetSystem

/*===========================================================================*/

void CoordSys::SetSystemByCode(long NewCode)
{
long NewID;

if ((NewID = ProjSysTable.FindRecordByFieldLong("CODE", NewCode)) < 0)
	NewID = 0;

SetSystem(NewID);

} // CoordSys::SetSystemByCode

/*===========================================================================*/

void CoordSys::SetSystem(long NewID)
{
long NewMethodID, NewDatumID, NewParamID, ParamTagCt, ParamCt, NewProjSysCode;
double NewParamValue;
char Str[256], ParamTag[32], ParamValTag[32];

if (ProjSysTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	strncpy(ProjSysName, Str, WCS_EFFECT_MAXNAMELENGTH);
	ProjSysName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	if (GlobalApp && GlobalApp->AppEffects)
		SetUniqueName(GlobalApp->AppEffects, ProjSysName);
	ProjSysID = NewID;
	if (ProjSysTable.FetchFieldValueLong(NewID, "METHOD", NewMethodID))
		{
		if ((NewMethodID = Method.MethodTable.FindRecordByFieldLong("CODE", NewMethodID)) >= 0)
			Method.SetMethod(NewMethodID);
		} // if
	if (ProjSysTable.FetchFieldValueLong(NewID, "DATUM", NewDatumID))
		{
		if ((NewDatumID = Datum.DatmTable.FindRecordByFieldLong("CODE", NewDatumID)) >= 0)
			Datum.SetDatum(NewDatumID);
		} // if
	for (ParamTagCt = 0; ParamTagCt < 5; ParamTagCt ++)
		{
		sprintf(ParamTag, "PARAM%d", ParamTagCt + 1);
		sprintf(ParamValTag, "P%dVALUE", ParamTagCt + 1);
		if (ProjSysTable.FetchFieldValueLong(NewID, ParamTag, NewParamID))
			{
			if (ProjSysTable.FetchFieldValueDbl(NewID, ParamValTag, NewParamValue))
				{
				for (ParamCt = 0; ParamCt < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamCt ++)
					{
					if (Method.ParamID[ParamCt] >= 0 && Method.ParamID[ParamCt] == NewParamID)
						{
						Method.AnimPar[ParamCt].SetValue(NewParamValue);
						} // if
					} // for
				} // if
			} // for
		// for surveyed data need to enter a reasonable center lat/lon
		if (ProjSysTable.FetchFieldValueLong(ProjSysID, "CODE", NewProjSysCode))
			{
			if (NewProjSysCode >= 14 && NewProjSysCode <= 17)
				{
				for (ParamCt = 0; ParamCt < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamCt ++)
					{
					if (Method.ParamID[ParamCt] == 5)
						{
						Method.AnimPar[ParamCt].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
						} // if
					if (Method.ParamID[ParamCt] == 4)
						{
						Method.AnimPar[ParamCt].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
						} // if
					} // for
				} // if
			} // if
		} // if
	if (ProjSysTable.FetchFieldValueStr(NewID, "ZONE_FILE", Str, 251))
		{
		strcat(Str, ".dbf");
		if (strcmp(ZoneTable.GetName(), Str))
			{
			ZoneTable.SetName(Str);
			ZoneTable.Instantiate();
			SetZone(0L);
			} // if
		else
			SetZone(ZoneID);
		} // if
	else
		{
		ZoneTable.SetName("");
		ZoneTable.FreeAll();
		ZoneName[0] = 0;
		ZoneID = 0;
		} // else
	} // if
else
	{
	// set it to custom
	ProjSysID = 0;
	ZoneTable.SetName("");
	ZoneTable.FreeAll();
	ZoneName[0] = 0;
	ZoneID = 0;
	} // else

} // CoordSys::SetSystem

/*===========================================================================*/

void CoordSys::SetZone(char *NewName)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldStr("NAME", NewName)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetZone

/*===========================================================================*/

void CoordSys::SetZoneByCode(long NewCode)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldLong("CODE", NewCode)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetZoneByCode

/*===========================================================================*/

void CoordSys::SetZoneByShort(char *Shorthand)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldStr("SHORT", Shorthand)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetZoneByShort
/*===========================================================================*/

void CoordSys::SetStatePlaneZoneByERMName(char *ERMZoneName)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldStr("ERMZONE", ERMZoneName)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetStatePlaneZoneByFIPS

/*===========================================================================*/

void CoordSys::SetStatePlaneZoneByFIPS(long FIPSZone)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldLong("FIPS", FIPSZone)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetStatePlaneZoneByFIPS

/*===========================================================================*/

void CoordSys::SetStatePlaneZoneByEPSG27(long NAD27SPZone)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldLong("EPSG", NAD27SPZone)) < 0)
	NewID = 0;

SetZone(NewID);

} // CoordSys::SetZoneByShort

/*===========================================================================*/

void CoordSys::SetZone(long NewID)
{
double NewParamValue;
char *it;
int i;
long NewParamID, NewMethodID, NewDatumID, ParamTagCt, ParamCt;
char newProjSysName[WCS_EFFECT_MAXNAMELENGTH], Str[256], ParamTag[32], ParamValTag[32], tempStr[32];

if (ZoneTable.FetchFieldValueStr(NewID, "NAME", Str, 256))
	{
	strncpy(ZoneName, Str, WCS_EFFECT_MAXNAMELENGTH);
	ZoneName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	ZoneID = NewID;
	if (ZoneTable.FetchFieldValueLong(NewID, "METHOD", NewMethodID))
		{
		if ((NewMethodID = Method.MethodTable.FindRecordByFieldLong("CODE", NewMethodID)) >= 0)
			Method.SetMethod(NewMethodID);
		} // if
	if (ZoneTable.FetchFieldValueLong(NewID, "DATUM", NewDatumID))
		{
		if ((NewDatumID = Datum.DatmTable.FindRecordByFieldLong("CODE", NewDatumID)) >= 0)
			Datum.SetDatum(NewDatumID);
		} // if
	for (ParamTagCt = 0; ParamTagCt < 7; ParamTagCt ++)
		{
		sprintf(ParamTag, "PARAM%d", ParamTagCt + 1);
		sprintf(ParamValTag, "P%dVALUE", ParamTagCt + 1);
		if (ZoneTable.FetchFieldValueLong(NewID, ParamTag, NewParamID))
			{
			if (ZoneTable.FetchFieldValueDbl(NewID, ParamValTag, NewParamValue))
				{
				for (ParamCt = 0; ParamCt < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamCt ++)
					{
					if (Method.ParamID[ParamCt] >= 0 && Method.ParamID[ParamCt] == NewParamID)
						{
						Method.AnimPar[ParamCt].SetValue(NewParamValue);
						} // if
					} // for
				} // if
			} // if
		} // for
	// Change the Projection System Name to include the zone number
	if (!strnicmp(ProjSysName, "UTM", 3))
		{
		strcpy(newProjSysName, "UTM ");
		i = (int)strcspn(ZoneName, " ");	// get zone number & southern hemisphere indicator only
		strncpy(tempStr, ZoneName, i);
		tempStr[i] = 0;
		strcat(newProjSysName, tempStr);
		strcat(newProjSysName, " ");
		if (it = strchr(ProjSysName, '-'))	// if we find a string like "- NAD 83", append it
			strcat(newProjSysName, it);
		strncpy(ProjSysName, newProjSysName, WCS_EFFECT_MAXNAMELENGTH);
		ProjSysName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
		if (GlobalApp && GlobalApp->AppEffects)
			SetUniqueName(GlobalApp->AppEffects, ProjSysName);
		} // if
	} // if
else
	{
	ZoneName[0] = 0;
	ZoneID = 0;
	} // else

} // CoordSys::SetZone

/*===========================================================================*/

long CoordSys::IdentifyProjSysIDFromName(char *ProjSysName)
{
long NewID;

if ((NewID = ProjSysTable.FindRecordByFieldStr("NAME", ProjSysName)) >= 0)
	return (NewID);

return (0);

} // CoordSys::IdentifyProjSysIDFromName

/*===========================================================================*/

void CoordSys::SetZoneFile(void)
{
char Str[256];

if (ProjSysTable.FetchFieldValueStr(ProjSysID, "ZONE_FILE", Str, 251))
	{
	strcat(Str, ".dbf");
	ZoneTable.SetName(Str);
	ZoneTable.FreeAll();
	} // if

} // CoordSys::SetZoneFile

/*===========================================================================*/

long CoordSys::IdentifyZoneIDFromName(char *ZoneName)
{
long NewID;

if ((NewID = ZoneTable.FindRecordByFieldStr("NAME", ZoneName)) >= 0)
	return (NewID);

return (0);

} // CoordSys::IdentifyZoneIDFromName

/*===========================================================================*/

void CoordSys::NewItemCallBack(long NewID, char ItemType)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;

switch (ItemType)
	{
	case 0:
		{
		SetSystem(NewID);
		break;
		} // projection system
	case 1:
		{
		SetZone(NewID);
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
		break;
		} // projection zone
	case 2:
		{
		Method.SetMethod(NewID);
		break;
		} // projection method
	case 3:
		{
		Datum.SetDatum(NewID);
		break;
		} // geodetic datum
	case 4:
		{
		Datum.Ellipse.SetEllipsoid(NewID);
		break;
		} // ellipsoid
	} // switch

if (GlobalApp->GUIWins->COS && GlobalApp->GUIWins->COS->GetActive() == this)
	GlobalApp->GUIWins->COS->FillZoneDrop();
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // CoordSys::NewItemCallBack

/*===========================================================================*/

void CoordSys::UpdateJoeBounds(void)
{
JoeList *CurJoe;

if (CurJoe = Joes)
	{
	while (CurJoe)
		{
		if (CurJoe->Me)
			{
			CurJoe->Me->ZeroUpTree();
			} // if
		CurJoe = CurJoe->Next;
		} // while
	CurJoe = Joes;
	while (CurJoe)
		{
		if (CurJoe->Me)
			{
			CurJoe->Me->RecheckBounds();
			} // if
		CurJoe = CurJoe->Next;
		} // while
	CurJoe = Joes;
	while (CurJoe)
		{
		if (CurJoe->Me)
			{
			GlobalApp->AppDB->BoundUpTree(CurJoe->Me);
			} // if
		CurJoe = CurJoe->Next;
		} // while
	} // if

// <<<>>> send notification that vectors have changed

} // CoordSys::UpdateJoeBounds

/*===========================================================================*/

int CoordSys::Initialize(void)
{
double Flattening;
#ifdef WCS_BUILD_VNS
long GeoID;
#endif // WCS_BUILD_VNS

Initialized = 0;

SMajor = fabs(Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue);
SMinor = fabs(Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue);
// our axes are different from ECEF
// AnimPars are in ECEF, Deltas are in WCS axis notation
DeltaX = Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].CurValue;
DeltaY = Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].CurValue;
DeltaZ = -Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].CurValue;
if (SMajor > 0.0 && SMinor > 0.0)
	{
	Flattening = (SMajor - SMinor) / SMajor;
	EccSq = 2.0 * Flattening - Flattening * Flattening;
	EccPrimeSq = (SMajor * SMajor - SMinor * SMinor) / (SMinor * SMinor);
	// if sphere GCTP expects SMinor to be 0
	if (SMajor == SMinor)
		SMinor = 0.0;
	memset(ToidValues, 0, sizeof ToidValues);
	if (InitializeMethod())
		{
		ToidValues[0] = SMajor;
		ToidValues[1] = SMinor;
		for_init(Method.GCTPMethod, 0, ToidValues, -1, NULL, NULL, &ErrorCode);
		inv_init(Method.GCTPMethod, 0, ToidValues, -1, NULL, NULL, &ErrorCode);
#ifdef WCS_BUILD_VNS
		Geographic = 0;
		if ((GeoID = Method.MethodTable.FindRecordByFieldStr("NAME", "Geographic")) >= 0)
			{
			Geographic = Method.MethodID == GeoID;
			} // if
#endif // WCS_BUILD_VNS
		Initialized = 1;
		} // if
	} // if

#ifndef WCS_BUILD_VNS
Geographic = 1;
#endif // WCS_BUILD_VNS

return (Initialized);

} // CoordSys::Initialize

/*===========================================================================*/

int CoordSys::InitializeMethod(void)
{
long ParamCt;

// angles need to be entered as packed DMS so: DDDMMMSSS.SS
// longitudes are negated to correct for our backwardness.
for (ParamCt = 0; (Method.ParamID[ParamCt] >= 0) && (ParamCt < 8); ParamCt ++)
	{
	switch (Method.ParamID[ParamCt])
		{
		case 1:
			{
			StdPar = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat std parallel
		case 2:
			{
			StdPar1 = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat 1st std parallel
		case 3:
			{
			StdPar2 = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat 2nd std parallel
		case 4:
			{
			CentralMeridian = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon central meridian
		case 5:
			{
			OriginLat = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat projection origin
		case 6:
			{
			FalseEast = Method.AnimPar[ParamCt].CurValue;
			break;
			} // False easting
		case 7:
			{
			FalseNorth = Method.AnimPar[ParamCt].CurValue;
			break;
			} // False northing
		case 8:
			{
			TrueScale = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat true scale
		case 9:
			{
			LongPole = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon below pole
		case 10:
			{
			Factor = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Scale factor
		case 11:
			{
			CenterLong = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon center projection
		case 12:
			{
			CenterLat = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat center projection
		case 13:
			{
			Height = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Height perspective point
		case 14:
			{
			Long1 = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon 1st point
		case 15:
			{
			Long2 = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon 2nd point
		case 16:
			{
			Lat1 = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat 1st point
		case 17:
			{
			Lat2 = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lat 2nd point
		case 18:
			{
			AzimuthAngle = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Azimuth angle
		case 19:
			{
			AzimuthPoint = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon azimuth point
		case 20:
			{
			InclinationAngle = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Inclination of orbit
		case 21:
			{
			AscendingLong = -DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Lon ascending orbit
		case 22:
			{
			PeriodSatRevolution = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Period of revolution
		case 23:
			{
			LandsatRatio = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Landsat norhern ratio
		case 24:
			{
			PathFlag = Method.AnimPar[ParamCt].CurValue;
			break;
			} // End of path flag
		case 25:
			{
			SatelliteNum = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Landsat satellite number
		case 26:
			{
			PathNum = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Landsat path number
		case 27:
			{
			ShapeParamM = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Shape parameter m
		case 28:
			{
			ShapeParamN = Method.AnimPar[ParamCt].CurValue;
			break;
			} // Shape parameter n
		case 29:
			{
			Angle = DecimalToPackedDMS(Method.AnimPar[ParamCt].CurValue);
			break;
			} // Rotation angle
		default:
			{
			return (0);
			} // unknown parameter
		} // switch
	} // for

// set values into 15 parameter array. positions of each are defined by GCTP
switch (Method.GCTPMethod)
	{
	case 0:
		{
		// nothing to init
		break;
		} // Geographic
	case 1:
		{
		return (0);
		} // UTM
	case 2:
		{
		return (0);
		} // State Plane
	case 3:
		{
		ToidValues[2] = StdPar1;
		ToidValues[3] = StdPar2;
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Albers Conical Equal Area
	case 4:
		{
		ToidValues[2] = StdPar1;
		ToidValues[3] = StdPar2;
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Lambert Conformal Conic
	case 5:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = TrueScale;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Mercator
	case 6:
		{
		ToidValues[4] = LongPole;
		ToidValues[5] = TrueScale;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Polar Stereographic
	case 7:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Polyconic
	case 8:
		{
		// has two variations
		if (! stricmp(Method.MethodName, "Equidistant Conic, A"))
			{
			ToidValues[2] = StdPar;
			ToidValues[8] = 0.0;
			} // if
		else
			{
			ToidValues[2] = StdPar1;
			ToidValues[3] = StdPar2;
			ToidValues[8] = 1.0;
			} // else
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Equidistant Conic, A, B
	case 9:
		{
		ToidValues[2] = Factor;
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Transverse Mercator
	case 10:
		{
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Stereographic
	case 11:
		{
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Lambert Azimuthal Equal Area
	case 12:
		{
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Azimuthal Equidistant
	case 13:
		{
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Gnomonic
	case 14:
		{
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Orthographic
	case 15:
		{
		ToidValues[2] = Height;
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // General Vertical Near-side Perspective
	case 16:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Sinusoidal
	case 17:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = TrueScale;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Equirectangular
	case 18:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Miller Cylindrical
	case 19:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[5] = OriginLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Van der Grinten
	case 20:
		{
		// has two variations
		if (! stricmp(Method.MethodName, "Hotine Oblique Mercator, A"))
			{
			ToidValues[2] = Factor;
			ToidValues[5] = OriginLat;
			ToidValues[6] = FalseEast;
			ToidValues[7] = FalseNorth;
			ToidValues[8] = Long1;
			ToidValues[9] = Lat1;
			ToidValues[10] = Long2;
			ToidValues[11] = Lat2;
			ToidValues[12] = 0.0;
			} // if
		else
			{
			ToidValues[2] = Factor;
			ToidValues[3] = AzimuthAngle;
			ToidValues[4] = AzimuthPoint;
			ToidValues[5] = OriginLat;
			ToidValues[6] = FalseEast;
			ToidValues[7] = FalseNorth;
			ToidValues[12] = 1.0;
			} // else
		break;
		} // Hotine Oblique Mercator, A, B
	case 21:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Robinson
	case 22:
		{
		// has two variations
		if (! stricmp(Method.MethodName, "Space Oblique Mercator, A"))
			{
			ToidValues[3] = InclinationAngle;
			ToidValues[4] = AscendingLong;
			ToidValues[6] = FalseEast;
			ToidValues[7] = FalseNorth;
			ToidValues[8] = PeriodSatRevolution;
			ToidValues[9] = LandsatRatio;
			ToidValues[10] = PathFlag;
			ToidValues[12] = 0.0;
			} // if
		else
			{
			ToidValues[2] = SatelliteNum;
			ToidValues[3] = PathNum;
			ToidValues[6] = FalseEast;
			ToidValues[7] = FalseNorth;
			ToidValues[12] = 1.0;
			} // else
		break;
		} // Space Oblique Mercator, A, B
	case 23:
		{
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Modified Stereographic Conformal
	case 24:
		{
		// nothing to init
		break;
		} // Interrupted Goode Homolosine
	case 25:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Mollweide
	case 26:
		{
		// nothing to init
		break;
		} // Interrupted Mollweide
	case 27:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Hammer
	case 28:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Wagner IV
	case 29:
		{
		ToidValues[4] = CentralMeridian;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		break;
		} // Wagner VII
	case 30:
		{
		ToidValues[2] = ShapeParamM;
		ToidValues[3] = ShapeParamN;
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		ToidValues[8] = Angle;
		break;
		} // Oblated Equal Area
	case 31:
		{
		// Nothing to do on this one
		break;
		} // New Zealand Map Grid
	case 32:
		{
		// Nothing to do on this one
		break;
		} // RD / Netherlands New
	case 33:
		{
		/***
		ToidValues[2] = StdPar;
		ToidValues[3] = Factor;
		ToidValues[4] = CenterLong;
		ToidValues[5] = CenterLat;
		ToidValues[6] = FalseEast;
		ToidValues[7] = FalseNorth;
		ToidValues[8] = AzimuthAngle;
		***/
		ToidValues[2] = 78030000.0;
		ToidValues[3] = 0.9999;
		ToidValues[4] = 42030000.0;
		ToidValues[5] = 49030000.0;
		ToidValues[6] = 0.0;
		ToidValues[7] = 0.0;
		ToidValues[8] = 30017017.3031;
		break;
		} // Krovak Oblique Conformal Conic
	default:
		{
		return (0);
		} // unknown method
	} // switch

return (1);

} // CoordSys::InitializeMethod

/*===========================================================================*/

int CoordSys::ProjToCart(VertexDEM *Vert)
{

if (! Initialized)
	Initialize();
if (Initialized)
	{
	if (ProjToDeg(Vert))
		{
		if (DegToCart(Vert))
			return (1);
		} // if
	} // if

return (0);

} // CoordSys::ProjToCart

/*===========================================================================*/

int CoordSys::ProjToDeg(VertexDEM *Vert)
{

if (! Initialized)
	Initialize();
if (Initialized)
	{
	if (Method.GCTPMethod)
		{
		// return value of non-zero indicates failure to transform
		if (CoordsInverse(Method.GCTPMethod, Vert->xyz[0], Vert->xyz[1], &Vert->Lon, &Vert->Lat))
			return (0);
		// convert to our backwards longitude notation and from radians to degrees
		Vert->Lon = -Vert->Lon * (1.0 / PiOver180);
		Vert->Lat = Vert->Lat * (1.0 / PiOver180);
		} // if projection
	else
		{
		Vert->Lon = Vert->xyz[0];
		Vert->Lat = Vert->xyz[1];
		} // else geographic
	Vert->Elev = Vert->xyz[2];
	return (1);
	} // if

return (0);

} // CoordSys::ProjToDeg

/*===========================================================================*/

// for conversion of ECEF to WCS axes:
// WCS   ECEF
//  X     Y
//  Y     Z
//  Z    -X

// for conversion of WCS to ECEF axes:
// ECEF  WCS
//  X    -Z
//  Y     X
//  Z     Y

//unsigned long PhiCacheHits, PhiCacheMisses, DoubleHit, LamCacheHits, LamCacheMisses;

int CoordSys::DegToCart(VertexDEM *Vert)
{
#ifdef WCS_BUILD_VNS
double Radius, Rsq, Rxz, Ysq, TempLon;
double Phi, Lambda, NofPhi, SinSqPhi, CosPhi, SinPhi, CosLambda, SinLambda;
int rVal = 0;

if (! Initialized)
	Initialize();
if (Initialized)
	{
	rVal = 1;
	if (SMinor != 0.0)
		{
		// from Dana 1996
		// we can't cache anything relying on the Elev or the EccSq values, as they could vary from call to call
		// we only match the independent Lat-to-Phi and Lon-to-Lambda conversions
		#pragma omp critical (OMP_CRITICAL_COORDCONVERT)
		{
		if (Vert->Lat == CachedLat)
			{
			//if (Vert->Lon == CachedLon) DoubleHit++;
			//PhiCacheHits++;
			SinPhi = CachedSinPhi;
			CosPhi = CachedCosPhi;
			} // if
		else
			{
			CachedLat = Vert->Lat;
			Phi = Vert->Lat * PiOver180;
			//PhiCacheMisses++;
			sincos(Phi, &SinPhi, &CosPhi);
			CachedSinPhi = SinPhi;
			CachedCosPhi = CosPhi;
			} // else
		if (Vert->Lon == CachedLon)
			{
			//LamCacheHits++;
			SinLambda = CachedSinLam;
			CosLambda = CachedCosLam;
			} // if
		else
			{
			CachedLon = Vert->Lon; // stored in cache non-negated, though cached result is result of negation
			Lambda = -Vert->Lon * PiOver180;
			//LamCacheMisses++;
			sincos(Lambda, &SinLambda, &CosLambda);
			CachedSinLam = SinLambda;
			CachedCosLam = CosLambda;
			} // else
		} // end omp critical
		SinSqPhi = SinPhi * SinPhi;
		// The following line would be an exception if EccSq * SinSqPhi were >= 1
		// For this to happen EccSq would need to be 1 and SinPhi would need to be one
		// That would only happen if semi-minor axis were 0 and latitude were 0
		// Since axes cannot recede to 0 this will never result in divide by 0
		NofPhi = SMajor / sqrt(1.0 - EccSq * SinSqPhi);
		// store results in WCS axis notation
		Vert->XYZ[0] = (NofPhi + Vert->Elev) * CosPhi * SinLambda;		// ECEF Y
		Vert->XYZ[1] = (NofPhi * (1.0 - EccSq) + Vert->Elev) * SinPhi;	// ECEF Z
		Vert->XYZ[2] = -(NofPhi + Vert->Elev) * CosPhi * CosLambda;		// ECEF -X
		} // if ellipsoid
	else
		{
		TempLon = Vert->Lon;
		Radius = SMajor + Vert->Elev;
		Vert->XYZ[1] = Radius * sin(Vert->Lat * PiOver180);
		Rxz = (Rsq = Radius * Radius) - (Ysq = Vert->XYZ[1] * Vert->XYZ[1]);
		Rxz = (Rxz <= 0.0 ? 0.0 : sqrt(Rxz));
		Vert->XYZ[0] = Rxz * sin(-Vert->Lon * PiOver180);
		Rsq = Rsq - Ysq - Vert->XYZ[0] * Vert->XYZ[0];
		Vert->XYZ[2] = (Rsq <= 0.0 ? 0.0 : sqrt(Rsq));

		if (fabs(TempLon) > 180.0)
			{
			TempLon += 180;
			if (fabs(TempLon) >= 360.0)
				TempLon = fmod(TempLon, 360.0);	// retains the original sign
			if (TempLon < 0.0)
				TempLon += 360.0;
			TempLon -= 180.0;
			} // if
		// replaced by above
		//while (TempLon > 180.0)
		//	TempLon -= 360.0;
		//while (TempLon < -180)
		//	TempLon += 360.0;
		if (fabs(TempLon) < 90.0)
			Vert->XYZ[2] = -Vert->XYZ[2];
		} // else sphere

	// apply positive Deltas. Remember that Deltas have been converted to WCS space in the Init function
	Vert->XYZ[0] += DeltaX;
	Vert->XYZ[1] += DeltaY;
	Vert->XYZ[2] += DeltaZ;
	} // if

return (rVal);
#else // WCS_BUILD_VNS

Vert->DegToCart(GlobalApp->AppEffects->GetPlanetRadius());

return (1);
#endif // WCS_BUILD_VNS
} // CoordSys::DegToCart

/*===========================================================================*/

int CoordSys::CartToProj(VertexDEM *Vert)
{

if (! Initialized)
	Initialize();
if (Initialized)
	{
	if (CartToDeg(Vert))
		{
		if (DegToProj(Vert))
			return (1);
		} // if
	} // if

return (0);

} // CoordSys::CartToProj

/*===========================================================================*/

int CoordSys::CartToDeg(VertexDEM *Vert)
{
#ifdef WCS_BUILD_VNS
double Radius, RadiusSq, Ysq, RxzSq;
double Phi, Lambda, Height, NofPhi, SinPhi, CosPhi, SinSqPhi, X, Y, Z, TempX, TempY, TempZ, p, Theta, SinCubedTheta, CosCubedTheta;
int rVal = 0;

if (! Initialized)
	Initialize();
if (Initialized)
	{
	TempX = Vert->XYZ[0] - DeltaX;
	TempY = Vert->XYZ[1] - DeltaY;
	TempZ = Vert->XYZ[2] - DeltaZ;
	rVal = 1;
	if (SMinor != 0.0)
		{
		// convert coords to ECEF
		X = -TempZ;
		Y = TempX;
		Z = TempY;
		// from Dana 1996
		if (X != 0.0 || Y != 0.0)
			{
			p = sqrt(X * X + Y * Y);
			Theta = atan(Z * SMajor / (p * SMinor));
			/***
			SinCubedTheta = sin(Theta);
			SinCubedTheta = SinCubedTheta * SinCubedTheta * SinCubedTheta;
			CosCubedTheta = cos(Theta);
			CosCubedTheta = CosCubedTheta * CosCubedTheta * CosCubedTheta;
			***/
			sincos(Theta, &SinCubedTheta, &CosCubedTheta);
			//SinCubedTheta = SinCubedTheta * SinCubedTheta * SinCubedTheta;
			//CosCubedTheta = CosCubedTheta * CosCubedTheta * CosCubedTheta;
			Phi = atan((Z + EccPrimeSq * SMinor * SinCubedTheta * SinCubedTheta * SinCubedTheta) /
				(p - EccSq * SMajor * CosCubedTheta * CosCubedTheta * CosCubedTheta));
			Lambda = atan2(Y, X);
			sincos(Phi, &SinPhi, &CosPhi);
			SinSqPhi = SinPhi * SinPhi;
			NofPhi = SMajor / sqrt(1.0 - EccSq * SinSqPhi);
			Height = (p / CosPhi) - NofPhi;
			Vert->Lat = Phi * PiUnder180;
			// this changes the sign to WCS convention
			Vert->Lon = -Lambda * PiUnder180;
			Vert->Elev = Height;
			} // if not on polar axis
		else
			{
			if (Z > 0.0)
				Vert->Lat = 90.0;
			else if (Z < 0.0)
				Vert->Lat = -90.0;
			else
				Vert->Lat = 0.0;
			Vert->Lon = 0.0;
			Vert->Elev = fabs(Z) - SMinor;
			} // else point is on polar axis
		} // if an ellipsoid
	else
		{
		RadiusSq = (TempX * TempX + (Ysq = TempY * TempY) + TempZ * TempZ);
		Radius = sqrt(RadiusSq);
		if (Radius > 0.0)
			{
			Vert->Lat = asin(TempY / Radius) * PiUnder180;
			RxzSq = (RadiusSq - Ysq);
			if (RxzSq != 0.0)
				{
				Vert->Lon = -asin(TempX / sqrt(RxzSq)) * PiUnder180;
				if (TempZ > 0.0)
					Vert->Lon = 180.0 - Vert->Lon;
				} // if
			else
				Vert->Lon = 0.0;
			} // if
		else
			{
			Vert->Lat = 0.0;
			Vert->Lon = 0.0;
			} // else

		Vert->Elev = Radius - SMajor;
		} // else sphere
	} // if

return (rVal);
#else // WCS_BUILD_VNS

Vert->CartToDeg(GlobalApp->AppEffects->GetPlanetRadius());

return (1);
#endif // WCS_BUILD_VNS
} // CoordSys::CartToDeg

/*===========================================================================*/

int CoordSys::DegToProj(VertexDEM *Vert)
{
double TransformX, TransformY;

if (! Initialized)
	Initialize();
if (Initialized)
	{
	if (Method.GCTPMethod)
		{
		// return value of non-zero indicates failure to transform
		if (CoordsForward(Method.GCTPMethod, -Vert->Lon * PiOver180, Vert->Lat * PiOver180, &TransformX, &TransformY))
			return (0);
		Vert->xyz[0] = TransformX;
		Vert->xyz[1] = TransformY;
		} // if projection
	else
		{
		Vert->xyz[0] = Vert->Lon;
		Vert->xyz[1] = Vert->Lat;
		} // else geographic
	Vert->xyz[2] = Vert->Elev;
	return (1);
	} // if

return (0);

} // CoordSys::DegToProj

/*===========================================================================*/

int CoordSys::ProjToDefDeg(VertexDEM *Vert)
{
double StashElev;

StashElev = Vert->xyz[2];
if (ProjToCart(Vert))
	{
	if (GlobalApp->AppEffects->FetchDefaultCoordSys()->CartToDeg(Vert))
		{
		//#ifndef WCS_ENABLE_COORDSCALC
		Vert->Elev = StashElev;
		//#endif // WCS_ENABLE_COORDSCALC
		return (1);
		} // if
	} // if

return (0);

} // CoordSys::ProjToDefDeg

/*===========================================================================*/

int CoordSys::DefDegToProj(VertexDEM *Vert)
{
double StashElev;

StashElev = Vert->Elev;
if (GlobalApp->AppEffects->FetchDefaultCoordSys()->DegToCart(Vert))
	{
	if (CartToProj(Vert))
		{
		//#ifndef WCS_ENABLE_COORDSCALC
		Vert->xyz[2] = Vert->Elev = StashElev;
		//#endif // WCS_ENABLE_COORDSCALC
		return (1);
		} // if
	} // if

return (0);

} // CoordSys::DefDegToProj

/*===========================================================================*/

int CoordSys::DatumDeltaCalculator(void)
{
double ThisXYZ[3], ECEF[3];
VertexDEM Vert;
CoordSys WGS84;
NotifyTag Changes[2];
char Str[128];

// get latitude reference
sprintf(Str, "%f", GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
if (GetInputString("Enter latitude of reference point in decimal degrees.", WCS_REQUESTER_NUMERIC_ONLY, Str) && Str[0])
	{
	Vert.Lat = atof(Str);
	// get longitude reference
	sprintf(Str, "%f", -GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	if (GetInputString("Enter longitude of reference point in decimal degrees with West longitude negative, Ease longitude positive.", WCS_REQUESTER_NUMERIC_ONLY, Str) && Str[0])
		{
		Vert.Lon = -atof(Str);

		// create a WGS 84 coord sys
		WGS84.SetSystem("Geographic - WGS 84");

		// DegToCart the reference point with this coord sys, remove deltas applied in DegToCart
		DegToCart(&Vert);
		ThisXYZ[0] = Vert.XYZ[0] - DeltaX;
		ThisXYZ[1] = Vert.XYZ[1] - DeltaY;
		ThisXYZ[2] = Vert.XYZ[2] - DeltaZ;

		// DegToCart the reference point with this WGS 84
		WGS84.DegToCart(&Vert);

		// subtract to obtain deltas, convert from WCS to ECEF axis notation
		ECEF[0] = -(Vert.XYZ[2] - ThisXYZ[2]);
		ECEF[1] = (Vert.XYZ[0] - ThisXYZ[0]);
		ECEF[2] = (Vert.XYZ[1] - ThisXYZ[1]);

		// store delta values
		Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX].SetValue(ECEF[0]);
		Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY].SetValue(ECEF[1]);
		Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ].SetValue(ECEF[2]);
		Initialized = 0;

		// send notification
		Changes[0] = MAKE_ID(GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		return (1);
		} // if lon
	} // if lat

return (0);

} // CoordSys::DatumDeltaCalculator

/*===========================================================================*/

const char *CoordSys::GetXUnitName(void)
{

return (Method.GCTPMethod ? "Easting ": "Longitude ");

} // CoordSys::GetXUnitName

/*===========================================================================*/

const char *CoordSys::GetYUnitName(void)
{

return (Method.GCTPMethod ? "Northing ": "Latitude ");

} // CoordSys::GetYUnitName

/*===========================================================================*/

ULONG CoordSys::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, int LinkImages)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID = 0;
ImageList **CurImage = &Images;
RasterAttribute *MyAttr;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_COORDSYS_PROJSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)ProjSysName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						ProjSysID = IdentifyProjSysIDFromName(ProjSysName);
						SetZoneFile();
						break;
						}
					case WCS_EFFECTS_COORDSYS_ZONENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)ZoneName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						ZoneID = IdentifyZoneIDFromName(ZoneName);
						break;
						}
					case WCS_EFFECTS_COORDSYS_PROJMETHOD:
						{
						BytesRead = Method.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_COORDSYS_GEODATUM:
						{
						BytesRead = Datum.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (LinkImages)
							{
							if (ImageID > 0)
								{
								if (*CurImage = new ImageList())
									{
									if (MyAttr = GlobalApp->LoadToImageLib->MatchRasterAttributeSetHost(ImageID, WCS_RASTERSHELL_TYPE_GEOREF, this))
										{
										(*CurImage)->Me = (GeoRefShell *)MyAttr->GetShell();
										CurImage = &(*CurImage)->Next;
										} // if
									else
										{
										delete (*CurImage);
										*CurImage = NULL;
										} // else
									} // if
								} // if
							} // if
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // CoordSys::Load

/*===========================================================================*/

unsigned long CoordSys::Save(FILE *ffile, int SaveImages)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long ImageID;
ImageList *CurImage;

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_COORDSYS_PROJSYSNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ProjSysName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)ProjSysName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_COORDSYS_ZONENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ZoneName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)ZoneName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_COORDSYS_GEODATUM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Datum.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if geo datum saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_COORDSYS_PROJMETHOD + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Method.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if geo datum saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (SaveImages)
	{
	CurImage = Images;
	while (CurImage)
		{
		if (CurImage->Me && (ImageID = CurImage->Me->GetRasterID()) > 0)
			{
			if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_IMAGEID, WCS_BLOCKSIZE_CHAR,
				WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
				WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
				goto WriteError;
			TotalWritten += BytesWritten;
			} // if
		CurImage = CurImage->Next;
		} // while
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // CoordSys::Save

/*===========================================================================*/

void CoordSys::Edit(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->COS)
	{
	delete GlobalApp->GUIWins->COS;
	}
GlobalApp->GUIWins->COS = new CoordSysEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this, 0);
if (GlobalApp->GUIWins->COS)
	{
	GlobalApp->GUIWins->COS->Open(GlobalApp->MainProj);
	}

} // CoordSys::Edit

/*===========================================================================*/

void CoordSys::EditModal(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->COS)
	{
	delete GlobalApp->GUIWins->COS;
	}
GlobalApp->GUIWins->COS = new CoordSysEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this, 1);
if (GlobalApp->GUIWins->COS)
	{
	GlobalApp->GUIWins->COS->Open(GlobalApp->MainProj);
	}

} // CoordSys::EditModal

/*===========================================================================*/

void CoordSys::OpenTableList(char ApplyTo, int GoneModal)
{
GenericTable *Tab;
char *Fields[4], *DataType;
long CurSel;

Fields[0] = "NAME";
Fields[1] = "USAGE";
Fields[2] = Fields[3] = NULL;

switch (ApplyTo)
	{
	case 0:
		{
		CurSel = ProjSysID;
		Tab = &ProjSysTable;
		DataType = "Projection System";
		break;
		} // projection system
	case 1:
		{
		CurSel = ZoneID;
		Tab = &ZoneTable;
		DataType = "Projection Zone";
		break;
		} // projection zone
	case 2:
		{
		CurSel = Method.MethodID;
		Tab = &Method.MethodTable;
		DataType = "Projection Method";
		break;
		} // projection method
	case 3:
		{
		CurSel = Datum.DatumID;
		Tab = &Datum.DatmTable;
		Fields[1] = "AREA_USE";
		Fields[2] = "ORIGIN_DES";
		Fields[3] = "REMARKS";
		DataType = "Geodetic Datum";
		break;
		} // geodetic datum
	case 4:
		{
		CurSel = Datum.Ellipse.EllipsoidID;
		Tab = &Datum.Ellipse.EllipseTable;
		DataType = "Ellipsoid";
		break;
		} // ellipsoid
	} // switch

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->TBG)
	{
	delete GlobalApp->GUIWins->TBG;
	}
GlobalApp->GUIWins->TBG = new TableListGUI(this, Tab, Fields, ApplyTo, DataType, CurSel);
if (GlobalApp->GUIWins->TBG)
	{
	if (GoneModal)
		GlobalApp->GUIWins->TBG->SetModal();
	GlobalApp->GUIWins->TBG->Open(GlobalApp->MainProj);
	}

} // CoordSys::OpenTableList

/*===========================================================================*/

void CoordSys::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];
ImageList *PrevImage = NULL, *CurImage = Images;

while (CurImage)
	{
	if (CurImage->Me == Shell)
		{
		CurImage->Me = NULL;
		if (PrevImage)
			PrevImage->Next = CurImage->Next;
		else
			Images = CurImage->Next;
		delete CurImage;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		return;
		} // if
	PrevImage = CurImage;
	CurImage = CurImage->Next;
	} // while

} // CoordSys::RemoveRaster

/*===========================================================================*/

ImageList *CoordSys::AddRaster(Raster *AddMe)
{
NotifyTag Changes[2];
ImageList **CurImage = &Images;
RasterAttribute *MyAttr;

if (AddMe)
	{
	while (*CurImage)
		{
		if ((*CurImage)->Me->GetRaster() == AddMe)
			return (NULL);
		CurImage = &(*CurImage)->Next;
		} // while
	if (*CurImage = new ImageList())
		{
		if ((MyAttr = AddMe->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			if (MyAttr->GetShell()->GetHost())
				{
				((CoordSys *)MyAttr->GetShell()->GetHost())->RemoveRaster(MyAttr->GetShell());
				} // if need to remove existing coord sys
			(*CurImage)->Me = (GeoRefShell *)MyAttr->GetShell();
			MyAttr->GetShell()->SetHostNotify(this);
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		else
			{
			delete (*CurImage);
			*CurImage = NULL;
			} // else
		} // if
	return (*CurImage);
	} // if

return (NULL);

} // CoordSys::AddRaster

/*===========================================================================*/

char *CoordSys::GetCritterName(RasterAnimHost *Test)
{


return ("");

} // CoordSys::GetCritterName

/*===========================================================================*/

char CoordSys::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
#ifdef WCS_BUILD_VNS
if (DropType == WCS_RAHOST_OBJTYPE_GEODATUM
	|| DropType == WCS_RAHOST_OBJTYPE_PROJMETHOD
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_CONTROLPT
	|| DropType == WCS_EFFECTSSUBCLASS_PLANETOPT
	|| DropType == WCS_RAHOST_OBJTYPE_DEM)
	return (1);
#endif // WCS_BUILD_VNS

return (0);

} // CoordSys::GetRAHostDropOK

/*===========================================================================*/

int CoordSys::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (CoordSys *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (CoordSys *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_GEODATUM)
	{
	Success = Datum.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_PROJMETHOD)
	{
	Success = Method.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_DEM)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (((Joe *)DropSource->DropSource)->AddEffect(this, -1))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = (int)AddRaster((Raster *)DropSource->DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_PLANETOPT)
	{
	Success = -1;
	sprintf(QueryStr, "Attach %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Success = ((PlanetOpt *)DropSource->DropSource)->SetCoords(this);
		} // if
	} // else if
#endif // WCS_BUILD_VNS
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // CoordSys::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long CoordSys::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_COORDSYS | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // CoordSys::GetRAFlags

/*===========================================================================*/

RasterAnimHost *CoordSys::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
ImageList *CurImage;
JoeList *CurJoe = Joes;

if (! Current)
	return (&Method);
if (Current == &Method)
	return (&Datum);
if (Current == &Datum)
	Found = 1;
CurImage = Images;
while (CurImage)
	{
	if (Found && CurImage->Me && CurImage->Me->GetRaster())
		return (CurImage->Me->GetRaster());
	if (CurImage->Me && Current == CurImage->Me->GetRaster())
		Found = 1;
	CurImage = CurImage->Next;
	} // while
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // CoordSys::GetRAHostChild

/*===========================================================================*/

int CoordSys::RemoveRAHost(RasterAnimHost *RemoveMe)
{
ImageList *CurImage = Images, *PrevImage = NULL;
RasterAttribute *MyAttr;
NotifyTag Changes[2];

while (CurImage)
	{
	if (CurImage->Me && CurImage->Me->GetRaster() == (Raster *)RemoveMe)
		{
		if (PrevImage)
			PrevImage->Next = CurImage->Next;
		else
			Images = CurImage->Next;

		if ((MyAttr = ((Raster *)RemoveMe)->MatchRasterShell(CurImage->Me)) && MyAttr->GetShell())
			{
			CurImage->Me = NULL;
			MyAttr->GetShell()->SetHostNotify(NULL);
			} // if
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		delete CurImage;
		return (1);
		} // if
	PrevImage = CurImage;
	CurImage = CurImage->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // CoordSys::RemoveRAHost

/*===========================================================================*/

int CoordSys::SetToTime(double Time)
{
long Found = 0;

if (Method.SetToTime(Time))
	Found = 1;
if (Datum.SetToTime(Time))
	Found = 1;

if (Found)
	Initialized = 0;
return (Found);

} // CoordSys::SetToTime

/*===========================================================================*/

int CoordSys::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
if (Method.GetRAHostAnimated())
	return (1);
if (Datum.GetRAHostAnimated())
	return (1);

return (0);

} // CoordSys::GetRAHostAnimated

/*===========================================================================*/

long CoordSys::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
if (Datum.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Method.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // CoordSys::GetKeyFrameRange

/*===========================================================================*/

int CoordSys::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{

Geographic = (char)GetGeographic();

return (1);

} // CoordSys::InitToRender

/*===========================================================================*/

int CoordSys::GetGeographic(void)
{
int IsGeographic;
#ifdef WCS_BUILD_VNS

if (! Initialized)
	Initialize();

IsGeographic = Geographic;

#else // WCS_BUILD_VNS
IsGeographic = 1;
#endif // WCS_BUILD_VNS

return (IsGeographic);

} // CoordSys::GetGeographic

/*===========================================================================*/

double CoordSys::FetchPlanetRadius(double Latitude)
{
#ifdef WCS_BUILD_VNS
double Radius, SinPhi;

if (! Initialized)
	Initialize();
if (Initialized)
	{
	if (SMinor != 0.0)
		{
		// from Dana 1996
		SinPhi = sin(Latitude * PiOver180);
		// The following line would be an exception if EccSq * SinSqPhi were >= 1
		// For this to happen EccSq would need to be 1 and SinPhi would need to be one
		// That would only happen if semi-minor axis were 0 and latitude were 0
		// Since axes cannot recede to 0 this will never result in divide by 0
		Radius = SMajor / sqrt(1.0 - EccSq * SinPhi * SinPhi);
		} // if ellipsoid
	else
		{
		Radius = SMajor;
		} // else sphere

	return (Radius);
	} // if

#endif // WCS_BUILD_VNS

return (GlobalApp->AppEffects->GetPlanetRadius());

} // CoordSys::FetchPlanetRadius

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int CoordSys::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
CoordSys *CurrentCS = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if material
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCS = new CoordSys(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentCS->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if 3d object
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentCS)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentCS);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // CoordSys::LoadObject

/*===========================================================================*/

int CoordSys::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

// CoordSys
strcpy(StrBuf, "CoordSys");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if CoordSys saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // CoordSys::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::CoordSys_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
CoordSys *Current;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTSBASE_DEFAULTCOORDS:
						{
						if (DefaultCoords || (DefaultCoords = new CoordSys()))
							{
							BytesRead = DefaultCoords->Load(ffile, Size, ByteFlip);
							}
						break;
						}
					#endif // WCS_BUILD_VNS
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new CoordSys(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (CoordSys *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // EffectsLib::CoordSys_Load()

/*===========================================================================*/

ULONG EffectsLib::CoordSys_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
CoordSys *Current;

#ifdef WCS_BUILD_VNS
if (DefaultCoords)
	{
	ItemTag = WCS_EFFECTSBASE_DEFAULTCOORDS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = DefaultCoords->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if DefaultCoords saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if
#endif // WCS_BUILD_VNS

Current = Coord;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if CoordSys saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (CoordSys *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::CoordSys_Save()

/*===========================================================================*/

CSLoadInfo *CoordSys::LoadFromArcPrj(char *FilePath)
{
CSLoadInfo *Result = NULL;
FILE *fHandle = NULL;

if (fHandle = PROJ_fopen(FilePath, "r"))
	{
	Result = LoadFromArcPrj(fHandle);
	fclose(fHandle);
	fHandle = NULL;
	} // if

return(Result);

} // CoordSys::LoadFromArcPrj

/*===========================================================================*/

// support function to invert longitude where necessary during assign
void AssignValueViaMetricType(AnimDoubleTime *DestADT, double NewVal)
{
if (DestADT)
	{
	if (DestADT->MetricType == WCS_ANIMDOUBLE_METRIC_LONGITUDE)
		{
		DestADT->CurValue = -NewVal;
		} // if
	else
		{
		DestADT->CurValue = NewVal;
		} // else
	}
} // AssignValueViaMetricType

/*===========================================================================*/

// TRUE = parsed ok/no match, keep going; FALSE = abort parsing
// Nothing is done with any info contained in this block
short CoordSys::WKT_Authority(char **first)
{
char *last;

// handle optional <authority>
if (strnicmp(",AUTHORITY", *first, 10) == 0)
	{
	if ((*(*first + 10) != '[') && (*(*first + 10) != '(') && (*(*first + 11) != '"'))
		return FALSE;
	last = *first + 12;
	do
		{
		last++;
		} while ((*last != 0) && (*last != ']') && (*last != ')'));
	if ((*last != ']') && (*last != ')'))
		return FALSE;
	last++;	// finish <authority>
	*first = last;
	} // <authority>

return TRUE;

} // CoordSys::WKT_Authority

/*===========================================================================*/

int CoordSys::WriteWKTParam(FILE *fHandle, double Value, COORDSYS_IDENTS Param)
{
int status;
char text[32];

switch (Param)
	{
	case COORDSYS_AZIMUTH:
		strcpy(text, "azimuth");
		break;
	case COORDSYS_CENTER_LAT:
		strcpy(text, "latitude_of_center");
		break;
	case COORDSYS_CENTER_LON:
		strcpy(text, "longitude_of_center");
		break;
	case COORDSYS_CENTRAL_MERIDIAN:
		strcpy(text, "central_meridian");
		break;
	case COORDSYS_FALSE_EASTING:
		strcpy(text, "false_easting");
		break;
	case COORDSYS_FALSE_NORTHING:
		strcpy(text, "false_northing");
		break;
	case COORDSYS_ORIGIN_LAT:
		strcpy(text, "latitude_of_origin");
		break;
	case COORDSYS_ORIGIN_LON:
		strcpy(text, "longitude_of_origin");
		break;
	case COORDSYS_POINT1_LAT:
		strcpy(text, "latitude_of_point_1");
		break;
	case COORDSYS_POINT1_LON:
		strcpy(text, "longitude_of_point_1");
		break;
	case COORDSYS_POINT2_LAT:
		strcpy(text, "latitude_of_point_2");
		break;
	case COORDSYS_POINT2_LON:
		strcpy(text, "longitude_of_point_2");
		break;
	case COORDSYS_POINT3_LAT:
		strcpy(text, "latitude_of_point_3");
		break;
	case COORDSYS_POINT3_LON:
		strcpy(text, "longitude_of_point_3");
		break;
	case COORDSYS_RECTIFIED_GRID_ANGLE:
		strcpy(text, "rectified_grid_angle");
		break;
	case COORDSYS_SCALE_FACTOR:
		strcpy(text, "scale_factor");
		break;
	case COORDSYS_STD_PARALLEL1:
		strcpy(text, "standard_parallel_1");
		break;
	case COORDSYS_STD_PARALLEL2:
		strcpy(text, "standard_parallel_2");
		break;
	} // switch

status = fprintf(fHandle, "PARAMETER[\"%s\",%f],", text, Value);

return status;

} // CoordSys::WriteWKTParam

/*===========================================================================*/

int CoordSys::SaveToArcPrj(FILE *fHandle)
{
// currently only writes WKT style
double inv_flat, smajor, sminor;
int status;
char WKTProjSysName[80];

if (this->Method.GCTPMethod != 0)
	{
	// <projected cs>
	fprintf(fHandle, "PROJCS[");	// projected
	// <name>
	fprintf(fHandle, "\"%s\",", this->ProjSysName);
	}

// <geographic cs>
fprintf(fHandle, "GEOGCS[");
// <name>
fprintf(fHandle, "\"%s\",", this->Datum.DatumName);
// <datum>
fprintf(fHandle, "DATUM[");
// <name>
fprintf(fHandle, "\"%s\",", this->Datum.DatumName);
// <spheroid>
fprintf(fHandle, "SPHEROID[");
// <name>, <semi-major axis>, <inverse flattening>
smajor = this->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].GetCurValue();
sminor = this->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetCurValue();
if (smajor != sminor)
	inv_flat = smajor / (smajor - sminor); // 1/f = a/(a-b)
else
	inv_flat = 0.0;	// ???
fprintf(fHandle, "\"%s\",%f,%f]],", this->Datum.Ellipse.EllipsoidName, smajor, inv_flat);
// <prime meridian>
fprintf(fHandle, "PRIMEM[\"Greenwich\", 0],");
// <unit>
status = fprintf(fHandle, "UNIT[\"Degree\",0.0174532925199433]]");

if (this->Method.GCTPMethod == 0)
	return status;	// that's all folks!

// <projection>
fprintf(fHandle, ",PROJECTION[");
// <name> {Strings found in gdal/frmts/gxf/gxf_ogcwkt.c}
switch (this->Method.GCTPMethod)
	{
	case 1:
		/***
		strcpy(WKTProjSysName, "Universal_Transverse_Mercator");
		***/
		break;
	case 2:
		/***
		strcpy(WKTProjSysName, "State_Plane_Coordinates");
		***/
		break;
	case 3:
		strcpy(WKTProjSysName, "Albers_Conic_Equal_Area");
		break;
	case 4:
		// assuming all LCC's are 2SP for now, which is what all US State Planes with this method are
		//strcpy(WKTProjSysName, "Lambert_Conformal_Conic_1SP");
		//strcpy(WKTProjSysName, "Lambert_Conformal_Conic_2SP");
		strcpy(WKTProjSysName, "Lambert_Conformal_Conic");
		break;
	case 5:
		/***
		strcpy(WKTProjSysName, "Mercator_1SP");
		strcpy(WKTProjSysName, "Mercator_2SP");
		***/
		break;
	case 6:
		strcpy(WKTProjSysName, "Polar_Stereographic");
		break;
	case 7:
		strcpy(WKTProjSysName, "Polyconic");
		break;
	case 8:
		strcpy(WKTProjSysName, "Equidistant_Conic");
		break;
	case 9:
		strcpy(WKTProjSysName, "Transverse_Mercator");
		break;
	case 10:
		strcpy(WKTProjSysName, "Stereographic");
		break;
	case 11:
		strcpy(WKTProjSysName, "Lambert_Azimuthal_Equal_Area");
		break;
	case 12:
		strcpy(WKTProjSysName, "Azimuthal_Equidistant");
		break;
	case 13:
		strcpy(WKTProjSysName, "Gnomonic");
		break;
	case 14:
		strcpy(WKTProjSysName, "Orthographic");
		break;
	case 15:
		/***
		strcpy(WKTProjSysName, "General_Vertical_Near-Side_Perspective");
		***/
		break;
	case 16:
		strcpy(WKTProjSysName, "Sinusoidal");
		break;
	case 17:
		strcpy(WKTProjSysName, "Equirectangular");
		break;
	case 18:
		strcpy(WKTProjSysName, "Miller_Cylindrical");
		break;
	case 19:
		strcpy(WKTProjSysName, "VanDerGrinten");
		break;
	case 20:
		strcpy(WKTProjSysName, "Hotine_Oblique_Mercator");
		break;
	case 21:
		strcpy(WKTProjSysName, "Robinson");
		break;
	case 22:
		/***
		strcpy(WKTProjSysName, "Space_Oblique_Mercator");
		***/
		break;
	case 23:
		/***
		strcpy(WKTProjSysName, "Alaska_Conformal");	// Modified Stereographic Conformal
		***/
		break;
	case 24:
		/***
		strcpy(WKTProjSysName, "Interrupted_Goode_Homolosine");
		***/
		break;
	case 25:
		strcpy(WKTProjSysName, "Mollweide");
		break;
	case 26:
		/***
		strcpy(WKTProjSysName, "Interrupted_Mollweide");
		***/
		break;
	case 27:
		/***
		strcpy(WKTProjSysName, "Hammer");
		***/
		break;
	case 28:
		/***
		strcpy(WKTProjSysName, "Wagner_IV");
		***/
		break;
	case 29:
		/***
		strcpy(WKTProjSysName, "Wagner_VII");
		***/
		break;
	case 30:
		/***
		strcpy(WKTProjSysName, "Oblated_Equal_Area");
		***/
		break;
	case 31:
		strcpy(WKTProjSysName, "New_Zealand_Map_Grid");
		break;
	default:
		strcpy(WKTProjSysName, " < unknown > ");
		break;
	} // switch
fprintf(fHandle, "\"%s\"],", WKTProjSysName);
// write all the projection params (note: I'm guessing at most of these, but I'm writing them in the order that GDAL passes params
switch (this->Method.GCTPMethod)
	{
	case 1:
		// UTM
		break;
	case 2:
		// State Plane
		break;
	case 3:
		// Albers Conical Equal Area
		WriteWKTParam(fHandle, this->Method.AnimPar[2].GetCurValue(), COORDSYS_STD_PARALLEL1);
		WriteWKTParam(fHandle, this->Method.AnimPar[3].GetCurValue(), COORDSYS_STD_PARALLEL2);
		//WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_ CenterLat {LatOrig});
		//WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_ CenterLon {CentralMeridian});
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 4:
		// Lambert Conformal Conic
		WriteWKTParam(fHandle, this->Method.AnimPar[0].GetCurValue(), COORDSYS_STD_PARALLEL1);
		WriteWKTParam(fHandle, this->Method.AnimPar[1].GetCurValue(), COORDSYS_STD_PARALLEL2);
		WriteWKTParam(fHandle, -(this->Method.AnimPar[2].GetCurValue()), COORDSYS_CENTRAL_MERIDIAN);
		WriteWKTParam(fHandle, this->Method.AnimPar[3].GetCurValue(), COORDSYS_ORIGIN_LAT);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 5:
		// Mercator
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ Scale);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 6:
		// Polar Stereographic
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ Scale);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 7:
		// Polyconic
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat {LatOrig});
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon {CentralMeridian});
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 8:
		// Equidistant Conic
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ StdP1);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ StdP2);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 9:
		// Transverse Mercator
		WriteWKTParam(fHandle, this->Method.AnimPar[2].GetCurValue(), COORDSYS_ORIGIN_LAT);
		WriteWKTParam(fHandle, -(this->Method.AnimPar[1].GetCurValue()), COORDSYS_CENTRAL_MERIDIAN);
		WriteWKTParam(fHandle, this->Method.AnimPar[0].GetCurValue(), COORDSYS_SCALE_FACTOR);
		WriteWKTParam(fHandle, this->Method.AnimPar[3].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 10:
		// Stereographic
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_SCALE_FACTOR);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 11:
		// Lambert Azimuthal Equal Area
		WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_CENTER_LAT);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_CENTER_LON);
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 12:
		// Azimuthal Equidistant
		WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_CENTER_LAT);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_CENTER_LON);
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 13:
		// Gnomonic
		WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_CENTER_LAT);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_CENTER_LON);
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 14:
		// Orthographic
		WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_CENTER_LAT);
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_CENTER_LON);
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 15:
		// General Vertical Near-Side Perspective
		break;
	case 16:
		// Sinusoidal
		//WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_ CenterLon {CentralMeridian});
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 17:
		// Equirectangular
		//WriteWKTParam(fHandle, this->Method.AnimPar[5].GetCurValue(), COORDSYS_ CenterLat {LatTrueScale});
		//WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_ CenterLon {CentralMeridian});
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 18:
		// Miller Cylindrical
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 19:
		// Van der Grinten
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 20:
		// Hotine Oblique Mercator
		/***
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLat);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ CenterLon);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ Azimuth);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_ RectToSkew);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_SCALE_FACTOR);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[].GetCurValue(), COORDSYS_FALSE_NORTHING);
		***/
		break;
	case 21:
		// Robinson
		//WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_ CenterLon {CentralMeridian});
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 22:
		// Space Oblique Mercator
		break;
	case 23:
		// Modified Stereographic Conformal (aka: Alaska Conformal)
		break;
	case 24:
		// Interrupted Goode Homolosine
		break;
	case 25:
		// Mollweide
		WriteWKTParam(fHandle, this->Method.AnimPar[4].GetCurValue(), COORDSYS_CENTRAL_MERIDIAN);
		WriteWKTParam(fHandle, this->Method.AnimPar[6].GetCurValue(), COORDSYS_FALSE_EASTING);
		WriteWKTParam(fHandle, this->Method.AnimPar[7].GetCurValue(), COORDSYS_FALSE_NORTHING);
		break;
	case 26:
		// Interrupted Mollweide
		break;
	case 27:
		// Hammer
		break;
	case 28:
		// Wagner IV
		break;
	case 29:
		// Wagner VII
		break;
	case 30:
		// Oblated Equal Area
		break;
	case 31:
		// New Zealand Map Grid
		break;
	default:
		break;
	} // switch
status = fprintf(fHandle, "UNIT[\"Meter\",1]]\n");

return status;

} // CoordSys::SaveToArcPrj

/*===========================================================================*/

void CoordSys::SetStatePlaneFIPS(const char *str)
{
const char *fipsStr;
unsigned long i;
long fipsNum, zoneLen;
char zoneStr[WCS_EFFECT_MAXNAMELENGTH];

if (fipsStr = strstr(str, "FIPS_"))
	{
	zoneLen = fipsStr - str - 21;
	fipsStr += 5;
	fipsNum = (long)atoi(fipsStr);
	SetStatePlaneZoneByFIPS(fipsNum);
	strncpy(zoneStr, str + 20, zoneLen);
	zoneStr[zoneLen] = 0;
	// change underscores to spaces
	for (i = 0; zoneStr[i]; ++i)
		{
		if (zoneStr[i] == '_')
			zoneStr[i] = ' ';
		} // for
	//SetZone(zoneStr);
	strcpy(ZoneName, zoneStr);	// grrrr....
	} // if

} // CoordSys::SetStatePlaneFIPS

/*===========================================================================*/

CSLoadInfo *CoordSys::LoadFromArcPrj(FILE *fHandle)
{
double ConvFactor, InvFlat, ParamVal, YShift = 0.0;
char *dest, *first, *last;
CSLoadInfo *CSLI = NULL, *rVal = NULL;
long ParamNum, ProjMethodCode = 0, foo;
int Format = -1, FipsZone = -1, IsNAD27 =0, IsNAD83 = 0, IsWGS84 = 0, IsSP = 0, IsUTM = 0, UTMZone = -10000, ReadingParams = 0, KeyType, ParamGet, ParamPut;
char InputLine[2048], tempstr1[256], tempstr2[256], Shorthand[10], ProjCS[256], Projection[256], GeogCS[256], DatumStr[256], SpheroidStr[256], PrimeM[256], ParamStr[64], Unit[256];
char ProjDone = 0, DatumDone = 0, ZUnitsDone = 0, UnitsDone = 0, SpheroidDone = 0, XShiftDone = 0, YShiftDone = 0, ParamDone = 0, parsing_pcs = FALSE;

if (!fHandle) return(NULL);

if (!(CSLI = new CSLoadInfo))
	return(NULL);

// stream through the file
SetSystem(0L); // identify System as custom so as not to mislead
while (!(feof(fHandle)) && fgetline(InputLine, 2000, fHandle, true, true) && (strncmp(InputLine, "EOP", 3) != 0))
	{
	InputLine[2001] = NULL; // ensure no over-runs
	if (Format == -1)
		{ // don't know if we're new single-line or old multi-line
		if (!strnicmp("PROJCS", InputLine, 6))
			{
			Format = 1;
			} // if
		else if (!strnicmp("GEOGCS", InputLine, 6))
			{
			Format = 1;
			} // if
		/*** F2 NOTE: What should be done about Geocentric, Vertical, Compound, Fitted, & Local Coordinate Systems? ***/
		} // if
	if (Format == 1)
		{ // new nested format
		/*** F2 NOTE: ACK! Need to handle close paren as terminator too! ***/
		ProjCS[0] = Projection[0] = DatumStr[0] = SpheroidStr[0] = PrimeM[0] = Unit[0] = 0;
		last = first = &InputLine[0];
		if (strnicmp("PROJCS", first, 6) == 0)
			{
			parsing_pcs = TRUE;
			if ((*(first + 6) != '[') && (InputLine[6] != '(') && (InputLine[7] != '"'))
				break;
			last = first = &InputLine[8];
			while ((*last != 0) && (*last != '"'))
				last++;
			strncpy(ProjCS, first, last - first);
			ProjCS[last - first] = NULL;
			if (*last++ == 0)
				break;
			if (*last++ != ',')
				break;
			first = last;
			if (ProjCS[0])
				{
				//strcpy(ProjSysName, "WKT: ");
				//strcat(ProjSysName, ProjCS);
				strcpy(ProjSysName, ProjCS);
				if (strnicmp("NAD_1927_UTM_Zone_", ProjCS, 18) == 0)
					{
					SetSystemByCode(2);
					SetZoneByShort(&ProjCS[18]);
					} // if
				else if (strnicmp("NAD_1983_UTM_Zone_", ProjCS, 18) == 0)
					{
					SetSystemByCode(3);
					SetZoneByShort(&ProjCS[18]);
					} // if
				else if (strnicmp("NAD_1927_StatePlane_", ProjCS, 20) == 0)
					SetStatePlaneFIPS(ProjCS);
				else if (strnicmp("NAD_1983_StatePlane_", ProjCS, 20) == 0)
					SetStatePlaneFIPS(ProjCS);
				} // if
			// now work on <geographic cs>
			} // if PROJCS
		if (strnicmp("GEOGCS", first, 6) == 0)
			{
			if ((*(first + 6) != '[') && (*(first + 6) != '(') && (*(first + 7) != '"'))
				break;
			last = first = first + 8;
			while ((*last != 0) && (*last != '"'))
				last++;
			strncpy(GeogCS, first, last - first);
			GeogCS[last - first] = NULL;
			first = last;
			if (GeogCS[0])
				{
				if (strnicmp("GCS_North_American_1983", GeogCS, 23) == 0)
					SetSystemByCode(11);
				else if (strnicmp("GCS_North_American_1927", GeogCS, 23) == 0)
					SetSystemByCode(13);
				} // if
			if (*first++ != '"')
				break;
			// now work on <datum>
			if (strnicmp(",DATUM", first, 6) == 0)
				{
				if ((*(first + 6) != '[') && (*(first + 6) != '(') && (*(first + 7) != '"'))
					break;
				last = first = first + 8;
				while ((*last != 0) && (*last != '"'))
					last++;
				strncpy(DatumStr, first, last - first);
				DatumStr[last - first] = NULL;
				if (*last++ != '"')
					break;
				if (*last++ != ',')
					break;
				first = last;
				/***
				if (DatumStr[0] && (stricmp(DatumStr, "D_North_American_1927") == 0))
					Datum.SetDatumByCode(220);
				else if (DatumStr[0] && (stricmp(DatumStr, "D_North_American_1983") == 0))
					Datum.SetDatumByCode(231);
				else
					{
					Datum.SetDatumByCode(0);	// Custom
					if (DatumStr[0])
						{
						strcpy(Datum.DatumName, "WKT: ");
						strcat(Datum.DatumName, DatumStr);
						} // if
					} // else
				***/
				Datum.SetDatumByCode(0);	// set up the default to Custom if we don't actually match anything
				if (DatumStr[0])
					{
					if (stricmp(DatumStr, "D_North_American_1927") == 0)
						Datum.SetDatumByCode(220);
					else if (stricmp(DatumStr, "North American 1927 - Continental US") == 0)
						Datum.SetDatumByCode(220);
					else if (stricmp(DatumStr, "D_North_American_1983") == 0)
						Datum.SetDatumByCode(231);
					else if (stricmp(DatumStr, "North American 1983 - Continental US") == 0)
						Datum.SetDatumByCode(231);
					else if (strcmp(DatumStr, "D_OSGB_1936") == 0)
						Datum.SetDatumByCode(249);
					else if (strcmp(DatumStr, "D_OSGA_1936") == 0)
						Datum.SetDatumByCode(250);
					else if (strcmp(DatumStr, "D_OSGM_1936") == 0)
						Datum.SetDatumByCode(251);
					else if (strcmp(DatumStr, "D_OSGC_1936") == 0)
						Datum.SetDatumByCode(252);
					else if (strcmp(DatumStr, "D_OSGD_1936") == 0)
						Datum.SetDatumByCode(253);
					else
						{
						//strcpy(Datum.DatumName, "WKT: ");
						//strcat(Datum.DatumName, DatumStr);
						strcpy(Datum.DatumName, DatumStr);
						} // else
					} // if
				// now work on <spheroid>
				if (strnicmp("SPHEROID", first, 8) == 0)
					{
					if ((*(first + 8) != '[') && (*(first + 8) != '(') && (*(first + 9) != '"'))
						break;
					last = first = first + 10;
					while ((*last != 0) && (*last != '"'))
						last++;
					strncpy(SpheroidStr, first, last - first);
					SpheroidStr[last - first] = NULL;
					if (*last++ == 0)
						break;
					if (*last++ != ',')
						break;
					first = last;
					dest = tempstr1;
					while ((*first != 0) && (*first != ','))	// copy semi-major axis
						*dest++ = *first++;
					*dest = NULL;
					if (*first == ',')
						first++;
					else
						break;
					dest = tempstr2;
					while ((*first != 0) && (*first != ',')	&& (*first != ']') && (*first != ')'))	// copy inverse flattening
						*dest++ = *first++;
					*dest = NULL;
					// handle optional <authority>
					if (! WKT_Authority(&first))
						break;
					if (*first++ == ']')	// finish <spheroid>
						{
						Datum.Ellipse.SetEllipsoidByCode(44);	// Custom
						SMajor = atof(tempstr1);
						InvFlat = atof(tempstr2);
						if ((SMajor > 0.0) && (InvFlat != 0.0))
							SMinor = SMajor * (1.0 - 1.0 / InvFlat);
						else
							SMinor = 0.0;
						if (SpheroidStr)
							{
							//strcpy(Datum.Ellipse.EllipsoidName, "WKT: ");
							//strcat(Datum.Ellipse.EllipsoidName, SpheroidStr);
							strcpy(Datum.Ellipse.EllipsoidName, SpheroidStr);
							} // if
						if (stricmp(SpheroidStr, "Clarke_1866") == 0)
							{
							Datum.Ellipse.SetEllipsoidByCode(8);
							} // if
						else
							{
							Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(SMajor);
							Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(SMinor);
							} // else
						SpheroidDone = 1;
						}
					else
						break;
					} // if SPHEROID
				// handle <to wgs84s>
				if (strnicmp(",TOWGS84", first, 8) == 0)
					{
					if ((*(first + 8) != '[') && (*(first + 8) != '('))
						break;
					last = first = first + 9;
					// the 7 params appear to be only integers
					sscanf(first, "%lf,%lf,%lf", &DeltaX, &DeltaY, &DeltaZ);
					// flip the sign of DeltaZ since WCS wants them reversed
					DeltaZ = -DeltaZ;
					do
						{
						first++;
						} while ((*first != 0) && (*first != ']') && (*first != ')'));
					if (*first == ']')
						first++;
					else
						break;
					} // if TOWGS84
				// handle optional <authority>
				if (! WKT_Authority(&first))
					break;
				if (*first++ != ']')	// finish <datum>
					break;
				} // if DATUM
			else
				break;
			// now handle <prime meridian>
			if (*first++ == ',')
				{
				/*** F2 NOTE: Nothing is currently done with this info ***/
				if (strnicmp("PRIMEM", first, 6) == 0)
					{
					if ((*(first + 6) != '[') && (*(first + 6) != '(') && (*(first + 7) != '"'))
						break;
					last = first = first + 8;
					do
						{
						last++;
						} while ((*last != 0) && (*last != '"'));
					if ((*last != '"') && (*(last + 1) != ','))
						break;
					// we would access the prime string now
					// we would get the value now
					// but instead we're just ignoring this, and reading until the end of the prime block
					first = last + 2;
					do
						{
						first++;
						} while ((*first != 0) && (*first != ',') && (*first != ']') && (*first != ')'));
					// now we've scanned past the value field
					// handle optional <authority>
					if (! WKT_Authority(&first))
						break;
					if ((*first != ']') && (*first != ')'))
						break;
					first++;	// finish <prime meridian>
					}
				} // <prime meridian>
			else
				break;
			// now handle <angular unit>
			if (*first++ == ',')
				{
				/*** F2 NOTE: Nothing is currently done with this info ***/
				if (strnicmp("UNIT", first, 4) == 0)
					{
					if ((*(first + 4) != '[') && (*(first + 4) != '(') && (*(first + 5) != '"'))
						break;
					last = first = first + 6;
					do
						{
						last++;
						} while ((*last != 0) && (*last != '"'));
					if (*last++ != '"')
						break;
					// we could get the unit string now
					first = last;
					if (*first++ != ',')
						break;
					// we could get the unit value now
					do
						{
						first++;
						} while ((*first != 0) && (*first != ',') && (*first != ']') && (*first != ')'));
					// handle optional <authority>
					if (! WKT_Authority(&first))
						break;
					if ((*first == ']') || (*first == ')'))
						first++;	// finish angular unit
					else
						break;
					}
				} // <angular unit>
			else
				break;
			// handle optional <twin axes>
			/*** F2 NOTE: Nothing is currently done with this info ***/
			if (strnicmp(",AXIS", first, 5) == 0)
				{
				if ((*(first + 5) != '[') && (*(first + 5) != '('))
					break;
				last = first = first + 6;
				do
					{
					first++;
					} while ((*first != 0) && (*first != ']') && (*first != ')'));
				if ((*first == ']') || (*first == ')'))
					first++;
				else
					break;
				if (strnicmp(",AXIS", first, 5) == 0)
					{
					if ((*(first + 5) != '[') && (*(first + 5) != '('))
						break;
					last = first = first + 6;
					do
						{
						first++;
						} while ((*first != 0) && (*first != ']') && (*first != ')'));
					if ((*first == ']') || (*first == ')'))
						first++;
					else
						break;
					} // if ",AXIS"
				else
					break;
				} // if ",AXIS"
			// handle optional <authority>
			if (! WKT_Authority(&first))
				break;
			if (*first == ']')
				{
				if (GeogCS[0])
					{
					//strcpy(ProjSysName, "WKT: ");
					//strcat(ProjSysName, GeogCS);
					strcpy(ProjSysName, GeogCS);
					strcpy(Datum.DatumName, ProjSysName);
					first++;
					} // if
				} // if
			else
				break;
			} // if GEOGCS
		if (parsing_pcs)
			{
			if (*first++ != ',')
				break;
			if (strnicmp("PROJECTION", first, 10) == 0)
				{
				if ((*(first + 10) != '[') && (*(first + 10) != '(') && (*(first + 11) != '"'))
					break;
				last = first = first + 12;
				do
					{
					if (*last == '_')
						*last = ' ';
					last++;
					} while ((*last != 0) && (*last != '"'));
				if (*last != '"')
					break;
				*last++ = NULL;
				if ((*last != ']') && (*last != ')'))
					break;
				if (*(++last) != ',')
					break;
				if (Method.ValidMethod2(first))
					{
					Method.SetMethod2(first);
					if (Method.MethodTable.FetchFieldValueLong(Method.MethodID, "CODE", foo))
						ProjMethodCode = foo;
					else
						ProjMethodCode = 0;	// F2_NOTE: Probably want to put up a warning requester if we hit this (maybe not needed due to ValidMethod)
					} // if
				else
					{
					// F2_NOTE: We should probably put up a requester saying that the projection wasn't recognized
					ProjMethodCode = 0;	// set a default
					if (stricmp(first, "Hotine Oblique Mercator Azimuth Natural Origin") == 0)
						{
						ProjMethodCode = 10;
						Method.SetMethodByCode(ProjMethodCode);
						} // if
					else if (stricmp(first, "Lambert Conformal Conic") == 0)
						{
						ProjMethodCode = 14;
						Method.SetMethodByCode(ProjMethodCode);
						} // else if
					else if (stricmp(first, "Transverse Mercator") == 0)
						{
						ProjMethodCode = 28;
						Method.SetMethodByCode(ProjMethodCode);
						} // else if
					} // else
				first = ++last;
				} // if PROJECTION
			else
				break;
			// handle <parameter> sections
			while (strnicmp("PARAMETER", first, 9) == 0)
				{
				if ((*(first + 9) != '[') && (*(first + 9) != '(') && (*(first + 10) != '"'))
					break;
				last = first = first + 11;
				do
					{
					last++;
					}
					while ((*last != 0) && (*last != '"'));
				if (*last != '"')
					break;
				*last++ = NULL;
				strncpy(ParamStr, first, 63);
				if (*last++ != ',')
					break;
				first = last;
				ParamVal = atof(first);
				do
					{
					last++;
					} while ((*last != 0) && (*last != ']') && (*last != ')'));
				if ((*last != ']') && (*last != ')'))
					break;
				if (*(++last) != ',')
					break;
				first = ++last;
				// now interpret our parameter (xref CoordSys::InitializeMethod)
				ParamNum = 0;
				if (stricmp(ParamStr, "latitude_of_origin") == 0)
					{
					ParamNum = 5;
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						case 2:		// Equidistant Conic, A
							Method.AnimPar[2].SetValue(ParamVal);
							break;
						case 3:		// Equidistant Conic, B
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						case 9:		// Hotine Oblique Mercator, A
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						case 10:	// Hotine Oblique Mercator, B
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						case 22:	// Polyconic
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						case 28:	// Transverse Mercator
							Method.AnimPar[2].SetValue(ParamVal);
							break;
						case 29:	// Van der Grinten
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "latitude_of_origin"
				else if (stricmp(ParamStr, "central_meridian") == 0)
					{
					ParamNum = 4;
					ParamVal = -ParamVal;	// flip sign!
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[2].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[2].SetValue(ParamVal);
							break;
						case 28:	// Transverse Mercator
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "central_meridian"
				else if (stricmp(ParamStr, "scale_factor") == 0)
					{
					ParamNum = 10;
					switch (ProjMethodCode)
						{
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[0].SetValue(ParamVal);
							break;
						case 28:	// Transverse Mercator
							Method.AnimPar[0].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "scale_factor"
				else if (stricmp(ParamStr, "false_easting") == 0)
					{
					ParamNum = 6;
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[4].SetValue(ParamVal);
							break;
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[4].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[4].SetValue(ParamVal);
							break;
						case 28:	// Transverse Mercator
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "false_easting"
				else if (stricmp(ParamStr, "false_northing") == 0)
					{
					ParamNum = 7;
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[5].SetValue(ParamVal);
							break;
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[5].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[5].SetValue(ParamVal);
							break;
						case 28:		// Transverse Mercator
							Method.AnimPar[4].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "false_northing"
				else if (stricmp(ParamStr, "Standard_Parallel_1") == 0)
					{
					ParamNum = 2;
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[0].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[0].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "false_northing"
				else if (stricmp(ParamStr, "Standard_Parallel_2") == 0)
					{
					ParamNum = 3;
					switch (ProjMethodCode)
						{
						case 0:		// Albers Conical Equal Area
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						case 14:	// Lambert Conformal Conic
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch Method.MethodID
					} // if "false_northing"
				else if (stricmp(ParamStr, "Azimuth") == 0)
					{
					switch (ProjMethodCode)
						{
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[1].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch
					} //
				else if (stricmp(ParamStr, "Longitude_Of_Center") == 0)
					{
					switch (ProjMethodCode)
						{
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[2].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch
					} // 
				else if (stricmp(ParamStr, "Latitude_Of_Center") == 0)
					{
					switch (ProjMethodCode)
						{
						case 10:	// Hotine Oblique Mercator B
							Method.AnimPar[3].SetValue(ParamVal);
							break;
						default:
							break;
						} // switch
					} // 
				} // while "PARAMETER"
			// handle <linear unit>
			if (strnicmp("UNIT", first, 4) == 0)
				{
				if ((*(first + 4) != '[') && (*(first + 4) != '(') && (*(first + 5) != '"'))
					break;
				last = first = first + 6;
				do
					{
					last++;
					} while ((*last != 0) && (*last != '"'));
				*last++ = NULL;
				// first = unit name string now
				if (*last++ != ',')
					break;
				if (stricmp(first, "Foot_US") == 0)
					CSLI->MatchEntry("Units FEET");	// this is US foot!
				// read conversion factor
				first = last;
				do
					{
					last++;
					} while ((*last != 0) && (*last != ']') && (*last != ')') && (*last != ','));
				strncpy(tempstr1, first, last - first);
				tempstr1[last - first] = NULL;
				first = last;
				ConvFactor = atof(tempstr1);
				// now scale False Eastings, etc. by ConvFactor
				switch (ProjMethodCode)
					{
					case 10:	// Hotine Oblique Mercator B
						Method.AnimPar[4].SetValue(Method.AnimPar[4].GetCurValue() * ConvFactor);
						Method.AnimPar[5].SetValue(Method.AnimPar[5].GetCurValue() * ConvFactor);
						break;
					case 14:	// Lambert Conformal Conic
						Method.AnimPar[4].SetValue(Method.AnimPar[4].GetCurValue() * ConvFactor);
						Method.AnimPar[5].SetValue(Method.AnimPar[5].GetCurValue() * ConvFactor);
						break;
					case 28:	// Transverse Mercator
						Method.AnimPar[3].SetValue(Method.AnimPar[3].GetCurValue() * ConvFactor);
						Method.AnimPar[4].SetValue(Method.AnimPar[4].GetCurValue() * ConvFactor);
						break;
					} // switch
				// handle optional <authority>
				if (! WKT_Authority(&first))
					break;
				if ((*first != ']') && (*first != ')'))
					break;
				++first;
				} // if "UNIT"
			else
				break;
			// handle optional <twin axes>
			/*** F2 NOTE: Nothing is currently done with this info ***/
			if (strnicmp(",AXIS", first, 5) == 0)
				{
				if ((*(first + 5) != '[') && (*(first + 5) != '('))
					break;
				last = first = first + 6;
				do
					{
					first++;
					} while ((*first != 0) && (*first != ']') && (*first != ')'));
				if (*first == ']')
					first++;
				else
					break;
				if (strnicmp(",AXIS", first, 5) == 0)
					{
					if ((*(first + 5) != '[') && (*(first + 5) != '('))
						break;
					last = first = first + 6;
					do
						{
						first++;
						} while ((*first != 0) && (*first != ']') && (*first != ')'));
					if (*first == ']')
						first++;
					else
						break;
					} // if ",AXIS"
				else
					break;
				} // if ",AXIS"
			// handle optional <authority>
			if (! WKT_Authority(&first))
				break;
			if ((*first != ']') && (*first != ')'))
				break;
			} // if parsing_pcs
		// All done!
		if (ProjCS[0])
			{
			if (strcmp(ProjCS, "British_National_Grid") == 0)
				{
				long cachedDatum = this->Datum.DatumID;

				SetSystemByCode(21);	// This sets up for England itself only.
				// See if the cached one was some OSGB variant & restore if so (NOTE: ID's are based on rows in memory, not Codes).
				if ((cachedDatum >= 253) && (cachedDatum <= 257))
					this->Datum.SetDatum(cachedDatum);
				} // if
			else if ((strncmp(ProjCS, "National RD/NAP", 15) == 0) && (strcmp(GeogCS, "DutchRD") == 0) && (strcmp(DatumStr, "D_DutchRD") == 0))
				{
				SetSystemByCode(33);
				} // else if
			else
				{
				//strcpy(Name, "WKT: ");
				//strncat(Name, ProjCS, WCS_EFFECT_MAXNAMELENGTH - 6);
				Name[0] = NULL;
				strncat(Name, ProjCS, WCS_EFFECT_MAXNAMELENGTH - 6);
				strcpy(ProjSysName, Name);
				} // else
			} // if ProjCS
		} // if Format == 1
	else // 0 or undetermined
		{ // old multiline format
		if (!ReadingParams)
			{
			if (KeyType = CSLI->MatchEntry(InputLine), 1)
				{
				Format = 0;
				switch(KeyType)
					{
					case WCS_COORDSYS_SLOT_PROJECTION: // PROJECT
						{ // [UTM], [STATEPLANE], [UPS]
						if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "UTM"))
							{
							SetSystemByCode(10); // UTM WGS 84
							IsUTM = 1;
							} // if
						else if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "STATEPLANE"))
							{
							// cant set system until we know datum NAD27/NA83
							IsSP = 1;
							if (DatumDone)
								{
								if (IsNAD27)
									{
									SetSystemByCode(4); // SP NAD27
									if (FipsZone != -1) SetStatePlaneZoneByFIPS(FipsZone);
									} // if
								else if (IsNAD83)
									{
									SetSystemByCode(5); // SP NAD83
									if (FipsZone != -1) SetStatePlaneZoneByFIPS(FipsZone);
									} // else if
								} // if
							} // else if
						else if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "GREATBRITAIN_GRID"))
							{
							SetSystemByCode(21); // UK National Grid
							}
						else
							{
							if (Method.SetMethodByPrjName(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal))
								{
								// Try to clue the user in with a useful string for CoordSys name
								SetUniqueName(GlobalApp->AppEffects, CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal);
								ProjDone = 1;
								} // if
							} // else
						break;
						} // WCS_COORDSYS_SLOT_PROJECTION
					case WCS_COORDSYS_SLOT_DATUM:
						{ // NAD27, NAD83, WGS84, HPGN
						// chop off trailing " CNT" if found
						int CNTCheck = 0;
						if ((CNTCheck = (int)strlen(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal)) > 4)
							{
							if (!strcmp(" CNT", &(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal[CNTCheck - 4])))
								{
								CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal[CNTCheck - 4] = 0; // truncate
								TrimTrailingSpaces(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal);
								} // if
							} // if
						if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "NAD27")) IsNAD27 = 1;
						else if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "NAD83")) IsNAD83 = 1;
						else if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "WGS84")) IsWGS84 = 1;
						if (IsSP)
							{
							if (IsNAD27)
								{ // NAD27
								SetSystemByCode(4); // SP NAD27
								if (FipsZone != -1) SetStatePlaneZoneByFIPS(FipsZone);
								DatumDone = 1;
								} // if
							else if (IsNAD83)
								{ // NAD83
								SetSystemByCode(5); // SP NAD83
								if (FipsZone != -1) SetStatePlaneZoneByFIPS(FipsZone);
								DatumDone = 1;
								} // else if
							} // if
						else if (IsUTM && UTMZone != -10000)
							{ // set more specific System now that we have UTM, Zone and Datum
							if (IsNAD27)
								{
								SetSystemByCode(2); // UTM NAD27
								Shorthand[0] = NULL;
								if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
								else sprintf(Shorthand, "%dN", UTMZone);
								SetZoneByShort(Shorthand);
								} // if NAD27
							else if (IsNAD83)
								{
								SetSystemByCode(3); // UTM NAD83
								Shorthand[0] = NULL;
								if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
								else sprintf(Shorthand, "%dN", UTMZone);
								SetZoneByShort(Shorthand);
								} // if NAD83
							else if (IsWGS84)
								{
								SetSystemByCode(10); // UTM WGS84
								Shorthand[0] = NULL;
								if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
								else sprintf(Shorthand, "%dN", UTMZone);
								SetZoneByShort(Shorthand);
								} // if WGS84
							} // if
						if (!DatumDone && Datum.SetDatumByPrjName(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal))
							{
							DatumDone = 1;
							} // if
						break;
						} // WCS_COORDSYS_SLOT_DATUM
					case WCS_COORDSYS_SLOT_ZUNITS: // METERS, NO, FEET, n
					case WCS_COORDSYS_SLOT_UNITS: // METERS, DD, DMS, FEET=USSurvey, 3.280839895 (Intl foot), 3.280833333
						{ // 
						if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "FEET"))
							{
							CSLI->Entries[CSLI->NumEntries - 1].IntVal = WCS_USEFUL_UNIT_FEET_US_SURVEY;
							} // if
						else if (!stricmp(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal, "METERS"))
							{
							CSLI->Entries[CSLI->NumEntries - 1].IntVal = WCS_USEFUL_UNIT_METER;
							} // else if
						else
							{ // custom unit
							CSLI->Entries[CSLI->NumEntries - 1].DVal = atof(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal);
							} // else
						break;
						} // WCS_COORDSYS_SLOT_UNITS, WCS_COORDSYS_SLOT_ZUNITS
					case WCS_COORDSYS_SLOT_SPHEROID:
						{ // GRS1980, AUSTRALIAN, CLARKE1866
						if (Datum.Ellipse.SetEllipsoidByPrjName(CSLI->Entries[CSLI->NumEntries - 1].OrigFieldVal))
							{
							SpheroidDone = 1;
							} // if
						break;
						} // WCS_COORDSYS_SLOT_SPHEROID
					case WCS_COORDSYS_SLOT_XSHIFT:
						{
						break;
						} // WCS_COORDSYS_SLOT_XSHIFT
					case WCS_COORDSYS_SLOT_YSHIFT:
						{
						YShift = CSLI->Entries[CSLI->NumEntries - 1].DVal;
						break;
						} // WCS_COORDSYS_SLOT_YSHIFT
					case WCS_COORDSYS_SLOT_ZONE:
						{
						if (IsUTM)
							{
							Shorthand[0] = NULL;
							UTMZone = CSLI->Entries[CSLI->NumEntries - 1].IntVal;
							if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
							else sprintf(Shorthand, "%dN", UTMZone);
							SetZoneByShort(Shorthand);
							if (DatumDone)
								{
								if (IsNAD27)
									{
									SetSystemByCode(2); // UTM NAD27
									Shorthand[0] = NULL;
									if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
									else sprintf(Shorthand, "%dN", UTMZone);
									SetZoneByShort(Shorthand);
									} // if NAD27
								else if (IsNAD83)
									{
									SetSystemByCode(3); // UTM NAD83
									Shorthand[0] = NULL;
									if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
									else sprintf(Shorthand, "%dN", UTMZone);
									SetZoneByShort(Shorthand);
									} // if NAD83
								else if (IsWGS84)
									{
									SetSystemByCode(10); // UTM WGS84
									Shorthand[0] = NULL;
									if (UTMZone < 0) sprintf(Shorthand, "%dS", abs(UTMZone));
									else sprintf(Shorthand, "%dN", UTMZone);
									SetZoneByShort(Shorthand);
									} // if WGS84
								} // if
							} // if
						break;
						} // WCS_COORDSYS_SLOT_ZONE
					case WCS_COORDSYS_SLOT_PARAMETERS: // parameters could have two values with it, but we ignore them
						{ // Stop reading key,value pairs and get ready to read params
						ReadingParams = 1;
						break;
						} // WCS_COORDSYS_SLOT_PARAMETERS
					case WCS_COORDSYS_SLOT_FIPSZONE:
						{
						FipsZone = CSLI->Entries[CSLI->NumEntries - 1].IntVal;
						if (IsSP && DatumDone)
							{
							SetStatePlaneZoneByFIPS(FipsZone);
							} // if
						break;
						} // WCS_COORDSYS_SLOT_FIPSZONE
					case WCS_COORDSYS_SLOT_QUADRANT:
						{
						break;
						} // WCS_COORDSYS_SLOT_QUADRANT
					case WCS_COORDSYS_SLOT_FLIP:
						{
						break;
						} // WCS_COORDSYS_SLOT_FLIP
					} // switch keytype
				} // if
			else
				{
				break;
				} // else
			if (IsUTM && !(IsNAD27 || IsNAD83 || IsWGS84))
				{
				SetSystem(0L); // System may erroneously be reading UTM WGS84 though it's not WGS 84 now -- set to custom
				} // if
			} // if !
		else
			{ // reading params
			// Do something with the silly params here
			CSLI->ReadParam(InputLine, 1);
			} // else
		} // else
	} // while

if (YShift == 10000000 && IsUTM && Shorthand[0])
	{
	int SHLen = (int)strlen(Shorthand);
	UTMZone = -UTMZone;
	if (Shorthand[SHLen - 1] == 'N')
		{
		Shorthand[SHLen - 1] = 'S'; // switch to Southern hemi
		SetZoneByShort(Shorthand); // re-set system with new hemisphere
		} // if
	// special handling for denoting southern hemi zone by YShift=10million
	} // if

// interpret params
if ((CSLI->NumParam != 0) && (!IsUTM) && (!IsSP))
	{ // first param may indicate number of params and therefore variation of projection
	switch(Method.GCTPMethod)
		{ // BTW: Arc/GCTP considers Equidistant Conic A/B, HOM A/B and SOM A/B to be the same, we don't
		case 0: // geog
		case 1: // utm
		case 2: // sp
			{
			// nothing to do here
			break;
			} // 
		case 3: // albers
		case 27: // hammer
		case 24: // interrupted goode
		case 26: // intr moll
		case 4: // lambert conf con
		case 5: // mercator
		case 25: // mollweide
		case 23: // alaska modified stereographic conf
		case 30: // oblated eq area
		case 6: // polar stereo
		case 7: // polycon
		case 21: // robinson
		case 9: // trans merc
		case 28: // wagner iv
		case 29: // wagner vii
		case 22: // SOM
			{
			// read params without a sphere definition
			for (ParamGet = ParamPut = 0; ParamGet < CSLI->NumParam && ParamPut < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamGet++, ParamPut++)
				{
				// no way this will work.
				AssignValueViaMetricType(&Method.AnimPar[ParamPut], CSLI->ParamBlock[ParamGet]);
				} // for
			break;
			} // no sphere def to skip
		case 11: // lamb az
		case 12: // azimuthal
		case 17: // equirect
		case 15: // gen vert pers
		case 13: // gnomonic
		case 18: // miller cyl
		case 14: // orthographic
		case 16: // sinus
			{
			// read params skipping initial sphere definition
			for (ParamGet = 1, ParamPut = 0; ParamGet < CSLI->NumParam && ParamPut < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamGet++, ParamPut++)
				{
				AssignValueViaMetricType(&Method.AnimPar[ParamPut], CSLI->ParamBlock[ParamGet]);
				} // for
			break;
			} // skip sphere definition
		case 8: // 8: Equidistant Conic A/B
			{
			// set method #3 for extra params
			if (CSLI->ParamBlock[0] == 2) Method.SetMethodByCode(3);
			// read params skipping type code
			for (ParamGet = 1, ParamPut = 0; ParamGet < CSLI->NumParam && ParamPut < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR; ParamGet++, ParamPut++)
				{
				AssignValueViaMetricType(&Method.AnimPar[ParamPut], CSLI->ParamBlock[ParamGet]);
				} // for
			break;
			} // 8: Equidistant Conic A/B
		case 10: // stereo
			{
			int MaxGet = WCS_EFFECTS_PROJMETHOD_NUMANIMPAR;
			ParamGet = 2; // default
			if (CSLI->ParamBlock[0] == 1) ParamGet = 2;
			else if (CSLI->ParamBlock[0] == 2) {ParamGet = 1; MaxGet = 3;}
			// read params possibly skipping some
			for (ParamPut = 0; ParamGet < CSLI->NumParam && ParamPut < MaxGet; ParamGet++, ParamPut++)
				{
				AssignValueViaMetricType(&Method.AnimPar[ParamPut], CSLI->ParamBlock[ParamGet]);
				} // for
			break;
			} // 
		case 19: // grinten
			{
			// we support one more param than Arc PRJ does, so we do this the hard way
			if (CSLI->NumParam > 1) AssignValueViaMetricType(&Method.AnimPar[0], CSLI->ParamBlock[1]);
			if (CSLI->NumParam > 2) AssignValueViaMetricType(&Method.AnimPar[2], CSLI->ParamBlock[2]);
			if (CSLI->NumParam > 3) AssignValueViaMetricType(&Method.AnimPar[3], CSLI->ParamBlock[3]);
			break;
			} // grinten
		case 20: // hotine
			{
			// set method #9 for extra params
			if (CSLI->ParamBlock[0] == 1)
				{
				Method.SetMethodByCode(9);
				if (CSLI->NumParam > 1) AssignValueViaMetricType(&Method.AnimPar[0], CSLI->ParamBlock[1]); // scale fac
				if (CSLI->NumParam > 2) AssignValueViaMetricType(&Method.AnimPar[1], CSLI->ParamBlock[2]); // lat proj
				if (CSLI->NumParam > 3) AssignValueViaMetricType(&Method.AnimPar[4], CSLI->ParamBlock[3]); // lon 1st
				if (CSLI->NumParam > 4) AssignValueViaMetricType(&Method.AnimPar[5], CSLI->ParamBlock[4]); // lat 1st
				if (CSLI->NumParam > 5) AssignValueViaMetricType(&Method.AnimPar[6], CSLI->ParamBlock[5]); // lon 2nd
				if (CSLI->NumParam > 6) AssignValueViaMetricType(&Method.AnimPar[7], CSLI->ParamBlock[6]); // lat 2nd
				if (CSLI->NumParam > 7) AssignValueViaMetricType(&Method.AnimPar[2], CSLI->ParamBlock[7]); // FE
				if (CSLI->NumParam > 8) AssignValueViaMetricType(&Method.AnimPar[3], CSLI->ParamBlock[8]); // FN
				} // if
			else
				{
				if (CSLI->NumParam > 1) AssignValueViaMetricType(&Method.AnimPar[0], CSLI->ParamBlock[1]);
				if (CSLI->NumParam > 2) AssignValueViaMetricType(&Method.AnimPar[2], CSLI->ParamBlock[2]);
				if (CSLI->NumParam > 3) AssignValueViaMetricType(&Method.AnimPar[3], CSLI->ParamBlock[3]);
				if (CSLI->NumParam > 4) AssignValueViaMetricType(&Method.AnimPar[1], CSLI->ParamBlock[4]);
				if (CSLI->NumParam > 5) AssignValueViaMetricType(&Method.AnimPar[4], CSLI->ParamBlock[5]);
				if (CSLI->NumParam > 6) AssignValueViaMetricType(&Method.AnimPar[5], CSLI->ParamBlock[6]);
				} // else
			break;
			} // Hotine Oblique Mercator A/B
		} // switch
	} // if


if (ProjDone || DatumDone || SpheroidDone)
	rVal = CSLI;

return (rVal);

} // CoordSys::LoadFromArcPrj

/*===========================================================================*/

int CSLoadInfo::MatchEntry(char *InputLine, int IgnoreArcComments)
{
int MatchedNum = 0, InLen, KeyScan;
char *MatchSpot;
char TempInput[WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN + 1];

// preprocessor will drop a NULL in wherever we find arc style comment of / followed by *
if (IgnoreArcComments)
	{
	for (MatchSpot = InputLine; MatchSpot[0] && MatchSpot[1]; MatchSpot++)
		{
		if (MatchSpot[0] == '/' && MatchSpot[1] == '*')
			{
			MatchSpot[0] = NULL;
			break;
			} // if
		} // for
	} // if

// skip over any leading whitespace
for (MatchSpot = InputLine; MatchSpot[0] && isspace(MatchSpot[0]); MatchSpot++)
	{
	// do nothing
	} // for

// scan through non-whitespace keyword
TempInput[0] = 0;
for (InLen = 0, MatchSpot = InputLine; MatchSpot[0] && !isspace(MatchSpot[0]) && InLen < WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN; MatchSpot++)
	{
	TempInput[InLen++] = (char)toupper(*MatchSpot);
	TempInput[InLen] = NULL; // Advance NULL
	} // for

if (InLen)
	{ // we got something
	for (KeyScan = 0; KeyScan < WCS_COORDSYS_SLOT_MAX; KeyScan++)
		{
		if (strstr(CSLoadNameMap[KeyScan], TempInput))
			{
			MatchedNum = KeyScan;
			Entries[NumEntries].IdentSlot = MatchedNum;
			strncpy(Entries[NumEntries].OrigFieldName, TempInput, WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN - 1);
			Entries[NumEntries].OrigFieldName[WCS_COORDSYS_LOADINFO_MAXFIELDNAMELEN - 1] = NULL;

			// skip over blanks or = sign
			while (MatchSpot[0] && (isspace(MatchSpot[0]) || MatchSpot[0] == '=')) MatchSpot++;

			// anything left to read?
			if (MatchSpot[0])
				{
				// trim off any EOLs or spaces
				while (isspace(MatchSpot[strlen(MatchSpot) - 1]))
					{
					MatchSpot[strlen(MatchSpot) - 1] = NULL;
					} // while
				strncpy(Entries[NumEntries].OrigFieldVal, MatchSpot, WCS_COORDSYS_LOADINFO_MAXFIELDVALLEN - 1);
				Entries[NumEntries].OrigFieldVal[WCS_COORDSYS_LOADINFO_MAXFIELDVALLEN - 1] = NULL;

				// see if we can convert for the caller
				if (CSLoadNumericMap[MatchedNum] == 1) // as double
					{
					Entries[NumEntries].DVal = atof(MatchSpot);
					} // if

				if (CSLoadNumericMap[MatchedNum] == 2) // as int
					{
					Entries[NumEntries].IntVal = atoi(MatchSpot);
					} // if
				} // if

			NumEntries++;
			break;
			} // got a match
		} // for
	} // if

return(MatchedNum);
} // CSLoadInfo::MatchEntry

/*===========================================================================*/

char *CSLoadInfo::FindEntry(char *FindStr)
{
int i;

for (i = 0; i < NumEntries; ++i)
	{
	if (stricmp(Entries[i].OrigFieldName, FindStr) == 0)
		return Entries[i].OrigFieldVal;
	}

return NULL;

} // CSLoadInfo::FindEntry

/*===========================================================================*/

char *CSLoadInfo::FindEntry(int Ident)
{
int i;

for (i = 0; i < NumEntries; ++i)
	{
	if (Entries[i].IdentSlot == Ident)
		return Entries[i].OrigFieldVal;
	}

return NULL;

} // CSLoadInfo::FindEntry

/*===========================================================================*/

int CSLoadInfo::ReadParam(char *InputLine, int IgnoreArcComments)
{
char *MatchSpot;
char *WholePart = NULL, *MinPart = NULL, *SecPart = NULL;
int Success = 0, IsNeg = 0;

// preprocessor will drop a NULL in wherever we find arc style comment of / followed by *
if (IgnoreArcComments)
	{
	for (MatchSpot = InputLine; MatchSpot[0] && MatchSpot[1]; MatchSpot++)
		{
		if ((MatchSpot[0] == '~') || (MatchSpot[0] == '/' && MatchSpot[1] == '*'))
			{
			MatchSpot[0] = NULL;
			break;
			} // if
		} // for
	} // if

// Do we have anything left to work with?
for (MatchSpot = InputLine; MatchSpot[0]; MatchSpot++)
	{
	if (!isspace(MatchSpot[0]))
		{
		WholePart = MatchSpot;
		break;
		} // if
	} // for

if (WholePart)
	{
	if (strchr(WholePart, '-')) IsNeg = 1;
	int LookForMore = 0;
	// Do we more to work with?
	for (MatchSpot = WholePart; MatchSpot[0]; MatchSpot++)
		{
		if (!isspace(MatchSpot[0]))
			{
			if (LookForMore)
				{
				MinPart = MatchSpot;
				break;
				} // if
			} // if
		else
			{ // found space after the data, start looking for next non-space data
			LookForMore = 1;
			MatchSpot[0] = NULL; // terminate here
			} // else
		} // for
	} // if

if (MinPart)
	{
	int LookForMore = 0;
	// Do we more to work with?
	for (MatchSpot = MinPart; MatchSpot[0]; MatchSpot++)
		{
		if (!isspace(MatchSpot[0]))
			{
			if (LookForMore)
				{
				SecPart = MatchSpot;
				break;
				} // if
			} // if
		else
			{ // found space after the data, start looking for next non-space data
			LookForMore = 1;
			MatchSpot[0] = NULL; // terminate here
			} // else
		} // for
	} // if

if (WholePart)
	{
	if (NumParam < WCS_COORDSYS_LOADINFO_MAXPARAMS)
		{
		ParamBlock[NumParam] = fabs(atof(WholePart));
		Success = 1;
		} // if
	} // if

if (MinPart)
	{
	if (NumParam < WCS_COORDSYS_LOADINFO_MAXPARAMS)
		{
		ParamBlock[NumParam] += fabs(atof(MinPart) / 60.0);
		} // if
	} // if

if (SecPart)
	{
	if (NumParam < WCS_COORDSYS_LOADINFO_MAXPARAMS)
		{
		ParamBlock[NumParam] += fabs(atof(SecPart) / 3600.0);
		} // if
	} // if

if (IsNeg) ParamBlock[NumParam] *= -1.0;

if (Success) NumParam++;

return(Success);

} // CSLoadInfo::ReadParam
