// DiagnosticGUI.cpp
// Code for Diagnostic Data Viewer
// Built from scratch on 11/17/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "DiagnosticGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "RasterAnimHost.h"
#include "Interactive.h"
#include "Useful.h"
#include "Render.h"
#include "Project.h"
#include "EffectsLib.h"
#include "resource.h"
#include "WCSVersion.h"
// these next two are to dock Diagnostics into S@G
#include "Conservatory.h"
#include "SceneViewGUI.h"

extern WCSApp *GlobalApp;
extern int ViewControlQualifier, ViewShiftQualifier;

NativeGUIWin DiagnosticGUI::Open(Project *Proj)
{
NativeGUIWin Success;

Success = GUIFenetre::Open(Proj);
return (Success);

} // DiagnosticGUI::Open

/*===========================================================================*/

NativeGUIWin DiagnosticGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_RENDER_DIAG, LocalWinSys()->RootWin, true);

	if(NativeWin)
		{
		SECURITY_INLINE_CHECK(046, 46);

/*
		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR2, 0, 100);
		WidgetSetScrollPos(IDC_SCROLLBAR1, 0);
		WidgetSetScrollPos(IDC_SCROLLBAR2, 100);
*/
		ConfigureWidgets(NULL);
		
		AddDiagnosticLine(IDC_RADIOCOMPONENT, IDC_COMP, IDI_DIAG_COMP, "Component");
		AddDiagnosticLine(IDC_RADIODISTANCE, IDC_DIST, IDI_DIAG_DIST, "Distance (Z)");
		AddDiagnosticLine(IDC_RADIOLATITUDE, IDC_LAT, IDI_DIAG_LAT, "Latitude");
		AddDiagnosticLine(IDC_RADIOLONGITUDE, IDC_LON, IDI_DIAG_LON, "Longitude");
		AddDiagnosticLine(IDC_RADIOELEVATION, IDC_ELEV, IDI_DIAG_ELEV, "Elevation");
		AddDiagnosticLine(IDC_RADIORELEL, IDC_RELEL, IDI_DIAG_RELEL, "Relative Elevation (RelEl)");
		AddDiagnosticLine(IDC_RADIOSLOPE, IDC_SLOPE, IDI_DIAG_SLOPE, "Slope");
		AddDiagnosticLine(IDC_RADIOASPECT, IDC_ASPECT, IDI_DIAG_ASPECT, "Aspect");
		AddDiagnosticLine(IDC_RADIOREFLECTION, IDC_REFL, IDI_DIAG_REFL, "Reflection");
		AddDiagnosticLine(IDC_RADIOILLUMINATON, IDC_SUNANG, IDI_DIAG_ILLUM, "Illumination");
		AddDiagnosticLine(IDC_RADIONORMAL, IDC_NORMAL, IDI_DIAG_NORM, "Normal XYZ");
		AddDiagnosticLine(IDC_RADIORGB, IDC_RGBHSV, IDI_DIAG_RGB, "RGB/HSV");
		AddDiagnosticLine(IDC_RADIOALPHA, IDC_ALPHA, IDI_DIAG_ALPHA, "Alpha");
		
		if(GlobalApp->GUIWins->SAG)
			{
			GlobalApp->GUIWins->SAG->AddLowerPanelTab("Diag", WCS_SCENEVIEWGUI_TAB_DIAG, NativeWin);
			} // if

		} // if

	} // if
 
return(NativeWin);
} // DiagnosticGUI::Construct

/*===========================================================================*/

DiagnosticGUI::DiagnosticGUI(void)
: GUIFenetre('DIAG', this, "Diagnostic Data") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, 0xff, 0xff, 0xff),
								0};

ConstructError = 0;
DisplayBuffer = WCS_DIAGNOSTIC_RGB;
Threshold[0] = 0;
Threshold[1] = 100;
NextDiagnosticLineY = 0;

DistanceADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
DistanceADT.SetMultiplier(1.0);
DistanceADT.SetIncrement(1.0);

ElevationADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ElevationADT.SetMultiplier(1.0);
ElevationADT.SetIncrement(1.0);

LatitudeADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
LatitudeADT.SetMultiplier(1.0);
LatitudeADT.SetIncrement(1.0);

LongitudeADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
LongitudeADT.SetMultiplier(1.0);
LongitudeADT.SetIncrement(1.0);

AspectADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AspectADT.SetMultiplier(1.0);
AspectADT.SetIncrement(1.0);

GlobalApp->AppEx->RegisterClient(this, AllEvents);
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_ISSUBDIALOG);

} // DiagnosticGUI::DiagnosticGUI

/*===========================================================================*/

DiagnosticGUI::~DiagnosticGUI()
{

GlobalApp->AppEx->RemoveClient(this);

} // DiagnosticGUI::~DiagnosticGUI()

/*===========================================================================*/

long DiagnosticGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_RDG, 0);

return(0);

} // DiagnosticGUI::HandleCloseWin

/*===========================================================================*/

long DiagnosticGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
NotifyTag Changes[2];

Changes[1] = 0;

SECURITY_INLINE_CHECK(090, 90);


switch(ButtonID)
	{
	case IDC_RADIOCOMPONENT:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_OBJECT;
		break;
		} //IDC_RADIOCOMPONENT
	case IDC_RADIODISTANCE:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_Z;
		break;
		} //IDC_RADIODISTANCE
	case IDC_RADIOLATITUDE:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_LATITUDE;
		break;
		} //IDC_RADIOLATITUDE
	case IDC_RADIOLONGITUDE:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_LONGITUDE;
		break;
		} //IDC_RADIOLONGITUDE
	case IDC_RADIOELEVATION:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_ELEVATION;
		break;
		} //IDC_RADIOELEVATION
	case IDC_RADIORELEL:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_RELEL;
		break;
		} //IDC_RADIORELEL
	case IDC_RADIOSLOPE:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_SLOPE;
		break;
		} //IDC_RADIOSLOPE
	case IDC_RADIOASPECT:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_ASPECT;
		break;
		} //IDC_RADIOASPECT
	case IDC_RADIOREFLECTION:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_REFLECTION;
		break;
		} //IDC_RADIOREFLECTION
	case IDC_RADIOILLUMINATON:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_ILLUMINATION;
		break;
		} //IDC_RADIOILLUMINATON
	case IDC_RADIORGB:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_RGB;
		break;
		} //IDC_RADIORGB
	case IDC_RADIOALPHA:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_ALPHA;
		break;
		} //IDC_RADIOALPHA
	case IDC_RADIONORMAL:
		{
		DisplayBuffer = WCS_DIAGNOSTIC_NORMALX;
		break;
		} //IDC_RADIONORMAL
	} // switch ButtonID

SyncButtonMutexes();


Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF, 0, DisplayBuffer);
GlobalApp->AppEx->GenerateNotify(Changes);

return(0);

} // DiagnosticGUI::HandleButtonClick

/*===========================================================================*/

long DiagnosticGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
NotifyTag Changes[2];
long rVal;

Changes[1] = 0;

if(ScrollCode == 1)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			Threshold[0] = (unsigned char)ScrollPos;
			break;
			}
		case IDC_SCROLLBAR2:
			{
			Threshold[1] = (unsigned char)ScrollPos;
			break;
			}
		default:
			break;
		} // switch
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THRESHOLD, Threshold[0], Threshold[1]);
	GlobalApp->AppEx->GenerateNotify(Changes);
	rVal = 0;
	} // HSCROLL
else
	rVal = 5;

return (rVal);

} // DiagnosticGUI::HandleScroll

/*===========================================================================*/

void DiagnosticGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	GlobalApp->GUIWins->SAG->SwitchToTab(WCS_SCENEVIEWGUI_TAB_DIAG);
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_UPDATETHRESHOLD, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	Threshold[0] = (unsigned char)((Changed & 0xff00) >> 8);
	Threshold[1] = (unsigned char)(Changed & 0xff);
	SyncScroll((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	} // if

} // DiagnosticGUI::HandleNotifyEvent()

/*===========================================================================*/

