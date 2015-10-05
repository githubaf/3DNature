// VectorProfGUI.cpp
// Code for Vector Profile editor
// Built from EffectGradProfEditGUI.cpp on 6/27/97 by Gary Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "VectorProfGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Database.h"
#include "Useful.h"
#include "Conservatory.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "DEM.h"
#include "VectorScaleGUI.h"
#include "VecProfExportGUI.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "resource.h"

extern NotifyTag DBChangeEventPOINTS[];

NativeGUIWin VectorProfileGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // VectorProfileGUI::Open

/*===========================================================================*/

NativeGUIWin VectorProfileGUI::Construct(void)
{
//int ListEntry;
//char *Units[] = {"Miles", "Kilometers", "Meters", "Feet", "Inches", "Centimeters", "Decimeters", "Millimeters", "Yards"};

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_VECTOR_GRAPH, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		GraphWidget = GetDlgItem(NativeWin, IDC_GRAPH);
		// Set up Widget bindings
		//for (ListEntry=0; ListEntry<6; ListEntry++)
		//	WidgetCBInsert(IDC_WIDTHUNITSDROP, -1, Units[ListEntry]);

		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR2, 1, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR4, 0, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR5, 0, 99);

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);

} // VectorProfileGUI::Construct

/*===========================================================================*/

VectorProfileGUI::VectorProfileGUI(Database *DBSource, Joe *ActiveSource)
: GUIFenetre('VPEF', this, "Vector Profile Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {0, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, 0xff, 0xff), 
									MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, 0xff, 0xff), 0};
static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff), 0};
static NotifyTag AllInterEvents[] = {MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, 0xff, 0xff),
										0};
double MaxDist, MinDist, ValRange;

Value = Slope = 0.0;
MinSlope = -10000.0;
MaxSlope = 10000.0;
HostDB = DBSource;
Active = ActiveSource;
Backup = NULL;
VecScaleGUI = NULL;
Disabled = 0;
IgnoreNotify = 0;
Elevation = -FLT_MAX;

if (! Active && DBSource)
	{
	if (Active = DBSource->GetActiveObj())
		{
		if (Active->TestFlags(WCS_JOEFLAG_ISDEM))
			Active = NULL;
		} // if
	} // if
if (Active && DBSource)
	{
	ConstructError = 0;
	DistanceADD.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	ElevationADD.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	DistanceADD.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
	ElevationADD.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
	ValueADT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	ValueADT.SetMetricType(DistanceADD.GetMetricType());
	ValueADT.SetMultiplier(DistanceADD.GetMultiplier());
	ValueADT.SetIncrement(DistanceADD.GetIncrement());
	DistanceADT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	DistanceADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	DistanceADT.SetMultiplier(1.0);
	DistanceADT.SetIncrement(1.0);
	if (! (Backup = new ((char *)Active->Name()) Joe))
		ConstructError = 1;
	else if (! Active->CopyPoints(Backup, Active, 0, 1))
		ConstructError = 1;
	else if (! SetDistanceADD())
		ConstructError = 1;
	WidgetLocal.drawgrid = 1;
	memset(&WidgetLocal, 0, sizeof WidgetLocal);
	WidgetLocal.GrWin = this;
	WidgetLocal.Crit = &DistanceADD;
	WidgetLocal.ElevCrit = &ElevationADD;
	WidgetLocal.ValuePrototype = &ValueADT;	// setting these to NULL will mean no value or distance labels in graph
	WidgetLocal.DistancePrototype = &DistanceADT;
	WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
	WidgetLocal.drawgrid = 1;
	WidgetLocal.NumGraphs = DistanceADD.GetNumGraphs();
	WidgetLocal.HPan = 0;
	WidgetLocal.HVisible = 100;
	WidgetLocal.VPan = 0;
	WidgetLocal.VVisible = 100;
	WidgetLocal.DistGridLg = 5;
	WidgetLocal.SnapToInt = 0;
	WidgetLocal.DisplayByFrame = 0;
	WidgetLocal.MaxDistRange = (DistanceADD.GetMinMaxDist(MinDist, MaxDist) ? (MaxDist > 1.0 ? MaxDist: 1.0): 1.0);
	if (WidgetLocal.MaxDistRange < 1.0)
		WidgetLocal.MaxDistRange = 1.0;
	WidgetLocal.LowDrawDist = 0.0;
	WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;
	DistanceADD.GetMinMaxVal(WidgetLocal.MinLowDrawVal, WidgetLocal.MaxHighDrawVal);
	if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
		{
		ValRange = DistanceADD.GetIncrement();
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // if
	else
		{
		ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .1;
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // else

	Distance = 0.0;
	Value = 0.0;
	WidgetLocal.FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

	AllEvents[0] = MAKE_ID(DistanceADD.GetNotifyClass(), DistanceADD.GetNotifySubclass(), DistanceADD.GetNotifyItem(), 0xff);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllInterEvents);
	HostDB->RegisterClient(this, AllDBEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // VectorProfileGUI::VectorProfileGUI

/*===========================================================================*/

VectorProfileGUI::~VectorProfileGUI()
{

if (HostDB)
	HostDB->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
if (VecScaleGUI)
	delete VecScaleGUI;
VecScaleGUI = NULL;
if (Backup)
	delete Backup;
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // VectorProfileGUI::~VectorProfileGUI()

/*===========================================================================*/

int VectorProfileGUI::NewActiveVector(Joe *NewActive)
{

if (VecScaleGUI)
	delete VecScaleGUI;
VecScaleGUI = NULL;
if (Backup)
	delete Backup;
Backup = NULL;

Value = Slope = 0.0;
MinSlope = -10000.0;
MaxSlope = 10000.0;
Active = NewActive;

if (Active)
	{
	ConstructError = 0;
	if (! (Backup = new ((char *)Active->Name()) Joe))
		ConstructError = 1;
	else if (! Active->CopyPoints(Backup, Active, 0, 1))
		ConstructError = 1;
	else if (! SetDistanceADD())
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

return (! ConstructError);

} // VectorProfileGUI::NewActiveVector

/*===========================================================================*/

long VectorProfileGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
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
		} // switch
	return(0);
	} // if
else if (ScrollCode == 2)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR4:
			{
			WidgetLocal.VPan = 100 - (short)ScrollPos;
			RefreshGraph();
			break;
			}
		case IDC_SCROLLBAR5:
			{
			WidgetLocal.VVisible = 100 - (short)ScrollPos;
			RefreshGraph();
			break;
			}
		} // switch
	return(0);
	} // VSCROLL
else
	return(5);

} // VectorProfileGUI::HandleScroll


