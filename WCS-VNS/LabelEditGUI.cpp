// LabelEditGUI.cpp
// Code for Label editor
// Built from FoliageEffectEditGUI.cpp on 4/14/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "LabelEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "EffectsLib.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

extern char *Label_TextSymbols[];
extern char *Label_TextLabels[];

char *LabelEditGUI::TabNames[WCS_LABELGUI_NUMTABS] = {"General", "Text", "Parts", "Controls", "Sizes", "Anchor"};
long LabelEditGUI::ActivePage;
char *Attrib_TextSymbol = "&AT";

NativeGUIWin LabelEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // LabelEditGUI::Open

/*===========================================================================*/

NativeGUIWin LabelEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_LABEL_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_LABEL_TEXT, 0, 1);
	CreateSubWinFromTemplate(IDD_LABEL_PARTS, 0, 2);
	CreateSubWinFromTemplate(IDD_LABEL_EXTRABITS, 0, 3);
	CreateSubWinFromTemplate(IDD_LABEL_ANIMPARAMS, 0, 4);
	CreateSubWinFromTemplate(IDD_LABEL_ANCHOR, 0, 5);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_LABELGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // LabelEditGUI::Construct

/*===========================================================================*/

LabelEditGUI::LabelEditGUI(EffectsLib *EffectsSource, Database *DBSource, Label *ActiveSource)
: GUIFenetre('LABL', this, "Label Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_LABEL, 0xff, 0xff, 0xff),
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),

								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, WCS_RAHOST_OBJTYPE_DEM, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, WCS_RAHOST_OBJTYPE_VECTOR, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;

if (EffectsHost && ActiveSource)
	{
	sprintf(NameStr, "Label Editor - %s ", Active->GetName());
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

} // LabelEditGUI::LabelEditGUI

/*===========================================================================*/

LabelEditGUI::~LabelEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // LabelEditGUI::~LabelEditGUI()

/*===========================================================================*/

long LabelEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_LBL, 0);

return(0);

} // LabelEditGUI::HandleCloseWin

/*===========================================================================*/

long LabelEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_LBL, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_SCALE:
		{
		ScaleSizes();
		break;
		} // IDC_SCALE
	default:
		break;
	} // ButtonID

return(0);

} // LabelEditGUI::HandleButtonClick

/*===========================================================================*/

long LabelEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_OVERLAYTEXT:
		{
		OverlayText();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // LabelEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long LabelEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_INSERTLIST:
		{
		HWND Wid;
		int LabelInsert;
		if((LabelInsert = WidgetLBGetCurSel(IDC_INSERTLIST)) != -1)
			{
			if(Wid = GetWidgetFromID(IDC_OVERLAYTEXT))
				{
				SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)Label_TextSymbols[LabelInsert]);
				OverlayText();
				} // if
			} // if
		break;
		} // INSERTLIST
	case IDC_ATTRLIST:
		{
		HWND Wid;
		int LabelInsert;
		LayerEntry *Entry;
		const char *LayerName;
		char TextReplace[264];

		if((LabelInsert = WidgetLBGetCurSel(IDC_ATTRLIST)) != -1)
			{
			if(Wid = GetWidgetFromID(IDC_OVERLAYTEXT))
				{
				if ((Entry = (LayerEntry *)WidgetLBGetItemData(IDC_ATTRLIST, LabelInsert)) && Entry != (LayerEntry *)LB_ERR)
					{
					LayerName = Entry->GetName();
					sprintf(TextReplace, "%s%s%s", Attrib_TextSymbol, &LayerName[1], Attrib_TextSymbol);
					SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)TextReplace);
					OverlayText();
					} // if
				} // if
			} // if
		break;
		} // IDC_ATTRLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // LabelEditGUI::HandleListDoubleClick

/*===========================================================================*/

long LabelEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		if (NewPageID >= WCS_LABELGUI_NUMTABS || NewPageID < 0)
			NewPageID = 0;
		ShowPanel(0, NewPageID);
		break;
		}
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // LabelEditGUI::HandlePageChange

/*===========================================================================*/

long LabelEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKPOLEFULLWIDTH:
		{
		Active->PolePosition = WCS_EFFECTS_LABEL_POLEPOSITION_CENTER;
		Active->PoleStyle = WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_CHECKPOLEFULLWIDTH
	default:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // default
	} // switch CtrlID

