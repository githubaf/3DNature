// CoordSysEditGUI.cpp
// Code for RasterTA editor
// Built from scratch on 12/28/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CoordSysEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Conservatory.h"
#include "CoordsCalculatorGUI.h"
#include "resource.h"
#include "CoordSys.h"
#include "Lists.h"

char *CoordSysEditGUI::TabNames[WCS_COORDSYSGUI_NUMTABS] = {"General", "System", "Method", "Datum", "Ellipsoid"};

long CoordSysEditGUI::ActivePage;
// advanced
long CoordSysEditGUI::DisplayAdvanced;

NativeGUIWin CoordSysEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CoordSysEditGUI::Open

/*===========================================================================*/

NativeGUIWin CoordSysEditGUI::Construct(void)
{
int TabIndex;
long NameFieldNum;
char Str[256];

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_COORDSYS_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_COORDSYS_PROJSYS, 0, 1);
	CreateSubWinFromTemplate(IDD_COORDSYS_PROJMETHOD, 0, 2);
	CreateSubWinFromTemplate(IDD_COORDSYS_DATUM, 0, 3);
	CreateSubWinFromTemplate(IDD_COORDSYS_ELLIPSOID, 0, 4);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_COORDSYSGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		if ((NameFieldNum = CoordSys::ProjSysTable.FindFieldByName("NAME")) >= 0)
			{
			TabIndex = 0;
			while (CoordSys::ProjSysTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
				{
				WidgetCBInsert(IDC_PROJSYSDROP, TabIndex, Str);
				TabIndex ++;
				} // while
			} // if field number found
		if ((NameFieldNum = ProjectionMethod::MethodTable.FindFieldByName("NAME")) >= 0)
			{
			TabIndex = 0;
			while (ProjectionMethod::MethodTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
				{
				WidgetCBInsert(IDC_METHODDROP, TabIndex, Str);
				TabIndex ++;
				} // while
			} // if field number found
		if ((NameFieldNum = GeoDatum::DatmTable.FindFieldByName("NAME")) >= 0)
			{
			TabIndex = 0;
			while (GeoDatum::DatmTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
				{
				WidgetCBInsert(IDC_DATUMDROP, TabIndex, Str);
				TabIndex ++;
				} // while
			} // if field number found
		if ((NameFieldNum = GeoEllipsoid::EllipseTable.FindFieldByName("NAME")) >= 0)
			{
			TabIndex = 0;
			while (GeoEllipsoid::EllipseTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
				{
				WidgetCBInsert(IDC_ELLIPSOIDDROP, TabIndex, Str);
				TabIndex ++;
				} // while
			} // if field number found
		FillZoneDrop();
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // CoordSysEditGUI::Construct

/*===========================================================================*/

CoordSysEditGUI::CoordSysEditGUI(EffectsLib *EffectsSource, Database *DBSource, CoordSys *ActiveSource, int ModalSource)
: GUIFenetre('COSY', this, "Coordinate System Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];
//VertexDEM Vert;

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
EccentricitySq = InvFlattening = 0.0;
if (GoneModal = ModalSource)
	{
	// prevent docking if a modal source
	SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
	GoModal();
	} // if

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Coordinate System Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	// testing
	//Vert.Lat = 45.0;
	//Vert.Lon = 106.0;
	//Vert.Elev = 245.0;
	//Active->DegToProj(&Vert);
	//Active->ProjToCart(&Vert);
	//Active->CartToProj(&Vert);
	//Active->ProjToDeg(&Vert);
	//printf("%f %f %f", Vert.Lat, Vert.Lon, Vert.Elev);
	} // if
else
	ConstructError = 1;

} // CoordSysEditGUI::CoordSysEditGUI

/*===========================================================================*/

CoordSysEditGUI::~CoordSysEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (GoneModal)
	EndModal();

} // CoordSysEditGUI::~CoordSysEditGUI()

/*===========================================================================*/

long CoordSysEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_COS, 0);

return(0);

} // CoordSysEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long CoordSysEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // CoordSysEditGUI::HandleShowAdvanced

