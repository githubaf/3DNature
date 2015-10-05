// Project.h
// Project Object
// Written from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_PROJECT_H
#define WCS_PROJECT_H

FILE *buffered_fopen(const char *a, const char *b);

#define fopen(a, b) buffered_fopen(a,b)

// These clever buggers intercept stdio calls and munge the path
// for you, thus adding assignlike capabilities to non-Amiga systems
// 021903: I think we're going to make them real functions now, to
// avoid the possibilities of multiple calls to function-type args passed
// to PROJ_fopen. -CXH

//#define PROJ_fopen(a, b) fopen((a) && (a)[0] ? GlobalApp->MainProj->MungPath(a): " ", b)
//#define PROJ_remove(a) remove(GlobalApp->MainProj->MungPath(a))
//#define PROJ_chdir(a) chdir(GlobalApp->MainProj->MungPath(a))
//#define PROJ_rename(a, b) rename(GlobalApp->MainProj->MungPath(a), GlobalApp->MainProj->MungPath(b, 1))
FILE *PROJ_fopen(const char *filename, const char *mode);
int PROJ_remove(const char *path);
int PROJ_rmdir(const char *path);
int PROJ_chdir(const char *dirname);
int PROJ_rename(const char *oldname, const char *newname);

int PROJ_mkdir(const char *path);

#define WCS_MAX_USER_WINDOWCONFIG	4
#define WCS_PROJECT_INIT_FENTRACK_SIZE 40
#define WCS_PROJECT_INIT_FENTRACK_INC  20
#define WCS_PROJECT_PARAM_SHORTS	14

#define WCS_PROJECT_PREFS_NAME_WCS "WCS.Prefs"
#ifdef WCS_BUILD_VNS
	#define WCS_PROJECT_PREFS_NAME "VNS.Prefs"
#else // !WCS_BUILD_VNS
	#define WCS_PROJECT_PREFS_NAME "WCS.Prefs"
#endif // !WCS_BUILD_VNS

#define WCS_PROJECT_CURRENT_VERSION		4
#define WCS_PROJECT_CURRENT_REVISION	0

#define WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX	32
#define WCS_PROJECT_PREFS_CONFIG_OPTIONS_LEN	80

// ProjectIODetail Flags
#define WCS_PROJECT_IODETAILFLAG_DESTROY		0x08000000		// high flag bit used to clear database before loading

// ChunkIODetail Flags & Styles
#define WCS_PROJECT_IODETAILFLAG_WHEREIS		0x00000000		// default operation
#define WCS_PROJECT_IODETAILFLAG_FIRSTAVAIL		0x00000001		// for ecosystem or color - ecotypes will be adjusted to match loaded positions. Items less than the fixed number for the group will be loaded where they are
#define WCS_PROJECT_IODETAILFLAG_NUMPROVIDED	0x00000002		// Item Number in UserData - for ecosystem or color. Items less than the fixed number for the group will be loaded where they are
#define WCS_PROJECT_IODETAILFLAG_ADDRPROVIDED	0x00000004		// Address in UserData - for loading ecotypes or loading and saving foliage groups only.
																// ItemList should contain the eco number that has the ecotype or foliage group when saving only. For loading ItemList should be NULL.
#define WCS_PROJECT_IODETAILFLAG_NOPATCH		0x00000008		// Don't patch from other versions to current - set this when loading only limited variables
#define WCS_PROJECT_IODETAILFLAG_LOADUNIQUE		0x00000010		// Don't duplicate colors. If a color name is already present don't load it

// ProjectIO and ChunkIODetail Tags
#define WCS_PROJECT_IODETAILTAG_CHUNKID			1
#define WCS_PROJECT_IODETAILTAG_GROUP			2
#define WCS_PROJECT_IODETAILTAG_NUMITEMS		3
#define WCS_PROJECT_IODETAILTAG_STYLE			4
#define WCS_PROJECT_IODETAILTAG_ITEM			5
#define WCS_PROJECT_IODETAILTAG_FLAGS			6
#define WCS_PROJECT_IODETAILTAG_USERDATA1		7
#define WCS_PROJECT_IODETAILTAG_USERDATA2		8
#define WCS_PROJECT_IODETAILTAG_DONE			~0

