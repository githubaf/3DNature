// ProjectPrefsGUI.cpp
// Code for Project Preferences editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "ProjectPrefsGUI.h"
#include "Project.h"
#include "ProjectIO.h"
#include "WCSWidgets.h"
#include "ProjectDispatch.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Notify.h"
#include "resource.h"

char *ProjectPrefsGUI::TabNames[8] = {"General", "Matrix", "Project", "Interaction", "Units", "Paths", "DEM Directories", "Config"};

long ProjectPrefsGUI::ActivePage;

NativeGUIWin ProjectPrefsGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ProjectPrefsGUI::Open

/*===========================================================================*/

NativeGUIWin ProjectPrefsGUI::Construct(void)
{
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_PREFS, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_PREFS_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_PREFS_VIEWPORTS, 0, 1);
	CreateSubWinFromTemplate(IDD_PREFS_PROJECT, 0, 2);
	CreateSubWinFromTemplate(IDD_PREFS_INTERACT, 0, 3);
	CreateSubWinFromTemplate(IDD_PREFS_UNITS, 0, 4);
	CreateSubWinFromTemplate(IDD_PREFS_PATHS, 0, 5);
	#ifdef WCS_BUILD_V2
	CreateSubWinFromTemplate(IDD_PREFS_DEMDIRS_VNS, 0, 6);
	#else // !WCS_BUILD_V2
	CreateSubWinFromTemplate(IDD_PREFS_DEMDIRS, 0, 6);
	#endif // !WCS_BUILD_V2
	CreateSubWinFromTemplate(IDD_PREFS_CONFIG, 0, 7);

	if (NativeWin)
		{
		WidgetCBAddEnd(IDC_FRAMERATEDROP, "NTSC Video");
		WidgetCBAddEnd(IDC_FRAMERATEDROP, "PAL Video");
		WidgetCBAddEnd(IDC_FRAMERATEDROP, "Film");
		WidgetCBAddEnd(IDC_FRAMERATEDROP, "Custom");

		for(TabIndex = WCS_USEFUL_UNIT_MILLIMETER; TabIndex <= WCS_USEFUL_UNIT_FEET_US_SURVEY; TabIndex ++)
			{
			WidgetCBAddEnd(IDC_DISTANCEUNITSDROP, GetUnitName(TabIndex));
			WidgetCBAddEnd(IDC_HEIGHTUNITSDROP, GetUnitName(TabIndex));
			} // for

		WidgetCBAddEnd(IDC_ANGLEUNITSDROP, "Decimal Degrees");
		WidgetCBAddEnd(IDC_ANGLEUNITSDROP, "Deg., Min., Sec.");

		WidgetCBAddEnd(IDC_TIMEUNITSDROP, "Seconds");
		WidgetCBAddEnd(IDC_TIMEUNITSDROP, "Frames");

		WidgetCBAddEnd(IDC_POSLONDROP, "West");
		WidgetCBAddEnd(IDC_POSLONDROP, "East");

		WidgetCBAddEnd(IDC_LATLONDISPLAY, "+/-");
		WidgetCBAddEnd(IDC_LATLONDISPLAY, "N/S, W/E");

		WidgetCBAddEnd(IDC_GEOPROJDISPLAY, "As Geographic");
		WidgetCBAddEnd(IDC_GEOPROJDISPLAY, "As Projected");

		for (TabIndex = 0; TabIndex < 8; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // ProjectPrefsGUI::Construct

/*===========================================================================*/

ProjectPrefsGUI::ProjectPrefsGUI(Project *Moi, InterCommon *ISource)
: GUIFenetre('PREF', this, "Preferences") // Yes, I know...
{
static NotifyTag AllICEvents[] = {MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, 0xff),
									MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0xff),
									MAKE_ID(WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_PROJFRAMERATE, 0xff),
									0};
static NotifyTag ProjEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
									0};
double RangeDefaults[3];

ConstructError = 0;
HostProj = Moi;
InterStash = ISource;

MoveInProgress = 0;
SwapInProgress = 0;
#ifdef _WIN32
Curse = NULL;
#endif // _WIN32

strcpy(Dirname, HostProj->dirname);
DLCopy = HostProj->DirList_Copy(HostProj->DL);