/*===========================================================================*/

long CoordSysEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_COS, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		Active->Method.OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		Active->Method.OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_LOADCOMPONENT2:
		{
		Active->Datum.OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT2:
		{
		Active->Datum.OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_LOADCOMPONENT3:
		{
		Active->Datum.Ellipse.OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT3:
		{
		Active->Datum.Ellipse.OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_PROJSYSLIST:
		{
		Active->OpenTableList(0, GoneModal);
		break;
		} //
	case IDC_LOADFROMFILE:
		{
		FileReq PRJfile;
		PRJfile.SetDefPat(WCS_REQUESTER_PARTIALWILD("prj"));
		if(PRJfile.Request(NULL))
			{
			CSLoadInfo *CSLI = NULL;

			if (CSLI = Active->LoadFromArcPrj((char *)PRJfile.GetFirstName()))
				{
				delete CSLI;
				CSLI = NULL;
				} // if

			// ensure everyone hears about the change for sure
			NotifyTag Changes[2];
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

			// probably redundent after the above, but to be safe
			ConfigureWidgets();
			} // if
		break;
		} //
	case IDC_ZONELIST:
		{
		Active->OpenTableList(1, GoneModal);
		break;
		} //
	case IDC_METHODLIST:
		{
		Active->OpenTableList(2, GoneModal);
		break;
		} //
	case IDC_DATUMLIST:
		{
		Active->OpenTableList(3, GoneModal);
		break;
		} //
	case IDC_ELLIPSOIDLIST:
		{
		Active->OpenTableList(4, GoneModal);
		break;
		} //
	case IDC_DATUMCALC:
		{
		#ifdef WCS_ENABLE_COORDSCALC
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1: 0)
			OpenCoordsCalculator();
		else
		#endif // WCS_ENABLE_COORDSCALC
		{
		Active->DatumDeltaCalculator();
		// set datum and system to custom
		SelectNewDatum(0);
		} // else
		break;
		} //
	default:
		break;
	} // ButtonID

return(0);

} // CoordSysEditGUI::HandleButtonClick

/*===========================================================================*/

long CoordSysEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // CoordSysEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long CoordSysEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

Active->Initialized = 0;

switch (CtrlID)
	{
	case IDC_PROJSYSDROP:
		{
		SelectNewSystem(-1);
		break;
		}
	case IDC_ZONEDROP:
		{
		SelectNewZone();
		break;
		}
	case IDC_METHODDROP:
		{
		SelectNewMethod();
		break;
		}
	case IDC_DATUMDROP:
		{
		SelectNewDatum(-1);
		break;
		}
	case IDC_ELLIPSOIDDROP:
		{
		SelectNewEllipsoid(-1);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // CoordSysEditGUI::HandleCBChange

/*===========================================================================*/

long CoordSysEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // CoordSysEditGUI::HandlePageChange

/*===========================================================================*/

long CoordSysEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

// this may be a too-late reaction if notifications from smart numerics happen first
// but with delayed redraws maybe OK.
Active->Initialized = 0;
Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_DELTAX:
	case IDC_DELTAY:
	case IDC_DELTAZ:
		{
		// set datum and system to custom
		SelectNewDatum(0);
		break;
		} // 
	case IDC_ECCENTRICITY:
		{
		ComputeFromEccSq();
		// set ellipsoid, datum and system to custom
		SelectNewEllipsoid(0);
		break;
		} // 
	case IDC_INVFLATTENING:
		{
		ComputeFromInvFlattening();
		// set ellipsoid, datum and system to custom
		SelectNewEllipsoid(0);
		break;
		} // 
	case IDC_MAJORAXIS:
		{
		if (Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue > 
			Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue)
			{
			Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue);
			Changes[0] = MAKE_ID(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetRAHostRoot());
			// generate notify
			} // if
		ComputeEF();
		// set ellipsoid, datum and system to custom
		SelectNewEllipsoid(0);
		SyncWidgets();
		break;
		} // 
	case IDC_MINORAXIS:
		{
		if (Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue > 
			Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue)
			{
			Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue);
			// generate notify
			Changes[0] = MAKE_ID(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].GetRAHostRoot());
			} // if
		ComputeEF();
		// set ellipsoid, datum and system to custom
		SelectNewEllipsoid(0);
		SyncWidgets();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // CoordSysEditGUI::HandleFIChange

/*===========================================================================*/

void CoordSysEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;
#ifdef WCS_BUILD_VNS
PlanetOpt *DefOpt;
#endif // WCS_BUILD_VNS

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

if (! Done)
	{
	ConfigureWidgets();
	} // if

#ifdef WCS_BUILD_VNS
Active->Initialized = 0;
if ((DefOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	&& DefOpt->Coords == Active)
	EffectsHost->UpdateDefaultCoords(DefOpt->Coords, TRUE);
else
	Active->UpdateJoeBounds();
#endif // WCS_BUILD_VNS

} // CoordSysEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void CoordSysEditGUI::ConfigureWidgets(void)
{
char TextStr[256], DatumStr[1024], BigStr[2048], DatumAdded;
long NumJoes, UsageFieldNum, BigStrLen, LocalLen;
double DummyDbl;
JoeList *CurJoe;
ImageList *CurImage;

if (CurJoe = Active->Joes)
	{
	NumJoes = 1;
	while (CurJoe->Next)
		{
		NumJoes ++;
		CurJoe = CurJoe->Next;
		} // while
	if (NumJoes > 1)
		sprintf(TextStr, "There are %d DB Objects attached to this Coordinate System.", NumJoes);
	else
		strcpy(TextStr, "There is one DB Object attached to this Coordinate System.");
	} // if
else
	strcpy(TextStr, "There are no DB Objects attached to this Coordinate System.");
WidgetSetText(IDC_JOESEXIST, TextStr);

if (CurImage = Active->Images)
	{
	NumJoes = 1;
	while (CurImage->Next)
		{
		NumJoes ++;
		CurImage = CurImage->Next;
		} // while
	if (NumJoes > 1)
		sprintf(TextStr, "There are %d Image Objects attached to this Coordinate System.", NumJoes);
	else
		strcpy(TextStr, "There is one Image Object attached to this Coordinate System.");
	} // if
else
	strcpy(TextStr, "There are no Image Objects attached to this Coordinate System.");
WidgetSetText(IDC_IMAGESEXIST, TextStr);

sprintf(TextStr, "Coordinate System Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

WidgetSetText(IDC_PROJSYSTXT, Active->ProjSysName);
WidgetSetText(IDC_ZONETXT, Active->ZoneName);
WidgetSetText(IDC_METHODTXT, Active->Method.MethodName);
WidgetSetText(IDC_DATUMTXT, Active->Datum.DatumName);
WidgetSetText(IDC_ELLIPSOIDTXT, Active->Datum.Ellipse.EllipsoidName);

BigStr[0] = 0;
TextStr[0] = 0;
BigStrLen = 0;
if ((UsageFieldNum = CoordSys::ProjSysTable.FindFieldByName("USAGE")) >= 0)
	{
	if (CoordSys::ProjSysTable.FetchFieldValueStr(Active->ProjSysID, UsageFieldNum, TextStr, 256))
		{
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			strcat(BigStr, "System: ");
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
WidgetSetText(IDC_PROJSYSUSAGETXT, TextStr);

TextStr[0] = 0;
if ((UsageFieldNum = Active->ZoneTable.FindFieldByName("USAGE")) >= 0)
	{
	if (Active->ZoneTable.FetchFieldValueStr(Active->ZoneID, UsageFieldNum, TextStr, 256))
		{
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			strcat(BigStr, "Zone: ");
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
WidgetSetText(IDC_ZONEUSAGETXT, TextStr);

TextStr[0] = 0;
if ((UsageFieldNum = ProjectionMethod::MethodTable.FindFieldByName("USAGE")) >= 0)
	{
	if (ProjectionMethod::MethodTable.FetchFieldValueStr(Active->Method.MethodID, UsageFieldNum, TextStr, 256))
		{
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			strcat(BigStr, "Method: ");
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
WidgetSetText(IDC_METHODUSAGETXT, TextStr);

DatumStr[0] = 0;
DatumAdded = 0;
if ((UsageFieldNum = GeoDatum::DatmTable.FindFieldByName("AREA_USE")) >= 0)
	{
	if (GeoDatum::DatmTable.FetchFieldValueStr(Active->Datum.DatumID, UsageFieldNum, TextStr, 256))
		{
		strcat(DatumStr, TextStr);
		strcat(DatumStr, "\r\n");
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			if (! DatumAdded)
				{
				strcat(BigStr, "Datum: ");
				DatumAdded = 1;
				} // if
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if
if ((UsageFieldNum = GeoDatum::DatmTable.FindFieldByName("ORIGIN_DES")) >= 0)
	{
	if (GeoDatum::DatmTable.FetchFieldValueStr(Active->Datum.DatumID, UsageFieldNum, TextStr, 256))
		{
		strcat(DatumStr, TextStr);
		strcat(DatumStr, "\r\n");
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			if (! DatumAdded)
				{
				strcat(BigStr, "Datum: ");
				DatumAdded = 1;
				} // if
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if
if ((UsageFieldNum = GeoDatum::DatmTable.FindFieldByName("REMARKS")) >= 0)
	{
	if (GeoDatum::DatmTable.FetchFieldValueStr(Active->Datum.DatumID, UsageFieldNum, TextStr, 256))
		{
		strcat(DatumStr, TextStr);
		strcat(DatumStr, "\r\n");
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			if (! DatumAdded)
				{
				strcat(BigStr, "Datum: ");
				DatumAdded = 1;
				} // if
			strcat(BigStr, TextStr);
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
if ((UsageFieldNum = GeoDatum::DatmTable.FindFieldByName("DELTA_X")) >= 0)
	{
	if (! GeoDatum::DatmTable.FetchFieldValueDbl(Active->Datum.DatumID, UsageFieldNum, DummyDbl))
		{
		strcat(DatumStr, "Delta X, Y and Z are not known for this Datum.");
		strcat(DatumStr, "\r\n");
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			if (! DatumAdded)
				{
				strcat(BigStr, "Datum: ");
				DatumAdded = 1;
				} // if
			strcat(BigStr, "Delta X, Y and Z are not known for this Datum.");
			strcat(BigStr, "\r\n");
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
WidgetSetText(IDC_DATUMUSAGETXT, DatumStr);

TextStr[0] = 0;
if ((UsageFieldNum = GeoEllipsoid::EllipseTable.FindFieldByName("USAGE")) >= 0)
	{
	if (GeoEllipsoid::EllipseTable.FetchFieldValueStr(Active->Datum.Ellipse.EllipsoidID, UsageFieldNum, TextStr, 256))
		{
		if ((LocalLen = (long)strlen(TextStr)) + BigStrLen < 2048)
			{
			strcat(BigStr, "Ellipsoid: ");
			strcat(BigStr, TextStr);
			BigStrLen += LocalLen;
			} // if
		} // if
	} // if field number found
WidgetSetText(IDC_ELLIPSOIDUSAGETXT, TextStr);
WidgetSetText(IDC_SUMMARYTXT, BigStr);

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT2, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT2, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT3, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT3, IDI_FILESAVE, NULL);

WidgetCBSetCurSel(IDC_PROJSYSDROP, Active->ProjSysID);
WidgetCBSetCurSel(IDC_ZONEDROP, Active->ZoneID);
WidgetCBSetCurSel(IDC_METHODDROP, Active->Method.MethodID);
WidgetCBSetCurSel(IDC_DATUMDROP, Active->Datum.DatumID);
WidgetCBSetCurSel(IDC_ELLIPSOIDDROP, Active->Datum.Ellipse.EllipsoidID);

if (Active->Method.ParamID[0] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM1, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM1]);
	WidgetSmartRAHConfig(IDC_PARAM1, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM1], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM1]);
	WidgetSetText(IDC_PARAM1, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM1, "");
	//WidgetSNConfig(IDC_PARAM1, NULL);
	WidgetSmartRAHConfig(IDC_PARAM1, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[1] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM2, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM2]);
	WidgetSmartRAHConfig(IDC_PARAM2, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM2], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM2]);
	WidgetSetText(IDC_PARAM2, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM2, "");
	//WidgetSNConfig(IDC_PARAM2, NULL);
	WidgetSmartRAHConfig(IDC_PARAM2, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[2] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM3, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM3]);
	WidgetSmartRAHConfig(IDC_PARAM3, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM3], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM3]);
	WidgetSetText(IDC_PARAM3, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM3, "");
	//WidgetSNConfig(IDC_PARAM3, NULL);
	WidgetSmartRAHConfig(IDC_PARAM3, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[3] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM4, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM4]);
	WidgetSmartRAHConfig(IDC_PARAM4, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM4], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM4]);
	WidgetSetText(IDC_PARAM4, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM4, "");
	//WidgetSNConfig(IDC_PARAM4, NULL);
	WidgetSmartRAHConfig(IDC_PARAM4, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[4] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM5, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM5]);
	WidgetSmartRAHConfig(IDC_PARAM5, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM5], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM5]);
	WidgetSetText(IDC_PARAM5, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM5, "");
	//WidgetSNConfig(IDC_PARAM5, NULL);
	WidgetSmartRAHConfig(IDC_PARAM5, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[5] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM6, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM6]);
	WidgetSmartRAHConfig(IDC_PARAM6, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM6], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM6]);
	WidgetSetText(IDC_PARAM6, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM6, "");
	//WidgetSNConfig(IDC_PARAM6, NULL);
	WidgetSmartRAHConfig(IDC_PARAM6, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[6] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM7, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM7]);
	WidgetSmartRAHConfig(IDC_PARAM7, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM7], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM7]);
	WidgetSetText(IDC_PARAM7, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM7, "");
	//WidgetSNConfig(IDC_PARAM7, NULL);
	WidgetSmartRAHConfig(IDC_PARAM7, (RasterAnimHost *)NULL, NULL);
	} // else
