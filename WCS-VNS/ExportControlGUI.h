// ExportControlGUI.h
// Header file for ExportControlGUI
// Created from scratch on 4/23/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"

#ifndef WCS_EXPORTCONTROLGUI_H
#define WCS_EXPORTCONTROLGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "RenderInterface.h"

class EffectsLib;
class ImageLib;
class Database;
class Project;
class Renderer;
class RealTimeFoliageWriteConfig;
class NameList;
class ExportFormat;
class Object3DInstance;
class RealtimeFoliageCellData;
class RealtimeFoliageIndex;

// Very important that we list the base classes in this order.
class ExportControlGUI : public RenderInterface, public WCSModule, public GUIFenetre
	{
	public:
		EffectsLib *EffectsHost;
		ImageLib *ImageHost;
		Database *DBHost;
		Project *ProjectHost;
		SceneExporter *ActiveJob;
		RealTimeFoliageWriteConfig *RTFWriteConfig;
		ExportFormat *ExportFmt;
		Object3DInstance *ObjectInstanceList;

		Renderer *Rend;
		char AText[200], FText[200], PText[200], PauseText[200],
			IText[200], RText[200], FRText[200], ProgCap[200], PrevCap[200],
			FrameTitle[200],
			Preview, Pause, Run, FrdWarned;
		unsigned long PMaxSteps, PCurSteps;
		unsigned long FMaxSteps, FCurSteps;
		time_t FStartSeconds, AStartSeconds;
		unsigned long AMaxSteps, ACurSteps;

		char ECPWDate[40], ECToday[40], ECProjDate[40];

		int ConstructError, Rendering;

		ExportControlGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource);
		~ExportControlGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);

		void ConfigureJobWidgets(void);
		void BuildJobList(void);
		void BuildJobListEntry(char *ListName, SceneExporter *Me);
		void DisableWidgets(void);
		void EnableJob(void);
		void DisableJob(void);
		void ChangeJobPriority(short Direction);
		void EditJob(void);
		void SetActiveJob(void);
		void PriorityLevel(void);
		void ExportGo(void);
		void ExportStop(void);
		int JobTestSettings(SceneExporter *CurJob);
		int ExportAJob(SceneExporter *CurJob);
		int ExportTerrain(SceneExporter *CurJob, NameList **FileNamesCreated);
		int RenderTerrain(SceneExporter *CurJob, RenderJob *CurRJ);
		int ExportSky(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportVectors(SceneExporter *CurJob, NameList **FileNamesCreated);
		int Export3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportObjects(SceneExporter *CurJob, NameList **FileNamesCreated, Object3DInstance **ObjectList);
		int Export3DFoliageObjects(SceneExporter *CurJob, NameList **FileNamesCreated, Object3DInstance **ObjectList);
		int RenderExportObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		int RenderObjects(SceneExporter *CurJob, RenderJob *CurRJ, NameList **FileNamesCreated);
		int StripObjectsFromFoliageFiles(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportWalls(SceneExporter *CurJob, NameList **FileNamesCreated);
		int RenderWalls(SceneExporter *CurJob, RenderJob *CurRJ, NameList **FileNamesCreated);
		int RenderWallSegments(SceneExporter *CurJob, Renderer *Rend, Object3DEffect *Object3D, 
			FenceVertexList *ListElement, long NumWallSegs, double WallHeight, int FlipNormals, int SpanMatNum, NameList **FileNamesCreated);
		int RenderRoofSegments(SceneExporter *CurJob, Renderer *Rend, Object3DEffect *Object3D, 
			FenceVertexList *ListElement, long NumRoofPolys, double RoofWidth, double RoofHeight, int RoofMatNum, NameList **FileNamesCreated);
		int ExportFoliageImages(SceneExporter *CurJob, NameList **FileNamesCreated);
		void FindOptimalFoliageRes(SceneExporter *CurJob, long SourceRows, long SourceCols, long &DestRows, long &DestCols);
		int ExportFoliageInstancesAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportFoliageAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated, int AlignToCamera,
			int SingleObjectAtOrigin);
		int ExportFoliageAsAligned3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportFoliageAsNonAligned3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		FILE *PrepareFoliageFileForWalkthrough(SceneExporter *CurJob, RealtimeFoliageIndex *Index, 
			RealtimeFoliageCellData *RFCD, NameList **FileNamesCreated, long *CountPos = NULL, char *Mode = NULL);
		FILE *OpenFoliageIndexFile(SceneExporter *CurJob, NameList **FileNamesCreated, char *Mode);
		void PurgeFoliageFiles(SceneExporter *CurJob, NameList **FileNamesCreated);
		int SearchForExportableFoliage(FILE *FolDataFile, SceneExporter *CurJob, RealtimeFoliageIndex *Index, 
			Raster *SearchRast, int &IsLabel);
		int PrepExportItems(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ResampleExports(SceneExporter *CurJob, NameList **FileNamesCreated);
		void ParseTileNumbers(const char *FileName, long &CurTileY, long &CurTileX);
		int ZipItAndShipIt(SceneExporter *CurJob, NameList **FileNamesCreated);
		int SaveBuffer(SceneExporter *CurJob, RenderOpt *Opt, RasterBounds *RBounds, char *FileType, char **BuffersToSave, 
			void **Buffers, char *OutPath, char *OutFile, long Frame, long Height, long Width, int WorldFile,
			float MaxTerrainElev, float MinTerrainElev);
		int ValidateExportPaths(SceneExporter *CurJob);
		NameList *AddNewNameList(NameList **Names, char *NewName, long FileType);
		NameList *RemoveNameList(NameList **Names, char *RemoveName, long FileType);
		ExportFormat *AllocExportFormat(char *Target, SceneExporter *CurJob);
		void DeleteExportFormat(void);
		void DeleteObjectInstanceList(void);
		int ExportTerrainAs3DObject(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportSkyAs3DObject(SceneExporter *CurJob, NameList **FileNamesCreated);
		int ExportVectorsAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated);
		void ExportError(char *Title, char *Mesg);
		int ConvertWCSGeoToExportCoords(SceneExporter *CurJob, NameList **FileNamesCreated);
		int CheckAlphaWorthwhile(unsigned char *AABuf, long BufSize);
		static int FormatSpecificAuth(char *TargetName);

		void SyncTexts(void);
		void UpdatePreviewState(void);
		void DoUpdate(unsigned long Step, unsigned long MaxSteps,
			time_t StartSeconds, unsigned long &CurSteps, int Rem, int Elap, int Comp, int GaugeID);
		int CheckAbort(void);
		void ECRepaintFuelGauge(unsigned long CurSteps, unsigned long MaxSteps, int GaugeID);
		Renderer *GetRenderHandle(void)	{return (Rend);};

		virtual void GUIGoModal(void) {GoModal();};
		virtual void GUIEndModal(void) {EndModal();};
		virtual void SetPreviewCheck(int State);
		virtual NativeGUIWin GetGUINativeWin(void)	{return (NativeWin);};
		virtual Renderer *GetRenderer(void)	{return (Rend);};
		virtual int GetPreview(void)	{return (Preview);};
		virtual int GetPause(void)	{return (Pause);};
		virtual int IsRunning(void) {return(Run);};
		virtual void SetRenderer(Renderer *NewRend)	{Rend = NewRend;};
		virtual void SetPreview(char PreviewOn);
		virtual void SetPause(char State)	{Pause = State;};
		virtual void SetRunning(char RunState)	{Run = RunState;};

		virtual void UpdateStamp(void);
		virtual void UpdateLastFrame(void);
		virtual void ClearAll(void);

		virtual void AnimInit(unsigned long Steps, char *Text);
		virtual void AnimUpdate(unsigned long NewSteps, char *Text = NULL);
		virtual void AnimClear(void);
		virtual void SetAnimText(char *Text);

		virtual void FrameTextInitA(char *Text);
		virtual void FrameTextClearA(void);
		virtual void SetFrameTextA(char *Text);
		virtual void FrameTextInit(char *Text)	{return;};
		virtual void FrameTextClear(void)	{return;};
		virtual void SetFrameText(char *Text)	{return;};
		virtual void SetFrameNum(char *FrameNum);
		virtual void GetFrameSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, time_t &StashStartSecs, char *StashText);
		virtual void RestoreFrameSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, time_t StashStartSecs, char *StashText);
		virtual void StashFrame(unsigned long FrameTime);

		virtual void FrameGaugeInit(unsigned long Steps);
		virtual void FrameGaugeUpdate(unsigned long NewSteps);
		virtual void FrameGaugeClear(void);

		virtual void ProcInit(unsigned long Steps, char *Text);
		virtual void ProcUpdate(unsigned long NewSteps, char *Text = NULL);
		virtual void ProcClear(void);
		virtual void SetProcText(char *Text);
		virtual void GetProcSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, char *StashText);
		virtual void RestoreProcSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, char *StashText);

		virtual void SetImageText(char *Text);
		virtual void SetResText(char *Text);
		virtual void SetFractText(char *Text);

		// Byte and Float data types are currently supported.
		int ResampleRaster(const char *InputPath, const char *OutputPath, int BandType, long InRows, long InCols,
			long OutRows, long OutCols, float NullValue = 0.0, int HonorNull = 0);
		int ResampleRasterTile(const char *InputPath, const char *OutputPath, int BandType, long InRows, long InCols,
			long OutRows, long OutCols, long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap, float NullValue = 0.0, int HonorNull = 0);

	}; // class ExportControlGUI

