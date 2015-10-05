// CloudEditGUI.cpp
// Code for Cloud editor
// Built from scratch on 6/12/99 by Gary R. Huber. Modified extensively 10/28/99.
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CloudEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Raster.h"
#include "Useful.h"
#include "Requester.h"
#include "AppMem.h"
#include "Render.h"
#include "ViewGUI.h"
#include "Conservatory.h"
#include "resource.h"
#include "Lists.h"

char *CloudEditGUI::TabNames[WCS_CLOUDGUI_NUMTABS] = {"General", "Basic", "Advanced", "Color", "Shadows", "Waves"};

long CloudEditGUI::ActivePage;
// advanced
long CloudEditGUI::DisplayAdvanced;

// material GUI
#define WCS_CLOUDED_MATGRADSET	1

NativeGUIWin CloudEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CloudEditGUI::Open

/*===========================================================================*/

NativeGUIWin CloudEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_CLOUD_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_CLOUD_HORIZONTAL, 0, 1);
	CreateSubWinFromTemplate(IDD_CLOUD_VERTICAL, 0, 2);
	CreateSubWinFromTemplate(IDD_CLOUD_COLOR, 0, 3);
	CreateSubWinFromTemplate(IDD_CLOUD_SHADOW, 0, 4);
	CreateSubWinFromTemplate(IDD_CLOUD_WAVES, 0, 5);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_CLOUDGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// Material GUI
		MatGUI->Construct(WCS_CLOUDED_MATGRADSET, WCS_CLOUDED_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		UpdateThumbnail();
		} // if
	} // if
 
return (NativeWin);

} // CloudEditGUI::Construct

/*===========================================================================*/

CloudEditGUI::CloudEditGUI(EffectsLib *EffectsSource, Database *DBSource, CloudEffect *ActiveSource)
: GUIFenetre('CLOD', this, "Cloud Model Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_CLOUD, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
Rast = WaveRast = NULL;
ActiveGrad = NULL;
SampleData = WaveSampleData = NULL;
ActiveWave = NULL;
BackgroundInstalled = 0;
EvolveFast = ReceivingDiagnostics = ReceivingWave = ReceivingWaveSource = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;
// Material GUI
MatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Cloud Model Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->ConfirmType();
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// material GUI
	if (MatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->ColorGrad)) // init ordinal 0
		{
		PopDropMaterialNotifier.Host = this; // to be able to call notifications later
		MatGUI->SetNotifyFunctor(&PopDropMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	if (! (Rast = new Raster()))
		ConstructError = 1;
	if (! (SampleData = new TextureSampleData()))
		ConstructError = 1;
	if (! (WaveRast = new Raster()))
		ConstructError = 1;
	if (! (WaveSampleData = new TextureSampleData()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // CloudEditGUI::CloudEditGUI

/*===========================================================================*/

CloudEditGUI::~CloudEditGUI()
{

GlobalApp->RemoveBGHandler(this);

if (Rast)
	delete Rast;
if (SampleData)
	delete SampleData;
if (WaveRast)
	delete WaveRast;
if (WaveSampleData)
	delete WaveSampleData;
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (MatGUI)
	delete MatGUI;

} // CloudEditGUI::~CloudEditGUI()

/*===========================================================================*/

long CloudEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_CLG, 0);

return(0);

} // CloudEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long CloudEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // CloudEditGUI::HandleShowAdvanced

/*===========================================================================*/

long CloudEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CLG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITPROFILE:
		{
		Active->ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
	case IDC_EDITVERTPROFILE:
		{
		Active->CovgProf.OpenTimeline();
		break;
		} // IDC_EDITVERTPROFILE
	case IDC_EDITDENSITYPROFILE:
		{
		Active->DensityProf.OpenTimeline();
		break;
		} // IDC_EDITVERTPROFILE
	case IDC_EDITSHADEPROFILE:
		{
		Active->ShadeProf.OpenTimeline();
		break;
		} // IDC_EDITSHADEPROFILE
	case IDC_SETBOUNDS:
		{
		SetBounds(NULL);
		break;
		} //
	case IDC_PRESETCIRRUS:
		{
		if (WidgetGetCheck(IDC_PRESETCIRRUS))
			{
			WidgetSetCheck(IDC_PRESETSTRATUS, false);
			WidgetSetCheck(IDC_PRESETNIMBUS, false);
			WidgetSetCheck(IDC_PRESETCUMULUS, false);
			WidgetSetCheck(IDC_PRESETCUSTOM, false);
			}
		else
			WidgetSetCheck(IDC_PRESETCUSTOM, true);
		CloudType();
		break;
		} //
	case IDC_PRESETSTRATUS:
		{
		if (WidgetGetCheck(IDC_PRESETSTRATUS))
			{
			WidgetSetCheck(IDC_PRESETCIRRUS, false);
			WidgetSetCheck(IDC_PRESETNIMBUS, false);
			WidgetSetCheck(IDC_PRESETCUMULUS, false);
			WidgetSetCheck(IDC_PRESETCUSTOM, false);
			}
		else
			WidgetSetCheck(IDC_PRESETCUSTOM, true);
		CloudType();
		break;
		} //
	case IDC_PRESETNIMBUS:
		{
		if (WidgetGetCheck(IDC_PRESETNIMBUS))
			{
			WidgetSetCheck(IDC_PRESETCIRRUS, false);
			WidgetSetCheck(IDC_PRESETSTRATUS, false);
			WidgetSetCheck(IDC_PRESETCUMULUS, false);
			WidgetSetCheck(IDC_PRESETCUSTOM, false);
			}
		else
			WidgetSetCheck(IDC_PRESETCUSTOM, true);
		CloudType();
		break;
		} //
	case IDC_PRESETCUMULUS:
		{
		if (WidgetGetCheck(IDC_PRESETCUMULUS))
			{
			WidgetSetCheck(IDC_PRESETCIRRUS, false);
			WidgetSetCheck(IDC_PRESETSTRATUS, false);
			WidgetSetCheck(IDC_PRESETNIMBUS, false);
			WidgetSetCheck(IDC_PRESETCUSTOM, false);
			}
		else
			WidgetSetCheck(IDC_PRESETCUSTOM, true);
		CloudType();
		break;
		} //
	case IDC_PRESETCUSTOM:
		{
		if (WidgetGetCheck(IDC_PRESETCUSTOM))
			{
			WidgetSetCheck(IDC_PRESETCIRRUS, false);
			WidgetSetCheck(IDC_PRESETSTRATUS, false);
			WidgetSetCheck(IDC_PRESETNIMBUS, false);
			WidgetSetCheck(IDC_PRESETCUMULUS, false);
			}
		else
			WidgetSetCheck(IDC_PRESETCUSTOM, true);
		CloudType();
		break;
		} //
	case IDC_ADDSOURCEINVIEW:
		{
		AddWaveInView(NULL);
		break;
		} //
	case IDC_SETSOURCEINVIEW:
		{
		SetSourcePosInView(NULL);
		break;
		} //
	case IDC_ADDSOURCE:
		{
		AddWave();
		break;
		} // IDC_ADDSOURCE
	case IDC_REMOVESOURCE:
		{
		RemoveWave();
		break;
		} // IDC_REMOVESOURCE
	case IDC_ZOOMIN:
		{
		ZoomScale(1);
		break;
		} // IDC_ZOOMIN
	case IDC_ZOOMOUT:
		{
		ZoomScale(-1);
		break;
		} // IDC_ZOOMOUT
	case IDC_ZOOMIN3:
		{
		ZoomWaveScale(1);
		break;
		} // IDC_ZOOMIN
	case IDC_ZOOMOUT3:
		{
		ZoomWaveScale(-1);
		break;
		} // IDC_ZOOMOUT
	// material GUI
	case IDC_POPDROP0:
		{
		if(WidgetGetCheck(IDC_POPDROP0))
			{
			ShowMaterialPopDrop(true);			} // if
		else
			{
			ShowMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	default:
		break;
	} // ButtonID

// Material GUI
MatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // CloudEditGUI::HandleButtonClick

/*===========================================================================*/

long CloudEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(029, 29);
switch (CtrlID)
	{
	case IDC_SOURCELIST:
		{
		SetActiveWave();
		break;
		} // IDC_SOURCELIST
	default:
		break;
	} // switch CtrlID

return (0);

} // CloudEditGUI::HandleListSel

/*===========================================================================*/

long CloudEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		RemoveWave();
		break;
		} // IDC_SOURCELIST
	default:
		break;
	} // switch

return(0);

} // CloudEditGUI::HandleListDelItem

