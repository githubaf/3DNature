// DataOpsDefs.h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef DATAOPS_DEFS
#define DATAOPS_DEFS

// Import Wizard supported file types - these MUST be in same order as drop box!
// When adding new formats, also add panel order arrays & set for new type in SetPanelOrder
enum
{
IW_INPUT_ARCASCII,
IW_INPUT_ARCBINARYADF_GRID,
//IW_INPUT_ARCEXPORT_COVER,
IW_INPUT_ARCEXPORT_DEM,
IW_INPUT_ASCII_ARRAY,
IW_INPUT_BINARY,
IW_INPUT_BRYCE,
IW_INPUT_DTED,
IW_INPUT_DXF,
IW_INPUT_GTOPO30,
IW_INPUT_IMAGE_BMP,
IW_INPUT_IMAGE_ECW,
IW_INPUT_IMAGE_IFF,
IW_INPUT_IMAGE_JPEG,
// IW_INPUT_IMAGE_MR_SID,
IW_INPUT_IMAGE_PICT,
IW_INPUT_IMAGE_PNG,
IW_INPUT_IMAGE_TARGA,
IW_INPUT_IMAGE_TIFF,
IW_INPUT_MDEM,
IW_INPUT_NTF_DTM,
//IW_INPUT_NTF_MERIDIAN2,
IW_INPUT_SDTS_DEM,
IW_INPUT_SDTS_DLG,
IW_INPUT_SHAPE,
IW_INPUT_SRTM,
IW_INPUT_STM,
#ifdef GO_SURFING
IW_INPUT_SURFER,
#endif // GO_SURFING
IW_INPUT_TERRAGEN,
IW_INPUT_USGS_ASCII_DEM,
IW_INPUT_USGS_ASCII_DLG,
IW_INPUT_VISTAPRO,
IW_INPUT_WCS_DEM,
IW_INPUT_WCS_XYZ,
IW_INPUT_WCS_ZBUFFER,
IW_INPUT_XYZ,
IW_INPUT_ASCII_GPS,
IW_INPUT_GPX,
IW_INPUT_MAX_FORMATS
};

enum
{
WCS_IMPORTDATA_FORMAT_UNKNOWN,
WCS_IMPORTDATA_FORMAT_WCSDEM,
WCS_IMPORTDATA_FORMAT_ARCVIEWASCII,
WCS_IMPORTDATA_FORMAT_BRYCE,
//WCS_IMPORTDATA_FORMAT_DLG,
WCS_IMPORTDATA_FORMAT_DTED,
WCS_IMPORTDATA_FORMAT_DXF,
WCS_IMPORTDATA_FORMAT_IMAGE,
//WCS_IMPORTDATA_FORMAT_SHAPE,
//WCS_IMPORTDATA_FORMAT_USGS30,
//WCS_IMPORTDATA_FORMAT_USGS90,
//WCS_IMPORTDATA_FORMAT_VISTAPRO3,
//WCS_IMPORTDATA_FORMAT_WDB,
//WCS_IMPORTDATA_FORMAT_WXYZ,
//WCS_IMPORTDATA_FORMAT_XYZ,
WCS_IMPORTDATA_MAX_FORMATS
};

enum
{
WCS_IMPORTDATA_OUTPUTSTYLE_DEM,
WCS_IMPORTDATA_OUTPUTSTYLE_CONTROL,
WCS_IMPORTDATA_OUTPUTSTYLE_VECTOR
};

enum
{
WCS_IMPORTDATA_HUNITS_DEGREES,
WCS_IMPORTDATA_HUNITS_KILOMETERS,
WCS_IMPORTDATA_HUNITS_METERS,
WCS_IMPORTDATA_HUNITS_DECIMETERS,
WCS_IMPORTDATA_HUNITS_CENTIMETERS,
WCS_IMPORTDATA_HUNITS_MILLIMETERS,
WCS_IMPORTDATA_HUNITS_MILES,
WCS_IMPORTDATA_HUNITS_YARDS,
WCS_IMPORTDATA_HUNITS_FEET,
WCS_IMPORTDATA_HUNITS_SURVEY_FEET,
WCS_IMPORTDATA_HUNITS_INCHES,
VNS_IMPORTDATA_HUNITS_DEGREES,
VNS_IMPORTDATA_HUNITS_LINEAR
};