enum
	{
	WCS_EXPORTCONTROL_FILETYPE_RAWELEV,		// elevations stored as a floating point raw array, image orientation
	WCS_EXPORTCONTROL_FILETYPE_TEX1,		// texture image, usually the terrain, sometimes with foliage burned in
	WCS_EXPORTCONTROL_FILETYPE_TEX2,		// texture image, usually the foliage, sometimes with terrain too
	WCS_EXPORTCONTROL_FILETYPE_TEX1_RED,		// texture image, usually the terrain, sometimes with foliage burned in
	WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN,		// texture image, usually the terrain, sometimes with foliage burned in
	WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU,		// texture image, usually the terrain, sometimes with foliage burned in
	WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA,		// texture image, usually the terrain, sometimes with foliage burned in
	WCS_EXPORTCONTROL_FILETYPE_TEX2_RED,		// texture image, usually the foliage, sometimes with terrain too
	WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN,		// texture image, usually the foliage, sometimes with terrain too
	WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU,		// texture image, usually the foliage, sometimes with terrain too
	WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA,		// texture image, usually the foliage, sometimes with terrain too
	WCS_EXPORTCONTROL_FILETYPE_WORLD,		// world file for texture image
	WCS_EXPORTCONTROL_FILETYPE_PRJ,			// .prj file for texture image if supported
	WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX,	// index file for foliage files
	WCS_EXPORTCONTROL_FILETYPE_FOLFILE,		// realtime foliage list file
	WCS_EXPORTCONTROL_FILETYPE_SKYNORTH,	// north of sky cube, east at right, up at top
	WCS_EXPORTCONTROL_FILETYPE_SKYEAST,		// east of sky cube, south at right, up at top
	WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH,	// south of sky cube, west at right, up at top
	WCS_EXPORTCONTROL_FILETYPE_SKYWEST,		// west of sky cube, north at right, up at top
	WCS_EXPORTCONTROL_FILETYPE_SKYTOP,		// top of sky cube, north at bottom of image, east at right
	WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM,	// bottom of sky cube, north at top of image, east at right
	WCS_EXPORTCONTROL_FILETYPE_WALLTEX,		// texture image for a wall element
	WCS_EXPORTCONTROL_FILETYPE_ROOFTEX,		// texture image for a wall roof
	WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX,	// texture for a 3D Object
	WCS_EXPORTCONTROL_FILETYPE_WALL,		// wall object in .3ds format with UV coordinates
	WCS_EXPORTCONTROL_FILETYPE_3DOBJ,		// 3D Object in .3ds format with UV coordinates
	WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN,	// Final exported terrain model, there may be more than one
	WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX,		// Resampled and alpha-embedded foliage texture image
	WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECTIDX,	// SXQueryItem's object index file
	WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECT,		// SXQueryItem's object file
	WCS_EXPORTCONTROL_FILETYPE_QUERYACTIONIDX,	// SXQueryAction's action index file
	WCS_EXPORTCONTROL_FILETYPE_QUERYACTION		// SXQueryAction's action file
	}; // file types

struct WallPolygonPair
	{
	Polygon3D *Poly1, *Poly2;
	double SegLen, StartX;
	long RowNum, ColNum, PairPlaced, LastSeg;
	}; //struct WallPolygonPair

int CompareWallPolygonPair(const void *elem1, const void *elem2);

struct ObjPolySort
	{
	Polygon3D *Poly;
	long PolyNum;
	float AxisDist;
	}; // struct ObjPolySort

int CompareObjPolySort(const void *elem1, const void *elem2);

#endif // WCS_EXPORTCONTROLGUI_H
