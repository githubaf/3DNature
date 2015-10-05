// ScenarioEditGUI.cpp
// Code for Scenario Editor
// Created from scratch on 9/11/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ScenarioEditGUI.h"
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

char *ScenarioEditGUI::TabNames[WCS_SCENARIOGUI_NUMTABS] = {"General", "Control", "State"};

long ScenarioEditGUI::ActivePage;
// advanced
long ScenarioEditGUI::DisplayAdvanced;

NativeGUIWin ScenarioEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ScenarioEditGUI::Open

/*===========================================================================*/

NativeGUIWin ScenarioEditGUI::Construct(void)
{
int TabIndex, Ct;
long ItemNumbers[] = {
	WCS_RAHOST_OBJTYPE_VECTOR,
	WCS_RAHOST_OBJTYPE_RASTER,
	WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_RASTERTA,
	WCS_EFFECTSSUBCLASS_ATMOSPHERE,
	WCS_EFFECTSSUBCLASS_CELESTIAL,
	WCS_EFFECTSSUBCLASS_CLOUD,
	WCS_EFFECTSSUBCLASS_CMAP,
	WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	WCS_EFFECTSSUBCLASS_FOLIAGE,
	WCS_EFFECTSSUBCLASS_GROUND,
	WCS_EFFECTSSUBCLASS_LABEL,
	WCS_EFFECTSSUBCLASS_LAKE,
	WCS_EFFECTSSUBCLASS_LIGHT,
	WCS_EFFECTSSUBCLASS_PLANETOPT,
	WCS_EFFECTSSUBCLASS_POSTPROC,
	WCS_EFFECTSSUBCLASS_SEARCHQUERY,
	WCS_EFFECTSSUBCLASS_SHADOW,
	WCS_EFFECTSSUBCLASS_SKY,
	WCS_EFFECTSSUBCLASS_SNOW,
	WCS_EFFECTSSUBCLASS_STARFIELD,
	WCS_EFFECTSSUBCLASS_STREAM,
	WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	WCS_EFFECTSSUBCLASS_TERRAINPARAM,
	WCS_EFFECTSSUBCLASS_THEMATICMAP,
	WCS_EFFECTSSUBCLASS_FENCE,
	WCS_EFFECTSSUBCLASS_WAVE,
	0};

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_SCENARIO_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_SCENARIO_ITEM, 0, 1);
	CreateSubWinFromTemplate(IDD_SCENARIO_STATE, 0, 2);

	WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
	WidgetSetScrollRange(IDC_SCROLLBAR2, 1, 100);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_SCENARIOGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);

		TabIndex = WidgetCBInsert(IDC_CLASSDROP, -1, "Database Object");
		WidgetCBSetItemData(IDC_CLASSDROP, TabIndex, (void *)ItemNumbers[0]);

		TabIndex = WidgetCBInsert(IDC_CLASSDROP, -1, "Image Object");
		WidgetCBSetItemData(IDC_CLASSDROP, TabIndex, (void *)ItemNumbers[1]);

		Ct = 2;
		while (ItemNumbers[Ct] > 0)
			{
			TabIndex = WidgetCBInsert(IDC_CLASSDROP, -1, EffectsHost->GetEffectTypeNameNonPlural(ItemNumbers[Ct]));
			WidgetCBSetItemData(IDC_CLASSDROP, TabIndex, (void *)ItemNumbers[Ct]);
			Ct ++;
			} // for
		WidgetCBSetCurSel(IDC_CLASSDROP, 0);

		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // ScenarioEditGUI::Construct

/*===========================================================================*/