RangeDefaults[0] = 90.0;
RangeDefaults[1] = -90.0;
RangeDefaults[2] = 1.0;
RefLatADT.SetDefaults(NULL, 0, HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
RefLatADT.SetRangeDefaults(RangeDefaults);
RefLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
RefLatADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

RangeDefaults[0] = 360.0;
RangeDefaults[1] = -360.0;
RangeDefaults[2] = 1.0;
RefLonADT.SetDefaults(NULL, 0, HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
RefLonADT.SetRangeDefaults(RangeDefaults);
RefLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
RefLonADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

RangeDefaults[0] = 1000000.0;
RangeDefaults[1] = -1000000.0;
RangeDefaults[2] = 1.0;
RefElevADT.SetDefaults(NULL, 0, HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z));
RefElevADT.SetRangeDefaults(RangeDefaults);
RefElevADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
RefElevADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

InterStash->RegisterClient(this, AllICEvents);
HostProj->RegisterClient(this, ProjEvents);
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // ProjectPrefsGUI::ProjectPrefsGUI

/*===========================================================================*/

ProjectPrefsGUI::~ProjectPrefsGUI()
{

HostProj->DirList_Del(DLCopy);
InterStash->RemoveClient(this);
HostProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ProjectPrefsGUI::~ProjectPrefsGUI()

/*===========================================================================*/

long ProjectPrefsGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_PPG, 0);

return(0);

} // ProjectPrefsGUI::HandleCloseWin

/*===========================================================================*/

long ProjectPrefsGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

if (SwapInProgress || MoveInProgress)
	{
	SwapInProgress = MoveInProgress = 0;
#ifdef _WIN32
	if (Curse)
		SetClassLong(GetDlgItem(NativeWin, IDC_PARLIST), GCL_HCURSOR, (long)Curse);
#endif // _WIN32
	EndPointer();
	} // if
switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_PPG, 0);
		break;
		} // 
	case IDCANCEL:
		{
		DoCancel();
		//AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
		//	WCS_TOOLBAR_ITEM_PPG, 0);
		break;
		} // 
	case IDC_MATRIX_1:
		{
		SetViewport(0);
		break;
		} // 
	case IDC_MATRIX_2:
		{
		SetViewport(1);
		break;
		} // 
	case IDC_MATRIX_3:
		{
		SetViewport(2);
		break;
		} // 
	case IDC_MATRIX_4:
		{
		SetViewport(3);
		break;
		} // 
	case IDC_MATRIX_5:
		{
		SetViewport(4);
		break;
		} // 
	case IDC_MATRIX_6:
		{
		SetViewport(5);
		break;
		} // 
	case IDC_MATRIX_7:
		{
		SetViewport(6);
		break;
		} // 
	case IDC_MATRIX_8:
		{
		SetViewport(7);
		break;
		} // 
	case IDC_MATRIX_9:
		{
		SetViewport(8);
		break;
		} // 
	case IDC_MATRIX_10:
		{
		SetViewport(9);
		break;
		} // 
	case IDC_MATRIX_11:
		{
		SetViewport(10);
		break;
		} // 
	case IDC_MATRIX_12:
		{
		SetViewport(11);
		break;
		} // 
	case ID_DEFAULT:
		{
		DoDefaultDir();
		break;
		} // 
	case IDC_ADD:
		{
		DoAdd();
		break;
		} // 
	case IDC_MOVEDLUP:
		{
		HandleMove(0); // up
		break;
		} // up
	case IDC_MOVEDLDOWN:
		{
		HandleMove(1); // down
		break;
		} // down
/*
	case IDC_SWAP:
		{
		//DoSwap(0);
		break;
		} // 
	case IDC_MOVE:
		{
		//DoMove(0);
		break;
		} // 
*/
	case IDC_REMOVE:
		{
		DoRemove();
		break;
		} // 
	case IDC_READONLY:
		{
		DoReadOnly();
		break;
		} // 
	case IDC_LOAD:
		{
		DoLoad();
		break;
		} // 
	case IDC_SET:
		{
		DoAdvConfig(1);
		break;
		} // SET
	case IDC_UNSET:
		{
		DoAdvConfig(2);
		break;
		} // IDC_UNSET
	} // switch

return(0);

} // ProjectPrefsGUI::HandleButtonClick

/*===========================================================================*/

long ProjectPrefsGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{
if (NW == NativeWin)
	{
	switch (CtrlID)
		{
		case IDC_PARLIST:
			{
			DoRemove();
			break;
			} // IDC_PARLIST
		} // switch
	} // if

return(0);
} // ProjectPrefsGUI::HandleListDelItem

