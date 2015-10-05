// TerraGeneratorEditGUI.cpp
// Code for RasterTA editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "TerraGeneratorEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "resource.h"

char *TerraGeneratorEditGUI::TabNames[WCS_TERRAGENERATORGUI_NUMTABS] = {"General", "Coverage"};

long TerraGeneratorEditGUI::ActivePage;

NativeGUIWin TerraGeneratorEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TerraGeneratorEditGUI::Open

/*===========================================================================*/

NativeGUIWin TerraGeneratorEditGUI::Construct(void)
{
int TabIndex;
char *AreaNames[] = {"Acre", "Hectare", "Square Kilometer", "Square Mile", "Township"};
if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_GENERATOR_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_GENERATOR_COVERAGE, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_TERRAGENERATORGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for (TabIndex = 0; TabIndex < 5; TabIndex ++)
			{
			WidgetCBInsert(IDC_AREADROP, -1, AreaNames[TabIndex]);
			} // for
		ShowPanel(0, ActivePage);
		if (Active->InitialSetup)
			{
			CalcArea();
			Active->TerraType.HandleSRAHPopMenuSelection((void *)"TX_CREATE", (RasterAnimHost *)NULL, 
				(RasterAnimHost **)Active->TerraType.GetTexRootPtrAddr(WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT));
			Active->InitialSetup = 0;
			} // if
		ConfigureWidgets();
		UpdateThumbnail();
		} // if
	} // if
 
return (NativeWin);

} // TerraGeneratorEditGUI::Construct

/*===========================================================================*/

TerraGeneratorEditGUI::TerraGeneratorEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraGenerator *ActiveSource)
: GUIFenetre('TGEN', this, "Terrain Generator"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
double MaxLat, MinLat, MaxLon, MinLon;
double RangeDefaults[3] = {FLT_MAX, 1.0, 1.0};
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_GENERATOR, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
float MaxEl = 5000.0f, MinEl = 0.0f;
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
ProjHost = ProjSource;
DBHost = DBSource;
Active = ActiveSource;

WidthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
WidthADT.SetMultiplier(1.0);
WidthADT.SetIncrement(1.0);
WidthADT.SetRangeDefaults(RangeDefaults);
HeightADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
HeightADT.SetMultiplier(1.0);
HeightADT.SetIncrement(1.0);
HeightADT.SetRangeDefaults(RangeDefaults);
CellSizeADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AreaWidth = AreaHeight = LatHeight = LonWidth = 1.0;
BackgroundInstalled = 0;
SampleData = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Terrain Generator - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	Active->PreviewEnabled = 1;
	TempPreviewSize = Active->PreviewSize;
	if (Active->InitialSetup)
		{
		// see if there are valid database bounds
		if (! (DBHost->GetDEMExtents(MaxLat, MinLat, MaxLon, MinLon) && DBHost->GetDEMElevRange(MaxEl, MinEl)))
			DBHost->GetBounds(MaxLat, MinLat, MinLon, MaxLon);
		if (! (fabs(MaxLat - MinLat) < 10.0) || ! (fabs(MaxLon - MinLon) < 10.0))
			{
			MaxLat = .25;
			MinLat = 0.0;
			MaxLon = .25;
			MinLon = 0.0;
			} // if
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(WCS_max(MaxLat, MinLat));
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(WCS_min(MaxLat, MinLat));
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(WCS_max(MaxLon, MinLon));
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(WCS_min(MaxLon, MinLon));
		Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV].SetValue((double)MinEl);
		Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].SetValue((double)MaxEl - MinEl);
		} // if
	if (! (SampleData = new TerraGenSampleData()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // TerraGeneratorEditGUI::TerraGeneratorEditGUI

/*===========================================================================*/

TerraGeneratorEditGUI::~TerraGeneratorEditGUI()
{
NotifyTag Changes[2];

if (SampleData)
	delete SampleData;
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (Active)
	Active->DitchPreview(EffectsHost);
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
Changes[1] = 0;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // TerraGeneratorEditGUI::~TerraGeneratorEditGUI()

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TGN, 0);

return(0);

} // TerraGeneratorEditGUI::HandleCloseWin

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

char checked;

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TGN, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		Active->TerraType.OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		Active->TerraType.OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_CREATEDEM:
		{
		CreateDEM();
		break;
		} // IDC_GETRES
	case IDC_TBTG_NLOCK:
		{
		checked = WidgetGetCheck(IDC_TBTG_NLOCK);
		WidgetEMSetReadOnly(IDC_NORTH, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBTG_SLOCK)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_SOUTH, 0);
			WidgetSetCheck(IDC_TBTG_SLOCK, 0);
			}
		break;
		} // IDC_TG_NLOCK
	case IDC_TBTG_SLOCK:
		{
		checked = WidgetGetCheck(IDC_TBTG_SLOCK);
		WidgetEMSetReadOnly(IDC_SOUTH, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBTG_NLOCK)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_NORTH, 0);
			WidgetSetCheck(IDC_TBTG_NLOCK, 0);
			}
		break;
		} // IDC_TG_SLOCK
	case IDC_TBTG_ELOCK:
		{
		checked = WidgetGetCheck(IDC_TBTG_ELOCK);
		WidgetEMSetReadOnly(IDC_EAST, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBTG_WLOCK)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_WEST, 0);
			WidgetSetCheck(IDC_TBTG_WLOCK, 0);
			}
		break;
		} // IDC_TG_ELOCK
	case IDC_TBTG_WLOCK:
		{
		checked = WidgetGetCheck(IDC_TBTG_WLOCK);
		WidgetEMSetReadOnly(IDC_WEST, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBTG_ELOCK)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_EAST, 0);
			WidgetSetCheck(IDC_TBTG_ELOCK, 0);
			}
		break;
		} // IDC_TG_WLOCK
	default: break;
	} // ButtonID
