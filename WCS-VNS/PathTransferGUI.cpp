// PathTransferGUI.cpp
// Code for PathTransferGUI
// Built from AtmosphereEditGUI.cpp on 7/14/01 by Gary R. Huber
// Copyright 2001 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "PathTransferGUI.h"
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

TransferData::TransferData()
{
double Defaults[7] = {10.0, 10.0, 10.0, 30.0, 0.0, 100.0, 0.0};
double RangeDefaults[7][3] = {FLT_MAX, .001, 1.0, FLT_MAX, .001, 1.0, FLT_MAX, .001, 1.0,
	FLT_MAX, 1.0, 1.0, FLT_MAX, -FLT_MAX, 1.0, FLT_MAX, .001, 1.0, FLT_MAX, 0.0, 1.0};

VP_KeyPlace = WCS_TRANSFER_KEYPLACE_EACHVERTEX;
VP_ConstUnit = WCS_TRANSFER_CONSTUNIT_INTERVAL;
ConsiderElev = 1;
PV_VertPlace = WCS_TRANSFER_KEYPLACE_EACHVERTEX;
ConformTopo = 0;
ConsiderTfx = 1;
InterpType = WCS_TRANSFER_INTERPTYPE_SPLINE;
InPathTime = 10.0; 
InVelocity = 10.0;
InFirstKey = 0.0;
InPathLength = 10.0;
InFrameRate = 30.0;
AlternateElev = 0.0;
PP_VelocityType = WCS_TRANSFER_VELOCITYTYPE_NOCHANGE;
VV_VertType = WCS_TRANSFER_VERTCOUNT_NOCHANGE;
VV_VertSpacing = WCS_TRANSFER_VERTSPACE_NOCHANGE;

// vector to path
OutPathTime.SetDefaults(NULL, 0, Defaults[0]);
OutPathTime.SetRangeDefaults(RangeDefaults[0]);
OutPathTime.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
OutPathTime.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutVelocity.SetDefaults(NULL, 0, Defaults[1]);
OutVelocity.SetRangeDefaults(RangeDefaults[1]);
OutVelocity.SetMetricType(WCS_ANIMDOUBLE_METRIC_VELOCITY);
OutVelocity.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutFrameInt.SetDefaults(NULL, 0, Defaults[2]);
OutFrameInt.SetRangeDefaults(RangeDefaults[2]);
OutFrameInt.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
OutFrameInt.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
// path to vector
OutFrameRate.SetDefaults(NULL, 0, Defaults[3]);
OutFrameRate.SetRangeDefaults(RangeDefaults[3]);
OutFrameRate.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ElevAdd.SetDefaults(NULL, 0, Defaults[4]);
ElevAdd.SetRangeDefaults(RangeDefaults[4]);
ElevAdd.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ElevAdd.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutVertInt.SetDefaults(NULL, 0, Defaults[5]);
OutVertInt.SetRangeDefaults(RangeDefaults[5]);
OutVertInt.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
OutVertInt.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
// path to path
OutFirstKey.SetDefaults(NULL, 0, Defaults[6]);
OutFirstKey.SetRangeDefaults(RangeDefaults[6]);
OutFirstKey.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
OutFirstKey.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

OutNumKeys = 10;
OutNumVerts = 10;
InNumVerts = 10;

} // TransferData::TransferData

/*===========================================================================*/
/*===========================================================================*/

long AnimPath::CountUniqueKeys(void)
{
double LastTime = -1.0;
RasterAnimHostProperties Prop;
long NumNodes = 0;

Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;

while (1)	//lint !e716
	{
	Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = LastTime;
	Prop.NewKeyNodeRange[0] = -DBL_MAX;
	Prop.NewKeyNodeRange[1] = DBL_MAX;
	Lat->GetRAHostProperties(&Prop);
	if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
		{
		NumNodes ++;
		LastTime = Prop.NewKeyNodeRange[1];
		} // if
	else
		break;
	} // while

return (NumNodes);

} // AnimPath::CountUniqueKeys

/*===========================================================================*/

int AnimPath::GetNextUniqueKey(double &LastTime)
{
long NumNodes = 0;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;

Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = LastTime;
Prop.NewKeyNodeRange[0] = -DBL_MAX;
Prop.NewKeyNodeRange[1] = DBL_MAX;
Lat->GetRAHostProperties(&Prop);
if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
	{
	LastTime = Prop.NewKeyNodeRange[1];
	return(1);
	} // if

return (0);

} // AnimPath::GetNextUniqueKey

/*===========================================================================*/

int AnimPath::GetKeyFrameRange(double &MinDist, double &MaxDist)
{
int Found = 0;
RasterAnimHostProperties Prop;

MinDist = DBL_MAX;
MaxDist = 0.0;

Lat->GetRAHostProperties(&Prop);
if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	Found = 1;
	if (Prop.KeyNodeRange[0] < MinDist)
		MinDist = Prop.KeyNodeRange[0];
	if (Prop.KeyNodeRange[1] > MaxDist)
		MaxDist = Prop.KeyNodeRange[1];
	} // if
Lon->GetRAHostProperties(&Prop);
if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	Found = 1;
	if (Prop.KeyNodeRange[0] < MinDist)
		MinDist = Prop.KeyNodeRange[0];
	if (Prop.KeyNodeRange[1] > MaxDist)
		MaxDist = Prop.KeyNodeRange[1];
	} // if
if (Elev)
	{
	Elev->GetRAHostProperties(&Prop);
	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		Found = 1;
		if (Prop.KeyNodeRange[0] < MinDist)
			MinDist = Prop.KeyNodeRange[0];
		if (Prop.KeyNodeRange[1] > MaxDist)
			MaxDist = Prop.KeyNodeRange[1];
		} // if
	} // if

if (! Found)
	{
	MinDist = MaxDist = 0.0;
	} // else

return (Found);

} // AnimPath::GetKeyFrameRange

/*===========================================================================*/

double AnimPath::MeasureSplinedPathLength(int ConsiderElev, double FrameRate, double PlanetRad)
{
double MinTime, MaxTime, TimeDiff, TestTime, IntervalTime, SegDist, SegVDist, Length = 0.0;
double Lat0 = 0.0, Lat1, Lon0 = 0.0, Lon1, Elev0 = 0.0, Elev1 = 0.0;
long NumIntervals, Ct;

if (GetKeyFrameRange(MinTime, MaxTime))
	{
	if (MaxTime > MinTime)
		{
		TimeDiff = MaxTime - MinTime;
		NumIntervals = (long)(TimeDiff * FrameRate);
		if (NumIntervals < 1)
			NumIntervals = 1;
		IntervalTime = TimeDiff / NumIntervals;
		for (Ct = 0, TestTime = MinTime; Ct < NumIntervals; Ct ++, TestTime += IntervalTime)
			{
			Lat1 = Lat->GetValue(0, TestTime);
			Lon1 = Lon->GetValue(0, TestTime);
			if (Elev)
				Elev1 = Elev->GetValue(0, TestTime);
			if (Ct > 0)
				{
				SegDist = FindDistance(Lat0, Lon0, Lat1, Lon1, PlanetRad);
				if (ConsiderElev)
					{
					SegVDist = Elev1 - Elev0;
					SegDist = sqrt(SegDist * SegDist + SegVDist * SegVDist);
					} // if
				Length += SegDist;
				} // if
			Lat0 = Lat1;
			Lon0 = Lon1;
			Elev0 = Elev1;
			} // for
		} // if
	} // if

return (Length);

} // AnimPath::MeasureSplinedPathLength

/*===========================================================================*/

int AnimPath::Smooth(int ConsiderElev, double FrameRate, double LatScaleMeters, long NumKeys)
{
double CurLat, CurLon, CurElev, CurTime, MinTime, MaxTime, TimeInterval;
AnimDoubleTime DummyElev, NewLat, NewLon, NewElev;
SmoothPath *Smoother;
long KeyCt, SampleSuccess = 1;

DummyElev.SetValue(0.0);
NewLat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
NewLon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
NewElev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

if (Smoother = new SmoothPath(Lon, Lat, ConsiderElev ? (Elev ? Elev: &DummyElev): NULL, 0.0, 0.0, FrameRate, TRUE, LatScaleMeters))
	{
	GetKeyFrameRange(MinTime, MaxTime);
	TimeInterval = MaxTime > MinTime && NumKeys > 1 ? (MaxTime - MinTime) / (NumKeys - 1): 1.0;
	KeyCt = 0;
	for (CurTime = MinTime; CurTime <= MaxTime && KeyCt < NumKeys; CurTime += TimeInterval, KeyCt ++)
		{
		if (! Smoother->GetSmoothPoint(CurLon, CurLat, CurElev, CurTime))
			{
			SampleSuccess = 0;
			break;
			} // if
		NewLat.AddNode(CurTime, CurLat, 0.0);
		NewLon.AddNode(CurTime, CurLon, 0.0);
		if (Elev)
			NewElev.AddNode(CurTime, CurElev, 0.0);
		} // for

	if (SampleSuccess)
		{
		Lat->Copy(Lat, &NewLat);
		Lon->Copy(Lon, &NewLon);
		if (Elev)
			Elev->Copy(Elev, &NewElev);
		} // if
	delete Smoother;
	return (1);
	} // if

return (0);

} // AnimPath::Smooth

/*===========================================================================*/
/*===========================================================================*/

NativeGUIWin PathTransferGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // PathTransferGUI::Open

/*===========================================================================*/

