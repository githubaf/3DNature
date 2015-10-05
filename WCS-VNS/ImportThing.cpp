// ImportThing.cpp
// Code for the Import Class
// 09/13/99 FPW2
// Copyright 3D Nature 1999

#include "stdafx.h"
#include "Log.h"
#include "ImportThing.h"
#include "DataOpsDefs.h"
#include "Useful.h"
#include "AppMem.h"

#ifdef WCS_BUILD_VNS
extern WCSApp *GlobalApp;
#endif // WCS_BUILD_VNS

Pier1::Pier1()
{
#ifdef WCS_BUILD_VNS
PlanetOpt *DefPlanetOpt;
#endif // WCS_BUILD_VNS

Next = NULL;
FileParent = LastJoe = NULL;
#ifdef WCS_BUILD_VNS
GridUnits = WCS_IMPORTDATA_HUNITS_METERS;
#else // WCS_BUILD_VNS
GridUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
#endif // WCS_BUILD_VNS
ElevUnits = DEM_DATA_UNITS_METERS;
ValueBytes = 1;
ValueFmt = 0;
// set platform default byte order
#ifdef BYTEORDER_BIGENDIAN
ByteOrder = IMWIZ_DATA_BYTEORDER_HILO;
#else // BYTEORDER_BIGENDIAN
ByteOrder = IMWIZ_DATA_BYTEORDER_LOHI;
#endif // BYTEORDER_BIGENDIAN
LoadOpt = LoadAs = 0;
OutValBytes = 1;
OutValFmt = 0;
UTMUnits = UTMZone = 0;
Bands = 1;
Mode = 0;
Signal = 0;
Flags = 0;
BandGapBytes = BandRowBytes = TotalRowBytes = HdrSize = InFileSize = 0;
InRows = InCols = 1;
OutDEMNS = OutDEMWE = 1;
OutRows = OutCols = 0;
CropTop = CropBottom = CropLeft = CropRight = 0;
Ceiling = Floor = 0.0;
TestMin = FLT_MAX;
TestMax = FLT_MIN;
GridSpaceNS = GridSpaceWE = NullVal = SaveNull = 0.0;
HScale = VScale = 1.0;
NewPlanetRadius = 0.0;
North = South = East = West = 0.0;
NBound = SBound = EBound = WBound = 0.0;
VertExag = 1.0;
DatumChange = InHeight = InWidth = 0.0;
ReadOrder = RowsEqual = Reference = ScaleOp = ScaleType = 0;
ScaleMMMax = ScaleMMMin = ScaleOVIn = ScaleOVOut = ScaleOVSet = ScaleOVType = 0;
ScaleTVV1In = ScaleTVV1Out = ScaleTVV2In = ScaleTVV2Out = 0;
AllowRef00 =  AllowPosE = AllowUnitChange = AskNull = 1;
InvertData = 0;
FullGeoRef = HasNulls = HasUTM = 0;
PosEast = SplineConstrain = 1;
SwapRC = 0;
TestOnly = UseCeiling = UseFloor = UTMSouthHemi = WantGridUnits = WrapData = 0;
UserMaxSet = UserMinSet = 0;
InFormat = 0;
OutFormat = DEM_DATA2_OUTPUT_UNSET;
InfileErr = OutfileErr = 0;
strcpy(InDir, "WCSProjects:");
InFile[0] = LoadName[0] = 0;
OutDir[0] = OutName[0] = NameBase[0] = 0;
MyIdentity[0] = 0;
UserMax = UserMin = 0;
InputData = OutputData = NULL;
MyNum = 0;
DBRenderFlag = TRUE;
KeepBounds = FALSE;
CoordSysWarn = TRUE;
NullMethod = IMWIZ_NULL_EQ;
ConvertInPlace = FALSE;
DBDisableNULLED = TRUE;
HideNULLed = TRUE;
BandStyle = IMWIZ_DATA_LAYOUT_BIL;
FileInfo[0] = 0;
ShapeAttribs = ShapeDBNames = TRUE;
#ifdef WCS_BUILD_VNS
ShapeLayers = FALSE;
#else // WCS_BUILD_VNS
ShapeLayers = TRUE;
#endif // WCS_BUILD_VNS
ShapeElevs = FALSE;
FoundFlags = FOUND_NOTHING;
Corners = NULL;
BoundsType = VNS_BOUNDSTYPE_DEGREES;
BoundTypeLocked = TRUE;
ProjectionFlags = 0;
memset(Scratch1, 0, sizeof(Scratch1));
memset(Scratch2, 0, sizeof(Scratch2));
memset(Scratch3, 0, sizeof(Scratch3));
memset(Scratch4, 0, sizeof(Scratch4));
Factored = FALSE;
Automatic = FALSE;
TemplateItem = Embed = Remove = 0;
Enabled = 1;
LayerName[0] = 0;
Conform = FALSE;
express = 0;
gpsColumn = -1;
gpsDatum = gpsStart = 1;
gpsFileType = 0;
gpsHasDatum = 0;
gpsDelims[0] = ',';
gpsDelims[1] = 0;
memset(gpsParsing, 0, sizeof(gpsParsing));
gpsDTab = gpsDSemicolon = gpsDComma = gpsDSpace = gpsDOther = 0;
gpsDOtherChar[0] = gpsDOtherChar[1] = 0;
gpsDConsecutive = 0;
gpsTextQualifier = IMWIZ_GPS_TEXTQUAL_DBLQUOTE;
gpsNumFormat = IMWIZ_GPS_COORDS_DDD;
forceGPS = false;

#ifdef WCS_BUILD_VNS
FileLayer = TRUE;
#else // WCS_BUILD_VNS
FileLayer = FALSE;
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_VNS
if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	{
	if (DefPlanetOpt->Coords)
		IWCoordSys.Copy(&IWCoordSys, DefPlanetOpt->Coords);
	} // if
#endif // WCS_BUILD_VNS

} // Pier1::Pier1()

