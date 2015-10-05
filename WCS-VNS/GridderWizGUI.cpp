// GridderWizGUI.cpp
// Code for Gridder Wizard GUI
// Created from MergerWizGUI.cpp on 03/20/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "GridderWizGUI.h"
#include "AppMem.h"
#include "Application.h"
#include "Conservatory.h"
#include "DBEditGUI.h"
#include "DBFilterEvent.h"
#include "ImageLibGUI.h"
#include "Layers.h"
#include "Lists.h"
#include "Notify.h"
#include "Project.h"
#include "Raster.h"
#include "Requester.h"
#include "resource.h"
#include "Toolbar.h"
#include "Useful.h"
#include "WCSWidgets.h"

extern unsigned short GridderWizPageResourceID[];

/*===========================================================================*/

void GridderWizGUI::ConfigureBounds(void)
{

#ifdef WCS_BUILD_VNS
if (gridder.Coords && gridder.Coords->Method.GCTPMethod)
	{
	NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	} // if
else
	{
	NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
	WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
	} // else
#else // WCS_BUILD_VNS
NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
#endif // WCS_BUILD_VNS

NorthADT.SetValue(gridder.NNG.yterm);
SouthADT.SetValue(gridder.NNG.ystart);
EastADT.SetValue(gridder.NNG.xterm);
WestADT.SetValue(gridder.NNG.xstart);

WidgetSNConfig(IDC_GWIZ_FIN, &NorthADT);
WidgetSNConfig(IDC_GWIZ_FIS, &SouthADT);
WidgetSNConfig(IDC_GWIZ_FIE, &EastADT);
WidgetSNConfig(IDC_GWIZ_FIW, &WestADT);

} // GridderWizGUI::ConfigureBounds

/*===========================================================================*/

void GridderWizGUI::ConfigureWidgets(void)
{

/*
	WCS_GRIDDERWIZ_WIZPAGE_BOUNDS,
	WCS_GRIDDERWIZ_WIZPAGE_CANCEL,
	WCS_GRIDDERWIZ_WIZPAGE_COMPLETE,
	WCS_GRIDDERWIZ_WIZPAGE_COMPLETEERROR,
	WCS_GRIDDERWIZ_WIZPAGE_COORDSYS,
	WCS_GRIDDERWIZ_WIZPAGE_DATALOADED,
	WCS_GRIDDERWIZ_WIZPAGE_DEMNAME,
	WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR,
	WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA,
	WCS_GRIDDERWIZ_WIZPAGE_WELCOME,
*/

// WCS_GRIDDERWIZ_WIZPAGE_BOUNDS - configuration done under DisplayPage

// WCS_GRIDDERWIZ_WIZPAGE_DEMNAME
WidgetSetModified(IDC_GWIZ_DEMNAME, FALSE);
WidgetSetText(IDC_GWIZ_DEMNAME, gridder.NNG.grd_file);

// WCS_GRIDDERWIZ_WIZPAGE_DEMSLOADED
ConfigureSR(NativeWin, IDC_GWIZ_DATANO, IDC_GWIZ_DATANO, &haveData, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_GWIZ_DATANO, IDC_GWIZ_DATAYES, &haveData, SRFlag_Char, 1, NULL, NULL);

} // GridderWizGUI::ConfigureWidgets

/*===========================================================================*/

NativeGUIWin GridderWizGUI::Construct(void)
{
GeneralEffect *myEffect;
long tabIndex;
unsigned short PanelCt = 0;

#ifdef WCS_BUILD_VNS
if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMWIZ, LocalWinSys()->RootWin);

	// these must be in the same order as the defines for the page numbers
	for (PanelCt = 0; PanelCt < WCS_GRIDDERWIZ_NUMPAGES; PanelCt++)
		{
		CreateSubWinFromTemplate(GridderWizPageResourceID[PanelCt], 0, PanelCt, false);
		} // for

	if (NativeWin)
		{
		WidgetCBInsert(IDC_GWIZ_CBCOORDS, -1, "New Coordinate System...");
		for (myEffect = effectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
			{
			tabIndex = WidgetCBInsert(IDC_GWIZ_CBCOORDS, -1, myEffect->GetName());
			WidgetCBSetItemData(IDC_GWIZ_CBCOORDS, tabIndex, myEffect);
			} // for

		strcpy(gridder.Name, "GWiz Gridder");
		strcpy(gridder.NNG.grd_file, "GWiz DEM");
		ConfigureWidgets();
		DisplayPage();
		} // if
	} // if
