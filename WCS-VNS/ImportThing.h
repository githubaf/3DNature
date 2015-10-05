// ImportThing.h
// Header for the Import Class
// 09/13/99 FPW2
// Copyright 3D Nature 1999

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMPORT_THING
#define WCS_IMPORT_THING

#include "Joe.h"
#include "Projection.h"
#include "EffectsLib.h"

class Pier1		// because Pier1 Imports (doh!)
	{
	public:
		double Ceiling, Floor, TestMin, TestMax;
		double GridSpaceNS, GridSpaceWE;
		double HScale, VScale;					// scale factors to convert to meters
		double NewPlanetRadius;					// If the data being loaded is supposed to override the users setting
		double NullVal, SaveNull;				// For DEM's
		double North, South, East, West;		// Store any boundary related values here (i.e.: Pixel centers)
		double NBound, SBound, EBound, WBound;	// Lat / Lon Bounds of data
		double VertExag, DatumChange, InHeight, InWidth;
		double UserMax, UserMin;										// NewVertStyle (Unified) Max/Min
		double ScaleMMMax, ScaleMMMin;									// Max-Min equivalence
		double ScaleOVIn, ScaleOVOut, ScaleOVSet;						// One value equivalence
		double ScaleTVV1In, ScaleTVV1Out, ScaleTVV2In, ScaleTVV2Out;	// Two value equivalence
		class Pier1 *Next;
		Joe *FileParent, *LastJoe;
		union MapProjection Coords;
		bool forceGPS;
		unsigned char ValueFmt, ValueBytes, ByteOrder;
		unsigned char LoadOpt;	// Import options for that type of data
		unsigned char LoadAs;	// What user chose to import as
		unsigned char OutValFmt, OutValBytes;
		unsigned char NullMethod;	// some people...
		short UTMUnits, UTMZone, Bands, Mode;
		unsigned char BandStyle;
		long Signal;	// pass additional signals here
		short BoundsType, GridUnits, ElevUnits, InFormat, OutFormat;
		long Flags;		// pass additional flags here
		long FoundFlags, ProjectionFlags;	// Primarily for binary files & header data
		long BandGapBytes, BandRowBytes, TotalRowBytes, InFileSize;
		long InRows, InCols, OutDEMNS, OutDEMWE, OutRows, OutCols, HdrSize;
		long CropTop, CropBottom, CropLeft, CropRight;
		char ScaleOVType;
		char ReadOrder, RowsEqual, Reference, ScaleOp, ScaleType;
		struct QuadCorners *Corners;
		void *InputData, *OutputData;
		char MyIdentity[32];	// What Snoopy says
		long MyNum;				// Via Snoopy
		class CoordSys IWCoordSys;
		char FileInfo[1024];
		char InDir[256],InFile[32],LoadName[256+32],OutName[80],NameBase[80];
		char OutDir[256], LayerName[80];
		// The following char vars are really booleans
		char AllowRef00, AllowPosE, AllowUnitChange, AskNull, BoundTypeLocked, ConvertInPlace, CoordSysWarn, DBDisableNULLED, DBRenderFlag;
		char FullGeoRef, HasNulls, HasUTM, HideNULLed, InvertData, KeepBounds, PosEast, ShapeAttribs, ShapeDBNames, ShapeElevs, ShapeLayers;
		char SplineConstrain, SwapRC, TestOnly, UseCeiling, UseFloor, UserMaxSet, UserMinSet, UTMSouthHemi, WantGridUnits, WrapData;
		char InfileErr, OutfileErr, express;
		char Automatic, Factored, TemplateItem, Enabled, Embed, Remove, Conform, FileLayer;
		unsigned short gpsDatum, gpsStart;
		unsigned char gpsFileType, gpsNumFormat;
		char gpsColumn, gpsDelims[8], gpsFieldType, gpsHasDatum, gpsParsing[256];
		char gpsDTab, gpsDSemicolon, gpsDComma, gpsDSpace, gpsDOther, gpsDOtherChar[2], gpsDConsecutive, gpsTextQualifier;
		// These are buffers to be used for any format specific options - currently only used as a byte field to keep track of shapefile selection choices
		char Scratch1[2048];
		char Scratch2[2048];
		char Scratch3[2048];
		char Scratch4[2048];
		// === Vars below here need to be added to Load/Save === //
		// === End of new variables block === //

		Pier1(void);
		~Pier1();
		short DocumentSelf(FILE *output);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
	private:
	};

