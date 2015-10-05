// Raster.h
// Header file for Raster objects, World Construction Set v4.
// Built from GeoRaster.cpp by Gary R. Huber on 4/13/98
// Copyright 1998 by Questar Productions. All rights reserved.

#ifndef WCS_RASTER_H
#define WCS_RASTER_H

#include "ImageFormatConfig.h"

#include "Types.h"
#include "PathAndFile.h"
#include "Notify.h"
#include "RasterAnimHost.h"
#include "GeoRegister.h"

#undef RGB

class Joe;
class VectorPoint;
class GeneralEffect;
class Raster;
class Image;
class GeoRaster;
class PathAndFile;
class GraphData;
class RasterAttribute;
class BaseMap;
class ImageLib;
class RasterShell;
class GeoRefShell;
class ImageShell;
class FoliageShell;
class RenderControlGUI;
class ShadowMap3D;
class RenderOpt;
class Renderer;
class rPixelBlockHeader;
class CoordSys;
class PostProcComposite;
class rPixelHeader;
class VectorPolygonList;
class VectorPolygon;
class VectorNode;
class VectorPart;

#define WCS_IMAGES_LOAD_CLEAR		 (1U << 31)
#define WCS_IMAGES_LOAD_CLOSEWINDOWS (1 << 0)

#define WCS_IMAGES_VERSION		1
#define WCS_IMAGES_REVISION		1
#define WCS_FOLIAGE_VERSION		5
#define WCS_FOLIAGE_REVISION	0

#define WCS_IMAGE_ERR	~0

class ImageLib : public SetCritter, public NotifyEx
	{
	public:
		Raster *List;
		Raster *ActiveRaster;
		long MaxTileMemory;
		unsigned long HighestID;
		Raster *StandInRast;

		ImageLib();
		~ImageLib();
		void DeleteAll(void);
		void RemoveAll(EffectsLib *CheckLib, int RemoveUnusedOnly);
		void FreeBands(char FreeLongevity);
		Raster *AddRaster(void);
		Raster *AddRequestRaster(char UseSimpleRequester = 0, Raster ***ManyRasters = NULL, long *NumMany = NULL);
		Raster *AddRaster(char *AddPath, char *AddName, short NonRedundant, short ConfirmFile, short LoadnPrep, char AllowImageManagement = 0);
		long AddRasterReturnID(Raster *AddRast);
		int RemoveRaster(Raster *RemoveMe, EffectsLib *CheckLib);
		void SetActive(Raster *NewActive);
		Raster *GetActive(void) {return(ActiveRaster);};
		Raster *FindByName(char *FindPath, char *FindName);
		Raster *FindByUserName(char *FindName);
		Raster *FindByID(unsigned long MatchID);
		Raster *FindNextByName(Raster *StartHere, char *FindPath, char *FindName);
		RasterAttribute *MatchRasterSetShell(unsigned long MatchID, RasterShell *ShellSource, RasterAnimHost *HostSource);
		RasterAttribute *MatchRasterAttributeSetHost(unsigned long MatchID, char AttributeType, RasterAnimHost *HostSource);
		// returns first Raster to have this attribute type
		Raster *MatchAttribute(char MatchType);
		Raster *MatchShell(RasterShell *FindMe, char MatchType);
		Raster *MatchRasterShell(RasterShell *FindMe, int SearchDeep);
		int MatchRaster(Raster *FindMe);
		Raster *MatchNameMakeRaster(Raster *MatchMe);
		void InitRasterIDs(void);
		void ClearRasterIDs(void);
		void SortImagesByAlphabet(void);
		void MungPaths(Project *ProjHost);
		void UnMungPaths(Project *ProjHost);
		void LinkDissolvesToRasters(void);
		Raster *CheckDissolveRecursion(void);
		Raster *GetFirstRast(void) {return (List);};
		Raster *GetNextRast(Raster *Me);
		void ApplicationSetTime(double Time, long Frame, double FrameFraction);
		long GetImageCount(void);
		int InitFoliageRasters(double Time, double FrameRate, int EcosystemsEnabled, int FoliageEffectsEnabled,
			int StreamEffectsEnabled, int LakeEffectsEnabled, Renderer *Rend);
		int InitFoliageRastersOneAtATime(double Time, double FrameRate, int EcosystemsEnabled, int FoliageEffectsEnabled,
			int StreamEffectsEnabled, int LakeEffectsEnabled, int LabelsEnabled, Raster *&Current, AnimColorTime *&ReplaceRGB, int &Shade3D);
		void ClearImageRasters(char FreeLongevity);
		void SetAllMemoryLongevity(char NewLongevity);
		void SetAllLoadFast(char NewLoadFast);
		ULONG Load(FILE *ffile, ULONG ReadSize, struct ChunkIODetail *Detail);
		ULONG Save(FILE *ffile, struct ChunkIODetail *Detail, int IDsInitialized = 0);

		// these are in EffectsDispatch.cpp for the time being
		virtual void SetParam(int Notify, ...);
		virtual void GetParam(void *Value, ...);

		// for removing rasters from SceneView
		int RemoveRAHost(RasterAnimHost *RemoveMe, EffectsLib *CheckLib);

		Raster *FetchStandInRast(void) {if(StandInRast) return(StandInRast); else return(StandInRast = CreateStandInRast());};
		Raster *CreateStandInRast(void);

	}; // class ImageLib

#define WCS_RASTER_MAX_BANDS	20
#define WCS_RASTER_TNAIL_SIZE	100
#define WCS_IMAGE_MAXNAMELENGTH	80
enum
	{
	WCS_RASTER_BANDSET_BYTE,
	WCS_RASTER_BANDSET_FLOAT,
	WCS_RASTER_BANDSET_SHORT,
	WCS_RASTER_BANDSET_MAXTYPES
	}; // Raster Band sets - byte or float

#define WCS_RASTER_BANDSET_ROWFLOAT	100		// just an arbitrary number greater than WCS_RASTER_BANDSET_MAXTYPES
#define WCS_RASTER_BANDSET_ALTBYTE	101		// just an arbitrary number greater than WCS_RASTER_BANDSET_MAXTYPES
#define WCS_RASTER_BANDSET_ALTFLOAT	102		// just an arbitrary number greater than WCS_RASTER_BANDSET_MAXTYPES

#define WCS_RASTER_INITFLAGS_IMAGELOADED		(1 << 0)
#define WCS_RASTER_INITFLAGS_FOLIAGELOADED		(1 << 1)
#define WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED	(1 << 2)
#define WCS_RASTER_INITFLAGS_FORCERELOAD		(1 << 3)
#define WCS_RASTER_INITFLAGS_DOWNSAMPLED		(1 << 4)

#define WCS_RASTER_FILEFLAGS_BASIC			(1 << 0)
#define WCS_RASTER_FILEFLAGS_TNAIL			(1 << 1)
#define WCS_RASTER_FILEFLAGS_ATTRIBUTE		(1 << 2)
#define WCS_RASTER_FILEFLAGS_BANDDATA		(1 << 3)
#define WCS_RASTER_FILEFLAGS_SMALLER		(1 << 4)

#define WCS_RASTER_FILEFOLIAGE	(WCS_RASTER_FILEFLAGS_BANDDATA | WCS_RASTER_FILEFLAGS_SMALLER)
#define WCS_RASTER_FILENORMAL	(WCS_RASTER_FILEFLAGS_BASIC | WCS_RASTER_FILEFLAGS_TNAIL | WCS_RASTER_FILEFLAGS_ATTRIBUTE)

enum
	{
	WCS_RASTER_IMAGE_BAND_RED,
	WCS_RASTER_IMAGE_BAND_GREEN,
	WCS_RASTER_IMAGE_BAND_BLUE,
	WCS_RASTER_IMAGE_BAND_ALPHA
	}; // raster names for image UBYTEs

enum
	{
	WCS_RASTER_IMAGE_FBAND_RED,
	WCS_RASTER_IMAGE_FBAND_GREEN,
	WCS_RASTER_IMAGE_FBAND_BLUE,
	WCS_RASTER_IMAGE_FBAND_ALPHA
	}; // raster names for image floats

