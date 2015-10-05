// Database.h
// Header file for Database.cpp
// Built from scratch on 03/20/95 by Chris "Xenon" Hanson
// Copyright 1995

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_DATABASE_H
#define WCS_DATABASE_H

#include "Points.h"
#include "Layers.h"
#include "Notify.h"
#include "Types.h"
#include "RasterBounds.h"

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

class ImportHook;
class Database;
class Joe;
class LoadStack;
class DEM;
class MessageLog;
class Project;
class ImportHook;
class EffectsLib;
struct ImportData;
class JoeApproveHook;
class Object3DEffect;
class Renderer;
class InterCommon;
class EngineController;
class RasterAnimHost;
class DEMBounds;
class JoeProperties;
class DBFilterEvent;
class RenderJoeList;
class SearchQuery;
class GeneralEffect;
class EffectEval;

#define WCS_DXF_MAX_OBJPTS	32767	// 2000 for V3 - increased by GRH on 6/6/98 at the request of Arc/Info users - there are 10 bytes per point of permanent storage in InterCommon
#define WCS_DXF_MAX_LAYERS	10
#define WCS_DXF_MAX_LAYERLENGTH	32
#define WCS_DXF_MAX_MATERIAL_NAMELEN	24

class ImportHook
	{
	public:
		short RefSys;
		double Lat, Lon, El;
		char Connected;
		void *ObjectThis;
		void (*Invoke)(ImportHook *);
	}; // ImportHook

class JoeApproveHook
	{
	public:
		int ExportClasses, ExportObjects, ExportDEMsAs;
		Joe *ApproveMe;
		void *ObjectThis;
		int (*Approve)(JoeApproveHook *);
	}; // JoeApproveHook

struct DXFImportPolygon
	{
	unsigned long Index[4];
	char MaterialStr[WCS_DXF_MAX_MATERIAL_NAMELEN];
	}; // class DXFImportPolygon

// Notify codes

// Notify class
#define WCS_NOTIFYCLASS_DBASE		132

// Subclass indicates what happened to the database
enum
	{
	WCS_SUBCLASS_JOE = 200,
	WCS_SUBCLASS_VECTOR,
	WCS_SUBCLASS_DEM,
	WCS_SUBCLASS_CONTROLPT,
	WCS_NOTIFYDBASE_NEW,
	WCS_NOTIFYDBASE_PRELOAD,
	WCS_NOTIFYDBASE_LOAD,
	WCS_NOTIFYDBASE_ADDOBJ,
	WCS_NOTIFYDBASE_PRECHANGEOBJ,
	WCS_NOTIFYDBASE_CHANGINGOBJ,
	WCS_NOTIFYDBASE_CHANGEOBJ,
	WCS_NOTIFYDBASE_PRECHANGEACTIVE,
	WCS_NOTIFYDBASE_CHANGEACTIVE,
	WCS_NOTIFYDBASE_DELOBJ
	}; // class

// Item indicates what is about to happen or what did happen
// to an object during PreChangeObj/ChangeObj
enum
	{
	WCS_NOTIFYDBASECHANGE_NAME,
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB,
	WCS_NOTIFYDBASECHANGE_FLAGS,
	WCS_NOTIFYDBASECHANGE_POINTS
	}; // item


enum
	{
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB_RGB,
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PEN,
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB_WIDTH,
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB_LSTYLE,
	WCS_NOTIFYDBASECHANGE_DRAWATTRIB_PSTYLE,
	WCS_NOTIFYDBASECHANGE_INTEREDIT_ERASE,
	WCS_NOTIFYDBASECHANGE_INTEREDIT_DRAW
	}; // Component

enum
	{
	WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM = 0,
	WCS_NOTIFYDBASECHANGE_POINTS_CONFORM
	}; // Component

enum
	{
	WCS_NOTIFYDBASECHANGE_FLAGS_SELSTATECLEAR = 1
	}; // Component


