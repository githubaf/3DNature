// TerraGridderEditGUI.cpp
// Code for TerraGridder editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "TerraGridderEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "GraphData.h"
#include "DBFilterEvent.h"
#include "Interactive.h"
#include "resource.h"
#include "FeatureConfig.h"


char *TerraGridderEditGUI::TabNames[WCS_TERRAGRIDDERGUI_NUMTABS] = {"General", "Output && Filters", "Current Filter"};

long TerraGridderEditGUI::ActivePage;

NativeGUIWin TerraGridderEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TerraGridderEditGUI::Open

/*===========================================================================*/

NativeGUIWin TerraGridderEditGUI::Construct(void)
{
int TabIndex;
#ifdef WCS_BUILD_VNS
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_TERRAGRIDDER_GENERALDEFG, 0, 0);
	CreateSubWinFromTemplate(IDD_TERRAGRIDDER_GRIDDEFG, 0, 1);
	CreateSubWinFromTemplate(IDD_TERRAGRIDDER_FILTER, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_TERRAGRIDDERGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, TabIndex, MyEffect);
			} // for
		#endif // WCS_BUILD_VNS
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // TerraGridderEditGUI::Construct

/*===========================================================================*/

TerraGridderEditGUI::TerraGridderEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraGridder *ActiveSource)
: GUIFenetre('TGRD', this, "Terrain Gridder"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_GRIDDER, 0xff, 0xff, 0xff), 
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
										0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
ProjHost = ProjSource;
DBHost = DBSource;
Active = ActiveSource;
ActiveFilter = NULL;
ReceivingDiagnostics = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Terrain Gridder - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);

	NorthADT.SetIncrement(.1);
	SouthADT.SetIncrement(.1);
	EastADT.SetIncrement(.1);
	WestADT.SetIncrement(.1);
	NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
	WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);

	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);

	if (stricmp(Active->NNG.grd_dir, GlobalApp->MainProj->dirname))
		{
		if (UserMessageYN(Active->NNG.grd_dir, "This is no longer the Project Default Directory where newly generated DEMs are normally stored. Do you wish to update the DEM storage location to the currently active Default Directory?"))
			strcpy(Active->NNG.grd_dir, GlobalApp->MainProj->dirname);
		} // if
	} // if
else
	ConstructError = 1;

} // TerraGridderEditGUI::TerraGridderEditGUI

/*===========================================================================*/

TerraGridderEditGUI::~TerraGridderEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // TerraGridderEditGUI::~TerraGridderEditGUI()

/*===========================================================================*/

long TerraGridderEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_GRD, 0);

return(0);

} // TerraGridderEditGUI::HandleCloseWin

/*===========================================================================*/

long TerraGridderEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_GRD, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDFILTER:
		{
		AddFilter();
		break;
		} // IDC_ADDFILTER
	case IDC_REMOVEFILTER:
		{
		RemoveFilter();
		break;
		} // IDC_REMOVEFILTER
	case IDC_MOVEFILTERUP:
		{
		ChangeFilterListPosition(1);
		break;
		} // IDC_MOVEFILTERUP
	case IDC_MOVEFILTERDOWN:
		{
		ChangeFilterListPosition(0);
		break;
		} // IDC_MOVEFILTERDOWN
	case IDC_DEFAULTBOUNDS:
		{
		Active->GetDefaultBounds(DBHost);
		break;
		} // 
	case IDC_SETBOUNDS:
		{
		SetBounds(NULL);
		break;
		} // 
	case IDC_GRID:
		{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("The Demo Version Gridder adds diagonal noise stripes.");
#endif //WCS_BUILD_DEMO
		Active->MakeGrid(EffectsHost, ProjHost, DBHost);
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (Active->Coords)
			Active->Coords->EditRAHost();
		break;
		} // IDC_EDITCOORDS
	default: break;
	} // ButtonID
return(0);

} // TerraGridderEditGUI::HandleButtonClick

/*===========================================================================*/

long TerraGridderEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_FILTERLIST:
		{
		RemoveFilter();
		break;
		} // IDC_FILTERLIST
	} // switch

return(0);

} // TerraGridderEditGUI::HandleListDelItem

/*===========================================================================*/

long TerraGridderEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(018, 18);
switch (CtrlID)
	{
	case IDC_FILTERLIST:
		{
		SetActiveFilter();
		break;
		}
	} // switch CtrlID