enum
	{
	WCS_RASTER_RENDER_BAND_RED,
	WCS_RASTER_RENDER_BAND_GREEN,
	WCS_RASTER_RENDER_BAND_BLUE,
	WCS_RASTER_RENDER_BAND_ANTIALIAS,
	WCS_RASTER_RENDER_BAND_FOLIAGERED,
	WCS_RASTER_RENDER_BAND_FOLIAGEGREEN,
	WCS_RASTER_RENDER_BAND_FOLIAGEBLUE,
	WCS_RASTER_RENDER_BAND_FOLIAGEANTIALIAS
	}; // raster names for renderer UBYTEs

enum
	{
	WCS_RASTER_RENDER_BAND_ZBUF,
	WCS_RASTER_RENDER_BAND_FOLIAGEZBUF,
	WCS_RASTER_RENDER_BAND_ELEVATION,
	WCS_RASTER_RENDER_BAND_SLOPE
	}; // raster names for renderer floats

enum
	{
	WCS_RASTER_SHADOWMAP_BAND_NEAROPACITY,
	WCS_RASTER_SHADOWMAP_BAND_FAROPACITY
	}; // raster names for shadow map bytes

enum
	{
	WCS_RASTER_SHADOWMAP_BAND_ZBUF,
	WCS_RASTER_SHADOWMAP_BAND_DEPTH
	}; // raster names for shadow map floats

enum
	{
	WCS_RASTER_FOLIAGE_BAND_RED,
	WCS_RASTER_FOLIAGE_BAND_GREEN,
	WCS_RASTER_FOLIAGE_BAND_BLUE,
	WCS_RASTER_FOLIAGE_BAND_ZOFF
	}; // raster names for foliage UBYTEs

enum
	{
	WCS_RASTER_FOLIAGE_BAND_COVERAGE
	}; // raster names for foliage floats

enum
	{
	WCS_RASTER_FOLIAGE_ARRAY_MIDPT,
	WCS_RASTER_FOLIAGE_ARRAY_SPAN
	}; // raster names for foliage row float arrays

enum
	{
	WCS_RASTER_GEOREF_BAND_RED,
	WCS_RASTER_GEOREF_BAND_GREEN,
	WCS_RASTER_GEOREF_BAND_BLUE
	}; // raster names for georef

enum
	{
	WCS_RASTER_EFFECT_BAND_NORMAL = 0
	}; // raster names for effects

#define WCS_RASTER_FOLIAGE_NUMBANDS_UBYTE		4
#define WCS_RASTER_FOLIAGE_NUMBANDS_FLOAT		1
#define WCS_RASTER_RENDER_NUMBANDS_UBYTE		6
#define WCS_RASTER_RENDER_NUMBANDS_FLOAT		1
#define WCS_RASTER_IMAGE_NUMBANDS_UBYTE			3
#define WCS_RASTER_IMAGE_NUMBANDS_FLOAT			0
#define WCS_RASTER_ZBUFFER_NUMBANDS_UBYTE		0
#define WCS_RASTER_ZBUFFER_NUMBANDS_FLOAT		1
#define WCS_RASTER_GEOREF_NUMBANDS_UBYTE		3
#define WCS_RASTER_GEOREF_NUMBANDS_FLOAT		0

enum
	{
	WCS_RASTER_ATTRIBUTE_NONE,
	WCS_RASTER_ATTRIBUTE_SEQUENCE,
	WCS_RASTER_ATTRIBUTE_DISSOLVE,
	WCS_RASTER_ATTRIBUTE_FOLIAGE,
	WCS_RASTER_ATTRIBUTE_BACKGROUND,
	WCS_RASTER_ATTRIBUTE_CELESTIAL,
	WCS_RASTER_ATTRIBUTE_STARFIELD,
	WCS_RASTER_ATTRIBUTE_COLORMAPEFFECT,
	WCS_RASTER_ATTRIBUTE_CLOUDEFFECT,
	WCS_RASTER_ATTRIBUTE_WAVEEFFECT,
	WCS_RASTER_ATTRIBUTE_FOGEFFECT,
	WCS_RASTER_ATTRIBUTE_ILLUMINATIONEFFECT,
	WCS_RASTER_ATTRIBUTE_RASTERTA,
	WCS_RASTER_ATTRIBUTE_TEMPERATURE,
	WCS_RASTER_ATTRIBUTE_MOISTURE,
	WCS_RASTER_ATTRIBUTE_ECOTEXTURE,
	WCS_RASTER_ATTRIBUTE_3DTEXTURE
	}; // raster attributes

enum
	{
	WCS_RASTER_LONGEVITY_SHORT,
	WCS_RASTER_LONGEVITY_MEDIUM,
	WCS_RASTER_LONGEVITY_LONG,
	WCS_RASTER_LONGEVITY_FORCEFREE
	}; // Image longevity - persistence in memory once loaded

enum
	{
	WCS_RASTERSHELL_TYPE_RASTER,
	WCS_RASTERSHELL_TYPE_GEOREF,
	WCS_RASTERSHELL_TYPE_IMAGE,
	WCS_RASTERSHELL_TYPE_FOLIAGE,
	WCS_RASTERSHELL_TYPE_SEQUENCE,
	WCS_RASTERSHELL_TYPE_DISSOLVE,
	WCS_RASTERSHELL_TYPE_COLORCONTROL,
	WCS_RASTERSHELL_TYPE_RENDER,
	WCS_RASTERSHELL_TYPE_IMAGEMANAGER
	}; // raster shell types

enum
	{
	WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_NOIMAGE,
	WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD,
	WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE
	}; // sequence end control

enum
	{
	WCS_RASTERSHELL_FUNCTYPE_OKREMOVERASTER,
	WCS_RASTERSHELL_FUNCTYPE_REMOVERASTER,
	WCS_RASTERSHELL_FUNCTYPE_EDIT,
	WCS_RASTERSHELL_FUNCTYPE_GETNAME,
	WCS_RASTERSHELL_FUNCTYPE_GETENABLED,
	WCS_RASTERSHELL_FUNCTYPE_GETTYPESTRING,
	WCS_RASTERSHELL_FUNCTYPE_GETANIMCRITTERTYPESTRING,
	WCS_RASTERSHELL_FUNCTYPE_GETAPPTYPESTRING
	}; // shell functions

class RasterShell
	{
	public:
		unsigned long ID;
		Raster *Rast;
		RasterAnimHost *Host;

		RasterShell();
		RasterShell(Raster *RastSource);
		virtual ~RasterShell();
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_RASTER);};
		Raster *GetRaster(void) {return (Rast);};
		virtual void SetDefaults(void) {return;};
		Raster *SetRaster(Raster *RastSource) {return (Rast = RastSource);};
		void SetHost(RasterAnimHost *HostSource) {Host = HostSource;};
		void SetHostNotify(RasterAnimHost *HostSource);
		RasterAnimHost *GetHost(void)	{return (Host);};
		unsigned long GetID(void) {return (ID);};
		inline unsigned long GetRasterID(void);
		void Copy(RasterShell *CopyTo, RasterShell *CopyFrom);
		int CheckOKRemove(void);
		virtual int CheckOKRemove(Raster *CheckMe) {return (1);};
		virtual int RemoveRaster(Raster *RemoveMe, int Iteration) {return (1);};
		virtual int IsInternal(void) {return(0);};
		char *GetHostName(void);
		char *GetHostTypeString(void);
		int GetHostEnabled(void);
		void EditHost(void);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
	}; // class RasterShell

#define WCS_GEOREFSHELL_BOUNDSTYPE_EDGES	0
#define WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS	1

class GeoRefShell : public RasterShell
	{
	public:
		GeoRegister GeoReg;
		char BoundsType;

		GeoRefShell();
		GeoRefShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_GEOREF);};
		virtual void SetDefaults(void);
		void Copy(GeoRefShell *CopyTo, GeoRefShell *CopyFrom);
		void SetBounds(double LatRange[2], double LonRange[2]);
		void ShiftBounds(double LatRange[2], double LonRange[2]);
		int SampleLatLonElev(double X, double Y, double &Lat, double &Lon, double &Elev, int &ElevReject);
		int SampleXY(double Lat, double Lon, double &X, double &Y);
		virtual int IsInternal(void) {return(1);};
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class GeoRefShell

