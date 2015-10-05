// ViewGUI.h
// Built from scratch and from ragged chunks of MapViewGUI.h/CamViewGUI.h
// Started on 10/22/99 (four days to my 28th birthday!) by CXH
// Copyright 1996-1999

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_VIEWGUI_H
#define WCS_VIEWGUI_H

class ViewPrefsGUI;
class ViewRTPrefsGUI;
class ViewContext;
class ViewGUI;
class JoeViewTemp;

#include "Application.h"
#include "Fenetre.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Render.h"
#include "Joe.h"
#include "Realtime.h"

#define WCS_VIEWGUI_VIEWS_OPEN_MAX	32
#define WCS_VIEWGUI_VIEWS_CAMSINPOPUP_MAX	99
#define WCS_VIEWGUI_VIEWS_ROPTINPOPUP_MAX	99

enum
	{
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CTRLPTS,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_3DOBJ,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ATFX,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CMAP,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ECOSYS,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_ENVIRON,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_FOLIAGE,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_GROUND,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_LAKE,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SHADOWS,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_SNOW,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_STREAM,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_TFX,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_WAVES,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_VECTORS_CLOUDS,
	WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX
	}; // Global DisplayLists

/*
enum
	{
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_CAMSETUP,
	WCS_VIEWGUI_LOCAL_DISPLAYLIST_MAX
	}; // ViewLocal DisplayLists
*/

enum
	{
	WCS_VIEWGUI_ENABLE_RENDERPREV,
	WCS_VIEWGUI_ENABLE_CONTROLPOINTS,
	WCS_VIEWGUI_ENABLE_MEASURES,
	WCS_VIEWGUI_ENABLE_ACTIVEOBJ,
	WCS_VIEWGUI_ENABLE_DEMEDGE,
	WCS_VIEWGUI_ENABLE_TERRAIN,
	WCS_VIEWGUI_ENABLE_TERRAIN_TRANS,
	WCS_VIEWGUI_ENABLE_TERRAFX,
	WCS_VIEWGUI_ENABLE_AREATERRAFX,
	WCS_VIEWGUI_ENABLE_SNOW,
	WCS_VIEWGUI_ENABLE_FOLIAGE,
	WCS_VIEWGUI_ENABLE_FOLIAGEFX,
	WCS_VIEWGUI_ENABLE_ECOFX,
	WCS_VIEWGUI_ENABLE_CMAPS,
	WCS_VIEWGUI_ENABLE_LAKES,
	WCS_VIEWGUI_ENABLE_STREAMS,
	WCS_VIEWGUI_ENABLE_WAVES,
	WCS_VIEWGUI_ENABLE_3DOBJ,
	WCS_VIEWGUI_ENABLE_PLAINVEC,
	WCS_VIEWGUI_ENABLE_SKY,
	WCS_VIEWGUI_ENABLE_ATMOS,
	WCS_VIEWGUI_ENABLE_CLOUDS,
	WCS_VIEWGUI_ENABLE_CELEST,
	WCS_VIEWGUI_ENABLE_GROUND,
	WCS_VIEWGUI_ENABLE_TERRAINSHADOWS,
	WCS_VIEWGUI_ENABLE_3DOBJSHADOWS,
	WCS_VIEWGUI_ENABLE_LIGHTS,
	WCS_VIEWGUI_ENABLE_CAMERAS,
	WCS_VIEWGUI_ENABLE_TARGETS,
	WCS_VIEWGUI_ENABLE_SAFEAREA,
	WCS_VIEWGUI_ENABLE_LTDREGION,
	WCS_VIEWGUI_ENABLE_EXPORTERS, // not previously used
	WCS_VIEWGUI_ENABLE_GRADREPEAT,
	WCS_VIEWGUI_ENABLE_DUMMY2, // retired, used to be WCS_VIEWGUI_ENABLE_VIEW_PLUS_TASK
	WCS_VIEWGUI_ENABLE_DUMMY3, // retired, used to be WCS_VIEWGUI_ENABLE_REND_PLUS_TASK
	WCS_VIEWGUI_ENABLE_GRAD_GREY,
	WCS_VIEWGUI_ENABLE_GRAD_EARTH,
	WCS_VIEWGUI_ENABLE_GRAD_PRIMARY,
	WCS_VIEWGUI_ENABLE_OVER_CONTOURS,
	WCS_VIEWGUI_ENABLE_OVER_SLOPE,
	WCS_VIEWGUI_ENABLE_OVER_RELEL,
	WCS_VIEWGUI_ENABLE_OVER_FRACTAL,
	WCS_VIEWGUI_ENABLE_OVER_ECOSYS,
	WCS_VIEWGUI_ENABLE_OVER_RENDER,
	WCS_VIEWGUI_ENABLE_CURSOR,
	WCS_VIEWGUI_ENABLE_POLYEDGE,
	WCS_VIEWGUI_ENABLE_SAFETITLE,
	WCS_VIEWGUI_ENABLE_SAFEACTION,
	WCS_VIEWGUI_ENABLE_RTFOLIAGE,
	WCS_VIEWGUI_ENABLE_RTFOLFILE,
	WCS_VIEWGUI_ENABLE_WALLS, // seems like this slot needs to be occupied even if RT Walls aren't supported
	WCS_VIEWGUI_ENABLE_MAX
	}; // View Enable Options