return(0);

} // TerraGeneratorEditGUI::HandleButtonClick

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_AREADROP:
		{
		SelectNewArea();
		break;
		}
	} // switch CtrlID

return (0);

} // TerraGeneratorEditGUI::HandleCBChange

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
		DEMName();
		break;
		} // 
	} // switch CtrlID

return (0);

} // TerraGeneratorEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long TerraGeneratorEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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

} // TerraGeneratorEditGUI::HandlePageChange

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_CHECKENABLED
	case IDC_CHECKPREVIEWENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		if (! Active->PreviewEnabled)
			{
			if (Active)
				Active->DitchPreview(EffectsHost);
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
			Changes[1] = 0;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		break;
		} // IDC_CHECKPREVIEWENABLED
	} // switch CtrlID

return(0);

} // TerraGeneratorEditGUI::HandleSCChange

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_ROWS:
	case IDC_COLS:
	case IDC_ROWMAPS:
	case IDC_COLMAPS:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_SEED:
		{
		Changes[0] = MAKE_ID(Active->TerraType.GetNotifyClass(), Active->TerraType.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->TerraType.GetRAHostRoot());
		break;
		} // 
	case IDC_PREVIEWSIZE:
		{
		Active->DitchPreview(EffectsHost);
		Active->PreviewSize = TempPreviewSize;
		Changes[0] = MAKE_ID(Active->TerraType.GetNotifyClass(), Active->TerraType.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->TerraType.GetRAHostRoot());
		break;
		} // 
	case IDC_AREAWIDTH:
	case IDC_AREAHEIGHT:
		{
		SetNewArea();
		break;
		} // 
	case IDC_NORTH:
		{
		SetNewBounds(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH);
		break;
		} // 
	case IDC_SOUTH:
		{
		SetNewBounds(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH);
		break;
		} // 
	case IDC_WEST:
		{
		SetNewBounds(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST);
		break;
		} // 
	case IDC_EAST:
		{
		SetNewBounds(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST);
		break;
		} // 
	} // switch CtrlID

return(0);

} // TerraGeneratorEditGUI::HandleFIChange

/*===========================================================================*/

void TerraGeneratorEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
int Done = 0;

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
	Done = 1;
	} // if

