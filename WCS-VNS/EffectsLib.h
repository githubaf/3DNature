// EffectsLib.h
// Header file for EffectsLib.cpp
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_EFFECTSLIB_H
#define WCS_EFFECTSLIB_H

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

#undef RGB // stupid Microsoft makes macros with hazardously-common names

class Joe;
class JoeList;
class RenderJoeList;
class EffectsLib;
union KeyFrame;
class Raster;
class GeoRefShell;
class FoliageShell;
class RasterShell;
class ImageLib;
class Renderer;
class InterCommon;
class GeneralEffect;

//class ComboEffect;
class LakeEffect;
class FogEffect;
class CmapEffect;
class WaveEffect;
class CloudEffect;
class EnvironmentEffect;
class EcosystemEffect;
class TintEffect;
class LandWaveEffect;
class RasterTerraffectorEffect;
class CloudShadowEffect;
class ShadowEffect;

class FoliageEffect;
class Object3DEffect;
class ObjectVecEffect;

class MorphEffect;
class PathFollowEffect;
class PathConformEffect;
class TerraffectorEffect;
class StreamEffect;
class MaterialEffect;
class BackgroundEffect;
class CelestialEffect;
class StarFieldEffect;
class PlanetOpt;
class TerrainParamEffect;
class GroundEffect;
class SnowEffect;
class Sky;
class Atmosphere;
class Light;
class Camera;
class RenderJob;
class RenderOpt;
class RenderData;
class TerraGridder;
class TerraGenerator;
class SearchQuery;
class ThematicMap;
class CoordSys;
class Fence;
class PostProcess;
class RenderScenario;
class DEMMerger;
class SceneExporter;
class Label;
// <<<>>> ADD_NEW_EFFECTS EffectsLib needs to know that the effect class exists

class ShadowMap3D;
class GeoRaster;
class Gradient;
class Vertex3D;
class Polygon3D;
class RootTexture;
class TextureData;
class PixelData;
class VertexData;
class VertexDEM;
class VertexCloud;
class PolygonData;
class Polygon3DRenderData;
class PRNGX;
struct coords;
struct ChunkIODetail;
class FoliageShellLink;
class Foliage;
class FoliageGroup;
class Ecotype;
struct Ecosystem;
struct Wave;
class Project;
class BufferNode;
class ImageOutputEvent;
class Database;
class DEM;
class DBFilterEvent;
class ControlPointDatum;
class LayerEntry;
class ProjectionMethod;
class GeoDatum;
class GeoEllipsoid;
class CSLoadInfo;
class PostProcessEvent;
class rPixelBlockHeader;
class BusyWin;
class RenderInterface;
class LWTagTable;
class VolumetricSubstance;
class RealtimeFoliageIndex;
class RealtimeFoliageData;
class SXExtension;
class SXQueryAction;
class DEFG;
class EffectList;
class ImageList;
class NumberList;
class JoeList;
class RenderJoeList;
class EffectJoeList;
class EffectEval;
class VectorNodeRenderData;
class VectorNode;
class MaterialList;
class VectorPolygonListDouble;
class MaterialTextureInfo;

#include "Notify.h"
#include "GraphData.h"
#include "MathSupport.h"
#include "PathAndFile.h"
#include "Color.h"
#include "RasterAnimHost.h"
#include "3dsftk.h"
#include "Ecotype.h"
#include "Texture.h"
#include "Random.h"
#include "GeoRegister.h"
#include "NNGrid.h"
#include "FeatureConfig.h"
#include "Tables.h"
#include "ThematicOwner.h"
#include "IncludeExcludeList.h"
#include "RasterBounds.h"
#include "GeoLib/GeoLib.h"
#include "SXQueryItem.h"
#include "DBFilterEvent.h"
#include "Realtime.h"
//#include "Render.h"

#define WCS_EFFECTS_LOAD_CLEAR			(1U << 31)
#define WCS_EFFECTS_LOAD_CLOSEWINDOWS	(1 << 0)

#define WCS_EFFECTS_VERSION		1
#define WCS_EFFECTS_REVISION	1

#define WCS_EFFECT_ABSOLUTE			1
#define WCS_EFFECT_RELATIVETOJOE	2	
#define WCS_EFFECT_RELATIVETOGROUND	0

#define WCS_EFFECT_MAXNAMELENGTH	60

#define WCS_MAX_FRACTAL_DEPTH	9

enum
	{
	WCS_CELESTIAL_PRESETS_SUN,
	WCS_CELESTIAL_PRESETS_MERCURY,
	WCS_CELESTIAL_PRESETS_VENUS,
	WCS_CELESTIAL_PRESETS_EARTH,
	WCS_CELESTIAL_PRESETS_MARS,
	WCS_CELESTIAL_PRESETS_JUPITER,
	WCS_CELESTIAL_PRESETS_SATURN,
	WCS_CELESTIAL_PRESETS_URANUS,
	WCS_CELESTIAL_PRESETS_NEPTUNE,
	WCS_CELESTIAL_PRESETS_PLUTO,
	WCS_CELESTIAL_PRESETS_MOON,
	WCS_CELESTIAL_PRESETS_COMET,
	WCS_CELESTIAL_PRESETS_BIGMETEOR,
	WCS_CELESTIAL_PRESETS_SATELLITE,
	WCS_CELESTIAL_PRESETS_LOWORBIT,
	WCS_PRESETS_NUMCELESTIALS
	}; // celestial bodies

#ifdef _MSC_VER // Are we using MSVC++?
// Disable annoying "warning C4355: 'this' : used in base member initializer list"
#pragma warning(disable : 4355)
#endif // _MSC_VER

// Notify codes

// Notify class
#define WCS_NOTIFYCLASS_EFFECTS				130

// Notify subclass
// do not change this order - it corresponds with the attribute
// values in the database for whatever that might be worth
enum
	{
	WCS_EFFECTSSUBCLASS_GENERIC = 0,
	WCS_EFFECTSSUBCLASS_COMBO,				// retired - used in WCS 6 and VNS 2 files, obsolete in VNS 3 and WCS 7, remove 3/1/07
	WCS_EFFECTSSUBCLASS_LAKE,
	WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	WCS_EFFECTSSUBCLASS_FOG,				// these probably won't be used
	WCS_EFFECTSSUBCLASS_WAVE,
	WCS_EFFECTSSUBCLASS_CLOUD,
	WCS_EFFECTSSUBCLASS_CLOUDSHADOW,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	WCS_EFFECTSSUBCLASS_TINT,				// these probably won't be used
	WCS_EFFECTSSUBCLASS_CMAP,
	WCS_EFFECTSSUBCLASS_ILLUMINATION,		// retired - used in WCS 4 files, obsolete in WCS 5 and VNS 1, removed 2/22/06
	WCS_EFFECTSSUBCLASS_LANDWAVE,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_RASTERTA,

	WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	WCS_EFFECTSSUBCLASS_PROFILE,			// retired - used in WCS 4 files, obsolete in WCS 5 and VNS 1, removed 2/22/06
	WCS_EFFECTSSUBCLASS_GRADIENTPROFILE,	// retired - used in WCS 4 files, obsolete in WCS 5 and VNS 1, removed 2/22/06
	WCS_EFFECTSSUBCLASS_PATHFOLLOW,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_PATHCONFORM,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_MORPH,				// these probably won't be used

	WCS_EFFECTSSUBCLASS_ECOTYPE,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_FOLIAGEGRP,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_FOLIAGE,
	WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_OBJECTVEC,			// these probably won't be used

	WCS_EFFECTSSUBCLASS_SHADOW,
	WCS_EFFECTSSUBCLASS_STREAM,
	WCS_EFFECTSSUBCLASS_MATERIAL,
	WCS_EFFECTSSUBCLASS_BACKGROUND,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_CELESTIAL,
	WCS_EFFECTSSUBCLASS_STARFIELD,
	WCS_EFFECTSSUBCLASS_PLANETOPT,
	WCS_EFFECTSSUBCLASS_TERRAINPARAM,
	WCS_EFFECTSSUBCLASS_GROUND,
	WCS_EFFECTSSUBCLASS_SNOW,
	WCS_EFFECTSSUBCLASS_SKY,
	WCS_EFFECTSSUBCLASS_ATMOSPHERE,
	WCS_EFFECTSSUBCLASS_LIGHT,
	WCS_EFFECTSSUBCLASS_CAMERA,
	WCS_EFFECTSSUBCLASS_RENDERJOB,
	WCS_EFFECTSSUBCLASS_RENDEROPT,
	WCS_EFFECTSSUBCLASS_GRIDDER,
	WCS_EFFECTSSUBCLASS_GENERATOR,
	WCS_EFFECTSSUBCLASS_SEARCHQUERY,
	WCS_EFFECTSSUBCLASS_THEMATICMAP,
	WCS_EFFECTSSUBCLASS_COORDSYS,
	WCS_EFFECTSSUBCLASS_FENCE,
	WCS_EFFECTSSUBCLASS_POSTPROC,
	WCS_EFFECTSSUBCLASS_SCENARIO,
	WCS_EFFECTSSUBCLASS_DEMMERGER,
	WCS_EFFECTSSUBCLASS_EXPORTER,
	WCS_EFFECTSSUBCLASS_LABEL,
// <<<>>> ADD_NEW_EFFECTS add new enum right above this line. ALWAYS there and nowhere else unless you 
// really, really know what you are doing and THAT means your name is Gary R. Huber! :-)
// If you get at least this much right chances are you won't destroy the world by adding new effects.

	WCS_MAXIMPLEMENTED_EFFECTS
	}; // subclass - these must match database joe attributes, don't rearrange


enum
	{
	WCS_SUBCLASS_MATERIALSTRATA = 141,
	WCS_SUBCLASS_WAVESOURCE,
	WCS_SUBCLASS_ATMOCOMPONENT,
	WCS_SUBCLASS_TERRAINTYPE,
	WCS_SUBCLASS_PROJECTIONMETHOD,
	WCS_SUBCLASS_GEODATUM,
	WCS_SUBCLASS_GEOELLIPSOID
	}; // subclasses contained in effects that are not effects per se
// from Ecotype.h
//enum
//	{
//	WCS_SUBCLASS_ECOTYPE = 120,
//	WCS_SUBCLASS_FOLIAGEGRP,
//	WCS_SUBCLASS_FOLIAGE
//	}; // subclasses contained in effects that are not effects per se
// from GraphData.h
//enum
//	{
//	WCS_SUBCLASS_ANIMGRADIENT = 130,
//	WCS_SUBCLASS_ANIMCOLORGRADIENT,
//	WCS_SUBCLASS_ANIMMATERIALGRADIENT,
//	WCS_SUBCLASS_COLORTEXTURE
//	}; // subclasses contained in effects that are not effects per se
// from Texture.h
//enum
//	{
//	WCS_SUBCLASS_ROOTTEXTURE = 140,
//	WCS_SUBCLASS_TEXTURE
//	}; // subclasses contained in effects that are not effects per se

//Notify item - these should be larger than the maximum number of animated parameters in any effect class
enum
	{
	WCS_EFFECTSGENERIC_NAME	= 50,
	WCS_EFFECTSGENERIC_ENABLED,
	WCS_EFFECTSGENERIC_JOESADDED,
	WCS_EFFECTSGENERIC_EFFECTADDED,
	WCS_EFFECTSGENERIC_PROFILEADDED,
	WCS_EFFECTSGENERIC_GROUPENABLED
	}; // item

// defines
enum
	{
	WCS_COMPARE_SORTORDER = 0,
	WCS_COMPARE_RANDOM,
	WCS_COMPARE_AVERAGE,
	WCS_COMPARE_MAXIMUM,
	WCS_COMPARE_MINIMUM,
	WCS_COMPARE_SUM
	}; // CompareMethods - these get saved in file, don't rearrange

enum
	{
	WCS_COMPARE_TA_USEASIS = 0,
	WCS_COMPARE_TA_AVERAGE,
	WCS_COMPARE_TA_MAXIMUM,
	WCS_COMPARE_TA_MINIMUM,
	WCS_COMPARE_TA_SUM
	}; // CompareMethods - these get saved in file, don't rearrange

enum
	{
	WCS_EFFECTGRAPH_UNITS_MILES,
	WCS_EFFECTGRAPH_UNITS_KILOMETERS,
	WCS_EFFECTGRAPH_UNITS_METERS,
	WCS_EFFECTGRAPH_UNITS_FEET,
	WCS_EFFECTGRAPH_UNITS_INCHES,
	WCS_EFFECTGRAPH_UNITS_CENTIMETERS,
	WCS_EFFECTGRAPH_UNITS_DECIMETERS,
	WCS_EFFECTGRAPH_UNITS_MILLIMETERS,
	WCS_EFFECTGRAPH_UNITS_YARDS
	}; // graph units

enum
	{
	WCS_EFFECT_MATERIAL_SHADING_INVISIBLE,
	WCS_EFFECT_MATERIAL_SHADING_FLAT,
	WCS_EFFECT_MATERIAL_SHADING_PHONG
	}; // material shading

enum
	{
	WCS_EFFECT_OBJECT3D_FORMAT_LIGHTWAVE = 1,
	WCS_EFFECT_OBJECT3D_FORMAT_DXF,
	WCS_EFFECT_OBJECT3D_FORMAT_3DS,
	WCS_EFFECT_OBJECT3D_FORMAT_W3O,
	WCS_EFFECT_OBJECT3D_FORMAT_W3D,
	WCS_EFFECT_OBJECT3D_FORMAT_OBJ,
	WCS_EFFECT_OBJECT3D_FORMAT_LAST // dummy entry, always last
	}; // object3d format

enum
	{
	WCS_EFFECTS_OBJECT3D_DRAW_NONE,
	WCS_EFFECTS_OBJECT3D_DRAW_CUBE,
	WCS_EFFECTS_OBJECT3D_DRAW_DETAIL
	}; // object3d camview draw mode

enum
	{
	WCS_CMAP_ORIENTATION_TOPWEST,
	WCS_CMAP_ORIENTATION_TOPNORTH
	}; // cmap orientation

#define WCS_SHADOWTYPE_TERRAIN	(1 << 0)
#define WCS_SHADOWTYPE_FOLIAGE	(1 << 1)
#define WCS_SHADOWTYPE_3DOBJECT	(1 << 2)
#define WCS_SHADOWTYPE_CLOUDSM	(1 << 3)
#define WCS_SHADOWTYPE_VOLUME	(1 << 4)
#define WCS_SHADOWTYPE_MISC		(1 << 5)


#define WCS_OBJECT3D_VERSION	1
#define WCS_OBJECT3D_REVISION	0
#define WCS_MATERIAL_VERSION	1
#define WCS_MATERIAL_REVISION	0

struct ShadowParams
	{
	double N, S, E, W, SafeAdd, SafeSub, MinFolHt;
	short MapWidth;
	Joe *ShadeMe;
	}; // ShadowParams

class MaterialAttributes; // moved down below to be able to use certain enums appearing later

class ObjectMaterialEntry
	{
	public:

		MaterialEffect *Mat;
		char Name[WCS_EFFECT_MAXNAMELENGTH];

		ObjectMaterialEntry();
		char *SetName(char *NewName);
		int CompareName(char *Compare) {return (strcmp(Name, Compare));};

	}; // ObjectMaterialEntry

enum
	{
	WCS_OBJPERVERTMAP_MAPTYPE_UV,
	WCS_OBJPERVERTMAP_MAPTYPE_WEIGHT
	}; // 

class ObjectPerVertexMap
	{
	public:

		char Name[WCS_EFFECT_MAXNAMELENGTH], UVMapType;
		long NumNodes;
		char *CoordsValid;
		float *CoordsArray[3];

		ObjectPerVertexMap();
		ObjectPerVertexMap(long NewNumNodes);
		~ObjectPerVertexMap();
		char *SetName(char *NewName);
		int CompareName(char *Compare) {return (strcmp(Name, Compare));};
		int AllocMap(long NewNumNodes);
		int MapsValid(void)	{return (CoordsValid && CoordsArray[0] && CoordsArray[1] && CoordsArray[2]);};
		void FreeMap(void);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, long NumVerts);
		unsigned long Save(FILE *ffile, int SaveArrays);

	}; // ObjectPerVertexMap

class VertexReferenceData
	{
	public:
		unsigned char MapType, MapNumber;
		long ObjVertNumber, VertRefNumber;
		VertexReferenceData *NextVertex, *NextMap;

		VertexReferenceData();
		VertexReferenceData(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber);
		~VertexReferenceData();
		VertexReferenceData *AddVertRef(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber);
		long GetMapIndex(unsigned char TestMapType, unsigned char TestMapNumber, long DefaultVertexNumber);
	}; // class VertexReferenceData

enum
	{
	WCS_VERTREFDATA_MAPTYPE_UVW,
	WCS_VERTREFDATA_MAPTYPE_COLORPERVERTEX
	}; // vertex reference map types

class Polygon3D
	{
	public:
		Point3d Normal;
		VertexReferenceData *RefData;
		long *VertRef;
		long Material;
		short NumVerts;
		char Normalized;

		Polygon3D();
		~Polygon3D();
		void Copy(Polygon3D *CopyTo, Polygon3D *CopyFrom);
		void Normalize(Vertex3D *Vert);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		VertexReferenceData *AddVertRef(long NewObjVertNumber, long NewVertRefNumber, unsigned char NewMapType, unsigned char NewMapNumber);
		VertexReferenceData *FindVertRefDataHead(long TestObjVertNumber);
	}; // Polygon3D

class Polygon3DRenderData
	{
	public:
		double SunAngle[3], Illumination, ShadowIntensity,
			DefLuminosity, DefTransparency, DefSpecularity, DefSpecularExp, DefTransluminance, DefTranslumExp, DefDisplacement,
			Luminosity, Transparency, Specularity, SpecularExp, Transluminance, TranslumExp, Displacement;
		MaterialEffect *Mat;
		Object3DEffect *Obj;
		TextureData *TexData;
		long ShadowInstance;
		short ShadingModel;
		char AffectedByHaze, AffectedByFog, TexturesExist;
		Point3d *VtxNormalPtr[3], *VertexPtr[3];
		Point3d ViewVec;
		Point3d NegViewVec;
		float *LocalVertexPtr[3];
		ShadowMap3D *ShadowMap;
		struct coords *SunVec;
		WCSRGBColor DefCC, CC;

		Polygon3DRenderData();
		~Polygon3DRenderData();

	}; // class Polygon3DRenderData

// in Camera.cpp
class SmoothPath
	{
	private:
		double *Smooth[3], FrameTime;
		long NumFrames;

	public:
		char ConstructError;

		SmoothPath();
		SmoothPath(AnimDoubleTime *AnimX, AnimDoubleTime *AnimY, AnimDoubleTime *AnimZ, 
			double EaseInTime, double EaseOutTime, double FrameRate, int GeoCoordVariable, double LatScaleMeters);
		~SmoothPath();
		int Copy(SmoothPath *CopyFrom);
		int GetSmoothPoint(double &GetX, double &GetY, double &GetZ, double Time);

	}; // class SmoothPath

enum
	{
	WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE,
	WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP,
	WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_WESTDIP,
	WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY,
	WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR
	}; // animated MaterialStrata params

enum
	{
	WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION,
	WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY,
	WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES
	}; // MaterialStrata textures

