// CelestialEditGUI.cpp
// Code for Celestial editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CelestialEditGUI.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Raster.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"

char *CelestialEditGUI::TabNames[WCS_CELESTIALGUI_NUMTABS] = {"General", "Size && Position"};

long CelestialEditGUI::ActivePage;

NativeGUIWin CelestialEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CelestialEditGUI::Open

/*===========================================================================*/

NativeGUIWin CelestialEditGUI::Construct(void)
{
Raster *MyRast;
int TabIndex, Pos;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_CELESTIAL_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_CELESTIAL_POSITION, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_CELESTIALGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
		for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
			{
			Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
			WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
			} // for
		for (TabIndex = 0; TabIndex < WCS_PRESETS_NUMCELESTIALS; TabIndex ++)
			{
			WidgetCBInsert(IDC_ELEVATIONDROP, -1, EffectsLib::CelestialPresetName[TabIndex]);
			WidgetCBInsert(IDC_RADIUSDROP, -1, EffectsLib::CelestialPresetName[TabIndex]);
			} // for
		WidgetCBSetCurSel(IDC_ELEVATIONDROP, -1);
		WidgetCBSetCurSel(IDC_RADIUSDROP, -1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // CelestialEditGUI::Construct

/*===========================================================================*/

CelestialEditGUI::CelestialEditGUI(EffectsLib *EffectsSource, CelestialEffect *ActiveSource)
: GUIFenetre('CELP', this, "Celestial Object Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_CELESTIAL, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Celestial Object Editor - %s", Active->GetName());
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

} // CelestialEditGUI::CelestialEditGUI

/*===========================================================================*/

CelestialEditGUI::~CelestialEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // CelestialEditGUI::~CelestialEditGUI()

/*===========================================================================*/

long CelestialEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_LPG, 0);

return(0);

} // CelestialEditGUI::HandleCloseWin

/*===========================================================================*/

long CelestialEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_LPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDLIGHT:
		{
		AddLight();
		break;
		} // IDC_ADDLIGHT
	case IDC_REMOVELIGHT:
		{
		RemoveLight();
		break;
		} // IDC_REMOVELIGHT
	default:
		break;
	} // ButtonID

return(0);

} // CelestialEditGUI::HandleButtonClick

/*===========================================================================*/

long CelestialEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL:
		{
		EditImage();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // CelestialEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long CelestialEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_IMAGEDROP:
		{
		SelectNewImage();
		break;
		}
	case IDC_ELEVATIONDROP:
		{
		ElevationPreset();
		break;
		}
	case IDC_RADIUSDROP:
		{
		RadiusPreset();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // CelestialEditGUI::HandleCBChange

/*===========================================================================*/

long CelestialEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_LIGHTLIST:
		{
		RemoveLight();
		break;
		} // IDC_LIGHTLIST
	default:
		break;
	} // switch

return(0);

} // CelestialEditGUI::HandleListDelItem

/*===========================================================================*/

long CelestialEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_LIGHTLIST:
		{
		EditLight();
		break;
		} // IDC_LIGHTLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // CelestialEditGUI::HandleListDoubleClick

/*===========================================================================*/

long CelestialEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // CelestialEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long CelestialEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // CelestialEditGUI::HandlePageChange

/*===========================================================================*/

long CelestialEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKPHASE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CelestialEditGUI::HandleSCChange

/*===========================================================================*/

void CelestialEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[8];
long Pos, CurPos, Done = 0;
Raster *MyRast, *MatchRast;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[6] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[7] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppImages->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchRast = (Active->Img) ? Active->Img->GetRaster(): NULL;
	WidgetCBClear(IDC_IMAGEDROP);
	WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
	for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
		{
		Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
		WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
		if (MyRast == MatchRast)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_IMAGEDROP, CurPos);
	Done = 1;
	} // if image name changed
else
	{
	Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (Changed = GlobalApp->AppImages->MatchNotifyClass(Interested, Changes, 0))
		{
		MatchRast = (Active->Img) ? Active->Img->GetRaster(): NULL;
		DisableWidgets();
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MatchRast);
		Done = 1;
		} // else
	} // else

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildLightList();
	Done = 1;
	} // if image name changed

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // CelestialEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void CelestialEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
Raster *MyRast, *TestRast;
long ListPos, Ct, NumEntries;

sprintf(TextStr, "Celestial Object Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);

//WidgetSNConfig(IDC_TRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY]);
//WidgetSNConfig(IDC_RADIUS, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS]);
//WidgetSNConfig(IDC_SIZEFACTOR, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR]);
//WidgetSNConfig(IDC_LAT, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE]);
//WidgetSNConfig(IDC_LON, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE]);
//WidgetSNConfig(IDC_DISTANCE, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE]);
WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY], Active);
WidgetSmartRAHConfig(IDC_RADIUS, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS], Active);
WidgetSmartRAHConfig(IDC_SIZEFACTOR, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR], Active);
WidgetSmartRAHConfig(IDC_LAT, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE], Active);
WidgetSmartRAHConfig(IDC_LON, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE], Active);
WidgetSmartRAHConfig(IDC_DISTANCE, &Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE], Active);
WidgetSmartRAHConfig(IDC_REPLACEGRAY, &Active->Color, Active);

ConfigureSC(NativeWin, IDC_CHECKPHASE, &Active->ShowPhase, SCFlag_Short, NULL, 0);

ConfigureTB(NativeWin, IDC_ADDLIGHT, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVELIGHT, IDI_DELETE, NULL);