// Project preferred Units
// WCS_PROJPREFS_UNITS_METERS
// We are now using WCS_USEFUL_UNIT_METER and its ilk
// from Useful.h

enum
	{
	WCS_PROJPREFS_ANGLEUNITS_DECDEG = 0,
	WCS_PROJPREFS_ANGLEUNITS_DMS
	}; // Project preferred  Angle Units

enum
	{
	WCS_PROJPREFS_LONCONVENTION_POSWEST = 0,
	WCS_PROJPREFS_LONCONVENTION_POSEAST
	}; // Project preferred  longitude display convention

enum
	{
	WCS_PROJPREFS_LATLONSIGN_NUMERIC = 0,
	WCS_PROJPREFS_LATLONSIGN_ALPHABETIC
	}; // Project preferred geog coord display

enum
	{
	WCS_PROJPREFS_TIMEUNITS_SECS = 0,
	WCS_PROJPREFS_TIMEUNITS_FRAMES
	};

enum
	{
	WCS_PROJPREFS_TASKMODE_ALL = 0,
	WCS_PROJPREFS_TASKMODE_TERRAIN,
	WCS_PROJPREFS_TASKMODE_LANDCOVER,
	WCS_PROJPREFS_TASKMODE_WATER,
	WCS_PROJPREFS_TASKMODE_AIR,
	WCS_PROJPREFS_TASKMODE_LIGHT,
	WCS_PROJPREFS_TASKMODE_OBJECT,
	WCS_PROJPREFS_TASKMODE_VECTOR,
	WCS_PROJPREFS_TASKMODE_RENDER
	};

class Database;
class EffectsLib;
class ImageLib;
class InterCommon;
class Pier1;
class Project; // Heh heh heh... You get a B- for your Class Project in C++ pun

#include "Notify.h"
#include "Useful.h"
#include "PathAndFile.h"
#include "RasterAnimHost.h"
#include "ViewGUI.h"
#include "DBEditGUI.h"
#include <stdio.h>

struct DirList
	{
	struct DirList *Next;
	char Read;
	char Name[255];
	};

struct ProjWinInfo
	{
	unsigned long WinID, Flags;
	char PreferredCell[4];
	short X, Y, W, H;
	}; // ProjWinInfo

struct RootMarker
	{
	short X, Y;
	}; // RootMarker

// RootMarker Enums
enum
	{
	WCS_PROJECT_ROOTMARKER_TOOLBAR_BOTEDGE,
	WCS_PROJECT_ROOTMARKER_TOOLBAR_RIGHTEDGE, // no longer used/set
	WCS_PROJECT_ROOTMARKER_ANIMBAR_LR,
	WCS_PROJECT_ROOTMARKER_ANIMBAR_MIDDLE,
	WCS_PROJECT_ROOTMARKER_PROGRESS_UL,  // no longer used/set
	WCS_PROJECT_ROOTMARKER_PROGRESS_UR, // no longer used/set
	WCS_PROJECT_ROOTMARKER_LOGSTATUS_LEFTEDGE,
	WCS_PROJECT_ROOTMARKER_SAG_RIGHTEDGE,
	WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN,
	WCS_PROJECT_ROOTMARKER_MATRIX_SIZE,
	// template:
	// WCS_PROJECT_ROOTMARKER_,

	// This must be last! Don't access it!
	WCS_PROJECT_NUM_ROOTMARKERS
	}; // RootMarkers


// Cell codes
// A: 1/1 Fullscreen
// B: 2/3
// C: 1/2 Half, (also 4/9)
// D: 1/3 Third
// E: 1/4 Quarter (also 2/9)
// F: 1/6
#define WCS_PROJECT_WINLAYOUT_MAX_CELLCODES		6
#define WCS_PROJECT_WINLAYOUT_MAX_LAYOUTS		12
#define WCS_PROJECT_WINLAYOUT_MAX_WINDOWS		10
#define WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT	-1

