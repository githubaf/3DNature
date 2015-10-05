// EDSSControlGUI.cpp
// Code module for Ecosystem Decision Support System control
// Created from scratch on 10/15/01 by Gary R. Huber
// Copyright 2001 3D Nature. All rights reserved.

#include "stdafx.h"
#include "EDSSControlGUI.h"
#include "EDSSControl.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "Raster.h"
#include "RenderOptEditGUI.h"
#include "ImageOutputEvent.h"
#include "DBFilterEvent.h"
#include "resource.h"

extern struct RenderPreset RPresets[];

#define EDSS_VIEWWIDTH		30574.781604
#define EDSS_VIEWLEFTLON	77.395239527
#define EDSS_VIEWRIGHTLON	77.136870644
#define EDSS_VIEWTOPLAT		38.224180068
#define EDSS_VIEWBOTTOMLAT	38.016532909

char *EDSSControlGUI::TabNames[WCS_EDSSCONTROLGUI_NUMTABS] = {"Camera Start", "Camera Middle", "Camera End", "Light", "Atmosphere", "Miscellaneous"};

long EDSSControlGUI::ActivePage;

NativeGUIWin EDSSControlGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // EDSSControlGUI::Open

/*===========================================================================*/

NativeGUIWin EDSSControlGUI::Construct(void)
{
int TabIndex;
char *SetName;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_EDSS_CONTROL, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_EDSS_CAMSTART, 0, 0);
	CreateSubWinFromTemplate(IDD_EDSS_CAMMIDDLE, 0, 1);
	CreateSubWinFromTemplate(IDD_EDSS_CAMEND, 0, 2);
	CreateSubWinFromTemplate(IDD_EDSS_LIGHT, 0, 3);
	CreateSubWinFromTemplate(IDD_EDSS_ATMOSPHERE, 0, 4);
	CreateSubWinFromTemplate(IDD_EDSS_MISC, 0, 5);

	if(NativeWin)
		{
		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 5000);
		WidgetSetScrollRange(IDC_SCROLLBAR2, 0, 5000);
		WidgetSetScrollRange(IDC_SCROLLBAR3, 0, 5000);
		WidgetSetScrollRange(IDC_SCROLLBAR4, 0, 100);	// h scroll
		WidgetSetScrollRange(IDC_SCROLLBAR5, 0, 100);	// v scroll
		WidgetSetScrollRange(IDC_SCROLLBAR6, 12, 100);	// visible
		WidgetSetScrollRange(IDC_SCROLLBAR7, 0, 100);	// h scroll
		WidgetSetScrollRange(IDC_SCROLLBAR8, 0, 100);	// v scroll
		WidgetSetScrollRange(IDC_SCROLLBAR9, 12, 100);	// visible
		WidgetSetScrollRange(IDC_SCROLLBAR10, 0, 100);	// h scroll
		WidgetSetScrollRange(IDC_SCROLLBAR11, 0, 100);	// v scroll
		WidgetSetScrollRange(IDC_SCROLLBAR12, 12, 100);	// visible
		WidgetSetScrollPos(IDC_SCROLLBAR6, (long)(SwatchData0.Visible * 100));
		WidgetSetScrollPos(IDC_SCROLLBAR9, (long)(SwatchData1.Visible * 100));
		WidgetSetScrollPos(IDC_SCROLLBAR12, (long)(SwatchData2.Visible * 100));
		// get sizes of RasterDrawWidgets
		WidgetGetSize(IDC_CAMPOS1, WidSize[0][0], WidSize[0][1]);
		WidgetGetSize(IDC_CAMPOS2, WidSize[1][0], WidSize[1][1]);
		WidgetGetSize(IDC_CAMPOS3, WidSize[2][0], WidSize[2][1]);
		WidSize[0][0] -= 4;
		WidSize[0][1] -= 4;
		WidSize[1][0] -= 4;
		WidSize[1][1] -= 4;
		WidSize[2][0] -= 4;
		WidSize[2][1] -= 4;

		for (TabIndex = 0; TabIndex < WCS_EDSSCONTROLGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for(TabIndex = 0; ; TabIndex++)
			{
			WidgetCBAddEnd(IDC_RENDERPRESETS, RPresets[TabIndex].PSName);
			if(RPresets[TabIndex].Aspect == 0.0)
				{
				CustomRes = TabIndex;
				break;
				} // if
			} // for
		SetName = NULL;
		for(TabIndex = 0; ; TabIndex++)
			{
			if (! (SetName = ImageSaverLibrary::GetNextFileFormat(SetName)))
				break;
			WidgetCBAddEnd(IDC_FORMATDROP, SetName);
			} // for
		ShowPanel(0, ActivePage);
		ConfigureOutputName();
		ConfigureWidgets();
		ConfigureInitialComponents();
		SetScrollPos(-1);
		SyncPlanCam();
		SyncTerrainElev();
		ReceivingDiagnostics = 1;
		} // if
	} // if
 
return (NativeWin);

} // EDSSControlGUI::Construct

/*===========================================================================*/

EDSSControlGUI::EDSSControlGUI(EDSSControl *ControlSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: GUIFenetre('EDSS', this, "EDSS Control") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
static NotifyTag AllWindowEvents[] = {MAKE_ID(WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_RCG, 0),
								0};
int Ct, KeyCt, Marker, KeyLen;
char NameStr[512], KeyTxt[128];

ConstructError = 0;
ControlHost = ControlSource;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
ActiveCam = NULL;
ActiveJob = NULL;
ActiveOpt = NULL;
ActiveAtmo = NULL;
ActiveLight = NULL;
ActiveSky = NULL;
ActiveTerrainPar = NULL;
ActivePlanetOpt = NULL;
BrowseList = NULL;
RasterList = NULL;
GeoShell = NULL;
Sampling = SamplingCamera = 0;
TNailsConfigured = SwatchConfigured = 0;
Rendering = 0;
ElevConfigured1 = ElevConfigured2 = 0;
PrimaryKeys = PKeyValues = MaxSecondaryKeys = 0;
ReceivingDiagnostics = 0;
// these widget sizes get set for real in Construct()
WidSize[0][0] = WidSize[0][1] = WidSize[1][0] = WidSize[1][1] = WidSize[2][0] = WidSize[2][1] = 100;

memset(NameList, 0, WCS_EDSS_NUMNAILS * sizeof (char *));
memset(&SwatchData0, 0, sizeof (struct RasterWidgetData));
memset(&SwatchData1, 0, sizeof (struct RasterWidgetData));
memset(&SwatchData2, 0, sizeof (struct RasterWidgetData));
SwatchData0.Visible = 1.0;
SwatchData1.Visible = 1.0;
SwatchData2.Visible = 1.0;
SwatchData0.RDWin = this;
SwatchData1.RDWin = this;
SwatchData2.RDWin = this;
SwatchData0.OverRast = new Raster();
SwatchData1.OverRast = new Raster();
SwatchData2.OverRast = new Raster();

for (Ct = 0; Ct < WCS_EDSS_MAX_PRIMARYKEYS; Ct ++)
	{
	PrimaryKeyValue[Ct][0] = 0;
	} // for
for (Ct = 0; Ct < WCS_EDSS_MAX_PRIMARYKEYVALUES; Ct ++)
	{
	for (KeyCt = 0; KeyCt < WCS_EDSS_MAX_SECONDARYKEYS; KeyCt ++)
		{
		SecondaryKeyValue[Ct][KeyCt][0] = 0;
		} // for
	} // for

if (ControlHost && EffectsHost && SwatchData0.OverRast && SwatchData1.OverRast && SwatchData2.OverRast)
	{
	if(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_config"))
		{
		strcpy(KeyTxt, GlobalApp->StartEDSS->ShapeFileName);
		StripExtension(KeyTxt);
		sprintf(NameStr, "%s - (%s) (%s)", GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_config"), GlobalApp->StartEDSS->NewProjectName, KeyTxt);
		SetTitle(NameStr);
		} // if

	ActiveJob = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB);
	while (ActiveJob)
		{
		if (ActiveJob->Cam && ActiveJob->Options)
			{
			if (ActiveJob->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
				{
				ActiveCam = ActiveJob->Cam;
				ActiveOpt = ActiveJob->Options;

				ControlHost->FirstCamLatADT.Copy(&ControlHost->FirstCamLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT));
				ControlHost->MidCamLatADT.Copy(&ControlHost->MidCamLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT));
				ControlHost->EndCamLatADT.Copy(&ControlHost->EndCamLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT));
				ControlHost->FirstCamLonADT.Copy(&ControlHost->FirstCamLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON));
				ControlHost->MidCamLonADT.Copy(&ControlHost->MidCamLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON));
				ControlHost->EndCamLonADT.Copy(&ControlHost->EndCamLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON));
				ControlHost->FirstCamElevADT.Copy(&ControlHost->FirstCamElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV));
				ControlHost->MidCamElevADT.Copy(&ControlHost->MidCamElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV));
				ControlHost->EndCamElevADT.Copy(&ControlHost->EndCamElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV));
				ControlHost->FirstTargLatADT.Copy(&ControlHost->FirstTargLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT));
				ControlHost->MidTargLatADT.Copy(&ControlHost->MidTargLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT));
				ControlHost->EndTargLatADT.Copy(&ControlHost->EndTargLatADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT));
				ControlHost->FirstTargLonADT.Copy(&ControlHost->FirstTargLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON));
				ControlHost->MidTargLonADT.Copy(&ControlHost->MidTargLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON));
				ControlHost->EndTargLonADT.Copy(&ControlHost->EndTargLonADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON));
				ControlHost->FirstTargElevADT.Copy(&ControlHost->FirstTargElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV));
				ControlHost->MidTargElevADT.Copy(&ControlHost->MidTargElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV));
				ControlHost->EndTargElevADT.Copy(&ControlHost->EndTargElevADT, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV));
				ControlHost->FirstFOV.Copy(&ControlHost->FirstFOV, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV));
				ControlHost->MidFOV.Copy(&ControlHost->MidFOV, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV));
				ControlHost->EndFOV.Copy(&ControlHost->EndFOV, ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV));

				break;
				} // if
			} // if
		ActiveJob = (RenderJob *)ActiveJob->Next;
		} // while
	ActivePlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL);
	ActiveTerrainPar = (TerrainParamEffect *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, NULL);
	ActiveLight = (Light *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_LIGHT, 1, NULL);
	ActiveAtmo = (Atmosphere *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 1, NULL);
	ActiveSky = (Sky *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_SKY, 1, NULL);

	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planwidth"))
		EDSS_PlanWidth = atof(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planwidth"));
	else
		EDSS_PlanWidth = EDSS_VIEWWIDTH;
	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_plannorth"))
		EDSS_PlanNorth = atof(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_plannorth"));
	else
		EDSS_PlanNorth = EDSS_VIEWTOPLAT;
	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_plansouth"))
		EDSS_PlanSouth = atof(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_plansouth"));
	else
		EDSS_PlanSouth = EDSS_VIEWBOTTOMLAT;
	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planwest"))
		EDSS_PlanWest = atof(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planwest"));
	else
		EDSS_PlanWest = EDSS_VIEWLEFTLON;
	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planeast"))
		EDSS_PlanEast = atof(GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("edss_planeast"));
	else
		EDSS_PlanEast = EDSS_VIEWRIGHTLON;
		
	for (Ct = 0; Ct < WCS_EDSS_MAX_PRIMARYKEYS; Ct ++)
		{
		sprintf(KeyTxt, "edss_primarykey_%d", Ct);
		if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt(KeyTxt))
			{
			strcpy(PrimaryKeyValue[PrimaryKeys], GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt(KeyTxt));
			PrimaryKeys ++;
			} // if
		} // for
	for (Ct = 0; Ct < WCS_EDSS_MAX_PRIMARYKEYVALUES; Ct ++)
		{
		sprintf(KeyTxt, "edss_secondarykey_%d", Ct);
		if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt(KeyTxt))
			{
			strcpy(NameStr, GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt(KeyTxt));
			for (KeyLen = 0; NameStr[KeyLen] != 0 && NameStr[KeyLen] != ':'; KeyLen ++);	//lint !e722
			strncpy(SecondaryKeyValue[PKeyValues][0], NameStr, KeyLen);
			SecondaryKeyValue[PKeyValues][0][KeyLen] = 0;
			for (; NameStr[KeyLen] != 0 && NameStr[KeyLen] == ':'; KeyLen ++);	//lint !e722
			for (KeyCt = 1; KeyCt < WCS_EDSS_MAX_SECONDARYKEYS && NameStr[KeyLen]; KeyCt ++)
				{
				Marker = KeyLen;
				for (; NameStr[KeyLen] != 0 && NameStr[KeyLen] != ','; KeyLen ++);	//lint !e722
				strncpy(SecondaryKeyValue[PKeyValues][KeyCt], &NameStr[Marker], KeyLen - Marker);
				SecondaryKeyValue[PKeyValues][KeyCt][KeyLen - Marker] = 0;
				if (KeyCt + 1 > MaxSecondaryKeys)
					MaxSecondaryKeys = KeyCt + 1;
				for (; NameStr[KeyLen] != 0 && NameStr[KeyLen] == ','; KeyLen ++);	//lint !e722
				} // if
			PKeyValues ++;
			} // if
		} // for


	if (! (BrowseList = new BrowseData[WCS_EDSS_NUMNAILS]))
		ConstructError = 1;
	if (! (RasterList = new Raster[WCS_EDSS_NUMNAILS]))
		ConstructError = 1;
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	GlobalApp->MCP->RegisterClient(this, AllWindowEvents);
	} // if
else
	ConstructError = 1;

if (! (ActiveJob && ActiveCam && ActiveOpt && ActiveLight && ActiveAtmo && ActiveSky && ActivePlanetOpt && ActiveTerrainPar))
	ConstructError = 1;

if (ConstructError)
	{
	UserMessageOK("EDSS", "Not all required project components are present. EDSS Interface can not be launched.");
	GlobalApp->SetTerminate();
	} // if

} // EDSSControlGUI::EDSSControlGUI