/*===========================================================================*/

long VectorProfileGUI::HandleCloseWin(NativeGUIWin NW)
{

if (HostDB)	// why is this necessary here since it is in the destructor?
	HostDB->RemoveClient(this);
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_VPG, 0);

return(0);

} // VectorProfileGUI::HandleCloseWin

/*===========================================================================*/

long VectorProfileGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(022, 22);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		if (HostDB)	// why is this necessary here since it is in the destructor?
			HostDB->RemoveClient(this);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		break;
		} // 
	case ID_CANCEL:
		{
		if (HostDB)	// why is this necessary here since it is in the destructor?
			HostDB->RemoveClient(this);
		Active->CopyPoints(Active, Backup, 1, 1);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		break;
		} // 
	case IDC_CHECKGRID:
		{
		InvalidateRect(GraphWidget, NULL, NULL);
		break;
		} // IDC_CHECKGRID
	case IDC_SCALE:
		{
		VectorScale();
		break;
		} // 
	case IDC_NEXTNODE:
		{
		NextNode();
		break;
		} // 
	case IDC_PREVNODE:
		{
		PrevNode();
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
	case IDC_UNDO:
		{
		Undo();
		break;
		} // 
	case IDC_RESET:
		{
		Restore();
		break;
		} // 
	#ifdef WCS_BUILD_VNS
	case IDC_EXPORT:
		{
		ExportGraph();
		break;
		} // 
	#endif // WCS_BUILD_VNS
	default:
		break;
	} // ButtonID

return(0);

} // VectorProfileGUI::HandleButtonClick

/*===========================================================================*/

long VectorProfileGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

if (NW == NativeWin)
	{
	switch(CtrlID)
		{
		case IDC_VALUEY:
			{
			Value = ValueADT.CurValue;
			NewValue();
			break;
			} // IDC_VALUEY
		case IDC_SLOPE:
			{
			SetValueBySlope();
			break;
			} // IDC_SLOPE
		case IDC_MINSLOPE:
		case IDC_MAXSLOPE:
			{
			SetValuesBySlopeLimits();
			break;
			} // IDC_MAXSLOPE
		case IDC_MINDISPLAYVAL:
			{
			WidgetLocal.MinLowDrawVal = MinValueADT.CurValue;
			RefreshGraph();
			break;
			}
		case IDC_MAXDISPLAYVAL:
			{
			WidgetLocal.MaxHighDrawVal = MaxValueADT.CurValue;
			RefreshGraph();
			break;
			}
		default:
			break;
		} // ID
	} // if

