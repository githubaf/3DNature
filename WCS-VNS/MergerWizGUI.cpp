// MergerWizGUI.cpp
// Code for Merger Wizard GUI
// Created from MergerWizGUI.cpp on 01/18/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "MergerWizGUI.h"
#include "AppMem.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Raster.h"
#include "Conservatory.h"
#include "ImageLibGUI.h"
#include "Lists.h"
#include "resource.h"
#include "DBEditGUI.h"
#include "Layers.h"
#include "DBFilterEvent.h"
#include "GraphData.h"

extern unsigned short MergerWizPageResourceID[];

// F2_NOTE: Code Next button enabled/disabled states on IDD_MERGERWIZ_SELECTDEMS page

/*===========================================================================*/

void MergerWizGUI::AddSQ(void)
{

EffectsHost->AddAttributeByList(&Merger, WCS_EFFECTSSUBCLASS_SEARCHQUERY);

} // MergerWizGUI::AddSQ

/*===========================================================================*/

void MergerWizGUI::BuildSQList(void)
{
GeneralEffect **SelectedItems = NULL;
EffectList *Current = Merger.Queries;
RasterAnimHost *CurrentRAHost = NULL;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

NumListItems = WidgetLBGetCount(IDC_MWIZ_LBSQLIST);

for (TempCt = 0; TempCt < NumListItems; ++TempCt)
	{
	if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, TempCt))
		{
		++NumSelected;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (GeneralEffect **)AppMem_Alloc(NumSelected * sizeof (GeneralEffect *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt++)
			{
			if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, TempCt))
				{
				SelectedItems[SelCt++] = (GeneralEffect *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildSQListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_MWIZ_LBSQLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_MWIZ_LBSQLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_MWIZ_LBSQLIST, 1, Ct);
							break;
							} // if
						} // for
					} // if
				++Ct;
				} // if
			else
				{
				FoundIt = 0;
				for (TempCt = Ct + 1; TempCt < NumListItems; TempCt++)
					{
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildSQListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_MWIZ_LBSQLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_MWIZ_LBSQLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_MWIZ_LBSQLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt-- ; TempCt >= Ct; TempCt--)
						{
						WidgetLBDelete(IDC_MWIZ_LBSQLIST, TempCt);
						NumListItems --;
						} // for
					++Ct;
					} // if
				else
					{
					BuildSQListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_MWIZ_LBSQLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_MWIZ_LBSQLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_MWIZ_LBSQLIST, 1, Place);
								break;
								} // if
							} // for
						} // if
					++NumListItems;
					++Ct;
					} // else
				} // if
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_MWIZ_LBSQLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));

} // MergerWizGUI::BuildSQList

/*===========================================================================*/

void MergerWizGUI::BuildSQListEntry(char *ListName, GeneralEffect *Me)
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

} // MergerWizGUI::BuildSQListEntry()

/*===========================================================================*/

void MergerWizGUI::ChangeSQListPosition(short MoveUp)
{
RasterAnimHost *MoveMe;
EffectList *Current, *PrevSQ = NULL, *PrevPrevSQ = NULL, *StashSQ;
NotifyTag Changes[2];
long Ct, NumListEntries, SendNotify = 0;

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_MWIZ_LBSQLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; ++Ct)
			{
			if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, Ct))
					{
					Current = Merger.Queries;
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
								Merger.Queries = Current;
								Merger.Queries->Next = PrevSQ;
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
			if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, Ct))
					{
					Current = Merger.Queries;
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
								Merger.Queries = Current->Next;
								Merger.Queries->Next = Current;
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
	Changes[0] = MAKE_ID(0xff, Merger.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Merger.GetRAHostRoot());
	} // if

} // MergerWizGUI::ChangeSQListPosition

/*===========================================================================*/

