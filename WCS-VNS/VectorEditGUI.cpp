// VectorEditGUI.cpp
// Code for Vector editor
// Built from ColorEditGUI.cpp on 11/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "VectorEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Joe.h"
#include "Interactive.h"
#include "DigitizeGUI.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "MathSupport.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "Conservatory.h"
#include "PathTransferGUI.h"
#include "resource.h"

long VectorEditGUI::ActivePage;

/*===========================================================================*/

bool VectorEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANUNDO:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // VectorEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin VectorEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // VectorEditGUI::Open

/*===========================================================================*/

NativeGUIWin VectorEditGUI::Construct(void)
{
const char *StyleList[] = {"Point", "Circle", "Square", "Cross",
	"Solid", "Dotted", "Dashed", "Broken", NULL};
const char *ClassList[] = {"Vector", "Control Pts", NULL };
int ListEntry;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_VECTOR_EDIT1, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_PROPERTIES_VNS1, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_PROPERTIES1, 0, 0);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_EXTENTS, 0, 1);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_ACTIVEPT, 0, 2);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_POINTS1, 0, 3);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_SMOOTH1, 0, 4);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_SCALE1, 0, 5);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_REFERENCE1, 0, 6);
	CreateSubWinFromTemplate(IDD_VECTOR_EDIT_TRANSFER1, 0, 7);

	if (NativeWin)
		{
		for (ListEntry=0; ListEntry<2; ListEntry++)
			WidgetCBInsert(IDC_CLASSDROP, -1, ClassList[ListEntry]);
		for (ListEntry=0; ListEntry<8; ListEntry++)
			WidgetCBInsert(IDC_STYLEDROP, -1, StyleList[ListEntry]);

		WidgetCBAddEnd(IDC_HORUNITSDROP, "Degrees");
		WidgetCBAddEnd(IDC_HORUNITSDROP, "Distance");

		WidgetTCInsertItem(IDC_TAB1, 0, "Properties");
		WidgetTCInsertItem(IDC_TAB1, 1, "Extents");
		WidgetTCInsertItem(IDC_TAB1, 2, "Selected Points");
		WidgetTCInsertItem(IDC_TAB1, 3, "Point Operations");
		WidgetTCInsertItem(IDC_TAB1, 4, "Smooth");
		WidgetTCInsertItem(IDC_TAB1, 5, "Scale, Move, Rotate");
		WidgetTCInsertItem(IDC_TAB1, 6, "Reference");
		WidgetTCInsertItem(IDC_TAB1, 7, "Transfer");
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);

		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);

} // VectorEditGUI::Construct

/*===========================================================================*/

VectorEditGUI::VectorEditGUI(Database *DBSource, Project *ProjSource, EffectsLib *EffectsSource, InterCommon *ISource)
: GUIFenetre('VTEG', this, "Vector Editor") // Yes, I know...
{
static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllVecEvents[] = {MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, 0xff, 0xff),
								MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, 0xff, 0xff),
								MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff),
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllDigEvents[] = {MAKE_ID(WCS_PROJECTCLASS_MODULE, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

ConstructError = 0;
IamResponsible = 0;
DBHost = DBSource;
ProjHost = ProjSource;
EffectsHost = EffectsSource;
InterStash = ISource;
DigInfo = NULL;
Enabled = DrawEnabled = RenderEnabled = LineWeight = Red = Green = Blue = LineStyle = Class = 0;
OldMaxEl = OldMinEl = 0.0;
OldWidth = OldHeight = 0.0;
Backup = NULL;

VecNorth.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecNorth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
VecNorth.SetIncrement(.0001);
VecSouth.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecSouth.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
VecSouth.SetIncrement(.0001);
VecWest.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecWest.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
VecWest.SetIncrement(.0001);
VecEast.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecEast.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
VecEast.SetIncrement(.0001);
MaxEl.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
MaxEl.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
MaxEl.SetIncrement(1.0);
MinEl.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
MinEl.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
MinEl.SetIncrement(1.0);
PtX.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
PtX.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
PtX.SetIncrement(1.0);
PtY.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
PtY.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
PtY.SetIncrement(1.0);
PtZ.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
PtZ.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
PtZ.SetIncrement(1.0);
VecWidth.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecWidth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
VecWidth.SetIncrement(1.0);
VecHeight.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
VecHeight.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
VecHeight.SetIncrement(1.0);

ArbLat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ArbLat.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
ArbLat.SetIncrement(1.0);
ArbLon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ArbLon.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
ArbLon.SetIncrement(1.0);
ArbElev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ArbElev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ArbElev.SetIncrement(1.0);
ProjLat.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ProjLat.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
ProjLat.SetIncrement(1.0);
ProjLon.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ProjLon.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
ProjLon.SetIncrement(1.0);
ProjElev.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ProjElev.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ProjElev.SetIncrement(1.0);

ShiftAmtX.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ShiftAmtX.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
ShiftAmtX.SetIncrement(1.0);
ShiftAmtY.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ShiftAmtY.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
ShiftAmtY.SetIncrement(1.0);
ShiftAmtZ.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
ShiftAmtZ.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ShiftAmtZ.SetIncrement(1.0);
RotateAmtZ.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
RotateAmtZ.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
RotateAmtZ.SetIncrement(1.0);

DBHost->RegisterClient(this, AllDBEvents);
ProjHost->RegisterClient(this, AllDigEvents);
InterStash->RegisterClient(this, AllVecEvents);
GlobalApp->AppEx->RegisterClient(this, AllVecEvents);
if (! (Active = DBHost->GetActiveObj()))
	ConstructError = 1;
else if (Active->TestFlags(WCS_JOEFLAG_ISDEM))
	{
	ConstructError = 1;
	Active = NULL;
	NoJoeMessage();
	} // else if not vector
else
	{
	if (! (Backup = new ((char *)Active->Name()) Joe))
		ConstructError = 1;
	else if (! Active->CopyPoints(Backup, Active, 0, 1))
		ConstructError = 1;
	} // else all is well so far

VectorColor.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // VectorEditGUI::VectorEditGUI

/*===========================================================================*/

VectorEditGUI::~VectorEditGUI()
{

DBHost->RemoveClient(this);
ProjHost->RemoveClient(this);
InterStash->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (Backup)
	delete Backup;

} // VectorEditGUI::~VectorEditGUI()

/*===========================================================================*/

void VectorEditGUI::NewActive(void)
{

if (Backup)
	delete Backup;
Backup = NULL;
if (Active)
	{
	if (Backup = new ((char *)Active->Name()) Joe)
		Active->CopyPoints(Backup, Active, 0, 1);
	} // if

} // VectorEditGUI::NewActive

/*===========================================================================*/

long VectorEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_VEG, 0);

return(0);

} // VectorEditGUI::HandleCloseWin

/*===========================================================================*/

long VectorEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
#endif // WCS_BUILD_VNS

SECURITY_INLINE_CHECK(059, 59);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VEG, 0);
		break;
		} // 
	case IDCANCEL:
		{
		if (Active && Backup)
			Active->CopyPoints(Active, Backup, 1, 1);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VEG, 0);
		break;
		} // 
	case IDC_VECPROFILE:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_VPG, 0);
		break;
		} // 
	case IDC_CREATECOPY:
		{
		CreateCopy();
		break;
		} // 
	case IDC_WINUNDO: // from titlebar
	case IDC_UNDO:
		{
		UndoPoints();
		break;
		} // 
	case IDC_RESET:
		{
		RestorePoints();
		break;
		} // 
	case IDC_COPYTOCAMERA:
		{
		VectorToPath();
		break;
		} // 
	case IDC_COPYFROMCAMERA:
		{
		PathToVector();
		break;
		} // 
	case IDC_CHANGENUMPOINTS:
		{
		InterpolatePoints();
		break;
		} // 
	case IDC_INSERT:
		{
		InsertPoints();
		break;
		} // 
	case IDC_CLOSEORIGIN:
		{
		CloseOrigin();
		break;
		} // 
	case IDC_REVERSEPOINTS:
		{
		ReversePoints();
		break;
		} // 
	case IDC_SETORIGIN:
		{
		SetOrigin();
		break;
		} // 
	case IDC_DELETE:
		{
		DeletePoints();
		break;
		} // 
	case IDC_SCALEX:
		{
		SetScale(WCS_INTERVEC_COMP_X, IDC_SCALEX);
		break;
		} // 
	case IDC_SCALEY:
		{
		SetScale(WCS_INTERVEC_COMP_Y, IDC_SCALEY);
		break;
		} // 
	case IDC_SCALEZ:
		{
		SetScale(WCS_INTERVEC_COMP_Z, IDC_SCALEZ);
		break;
		} // 
	case IDC_SHIFTX:
		{
		SetShift(WCS_INTERVEC_COMP_X, IDC_SHIFTX);
		break;
		} // 
	case IDC_SHIFTY:
		{
		SetShift(WCS_INTERVEC_COMP_Y, IDC_SHIFTY);
		break;
		} // 
	case IDC_SHIFTZ:
		{
		SetShift(WCS_INTERVEC_COMP_Z, IDC_SHIFTZ);
		break;
		} // 
	case IDC_ROTATE:
		{
		SetRotate(IDC_ROTATE);
		break;
		} // 
	case IDC_OPERATE:
		{
		ScaleRotateShift();
		break;
		} // 
	case IDC_SMOOTH:
		{
		SmoothPoints();
		break;
		} // 
	case IDC_SPACING:
		{
		EvenSpacing();
		break;
		} // 
	case IDC_ENABLED:
		{
		DoEnable();
		break;
		} // 
	case IDC_ENABLED2:
		{
		DoDrawEnable();
		break;
		} // 
	case IDC_ENABLED3:
		{
		DoRenderEnable();
		break;
		} // 
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

} // VectorEditGUI::HandleButtonClick

/*===========================================================================*/

long VectorEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_HORUNITSDROP:
		{
		SetHorUnits();
		break;
		}
	case IDC_CLASSDROP:
		{
		DoObjectClass();
		break;
		} // day
	case IDC_STYLEDROP:
		{
		DoLineStyle();
		break;
		} // day
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

} // VectorEditGUI::HandleCBChange

/*===========================================================================*/

long VectorEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// note that Handle is 0 if called from ConfigureWidgets
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
			case 3:
				{
				ShowPanel(0, 3);
				break;
				} // 3
			case 4:
				{
				ShowPanel(0, 4);
				break;
				} // 4
			case 5:
				{
				ShowPanel(0, 5);
				break;
				} // 5
			case 6:
				{
				ShowPanel(0, 6);
				break;
				} // 6
			case 7:
				{
				ShowPanel(0, 7);
				break;
				} // 7
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

} // VectorEditGUI::HandlePageChange

