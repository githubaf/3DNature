// RasterTAEditGUI.cpp
// Code for RasterTA editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RasterTAEditGUI.h"
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
#ifdef WCS_DEM_TFX_FREEZE
#include "DEMEval.h"
#endif // WCS_DEM_TFX_FREEZE
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


char *RasterTAEditGUI::TabNames[WCS_RASTERTAGUI_NUMTABS] = {"General", "Elevation && Roughness"};

long RasterTAEditGUI::ActivePage;

NativeGUIWin RasterTAEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // RasterTAEditGUI::Open

/*===========================================================================*/

NativeGUIWin RasterTAEditGUI::Construct(void)
{
int TabIndex;
//char *CompareMethods[] = {"First Found", "Random Select", "Weighted Average", "Maximum Value", "Minimum Value", "Sum Values"};

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_RASTERTA_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_RASTERTA_BASIC, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_RASTERTAGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // RasterTAEditGUI::Construct

/*===========================================================================*/

RasterTAEditGUI::RasterTAEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, RasterTerraffectorEffect *ActiveSource)
: GUIFenetre('ATEF', this, "Area Terraffector Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_RASTERTA, 0xff, 0xff, 0xff), 
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Area Terraffector Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // RasterTAEditGUI::RasterTAEditGUI

/*===========================================================================*/

RasterTAEditGUI::~RasterTAEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // RasterTAEditGUI::~RasterTAEditGUI()

/*===========================================================================*/

long RasterTAEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_RTG, 0);

return(0);

} // RasterTAEditGUI::HandleCloseWin

/*===========================================================================*/

long RasterTAEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_RTG, 0);
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
#ifdef WCS_DEM_TFX_FREEZE
	case IDC_FREEZEALL:
		{
		DEMEval Frigidaire;
		Frigidaire.FreezeDEMTFXUI(NULL, NULL);
		break;
		} // IDC_FREEZEALL
#endif // WCS_DEM_TFX_FREEZE
	default: break;
	} // ButtonID
return(0);

} // RasterTAEditGUI::HandleButtonClick

/*===========================================================================*/

long RasterTAEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RasterTAEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long RasterTAEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				break;
				} // 1
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

} // RasterTAEditGUI::HandlePageChange

/*===========================================================================*/

long RasterTAEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->RasterTerraffectorBase.SetFloating(EffectsHost->RasterTerraffectorBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RasterTAEditGUI::HandleSCChange

/*===========================================================================*/

long RasterTAEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIORELATIVE:
	case IDC_RADIOABSOLUTE:
	case IDC_RADIOMODTERRAIN:
	case IDC_RADIOUNMODTERRAIN:
	case IDC_RADIOINCORDEC:
	case IDC_RADIOINCONLY:
	case IDC_RADIODECONLY:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RasterTAEditGUI::HandleSRChange

/*===========================================================================*/

long RasterTAEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	//case IDC_RESOLUTION:
	//	{
	//	EffectsHost->RasterTerraffectorBase.SetFloating(0, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	case IDC_PRIORITY:
	case IDC_EVALORDER:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RasterTAEditGUI::HandleFIChange

/*===========================================================================*/

void RasterTAEditGUI::HandleNotifyEvent(void)
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
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

} // RasterTAEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void RasterTAEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->RasterTerraffectorBase.AreThereEdges((GeneralEffect *)EffectsHost->RasterTA))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->RasterTerraffectorBase.AreThereGradients((GeneralEffect *)EffectsHost->RasterTA))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->RasterTerraffectorBase.Resolution,
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

ConfigureFI(NativeWin, IDC_EVALORDER,
 &Active->EvalOrder,
  1.0,
   1.0,
	10000.0,
	 FIOFlag_Short,
	  NULL,
	   0);

sprintf(TextStr, "Area Terraffector Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->RasterTerraffectorBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->RasterTerraffectorBase.Floating, SCFlag_Short, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIORELATIVE, IDC_RADIORELATIVE, &Active->Absolute, SRFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORELATIVE, IDC_RADIOABSOLUTE, &Active->Absolute, SRFlag_Short, 1, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOMODTERRAIN, IDC_RADIOMODTERRAIN, &Active->ApplyToOrigElev, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOMODTERRAIN, IDC_RADIOUNMODTERRAIN, &Active->ApplyToOrigElev, SRFlag_Char, 1, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIOINCORDEC, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIOINCONLY, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIODECONLY, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_DECREASE, NULL, NULL);

WidgetSmartRAHConfig(IDC_ELEV, &Active->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV], Active);
WidgetSmartRAHConfig(IDC_ROUGHNESS, &Active->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS], Active);

DisableWidgets();

} // RasterTAEditGUI::ConfigureWidgets()

/*===========================================================================*/

void RasterTAEditGUI::SyncWidgets(void)
{
/*
char TextStr[32];

if (EffectsHost->RasterTerraffectorBase.AreThereEdges((GeneralEffect *)EffectsHost->RasterTA))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->RasterTerraffectorBase.AreThereGradients((GeneralEffect *)EffectsHost->RasterTA))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_EVALORDER, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ROUGHNESS, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKHIRESEDGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIORELATIVE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOABSOLUTE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOMODTERRAIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOUNMODTERRAIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCORDEC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCONLY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIODECONLY, WP_SRSYNC_NONOTIFY);

} // RasterTAEditGUI::SyncWidgets

/*===========================================================================*/

void RasterTAEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);
#ifdef WCS_DEM_TFX_FREEZE
WidgetShow(IDC_FREEZEALL, true);
#else // WCS_DEM_TFX_FREEZE
WidgetShow(IDC_FREEZEALL, false);
#endif // WCS_DEM_TFX_FREEZE

} // RasterTAEditGUI::DisableWidgets

/*===========================================================================*/

void RasterTAEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // RasterTAEditGUI::Cancel

/*===========================================================================*/

void RasterTAEditGUI::Name(void)
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

} // RasterTAEditGUI::Name()

/*===========================================================================*/

