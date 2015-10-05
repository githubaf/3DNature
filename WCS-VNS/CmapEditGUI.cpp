// CmapEditGUI.cpp
// Code for Cmap editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CmapEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Raster.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "AppMem.h"
#include "ViewGUI.h"
#include "Conservatory.h"
#include "resource.h"
#include "Lists.h"

char *CmapEditGUI::TabNames[WCS_CMAPGUI_NUMTABS] = {"General", "Ecosystems"};

long CmapEditGUI::ActivePage;
// advanced
long CmapEditGUI::DisplayAdvanced;

NativeGUIWin CmapEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CmapEditGUI::Open

/*===========================================================================*/

NativeGUIWin CmapEditGUI::Construct(void)
{
int TabIndex, Pos;
Raster *MyRast;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_CMAP_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_CMAP_ECOSYSTEMS, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_CMAPGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
		for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
			{
			if(MyRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
				{
				Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
				WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
				} // if
			} // for
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // CmapEditGUI::Construct

/*===========================================================================*/

CmapEditGUI::CmapEditGUI(EffectsLib *EffectsSource, Database *DBSource, CmapEffect *ActiveSource)
: GUIFenetre('CMAP', this, "Color Map Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_CMAP, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
ActiveEco = NULL;
ResponseEnabled = 0;
MatchColorsChanged = 0;
PickColorType = FirstPick = 0;
ReceivingDiagnostics = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Color Map Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // CmapEditGUI::CmapEditGUI

/*===========================================================================*/

CmapEditGUI::~CmapEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // CmapEditGUI::~CmapEditGUI()

/*===========================================================================*/

long CmapEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_CMG, 0);

return(0);

} // CmapEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long CmapEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // CmapEditGUI::HandleShowAdvanced

/*===========================================================================*/

long CmapEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CMG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDECO:
		{
		AddEco();
		break;
		} // IDC_ADDECO
	case IDC_REMOVEECO:
		{
		RemoveEco();
		break;
		} // IDC_REMOVEECO
	case IDC_MOVEECOUP:
		{
		ChangeEcoListPosition(1);
		break;
		} // IDC_MOVEECOUP
	case IDC_MOVEECODOWN:
		{
		ChangeEcoListPosition(0);
		break;
		} // IDC_MOVEECODOWN
	case IDC_GRABALL:
		{
		Active->GrabAllEcosystems();
		break;
		} // IDC_GRABALL
	case IDC_VIEWIMAGE:
		{
		OpenPreview();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // CmapEditGUI::HandleButtonClick

/*===========================================================================*/

long CmapEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
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

} // CmapEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long CmapEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

return (0);

} // CmapEditGUI::HandleCBChange

/*===========================================================================*/

long CmapEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(028, 28);
switch (CtrlID)
	{
	case IDC_ECOLIST:
		{
		SetActiveEco();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // CmapEditGUI::HandleListSel

/*===========================================================================*/

long CmapEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_ECOLIST:
		{
		RemoveEco();
		break;
		} // IDC_ECOLIST
	default:
		break;
	} // switch

return(0);

} // CmapEditGUI::HandleListDelItem

/*===========================================================================*/

long CmapEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ECOLIST:
		{
		EditEco();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // CmapEditGUI::HandleListDoubleClick

/*===========================================================================*/

long CmapEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // CmapEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long CmapEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
		ActivePage = NewPageID;
		break;
		}
	default:
		break;
	} // switch

return(0);

} // CmapEditGUI::HandlePageChange

/*===========================================================================*/

long CmapEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKRESPONSEENABLED:
		{
		FirstPick = 1;
		ConfigureMatchColors();
		DisableWidgets();
		break;
		} // 
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKMATCHRANGE:
		{
		if (ActiveEco && EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
			{
			Changes[0] = MAKE_ID(ActiveEco->GetNotifyClass(), ActiveEco->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEco->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CmapEditGUI::HandleSCChange

/*===========================================================================*/

long CmapEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOCOLORBYPIXEL:
	case IDC_RADIOCOLORBYPOLY:
		{
		if (Active->Orientation == WCS_CMAP_ORIENTATION_TOPWEST && Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL)
			{
			if (UserMessageYN("Color Map", "This is an old format Color Map component which has West at the top of the image. Color Maps with West at the top can not be draped. Do you wish to modify the format (the image will need to be rotated counter-clockwise by 90 degrees in a paint program)?"))
				Active->Orientation = WCS_CMAP_ORIENTATION_TOPNORTH;
			else
				Active->EvalByPixel = WCS_CMAP_EVAL_BYPOLYGON;
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // CmapEditGUI::HandleSRChange

/*===========================================================================*/

long CmapEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_MATCHRED:
	case IDC_MATCHGREEN:
	case IDC_MATCHBLUE:
	case IDC_MATCHRED2:
	case IDC_MATCHGREEN2:
	case IDC_MATCHBLUE2:
		{
		if (ActiveEco && EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
			{
			MatchColorsChanged = 1;
			// these float ints aren't really configured to anim colortimes but that's what everyone ese is listening for
			Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEco->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CmapEditGUI::HandleFIChange

/*===========================================================================*/

void CmapEditGUI::HandleNotifyEvent(void)
{
Raster *MyRast, *MatchRast;
NotifyTag *Changes, Changed, Interested[7];
long Pos, CurPos, Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ResponseEnabled)
		{
		if (((DiagnosticData *)Activity->ChangeNotify->NotifyData)->ValueValid[WCS_DIAGNOSTIC_RGB])
			RespondColorNotify((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	Done = 1;
	} // if

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
	// advanced
	DisplayAdvancedFeatures();
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
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MatchRast);
		} // else
	} // else

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureMatchColors();
	DisableWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // CmapEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void CmapEditGUI::ConfigureWidgets(void)
{
Raster *MyRast, *TestRast;
long ListPos, Ct, NumEntries;
char TextStr[256];

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

sprintf(TextStr, "Color Map Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKRESPONSEENABLED, &ResponseEnabled, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOCOLORBYPIXEL, IDC_RADIOCOLORBYPIXEL, &Active->EvalByPixel, SRFlag_Short, WCS_CMAP_EVAL_BYPIXEL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCOLORBYPIXEL, IDC_RADIOCOLORBYPOLY, &Active->EvalByPixel, SRFlag_Short, WCS_CMAP_EVAL_BYPOLYGON, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOPICKMINIMUM, IDC_RADIOPICKMINIMUM, &PickColorType, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPICKMINIMUM, IDC_RADIOPICKMAXIMUM, &PickColorType, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPICKMINIMUM, IDC_RADIOPICKRANGE, &PickColorType, SRFlag_Char, 2, NULL, NULL);

ConfigureSC(NativeWin, IDC_CHECKRANDOMIZE, &Active->RandomizeEdges, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKLUMINOUS, &Active->LuminousColors, SCFlag_Short, NULL, 0);

ConfigureTB(NativeWin, IDC_ADDECO, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEECO, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEECOUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEECODOWN, IDI_ARROWDOWN, NULL);

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

BuildEcoList();
ConfigureMatchColors();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // CmapEditGUI::ConfigureWidgets()

/*===========================================================================*/

void CmapEditGUI::ConfigureMatchColors(void)
{
unsigned char Red, Grn, Blu;

if (ActiveEco)
	{
	// check to see if active eco is valid
	if (! EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
		ActiveEco = NULL;
	} // if
if (! ActiveEco && Active->Ecosystems)
	ActiveEco = (EcosystemEffect *)Active->Ecosystems->Me;
if (ActiveEco)
	{
	PickColorType = ActiveEco->CmapMatchRange ? PickColorType: 0;
	ConfigureSC(NativeWin, IDC_CHECKMATCHRANGE, &ActiveEco->CmapMatchRange, SCFlag_Short, NULL, 0);

	ConfigureFI(NativeWin, IDC_MATCHRED,
	 &ActiveEco->MatchColor[0],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_MATCHGREEN,
	 &ActiveEco->MatchColor[1],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_MATCHBLUE,
	 &ActiveEco->MatchColor[2],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_MATCHRED2,
	 &ActiveEco->MatchColor[3],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_MATCHGREEN2,
	 &ActiveEco->MatchColor[4],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_MATCHBLUE2,
	 &ActiveEco->MatchColor[5],
	  1.0,
	   0.0,
		255.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	Red = (unsigned char)(ActiveEco->MatchColor[0]);
	Grn = (unsigned char)(ActiveEco->MatchColor[1]);
	Blu = (unsigned char)(ActiveEco->MatchColor[2]);
	SetColorPot(0, Red, Grn, Blu, 1);
	ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);
	Red = (unsigned char)(ActiveEco->MatchColor[3]);
	Grn = (unsigned char)(ActiveEco->MatchColor[4]);
	Blu = (unsigned char)(ActiveEco->MatchColor[5]);
	SetColorPot(1, Red, Grn, Blu, 1);
	ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);

	WidgetSRSync(IDC_RADIOPICKMINIMUM, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOPICKMAXIMUM, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOPICKRANGE, WP_SRSYNC_NONOTIFY);

	WidgetSetText(IDC_MINTEXT, ActiveEco->CmapMatch && ActiveEco->CmapMatchRange ? "Minimum": "Values");
	WidgetSetText(IDC_RADIOPICKMINIMUM, ActiveEco->CmapMatchRange ? "Min": "Val");
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_CHECKMATCHRANGE, NULL, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHRED, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHGREEN, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHBLUE, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHRED2, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHGREEN2, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_MATCHBLUE2, NULL, 1.0, 0.0, 255.0, 0, NULL, 0);
	WidgetSetText(IDC_MINTEXT, "Values");
	WidgetSetText(IDC_RADIOPICKMINIMUM, "Val");
	} // else

} // CmapEditGUI::ConfigureMatchColors

/*===========================================================================*/

void CmapEditGUI::BuildEcoList(void)
{
EffectList *Current = Active->Ecosystems;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_ECOLIST);

if (ActiveEco)
	{
	// check to see if active eco is valid
	if (! EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
		ActiveEco = NULL;
	} // if
if (! ActiveEco && Active->Ecosystems)
	ActiveEco = (EcosystemEffect *)Active->Ecosystems->Me;

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_ECOLIST, TempCt))
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
			if (WidgetLBGetSelState(IDC_ECOLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_ECOLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_ECOLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildEcoListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_ECOLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_ECOLIST, Ct, Current->Me);
				if (Current->Me == ActiveEco)
					WidgetLBSetSelState(IDC_ECOLIST, 1, Ct);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_ECOLIST, 1, Ct);
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
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_ECOLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildEcoListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_ECOLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_ECOLIST, TempCt, Current->Me);
					if (Current->Me == ActiveEco)
						WidgetLBSetSelState(IDC_ECOLIST, 1, TempCt);
					else if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ECOLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_ECOLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildEcoListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_ECOLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_ECOLIST, Place, Current->Me);
					if (Current->Me == ActiveEco)
						WidgetLBSetSelState(IDC_ECOLIST, 1, Place);
					else if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ECOLIST, 1, Place);
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
		WidgetLBDelete(IDC_ECOLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));

} // CmapEditGUI::BuildEcoList

/*===========================================================================*/

void CmapEditGUI::BuildEcoListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled && ((EcosystemEffect *)Me)->CmapMatch)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // CmapEditGUI::BuildEcoListEntry()

/*===========================================================================*/

void CmapEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOCOLORBYPIXEL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCOLORBYPOLY, WP_SRSYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRANDOMIZE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLUMINOUS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKMATCHECO, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKMATCHRANGE, WP_SCSYNC_NONOTIFY);

} // CmapEditGUI::SyncWidgets