/*===========================================================================*/

long CloudEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		CopyWave();
		break;
		} // IDC_SOURCELIST
	default:
		break;
	} // switch

return(0);

} // CloudEditGUI::HandleListCopyItem

/*===========================================================================*/

long CloudEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		PasteWave();
		break;
		} // IDC_SOURCELIST
	default:
		break;
	} // switch

return(0);

} // CloudEditGUI::HandleListPasteItem

/*===========================================================================*/

long CloudEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // CloudEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long CloudEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // CloudEditGUI::HandleCBChange

/*===========================================================================*/

long CloudEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// Material GUI
ShowMaterialPopDrop(false);

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				ShowPanel(0, 1);
				break;
				} // 1
			case 2:
				{
				ShowPanel(0, 2);
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

} // CloudEditGUI::HandlePageChange

/*===========================================================================*/

long CloudEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_SOURCEENABLED:
		{
		if (ActiveWaveValid())
			{
			Changes[0] = MAKE_ID(ActiveWave->GetNotifyClass(), ActiveWave->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveWave->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_CHECKGRADFILL:
	case IDC_CHECKCLOUDFEATHER:
	case IDC_CHECKCASTSHADOWS:
	case IDC_CHECKUSEMAPFILE:
	case IDC_CHECKREGENMAPFILE:
	case IDC_CHECKSHADOWSONLY:
	case IDC_CHECKATMOSPHERE:
	case IDC_CHECKRECEIVESHADOWSTER:
	case IDC_CHECKRECEIVESHADOWSFOL:
	case IDC_CHECKRECEIVESHADOWS3D:
	case IDC_CHECKRECEIVESHADOWSCSM:
	case IDC_CHECKRECEIVESHADOWSVOL:
	case IDC_CHECKRECEIVESHADOWSMISC:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CloudEditGUI::HandleSCChange

/*===========================================================================*/

long CloudEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOEVOLVEFAST:
	case IDC_RADIOEVOLVESLOW:
	case IDC_RADIOEVOLVENOT:
		{
		DoEvolution();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOSHADOWMAPLARGE:
	case IDC_RADIOSHADOWMAPMED:
	case IDC_RADIOSHADOWMAPSMALL:
	case IDC_RADIOSHADOWMAPVERYLARGE:
	case IDC_RADIOCASTSHADOWMAP:
	case IDC_RADIOSHADOWCASTVOLUMETRIC:
	case IDC_RADIOSHADOWCASTCOMBINATION:
	case IDC_RADIOBEFOREREFL:
	case IDC_RADIOAFTERREFL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CloudEditGUI::HandleSRChange

/*===========================================================================*/

long CloudEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch(CtrlID)
	{
	case IDC_ROWS:
	case IDC_COLS:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_NUMLAYERS:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		} // IDC_NUMLAYERS
		//lint -fallthrough
	case IDC_THICKNESS:
		{
		ConfigureLayerThickness();
		break;
		} // IDC_THICKNESS
	case IDC_SOURCEAMP:
		{
		BuildWaveList();
		break;
		} // 
	default:
		break;
	} // switch

// Material GUI
MatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // CloudEditGUI::HandleFIChange

/*===========================================================================*/

void CloudEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[8];
int UpdateThumbs = 0, Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	else if (ReceivingWave)
		AddWaveInView((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	else if (ReceivingWaveSource)
		SetSourcePosInView((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	DisableWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[6] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureColors();
	SyncWidgets();
	UpdateThumbs = 1;
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	UpdateThumbs = 1;
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

if (! Done)
	{
	ConfigureWidgets();
	UpdateThumbnail();
	} // if
else if (UpdateThumbs)
	UpdateThumbnail();

} // CloudEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void CloudEditGUI::ConfigureWidgets(void)
{
char TextStr[256], EvolveEnabled;
long Ct;
double Width;
Texture *Tex;

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

sprintf(TextStr, "Cloud Model Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSHADOWSONLY, &Active->ShadowsOnly, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLOUDFEATHER, &Active->Feather, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCASTSHADOWS, &Active->CastShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKUSEMAPFILE, &Active->UseMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREGENMAPFILE, &Active->RegenMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, &Active->Volumetric, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSTER, &Active->ReceiveShadowsTerrain, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSFOL, &Active->ReceiveShadowsFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS3D, &Active->ReceiveShadows3DObject, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSCSM, &Active->ReceiveShadowsCloudSM, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSVOL, &Active->ReceiveShadowsVolumetric, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSMISC, &Active->ReceiveShadowsMisc, SCFlag_Char, NULL, 0);

EvolveEnabled = (Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && (Tex = Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex) &&
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE));
EvolveFast = (EvolveEnabled &&
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE) &&
	(Tex->TexVelocity[2].CurValue != 0.0));
if (EvolveFast)
	EvolveFast = (Tex->TexVelocity[2].CurValue > 45.0) ? 2: 1;	// cutoff is 1.5 meters/frame or 45 m/sec

ConfigureSR(NativeWin, IDC_RADIOEVOLVENOT, IDC_RADIOEVOLVENOT, &EvolveFast, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOEVOLVENOT, IDC_RADIOEVOLVESLOW, &EvolveFast, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOEVOLVENOT, IDC_RADIOEVOLVEFAST, &EvolveFast, SRFlag_Char, 2, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPSMALL, IDC_RADIOSHADOWMAPSMALL, &Active->ShadowMapWidth, SCFlag_Short, 512, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPSMALL, IDC_RADIOSHADOWMAPMED, &Active->ShadowMapWidth, SCFlag_Short, 1024, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPSMALL, IDC_RADIOSHADOWMAPLARGE, &Active->ShadowMapWidth, SCFlag_Short, 2048, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPSMALL, IDC_RADIOSHADOWMAPVERYLARGE, &Active->ShadowMapWidth, SCFlag_Short, 4096, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOCASTSHADOWMAP, IDC_RADIOCASTSHADOWMAP, &Active->CastShadowStyle, SCFlag_Char, WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCASTSHADOWMAP, IDC_RADIOSHADOWCASTVOLUMETRIC, &Active->CastShadowStyle, SCFlag_Char, WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCASTSHADOWMAP, IDC_RADIOSHADOWCASTCOMBINATION, &Active->CastShadowStyle, SCFlag_Char, WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOBEFOREREFL, &Active->VolumeBeforeRefl, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOAFTERREFL, &Active->VolumeBeforeRefl, SRFlag_Char, 0, NULL, NULL);

WidgetAGConfig(IDC_ANIMGRADIENT, &Active->ColorGrad);
// Material GUI
MatGUI->ConfigureWidgets();

// material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);

WidgetSmartRAHConfig(IDC_THICKNESS, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS], Active);
WidgetSmartRAHConfig(IDC_SHADOWINTENS, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS], Active);
WidgetSmartRAHConfig(IDC_DENSITYPTRN, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE), Active);
WidgetSmartRAHConfig(IDC_SELFSHADE, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING], Active);
WidgetSmartRAHConfig(IDC_COVERAGE, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE], Active);
WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY], Active);
WidgetSmartRAHConfig(IDC_FALLOFF, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_FALLOFF], Active);
WidgetSmartRAHConfig(IDC_MAPHEIGHT, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT], Active);
WidgetSmartRAHConfig(IDC_MAPWIDTH, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH], Active);
WidgetSmartRAHConfig(IDC_CENTERLAT, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT], Active);
WidgetSmartRAHConfig(IDC_CENTERLON, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON], Active);
WidgetSmartRAHConfig(IDC_ALT, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV], Active);
WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_OPTICALDEPTH], Active);
WidgetSmartRAHConfig(IDC_MINSAMPLE, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE], Active);
WidgetSmartRAHConfig(IDC_MAXSAMPLE, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE], Active);
WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT], Active);
WidgetSmartRAHConfig(IDC_TRANSLUMEXP, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTEXP], Active);
WidgetSmartRAHConfig(IDC_SPECCOLORPCT, &Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT], Active);