if (Active->Method.ParamID[7] >= 0)
	{
	//WidgetSNConfig(IDC_PARAM8, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM8]);
	WidgetSmartRAHConfig(IDC_PARAM8, &Active->Method.AnimPar[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM8], &Active->Method);
	sprintf(TextStr, "%s ", Active->Method.ParamName[WCS_EFFECTS_PROJMETHOD_ANIMPAR_PARAM8]);
	WidgetSetText(IDC_PARAM8, TextStr);
	} // if
else
	{
	WidgetSetText(IDC_PARAM8, "");
	//WidgetSNConfig(IDC_PARAM8, NULL);
	WidgetSmartRAHConfig(IDC_PARAM8, (RasterAnimHost *)NULL, NULL);
	} // else


//WidgetSNConfig(IDC_DELTAX, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX]);
//WidgetSNConfig(IDC_DELTAY, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY]);
//WidgetSNConfig(IDC_DELTAZ, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ]);
//WidgetSNConfig(IDC_MAJORAXIS, &Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR]);
//WidgetSNConfig(IDC_MINORAXIS, &Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR]);
WidgetSmartRAHConfig(IDC_DELTAX, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAX], &Active->Datum);
WidgetSmartRAHConfig(IDC_DELTAY, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAY], &Active->Datum);
WidgetSmartRAHConfig(IDC_DELTAZ, &Active->Datum.AnimPar[WCS_EFFECTS_GEODATUM_ANIMPAR_DELTAZ], &Active->Datum);
WidgetSmartRAHConfig(IDC_MAJORAXIS, &Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR], &Active->Datum.Ellipse);
WidgetSmartRAHConfig(IDC_MINORAXIS, &Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR], &Active->Datum.Ellipse);

