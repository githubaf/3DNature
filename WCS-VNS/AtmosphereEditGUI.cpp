// AtmosphereEditGUI.cpp
// Code for Atmosphere editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "AtmosphereEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "AppMem.h"
#include "resource.h"

char *AtmosphereEditGUI::TabNames[WCS_ATMOSPHEREGUI_NUMTABS] = {"General", "Haze && Fog"};
char *AtmosphereEditGUI::VolTabNames[3] = {"Profile", "Thickness", "Shadow"};

long AtmosphereEditGUI::ActivePage;
long AtmosphereEditGUI::ActiveSubPanel;
// advanced
long AtmosphereEditGUI::DisplayAdvanced;

NativeGUIWin AtmosphereEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // AtmosphereEditGUI::Open

/*===========================================================================*/

NativeGUIWin AtmosphereEditGUI::Construct(void)
{
int TabIndex;
char *ParticleTypes[] = {"Air", "Light Haze", "Medium Haze", "Heavy Haze", "Light Fog", "Medium Fog", 
						"Heavy Fog", "Ground Fog", "Dust", "Smog", "Smoke"};

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_SIMPLEHAZE, 0, 1);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_VOLUMETRIC, 0, 2);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_VOLUMETRIC_COLOR, 1, 0);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_VOLUMETRIC_MISC, 1, 1);
	CreateSubWinFromTemplate(IDD_ATMOSPHERE_VOLUMETRIC_SHADOW, 1, 2);

	if(NativeWin)
		{
		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
		for (TabIndex = 0; TabIndex < WCS_ATMOSPHEREGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		for (TabIndex = 0; TabIndex < 3; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB2, TabIndex, VolTabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetTCSetCurSel(IDC_TAB2, ActiveSubPanel);
		for (TabIndex = 0; TabIndex < 11; TabIndex ++)
			{
			WidgetCBInsert(IDC_PARTICLETYPEDROP, -1, ParticleTypes[TabIndex]);
			} // for
		WidgetCBSetCurSel(IDC_PARTICLETYPEDROP, -1);
		SelectPanel(ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // AtmosphereEditGUI::Construct

/*===========================================================================*/

AtmosphereEditGUI::AtmosphereEditGUI(EffectsLib *EffectsSource, Atmosphere *ActiveSource)
: GUIFenetre('ATMG', this, "Atmosphere Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0xff, 0xff, 0xff), 
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
ActiveComponent = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Atmosphere Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	ActiveComponent = Active->Components;
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // AtmosphereEditGUI::AtmosphereEditGUI

/*===========================================================================*/

AtmosphereEditGUI::~AtmosphereEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // AtmosphereEditGUI::~AtmosphereEditGUI()

/*===========================================================================*/

long AtmosphereEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_AEG, 0);

return(0);

} // AtmosphereEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long AtmosphereEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // AtmosphereEditGUI::HandleShowAdvanced

/*===========================================================================*/

long AtmosphereEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_AEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveComponent)
			Active->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveComponent)
			ActiveComponent->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDPARTICLE:
		{
		AddComponent();
		break;
		} // IDC_ADDPARTICLE
	case IDC_REMOVECOMPONENT:
		{
		RemoveComponent();
		break;
		} // IDC_REMOVECOMPONENT
	case IDC_EDITVERTPROFILE:
	case IDC_EDITVERTPROFILE2:
		{
		if (ActiveComponent)
			ActiveComponent->CovgProf.OpenTimeline();
		break;
		} // IDC_EDITVERTPROFILE
	case IDC_EDITDENSITYPROFILE:
	case IDC_EDITDENSITYPROFILE2:
		{
		if (ActiveComponent)
			ActiveComponent->DensityProf.OpenTimeline();
		break;
		} // IDC_EDITVERTPROFILE
	case IDC_EDITSHADEPROFILE:
	case IDC_EDITSHADEPROFILE2:
		{
		if (ActiveComponent)
			ActiveComponent->ShadeProf.OpenTimeline();
		break;
		} // IDC_EDITSHADEPROFILE
	default:
		break;
	} // ButtonID
return(0);

} // AtmosphereEditGUI::HandleButtonClick