class MaterialStrata : public RasterAnimHost, public RootTextureParent
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR];
		AnimColorTime StrataColor[4];
		char Enabled, LinesEnabled, ColorStrata, BumpLines, PixelTexturesExist;
		RootTexture *TexRoot[WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES];
		static unsigned char *StrataNoiseMap;
		PRNGX Rand;

		MaterialStrata(RasterAnimHost *RAHost);
		MaterialStrata(void);
		~MaterialStrata();
		void SetDefaults(void);
		void Copy(MaterialStrata *CopyTo, MaterialStrata *CopyFrom);

		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		long GetNumAnimParams(void) {return (WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_MATERIALSTRATA_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		short AnimateShadows(void)	{return (0);};
		int SetToTime(double Time);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_MATERIALSTRATA);};
		int GetRAHostAnimated(void);
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend)	{return (1);};
		double ComputeTexture(double ElevRange, TextureData *TexData);
		double ComputeTextureColor(double ElevRange, double SetRGB[3], TextureData *TexData);
		long MakeNoise(long MaxNoise, double Lat, double Lon);
		unsigned char *InitNoise(void);
		static void FreeNoiseMap(void);	// called from ~EffectsLib()
		int EvaluateStrataBump(PixelData *Pix, VertexDEM *StratumNormal, double MaterialWeight);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil);
		int BuildFileComponentsList(EffectList **Coords);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_MATERIALSTRATA);};
		virtual char *GetRAHostTypeString(void) {return ("(Strata)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual void EditRAHost(void);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_MATERIALSTRATA_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

	}; // class MaterialStrata

class TerraGenSampleData
	{
	public:
		double PlanetRad, MetersPerDegLat, CenterLat, CenterLon, YVInc, XHInc, BaseElev, ElevRange, 
			SeedXOffset, SeedYOffset, HighDEMLon, StartY;
		long y;
		TextureData TexData;
		unsigned char Running;

		TerraGenSampleData();

	}; // class TerraGenSampleData

class DEMBounds
	{
	public:
		double North, South, West, East, HighElev, LowElev;

		DEMBounds();
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class DEMBounds

class Object3DVertexList
	{
	public:
		double Lat, Lon;
		Object3DEffect *Obj;
		Joe *Vec;
		VectorPoint *Point;
		char Rendered;

		Object3DVertexList() {Obj = NULL; Vec = NULL; Point = NULL; Rendered = 0; Lat = Lon = 0.0;};
	}; // class Object3DVertexList

class FoliageVertexList
	{
	public:
		double Lat, Lon;
		FoliageEffect *Fol;
		Joe *Vec;
		void *Point;
		char Rendered;

		FoliageVertexList() {Fol = NULL; Vec = NULL; Point = NULL; Rendered = 0; Lat = Lon = 0.0;};
	}; // class FoliageVertexList

class LabelVertexList
	{
	public:
		double Lat, Lon;
		Label *Labl;
		Joe *Vec;
		VectorPoint *Point;
		char Rendered;

		LabelVertexList() {Labl = NULL; Vec = NULL; Point = NULL; Rendered = 0; Lat = Lon = 0.0;};
	}; // class LabelVertexList

enum
	{
	WCS_FENCEPIECE_POST,
	WCS_FENCEPIECE_ALTPOST,
	WCS_FENCEPIECE_SPAN,
	WCS_FENCEPIECE_ROOF,
	WCS_FENCEPIECE_SKINFRAME
	}; // 

class FenceVertexList
	{
	public:
		double Lat, Lon, Elev;
		Fence *Fnce;
		Joe *Vec;
		JoeList *Holes;
		VectorPoint *PointA, *PointB;
		char Rendered, PieceType;

		FenceVertexList() {Fnce = NULL; Vec = NULL; Holes = NULL; PointA = PointB = NULL; Rendered = 0; PieceType = WCS_FENCEPIECE_POST; Lat = Lon = Elev = 0.0;};
		~FenceVertexList();
	}; // class FenceVertexList

class FoliagePreviewData
	{
	public:
		double ColorImageOpacity, Width, Height, Rotate[3], RGB[3];
		Object3DEffect *Object3D;
		Raster *CurRast;
		char FlipX, Shade3D;

		FoliagePreviewData()	
			{Object3D = NULL; CurRast = NULL; ColorImageOpacity = Width = Height = Rotate[0] = Rotate[1] = Rotate[2] = RGB[0] = RGB[1] = RGB[2] = 0.0;
			FlipX = Shade3D = 0;};
	}; // class FoliagePreviewData

#define WCS_OBJECT3DINSTANCE_FLAGBIT_ALIGNTOCAMERA	(1 << 0)	// pertains only to Lightwave
#define WCS_OBJECT3DINSTANCE_FLAGBIT_CLIPMAP		(1 << 1)	// pertains only to Lightwave
#define WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE		(1 << 2)	// pertains only to Lightwave
#define WCS_OBJECT3DINSTANCE_FLAGBIT_FOLIAGE		(1 << 3)	// pertains only to Lightwave
#define WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE		(1 << 4)	// pertains only to Lightwave
#define WCS_OBJECT3DINSTANCE_FLAGBIT_UNSEENBYCAMERA	(1 << 5)	// pertains only to Lightwave

class Object3DInstance
	{
	public:
		Object3DEffect *MyObj;
		Object3DInstance *Next;
		Point3d Euler, Scale, Translation, WCSGeographic, ExportXYZ;	// geogr/position: 0 = lon/x, 1 = lat/y, 2 = elev
		Point4d Quaternion;
		unsigned char Flags;
		long ClickQueryObjectID;

		Object3DInstance()	{MyObj = NULL; Next = NULL;
			Euler[0] = Euler[1] = Euler[2] = 0.0;
			Quaternion[0] = Quaternion[1] = Quaternion[2] = 0.0;
			Quaternion[3] = 1.0;
			Scale[0] = Scale[1] = Scale[2] = 1.0;
			WCSGeographic[0] = WCSGeographic[1] = WCSGeographic[2] = 0.0;
			ExportXYZ[0] = ExportXYZ[1] = ExportXYZ[2] = 0.0; Flags = 0; ClickQueryObjectID = -1;};
		long CountBoundedInstances(RasterBounds *RBounds);
		long IsBounded(RasterBounds *RBounds);
	}; // class Object3DInstance

class ExportReferenceData
	{
	public:
		double WCSRefLat, WCSRefLon, ExportRefLat, ExportRefLon, RefElev, MaxElev, WCSLatScale, WCSLonScale, 
			ExportLatScale, ExportLonScale, ElevScale;

		ExportReferenceData()	{WCSRefLat = WCSRefLon = ExportRefLat = ExportRefLon = RefElev = MaxElev = 
									WCSLatScale = WCSLonScale = ExportLatScale = ExportLonScale = ElevScale = 0.0;};
	}; // class ExportReferenceData

class VectorExportItem
	{
	public:
		Joe *MyJoe;
		VectorPoint *Points;
		long NumPoints;

		VectorExportItem();
		~VectorExportItem();
	}; // class VectorExportItem

class GeoBounds
	{
	public:
		double North, South, West, East;

		GeoBounds()	{North = South = West = East = 0.0;};
		int PointContained(double Lat, double Lon)	{return (Lat <= North && Lat >= South && Lon <= West && Lon >= East);};
	}; // class GeoBounds

#define WCS_MOSTEFFECTS_MAXCELLS	2000000.0
#define WCS_MOSTEFFECTS_MINCELLS	100.0
#define WCS_SHADOWEFFECT_MAXCELLS	2000000.0
#define WCS_SHADOWEFFECT_MINCELLS	1000.0
#define WCS_MOSTEFFECTS_HIRESEDGES_ENABLED	true
#define WCS_MOSTEFFECTS_OVERLAP_ENABLED	true
#define WCS_SHADOW_HIRESEDGES_ENABLED	false
#define WCS_SHADOW_OVERLAP_ENABLED	true

class EcosystemEffectBase
	{
	public:

		float Resolution;
		short Randomize, EdgesExist, GradientsExist, OverlapOK, Floating, DissolveByImageSize, DissolveRefImageHt;
		GeoRaster *GeoRast;
		EcosystemEffect *TempEco;
		double Wt, SumWt;
		PRNGX Rand;

		EcosystemEffectBase();
		void SetDefaults(void);
		#ifdef WCS_BUILD_V3
		GeoRaster *Init(EffectEval *EvalEffects, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		#else // WCS_BUILD_V3
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		#endif // WCS_BUILD_V3
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int Eval(PolygonData *Poly);
		short Compare(EcosystemEffect *Effect, short ValuesRead);
		short CompareWeighted(EcosystemEffect *Effect);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow);
		bool Eval(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow);
		bool Compare(RenderData *Rend, EcosystemEffect *Effect, Joe *RefVec, VectorNode *Node, double ProfWt);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // EcosystemEffectBase

class LakeEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, OverlapOK, Floating;
		double TempBeachLevel;
		GeoRaster *GeoRast;
		AnimMaterialGradient *CurBeach;
		MaterialList *RefMat;
		PRNGX Rand;

		LakeEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);
		int Eval(RenderData *Rend, VertexData *Vert);
		double GetThemedMaxLevel(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double &ThemedWaterLevel, double &ThemedBeachLevel);
		int Compare(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double ThemedWaterLevel);
		int CompareBeach(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double ThemedMaxLevel);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, LakeEffect *DefaultLake, double &SumOfAllCoverages, bool AddBeachMaterials);
		bool AddDefaultLake(RenderData *Rend, VectorNode *CurNode, LakeEffect *DefaultLake, double &SumOfAllCoverages, bool AddBeachMaterials);
		bool AddBeachMaterials(RenderData *Rend, VectorNode *CurNode, MaterialList *BeachWaterMat, double &SumOfAllCoverages);
		double GetThemedMaxLevel(RenderData *Rend, LakeEffect *CurEffect, Joe *CurVec, 
			double &ThemedWaterLevel, double &ThemedBeachLevel);
		MaterialList *Compare(RenderData *Rend, VectorNode *CurNode, LakeEffect *CurEffect, Joe *CurVec, 
			double ThemedWaterLevel, bool &Success);
		bool CompareBeach(RenderData *Rend, VectorNode *CurNode, LakeEffect *CurEffect, Joe *RefVec, 
			double ThemedMaxLevel);

	}; // LakeEffectBase

class EnvironmentEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;
		EnvironmentEffect *TempEnv;
		double Wt, SumWt;
		PRNGX Rand;

		EnvironmentEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		#ifdef WCS_VECPOLY_EFFECTS
		bool Eval(RenderData *Rend, VectorNode *CurNode);
		#else // WCS_VECPOLY_EFFECTS
		int Eval(PolygonData *Poly);
		#endif // WCS_VECPOLY_EFFECTS
		short Compare(EnvironmentEffect *Effect, short ValuesRead);
		short CompareWeighted(EnvironmentEffect *Effect);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // EnvironmentEffectBase

class CmapEffectBase
	{
	public:

		EffectList *DEMSpecificCmaps;
		PRNGX Rand;

		CmapEffectBase();
		void SetDefaults(void);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		int BuildDEMSpecificCmapList(EffectsLib *Lib, DEM *MyDEM);
		int Eval(RenderData *Rend, PolygonData *Poly);
		bool Eval(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow);

	}; // CmapEffectBase

class RasterTerraffectorEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;
		double TempElev, OrigElev, TempRoughness;
		PRNGX Rand;

		RasterTerraffectorEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int Eval(RenderData *Rend, VertexData *Vert, short AfterFractals);
		int Compare(RenderData *Rend, VertexData *Vert, RasterTerraffectorEffect *Effect, double ProfWt);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode);
		bool Compare(RenderData *Rend, RasterTerraffectorEffect *Effect, Joe *RefVec, VectorNode *Node, double ProfWt);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // RasterTerraffectorEffectBase

class TerrainParamEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;

		char HorFractDisplace, BackfaceCull, FrdInvalid, FractalMethod, RegenFDMsEachRender;
		double DepthMapPixSize;

		TerrainParamEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int CreateFractalMaps(int ReportOptimum);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // TerrainParamEffectBase

class GroundEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;
		GroundEffect *TempGround;
		double Wt, SumWt;
		PRNGX Rand;

		GroundEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int Eval(PolygonData *Poly);
		short Compare(GroundEffect *Effect, short ValuesRead);
		short CompareWeighted(GroundEffect *Effect);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, GroundEffect *DefaultGround);
		bool AddDefaultGround(RenderData *Rend, VectorNode *CurNode, GroundEffect *DefaultGround);
		bool Compare(RenderData *Rend, GroundEffect *Effect, Joe *RefVec, VectorNode *Node, double ProfWt);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // GroundEffectBase

class SnowEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;
		SnowEffect *TempSnow;
		double Wt, SumWt;
		PRNGX Rand;

		SnowEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int Eval(PolygonData *Poly);
		short Compare(SnowEffect *Effect, short ValuesRead);
		short CompareWeighted(SnowEffect *Effect);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, SnowEffect *SnowList, double PlowedSnow);
		bool AddDefaultSnow(RenderData *Rend, VectorNode *CurNode, SnowEffect *SnowList, double PlowedSnow);
		bool Compare(RenderData *Rend, SnowEffect *Effect, Joe *RefVec, VectorNode *Node, double ProfWt);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // SnowEffectBase

class TerraffectorEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, HighestSegPriority, Floating;
		GeoRaster *GeoRast;
		double OrigElev, CurElev, CurRough, LastSegPercentDistance, CurSlope;
		EcosystemEffect *CurEco;
		MaterialList *RefMat;
		PRNGX Rand;

		TerraffectorEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		int Eval(RenderData *Rend, VertexData *Vert);
		int Compare(RenderData *Rend, VertexData *Vert, TerraffectorEffect *Effect, Joe *JoeVec, double JoeElev, 
			double JoeSlope, double Distance, float *VecOffsets);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, bool EcoOnly, double &SumOfAllCoverages);
		bool Compare(RenderData *Rend, VectorNode *CurNode, TerraffectorEffect *Effect, Joe *JoeVec, 
			short SegNumber, double JoeElev, double JoeSlope, double Distance, float *VecOffsets);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // TerraffectorEffectBase

class ShadowEffectBase
	{
	public:

		float Resolution;
		short EdgesExist, GradientsExist, OverlapOK, Floating;
		GeoRaster *GeoRast;
		double TempIntensity, TempOffset;

		ShadowEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		short AreThereEdges(GeneralEffect *EffectList);
		short AreThereGradients(GeneralEffect *EffectList);
		int Eval(RenderData *Rend, PolygonData *Poly);
		short Compare(RenderData *Rend, PolygonData *Poly, ShadowEffect *Effect, double ProfWt);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // ShadowEffectBase

class FoliageEffectBase
	{
	public:
		long VerticesToRender;
		FoliageVertexList *VertList;

		FoliageEffectBase();
		void SetDefaults(void);
		int Init(FoliageEffect *EffectList, RenderJoeList *JL);
		void Destroy(void);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, CoordSys *MyCoords, GeoRegister *MyBounds);
		int InitFrameToRender(void);

	}; // FoliageEffectBase

class StreamEffectBase
	{
	public:

		float Resolution;
		short OverlapOK, Floating;
		double TempLevel, TempWaveHt, TempSlope, TempBeachLevel;
		StreamEffect *CurStream;
		AnimMaterialGradient *CurBeach;
		GeoRaster *GeoRast;
		MaterialList *RefMat;
		PRNGX Rand;

		StreamEffectBase();
		void SetDefaults(void);
		GeoRaster *Init(GeneralEffect *EffectList, RenderJoeList *JL, double N, double S, double E, double W, 
			double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem);
		void Destroy(void);
		int Eval(RenderData *Rend, VertexData *Vert);
		int Compare(RenderData *Rend, VertexData *Vert, StreamEffect *Effect, Joe *JoeVec, double JoeElev, 
			double JoeSlope, double Distance, float *VecOffsets);
		int CompareBeach(VertexData *Vert, StreamEffect *Effect, double JoeElev);
		bool Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, bool AddBeachMaterials);
		MaterialList *Compare(RenderData *Rend, VectorNode *CurNode, StreamEffect *CurEffect, Joe *CurVec, double JoeElev, 
			double JoeSlope, double Distance, float *VecOffsets, bool &Success);
		bool CompareBeach(VectorNode *CurNode, StreamEffect *CurEffect, double JoeElev);
		void SetFloating(int NewFloating, Project *CurProj);
		void SetFloating(int NewFloating, float NewResolution);

	}; // StreamEffectBase

class Object3DEffectBase
	{
	public:
		long VerticesToRender;
		Object3DVertexList *VertList;

		Object3DEffectBase();
		void SetDefaults(void);
		int Init(Object3DEffect *EffectList, RenderJoeList *JL);
		void Destroy(void);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, CoordSys *MyCoords, GeoRegister *MyBounds);
		int InitFrameToRender(void);

	}; // Object3DEffectBase

// defines used in early V5 release. Still needed to translate those files.
#define	WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_LOW			1
#define	WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUMLOW	2
#define	WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUM		3
#define	WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUMHIGH	4
#define	WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_HIGH		5

class AtmosphereBase
	{
	public:
		char SpeedBoost;
		long AdaptiveThreshold;

		AtmosphereBase();
		void SetDefaults(void);
		void InitToRender(void);
	}; // class AtmosphereBase

class FenceBase
	{
	public:
		long VerticesToRender;
		FenceVertexList *VertList;

		FenceBase();
		void SetDefaults(void);
		int Init(Database *DBHost, RenderJoeList *&JL);
		void Destroy(void);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, CoordSys *MyCoords, GeoRegister *MyBounds);
		int InitFrameToRender(void);

	}; // FenceBase

class LabelBase
	{
	public:
		long VerticesToRender;
		LabelVertexList *VertList;

		LabelBase();
		void SetDefaults(void);
		int Init(RenderJoeList *JL);
		void Destroy(void);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, CoordSys *MyCoords, GeoRegister *MyBounds);
		int InitFrameToRender(void);

	}; // LabelBase

// <<<>>> ADD_NEW_EFFECTS if there will be a special object to handle evaluation during rendering
// add it here. Usually it implies that the effect can be attached to vectors or needs special values that apply to
// all instances of the effect's class such as Resolution...

#define WCS_EFFECTS_SHADOWMAP_ZOFFSET	100000.0

class EffectsLib
	{
	friend class Joe;

	private:
		CoordSys *DefaultCoords;

	public:
		// area
		//ComboEffect *Combo, *LastCombo;
		LakeEffect *Lake, *LastLake, *DefaultLake;
		EcosystemEffect *Ecosystem, *LastEcosystem;
		WaveEffect *Wave, *LastWave;
		CloudEffect *Cloud, *LastCloud;
		EnvironmentEffect *Environment, *LastEnvironment, *DefaultEnvironment;
		CmapEffect *Cmap, *LastCmap;
		RasterTerraffectorEffect *RasterTA, *LastRasterTA;
		ShadowEffect *Shadow, *LastShadow;


		// line
		TerraffectorEffect *Terraffector, *LastTerraffector;
		StreamEffect *Stream, *LastStream;


		// point
		FoliageEffect *Foliage, *LastFoliage;
		Object3DEffect *Object3D, *LastObject3D;


		// other
		MaterialEffect *Material, *LastMaterial;
		CelestialEffect *Celestial, *LastCelestial;
		StarFieldEffect *StarField, *LastStarField;

		PlanetOpt *PlanetOpts, *LastPlanetOpt;
		TerrainParamEffect *TerrainParam, *LastTerrainParam, *DefaultTerrainParam;
		GroundEffect *Ground, *LastGround, *DefaultGround;
		SnowEffect *Snow, *LastSnow;
		Sky *Skies, *LastSky;
		Atmosphere *Atmospheres, *LastAtmosphere;
		Light *Lights, *LastLight;
		Camera *Cameras, *LastCamera;
		RenderJob *RenderJobs, *LastRenderJob;
		RenderOpt *RenderOpts, *LastRenderOpt;
		TerraGridder *Gridder, *LastGridder;
		TerraGenerator *Generator, *LastGenerator;
		SearchQuery *Search, *LastSearch;
		ThematicMap *Theme, *LastTheme;
		CoordSys *Coord, *LastCoord;
		Fence *Fences, *LastFence;
		PostProcess *PostProc, *LastPostProc;
		RenderScenario *Scenario, *LastScenario;
		DEMMerger *Merger, *LastMerger;
		SceneExporter *Exporter, *LastExporter;
		Label *Labels, *LastLabel;
// <<<>>> ADD_NEW_EFFECTS create a name derived from the effect type and add first and last pointers

		LakeEffectBase LakeBase;
		EcosystemEffectBase EcosystemBase;
		EnvironmentEffectBase EnvironmentBase;
		CmapEffectBase CmapBase;
		RasterTerraffectorEffectBase RasterTerraffectorBase;
		TerrainParamEffectBase TerrainParamBase;
		GroundEffectBase GroundBase;
		SnowEffectBase SnowBase;
		TerraffectorEffectBase TerraffectorBase;
		ShadowEffectBase ShadowBase;
		FoliageEffectBase FoliageBase;
		StreamEffectBase StreamBase;
		Object3DEffectBase Object3DBase;
		AtmosphereBase AtmoBase;
		FenceBase FnceBase;
		LabelBase LablBase;
// <<<>>> ADD_NEW_EFFECTS if there is a special class to handle evaluation during rendering add an embedded instance here

		UBYTE GroupVecPolyEnabled[WCS_MAXIMPLEMENTED_EFFECTS];
		UBYTE GroupDisplayAdvancedEnabled[WCS_MAXIMPLEMENTED_EFFECTS];
		long MaxMem, ProjectFileSavedWithForestEd;
		static PRNGX EffectsRand;
		static char LoadQueries;
		static char *DefaultExtensions[WCS_MAXIMPLEMENTED_EFFECTS]; 
		static char *DefaultPaths[WCS_MAXIMPLEMENTED_EFFECTS]; 
		static char *CelestialPresetName[WCS_PRESETS_NUMCELESTIALS];
		static double CelestialPresetRadius[WCS_PRESETS_NUMCELESTIALS];
		static double CelestialPresetDistance[WCS_PRESETS_NUMCELESTIALS];
		static unsigned char AlphabetizedEffects[WCS_MAXIMPLEMENTED_EFFECTS];
		static EffectJoeList *EffectChain[1000];

		EffectsLib();
		~EffectsLib();
		GeneralEffect *AddEffect(long EffectClass, char *NewName = NULL, GeneralEffect *Proto = NULL);
		void RemoveEffect(GeneralEffect *Remove);
		GeneralEffect *GetListPtr(long EffectClass);
		void SetListHead(long EffectClass, GeneralEffect *NewHead);
		void SetListTail(long EffectClass, GeneralEffect *NewTail);
		int EffectTypeImplemented(long EffectClass);
		ULONG Load(FILE *ffile, ULONG ReadSize, struct ChunkIODetail *Detail);
		ULONG OtherEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail);
		ULONG AreaEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail);
		ULONG LineEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail);
		ULONG PointEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail);

		//ULONG ComboEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG LakeEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG EcosystemEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG FogEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG WaveEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG CloudEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG CloudShadowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG EnvironmentEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG TintEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG CmapEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG LandWaveEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG RasterTerraffectorEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG ShadowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);

		ULONG TerraffectorEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG PathFollowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG PathConformEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG MorphEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG StreamEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);

		ULONG FoliageEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Object3DEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG ObjectVecEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);

		ULONG MaterialEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG BackgroundEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG CelestialEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG StarFieldEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);

		ULONG PlanetOpt_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG TerrainParamEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG GroundEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG SnowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Sky_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Atmosphere_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Light_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Camera_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG RenderJob_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG RenderOpt_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG TerraGridder_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG TerraGenerator_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG SearchQuery_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG ThematicMap_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG CoordSys_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Fence_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG PostProcess_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG RenderScenario_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG DEMMerger_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG SceneExporter_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		ULONG Label_Load(FILE *ffile, ULONG ReadSize, short ByteFlip);

// <<<>>> ADD_NEW_EFFECTS every effect class needs a load function to load all instances in the project file

		ULONG Save(FILE *ffile, struct ChunkIODetail *Detail);
		ULONG OtherEffects_Save(FILE *ffile, struct ChunkIODetail *Detail);
		ULONG AreaEffects_Save(FILE *ffile, struct ChunkIODetail *Detail);
		ULONG LineEffects_Save(FILE *ffile, struct ChunkIODetail *Detail);
		ULONG PointEffects_Save(FILE *ffile, struct ChunkIODetail *Detail);

		//ULONG ComboEffect_Save(FILE *ffile);
		ULONG LakeEffect_Save(FILE *ffile);
		ULONG EcosystemEffect_Save(FILE *ffile);
		ULONG FogEffect_Save(FILE *ffile);
		ULONG WaveEffect_Save(FILE *ffile);
		ULONG CloudEffect_Save(FILE *ffile);
		ULONG CloudShadowEffect_Save(FILE *ffile);
		ULONG EnvironmentEffect_Save(FILE *ffile);
		ULONG TintEffect_Save(FILE *ffile);
		ULONG CmapEffect_Save(FILE *ffile);
		ULONG LandWaveEffect_Save(FILE *ffile);
		ULONG RasterTerraffectorEffect_Save(FILE *ffile);
		ULONG ShadowEffect_Save(FILE *ffile);

		ULONG TerraffectorEffect_Save(FILE *ffile);
		ULONG PathFollowEffect_Save(FILE *ffile);
		ULONG PathConformEffect_Save(FILE *ffile);
		ULONG MorphEffect_Save(FILE *ffile);
		ULONG StreamEffect_Save(FILE *ffile);

		ULONG FoliageEffect_Save(FILE *ffile);
		ULONG Object3DEffect_Save(FILE *ffile);
		ULONG ObjectVecEffect_Save(FILE *ffile);

		ULONG MaterialEffect_Save(FILE *ffile);
		ULONG BackgroundEffect_Save(FILE *ffile);
		ULONG CelestialEffect_Save(FILE *ffile);
		ULONG StarFieldEffect_Save(FILE *ffile);

		ULONG PlanetOpt_Save(FILE *ffile);
		ULONG TerrainParamEffect_Save(FILE *ffile);
		ULONG GroundEffect_Save(FILE *ffile);
		ULONG SnowEffect_Save(FILE *ffile);
		ULONG Sky_Save(FILE *ffile);
		ULONG Atmosphere_Save(FILE *ffile);
		ULONG Light_Save(FILE *ffile);
		ULONG Camera_Save(FILE *ffile);
		ULONG RenderJob_Save(FILE *ffile);
		ULONG RenderOpt_Save(FILE *ffile);
		ULONG TerraGridder_Save(FILE *ffile);
		ULONG TerraGenerator_Save(FILE *ffile);
		ULONG SearchQuery_Save(FILE *ffile);
		ULONG ThematicMap_Save(FILE *ffile);
		ULONG CoordSys_Save(FILE *ffile);
		ULONG Fence_Save(FILE *ffile);
		ULONG PostProcess_Save(FILE *ffile);
		ULONG RenderScenario_Save(FILE *ffile);
		ULONG DEMMerger_Save(FILE *ffile);
		ULONG SceneExporter_Save(FILE *ffile);
		ULONG Label_Save(FILE *ffile);