void MergerWizGUI::ConfigureWidgets(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *testCS;
long listPos, numEntries;
#endif // WCS_BUILD_VNS

/*
	WCS_MERGERWIZ_WIZPAGE_CANCEL,
	WCS_MERGERWIZ_WIZPAGE_COMPLETE,
	WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR,
	WCS_MERGERWIZ_WIZPAGE_CONFIRM,
	WCS_MERGERWIZ_WIZPAGE_COORDSYS,
	WCS_MERGERWIZ_WIZPAGE_DEMSLOADED,
	WCS_MERGERWIZ_WIZPAGE_MERGENAMES,
	WCS_MERGERWIZ_WIZPAGE_MERGERES,
	WCS_MERGERWIZ_WIZPAGE_MERGETYPE,
	WCS_MERGERWIZ_WIZPAGE_PAGEERROR,
	WCS_MERGERWIZ_WIZPAGE_QUERYEDIT,
	WCS_MERGERWIZ_WIZPAGE_QUERYMAKE,
	WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS,
	WCS_MERGERWIZ_WIZPAGE_WELCOME,
*/

// WCS_MERGERWIZ_WIZPAGE_BOUNDS
ConfigureSR(NativeWin, IDC_MWIZ_BOUNDSQ, IDC_MWIZ_BOUNDSQ, &boundType, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_BOUNDSQ, IDC_MWIZ_BOUNDMAN, &boundType, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_BOUNDSQ, IDC_MWIZ_BOUNDVIEW, &boundType, SRFlag_Char, 2, NULL, NULL);

// WCS_MERGERWIZ_WIZPAGE_BOUNDS1
ConfigureFI(NativeWin, IDC_MWIZ_FIN, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, 1.0, -90.0, 90.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIS, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, 0.0, -90.0, 90.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIE, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, 1.0, -180.0, 180.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIW, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, 0.0, -180.0, 180.0, FIOFlag_Double, NULL, 0);

// WCS_MERGERWIZ_WIZPAGE_BOUNDS2
ConfigureFI(NativeWin, IDC_MWIZ_FIHN, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, 1.0, -90.0, 90.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIHS, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, 0.0, -90.0, 90.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIHE, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, 1.0, -180.0, 180.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_MWIZ_FIHW, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, 0.0, -180.0, 180.0, FIOFlag_Double, NULL, 0);

// WCS_MERGERWIZ_WIZPAGE_DEMSLOADED
ConfigureSR(NativeWin, IDC_MWIZ_DEMSNO, IDC_MWIZ_DEMSNO, &haveDEMs, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_DEMSNO, IDC_MWIZ_DEMSYES, &haveDEMs, SRFlag_Char, 1, NULL, NULL);

// WCS_MERGERWIZ_WIZPAGE_MERGENAMES
WidgetSetModified(IDC_MWIZ_EDITMERGENAME, FALSE);
WidgetSetModified(IDC_MWIZ_EDITDEMNAME, FALSE);
WidgetSetModified(IDC_MWIZ_EDITHIRESNAME, FALSE);

// WCS_MERGERWIZ_WIZPAGE_MERGERES
ConfigureSR(NativeWin, IDC_MWIZ_RESMAN, IDC_MWIZ_RESMAN, &resAuto, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_RESMAN, IDC_MWIZ_RESSQ, &resAuto, SRFlag_Char, 1, NULL, NULL);

// WCS_MERGERWIZ_WIZPAGE_MERGETYPE
ConfigureSR(NativeWin, IDC_MWIZ_NORMAL, IDC_MWIZ_NORMAL, &Merger.MultiRes, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_NORMAL, IDC_MWIZ_MULTIRES, &Merger.MultiRes, SRFlag_Char, 1, NULL, NULL);

// WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS
ConfigureSR(NativeWin, IDC_MWIZ_SQYES, IDC_MWIZ_SQYES, &Wizzer.queryStatus, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_SQYES, IDC_MWIZ_SQNO, &Wizzer.queryStatus, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_MWIZ_SQYES, IDC_MWIZ_SQDB, &Wizzer.queryStatus, SRFlag_Char, 2, NULL, NULL);

#ifdef WCS_BUILD_VNS
if (Merger.MergeCoordSys)
	{
	listPos = -1;
	numEntries = WidgetCBGetCount(IDC_MWIZ_CBCOORDS);
	for (long ct = 0; ct < numEntries; ct++)
		{
		if ((testCS = (CoordSys *)WidgetCBGetItemData(IDC_MWIZ_CBCOORDS, ct)) != (CoordSys *)CB_ERR && (testCS == Merger.MergeCoordSys))
			{
			listPos = ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_MWIZ_CBCOORDS, listPos);
	} // if
else
	WidgetCBSetCurSel(IDC_MWIZ_CBCOORDS, -1);
#endif // WCS_BUILD_VNS

ConfigureTB(NativeWin, IDC_MWIZ_ADDSQ, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_MWIZ_REMOVESQ, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MWIZ_MOVESQUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MWIZ_MOVESQDOWN, IDI_ARROWDOWN, NULL);

BuildSQList();

} // MergerWizGUI::ConfigureWidgets

/*===========================================================================*/

NativeGUIWin MergerWizGUI::Construct(void)
{
GeneralEffect *myEffect;
long tabIndex;
unsigned short PanelCt = 0;

#ifdef WCS_BUILD_VNS
if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMWIZ, LocalWinSys()->RootWin);

	// these must be in the same order as the defines for the page numbers
	for (PanelCt = 0; PanelCt < WCS_MERGERWIZ_NUMPAGES; PanelCt++)
		{
		CreateSubWinFromTemplate(MergerWizPageResourceID[PanelCt], 0, PanelCt, false);
		} // for

	if (NativeWin)
		{
		WidgetCBInsert(IDC_MWIZ_CBCOORDS, -1, "New Coordinate System...");
		for (myEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
			{
			tabIndex = WidgetCBInsert(IDC_MWIZ_CBCOORDS, -1, myEffect->GetName());
			WidgetCBSetItemData(IDC_MWIZ_CBCOORDS, tabIndex, myEffect);
			} // for

		strcpy(Merger.Name, "MWiz Merger");
		ConfigureWidgets();
		DisplayPage();
		} // if
	} // if
#endif // WCS_BUILD_VNS

return (NativeWin);

} // MergerWizGUI::Construct