ConfigureFI(NativeWin, IDC_ROWS,
 &Active->Rows,
  1.0,
   10.0,
	10000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_COLS,
 &Active->Cols,
  1.0,
   10.0,
	10000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_NUMLAYERS,
 &Active->NumLayers,
  1.0,
   1.0,
	1000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureTB(NativeWin, IDC_ZOOMIN, IDI_ZMINARROW, NULL);
ConfigureTB(NativeWin, IDC_ZOOMOUT, IDI_ZMOUTARROW, NULL);
ConfigureTB(NativeWin, IDC_ZOOMIN3, IDI_ZMINARROW, NULL);
ConfigureTB(NativeWin, IDC_ZOOMOUT3, IDI_ZMOUTARROW, NULL);
ConfigureTB(NativeWin, IDC_ADDSOURCE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESOURCE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_PRESETCIRRUS, IDI_PRESETCIRRUS, NULL);
ConfigureTB(NativeWin, IDC_PRESETSTRATUS, IDI_PRESETTHICKER, NULL);
ConfigureTB(NativeWin, IDC_PRESETNIMBUS, IDI_PRESETDENSE, NULL);
ConfigureTB(NativeWin, IDC_PRESETCUMULUS, IDI_PRESETCUMULUS, NULL);
ConfigureTB(NativeWin, IDC_PRESETCUSTOM, IDI_PRESETCUSTOM, NULL);

WidgetSetCheck(IDC_PRESETCIRRUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CIRRUS);
WidgetSetCheck(IDC_PRESETSTRATUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_STRATUS);
WidgetSetCheck(IDC_PRESETNIMBUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_NIMBUS);
WidgetSetCheck(IDC_PRESETCUMULUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CUMULUS);
WidgetSetCheck(IDC_PRESETCUSTOM, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CUSTOM);

Width = max(Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue, Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue);
for (Ct = 1; Ct < Active->PreviewSize; Ct ++)
	Width /= 2.0;
Width /= 1000.0;
sprintf(TextStr, "%1.1f Km", Width);
WidgetSetText(IDC_PREVIEWSIZETXT, TextStr);

Width = 50000.0;
for (Ct = 1; Ct < Active->WavePreviewSize; Ct ++)
	Width /= 2.0;
sprintf(TextStr, "%1.1f m", Width);
WidgetSetText(IDC_PREVIEWSIZETXT3, TextStr);

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

BuildWaveList();
ConfigureWave();
ConfigureColors();
ConfigureLayerThickness();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // CloudEditGUI::ConfigureWidgets()

/*===========================================================================*/

void CloudEditGUI::ConfigureColors(void)
{
ColorTextureThing *ActiveNode;

WidgetAGSync(IDC_ANIMGRADIENT);

if ((ActiveGrad = Active->ColorGrad.GetActiveNode()) && (ActiveNode = (ColorTextureThing *)ActiveGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_COLOR1, &ActiveNode->Color, ActiveNode);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_COLOR1, (RasterAnimHost *)NULL, NULL);
	} // else

WidgetSmartRAHConfig(IDC_BACKLIGHTCOLOR, &Active->BacklightColor, Active);

// material GUI
MatGUI->ConfigureMaterial();


} // CloudEditGUI::ConfigureColors