class ColorControlShell : public RasterShell
	{
	public:
		short RGB[3][3];	// 0 = max transparent, 1 = min transparent, 2 = replacement color for thumbnail
		char UseAsColor, 		// UseAsColor: 0 = gray, 1 = color, 2 = color+ (for more than 3 bands)
			GrayAutoRange,		// expands gray images to full range of luminance values
			DisplayRGBSet,		// determines which set of rgb values are displayed
			ShowTransparency,	// determines if transparent color range is shown in a different color in thumbnails
			UseBandAssignment,	// can either use band assignments or functions for mixing bands
			UseBandAs[3];		// optional band assignments
		double BandFactor[3][WCS_RASTER_MAX_BANDS];

		ColorControlShell();
		ColorControlShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_COLORCONTROL);};
		virtual void SetDefaults(void);
		void SetUseAsColor(char NewUseAsColor) {UseAsColor = NewUseAsColor;};
		void SetGrayAutoRange(char NewGrayAutoRange) {GrayAutoRange = NewGrayAutoRange;};
		virtual int IsInternal(void) {return(1);};
		void Copy(ColorControlShell *CopyTo, ColorControlShell *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class ColorControlShell

class ImageShell : public RasterShell
	{
	public:
		short Color;

		ImageShell();
		ImageShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_IMAGE);};
		virtual void SetDefaults(void);
		void Copy(ImageShell *CopyTo, ImageShell *CopyFrom);
		void SetColor(short NewColor) {Color = NewColor;};
		unsigned long Save(FILE *ffile, short SaveID);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class ImageShell

class FoliageShell : public ImageShell
	{
	public:
		double HeightPct, DensityPct;
		char PosShade, Shade3D, Colors;		// Colors is true if color image and false if gray

		FoliageShell();
		FoliageShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_FOLIAGE);};
		virtual void SetDefaults(void);
		void Copy(FoliageShell *CopyTo, FoliageShell *CopyFrom);
		void SetColors(char NewColors) {Colors = NewColors;};
		void SetPosShade(char NewPosShade) {PosShade = NewPosShade;};
		void SetShade3D(char NewShade3D) {Shade3D = NewShade3D;};
		void SetDensityPct(double NewDensity) {DensityPct = NewDensity;};
		void SetHeightPct(double NewHeight) {HeightPct = NewHeight;};
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class FoliageShell

class SequenceShell : public RasterShell
	{
	public:
		double StartFrame, EndFrame, Duration, StartSpace, EndSpace;
		long StartImage, EndImage;
		double Speed, NumLoops;
		char StartBehavior, EndBehavior;
		PathAndFile PAF;

		RasterShell *NextSeq, *PrevSeq;		// these will be created as SequenceShells

		SequenceShell();
		SequenceShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_SEQUENCE);};
		virtual void SetDefaults(void);
		virtual int IsInternal(void) {return(1);};
		void SetSeqPath(char *Set) {(void)PAF.SetPath(Set);};
		const char *GetSeqPath(void) {return (PAF.GetPath());};
		void GetOpenFileNames(char *FirstName, char *LastName, double &Offset, double SequenceFrame);
		void SetSeqName(char *Set) {(void)PAF.SetName(Set);};
		const char *GetSeqName(void) {return (PAF.GetName());};
		void Copy(SequenceShell *CopyTo, SequenceShell *CopyFrom);
		void AdjustSubsequent(void);
		FILE *OpenFile(double Time, double TimeInterval);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource);

	}; // class SequenceShell

class DissolveShell : public RasterShell
	{
	public:
		long StartFrame, EndFrame, Duration;
		char EaseIn, EaseOut;
		Raster *Target;
		unsigned long TargetID;

		RasterShell *NextDis, *PrevDis;		// these will be created as DissolveShells

		DissolveShell();
		DissolveShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_DISSOLVE);};
		virtual void SetDefaults(void);
		virtual int IsInternal(void) {return(1);};
		char *GetDisName(void);
		Raster *GetTarget(void) {return (Target);};
		unsigned long GetTargetID(void) {return (TargetID);};
		void GetEndMembers(Raster *&FirstRast, Raster *&LastRast, double &Offset, double DissolveFrame);
		void SetTarget(Raster *NewRast) {Target = NewRast;};
		void Copy(DissolveShell *CopyTo, DissolveShell *CopyFrom);
		virtual int CheckOKRemove(Raster *CheckMe);
		virtual int RemoveRaster(Raster *RemoveMe, int Iteration);
		void AdjustSubsequent(void);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource);

	}; // class DissolveShell

enum
	{
	WCS_IMAGEMGRSHELL_TILINGTYPE_AUTO,
	WCS_IMAGEMGRSHELL_TILINGTYPE_MANUAL
	}; // tiling control types

enum
	{
	WCS_IMAGEMGRSHELL_VIRTRESTYPE_PIXELS,
	WCS_IMAGEMGRSHELL_VIRTRESTYPE_SCALINGFACTOR,
	WCS_IMAGEMGRSHELL_VIRTRESTYPE_GROUNDUNITS
	}; // tiling control types

class ImageManagerShell : public RasterShell
	{
	public:
		char TilingControlType, VirtResEnabled, VirtResType;
		unsigned long AutoTileSizeX, AutoTileSizeY, 
			ManualTileSizeX, ManualTileSizeY, VirtResX, VirtResY;

		ImageManagerShell();
		ImageManagerShell(Raster *RastSource);
		virtual char GetType(void) {return (WCS_RASTERSHELL_TYPE_IMAGEMANAGER);};
		virtual void SetDefaults(void);
		virtual int IsInternal(void) {return(1);};
		void Copy(ImageManagerShell *CopyTo, ImageManagerShell *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long GetPreferredTileSizeX(void) {return(TilingControlType == WCS_IMAGEMGRSHELL_TILINGTYPE_AUTO ? AutoTileSizeX : ManualTileSizeX);}
		unsigned long GetPreferredTileSizeY(void) {return(TilingControlType == WCS_IMAGEMGRSHELL_TILINGTYPE_AUTO ? AutoTileSizeY : ManualTileSizeY);}

	}; // class ImageManagerShell

/*************************************************************************/

class FoliageShellLink
	{
	public:

	FoliageShell *Shell;
	FoliageShellLink *Next;

	FoliageShellLink(FoliageShell *SetShell);
	}; // class FoliageShellLink

/*************************************************************************/

class RasterAttribute
	{
	public:
		RasterShell *Shell;
		RasterAttribute *Next;

		RasterAttribute();
		RasterAttribute(RasterShell *AddShell, char AddType, Raster *RastSource, RasterAnimHost *HostSource);
		~RasterAttribute();
		RasterShell *GetShell(void) {return(Shell);};
		void Copy(RasterAttribute *CopyTo, RasterAttribute *CopyFrom);
		int CheckOKRemove(void);
		int CheckOKRemove(Raster *CheckMe);
		int RemoveRaster(Raster *RemoveMe);
		char *GetName(void);
		char *GetTypeString(void);
		int GetEnabled(void);
		int IsInternal(void) {return(Shell ? Shell->IsInternal(): 0);};
		void Edit(void);
		Raster *TraceDissolveRasterMatch(Raster *MatchRast, short Iteration);
		RasterShell *RemovePartialSequence(SequenceShell *RemoveMe);
		RasterShell *RemovePartialDissolve(DissolveShell *RemoveMe);
		RasterShell *AdjustSequenceOrder(SequenceShell *AdjustMe, long Adjustment);
		RasterShell *AdjustDissolveOrder(DissolveShell *AdjustMe, long Adjustment);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource);

	}; // class RasterAttribute

/*************************************************************************/

class Thumbnail
	{
	public:
		UBYTE *TNail[3];
		unsigned char TNailPadX, TNailPadY;

		Thumbnail();
		~Thumbnail();
		void Copy(Thumbnail *CopyTo, Thumbnail *CopyFrom);
		UBYTE **AllocTNails(void);
		UBYTE *AllocTNail(char Band);
		void ClearTNails(void);
		void ClearTNail(char Band);
		void ClearPadArea(void);
		int TNailsValid(void);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

	}; // class Thumbnail