/*===========================================================================*/

bool MergerWizGUI::CreateLayers(void)
{
//LayerEntry *layerEntry;
EffectList **curEL;
SearchQuery **curSQ;
GeneralEffect *genFX;
struct tm *newTime;
time_t szClock;
bool success = false;
char name[32], timeString[16];
NotifyTag Changes[2];

if (GlobalApp->GUIWins->DBE)
	{
	if (UserMessageOKCAN("Create Search Query", "If desired DEMs are selected in the Database Editor click \"OK\". If not then click \"Cancel\", multi-select the DEMs and click \"Create Search Query\" again."))
		{
		// try to create a unique ID from project name & time
		strcpy(name, "MWiz_");
		strncat(name, GlobalApp->MainProj->projectname, 8);
		time(&szClock); // Get time in seconds
		newTime = gmtime(&szClock);
		sprintf(timeString, "_%d%02d%02d%02d%02d%02d", newTime->tm_year, newTime->tm_mon + 1, newTime->tm_mday,
				newTime->tm_hour, newTime->tm_min, newTime->tm_sec);
		strcat(name, timeString);

		// F2_NOTE: Should check layers to make sure name is truly unique
		// F2_NOTE: Probably want to store layer name or pointer so we can delete it if wizard is cancelled
		//layerEntry = DBHost->DBLayers.MatchMakeLayer(name, 0);
		// broadcast a freeze so that we don't disrupt the DBE selection state
		Changes[1] = NULL;
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		success = GlobalApp->GUIWins->DBE->CreateSelectedLayer(name, true);
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		if (! success)
			UserMessageOK("Create Search Queries", "There are no DEMs selected in the Database Editor!");
		else
			{
			curEL = &Merger.Queries;
			while (*curEL)
				curEL = &(*curEL)->Next;
			curSQ = &sq;
			while (*curSQ)
				curSQ = (SearchQuery **)&(*curSQ)->Next;
			if ((*curEL = new EffectList) && (*curSQ = new SearchQuery))
				{
				strcpy((*curSQ)->Name, name);
				(*curEL)->Me = *curSQ;
				// leave only DEM, Enabled & Disabled set
				(*curSQ)->Filters->PassControlPt = 0;
				(*curSQ)->Filters->PassLine = 0;
				(*curSQ)->Filters->PassPoint = 0;
				(*curSQ)->Filters->PassVector = 0;
				// set to match our newly created layer
				(*curSQ)->Filters->LayerEquals = 1;
				(*curSQ)->Filters->NewLayer(name);
				genFX = EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_SEARCHQUERY, name, (*curEL)->Me);
				Merger.AddQuery(genFX);
				} // if
			// F2_NOTE: else error
			} // else
		} // if
	} // if