/*===========================================================================*/

long VectorEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_VECNORTH:
		{
		ChangeNorth();
		break;
		} // IDC_VECNORTH
	case IDC_VECSOUTH:
		{
		ChangeSouth();
		break;
		} // IDC_VECSOUTH
	case IDC_VECWEST:
		{
		ChangeWest();
		break;
		} // IDC_VECWEST
	case IDC_VECEAST:
		{
		ChangeEast();
		break;
		} // IDC_VECEAST
	case IDC_MAXEL:
		{
		ChangeHighElev();
		break;
		} // IDC_MAXEL
	case IDC_MINEL:
		{
		ChangeLowElev();
		break;
		} // IDC_MINEL
	case IDC_VECWIDTH:
		{
		ChangeWidth();
		break;
		} // IDC_VECWIDTH
	case IDC_VECHEIGHT:
		{
		ChangeHeight();
		break;
		} // IDC_VECHEIGHT
	case IDC_PTLAT:
		{
		ChangePointLat();
		break;
		} // IDC_PTLAT
	case IDC_PTLON:
		{
		ChangePointLon();
		break;
		} // IDC_PTLON
	case IDC_PTELEV:
		{
		ChangePointElev();
		break;
		} // IDC_PTELEV
	case IDC_LINEWT:
		{
		DoLineWeight();
		break;
		} //
	case IDC_SCALEAMTX:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALE, WCS_INTERVEC_COMP_X, 1, NULL);
		break;
		} //
	case IDC_SCALEAMTY:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALE, WCS_INTERVEC_COMP_Y, 1, NULL);
		break;
		} //
	case IDC_SCALEAMTZ:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALE, WCS_INTERVEC_COMP_Z, 1, NULL);
		break;
		} //
	case IDC_SHIFTAMTX:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_X, ShiftAmtX.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFT, WCS_INTERVEC_COMP_X, 1, NULL);
		break;
		} //
	case IDC_SHIFTAMTY:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Y, ShiftAmtY.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFT, WCS_INTERVEC_COMP_Y, 1, NULL);
		break;
		} //
	case IDC_SHIFTAMTZ:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Z, ShiftAmtZ.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFT, WCS_INTERVEC_COMP_Z, 1, NULL);
		break;
		} //
	case IDC_ROTATEAMT:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ROTATEAMOUNT, 0, RotateAmtZ.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ROTATE, 0, 1, NULL);
		break;
		} //
	case IDC_ARBLAT:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Y, ArbLat.CurValue, NULL);
		break;
		} //
	case IDC_ARBLON:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_X, ArbLon.CurValue, NULL);
		break;
		} //
	case IDC_ARBELEV:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Z, ArbElev.CurValue, NULL);
		break;
		} //
	case IDC_PROJLAT:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Y, ProjLat.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} //
	case IDC_PROJLON:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_X, ProjLon.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} //
	case IDC_PROJELEV:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Z, ProjElev.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} //
	default:
		break;
	} // ID

return(0);

} // VectorEditGUI::HandleFIChange

/*===========================================================================*/

void VectorEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
if (Changed = DBHost->MatchNotifyClass(Interested, Changes, 1))
	{
	if (Active = DBHost->GetActiveObj())
		{
		if (Active->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			Active = NULL;
			} // else
		} // if
	NewActive();
	ConfigureWidgets();
	} // if
Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_DRAWATTRIB, 0xff);
Interested[2] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_FLAGS, 0xff);
Interested[3] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_NAME, 0xff);
Interested[4] = NULL;
if (Changed = DBHost->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	} // if

Interested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, 0xff, 0xff);
Interested[1] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, 0xff, 0xff);
Interested[2] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_BACKUP, 0xff, 0xff);
Interested[3] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[4] = NULL;
if (Changed = InterStash->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	} // if

if (Active)
	{
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		ConfigureCoords();
		} // if
	} // if

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_MODULE, 0xff, 0xff, 0xff);
if (Changed = ProjHost->MatchNotifyClass(Interested, Changes, 1))
	{
	switch((Changed & 0x0000ff00) >> 8)
		{
		case WCS_ITEM_MODULE_DIGITIZEGUI:
			{
			if (((Changed & 0x00ff0000) >> 16) ==  WCS_SUBCLASS_MODULE_OPEN)
				{
				DigInfo = (DigitizeGUI *)Activity->ChangeNotify->NotifyData;
				if (DigInfo)
					{
					if (IamResponsible)
						{
						IamResponsible = 0;
						InitDigitize();
						} // if
					} // if
				} // if
			else if (((Changed & 0x00ff0000) >> 16) ==  WCS_SUBCLASS_MODULE_CLOSE)
				{
				DigInfo = NULL;
				}
			break;
			} // WCS_ITEM_MODULE_DIGITIZEGUI
		} // switch
	} // if open or close digitize window
Interested[0] = MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// was it from our VectorColor?
	if (Activity->ChangeNotify->NotifyData == &VectorColor)
		{
		// color of vector is being changed
		Red = (short)WCS_round(VectorColor.CurValue[0] * 255.0);
		Green = (short)WCS_round(VectorColor.CurValue[1] * 255.0);
		Blue = (short)WCS_round(VectorColor.CurValue[2] * 255.0);
		DoRGBTwiddle();
		} // if
	} // if

} // VectorEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void VectorEditGUI::ConfigureWidgets(void)
{
double PlanetRad;
const char *TempName;
float TempFloatMax = 0.0f, TempFloatMin = 0.0f;
char Str[24], Label[256];

PlanetRad = EffectsHost->GetPlanetRadius();
if (InterStash->GetHorUnits() != WCS_VECTOR_HUNITS_DEGREES)
	{
	PtX.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	PtY.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	VecWidth.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	VecHeight.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	PtX.SetIncrement(1.0);
	PtY.SetIncrement(1.0);
	VecWidth.SetIncrement(1.0);
	VecHeight.SetIncrement(1.0);
	} // if
else
	{
	if (InterStash->GetPtRelative())
		{
		PtX.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
		PtY.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
		} // if
	else
		{
		PtX.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
		PtY.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
		} // else
	VecWidth.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	VecHeight.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	PtX.SetIncrement(.0001);
	PtY.SetIncrement(.0001);
	VecWidth.SetIncrement(.0001);
	VecHeight.SetIncrement(.0001);
	} // else

InterStash->GetParam(&ArbLat.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Y));
InterStash->GetParam(&ArbLon.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_X));
InterStash->GetParam(&ArbElev.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Z));
InterStash->GetParam(&ProjLat.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Y));
InterStash->GetParam(&ProjLon.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_X));
InterStash->GetParam(&ProjElev.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Z));

InterStash->GetParam(&ShiftAmtX.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_X));
InterStash->GetParam(&ShiftAmtY.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Y));
InterStash->GetParam(&ShiftAmtZ.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Z));
InterStash->GetParam(&RotateAmtZ.CurValue, MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ROTATEAMOUNT, 0));

if (Active)
	{
// main window
	Enabled = Active->TestFlags(WCS_JOEFLAG_ACTIVATED) ? 1: 0;
	DrawEnabled = Active->TestDrawFlags() ? 1: 0;
	RenderEnabled = Active->TestRenderFlags() ? 1: 0;
	LineWeight = Active->GetLineWidth();
	Red = Active->Red();
	Green = Active->Green();
	Blue = Active->Blue();
	LineStyle = Active->GetLineStyle();
	Class = GetClass(Active);
	Label[0] = NULL;
	if (TempName = Active->GetBestName())
		strcpy(Label, TempName);
	WidgetSetText(IDC_NAMETEXT, Label);

	sprintf(Str, "%d", Active->GetNumRealPoints());
	WidgetSetText(IDC_TOTALNUMPOINTS, Str);
	sprintf(Str, "%d", InterStash->GetNumSelectedPoints());
	WidgetSetText(IDC_SELECTEDNUMPTS, Str);

	VecNorth.SetValue(Active->GetNorth());
	VecSouth.SetValue(Active->GetSouth());
	VecWest.SetValue(Active->GetWest());
	VecEast.SetValue(Active->GetEast());
	if (VecNorth.CurValue < VecSouth.CurValue || VecWest.CurValue < VecEast.CurValue)
		VecNorth.CurValue = VecSouth.CurValue = VecWest.CurValue = VecEast.CurValue = 0.0;
	Active->GetElevRange(TempFloatMax, TempFloatMin);
	MaxEl.CurValue = TempFloatMax;
	MinEl.CurValue = TempFloatMin;
	OldMaxEl = MaxEl.CurValue;
	OldMinEl = MinEl.CurValue;
	//InterStash->ConvertElev(MaxEl.CurValue);
	//InterStash->ConvertElev(MinEl.CurValue);
	VecWidth.CurValue = VecWest.CurValue - VecEast.CurValue;
	OldWidth = VecWidth.CurValue;
	InterStash->ConvertDegToDist(PlanetRad, (VecNorth.CurValue + VecSouth.CurValue) * 0.5, VecWidth.CurValue);
	VecHeight.CurValue = VecNorth.CurValue - VecSouth.CurValue;
	OldHeight = VecHeight.CurValue;
	InterStash->ConvertDegToDist(PlanetRad, VecHeight.CurValue);
	InterStash->GetActivePointCoords(PtX.CurValue, PtY.CurValue, TempFloatMin);
	InterStash->ConvertPointToXYZ(PlanetRad, PtX.CurValue, PtY.CurValue, TempFloatMin, InterStash->GetPtRelative());
	PtZ.CurValue = TempFloatMin;

	WidgetSNConfig(IDC_VECNORTH, &VecNorth);
	WidgetSNConfig(IDC_VECSOUTH, &VecSouth);
	WidgetSNConfig(IDC_VECWEST, &VecWest);
	WidgetSNConfig(IDC_VECEAST, &VecEast);
	WidgetSNConfig(IDC_MAXEL, &MaxEl);
	WidgetSNConfig(IDC_MINEL, &MinEl);
	WidgetSNConfig(IDC_PTELEV, &PtZ);
	WidgetSNConfig(IDC_VECHEIGHT, &VecHeight);
	WidgetSNConfig(IDC_VECWIDTH, &VecWidth);
	WidgetSNConfig(IDC_PTLON, &PtX);
	WidgetSNConfig(IDC_PTLAT, &PtY);

/*
	ConfigureFI(NativeWin, IDC_VECHEIGHT,
	 &VecHeight,
	  .01,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_VECWIDTH,
	 &VecWidth,
	  .01,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_VECNORTH,
	 &VecNorth,
	  .01,
	   -90.0,
	    90.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_VECSOUTH,
	 &VecSouth,
	  .01,
	   -90.0,
	    90.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_VECWEST,
	 &VecWest,
	  .01,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_VECEAST,
	 &VecEast,
	  .01,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_MAXEL,
	 &MaxEl,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Float,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_MINEL,
	 &MinEl,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Float,
	      (SetCritter *)NULL,
	       NULL);
*/
	ConfigureFI(NativeWin, IDC_FIRSTPOINT,
	 NULL,
	  1.0,
	   0.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_FIRSTRANGEPT, 0));

	ConfigureFI(NativeWin, IDC_LASTPOINT,
	 NULL,
	  1.0,
	   0.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_LASTRANGEPT, 0));