/*===========================================================================*/

long AtmosphereEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARTICLETYPEDROP:
		{
		ParticleTypePreset();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // AtmosphereEditGUI::HandleCBChange

/*===========================================================================*/

long AtmosphereEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(043, 43);
switch (CtrlID)
	{
	case IDC_PARTICLELIST:
		{
		SetActiveComponent();
		break;
		} // IDC_PARTICLELIST
	default:
		break;
	} // switch CtrlID

return (0);

} // AtmosphereEditGUI::HandleListSel

/*===========================================================================*/

long AtmosphereEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_PARTICLELIST:
		{
		RemoveComponent();
		break;
		} // IDC_PARTICLELIST
	default:
		break;
	} // switch

return(0);

} // AtmosphereEditGUI::HandleListDelItem

/*===========================================================================*/

long AtmosphereEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_PARTICLENAME:
		{
		ComponentName();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // AtmosphereEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long AtmosphereEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				} // 0
			default:
				{
				SelectPanel(0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		ActivePage = NewPageID;
		break;
		}
	case IDC_TAB2:
		{
		ActiveSubPanel = NewPageID;
		SelectPanel(1);
		break;
		}
	default:
		break;
	} // switch

return(0);

} // AtmosphereEditGUI::HandlePageChange

/*===========================================================================*/

long AtmosphereEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKPARTICLEENABLED:
		{
		if (ActiveComponentValid())
			{
			Changes[0] = MAKE_ID(ActiveComponent->GetNotifyClass(), ActiveComponent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveComponent->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_CHECKHAZEENABLED:
	case IDC_CHECKCLOUDHAZEENABLED:
	case IDC_CHECKACTASFILTER:
	case IDC_CHECKFOGENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKCASTSHADOWS:
	case IDC_CHECKRECEIVESHADOWSTER:
	case IDC_CHECKRECEIVESHADOWSFOL:
	case IDC_CHECKRECEIVESHADOWS3D:
	case IDC_CHECKRECEIVESHADOWSCSM:
	case IDC_CHECKRECEIVESHADOWSVOL:
	case IDC_CHECKRECEIVESHADOWSMISC:
		{
		if (ActiveComponent)
			{
			Changes[0] = MAKE_ID(ActiveComponent->GetNotifyClass(), ActiveComponent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveComponent->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // AtmosphereEditGUI::HandleSCChange

/*===========================================================================*/

long AtmosphereEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOLINEAR:
	case IDC_RADIOSLOWINCREASE:
	case IDC_RADIOFASTINCREASE:
	case IDC_RADIOVOLUMETRIC:
	case IDC_RADIOEXPONENTIAL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOSPEEDBOOST1X:
	case IDC_RADIOSPEEDBOOST4X:
	case IDC_RADIOSPEEDBOOST9X:
	case IDC_RADIOSPEEDBOOST16X:
	case IDC_RADIOSPEEDBOOST25X:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOBEFOREREFL:
	case IDC_RADIOAFTERREFL:
		{
		if (ActiveComponent)
			{
			Changes[0] = MAKE_ID(ActiveComponent->GetNotifyClass(), ActiveComponent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveComponent->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // AtmosphereEditGUI::HandleSRChange

/*===========================================================================*/

void AtmosphereEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[8];
long Done = 0;

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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // AtmosphereEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void AtmosphereEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Atmosphere Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKHAZEENABLED, &Active->HazeEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLOUDHAZEENABLED, &Active->SeparateCloudHaze, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKACTASFILTER, &Active->ActAsFilter, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFOGENABLED, &Active->FogEnabled, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOLINEAR, &Active->AtmosphereType, SRFlag_Char, WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOEXPONENTIAL, &Active->AtmosphereType, SRFlag_Char, WCS_EFFECTS_ATMOSPHERETYPE_EXPONENTIAL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOSLOWINCREASE, &Active->AtmosphereType, SRFlag_Char, WCS_EFFECTS_ATMOSPHERETYPE_SLOWINCREASE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOFASTINCREASE, &Active->AtmosphereType, SRFlag_Char, WCS_EFFECTS_ATMOSPHERETYPE_FASTINCREASE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOVOLUMETRIC, &Active->AtmosphereType, SRFlag_Char, WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOSPEEDBOOST1X, IDC_RADIOSPEEDBOOST1X, &EffectsHost->AtmoBase.SpeedBoost, SRFlag_Char, WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_LOW, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSPEEDBOOST1X, IDC_RADIOSPEEDBOOST4X, &EffectsHost->AtmoBase.SpeedBoost, SRFlag_Char, WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUMLOW, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSPEEDBOOST1X, IDC_RADIOSPEEDBOOST9X, &EffectsHost->AtmoBase.SpeedBoost, SRFlag_Char, WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUM, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSPEEDBOOST1X, IDC_RADIOSPEEDBOOST16X, &EffectsHost->AtmoBase.SpeedBoost, SRFlag_Char, WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_MEDIUMHIGH, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSPEEDBOOST1X, IDC_RADIOSPEEDBOOST25X, &EffectsHost->AtmoBase.SpeedBoost, SRFlag_Char, WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_HIGH, NULL, NULL);

WidgetSmartRAHConfig(IDC_HAZESTART, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART], Active);
WidgetSmartRAHConfig(IDC_HAZERANGE, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE], Active);
WidgetSmartRAHConfig(IDC_HAZESTARTINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY], Active);
WidgetSmartRAHConfig(IDC_HAZEENDINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY], Active);
WidgetSmartRAHConfig(IDC_CLOUDHAZESTART, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART], Active);
WidgetSmartRAHConfig(IDC_CLOUDHAZERANGE, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE], Active);
WidgetSmartRAHConfig(IDC_CLOUDHAZESTARTINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY], Active);
WidgetSmartRAHConfig(IDC_CLOUDHAZEENDINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY], Active);
WidgetSmartRAHConfig(IDC_FOGNONE, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV], Active);
WidgetSmartRAHConfig(IDC_FOGFULL, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV], Active);
WidgetSmartRAHConfig(IDC_FOGLOWINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY], Active);
WidgetSmartRAHConfig(IDC_HIGHFOGINTENSITY, &Active->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY], Active);
WidgetSmartRAHConfig(IDC_COLOR1, &Active->TopAmbientColor, Active);
WidgetSmartRAHConfig(IDC_COLOR2, &Active->BottomAmbientColor, Active);
WidgetSmartRAHConfig(IDC_COLOR3, &Active->HazeColor, Active);
WidgetSmartRAHConfig(IDC_COLOR4, &Active->FogColor, Active);
WidgetSmartRAHConfig(IDC_COLOR6, &Active->CloudHazeColor, Active);

ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->AtmoBase.AdaptiveThreshold,
  1.0,
   0.0,
	100.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_ADDPARTICLE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVECOMPONENT, IDI_DELETE, NULL);

BuildComponentList();
ConfigureComponent();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // AtmosphereEditGUI::ConfigureWidgets()

/*===========================================================================*/

