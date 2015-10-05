// TerrainParamEditGUI.cpp
// Code for TerrainParam editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "TerrainParamEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Interactive.h"
#include "ProjectDispatch.h"
#include "resource.h"
#include "FeatureConfig.h"

char *TerrainParamEditGUI::TabNames[WCS_TERRAINPARAMGUI_NUMTABS] = {"General"};

long TerrainParamEditGUI::ActivePage;

NativeGUIWin TerrainParamEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TerrainParamEditGUI::Open

/*===========================================================================*/

NativeGUIWin TerrainParamEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_VECPOLY_EFFECTS
	CreateSubWinFromTemplate(IDD_TERRAINPARAM_COMMON_VNS3, 0, 0);
	#else // WCS_VECPOLY_EFFECTS
	CreateSubWinFromTemplate(IDD_TERRAINPARAM_COMMON, 0, 0);
	#endif // WCS_VECPOLY_EFFECTS

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_TERRAINPARAMGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // TerrainParamEditGUI::Construct

/*===========================================================================*/

TerrainParamEditGUI::TerrainParamEditGUI(EffectsLib *EffectsSource, Database *DBSource, TerrainParamEffect *ActiveSource)
: GUIFenetre('TERP', this, "Terrain Parameter Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Terrain Parameter Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	#ifndef WCS_VECPOLY_EFFECTS
	WidgetShow(IDC_CHECKPHONG, false);
	#endif // WCS_VECPOLY_EFFECTS
	} // if
else
	ConstructError = 1;

} // TerrainParamEditGUI::TerrainParamEditGUI

/*===========================================================================*/

TerrainParamEditGUI::~TerrainParamEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // TerrainParamEditGUI::~TerrainParamEditGUI()

/*===========================================================================*/

long TerrainParamEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TPG, 0);

return(0);

} // TerrainParamEditGUI::HandleCloseWin

/*===========================================================================*/

long TerrainParamEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_CREATEFMAPS:
		{
		EffectsHost->TerrainParamBase.CreateFractalMaps(TRUE);	// true means report optimum fractal depth to user
		break;
		} // IDC_CREATEFMAPS
	default: break;
	} // ButtonID
return(0);

} // TerrainParamEditGUI::HandleButtonClick

/*===========================================================================*/

long TerrainParamEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // TerrainParamEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long TerrainParamEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 0:
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

} // TerrainParamEditGUI::HandlePageChange

/*===========================================================================*/

long TerrainParamEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKBFCULL:
	#ifndef WCS_VECPOLY_EFFECTS
	case IDC_CHECKHORIZ:
	#endif // WCS_VECPOLY_EFFECTS
	case IDC_CHECKREGENFDM:
	case IDC_CHECKPHONG:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // TerrainParamEditGUI::HandleSCChange

/*===========================================================================*/

long TerrainParamEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOFDVAR:
	case IDC_RADIOFDCONST:
	case IDC_RADIOFDDEPTH:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // TerrainParamEditGUI::HandleSRChange

/*===========================================================================*/

void TerrainParamEditGUI::HandleNotifyEvent(void)
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
	ConfigureWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // TerrainParamEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void TerrainParamEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Terrain Parameter Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

#ifdef WCS_VECPOLY_EFFECTS
WidgetSetText(IDC_DISPLACEMENT, "Vertical Displacement (m) ");
#endif // WCS_VECPOLY_EFFECTS

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPHONG, &Active->PhongShading, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOFDVAR, IDC_RADIOFDVAR, &EffectsHost->TerrainParamBase.FractalMethod, SRFlag_Char, WCS_FRACTALMETHOD_VARIABLE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFDVAR, IDC_RADIOFDCONST, &EffectsHost->TerrainParamBase.FractalMethod, SRFlag_Char, WCS_FRACTALMETHOD_CONSTANT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFDVAR, IDC_RADIOFDDEPTH, &EffectsHost->TerrainParamBase.FractalMethod, SRFlag_Char, WCS_FRACTALMETHOD_DEPTHMAPS, NULL, NULL);

ConfigureSC(NativeWin, IDC_CHECKBFCULL, &EffectsHost->TerrainParamBase.BackfaceCull, SCFlag_Char, NULL, 0);
#ifndef WCS_VECPOLY_EFFECTS
ConfigureSC(NativeWin, IDC_CHECKHORIZ, &EffectsHost->TerrainParamBase.HorFractDisplace, SCFlag_Char, NULL, 0);
#endif // WCS_VECPOLY_EFFECTS
ConfigureSC(NativeWin, IDC_CHECKREGENFDM, &EffectsHost->TerrainParamBase.RegenFDMsEachRender, SCFlag_Char, NULL, 0);

ConfigureFI(NativeWin, IDC_DEPTHMAPPIXSIZE,
 &EffectsHost->TerrainParamBase.DepthMapPixSize,
  .25,
   .25,
	10.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_FRACTALDEPTH,
 &Active->FractalDepth,
  1.0,
   0.0,
	7.0,
	 FIOFlag_Char,
	  NULL,
	   0);

WidgetSmartRAHConfig(IDC_FRDTEX, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH), Active);
WidgetSmartRAHConfig(IDC_DISPLACEMENT, &Active->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT], Active);
#ifndef WCS_VECPOLY_EFFECTS
WidgetSmartRAHConfig(IDC_SLOPEFACTOR, &Active->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR], Active);
#endif // WCS_VECPOLY_EFFECTS

DisableWidgets();

} // TerrainParamEditGUI::ConfigureWidgets()

/*===========================================================================*/

void TerrainParamEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_DEPTHMAPPIXSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FRACTALDEPTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DISPLACEMENT, WP_FISYNC_NONOTIFY);
#ifndef WCS_VECPOLY_EFFECTS
WidgetSNSync(IDC_SLOPEFACTOR, WP_FISYNC_NONOTIFY);
#endif // WCS_VECPOLY_EFFECTS
WidgetSNSync(IDC_FRDTEX, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKBFCULL, WP_SCSYNC_NONOTIFY);
#ifndef WCS_VECPOLY_EFFECTS
WidgetSCSync(IDC_CHECKHORIZ, WP_SCSYNC_NONOTIFY);
#endif // WCS_VECPOLY_EFFECTS
WidgetSCSync(IDC_CHECKREGENFDM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPHONG, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOFDVAR, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFDCONST, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFDDEPTH, WP_SRSYNC_NONOTIFY);

} // TerrainParamEditGUI::SyncWidgets

/*===========================================================================*/

void TerrainParamEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_DEPTHMAPPIXSIZE, EffectsHost->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_CONSTANT);
WidgetSetDisabled(IDC_CREATEFMAPS, EffectsHost->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_CONSTANT);
WidgetSetDisabled(IDC_DEPTHMAPLABEL, EffectsHost->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_CONSTANT);
WidgetSetDisabled(IDC_CHECKREGENFDM, EffectsHost->TerrainParamBase.FractalMethod != WCS_FRACTALMETHOD_DEPTHMAPS);

} // TerrainParamEditGUI::DisableWidgets

/*===========================================================================*/

void TerrainParamEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // TerrainParamEditGUI::Cancel

/*===========================================================================*/

void TerrainParamEditGUI::Name(void)
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

} // TerrainParamEditGUI::Name()

/*===========================================================================*/