NativeGUIWin PathTransferGUI::Construct(void)
{
char *Categories[] = {"Vector", "Control Point", "Camera", "Camera Target", "Light", "Celestial Object", 
	"3D Object", "Wave Model", "Cloud"};
long CategoryIDs[] = {WCS_RAHOST_OBJTYPE_VECTOR, WCS_RAHOST_OBJTYPE_CONTROLPT, WCS_EFFECTSSUBCLASS_CAMERA,
	WCS_EFFECTSSUBCLASS_CAMERA, WCS_EFFECTSSUBCLASS_LIGHT, WCS_EFFECTSSUBCLASS_CELESTIAL, WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_WAVE, WCS_EFFECTSSUBCLASS_CLOUD};
long TabIndex, FoundIn = -1, FoundOut = -1;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_PATH_TRANSFER, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_PATH_SELECTMSG, 0, 0);
	CreateSubWinFromTemplate(IDD_PATH_VECTORTOPATH, 0, 1);
	CreateSubWinFromTemplate(IDD_PATH_PATHTOVECTOR, 0, 2);
	CreateSubWinFromTemplate(IDD_PATH_VECTORTOVECTOR, 0, 3);
	CreateSubWinFromTemplate(IDD_PATH_PATHTOPATH, 0, 4);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < 9; TabIndex ++)
			{
			WidgetCBInsert(IDC_FROMCLASSDROP, TabIndex, Categories[TabIndex]);
			WidgetCBSetItemData(IDC_FROMCLASSDROP, TabIndex, (void *)CategoryIDs[TabIndex]);
			WidgetCBInsert(IDC_TOCLASSDROP, TabIndex, Categories[TabIndex]);
			WidgetCBSetItemData(IDC_TOCLASSDROP, TabIndex, (void *)CategoryIDs[TabIndex]);
			if (InType == CategoryIDs[TabIndex] && FoundIn < 0)
				FoundIn = TabIndex;
			if (OutType == CategoryIDs[TabIndex] && FoundIn < 0)
				FoundOut = TabIndex;
			} // for
		WidgetCBSetCurSel(IDC_FROMCLASSDROP, FoundIn);
		WidgetCBSetCurSel(IDC_TOCLASSDROP, FoundOut);
		BuildList(0);
		BuildList(1);
		SelectPanel();
		if (TransferFrom)
			NewInputSelection();	// calls ConfigureWidgets
		else
			ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // PathTransferGUI::Construct

/*===========================================================================*/

PathTransferGUI::PathTransferGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, 
	RasterAnimHost *TransferFromSource, RasterAnimHost *TransferToSource)
: GUIFenetre('PTHT', this, "Path-Vector Transfer") // Yes, I know...
{
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
static NotifyTag AllChangeEvents[] = {MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								0};
RasterAnimHostProperties Prop;

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
TransferFrom = TransferFromSource;
TransferTo = TransferToSource;
InType = OutType = -1;
InCamTarget = OutCamTarget = 0;
MakeNew = 0;

if (EffectsHost && DBHost && ProjHost)
	{
	TransData.OutFrameRate.CurValue = TransData.InFrameRate = ProjSource->Interactive->GetFrameRate();
	TransData.OutFirstKey.SetIncrement(TransData.InFrameRate > 0.0 ? 1.0 / TransData.InFrameRate: 0.0);
	TransData.OutPathTime.SetIncrement(TransData.InFrameRate > 0.0 ? 1.0 / TransData.InFrameRate: 0.0);
	TransData.OutFrameInt.SetIncrement(TransData.InFrameRate > 0.0 ? 1.0 / TransData.InFrameRate: 0.0);
	if (TransferFrom)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		TransferFrom->GetRAHostProperties(&Prop);
		InType = Prop.TypeNumber;
		} // if
	if (TransferTo)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		TransferTo->GetRAHostProperties(&Prop);
		OutType = Prop.TypeNumber;
		} // if
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	GlobalApp->AppEx->RegisterClient(this, AllChangeEvents);
	} // if
else
	ConstructError = 1;

} // PathTransferGUI::PathTransferGUI

/*===========================================================================*/

PathTransferGUI::~PathTransferGUI()
{

GlobalApp->MainProj->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // PathTransferGUI::~PathTransferGUI()

/*===========================================================================*/

long PathTransferGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_PTH, 0);

return(0);

} // PathTransferGUI::HandleCloseWin

/*===========================================================================*/

long PathTransferGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_PTH, 0);
		break;
		} // 
	case IDC_TRANSFER:
		{
		Transfer();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // PathTransferGUI::HandleButtonClick

/*===========================================================================*/

long PathTransferGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FROMLIST:
		{
		NewInputSelection();
		break;
		}
	case IDC_TOLIST:
		{
		NewOutputSelection();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PathTransferGUI::HandleListSel

/*===========================================================================*/

long PathTransferGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FROMLIST:
		{
		if (TransferFrom)
			TransferFrom->EditRAHost();
		break;
		}
	case IDC_TOLIST:
		{
		if (TransferTo)
			TransferTo->EditRAHost();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PathTransferGUI::HandleListDoubleClick

/*===========================================================================*/

long PathTransferGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FROMCLASSDROP:
		{
		TransferFrom = NULL;
		BuildList(0);
		SelectPanel();
		DisableWidgets();
		break;
		}
	case IDC_TOCLASSDROP:
		{
		TransferTo = NULL;
		BuildList(1);
		SelectPanel();
		DisableWidgets();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PathTransferGUI::HandleCBChange

/*===========================================================================*/

long PathTransferGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKVP_CONSIDERELEV:
	case IDC_CHECKPP_CONSIDERELEV:
	case IDC_CHECKVV_CONSIDERELEV:
	case IDC_CHECKVV_CONSIDERELEV2:
		{
		// this recalculates path length and reconfigures widgets
		NewInputSelection();
		break;
		} // 
	case IDC_CHECKPV_CONSIDERELEV:
	case IDC_CHECKPV_CONFORM:
	case IDC_CHECKPV_CONIDERTFX:
		{
		SyncWidgets();
		DisableWidgets();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PathTransferGUI::HandleSCChange

/*===========================================================================*/

long PathTransferGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RADIOVP_KEYEACHVERT:
	case IDC_RADIOVP_CONSTINTERVAL:
	case IDC_RADIOVP_CONSTVELOCITY:
	case IDC_RADIOVP_FIXEDFRAMES:
	case IDC_RADIOVP_SPLINE1:
	case IDC_RADIOVP_LINEAR1:
		{
		DisableWidgets();
		break;
		} // vector to path
	case IDC_RADIOPV_VERTEACHKEY:
	case IDC_RADIOPV_FIXEDINTERVAL:
	case IDC_RADIOPV_FIXEDCOUNT:
		{
		DisableWidgets();
		break;
		} // path to vector
	case IDC_RADIOPP_PRESERVEVELOCITY:
	case IDC_RADIOPP_FIXEDVELOCITY:
		{
		DisableWidgets();
		break;
		} // path to vector
	case IDC_RADIOVV_PRESERVENUMVERTS:
	case IDC_RADIOVV_CHANGENUMVERTS:
	case IDC_RADIOVV_PRESERVESPACING:
	case IDC_RADIOVV_FIXEDSPACING:
		{
		DisableWidgets();
		break;
		} // vector to vector
	case IDC_RADIOVV_SPLINE1:
	case IDC_RADIOVV_LINEAR1:
	case IDC_RADIOVV_SPLINE2:
	case IDC_RADIOVV_LINEAR2:
		{
		SyncWidgets();
		DisableWidgets();
		break;
		} // vector to vector
	default:
		break;
	} // switch CtrlID

return(0);

} // PathTransferGUI::HandleSRChange

/*===========================================================================*/

long PathTransferGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_VP_NUMKEYS:
	case IDC_PV_ELEVADD:
	case IDC_VP_ELEVADD:
	case IDC_PP_OUTPATHFIRSTKEY:
		{
		break;
		} // path to vector
	case IDC_PV_VERTINTERVAL:
		{
		ComputeNumVerticesFromVertexSpacing();
		break;
		} // path to vector
	case IDC_PV_NUMVERTS:
	case IDC_VV_OUTVERTICES:
		{
		ComputeIntervalFromNumVertices();
		ComputeVertexSpacingFromNumVertices();
		SyncWidgets();
		break;
		} // path to vector
	case IDC_VP_VELOCITY:
	case IDC_PP_OUTPATHVELOCITY:
		{
		ComputePathTimeFromVelocity();
		SyncWidgets();
		break;
		} // path to vector
	case IDC_VP_FRAMEINT:
		{
		ComputePathTimeFromInterval();
		SyncWidgets();
		break;
		} // path to vector
	case IDC_PV_FRAMEINT:
		{
		ComputeNumVerticesFromInterval();
		SyncWidgets();
		break;
		} // path to vector
	case IDC_VP_PATHLEN:
	case IDC_PP_OUTPATHLEN:
		{
		ComputeIntervalFromPathTime();
		ComputeVelocityFromPathTime();
		break;
		} // path to vector
	case IDC_PV_FRAMERATE:
		{
		TransData.OutFrameInt.SetIncrement(TransData.OutFrameRate.CurValue > 0.0 ? 1.0 / TransData.OutFrameRate.CurValue: 0.0);
		SyncWidgets();
		break;
		} // path to vector
	default:
		break;
	} // ID

return(0);

} // PathTransferGUI::HandleFIChange

/*===========================================================================*/

void PathTransferGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	} // if

Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildList(0);
	BuildList(1);
	DisableWidgets();
	} // if

} // PathTransferGUI::HandleNotifyEvent()

