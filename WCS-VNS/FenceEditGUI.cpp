// FenceEditGUI.cpp
// Code for Fence editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "FenceEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

char *FenceEditGUI::TabNames[WCS_FENCEGUI_NUMTABS] = {"General", "Panels", "Roof"};

long FenceEditGUI::ActivePage;
// advanced
long FenceEditGUI::DisplayAdvanced;

// material GUI
#define WCS_FENCEED_PANEL_MATGRADSET	1
#define WCS_FENCEED_ROOF_MATGRADSET	3

NativeGUIWin FenceEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // FenceEditGUI::Open

/*===========================================================================*/

NativeGUIWin FenceEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_FENCE_GENERAL_VNS, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_FENCE_GENERAL, 0, 0);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_FENCE_PANEL, 0, 1);
	CreateSubWinFromTemplate(IDD_FENCE_ROOF, 0, 2);

	if(NativeWin)
		{
		#ifndef WCS_FENCE_LIMITED
		for (TabIndex = 0; TabIndex < WCS_FENCEGUI_NUMTABS; TabIndex ++)
		#else // WCS_FENCE_LIMITED
		for (TabIndex = 0; TabIndex < WCS_FENCEGUI_NUMTABS - 2; TabIndex ++)
		#endif // WCS_FENCE_LIMITED
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// Material GUI
		PanelMatGUI->Construct(WCS_FENCEED_PANEL_MATGRADSET, WCS_FENCEED_PANEL_MATGRADSET + 1);
		RoofMatGUI->Construct(WCS_FENCEED_ROOF_MATGRADSET, WCS_FENCEED_ROOF_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // FenceEditGUI::Construct

/*===========================================================================*/

FenceEditGUI::FenceEditGUI(EffectsLib *EffectsSource, Database *DBSource, Fence *ActiveSource)
: GUIFenetre('FNCE', this, "Wall Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_FENCE, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_FOLIAGE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_FOLIAGE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_FOLIAGE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
ActiveSpanGrad = ActiveRoofGrad = NULL;
// Material GUI
RoofMatGUI = PanelMatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Wall Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// Material GUI
	if (PanelMatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->SpanMat, WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER, WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER)) // init ordinal 0
		{
		PopDropPanelMaterialNotifier.Host = this; // to be able to call notifications later
		PanelMatGUI->SetNotifyFunctor(&PopDropPanelMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	if (RoofMatGUI = new PortableMaterialGUI(1, this, EffectsSource, Active, &Active->RoofMat, WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER, WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER)) // init ordinal 1
		{
		PopDropRoofMaterialNotifier.Host = this; // to be able to call notifications later
		RoofMatGUI->SetNotifyFunctor(&PopDropRoofMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // FenceEditGUI::FenceEditGUI

/*===========================================================================*/

FenceEditGUI::~FenceEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (PanelMatGUI)
	delete PanelMatGUI;
if (RoofMatGUI)
	delete RoofMatGUI;

} // FenceEditGUI::~FenceEditGUI()

/*===========================================================================*/

long FenceEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_FCG, 0);

return(0);

} // FenceEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long FenceEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // FenceEditGUI::HandleShowAdvanced

/*===========================================================================*/

long FenceEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_FCG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveSpanGrad && ActiveSpanGrad->GetThing())
			((MaterialEffect *)ActiveSpanGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveSpanGrad && ActiveSpanGrad->GetThing())
			((MaterialEffect *)ActiveSpanGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_LOADROOFCOMPONENT:
		{
		if (ActiveRoofGrad && ActiveRoofGrad->GetThing())
			((MaterialEffect *)ActiveRoofGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVEROOFCOMPONENT:
		{
		if (ActiveRoofGrad && ActiveRoofGrad->GetThing())
			((MaterialEffect *)ActiveRoofGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	// material GUI
	case IDC_POPDROP0:
		{
		if(WidgetGetCheck(IDC_POPDROP0))
			{
			ShowPanelMaterialPopDrop(true);			} // if
		else
			{
			ShowPanelMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	case IDC_POPDROP1:
		{
		if(WidgetGetCheck(IDC_POPDROP1))
			{
			ShowRoofMaterialPopDrop(true);			} // if
		else
			{
			ShowRoofMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP1
	default: break;
	} // ButtonID

// Material GUI
PanelMatGUI->HandleButtonClick(Handle, NW, ButtonID);
RoofMatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // FenceEditGUI::HandleButtonClick

/*===========================================================================*/

long FenceEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
PanelMatGUI->HandleCBChange(Handle, NW, CtrlID);
RoofMatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // FenceEditGUI::HandleCBChange

/*===========================================================================*/

long FenceEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

// Material GUI
PanelMatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);
RoofMatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);

return (0);

} // FenceEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long FenceEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// Material GUI
ShowPanelMaterialPopDrop(false);
ShowRoofMaterialPopDrop(false);

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
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		ActivePage = NewPageID;
		break;
		}
	default:
		break;
	} // switch

return(0);

} // FenceEditGUI::HandlePageChange

/*===========================================================================*/

long FenceEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_SKINFRAME:
	case IDC_PANELENABLED:
	case IDC_ROOFENABLED:
	case IDC_ROOFMATENABLED:
	case IDC_CONNECTORIGIN:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // FenceEditGUI::HandleSCChange

/*===========================================================================*/

long FenceEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOABSOLUTE:
	case IDC_RADIORELATIVEGRND:
	case IDC_RADIORELATIVEJOE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // FenceEditGUI::HandleSRChange

/*===========================================================================*/

long FenceEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
PanelMatGUI->HandleFIChange(Handle, NW, CtrlID);
RoofMatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // FenceEditGUI::HandleFIChange

/*===========================================================================*/

void FenceEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[8];
long Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[6] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[7] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->SpanMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), Active->SpanMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigurePanel();
	ConfigureRoof();
	SyncWidgets();
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

#ifdef WCS_BUILD_VNS
// query drop
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	WidgetLWSync(IDC_VECLINKAGE);
	Done = 1;
	} // if query changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // FenceEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void FenceEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

sprintf(TextStr, "Wall Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
#ifndef WCS_FENCE_LIMITED
ConfigureSC(NativeWin, IDC_SKINFRAME, &Active->SkinFrame, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_PANELENABLED, &Active->SpansEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_ROOFENABLED, &Active->RoofEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_ROOFMATENABLED, &Active->SeparateRoofMat, SCFlag_Char, NULL, 0);
#endif // WCS_FENCE_LIMITED
ConfigureSC(NativeWin, IDC_CONNECTORIGIN, &Active->ConnectToOrigin, SCFlag_Char, NULL, 0);

#ifndef WCS_FENCE_LIMITED
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIOABSOLUTE, &Active->Absolute, SRFlag_Short, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEGRND, &Active->Absolute, SRFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEJOE, &Active->Absolute, SRFlag_Short, 2, NULL, NULL);
#else // WCS_FENCE_LIMITED
ConfigureSR(NativeWin, IDC_RADIORELATIVEGRND, IDC_RADIORELATIVEGRND, &Active->Absolute, SRFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORELATIVEGRND, IDC_RADIORELATIVEJOE, &Active->Absolute, SRFlag_Short, 2, NULL, NULL);
#endif // WCS_FENCE_LIMITED

WidgetSmartRAHConfig(IDC_PANELTOPELEV, &Active->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV], Active);
WidgetSmartRAHConfig(IDC_PANELBOTELEV, &Active->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV], Active);
#ifndef WCS_FENCE_LIMITED
WidgetSmartRAHConfig(IDC_ROOFELEV, &Active->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV], Active);
#endif // WCS_FENCE_LIMITED