void AtmosphereEditGUI::BuildComponentList(void)
{
AtmosphereComponent **SelectedItems = NULL;
AtmosphereComponent *Current = Active->Components;
RasterAnimHost *CurrentRAHost = NULL;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

NumListItems = WidgetLBGetCount(IDC_PARTICLELIST);

ActiveComponentValid();

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_PARTICLELIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (AtmosphereComponent **)AppMem_Alloc(NumSelected * sizeof (AtmosphereComponent *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_PARTICLELIST, TempCt))
				{
				SelectedItems[SelCt ++] = (AtmosphereComponent *)WidgetLBGetItemData(IDC_PARTICLELIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_PARTICLELIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == (AtmosphereComponent *)CurrentRAHost)
			{
			BuildComponentListEntry(ListName, Current);
			WidgetLBReplace(IDC_PARTICLELIST, Ct, ListName);
			WidgetLBSetItemData(IDC_PARTICLELIST, Ct, Current);
			if (Current == ActiveComponent)
				WidgetLBSetSelState(IDC_PARTICLELIST, 1, Ct);
			else if (SelectedItems)
				{
				for (SelCt = 0; SelCt < NumSelected; SelCt ++)
					{
					if (SelectedItems[SelCt] == Current)
						{
						WidgetLBSetSelState(IDC_PARTICLELIST, 1, Ct);
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
				if (Current == (AtmosphereComponent *)WidgetLBGetItemData(IDC_PARTICLELIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildComponentListEntry(ListName, Current);
				WidgetLBReplace(IDC_PARTICLELIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_PARTICLELIST, TempCt, Current);
				if (Current == ActiveComponent)
					WidgetLBSetSelState(IDC_PARTICLELIST, 1, TempCt);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_PARTICLELIST, 1, TempCt);
							break;
							} // if
						} // for
					} // if
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_PARTICLELIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildComponentListEntry(ListName, Current);
				Place = WidgetLBInsert(IDC_PARTICLELIST, Ct, ListName);
				WidgetLBSetItemData(IDC_PARTICLELIST, Place, Current);
				if (Current == ActiveComponent)
					WidgetLBSetSelState(IDC_PARTICLELIST, 1, Place);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_PARTICLELIST, 1, Place);
							break;
							} // if
						} // for
					} // if
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_PARTICLELIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (AtmosphereComponent *));

} // AtmosphereEditGUI::BuildComponentList

/*===========================================================================*/

void AtmosphereEditGUI::BuildComponentListEntry(char *ListName, AtmosphereComponent *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->Name);

} // AtmosphereEditGUI::BuildComponentListEntry()

/*===========================================================================*/

AtmosphereComponent *AtmosphereEditGUI::ActiveComponentValid(void)
{
AtmosphereComponent *CurComp;

if (ActiveComponent)
	{
	CurComp = Active->Components;
	while (CurComp)
		{
		if (CurComp == ActiveComponent)
			{
			return (ActiveComponent);
			} // if
		CurComp = CurComp->Next;
		} // while
	} // if

return (ActiveComponent = Active->Components);

} // AtmosphereEditGUI::ActiveComponentValid

/*===========================================================================*/

void AtmosphereEditGUI::ConfigureComponent(void)
{

if (ActiveComponentValid() || (Active->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC && Active->AddComponent(NULL) && ActiveComponentValid()))
	{
	WidgetSetModified(IDC_PARTICLENAME, FALSE);
	WidgetSetText(IDC_PARTICLENAME, ActiveComponent->Name);

	ConfigureSC(NativeWin, IDC_CHECKPARTICLEENABLED, &ActiveComponent->Enabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKCASTSHADOWS, &ActiveComponent->CastShadows, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSTER, &ActiveComponent->ReceiveShadowsTerrain, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSFOL, &ActiveComponent->ReceiveShadowsFoliage, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS3D, &ActiveComponent->ReceiveShadows3DObject, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSCSM, &ActiveComponent->ReceiveShadowsCloudSM, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSVOL, &ActiveComponent->ReceiveShadowsVolumetric, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSMISC, &ActiveComponent->ReceiveShadowsMisc, SCFlag_Char, NULL, 0);

	ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOBEFOREREFL, &ActiveComponent->VolumeBeforeRefl, SRFlag_Char, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOAFTERREFL, &ActiveComponent->VolumeBeforeRefl, SRFlag_Char, 0, NULL, NULL);

	WidgetSmartRAHConfig(IDC_THICKNESS, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS], ActiveComponent);
	WidgetSmartRAHConfig(IDC_SHADOWINTENS, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_RECVSHADOWINTENS], ActiveComponent);
	WidgetSmartRAHConfig(IDC_DENSITYPTRN, (RasterAnimHost **)ActiveComponent->GetTexRootPtrAddr(WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE), ActiveComponent);
	WidgetSmartRAHConfig(IDC_SELFSHADE, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_SELFSHADING], ActiveComponent);
	WidgetSmartRAHConfig(IDC_COVERAGE, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_COVERAGE], ActiveComponent);
	WidgetSmartRAHConfig(IDC_DENSITY, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY], ActiveComponent);
	WidgetSmartRAHConfig(IDC_ALT, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV], ActiveComponent);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH], ActiveComponent);
	WidgetSmartRAHConfig(IDC_MINSAMPLE, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE], ActiveComponent);
	WidgetSmartRAHConfig(IDC_MAXSAMPLE, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE], ActiveComponent);
	WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT], ActiveComponent);
	WidgetSmartRAHConfig(IDC_TRANSLUMEXP, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTEXP], ActiveComponent);
	WidgetSmartRAHConfig(IDC_SPECCOLORPCT, &ActiveComponent->AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT], ActiveComponent);
	WidgetSmartRAHConfig(IDC_COLOR5, &ActiveComponent->ParticleColor, ActiveComponent);
	WidgetSmartRAHConfig(IDC_BACKLIGHTCOLOR, &ActiveComponent->BacklightColor, ActiveComponent);
	WidgetSetDisabled(IDC_PARTICLETYPEDROP, 0);
	} // if