/*===========================================================================*/

long ProjectPrefsGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
/*
		if (MoveInProgress)
			{
			DoMove(1);
			}
		else if (SwapInProgress)
			{
			DoSwap(1);
			}
		else
*/
			DoActiveChange();
		break;
		} // IDC_PARLIST
	case IDC_ADVLIST:
		{
		DoAdvConfig(0);
		break;
		} // 
	} // switch CtrlID

return (0);

} // ProjectPrefsGUI::HandleListSel

/*===========================================================================*/

long ProjectPrefsGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_PRJ_WCSPROJECTS:
		{
		strcpy(HostProj->projectpath, "WCSProjects:");
		break;
		} // IDC_PRJ_WCSPROJECTS
	default: break;
	} // ID
Changes[0] = MAKE_ID(WCS_PROJECTCLASS_PATHS, 0, 0, 0);
Changes[1] = 0;
HostProj->GenerateNotify(Changes);

return(0);

} // ProjectPrefsGUI::HandleDDChange

/*===========================================================================*/

long ProjectPrefsGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_DISTANCEUNITSDROP:
		{
		SetDistanceUnits();
		break;
		}
	case IDC_HEIGHTUNITSDROP:
		{
		SetHeightUnits();
		break;
		}
	case IDC_ANGLEUNITSDROP:
		{
		SetAngleUnits();
		break;
		}
	case IDC_TIMEUNITSDROP:
		{
		SetTimeUnits();
		break;
		}
	case IDC_POSLONDROP:
		{
		SetPosLonHemi();
		break;
		}
	case IDC_LATLONDISPLAY:
		{
		SetLatLonDisplay();
		break;
		}
	case IDC_GEOPROJDISPLAY:
		{
		SetGeoProjDisplay();
		break;
		}
	case IDC_FRAMERATEDROP:
		{
		SetFrameRate();
		break;
		}
	} // switch CtrlID

return (0);

} // ProjectPrefsGUI::HandleCBChange

/*===========================================================================*/

long ProjectPrefsGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
	} // switch

ActivePage = NewPageID;

return(0);

} // ProjectPrefsGUI::HandlePageChange

/*===========================================================================*/

long ProjectPrefsGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKMATRIXTASKMODE:
		{
		if (GlobalApp->MCP)
			GlobalApp->MCP->EnforceTaskModeWindowCompliance(GlobalApp->MainProj->Prefs.TaskMode);	// a long name for a tedious task
		break;
		} // 
	} // switch CtrlID

return(0);

} // ProjectPrefsGUI::HandleSCChange

/*===========================================================================*/

long ProjectPrefsGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_REFLAT:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Y, RefLatADT.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} // 
	case IDC_REFLON:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_X, RefLonADT.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} // 
	case IDC_REFELEV:
		{
		InterStash->SetParam(1, WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, WCS_INTERVEC_COMP_Z, RefElevADT.CurValue,
			WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0, 0, NULL);
		break;
		} // 
	} // switch CtrlID

return(0);

} // ProjectPrefsGUI::HandleFIChange

/*===========================================================================*/

void ProjectPrefsGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORD, 0xff);
Interested[1] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[2] = NULL;
if (Changed = InterStash->MatchNotifyClass(Interested, Changes, 0))
	{
	RefLatADT.SetValue(HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
	RefLonADT.SetValue(HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	RefElevADT.SetValue(HostProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z));
	WidgetSNSync(IDC_REFLAT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFLON, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFELEV, WP_FISYNC_NONOTIFY);
	} // if
Interested[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0xff);
Interested[1] = NULL;
if (Changed = InterStash->MatchNotifyClass(Interested, Changes, 0))
	{
	WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
	} // if
Interested[0] = MAKE_ID(WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_PROJFRAMERATE, 0xff);
if (Changed = InterStash->MatchNotifyClass(Interested, Changes, 1))
	{
	WidgetFISync(IDC_FRAMERATE, WP_FISYNC_NONOTIFY);
	SyncFrameDrop();
	} // if

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff);
if (HostProj->MatchNotifyClass(Interested, Changes, 1))
	{
	ConfigureWidgets();
	} // if

} // ProjectPrefsGUI::HandleNotifyEvent()

/*===========================================================================*/

void ProjectPrefsGUI::ConfigureWidgets(void)
{
short Config;

ConfigureSC(NativeWin, IDC_CHECKVECPOLYLIMIT, &HostProj->Prefs.MemoryLimitsEnabled, SCFlag_Short);
ConfigureSC(NativeWin, IDC_CHECKGLOBALADVANCED, NULL, SCFlag_Short, HostProj, MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0));

