// LightEditGUI.cpp
// Code for Light editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "LightEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "AtmosphereEditGUI.h"
#include "Conservatory.h"
#include "SunPosGUI.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"

char *LightEditGUI::TabNames[WCS_LIGHTGUI_NUMTABS] = {"General", "Position && Orientation", "Color && Shadow", "Include"};

long LightEditGUI::ActivePage;
// advanced
long LightEditGUI::DisplayAdvanced;

NativeGUIWin LightEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // LightEditGUI::Open

/*===========================================================================*/

NativeGUIWin LightEditGUI::Construct(void)
{
int TabIndex;
GeneralEffect *MyEffect;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_LIGHT_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_LIGHT_POSITION, 0, 1);
	CreateSubWinFromTemplate(IDD_LIGHT_SHADOWS, 0, 2);
	CreateSubWinFromTemplate(IDD_LIGHT_INCLUDE, 0, 3);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_LIGHTGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_TARGETDROP, -1, "New 3D Object...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_TARGETDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_TARGETDROP, TabIndex, MyEffect);
			} // for
		for (TabIndex = 0; TabIndex < WCS_PRESETS_NUMCELESTIALS; TabIndex ++)
			{
			WidgetCBInsert(IDC_ELEVATIONDROP, -1, EffectsLib::CelestialPresetName[TabIndex]);
			WidgetCBInsert(IDC_RADIUSDROP, -1, EffectsLib::CelestialPresetName[TabIndex]);
			} // for
		WidgetCBSetCurSel(IDC_ELEVATIONDROP, -1);
		WidgetCBSetCurSel(IDC_RADIUSDROP, -1);
		FillClassDrop();
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // LightEditGUI::Construct

/*===========================================================================*/

LightEditGUI::LightEditGUI(EffectsLib *EffectsSource, Light *ActiveSource)
: GUIFenetre('SUNP', this, "Light Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_CELESTIAL, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
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
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Light Editor - %s", Active->GetName());
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

} // LightEditGUI::LightEditGUI

/*===========================================================================*/

LightEditGUI::~LightEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // LightEditGUI::~LightEditGUI()

/*===========================================================================*/

long LightEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_NPG, 0);

return(0);

} // LightEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long LightEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // LightEditGUI::HandleShowAdvanced

/*===========================================================================*/

long LightEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_NPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDITEM:
		{
		AddItem();
		break;
		} // IDC_ADDITEM
	case IDC_REMOVEITEM:
		{
		RemoveItem();
		break;
		} // IDC_REMOVEITEM
	case IDC_FREETARGET:
		{
		FreeTarget();
		break;
		} // IDC_FREETARGET
	case IDC_SETPOSBYTIME:
		{
		SetPosByTime();
		break;
		} // IDC_SETPOSBYTIME
	case IDC_EDITAMBIENT:
		{
		Atmosphere *CurAtmo;

		AtmosphereEditGUI::SetActivePage(0);
		if (CurAtmo = (Atmosphere *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 1, NULL))
			CurAtmo->EditRAHost();
		break;
		} // IDC_EDITAMBIENT
	case ID_COLORPOT1:
		{
		Active->Color.EditRAHost();
		break;
		} // ID_COLORPOT1
	default:
		break;
	} // ButtonID

return(0);

} // LightEditGUI::HandleButtonClick

/*===========================================================================*/

long LightEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
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
	case IDC_TARGETDROP:
		{
		SelectNewTarget();
		break;
		}
	default:
		break;
	} // switch CtrlID

if (Active->Floating)
	Active->SetFloating(0);

return (0);

} // LightEditGUI::HandleCBChange

/*===========================================================================*/

long LightEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_ITEMLIST:
		{
		RemoveItem();
		break;
		} // IDC_ITEMLIST
	default:
		break;
	} // switch

return(0);

} // LightEditGUI::HandleListDelItem

/*===========================================================================*/

long LightEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ITEMLIST:
		{
		EditItem();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // LightEditGUI::HandleListDoubleClick

/*===========================================================================*/

long LightEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // LightEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long LightEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
			case 3:
				{
				ShowPanel(0, 3);
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

} // LightEditGUI::HandlePageChange

/*===========================================================================*/

long LightEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
int FloatStash;

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKCASTSHADOW:
	case IDC_CHECKAASHADOWS:
	case IDC_CHECKSOFTSHADOWS:
	case IDC_CHECKATMOSPHERE:
	case IDC_CHECKDISTANT:
	case IDC_CHECKFLIPTREES:
	case IDC_CHECKINCLUDEENABLED:
		{
		if (Active->Floating)
			Active->SetFloating(0);		// this sends the valuechanged message
		else
			{
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // else
		break;
		} // 
	case IDC_CHECKFLOATING:
		{
		FloatStash = Active->Floating;
		Active->SetFloating(Active->Floating);		// this sends the valuechanged message
		if (Active->Floating != FloatStash)
			WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // LightEditGUI::HandleSCChange

/*===========================================================================*/

long LightEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOOMNI:
	case IDC_RADIOSPOT:
	case IDC_RADIOPARALLEL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOINCLUDE:
	case IDC_RADIOEXCLUDE:
		{
		Changes[0] = MAKE_ID(Active->InclExcl.GetNotifyClass(), Active->InclExcl.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->InclExcl.GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

if (Active->Floating)
	Active->SetFloating(0);

return(0);

} // LightEditGUI::HandleSRChange

/*===========================================================================*/

void LightEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[8];
long Pos, CurPos, Done = 0;
GeneralEffect *MyEffect, *MatchEffect;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[6] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[7] = NULL;
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

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = Active->TargetObj;
	WidgetCBClear(IDC_TARGETDROP);
	WidgetCBInsert(IDC_TARGETDROP, -1, "New 3D Object...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_TARGETDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_TARGETDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_TARGETDROP, CurPos);
	Done = 1;
	} // if image name changed

if (! Done)
	ConfigureWidgets();

} // LightEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void LightEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
long ListPos, Ct, NumCelest = 0, NumEntries;
CelestialEffect *CurCelest;
Object3DEffect *TestObj;
EffectList *CurLight;

sprintf(TextStr, "Light Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

if (CurCelest = (CelestialEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_CELESTIAL))
	{
	while (CurCelest)
		{
		if (CurLight = CurCelest->Lights)
			{
			while (CurLight)
				{
				if (CurLight->Me == (GeneralEffect *)Active)
					NumCelest ++;
				CurLight = CurLight->Next;
				} // while
			} // if
		CurCelest = (CelestialEffect *)CurCelest->Next;
		} // while
	} // if
if (NumCelest > 1)
	sprintf(TextStr, "There are %d Celestial Objects attached to this Light.", NumCelest);
else if (NumCelest == 1)
	strcpy(TextStr, "There is one Celestial Object attached to this Light.");
else
	strcpy(TextStr, "There are no Celestial Objects attached to this Light.");
WidgetSetText(IDC_CELESTSEXIST, TextStr);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCASTSHADOW, &Active->CastShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKAASHADOWS, &Active->AAShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSOFTSHADOWS, &Active->SoftShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, &Active->IllumAtmosphere, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDISTANT, &Active->Distant, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLOATING, &Active->Floating, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLIPTREES, &Active->FlipFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKINCLUDEENABLED, &Active->InclExcl.Enabled, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOOMNI, IDC_RADIOOMNI, &Active->LightType, SRFlag_Char, WCS_EFFECTS_LIGHTTYPE_OMNI, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOOMNI, IDC_RADIOPARALLEL, &Active->LightType, SRFlag_Char, WCS_EFFECTS_LIGHTTYPE_PARALLEL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOOMNI, IDC_RADIOSPOT, &Active->LightType, SRFlag_Char, WCS_EFFECTS_LIGHTTYPE_SPOT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOINCLUDE, &Active->InclExcl.Include, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOINCLUDE, IDC_RADIOEXCLUDE, &Active->InclExcl.Include, SRFlag_Char, 0, NULL, NULL);

WidgetSmartRAHConfig(IDC_LIGHTLAT, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], Active);
WidgetSmartRAHConfig(IDC_LIGHTLON, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], Active);
WidgetSmartRAHConfig(IDC_LIGHTELEV, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], Active);
WidgetSmartRAHConfig(IDC_HEADING, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING], Active);
WidgetSmartRAHConfig(IDC_PITCH, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH], Active);
WidgetSmartRAHConfig(IDC_FALLOFFEXP, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP], Active);
WidgetSmartRAHConfig(IDC_CONEANGLE, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE], Active);
WidgetSmartRAHConfig(IDC_SOFTEDGE, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE], Active);
WidgetSmartRAHConfig(IDC_LIGHTRADIUS, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS], Active);
WidgetSmartRAHConfig(IDC_LIGHTCOLOR, &Active->Color, Active);

