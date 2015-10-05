// DEMEditGUI.cpp
// Code for DEM editor
// Built from scratch on 1/27/01 by Gary R. Huber
// Copyright 2001 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "DEMEditGUI.h"
#include "DEMPaintGUI.h"
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
#include "resource.h"

char *DEMEditGUI::TabNames[WCS_DEMGUI_NUMTABS] = {"Elevations", "File"};

long DEMEditGUI::ActivePage;
long DEMEditGUI::ActivePanel;

/*===========================================================================*/

bool DEMEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANUNDO:
	case WCS_FENETRE_WINCAP_CANNEXT:
	case WCS_FENETRE_WINCAP_CANPREV:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // DEMEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin DEMEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // DEMEditGUI::Open

/*===========================================================================*/

NativeGUIWin DEMEditGUI::Construct(void)
{
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_DEMEDIT_ELEV, 0, 0);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_DEMEDIT_FILE_VNS, 0, 1);
	// this string is so long it bombs the Mac resource compiler, so we leave it blank until here and set it on the fly.
	WidgetSetText(IDC_STATIC_REGCOORD, "Registration Coordinates below are referenced to the Coordinate System selected above or in degrees if there is no selection. You can set the units and the way the values are displayed in the Preferences Editor (CTRL+=).");
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_DEMEDIT_FILE, 0, 1);
	#endif // WCS_BUILD_VNS

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_DEMGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		if (Active)
			NewDEMFile();
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // DEMEditGUI::Construct

/*===========================================================================*/

DEMEditGUI::DEMEditGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, Joe *ActiveSource)
: GUIFenetre('DEME', this, "DEM Editor") // Yes, I know...
{
double RangeDefaults[3];
static NotifyTag AllEvents[] = {MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
Active = ActiveSource;
RowOffset = ColOffset = 0;
ActiveModified = 0;
ActiveDEM = NULL;
NULLValue = -9999.0f;

RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = -FLT_MAX;
RangeDefaults[2] = 1.0;
NorthADT.SetRangeDefaults(RangeDefaults);
NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
NorthADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
SouthADT.SetRangeDefaults(RangeDefaults);
SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
SouthADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
WestADT.SetRangeDefaults(RangeDefaults);
WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
WestADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
EastADT.SetRangeDefaults(RangeDefaults);
EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
EastADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

if (EffectsSource && DBSource && ProjHost)
	{
	if (Active)
		sprintf(NameStr, "DEM Editor - %s", Active->GetBestName());
	else
		strcpy(NameStr, "DEM Editor");
	SetTitle(NameStr);
	//Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	if (ActiveDEM = new DEM)
		{
		if (Active)
			{
			if (ActiveDEM->AttemptLoadDEM(Active, 1, ProjHost))
				{
				} // if
			else
				ConstructError = 1;
			} // if
		} // if
	else
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // DEMEditGUI::DEMEditGUI

/*===========================================================================*/

DEMEditGUI::~DEMEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (ActiveDEM)
	delete ActiveDEM;

} // DEMEditGUI::~DEMEditGUI()

/*===========================================================================*/

long DEMEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DEM, 0);

return(0);

} // DEMEditGUI::HandleCloseWin

/*===========================================================================*/

long DEMEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
#endif // WCS_BUILD_VNS

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DEM, 0);
		break;
		} // 
	case IDCANCEL:
	case IDC_WINUNDO:
		{
		ReloadDEM();
		break;
		} // 
	case IDC_PREV:
		{
		PrevDEM();
		return (1);
		} //
	case IDC_NEXT:
		{
		NextDEM();
		return (1);
		} //
	case IDC_SAVEDEM:
		{
		SaveDEM();
		break;
		} // IDC_SAVEDEM
	case IDC_RELOAD:
		{
		ReloadDEM();
		break;
		} // IDC_RELOAD
	case IDC_PAINT:
		{
		if (GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->GetActive() != GetActive())
			{
			delete GlobalApp->GUIWins->DPG;
			GlobalApp->GUIWins->DPG = NULL;
			}
		if (! GlobalApp->GUIWins->DPG)
			GlobalApp->GUIWins->DPG = new DEMPaintGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->MainProj, GetActive());
		if (GlobalApp->GUIWins->DPG)
			{
			if (GlobalApp->GUIWins->DPG->ConstructError)
				{
				delete GlobalApp->GUIWins->DPG;
				GlobalApp->GUIWins->DPG = NULL;
				} // if
			else
				GlobalApp->GUIWins->DPG->Open(GlobalApp->MainProj);
			}
		break;
		} // PAINT
	#ifdef WCS_BUILD_VNS
	case IDC_EDITCOORDS:
		{
		if (Active)
			{
			if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				{
				if (MyAttr->Coord)
					MyAttr->Coord->EditRAHost();
				} // if
			} // if
		break;
		} // IDC_EDITCOORDS
	#endif // WCS_BUILD_VNS
	default:
		break;
	} // ButtonID

return(0);

} // DEMEditGUI::HandleButtonClick

/*===========================================================================*/

long DEMEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
long Row = -1, Col = -1;

GetRowColFromElevWidget(CtrlID, Row, Col);

if (Row >= 0 && Col >= 0)
	SetNewElev(CtrlID, Row, Col);

return (0);

} // DEMEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long DEMEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // DEMEditGUI::HandleCBChange

/*===========================================================================*/

long DEMEditGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
short SetSheet = 0;

if (ScrollCode)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			RowOffset = ScrollPos;
			SetSheet = 1;
			break;
			}
		case IDC_SCROLLBAR2:
			{
			ColOffset = ScrollPos;
			SetSheet = 1;
			break;
			}
		default:
			break;
		} // switch
	if (SetSheet)
		{
		SetRowColText();
		FillSpreadSheet();
		} // if
	return(0);
	} // if
else
	{
	return(5); // default scroll amount
	} // else

} // DEMEditGUI::HandleScroll

/*===========================================================================*/

long DEMEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				} // 0
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

} // DEMEditGUI::HandlePageChange

/*===========================================================================*/

long DEMEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKUSENULL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // DEMEditGUI::HandleSCChange

/*===========================================================================*/

long DEMEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_NORTH:
	case IDC_SOUTH:
	case IDC_WEST:
	case IDC_EAST:
		{
		if (ActiveDEM)
			{
			ActiveDEM->SetBounds(NorthADT.CurValue, SouthADT.CurValue, WestADT.CurValue, EastADT.CurValue);
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_NULLVALUE:
		{
		if (ActiveDEM)
			{
			ActiveDEM->SetNullValue(NULLValue);
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // DEMEditGUI::HandleFIChange

/*===========================================================================*/

void DEMEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;
DiagnosticData *Diagnostics;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	} // if DEM count changed

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	} // if units changed

if (Active)
	{
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		ConfigureCoords();
		Done = 1;
		} // if

	Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		DisableWidgets();
		Done = 1;
		} // if

	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		if (Diagnostics = (DiagnosticData *)Activity->ChangeNotify->NotifyData)
			{
			if (Diagnostics->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Diagnostics->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				SelectPoints(Diagnostics->Value[WCS_DIAGNOSTIC_LATITUDE], Diagnostics->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			} // if
		Done = 1;
		} // if
	} // if

if (! Done)
	ConfigureWidgets();

} // DEMEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void DEMEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

if (Active)
	sprintf(TextStr, "DEM Editor - %s", Active->GetBestName());
else
	strcpy(TextStr, "DEM Editor");
SetTitle(TextStr);

DisableWidgets();
HideWidgets();
FillSpreadSheet();

ConfigureCoords();

} // DEMEditGUI::ConfigureWidgets()

/*===========================================================================*/

