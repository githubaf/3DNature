// TerraffectorEditGUI.cpp
// Code for Terraffector editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "TerraffectorEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Interactive.h"
#include "ProjectDispatch.h"
#ifdef WCS_DEM_TFX_FREEZE
#include "DEMEval.h"
#endif // WCS_DEM_TFX_FREEZE
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


char *TerraffectorEditGUI::TabNames[WCS_TERRAFFECTORGUI_NUMTABS] = {"General", "Elevation", "Approach Slope"};

long TerraffectorEditGUI::ActivePage;

NativeGUIWin TerraffectorEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TerraffectorEditGUI::Open

/*===========================================================================*/

NativeGUIWin TerraffectorEditGUI::Construct(void)
{
int TabIndex;
GeneralEffect *MyEffect;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_TERRAFFECTOR_GENERAL_VNS, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_TERRAFFECTOR_GENERAL, 0, 0);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_TERRAFFECTOR_BASIC, 0, 1);
	CreateSubWinFromTemplate(IDD_TERRAFFECTOR_SLOPE, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_TERRAFFECTORGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_ECODROP, -1, "New Ecosystem...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_ECODROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_ECODROP, TabIndex, MyEffect);
			} // for
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // TerraffectorEditGUI::Construct

/*===========================================================================*/

TerraffectorEditGUI::TerraffectorEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraffectorEffect *ActiveSource)
: GUIFenetre('TERF', this, "Terraffector Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
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
	sprintf(NameStr, "Terraffector Editor - %s", Active->GetName());
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

} // TerraffectorEditGUI::TerraffectorEditGUI

/*===========================================================================*/

TerraffectorEditGUI::~TerraffectorEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // TerraffectorEditGUI::~TerraffectorEditGUI()

/*===========================================================================*/

long TerraffectorEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TAG, 0);

return(0);

} // TerraffectorEditGUI::HandleCloseWin

/*===========================================================================*/

long TerraffectorEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TAG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_FREEECO:
		{
		FreeEcosystem();
		break;
		} // IDC_FREEECO
	case IDC_EDITECO:
		{
		if (Active->SlopeEco)
			Active->SlopeEco->EditRAHost();
		break;
		} // IDC_EDITECO
	case IDC_EDITPROFILE:
	case IDC_EDITPROFILE2:
		{
		Active->ADSection.OpenTimeline();
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
	default:
		break;
	} // ButtonID

return(0);

} // TerraffectorEditGUI::HandleButtonClick

/*===========================================================================*/

long TerraffectorEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ECODROP:
		{
		SelectNewEco();
		break;
		} // IDC_ECODROP
	default:
		break;
	} // switch CtrlID

return (0);

} // TerraffectorEditGUI::HandleCBChange

/*===========================================================================*/

long TerraffectorEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // TerraffectorEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long TerraffectorEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				} // 1
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

} // TerraffectorEditGUI::HandlePageChange

/*===========================================================================*/

long TerraffectorEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKSPLINE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->TerraffectorBase.SetFloating(EffectsHost->TerraffectorBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // TerraffectorEditGUI::HandleSCChange

/*===========================================================================*/

long TerraffectorEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOMODTERRAIN:
	case IDC_RADIOUNMODTERRAIN:
	case IDC_RADIOINCORDEC:
	case IDC_RADIOINCONLY:
	case IDC_RADIODECONLY:
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

} // TerraffectorEditGUI::HandleSRChange

/*===========================================================================*/

long TerraffectorEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	//case IDC_RESOLUTION:
	//	{
	//	EffectsHost->TerraffectorBase.SetFloating(0, ProjHost);		// this sends the valuechanged message
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

} // TerraffectorEditGUI::HandleFIChange

/*===========================================================================*/

void TerraffectorEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Changed, Interested[7];
long Pos, CurPos, Done = 0;
GeneralEffect *MyEffect, *MatchEffect;

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

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = Active->SlopeEco;
	WidgetCBClear(IDC_ECODROP);
	WidgetCBInsert(IDC_ECODROP, -1, "New Ecosystem...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_ECODROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_ECODROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_ECODROP, CurPos);
	Done = 1;
	} // if eco name changed

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

} // TerraffectorEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void TerraffectorEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
long ListPos, Ct, NumEntries;
EcosystemEffect *TestObj;

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->TerraffectorBase.Resolution,
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

