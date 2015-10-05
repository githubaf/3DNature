// CoordsCalculatorGUI.cpp
// Code for Coordinate system transformation calculator
// Created from scratch on 01/31/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CoordsCalculatorGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Render.h"
#include "resource.h"

extern WCSApp *GlobalApp;

NativeGUIWin CoordsCalculatorGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CoordsCalculatorGUI::Open

/*===========================================================================*/

NativeGUIWin CoordsCalculatorGUI::Construct(void)
{
#ifdef WCS_BUILD_VNS
int TabIndex;
GeneralEffect *MyEffect;
		#endif // WCS_BUILD_VNS

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_COORDS_TRANSFORM, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		WidgetCBInsert(IDC_COORDSDROP2, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, TabIndex, MyEffect);
			TabIndex = WidgetCBInsert(IDC_COORDSDROP2, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP2, TabIndex, MyEffect);
			} // for
		#endif // WCS_BUILD_VNS
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // CoordsCalculatorGUI::Construct

/*===========================================================================*/

CoordsCalculatorGUI::CoordsCalculatorGUI(EffectsLib *EffectsSource, CoordSys *ActiveSource)
: GUIFenetre('CSCU', this, "Coordinate System Calculator") // Yes, I know...
{
static NotifyTag AllEvents[] = {
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

ConstructError = 0;
EffectsHost = EffectsSource;
Source = ActiveSource;
Dest = NULL;
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
SourceElev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
DestElev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ReverseElev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

if (EffectsSource)
	{
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // CoordsCalculatorGUI::CoordsCalculatorGUI

/*===========================================================================*/

CoordsCalculatorGUI::~CoordsCalculatorGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // CoordsCalculatorGUI::~CoordsCalculatorGUI()

/*===========================================================================*/

long CoordsCalculatorGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_CSC, 0);

return(0);

} // CoordsCalculatorGUI::HandleCloseWin

/*===========================================================================*/

long CoordsCalculatorGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case ID_KEEP:
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CSC, 0);
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (Source)
			Source->EditRAHost();
		break;
		} // 
	case IDC_EDITCOORDS2:
		{
		if (Dest)
			Dest->EditRAHost();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // CoordsCalculatorGUI::HandleButtonClick

/*===========================================================================*/

long CoordsCalculatorGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_COORDSDROP:
		{
		SelectNewSource();
		break;
		}
	case IDC_COORDSDROP2:
		{
		SelectNewDest();
		break;
		}
	} // switch CtrlID

return (0);

} // CoordsCalculatorGUI::HandleCBChange

/*===========================================================================*/

long CoordsCalculatorGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

if (CtrlID == IDC_NORTHING || CtrlID == IDC_EASTING || CtrlID == IDC_ELEVATION)
	{
	ComputeTransforms();
	} // if

return(0);

} // CoordsCalculatorGUI::HandleFIChange

/*===========================================================================*/

void CoordsCalculatorGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes;
long Done = 0;
#ifdef WCS_BUILD_VNS
NotifyTag Interested[7];
long Pos, CurPosSource, CurPosDest;
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

#ifdef WCS_BUILD_VNS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPosSource = CurPosDest = -1;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBClear(IDC_COORDSDROP2);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	WidgetCBInsert(IDC_COORDSDROP2, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == Source)
			CurPosSource = Pos;
		Pos = WidgetCBInsert(IDC_COORDSDROP2, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP2, Pos, MyEffect);
		if (MyEffect == Dest)
			CurPosDest = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPosSource);
	WidgetCBSetCurSel(IDC_COORDSDROP2, CurPosDest);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		Done = 1;
		} // if
	if (Source)
		{
		if (Source->GetGeographic())
			{
			SourceEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
			SourceNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
			ReverseEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
			ReverseNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
			WidgetSetText(IDC_NORTHING, "Latitude ");
			WidgetSetText(IDC_NORTHING3, "Latitude ");
			WidgetSetText(IDC_EASTING, "Longitude ");
			WidgetSetText(IDC_EASTING3, "Longitude ");
			} // if
		else
			{
			SourceEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			SourceNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			ReverseEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			ReverseNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			WidgetSetText(IDC_NORTHING, "Northing ");
			WidgetSetText(IDC_NORTHING3, "Northing ");
			WidgetSetText(IDC_EASTING, "Easting ");
			WidgetSetText(IDC_EASTING3, "Easting ");
			} // else
		} // if
	if (Dest)
		{
		if (Dest->GetGeographic())
			{
			DestEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
			DestNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
			WidgetSetText(IDC_NORTHING2, "Latitude ");
			WidgetSetText(IDC_EASTING2, "Longitude ");
			} // if
		else
			{
			DestEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			DestNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			WidgetSetText(IDC_NORTHING2, "Northing ");
			WidgetSetText(IDC_EASTING2, "Easting ");
			} // else
		} // if
	SyncWidgets();
	} // if Coordinate System name changed

#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // CoordsCalculatorGUI::HandleNotifyEvent()