class Database : public NotifyEx
	{
	friend class Joe;
//	private:
	public:
		Joe *StaticRoot, *DynamicRoot, *LogRoot;
		double GeoClipNorth, GeoClipSouth, GeoClipEast, GeoClipWest;
		Joe *GenericPrev, *ActiveObj;
		unsigned long NextFlags;
		unsigned char SuppressNotifiesDuringLoad;

		Joe* FASTCALL GimmeNext(Joe *Previous);
		Joe *GimmeNextGroup(Joe *Previous);
		Joe *GimmeNextSameLevel(Joe *Previous);
		unsigned long WriteTwig(FILE *SaveFile, Joe *SaveRoot);

		unsigned long LoadAttrib(FILE *LoadFile, Joe *LoadRoot,
		 unsigned long Number, unsigned long LIdx,
		 LayerEntry **LayerIndex, char Flop, unsigned short Version, unsigned short Revision, 
		 Project *CurProj, EffectsLib *CurEffects);
		unsigned long ParseEffectAttribute(char *JBIdx, Joe *LoadIn, char Flop, EffectsLib *CurEffects);

		unsigned long LoadGroup(FILE *LoadFile,
		 Joe *LoadRoot, unsigned long Depth, char Flop,
		 unsigned short Version, unsigned short Revision, Project *CurProj, EffectsLib *CurEffects);

		Joe *ParseJoe(char *JoeBuf, unsigned long JBSize,
		 char *StringBuf, char Flop, LoadStack *Tree,
		 unsigned long NLayers, LayerEntry **LayerIndex,
		 Joe *LoadRoot, Joe *LoadTarget, Joe *PrevJoe,
		 unsigned short Version, unsigned short Revision, Project *CurProj, EffectsLib *CurEffects);

		void ScanKill(Joe *Origin);

/*		struct DirList *V1AttemptLoadPoints(struct DirList *Dir,
		 struct DirList *Hot, Joe *Me);
		int V1ReadPoints(FILE *Input, Joe *MyJoe);

		void DeBlockLon(double *CoordBlock, VectorPoint *PList,
		 unsigned long Points, unsigned char Flop);
		void DeBlockLat(double *CoordBlock, VectorPoint *PList,
		 unsigned long Points, unsigned char Flop);
		void DeBlockElev(double *CoordBlock, VectorPoint *PList,
		 unsigned long Points, unsigned char Flop,
		 double &North, double &South, double &East, double &West);
*/
		Joe *InternalValidateJoe(RasterAnimHost *JoeRAH, unsigned long Flags);
		void RemoveUnusedLayers(void);


//	public:
		PointAllocator MasterPoint;
		LayerTable DBLayers;

		Database();
		~Database();
		int InitValid(void);
		void DestroyAll(void);
		void FreeVecSegmentData(void);
		unsigned long HowManyObjs(void);
		void GetBounds(double &North, double &South, double &East, double &West);
		void SetGeoClip(double North, double South, double East, double West);
		void ResetGeoClip(void);
		Joe *GeoClipMatch(Joe *Me);
		Joe *GetFirst(unsigned long Flags = NULL);
		Joe *GetNext(Joe *Previous = NULL);
		Joe *GetNextSameLevel(Joe *Previous = NULL);
		Joe *AddJoe(Joe *Me, unsigned long Flags, Project *CurProj);
		Joe *AddJoe(Joe *Me, Joe *MyParent, Project *CurProj);
		// These return the number of bytes read or saved.
		unsigned long SaveV2(FILE *SaveFile, unsigned long SaveFlags);
		unsigned long SaveV2(FILE *SaveFile, Joe *SaveRoot);
		unsigned long LoadV2(FILE *LoadFile, unsigned long LoadBytes, unsigned long LoadFlags,
			Project *CurProj, EffectsLib *CurEffects);
		unsigned long LoadV2(FILE *LoadFile, unsigned long LoadBytes, Joe *LoadRoot,
			Project *CurProj, EffectsLib *CurEffects);
		//unsigned long ImportWDB(FILE *fWDB, Joe *FileParent);
		//unsigned long ImportPPL(FILE *, Joe *);
		//unsigned long ImportGNIS(FILE *, Joe *);
		//unsigned long ImportCOUNTY(FILE *, Joe *);
		int ExportDXF(Project *Proj, JoeApproveHook *JAH, CoordSys *OutCoords, double ExportXYScale, double ExportZScale, int FlipLon = 0);
		int ExportShape(Project *Proj, JoeApproveHook *JAH, CoordSys *OutCoords, double ExportXYScale, double ExportZScale, long FlipLon = 0,
			long ViaSX = 0, long Force2D = 0, double AddElev = 0.0, RasterBounds *RBounds = NULL, long SXType = 0, char *xpath = NULL, char *xname = NULL);
		void PrintDXFNameOrLayer(FILE *fFile, Joe *Current);
		DEM *ObtainDEM(Joe *, unsigned int DownSample);
		void ReleaseDEM(DEM *);
		Joe *SetActiveObj(Joe *NewActive);
		Joe *SetFirstActive(void);
		inline Joe *GetActiveObj(void) {return(ActiveObj);};
		Joe *NewObject(Project *CurProj, char *NewNameStr = NULL);
		Joe *AddDEMToDatabase(char *PathName, char *FileName, Project *CurProj, EffectsLib *CurEffects);
		Joe *AddDEMToDatabase(char *SourceStr, char *BaseName, DEM *Topo, CoordSys *MyCoords, Project *CurProj, EffectsLib *CurEffects);
		Joe *FindByBestName(char *MatchName);

		// Recalculate bounds of cleared objects
		void BoundUpTree(Joe *Origin);
		void ReBoundTree(unsigned long DBFlags);
		void ReBoundTree(Joe *Origin);

		// Can be used to repair bounds of erroneous databases.
		// Somewhat time-intensive
		void MegaReBoundTree(unsigned long DBFlags);
		void MegaReBoundTree(Joe *Origin);

		// Update bounds of projected Joes
		void UpdateProjectedJoeBounds(void);

		#ifdef WCS_BUILD_V3
		long InitEffects(Project *CurProj, EffectsLib *Lib, EffectEval *EvalEffects, char *EffectEnabled, short ElevationOnly, double MetersPerDegLat);
		#else // WCS_BUILD_V3
		long InitEffects(Project *CurProj, EffectsLib *Lib, char *EffectEnabled, short ElevationOnly, double MetersPerDegLat);
		#endif // WCS_BUILD_V3
		RenderJoeList *CreateRenderJoeList(EffectsLib *Lib, long EffectClass);
		GeneralEffect *GetNextPointEffect(long EffectType, Joe *&Current, VectorPoint *&Point, double &Elev, double &Lat, double &Lon);
		double TerrainWidth(void);
		int GetDEMElevRange(float &MaxEl, float &MinEl);
		int GetDEMExtents(double &MaxLat, double &MinLat, double &MaxLon, double &MinLon);
		int GetMinDEMCellSizeMeters(double &CellSizeNS, double &CellSizeWE, Project *CurProj);
		unsigned long GetMinDEMCellSizeMetersMaxCount(double &CellSizeNS, double &CellSizeWE, unsigned long MaxCount, Project *CurProj);
		int FillDEMBounds(DEMBounds *MyBounds);
		long MatchCoordSys(CoordSys *TestCoords, int DEMOnly);
		long GetBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly);
		long GetNativeBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly, Project *CurProj);
		VectorPoint *NearestControlPointSearch(double Lat, double Lon, double PlanetRad);
		Joe *GetNextByQuery(SearchQuery *Search, Joe *Current);
		int AreThereVectors(void);

		// called by application when current time value changes
		void ApplicationSetTime(double Time, long Frame, double FrameFraction);

		// called by SceneView to delete an object
		void RemoveAll(Project *CurProj, EffectsLib *Effects, long ObjType);
		int RemoveRAHost(RasterAnimHost *RemoveMe, Project *CurProj, EffectsLib *Effects, int DoRemoveAll, int SuppressNitnoidRequest);

		Joe *ValidateJoe(RasterAnimHost *JoeRAH);

		// for use by scripts
		int SetJoeProperties(DBFilterEvent *DBFilter, JoeProperties *JoeProp);

		// for use by VNS2's Scenario loader and saver
		unsigned long EnumerateUniqueLoadSaveIDs(int ClearOnly);
		unsigned long CountHighestUniqueID(void);
		Joe **PopulateUniqueIDTable(unsigned long HighestID);
		void FreeUniqueIDTable(Joe **UniqueTable);

	}; // Database

