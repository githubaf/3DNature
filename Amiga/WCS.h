/* WCS.h
** Structures and variables for GIS source
** Renamed from GIS.h on 14 Jan 1994 by CXH
** Built on 24 Jul 1993 from gis.c and gisam.c by Chris "Xenon" Hanson.
** Original code and subsequent rape, pillage and plunder by Gary R. Huber.
*/
#ifndef _WCS_H_
#define _WCS_H_

#define DEB_MAX 100000
#define AF_DEBUG(s)                {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s\n",            __FILE__,__func__,__LINE__,s);} }
#define AF_DEBUG_d(s,x)            {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %d\n",         __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_f(s,x)            {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %f\n",         __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_f_f(s,x,y)        {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %.8f %.8f\n",      __FILE__,__func__,__LINE__,s,x,y);} }
#define AF_DEBUG_f_f_f(s,x,y,z)    {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %f %f %f\n",   __FILE__,__func__,__LINE__,s,x,y,z);} }
#define AF_DEBUG_hd(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %hd\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_hd_hd_hd(s,x,y,z) {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %hd %hd %hd\n",__FILE__,__func__,__LINE__,s,x,y,z);} }
#define AF_DEBUG_hu(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %hu\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_hu_hu(s,x,y)      {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %hu %hu\n",    __FILE__,__func__,__LINE__,s,x,y);} }
#define AF_DEBUG_hx(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %hx\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_ld(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %ld\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_ld_ld(s,x,y)      {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %ld %ld\n",    __FILE__,__func__,__LINE__,s,x,y);} }
#define AF_DEBUG_ld_ld_ld(s,x,y,z) {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %ld %ld %ld\n",__FILE__,__func__,__LINE__,s,x,y,z);} }
#define AF_DEBUG_lu(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %lu\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_lx(s,x)           {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %lx\n",        __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_s(s,x)            {static int i; if(i++<DEB_MAX) {printf("%s %s() Line %d: %s %s\n",         __FILE__,__func__,__LINE__,s,x);} }
#define AF_DEBUG_float_hex(s,x)    {static int i; if(i++<DEB_MAX) \
        { \
            union \
            { \
                float a; \
                unsigned char bytes[sizeof(float)]; /* sizeof float = 4 */ \
            } u; \
            u.a = x; \
            printf("%s %s() Line %d: %s %02x %02x %02x %02x\n",__FILE__,__func__,__LINE__,s,u.bytes[0],u.bytes[1],u.bytes[2],u.bytes[3]); \
        }}
#define AF_DEBUG_double_hex(s,x)  {static int i; if(i++<DEB_MAX) \
        { \
            union \
            { \
                double a; \
                unsigned char bytes[sizeof(double)]; /* sizeof double = 8 */ \
            } u; \
            u.a = x; \
            printf("%s %s() Line %d: %s %02x %02x %02x %02x %02x %02x %02x %02x\n",        __FILE__,__func__,__LINE__,s,u.bytes[0],u.bytes[1],u.bytes[2],u.bytes[3],u.bytes[4],u.bytes[5],u.bytes[6],u.bytes[7]); \
        }}



#ifndef GIS_GIS_H
#define GIS_GIS_H

#include <fcntl.h>
#include <unistd.h>

#ifndef __SASC
   #include "sasc_functions.h"   // own implemtation of SAS/C specific functions
#else
   int Mkdir(const char *name); // AF:  a wrapper for mkdir()
#endif


//#define O_RDONLY 0               /* ALEXANDER */
//#define O_WRONLY 1
//#define O_RDWR   2               /* ALEXANDER */
//#define O_APPEND    (1<<2)
//#define O_CREAT     (1<<3)
//#define O_TRUNC     (1<<5)

#ifdef __SASC
#define __func__ __FUNC__  // for debug output of function name
#endif

#ifdef MAIN
#define EXTERN
#else
 #define EXTERN extern
#endif

#define USE_GIS_GUI

#include "Headers.h"
#include "GUIExtras.h"
#include "Defines.h"
#include "GUIDefines.h"

#ifdef AMIGA_GUI
EXTERN struct IntuitionBase *IntuitionBase
#ifdef MAIN
 = NULL
#endif /* MAIN */
;
EXTERN struct GfxBase *GfxBase
#ifdef MAIN
 = NULL
#endif /* MAIN */
;
EXTERN struct Library *AslBase
#ifdef MAIN
 = NULL
#endif /* MAIN */
;
EXTERN struct Library *GadToolsBase
#ifdef MAIN
 = NULL
#endif /* MAIN */
;
EXTERN struct Library *MUIMasterBase
#ifdef MAIN
 = NULL
#endif /* MAIN */
;

#ifdef __SASC
   #include <clib/muimaster_protos.h>   //ALEXANDER
#else
   #ifdef __AROS__
      #include <proto/muimaster.h>
      #include <libraries/mui.h>
   #else
      #define NO_INLINE_STDARG              // Bebbo
      #include <proto/muimaster_lib.h>      // Alexander
      #undef  NO_INLINE_STDARG
   #endif
#endif


EXTERN LONG MemTrack;


EXTERN struct Screen *WCSScrn;
EXTERN struct Window *InterWind0,*InterWind2,
	*RenderWind0;
//EXTERN struct FileRequester *frbase,*frparam,*frfile;  static AF
EXTERN struct IntuiMessage Event;
#endif /* AMIGA_GUI */

/*EXTERN*/ struct WCSApp
	{
	void *MUIApp;
	APTR OldSysReqWin;
	struct Process *Proc;
	}; /* struct WCSApp */

EXTERN struct ARexxContext *RexxAp;

/*EXTERN*/ struct WCSScreenMode
	{
	struct WCSScreenMode *Next;
	ULONG ModeID;
	char ModeName[32];
	int X, Y, OX, OY, MaxX, MaxY, UX, UY;
	struct tPoint OScans[4]; /* By order: Text, Std, Max, Video */
	ULONG PropertyFlags;
	UWORD PixelSpeed;
	}; /* struct WCSScreenMode */

EXTERN struct WCSScreenData
	{
        ULONG ModeID, OTag, OVal, AutoTag, AutoVal;
	long Width, Height;
	} ScrnData;


#ifdef __AROS__
   #define PACKED __attribute__((__packed__))
#else
   #define PACKED
#endif

/* AF struct packed for AROS, 12.Dec.22 */
EXTERN struct PACKED database {
 char 	Enabled;
 char 	Name[11];
 char 	Layer1[3];
 char 	Layer2[4];
 USHORT LineWidth;
 USHORT Color;
 char 	Pattern[2];
 char 	Label[16];
 USHORT Points;
 char 	Mark[2];
 USHORT Red, Grn, Blu;
 char	Special[4];
 USHORT MaxFract;
 UBYTE 	Flags;
/* Flag bits:
	1	Modified since last save
	2	Enabled for mapping and rendering	
	3	Enabled when file loaded	
	4
	5
	6
	7
	8
*/
 ULONG	VecArraySize;
 double	*Lat,
	*Lon;
 short 	*Elev;	
/* SE, SE, SW, NW, NE */
} *DBase;

EXTERN struct DirList {
 struct DirList *Next;
 char Read;
 char Name[255];
} *DL;

/*EXTERN*/ struct ParHeaderV1 {
 char 	FType[8];
 float 	Version;
 short  FirstFrame,
	LastFrame,
	KeyFrames,
	CurrentKey;
};
/* AF struct packed for AROS, 12.Dec.22 */
/*EXTERN*/ struct PACKED ParHeader {
 char	FType[8];
 float	Version;
 LONG	ByteOrder;
 short	KeyFrames;
 LONG	MotionParamsPos,
	ColorParamsPos,
	EcoParamsPos,
	SettingsPos,
	KeyFramesPos;
};

EXTERN struct ParHeader ParHdr;

/*EXTERN*/ struct MotionV1 {
 double Value[2];		/* 0=First, 1=Last */
 short	Frame[2],		/* 0=Start, 1=End */
	Ease[2];		/* 0=In, 1=Out */
};

/*EXTERN*/ struct Motion {
 double Value;
};

/*EXTERN*/ struct MotionShift {
 double Value;
};

EXTERN struct MotionShift MoShift[MOTIONPARAMS];

/*EXTERN*/ struct AnimationV1 {
 struct MotionV1 mn[MOTIONPARAMSV1];
};

/*EXTERN*/ struct Animation {
 struct Motion mn[MOTIONPARAMS];
};

EXTERN /*__far*/ struct Animation MoPar;   // ALEXANDER __far raus

EXTERN /*__far*/ struct Animation UndoMoPar[2];    // ALEXANDER __far raus
 
/*EXTERN*/ struct ColorV1 {
 char   Name[24];
 short	Value[6],
	Frame[2],
	Ease[2];
};

/*EXTERN*/ struct Color {
 char	Name[24];
 short	Value[3];
};

/*EXTERN*/ struct ColorShift {
 double Value[3];
};

EXTERN struct ColorShift CoShift[COLORPARAMS];  // not static. Is used via macro from everywhere, AF

/*EXTERN*/ struct PaletteV1 {
 struct ColorV1 cn[COLORPARAMSV1];
};

/*EXTERN*/ struct Palette {
 struct Color cn[COLORPARAMS];
};

EXTERN __far struct Palette CoPar;

EXTERN __far struct Palette UndoCoPar[2];

/*EXTERN*/ struct EcosystemV1 {
 char   Name[24];
 short	Line[2],		/* Snow or Tree line... */
	Skew[2],		/* Line skew or water depth */
	SkewAz[2],		/* Azimuth for skew effect or wind direction */
	RelEl[2],		/* Relative Elevation multiplier */
	MaxRelEl[2],		/* Max Relative Elevation */
	MinRelEl[2],		/* Min Relative Elevation */
	MaxSlope[2],		/* Max slope */
	MinSlope[2],		/* Min slope */
	Type,			/* 0=water, 1=snow, 2=rock, 3=bare, 4=conifer, */
				/*  5=deciduous, 6=low vegetation */
	Tree[2],		/* 0=tree density (%), 1=tree height (meters) */
	Color,			/* Palette number, 0=main color, 1=understory */
	UnderEco,		/* Understory ecosystem */
	MatchColor[3];		/* RGB values for color map matching */
 char	Model[32];
}; 

/*EXTERN*/ struct OldEcosystemV1 {
 char   Name[24];
 short	Line[2],		/* Snow or Tree line... */
	Skew[2],		/* Snow line skew... */
	SkewAz[2],		/* Azimuth for skew effect */
	RelEl[2],		/* Relative Elevation multiplier */
	MaxRelEl[2],		/* Max Relative Elevation */
	MinRelEl[2],		/* Min Relative Elevation */
	MaxSlope[2],		/* Max slope */
	MinSlope[2],		/* Min slope */
	Type,			/* 0=water, 1=snow, 2=rock, 3=bare, 4=conifer, */
				/*  5=deciduous, 6=low vegetation */
	Tree[2],		/* 0=tree density (%), 1=tree height (meters) */
	Color,			/* Palette number, 0=main color, 1=understory */
	UnderEco,		/* Understory ecosystem */
	MatchColor[3],		/* RGB values for color map matching */
        Frame[2],
        Ease[2];
}; 

/*EXTERN*/ struct Ecosystem2V1 {
 char   Name[24];
 short	Value[24];
 char 	Model[32];
}; 

/*EXTERN*/ struct Ecosystem {
 char	Name[24];
 float	Line,
	Skew,
	SkewAz,
	RelEl,
	MaxRelEl,
	MinRelEl,
	MaxSlope,
	MinSlope,
	Density,
	Height;
 short	Type,
	Color,
	UnderEco,
	MatchColor[3];
 char	Model[32];
};

/*EXTERN*/ struct Ecosystem2 {
 char	Name[24];
 float	FltValue[10];
 short	ShtValue[6];
 char	Model[32];
};

/*EXTERN*/ struct EcosystemShiftV1 {
 double Line[3],		/* Snow or Tree line... */
	Skew[3],		/* Snow line skew... */
	SkewAz[3],		/* Azimuth for skew effect */
	RelEl[3],		/* Relative Elevation multiplier */
	MaxRelEl[3],		/* Max Relative Elevation */
	MinRelEl[3],		/* Min Relative Elevation */
	MaxSlope[3],		/* Max slope */
	MinSlope[3],		/* Min slope */
	SkewLat,		/* Skew and SkewAz combined */
	SkewLon,
	MXSlope,		/* Radian MaxSlope */
	MNSlope;		/* Radian MinSlope */
 short	MainCol[3],		/* Overstory or dominant color */
	SubCol[3];		/* Understory or subdominant color */
};

/*EXTERN*/ struct BitmapImage {
 UBYTE	*Bitmap[3];
 float  *Covg, *MidPt, *Span;
 short	Width, Height, Colors, PalCol;
 double HeightPct, DensityPct;
 struct BitmapImage *Smaller, *Next;
};

/*EXTERN*/ struct EcosystemShift {
 double Line,		/* Snow or Tree line... */
	Skew,		/* Snow line skew... */
	SkewAz,		/* Azimuth for skew effect */
	RelEl,		/* Relative Elevation multiplier */
	MaxRelEl,		/* Max Relative Elevation */
	MinRelEl,		/* Min Relative Elevation */
	MaxSlope,		/* Max slope */
	MinSlope,		/* Min slope */
	Density,		/* Tree density */
	Height,		/* Tree height */
	SkewLat,		/* Skew and SkewAz combined */
	SkewLon,
	MXSlope,		/* Radian MaxSlope */
	MNSlope;		/* Radian MinSlope */
 short	MainCol[3],		/* Overstory or dominant color */
	SubCol[3],		/* Understory or subdominant color */
	BitmapImages;		/* number of bitmap images for this ecosystem */
 struct BitmapImage *BMImage;
 struct Ecotype *Ecotype;
 float	ImageIncr;
};

EXTERN struct EcosystemShift EcoShift[ECOPARAMS];

/*EXTERN*/ union EnvironmentV1 {
 struct EcosystemV1 en[ECOPARAMSV1];
 struct Ecosystem2V1 en2[ECOPARAMSV1];
};

/*EXTERN*/ union Environment {
 struct Ecosystem en[ECOPARAMS];
 struct Ecosystem2 en2[ECOPARAMS];
};

EXTERN __far union Environment EcoPar;

EXTERN __far union Environment UndoEcoPar[2];

/*EXTERN*/ struct SettingsV1 {
 short	startframe,
	maxframes,
	startseg,		/* formerly campath */
	smoothfaces,		/* formerly focpath */
	bankturn,
	colrmap,
	borderandom,
	cmaptrees,
	rendertrees,
	statistics,
	stepframes,
	zbufalias,
	horfix,
	horizonmax,
	clouds,
	linefade,
	drawgrid,
	gridsize,
	alternateq,
	linetoscreen,
	mapassfc,
        cmapluminous,		/* formerly maptoraster */
        surfel[4],
	worldmap,
	flatteneco,
	fixfract,
	vecsegs,
        reliefshade,
	renderopts,
	scrnwidth,
	scrnheight,
 	rendersegs,
	overscan,
	lookahead,
	composite,
	defaulteco,
	ecomatch,
	Yoffset,
	saveIFF,
	background,
	zbuffer,
	antialias,
	scaleimage,
	fractal,
	aliasfactor,
	scalewidth,
	scaleheight;
 double zalias,
	bankfactor,
	skyalias,
	lineoffset,
	altqlat,
	altqlon,
	treefactor,
	displacement,	/* formerly lowglobe */
	unused3,	/* obsolete, formerly highglobe */
	picaspect,
	zenith;
 short	exportzbuf,
	zformat,
	fieldrender,
 	lookaheadframes,
	velocitydistr,
	easein,
	easeout,
	displace,
	mastercmap,		/* use master color map */
	cmaporientation,	/* 0=WCS DEM, 1=normal image */
	fielddominance,
	fractalmap,
	perturb,
	realclouds,
	reflections,
	waves;
 double	dispslopefact,
	globecograd,
	globsnowgrad,
	globreflat;
};