ConfigureSC(NativeWin, IDC_LOADONOPEN, &HostProj->Prefs.LoadOnOpen, SCFlag_Short);
ConfigureSC(NativeWin, IDC_OPENWINS, &HostProj->Prefs.OpenWindows, SCFlag_Short);

ConfigureSC(NativeWin, IDC_CHECKMATRIXTASKMODE, &HostProj->Prefs.MatrixTaskModeEnabled, SCFlag_Short);

ConfigureSC(NativeWin, IDC_CHECKFLOATING, NULL, SCFlag_Short,
	  (SetCritter *)InterStash,
	   MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_VALUE, WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT, 0));

ConfigureSR(NativeWin, IDC_RADIORELTOGROUND, IDC_RADIORELTOGROUND, NULL, SRFlag_Short, WCS_INTERACTIVE_STYLE_LIGHTWAVE, HostProj, MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_INTERSTYLE, 0));
ConfigureSR(NativeWin, IDC_RADIORELTOGROUND, IDC_RADIORELTOCAMERA, NULL, SRFlag_Short, WCS_INTERACTIVE_STYLE_MAX, HostProj, MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_INTERSTYLE, 0));

WidgetSNConfig(IDC_REFLAT, &RefLatADT);
WidgetSNConfig(IDC_REFLON, &RefLonADT);
WidgetSNConfig(IDC_REFELEV, &RefElevADT);

ConfigureFI(NativeWin, IDC_VECPOLYLIMITMEGS,
 NULL,
  10.0,
   20.0,
	1000000.0,
	 FIOFlag_Long,
	  (SetCritter *)HostProj,
	   MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_VECPOLYLIMITMEGS, 0));

ConfigureFI(NativeWin, IDC_DEMLIMITMEGS,
 NULL,
  10.0,
   20.0,
	1000000.0,
	 FIOFlag_Long,
	  (SetCritter *)HostProj,
	   MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_DEMLIMITMEGS, 0));

ConfigureFI(NativeWin, IDC_FRAMERATE,
 NULL,
  1.0,
   1.0,
	1000.0,
	 FIOFlag_Double,
	  (SetCritter *)InterStash,
	   MAKE_ID(WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_PROJFRAMERATE, 0));

ConfigureFI(NativeWin, IDC_MAXDBITEMS,
 NULL,
  1.0,
   100.0,
	10000000.0,
	 FIOFlag_Long,
	  (SetCritter *)HostProj,
	   MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_MAXSAGDBITEMS, 0));

ConfigureFI(NativeWin, IDC_MAXSORTEDDBITEMS,
 NULL,
  1.0,
   100.0,
	10000000.0,
	 FIOFlag_Long,
	  (SetCritter *)HostProj,
	   MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_MAXSORTEDSAGDBITEMS, 0));

ConfigureFI(NativeWin, IDC_NUMSIGDIGITS,
 NULL,
  1.0,
   4.0,
	20.0,
	 FIOFlag_Short,
	  (SetCritter *)HostProj,
	   MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_SIGNIFICANTDIGITS, 0));

SyncFrameDrop();

WidgetCBSetCurSel(IDC_DISTANCEUNITSDROP, HostProj->Prefs.HorDisplayUnits - WCS_USEFUL_UNIT_MILLIMETER);
WidgetCBSetCurSel(IDC_HEIGHTUNITSDROP, HostProj->Prefs.VertDisplayUnits - WCS_USEFUL_UNIT_MILLIMETER);
WidgetCBSetCurSel(IDC_ANGLEUNITSDROP, HostProj->Prefs.AngleDisplayUnits);
WidgetCBSetCurSel(IDC_TIMEUNITSDROP, HostProj->Prefs.TimeDisplayUnits);
WidgetCBSetCurSel(IDC_POSLONDROP, HostProj->Prefs.PosLonHemisphere);
WidgetCBSetCurSel(IDC_LATLONDISPLAY, HostProj->Prefs.LatLonSignDisplay);
WidgetCBSetCurSel(IDC_GEOPROJDISPLAY, HostProj->Prefs.DisplayGeoUnitsProjected);