#endif // WCS_BUILD_VNS

return (NativeWin);

} // GridderWizGUI::Construct

/*===========================================================================*/

void GridderWizGUI::CreateGridList(void)
{
CoordSys *myCoords;
GeneralEffect *myEffect;
Joe *curJoe;
JoeCoordSys *myAttr;
list<CoordSys *>::iterator cli;
list<Joe *>::iterator gli;
long lastOBN = -1;
long tabIndex;

if (GlobalApp->GUIWins->DBE)
	{
	while (curJoe = GlobalApp->GUIWins->DBE->GetNextSelected(lastOBN))
		{
		if (! curJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (curJoe->GetClass() != 0)	// not interested in DEMs
				{
				gridList.push_back(curJoe);

				totalPoints += curJoe->nNumPoints;

				if (myAttr = (JoeCoordSys *)curJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					myCoords = myAttr->Coord;
				else
					myCoords = NULL;

				if (myCoords)
					coordsList.push_back(myCoords);	// we'll take care of duplicates later

				} // if
			} // if
		} // if
	if (gridList.empty())
		{
		// F2_NOTE: handle this error
		UserMessageOK("Gridder Wizard", "No Control Points or Vectors were included in your selection.");
		} //
	} // if
else
	{
	UserMessageOK("Gridder Wizard", "You must leave the Database Editor open before pressing the NEXT button.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_DBG, 0);
	} // else

// F2_NOTE: no CS for WCS!!!
WidgetCBClear(IDC_GWIZ_CBCOORDS);
WidgetCBInsert(IDC_GWIZ_CBCOORDS, -1, "New Coordinate System...");
for (myEffect = effectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
	{
	tabIndex = WidgetCBInsert(IDC_GWIZ_CBCOORDS, -1, myEffect->GetName());
	WidgetCBSetItemData(IDC_GWIZ_CBCOORDS, tabIndex, myEffect);
	} // for

if (! coordsList.empty())	// empty() is generally faster than size()
	{
	CoordSys *curCS, *testCS;
	long ct, listPos, numEntries;

	coordsList.sort();		// sort the list
	coordsList.unique();	// remove duplicates from sorted list
	listPos = -1;
	numEntries = WidgetCBGetCount(IDC_GWIZ_CBCOORDS);
	for (cli = coordsList.begin(); cli != coordsList.end(); ++cli)
		{
		curCS = *cli;
		for (ct = 0; ct < numEntries; ct++)
			{
			if ((testCS = (CoordSys *)WidgetCBGetItemData(IDC_GWIZ_CBCOORDS, ct)) != (CoordSys *)CB_ERR && (testCS == curCS))
				{
				listPos = ct;
				break;
				} // if
			} // for
		} // for
	WidgetCBSetCurSel(IDC_GWIZ_CBCOORDS, listPos);
	} // if

for (gli = gridList.begin(); gli != gridList.end(); ++gli)
	{
	curJoe = *gli;

	if (1 == curJoe->GetClass())
		hasVector = true;
	else if (2 == curJoe->GetClass())
		hasCP = true;

	if (curJoe->TestFlags(WCS_JOEFLAG_ACTIVATED))
		hasEnabled = true;
	else
		hasDisabled = true;

	if (curJoe->GetLineStyle() >= 4)
		hasLines = true;
	else
		hasPoints = true;

	} // while

} // GridderWizGUI::CreateGridList

/*===========================================================================*/

bool GridderWizGUI::CreateLayers(void)
{
Joe *curJoe;
LayerEntry *layerEntry;
list<Joe *>::iterator gli;
struct tm *newTime;
time_t szClock;
bool success = false;
char timeString[16];

// try to create a unique ID from project name & time
strcpy(layerName, "GWiz_");
strncat(layerName, GlobalApp->MainProj->projectname, 8);
time(&szClock); // Get time in seconds
newTime = gmtime(&szClock);
sprintf(timeString, "_%d%02d%02d%02d%02d%02d", newTime->tm_year, newTime->tm_mon + 1, newTime->tm_mday,
		newTime->tm_hour, newTime->tm_min, newTime->tm_sec);
strcat(layerName, timeString);

// F2_NOTE: Should check layers to make sure name is truly unique
// F2_NOTE: Probably want to store layer name or pointer so we can delete it if wizard is cancelled

if (layerEntry = dbHost->DBLayers.MatchMakeLayer(layerName, 0))
	{
	for (gli = gridList.begin(); gli != gridList.end(); ++gli)
		{
		curJoe = *gli;
		curJoe->AddObjectToLayer(layerEntry);
		} // for
	success = true;
	} // if

return success;

} // GridderWizGUI::CreateLayers

/*===========================================================================*/

void GridderWizGUI::DisplayPage(void)
{

if (activePage)
	{
	char nextText[15];
	char disability;

	WidgetSetText(IDC_IMWIZTEXT, activePage->Text);

	if ((activePage->WizPageID == WCS_GRIDDERWIZ_WIZPAGE_COMPLETE) ||
		(activePage->WizPageID == WCS_GRIDDERWIZ_WIZPAGE_COMPLETEERROR) ||
		(activePage->WizPageID == WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR))
		{
		strcpy(nextText, "Close");
		WidgetSetDisabled(IDC_PREV, 1);
		} // if
	else
		strcpy(nextText, "Next -->");
	WidgetSetText(IDC_NEXT, nextText);

	disability = ((activePage->WizPageID == WCS_GRIDDERWIZ_WIZPAGE_DATALOADED) && (! haveData));
	WidgetSetDisabled(IDC_NEXT, disability);

	// WCS_GRIDDERWIZARD_ADDPAGE - if there are specific items that need to be reconfigured or disabled do it here
	switch (activePage->WizPageID)
		{
		case WCS_GRIDDERWIZ_WIZPAGE_BOUNDS:
			InitBounds();
			break;
		case WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA:
			// open Database Editor, keeping track of whether or not it was already open
			if (GlobalApp->GUIWins->DBE)
				dbWasOpen = true;
			GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_DBG, 0);
			break;
		default:
			break;
		} // switch

	SelectPanel(activePage->WizPageID);
	} // if

} // GridderWizGUI::DisplayPage

/*===========================================================================*/

void GridderWizGUI::DoCancel(void)
{

/***
if (wizzer.cancelOrder)
	wizzer.finalOrderCancelled = 1;
wizzer.cancelOrder = 1;
if (activePage = wizzer.ProcessPage(activePage, projectHost))
	DisplayPage();
else
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_GWZ, 0);
***/

if (UserMessageYN("DEM Gridder Wizard", "Do you really wish to cancel?"))
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_GWZ, 0);

} // GridderWizGUI::DoCancel

/*===========================================================================*/

void GridderWizGUI::DoNext(void)
{

if (activePage)
	{
	if (WCS_GRIDDERWIZ_WIZPAGE_COORDSYS == activePage->WizPageID)
		gridder.Coords = (CoordSys *)WidgetCBGetItemData(IDC_GWIZ_CBCOORDS, WidgetCBGetCurSel(IDC_GWIZ_CBCOORDS));
	else if (WCS_GRIDDERWIZ_WIZPAGE_DEMNAME == activePage->WizPageID)
		PlugItIn();
	else if (WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA == activePage->WizPageID)
		{
		// close Database Editor if the wizard opened it
		if (! dbWasOpen)
			GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_DBG, 0);
		CreateGridList();
		} // if

	if (activePage = wizzer.ProcessPage(activePage, projectHost))
		DisplayPage();
	else
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,	WCS_TOOLBAR_ITEM_GWZ, 0);
	} // if

} // GridderWizGUI::DoNext