void DiagnosticGUI::ConfigureWidgets(DiagnosticData *Data)
{
double RGB[3], HSV[3];
#ifdef WCS_BUILD_VNS
CoordSys *DefCoords;
PlanetOpt *DefPlanetOpt;
#endif // WCS_BUILD_VNS
VertexDEM Vert;
char TextStr[256];


if (Data)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_Z] && Data->Value[WCS_DIAGNOSTIC_Z] < FLT_MAX)
		{
		FormatAsPreferredUnit(TextStr, &DistanceADT, Data->Value[WCS_DIAGNOSTIC_Z]);
		//sprintf(TextStr, "%.2fm", Data->Value[WCS_DIAGNOSTIC_Z]);
		WidgetSetText(IDC_DIST, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_DIST, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		FormatAsPreferredUnit(TextStr, &ElevationADT, Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		//sprintf(TextStr, "%.2fm", Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		WidgetSetText(IDC_ELEV, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_ELEV, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_RELEL])
		{
		sprintf(TextStr, "%.2f", Data->Value[WCS_DIAGNOSTIC_RELEL]);
		WidgetSetText(IDC_RELEL, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_RELEL, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_REFLECTION])
		{
		sprintf(TextStr, "%.1f%%", Data->Value[WCS_DIAGNOSTIC_REFLECTION] * 100.0);
		WidgetSetText(IDC_REFL, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_REFL, "Not Available");
		} // else

	#ifdef WCS_BUILD_VNS
	if (GlobalApp->MainProj->Prefs.DisplayGeoUnitsProjected && (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
		&& (DefCoords = DefPlanetOpt->Coords) && Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		{
		Vert.Lat = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
		Vert.Lon = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
		Vert.Elev = /*Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] ? Data->Value[WCS_DIAGNOSTIC_ELEVATION]:*/ 0.0;
		if (DefCoords->DefDegToProj(&Vert))
			{
			sprintf(TextStr, "%.3f", Vert.xyz[1]); 
			WidgetSetText(IDC_LAT, TextStr);
			sprintf(TextStr, "%.3f", Vert.xyz[0]); 
			WidgetSetText(IDC_LON, TextStr);
			} // if
		else
			{
			WidgetSetText(IDC_LAT, "Projection Error");
			WidgetSetText(IDC_LON, "Projection Error");
			} // else
		WidgetSetText(IDC_LATLABEL, "Northing");
		WidgetSetText(IDC_LONLABEL, "Easting");
		} // if
	else
	#endif // WCS_BUILD_VNS
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
			{
			FormatAsPreferredUnit(TextStr, &LatitudeADT, Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			//sprintf(TextStr, "%f deg", Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			WidgetSetText(IDC_LAT, TextStr);
			} // if
		else
			{
			WidgetSetText(IDC_LAT, "Not Available");
			} // else

		if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			FormatAsPreferredUnit(TextStr, &LongitudeADT, Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			//sprintf(TextStr, "%f deg", Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			WidgetSetText(IDC_LON, TextStr);
			} // if
		else
			{
			WidgetSetText(IDC_LON, "Not Available");
			} // else
		WidgetSetText(IDC_LATLABEL, "Latitude");
		WidgetSetText(IDC_LONLABEL, "Longitude");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_ASPECT])
		{
		FormatAsPreferredUnit(TextStr, &AspectADT, Data->Value[WCS_DIAGNOSTIC_ASPECT]);
		//sprintf(TextStr, "%.2f deg", Data->Value[WCS_DIAGNOSTIC_ASPECT]);
		WidgetSetText(IDC_ASPECT, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_ASPECT, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_SLOPE])
		{
		char TextStrPartial[256];
		FormatAsPreferredUnit(TextStrPartial, &AspectADT, Data->Value[WCS_DIAGNOSTIC_SLOPE]);
		if(Data->Value[WCS_DIAGNOSTIC_SLOPE] <= 89.0)
			{ // show it in percent too
			double SlopePercent = 100.0 * tan(Data->Value[WCS_DIAGNOSTIC_SLOPE] * PiOver180);
			sprintf(TextStr, "%s (%f%%)", TextStrPartial, SlopePercent);
			} // if
		else
			{
			strcpy(TextStr, TextStrPartial);
			} // else
		WidgetSetText(IDC_SLOPE, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_SLOPE, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_ILLUMINATION])
		{
		sprintf(TextStr, "%.1f%%", Data->Value[WCS_DIAGNOSTIC_ILLUMINATION] * 100.0);
		WidgetSetText(IDC_SUNANG, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_SUNANG, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX] && Data->ValueValid[WCS_DIAGNOSTIC_NORMALY] && Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ])
		{
		sprintf(TextStr, "%.3f, %.3f, %.3f", Data->Value[WCS_DIAGNOSTIC_NORMALX], Data->Value[WCS_DIAGNOSTIC_NORMALY], Data->Value[WCS_DIAGNOSTIC_NORMALZ]);
		WidgetSetText(IDC_NORMAL, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_NORMAL, "Not Available");
		} // else

	if (ViewControlQualifier && ViewShiftQualifier)
		{ // display X&Y
		sprintf(TextStr, "%d,%d", Data->PixelX, Data->PixelY);
		WidgetSetText(IDC_RGBHSV, TextStr);
		} // if
	else
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_RGB])
			{
			RGB[0] = Data->DataRGB[0] / 255.0;
			RGB[1] = Data->DataRGB[1] / 255.0;
			RGB[2] = Data->DataRGB[2] / 255.0;
			RGBtoHSV(HSV, RGB);
			sprintf(TextStr, "%d,%d,%d / %d,%d,%d", Data->DataRGB[0], Data->DataRGB[1], Data->DataRGB[2], (long)HSV[0], (long)HSV[1], (long)HSV[2]);
			WidgetSetText(IDC_RGBHSV, TextStr);
			} // if
		else
			{
			WidgetSetText(IDC_RGBHSV, "Not Available");
			} // else
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_ALPHA])
		{
		sprintf(TextStr, "%d", Data->Alpha);
		WidgetSetText(IDC_ALPHA, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_ALPHA, "Not Available");
		} // else

	if (Data->ValueValid[WCS_DIAGNOSTIC_OBJECT])
		{
		if (Data->Object->GetRAHostName())
			sprintf(TextStr, "%s", Data->Object->GetRAHostName());
		else
			strcpy(TextStr, "Unnamed");
		WidgetSetText(IDC_COMP, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_COMP, "Not Available");
		} // else
	WidgetSetDisabled(IDC_RADIODISTANCE, ! Data->ValueValid[WCS_DIAGNOSTIC_Z]);
	WidgetSetDisabled(IDC_RADIOELEVATION, ! Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION]);
	WidgetSetDisabled(IDC_RADIOREFLECTION, ! Data->ValueValid[WCS_DIAGNOSTIC_REFLECTION]);
	WidgetSetDisabled(IDC_RADIOLATITUDE, ! Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE]);
	WidgetSetDisabled(IDC_RADIOLONGITUDE, ! Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE]);
	WidgetSetDisabled(IDC_RADIOASPECT, ! Data->ValueValid[WCS_DIAGNOSTIC_ASPECT]);
	WidgetSetDisabled(IDC_RADIOSLOPE, ! Data->ValueValid[WCS_DIAGNOSTIC_SLOPE]);
	WidgetSetDisabled(IDC_RADIOILLUMINATON, ! Data->ValueValid[WCS_DIAGNOSTIC_ILLUMINATION]);
	WidgetSetDisabled(IDC_RADIORGB, ! Data->ValueValid[WCS_DIAGNOSTIC_RGB]);
	WidgetSetDisabled(IDC_RADIOCOMPONENT, ! Data->ValueValid[WCS_DIAGNOSTIC_OBJECT]);
	WidgetSetDisabled(IDC_RADIORELEL, ! Data->ValueValid[WCS_DIAGNOSTIC_RELEL]);
	WidgetSetDisabled(IDC_RADIOALPHA, ! Data->ValueValid[WCS_DIAGNOSTIC_ALPHA]);
	WidgetSetDisabled(IDC_RADIONORMAL, ! (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX] && Data->ValueValid[WCS_DIAGNOSTIC_NORMALY] && Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ]));
	DisplayBuffer = Data->DisplayedBuffer;
	SyncButtonMutexes();
	/*
	WidgetSetDisabled(IDC_SCROLLBAR1, ! Data->ThresholdValid);
	WidgetSetDisabled(IDC_SCROLLBAR2, ! Data->ThresholdValid);
*/
	} // if
