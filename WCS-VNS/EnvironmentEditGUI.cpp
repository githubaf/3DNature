// EnvironmentEditGUI.cpp
// Code for Environment editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EnvironmentEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

char *EnvironmentEditGUI::TabNames[WCS_ENVIRONMENTGUI_NUMTABS] = {"General", "Ecosystems", "Foliage && Gradients"};

long EnvironmentEditGUI::ActivePage;
// advanced
long EnvironmentEditGUI::DisplayAdvanced;

NativeGUIWin EnvironmentEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // EnvironmentEditGUI::Open

/*===========================================================================*/

NativeGUIWin EnvironmentEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_ENVIRONMENT_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_ENVIRONMENT_ECOSYSTEMS, 0, 1);
	CreateSubWinFromTemplate(IDD_ENVIRONMENT_BASIC, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_ENVIRONMENTGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // EnvironmentEditGUI::Construct

/*===========================================================================*/

EnvironmentEditGUI::EnvironmentEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, EnvironmentEffect *ActiveSource)
: GUIFenetre('ENVI', this, "Environment Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
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
ProjHost = ProjSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Environment Editor - %s", Active->GetName());
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

} // EnvironmentEditGUI::EnvironmentEditGUI

/*===========================================================================*/

EnvironmentEditGUI::~EnvironmentEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // EnvironmentEditGUI::~EnvironmentEditGUI()

/*===========================================================================*/

long EnvironmentEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_ENG, 0);

return(0);

} // EnvironmentEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long EnvironmentEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // EnvironmentEditGUI::HandleShowAdvanced

/*===========================================================================*/

long EnvironmentEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_ENG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITPROFILE:
		{
		Active->ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
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
		Active->SortEcosystems();
		break;
		} // IDC_GRABALL
	case IDC_SORT:
		{
		Active->SortEcosystems();
		break;
		} // IDC_SORT
	case IDC_SCALERULES:
		{
		// <<<>>>
		// analyze the ranges of terrain elevations, slopes and relel
		// and scale the ranges of ecosystem rules to those in the DEM
		break;
		} // IDC_SCALERULES
	default:
		break;
	} // ButtonID

return(0);

} // EnvironmentEditGUI::HandleButtonClick

/*===========================================================================*/

long EnvironmentEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
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

} // EnvironmentEditGUI::HandleListDelItem

/*===========================================================================*/

long EnvironmentEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // EnvironmentEditGUI::HandleListDoubleClick

/*===========================================================================*/

long EnvironmentEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // EnvironmentEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long EnvironmentEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
		ActivePage = NewPageID;
		break;
		}
	default:
		break;
	} // switch

return(0);

} // EnvironmentEditGUI::HandlePageChange

/*===========================================================================*/

long EnvironmentEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKGRADENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->EnvironmentBase.SetFloating(EffectsHost->EnvironmentBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // EnvironmentEditGUI::HandleSCChange

/*===========================================================================*/

long EnvironmentEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
	{
	//case IDC_RESOLUTION:
	//	{
	//	EffectsHost->EnvironmentBase.SetFloating(0, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	} // switch CtrlID

return(0);

} // EnvironmentEditGUI::HandleFIChange

/*===========================================================================*/

void EnvironmentEditGUI::HandleNotifyEvent(void)
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

#ifdef WCS_BUILD_VNS
// query drop
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// In this editor there is a message sent with 0xff for the class to notify S@G to completely redraw
	// when an ecosystem is moved up or down. Need to be aware of that and not set the done flag here when it 
	// is received.
	Interested[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Interested[1] = NULL;
	if (! GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		WidgetLWSync(IDC_VECLINKAGE);
		Done = 1;
		} // if
	} // if query changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // EnvironmentEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void EnvironmentEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->EnvironmentBase.AreThereEdges((GeneralEffect *)EffectsHost->Environment))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->EnvironmentBase.AreThereGradients((GeneralEffect *)EffectsHost->Environment))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->EnvironmentBase.Resolution,
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

sprintf(TextStr, "Environment Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->EnvironmentBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->EnvironmentBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADENABLED, &Active->GlobalGradientsEnabled, SCFlag_Char, NULL, 0);

WidgetSmartRAHConfig(IDC_FOLIAGEHTFACTOR, &Active->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT], Active);
WidgetSmartRAHConfig(IDC_ENVIRONMENT_ECOGRAD, &Active->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALECOGRAD], Active);
WidgetSmartRAHConfig(IDC_ENVIRONMENT_REFLAT, &Active->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT], Active);