return(0);

} // LabelEditGUI::HandleSCChange

/*===========================================================================*/

long LabelEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOABSOLUTE:
	case IDC_RADIORELATIVEGRND:
	case IDC_RADIORELATIVEJOE:
	case IDC_RADIOJUSTIFYLEFT:
	case IDC_RADIOJUSTIFYCENTER:
	case IDC_RADIOJUSTIFYRIGHT:
	case IDC_RADIOFLAGWIDTHFIXED:
	case IDC_RADIOFLAGWIDTHFLOAT:
	case IDC_RADIOFLAGWIDTHTEXTFLOAT:
	case IDC_RADIOFLAGHEIGHTFIXED:
	case IDC_RADIOFLAGHEIGHTFLOAT:
	case IDC_RADIOPOLEPOSLEFT:
	case IDC_RADIOPOLEPOSRIGHT:
	case IDC_RADIOPOLESTRAIGHT:
	case IDC_RADIOPOLEANGLED:
	case IDC_RADIOPOLEBASETAPERED:
	case IDC_RADIOHIRESALWAYS:
	case IDC_RADIOHIRESSOMETIMES:
	case IDC_RADIOHIRESNEVER:
	case IDC_RADIOANCHORCENTER:
	case IDC_RADIOANCHORLOWERLEFT:
	case IDC_RADIOANCHORLEFTEDGE:
	case IDC_RADIOANCHORUPPERLEFT:
	case IDC_RADIOANCHORTOPEDGE:
	case IDC_RADIOANCHORUPPERRIGHT:
	case IDC_RADIOANCHORRIGHTEDGE:
	case IDC_RADIOANCHORLOWERRIGHT:
	case IDC_RADIOANCHORBOTTOMEDGE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOPOLEPOSCENTER:
	case IDC_RADIOPOLEBASESQUARE:
		{
		Active->PoleStyle = WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_RADIOPOLEPOSCENTER
	default:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // default
	} // switch CtrlID

return(0);

} // LabelEditGUI::HandleSRChange

/*===========================================================================*/

long LabelEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

/*
switch (CtrlID)
	{
	} // switch CtrlID
*/
return(0);

} // LabelEditGUI::HandleFIChange

/*===========================================================================*/

void LabelEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[5] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[6] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
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

} // LabelEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void LabelEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
long LabelInsert;
char TempLabelFormat[100];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

