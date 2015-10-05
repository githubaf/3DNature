// SunPosGUI.cpp
// Code for Wave editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "SunPosGUI.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "ProjectDispatch.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Interactive.h"
#include "resource.h"


NativeGUIWin SunPosGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SunPosGUI::Open

/*===========================================================================*/

NativeGUIWin SunPosGUI::Construct(void)
{
static const char *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
int ListEntry;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_SUN_POS, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		for (ListEntry=0; ListEntry<12; ListEntry++)
			WidgetCBInsert(IDC_MONTHDROP, -1, Months[ListEntry]);

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // SunPosGUI::Construct

/*===========================================================================*/

SunPosGUI::SunPosGUI(Project *ProjSource, Light *ActiveSource)
: GUIFenetre('EPTS', this, "Light Position by Time") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
double RangeDefaults[3] = {FLT_MAX, -FLT_MAX, .0001};

ConstructError = 0;
Active = ActiveSource;
AMPM = 0;
SunDate = 1;
RefLon.SetRangeDefaults(RangeDefaults);
RefLon.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);

if (ProjSource && ActiveSource)
	{
	RefLon.SetValue(ProjSource->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&BackupLat, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
	Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&BackupLon, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // SunPosGUI::SunPosGUI

/*===========================================================================*/

SunPosGUI::~SunPosGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // SunPosGUI::~SunPosGUI()

/*===========================================================================*/

long SunPosGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SPG, 0);

return(0);

} // SunPosGUI::HandleCloseWin

/*===========================================================================*/

long SunPosGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(027, 27);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SPG, 0);
		break;
		} // 
	case IDCANCEL:
		{
		Cancel();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SPG, 0);
		break;
		} // 
	case ID_REVERSE:
		{
		ReverseSeasons();
		break;
		} // reverse seasons
	default: break;
	} // ButtonID

return(0);

} // SunPosGUI::HandleButtonClick

/*===========================================================================*/

long SunPosGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_TIME:
		{
		TimeChange();
		break;
		}
	} // switch CtrlID

return (0);

} // SunPosGUI::HandleStringLoseFocus

/*===========================================================================*/

long SunPosGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_MONTHDROP:
		{
		MonthDateChange();
		break;
		} // date
	} // switch CtrlID

return (0);

} // SunPosGUI::HandleCBChange

/*===========================================================================*/

long SunPosGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RADIOAM:
	case IDC_RADIOPM:
		{
		SetPosition();
		break;
		} // 
	} // switch CtrlID

return(0);

} // SunPosGUI::HandleSRChange

/*===========================================================================*/

long SunPosGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SUNDATE:
		{
		MonthDateChange();
		break;
		} // date
	case IDC_REFLON:
		{
		ChangeRefLon();
		break;
		} // 
	} // switch CtrlID

return(0);

} // SunPosGUI::HandleFIChange

/*===========================================================================*/

void SunPosGUI::HandleNotifyEvent(void)
{

ConfigureWidgets();

} // SunPosGUI::HandleNotifyEvent()

/*===========================================================================*/

void SunPosGUI::ConfigureWidgets(void)
{

WidgetSetText(IDC_NAMETXT, Active->GetName());

ConfigureSR(NativeWin, IDC_RADIOAM, IDC_RADIOAM, &AMPM, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAM, IDC_RADIOPM, &AMPM, SRFlag_Char, 1, NULL, NULL);

//WidgetSNConfig(IDC_LIGHTLAT, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
//WidgetSNConfig(IDC_LIGHTLON, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
WidgetSNConfig(IDC_REFLON, &RefLon);
WidgetSmartRAHConfig(IDC_LIGHTLAT, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], Active);
WidgetSmartRAHConfig(IDC_LIGHTLON, &Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], Active);

ConfigureFI(NativeWin, IDC_SUNDATE, &SunDate, 1.0, 0.0, 32.0, FIOFlag_Char, NULL, NULL);

SetReverse(0);	// F2 NOTE: Argument is never used

} // SunPosGUI::ConfigureWidgets()

/*===========================================================================*/

void SunPosGUI::ReverseSeasons(void)
{

Active->FallSeason = 1 - Active->FallSeason;
SetReverse(1);	// F2 NOTE: Argument is never used

} // SunPosGUI::ReverseSeasons()

/*===========================================================================*/

void SunPosGUI::ChangeRefLon(void)
{

SetReverse(2);	// F2 NOTE: Argument is never used

} // SunPosGUI::ChangeRefLon()

/*===========================================================================*/

void SunPosGUI::TimeChange(void)
{

if (WidgetGetModified(IDC_TIME))
	SetPosition();

} // SunPosGUI::TimeChange()

/*===========================================================================*/

void SunPosGUI::MonthDateChange(void)
{
long SunMonth;

SunMonth = WidgetCBGetCurSel(IDC_MONTHDROP);

if ((SunMonth == 11 && SunDate - 1 > 19) || (SunMonth == 5 && SunDate - 1 < 20)
	|| (SunMonth < 5))
	Active->FallSeason = 0;
else
	Active->FallSeason = 1;

SetPosition();

} // SunPosGUI::MonthDateChange

/*===========================================================================*/

