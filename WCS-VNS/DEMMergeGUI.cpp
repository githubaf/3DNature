// DEMMergeGUI.cpp
// Code for DEM merger GUI
// Built from scratch on 08/30/02 by Frank Weed II
// Copyright 2002 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "DEMMergeGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Conservatory.h"
#include "Joe.h"
#include "DEM.h"
#include "AppMem.h"
#include "Interactive.h"
#include "resource.h"
#include "Lists.h"

long DEMMergeGUI::ActivePage;
// advanced
long DEMMergeGUI::DisplayAdvanced;

#ifdef DOMERGE
extern unsigned long DoMerge;
#endif // DOMERGE

char *DEMMergeGUI::TabNames[WCS_DEMMERGEGUI_NUMTABS] = {"General", "Queries", "MultiRes"};

NativeGUIWin DEMMergeGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

#ifdef DOMERGE
if(DoMerge)
	{
	if (Active->MultiRes)
		MergeMultiRes();
	else
		Merge();
	GlobalApp->SetTerminate(); // autoquit after merge
	} // if
#endif // DOMERGE

return (Success);

} // DEMMergeGUI::Open

/*===========================================================================*/

NativeGUIWin DEMMergeGUI::Construct(void)
{
#ifdef WCS_BUILD_VNS
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS
long TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_DEMMERGE_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_DEMMERGE_QUERIES, 0, 1);
	CreateSubWinFromTemplate(IDD_DEMMERGE_MULTIRES, 0, 2);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_DEMMERGEGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, TabIndex, MyEffect);
			} // for
		#endif // WCS_BUILD_VNS
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // DEMMergeGUI::Construct

/*===========================================================================*/

DEMMergeGUI::DEMMergeGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, InterCommon *InteractSource,
			DEMMerger *ActiveSource)
: GUIFenetre('DMRG', this, "DEM Merger"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_DEMMERGER, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 1;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
InteractHost = InteractSource;
Active = ActiveSource;

if (ProjSource && ProjSource->ProjectLoaded)
	{
	#ifdef WCS_BUILD_VNS
	if (EffectsSource && DBSource && InteractSource && ActiveSource)
		{
		sprintf(NameStr, "DEM Merger - %s", Active->GetName());
		if (Active->GetRAHostRoot()->TemplateItem)
			strcat(NameStr, " (Templated)");
		SetTitle(NameStr);
		// advanced
		DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
		if (! Active->DEMPath[0])
			strcpy(Active->DEMPath, ProjHost->dirname);
		if (! Active->HiResPath[0])
			strcpy(Active->HiResPath, ProjHost->dirname);
		Active->SetGoodBoundsEtc();
		Active->Copy(&Backup, Active);
		GlobalApp->AppEx->RegisterClient(this, AllEvents);
		GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
		GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
		ConstructError = 0;
		} // if
	#endif // WCS_BUILD_VNS
	} // if
else
	{
	UserMessageOK("DEM Merger", "There is no Project in memory. You must load or create a Project before you can use the DEM Merger.");
	} // else

} // DEMMergeGUI::DEMMergeGUI

/*===========================================================================*/

DEMMergeGUI::~DEMMergeGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // DEMMergeGUI::~DEMMergeGUI()

/*===========================================================================*/

long DEMMergeGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_MRG, 0);

return(0);

} // DEMMergeGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long DEMMergeGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // DEMMergeGUI::HandleShowAdvanced

/*===========================================================================*/

long DEMMergeGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_MRG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (Active->MergeCoordSys)
			Active->MergeCoordSys->EditRAHost();
		SetMetrics();
		break;
		} // IDC_EDITCOORDS
	case IDC_ADDSQ:
		{
		AddSQ();
		break;
		} // IDC_ADDSQ
	case IDC_REMOVESQ:
		{
		RemoveSQ();
		break;
		} // IDC_REMOVESQ
	case IDC_MOVESQUP:
		{
		ChangeSQListPosition(1);
		break;
		} // IDC_MOVESQUP
	case IDC_MOVESQDOWN:
		{
		ChangeSQListPosition(0);
		break;
		} // IDC_MOVESQDOWN
	case IDC_GRABALL:
		{
		Active->GrabAllQueries();
		Active->ScanForRes(GlobalApp->AppDB);
		Display2ndRes();
		break;
		} // IDC_GRABALL
	case IDC_DOMERGE:
	case IDC_DOMERGE2:
	case IDC_DOMERGE3:
		{
		if (Active->MultiRes)
			MergeMultiRes();
		else
			Merge();
		break;
		} // IDC_DOMERGE
	case IDC_UPDATE_BOUNDS:
		{
		Active->UpdateBounds(0, DBHost, ProjHost);
		ComputeMemUse();
		break;
		} // IDC_UPDATE_BOUNDS
	case IDC_UPDATE_HIRES:
		{
		Active->UpdateBounds(1, DBHost, ProjHost);
		ComputeMemUse();
		break;
		} // IDC_UPDATE_HIRES
	default: break;
	} // ButtonID