if (! Done)
	{
	ConfigureWidgets();
	UpdateThumbnail();
	} // if
else
	UpdateThumbnail();

} // TerraGeneratorEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void TerraGeneratorEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

CalcArea();

ConfigureTB(NativeWin, IDC_TBTG_NLOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_SLOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_ELOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_WLOCK, IDI_ICONLOCKINTERSM, NULL);

ConfigureFI(NativeWin, IDC_PREVIEWSIZE,
 &TempPreviewSize,
  10.0,
   100.0,
	10000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_ROWS,
 &Active->Rows,
  1.0,
   2.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_COLS,
 &Active->Cols,
  1.0,
   2.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_ROWMAPS,
 &Active->RowMaps,
  1.0,
   1.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_COLMAPS,
 &Active->ColMaps,
  1.0,
   1.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_SEED,
 &Active->TerraType.Seed,
  100.0,
   0.0,
	100000.0,
	 FIOFlag_Long,
	  NULL,
	   0);

WidgetSetModified(IDC_DEMNAME, FALSE);
WidgetSetText(IDC_DEMNAME, Active->DEMName);

sprintf(TextStr, "Terrain Generator - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPREVIEWENABLED, &Active->PreviewEnabled, SCFlag_Char, NULL, 0);

//WidgetSNConfig(IDC_BASEELEV, &Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV]);
//WidgetSNConfig(IDC_ELEVRANGE, &Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE]);
WidgetSmartRAHConfig(IDC_BASEELEV, &Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV], &Active->TerraType);
WidgetSmartRAHConfig(IDC_ELEVRANGE, &Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE], &Active->TerraType);

WidgetSNConfig(IDC_AREAWIDTH, &WidthADT);
WidgetSNConfig(IDC_AREAHEIGHT, &HeightADT);

//WidgetSNConfig(IDC_NORTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH]);
//WidgetSNConfig(IDC_WEST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST]);
//WidgetSNConfig(IDC_EAST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST]);
//WidgetSNConfig(IDC_SOUTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH]);
WidgetSmartRAHConfig(IDC_NORTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &Active->GeoReg);
WidgetSmartRAHConfig(IDC_WEST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &Active->GeoReg);
WidgetSmartRAHConfig(IDC_EAST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], &Active->GeoReg);
WidgetSmartRAHConfig(IDC_SOUTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], &Active->GeoReg);

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);

DisableWidgets();
DisplayGridSize();

} // TerraGeneratorEditGUI::ConfigureWidgets()

/*===========================================================================*/

void TerraGeneratorEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_PREVIEWSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_ROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_COLS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_ROWMAPS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_COLMAPS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_SEED, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_AREAWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_AREAHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BASEELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVRANGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
DisplayGridSize();

} // TerraGeneratorEditGUI::SyncWidgets

/*===========================================================================*/

void TerraGeneratorEditGUI::DisplayGridSize(void)
{
long NumColCells, NumRowCells;
double CellWidth, CellHeight;
char Str[256];

NumColCells = (Active->Cols - 1) * Active->ColMaps;
NumRowCells = (Active->Rows - 1) * Active->RowMaps;

CellWidth = AreaWidth / NumColCells;
CellHeight = AreaHeight / NumRowCells;

FormatAsPreferredUnit(Str, &CellSizeADT, CellWidth);
WidgetSetText(IDC_GRIDWE, Str);
FormatAsPreferredUnit(Str, &CellSizeADT, CellHeight);
WidgetSetText(IDC_GRIDNS, Str);

} // TerraGeneratorEditGUI::DisplayGridSize

/*===========================================================================*/

void TerraGeneratorEditGUI::DisableWidgets(void)
{

} // TerraGeneratorEditGUI::DisableWidgets

/*===========================================================================*/

void TerraGeneratorEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // TerraGeneratorEditGUI::Cancel

/*===========================================================================*/

void TerraGeneratorEditGUI::Name(void)
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

} // TerraGeneratorEditGUI::Name()

/*===========================================================================*/