/*EXTERN*/ struct Settings {
 short	startframe,
	maxframes,
	startseg,		/* formerly campath */
	smoothfaces,		/* formerly focpath */
	bankturn,
	colrmap,
	borderandom,
	cmaptrees,
	rendertrees,
	statistics,		/* why is this still here? */
	stepframes,
	zbufalias,
	horfix,
	horizonmax,
	clouds,
	linefade,
	drawgrid,
	gridsize,
	alternateq,
	linetoscreen,
	mapassfc,
        cmapluminous,		/* formerly maptoraster */
        surfel[4],
	worldmap,
	flatteneco,
	fixfract,
	vecsegs,
        reliefshade,
	renderopts,
	scrnwidth,
	scrnheight,
 	rendersegs,
	overscan,
	lookahead,
	composite,
	defaulteco,
	ecomatch,
	Yoffset,
	saveIFF,
	background,
	zbuffer,
	antialias,
	scaleimage,
	fractal,
	aliasfactor,
	scalewidth,
	scaleheight;
 short	exportzbuf,
	zformat,
	fieldrender,
 	lookaheadframes,
	velocitydistr,
	easein,
	easeout,
	displace,
	mastercmap,		/* use master color map */
	cmaporientation,	/* 0=WCS DEM, 1=normal image */
	fielddominance,
	fractalmap,
	perturb,
	realclouds,
	reflections,
	waves,
	colorstrata,
	cmapsurface,
	deformationmap,
	moon,
	sun,
	tides,
	sunhalo,
	moonhalo;
 short	extrashorts[EXTRASHORTSETTINGS];
 double	extradoubles[EXTRADOUBLESETTINGS];
 double	deformscale,
	stratadip,
	stratastrike,
	dispslopefact,
	globecograd,
	globsnowgrad,
	globreflat,
	zalias,
	bankfactor,
	skyalias,
	lineoffset,
	altqlat,
	altqlon,
	treefactor,
	displacement,	/* formerly lowglobe */
	unused3,	/* obsolete, formerly highglobe */
	picaspect,
	zenith;
};

EXTERN struct Settings settings;

EXTERN __far struct Settings UndoSetPar[2];

/*EXTERN*/ struct NoLinearMotionKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 double	Value;
};

/*EXTERN*/ struct NoLinearColorKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Value[3];
};

/*EXTERN*/ struct NoLinearEcosystemKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Line,			/* Snow or Tree line... */
	Skew,			/* Snow line skew... */
	SkewAz,			/* Azimuth for skew effect */
	RelEl,			/* Relative Elevation multiplier */
	MaxRelEl,		/* Max Relative Elevation */
	MinRelEl,		/* Min Relative Elevation */
	MaxSlope,		/* Max slope */
	MinSlope;		/* Min slope */
};

/*EXTERN*/ struct NoLinearEcosystemKey2 {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Value[8];
};