return (0);

} // TerraGridderEditGUI::HandleListSel

/*===========================================================================*/

long TerraGridderEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FILTERLIST:
		{
		WidgetTCSetCurSel(IDC_TAB1, 2);
		ShowPanel(0, 2);
		break;
		}
	} // switch CtrlID

return (0);

} // TerraGridderEditGUI::HandleListDoubleClick

/*===========================================================================*/

long TerraGridderEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_DEMNAME:
		{
		DoNewOutputName();
		break;
		} // 
	case IDC_LAYERMATCH:
		{
		NewMatchLayer();
		break;
		} // 
	case IDC_NAMEMATCH:
		{
		NewMatchName();
		break;
		} // 
	case IDC_LABELMATCH:
		{
		NewMatchLabel();
		break;
		} // 
	} // switch CtrlID

return (0);

} // TerraGridderEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long TerraGridderEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		}
	} // switch CtrlID

return (0);

} // TerraGridderEditGUI::HandleCBChange

/*===========================================================================*/

long TerraGridderEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // TerraGridderEditGUI::HandlePageChange

/*===========================================================================*/

long TerraGridderEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_CHECKFILTERENABLED:
		{
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKCONTROLPT:
	case IDC_CHECKVECTORS:
	case IDC_CHECKPASSENABLED:
	case IDC_CHECKPASSDISABLED:
	case IDC_CHECKPASSLINES:
	case IDC_CHECKPASSPOINTS:
	case IDC_CHECKLAYERNOT:
	case IDC_CHECKNAMENOT:
	case IDC_CHECKLABELNOT:
		{
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLAYEREQUALS:
		{
		if (ActiveFilter && ActiveFilter->LayerEquals)
			ActiveFilter->LayerSimilar = ActiveFilter->LayerNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLAYERSIMILAR:
		{
		if (ActiveFilter && ActiveFilter->LayerSimilar)
			ActiveFilter->LayerEquals = ActiveFilter->LayerNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLAYERNUMERIC:
		{
		if (ActiveFilter && ActiveFilter->LayerNumeric)
			ActiveFilter->LayerEquals = ActiveFilter->LayerSimilar = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKNAMEEQUALS:
		{
		if (ActiveFilter && ActiveFilter->NameEquals)
			ActiveFilter->NameSimilar = ActiveFilter->NameNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKNAMESIMILAR:
		{
		if (ActiveFilter && ActiveFilter->NameSimilar)
			ActiveFilter->NameEquals = ActiveFilter->NameNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKNAMENUMERIC:
		{
		if (ActiveFilter && ActiveFilter->NameNumeric)
			ActiveFilter->NameEquals = ActiveFilter->NameSimilar = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLABELEQUALS:
		{
		if (ActiveFilter && ActiveFilter->LabelEquals)
			ActiveFilter->LabelSimilar = ActiveFilter->LabelNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLABELSIMILAR:
		{
		if (ActiveFilter && ActiveFilter->LabelSimilar)
			ActiveFilter->LabelEquals = ActiveFilter->LabelNumeric = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKLABELNUMERIC:
		{
		if (ActiveFilter && ActiveFilter->LabelNumeric)
			ActiveFilter->LabelEquals = ActiveFilter->LabelSimilar = 0;
		Active->SetFloating(1);
		break;
		} // 
	case IDC_CHECKFLOATING:
		{
		Active->SetFloating(Active->Floating);
		break;
		} // 
	case IDC_CHECKEXTRAP:
		{
		if (Active->NNG.extrap)
			WidgetSetDisabled(IDC_OUTNULL, 1);
		else
			WidgetSetDisabled(IDC_OUTNULL, 0);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

if (! Active->Floating)
	{
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

return(0);

} // TerraGridderEditGUI::HandleSCChange

/*===========================================================================*/

long TerraGridderEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

Active->SetFloating(1);
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

return(0);

} // TerraGridderEditGUI::HandleSRChange

/*===========================================================================*/

long TerraGridderEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_BOUNDN:
		{
		Active->SetFloating(0);
		Active->NNG.yterm = NorthADT.CurValue;
		break;
		} // 
	case IDC_BOUNDS:
		{
		Active->SetFloating(0);
		Active->NNG.ystart = SouthADT.CurValue;
		break;
		} // 
	case IDC_BOUNDE:
		{
		Active->SetFloating(0);
		Active->NNG.xterm = EastADT.CurValue;
		break;
		} // 
	case IDC_BOUNDW:
		{
		Active->SetFloating(0);
		Active->NNG.xstart = WestADT.CurValue;
		break;
		} // 
	} // switch CtrlID

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

return(0);

} // TerraGridderEditGUI::HandleFIChange

/*===========================================================================*/

void TerraGridderEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;
#ifdef WCS_BUILD_VNS
long Pos, CurPos;
GeneralEffect *MyEffect, *MatchEffect;
#endif // WCS_BUILD_VNS

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	} // if

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	Active->SetFloating(Active->Floating);
	} // if

#ifdef WCS_BUILD_VNS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPos = -1;
	MatchEffect = Active->Coords;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		Done = 1;
		} // if
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // TerraGridderEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void TerraGridderEditGUI::ConfigureWidgets(void)
{
char TextStr[256];
#ifdef WCS_BUILD_VNS
long Ct, ListPos, NumEntries;
CoordSys *TestObj;
#endif // WCS_BUILD_VNS

if(Active && Active->Floating)
	Active->GetDefaultBounds(DBHost);

sprintf(TextStr, "Terrain Gridder - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureTB(NativeWin, IDC_ADDFILTER, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEFILTER, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEFILTERUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEFILTERDOWN, IDI_ARROWDOWN, NULL);

#ifdef WCS_BUILD_VNS
if (Active->Coords && Active->Coords->Method.GCTPMethod)
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
#endif // WCS_BUILD_VNS

NorthADT.SetValue(Active->NNG.yterm);
SouthADT.SetValue(Active->NNG.ystart);
EastADT.SetValue(Active->NNG.xterm);
WestADT.SetValue(Active->NNG.xstart);

WidgetSNConfig(IDC_BOUNDN, &NorthADT);
WidgetSNConfig(IDC_BOUNDS, &SouthADT);
WidgetSNConfig(IDC_BOUNDE, &EastADT);
WidgetSNConfig(IDC_BOUNDW, &WestADT);

ConfigureFI(NativeWin, IDC_OVERLAPH, &Active->NNG.horilap, 	1.0, 0.0, 100000.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_OVERLAPV, &Active->NNG.vertlap, 	1.0, 0.0, 100000.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_TAUT1, &Active->NNG.bI, 			.1, 0.0, 100.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_OUTCOLS, &Active->NNG.x_nodes, 	10.0, 10.0, 100000.0, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_OUTROWS, &Active->NNG.y_nodes, 	10.0, 10.0, 100000.0, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_EWMAPS, &Active->EWMaps, 	1.0, 1.0, 1000.0, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_NSMAPS, &Active->NSMaps, 	1.0, 1.0, 1000.0, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_OUTNULL, &Active->NNG.nuldat, 	1.0, -100000.0, 100000.0, FIOFlag_Double, NULL, NULL);

ConfigureSC(NativeWin, IDC_CHECKEXTRAP, &Active->NNG.extrap, SCFlag_Long, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKFLOATING, &Active->Floating, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKDENSIFY, &Active->Densify, SCFlag_Short, NULL, NULL);

if (Active->NNG.extrap)
	WidgetSetDisabled(IDC_OUTNULL, 1);
else
	WidgetSetDisabled(IDC_OUTNULL, 0);

WidgetSetText(IDC_DEMNAME, Active->NNG.grd_file);

#ifdef WCS_BUILD_VNS
if (Active->Coords)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestObj == Active->Coords)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	} // if
else
	{
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
	} // else
#else // !WCS_BUILD_VNS
WidgetShow(IDC_COORDSDROP, 0);
WidgetShow(IDC_EDITCOORDS, 0);
WidgetShow(IDC_STATIC_CS, 0);
#endif // !WCS_BUILD_VNS

SetGridSizeText();
BuildFilterList();
ConfigureFilter();

} // TerraGridderEditGUI::ConfigureWidgets()

/*===========================================================================*/

void TerraGridderEditGUI::SetGridSizeText(void)
{
char GridText[64];
double LatScaleMeters, GridSize;
AnimDoubleTime ADT;

ADT.SetMultiplier(1.0);
ADT.SetIncrement(1.0);
ADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
LatScaleMeters = LatScale(EffectsHost->GetPlanetRadius());

// NS size
if (Active->NSMaps > 0 && Active->NNG.y_nodes > 1)
	{
	#ifdef WCS_BUILD_VNS
	// is it a projected system?
	if(Active->Coords && Active->Coords->Method.GCTPMethod)
		{
		GridSize = fabs((Active->NNG.yterm - Active->NNG.ystart) / (Active->NSMaps * (Active->NNG.y_nodes - 1)));
		} // if
	else
	#endif // WCS_BUILD_VNS
		{ // else, or temp scope
		GridSize = LatScaleMeters * (Active->NNG.yterm - Active->NNG.ystart) / (Active->NSMaps * (Active->NNG.y_nodes - 1));
		} // else, or temp scope
	FormatAsPreferredUnit(GridText, &ADT, GridSize);
	WidgetSetText(IDC_NSGRIDSIZETEXT, GridText);
	} // if
else
	{
	WidgetSetText(IDC_WEGRIDSIZETEXT, "");
	} // else

// EW size
if (Active->EWMaps > 0 && Active->NNG.x_nodes > 1)
	{
	#ifdef WCS_BUILD_VNS
	// is it a projected system?
	if(Active->Coords && Active->Coords->Method.GCTPMethod)
		{
		GridSize = fabs((Active->NNG.xstart - Active->NNG.xterm) / (Active->EWMaps * (Active->NNG.x_nodes - 1)));
		} // if
	else
	#endif // WCS_BUILD_VNS
		{ // else, or temp scope
		GridSize = LatScaleMeters * cos(PiOver180 * (Active->NNG.yterm + Active->NNG.ystart) / 2.0) * (Active->NNG.xstart - Active->NNG.xterm) / (Active->EWMaps * (Active->NNG.x_nodes - 1));
		} // else, or temp scope
	FormatAsPreferredUnit(GridText, &ADT, GridSize);
	WidgetSetText(IDC_WEGRIDSIZETEXT, GridText);
	} // if
else
	{
	WidgetSetText(IDC_WEGRIDSIZETEXT, "");
	} // else

} // TerraGridderEditGUI::SetGridSizeText()

/*===========================================================================*/

void TerraGridderEditGUI::BuildFilterList(void)
{
DBFilterEvent *Current = Active->Filters, *CurrentFilter;
long Ct = 0, TempCt, Place, NumListItems, FoundIt;
unsigned MaxLen = 10;
char ListName[512];

NumListItems = WidgetLBGetCount(IDC_FILTERLIST);

ActiveFilterValid();

while (Current || Ct < NumListItems)
	{
	CurrentFilter = Ct < NumListItems ? (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == (DBFilterEvent *)CurrentFilter)
			{
			BuildFilterListEntry(ListName, Current);
			if (strlen(ListName) > MaxLen)
				MaxLen = (unsigned int)strlen(ListName);
			WidgetLBReplace(IDC_FILTERLIST, Ct, ListName);
			WidgetLBSetItemData(IDC_FILTERLIST, Ct, Current);
			if (Current == ActiveFilter)
				WidgetLBSetCurSel(IDC_FILTERLIST, Ct);
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current == (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildFilterListEntry(ListName, Current);
				if (strlen(ListName) > MaxLen)
					MaxLen = (unsigned int)strlen(ListName);
				WidgetLBReplace(IDC_FILTERLIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_FILTERLIST, TempCt, Current);
				if (Current == ActiveFilter)
					WidgetLBSetCurSel(IDC_FILTERLIST, TempCt);
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_FILTERLIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildFilterListEntry(ListName, Current);
				if (strlen(ListName) > MaxLen)
					MaxLen = (unsigned int)strlen(ListName);
				Place = WidgetLBInsert(IDC_FILTERLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_FILTERLIST, Place, Current);
				if (Current == ActiveFilter)
					WidgetLBSetCurSel(IDC_FILTERLIST, Place);
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_FILTERLIST, Ct);
		NumListItems --;
		} // else
	} // while

WidgetLBSetHorizExt(IDC_FILTERLIST, 5 * MaxLen);

} // TerraGridderEditGUI::BuildFilterList

/*===========================================================================*/

void TerraGridderEditGUI::BuildFilterListEntry(char *ListName, DBFilterEvent *Me)
{
int AddComma = 0, AddAmpersand = 0;

if (Me->Enabled && (Me->PassControlPt || Me->PassVector) && (Me->PassEnabled || Me->PassDisabled) && (Me->PassLine || Me->PassPoint))
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
	{
	strcat(ListName, "Add ");
	} // if
else
	{
	strcat(ListName, "Subtract ");
	} // else
if (Me->PassControlPt)
	{
	strcat(ListName, "Control Points");
	AddComma = AddAmpersand = 1;
	} // if
if (Me->PassVector)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Vectors");
	AddComma = 1;
	} // if
if (Me->PassEnabled)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Enabled");
	AddComma = AddAmpersand = 1;
	} // if
else
	AddAmpersand = 0;
if (Me->PassDisabled)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Disabled");
	AddComma = 1;
	} // if
if (Me->PassLine)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Lines");
	AddComma = AddAmpersand = 1;
	} // if
else
	AddAmpersand = 0;
if (Me->PassPoint)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Points");
	AddComma = 1;
	} // if