/*===========================================================================*/

void PathTransferGUI::ConfigureWidgets(void)
{
char FormatStr[128];

ComputeIntervalFromPathTime();
ComputeVelocityFromPathTime();
ComputeVertexSpacingFromNumVertices();

// Vector to path
ConfigureSC(NativeWin, IDC_CHECKVP_CONSIDERELEV, &TransData.ConsiderElev, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOVP_KEYEACHVERT, IDC_RADIOVP_KEYEACHVERT, &TransData.VP_KeyPlace, SRFlag_Char, WCS_TRANSFER_KEYPLACE_EACHVERTEX, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVP_KEYEACHVERT, IDC_RADIOVP_FIXEDFRAMES, &TransData.VP_KeyPlace, SRFlag_Char, WCS_TRANSFER_KEYPLACE_FIXEDCOUNT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOVP_CONSTINTERVAL, IDC_RADIOVP_CONSTINTERVAL, &TransData.VP_ConstUnit, SRFlag_Char, WCS_TRANSFER_CONSTUNIT_INTERVAL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVP_CONSTINTERVAL, IDC_RADIOVP_CONSTVELOCITY, &TransData.VP_ConstUnit, SRFlag_Char, WCS_TRANSFER_CONSTUNIT_VELOCITY, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOVP_SPLINE1, IDC_RADIOVP_SPLINE1, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_SPLINE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVP_SPLINE1, IDC_RADIOVP_LINEAR1, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_LINEAR, NULL, NULL);

WidgetSNConfig(IDC_VP_FRAMEINT, &TransData.OutFrameInt);
WidgetSNConfig(IDC_VP_PATHLEN, &TransData.OutPathTime);
WidgetSNConfig(IDC_VP_VELOCITY, &TransData.OutVelocity);
WidgetSNConfig(IDC_VP_ELEVADD, &TransData.ElevAdd);

ConfigureFI(NativeWin, IDC_VP_NUMKEYS,
 &TransData.OutNumKeys,
  1.0,
   1.0,
    (double)LONG_MAX,
     FIOFlag_Long,
      NULL,
       NULL);

// Path to vector
ConfigureSC(NativeWin, IDC_CHECKPV_CONSIDERELEV, &TransData.ConsiderElev, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPV_CONFORM, &TransData.ConformTopo, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPV_CONIDERTFX, &TransData.ConsiderTfx, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOPV_VERTEACHKEY, IDC_RADIOPV_VERTEACHKEY, &TransData.PV_VertPlace, SRFlag_Char, WCS_TRANSFER_KEYPLACE_EACHVERTEX, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPV_VERTEACHKEY, IDC_RADIOPV_FIXEDINTERVAL, &TransData.PV_VertPlace, SRFlag_Char, WCS_TRANSFER_KEYPLACE_EACHINTERVAL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPV_VERTEACHKEY, IDC_RADIOPV_FIXEDCOUNT, &TransData.PV_VertPlace, SRFlag_Char, WCS_TRANSFER_KEYPLACE_FIXEDCOUNT, NULL, NULL);

WidgetSNConfig(IDC_PV_ELEVADD, &TransData.ElevAdd);
WidgetSNConfig(IDC_PV_FRAMEINT, &TransData.OutFrameInt);
WidgetSNConfig(IDC_PV_FRAMERATE, &TransData.OutFrameRate);
WidgetSNConfig(IDC_PV_VERTINTERVAL, &TransData.OutVertInt);

ConfigureFI(NativeWin, IDC_PV_NUMVERTS,
 &TransData.OutNumVerts,
  1.0,
   1.0,
    32767.0,
     FIOFlag_Long,
      NULL,
       NULL);

// path to path
ConfigureSC(NativeWin, IDC_CHECKPP_CONSIDERELEV, &TransData.ConsiderElev, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOPP_PRESERVEVELOCITY, IDC_RADIOPP_PRESERVEVELOCITY, &TransData.PP_VelocityType, SRFlag_Char, WCS_TRANSFER_VELOCITYTYPE_NOCHANGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPP_PRESERVEVELOCITY, IDC_RADIOPP_FIXEDVELOCITY, &TransData.PP_VelocityType, SRFlag_Char, WCS_TRANSFER_VELOCITYTYPE_CONSTANT, NULL, NULL);

WidgetSNConfig(IDC_PP_OUTPATHVELOCITY, &TransData.OutVelocity);
WidgetSNConfig(IDC_PP_OUTPATHFIRSTKEY, &TransData.OutFirstKey);
WidgetSNConfig(IDC_PP_OUTPATHLEN, &TransData.OutPathTime);

FormatAsPreferredUnit(FormatStr, &TransData.OutVelocity, TransData.InVelocity);
WidgetSetText(IDC_PP_INPATHVELOCITY, FormatStr);
FormatAsPreferredUnit(FormatStr, &TransData.OutFirstKey, TransData.InFirstKey);
WidgetSetText(IDC_PP_INPATHFIRSTKEY, FormatStr);
FormatAsPreferredUnit(FormatStr, &TransData.OutPathTime, TransData.InPathTime);
WidgetSetText(IDC_PP_INPATHLEN, FormatStr);

// vector to vector
ConfigureSC(NativeWin, IDC_CHECKVV_CONSIDERELEV, &TransData.ConsiderElev, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVV_CONSIDERELEV2, &TransData.ConsiderElev, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOVV_PRESERVENUMVERTS, IDC_RADIOVV_PRESERVENUMVERTS, &TransData.VV_VertType, SRFlag_Char, WCS_TRANSFER_VERTCOUNT_NOCHANGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVV_PRESERVENUMVERTS, IDC_RADIOVV_CHANGENUMVERTS, &TransData.VV_VertType, SRFlag_Char, WCS_TRANSFER_VERTCOUNT_CHANGE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOVV_PRESERVESPACING, IDC_RADIOVV_PRESERVESPACING, &TransData.VV_VertSpacing, SRFlag_Char, WCS_TRANSFER_VERTSPACE_NOCHANGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVV_PRESERVESPACING, IDC_RADIOVV_FIXEDSPACING, &TransData.VV_VertSpacing, SRFlag_Char, WCS_TRANSFER_VERTSPACE_EVEN, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOVV_SPLINE1, IDC_RADIOVV_SPLINE1, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_SPLINE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVV_SPLINE1, IDC_RADIOVV_LINEAR1, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_LINEAR, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOVV_SPLINE2, IDC_RADIOVV_SPLINE2, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_SPLINE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOVV_SPLINE2, IDC_RADIOVV_LINEAR2, &TransData.InterpType, SRFlag_Char, WCS_TRANSFER_INTERPTYPE_LINEAR, NULL, NULL);

sprintf(FormatStr, "%d", TransData.InNumVerts);
WidgetSetText(IDC_VV_INNUMVERTS, FormatStr);

ConfigureFI(NativeWin, IDC_VV_OUTVERTICES,
 &TransData.OutNumVerts,
  1.0,
   1.0,
    32767.0,
     FIOFlag_Long,
      NULL,
       NULL);

DisableWidgets();

} // PathTransferGUI::ConfigureWidgets()

/*===========================================================================*/

void PathTransferGUI::SyncWidgets(void)
{
char FormatStr[128];

WidgetFISync(IDC_VP_NUMKEYS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PV_NUMVERTS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_VV_OUTVERTICES, WP_FISYNC_NONOTIFY);

WidgetSNSync(IDC_VP_FRAMEINT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VP_PATHLEN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VP_VELOCITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VP_ELEVADD, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PV_ELEVADD, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PV_FRAMEINT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PV_FRAMERATE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PV_VERTINTERVAL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHVELOCITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHFIRSTKEY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHLEN, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKVP_CONSIDERELEV, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPV_CONSIDERELEV, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPV_CONFORM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPV_CONIDERTFX, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPP_CONSIDERELEV, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVV_CONSIDERELEV, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVV_CONSIDERELEV2, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOVP_CONSTINTERVAL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVP_CONSTVELOCITY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVP_SPLINE1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVP_LINEAR1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVP_KEYEACHVERT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVP_FIXEDFRAMES, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPV_VERTEACHKEY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPV_FIXEDINTERVAL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPV_FIXEDCOUNT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPP_PRESERVEVELOCITY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPP_FIXEDVELOCITY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_PRESERVENUMVERTS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_CHANGENUMVERTS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_PRESERVESPACING, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_FIXEDSPACING, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_SPLINE1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_LINEAR1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_SPLINE2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOVV_LINEAR2, WP_SRSYNC_NONOTIFY);

FormatAsPreferredUnit(FormatStr, &TransData.OutVelocity, TransData.InVelocity);
WidgetSetText(IDC_PP_INPATHVELOCITY, FormatStr);
FormatAsPreferredUnit(FormatStr, &TransData.OutFirstKey, TransData.InFirstKey);
WidgetSetText(IDC_PP_INPATHFIRSTKEY, FormatStr);
FormatAsPreferredUnit(FormatStr, &TransData.OutPathTime, TransData.InPathTime);
WidgetSetText(IDC_PP_INPATHLEN, FormatStr);

} // PathTransferGUI::SyncWidgets

/*===========================================================================*/

void PathTransferGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOVP_CONSTINTERVAL, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_EACHVERTEX);
WidgetSetDisabled(IDC_RADIOVP_CONSTVELOCITY, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_EACHVERTEX);
WidgetSetDisabled(IDC_VP_FRAMEINT, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_EACHVERTEX || TransData.VP_ConstUnit != WCS_TRANSFER_CONSTUNIT_INTERVAL);
WidgetSetDisabled(IDC_VP_VELOCITY, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_EACHVERTEX || TransData.VP_ConstUnit != WCS_TRANSFER_CONSTUNIT_VELOCITY);

WidgetSetDisabled(IDC_VP_NUMKEYS, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_FIXEDCOUNT);
WidgetSetDisabled(IDC_RADIOVP_SPLINE1, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_FIXEDCOUNT);
WidgetSetDisabled(IDC_RADIOVP_LINEAR1, TransData.VP_KeyPlace != WCS_TRANSFER_KEYPLACE_FIXEDCOUNT);

WidgetSetDisabled(IDC_CHECKPV_CONIDERTFX, ! TransData.ConformTopo);

WidgetSetDisabled(IDC_PV_FRAMEINT, TransData.PV_VertPlace != WCS_TRANSFER_KEYPLACE_EACHINTERVAL);
WidgetSetDisabled(IDC_PV_FRAMERATE, TransData.PV_VertPlace != WCS_TRANSFER_KEYPLACE_EACHINTERVAL);

WidgetSetDisabled(IDC_PV_NUMVERTS, TransData.PV_VertPlace != WCS_TRANSFER_KEYPLACE_FIXEDCOUNT);
WidgetSetDisabled(IDC_PV_VERTINTERVAL, TransData.PV_VertPlace != WCS_TRANSFER_KEYPLACE_FIXEDCOUNT);

WidgetSetDisabled(IDC_PP_INPATHVELOCITY, TransData.PP_VelocityType != WCS_TRANSFER_VELOCITYTYPE_CONSTANT);
WidgetSetDisabled(IDC_PP_OUTPATHVELOCITY, TransData.PP_VelocityType != WCS_TRANSFER_VELOCITYTYPE_CONSTANT);
WidgetSetDisabled(IDC_AVGSOURCETXT, TransData.PP_VelocityType != WCS_TRANSFER_VELOCITYTYPE_CONSTANT);
WidgetSetDisabled(IDC_CHECKPP_CONSIDERELEV, TransData.PP_VelocityType != WCS_TRANSFER_VELOCITYTYPE_CONSTANT);

WidgetSetDisabled(IDC_RADIOVV_PRESERVESPACING, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_NOCHANGE);
WidgetSetDisabled(IDC_RADIOVV_FIXEDSPACING, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_NOCHANGE);
WidgetSetDisabled(IDC_RADIOVV_SPLINE1, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_NOCHANGE || TransData.VV_VertSpacing != WCS_TRANSFER_VERTSPACE_EVEN);
WidgetSetDisabled(IDC_RADIOVV_LINEAR1, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_NOCHANGE || TransData.VV_VertSpacing != WCS_TRANSFER_VERTSPACE_EVEN);
WidgetSetDisabled(IDC_CHECKVV_CONSIDERELEV, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_NOCHANGE || TransData.VV_VertSpacing != WCS_TRANSFER_VERTSPACE_EVEN);

WidgetSetDisabled(IDC_CHECKVV_CONSIDERELEV2, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);
WidgetSetDisabled(IDC_VV_INNUMVERTS, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);
WidgetSetDisabled(IDC_INVERTTXT, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);
WidgetSetDisabled(IDC_VV_OUTVERTICES, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);
WidgetSetDisabled(IDC_RADIOVV_SPLINE2, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);
WidgetSetDisabled(IDC_RADIOVV_LINEAR2, TransData.VV_VertType != WCS_TRANSFER_VERTCOUNT_CHANGE);

WidgetSetDisabled(IDC_TRANSFER, ! (TransferFrom && (TransferTo || MakeNew)));

} // PathTransferGUI::DisableWidgets

/*===========================================================================*/

void PathTransferGUI::BuildList(int InputOrOutput)
{
short ListID, ComboID;
long Current, TypeNumber, Pos, IsControl, Found = -1;
GeneralEffect *CurEffect;
Joe *CurJoe;
char CategoryName[64], NewItemText[256];
RasterAnimHost *FindMe, **SetMeNULL;

ListID = InputOrOutput ? IDC_TOLIST: IDC_FROMLIST;
ComboID = InputOrOutput ? IDC_TOCLASSDROP: IDC_FROMCLASSDROP;
FindMe = InputOrOutput ? TransferTo: TransferFrom;
SetMeNULL = InputOrOutput ? &TransferTo: &TransferFrom;

WidgetLBClear(ListID);

if ((Current = WidgetCBGetCurSel(ComboID)) != CB_ERR)
	{
	// if output side, insert a New Item line
	WidgetCBGetText(ComboID, Current, CategoryName);
	if (InputOrOutput)
		{
		if (! strnicmp(CategoryName, "Camera Target", 13))
			OutCamTarget = 1;
		else
			OutCamTarget = 0;
		if (! strnicmp(CategoryName, "Camera ", 7))
			CategoryName[6] = 0;
		sprintf(NewItemText, "* New %s", CategoryName);
		WidgetLBInsert(ListID, -1, NewItemText);
		} // if
	else if (! strnicmp(CategoryName, "Camera Target", 13))
		InCamTarget = 1;
	else
		InCamTarget = 0;
	TypeNumber = (long)WidgetCBGetItemData(ComboID, Current);
	if (InputOrOutput)
		OutType = TypeNumber;
	else
		InType = TypeNumber;
	if (TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR || TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		DBHost->ResetGeoClip();
		CurJoe = DBHost->GetFirst();
		while (CurJoe)
			{
			if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && ! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				IsControl = CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL);
				if ((IsControl && TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT) ||
					(! IsControl && TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR))
					{
					if (CurJoe->TestFlags(WCS_JOEFLAG_ACTIVATED))
						strcpy(NewItemText, "* ");
					else
						strcpy(NewItemText, "  ");
					strcat(NewItemText, CurJoe->GetBestName());
					Pos = WidgetLBInsert(ListID, -1, NewItemText);
					WidgetLBSetItemData(ListID, Pos, CurJoe);
					if (CurJoe == (Joe *)FindMe)
						Found = Pos;
					} // if
				} // if
			CurJoe = DBHost->GetNext(CurJoe);
			} // while
		} // if
	else
		{
		CurEffect = EffectsHost->GetListPtr(TypeNumber);
		while (CurEffect)
			{
			if (CurEffect->Enabled)
				strcpy(NewItemText, "* ");
			else
				strcpy(NewItemText, "  ");
			strcat(NewItemText, CurEffect->Name);
			Pos = WidgetLBInsert(ListID, -1, NewItemText);
			WidgetLBSetItemData(ListID, Pos, CurEffect);
			if (CurEffect == (GeneralEffect *)FindMe)
				Found = Pos;
			CurEffect = CurEffect->Next;
			} //
		} // else
	} // if

if (Found < 0)
	*SetMeNULL = NULL;
else
	WidgetLBSetCurSel(ListID, Found);

} // PathTransferGUI::BuildList

/*===========================================================================*/

void PathTransferGUI::SelectPanel(void)
{
long Current, PanelID = 0, InType = -1, OutType = -1;

if ((Current = WidgetCBGetCurSel(IDC_TOCLASSDROP)) != CB_ERR)
	{
	OutType = (long)WidgetCBGetItemData(IDC_TOCLASSDROP, Current);
	} // if
if ((Current = WidgetCBGetCurSel(IDC_FROMCLASSDROP)) != CB_ERR)
	{
	InType = (long)WidgetCBGetItemData(IDC_FROMCLASSDROP, Current);
	} // if
if (InType >= 0 && OutType >= 0)
	{
	if (InType == WCS_RAHOST_OBJTYPE_VECTOR || InType == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		if (OutType == WCS_RAHOST_OBJTYPE_VECTOR || OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			PanelID = 3;	// IDD_PATH_VECTORTOVECTOR
			} // if
		else
			{
			PanelID = 1;	// IDD_PATH_VECTORTOPATH
			} // else
		} // 
	else
		{
		if (OutType == WCS_RAHOST_OBJTYPE_VECTOR || OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			PanelID = 2;	// IDD_PATH_PATHTOVECTOR
			} // if
		else
			{
			PanelID = 4;	// IDD_PATH_PATHTOPATH
			} // else
		} // effect
	} // if 

switch (PanelID)
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
	case 2:
		{
		ShowPanel(0, 2);
		break;
		} // 2
	case 3:
		{
		ShowPanel(0, 3);
		break;
		} // 3
	case 4:
		{
		ShowPanel(0, 4);
		break;
		} // 3
	} // PanelID

} // PathTransferGUI::SelectPanel

/*===========================================================================*/