/*===========================================================================*/

EDSSControlGUI::~EDSSControlGUI()
{
int Ct;

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
ConfigureRD(NativeWin, IDC_CAMPOS1, NULL);
ConfigureRD(NativeWin, IDC_CAMPOS2, NULL);
ConfigureRD(NativeWin, IDC_CAMPOS3, NULL);
ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL4, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL5, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL6, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL7, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL8, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL9, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL10, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL11, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL12, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_TNAIL13, NULL, NULL, NULL);
if (BrowseList)
	delete [] BrowseList;
if (SwatchData0.OverRast)
	delete SwatchData0.OverRast;
if (SwatchData1.OverRast)
	delete SwatchData1.OverRast;
if (SwatchData2.OverRast)
	delete SwatchData2.OverRast;

for (Ct = 0; Ct < WCS_EDSS_NUMNAILS; Ct ++)
	{
	if (NameList[Ct])
		AppMem_Free(NameList[Ct], strlen(NameList[Ct]) + 1);
	if (RasterList)
		RasterList[Ct].Thumb = NULL;
	} // for

if (RasterList)
	delete [] RasterList;

GlobalApp->EDSSEnabled = 0;

} // EDSSControlGUI::~EDSSControlGUI()

/*===========================================================================*/

long EDSSControlGUI::HandleCloseWin(NativeGUIWin NW)
{

TransferCamera();
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DSS, 0);

return(0);

} // EDSSControlGUI::HandleCloseWin

/*===========================================================================*/

long EDSSControlGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case IDC_RESUMEVNS:
	case ID_KEEP:
	case IDCANCEL:
		{
		TransferCamera();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DSS, 0);
		break;
		} // 
	case IDC_QUIT:
		{
		AppScope->SetTerminate();
		break;
		} // 
	case IDC_VIEWIMAGE:
		{
		#ifdef WCS_BUILD_DEMO
		UserMessageDemo("Demo version does not save rendered images.");
		#else // WCS_BUILD_DEMO
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_IVG, 1);
		#endif // WCS_BUILD_DEMO
		break;
		} // IDC_VIEWIMAGE
	case IDC_RENDER:
		{
		InitiateRender();
		break;
		} // 
	case IDC_CONSTRAIN:
		{
		SetConstrain();
		break;
		} // 
	case IDC_SCALE:
		{
		ScaleSize();
		break;
		} // 

	// tool buttons
	case IDC_TNAIL1:
	case IDC_TNAIL2:
	case IDC_TNAIL3:
	case IDC_TNAIL4:
	case IDC_TNAIL5:
	case IDC_TNAIL6:
		{
		WidgetSetCheck(IDC_TNAIL1, 0);
		WidgetSetCheck(IDC_TNAIL2, 0);
		WidgetSetCheck(IDC_TNAIL3, 0);
		WidgetSetCheck(IDC_TNAIL4, 0);
		WidgetSetCheck(IDC_TNAIL5, 0);
		WidgetSetCheck(IDC_TNAIL6, 0);
		WidgetSetCheck(ButtonID, 1);
		LoadComponent(ButtonID);
		break;
		} // light buttons
	case IDC_TNAIL7:
	case IDC_TNAIL8:
	case IDC_TNAIL9:
	case IDC_TNAIL10:
		{
		WidgetSetCheck(IDC_TNAIL7, 0);
		WidgetSetCheck(IDC_TNAIL8, 0);
		WidgetSetCheck(IDC_TNAIL9, 0);
		WidgetSetCheck(IDC_TNAIL10, 0);
		WidgetSetCheck(ButtonID, 1);
		LoadComponent(ButtonID);
		break;
		} // atmo buttons
	case IDC_TNAIL11:
	case IDC_TNAIL12:
	case IDC_TNAIL13:
		{
		WidgetSetCheck(IDC_TNAIL11, 0);
		WidgetSetCheck(IDC_TNAIL12, 0);
		WidgetSetCheck(IDC_TNAIL13, 0);
		WidgetSetCheck(ButtonID, 1);
		LoadComponent(ButtonID);
		break;
		} // sky buttons
	default:
		break;
	} // ButtonID

return(0);

} // EDSSControlGUI::HandleButtonClick

/*===========================================================================*/

long EDSSControlGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RENDERPRESETS:
		{
		SelectNewResolution();
		break;
		} // IDC_RENDERPRESETS
	case IDC_FORMATDROP:
		{
		SelectNewOutputFormat();
		break;
		} // IDC_FORMATDROP
	default:
		break;
	} // switch CtrlID

return (0);

} // EDSSControlGUI::HandleCBChange

/*===========================================================================*/

long EDSSControlGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

return (0);

} // EDSSControlGUI::HandleStringLoseFocus

/*===========================================================================*/

long EDSSControlGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
AnimDoubleTime *AnimPtr = NULL;
short ZoomChanged = 0;