sprintf(TextStr, "Label Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPREVIEWENABLED, &Active->PreviewEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRENDEROCCLUDED, &Active->RenderOccluded, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTEXTENABLED, &Active->TextEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOUTLINEENABLED, &Active->OutlineEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLAGENABLED, &Active->FlagEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKBORDERENABLED, &Active->BorderEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPOLEENABLED, &Active->PoleEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKWRAPENABLED, &Active->WordWrapEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPOLEFULLWIDTH, &Active->PoleFullWidth, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPOLEFULLHEIGHT, &Active->PoleFullHeight, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOVERHEADPOLE, &Active->OverheadViewPole, SCFlag_Char, NULL, 0);

WidgetSmartRAHConfig(IDC_BASEELEV, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV], Active);
WidgetSmartRAHConfig(IDC_MASTERSIZE, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE], Active);
WidgetSmartRAHConfig(IDC_POLEHEIGHT, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT], Active);
WidgetSmartRAHConfig(IDC_POLEWIDTH, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH], Active);
WidgetSmartRAHConfig(IDC_MAXFLAGWIDTH, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH], Active);
WidgetSmartRAHConfig(IDC_MAXFLAGHEIGHT, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT], Active);
WidgetSmartRAHConfig(IDC_BORDERWIDTH, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH], Active);
WidgetSmartRAHConfig(IDC_TEXTOUTLINEWIDTH, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH], Active);
WidgetSmartRAHConfig(IDC_TEXTLINEHEIGHT, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT], Active);
WidgetSmartRAHConfig(IDC_TEXTLINESPACE, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE], Active);
WidgetSmartRAHConfig(IDC_TEXTLETTERSPACE, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE], Active);
WidgetSmartRAHConfig(IDC_TEXTTRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR], Active);
WidgetSmartRAHConfig(IDC_OUTLINETRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR], Active);
WidgetSmartRAHConfig(IDC_FLAGTRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR], Active);
WidgetSmartRAHConfig(IDC_BORDERTRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR], Active);
WidgetSmartRAHConfig(IDC_POLETRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR], Active);
WidgetSmartRAHConfig(IDC_TEXTCOLOR, &Active->TextColor, Active);
WidgetSmartRAHConfig(IDC_OUTLINECOLOR, &Active->OutlineColor, Active);
WidgetSmartRAHConfig(IDC_FLAGCOLOR, &Active->FlagColor, Active);
WidgetSmartRAHConfig(IDC_BORDERCOLOR, &Active->BorderColor, Active);
WidgetSmartRAHConfig(IDC_POLECOLOR, &Active->PoleColor, Active);

ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIOABSOLUTE, &Active->Absolute, SRFlag_Short, WCS_EFFECT_ABSOLUTE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEGRND, &Active->Absolute, SRFlag_Short, WCS_EFFECT_RELATIVETOGROUND, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEJOE, &Active->Absolute, SRFlag_Short, WCS_EFFECT_RELATIVETOJOE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOJUSTIFYLEFT, IDC_RADIOJUSTIFYLEFT, &Active->Justification, SRFlag_Char, WCS_EFFECTS_LABEL_JUSTIFY_LEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOJUSTIFYLEFT, IDC_RADIOJUSTIFYCENTER, &Active->Justification, SRFlag_Char, WCS_EFFECTS_LABEL_JUSTIFY_CENTER, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOJUSTIFYLEFT, IDC_RADIOJUSTIFYRIGHT, &Active->Justification, SRFlag_Char, WCS_EFFECTS_LABEL_JUSTIFY_RIGHT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFLAGWIDTHFIXED, IDC_RADIOFLAGWIDTHFIXED, &Active->FlagWidthStyle, SRFlag_Char, WCS_EFFECTS_LABEL_FLAGSIZE_FIXED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFLAGWIDTHFIXED, IDC_RADIOFLAGWIDTHFLOAT, &Active->FlagWidthStyle, SRFlag_Char, WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFLAGWIDTHFIXED, IDC_RADIOFLAGWIDTHTEXTFLOAT, &Active->FlagWidthStyle, SRFlag_Char, WCS_EFFECTS_LABEL_FLAGSIZE_FIXEDFLOATTEXT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFLAGHEIGHTFIXED, IDC_RADIOFLAGHEIGHTFIXED, &Active->FlagHeightStyle, SRFlag_Char, WCS_EFFECTS_LABEL_FLAGSIZE_FIXED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFLAGHEIGHTFIXED, IDC_RADIOFLAGHEIGHTFLOAT, &Active->FlagHeightStyle, SRFlag_Char, WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOPOLEPOSLEFT, IDC_RADIOPOLEPOSLEFT, &Active->PolePosition, SRFlag_Char, WCS_EFFECTS_LABEL_POLEPOSITION_LEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPOLEPOSLEFT, IDC_RADIOPOLEPOSCENTER, &Active->PolePosition, SRFlag_Char, WCS_EFFECTS_LABEL_POLEPOSITION_CENTER, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPOLEPOSLEFT, IDC_RADIOPOLEPOSRIGHT, &Active->PolePosition, SRFlag_Char, WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOPOLESTRAIGHT, IDC_RADIOPOLESTRAIGHT, &Active->PoleStyle, SRFlag_Char, WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPOLESTRAIGHT, IDC_RADIOPOLEANGLED, &Active->PoleStyle, SRFlag_Char, WCS_EFFECTS_LABEL_POLESTYLE_ANGLED, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOPOLEBASESQUARE, IDC_RADIOPOLEBASESQUARE, &Active->PoleBaseStyle, SRFlag_Char, WCS_EFFECTS_LABEL_POLEBASESTYLE_SQUARE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPOLEBASESQUARE, IDC_RADIOPOLEBASETAPERED, &Active->PoleBaseStyle, SRFlag_Char, WCS_EFFECTS_LABEL_POLEBASESTYLE_TAPERED, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOHIRESALWAYS, IDC_RADIOHIRESALWAYS, &Active->HiResFont, SRFlag_Char, WCS_EFFECTS_LABEL_HIRESFONT_ALWAYS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOHIRESALWAYS, IDC_RADIOHIRESSOMETIMES, &Active->HiResFont, SRFlag_Char, WCS_EFFECTS_LABEL_HIRESFONT_SOMETIMES, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOHIRESALWAYS, IDC_RADIOHIRESNEVER, &Active->HiResFont, SRFlag_Char, WCS_EFFECTS_LABEL_HIRESFONT_NEVER, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORCENTER, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_CENTER, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORLOWERLEFT, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERLEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORLEFTEDGE, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_LEFTEDGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORUPPERLEFT, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERLEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORTOPEDGE, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_TOPEDGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORUPPERRIGHT, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERRIGHT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORRIGHTEDGE, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_RIGHTEDGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORLOWERRIGHT, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERRIGHT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOANCHORCENTER, IDC_RADIOANCHORBOTTOMEDGE, &Active->AnchorPoint, SRFlag_Char, WCS_EFFECTS_LABEL_ANCHORPOINT_BOTTOMEDGE, NULL, NULL);