return(0);

} // DEMMergeGUI::HandleButtonClick

/*===========================================================================*/

long DEMMergeGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SQLIST:
		{
		RemoveSQ();
		break;
		} // IDC_SQLIST
	default: break;
	} // switch

return(0);

} // DEMMergeGUI::HandleListDelItem

/*===========================================================================*/

long DEMMergeGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SQLIST:
		{
		EditSQ();
		break;
		}
	default: break;
	} // switch CtrlID

return (0);

} // DEMMergeGUI::HandleListDoubleClick

/*===========================================================================*/

long DEMMergeGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	default: break;
	} // switch CtrlID

return (0);

} // DEMMergeGUI::HandleStringLoseFocus

/*===========================================================================*/

long DEMMergeGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		SetMetrics();
		#endif // WCS_BUILD_VNS
		break;
		}
	default: break;
	} // switch CtrlID

return (0);

} // DEMMergeGUI::HandleCBChange

/*===========================================================================*/

long DEMMergeGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
	default: break;
	} // switch

ActivePage = NewPageID;

return(0);

} // DEMMergeGUI::HandlePageChange

/*===========================================================================*/

long DEMMergeGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_MULTIRES:
		{
		Display2ndRes();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} //
	default: break;
	} // switch CtrlID

return(0);

} // DEMMergeGUI::HandleSCChange

/*===========================================================================*/

long DEMMergeGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_DIVIDER:
		{
		ConfigureWidgets();
		break;
		} // IDC_DIVIDER
	default:
		break;
	} // switch CtrlID

Active->SetGoodBoundsEtc();
ComputeMemUse();

return(0);

} // DEMMergeGUI::HandleFIChange

/*===========================================================================*/

void DEMMergeGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;
#ifdef WCS_BUILD_VNS
long Pos, CurPos;
GeneralEffect *MyEffect, *MatchEffect;
#endif // WCS_BUILD_VNS

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
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPos = -1;
	MatchEffect = Active->MergeCoordSys;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		Done = 1;
		} // if
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // DEMMergeGUI::HandleNotifyEvent()

/*===========================================================================*/

void DEMMergeGUI::ConfigureWidgets(void)
{
#ifdef WCS_BUILD_VNS
long ListPos, Ct, NumEntries;
CoordSys *TestObj;
#endif // WCS_BUILD_VNS
char TextStr[256];
char SQStr[] = "If you want to create a high resolution DEM Insert, the first Query is used to select the DEM or DEMs for the High Res \
Insert. Remaining Queries select outlying or background DEMs.";

sprintf(TextStr, "DEM Merger - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureDD(NativeWin, IDC_MRG_DEMNAME, Active->DEMPath, 191, Active->DEMName, 63, IDC_LABEL_CMAP);
ConfigureDD(NativeWin, IDC_MRG_HIRESNAME, Active->HiResPath, 191, Active->HiResName, 63, IDC_LABEL_CMAP);

ConfigureSC(NativeWin, IDC_MULTIRES, &Active->MultiRes, SCFlag_Char, NULL, 0);

WidgetSmartRAHConfig(IDC_MERGEXRES, &Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES], Active);
WidgetSmartRAHConfig(IDC_MERGEYRES, &Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES], Active);
WidgetSmartRAHConfig(IDC_DIVIDER, &Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER], Active);
WidgetSmartRAHConfig(IDC_NORTHBOUND, &Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &Active->NormalBounds);
WidgetSmartRAHConfig(IDC_SOUTHBOUND, &Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], &Active->NormalBounds);
WidgetSmartRAHConfig(IDC_EASTBOUND, &Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], &Active->NormalBounds);
WidgetSmartRAHConfig(IDC_WESTBOUND, &Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &Active->NormalBounds);
WidgetSmartRAHConfig(IDC_NORTHHIRES, &Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &Active->HiResBounds);
WidgetSmartRAHConfig(IDC_SOUTHHIRES, &Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], &Active->HiResBounds);
WidgetSmartRAHConfig(IDC_EASTHIRES, &Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], &Active->HiResBounds);
WidgetSmartRAHConfig(IDC_WESTHIRES, &Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &Active->HiResBounds);