void PathTransferGUI::NewInputSelection(void)
{
long Current;
double KeyRange[2];
RasterAnimHost *CurHost;

if ((Current = WidgetLBGetCurSel(IDC_FROMLIST)) != LB_ERR)
	{
	if ((CurHost = (RasterAnimHost *)WidgetLBGetItemData(IDC_FROMLIST, Current)) && CurHost != (RasterAnimHost *)LB_ERR)
		{
		if (TransferTo == CurHost && (InType == WCS_EFFECTSSUBCLASS_CAMERA && ! InCamTarget && OutCamTarget) 
			|| (OutType == WCS_EFFECTSSUBCLASS_CAMERA && ! OutCamTarget && InCamTarget))
			{
			UserMessageOK("Illegal Selection", "This would create coincident Camera and Camera Target positions. This selection is not allowed.");
			WidgetLBSetCurSel(IDC_FROMLIST, -1);
			return;
			} // if
		if (InType == WCS_RAHOST_OBJTYPE_VECTOR || InType == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			TransData.InNumVerts = (long)WCS_max((double)((Joe *)CurHost)->GetNumRealPoints(), 0.0);
			TransData.OutNumVerts = TransData.OutNumKeys = TransData.InNumVerts;
			TransData.InPathLength = ((Joe *)CurHost)->GetVecLength(TransData.ConsiderElev);
			TransData.InPathTime = TransData.InNumVerts > 1 ? 10.0: 0.0;
			TransData.OutPathTime.CurValue = TransData.InPathTime;
			TransData.InFirstKey = 0.0;
			TransData.OutFirstKey.CurValue = TransData.InFirstKey;
			TransData.InVelocity = TransData.InPathTime > 0.0 ? TransData.InPathLength / TransData.InPathTime: 0.0;
			TransData.OutVelocity.CurValue = TransData.InVelocity;
			} // if
		else
			{
			switch (InType)
				{
				case WCS_EFFECTSSUBCLASS_CAMERA:
					{
					if (! InCamTarget)
						{
						TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
						TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
						TransData.InPath.Elev = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
						} // if
					else
						{
						TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
						TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
						TransData.InPath.Elev = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
						} // else
					break;
					} // WCS_EFFECTSSUBCLASS_CAMERA
				case WCS_EFFECTSSUBCLASS_LIGHT:
					{
					TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LAT);
					TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LON);
					TransData.InPath.Elev = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_ELEV);
					break;
					} // WCS_EFFECTSSUBCLASS_LIGHT
				case WCS_EFFECTSSUBCLASS_CELESTIAL:
					{
					TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE);
					TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE);
					TransData.InPath.Elev = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE);
					break;
					} // WCS_EFFECTSSUBCLASS_CELESTIAL
				case WCS_EFFECTSSUBCLASS_OBJECT3D:
					{
					TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT);
					TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LON);
					TransData.InPath.Elev = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV);
					break;
					} // WCS_EFFECTSSUBCLASS_OBJECT3D
				case WCS_EFFECTSSUBCLASS_WAVE:
					{
					TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE);
					TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE);
					TransData.InPath.Elev = NULL;
					break;
					} // WCS_EFFECTSSUBCLASS_WAVE
				case WCS_EFFECTSSUBCLASS_CLOUD:
					{
					TransData.InPath.Lat = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT);
					TransData.InPath.Lon = ((GeneralEffect *)CurHost)->GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON);
					TransData.InPath.Elev = NULL;
					break;
					} // WCS_EFFECTSSUBCLASS_CLOUD
				} // switch
			TransData.InPath.GetKeyFrameRange(KeyRange[0], KeyRange[1]);
			TransData.InPathTime = KeyRange[1] > KeyRange[0] ? KeyRange[1] - KeyRange[0]: 0.0;
			TransData.OutPathTime.CurValue = TransData.InPathTime;
			TransData.InPathLength = KeyRange[1] > KeyRange[0] ? 
				TransData.InPath.MeasureSplinedPathLength(TransData.ConsiderElev, TransData.InFrameRate, EffectsHost->GetPlanetRadius()): 0.0;
			TransData.InFirstKey = KeyRange[0] > 0.0 && KeyRange[0] < FLT_MAX ? KeyRange[0]: 0.0;
			TransData.OutFirstKey.CurValue = TransData.InFirstKey;
			TransData.InVelocity = TransData.InPathTime > 0.0 ? TransData.InPathLength / TransData.InPathTime: 0.0;
			TransData.OutVelocity.CurValue = TransData.InVelocity;
			TransData.OutNumKeys = TransData.InNumVerts = TransData.InPath.CountUniqueKeys();
			if (TransData.InNumVerts <= 0)
				TransData.InNumVerts = 1;
			TransData.OutNumVerts = TransData.InNumVerts;
			TransData.OutVertInt.CurValue = TransData.OutNumVerts > 1 ?
				TransData.InPathLength / (TransData.OutNumVerts - 1): 0.0;
			} // else
		TransferFrom = CurHost;
		} // if
	} // if

ConfigureWidgets();

} // PathTransferGUI::NewInputSelection

/*===========================================================================*/

void PathTransferGUI::NewOutputSelection(void)
{
long Current;
RasterAnimHost *CurHost;

if ((Current = WidgetLBGetCurSel(IDC_TOLIST)) != LB_ERR)
	{
	if ((CurHost = (RasterAnimHost *)WidgetLBGetItemData(IDC_TOLIST, Current)) && CurHost != (RasterAnimHost *)LB_ERR)
		{
		if (CurHost == TransferFrom && (InType == WCS_EFFECTSSUBCLASS_CAMERA && ! InCamTarget && OutCamTarget) 
			|| (OutType == WCS_EFFECTSSUBCLASS_CAMERA && ! OutCamTarget && InCamTarget))
			{
			UserMessageOK("Illegal Selection", "This would create coincident Camera and Camera Target positions. This selection is not allowed.");
			WidgetLBSetCurSel(IDC_TOLIST, -1);
			return;
			} // if
		TransferTo = CurHost;
		MakeNew = 0;
		} // if
	else if (! CurHost)
		{
		TransferTo = NULL;
		MakeNew = 1;
		}
	} // if

DisableWidgets();

} // PathTransferGUI::NewOutputSelection

/*===========================================================================*/