void DEMEditGUI::ConfigureCoords(void)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
GeneralEffect *MyEffect;
long CurPos, Pos;
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_VNS
CurPos = -1;
if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;
WidgetCBClear(IDC_COORDSDROP);
WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
	{
	Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
	WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
	if (MyEffect == MyCoords)
		CurPos = Pos;
	} // for
WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
#endif // WCS_BUILD_VNS
if (ActiveDEM)
	{
	#ifdef WCS_BUILD_VNS
	NULLValue = ActiveDEM->NullValue();
	if (MyCoords && MyCoords->Method.GCTPMethod)
		{
		NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		} // if projected
	else
	#endif // WCS_BUILD_VNS
		{
		NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		} // else geographic
	NorthADT.SetValue(ActiveDEM->Northest());
	SouthADT.SetValue(ActiveDEM->Southest());
	WestADT.SetValue(ActiveDEM->Westest());
	EastADT.SetValue(ActiveDEM->Eastest());
	} // if
WidgetSNConfig(IDC_NORTH, &NorthADT);
WidgetSNConfig(IDC_WEST, &WestADT);
WidgetSNConfig(IDC_EAST, &EastADT);
WidgetSNConfig(IDC_SOUTH, &SouthADT);

#ifdef WCS_BUILD_VNS
ConfigureSC(NativeWin, IDC_CHECKUSENULL, &ActiveDEM->NullReject, SCFlag_Short, NULL, NULL);
ConfigureFI(NativeWin, IDC_NULLVALUE, &NULLValue, 1.0, (double)-FLT_MAX, (double)FLT_MAX, FIOFlag_Float, NULL, NULL);
#endif // WCS_BUILD_VNS

} // DEMEditGUI::ConfigureCoords

/*===========================================================================*/

void DEMEditGUI::NewDEMFile(void)
{

RowOffset = ColOffset = 0;
if (ActiveDEM->LatEntries() > 10)
	{
	WidgetSetDisabled(IDC_SCROLLBAR1, FALSE);
	WidgetSetScrollRange(IDC_SCROLLBAR1, 0, ActiveDEM->LatEntries() - 10);
	} // if
else
	WidgetSetDisabled(IDC_SCROLLBAR1, TRUE);
if (ActiveDEM->LonEntries() > 5)
	{
	WidgetSetDisabled(IDC_SCROLLBAR2, FALSE);
	WidgetSetScrollRange(IDC_SCROLLBAR2, 0, ActiveDEM->LonEntries() - 5);
	} // if
else
	WidgetSetDisabled(IDC_SCROLLBAR2, TRUE);
WidgetSetScrollPos(IDC_SCROLLBAR1, 0);
WidgetSetScrollPos(IDC_SCROLLBAR2, 0);
SetRowColText();

} // DEMEditGUI::NewDEMFile

/*===========================================================================*/

void DEMEditGUI::SetRowColText(void)
{
char RowTxt[128];

sprintf(RowTxt, "%d", RowOffset + 1);
WidgetSetText(IDC_ROW, RowTxt);
sprintf(RowTxt, "%d", ColOffset + 1);
WidgetSetText(IDC_COL, RowTxt);

} // DEMEditGUI::SetRowColText

/*===========================================================================*/

void DEMEditGUI::FillSpreadSheet(void)
{
long Row, Col, LatEntriesM1;
int WidID;
char ElevTxt[128];

if (ActiveDEM && ActiveDEM->Map())
	{
	LatEntriesM1 = ActiveDEM->LatEntries() - 1;
	for (Row = 0; Row < 10; Row ++)
		{
		for (Col = 0; Col < 5; Col ++)
			{
			if ((WidID = GetElevWidgetFromRowCol(Row, Col)) >= 0)
				{
				if (Col + ColOffset < (long)ActiveDEM->LonEntries() && Row + RowOffset <= LatEntriesM1)
					{
					sprintf(ElevTxt, "%f", ActiveDEM->Sample(Col + ColOffset, LatEntriesM1 - (Row + RowOffset)));
					TrimZeros(ElevTxt);
					WidgetSetText(WidID, ElevTxt);
					} // if
				else
					{
					WidgetSetText(WidID, "");
					} // else
				} // if
			} // for
		} // for
	} // if
else
	{
	sprintf(ElevTxt, "");
	for (Row = 0; Row < 10; Row ++)
		{
		for (Col = 0; Col < 5; Col ++)
			{
			WidID = GetElevWidgetFromRowCol(Row, Col);
			WidgetSetText(WidID, ElevTxt);
			} // for
		} // for
	} // if

} // DEMEditGUI::FillSpreadSheet