/*===========================================================================*/

void CmapEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_CHECKMATCHECO, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
WidgetSetDisabled(IDC_CHECKRANDOMIZE, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
// disabling this is deemed unnecessary. Code to set the material to luminous was added to EffectCmap.cpp on 7/5/05
//WidgetSetDisabled(IDC_CHECKLUMINOUS, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);

if (ActiveEco)
	{
	// check to see if active eco is valid
	if (! EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
		ActiveEco = NULL;
	} // if
if (! ActiveEco && Active->Ecosystems)
	ActiveEco = (EcosystemEffect *)Active->Ecosystems->Me;
if (ActiveEco)
	{
	WidgetSetDisabled(IDC_MATCHCOLORTXT, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! ActiveEco->CmapMatch);
	WidgetSetDisabled(IDC_CHECKMATCHRANGE, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! ActiveEco->CmapMatch);
	WidgetSetDisabled(IDC_MATCHRED, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch));
	WidgetSetDisabled(IDC_MATCHGREEN, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch));
	WidgetSetDisabled(IDC_MATCHBLUE, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch));
	WidgetSetDisabled(IDC_MATCHRED2, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_MATCHGREEN2, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_MATCHBLUE2, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_MINTEXT, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! ActiveEco->CmapMatch);
	WidgetSetDisabled(IDC_MAXTEXT, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_CHECKRESPONSEENABLED, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! ActiveEco->CmapMatch);
	WidgetSetDisabled(IDC_RADIOPICKMINIMUM, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ResponseEnabled));
	WidgetSetDisabled(IDC_RADIOPICKMAXIMUM, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ResponseEnabled && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_RADIOPICKRANGE, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL || ! (ActiveEco->CmapMatch && ResponseEnabled && ActiveEco->CmapMatchRange));
	WidgetSetDisabled(IDC_GRABALL, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_ADDECO, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_REMOVEECO, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_MOVEECOUP, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_MOVEECODOWN, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	} // if