// Flag bits for the different Database trees
#define WCS_DATABASE_STATIC		(1 << 0)
#define WCS_DATABASE_DYNAMIC	(1 << 1)
#define WCS_DATABASE_LOG		(1 << 2)
#define WCS_DATABASE_LOAD_CLEAR (1U << 31)
#define WCS_DATABASE_ALL		(WCS_DATABASE_STATIC | WCS_DATABASE_DYNAMIC | WCS_DATABASE_LOG)

// Types for Import
#define WCS_DATABASE_IMPORT_DLG			1
#define WCS_DATABASE_IMPORT_PPL			2
#define WCS_DATABASE_IMPORT_GNIS		3
#define WCS_DATABASE_IMPORT_COUNTY		4
#define WCS_DATABASE_IMPORT_DXF			5
#define WCS_DATABASE_IMPORT_WDB			6
#define	WCS_DATABASE_IMPORT_SHAPE		7
#define	WCS_DATABASE_IMPORT_MERIDIAN2	8

// ArcView Shape File types
#define WCS_ARCVIEW_SHAPETYPE_NULL			0
#define WCS_ARCVIEW_SHAPETYPE_POINT			1
#define WCS_ARCVIEW_SHAPETYPE_POLYLINE		3
#define WCS_ARCVIEW_SHAPETYPE_POLYGON		5
#define WCS_ARCVIEW_SHAPETYPE_MULTIPOINT	8
#define WCS_ARCVIEW_SHAPETYPE_POINTZ		11
#define WCS_ARCVIEW_SHAPETYPE_POLYLINEZ		13
#define WCS_ARCVIEW_SHAPETYPE_POLYGONZ		15
#define WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ	18
#define WCS_ARCVIEW_SHAPETYPE_POINTM		21
#define WCS_ARCVIEW_SHAPETYPE_POLYLINEM		23
#define WCS_ARCVIEW_SHAPETYPE_POLYGONM		25
#define WCS_ARCVIEW_SHAPETYPE_MULTIPOINTM	28
/*** the next type isn't handled ***/
#define WCS_ARCVIEW_SHAPETYPE_MULTIPATCH	31

// Used to handle recursion non-recursively in loading
class LoadStack
	{
	public:
		Joe *LevelParent;
		unsigned long AttribKids, GroupKids;
		inline LoadStack() {LevelParent = NULL; AttribKids = GroupKids = 0;};
	}; // LoadStack

// functions for writing DXF files
int PrintDXFSection(FILE *fFile, char *Section);
int PrintDXFTableType(FILE *fFile, char *TableType);
int PrintDXFIntValue(FILE *fFile, int GroupCode, int Value);
int PrintDXFFltValue(FILE *fFile, int GroupCode, double Value);
int PrintDXFTextValue(FILE *fFile, int GroupCode, char *Text);
int PrintDXFHdrTriple(FILE *fFile, char *Variable, int VtxNum, double *XYZ);
int PrintDXFTriple(FILE *fFile, int VtxNum, double X, double Y, float Z, CoordSys *OutCoords, double XYScale, double ZScale);
int PrintDXFEndSec(FILE *fFile);
int PrintDXFEndTab(FILE *fFile);
int PrintDXFSeqEnd(FILE *fFile);
int PrintDXFEndFile(FILE *fFile);
void CreateValidDXFString(char *dest, char *src);

#endif // WCS_DATABASE_H