/*===========================================================================*/

int DEMEditGUI::GetElevWidgetFromRowCol(long Row, long Col)
{

switch (Row)
	{
	case 0:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_0_0);
			case 1: return (IDC_ELEV_0_1);
			case 2: return (IDC_ELEV_0_2);
			case 3: return (IDC_ELEV_0_3);
			case 4: return (IDC_ELEV_0_4);
			} // switch Col
		break;
		} // 0
	case 1:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_1_0);
			case 1: return (IDC_ELEV_1_1);
			case 2: return (IDC_ELEV_1_2);
			case 3: return (IDC_ELEV_1_3);
			case 4: return (IDC_ELEV_1_4);
			} // switch Col
		break;
		} // 1
	case 2:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_2_0);
			case 1: return (IDC_ELEV_2_1);
			case 2: return (IDC_ELEV_2_2);
			case 3: return (IDC_ELEV_2_3);
			case 4: return (IDC_ELEV_2_4);
			} // switch Col
		break;
		} // 2
	case 3:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_3_0);
			case 1: return (IDC_ELEV_3_1);
			case 2: return (IDC_ELEV_3_2);
			case 3: return (IDC_ELEV_3_3);
			case 4: return (IDC_ELEV_3_4);
			} // switch Col
		break;
		} // 3
	case 4:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_4_0);
			case 1: return (IDC_ELEV_4_1);
			case 2: return (IDC_ELEV_4_2);
			case 3: return (IDC_ELEV_4_3);
			case 4: return (IDC_ELEV_4_4);
			} // switch Col
		break;
		} // 4
	case 5:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_5_0);
			case 1: return (IDC_ELEV_5_1);
			case 2: return (IDC_ELEV_5_2);
			case 3: return (IDC_ELEV_5_3);
			case 4: return (IDC_ELEV_5_4);
			} // switch Col
		break;
		} // 5
	case 6:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_6_0);
			case 1: return (IDC_ELEV_6_1);
			case 2: return (IDC_ELEV_6_2);
			case 3: return (IDC_ELEV_6_3);
			case 4: return (IDC_ELEV_6_4);
			} // switch Col
		break;
		} // 6
	case 7:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_7_0);
			case 1: return (IDC_ELEV_7_1);
			case 2: return (IDC_ELEV_7_2);
			case 3: return (IDC_ELEV_7_3);
			case 4: return (IDC_ELEV_7_4);
			} // switch Col
		break;
		} // 7
	case 8:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_8_0);
			case 1: return (IDC_ELEV_8_1);
			case 2: return (IDC_ELEV_8_2);
			case 3: return (IDC_ELEV_8_3);
			case 4: return (IDC_ELEV_8_4);
			} // switch Col
		break;
		} // 8
	case 9:
		{
		switch (Col)
			{
			case 0: return (IDC_ELEV_9_0);
			case 1: return (IDC_ELEV_9_1);
			case 2: return (IDC_ELEV_9_2);
			case 3: return (IDC_ELEV_9_3);
			case 4: return (IDC_ELEV_9_4);
			} // switch Col
		break;
		} // 9
	} // switch Row

return (-1);

} // DEMEditGUI::GetElevWidgetFromRowCol

/*===========================================================================*/