else
	{
	WidgetSetDisabled(IDC_MINTEXT, 1);
	WidgetSetDisabled(IDC_MAXTEXT, 1);
	WidgetSetDisabled(IDC_CHECKRESPONSEENABLED, 1);
	WidgetSetDisabled(IDC_RADIOPICKMINIMUM, 1);
	WidgetSetDisabled(IDC_RADIOPICKMAXIMUM, 1);
	WidgetSetDisabled(IDC_RADIOPICKRANGE, 1);
	WidgetSetDisabled(IDC_GRABALL, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_ADDECO, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL );
	WidgetSetDisabled(IDC_REMOVEECO, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_MOVEECOUP, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	WidgetSetDisabled(IDC_MOVEECODOWN, Active->EvalByPixel == WCS_CMAP_EVAL_BYPIXEL);
	} // else

} // CmapEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void CmapEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced || Active->EvalByPixel == WCS_CMAP_EVAL_BYPOLYGON)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_MATCHRED, true);
	WidgetShow(IDC_MATCHGREEN, true);
	WidgetShow(IDC_MATCHBLUE, true);
	WidgetShow(IDC_MATCHRED2, true);
	WidgetShow(IDC_MATCHGREEN2, true);
	WidgetShow(IDC_MATCHBLUE2, true);
	WidgetShow(IDC_CHECKMATCHRANGE, true);
	WidgetShow(IDC_MINTEXT, true);
	WidgetShow(IDC_MAXTEXT, true);
	WidgetShow(IDC_MATCHCOLORTXT, true);
	WidgetShow(IDC_RTXT, true);
	WidgetShow(IDC_GTXT, true);
	WidgetShow(IDC_BTXT, true);
	WidgetShow(ID_COLORPOT1, true);
	WidgetShow(ID_COLORPOT2, true);
	WidgetShow(IDC_SELECOTXT, true);
	WidgetShow(IDC_ECOLIST, true);
	WidgetShow(IDC_ADDECO, true);
	WidgetShow(IDC_REMOVEECO, true);
	WidgetShow(IDC_MOVEECOUP, true);
	WidgetShow(IDC_MOVEECODOWN, true);
	WidgetShow(IDC_GRABALL, true);
	WidgetShow(IDC_VIEWIMAGE, true);
	WidgetShow(IDC_CHECKRESPONSEENABLED, true);
	WidgetShow(IDC_RADIOPICKMINIMUM, true);
	WidgetShow(IDC_RADIOPICKMAXIMUM, true);
	WidgetShow(IDC_RADIOPICKRANGE, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_MATCHRED, false);
	WidgetShow(IDC_MATCHGREEN, false);
	WidgetShow(IDC_MATCHBLUE, false);
	WidgetShow(IDC_MATCHRED2, false);
	WidgetShow(IDC_MATCHGREEN2, false);
	WidgetShow(IDC_MATCHBLUE2, false);
	WidgetShow(IDC_CHECKMATCHRANGE, false);
	WidgetShow(IDC_MINTEXT, false);
	WidgetShow(IDC_MAXTEXT, false);
	WidgetShow(IDC_MATCHCOLORTXT, false);
	WidgetShow(IDC_RTXT, false);
	WidgetShow(IDC_GTXT, false);
	WidgetShow(IDC_BTXT, false);
	WidgetShow(ID_COLORPOT1, false);
	WidgetShow(ID_COLORPOT2, false);
	WidgetShow(IDC_SELECOTXT, false);
	WidgetShow(IDC_ECOLIST, false);
	WidgetShow(IDC_ADDECO, false);
	WidgetShow(IDC_REMOVEECO, false);
	WidgetShow(IDC_MOVEECOUP, false);
	WidgetShow(IDC_MOVEECODOWN, false);
	WidgetShow(IDC_GRABALL, false);
	WidgetShow(IDC_VIEWIMAGE, false);
	WidgetShow(IDC_CHECKRESPONSEENABLED, false);
	WidgetShow(IDC_RADIOPICKMINIMUM, false);
	WidgetShow(IDC_RADIOPICKMAXIMUM, false);
	WidgetShow(IDC_RADIOPICKRANGE, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // CmapEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void CmapEditGUI::Cancel(void)
{
NotifyTag Changes[2];

if (MatchColorsChanged)
	{
	UserMessageOK("Undo", "Changes made to Ecosystem Match Colors will not be undone.");
	MatchColorsChanged = 0;
	} // if

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CmapEditGUI::Cancel

/*===========================================================================*/

void CmapEditGUI::EditImage(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	GlobalApp->AppImages->SetActive(Active->Img->GetRaster());
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // if

} // CmapEditGUI::DoEditImage()

/*===========================================================================*/

void CmapEditGUI::OpenPreview(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	Active->Img->GetRaster()->OpenPreview(FALSE);
	} // if

} // CmapEditGUI::OpenPreview()

/*===========================================================================*/

void CmapEditGUI::AddEco(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_ECOSYSTEM);

} // CmapEditGUI::AddEco

