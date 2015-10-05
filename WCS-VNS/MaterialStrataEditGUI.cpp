// MaterialStrataEditGUI.cpp
// Code for MaterialStrata editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "MaterialStrataEditGUI.h"
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

bool MaterialStrataEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch(AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANLOAD:
	case WCS_FENETRE_WINCAP_CANSAVE:
	case WCS_FENETRE_WINCAP_CANUNDO:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // MaterialStrataEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin MaterialStrataEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // MaterialStrataEditGUI::Open

/*===========================================================================*/

NativeGUIWin MaterialStrataEditGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_MATERIALSTRATA_EDIT, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_MATERIALSTRATA_GENERAL, 0, 0, false);

	if(NativeWin)
		{
		ShowPanel(0, 0);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // MaterialStrataEditGUI::Construct

/*===========================================================================*/

MaterialStrataEditGUI::MaterialStrataEditGUI(EffectsLib *EffectsSource, MaterialStrata *ActiveSource)
: GUIFenetre('MATS', this, "Material Strata Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {0, 0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;

AllEvents[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, 0xff);

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Material Strata Editor - %s", Active->GetRAHostRoot()->GetRAHostName());
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // MaterialStrataEditGUI::MaterialStrataEditGUI

/*===========================================================================*/

MaterialStrataEditGUI::~MaterialStrataEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // MaterialStrataEditGUI::~MaterialStrataEditGUI()

/*===========================================================================*/

long MaterialStrataEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_MSG, 0);

return(0);

} // MaterialStrataEditGUI::HandleCloseWin

/*===========================================================================*/

long MaterialStrataEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_MSG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_GALLERY:
		{
		Active->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_LOAD:
		{
		Active->LoadComponentFile(NULL);
		break;
		} //
	case IDC_SAVE:
		{
		Active->OpenBrowseData(EffectsHost);
		break;
		} //
	default:
		break;
	} // ButtonID

return(0);

} // MaterialStrataEditGUI::HandleButtonClick

/*===========================================================================*/

long MaterialStrataEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKCOLORSTRATA:
	case IDC_CHECKLINESENABLED:
	case IDC_CHECKBUMPLINES:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // MaterialStrataEditGUI::HandleSCChange

/*===========================================================================*/

void MaterialStrataEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[6] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // MaterialStrataEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void MaterialStrataEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Material Strata Editor - %s", Active->GetRAHostRoot()->GetRAHostName());
SetTitle(TextStr);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKLINESENABLED, &Active->LinesEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCOLORSTRATA, &Active->ColorStrata, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKBUMPLINES, &Active->BumpLines, SCFlag_Char, NULL, 0);

//WidgetSNConfig(IDC_DEFORMSCALE, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE]);
//WidgetSNConfig(IDC_NORTHDIP, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP]);
//WidgetSNConfig(IDC_WESTDIP, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_WESTDIP]);
//WidgetSNConfig(IDC_BUMPINTENSITY, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY]);
WidgetSmartRAHConfig(IDC_DEFORMSCALE, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE], Active);
WidgetSmartRAHConfig(IDC_NORTHDIP, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP], Active);
WidgetSmartRAHConfig(IDC_WESTDIP, &Active->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT], Active);
WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Active->AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY], Active);
WidgetSmartRAHConfig(IDC_COLOR1, &Active->StrataColor[0], Active);
WidgetSmartRAHConfig(IDC_COLOR2, &Active->StrataColor[1], Active);
WidgetSmartRAHConfig(IDC_COLOR3, &Active->StrataColor[2], Active);
WidgetSmartRAHConfig(IDC_COLOR4, &Active->StrataColor[3], Active);

DisableWidgets();

} // MaterialStrataEditGUI::ConfigureWidgets()

/*===========================================================================*/

void MaterialStrataEditGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_DEFORMSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTHDIP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WESTDIP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR1, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR4, WP_FISYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCOLORSTRATA, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLINESENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);

} // MaterialStrataEditGUI::DisableWidgets

/*===========================================================================*/

void MaterialStrataEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_BUMPINTENSITY, ! Active->BumpLines);
WidgetSetDisabled(IDC_BANDTXT, ! Active->ColorStrata);
WidgetSetDisabled(IDC_COLOR1, ! Active->ColorStrata);
WidgetSetDisabled(IDC_COLOR2, ! Active->ColorStrata);
WidgetSetDisabled(IDC_COLOR3, ! Active->ColorStrata);
WidgetSetDisabled(IDC_COLOR4, ! Active->ColorStrata);

} // MaterialStrataEditGUI::DisableWidgets

/*===========================================================================*/

void MaterialStrataEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // MaterialStrataEditGUI::Cancel

/*===========================================================================*/