ConfigureTB(NativeWin, IDC_ADDSQ, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESQ, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVESQUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVESQDOWN, IDI_ARROWDOWN, NULL);

#ifdef WCS_BUILD_VNS
if (Active->MergeCoordSys)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestObj == Active->MergeCoordSys)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);

	if (Active->MergeCoordSys->GetGeographic())	// Geographic?
		{
		Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
		Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
		} // if
	else
		{
		Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		} // else

	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
#endif // WCS_BUILD_VNS

Display2ndRes();

WidgetSetText(IDC_MRG_DEMNAME, Active->DEMName);
WidgetSetText(IDC_MRG_HIRESNAME, Active->HiResName);

WidgetSetText(IDC_MRG_SQTEXT, SQStr);

SetMetrics();
BuildSQList();
ComputeMemUse();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // DEMMergeGUI::ConfigureWidgets()

/*===========================================================================*/

void DEMMergeGUI::Display2ndRes(void)
{
double MergeXRes, MergeYRes, Divider;
char TextStr[256];

MergeXRes = Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
MergeYRes = Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue;
Divider = Active->AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue;
if (! Active->MergeCoordSys || Active->MergeCoordSys->GetGeographic())
	{
	// convert degrees to meters based on 1 arc second = 30m
	MergeXRes *= 108000.0;
	MergeYRes *= 108000.0;
	} // if
sprintf(TextStr, "%.4fm x %.4fm", MergeXRes / Divider,  MergeYRes / Divider);
WidgetSetText(IDC_2NDRES, TextStr, NULL);

} // DEMMergeGUI::Display2ndRes

/*===========================================================================*/

void DEMMergeGUI::BuildSQList(void)
{
EffectList *Current = Active->Queries;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_SQLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_SQLIST, TempCt))
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
			if (WidgetLBGetSelState(IDC_SQLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_SQLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_SQLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildSQListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_SQLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_SQLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_SQLIST, 1, Ct);
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
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_SQLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildSQListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_SQLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_SQLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SQLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_SQLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildSQListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_SQLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_SQLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SQLIST, 1, Place);
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
		WidgetLBDelete(IDC_SQLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));

} // DEMMergeGUI::BuildSQList

/*===========================================================================*/

void DEMMergeGUI::BuildSQListEntry(char *ListName, GeneralEffect *Me)
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

} // DEMMergeGUI::BuildSQListEntry()

/*===========================================================================*/

void DEMMergeGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_MULTIRES, WP_SCSYNC_NONOTIFY);

WidgetSNSync(IDC_MERGEXRES, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MERGEYRES, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DIVIDER, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTHBOUND, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOUTHBOUND, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASTBOUND, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WESTBOUND, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTHHIRES, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOUTHHIRES, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASTHIRES, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WESTHIRES, WP_FISYNC_NONOTIFY);

} // DEMMergeGUI::SyncWidgets

/*===========================================================================*/

void DEMMergeGUI::DisableWidgets(void)
{
char disability;

if (Active->Queries)
	disability = 0;
else
	disability = 1;
WidgetSetDisabled(IDC_NORTHBOUND, disability);
WidgetSetDisabled(IDC_SOUTHBOUND, disability);
WidgetSetDisabled(IDC_EASTBOUND, disability);
WidgetSetDisabled(IDC_WESTBOUND, disability);
WidgetSetDisabled(IDC_UPDATE_BOUNDS, disability);

if (Active->GoodBounds && Active->Queries)
	disability = 0;
else
	disability = 1;
WidgetSetDisabled(IDC_DOMERGE, disability);

if (Active->MultiRes && Active->Queries)
	disability = 0;
else
	disability = 1;
WidgetSetDisabled(IDC_DIVIDER, disability);
WidgetSetDisabled(IDC_NORTHHIRES, disability);
WidgetSetDisabled(IDC_SOUTHHIRES, disability);
WidgetSetDisabled(IDC_WESTHIRES, disability);
WidgetSetDisabled(IDC_EASTHIRES, disability);
WidgetSetDisabled(IDC_MRG_HIRESNAME, disability);
WidgetSetDisabled(IDC_UPDATE_HIRES, disability);

} // DEMMergeGUI::DisableWidgets