ScenarioEditGUI::ScenarioEditGUI(EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource, RenderScenario *ActiveSource)
: GUIFenetre('SCEN', this, "Render Scenario Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, 0xff), 
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];
double RangeDefaults[3] = {FLT_MAX, 0.0, 1.0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
Active = ActiveSource;

DistanceADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
DistanceADT.SetMultiplier(1.0);
DistanceADT.SetRangeDefaults(RangeDefaults);
DistanceADT.SetIncrement(1.0 / GlobalApp->MainProj->Interactive->GetFrameRate());

memset(&WidgetLocal, 0, sizeof WidgetLocal);
WidgetLocal.GrWin = this;
WidgetLocal.Crit = &Active->MasterControl;
WidgetLocal.ValuePrototype = NULL;	// setting these to NULL will mean no value or distance labels in graph
WidgetLocal.DistancePrototype = &DistanceADT;
WidgetLocal.ActiveNode = WidgetLocal.Crit->GetFirstSelectedNode(0);
WidgetLocal.drawgrid = 1;
WidgetLocal.NumGraphs = WidgetLocal.Crit->GetNumGraphs();
WidgetLocal.HPan = 0;
WidgetLocal.HVisible = 100;
WidgetLocal.VPan = 0;
WidgetLocal.VVisible = 100;
WidgetLocal.DistGridLg = 5;
WidgetLocal.SnapToInt = 0;
WidgetLocal.DisplayByFrame = (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES);
WidgetLocal.MaxDistRange = 1.0;
WidgetLocal.LowDrawDist = 0.0;
WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;
WidgetLocal.MinLowDrawVal = -0.0000001;
WidgetLocal.MaxHighDrawVal = 1.0000001;
WidgetLocal.FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Render Scenario Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	BasicOnOff = Active->MasterControl.GetValue(0, 0.0) > .5;
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // ScenarioEditGUI::ScenarioEditGUI

/*===========================================================================*/

ScenarioEditGUI::~ScenarioEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ScenarioEditGUI::~ScenarioEditGUI()

/*===========================================================================*/

long ScenarioEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SCN, 0);

return(0);

} // ScenarioEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long ScenarioEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // ScenarioEditGUI::HandleShowAdvanced

/*===========================================================================*/

long ScenarioEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SCN, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_SELECTNOW:
		{
		Actionate();
		break;
		} // IDC_SELECTNOW
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
	case IDC_GRABALL:
		{
		GrabAll();
		break;
		} // IDC_GRABALL
	case IDC_ADDNODE:
		{
		AddNode();
		break;
		} // 
	case IDC_DELETENODE:
		{
		RemoveNode();
		break;
		} // 
	case IDC_SELECTALL:
		{
		SelectAllNodes();
		break;
		} // 
	case IDC_SELECTCLEAR:
		{
		ClearSelectNodes();
		break;
		} // 
	case IDC_SELECTTOGGLE:
		{
		ToggleSelectNodes();
		break;
		} // 
	case IDC_COPYCONTROL:
		{
		char CopyMsg[256];
		RasterAnimHostProperties Prop;

		Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Active->MasterControl.GetRAHostProperties(&Prop);
		RasterAnimHost::SetCopyOfRAHost(&Active->MasterControl);
		sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
		break;
		} // 
	case IDC_PASTECONTROL:
		{
		char CopyMsg[256];
		RasterAnimHostProperties Prop;

		if (RasterAnimHost::GetCopyOfRAHost())
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
			if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEBOOLEAN)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				Active->MasterControl.SetRAHostProperties(&Prop);
				sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // ScenarioEditGUI::HandleButtonClick

/*===========================================================================*/

long ScenarioEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
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

} // ScenarioEditGUI::HandleListDelItem

/*===========================================================================*/

long ScenarioEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_ITEMLIST:
		{
		ConfigureItem();
		break;
		} // IDC_ITEMLIST
	default:
		break;
	} // switch

return(0);

} // ScenarioEditGUI::HandleListSel

/*===========================================================================*/

long ScenarioEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // ScenarioEditGUI::HandleListDoubleClick

/*===========================================================================*/

long ScenarioEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // ScenarioEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long ScenarioEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CLASSDROP:
		{
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // ScenarioEditGUI::HandleCBChange

/*===========================================================================*/

long ScenarioEditGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{

if(ScrollCode == 1)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			WidgetLocal.HPan = (short)ScrollPos;
			RefreshGraph();
			break;
			}
		case IDC_SCROLLBAR2:
			{
			WidgetLocal.HVisible = (short)ScrollPos;
			RefreshGraph();
			break;
			}
		default:
			break;
		} // switch
	return(0);
	} // HSCROLL