void TerraGeneratorEditGUI::DEMName(void)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_DEMNAME))
	{
	WidgetGetText(IDC_DEMNAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_DEMNAME, FALSE);
	strncpy(Active->DEMName, NewName, WCS_EFFECT_MAXNAMELENGTH);
	Active->DEMName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // TerraGeneratorEditGUI::DEMName()

/*===========================================================================*/

double TerraGeneratorEditGUI::GetMinDimension(void)
{
double TempSize;

TempSize = min(AreaWidth, AreaHeight);

return (TempSize);

} // TerraGeneratorEditGUI::GetMinDimension

/*===========================================================================*/

void TerraGeneratorEditGUI::CalcArea(void)
{
double MetersPerDegLat, MetersPerDegLon, PlanetRad, AvgLat;

PlanetRad = EffectsHost->GetPlanetRadius();
MetersPerDegLat = LatScale(PlanetRad);
AvgLat = (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) / 2.0;
MetersPerDegLon = LonScale(PlanetRad, AvgLat);

LonWidth = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
LatHeight = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
AreaWidth = LonWidth * MetersPerDegLon;
AreaHeight = LatHeight * MetersPerDegLat;
WidthADT.SetValue(AreaWidth);
HeightADT.SetValue(AreaHeight);
WidgetSNSync(IDC_AREAWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_AREAHEIGHT, WP_FISYNC_NONOTIFY);
DisplayGridSize();

} // TerraGeneratorEditGUI::CalcArea

/*===========================================================================*/

void TerraGeneratorEditGUI::SelectNewArea(void)
{
long Current;
double NewWidth, NewHeight, NewRange;
Texture *Tex;

Current = WidgetCBGetCurSel(IDC_AREADROP);
switch (Current)
	{
	case 0:
		{
		NewWidth = NewHeight = ConvertToMeters(208.71, WCS_USEFUL_UNIT_FEET);
		break;
		} // acre
	case 1:
		{
		NewWidth = NewHeight = 100.0;
		break;
		} // hectare
	case 2:
		{
		NewWidth = NewHeight = 1000.0;
		break;
		} // sq km
	case 3:
		{
		NewWidth = NewHeight = ConvertToMeters(1.0, WCS_USEFUL_UNIT_MILE_US_STATUTE);
		break;
		} // sq mi
	case 4:
		{
		NewWidth = NewHeight = ConvertToMeters(6.0, WCS_USEFUL_UNIT_MILE_US_STATUTE);
		break;
		} // township
	default:
		{
		NewWidth = NewHeight = 1000.0;
		break;
		} // sq km
	} // switch

HeightADT.SetValue(NewHeight);
WidthADT.SetValue(NewWidth);
NewRange = 10.0 * WCS_floor(WCS_min(NewWidth, NewHeight) * .02);
if (NewRange > 5000.0)
	NewRange = 5000.0;
Active->TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].SetValue(NewRange);
WidgetFISync(IDC_ELEVRANGE, WP_FISYNC_NONOTIFY);
if (Active->TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] &&
	(Tex = Active->TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Tex))
	{
	NewWidth *= .5;
	Active->TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->PreviewSize.SetValue(NewWidth);
	Tex->TexSize[0].SetValue(NewWidth);
	Tex->TexSize[1].SetValue(NewWidth);
	Tex->TexSize[2].SetValue(NewRange);
	} // if
SetNewArea();

} // TerraGeneratorEditGUI::SelectNewArea

/*===========================================================================*/

void TerraGeneratorEditGUI::SetNewArea(void)
{
double NewLatHeight, MetersPerDegLat, MetersPerDegLon, PlanetRad, AvgLat;
char checked;

PlanetRad = EffectsHost->GetPlanetRadius();
MetersPerDegLat = LatScale(PlanetRad);

if (AreaHeight != HeightADT.CurValue)
	{
	AreaHeight = HeightADT.CurValue;
	NewLatHeight = AreaHeight / MetersPerDegLat;

	if (checked = WidgetGetCheck(IDC_TBTG_NLOCK))
		{
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - NewLatHeight > -90.0)
			Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - NewLatHeight);
		else
			return;
		} // adjust south only
	else if (checked = WidgetGetCheck(IDC_TBTG_SLOCK))
		{
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + NewLatHeight < 90.0)
			Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + NewLatHeight);
		else
			return;
		} // adjust north only
	else if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - NewLatHeight > -90.0)
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - NewLatHeight);
	else if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + NewLatHeight < 90.0)
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + NewLatHeight);
	else
		return;
	LatHeight = NewLatHeight;
	HeightADT.SetValue(AreaHeight);
	WidgetSNSync(IDC_AREAHEIGHT, WP_FISYNC_NONOTIFY);
	} // if