/*===========================================================================*/

void GridderWizGUI::DoPrev(void)
{

if (activePage)
	{
	activePage = activePage->Prev;
	activePage->Revert();
	DisplayPage();
	} // if

} // GridderWizGUI::DoPrev

/*===========================================================================*/

GridderWizGUI::GridderWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource)
: GUIFenetre('GWIZ', this, "Gridder Wizard")
{
double RangeDefaults[3] = {10000.0, 0.0, 1.0};
PlanetOpt *curPO;
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_GRIDDER, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
								0};

if (EffectsSource && ImageSource && DBSource && ProjectSource)
	{
	_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
	effectsHost = EffectsSource;
	dbHost = DBSource;
	imageHost = ImageSource;
	projectHost = ProjectSource;
	activePage = &wizzer.Wizzes[WCS_GRIDDERWIZ_WIZPAGE_WELCOME];
	dbWasOpen = hasCP = hasVector = hasEnabled = hasDisabled = hasLines = hasPoints = false;
	haveData = 0;
	ReceivingDiagnostics = 0;
	totalPoints = 0;

	if (projectHost->ProjectLoaded)
		{
		#ifdef WCS_BUILD_VNS
		if (effectsHost && dbHost && imageHost && projectHost)
			{
			// get the default CoordSys from PlanetOpt
			curPO = (PlanetOpt *)effectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL);
			assert(curPO);	// this crashes & burns occasionally - why???... maybe because no project was loaded where we had this before???
			if (curPO && curPO->Coords)
				coordsList.push_front(curPO->Coords);

			// set output directory to this project
			strcpy(gridder.NNG.grd_dir, GlobalApp->MainProj->dirname);

			GlobalApp->AppEx->RegisterClient(this, AllEvents);
			ConstructError = 0;
			} // if
		else
		#endif // WCS_BUILD_VNS
			ConstructError = 1;
		} // if
	else
		{
		UserMessageOK("Gridder Wizard", "There is no Project in memory. You must load or create a Project before you can use the Gridder Wizard.");
		ConstructError = 1;
		} // else
	} // if