/*===========================================================================*/

void CmapEditGUI::RemoveEco(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_ECOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ECOLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_ECOLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_ECOLIST, Ct);
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
		UserMessageOK("Remove Ecosystem", "There are no Ecosystems selected to remove.");
		} // else
	} // if

} // CmapEditGUI::RemoveEco

/*===========================================================================*/

void CmapEditGUI::EditEco(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_ECOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ECOLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_ECOLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // CmapEditGUI::EditEco

/*===========================================================================*/

void CmapEditGUI::Name(void)
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

} // CmapEditGUI::Name()

/*===========================================================================*/

void CmapEditGUI::ChangeEcoListPosition(short MoveUp)
{
long Ct, NumListEntries, SendNotify = 0;
RasterAnimHost *MoveMe;
EffectList *Current, *PrevEco = NULL, *PrevPrevEco = NULL, *StashEco;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_ECOLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_ECOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_ECOLIST, Ct))
					{
					if (Current = Active->Ecosystems)
						{
						while (Current->Me != MoveMe)
							{
							PrevPrevEco = PrevEco;
							PrevEco = Current;
							Current = Current->Next;
							} // while
						if (Current && Current->Me)
							{
							if (PrevEco)
								{
								StashEco = Current->Next;
								if (PrevPrevEco)
									{
									PrevPrevEco->Next = Current;
									Current->Next = PrevEco;
									} // if
								else
									{
									Active->Ecosystems = Current;
									Active->Ecosystems->Next = PrevEco;
									} // else
								PrevEco->Next = StashEco;
								SendNotify = 1;
								} // else if
							else
								break;
							} // if
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
		} // if
	else
		{
		for (Ct = NumListEntries - 1; Ct >= 0; Ct --)
			{
			if (WidgetLBGetSelState(IDC_ECOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_ECOLIST, Ct))
					{
					if (Current = Active->Ecosystems)
						{
						while (Current->Me != MoveMe)
							{
							PrevPrevEco = PrevEco;
							PrevEco = Current;
							Current = Current->Next;
							} // while
						if (Current && Current->Me)
							{
							if (Current->Next)
								{
								StashEco = Current->Next->Next;
								if (PrevEco)
									{
									PrevEco->Next = Current->Next;
									PrevEco->Next->Next = Current;
									} // if
								else
									{
									Active->Ecosystems = Current->Next;
									Active->Ecosystems->Next = Current;
									} // else
								Current->Next = StashEco;
								SendNotify = 1;
								} // if move down
							else
								break;
							} // if
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
		} // else
	} // if

// need to send a very general message that will cause SAG to completely rebuild
// Just updating the object will cause crash in SAG with NULL pointer
if (SendNotify)
	{
	Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // CmapEditGUI::ChangeEcoListPosition

/*===========================================================================*/

void CmapEditGUI::SetActiveEco(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_ECOLIST);
Current = (long)WidgetLBGetItemData(IDC_ECOLIST, Current);
if (Current != LB_ERR)
	ActiveEco = (EcosystemEffect *)Current;
ConfigureMatchColors();

} // CmapEditGUI::SetActiveEco()

/*===========================================================================*/

void CmapEditGUI::SelectNewImage(void)
{
long Current;
Raster *NewRast, *MadeRast = NULL;

Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)LB_ERR && NewRast)
	|| (MadeRast = NewRast = GlobalApp->AppImages->AddRequestRaster()))
	{
	Active->SetRaster(NewRast);
	if (MadeRast)
		{
		GlobalApp->AppImages->SetActive(MadeRast);
		} // if
	} // if

ValidateGeoRefImage();

} // CmapEditGUI::SelectNewImage