return(0);

} // VectorProfileGUI::HandleFIChange

/*===========================================================================*/

void VectorProfileGUI::HandleNotifyEvent(void)
{
int Reconfig;
NotifyTag *Changes, Interested[7];
Joe *NewActive;

if (! NativeWin || Disabled)
	return;

if (IgnoreNotify)
	{
	IgnoreNotify = 0;
	return;
	} // if

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
if (HostDB->MatchNotifyClass(Interested, Changes, 1))
	{
	ConfigureWidgets();
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
if (HostDB->MatchNotifyClass(Interested, Changes, 1))
	{
	if (NewActive = HostDB->GetActiveObj())
		{
		if (NewActive->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			NewActive = NULL;
			} // else
		} // if
	Reconfig = (Active != NewActive);
	if (NewActive && NewActiveVector(NewActive))
		{
		if (Reconfig)
			ConfigureWidgets();
		} // if
	else
		{
		if (HostDB)	// why is this necessary here since it is in the destructor?
			HostDB->RemoveClient(this);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		Disabled = 1;
		} // else
	return;
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_NEW, 0xff, 0xff);
Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRELOAD, 0xff, 0xff);
Interested[2] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_DELOBJ, 0xff, 0xff);
Interested[3] = NULL;
if (HostDB->MatchNotifyClass(Interested, Changes, 0))
	{
	if (! Activity->ChangeNotify->NotifyData || Activity->ChangeNotify->NotifyData == Active)
		{
		if (HostDB)
			HostDB->RemoveClient(this);
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		} // if
	return;
	} // if joe changed
else
	{
	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_NAME, 0xff);
	if (HostDB->MatchNotifyClass(Interested, Changes, 1))
		{
		WidgetSetText(IDC_VECNAME, Active->GetBestName());
		} // if name changed
	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
	if (HostDB->MatchNotifyClass(Interested, Changes, 1))
		{
		Slope = 0.0;
		if (! SetDistanceADD())
			{
			if (HostDB)
				HostDB->RemoveClient(this);
			GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_VPG, 0);
			Disabled = 1;
			return;
			} // if
		else
			ConfigureWidgets();
		} // if points changed
	} // else

Interested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, 0xff, 0xff);
Interested[1] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, 0xff, 0xff);
Interested[2] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_BACKUP, 0xff, 0xff);
Interested[3] = NULL;
if (HostDB->MatchNotifyClass(Interested, Changes, 0))
	{
	Interested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_FIRSTRANGEPT, 0xff);
	Interested[1] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_LASTRANGEPT, 0xff);
	Interested[2] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ADDPOINTSELECT, 0xff);
	Interested[3] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_TOGGLEPOINTSELECT, 0xff);
	Interested[4] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0xff);
	Interested[5] = NULL;
	if (HostDB->MatchNotifyClass(Interested, Changes, 0))
		{
		UpdateNodeSelection();
		RefreshGraph();
		} // if
	else if (! SetDistanceADD())
		{
		if (HostDB)
			HostDB->RemoveClient(this);
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		Disabled = 1;
		return;
		} // if
	else
		ConfigureWidgets();
	} // if vector points changed

Interested[0] = MAKE_ID(DistanceADD.GetNotifyClass(), DistanceADD.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ANIM_SELECTIONCHANGED);
Interested[1] = NULL;
if (HostDB->MatchNotifyClass(Interested, Changes, 0))
	{
	UpdateVectorSelection();
	} // if

Interested[0] = MAKE_ID(DistanceADD.GetNotifyClass(), DistanceADD.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = NULL;
if (HostDB->MatchNotifyClass(Interested, Changes, 0))
	{
	UpdateVectorElevations();
	} // if

} // VectorProfileGUI::HandleNotifyEvent()

/*===========================================================================*/