else
	{
	ConstructError = 1;
	} // else

} // GridderWizGUI::GridderWizGUI

/*===========================================================================*/

GridderWizGUI::~GridderWizGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // GridderWizGUI::~GridderWizGUI()

/*===========================================================================*/

long GridderWizGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
	case IDC_GWIZ_SETBOUNDSVIEW:
		SetBounds(NULL);
		break;
	case IDC_NEXT:
		DoNext();
		break;
	case IDC_PREV:
		DoPrev();
		break;
	case IDC_WIZ_SAVEPROJECT:
		// we already made sure a project was loaded in the constructor
		projectHost->Save(NULL, NULL, dbHost, effectsHost, imageHost, NULL, 0xffffffff);
		#ifndef WCS_BUILD_DEMO
		projectHost->SavePrefs(AppScope->GetProgDir());
		#endif // WCS_BUILD_DEMO
		DoNext();
		break;
	case IDC_WIZ_CANCELORDER:
		{
		wizzer.finalOrderCancelled = 1;
		DoNext();
		break;
		} // 
	case IDCANCEL:
		DoCancel();
		break;
	default:
		break;
	} // ButtonID

return(0);

} // GridderWizGUI::HandleButtonClick

/*===========================================================================*/

long GridderWizGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch CtrlID

return(0);

} // GridderWizGUI::HandleCBChange

/*===========================================================================*/

long GridderWizGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_GWZ, 0);

return(0);

} // GridderWizGUI::HandleCloseWin

/*===========================================================================*/

//long GridderWizGUI::HandleEvent(void)
//{
//
//if (Activity->Type != 2)
//	printf("yo!");
//BuildSQList();
//return(0);
//
//} // GridderWizGUI::HandleEvent

/*===========================================================================*/

long GridderWizGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch CtrlID

return(0);

} // GridderWizGUI::HandleFIChange

/*===========================================================================*/

long GridderWizGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch CtrlID

return (0);

} // GridderWizGUI::HandleListSel

/*===========================================================================*/

void GridderWizGUI::HandleNotifyEvent(void)
{
NotifyTag *changes;
NotifyTag interested[2];
int done = 0;

if (! NativeWin)
	return;
changes = Activity->ChangeNotify->ChangeList;

interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0xff);
interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(interested, changes, 0))
	{
	if (ReceivingDiagnostics)
		SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	done = 1;
	} // if

} // GridderWizGUI::HandleNotifyEvent

/*===========================================================================*/

long GridderWizGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default:
//		break;
//	} // switch

return(0);

} // GridderWizGUI::HandleSCChange

/*===========================================================================*/

long GridderWizGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GWIZ_DATANO:
	case IDC_GWIZ_DATAYES:
		WidgetSetDisabled(IDC_NEXT, ! haveData);
		break;
	default:
		break;
	} // switch

return(0);

} // GridderWizGUI::HandleSRChange

/*===========================================================================*/

long GridderWizGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GWIZ_DEMNAME:
		{
		NameChange();
		break;
		} // 
	} // switch CtrlID

return (0);

} // GridderWizGUI::HandleStringLoseFocus

/*===========================================================================*/

#ifdef WCS_BUILD_VNS