ConfigureTB(NativeWin, IDC_MATRIX_1, IDI_MATRIX_1, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_2, IDI_MATRIX_2, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_3, IDI_MATRIX_3, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_4, IDI_MATRIX_4, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_5, IDI_MATRIX_5, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_6, IDI_MATRIX_6, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_7, IDI_MATRIX_7, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_8, IDI_MATRIX_8, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_9, IDI_MATRIX_9, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_10, IDI_MATRIX_10, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_11, IDI_MATRIX_11, NULL);
ConfigureTB(NativeWin, IDC_MATRIX_12, IDI_MATRIX_12, NULL);
Config = HostProj->Prefs.GUIConfiguration;
WidgetSetCheck(IDC_MATRIX_1, (Config == 0));
WidgetSetCheck(IDC_MATRIX_2, (Config == 1));
WidgetSetCheck(IDC_MATRIX_3, (Config == 2));
WidgetSetCheck(IDC_MATRIX_4, (Config == 3));
WidgetSetCheck(IDC_MATRIX_5, (Config == 4));
WidgetSetCheck(IDC_MATRIX_6, (Config == 5));
WidgetSetCheck(IDC_MATRIX_7, (Config == 6));
WidgetSetCheck(IDC_MATRIX_8, (Config == 7));
WidgetSetCheck(IDC_MATRIX_9, (Config == 8));
WidgetSetCheck(IDC_MATRIX_10, (Config == 9));
WidgetSetCheck(IDC_MATRIX_11, (Config == 10));
WidgetSetCheck(IDC_MATRIX_12, (Config == 11));

ConfigureDD(NativeWin, IDC_PRJ_WCSPROJECTS, HostProj->pcprojectpath, 255, NULL, 0, IDC_LABEL_WCSPROJECTS);
ConfigureDD(NativeWin, IDC_PRJ_WCSFRAMES, HostProj->pcframespath, 255, NULL, 0, IDC_LABEL_WCSFRAMES);
ConfigureDD(NativeWin, IDC_PRJ_WCSCONTENT, HostProj->contentpath, 255, NULL, 0, IDC_LABEL_WCSCONTENT);
ConfigureDD(NativeWin, IDC_PRJ_PROJ, HostProj->projectpath, 255, HostProj->projectname, 63, IDC_LABEL_PROJ);
ConfigureDD(NativeWin, IDC_PRJ_DEFDIR, HostProj->dirname, 255, NULL, 0, IDC_LABEL_DEFDIR);
ConfigureTB(NativeWin, IDCANCEL, IDI_REVERT, NULL);
ConfigureTB(NativeWin, IDC_LOAD, IDI_FILEOPEN, NULL);

ConfigureTB(NativeWin, IDC_ADD, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEDLUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEDLDOWN, IDI_ARROWDOWN, NULL);

BuildList(0);

{ // build advanced config list
int ConfigLoop;

WidgetLBClear(IDC_ADVLIST);
for(ConfigLoop = 0; ConfigLoop < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; ConfigLoop++)
	{
	if (GlobalApp->MainProj->Prefs.ConfigOptions[ConfigLoop][0] != NULL)
		{
		WidgetLBInsert(IDC_ADVLIST, -1, GlobalApp->MainProj->Prefs.ConfigOptions[ConfigLoop]);
		} // if
	} // for
} // build advanced config list

WidgetSetText(IDC_LABEL_TEMP, HostProj->dirname);

} // ProjectPrefsGUI::ConfigureWidgets()

/*===========================================================================*/

void ProjectPrefsGUI::SyncFrameDrop(void)
{
int RateNum;
double FrameRate;

FrameRate = InterStash->GetFrameRate();
if (FrameRate == WCS_FRAMERATE_NTSC_VIDEO)
	RateNum = 0;
else if (FrameRate == WCS_FRAMERATE_PAL_VIDEO)
	RateNum = 1;
else if (FrameRate == WCS_FRAMERATE_FILM)
	RateNum = 2;
else
	RateNum = 3;

WidgetCBSetCurSel(IDC_FRAMERATEDROP, RateNum);

} // ProjectPrefsGUI::SyncFrameDrop

/*===========================================================================*/

