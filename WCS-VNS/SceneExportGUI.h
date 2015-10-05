// SceneExportGUI.h
// Header file for Scene Exporter
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Modified by GRH on 3/29/00
// Copyright 1996-2000 Questar Productions

#ifndef WCS_SCENEEXPORTGUI_H
#define WCS_SCENEEXPORTGUI_H

class RenderData;
class Database;
class EffectsLib;
class Project;
class DEM;
class VertexDEM;
class Light;
class Joe;

#include "Application.h"
#include "Fenetre.h"
#include "Types.h"
#include "3dsftk.h"
#include "GraphData.h"

enum
	{
	WCS_EXPORT_FORMAT_LIGHTWAVE_3 = 0,
	WCS_EXPORT_FORMAT_LIGHTWAVE_7,
	WCS_EXPORT_FORMAT_3DS,
	WCS_EXPORT_FORMAT_LAST // not our usual convention of WCS_EXPORT_FORMAT_MAX because MAX is a possible value!
	}; // File Format

// reasonable limits, you could make them unreasonable if you like
#define WCS_EXPORT_LWOB_MAX_POLYGONS	1000000
#define WCS_EXPORT_LWOB_MAX_VERTICES	32767
// TEN! MILLION! POLYGONS!
#define WCS_EXPORT_LWO2_MAX_POLYGONS	10000000
#define WCS_EXPORT_LWO2_MAX_VERTICES	16000000

class ImportInfo
	{
	public:
		double RefLat, RefLon, RefPt[3], UnitScale;
		long MaxPolys, MaxVerts;
		short ExportItem, Bathymetry, SaveDEMs, KeyFrameInt;
		char Path[512], Name[256];
		Matx3x3 RotMatx;

		ImportInfo(Project *ProjSource);
	};

class LightWaveMotion
	{
	public:
		double XYZ[3], HPB[3], SCL[3];
		long Frame, Linear;
		double TCB[3];

		LightWaveMotion();
	};

class ThreeDSMotion
	{
	public:
		double CamXYZ[3], TargetXYZ[3], Bank, FOV;
		long Frame;
		double TCB[3];

		ThreeDSMotion();
	};

// Very important that we list the base classes in this order.
class SceneExportGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		RenderData *RendData;
		AnimDoubleTime RefLatADT, RefLonADT;
		
	public:

		int ConstructError, ExportUnits;
		ImportInfo LWInfo;


		SceneExportGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource);
		~SceneExportGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		void ConfigureWidgets(void);
		void BuildJobList(void);
		void SetExportUnitScale(void);
		void Export(void);

		// 3D Studio functions
		void ThreeDSScene_Export(ImportInfo *LWInfo, RenderData *Rend);
		int ThreeDSObject_VertexCount(Joe *JoeObj, long &Vertices, long &Polys,
			long &LWRows, long &LWCols, long &LWRowInt, long &LWColInt,
			long MaxVertices, long MaxPolys, DEM *Topo);
		int ThreeDSCamTarget_VertexCount(long &Vertices, long &Polys);
		int ThreeDSObject_FillData(VertexDEM *PP,
			long Vertices, long Polys, double LonRot, ImportInfo *LWInfo, RenderData *Rend,
			long LWRows, long LWCols, long LWRowInt, long LWColInt,
			point3ds *VertexData, face3ds *PolyData, DEM *Topo);
		int ThreeDSCamTarget_FillData(point3ds *VertexData, face3ds *PolyData);
		void ThreeDSObject_FindBounds(ushort3ds nvertices, point3ds *vertexarray, point3ds *boundmin, point3ds *boundmax);
		int ThreeDSLight_SetKeys(kfspot3ds **KFSpot3dsPtr, ImportInfo *LWInfo, RenderData *Rend, Light *CurLight);
		int ThreeDSCamera_SetKeys(kfcamera3ds **KFCamera3dsPtr, ImportInfo *LWInfo, RenderData *Rend);
		int Set_3DS(ThreeDSMotion *LWM, ImportInfo *LWInfo, long Frame, RenderData *Rend);
		int ThreeDSCamTarget_SetKeys(kfmesh3ds **KFMesh3dsPtr, ImportInfo *LWInfo, RenderData *Rend);

		// LightWave functions
		void SetupForExport(ImportInfo *LWInfo, RenderData *Rend);
		int ExportWave(ImportInfo *LWInfo, FILE *Supplied, RenderData *Rend, int ExportAsVersion = WCS_EXPORT_FORMAT_LIGHTWAVE_3);
		int Set_LWM(LightWaveMotion *LWM, ImportInfo *LWInfo,
			long Frame, RenderData *Rend);
		int LWOB_Export(Joe *JoeObj, char *OutputName, VertexDEM *PP,
			short SaveObject, double LonRot, double LatRot, ImportInfo *LWInfo, RenderData *Rend, int ExportAsVersion = WCS_EXPORT_FORMAT_LIGHTWAVE_3);
		void LWScene_Export(ImportInfo *LWInfo, RenderData *Rend, int ExportAsVersion = WCS_EXPORT_FORMAT_LIGHTWAVE_3);

	}; // class SceneExportGUI

#endif // WCS_SCENEEXPORTGUI_H
