// EDSSControlGUI.h
// Header file for Ecosystem Decision Support System control
// Created from scratch on 10/15/01 by Gary R. Huber
// Copyright 2001 3D Nature. All rights reserved.

#ifndef WCS_EDSSCONTROLGUI_H
#define WCS_EDSSCONTROLGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "RasterDrawWidget.h"

class EDSSControl;
class EffectsLib;
class RenderJob;
class Camera;
class RenderOpt;
class Atmosphere;
class Sky;
class Light;
class TerrainParamEffect;
class PlanetOpt;
class Raster;
class BrowseData;
class GeoRefShell;
class Database;
class DiagnosticData;

#define WCS_EDSSCONTROLGUI_NUMTABS	6
#define WCS_EDSS_NUMNAILS	13
#define WCS_EDSS_MAX_PRIMARYKEYS		4
#define WCS_EDSS_MAX_PRIMARYKEYVALUES	30
#define WCS_EDSS_MAX_SECONDARYKEYS		4

// Very important that we list the base classes in this order.
class EDSSControlGUI : public WCSModule, public GUIFenetre
	{
	private:
		static char *TabNames[WCS_EDSSCONTROLGUI_NUMTABS];
		static long ActivePage;
		EDSSControl *ControlHost;
		EffectsLib *EffectsHost;
		Database *DBHost;
		ImageLib *ImageHost;
		RenderJob *ActiveJob;
		Camera *ActiveCam;
		RenderOpt *ActiveOpt;
		TerrainParamEffect *ActiveTerrainPar;
		PlanetOpt *ActivePlanetOpt;
		Atmosphere *ActiveAtmo;
		Sky *ActiveSky;
		Light *ActiveLight;
		GeoRefShell *GeoShell;

		Raster *RasterList;
		char *NameList[WCS_EDSS_NUMNAILS];
		char CustomRes;
		BrowseData *BrowseList;
		struct RasterWidgetData SwatchData0, SwatchData1, SwatchData2;
		int Sampling, SamplingCamera, TNailsConfigured, SwatchConfigured, Rendering, CurCamPage, ElevConfigured1, 
			ElevConfigured2, PrimaryKeys, PKeyValues, MaxSecondaryKeys, ReceivingDiagnostics;
		int WidSize[3][2];
		char PrimaryKeyValue[WCS_EDSS_MAX_PRIMARYKEYS][80];
		char SecondaryKeyValue[WCS_EDSS_MAX_PRIMARYKEYVALUES][WCS_EDSS_MAX_SECONDARYKEYS][32];
		double EDSS_PlanWidth, EDSS_PlanNorth, EDSS_PlanSouth, EDSS_PlanWest, EDSS_PlanEast;

	public:

		int ConstructError;

		EDSSControlGUI(EDSSControl *ControlSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~EDSSControlGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);

		// Help lookup
		unsigned long QueryHelpTopic(void);

	private:
		void ConfigureTNails(void);
		void ConfigureColorSwatch(void);
		void ConfigureSizeDrop(void);
		void ConfigureOutputEvent(void);
		void ConfigureInitialComponents(void);
		void ConfigureOutputName(void);
		void SetScrollPos(int CamPos);
		void DisableWidgets(void);
		void SetCamPage(int Page);
		int InitCameraMotion(int CamPos, double X, double Y);
		void SetLatLonFromXY(int CamPos, double X, double Y);
		void ComputeOverlay(int CamPos);
		void SelectNewResolution(void);
		void SelectNewOutputFormat(void);
		void ScaleSize(void);
		void SetConstrain(void);
		void LoadComponent(int ButtonID);
		void InitiateRender(void);
		void TransferCamera(void);
		void DisableRenderWidgets(char Disable);
		void RenderComplete(void);
		void SyncPlanCam(void);
		void SyncTerrainElev(void);
		void SyncVectorMessage(int LatLonProvided = 0, double Lat = 0.0, double Lon = 0.0);
		char *MatchSecondaryKey(char *PKeyValue, int &KeyCt, char *LastSecondaryKeyValue);
		void CaptureDiagnostics(DiagnosticData *Data);
		//char *MatchDensityFieldToTree(char *TreeName);
		//char *MatchHeightFieldToTree(char *TreeName);
		

	}; // class EDSSControlGUI

#endif // WCS_EDSSCONTROLGUI_H