ConfigureTB(NativeWin, IDC_ADDITEM, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEITEM, IDI_DELETE, NULL);

if (Active->TargetObj)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_TARGETDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (Object3DEffect *)WidgetCBGetItemData(IDC_TARGETDROP, Ct)) != (Object3DEffect *)LB_ERR && TestObj == Active->TargetObj)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_TARGETDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_TARGETDROP, -1);

for (Ct = 0; Ct < WCS_PRESETS_NUMCELESTIALS; Ct ++)
	{
	if (Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue == EffectsLib::CelestialPresetDistance[Ct])
		WidgetCBSetCurSel(IDC_ELEVATIONDROP, Ct);
	if (Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].CurValue == EffectsLib::CelestialPresetRadius[Ct])
		WidgetCBSetCurSel(IDC_RADIUSDROP, Ct);
	} // for

BuildItemList();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // LightEditGUI::ConfigureWidgets()

/*===========================================================================*/

void LightEditGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_LIGHTLAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LIGHTLON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LIGHTELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HEADING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PITCH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FALLOFFEXP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CONEANGLE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOFTEDGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LIGHTRADIUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LIGHTCOLOR, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCASTSHADOW, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKAASHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSOFTSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATMOSPHERE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDISTANT, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLIPTREES, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKINCLUDEENABLED, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOOMNI, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPARALLEL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPOT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOINCLUDE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOEXCLUDE, WP_SRSYNC_NONOTIFY);

} // LightEditGUI::SyncWidgets

/*===========================================================================*/

void LightEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_HEADING, Active->Distant || Active->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI);
WidgetSetDisabled(IDC_PITCH, Active->Distant || Active->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI);
WidgetSetDisabled(IDC_TARGETDROP, Active->Distant || Active->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI);
WidgetSetDisabled(IDC_CHECKAASHADOWS, ! Active->CastShadows || Active->SoftShadows);
WidgetSetDisabled(IDC_CHECKSOFTSHADOWS, ! Active->CastShadows);
WidgetSetDisabled(IDC_CHECKFLIPTREES, ! Active->CastShadows);
WidgetSetDisabled(IDC_LIGHTRADIUS, ! Active->CastShadows || ! Active->SoftShadows);
WidgetSetDisabled(IDC_RADIUSDROP, ! Active->CastShadows || ! Active->SoftShadows);

WidgetSetDisabled(IDC_FALLOFFEXP, Active->Distant);
WidgetSetDisabled(IDC_CONEANGLE, Active->LightType != WCS_EFFECTS_LIGHTTYPE_SPOT);
WidgetSetDisabled(IDC_SOFTEDGE, Active->LightType != WCS_EFFECTS_LIGHTTYPE_SPOT);

WidgetSetDisabled(IDC_RADIOINCLUDE, ! Active->InclExcl.Enabled);
WidgetSetDisabled(IDC_RADIOEXCLUDE, ! Active->InclExcl.Enabled);

} // LightEditGUI::DisableWidgets

/*===========================================================================*/