void GridderWizGUI::InitBounds(void)
{
double testNorth, testWest, testSouth, testEast;
Joe *curJoe;
list<Joe *>::iterator gli;
bool geographic = true;

#ifdef WCS_BUILD_VNS
geographic = gridder.Coords && gridder.Coords->GetGeographic();
#endif // WCS_BUILD_VNS

if (geographic)
	{
	gridder.NNG.yterm = -FLT_MAX;	// north
	gridder.NNG.ystart = FLT_MAX;	// south
	gridder.NNG.xstart = -FLT_MAX;	// west
	gridder.NNG.xterm = FLT_MAX;	// east
	} // if
else
	{
	gridder.NNG.yterm = -FLT_MAX;	// north
	gridder.NNG.ystart = FLT_MAX;	// south
	gridder.NNG.xstart = FLT_MAX;	// west
	gridder.NNG.xterm = -FLT_MAX;	// east
	} // else

for (gli = gridList.begin(); gli != gridList.end(); ++gli)
	{
	curJoe = *gli;

	if (gridder.Coords)
		{
		if (curJoe->GetBoundsProjected(gridder.Coords, testNorth, testWest, testSouth, testEast, false))
			{
			if (testNorth > gridder.NNG.yterm)
				gridder.NNG.yterm = testNorth;
			if (testSouth < gridder.NNG.ystart)
				gridder.NNG.ystart = testSouth;
			if (geographic)
				{
				if (testWest > gridder.NNG.xstart)
					gridder.NNG.xstart = testWest;
				if (testEast < gridder.NNG.xterm)
					gridder.NNG.xterm = testEast;
				} // if
			else
				{
				if (testWest < gridder.NNG.xstart)
					gridder.NNG.xstart = testWest;
				if (testEast > gridder.NNG.xterm)
					gridder.NNG.xterm = testEast;
				} // else
			} // if
		} // if
	else
		{
		curJoe->GetTrueBounds(testNorth, testWest, testSouth, testEast);
		if (testNorth > gridder.NNG.yterm)
			gridder.NNG.yterm = testNorth;
		if (testSouth < gridder.NNG.ystart)
			gridder.NNG.ystart = testSouth;
		if (testWest > gridder.NNG.xstart)
			gridder.NNG.xstart = testWest;
		if (testEast < gridder.NNG.xterm)
			gridder.NNG.xterm = testEast;
		} // if

	} // for

ConfigureBounds();

} // GridderWizGUI::InitBounds

#else // WCS_BUILD_VNS

// F2_NOTE: Code this

#endif // WCS_BUILD_VNS

/*===========================================================================*/

void GridderWizGUI::NameChange(void)
{
NotifyTag Changes[2];

if (WidgetGetModified(IDC_GWIZ_DEMNAME))
	{
	WidgetGetText(IDC_GWIZ_DEMNAME, 64, gridder.NNG.grd_file);
	WidgetSetModified(IDC_GWIZ_DEMNAME, false);
	Changes[0] = MAKE_ID(gridder.GetNotifyClass(), gridder.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, gridder.GetRAHostRoot());
	} // if

} // GridderWizGUI::NameChange

/*===========================================================================*/

NativeGUIWin GridderWizGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // GridderWizGUI::Open

/*===========================================================================*/

void GridderWizGUI::PlugItIn(void)
{
double deltax, deltay, gridsize;
GeneralEffect *gfx;

CreateLayers();

gridder.Filters->PassDEM = false;
gridder.Filters->PassControlPt = hasCP;
gridder.Filters->PassVector = hasVector;
gridder.Filters->PassEnabled = hasEnabled;
gridder.Filters->PassDisabled = hasDisabled;
gridder.Filters->PassLine = hasLines;
gridder.Filters->PassPoint = hasPoints;
gridder.Filters->LayerEquals = true;
gridder.Filters->NewLayer(layerName);

deltay = gridder.NNG.yterm - gridder.NNG.ystart;
if (gridder.Coords && gridder.Coords->GetGeographic())
	deltax = gridder.NNG.xstart - gridder.NNG.xterm;
else
	deltax = gridder.NNG.xterm - gridder.NNG.xstart;
gridsize = sqrt((deltax * deltay) / (double)totalPoints);
gridder.NNG.x_nodes = (long)(deltax / gridsize + 0.5);
gridder.NNG.y_nodes = (long)(deltay / gridsize + 0.5);

gridder.NNG.horilap = 0.0;
gridder.NNG.vertlap = 0.0;

#ifdef WCS_BUILD_VNS
gridder.NNG.extrap = false;	// on by default for WCS
#endif // WCS_BUILD_VNS

// Add the gridder object to the Effects Library
gfx = effectsHost->AddEffect(WCS_EFFECTSSUBCLASS_GRIDDER, NULL, &gridder);

} // GridderWizGUI::PlugItIn