// <<<>>> ADD_NEW_EFFECTS every effect class needs a save function to save all instances in the library

		void ResolveLoadLinkages(void);
		void ResolveDBLoadLinkages(Database *HostDB);
		void SetDefaults(void);
		void DeleteAll(int ResetDefaults);
		void DeleteGroup(long EffectType);
		int InitAll(void);
		void FreeAll(int FinalCleanup);
		GeneralEffect *FindByName(long EffectType, char *FindName);
		GeneralEffect *FindDuplicateByName(long EffectType, GeneralEffect *FindMe);
		char GetIconType(long EffectClass);
		int ContributesToMaterials(long EffectClass);
		GeneralEffect *GetDefaultEffect(long EffectClass, short MakeEffect, Database *DBSource);
		char *GetEffectTypeName(long EffectType);
		char *GetEffectTypeNameNonPlural(long EffectType);
		long GetEffectTypeFromName(char *GroupName);
		int EnabledEffectExists(long EffectType);
		char *CreateUniqueName(long EffectType, char *NameBase);
		void ApplicationSetTime(double Time);
		unsigned long GetObjectID(long EffectType, GeneralEffect *FindMe);
		GeneralEffect *FindByID(long EffectType, unsigned long MatchID);
		long CountEffects(long EffectClass, int EnabledOnly);
		long CloneEffects(EffectsLib *EffectsHost, long EffectClass, int EnabledOnly);
		GeneralEffect *CloneNamedEffect(EffectsLib *EffectsHost, long EffectClass, char *FXName);
		short IsEffectValid(GeneralEffect *TestMe, char CheckChildren);
		short IsEffectValid(GeneralEffect *TestMe, long EffectType, char CheckChildren);
		short RemoveFromJoeLists(Joe *RemoveMe);
		long RemoveDBLinks(void);
		void EditNext(short GoForward, GeneralEffect *Current, int EffectClass);
		void ReGenerateDelayedEditNext(unsigned char SubClass, void *NotifyData);
		void SetShadowMapRegen(void);
		short AnimateShadows(void);
		bool AnimateMaterials(void);
		MaterialEffect *SetMakeMaterial(ObjectMaterialEntry *Me, MaterialAttributes *MatList);
		Raster *SetMakeTexture(MaterialTextureInfo *MTI, MaterialEffect *NameTest, int TexChannel, 
			bool InvertTexture, bool AlphaOnly);
		void MaterialNameChanging(char *OldName, char *NewName);
		void RemoveObjectMaterials(Object3DEffect *PurgeMe);
		GeneralEffect *MatchNameMakeEffect(GeneralEffect *MatchMe);
		GeneralEffect *MatchNameMakeEffectNoValidation(GeneralEffect *MatchMe);	// doesn't validate the effect being copied
		CoordSys *CompareMakeCoordSys(CoordSys *MatchMe, int MakeNew);
		int RemoveRAHost(RasterAnimHost *RemoveMe, int NoQuery);
		void AddAttributeByList(RasterAnimHost *AddToMe, long EffectType, ThematicOwner *ThemeOwner = NULL, long ThemeNumber = 0);
		void GetRAHostProperties(RasterAnimHostProperties *Prop);	// call for getting key frame range
		int SetRAHostProperties(RasterAnimHostProperties *DropSource);	// call for setting key frame range or removing keys
		void SyncFloaters(Database *CurDB, Project *CurProj, int SyncResolution);
		double GetPlanetRadius(void);
		void SetPlanetOptEnabled(PlanetOpt *Me, short EnabledState);
		char GetFractalMethod(void) { return(TerrainParamBase.FractalMethod);};
		void SetFractalMethod(char NewMethod) { TerrainParamBase.FractalMethod = NewMethod;};
		char GetBackFaceCull(void) { return(TerrainParamBase.BackfaceCull);};
		void SetBackFaceCull(char NewCull) { TerrainParamBase.BackfaceCull = NewCull;};
		void UpdateDefaultCoords(CoordSys *Source, int SyncAll);
		void UpdateExportDefaultCoords(CoordSys *Source, int SyncAll, Database *ExportDB, Project *ExportProj);
		CoordSys *FetchDefaultCoordSys(void) {return (DefaultCoords);};
		long CountRenderItemVertices(RenderData *Rend, Database *DB);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM);
		void FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, CoordSys *MyCoords, GeoRegister *MyBounds);
		EcosystemEffect *FindEcoInEnvironment(EcosystemEffect *FindMe);
		
		// Rendering stuff
		int TestInitToRender(long EffectClass, short ElevationOnly);
		int TestInitToRender(int EffectClass, bool ElevationOnly, bool AreaEffects, bool LineEffects);
		#ifdef WCS_VECPOLY_EFFECTS
		int InitToRender(Database *DBase, Project *CurProj, RenderOpt *Opt, BufferNode *Buffers, EffectEval *EvalEffects, long Width, long Height, 
			int InitForShadows, int InitElevationEffectsOnly, int FirstInit);
		#else // WCS_VECPOLY_EFFECTS
		int InitToRender(Database *DBase, Project *CurProj, RenderOpt *Opt, BufferNode *Buffers, long Width, long Height, 
			int InitForShadows, int InitElevationEffectsOnly, int FirstInit);
		#endif // WCS_VECPOLY_EFFECTS
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int InitFrameToRender(RenderData *Rend);		// call this before rendering a frame after time value has been set
		int InitBaseToRender(void);		// call this before rendering a frame or after shadow mapping after time value has been set
		char GetMaxFractalDepth(void);
		void LogInitMemoryUsage(int Category, long UsedMemStart, long UsedMemEnd);
		GeoRaster *AreThereLakes(void) { return (LakeBase.GeoRast);};
		GeoRaster *AreThereEnvironments(void) { return (EnvironmentBase.GeoRast);};
		GeoRaster *AreThereEcosystems(void) { return (EcosystemBase.GeoRast);};
		GeoRaster *AreThereRasterTerraffectors(void) { return (RasterTerraffectorBase.GeoRast);};
		GeoRaster *AreThereTerrainParams(void) { return (TerrainParamBase.GeoRast);};
		GeoRaster *AreThereGrounds(void) { return (GroundBase.GeoRast);};
		GeoRaster *AreThereSnows(void) { return (SnowBase.GeoRast);};
		GeoRaster *AreThereTerraffectors(void) { return (TerraffectorBase.GeoRast);};
		GeoRaster *AreThereShadows(void) { return (ShadowBase.GeoRast);};
		GeoRaster *AreThereStreams(void) { return (StreamBase.GeoRast);};
		int AreThereFoliages(void) { return (FoliageBase.VertList ? 1: 0);};
		int AreThereObject3Ds(void) { return (Object3DBase.VertList ? 1: 0);};
		int AreThereLabels(void) { return (LablBase.VertList ? 1: 0);};
		int AreThereVolumetricAtmospheres(void);
		int AreThereLightTextures(void);
		int AreThereAtmoTextures(void);
		int AreThereOpticallyTransparentEffects(void);
		int BuildDEMSpecificCmapList(DEM *MyDEM) {return (CmapBase.BuildDEMSpecificCmapList(this, MyDEM));};
		int SetTfxGeoClip(GeoBounds *GeoClip, Database *DBase, double EarthLatScaleMeters);
		VolumetricSubstance *BuildVolumeSubstanceList(RenderOpt *Opt);
		void InitScenarios(EffectList *UsedScenarios, double RenderTime, Database *DBHost);
		void RestoreScenarios(EffectList *UsedScenarios, Database *DBHost);
		int UpdateScenarios(EffectList *ProcessScenarios, double RenderTime, Database *DBHost);
		void SortEffectsByAlphabet(long EffectClass);
		int GetEffectTypePrecedence(long EffectType);
		static void SortEffectChain(long &EffectCt, long EffectType);
		void SetDisplayAdvanced(long EffectClass, UBYTE DisplayAdvanced)
			{GroupDisplayAdvancedEnabled[EffectClass] = DisplayAdvanced;};
		UBYTE GetDisplayAdvanced(long EffectClass)
			{return(GroupDisplayAdvancedEnabled[EffectClass]);};

// <<<>>> ADD_NEW_EFFECTS some of these are used by the renderer mostly to check for GeoRasters initialized

		// added for v5
		void EvalRasterTerraffectors(RenderData *Rend, VertexData *Vert, short AfterFractals);
		void EvalTerraffectors(RenderData *Rend, VertexData *Vert);
		void EvalStreams(RenderData *Rend, VertexData *Vert);
		void EvalLakes(RenderData *Rend, VertexData *Vert);
		void EvalEcosystems(PolygonData *Poly);
		void EvalCmaps(RenderData *Rend, PolygonData *Poly);
		#ifdef WCS_VECPOLY_EFFECTS
		void EvalEnvironment(RenderData *Rend, VectorNode *CurNode);
		#else // WCS_VECPOLY_EFFECTS
		void EvalEnvironment(PolygonData *Poly);
		#endif // WCS_VECPOLY_EFFECTS
		void EvalSnow(PolygonData *Poly);
		void EvalGround(PolygonData *Poly);
		void EvalShadows(RenderData *Rend, PolygonData *Poly);

		void EvalRasterTAsNoInit(RenderData *Rend, VertexData *Vert, short AfterFractals);
		void EvalTerraffectorsNoInit(RenderData *Rend, VertexData *Vert);
		void EvalLakesNoInit(RenderData *Rend, VertexData *Vert)	{};
		void EvalStreamsNoInit(RenderData *Rend, VertexData *Vert)	{};

		// VNS 3 versions
		bool EvalRasterTerraffectors(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode)
			{return (RasterTerraffectorBase.Eval(Rend, MyEffects, CurNode));};
		bool EvalTerraffectors(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, bool EcoOnly, double &SumOfAllCoverages)
			{return (TerraffectorBase.Eval(Rend, MyEffects, CurNode, EcoOnly, SumOfAllCoverages));};
		bool EvalLakes(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, bool AddBeachMaterials)
			{return (LakeBase.Eval(Rend, MyEffects, CurNode, DefaultLake, SumOfAllCoverages, AddBeachMaterials));};
		bool AddDefaultLake(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, bool AddBeachMaterials)
			{return (LakeBase.AddDefaultLake(Rend, CurNode, DefaultLake, SumOfAllCoverages, AddBeachMaterials));};
		bool AddBeachMaterials(RenderData *Rend, VectorNode *CurNode, MaterialList *BeachWaterMat, double &SumOfAllCoverages)
			{return (LakeBase.AddBeachMaterials(Rend, CurNode, BeachWaterMat, SumOfAllCoverages));};
		bool EvalStreams(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, bool AddBeachMaterials)
			{return (StreamBase.Eval(Rend, MyEffects, CurNode, SumOfAllCoverages, AddBeachMaterials));};
		bool EvalEcosystems(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
			{return (GroupVecPolyEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] ? 
			EcosystemBase.Eval(Rend, MyEffects, CurNode, SumOfAllCoverages, PlowedSnow):
			EcosystemBase.Eval(Rend, CurNode, SumOfAllCoverages, PlowedSnow));};
		bool EvalColorMaps(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
			{return (CmapBase.Eval(Rend, CurNode, SumOfAllCoverages, PlowedSnow));};
		bool EvalGrounds(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode)
			{return (GroundBase.Eval(Rend, MyEffects, CurNode, DefaultGround));};
		bool AddDefaultGround(RenderData *Rend, VectorNode *CurNode)
			{return (GroundBase.AddDefaultGround(Rend, CurNode, DefaultGround));};
		bool EvalSnows(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double PlowedSnow)
			{return (SnowBase.Eval(Rend, MyEffects, CurNode, Snow, PlowedSnow));};
		bool AddDefaultSnow(RenderData *Rend, VectorNode *CurNode, double PlowedSnow)
			{return (SnowBase.AddDefaultSnow(Rend, CurNode, Snow, PlowedSnow));};
		// and a method to evaluate a list of effects for their possible effect on elevation
		void EvalHighLowEffectExtrema(VectorPolygonListDouble *ListOfPolygons, RenderData *Rend, double &RelativeEffectAdd,
			double &RelativeEffectSubtract, double &AbsoluteEffectMax, double &AbsoluteEffectMin);

// <<<>>> ADD_NEW_EFFECTS evaluation functions used by the renderer

	}; // EffectsLib

class GeneralEffect : public RasterAnimHost, public RootTextureParent, public ThematicOwner
	{
	public:
		GeneralEffect *Prev, *Next;
		JoeList *Joes, *RenderJoes;
		SearchQuery *Search;
		long EffectType;
		short Enabled, Priority, HiResEdge, UseGradient, Absolute, RenderOccluded;
		char Name[WCS_EFFECT_MAXNAMELENGTH];

		GeneralEffect(RasterAnimHost *RAHost);
		virtual ~GeneralEffect();
		void Copy(GeneralEffect *CopyTo, GeneralEffect *CopyFrom);
		void SetDefaults(void);
		JoeList *AddToJoeList(Joe *Add);
		JoeList *FastDangerousAddToJoeList(Joe *Add);
		int RemoveFromJoeList(Joe *Remove);
		int RemoveFromJoeList(JoeList *Remove);
		char *GetName(void) {return (Name);};
		char *SetUniqueName(EffectsLib *Library, char *NameBase);
		int GetEnabled(void) {return ((int)Enabled);};
		void SetEnabled(short NewState) {Enabled = NewState;};
		int SetQuery(SearchQuery *NewQuery);
		void HardLinkSelectedVectors(Database *DBHost);
		void SelectVectors(Database *DBHost);
		void EnableVectors(Database *DBHost, bool NewState);
		void AddRenderJoe(Joe *AddMe);
		void RemoveRenderJoes(void);
		int BuildFileComponentsList(EffectList **Queries, EffectList **Themes, EffectList **Coords);
		virtual unsigned long Save(FILE *ffile) = 0;
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip) = 0;
		virtual short AnimateShadows(void) {return (0);};
		virtual bool AnimateMaterials(void);
		virtual void SetColor(short NewColor) {return;};
		virtual long GetNumAnimParams(void)						{return (0);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum)		{return (NULL);};
		virtual double GetMaxProfileDistance(void)				{return (0.0);};
		virtual int GetRAHostAnimated(void);
		virtual int SetToTime(double Time);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual void Edit(void)									{return;};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0) = 0;
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetRAHostTypeNumber(void)					{return (EffectType);};
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual int GetPopClassFlags(RasterAnimHostProperties *Prop);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual long InitImageIDs(long &ImageID);
		virtual void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)	{return;};
		virtual int GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2]);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName)		{return (-2);};
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
			{return (-2);};
		virtual void ResolveLoadLinkages(EffectsLib *Lib);
		virtual void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)	{return;};
		virtual void HardLinkVectors(Database *DBHost);
		void SetDisplayAdvanced(EffectsLib *Lib, UBYTE DisplayAdvanced)
			{Lib->SetDisplayAdvanced(EffectType, DisplayAdvanced);};
		UBYTE GetDisplayAdvanced(EffectsLib *Lib)	{return(Lib->GetDisplayAdvanced(EffectType));};

		// inherited from RootTextureParent
		virtual char *GetTextureName(long TexNumber)			{return ("");};
		virtual RootTexture *NewRootTexture(long TexNum)		{return (NULL);};
		virtual long GetNumTextures(void)						{return (0);};
		virtual RootTexture *GetTexRootPtr(long TexNum)			{return (NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)			{return (NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum)		{return (NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {return;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum)				{return ("");};
		virtual long GetNumThemes(void)							{return (0);};
		virtual ThematicMap *GetTheme(long ThemeNum)			{return (NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)			{return (NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum)		{return (NULL);};
		virtual int SetTheme(long ThemeNum, ThematicMap *NewTheme);	// this doesn't need to be implemented in derived classes
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme) {return (NULL);};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifyClass(void) {return (RAParent ? RAParent->GetNotifyClass(): (unsigned char)EffectType);};
		virtual unsigned char GetNotifySubclass(void) {return ((unsigned char)EffectType);};
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual char *OKRemoveRaster(void)						{return (NULL);};
		virtual void RemoveRaster(RasterShell *Shell)			{return;};
		virtual void EditRAHost(void)							{RAParent ? RAParent->EditRAHost(): Edit();};
		virtual char *GetRAHostName(void)						{return (RAParent ? RAParent->GetRAHostName(): Name);};
		virtual char *GetRAHostTypeString(void)					{return ("(Effect)");};
		virtual char *GetCritterName(RasterAnimHost *TestMe)			{return ("");};
		//virtual long GetNextKeyFrame(double &NewDist, short Direction, double CurrentDist);
		//virtual long DeleteKeyFrame(double FirstDist, double LastDist);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHostQuery(RasterAnimHost *RemoveMe)	{return(RasterAnimHost::RemoveRAHostQuery(RemoveMe));};
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace) {ApplyToColor = 0; ApplyToDisplace = 0;};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);	// return 1 means object found and deleted
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother)	{return (NULL);};
		virtual void SetFloating(char NewFloating)		{return;};
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)	{if (RAParent) RAParent->GetInterFlags(Prop, FlagMe); else Prop->InterFlags = 0; return;};
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation) 
			{return (RAParent ? RAParent->ScaleMoveRotate(MoveMe, Data, Operation): 0);};
		virtual int ComponentOverwriteOK(RasterAnimHostProperties *Prop) {return (1);};
		virtual void HeresYourNewFilePathIfYouCare(char *NewFullPath) {return;};

		// SRAH Popup Context Menu-handling methods
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		
	}; // GeneralEffect

/* retired - used in WCS 6 files, obsolete in WCS 7 and VNS 3, removed 3/1/07
class ComboEffect: public GeneralEffect
	{
	public:

		EffectList *Effects;

		// Initialize GeneralEffect base members in constructor...
		ComboEffect(RasterAnimHost *RAHost, EffectsLib *Library, EffectList *List);
		ComboEffect(RasterAnimHost *RAHost, EffectsLib *Library);
		~ComboEffect();

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Combo Effect)");};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// inherited from RasterAnimHost

	}; // ComboEffect
*/
enum
	{
	WCS_THEMATICMAP_NULLTREATMENT_IGNORE,
	WCS_THEMATICMAP_NULLTREATMENT_CONSTANT
	}; // thematic map null treatments

enum
	{
	WCS_THEMATICMAP_OUTCHANNELS_ONE,
	WCS_THEMATICMAP_OUTCHANNELS_THREE
	}; // thematic map null treatments

class ThematicMap: public GeneralEffect
	{
	public:
		double AttribFactor[3], NullConstant[3];
		DBFilterEvent Condition;
		char OutputChannels, NullTreatment, ConditionEnabled,
			AttribField[3][WCS_EFFECT_MAXNAMELENGTH];	// careful! layer names may be longer than this

		// Initialize GeneralEffect base members in constructor...
		ThematicMap();		// default constructor
		ThematicMap(RasterAnimHost *RAHost);
		ThematicMap(RasterAnimHost *RAHost, EffectsLib *Library, ThematicMap *Proto);
		~ThematicMap();
		void SetDefaults(void);
		void Copy(ThematicMap *CopyTo, ThematicMap *CopyFrom);
		int SetAttribute(LayerEntry *NewAttrib, int AttribNum);
		bool Eval(double Value[3], Joe *CurVec);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Thematic Map)");};
		char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // ThematicMap

enum
	{
	WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY,		//0
	WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY,		//1
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY,		//2
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP,		//3
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT,	//4	//new position
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2,		//5
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2,		//6
	WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2,	//7	//new
	WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE,	//8
	WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP,		//9
	WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT,		//10
	WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY,		//12
	WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY,		//13
	WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH,		//14
	WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE,		//15
	WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH,		//16
	WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY,	//17
	WCS_EFFECTS_MATERIAL_NUMANIMPAR
	}; // animated MaterialEffect params

enum
	{
	WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR,		//0
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR,		//1
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2,	//2	//new
	WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY,		//3
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY,		//4
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY,		//5
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP,		//6
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT,	//7	//new position
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2,		//8
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2,		//9
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2,	//10	//new
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE,	//11
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP,		//12
	WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT,		//13
	WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY,		//14
	WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY,		//15
	WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE,		//16
	WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH,		//17
	WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY,	//18
	WCS_EFFECTS_MATERIAL_TEXTURE_BUMP,				//19
	WCS_EFFECTS_MATERIAL_NUMTEXTURES
	}; // MaterialEffect textures

enum
	{
	WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR,
	WCS_EFFECTS_MATERIAL_NUMTHEMES
	}; // MaterialEffect themes

#define WCS_EFFECTS_MATERIAL_V4NUMTEXTURES	8	// the number of textures used in v4

enum
	{
	WCS_EFFECTS_MATERIALTYPE_OBJECT3D,
	WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM,
	WCS_EFFECTS_MATERIALTYPE_GROUND,
	WCS_EFFECTS_MATERIALTYPE_SNOW,
	WCS_EFFECTS_MATERIALTYPE_WATER,
	WCS_EFFECTS_MATERIALTYPE_FOAM,
	WCS_EFFECTS_MATERIALTYPE_BEACH,
	WCS_EFFECTS_MATERIALTYPE_FENCE,
	WCS_EFFECTS_NUMMATERIALTYPES
	}; // material types

enum
	{
	WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE,
	WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA,
	WCS_EFFECTS_MATERIAL_MISCENTITY_WAVES,
	WCS_EFFECTS_MATERIAL_MISCENTITY_FOAM,
	WCS_EFFECTS_MATERIAL_NUMMISCENTITIES
	}; // material types


class MaterialTextureInfo;

class MaterialAttributes
	{
	public:
		short Shading, Red, Green, Blue, DoubleSided;
		float FPRed, FPGreen, FPBlue;
		char UseFPGuns;
		double SmoothAngle, Luminosity, Diffuse, Transparency, Translucency, Specularity, SpecExponent, Reflectivity, BumpIntensity, DiffuseSharpness;
		MaterialTextureInfo *MTI[WCS_EFFECTS_MATERIAL_NUMTEXTURES];

		MaterialAttributes();
		~MaterialAttributes();
		MaterialTextureInfo *GetCreateMaterialTextureInfo(int Attribute); // Attribute like WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR
	}; // class MaterialAttributes