else
	{
	UserMessageOK("Create Search Queries", "When the Database Editor opens, select the desired DEMs and then click \"Create Search Queries\" again.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_DBG, 0);
	} // else

return success;

} // MergerWizGUI::CreateLayers

/*===========================================================================*/

void MergerWizGUI::DisplayPage(void)
{
//char dtxt[64];

if (ActivePage)
	{
	char nextText[15];
	char disability;

	WidgetSetText(IDC_IMWIZTEXT, ActivePage->Text);

	if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_MERGERES)
		WidgetSRSync(IDC_MWIZ_RESMAN, WP_SRSYNC_NONOTIFY);	// needed since initial value is non-zero

	//sprintf(dtxt, "ActivePage = %d\n", ActivePage->WizPageID);
	//OutputDebugString(dtxt);
	strcpy(nextText, "Next -->");
	if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_COMPLETE ||
		ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR ||
		ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_PAGEERROR)
		strcpy(nextText, "Close");
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_MERGENAMES)
		strcpy(nextText, "Finish");
	//else if (ActivePage->Next)
	//	strcpy(nextText, "Next -->");
	//else
	//	strcpy(nextText, "Finish");
	WidgetSetText(IDC_NEXT, nextText);

	disability = ((ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_SELECTDEMS) && ! Merger.Queries) ||
		(ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_CANCEL) ||
		((ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_DEMSLOADED) && (! haveDEMs)) ||
		((ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS) && (Wizzer.queryStatus == 0));
	WidgetSetDisabled(IDC_NEXT, disability);
	WidgetSetDisabled(IDC_PREV, ActivePage->Prev && ActivePage->Prev->WizPageID != WCS_MERGERWIZ_WIZPAGE_WELCOME && ActivePage->WizPageID != WCS_MERGERWIZ_WIZPAGE_COMPLETE ? 0: 1);
	WidgetSetDisabled(IDCANCEL, ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_COMPLETE || ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR);
	SelectPanel(ActivePage->WizPageID);
	// WCS_MERGERWIZARD_ADDPAGE - if there are specific items that need to be reconfigured or disabled do it here
	} // if

} // MergerWizGUI::DisplayPage

/*===========================================================================*/

void MergerWizGUI::DoCancel(void)
{

/***
if (Wizzer.cancelOrder)
	Wizzer.finalOrderCancelled = 1;
Wizzer.cancelOrder = 1;
if (ActivePage = Wizzer.ProcessPage(ActivePage, ProjectHost))
	DisplayPage();
else
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_MWZ, 0);
***/

if (UserMessageYN("DEM Merger Wizard", "Do you really wish to cancel?"))
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_MWZ, 0);

} // MergerWizGUI::DoCancel

/*===========================================================================*/

void MergerWizGUI::DoNext(void)
{
long advance = true;

if (ActivePage)
	{
	if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS)
		{
		WidgetSetText(IDC_MWIZ_EDITMERGENAME, Merger.Name);
		WidgetSetText(IDC_MWIZ_EDITDEMNAME, Merger.DEMName);
		WidgetSetText(IDC_MWIZ_EDITHIRESNAME, Merger.HiResName);
		if (Merger.MultiRes)
			{
			WidgetSetDisabled(IDC_MWIZ_EDITHIRESNAME, false);
			WidgetSetDisabled(IDC_MWIZ_STATICHIRES, false);
			} // if
		else
			{
			WidgetSetDisabled(IDC_MWIZ_EDITHIRESNAME, true);
			WidgetSetDisabled(IDC_MWIZ_STATICHIRES, true);
			} // else
		if ((! Merger.DEMName[0]) || (Merger.MultiRes && (! Merger.HiResName[0])))
			WidgetSetDisabled(IDC_NEXT, false);
		else
			WidgetSetDisabled(IDC_NEXT, true);
		} // if WCS_MERGERWIZ_WIZPAGE_SEARCHQUERIES
	else if (WCS_MERGERWIZ_WIZPAGE_MERGERES == ActivePage->WizPageID)
		{
		if (!resAuto)
			{
			AnimDoubleTime tempADT;

			tempADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			tempADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			tempADT.SetValue(30.0);
			// defaults in case they cancel
			Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetCurValue(30.0);
			Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetCurValue(30.0);
			if (GetInputValue("Set Merge Resolution:", &tempADT))
				{
				Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetCurValue(tempADT.CurValue);
				Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetCurValue(tempADT.CurValue);
				if (Merger.MultiRes)
					{
					bool valid = false;
					bool cancelled = false;

					tempADT.SetValue(2.0);
					// a default if they cancel
					tempADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
					Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].SetCurValue(2.0);
					do
						{
						if (GetInputValue("Set Divider for High-Res DEM:", &tempADT))
							{
							int value;

							value = (int)tempADT.CurValue;
							if ((value > 1) && (value == tempADT.CurValue))
								valid = true;
							else
								UserMessageOK("Illegal Value", "Enter an integer value of 2 or more");
							} // if
						else
							cancelled = true;
						} while (!valid && !cancelled);
					Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].SetCurValue(tempADT.CurValue);
					} // if
				} // if
			} // if
		} // else if WCS_MERGERWIZ_WIZPAGE_MERGERES
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_MERGETYPE)
		{
		if (haveDEMs)
			WidgetSetDisabled(IDC_NEXT, false);
		else
			WidgetSetDisabled(IDC_NEXT, true);
		} // else if WCS_MERGERWIZ_WIZPAGE_MERGETYPE