void ProjectPrefsGUI::SetFrameRate(void)
{
long Current;
short NewRate = 0;

Current = WidgetCBGetCurSel(IDC_FRAMERATEDROP);
switch (Current)
	{
	case 0:
		{
		NewRate = WCS_FRAMERATE_NTSC_VIDEO;
		break;
		} // ntsc video
	case 1:
		{
		NewRate = WCS_FRAMERATE_PAL_VIDEO;
		break;
		} // pal video
	case 2:
		{
		NewRate = WCS_FRAMERATE_FILM;
		break;
		} // film
	} // switch
if (NewRate)
	{
	InterStash->SetParam(1, WCS_INTERCLASS_MISC, WCS_INTERMISC_SUBCLASS_MISC, WCS_INTERMISC_ITEM_PROJFRAMERATE, 0, (double)NewRate, NULL);
	} // if

} // ProjectPrefsGUI::SetFrameRate

/*===========================================================================*/

void ProjectPrefsGUI::SetDistanceUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_DISTANCEUNITSDROP) + WCS_USEFUL_UNIT_MILLIMETER;
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_HORDISPLAYUNITS, 0, Current, NULL);

} // ProjectPrefsGUI::SetDistanceUnits

/*===========================================================================*/

void ProjectPrefsGUI::SetHeightUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_HEIGHTUNITSDROP) + WCS_USEFUL_UNIT_MILLIMETER;
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_VERTDISPLAYUNITS, 0, Current, NULL);

} // ProjectPrefsGUI::SetHeightUnits

/*===========================================================================*/

void ProjectPrefsGUI::SetAngleUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_ANGLEUNITSDROP);
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_ANGLEDISPLAYUNITS, 0, Current, NULL);

} // ProjectPrefsGUI::SetAngleUnits

/*===========================================================================*/

void ProjectPrefsGUI::SetTimeUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_TIMEUNITSDROP);
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_TIMEDISPLAYUNITS, 0, Current, NULL);

} // ProjectPrefsGUI::SetTimeUnits

/*===========================================================================*/

void ProjectPrefsGUI::SetPosLonHemi(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_POSLONDROP);
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_POSLONHEMISPHERE, 0, Current, NULL);

} // ProjectPrefsGUI::SetPosLonHemi

/*===========================================================================*/

void ProjectPrefsGUI::SetLatLonDisplay(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_LATLONDISPLAY);
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_LATLONSIGNDISPLAY, 0, Current, NULL);

} // ProjectPrefsGUI::SetLatLonDisplay

/*===========================================================================*/

void ProjectPrefsGUI::SetGeoProjDisplay(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_GEOPROJDISPLAY);
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, WCS_PROJPREFS_GEOPROJDISPLAY, 0, Current, NULL);

} // ProjectPrefsGUI::SetGeoProjDisplay

/*===========================================================================*/

void ProjectPrefsGUI::SetViewport(short ViewportID)
{

WidgetSetCheck(IDC_MATRIX_1, (ViewportID == 0));
WidgetSetCheck(IDC_MATRIX_2, (ViewportID == 1));
WidgetSetCheck(IDC_MATRIX_3, (ViewportID == 2));
WidgetSetCheck(IDC_MATRIX_4, (ViewportID == 3));
WidgetSetCheck(IDC_MATRIX_5, (ViewportID == 4));
WidgetSetCheck(IDC_MATRIX_6, (ViewportID == 5));
WidgetSetCheck(IDC_MATRIX_7, (ViewportID == 6));
WidgetSetCheck(IDC_MATRIX_8, (ViewportID == 7));
WidgetSetCheck(IDC_MATRIX_9, (ViewportID == 8));
WidgetSetCheck(IDC_MATRIX_10, (ViewportID == 9));
WidgetSetCheck(IDC_MATRIX_11, (ViewportID == 10));
WidgetSetCheck(IDC_MATRIX_12, (ViewportID == 11));
HostProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, ViewportID, NULL);

} // ProjectPrefsGUI::SetViewport

/*===========================================================================*/

short ProjectPrefsGUI::BuildList(int Active)
{
struct DirList *DLItem, *DLActive;
int Count = 0;

WidgetLBClear(IDC_PARLIST);

if (HostProj->DL)
	{
	DLItem = DLActive = HostProj->DL;
	while (DLItem)
		{
		if (Count == ActiveItem)
			DLActive = DLItem;
		WidgetLBInsert(IDC_PARLIST, -1, &DLItem->Read);
		DLItem = DLItem->Next;
		Count ++;
		}
	WidgetLBSetCurSel(IDC_PARLIST, Active);
	WidgetSetCheck(IDC_READONLY, (DLActive->Read == '*'));
	return (1);
	} // if

return (0);

} // ProjectPrefsGUI::BuildList()

/*===========================================================================*/