enum
	{
	WCS_VIEWGUI_MANIP_NONE = 0,
	WCS_VIEWGUI_MANIP_MOVE,
	WCS_VIEWGUI_MANIP_ROT,
	WCS_VIEWGUI_MANIP_SCALE
	}; // Manipulation modes

enum
	{
	WCS_VIEWGUI_MANIPSTATE_OBJECT = 0,
	WCS_VIEWGUI_MANIPSTATE_VIEW,
	WCS_VIEWGUI_MANIPSTATE_VIEW_STUCK
	}; // Manipulation modes

enum
	{
	WCS_VIEWGUI_VIEWTYPE_PERSPECTIVE = 0,
	WCS_VIEWGUI_VIEWTYPE_OVERHEAD,
	WCS_VIEWGUI_VIEWTYPE_PLANIMETRIC,
	WCS_VIEWGUI_VIEWTYPE_MAX
	}; // View types

// flag bits for ViewContext::QueryViewState()

#define WCS_VIEWGUI_VIEWSTATE_PANVIEW		(1 << 0)
#define WCS_VIEWGUI_VIEWSTATE_ROTVIEW		(1 << 1)
#define WCS_VIEWGUI_VIEWSTATE_ZOOMVIEW		(1 << 2)
#define WCS_VIEWGUI_VIEWSTATE_ZOOMBOX		(1 << 3)
#define WCS_VIEWGUI_VIEWSTATE_RENDERPREV	(1 << 4)
#define WCS_VIEWGUI_VIEWSTATE_CONSTRAIN		(1 << 5)

#define WCS_VIEWGUI_VIEWSTATE_VIEWMANIP (WCS_VIEWGUI_VIEWSTATE_PANVIEW | WCS_VIEWGUI_VIEWSTATE_ROTVIEW | WCS_VIEWGUI_VIEWSTATE_ZOOMVIEW)


#define WCS_VIEWGUI_SELBUF_SIZE 64

// Very important that we list the base classes in this order.
class ViewPrefsGUI : public WCSModule, public GUIFenetre
	{
	friend class ViewGUI;
	private:
		static char *TabNames[6];
		static long ActivePage;
		int ActiveViewNum, PrevActiveView;
		void ShowLine(int Line, int Show);
		void ShowLines(int Lines);
		void EnableChecks(int Line, int Real, int Rend);
		void DisableRendCheck(int Line);
		void SetChecks(int Line, int Real, int Rend);
		void SetupLine(int Line, char *Name, int Real, int Rend);
		void SetupLabel(int Line, char *Name);
		void ClearLabels(void);
		void SetupReal(int Line, int EnableItem);
		void SetupRend(int Line, void *Item);

	public:

		int ConstructError;

		ViewPrefsGUI();
		~ViewPrefsGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);

		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DoKeep(void);
		void DoCancel(void);

		void DoView(int ViewNum);
		void DoActiveChange(void);
		static void SetActivePage(long NewPage) {ActivePage = NewPage;};
		void SetActiveView(int NewView) {if(NewView != ActiveViewNum){PrevActiveView = ActiveViewNum; ActiveViewNum = NewView;}};
		int GetActiveView(void) {return(ActiveViewNum);};
		int GetPrevView(void) {return(PrevActiveView);};

	}; // class ViewPrefsGUI


// Very important that we list the base classes in this order.
class ViewRTPrefsGUI : public WCSModule, public GUIFenetre
	{
	friend class ViewGUI;
	private:
		static char *TabNames[2];
		static long ActivePage;

	public:

		int ConstructError;

		ViewRTPrefsGUI();
		~ViewRTPrefsGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);

		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DoKeep(void);
		void DoCancel(void);

		void DoView(int ViewNum);
		void DoLoad(void);
		void DoActiveChange(void);
		static void SetActivePage(long NewPage) {ActivePage = NewPage;};

	}; // class ViewRTPrefsGUI