WidgetSetModified(IDC_OVERLAYTEXT, FALSE);
WidgetSetText(IDC_OVERLAYTEXT, Active->MesgText);

WidgetLBClear(IDC_INSERTLIST);
for(LabelInsert = 0; Label_TextLabels[LabelInsert]; LabelInsert++)
	{
	if(Label_TextSymbols[LabelInsert]) // safegaurd
		{
		sprintf(TempLabelFormat, "%s %s", Label_TextLabels[LabelInsert], Label_TextSymbols[LabelInsert]);
		WidgetLBInsert(IDC_INSERTLIST, -1, TempLabelFormat);
		} // if
	} // for

BuildAttributeList();
DisableWidgets();

} // LabelEditGUI::ConfigureWidgets()

/*===========================================================================*/

void LabelEditGUI::BuildAttributeList(void)
{
LayerEntry *Entry;
const char *LayerName;
long ItemCt;

WidgetLBClear(IDC_ATTRLIST);

Entry = DBHost->DBLayers.FirstEntry();
while (Entry)
	{
	if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		LayerName = Entry->GetName();
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL || LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
			{
			ItemCt = WidgetLBInsert(IDC_ATTRLIST, -1, &LayerName[1]);
			WidgetLBSetItemData(IDC_ATTRLIST, ItemCt, Entry);
			} // if an attribute layer
		} // if
	Entry = DBHost->DBLayers.NextEntry(Entry);
	} // while

} // LabelEditGUI::BuildAttributeList

/*===========================================================================*/

void LabelEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPREVIEWENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRENDEROCCLUDED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTEXTENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOUTLINEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLAGENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKBORDERENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPOLEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKWRAPENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPOLEFULLWIDTH, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPOLEFULLHEIGHT, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOVERHEADPOLE, WP_SCSYNC_NONOTIFY);

WidgetSNSync(IDC_BASEELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MASTERSIZE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_POLEHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_POLEWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXFLAGWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXFLAGHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BORDERWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTOUTLINEWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTLINEHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTLINESPACE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTLETTERSPACE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTTRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_OUTLINETRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FLAGTRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BORDERTRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_POLETRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEXTCOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_OUTLINECOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FLAGCOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BORDERCOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_POLECOLOR, WP_FISYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOABSOLUTE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEGRND, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEJOE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOJUSTIFYLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOJUSTIFYCENTER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOJUSTIFYRIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFLAGWIDTHFIXED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFLAGWIDTHFLOAT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFLAGWIDTHTEXTFLOAT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFLAGHEIGHTFIXED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFLAGHEIGHTFLOAT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEPOSLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEPOSCENTER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEPOSRIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLESTRAIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEANGLED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEBASESQUARE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOLEBASETAPERED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORCENTER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORLOWERLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORLEFTEDGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORUPPERLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORTOPEDGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORUPPERRIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORRIGHTEDGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOANCHORLOWERRIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOHIRESALWAYS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOHIRESSOMETIMES, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOHIRESNEVER, WP_SRSYNC_NONOTIFY);

} // LabelEditGUI::SyncWidgets