/***
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_COORDSYS)
		;
***/
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_BOUNDS)
		{
		Wizzer.highres = Merger.MultiRes;
		if (boundType == 1)
			{
			Wizzer.manualBounds = true;
			if (! Merger.MergeCoordSys->GetGeographic())
				{
				// WCS_MERGERWIZ_WIZPAGE_BOUNDS1
				ConfigureFI(NativeWin, IDC_MWIZ_FIN, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, 1000000.0, 0.0, 10000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIS, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, 1000000.0, 0.0, 10000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIE, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, 500000.0, 0.0, 5000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIW, &Merger.NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, 500000.0, 0.0, 5000000.0, FIOFlag_Double, NULL, 0);

				// WCS_MERGERWIZ_WIZPAGE_BOUNDS2
				ConfigureFI(NativeWin, IDC_MWIZ_FIHN, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, 1000000.0, 0.0, 10000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIHS, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, 1000000.0, 0.0, 10000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIHE, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, 500000.0, 0.0, 5000000.0, FIOFlag_Double, NULL, 0);
				ConfigureFI(NativeWin, IDC_MWIZ_FIHW, &Merger.HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue, 500000.0, 0.0, 5000000.0, FIOFlag_Double, NULL, 0);
				} // if
			} // if
		else
			Wizzer.manualBounds = false;
		if (boundType == 2)
			SetBounds(NULL);
		} // else if
/***
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_BOUNDS)
		{
		printf("Here");
		} // else if
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_BOUNDS1)
		{
		printf("Here");
		} // else if
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_BOUNDS2)
		{
		printf("Here");
		} // else if
***/
	//else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_MERGERES)
	//	WidgetSetText(IDC_NEXT, "Finish");
	else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_MERGENAMES)
		PlugItIn();
	//else if (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_DEMSLOADED)
	//	if (! haveDEMs)
	//		{
	//		advance = false;
	//		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_IWG, 0);
	//		} // if
	//	;

	// F2_NOTE: We shouldn't advance while an action is happening (bounds in view, IW, etc)
	if (advance)
		{
		if (ActivePage = Wizzer.ProcessPage(ActivePage, ProjectHost))
			DisplayPage();
		else
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,	WCS_TOOLBAR_ITEM_MWZ, 0);
		} // if
	} // if

} // MergerWizGUI::DoNext

/*===========================================================================*/

void MergerWizGUI::DoPrev(void)
{

WidgetSetText(IDC_NEXT, "Next -->");

if (ActivePage->Prev)
	{
	/***
	if (Wizzer.CMapColorResponseEnabled)	// color map match specification
		ProcessMatchColor(TRUE, FALSE);
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO)	// eco info
		{
		ProcessEcoUnit();
		ConfigureEcoData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO)	// poly info
		{
		ProcessClass();
		ConfigureClassData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD)	// poly multi species size
		{
		ProcessSpecies();
		ConfigureSpeciesData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD)	// poly multi species diameter
		{
		ProcessDBH();
		ConfigureDBHData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD)	// poly multi species diameter
		{
		ProcessSPDens();
		ConfigureSPDensData(TRUE);
		} // else if
	***/
	Wizzer.cancelOrder = Wizzer.finalOrderCancelled = 0;
	ActivePage = ActivePage->Prev;
	ActivePage->Revert();
	DisplayPage();
	} // if

} // MergerWizGUI::DoPrev