else
	return(5);

} // ScenarioEditGUI::HandleScroll

/*===========================================================================*/

long ScenarioEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // ScenarioEditGUI::HandlePageChange

/*===========================================================================*/

long ScenarioEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKREGENSHADOWS:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKREVERSE:
		{
		CaptureReverseCheck();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // ScenarioEditGUI::HandleSCChange

/*===========================================================================*/

long ScenarioEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOSTATEON:
	case IDC_RADIOSTATEOFF:
	case IDC_RADIOSTATEON1:
	case IDC_RADIOSTATEOFF1:
	case IDC_RADIOSTATEON2:
	case IDC_RADIOSTATEOFF2:
		{
		ChangeBasicState();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // ScenarioEditGUI::HandleSRChange

/*===========================================================================*/

long ScenarioEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_DISTANCE:
		{
		NewDistance();
		break;
		}
	default:
		break;
	} // switch CtrlID

return(0);

} // ScenarioEditGUI::HandleFIChange

/*===========================================================================*/

void ScenarioEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WidgetLocal.Crit->GetNotifyClass(), WidgetLocal.Crit->GetNotifySubclass(), WidgetLocal.Crit->GetNotifyItem(), 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if ((Changed & 0x000000ff) == WCS_NOTIFYCOMP_ANIM_ABOUTTOCHANGE)
		return;
	else
		{
		if ((Changed & 0x000000ff) == WCS_NOTIFYCOMP_ANIM_NODEREMOVED)
			WidgetLocal.ActiveNode = WidgetLocal.Crit->GetFirstSelectedNode(0);
		} // else
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
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
	WidgetLWSync(IDC_VECLINKAGE);
	Done = 1;
	} // if query changed
Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// Cancel button invokes the LWSync above
	if (! Done)
		WidgetLWSync(IDC_VECLINKAGE);
	Done = 0;
	} // if linked control objects changed changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // ScenarioEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void ScenarioEditGUI::ConfigureWidgets(void)
{
double Distance;
long NumJobs = 0, SelectedNodes;
RenderJob *CurJob;
EffectList *CurScenario;
char TextStr[256];

sprintf(TextStr, "Render Scenario Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

if (CurJob = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB))
	{
	while (CurJob)
		{
		for (CurScenario = CurJob->Scenarios; CurScenario; CurScenario = CurScenario->Next)
			{
			if (CurScenario->Me == Active)
				NumJobs ++;
			} 
		CurJob = (RenderJob *)CurJob->Next;
		} // while
	} // if
if (NumJobs > 1)
	sprintf(TextStr, "There are %d Render Jobs using this Scenario.", NumJobs);
else if (NumJobs == 1)
	strcpy(TextStr, "There is one Render Job using this Scenario.");
else
	strcpy(TextStr, "There are no Render Jobs using this Scenario.");
WidgetSetText(IDC_JOBSEXIST, TextStr);

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREGENSHADOWS, &Active->RegenShadows, SCFlag_Char, NULL, 0);

BasicOnOff = Active->MasterControl.GetValue(0, 0.0) > .5;
ConfigureSR(NativeWin, IDC_RADIOSTATEON, IDC_RADIOSTATEON, &BasicOnOff, SCFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSTATEON, IDC_RADIOSTATEOFF, &BasicOnOff, SCFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSTATEON1, IDC_RADIOSTATEON1, &BasicOnOff, SCFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSTATEON1, IDC_RADIOSTATEOFF1, &BasicOnOff, SCFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSTATEON2, IDC_RADIOSTATEON2, &BasicOnOff, SCFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSTATEON2, IDC_RADIOSTATEOFF2, &BasicOnOff, SCFlag_Char, 0, NULL, NULL);