// The following fields in a ViewContext should be saved/restored:
// VCamera, RO, Enabled, VCWinID, VPID
// VCWinID and VPID are not necessarily valid at all times, they should be
// fetched from the equivalent GLDrawingFenetre (GLDF->GetVPID(), GLDF->FenID)
// before saving, and will be used directly after reload. 

class ViewContext
	{
	friend class ViewGUI;
	friend class ViewPrefsGUI;
	friend class ViewRTPrefsGUI;
	friend class ExportFormatWCSVNS;
	friend double CalcExag(double Elev, ViewContext *VC);
	private:
		VertexDEM LocalOrigin, CamPos;
		Point3d LiveCamPos, LiveTargetPos;
		Point3d NegOrigin; // Negated cartesian origin, used for translating
		Point3d GLNegOrigin; // Gl-notation negated cartesian origin, used for glTranslated
		Point3d SceneBoundsPos, SceneBoundsNeg; // Bounding cube of scene
		double SceneLow, SceneHigh;
		double LocalTime, GlobeRadius;
		double NearCeles, FarCeles;
		double VFOV, PixAspect, NearClip, FarClip;
		double FrustumLeft, FrustumRight, FrustumTop, FrustumBottom;
		char VCNum;
		Camera *VCamera;
		RenderOpt *RO;
		Sky *VSky;
		PlanetOpt *Planet;
		Renderer *BigR;
		unsigned char Enabled[WCS_VIEWGUI_ENABLE_MAX];
		unsigned char ShowRender;
		GLdouble GLModelMatrix[16], GLProjMatrix[16];
		GLint GLViewport[4];
		long LastSampleX, LastSampleY;
		double RenderRegion[4]; // 0.0 ... 1.0
		double PlanDegPerPix;
		AnimDoubleTime ContourOverlayInterval;
		unsigned long ViewState;
		unsigned long VCWinID;
		char VPID[4];
	public:
		char Alive;
		ViewContext();
		~ViewContext();
		void SetAlive(char State) {Alive = State;};
		void SetEnabled(int Item, unsigned char State) {Enabled[Item] = State;};
		void ToggleEnabled(int Item) {Enabled[Item] = !Enabled[Item];};
		void EnableGradOverlay(int Which, int State = 1);
		void SetTime(double NewTime) {LocalTime = NewTime;};
		void SetCamera(Camera *NewCam) {VCamera = NewCam; SetAlive(1);};
		void SetSky(Sky *NewSky) {VSky = NewSky;};
		void SetRenderOpt(RenderOpt *NewRO) {RO = NewRO;};
		void SetPlanet(PlanetOpt *NewPlanet);
		void SetOrigin(double Lat, double Lon, double Elev);
		void SetOrigin(Point3d NewOrigin){SetOrigin(NewOrigin[0], NewOrigin[1], NewOrigin[2]);};
		void CalcWorldBounds(Database *SceneDB);
		Point3d *GetOriginCart(void) {return(&LocalOrigin.XYZ);};
		double OriginLat(void)  const {return(LocalOrigin.Lat);};
		double OriginLon(void)  const {return(LocalOrigin.Lon);};
		double OriginElev(void) const {return(LocalOrigin.Elev);};
		Camera *GetCamera(void) const {return(VCamera);};
		void FetchCamCoord(double TheTime);
		void FetchCurCamCoord(void);
		double CamLat(void)  const {return(CamPos.Lat);};
		double CamLon(void)  const {return(CamPos.Lon);};
		double CamElev(void) const {return(CamPos.Elev);};
		double CamX(void)  const {return(CamPos.XYZ[0]);};
		double CamY(void)  const {return(CamPos.XYZ[1]);};
		double CamZ(void) const {return(CamPos.XYZ[2]);};
		double GetRadius(void) const {return(GlobeRadius);};
		Sky *GetSky(void) const {return(VSky);};
		RenderOpt *GetRenderOpt(void) const {return(RO);};
		char GetAlive(void) const {return(Alive == 1);};
		char GetEnabled(int Item) const {return(Enabled[Item] ? 1 : 0);};
		char GetVCNum(void) const {return(VCNum);};
		// retired
		//char GetEnabledTask(int Item);
		char CheckRendererPresent(void) const {return(BigR ? 1 : 0);};
		Renderer *FetchRenderer(void) const {return(BigR);};
		bool DismissRendererIfPresent(bool AskRetainDiagnosticsFirst);
		void ShowRendererIfPresent(void);
		void FetchGLProjection(void);
		void GLProject  (Point3d WorldXYZ, Point3d ScreenXYZ);
		void GLUnProject(Point3d WorldXYZ, Point3d ScreenXYZ);
		int IsPlan(void) const {return(VCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);};
		inline double VCCalcExag(double Elev);
		inline double VCUnCalcExag(double Elev);
		int CheckCamValid(void);
		int CheckROValid(void);
		int CheckSkyValid(void);
		int CheckImportantValid(void) {return(CheckCamValid() && CheckROValid());}; // cam and RO
		CoordSys *GetProjected(void) const {if(VCamera && VCamera->Projected && VCamera->Coords) return(VCamera->Coords); else return(NULL);};
		unsigned long QueryViewState(void);

		// state transfer and persistance
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);

	}; // ViewContext