if(ScrollCode)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			ControlHost->FirstCamElevADT.SetValue((double)(5000 - ScrollPos));
			AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
			AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamElevADT);
			WidgetSNSync(IDC_CAMALT1, WP_FISYNC_NONOTIFY);
			break;
			}
		case IDC_SCROLLBAR2:
			{
			ControlHost->MidCamElevADT.SetValue((double)(5000 - ScrollPos));
			AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
			AnimPtr->Copy(AnimPtr, &ControlHost->MidCamElevADT);
			WidgetSNSync(IDC_CAMALT2, WP_FISYNC_NONOTIFY);
			break;
			}
		case IDC_SCROLLBAR3:
			{
			ControlHost->EndCamElevADT.SetValue((double)(5000 - ScrollPos));
			AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
			AnimPtr->Copy(AnimPtr, &ControlHost->EndCamElevADT);
			WidgetSNSync(IDC_CAMALT3, WP_FISYNC_NONOTIFY);
			break;
			}
		case IDC_SCROLLBAR4:
			{
			double HPos;

			HPos = ScrollPos * 0.01;	// was / 100.0

			HPos *= (1.0 - SwatchData0.Visible);
			SwatchData0.OffsetX = HPos;
			ConfigureRD(NativeWin, IDC_CAMPOS1, &SwatchData0);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR7:
			{
			double HPos;

			HPos = ScrollPos * 0.01;	// was / 100.0

			HPos *= (1.0 - SwatchData1.Visible);
			SwatchData1.OffsetX = HPos;
			ConfigureRD(NativeWin, IDC_CAMPOS2, &SwatchData1);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR10:
			{
			double HPos;

			HPos = ScrollPos * 0.01;	// was / 100.0

			HPos *= (1.0 - SwatchData2.Visible);
			SwatchData2.OffsetX = HPos;
			ConfigureRD(NativeWin, IDC_CAMPOS3, &SwatchData2);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR5:
			{
			double VPos;

			VPos = ScrollPos * 0.01;	// was / 100.0

			VPos *= (1.0 - SwatchData0.Visible);
			SwatchData0.OffsetY = VPos;
			ConfigureRD(NativeWin, IDC_CAMPOS1, &SwatchData0);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR8:
			{
			double VPos;

			VPos = ScrollPos * 0.01;	// was / 100.0

			VPos *= (1.0 - SwatchData1.Visible);
			SwatchData1.OffsetY = VPos;
			ConfigureRD(NativeWin, IDC_CAMPOS2, &SwatchData1);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR11:
			{
			double VPos;

			VPos = ScrollPos * 0.01;	// was / 100.0

			VPos *= (1.0 - SwatchData2.Visible);
			SwatchData2.OffsetY = VPos;
			ConfigureRD(NativeWin, IDC_CAMPOS3, &SwatchData2);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR6:
			{
			double CenterX, CenterY, NewOffsetX, NewOffsetY, MaxOffset;

			CenterX = SwatchData0.OffsetX + .5 * SwatchData0.Visible;
			CenterY = SwatchData0.OffsetY + .5 * SwatchData0.Visible;

			SwatchData0.Visible = ScrollPos * 0.01;	// was / 100.0

			NewOffsetX = SwatchData0.Visible < 1.0 ? (CenterX - SwatchData0.Visible * .5): 0.0;
			NewOffsetY = SwatchData0.Visible < 1.0 ? (CenterY - SwatchData0.Visible * .5): 0.0;

			MaxOffset = (1.0 - SwatchData0.Visible);
			SwatchData0.OffsetX = NewOffsetX > MaxOffset ? MaxOffset: NewOffsetX < 0.0 ? 0.0: NewOffsetX;
			SwatchData0.OffsetY = NewOffsetY > MaxOffset ? MaxOffset: NewOffsetY < 0.0 ? 0.0: NewOffsetY;

			ScrollPos = SwatchData0.Visible < 1.0 ? (long)(100.0 * SwatchData0.OffsetX / (1.0 - SwatchData0.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR4, ScrollPos);
			ScrollPos = SwatchData0.Visible < 1.0 ? (long)(100.0 * SwatchData0.OffsetY / (1.0 - SwatchData0.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR5, ScrollPos);

			ComputeOverlay(0);
			ConfigureRD(NativeWin, IDC_CAMPOS1, &SwatchData0);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR9:
			{
			double CenterX, CenterY, NewOffsetX, NewOffsetY, MaxOffset;

			CenterX = SwatchData1.OffsetX + .5 * SwatchData1.Visible;
			CenterY = SwatchData1.OffsetY + .5 * SwatchData1.Visible;

			SwatchData1.Visible = ScrollPos * 0.01;	// was / 100.0

			NewOffsetX = SwatchData1.Visible < 1.0 ? (CenterX - SwatchData1.Visible * .5): 0.0;
			NewOffsetY = SwatchData1.Visible < 1.0 ? (CenterY - SwatchData1.Visible * .5): 0.0;

			MaxOffset = (1.0 - SwatchData1.Visible);
			SwatchData1.OffsetX = NewOffsetX > MaxOffset ? MaxOffset: NewOffsetX < 0.0 ? 0.0: NewOffsetX;
			SwatchData1.OffsetY = NewOffsetY > MaxOffset ? MaxOffset: NewOffsetY < 0.0 ? 0.0: NewOffsetY;

			ScrollPos = SwatchData1.Visible < 1.0 ? (long)(100.0 * SwatchData1.OffsetX / (1.0 - SwatchData1.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR7, ScrollPos);
			ScrollPos = SwatchData1.Visible < 1.0 ? (long)(100.0 * SwatchData1.OffsetY / (1.0 - SwatchData1.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR8, ScrollPos);

			ComputeOverlay(1);
			ConfigureRD(NativeWin, IDC_CAMPOS2, &SwatchData1);
			ZoomChanged = 1;
			break;
			}
		case IDC_SCROLLBAR12:
			{
			double CenterX, CenterY, NewOffsetX, NewOffsetY, MaxOffset;

			CenterX = SwatchData2.OffsetX + .5 * SwatchData2.Visible;
			CenterY = SwatchData2.OffsetY + .5 * SwatchData2.Visible;

			SwatchData2.Visible = ScrollPos * 0.01;	// was / 100.0

			NewOffsetX = SwatchData2.Visible < 1.0 ? (CenterX - SwatchData2.Visible * .5): 0.0;
			NewOffsetY = SwatchData2.Visible < 1.0 ? (CenterY - SwatchData2.Visible * .5): 0.0;

			MaxOffset = (1.0 - SwatchData2.Visible);
			SwatchData2.OffsetX = NewOffsetX > MaxOffset ? MaxOffset: NewOffsetX < 0.0 ? 0.0: NewOffsetX;
			SwatchData2.OffsetY = NewOffsetY > MaxOffset ? MaxOffset: NewOffsetY < 0.0 ? 0.0: NewOffsetY;

			ScrollPos = SwatchData2.Visible < 1.0 ? (long)(100.0 * SwatchData2.OffsetX / (1.0 - SwatchData2.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR10, ScrollPos);
			ScrollPos = SwatchData2.Visible < 1.0 ? (long)(100.0 * SwatchData2.OffsetY / (1.0 - SwatchData2.Visible)): 0;
			WidgetSetScrollPos(IDC_SCROLLBAR11, ScrollPos);

			ComputeOverlay(2);
			ConfigureRD(NativeWin, IDC_CAMPOS3, &SwatchData2);
			ZoomChanged = 1;
			break;
			}
		default:
			break;
		} // switch
	if (AnimPtr)
		{
		AnimPtr->ValueChanged();
		} // if
	if (ZoomChanged)
		{
		SyncPlanCam();
		} // if
	return(0);
	} // if
else
	{
	return(50); // default scroll amount
	} // else

} // EDSSControlGUI::HandleScroll

/*===========================================================================*/

long EDSSControlGUI::HandleLeftButtonDown(int CtrlID, long int X, long int Y, char Alt, char Control, char Shift)
{

switch (CtrlID)
	{
	case IDC_CAMPOS1:
		{
		Sampling = InitCameraMotion(0, (double)X / WidSize[0][0], (double)Y / WidSize[0][1]);
		break;
		} // IDC_CAMPOS1
	case IDC_CAMPOS2:
		{
		Sampling = InitCameraMotion(1, (double)X / WidSize[1][0], (double)Y / WidSize[1][1]);
		break;
		} // IDC_CAMPOS2
	case IDC_CAMPOS3:
		{
		Sampling = InitCameraMotion(2, (double)X / WidSize[2][0], (double)Y / WidSize[2][1]);
		break;
		} // IDC_CAMPOS3
	default:
		break;
	} // switch CtrlID

return (0);

} // EDSSControlGUI::HandleLeftButtonDown

/*===========================================================================*/

long EDSSControlGUI::HandleLeftButtonUp(int CtrlID, long int X, long int Y, char Alt, char Control, char Shift)
{

if (SamplingCamera)
	SyncVectorMessage();

Sampling = 0;
SamplingCamera = 0;

return (0);

} // EDSSControlGUI::HandleLeftButtonUp

/*===========================================================================*/

long EDSSControlGUI::HandleMouseMove(int CtrlID, long int X, long int Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{

switch (CtrlID)
	{
	case IDC_CAMPOS1:
		{
		if (Sampling)
			{
			SetLatLonFromXY(0, (double)X / WidSize[0][0], (double)Y / WidSize[0][1]);
			SyncTerrainElev();
			} // if
		break;
		} // IDC_CAMPOS1
	case IDC_CAMPOS2:
		{
		if (Sampling)
			{
			SetLatLonFromXY(1, (double)X / WidSize[1][0], (double)Y / WidSize[1][1]);
			SyncTerrainElev();
			} // if
		break;
		} // IDC_CAMPOS2
	case IDC_CAMPOS3:
		{
		if (Sampling)
			{
			SetLatLonFromXY(2, (double)X / WidSize[2][0], (double)Y / WidSize[2][1]);
			SyncTerrainElev();
			} // if
		break;
		} // IDC_CAMPOS3
	default:
		break;
	} // switch CtrlID

return (0);

} // EDSSControlGUI::HandleMouseMove

/*===========================================================================*/

long EDSSControlGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				ShowPanel(0, 1);
				SetCamPage(1);
				ActivePage = NewPageID;
				SyncPlanCam();
				if (! ElevConfigured1)
					SyncTerrainElev();
				break;
				} // 1
			case 2:
				{
				ShowPanel(0, 2);
				SetCamPage(2);
				ActivePage = NewPageID;
				SyncPlanCam();
				if (! ElevConfigured2)
					SyncTerrainElev();
				break;
				} // 2
			case 3:
				{
				ShowPanel(0, 3);
				break;
				} // 3
			case 4:
				{
				ShowPanel(0, 4);
				break;
				} // 4
			case 5:
				{
				ShowPanel(0, 5);
				break;
				} // 5
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				SetCamPage(0);
				ActivePage = NewPageID;
				SyncPlanCam();
				break;
				} // 0
			} // switch
		break;
		}
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // EDSSControlGUI::HandlePageChange

/*===========================================================================*/

long EDSSControlGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKMIDKEY:
	case IDC_CHECKENDKEY:
		{
		DisableWidgets();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // EDSSControlGUI::HandleSCChange

/*===========================================================================*/

long EDSSControlGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
AnimDoubleTime *AnimPtr = NULL;
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_CAMLAT1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamLatADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMLAT2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamLatADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMLAT3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamLatADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMLON1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamLonADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMLON2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamLonADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMLON3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamLonADT);
		SyncTerrainElev();
		break;
		} // 
	case IDC_CAMALT1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamElevADT);
		SetScrollPos(0);
		break;
		} // 
	case IDC_CAMALT2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamElevADT);
		SetScrollPos(1);
		break;
		} // 
	case IDC_CAMALT3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamElevADT);
		SetScrollPos(2);
		break;
		} // 
	case IDC_FOCLAT1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargLatADT);
		break;
		} // 
	case IDC_FOCLAT2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargLatADT);
		break;
		} // 
	case IDC_FOCLAT3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargLatADT);
		break;
		} // 
	case IDC_FOCLON1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargLonADT);
		break;
		} // 
	case IDC_FOCLON2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargLonADT);
		break;
		} // 
	case IDC_FOCLON3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargLonADT);
		break;
		} // 
	case IDC_FOCALT1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargElevADT);
		break;
		} // 
	case IDC_FOCALT2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargElevADT);
		break;
		} // 
	case IDC_FOCALT3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargElevADT);
		break;
		} // 
	case IDC_VIEWARC1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstFOV);
		break;
		} // 
	case IDC_VIEWARC2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidFOV);
		break;
		} // 
	case IDC_VIEWARC3:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndFOV);
		break;
		} // 
	case IDC_IMAGEWIDTH:
	case IDC_IMAGEHEIGHT:
		{
		if (ActiveOpt->LockAspect)
			{
			if (CtrlID == IDC_IMAGEWIDTH)
				ActiveOpt->OutputImageHeight = (long)(.5 + ActiveOpt->OutputImageWidth  / ActiveOpt->ImageAspectRatio);
			else
				ActiveOpt->OutputImageWidth = (long)(.5 + ActiveOpt->OutputImageHeight * ActiveOpt->ImageAspectRatio);
			if (ActiveOpt->OutputImageHeight < 1)
				ActiveOpt->OutputImageHeight = 1;
			if (ActiveOpt->OutputImageWidth < 1)
				ActiveOpt->OutputImageWidth = 1;
			} // if
		} // 
		//lint -fallthrough
	case IDC_SEGMENT:
		{
		#ifdef WCS_BUILD_DEMO
		if (ActiveOpt->OutputImageWidth > 640)
			ActiveOpt->OutputImageWidth = 640;
		if (ActiveOpt->OutputImageHeight > 450)
			ActiveOpt->OutputImageHeight = 450;
		#endif // WCS_BUILD_DEMO
		Changes[0] = MAKE_ID(ActiveOpt->GetNotifyClass(), ActiveOpt->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveOpt->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

if (AnimPtr)
	{
	AnimPtr->ValueChanged();
	} // if

return(0);

} // EDSSControlGUI::HandleFIChange

/*===========================================================================*/

long EDSSControlGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(ActiveOpt->GetNotifyClass(), ActiveOpt->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, ActiveOpt->GetRAHostRoot());

return(0);

} // EDSSControlGUI::HandleDDChange

/*===========================================================================*/

void EDSSControlGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[3];
int Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_RCG, 0);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	RenderComplete();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		{
		CaptureDiagnostics((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // EDSSControlGUI::HandleNotifyEvent()

/*===========================================================================*/

void EDSSControlGUI::ConfigureWidgets(void)
{

if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
	{
	WidgetSetText(IDC_MIDTIME, "Frame of Middle Position ");
	WidgetSetText(IDC_ENDTIME, "Frame of Ending Position ");
	} // if
else
	{
	WidgetSetText(IDC_MIDTIME, "Time of Middle Position ");
	WidgetSetText(IDC_ENDTIME, "Time of Ending Position ");
	} // else

ConfigureSC(NativeWin, IDC_CHECKMIDKEY, &ControlHost->MidKeyEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKENDKEY, &ControlHost->EndKeyEnabled, SCFlag_Char, NULL, 0);

WidgetSNConfig(IDC_CAMLAT1, &ControlHost->FirstCamLatADT);
WidgetSNConfig(IDC_CAMLON1, &ControlHost->FirstCamLonADT);
WidgetSNConfig(IDC_CAMALT1, &ControlHost->FirstCamElevADT);
WidgetSNConfig(IDC_FOCLAT1, &ControlHost->FirstTargLatADT);
WidgetSNConfig(IDC_FOCLON1, &ControlHost->FirstTargLonADT);
WidgetSNConfig(IDC_FOCALT1, &ControlHost->FirstTargElevADT);
WidgetSNConfig(IDC_VIEWARC1, &ControlHost->FirstFOV);
WidgetSNConfig(IDC_VIEWARC1, &ControlHost->FirstFOV);
WidgetSNConfig(IDC_VIEWARC1, &ControlHost->FirstFOV);
WidgetSmartRAHConfig(IDC_ZMINIMUM, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN], ActiveCam);
WidgetSmartRAHConfig(IDC_ZMAXIMUM, &ActiveCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX], ActiveCam);

WidgetSNConfig(IDC_CAMLAT2, &ControlHost->MidCamLatADT);
WidgetSNConfig(IDC_CAMLON2, &ControlHost->MidCamLonADT);
WidgetSNConfig(IDC_CAMALT2, &ControlHost->MidCamElevADT);
WidgetSNConfig(IDC_FOCLAT2, &ControlHost->MidTargLatADT);
WidgetSNConfig(IDC_FOCLON2, &ControlHost->MidTargLonADT);
WidgetSNConfig(IDC_FOCALT2, &ControlHost->MidTargElevADT);
WidgetSNConfig(IDC_VIEWARC2, &ControlHost->MidFOV);
WidgetSNConfig(IDC_MIDTIME, &ControlHost->MidTime);

WidgetSNConfig(IDC_CAMLAT3, &ControlHost->EndCamLatADT);
WidgetSNConfig(IDC_CAMLON3, &ControlHost->EndCamLonADT);
WidgetSNConfig(IDC_CAMALT3, &ControlHost->EndCamElevADT);
WidgetSNConfig(IDC_FOCLAT3, &ControlHost->EndTargLatADT);
WidgetSNConfig(IDC_FOCLON3, &ControlHost->EndTargLonADT);
WidgetSNConfig(IDC_FOCALT3, &ControlHost->EndTargElevADT);
WidgetSNConfig(IDC_VIEWARC3, &ControlHost->EndFOV);
WidgetSNConfig(IDC_ENDTIME, &ControlHost->EndTime);

WidgetSmartRAHConfig(IDC_PLANET_VERTICALEXAG, &ActivePlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG], ActivePlanetOpt);
WidgetSNConfig(IDC_PIXELASPECT, &ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT]);

