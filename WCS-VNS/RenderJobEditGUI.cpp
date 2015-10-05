// RenderJobEditGUI.cpp
// Code for RenderJob editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RenderJobEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "resource.h"
#include "AppMem.h"
#include "Lists.h"

char *RenderJobEditGUI::TabNames[WCS_RENDERJOBGUI_NUMTABS] = {"General"};

long RenderJobEditGUI::ActivePage;

NativeGUIWin RenderJobEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // RenderJobEditGUI::Open

/*===========================================================================*/

NativeGUIWin RenderJobEditGUI::Construct(void)
{
GeneralEffect *MyEffect;
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_RENDER_SCENARIOS
	CreateSubWinFromTemplate(IDD_RENDERJOB_GENERAL_VNS, 0, 0);
	#else // WCS_RENDER_SCENARIOS
	CreateSubWinFromTemplate(IDD_RENDERJOB_GENERAL, 0, 0);
	#endif // WCS_RENDER_SCENARIOS

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_RENDERJOBGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_CAMERADROP, -1, "New Camera...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_CAMERADROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_CAMERADROP, TabIndex, MyEffect);
			} // for
		WidgetCBInsert(IDC_RENDEROPTDROP, -1, "New Render Options...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_RENDEROPTDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_RENDEROPTDROP, TabIndex, MyEffect);
			} // for
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // RenderJobEditGUI::Construct

/*===========================================================================*/

RenderJobEditGUI::RenderJobEditGUI(EffectsLib *EffectsSource, RenderJob *ActiveSource)
: GUIFenetre('RNDJ', this, "Render Job Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_RENDERJOB, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								#ifdef WCS_RENDER_SCENARIOS
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								#endif // WCS_RENDER_SCENARIOS
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Render Job Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
	ConstructError = 1;

} // RenderJobEditGUI::RenderJobEditGUI

/*===========================================================================*/

RenderJobEditGUI::~RenderJobEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // RenderJobEditGUI::~RenderJobEditGUI()

/*===========================================================================*/

long RenderJobEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_RJG, 0);

return(0);

} // RenderJobEditGUI::HandleCloseWin

/*===========================================================================*/

long RenderJobEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_RJG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITCAMERA:
		{
		if (Active->Cam)
			Active->Cam->EditRAHost();
		break;
		} //
	case IDC_EDITRENDEROPT:
		{
		if (Active->Options)
			Active->Options->EditRAHost();
		break;
		} //
	case IDC_ADDSCENARIO:
		{
		AddScenario();
		break;
		} // IDC_ADDSCENARIO
	case IDC_REMOVESCENARIO:
		{
		RemoveScenario();
		break;
		} // IDC_REMOVESCENARIO
	case IDC_MOVESCENARIOUP:
		{
		ChangeScenarioListPosition(1);
		break;
		} // IDC_MOVESCENARIOUP
	case IDC_MOVESCENARIODOWN:
		{
		ChangeScenarioListPosition(0);
		break;
		} // IDC_MOVESCENARIODOWN
	case IDC_SELECTNOW:
		{
		ActionateScenarios();
		break;
		} // IDC_SELECTNOW
	default:
		break;
	} // ButtonID

return(0);

} // RenderJobEditGUI::HandleButtonClick

/*===========================================================================*/

long RenderJobEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SCENARIOLIST:
		{
		RemoveScenario();
		break;
		} // IDC_SCENARIOLIST
	default:
		break;
	} // switch

return(0);

} // RenderJobEditGUI::HandleListDelItem

/*===========================================================================*/

long RenderJobEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SCENARIOLIST:
		{
		EditScenario();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // RenderJobEditGUI::HandleListDoubleClick

/*===========================================================================*/

long RenderJobEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CAMERADROP:
		{
		SelectNewCamera();
		break;
		}
	case IDC_RENDEROPTDROP:
		{
		SelectNewRenderOpt();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // RenderJobEditGUI::HandleCBChange

/*===========================================================================*/

long RenderJobEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RenderJobEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long RenderJobEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 0:
				{
				ShowPanel(0, 0);
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

} // RenderJobEditGUI::HandlePageChange

/*===========================================================================*/

long RenderJobEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RenderJobEditGUI::HandleSCChange

/*===========================================================================*/

long RenderJobEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_PRIORITY:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RenderJobEditGUI::HandleFIChange

/*===========================================================================*/

void RenderJobEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Pos, CurPos, Done = 0;
GeneralEffect *MyEffect, *MatchEffect;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = Active->Cam;
	WidgetCBClear(IDC_CAMERADROP);
	WidgetCBInsert(IDC_CAMERADROP, -1, "New Camera...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_CAMERADROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_CAMERADROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_CAMERADROP, CurPos);
	Done = 1;
	} // if CAMERA name changed

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = Active->Options;
	WidgetCBClear(IDC_RENDEROPTDROP);
	WidgetCBInsert(IDC_RENDEROPTDROP, -1, "New Render Options...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_RENDEROPTDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_RENDEROPTDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_RENDEROPTDROP, CurPos);
	Done = 1;
	} // if RENDEROPT name changed


if (! Done)
	ConfigureWidgets();

} // RenderJobEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void RenderJobEditGUI::ConfigureWidgets(void)
{
Camera *TestCam;
RenderOpt *TestOpt;
long ListPos, Ct, NumEntries;
char TextStr[256];

sprintf(TextStr, "Render Job Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

#ifdef WCS_RENDER_SCENARIOS
ConfigureTB(NativeWin, IDC_ADDSCENARIO, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESCENARIO, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVESCENARIOUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVESCENARIODOWN, IDI_ARROWDOWN, NULL);
#endif // WCS_RENDER_SCENARIOS

if (Active->Cam)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_CAMERADROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestCam = (Camera *)WidgetCBGetItemData(IDC_CAMERADROP, Ct)) != (Camera *)LB_ERR && TestCam == Active->Cam)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_CAMERADROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_CAMERADROP, -1);

