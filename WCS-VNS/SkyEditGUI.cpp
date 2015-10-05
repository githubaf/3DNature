// SkyEditGUI.cpp
// Code for Sky editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "SkyEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Raster.h"
#include "Texture.h"
#include "resource.h"

char *SkyEditGUI::TabNames[WCS_SKYGUI_NUMTABS] = {"General", "Color Gradients"};

long SkyEditGUI::ActivePage;
// advanced
long SkyEditGUI::DisplayAdvanced;

// Material GUI
#define WCS_SKYED_SKY_MATGRADSET	1
#define WCS_SKYED_LIGHT_MATGRADSET	3

NativeGUIWin SkyEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SkyEditGUI::Open

/*===========================================================================*/

NativeGUIWin SkyEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_SKY_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_SKY_COLORS, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_SKYGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// Material GUI
		SkyMatGUI->Construct(WCS_SKYED_SKY_MATGRADSET, WCS_SKYED_SKY_MATGRADSET + 1);
		LightMatGUI->Construct(WCS_SKYED_LIGHT_MATGRADSET, WCS_SKYED_LIGHT_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		UpdateThumbnail();
		} // if
	} // if
 
return (NativeWin);

} // SkyEditGUI::Construct

/*===========================================================================*/

SkyEditGUI::SkyEditGUI(EffectsLib *EffectsSource, Sky *ActiveSource)
: GUIFenetre('SKYP', this, "Sky Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_SKY, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
Rast = NULL;
SampleData = NULL;
BackgroundInstalled = 0;
ActiveSkyGrad = ActiveLightGrad = NULL;
ActiveSkyNode = ActiveLightNode = NULL;
// Material GUI
SkyMatGUI = NULL;
LightMatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Sky Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// Material GUI
	if (SkyMatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->SkyGrad)) // init ordinal 0
		{
		PopDropSkyMaterialNotifier.Host = this; // to be able to call notifications later
		SkyMatGUI->SetNotifyFunctor(&PopDropSkyMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	// Material GUI
	if (LightMatGUI = new PortableMaterialGUI(1, this, EffectsSource, Active, &Active->LightGrad)) // init ordinal 1
		{
		PopDropLightMaterialNotifier.Host = this; // to be able to call notifications later
		LightMatGUI->SetNotifyFunctor(&PopDropLightMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	if (! (Rast = new Raster()))
		ConstructError = 1;
	if (! (SampleData = new TextureSampleData()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // SkyEditGUI::SkyEditGUI

/*===========================================================================*/

SkyEditGUI::~SkyEditGUI()
{

GlobalApp->RemoveBGHandler(this);

if (Rast)
	delete Rast;
if (SampleData)
	delete SampleData;
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (SkyMatGUI)
	delete SkyMatGUI;
if (LightMatGUI)
	delete LightMatGUI;

} // SkyEditGUI::~SkyEditGUI()

/*===========================================================================*/

long SkyEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_KPG, 0);

return(0);

} // SkyEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long SkyEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // SkyEditGUI::HandleShowAdvanced

/*===========================================================================*/

long SkyEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_KPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	// Material GUI
	case IDC_POPDROP0:
		{
		if(WidgetGetCheck(IDC_POPDROP0))
			{
			ShowSkyMaterialPopDrop(true);			} // if
		else
			{
			ShowSkyMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	case IDC_POPDROP1:
		{
		if(WidgetGetCheck(IDC_POPDROP1))
			{
			ShowLightMaterialPopDrop(true);			} // if
		else
			{
			ShowLightMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP1
	default:
		break;
	} // ButtonID

// Material GUI
SkyMatGUI->HandleButtonClick(Handle, NW, ButtonID);
LightMatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // SkyEditGUI::HandleButtonClick

/*===========================================================================*/

long SkyEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // SkyEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long SkyEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
SkyMatGUI->HandleCBChange(Handle, NW, CtrlID);
LightMatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // SkyEditGUI::HandleCBChange

/*===========================================================================*/

long SkyEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// Material GUI
ShowSkyMaterialPopDrop(false);
ShowLightMaterialPopDrop(false);

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

} // SkyEditGUI::HandlePageChange

/*===========================================================================*/

long SkyEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	} // switch CtrlID

return(0);

} // SkyEditGUI::HandleSCChange

/*===========================================================================*/

long SkyEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
SkyMatGUI->HandleFIChange(Handle, NW, CtrlID);
LightMatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // SkyEditGUI::HandleFIChange

/*===========================================================================*/

void SkyEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[12];
long UpdateThumbs = 0, Done = 0;

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
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), Active->SkyGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[6] = MAKE_ID(Active->GetNotifyClass(), Active->SkyGrad.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[7] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[8] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[9] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[10] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[11] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureColors();
	SyncWidgets();
	UpdateThumbs = 1;
	Done = 1;
	} // if color name changed

if (! Done)
	{
	ConfigureWidgets();
	UpdateThumbnail();
	} // if
else if (UpdateThumbs)
	UpdateThumbnail();

} // SkyEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void SkyEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Sky Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);

WidgetSmartRAHConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY], Active);
WidgetSmartRAHConfig(IDC_DITHERING, &Active->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER], Active);