if (AreaWidth != WidthADT.CurValue)
	{
	AreaWidth = WidthADT.CurValue;
	AvgLat = (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
	MetersPerDegLon = LonScale(PlanetRad, AvgLat);
	LonWidth = AreaWidth / MetersPerDegLon;

	if (checked = WidgetGetCheck(IDC_TBTG_ELOCK))
		{
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue + LonWidth);
		} // adjust west only
	else
		Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - LonWidth);
	WidthADT.SetValue(AreaWidth);
	WidgetSNSync(IDC_AREAWIDTH, WP_FISYNC_NONOTIFY);
	} // if

} // TerraGeneratorEditGUI::SetNewArea

/*===========================================================================*/

void TerraGeneratorEditGUI::SetNewBounds(long VarChanged)
{

// change north or west, modify east or south
if (VarChanged == WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)
	{
	Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - LatHeight);
	// changing latitude of DEM causes need to resize EW dimension to preserve user's width specification
	AreaWidth = 1.0;	// triggers recalculation
	SetNewArea();	// recalculates area width
	} // if
else if (VarChanged == WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)
	{
	Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetCurValue(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - LonWidth);
	} // else if

// change east or south, modify width or height
if (VarChanged == WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)
	{
	CalcArea();
	} // if
else if (VarChanged == WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)
	{
	CalcArea();
	} // else if

} // TerraGeneratorEditGUI::SetNewBounds

/*===========================================================================*/

void TerraGeneratorEditGUI::CreateDEM(void)
{

//Active->PreviewEnabled = 0;
if (Active)
	Active->DitchPreview(EffectsHost);
WidgetSCSync(IDC_CHECKPREVIEWENABLED, WP_SCSYNC_NONOTIFY);
Active->DoSomethingConstructive(ProjHost, DBHost);

} // TerraGeneratorEditGUI::CreateDEM

/*===========================================================================*/

long TerraGeneratorEditGUI::HandleBackgroundCrunch(int Siblings)
{
int Handled = 0;
NotifyTag Changes[4];

if (SampleData->Running)
	{
	if (Active->EvalOneSampleLine(SampleData))
		{
		// old database-notify. Doesn't seem necessary.
		// Notify floating things of presence of new objects
		if (Active->PreviewJoe)
			{
			Changes[0] = MAKE_ID(Active->PreviewJoe->GetNotifyClass(), Active->PreviewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			// Notify Views to regen dynamic entities
			Changes[1] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
			Changes[2] = 0;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->PreviewJoe->GetRAHostRoot());
			} // if
		} // if
	Handled = 1;
	} // if

if (! Handled)
	{
	// we're all done
	BackgroundInstalled = 0;
	return (1);		// 1 uninstalls background process
	} // if

// we've got more to do, come again when you have more time
return (0);

} // TerraGeneratorEditGUI::HandleBackgroundCrunch

/*===========================================================================*/

void TerraGeneratorEditGUI::UpdateThumbnail(void)
{

if (Active->EvalSampleInit(SampleData, EffectsHost, ProjHost, DBHost))
	{
	if (! BackgroundInstalled)
		{
		if (GlobalApp->AddBGHandler(this))
			BackgroundInstalled = 1;
		} // if
	} // if initialized

} // TerraGeneratorEditGUI::UpdateThumbnail

/*===========================================================================*/

int TerraGeneratorEditGUI::CreatingInitialSetup(void)
{

return (Active ? Active->InitialSetup: 0);

} // TerraGeneratorEditGUI::CreatingInitialSetup