void VectorProfileGUI::ConfigureWidgets(void)
{
char Title[256];
long SelectedNodes;
double TempMinLowDrawVal, TempMaxHighDrawVal, ElevTempMinLowDrawVal, ElevTempMaxHighDrawVal, MaxDist, MinDist, ValRange;

#ifndef WCS_BUILD_VNS
WidgetShow(IDC_EXPORT, 0);
#endif // WCS_BUILD_VNS

WidgetSetScrollPos(IDC_SCROLLBAR1, WidgetLocal.HPan);
WidgetSetScrollPos(IDC_SCROLLBAR2, WidgetLocal.HVisible);
WidgetSetScrollPos(IDC_SCROLLBAR4, 100 - WidgetLocal.VPan);
WidgetSetScrollPos(IDC_SCROLLBAR5, 100 - WidgetLocal.VVisible);
ConfigureSC(NativeWin, IDC_CHECKGRID, &WidgetLocal.drawgrid, SCFlag_Short, NULL, NULL);

WidgetLocal.MaxDistRange = (DistanceADD.GetMinMaxDist(MinDist, MaxDist) ? (MaxDist > 1.0 ? MaxDist: 1.0): 1.0);
if (WidgetLocal.MaxDistRange < 1.0)
	WidgetLocal.MaxDistRange = 1.0;
WidgetLocal.LowDrawDist = 0.0;
WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;

DistanceADD.GetMinMaxVal(TempMinLowDrawVal, TempMaxHighDrawVal);
ElevationADD.GetMinMaxVal(ElevTempMinLowDrawVal, ElevTempMaxHighDrawVal);
TempMinLowDrawVal = min(TempMinLowDrawVal, ElevTempMinLowDrawVal);
TempMaxHighDrawVal = max(TempMaxHighDrawVal, ElevTempMaxHighDrawVal);

if (TempMinLowDrawVal < WidgetLocal.MinLowDrawVal || TempMaxHighDrawVal > WidgetLocal.MaxHighDrawVal)
	{
	WidgetLocal.MinLowDrawVal = TempMinLowDrawVal;
	WidgetLocal.MaxHighDrawVal = TempMaxHighDrawVal;
	if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
		{
		ValRange = DistanceADD.GetIncrement();
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // if
	else
		{
		ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .1;
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // else
	} // if

MaxValueADT.SetValue(WidgetLocal.MaxHighDrawVal);
MaxValueADT.SetMetricType(DistanceADD.GetMetricType());
MaxValueADT.SetMultiplier(DistanceADD.GetMultiplier());
MaxValueADT.SetIncrement(DistanceADD.GetIncrement());

MinValueADT.SetValue(WidgetLocal.MinLowDrawVal);
MinValueADT.SetMetricType(DistanceADD.GetMetricType());
MinValueADT.SetMultiplier(DistanceADD.GetMultiplier());
MinValueADT.SetIncrement(DistanceADD.GetIncrement());

// figure distance increment
WidgetSNConfig(IDC_MAXDISPLAYVAL, &MaxValueADT);
WidgetSNConfig(IDC_MINDISPLAYVAL, &MinValueADT);

if (WidgetLocal.ActiveNode)
	{
	Distance = WidgetLocal.ActiveNode->GetDistance();
	Value = WidgetLocal.ActiveNode->GetValue();

	DistanceADT.SetValue(Distance);
	ValueADT.SetValue(Value);

	FormatAsPreferredUnit(Title, &DistanceADT, Distance);
	WidgetSetText(IDC_VALUEX, Title);
	WidgetSNConfig(IDC_VALUEY, &ValueADT);

	if (Elevation > -FLT_MAX)
		FormatAsPreferredUnit(Title, &ValueADT, Elevation);
	else
		strcpy(Title, "unknown");
	WidgetSetText(IDC_TERRAINEL, Title);

	WidgetSetDisabled(IDC_PREVNODE, ! WidgetLocal.ActiveNode->Prev);
	WidgetSetDisabled(IDC_NEXTNODE, ! WidgetLocal.ActiveNode->Next);

	SelectedNodes = DistanceADD.GetNumSelectedNodes();
	sprintf(Title, "%1d", SelectedNodes);
	WidgetSetText(IDC_NUMNODES, Title);

	} // if
else
	{
	Value = DistanceADD.GetCurValue(0);
	ValueADT.SetValue(Value);
	WidgetSNConfig(IDC_VALUEY, &ValueADT);
	WidgetSetText(IDC_VALUEX, "");
	WidgetSetText(IDC_TERRAINEL, "");

	WidgetSetDisabled(IDC_PREVNODE, TRUE);
	WidgetSetDisabled(IDC_NEXTNODE, TRUE);
	sprintf(Title, "%1d", 0);
	WidgetSetText(IDC_NUMNODES, Title);
	} // else
WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_GRAPH, &WidgetLocal);

//*****************

ConfigureFI(NativeWin, IDC_SLOPE,
 &Slope,
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_MINSLOPE,
 &MinSlope,
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_MAXSLOPE,
 &MaxSlope,
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);

WidgetSetText(IDC_VECNAME, Active->GetBestName());

} // VectorProfileGUI::ConfigureWidgets()

/*===========================================================================*/

void VectorProfileGUI::Name(void)
{

WidgetSetText(IDC_NAME, Active->Name());

} // VectorProfileGUI::Name()

/*===========================================================================*/

void VectorProfileGUI::SignalNewPoints(short Conform)
{
NotifyTag Changes[2];

Active->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
Active->RecheckBounds();
Active->ZeroUpTree();
HostDB->ReBoundTree(WCS_DATABASE_STATIC);
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, Conform);
Changes[1] = NULL;
HostDB->GenerateNotify(Changes);
Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes);

} // VectorProfileGUI::SignalNewPoints