// Pier1 - BoundsType
enum
{
VNS_BOUNDSTYPE_DEGREES,
VNS_BOUNDSTYPE_LINEAR
};


enum
{
WCS_IMPORTDATA_COORD_X,
WCS_IMPORTDATA_COORD_Y,
WCS_IMPORTDATA_COORD_Z
};

// InFormat
enum
{
DEM_DATA2_INPUT_UNKNOWN,
DEM_DATA2_INPUT_ARCVIEWASCII,
DEM_DATA2_INPUT_ASCII,
DEM_DATA2_INPUT_BINARY,
DEM_DATA2_INPUT_BIL,
DEM_DATA2_INPUT_BIP,
DEM_DATA2_INPUT_BSQ,
DEM_DATA2_INPUT_BRYCE,
DEM_DATA2_INPUT_DTED,
DEM_DATA2_INPUT_DXF,
DEM_DATA2_INPUT_IFF,
DEM_DATA2_INPUT_MDEM,
DEM_DATA2_INPUT_SDTS_DEM,
DEM_DATA2_INPUT_SURFER,
DEM_DATA2_INPUT_TERRAGEN,
DEM_DATA2_INPUT_USGS_DEM,
DEM_DATA2_INPUT_VISTA,
DEM_DATA2_INPUT_WCSDEM,
DEM_DATA2_INPUT_ZBUF,
VECT_DATA2_INPUT_DXF,
VECT_DATA2_INPUT_SDTS_DLG,
VECT_DATA2_INPUT_SHAPEFILE,
VECT_DATA2_INPUT_USGS_DLG,
DEM_DATA2_INPUT_NTF_DTM,
DEM_DATA2_INPUT_NTF_MERIDIAN2,
VECT_DATA2_INPUT_NTF_MERIDIAN2,
DEM_DATA2_INPUT_ARC_EXPORT_GRID,
DEM_DATA2_INPUT_ARC_BINARYADF_GRID,
VECT_DATA2_ASCII_GPS,
VECT_DATA2_INPUT_GPX
};

// These need to be in same order as IDC_COMBOOUTTYPEOUTFORMAT
enum
{
DEM_DATA2_OUTPUT_BINARY,
DEM_DATA2_OUTPUT_BINARY_TERRAIN,
DEM_DATA2_OUTPUT_COLORBMP,
DEM_DATA2_OUTPUT_COLORIFF,
DEM_DATA2_OUTPUT_COLORPICT,
DEM_DATA2_OUTPUT_COLORTGA,
DEM_DATA2_OUTPUT_CONTROLPTS,
DEM_DATA2_OUTPUT_GRAYIFF,
DEM_DATA2_OUTPUT_MASTERCMAP,
DEM_DATA2_OUTPUT_WCSDEM,
DEM_DATA2_OUTPUT_ZBUF,
DEM_DATA2_OUTPUT_UNSET,			// keep these two last
DEM_DATA2_OUTPUT_SET
};

// FoundFlags
#define FOUND_NOTHING			0
#define FOUND_X_MIN				(1 << 0)
#define FOUND_X_MAX				(1 << 1)
#define FOUND_Y_MIN				(1 << 2)
#define FOUND_Y_MAX				(1 << 3)
#define FOUND_DATA_FORMAT		(1 << 4)
#define FOUND_DATA_BYTES		(1 << 5)
#define FOUND_DATA_ORDER		(1 << 6)
#define FOUND_WIDTH				(1 << 7)
#define FOUND_HEIGHT			(1 << 8)

// ProjectionFlags
#define PROJECTION_FLAG_PROJECTION		(1 << 0)
#define PROJECTION_FLAG_DATUM			(1 << 1)
#define PROJECTION_FLAG_ZUNITS			(1 << 2)
#define PROJECTION_FLAG_UNITS			(1 << 3)
#define PROJECTION_FLAG_SPHEROID		(1 << 4)
#define PROJECTION_FLAG_XSHIFT			(1 << 5)
#define PROJECTION_FLAG_YSHIFT			(1 << 6)