ConfigureTB(NativeWin, IDC_ADDITEM, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEITEM, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_COPYCONTROL, IDI_COPY, NULL);
ConfigureTB(NativeWin, IDC_PASTECONTROL, IDI_PASTE, NULL);

BuildItemList();
DisableWidgets();

WidgetSetScrollPos(IDC_SCROLLBAR1, WidgetLocal.HPan);
WidgetSetScrollPos(IDC_SCROLLBAR2, WidgetLocal.HVisible);

if (WidgetLocal.ActiveNode = WidgetLocal.Crit->ValidateNode(WidgetLocal.ActiveNode))
	{
	Distance = WidgetLocal.ActiveNode->GetDistance();
	if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
		{
		WidgetSetText(IDC_DISTFRAMETEXT, "Frame");
		} // IF
	else
		WidgetSetText(IDC_DISTFRAMETEXT, "Time");

	DistanceADT.SetValue(Distance);

	WidgetSNConfig(IDC_DISTANCE, &DistanceADT);

	if (! (SelectedNodes = WidgetLocal.Crit->GetNumSelectedNodes()))
		{
		WidgetLocal.ActiveNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
		SelectedNodes = 1;
		} // if
	sprintf(TextStr, "%1d", SelectedNodes);
	WidgetSetText(IDC_NUMNODES, TextStr);

	WidgetLocal.MaxDistRange = GlobalApp->MCP->GetMaxFrame() / GlobalApp->MainProj->Interactive->GetFrameRate();
	if (WidgetLocal.MaxDistRange < 1.0)
		WidgetLocal.MaxDistRange = 1.0;
	WidgetLocal.LowDrawDist = 0.0;
	WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;
	} // if
else
	{
	ConfigureFI(NativeWin, IDC_DISTANCE, NULL, 0.0, 0.0, (double)FLT_MAX, 0, NULL, 0);
	sprintf(TextStr, "%1d", 0);
	WidgetSetText(IDC_NUMNODES, TextStr);
	if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
		WidgetSetText(IDC_DISTFRAMETEXT, "Frame");
	else
		WidgetSetText(IDC_DISTFRAMETEXT, "Time");
	} // else

WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_MASTERGRAPH, &WidgetLocal);
ConfigureItem();
// advanced
DisplayAdvancedFeatures();

} // ScenarioEditGUI::ConfigureWidgets()

/*===========================================================================*/

void ScenarioEditGUI::BuildItemList(void)
{
RasterAnimHostBooleanList *Current = Active->Items;
RasterAnimHost *CurrentRAHost = NULL;
RasterAnimHost **SelectedItems = NULL;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

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
			if (Current->Me == CurrentRAHost)
				{
				BuildItemListEntry(ListName, Current->Me, Current->TrueFalse);
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
					BuildItemListEntry(ListName, Current->Me, Current->TrueFalse);
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
					BuildItemListEntry(ListName, Current->Me, Current->TrueFalse);
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
		Current = (RasterAnimHostBooleanList *)Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_ITEMLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (RasterAnimHost *));

} // ScenarioEditGUI::BuildItemList

/*===========================================================================*/

void ScenarioEditGUI::BuildItemListEntry(char *ListName, RasterAnimHost *Me, char AddAsterisk)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;

Me->GetRAHostProperties(&Prop);
if (Prop.Name && Prop.Name[0])
	{
	strcpy(ListName, "* ");
	if (AddAsterisk)
		strcat(ListName, "* ");
	else
		strcat(ListName, "  ");
	strcat(ListName, Prop.Name);
	if (Prop.Type && Prop.Type[0])
		{
		strcat(ListName, " ");
		strcat(ListName, Prop.Type);
		} // if
	} // if
else
	strcat(ListName, "  ");

} // ScenarioEditGUI::BuildItemListEntry()

/*===========================================================================*/

void ScenarioEditGUI::SyncWidgets(void)
{

BasicOnOff = Active->MasterControl.GetValue(0, 0.0) > .5;

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREGENSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEON, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEOFF, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEON1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEOFF1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEON2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSTATEOFF2, WP_SRSYNC_NONOTIFY);

