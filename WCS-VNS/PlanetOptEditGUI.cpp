// PlanetOptEditGUI.cpp
// Code for PlanetOpt editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "PlanetOptEditGUI.h"
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

char *PlanetOptEditGUI::TabNames[WCS_PLANETOPTGUI_NUMTABS] = {"General"};

long PlanetOptEditGUI::ActivePage;

NativeGUIWin PlanetOptEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // PlanetOptEditGUI::Open

/*===========================================================================*/

NativeGUIWin PlanetOptEditGUI::Construct(void)
{
#ifdef WCS_BUILD_VNS
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_PLANETOPT_GENERAL_VNS, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_PLANETOPT_GENERAL, 0, 0);
	#endif // WCS_BUILD_VNS

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_PLANETOPTGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, TabIndex, MyEffect);
			} // for
		#else // WCS_BUILD_VNS
		for (TabIndex = 0; TabIndex < WCS_PRESETS_NUMCELESTIALS; TabIndex ++)
			{
			WidgetCBInsert(IDC_RADIUSDROP, -1, EffectsLib::CelestialPresetName[TabIndex]);
			} // for
		#endif // WCS_BUILD_VNS
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // PlanetOptEditGUI::Construct

/*===========================================================================*/

PlanetOptEditGUI::PlanetOptEditGUI(EffectsLib *EffectsSource, PlanetOpt *ActiveSource)
: GUIFenetre('PLAN', this, "Planet Options Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_PLANETOPT, 0xff, 0xff, 0xff), 
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Planet Options Editor - %s", Active->GetName());
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

} // PlanetOptEditGUI::PlanetOptEditGUI

/*===========================================================================*/

PlanetOptEditGUI::~PlanetOptEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // PlanetOptEditGUI::~PlanetOptEditGUI()

/*===========================================================================*/

long PlanetOptEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_POG, 0);

return(0);

} // PlanetOptEditGUI::HandleCloseWin

/*===========================================================================*/

long PlanetOptEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_POG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (Active->Coords)
			Active->Coords->EditRAHost();
		break;
		} // IDC_EDITCOORDS
	default:
		break;
	} // ButtonID

return(0);

} // PlanetOptEditGUI::HandleButtonClick

/*===========================================================================*/

long PlanetOptEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		}
	case IDC_RADIUSDROP:
		{
		#ifndef WCS_BUILD_VNS
		RadiusPreset();
		#endif // WCS_BUILD_VNS
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PlanetOptEditGUI::HandleCBChange

/*===========================================================================*/

long PlanetOptEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // PlanetOptEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long PlanetOptEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // PlanetOptEditGUI::HandlePageChange

/*===========================================================================*/

long PlanetOptEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
short Enabled;

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		// smart check already changed enabled status so need to reverse the change first
		Enabled = Active->Enabled;
		Active->Enabled = 1 - Active->Enabled;
		// let effects lib make the change - it takes care of notification and only allows one enabled planet opt
		EffectsHost->SetPlanetOptEnabled(Active, Enabled);
		break;
		} // 
	case IDC_CHECKEXAGECO:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PlanetOptEditGUI::HandleSCChange

/*===========================================================================*/

void PlanetOptEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;
#ifdef WCS_BUILD_VNS
long Pos, CurPos;
GeneralEffect *MyEffect, *MatchEffect;
#endif // WCS_BUILD_VNS

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

#ifdef WCS_BUILD_VNS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPos = -1;
	MatchEffect = Active->Coords;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		Done = 1;
		} // if
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // PlanetOptEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void PlanetOptEditGUI::ConfigureWidgets(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *TestObj;
long ListPos, NumEntries;
#endif // WCS_BUILD_VNS
long Ct;
char TextStr[256];

sprintf(TextStr, "Planet Options Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKEXAGECO, &Active->EcoExageration, SCFlag_Char, NULL, 0);

#ifndef WCS_BUILD_VNS
//WidgetSNConfig(IDC_PLANET_RADIUS, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS]);
WidgetSmartRAHConfig(IDC_PLANET_RADIUS, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS], Active);
#endif // WCS_BUILD_VNS
//WidgetSNConfig(IDC_PLANET_ROTATION, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION]);
//WidgetSNConfig(IDC_PLANET_VERTICALEXAG, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG]);
//WidgetSNConfig(IDC_PLANET_DATUM, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM]);
WidgetSmartRAHConfig(IDC_PLANET_ROTATION, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION], Active);
WidgetSmartRAHConfig(IDC_PLANET_VERTICALEXAG, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG], Active);
WidgetSmartRAHConfig(IDC_PLANET_DATUM, &Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM], Active);

#ifdef WCS_BUILD_VNS
if (Active->Coords)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestObj == Active->Coords)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
#else // WCS_BUILD_VNS
for (Ct = 0; Ct < WCS_PRESETS_NUMCELESTIALS; Ct ++)
	{
	if (Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].CurValue == EffectsLib::CelestialPresetRadius[Ct])
		WidgetCBSetCurSel(IDC_RADIUSDROP, Ct);
	} // for
#endif // WCS_BUILD_VNS

} // PlanetOptEditGUI::ConfigureWidgets()

/*===========================================================================*/

void PlanetOptEditGUI::SyncWidgets(void)
{

#ifndef WCS_BUILD_VNS
WidgetSNSync(IDC_PLANET_RADIUS, WP_FISYNC_NONOTIFY);
#endif // WCS_BUILD_VNS
WidgetSNSync(IDC_PLANET_ROTATION, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PLANET_VERTICALEXAG, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PLANET_DATUM, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);

} // PlanetOptEditGUI::SyncWidgets

/*===========================================================================*/

void PlanetOptEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // PlanetOptEditGUI::Cancel

/*===========================================================================*/

void PlanetOptEditGUI::Name(void)
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

} // PlanetOptEditGUI::Name()

/*===========================================================================*/

void PlanetOptEditGUI::RadiusPreset(void)
{
#ifndef WCS_BUILD_VNS
long Current;

if ((Current = WidgetCBGetCurSel(IDC_RADIUSDROP)) != CB_ERR && Current < WCS_PRESETS_NUMCELESTIALS)
	{
	Active->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].SetCurValue(EffectsLib::CelestialPresetRadius[Current]);
	} // if
#endif // WCS_BUILD_VNS

} // PlanetOptEditGUI::RadiusPreset

/*===========================================================================*/

void PlanetOptEditGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Active->SetCoords(NewObj);
	} // if
#endif // WCS_BUILD_VNS

} // PlanetOptEditGUI::SelectNewCoords

/*===========================================================================*/