void PathTransferGUI::ComputeIntervalFromPathTime(void)
{

if (TransData.InNumVerts > 1)
	{
	TransData.OutFrameInt.CurValue = TransData.OutPathTime.CurValue / (TransData.InNumVerts - 1);
	} // if
else
	TransData.OutFrameInt.CurValue = 0.0;
WidgetSNSync(IDC_VP_FRAMEINT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PV_FRAMEINT, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeIntervalFromPathTime

/*===========================================================================*/

void PathTransferGUI::ComputePathTimeFromInterval(void)
{

TransData.OutPathTime.CurValue = TransData.OutFrameInt.CurValue * (TransData.InNumVerts - 1);
WidgetSNSync(IDC_VP_PATHLEN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHLEN, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputePathTimeFromInterval

/*===========================================================================*/

void PathTransferGUI::ComputeVelocityFromPathTime(void)
{

if (TransData.OutPathTime.CurValue > 0.0)
	{
	TransData.OutVelocity.CurValue = TransData.InPathLength / TransData.OutPathTime.CurValue;
	} // if
else
	TransData.OutVelocity.CurValue = 0.0;
WidgetSNSync(IDC_VP_VELOCITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHVELOCITY, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeVelocityFromPathTime

/*===========================================================================*/

void PathTransferGUI::ComputePathTimeFromVelocity(void)
{

if (TransData.OutVelocity.CurValue > 0.0)
	{
	TransData.OutPathTime.CurValue = TransData.InPathLength / TransData.OutVelocity.CurValue;
	} // if
else
	TransData.OutPathTime.CurValue = 0.0;
WidgetSNSync(IDC_VP_PATHLEN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PP_OUTPATHLEN, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputePathTimeFromVelocity

/*===========================================================================*/

void PathTransferGUI::ComputeNumVerticesFromVertexSpacing(void)
{

if (TransData.OutVertInt.CurValue > 0.0)
	{
	TransData.OutNumVerts = 1 + (long)(TransData.InPathLength / TransData.OutVertInt.CurValue);
	} // if
else
	TransData.OutNumVerts = 1;
WidgetFISync(IDC_PV_NUMVERTS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_VV_OUTVERTICES, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeNumVerticesFromVertexSpacing

/*===========================================================================*/

void PathTransferGUI::ComputeVertexSpacingFromNumVertices(void)
{

if (TransData.OutNumVerts > 1)
	{
	TransData.OutVertInt.CurValue = TransData.InPathLength / (TransData.OutNumVerts - 1);
	} // if
else
	TransData.OutVertInt.CurValue = 0.0;
WidgetSNSync(IDC_PV_VERTINTERVAL, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeVertexSpacingFromNumVertices

/*===========================================================================*/

void PathTransferGUI::ComputeIntervalFromNumVertices(void)
{

if (TransData.OutNumVerts > 1)
	{
	TransData.OutFrameInt.CurValue = TransData.InPathTime / (TransData.OutNumVerts - 1);
	} // if
else
	TransData.OutFrameInt.CurValue = 0.0;
WidgetSNSync(IDC_PV_FRAMEINT, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeIntervalFromNumVertices

/*===========================================================================*/

void PathTransferGUI::ComputeNumVerticesFromInterval(void)
{

if (TransData.OutFrameInt.CurValue > 0.0)
	{
	TransData.OutNumVerts = 1 + (long)(TransData.InPathTime / TransData.OutFrameInt.CurValue);
	} // if
else
	TransData.OutNumVerts = 1;
WidgetSNSync(IDC_PV_NUMVERTS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VV_OUTVERTICES, WP_FISYNC_NONOTIFY);

} // PathTransferGUI::ComputeNumVerticesFromInterval

/*===========================================================================*/

void PathTransferGUI::Transfer(void)
{
double CurTime;
int Success = 1;
NotifyTag Changes[2];
AnimDoubleTime TempLat, TempLon, TempElev;

if (TransferFrom)
	{
	if (! TransferTo)
		{
		switch (OutType)
			{
			case WCS_RAHOST_OBJTYPE_VECTOR:
			case WCS_RAHOST_OBJTYPE_CONTROLPT:
				{
				TransferTo = DBHost->NewObject(NULL);
				break;
				} // 
			case WCS_EFFECTSSUBCLASS_CAMERA:
			case WCS_EFFECTSSUBCLASS_LIGHT:
			case WCS_EFFECTSSUBCLASS_CELESTIAL:
			case WCS_EFFECTSSUBCLASS_OBJECT3D:
			case WCS_EFFECTSSUBCLASS_WAVE:
			case WCS_EFFECTSSUBCLASS_CLOUD:
				{
				if (TransferTo = EffectsHost->AddEffect(OutType, NULL, NULL))
					TransferTo->SetFloating(0);
				break;
				} // 
			} // switch
		} // if
	if (TransferTo)
		{
		switch (OutType)
			{
			case WCS_RAHOST_OBJTYPE_VECTOR:
			case WCS_RAHOST_OBJTYPE_CONTROLPT:
				{
				if (OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
					((Joe *)TransferTo)->SetFlags(WCS_JOEFLAG_ISCONTROL);
				else
					((Joe *)TransferTo)->ClearFlags(WCS_JOEFLAG_ISCONTROL);
				break;
				} // 
			case WCS_EFFECTSSUBCLASS_CAMERA:
			case WCS_EFFECTSSUBCLASS_LIGHT:
			case WCS_EFFECTSSUBCLASS_CELESTIAL:
			case WCS_EFFECTSSUBCLASS_OBJECT3D:
			case WCS_EFFECTSSUBCLASS_WAVE:
			case WCS_EFFECTSSUBCLASS_CLOUD:
				{
				switch (OutType)
					{
					case WCS_EFFECTSSUBCLASS_CAMERA:
						{
						if (! OutCamTarget)
							{
							TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT);
							TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON);
							TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV);
							} // if
						else
							{
							TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT);
							TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON);
							TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV);
							} // else
						break;
						} // WCS_EFFECTSSUBCLASS_CAMERA
					case WCS_EFFECTSSUBCLASS_LIGHT:
						{
						TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LAT);
						TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_LON);
						TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_ELEV);
						break;
						} // WCS_EFFECTSSUBCLASS_LIGHT
					case WCS_EFFECTSSUBCLASS_CELESTIAL:
						{
						TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE);
						TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE);
						TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE);
						break;
						} // WCS_EFFECTSSUBCLASS_CELESTIAL
					case WCS_EFFECTSSUBCLASS_OBJECT3D:
						{
						TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT);
						TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_LON);
						TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV);
						break;
						} // WCS_EFFECTSSUBCLASS_OBJECT3D
					case WCS_EFFECTSSUBCLASS_WAVE:
						{
						TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE);
						TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE);
						TransData.OutPath.Elev = NULL;
						TransData.AlternateElev = 0.0;
						break;
						} // WCS_EFFECTSSUBCLASS_WAVE
					case WCS_EFFECTSSUBCLASS_CLOUD:
						{
						TransData.OutPath.Lat = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT);
						TransData.OutPath.Lon = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON);
						TransData.OutPath.Elev = ((GeneralEffect *)TransferTo)->GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV);
						break;
						} // WCS_EFFECTSSUBCLASS_CLOUD
					} // switch
				// make a backup copy of basic settings since they will get overwritten by copy operations during path creation
				TempLat.CopyRangeDefaults(TransData.OutPath.Lat);
				TempLon.CopyRangeDefaults(TransData.OutPath.Lon);
				if (TransData.OutPath.Elev)
					TempElev.CopyRangeDefaults(TransData.OutPath.Elev);
				break;
				} // 
			} // switch
		if (InType == WCS_RAHOST_OBJTYPE_VECTOR || InType == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			if (OutType == WCS_RAHOST_OBJTYPE_VECTOR || OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				Success = TransferVectorToVector();
				} // if vector out
			else
				{
				Success = TransferVectorToPath();
				} // else path out
			} // if vector source
		else
			{
			if (OutType == WCS_RAHOST_OBJTYPE_VECTOR || OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				Success = TransferPathToVector();
				} // if vector out
			else
				{
				Success = TransferPathToPath();
				} // else path out
			} // else path source
		// copy the correct metric type, multiplier, increment, value range from the backup
		if (OutType != WCS_RAHOST_OBJTYPE_VECTOR && OutType != WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			TransData.OutPath.Lat->CopyRangeDefaults(&TempLat);
			TransData.OutPath.Lon->CopyRangeDefaults(&TempLon);
			if (TransData.OutPath.Elev)
				TransData.OutPath.Elev->CopyRangeDefaults(&TempElev);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			TransData.OutPath.Lat->SetToTime(CurTime);
			TransData.OutPath.Lon->SetToTime(CurTime);
			if (TransData.OutPath.Elev)
				TransData.OutPath.Elev->SetToTime(CurTime);
			} // else
		// give user some feedback
		if (Success)
			UserMessageOK("Transfer Path", "Transfer complete.");
		else
			UserMessageOK("Transfer Path", "Transfer could not be completed.");
		// announce the changes
		if (MakeNew)
			Changes[0] = MAKE_ID(TransferTo->GetNotifyClass(), TransferTo->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		else
			Changes[0] = MAKE_ID(TransferTo->GetNotifyClass(), TransferTo->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, TransferTo);
		if (OutType == WCS_RAHOST_OBJTYPE_VECTOR || OutType == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			((Joe *)TransferTo)->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
			((Joe *)TransferTo)->ZeroUpTree();
			((Joe *)TransferTo)->RecheckBounds();
			DBHost->BoundUpTree((Joe *)TransferTo);
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
			DBHost->GenerateNotify(Changes);
			Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
			GlobalApp->AppEx->GenerateNotify(Changes, TransferTo);
			} // if
		// make the new thing the active thing
		RasterAnimHost::SetActiveRAHost(TransferTo);
		} // if
	else
		{
		UserMessageOK("Transfer Path", "An output object could not be created. Transfer could not be completed.");
		} // else no to
	} // if from

} // PathTransferGUI::Transfer

/*===========================================================================*/