class MaterialEffect: public GeneralEffect
	{

	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_MATERIAL_NUMANIMPAR];
		AnimColorTime DiffuseColor, SpecularColor, SpecularColor2;
		char MaterialType, PixelTexturesExist, TransparencyTexturesExist, ShortName[24];
		short Shading, DoubleSided, FlipNormal;
		double SmoothAngle, CosSmoothAngle, MaxWaveAmp, MinWaveAmp, WaveAmpRange, OverstoryDissolve, UnderstoryDissolve;
		Ecotype *EcoFol[2];	// 0 = overstory, 1 = understory
		RootTexture *TexRoot[WCS_EFFECTS_MATERIAL_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_MATERIAL_NUMTHEMES];
		MaterialStrata *Strata;
		MaterialEffect *Foam;
		EffectList *Waves;	//new
		static unsigned char ParamAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMANIMPAR];
		static unsigned char TextureAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMTEXTURES];
		static unsigned char MiscEntityAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMMISCENTITIES];

		// Initialize GeneralEffect base members in constructor...
		MaterialEffect(char NewMatType);		// default constructor
		MaterialEffect(RasterAnimHost *RAHost, char NewMatType);
		MaterialEffect(RasterAnimHost *RAHost, EffectsLib *Library, MaterialEffect *Proto, char NewMatType);
		~MaterialEffect();		// gotta delete textures
		void SetDefaults(void);
		void Copy(MaterialEffect *CopyTo, MaterialEffect *CopyFrom);
		int ApproveTexture(int TexNum)			{return(TextureAllowed[MaterialType][TexNum]);};
		int ApproveParam(int ParamNum)			{return(ParamAllowed[MaterialType][ParamNum]);};
		int ApproveMiscEntity(int EntityNum)	{return(MiscEntityAllowed[MaterialType][EntityNum]);};
		short AnimateShadow3D(void);
		short AnimateEcoShadows(void);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **WaveList, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil, MaterialStrata **&StrataAffil, 
			MaterialEffect **&FoamAffil, Ecotype **&EcoAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil, MaterialStrata **&StrataAffil, 
			MaterialEffect **&FoamAffil, Ecotype **&EcoAffil);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB, AnimCritter *&AnimAffil, RootTexture **&TexAffil, ThematicMap **&ThemeAffil, 
			MaterialStrata **&StrataAffil, MaterialEffect **&FoamAffil, Ecotype **&EcoAffil);

		int ConfigureTextureForEcosystem(void);
		int ConfigureTextureForObject3D(void);
		Ecotype *GetEcotypePtr(long EcoNumber)	{return (EcoNumber < 2 ? EcoFol[EcoNumber]: NULL);};
		Ecotype *NewEcotype(long EcoNumber);
		MaterialStrata *GetStrataPtr(void)	{return (Strata);};
		MaterialStrata *NewStrata(void);
		MaterialEffect *GetFoamPtr(void)	{return (Foam);};
		MaterialEffect *NewFoam(void);
		void Evaluate(PixelData *Pix, double MaterialWeight);
		void EvaluateTransparency(PixelData *Pix, double MaterialWeight);
		double EvaluateDisplacement(TextureData *TexData);
		int EvaluateTerrainBump(RenderData *Rend, PixelData *Pix, double *NormalAdd, double MaterialWeight, 
			double PixWidth, double PlanetRad);
		int Evaluate3DBump(RenderData *Rend, PixelData *Pix, double *NormalAdd, double MaterialWeight, 
			double PixWidth, Object3DEffect *Object);
		double EvaluateOpacity(PixelData *Pix);
		int VertexColorsAvailable(void);
		int VertexUVWAvailable(void);
		double GetMaxWaveAmp(double &Displacement);
		double EvalWaves(RenderData *Rend, VertexData *Vert);
		double EvalWaves(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat);
		int IsThereOpticallyTransparentMaterial(void) 
			{return (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH].CurValue > 0.0);};
		int IsThereTransparentMaterial(void) 
			{return (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue > 0.0);};
		int AreTexturesTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY);
		void GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod);
		char **GetVertexMapNames(int MapType);	// 0 = UVW, 1 = ColorPerVertex

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_MATERIAL_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_MATERIAL_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual int GetPopClassFlags(RasterAnimHostProperties *Prop);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual long InitImageIDs(long &ImageID);
		virtual int GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2]);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_MATERIAL_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_MATERIAL_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_MATERIAL_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_MATERIAL_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_MATERIAL_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_MATERIAL_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_MATERIAL_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_MATERIAL_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Material)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// obsolete
		//struct ChunkIODetail *SaveIODetail(void);
		//int SetIOColorList(short *ColorList);

	}; // MaterialEffect

enum
	{
	WCS_EFFECTS_LAKE_ANIMPAR_ELEV,
	WCS_EFFECTS_LAKE_ANIMPAR_WATERLINEREF,
	WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT,
	WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR,
	WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER,
	WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER,
	WCS_EFFECTS_LAKE_NUMANIMPAR
	}; // animated LakeEffect params

enum
	{
	WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR,
	WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER,
	WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER,
	WCS_EFFECTS_LAKE_NUMTEXTURES
	}; // LakeEffect textures

enum
	{
	WCS_EFFECTS_LAKE_THEME_ELEV,
	WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER,
	WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER,
	WCS_EFFECTS_LAKE_NUMTHEMES
	}; // LakeEffect themes

enum
	{
	WCS_LAKE_REFTYPE_WATERLINE,
	WCS_LAKE_REFTYPE_ANIMVALUE
	}; // waterline ref types

class LakeEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_LAKE_NUMANIMPAR];
		char WaterlineRefType, PixelTexturesExist;
		double Level, MaxLevel, MaxWaveHeight, MaxBeachLevel;
		AnimMaterialGradient BeachMat, WaterMat;
		RootTexture *TexRoot[WCS_EFFECTS_LAKE_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_LAKE_NUMTHEMES];

		// Initialize GeneralEffect base members in constructor...
		LakeEffect();		// default constructor
		LakeEffect(RasterAnimHost *RAHost);
		LakeEffect(RasterAnimHost *RAHost, EffectsLib *Library, LakeEffect *Proto);
		~LakeEffect();
		void SetDefaults(void);
		void Copy(LakeEffect *CopyTo, LakeEffect *CopyFrom);
		double GetRenderBeachMatGradientPos(RenderData *Rend, PolygonData *Poly);
		double GetRenderBeachMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat);
		double GetRenderWaterMatGradientPos(RenderData *Rend, PolygonData *Poly);
		double GetRenderWaterMatGradientPos(RenderData *Rend, VertexData *Vert);
		double GetRenderWaterMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		int IsThereOpticallyTransparentMaterial(void)	{return (WaterMat.IsThereOpticallyTransparentMaterial());};
		void GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod)	{WaterMat.GetWaterDepthAndWaveRange(MaximumMod, MinimumMod);};

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_LAKE_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_LAKE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_LAKE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_LAKE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_LAKE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_LAKE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Lake)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_LAKE_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_LAKE_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_LAKE_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_LAKE_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

	}; // LakeEffect

enum
	{
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE,
	WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER,
	WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR
	}; // animated EcosystemEffect params

enum
	{
	WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER,
	WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES
	}; // EcosystemEffect textures

enum
	{
	WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER,
	WCS_EFFECTS_ECOSYSTEM_NUMTHEMES
	}; // EcosystemEffect themes

#define WCS_SEEDOFFSET_ECOSYSTEM		83		// this is an offset added to the vertex seed values

class EcosystemEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		short Transparent, PlowSnow, CmapMatch, CmapMatchRange, MatchColor[6];
		double Line, Skew, SkewAzimuth, MaxSlope, MinSlope, MaxRelEl, MinRelEl;
		AnimMaterialGradient EcoMat;
		RootTexture *TexRoot[WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_ECOSYSTEM_NUMTHEMES];

		// Initialize GeneralEffect base members in constructor...
		EcosystemEffect(char NewMatType);		// default constructor
		EcosystemEffect(RasterAnimHost *RAHost, char NewMatType);
		EcosystemEffect(RasterAnimHost *RAHost, EffectsLib *Library, EcosystemEffect *Proto, char NewMatType);
		~EcosystemEffect();
		void SetDefaults(void);
		void Copy(EcosystemEffect *CopyTo, EcosystemEffect *CopyFrom);
		double GetRenderMatGradientPos(RenderData *Rend, PolygonData *Poly);
		double GetRenderMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Ecosystem)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_ECOSYSTEM_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_ECOSYSTEM_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_ECOSYSTEM_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_ECOSYSTEM_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

	}; // EcosystemEffect

enum
	{
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX,	// no nodes
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY,	// no nodes
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE,
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH,	// no nodes
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE,
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY,	// no nodes
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME,	// no nodes
	WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY,	// no nodes
	WCS_EFFECTS_WAVESOURCE_NUMANIMPAR
	}; // animated WaveSource params

enum
	{
	WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE,
	WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE,
	WCS_EFFECTS_WAVESOURCE_NUMTEXTURES
	}; // WaveSource textures

class WaveSource: public RasterAnimHost, public RootTextureParent
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_WAVESOURCE_NUMANIMPAR];
		AnimDoubleEnvelope Envelope;
		char Enabled, EnvelopeEnabled, RepeatEnvelopeBefore, RepeatEnvelopeAfter;
		RootTexture *TexRoot[WCS_EFFECTS_WAVESOURCE_NUMTEXTURES];
		WaveSource *Next;

		WaveSource(RasterAnimHost *RAHost);
		~WaveSource();
		void Copy(WaveSource *CopyTo, WaveSource *CopyFrom);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		long GetNumAnimParams(void) {return (WCS_EFFECTS_WAVESOURCE_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_WAVESOURCE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		bool AnimateMaterials(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_WAVESOURCE);};
		long InitImageIDs(long &ImageID);
		double EvalHeight(RenderData *Rend, VertexData *Vert, double RelX, double RelY, double CurTime, double EnvTimeOffset);
		double EvalHeight(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat, double RelX, double RelY, double CurTime, double EnvTimeOffset);
		double EvalSampleHeight(double RelX, double RelY, double CurTime, double EnvTimeOffset);
		double EvalCloudWaveHeight(CloudEffect *Cld, VertexCloud *Vert, TextureData *TexData, double RelX, double RelY, double CurTime);
		double EvalSampleCloudWaveHeight(double RelX, double RelY, double CurTime);
		int InitToRender(void);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil);
		int BuildFileComponentsList(EffectList **Coords);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_WAVESOURCE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_WAVESOURCE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_WAVESOURCE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_WAVESOURCE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_WAVESOURCE);};
		virtual char *GetRAHostTypeString(void) {return ("(Wave Source)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // class WaveSource

enum
	{
	WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE,
	WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE,
	WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE,
	WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE,	// no nodes
	WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY,	// no nodes
	WCS_EFFECTS_WAVE_NUMANIMPAR
	}; // animated WaveEffect params

enum
	{
	WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE,
	WCS_EFFECTS_WAVE_NUMTEXTURES
	}; // WaveEffect textures

enum
	{
	WCS_WAVE_WAVETYPE_AREA,		// waves fill an area and may use gradient profiles to feather
	WCS_WAVE_WAVETYPE_POINT		// waves are replicated at each vertex of a vector, profiles are ignored
	}; // wave effect types

enum
	{
	WCS_WAVE_RANDOMIZETYPE_POISSON,		// waves begin randomly within designated envelope start range
	WCS_WAVE_RANDOMIZETYPE_GAUSS		// waves begin randomly but clumped more towards middle of start range
	}; // point wave randomize types

#define WCS_SEEDOFFSET_WAVE		32		// this is an offset added to the vertex seed values

class WaveEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_WAVE_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		RootTexture *TexRoot[WCS_EFFECTS_WAVE_NUMTEXTURES];
		WaveSource *WaveSources, **WaveSourceArray;
		long WavePreviewSize;
		short WaveSourcesPerVertex, NumSources;
		char WaveEffectType, RandomizeType, RandomizeEnvStart;
		PRNGX Rand;

		// Initialize GeneralEffect base members in constructor...
		WaveEffect();		// default constructor
		WaveEffect(RasterAnimHost *RAHost);
		WaveEffect(RasterAnimHost *RAHost, EffectsLib *Library, WaveEffect *Proto);
		~WaveEffect();
		void SetDefaults(void);
		void Copy(WaveEffect *CopyTo, WaveEffect *CopyFrom);
		WaveSource *AddWave(WaveSource *AddMe);
		WaveSource *AddWave(double NewLat, double NewLon);
		double GetMaxWaveAmp(void);
		double EvalHeight(RenderData *Rend, VertexData *Vert);
		double EvalHeight(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat);
		double EvalSampleHeight(double EarthLatScaleMeters, double CurTime, double Lat, double Lon);
		void SetPosition(double NewLat, double NewLon);
		void SetSourcePosition(WaveSource *SetSource, double NewLat, double NewLon);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		int EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneSampleLine(TextureSampleData *Samp);
		int BuildFileComponentsList(EffectList **Queries, EffectList **Themes, EffectList **Coords);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_WAVE_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_WAVE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual void SetFloating(char NewFloating);
		virtual long InitImageIDs(long &ImageID);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_WAVE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_WAVE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_WAVE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_WAVE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Wave Model)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);

	}; // WaveEffect

class VolumeRayList
	{
	public:
		double RayOnDist, RayOffDist, SegMidXYZ[3], SampleRate, SegStartDist, SegEndDist,
			SegOpticalDepth, SegColor[3];
		VolumeRayList *Next;
		int SegEvaluated;

		VolumeRayList()	
			{RayOnDist = RayOffDist = SegMidXYZ[0] = SegMidXYZ[1] = SegMidXYZ[2] = SampleRate = SegStartDist = SegEndDist =
			SegOpticalDepth = SegColor[0] = SegColor[1] = SegColor[2] = 0.0; 
			Next = NULL; SegEvaluated = 0;};
	}; // class VolumeRayList

class VolumetricSubstance
	{
	public:
		// bounds are stored with low value in 0, high value in 1, global cartesian coords
		double BoxBoundsX[2], BoxBoundsY[2], BoxBoundsZ[2], QuadNormal[6][3];
		VolumetricSubstance *NextSub;
		VolumeRayList *RayList, *ShadowRayList, *CurRayList, *CurShadowRayList;

		VolumetricSubstance();
		~VolumetricSubstance();
		void DeleteRayList(void);
		void DeleteShadowRayList(void);
		void CompareSetBounds(VertexDEM *CurVert);
		void SetQuadNormals(void);
		virtual void ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon);
		virtual int PointSampleVolumetric(VolumeRayList *CurRay, VertexDEM *CurVert, TextureData *TexData, double &Shading)	
			{
			CurRay->SegOpticalDepth = FLT_MAX;
			CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;
			return (0);
			};
		virtual VolumeRayList *BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3]);
		virtual unsigned long GetShadowFlags(void)	{return (0);};
		virtual double GetShadowIntenstiy(void)	{return (0.0);};
		virtual int GetBeforeReflections(void)	{return (1);};
		virtual int GetShadowsOnly(void)	{return (0);};
		virtual int GetCastVolumetricShadows(void)	{return (0);};
		virtual double GetBacklightPct(void)	{return (0.0);};
		virtual double GetBacklightExp(void)	{return (1.0);};
		virtual double GetBacklightColor(int Channel)	{return (0.0);};
		virtual double GetAltBacklightColorPct(void)	{return (0.0);};
		virtual int GetRenderVolumetric(void)	{return (1);};

	}; // class VolumetricSubstance

class CloudLayerData
	{
	public:
		double Coverage, Density, Shading, Elev, Dist, RGB[3];
		CloudEffect *Cloud;
		long LayerNum;
		char Above;

		CloudLayerData();
	}; // class CloudLayerData

enum
	{
	WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT,
	WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON,
	WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH,
	WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT,
	WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV,
	WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE,
	WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY,
	WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS,
	WCS_EFFECTS_CLOUD_ANIMPAR_FALLOFF,
	WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING,
	WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS,
	WCS_EFFECTS_CLOUD_ANIMPAR_OPTICALDEPTH,
	WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE,
	WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE,
	WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT,
	WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTEXP,
	WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT,
	WCS_EFFECTS_CLOUD_NUMANIMPAR
	}; // animated CloudEffect params

enum
	{
	WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE,
	WCS_EFFECTS_CLOUD_NUMTEXTURES
	}; // CloudEffect textures

enum
	{
	WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP,
	WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC,
	WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION
	}; // CloudEffect shadow style

// Cloud types used in cloud defaults
#define WCS_EFFECTS_CLOUDTYPE_CIRRUS		0
#define WCS_EFFECTS_CLOUDTYPE_STRATUS		1
#define WCS_EFFECTS_CLOUDTYPE_NIMBUS		2
#define WCS_EFFECTS_CLOUDTYPE_CUMULUS		3
#define WCS_EFFECTS_CLOUDTYPE_CUSTOM		4

#define WCS_EFFECTS_CLOUD_LAYERTHICKNESS	50.0

class CloudEffect: public GeneralEffect, public VolumetricSubstance
	{
	// color gradient first color is bottom
	public:
		double MetersPerDegLat, MetersPerDegLon, RefLat, RefLon, LowLon, HighLon, LowLat, HighLat,
			LowElev, HighElev, ElevRange, LatStep, LonStep, MaxAmp, MinAmp, AmpRange, RadiusInside, RadiusOutside,	// for rendering
			CompleteBacklightColor[3];
		RootTexture *TexRoot[WCS_EFFECTS_CLOUD_NUMTEXTURES];
		VertexCloud *Vertices;
		WaveSource *WaveSources;
		AnimColorGradient ColorGrad;
		AnimColorTime BacklightColor;
		AnimDoubleTime AnimPar[WCS_EFFECTS_CLOUD_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		AnimDoubleVerticalProfile CovgProf, DensityProf, ShadeProf;
		long Rows, Cols, NumLayers, NumSources, PreviewSize, WavePreviewSize, CenterRow, CenterCol;
		unsigned long ShadowFlags;
		char Feather, PixelTexturesExist, CovgTextureExists, CloudType, CastShadows, CastShadowStyle, ShadowsOnly, 
			Volumetric, ReceiveShadowsTerrain, ReceiveShadowsFoliage, ReceiveShadows3DObject, ReceiveShadowsCloudSM, 
			ReceiveShadowsVolumetric, ReceiveShadowsMisc, VolumeBeforeRefl, VolumeSub, RenderShadowMap, RenderVolumetric,
			RenderPlanar, CastVolumeShadows;
		short ShadowMapWidth, UseMapFile, RegenMapFile;
		PRNGX Rand;

		// Initialize GeneralEffect base members in constructor...
		CloudEffect();		// default constructor
		CloudEffect(RasterAnimHost *RAHost);
		CloudEffect(RasterAnimHost *RAHost, EffectsLib *Library, CloudEffect *Proto);
		~CloudEffect();
		void SetDefaults(void);
		void Copy(CloudEffect *CopyTo, CloudEffect *CopyFrom);
		void DeleteLayerList(void);
		bool ConfirmType(void);
		int SetCloudType(void);
		int AnimateCloudShadows(void);
		int EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneSampleLine(TextureSampleData *Samp);
		int EvalWaveSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneWaveSampleLine(TextureSampleData *Samp);
		int AllocVertices(void);
		void RemoveVertices(void);
		int EvaluateDensity(PixelData *Pix, CloudLayerData *LayerData, double VertexIntensity);
		void EvalVertices(double CurTime, double MetersPerDegLat);
		double GetMaxWaveAmp(void);
		WaveSource *AddWave(WaveSource *AddMe);
		WaveSource *AddWave(double NewLat, double NewLon);
		double EvalWaveHeight(VertexCloud *Vert, TextureData *TexData, double CurTime);
		double EvalSampleWaveHeight(double CurTime, double RelX, double RelY);
		TextureData *TransferTextureData(VertexDEM *Vert, TextureData *TexData);
		int ProjectBounds(Camera *Cam, RenderData *Rend, double *LimitsX, double *LimitsY, double *LimitsZ);
		void SetBounds(double LatRange[2], double LonRange[2]);
		void SetSourcePosition(WaveSource *SetSource, double NewLat, double NewLon);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		int BuildFileComponentsList(EffectList **Coords);

		// in RenderSky.cpp
		//void SetVertexDensity(VertexCloud *Vert, CloudLayerData *LayerData);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_CLOUD_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_CLOUD_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual void SetFloating(char NewFloating);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2]);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_CLOUD_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_CLOUD_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_CLOUD_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_CLOUD_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Cloud Model)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual int GetDeletable(RasterAnimHost *Test);

		// inherited from VolumetricSubstance
		virtual void ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon);
		virtual VolumeRayList *BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3]);
		virtual int PointSampleVolumetric(VolumeRayList *CurRay, VertexDEM *CurVert, TextureData *TexData, double &Shading);
		virtual unsigned long GetShadowFlags(void)	{return (ShadowFlags);};
		virtual double GetShadowIntenstiy(void)	{return (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS].CurValue);};
		virtual int GetBeforeReflections(void)	{return (VolumeBeforeRefl);};
		virtual int GetShadowsOnly(void)	{return (ShadowsOnly);};
		virtual int GetCastVolumetricShadows(void)	{return (CastVolumeShadows);};
		virtual double GetBacklightPct(void)	{return (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT].CurValue);};
		virtual double GetBacklightExp(void)	{return (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTEXP].CurValue);};
		virtual double GetAltBacklightColorPct(void)	{return (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT].CurValue);};
		virtual double GetBacklightColor(int Channel)	{return (CompleteBacklightColor[Channel]);};
		virtual int GetRenderVolumetric(void)	{return (RenderVolumetric);};

	}; // CloudEffect

enum
	{
	WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT,
	WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALECOGRAD,
	WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT,
	WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR
	}; // animated EnvironmentEffect params

#define WCS_SEEDOFFSET_ENVIRONMENT		19		// this is an offset added to the vertex seed values