/*===========================================================================*/

void CoordsCalculatorGUI::ConfigureWidgets(void)
{
#ifdef WCS_BUILD_VNS
long Ct, ListPos, NumEntries;
CoordSys *TestObj;
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_VNS
if (Source)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestObj == Source)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
if (Dest)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP2);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP2, Ct)) != (CoordSys *)LB_ERR && TestObj == Dest)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP2, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP2, -1);
if (Source)
	{
	if (Source->GetGeographic())
		{
		SourceEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		SourceNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		ReverseEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		ReverseNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		WidgetSetText(IDC_NORTHING, "Latitude ");
		WidgetSetText(IDC_NORTHING3, "Latitude ");
		WidgetSetText(IDC_EASTING, "Longitude ");
		WidgetSetText(IDC_EASTING3, "Longitude ");
		} // if
	else
		{
		SourceEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		SourceNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		ReverseEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		ReverseNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		WidgetSetText(IDC_NORTHING, "Northing ");
		WidgetSetText(IDC_NORTHING3, "Northing ");
		WidgetSetText(IDC_EASTING, "Easting ");
		WidgetSetText(IDC_EASTING3, "Easting ");
		} // else
	} // if
if (Dest)
	{
	if (Dest->GetGeographic())
		{
		DestEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		DestNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		WidgetSetText(IDC_NORTHING2, "Latitude ");
		WidgetSetText(IDC_EASTING2, "Longitude ");
		} // if
	else
		{
		DestEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		DestNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		WidgetSetText(IDC_NORTHING2, "Northing ");
		WidgetSetText(IDC_EASTING2, "Easting ");
		} // else
	} // if
#endif // WCS_BUILD_VNS

WidgetSNConfig(IDC_NORTHING, &SourceNorth);
WidgetSNConfig(IDC_EASTING, &SourceEast);
WidgetSNConfig(IDC_ELEVATION, &SourceElev);
WidgetSNConfig(IDC_NORTHING2, &DestNorth);
WidgetSNConfig(IDC_EASTING2, &DestEast);
WidgetSNConfig(IDC_ELEVATION2, &DestElev);
WidgetSNConfig(IDC_NORTHING3, &ReverseNorth);
WidgetSNConfig(IDC_EASTING3, &ReverseEast);
WidgetSNConfig(IDC_ELEVATION3, &ReverseElev);

} // CoordsCalculatorGUI::ConfigureWidgets()

/*===========================================================================*/

void CoordsCalculatorGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_NORTHING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASTING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVATION, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTHING2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASTING2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVATION2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTHING3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASTING3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVATION3, WP_FISYNC_NONOTIFY);

} // CoordsCalculatorGUI::SyncWidgets

/*===========================================================================*/

void CoordsCalculatorGUI::SelectNewSource(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Source = NewObj;
	ConfigureWidgets();
	ComputeTransforms();
	} // if
#endif // WCS_BUILD_VNS
} // CoordsCalculatorGUI::SelectNewSource

/*===========================================================================*/

void CoordsCalculatorGUI::SelectNewDest(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP2);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP2, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Dest = NewObj;
	ConfigureWidgets();
	ComputeTransforms();
	} // if
#endif // WCS_BUILD_VNS
} // CoordsCalculatorGUI::SelectNewDest

/*===========================================================================*/

void CoordsCalculatorGUI::ComputeTransforms(void)
{
#ifdef WCS_BUILD_VNS
VertexDEM TForm;

// commented out version transforms through default degrees
// other version transforms through world cartesian but not through default degrees.
// The default degrees one will mess things up mightily if elevation is locked down in CoordSys.cpp as it is
// in VNS release builds when the default CS has a very different planet radius (e.g. Back Compatible Sphere) from the 
// system being converted from and/or to.

if (Source && Dest)
	{
	TForm.xyz[0] = SourceEast.CurValue;
	TForm.xyz[1] = SourceNorth.CurValue;
	TForm.xyz[2] = SourceElev.CurValue;

	Source->ProjToDefDeg(&TForm);
	Dest->DefDegToProj(&TForm);
	//Source->ProjToCart(&TForm);
	//Dest->CartToProj(&TForm);

	DestEast.CurValue = TForm.xyz[0];
	DestNorth.CurValue = TForm.xyz[1];
	DestElev.CurValue = TForm.xyz[2];

	Dest->ProjToDefDeg(&TForm);
	Source->DefDegToProj(&TForm);
	//Dest->ProjToCart(&TForm);
	//Source->CartToProj(&TForm);

	ReverseEast.CurValue = TForm.xyz[0];// - SourceEast.CurValue;
	ReverseNorth.CurValue = TForm.xyz[1];// - SourceNorth.CurValue;
	ReverseElev.CurValue = TForm.xyz[2];// - SourceElev.CurValue;

	SyncWidgets();
	} // if

#endif // WCS_BUILD_VNS
} // CoordsCalculatorGUI::ComputeTransforms

/*===========================================================================*/