// Flags
#define ARC_VARIANT			(1<<1)

#define GET_ELEV_UNITS		(1<<13)		// vertical units need defining
#define GET_NSEW_UNITS		(1<<14)		// horizontal units need defining
#define DEAD_END			(1<<15)		// don't ever use this for anything else

#define FOUND_COLS			(1<<1)
#define FOUND_ROWS			(1<<2)
#define FOUND_XLL			(1<<3)
#define FOUND_YLL			(1<<4)
#define FOUND_CELLSIZE		(1<<5)
#define FOUND_BYTEORDER		(1<<6)
#define FOUND_BITS			(1<<7)
#define FOUND_TFWDATA		(1<<8)
#define FOUND_XDIM			(1<<9)
#define FOUND_YDIM			(1<<10)

#define FOUND_ULX			(1<<3)
#define FOUND_ULY			(1<<4)

#define FOUND_ALTW			(1<<3)

#define DXF_3DFACE			(1<<12)

#define DEM_DATA_FORMAT_SIGNEDINT	0
#define DEM_DATA_FORMAT_UNSIGNEDINT	1
#define DEM_DATA_FORMAT_FLOAT		2
#define DEM_DATA_FORMAT_UNKNOWN		3
#define DEM_DATA_VALSIZE_UNKNOWN	0	
#define DEM_DATA_VALSIZE_BYTE		1
#define DEM_DATA_VALSIZE_SHORT		2
#define DEM_DATA_VALSIZE_LONG		4
#define DEM_DATA_VALSIZE_DOUBLE		8
#define DEM_DATA_BYTEORDER_HILO		0
#define DEM_DATA_BYTEORDER_LOHI		1
#define DEM_DATA_BYTEORDER_UNKNOWN	2
#define DEM_DATA_ROW_LAT			0
#define DEM_DATA_ROW_LON			1
#define DEM_DATA_UNITS_DEGREE		0
#define DEM_DATA_UNITS_KILOM		1
#define DEM_DATA_UNITS_METERS		2
#define DEM_DATA_UNITS_DECIM		3
#define DEM_DATA_UNITS_CENTIM		4
#define DEM_DATA_UNITS_MILLIM		5
#define DEM_DATA_UNITS_MILES		6
#define DEM_DATA_UNITS_YARDS		7
#define DEM_DATA_UNITS_FEET			8
#define DEM_DATA_UNITS_SURVEY_FEET	9
#define DEM_DATA_UNITS_INCHES		10
#define DEM_DATA_UNITS_OTHER		11
#define DEM_DATA_READORDER_ROWS		0
#define DEM_DATA_READORDER_COLS		1
#define DEM_DATA_SCALEOP_MATCHMATCH		0
#define DEM_DATA_SCALEOP_MATCHSCALE		1
#define DEM_DATA_SCALEOP_MAXMINSCALE	2
#define DEM_DATA_SCALEOP_UNIFIED		3
#define DEM_DATA_SCALETYPE_MAXEL		0
#define DEM_DATA_SCALETYPE_MINEL		1
#define DEM_DATA_SCALETYPE_SCALE		2

struct ImportData
{
short RasterInput, OutputStyle, Resample, ReverseX, ReverseXEnabled, InputFormat,
	FormatEnabled[WCS_IMPORTDATA_MAX_FORMATS], OutStyleEnabled[3],
	InputHUnitsEnabled, InputHUnits, InputVUnits, OutputVUnits, UTMUnits, UTMZone, ColorImage, ZeroReference;
long InputRows, InputCols, OutputRows, OutputCols, OutputNSMaps, OutputWEMaps, HdrSize;
double InputHigh[3], InputLow[3], OutputHigh[3], OutputLow[3], GridSpaceNS, GridSpaceWE, OutputHighLat, OutputHighLon,
	OutputLowLat, OutputLowLon, OutputHighElev, OutputLowElev, InputXRange, OutputXRange, InputYRange, OutputYRange,
	InputElRange, OutputElRange, OutputXScale, OutputYScale, OutputElScale;
double MaxVal[3], MinVal[3];
char OutName[80];
char filename[256];
void (*ScaleLatFunc)(struct ImportData *, double &);
void (*ScaleLonFunc)(struct ImportData *, double &);
void (*ScaleElevFunc)(struct ImportData *, float &);
};