ConfigureFI(NativeWin, IDC_SEGMENT,
 &ActiveOpt->RenderImageSegments,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_IMAGEWIDTH,
 &ActiveOpt->OutputImageWidth,
  1.0,
   1.0,
#ifdef WCS_BUILD_DEMO
	640.0,
#else // WCS_BUILD_DEMO
	100000.0,
#endif // WCS_BUILD_DEMO
	 FIOFlag_Long,
	  NULL,
	   0);

#ifdef WCS_BUILD_DEMO

#endif // WCS_BUILD_DEMO

ConfigureFI(NativeWin, IDC_IMAGEHEIGHT,
 &ActiveOpt->OutputImageHeight,
  1.0,
   1.0,
#ifdef WCS_BUILD_DEMO
	450.0,
#else // WCS_BUILD_DEMO
	100000.0,
#endif // WCS_BUILD_DEMO
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureDD(NativeWin, IDC_TEMPPATH, (char *)ActiveOpt->TempPath.GetPath(), 255, NULL, 0, IDC_LABEL_TEMP);

ConfigureTB(NativeWin, IDC_CONSTRAIN, IDI_CONSTRASP, NULL);
WidgetSetCheck(IDC_CONSTRAIN, ActiveOpt->LockAspect);

if (! TNailsConfigured)
	ConfigureTNails();
if (! SwatchConfigured)
	ConfigureColorSwatch();
ConfigureSizeDrop();
ConfigureOutputEvent();

DisableWidgets();

} // EDSSControlGUI::ConfigureWidgets()

/*===========================================================================*/

void EDSSControlGUI::ConfigureTNails(void)
{
char *Title, NameStr[512];
long Ct;

for (Ct = 0; Ct < WCS_EDSS_NUMNAILS; Ct ++)
	{
	switch (Ct)
		{
		case 0:
			{
			Title = "7AM.lgt";
			break;
			} // 0
		case 1:
			{
			Title = "9AM.lgt";
			break;
			} // 1
		case 2:
			{
			Title = "Noon.lgt";
			break;
			} // 2
		case 3:
			{
			Title = "3PM.lgt";
			break;
			} // 3
		case 4:
			{
			Title = "5PM.lgt";
			break;
			} // 4
		case 5:
			{
			Title = "9PM.lgt";
			break;
			} // 5
		case 6:
			{
			Title = "Visibility Infinite.atm";
			break;
			} // 6
		case 7:
			{
			Title = "Visibility 50Km.atm";
			break;
			} // 7
		case 8:
			{
			Title = "Visibility 1Km.atm";
			break;
			} // 8
		case 9:
			{
			Title = "Visibility 250m.atm";
			break;
			} // 9
		case 10:
			{
			Title = "Clear.sky";
			break;
			} // 10
		case 11:
			{
			Title = "Overcast.sky";
			break;
			} // 11
		case 12:
			{
			Title = "Night.sky";
			break;
			} // 12
		} // switch
	if (NameList[Ct] = (char *)AppMem_Alloc(strlen(Title) + 1, APPMEM_CLEAR))
		{
		strcpy(NameList[Ct], Title);
		// read browse data
		strmfp(NameStr, "WCSProjects:Components", NameList[Ct]);
		if (BrowseList[Ct].LoadBrowseInfo(NameStr))
			{
			StripExtension(NameList[Ct]);
			} // if
		} // if
	RasterList[Ct].Thumb = BrowseList[Ct].Thumb;
	} // for

WidgetSetText(IDC_TITLE1, NameList[0] ? NameList[0]: "");
ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, &RasterList[0]);
WidgetSetText(IDC_TITLE2, NameList[1] ? NameList[1]: "");
ConfigureTB(NativeWin, IDC_TNAIL2, NULL, NULL, &RasterList[1]);
WidgetSetText(IDC_TITLE3, NameList[2] ? NameList[2]: "");
ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, &RasterList[2]);
WidgetSetText(IDC_TITLE4, NameList[3] ? NameList[3]: "");
ConfigureTB(NativeWin, IDC_TNAIL4, NULL, NULL, &RasterList[3]);
WidgetSetText(IDC_TITLE5, NameList[4] ? NameList[4]: "");
ConfigureTB(NativeWin, IDC_TNAIL5, NULL, NULL, &RasterList[4]);
WidgetSetText(IDC_TITLE6, NameList[5] ? NameList[5]: "");
ConfigureTB(NativeWin, IDC_TNAIL6, NULL, NULL, &RasterList[5]);
WidgetSetText(IDC_TITLE7, NameList[6] ? NameList[6]: "");
ConfigureTB(NativeWin, IDC_TNAIL7, NULL, NULL, &RasterList[6]);
WidgetSetText(IDC_TITLE8, NameList[7] ? NameList[7]: "");
ConfigureTB(NativeWin, IDC_TNAIL8, NULL, NULL, &RasterList[7]);
WidgetSetText(IDC_TITLE9, NameList[8] ? NameList[8]: "");
ConfigureTB(NativeWin, IDC_TNAIL9, NULL, NULL, &RasterList[8]);
WidgetSetText(IDC_TITLE10, NameList[9] ? NameList[9]: "");
ConfigureTB(NativeWin, IDC_TNAIL10, NULL, NULL, &RasterList[9]);
WidgetSetText(IDC_TITLE11, NameList[10] ? NameList[10]: "");
ConfigureTB(NativeWin, IDC_TNAIL11, NULL, NULL, &RasterList[10]);
WidgetSetText(IDC_TITLE12, NameList[11] ? NameList[11]: "");
ConfigureTB(NativeWin, IDC_TNAIL12, NULL, NULL, &RasterList[11]);
WidgetSetText(IDC_TITLE13, NameList[12] ? NameList[12]: "");
ConfigureTB(NativeWin, IDC_TNAIL13, NULL, NULL, &RasterList[12]);

TNailsConfigured = 1;

} // EDSSControlGUI::ConfigureTNails

/*===========================================================================*/

void EDSSControlGUI::ConfigureColorSwatch(void)
{
RasterAttribute *MyAttr;
int Configured = 0, AllocError = 0;

if (SwatchData0.MainRast || (SwatchData0.MainRast = ImageHost->FindByUserName("Base Image")))
	{
	SwatchData1.MainRast = SwatchData2.MainRast = SwatchData0.MainRast;
	if ((MyAttr = SwatchData0.MainRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && (GeoShell = (GeoRefShell *)MyAttr->GetShell()))
		{
		if (SwatchData0.MainRast->LoadnProcessImage(0))
			{
			SwatchData0.OverRast->Rows = 224;
			SwatchData0.OverRast->Cols = 220;
			SwatchData0.OverRast->ByteBands = 3;
			if (SwatchData0.OverRast->AllocByteBand(0))
				{
				SwatchData0.OverRast->ClearByteBand(0);
				if (SwatchData0.OverRast->AllocByteBand(1))
					{
					SwatchData0.OverRast->ClearByteBand(1);
					if (SwatchData0.OverRast->AllocByteBand(2))
						{
						SwatchData0.OverRast->ClearByteBand(2);
						} // if
					else
						AllocError = 1;
					} // if
				else
					AllocError = 1;
				} // if
			else
				AllocError = 1;

			SwatchData1.OverRast->Rows = 224;
			SwatchData1.OverRast->Cols = 220;
			SwatchData1.OverRast->ByteBands = 3;
			if (SwatchData1.OverRast->AllocByteBand(0))
				{
				SwatchData1.OverRast->ClearByteBand(0);
				if (SwatchData1.OverRast->AllocByteBand(1))
					{
					SwatchData1.OverRast->ClearByteBand(1);
					if (SwatchData1.OverRast->AllocByteBand(2))
						{
						SwatchData1.OverRast->ClearByteBand(2);
						} // if
					else
						AllocError = 1;
					} // if
				else
					AllocError = 1;
				} // if
			else
				AllocError = 1;

			SwatchData2.OverRast->Rows = 224;
			SwatchData2.OverRast->Cols = 220;
			SwatchData2.OverRast->ByteBands = 3;
			if (SwatchData2.OverRast->AllocByteBand(0))
				{
				SwatchData2.OverRast->ClearByteBand(0);
				if (SwatchData2.OverRast->AllocByteBand(1))
					{
					SwatchData2.OverRast->ClearByteBand(1);
					if (SwatchData2.OverRast->AllocByteBand(2))
						{
						SwatchData2.OverRast->ClearByteBand(2);
						} // if
					else
						AllocError = 1;
					} // if
				else
					AllocError = 1;
				} // if
			else
				AllocError = 1;

			if (! AllocError)
				{
				Configured = 1;
				ComputeOverlay(0);
				ComputeOverlay(1);
				ComputeOverlay(2);
				ConfigureRD(NativeWin, IDC_CAMPOS1, &SwatchData0);
				ConfigureRD(NativeWin, IDC_CAMPOS2, &SwatchData1);
				ConfigureRD(NativeWin, IDC_CAMPOS3, &SwatchData2);
				} // if
			} // if
		} // if
	} // if

if (! Configured)
	{
	ConfigureRD(NativeWin, IDC_CAMPOS1, NULL);
	ConfigureRD(NativeWin, IDC_CAMPOS2, NULL);
	ConfigureRD(NativeWin, IDC_CAMPOS3, NULL);
	} // else

SwatchConfigured = 1;

} // EDSSControlGUI::ConfigureColorSwatch

/*===========================================================================*/

void EDSSControlGUI::ConfigureSizeDrop(void)
{
int Preset;
int TestW, TestH;
double TestA;

// Identify closest preset
TestW = ActiveOpt->OutputImageWidth;
TestH = ActiveOpt->OutputImageHeight;
TestA = ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

for (Preset = 0; ; Preset++)
	{
	if(RPresets[Preset].Aspect == 0.0)
		{ // end of list, break and set to Custom
		break;
		} // if
	if((RPresets[Preset].Aspect == TestA) &&
	   (RPresets[Preset].Width  == TestW) &&
	   (RPresets[Preset].Height == TestH))
		{
		break;
		} // if
	if((RPresets[Preset].Aspect == TestA) &&
	   (RPresets[Preset].Width  == 0) &&
	   (RPresets[Preset].Height == 0))
		{
		break;
		} // if
	} // for

WidgetCBSetCurSel(IDC_RENDERPRESETS, Preset);


} // EDSSControlGUI::ConfigureSizeDrop

/*===========================================================================*/

void EDSSControlGUI::ConfigureOutputEvent(void)
{
long NumCBEntries, Ct;
char FormatStr[WCS_MAX_FILETYPELENGTH];

if (ActiveOpt->OutputEvents)
	{
	ConfigureDD(NativeWin, IDC_SAVEPATH, (char *)ActiveOpt->OutputEvents->PAF.GetPath(), 255, (char *)ActiveOpt->OutputEvents->PAF.GetName(), 63, NULL);
	WidgetSetDisabled(IDC_SAVEPATH, 0);

	NumCBEntries = WidgetCBGetCount(IDC_FORMATDROP);
	for (Ct = 0; Ct < NumCBEntries; Ct ++)
		{
		WidgetCBGetText(IDC_FORMATDROP, Ct, FormatStr);
		if (! stricmp(ActiveOpt->OutputEvents->FileType, FormatStr))
			{
			WidgetCBSetCurSel(IDC_FORMATDROP, Ct);
			break;
			} // if
		} // for 
	} // if
else
	{
	ConfigureDD(NativeWin, IDC_SAVEPATH, " ", 1, " ", 1, NULL);
	WidgetSetDisabled(IDC_SAVEPATH, 1);
	} // else

} // EDSSControlGUI::ConfigureOutputEvents

/*===========================================================================*/

void EDSSControlGUI::ConfigureInitialComponents(void)
{

WidgetSetCheck(IDC_TNAIL4, 1);
WidgetSetCheck(IDC_TNAIL8, 1);
WidgetSetCheck(IDC_TNAIL11, 1);
LoadComponent(IDC_TNAIL4);
LoadComponent(IDC_TNAIL8);
LoadComponent(IDC_TNAIL11);

} // EDSSControlGUI::ConfigureInitialComponents

/*===========================================================================*/

void EDSSControlGUI::ConfigureOutputName(void)
{

if (ActiveOpt && ActiveOpt->OutputEvents)
	{
	ActiveOpt->OutputEvents->PAF.SetName(GlobalApp->StartEDSS->NewProjectName);
	} // if

} // EDSSControlGUI::ConfigureOutputName

/*===========================================================================*/

void EDSSControlGUI::SetScrollPos(int CamPos)
{

if (CamPos == 0 || CamPos == -1)
	WidgetSetScrollPos(IDC_SCROLLBAR1, (long)(5000.0 - ControlHost->FirstCamElevADT.GetCurValue()));
if (CamPos == 1 || CamPos == -1)
	WidgetSetScrollPos(IDC_SCROLLBAR2, (long)(5000.0 - ControlHost->MidCamElevADT.GetCurValue()));
if (CamPos == 2 || CamPos == -1)
	WidgetSetScrollPos(IDC_SCROLLBAR3, (long)(5000.0 - ControlHost->EndCamElevADT.GetCurValue()));

} // EDSSControlGUI::SetScrollPos

/*===========================================================================*/

void EDSSControlGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_CHECKMIDKEY, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_CAMLAT2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_CAMLON2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_CAMALT2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_FOCLAT2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_FOCLON2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_FOCALT2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_VIEWARC2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_MIDTIME, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_CAMPOS2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_SCROLLBAR2, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_SCROLLBAR7, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_SCROLLBAR8, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));
WidgetSetDisabled(IDC_SCROLLBAR9, ! (ControlHost->EndKeyEnabled && ControlHost->MidKeyEnabled));