ComputeEF();

ConfigureFI(NativeWin, IDC_ECCENTRICITY,
 &EccentricitySq,
  .001,
   0.0,
	1.0,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_INVFLATTENING,
 &InvFlattening,
  1.0,
   0.0,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);


DisableWidgets();
HideWidgets();
// advanced
DisplayAdvancedFeatures();

} // CoordSysEditGUI::ConfigureWidgets()

/*===========================================================================*/

void CoordSysEditGUI::FillZoneDrop(void)
{
long TabIndex;
long NameFieldNum;
char Str[256];

WidgetCBClear(IDC_ZONEDROP);

if ((NameFieldNum = Active->ZoneTable.FindFieldByName("NAME")) >= 0)
	{
	TabIndex = 0;
	while (Active->ZoneTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
		{
		WidgetCBInsert(IDC_ZONEDROP, TabIndex, Str);
		TabIndex ++;
		} // while
	} // if field number found

} // CoordSysEditGUI::FillZoneDrop()

/*===========================================================================*/

void CoordSysEditGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_PARAM1, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM4, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM5, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM6, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM7, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PARAM8, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DELTAX, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DELTAY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DELTAZ, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAJORAXIS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINORAXIS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_ECCENTRICITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_INVFLATTENING, WP_FISYNC_NONOTIFY);

} // CoordSysEditGUI::SyncWidgets