void ScaleLatitude(struct ImportData *DaData, double &Value);
void ScaleLongitude(struct ImportData *DaData, double &Value);
void ScaleElevation(struct ImportData *DaData, float &Value);

struct USGS_DEMHeader
{
char 	FileName[145],
		LevelCode[7],
		ElPattern[7],
		RefSysCode[7],
		Zone[7],
		ProjPar[15][25],
		GrUnits[7],
		ElUnits[7],
		PolySides[7],
		Coords[4][2][25],
		ElMin[25],
		ElMax[25],
		AxisRot[25],
		Accuracy[7],
		Resolution[3][13],
		Rows[7],
		Columns[7];
// old format stops at above
char	VDatum[3], HDatum[3];
short   voidvalue, fillvalue;
};

struct USGS_DEMProfileHeader
{
char	Row[7],
		Column[7],
		ProfRows[7],
		ProfCols[7],
		Coords[2][25],
		ElDatum[25],
		ElMin[25],
		ElMax[25];
};

struct DEMExtractData
{
struct	USGS_DEMHeader USGSHdr;
struct	USGS_DEMProfileHeader ProfHdr;
double	SELat,
		SELon;
		FileReq *FR;
char 	elevpath[256],
		elevfile[32];
char 	*pattern;
struct	DEMInfo *Info;
struct	UTMLatLonCoords *Convert;
double	MaxNorth[2],
		MinNorth[2],
		MaxEast[2],
		MinEast[2],
		MaxLat,
		MinLat,
		MaxLon,
		MinLon,
		UTMRowInt,
		UTMColInt,
		LLRowInt,
		LLColInt,
		FirstRow,
		FirstCol;
long	UTMRows,
		UTMCols,
		LLRows,
		LLCols,
		UTMSize,
		LLSize;
float	*UTMData,
		*LLData;
};

struct DXFLayerPens
{
struct	DXFLayerPens *Next;
char	LayerName[256];
short	Pen;
};

// MicroDEM support

struct tDMAMapBasicDef
{
double	h_Adat, h_f;
short	h_XDat, h_YDat, h_ZDat;
char	DatumCode[6];	// string[5] in Object Pascal
char	EllipsCode[3];	// string[2] in Object Pascal
};

enum	// DEMAvailableType
{
MDEM_TYPE_UTMDEM,
MDEM_TYPE_ARCSECONDDEM
};

struct MDEM_Hdr
{
short	NumCol, NumRow;
char	OSquareID[3];	// string[2] in Object Pascal
short	MaxElev;
short	MinElev;
char	LatInterval,LongInterval;
char	UnusedWhereMax[9];	// string[8] in Object Pascal
char	UnusedWhereMin[9];	// string[8] in Object Pascal
char	WhichSphere;
char	UnusedOldDatum;	// correct size???
long	BaseXUTM24K, BaseYUTM24K;
char	ElevUnits;		// correct size???
char	Unused2[8];
char	UTMZone;
char	DEMUsed;		// correct size???
char	DataSpacing;	// correct size???
char	DigitizeDatum;	// correct size???
short	DigitizeLong0, DigitizeLat0, DigitizeScale;
long	DigitizeRadEarth;
short	ElevOffset;
char	LatHemisph;
char	unusedDatumNumber;
short	unusedEllpsoidNumber;
tDMAMapBasicDef	DMAMapDefinition;
char	FutureUse;
char	CreationInformation[30];	// string[29] in Object Pascal
};

struct QuadCorners
{
double Easting[4];
double Northing[4];
};

#endif // DATAOPS_DEFS