if (((Me->LayerEquals || Me->LayerSimilar) && Me->Layer) || Me->LayerNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Layer ");
	if (Me->LayerNot)
		strcat(ListName, "Not ");
	if (Me->LayerEquals)
		strcat(ListName, "Equals ");
	else if (Me->LayerSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->LayerNumeric)
		strcat(ListName, "Numeric");
	if (! Me->LayerNumeric)
		strcat(ListName, Me->Layer);
	AddComma = 1;
	} // if
if (((Me->NameEquals || Me->NameSimilar) && Me->Name) || Me->NameNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Name ");
	if (Me->NameNot)
		strcat(ListName, "Not ");
	if (Me->NameEquals)
		strcat(ListName, "Equals ");
	else if (Me->NameSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->NameNumeric)
		strcat(ListName, "Numeric");
	if (! Me->NameNumeric)
		strcat(ListName, Me->Name);
	AddComma = 1;
	} // if
if (((Me->LabelEquals || Me->LabelSimilar) && Me->Label) || Me->LabelNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Label ");
	if (Me->LabelNot)
		strcat(ListName, "Not ");
	if (Me->LabelEquals)
		strcat(ListName, "Equals ");
	else if (Me->LabelSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->LabelNumeric)
		strcat(ListName, "Numeric");
	if (! Me->LabelNumeric)
		strcat(ListName, Me->Label);
	} // if

} // TerraGridderEditGUI::BuildFilterListEntry()