/*===========================================================================*/

void CoordSysEditGUI::HideWidgets(void)
{

WidgetShow(IDC_PARAM1, Active->Method.ParamID[0] >= 0);
WidgetShow(IDC_PARAM2, Active->Method.ParamID[1] >= 0);
WidgetShow(IDC_PARAM3, Active->Method.ParamID[2] >= 0);
WidgetShow(IDC_PARAM4, Active->Method.ParamID[3] >= 0);
WidgetShow(IDC_PARAM5, Active->Method.ParamID[4] >= 0);
WidgetShow(IDC_PARAM6, Active->Method.ParamID[5] >= 0);
WidgetShow(IDC_PARAM7, Active->Method.ParamID[6] >= 0);
WidgetShow(IDC_PARAM8, Active->Method.ParamID[7] >= 0);

} // CoordSysEditGUI::HideWidgets

/*===========================================================================*/

void CoordSysEditGUI::DisableWidgets(void)
{

} // CoordSysEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void CoordSysEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_METHODDROP_LABEL, true);
	WidgetShow(IDC_METHODDROP, true);
	WidgetShow(IDC_METHODLIST, true);
	WidgetShow(IDC_PARAM1, true);
	WidgetShow(IDC_PARAM2, true);
	WidgetShow(IDC_PARAM3, true);
	WidgetShow(IDC_PARAM4, true);
	WidgetShow(IDC_PARAM5, true);
	WidgetShow(IDC_PARAM6, true);
	WidgetShow(IDC_PARAM7, true);
	WidgetShow(IDC_PARAM8, true);
	WidgetShow(IDC_LOADCOMPONENT, true);
	WidgetShow(IDC_SAVECOMPONENT, true);
	WidgetShow(IDC_METHODUSAGETXT_LABEL, true);
	WidgetShow(IDC_METHODUSAGETXT, true);
	WidgetShow(IDC_DATUMDROP_LABEL, true);
	WidgetShow(IDC_DATUMDROP, true);
	WidgetShow(IDC_DATUMLIST, true);
	WidgetShow(IDC_DELTA_TEXT, true);
	WidgetShow(IDC_DELTAX, true);
	WidgetShow(IDC_DELTAY, true);
	WidgetShow(IDC_DELTAZ, true);
	WidgetShow(IDC_DATUMCALC, true);
	WidgetShow(IDC_LOADCOMPONENT2, true);
	WidgetShow(IDC_SAVECOMPONENT2, true);
	WidgetShow(IDC_DATUMUSAGETXT_LABEL, true);
	WidgetShow(IDC_DATUMUSAGETXT, true);
	WidgetShow(IDC_ELLIPSOIDDROP_LABEL, true);
	WidgetShow(IDC_ELLIPSOIDDROP, true);
	WidgetShow(IDC_ELLIPSOIDLIST, true);
	WidgetShow(IDC_MAJORAXIS, true);
	WidgetShow(IDC_MINORAXIS, true);
	WidgetShow(IDC_ECCENTRICITY, true);
	WidgetShow(IDC_INVFLATTENING, true);
	WidgetShow(IDC_LOADCOMPONENT3, true);
	WidgetShow(IDC_SAVECOMPONENT3, true);
	WidgetShow(IDC_ELLIPSOIDUSAGETXT_LABEL, true);
	WidgetShow(IDC_ELLIPSOIDUSAGETXT, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_METHODDROP_LABEL, false);
	WidgetShow(IDC_METHODDROP, false);
	WidgetShow(IDC_METHODLIST, false);
	WidgetShow(IDC_PARAM1, false);
	WidgetShow(IDC_PARAM2, false);
	WidgetShow(IDC_PARAM3, false);
	WidgetShow(IDC_PARAM4, false);
	WidgetShow(IDC_PARAM5, false);
	WidgetShow(IDC_PARAM6, false);
	WidgetShow(IDC_PARAM7, false);
	WidgetShow(IDC_PARAM8, false);
	WidgetShow(IDC_LOADCOMPONENT, false);
	WidgetShow(IDC_SAVECOMPONENT, false);
	WidgetShow(IDC_METHODUSAGETXT_LABEL, false);
	WidgetShow(IDC_METHODUSAGETXT, false);
	WidgetShow(IDC_DATUMDROP_LABEL, false);
	WidgetShow(IDC_DATUMDROP, false);
	WidgetShow(IDC_DATUMLIST, false);
	WidgetShow(IDC_DELTA_TEXT, false);
	WidgetShow(IDC_DELTAX, false);
	WidgetShow(IDC_DELTAY, false);
	WidgetShow(IDC_DELTAZ, false);
	WidgetShow(IDC_DATUMCALC, false);
	WidgetShow(IDC_LOADCOMPONENT2, false);
	WidgetShow(IDC_SAVECOMPONENT2, false);
	WidgetShow(IDC_DATUMUSAGETXT_LABEL, false);
	WidgetShow(IDC_DATUMUSAGETXT, false);
	WidgetShow(IDC_ELLIPSOIDDROP_LABEL, false);
	WidgetShow(IDC_ELLIPSOIDDROP, false);
	WidgetShow(IDC_ELLIPSOIDLIST, false);
	WidgetShow(IDC_MAJORAXIS, false);
	WidgetShow(IDC_MINORAXIS, false);
	WidgetShow(IDC_ECCENTRICITY, false);
	WidgetShow(IDC_INVFLATTENING, false);
	WidgetShow(IDC_LOADCOMPONENT3, false);
	WidgetShow(IDC_SAVECOMPONENT3, false);
	WidgetShow(IDC_ELLIPSOIDUSAGETXT_LABEL, false);
	WidgetShow(IDC_ELLIPSOIDUSAGETXT, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // CoordSysEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void CoordSysEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CoordSysEditGUI::Cancel

/*===========================================================================*/

void CoordSysEditGUI::Name(void)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // CoordSysEditGUI::Name()

/*===========================================================================*/

void CoordSysEditGUI::SelectNewSystem(int NewSystem)
{
long Current;
NotifyTag Changes[2];

if (NewSystem >= 0)
	Current = NewSystem;
else
	Current = WidgetCBGetCurSel(IDC_PROJSYSDROP);
Active->SetSystem(Current);
FillZoneDrop();

// <<<>>> Might be nice to change this to one GenerateNotify in the future, once we're sure everybody can handle it
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
// name might have been changed by SetSystem()
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CoordSysEditGUI::SelectNewSystem

/*===========================================================================*/

void CoordSysEditGUI::SelectNewZone(void)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_ZONEDROP);
Active->SetZone(Current);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CoordSysEditGUI::SelectNewZone

/*===========================================================================*/

void CoordSysEditGUI::SelectNewMethod(void)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_METHODDROP);
Active->Method.SetMethod(Current);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CoordSysEditGUI::SelectNewMethod