/*===========================================================================*/

void DEMMergeGUI::ComputeMemUse(void)
{
unsigned long normal, highres = 0;
char String[32], disability;

if (Active->GoodBounds)
	{
	normal = (unsigned long)((double)Active->MergeWidth * (double)Active->MergeHeight * 6.0 * 10.0 / 9.0);	// 1 & 1/9
	if (Active->MultiRes)
		highres = Active->HighResMergeWidth * Active->HighResMergeHeight * 6UL;
	sprintf(String, "%u bytes", normal + highres);
	WidgetSetText(IDC_MEMUSE, String);
	WidgetSetText(IDC_MEMUSE2, String);
	} // if
else
	{
	WidgetSetText(IDC_MEMUSE, " ");
	WidgetSetText(IDC_MEMUSE2, " ");
	} // else

sprintf(String, "Cols: %u", Active->MergeWidth);
WidgetSetText(IDC_MERGECOLS, String);
sprintf(String, "Rows: %u", Active->MergeHeight);
WidgetSetText(IDC_MERGEROWS, String);
sprintf(String, "Cols: %u", Active->HighResMergeWidth);
WidgetSetText(IDC_MRG_HIRESCOLS, String);
sprintf(String, "Rows: %u", Active->HighResMergeHeight);
WidgetSetText(IDC_MRG_HIRESROWS, String);

if (Active->GoodBounds && Active->Queries)
	disability = 0;
else
	disability = 1;
WidgetSetDisabled(IDC_DOMERGE, disability);

} // DEMMergeGUI::ComputeMemUse

/*===========================================================================*/

// advanced
void DEMMergeGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_PURPOSE_TXT, true);
	WidgetShow(IDC_MULTIRES, true);
	WidgetShow(IDC_DIVIDER, true);
	WidgetShow(IDC_2NDRES, true);
	WidgetShow(IDC_HIRES_BOX, true);
	WidgetShow(IDC_BOUNDS_BOX, true);
	WidgetShow(IDC_OUTPUT_BOX, true);
	WidgetShow(IDC_NORTHHIRES, true);
	WidgetShow(IDC_WESTHIRES, true);
	WidgetShow(IDC_EASTHIRES, true);
	WidgetShow(IDC_SOUTHHIRES, true);
	WidgetShow(IDC_MRG_HIRESCOLS, true);
	WidgetShow(IDC_MRG_HIRESROWS, true);
	WidgetShow(IDC_UPDATE_HIRES, true);
	WidgetShow(IDC_MRG_HIRESNAME, true);
	WidgetShow(IDC_MEMNEEDED_TXT, true);
	WidgetShow(IDC_MEMUSE2, true);
	WidgetShow(IDC_DOMERGE2, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_PURPOSE_TXT, false);
	WidgetShow(IDC_MULTIRES, false);
	WidgetShow(IDC_DIVIDER, false);
	WidgetShow(IDC_2NDRES, false);
	WidgetShow(IDC_HIRES_BOX, false);
	WidgetShow(IDC_BOUNDS_BOX, false);
	WidgetShow(IDC_OUTPUT_BOX, false);
	WidgetShow(IDC_NORTHHIRES, false);
	WidgetShow(IDC_WESTHIRES, false);
	WidgetShow(IDC_EASTHIRES, false);
	WidgetShow(IDC_SOUTHHIRES, false);
	WidgetShow(IDC_MRG_HIRESCOLS, false);
	WidgetShow(IDC_MRG_HIRESROWS, false);
	WidgetShow(IDC_UPDATE_HIRES, false);
	WidgetShow(IDC_MRG_HIRESNAME, false);
	WidgetShow(IDC_MEMNEEDED_TXT, false);
	WidgetShow(IDC_MEMUSE2, false);
	WidgetShow(IDC_DOMERGE2, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // DEMMergeGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void DEMMergeGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // DEMMergeGUI::Cancel

/*===========================================================================*/

void DEMMergeGUI::AddSQ(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_SEARCHQUERY);

} // DEMMergeGUI::AddSQ