else
	{
	WidgetSetDisabled(IDC_RADIODISTANCE, 1);
	WidgetSetDisabled(IDC_RADIOELEVATION, 1);
	WidgetSetDisabled(IDC_RADIOREFLECTION, 1);
	WidgetSetDisabled(IDC_RADIOLATITUDE, 1);
	WidgetSetDisabled(IDC_RADIOLONGITUDE, 1);
	WidgetSetDisabled(IDC_RADIOASPECT, 1);
	WidgetSetDisabled(IDC_RADIOSLOPE, 1);
	WidgetSetDisabled(IDC_RADIOILLUMINATON, 1);
	WidgetSetDisabled(IDC_RADIORGB, 1);
	WidgetSetDisabled(IDC_RADIOCOMPONENT, 1);
	WidgetSetDisabled(IDC_RADIORELEL, 1);
	WidgetSetDisabled(IDC_RADIOALPHA, 1);
	WidgetSetDisabled(IDC_RADIONORMAL, 1);
	WidgetSetDisabled(IDC_SCROLLBAR1, 1);
	WidgetSetDisabled(IDC_SCROLLBAR2, 1);
	} // else

} // DiagnosticGUI::ConfigureWidgets()

/*===========================================================================*/

void DiagnosticGUI::SyncScroll(DiagnosticData *Data)
{

WidgetSetScrollPos(IDC_SCROLLBAR1, Threshold[0]);
WidgetSetScrollPos(IDC_SCROLLBAR2, Threshold[1]);
WidgetSetDisabled(IDC_SCROLLBAR1, ! Data->ThresholdValid);
WidgetSetDisabled(IDC_SCROLLBAR2, ! Data->ThresholdValid);

} // DiagnosticGUI::SyncScroll