enum // these are for the Signal variable
	{
	IMSIG_NONE = 0,
	IMSIG_GRIDFLOAT
	};

enum
	{
	IMWIZ_DATA_FORMAT_SIGNEDINT,
	IMWIZ_DATA_FORMAT_UNSIGNEDINT,
	IMWIZ_DATA_FORMAT_FLOAT,
	IMWIZ_DATA_FORMAT_UNKNOWN
	};

enum
	{
	IMWIZ_DATA_BYTEORDER_HILO,
	IMWIZ_DATA_BYTEORDER_LOHI,
	IMWIZ_DATA_BYTEORDER_UNKNOWN
	};

enum
	{
	IMWIZ_DATA_LAYOUT_BIL,
	IMWIZ_DATA_LAYOUT_BIP,
	IMWIZ_DATA_LAYOUT_BSQ
	};

enum
	{
	WCS_IMWIZ_REF_IGNORE = 0,
	WCS_IMWIZ_REF_LOWXY,
	WCS_IMWIZ_REF_ZERO
	};

enum
	{
	WCS_IMWIZ_LOAD_DEM = 0,
	WCS_IMWIZ_LOAD_CMAP,
	WCS_IMWIZ_LOAD_CP,
	WCS_IMWIZ_LOAD_IMAGE,
	WCS_IMWIZ_LOAD_VECTOR
	};

enum
	{
	ELEV_METHOD_ADD = 1,
	ELEV_METHOD_16BIT,
	ELEV_METHOD_24BIT
	};

enum
	{
	IMWIZ_HORUNITS_LATLON = 0,
	IMWIZ_HORUNITS_UTM,
	IMWIZ_HORUNITS_ARBITRARY
	};

enum
	{
	IMWIZ_SURFER_ASCII = 1,		// Version 5 & prior
	IMWIZ_SURFER_BINARY,		// Version 6
	IMWIZ_SURFER_TAGGED			// Version 7 & later
	};

// null value comparison method
enum
	{
	IMWIZ_NULL_EQ,		// equal to
	IMWIZ_NULL_LE,		// less than or equal
	IMWIZ_NULL_GE		// greater than or equal
	};

enum
	{
	IMWIZ_GPS_DELIMITED,
	IMWIZ_GPS_FIXED
	};

enum
	{
	IMWIZ_GPS_COORDS_DDD,
	IMWIZ_GPS_COORDS_DMS
	};

enum
	{
	IMWIZ_GPS_TEXTQUAL_DBLQUOTE,
	IMWIZ_GPS_TEXTQUAL_APOSTROPHE,
	IMWIZ_GPS_TEXTQUAL_NONE
	};

// Load As defines
#define LAS_ERR			0x00
#define LAS_DEM			0x01
#define LAS_CP			0x02
#define LAS_VECT		0x04
#define LAS_IMAGE		0x08

#define IMWIZ_DATA_VALSIZE_UNKNOWN	0
#define IMWIZ_DATA_VALSIZE_BYTE		1
#define IMWIZ_DATA_VALSIZE_SHORT	2
#define IMWIZ_DATA_VALSIZE_LONG		4
#define IMWIZ_DATA_VALSIZE_DOUBLE	8

// WCS file IO defines
#define	WCS_PIER1_VALUEFMT			0x00010000
#define	WCS_PIER1_VALUEBYTES		0x00020000
#define	WCS_PIER1_BYTEORDER			0x00030000
#define	WCS_PIER1_LOADOPT			0x00040000
#define	WCS_PIER1_LOADAS			0x00050000
#define	WCS_PIER1_OUTVALFMT			0x00060000
#define	WCS_PIER1_OUTVALBYTES		0x00070000
#define	WCS_PIER1_NULLMETHOD		0x00080000
#define	WCS_PIER1_BANDSTYLE			0x00090000
#define	WCS_PIER1_SCALEOVTYPE		0x000a0000
#define	WCS_PIER1_READORDER			0x000b0000
#define	WCS_PIER1_ROWSEQUAL			0x000c0000
#define	WCS_PIER1_REFERENCE			0x000d0000
#define	WCS_PIER1_SCALEOP			0x000e0000
#define	WCS_PIER1_SCALETYPE			0x000f0000