/*===========================================================================*/

void DEMMergeGUI::RemoveSQ(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_SQLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SQLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_SQLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_SQLIST, Ct);
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
		UserMessageOK("Remove Search Query", "There are no search Queries selected to remove.");
		} // else
	} // if

} // DEMMergeGUI::RemoveSQ

/*===========================================================================*/

void DEMMergeGUI::EditSQ(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_SQLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SQLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_SQLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // DEMMergeGUI::EditSQ

/*===========================================================================*/

void DEMMergeGUI::Name(void)
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

} // DEMMergeGUI::Name()

/*===========================================================================*/

void DEMMergeGUI::ChangeSQListPosition(short MoveUp)
{
long Ct, NumListEntries, SendNotify = 0;
RasterAnimHost *MoveMe;
EffectList *Current, *PrevSQ = NULL, *PrevPrevSQ = NULL, *StashSQ;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_SQLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_SQLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SQLIST, Ct))
					{
					Current = Active->Queries;
					while (Current->Me != MoveMe)
						{
						PrevPrevSQ = PrevSQ;
						PrevSQ = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (PrevSQ)
							{
							StashSQ = Current->Next;
							if (PrevPrevSQ)
								{
								PrevPrevSQ->Next = Current;
								Current->Next = PrevSQ;
								} // if
							else
								{
								Active->Queries = Current;
								Active->Queries->Next = PrevSQ;
								} // else
							PrevSQ->Next = StashSQ;
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
			if (WidgetLBGetSelState(IDC_SQLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SQLIST, Ct))
					{
					Current = Active->Queries;
					while (Current->Me != MoveMe)
						{
						PrevPrevSQ = PrevSQ;
						PrevSQ = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (Current->Next)
							{
							StashSQ = Current->Next->Next;
							if (PrevSQ)
								{
								PrevSQ->Next = Current->Next;
								PrevSQ->Next->Next = Current;
								} // if
							else
								{
								Active->Queries = Current->Next;
								Active->Queries->Next = Current;
								} // else
							Current->Next = StashSQ;
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

} // DEMMergeGUI::ChangeSQListPosition

/*===========================================================================*/

void DEMMergeGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Active->SetCoords(NewObj);
	} // if
#endif // WCS_BUILD_VNS
} // DEMMergeGUI::SelectNewCoords

/*===========================================================================*/

void DEMMergeGUI::Merge(void)
{
double etime;
char msg[80];

StartHiResTimer();

Active->Merge(DBHost, ProjHost, EffectsHost, InteractHost);

etime = StopHiResTimerSecs();
sprintf(msg, "Merge time = %f", etime);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);

} // DEMMergeGUI::Merge

/*===========================================================================*/

void DEMMergeGUI::MergeMultiRes(void)
{
double etime;
char msg[80];

StartHiResTimer();

Active->MergeMultiRes(DBHost, ProjHost, EffectsHost, InteractHost);

etime = StopHiResTimerSecs();
sprintf(msg, "Merge time = %f", etime);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);

} // DEMMergeGUI::MergeMultiRes

/*===========================================================================*/

void DEMMergeGUI::SetMetrics(void)
{
unsigned char mtlat = WCS_ANIMDOUBLE_METRIC_LATITUDE, mtlon = WCS_ANIMDOUBLE_METRIC_LONGITUDE;

if (Active->MergeCoordSys && ! Active->MergeCoordSys->GetGeographic())	// are we projected?
	mtlat = mtlon = WCS_ANIMDOUBLE_METRIC_DISTANCE;

Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(mtlat);
Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(mtlat);
Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(mtlon);
Active->NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(mtlon);
Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(mtlat);
Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(mtlat);
Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(mtlon);
Active->HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(mtlon);
SyncWidgets();

} // DEMMergeGUI::SetMetrics

/*===========================================================================*/

bool DEMMergeGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->MultiRes ? true : false);

} // DEMMergeGUI::QueryLocalDisplayAdvancedUIVisibleState