void DEMEditGUI::GetRowColFromElevWidget(int WidID, long &Row, long &Col)
{

switch (WidID)
	{
	case IDC_ELEV_0_0:
	case IDC_ELEV_0_1:
	case IDC_ELEV_0_2:
	case IDC_ELEV_0_3:
	case IDC_ELEV_0_4:
		{
		Row = 0;
		break;
		} // 
	case IDC_ELEV_1_0:
	case IDC_ELEV_1_1:
	case IDC_ELEV_1_2:
	case IDC_ELEV_1_3:
	case IDC_ELEV_1_4:
		{
		Row = 1;
		break;
		} // 
	case IDC_ELEV_2_0:
	case IDC_ELEV_2_1:
	case IDC_ELEV_2_2:
	case IDC_ELEV_2_3:
	case IDC_ELEV_2_4:
		{
		Row = 2;
		break;
		} // 
	case IDC_ELEV_3_0:
	case IDC_ELEV_3_1:
	case IDC_ELEV_3_2:
	case IDC_ELEV_3_3:
	case IDC_ELEV_3_4:
		{
		Row = 3;
		break;
		} // 
	case IDC_ELEV_4_0:
	case IDC_ELEV_4_1:
	case IDC_ELEV_4_2:
	case IDC_ELEV_4_3:
	case IDC_ELEV_4_4:
		{
		Row = 4;
		break;
		} // 
	case IDC_ELEV_5_0:
	case IDC_ELEV_5_1:
	case IDC_ELEV_5_2:
	case IDC_ELEV_5_3:
	case IDC_ELEV_5_4:
		{
		Row = 5;
		break;
		} // 
	case IDC_ELEV_6_0:
	case IDC_ELEV_6_1:
	case IDC_ELEV_6_2:
	case IDC_ELEV_6_3:
	case IDC_ELEV_6_4:
		{
		Row = 6;
		break;
		} // 
	case IDC_ELEV_7_0:
	case IDC_ELEV_7_1:
	case IDC_ELEV_7_2:
	case IDC_ELEV_7_3:
	case IDC_ELEV_7_4:
		{
		Row = 7;
		break;
		} // 
	case IDC_ELEV_8_0:
	case IDC_ELEV_8_1:
	case IDC_ELEV_8_2:
	case IDC_ELEV_8_3:
	case IDC_ELEV_8_4:
		{
		Row = 8;
		break;
		} // 
	case IDC_ELEV_9_0:
	case IDC_ELEV_9_1:
	case IDC_ELEV_9_2:
	case IDC_ELEV_9_3:
	case IDC_ELEV_9_4:
		{
		Row = 9;
		break;
		} // 
	} // switch WidID

switch (WidID)
	{
	case IDC_ELEV_0_0:
	case IDC_ELEV_1_0:
	case IDC_ELEV_2_0:
	case IDC_ELEV_3_0:
	case IDC_ELEV_4_0:
	case IDC_ELEV_5_0:
	case IDC_ELEV_6_0:
	case IDC_ELEV_7_0:
	case IDC_ELEV_8_0:
	case IDC_ELEV_9_0:
		{
		Col = 0;
		break;
		} // 
	case IDC_ELEV_0_1:
	case IDC_ELEV_1_1:
	case IDC_ELEV_2_1:
	case IDC_ELEV_3_1:
	case IDC_ELEV_4_1:
	case IDC_ELEV_5_1:
	case IDC_ELEV_6_1:
	case IDC_ELEV_7_1:
	case IDC_ELEV_8_1:
	case IDC_ELEV_9_1:
		{
		Col = 1;
		break;
		} // 
	case IDC_ELEV_0_2:
	case IDC_ELEV_1_2:
	case IDC_ELEV_2_2:
	case IDC_ELEV_3_2:
	case IDC_ELEV_4_2:
	case IDC_ELEV_5_2:
	case IDC_ELEV_6_2:
	case IDC_ELEV_7_2:
	case IDC_ELEV_8_2:
	case IDC_ELEV_9_2:
		{
		Col = 2;
		break;
		} // 
	case IDC_ELEV_0_3:
	case IDC_ELEV_1_3:
	case IDC_ELEV_2_3:
	case IDC_ELEV_3_3:
	case IDC_ELEV_4_3:
	case IDC_ELEV_5_3:
	case IDC_ELEV_6_3:
	case IDC_ELEV_7_3:
	case IDC_ELEV_8_3:
	case IDC_ELEV_9_3:
		{
		Col = 3;
		break;
		} // 
	case IDC_ELEV_0_4:
	case IDC_ELEV_1_4:
	case IDC_ELEV_2_4:
	case IDC_ELEV_3_4:
	case IDC_ELEV_4_4:
	case IDC_ELEV_5_4:
	case IDC_ELEV_6_4:
	case IDC_ELEV_7_4:
	case IDC_ELEV_8_4:
	case IDC_ELEV_9_4:
		{
		Col = 4;
		break;
		} // 
	} // switch WidID

} // DEMEditGUI::GetRowColFromElevWidget

