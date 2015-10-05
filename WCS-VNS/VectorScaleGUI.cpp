// VectorScaleGUI.cpp
// Code for Vector Elevation Scale editor
// Built from KeyScaleGUI.cpp on 7/1/97 by Gary R. Huber
// Copyright 1997 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "VectorScaleGUI.h"
#include "Joe.h"
#include "WCSWidgets.h"
#include "Database.h"
#include "Notify.h"
#include "Application.h"
#include "Points.h"
#include "Useful.h"
#include "Security.h"
#include "Toolbar.h"
#include "Interactive.h"
#include "ProjectDispatch.h"
#include "Project.h"
#include "AppMem.h"
#include "resource.h"

extern WCSApp *GlobalApp;

extern NotifyTag DBPreChangeEventPOINTS[];
extern NotifyTag DBChangeEventPOINTS[];

NativeGUIWin VectorScaleGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // VectorScaleGUI::Open

/*===========================================================================*/

NativeGUIWin VectorScaleGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_VECTOR_SCALE, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // VectorScaleGUI::Construct

/*===========================================================================*/

VectorScaleGUI::VectorScaleGUI(Database *DBSource, Project *ProjSource, Joe *ActiveSource)
: GUIFenetre('VESC', this, "Scale Vector Elevations") // Yes, I know...
{
static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 0};
static NotifyTag AllProjEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

ConstructError = 0;
HostDB = DBSource;
ProjHost = ProjSource;
Backup = NULL;
Active = ActiveSource;

HostDB->RegisterClient(this, AllDBEvents);
ProjHost->RegisterClient(this, AllProjEvents);

if (Active)
	{
	VSh.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE | WCS_ANIMCRITTER_FLAG_NONODES);
	VSh.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
	VSh.SetIncrement(1.0);
	VSh.SetValue(0.0);
	VScDbl = 1.0;
	VShDbl = 0.0;
	ValueScale = ValueShift = 0;
	if (Backup = new ((char *)Active->Name()) Joe)
		{
		if (! Active->CopyPoints(Backup, Active, 0, 1))
			ConstructError = 1;
		} // if
	else
		ConstructError = 1;
	} // else
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // VectorScaleGUI::VectorScaleGUI

/*===========================================================================*/

VectorScaleGUI::~VectorScaleGUI()
{

HostDB->RemoveClient(this);
ProjHost->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // VectorScaleGUI::~VectorScaleGUI()

/*===========================================================================*/

long VectorScaleGUI::HandleCloseWin(NativeGUIWin NW)
{

HostDB->RemoveClient(this);
ProjHost->RemoveClient(this);
Close();

return(0);

} // VectorScaleGUI::HandleCloseWin

/*===========================================================================*/

long VectorScaleGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(037, 37);
switch(ButtonID)
	{
	case IDCANCEL:
		{
		HostDB->RemoveClient(this);
		ProjHost->RemoveClient(this);
		Active->CopyPoints(Active, Backup, 1, 1);
		Close();
		break;
		} // 
	case ID_KEEP:
		{
		HostDB->RemoveClient(this);
		ProjHost->RemoveClient(this);
		Close();
		break;
		} // 
	case IDC_SCALEVALUES:
		{
		short Checked;
		Checked = WidgetGetCheck(IDC_SCALEVALUES) ? 1: 0;
		ValueScale = Checked;
		break;
		} //
	case IDC_SHIFTVALUES:
		{
		short Checked;
		Checked = WidgetGetCheck(IDC_SHIFTVALUES) ? 1: 0;
		ValueShift = Checked;
		break;
		} //
	case ID_OPERATE:
		{
		ScaleKeys();
		break;
		} //
	default: break;
	} // ButtonID

return(0);

} // VectorScaleGUI::HandleButtonClick

/*===========================================================================*/

long VectorScaleGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_SCALEVALAMT:
		{
		WidgetSetCheck(IDC_SCALEVALUES, TRUE);
		ValueScale = 1;
		break;
		} //
	case IDC_SHIFTVALAMT:
		{
		WidgetSetCheck(IDC_SHIFTVALUES, TRUE);
		VShDbl = VSh.CurValue;
		ValueShift = 1;
		break;
		} //
	} // ID

return(0);

} // VectorScaleGUI::HandleFIChange

/*===========================================================================*/

void VectorScaleGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[4];
//short ListEntry;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_NEW, 0xff, 0xff);
Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRELOAD, 0xff, 0xff);
Interested[2] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_DELOBJ, 0xff, 0xff);
Interested[3] = NULL;
if (HostDB->MatchNotifyClass(Interested, Changes, 0))
	{
	if (HostDB)
		HostDB->RemoveClient(this);
	Close();
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
		// <<<>>> there may not be anything that needs to be done here
		} // if points changed
	} // else

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (ProjHost->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	} // if

} // VectorScaleGUI::HandleNotifyEvent()

/*===========================================================================*/

void VectorScaleGUI::ConfigureWidgets(void)
{

WidgetSetText(IDC_VECNAME, Active->GetBestName());

ConfigureFI(NativeWin, IDC_SCALEVALAMT,
 &VScDbl,
  1.0,
   (double)-FLT_MAX,
    (double)FLT_MAX,
     FIOFlag_Double,
      (SetCritter *)NULL,
       NULL);

WidgetSNConfig(IDC_SHIFTVALAMT, &VSh);

} // VectorScaleGUI::ConfigureWidgets()

/*===========================================================================*/

void VectorScaleGUI::ScaleKeys(void)
{
long Count, NumPoints = Active->NumPoints();
VectorPoint *PLink;
NotifyTag Changes[2];

if ((ValueScale && VScDbl != 0.0) || (ValueShift && VShDbl != 0.0))
	{
	HostDB->GenerateNotify(DBPreChangeEventPOINTS, Active);

	for (Count = 0, PLink = Active->Points(); Count < NumPoints && PLink; Count ++, PLink = PLink->Next)
		{
		if (ValueScale)
			{
			PLink->Elevation = (float)(PLink->Elevation * VScDbl);
			} // if
		if (ValueShift)
			{
			PLink->Elevation = (float)(PLink->Elevation + VShDbl);
			} // if
		} // for

	HostDB->GenerateNotify(DBChangeEventPOINTS, Active);
	Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active);
	} // if

} // VectorScaleGUI::ScaleKeys()

/*===========================================================================*/