//#define DoDisplayList(ListNum) {if(ListGenInProgress){glNewList((ListNum), GL_COMPILE);DisplayLists[(ListNum)] = 1;}else if(DisplayLists[ListNum]){glCallList(ListNum);return;}};

class ViewGUI : public WCSModule
	{
	friend class ViewPrefsGUI;
	friend class ViewRTPrefsGUI;
	friend class ViewContext;
	friend class Fenetre;
	friend class GUIFenetre;
	friend class GLDrawingFenetre;
	friend class ExportFormatWCSVNS;
	private:
		int OpenState, GLOK, InterLock, QueuedRedraw, QueuedViewNum, LastActiveView, QuietDuringLoad;
		char DelayedSync, SyncDelayTicks;
		char BGRedraw, BGRedrawCurView;
		char MotionControlEnable, MotionControlMode;
		char DigitizeOneShot;
		double OriginThresh, HMotionScaleFactor, VMotionScaleFactor, GlobeRadius, ObjectUnit;
		char ViewManipulationEnable, ViewManipulationMode, ObjectManipulationMode;
		char AxisEnable[4], RegenDyn, RegenDEMs, RegenVecs, RegenWalls, RegenSlopeTex, RegenFractTex, RegenEcoTex, RegenRelelTex;
		long LastMouseX, LastMouseY;
		unsigned long InterCode;
		unsigned long SyncDelayTime;
		//ViewContext *DeferredGrab;
		double DEMPolyPercent, ObjPolyPercent;
		//GLDrawingFenetre *DeferredWin;
		double LastMouseLat, LastMouseLon;
		VertexDEM Cursor, DBCenter;
		Project *LocalProject;
		Database *LocalDB;
		ViewContext *ViewSpaces[WCS_VIEWGUI_VIEWS_OPEN_MAX];
		ViewPrefsGUI *PrefsGUI;
		ViewRTPrefsGUI *RTPrefsGUI;
		RenderData *CamSetup;
		InterCommon *InterStash;
		Joe *TempVec;
		Point3d Dif;
		int JoeChanged;
		GLdouble PickMatrix[4]; // X, Y, W, H
		GLuint SelBuf[WCS_VIEWGUI_SELBUF_SIZE];
		char PickEnabled, ClearPointsPick, ZoomInProgress, ZoomView, RegionInProgress, RegionView, MeasureInProgress;
		long ZoomCoord[2], RegionCoord[2];
		double MeasureString[6];
		RasterAnimHost *NewSelected;
		double GlobalFloor, TexElRange, TexElMin, TexElMax;
		double StaticGlobalFloor, StaticTexElRange, StaticTexElMin, StaticTexElMax;
		double DynGlobalFloor, DynTexElRange, DynTexElMin, DynTexElMax;
		float JOX, JOY, JOZ;

		AnimColorGradient *EarthGradient, *PrimaryGradient, *GreyGradient;
		unsigned char *EarthTexGrad, *PrimaryTexGrad, *GreyTexGrad, *ContourTex, *GridTex;

		// Array of DisplayList numbers for Perspective and Planimetric cameras
		unsigned long PerspVectorLists[WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX],
		 PlanVectorLists[WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX];
		#ifdef WCS_BUILD_VNS
		unsigned long ProjPlanVectorLists[WCS_VIEWGUI_GLOBAL_DISPLAYLIST_MAX];
		#endif // WCS_BUILD_VNS
		signed long WallPerspList, WallPlanList;
		#ifdef WCS_BUILD_VNS
		signed long WallProjPlanList;
		#endif // WCS_BUILD_VNS
		Point3d WallPerspOri;
		int VectorListsAreReserved, LoadInProgress;

		// these are the config structures for creating and displaying Realtime Foliage List files
		RealTimeFoliageDisplayConfig RTFDispConf;
		RealTimeFoliageWriteConfig RTFWriteConf;

		void ReserveVectorDisplayLists(void);
		void KillVectorDisplayLists(void);
		void KillDEMDisplayLists(void);

		double Weigh3DObjects(void);
		double WeighDEMGeometry(void);

		int ReboundDEMGeometry(int ViewNum, unsigned long DisplayList, ViewContext *VC, unsigned long Flags, double &MaxEl, double &MinEl, double &CellSizeNS, double &CellSizeWE);
		int ReboundDynamic(int ViewNum, ViewContext *VC);
		int RegenDEMGeometry(int ViewNum, double PolyPercent, unsigned long DisplayList, ViewContext *VC, int JustReboundFlags = NULL);
		int GenOneDEM(int ViewNum, Point3d UseOrigin, ViewContext *VC, Joe *Clip, DEM *Terrain, int ForceTFXPreviewOff = 0, CoordSys *ProjectedSys = NULL);
		int LoadOneDEM(int ViewNum, double PolyPercent, ViewContext *VC, Joe *Clip, JoeViewTemp *JVT);
		int FreeDEM(int ViewNum, Point3d UseOrigin, ViewContext *VC, Joe *Clip, JoeViewTemp *JVT, JoeDEM *MyDEM);
		int RegenVectorGeometry(int ViewNum, long Simplify, unsigned long DisplayList, ViewContext *VC, CoordSys *ProjectedSys = NULL);
		int GenOneVector(int ViewNum, long Simplify, ViewContext *VC, Joe *Clip, VertexDEM *OriginVert, int AsPoints = 0, CoordSys *ProjectedSys = NULL);
		int GenOneLake(int ViewNum, long Simplify, ViewContext *VC, Joe *Clip, LakeEffect *Lee, VertexDEM *OriginVert, CoordSys *ProjectedSys = NULL);
		void PushAndTranslateOrigin(ViewContext *VC);
		void PopOrigin(void);
		
		void DrawView(int ViewNum, ViewContext *VC);
		void SetupLightsAndCameras(int ViewNum, ViewContext *VC);
		void DrawSetup(int ViewNum, ViewContext *VC);
		void DrawClear(int ViewNum, ViewContext *VC);
		void DrawCamera(int ViewNum, ViewContext *VC);
		void DrawLights(int ViewNum, ViewContext *VC);
		void DrawCelestial(int ViewNum, ViewContext *VC);
		void DrawGLProjection(int ViewNum, ViewContext *VC, char Celestial, char FirstPick);
		void DrawFog(ViewContext *VC);
		void DrawDEMObjects(int ViewNum, ViewContext *VC, unsigned long DBFlags);
		void DrawVectorObjects(int ViewNum, ViewContext *VC);
		void DrawDigitizeObject(int ViewNum, ViewContext *VC);
#ifdef WCS_BUILD_VNS
		void DrawActiveSQ(int ViewNum, ViewContext *VC);
#endif // WCS_BUILD_VNS
		void DrawCursor(int ViewNum, ViewContext *VC);
		void DrawCameraObjects(int ViewNum, ViewContext *VC);
		void Draw3DObjects(int ViewNum, ViewContext *VC, double Simplification);
		void DrawWalls(int ViewNum, ViewContext *VC);
		void DrawFoliage(int ViewNum, ViewContext *VC, double Simplification, int FolType = 0);
		void DrawTargetObjects(int ViewNum, ViewContext *VC);
		void DrawWaveObjects(int ViewNum, ViewContext *VC);
		void DrawLightObjects(int ViewNum, ViewContext *VC);
		void DrawWater(int ViewNum, ViewContext *VC);
		void DrawClouds(int ViewNum, ViewContext *VC);
		void DrawCMaps(int ViewNum, ViewContext *VC);
#ifdef WCS_BUILD_RTX
		void DrawExportBounds(int ViewNum, ViewContext *VC);
#endif // WCS_BUILD_RTX
		void DrawVariousLines(int ViewNum, ViewContext *VC);
		void DrawCleanup(int ViewNum, ViewContext *VC);
		void DrawSync(int ViewNum, ViewContext *VC);

		//void DrawDEMOutline(int ViewNum, ViewContext *VC, double N, double S, double E, double W);

		void DrawSetupActiveObject(ViewContext *VC, int Full = 1);
		void DrawFinishActiveObject(ViewContext *VC, int Full = 1);

		// demo the technique of reading foliage data from list files
		int PrepRealtimeFoliage(char *IndexName);
		void FreeRealtimeFoliage(void);
		//void AllInOneRealtimeFoliage(void);

		// DoDisplayList is now a macro
		//inline void FinishDisplayList(void) {if(ListGenInProgress) glEndList();};

		// retired
		//int CheckTask(int Mode);

		void DoPointHit(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, int ClearPoints);
		void DoSampleDiag(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, char Alt, char Control, char Shift, char LeftMouseTransition);
		void DoCopy(int View, int Direction);
		void BeginInteractive(void);
		void DoInteractive(ViewContext *OriVC, GLDrawingFenetre *GLDF, long XDif, long YDif, long ZXDif, long ZYDif, unsigned long Code);
		void EndInteractive(void);
		void DoInteractivePlace(ViewContext *OriVC, GLDrawingFenetre *GLDF, double Lat, double Lon, double Elev, long X, long Y);
		RasterAnimHost *DoSelectObject(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, int PointHit, int ClearPoints);

		double GLGetScaledPixelZ(ViewContext *OriVC, GLDrawingFenetre *GLDF, int X, int Y);
		void SetLastMouse(long X, long Y) {LastMouseX = X; LastMouseY = Y;};
		void SetLastMouseLatLon(double Lat, double Lon) {LastMouseLat = Lat; LastMouseLon = Lon;};
		void ClearLastMouse(void) {LastMouseX = LONG_MIN; LastMouseY = LONG_MIN; LastMouseLat = DBL_MAX; LastMouseLon = DBL_MAX;};
		char LastMouseValid(void) {return(LastMouseX != LONG_MIN);};
		char MouseDif(long &XD, long &YD) {if(LastMouseX != LONG_MIN) {XD -= LastMouseX; YD -= LastMouseY; return(1);} else return(0);};
		char LatLonDif(double &LatD, double &LonD) {if(LastMouseLat != DBL_MAX) {LatD -= LastMouseLat; LonD -= LastMouseLon; return(1);} else return(0);};
		void LatLonFromXY(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double &Lat, double &Lon);
		void LatLonElevFromXY(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double &Lat, double &Lon, double &Elev);
		void XYZfromXYZ(ViewContext *OriVC, GLDrawingFenetre *GLDF, long X, long Y, double Z, Point3d DestXYZ);

		void UpdateAxes(void);
		GLDrawingFenetre *CreateNewViewFen(int ViewSpaceNum, unsigned long ViewWinID);
		void ConfigureTitle(int ViewSpaceNum);
		int VecCtrlScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation);

		RasterAnimHost *ProcessSelectHits(ViewContext *VC, int NumHits);
		inline void SetupPickName(RasterAnimHost *Nom) {if(PickEnabled) glLoadName((GLuint)Nom);};
		inline void ClearPickName(void) {if(PickEnabled) glLoadName(0);};

		void SetQueuedRedraw(void) {QueuedRedraw = 1;};
		void BeginBGRedraw(void);
		void EndBGRedraw(void);

		double GetRadius(void) {return(GlobeRadius);};
		int GenTexMaps(PlanetOpt *PO, int WhichMap);
		int GenTexMap(PlanetOpt *PO, Joe *Clip, JoeViewTemp *JVT, JoeDEM *MyDEM, int WhichMap, EnvironmentEffect *DefEnv, GroundEffect *DefGround, char CmapsExist);
		void CalcTexMapSize(JoeDEM *MyDEM, int &TexMapWidth, int &TexMapHeight);
		void DeleteAllTexMaps(int WhichMap = -1);
		void TryFreeTex(int WhichMap);

		int ProjectForViews(ViewContext *VC, VertexDEM *Input);
				
	public:

		int ConstructError;
		GLDrawingFenetre *ViewWins[WCS_VIEWGUI_VIEWS_OPEN_MAX];

		ViewGUI(Project *ProjSource, Database *DBStore, InterCommon *IC);
		~ViewGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		void HandleNotifyEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandlePopupMenuSelect(int MenuID);
		long HandleLoseFocus(void);
		long HandleGainFocus(void);
		long HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift);
		long HandleRightButtonDown(long X, long Y, char Alt, char Control, char Shift);
		long HandleRightButtonUp(long X, long Y, char Alt, char Control, char Shift);
		long HandleKeyUp(int Key, char Alt, char Control, char Shift);
		long HandleKeyDown(int Key, char Alt, char Control, char Shift);
		long HandleKeyPress(int Key, char Alt, char Control, char Shift);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
		long HandleMouseWheelVert(long X, long Y, float Amount, char Alt, char Control, char Shift);
		long HandleMouseWheelHoriz(long X, long Y, float Amount, char Alt, char Control, char Shift);