/*===========================================================================*/

void LabelEditGUI::DisableWidgets(void)
{

// angled pole only if tapered and position not centered
WidgetSetDisabled(IDC_RADIOPOLEANGLED, Active->PoleBaseStyle == WCS_EFFECTS_LABEL_POLEBASESTYLE_SQUARE || Active->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_CENTER);
// full width only if not full height
WidgetSetDisabled(IDC_CHECKPOLEFULLWIDTH, Active->PoleFullHeight || Active->PolePosition != WCS_EFFECTS_LABEL_POLEPOSITION_CENTER);
// full height only if not full width and not pole centered
WidgetSetDisabled(IDC_CHECKPOLEFULLHEIGHT, Active->PoleFullWidth || Active->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_CENTER);
// outline enabled only if text enabled
WidgetSetDisabled(IDC_CHECKOUTLINEENABLED, ! Active->TextEnabled);
// center pole position only if not pole full height
WidgetSetDisabled(IDC_RADIOPOLEPOSCENTER, Active->PoleFullHeight || Active->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_ANGLED);
// pole positions enabled only if not pole full width
WidgetSetDisabled(IDC_RADIOPOLEPOSLEFT, Active->PoleFullWidth);
WidgetSetDisabled(IDC_RADIOPOLEPOSRIGHT, Active->PoleFullWidth);
// square base only if not angled
WidgetSetDisabled(IDC_RADIOPOLEBASESQUARE, Active->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_ANGLED);

WidgetSetText(IDC_MAXFLAGWIDTH, Active->FlagWidthStyle != WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING ? "Text Width ": "Max Text Width ");
WidgetSetText(IDC_MAXFLAGHEIGHT, Active->FlagHeightStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FIXED ? "Text Height ": "Max Text Height ");

} // LabelEditGUI::DisableWidgets

/*===========================================================================*/

void LabelEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // LabelEditGUI::Cancel

/*===========================================================================*/

void LabelEditGUI::Name(void)
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

} // LabelEditGUI::Name()

/*===========================================================================*/

void LabelEditGUI::OverlayText(void)
{
char NewText[WCS_LABELTEXT_MAXLEN];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_OVERLAYTEXT))
	{
	WidgetGetText(IDC_OVERLAYTEXT, WCS_LABELTEXT_MAXLEN, NewText);
	WidgetSetModified(IDC_OVERLAYTEXT, FALSE);
	strcpy(Active->MesgText, NewText);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // LabelEditGUI::OverlayText()

/*===========================================================================*/

void LabelEditGUI::ScaleSizes(void)
{
char Str[64];
double Factor = 1.0, Test;

Str[0] = 0;

if (GetInputString("Enter a scale factor, percent or letter (such as \"h\" for half).", "", Str) && Str[0])
	{
	if (Str[strlen(Str) - 1] == '%')
		{
		Factor = atof(Str) / 100.0;
		if (Factor <= 0.0)
			{
			if (Factor <= -1.0 || Factor == 0.0)
				Factor = 1.0;
			else
				Factor = 1.0 + Factor;
			} // if
		} // if
	else
		{
		switch (Str[0])
			{
			case 'h':
			case 'H':
				{
				Factor = .5;
				break;
				} // half
			case 'q':
			case 'Q':
				{
				Factor = .25;
				break;
				} // half
			case 'd':
			case 'D':
				{
				Factor = 2.0;
				break;
				} // half
			case 't':
			case 'T':
				{
				if (Str[1] == 'h' || Str[1] == 'H')
					Factor = .33334;
				else
					Factor = 3.0;
				break;
				} // half
			case 'f':
				{
				Factor = 4.0;
				break;
				} // half
			default:
				{
				Test = atof(Str);
				if (Test > -100.0 && Test <= -1.0)
					{
					Factor = (100.0 + Test) / 100.0;
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > 0.0 && Test <= 10.0)
					{
					Factor = Test;
					} // if
				else if (Test > 10.0)
					{
					Factor = Test / 100.0;
					} // if
				break;
				} // default

			} // switch
		} // else

	// takes care of notifications on each Anim value
	Active->ScaleSizes(Factor);
	} // if

} // LabelEditGUI::ScaleSizes