ConfigureFI(NativeWin, IDC_DISTFOLINTENS, &Active->FoliageMinSize, 1.0, 0.0, 1000.0, FIOFlag_Short, NULL, 0);
ConfigureFI(NativeWin, IDC_FOLIAGEBLEND, &Active->FoliageBlending, .1, 0.0, 10.0, FIOFlag_Double, NULL, 0);

ConfigureTB(NativeWin, IDC_ADDECO, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEECO, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEECOUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEECODOWN, IDI_ARROWDOWN, NULL);

BuildEcoList();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // EnvironmentEditGUI::ConfigureWidgets()

/*===========================================================================*/

void EnvironmentEditGUI::BuildEcoList(void)
{
EffectList *Current = Active->Ecosystems;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_ECOLIST);

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
				if (SelectedItems)
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
					if (SelectedItems)
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
					if (SelectedItems)
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

} // EnvironmentEditGUI::BuildEcoList

/*===========================================================================*/

void EnvironmentEditGUI::BuildEcoListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // EnvironmentEditGUI::BuildEcoListEntry()

/*===========================================================================*/

void EnvironmentEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKHIRESEDGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);

//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOLIAGEHTFACTOR, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ENVIRONMENT_ECOGRAD, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ENVIRONMENT_REFLAT, WP_FISYNC_NONOTIFY);

} // EnvironmentEditGUI::SyncWidgets

/*===========================================================================*/

void EnvironmentEditGUI::DisableWidgets(void)
{

// env base
WidgetSetDisabled(IDC_COMPAREDROP, ! EffectsHost->EnvironmentBase.OverlapOK);

// global gradient
WidgetSetDisabled(IDC_ENVIRONMENT_ECOGRAD, ! Active->GlobalGradientsEnabled);
WidgetSetDisabled(IDC_ENVIRONMENT_REFLAT, ! Active->GlobalGradientsEnabled);

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);

} // EnvironmentEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void EnvironmentEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_CHECKGRADENABLED, true);
	WidgetShow(IDC_ENVIRONMENT_ECOGRAD, true);
	WidgetShow(IDC_ENVIRONMENT_REFLAT, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_CHECKGRADENABLED, false);
	WidgetShow(IDC_ENVIRONMENT_ECOGRAD, false);
	WidgetShow(IDC_ENVIRONMENT_REFLAT, false);
	} // else

// foliage controls are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced )
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_DISTFOLINTENS, true);
	WidgetShow(IDC_FOLIAGEBLEND, true);
	WidgetShow(IDC_FOLIAGEHTFACTOR, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_DISTFOLINTENS, true);
	WidgetShow(IDC_FOLIAGEBLEND, false);
	WidgetShow(IDC_FOLIAGEHTFACTOR, false);
	} // else
	
SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // EnvironmentEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void EnvironmentEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // EnvironmentEditGUI::Cancel

/*===========================================================================*/

void EnvironmentEditGUI::AddEco(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_ECOSYSTEM);

} // EnvironmentEditGUI::AddEco

/*===========================================================================*/

void EnvironmentEditGUI::RemoveEco(void)
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

} // EnvironmentEditGUI::RemoveEco

/*===========================================================================*/

void EnvironmentEditGUI::EditEco(void)
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

} // EnvironmentEditGUI::EditEco

/*===========================================================================*/

void EnvironmentEditGUI::Name(void)
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

} // EnvironmentEditGUI::Name()

/*===========================================================================*/

void EnvironmentEditGUI::ChangeEcoListPosition(short MoveUp)
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
					Current = Active->Ecosystems;
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
					Current = Active->Ecosystems;
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

} // EnvironmentEditGUI::ChangeEcoListPosition

/*===========================================================================*/