else
	{
	WidgetSetText(IDC_PARTICLENAME, "");
	ConfigureSC(NativeWin, IDC_CHECKPARTICLEENABLED, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKCASTSHADOWS, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSTER, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSFOL, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS3D, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSCSM, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSVOL, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSMISC, NULL, 0, NULL, 0);
	ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOBEFOREREFL, NULL, 0, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOBEFOREREFL, IDC_RADIOAFTERREFL, NULL, 0, 0, NULL, NULL);
	WidgetSmartRAHConfig(IDC_DENSITYPTRN, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_THICKNESS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SHADOWINTENS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SELFSHADE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_COVERAGE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ALT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_MINSAMPLE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_MAXSAMPLE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSLUMEXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECCOLORPCT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_COLOR5, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BACKLIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSetDisabled(IDC_PARTICLETYPEDROP, 1);
	} // else

} // AtmosphereEditGUI::ConfigureComponent

/*===========================================================================*/

void AtmosphereEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HAZESTART, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HAZERANGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HAZESTARTINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HAZEENDINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CLOUDHAZESTART, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CLOUDHAZERANGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CLOUDHAZESTARTINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CLOUDHAZEENDINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOGNONE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOGFULL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOGLOWINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HIGHFOGINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR1, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR4, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR6, WP_FISYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOLINEAR, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSLOWINCREASE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFASTINCREASE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVOLUMETRIC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOEXPONENTIAL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPEEDBOOST1X, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPEEDBOOST4X, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPEEDBOOST9X, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPEEDBOOST16X, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSPEEDBOOST25X, WP_SRSYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKHAZEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCLOUDHAZEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKACTASFILTER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFOGENABLED, WP_SCSYNC_NONOTIFY);

if (ActiveComponentValid())
	{
	WidgetSNSync(IDC_DENSITYPTRN, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_THICKNESS, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SHADOWINTENS, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SELFSHADE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_COVERAGE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ALT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_MINSAMPLE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_MAXSAMPLE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSLUMINANCE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSLUMEXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECCOLORPCT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_COLOR5, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BACKLIGHTCOLOR, WP_FISYNC_NONOTIFY);

	WidgetSCSync(IDC_CHECKPARTICLEENABLED, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKCASTSHADOWS, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWSTER, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWSFOL, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWS3D, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWSCSM, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWSVOL, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKRECEIVESHADOWSMISC, WP_SCSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOBEFOREREFL, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOAFTERREFL, WP_SRSYNC_NONOTIFY);
	} // if

} // AtmosphereEditGUI::SyncWidgets

/*===========================================================================*/

void AtmosphereEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_HAZESTART, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_HAZERANGE, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_HAZESTARTINTENSITY, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_HAZEENDINTENSITY, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_COLOR3, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_FOGNONE, ! Active->FogEnabled);
WidgetSetDisabled(IDC_FOGFULL, ! Active->FogEnabled);
WidgetSetDisabled(IDC_FOGLOWINTENSITY, ! Active->FogEnabled);
WidgetSetDisabled(IDC_HIGHFOGINTENSITY, ! Active->FogEnabled);
WidgetSetDisabled(IDC_COLOR4, ! Active->FogEnabled);
WidgetSetDisabled(IDC_CHECKCLOUDHAZEENABLED, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_CHECKACTASFILTER, ! Active->HazeEnabled);
WidgetSetDisabled(IDC_CLOUDHAZESTART, ! (Active->HazeEnabled && Active->SeparateCloudHaze));
WidgetSetDisabled(IDC_CLOUDHAZERANGE, ! (Active->HazeEnabled && Active->SeparateCloudHaze));
WidgetSetDisabled(IDC_CLOUDHAZESTARTINTENSITY, ! (Active->HazeEnabled && Active->SeparateCloudHaze));
WidgetSetDisabled(IDC_CLOUDHAZEENDINTENSITY, ! (Active->HazeEnabled && Active->SeparateCloudHaze));
WidgetSetDisabled(IDC_COLOR6, ! (Active->HazeEnabled && Active->SeparateCloudHaze));

WidgetSetDisabled(IDC_RESOLUTION, EffectsHost->AtmoBase.SpeedBoost == WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_LOW);

} // AtmosphereEditGUI::DisableWidgets

/*===========================================================================*/

void AtmosphereEditGUI::SelectPanel(long PanelID)
{

switch(PanelID)
	{
	case 0:
		{
		ShowPanel(0, 0);
		ShowPanel(1, -1);
		break;
		} // 0
	case 1:
		{
		if (Active->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
			{
			ShowPanel(0, 2);
			ShowPanel(1, ActiveSubPanel);
			} // if
		else
			{
			ShowPanel(0, 1);
			ShowPanel(1, -1);
			} // else
		break;
		} // 1
	} // PanelID

} // AtmosphereEditGUI::SelectPanel

/*===========================================================================*/

// advanced
void AtmosphereEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_HIDDENCONTROLMSG5, false);
	WidgetShow(IDC_RATELABEL, true);
	WidgetShow(IDC_RADIOSPEEDBOOST1X, true);
	WidgetShow(IDC_RADIOSPEEDBOOST4X, true);
	WidgetShow(IDC_RADIOSPEEDBOOST9X, true);
	WidgetShow(IDC_RADIOSPEEDBOOST16X, true);
	WidgetShow(IDC_RADIOSPEEDBOOST25X, true);
	WidgetShow(IDC_RESOLUTION, true);
	WidgetShow(IDC_RADIOLINEAR, true);
	WidgetShow(IDC_RADIOEXPONENTIAL, true);
	WidgetShow(IDC_RADIOSLOWINCREASE, true);
	WidgetShow(IDC_RADIOFASTINCREASE, true);
	WidgetShow(IDC_RADIOVOLUMETRIC, true);
	WidgetShow(IDC_ATMOTYPE_TXT, true);
	WidgetShow(IDC_CHECKCLOUDHAZEENABLED, true);
	WidgetShow(IDC_COLOR6, true);
	WidgetShow(IDC_CLOUDHAZESTART, true);
	WidgetShow(IDC_CLOUDHAZERANGE, true);
	WidgetShow(IDC_CLOUDHAZESTARTINTENSITY, true);
	WidgetShow(IDC_CLOUDHAZEENDINTENSITY, true);
	WidgetShow(IDC_CHECKFOGENABLED, true);
	WidgetShow(IDC_COLOR4, true);
	WidgetShow(IDC_FOGNONE, true);
	WidgetShow(IDC_FOGFULL, true);
	WidgetShow(IDC_FOGLOWINTENSITY, true);
	WidgetShow(IDC_HIGHFOGINTENSITY, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_HIDDENCONTROLMSG5, true);
	WidgetShow(IDC_RATELABEL, false);
	WidgetShow(IDC_RADIOSPEEDBOOST1X, false);
	WidgetShow(IDC_RADIOSPEEDBOOST4X, false);
	WidgetShow(IDC_RADIOSPEEDBOOST9X, false);
	WidgetShow(IDC_RADIOSPEEDBOOST16X, false);
	WidgetShow(IDC_RADIOSPEEDBOOST25X, false);
	WidgetShow(IDC_RESOLUTION, false);
	WidgetShow(IDC_RADIOLINEAR, false);
	WidgetShow(IDC_RADIOEXPONENTIAL, false);
	WidgetShow(IDC_RADIOSLOWINCREASE, false);
	WidgetShow(IDC_RADIOFASTINCREASE, false);
	WidgetShow(IDC_RADIOVOLUMETRIC, false);
	WidgetShow(IDC_ATMOTYPE_TXT, false);
	WidgetShow(IDC_CHECKCLOUDHAZEENABLED, false);
	WidgetShow(IDC_COLOR6, false);
	WidgetShow(IDC_CLOUDHAZESTART, false);
	WidgetShow(IDC_CLOUDHAZERANGE, false);
	WidgetShow(IDC_CLOUDHAZESTARTINTENSITY, false);
	WidgetShow(IDC_CLOUDHAZEENDINTENSITY, false);
	WidgetShow(IDC_CHECKFOGENABLED, false);
	WidgetShow(IDC_COLOR4, false);
	WidgetShow(IDC_FOGNONE, false);
	WidgetShow(IDC_FOGFULL, false);
	WidgetShow(IDC_FOGLOWINTENSITY, false);
	WidgetShow(IDC_HIGHFOGINTENSITY, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // AtmosphereEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void AtmosphereEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // AtmosphereEditGUI::Cancel

/*===========================================================================*/

void AtmosphereEditGUI::Name(void)
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

} // AtmosphereEditGUI::Name()

