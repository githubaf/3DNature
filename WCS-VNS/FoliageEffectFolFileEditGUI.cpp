// FoliageEffectFolFileEditGUI.cpp
// Code for FoliageEffect editor when Foliage Effect can only render from a Foliage File of data instances.
// Built from FoliageEffectEditGUI on 8/24/05 by Gary R. Huber
// Copyright 2005 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "FoliageEffectFolFileEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "ProjectDispatch.h"
#include "EffectsLib.h"
#include "resource.h"

char *FoliageEffectFolFileEditGUI::TabNames[WCS_FOLIAGEEFFECTFOLFILEGUI_NUMTABS] = {"General", "Dissolve"};
long FoliageEffectFolFileEditGUI::ActivePage;

bool FoliageEffectFolFileEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch(AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANNEXT:
	case WCS_FENETRE_WINCAP_CANPREV:
	case WCS_FENETRE_WINCAP_CANUNDO:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // FoliageEffectFolFileEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin FoliageEffectFolFileEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // FoliageEffectFolFileEditGUI::Open

/*===========================================================================*/

NativeGUIWin FoliageEffectFolFileEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_GENERAL_FOLFILE, 0, 0);
	CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_DISSOLVE_FOLFILE, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_FOLIAGEEFFECTFOLFILEGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		SelectPanel(ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // FoliageEffectFolFileEditGUI::Construct

/*===========================================================================*/

FoliageEffectFolFileEditGUI::FoliageEffectFolFileEditGUI(EffectsLib *EffectsSource, FoliageEffect *ActiveSource)
: GUIFenetre('EDFO', this, "Foliage Effect Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {0,
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;

if (EffectsHost && ActiveSource)
	{
	AllEvents[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, 0xff);
	sprintf(NameStr, "FoliageEffect Editor - %s ", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
	ConstructError = 1;

} // FoliageEffectFolFileEditGUI::FoliageEffectFolFileEditGUI

/*===========================================================================*/

FoliageEffectFolFileEditGUI::~FoliageEffectFolFileEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // FoliageEffectFolFileEditGUI::~FoliageEffectFolFileEditGUI()

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_FLG, 0);

return(0);

} // FoliageEffectFolFileEditGUI::HandleCloseWin

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_FLG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_PREV:
		{
		//EffectsHost->EditNext(0, Active, WCS_EFFECTSSUBCLASS_FOLIAGE);
		ULONG NotifyChanges[2] = {MAKE_ID(WCS_NOTIFYCLASS_DELAYEDEDIT, WCS_NOTIFYSUBCLASS_REVERSE, 0, 0), 0};
		GlobalApp->AppEx->GenerateDelayedNotify(NotifyChanges, Active);
		return (1);
		} //
	case IDC_NEXT:
		{
		//EffectsHost->EditNext(1, Active, WCS_EFFECTSSUBCLASS_FOLIAGE);
		ULONG NotifyChanges[2] = {MAKE_ID(WCS_NOTIFYCLASS_DELAYEDEDIT, 0, 0, 0), 0};
		GlobalApp->AppEx->GenerateDelayedNotify(NotifyChanges, Active);
		return (1);
		} //
	default:
		break;
	} // ButtonID

return(0);

} // FoliageEffectFolFileEditGUI::HandleButtonClick

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // FoliageEffectFolFileEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				SelectPanel(1);
				break;
				} // 1
			default:
				{
				SelectPanel(0);
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

} // FoliageEffectFolFileEditGUI::HandlePageChange

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
		} // IDC_CHECKENABLED
	case IDC_CHECKPREVIEWENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_CHECKPREVIEWENABLED
	case IDC_CHECKDISSOLVEENABLED:
	case IDC_CHECKRENDEROCCLUDED:
		{
		Changes[0] = MAKE_ID(Active->Ecotp.GetNotifyClass(), Active->Ecotp.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->Ecotp.GetRAHostRoot());
		break;
		} // IDC_CHECKDISSOLVEENABLED
	default:
		break;
	} // switch CtrlID

return(0);

} // FoliageEffectFolFileEditGUI::HandleSCChange

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOBYPIXEL:
	case IDC_RADIOBYIMAGEHT:
		{
		if (! UserMessageOKCAN("Common Distance Dissolve", "Warning! This change will affect how all Foliage Effects and all Ecotype foliage is rendered."))
			{
			GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize = 1 - GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize;
			} // if
		Changes[0] = MAKE_ID(Active->Ecotp.GetNotifyClass(), Active->Ecotp.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->Ecotp.GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // FoliageEffectFolFileEditGUI::HandleSRChange

/*===========================================================================*/

long FoliageEffectFolFileEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_DISSOLVEREFHT:
		{
		Changes[0] = MAKE_ID(Active->Ecotp.GetNotifyClass(), Active->Ecotp.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->Ecotp.GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // FoliageEffectFolFileEditGUI::HandleFIChange

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::HandleNotifyEvent(void)
{

if (! NativeWin)
	return;

ConfigureWidgets();

} // FoliageEffectFolFileEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Foliage Effect Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSR(NativeWin, IDC_RADIOBYPIXEL, IDC_RADIOBYPIXEL, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOBYPIXEL, IDC_RADIOBYIMAGEHT, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 1, NULL, NULL);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPREVIEWENABLED, &Active->PreviewEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDISSOLVEENABLED, &Active->Ecotp.DissolveEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRENDEROCCLUDED, &Active->Ecotp.RenderOccluded, SCFlag_Char, NULL, 0);

ConfigureFI(NativeWin, IDC_DISSOLVEHEIGHT,
 &Active->Ecotp.DissolvePixelHeight,
  .5,
   0.0,
	1000.0,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_DISSOLVEREFHT,
 &GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt,
  1.0,
   1.0,
	32767.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureDD(NativeWin, IDC_FOLIAGE_FILE, Active->FoliageFile.Path, WCS_PATHANDFILE_PATH_LEN_MINUSONE, Active->FoliageFile.Name, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);

DisableWidgets();

} // FoliageEffectFolFileEditGUI::ConfigureWidgets()

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPREVIEWENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDISSOLVEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRENDEROCCLUDED, WP_SCSYNC_NONOTIFY);

WidgetFISync(IDC_DISSOLVEHEIGHT, WP_FISYNC_NONOTIFY);

} // FoliageEffectFolFileEditGUI::SyncWidgets

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_DISSOLVEHEIGHT, ! Active->Ecotp.DissolveEnabled);
WidgetSetDisabled(IDC_DISSOLVEREFHT, ! GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize);

} // FoliageEffectFolFileEditGUI::DisableWidgets

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::SelectPanel(long PanelID)
{

switch(PanelID)
	{
	case 1:
		{
		ShowPanel(0, 1);
		break;
		} // 1
	default:
		{
		ShowPanel(0, 0);
		break;
		} // None
	} // PanelID

} // FoliageEffectFolFileEditGUI::SelectPanel

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // FoliageEffectFolFileEditGUI::Cancel

/*===========================================================================*/

void FoliageEffectFolFileEditGUI::Name(void)
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

} // FoliageEffectFolFileEditGUI::Name()

/*===========================================================================*/