/*===========================================================================*/

DBFilterEvent *TerraGridderEditGUI::ActiveFilterValid(void)
{
DBFilterEvent *CurFilt;

if (ActiveFilter)
	{
	CurFilt = Active->Filters;
	while (CurFilt)
		{
		if (CurFilt == ActiveFilter)
			{
			return (ActiveFilter);
			} // if
		CurFilt = CurFilt->Next;
		} // while
	} // if

return (ActiveFilter = Active->Filters);

} // TerraGridderEditGUI::ActiveFilterValid

/*===========================================================================*/

void TerraGridderEditGUI::ConfigureFilter(void)
{

if (ActiveFilter = ActiveFilterValid())
	{
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOADD, &ActiveFilter->EventType, SRFlag_Char, WCS_DBFILTER_EVENTTYPE_ADD, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOSUB, &ActiveFilter->EventType, SRFlag_Char, WCS_DBFILTER_EVENTTYPE_SUB, NULL, NULL);

	ConfigureSC(NativeWin, IDC_CHECKFILTERENABLED, &ActiveFilter->Enabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCONTROLPT, &ActiveFilter->PassControlPt, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKVECTORS, &ActiveFilter->PassVector, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSENABLED, &ActiveFilter->PassEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSDISABLED, &ActiveFilter->PassDisabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSLINES, &ActiveFilter->PassLine, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSPOINTS, &ActiveFilter->PassPoint, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYEREQUALS, &ActiveFilter->LayerEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERSIMILAR, &ActiveFilter->LayerSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNUMERIC, &ActiveFilter->LayerNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNOT, &ActiveFilter->LayerNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMEEQUALS, &ActiveFilter->NameEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMESIMILAR, &ActiveFilter->NameSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENUMERIC, &ActiveFilter->NameNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENOT, &ActiveFilter->NameNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELEQUALS, &ActiveFilter->LabelEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELSIMILAR, &ActiveFilter->LabelSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNUMERIC, &ActiveFilter->LabelNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNOT, &ActiveFilter->LabelNot, SCFlag_Char, NULL, NULL);

	WidgetSetModified(IDC_LAYERMATCH, FALSE);
	if (ActiveFilter->Layer)
		WidgetSetText(IDC_LAYERMATCH, ActiveFilter->Layer);
	else
		WidgetSetText(IDC_LAYERMATCH, "");

	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	if (ActiveFilter->Name)
		WidgetSetText(IDC_NAMEMATCH, ActiveFilter->Name);
	else
		WidgetSetText(IDC_NAMEMATCH, "");

	WidgetSetModified(IDC_LABELMATCH, FALSE);
	if (ActiveFilter->Label)
		WidgetSetText(IDC_LABELMATCH, ActiveFilter->Label);
	else
		WidgetSetText(IDC_LABELMATCH, "");

	WidgetSetDisabled(IDC_LAYERMATCH, ! (ActiveFilter->LayerEquals || ActiveFilter->LayerSimilar));
	WidgetSetDisabled(IDC_NAMEMATCH, ! (ActiveFilter->NameEquals || ActiveFilter->NameSimilar));
	WidgetSetDisabled(IDC_LABELMATCH, ! (ActiveFilter->LabelEquals || ActiveFilter->LabelSimilar));
	WidgetSetDisabled(IDC_CHECKLAYERNOT, ! (ActiveFilter->LayerEquals || ActiveFilter->LayerSimilar || ActiveFilter->LayerNumeric));
	WidgetSetDisabled(IDC_CHECKNAMENOT, ! (ActiveFilter->NameEquals || ActiveFilter->NameSimilar || ActiveFilter->NameNumeric));
	WidgetSetDisabled(IDC_CHECKLABELNOT, ! (ActiveFilter->LabelEquals || ActiveFilter->LabelSimilar || ActiveFilter->LabelNumeric));
	} // if