#define WCS_PROJECT_WINLAYOUT_DIRECTION_ENDSPAN	0.0

#define WCS_PROJECT_WINLAYOUT_DIRECTION_END	0
#define WCS_PROJECT_WINLAYOUT_DIRECTION_COL	1
#define WCS_PROJECT_WINLAYOUT_DIRECTION_ROW	2

#define WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW	1


class MatrixPane
	{
	public:
		char PaneID[4];
		short OffsetX, OffsetY, Width, Height, Occupants;
		float OffsetXPct, OffsetYPct, WidthPct, HeightPct,
		 AreaFrac;

		//MatrixPane(float aOffsetXPct, float aOffsetYPct, float aWidthPct, float aHeightPct) {OffsetXPct = aOffsetXPct; OffsetYPct = aOffsetYPct; WidthPct = aWidthPct; HeightPct = aHeightPct;};
		MatrixPane() {OffsetX = OffsetY = Width = Height = 0; Occupants = 0; OffsetXPct = OffsetYPct = 0.0f; WidthPct = HeightPct = AreaFrac = 100.0f; PaneID[0] = PaneID[1] = PaneID[2] = NULL;};
		void MoveIn(void) {Occupants++;};
		void MoveOut(void) {if(Occupants)Occupants--;};
		short Inhabitants(void) {return(Occupants);};
	}; // MatrixPane

class WinLayout
	{
	public:
		long NumWindows;
		MatrixPane MPanes[WCS_PROJECT_WINLAYOUT_MAX_WINDOWS];
		double AvgSize;
		
		WinLayout() {NumWindows = 0; AvgSize = 0.0;};
		int InitLayout(char *CellIDs, ...);
		int PaneFromID(char *SearchID);
		int PaneFromXY(short Xc, short Yc);
	}; // WinLayout

class MatrixLayouts
	{
	public:
		WinLayout Matrices[WCS_PROJECT_WINLAYOUT_MAX_LAYOUTS];
		WinLayout Current;
		// More than we'll need on the second axis, but no way to know.
		unsigned long LayoutFlags[WCS_PROJECT_WINLAYOUT_MAX_CELLCODES][WCS_PROJECT_WINLAYOUT_MAX_WINDOWS];

		MatrixLayouts();
		double   GetAvgSize(int Layout)            {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.AvgSize)				: (Matrices[Layout].AvgSize));};
		int   GetNumPanes(int Layout)           {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.NumWindows)				: (Matrices[Layout].NumWindows));};
		short GetPaneX(int Pane)    {return(Current.MPanes[Pane].OffsetX);};
		short GetPaneY(int Pane)    {return(Current.MPanes[Pane].OffsetY);};
		short GetPaneW(int Pane)    {return(Current.MPanes[Pane].Width);};
		short GetPaneH(int Pane)    {return(Current.MPanes[Pane].Height);};
		float GetPercentX(int Layout, int Pane)    {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].OffsetXPct)	: (Matrices[Layout].MPanes[Pane].OffsetXPct));};
		float GetPercentY(int Layout, int Pane)    {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].OffsetYPct)	: (Matrices[Layout].MPanes[Pane].OffsetYPct));};
		float GetPercentW(int Layout, int Pane)    {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].WidthPct)		: (Matrices[Layout].MPanes[Pane].WidthPct));};
		float GetPercentH(int Layout, int Pane)    {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].HeightPct)	: (Matrices[Layout].MPanes[Pane].HeightPct));};
		float GetPercentArea(int Layout, int Pane) {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].AreaFrac)		: (Matrices[Layout].MPanes[Pane].AreaFrac));};
		char *GetPaneID(int Layout, int Pane)   {return(Layout == WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT ? (Current.MPanes[Pane].PaneID)		: (Matrices[Layout].MPanes[Pane].PaneID));};
		int GetPaneFromID(int Layout, char *PaneID);
		int GetPaneFromXY(short Xc, short Yc);
		void SetCurrent(int Layout);

		unsigned long TestFlag(char *CellCode, unsigned long FlagToTest);
		void SetFlag(const char *CellCode, unsigned long FlagToSet);
		void ClearFlag(const char *CellCode, unsigned long FlagToClear);

		void MoveIn(int Pane) {Current.MPanes[Pane].MoveIn();};
		void MoveOut(int Pane) {Current.MPanes[Pane].MoveOut();};
		short Inhabitants(int Pane) {return(Current.MPanes[Pane].Inhabitants());};
	}; // MatrixLayouts

