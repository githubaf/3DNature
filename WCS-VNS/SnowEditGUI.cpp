// SnowEditGUI.cpp
// Code for Snow editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "SnowEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Database.h"
#include "Conservatory.h"
#include "GalleryGUI.h"
#include "BrowseDataGUI.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


char *SnowEditGUI::TabNames[WCS_SNOWGUI_NUMTABS] = {"General", "Material", "Rules", "Global Gradient"};

long SnowEditGUI::ActivePage;
// advanced
long SnowEditGUI::DisplayAdvanced;

// material GUI
#define WCS_SNOWED_MATGRADSET	1

NativeGUIWin SnowEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SnowEditGUI::Open

/*===========================================================================*/

NativeGUIWin SnowEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_SNOW_GENERAL_VNS3, 0, 0);
	CreateSubWinFromTemplate(IDD_SNOW_MATERIAL_VNS3, 0, 1);
	CreateSubWinFromTemplate(IDD_SNOW_RULES_VNS3, 0, 2);
	CreateSubWinFromTemplate(IDD_SNOW_MISC, 0, 3);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_SNOWGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// Material GUI
		MatGUI->Construct(WCS_SNOWED_MATGRADSET, WCS_SNOWED_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // SnowEditGUI::Construct

/*===========================================================================*/

SnowEditGUI::SnowEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, SnowEffect *ActiveSource)
: GUIFenetre('SNOW', this, "Snow Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_SNOW, 0xff, 0xff, 0xff), 
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
ProjHost = ProjSource;
Active = ActiveSource;
ActiveGrad = NULL;
// Material GUI
MatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Snow Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// material GUI
	if (MatGUI = new PortableMaterialGUI(0, this, EffectsSource, &Active->Eco, &Active->Eco.EcoMat, WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER, WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER)) // init ordinal 0
		{
		PopDropMaterialNotifier.Host = this; // to be able to call notifications later
		MatGUI->SetNotifyFunctor(&PopDropMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppDB->GetDEMElevRange(MaxTerrainElev, MinTerrainElev);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // SnowEditGUI::SnowEditGUI

/*===========================================================================*/

SnowEditGUI::~SnowEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (MatGUI)
	delete MatGUI;

} // SnowEditGUI::~SnowEditGUI()

/*===========================================================================*/

long SnowEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SNG, 0);

return(0);

} // SnowEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long SnowEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // SnowEditGUI::HandleShowAdvanced

/*===========================================================================*/

long SnowEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SNG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_EDITPROFILE:
		{
		Active->Eco.ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
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

} // SnowEditGUI::HandleButtonClick

/*===========================================================================*/

long SnowEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // SnowEditGUI::HandleCBChange

/*===========================================================================*/

long SnowEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
MatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);

return (0);

} // SnowEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long SnowEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // SnowEditGUI::HandlePageChange

/*===========================================================================*/

long SnowEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	//case IDC_CHECKOVERLAP:
	//case IDC_CHECKHIRESEDGE:
	case IDC_CHECKGRADFILL:
	case IDC_CHECKGRADENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->SnowBase.SetFloating(EffectsHost->SnowBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	case IDC_CHECKTRANSPARENT:
		{
		Changes[0] = MAKE_ID(Active->Eco.GetNotifyClass(), Active->Eco.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->Eco.GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // SnowEditGUI::HandleSCChange

/*===========================================================================*/

long SnowEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // SnowEditGUI::HandleFIChange

/*===========================================================================*/

void SnowEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	ConfigureColors();
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->Eco.EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), Active->Eco.EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureMaterial();
	ConfigureColors();
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

} // SnowEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void SnowEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->SnowBase.AreThereEdges((GeneralEffect *)EffectsHost->Snow))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->SnowBase.AreThereGradients((GeneralEffect *)EffectsHost->Snow))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->SnowBase.Resolution,
  1.0,
   0.00001,
	1000000.0,
	 FIOFlag_Float,
	  NULL,
	   NULL);*/

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

sprintf(TextStr, "Snow Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->SnowBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->SnowBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTRANSPARENT, &Active->Eco.Transparent, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADENABLED, &Active->GlobalGradientsEnabled, SCFlag_Char, NULL, 0);

WidgetSmartRAHConfig(IDC_FEATHERING, &Active->AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING], Active);
WidgetSmartRAHConfig(IDC_SNOW_SNOWGRAD, &Active->AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALSNOWGRAD], Active);
WidgetSmartRAHConfig(IDC_SNOW_REFLAT, &Active->AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT], Active);

WidgetAGConfig(IDC_ANIMGRADIENT2, &Active->Eco.EcoMat);
// Material GUI
MatGUI->ConfigureWidgets();

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
// material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);

ConfigureMaterial();
ConfigureColors();
ConfigureRules();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // SnowEditGUI::ConfigureWidgets()

/*===========================================================================*/