else
	{
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOADD, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOSUB, NULL, 0, 0, NULL, NULL);

	ConfigureSC(NativeWin, IDC_CHECKFILTERENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCONTROLPT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKVECTORS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSDISABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSLINES, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSPOINTS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYEREQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERSIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMEEQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMESIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELEQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELSIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNOT, NULL, 0, NULL, NULL);

	WidgetSetModified(IDC_LAYERMATCH, FALSE);
	WidgetSetText(IDC_LAYERMATCH, "");

	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	WidgetSetText(IDC_NAMEMATCH, "");

	WidgetSetModified(IDC_LABELMATCH, FALSE);
	WidgetSetText(IDC_LABELMATCH, "");

	WidgetSetDisabled(IDC_LAYERMATCH, TRUE);
	WidgetSetDisabled(IDC_NAMEMATCH, TRUE);
	WidgetSetDisabled(IDC_LABELMATCH, TRUE);
	} // else

} // TerraGridderEditGUI::ConfigureFilter

/*===========================================================================*/

void TerraGridderEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // TerraGridderEditGUI::Cancel

/*===========================================================================*/

void TerraGridderEditGUI::Name(void)
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

} // TerraGridderEditGUI::Name()

/*===========================================================================*/

void TerraGridderEditGUI::NewMatchLayer(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_LAYERMATCH))
	{
	WidgetGetText(IDC_LAYERMATCH, 512, NewName);
	WidgetSetModified(IDC_LAYERMATCH, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewLayer(NewName);
		Active->SetFloating(1);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // TerraGridderEditGUI::NewMatchLayer()

/*===========================================================================*/

void TerraGridderEditGUI::NewMatchName(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAMEMATCH))
	{
	WidgetGetText(IDC_NAMEMATCH, 512, NewName);
	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewName(NewName);
		Active->SetFloating(1);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // TerraGridderEditGUI::NewMatchName()

/*===========================================================================*/

void TerraGridderEditGUI::NewMatchLabel(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_LABELMATCH))
	{
	WidgetGetText(IDC_LABELMATCH, 512, NewName);
	WidgetSetModified(IDC_LABELMATCH, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewLabel(NewName);
		Active->SetFloating(1);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // TerraGridderEditGUI::NewMatchLabel()

/*===========================================================================*/

void TerraGridderEditGUI::AddFilter(void)
{

ActiveFilter = Active->AddFilter(NULL);
Active->SetFloating(1);
if (! Active->Floating)
	{
	BuildFilterList();
	ConfigureFilter();
	} // if

} // TerraGridderEditGUI::AddFilter

/*===========================================================================*/

void TerraGridderEditGUI::RemoveFilter(void)
{
long RemoveItem;
DBFilterEvent *RemoveMe;

if ((RemoveItem = WidgetLBGetCurSel(IDC_FILTERLIST)) != LB_ERR)
	{
	if ((RemoveMe = (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, RemoveItem)) != (DBFilterEvent *)LB_ERR && RemoveMe)
		{
		Active->RemoveFilter(RemoveMe);
		Active->SetFloating(1);
		if (! Active->Floating)
			{
			BuildFilterList();
			ConfigureFilter();
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Filter", "There are no Filters selected to remove.");
		} // else
	} // if
else
	{
	UserMessageOK("Remove Filter", "There are no Filters selected to remove.");
	} // else

} // TerraGridderEditGUI::RemoveFilter

/*===========================================================================*/

void TerraGridderEditGUI::SetActiveFilter(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_FILTERLIST);
Current = (long)WidgetLBGetItemData(IDC_FILTERLIST, Current);
if (Current != LB_ERR)
	ActiveFilter = (DBFilterEvent *)Current;
ConfigureFilter();

} // TerraGridderEditGUI::SetActiveFilter()

/*===========================================================================*/

void TerraGridderEditGUI::ChangeFilterListPosition(short MoveUp)
{
long MoveItem, NumListEntries, SendNotify = 0;
DBFilterEvent *MoveMe, *Current, *PrevFilt = NULL, *PrevPrevFilt = NULL, *StashFilt;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_FILTERLIST)) > 0)
	{
	if ((MoveItem = WidgetLBGetCurSel(IDC_FILTERLIST)) != LB_ERR)
		{
		if ((MoveMe = (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, MoveItem)) != (DBFilterEvent *)LB_ERR && MoveMe)
			{
			if (MoveUp && MoveItem > 0)
				{
				Current = Active->Filters;
				while (Current != MoveMe)
					{
					PrevPrevFilt = PrevFilt;
					PrevFilt = Current;
					Current = Current->Next;
					} // while
				if (Current)
					{
					if (PrevFilt)
						{
						StashFilt = Current->Next;
						if (PrevPrevFilt)
							{
							PrevPrevFilt->Next = Current;
							Current->Next = PrevFilt;
							} // if
						else
							{
							Active->Filters = Current;
							Active->Filters->Next = PrevFilt;
							} // else
						PrevFilt->Next = StashFilt;
						SendNotify = 1;
						} // else if
					} // if
				} // if
			else if (! MoveUp && MoveItem < NumListEntries - 1)
				{
				Current = Active->Filters;
				while (Current != MoveMe)
					{
					PrevPrevFilt = PrevFilt;
					PrevFilt = Current;
					Current = Current->Next;
					} // while
				if (Current)
					{
					if (Current->Next)
						{
						StashFilt = Current->Next->Next;
						if (PrevFilt)
							{
							PrevFilt->Next = Current->Next;
							PrevFilt->Next->Next = Current;
							} // if
						else
							{
							Active->Filters = Current->Next;
							Active->Filters->Next = Current;
							} // else
						Current->Next = StashFilt;
						SendNotify = 1;
						} // if move down
					} // if
				} // else if
			} // if
		} // if
	} // if

if (SendNotify)
	{
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // TerraGridderEditGUI::ChangeFilterListPosition

/*===========================================================================*/

void TerraGridderEditGUI::DoNewOutputName(void)
{
char NewName[64];

if (WidgetGetModified(IDC_DEMNAME))
	{
	WidgetGetText(IDC_DEMNAME, 63, NewName);
	strcpy(Active->NNG.grd_file, NewName);
	} // if

} // TerraGridderEditGUI::DoNewOutputName

/*===========================================================================*/

void TerraGridderEditGUI::SetBounds(DiagnosticData *Data)
{

if (ReceivingDiagnostics == 0)
	{
	if (UserMessageOKCAN("Set Grid Bounds", "The next two points clicked in any View\n will become this Grid's new bounds.\n\nPoints may be selected in any order."))
		ReceivingDiagnostics = 1;
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
			Active->SetFloating(0);
			Active->SetBounds(LatEvent, LonEvent);
			ReceivingDiagnostics = 0;
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);

} // TerraGridderEditGUI::SetBounds()

/*===========================================================================*/

void TerraGridderEditGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Active->SetCoords(NewObj);
	} // if
#endif // WCS_BUILD_VNS

} // TerraGridderEditGUI::SelectNewCoords

/*===========================================================================*/