/*===========================================================================*/

void VectorProfileGUI::SignalPreChangePoints(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
Changes[1] = NULL;
HostDB->GenerateNotify(Changes);

} // VectorProfileGUI::SignalPreChangePoints

/*===========================================================================*/

void VectorProfileGUI::Undo(void)
{

SignalPreChangePoints();
GlobalApp->MainProj->Interactive->UndoPoints(Active);
SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
if (GlobalApp->MainProj->Interactive->GetPointOperate() == WCS_VECTOR_PTOPERATE_ALLPTS)
	GlobalApp->MainProj->Interactive->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);

WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
FetchElevation();
FetchSlope();

} // VectorProfileGUI::Undo

/*===========================================================================*/

void VectorProfileGUI::Restore(void)
{

SignalPreChangePoints();
GlobalApp->MainProj->Interactive->RestorePoints(Active);
SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
if (GlobalApp->MainProj->Interactive->GetPointOperate() == WCS_VECTOR_PTOPERATE_ALLPTS)
	GlobalApp->MainProj->Interactive->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);

WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
FetchElevation();
FetchSlope();

} // VectorProfileGUI::Restore

/*===========================================================================*/

void VectorProfileGUI::NewActiveNode(GraphNode *NewActive)
{

WidgetLocal.ActiveNode = NewActive;
FetchElevation();
FetchSlope();
ConfigureWidgets();

} // VectorProfileGUI::NewActiveNode

/*===========================================================================*/

void VectorProfileGUI::NextNode(void)
{
GraphNode *TempNode;

if (WidgetLocal.ActiveNode)
	{
	TempNode = WidgetLocal.ActiveNode->Next;
	DistanceADD.ClearNodeSelectedAll();
	DistanceADD.SetNodeSelected(TempNode, 1);
	} // if
else
	WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);

FetchElevation();
FetchSlope();
ConfigureWidgets();

} // VectorProfileGUI::NextNode

/*===========================================================================*/

void VectorProfileGUI::PrevNode(void)
{
GraphNode *TempNode;

if (WidgetLocal.ActiveNode)
	{
	TempNode = WidgetLocal.ActiveNode->Prev;
	DistanceADD.ClearNodeSelectedAll();
	DistanceADD.SetNodeSelected(TempNode, 1);
	} // if
else
	WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);

FetchElevation();
FetchSlope();
ConfigureWidgets();

} // VectorProfileGUI::PrevNode

/*===========================================================================*/

void VectorProfileGUI::SelectAllNodes(void)
{

DistanceADD.SetNodeSelectedAll();

} // VectorProfileGUI::SelectAllNodes

/*===========================================================================*/

void VectorProfileGUI::ClearSelectNodes(void)
{

DistanceADD.ClearNodeSelectedAll();
if (WidgetLocal.ActiveNode)
	DistanceADD.SetNodeSelected(WidgetLocal.ActiveNode, 1);

} // VectorProfileGUI::ClearSelectNodes

/*===========================================================================*/

void VectorProfileGUI::ToggleSelectNodes(void)
{

DistanceADD.ToggleNodeSelectedAll();
WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
FetchElevation();
FetchSlope();

} // VectorProfileGUI::ToggleSelectNodes

/*===========================================================================*/

void VectorProfileGUI::FetchElevation(void)
{
GraphNode *ElevNode;

if (WidgetLocal.ActiveNode)
	{
	if (ElevNode = ElevationADD.FindNode(WidgetLocal.ActiveNode->GetDistance(), 0.001, 0.0, (double)FLT_MAX))
		Elevation = ElevNode->GetValue();
	else
		Elevation = -FLT_MAX;
	} // if
else
	Elevation = -FLT_MAX;

} // VectorProfileGUI::FetchElevation

/*===========================================================================*/