class EnvironmentEffect: public GeneralEffect
	{
	public:
		double MinPixelSize, FoliageBlending, GlobalGradient;	// calc for rendering
		AnimDoubleTime AnimPar[WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		EffectList *Ecosystems;
		short FoliageMinSize;
		char GlobalGradientsEnabled;

		// Initialize GeneralEffect base members in constructor...
		EnvironmentEffect();		// default constructor
		EnvironmentEffect(RasterAnimHost *RAHost);
		EnvironmentEffect(RasterAnimHost *RAHost, EffectsLib *Library, EnvironmentEffect *Proto);
		~EnvironmentEffect();
		void SetDefaults(void);
		void Copy(EnvironmentEffect *CopyTo, EnvironmentEffect *CopyFrom);
		EffectList *AddEcosystem(GeneralEffect *AddMe);
		int GrabAllEcosystems(void);
		int SortEcosystems(void);
		EcosystemEffect *FindEco(EcosystemEffect *FindMe);
		int EvalEcosystem(PolygonData *Poly);
		bool EvalEcosystem(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow);
		int BuildFileComponentsList(EffectList **Ecosys, EffectList **Material3Ds, 
			EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		
		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual double GetMaxProfileDistance(void);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Environment)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // EnvironmentEffect

#define WCS_SEEDOFFSET_CMAP		213		// this is an offset added to the vertex seed values

#define WCS_CMAP_EVAL_BYPOLYGON	0
#define WCS_CMAP_EVAL_BYPIXEL	1

class CmapEffect: public GeneralEffect
	{
	public:
		double North, South, West, East, NSLow, NSHigh, WELow, WEHigh, DegLatPerCell, DegLonPerCell;	// for use by renderer
		EffectList *Ecosystems;
		GroundEffect *ByPixelGround;	// for use by renderer
		Raster *Rast;	// for use by renderer
		CoordSys *Coords;	// for use by renderer
		RasterShell *Img;
		//GeoRegister GeoReg;
		short RandomizeEdges, LuminousColors, Orientation, EvalByPixel;

		// Initialize GeneralEffect base members in constructor...
		CmapEffect();		// default constructor
		CmapEffect(RasterAnimHost *RAHost);
		CmapEffect(RasterAnimHost *RAHost, EffectsLib *Library, CmapEffect *Proto);
		~CmapEffect();
		void SetDefaults(void);
		void Copy(CmapEffect *CopyTo, CmapEffect *CopyFrom);
		int SetRaster(Raster *NewRast);
		EffectList *AddEcosystem(GeneralEffect *AddMe);
		int SortEcosystems(void);
		int GrabAllEcosystems(void);
		int SnapToDEMBounds(void);
		void CleanupFromRender(void);	// deletes ByPixelGround
		void SetBounds(double LatRange[2], double LonRange[2]);
		int BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, 
			EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		int CreateImageObject(Database *DBHost, ImageLib *ImageHost);
		void PlotVectors(Database *DBHost, Raster *Rast, double CellSizeNS, double CellSizeWE);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int SetToTime(double Time);
		virtual short AnimateShadows(void);
		virtual int GetRAHostAnimated(void);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual void SetFloating(char NewFloating);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Color Map)");};
		char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // CmapEffect

/* retired - used in WCS 4 files, obsolete in WCS 5 and VNS 1, removed 2/22/06
enum
	{
	WCS_EFFECTS_ILLUMINATION_ANIMPAR_ILLUM,
	WCS_EFFECTS_ILLUMINATION_NUMANIMPAR
	}; // animated IlluminationEffect params

enum
	{
	WCS_EFFECTS_ILLUMINATION_TEXTURE_COLOR,
	WCS_EFFECTS_ILLUMINATION_TEXTURE_INTENSITY,
	WCS_EFFECTS_ILLUMINATION_NUMTEXTURES
	}; // IlluminationEffect textures

class IlluminationEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_ILLUMINATION_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		AnimColorTime IllumColor;
		RootTexture *TexRoot[WCS_EFFECTS_ILLUMINATION_NUMTEXTURES];

		// Initialize GeneralEffect base members in constructor...
		IlluminationEffect();		// default constructor
		IlluminationEffect(RasterAnimHost *RAHost);
		IlluminationEffect(RasterAnimHost *RAHost, EffectsLib *Library, IlluminationEffect *Proto);
		~IlluminationEffect();
		void SetDefaults(void);
		void Copy(IlluminationEffect *CopyTo, IlluminationEffect *CopyFrom);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_ILLUMINATION_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_ILLUMINATION_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_ILLUMINATION_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_ILLUMINATION_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_ILLUMINATION_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_ILLUMINATION_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Illumination)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);

	}; // IlluminationEffect
*/

enum
	{
	WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV,
	WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS,
	WCS_EFFECTS_RASTERTA_NUMANIMPAR
	}; // animated RasterTerraffectorEffect params

enum
	{
	WCS_EFFECTS_RASTERTA_TEXTURE_ELEV,
	WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS,
	WCS_EFFECTS_RASTERTA_NUMTEXTURES
	}; // RasterTerraffectorEffect textures

enum
	{
	WCS_EFFECTS_RASTERTA_THEME_ELEV,
	WCS_EFFECTS_RASTERTA_NUMTHEMES
	}; // RasterTerraffectorEffect themes

#define WCS_SEEDOFFSET_RASTERTA		61		// this is an offset added to the vertex seed values

enum
	{
	WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC,
	WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE,
	WCS_EFFECTS_RASTERTA_COMPARETYPE_DECREASE
	}; //  compare types

class RasterTerraffectorEffect: public GeneralEffect
	{
	public:
		double Elev;	// calculated by renderer
		RootTexture *TexRoot[WCS_EFFECTS_RASTERTA_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_RASTERTA_NUMTHEMES];
		AnimDoubleTime AnimPar[WCS_EFFECTS_RASTERTA_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		short EvalOrder;
		char CompareType, ApplyToOrigElev;

		// Initialize GeneralEffect base members in constructor...
		RasterTerraffectorEffect();		// default constructor
		RasterTerraffectorEffect(RasterAnimHost *RAHost);
		RasterTerraffectorEffect(RasterAnimHost *RAHost, EffectsLib *Library, RasterTerraffectorEffect *Proto);
		~RasterTerraffectorEffect();
		void SetDefaults(void);
		void Copy(RasterTerraffectorEffect *CopyTo, RasterTerraffectorEffect *CopyFrom);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_RASTERTA_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_RASTERTA_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual double GetMaxProfileDistance(void);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Area Terraffector)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_RASTERTA_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_RASTERTA_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_RASTERTA_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_RASTERTA_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_RASTERTA_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_RASTERTA_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_RASTERTA_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_RASTERTA_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

	}; // RasterTerraffectorEffect

enum
	{
	WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY,
	WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET,
	WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT,	// no nodes
	WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD,	// no nodes
	WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB,	// no nodes
	WCS_EFFECTS_SHADOW_NUMANIMPAR
	}; // animated ShadowEffect params

enum
	{
	WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY,
	WCS_EFFECTS_SHADOW_NUMTEXTURES
	}; // ShadowEffect textures

class ShadowEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_SHADOW_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		RootTexture *TexRoot[WCS_EFFECTS_SHADOW_NUMTEXTURES];
		unsigned long ShadowFlags;
		short ShadowMapWidth, UseMapFile, RegenMapFile;
		char CastShadows, ReceiveShadowsTerrain, ReceiveShadowsFoliage, ReceiveShadows3DObject, ReceiveShadowsCloudSM, 
			ReceiveShadowsVolumetric;

		// Initialize GeneralEffect base members in constructor...
		ShadowEffect();		// default constructor
		ShadowEffect(RasterAnimHost *RAHost);
		ShadowEffect(RasterAnimHost *RAHost, EffectsLib *Library, ShadowEffect *Proto);
		~ShadowEffect();
		void SetDefaults(void);
		void Copy(ShadowEffect *CopyTo, ShadowEffect *CopyFrom);
		int ProjectBounds(Camera *Cam, RenderData *Rend, Joe *CurJoe, 
			double *LatBounds, double *LonBounds, double *ElevBounds,
			double *XLimits, double *YLimits, double *ZLimits);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_SHADOW_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_SHADOW_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Shadow)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_SHADOW_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_SHADOW_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_SHADOW_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_SHADOW_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

	}; // ShadowEffect

enum
	{
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE,
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE,
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS,
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING,
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS,
	WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY,
	WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR
	}; // animated TerraffectorEffect params

enum
	{
	WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY,
	WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES
	}; // TerraffectorEffect textures

#define WCS_SEEDOFFSET_TERRAFFECTOR		117		// this is an offset added to the vertex seed values

enum
	{
	WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC,
	WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE,
	WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_DECREASE
	}; //  compare types

class TerraffectorEffect: public GeneralEffect
	{
	public:
		double MaxSectionDist, MinSlope, MaxSlope, MinSlopeDegrees, MaxSlopeDegrees;	// MaxSectionDist is farthest section graph node for rendering efficiency
		AnimDoubleTime AnimPar[WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR];
		AnimDoubleCrossSection ADSection;
		EcosystemEffect *SlopeEco;
		RootTexture *TexRoot[WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES];
		short SlopePriority, EvalOrder;
		char CompareType, ApplyToOrigElev;
		char InterCompareMethod, Splined;

		// Initialize GeneralEffect base members in constructor...
		TerraffectorEffect();		// default constructor
		TerraffectorEffect(RasterAnimHost *RAHost);
		TerraffectorEffect(RasterAnimHost *RAHost, EffectsLib *Library, TerraffectorEffect *Proto);
		~TerraffectorEffect();
		void SetDefaults(void);
		void Copy(TerraffectorEffect *CopyTo, TerraffectorEffect *CopyFrom);
		int SetSlopeEco(EcosystemEffect *NewEco);
		int BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, 
			EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int FindnRemoveEcosystems(EcosystemEffect *RemoveMe);
		void *GetNextSegmentWidth(void *PlaceHolder, double &SegWidth)	{return (ADSection.GetNextSegmentWidth(PlaceHolder, SegWidth));};
		double GetRadiusWidth(void);
		unsigned long GetNumSegments(void);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SetToTime(double Time);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Terraffector)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int GetDeletable(RasterAnimHost *Test);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

	}; // TerraffectorEffect

enum
	{
	WCS_EFFECTS_STREAM_ANIMPAR_WWSLOPE_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_WWSLOPE_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_WATERDEPTH_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_WATERDEPTH_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_WWROUGHNESS_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_WWROUGHNESS_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_WWCUTOFF_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_WWCUTOFF_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_REFLECTION_MIN,
	WCS_EFFECTS_STREAM_ANIMPAR_REFLECTION_MAX,
	WCS_EFFECTS_STREAM_ANIMPAR_DEEPWATERDEPTH,
	WCS_EFFECTS_STREAM_ANIMPAR_WWTRANSHT,
	WCS_EFFECTS_STREAM_V4NUMANIMPAR
	}; // V4 animated StreamEffect params

enum
	{
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR,
	WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER,
	WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER,
	WCS_EFFECTS_STREAM_ANIMPAR_RADIUS,
	WCS_EFFECTS_STREAM_NUMANIMPAR
	}; // animated StreamEffect params

enum
	{
	WCS_EFFECTS_STREAM_TEXTURE_BEACHHTVAR,
	WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER,
	WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER,
	WCS_EFFECTS_STREAM_NUMTEXTURES
	}; // Stream textures

class StreamEffect: public GeneralEffect
	{
	public:
		double MaxBeachLevel, MaxLevel;	// init for rendering
		AnimDoubleTime AnimPar[WCS_EFFECTS_STREAM_NUMANIMPAR];
		AnimMaterialGradient BeachMat, WaterMat;
		RootTexture *TexRoot[WCS_EFFECTS_STREAM_NUMTEXTURES];
		char PixelTexturesExist, Splined;

		// Initialize GeneralEffect base members in constructor...
		StreamEffect();		// default constructor
		StreamEffect(RasterAnimHost *RAHost);
		StreamEffect(RasterAnimHost *RAHost, EffectsLib *Library, StreamEffect *Proto);
		~StreamEffect();
		void SetDefaults(void);
		void Copy(StreamEffect *CopyTo, StreamEffect *CopyFrom);
		double GetRenderWaterMatGradientPos(RenderData *Rend, VertexData *Vert);
		double GetRenderWaterMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat);
		double GetRenderWaterMatGradientPos(RenderData *Rend, PolygonData *Poly);
		double GetRenderBeachMatGradientPos(RenderData *Rend, PolygonData *Poly);
		double GetRenderBeachMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int IsThereOpticallyTransparentMaterial(void)	{return (WaterMat.IsThereOpticallyTransparentMaterial());};
		void GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod)	{WaterMat.GetWaterDepthAndWaveRange(MaximumMod, MinimumMod);};

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_STREAM_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_STREAM_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual int SetToTime(double Time);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Stream)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_STREAM_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_STREAM_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_STREAM_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_STREAM_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

	}; // StreamEffect

enum
	{
	WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV,
	WCS_EFFECTS_FOLIAGE_NUMANIMPAR
	}; // animated FoliageEffect params

class FoliageEffect: public GeneralEffect, public SXQueryItem
	{
	public:
		double Elev;	// for use in rendering
		AnimDoubleTime AnimPar[WCS_EFFECTS_FOLIAGE_NUMANIMPAR];
		Ecotype Ecotp;
		PathAndFile FoliageFile;
		RealtimeFoliageIndex FoliageFileIndex;
		char PreviewEnabled, UseFoliageFile;

		// Initialize GeneralEffect base members in constructor...
		FoliageEffect();		// default constructor
		FoliageEffect(RasterAnimHost *RAHost);
		FoliageEffect(RasterAnimHost *RAHost, EffectsLib *Library, FoliageEffect *Proto);
		~FoliageEffect();
		void SetDefaults(void);
		void Copy(FoliageEffect *CopyTo, FoliageEffect *CopyFrom);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, void *CurVtx);
		#ifdef WCS_USE_OLD_FOLIAGE
		int SelectImageOrObject(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly,
			FoliagePreviewData *PointData);
		#endif // WCS_USE_OLD_FOLIAGE
		int SelectForestryImageOrObject(RenderData *Rend, PolygonData *Poly, FoliagePreviewData *PointData);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_FOLIAGE_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_FOLIAGE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int SetToTime(double Time);
		virtual int GetRAHostAnimated(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Foliage Effect)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // FoliageEffect

enum
	{
	WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_LON,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS,
	WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS,
	WCS_EFFECTS_OBJECT3D_NUMANIMPAR
	}; // animated Object3DEffect params

enum
	{
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY,
	WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ,
	WCS_EFFECTS_OBJECT3D_NUMTEXTURES
	}; // Object3DEffect textures

enum
	{
	WCS_EFFECTS_OBJECT3D_THEME_ROTATIONX,
	WCS_EFFECTS_OBJECT3D_THEME_ROTATIONY,
	WCS_EFFECTS_OBJECT3D_THEME_ROTATIONZ,
	WCS_EFFECTS_OBJECT3D_THEME_SCALINGX,
	WCS_EFFECTS_OBJECT3D_THEME_SCALINGY,
	WCS_EFFECTS_OBJECT3D_THEME_SCALINGZ,
	WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONX,
	WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONY,
	WCS_EFFECTS_OBJECT3D_THEME_TRANSLATIONZ,
	WCS_EFFECTS_OBJECT3D_NUMTHEMES
	}; // Object3DEffect themes

enum
	{
	WCS_EFFECTS_OBJECT3D_ALIGN_X,
	WCS_EFFECTS_OBJECT3D_ALIGN_Y,
	WCS_EFFECTS_OBJECT3D_ALIGN_Z
	}; // alignment axes

enum
	{
	WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR,
	WCS_EFFECTS_OBJECT3D_ALIGNVERT_TERRAIN
	}; // alignment axes

enum
	{
	WCS_EFFECTS_OBJECT3D_RANDOMIZE_SCALE,
	WCS_EFFECTS_OBJECT3D_RANDOMIZE_ROTATE,
	WCS_EFFECTS_OBJECT3D_RANDOMIZE_POSITION
	}; // randomize axes

enum
	{
	WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE,
	WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJROTATE,
	WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJPOSITION,
	WCS_EFFECTS_OBJECT3D_SHOWDETAIL_VERTPOSITION
	}; // view options

enum
	{
	WCS_EFFECTS_OBJECT3D_OPTIMIZE_PEROBJECT,
	WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPASS,
	WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPIXEL
	}; // fragment optimization


class Object3DEffect: public GeneralEffect, public SXQueryItem
	{
	public:
		double Elev, ObjectBounds[6];	// 0 = hi x, 1 = lo x, 2 = hi y, 3 = lo y, 4 = hi z, 5 = lo z
		AnimDoubleTime AnimPar[WCS_EFFECTS_OBJECT3D_NUMANIMPAR];
		char FilePath[256], FileName[64], ShortName[24], AAPasses, DrawEnabled, ShadowsOnly,
			CastShadows, ReceiveShadowsTerrain, ReceiveShadowsFoliage, ReceiveShadows3DObject, ReceiveShadowsCloudSM, 
			ReceiveShadowsVolumetric, Units, CalcNormals, VertexColorsAvailable, VertexUVWAvailable, 
			AlignHeading, AlignVertical, HeadingAxis, VerticalAxis, AlignVertVec, ReverseHeading, RandomizeObj[3], 
			RandomizeVert, ShowDetail, Isometric, AlignSpecialVec, GeographicInstance, FragmentOptimize, lockScales;
		unsigned long ShadowFlags;
		short ShadowMapWidth, UseMapFile, RegenMapFile;
		long NumVertices, NumPolys, NumMaterials, NumUVWMaps, NumCPVMaps;
		ObjectMaterialEntry *NameTable;
		ObjectPerVertexMap *UVWTable, *CPVTable;
		Vertex3D *Vertices;
		Polygon3D *Polygons;
		Joe *AlignVec;
		RootTexture *TexRoot[WCS_EFFECTS_OBJECT3D_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_OBJECT3D_NUMTHEMES];
		Matx4x4 WorldMatx, InvWorldMatx;
		Matx3x3 WorldNormalMatx;
		PRNGX Rand;
		long *VertRefBlock, *PolyRefBlock; // used for mass-allocation/deallocation of blocks of VertRefs and PolyRefs

		// Initialize GeneralEffect base members in constructor...
		Object3DEffect();		// default constructor
		Object3DEffect(RasterAnimHost *RAHost);
		Object3DEffect(RasterAnimHost *RAHost, EffectsLib *Library, Object3DEffect *Proto);
		~Object3DEffect();
		void SetDefaults(void);
		void Copy(Object3DEffect *CopyTo, Object3DEffect *CopyFrom);
		char *GetNextName(char *Current, char *MaxPtr);
		short AnimateShadow3D(void);
		void ApplyUnitScaling(void);
		int GetObjectBounds(void);
		void CopyCoordinatesToXYZ(void);
		int TransformToWCSDefGeographic(RenderData *Rend, Object3DInstance *CurInstance);
		int Transform(Vertex3D *LocalVertices, long NumLocalVertices, RenderData *Rend, PolygonData *Poly, VertexDEM *BaseVtx, double ExtraRotation[3], double Height);
		void CalcTransformParams(RenderData *Rend, PolygonData *Poly, 
			VertexDEM *BaseVtx, double ExtraRotation[3], double Height, 
			Point3d OutRotation, Point4d OutQuaternion, Point3d OutTranslation, Point3d OutScale);
		void TransformNormals(void);
		int ProjectBounds(Camera *Cam, RenderData *Rend, VertexDEM *BaseVtx, PolygonData *Poly, double *LimitsX, double *LimitsY, double *LimitsZ);
		int FindCurrentCartesian(RenderData *Rend, VertexDEM *Vert, char CenterOnOrigin);
		int FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, VectorPoint *CurVtx);
		void MaterialNameChanging(char *OldName, char *NewName);
		int MatchMaterialName(char *MatchName);
		ObjectPerVertexMap *MatchVertexMap(int MapType, char *MatchName, unsigned char &MapNumber);
		ObjectPerVertexMap *GetVertexMapNumber(int MapType, char *MatchName, unsigned char &MapNumber);
		ObjectPerVertexMap *AllocUVWMaps(long NumMaps) {NumUVWMaps = NumMaps; return(UVWTable = new ObjectPerVertexMap[NumMaps]);};
		ObjectPerVertexMap *AllocCPVMaps(long NumMaps) {NumCPVMaps = NumMaps; return(CPVTable = new ObjectPerVertexMap[NumMaps]);};
		void FreeUVWMaps(void);
		void FreeCPVMaps(void);
		void FreeNameTable(void);
		long *AllocVertRef(long NumAllocVerts)	{return(VertRefBlock = new long [NumAllocVerts]);};
		int GetMaterialBoundsXYZ(char *MatchName, double &HighX, double &LowX, double &HighY, double &LowY, double &HighZ, double &LowZ);
		void SetAlignBias(double NewBias)	{AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].SetCurValue(NewBias);};
		int SetAlignVec(Joe *NewVec);
		int VectorNotInJoeList(Joe *TestVec);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		void VectorAdded(void);
 		int TestOriginOutOfBounds(void);
 		int OutOfBoundsMessage(int Condition);
 		int RecenterObject(void);
		void FreeDynamicStuff(void);
		int ObjectCartesianLegal(RootTexture *TexTest);
		void CompleteInstanceInfo(RenderData *Rend, Object3DInstance *CurInstance, PolygonData *Poly, double RefLat, double RefLon, double RefElev,
			double PointLat, double PointLon, double PointElev, double ExtraRotation[3], double Height);
		void CompleteInstanceInfo(RenderData *Rend, Object3DInstance *CurInstance, RealtimeFoliageIndex *Index, 
			RealtimeFoliageData *FolData, FoliagePreviewData *PointData);
		long CountMaterialPolygons(long MatNum);
		long Count3SidedPolygons(void);
		int AllMaterialsLuminous(void);

		void FreePolygons(void); // understand VertRefBlock allocation
		void FreeVertices(void); // understand PolyRefBlock allocation

		// file functions - see Effect3DObjectIO.cpp for more detailed explanation of rationale

		// Saving
			// .proj
				// A, B, D  Save Project
				virtual unsigned long Save(FILE *ffile);
					// does not save geometry
			// .w3o
				// B, C, D  Select new alien file, Apply scaling, Load to render alien file
				int SaveObjectW3O(void);
					// opens file, writes project version #, byte order, "Object3D" chunk tag
			// .w3d
				// A, B, C, D  Save component
				// RasterAnimHost::SaveFilePrep()
				// A, B, C, D  Apply scaling
				int SaveObjectW3D(void);
					// calls SaveFilePrep()

		// Loading
			// .proj
				// A, B, D  Load project
				virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
					// does not load geometry
			// .w3o
				// B, C, D  Select new file, Apply scaling
				// B, C     Load to render
				int LoadObjectW3O(short LoadMaterials);
					// LoadMaterials instructs whether or not to load part D
			// .w3d
				// A, B, C, D  Load component
				// RasterAnimHost::LoadFilePrep()
				// A, B, C, D  Apply scaling
				// B, C, D     Select new file
				// B, C        Load to render
				int LoadObjectW3D(short LoadParameters, short LoadMaterials, short Queries);
					// LoadParameters instructs whether or not to load part A
					// LoadMaterials instructs whether or not to load part D

		int OpenInputFileRequest(void);
		int OpenInputFile(char *Ext, short LoadMaterials, short LoadParameters, short Queries);

		unsigned long ObjectData_Save(FILE *ffile);	// called from SaveObjectW3O()
			// writes 3d object version #, byte order, WCS_EFFECTSPOINT_OBJECT3D chunk tag
		unsigned long SaveData(FILE *ffile);
			// writes num vertices, polys & materials, writes each vertex, polygon & material name

		// corresponding functions to Save f()'s with similar names
		unsigned long ObjectData_Load(FILE *ffile, ULONG ReadSize, short LoadMaterials);	// called from LoadObjectW3O()
		unsigned long LoadData(FILE *ffile, unsigned long ReadSize, short ByteFlip, short LoadMaterials); // called from ObjectData_Load()

		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);	// called from SetRAHostProperties() to save w3d file
			// compiles a list of associated components and writes complete object file including primitives and material names
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);	// called from SetRAHostProperties() to load w3d component file
			// should load the entire object description
		unsigned long SaveWithGeometry(FILE *ffile);	// for writing to w3d file
		unsigned long LoadWithGeometry(FILE *ffile, unsigned long ReadSize, short ByteFlip, short LoadMaterials, short LoadParameters);	// for reading from w3d file


		// supplementary file-related functions
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		short ReadLWObjectFile(char *filename, short LoadIt);
		void ProcessLWMaterial(char *NamePool, long ChunkSize, ObjectMaterialEntry *Names, MaterialAttributes *Mat);
		short Read3dsObjectFile(char *filename, short LoadObj);
		void Process3dsMaterial(MaterialAttributes *Mat, material3ds *Material3ds, char *ObjectPath);
		short ReadDXFObjectFile(char *filename, short LoadObj);
		short ProcessDXFInput(struct DXFImportPolygon *Polygons, VectorPoint *Points, unsigned long NumPolys, unsigned long NumPts, short CheckRedundancy);
		short ReadWF_OBJFile(char *filename, short LoadIt);
		short ProcessOBJMaterialFile(FILE *MTLFile, ObjectMaterialEntry *Names, MaterialAttributes *Mat, LWTagTable *TT, char *FilePath = NULL);

		// inherited from GeneralEffect
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_OBJECT3D_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_OBJECT3D_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(3D Object)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHostQuery(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual int ComponentOverwriteOK(RasterAnimHostProperties *Prop);
		virtual char *OKRemoveRaster(void);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual void HeresYourNewFilePathIfYouCare(char *NewFullPath);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_OBJECT3D_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_OBJECT3D_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_OBJECT3D_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_OBJECT3D_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_OBJECT3D_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_OBJECT3D_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_OBJECT3D_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_OBJECT3D_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};
	}; // Object3DEffect

