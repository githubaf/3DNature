// SceneImportGUI.h
// Header file for Scene Import module
// Created from LWExportGUI.h on 4/9/97 by GRH
// Copyright 1996 Questar Productions

#ifndef WCS_SCENEIMPORTGUI_H
#define WCS_SCENEIMPORTGUI_H

class Database;
class EffectsLib;
class Project;
class VertexDEM;
class ImportInfo;
class Light;
class Camera;
class Sky;
class Atmosphere;
class RenderOpt;
class CoordSys;

#include "Application.h"
#include "Fenetre.h"
#include "Types.h"
#include "GraphData.h"
#include "SceneExportGUI.h"


enum
	{
	WCS_IMPORT_FORMAT_LIGHTWAVE_3 = 0,
	WCS_IMPORT_FORMAT_LIGHTWAVE_7,
	WCS_IMPORT_FORMAT_3DS,
	WCS_IMPORT_FORMAT_LAST // not our usual convention of WCS_EXPORT_FORMAT_MAX because MAX is a possible value!
	}; // File Format


// Very important that we list the base classes in this order.
class SceneImportGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		AnimDoubleTime RefLatADT, RefLonADT;
		char ClearCameras, ClearLights, ClearSkies, ClearAtmos, ClearOptions;
		double PlanetRad, FrameRate;
		Light *CurLight;
		Camera *CurCamera;
		Sky *CurSky;
		Atmosphere *CurAtmo;
		RenderOpt *CurOpt;
		CoordSys *DefCoords;
		
	public:

		int ConstructError, ImportItems, ImportUnits;
		ImportInfo LWInfo;
		char ImportPath[256], ImportFile[32];

		SceneImportGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource);
		~SceneImportGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);
		void SetImportUnitScale(void);
		void SetupForImport(ImportInfo *LWInfo);
		void Import(void);
		void ImportLWMotion(int MotionItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, char *InputBuf, int ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_3);
		void ImportLWColor(int ColorItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, char *InputBuf, int ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_3);
		void ImportLWSetting(int SettingItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, int ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_3);
		void AttemptPreReadResAspect(ImportInfo *LWInfo);
		void GetInputFile(void);
		int ScanLightWave(FILE *fh);
		long ImportLightWave(FILE *fh, ImportInfo *LWInfo);
		char *AdvanceToNext(char *Buffer);
		char *AdvanceToFirst(char *Buffer);
		long HasEnvelope(char *Buffer);
		void SelectAll(void);
		void Import3DS(file3ds *File3ds, short ImportIt, ImportInfo *LWInfo);
		short Match3DSImport(NativeControl ListView, int ItemMatch, char *SecondMatch);

		int UnSet_3DSPoint(ImportInfo *LWInfo, VertexDEM *Vert);

		int UnSet_LWM(ImportInfo *LWInfo, double *Value, VertexDEM *Vert, double &Heading, double &Pitch, double &Bank);
		int UnSet_LWMLight(ImportInfo *LWInfo, double *Value, VertexDEM *Vert);

	}; // class SceneImportGUI

enum
	{
//	WCS_LWIMPORT_THEME_LOADOBJECT = 0,
	WCS_LWIMPORT_THEME_CAMERAMOTION = 0,
	WCS_LWIMPORT_THEME_ADDLIGHT,
	WCS_LWIMPORT_THEME_LIGHTMOTION,
	WCS_LWIMPORT_THEME_ZOOMFACTOR,
	WCS_LWIMPORT_THEME_ZENITHCOLOR,
	WCS_LWIMPORT_THEME_SKYCOLOR,
	WCS_LWIMPORT_THEME_FOGMINDIST,
	WCS_LWIMPORT_THEME_FOGMAXDIST,
	WCS_LWIMPORT_THEME_FOGCOLOR,
//	WCS_LWIMPORT_THEME_FIELDRENDERING,
//	WCS_LWIMPORT_THEME_ANTIALIASING,
//	WCS_LWIMPORT_THEME_MOTIONBLUR,
//	WCS_LWIMPORT_THEME_RESOLUTION,
//	WCS_LWIMPORT_THEME_LETTERBOX,
	WCS_LWIMPORT_THEME_PIXELASPECTRATIO,
	WCS_LWIMPORT_THEME_LIGHTCOLOR,
	WCS_LWIMPORT_THEME_LGTINTENSITY,
	WCS_LWIMPORT_THEME_FIRSTFRAME,
	WCS_LWIMPORT_THEME_LASTFRAME,
	WCS_LWIMPORT_THEME_FRAMESTEP,
	WCS_LWIMPORT_THEME_AMBIENTCOLOR,
	WCS_LWIMPORT_THEME_AMBINTENSITY,
	WCS_LWIMPORT_THEME_FRAMESIZE
	}; // Import themes

enum
	{
	WCS_3DSIMPORT_THEME_LOADOBJECT = 0,
	WCS_3DSIMPORT_THEME_OBJECTMOTION,
	WCS_3DSIMPORT_THEME_ADDOMNILIGHT,
	WCS_3DSIMPORT_THEME_ADDSPOTLIGHT,
	WCS_3DSIMPORT_THEME_OMNILIGHTMOTION,
	WCS_3DSIMPORT_THEME_SPOTLIGHTMOTION,
	WCS_3DSIMPORT_THEME_ADDCAMERA,
	WCS_3DSIMPORT_THEME_CAMERAMOTION,
	WCS_3DSIMPORT_THEME_ZOOMFACTOR,
	WCS_3DSIMPORT_THEME_ZENITHCOLOR,
	WCS_3DSIMPORT_THEME_SKYCOLOR,
	WCS_3DSIMPORT_THEME_FOGMINDIST,
	WCS_3DSIMPORT_THEME_FOGMAXDIST,
	WCS_3DSIMPORT_THEME_FOGCOLOR,
	WCS_3DSIMPORT_THEME_LIGHTCOLOR,
	WCS_3DSIMPORT_THEME_LGTINTENSITY,
	WCS_3DSIMPORT_THEME_ANIMLENGTH,
	WCS_3DSIMPORT_THEME_AMBIENTCOLOR,
	WCS_3DSIMPORT_THEME_AMBINTENSITY
	}; // Import themes

#endif // WCS_SCENEIMPORTGUI_H