WidgetSNSync(IDC_DISTANCE, WP_FISYNC_NONOTIFY);
RefreshGraph();

} // ScenarioEditGUI::SyncWidgets

/*===========================================================================*/

void ScenarioEditGUI::DisableWidgets(void)
{

} // ScenarioEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void ScenarioEditGUI::DisplayAdvancedFeatures(void)
{
bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();
	
if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_STATE_TXT, false);
	WidgetShow(IDC_RADIOSTATEON, false);
	WidgetShow(IDC_RADIOSTATEOFF, false);
	WidgetShow(IDC_RADIOSTATEON1, true);
	WidgetShow(IDC_RADIOSTATEOFF1, true);
	WidgetShow(IDC_RADIOSTATEON2, true);
	WidgetShow(IDC_RADIOSTATEOFF2, true);
	WidgetShow(IDC_INITIAL_TXT, true);
	WidgetShow(IDC_INITIAL_TXT2, true);
	WidgetShow(IDC_MASTERGRAPH, true);
	WidgetShow(IDC_ON_TXT, true);
	WidgetShow(IDC_OFF_TXT, true);
	WidgetShow(IDC_PAN_TXT, true);
	WidgetShow(IDC_VISIBLE_TXT, true);
	WidgetShow(IDC_SELPTS_TXT, true);
	WidgetShow(IDC_DISTFRAMETEXT, true);
	WidgetShow(IDC_SCROLLBAR1, true);
	WidgetShow(IDC_SCROLLBAR2, true);
	WidgetShow(IDC_ADDNODE, true);
	WidgetShow(IDC_DELETENODE, true);
	WidgetShow(IDC_NUMNODES, true);
	WidgetShow(IDC_SELECTALL, true);
	WidgetShow(IDC_SELECTCLEAR, true);
	WidgetShow(IDC_SELECTTOGGLE, true);
	WidgetShow(IDC_DISTANCE, true);
	WidgetShow(IDC_COPYCONTROL, true);
	WidgetShow(IDC_PASTECONTROL, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_STATE_TXT, true);
	WidgetShow(IDC_RADIOSTATEON, true);
	WidgetShow(IDC_RADIOSTATEOFF, true);
	WidgetShow(IDC_RADIOSTATEON1, false);
	WidgetShow(IDC_RADIOSTATEOFF1, false);
	WidgetShow(IDC_RADIOSTATEON2, false);
	WidgetShow(IDC_RADIOSTATEOFF2, false);
	WidgetShow(IDC_INITIAL_TXT, false);
	WidgetShow(IDC_INITIAL_TXT2, false);
	WidgetShow(IDC_MASTERGRAPH, false);
	WidgetShow(IDC_ON_TXT, false);
	WidgetShow(IDC_OFF_TXT, false);
	WidgetShow(IDC_PAN_TXT, false);
	WidgetShow(IDC_VISIBLE_TXT, false);
	WidgetShow(IDC_SELPTS_TXT, false);
	WidgetShow(IDC_DISTFRAMETEXT, false);
	WidgetShow(IDC_SCROLLBAR1, false);
	WidgetShow(IDC_SCROLLBAR2, false);
	WidgetShow(IDC_ADDNODE, false);
	WidgetShow(IDC_DELETENODE, false);
	WidgetShow(IDC_NUMNODES, false);
	WidgetShow(IDC_SELECTALL, false);
	WidgetShow(IDC_SELECTCLEAR, false);
	WidgetShow(IDC_SELECTTOGGLE, false);
	WidgetShow(IDC_DISTANCE, false);
	WidgetShow(IDC_COPYCONTROL, false);
	WidgetShow(IDC_PASTECONTROL, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // ScenarioEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void ScenarioEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ScenarioEditGUI::Cancel

/*===========================================================================*/

void ScenarioEditGUI::AddItem(void)
{
long ItemData, Current;

Current = WidgetCBGetCurSel(IDC_CLASSDROP);
ItemData = (long)WidgetCBGetItemData(IDC_CLASSDROP, Current, 0);

if (ItemData >= WCS_EFFECTSSUBCLASS_LAKE && ItemData < WCS_MAXIMPLEMENTED_EFFECTS)
	EffectsHost->AddAttributeByList(Active, ItemData);
else if (ItemData == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Active->AddDBItem();
	} // else if
else if (ItemData == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Active->AddImageItem();
	} // else if

} // ScenarioEditGUI::AddItem

/*===========================================================================*/

void ScenarioEditGUI::RemoveItem(void)
{
RasterAnimHost **RemoveItems;
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;

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
		UserMessageOK("Remove Item", "There are no Items selected to remove.");
		} // else
	} // if

} // ScenarioEditGUI::RemoveItem