/*===========================================================================*/

void DEMEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_NULLVALUE, ! (ActiveDEM && ActiveDEM->GetNullReject()));

} // DEMEditGUI::DisableWidgets

/*===========================================================================*/

void DEMEditGUI::HideWidgets(void)
{
} // DEMEditGUI::HideWidgets

/*===========================================================================*/

void DEMEditGUI::SetNewElev(int CtrlID, long Row, long Col)
{
float NewElev;
char NewText[128];

// add scroll pos to Row & Col
Row += RowOffset;
Col += ColOffset;

if (WidgetGetModified(CtrlID))
	{
	WidgetGetText(CtrlID, 128, NewText);
	WidgetSetModified(CtrlID, FALSE);
	NewElev = (float)atof(NewText);
	if (Row < (long)ActiveDEM->LatEntries() && Col < (long)ActiveDEM->LonEntries())
		{
		if (ActiveDEM->Map())
			{
			ActiveDEM->StoreElevation(ActiveDEM->LatEntries() - 1 - Row, Col, NewElev);
			ActiveModified = 1;
			} // if
		} // if
	} // if

} // DEMEditGUI::SetNewElev

/*===========================================================================*/

void DEMEditGUI::SelectPoints(double SelectLat, double SelectLon)
{
VertexDEM Vert;
long Row = -1, Col = -1, LatEntriesM1;
int WidID;
double dRow, dCol;

if (ActiveDEM->Map())
	{
	if (ActiveDEM->XYFromLatLon(GlobalApp->AppEffects->FetchDefaultCoordSys(), SelectLat, SelectLon, dCol, dRow))
		{
		Row = (long)dRow;
		Col = (long)dCol;
		if (Row >= (long)ActiveDEM->LatEntries())
			Row = ActiveDEM->LatEntries() - 1;
		if (Col >= (long)ActiveDEM->LonEntries())
			Col = ActiveDEM->LonEntries() - 1;
		LatEntriesM1 = ActiveDEM->LatEntries() - 1;
		RowOffset = LatEntriesM1 - Row - 5;
		ColOffset = Col - 2;
		if (RowOffset + 10 > (long)ActiveDEM->LatEntries())
			RowOffset = ActiveDEM->LatEntries() - 10;
		if (ColOffset + 5 > (long)ActiveDEM->LonEntries())
			ColOffset = ActiveDEM->LonEntries() - 5;
		if (RowOffset < 0)
			RowOffset = 0;
		if (ColOffset < 0)
			ColOffset = 0;
		SetRowColText();
		FillSpreadSheet();
		WidgetSetScrollPos(IDC_SCROLLBAR1, RowOffset);
		WidgetSetScrollPos(IDC_SCROLLBAR2, ColOffset);
		if ((WidID = GetElevWidgetFromRowCol(LatEntriesM1 - Row - RowOffset, Col - ColOffset)) >= 0)
			{
			SetFocus((HWND)GetWidgetFromID(WidID));
			WidgetEMSetSelected(WidID, 0, -1);
			} // if
		} // if
	} // if

} // DEMEditGUI::SelectPoints