void ProjectPrefsGUI::DoKeep(void)
{

// Nothing to do

} // ProjectPrefsGUI::DoKeep()

/*===========================================================================*/

void ProjectPrefsGUI::DoCancel(void)
{
NotifyTag Changes[3];

Changes[0] = MAKE_ID(WCS_PROJECTCLASS_PATHS, WCS_PATHS_DIRLIST, 0, 0);
Changes[1] = MAKE_ID(WCS_PROJECTCLASS_PATHS, WCS_PATHS_DEFAULTDIR, 0, 0);
Changes[2] = 0;
swmem(&DLCopy, &HostProj->DL, sizeof (struct DirList *));
strcpy(HostProj->dirname, Dirname);
HostProj->GenerateNotify(Changes);

} // ProjectPrefsGUI::DoCancel()

/*===========================================================================*/

void ProjectPrefsGUI::DoDefaultDir(void)
{
int Current;
struct DirList *DLDef;
NotifyTag Changes[3];

Current = WidgetLBGetCurSel(IDC_PARLIST);
if (DLDef = HostProj->DirList_Search(HostProj->DL, (short)Current))
	{
	strcpy(HostProj->dirname, DLDef->Name);
	Changes[0] = MAKE_ID(WCS_PROJECTCLASS_PATHS, WCS_PATHS_DEFAULTDIR, 0, 0);
	Changes[1] = 0;
	HostProj->GenerateNotify(Changes);
	} // if

} // ProjectPrefsGUI::DoDefaultDir()

/*===========================================================================*/

void ProjectPrefsGUI::DoAdd(void)
{
struct DirList *DLNew;

if (DLNew = HostProj->DirList_Add(HostProj->DL, NULL, 0))
	{
	if (! HostProj->DL)
		HostProj->DL = DLNew;
	BuildList(HostProj->DirList_ItemExists(HostProj->DL, DLNew->Name));
	} // if

} // ProjectPrefsGUI::DoAdd()

/*===========================================================================*/

/*
void ProjectPrefsGUI::DoSwap(short EndIt)
{
struct DirList *DLItem1, *DLItem2;

if (EndIt)
	{
	DLItem1 = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem);
	ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
	DLItem2 = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem);
	if (DLItem1 && DLItem2)
		{
		swmem(&DLItem1->Read, &DLItem2->Read, 256);
		BuildList(ActiveItem);
		} // if
	SwapInProgress = 0;
#ifdef _WIN32
	if (Curse)
		SetClassLong(GetDlgItem(NativeWin, IDC_PARLIST), GCL_HCURSOR, (long)Curse);
#endif // _WIN32
	EndPointer();
	} // if
else
	{
	ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
#ifdef _WIN32
	Curse = (HCURSOR)SetClassLong(GetDlgItem(NativeWin, IDC_PARLIST), GCL_HCURSOR, NULL);
#endif // _WIN32
	//GoPointer(WCS_FENETRE_POINTER_SWAP);
	SwapInProgress = 1;
	} // else

} // ProjectPrefsGUI::DoSwap()
*/

/*===========================================================================*/

/*
void ProjectPrefsGUI::DoMove(short EndIt)
{
int MoveItem;
struct DirList *DLItem1, *DLItem2;

if (EndIt)
	{
	MoveItem = ActiveItem;
	DLItem1 = HostProj->DirList_Search(HostProj->DL, (short)MoveItem);
	ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
	DLItem2 = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem);
	if (DLItem1 && DLItem2)
		{
		HostProj->DirList_Move(HostProj->DL, (short)MoveItem, (short)ActiveItem);
		BuildList(ActiveItem);
		} // if
	MoveInProgress = 0;
#ifdef _WIN32
	if (Curse)
		SetClassLong(GetDlgItem(NativeWin, IDC_PARLIST), GCL_HCURSOR, (long)Curse);
#endif // _WIN32
	EndPointer();
	} // if
else
	{
	ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
#ifdef _WIN32
	Curse = (HCURSOR)SetClassLong(GetDlgItem(NativeWin, IDC_PARLIST), GCL_HCURSOR, NULL);
#endif // _WIN32
	//GoPointer(WCS_FENETRE_POINTER_SWAP);
	MoveInProgress = 1;
	} // else


} // ProjectPrefsGUI::DoMove()
*/

/*===========================================================================*/