void VectorProfileGUI::FetchSlope(void)
{
double Dist;

Slope = 0.0;

if (WidgetLocal.ActiveNode)
	{
	if (WidgetLocal.ActiveNode->Prev)
		{
		if ((Dist = WidgetLocal.ActiveNode->GetDistance() - WidgetLocal.ActiveNode->Prev->GetDistance()) > 0.0)
			Slope = 100.0 * (WidgetLocal.ActiveNode->GetValue() - WidgetLocal.ActiveNode->Prev->GetValue()) / (Dist);
		} // if not first key
	else if (WidgetLocal.ActiveNode->Next)
		{
		if ((Dist = WidgetLocal.ActiveNode->Next->GetDistance() - WidgetLocal.ActiveNode->GetDistance()) > 0.0)
			Slope = 100.0 * (WidgetLocal.ActiveNode->GetValue() - WidgetLocal.ActiveNode->Next->GetValue()) / (Dist);
		} // else if not last key
	} // if

} // VectorProfileGUI::FetchSlope

/*===========================================================================*/

void VectorProfileGUI::NewValue(void)
{
GraphNode *CurNode;
VectorPoint *PLink;

if (WidgetLocal.ActiveNode)
	{
	DistanceADD.RemoteAlterSelectedNodeValue(Value, WidgetLocal.ActiveNode->GetValue());
	} // if
else
	{
	DistanceADD.SetCurValue(0, Value);
	} // else

// copy values to vector
if (Active && (PLink = Active->GetFirstRealPoint()) && (CurNode = DistanceADD.GetFirstNode(0)))
	{
	while (PLink && CurNode)
		{
		PLink->Elevation = (float)CurNode->Value;
		CurNode = CurNode->Next;
		PLink = PLink->Next;
		} // while
	} // if

} // VectorProfileGUI::NewValue

/*===========================================================================*/

void VectorProfileGUI::SetValueBySlope()
{
int OKtoChange = 0;
double Dist;

if (WidgetLocal.ActiveNode)
	{
	if (WidgetLocal.ActiveNode->Prev)
		{
		Dist = WidgetLocal.ActiveNode->GetDistance() - WidgetLocal.ActiveNode->Prev->GetDistance();
		Value = WidgetLocal.ActiveNode->Prev->GetValue() + Slope * Dist / 100.0;
		OKtoChange = 1;
		} // if not first key
	else if (WidgetLocal.ActiveNode->Next)
		{
		Dist = WidgetLocal.ActiveNode->Next->GetDistance() - WidgetLocal.ActiveNode->GetDistance();
		Value = WidgetLocal.ActiveNode->Next->GetValue() - Slope * Dist / 100.0;
		OKtoChange = 1;
		} // else if not last key
	} // if

if (OKtoChange)
	{
	ValueADT.SetValue(Value);
	NewValue();
	} // if

} // VectorProfileGUI::SetValueBySlope

/*===========================================================================*/

void VectorProfileGUI::SetValuesBySlopeLimits()
{
int OKtoChange;
double Dist, MinValue, MaxValue;
GraphNode *CurNode;

if (MaxSlope < MinSlope)
	{
	swmem(&MaxSlope, &MinSlope, sizeof (double));
	WidgetFISync(IDC_MAXSLOPE, WP_FISYNC_NONOTIFY);
	WidgetFISync(IDC_MINSLOPE, WP_FISYNC_NONOTIFY);
	} // if

if (CurNode = DistanceADD.GetFirstSelectedNodeNoSet(0))
	{
	while (CurNode)
		{
		OKtoChange = 0;
		if (CurNode->Prev)
			{
			Dist = CurNode->GetDistance() - CurNode->Prev->GetDistance();
			MaxValue = CurNode->Prev->GetValue() + MaxSlope * Dist / 100.0;
			MinValue = CurNode->Prev->GetValue() + MinSlope * Dist / 100.0;
			OKtoChange = 1;
			} // while
		else if (CurNode->Next)
			{
			Dist = CurNode->Next->GetDistance() - CurNode->GetDistance();
			MaxValue = CurNode->Next->GetValue() - MaxSlope * Dist / 100.0;
			MinValue = CurNode->Next->GetValue() - MinSlope * Dist / 100.0;
			OKtoChange = 1;
			} // else
		if (OKtoChange)
			{
			if (CurNode->GetValue() > MaxValue)
				CurNode->SetValue(MaxValue);
			else if (CurNode->GetValue() < MinValue)
				CurNode->SetValue(MinValue);
			} // if
		CurNode = DistanceADD.GetNextSelectedNode(CurNode);
		} // while
	UpdateVectorElevations();
	} // if

} // VectorProfileGUI::SetValuesBySlopeLimits

/*===========================================================================*/

