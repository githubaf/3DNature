// StarfieldEditGUI.cpp
// Code for Starfield editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "StarfieldEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Raster.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "resource.h"

char *StarfieldEditGUI::TabNames[WCS_STARFIELDGUI_NUMTABS] = {"General", "Size && Position"};

long StarfieldEditGUI::ActivePage;

// Material GUI
#define WCS_STARFIELDED_MATGRADSET	1

NativeGUIWin StarfieldEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // StarfieldEditGUI::Open

/*===========================================================================*/

NativeGUIWin StarfieldEditGUI::Construct(void)
{
Raster *MyRast;
int TabIndex, Pos;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_STARFIELD_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_STARFIELD_SIZE, 0, 1);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_STARFIELDGUI_NUMTABS; TabIndex ++)
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
		// Material GUI
		MatGUI->Construct(WCS_STARFIELDED_MATGRADSET, WCS_STARFIELDED_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // StarfieldEditGUI::Construct

/*===========================================================================*/

StarfieldEditGUI::StarfieldEditGUI(EffectsLib *EffectsSource, StarFieldEffect *ActiveSource)
: GUIFenetre('STAR', this, "Starfield Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_STARFIELD, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
// Material GUI
MatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Starfield Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	// Material GUI
	if (MatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->ColorGrad)) // init ordinal 0
		{
		PopDropMaterialNotifier.Host = this; // to be able to call notifications later
		MatGUI->SetNotifyFunctor(&PopDropMaterialNotifier);
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

} // StarfieldEditGUI::StarfieldEditGUI

/*===========================================================================*/

StarfieldEditGUI::~StarfieldEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (MatGUI)
	delete MatGUI;

} // StarfieldEditGUI::~StarfieldEditGUI()

/*===========================================================================*/

long StarfieldEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_STG, 0);

return(0);

} // StarfieldEditGUI::HandleCloseWin

/*===========================================================================*/

long StarfieldEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_STG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_FREEIMAGE:
		{
		ClearImage();
		break;
		} // IDC_FREEIMAGE
	case ID_COLORPOT1:
		{
		if (Active->ColorGrad.ValidateNode(ActiveColorGrad) && ActiveColorNode)
			ActiveColorNode->Color.EditRAHost();
		break;
		} // 
	// Material GUI
	case IDC_POPDROP0:
		{
		if (WidgetGetCheck(IDC_POPDROP0))
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

} // StarfieldEditGUI::HandleButtonClick

/*===========================================================================*/

long StarfieldEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
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

} // StarfieldEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long StarfieldEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_IMAGEDROP:
		{
		SelectNewImage();
		break;
		}
	default:
		break;
	} // switch CtrlID

// Material GUI
MatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // StarfieldEditGUI::HandleCBChange

/*===========================================================================*/

long StarfieldEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // StarfieldEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long StarfieldEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // StarfieldEditGUI::HandlePageChange

/*===========================================================================*/

long StarfieldEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	default:
		break;
	} // switch CtrlID

return(0);

} // StarfieldEditGUI::HandleSCChange

/*===========================================================================*/

long StarfieldEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // StarfieldEditGUI::HandleFIChange

/*===========================================================================*/

void StarfieldEditGUI::HandleNotifyEvent(void)
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
Interested[5] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Interested[6] = NULL;
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[6] = MAKE_ID(Active->GetNotifyClass(), Active->ColorGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[7] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureColors();
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

if (! Done)
	ConfigureWidgets();

} // StarfieldEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void StarfieldEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
Raster *MyRast, *TestRast;
long ListPos, Ct, NumEntries;

