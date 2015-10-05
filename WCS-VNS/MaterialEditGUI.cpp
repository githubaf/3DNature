// MaterialEditGUI.cpp
// Code for Material editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "MaterialEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "resource.h"

char *MaterialEditGUI::TabNames[WCS_MATERIALGUI_NUMTABS] = {"General", "Properties 1", "Properties 2"};

long MaterialEditGUI::ActivePage;
// advanced
long MaterialEditGUI::DisplayAdvanced;

NativeGUIWin MaterialEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // MaterialEditGUI::Open

/*===========================================================================*/

NativeGUIWin MaterialEditGUI::Construct(void)
{
char *ShadeType[] = {"Invisible", "Flat", "Phong"};
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_MATERIAL_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_MATERIAL_PROPERTIES, 0, 1);
	CreateSubWinFromTemplate(IDD_MATERIAL_PROPERTIES2, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_MATERIALGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for (TabIndex = 0; TabIndex < 3; TabIndex ++)
			{
			WidgetCBInsert(IDC_SHADEDROP, TabIndex, ShadeType[TabIndex]);
			} // for
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // MaterialEditGUI::Construct

/*===========================================================================*/

MaterialEditGUI::MaterialEditGUI(EffectsLib *EffectsSource, MaterialEffect *ActiveSource)
: GUIFenetre('MATE', this, "Material Editor"), Backup(WCS_EFFECTS_MATERIALTYPE_OBJECT3D), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
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
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Material Editor - %s", Active->GetName());
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

} // MaterialEditGUI::MaterialEditGUI

/*===========================================================================*/

MaterialEditGUI::~MaterialEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // MaterialEditGUI::~MaterialEditGUI()

/*===========================================================================*/

long MaterialEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_MAG, 0);

return(0);

} // MaterialEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long MaterialEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // MaterialEditGUI::HandleShowAdvanced

/*===========================================================================*/

long MaterialEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_MAG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // MaterialEditGUI::HandleButtonClick

/*===========================================================================*/

long MaterialEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SHADEDROP:
		{
		SelectShade();
		break;
		} // IDC_SHADEDROP
	default:
		break;
	} // switch CtrlID

return (0);

} // MaterialEditGUI::HandleCBChange

/*===========================================================================*/

long MaterialEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // MaterialEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long MaterialEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				} // 2
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

} // MaterialEditGUI::HandlePageChange

/*===========================================================================*/

long MaterialEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKFLIPNORMALS:
	case IDC_CHECKDOUBLESIDED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // MaterialEditGUI::HandleSCChange

/*===========================================================================*/

long MaterialEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_SMOOTHANGLE:
		{
		Active->CosSmoothAngle = cos(Active->SmoothAngle);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // MaterialEditGUI::HandleFIChange

/*===========================================================================*/

void MaterialEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;

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
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureNumObjects();
	DisableWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // MaterialEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void MaterialEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

ConfigureNumObjects();

sprintf(TextStr, "Material Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLIPNORMALS, &Active->FlipNormal, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDOUBLESIDED, &Active->DoubleSided, SCFlag_Short, NULL, 0);

ConfigureFI(NativeWin, IDC_SMOOTHANGLE,
 &Active->SmoothAngle,
  1.0,
   0.0,
	180.0,
	 FIOFlag_Double,
	  NULL,
	   NULL);

WidgetSmartRAHConfig(IDC_LUMINOSITY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Active);
WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Active);
WidgetSmartRAHConfig(IDC_SPECULARITY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Active);
WidgetSmartRAHConfig(IDC_SPECULAREXP, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Active);
WidgetSmartRAHConfig(IDC_SPECULARITY2, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2], Active);
WidgetSmartRAHConfig(IDC_SPECULAREXP2, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2], Active);
WidgetSmartRAHConfig(IDC_SPECCOLORPCT, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT], Active);
WidgetSmartRAHConfig(IDC_SPECCOLORPCT2, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2], Active);
WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE], Active);
WidgetSmartRAHConfig(IDC_TRANSLUMEXP, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP], Active);
WidgetSmartRAHConfig(IDC_REFLECTIVITY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Active);
WidgetSmartRAHConfig(IDC_INTENSITY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Active);
WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Active->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Active);
WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Active);
WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Active->DiffuseColor, Active);
WidgetSmartRAHConfig(IDC_SPECULARCOLOR, &Active->SpecularColor, Active);
WidgetSmartRAHConfig(IDC_SPECULARCOLOR2, &Active->SpecularColor2, Active);