void LightEditGUI::BuildItemList(void)
{
RasterAnimHostList *Current = Active->InclExcl.RAHList;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 64];
RasterAnimHost *CurrentRAHost = NULL;
RasterAnimHost **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_ITEMLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (RasterAnimHost *)CurrentRAHost)
				{
				BuildItemListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_ITEMLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_ITEMLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_ITEMLIST, 1, Ct);
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
					if (Current->Me == (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildItemListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_ITEMLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_ITEMLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ITEMLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_ITEMLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildItemListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_ITEMLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_ITEMLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_ITEMLIST, 1, Place);
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
		WidgetLBDelete(IDC_ITEMLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (RasterAnimHost *));

} // LightEditGUI::BuildItemList

/*===========================================================================*/

void LightEditGUI::BuildItemListEntry(char *ListName, RasterAnimHost *Me)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_FLAGS;
Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;

Me->GetRAHostProperties(&Prop);

if (Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Prop.Name);
strcat(ListName, " ");
strcat(ListName, Prop.Type);

} // LightEditGUI::BuildItemListEntry()

/*===========================================================================*/

void LightEditGUI::FillClassDrop(void)
{
long Pos, MyClass;

for (MyClass = WCS_EFFECTSSUBCLASS_LAKE; MyClass < WCS_MAXIMPLEMENTED_EFFECTS; MyClass ++)
	{
	if (Active->ApproveInclExclClass(MyClass))
		{
		Pos = WidgetCBInsert(IDC_CLASSDROP, -1, EffectsHost->GetEffectTypeNameNonPlural(MyClass));
		WidgetCBSetItemData(IDC_CLASSDROP, Pos, (void *)MyClass);
		} // if
	} // for
WidgetCBSetCurSel(IDC_CLASSDROP, 0);

} // LightEditGUI::FillClassDrop

/*===========================================================================*/

void LightEditGUI::AddItem(void)
{
long AddClass, Current;

Current = WidgetCBGetCurSel(IDC_CLASSDROP);
if ((AddClass = (long)WidgetCBGetItemData(IDC_CLASSDROP, Current, 0)) != (long)LB_ERR)
	EffectsHost->AddAttributeByList(&Active->InclExcl, AddClass);

} // LightEditGUI::AddItem

/*===========================================================================*/

void LightEditGUI::RemoveItem(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_ITEMLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					if (Active->InclExcl.FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
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
		UserMessageOK("Remove Item", "There are no items selected to remove.");
		} // else
	} // if

BuildItemList();

} // LightEditGUI::RemoveItem

/*===========================================================================*/

void LightEditGUI::EditItem(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_ITEMLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_ITEMLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_ITEMLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // LightEditGUI::EditItem

/*===========================================================================*/

// advanced
void LightEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_ORIENT_TXT, true);
	WidgetShow(IDC_HEADING, true);
	WidgetShow(IDC_PITCH, true);
	WidgetShow(IDC_TARGOBJ_TXT, true);
	WidgetShow(IDC_TARGETDROP, true);
	WidgetShow(IDC_FREETARGET, true);
	WidgetShow(IDC_RADIOOMNI, true);
	WidgetShow(IDC_RADIOSPOT, true);
	WidgetShow(IDC_RADIOPARALLEL, true);
	WidgetShow(IDC_CHECKDISTANT, true);
	WidgetShow(IDC_FALLOFFEXP, true);
	WidgetShow(IDC_CONEANGLE, true);
	WidgetShow(IDC_SOFTEDGE, true);
	WidgetShow(IDC_CHECKINCLUDEENABLED, true);
	WidgetShow(IDC_RADIOINCLUDE, true);
	WidgetShow(IDC_RADIOEXCLUDE, true);
	WidgetShow(IDC_ITEMLIST, true);
	WidgetShow(IDC_ADDITEM, true);
	WidgetShow(IDC_REMOVEITEM, true);
	WidgetShow(IDC_CLASSDROP, true);
	WidgetShow(IDC_ADDITEM_TXT, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_ORIENT_TXT, false);
	WidgetShow(IDC_HEADING, false);
	WidgetShow(IDC_PITCH, false);
	WidgetShow(IDC_TARGOBJ_TXT, false);
	WidgetShow(IDC_TARGETDROP, false);
	WidgetShow(IDC_FREETARGET, false);
	WidgetShow(IDC_RADIOOMNI, false);
	WidgetShow(IDC_RADIOSPOT, false);
	WidgetShow(IDC_RADIOPARALLEL, false);
	WidgetShow(IDC_CHECKDISTANT, false);
	WidgetShow(IDC_FALLOFFEXP, false);
	WidgetShow(IDC_CONEANGLE, false);
	WidgetShow(IDC_SOFTEDGE, false);
	WidgetShow(IDC_CHECKINCLUDEENABLED, false);
	WidgetShow(IDC_RADIOINCLUDE, false);
	WidgetShow(IDC_RADIOEXCLUDE, false);
	WidgetShow(IDC_ITEMLIST, false);
	WidgetShow(IDC_ADDITEM, false);
	WidgetShow(IDC_REMOVEITEM, false);
	WidgetShow(IDC_CLASSDROP, false);
	WidgetShow(IDC_ADDITEM_TXT, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // LightEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void LightEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // LightEditGUI::Cancel

/*===========================================================================*/

void LightEditGUI::Name(void)
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

} // LightEditGUI::Name()