if (Active->Options)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_RENDEROPTDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestOpt = (RenderOpt *)WidgetCBGetItemData(IDC_RENDEROPTDROP, Ct)) != (RenderOpt *)LB_ERR && TestOpt == Active->Options)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_RENDEROPTDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_RENDEROPTDROP, -1);

BuildScenarioList();

} // RenderJobEditGUI::ConfigureWidgets()

/*===========================================================================*/

void RenderJobEditGUI::BuildScenarioList(void)
{
#ifdef WCS_RENDER_SCENARIOS
EffectList *Current = Active->Scenarios;
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

NumListItems = WidgetLBGetCount(IDC_SCENARIOLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_SCENARIOLIST, TempCt))
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
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildScenarioListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_SCENARIOLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_SCENARIOLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_SCENARIOLIST, 1, Ct);
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
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildScenarioListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_SCENARIOLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_SCENARIOLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SCENARIOLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_SCENARIOLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildScenarioListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_SCENARIOLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_SCENARIOLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SCENARIOLIST, 1, Place);
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
		WidgetLBDelete(IDC_SCENARIOLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));
#endif // WCS_RENDER_SCENARIOS
} // RenderJobEditGUI::BuildScenarioList

/*===========================================================================*/

void RenderJobEditGUI::BuildScenarioListEntry(char *ListName, GeneralEffect *Me)
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

} // RenderJobEditGUI::BuildScenarioListEntry()

/*===========================================================================*/

void RenderJobEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // RenderJobEditGUI::Cancel

/*===========================================================================*/

void RenderJobEditGUI::AddScenario(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_SCENARIO);

} // RenderJobEditGUI::AddScenario

/*===========================================================================*/

void RenderJobEditGUI::RemoveScenario(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct);
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
		UserMessageOK("Remove Render Scenario", "There are no Render Scenarios selected to remove.");
		} // else
	} // if

} // RenderJobEditGUI::RemoveScenario

/*===========================================================================*/

void RenderJobEditGUI::EditScenario(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // RenderJobEditGUI::EditScenario

/*===========================================================================*/

void RenderJobEditGUI::ChangeScenarioListPosition(short MoveUp)
{
long Ct, NumListEntries, SendNotify = 0;
RasterAnimHost *MoveMe;
EffectList *Current, *PrevScenario = NULL, *PrevPrevScenario = NULL, *StashScenario;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
					{
					Current = Active->Scenarios;
					while (Current->Me != MoveMe)
						{
						PrevPrevScenario = PrevScenario;
						PrevScenario = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (PrevScenario)
							{
							StashScenario = Current->Next;
							if (PrevPrevScenario)
								{
								PrevPrevScenario->Next = Current;
								Current->Next = PrevScenario;
								} // if
							else
								{
								Active->Scenarios = Current;
								Active->Scenarios->Next = PrevScenario;
								} // else
							PrevScenario->Next = StashScenario;
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
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
					{
					Current = Active->Scenarios;
					while (Current->Me != MoveMe)
						{
						PrevPrevScenario = PrevScenario;
						PrevScenario = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (Current->Next)
							{
							StashScenario = Current->Next->Next;
							if (PrevScenario)
								{
								PrevScenario->Next = Current->Next;
								PrevScenario->Next->Next = Current;
								} // if
							else
								{
								Active->Scenarios = Current->Next;
								Active->Scenarios->Next = Current;
								} // else
							Current->Next = StashScenario;
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

} // RenderJobEditGUI::ChangeScenarioListPosition

/*===========================================================================*/

void RenderJobEditGUI::Name(void)
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

} // RenderJobEditGUI::Name()

/*===========================================================================*/

void RenderJobEditGUI::SelectNewCamera(void)
{
Camera *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_CAMERADROP);
if (((NewObj = (Camera *)WidgetCBGetItemData(IDC_CAMERADROP, Current, 0)) != (Camera *)LB_ERR && NewObj)
	|| (NewObj = (Camera *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, NULL, NULL)))
	{
	Active->SetCamera(NewObj);
	} // if

} // RenderJobEditGUI::SelectNewCamera

/*===========================================================================*/

void RenderJobEditGUI::SelectNewRenderOpt(void)
{
RenderOpt *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_RENDEROPTDROP);
if (((NewObj = (RenderOpt *)WidgetCBGetItemData(IDC_RENDEROPTDROP, Current, 0)) != (RenderOpt *)LB_ERR && NewObj)
	|| (NewObj = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
	{
	Active->SetRenderOpt(NewObj);
	} // if

} // RenderJobEditGUI::SelectNewRenderOpt

/*===========================================================================*/

void RenderJobEditGUI::ActionateScenarios(void)
{
double CurTime;
EffectList *CurScenario;
NotifyTag Cryogenics[2];
int Dummy;

Cryogenics[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Cryogenics[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Cryogenics);

CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();

for (CurScenario = Active->Scenarios; CurScenario; CurScenario = CurScenario->Next)
	{
	if (CurScenario->Me)
		((RenderScenario *)CurScenario->Me)->ProcessFrameToRender(CurTime, Dummy, GlobalApp->AppDB);
	} // for

Cryogenics[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Cryogenics);

} // RenderJobEditGUI::ActionateScenarios

/*===========================================================================*/