WidgetCBSetCurSel(IDC_SHADEDROP, Active->Shading);

DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // MaterialEditGUI::ConfigureWidgets()

/*===========================================================================*/

void MaterialEditGUI::ConfigureNumObjects(void)
{
Object3DEffect *CurObj;
long Ct, NumObjs = 0;
char TextStr[64];

if (CurObj = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D))
	{
	while (CurObj)
		{
		if (CurObj->NameTable)
			{
			for (Ct = 0; Ct < CurObj->NumMaterials; Ct ++)
				{
				if (! strcmp(CurObj->NameTable[Ct].Name, Active->Name))
					{
					NumObjs ++;
					break;
					} // if
				} // if
			} // if
		CurObj = (Object3DEffect *)CurObj->Next;
		} // while
	} // if
if (NumObjs > 1)
	sprintf(TextStr, "There are %d 3D Objects using this Material.", NumObjs);
else if (NumObjs == 1)
	strcpy(TextStr, "There is one 3D Object using this Material.");
else
	strcpy(TextStr, "There are no 3D Objects using this Material.");
WidgetSetText(IDC_OBJSEXIST, TextStr);

} // MaterialEditGUI::ConfigureNumObjects()

/*===========================================================================*/

void MaterialEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLIPNORMALS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDOUBLESIDED, WP_SCSYNC_NONOTIFY);

WidgetFISync(IDC_SMOOTHANGLE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LUMINOSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULARITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULAREXP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULARITY2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULAREXP2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECCOLORPCT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECCOLORPCT2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSLUMINANCE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TRANSLUMEXP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_REFLECTIVITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BUMP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DIFFUSECOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULARCOLOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SPECULARCOLOR2, WP_FISYNC_NONOTIFY);

WidgetCBSetCurSel(IDC_SHADEDROP, Active->Shading);

DisableWidgets();

} // MaterialEditGUI::SyncWidgets()

/*===========================================================================*/

void MaterialEditGUI::DisableWidgets(void)
{

} // MaterialEditGUI::DisableWidgets()

/*===========================================================================*/

// advanced
void MaterialEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_HIDDENCONTROLMSG5, false);
	WidgetShow(IDC_SMOOTHANGLE, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_TRANSLUMINANCE, true);
	WidgetShow(IDC_TRANSLUMEXP, true);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_SPECCOLORPCT, true);
	WidgetShow(IDC_SPECULARITY2, true);
	WidgetShow(IDC_SPECULAREXP2, true);
	WidgetShow(IDC_SPECCOLORPCT2, true);
	WidgetShow(IDC_SPECULARCOLOR, true);
	WidgetShow(IDC_SPECULARCOLOR2, true);
	WidgetShow(IDC_ALTSPEC_TXT, true);
	WidgetShow(IDC_ALTSPEC_TXT2, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_HIDDENCONTROLMSG5, true);
	WidgetShow(IDC_SMOOTHANGLE, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_TRANSLUMINANCE, false);
	WidgetShow(IDC_TRANSLUMEXP, false);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_SPECCOLORPCT, false);
	WidgetShow(IDC_SPECULARITY2, false);
	WidgetShow(IDC_SPECULAREXP2, false);
	WidgetShow(IDC_SPECCOLORPCT2, false);
	WidgetShow(IDC_SPECULARCOLOR, false);
	WidgetShow(IDC_SPECULARCOLOR2, false);
	WidgetShow(IDC_ALTSPEC_TXT, false);
	WidgetShow(IDC_ALTSPEC_TXT2, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // MaterialEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void MaterialEditGUI::Cancel(void)
{
NotifyTag Changes[2];
char TempName[WCS_EFFECT_MAXNAMELENGTH];

strcpy(TempName, Active->Name);
Active->Copy(Active, &Backup);
EffectsHost->MaterialNameChanging(TempName, Active->Name);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // MaterialEditGUI::Cancel

/*===========================================================================*/

void MaterialEditGUI::Name(void)
{
NotifyTag Changes[2];
char NewName[WCS_EFFECT_MAXNAMELENGTH], TempName[WCS_EFFECT_MAXNAMELENGTH];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	strcpy(TempName, Active->Name);
	Active->SetUniqueName(EffectsHost, NewName);
	EffectsHost->MaterialNameChanging(TempName, Active->Name);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // MaterialEditGUI::Name

/*===========================================================================*/

void MaterialEditGUI::SelectShade(void)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_SHADEDROP);
Active->Shading = (short)Current;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // MaterialEditGUI::SelectShade

/*===========================================================================*/