/*
	ConfigureFI(NativeWin, IDC_PTLON,
	 &PtX,
	  .001,
	   -1000000,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_PTLAT,
	 &PtY,
	  .001,
	   -1000000,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)NULL,
	       NULL);

	ConfigureFI(NativeWin, IDC_PTELEV,
	 &PtZ,
	  1.0,
	   -1000000,
	    1000000.0,
	     FIOFlag_Float,
	      (SetCritter *)NULL,
	       NULL);
*/
	ConfigureSC(NativeWin, IDC_TOPOCONFORM, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_TOPOCONFORM, 0));
	ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIOABSOLUTE, NULL, SRFlag_Short, WCS_VECTOR_DISPLAYMODE_ABSOLUTE, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTRELATIVE, 0));
	ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVE, NULL, SRFlag_Short, WCS_VECTOR_DISPLAYMODE_RELATIVE, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTRELATIVE, 0));

	ConfigureSR(NativeWin, IDC_RADIOALLPOINTS, IDC_RADIOALLPOINTS, NULL, SRFlag_Short, WCS_VECTOR_PTOPERATE_ALLPTS, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0));
	ConfigureSR(NativeWin, IDC_RADIOALLPOINTS, IDC_RADIOMAPSELECTED, NULL, SRFlag_Short, WCS_VECTOR_PTOPERATE_MAPSELECTED, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0));
	ConfigureSR(NativeWin, IDC_RADIOALLPOINTS, IDC_RADIOSINGLEPT, NULL, SRFlag_Short, WCS_VECTOR_PTOPERATE_SINGLEPT, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0));
	ConfigureSR(NativeWin, IDC_RADIOALLPOINTS, IDC_RADIORANGEPT, NULL, SRFlag_Short, WCS_VECTOR_PTOPERATE_PTRANGE, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0));

	WidgetCBSetCurSel(IDC_HORUNITSDROP, InterStash->GetHorUnits() > 0 ? 1: 0);

// Properties Panel
	ConfigureFI(NativeWin, IDC_LINEWT,
	 &LineWeight,
	  1.0,
	   0.0,
	    50.0,
	     FIOFlag_Short,
	      (SetCritter *)NULL,
	       NULL);


	WidgetCBSetCurSel(IDC_STYLEDROP, LineStyle);
	WidgetCBSetCurSel(IDC_CLASSDROP, Class);
	WidgetSetCheck(IDC_ENABLED, Enabled);
	WidgetSetCheck(IDC_ENABLED2, DrawEnabled);
	WidgetSetCheck(IDC_ENABLED3, RenderEnabled);

	VectorColor.SetValue3(Red / 255.0, Green / 255.0, Blue / 255.0);
	WidgetSmartRAHConfig(ID_COLORPOT1, &VectorColor, NULL);

// Reference Panel
	WidgetSNConfig(IDC_ARBLAT, &ArbLat);
	WidgetSNConfig(IDC_ARBLON, &ArbLon);
	WidgetSNConfig(IDC_ARBELEV, &ArbElev);
	WidgetSNConfig(IDC_PROJLAT, &ProjLat);
	WidgetSNConfig(IDC_PROJLON, &ProjLon);
	WidgetSNConfig(IDC_PROJELEV, &ProjElev);
/*
	ConfigureFI(NativeWin, IDC_ARBLAT,
	 NULL,
	  1,
	   -90.0,
	    90.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Y));

	ConfigureFI(NativeWin, IDC_ARBLON,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_X));

	ConfigureFI(NativeWin, IDC_ARBELEV,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ARBREFCOORD, WCS_INTERVEC_COMP_Z));

	ConfigureFI(NativeWin, IDC_PROJLAT,
	 NULL,
	  1,
	   -90.0,
	    90.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Y));

	ConfigureFI(NativeWin, IDC_PROJLON,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_X));

	ConfigureFI(NativeWin, IDC_PROJELEV,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Z));
*/
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOGEOCOORD, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_GEOCOORD, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOPROJECT, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_PROJCOORD, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOARBITRARY, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_ARBCOORD, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOVECORIGIN, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_VECORIGIN, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOVECCENTER, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_VECCENTER, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOACTIVEPT, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_ACTIVEPT, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));
	ConfigureSR(NativeWin, IDC_RADIOGEOCOORD, IDC_RADIOSELPTCTR, NULL, SRFlag_Short, WCS_VECTOR_REFERENCE_SELPTAVG, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_REFCONTROL, 0));

// Scale Panel
	WidgetSNConfig(IDC_SHIFTAMTX, &ShiftAmtX);
	WidgetSNConfig(IDC_SHIFTAMTY, &ShiftAmtY);
	WidgetSNConfig(IDC_SHIFTAMTZ, &ShiftAmtZ);
	WidgetSNConfig(IDC_ROTATEAMT, &RotateAmtZ);

	ConfigureFI(NativeWin, IDC_SCALEAMTX,
	 NULL,
	  1.0,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALEAMOUNT, WCS_INTERVEC_COMP_X));

	ConfigureFI(NativeWin, IDC_SCALEAMTY,
	 NULL,
	  1.0,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALEAMOUNT, WCS_INTERVEC_COMP_Y));

	ConfigureFI(NativeWin, IDC_SCALEAMTZ,
	 NULL,
	  1.0,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALEAMOUNT, WCS_INTERVEC_COMP_Z));
/*
	ConfigureFI(NativeWin, IDC_SHIFTAMTX,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_X));

	ConfigureFI(NativeWin, IDC_SHIFTAMTY,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Y));

	ConfigureFI(NativeWin, IDC_SHIFTAMTZ,
	 NULL,
	  1,
	   -1000000.0,
	    1000000.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFTAMOUNT, WCS_INTERVEC_COMP_Z));

	ConfigureFI(NativeWin, IDC_ROTATEAMT,
	 NULL,
	  1,
	   -360.0,
	    360.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ROTATEAMOUNT, 0));
*/
	WidgetSetCheck(IDC_SCALEX, InterStash->GetScale(WCS_INTERVEC_COMP_X));
	WidgetSetCheck(IDC_SCALEY, InterStash->GetScale(WCS_INTERVEC_COMP_Y));
	WidgetSetCheck(IDC_SCALEZ, InterStash->GetScale(WCS_INTERVEC_COMP_Z));
	WidgetSetCheck(IDC_SHIFTX, InterStash->GetShift(WCS_INTERVEC_COMP_X));
	WidgetSetCheck(IDC_SHIFTY, InterStash->GetShift(WCS_INTERVEC_COMP_Y));
	WidgetSetCheck(IDC_SHIFTZ, InterStash->GetShift(WCS_INTERVEC_COMP_Z));
	WidgetSetCheck(IDC_ROTATE, InterStash->GetRotate());
	ConfigureSC(NativeWin, IDC_PRESERVEXY, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PRESERVEXY, 0));

// Points Panel
	#ifdef WCS_JOE_LABELPOINTEXISTS
	ConfigureFI(NativeWin, IDC_NUMPOINTS,
	 NULL,
	  1.0,
	   2.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INTERPPTS, 0));

	ConfigureFI(NativeWin, IDC_INSERTPOINTS,
	 NULL,
	  1.0,
	   1.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTPTS, 0));

	ConfigureFI(NativeWin, IDC_INSERTAFTER,
	 NULL,
	  1.0,
	   1.0,
	    WCS_DXF_MAX_OBJPTS - 2.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTAFTER, 0));

	ConfigureFI(NativeWin, IDC_ORIGINVERTEX,
	 NULL,
	  1.0,
	   1.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ORIGINVERTEX, 0));

	#else // WCS_JOE_LABELPOINTEXISTS

	ConfigureFI(NativeWin, IDC_NUMPOINTS,
	 NULL,
	  1.0,
	   1.0,
	    WCS_DXF_MAX_OBJPTS,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INTERPPTS, 0));

	ConfigureFI(NativeWin, IDC_INSERTPOINTS,
	 NULL,
	  1.0,
	   1.0,
	    WCS_DXF_MAX_OBJPTS,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTPTS, 0));

	ConfigureFI(NativeWin, IDC_INSERTAFTER,
	 NULL,
	  1.0,
	   0.0,
	    WCS_DXF_MAX_OBJPTS - 1.0,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTAFTER, 0));

	ConfigureFI(NativeWin, IDC_ORIGINVERTEX,
	 NULL,
	  1.0,
	   0.0,
	    WCS_DXF_MAX_OBJPTS,
	     FIOFlag_Long,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ORIGINVERTEX, 0));
	#endif // WCS_JOE_LABELPOINTEXISTS


	ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOLINEAR, NULL, SRFlag_Short, WCS_VECTOR_INSERT_LINEAR, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTSPLINE, 0));
	ConfigureSR(NativeWin, IDC_RADIOLINEAR, IDC_RADIOSPLINE, NULL, SRFlag_Short, WCS_VECTOR_INSERT_SPLINE, (SetCritter *)InterStash, 
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_INSERTSPLINE, 0));

// Smooth Panel
	ConfigureFI(NativeWin, IDC_SMOOTHING,
	 NULL,
	  1.0,
	   0.0,
	    100.0,
	     FIOFlag_Double,
	      (SetCritter *)InterStash,
	       MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SMOOTHING, 0));

	ConfigureSC(NativeWin, IDC_SMOOTHX, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SMOOTH, WCS_INTERVEC_COMP_X));
	ConfigureSC(NativeWin, IDC_SMOOTHY, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SMOOTH, WCS_INTERVEC_COMP_Y));
	ConfigureSC(NativeWin, IDC_SMOOTHZ, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SMOOTH, WCS_INTERVEC_COMP_Z));
	ConfigureSC(NativeWin, IDC_CONSIDERELEV, NULL, SCFlag_Short, (SetCritter *)InterStash,
		MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_CONSIDERELEV, 0));

	DisableWidgets();
	} // if

ConfigureCoords();

} // VectorEditGUI::ConfigureWidgets()

/*===========================================================================*/