WidgetAGConfig(IDC_PANELANIMGRADIENT, &Active->SpanMat);
WidgetAGConfig(IDC_ROOFANIMGRADIENT, &Active->RoofMat);

// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_POPDROP1, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_LOADROOFCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVEROOFCOMPONENT, IDI_FILESAVE, NULL);
// Material GUI
PanelMatGUI->ConfigureWidgets();
RoofMatGUI->ConfigureWidgets();

ConfigurePanel();
ConfigureRoof();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // FenceEditGUI::ConfigureWidgets()

/*===========================================================================*/

void FenceEditGUI::ConfigurePanel(void)
{
char GroupWithMatName[200];
MaterialEffect *Mat;

if ((ActiveSpanGrad = Active->SpanMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveSpanGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_INTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)&Mat->Strata, Mat);

	sprintf(GroupWithMatName, "Selected Panel Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS, GroupWithMatName);
	} // if
else
	{
	// configure everything to NULL
	WidgetSmartRAHConfig(IDC_LUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)NULL, NULL);

	WidgetSetText(IDC_MATERIALS, "Selected Panel Material");
	} // else

// Material GUI
PanelMatGUI->ConfigureMaterial();

} // FenceEditGUI::ConfigurePanel

/*===========================================================================*/

void FenceEditGUI::ConfigureRoof(void)
{
char GroupWithMatName[200];
MaterialEffect *Mat;

if ((ActiveRoofGrad = Active->RoofMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveRoofGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_ROOFLUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFTRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFSPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFSPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_ROOFREFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFBUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_ROOFBUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_ROOFDIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	WidgetSmartRAHConfig(IDC_ROOFSTRATA, (RasterAnimHost **)&Mat->Strata, Mat);

	sprintf(GroupWithMatName, "Selected Roof Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS2, GroupWithMatName);
	} // if
else
	{
	// configure everything to NULL
	WidgetSmartRAHConfig(IDC_ROOFLUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFTRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFSPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFSPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFREFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFBUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFBUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFDIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ROOFSTRATA, (RasterAnimHost **)NULL, NULL);

	WidgetSetText(IDC_MATERIALS2, "Selected Roof Material");
	} // else

// Material GUI
RoofMatGUI->ConfigureMaterial();

} // FenceEditGUI::ConfigureRoof

/*===========================================================================*/

void FenceEditGUI::SyncWidgets(void)
{

if ((Active->SpanMat.GetActiveNode() != ActiveSpanGrad)
	|| (Active->RoofMat.GetActiveNode() != ActiveRoofGrad))
	{
	ConfigureWidgets();
	return;
	} // if

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
#ifndef WCS_FENCE_LIMITED
WidgetSCSync(IDC_SKINFRAME, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_PANELENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_ROOFENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_ROOFMATENABLED, WP_SCSYNC_NONOTIFY);
#endif // WCS_FENCE_LIMITED
WidgetSCSync(IDC_CONNECTORIGIN, WP_SCSYNC_NONOTIFY);

#ifndef WCS_FENCE_LIMITED
WidgetSRSync(IDC_RADIOABSOLUTE, WP_SRSYNC_NONOTIFY);
#endif // WCS_FENCE_LIMITED
WidgetSRSync(IDC_RADIORELATIVEGRND, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEJOE, WP_SRSYNC_NONOTIFY);

WidgetAGConfig(IDC_PANELANIMGRADIENT, &Active->SpanMat);
WidgetAGConfig(IDC_ROOFANIMGRADIENT, &Active->RoofMat);

WidgetSNSync(IDC_PANELTOPELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PANELBOTELEV, WP_FISYNC_NONOTIFY);
#ifndef WCS_FENCE_LIMITED
WidgetSNSync(IDC_ROOFELEV, WP_FISYNC_NONOTIFY);
#endif // WCS_FENCE_LIMITED

if (ActiveSpanGrad = Active->SpanMat.GetActiveNode())
	{
	WidgetSNSync(IDC_LUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_STRATA, WP_FISYNC_NONOTIFY);
	} // if

if (ActiveRoofGrad = Active->RoofMat.GetActiveNode())
	{
	WidgetSNSync(IDC_ROOFLUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFTRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFSPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFSPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFREFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFBUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFBUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFDIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ROOFSTRATA, WP_FISYNC_NONOTIFY);
	} // if

// Material GUI
RoofMatGUI->SyncWidgets();
PanelMatGUI->SyncWidgets();

} // FenceEditGUI::SyncWidgets

/*===========================================================================*/

void FenceEditGUI::DisableWidgets(void)
{

// material
WidgetSetDisabled(IDC_LUMINOSITY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_TRANSPARENCY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_SPECULARITY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_SPECULAREXP, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_REFLECTIVITY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_INTENSITY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_BUMPINTENSITY, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_BUMP, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_DIFFUSECOLOR, ! ActiveSpanGrad);
WidgetSetDisabled(IDC_STRATA, ! ActiveSpanGrad);

WidgetSetDisabled(IDC_ROOFLUMINOSITY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFTRANSPARENCY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFSPECULARITY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFSPECULAREXP, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFREFLECTIVITY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFINTENSITY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFBUMPINTENSITY, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFBUMP, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFDIFFUSECOLOR, ! ActiveRoofGrad);
WidgetSetDisabled(IDC_ROOFSTRATA, ! ActiveRoofGrad);

#ifndef WCS_FENCE_LIMITED
WidgetSetDisabled(IDC_HEIGHTSTXT, Active->SkinFrame);
WidgetSetDisabled(IDC_RADIOABSOLUTE, Active->SkinFrame);
WidgetSetDisabled(IDC_RADIORELATIVEGRND, Active->SkinFrame);
WidgetSetDisabled(IDC_RADIORELATIVEJOE, Active->SkinFrame);
WidgetSetDisabled(IDC_CONNECTORIGIN, Active->SkinFrame);
WidgetSetDisabled(IDC_PANELENABLED, Active->SkinFrame);
WidgetSetDisabled(IDC_PANELTOPELEV, Active->SkinFrame);
WidgetSetDisabled(IDC_PANELBOTELEV, Active->SkinFrame);
WidgetSetDisabled(IDC_ROOFENABLED, Active->SkinFrame);
WidgetSetDisabled(IDC_ROOFMATENABLED, ! (Active->RoofEnabled || Active->SkinFrame));
WidgetSetDisabled(IDC_ROOFELEV, Active->SkinFrame || ! Active->RoofEnabled);
#endif // WCS_FENCE_LIMITED

// Material GUI
PanelMatGUI->DisableWidgets();
RoofMatGUI->DisableWidgets();

} // FenceEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void FenceEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

// All the material properties are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced)
	{
	// panel
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_PANELANIMGRADIENT, ! PanelMatGUI->QueryIsDisplayed());
	WidgetShow(IDC_POPDROP0, true);
	// roof
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_ROOFLUMINOSITY, true);
	WidgetShow(IDC_ROOFTRANSPARENCY, true);
	WidgetShow(IDC_ROOFSPECULARITY, true);
	WidgetShow(IDC_ROOFSPECULAREXP, true);
	WidgetShow(IDC_ROOFREFLECTIVITY, true);
	WidgetShow(IDC_ROOFANIMGRADIENT, ! RoofMatGUI->QueryIsDisplayed());
	WidgetShow(IDC_POPDROP1, true);
	} 