/*===========================================================================*/

long MergerWizGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
	case IDCANCEL:
		DoCancel();
		break;
	case IDC_NEXT:
		DoNext();
		break;
	case IDC_PREV:
		DoPrev();
		break;
	case IDC_MWIZ_ADDSQ:
		AddSQ();
		break;
	case IDC_MWIZ_COORDS:
		if (Merger.MergeCoordSys)
			Merger.MergeCoordSys->EditRAHost();
		break;
	case IDC_MWIZ_CREATESQ:
		CreateLayers();
		WidgetSetDisabled(IDC_NEXT, ! Merger.Queries);
		break;
	case IDC_MWIZ_GRABALL:
		Merger.GrabAllQueries();
		break;
	case IDC_MWIZ_MOVESQUP:
		ChangeSQListPosition(1);
		break;
	case IDC_MWIZ_MOVESQDOWN:
		ChangeSQListPosition(0);
		break;
	case IDC_MWIZ_REMOVESQ:
		RemoveSQ();
		break;
	case IDC_WIZ_SAVEPROJECT:
		// we already made sure a project was loaded in the constructor
		ProjectHost->Save(NULL, NULL, DBHost, EffectsHost, ImageHost, NULL, 0xffffffff);
		#ifndef WCS_BUILD_DEMO
		ProjectHost->SavePrefs(AppScope->GetProgDir());
		#endif // WCS_BUILD_DEMO
		DoNext();
		break;
	case IDC_WIZ_CANCELORDER:
		{
		Wizzer.finalOrderCancelled = 1;
		DoNext();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // MergerWizGUI::HandleButtonClick

/*===========================================================================*/

long MergerWizGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_MWIZ_CBCOORDS:
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
	default:
		break;
	} // switch CtrlID

return(0);

} // MergerWizGUI::HandleCBChange

/*===========================================================================*/

long MergerWizGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_MWZ, 0);

return(0);

} // MergerWizGUI::HandleCloseWin

/*===========================================================================*/

//long MergerWizGUI::HandleEvent(void)
//{
//
//if (Activity->Type != 2)
//	printf("yo!");
//BuildSQList();
//return(0);
//
//} // MergerWizGUI::HandleEvent

/*===========================================================================*/

long MergerWizGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch CtrlID

return(0);

} // MergerWizGUI::HandleFIChange

/*===========================================================================*/

long MergerWizGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	case IDC_MWIZ_LBSQLIST:
//		SetActiveSQ();
//		break;
//	default:
//		break;
//	} // switch CtrlID

return (0);

} // MergerWizGUI::HandleListSel

/*===========================================================================*/