int VectorProfileGUI::SetDistanceADD(void)
{
double SumDistance, TerrainElevation = 0.0, PlanetRad;
VertexDEM CurVert, NextVert;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VectorPoint *PLink, *SelNode;
GraphNode *CurNode = NULL, *ElevNode = NULL;
unsigned long int Marker = 0;
int ElevNULL = 0, ElevSkip = 0;

if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();

// remove existing nodes
DistanceADD.ReleaseNodes();
ElevationADD.ReleaseNodes();

if (Active && (PLink = Active->GetFirstRealPoint()))
	{
	SumDistance = 0.0;
	#ifdef WCS_JOE_LABELPOINTEXISTS
	// label point not legal for SelNode
	if ((SelNode = GlobalApp->MainProj->Interactive->GetFirstSelectedPt(Marker)) == Active->Points())
		SelNode = GlobalApp->MainProj->Interactive->GetNextSelectedPt(Marker);
	#endif // WCS_JOE_LABELPOINTEXISTS
	if (PLink->ProjToDefDeg(MyCoords, &CurVert))
		{
		if (! (CurNode = DistanceADD.AddNodeEnd(CurNode, SumDistance, CurVert.Elev, 0.0)))
			return (0);
		if (! ElevSkip)
			{
			TerrainElevation = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(CurVert.Lat, CurVert.Lon, ElevNULL);
			if (! ElevNULL)
				{
				if (! (ElevNode = ElevationADD.AddNodeEnd(ElevNode, SumDistance, TerrainElevation, 0.0)))
					return (0);
				ElevNode->SetLinear(1);
				} // if elevation OK
			else
				{
				ElevSkip = 10;
				} // else
			} // if
		else
			{
			ElevSkip --;
			} // else
		// set node selection state based on selection state of vector vertex
		if (SelNode == PLink)
			{
			CurNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
			SelNode = GlobalApp->MainProj->Interactive->GetNextSelectedPt(Marker);
			} // if
		PLink = PLink->Next;
		while (PLink)
			{
			if (PLink->ProjToDefDeg(MyCoords, &NextVert))
				{
				SumDistance += FindDistance(CurVert.Lat, CurVert.Lon, NextVert.Lat, NextVert.Lon, PlanetRad);
				if (! (CurNode = DistanceADD.AddNodeEnd(CurNode, SumDistance, NextVert.Elev, 0.0)))
					return (0);
				if (! ElevSkip)
					{
					TerrainElevation = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(NextVert.Lat, NextVert.Lon, ElevNULL);
					if (! ElevNULL)
						{
						if (! (ElevNode = ElevationADD.AddNodeEnd(ElevNode, SumDistance, TerrainElevation, 0.0)))
							return (0);
						ElevNode->SetLinear(1);
						} // if elevation OK
					else
						{
						ElevSkip = 10;
						} // else
					} // if
				else
					{
					ElevSkip --;
					} // else
				// set node selection state based on selection state of vector vertex
				if (SelNode == PLink)
					{
					CurNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
					SelNode = GlobalApp->MainProj->Interactive->GetNextSelectedPt(Marker);
					} // if
				CurVert.CopyLatLon(&NextVert);
				} // if
			else
				return (0);
			PLink = PLink->Next;
			} // while
		WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
		FetchElevation();
		FetchSlope();
		return (1);
		} // if
	} // if
WidgetLocal.ActiveNode = NULL;

return (0);

} // VectorProfileGUI::SetDistanceADD

/*===========================================================================*/

void VectorProfileGUI::UpdateNodeSelection(void)
{
VectorPoint *PLink, *SelNode;
GraphNode *CurNode;
unsigned long int Marker = 0;
long Selected;
char SelTxt[64];

if (Active && (PLink = Active->GetFirstRealPoint()))
	{
	CurNode = DistanceADD.GetFirstNode(0);
	#ifdef WCS_JOE_LABELPOINTEXISTS
	// label point not legal for SelNode
	if ((SelNode = GlobalApp->MainProj->Interactive->GetFirstSelectedPt(Marker)) == Active->Points())
		SelNode = GlobalApp->MainProj->Interactive->GetNextSelectedPt(Marker);
	#endif // WCS_JOE_LABELPOINTEXISTS
	while (PLink && CurNode)
		{
		// set node selection state based on selection state of vector vertex
		if (SelNode == PLink)
			{
			CurNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
			SelNode = GlobalApp->MainProj->Interactive->GetNextSelectedPt(Marker);
			} // if
		else
			CurNode->ClearFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
		PLink = PLink->Next;
		CurNode = DistanceADD.GetNextNode(0, CurNode);
		} // if
	} // if
WidgetLocal.ActiveNode = DistanceADD.GetFirstSelectedNodeNoSet(0);
FetchElevation();
FetchSlope();
Selected = DistanceADD.GetNumSelectedNodes();
sprintf(SelTxt, "%1d", Selected);
WidgetSetText(IDC_NUMNODES, SelTxt);

} // VectorProfileGUI::UpdateNodeSelection