/*===========================================================================*/

void DEMEditGUI::ReloadDEM(void)
{
NotifyTag Changes[2];

if (Active)
	{
	ActiveDEM->FreeRawElevs();
	ActiveDEM->AttemptLoadDEM(Active, 1, ProjHost);
	} // if

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // DEMEditGUI::ReloadDEM

/*===========================================================================*/

void DEMEditGUI::SaveDEM(void)
{
#ifndef WCS_BUILD_DEMO
char filename[512], *FilePath;
NotifyTag Changes[2];
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_DEMO

UserMessageDemo("The DEM Editor doesn't save in the demo version.");

#else // WCS_BUILD_DEMO

if (Active)
	{
	if (UserMessageOKCAN((char *)Active->GetBestName(), "Save DEM file? Changes to this DEM will affect all Projects that reference this DEM file."))
		{
		if (FilePath = ActiveDEM->AttemptFindDEMPath((char *)Active->FileName(), ProjHost))
			{
			strmfp(filename, FilePath, (char *)Active->FileName());
			strcat(filename, ".elev");
			if (ActiveDEM->SaveDEM(filename, GlobalApp->StatusLog))
				{
				Active->RecheckBounds();
				UserMessageOK((char *)Active->FileName(), "DEM has been saved.");
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				return;
				} // if
			} // if
		UserMessageOK((char *)Active->FileName(), "Error saving DEM.");
		} // if
	} // if

#endif // WCS_BUILD_DEMO

} // DEMEditGUI::SaveDEM

/*===========================================================================*/

void DEMEditGUI::NextDEM(void)
{
Joe *CurJoe, *FoundJoe = NULL;
int Found = 0;

DBHost->ResetGeoClip();

// find next DEM if there is one and activate it, then call edit on it
for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (CurJoe == Active)
			Found = 1;
		else if (Found)
			{
			FoundJoe = CurJoe;
			break;
			} // else if
		} // if
	} // for

// find the first DEM if one hasn't been found so far
if (Found && ! FoundJoe)
	{
	for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (CurJoe != Active)
				{
				FoundJoe = CurJoe;
				break;
				} // if
			} // if
		} // for
	} // if didn't find a following DEM

if (FoundJoe)
	{
	FoundJoe->EditRAHost();
	} // if

} // DEMEditGUI::NextDEM

/*===========================================================================*/

void DEMEditGUI::PrevDEM(void)
{
Joe *CurJoe, *PrevJoe = NULL, *FoundJoe = NULL;
int Found = 0;

DBHost->ResetGeoClip();

// find next DEM if there is one and activate it, then call edit on it
for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (CurJoe == Active)
			{
			Found = 1;
			FoundJoe = PrevJoe;
			break;
			} // if
		PrevJoe = CurJoe;
		} // if
	} // for

// find the last DEM if one hasn't been found so far
if (Found && ! FoundJoe)
	{
	for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			FoundJoe = CurJoe;
			} // if
		} // for
	} // if didn't find a following DEM

if (FoundJoe)
	{
	FoundJoe->EditRAHost();
	} // if

} // DEMEditGUI::PrevDEM

/*===========================================================================*/

void DEMEditGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

if (Active)
	{
	Current = WidgetCBGetCurSel(IDC_COORDSDROP);
	if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
		|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
		{
		Active->AddEffect(NewObj, 1);	// the 1 will make it not ask about replacing the current coord sys
		} // if
	} // if
#endif // WCS_BUILD_VNS

} // DEMEditGUI::SelectNewCoords

/*===========================================================================*/