void DiagnosticGUI::SyncButtonMutexes(void)
{

// enforce radio-button style mutexing, since we're misusing icon toolbuttons as a radiobutton set
WidgetSetCheck(IDC_RADIOCOMPONENT, DisplayBuffer == WCS_DIAGNOSTIC_OBJECT);
WidgetSetCheck(IDC_RADIODISTANCE, DisplayBuffer == WCS_DIAGNOSTIC_Z);
WidgetSetCheck(IDC_RADIOLATITUDE, DisplayBuffer == WCS_DIAGNOSTIC_LATITUDE);
WidgetSetCheck(IDC_RADIOLONGITUDE, DisplayBuffer == WCS_DIAGNOSTIC_LONGITUDE);
WidgetSetCheck(IDC_RADIOELEVATION, DisplayBuffer == WCS_DIAGNOSTIC_ELEVATION);
WidgetSetCheck(IDC_RADIORELEL, DisplayBuffer == WCS_DIAGNOSTIC_RELEL);
WidgetSetCheck(IDC_RADIOSLOPE, DisplayBuffer == WCS_DIAGNOSTIC_SLOPE);
WidgetSetCheck(IDC_RADIOASPECT, DisplayBuffer == WCS_DIAGNOSTIC_ASPECT);
WidgetSetCheck(IDC_RADIOREFLECTION, DisplayBuffer == WCS_DIAGNOSTIC_REFLECTION);
WidgetSetCheck(IDC_RADIOILLUMINATON, DisplayBuffer == WCS_DIAGNOSTIC_ILLUMINATION);
WidgetSetCheck(IDC_RADIORGB, DisplayBuffer == WCS_DIAGNOSTIC_RGB);
WidgetSetCheck(IDC_RADIOALPHA, DisplayBuffer == WCS_DIAGNOSTIC_ALPHA);
WidgetSetCheck(IDC_RADIONORMAL, DisplayBuffer == WCS_DIAGNOSTIC_NORMALX);

} // DiagnosticGUI::SyncButtonMutexes


int DiagnosticGUI::AddDiagnosticLine(int ButtonID, int FieldID, int IconID, char *IconCaption)
{
HWND Button, Field;
unsigned long int WStyle = WS_VISIBLE | WS_CHILD | WCSW_TB_STYLE_NOFOCUS | WCSW_TB_STYLE_TOG | WCSW_TB_STYLE_XPLOOK;

// create field
if(Field = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
 20, NextDiagnosticLineY, 140, 20, NativeWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	SetWindowLong(Field, GWL_ID, FieldID);
	SendMessage(Field, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);
	// create button
	if(Button = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", IconCaption, WStyle,
	 0, NextDiagnosticLineY + 2, 16, 16, NativeWin, NULL, LocalWinSys()->Instance(), NULL))
		{
		// configure button
		SetWindowLong(Button, GWL_ID, ButtonID);
		ConfigureTB(NativeWin, ButtonID, IconID, NULL);
		SendMessage(Button, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);
		SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		NextDiagnosticLineY += 22;
		return(1);
		} // if
	} // if

return(0);
} // DiagnosticGUI::AddDiagnosticLine