enum
	{
	WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE,
	WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY,
	WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE,
	WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE,
	WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS,
	WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR,
	WCS_EFFECTS_CELESTIAL_NUMANIMPAR
	}; // animated CelestialEffect params

class CelestialEffect: public GeneralEffect
	{

	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_CELESTIAL_NUMANIMPAR];
		AnimColorTime Color;
		short ShowPhase, ShowHalo;
		RasterShell *Img;
		EffectList *Lights;

		// Initialize GeneralEffect base members in constructor...
		CelestialEffect();		// default constructor
		CelestialEffect(RasterAnimHost *RAHost);
		CelestialEffect(RasterAnimHost *RAHost, EffectsLib *Library, CelestialEffect *Proto);
		~CelestialEffect();
		void SetDefaults(void);
		void Copy(CelestialEffect *CopyTo, CelestialEffect *CopyFrom);
		int SetRaster(Raster *NewRast);
		EffectList *AddLight(GeneralEffect *AddMe);
		int BuildFileComponentsList(EffectList **LightList);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual void SetDistance(double NewDist) {AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].SetValue(NewDist);};
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_CELESTIAL_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_CELESTIAL_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual int SetToTime(double Time);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual void SetFloating(char NewFloating);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Celestial Object)");};
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int GetDeletable(RasterAnimHost *Test);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);

	}; // CelestialEffect

enum
	{
	WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY,
	WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR,
	WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY,
	WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE,
	WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT,
	WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP,
	WCS_EFFECTS_STARFIELD_NUMANIMPAR
	}; // animated StarFieldEffect params

class StarFieldEffect: public GeneralEffect
	{

	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_STARFIELD_NUMANIMPAR];
		AnimColorGradient ColorGrad;
		short RandomField;
		long RandomSeed;
		RasterShell *Img;
		PRNGX Rand;

		// Initialize GeneralEffect base members in constructor...
		StarFieldEffect();		// default constructor
		StarFieldEffect(RasterAnimHost *RAHost);
		StarFieldEffect(RasterAnimHost *RAHost, EffectsLib *Library, StarFieldEffect *Proto);
		~StarFieldEffect();
		void SetDefaults(void);
		void Copy(StarFieldEffect *CopyTo, StarFieldEffect *CopyFrom);
		int SetRaster(Raster *NewRast);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_STARFIELD_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_STARFIELD_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual int SetToTime(double Time);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Starfield)");};
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // StarFieldEffect

enum
	{
	WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS,
	WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION,
	WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG,
	WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM,
	WCS_EFFECTS_PLANETOPT_NUMANIMPAR
	}; // animated PlanetOpt params

class PlanetOpt: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_PLANETOPT_NUMANIMPAR];
		char EcoExageration;
		CoordSys *Coords;
		
		// Initialize GeneralEffect base members in constructor...
		PlanetOpt();		// default constructor
		PlanetOpt(RasterAnimHost *RAHost);
		PlanetOpt(RasterAnimHost *RAHost, EffectsLib *Library, PlanetOpt *Proto);
		~PlanetOpt();
		void SetDefaults(void);
		void Copy(PlanetOpt *CopyTo, PlanetOpt *CopyFrom);
		int SetCoords(CoordSys *NewCoords);
		int SetExportCoords(CoordSys *NewCoords, Database *ExportDB, Project *ExportProj);
		int BuildFileComponentsList(EffectList **CoordSystems);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_PLANETOPT_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_PLANETOPT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual long InitImageIDs(long &ImageID);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Planet Options)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual char GetRAHostDropOK(long DropType);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // PlanetOpt

enum
	{
	WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT,
	WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR,
	WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR
	}; // animated TerrainParamEffect params

enum
	{
	WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH,
	WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT,
	WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR,
	WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES
	}; // TerrainParamEffect textures

enum
	{
	WCS_FRACTALMETHOD_VARIABLE,
	WCS_FRACTALMETHOD_CONSTANT,
	WCS_FRACTALMETHOD_DEPTHMAPS
	};

#define WCS_SEEDOFFSET_TERRAINPARAM		308		// this is an offset added to the vertex seed values

class TerrainParamEffect: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR];
		AnimDoubleProfile ADProf;
		RootTexture *TexRoot[WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES];
		char FractalDepth, PhongShading;

		// Initialize GeneralEffect base members in constructor...
		TerrainParamEffect();		// default constructor
		TerrainParamEffect(RasterAnimHost *RAHost);
		~TerrainParamEffect();
		TerrainParamEffect(RasterAnimHost *RAHost, EffectsLib *Library, TerrainParamEffect *Proto);
		void SetDefaults(void);
		void Copy(TerrainParamEffect *CopyTo, TerrainParamEffect *CopyFrom);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual double GetMaxProfileDistance(void);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Terrain Parameters)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);

	}; // TerrainParamEffect

#define WCS_SEEDOFFSET_GROUND		162		// this is an offset added to the vertex seed values

class GroundEffect: public EcosystemEffect
	{
	public:

		// Initialize GeneralEffect base members in constructor...
		GroundEffect();		// default constructor
		GroundEffect(RasterAnimHost *RAHost);
		GroundEffect(RasterAnimHost *RAHost, EffectsLib *Library, GroundEffect *Proto);
		~GroundEffect();
		void SetDefaults(void);
		void Copy(GroundEffect *CopyTo, GroundEffect *CopyFrom);

		// inherited from GeneralEffect
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Ground Effect)");};
		virtual char *OKRemoveRaster(void);

	}; // GroundEffect

enum
	{
	WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING,
	WCS_EFFECTS_SNOW_ANIMPAR_GLOBALSNOWGRAD,
	WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT,
	WCS_EFFECTS_SNOW_NUMANIMPAR
	}; // animated SnowEffect params

enum
	{
	WCS_EFFECTS_SNOW_TEXTURE_FEATHERING,
	WCS_EFFECTS_SNOW_NUMTEXTURES
	}; // SnowEffect textures

#define WCS_SEEDOFFSET_SNOW		98		// this is an offset added to the vertex seed values

class SnowEffect: public GeneralEffect
	{
	public:
		double GlobalGradient;	// for rendering
		AnimDoubleTime AnimPar[WCS_EFFECTS_SNOW_NUMANIMPAR];
		RootTexture *TexRoot[WCS_EFFECTS_SNOW_NUMTEXTURES];
		EcosystemEffect Eco;
		char GlobalGradientsEnabled;

		// Initialize GeneralEffect base members in constructor...
		SnowEffect();		// default constructor
		SnowEffect(RasterAnimHost *RAHost);
		SnowEffect(RasterAnimHost *RAHost, EffectsLib *Library, SnowEffect *Proto);
		~SnowEffect();
		void SetDefaults(void);
		void Copy(SnowEffect *CopyTo, SnowEffect *CopyFrom);
		double EvaluateCoverage(RenderData *Rend, PolygonData *Poly);
		double EvaluateCoverage(RenderData *Rend, VectorNode *CurNode);
		double EvaluateCoverage(RenderData *Rend, PixelData *Pix);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_SNOW_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_SNOW_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual int GetRAHostAnimated(void);
		virtual bool AnimateMaterials(void);
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual void SetFloating(char NewFloating);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_SNOW_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_SNOW_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_SNOW_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_SNOW_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Snow Effect)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // SnowEffect

enum
	{
	WCS_EFFECTS_SKY_ANIMPAR_INTENSITY,
	WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER,
	WCS_EFFECTS_SKY_NUMANIMPAR
	}; // animated Sky params

class Sky: public GeneralEffect
	{
	public:
		double *DitherTable;
		AnimDoubleTime AnimPar[WCS_EFFECTS_SKY_NUMANIMPAR];
		AnimColorGradient SkyGrad, LightGrad;
		PRNGX Rand;

		// Initialize GeneralEffect base members in constructor...
		Sky();		// default constructor
		Sky(RasterAnimHost *RAHost);
		Sky(RasterAnimHost *RAHost, EffectsLib *Library, Sky *Proto);
		~Sky();
		void SetDefaults(void);
		void Copy(Sky *CopyTo, Sky *CopyFrom);
		int EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp);
		int EvalOneSampleLine(TextureSampleData *Samp);
		void CleanupFromRender(void);	// deletes DitherTable

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_SKY_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_SKY_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual int GetRAHostAnimated(void);
		virtual int SetToTime(double Time);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Sky)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // Sky

enum
	{
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_COVERAGE,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_SELFSHADING,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_RECVSHADOWINTENS,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTEXP,
	WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT,
	WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR
	}; // animated AtmosphereComponent params

enum
	{
	WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE,
	WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES
	}; // AtmosphereComponent textures

class AtmosphereComponent: public RasterAnimHost, public RootTextureParent, public VolumetricSubstance
	{
	// color gradient first color is bottom
	public:
		double LowElev, HighElev, ElevRange, RadiusInside, RadiusOutside, RefLon, RefLat,
			MetersPerDegLon, MetersPerDegLat,	// for rendering
			CompleteBacklightColor[3], CompleteParticleColor[3];
		char Name[WCS_EFFECT_MAXNAMELENGTH];
		AnimColorTime ParticleColor, BacklightColor;
		AnimDoubleTime AnimPar[WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR];
		AnimDoubleVerticalProfile CovgProf, DensityProf, ShadeProf;
		unsigned long ShadowFlags;
		char Enabled, CovgTextureExists, CastShadows, 
			ReceiveShadowsTerrain, ReceiveShadowsFoliage, ReceiveShadows3DObject, ReceiveShadowsCloudSM, 
			ReceiveShadowsVolumetric, ReceiveShadowsMisc, VolumeBeforeRefl, NoSubBaseDensity;
		RootTexture *TexRoot[WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES];
		PRNGX Rand;
		AtmosphereComponent *Next;

		AtmosphereComponent(RasterAnimHost *RAHost);
		~AtmosphereComponent();
		void Copy(AtmosphereComponent *CopyTo, AtmosphereComponent *CopyFrom);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		long GetNumAnimParams(void) {return (WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		long InitImageIDs(long &ImageID);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_ATMOCOMPONENT);};
		char *GetName(void)									{return (Name);};
		void SetDefaultProperties(char *ParticleTypeStr, double BaseElev);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil, RootTexture **&TexAffil);
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int InitFrameToRender(RenderData *Rend);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_ATMOCOMPONENT);};
		virtual char *GetRAHostName(void)						{return (Name);};
		virtual char *GetRAHostTypeString(void) {return ("(Atmosphere Component)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		int BuildFileComponentsList(EffectList **Coords);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from VolumetricSubstance
		virtual void ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon);
		virtual VolumeRayList *BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3]);
		virtual int PointSampleVolumetric(VolumeRayList *CurRay, VertexDEM *CurVert, TextureData *TexData, double &Shading);
		virtual unsigned long GetShadowFlags(void)	{return (ShadowFlags);};
		virtual double GetShadowIntenstiy(void)	{return (AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_RECVSHADOWINTENS].CurValue);};
		virtual int GetBeforeReflections(void)	{return (VolumeBeforeRefl);};
		virtual int GetShadowsOnly(void)	{return (0);};
		virtual int GetCastVolumetricShadows(void)	{return (CastShadows);};
		virtual double GetBacklightPct(void)	{return (AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].CurValue);};
		virtual double GetBacklightExp(void)	{return (AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTEXP].CurValue);};
		virtual double GetAltBacklightColorPct(void)	{return (AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].CurValue);};
		virtual double GetBacklightColor(int Channel)	{return (CompleteBacklightColor[Channel]);};
		virtual int GetRenderVolumetric(void)	{return (1);};

	}; // AtmosphereComponent

enum
	{
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY,
	WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR
	}; // animated Atmosphere params

enum
	{
	WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR,
	WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR,
	WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR,
	WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES
	}; // Atmosphere textures

enum
	{
	WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE,
	WCS_EFFECTS_ATMOSPHERETYPE_SLOWINCREASE,
	WCS_EFFECTS_ATMOSPHERETYPE_FASTINCREASE,
	WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC,
	WCS_EFFECTS_ATMOSPHERETYPE_EXPONENTIAL
	}; // Atmosphere types

enum
	{
	WCS_EFFECTS_ATMOSPHERECOLORTYPE_HAZE,
	WCS_EFFECTS_ATMOSPHERECOLORTYPE_CLOUDHAZE,
	WCS_EFFECTS_ATMOSPHERECOLORTYPE_FOG
	}; // Atmosphere color types

#define WCS_ATMOSPHERE_OPTDEPTH_TABLESIZE	81

class Atmosphere: public GeneralEffect
	{
	public:
		double HazeEndDistance, CloudHazeEndDistance, FogLowElev, FogHighElev, FogRange, HazeIntensityRange, CloudHazeIntensityRange, 
			FogLowIntensity, FogHighIntensity, FogIntensityRange,
			CompleteTopAmbient[3], CompleteBottomAmbient[3], CompleteHazeColor[3], CompleteCloudHazeColor[3], 
			CompleteFogColor[3], CompleteColor[3];	// calc by renderer
		AnimDoubleTime AnimPar[WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR];
		AnimColorTime TopAmbientColor, BottomAmbientColor;
		char AtmosphereType, HazeEnabled, FogEnabled, SeparateCloudHaze, ActAsFilter;

		// simple
		AnimColorTime HazeColor, FogColor, CloudHazeColor;
		RootTexture *TexRoot[WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES];
		
		// participating
		AtmosphereComponent *Components;
		double ODTable[WCS_ATMOSPHERE_OPTDEPTH_TABLESIZE];

		// Initialize GeneralEffect base members in constructor...
		Atmosphere();		// default constructor
		Atmosphere(RasterAnimHost *RAHost);
		Atmosphere(RasterAnimHost *RAHost, EffectsLib *Library, Atmosphere *Proto);
		~Atmosphere();
		void SetDefaults(void);
		void Copy(Atmosphere *CopyTo, Atmosphere *CopyFrom);
		AtmosphereComponent *AddComponent(AtmosphereComponent *AddMe);
		void EvalColor(PixelData *Pix, int ColorType);
		int BuildFileComponentsList(EffectList **Coords);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual long InitImageIDs(long &ImageID);
		virtual int GetRAHostAnimated(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Atmosphere)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual char *OKRemoveRaster(void);

	}; // Atmosphere

enum
	{
	WCS_EFFECTS_LIGHT_ANIMPAR_LAT,
	WCS_EFFECTS_LIGHT_ANIMPAR_LON,
	WCS_EFFECTS_LIGHT_ANIMPAR_ELEV,
	WCS_EFFECTS_LIGHT_ANIMPAR_HEADING,
	WCS_EFFECTS_LIGHT_ANIMPAR_PITCH,
	WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP,
	WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE,
	WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE,
	WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS,
	WCS_EFFECTS_LIGHT_NUMANIMPAR
	}; // animated Light params

enum
	{
	WCS_EFFECTS_LIGHT_TEXTURE_COLOR,
	WCS_EFFECTS_LIGHT_NUMTEXTURES
	}; // Light textures

enum
	{
	WCS_EFFECTS_LIGHTTYPE_PARALLEL,
	WCS_EFFECTS_LIGHTTYPE_OMNI,
	WCS_EFFECTS_LIGHTTYPE_SPOT
	}; // Light types

#define WCS_LIGHT_MAXSHADOWAASAMPLES	9

class Light: public GeneralEffect
	{
	public:
		double MaxConeCosAngle, MaxHotSpotCosAngle, ConeEdgeCosAngle, InvConeEdgeCosAngle, MaxIllumDist, MaxIllumDistSq,
			TangentDistance, CosTangentAngle, InvCosTangentAngleDifference, CosTangentAnglePlus5Pct, FallOffExp, CompleteColor[3];	// calc by renderer
		AnimDoubleTime AnimPar[WCS_EFFECTS_LIGHT_NUMANIMPAR];
		AnimColorTime Color;
		Object3DEffect *TargetObj;
		ShadowMap3D *Shadows;
		VertexDEM *LightPos, *LightAim;	// calc by renderer
		Point3d LightPosUnit; // a unitized vector version of LightPos->XYZ for more optimal use with VectorAngle calls
		RootTexture *TexRoot[WCS_EFFECTS_LIGHT_NUMTEXTURES];
		IncludeExcludeList InclExcl;
		char LightType, Distant, CastShadows, SoftShadows, AAShadows, IllumAtmosphere, Floating, FlipFoliage, FallSeason, ColorEvaluated;

		// Initialize GeneralEffect base members in constructor...
		Light();		// default constructor
		Light(RasterAnimHost *RAHost);
		Light(RasterAnimHost *RAHost, EffectsLib *Library, Light *Proto);
		~Light();
		void KillShadows(void);
		void SetDefaults(void);
		void Copy(Light *CopyTo, Light *CopyFrom);
		int SetTarget(Object3DEffect *NewTarget);
		double EvaluateShadows(PixelData *Pix);
		ShadowMap3D *MatchShadow(GeneralEffect *TestObj, Joe *TestJoe, VectorPoint *TestVtx, char TestType);
		void AddShadowMap(ShadowMap3D *AddMe);
		void RemoveShadowMap(ShadowMap3D *RemoveMe);
		int AttemptLoadShadowMapFile(ShadowMap3D *Map, long MapWidth, GeneralEffect *MapObj, Joe *MapVec, VectorPoint *MapVtx, long MapVtxNum, char MapType);
		int SaveShadowMapFile(ShadowMap3D *Map, GeneralEffect *MapObj, Joe *MapVec, VectorPoint *MapVtx, long MapVtxNum, char MapType);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		void EvalColor(PixelData *Pix);
		int PassTest(RasterAnimHost *TestMe)	{return (InclExcl.PassTest(TestMe));};
		int ApproveInclExclClass(long MyClass);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual short AnimateShadows(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_LIGHT_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_LIGHT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int GetRAHostAnimated(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SetToTime(double Time);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual void ResolveLoadLinkages(EffectsLib *Lib)	{InclExcl.ResolveLoadLinkages(Lib); GeneralEffect::ResolveLoadLinkages(Lib);};

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_LIGHT_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_LIGHT_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_LIGHT_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_LIGHT_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Light)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual void SetFloating(char NewFloating);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual char *OKRemoveRaster(void);

	}; // Light

enum
	{
	WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT,
	WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON,
	WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV,
	WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT,
	WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON,
	WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV,
	WCS_EFFECTS_CAMERA_ANIMPAR_HEADING,
	WCS_EFFECTS_CAMERA_ANIMPAR_PITCH,
	WCS_EFFECTS_CAMERA_ANIMPAR_BANK,
	WCS_EFFECTS_CAMERA_ANIMPAR_HFOV,
	WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX,
	WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY,
	WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN,
	WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX,
	WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ,
	WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH,
	WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE,
	WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST,
	WCS_EFFECTS_CAMERA_ANIMPAR_FSTOP,
	WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT,
	WCS_EFFECTS_CAMERA_ANIMPAR_ZBUFBOXFILTOFFSET,
	WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN,	// no nodes
	WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT,	// no nodes
	WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH,
	WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION,
	WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE,
	WCS_EFFECTS_CAMERA_NUMANIMPAR
	}; // animated Camera params

enum
	{
	WCS_EFFECTS_CAMERATYPE_TARGETED,
	WCS_EFFECTS_CAMERATYPE_UNTARGETED,
	WCS_EFFECTS_CAMERATYPE_ALIGNED,
	WCS_EFFECTS_CAMERATYPE_OVERHEAD,
	WCS_EFFECTS_CAMERATYPE_PLANIMETRIC
	}; // Camera types

#define WCS_CAMERA_STEREOCHANNEL_LEFT		-1
#define WCS_CAMERA_STEREOCHANNEL_RIGHT		1
#define WCS_CAMERA_STEREOCHANNEL_CENTER		0

#define WCS_RENDERTIME_WEEBIT	.01		// used as a delta T value for align to path
#define WCS_EFFECTS_CAMERA_NUMRESTOREVALUES		13

class Camera: public GeneralEffect
	{
	public:
		double CenterX, CenterY,	// pixels, used in rendering
			HorScale, VertScale,	// scaling factors for plotting to output
			CamLonScale, PlanWidth,	PlanTranslateWidth, // calc by renderer for planimetric scaling
			IAUpVec2D[2], IARightVec2D[2],	// calc by renderer for scaling interactive motion
			CamHeading, CamPitch, CamBank,	// calc by renderer for post process reference
			ProjectedEasting, ProjectedNorthing,	// calc by renderer for projected center of view
			RestoreVal[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES];
		Matx3x3 ScrRotMatx, InvScrRotMatx;
		AnimDoubleTime AnimPar[WCS_EFFECTS_CAMERA_NUMANIMPAR];
		RasterShell *Img;
		Object3DEffect *TargetObj;
		CoordSys *Coords;
		VertexDEM *CamPos, *TargPos, *CamVertical, *CamRight;
		SmoothPath *Smooth;
		long BoxFilterSize, AAPasses, MaxDOFBlurRadius, VRSegments, PanoPanels;
		char CameraType, RenderBeyondHorizon, DepthOfField, BoxFilter, MotionBlur,
			ZBufBoxFilter, FieldRender, FieldRenderPriority, VelocitySmoothing, AAOptimizeReflection, 
			BackgroundImageEnabled, Orthographic, PanoCam, LensDistortion, ApplyLensDistortion, CenterOnOrigin, 
			Floating, InterElevFollow, RestoreValid, StereoCam, StereoConvergence, StereoPreviewChannel, 
			StereoRenderChannel, Projected;

		// Initialize GeneralEffect base members in constructor...
		Camera();		// default constructor
		Camera(RasterAnimHost *RAHost);
		Camera(RasterAnimHost *RAHost, EffectsLib *Library, Camera *Proto);
		~Camera();
		void SetDefaults(void);
		void Copy(Camera *CopyTo, Camera *CopyFrom);
		int SetRaster(Raster *NewRast);
		int SetTarget(Object3DEffect *NewTarget);
		int SetCoords(CoordSys *NewCoords);
		void ProjectVertexDEM(CoordSys *MyCoords, VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx);
		void UnProjectVertexDEM(CoordSys *MyCoords, VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx);
		void UnProjectProjectedVertexDEM(VertexDEM *Vert, double EarthLatScaleMeters);
		int TestValidScreenPosition(VertexDEM *Vert, double CompareValue);
		void SetScales(long ImageWidth, double Aspect);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		void CaptureRestoreValues(double Restore[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES]);
		void RestoreRestoreValues(double Restore[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES]);
		void Zoom(float ZoomDirectionAndAmount);
		void RestoreZoom(void);
		void RestoreZoomProxy(Camera *TheCameraToRestore);
		void ZoomBox(long OldWidth, long OldHeight, long NewWidth, long NewHeight, long NewCenterX, long NewCenterY);
		void ZoomBoxProxy(Camera *TheCameraToZoom, long OldWidth, long OldHeight, long NewWidth, long NewHeight, long NewCenterX, long NewCenterY);
		long GetNextMotionKeyFrame(long LastFrame, double FrameRate);
		GraphNode *GetNearestMotionNode(double KeyTime);
		void CreateBankKeys(void);
		void GetTargetPosition(RenderData *Rend);
		int BuildFileComponentsList(EffectList **CoordSystems);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_CAMERA_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_CAMERA_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Camera)");};
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual void SetFloating(char NewFloating);
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe);
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);

		// S@G context popup menus
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags);
		virtual int HandlePopMenuSelection(void *Action);


	}; // Camera