WidgetSetDisabled(IDC_CAMLAT3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_CAMLON3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_CAMALT3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_FOCLAT3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_FOCLON3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_FOCALT3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_VIEWARC3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_ENDTIME, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_CAMPOS3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_SCROLLBAR3, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_SCROLLBAR10, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_SCROLLBAR11, ! ControlHost->EndKeyEnabled);
WidgetSetDisabled(IDC_SCROLLBAR12, ! ControlHost->EndKeyEnabled);

} // EDSSControlGUI::DisableWidgets

/*===========================================================================*/

void EDSSControlGUI::SetCamPage(int Page)
{
AnimDoubleTime *AnimPtr = NULL;

switch (Page)
	{
	case 0:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstCamElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstTargElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->FirstFOV);
		break;
		} // first position
	case 1:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidCamElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidTargElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->MidFOV);
		break;
		} // mid position
	case 2:
		{
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndCamElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargLatADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargLonADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndTargElevADT);
		AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
		AnimPtr->Copy(AnimPtr, &ControlHost->EndFOV);
		break;
		} // end position
	} // switch

if (AnimPtr)
	{
	AnimPtr->ValueChanged();
	} // if

} // EDSSControlGUI::SetCamPage

/*===========================================================================*/

int EDSSControlGUI::InitCameraMotion(int CamPos, double X, double Y)
{
double dX, dY;
AnimDoubleTime *LatPtr, *LonPtr;

// camera
LatPtr = CamPos == 0 ? &ControlHost->FirstCamLatADT: CamPos == 1 ? &ControlHost->MidCamLatADT: &ControlHost->EndCamLatADT;
LonPtr = CamPos == 0 ? &ControlHost->FirstCamLonADT: CamPos == 1 ? &ControlHost->MidCamLonADT: &ControlHost->EndCamLonADT;

X = CamPos == 0 ? X * SwatchData0.Visible + SwatchData0.OffsetX: CamPos == 1 ? X * SwatchData1.Visible + SwatchData1.OffsetX: X * SwatchData2.Visible + SwatchData2.OffsetX;
Y = CamPos == 0 ? Y * SwatchData0.Visible + SwatchData0.OffsetY: CamPos == 1 ? Y * SwatchData1.Visible + SwatchData1.OffsetY: Y * SwatchData2.Visible + SwatchData2.OffsetY;

if (GeoShell->SampleXY(LatPtr->GetCurValue(), LonPtr->GetCurValue(), dX, dY))
	{
	if (fabs(X - dX) < .015 && fabs(Y - dY) < .015)
		{
		SamplingCamera = 1;
		return (1);
		} // if
	} // if

// target
LatPtr = CamPos == 0 ? &ControlHost->FirstTargLatADT: CamPos == 1 ? &ControlHost->MidTargLatADT: &ControlHost->EndTargLatADT;
LonPtr = CamPos == 0 ? &ControlHost->FirstTargLonADT: CamPos == 1 ? &ControlHost->MidTargLonADT: &ControlHost->EndTargLonADT;

if (GeoShell->SampleXY(LatPtr->GetCurValue(), LonPtr->GetCurValue(), dX, dY))
	{
	if (fabs(X - dX) < .015 && fabs(Y - dY) < .015)
		{
		SamplingCamera = 0;
		return (1);
		} // if
	} // if

SamplingCamera = 0;
return (0);

} // EDSSControlGUI::InitCameraMotion

/*===========================================================================*/

void EDSSControlGUI::SetLatLonFromXY(int CamPos, double X, double Y)
{
AnimDoubleTime *AnimPtr = NULL;
double Lat, Lon, Elev;
int ElevReject;

X = CamPos == 0 ? X * SwatchData0.Visible + SwatchData0.OffsetX: CamPos == 1 ? X * SwatchData1.Visible + SwatchData1.OffsetX: X * SwatchData2.Visible + SwatchData2.OffsetX;
Y = CamPos == 0 ? Y * SwatchData0.Visible + SwatchData0.OffsetY: CamPos == 1 ? Y * SwatchData1.Visible + SwatchData1.OffsetY: Y * SwatchData2.Visible + SwatchData2.OffsetY;

if (GeoShell->SampleLatLonElev(X, Y, Lat, Lon, Elev, ElevReject))
	{
	switch (CamPos)
		{
		case 0:
			{
			if (SamplingCamera)
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->FirstCamLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
				AnimPtr->SetValue(Lon);
				ControlHost->FirstCamLonADT.SetValue(Lon);
				WidgetSNSync(IDC_CAMLAT1, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_CAMLON1, WP_FISYNC_NONOTIFY);
				} // if
			else
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->FirstTargLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
				AnimPtr->SetValue(Lon);
				ControlHost->FirstTargLonADT.SetValue(Lon);
				WidgetSNSync(IDC_FOCLAT1, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_FOCLON1, WP_FISYNC_NONOTIFY);
				} // else
			break;
			} // first position
		case 1:
			{
			if (SamplingCamera)
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->MidCamLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
				AnimPtr->SetValue(Lon);
				ControlHost->MidCamLonADT.SetValue(Lon);
				WidgetSNSync(IDC_CAMLAT2, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_CAMLON2, WP_FISYNC_NONOTIFY);
				} // if
			else
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->MidTargLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
				AnimPtr->SetValue(Lon);
				ControlHost->MidTargLonADT.SetValue(Lon);
				WidgetSNSync(IDC_FOCLAT2, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_FOCLON2, WP_FISYNC_NONOTIFY);
				} // else
			break;
			} // mid position
		case 2:
			{
			if (SamplingCamera)
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->EndCamLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
				AnimPtr->SetValue(Lon);
				ControlHost->EndCamLonADT.SetValue(Lon);
				WidgetSNSync(IDC_CAMLAT3, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_CAMLON3, WP_FISYNC_NONOTIFY);
				} // if
			else
				{
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
				AnimPtr->SetValue(Lat);
				ControlHost->EndTargLatADT.SetValue(Lat);
				AnimPtr = ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
				AnimPtr->SetValue(Lon);
				ControlHost->EndTargLonADT.SetValue(Lon);
				WidgetSNSync(IDC_FOCLAT3, WP_FISYNC_NONOTIFY);
				WidgetSNSync(IDC_FOCLON3, WP_FISYNC_NONOTIFY);
				} // else
			break;
			} // end position
		} // switch
	} // if

if (AnimPtr)
	{
	AnimPtr->ValueChanged();
	ComputeOverlay(CamPos);
	if (CamPos == 0)
		ConfigureRD(NativeWin, IDC_CAMPOS1, &SwatchData0);
	else if (CamPos == 1)
		ConfigureRD(NativeWin, IDC_CAMPOS2, &SwatchData1);
	else
		ConfigureRD(NativeWin, IDC_CAMPOS3, &SwatchData2);
	} // if

} // EDSSControlGUI::SetLatLonFromXY

/*===========================================================================*/