/*
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleKeyDown(char Key, char Alt, char Control, char Shift);
		long HandleRepaint(NativeDrawContext NDC);
		long HandlePopupMenuSelect(int MenuID);

		void HandleNotifyEvent(void);
*/
		long HandleBackgroundCrunch(int Siblings); // return 1 when complete
		void ConfigureWidgets(void);
		void EnableViewButtons(int IsView);
		void EnableJoeControls(char IsVectorOrControlPt, char IsDEM);
		void EnableAxisButtons(char X, char Y, char Z, char E);
		void EnableModeButtons(char M, char R, char S);
		void EnablePoints(char Points);
		void FetchObjectActionsAxes(RasterAnimHost *Active, char &Move, char &Rot, char &Scale, char &Xen, char &Yen, char &Zen, char &Points);


		void DisableWidgets(void);

		void SelectPanel(int Panel, int Pane);
		int Draw(int WhichView = -1);
		int DrawImmediately(int WhichView = -1);
		int DrawBG(void);
		int DrawForceRegen(int Vecs, int DEMs, int WhichView = -1);

		void PrepForRedraw(void);
		void FinishRedraw(void);
		void DoAnim(int ViewNum);
		void DoGLCompatCheck(void);
		void DoGLVersion(void);
		void DoGLBench(int ViewNum);
		void DoSaveImage(int ViewNum);
		void DoClose(int ViewOrigin);

		// used to be private, now S@G (SceneViewGUI) wants to access it -- make it public rather than friend it
		void DoCreate(short Mode, long ForceCategory = 0); // Cat = 0 would be GenericEffect, which DigitizeGUI will ignore
		void EndCreate(void);
		void DoRTPrefs(void);
		void UpdateRTPrefs(int ViewNum);
		void DoPrefs(int ViewNum);
		void UpdatePrefs(int ViewNum);
		void ChangePrefs(int ViewNum, int ID);
		void DoRenderOpt(int ViewNum, int ID);
		int DoRender(int ViewNum, int WithRTF = 0);
		int DoDelete(void);
		int IdentActiveView(void);
		int IdentViewOrigin(AppEvent *Activity);
		ViewContext *GetViewSpace(int VS) {return(ViewSpaces[VS]);};
		void SetCursor(double Lat, double Lon, double Elev) {Cursor.Lat = Lat; Cursor.Lon = Lon; Cursor.Elev = Elev;};
		void SetAxisEnable(char Axis, char NewState) {AxisEnable[Axis] = NewState; UpdateAxes();};
		char GetAxisEnable(char Axis) {return(AxisEnable[Axis]);};

		void CheckSpaceMode(void);
		void SetInterLock(int NewLock) {InterLock = NewLock;};
		int GetInterLock(void) {return(InterLock);};

		void DoCancelMeasure(int Quietly);
		void StartMeasure(void);
		void DoMeasurePoint(int ViewNum, double PointLat, double PointLon, double PointElev);

		void DoCancelZoom(int Quietly);
		void StartZoom(void);
		void DoZoomPoint(int ViewNum, int PointX, int PointY);

		void DoCancelRegion(int Quietly);
		void StartRegion(void);
		void DoRegionPoint(int ViewNum, int PointX, int PointY);
		void DrawTempBox(int ViewNum, unsigned short XS, unsigned short YS, unsigned short XE, unsigned short YE);

		int GetQueuedRedraw(void) {return(QueuedRedraw);};
		void ClearQueuedRedraw(void) {QueuedRedraw = 0;};
		
		void UpdateManipulationMode(void);
		void SetViewManipulationEnable(char NewState);
		char GetViewManipulationEnable(void) {return(ViewManipulationEnable);};
		void SetViewManipulationMode(char NewState);
		char GetViewManipulationMode(void) {return(ViewManipulationMode);};
		void SetObjectManipulationMode(char NewState);
		char GetObjectManipulationMode(void) {return(ObjectManipulationMode);};
		void ActiveItemSwitch(void);

		// The following method is safe to call with a NULL instance
		char GetEnabled(int View, int Item);
		char *GetViewName(int View);
		char GetViewAlive(int View);

		int CreateNewView(char *DockCell, Camera *ViewCam, RenderOpt *ViewRO);
		Camera *CreateNewCamera(char CamType);
		int CreateNewViewAndCamera(char *DockCell, char CamType);

		long ViewNumFromVC(ViewContext *VC);
		long ViewNumFromFenetre(Fenetre *Host);
		long GetNumOpenViews(void);
		long GetNumOpenLiveViews(void);
		Renderer *FindNextLiveRenderer(long &CurViewNum);

		// Unproject-intersection
		int CollideCoord(int ViewNum, double &LatOut, double &LonOut, long XIn, long YIn, double ElevIn);
		int ScaleMotion(DiagnosticData *Diag);

		// state transfer and persistance
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		void CloseViewsForIO(int AlterQuiet = 0);
		void ReopenIOComplete(int AlterQuiet = 0);
		
		// retired
		//void TaskModeChanged(void);
		void ActiveObjectChanged(RasterAnimHost *AO);
		void Invalidate(RasterAnimHost *Invalid);

		// New motion control
		void ProcessMotionControl(void);
		void PrepMotionControl(void); // take care of startup and cleanup
		void SetMotionControl(char NewControlState) {MotionControlEnable = NewControlState;};
		void SetMotionControlMode(char NewMode) {MotionControlMode = NewMode;};
		char GetMotionControl(void) {return(MotionControlEnable);};
		char GetMotionControlMode(void) {return(MotionControlMode);}; // 0=drive, 1=slide

		// Realtime Foliage List generation
		int BuildRealtimeFoliage(int ViewNum);
		
		// expose per-View state to outside for WinCaptionButtonBar to access
		unsigned long QueryViewWindowState(long ViewNum);
		unsigned long QueryViewWindowState(ViewContext *VC);

	}; // class ViewGUI