/*===========================================================================*/

void ScenarioEditGUI::GrabAll(void)
{
long ItemData, Current;

Current = WidgetCBGetCurSel(IDC_CLASSDROP);
ItemData = (long)WidgetCBGetItemData(IDC_CLASSDROP, Current, 0);

Active->AddItemsByClass(EffectsHost, ImageHost, DBHost, ItemData);

} // ScenarioEditGUI::GrabAll

/*===========================================================================*/

void ScenarioEditGUI::EditItem(void)
{
RasterAnimHost *EditMe;
long Ct, NumListEntries;

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

} // ScenarioEditGUI::EditItem

/*===========================================================================*/

void ScenarioEditGUI::Name(void)
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

} // ScenarioEditGUI::Name()

/*===========================================================================*/

void ScenarioEditGUI::RefreshGraph(void)
{

WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_MASTERGRAPH, &WidgetLocal);

} // ScenarioEditGUI::RefreshGraph

/*===========================================================================*/

void ScenarioEditGUI::AddNode(void)
{
double LastState = 0.0;
GraphNode *LastNode;
RasterAnimHostProperties Prop;

if (GetInputValue("Enter time or frame number for new Control node.", &DistanceADT))
	{
	// Prop->KeyNodeRange[0] contains time to search backwards from
	Prop.KeyNodeRange[0] = DistanceADT.CurValue;
	if (WidgetLocal.Crit->GetNextAnimNode(&Prop))
		{
		// Prop->NewKeyNodeRange[0] contains time of last node node
		if (LastNode = WidgetLocal.Crit->FindNode(Prop.NewKeyNodeRange[0], .00001, 1.0, 2.0))
			LastState = LastNode->GetValue();
		if (LastState > .5)
			LastState = 0.0;
		else
			LastState = 1.0;
		} // if

	((AnimDoubleTime *)WidgetLocal.Crit)->AddNode(DistanceADT.CurValue, LastState, 0.0);
	} // if

} // ScenarioEditGUI::AddNode

/*===========================================================================*/

void ScenarioEditGUI::RemoveNode(void)
{

if (WidgetLocal.ActiveNode)
	{
	WidgetLocal.ActiveNode = NULL;
	WidgetLocal.Crit->RemoteRemoveSelectedNode();
	} // if

} // ScenarioEditGUI::RemoveNode

/*===========================================================================*/

void ScenarioEditGUI::SelectAllNodes(void)
{

WidgetLocal.Crit->SetNodeSelectedAll();

} // ScenarioEditGUI::SelectAllNodes

/*===========================================================================*/

void ScenarioEditGUI::ClearSelectNodes(void)
{

WidgetLocal.Crit->ClearNodeSelectedAll();
if (WidgetLocal.ActiveNode)
	WidgetLocal.Crit->SetNodeSelected(WidgetLocal.ActiveNode, 1);

} // ScenarioEditGUI::ClearSelectNodes

/*===========================================================================*/

void ScenarioEditGUI::ToggleSelectNodes(void)
{

WidgetLocal.Crit->ToggleNodeSelectedAll();
WidgetLocal.ActiveNode = WidgetLocal.Crit->GetFirstSelectedNode(0);

} // ScenarioEditGUI::ToggleSelectNodes

/*===========================================================================*/