/*===========================================================================*/

void GridderWizGUI::SetBounds(DiagnosticData *Data)
{
VertexDEM myVert;
int geographic;

if (ReceivingDiagnostics == 0)
	{
	if (GlobalApp->GUIWins->CVG->GetNumOpenViews())
		{
		if (UserMessageOKCAN("Set Geographic Bounds", "The next two points clicked in any View\n will become this Gridder's new bounds.\n\nPoints may be selected in any order."))
			{
			ReceivingDiagnostics = 1;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			WidgetSetText(IDC_IMWIZTEXT, "Set the 1st point.");
			} // if
		} // if
	else
		UserMessageOK("Set Geographic Bounds", "You must have a view open in order to perform this operation.  Open a view and then press the button again.");
	} // if
else if (ReceivingDiagnostics == 1)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			ReceivingDiagnostics = 2;
			WidgetSetText(IDC_IMWIZTEXT, "Set the 2nd point.");
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else if (ReceivingDiagnostics == 2)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			//Active->GeoReg.SetBounds(Active->Coords, LatEvent, LonEvent);
			//BackupPrevBounds();
			ReceivingDiagnostics = 0;
			myVert.Elev = 0.0;
			myVert.Lat = LatEvent[0];
			myVert.Lon = LonEvent[0];
			if (! gridder.Coords)
				{
				gridder.NNG.yterm = myVert.Lat;
				gridder.NNG.ystart = myVert.Lat;
				gridder.NNG.xstart = myVert.Lon;
				gridder.NNG.xterm = myVert.Lon;
				} // if
			else if (gridder.Coords->DefDegToProj(&myVert))
				{
				geographic = gridder.Coords->GetGeographic();
				if (geographic)
					{						
					gridder.NNG.yterm = myVert.xyz[1];
					gridder.NNG.ystart = myVert.xyz[1];
					gridder.NNG.xstart = myVert.xyz[0];
					gridder.NNG.xterm = myVert.xyz[0];
					} // if
				else
					{
					gridder.NNG.yterm = myVert.xyz[1];
					gridder.NNG.ystart = myVert.xyz[1];
					gridder.NNG.xstart = myVert.xyz[0];
					gridder.NNG.xterm = myVert.xyz[0];
					} // else
				} // else if
			myVert.Lat = LatEvent[1];
			myVert.Lon = LonEvent[1];
			if (! gridder.Coords)
				{
				if (myVert.Lat > gridder.NNG.yterm)
					gridder.NNG.yterm = myVert.Lat;
				if (myVert.Lat < gridder.NNG.ystart)
					gridder.NNG.ystart = myVert.Lat;
				if (myVert.Lon > gridder.NNG.xstart)
					gridder.NNG.xstart = myVert.Lon;
				if (myVert.Lon < gridder.NNG.xterm)
					gridder.NNG.xterm = myVert.Lon;
				} // if
			else if (gridder.Coords->DefDegToProj(&myVert))
				{
				if (geographic)
					{						
					if (myVert.xyz[1] > gridder.NNG.yterm)
						gridder.NNG.yterm = myVert.xyz[1];
					if (myVert.xyz[1] < gridder.NNG.ystart)
						gridder.NNG.ystart = myVert.xyz[1];
					if (myVert.xyz[0] > gridder.NNG.xstart)
						gridder.NNG.xstart = myVert.xyz[0];
					if (myVert.xyz[0] < gridder.NNG.xterm)
						gridder.NNG.xterm = myVert.xyz[0];
					} // if
				else
					{
					if (myVert.xyz[1] > gridder.NNG.yterm)
						gridder.NNG.yterm = myVert.xyz[1];
					if (myVert.xyz[1] < gridder.NNG.ystart)
						gridder.NNG.ystart = myVert.xyz[1];
					if (myVert.xyz[0] < gridder.NNG.xstart)
						gridder.NNG.xstart = myVert.xyz[0];
					if (myVert.xyz[0] > gridder.NNG.xterm)
						gridder.NNG.xterm = myVert.xyz[0];
					} // else
				} // else if
			ConfigureBounds();
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

//WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);

} // GridderWizGUI::SetBounds

/*===========================================================================*/

void GridderWizGUI::SelectNewCoords(void)
{

} // GridderWizGUI::SelectNewCoords

/*===========================================================================*/

void GridderWizGUI::SelectPanel(unsigned short PanelID)
{

ShowPanel(0, PanelID);

} // GridderWizGUI::SelectPanel