void EDSSControlGUI::ComputeOverlay(int CamPos)
{
long Rows, Cols, X, Y, CamX = -1, CamY = -1, i, j, FillX, FillY, Zip, Sign, NominalSize, NominalWidth, DongLen;
unsigned char *Red, *Green, *Blue;
double dX, dY, m;
RasterWidgetData *CurData;
AnimDoubleTime *LatPtr, *LonPtr;

CurData = CamPos == 0 ? &SwatchData0: CamPos == 1 ? &SwatchData1: &SwatchData2;

Rows = CurData->OverRast->Rows;
Cols = CurData->OverRast->Cols;
Red = CurData->OverRast->ByteMap[0];
Green = CurData->OverRast->ByteMap[1];
Blue = CurData->OverRast->ByteMap[2];

CurData->OverRast->ClearByteBand(0);
CurData->OverRast->ClearByteBand(1);
CurData->OverRast->ClearByteBand(2);

if (CamPos == 0)
	NominalSize = SwatchData0.Visible > .66 ? 3: SwatchData0.Visible > .33 ? 2: 1;
else if (CamPos == 1)
	NominalSize = SwatchData1.Visible > .66 ? 3: SwatchData1.Visible > .33 ? 2: 1;
else
	NominalSize = SwatchData2.Visible > .66 ? 3: SwatchData2.Visible > .33 ? 2: 1;

// camera
LatPtr = CamPos == 0 ? &ControlHost->FirstCamLatADT: CamPos == 1 ? &ControlHost->MidCamLatADT: &ControlHost->EndCamLatADT;
LonPtr = CamPos == 0 ? &ControlHost->FirstCamLonADT: CamPos == 1 ? &ControlHost->MidCamLonADT: &ControlHost->EndCamLonADT;

if (GeoShell->SampleXY(LatPtr->GetCurValue(), LonPtr->GetCurValue(), dX, dY))
	{
	X = CamX = (long int)(dX * Cols + .5);
	Y = CamY = (long int)(dY * Rows + .5);
	for (i = -NominalSize; i <= NominalSize; i ++)
		{
		FillY = Y + i;
		if (FillY >= 0 && FillY < Rows)
			{
			for (j = -NominalSize; j <= NominalSize; j ++)
				{
				FillX = X + j;
				if (FillX >= 0 && FillX < Cols)
					{
					Zip = FillY * Cols + FillX;
					Red[Zip] = (unsigned char)(255);
					Green[Zip] = (unsigned char)(0);
					Blue[Zip] = (unsigned char)(0);
					} // if
				} // for
			} // if
		} // for
	} // if

// target
LatPtr = CamPos == 0 ? &ControlHost->FirstTargLatADT: CamPos == 1 ? &ControlHost->MidTargLatADT: &ControlHost->EndTargLatADT;
LonPtr = CamPos == 0 ? &ControlHost->FirstTargLonADT: CamPos == 1 ? &ControlHost->MidTargLonADT: &ControlHost->EndTargLonADT;

NominalSize ++;
if (CamPos == 0)
	{
	NominalWidth = SwatchData0.Visible > .5 ? 1: 0;
	DongLen = SwatchData0.Visible > .5 ? 10: 5;
	} // if
else if (CamPos == 1)
	{
	NominalWidth = SwatchData1.Visible > .5 ? 1: 0;
	DongLen = SwatchData1.Visible > .5 ? 10: 5;
	} // if
else
	{
	NominalWidth = SwatchData2.Visible > .5 ? 1: 0;
	DongLen = SwatchData2.Visible > .5 ? 10: 5;
	} // else

if (GeoShell->SampleXY(LatPtr->GetCurValue(), LonPtr->GetCurValue(), dX, dY))
	{
	X = (long int)(dX * Cols + .5);
	Y = (long int)(dY * Rows + .5);
	for (j = -NominalWidth; j <= NominalWidth; j ++)
		{
		FillX = X + j;
		if (FillX >= 0 && FillX < Cols)
			{
			for (i = -NominalSize; i <= NominalSize; i ++)
				{
				FillY = Y + i;
				if (FillY >= 0 && FillY < Rows)
					{
					Zip = FillY * Cols + FillX;
					Red[Zip] = (unsigned char)(0);
					Green[Zip] = (unsigned char)(255);
					Blue[Zip] = (unsigned char)(0);
					} // if
				} // for
			} // if
		} // for
	for (i = -NominalWidth; i <= NominalWidth; i ++)
		{
		FillY = Y + i;
		if (FillY >= 0 && FillY < Rows)
			{
			for (j = -NominalSize; j <= NominalSize; j ++)
				{
				FillX = X + j;
				if (FillX >= 0 && FillX < Cols)
					{
					Zip = FillY * Cols + FillX;
					Red[Zip] = (unsigned char)(0);
					Green[Zip] = (unsigned char)(255);
					Blue[Zip] = (unsigned char)(0);
					} // if
				} // for
			} // if
		} // for

	// line from camera towards target
	if (CamX >= 0 && CamY >= 0)
		{
		if (abs(X - CamX) > abs(Y - CamY))
			{
			Sign = X > CamX ? 1: -1;
			m = ((double)Y - CamY) / ((double)X - CamX) * Sign;
			for (FillX = CamX, i = 0, dY = CamY; FillX != X && i < DongLen; FillX += Sign, i ++, dY += m)
				{
				if (FillX >= 0 && FillX < Cols)
					{
					FillY = (long)dY;
					if (FillY >= 0 && FillY < Rows)
						{
						Zip = FillY * Cols + FillX;
						Red[Zip] = (unsigned char)(0);
						Green[Zip] = (unsigned char)(255);
						Blue[Zip] = (unsigned char)(0);
						} // if
					} // if
				} // for
			} // if increment X
		else if (Y != CamY)
			{
			Sign = Y > CamY ? 1: -1;
			m = ((double)X - CamX) / ((double)Y - CamY) * Sign;
			for (FillY = CamY, i = 0, dX = CamX; FillY != Y && i < DongLen; FillY += Sign, i ++, dX += m)
				{
				if (FillY >= 0 && FillY < Rows)
					{
					FillX = (long)dX;
					if (FillX >= 0 && FillX < Cols)
						{
						Zip = FillY * Cols + FillX;
						Red[Zip] = (unsigned char)(0);
						Green[Zip] = (unsigned char)(255);
						Blue[Zip] = (unsigned char)(0);
						} // if
					} // if
				} // for
			} // else increment Y

		} // if

	} // if

} // EDSSControlGUI::ComputeOverlay

/*===========================================================================*/

void EDSSControlGUI::SelectNewResolution(void)
{
double RangeDefaults[3];
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_RENDERPRESETS);
if (Current != CustomRes)
	{
	if (RPresets[Current].Width)
		ActiveOpt->OutputImageWidth = RPresets[Current].Width;
	if (RPresets[Current].Height)
		ActiveOpt->OutputImageHeight = RPresets[Current].Height;
	if (RPresets[Current].Aspect > 0.0)
		ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(RPresets[Current].Aspect);
	if (RPresets[Current].FrameRate > 0.0)
		ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].SetValue(RPresets[Current].FrameRate);

	#ifdef WCS_BUILD_DEMO
	RangeDefaults[0] = 5.0;
	#else // WCS_BUILD_DEMO
	RangeDefaults[0] = FLT_MAX;
	#endif // WCS_BUILD_DEMO
	RangeDefaults[1] = 0.0;
	RangeDefaults[2] = ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue > 0.0 ? 1.0 / ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue: 1.0 / 30.0;
	ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetRangeDefaults(RangeDefaults);
	ActiveOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetRangeDefaults(RangeDefaults);

	SetConstrain();

	Changes[0] = MAKE_ID(ActiveOpt->GetNotifyClass(), ActiveOpt->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveOpt->GetRAHostRoot());
	} // if

} // EDSSControlGUI::SelectNewResolution

/*===========================================================================*/

void EDSSControlGUI::SelectNewOutputFormat(void)
{
long Selected, BufCt;
char FormatStr[WCS_MAX_FILETYPELENGTH];
char *DefBuf;

if (ActiveOpt->OutputEvents)
	{
	Selected = WidgetCBGetCurSel(IDC_FORMATDROP);

	WidgetCBGetText(IDC_FORMATDROP, Selected, FormatStr);

	if (strcmp(FormatStr, ActiveOpt->OutputEvents->FileType))
		{
		if (ImageSaverLibrary::GetPlanOnly(FormatStr))
			{
			if (! UserMessageOKCAN(FormatStr, "This output format is suitable only for rendering with Planimetric\n type Cameras. Continue with format selection?"))
				{
				ConfigureOutputEvent();
				return;
				} // if
			} // if WCS DEM
		// set new file type
		strcpy(ActiveOpt->OutputEvents->FileType, FormatStr);

		// clear old values
		for (BufCt = 0; BufCt < WCS_MAX_IMAGEOUTBUFFERS; BufCt ++)
			ActiveOpt->OutputEvents->OutBuffers[BufCt][0] = 0;
		ActiveOpt->OutputEvents->Codec[0] = 0;

		// set new values for buffers and codec
		BufCt = 0;
		DefBuf = NULL;
		while (DefBuf = ImageSaverLibrary::GetNextDefaultBuffer(FormatStr, DefBuf))
			strcpy(ActiveOpt->OutputEvents->OutBuffers[BufCt ++], DefBuf);
		if (DefBuf = ImageSaverLibrary::GetNextCodec(FormatStr, NULL))
			strcpy(ActiveOpt->OutputEvents->Codec, DefBuf);
			
		ConfigureOutputEvent();
		} // if file type actually changed
	} // if

} // EDSSControlGUI::SelectNewOutputFormat

/*===========================================================================*/

void EDSSControlGUI::ScaleSize(void)
{
char Str[64];
double Factor = 1.0, Test;
NotifyTag Changes[2];

Str[0] = 0;

if (GetInputString("Enter a scale factor, percent or letter (such as \"h\" for half).", "", Str) && Str[0])
	{
	if (Str[strlen(Str) - 1] == '%')
		{
		Factor = atof(Str) / 100.0;
		if (Factor <= 0.0)
			{
			if (Factor <= -1.0 || Factor == 0.0)
				Factor = 1.0;
			else
				Factor = 1.0 + Factor;
			} // if
		} // if
	else
		{
		switch (Str[0])
			{
			case 'h':
			case 'H':
				{
				Factor = .5;
				break;
				} // half
			case 'q':
			case 'Q':
				{
				Factor = .25;
				break;
				} // half
			case 'd':
			case 'D':
				{
				Factor = 2.0;
				break;
				} // half
			case 't':
			case 'T':
				{
				if (Str[1] == 'h' || Str[1] == 'H')
					Factor = .33334;
				else
					Factor = 3.0;
				break;
				} // half
			case 'f':
				{
				Factor = 4.0;
				break;
				} // half
			default:
				{
				Test = atof(Str);
				if (Test > -100.0 && Test <= -1.0)
					{
					Factor = (100.0 + Test) / 100.0;
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > 0.0 && Test <= 10.0)
					{
					Factor = Test;
					} // if
				else if (Test > 10.0)
					{
					Factor = Test / 100.0;
					} // if
				break;
				} // half

			} // switch
		} // else

	ActiveOpt->OutputImageWidth = (long)(.5 + Factor * ActiveOpt->OutputImageWidth);
	ActiveOpt->OutputImageHeight = (long)(.5 + Factor * ActiveOpt->OutputImageHeight);

	if (ActiveOpt->OutputImageWidth < 1)
		ActiveOpt->OutputImageWidth = 1;
	if (ActiveOpt->OutputImageHeight < 1)
		ActiveOpt->OutputImageHeight = 1;
	#ifdef WCS_BUILD_DEMO
	if (ActiveOpt->OutputImageWidth > 640)
		ActiveOpt->OutputImageWidth = 640;
	if (ActiveOpt->OutputImageHeight > 450)
		ActiveOpt->OutputImageHeight = 450;
	#endif // WCS_BUILD_DEMO
	Changes[0] = MAKE_ID(ActiveOpt->GetNotifyClass(), ActiveOpt->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveOpt->GetRAHostRoot());
	} // if

} // EDSSControlGUI::ScaleSize

/*===========================================================================*/

void EDSSControlGUI::SetConstrain(void)
{

ActiveOpt->LockAspect = WidgetGetCheck(IDC_CONSTRAIN);

ActiveOpt->ImageAspectRatio = (double)ActiveOpt->OutputImageWidth / ActiveOpt->OutputImageHeight;

} // EDSSControlGUI::SetConstrain

/*===========================================================================*/