void MergerWizGUI::HandleNotifyEvent(void)
{
NotifyTag changed, *changes, interested[6];
long done = 0;
#ifdef WCS_BUILD_VNS
long curPos, pos;
GeneralEffect *myEffect, *matchEffect;
#endif // WCS_BUILD_VNS

if (! NativeWin)
	return;

changes = Activity->ChangeNotify->ChangeList;

//interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
//interested[1] = NULL;
//if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
//	{
//	SyncWidgets();
//	DisableWidgets();
//	Done = 1;
//	} // if

interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
interested[1] = NULL;
if (changed = GlobalApp->AppEx->MatchNotifyClass(interested, changes, 0))
	{
	if (ReceivingDiagnostics)
		{
		if (((changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	done = 1;
	} // if

#ifdef WCS_BUILD_VNS
interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(interested, changes, 1))
	{
	curPos = -1;
	matchEffect = Merger.MergeCoordSys;
	WidgetCBClear(IDC_MWIZ_CBCOORDS);
	WidgetCBInsert(IDC_MWIZ_CBCOORDS, -1, "New Coordinate System...");
	for (myEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
		{
		pos = WidgetCBInsert(IDC_MWIZ_CBCOORDS, -1, myEffect->GetName());
		WidgetCBSetItemData(IDC_MWIZ_CBCOORDS, pos, myEffect);
		if (myEffect == matchEffect)
			curPos = pos;
		} // for
	WidgetCBSetCurSel(IDC_MWIZ_CBCOORDS, curPos);
	interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(interested, changes, 0))
		{
		done = 1;
		} // if
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
interested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
interested[3] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
interested[4] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(interested, changes, 0))
	{
	BuildSQList();
	} // if Search Query change

if (! done)
	ConfigureWidgets();

} // MergerWizGUI::HandleNotifyEvent

/*===========================================================================*/

long MergerWizGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch

return(0);

} // MergerWizGUI::HandleSCChange

/*===========================================================================*/

long MergerWizGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
char disability;

switch (CtrlID)
	{
	case IDC_MWIZ_DEMSNO:
	case IDC_MWIZ_DEMSYES:
		WidgetSetDisabled(IDC_NEXT, ! haveDEMs);
		break;
	case IDC_MWIZ_SQYES:
	case IDC_MWIZ_SQNO:
	case IDC_MWIZ_SQDB:
		disability = (ActivePage->WizPageID == WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS) && (Wizzer.queryStatus == 0);
		WidgetSetDisabled(IDC_NEXT, disability);
		break;
	default:
		break;
	} // switch

return(0);

} // MergerWizGUI::HandleSRChange

/*===========================================================================*/

long MergerWizGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_MWIZ_EDITMERGENAME:
	case IDC_MWIZ_EDITDEMNAME:
	case IDC_MWIZ_EDITHIRESNAME:
		{
		NameChange();
		break;
		} // 
	} // switch CtrlID

return (0);

} // MergerWizGUI::HandleStringLoseFocus

/*===========================================================================*/

MergerWizGUI::MergerWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource)
: GUIFenetre('MWIZ', this, "Merger Wizard")
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_DEMMERGER, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), // add search query to Merger
								0};
double RangeDefaults[3] = {10000.0, 0.0, 1.0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
ProjectHost = ProjectSource;
ActivePage = &Wizzer.Wizzes[WCS_MERGERWIZ_WIZPAGE_WELCOME];
addedLayer = false;
boundType = 0;
haveCS = 0;
haveDEMs = 0;
Wizzer.queryStatus = 0;
ReceivingDiagnostics = 0;
resAuto = 1;
sq = NULL;

if (ProjectHost->ProjectLoaded)
	{
	#ifdef WCS_BUILD_VNS
	if (EffectsHost && DBHost && ImageHost && ProjectHost)
		{
		GlobalApp->AppEx->RegisterClient(this, AllEvents);
		ConstructError = 0;
		} // if
	else
	#endif // WCS_BUILD_VNS
		ConstructError = 1;
	} // if
else
	{
	UserMessageOK("Merger Wizard", "There is no Project in memory. You must load or create a Project before you can use the Merger Wizard.");
	ConstructError = 1;
	} // else

} // MergerWizGUI::MergerWizGUI

/*===========================================================================*/

MergerWizGUI::~MergerWizGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // MergerWizGUI::~MergerWizGUI()

/*===========================================================================*/

void MergerWizGUI::NameChange(void)
{
NotifyTag Changes[2];

if (WidgetGetModified(IDC_MWIZ_EDITMERGENAME))
	{
	WidgetGetText(IDC_MWIZ_EDITMERGENAME, WCS_EFFECT_MAXNAMELENGTH, Merger.Name);
	WidgetSetModified(IDC_MWIZ_EDITMERGENAME, false);
	Changes[0] = MAKE_ID(Merger.GetNotifyClass(), Merger.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Merger.GetRAHostRoot());
	} // if

if (WidgetGetModified(IDC_MWIZ_EDITDEMNAME))
	{
	WidgetGetText(IDC_MWIZ_EDITDEMNAME, 64, Merger.DEMName);
	WidgetSetModified(IDC_MWIZ_EDITDEMNAME, false);
	Changes[0] = MAKE_ID(Merger.GetNotifyClass(), Merger.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Merger.GetRAHostRoot());
	} // if

if (WidgetGetModified(IDC_MWIZ_EDITHIRESNAME))
	{
	WidgetGetText(IDC_MWIZ_EDITHIRESNAME, 64, Merger.HiResName);
	WidgetSetModified(IDC_MWIZ_EDITHIRESNAME, false);
	Changes[0] = MAKE_ID(Merger.GetNotifyClass(), Merger.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Merger.GetRAHostRoot());
	} // if

} // MergerWizGUI::NameChange