/*===========================================================================*/

void LightEditGUI::SelectNewTarget(void)
{
Object3DEffect *OldObj, *NewObj, *MadeObj = NULL;
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_TARGETDROP);
if (((NewObj = (Object3DEffect *)WidgetCBGetItemData(IDC_TARGETDROP, Current, 0)) != (Object3DEffect *)LB_ERR && NewObj)
	|| (MadeObj = NewObj = (Object3DEffect *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, NULL, NULL)))
	{
	OldObj = Active->TargetObj;
	if (MadeObj)
		{
		if (! MadeObj->OpenInputFileRequest())
			{
			Changes[0] = MAKE_ID(MadeObj->GetNotifyClass(), MadeObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			EffectsHost->RemoveEffect(MadeObj);
			MadeObj = NULL;
			NewObj = OldObj;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		} // if
	Active->SetTarget(NewObj);
	} // if

} // LightEditGUI::SelectNewTarget

/*===========================================================================*/

void LightEditGUI::FreeTarget(void)
{

Active->RemoveRAHost(Active->TargetObj);

} // LightEditGUI::FreeTarget

/*===========================================================================*/

void LightEditGUI::ElevationPreset(void)
{
long Current;

if ((Current = WidgetCBGetCurSel(IDC_ELEVATIONDROP)) != CB_ERR && Current < WCS_PRESETS_NUMCELESTIALS)
	{
	Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetCurValue(EffectsLib::CelestialPresetDistance[Current]);
	} // if

} // LightEditGUI::ElevationPreset

/*===========================================================================*/

void LightEditGUI::RadiusPreset(void)
{
long Current;

if ((Current = WidgetCBGetCurSel(IDC_RADIUSDROP)) != CB_ERR && Current < WCS_PRESETS_NUMCELESTIALS)
	{
	Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetCurValue(EffectsLib::CelestialPresetRadius[Current]);
	} // if

} // LightEditGUI::RadiusPreset

/*===========================================================================*/

void LightEditGUI::SetPosByTime(void)
{

if(GlobalApp->GUIWins->SPG)
	{
	delete GlobalApp->GUIWins->SPG;
	}
GlobalApp->GUIWins->SPG = new SunPosGUI(GlobalApp->MainProj, Active);
if(GlobalApp->GUIWins->SPG)
	{
	GlobalApp->GUIWins->SPG->Open(GlobalApp->MainProj);
	}

} // LightEditGUI::SetPosByTime

/*===========================================================================*/

bool LightEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->LightType != WCS_EFFECTS_LIGHTTYPE_PARALLEL || ! Active->Distant ||
	Active->TargetObj || Active->InclExcl.Enabled || Active->InclExcl.RAHList ? true : false);

} // LightEditGUI::QueryLocalDisplayAdvancedUIVisibleState