void EDSSControlGUI::LoadComponent(int ButtonID)
{
RasterAnimHost *LoadTo;
int LoadItem;
char filename[256], *DefaultExt;
long Result;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;

switch (ButtonID)
	{
	case IDC_TNAIL1:
		{
		LoadItem = 0;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL2:
		{
		LoadItem = 1;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL3:
		{
		LoadItem = 2;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL4:
		{
		LoadItem = 3;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL5:
		{
		LoadItem = 4;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL6:
		{
		LoadItem = 5;
		LoadTo = ActiveLight;
		DefaultExt = ".lgt";
		break;
		} // 
	case IDC_TNAIL7:
		{
		LoadItem = 6;
		LoadTo = ActiveAtmo;
		DefaultExt = ".atm";
		break;
		} // 
	case IDC_TNAIL8:
		{
		LoadItem = 7;
		LoadTo = ActiveAtmo;
		DefaultExt = ".atm";
		break;
		} // 
	case IDC_TNAIL9:
		{
		LoadItem = 8;
		LoadTo = ActiveAtmo;
		DefaultExt = ".atm";
		break;
		} // 
	case IDC_TNAIL10:
		{
		LoadItem = 9;
		LoadTo = ActiveAtmo;
		DefaultExt = ".atm";
		break;
		} // 
	case IDC_TNAIL11:
		{
		LoadItem = 10;
		LoadTo = ActiveSky;
		DefaultExt = ".sky";
		break;
		} // 
	case IDC_TNAIL12:
		{
		LoadItem = 11;
		LoadTo = ActiveSky;
		DefaultExt = ".sky";
		break;
		} // 
	case IDC_TNAIL13:
		{
		LoadItem = 12;
		LoadTo = ActiveSky;
		DefaultExt = ".sky";
		break;
		} // 
	} // switch

if (LoadTo)
	{
	strmfp(filename, "WCSProjects:Components", NameList[LoadItem]);
	strcat(filename, DefaultExt);
	Prop.Path = filename;
	Prop.Queries = 0;
	Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
	if ((Result = LoadTo->LoadFilePrep(&Prop)) > 0)
		{
		Changes[0] = MAKE_ID(LoadTo->GetNotifyClass(), LoadTo->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, LoadTo->GetRAHostRoot());
		} // if
	} // if

} // EDSSControlGUI::LoadComponent

/*===========================================================================*/

void EDSSControlGUI::InitiateRender(void)
{

TransferCamera();
Rendering = 1;
ReceivingDiagnostics = 0;
DisableRenderWidgets(1);
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_ITEM_RCG, 0);

} // EDSSControlGUI::InitiateRender

/*===========================================================================*/

void EDSSControlGUI::TransferCamera(void)
{

// copy camera settings to camera
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV)->ReleaseNodes();
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT)->AddNode(0.0, ControlHost->FirstCamLatADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON)->AddNode(0.0, ControlHost->FirstCamLonADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV)->AddNode(0.0, ControlHost->FirstCamElevADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT)->AddNode(0.0, ControlHost->FirstTargLatADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON)->AddNode(0.0, ControlHost->FirstTargLonADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)->AddNode(0.0, ControlHost->FirstTargElevADT.GetCurValue(), 0.0);
ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV)->AddNode(0.0, ControlHost->FirstFOV.GetCurValue(), 0.0);
ActiveOpt->GetAnimPtr(WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME)->SetValue(0.0);
if (ControlHost->EndKeyEnabled)
	{
	if (ControlHost->MidKeyEnabled)
		{
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidCamLatADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidCamLonADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidCamElevADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidTargLatADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidTargLonADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidTargElevADT.GetCurValue(), 0.0);
		ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV)->AddNode(ControlHost->MidTime.GetCurValue(), ControlHost->MidFOV.GetCurValue(), 0.0);
		} // if
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndCamLatADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndCamLonADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndCamElevADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndTargLatADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndTargLonADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndTargElevADT.GetCurValue(), 0.0);
	ActiveCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV)->AddNode(ControlHost->EndTime.GetCurValue(), ControlHost->EndFOV.GetCurValue(), 0.0);
	ActiveOpt->GetAnimPtr(WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME)->SetValue(ControlHost->EndTime.GetCurValue());
	} // if
else
	{
	ActiveOpt->GetAnimPtr(WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME)->SetValue(0.0);
	} // else

} // EDSSControlGUI::TransferCamera

/*===========================================================================*/

void EDSSControlGUI::DisableRenderWidgets(char Disable)
{

WidgetSetDisabled(IDC_TAB1, Disable);
WidgetSetDisabled(IDC_SCROLLBAR1, Disable);
WidgetSetDisabled(IDC_SCROLLBAR2, Disable);
WidgetSetDisabled(IDC_SCROLLBAR3, Disable);
WidgetSetDisabled(IDC_SCROLLBAR4, Disable);
WidgetSetDisabled(IDC_SCROLLBAR5, Disable);
WidgetSetDisabled(IDC_SCROLLBAR6, Disable);
WidgetSetDisabled(IDC_SCROLLBAR7, Disable);
WidgetSetDisabled(IDC_SCROLLBAR8, Disable);
WidgetSetDisabled(IDC_SCROLLBAR9, Disable);
WidgetSetDisabled(IDC_SCROLLBAR10, Disable);
WidgetSetDisabled(IDC_SCROLLBAR11, Disable);
WidgetSetDisabled(IDC_SCROLLBAR12, Disable);
WidgetSetDisabled(IDC_RENDERPRESETS, Disable);
WidgetSetDisabled(IDC_FORMATDROP, Disable);
WidgetSetDisabled(IDC_RENDER, Disable);
WidgetSetDisabled(IDC_CONSTRAIN, Disable);
WidgetSetDisabled(IDC_SCALE, Disable);
WidgetSetDisabled(IDC_TNAIL1, Disable);
WidgetSetDisabled(IDC_TNAIL2, Disable);
WidgetSetDisabled(IDC_TNAIL3, Disable);
WidgetSetDisabled(IDC_TNAIL4, Disable);
WidgetSetDisabled(IDC_TNAIL5, Disable);
WidgetSetDisabled(IDC_TNAIL6, Disable);
WidgetSetDisabled(IDC_TNAIL7, Disable);
WidgetSetDisabled(IDC_TNAIL8, Disable);
WidgetSetDisabled(IDC_TNAIL9, Disable);
WidgetSetDisabled(IDC_TNAIL10, Disable);
WidgetSetDisabled(IDC_TNAIL11, Disable);
WidgetSetDisabled(IDC_TNAIL12, Disable);
WidgetSetDisabled(IDC_TNAIL13, Disable);
WidgetSetDisabled(IDC_CAMPOS1, Disable);
WidgetSetDisabled(IDC_CAMPOS2, Disable);
WidgetSetDisabled(IDC_CAMPOS3, Disable);
WidgetSetDisabled(IDC_CHECKMIDKEY, Disable);
WidgetSetDisabled(IDC_CHECKENDKEY, Disable);
WidgetSetDisabled(IDC_CAMLAT1, Disable);
WidgetSetDisabled(IDC_CAMLAT2, Disable);
WidgetSetDisabled(IDC_CAMLAT3, Disable);
WidgetSetDisabled(IDC_CAMLON1, Disable);
WidgetSetDisabled(IDC_CAMLON2, Disable);
WidgetSetDisabled(IDC_CAMLON3, Disable);
WidgetSetDisabled(IDC_CAMALT1, Disable);
WidgetSetDisabled(IDC_CAMALT2, Disable);
WidgetSetDisabled(IDC_CAMALT3, Disable);
WidgetSetDisabled(IDC_FOCLAT1, Disable);
WidgetSetDisabled(IDC_FOCLAT2, Disable);
WidgetSetDisabled(IDC_FOCLAT3, Disable);
WidgetSetDisabled(IDC_FOCLON1, Disable);
WidgetSetDisabled(IDC_FOCLON2, Disable);
WidgetSetDisabled(IDC_FOCLON3, Disable);
WidgetSetDisabled(IDC_FOCALT1, Disable);
WidgetSetDisabled(IDC_FOCALT2, Disable);
WidgetSetDisabled(IDC_FOCALT3, Disable);
WidgetSetDisabled(IDC_VIEWARC1, Disable);
WidgetSetDisabled(IDC_VIEWARC2, Disable);
WidgetSetDisabled(IDC_VIEWARC3, Disable);
WidgetSetDisabled(IDC_ZMINIMUM, Disable);
WidgetSetDisabled(IDC_ZMAXIMUM, Disable);
WidgetSetDisabled(IDC_IMAGEWIDTH, Disable);
WidgetSetDisabled(IDC_IMAGEHEIGHT, Disable);
WidgetSetDisabled(IDC_SEGMENT, Disable);
WidgetSetDisabled(IDC_MIDTIME, Disable);
WidgetSetDisabled(IDC_ENDTIME, Disable);
WidgetSetDisabled(IDC_PLANET_VERTICALEXAG, Disable);
WidgetSetDisabled(IDC_PIXELASPECT, Disable);
WidgetSetDisabled(IDC_TEMPPATH, Disable);
WidgetSetDisabled(IDC_SAVEPATH, Disable);
WidgetShow(IDC_ELEVTXT1, 1);
WidgetShow(IDC_ELEVTXT2, 1);
WidgetShow(IDC_ELEVTXT3, 1);
WidgetShow(IDC_VECTORTXT1, 1);
if (! Disable)
	DisableWidgets();

} // EDSSControlGUI::DisableRenderWidgets

/*===========================================================================*/

void EDSSControlGUI::RenderComplete(void)
{

DisableRenderWidgets(0);
Rendering = 0;
ReceivingDiagnostics = 1;

} // EDSSControlGUI::RenderComplete

/*===========================================================================*/

void EDSSControlGUI::SyncPlanCam(void)
{
int DataSet = 0;
double CenterX, CenterY, ViewWidth, CenterLat, CenterLon;
Camera *PlanCam;

switch (ActivePage)
	{
	case 0:
		{
		ViewWidth = EDSS_PlanWidth * SwatchData0.Visible;
		CenterX = SwatchData0.OffsetX + .5 * SwatchData0.Visible;
		CenterY = SwatchData0.OffsetY + .5 * SwatchData0.Visible;
		DataSet = 1;
		break;
		} // 0
	case 1:
		{
		ViewWidth = EDSS_PlanWidth * SwatchData1.Visible;
		CenterX = SwatchData1.OffsetX + .5 * SwatchData1.Visible;
		CenterY = SwatchData1.OffsetY + .5 * SwatchData1.Visible;
		DataSet = 1;
		break;
		} // 0
	case 2:
		{
		ViewWidth = EDSS_PlanWidth * SwatchData2.Visible;
		CenterX = SwatchData2.OffsetX + .5 * SwatchData2.Visible;
		CenterY = SwatchData2.OffsetY + .5 * SwatchData2.Visible;
		DataSet = 1;
		break;
		} // 0
	} // switch

if (DataSet)
	{
	PlanCam = (Camera *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA);
	while (PlanCam && PlanCam->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		PlanCam = (Camera *)PlanCam->Next;
	if (PlanCam)
		{
		CenterLat = EDSS_PlanNorth + (EDSS_PlanSouth - EDSS_PlanNorth) * CenterY;
		CenterLon = EDSS_PlanWest + (EDSS_PlanEast - EDSS_PlanWest) * CenterX;
		PlanCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(ViewWidth);
		PlanCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CenterLat);
		PlanCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CenterLon);
		PlanCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].ValueChanged();
		} // if
	} // if

} // EDSSControlGUI::SyncPlanCam

/*===========================================================================*/

void EDSSControlGUI::SyncTerrainElev(void)
{
AnimDoubleTime *LatPtr, *LonPtr;
int Reject;
double TerrainElev;
char ElevStr[64];

LatPtr = ActivePage == 0 ? &ControlHost->FirstCamLatADT: ActivePage == 1 ? &ControlHost->MidCamLatADT: &ControlHost->EndCamLatADT;
LonPtr = ActivePage == 0 ? &ControlHost->FirstCamLonADT: ActivePage == 1 ? &ControlHost->MidCamLonADT: &ControlHost->EndCamLonADT;

TerrainElev = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(LatPtr->CurValue, LonPtr->CurValue, Reject);

if (! Reject)
	{
	FormatAsPreferredUnit(ElevStr, &ControlHost->FirstCamElevADT, TerrainElev);
	if (ActivePage == 0)
		WidgetSetText(IDC_ELEVTXT1, ElevStr);
	else if (ActivePage == 1)
		{
		ElevConfigured1 = 1;
		WidgetSetText(IDC_ELEVTXT2, ElevStr);
		} // else if
	else
		{
		ElevConfigured2 = 1;
		WidgetSetText(IDC_ELEVTXT3, ElevStr);
		} // else
	} // if

} // EDSSControlGUI::SyncTerrainElev

/*===========================================================================*/