void SnowEditGUI::ConfigureRules(void)
{

WidgetSmartRAHConfig(IDC_ELEVLINE, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE], &Active->Eco);
WidgetSmartRAHConfig(IDC_ELEVSKEW, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW], &Active->Eco);
WidgetSmartRAHConfig(IDC_SKEWAZIMUTH, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ], &Active->Eco);
WidgetSmartRAHConfig(IDC_RELELEFFECT, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL], &Active->Eco);
WidgetSmartRAHConfig(IDC_MAXRELEL, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL], &Active->Eco);
WidgetSmartRAHConfig(IDC_MINRELEL, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL], &Active->Eco);
WidgetSmartRAHConfig(IDC_MAXSLOPE, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE], &Active->Eco);
WidgetSmartRAHConfig(IDC_MINSLOPE, &Active->Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE], &Active->Eco);

} // SnowEditGUI::ConfigureRules

/*===========================================================================*/


void SnowEditGUI::ConfigureMaterial(void)
{
char GroupWithMatName[200];
MaterialEffect *Mat;

if ((ActiveGrad = Active->Eco.EcoMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing()))
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

	sprintf(GroupWithMatName, "Selected Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS, GroupWithMatName);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);

	WidgetSetText(IDC_MATERIALS, "Selected Material");
	} // else
// material GUI
MatGUI->ConfigureMaterial();

} // SnowEditGUI::ConfigureMaterial

/*===========================================================================*/

void SnowEditGUI::ConfigureColors(void)
{

// this is harmless to call even if there is no active gradient node, it will cause 
// a valid node to be set if there is one..
WidgetAGSync(IDC_ANIMGRADIENT2);
MatGUI->SyncWidgets();
ActiveGrad = Active->Eco.EcoMat.GetActiveNode();

} // SnowEditGUI::ConfigureColors

/*===========================================================================*/

void SnowEditGUI::SyncWidgets(void)
{

if (Active->Eco.EcoMat.GetActiveNode() != ActiveGrad)
	{
	ConfigureWidgets();
	return;
	} // if

//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FEATHERING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVLINE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVSKEW, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SKEWAZIMUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RELELEFFECT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXRELEL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINRELEL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXSLOPE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINSLOPE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SNOW_SNOWGRAD, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SNOW_REFLAT, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKHIRESEDGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTRANSPARENT, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);

if (ActiveGrad = Active->Eco.EcoMat.GetActiveNode())
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
	} // if

// Material GUI
MatGUI->SyncWidgets();

} // SnowEditGUI::SyncWidgets

/*===========================================================================*/

void SnowEditGUI::DisableWidgets(void)
{

// global gradient
WidgetSetDisabled(IDC_SNOW_SNOWGRAD, ! Active->GlobalGradientsEnabled);
WidgetSetDisabled(IDC_SNOW_REFLAT, ! Active->GlobalGradientsEnabled);

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);

// Material GUI
MatGUI->DisableWidgets();

} // SnowEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void SnowEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_SNOW_SNOWGRAD, true);
	WidgetShow(IDC_SNOW_REFLAT, true);
	WidgetShow(IDC_CHECKGRADENABLED, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_SNOW_SNOWGRAD, false);
	WidgetShow(IDC_SNOW_REFLAT, false);
	WidgetShow(IDC_CHECKGRADENABLED, false);
	} // else

// Rules of nature are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced )
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_ELEVLINE, true);
	WidgetShow(IDC_ELEVSKEW, true);
	WidgetShow(IDC_SKEWAZIMUTH, true);
	WidgetShow(IDC_RELELEFFECT, true);
	WidgetShow(IDC_MAXRELEL, true);
	WidgetShow(IDC_MINRELEL, true);
	WidgetShow(IDC_MAXSLOPE, true);
	WidgetShow(IDC_MINSLOPE, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_ELEVLINE, true);
	WidgetShow(IDC_ELEVSKEW, false);
	WidgetShow(IDC_SKEWAZIMUTH, false);
	WidgetShow(IDC_RELELEFFECT, false);
	WidgetShow(IDC_MAXRELEL, false);
	WidgetShow(IDC_MINRELEL, false);
	WidgetShow(IDC_MAXSLOPE, true);
	WidgetShow(IDC_MINSLOPE, true);
	} // else
	
// All the material properties are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_ANIMGRADIENT2, ! MatGUI->QueryIsDisplayed());
	// Material GUI
	WidgetShow(IDC_POPDROP0, true);
	} 
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_ANIMGRADIENT2, false);
	// Material GUI
	WidgetShow(IDC_POPDROP0, false);
	ShowMaterialPopDrop(false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // SnowEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void SnowEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // SnowEditGUI::Cancel

/*===========================================================================*/

void SnowEditGUI::Name(void)
{
NotifyTag Changes[2];
char NewName[WCS_EFFECT_MAXNAMELENGTH];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // SnowEditGUI::Name()

/*===========================================================================*/

// material GUI
void SnowEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureMaterial();
} // SnowEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// material GUI
void SnowEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveGrad(NewNode);
} // SnowEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// material GUI
void SnowEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, MatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT2, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(MatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT2, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // SnowEditGUI::ShowMaterialPopDrop

/*===========================================================================*/

bool SnowEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->Eco.EcoMat.CountNodes() > 1 ? true : false);

} // SnowEditGUI::QueryLocalDisplayAdvancedUIVisibleState