// if you change this code, change SunTimeString also
void SunPosGUI::SetReverse(int Reverse)	// F2 NOTE: Reverse parameter isn't used
{
double FloatDays, SunTime;
long SunMonth;
short Hour, Mins, Days;
char Str[24];

if (Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue > 23.0)
	{
	SunMonth = 5;
	SunDate = 20;
	}
else if (Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue < -23)
	{
	SunMonth = 11;
	SunDate = 19;
	}
else
	{
	FloatDays = .5 + acos(-(Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue / 23.0)) * PiUnder180 * 365.0 / 360.0; 

	SunMonth = WidgetCBGetCurSel(IDC_MONTHDROP);	// F2 NOTE: this appears to be unneeded

	if (Active->FallSeason)
		FloatDays = 365.0 - (int)FloatDays;

	Days = (short)FloatDays;
	if (Days < 12)
		{
		SunMonth = 11;
		SunDate = 19 + Days;
		}
	else if (Days < 43)
		{
		SunMonth = 0;
		SunDate = Days - 12;
		}
	else if (Days < 71)
		{
		SunMonth = 1;
		SunDate = Days - 43;
		}
	else if (Days < 102)
		{
		SunMonth = 2;
		SunDate = Days - 71;
		}
	else if (Days < 132)
		{
		SunMonth = 3;
		SunDate = Days - 102;
		}
	else if (Days < 163)
		{
		SunMonth = 4;
		SunDate = Days - 132;
		}
	else if (Days < 193)
		{
		SunMonth = 5;
		SunDate = Days - 163;
		}
	else if (Days < 224)
		{
		SunMonth = 6;
		SunDate = Days - 193;
		}
	else if (Days < 255)
		{
		SunMonth = 7;
		SunDate = Days - 224;
		}
	else if (Days < 285)
		{
		SunMonth = 8;
		SunDate = Days - 255;
		}
	else if (Days < 316)
		{
		SunMonth = 9;
		SunDate = Days - 285;
		}
	else if (Days < 346)
		{
		SunMonth = 10;
		SunDate = Days - 316;
		}
	else
		{
		SunMonth = 11;
		SunDate = Days - 346;
		} /* else */
	} /* else */
SunDate += 1;	// for display in float int

SunTime = 12.0 + (Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue - RefLon.CurValue) * 12.0 / 180.0;
while (SunTime < 0.0)
	SunTime += 24.0;
while (SunTime > 24.0)
	SunTime -= 24.0;
Hour = (short)SunTime;
Mins = (short)((SunTime - Hour) * 60 + .5);
if (Mins == 60)
	{
	Mins = 0;
	Hour++;
	}
if (Hour >= 12)
	{
	Hour -= 12;
	AMPM = 1;
	} /* if */
else
	AMPM = 0;
if (Hour == 0)
	Hour = 12;

sprintf(Str, "%1d:%02d", Hour, Mins);

WidgetCBSetCurSel(IDC_MONTHDROP, SunMonth);
WidgetFISync(IDC_SUNDATE, WP_FISYNC_NONOTIFY);
// <<<>>> for some reason WidgetSRSync does not update these two widgets as it should
//WidgetSRSync(IDC_RADIOAM, WP_SRSYNC_NONOTIFY);
//WidgetSRSync(IDC_RADIOPM, WP_SRSYNC_NONOTIFY);
// so we use ConfigureSR instead
ConfigureSR(NativeWin, IDC_RADIOAM, IDC_RADIOAM, &AMPM, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAM, IDC_RADIOPM, &AMPM, SRFlag_Char, 1, NULL, NULL);
WidgetSetText(IDC_TIME, Str);
WidgetSetModified(IDC_TIME, 0);

} // SunPosGUI::SetReverse()

/*===========================================================================*/

void SunPosGUI::SetPosition(void)
{
double SunLon, SunLat, SunTime, Days = 0;
long SunMonth;
short Pos = 0, Hour, Mins;
#ifdef _WIN32
char TimeStr[24];
#endif // _WIN32

SunMonth = WidgetCBGetCurSel(IDC_MONTHDROP);
WidgetGetText(IDC_TIME, 24, TimeStr);

while (TimeStr[Pos] != '.' && TimeStr[Pos] != ':' && TimeStr[Pos] != 0)
	Pos ++;
Hour = atoi(TimeStr);

if (Hour == 12 && ! AMPM)
	Hour = 0;
else if (Hour < 12 && AMPM)
	Hour += 12;
 
if (Pos < (short)strlen(TimeStr))
	Mins = atoi(&TimeStr[Pos + 1]);
else
	Mins = 0;

switch (SunMonth)
	{
	case 0:
		{
		Days = 12;
		break;
		}
	case 1:
		{
		Days = 43;
		break;
		}
	case 2:
		{
		Days = 71;
		break;
		}
	case 3:
		{
		Days = 102;
		break;
		}
	case 4:
		{
		Days = 132;
		break;
		}
	case 5:
		{
		Days = 163;
		break;
		}
	case 6:
		{
		Days = 193;
		break;
		}
	case 7:
		{
		Days = 224;
		break;
		}
	case 8:
		{
		Days = 255;
		break;
		}
	case 9:
		{
		Days = 285;
		break;
		}
	case 10:
		{
		Days = 316;
		break;
		}
	case 11:
		{
		Days = 346;
		break;
		}
	} // switch

Days = (short)(Days + SunDate - 1);
if (Days >= 365.0)
	Days -= 365.0;
if (Days < 0.0)
	Days += 365.0;

SunTime = Hour + (double)Mins / 60.0;

//Days += SunTime / 24.0;
//if (AMPM)
//	Days += .5;

SunTime -= 12.0;

SunLat = -23.0 * cos((360.0 * Days / 365.0) * PiOver180); 

SunLon = RefLon.CurValue + 180.0 * SunTime / 12.0;

Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue = SunLon;
Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(SunLat);

} // SunPosGUI::SetPosition()

/*===========================================================================*/

void SunPosGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &BackupLat);
Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&Active->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &BackupLon);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // SunPosGUI::Cancel()

/*===========================================================================*/
