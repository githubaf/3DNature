// GroundEditGUI.cpp
// Code for Ground editor
// Built from EcosystemEditGUI on 7/20/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "GroundEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Database.h"
#include "Conservatory.h"
#include "GalleryGUI.h"
#include "BrowseDataGUI.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


char *GroundEditGUI::TabNames[WCS_GROUNDGUI_NUMTABS] = {"General", "Material"};

long GroundEditGUI::ActivePage;
// advanced
long GroundEditGUI::DisplayAdvanced;

// material GUI
#define WCS_GROUNDED_MATGRADSET	1

NativeGUIWin GroundEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // GroundEditGUI::Open

/*===========================================================================*/

NativeGUIWin GroundEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_GROUND_GENERAL_VNS3, 0, 0);
	CreateSubWinFromTemplate(IDD_GROUND_MATERIAL_VNS3, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_GROUNDGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// material GUI
		MatGUI->Construct(WCS_GROUNDED_MATGRADSET, WCS_GROUNDED_MATGRADSET + 1);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // GroundEditGUI::Construct

/*===========================================================================*/

GroundEditGUI::GroundEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, GroundEffect *ActiveSource)
: GUIFenetre('GRND', this, "Ground Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_GROUND, 0xff, 0xff, 0xff), 
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
Active = ActiveSource;
ActiveGrad = NULL;
// Material GUI
MatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Ground Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// material GUI
	if (MatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->EcoMat, WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER, WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER)) // init ordinal 0
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

} // GroundEditGUI::GroundEditGUI

/*===========================================================================*/

GroundEditGUI::~GroundEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (MatGUI)
	delete MatGUI;

} // GroundEditGUI::~GroundEditGUI()

/*===========================================================================*/

long GroundEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_GNG, 0);

return(0);

} // GroundEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long GroundEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // GroundEditGUI::HandleShowAdvanced

/*===========================================================================*/

long GroundEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_GNG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_EDITPROFILE:
		{
		Active->ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
	// material GUI
	case IDC_POPDROP0:
		{
		if(WidgetGetCheck(IDC_POPDROP0))
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

} // GroundEditGUI::HandleButtonClick

/*===========================================================================*/

long GroundEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // GroundEditGUI::HandleCBChange

/*===========================================================================*/

long GroundEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

// Material GUI
MatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);

return (0);

} // GroundEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long GroundEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // GroundEditGUI::HandlePageChange

/*===========================================================================*/

long GroundEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	//case IDC_CHECKOVERLAP:
	//case IDC_CHECKHIRESEDGE:
	case IDC_CHECKGRADFILL:
	case IDC_CHECKTRANSPARENT:
	case IDC_CHECKMATCHECO:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->GroundBase.SetFloating(EffectsHost->GroundBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // GroundEditGUI::HandleSCChange

/*===========================================================================*/

long GroundEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// Material GUI
MatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // GroundEditGUI::HandleFIChange

/*===========================================================================*/

void GroundEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;

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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	ConfigureColors();
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), Active->EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureMaterial();
	ConfigureColors();
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
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

} // GroundEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void GroundEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->GroundBase.AreThereEdges((GeneralEffect *)EffectsHost->Ground))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->GroundBase.AreThereGradients((GeneralEffect *)EffectsHost->Ground))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->GroundBase.Resolution,
  1.0,
   0.00001,
	1000000.0,
	 FIOFlag_Float,
	  NULL,
	   NULL);*/

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

sprintf(TextStr, "Ground Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->GroundBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->GroundBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTRANSPARENT, &Active->Transparent, SCFlag_Short, NULL, 0);

WidgetAGConfig(IDC_ANIMGRADIENT2, &Active->EcoMat);
// Material GUI
MatGUI->ConfigureWidgets();

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
// material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);

ConfigureMaterial();
ConfigureColors();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // GroundEditGUI::ConfigureWidgets()

/*===========================================================================*/

void GroundEditGUI::ConfigureMaterial(void)
{
char GroupWithMatName[200];
MaterialEffect *Mat;

if ((ActiveGrad = Active->EcoMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_INTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)&Mat->Strata, Mat);

	sprintf(GroupWithMatName, "Selected Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS, GroupWithMatName);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)NULL, NULL);

	WidgetSetText(IDC_MATERIALS, "Selected Material");
	} // else
// material GUI
MatGUI->ConfigureMaterial();

} // GroundEditGUI::ConfigureMaterial

/*===========================================================================*/

void GroundEditGUI::ConfigureColors(void)
{

// this is harmless to call even if there is no active gradient node, it will cause 
// a valid node to be set if there is one..
WidgetAGSync(IDC_ANIMGRADIENT2);
// material GUI
MatGUI->SyncWidgets();
ActiveGrad = Active->EcoMat.GetActiveNode();

} // GroundEditGUI::ConfigureColors

/*===========================================================================*/

void GroundEditGUI::SyncWidgets(void)
{

if (Active->EcoMat.GetActiveNode() != ActiveGrad)
	{
	ConfigureWidgets();
	return;
	} // if

//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKHIRESEDGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTRANSPARENT, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);

if (ActiveGrad = Active->EcoMat.GetActiveNode())
	{
	WidgetSNSync(IDC_LUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_STRATA, WP_FISYNC_NONOTIFY);
	} // if

// Material GUI
MatGUI->SyncWidgets();

} // GroundEditGUI::SyncWidgets

/*===========================================================================*/

void GroundEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);

// Material GUI
MatGUI->DisableWidgets();

} // GroundEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void GroundEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

// All the material properties are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_ANIMGRADIENT2, ! MatGUI->QueryIsDisplayed());
	// material GUI
	WidgetShow(IDC_POPDROP0, true);
	} 
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_ANIMGRADIENT2, false);
	// material GUI
	WidgetShow(IDC_POPDROP0, false);
	ShowMaterialPopDrop(false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // GroundEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void GroundEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // GroundEditGUI::Cancel

/*===========================================================================*/

void GroundEditGUI::Name(void)
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

} // GroundEditGUI::Name()

/*===========================================================================*/

void GroundEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{
if(Host) Host->ConfigureMaterial();
} // GroundEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

void GroundEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{
if(Host) Host->SetNewActiveGrad(NewNode);
} // GroundEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// material GUI
void GroundEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if(ShowState)
	{
	// position and show
	ShowPanelAsPopDrop(IDC_POPDROP0, MatGUI->GetPanel(), 0, SubPanels[0][1]);
	WidgetShow(IDC_ANIMGRADIENT2, 0); // hide master gradient widget since it looks weird otherwise
	WidgetSetCheck(IDC_POPDROP0, true);
	} // if
else
	{
	ShowPanel(MatGUI->GetPanel(), -1); // hide
	WidgetShow(IDC_ANIMGRADIENT2, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
	WidgetSetCheck(IDC_POPDROP0, false);
	} // else

} // GroundEditGUI::ShowMaterialPopDrop

/*===========================================================================*/

bool GroundEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->EcoMat.CountNodes() > 1 ? true : false);

} // GroundEditGUI::QueryLocalDisplayAdvancedUIVisibleState