sprintf(TextStr, "Starfield Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);

ConfigureFI(NativeWin, IDC_RANDSEED,
 &Active->RandomSeed,
  1.0,
   1.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

//WidgetSNConfig(IDC_DENSITY, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY]);
//WidgetSNConfig(IDC_SIZEFACTOR, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR]);
//WidgetSNConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY]);
//WidgetSNConfig(IDC_INTENSITYRANGE, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE]);
//WidgetSNConfig(IDC_LONGITUDEROT, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT]);
//WidgetSNConfig(IDC_TWINKLEAMP, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP]);
WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY], Active);
WidgetSmartRAHConfig(IDC_SIZEFACTOR, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR], Active);
WidgetSmartRAHConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY], Active);
WidgetSmartRAHConfig(IDC_INTENSITYRANGE, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE], Active);
WidgetSmartRAHConfig(IDC_LONGITUDEROT, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT], Active);
WidgetSmartRAHConfig(IDC_TWINKLEAMP, &Active->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP], Active);

WidgetAGConfig(IDC_ANIMGRADIENT, &Active->ColorGrad);
// Material GUI
MatGUI->ConfigureWidgets();

// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);

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

ConfigureColors();
DisableWidgets();

} // StarfieldEditGUI::ConfigureWidgets()

/*===========================================================================*/

void StarfieldEditGUI::ConfigureColors(void)
{

WidgetAGSync(IDC_ANIMGRADIENT);

if ((ActiveColorGrad = Active->ColorGrad.GetActiveNode()) && (ActiveColorNode = (ColorTextureThing *)ActiveColorGrad->GetThing()))
	WidgetSmartRAHConfig(IDC_COLOR1, &ActiveColorNode->Color, ActiveColorNode);
else
	WidgetSmartRAHConfig(IDC_COLOR1, (RasterAnimHost *)NULL, NULL);

// Material GUI
MatGUI->ConfigureMaterial();

} // StarfieldEditGUI::ConfigureColors

/*===========================================================================*/

void StarfieldEditGUI::SyncWidgets(void)
{

if (Active->ColorGrad.GetActiveNode() != ActiveColorGrad)
	{
	ConfigureWidgets();
	return;
	} // if

WidgetSNSync(IDC_DENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SIZEFACTOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_INTENSITYRANGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LONGITUDEROT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TWINKLEAMP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RANDSEED, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR1, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);

// Material GUI
MatGUI->SyncWidgets();

} // StarfieldEditGUI::SyncWidgets

/*===========================================================================*/

void StarfieldEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_COLOR1, ! (ActiveColorGrad && ActiveColorNode));

// Material GUI
MatGUI->DisableWidgets();

} // StarfieldEditGUI::DisableWidgets

/*===========================================================================*/

void StarfieldEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // StarfieldEditGUI::Cancel

/*===========================================================================*/

void StarfieldEditGUI::EditImage(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	GlobalApp->AppImages->SetActive(Active->Img->GetRaster());
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // if

} // StarfieldEditGUI::EditImage()

/*===========================================================================*/

void StarfieldEditGUI::ClearImage(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	Active->RemoveRAHost(Active->Img->GetRaster());
	} // if

} // StarfieldEditGUI::ClearImage()

/*===========================================================================*/

void StarfieldEditGUI::Name(void)
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

} // StarfieldEditGUI::Name()

/*===========================================================================*/

void StarfieldEditGUI::SetNewActiveGrad(GradientCritter *NewNode)
{

if (ActiveColorGrad = NewNode)
	ActiveColorNode = (ColorTextureThing *)ActiveColorGrad->GetThing();
else
	ActiveColorNode = NULL;
	
} // StarfieldEditGUI::SetNewActiveGrad

/*===========================================================================*/

void StarfieldEditGUI::SelectNewImage(void)
{
Raster *NewRast, *MadeRast = NULL;
long Current;

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

} // StarfieldEditGUI::SelectNewImage

/*===========================================================================*/

// Material GUI
void StarfieldEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{

if (Host) Host->ConfigureColors();

} // StarfieldEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void StarfieldEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{

if (Host) Host->SetNewActiveGrad(NewNode);

} // StarfieldEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void StarfieldEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if (ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, MatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(MatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT, true);
	//WidgetShow(IDC_ANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // StarfieldEditGUI::ShowMaterialPopDrop

/*===========================================================================*/