void VectorEditGUI::ConfigureCoords(void)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
GeneralEffect *MyEffect;
long CurPos, Pos;

WidgetCBClear(IDC_COORDSDROP);
if (Active)
	{
	CurPos = -1;
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MyCoords)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	} // if
#endif // WCS_BUILD_VNS

} // VectorEditGUI::ConfigureCoords

/*===========================================================================*/

void VectorEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOABSOLUTE, InterStash->GetHorUnits() != WCS_VECTOR_HUNITS_DEGREES);
WidgetSetDisabled(IDC_FIRSTPOINT, (InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_SINGLEPT && 
	InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_PTRANGE));
WidgetSetDisabled(IDC_LASTPOINT, InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_PTRANGE);
WidgetSetDisabled(IDC_NUMPOINTS, (InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_ALLPTS));
WidgetSetDisabled(IDC_CHANGENUMPOINTS, (InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_ALLPTS));
WidgetSetDisabled(IDC_SCALEY, (char)InterStash->GetPreserveXY());
WidgetSetDisabled(IDC_SCALEAMTY, (char)InterStash->GetPreserveXY());
WidgetSetDisabled(IDC_SMOOTH, (InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_ALLPTS));
WidgetSetDisabled(IDC_SPACING, (InterStash->GetPointOperate() != WCS_VECTOR_PTOPERATE_ALLPTS));
 
} // VectorEditGUI::DisableWidgets

/*===========================================================================*/

short VectorEditGUI::GetClass(Joe *Me)
{
// 0=Topo, 1=Surface, 2=DryLand, 3=Vector, 4=Illum Vec, 5=Seg Vec, 6=Illum Seg Vec
// V6/VNS: 0=Vector, 1=Control Points

if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
	{
	return(0); // Plain ol' DEM -- no rational value to return
	} // if
if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL)) return(1);

// All else aside, it's a vector
return(0);

} // VectorEditGUI::GetClass()

/*===========================================================================*/

void VectorEditGUI::SetScale(short Component, unsigned short WidID)
{

InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SCALE, Component,
	WidgetGetCheck(WidID), NULL);

} // VectorEditGUI::SetScale

/*===========================================================================*/

void VectorEditGUI::SetShift(short Component, unsigned short WidID)
{

InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_SHIFT, Component,
	WidgetGetCheck(WidID), NULL);

} // VectorEditGUI::SetShift

/*===========================================================================*/

void VectorEditGUI::SetRotate(unsigned short WidID)
{

InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_ROTATE, 0,
	WidgetGetCheck(WidID), NULL);

} // VectorEditGUI::SetRotate

/*===========================================================================*/