#define WCS_MAX_TEMPLATES	15

class Template
	{
	public:
		PathAndFile PAF;
		char Enabled, Embed, Remove;

		Template()	{Enabled = 1; Embed = 0; Remove = 0;};
		void Clear(void);
		void Copy(Template *CopyFrom);
		ULONG Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
	}; // class Template

class NewProject
	{
	public:
		PathAndFile ProjName, CloneName;
		char Clone, CloneType, PlaceInSubDir, ProjectPathIncludesProjDir;
		Project *ProjHost;
		Database *DBHost;
		EffectsLib *EffectsHost;
		ImageLib *ImagesHost;

		NewProject(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, ImageLib *ImageSource);
		int Create(void);

	}; // class NewProject

#define WCS_INTERACTIVE_STYLE_LIGHTWAVE 0
#define WCS_INTERACTIVE_STYLE_MAX		1

#define WCS_LINESTYLE_SOLID		0
#define WCS_LINESTYLE_DASHED	1
#define WCS_LINESTYLE_DOTTED	2
#define WCS_LINESTYLE_BROKEN	3

class VectorExportData
	{
	public:
		double HorScale, VertScale, HorSize, VertSize, HorGridInterval, VertGridInterval, HorTicInterval, VertTicInterval,
			HorLabelInterval, VertLabelInterval;
		double VectorLineWeight, TerrainLineWeight, HorGridWeight, VertGridWeight, HorTicWeight, VertTicWeight,
			GraphOutlineWeight, HorTicLength, VertTicLength;
		char HorUnits, VertUnits, LayoutUnits, HorScaleLabel, VertScaleLabel, DrawVector, DrawTerrain, 
			DrawHorGrid, DrawVertGrid, DrawHorTics, DrawVertTics, DrawHorLabels, DrawVertLabels, DrawGraphOutline,
			VectorLineStyle, TerrainLineStyle, HorGridStyle, VertGridStyle, GraphOutlineStyle, LaunchIllustrator;
		char PreferredFont[256];
		unsigned char VectorColor[3], TerrainColor[3], HorGridColor[3], VertGridColor[3], HorTicColor[3], VertTicColor[3],
			HorLabelColor[3], VertLabelColor[3], GraphOutlineColor[3];
			PathAndFile PAF;

		VectorExportData();
		unsigned long Load(FILE *ffile, ULONG ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

	}; // VectorExportData

class ProjPrefsInfo
	{
	private:
		char *QueryConfigOpt(char *CO);
		bool QueryConfigOptTrue(char *CO);
		bool QueryConfigOptFalse(char *CO);

	public:

		short RenderTaskPri, RenderSize, PosLonHemisphere, VertDisplayUnits, HorDisplayUnits, AngleDisplayUnits,
			TimeDisplayUnits, LatLonSignDisplay, TaskMode, EnabledFilter, AnimatedFilter, ShowDBbyLayer,
			GUIConfiguration, SAGExpanded, RecordMode, InteractiveMode, KeyGroupMode, SignificantDigits, // RecordMode is currently unused and may be retired eventually 
			MatrixTaskModeEnabled, SAGBottomHtPct, InteractiveStyle, LastUTMZone, MultiUserMode, DisplayGeoUnitsProjected,
			NewProjUseTemplates, PaintDefaultsValid, MemoryLimitsEnabled, GlobalAdvancedEnabled;
		//short ReportMesg[4];
		short LoadOnOpen;
		short OpenWindows;
		short ProjShowIconTools, ProjShowAnimTools;
		long MaxSAGDBEntries, MaxSortedSAGDBEntries;
		long LastUpdateDate; // <<<>>> TIME64 issue -- should be time_t for future-proofing
		long GlobalStartupPage;
		unsigned long PaintMode, GradMode, ActiveBrush, Effect, VecPolyMemoryLimit, DEMMemoryLimit;
		unsigned char ViewEnabled[WCS_VIEWGUI_VIEWTYPE_MAX][WCS_VIEWGUI_ENABLE_MAX];
		char CurrentUserName[64], CurrentPassword[64], LastColorSwatch[64];
		double ViewContOverlayInt[WCS_VIEWGUI_VIEWTYPE_MAX], Tolerance, ForeElev, BackElev;
		float BrushScale, Opacity;
		VectorExportData VecExpData;
		Template DefaultTemplates[WCS_MAX_TEMPLATES];
		// Database Editor columns and filter
		unsigned short DBENumFixedColumns, DBEFixedColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX],
			DBENumLayerColumns, DBENumAttribColumns, DBELayerColumnWidths[WCS_DBEDITGUI_MAX_GRID_COLUMNS], 
			DBEAttribColumnWidths[WCS_DBEDITGUI_MAX_GRID_COLUMNS];
		unsigned char DBEFilterMode;
		char DBESearchQueryFilterName[WCS_EFFECT_MAXNAMELENGTH], DBELayerFilterName[WCS_EFFECT_MAXNAMELENGTH];
		
		char ConfigOptions[WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX][WCS_PROJECT_PREFS_CONFIG_OPTIONS_LEN];

		ProjPrefsInfo();

		int PublishConfigOpt(char *CO);
		int SetConfigOpt(char *CO, char *Val);
		void RemoveConfigOpt(char *CO);
		// ConfigOpts that are publicly documented
		char *PublicQueryConfigOpt(char *CO) {return(QueryConfigOpt(CO));};
		bool PublicQueryConfigOptTrue(char *CO) {return(QueryConfigOptTrue(CO));};
		bool PublicQueryConfigOptFalse(char *CO) {return(QueryConfigOptFalse(CO));};
		// ConfigOpts that shouldn't be documented
		char *PrivateQueryConfigOpt(char *CO) {return(QueryConfigOpt(CO));};
		bool PrivateQueryConfigOptTrue(char *CO) {return(QueryConfigOptTrue(CO));};
		bool PrivateQueryConfigOptFalse(char *CO) {return(QueryConfigOptFalse(CO));};
	}; // ProjPrefsInfo