/*EXTERN*/ union NoLinearKeyFrame {
 struct NoLinearMotionKey MoKey;
 struct NoLinearColorKey CoKey;
 struct NoLinearEcosystemKey EcoKey;
 struct NoLinearEcosystemKey2 EcoKey2;
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  MotionKey {
 short	KeyFrame,
	Group,            // AF: 2.Jan23  0=Motion Key, 1=Color Key, 2=Ecosystem Key
	Item;             // AF: 2.Jan23  Motion 8 = Bank Key (?)
 float	TCB[3];
 short	Linear;
 double	Value;
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  MotionKey2 {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 double	Value[1];
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  ColorKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 short	Value[3];
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  EcosystemKeyV1 {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 short	Line,			/* Snow or Tree line... */
	Skew,			/* Snow line skew... */
	SkewAz,			/* Azimuth for skew effect */
	RelEl,			/* Relative Elevation multiplier */
	MaxRelEl,		/* Max Relative Elevation */
	MinRelEl,		/* Min Relative Elevation */
	MaxSlope,		/* Max slope */
	MinSlope;		/* Min slope */
};

/*EXTERN*/ struct PACKED  EcosystemKey2V1 {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 short	Value[8];
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  EcosystemKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 float	Line,			/* Snow or Tree line... */
	Skew,			/* Snow line skew... */
	SkewAz,			/* Azimuth for skew effect */
	RelEl,			/* Relative Elevation multiplier */
	MaxRelEl,		/* Max Relative Elevation */
	MinRelEl,		/* Min Relative Elevation */
	MaxSlope,		/* Max slope */
	MinSlope,		/* Min slope */
	Density,		/* Tree density */
	Height;			/* Tree height */
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  EcosystemKey2 {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 float	Value[10];
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  CloudKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 float	Value[7];	/* Coverage, Density, StdDev, H, Alt, LatOff, LonOff*/
};

/* AF struct packed for AROS, 30.Dec.22 */
/*EXTERN*/ struct PACKED  WaveKey {
 short	KeyFrame,
	Group,
	Item;
 float	TCB[3];
 short	Linear;
 float	Value[4];	/* Amp, WhiteCapHt, LatOff, LonOff */ 
};

/*EXTERN*/ union KeyFrameV1 {
 struct MotionKey MoKey;
 struct ColorKey CoKey;
 struct EcosystemKeyV1 EcoKey;
 struct EcosystemKey2V1 EcoKey2;
};

/*EXTERN*/ union KeyFrame {
 struct MotionKey MoKey;
 struct MotionKey2 MoKey2;
 struct ColorKey CoKey;
 struct EcosystemKey EcoKey;
 struct EcosystemKey2 EcoKey2;
 struct CloudKey CldKey;
 struct WaveKey WvKey;
};

EXTERN union KeyFrame *KF, *UndoKF;

/*EXTERN */struct KeyTable {
 short NumKeys;
 union KeyFrame **Key;
 double *Val[10];
};

EXTERN struct KeyTable *KT, *SKT[3];

/*EXTERN*/ struct coords {
 double lat,lon,alt,
	x,y,z,q;
};

EXTERN struct coords VP, FP, DP, SP, SNB, PP[5];

/*EXTERN*/ struct clipbounds {
 short lowx, highx, lowy, highy;
};

/*EXTERN*/ struct Vertex {
 short	X, Y;
};

/*EXTERN*/ struct Box {
 struct Vertex Low, High;
};

/*EXTERN*/ struct poly4 {
 float X[5], Y[5];
};

/*EXTERN*/ struct poly3 {
 float X[3], Y[3];
};

/*EXTERN*/ struct ColorComponents {
 short Red, Grn, Blu;
};

EXTERN struct faces {
 float	slope,
	aspect,
	diplat,
	diplong;
} face;

/* changed float* to double* 101995 */
EXTERN struct VertexIndex {
double	*Lat, *Lon, *El, *RelEl, *Cld;
LONG 	*Use;  // MapTopoObject.c fread((char *)&Vertices, sizeof (LONG), 1, fVtx); // sizeof LONG, so here it must be LONG for AROS, too. AF 9.Feb.23
UBYTE 	*Pert;
UBYTE 	*Edge;
long	PSize;
short	Max[7];
} Vtx[7];

/*EXTERN*/ struct elmapheaderV101 {
			/* same as V1.02 in size and members */
			/* 1.02 allowed the elscale variable to reflect whether */
			/* the data was in meters, feet, etc... */
 LONG	rows,columns;
 double lolat,lolong,steplat,steplong,elscale;
 short  MaxEl, MinEl;
 LONG	Samples;
 float	SumElDif, SumElDifSq;
 short	*map;
 LONG	*lmap;
 LONG	size, scrnptrsize, fractalsize;
 float	*scrnptrx,
 	*scrnptry,
 	*scrnptrq;
 struct	faces *face;
 BYTE	*fractal;
 LONG	facept[3], facect, fracct, Lr, Lc;
 short	MapAsSFC, ForceBath;
 float	LonRange, LatRange;
};

EXTERN struct elmapheaderV101 *elmap, relelev, DeformMap;

/*EXTERN*/ struct GeoPoint {
 double 		Lat, Lon;
};

/*EXTERN*/ struct WaveData {
 short 			NumKeys, NumWaves, KT_MaxFrames;
 long 			KFSize;
 double			Amp, WhiteCapHt, LatOff, LonOff;
 struct Wave *		Wave;
 union KeyFrame *	WaveKey;
 struct KeyTable *	KT;
};

/*EXTERN*/ struct WaveData2 {
 short 			NumKeys, NumWaves, KT_MaxFrames;
 long 			KFSize;
 double			Value[4];
 struct Wave *		Wave;
 union KeyFrame *	WaveKey;
 struct KeyTable *	KT;
};

EXTERN struct WaveData *Tsunami;
 
/*EXTERN*/ struct Wave {
 struct Wave *		Next;
 short			NumKeys;
 double 		Amp, Length, Velocity;
 struct GeoPoint 	Pt;
};

/*EXTERN*/ struct CloudLayer {
 struct CloudLayer *	Next, *Prev;
 double  		Covg, Dens, Illum, Alt;
};

/*EXTERN*/ struct CloudData {
 long   		RandSeed, Rows, Cols, PlaneSize, KFSize;
 short			NumKeys, CloudType, NumLayers, Dynamic,
			KT_MaxFrames;
 double			Coverage, Density, StdDev, H, Alt, LatOff, LonOff;
 double			Lat[2], Lon[2], AltDiff;
 struct elmapheaderV101 Map;
 struct WaveData *	WD;
 struct CloudLayer *	Layer;
 struct KeyTable *	KT;
 union KeyFrame *	CloudKey;
 float *		CloudPlane;
};

/*EXTERN*/ struct CloudData2 {
 long   		RandSeed, Rows, Cols, PlaneSize, KFSize;
 short			NumKeys, CloudType, NumWaves, NumLayers, Dynamic,
			KT_MaxFrames;
 double			Value[12];
 struct elmapheaderV101 Map;
 struct Wave *		Wave;
 struct CloudLayer *	Layer;
 struct KeyTable *	KT;
 union KeyFrame *	CloudKey;
 float *		CloudPlane;
};

/*EXTERN*/ struct DEMData {
 long			Rows, Columns;
 struct GeoPoint	Pt[4];
 double 		LatStep, LonStep, ElScale;
 short  		MaxEl, MinEl;
 long			Samples;
 float			SumElDif, SumElDifSq;
 struct CloudData *	Cloud;
 struct MotionKey *	MoKey;
 struct ColorKey *	CoKey;
 short *		map;
 long *			lmap;
 long			size;
 float *		scrnptrx,
 	*		scrnptry,
 	*		scrnptrq;
 long			scrnptrsize;
 short			MapAsSFC;
};

/*EXTERN*/ struct MaxMin3 {
 double MxLon, MnLon, MxLat, MnLat, MxEl, MnEl;
};

/* AF struct packed for AROS, 12.Dec.22 */
/*EXTERN*/ struct PACKED  vectorheaderV100 {
 char	Name[10];
 LONG	points;
 SHORT	elevs;
 double avglat, avglon, avgelev, elscale;
 SHORT  MaxEl, MinEl;
};

/*EXTERN*/ struct DEMInfo {
 short	Code,
	Zone,
	HUnits,
	VUnits;
 double Res[3],
	UTM[4][2],
	LL[4][2];
};

/*EXTERN*/ struct UTMLatLonCoords {
 double	North, East, Lat, Lon;
 double	RefNorth[4], RefEast[4], RefLat[4], RefLon[4];
 double a, e_sq, k_0, M_0, lam_0, e_pr_sq, e_sq_sq, e_1, e_1_sq;
};

/*EXTERN*/ struct AlbLatLonCoords {
 double	North, East, Lat, Lon;
 double	RefNorth[4], RefEast[4], RefLat[4], RefLon[4];
 double ProjPar[8];
 double a, e_sq, e, two_e, C, n, rho_0, lam_0;
};

/*EXTERN*/ union MapProjection {
 struct UTMLatLonCoords UTM;
 struct AlbLatLonCoords Alb;
};

/*EXTERN*/ struct USGS_DEMHeader {
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
};

/*EXTERN*/ struct USGS_DEMProfileHeader {
 char	Row[7],
	Column[7],
	ProfRows[7],
	ProfCols[7],
	Coords[2][25],
	ElDatum[25],
	ElMin[25],
	ElMax[25];
};

/*EXTERN*/ struct boundingbox {
 double	lat[13],lon[13];
 short	elev[13];
 float	scrnx[13],
	scrny[13],
	scrnq[13];
};

EXTERN struct boundingbox *BBox;

EXTERN short LandBounds
#ifdef MAIN
= 1
#endif
;
EXTERN short BoxBounds
#ifdef MAIN
= 0
#endif
;
EXTERN short ProfileBounds
#ifdef MAIN
= 0
#endif
;
EXTERN short GridBounds
#ifdef MAIN
= 0
#endif
;
EXTERN short CompassBounds
#ifdef MAIN
= 1
#endif
;

EXTERN struct ViewCenter {
 float  x[8],
	y[8],
	q[8];
} *viewctr;

EXTERN struct SmallWindow {
 struct Window *win;
 ULONG  Signal;
 char   Title[36];
 struct clipbounds cb;
 short  width;
 struct elmapheaderV101 elmap;
 struct Animation MoPar;
 struct Palette CoPar;
 union  Environment EcoPar;
 struct Settings settings;
} *SmWin[10];

EXTERN struct InterAction {
 short shifting, shiftx, shifty, startx, starty, fixX, fixY, fixZ, 
	gridpresent, recompute, mult, newgridsize, scrnwidth, scrnheight,
	CompHCenter, CompVCenter, CompRadius, Digitizing, WindowCounter;
 long	button, SunX, SunY, SunRad, MoonX, MoonY, MoonRad;
 double wscale, hscale;
 struct clipbounds cb;  
} *IA;

/*EXTERN*/ struct InterpPoints {
 double lowx, midx, hix, lowel, midel, hiel, pty1, pty2;
};

EXTERN struct FocusProfiles {
 struct elmapheaderV101 *map;
 short row;
 short col;
} *FocProf;

EXTERN struct MapData {
 short ptsdrawn, camptx[2], campty[2], focptx[4], focpty[4], focctrx, focctry,
	hazerad[2], viewlineptx[2], viewlinepty[2], hazectrx[2], hazectry[2],
	LastWinX, LastWinY, LastSeconds, LastMicros, LastObject, sunctrx, sunctry;
 float viewlineslope[2], viewlineaz[2];
 APTR MAPC, MapMsg[2], ClearFrame, MapAutoClear, MapClr, MapDraw, MapRefine,
  MapObject, MapTopo, MapVec, MapInter, MapZoom, MapPan, MapCenter, MapAuto,
  Scale, ScaleLess, ScaleMore, Lat, LatLess, LatMore, Lon, LonLess, LonMore,
  Exag, ExagLess, ExagMore, MapEco, MapDither, MapStyle, MapColor, AlignCheck,
  VisualInfo, AlignWin, FloatStr[4], IntStr[4], MapAlign, Register, ScaleStuff,
  BT_EcoLegend;
  struct Menu *MenuStrip;
  struct Window *MAPCWin;
} *MP;

EXTERN struct Box AlignBox;

EXTERN APTR UnderConst;
// UnderConstOK; static AF
EXTERN APTR ModControlWin;

EXTERN struct Viewshed {
 short 	*Map,
	*View;
 long 	Mapsize;
 short 	Width,
	Height;
 struct clipbounds cb;
} *VS;

struct lineseg {
 double firstx, firsty, lastx, lasty, oldx, oldy;
};

/*EXTERN*/ struct QCvalues {
 long	compval1, compval2, compval3;
};

/*EXTERN*/ struct FaceData {
 double El[3], Lat[3], Lon[3];
};

/*EXTERN*/ struct TreeModel {
 long	Ht, Stems;
 short	Class, Red, Grn, Blu;
 double	Pct;
};

EXTERN struct ForestModel {
 long Items;
 struct TreeModel *TM;
} FM[ECOPARAMS];

/*EXTERN*/ struct DEMConvertData {
 short	FormatCy[10],
	FormatInt[5],
	OutputMaps[2],
	WrapLon,
	VSOperator,
	ScaleType,
	SplineConstrain,
	ActiveReplace, // AF, 19.Sep.2023
	ActiveFC[2],
	Crop[4];
 float	LateralScale[4],
	VertScale[9],
	FloorCeiling[2],
	MaxMin[2],
 	Replace[2];   // AF, 18.Sep.2023
 char	NameBase[24],
	OutputDir[256];	
};

struct DEMInterpolateData {
 double elvar,
	flatmax;
 struct FileRequester *FrFile;
 char 	elevpath[256],
	elevfile[32];
 char 	*pattern;
};

/*EXTERN*/ struct DEMExtractData {
 struct USGS_DEMHeader USGSHdr;
 struct USGS_DEMProfileHeader ProfHdr;
 double SELat,
	SELon;
 struct FileRequester *FrFile;
 char 	elevpath[256],
	elevfile[32];
 char 	*pattern;
 struct DEMInfo *Info;
 struct	UTMLatLonCoords *Convert;
 double MaxNorth[2],
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
 short  *UTMData,
	*LLData;
};

/*EXTERN*/ struct DLGInfo {
 long 	IDNum, StartNode, EndNode;
 short 	LeftArea, RightArea, Pairs, Attrs, Text, MajorAttr, MinorAttr,
	Color, LineWidth;
 char 	Name[16], Layer1[3], Layer2[4];
};

/*EXTERN*/ struct VelocityDistr {
 short PtsIn, PtsOut, EaseIn, EaseOut, Base;
 double *Lat, *Lon, *Alt;
};

/*EXTERN*/ struct RenderAnim {
 char AnimPath[256], AnimName[32];
 long OutToFile, StartFrame, EndFrame, Width, Height, FrameStep;
};

/*EXTERN*/ struct TerrainControl {
 float Lat, Lon, El;
};

/*EXTERN*/ struct datum
{
 double       values[3];
 struct datum *nextdat;
};

/*EXTERN*/ struct MinimumDistance {
 float Dist, El;
};

EXTERN struct Color_Map {
 long Rows, Cols, Size;
 double North, South, East, West, StepLat, StepLon;
} *CMap;
 
EXTERN USHORT Colors[16]
#ifdef MAIN
={
 0xddf,0x000,0xfff,0xeef,
 0x99c,0x4a5,0xbbe,0x583,
 0x474,0x464,0x454,0x344,
 0xcca,0xaa8,0x887,0x555
 }
#endif /* MAIN */
;

EXTERN USHORT AltPen[16]
#ifdef MAIN
={ 1, 2, 3, 0, 6, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
#endif
;

EXTERN USHORT AltColors[16]
#ifdef MAIN
={
#ifndef DAVE_WARNER
 0x89b,	/* 0, gray-blue */
 0x000,	/* 1, black */
 0xddd,	/* 2, almost white */
#else /* DAVE_WARNER */
 0x000,	/* 0, gray-blue, now black */
 0xfff,	/* 1, black, now white */
 0xbbb,	/* 2, almost white, now greyish */
#endif /* DAVE_WARNER */
 0xb10, /* 3, red */
 0x348,	/* 4, dark blue */
 0x392,	/* 5, green */
 0x37c,	/* 6, med blue */
 0xdd2,	/* 7, yellow */
 0xfff,	/* 8-15, gray scale */
 0xddd,
 0xbbb,
 0x999,
 0x777,
 0x555,
 0x333,
 0x111
 }
#endif /* MAIN */
;

//#ifdef VISTA_SCHEME
//
//EXTERN USHORT PrintColors[16]
//#ifdef MAIN
//={
//#ifndef DAVE_WARNER
// 0x89b,	/* 0, gray-blue */
// 0x000,	/* 1, black */
// 0xddd,	/* 2, almost white */
//#else /* DAVE_WARNER */
// 0x000,	/* 0, gray-blue, now black */
// 0xfff,	/* 1, black, now white */
// 0xbbb,	/* 2, almost white, now greyish */
//#endif /* DAVE_WARNER */
// 0xb10,
// 0x348,
// 0x392,
// 0x37c,
// 0xdd2,
// 0xfff,	/* VistaLike color scheme */
// 0xbbb,
// 0x777,
// 0xd95,
// 0x940,
// 0x7b7,
// 0x090,
// 0x060
// }
//#endif /* MAIN */
//;
//
//#endif /* VISTA_SCHEME */
//
//
//EXTERN USHORT PrintColors[16]
//#ifdef MAIN
//={
//#ifndef DAVE_WARNER
// 0x89b,	/* 0, gray-blue */
// 0x000,	/* 1, black */
// 0xddd,	/* 2, almost white */
//#else /* DAVE_WARNER */
// 0x000,	/* 0, gray-blue, now black */
// 0xfff,	/* 1, black, now white */
// 0xbbb,	/* 2, almost white, now greyish */
//#endif /* DAVE_WARNER */
// 0xb10,
// 0x348,
// 0x392,
// 0x37c,
// 0xdd2,
// 0xfff,	/* lighten color gradient */
// 0xeee,
// 0xddd,
// 0xccc,
// 0xbbb,
// 0xaaa,
// 0x999,
// 0x888
// }
//#endif /* MAIN */
//;


// EXTERN USHORT PrimaryColors[16]  static AF
// #ifdef MAIN
// ={
// #ifndef DAVE_WARNER
//  0x89b,	/* 0, gray-blue */
//  0x000,	/* 1, black */
//  0xddd,	/* 2, almost white */
// #else /* DAVE_WARNER */
//  0x000,	/* 0, gray-blue, now black */
//  0xfff,	/* 1, black, now white */
//  0xbbb,	/* 2, almost white, now greyish */
// #endif /* DAVE_WARNER */
//  0xb10,
//  0x348,
//  0x392,
//  0x37c,
//  0xdd2,
//  0xfff,
//  0xf00,
//  0xf70,
//  0xff0,
//  0x0f0,
//  0x00f,
//  0xf0f,
//  0x000
// }
// #endif /* MAIN */
// ;


EXTERN __far char *typename[]
#ifdef MAIN
= {"Water", "Snow", "Rock",/* "Strata",*/ "Ground","Conifer",
		"Decid", "Low Veg", "Snag", "Stump", NULL}
#endif /* MAIN */
;

EXTERN __far char *varname[]
#ifdef MAIN
= { 
 "Camera Altitude",
 "Camera Latitude",
 "Camera Longitude",
 "Focus Altitude",
 "Focus Latitude",
 "Focus Longitude",
 "Center X",
 "Center Y",
 "Bank",
 "Earth Rotation",
 "Scale",
 "View Arc",
 "Flattening",
 "Datum",
 "Vertical Exag",
 "Sun Light Lat",
 "Sun Light Lon",
 "Horizon Line",
 "Horizon Point",
 "Horizon Stretch",
 "Haze Start",
 "Haze Range",
 "Shade Factor",
 "Fog None",
 "Fog Full",
 "Z Minimum",
 "Sun Lat",
 "Sun Lon",
 "Sun Size",
 "Moon Lat",
 "Moon Lon",
 "Moon Size",
 "Reflection %",
 NULL
}
#endif /* MAIN */
;

EXTERN __far char *StdMesg[]
#ifdef MAIN
= {
 " Out of memory!",
 " Open file failed!",
 " Read file failed!",
 " Writing to file failed!",
 " Wrong file type!",
 " Illegal instruction!",
 " Illegal value!",
 " File not loaded!",
 " Open file failed.",
 " Read file failed.",
 " Wrong file type.",
 " Illegal instruction.",
 " Illegal value.",
 " Module not implemented.",
 " GUI not implemented.",
 " Parameter file loaded.",
 " Parameter file saved.",
 " Database file loaded.",
 " Database file saved.",
 " DEM file loaded.",
 " DEM file saved.",
 " Vector file loaded.",
 " Vector file saved.",
 " Image file loaded.",
 " Image file saved.",
 " Color Map file loaded.",
 " Color Map file saved.",
 " File not loaded.",
 "\33t",
 " Mapping Module.",
 " Directory not found!",
 " Open window failed!",
 "",
 "",
 " Incorrect file size!",
 " Open window failed.",
 " Incorrect file size!",
 " Incorrect file version!",
 " Relative Elevation File Saved.",
 "",
 " Vector objects loaded.",
 " Project file loaded.",
 " Project file saved.",
 " Directory List loaded",
 " Incorrect file version!",
 " Render time for frame",
 " Render time for anim,",
 ""
}
#endif
;

EXTERN __far char ErrMagnitude[]
/* 200="ERR:", 128="WNG:", 100="MSG:", 50="DTA:", 0="" */
#ifdef MAIN
= {
 200,
 200,
 200,
 200,
 200,
 200,
 200,
 200,
 128,
 128,
 128,
 128,
 128,
 128,
 128,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 100,
 0,
 100,
 200,
 200,
 100,
 50,
 200,
 128,
 128,
 128,
 100,
 128,
 100,
 100,
 100,
 100, 	/* MSG Directory List loaded */
 200,	/* ERR file version */
 100,	/* MSG render time */
 100,	/* MSG total render time */
 200	/* ERR Null */
}
#endif
;

/*EXTERN FILE *RndrData;*/


EXTERN __far char LogMesg[MAXLOGITEMS][80];

EXTERN UBYTE red,green,blue;
//ptred,ptgreen,ptblue;    static AF
EXTERN UBYTE *colmap[3];
EXTERN UBYTE *cloudmap;
EXTERN UBYTE *NoiseMap;
//EXTERN __far char fieldname[14][20];  static AF
EXTERN __far char str[256],paramfile[32],
	framepath[256],temppath[256],dbasepath[256],parampath[256],
	path[256],dirname[256],ILBMname[256],
//	ILBMnum[32],  static AF
	dbasename[32],
	statname[32],statpath[256],linepath[256],linefile[32],
	backgroundpath[256], zbufferpath[256], backgroundfile[32],
	zbufferfile[32], colormappath[256], colormapfile[32], projectpath[256],
	projectname[32], framefile[32], tempfile[32], modelpath[256],
	cloudpath[256], cloudfile[32], wavepath[256], wavefile[32],
	deformpath[256], deformfile[32], imagepath[256], sunfile[32],
	moonfile[32], pcprojectpath[256], pcframespath[256], altobjectpath[256];
EXTERN UBYTE *bitmap[3];
EXTERN USHORT *bytemap;
EXTERN UBYTE *ReflectionMap;
EXTERN UBYTE *SubPix, *TreePix;

EXTERN short *Edge1,*Edge2,
      render,horline,horpt,length[14],wide,high,oshigh,
      DBaseRecords,NoOfObjects,RenderObjects,
//      RecordLength,   static AF
//      NoOfFields,  static AF
      NoOfSmWindows,
//      WindowNumber,  static AF
      fixfocus,frame,col,OBN,cmap,dir,
      treedraw,undertreedraw,
//      polyct[10],  // static AF
      paramsloaded,dbaseloaded,NoOfElMaps,item[3],
      aliasred,aliasgreen,aliasblue,altred,altgreen,altblue,LogItem, IA_GridSize,
      IA_GridStyle, IA_Movement, KT_MaxFrames, ia_Top, ia_Left, ia_Width,   // AF: AROS IA_Top usw gibt es in imageclass.h
      IA_AnimStart, IA_AnimEnd,
      //IA_AnimStep,  static AF
      ia_Height, IA_CompTop, IA_CompLeft, IA_CompWidth, IA_CompHeight, IA_AutoDraw,
      ColMax, Clouds, DB_Mod, Par_Mod, Proj_Mod, Mod_Warn, EcoClass,
//      UndoKeyFrames,  static AF
      RenderTaskPri,
      //SaveAscii,  static AF
      RenderSize, Reflections;

EXTERN short ReportMesg[4]
#ifdef MAIN
= {1, 1, 1, 1}
#endif
;
EXTERN short IA_GBDens
#ifdef MAIN
=2
#endif
;
EXTERN short IA_Sensitivity
#ifdef MAIN
=10
#endif
;
EXTERN short showX
#ifdef MAIN
=1
#endif /* MAIN */
;
EXTERN short showY
#ifdef MAIN
=1
#endif /* MAIN */
;
// Not used, AF, 21.Jun22, found with --gc-sections,--print-gc-sections
//EXTERN short showhaze
//#ifdef MAIN
//=1
//#endif /* MAIN */
//;
EXTERN short *CamPathElev;
EXTERN short *FocPathElev;
EXTERN __far short RenderList[1000][2];
EXTERN __far short *AltRenderList;

EXTERN ULONG WCS_Signals,
//InterWind0_Sig,  satatic AF
InterWind2_Sig;
// MapWind0_Sig; static AF
	// MapWind3_Sig; static AF
//	RenderWind0_Sig;  static AF

EXTERN LONG __stack
#ifdef MAIN
 = 8192
#endif /* MAIN */
;
EXTERN LONG /*long*/ *QCmap[3];  // AF, 3.Apr.23  32 Bit values!

EXTERN long extrarand,maxfract,fracount[10],
 //lastfacect,  static AF
     xx[3],yy[3],pts,hseed,b,randomint,
     redrand,greenrand,bluerand,CamPathPts,FocPathPts,
     drawoffsetX,drawoffsetY,DMod,SubPixArraySize,TreePixArraySize,
     EdgeSize, ElmapSize, BBoxSize, VtxNum;

#ifdef ENABLE_STATISTICS
EXTERN long samples;
EXTERN __far long statcount[37][20];
#endif /* ENABLE_STATISTICS */

EXTERN __far long scrnrowzip[2000];
EXTERN long ecocount[ECOPARAMS];
EXTERN long zbufsize,bmapsize,QCmapsize,
//AltRenderListSize,  static AF
KFsize, UndoKFsize;

EXTERN FILE *fvector;

/* changed to double from float 101995 */
EXTERN double qqq,qmin,
//ptqq[3],  static AF
ptx[3],pty[3];

#ifdef ENABLE_STATISTICS
EXTERN float meanaspect, meanslope, meanel;
#endif /* ENABLE_STATISTICS */
EXTERN float weight[11][11];
/* changed to double from float 10/21/95 and back to float on 12/2/95 */
EXTERN float *zbuf;

//EXTERN float *CosTable; static AF
// *SinTable; static AF
// *ASinTable; // static AF
//*ACosTable; //static AF

//EXTERN long TrigTableEntries   //static AF geht nicht, Aerger mit WCS.c Assembler????? behoben mit gcc vom 24.10.2021
//#ifdef MAIN
//= 361
//#endif
//;

/* changed to double from float 101995 */
EXTERN double polyq[10][3];
EXTERN double polyslope[10][3];
EXTERN double polyrelel[10][3];
EXTERN double polydiplat[10][3];
EXTERN double polydiplon[10][3];
EXTERN double polylat[10][3];
EXTERN double polylon[10][3];
EXTERN double polyx[10][3];
#ifdef SWMEM_FAST_INLINE
#warning SWMEM_FAST_INLINE needs volatile at the moment
   volatile
#endif
EXTERN double polyy[10][3];    // this one must be volatile if swmem() is inlined !?
EXTERN double polyel[10][3];
EXTERN double polycld[10][3];
EXTERN float *QCcoords[2];
EXTERN float *ElevationMap;
EXTERN float *SlopeMap;

EXTERN double fractperturb[10];

EXTERN __far float parambounds[USEDMOTIONPARAMS][2]
#ifdef MAIN
= {
 {FLLARGENUM,-EARTHRAD},	/* Camera Altitude */
 {90.0,      -90.0},	/* Camera Latitude */
 {FLLARGENUM,-FLLARGENUM},/* Camera Longitude */
 {FLLARGENUM,-EARTHRAD},	/* Focus Altitude */
 {FLLARGENUM,-FLLARGENUM},/* Focus Latitude */
 {FLLARGENUM,-FLLARGENUM},/* Focus Longitude */
 {32000,	    -32000},	/* Center X */
 {32000,	    -32000},	/* Center Y */
 {FLLARGENUM,-FLLARGENUM},/* Bank */
 {FLLARGENUM,-FLLARGENUM},/* Earth Rotation */
 {FLLARGENUM, .0001},	/* Scale Factor */
 {179.0,	     1.0},	/* View Arc */
 {100.0,	    -100.0},	/* Flattening */
 {FLLARGENUM,-FLLARGENUM},/* Datum */
 {100.0,	    -100.0},	/* Vertical Exageration */
 {90.0,      -90.0},	/* Sun Light Latitude */
 {FLLARGENUM,-FLLARGENUM},/* Sun Light Longitude */
 {100.0,	     0.0},	/* Horizon Line */
 {FLLARGENUM,-FLLARGENUM},/* Horizon Point */
 {100.0,	    -100.0},	/* Horizon Stretch */
 {FLLARGENUM,-FLLARGENUM},/* Haze Start */
 {FLLARGENUM,-FLLARGENUM},/* Haze Range */
 {2.0,	     0.0},	/* Shade Factor */
 {FLLARGENUM,-FLLARGENUM},/* Fog None */
 {FLLARGENUM,-FLLARGENUM},/* Fog Full */
 {FLLARGENUM, 0.0},	/* Q Minimum */
 {90.0,	    -90.0},	/* Sun Lat */
 {FLLARGENUM,-FLLARGENUM},/* Sun Lon */
 {5000.0,     0.0},	/* Sun Size */
 {90.0,	    -90.0},	/* Moon Lat */
 {FLLARGENUM,-FLLARGENUM},/* Moon Lon */
 {5000.0,     0.0},	/* Moon Size */
 {100.0,	     0.0},	/* Reflection % */
}
#endif /* MAIN */
;

EXTERN double // redsky,greensky,bluesky, static AF
horstretch;
EXTERN double *Banking;
EXTERN double qmax,cosviewlat,sunlat,sunlong,diplat,diplong,
       facelat,facelong,el,
       //faceel,  Mapword.c, not used in WCS-program AF
       slope,aspect,CenterX,CenterY,
       ralt,vertscale,horscale,sunangle,sunfactor,fade,
       //colavg,  static, AF
       sunshade,
       seadepthfact,
       redsun,greensun,bluesun,Random,dlat,dlong,dslope,
       //flred,flgreen,flblue,   static AF
       maxdlat,mindlat,maxdlong,mindlong,transpar,PtrnOffset,HalfPtrnOffset,
       relel,relfactor,treehtfact,treerand,treerand2,treerand3,treerand4,
       cloudcover,xrot,yrot,zrot,cosxrot,cosyrot,coszrot,
       sinxrot,sinyrot,sinzrot,
       //elface[3],  static AF
       latface[3],longface[3],incr[3],
       azimuth,oldazimuth,focdist,treeheight,
       fog, fogrange, FloatCol;
EXTERN double SeaLevel, MaxSeaLevel, MaxWaveAmp, ReflectionStrength;
	//VertSunFact, HorSunFact, HorSunAngle;  static AF

EXTERN double *DTable;

/* stuff for nngridr */

/* NOTE: some of these structure pointers need to be initialized to 0 */

/*EXTERN*/ struct simp
{  int          vert[3];
   double       cent[3];
   struct simp  *nextsimp;
};

/*EXTERN*/ struct temp
{  int          end[2];
   struct temp  *nexttemp;
};

/*EXTERN*/ struct neig
{  int          neinum;
   double       narea;
   double       coord;
   struct neig  *nextneig;
};

/*EXTERN*/ struct NNGrid {
 double  xstart, ystart, xterm, yterm, horilap, 
        vertlap, arriba, bI, bJ, nuldat,
        **points, **joints, wbit, maxxy[2][3], 
        maxhoriz, aaa, bbb, ccc, det, 
        work3[3][3], xxxx, sumx, sumy, sumz, 
        sumx2, sumy2, sumxy, sumxz, sumyz, 
        asum, pi, piby2, piby32, rad2deg, 
        bigtri[3][3], magx, magy, magz, xspace, yspace;
 int     igrad, x_nodes, y_nodes, non_neg, 
        comma, ichoro, densi, extrap,
        ioK, ixmag, iymag, izmag, magcnt, 
        ccolor, updir, southhemi,  
        datcnt, datcnt3, numtri, imag, numnei, 
        ext, *jndx, neicnt, goodflag, scor[3][2],
	PointMatxRows, PointMatxCols, JointMatxRows, JointMatxCols,
	IntVectCols, GridSize;
 char    grd_file[256], dat_file[256]; 
 struct datum    *rootdat, *curdat, *holddat;
 struct simp     *rootsimp, *cursimp, *lastsimp, *prevsimp;
 struct temp     *roottemp, *curtemp, *lasttemp, *prevtemp;
 struct neig     *rootneig, *curneig, *lastneig;
 float *Grid;
};

#endif /* GIS_GIS_H */

#ifndef GIS_GUI_H
#define GIS_GUI_H

extern struct Image AboutScape, CompRose, DatabaseMod, DataOpsMod, MappingMod,
  EditingMod, ImageOpsMod, RenderMod,/* EXIT,*/ EC_PalGrad, Xenon, Gary,
  EC_Button2, EC_Button3, EC_Button4, EC_Button5, EC_Button6, EC_Button7,
  EC_Button8, EC_Button9,
  DangerSign;

EXTERN __chip UWORD WaitPointer[]
#ifdef MAIN
 = {
   0x0000,   0x0000,

   0x0400,   0x07c0,
   0x0000,   0x07c0,
   0x0100,   0x0380,
   0x0000,   0x07e0,
   0x07c0,   0x1ff8,
   0x1ff0,   0x3fec,
   0x3ff8,   0x7fde,
   0x3ff8,   0x7fbe,
   0x7ffc,   0xff7f,
   0x7efc,   0xffff,
   0x7ffc,   0xffff,
   0x3ff8,   0x7ffe,
   0x3ff8,   0x7ffe,
   0x1ff0,   0x3ffc,
   0x07c0,   0x1ff8,
   0x0000,   0x07e0,

   0x0000,   0x0000,
 }
#endif /* MAIN */
;

EXTERN __chip UWORD CopyPointer[]
#ifdef MAIN
 = {
   0x0000,   0x0000,

   0x0000,   0xEEE9,
   0x0000,   0x8AAF,
   0x0000,   0x8AE2,
   0x0000,   0xEE86,
   0x0000,   0x0000,
   0xC000,   0xCEE0,
   0x7000,   0xB4A0,
   0x3800,   0x4CA0,
   0x3B00,   0x47E0,
   0x1FC0,   0x20C0,
   0x1FC0,   0x2000,
   0x0F00,   0x1100,
   0x0D80,   0x1280,
   0x04C0,   0x0940,
   0x0460,   0x08A0,
   0x0020,   0x0040,

   0x0000,   0x0000
   }
#endif /* MAIN */
;

EXTERN __chip UWORD SwapPointer[]
#ifdef MAIN
 = {
   0x0000,   0x0000,

   0xC000,   0xC000,
   0x7000,   0xB000,
   0x3F80,   0x4F80,
   0x3C00,   0x4380,
   0x1C58,   0x27D8,
   0x1954,   0x2E9C,
   0x1302,   0x1D0E,
   0x0581,   0x1A87,
   0x0480,   0x1903,
   0x0400,   0x1C03,
   0x0204,   0x0E07,
   0x0138,   0x073E,
   0x0020,   0x033C,
   0x0000,   0x0038,
   0x0000,   0x003C,
   0x0000,   0x0000,

   0x0000,   0x0000
   }
#endif /* MAIN */
;

EXTERN struct Window *MCPWin;

//EXTERN struct ColorSpec NewAltColors[]   static AF
//#ifdef MAIN
// ={
//  {0x00, 0x08, 0x09, 0x0b},
//  {0x01, 0x00, 0x00, 0x00},
//  {0x02, 0x0d, 0x0d, 0x0d},
//  {0x03, 0x0b, 0x01, 0x00},
//  {0x04, 0x03, 0x04, 0x08},
//  {0x05, 0x03, 0x09, 0x02},
//  {0x06, 0x03, 0x07, 0x0c},
//  {0x07, 0x0d, 0x0d, 0x02},
//  {0x08, 0x0f, 0x0f, 0x0f},
//  {0x09, 0x0d, 0x0d, 0x0d},
//  {0x0a, 0x0b, 0x0b, 0x0b},
//  {0x0b, 0x09, 0x09, 0x09},
//  {0x0c, 0x07, 0x07, 0x07},
//  {0x0d, 0x05, 0x05, 0x05},
//  {0x0e, 0x03, 0x03, 0x03},
//  {0x0f, 0x01, 0x01, 0x01},
//  {-1, 0, 0, 0}
//  }
//#endif /* MAIN */
//; /* NewAltColors */
//
//EXTERN WORD PenSpec[]   static AF
//#ifdef MAIN
// ={
//  6,	/* cycle dropdowns text normal			 */
//  2,	/* cycle dropdowns text highlighted		 */
//  1,	/* text, window title inactive,
//	cycle drop downs background normal		 */
//  2,	/* shine					 */
//  4,	/* shadow, active gadget outline		 */
//  6,	/* window frame fill active			 */
//  1,	/* window title active				 */
//  0,	/* cycle dropdowns background highlighted	 */
//  3,	/* special text					 */
//  ~0 	/* ???						 */
//  }
//#endif /* MAIN */
//;

EXTERN APTR app;
EXTERN APTR AboutWin, CreditWin, CreditList;

#ifdef XENON_DOESNT_LIKE_THIS
EXTERN APTR CreditWinOK, InfoWinOK;
#endif /* XENON_DOESNT_LIKE_THIS */

/*EXTERN*/ struct WindowKeyStuff {
  short IsKey, PrevKey, NextKey, KeysExist, ItemKeys, Frame,
	Linear, Group, Item, NumValues, Precision, StrBlock, PropBlock;
  float TCB[3];
};

/*EXTERN*/ struct GUIKeyStuff {
  APTR FramePages,
	BT_MakeKey, BT_UpdateKeys, BT_AllKeys, BT_NextKey, BT_PrevKey, BT_DeleteKey,
	BT_KeyScale, BT_TimeLines, BT_DeleteAll, Str[1], StrArrow[2];
};

extern struct WaveWindow *WVWin[2];

EXTERN APTR InfoWin, InfoWinFlush, InfoTime, InfoDate, InfoARexx,
 InfoDataBase, InfoPar, InfoScreenMode, InfoFastAvail, InfoChipAvail,
 InfoFastLarge, InfoChipLarge, InfoMapTopos, InfoInterTopos;

/*EXTERN*/ struct PaletteItem {
  long red, grn, blu, hue, sat, val;
};

EXTERN struct EditModWindow {
  APTR EditWindow, CY_Layout, BT_EdMoPar, BT_EdCoPar, BT_EdEcoPar,
	BT_EdSettings, BT_EdClouds, BT_EdWaves, BT_Defaults, BT_ExportMo;
  short Layout;
} *EP_Win;

EXTERN struct DatabaseModWindow {
  APTR DatabaseWindow, CY_Layout, BT_Load, BT_Append, BT_Create,
	BT_Edit, BT_SaveAs, BT_DirList;
  short Layout;
} *DB_Win;

EXTERN struct DatabaseEditWindow {
  APTR DatabaseEditWin, BT_Load, BT_Append, BT_Create,
	BT_Save, BT_Search, BT_Sort,
	BT_New, BT_Add, BT_Remove, BT_LayerSel[2], BT_LayerOn[2],
	BT_LayerOff[2], BT_Name,
	Str[4], IntStr[7], CY_Line, CY_Spec, Prop[3], Check, Arrow[4][2],
	LgArrow[2], PointTxt, LS_List, BT_Label, BT_Settings[3];
  char **DBName;
  long DBNameSize, MaxDBItems;
  struct Window *Win;
  USHORT Colors[16], Block[12];
} *DE_Win;

EXTERN struct DirListWindow {
  APTR DirListWin, DefaultTxt, BT_Cancel, BT_Apply, BT_ReadOnly, BT_Load,
	BT_Add, BT_Remove, BT_Swap, BT_Move, BT_Default, LS_List;
  char **DLName;
  char Dirname[256];
  short Proj_Mod;
  long DLNameSize, MaxDLItems;
  struct DirList *DLCopy;
  struct Window *Win;
} *DL_Win;

EXTERN struct DataOpsModWindow {
  APTR DataOpsWindow, CY_Layout, BT_Extract, BT_Convert, BT_ImportDLG,
	BT_ImportDXF, BT_ImportWDB, BT_RelElev, BT_InterpMap, BT_ExportLWOB;
  short Layout;
} *DO_Win;

EXTERN struct DEMConvertWindow {
  APTR ConvertWin, Cycle[10], VSRegister, FileSizeTxt, FileNameStr, DBaseNameStr,
	FormatIntStr[5], LatScaleStr[4], VertScaleStr[9], OutputMapStr[2],
	FloatStr[2], FloorCeilingCheck[2],ReplaceCheck[1],CropStr[4],ReplaceStr[2], MinValTxt, MaxValTxt, // AF: 18.Sep.23, added ReplaceStr
	OutputMapArrow[2][2], ScaleCycle, BT_GetFile, BT_Convert,
	WrapCheck, ConstraintCheck, BT_GetOutDir, BT_Test, OutDirStr;
  struct DEMConvertData *DEMData;
  char InPath[256], InFile[32];
  struct Window *Win;
} *DC_Win;

EXTERN struct DEMInterpolateWindow {
  APTR InterpWin, FlatMaxStr, ElVarStr, BT_GetFiles, DirTxt, SumFilesTxt,
	BT_Interpolate;
  struct DEMInterpolateData *DEMInterp;
} *DI_Win;

EXTERN struct DEMExtractWindow {
  APTR ExtractWin, BT_GetFiles, DirTxt, SumFilesTxt, LatStr, LonStr,
	Txt[14], ResTxt[3],
	CoordTxt[4][2], ProjTxt[15], ProfTxt[7], ProfCoordTxt[2],
	BT_Extract;
  struct DEMExtractData *DEMExtract;
} *DM_Win;

EXTERN struct StatusLogWindow {
  APTR LogWindow, LS_List, BT_Hide, BT_Clear, BT_Quit;
  int Hiding;
} *Log_Win;

EXTERN struct EcoPalWindow {
  APTR EcoPalWin, Str[6], CoStr[6], PropArrow[6][2], StrArrow[5][2], Prop[6],
	BT_MakeKey, BT_UpdateKeys, BT_UpdateAll, BT_NextKey, BT_PrevKey,
	BT_DeleteKey, BT_KeyScale,
	BT_TimeLines, BT_Copy, BT_Swap, BT_Insert, BT_Remove,
	BT_Apply, BT_Cancel, BT_DeleteAll,
	Nme[4],	BT_Col[4], LS_List, FramePages, BT_Settings[3];
  char Colorname[COLORPARAMS][30];
  char *CName[COLORPARAMS + 1];
  struct Window *Win;
  short Last[6], ListBlock, StrBlock, PropBlock;
  short PalItem, ActiveRow, PaI[4], Frame, IsKey, PrevKey,
	NextKey, KeysExist, ItemKeys, Linear;
  USHORT Colors[16];
  short AltKeyFrames;
  union KeyFrame *AltKF;
  ULONG AltKFsize;
  float TCB[3];
  struct WindowKeyStuff WKS;
} *EC_Win;

EXTERN struct MotionWindow {
  APTR MotionWin, Str[5], ValStr[1], StrArrow[5][2],
	IA_SensStr, IA_L_SensArrow, IA_R_SensArrow,
	L_IAArrow[5], R_IAArrow[5], ParTxt[3], ValTxt[5], LS_List, FramePages,
	BT_MakeKey, BT_GroupKey, BT_UpdateKeys, BT_AllKeys, BT_UpdateGroup,
	BT_NextKey, BT_PrevKey, BT_DeleteKey, BT_BankKeys, BT_KeyScale, BT_SunSet,
	BT_TimeLines, BT_WinSize, BT_Interactive,
	BT_Apply, BT_Cancel, BT_DeleteAll, BT_Sens[3], BT_Settings[5];
  struct Window *Win;
  char Motionname[USEDMOTIONPARAMS][30];
  char *MName[USEDMOTIONPARAMS + 1];
  short MoItem, IA_Item, Frame, IsKey, PrevKey, NextKey, KeysExist, ItemKeys,
	StrBlock, PropBlock, Linear;
  short AltKeyFrames;
  union KeyFrame *AltKF;
  ULONG AltKFsize;
  double Prop_Scale;
  float TCB[3];
  struct WindowKeyStuff WKS;
} *EM_Win;

EXTERN struct IAMotionWindow {
  APTR IAMotionWin, Str[2], StrArrow[3][2], CY_Move, CY_Grid, BT_GBDens[3], BT_AutoDraw,
	BT_Grid, BT_ElShade, BT_SunShade, BT_EcoRender, BT_DiagRender, BT_Vector,
	BT_Anim, BT_CompassBounds, BT_LandBounds, BT_ProfileBounds, BT_BoxBounds,
	BT_GridBounds, BT_ShowLat, BT_ShowLon, BT_MoveX, BT_MoveY, BT_MoveZ,
	BT_AutoCenter, BT_Aspect, BT_MakeKey, BT_Apply, BT_Cancel;
  short GridStrBlock;
  short AltKeyFrames;
  union KeyFrame *AltKF;
  ULONG AltKFsize;
  struct Window *Win;
} *EMIA_Win;

EXTERN struct ParListWindow {
  APTR ParListWin, LS_List;
} *EMPL_Win;

EXTERN struct EcosystemWindow {
  APTR EcosystemWin,
	BT_MakeKey, BT_UpdateKeys, BT_UpdateAll, BT_NextKey, BT_PrevKey,
	BT_DeleteKey, BT_KeyScale,
	BT_TimeLines, BT_Copy, BT_Swap, BT_Insert, BT_Remove, BT_Sort,
	BT_Apply, BT_Cancel, BT_DeleteAll, ColorCy[2], Label[10],
	IntStr[15], IntArrow[10][2], Str[5], StrArrow[2], ProcCheck[4],
	LS_List, BT_MakeModel, NameStr, ClassCycle, TexClassCycle,
	ModelStr, TexPageGrp, TextureText[3], BT_Edit[1], FramePages,
	BT_Settings[6];
  char Econame[ECOPARAMS][30];
  char *EName[ECOPARAMS + 1];
  char **EEList, **ECList;
  struct Window *Win;
  struct elmapheaderV101 MapHdr;
  struct faces *Face;
  long Facesize, MapLeft, MapTop, MapWidth, MapHeight;
  short *RelEl;
  short EcoItem, ContInt, Frame, IsKey, PrevKey, NextKey, KeysExist,
	ItemKeys, MapEco[4][2], EcoLimits[15][2], Linear, PropBlock, StrBlock;
  short AltKeyFrames;
  union KeyFrame *AltKF;
  ULONG AltKFsize, EEListSize, ECListSize;
  USHORT Colors[16];
  float TCB[3];
  struct WindowKeyStuff WKS;
} *EE_Win;

EXTERN struct SettingsWindow {
  APTR SettingsWin, IntStr[23], IntStrArrow[23][2], Cycle[51], FloatStr[16],
	FloatStrArrow[16][2], Str[15], BT_Get[8], Txt[1], TxtArrow[1][2],
	UseTxt, SpeedGauge,
	BT_ChangeScale, BT_FractalSet, BT_Apply, BT_Render, BT_Cancel, PageCycle, Pages;
  struct Window *Win;
} *ES_Win;

EXTERN struct TimeLineWindow {
  APTR TimeLineWin, ParCycle, Prop[3], FrameTxt, FrameTxtLbl, KeysExistTxt, TCB_Cycle,
	CycleStr, ValStr[3], StrArrow[2], TxtArrow[2], TxtArrowLg[2],
	TimeLineGroup, TimeLineObj[WCS_MAX_TLVALUES],
	SuperClass, BT_PrevKey, BT_NextKey, BT_AddKey, BT_DelKey,
	BT_Apply, BT_Play, BT_Cancel, BT_Grid, BT_Linear;
  struct IClass *TL_Class;
  struct WindowKeyStuff *WKS;
  struct GUIKeyStuff *GKS;
  struct TimeLineWindow **TLPtr;
  union KeyFrame *AltKF, **KFPtr;
  struct KeyTable *SKT;
  char **List;
  short *ListID, *KeyFramesPtr;
  struct Window *Win;
  float MaxMin[WCS_MAX_TLVALUES][2];
  long *KFSizePtr;
  double *DblValue;
  float *FltValue;
  short *ShortValue;
  APTR *PrntValStr;
  ULONG AltKFsize, ListSize, ListIDSize;
  short Frames, ActiveKey, ActiveItem, AltKeyFrames, KeyItem, ElevProf, WinNum;
} *EMTL_Win, *ECTL_Win, *EETL_Win, *TLWin[WCS_MAX_TLWINDOWS];

struct ScaleKeyInfo {
 short ModGroup[3], Group, Frame, Item, AllFrames, FrameScale,
	ValueScale, FrameShift, ValueShift, FSh;
 float  FSc, VSc, VSh;
};

EXTERN struct ScaleWindow {
  APTR ScaleWin, IntStr[2], FloatStr[3], ItemCycle, BT_Group[3],
	BT_AllFrames, BT_OneFrame, BT_FrameScale, BT_ValueScale,
	BT_FrameShift, BT_ValueShift,
	BT_Apply, BT_Operate, BT_Cancel;
  char **PSList;
  short *PSListID;
  short Group, Frame, Item, AltKeyFrames;
  union KeyFrame *AltKF;
  ULONG AltKFsize, PSListSize, PSListIDSize;
  struct ScaleKeyInfo SKInfo;
} *PS_Win;

struct LightWaveInfo {
 char	Path[256], Name[32];
 short	SaveDEMs, ExportItem, RotateHorizontal, Bathymetry;
 float	RefLat, RefLon;
 long 	MaxPolys, MaxVerts;
};

EXTERN struct LightWaveIOWindow {
  APTR IOWin, FloatStr[2], IntStr[2], ExportItem,
	BT_Bathymetry, BT_RotateHorizontal, BT_Export;
  struct LightWaveInfo LWInfo;
  struct Window *Win;
} *LW_Win;

/*EXTERN*/ struct BusyWindow {
  ULONG StartSeconds, MaxSteps, CurSteps;
  APTR BusyWin, BW_Percent, BW_Elapse, BW_Remain, BW_Cancel;
};

EXTERN struct DiagnosticWindow {
  APTR DiagnosticWin, TitleText, Txt[10], BT_Database, BT_Digitize;
	struct Window *EcosystemWin;
} *DIAG_Win;

EXTERN struct ForestModelWindow {
  APTR ModelWin, NameStr, IntStr[5], ClassCycle,
	BT_Add, BT_Remove, BT_Load, BT_Save, LS_List;
	char **Item, ItemStr[99][40];
	short MaxItems, Mod, ItemNum;
	ULONG ItemListSize;
} *FM_Win;

EXTERN struct FoliageWindow {
  APTR FoliageWin,
	BT_Suggest, BT_AddGroup, BT_RemoveGroup, BT_NewGroup, BT_Export, BT_AddImage,
	BT_RemoveImage, BT_ViewImage, BT_Keep, BT_Cancel, PalColCy[2],
	FloatStr[6], Arrow[6][2], Check[2], LS_GroupList, LS_ImageList,
	EcosysText, WidthText, HeightText;
  char **ECList, **GroupList, **ImageList;
  short Mod, FolEco, GroupEntries, CurrentGroup, ImageEntries, CurrentImage;
  ULONG ECListSize, GroupListSize, ImageListSize;
  USHORT Colors[16];
  struct Ecotype *Backup;
  struct FoliageGroup *CurGrp, **GrpAddrList;
  struct Foliage *CurImg, **ImgAddrList;
  struct Window *Win;
} *FE_Win;

EXTERN struct AnimWindow {
  APTR AnimWin, Str[2], IntStr[5], AspectCheck, BT_GetPath,
	BT_Render, BT_Save;
} *AN_Win;

EXTERN struct ProjectWindow {
  APTR ProjWin, Str[32], BT_Get[20], BT_DirList, BT_Apply, BT_Save, BT_Cancel;
} *PJ_Win;

EXTERN struct NewProjectWindow {
  APTR NewProjWin, Str[2], BT_Get[2], BT_Save, BT_Cancel;
} *PN_Win;

EXTERN struct EcoLegendWindow {
  APTR EcoLegendWin, ColBut[6], Cycle[6];
  char **EEList;
  ULONG EEListSize;
} *EL_Win;

EXTERN struct ScaleImageWindow {
  APTR ScaleWin, Str[3], BT_Halve, BT_Double, BT_Apply, BT_Cancel;
} *SC_Win;

EXTERN struct PrefsWindow {
  APTR PrefsWin, BT_LoPri, BT_NorPri, BT_HiPri, BT_SaveAsc, BT_SaveBin,
	BT_RenderQuarter, BT_RenderHalf, BT_RenderFull, BT_Close,
	BT_ERR[4];
  short LoPriBlock, NorPriBlock, HiPriBlock, RndrQBlock, RndrHBlock, RndrFBlock;
} *PR_Win;

EXTERN struct TimeSetWindow {
  APTR TimeSetWin, LonStr, MonthCycle, DateCycle, TimeStr, AMPMCycle,
	SunLatStr, SunLonStr, BT_Reverse, BT_Apply, BT_Cancel;
  short AltKeyFrames, Mod;
  union KeyFrame *AltKF;
  ULONG AltKFsize;
  double SunLat, SunLon;
} *TS_Win;

EXTERN struct CloudWindow {
  APTR CloudWin, Cycle, Text, Check, IntStr[1], CloudStr[6], FloatStr[7],
	CloudArrow[6][2], FloatArrow[7][2], BT_AddWave, BT_EditWave,
	BT_DrawCloud, BT_SetBounds, BT_Load, BT_Save, BT_Settings[2];
  struct CloudData *CD;
  struct Window *Win;
  struct GUIKeyStuff GKS;
  struct WindowKeyStuff WKS;
  struct TimeLineWindow *TL;
  short Mod, ReGen;
  char CloudDir[256], CloudFile[32];
} *CL_Win;

EXTERN struct MakeDEMWindow {
  APTR MakeDEMWin, Cycle[5], Text, IntStr[3], FloatStr[3], Prop, ArrowLeft[3],
	ArrowRight[3], Button[5], ModeButton[6], ReverseButton;
  struct datum *TC, *CurDat;
  short LastX, LastY, PMode, DMode, MaxEl, MinEl, CurEl, Units, ElSource,
	ControlPts, Displace, FileType, NoReversal;
  double UnitScale, StdDev, MinDist, NonLin;
  char FileIn[32], FileOut[32], DirIn[256], DirOut[256];
} *MD_Win;

EXTERN struct NNGridWindow {
  APTR NNGridWin, FloatStr[15], IntStr[5], Check[6], Button[4], Str[1],
	Text, Arrow[2], ApplyButton;
  struct NNGrid NNG;
  long NRows, NCols, RandSeed, Scope, OffX, OffY, NoiseSize;
  double XPow, YPow, Delta, H;
  float *NoiseMap;
} *GR_Win;

#endif /* GIS_GUI_H */

#ifndef GIS_MAP_H
#define GIS_MAP_H

EXTERN struct Window *MapWind0, *MapWind3;

EXTERN struct elmapheaderV101 *mapelmap;

EXTERN char graphpath[255],graphname[64],
     statdir[255],
//     statfile[64],   static AF
     filetype[6];

EXTERN USHORT RecordNumber, InterStuff, AutoClear,
       vectorenabled, ecoenabled,topo,topoload,align,
       //graphtype,  static AF
       MapDither,
       ContInt, NoOfTypes;

EXTERN short EcoLegend[6]
#ifdef MAIN
= {1, 4, 2, 3, 0, 5}
#endif /* MAIN */
;

EXTERN short EcoUse[6]
#ifdef MAIN
= {1, 1, 1, 1, 1, 1}
#endif /* MAIN */
;

EXTERN short topomaps,
     MapHighEl, MapLowEl,
     //ptstore[8],  static AF
     backpen, MP_DigMode, MP_Width, MP_Height, MP_Left, MP_Top;

#ifdef ENABLE_STATISTICS
EXTERN short statrows,statcols,statsize,normrows,normcols,normsize,
	maplowx,maplowy,maphighx,maphighy;
#endif /* ENABLE_STATISTICS */

EXTERN short *TopoOBN;

#ifdef MAIN
short //frontpen=2,  static AF
/*cornerx=30,cornery=30,grwidth=580,grheight=340,*/
	ContInterval=192;
#else
extern short // frontpen, static AF
/*cornerx,cornery,grwidth,grheight,*/ContInterval;
#endif

EXTERN long MapElmapSize, MapCoordSize, TopoOBNSize;

EXTERN __far long mapxx[MAXOBJPTS];
EXTERN __far long mapyy[MAXOBJPTS];

/*EXTERN*/ struct MapCoords {
 float C[4];
};

EXTERN struct MapCoords *mapcoords;

#ifdef ENABLE_STATISTICS
EXTERN double *statptr,*normptr,*normvalptr,*normstatptr,*graphptr;
#endif /* ENABLE_STATISTICS */

EXTERN __far double mlat[MAXOBJPTS];
EXTERN __far double mlon[MAXOBJPTS];
EXTERN __far double tlat[MAXOBJPTS];
EXTERN __far double tlon[MAXOBJPTS];
EXTERN double mapscale,maplat,maplon,latzero,lonzero,
       latscalefactor,lonscalefactor,
       //lat_y, static AF
       //lon_x,static AF
       rlat[2],rlon[2],
       y_lat, x_lon,
       MaxElevDiff;
       // MP_Nlat, MP_Wlon, static AF
       //MP_DigLatScale, MP_DigLonScale, static AF
       //MP_Rotate, MP_ORx, MP_ORy;  static AF

#endif /* GIS_MAP_H */





#ifdef MAIN
 #ifdef __SASC
 int CXBRK(void) { return(0); }
 int _CXBRK(void) { return(0); }
 void chkabort(void) {}
 #endif
#endif

EXTERN /*__far*/ const UBYTE grass[40][40]   // ALEXANDER __far raus
#ifdef MAIN
 =
{
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0, 0, 0, 0,15,
 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0,15, 0, 0, 0, 0, 0,15,
 0, 0, 0, 0,14, 0, 0, 0,15, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0,15, 0, 0, 0, 0, 0, 0,14, 0, 0, 0,13, 0, 0, 0, 0, 0,
15, 0, 0,15,14, 0, 0,15,14,14, 0,15, 0,15, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0,14, 0, 0, 0, 0, 0,14, 0, 0, 0,14, 0, 0, 0, 0, 0,
14,14, 0,14,14, 0, 0, 0,14, 0, 0,13, 0,15, 0, 0, 0,15, 0, 0},
 {0, 0, 0, 0,14, 0, 0, 0,15, 0,14, 0, 0, 0,14, 0,13, 0, 0, 0,
14,14, 0,14, 0,13, 0,15,15, 0,14, 0,14,13, 0, 0, 0,15, 0, 0},
 {0, 0, 0, 0,14,13, 0, 0,15, 0,15,14, 0, 0,14, 0, 0,15, 0, 0,
14, 0, 0,14, 0,15, 0,15, 0, 0, 0,14,14, 0, 0, 0,13, 0, 0, 0},
 {0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0,14, 0, 0,14, 0, 0,15, 0, 0,
14, 0,15, 0,12,15, 0,15, 0, 0,15,14, 0, 0, 0,13, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,12, 0, 0, 0, 0, 0,15, 0,14,13,13, 0,15,15, 0,
14, 0,13, 0,14,14, 0,14,12, 0, 0,14,13, 0, 0,14, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,12, 0, 0,14, 0,15, 0, 0,14,14, 0, 0,15,13, 0,
15,12,13, 0, 0,14, 0,14,14, 0,13, 0,13, 0, 0,14, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15,15, 0,14, 0,15,15, 0, 0,15, 0, 0,14, 0, 0,
14,14, 0, 0, 0,14, 0,15,12, 0, 0,13,13, 0, 0,14,14, 0, 0, 0},
 {0, 0, 0, 0, 0,12,15,12,12, 0,15,12,15, 0,15,14, 0,12,13, 0,
15,14,15, 0,12,15,15, 0,12, 0,13,15,13,12, 0,15,14, 0, 0, 0},
 {0, 0, 0, 0, 0, 0,14, 0,12, 0, 0,14,12, 0,15,14, 0, 0,13, 0,
15,14,15, 0,12, 0,15, 0, 0,13,13,15, 0,12, 0, 0,14, 0, 0, 0},
{12,12, 0, 0, 0, 0,15, 0,12, 0, 0, 0,12, 0,15,14, 0, 0,13, 0,
 0,12,15, 0, 0, 0,15, 0,12,14,13,15,15,12,14,14,15,12,15, 0},
 {0,15, 0, 0, 0, 0,15,12, 0, 0,15, 0,14,15, 0, 0, 0,14,13, 0,
14,14,15, 0, 0,14,14, 0,12,14,13, 0,15, 0,12, 0,15,12, 0,15},
{12, 0,12, 0, 0, 0,15,14, 0, 0,15, 0,14, 0,13, 0, 0,14, 0, 0,
14,14,14, 0, 0,14,14,13,12, 0,13, 0,15, 0,12, 0,15, 0, 0, 0},
{ 0, 0,12, 0, 0, 0,15,14,12, 0,13,13,14,14, 0, 0, 0,14, 0, 0,
14,12,14,15, 0, 0,13,14, 0, 0,13,14,15, 0, 0, 0,15, 0, 0, 0},
{ 0, 0, 0,12, 0, 0,15, 0,12,12,15, 0,13,14, 0, 0, 0,12, 0, 0,
14,14,12,15, 0, 0, 0,13, 0, 0,13,14,14,15, 0, 0,15, 0, 0, 0},
{ 0,15, 0,14, 0, 0,14,15, 0, 0,15,15,12,14,13, 0,14,14, 0, 0,
15, 0,14,15,12, 0,13,15,13, 0, 0,13,14,15, 0,14,15, 0, 0, 0},
{ 0, 0,15,14, 0,15,14,15,14, 0, 0,15,14,14,13, 0,13,14, 0, 0,
15, 0,14,15, 0, 0,13, 0,13, 0,15,13, 0,15, 0,14,15, 0, 0, 0},
{ 0, 0,15, 0, 0, 0,14,13,14, 0, 0, 0,14,13,13, 0, 0,14,13, 0,
15, 0,14,15, 0,15,13,13,15, 0,15,13,14,15, 0,14, 0, 0,12, 0},
{ 0, 0,15, 0, 0, 0,14,14,15, 0, 0, 0,14,15,12,15, 0,14,13, 0,
 0, 0,12,15, 0,15,13,13,15, 0,15,13,14,15, 0,14, 0,12, 0, 0},
 { 0, 0, 0,12,12, 0, 0,14,15, 0, 0, 0,14,15,15,15, 0,14, 0,12,
 0,15, 0,15, 0,15, 0,13,15, 0, 0,13,14,15,12,14,12,12, 0, 0},
 {0, 0, 0,12, 0,15,13,13, 0, 0,12, 0, 0,15, 0,15,12,14,12, 0,
 0,15, 0, 0,12, 0, 0,12,15,12, 0,13,14,12,14,12,14,12, 0, 0},
 {0,12, 0, 0, 0,15, 0,13, 0, 0, 0, 0,14,15, 0,14, 0,12,15,15,
 0,15,15, 0,14, 0,15, 0,15,12, 0,15,13,12,14, 0,14,12, 0, 0},
 {0, 0,12, 0,15, 0, 0,13, 0,15, 0, 0,14,15, 0,14,13,14,13,15,
 0, 0,15, 0,14, 0, 0,14,15,12,13,15,15,12,14,13,14, 0, 0, 0},
 {0, 0, 0, 0,12, 0, 0,15,15,15,15,13,14,12, 0,14,13,14,15,12,
 0, 0,15, 0,14, 0,12,14,15,13,13,12,11, 0,12,13,14,12, 0, 0},
 {0, 0, 0, 0,15,12, 0,15,15, 0, 0,12,14, 0,12,14,13,14,15,12,
 0,12,15, 0,14, 0,12,14,14,14,13,14,12, 0, 0,13,12, 0, 0, 0},
 {0, 0, 0, 0,15,14,12,15,15,12, 0,12,14, 0,15,14,15,13, 0, 0,
12, 0,15, 0,12,12, 0,13,15,14,13,14,14, 0, 0,12,12, 0, 0, 0},
 {0, 0, 0, 0,15,14, 0,15, 0, 0, 0,13,14, 0,12,14,15,13, 0, 0,
 0,11,15, 0, 0,13, 0, 0,13,13, 0, 0,14, 0,13,12, 0, 0, 0, 0},
 {0, 0, 0, 0,15,14,12,15, 0, 0, 0, 0,14, 0,15,14,13,12, 0, 0,
12,13,15,12,12,13, 0, 0,12,15,13, 0,14,13, 0,12, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15,12,15, 0,12,15, 0, 0, 0,15, 0,13, 0,15,13,
14,12,15, 0,12,13, 0,15, 0,15,12, 0,14,10,13, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15, 0,15,15, 0,15,12,12, 0,15, 0,13, 0,15,13,
 0, 0,14, 0, 0, 0,14,15, 0,15, 0,12,12,12,13, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15, 0, 0,15, 0,15, 0, 0,13,15, 0,13,10,15,13,
 0,13,14,15, 0,12,14,15, 0,15,14,10,14, 0,12, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15, 0, 0,15, 0,15, 0,13,13,15,15,13,13,15, 0,
 0,13, 0, 0,15, 0,15,15, 0,15,14, 0,14, 0,12, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,13, 0,15,15,13,13, 0,10,13,13,15, 0,
 0,13,13, 0,15,15,14,14, 0,13, 0,12,14, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0,15,13, 0,15, 0, 0,13,14, 0,15,15,13, 0,
 0, 0,13,15, 0, 0, 0,14,10,13, 0,14,14, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,13,13, 0,12, 0,13, 0, 0,14, 0,13, 0,
13,15, 0,15,15,15, 0,14,14, 0, 0, 0,14, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0,12, 0, 0, 0, 0, 0, 0,13, 0,
14,13,15, 0,12,15,14,14, 0, 0, 0, 0,14, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0,
 0, 0,15, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
#endif /* MAIN */
;


EXTERN /*__far*/ const UBYTE evergreen[60][20]     // ALEXANDER __far raus
#ifdef MAIN
 =
{
 {0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,15,13, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,13,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,15,13, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0,13,13,13,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0,13,13, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,13,13,13, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0,13,13,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,13,11,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0,15,11,11,11,13,15, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0,13,13,11,11,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,11,13,15, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15,13,11,11, 9,11,13,15, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0,15,11, 9,11, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15, 0,11,11, 9,15,11,15, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0,15,11,15, 6, 9,13, 0,15, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,15, 0,11, 9,11, 6,13, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0,15,11, 9, 9,11,13,13,15, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 9, 9,11,13,13,13,11, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0,13, 0,13,11, 9, 6, 9, 9,15,15, 0, 0, 0, 0, 0},
 {0, 0, 0, 0,15, 9,13,13,13,13,11,15, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0,13,11, 9,11,11, 9,15,15, 0, 0, 0, 0, 0},
 {0, 0, 0, 0,15,13,13,11, 9, 9, 9,11,13,13, 0, 0, 0, 0, 0, 0},
 {0, 0, 0,15, 0, 0,13,11, 9,11,13,13, 9,13,13,13, 0, 0, 0, 0},
 {0, 0, 0, 0,13,13,13, 9,15,11, 6, 9, 9, 9, 0,13,13, 0, 0, 0},
 {0, 0,15, 9, 9, 0,11,11, 9,15,11,11,11,15, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 9, 9, 9, 9,11, 9, 9, 9, 0,13,15, 0, 0, 0},
 {0, 0, 0,15,15, 0, 0,11,11,11, 6, 9, 9,13,15, 0, 0, 0, 0, 0},
 {0, 0, 0, 0,15,15,11,15,11,11, 6,13, 9,11,13,13,15, 0, 0, 0},
 {0, 0,15,15, 0,15,13, 9,13, 9, 6,11, 9,11,13, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 9, 9, 9, 9,13,11, 9, 9,13, 9, 9,13,13,15, 0, 0},
 {0, 0, 0, 0,13, 9, 9,13, 9,11,11, 9,11,11,15,15,15, 0,15, 0},
 {0, 0, 0,15,15,13,11,11,11, 9, 6, 9,13,11, 6, 6,11,15, 0, 0},
 {0,15,15,13,13, 9, 9, 6, 6, 9, 9,11, 9, 9,13,15,15, 0, 0, 0},
 {0, 0,15, 9, 9, 9,11, 9, 9, 9,11,11,11, 9,11,11,13,13,13, 0},
 {0, 0, 0, 0, 0,15, 9, 9,11, 6, 9, 9, 9,13,15,11,11,11,15,15},
{15,13,13,11,11,13,11,11,11, 6, 9, 6, 9,11,11,11,15, 0, 0, 0},
 {0, 0,15,11, 9, 9, 9,13, 9, 9, 9, 6, 9, 9,15,13,13,13,13, 0},
 {0,13,13,11,13,13,13,11,11,15, 9,13,11,11, 9, 9, 9,13,15, 0},
 {9, 0,15,15, 9, 9, 9,13,13, 9, 6, 9,13,13, 6, 6,11,13,13,13},
 {0, 9,11,11,11, 9, 9, 9, 9, 9, 9,11,11, 9, 9, 9,11,11, 9,11},
 {0, 0, 0, 9, 9, 9, 6, 6, 6, 9, 9, 6, 9, 9,13, 9, 9, 9,11, 0},
 {0, 0, 0, 9,11,11,11,11,13,11, 4,11, 9, 6, 6, 6, 9, 9, 0, 0},
 {0, 0, 0, 0, 0, 0, 9, 6, 6,11, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0}
}
#endif /* MAIN */
;


EXTERN /*__far*/ const UBYTE deciduous[60][40]      // ALEXANDER __far raus
#ifdef MAIN
 =
{
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,15, 0,14,15,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,15,14, 0, 0,15, 0,14, 0,11, 0, 0,15,15,15, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,14,14,14, 0,15,15, 0,15,15,11,15, 0,15,15, 0, 0, 0,15,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0,15,14, 0, 0,14,14, 0,15,10,15, 0,15, 0,15, 0,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11, 0,14,11, 0,14,10, 0,14, 0, 0, 0,15, 8,15, 0,15, 0, 0, 0, 0, 0, 0,15, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11,11,11, 0,15, 8, 0,10, 0, 8,14,15,15, 8,10,11,15,11,11,15, 0, 0, 0, 0, 0,15,15, 0},
 { 0, 0, 0, 0, 0, 0, 0,15,15, 0, 0, 0, 0, 0, 0,15,10, 8,10,15, 8,14,10, 0,11,14, 8,15, 0,15, 0, 0, 0, 0, 0,15, 0,15, 0, 0},
 { 0, 0, 0,15, 0, 0, 0, 0, 0,14,15, 0, 0, 0,15, 0,15,11, 8,10, 8,10, 8,10,15, 6, 8,11,10,11, 0,15, 0, 0, 0,15,12, 0,15, 0},
 { 0, 0,15,14,14,15, 0,15, 0,14, 0, 0, 0, 0, 0,15, 0,10,15, 8, 8,15,15, 8, 8,14,15, 0,14,14, 0,15,15, 0,15, 0,12,12, 0, 0},
 { 0, 0, 0,11,12,15,12,14,12,15,14, 0,15,12,15, 0, 0, 0,15, 8,10, 8,15, 0, 8, 0, 0,15,10,15,11, 8,10, 8,15, 0, 0,15, 0, 0},
 { 0, 0, 0, 0, 0,11,12, 8,15, 8,14, 0, 8,12,12, 0,15, 0,15,10,15, 8, 0,15,15, 0,15, 0,10,15, 8, 8,15, 0, 0,15,11,11, 0, 0},
 { 0, 0,15,15, 0, 0,11,10, 9, 7, 9,14,10,14, 7,14,14,12,15, 9,10, 8, 0, 0, 8, 0, 0,10, 8,11, 8,15,11, 0,11,11, 0, 0,15, 0},
 { 0, 0,11,15,15, 0, 0,11,11,10, 8, 8,15, 8,14, 7,10, 0, 0,15, 9,13,13, 8,13,15, 6,11, 8,11,11, 0, 0, 0,11, 0, 0, 0, 0, 0},
 { 0, 0, 0,11, 0,15,10,11, 0,10,11, 0, 8, 0,15, 7,15, 0,15,15, 0, 8,11, 8, 8,11, 0, 0,15, 0, 0,15,15, 0, 0, 0,15,15, 0, 0},
 { 0, 0, 0, 0, 0, 0,15, 0, 8,11,11, 0, 0, 6,15, 9,15,11, 0, 0, 8, 8,11,11, 9,11,11, 0,10,15,11, 0, 0, 0, 0,11, 0,15,15, 0},
 { 0,15, 0, 0, 0,15,11,11,11,11, 8, 8,15,15,11,15, 9,15, 0,15, 0, 6, 8, 0, 0,11, 0,15,15, 0,15, 0, 0,11, 0, 0,10,15, 0, 0},
 {15,15, 0,15, 0, 0, 0, 0, 0,11, 0,15, 8, 0,15, 8, 8,15, 0, 0, 6,15, 6,15,11, 0,11, 0, 0, 0, 0,15,15,15,15, 0,15,10,10,15},
 { 0,14,13,14,15, 0,15,15, 0, 0, 0, 0, 0,11, 9,11,11, 8, 0, 0,15,11,11,11, 0, 0, 0, 0, 0,15,15,15, 0, 0, 8, 8,10, 0,11, 0},
 { 0, 0,15,13,14,14, 0, 0, 0, 0, 0, 0,15,15,11, 8,11, 9,13,15, 6,11,15,11, 0,15,15,15,15, 7,15, 8, 9, 7,15, 7, 0,11,15, 0},
 {15,11, 0,10,10, 0,15, 0,15, 0, 0, 0, 0,11,10,15,10,13, 9,15, 6, 9,15, 0,15,11, 9,15,10,15,10, 8,15, 0,15,11, 0,11, 0, 0},
 { 0,15, 0,15, 9, 0, 0, 7,15, 0,15,15,15,15, 0, 0,13, 8, 6, 9,11,12,12, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0,11,11,11, 0, 0, 0, 0},
 {13, 0,15,11,11,11,11, 0, 7, 0, 0,15, 0,15,12, 0,15,12, 8,12, 6, 6,12,12,11,15, 5, 8, 0, 0,15,15,15, 0, 0, 0, 0, 0, 0, 0},
 { 0,15, 0,15, 0, 9,15,11,15, 7,15,11, 0, 0, 0,14, 9,12, 0, 9, 9, 9, 6,12, 6,15, 8,15,12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0,15, 0, 0,15, 9, 9,10,10,15,15,11,11,11,15,14, 8,12, 0, 0, 6,15, 6,12,15, 9,11,12,15,14,10,14,14, 0, 0,15,15, 0, 0, 0},
 { 0, 0,15,15, 9,15, 8, 9,10, 7,15, 8, 0,11,11, 8, 6, 8, 8, 0, 5, 6,15, 6, 0, 0, 0, 0, 7,15,14, 0, 0,14,15, 9,15,15, 0, 0},
 { 0, 0,12,11,15,11,15, 8, 6,10, 8, 8,15, 0,15,11, 8,13,14, 8,14, 6, 6, 0, 0, 0,11, 0,15, 7,15, 0, 0,15,10,15, 9,15,12, 0},
 { 0, 0, 0,12, 0, 0, 8, 8, 9,15,15, 8,15, 0, 0, 0,14, 8,10,14,10,11, 8, 0,11,11,11, 0,11,11, 7,15,11, 9,15, 7, 0, 0,12,11},
 { 0, 0, 0, 0, 0, 0, 0, 0,15, 9, 9, 6, 8, 8, 7,13,14,14,15,10,11, 5, 6, 0, 0, 0, 0,11, 0, 0, 7,11, 6, 0, 0,11,12, 0, 0, 0},
 { 0, 0, 0,15, 0, 0, 0, 0,15,13,13, 9,13, 8, 8, 7,12, 7, 8,12, 6, 5, 6,12,12,15, 0, 0, 0, 0,11,11,15, 0, 0, 0, 0, 0, 0, 0},
 { 0,15, 0,13,15,13, 0, 0,12, 0, 0, 6,13, 6, 5,12,10,12, 7,10, 0, 6,15, 9, 9, 7,15,12,13,15,15, 6,15,15,15, 0, 0, 0, 0, 0},
 {15,15, 0, 0,13, 0, 0,11,11,11, 0,13,13, 6,12, 6, 8,10,12,10,12, 6, 5, 6, 0,15, 7,15,12, 9, 6,15, 9,15,15,10,15,15, 0, 0},
 { 0,11,15,15,15, 0, 0, 0, 0,11,11, 0, 0, 0, 8,15, 8,12, 6,12,10,15, 5, 6, 0,15,15, 7,10, 6,15, 8,15,12, 7, 8,10,15, 0, 0},
 { 0, 0,15, 0, 0,15, 0, 0, 0,11,11,15,11,15, 6, 8,12, 6,15, 6,10, 6,13,13,15, 8,15, 6, 8, 6, 9,15, 5, 6,10,15,10, 0, 0, 0},
 { 0,12,15, 9, 9,15,13,15, 0, 0, 0, 0,11,11, 9,15,15, 7,10,15,15, 6,13,15, 8, 9,15, 6, 6, 7,12,12,15,10, 0, 0,12, 0, 0, 0},
 { 0, 0,12,15, 9,15,10,13,15,13,15, 0,15,11, 6, 6,15, 8, 8, 8, 6, 5, 6,12, 6, 6,15, 8,15,15,10, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0,15, 0,15, 9, 6, 0,15,15, 7,15, 0, 0, 0,13,13, 6, 8, 7, 8,15,10,15, 6, 5, 0, 0, 0,15,12, 0, 0, 0,15, 0, 0, 0,13, 0, 0},
 { 0,12,12,12,12, 9, 6,10,10, 7,15,15,13,13,13, 6,15,13,12,13, 5, 6,15, 6, 0,13, 8,13, 0, 0, 0,13, 0,15,15, 0,13, 0, 0, 0},
 { 0, 0, 0,12,10,10, 0, 6, 9,15,10, 8,15,15,13,13, 6, 8, 0, 0, 5, 6, 6, 6, 0,15, 8,13, 0, 0,13,10,13, 0, 0, 0,15,15, 0, 0},
 { 0, 0, 0,12,15, 0,15, 9,10, 6, 9,15, 0, 8,15, 8,13,12,12, 0,15, 5, 6, 0, 0,13,12, 0, 0,13,13,10,15, 0,15, 7,15, 0, 0, 0},
 { 0, 0, 0,12, 0, 0, 0, 0,11, 6,15, 6, 9,15,15,15, 0,12,12, 0, 6, 5, 5, 0, 0, 8, 0, 0, 0, 0, 0,15, 7,15, 6, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0,12, 0,15,11, 9,11, 6, 6, 0, 6,15, 6, 0, 0, 0,12, 6, 5, 6, 0, 0,15, 0,15,11,15, 8, 8, 8, 6,15, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0,15, 0,11, 0,11, 6, 6,15,15, 0, 0, 0, 0,15,10, 5, 6,11,11, 0,15,10, 6, 0, 0,15,15, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0,11,11, 0, 0,11, 6, 6, 5, 0, 0, 0, 0,10,10, 5, 6,15,10,11, 6, 6, 6,11,11, 0, 0,11,11, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,15, 0,12, 0,15,10, 0, 0,11, 6, 5, 5, 0, 0, 5, 5, 6, 5, 0, 6, 6, 6,11, 7,11, 7, 0,11,11, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0,15,12, 0, 0,10, 0,11, 0, 0, 5, 5, 5, 6, 6, 5, 6, 0, 0, 6,11, 6,11, 0, 0, 0,11, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0,11, 0,11,11, 5, 6, 6, 5, 6, 0, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0,11, 0, 5, 5, 6, 5, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 6, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 6, 5, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5, 6, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
#endif /* MAIN */
;


EXTERN /*__far*/ const UBYTE rockbrush[60][40]       // ALEXANDER __far raus
#ifdef MAIN
 =
{
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,15,15, 0, 0, 0,15,15, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15,15,12,15,15,15, 0,15,12,15, 0,15,15,15, 0,15,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0,15, 0,12, 0,15, 0,13,15,12,14,15,11,15,15, 0,15, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0,14,14, 0,14,13,12, 0,13,13,14, 0,14,15, 0, 0,13,13, 0,11,15, 0,15, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0,15,14,14,14,14, 0,15,15,13, 0,14,14,15, 0,15,11,15,15,14,14,15, 0,14,13,15, 0,13,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,15,11, 0,15, 0,13,12, 0,14,13,11,15,15, 0, 0,15, 0,13,13,14, 0,15,13,14,14, 0,15,13,14, 0,15, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0,11,14,14, 0, 0,12,14,14,15,11,13,13, 0,13,15,13,14,14,15,15,15, 0,14,15, 0,13,14,14,13, 0, 0, 0},
 { 0, 0, 0, 0, 0,13, 0,13, 0,15,14,15,14,14,15,14,15,13,15,15,15,15,13,14,14,11,15,12,15,11,13, 0,14,13,11,14,14, 0,15, 0},
 { 0, 0, 0, 0,11, 0,11,15,14,11, 0, 0,15, 0,15,14,11,15,14,15,12,12,12,14,14,13, 0,14,15, 0,13,13, 0,14,15,15,14,15, 0,15},
 { 0, 0,15, 0,14,14,13,11,15, 0,11,15, 0,15,15,13,15,14,14, 0, 0,15,15, 0,14,14, 0,15, 0,14,14, 0,14,14, 0, 0,14, 0,15, 0},
 { 0, 0, 0,15, 0,15,13,13, 0,14,15,15,14,12,15,12,12,14,11, 0,15,11,15,11, 0,14,14,11,15,15,14,14,11, 0,13,15,14, 0,14, 0},
 { 0,15, 0,11,14, 0,15,15,13,14, 0,15, 0,15,11,14,12,12,13,13,15, 0,14,11,14,12,12,14,14, 0,11,14,14,13,13,14,14,11, 0, 0},
 { 0, 0,15, 0,13,11,12,12,14,14,11,15,15,11,15,13,12,12,12,13,11,15,14,14,12,12,12, 0,13,13,15, 0,14,14,13,14,14, 0,13, 0},
 { 0, 0, 0,14,13, 0, 0,15, 0,14,14,11,15,15,13,15,15,12,15,15,11, 0,11,14,13,15,13,15, 0,13, 0,15,11,14, 0,15,12, 0,13,14},
 {15, 0,14, 0,14,13, 0,14, 0,14,14,13,13,13,14,14,14,14,13,15,14,15,14,14,15, 0, 0,15,11,15,13,11, 0,11,14, 0,13,14,14,15},
 { 0,13, 0,12,14,12, 0, 0,14, 0,12,13,13,13,14,14,11, 0, 0,15,14,12,13,15, 0, 0,15,11,15,14,13,14,15,11,12,13,14,14, 0, 0},
 { 0, 0,11, 0,14, 0,14, 0,14,12,15,15,13,14,10,14,11,15, 0,14,12,12,12,15,15, 0, 0,14,15,11,13,15,15, 0,11,13,14, 0, 0, 0},
 { 0, 0,14,12,15, 0,15,12,13,13,15,13,13,14,14,14,15, 0,14, 0,13,12,15,15, 0,15,14,14,14,14,14,15, 0,15,13, 0,14,11, 0,14},
 { 0, 0, 0,15, 0,14, 0,12,14,14,12,12,12,14,11,15, 0, 0, 0,14,13,12, 0,15,15,13,14,14,11,15,13,12,15, 0, 0,11,11, 0, 0,15},
 {15, 0,13,15,12,14,14,14, 0,15,15,15,12,13, 0, 0,15,14,11,14,12,13,15, 0,13,13,15, 0,15,11,13,12,15, 0,14,15, 0,15,13, 0},
 { 0,11, 0,15, 0, 0,14, 0,12,15,11,15,13,13,15,14,14, 0,11,15,13,11,15,13,15,13,13,15, 0,11,13,14, 0,15, 0,15, 0, 0, 0, 0},
 { 0, 0,14,15, 0,14,11,15, 0,15,14,15,13,15,13, 0,14,15,14,13,13,15,15,13,13,14,13, 0,15, 0,14,14, 0, 0,14,11,12, 0,13,13},
 { 0,14,14, 0,15,15, 0, 0, 0,15,15,14,11,15,15,15,14,14,14,13,13,13,15,13,13,14,15,15,14,12,14,14, 0, 0,14, 0, 0,13,13, 0},
 {15, 0, 0,11, 0,14,11,15,15,10,15,15,11,13,13,13,13,14,14,11,14,15,15,15,13,15, 0,14,15,12,13,14,15, 0, 0, 0,12,15, 0,13},
 { 0, 0,11,12,14,15, 0, 0, 0,10,15,13,15,13, 0,13,11,15,14,14,14,15, 0,15,13, 0,15,14,15,15,11, 0,14,14,13,13,15,15, 0, 0},
 { 0, 0,15, 0,11,15,14,15, 0,15,13,13, 0,15,13,13,15, 0,15,15,14,11,14,11,15, 0,14,14,15,15,11,15,15,13,13, 0,12, 0,14, 0},
 { 0,15,11, 0,15, 0,11,13,11,15,13,11,13,13,13,15, 0,15,11,15,15,15,15,13,13,13,14,14,15,15, 0, 0,15,13, 0, 0,12, 0, 0,15},
 { 0, 0,12,14,14, 0,15,13,13,14,11,15,13,15, 0,14, 0,14, 0,11,14,11,13,13, 0,15,15,13,15,15,14, 0,13,13,14, 0,13, 0, 0, 0},
 { 0, 0, 0,11, 0,15, 0, 0,15,13,11,14,14, 0, 0, 0,14, 0,14,12,14,14,13,11,15,15,13,13,15,11,14, 0,13,10,14,11, 0,13, 0,15},
 { 0,11, 0,11,15, 0,15, 0,15,15,13,11,14,15,15, 0,14, 0,14,12,14,14,14, 0,15,15,15,13,13,13,14,13,11,14,14,11, 0,13, 0, 0},
 {15, 0,11,14,13, 0, 0,14, 0,14,13,13,13,14,14,15,13,15,11,15,14,14,14,14, 0,13,15,12,13,14,14,14,14,14,14,11,11, 0,13, 0},
 { 0,15,14,11, 0, 0,12,15,12,14,14,11,14,14,14,15,13,12,15,15,10,13,14, 0,15,15,12,12,13,14,14,15, 0,15,13,13,11,15, 0, 0},
 {15, 0, 0,12, 0,14,14,13,14,14,14,15,13,11,15,15,15,13,13,10,13,13,11,14,15,15, 0, 0,14,13,13,15, 0, 0,15, 0, 0,11,15, 0},
 { 0,15, 0, 0,12, 0, 0,13,14,15,12,15,13,11, 0,13,15,12,13,13,13,11,14,14, 0,14,14,14, 0,13,11, 0,15,15,13,11,15,11,11,15},
 {15, 0,15,15, 0,14, 0,13,13,15,12,14,11,15,15, 0, 0,15,14,13,13,15,14,11,11,14,14,14,14,11,15, 0,15, 0,13,11, 0, 0,14, 0},
 { 0,15,15, 0,15, 0, 0,13,15, 0, 0, 0,12,14, 0,15,15, 0,14,11,14,15,11, 0,14,14, 0, 0,14, 0,15,15,15,13,15,15,15,14, 0, 0},
 { 0,15,15,15,11,15,15,13,15,15, 0,15,12,14,15, 0,15,14,13,12,14,11,15,13, 0, 0,15, 0,11,12,15,15,15,15, 0, 0, 0, 0, 0, 0},
 { 0, 0,14,15,13, 0,13,13,15,15,11,14,14,14,10,15, 0,14,13,12,12,13,15,15,14,13, 0, 0,12,15,15, 0,15,13, 0,15, 0,11, 0, 0},
 { 0, 0, 0,13,13,13,13, 0,15, 0,15,11,14,11,14,11, 0,14,11,15, 0,15,14, 0,14,15, 0,15,11,15, 0,15,15,13,14, 0, 0,13, 0, 0},
 { 0, 0,11, 0,13, 0,15,15, 0, 0,15,14,14,14,14,11, 0,10,13,13,13,13,15,14,14, 0,15,11,15,15,11,14, 0, 0,13,15,13,13,15,15},
 { 0, 0,14,15,13, 0, 0, 0,15,14,11,14,14,15,14,14,11,14,13,15,13,15,15, 0,15, 0,14,14,13,13,14,13,15,15,13,13,13,13,13, 0},
 { 0,11, 0, 0, 0, 0, 0,15,14, 0,11,14,13,14,14,14,14,14, 0,15,15,15,13,13, 0,15,14,13,13,13,14,15,15,15, 0,15,15,11,15, 0},
 {15,11, 0,13,15,15, 0,14, 0,11,13,14,14,14,10,14,14,13,13,15, 0,15,15,13,15, 0,14,14,14,14,14,13,15,15, 0, 0,12, 0, 0,15},
 { 0, 0, 0,15, 0, 0, 0,14,15,13,13,15,14,13,13,11,14,14,13,15, 0, 0,15,15,15,13,14,14,13,14,14,15, 0, 0,15,11,12,15, 0, 0},
 { 0,12,12,12,15, 0,14,14,11,13,12,15,14,13, 0,11,11,14,13,11,15, 0,15,15, 0,13,14,14,14,14,14,12,12,15,14,14, 0,15, 0, 0},
 {15,11, 0,12, 0,15, 0,13,11,15,15,15,13, 0, 0,15, 0,13,14,13,11,15,13,15,15,13,15,14,11, 0,15, 0,15,11,14,11,15, 0, 0, 0},
 { 0, 0, 0,15,14, 0,14,13,13,15,11,13,13,15,15, 0,15,13,14,14,13,13,15,15, 0,15,11,14,14,14,15, 0,15,15,15,13,13, 0, 0, 0},
 { 0, 0,15, 0, 0,14, 0,11,11,13,13,13,15, 0,15,13,15,15,13,14,11,13,15,15,15,14,15,11,15,14,14,15, 0,15, 0,11,13,13, 0, 0},
 { 0, 0, 0,15, 0,14, 0, 0,13,13,15, 0,14,13,13,13,15, 0,13,11,14,15, 0, 0,15,14,15,15,15,13,14, 0,15,15,11,15,13, 0, 0, 0},
 { 0, 0, 0, 0, 0,14, 0,13,13,15,14,14,14,15,13,13,15,15,13,14,14,15,13,15, 0,11, 0,15, 0,13,13,13, 0,15,11, 0, 0, 0,11, 0},
 { 0, 0, 0, 0,15, 0,13,13, 0, 0,15,13, 0, 0, 0,15, 0,15,14,12,13,14,13, 0, 0,13, 0, 0,13,15,11,13,13,11,14,14,13, 0, 0, 0},
 { 0, 0, 0,15, 0,13,13, 0,11, 0,15,13,13,13,15, 0,13,15, 0,15,11, 0, 0, 0, 0,11,15,14,14, 0,15, 0, 0,13,15,14, 0, 0, 0, 0},
 { 0, 0, 0, 0,15, 0, 0, 0,14, 0,13,13,13, 0,15, 0,13,15, 0,15,13,14,15, 0, 0,14,14,15,13, 0,11, 0, 0,13,13,13, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0,15, 0, 0,13, 0, 0,13, 0,12,12,15,15,15, 0,14,11,15,13,11, 0,15, 0, 0, 0, 0,15, 0,13, 0,15, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13,15, 0, 0,12, 0,15, 0,15,11,14,14,14,14,14, 0, 0, 0,15,11, 0, 0, 0,15, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0,11, 0, 0,13, 0, 0, 0,12,15, 0,15,11,11,15,14,14,14,12,11,15,15, 0, 0,15, 0,13, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,12, 0,15,15, 0, 0,15, 0,11,14,11, 0, 0, 0, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,15, 0, 0, 0,12, 0,15, 0,15,15, 0,15, 0, 0,11,13,15,15, 0,13, 0, 0,15, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11, 0, 0, 0,15,12, 0,12, 0, 0, 0, 0, 0,12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11, 0,15, 0, 0, 0, 0, 0, 0, 0,11, 0, 0, 0,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
}
#endif /* MAIN */
;


EXTERN /*__far*/ const UBYTE snag[60][12]      // ALEXANDER __far raus
#ifdef MAIN
=
{
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0,14,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14,13, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0, 0,14, 0,13, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0,14,14,13, 0, 0, 0, 0, 0},
 { 0, 0,14,13, 0,13,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,14,14,14, 0, 0, 0},
 { 0, 0, 0, 0,14,14,12, 0, 0,14, 0, 0},
 { 0, 0, 0, 0, 0,14,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,12,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0,14,13,14, 0, 0, 0, 0, 0},
 { 0,14,13,13, 0,12,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,12,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,12,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,12,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,12, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,12,14, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,13, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12, 0, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,12,13, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,12,13,13,12,14, 0, 0},
 { 0, 0, 0, 0, 0,14,12,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,13,13, 0, 0, 0, 0},
 { 0, 0, 0, 0,13,14,13,12, 0, 0, 0, 0},
 { 0, 0, 0, 0,14,12,13,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12,14, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,14,12,13, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,12,11, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,12,11, 0, 0, 0, 0},
 { 0, 0, 0, 0, 0,13,13,11, 0, 0, 0, 0}
}
#endif /* MAIN */
;

EXTERN /*__far*/ const UBYTE stump[10][4]     // ALEXANDER __far raus
#ifdef MAIN
=
{
 { 0,13, 0, 0},
 {14,12,14, 0},
 {13,13,13, 0},
 {14,13,12, 0},
 {12,13,14, 0},
 {14,12,14, 0},
 {14,12,13, 0},
 {13,12,11, 0},
 {13,12,11, 0},
 {13,13,11, 0}
}
#endif /* MAIN */
;

EXTERN /*__far*/ const short StrataTex[1200]
#ifdef MAIN
= {
255, 255, 230, 250, 247, 240, 233, 209, 206, 205, 
195, 173, 222, 230, 231, 234, 233, 229, 225, 210, 
180, 129, 147, 189, 213, 224, 223, 206, 232, 240, 
241, 239, 225, 207, 231, 232, 196, 176, 215, 238, 
246, 248, 246, 249, 249, 247, 235, 212, 167, 186, 
172, 186, 206, 218, 218, 193, 209, 210, 171, 190, 
210, 219, 232, 233, 237, 208, 223, 236, 246, 247, 
247, 247, 250, 239, 234, 199, 155, 185, 135, 198, 
209, 187, 186, 169, 195, 204, 217, 206, 220, 227, 
215, 239, 253, 252, 248, 248, 242, 223, 249, 250, 
228, 182, 211, 243, 240, 239, 224, 195, 170, 215, 
221, 237, 238, 243, 242, 243, 244, 243, 242, 236, 
231, 215, 174, 130, 143, 184, 208, 224, 183, 172, 
228, 202, 224, 210, 201, 200, 188, 221, 227, 230, 
224, 208, 234, 235, 227, 219, 201, 185, 219, 224, 
217, 174, 193, 200, 210, 211, 208, 200, 158, 171, 
129, 192, 214, 183, 196, 184, 212, 211, 172, 206, 
239, 253, 255, 251, 234, 197, 230, 183, 232, 241, 
242, 229, 203, 170, 222, 200, 226, 229, 233, 228, 
204, 152, 194, 214, 223, 224, 230, 231, 229, 228, 
228, 215, 214, 188, 156, 171, 207, 217, 225, 224, 
223, 219, 199, 219, 236, 202, 166, 227, 247, 247, 
249, 250, 250, 250, 250, 248, 230, 205, 238, 246, 
244, 221, 196, 158, 136, 182, 210, 215, 216, 224, 
224, 230, 224, 223, 200, 180, 205, 212, 229, 233, 
234, 235, 236, 237, 237, 234, 224, 223, 210, 209, 
205, 224, 228, 222, 221, 222, 189, 204, 232, 238, 
241, 241, 240, 234, 212, 237, 250, 247, 252, 250, 
248, 251, 252, 251, 243, 216, 234, 251, 246, 213, 
152, 136, 199, 220, 226, 225, 226, 227, 232, 235, 
220, 204, 230, 241, 240, 240, 233, 207, 224, 238, 
240, 224, 235, 229, 204, 175, 201, 212, 228, 220, 
193, 157, 174, 198, 221, 203, 151, 192, 214, 224, 
221, 220, 196, 169, 199, 156, 203, 236, 222, 176, 
194, 227, 245, 236, 239, 204, 158, 128, 208, 248, 
254, 225, 161, 182, 209, 231, 255, 255, 253, 225, 
182, 191, 220, 235, 213, 225, 188, 232, 231, 208, 
170, 152, 197, 217, 219, 216, 211, 204, 207, 208, 
212, 202, 214, 215, 211, 210, 188, 166, 202, 214, 
213, 214, 200, 164, 207, 239, 193, 224, 253, 255, 
255, 255, 193, 248, 219, 244, 233, 193, 207, 208, 
210, 164, 195, 236, 240, 244, 247, 245, 237, 189, 
165, 147, 155, 222, 237, 219, 209, 192, 225, 238, 
237, 233, 228, 203, 210, 211, 213, 198, 212, 223, 
215, 207, 206, 207, 229, 228, 224, 217, 203, 229, 
216, 231, 241, 239, 226, 211, 233, 232, 214, 153, 
194, 208, 222, 234, 241, 243, 244, 244, 246, 247, 
247, 247, 249, 248, 246, 231, 194, 167, 141, 220, 
228, 231, 232, 214, 233, 234, 236, 197, 225, 240, 
232, 239, 243, 243, 238, 237, 230, 215, 223, 215, 
195, 180, 205, 216, 218, 210, 203, 169, 161, 198, 
235, 253, 254, 254, 253, 251, 238, 250, 249, 248, 
241, 223, 199, 154, 167, 184, 206, 214, 189, 191, 
236, 237, 223, 217, 223, 225, 225, 231, 231, 232, 
233, 213, 229, 241, 242, 241, 209, 187, 224, 224, 
228, 232, 233, 233, 230, 216, 196, 166, 224, 223, 
170, 190, 230, 231, 219, 231, 235, 229, 177, 229, 
240, 243, 238, 237, 215, 154, 180, 231, 252, 254, 
255, 255, 230, 170, 203, 241, 250, 250, 250, 249, 
226, 145, 178, 204, 221, 228, 211, 228, 229, 235, 
235, 230, 223, 222, 191, 213, 224, 225, 214, 213, 
188, 166, 207, 150, 163, 213, 229, 244, 247, 247, 
249, 250, 250, 250, 249, 242, 229, 192, 218, 231, 
232, 196, 228, 227, 159, 182, 201, 213, 223, 231, 
224, 237, 236, 236, 218, 178, 236, 239, 238, 225, 
218, 190, 175, 202, 207, 204, 176, 187, 188, 199, 
177, 207, 214, 218, 220, 218, 211, 198, 214, 215, 
221, 223, 224, 225, 219, 209, 178, 216, 221, 218, 
217, 213, 204, 180, 216, 217, 185, 171, 194, 218, 
224, 223, 207, 183, 235, 249, 252, 253, 252, 253, 
242, 215, 174, 231, 249, 246, 217, 155, 189, 227, 
235, 244, 246, 244, 235, 150, 226, 201, 237, 239, 
230, 166, 179, 216, 240, 243, 246, 249, 250, 250, 
250, 249, 247, 236, 201, 138, 212, 224, 221, 148, 
171, 200, 210, 214, 215, 215, 210, 166, 190, 209, 
220, 221, 205, 235, 250, 254, 255, 255, 253, 252, 
222, 177, 200, 229, 228, 210, 213, 142, 128, 190, 
220, 207, 223, 222, 223, 228, 216, 234, 246, 255, 
255, 253, 253, 255, 250, 235, 200, 185, 225, 241, 
243, 242, 228, 181, 213, 251, 234, 255, 253, 255, 
255, 255, 255, 253, 249, 250, 221, 234, 203, 222, 
221, 204, 149, 171, 210, 211, 206, 205, 185, 134, 
168, 128, 200, 230, 231, 233, 234, 234, 234, 233, 
226, 221, 209, 162, 191, 216, 217, 144, 202, 238, 
246, 248, 246, 249, 249, 247, 249, 239, 217, 208, 
224, 241, 246, 250, 250, 249, 245, 244, 228, 175, 
204, 230, 231, 230, 228, 199, 223, 241, 232, 177, 
218, 247, 251, 249, 248, 242, 153, 230, 228, 185, 
228, 227, 226, 178, 207, 185, 207, 170, 221, 222, 
217, 216, 215, 215, 172, 204, 236, 247, 249, 248, 
246, 172, 210, 236, 237, 236, 235, 231, 171, 223, 
247, 249, 254, 254, 253, 251, 245, 230, 201, 173, 
213, 212, 199, 154, 167, 192, 191, 184, 146, 145, 
157, 182, 197, 203, 204, 189, 175, 209, 240, 248, 
250, 250, 251, 251, 248, 241, 209, 235, 244, 242, 
231, 191, 215, 224, 226, 219, 193, 169, 218, 221, 
142, 160, 207, 241, 242, 253, 255, 255, 243, 251, 
246, 245, 229, 226, 182, 145, 219, 249, 252, 254, 
255, 255, 254, 248, 253, 255, 252, 250, 242, 226, 
178, 146, 223, 234, 233, 230, 229, 203, 166, 181, 
211, 210, 204, 191, 168, 218, 241, 242, 239, 227, 
226, 200, 222, 152, 229, 234, 228, 227, 227, 217, 
139, 128, 156, 185, 208, 209, 212, 212, 213, 208, 
207, 208, 212, 213, 194, 215, 212, 186, 160, 199, 
232, 243, 245, 245, 245, 246, 246, 247, 245, 237, 
223, 193, 157, 209, 229, 238, 241, 242, 242, 241, 
236, 235, 221, 220, 194, 162, 183, 212, 219, 220, 
221, 221, 222, 226, 222, 221, 218, 217, 218, 187, 
165, 142, 200, 215, 239, 197, 162, 137, 179, 228, 
243, 222, 202, 160, 230, 242, 252, 253, 252, 249, 
239, 214, 184, 231, 253, 252, 247, 228, 191, 218, 
235, 234, 233, 232, 208, 193, 170, 216, 238, 239, 
204, 150, 185, 215, 233, 235, 236, 238, 233, 232, 
217, 195, 179, 211, 226, 220, 219, 220, 209, 162, 
204, 222, 236, 237, 231, 230, 210, 190, 216, 236, 
251, 255, 250, 252, 255, 254, 255, 255, 253, 245, 
227, 199, 165, 182, 213, 214, 213, 186, 151, 163, 
202, 226, 215, 230, 238, 239, 238, 239, 241, 239, 
238, 236, 227, 231, 203, 186, 220, 209, 248, 254, 
251, 235, 247, 255, 244, 233, 247, 253, 255, 255 
}
#endif /* MAIN */
;

EXTERN /*__far*/ const short StrataCol[1200]      // ALEXANDER __far raus
#ifdef MAIN
= {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
/* too many color changes
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 
1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 
1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 
2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 
3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 
0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 
3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 
0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 
0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 2, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 
3, 3, 3, 3, 3, 3, 1, 1, 1, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 
2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 
2, 3, 3, 3, 3, 3, 2, 2, 2, 2, 
2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 
1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 2, 2, 2, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 
1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 
1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 
1, 1, 1, 0, 0, 0, 0, 0, 0, 0
*/ 
}
#endif /* MAIN */
;

EXTERN /*__far*/ const short StrataColIndex[4]      // ALEXANDER __far raus
#ifdef MAIN
= {
  15, 16, 17, 18
}
#endif /* MAIN */
;

EXTERN struct NewMenu WCSNewMenus[]
#ifdef MAIN
 =
	{
		{ NM_TITLE, (STRPTR)"Project",		 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"New...",		 (STRPTR)"N", 0, 0, (APTR)(ID_PN_WINDOW) },
		{  NM_ITEM, (STRPTR)"Edit...",		 (STRPTR)"E", 0, 0, (APTR)(ID_PJ_WINDOW) },
		{  NM_ITEM, (STRPTR)"Open...",		 (STRPTR)"O", 0, 0, (APTR)(ID_LOADPROJ) },
		{  NM_ITEM, (STRPTR)"Save",		 (STRPTR)"S", 0, 0, (APTR)(ID_SAVEPROJ) },
		{  NM_ITEM, (STRPTR)"Save As...",	 (STRPTR)"A", 0, 0, (APTR)(ID_SAVEPROJNEW) },
		{  NM_ITEM, (STRPTR)"Load Config",	 (STRPTR)"G", 0, 0, (APTR)(ID_DB_LOADCONFIG) },
		{  NM_ITEM, (STRPTR)"Save Config",	 (STRPTR)"C", 0, 0, (APTR)(ID_DB_SAVECONFIG) },
		{  NM_ITEM, (STRPTR)"Save Screen...",	 (STRPTR)"$", 0, 0, (APTR)(ID_SCREENSAVE) },
		{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Info...",		 (STRPTR)"/", CHECKIT | MENUTOGGLE, 0, (APTR)(ID_INFO) },
		{  NM_ITEM, (STRPTR)"Version...",	 (STRPTR)"?", CHECKIT | MENUTOGGLE, 0, (APTR)(ID_VERSION) },
		{  NM_ITEM, (STRPTR)"Credits...",	 (STRPTR)".", CHECKIT | MENUTOGGLE, 0, (APTR)(ID_CREDITS) },
		{  NM_ITEM, (STRPTR)"Log...",		 (STRPTR)"L", CHECKIT | MENUTOGGLE, 0, (APTR)(ID_LOG) },
		{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Quit...",		 (STRPTR)"Q", 0, 0, (APTR)(MO_EXIT) },
		{  NM_ITEM, (STRPTR)"Iconify...",	 (STRPTR)"I", NM_ITEMDISABLED, 0, (APTR)(ID_ICONIFY) },

		{ NM_TITLE, (STRPTR)"Modules",		 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"DataBase",		 (STRPTR)"1", CHECKIT | MENUTOGGLE, 0, (APTR)((GA_GADNUM(1) | MO_DATABASE)) },
		{  NM_ITEM, (STRPTR)"Data Ops",		 (STRPTR)"2", CHECKIT | MENUTOGGLE, 0, (APTR)((GA_GADNUM(1) | MO_DATAOPS)) },
		{  NM_ITEM, (STRPTR)"Map View",		 (STRPTR)"3", CHECKIT | MENUTOGGLE, 0, (APTR)((GA_GADNUM(1) | MO_MAPPING)) },
		{  NM_ITEM, (STRPTR)"Parameters",	 (STRPTR)"4", CHECKIT | MENUTOGGLE, 0, (APTR)((GA_GADNUM(1) | MO_EDITING)) },
		{  NM_ITEM, (STRPTR)"Render",		 (STRPTR)"5", 0, 0, (APTR)(ID_ES_WINDOW) },
		{  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Motion Editor",	 (STRPTR)"6", 0, 0, (APTR)(ID_EM_WINDOW) },
		{  NM_ITEM, (STRPTR)"Color Editor",	 (STRPTR)"7", 0, 0, (APTR)(ID_EC_WINDOW) },
		{  NM_ITEM, (STRPTR)"Ecosys Editor",	 (STRPTR)"8", 0, 0, (APTR)(ID_EE_WINDOW) },

		{ NM_TITLE, (STRPTR)"Prefs",		 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Preferences...",	 (STRPTR)"P", 0, 0, (APTR)(ID_PR_WINDOW) },
		{  NM_ITEM, (STRPTR)"Screen Mode...",	 (STRPTR)"M", 0, 0, (APTR)(ID_SCRNRESET) },

		{ NM_TITLE, (STRPTR)"Parameters",	 0 , 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Load All...",	 (STRPTR)"(", 0, 0, (APTR)(ID_EP_LOAD) },
		{  NM_ITEM, (STRPTR)"Save All...",	 (STRPTR)")", 0, 0, (APTR)(ID_EP_SAVE) },
		{  NM_ITEM, (STRPTR)"Freeze",		 (STRPTR)"F", 0, 0, (APTR)(ID_EP_FIX) },
		{  NM_ITEM, (STRPTR)"Restore",		 (STRPTR)"R", 0, 0, (APTR)(ID_EP_UNDO) },

		{ NM_END,	NULL,		 0 , 0, 0, 0 },
		{  NM_ITEM, 	NULL,		(STRPTR)"[", 0, 0, 0 },
		{  NM_ITEM,	NULL,		(STRPTR)"]", 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Load Active...",	 (STRPTR)"{", 0, 0, 0 },
		{  NM_ITEM, (STRPTR)"Save Active...",	 (STRPTR)"}", 0, 0, 0 },

		{ NM_END,	NULL,		 0 , 0,	0, 0 },
	}
#endif
; /* WCSNewMenus */

/* These are not being used now - see MapTopoObject.c

EXTERN long StudyVertex;
*/
EXTERN short OldLightingModel;

struct ILBMHeader {
 UBYTE ChunkID[4];
 LONG ChunkSize;
};

struct WcsBitMapHeader {
 USHORT Width, Height;
 SHORT XPos, YPos;
 UBYTE Planes, Masking, Compression, Pad;
 USHORT Transparent;
 UBYTE XAspect, YAspect;
 SHORT PageWidth, PageHeight;
};

struct ZBufferHeader {
 ULONG  Width, Height;
 USHORT VarType, Compression, Sorting, Units;
 float  Min, Max, Bkgrnd, ScaleFactor, ScaleBase;
};

struct LightWaveMotion {
 double XYZ[3], HPB[3], SCL[3];
 long Frame, Linear;
 double TCB[3];
};

struct Gauss {
        double Arand, Nrand, Add, Fac;
        long Seed;
};

#include "GrammarTable.h"
#include "Proto.h"  // Proto.h needs the structures defined above

EXTERN Matx3x3 ScrRotMatx, NoBankMatx;


#ifndef MakeID
   #define MakeID(a,b,c,d) ( (a)<<24 | (b)<<16 | (c)<<8 | (d) )
#endif


#endif