/*===========================================================================*/

void CoordSysEditGUI::SelectNewDatum(int NewDatum)
{
long Current;
NotifyTag Changes[2];

if (NewDatum >= 0)
	Current = NewDatum;
else
	Current = WidgetCBGetCurSel(IDC_DATUMDROP);
Active->Datum.SetDatum(Current);

SelectNewSystem(0);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CoordSysEditGUI::SelectNewDatum

/*===========================================================================*/

void CoordSysEditGUI::SelectNewEllipsoid(int NewEllipse)
{
long Current;
NotifyTag Changes[2];

if (NewEllipse >= 0)
	Current = NewEllipse;
else
	Current = WidgetCBGetCurSel(IDC_ELLIPSOIDDROP);
Active->Datum.Ellipse.SetEllipsoid(Current);

SelectNewDatum(0);

Changes[0] = MAKE_ID(Active->Datum.GetNotifyClass(), Active->Datum.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->Datum.GetRAHostRoot());

} // CoordSysEditGUI::SelectNewEllipsoid

/*===========================================================================*/

void CoordSysEditGUI::ComputeEF(void)
{
double a, b, f;

a = Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
b = Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue;
if (b > a)
	b = a;
f = (a - b) / a;
EccentricitySq = 2.0 * f - f * f;
if (f > 0.0)
	InvFlattening = 1.0 / f;
else
	InvFlattening = 0.0;

} // CoordSysEditGUI::ComputeEF

/*===========================================================================*/

void CoordSysEditGUI::ComputeFromEccSq(void)
{
double a, b, f;
NotifyTag Changes[2];

a = Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;

// EccSq must be <= 1 and >= 0
f = (2.0 - sqrt(4.0 - 4.0 * EccentricitySq)) / 2.0;
b = a - f * a;
Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(b);
if (f > 0.0)
	InvFlattening = 1.0 / f;
else
	InvFlattening = 0.0;
Changes[0] = MAKE_ID(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetRAHostRoot());

} // CoordSysEditGUI::ComputeFromEccSq

/*===========================================================================*/

void CoordSysEditGUI::ComputeFromInvFlattening(void)
{
double a, b, f;
NotifyTag Changes[2];

f = InvFlattening;
a = Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
if (f > 0.0)
	f = 1.0 / f;
b = a - f * a;
EccentricitySq = 2.0 * f - f * f;
Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(b);
Changes[0] = MAKE_ID(Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].GetRAHostRoot());

} // CoordSysEditGUI::ComputeFromInvFlattening

/*===========================================================================*/

void CoordSysEditGUI::OpenCoordsCalculator(void)
{

#ifdef WCS_ENABLE_COORDSCALC
if(GlobalApp->GUIWins->CSC)
	{
	delete GlobalApp->GUIWins->CSC;
	}
GlobalApp->GUIWins->CSC = new CoordsCalculatorGUI(EffectsHost, Active);
if(GlobalApp->GUIWins->CSC)
	{
	GlobalApp->GUIWins->CSC->Open(GlobalApp->MainProj);
	}
#endif // WCS_ENABLE_COORDSCALC

} // CoordSysEditGUI::OpenCoordsCalculator