/*************************************************************************/

class TriStimulus
	{
	public:
		unsigned char RGB[3];
	}; // struct TriStimulus

#define WCS_MAX_BUFFERNODE_NAMELEN	24

class BufferNode
	{
	public:
		char Name[WCS_MAX_BUFFERNODE_NAMELEN];
		char Type, Index, *RowData;
		long RowDataSize;
		void *Buffer;
		BufferNode *Next;
		FILE *fFile;

		BufferNode();
		~BufferNode();
		BufferNode(char *NewName, char NewType);
		BufferNode *AddBufferNode(char *NewName, char NewType);	// affects entire chain
		void *FindBuffer(char *FindName, char FindType);	// affects entire chain
		BufferNode *FindBufferNode(char *FindName, char FindType);	// affects entire chain
		int PrepToSave(RenderOpt *Opt, long Frame, long Width, long BeforePost);
		void CleanupFromSave(void);
		void RemoveTempFiles(RenderOpt *Opt, long Frame);	// affects entire chain
		char *GetDataLine(long Line, char CheckType);

		static char GetStandardBufferType(char *BufName);

	}; // class BufferNode

class RLASampleData
	{
	public:
		unsigned long Channels;
		unsigned char Alpha;
		long X, Y, ImageWidth, ImageHeight;
		float Z;
		unsigned char mtl_id;
		unsigned short node_id;
		float u;
		float v;
		unsigned long normal;
		unsigned char realpix[4];	// r, g, b, e
		unsigned char coverage;
		unsigned short rend_id;
		unsigned char color[3];		// r, g, b
		unsigned char transp[3];	// r, g, b
		unsigned char weight[3];	// r, g, b
		float veloc_x;
		float veloc_y;
		unsigned short mask;

		RLASampleData();

	}; // class RLASampleData

/*************************************************************************/

#define SPAN(x) RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][x]
#define MIDPT(x) RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][x]

class ImageCacheControl
	{
	public:
		unsigned long QueryOnlyFlags;
		unsigned long NativeTileWidth, NativeTileHeight;
		unsigned long LoadXOri, LoadYOri, LoadWidth, LoadHeight;
		char ImageManagementEnable, LoadingSubTile;
		ImageCacheControl();
	}; // ImageCacheControl

// resolution at which (in either direction) we auto-switch to Image memory management (for georeferenced images)
#define WCS_RASTER_TILECACHE_AUTOENABLE_GEO_THRESH	1600
// resolution at which (in either direction) we auto-switch to Image memory management (for non-georeferenced images)
#define WCS_RASTER_TILECACHE_AUTOENABLE_THRESH	3000
// default number of tiles in cache
#define WCS_RASTER_TILECACHE_NUMTILES_DEFAULT	9

#define WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE		0x01
#define WCS_BITMAPS_IMAGECAPABILITY_HASNATIVETILES		0x02
#define WCS_BITMAPS_IMAGECAPABILITY_SUPPORTVIRTUALRES	0x04
#define WCS_BITMAPS_IMAGECAPABILITY_EFFICIENTTHUMBNAIL	0x08

//#define WCS_BITMAPS_IMAGECAPABILITY_		0x