/*===========================================================================*/

void CloudEditGUI::ConfigureLayerThickness(void)
{
double LayerThickness = 0.0;
char ThickTxt[256];

if (Active->NumLayers > 1)
	{
	if ((LayerThickness = Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].CurValue / (Active->NumLayers - 1)) > 1000000.0)
		LayerThickness = 1000000.0;
	} // if

sprintf(ThickTxt, "%.0f", LayerThickness);
WidgetSetText(IDC_LAYERTHICKNESS, ThickTxt);

} // CloudEditGUI::ConfigureLayerThickness

/*===========================================================================*/

void CloudEditGUI::BuildWaveList(void)
{
WaveSource *Current = Active->WaveSources;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
WaveSource **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_SOURCELIST);

ActiveWaveValid();

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_SOURCELIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (WaveSource **)AppMem_Alloc(NumSelected * sizeof (WaveSource *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_SOURCELIST, TempCt))
				{
				SelectedItems[SelCt ++] = (WaveSource *)WidgetLBGetItemData(IDC_SOURCELIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_SOURCELIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == (WaveSource *)CurrentRAHost)
			{
			BuildWaveListEntry(ListName, Current);
			WidgetLBReplace(IDC_SOURCELIST, Ct, ListName);
			WidgetLBSetItemData(IDC_SOURCELIST, Ct, Current);
			if (Current == ActiveWave)
				WidgetLBSetSelState(IDC_SOURCELIST, 1, Ct);
			else if (SelectedItems)
				{
				for (SelCt = 0; SelCt < NumSelected; SelCt ++)
					{
					if (SelectedItems[SelCt] == Current)
						{
						WidgetLBSetSelState(IDC_SOURCELIST, 1, Ct);
						break;
						} // if
					} // for
				} // if
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current == (WaveSource *)WidgetLBGetItemData(IDC_SOURCELIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildWaveListEntry(ListName, Current);
				WidgetLBReplace(IDC_SOURCELIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_SOURCELIST, TempCt, Current);
				if (Current == ActiveWave)
					WidgetLBSetSelState(IDC_SOURCELIST, 1, TempCt);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_SOURCELIST, 1, TempCt);
							break;
							} // if
						} // for
					} // if
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_SOURCELIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildWaveListEntry(ListName, Current);
				Place = WidgetLBInsert(IDC_SOURCELIST, Ct, ListName);
				WidgetLBSetItemData(IDC_SOURCELIST, Place, Current);
				if (Current == ActiveWave)
					WidgetLBSetSelState(IDC_SOURCELIST, 1, Place);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_SOURCELIST, 1, Place);
							break;
							} // if
						} // for
					} // if
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_SOURCELIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (WaveSource *));

} // CloudEditGUI::BuildWaveList

/*===========================================================================*/

void CloudEditGUI::BuildWaveListEntry(char *ListName, WaveSource *Me)
{
char AmpStr[64];

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
sprintf(AmpStr, "%f", Me->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue);
TrimZeros(AmpStr);
strcat(ListName, AmpStr);

} // CloudEditGUI::BuildWaveListEntry()

/*===========================================================================*/

WaveSource *CloudEditGUI::ActiveWaveValid(void)
{
WaveSource *CurWave;

if (ActiveWave)
	{
	CurWave = Active->WaveSources;
	while (CurWave)
		{
		if (CurWave == ActiveWave)
			{
			return (ActiveWave);
			} // if
		CurWave = CurWave->Next;
		} // while
	} // if

return (ActiveWave = Active->WaveSources);

} // CloudEditGUI::ActiveWaveValid