void VectorEditGUI::ChangeNorth(void)
{

if (Active)
	{
	SignalPreChangePoints();
	Active->StretchNorth(VecNorth.CurValue);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeNorth

/*===========================================================================*/

void VectorEditGUI::ChangeSouth(void)
{

if (Active)
	{
	SignalPreChangePoints();
	Active->StretchSouth(VecSouth.CurValue);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeSouth

/*===========================================================================*/

void VectorEditGUI::ChangeWest(void)
{

if (Active)
	{
	SignalPreChangePoints();
	Active->StretchWest(VecWest.CurValue);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeWest

/*===========================================================================*/

void VectorEditGUI::ChangeEast(void)
{

if (Active)
	{
	SignalPreChangePoints();
	Active->StretchEast(VecEast.CurValue);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeEast

/*===========================================================================*/

void VectorEditGUI::ChangeLowElev(void)
{
//float RealMaxElev, RealMinElev;

if (Active)
	{
	//RealMaxElev = MaxEl.CurValue;
	//RealMinElev = MinEl.CurValue;
	//InterStash->UnConvertElev(RealMaxElev);
	//InterStash->UnConvertElev(RealMinElev);
	SignalPreChangePoints();
	Active->StretchLowElev((float)MaxEl.CurValue, (float)MinEl.CurValue, (float)OldMinEl);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeLowElev

/*===========================================================================*/

void VectorEditGUI::ChangeHighElev(void)
{
//float RealMaxElev, RealMinElev;

if (Active)
	{
	//RealMaxElev = MaxEl.CurValue;
	//RealMinElev = MinEl.CurValue;
	//InterStash->UnConvertElev(RealMaxElev);
	//InterStash->UnConvertElev(RealMinElev);
	SignalPreChangePoints();
	Active->StretchHighElev((float)MaxEl.CurValue, (float)MinEl.CurValue, (float)OldMaxEl);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeHighElev

/*===========================================================================*/

void VectorEditGUI::SignalNewPoints(short Conform)
{
NotifyTag Changes[2];

Active->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
Active->RecheckBounds();
Active->ZeroUpTree();
DBHost->ReBoundTree(WCS_DATABASE_STATIC);
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, Conform);
Changes[1] = NULL;
DBHost->GenerateNotify(Changes);
Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes);

} // VectorEditGUI::SignalNewPoints

/*===========================================================================*/

void VectorEditGUI::SignalPreChangePoints(void)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0);
Changes[1] = NULL;
DBHost->GenerateNotify(Changes);

} // VectorEditGUI::SignalPreChangePoints

/*===========================================================================*/

void VectorEditGUI::UndoPoints(void)
{

if (Active)
	{
	SignalPreChangePoints();
	InterStash->UndoPoints(Active);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	if (InterStash->GetPointOperate() == WCS_VECTOR_PTOPERATE_ALLPTS)
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::UndoPoints

/*===========================================================================*/

void VectorEditGUI::RestorePoints(void)
{

if (Active)
	{
	SignalPreChangePoints();
	InterStash->RestorePoints(Active);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	if (InterStash->GetPointOperate() == WCS_VECTOR_PTOPERATE_ALLPTS)
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0, WCS_VECTOR_PTOPERATE_ALLPTS, 0);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::RestorePoints

/*===========================================================================*/

void VectorEditGUI::SetHorUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_HORUNITSDROP);
InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_HORUNITS, 0, Current, 0);
WidgetCBSetCurSel(IDC_HORUNITSDROP, InterStash->GetHorUnits());

} // VectorEditGUI::SetHorUnits

/*===========================================================================*/

void VectorEditGUI::ChangePointLat(void)
{
double Lat, Lon, PlanetRad;
float Elev;
JoeCoordSys *MyAttr;
VertexDEM Vert;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	Lat = PtY.CurValue;
	Lon = PtX.CurValue;
	Elev = (float)PtZ.CurValue;
	// returns values in projected form
	InterStash->ConvertPointFromXYZ(PlanetRad, Lon, Lat, Elev, InterStash->GetPtRelative());
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		if (MyAttr->Coord)
			{
			Vert.xyz[0] = Lon;
			Vert.xyz[1] = Lat;
			Vert.xyz[2] = Elev;
			MyAttr->Coord->ProjToDefDeg(&Vert);
			Lat = Vert.Lat;
			} // if
		} // if
	SignalPreChangePoints();
	InterStash->ChangePointLat(Lat);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangePointLat

/*===========================================================================*/

void VectorEditGUI::ChangePointLon(void)
{
double Lat, Lon, PlanetRad;
float Elev;
JoeCoordSys *MyAttr;
VertexDEM Vert;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	Lat = PtY.CurValue;
	Lon = PtX.CurValue;
	Elev = (float)PtZ.CurValue;
	// returns values in projected form
	InterStash->ConvertPointFromXYZ(PlanetRad, Lon, Lat, Elev, InterStash->GetPtRelative());
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		if (MyAttr->Coord)
			{
			Vert.xyz[0] = Lon;
			Vert.xyz[1] = Lat;
			Vert.xyz[2] = Elev;
			MyAttr->Coord->ProjToDefDeg(&Vert);
			Lon = Vert.Lon;
			} // if
		} // if
	SignalPreChangePoints();
	InterStash->ChangePointLon(Lon);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangePointLon

/*===========================================================================*/

void VectorEditGUI::ChangePointElev(void)
{
double Lat, Lon, PlanetRad;
float Elev;
JoeCoordSys *MyAttr;
VertexDEM Vert;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	Lat = PtY.CurValue;
	Lon = PtX.CurValue;
	Elev = (float)PtZ.CurValue;
	// returns values in projected form
	InterStash->ConvertPointFromXYZ(PlanetRad, Lon, Lat, Elev, InterStash->GetPtRelative());
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		if (MyAttr->Coord)
			{
			Vert.xyz[0] = Lon;
			Vert.xyz[1] = Lat;
			Vert.xyz[2] = Elev;
			MyAttr->Coord->ProjToDefDeg(&Vert);
			Elev = (float)Vert.Elev;
			} // if
		} // if
	SignalPreChangePoints();
	InterStash->ChangePointElev(Elev);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangePointElev

/*===========================================================================*/

void VectorEditGUI::ChangeWidth(void)
{
double NewWidth, PlanetRad;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	NewWidth = VecWidth.CurValue;
	InterStash->ConvertDistToDeg(PlanetRad, (VecNorth.CurValue + VecSouth.CurValue) * 0.5, NewWidth);
	SignalPreChangePoints();
	Active->StretchWidth(InterStash->GetRefCoords(WCS_INTERVEC_COMP_X), NewWidth, OldWidth);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeWidth

/*===========================================================================*/

void VectorEditGUI::ChangeHeight(void)
{
double NewHeight, PlanetRad;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	NewHeight = VecHeight.CurValue;
	InterStash->ConvertDistToDeg(PlanetRad, NewHeight);
	SignalPreChangePoints();
	Active->StretchHeight(InterStash->GetRefCoords(WCS_INTERVEC_COMP_Y), NewHeight, OldHeight);
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ChangeHeight

/*===========================================================================*/

void VectorEditGUI::ScaleRotateShift(void)
{
double LatShift, LonShift, PlanetRad;
float ElevShift;
short TopoConform = WCS_NOTIFYDBASECHANGE_POINTS_CONFORM;

if (Active)
	{
	PlanetRad = EffectsHost->GetPlanetRadius();
	SignalPreChangePoints();
	LonShift = InterStash->GetShiftAmt(WCS_INTERVEC_COMP_X);
	InterStash->ConvertDistToDegNoOptions(PlanetRad, InterStash->GetRefCoords(WCS_INTERVEC_COMP_Y), LonShift);
	LatShift = InterStash->GetShiftAmt(WCS_INTERVEC_COMP_Y);
	InterStash->ConvertDistToDegNoOptions(PlanetRad, LatShift);
	ElevShift = (float)InterStash->GetShiftAmt(WCS_INTERVEC_COMP_Z);
	//InterStash->UnConvertElev(ElevShift);
	InterStash->ScaleRotateShift(PlanetRad, InterStash->GetScaleAmt(WCS_INTERVEC_COMP_X),
		InterStash->GetScaleAmt(WCS_INTERVEC_COMP_Y),
		InterStash->GetScaleAmt(WCS_INTERVEC_COMP_Z),
		InterStash->GetRotateAmt(),
		LonShift, LatShift, ElevShift,
		TRUE);

	if (InterStash->GetShift(WCS_INTERVEC_COMP_Z) || InterStash->GetScale(WCS_INTERVEC_COMP_Z))
		{
		TopoConform = WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM;
		} // if

	SignalNewPoints(TopoConform);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ScaleShiftRotate

/*===========================================================================*/

void VectorEditGUI::InitDigitize(void)
{

if (DigInfo)
	{
	DigInfo->AddInfo.DigObjectType = WCS_DIGITIZE_DIGOBJTYPE_VECTOR;
	DigInfo->AddInfo.Append = 0;
	if (! DigInfo->AddInfo.Mode)
		DigInfo->AddInfo.Mode = WCS_DIGITIZE_ADDPOINTS_SINGLE;
	DigInfo->AddInfo.ElevType = WCS_DIGITIZE_ELEVTYPE_VECTOR;
	DigInfo->Initialized = 1;
	} // if

} // VectorEditGUI::InitDigitize()

/*===========================================================================*/

void VectorEditGUI::CreateCopy(void)
{
Joe *OldActive, *NewJoe;
RasterAnimHost *CurHost;
NotifyTag Changes[2];
RasterAnimHostProperties TempProp;

if (Active)
	{
	OldActive = Active;
	if (NewJoe = DBHost->NewObject(ProjHost))
		{
		NewJoe->CopyPoints(NewJoe, OldActive, 1, 1);
		NewJoe->ClearFlags(NewJoe->GetFlags());
		NewJoe->SetFlags(OldActive->GetFlags() & (WCS_JOEFLAG_ISCONTROL | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED));
		// add effect attributes
		CurHost = NULL; 
		TempProp.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		while (CurHost = OldActive->GetRAHostChild(CurHost, 0))
			{
			CurHost->GetRAHostProperties(&TempProp);
			if (TempProp.TypeNumber >= WCS_JOE_ATTRIB_INTERNAL_LAKE && TempProp.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
				NewJoe->AddEffect((GeneralEffect *)CurHost, -1);
			} // while 
		// needs to be done after points added for undo functions
		Changes[0] = MAKE_ID(NewJoe->GetNotifyClass(), NewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NewJoe);
		DBHost->SetActiveObj(NewJoe);	// calls RasterAnimHost::SetActiveRAHost
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::CreateCopy()

/*===========================================================================*/

void VectorEditGUI::NoJoeMessage(void)
{

UserMessageOK("Vector Edit", "Active object is not a vector.\nSelect the object you wish to modify in the\n Database Editor or Map View and try again");

} // VectorEditGUI::NoJoeMessage


/*===========================================================================*/

void VectorEditGUI::DoLineWeight(void)
{

if (Active)
	Active->SetLineWidth((unsigned char)LineWeight);
else
	NoJoeMessage();

} // VectorEditGUI::DoLineWeight()

/*===========================================================================*/

void VectorEditGUI::DoEnable(void)
{
NotifyTag Changes[2];

Enabled = WidgetGetCheck(IDC_ENABLED);
if (Active)
	{
	if (Enabled) Active->SetFlags  (WCS_JOEFLAG_ACTIVATED);
	else       Active->ClearFlags(WCS_JOEFLAG_ACTIVATED);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::DoEnable()

/*===========================================================================*/

void VectorEditGUI::DoDrawEnable(void)
{
NotifyTag Changes[2];

DrawEnabled = WidgetGetCheck(IDC_ENABLED2);
if (Active)
	{
	if (DrawEnabled) Active->SetFlags  (WCS_JOEFLAG_DRAWENABLED);
	else       Active->ClearFlags(WCS_JOEFLAG_DRAWENABLED);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::DoDrawEnable()

/*===========================================================================*/

void VectorEditGUI::DoRenderEnable(void)
{
NotifyTag Changes[2];

RenderEnabled = WidgetGetCheck(IDC_ENABLED3);
if (Active)
	{
	if (RenderEnabled) Active->SetFlags  (WCS_JOEFLAG_RENDERENABLED);
	else       Active->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::DoRenderEnable()

/*===========================================================================*/

void VectorEditGUI::DoRGBTwiddle(void)
{

WidgetSNSync(ID_COLORPOT1, WP_FISYNC_NONOTIFY);
if (Active)
	Active->SetRGB((unsigned char)Red, (unsigned char)Green, (unsigned char)Blue);
else
	NoJoeMessage();

} // VectorEditGUI::DoRGBTwiddle()

/*===========================================================================*/

void VectorEditGUI::DoObjectClass(void)
{

Class = (short)WidgetCBGetCurSel(IDC_CLASSDROP);

if (Active)
	{
	if (Class == 1) // Control, !Illum, !Seg
		{
		Active->SetFlags(WCS_JOEFLAG_ISCONTROL);
		Active->ClearFlags(WCS_JOEFLAG_VEC_AS_SEGMENTED);
		Active->ClearFlags(WCS_JOEFLAG_VEC_ILLUMINATED);
		} // else if
	else // Normal vector, !Control, !Illum, !Seg
		{
		Active->ClearFlags(WCS_JOEFLAG_VEC_ILLUMINATED);
		Active->ClearFlags(WCS_JOEFLAG_VEC_AS_SEGMENTED);
		Active->ClearFlags(WCS_JOEFLAG_ISCONTROL);
		} // else
	Class = GetClass(Active);
	WidgetCBSetCurSel(IDC_CLASSDROP, Class);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::DoObjectClass()

/*===========================================================================*/

void VectorEditGUI::DoLineStyle(void)
{

LineStyle = (short)WidgetCBGetCurSel(IDC_STYLEDROP);

if (Active)
	Active->SetLineStyle(LineStyle);
else
	NoJoeMessage();

} // VectorEditGUI::DoLineStyle()

/*===========================================================================*/

void VectorEditGUI::CloseOrigin(void)
{
VectorPoint *NewList, *PLinkA, *PLinkB, *LastOrigPt;

if (Active)
	{
	if (Active->GetSecondRealPoint() && Active->GetNumRealPoints() > 1)
		{
		if (NewList = DBHost->MasterPoint.Allocate(Active->NumPoints() + 1))
			{
			for (PLinkA = Active->Points(), PLinkB = NewList; PLinkA && PLinkB; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
				{
				PLinkB->Latitude = PLinkA->Latitude;
				PLinkB->Longitude = PLinkA->Longitude;
				PLinkB->Elevation = PLinkA->Elevation;
				LastOrigPt = PLinkA;
				} // for
			if (! LastOrigPt->SamePoint(Active->GetFirstRealPoint()))
				{
				PLinkB->Latitude = Active->GetFirstRealPoint()->Latitude;
				PLinkB->Longitude = Active->GetFirstRealPoint()->Longitude;
				PLinkB->Elevation = Active->GetFirstRealPoint()->Elevation;
				SignalPreChangePoints();
				DBHost->MasterPoint.DeAllocate(Active->Points());
				Active->Points(NewList);
				Active->NumPoints(Active->NumPoints() + 1);
				SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
				} // if
			else
				{
				DBHost->MasterPoint.DeAllocate(NewList);
				} // else no closure necessary, already closed
			} // if
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::CloseOrigin()

/*===========================================================================*/

void VectorEditGUI::SetOrigin(void)
{
VectorPoint *NewList, *PLinkA, *PLinkB, *PLinkPrev;
unsigned long int Ct, NewOrigin, ObjectClosed = 0;

if (Active)
	{
	if (Active->GetSecondRealPoint() && Active->GetNumRealPoints() > 1)
		{
		NewOrigin = InterStash->GetOriginVertex();
		if (NewOrigin < Active->NumPoints() && NewOrigin > Joe::GetFirstRealPtNum())
			{
			if (NewList = DBHost->MasterPoint.Allocate(Active->NumPoints()))
				{
				for (Ct = 0, PLinkA = Active->Points(); PLinkA && Ct < NewOrigin; PLinkA = PLinkA->Next, Ct ++);	//lint !e722
				for (PLinkPrev = PLinkA; PLinkPrev->Next; PLinkPrev = PLinkPrev->Next);	//lint !e722
				if (PLinkPrev && PLinkPrev->SamePoint(Active->GetFirstRealPoint()))
					ObjectClosed = 1;
				PLinkB = NewList;
				#ifdef WCS_JOE_LABELPOINTEXISTS
				PLinkB->Latitude = PLinkA->Latitude;
				PLinkB->Longitude = PLinkA->Longitude;
				PLinkB->Elevation = PLinkA->Elevation;
				PLinkB = PLinkB->Next;
				#endif // WCS_JOE_LABELPOINTEXISTS
				for (; PLinkB; PLinkB = PLinkB->Next, PLinkA = PLinkA->Next)
					{
					if (! PLinkA)
						{
						PLinkA = Active->GetFirstRealPoint();
						if (ObjectClosed)	// can only happen if PLinkPrev exists
							PLinkB = PLinkPrev;
						} // if
					PLinkB->Latitude = PLinkA->Latitude;
					PLinkB->Longitude = PLinkA->Longitude;
					PLinkB->Elevation = PLinkA->Elevation;
					PLinkPrev = PLinkB;
					} // for
				if (ObjectClosed)	// can only happen if PLinkPrev exists
					{
					#ifdef WCS_JOE_LABELPOINTEXISTS
					PLinkPrev->Latitude = NewList->Next->Latitude;
					PLinkPrev->Longitude = NewList->Next->Longitude;
					PLinkPrev->Elevation = NewList->Next->Elevation;
					#else // WCS_JOE_LABELPOINTEXISTS
					PLinkPrev->Latitude = NewList->Latitude;
					PLinkPrev->Longitude = NewList->Longitude;
					PLinkPrev->Elevation = NewList->Elevation;
					#endif // WCS_JOE_LABELPOINTEXISTS
					} // if
				SignalPreChangePoints();
				DBHost->MasterPoint.DeAllocate(Active->Points());
				Active->Points(NewList);
				SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
				} // if
			} // if
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::SetOrigin()

/*===========================================================================*/

void VectorEditGUI::ReversePoints(void)
{
VectorPoint **PointList;
VectorPoint *PLink, TempPt;
unsigned long int Ct, LastCt, NumRealPts;

if (Active)
	{
	if (Active->GetSecondRealPoint() && (NumRealPts = Active->GetNumRealPoints()) > 1)
		{
		if (PointList = (VectorPoint **)AppMem_Alloc(NumRealPts * sizeof (VectorPoint *), 0))
			{
			SignalPreChangePoints();
			for (Ct = 0, PLink = Active->GetFirstRealPoint(); Ct < NumRealPts; Ct ++, PLink = PLink->Next)
				{
				PointList[Ct] = PLink;
				} // for
			for (Ct = 0, LastCt = NumRealPts - 1; Ct < NumRealPts / 2; Ct ++, LastCt --)
				{
				TempPt.Latitude = PointList[Ct]->Latitude;
				TempPt.Longitude = PointList[Ct]->Longitude;
				TempPt.Elevation = PointList[Ct]->Elevation;
				PointList[Ct]->Latitude = PointList[LastCt]->Latitude;
				PointList[Ct]->Longitude = PointList[LastCt]->Longitude;
				PointList[Ct]->Elevation = PointList[LastCt]->Elevation;
				PointList[LastCt]->Latitude = TempPt.Latitude;
				PointList[LastCt]->Longitude = TempPt.Longitude;
				PointList[LastCt]->Elevation = TempPt.Elevation;
				} // for
			#ifdef WCS_JOE_LABELPOINTEXISTS
			Active->Points()->Latitude = Active->GetFirstRealPoint()->Latitude;
			Active->Points()->Longitude = Active->GetFirstRealPoint()->Longitude;
			Active->Points()->Elevation = Active->GetFirstRealPoint()->Elevation;
			#endif // WCS_JOE_LABELPOINTEXISTS
			AppMem_Free(PointList, NumRealPts * sizeof (VectorPoint *));
			SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
			} // if
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::ReversePoints()

/*===========================================================================*/

void VectorEditGUI::VectorToPath(void)
{

if (GlobalApp->GUIWins->PTH)
	{
	UserMessageOK("Transfer to Path", "Path Transfer window is in use. Complete the current transfer operation and try again.");
	return;
	}
GlobalApp->GUIWins->PTH = new PathTransferGUI(GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppEffects, Active, NULL);
if (GlobalApp->GUIWins->PTH)
	{
	GlobalApp->GUIWins->PTH->Open(GlobalApp->MainProj);
	}

/*
char LengthStr[80];
long TotalFrames, FlattenExag = 0;
double Elev, Exag = 1.0, Datum = 0.0, FrameRate, FrameTime, SegLengthH, SegLengthV, LengthUsed = 0.0, VecLength = 0.0;
VectorPoint *PLink;
Camera *Cam;
PlanetOpt *DefPlanetOpt;

if (Active)
	{
	if (Active->Points && Active->NumPoints > 1)
		{
		LengthStr[0] = 0;
		if (GetInputString("Enter the path length in frames.", "-.", LengthStr) && (TotalFrames = atoi(LengthStr)) > 0 && TotalFrames < 32767)
			{
			if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
				{
				Datum = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
				Exag = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
				if (Exag != 1.0)
					{
					FlattenExag = UserMessageYN("Vector Editor", "Apply the same Vertical Scaling to the new path as is applied to terrain?");
					} // if
				} // if
			FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
			if (FrameRate <= 0.0)
				FrameRate = 30.0;

			if (Cam = (Camera *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, "Camera from Vector", NULL))
				{
				Cam->SetFloating(0);
				for (PLink = Active->Points->Next; PLink->Next; PLink = PLink->Next)
					{
					SegLengthH = FindDistance(PLink->Latitude, PLink->Longitude, PLink->Next->Latitude, PLink->Next->Longitude, EARTHRAD);
					SegLengthV = PLink->Next->Elevation - PLink->Elevation;
					VecLength += sqrt(SegLengthH * SegLengthH + SegLengthV * SegLengthV);
					} // for
				if (VecLength > 0.0)
					{
					for (PLink = Active->Points->Next; PLink; PLink = PLink->Next)
						{
						FrameTime = (TotalFrames * LengthUsed / VecLength) / FrameRate;
						Elev = FlattenExag ? (PLink->Elevation - Datum) * Exag + Datum: PLink->Elevation;
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].AddNode(FrameTime, PLink->Latitude, 0.0);
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].AddNode(FrameTime, PLink->Longitude, 0.0);
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].AddNode(FrameTime, Elev, 0.0);
						if (PLink->Next)
							{
							SegLengthH = FindDistance(PLink->Latitude, PLink->Longitude, PLink->Next->Latitude, PLink->Next->Longitude, EARTHRAD);
							SegLengthV = PLink->Next->Elevation - PLink->Elevation;
							LengthUsed += sqrt(SegLengthH * SegLengthH + SegLengthV * SegLengthV);
							} // if
						} // for
					} // if
				else
					{
					FrameTime = 0.0;
					Elev = FlattenExag ? (PLink->Elevation - Datum) * Exag + Datum: PLink->Elevation;
					Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].AddNode(FrameTime, PLink->Latitude, 0.0);
					Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].AddNode(FrameTime, PLink->Longitude, 0.0);
					Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].AddNode(FrameTime, Elev, 0.0);
					} // else only create one keyframe
				} // if new camera
			} // if
		} // if
	} // if
else
	NoJoeMessage();
*/
} // VectorEditGUI::VectorToPath()

/*===========================================================================*/

void VectorEditGUI::PathToVector(void)
{

if (GlobalApp->GUIWins->PTH)
	{
	UserMessageOK("Transfer to Vector", "Path Transfer window is in use. Complete the current transfer operation and try again.");
	return;
	}
GlobalApp->GUIWins->PTH = new PathTransferGUI(GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppEffects, NULL, Active);
if (GlobalApp->GUIWins->PTH)
	{
	GlobalApp->GUIWins->PTH->Open(GlobalApp->MainProj);
	}


/*
long i, KeyFrames, LastKey, NumKeys;
union KeyFrame *KF;
VectorPoint *NewList, *PLink;
*/
//if (Active)
//	{
//	GlobalApp->AppEffects->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_CAMERA);
	/*
	KF = ParamHost->GetKeyFrameAddress();
	NumKeys = ParamHost->GetNumKeys();
	LastKey = -1;
	for (i=KeyFrames=0; i<NumKeys; i++)
		{
		if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == WCS_KEYFRAME_GROUP_MOTION && 
			(KF[i].MoKey.Item == WCS_MOTION_CAMLAT || KF[i].MoKey.Item == WCS_MOTION_CAMLON || KF[i].MoKey.Item == WCS_MOTION_CAMALT)))
			{
			LastKey = KF[i].MoKey.KeyFrame;
			KeyFrames ++;
			} // if 
		} // for i=0... 
	if (KeyFrames > 0)
		{
		if (ParamHost->BuildKeyTable())
			{
			if (NewList = DBHost->MasterPoint.Allocate(KeyFrames + 1))
				{
				PLink = NewList->Next;
				LastKey = -1;
				for (i = 0; i < NumKeys && PLink; i ++)
					{
					if (KF[i].MoKey.KeyFrame != LastKey && (KF[i].MoKey.Group == WCS_KEYFRAME_GROUP_MOTION && 
						(KF[i].MoKey.Item == WCS_MOTION_CAMLAT || KF[i].MoKey.Item == WCS_MOTION_CAMLON || KF[i].MoKey.Item == WCS_MOTION_CAMALT)))
						{
						LastKey = KF[i].MoKey.KeyFrame;
						if (ParamHost->GetKeyTableItem(WCS_MOTION_CAMLAT))
							PLink->Latitude = ParamHost->GetKeyTableValue(WCS_MOTION_CAMLAT, LastKey, 0);
						else
							PLink->Latitude = ParamHost->GetMoPar(WCS_MOTION_CAMLAT);
						if (ParamHost->GetKeyTableItem(WCS_MOTION_CAMLON))
							PLink->Longitude = ParamHost->GetKeyTableValue(WCS_MOTION_CAMLON, LastKey, 0);
						else
							PLink->Longitude = ParamHost->GetMoPar(WCS_MOTION_CAMLON);
						if (ParamHost->GetKeyTableItem(WCS_MOTION_CAMALT))
							PLink->Elevation = (float)(1000.0 * ParamHost->GetKeyTableValue(WCS_MOTION_CAMALT, LastKey, 0));
						else
							PLink->Elevation = (float)(1000.0 * ParamHost->GetMoPar(WCS_MOTION_CAMALT));
						PLink = PLink->Next;
						} // if 
					} // for i=0... 
				NewList->Latitude = NewList->Next->Latitude;
				NewList->Longitude = NewList->Next->Longitude;
				NewList->Elevation = NewList->Next->Elevation;
				SignalPreChangePoints();
				if (Active->Points)
					{
					DBHost->MasterPoint.DeAllocate(Active->Points);
					} // if
				Active->Points = NewList;
				Active->NumPoints = KeyFrames + 1;
				SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
				} // if
			} // if key table
		} // if
	else
		UserMessageOK("Vector Editor", "There are no Key Frames to convert.");
	*/
//	} // if
//else
//	NoJoeMessage();

} // VectorEditGUI::PathToVector()

/*===========================================================================*/

void VectorEditGUI::DeletePoints(void)
{

if (Active)
	{
	SignalPreChangePoints();
	InterStash->DeleteSelectedPoints();
	SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::DeletePoints()

/*===========================================================================*/

void VectorEditGUI::InterpolatePoints(void)
{
VectorPoint *NewList;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
short Geographic;

if (Active)
	{
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		{
		MyCoords = MyAttr->Coord;
		Geographic = MyCoords->GetGeographic();
		} // if
	else
		Geographic = 1;
	if (NewList = Interpolate(Active->Points(), Active->NumPoints(), InterStash->GetInterpPts(), FALSE, TRUE, !InterStash->GetInsertSpline(), Geographic))
		{
		SignalPreChangePoints();
		DBHost->MasterPoint.DeAllocate(Active->Points());
		Active->Points(NewList);
		Active->NumPoints(InterStash->GetInterpPts() + Joe::GetFirstRealPtNum());		// + 1 for label point
		SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0,
			WCS_VECTOR_PTOPERATE_ALLPTS, NULL);
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::InterpolatePoints

/*===========================================================================*/

void VectorEditGUI::SmoothPoints(void)
{
unsigned long int FirstPassNumPts;
VectorPoint *NewList1, *NewList2, *PLinkA, *PLinkB;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
short Geographic;

if (Active)
	{
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		{
		MyCoords = MyAttr->Coord;
		Geographic = MyCoords->GetGeographic();
		} // if
	else
		Geographic = 1;
	if (Active->Points() && Active->GetNumRealPoints() > 2 && InterStash->GetSmoothing() > 0.0)
		{
		FirstPassNumPts = 2 + Joe::GetFirstRealPtNum() + (Active->GetNumRealPoints() - 2) - (unsigned long int)((Active->GetNumRealPoints() - 2) * InterStash->GetSmoothing() / 100.0);
		if (NewList1 = Interpolate(Active->Points(), Active->NumPoints(), FirstPassNumPts - Joe::GetFirstRealPtNum(), FALSE, TRUE, FALSE, Geographic))
			{
			if (NewList2 = Interpolate(NewList1, FirstPassNumPts, Active->NumPoints() - Joe::GetFirstRealPtNum(), FALSE, TRUE, FALSE, Geographic))
				{
				DBHost->MasterPoint.DeAllocate(NewList1);
				SignalPreChangePoints();
				if (! InterStash->GetSmooth(WCS_INTERVEC_COMP_X) || ! InterStash->GetSmooth(WCS_INTERVEC_COMP_Y) || ! InterStash->GetSmooth(WCS_INTERVEC_COMP_Z))
					{
					for (PLinkA = NewList2, PLinkB = Active->Points(); PLinkA && PLinkB; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
						{
						if (InterStash->GetSmooth(WCS_INTERVEC_COMP_X))
							{
							PLinkB->Longitude = PLinkA->Longitude;
							} // if
						if (InterStash->GetSmooth(WCS_INTERVEC_COMP_Y))
							{
							PLinkB->Latitude = PLinkA->Latitude;
							} // if
						if (InterStash->GetSmooth(WCS_INTERVEC_COMP_Z))
							{
							PLinkB->Elevation = PLinkA->Elevation;
							} // if
						} // for
					} // if not all components
				else
					{
					DBHost->MasterPoint.DeAllocate(Active->Points());
					Active->Points(NewList2);
					} // else all components
				// no change in number of points
				SignalNewPoints((short)(! InterStash->GetSmooth(WCS_INTERVEC_COMP_Z)));
				InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0,
					WCS_VECTOR_PTOPERATE_ALLPTS, NULL);
				} // if
			} // if
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::SmoothPoints

/*===========================================================================*/

void VectorEditGUI::EvenSpacing(void)
{
VectorPoint *NewList;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
short Geographic;

if (Active)
	{
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		{
		MyCoords = MyAttr->Coord;
		Geographic = MyCoords->GetGeographic();
		} // if
	else
		Geographic = 1;
	if (NewList = Interpolate(Active->Points(), Active->NumPoints(), Active->NumPoints() - Joe::GetFirstRealPtNum(), TRUE, FALSE, InterStash->GetConsiderElev(), Geographic))
		{
		SignalPreChangePoints();
		DBHost->MasterPoint.DeAllocate(Active->Points());
		Active->Points(NewList);
		// no change in number of points
		SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PTOPERATE, 0,
			WCS_VECTOR_PTOPERATE_ALLPTS, NULL);
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::EvenSpacing

/*===========================================================================*/

void VectorEditGUI::InsertPoints(void)
{
unsigned long int InsertPoint, InsertNumPts, InsertAfter, NumPts, Pt;
VectorPoint *NewList, *InsertList, *RootPoint, *PLinkA, *PLinkB, *PLinkC;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
short Geographic;

if (Active)
	{
	if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		{
		MyCoords = MyAttr->Coord;
		Geographic = MyCoords->GetGeographic();
		} // if
	else
		Geographic = 1;
	if (Active->GetSecondRealPoint() && Active->GetNumRealPoints() > 1)
		{
		InsertPoint = InterStash->GetInsertAfter();
		if (InsertPoint >= Joe::GetFirstRealPtNum() && InsertPoint < Active->NumPoints() - 1)
			{
			InsertNumPts = InterStash->GetInsertPts();
			if (Active->NumPoints() + InsertNumPts > WCS_DXF_MAX_OBJPTS)
				InsertNumPts = WCS_DXF_MAX_OBJPTS - Active->NumPoints();
			if (InsertNumPts > 0)
				{
				if (InsertPoint == Joe::GetFirstRealPtNum())
					{
					RootPoint = Active->GetFirstRealPoint();
					InsertAfter = 0;
					NumPts = 3;
					} // if
				else
					{
					for (Pt = 0, PLinkA = Active->Points(); Pt < InsertPoint && PLinkA; Pt ++, PLinkA = PLinkA->Next)
						{
						RootPoint = PLinkA;
						} // for
					InsertAfter = 1;
					NumPts = 4;
					} // else
				if (! RootPoint->Next->Next)
					NumPts --;
				if (InsertList = Insert(RootPoint, NumPts, InsertAfter, InsertNumPts, InterStash->GetInsertSpline(), Geographic))
					{
					if (NewList = DBHost->MasterPoint.Allocate(Active->NumPoints() + InsertNumPts))
						{
						for (Pt = 0, PLinkA = Active->Points(), PLinkB = NewList; Pt <= InsertPoint; Pt ++, PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
							{
							PLinkB->Latitude = PLinkA->Latitude;
							PLinkB->Longitude = PLinkA->Longitude;
							PLinkB->Elevation = PLinkA->Elevation;
							} // for
						for (Pt = 0, PLinkC = InsertList; Pt < InsertNumPts; Pt ++, PLinkC = PLinkC->Next, PLinkB = PLinkB->Next)
							{
							PLinkB->Latitude = PLinkC->Latitude;
							PLinkB->Longitude = PLinkC->Longitude;
							PLinkB->Elevation = PLinkC->Elevation;
							} // for
						for (; PLinkA && PLinkB; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
							{
							PLinkB->Latitude = PLinkA->Latitude;
							PLinkB->Longitude = PLinkA->Longitude;
							PLinkB->Elevation = PLinkA->Elevation;
							} // for
						SignalPreChangePoints();
						DBHost->MasterPoint.DeAllocate(Active->Points());
						Active->Points(NewList);
						Active->NumPoints(Active->NumPoints() + InsertNumPts);
						SignalNewPoints(WCS_NOTIFYDBASECHANGE_POINTS_CONFORM);
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if
else
	NoJoeMessage();

} // VectorEditGUI::InsertPoints

/*===========================================================================*/

VectorPoint *VectorEditGUI::Interpolate(VectorPoint *DataPts, unsigned long int OrigNumPts, unsigned long int NewNumPts,
	short EvenOutSpacing, short ConsiderElev, short Linear, short Geographic)
{
VectorPoint *NewList = NULL, *PLink, **OrigPtList;
double SegLength, SegHeight, SumDist = 0.0, OrigSpacing, NewSpacing, Remainder, ThisPt,
	P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, CurFr, NxtFr, IbtFr, NxtInt, LstInt, *OrigDist, *NewDist;
unsigned long int OrigNumPtsM1, NewNumPtsM1, OrigPt, Pt;

if (DataPts && OrigNumPts > 1 + Joe::GetFirstRealPtNum())
	{
	OrigNumPtsM1 = OrigNumPts - Joe::GetFirstRealPtNum();
	if (OrigDist = (double *)AppMem_Alloc(OrigNumPtsM1 * sizeof (double), 0))
		{
		// build distance table to existing points
		if (OrigPtList = (VectorPoint **)AppMem_Alloc(OrigNumPtsM1 * sizeof (VectorPoint *), 0))
			{
			for (Pt = 0, PLink = Joe::GetFirstRealPtNum() > 0 ? DataPts->Next: DataPts; PLink; PLink = PLink->Next, Pt ++)
				{
				OrigDist[Pt] = SumDist;
				OrigPtList[Pt] = PLink;
				if (PLink->Next)
					{
					if (Geographic)
						SegLength = FindDistance(PLink->Latitude, PLink->Longitude, PLink->Next->Latitude, PLink->Next->Longitude, EARTHRAD);
					else
						SegLength = FindDistanceCartesian(PLink->Latitude, PLink->Longitude, PLink->Next->Latitude, PLink->Next->Longitude);
					if (ConsiderElev)
						SegHeight = (PLink->Elevation - PLink->Next->Elevation) / 1000.0;
					else
						SegHeight = 0.0;
					SumDist += sqrt(SegLength * SegLength + SegHeight * SegHeight);
					} // if
				} // for
			OrigSpacing = SumDist / (OrigNumPtsM1 - 1);
			NewNumPtsM1 = NewNumPts;		// do not subtract 1 for label pt
			if (NewNumPtsM1 > 1 && NewNumPtsM1 < WCS_DXF_MAX_OBJPTS - 1)
				{
				if (NewDist = (double *)AppMem_Alloc(NewNumPtsM1 * sizeof (double), 0))
					{
					NewSpacing = SumDist / (NewNumPtsM1 - 1);
					for (Pt = 0; Pt < NewNumPtsM1; Pt ++)
						{
						ThisPt = Pt * NewSpacing / OrigSpacing;
						OrigPt = (unsigned long int)ThisPt;
						Remainder = ThisPt -  OrigPt;
						if (OrigPt < OrigNumPtsM1 - 1)
							NewDist[Pt] = OrigDist[OrigPt] * (1.0 - Remainder) + OrigDist[OrigPt + 1] * Remainder;
						else
							NewDist[Pt] = OrigDist[OrigPt];
						} // if
					// now spline the lat, lon and elev values at NewDist distances
					if (NewList = DBHost->MasterPoint.Allocate(NewNumPtsM1 + Joe::GetFirstRealPtNum()))
						{
						OrigPt = 0;
						for (Pt = 0, PLink = Joe::GetFirstRealPtNum() > 0 ? NewList->Next: NewList; PLink; PLink = PLink->Next, Pt ++)
							{
							if (EvenOutSpacing)
								{
								ThisPt = Pt * NewSpacing;
								} // if
							else
								{
								ThisPt = NewDist[Pt];
								} // else
							for (; OrigPt < OrigNumPtsM1; OrigPt ++)
								{
								if (OrigDist[OrigPt] > ThisPt)
									break;
								} // for
							if (OrigPt > 0)
								OrigPt --;

							if (OrigPt < OrigNumPtsM1 - 1)
								{
								CurFr = OrigDist[OrigPt];
								NxtFr = OrigDist[OrigPt + 1];
								IbtFr = NxtFr - CurFr;

								if (IbtFr > 0.0)
									{
									if (Linear)
										{
										S1 = (ThisPt - OrigDist[OrigPt]) / IbtFr;
										PLink->Latitude = OrigPtList[OrigPt]->Latitude + (OrigPtList[OrigPt + 1]->Latitude - OrigPtList[OrigPt]->Latitude) * S1;
										PLink->Longitude = OrigPtList[OrigPt]->Longitude + (OrigPtList[OrigPt + 1]->Longitude - OrigPtList[OrigPt]->Longitude) * S1;
										PLink->Elevation = OrigPtList[OrigPt]->Elevation + (float)((OrigPtList[OrigPt + 1]->Elevation - OrigPtList[OrigPt]->Elevation) * S1);
										} // if
									else
										{ // splined
										S1 = (ThisPt - OrigDist[OrigPt]) / IbtFr;
										S2 = S1 * S1;
										S3 = S1 * S2;
										h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
										h2 = -2.0 * S3 + 3.0 * S2;
										h3 = S3 - 2.0 * S2 + S1;
										h4 = S3 - S2;

										if (OrigPt > 0)
											LstInt = CurFr - OrigDist[OrigPt - 1];
										if (OrigPt < OrigNumPtsM1 - 2)
											NxtInt = OrigDist[OrigPt + 2] - NxtFr;

										P1 = OrigPtList[OrigPt]->Latitude;
										P2 = OrigPtList[OrigPt + 1]->Latitude;
										D1 = OrigPt > 0 ?
											((.5 * (P1 - OrigPtList[OrigPt - 1]->Latitude))
											+ (.5 * (P2 - P1)))
											* (2.0 * IbtFr / (LstInt + IbtFr)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));

										D2 = OrigPt < OrigNumPtsM1 - 2 ?
											((.5 * (P2 - P1))
											+ (.5 * (OrigPtList[OrigPt + 2]->Latitude - P2)))
											* (2.0 * IbtFr / (IbtFr + NxtInt)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));
										PLink->Latitude = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

										P1 = OrigPtList[OrigPt]->Longitude;
										P2 = OrigPtList[OrigPt + 1]->Longitude;
										D1 = OrigPt > 0 ?
											((.5 * (P1 - OrigPtList[OrigPt - 1]->Longitude))
											+ (.5 * (P2 - P1)))
											* (2.0 * IbtFr / (LstInt + IbtFr)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));

										D2 = OrigPt < OrigNumPtsM1 - 2 ?
											((.5 * (P2 - P1))
											+ (.5 * (OrigPtList[OrigPt + 2]->Longitude - P2)))
											* (2.0 * IbtFr / (IbtFr + NxtInt)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));
										PLink->Longitude = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

										P1 = OrigPtList[OrigPt]->Elevation;
										P2 = OrigPtList[OrigPt + 1]->Elevation;
										D1 = OrigPt > 0 ?
											((.5 * (P1 - OrigPtList[OrigPt - 1]->Elevation))
											+ (.5 * (P2 - P1)))
											* (2.0 * IbtFr / (LstInt + IbtFr)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));

										D2 = OrigPt < OrigNumPtsM1 - 2 ?
											((.5 * (P2 - P1))
											+ (.5 * (OrigPtList[OrigPt + 2]->Elevation - P2)))
											* (2.0 * IbtFr / (IbtFr + NxtInt)):
											((.5 * (P2 - P1))
											+ (.5 * (P2 - P1)));
										PLink->Elevation = (float)(P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
										} // else Splined
									} // if dist > 0
								else
									{
									PLink->Latitude = OrigPtList[OrigPt]->Latitude;
									PLink->Longitude = OrigPtList[OrigPt]->Longitude;
									PLink->Elevation = OrigPtList[OrigPt]->Elevation;
									} // else
								} // if not last orig point
							else
								{
								PLink->Latitude = OrigPtList[OrigNumPtsM1 - 1]->Latitude;
								PLink->Longitude = OrigPtList[OrigNumPtsM1 - 1]->Longitude;
								PLink->Elevation = OrigPtList[OrigNumPtsM1 - 1]->Elevation;
								} // else
							} // for
						#ifdef WCS_JOE_LABELPOINTEXISTS
						NewList->Latitude = DataPts->Latitude;
						NewList->Longitude = DataPts->Longitude;
						NewList->Elevation = DataPts->Elevation;
						#endif // WCS_JOE_LABELPOINTEXISTS
						} // if
					AppMem_Free(NewDist, NewNumPtsM1 * sizeof (double));
					} // if
				} // if
			AppMem_Free(OrigPtList, OrigNumPtsM1 * sizeof (VectorPoint *));
			} // if
		AppMem_Free(OrigDist, OrigNumPtsM1 * sizeof (double));
		} // if
	} // if numpoints > 2

return (NewList);

} // VectorEditGUI::Interpolate

/*===========================================================================*/

VectorPoint *VectorEditGUI::Insert(VectorPoint *OrigPtList, unsigned long int OrigNumPts, unsigned long int InsertAfter,
	unsigned long int InsertNumPts, short InsertSpline, short Geographic)
{
VectorPoint *InsertList = NULL, *PLink, *SLink1, *SLink2, *SLink3, *SLink4;
double InsertSpacing, Dist1, Dist2, Dist3, SegHeight,
	P1, P2, D1, D2, S1, S2, S3, h1, h2, h3, h4, CurFr, NxtFr, IbtFr, NxtInt, LstInt;
unsigned long int Pt;

if (OrigPtList && OrigNumPts > 1 && InsertAfter < OrigNumPts - 1 && InsertNumPts > 0)
	{
	if (InsertAfter == 0)
		{
		SLink1 = NULL;
		SLink2 = OrigPtList;
		} // if
	else
		{
		SLink1 = OrigPtList;
		SLink2 = SLink1->Next;
		} // else
	if (SLink2)
		{
		SLink3 = SLink2->Next;
		if (SLink3)
			SLink4 = SLink3->Next;
		else
			return (NULL);
		} // if
	else
		return (NULL);
	// we can only get this far if SLink2 and SLinf3 are valid. SLink1 and SLink4 may be null
	if (SLink1)
		{
		Dist1 = FindDistance(SLink1->Latitude, SLink1->Longitude, SLink2->Latitude, SLink2->Longitude, EARTHRAD);
		if (Geographic)
			Dist1 = FindDistance(SLink1->Latitude, SLink1->Longitude, SLink2->Latitude, SLink2->Longitude, EARTHRAD);
		else
			Dist1 = FindDistanceCartesian(SLink1->Latitude, SLink1->Longitude, SLink2->Latitude, SLink2->Longitude);
		SegHeight = (SLink1->Elevation - SLink2->Elevation) / 1000.0;
		Dist1 = sqrt(Dist1 * Dist1 + SegHeight * SegHeight);
		if (Dist1 == 0.0)
			SLink1 = NULL;
		} // if
	else
		Dist1 = 0.0;
	Dist2 = FindDistance(SLink2->Latitude, SLink2->Longitude, SLink3->Latitude, SLink3->Longitude, EARTHRAD);
	SegHeight = (SLink2->Elevation - SLink3->Elevation) / 1000.0;
	Dist2 = sqrt(Dist2 * Dist2 + SegHeight * SegHeight);
	InsertSpacing = Dist2 / (InsertNumPts + 1);
	Dist2 += Dist1;
	if (SLink4)
		{
		Dist3 = FindDistance(SLink3->Latitude, SLink3->Longitude, SLink4->Latitude, SLink4->Longitude, EARTHRAD);
		SegHeight = (SLink3->Elevation - SLink4->Elevation) / 1000.0;
		Dist3 = sqrt(Dist3 * Dist3 + SegHeight * SegHeight);
		if (Dist3 == 0.0)
			SLink4 = NULL;
		Dist3 += Dist2;
		} // if
	else 
		Dist3 = Dist2;	// probably not needed
	if (InsertList = DBHost->MasterPoint.Allocate(InsertNumPts))
		{
		for (PLink = InsertList, Pt = 1; PLink; PLink = PLink->Next, Pt ++)
			{
			CurFr = Dist1;
			NxtFr = Dist2;
			IbtFr = NxtFr - CurFr;

			if (IbtFr > 0.0)
				{
				S1 = (Pt * InsertSpacing) / IbtFr;
				if (InsertSpline)
					{
					S2 = S1 * S1;
					S3 = S1 * S2;
					h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
					h2 = -2.0 * S3 + 3.0 * S2;
					h3 = S3 - 2.0 * S2 + S1;
					h4 = S3 - S2;

					if (SLink1)
						LstInt = Dist1;
					if (SLink4)
						NxtInt = Dist3 - Dist2;

					P1 = SLink2->Latitude;
					P2 = SLink3->Latitude;
					D1 = SLink1 ?
						((.5 * (P1 - SLink1->Latitude))
						+ (.5 * (P2 - P1)))
						* (2.0 * IbtFr / (LstInt + IbtFr)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));

					D2 = SLink4 ?
						((.5 * (P2 - P1))
						+ (.5 * (SLink4->Latitude - P2)))
						* (2.0 * IbtFr / (IbtFr + NxtInt)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));
					PLink->Latitude = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

					P1 = SLink2->Longitude;
					P2 = SLink3->Longitude;
					D1 = SLink1 ?
						((.5 * (P1 - SLink1->Longitude))
						+ (.5 * (P2 - P1)))
						* (2.0 * IbtFr / (LstInt + IbtFr)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));

					D2 = SLink4 ?
						((.5 * (P2 - P1))
						+ (.5 * (SLink4->Longitude - P2)))
						* (2.0 * IbtFr / (IbtFr + NxtInt)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));
					PLink->Longitude = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

					P1 = SLink2->Elevation;
					P2 = SLink3->Elevation;
					D1 = SLink1 ?
						((.5 * (P1 - SLink1->Elevation))
						+ (.5 * (P2 - P1)))
						* (2.0 * IbtFr / (LstInt + IbtFr)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));

					D2 = SLink4 ?
						((.5 * (P2 - P1))
						+ (.5 * (SLink4->Elevation - P2)))
						* (2.0 * IbtFr / (IbtFr + NxtInt)):
						((.5 * (P2 - P1))
						+ (.5 * (P2 - P1)));
					PLink->Elevation = (float)(P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4);
					} // if ! linear
				else
					{
					PLink->Latitude = SLink2->Latitude + (SLink3->Latitude - SLink2->Latitude) * S1;
					PLink->Longitude = SLink2->Longitude + (SLink3->Longitude - SLink2->Longitude) * S1;
					PLink->Elevation = SLink2->Elevation + (float)((SLink3->Elevation - SLink2->Elevation) * S1);
					} // else
				} // if between > 0
			else
				{
				PLink->Latitude = SLink2->Latitude;
				PLink->Longitude = SLink2->Longitude;
				PLink->Elevation = SLink2->Elevation;
				} // else
			} // for
		} // if
	} // if

return (InsertList);

} // VectorEditGUI::Insert

/*===========================================================================*/

void VectorEditGUI::SelectNewCoords(void)
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

} // VectorEditGUI::SelectNewCoords

/*===========================================================================*/