WidgetAGConfig(IDC_ANIMGRADIENT, &Active->SkyGrad);
WidgetAGConfig(IDC_ANIMGRADIENT2, &Active->LightGrad);
// Material GUI
SkyMatGUI->ConfigureWidgets();
LightMatGUI->ConfigureWidgets();

// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_POPDROP1, IDI_EXPAND, IDI_CONTRACT);

ConfigureColors();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // SkyEditGUI::ConfigureWidgets()

/*===========================================================================*/

void SkyEditGUI::ConfigureColors(void)
{

WidgetAGSync(IDC_ANIMGRADIENT);
WidgetAGSync(IDC_ANIMGRADIENT2);

if ((ActiveSkyGrad = Active->SkyGrad.GetActiveNode()) && (ActiveSkyNode = (ColorTextureThing *)ActiveSkyGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_COLOR1, &ActiveSkyNode->Color, ActiveSkyNode);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_COLOR1, (RasterAnimHost *)NULL, NULL);
	} // else
if ((ActiveLightGrad = Active->LightGrad.GetActiveNode()) && (ActiveLightNode = (ColorTextureThing *)ActiveLightGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_COLOR2, &ActiveLightNode->Color, ActiveLightNode);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_COLOR2, (RasterAnimHost *)NULL, NULL);
	} // else

// Material GUI
SkyMatGUI->ConfigureMaterial();
LightMatGUI->ConfigureMaterial();

} // SkyEditGUI::ConfigureColors

/*===========================================================================*/

void SkyEditGUI::SyncWidgets(void)
{

if ((Active->SkyGrad.GetActiveNode() != ActiveSkyGrad)
	|| (Active->LightGrad.GetActiveNode() != ActiveLightGrad))
	{
	ConfigureWidgets();
	return;
	} // if

WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DITHERING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR1, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_COLOR2, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);

// Material GUI
SkyMatGUI->SyncWidgets();
LightMatGUI->SyncWidgets();

} // SkyEditGUI::SyncWidgets

/*===========================================================================*/

void SkyEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_COLOR1, ! ActiveSkyGrad);
WidgetSetDisabled(IDC_COLOR1, ! ActiveLightGrad);

// Material GUI
SkyMatGUI->DisableWidgets();
LightMatGUI->DisableWidgets();

} // SkyEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void SkyEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced || Active->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue != 1.0)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_INTENSITY, true);
	WidgetShow(IDC_DITHERING, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_INTENSITY, false);
	WidgetShow(IDC_DITHERING, false);
	} // else


SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // SkyEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void SkyEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // SkyEditGUI::Cancel

/*===========================================================================*/

void SkyEditGUI::Name(void)
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

} // SkyEditGUI::Name()

/*===========================================================================*/

long SkyEditGUI::HandleBackgroundCrunch(int Siblings)
{
int Handled = 0;

if (SampleData->Running)
	{
	if (Active->EvalOneSampleLine(SampleData) || ! (SampleData->y % 5))
		ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, Rast);
	Handled = 1;
	} // if

if (! Handled)
	{
	// we're all done
	BackgroundInstalled = 0;
	return (1);		// 1 uninstalls background process
	} // if

// we've got more to do, come again when you have more time
return (0);

} // SkyEditGUI::HandleBackgroundCrunch

/*===========================================================================*/

void SkyEditGUI::UpdateThumbnail(void)
{

if (Active->EvalSampleInit(Rast, SampleData))
	{
	if (! BackgroundInstalled)
		{
		if (GlobalApp->AddBGHandler(this))
			BackgroundInstalled = 1;
		} // if
	} // if initialized

} // SkyEditGUI::UpdateThumbnail

/*===========================================================================*/

// Material GUI
void SkyEditGUISkyPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureColors();
} // SkyEditGUISkyPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void SkyEditGUISkyPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveSkyGrad(NewNode);
} // SkyEditGUISkyPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void SkyEditGUI::ShowSkyMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, SkyMatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(SkyMatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT, true);
	//WidgetShow(IDC_ANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // SkyEditGUI::ShowSkyMaterialPopDrop

/*===========================================================================*/

// Material GUI
void SkyEditGUILightPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureColors();
} // SkyEditGUILightPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void SkyEditGUILightPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveLightGrad(NewNode);
} // SkyEditGUILightPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void SkyEditGUI::ShowLightMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP1, LightMatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT2, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP1, true);
	} // if
else
	{
	ShowPanel(LightMatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT2, true);
	//WidgetShow(IDC_ANIMGRADIENT2, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP1, false);
	} // else

} // SkyEditGUI::ShowLightMaterialPopDrop

/*===========================================================================*/