class Raster : public RasterAnimHost
	{
	public:
		double AverageCoverage, AverageBand[3];	// these will store average red, green and blue, range 0-1
		unsigned long ID, InitFlags;
		long Rows, Cols, ByteBandSize, FloatBandSize, ShortBandSize, AltByteBandSize, AltFloatBandSize, Row, Col, RasterCell, AltData, TNRowsReq, TNColsReq, TNRealRows, TNRealCols;
		char ConstructError, ByteBands, FloatBands, FormatLastLoaded, SaveFormat, Longevity, LoadFast, AlphaAvailable,
			AlphaEnabled, DissolveEnabled, SequenceEnabled, ColorControlEnabled, ImageManagerEnabled, Enabled;
		CornerMask Mask;
		UBYTE *ByteMap[WCS_RASTER_MAX_BANDS], *AltByteMap, *Red, *Green, *Blue, *AABuf;
		float *FloatMap[WCS_RASTER_MAX_BANDS], *AltFloatMap, *RowFloatArray[2], *ZBuf, *ReflectionBuf, *NormalBuf[3];
		unsigned short *ShortMap[WCS_RASTER_MAX_BANDS], *ExponentBuf;
		long *RowZip;
		rPixelBlockHeader *rPixelBlock;
		VectorPolygonList **PolyListBlock;
		RasterAttribute *Attr;
		Raster *Next, *Smaller;
		Thumbnail *Thumb;
		unsigned long ThumbDisplayListNum;
		GeoRefShell *LoaderGeoRefShell;
		BufferNode *Buffers;
		PathAndFile PAF;
		char Name[WCS_IMAGE_MAXNAMELENGTH];

		// for compositing-on-load callbacks
		PostProcComposite *LoadCompositer;

		// for Tiling identification and such support
		unsigned long ImageCapabilityFlags;
		// both must be non-zero if supported
		// reused as tile X and Y offset when Raster is a member of TileCache set
		unsigned long NativeTileWidth, NativeTileHeight;
		// double-linked list of cached tiles using Raster::CacheNext/CachePrev pointer, sorted MRU first
		static Raster *TileCache; // static, one copy is shared by all Rasters but only scoped to Raster object
		Raster *CacheNext, *CachePrev; // one pair per Raster object (Next pointer will be used to point to parent raster of tile for identification)
		// cache memory consumption control (static, one copy is shared by all Rasters but only scoped to Raster object)
		static unsigned long CacheCurrentMemorySize;

		Raster();
		~Raster();
		void Copy(Raster *CopyTo, Raster *CopyFrom, int CopyExternalAttributes = 0);
		void SetID(unsigned long NewID) {ID = NewID;};
		unsigned long GetID(void) {return (ID);};
		int SetToTime(double Time);
		UBYTE **CreateImageRGBThumbNail(void);
		UBYTE **CreateRenderRGBThumbNail(void);
		UBYTE **CreateRenderMergedRGBThumbNail(void);
		UBYTE **CreateFoliageRGBThumbNail(void);
		UBYTE **CreateFoliageZOffThumbNail(void);
		UBYTE **CreateFoliageCovgThumbNail(void);
		UBYTE **CreateEffectThumbNail(void);
		UBYTE **CreateThumbNail(char BandType, void *Band1, void *Band2, void *Band3, unsigned short *OptExponBuf = NULL);
		Thumbnail *CreateThumbnail(void);
		UBYTE **CreateMergedThumbNail(UBYTE *Band1, UBYTE *Band2, UBYTE *Band3, float *Depth,
			UBYTE *AltBand1, UBYTE *AltBand2, UBYTE *AltBand3, float *AltDepth);
		UBYTE **CreateFragThumbnail(rPixelHeader *HeaderArray);
		float FindMaxMinFloatValues(float *Band, float &MaxFlt, float &MinFlt);
		int CheckOKRemove(void);
		int CheckImageInUse(void);
		int CheckOKRemove(Raster *CheckMe);
		int RemoveRaster(Raster *RemoveMe);
		int ThumbnailValid(void)		{return (Thumb && Thumb->TNail[0] && Thumb->TNail[1] && Thumb->TNail[2]);};
		Thumbnail *AllocThumbnail(void);
		void ClearThumbnail(void);
		int BandValid(void *Band) {return (Band ? 1: 0);};
		long SampleBytePoint(double dX, double dY, UBYTE *Band1, UBYTE *Band2, UBYTE *Band3, UBYTE &RedVal, UBYTE &GreenVal, UBYTE &BlueVal, unsigned short *OptExponBuf = NULL);
		long SampleFloatPoint(double dX, double dY, float Base,  float Scale, float *Band1, UBYTE &RedVal, UBYTE &GreenVal, UBYTE &BlueVal);
		long SampleFloatPoint(double dX, double dY, float *Band1, float &Value);
		void FreeAllBands(char FreeLongevity);
		void NullMapPtrs(void);
		void *AllocBuffer(BufferNode *BufNode);
		UBYTE *AllocByteBand(char Band);
		UBYTE *ClearByteBand(char Band);
		void FreeByteBand(char Band);
		UBYTE *AllocAltByteBand(long NewRows, long NewCols);
		UBYTE *ClearAltByteBand(void);
		void FreeAltByteBand(void);
		float *AllocFloatBand(char Band);
		float *ClearFloatBand(char Band);
		float *ClearFloatBandValue(char Band, float Value);
		void FreeFloatBand(char Band);
		float *AllocAltFloatBand(long NewRows, long NewCols);
		float *ClearAltFloatBand(void);
		void FreeAltFloatBand(void);
		float *AllocRowFloatArray(char Band);
		void FreeRowFloatArray(char Band);
		unsigned short *AllocShortBand(char Band);
		unsigned short *ClearShortBand(char Band);
		void FreeShortBand(char Band);
		int AllocFoliageBands(void);
		int FoliageBandsValid(void);
		int AllocRender3DBands(void);
		void ClearRender3DBands(void);
		int AllocRGBBands(void);	// allocates and clears RGB bands, sets Red, Green, Blue
		int AllocShadow3DBands(void);
		long *AllocRowZip(void);
		rPixelBlockHeader *AllocPixelFragmentMap(void);
		void FreePixelFragmentMap(void);
		rPixelBlockHeader *ClearPixelFragMap(void);
		VectorPolygonList **AllocPolyListMap(void);
		void FreePolyListMap(void);
		VectorPolygonList **ClearPolyListMap(void);
		RasterAttribute *AddAttribute(void);
		RasterAttribute *AddAttribute(char AddType, RasterShell *AddShell = NULL, RasterAnimHost *HostSource = NULL);
		RasterShell *MatchRasterShell(unsigned long MatchID);
		RasterAttribute *MatchRasterShell(RasterShell *MatchShell, int SearchDeep = 0);
		RasterAttribute *MatchAttribute(char MatchType);
		RasterAttribute *MatchNextAttribute(char MatchType, RasterAttribute *Current);
		int MatchPathAndName(char *FindPath, char *FindName);
		char *SetUniqueName(ImageLib *Library, char *NameBase);
		int RemoveAttribute(char RemoveShellType, int RemoveAll);
		int RemoveAttribute(RasterShell *RemoveShell);
		int RemoveAttribute(RasterAttribute *RemoveAttr);
		char *SetDefaultName(void);
		char *GetUserName(void) {return (Name);};
		char *SetUserName(char *NewName);
		int LoadToRender(double RenderTime, double FrameRate, unsigned long LocalInitFlags);
		int LoadToComposite(double RenderTime, double FrameRate, PostProcComposite *Compositer, long OutputCols, long OutputRows);
		int LoadnPrepImage(int NameOnly, int UpdateCoordSys, char AllowImageManagement = 0);
		int LoadnProcessImage(int ReplaceTransparent, char *SuppliedName = NULL);
		int CopyBitmaps(Raster *Source, double SourceWt, int AsFoliage);
		int ColorToGray(Raster *Source);
		int GrayToColor(Raster *Source);
		int CopyAlphaToCoverage(UBYTE *Source);
		int CopyCoverageToAlpha(void);
		int CopyNonRGBBands(Raster *Source, double SourceWt);
		int CopyByteMap(UBYTE *Dest, UBYTE *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt);
		int CopyByteMapMerge(Raster *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt);
		int CopyRenderMapMerge(double SourceWt);
		int CopyFloatMap(float *Dest, float *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt);
		int CreateFoliageBands(void);
		void CreateCoverage(void);
		int CreateZOffset(void);
		void CreateMidPtSpan(void);
		void CopyImageToShort(USHORT *DBits, short ColorImage, USHORT FillVal);
		int CreateSmallerFoliage(void);
		int CreateSmallerImage(void);
		void ComputeAverageBands(void);
		int DownsampleFoliage(Raster *Parent, short ColorImage, double Dx, double Dy);
		int DownsampleImage(Raster *Parent, short ColorImage, double Dx, double Dy);
		USHORT AddLuma(USHORT *DBits, short ColorImage);
		USHORT BoxFilter(USHORT *SBits, USHORT *DBits, short FiltSize);
		USHORT ImageGradient(USHORT *DBits, USHORT FillVal);
		USHORT MinInline(USHORT *DBits, long Zip, long x, long y, USHORT MaxSearch);
		USHORT MinDiag(USHORT *DBits, long Zip, long x, long y, USHORT MaxSearch);
		void BlurByteFloatMaps(char Band);
		int GetPreppedStatus(void);
		char GetEnabled(void) {return (Enabled);};
		void SetEnabled(char NewState) {Enabled = NewState;};
		void SetAlphaEnabled(char NewState) {AlphaEnabled = NewState;};
		void SetColorControlEnabled(char NewState) {ColorControlEnabled = NewState;};
		unsigned long TestInitFlags(unsigned long TestMe) {return (TestMe & InitFlags);};
		int TestAllInitFlags(unsigned long TestMe);
		void SetInitFlags(unsigned long SetMe) { InitFlags |= SetMe;};
		void ClearInitFlags(void) { InitFlags = 0;};
		int RemovePartialSequence(SequenceShell *RemoveMe);
		int RemovePartialDissolve(DissolveShell *RemoveMe);
		int AdjustSequenceOrder(SequenceShell *AdjustMe, long Adjustment);
		int AdjustDissolveOrder(DissolveShell *AdjustMe, long Adjustment);
		int SaveImage(int SaveAsNamed);
		short OpenBitmaps(void);	// Bands, Rows, Cols and Mapsize are all in this object
		const char *GetPath(void)	{return(PAF.GetPath());};
		const char *GetName(void)	{return(PAF.GetName());};
		char *GetPathAndName(char *Dest) {return(PAF.GetPathAndName(Dest));};
		long GetWidth(void)	{return(Cols);};
		long GetHeight(void)	{return(Rows);};
		char GetColors(void)	{return(ByteBands);};
		char GetAlphaStatus(void) {return (AlphaAvailable && AlphaEnabled);};
		char GetIsColor(void);
		char GetLongevity(void) {return (Longevity);};
		void SetLongevity(char NewLongevity) {Longevity = NewLongevity;};
		char GetLoadFast(void) {return (LoadFast);};
		void SetLoadFast(char NewLoadFast) {LoadFast = NewLoadFast;};
		unsigned long Save(FILE *ffile, unsigned long SaveFlags);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, unsigned long LoadFlags, int Validate);
		unsigned long SaveData(FILE *ffile, unsigned long SaveFlags);
		unsigned long LoadData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long LoadTNail(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long SaveBandData(FILE *ffile, char Band, char BandType);
		unsigned long LoadBandData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SaveFoliage(void);
		unsigned long FoliageRaster_Save(FILE *ffile);
		int LoadFoliage(void);
		unsigned long FoliageRaster_Load(FILE *ffile, ULONG ReadSize);
		int SaveShadow3D(char *SuppliedName, ShadowMap3D *SM3D);
		unsigned long ShadowRaster3D_Save(FILE *ffile, ShadowMap3D *SM3D);
		unsigned long SaveShadowData3D(FILE *ffile, ShadowMap3D *SM3D);
		int LoadShadow3D(char *SuppliedName, ShadowMap3D *SM3D);
		unsigned long ShadowRaster3D_Load(FILE *ffile, ULONG ReadSize, ShadowMap3D *SM3D);
		unsigned long LoadShadowData3D(FILE *ffile, unsigned long ReadSize, short ByteFlip, ShadowMap3D *SM3D);
		Raster *FindBestDownsampledVersion(double SampleWidth, double SampleHeight);
		int SampleByteCell3(unsigned char *Output, long SampleX, long SampleY, int &Abort);
		double SampleByteDouble3(double *Output, double SampleXLow, double SampleYLow, int &Abort);
		double SampleBlendDouble3(double *Output, double SampleXLow, double SampleYLow, int &Abort);
		double SampleRangeByteDouble3(double *Output, double SampleXLow, double SampleXHigh, double SampleYLow, double SampleYHigh, int &Abort, int SelfMasking = 0);
		// this one is used for sampling from tilesets, if WCS_IMAGE_MANAGEMENT is enabled
		double SampleManagedByteDouble3(double *Output, unsigned long SampleXCoord, unsigned long SampleYCoord, int &Abort);
		// pass doubles, have them converted to ints
		inline double SampleManagedByteDouble3_DXDY(double *Output, double SampleXLow, double SampleYLow, int &Abort) {return(SampleManagedByteDouble3(Output, ((unsigned int)(SampleXLow * (Cols - .0001))), ((unsigned int)(SampleYLow * (Rows - .0001))), Abort));};
		void CreateTileableImage(int TileWidth, int TileHeight, double OverlapFraction);

		double GetAverageColor(char Component) {return (AverageBand[Component]);};
		double GetAverageCoverage(void) {return (AverageCoverage);};
		void Edit(void);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_RASTER);};
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		TriStimulus *SortByteMaps(unsigned long &Elements);
		void OpenPreview(int LoadAsRendered);
		int AttemptValidatePathAndName(void);
		int GetRAHostAnimated(void);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifyClass(void) {return (RAParent ? RAParent->GetNotifyClass(): (unsigned char)WCS_RAHOST_OBJTYPE_RASTER);};
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)WCS_RAHOST_OBJTYPE_RASTER);};
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual void EditRAHost(void)							{RAParent ? RAParent->EditRAHost(): Edit();};
		virtual char *GetRAHostName(void)						{return (RAParent ? RAParent->GetRAHostName(): Name);};
		virtual char *GetRAHostTypeString(void)					{return ("(Image Object)");};
		virtual char *GetCritterName(RasterAnimHost *TestMe)			{return ("");};
		virtual long GetNextKeyFrame(double &NewDist, short Direction, double CurrentDist)	{NewDist = 0.0; return(0);};
		virtual long DeleteKeyFrame(double FirstDist, double LastDist)	{return (0);};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		void DiscardLoaderGeoRefShell(void);

		// Loaders were moved here from Bitmaps
		// Only loaders that currently or in the future might support smart attributes
		// like tiling and virtual res have the query arg passed to them to lessen the
		// amount of gratuitous rewrite necessary.
		short LoadIFFILBM(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		short LoadIFFZBUF(char *Name, short SupressWng);
		short LoadTGA(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		short LoadBMP(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		short LoadPICT(char *Name, short SupressWng);
		short LoadRPF_RLA(char *Name, short SupressWng);
		short LoadSGIRGB(char *Name, short SupressWng);
		#ifdef WCS_BUILD_JPEG_SUPPORT
		short LoadJPG(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		#endif // WCS_BUILD_JPEG_SUPPORT
		#ifdef WCS_BUILD_PNG_SUPPORT
		short LoadPNG(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		#endif // WCS_BUILD_PNG_SUPPORT
		#ifdef WCS_BUILD_TIFF_SUPPORT
		short LoadTIFF(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		#endif // WCS_BUILD_TIFF_SUPPORT
		#ifdef WCS_BUILD_ECW_SUPPORT
		short LoadECW(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		#endif // WCS_BUILD_ECW_SUPPORT
		#ifdef WCS_BUILD_MRSID_SUPPORT
		short LoadMRSID(char *Name, short SupressWng, ImageCacheControl *ICC = NULL);
		#endif // WCS_BUILD_MRSID_SUPPORT

		// for Tiling identification and such support
		inline unsigned long QueryImageCapability(unsigned long FlagsTest) {return(ImageCapabilityFlags & FlagsTest);};
		inline unsigned char QueryIsSmartTileable(void) {return(QueryImageCapability(WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE) ? 1 : 0);};
		inline unsigned char QueryHasNativeTiles(void) {return(QueryImageCapability(WCS_BITMAPS_IMAGECAPABILITY_HASNATIVETILES) ? 1 : 0);};
		inline unsigned char QuerySupportVirtualRes(void) {return(QueryImageCapability(WCS_BITMAPS_IMAGECAPABILITY_SUPPORTVIRTUALRES) ? 1 : 0);};
		inline unsigned char QuerySupportEfficientThumbnail(void) {return(QueryImageCapability(WCS_BITMAPS_IMAGECAPABILITY_EFFICIENTTHUMBNAIL) ? 1 : 0);};
		inline unsigned char QueryImageManagementEnabled(void) {return(ImageManagerEnabled);};
		// both must be non-zero if supported
		void QueryNativeTileSize(unsigned long &QueryWidth, unsigned long &QueryHeight) {QueryWidth = NativeTileWidth; QueryHeight = NativeTileHeight;};
		unsigned long QueryNativeTileWidth() {return(NativeTileWidth);};
		unsigned long QueryNativeTileHeight() {return(NativeTileHeight);};

		void ClearAllTilesFromCache(void);
		unsigned long QueryMaxCacheSize(void); // value is stored in Mb

	}; // class Raster

/*************************************************************************/

class GeoRaster : public Raster
	{
	public:
		double N, S, E, W, CellSizeNS, CellSizeEW, CellArea;
		long LUTItems, LUTSize, JLUTItems, JLUTSize;
		char GradFill, Overflow, Initialized;
		unsigned char HighLUT, LUTSet;
		GeneralEffect **LUT;
		Joe **JLUT;

		UBYTE *floodmap;
		long mapsize;
		long xsize,ysize;
		struct RasterPoint *vert;

		GeoRaster();
		GeoRaster(double sN, double sS, double sE, double sW, double sCellSize, long MaxMem, long &UsedMem,
			short MapType = WCS_RASTER_BANDSET_BYTE, long sLUTItems = 0, long sJLUTItems = 0, long AltMapMultiplier = 0);
		GeoRaster(double sN, double sS, double sE, double sW, double sCellSize, long &UsedMem);
		GeoRaster(GeoRaster *CopyRast, long MaxMem, long &UsedMem);
		~GeoRaster();
		void Copy(GeoRaster *CopyTo, GeoRaster *CopyFrom);

		// placing values
		unsigned char *AllocFloodmap(Joe *FillMe, unsigned char ByteFill, long &minx, long &miny);
		unsigned char *AllocFloodmap(VectorPolygon *FillMe, unsigned char ByteFill, long &minx, long &miny);
		void FreeFloodmap(void);
		struct RasterPoint *BuildRasterPointList(CoordSys *MyCoords, VectorPoint *StartPt, long &NumPts);
		struct RasterPoint *BuildRasterPointList(VectorNode *StartPt, unsigned long NumPts);
		void FreeRasterPointList(RasterPoint *PtList, long NumPts);
		void ClearByte(short Band, long *ClearBounds, UBYTE ClearVal = 0);
		GeoRaster *PolyRasterFillByte(long MaxMem, long &UsedMem, Joe *FillMe, short Band, UBYTE FillVal, short OverlapOK, short GradntFill = 0);
		int PolyRasterFillFloat(Joe *FillMe, short Band, float FloatFill);
		void PolyRasterTransferFloat(short Band, long MinX, long MinY, float FloatFill);
		GeoRaster *PolyRasterCopyByte(GeoRaster *CopyFrom, long MaxMem, long &UsedMem, Joe *FillMe, GeneralEffect *Effect, short Band, long *CopyBounds, short OverlapOK);
		GeoRaster *PolyRasterCopyFloodByte(long MaxMem, long &UsedMem, 
			short Band, UBYTE FillVal, short OverlapOK, short GradntFill, long OffsetX, long OffsetY);
		bool PolyRasterCopyFloodPolyList(VectorPolygon *CopyMe, long OffsetX, long OffsetY);
		GeoRaster *PolyRasterEdgeByte(long MaxMem, long &UsedMem, Joe *OutlineMe, short Band, UBYTE FillVal, short OverlapOK, short ConnectBack);
		bool PolyRasterEdgePolyList(VectorPolygon *OutlineMe);
		void PolyRasterEdgeByteFloodmap(Joe *OutlineMe, UBYTE FillVal, 
			long OffsetX, long OffsetY, long MapWidth, long MapHeight);
		void PolyRasterEdgeByteFloodmap(VectorPart *OutlineMe, UBYTE FillVal, 
			long OffsetX, long OffsetY, long MapWidth, long MapHeight);
		GeoRaster *PolyRasterEdgeBytePoint(long MaxMem, long &UsedMem, Joe *OutlineMe, short Band, UBYTE FillVal, short OverlapOK, short ConnectBack);
		GeoRaster *PolyRasterSegByte(long MaxMem, long &UsedMem, double fx, double fy, double tx, double ty, short Band, UBYTE FillVal, short OverlapOK);
		bool PolyRasterSegPolyList(VectorPolygon *PlotMe, double fx, double fy, double tx, double ty);
		void PolyRasterSegByteFloodmap(double fx, double fy, double tx, double ty, UBYTE FillVal, 
			long OffsetX, long OffsetY, long MapWidth, long MapHeight);
		GeoRaster *PolyRasterFillCell(long LocalRow, long LocalCol, short Band, UBYTE FillVal, short OverlapOK, long MaxMem, long &UsedMem);
		void PolyRasterGradientByte(short Band, UBYTE FillVal);
		void PolyRasterFillOutByte(short Band, UBYTE FillVal, long Iterations, long *CopyBounds);
		UBYTE MinInline(short Band, long zip, long x, long y, short MaxSearch);
		UBYTE MinInlineFillOut(short Band, long zip, long x, long y, short MaxSearch);
		UBYTE MinDiag(short Band, long zip, long x, long y, short MaxSearch);
		UBYTE MinDiagFillOut(short Band, long zip, long x, long y, short MaxSearch);

		// putting values into PolygonList block
		bool PlotPolygon(VectorPolygon *PlotMe);
		bool PlotPolygonPoint(VectorPolygon *PlotMe, long x, long y, bool OnEdge);
		bool PlotPolygonPoint(VectorPolygon *PlotMe, VectorPolygonList **PlotCell, bool OnEdge);
		
		// retrieving values
		long GetCell(double Lat, double Lon);
		long GetValidByteCell(short Band, double Lat, double Lon);
		long GetValidFloatCell(short Band, double Lat, double Lon);
		UBYTE GetByte(short Band, long TheCell) {return(ByteMap[Band][TheCell]);};
		float GetFloat(short Band, long TheCell) {return(FloatMap[Band][TheCell]);};
		CornerMask GetByteCellMask(short Band, double Lat, double Lon);
		CornerMask GetFloatCellMask(short Band, double Lat, double Lon);
		double GetByteMaskInterp(short Band, double Lat, double Lon);
		double GetFloatMaskInterp(short Band, double Lat, double Lon);

		// flood fill routines
		UBYTE PixelRead(long, long);
		void SeedFill(UBYTE);
		// obsolete 3/5/01
		//void DrawLine(long, long, long, long, UBYTE);
		//void DrawPoly(long, long, long, UBYTE);
	}; // GeoRaster

/*************************************************************************/
/*************************************************************************/

// IO defines
#define WCS_THUMBNAIL_BANDRED		0x00210000
#define WCS_THUMBNAIL_BANDGREEN		0x00220000
#define WCS_THUMBNAIL_BANDBLUE		0x00230000
#define WCS_THUMBNAIL_TNAILPADX		0x00240000
#define WCS_THUMBNAIL_TNAILPADY		0x00250000

#define WCS_IMAGES_RASTER		0x01000000
#define WCS_IMAGES_GEORASTER	0x02000000
#define WCS_IMAGES_IMAGE		0x03000000
#define WCS_IMAGES_SHADOWRASTER	0x04000000
#define WCS_IMAGES_MAXTILEMEM	0x05000000

#define WCS_RASTER_ID			0x00100000
#define WCS_RASTER_DATA			0x00200000
#define WCS_RASTER_NAME			0x00300000
#define WCS_RASTER_TNAIL		0x00400000
#define WCS_RASTER_ATTRIB		0x00500000
#define WCS_RASTER_BANDDATA		0x00600000
#define WCS_RASTER_SMALLER		0x00700000
#define WCS_RASTER_AVGCOVG		0x00800000
#define WCS_RASTER_AVGRED		0x00900000
#define WCS_RASTER_AVGGREEN		0x00a00000
#define WCS_RASTER_AVGBLUE		0x00b00000
#define WCS_RASTER_THUMBNAIL	0x00c00000
#define WCS_IMAGE_DATA			0x01100000
#define WCS_IMAGE_RASTER		0x01200000
#define WCS_GEORASTER_DATA		0x02100000
#define WCS_GEORASTER_RASTER	0x02200000

#define WCS_RASTER_DATA_ROWS				0x00010000
#define WCS_RASTER_DATA_COLS				0x00020000
#define WCS_RASTER_DATA_BYTEBANDS			0x00030000
#define WCS_RASTER_DATA_FLOATBANDS			0x00040000
#define WCS_RASTER_DATA_FORMATLASTLOADED	0x00050000
#define WCS_RASTER_DATA_SAVEFORMAT			0x00060000
#define WCS_RASTER_DATA_LONGEVITY			0x00070000
#define WCS_RASTER_DATA_TNAILPADX			0x00080000
#define WCS_RASTER_DATA_TNAILPADY			0x00090000
#define WCS_RASTER_DATA_USERNAME			0x000a0000
#define WCS_RASTER_DATA_ENABLED				0x000b0000
#define WCS_RASTER_DATA_SEQUENCEENABLED		0x000c0000
#define WCS_RASTER_DATA_DISSOLVEENABLED		0x000d0000
#define WCS_RASTER_DATA_TILINGENABLED		0x000e0000	// unused
#define WCS_RASTER_DATA_COLORCONTROLENABLED	0x000f0000
#define WCS_RASTER_DATA_LOADFAST			0x00110000
#define WCS_RASTER_DATA_ALPHAAVAILABLE		0x00120000
#define WCS_RASTER_DATA_ALPHAENABLED		0x00130000
#define WCS_RASTER_DATA_IMAGEMANAGERENABLED	0x00140000
#define WCS_RASTER_DATA_IMAGECAPABILITY		0x00150000
#define WCS_RASTER_DATA_NATIVETILEWIDTH		0x00160000
#define WCS_RASTER_DATA_NATIVETILEHEIGHT	0x00170000

#define WCS_SM3D_DATA_VPX					0x00410000
#define WCS_SM3D_DATA_VPY					0x00420000
#define WCS_SM3D_DATA_VPZ					0x00430000
#define WCS_SM3D_DATA_CENTERX				0x00440000
#define WCS_SM3D_DATA_CENTERY				0x00450000
#define WCS_SM3D_DATA_HORSCALE				0x00460000
#define WCS_SM3D_DATA_SOLARROTMATX			0x00470000
#define WCS_SM3D_DATA_ZOFFSET				0x00480000
#define WCS_SM3D_DATA_TYPE					0x00490000
#define WCS_SM3D_DATA_OBJX					0x004a0000
#define WCS_SM3D_DATA_OBJY					0x004b0000
#define WCS_SM3D_DATA_OBJZ					0x004c0000
#define WCS_SM3D_DATA_DISTOFFSET			0x004d0000

#define WCS_IMAGE_DATA_HEIGHT				0x00010000
#define WCS_IMAGE_DATA_DENSITY				0x00020000
#define WCS_IMAGE_DATA_PALCOL				0x00030000
#define WCS_IMAGE_DATA_MAXTRANSPAR_1		0x00040000
#define WCS_IMAGE_DATA_MINTRANSPAR_1		0x00050000
#define WCS_IMAGE_DATA_MAXTRANSPAR_2		0x00060000
#define WCS_IMAGE_DATA_MINTRANSPAR_2		0x00070000
#define WCS_IMAGE_DATA_MAXTRANSPAR_3		0x00080000
#define WCS_IMAGE_DATA_MINTRANSPAR_3		0x00090000
#define WCS_IMAGE_DATA_POSSHADE				0x000a0000
#define WCS_IMAGE_DATA_SHADE3D				0x000b0000
#define WCS_IMAGE_DATA_USEIMAGECOLOR		0x000c0000
#define WCS_IMAGE_DATA_TRANSPARENCYENABLED	0x000d0000

#define WCS_GEORASTER_DATA_NORTH			0x00310000
#define WCS_GEORASTER_DATA_SOUTH			0x00320000
#define WCS_GEORASTER_DATA_EAST				0x00330000
#define WCS_GEORASTER_DATA_WEST				0x00340000
#define WCS_GEORASTER_DATA_CELLSIZENS		0x00350000
#define WCS_GEORASTER_DATA_CELLSIZEEW		0x00360000
#define WCS_GEORASTER_DATA_CELLAREA			0x00370000
#define WCS_GEORASTER_DATA_GRADFILL			0x00380000

#define WCS_RASTER_ATTRIB_SHELL_GEOREF			0x00010000
#define WCS_RASTER_ATTRIB_SHELL_IMAGE			0x00020000
#define WCS_RASTER_ATTRIB_SHELL_FOLIAGE			0x00030000
#define WCS_RASTER_ATTRIB_SHELL_SEQUENCE		0x00040000
#define WCS_RASTER_ATTRIB_SHELL_DISSOLVE		0x00050000
#define WCS_RASTER_ATTRIB_SHELL_COLORCONTROL	0x00070000
#define WCS_RASTER_ATTRIB_SHELL_IMAGEMANAGER	0x00080000

#define WCS_RASTER_TNAIL_BANDNUM			0x00010000
#define WCS_RASTER_TNAIL_BYTEDATA			0x00020000

#define WCS_RASTER_BANDDATA_BANDNUM			0x00010000
#define WCS_RASTER_BANDDATA_BYTEDATA		0x00020000
#define WCS_RASTER_BANDDATA_FLOATDATA		0x00030000
#define WCS_RASTER_BANDDATA_ROWFLOATDATA	0x00040000
#define WCS_RASTER_BANDDATA_ALTBYTEDATA		0x00050000
#define WCS_RASTER_BANDDATA_ALTFLOATDATA	0x00060000

#define WCS_RASTER_SHELL_ID					0x00010000

#define WCS_RASTER_GEOREFSHELL_GEOREG		0x00020000
#define WCS_RASTER_GEOREFSHELL_BOUNDSTYPE	0x00030000

#define WCS_RASTER_COLORCONTROLSHELL_USEASCOLOR			0x00020000
#define WCS_RASTER_COLORCONTROLSHELL_GRAYAUTORANGE		0x00030000
#define WCS_RASTER_COLORCONTROLSHELL_DISPLAYRGBSET		0x00040000
#define WCS_RASTER_COLORCONTROLSHELL_SHOWTRANSPAR		0x00050000
#define WCS_RASTER_COLORCONTROLSHELL_USEBANDASSIGN		0x00060000
#define WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_0		0x00070000
#define WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_1		0x00080000
#define WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_2		0x00090000
#define WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_OUTBAND	0x000a0000
#define WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_INBAND	0x000b0000
#define WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR			0x000c0000
#define WCS_RASTER_COLORCONTROLSHELL_RGB_APP			0x000d0000
#define WCS_RASTER_COLORCONTROLSHELL_RGB_BAND			0x000e0000
#define WCS_RASTER_COLORCONTROLSHELL_RGB				0x000f0000

#define WCS_RASTER_IMAGEESHELL_COLOR				0x00020000
/*
#define WCS_RASTER_IMAGEESHELL_MAXTRANSPAR_1		0x00030000
#define WCS_RASTER_IMAGEESHELL_MINTRANSPAR_1		0x00040000
#define WCS_RASTER_IMAGEESHELL_MAXTRANSPAR_2		0x00050000
#define WCS_RASTER_IMAGEESHELL_MINTRANSPAR_2		0x00060000
#define WCS_RASTER_IMAGEESHELL_MAXTRANSPAR_3		0x00070000
#define WCS_RASTER_IMAGEESHELL_MINTRANSPAR_3		0x00080000
#define WCS_RASTER_IMAGEESHELL_USEIMAGECOLOR		0x00090000
#define WCS_RASTER_IMAGEESHELL_TRANSPARENCYENABLED	0x000a0000
*/
#define WCS_RASTER_FOLIAGESHELL_HEIGHTPCT			0x00020000
#define WCS_RASTER_FOLIAGESHELL_DENSITYPCT			0x00030000
#define WCS_RASTER_FOLIAGESHELL_POSSHADE			0x00040000
#define WCS_RASTER_FOLIAGESHELL_SHADE3D				0x00050000
#define WCS_RASTER_FOLIAGESHELL_COLORS				0x00060000
#define WCS_RASTER_FOLIAGESHELL_IMAGEDATA			0x00110000

#define WCS_RASTER_SEQUENCESHELL_NAME				0x00020000
#define WCS_RASTER_SEQUENCESHELL_STARTFRAME			0x00030000
#define WCS_RASTER_SEQUENCESHELL_ENDFRAME			0x00040000
#define WCS_RASTER_SEQUENCESHELL_STARTIMAGE			0x00050000
#define WCS_RASTER_SEQUENCESHELL_ENDIMAGE			0x00060000
#define WCS_RASTER_SEQUENCESHELL_NUMLOOPS			0x00070000
#define WCS_RASTER_SEQUENCESHELL_SPEED				0x00080000
#define WCS_RASTER_SEQUENCESHELL_STARTBEHAVIOR		0x00090000
#define WCS_RASTER_SEQUENCESHELL_ENDBEHAVIOR		0x000a0000
#define WCS_RASTER_SEQUENCESHELL_NEXTSEQUENCE		0x000b0000
#define WCS_RASTER_SEQUENCESHELL_DURATION			0x000c0000
#define WCS_RASTER_SEQUENCESHELL_STARTSPACE			0x000d0000
#define WCS_RASTER_SEQUENCESHELL_ENDSPACE			0x000e0000

#define WCS_RASTER_DISSOLVESHELL_RASTERID			0x00020000
#define WCS_RASTER_DISSOLVESHELL_STARTFRAME			0x00030000
#define WCS_RASTER_DISSOLVESHELL_ENDFRAME			0x00040000
#define WCS_RASTER_DISSOLVESHELL_DURATION			0x00050000
#define WCS_RASTER_DISSOLVESHELL_EASEIN				0x00060000
#define WCS_RASTER_DISSOLVESHELL_EASEOUT			0x00070000
#define WCS_RASTER_DISSOLVESHELL_NEXTDISSOLVE		0x00080000

#define WCS_RASTER_IMAGEMGRSHELL_TILINGTYPE			0x00020000
#define WCS_RASTER_IMAGEMGRSHELL_VIRTRESENABLED		0x00030000
#define WCS_RASTER_IMAGEMGRSHELL_VIRTRESTYPE		0x00040000
#define WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEX		0x00050000
#define WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEY		0x00060000
#define WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEX	0x00070000
#define WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEY	0x00080000
#define WCS_RASTER_IMAGEMGRSHELL_VIRTRESX			0x00090000
#define WCS_RASTER_IMAGEMGRSHELL_VIRTRESY			0x000a0000

#define WCS_NOTIFYCLASS_IMAGES				131

// Notify subclass
enum
	{
	WCS_SUBCLASS_RASTER = 190,
	WCS_IMAGESSUBCLASS_GENERIC
	}; // subclass

//Notify item
enum
	{
	WCS_IMAGESGENERIC_NAME	= 1,
	WCS_IMAGESGENERIC_ENABLED,
	WCS_IMAGESGENERIC_IMAGEADDED,
	WCS_IMAGESGENERIC_ATTRIBUTEADDED,
	WCS_IMAGESGENERIC_ACTIVECHANGED
	}; // item

inline unsigned long RasterShell::GetRasterID(void)
{

return (Rast ? Rast->GetID(): 0);

} // RasterShell::GetRasterID

// methods used by qsort for sorting TriStimulus arrays by component value
int RasterSortByRed(const void *elem1, const void *elem2);
int RasterSortByGreen(const void *elem1, const void *elem2);
int RasterSortByBlue(const void *elem1, const void *elem2);

#endif // WCS_RASTER_H