/*===========================================================================*/

void CloudEditGUI::ConfigureWave(void)
{

if (ActiveWaveValid())
	{
	ConfigureSC(NativeWin, IDC_SOURCEENABLED, &ActiveWave->Enabled, SCFlag_Char, NULL, 0);

	WidgetSNConfig(IDC_OFFSETX, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX]);
	WidgetSNConfig(IDC_OFFSETY, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY]);
	WidgetSNConfig(IDC_WAVELEN, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH]);
	WidgetSNConfig(IDC_VEL, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY]);
	WidgetSmartRAHConfig(IDC_SOURCEAMP, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE], ActiveWave);
	WidgetSmartRAHConfig(IDC_PHASE, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE], ActiveWave);
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_SOURCEENABLED, NULL, 0, NULL, 0);
	WidgetSNConfig(IDC_OFFSETX, NULL);
	WidgetSNConfig(IDC_OFFSETY, NULL);
	WidgetSNConfig(IDC_WAVELEN, NULL);
	WidgetSNConfig(IDC_VEL, NULL);
	WidgetSmartRAHConfig(IDC_SOURCEAMP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PHASE, (RasterAnimHost *)NULL, NULL);
	} // else

} // CloudEditGUI::ConfigureWave

/*===========================================================================*/

void CloudEditGUI::SyncWidgets(void)
{
Texture *Tex;
double Width;
char TextStr[48];
long Ct;

if (Active->ColorGrad.GetActiveNode() != ActiveGrad)
	{
	ConfigureWidgets();
	return;
	} // if

WidgetFISync(IDC_ROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_COLS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NUMLAYERS, WP_FISYNC_NONOTIFY);

WidgetSNSync(IDC_MAPHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAPWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CENTERLAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CENTERLON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ALT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COVERAGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FALLOFF, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_THICKNESS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SELFSHADE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SHADOWINTENS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINSAMPLE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXSAMPLE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSLUMINANCE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSLUMEXP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECCOLORPCT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BACKLIGHTCOLOR, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSHADOWSONLY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCLOUDFEATHER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATMOSPHERE, WP_SCSYNC_NONOTIFY);

EvolveFast = (Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && (Tex = Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex) && 
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE) &&
	(Tex->TexVelocity[2].CurValue != 0.0));
if (EvolveFast)
	EvolveFast = (Tex->TexVelocity[2].CurValue > 45.0) ? 2: 1;	// cutoff is 1.5 meters/frame or 45 m/sec

WidgetSCSync(IDC_CHECKCASTSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSTER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSFOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWS3D, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSCSM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSVOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSMISC, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKUSEMAPFILE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREGENMAPFILE, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOEVOLVEFAST, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOEVOLVESLOW, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOEVOLVENOT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPLARGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPMED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPSMALL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCASTSHADOWMAP, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWCASTVOLUMETRIC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWCASTCOMBINATION, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOBEFOREREFL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAFTERREFL, WP_SRSYNC_NONOTIFY);

WidgetSetCheck(IDC_PRESETCIRRUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CIRRUS);
WidgetSetCheck(IDC_PRESETSTRATUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_STRATUS);
WidgetSetCheck(IDC_PRESETNIMBUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_NIMBUS);
WidgetSetCheck(IDC_PRESETCUMULUS, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CUMULUS);
WidgetSetCheck(IDC_PRESETCUSTOM, Active->CloudType == WCS_EFFECTS_CLOUDTYPE_CUSTOM);

Width = max(Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue, Active->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue);
for (Ct = 1; Ct < Active->PreviewSize; Ct ++)
	Width /= 2.0;
Width /= 1000.0;
sprintf(TextStr, "%1.1f Km", Width);
WidgetSetText(IDC_PREVIEWSIZETXT, TextStr);

Width = 50000.0;
for (Ct = 1; Ct < Active->WavePreviewSize; Ct ++)
	Width /= 2.0;
sprintf(TextStr, "%1.1f m", Width);
WidgetSetText(IDC_PREVIEWSIZETXT3, TextStr);

if (ActiveWaveValid())
	{
	WidgetSNSync(IDC_OFFSETX, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_OFFSETY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SOURCEAMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_WAVELEN, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_PHASE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_VEL, WP_FISYNC_NONOTIFY);

	WidgetSCSync(IDC_SOURCEENABLED, WP_SCSYNC_NONOTIFY);
	} // if

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

// Material GUI
MatGUI->SyncWidgets();

} // CloudEditGUI::SyncWidgets

/*===========================================================================*/

void CloudEditGUI::DisableWidgets(void)
{
char EvolveEnabled;
Texture *Tex;

WidgetSetDisabled(IDC_FALLOFF, ! Active->Feather);

WidgetSetDisabled(IDC_COLOR1, ! ActiveGrad);

WidgetSetDisabled(IDC_ZOOMOUT, Active->PreviewSize <= 1);
WidgetSetDisabled(IDC_ZOOMOUT3, Active->WavePreviewSize <= 1);

EvolveEnabled = (Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && (Tex = Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex) &&
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE));
EvolveFast = (EvolveEnabled &&
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE) &&
	(Tex->TexVelocity[2].CurValue != 0.0));
if (EvolveFast)
	EvolveFast = (Tex->TexVelocity[2].CurValue > 45.0) ? 2: 1;	// cutoff is 1.5 meters/frame or 45 m/sec

WidgetSetDisabled(IDC_RADIOEVOLVEFAST, ! (EvolveEnabled));
WidgetSetDisabled(IDC_RADIOEVOLVESLOW, ! (EvolveEnabled));
WidgetSetDisabled(IDC_RADIOEVOLVENOT, ! (EvolveEnabled));