#define	WCS_PIER1_MYIDENTITY		0x00110000
#define	WCS_PIER1_FILEINFO			0x00120000
#define	WCS_PIER1_INDIR				0x00130000
#define	WCS_PIER1_INFILE			0x00140000
#define	WCS_PIER1_LOADNAME			0x00150000
#define	WCS_PIER1_OUTNAME			0x00160000
#define	WCS_PIER1_NAMEBASE			0x00170000
#define	WCS_PIER1_OUTDIR			0x00180000
#define	WCS_PIER1_ALLOWREF00		0x00190000
#define	WCS_PIER1_ALLOWPOSE			0x001a0000
#define	WCS_PIER1_ALLOWUNITCHANGE	0x001b0000
#define	WCS_PIER1_ASKNULL			0x001c0000
#define	WCS_PIER1_CONVERTINPLACE	0x001d0000
#define	WCS_PIER1_COORDSYSWARN		0x001e0000
#define	WCS_PIER1_DBDISABLEDNULL	0x001f0000

#define	WCS_PIER1_DBRENDERFLAG		0x00210000
#define	WCS_PIER1_FULLGEOREF		0x00220000
#define	WCS_PIER1_HASNULLS			0x00230000
#define	WCS_PIER1_HASUTM			0x00240000
#define	WCS_PIER1_HIDENULLED		0x00250000
#define	WCS_PIER1_INVERTDATA		0x00260000
#define	WCS_PIER1_KEEPBOUNDS		0x00270000
#define	WCS_PIER1_POSEAST			0x00280000
#define	WCS_PIER1_SHAPEATTRIBS		0x00290000
#define	WCS_PIER1_SHAPEDBNAMES		0x002a0000
#define	WCS_PIER1_SHAPEELEVS		0x002b0000
#define	WCS_PIER1_SHAPELAYERS		0x002c0000
#define	WCS_PIER1_SPLINECONSTRAIN	0x002d0000
#define	WCS_PIER1_SWAPRC			0x002e0000
#define	WCS_PIER1_TESTONLY			0x002f0000

#define	WCS_PIER1_USECEILING		0x00310000
#define	WCS_PIER1_USEFLOOR			0x00320000
#define	WCS_PIER1_USERMAXSET		0x00330000
#define	WCS_PIER1_USERMINSET		0x00340000
#define	WCS_PIER1_UTMSOUTHHEMI		0x00350000
#define	WCS_PIER1_WANTGRIDUNITS		0x00360000
#define	WCS_PIER1_WRAPDATA			0x00370000
#define	WCS_PIER1_INFILEERR			0x00380000
#define	WCS_PIER1_OUTFILEERR		0x00390000
#define	WCS_PIER1_UTMUNITS			0x003a0000
#define	WCS_PIER1_UTMZONE			0x003b0000
#define	WCS_PIER1_BANDS				0x003c0000
#define	WCS_PIER1_MODE				0x003d0000
#define	WCS_PIER1_GRODUNITS			0x003e0000
#define	WCS_PIER1_ELEVUNITS			0x003f0000

#define	WCS_PIER1_INFORMAT			0x00410000
#define	WCS_PIER1_OUTFORMAT			0x00420000
#define	WCS_PIER1_SIGNAL			0x00430000
#define	WCS_PIER1_FLAGS				0x00440000
#define	WCS_PIER1_FOUNDFLAGS		0x00450000
#define	WCS_PIER1_BANDGAPBYTES		0x00460000
#define	WCS_PIER1_BANDROWBYTES		0x00470000
#define	WCS_PIER1_TOTALROWBYTES		0x00480000
#define	WCS_PIER1_INFILESIZE		0x00490000
#define	WCS_PIER1_INROWS			0x004a0000
#define	WCS_PIER1_INCOLS			0x004b0000
#define	WCS_PIER1_OUTDEMNS			0x004c0000
#define	WCS_PIER1_OUTDEMWE			0x004d0000
#define	WCS_PIER1_OUTROWS			0x004e0000
#define	WCS_PIER1_OUTCOLS			0x004f0000