if (Active->Img && (MyRast = Active->Img->GetRaster()))
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_IMAGEDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Ct)) != (Raster *)LB_ERR && TestRast == MyRast)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_IMAGEDROP, ListPos);
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MyRast);
	} // if
else
	{
	WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
	} // else

for (Ct = 0; Ct < WCS_PRESETS_NUMCELESTIALS; Ct ++)
	{
	if (Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].CurValue == EffectsLib::CelestialPresetDistance[Ct])
		WidgetCBSetCurSel(IDC_ELEVATIONDROP, Ct);
	if (Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].CurValue == EffectsLib::CelestialPresetRadius[Ct])
		WidgetCBSetCurSel(IDC_RADIUSDROP, Ct);
	} // for

BuildLightList();
DisableWidgets();

} // CelestialEditGUI::ConfigureWidgets()

/*===========================================================================*/

void CelestialEditGUI::BuildLightList(void)
{
EffectList *Current = Active->Lights;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_LIGHTLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_LIGHTLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (GeneralEffect **)AppMem_Alloc(NumSelected * sizeof (GeneralEffect *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_LIGHTLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_LIGHTLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_LIGHTLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildLightListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_LIGHTLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_LIGHTLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_LIGHTLIST, 1, Ct);
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
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_LIGHTLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildLightListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_LIGHTLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_LIGHTLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_LIGHTLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_LIGHTLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildLightListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_LIGHTLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_LIGHTLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_LIGHTLIST, 1, Place);
								break;
								} // if
							} // for
						} // if
					NumListItems ++;
					Ct ++;
					} // else
				} // if
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_LIGHTLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));

} // CelestialEditGUI::BuildLightList

/*===========================================================================*/

void CelestialEditGUI::BuildLightListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled && ((Light *)Me)->Distant)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->Name);

} // CelestialEditGUI::BuildLightListEntry()

/*===========================================================================*/

void CelestialEditGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RADIUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SIZEFACTOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DISTANCE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_REPLACEGRAY, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPHASE, WP_SCSYNC_NONOTIFY);

} // CelestialEditGUI::SyncWidgets

/*===========================================================================*/

void CelestialEditGUI::DisableWidgets(void)
{
char Disable = 0;
EffectList *CurLight;

if (CurLight = Active->Lights)
	{
	while (CurLight)
		{
		if (CurLight->Me && CurLight->Me->Enabled)
			{
			Disable = 1;
			break;
			} // if
		CurLight = CurLight->Next;
		} // while
	} // if

WidgetSetDisabled(IDC_LAT, Disable);
WidgetSetDisabled(IDC_LON, Disable);
WidgetSetDisabled(IDC_DISTANCE, Disable);
WidgetSetDisabled(IDC_ELEVATIONDROP, Disable);

WidgetSetDisabled(IDC_GRAYTEXT, Active->Img && Active->Img->GetRaster() && ! Active->Img->GetRaster()->GetIsColor() ? 0: 1);

} // CelestialEditGUI::DisableWidgets

/*===========================================================================*/

void CelestialEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CelestialEditGUI::Cancel

/*===========================================================================*/

void CelestialEditGUI::EditImage(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	GlobalApp->AppImages->SetActive(Active->Img->GetRaster());
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // if

} // CelestialEditGUI::DoEditImage()

/*===========================================================================*/

void CelestialEditGUI::AddLight(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_LIGHT);

} // CelestialEditGUI::AddLight

/*===========================================================================*/

void CelestialEditGUI::RemoveLight(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_LIGHTLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_LIGHTLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_LIGHTLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_LIGHTLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					if (Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
						{
						//EffectsHost->RemoveRAHost(RemoveItems[Ct], 0);
						} // if
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Light", "There are no Lights selected to remove.");
		} // else
	} // if

} // CelestialEditGUI::RemoveLight

/*===========================================================================*/

void CelestialEditGUI::EditLight(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_LIGHTLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_LIGHTLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_LIGHTLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // CelestialEditGUI::EditLight

/*===========================================================================*/

void CelestialEditGUI::Name(void)
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

} // CelestialEditGUI::Name()

/*===========================================================================*/

void CelestialEditGUI::SelectNewImage(void)
{
long Current;
Raster *NewRast, *MadeRast = NULL;

Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)CB_ERR && NewRast)
	|| (MadeRast = NewRast = GlobalApp->AppImages->AddRequestRaster()))
	{
	Active->SetRaster(NewRast);
	if (MadeRast)
		{
		GlobalApp->AppImages->SetActive(MadeRast);
		} // if
	} // if

} // CelestialEditGUI::SelectNewImage

/*===========================================================================*/

void CelestialEditGUI::ElevationPreset(void)
{
long Current;

if ((Current = WidgetCBGetCurSel(IDC_ELEVATIONDROP)) != CB_ERR && Current < WCS_PRESETS_NUMCELESTIALS)
	{
	Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].SetCurValue(EffectsLib::CelestialPresetDistance[Current]);
	} // if

} // CelestialEditGUI::ElevationPreset

/*===========================================================================*/

void CelestialEditGUI::RadiusPreset(void)
{
long Current;

if ((Current = WidgetCBGetCurSel(IDC_RADIUSDROP)) != CB_ERR && Current < WCS_PRESETS_NUMCELESTIALS)
	{
	Active->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].SetCurValue(EffectsLib::CelestialPresetRadius[Current]);
	} // if

} // CelestialEditGUI::RadiusPreset

/*===========================================================================*/