WidgetSetDisabled(IDC_CHECKREGENMAPFILE, ! (Active->UseMapFile && (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));
WidgetSetDisabled(IDC_SHADOWINTENS, ! (Active->ReceiveShadowsTerrain || Active->ReceiveShadowsFoliage || 
	Active->ReceiveShadows3DObject || Active->ReceiveShadowsCloudSM || Active->ReceiveShadowsVolumetric || 
	Active->ReceiveShadowsMisc));
WidgetSetDisabled(IDC_CHECKUSEMAPFILE, ! (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
WidgetSetDisabled(IDC_RADIOSHADOWMAPVERYLARGE, ! (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
WidgetSetDisabled(IDC_RADIOSHADOWMAPLARGE, ! (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
WidgetSetDisabled(IDC_RADIOSHADOWMAPMED, ! (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
WidgetSetDisabled(IDC_RADIOSHADOWMAPSMALL, ! (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));

WidgetSetDisabled(IDC_RADIOCASTSHADOWMAP, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWCASTVOLUMETRIC, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWCASTCOMBINATION, ! Active->CastShadows);
WidgetSetDisabled(IDC_CASTCOMBINATIONTEXT, ! Active->CastShadows);

WidgetSetDisabled(IDC_RADIOBEFOREREFL, ! Active->Volumetric);
WidgetSetDisabled(IDC_RADIOAFTERREFL, ! Active->Volumetric);

WidgetSetDisabled(IDC_MINSAMPLE, ! (Active->Volumetric || (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));
WidgetSetDisabled(IDC_MAXSAMPLE, ! (Active->Volumetric || (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));
WidgetSetDisabled(IDC_NUMLAYERS, ! (! Active->Volumetric || (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));
WidgetSetDisabled(IDC_LAYERTHICKNESS, ! (! Active->Volumetric || (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));
WidgetSetDisabled(IDC_LAYERTHICK_TXT, ! (! Active->Volumetric || (Active->CastShadows && (Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP
	|| Active->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION))));

// need vectors
WidgetSetDisabled(IDC_CHECKGRADFILL, ! Active->Joes);
WidgetSetDisabled(IDC_EDITPROFILE, ! Active->Joes);

// Material GUI
MatGUI->DisableWidgets();

} // CloudEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void CloudEditGUI::DisplayAdvancedFeatures(void)
{
char OldType = Active->CloudType;

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

Active->CloudType = OldType;

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_CHECKATMOSPHERE, true);
	WidgetShow(IDC_RADIOEVOLVEFAST, true);
	WidgetShow(IDC_RADIOEVOLVESLOW, true);
	WidgetShow(IDC_RADIOEVOLVENOT, true);
	WidgetShow(IDC_SELFSHADE, true);
	WidgetShow(IDC_EDITSHADEPROFILE, true);
	WidgetShow(IDC_THICKNESS, true);
	WidgetShow(IDC_NUMLAYERS, true);
	WidgetShow(IDC_THICKNESS_BOX, true);
	WidgetShow(IDC_LAYERTHICKNESS, true);
	WidgetShow(IDC_LAYERTHICK_TXT, true);
	WidgetShow(IDC_VOLUMETRIC_BOX, true);
	WidgetShow(IDC_VOLUMETRIC_TXT, true);
	WidgetShow(IDC_SOURCELIST, true);
	WidgetShow(IDC_ADDSOURCE, true);
	WidgetShow(IDC_REMOVESOURCE, true);
	WidgetShow(IDC_TNAIL3, true);
	WidgetShow(IDC_PREVIEWSIZETXT3, true);
	WidgetShow(IDC_ZOOMIN3, true);
	WidgetShow(IDC_ZOOMOUT3, true);
	WidgetShow(IDC_SOURCEENABLED, true);
	WidgetShow(IDC_OFFSETX, true);
	WidgetShow(IDC_OFFSETY, true);
	WidgetShow(IDC_SOURCEAMP, true);
	WidgetShow(IDC_WAVELEN, true);
	WidgetShow(IDC_VEL, true);
	WidgetShow(IDC_PHASE, true);
	WidgetShow(IDC_ADDSOURCEINVIEW, true);
	WidgetShow(IDC_SETSOURCEINVIEW, true);
	WidgetShow(IDC_TRANSLUMINANCE, true);
	WidgetShow(IDC_TRANSLUMEXP, true);
	WidgetShow(IDC_SPECCOLORPCT, true);
	WidgetShow(IDC_BACKLIGHTCOLOR, true);
	WidgetShow(IDC_SOURCES_TXT, true);
	WidgetShow(IDC_WAVE_BOX, true);
	WidgetShow(IDC_SELSRC_BOX, true);
	WidgetShow(IDC_MINSAMPLE, true);
	WidgetShow(IDC_MAXSAMPLE, true);
	WidgetShow(IDC_RADIOBEFOREREFL, true);
	WidgetShow(IDC_RADIOAFTERREFL, true);
	WidgetShow(IDC_EDITVERTPROFILE, true);
	WidgetShow(IDC_EDITDENSITYPROFILE, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_CHECKATMOSPHERE, false);
	WidgetShow(IDC_RADIOEVOLVEFAST, false);
	WidgetShow(IDC_RADIOEVOLVESLOW, false);
	WidgetShow(IDC_RADIOEVOLVENOT, false);
	WidgetShow(IDC_SELFSHADE, false);
	WidgetShow(IDC_EDITSHADEPROFILE, false);
	WidgetShow(IDC_THICKNESS, false);
	WidgetShow(IDC_NUMLAYERS, false);
	WidgetShow(IDC_THICKNESS_BOX, false);
	WidgetShow(IDC_LAYERTHICKNESS, false);
	WidgetShow(IDC_LAYERTHICK_TXT, false);
	WidgetShow(IDC_VOLUMETRIC_BOX, false);
	WidgetShow(IDC_VOLUMETRIC_TXT, false);
	WidgetShow(IDC_SOURCELIST, false);
	WidgetShow(IDC_ADDSOURCE, false);
	WidgetShow(IDC_REMOVESOURCE, false);
	WidgetShow(IDC_TNAIL3, false);
	WidgetShow(IDC_PREVIEWSIZETXT3, false);
	WidgetShow(IDC_ZOOMIN3, false);
	WidgetShow(IDC_ZOOMOUT3, false);
	WidgetShow(IDC_SOURCEENABLED, false);
	WidgetShow(IDC_OFFSETX, false);
	WidgetShow(IDC_OFFSETY, false);
	WidgetShow(IDC_SOURCEAMP, false);
	WidgetShow(IDC_WAVELEN, false);
	WidgetShow(IDC_VEL, false);
	WidgetShow(IDC_PHASE, false);
	WidgetShow(IDC_ADDSOURCEINVIEW, false);
	WidgetShow(IDC_SETSOURCEINVIEW, false);
	WidgetShow(IDC_TRANSLUMINANCE, false);
	WidgetShow(IDC_TRANSLUMEXP, false);
	WidgetShow(IDC_SPECCOLORPCT, false);
	WidgetShow(IDC_BACKLIGHTCOLOR, false);
	WidgetShow(IDC_SOURCES_TXT, false);
	WidgetShow(IDC_WAVE_BOX, false);
	WidgetShow(IDC_SELSRC_BOX, false);
	WidgetShow(IDC_MINSAMPLE, false);
	WidgetShow(IDC_MAXSAMPLE, false);
	WidgetShow(IDC_RADIOBEFOREREFL, false);
	WidgetShow(IDC_RADIOAFTERREFL, false);
	WidgetShow(IDC_EDITVERTPROFILE, false);
	WidgetShow(IDC_EDITDENSITYPROFILE, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // CloudEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void CloudEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CloudEditGUI::Cancel

/*===========================================================================*/

void CloudEditGUI::Name(void)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // CloudEditGUI::Name()

/*===========================================================================*/

void CloudEditGUI::CloudType(void)
{
NotifyTag Changes[2];

if (WidgetGetCheck(IDC_PRESETCIRRUS))
	Active->CloudType = WCS_EFFECTS_CLOUDTYPE_CIRRUS;
else if (WidgetGetCheck(IDC_PRESETSTRATUS))
	Active->CloudType = WCS_EFFECTS_CLOUDTYPE_STRATUS;
else if (WidgetGetCheck(IDC_PRESETNIMBUS))
	Active->CloudType = WCS_EFFECTS_CLOUDTYPE_NIMBUS;
else if (WidgetGetCheck(IDC_PRESETCUMULUS))
	Active->CloudType = WCS_EFFECTS_CLOUDTYPE_CUMULUS;
else if (WidgetGetCheck(IDC_PRESETCUSTOM))
	Active->CloudType = WCS_EFFECTS_CLOUDTYPE_CUSTOM;
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
Active->SetCloudType();		// this should generate a notification about the coverage texture

} // CloudEditGUI::BlendStyle()

/*===========================================================================*/

void CloudEditGUI::ZoomScale(char InOrOut)
{
NotifyTag Changes[2];

Active->PreviewSize += InOrOut;
if (Active->PreviewSize < 1)
	Active->PreviewSize = 1;
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CloudEditGUI::ZoomScale

/*===========================================================================*/

void CloudEditGUI::ZoomWaveScale(char InOrOut)
{
NotifyTag Changes[2];

Active->WavePreviewSize += InOrOut;
if (Active->WavePreviewSize < 1)
	Active->WavePreviewSize = 1;
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CloudEditGUI::ZoomWaveScale

/*===========================================================================*/

void CloudEditGUI::AddWave(void)
{

ActiveWave = Active->AddWave(NULL);
BuildWaveList();
ConfigureWave();
DisableWidgets();

} // CloudEditGUI::AddWave

/*===========================================================================*/

void CloudEditGUI::RemoveWave(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_SOURCELIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SOURCELIST, Ct))
			{
			NumSelected ++;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_SOURCELIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_SOURCELIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll);
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Wave Source", "There are no Wave Sources selected to remove.");
		} // else
	} // if

} // CloudEditGUI::RemoveWave

/*===========================================================================*/

void CloudEditGUI::SetActiveWave(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_SOURCELIST);
Current = (long)WidgetLBGetItemData(IDC_SOURCELIST, Current);
if (Current != LB_ERR)
	ActiveWave = (WaveSource *)Current;
ConfigureWave();
DisableWidgets();

} // CloudEditGUI::SetActiveWave()

/*===========================================================================*/

long CloudEditGUI::HandleBackgroundCrunch(int Siblings)
{
int Handled = 0;

if (SampleData->Running)
	{
	if (Active->EvalOneSampleLine(SampleData) || ! (SampleData->y % 10))
		{
		ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, Rast);
		} // if
	Handled = 1;
	} // if
if (WaveSampleData->Running)
	{
	if (Active->EvalOneWaveSampleLine(WaveSampleData) || ! (WaveSampleData->y % 10))
		{
		ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, WaveRast);
		} // if
	Handled = 1;
	} // if

if (! Handled)
	{
	// we're all done
	BackgroundInstalled = 0;
	return (1);		// 1 uninstalls background process
	} // if

// we've got more to do, come again when you have more time
return (0);

} // CloudEditGUI::HandleBackgroundCrunch

/*===========================================================================*/

void CloudEditGUI::UpdateThumbnail(void)
{

SampleData->PreviewDirection = 2;
SampleData->AndChildren = 1;
WaveSampleData->PreviewDirection = 2;
WaveSampleData->AndChildren = 1;
if (Active->EvalSampleInit(Rast, SampleData))
	{
	if (! BackgroundInstalled)
		{
		if (GlobalApp->AddBGHandler(this))
			BackgroundInstalled = 1;
		} // if
	} // if initialized
if (Active->EvalWaveSampleInit(WaveRast, WaveSampleData))
	{
	if (! BackgroundInstalled)
		{
		if (GlobalApp->AddBGHandler(this))
			BackgroundInstalled = 1;
		} // if
	} // if initialized

} // CloudEditGUI::UpdateThumbnail

/*===========================================================================*/

void CloudEditGUI::DoEvolution(void)
{
Texture *Tex;
NotifyTag Changes[2];

if (Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && (Tex = Active->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex) &&
	(Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE))
	{
	if (EvolveFast)
		{
		Tex->TexVelocity[2].SetValue(EvolveFast > 1 ? 90.0: 15.0);
		} // if
	else
		{
		Tex->TexVelocity[2].SetValue(0.0);
		} // else
	Changes[0] = MAKE_ID(Tex->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Tex->GetRAHostRoot());
	} // if

} // CloudEditGUI::DoEvolution

/*===========================================================================*/

void CloudEditGUI::SetBounds(DiagnosticData *Data)
{

if (ReceivingDiagnostics == 0)
	{
	if (UserMessageOKCAN("Set Cloud Model Bounds", "The next two points clicked in any View\n will become this Cloud Model's new bounds.\n\nPoints may be selected in any order."))
		{
		ReceivingDiagnostics = 1;
		ReceivingWave = 0;
		ReceivingWaveSource = 0;
		GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		} // if
	} // if
else if (ReceivingDiagnostics == 1)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			ReceivingDiagnostics = 2;
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else if (ReceivingDiagnostics == 2)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			Active->SetBounds(LatEvent, LonEvent);
			ReceivingDiagnostics = 0;
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // CloudEditGUI::SetBounds

/*===========================================================================*/

void CloudEditGUI::SetSourcePosInView(DiagnosticData *Data)
{

if (ActiveWaveValid())
	{
	if (ReceivingWaveSource == 0)
		{
		if (UserMessageOKCAN("Set Wave Source Position", "The next point clicked in any View will\n become the active Wave Source's new position."))
			{
			ReceivingWaveSource = 1;
			ReceivingWave = 0;
			ReceivingDiagnostics = 0;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingWaveSource == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				Active->SetSourcePosition(ActiveWave, LatEvent[0], LonEvent[0]);
				ReceivingWaveSource = 0;
				} // if
			} // if
		else
			ReceivingWaveSource = 0;
		} // else if
	else
		ReceivingWaveSource = 0;
	} // if
else
	{
	UserMessageOK("Set Wave Source Position", "There is no active Wave Source selected in the list");
	ReceivingWaveSource = 0;
	} // else

WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // CloudEditGUI::SetSourcePosInView

/*===========================================================================*/

void CloudEditGUI::AddWaveInView(DiagnosticData *Data)
{

if (ReceivingWave == 0)
	{
	if (UserMessageOKCAN("Add Wave Source", "The next point clicked in any View will\n create a new Wave Source for this Cloud Model."))
		{
		ReceivingWave = 1;
		ReceivingWaveSource = 0;
		ReceivingDiagnostics = 0;
		GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		} // if
	} // if
else if (ReceivingWave == 1)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			Active->AddWave(LatEvent[0], LonEvent[0]);
			ReceivingWave = 0;
			} // if
		} // if
	else
		ReceivingWave = 0;
	} // else if
else
	ReceivingWave = 0;

WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // CloudEditGUI::AddWaveInView

/*===========================================================================*/

void CloudEditGUI::CopyWave(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost;
char CopyMsg[256];

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveWaveValid() && (CopyHost = ActiveWave))
		{
		CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		CopyHost->GetRAHostProperties(CopyProp);
		if (CopyProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			RasterAnimHost::SetCopyOfRAHost(CopyHost);
			sprintf(CopyMsg, "%s %s copied to clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
			} // if
		else
			{
			UserMessageOK("Copy", "Selected item cannot be copied.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // CloudEditGUI::CopyWave

/*===========================================================================*/

void CloudEditGUI::PasteWave(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *PasteHost, *CopyHost, *TempHost;
char CopyMsg[256], CopyIllegal = 0;

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveWaveValid() && (PasteHost = ActiveWave))
		{
		if (CopyHost = RasterAnimHost::GetCopyOfRAHost())
			{
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			CopyHost->GetRAHostProperties(CopyProp);
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
			PasteHost->GetRAHostProperties(CopyProp);
			if (CopyProp->DropOK)
				{
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				CopyProp->DropSource = CopyHost;
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				// determine if copying is legal - can't copy from a texture to a child of itself or a parent of itself
				TempHost = PasteHost;
				while (TempHost)
					{
					if (CopyHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->RAParent;
					} // while
				TempHost = CopyHost;
				while (TempHost)
					{
					if (PasteHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->RAParent;
					} // while
				if (! CopyIllegal)
					{
					PasteHost->SetRAHostProperties(CopyProp);
					sprintf(CopyMsg, "%s %s pasted from clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Can't copy from parent to child or child to its parent or copy texture to itself.");
					} // else
				} // if
			else
				{
				UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // CloudEditGUI::PasteWave

/*===========================================================================*/

// Material GUI
void CloudEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureColors();
} // CloudEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void CloudEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveGrad(NewNode);
} // CloudEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void CloudEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, MatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(MatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT, true);
	//WidgetShow(IDC_ANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // CloudEditGUI::ShowMaterialPopDrop

/*===========================================================================*/

bool CloudEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->Volumetric || Active->WaveSources ||
	! Active->ConfirmType() || EvolveFast ? true : false);

} // CloudEditGUI::QueryLocalDisplayAdvancedUIVisibleState