class JoeViewTemp: public JoeAttribute
	{
	friend class Joe;
	friend class Database;
	friend class JoeList;
	friend class Renderer;
	public:
		unsigned long PerspDisplayListNum, PlanDisplayListNum;
		#ifdef WCS_BUILD_VNS
		unsigned long ProjPlanDisplayListNum;
		#endif // WCS_BUILD_VNS
		signed long MaxPolys, MyNextBGRedrawAction;
		int TexMapWidth, TexMapHeight;
		unsigned char *SlopeTex, *RelelTex, *FractTex, *EcoTex; 
		// Initialize JoeAttribute base members in constructor...
		JoeViewTemp() {InitClear();};
		virtual ~JoeViewTemp() {DeleteTexMaps();};
		void DeleteTexMaps(void);
		void DeleteTexMap(int WhichMap);
		// We don't save anything
		virtual unsigned long WriteToFile(FILE *Out) {return(0);};
		void InitClear(void);
	}; // JoeViewTemp


// file IO
#define WCS_VIEWINIT_VIEWCONTEXT			0x00010000
#define WCS_VIEWINIT_FOLWRITECONFIG			0x00020000
#define WCS_VIEWINIT_FOLDISPLAYCONFIG		0x00030000

#define WCS_VIEWCONTEXT_CAMERANAME			0x00210000
#define WCS_VIEWCONTEXT_OPTIONSNAME			0x00220000
#define WCS_VIEWCONTEXT_VCWINID				0x00230000
#define WCS_VIEWCONTEXT_FENID				0x00240000
#define WCS_VIEWCONTEXT_NUMENABLED			0x00250000
#define WCS_VIEWCONTEXT_ENABLED				0x00260000
#define WCS_VIEWCONTEXT_LTDREGIONCTRX		0x00270000
#define WCS_VIEWCONTEXT_LTDREGIONCTRY		0x00280000
#define WCS_VIEWCONTEXT_LTDREGIONWIDTH		0x00290000
#define WCS_VIEWCONTEXT_LTDREGIONHEIGHT		0x002a0000
#define WCS_VIEWCONTEXT_CONTOURINTERVAL		0x002b0000

#define WCS_DOINTER_CODE_SHIFT		0x01

inline double ViewContext::VCCalcExag(double Elev)
{
return(Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue + (Elev - Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) * Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue);
} // ViewContext::VCCalcExag

inline double ViewContext::VCUnCalcExag(double Elev)
{
double Exag;
Exag = Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
if(Exag == 0.0) return(Elev);
return(Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue + (Elev - Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) / Exag);
} // ViewContext::VCUnCalcExag

#endif // WCS_VIEWGUI_H