/*===========================================================================*/

void VectorProfileGUI::UpdateVectorSelection(void)
{
VectorPoint *PLink;
GraphNode *CurNode;
long PtCt, LastPt = 0, Selected = 0;
char SelTxt[64];

if (Active && (PLink = Active->GetFirstRealPoint()))
	{
	// unselect all points
	GlobalApp->MainProj->Interactive->UnSelectAllPoints();
	PtCt = 1;
	CurNode = DistanceADD.GetFirstNode(0);
	while (PLink && CurNode)
		{
		// set node selection state based on selection state of vector vertex
		Selected = CurNode->GetSelectedState();
		// no notification or it feeds back
		GlobalApp->MainProj->Interactive->SetParam(0,
			MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ADDPOINTSELECT, 0),
			PtCt, Selected, 0);
		PLink = PLink->Next;
		CurNode = DistanceADD.GetNextNode(0, CurNode);
		LastPt = PtCt;
		PtCt ++;
		} // if
	// this one generates notification
	GlobalApp->MainProj->Interactive->SetParam(1,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ADDPOINTSELECT, 0),
		LastPt, Selected, 0);
	} // if
Selected = DistanceADD.GetNumSelectedNodes();
sprintf(SelTxt, "%1d", Selected);
WidgetSetText(IDC_NUMNODES, SelTxt);

} // VectorProfileGUI::UpdateVectorSelection

/*===========================================================================*/

void VectorProfileGUI::UpdateVectorElevations(void)
{
VectorPoint *PLink;
GraphNode *CurNode;
NotifyTag Changes[2];

if (Active && (PLink = Active->GetFirstRealPoint()))
	{
	CurNode = DistanceADD.GetFirstNode(0);
	while (PLink && CurNode)
		{
		PLink->Elevation = (float)CurNode->Value;
		PLink = PLink->Next;
		CurNode = DistanceADD.GetNextNode(0, CurNode);
		} // if
	// this generates notification
	if (WidgetLocal.ActiveNode)
		{
		ValueADT.SetValue(WidgetLocal.ActiveNode->GetValue());
		WidgetSNSync(IDC_VALUEY, WP_FISYNC_NONOTIFY);
		} // if
	RefreshGraph();
	IgnoreNotify = 1;
	HostDB->GenerateNotify(DBChangeEventPOINTS, Active);
	Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active);
	} // if

} // VectorProfileGUI::UpdateVectorElevations

/*===========================================================================*/

void VectorProfileGUI::RefreshGraph(void)
{

WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_GRAPH, &WidgetLocal);

} // VectorProfileGUI::RefreshGraph

/*===========================================================================*/

void VectorProfileGUI::VectorScale(void)
{

if(VecScaleGUI)
	{
	delete VecScaleGUI;
	}
VecScaleGUI = new VectorScaleGUI(HostDB, GlobalApp->MainProj, Active);
if(VecScaleGUI)
	{
	if (VecScaleGUI->ConstructError)
		{
		delete VecScaleGUI;
		VecScaleGUI = NULL;
		} // if
	else
		VecScaleGUI->Open(GlobalApp->MainProj);
	}

} // VectorProfileGUI::VectorScale

/*===========================================================================*/

void VectorProfileGUI::ExportGraph(void)
{

#ifdef WCS_BUILD_VNS
if (GlobalApp->GUIWins->VPX)
	{
	if (GlobalApp->GUIWins->VPX->GetActive() == Active)
		{
		GlobalApp->GUIWins->VPX->Open(GlobalApp->MainProj);
		return;
		} // if
	else
		{
		delete GlobalApp->GUIWins->VPX;
		GlobalApp->GUIWins->VPX = NULL;
		} // else
	} // if
GlobalApp->GUIWins->VPX = new VecProfExportGUI(GlobalApp->MainProj, Active);
if (GlobalApp->GUIWins->VPX)
	{
	if (GlobalApp->GUIWins->VPX->ConstructError)
		{
		delete GlobalApp->GUIWins->VPX;
		GlobalApp->GUIWins->VPX = NULL;
		} // if
	else
		GlobalApp->GUIWins->VPX->Open(GlobalApp->MainProj);
	}
#endif // WCS_BUILD_VNS

} // VectorProfileGUI::ExportGraph

/*===========================================================================*/