struct ChunkIODetail
	{
	struct ChunkIODetail *Next;
	unsigned long  Flags;
	unsigned short Group;
	unsigned short NumItems;
	//unsigned short *StyleList;
	short          *ItemList;
	void           *UserData1, *UserData2;
	}; // DetailInfo

class Project : public SetCritter, public NotifyEx, public RasterAnimHost
	{
	//private:
	public:

	struct DirList *DL;
	ProjPrefsInfo Prefs;
	InterCommon *Interactive;
	Pier1 *Imports;
	MatrixLayouts ViewPorts;

	char str[256]; // Temp string, used all over the ^%$# place...
	char paramfile[32],
		framepath[256],temppath[256],dbasepath[256],parampath[256],
		path[256],dirname[256],dbasename[32],
		backgroundpath[256], backgroundfile[32],
		colormappath[256], colormapfile[32], projectpath[256],
		projectname[64], framefile[64], 
		cloudpath[256], cloudfile[32], wavepath[256], wavefile[32],
		deformpath[256], deformfile[32], imagepath[256], sunfile[32], moonfile[32],
		sunpath[256], moonpath[256], pcprojectpath[256], pcframespath[256],
		altobjectpath[256], animpath[256], animfile[64], savezbufpath[256], savezbuffile[32],
		contentpath[256], UserName[128], UserEmail[128], importdatapath[256],
		LastProject[6][512], ProjectPassword[64], AuxImportPath[512], AuxImportFile[256];

	unsigned short FenTrackSize, AltFenTrackSize[WCS_MAX_USER_WINDOWCONFIG], ProjectLoaded, PrefsLoaded;
	// Saved with project
	ProjWinInfo *FenTrack, *AltFenTrack[WCS_MAX_USER_WINDOWCONFIG];
	// Generated dynamically on startup/resize
	RootMarker RootMarkers[WCS_PROJECT_NUM_ROOTMARKERS];
	Template ProjectTemplates[WCS_MAX_TEMPLATES];

	public:
		Project();
		~Project();

		short LoadPrefs(char *ProgDir, char *PreferredFile = NULL);
		short LoadResumeState(char *ProgDir);
		short SavePrefs(char *ProgDir);
		void OpenWindows(void);
		int ValidateAssignPaths(void);
		struct ProjectIODetail *ProjectIODetail_New(void);
		struct ChunkIODetail *ChunkIODetail_New(void);
		struct ProjectIODetail *IODetail(int Flags, ...);
		void ProjectIODetail_Del(struct ProjectIODetail *This);
		struct ProjectIODetail *ProjectIODetailSearch(struct ProjectIODetail *This, char *Search);
		struct ChunkIODetail *ChunkIODetailSearch(struct ChunkIODetail *This, unsigned short Group);
		short ItemIODetailSearch(struct ChunkIODetail *This, short Item);
		int Load(RasterAnimHostProperties *fFileSupplied, char *LoadName, Database *DB, EffectsLib *Effects, ImageLib *Images, struct ProjectIODetail *Detail = NULL, unsigned long loadcode = 0xffffffff);
		int LoadProjectAsTemplate(char *LoadName, int Embed, Database *LoadToDB, EffectsLib *LoadToEffects, struct ProjectIODetail *Detail);
		unsigned long Paths_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, ImageLib *ImgLib, struct ChunkIODetail *Detail = NULL);
		unsigned long GUIConfig_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long UserGUIConfig_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long GUIDimensions_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, ProjWinInfo **FTPtr, unsigned short *FTSizePtr);
		unsigned long Prefs_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, struct ProjectIODetail *LocalDetail);
		unsigned long ProjectPrefs_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Templates_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, Database *LoadToDB, EffectsLib *LoadToEffects, struct ProjectIODetail *Detail);
		unsigned long Password_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ProjectIODetail *LocalDetail);
		unsigned long InterPrefs_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long ImportOps_Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Database *DB, EffectsLib *Effects);
		int Save(RasterAnimHostProperties *fFileSupplied, char *SaveName, Database *DB, EffectsLib *Effects, ImageLib *Images, struct ProjectIODetail *Detail = NULL, unsigned long savecode = 0xffffffff);
		unsigned long Summary_Save(FILE *ffile, EffectsLib *Effects);
		unsigned long Paths_Save(FILE *ffile);
		unsigned long GUIConfig_Save(FILE *ffile);
		unsigned long UserGUIConfig_Save(FILE *ffile);
		unsigned long GUIDimensions_Save(FILE *ffile, ProjWinInfo *FT, unsigned short FTSize);
		unsigned long Prefs_Save(FILE *ffile);
		unsigned long ProjectPrefs_Save(FILE *ffile);
		unsigned long Templates_Save(FILE *ffile);
		unsigned long Password_Save(FILE *ffile);
		unsigned long InterPrefs_Save(FILE *ffile);
		unsigned long ImportOps_Save(FILE *ffile);

		struct DirList *DirList_New(char *firstpath, short ReadOnly);
		struct DirList *DirList_Append(struct DirList *DLOld, struct DirList *DLNew);
		struct DirList *DirList_Add(struct DirList *DLOld, char *addpath, short ReadOnly);
		struct DirList *DirList_Remove(struct DirList *DLOld, short Item);
		struct DirList *DirList_Search(struct DirList *DLOld, short Item);
		short DirList_ItemExists(struct DirList *DLItem, char *Item);
		void DirList_Del(struct DirList *DLDel);
		void DirList_Move(struct DirList *DLOld, short Move, short MoveTo);
		struct DirList *DirList_Copy(struct DirList *DLOld);
		struct DirList *DirList_Mung(struct DirList *DLOld);
		struct DirList *DirList_UnMung(struct DirList *DLOld);

		int FindWinID(unsigned long WinID);
		void InquireWindowCoords(unsigned long WinID, short &X,
 			short &Y, short &W, short &H);
		void SetWindowCoords(unsigned long WinID, short X,
 			short Y, short W, short H, ProjWinInfo **FTPtr = NULL, unsigned short *FTSizePtr = NULL);
		void SetWindowCell(unsigned long WinID, char *WinCell, short X,
 			short Y, short W, short H, ProjWinInfo **FTPtr = NULL, unsigned short *FTSizePtr = NULL);
		void InquireWindowCell(unsigned long WinID, char *WinCell);