else
	{
	// panel
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_PANELANIMGRADIENT, false);
	WidgetShow(IDC_POPDROP0, false);
	// roof
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_ROOFLUMINOSITY, false);
	WidgetShow(IDC_ROOFTRANSPARENCY, false);
	WidgetShow(IDC_ROOFSPECULARITY, false);
	WidgetShow(IDC_ROOFSPECULAREXP, false);
	WidgetShow(IDC_ROOFREFLECTIVITY, false);
	WidgetShow(IDC_ROOFANIMGRADIENT, false);
	WidgetShow(IDC_POPDROP1, false);
	// Material GUI
	ShowPanelMaterialPopDrop(false);
	ShowRoofMaterialPopDrop(false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // FenceEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void FenceEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // FenceEditGUI::Cancel

/*===========================================================================*/

void FenceEditGUI::Name(void)
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

} // FenceEditGUI::Name()

/*===========================================================================*/

// Material GUI
void FenceEditGUIPanelPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigurePanel();
} // FenceEditGUIPanelPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void FenceEditGUIPanelPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActivePanelGrad(NewNode);
} // FenceEditGUIPanelPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void FenceEditGUIRoofPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureRoof();
} // FenceEditGUIRoofPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void FenceEditGUIRoofPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveRoofGrad(NewNode);
} // FenceEditGUIRoofPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// material GUI
void FenceEditGUI::ShowPanelMaterialPopDrop(bool ShowState)
{
if(ShowState)
	{
	// position and show
	if(PanelMatGUI)
		{
		ShowPanelAsPopDrop(IDC_POPDROP0, PanelMatGUI->GetPanel(), 0, SubPanels[0][1]);
		WidgetShow(IDC_PANELANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
		WidgetSetCheck(IDC_POPDROP0, true);
		} // if
	} // if
else
	{
	if(PanelMatGUI)
		{
		ShowPanel(PanelMatGUI->GetPanel(), -1); // hide
		WidgetShow(IDC_PANELANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
		WidgetSetCheck(IDC_POPDROP0, false);
		} // if
	} // else

} // FenceEditGUI::ShowPanelMaterialPopDrop

/*===========================================================================*/

void FenceEditGUI::ShowRoofMaterialPopDrop(bool ShowState)
{
if(ShowState)
	{
	// position and show
	if(RoofMatGUI)
		{
		ShowPanelAsPopDrop(IDC_POPDROP1, RoofMatGUI->GetPanel(), 0, SubPanels[0][1]);
		WidgetShow(IDC_ROOFANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
		WidgetSetCheck(IDC_POPDROP1, true);
		} // if
	} // if
else
	{
	if(RoofMatGUI)
		{
		ShowPanel(RoofMatGUI->GetPanel(), -1); // hide
		WidgetShow(IDC_ROOFANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
		WidgetSetCheck(IDC_POPDROP1, false);
		} // if
	} // else

} // FenceEditGUI::ShowRoofMaterialPopDrop

/*===========================================================================*/

bool FenceEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->SpanMat.CountNodes() > 1 || Active->RoofMat.CountNodes() > 1 ? true : false);

} // FenceEditGUI::QueryLocalDisplayAdvancedUIVisibleState