#define	WCS_PIER1_HDRSIZE			0x00510000
#define	WCS_PIER1_CROPTOP			0x00520000
#define	WCS_PIER1_CROPBOTTOM		0x00530000
#define	WCS_PIER1_CROPLEFT			0x00540000
#define	WCS_PIER1_CROPRIGHT			0x00550000
#define	WCS_PIER1_MYNUM				0x00560000
#define	WCS_PIER1_CEILING			0x00570000
#define	WCS_PIER1_FLOOR				0x00580000
#define	WCS_PIER1_TESTMIN			0x00590000
#define	WCS_PIER1_TESTMAX			0x005a0000
#define	WCS_PIER1_GRIDSPACENS		0x005b0000
#define	WCS_PIER1_GRIDSPACEWE		0x005c0000
#define	WCS_PIER1_HSCALE			0x005d0000
#define	WCS_PIER1_VSCALE			0x005e0000
#define	WCS_PIER1_NEWPLANETRADIUS	0x005f0000

#define	WCS_PIER1_NULLVAL			0x00610000
#define	WCS_PIER1_SAVENULL			0x00620000
#define	WCS_PIER1_NORTH				0x00630000
#define	WCS_PIER1_SOUTH				0x00640000
#define	WCS_PIER1_EAST				0x00650000
#define	WCS_PIER1_WEST				0x00660000
#define	WCS_PIER1_NBOUND			0x00670000
#define	WCS_PIER1_SBOUND			0x00680000
#define	WCS_PIER1_EBOUND			0x00690000
#define	WCS_PIER1_WBOUND			0x006a0000
#define	WCS_PIER1_VERTEXAG			0x006b0000
#define	WCS_PIER1_DATUMCHANGE		0x006c0000
#define	WCS_PIER1_INHEIGHT			0x006d0000
#define	WCS_PIER1_INWIDTH			0x006e0000
#define	WCS_PIER1_USERMAX			0x006f0000

#define	WCS_PIER1_USERMIN			0x00710000
#define	WCS_PIER1_SCALEMMMAX		0x00720000
#define	WCS_PIER1_SCALEMMMIN		0x00730000
#define	WCS_PIER1_SCALEOVIN			0x00740000
#define	WCS_PIER1_SCALEOVOUT		0x00750000
#define	WCS_PIER1_SCALEOVSET		0x00760000
#define	WCS_PIER1_SCALETVV1IN		0x00770000
#define	WCS_PIER1_SCALETVV1OUT		0x00780000
#define	WCS_PIER1_SCALETVV2IN		0x00790000
#define	WCS_PIER1_SCALETVV2OUT		0x007a0000
#define	WCS_PIER1_CORNEREASTING0	0x007b0000
#define	WCS_PIER1_CORNEREASTING1	0x007c0000
#define	WCS_PIER1_CORNEREASTING2	0x007d0000
#define	WCS_PIER1_CORNEREASTING3	0x007e0000
#define	WCS_PIER1_CORNERNORTHING0	0x007f0000

#define	WCS_PIER1_CORNERNORTHING1	0x00810000
#define	WCS_PIER1_CORNERNORTHING2	0x00820000
#define	WCS_PIER1_CORNERNORTHING3	0x00830000
#define	WCS_PIER1_IWCOORDSYS		0x00840000
#define	WCS_PIER1_AUTOMATIC			0x00850000
#define	WCS_PIER1_FACTORED			0x00860000
#define	WCS_PIER1_ENABLED			0x00870000
#define	WCS_PIER1_SCRATCH1			0x00880000
#define	WCS_PIER1_SCRATCH2			0x00890000
#define	WCS_PIER1_SCRATCH3			0x008a0000
#define	WCS_PIER1_SCRATCH4			0x008b0000
#define	WCS_PIER1_EMBED				0x008c0000
#define	WCS_PIER1_LAYERNAME			0x008d0000
#define	WCS_PIER1_FILELAYER			0x008e0000
#define	WCS_PIER1_CONFORM			0x008f0000

#endif // WCS_IMPORT_THING