/*===========================================================================*/

Pier1::~Pier1()
{

if (Corners)
	AppMem_Free(Corners, sizeof(struct QuadCorners));

} // Pier1::~Pier1()

/*===========================================================================*/

short Pier1::DocumentSelf(FILE *output)
{

if (output == NULL)
	return 0;

fprintf(output, "*** WCS IMPORT WIZARD SETTINGS ***\n");
fprintf(output, "InFile = %s\n", InFile);
return 1;

} // Pier1::DocumentSelf

/*===========================================================================*/

unsigned long Pier1::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_PIER1_VALUEFMT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ValueFmt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_VALUEBYTES:
						{
						BytesRead = ReadBlock(ffile, (char *)&ValueBytes, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_BYTEORDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ByteOrder, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_LOADOPT:
						{
						BytesRead = ReadBlock(ffile, (char *)&LoadOpt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_LOADAS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LoadAs, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTVALFMT:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutValFmt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTVALBYTES:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutValBytes, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NULLMETHOD:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_BANDSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BandStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEOVTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleOVType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_READORDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReadOrder, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ROWSEQUAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&RowsEqual, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_REFERENCE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Reference, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEOP:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleOp, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_MYIDENTITY:
						{
						BytesRead = ReadBlock(ffile, (char *)MyIdentity, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_PIER1_FILEINFO:
						{
						BytesRead = ReadBlock(ffile, (char *)FileInfo, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#endif // WCS_BUILD_VNS
					case WCS_PIER1_INDIR:
						{
						BytesRead = ReadBlock(ffile, (char *)InDir, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)InFile, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_LOADNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)LoadName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)OutName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NAMEBASE:
						{
						BytesRead = ReadBlock(ffile, (char *)NameBase, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTDIR:
						{
						BytesRead = ReadBlock(ffile, (char *)OutDir, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_LAYERNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)LayerName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCRATCH1:
						{
						BytesRead = ReadLongBlock(ffile, (char *)Scratch1, Size);
						break;
						}
					case WCS_PIER1_SCRATCH2:
						{
						BytesRead = ReadLongBlock(ffile, (char *)Scratch2, Size);
						break;
						}
					case WCS_PIER1_SCRATCH3:
						{
						BytesRead = ReadLongBlock(ffile, (char *)Scratch3, Size);
						break;
						}
					case WCS_PIER1_SCRATCH4:
						{
						BytesRead = ReadLongBlock(ffile, (char *)Scratch4, Size);
						break;
						}
					case WCS_PIER1_ALLOWREF00:
						{
						BytesRead = ReadBlock(ffile, (char *)&AllowRef00, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ALLOWPOSE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AllowPosE, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ALLOWUNITCHANGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AllowUnitChange, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ASKNULL:
						{
						BytesRead = ReadBlock(ffile, (char *)&AskNull, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CONVERTINPLACE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConvertInPlace, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_COORDSYSWARN:
						{
						BytesRead = ReadBlock(ffile, (char *)&CoordSysWarn, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_DBDISABLEDNULL:
						{
						BytesRead = ReadBlock(ffile, (char *)&DBDisableNULLED, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_DBRENDERFLAG:
						{
						BytesRead = ReadBlock(ffile, (char *)&DBRenderFlag, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FULLGEOREF:
						{
						BytesRead = ReadBlock(ffile, (char *)&FullGeoRef, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_HASNULLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&HasNulls, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_HASUTM:
						{
						BytesRead = ReadBlock(ffile, (char *)&HasUTM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_HIDENULLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&HideNULLed, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INVERTDATA:
						{
						BytesRead = ReadBlock(ffile, (char *)&InvertData, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_KEEPBOUNDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&KeepBounds, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_POSEAST:
						{
						BytesRead = ReadBlock(ffile, (char *)&PosEast, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SHAPEATTRIBS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShapeAttribs, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SHAPEDBNAMES:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShapeDBNames, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SHAPEELEVS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShapeElevs, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SHAPELAYERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShapeLayers, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SPLINECONSTRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&SplineConstrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SWAPRC:
						{
						BytesRead = ReadBlock(ffile, (char *)&SwapRC, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_TESTONLY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TestOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USECEILING:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseCeiling, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USEFLOOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseFloor, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USERMAXSET:
						{
						BytesRead = ReadBlock(ffile, (char *)&UserMaxSet, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USERMINSET:
						{
						BytesRead = ReadBlock(ffile, (char *)&UserMinSet, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_UTMSOUTHHEMI:
						{
						BytesRead = ReadBlock(ffile, (char *)&UTMSouthHemi, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_WANTGRIDUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&WantGridUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_WRAPDATA:
						{
						BytesRead = ReadBlock(ffile, (char *)&WrapData, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INFILEERR:
						{
						BytesRead = ReadBlock(ffile, (char *)&InfileErr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTFILEERR:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutfileErr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_AUTOMATIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Automatic, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FACTORED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Factored, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_EMBED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Embed, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FILELAYER:
						{
						BytesRead = ReadBlock(ffile, (char *)&FileLayer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CONFORM:
						{
						BytesRead = ReadBlock(ffile, (char *)&Conform, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}

					case WCS_PIER1_UTMUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&UTMUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_UTMZONE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UTMZone, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_BANDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Bands, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_MODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Mode, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_GRODUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&GridUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_ELEVUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ElevUnits, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)&InFormat, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutFormat, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}

					case WCS_PIER1_SIGNAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&Signal, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FLAGS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Flags, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FOUNDFLAGS:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoundFlags, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_BANDGAPBYTES:
						{
						BytesRead = ReadBlock(ffile, (char *)&BandGapBytes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_BANDROWBYTES:
						{
						BytesRead = ReadBlock(ffile, (char *)&BandRowBytes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_TOTALROWBYTES:
						{
						BytesRead = ReadBlock(ffile, (char *)&TotalRowBytes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INFILESIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&InFileSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INROWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&InRows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INCOLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&InCols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTDEMNS:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutDEMNS, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTDEMWE:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutDEMWE, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTROWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutRows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_OUTCOLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutCols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_HDRSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HdrSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CROPTOP:
						{
						BytesRead = ReadBlock(ffile, (char *)&CropTop, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CROPBOTTOM:
						{
						BytesRead = ReadBlock(ffile, (char *)&CropBottom, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CROPLEFT:
						{
						BytesRead = ReadBlock(ffile, (char *)&CropLeft, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_CROPRIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&CropRight, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_MYNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&MyNum, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}

					case WCS_PIER1_CEILING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Ceiling, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_FLOOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&Floor, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_TESTMIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&TestMin, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_TESTMAX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TestMax, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_GRIDSPACENS:
						{
						BytesRead = ReadBlock(ffile, (char *)&GridSpaceNS, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_GRIDSPACEWE:
						{
						BytesRead = ReadBlock(ffile, (char *)&GridSpaceWE, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_HSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_VSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NEWPLANETRADIUS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NewPlanetRadius, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NULLVAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullVal, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SAVENULL:
						{
						BytesRead = ReadBlock(ffile, (char *)&SaveNull, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NORTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&North, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SOUTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&South, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_EAST:
						{
						BytesRead = ReadBlock(ffile, (char *)&East, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_WEST:
						{
						BytesRead = ReadBlock(ffile, (char *)&West, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_NBOUND:
						{
						BytesRead = ReadBlock(ffile, (char *)&NBound, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SBOUND:
						{
						BytesRead = ReadBlock(ffile, (char *)&SBound, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_EBOUND:
						{
						BytesRead = ReadBlock(ffile, (char *)&EBound, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_WBOUND:
						{
						BytesRead = ReadBlock(ffile, (char *)&WBound, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_VERTEXAG:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertExag, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_DATUMCHANGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&DatumChange, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&InHeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_INWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&InWidth, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USERMAX:
						{
						BytesRead = ReadBlock(ffile, (char *)&UserMax, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_USERMIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&UserMin, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEMMMAX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleMMMax, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEMMMIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleMMMin, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEOVIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleOVIn, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEOVOUT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleOVOut, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALEOVSET:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleOVSet, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALETVV1IN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleTVV1In, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALETVV1OUT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleTVV1Out, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALETVV2IN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleTVV2In, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_PIER1_SCALETVV2OUT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ScaleTVV2Out, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}

					case WCS_PIER1_CORNEREASTING0:
						{
						if (Corners || (Corners = (struct QuadCorners *)AppMem_Alloc(sizeof (struct QuadCorners), APPMEM_CLEAR)))
							BytesRead = ReadBlock(ffile, (char *)&Corners->Easting[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNEREASTING1:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Easting[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNEREASTING2:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Easting[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNEREASTING3:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Easting[3], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNERNORTHING0:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Northing[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNERNORTHING1:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Northing[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNERNORTHING2:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Northing[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_PIER1_CORNERNORTHING3:
						{
						if (Corners)
							BytesRead = ReadBlock(ffile, (char *)&Corners->Northing[3], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}

					case WCS_PIER1_IWCOORDSYS:
						{
						BytesRead = IWCoordSys.Load(ffile, Size, ByteFlip);
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

} // Pier1::Load

/*===========================================================================*/

unsigned long Pier1::Save(FILE *ffile)
{
ULONG BytesWritten, ItemTag, TotalWritten = 0;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_VALUEFMT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ValueFmt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_VALUEBYTES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ValueBytes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_BYTEORDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ByteOrder)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_LOADOPT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LoadOpt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_LOADAS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LoadAs)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTVALFMT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OutValFmt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTVALBYTES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OutValBytes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NULLMETHOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NullMethod)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_BANDSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BandStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEOVTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ScaleOVType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_READORDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReadOrder)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ROWSEQUAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RowsEqual)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_REFERENCE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Reference)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEOP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ScaleOp)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ScaleType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ALLOWREF00, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AllowRef00)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ALLOWPOSE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AllowPosE)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ALLOWUNITCHANGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AllowUnitChange)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ASKNULL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AskNull)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CONVERTINPLACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ConvertInPlace)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_COORDSYSWARN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CoordSysWarn)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_DBDISABLEDNULL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DBDisableNULLED)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_DBRENDERFLAG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DBRenderFlag)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FULLGEOREF, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FullGeoRef)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_HASNULLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HasNulls)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_HASUTM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HasUTM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_HIDENULLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HideNULLed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INVERTDATA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&InvertData)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_KEEPBOUNDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&KeepBounds)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_POSEAST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PosEast)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SHAPEATTRIBS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShapeAttribs)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SHAPEDBNAMES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShapeDBNames)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SHAPEELEVS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShapeElevs)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SHAPELAYERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShapeLayers)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SPLINECONSTRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SplineConstrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SWAPRC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SwapRC)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_TESTONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TestOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USECEILING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseCeiling)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USEFLOOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseFloor)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USERMAXSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UserMaxSet)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USERMINSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UserMinSet)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_UTMSOUTHHEMI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UTMSouthHemi)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_WANTGRIDUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WantGridUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_WRAPDATA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WrapData)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INFILEERR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&InfileErr)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTFILEERR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OutfileErr)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_AUTOMATIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Automatic)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FACTORED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Factored)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_EMBED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Embed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FILELAYER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FileLayer)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CONFORM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Conform)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_MYIDENTITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(MyIdentity) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MyIdentity)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#ifdef WCS_BUILD_VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FILEINFO, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(FileInfo) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FileInfo)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#endif // WCS_BUILD_VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INDIR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(InDir) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)InDir)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(InFile) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)InFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_LOADNAME, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(LoadName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)LoadName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(OutName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)OutName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NAMEBASE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(NameBase) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)NameBase)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTDIR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(OutDir) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)OutDir)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_LAYERNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(LayerName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)LayerName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PIER1_SCRATCH1, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, sizeof(Scratch1),
	WCS_BLOCKTYPE_CHAR, (char *)Scratch1)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PIER1_SCRATCH2, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, sizeof(Scratch2),
	WCS_BLOCKTYPE_CHAR, (char *)Scratch2)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PIER1_SCRATCH3, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, sizeof(Scratch3),
	WCS_BLOCKTYPE_CHAR, (char *)Scratch3)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_PIER1_SCRATCH4, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, sizeof(Scratch4),
	WCS_BLOCKTYPE_CHAR, (char *)Scratch4)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_UTMUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UTMUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_UTMZONE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UTMZone)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_BANDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Bands)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_MODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Mode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_GRODUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&GridUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_ELEVUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ElevUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&InFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&OutFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SIGNAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Signal)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FLAGS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Flags)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FOUNDFLAGS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&FoundFlags)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_BANDGAPBYTES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&BandGapBytes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_BANDROWBYTES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&BandRowBytes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_TOTALROWBYTES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TotalRowBytes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INFILESIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&InFileSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&InRows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INCOLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&InCols)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTDEMNS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutDEMNS)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTDEMWE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutDEMWE)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutRows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_OUTCOLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutCols)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_HDRSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&HdrSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CROPTOP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&CropTop)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CROPBOTTOM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&CropBottom)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CROPLEFT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&CropLeft)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CROPRIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&CropRight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_MYNUM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MyNum)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CEILING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Ceiling)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_FLOOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Floor)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_TESTMIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TestMin)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_TESTMAX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TestMax)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_GRIDSPACENS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&GridSpaceNS)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_GRIDSPACEWE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&GridSpaceWE)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_HSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_VSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NEWPLANETRADIUS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NewPlanetRadius)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NULLVAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NullVal)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SAVENULL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SaveNull)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NORTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&North)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SOUTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&South)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_EAST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&East)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_WEST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&West)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_NBOUND, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NBound)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SBOUND, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SBound)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_EBOUND, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&EBound)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_WBOUND, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&WBound)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_VERTEXAG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertExag)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_DATUMCHANGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&DatumChange)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&InHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_INWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&InWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USERMAX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&UserMax)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_USERMIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&UserMin)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEMMMAX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleMMMax)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEMMMIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleMMMin)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEOVIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleOVIn)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEOVOUT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleOVOut)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALEOVSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleOVSet)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALETVV1IN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleTVV1In)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALETVV1OUT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleTVV1Out)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALETVV2IN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleTVV2In)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_SCALETVV2OUT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ScaleTVV2Out)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (Corners)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNEREASTING0, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Easting[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNEREASTING1, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Easting[1])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNEREASTING2, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Easting[2])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNEREASTING3, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Easting[3])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNERNORTHING0, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Northing[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNERNORTHING1, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Northing[1])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNERNORTHING2, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Northing[2])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_PIER1_CORNERNORTHING3, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&Corners->Northing[3])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if corners

// CoordSys chunk
ItemTag = WCS_PIER1_IWCOORDSYS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = IWCoordSys.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -((signed long int)BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if coord system saved 
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
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // Pier1::Save