void ProjectPrefsGUI::HandleMove(int Direction)
{
int MoveItem;
struct DirList *DLItem1, *DLItem2;

ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
if (Direction)
	{
	MoveItem = ActiveItem + 1;
	} // if
else
	{
	MoveItem = ActiveItem - 1;
	} // else
DLItem1 = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem);
DLItem2 = HostProj->DirList_Search(HostProj->DL, (short)MoveItem);
if (DLItem1 && DLItem2)
	{
	// this is scary! I stole it from DoSwap() but I dislike it much!
	swmem(&DLItem1->Read, &DLItem2->Read, 256);
	BuildList(MoveItem);
	} // if

} // ProjectPrefsGUI::HandleMove()


/*===========================================================================*/

void ProjectPrefsGUI::DoRemove(void)
{

ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
HostProj->DL = HostProj->DirList_Remove(HostProj->DL, (short)ActiveItem);
BuildList(ActiveItem);

} // ProjectPrefsGUI::DoRemove()

/*===========================================================================*/

void ProjectPrefsGUI::DoReadOnly(void)
{
int ReadOnly;
struct DirList *DLItem;

ReadOnly = WidgetGetCheck(IDC_READONLY) ? 1: 0;
ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
if (DLItem = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem))
	{
	if (ReadOnly)
		{
		DLItem->Read = '*';
		} // if
	else
		{
		DLItem->Read = ' ';
		} // else
	WidgetLBInsert(IDC_PARLIST, ActiveItem, &DLItem->Read);
	WidgetLBDelete(IDC_PARLIST, ActiveItem + 1);
	WidgetLBSetCurSel(IDC_PARLIST, ActiveItem);
	} // if

} // ProjectPrefsGUI::DoReadOnly()

/*===========================================================================*/

void ProjectPrefsGUI::DoLoad(void)
{

HostProj->Load(NULL, NULL, NULL, NULL, NULL,
	HostProj->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
	WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
	WCS_PROJECT_IODETAILTAG_GROUP, WCS_PROJECT_LOAD_PATHS_DIRLIST,
	WCS_PROJECT_IODETAILTAG_DONE),
	WCS_PROJECT_LOAD_PATHS_DIRLIST);

} // ProjectPrefsGUI::DoLoad()

/*===========================================================================*/

void ProjectPrefsGUI::DoActiveChange(void)
{
struct DirList *DLItem;

ActiveItem = WidgetLBGetCurSel(IDC_PARLIST);
if (DLItem = HostProj->DirList_Search(HostProj->DL, (short)ActiveItem))
	{
	WidgetSetCheck(IDC_READONLY, (DLItem->Read == '*'));
	} // if

} // ProjectPrefsGUI::DoActiveChange()

/*===========================================================================*/


void ProjectPrefsGUI::DoAdvConfig(int DoWhat)
{
int ActiveAdv;
char ActText[200], ActVal[80], *Temp;

switch (DoWhat)
	{
	case 0:
		{
		ActText[0] = NULL;
		ActiveAdv = WidgetLBGetCurSel(IDC_ADVLIST);
		WidgetLBGetText(IDC_ADVLIST, ActiveAdv, ActText);
		if (ActText[0])
			{
			if (Temp = strchr(ActText, ' '))
				{
				Temp[0] = NULL; // chop off arg part
				} // if
			if (Temp = strchr(ActText, '='))
				{
				Temp[0] = NULL; // chop off arg part
				} // if
			WidgetSetText(IDC_OPTNAME, ActText);
			if (Temp = GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt(ActText))
				{
				WidgetSetText(IDC_OPTVAL, Temp);
				} // if
			} // if
		break;
		} // 0: select
	case 1:
		{
		ActText[0] = ActVal[0] = NULL;
		WidgetGetText(IDC_OPTNAME, 50, ActText);
		WidgetGetText(IDC_OPTVAL, 75, ActVal);
		if (ActText[0])
			{
			GlobalApp->MainProj->Prefs.SetConfigOpt(ActText, ActVal);
			ConfigureWidgets();
			} // if
		break;
		} // 1: set
	case 2:
		{
		ActText[0] = ActVal[0] = NULL;
		WidgetGetText(IDC_OPTNAME, 50, ActText);
		if (ActText[0])
			{
			GlobalApp->MainProj->Prefs.RemoveConfigOpt(ActText);
			WidgetSetText(IDC_OPTNAME, "");
			WidgetSetText(IDC_OPTVAL, "");
			ConfigureWidgets();
			} // if
		break;
		} // 2: unset
	} // DoWhat

} // ProjectPrefsGUI::DoAdvConfig