class RenderJob: public GeneralEffect
	{
	public:
		Camera *Cam;
		RenderOpt *Options;
		EffectList *Scenarios;
		NameList *ScenarioNames;

		// Initialize GeneralEffect base members in constructor...
		RenderJob();		// default constructor
		RenderJob(RasterAnimHost *RAHost);
		RenderJob(RasterAnimHost *RAHost, EffectsLib *Library, RenderJob *Proto);
		~RenderJob();
		void SetDefaults(void);
		void Copy(RenderJob *CopyTo, RenderJob *CopyFrom);
		int SetCamera(Camera *NewCamera);
		int SetRenderOpt(RenderOpt *NewRenderOpt);
		EffectList *AddScenario(GeneralEffect *AddMe);
		int BuildFileComponentsList(EffectList **Cameras, EffectList **RenderOpts, EffectList **ScenarioList);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void ResolveLoadLinkages(EffectsLib *Lib);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Render Job)");};
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // RenderJob

enum
	{
	WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN,
	WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET,
	WCS_EFFECTS_RENDEROPT_NUMANIMPAR
	}; // animated Render Opt params

enum
	{
	WCS_REFLECTIONSTYLE_COMPRESSEDFRAGS,	// obsolete
	WCS_REFLECTIONSTYLE_BEAMTRACE,
	WCS_REFLECTIONSTYLE_ZONESAMPLED,	// obsolete
	WCS_REFLECTIONSTYLE_BTFRAGS,	// was LIMIT3FRAGS
	WCS_REFLECTIONSTYLE_PRETTYSLOW,	// obsolete
	WCS_REFLECTIONSTYLE_SPEEDKILLER	// obsolete
	}; // reflection styles

class RenderOpt: public GeneralEffect
	{
	public:
		double ImageAspectRatio;
		AnimDoubleTime AnimPar[WCS_EFFECTS_RENDEROPT_NUMANIMPAR];
		ImageOutputEvent *OutputEvents;
		EffectList *Post;
		long OutputImageWidth, OutputImageHeight, RenderOffsetX, RenderOffsetY, SetupOffsetX, SetupOffsetY, CamSetupRenderWidth, CamSetupRenderHeight, FrameStep;
		short RenderImageSegments, FragmentDepth, TilesX, TilesY;
		PathAndFile TempPath;
		char EffectEnabled[WCS_MAXIMPLEMENTED_EFFECTS],
			ReflectionsEnabled, CloudShadowsEnabled, ObjectShadowsEnabled, TerrainEnabled,
			RenderDiagnosticData, MultiPassAAEnabled, DepthOfFieldEnabled,
			VectorsEnabled, FoliageEnabled, HazeVectors, RenderFromOverhead, LuminousVectors, LockAspect,
			TransparentWaterEnabled, FragmentRenderingEnabled, ReflectionType, VolumetricsEnabled, TilingEnabled, 
			ConcatenateTiles, FoliageShadowsOnly;

		// Initialize GeneralEffect base members in constructor...
		RenderOpt();		// default constructor
		RenderOpt(RasterAnimHost *RAHost);
		RenderOpt(RasterAnimHost *RAHost, EffectsLib *Library, RenderOpt *Proto);
		~RenderOpt();
		void SetDefaults(void);
		void Copy(RenderOpt *CopyTo, RenderOpt *CopyFrom);
		void CopyEnabledSettings(RenderOpt *CopyTo, RenderOpt *CopyFrom);
		int OutputImages(void);
		int OutputZBuffer(void);
		int OutputDiagnosticData(void);
		int SaveImage(Renderer *RHost, RasterBounds *RBounds, BufferNode *Buffers, long Width, long Height, long Frame, long StereoSide, long PanoPanel, long Field, 
			long AAPass, long Segment, long XTile, long YTile, long PanoPanels, long Fields, long AAPasses, long ImageSegments, long XTiles, long YTiles, 
			long FieldDominance, long BeforePost);
		int SaveSegmentTempFile(BufferNode *Node, long Width, long Height, long Frame, long Segment, long BeforePost);
		int SaveAAPassTempFile(BufferNode *Buffers, BufferNode *Node, long Width, long Height, long Frame, long AAPass, long AAPasses, long ImageSegments, long BeforePost);
		int SaveFieldTempFile(BufferNode *Node, long Width, long Height, long Frame, long Field, long FieldDominance, long AAPasses, long ImageSegments, long BeforePost);
		int SavePanoramaTempFile(BufferNode *Node, long Width, long Height, long Frame, long PanoPanel, long Fields, long AAPasses, long ImageSegments, long BeforePost);
		int SaveTileTempFile(BufferNode *Node, long Width, long Height, long Frame, long XTile, long YTile, long XTiles, long BeforePost);
		FILE *OpenTempFile(char *Mode, char *FileType, char *BufferType, long Frame, long BeforePost);
		FILE *OpenVeryTempFile(char *Mode, char *FileType, char *BufferType, long Frame, long BeforePost);
		void RemoveTempFile(char *FileType, char *BufferType, long Frame, long BeforePost);
		void RenameTempFile(char *FileType, char *BufferType, long Frame, long BeforePost);
		char *MakeTempFileName(char *FullPath, char *FileType, char *BufferType, long Frame, long BeforePost);
		ImageOutputEvent *AddOutputEvent(void);
		int RemoveOutputEvent(ImageOutputEvent *RemoveMe);
		int InitToRender(BufferNode *Buffers, long Width, long Height, int InitSequence);
		int CleanupFromRender(int EndSequence);
		const char *GetFirstOutputName(char &MoreOutputEvents);
		EffectList *AddPostProc(GeneralEffect *AddMe);
		virtual long InitImageIDs(long &ImageID);
		int BuildFileComponentsList(EffectList **PostProc, EffectList **Coords);
		int RenderPostProc(int PreReflection, RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics);
		int ValidateOutputPaths(void);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_RENDEROPT_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_RENDEROPT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Render Options)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // RenderOpt

enum
	{
	WCS_EFFECTS_GRIDDER_NUMANIMPAR
	}; // animated TerraGridder params

class TerraGridder: public GeneralEffect
	{
	public:
		NNGrid NNG;
		DEFG *Defg;
		char Floating;
		long NRows, NCols, EWMaps, NSMaps, RandSeed, Scope, NoiseSize;
		double XPow, YPow, Delta, H;
		float *NoiseMap;
		short UseNoise, Units, Densify;
		CoordSys *Coords;
		DBFilterEvent *Filters;
		//Point3d *TC;
		Joe **TCJoeList;
		int TCCount;
		int TCJoeCount;

		// Initialize GeneralEffect base members in constructor...
		TerraGridder();		// default constructor
		TerraGridder(RasterAnimHost *RAHost);
		TerraGridder(RasterAnimHost *RAHost, EffectsLib *Library, TerraGridder *Proto);
		~TerraGridder();
		void SetDefaults(void);
		void SetFilterDefaults(DBFilterEvent *CurFilter);
		void Copy(TerraGridder *CopyTo, TerraGridder *CopyFrom);
		int GetDefaultBounds(Database *DBHost);
		void SetBounds(double LatRange[2], double LonRange[2]);
		DBFilterEvent *AddFilter(DBFilterEvent *AddMe);
		void RemoveFilter(DBFilterEvent *RemoveMe);
		int MakeGrid(EffectsLib *EffectsHost, Project *ProjHost, Database *DBHost);
		int AllocControlPts(Database *DBHost);
		void FreeControlPts(void);
		int InitializeNNGrid(void);
		void UnInitializeNNGrid(void);
		int SetImportData(struct ImportData *ImpData);
		int SetCoords(CoordSys *NewCoords);
		int BuildFileComponentsList(EffectList **CoordSystems);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_GRIDDER_NUMANIMPAR);};
		//virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_GRIDDER_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void SetFloating(char NewFloating);
		virtual long InitImageIDs(long &ImageID);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Terrain Gridder)");};
		//virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char GetRAHostDropOK(long DropType);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int GetDeletable(RasterAnimHost *TestMe);

	}; // TerraGridder

enum
	{
	WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV,
	WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE,
	WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR
	}; // animated TerrainType params

enum
	{
	WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT,
	WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES
	}; // TerrainType textures

class TerrainType : public RasterAnimHost, public RootTextureParent
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR];
		RootTexture *TexRoot[WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES];
		unsigned long Seed;

		TerrainType(RasterAnimHost *RAHost);
		TerrainType(void);
		~TerrainType();
		void SetDefaults(void);
		void Copy(TerrainType *CopyTo, TerrainType *CopyFrom);

		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		long GetNumAnimParams(void) {return (WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		short AnimateShadows(void);
		int SetToTime(double Time);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_TERRAINTYPE);};
		int GetRAHostAnimated(void);
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend)	{return (1);};
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil, RootTexture **&TexAffil);
		int BuildFileComponentsList(EffectList **Coords);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_TERRAINTYPE);};
		virtual char *GetRAHostTypeString(void) {return ("(Terrain Type)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (RAParent->GetRAEnabled()): 1);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);

	}; // TerrainType

class TerraGenerator: public GeneralEffect
	{
	public:
		TerrainType TerraType;
		GeoRegister GeoReg;
		char PreviewEnabled, InitialSetup, DEMName[WCS_EFFECT_MAXNAMELENGTH];
		long Rows, Cols, RowMaps, ColMaps, PreviewSize;
		DEM *PreviewMap;
		Joe *PreviewJoe;

		// Initialize GeneralEffect base members in constructor...
		TerraGenerator();		// default constructor
		TerraGenerator(RasterAnimHost *RAHost);
		TerraGenerator(RasterAnimHost *RAHost, EffectsLib *Library, TerraGenerator *Proto);
		~TerraGenerator();
		void SetDefaults(void);
		void Copy(TerraGenerator *CopyTo, TerraGenerator *CopyFrom);
		void DoSomethingConstructive(Project *ProjHost, Database *DBHost);
		DEM *MakeTerrain(DEM *MyDEM, long ColMap, long RowMap);
		void DitchPreview(EffectsLib *EffectsHost);
		int EvalSampleInit(TerraGenSampleData *Samp, EffectsLib *EffectsHost, Project *ProjHost, Database *DBHost);
		int EvalOneSampleLine(TerraGenSampleData *Samp);
		Joe *GetPreviewJoe(void)	{return (PreviewJoe);};
		DEM *GetPreviewMap(void)	{return (PreviewMap);};
		long GetPreviewSize(void)	{return (PreviewSize);};
		long GetPreviewEnabled(void)	{return (PreviewEnabled);};
		int BuildFileComponentsList(EffectList **Coords);
		long InitImageIDs(long &ImageID);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		//virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_GENERATOR_NUMANIMPAR);};
		//virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_GENERATOR_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual short AnimateShadows(void);
		virtual int SetToTime(double Time);
		virtual int GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2]);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Terrain Generator)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char GetRAHostDropOK(long DropType);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *OKRemoveRaster(void);

	}; // TerraGenerator

class SearchQuery: public GeneralEffect
	{
	public:
		DBFilterEvent *Filters;

		// Initialize GeneralEffect base members in constructor...
		SearchQuery();		// default constructor
		SearchQuery(RasterAnimHost *RAHost);
		SearchQuery(RasterAnimHost *RAHost, EffectsLib *Library, SearchQuery *Proto);
		~SearchQuery();
		void SetDefaults(void);
		void Copy(SearchQuery *CopyTo, SearchQuery *CopyFrom);
		DBFilterEvent *AddFilter(DBFilterEvent *AddMe);
		void RemoveFilter(DBFilterEvent *RemoveMe);
		int ApproveJoe(Joe *ApproveMe);
		int OneFilterValid(void);
		void HardLinkVectors(Database *DBHost, GeneralEffect *Effect);
		void SelectVectors(Database *DBHost, bool DeselectFirst, bool SetupDB);
		void EnableVectors(Database *DBHost, bool NewState);
		unsigned long int CountSuccessfulVectorCandidates(Database *DBHost);
		unsigned long int CountSuccessfulCandidates(Database *DBHost);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Search Query)");};
		virtual char GetRAHostDropOK(long DropType);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

		// S@G context popup menus
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags);
		virtual int HandlePopMenuSelection(void *Action);
		int ActionNow(void);
	}; // SearchQuery

enum
	{
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM1,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM2,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM3,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM4,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM5,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM6,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM7,
	WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM8,
	WCS_EFFECTS_PROJMETHOD_NUMANIMPAR
	}; // animated ProjectionMethod params

class ProjectionMethod: public RasterAnimHost
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_PROJMETHOD_NUMANIMPAR];
		long MethodID, GCTPMethod, ParamID[WCS_EFFECTS_PROJMETHOD_NUMANIMPAR];
		char MethodName[WCS_EFFECT_MAXNAMELENGTH], ParamName[WCS_EFFECTS_PROJMETHOD_NUMANIMPAR][WCS_EFFECT_MAXNAMELENGTH];
		static ProjectionMethodTable MethodTable;
		static ProjectionParameterTable ParamTable;

		ProjectionMethod();
		ProjectionMethod(RasterAnimHost *RAHost);
		void SetDefaults(void);
		void Copy(ProjectionMethod *CopyTo, ProjectionMethod *CopyFrom);
		int Equals(ProjectionMethod *MatchMe);
		int GTEquals(ProjectionMethod *MatchMe);
		unsigned short GetGeoTIFFMethodCode(void);
		void SetMethod(char *NewName);	// NAME field
		void SetMethod2(char *NewName);	// PRJ_NAME field
		void SetMethod(long NewID);
		void SetMethodByCode(long NewCode);
		void SetMethodByEPSGCode(long NewCode);
		void SetMethodByGeoTIFFCode(long NewCode);
 		int SetMethodByPrjName(char *PrjName);
		bool ValidMethod(char *NewName);	// NAME field
		bool ValidMethod2(char *NewName);	// PRJ_NAME field
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_PROJMETHOD);};
		long GetNumAnimParams(void) {return (WCS_EFFECTS_PROJMETHOD_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_PROJMETHOD_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		char GetRAHostDropOK(long DropType);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		int GetRAHostAnimated(void);
		void SetMethodParams(long NewID);
		void SetParamMetrics(void);
		long IdentifyMethodIDFromName(char *ProjMethodName);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);

		// parameter ID to subscript mapping
		int FindParamAnimParSlotByGCTPID(int GCTPID); // returns -1 for failure

		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_PROJECTIONMETHOD);};
		virtual char *GetRAHostTypeString(void) {return ("(Projection Method)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (RAParent->GetRAEnabled()): 1);};
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // class ProjectionMethod

enum
	{
	WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR,
	WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR,
	WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR
	}; // animated GeoEllipsoid params

class GeoEllipsoid: public RasterAnimHost
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR];
		long EllipsoidID;
		char EllipsoidName[WCS_EFFECT_MAXNAMELENGTH];
		static EllipsoidTable EllipseTable;

		GeoEllipsoid();
		GeoEllipsoid(RasterAnimHost *RAHost);
		void SetDefaults(void);
		void Copy(GeoEllipsoid *CopyTo, GeoEllipsoid *CopyFrom);
		int Equals(GeoEllipsoid *MatchMe);
		int GTEquals(GeoEllipsoid *MatchMe);
		void SetEllipsoid(char *NewName);
		void SetEllipsoid(long NewID);
		void SetEllipsoidByCode(long NewCode);
		void SetEllipsoidByEPSGCode(long NewCode);
 		int SetEllipsoidByPrjName(char *PrjName);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_GEOELLIPSOID);};
		long GetNumAnimParams(void) {return (WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_GEOELLIPSOID_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		char GetRAHostDropOK(long DropType);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		int GetRAHostAnimated(void);
		long IdentifyEllipsoidIDFromName(char *GeoEllipseName);
		unsigned short GetEPSGEllipsoidCodeFromName(char *GeoEllipseName);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);

		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_GEOELLIPSOID);};
		virtual char *GetRAHostTypeString(void) {return ("(Ellipsoid)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (RAParent->GetRAEnabled()): 1);};
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);

	}; // class GeoEllipsoid

enum
	{
	WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX,
	WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY,
	WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ,
	WCS_EFFECTS_GEODATUM_NUMANIMPAR
	}; // animated GeoDatum params

class GeoDatum: public RasterAnimHost
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_GEODATUM_NUMANIMPAR];
		long DatumID;
		char DatumName[WCS_EFFECT_MAXNAMELENGTH];
		GeoEllipsoid Ellipse;
		static DatumTable DatmTable;

		GeoDatum();
		GeoDatum(RasterAnimHost *RAHost);
		void SetDefaults(void);
		void Copy(GeoDatum *CopyTo, GeoDatum *CopyFrom);
		int Equals(GeoDatum *MatchMe);
		int GTEquals(GeoDatum *MatchMe);
		void SetDatum(char *NewName);
		void SetDatum(long NewID);
		void SetDatumByCode(long NewCode);
		void SetDatumByEPSGCode(long NewCode);
		int SetDatumByERMName(char *ERMName);
 		int SetDatumByPrjName(char *PrjName);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_GEODATUM);};
		long GetNumAnimParams(void) {return (WCS_EFFECTS_GEODATUM_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_GEODATUM_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		char GetRAHostDropOK(long DropType);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		int GetRAHostAnimated(void);
		long IdentifyDatumIDFromName(char *GeoDatumName);
		unsigned short GetEPSGDatumCodeFromName(char *GeoDatumName);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);

		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_GEODATUM);};
		virtual char *GetRAHostTypeString(void) {return ("(Geodetic Datum)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (RAParent->GetRAEnabled()): 1);};
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);

	}; // class GeoDatum

enum COORDSYS_IDENTS
	{
	COORDSYS_AZIMUTH,
	COORDSYS_CENTER_LAT,
	COORDSYS_CENTER_LON,
	COORDSYS_CENTRAL_MERIDIAN,
	COORDSYS_FALSE_EASTING,
	COORDSYS_FALSE_NORTHING,
	COORDSYS_ORIGIN_LAT,
	COORDSYS_ORIGIN_LON,
	COORDSYS_POINT1_LAT,
	COORDSYS_POINT1_LON,
	COORDSYS_POINT2_LAT,
	COORDSYS_POINT2_LON,
	COORDSYS_POINT3_LAT,
	COORDSYS_POINT3_LON,
	COORDSYS_RECTIFIED_GRID_ANGLE,
	COORDSYS_SCALE_FACTOR,
	COORDSYS_STD_PARALLEL1,
	COORDSYS_STD_PARALLEL2
	};

class CoordSys: public GeneralEffect, public Projectoid
	{
	public:
		double ToidValues[15], DeltaX, DeltaY, DeltaZ, EccSq, EccPrimeSq;
		ImageList *Images;
		long ProjSysID, ZoneID, ErrorCode;
		char Initialized, Geographic;	// Geographic may or may have been set correctly - use GetGeographic() only
		char ProjSysName[WCS_EFFECT_MAXNAMELENGTH], ZoneName[WCS_EFFECT_MAXNAMELENGTH];
		ProjectionMethod Method;
		GeoDatum Datum;
		static ProjectionSystemTable ProjSysTable;
		ProjectionZoneTable ZoneTable;

		// result caching optimization
		double CachedLat, CachedSinPhi, CachedCosPhi;
		double CachedLon, CachedSinLam, CachedCosLam;

		// Initialize GeneralEffect base members in constructor...
		CoordSys();		// default constructor
		CoordSys(RasterAnimHost *RAHost);
		CoordSys(RasterAnimHost *RAHost, EffectsLib *Library, CoordSys *Proto);
		~CoordSys();
		void SetDefaults(void);
		void Copy(CoordSys *CopyTo, CoordSys *CopyFrom);
		int Equals(CoordSys *MatchMe);
		int GTEquals(CoordSys *MatchMe);
		unsigned short GetEPSGStatePlaneCode(void);
		void SetSystem(char *NewName);
		void SetSystem(long NewID);
		void SetSystemByCode(long NewCode);
		void SetZone(char *NewName);
		void SetZone(long NewID);
		void SetZoneByCode(long NewCode);
		void SetZoneByShort(char *Shorthand);
		void SetStatePlaneZoneByEPSG27(long NAD27SPZone);
 		void SetStatePlaneZoneByERMName(char *ERMZoneName);
 		void SetStatePlaneZoneByFIPS(long FIPSZone);
		long IdentifyProjSysIDFromName(char *ProjSysName);
		void SetZoneFile(void);
		long IdentifyZoneIDFromName(char *ZoneName);
		ImageList *AddRaster(Raster *NewRast);
		void NewItemCallBack(long NewID, char ItemType);
		void OpenTableList(char ApplyTo, int GoneModal);
		int Initialize(void);
		int InitializeMethod(void);
		int ProjToCart(VertexDEM *Vert);
		int ProjToDeg(VertexDEM *Vert);
		int DegToCart(VertexDEM *Vert);
		int CartToProj(VertexDEM *Vert);
		int CartToDeg(VertexDEM *Vert);
		int DegToProj(VertexDEM *Vert);
		int ProjToDefDeg(VertexDEM *Vert);
		int DefDegToProj(VertexDEM *Vert);
		void EditModal(void);
		void UpdateJoeBounds(void);
		int DatumDeltaCalculator(void);
		const char *GetXUnitName(void);
		const char *GetYUnitName(void);
		int GetGeographic(void);	// this will init the CoordSys if necessary
		double FetchPlanetRadius(double Latitude);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, int LinkImages);
		unsigned long Save(FILE *ffile, int SaveImages);
		void SetStatePlaneFIPS(const char *str);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile)
			{return (Save(ffile, true));};
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)	
			{return (Load(ffile, ReadSize, ByteFlip, true));};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int SetToTime(double Time);
		virtual int GetRAHostAnimated(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Coordinate System)");};
		char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		int SaveToArcPrj(FILE *fHandle);
		int WriteWKTParam(FILE *fHandle, double Value, COORDSYS_IDENTS Param);
		// Utility function to parse Arc .prj files
		CSLoadInfo *LoadFromArcPrj(char *FilePath);
		CSLoadInfo *LoadFromArcPrj(FILE *fHandle);

		// Utility functions for parsing WKT (Well-Known Text)
		short WKT_Authority(char **str);
		short WKT_Axis(void);
		short WKT_Datum(void);
		short WKT_PrimeMeridian(void);
		short WKT_Projection(void);
		short WKT_Spheroid(void);
		short WKT_ToWGS84(void);
	}; // CoordSys

