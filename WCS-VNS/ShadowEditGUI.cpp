// ShadowEditGUI.cpp
// Code for Shadow editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ShadowEditGUI.h"
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

char *ShadowEditGUI::TabNames[WCS_SHADOWGUI_NUMTABS] = {"General", "Cast Shadows", "Receive Shadows"};

long ShadowEditGUI::ActivePage;

NativeGUIWin ShadowEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ShadowEditGUI::Open

/*===========================================================================*/

NativeGUIWin ShadowEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_SHADOW_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_SHADOW_BASIC, 0, 1);
	CreateSubWinFromTemplate(IDD_SHADOW_RECEIVE, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_SHADOWGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // ShadowEditGUI::Construct

/*===========================================================================*/

ShadowEditGUI::ShadowEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ShadowEffect *ActiveSource)
: GUIFenetre('SHEF', this, "Shadow Effect Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_SHADOW, 0xff, 0xff, 0xff), 
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
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
	sprintf(NameStr, "Shadow Editor - %s", Active->GetName());
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

} // ShadowEditGUI::ShadowEditGUI

/*===========================================================================*/

ShadowEditGUI::~ShadowEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ShadowEditGUI::~ShadowEditGUI()

/*===========================================================================*/

long ShadowEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SHG, 0);

return(0);

} // ShadowEditGUI::HandleCloseWin

/*===========================================================================*/

long ShadowEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SHG, 0);
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
	default:
		break;
	} // ButtonID

return(0);

} // ShadowEditGUI::HandleButtonClick

/*===========================================================================*/

long ShadowEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	} // switch CtrlID

return (0);

} // ShadowEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long ShadowEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
		break;
		}
	} // switch

ActivePage = NewPageID;

return(0);

} // ShadowEditGUI::HandlePageChange

/*===========================================================================*/

long ShadowEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKUSEMAPFILE:
	case IDC_CHECKREGENMAPFILE:
	case IDC_CHECKCASTSHADOWS:
	case IDC_CHECKRECEIVESHADOWSTER:
	case IDC_CHECKRECEIVESHADOWSFOL:
	case IDC_CHECKRECEIVESHADOWS3D:
	case IDC_CHECKRECEIVESHADOWSCSM:
	case IDC_CHECKRECEIVESHADOWSVOL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->ShadowBase.SetFloating(EffectsHost->ShadowBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	} // switch CtrlID

return(0);

} // ShadowEditGUI::HandleSCChange

/*===========================================================================*/

long ShadowEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOSHADOWMAPVERYLARGE:
	case IDC_RADIOSHADOWMAPLARGE:
	case IDC_RADIOSHADOWMAPMED:
	case IDC_RADIOSHADOWMAPSMALL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // ShadowEditGUI::HandleSRChange

/*===========================================================================*/

long ShadowEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

//switch (CtrlID)
	{
	//case IDC_RESOLUTION:
	//	{
	//	EffectsHost->ShadowBase.SetFloating(0, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	} // switch CtrlID

return(0);

} // ShadowEditGUI::HandleFIChange

/*===========================================================================*/

void ShadowEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;

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

} // ShadowEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void ShadowEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->ShadowBase.AreThereEdges((GeneralEffect *)EffectsHost->Shadow))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->ShadowBase.AreThereGradients((GeneralEffect *)EffectsHost->Shadow))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->ShadowBase.Resolution,
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

sprintf(TextStr, "Shadow Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->ShadowBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->ShadowBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKUSEMAPFILE, &Active->UseMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREGENMAPFILE, &Active->RegenMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCASTSHADOWS, &Active->CastShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSTER, &Active->ReceiveShadowsTerrain, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSFOL, &Active->ReceiveShadowsFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS3D, &Active->ReceiveShadows3DObject, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSCSM, &Active->ReceiveShadowsCloudSM, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSVOL, &Active->ReceiveShadowsVolumetric, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPLARGE, &Active->ShadowMapWidth, SCFlag_Short, 2048, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPVERYLARGE, &Active->ShadowMapWidth, SCFlag_Short, 4096, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPSMALL, &Active->ShadowMapWidth, SCFlag_Short, 512, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPMED, &Active->ShadowMapWidth, SCFlag_Short, 1024, NULL, NULL);

WidgetSmartRAHConfig(IDC_INTENS, &Active->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY], Active);
WidgetSmartRAHConfig(IDC_MINFOLHT, &Active->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT], Active);
WidgetSmartRAHConfig(IDC_SAFEADD, &Active->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD], Active);
WidgetSmartRAHConfig(IDC_SAFESUB, &Active->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB], Active);
WidgetSmartRAHConfig(IDC_SHADOWOFFSET, &Active->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET], Active);

DisableWidgets();

} // ShadowEditGUI::ConfigureWidgets()

/*===========================================================================*/

void ShadowEditGUI::SyncWidgets(void)
{
/*
char TextStr[32];

if (EffectsHost->ShadowBase.AreThereEdges((GeneralEffect *)EffectsHost->Shadow))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->ShadowBase.AreThereGradients((GeneralEffect *)EffectsHost->Shadow))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_INTENS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINFOLHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SAFEADD, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SAFESUB, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SHADOWOFFSET, WP_FISYNC_NONOTIFY);

//WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKHIRESEDGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKUSEMAPFILE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREGENMAPFILE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCASTSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSTER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSFOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWS3D, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSCSM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSVOL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOSHADOWMAPVERYLARGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPLARGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPMED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPSMALL, WP_SRSYNC_NONOTIFY);

} // ShadowEditGUI::SyncWidgets

/*===========================================================================*/

void ShadowEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOSHADOWMAPVERYLARGE, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPLARGE, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPMED, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPSMALL, ! Active->CastShadows);
WidgetSetDisabled(IDC_CHECKUSEMAPFILE, ! Active->CastShadows);
WidgetSetDisabled(IDC_CHECKREGENMAPFILE, ! (Active->CastShadows && Active->UseMapFile));
WidgetSetDisabled(IDC_SAFEADD, ! Active->CastShadows);
WidgetSetDisabled(IDC_SAFESUB, ! Active->CastShadows);
WidgetSetDisabled(IDC_MINFOLHT, ! Active->CastShadows);
WidgetSetDisabled(IDC_INTENS, ! (Active->ReceiveShadowsTerrain || Active->ReceiveShadowsFoliage || 
	Active->ReceiveShadows3DObject || Active->ReceiveShadowsCloudSM || Active->ReceiveShadowsVolumetric));
WidgetSetDisabled(IDC_SHADOWOFFSET, ! (Active->ReceiveShadowsTerrain || Active->ReceiveShadowsFoliage || 
	Active->ReceiveShadows3DObject || Active->ReceiveShadowsCloudSM));

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);

} // ShadowEditGUI::DisableWidgets

/*===========================================================================*/

void ShadowEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ShadowEditGUI::Cancel

/*===========================================================================*/

void ShadowEditGUI::Name(void)
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

} // ShadowEditGUI::Name()

/*===========================================================================*/