/*===========================================================================*/

void AtmosphereEditGUI::ComponentName(void)
{
char Name[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (ActiveComponentValid())
	{
	if (WidgetGetModified(IDC_PARTICLENAME))
		{
		WidgetGetText(IDC_PARTICLENAME, WCS_EFFECT_MAXNAMELENGTH, Name);
		WidgetSetModified(IDC_PARTICLENAME, FALSE);
		strncpy(ActiveComponent->Name, Name, WCS_EFFECT_MAXNAMELENGTH);
		ActiveComponent->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
		Changes[0] = MAKE_ID(ActiveComponent->GetNotifyClass(), ActiveComponent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveComponent->GetRAHostRoot());
		} // if 
	} // if

} // AtmosphereEditGUI::ComponentName()

/*===========================================================================*/

void AtmosphereEditGUI::AddComponent(void)
{

ActiveComponent = Active->AddComponent(NULL);
BuildComponentList();
ConfigureComponent();

} // AtmosphereEditGUI::AddComponent

/*===========================================================================*/

void AtmosphereEditGUI::RemoveComponent(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_PARTICLELIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_PARTICLELIST, Ct))
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
				if (WidgetLBGetSelState(IDC_PARTICLELIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_PARTICLELIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll);
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Constituent", "There are no Constituents selected to remove.");
		} // else
	} // if

} // AtmosphereEditGUI::RemoveComponent

/*===========================================================================*/

void AtmosphereEditGUI::SetActiveComponent(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_PARTICLELIST);
Current = (long)WidgetLBGetItemData(IDC_PARTICLELIST, Current);
if (Current != LB_ERR)
	ActiveComponent = (AtmosphereComponent *)Current;
ConfigureComponent();

} // AtmosphereEditGUI::SetActiveComponent()

/*===========================================================================*/

void AtmosphereEditGUI::ParticleTypePreset(void)
{
long Current;
char ParticleTypeStr[64];
float MaxElev, MinElev;

if (ActiveComponent)
	{
	if ((Current = WidgetCBGetCurSel(IDC_PARTICLETYPEDROP)) != CB_ERR)
		{
		if (WidgetCBGetText(IDC_PARTICLETYPEDROP, Current, ParticleTypeStr) != CB_ERR)
			{
			GlobalApp->AppDB->GetDEMElevRange(MaxElev, MinElev);
			ActiveComponent->SetDefaultProperties(ParticleTypeStr, (double)MinElev);
			} // if
		} // if
	} // if

} // AtmosphereEditGUI::ParticleTypePreset

/*===========================================================================*/

bool AtmosphereEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->AtmosphereType != WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE ||
	Active->SeparateCloudHaze || Active->FogEnabled ? true : false);

} // AtmosphereEditGUI::QueryLocalDisplayAdvancedUIVisibleState