void EDSSControlGUI::SyncVectorMessage(int LatLonProvided, double Lat, double Lon)
{
AnimDoubleTime *LatPtr, *LonPtr;
SearchQuery *VecSearch;
Joe *CurVec = NULL;
DBFilterEvent *Filt;
JoeAttribLayerData *AttrList, *CurAttr, *TempAttr, *PrimaryPtr, **AttrPtr;
char VecTxt[256], TempTxt[256], *SecondaryKeyValue;
int KeyCt, LastKey;

VecSearch = (SearchQuery *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY);
while (VecSearch && stricmp(VecSearch->Name, "Foliage Poly Point Contained"))
	VecSearch = (SearchQuery *)VecSearch->Next;

if (! LatLonProvided)
	{
	LatPtr = ActivePage == 0 ? &ControlHost->FirstCamLatADT: ActivePage == 1 ? &ControlHost->MidCamLatADT: &ControlHost->EndCamLatADT;
	LonPtr = ActivePage == 0 ? &ControlHost->FirstCamLonADT: ActivePage == 1 ? &ControlHost->MidCamLonADT: &ControlHost->EndCamLonADT;
	Lat = LatPtr->CurValue;
	Lon = LonPtr->CurValue;
	} // if

if (VecSearch)
	{
	Filt = VecSearch->Filters;
	while (Filt)
		{
		if (Filt->GeoPtContained || Filt->GeoPtUncontained)
			break;
		} // while
	if (Filt)
		{
		Filt->SetGeoPoint(Lat, Lon);
		DBHost->ResetGeoClip();
		while (CurVec = DBHost->GetNextByQuery(VecSearch, CurVec))
			{
			AttrPtr = &AttrList;
			for (KeyCt = 0; KeyCt < PrimaryKeys; KeyCt ++)
				{
				if (*AttrPtr = new JoeAttribLayerData())
					{
					PrimaryPtr = *AttrPtr;
					PrimaryPtr->AttribName = PrimaryKeyValue[KeyCt];
					AttrPtr = &(*AttrPtr)->Next;
					CurVec->GetAttribValueList(PrimaryPtr);
					if (PrimaryPtr->AttrValueTxt)
						{
						SecondaryKeyValue = NULL;
						LastKey = -1;
						while (SecondaryKeyValue = MatchSecondaryKey(PrimaryPtr->AttrValueTxt, LastKey, SecondaryKeyValue))
							{
							if (*AttrPtr = new JoeAttribLayerData())
								{
								(*AttrPtr)->AttribName = SecondaryKeyValue;
								AttrPtr = &(*AttrPtr)->Next;
								} // if
							} // while
						if (PrimaryPtr->Next)
							CurVec->GetAttribValueList(PrimaryPtr->Next);
						} // if
					} // if
				} // for
			sprintf(VecTxt, "Foliage Polygon: %s", CurVec->GetBestName());
			CurAttr = AttrList;
			while (CurAttr)
				{
				if (CurAttr->AttribName)
					{
					if (CurAttr->AttrValueTxt)
						sprintf(TempTxt, ", %s: %s", CurAttr->AttribName, CurAttr->AttrValueTxt);
					else
						{
						sprintf(TempTxt, ", %s: %f", CurAttr->AttribName, CurAttr->AttrValueDbl);
						TrimZeros(TempTxt);
						} // else
					strcat(VecTxt, TempTxt);
					} // if
				CurAttr = CurAttr->Next;
				} // while
			WidgetSetText(IDC_VECTORTXT1, VecTxt);
			CurAttr = AttrList;
			while (CurAttr)
				{
				TempAttr = CurAttr->Next;
				delete CurAttr;
				CurAttr = TempAttr;
				} // while
			AttrList = NULL;
			break;	// only interested in one vector for now
			} // if

		/*
		if (AttrList = new JoeAttribLayerData())
			{
			AttrList->AttribName = "Major_tree";
			if (AttrList->Next = new JoeAttribLayerData())
				{
				AttrList->Next->AttribName = "Minor_tree";

				Filt->SetGeoPoint(LatPtr->CurValue, LonPtr->CurValue);
				DBHost->ResetGeoClip();
				while (CurVec = DBHost->GetNextByQuery(VecSearch, CurVec))
					{
					AttrPtr = &AttrList->Next->Next;
					CurVec->GetAttribValueList(AttrList);
					if (AttrList->AttrValueTxt)
						{
						if (*AttrPtr = new JoeAttribLayerData())
							{
							(*AttrPtr)->AttribName = MatchDensityFieldToTree(AttrList->AttrValueTxt);
							AttrPtr = &(*AttrPtr)->Next;
							} // if
						if (*AttrPtr = new JoeAttribLayerData())
							{
							(*AttrPtr)->AttribName = MatchHeightFieldToTree(AttrList->AttrValueTxt);
							AttrPtr = &(*AttrPtr)->Next;
							} // if
						} // if
					if (AttrList->Next->AttrValueTxt)
						{
						if (*AttrPtr = new JoeAttribLayerData())
							{
							(*AttrPtr)->AttribName = MatchDensityFieldToTree(AttrList->Next->AttrValueTxt);
							AttrPtr = &(*AttrPtr)->Next;
							} // if
						if (*AttrPtr = new JoeAttribLayerData())
							{
							(*AttrPtr)->AttribName = MatchHeightFieldToTree(AttrList->Next->AttrValueTxt);
							AttrPtr = &(*AttrPtr)->Next;
							} // if
						} // if
					if (AttrList->Next->Next)
						CurVec->GetAttribValueList(AttrList->Next->Next);
					sprintf(VecTxt, "Foliage Polygon: %s", CurVec->GetBestName());
					CurAttr = AttrList;
					while (CurAttr)
						{
						if (CurAttr->AttribName)
							{
							if (CurAttr->AttrValueTxt)
								sprintf(TempTxt, ", %s: %s", CurAttr->AttribName, CurAttr->AttrValueTxt);
							else
								{
								sprintf(TempTxt, ", %s: %f", CurAttr->AttribName, CurAttr->AttrValueDbl);
								TrimZeros(TempTxt);
								} // else
							strcat(VecTxt, TempTxt);
							} // if
						CurAttr = CurAttr->Next;
						} // while
					WidgetSetText(IDC_VECTORTXT1, VecTxt);
					CurAttr = AttrList->Next->Next;
					while (CurAttr)
						{
						TempAttr = CurAttr->Next;
						delete CurAttr;
						CurAttr = TempAttr;
						} // while
					// could do something here and see if we want to continue looking at more vectors
					break;
					} // while
				delete AttrList->Next;
				} // if
			delete AttrList;
			} // if
		*/
		} // if
	} // if

} // EDSSControlGUI::SyncVectorMessage

/*===========================================================================*/

char *EDSSControlGUI::MatchSecondaryKey(char *PKeyValue, int &KeyCt, char *LastSecondaryKeyValue)
{
int SecondCt;

// find a match between PKeyValue and one of the entries in the SecondaryTable
if (KeyCt < 0 || KeyCt >= PKeyValues)
	{
	for (KeyCt = 0; KeyCt < PKeyValues; KeyCt ++) 
		{
		if (! stricmp(PKeyValue, SecondaryKeyValue[KeyCt][0]))
			{
			break;
			} // if
		} // for
	} // if need to find it

if (KeyCt < PKeyValues)
	{
	if (LastSecondaryKeyValue)
		{
		for (SecondCt = 1; SecondCt < MaxSecondaryKeys; SecondCt ++)
			{
			if (! stricmp(LastSecondaryKeyValue, SecondaryKeyValue[KeyCt][SecondCt]))
				{
				SecondCt ++;
				break;
				} // if
			} // for
		} // if already a last value
	else
		SecondCt = 1;
	if (SecondCt < MaxSecondaryKeys)
		{
		return (SecondaryKeyValue[KeyCt][SecondCt][0] ? SecondaryKeyValue[KeyCt][SecondCt]: NULL);
		} // if
	} // if

return (NULL);

} // EDSSControlGUI::MatchSecondaryKey

/*===========================================================================*/

void EDSSControlGUI::CaptureDiagnostics(DiagnosticData *Data)
{
double Latitude, Longitude;

if (Data)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		{
		Latitude = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
		} // if
	else
		{
		return;
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		{
		Longitude = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
		} // if
	else
		{
		return;
		} // else
	} // if

SyncVectorMessage(1, Latitude, Longitude);

} // EDSSControlGUI::CaptureDiagnostics

/*===========================================================================*/


/*
char *EDSSControlGUI::MatchDensityFieldToTree(char *TreeName)
{

if (! stricmp(TreeName, "American Beech"))
	{
	return ("TPA_AB");
	} // if
else if (! stricmp(TreeName, "Black Cherry"))
	{
	return ("TPA_BC");
	} // else if
else if (! stricmp(TreeName, "Black Gum"))
	{
	return ("TPA_BG");
	} // else if
else if (! stricmp(TreeName, "Chestnut Oak"))
	{
	return ("TPA_CO");
	} // else if
else if (! stricmp(TreeName, "Hickory"))
	{
	return ("TPA_HI");
	} // else if
else if (! stricmp(TreeName, "Loblolly Pine"))
	{
	return ("TPA_LP");
	} // else if
else if (! stricmp(TreeName, "Red Maple"))
	{
	return ("TPA_RM");
	} // else if
else if (! stricmp(TreeName, "Red Oak"))
	{
	return ("TPA_SK");
	} // else if
else if (! stricmp(TreeName, "Shortleaf Pine"))
	{
	return ("TPA_SP");
	} // else if
else if (! stricmp(TreeName, "Sweetgum"))
	{
	return ("TPA_SU");
	} // else if
else if (! stricmp(TreeName, "Virginia Pine"))
	{
	return ("TPA_VP");
	} // else if
else if (! stricmp(TreeName, "Black Walnut"))
	{
	return ("TPA_WN");
	} // else if
else if (! stricmp(TreeName, "White Oak"))
	{
	return ("TPA_WO");
	} // else if
else if (! stricmp(TreeName, "Yellow Poplar"))
	{
	return ("TPA_YP");
	} // else if

return (NULL);

} // EDSSControlGUI::MatchDensityFieldToTree
*/
/*===========================================================================*/
/*
char *EDSSControlGUI::MatchHeightFieldToTree(char *TreeName)
{

if (! stricmp(TreeName, "American Beech"))
	{
	return ("HT_AB");
	} // if
else if (! stricmp(TreeName, "Black Cherry"))
	{
	return ("HT_BC");
	} // else if
else if (! stricmp(TreeName, "Black Gum"))
	{
	return ("HT_BG");
	} // else if
else if (! stricmp(TreeName, "Chestnut Oak"))
	{
	return ("HT_CO");
	} // else if
else if (! stricmp(TreeName, "Hickory"))
	{
	return ("HT_HI");
	} // else if
else if (! stricmp(TreeName, "Loblolly Pine"))
	{
	return ("HT_LP");
	} // else if
else if (! stricmp(TreeName, "Red Maple"))
	{
	return ("HT_RM");
	} // else if
else if (! stricmp(TreeName, "Red Oak"))
	{
	return ("HT_SK");
	} // else if
else if (! stricmp(TreeName, "Shortleaf Pine"))
	{
	return ("HT_SP");
	} // else if
else if (! stricmp(TreeName, "Sweetgum"))
	{
	return ("HT_SU");
	} // else if
else if (! stricmp(TreeName, "Virginia Pine"))
	{
	return ("HT_VP");
	} // else if
else if (! stricmp(TreeName, "Black Walnut"))
	{
	return ("HT_WN");
	} // else if
else if (! stricmp(TreeName, "White Oak"))
	{
	return ("HT_WO");
	} // else if
else if (! stricmp(TreeName, "Yellow Poplar"))
	{
	return ("HT_YP");
	} // else if

return (NULL);

} // EDSSControlGUI::MatchHeightFieldToTree
*/
/*===========================================================================*/

unsigned long int EDSSControlGUI::QueryHelpTopic(void)
{
switch(ActivePage)
	{
	case 0: return('EDS0'); // cam start
	case 1: return('EDS1'); // cam mid
	case 2: return('EDS2'); // cam end
	case 3: return('EDSL'); // light
	case 4: return('EDSA'); // atmos
	case 5: return('EDSM'); // misc
	default: return(FenID); // general
	} // switch
} // EDSSControlGUI::QueryHelpTopic