/*===========================================================================*/

int CmapEditGUI::ValidateGeoRefImage(void)
{
RasterAttribute *MyAttr;

if (Active->Img && Active->Img->GetRaster())
	{
	if (! ((MyAttr = Active->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell()))
		{
		UserMessageOK("Georeferenced Image", "Selected image is not geo-referenced.\n Set up reference coordinates in the Image Object Library.");
		} // if
	else
		return (1);
	} // if
else
	{
	UserMessageOK("Georeferenced Image", "There is no image selected to geo-reference.");
	} // else
return (0);

} // CmapEditGUI::ValidateGeoRefImage

/*===========================================================================*/

void CmapEditGUI::RespondColorNotify(DiagnosticData *Data)
{
NotifyTag Changes[2];

if (ActiveEco && EffectsHost->IsEffectValid(ActiveEco, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0))
	{
	if (Data->DataRGB[0] || Data->DataRGB[1] || Data->DataRGB[2])
		{
		MatchColorsChanged = 1;
		if (PickColorType == 0)
			{
			ActiveEco->MatchColor[0] = Data->DataRGB[0];
			ActiveEco->MatchColor[1] = Data->DataRGB[1];
			ActiveEco->MatchColor[2] = Data->DataRGB[2];
			} // if
		else if (PickColorType == 1)
			{
			if (Data->DataRGB[0] >= ActiveEco->MatchColor[0])
				ActiveEco->MatchColor[3] = Data->DataRGB[0];
			else
				ActiveEco->MatchColor[3] = ActiveEco->MatchColor[0];
			if (Data->DataRGB[1] >= ActiveEco->MatchColor[1])
				ActiveEco->MatchColor[4] = Data->DataRGB[1];
			else
				ActiveEco->MatchColor[4] = ActiveEco->MatchColor[1];
			if (Data->DataRGB[2] >= ActiveEco->MatchColor[2])
				ActiveEco->MatchColor[5] = Data->DataRGB[2];
			else
				ActiveEco->MatchColor[5] = ActiveEco->MatchColor[2];
			} // if
		else
			{
			if (FirstPick)
				{
				ActiveEco->MatchColor[0] = ActiveEco->MatchColor[3] = Data->DataRGB[0];
				ActiveEco->MatchColor[1] = ActiveEco->MatchColor[4] = Data->DataRGB[1];
				ActiveEco->MatchColor[2] = ActiveEco->MatchColor[5] = Data->DataRGB[2];
				FirstPick = 0;
				} // if
			else
				{
				if (Data->DataRGB[0] < ActiveEco->MatchColor[0])
					ActiveEco->MatchColor[0] = Data->DataRGB[0];
				if (Data->DataRGB[0] > ActiveEco->MatchColor[3])
					ActiveEco->MatchColor[3] = Data->DataRGB[0];
				if (Data->DataRGB[1] < ActiveEco->MatchColor[1])
					ActiveEco->MatchColor[1] = Data->DataRGB[1];
				if (Data->DataRGB[1] > ActiveEco->MatchColor[4])
					ActiveEco->MatchColor[4] = Data->DataRGB[1];
				if (Data->DataRGB[2] < ActiveEco->MatchColor[2])
					ActiveEco->MatchColor[2] = Data->DataRGB[2];
				if (Data->DataRGB[2] > ActiveEco->MatchColor[5])
					ActiveEco->MatchColor[5] = Data->DataRGB[2];
				} // if
			} // if
		Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveEco->GetRAHostRoot());
		} // if
	} // if

} // CmapEditGUI::RespondColorNotify

/*===========================================================================*/