/*		void SetWindowCoords(unsigned long WinID, short X,
 			short Y, short W, short H, ProjWinInfo **FTPtr = NULL, unsigned short *FTSizePtr = NULL); */
		void InquireWindowFlags(unsigned long WinID, unsigned long &Flags);
		void SetWindowFlags(unsigned long WinID, unsigned long Flags, short Operation = 0, ProjWinInfo **FTPtr = NULL, unsigned short *FTSizePtr = NULL);
		short InquireRenderPri(void) {return(Prefs.RenderTaskPri);};

		void SetRootMarkerQuiet(unsigned long MarkerNum, short X, short Y) {RootMarkers[MarkerNum].X = X; RootMarkers[MarkerNum].Y = Y;};
		void SetRootMarker(unsigned long MarkerNum, short X, short Y) {RootMarkers[MarkerNum].X = X; RootMarkers[MarkerNum].Y = Y; Relayout();};
		short InquireRootMarkerX(unsigned long MarkerNum) {return(RootMarkers[MarkerNum].X);};
		short InquireRootMarkerY(unsigned long MarkerNum) {return(RootMarkers[MarkerNum].Y);};
		void Relayout(void);

		void SetRenderPri(short NewPri) {Prefs.RenderTaskPri = NewPri;};

		char *MungPath(const char *Name, int WhichName = 0);
		char *UnMungPath(char *Name);
		char *UnMungNulledPath(char *Name);

		void SetParam(int Notify, ...);
		void GetParam(void *Value, ...);

		void SetFRDInvalid(short Invalid);
		int VerifyProjectLoaded(void);
		void ApplicationSetTime(double Time, long Frame, double FrameFraction);

		short ShowIconTools(void) {return (Prefs.ProjShowIconTools);};
		short ShowAnimTools(void) {return (Prefs.ProjShowAnimTools);};

		short GetKeyGroupMode(void)	{return (Prefs.KeyGroupMode);};

		void DeleteFromRecentProjectList(unsigned long projNum);
		void UpdateRecentProjectList(void);
		Pier1 *AddPier(Pier1 *NewPier);
		void RemovePier(Pier1 *RemoveMe);
		void ClearPiers();
		void ClearTemplates(void);
		void SetAuxAutoImportFile(char *AuxPath, char *AuxFile);

		// more well-defined access to project, frames and content paths
		// These do not generate any notifications, and the Project Prefs window
		// may not even listen to any such notifies, so be aware that these will not
		// cause the ProjPrefsGUI to update.
		void SetProjectPath(char *NewProjectPath) {strcpy(pcprojectpath, NewProjectPath);};
		void SetFramesPath(char *NewFramesPath) {strcpy(pcframespath, NewFramesPath);};
		void SetContentPath(char *NewContentPath) {strcpy(contentpath, NewContentPath);};

	private:
		short ParamData[WCS_PROJECT_PARAM_SHORTS];

	public:
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_PROJECT);};

		// inherited from RasterAnimHost
		virtual char *GetRAHostName(void)						{return (projectname);};
		virtual char *GetRAHostTypeString(void)					{return ("(Project)");};
		//virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		//virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual unsigned char GetNotifyClass(void)				{return (WCS_RAHOST_OBJTYPE_PROJECT);};
		virtual unsigned char GetNotifySubclass(void)			{return (WCS_RAHOST_OBJTYPE_PROJECT);};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		//virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);

	}; // Project


#endif // WCS_PROJECT_H