/*===========================================================================*/

NativeGUIWin MergerWizGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // MergerWizGUI::Open

/*===========================================================================*/

void MergerWizGUI::PlugItIn(void)
{
GeneralEffect *gfx;
double tempDiv, tempRes;
bool setRes = (resAuto == 1);	// this avoids a compiler performance warning

tempRes = Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
tempDiv = Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue;

// F2_NOTE: test only for now
//if (boundType == 0)
	Merger.ScanForRes(DBHost);

if (!resAuto)
	{
	Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetCurValue(tempRes);
	Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetCurValue(tempRes);
	Merger.AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].SetCurValue(tempDiv);
	} // if

Merger.UpdateBounds(0, DBHost, ProjectHost, setRes);
if (Merger.MultiRes)
	Merger.UpdateBounds(1, DBHost, ProjectHost, setRes);

// Add our merger object to the Effects Library
gfx = EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_DEMMERGER, NULL, &Merger);

} // MergerWizGUI::PlugItIn

/*===========================================================================*/

void MergerWizGUI::RemoveSQ(void)
{
RasterAnimHost **RemoveItems;
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;

if ((NumListEntries = WidgetLBGetCount(IDC_MWIZ_LBSQLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; ++Ct)
		{
		if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, Ct))
			{
			++NumSelected;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct++)
				{
				if (WidgetLBGetSelState(IDC_MWIZ_LBSQLIST, Ct))
					{
					RemoveItems[Found++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct++)
				{
				if (RemoveItems[Ct])
					{
					if (Merger.FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
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

} // MergerWizGUI::RemoveSQ

/*===========================================================================*/

void MergerWizGUI::SetBounds(DiagnosticData *Data)
{

if (ReceivingDiagnostics == 0)
	{
	if (UserMessageOKCAN("Set Geographic Bounds", "The next two points clicked in any View\n will become this DEM Merger's new bounds.\n\nPoints may be selected in any order."))
		{
		ReceivingDiagnostics = 1;
		GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		//WidgetSetText(IDC_IMWIZTEXT, "Set the 1st point.");
		} // if
	} // if
else if (ReceivingDiagnostics == 1)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			ReceivingDiagnostics = 2;
			//WidgetSetText(IDC_IMWIZTEXT, "Set the 2nd point.");
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else if (ReceivingDiagnostics == 2)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			//Active->GeoReg.SetBounds(Active->Coords, LatEvent, LonEvent);
			//BackupPrevBounds();
			ReceivingDiagnostics = 0;
			ConfigureWidgets();
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

//WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);

} // MergerWizGUI::SetBounds

/*===========================================================================*/

void MergerWizGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long current;
//NotifyTag Changes[2];

current = WidgetCBGetCurSel(IDC_MWIZ_CBCOORDS);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_MWIZ_CBCOORDS, current, 0)) != (CoordSys *)CB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Merger.SetCoords(NewObj);
	} // if
#endif // WCS_BUILD_VNS

} // MergerWizGUI::SelectNewCoords

/*===========================================================================*/

void MergerWizGUI::SelectPanel(unsigned short PanelID)
{

ShowPanel(0, PanelID);

} // MergerWizGUI::SelectPanel

/*===========================================================================*/

//void MergerWizGUI::SetActiveSQ(void)
//{
//long Current;
//SearchQuery *newSQ;
//
//Current = WidgetLBGetCurSel(IDC_MWIZ_LBSQLIST);
//if ((newSQ = (SearchQuery *)WidgetLBGetItemData(IDC_MWIZ_LBSQLIST, Current, 0)) != (SearchQuery *)LB_ERR && newSQ)
//	{
//	if (newSQ != ActiveJob)
//		{
//		ActiveJob = newSQ;
//		ConfigureJobWidgets();
//		} // if
//	} // if
//else
//	ConfigureJobWidgets();
//
//} // MergerWizGUI::SetActiveSQ