int PathTransferGUI::TransferVectorToVector(void)
{
double Dist, SumDist, PlanetRad, VDist, LastLat, LastLon, LastElev, PathSegLen = 0.0;
long NewNumPoints;
VectorPoint *PrevLink, *PLinkA, *PLinkB, *NewPoints = NULL;
GraphNode *TempNode;
JoeCoordSys *MyAttr;
CoordSys *SourceCoords;
AnimPath OutPath;
AnimDoubleTime Lat, Lon, Elev;
VertexDEM Vert;

/*
TransData.ConsiderElev
TransData.InterpType  WCS_TRANSFER_INTERPTYPE_SPLINE, WCS_TRANSFER_INTERPTYPE_LINEAR
*/

Lat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Elev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutPath.Lat = &Lat;
OutPath.Lon = &Lon;
OutPath.Elev = &Elev;

if (MyAttr = (JoeCoordSys *)((Joe *)TransferFrom)->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	SourceCoords = MyAttr->Coord;
else
	SourceCoords = NULL;

if (TransData.InNumVerts > 0)
	{
	NewNumPoints = TransData.VV_VertType == WCS_TRANSFER_VERTCOUNT_NOCHANGE ? TransData.InNumVerts: TransData.OutNumVerts;
	if (NewNumPoints > 0)
		{
		if (NewPoints = DBHost->MasterPoint.Allocate(NewNumPoints + Joe::GetFirstRealPtNum()))
			{
			if (TransData.VV_VertType == WCS_TRANSFER_VERTCOUNT_NOCHANGE && TransData.VV_VertSpacing == WCS_TRANSFER_VERTSPACE_NOCHANGE)
				{
				for (PLinkA = ((Joe *)TransferFrom)->Points(), PLinkB = NewPoints; PLinkA && PLinkB; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
					{
					PLinkB->Latitude = PLinkA->Latitude;
					PLinkB->Longitude = PLinkA->Longitude;
					PLinkB->Elevation = PLinkA->Elevation;
					} // for
				} // if same number of vertices and same spacing
			else
				{
				SumDist = 0.0;
				PlanetRad = EffectsHost->GetPlanetRadius();
				PrevLink = NULL;
				for (PLinkA = ((Joe *)TransferFrom)->GetFirstRealPoint(); PLinkA; PLinkA = PLinkA->Next)
					{
					LastLat = Vert.Lat;
					LastLon = Vert.Lon;
					LastElev = Vert.Elev;
					if (PLinkA->ProjToDefDeg(SourceCoords, &Vert))
						{
						if (PrevLink)
							{
							Dist = FindDistance(LastLat, LastLon, Vert.Lat, Vert.Lon, PlanetRad);
							if (TransData.ConsiderElev)
								{
								VDist = Vert.Elev - LastElev;
								Dist = sqrt(Dist * Dist + VDist * VDist);
								} // if
							SumDist += Dist;
							} // if
						TempNode = Lat.AddNode(SumDist, Vert.Lat, 0.0);
						if (TempNode && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
							TempNode->SetLinear(1);
						TempNode = Lon.AddNode(SumDist, Vert.Lon, 0.0);
						if (TempNode && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
							TempNode->SetLinear(1);
						TempNode = Elev.AddNode(SumDist, Vert.Elev, 0.0);
						if (TempNode && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
							TempNode->SetLinear(1);
						} // if
					else
						break;
					PrevLink = PLinkA;
					} // for
				PathSegLen = NewNumPoints > 1 ? SumDist / (NewNumPoints - 1): 0.0;
				Dist = 0.0;
				for (PLinkA = Joe::GetFirstRealPtNum() > 0 ? NewPoints->Next: NewPoints; PLinkA; PLinkA = PLinkA->Next, Dist += PathSegLen)
					{
					Vert.Lat = Lat.GetValue(0, Dist);
					Vert.Lon = Lon.GetValue(0, Dist);
					Vert.Elev = Elev.GetValue(0, Dist);
					PLinkA->DefDegToProj(SourceCoords, &Vert);
					} // for
				#ifdef WCS_JOE_LABELPOINTEXISTS
				NewPoints->Latitude = NewPoints->Next->Latitude;
				NewPoints->Longitude = NewPoints->Next->Longitude;
				NewPoints->Elevation = NewPoints->Next->Elevation;
				#endif // WCS_JOE_LABELPOINTEXISTS
				} // else even spacing
			} // if
		} // if
	} // if
else
	UserMessageOK("Transfer Path", "Source vector has no vertices.");

if (NewPoints)
	{
	if (TransferFrom == TransferTo && ((Joe *)TransferFrom)->Points())
		DBHost->MasterPoint.DeAllocate(((Joe *)TransferFrom)->Points());
	((Joe *)TransferTo)->Points(NewPoints);
	((Joe *)TransferTo)->CountSetNumPoints();
	} // if
if (SourceCoords && TransferFrom != TransferTo)
	{
	((Joe *)TransferTo)->AddEffect(SourceCoords, 1);
	} // if

return (NewPoints ? 1: 0);

} // PathTransferGUI::TransferVectorToVector

/*===========================================================================*/

int PathTransferGUI::TransferVectorToPath(void)
{
double SumDist, PlanetRad, InvVelocity, VDist, LastLat, LastLon, LastElev, PathSegLen;
VectorPoint *PrevLink, *PLink;
GraphNode *TempNode;
JoeCoordSys *MyAttr;
CoordSys *SourceCoords;
PlanetOpt *PO;
AnimPath OutPath;
AnimDoubleTime Lat, Lon, Elev;
VertexDEM Vert;

Lat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Elev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutPath.Lat = &Lat;
OutPath.Lon = &Lon;
OutPath.Elev = &Elev;

PO = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, TRUE, DBHost);

if (MyAttr = (JoeCoordSys *)((Joe *)TransferFrom)->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	SourceCoords = MyAttr->Coord;
else
	SourceCoords = NULL;

if (TransData.InNumVerts > 0)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	PrevLink = NULL;
	SumDist = 0.0;
	InvVelocity = TransData.OutVelocity.CurValue > 0.0 ? 1.0 / TransData.OutVelocity.CurValue: 0.0;
	for (PLink = ((Joe *)TransferFrom)->GetFirstRealPoint(); PLink; PLink = PLink->Next)
		{
		LastLat = Vert.Lat;
		LastLon = Vert.Lon;
		LastElev = Vert.Elev;
		PLink->ProjToDefDeg(SourceCoords, &Vert);
		// if vertical exaggeration applied to terrain, apply it now to path
		Vert.Elev = CalcExag(Vert.Elev, PO);

		if (PrevLink)
			{
			if (TransData.VP_ConstUnit == WCS_TRANSFER_CONSTUNIT_INTERVAL)
				{
				SumDist += TransData.OutFrameInt.CurValue;
				} // if constant interval
			else
				{
				PathSegLen = FindDistance(LastLat, LastLon, Vert.Lat, Vert.Lon, PlanetRad);
				if (TransData.ConsiderElev)
					{
					VDist = Vert.Elev - LastElev;
					PathSegLen = sqrt(PathSegLen * PathSegLen + VDist * VDist);
					} // if
				SumDist += PathSegLen * InvVelocity;
				} // else
			} // else constant velocity
		TempNode = Lat.AddNode(SumDist, Vert.Lat, 0.0);
		if (TempNode && TransData.VP_KeyPlace == WCS_TRANSFER_KEYPLACE_FIXEDCOUNT && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
			TempNode->SetLinear(1);
		TempNode = Lon.AddNode(SumDist, Vert.Lon, 0.0);
		if (TempNode && TransData.VP_KeyPlace == WCS_TRANSFER_KEYPLACE_FIXEDCOUNT && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
			TempNode->SetLinear(1);
		TempNode = Elev.AddNode(SumDist, Vert.Elev + TransData.ElevAdd.CurValue, 0.0);
		if (TempNode && TransData.VP_KeyPlace == WCS_TRANSFER_KEYPLACE_FIXEDCOUNT && TransData.InterpType == WCS_TRANSFER_INTERPTYPE_LINEAR)
			TempNode->SetLinear(1);
		PrevLink = PLink;
		} // for
	if (TransData.VP_KeyPlace == WCS_TRANSFER_KEYPLACE_FIXEDCOUNT)
		{
		OutPath.Smooth(TransData.ConsiderElev, TransData.InFrameRate, LatScale(EffectsHost->GetPlanetRadius()), TransData.OutNumKeys);
		} // else
	} // if
else
	UserMessageOK("Transfer Path", "Source vector has no vertices.");

TransData.OutPath.Lat->Copy(TransData.OutPath.Lat, &Lat);
TransData.OutPath.Lon->Copy(TransData.OutPath.Lon, &Lon);
if (TransData.OutPath.Elev)
	TransData.OutPath.Elev->Copy(TransData.OutPath.Elev, &Elev);

return (1);

} // PathTransferGUI::TransferVectorToPath

/*===========================================================================*/

int PathTransferGUI::TransferPathToVector(void)
{
double Dist, CurDist, CurTime, FirstTime, FinalTime, TimeInterval, DistInterval, TestElev, PlanetRad, 
	VDist, LastLat, LastLon, LastElev, CurLat, CurLon, CurElev, PathSegLen, Elevation;
long NodeCt, NodesAdded = 0;
unsigned long int Flags;
VectorPoint *PLink;
GraphNode *LatNode, *LonNode, *ElevNode;
JoeCoordSys *MyAttr;
CoordSys *SourceCoords;
VectorPoint *NewPoints = NULL;
RenderData *RendData = NULL;
PlanetOpt *PO;
int Reject;
AnimPath OutPath;
AnimDoubleTime Lat, Lon, Elev, Lat2, Lon2, Elev2;
VertexData Vert;
PolygonData Poly;

Lat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Elev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lat2.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lon2.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Elev2.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

PO = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, TRUE, DBHost);

if (MyAttr = (JoeCoordSys *)((Joe *)TransferFrom)->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	SourceCoords = MyAttr->Coord;
else
	SourceCoords = NULL;

if (TransData.PV_VertPlace == WCS_TRANSFER_KEYPLACE_EACHVERTEX)
	{
	Dist = -1.0;
	while (TransData.InPath.GetNextUniqueKey(Dist))
		{
		Lat.AddNode(Dist, TransData.InPath.Lat->GetValue(0, Dist), 0.0);
		Lon.AddNode(Dist, TransData.InPath.Lon->GetValue(0, Dist), 0.0);
		Elevation = TransData.InPath.Elev ? TransData.InPath.Elev->GetValue(0, Dist): TransData.AlternateElev;
		// if vertical exaggeration applied to terrain, unapply it now to vector
		Elevation = UnCalcExag(Elevation, PO);
		Elev.AddNode(Dist, Elevation, 0.0);
		NodesAdded ++;
		} // while
	if (! NodesAdded)
		{
		Lat.AddNode(0.0, TransData.InPath.Lat->GetValue(0, 0.0), 0.0);
		Lon.AddNode(0.0, TransData.InPath.Lon->GetValue(0, 0.0), 0.0);
		Elevation = TransData.InPath.Elev ? TransData.InPath.Elev->GetValue(0, 0.0): TransData.AlternateElev;
		// if vertical exaggeration applied to terrain, unapply it now to vector
		Elevation = UnCalcExag(Elevation, PO);
		Elev.AddNode(0.0, Elevation, 0.0);
		NodesAdded = 1;
		} // if no nodes added, add one
	OutPath.Lat = &Lat;
	OutPath.Lon = &Lon;
	OutPath.Elev = &Elev;
	} // if vertex at each key frame
else if (TransData.PV_VertPlace == WCS_TRANSFER_KEYPLACE_EACHINTERVAL)
	{
	FirstTime = TransData.InFirstKey;
	FinalTime = TransData.InFirstKey + TransData.InPathTime;
	TimeInterval = TransData.OutFrameInt.CurValue > 0.0 ? TransData.OutFrameInt.CurValue: 1.0;
	for (CurTime = FirstTime; CurTime <= FinalTime; CurTime += TimeInterval)
		{
		Lat.AddNode(CurTime, TransData.InPath.Lat->GetValue(0, CurTime), 0.0);
		Lon.AddNode(CurTime, TransData.InPath.Lon->GetValue(0, CurTime), 0.0);
		Elevation = TransData.InPath.Elev ? TransData.InPath.Elev->GetValue(0, CurTime): TransData.AlternateElev;
		// if vertical exaggeration applied to terrain, unapply it now to vector
		Elevation = UnCalcExag(Elevation, PO);
		Elev.AddNode(CurTime, Elevation, 0.0);
		NodesAdded ++;
		} // for
	OutPath.Lat = &Lat;
	OutPath.Lon = &Lon;
	OutPath.Elev = &Elev;
	} // else if
else
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	CurTime = -1.0;
	Dist = 0.0;
	while (TransData.InPath.GetNextUniqueKey(CurTime))
		{
		CurLat = TransData.InPath.Lat->GetValue(0, CurTime);
		CurLon = TransData.InPath.Lon->GetValue(0, CurTime);
		Elevation = TransData.InPath.Elev ? TransData.InPath.Elev->GetValue(0, CurTime): TransData.AlternateElev;
		// if vertical exaggeration applied to terrain, unapply it now to vector
		Elevation = UnCalcExag(Elevation, PO);
		CurElev = Elevation;
		if (NodesAdded)
			{
			PathSegLen = FindDistance(LastLat, LastLon, CurLat, CurLon, PlanetRad);	//lint !e530
			if (TransData.ConsiderElev)
				{
				VDist = CurElev - LastElev;	//lint !e530
				PathSegLen = sqrt(PathSegLen * PathSegLen + VDist * VDist);
				} // if
			Dist += PathSegLen;
			} // if
		Lat.AddNode(Dist, CurLat, 0.0);
		Lon.AddNode(Dist, CurLon, 0.0);
		Elev.AddNode(Dist, CurElev, 0.0);
		LastLat = CurLat;
		LastLon = CurLon;
		LastElev = CurElev;
		NodesAdded ++;
		} // while
	if (! NodesAdded)
		{
		Lat.AddNode(0.0, TransData.InPath.Lat->GetValue(0, 0.0), 0.0);
		Lon.AddNode(0.0, TransData.InPath.Lon->GetValue(0, 0.0), 0.0);
		Elevation = TransData.InPath.Elev ? TransData.InPath.Elev->GetValue(0, 0.0): TransData.AlternateElev;
		// if vertical exaggeration applied to terrain, unapply it now to vector
		Elevation = UnCalcExag(Elevation, PO);
		Elev.AddNode(0.0, Elevation, 0.0);
		NodesAdded = 1;
		} // if
	// sample at a fixed distance into yet another path
	DistInterval = TransData.OutVertInt.CurValue > 0.0 ? TransData.OutVertInt.CurValue: FLT_MAX;
	CurDist = 0.0;
	for (NodeCt = 0; NodeCt < TransData.OutNumVerts && CurDist <= Dist; NodeCt ++, CurDist += DistInterval)
		{
		Lat2.AddNode(CurDist, Lat.GetValue(0, CurDist), 0.0);
		Lon2.AddNode(CurDist, Lon.GetValue(0, CurDist), 0.0);
		Elev2.AddNode(CurDist, Elev.GetValue(0, CurDist), 0.0);
		} // for
	OutPath.Lat = &Lat2;
	OutPath.Lon = &Lon2;
	OutPath.Elev = &Elev2;
	} // else fixed count

// allocate number of vertices
NodeCt = 0;
LatNode = OutPath.Lat->GetFirstNode(0);
LonNode = OutPath.Lon->GetFirstNode(0);
ElevNode = OutPath.Elev->GetFirstNode(0);
while (LatNode && LonNode && ElevNode)
	{
	NodeCt ++;
	LatNode = OutPath.Lat->GetNextNode(0, LatNode);
	LonNode = OutPath.Lon->GetNextNode(0, LonNode);
	ElevNode = OutPath.Elev->GetNextNode(0, ElevNode);
	} // while

if (TransData.ConformTopo && TransData.ConsiderTfx)
	{
	Flags = WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED;
	if (RendData = new RenderData(NULL))
		RendData->InitToView(EffectsHost, ProjHost, DBHost, ProjHost->Interactive, NULL, NULL, 100, 100);
	} // if

if (NodeCt > 0 && (NewPoints = DBHost->MasterPoint.Allocate(NodeCt + Joe::GetFirstRealPtNum())))
	{
	// transfer
	LatNode = OutPath.Lat->GetFirstNode(0);
	LonNode = OutPath.Lon->GetFirstNode(0);
	ElevNode = OutPath.Elev->GetFirstNode(0);
	for (PLink = Joe::GetFirstRealPtNum() > 0 ? NewPoints->Next: NewPoints; PLink && LatNode && LonNode && ElevNode; PLink = PLink->Next)
		{
		PLink->Latitude = LatNode->GetValue();
		PLink->Longitude = LonNode->GetValue();
		if (TransData.ConformTopo)
			{
			if (TransData.ConsiderTfx && RendData)
				{
				if (ProjHost->Interactive->VertexDataPoint(RendData, &Vert, &Poly, Flags))
					{
					if (RendData->Exageration != 0.0)
						Vert.Elev = RendData->ElevDatum + (Vert.Elev - RendData->ElevDatum) / RendData->Exageration;
					PLink->Elevation = (float)(Vert.Elev + TransData.ElevAdd.CurValue);
					} // if
				else
					PLink->Elevation = (float)TransData.ElevAdd.CurValue;
				} // if
			else
				{
				TestElev = ProjHost->Interactive->ElevationPointNULLReject(PLink->Latitude, PLink->Longitude, Reject);
				if (! Reject)
					PLink->Elevation = (float)(TestElev + TransData.ElevAdd.CurValue);
				else
					PLink->Elevation = (float)TransData.ElevAdd.CurValue;
				} // else
			} // if
		else
			PLink->Elevation = (float)(ElevNode->GetValue() + TransData.ElevAdd.CurValue);
		LatNode = OutPath.Lat->GetNextNode(0, LatNode);
		LonNode = OutPath.Lon->GetNextNode(0, LonNode);
		ElevNode = OutPath.Elev->GetNextNode(0, ElevNode);
		} // for
	#ifdef WCS_JOE_LABELPOINTEXISTS
	NewPoints->Latitude = NewPoints->Next->Latitude;
	NewPoints->Longitude = NewPoints->Next->Longitude;
	NewPoints->Elevation = NewPoints->Next->Elevation;
	#endif // WCS_JOE_LABELPOINTEXISTS
	} // if

if (RendData)
	delete RendData;

if (NewPoints)
	{
	if (((Joe *)TransferTo)->Points())
		DBHost->MasterPoint.DeAllocate(((Joe *)TransferTo)->Points());
	((Joe *)TransferTo)->Points(NewPoints);
	((Joe *)TransferTo)->CountSetNumPoints();
	} // if

return (NewPoints ? 1: 0);

} // PathTransferGUI::TransferPathToVector

/*===========================================================================*/

int PathTransferGUI::TransferPathToPath(void)
{
double TimeChange, NewDist;
AnimPath OutPath;
AnimDoubleTime Lat, Lon, Elev;
GraphNode *CurNode, *NewNode;

Lat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Lon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
Elev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
OutPath.Lat = &Lat;
OutPath.Lon = &Lon;
if (TransData.InPath.Elev)
	OutPath.Elev = &Elev;

if (TransData.InPathTime > 0.0)
	TimeChange = TransData.OutPathTime.CurValue / TransData.InPathTime;
else
	TimeChange = 1.0;

for (CurNode = TransData.InPath.Lat->GetFirstNode(0); CurNode; CurNode = TransData.InPath.Lat->GetNextNode(0, CurNode))
	{
	NewDist = (CurNode->GetDistance() - TransData.InFirstKey) * TimeChange + TransData.OutFirstKey.CurValue;
	NewNode = Lat.AddNode(NewDist, CurNode->GetValue(), 0.0);
	if (NewNode && CurNode->GetLinear())
		NewNode->SetLinear(1);
	} // for
for (CurNode = TransData.InPath.Lon->GetFirstNode(0); CurNode; CurNode = TransData.InPath.Lon->GetNextNode(0, CurNode))
	{
	NewDist = (CurNode->GetDistance() - TransData.InFirstKey) * TimeChange + TransData.OutFirstKey.CurValue;
	NewNode = Lon.AddNode(NewDist, CurNode->GetValue(), 0.0);
	if (NewNode && CurNode->GetLinear())
		NewNode->SetLinear(1);
	} // for
if (TransData.InPath.Elev)
	{
	for (CurNode = TransData.InPath.Elev->GetFirstNode(0); CurNode; CurNode = TransData.InPath.Elev->GetNextNode(0, CurNode))
		{
		NewDist = (CurNode->GetDistance() - TransData.InFirstKey) * TimeChange + TransData.OutFirstKey.CurValue;
		NewNode = Elev.AddNode(NewDist, CurNode->GetValue(), 0.0);
		if (NewNode && CurNode->GetLinear())
			NewNode->SetLinear(1);
		} // for
	} // if

if (TransData.PP_VelocityType == WCS_TRANSFER_VELOCITYTYPE_CONSTANT)
	{
	OutPath.Smooth(TransData.ConsiderElev, TransData.InFrameRate, LatScale(EffectsHost->GetPlanetRadius()), TransData.OutNumKeys);
	} // else

if (TransferTo == TransferFrom)
	{
	TransData.InPath.Lat->Copy(TransData.InPath.Lat, &Lat);
	TransData.InPath.Lon->Copy(TransData.InPath.Lon, &Lon);
	if (TransData.InPath.Elev)
		TransData.InPath.Elev->Copy(TransData.InPath.Elev, &Elev);
	} // if
else
	{
	TransData.OutPath.Lat->Copy(TransData.OutPath.Lat, &Lat);
	TransData.OutPath.Lon->Copy(TransData.OutPath.Lon, &Lon);
	if (TransData.OutPath.Elev)
		TransData.OutPath.Elev->Copy(TransData.OutPath.Elev, &Elev);
	} // else

return (1);

} // PathTransferGUI::TransferPathToPath

/*===========================================================================*/