void ScenarioEditGUI::NewActiveNode(GraphNode *NewActive)
{

WidgetLocal.ActiveNode = NewActive;
ConfigureWidgets();
GlobalApp->MainProj->Interactive->SetParam(1, WCS_INTERCLASS_TIME, WCS_TIME_SUBCLASS_TIME, 0, 0,
	NewActive->GetDistance(), 0);

} // ScenarioEditGUI::NewActiveNode

/*===========================================================================*/

void ScenarioEditGUI::NewDistance(void)
{
double Distance, FrameRate;

if (WidgetLocal.ActiveNode)
	{
	if (WidgetLocal.DisplayByFrame)
		{
		FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
		Distance = WCS_floor(DistanceADT.CurValue * FrameRate + .5) / FrameRate;
		} // if
	else
		Distance = DistanceADT.CurValue;
	WidgetFISync(IDC_DISTANCE, WP_FISYNC_NONOTIFY);
	WidgetLocal.Crit->RemoteAlterSelectedNodePosition(Distance, WidgetLocal.ActiveNode->GetDistance());
	} // if

} // ScenarioEditGUI::NewDistance

/*===========================================================================*/

void ScenarioEditGUI::Actionate(void)
{
// this code moved to RenderScenario::ActionNow
Active->ActionNow();
} // ScenarioEditGUI::Actionate

/*===========================================================================*/

void ScenarioEditGUI::ChangeBasicState(void)
{
double SetValue;
GraphNode *CurNode;
bool FirstNode = true;

SetValue = BasicOnOff ? 1.0: 0.0;
for ((CurNode = Active->MasterControl.Graph[0]->FindNearestNode(0.0)) || 
	(CurNode = Active->MasterControl.AddNode(0.0)); CurNode; CurNode = CurNode->Next)
	{
	if (FirstNode && CurNode->Value == SetValue)
		break;
	CurNode->SetValue(CurNode->Value > .5 ? 0.0: 1.0);
	FirstNode = false;
	} // if
	
} // ScenarioEditGUI::ChangeBasicState

/*===========================================================================*/

void ScenarioEditGUI::ConfigureItem(void)
{
long TotNumSel, Test, TempCt, NumListItems;
RasterAnimHostBooleanList *Current = Active->Items;

Test = WidgetLBGetCurSel(IDC_ITEMLIST);
if (Test != -1)
	{
	if ((TotNumSel = WidgetLBGetSelCount(IDC_ITEMLIST)) == 1)
		{
		NumListItems = WidgetLBGetCount(IDC_ITEMLIST);
		// iterate through item list to find the right item
		for (TempCt = 0; TempCt < NumListItems && Current; TempCt ++, Current = (RasterAnimHostBooleanList *)Current->Next)
			{
			if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
				{
				WidgetSetCheck(IDC_CHECKREVERSE, Current->TrueFalse);
				break;
				} // if
			} // for
		} // if
	} // if something selected
else
	{
	WidgetSetCheck(IDC_CHECKREVERSE, false);
	} // else
	
} // ScenarioEditGUI::ConfigureItem

/*===========================================================================*/

void ScenarioEditGUI::CaptureReverseCheck(void)
{
long TempCt, NumListItems;
RasterAnimHostBooleanList *Current = Active->Items;
char Checked;

Checked = WidgetGetCheck(IDC_CHECKREVERSE);

NumListItems = WidgetLBGetCount(IDC_ITEMLIST);

for (TempCt = 0; TempCt < NumListItems && Current; TempCt ++, Current = (RasterAnimHostBooleanList *)Current->Next)
	{
	if (WidgetLBGetSelState(IDC_ITEMLIST, TempCt))
		{
		Current->TrueFalse = Checked;
		} // if
	} // for

} // ScenarioEditGUI::CaptureReverseCheck

/*===========================================================================*/

bool ScenarioEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || (Active->MasterControl.Graph[0] && Active->MasterControl.Graph[0]->NumNodes > 1) ? true : false);

} // ScenarioEditGUI::QueryLocalDisplayAdvancedUIVisibleState