enum
	{
	WCS_EFFECTS_FENCE_POLYGON_SPAN,
	WCS_EFFECTS_FENCE_POLYGON_ROOF
	}; // fence polygon types

enum
	{
	WCS_EFFECTS_FENCE_POSTTYPE_KEY,
	WCS_EFFECTS_FENCE_POSTTYPE_ALT
	}; // fence post types

enum
	{
	WCS_EFFECTS_FENCE_POSTCOUNT_ALL,
	WCS_EFFECTS_FENCE_POSTCOUNT_KEY
	}; // fence post types

enum
	{
	//WCS_EFFECTS_FENCE_ANIMPAR_POSTTOPELEV,
	//WCS_EFFECTS_FENCE_ANIMPAR_POSTBOTELEV,
	WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV,
	WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV,
	WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV,
	WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER,
	WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER,
	WCS_EFFECTS_FENCE_NUMANIMPAR
	}; // animated Fence params

enum
	{
	WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER,
	WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER,
	WCS_EFFECTS_FENCE_NUMTEXTURES
	}; // Fence textures

enum
	{
	WCS_EFFECTS_FENCE_THEME_SPANTOPELEV,
	WCS_EFFECTS_FENCE_THEME_SPANBOTELEV,
	WCS_EFFECTS_FENCE_THEME_ROOFELEV,
	WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER,
	WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER,
	WCS_EFFECTS_FENCE_NUMTHEMES
	}; // Fence themes

class Fence: public GeneralEffect, public SXQueryItem
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_FENCE_NUMANIMPAR];
		GeneralEffect *KeyPost, *AltPost;
		RootTexture *TexRoot[WCS_EFFECTS_FENCE_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_FENCE_NUMTHEMES];
		long KeyPostInterval, KeyPostType, AltPostType;
		AnimMaterialGradient SpanMat, RoofMat;
		char PostsEnabled, SpansEnabled, RoofEnabled, AllOrKeyPosts, 
			FirstKeyPost, LastKeyPost, PostHtFromObject, AltPostHtFromObject, SeparateRoofMat, ConnectToOrigin, SkinFrame;

		// Initialize GeneralEffect base members in constructor...
		Fence();		// default constructor
		Fence(RasterAnimHost *RAHost);
		Fence(RasterAnimHost *RAHost, EffectsLib *Library, Fence *Proto);
		~Fence();
		void SetDefaults(void);
		void Copy(Fence *CopyTo, Fence *CopyFrom);
		int BuildFileComponentsList(EffectList **Foliages, EffectList **Material3Ds, EffectList **Object3Ds, 
			EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int SetPost(GeneralEffect *NewPost, int WhichPost);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		double GetRenderMatGradientPos(RenderData *Rend, PolygonData *Poly);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int GetRAHostAnimated(void);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_FENCE_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_FENCE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual int SetToTime(double Time);
		virtual short AnimateShadows(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Wall)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char GetRAHostDropOK(long DropType);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_FENCE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_FENCE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_FENCE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_FENCE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_FENCE_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_FENCE_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_FENCE_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_FENCE_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

	}; // Fence

class PostProcess: public GeneralEffect
	{
	public:
		char BeforeReflection;
		PostProcessEvent *Events;

		// Initialize GeneralEffect base members in constructor...
		PostProcess();		// default constructor
		PostProcess(RasterAnimHost *RAHost);
		PostProcess(RasterAnimHost *RAHost, EffectsLib *Library, PostProcess *Proto);
		~PostProcess();
		void SetDefaults(void);
		void Copy(PostProcess *CopyTo, PostProcess *CopyFrom);
		PostProcessEvent *AddEvent(PostProcessEvent *AddMe);
		PostProcessEvent *ChangeEventType(PostProcessEvent *ChangeMe, unsigned char NewType);
		int AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers);
		int BuildFileComponentsList(EffectList **Coords);
		int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, long Width, long Height, 
			BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics);
		int CheckBeforeReflectionsLegal(void);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual long InitImageIDs(long &ImageID);
		virtual int GetRAHostAnimated(void);
		virtual int SetToTime(double Time);
		virtual void ResolveLoadLinkages(EffectsLib *Lib);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Post Process)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char GetRAHostDropOK(long DropType);
		virtual char *OKRemoveRaster(void);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *Test);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

	}; // PostProcess

class RenderScenario: public GeneralEffect
	{
	public:
		char RegenShadows;
		char *InitialStates;
		long NumItems;
		RasterAnimHostBooleanList *Items;
		NumberList *DBItemNumbers;
		AnimDoubleBoolean MasterControl;

		// Initialize GeneralEffect base members in constructor...
		RenderScenario();		// default constructor
		RenderScenario(RasterAnimHost *RAHost);
		RenderScenario(RasterAnimHost *RAHost, EffectsLib *Library, RenderScenario *Proto);
		~RenderScenario();
		void SetDefaults(void);
		void Copy(RenderScenario *CopyTo, RenderScenario *CopyFrom);
		RasterAnimHostBooleanList *AddItem(RasterAnimHost *AddMe, int GenNotify);
		long AddDBItem(void);
		long AddQueryDBItems(Database *DBHost);
		long AddImageItem(void);
		long CountVectors(void);
		long AddItemsByClass(EffectsLib *HostEffects, ImageLib *HostImages, Database *HostDB, long ItemClass);
		int SetupToRender(double RenderTime, int &RegenShadowMaps, Database *DBHost);	// stores enabled state of all items, sets initial state
		void CleanupFromRender(Database *DBHost);	// restores enabled state of all items
		int ProcessFrameToRender(double FrameTime, int &RegenShadowMaps, Database *DBHost);	// returns 1 if an item's enabled state changed at this frame

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID);
		virtual void HardLinkVectors(Database *DBHost);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Scenario)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// S@G context popup menus
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags);
		virtual int HandlePopMenuSelection(void *Action);
		int ActionNow(void);
	}; // RenderScenario

enum
	{
	WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES,
	WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES,
	WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER,
	WCS_EFFECTS_DEMMERGER_NUMANIMPAR
	}; // animated DEMMerger params

class DEMMerger: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_DEMMERGER_NUMANIMPAR];
		double MergeXMax, MergeXMin, MergeYMax, MergeYMin, HiResMergeXMax, HiResMergeXMin, HiResMergeYMax, HiResMergeYMin, XCellSize, XOrigin, YCellSize, YOrigin;
		float NullVal;
		unsigned long MergeWidth, MergeHeight, HighResMergeWidth, HighResMergeHeight;
		char DEMName[64], DEMPath[192], HiResName[64], HiResPath[192], GoodBounds, MultiRes;
		GeoRegister NormalBounds, HiResBounds;
		CoordSys *BCS_CoordSys, *MergeCoordSys;
		EffectList *Queries;

		// Initialize GeneralEffect base members in constructor...
		DEMMerger();		// default constructor
		DEMMerger(RasterAnimHost *RAHost);
		~DEMMerger();
		DEMMerger(RasterAnimHost *RAHost, EffectsLib *Library, DEMMerger *Proto);
		void SetDefaults(void);
		void Copy(DEMMerger *CopyTo, DEMMerger *CopyFrom);
		EffectList *AddQuery(GeneralEffect *AddMe);
		int GrabAllQueries(void);
		int SetCoords(CoordSys *NewCoords);
		long InitImageIDs(long &ImageID);
		int BuildFileComponentsList(EffectList **QueryList, EffectList **CoordsList);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		void SetGoodBoundsEtc(void);
		void UpdateMergeBounds(DEM *CheckMe, double &XMin, double &YMin, double &XMax, double &YMax);
#ifdef GORILLA
		float GaussFix(float *Elevs, unsigned long Rows, unsigned long CtrCol, unsigned long CtrRow);
#endif // GORILLA
		void Merge(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, InterCommon *InteractHost);
		void MergeMultiRes(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, InterCommon *InteractHost);
		void UpdateBounds(unsigned long HiRes, Database *DBHost, Project *ProjHost, bool setRes = false);
		void ScanForRes(Database *DBHost);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_DEMMERGER_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_DEMMERGER_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(DEM Merger)");};
		virtual char *GetCritterName(RasterAnimHost *Test);

	}; // DEMMerger

enum
	{
	WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES,
	WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS,
	WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS
	}; // VectorExpType
enum
	{
	WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST,	// no foliage in either bitmap, no second bitmap
	WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST,		// foliage and terrain in first bitmap, no second bitmap
	WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND,	// foliage only in second bitmap, terrain only in first
	WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND		// foliage and terrain in second bitmap, no foliage in first
	}; // TextureFoliageType
enum
	{
	WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES,
	WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS
	}; // BoundsType
enum
	{
	WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2,
	WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1,
	WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE
	}; // TerrainResOption
enum
	{
	WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS,
	WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS,
	WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER
	}; // FoliageStyle
enum
	{
	WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_ALPHA,
	WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_CLIPMAP
	}; // FolTransparencyStyle

enum
	{
	WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY,
	WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_CREATEALL
	}; // FolTransparencyStyle

enum
	{
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH,
	WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH,
	WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR
	}; // animated SceneExporter params

class SceneExporter: public GeneralEffect
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR];
		double TimeRendered, PlanetRadius;
		float ReplaceNULLElev, MinRenderedElevation, MaxRenderedElevation;
		long DEMTilesX, DEMTilesY, TexTilesX, TexTilesY, DEMResX, DEMResY, TexResX, TexResY, 
			OneDEMResX, OneDEMResY, OneTexResX, OneTexResY, DEMTileOverlap, TexTileOverlap,
			SkyRes, FoliageRes, MaxDEMTiles, MaxTexTiles, MaxFolRes, MaxTexRes, MaxDEMRes, MaxDEMVerts, 
			RowsRendered, ColsRendered, FrameRendered, MaxDEMTileOverlap, MinDEMTileOverlap, MaxTexTileOverlap, 
			MinTexTileOverlap, MaxWallTexSize, Max3DOTexSize, LODLevels, NumFoliageBoards, NumVecInstances, 
			MaxCrossBd, MinCrossBd, Unique3DObjectInstances, LabelInstance, OrigDEMResX, OrigDEMResY;
		char ExportTerrain, ExportVectors, ExportSky, ExportCelest, ExportClouds, ExportStars, ExportFoliage, ExportLabels,
			ExportTexture, Export3DObjects, Export3DFoliage, ExportWalls, ExportAtmosphere, ExportVolumetrics,
			VectorExpType, TextureFoliageType, BoundsType, DEMResOption, TexResOption, SkyResOption, FolResOption,
			FoliageStyle, FolTransparencyStyle, ObjectTreatment, BurnShadows, BurnShading, BurnVectors, FractalMethod, FragmentCollapseType,
			RenderSizesSet, ExportCameras, ExportLights, ExportHaze, ZipItUp, TerrainAsObj, FoliageAsObj, SkyAsObj, ObjectAsObj, WallAsObj,
			VectorAsObj, EqualTiles, LODFillGaps, TileWallTex, BurnWallShading, AllowDEMNULL, FoliageInstancesAsObj,
			AlignFlipBoardsToCamera, PadFolImageBottom, TransparentPixelsExist, WallFloors, WorldFile, SquareCells;
		char ExportTarget[64], ImageFormat[64], FoliageImageFormat[64], SingleOpt[128], MultiOpt[128], ObjectFormat[64], 
			DEMFormat[64], PathBackup[512], DEMFormatBackup[64], TextureImageFormat[64];
		CoordSys *Coords;
		EffectList *Scenarios;
		EffectList *Cameras;
		EffectList *Lights;
		EffectList *Haze;
		RasterAnimHostList *EphemeralObjects, *SXActionItemIterator;	// SXActionItemIterator is used in GetRAHostChild
		SXQueryAction *SXActionIterator;	// used in GetRAHostChild
		NameList *ScenarioNames, *CameraNames, *LightNames, *HazeNames;
		Object3DInstance *ObjectInstanceList;
		VectorExportItem *VecInstanceList;
		SXExtension *FormatExtension;
		RasterBounds RBounds;
		GeoRegister GeoReg;
		ExportReferenceData ExportRefData;
		PathAndFile TempPath, OutPath;

		// Initialize GeneralEffect base members in constructor...
		SceneExporter();		// default constructor
		SceneExporter(RasterAnimHost *RAHost);
		SceneExporter(RasterAnimHost *RAHost, EffectsLib *Library, SceneExporter *Proto);
		~SceneExporter();
		void SetDefaults(void);
		void Copy(SceneExporter *CopyTo, SceneExporter *CopyFrom);
		EffectList *AddScenario(GeneralEffect *AddMe);
		EffectList *AddCamera(GeneralEffect *AddMe, int SendNotify);
		EffectList *AddLight(GeneralEffect *AddMe, int SendNotify);
		EffectList *AddHaze(GeneralEffect *AddMe, int SendNotify);
		EffectList *AddExportItem(GeneralEffect *AddMe, int SendNotify);
		RasterAnimHostList *AddObjectToEphemeralList(RasterAnimHost *AddMe);
		int FindInEphemeralList(RasterAnimHost *FindMe);
		void DeleteExportItems(long EffectClass);
		int BuildFileComponentsList(EffectList **ScenarioList, EffectList **CoordSystems);
		int SetCoords(CoordSys *NewCoords);
		void SetTarget(char *NewTarget);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		void ClearRBounds(void)	{RBounds.SetDefaults();};
		void SetupRBounds(Database *DBHost, EffectsLib *EffectsHost);
		void SetupExportReferenceData(double PlanetRad, double MinElev, double MaxElev);
		RasterBounds *FetchRBounds(void)	{return &RBounds;};
		void ValidateTileRowsCols(long &OutRows, long &OutCols, long TilesY, long TilesX, long Overlap,
			long &TileRows, long &TileCols, int ResOption);
		void DeleteObjectInstanceList(void);
		void DeleteObjectEphemeralList(void);
		void DeleteVectorInstanceList(void);
		int InitToExport(void);
		void CleanupFromExport(void);
		void CloseQueryFiles(void);
		void CalcFullResolutions(void);
		void ComputeRowsFromNumCells(long NumCells);
		void ComputeColsFromNumCells(long NumCells);
		int MultipleObjectUVMappingsSupported(void);
		SXExtension *AllocFormatExtension(void);
		void RemoveFormatExtension(void);
		long SaveLabelImage(Raster *LabelRast, ImageLib *Lib, Label *Labl);
		double CalcDefaultSpeed(void);
		SXQueryAction *AddQueryAction(void);
		SXQueryAction *RemoveQueryAction(SXQueryAction *RemoveMe);
		SXQueryAction *GetActionList(void);
		SXQueryAction *VerifyActiveAction(SXQueryAction *VerifyMe);
		bool ApproveActionAvailable(int TestType);
		bool ApproveActionItemAvailable(int TestItem);
		void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID);
		void SetActiveAction(SXQueryAction *NewActive);
		void BackupPath(void);
		void RestorePath(void);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual void Edit(void);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual long InitImageIDs(long &ImageID);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void ResolveLoadLinkages(EffectsLib *Lib);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Scene Exporter)");};
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual char *GetCritterName(RasterAnimHost *Test);

	}; // SceneExporter

enum
	{
	WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV,
	WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE,
	WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT,
	WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH,
	WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH,
	WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT,
	WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH,
	WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH,
	WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT,
	WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE,
	WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE,
	WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR,
	WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR,
	WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR,
	WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR,
	WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR,
	WCS_EFFECTS_LABEL_NUMANIMPAR
	}; // animated Label params

enum
	{
	WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE,
	WCS_EFFECTS_LABEL_NUMTEXTURES
	}; // Label textures

enum
	{
	WCS_EFFECTS_LABEL_THEME_MASTERSIZE,
	WCS_EFFECTS_LABEL_THEME_BASEELEV,
	WCS_EFFECTS_LABEL_NUMTHEMES
	}; // Label themes


enum
	{
	WCS_EFFECTS_LABEL_JUSTIFY_LEFT,
	WCS_EFFECTS_LABEL_JUSTIFY_CENTER,
	WCS_EFFECTS_LABEL_JUSTIFY_RIGHT
	}; // label text Justification

enum
	{
	WCS_EFFECTS_LABEL_FLAGSIZE_FIXED,
	WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING,
	WCS_EFFECTS_LABEL_FLAGSIZE_FIXEDFLOATTEXT
	}; // label FlagWidthStyle, FlagHeightStyle, TextSizeStyle

enum
	{
	WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL,
	WCS_EFFECTS_LABEL_POLESTYLE_ANGLED
	}; // label PoleStyle

enum
	{
	WCS_EFFECTS_LABEL_POLEBASESTYLE_SQUARE,
	WCS_EFFECTS_LABEL_POLEBASESTYLE_TAPERED
	}; // label PoleStyle

enum
	{
	WCS_EFFECTS_LABEL_POLEPOSITION_LEFT,
	WCS_EFFECTS_LABEL_POLEPOSITION_CENTER,
	WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT
	}; // label PolePosition

enum
	{
	WCS_EFFECTS_LABEL_HIRESFONT_ALWAYS,
	WCS_EFFECTS_LABEL_HIRESFONT_SOMETIMES,
	WCS_EFFECTS_LABEL_HIRESFONT_NEVER
	}; // label HiResFont

enum
	{
	WCS_EFFECTS_LABEL_ANCHORPOINT_CENTER,
	WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERLEFT,
	WCS_EFFECTS_LABEL_ANCHORPOINT_LEFTEDGE,
	WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERLEFT,
	WCS_EFFECTS_LABEL_ANCHORPOINT_TOPEDGE,
	WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERRIGHT,
	WCS_EFFECTS_LABEL_ANCHORPOINT_RIGHTEDGE,
	WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERRIGHT,
	WCS_EFFECTS_LABEL_ANCHORPOINT_BOTTOMEDGE
	}; // label text AnchorPoint

#define WCS_LABELTEXT_MAXLEN	2048

class Label: public GeneralEffect, public SXQueryItem
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_LABEL_NUMANIMPAR];
		AnimColorTime TextColor, OutlineColor, FlagColor, BorderColor, PoleColor;
		double CompleteTextColor[3], CompleteOutlineColor[3], CompleteFlagColor[3], CompleteBorderColor[3], CompletePoleColor[3];
		RootTexture *TexRoot[WCS_EFFECTS_LABEL_NUMTEXTURES];
		ThematicMap *Theme[WCS_EFFECTS_LABEL_NUMTHEMES];
		char PreviewEnabled, Justification, FlagWidthStyle, FlagHeightStyle, WordWrapEnabled, PoleStyle, PoleBaseStyle,
			PoleFullWidth, PoleFullHeight, PolePosition, AnchorPoint,
			TextEnabled, OutlineEnabled, FlagEnabled, BorderEnabled, PoleEnabled, OverheadViewPole, HiResFont;
		char MesgText[WCS_LABELTEXT_MAXLEN];

		// Initialize GeneralEffect base members in constructor...
		Label();		// default constructor
		Label(RasterAnimHost *RAHost);
		Label(RasterAnimHost *RAHost, EffectsLib *Library, Label *Proto);
		~Label();
		void SetDefaults(void);
		void Copy(Label *CopyTo, Label *CopyFrom);
		int FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, VectorPoint *CurVtx);
		AnimColorTime *GetReplacementColor(void)	{return (&FlagColor);};
		char GetShade3D(void)	{return (0);};
		void ScaleSizes(double ScaleFactor);

		// inherited from GeneralEffect
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void Edit(void);
		//virtual short AnimateShadows(void); // removed 07/15/05 by CXH and GRH because labels never cast shadows anyway.
		virtual long GetNumAnimParams(void) {return (WCS_EFFECTS_LABEL_NUMANIMPAR);};
		virtual AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_LABEL_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		virtual int SetToTime(double Time);
		virtual int GetRAHostAnimated(void);
		virtual long GetKeyFrameRange(double &FirstKey, double &LastKey);
		virtual char GetRAHostDropOK(long DropType);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		virtual int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		virtual int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int SaveObject(FILE *ffile, const char *SuppliedFileName);
		virtual int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		// inherited from RasterAnimHost
		virtual char *GetRAHostTypeString(void) {return ("(Label)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_EFFECTS_LABEL_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_EFFECTS_LABEL_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum)	{return (TexNum < WCS_EFFECTS_LABEL_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_EFFECTS_LABEL_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_EFFECTS_LABEL_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_LABEL_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum)	{return (ThemeNum < WCS_EFFECTS_LABEL_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_EFFECTS_LABEL_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};

	}; // Label


// <<<>>> ADD_NEW_EFFECTS here is where the definition of the class goes along with enums for 
// anim params and textures or any other defines that pertain to the class


inline double CalcExag(double Elev, PlanetOpt *Planet)
{
return(Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue + (Elev - Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) * Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue);
} // CalcExag

inline double UnCalcExag(double Elev, PlanetOpt *Planet)
{
double Exag;
Exag = Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
if(Exag == 0.0) return(Elev);
return(Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue + (Elev - Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) / Exag);
} // UnCalcExag

#endif // WCS_EFFECTSLIB_H