sprintf(TextStr, "Terraffector Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->TerraffectorBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->TerraffectorBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSPLINE, &Active->Splined, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIOABSOLUTE, &Active->Absolute, SCFlag_Short, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEGRND, &Active->Absolute, SCFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEJOE, &Active->Absolute, SCFlag_Short, 2, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOMODTERRAIN, IDC_RADIOMODTERRAIN, &Active->ApplyToOrigElev, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOMODTERRAIN, IDC_RADIOUNMODTERRAIN, &Active->ApplyToOrigElev, SRFlag_Char, 1, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIOINCORDEC, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIOINCONLY, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOINCORDEC, IDC_RADIODECONLY, &Active->CompareType, SRFlag_Char, WCS_EFFECTS_RASTERTA_COMPARETYPE_DECREASE, NULL, NULL);

//WidgetSNConfig(IDC_SLOPEROUGH, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS]);
//WidgetSNConfig(IDC_SLOPEECOMIX, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING]);
//WidgetSNConfig(IDC_MAXSLOPE, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE]);
//WidgetSNConfig(IDC_MINSLOPE, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE]);
//WidgetSNConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY]);
WidgetSNConfig(IDC_RADIUS, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS]);
WidgetSmartRAHConfig(IDC_SLOPEROUGH, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS], Active);
WidgetSmartRAHConfig(IDC_SLOPEECOMIX, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING], Active);
WidgetSmartRAHConfig(IDC_MAXSLOPE, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE], Active);
WidgetSmartRAHConfig(IDC_MINSLOPE, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE], Active);
WidgetSmartRAHConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY], Active);

ConfigureFI(NativeWin, IDC_SLOPEPRIOR,
 &Active->SlopePriority,
  1.0,
   -100.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);

if (Active->SlopeEco)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_ECODROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (EcosystemEffect *)WidgetCBGetItemData(IDC_ECODROP, Ct)) != (EcosystemEffect *)LB_ERR && TestObj == Active->SlopeEco)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_ECODROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_ECODROP, -1);

DisableWidgets();

} // TerraffectorEditGUI::ConfigureWidgets()

/*===========================================================================*/

void TerraffectorEditGUI::SyncWidgets(void)
{

//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_EVALORDER, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_SLOPEPRIOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SLOPEROUGH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SLOPEECOMIX, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXSLOPE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINSLOPE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RADIUS, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSPLINE, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOABSOLUTE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEGRND, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEJOE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOMODTERRAIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOUNMODTERRAIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCORDEC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCONLY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIODECONLY, WP_SRSYNC_NONOTIFY);

} // TerraffectorEditGUI::SyncWidgets

/*===========================================================================*/

void TerraffectorEditGUI::DisableWidgets(void)
{

// need vectors - may be dynamic linked
//WidgetSetDisabled(IDC_PRIORITY, ! Active->Joes);

} // TerraffectorEditGUI::DisableWidgets

/*===========================================================================*/

void TerraffectorEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // TerraffectorEditGUI::Cancel

/*===========================================================================*/

void TerraffectorEditGUI::Name(void)
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

} // TerraffectorEditGUI::Name()

/*===========================================================================*/

void TerraffectorEditGUI::SelectNewEco(void)
{
EcosystemEffect *NewEco;
long Current;

Current = WidgetCBGetCurSel(IDC_ECODROP);
if (((NewEco = (EcosystemEffect *)WidgetCBGetItemData(IDC_ECODROP, Current, 0)) != (EcosystemEffect *)LB_ERR && NewEco)
	|| (NewEco = (EcosystemEffect *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ECOSYSTEM, NULL, NULL)))
	{
	Active->SetSlopeEco(NewEco);
	} // if

} // TerraffectorEditGUI::SelectNewEco

/*===========================================================================*/

void TerraffectorEditGUI::FreeEcosystem(void)
{

Active->RemoveRAHost(Active->SlopeEco);

} // TerraffectorEditGUI::FreeEcosystem

/*===========================================================================*/

