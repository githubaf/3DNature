// DEMPaintGUI.cpp
// Code for DEM painter
// Created from DEMPaintGUI 04/12/02 FPW2
// Copyright 2002 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "DEMPaintGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Conservatory.h"
#include "Joe.h"
#include "DEM.h"
#include "AppMem.h"
#include "resource.h"
#include "Raster.h"

long DEMPaintGUI::ActivePanel;
static long DamageXMin, DamageXMax, DamageYMin, DamageYMax, LMBDown;

/*===========================================================================*/

bool DEMPaintGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANUNDO:
	case WCS_FENETRE_WINCAP_CANSAVE:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // DEMPaintGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin DEMPaintGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	HandleReSized(0, 0, 0); // our resize handler gets its own coords, these are dummies
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // DEMPaintGUI::Open

/*===========================================================================*/

NativeGUIWin DEMPaintGUI::Construct(void)
{

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DEMPAINT, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_DEMPAINT_AREA, 0, 0, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_CONTROL, 1, 0, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_TOOLS, 2, 0, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_OPTIONS, 2, 1, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_BRUSHES, 2, 2, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_ELEVATIONS, 2, 3, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_THUMBNAIL, 3, 0, false);
	CreateSubWinFromTemplate(IDD_DEMPAINT_ELEVGRAD, 4, 0, false);

	if (NativeWin)
		{
		WidgetSetScrollRange(IDC_DPG_HPOS, 0, 100);
		WidgetSetScrollRange(IDC_DPG_VPOS, 0, 100);
		WidgetSetScrollRange(IDC_DPG_ZOOM, 10, 100);
		WidgetSetScrollRange(IDC_DPG_BRUSHSIZE, 0, 100);
		WidgetSetScrollPos(IDC_DPG_ZOOM, (long)(Elevs.Visible * 100));
		WidgetSetScrollPos(IDC_DPG_BRUSHSIZE, (long)BrushScale);
		WidgetGetSize(IDC_DPG_AREA, WidSize[0][0], WidSize[0][1]);
		WidgetGetSize(IDC_DPG_THUMB, WidSize[1][0], WidSize[1][1]);
		WidgetGetSize(IDC_DPG_ELEVGRAD, WidSize[2][0], WidSize[2][1]);
		WidSize[0][0] -= 4;
		WidSize[0][1] -= 4;
		WidSize[1][0] -= 4;
		WidSize[1][1] -= 4;
		WidSize[2][0] -= 4;
		WidSize[2][1] -= 4;
		if (Active)
			NewDEMFile();
		ShowPanel(0, 0);
		ShowPanel(1, 0);
		ShowPanel(2, ActivePanel);
		ShowPanel(3, 0);
		ShowPanel(4, 0);

		WidgetSetCheck(IDC_TOOLS, ActivePanel == 0);
		WidgetSetCheck(IDC_OPTIONS, ActivePanel == 1);
		WidgetSetCheck(IDC_BRUSHES, ActivePanel == 2);
		WidgetSetCheck(IDC_ELEVATIONS, ActivePanel == 3);

		SetSNDefaults();
		ConfigureWidgets();
		ConfigureOverlays();
		SetPaintMode2(PaintMode);
		SetBrush(ActiveBrush);
		DrawForegroundIndicator();
		DrawAreaOverlays();
		MaskMode = DPG_MASKMODE_NONE;
		memset(MaskMap, 255, ElevCols * ElevRows);
		UpdatePreview();
		DamageXMax = DamageYMax = 0;
		DamageXMin = DamageYMin = LONG_MAX;
		} // if
	} // if
 
return (NativeWin);

} // DEMPaintGUI::Construct

/*===========================================================================*/

DEMPaintGUI::DEMPaintGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, Joe *ActiveSource)
: GUIFenetre('DEMP', this, "DEM Painter") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];
double RangeDefaults[3];
long x, y;
unsigned char *ptr;
unsigned char val;
JoeCoordSys *MyAttr;

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
LMBDown = 0;
SavedRegion = 0;
BrushDef = NULL;
BrushOutline = NULL;
BrushRegion = NULL;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
Active = ActiveSource;
RowOffset = ColOffset = 0;
ElevCols = ElevRows = 0;
ActiveBrush = 0;
ActiveModified = 0;
ActiveDEM = NULL;
BrushWorkArea = NULL;
RevertMap = NULL;
UndoMap = NULL;
WorkMap = NULL;
FloodMap = NULL;
MaskMap = NULL;
ModifierBaseMap = NULL;
ModifierMap = NULL;
NullMap = NULL;
TmpMaskMap = NULL;
PaintCoords = NULL;
PreviewJoe = NULL;
MyDEM = NULL;
Solo = 0;
Previewing = 0;
NULLValue = -9999;
Opacity = 100.0f;
boxx0 = boxy0 = 0.0f;
boxx1 = boxy1 = 1.0f;
DefiningMask = false;
MakeTable = false;
MaskNulls = true;	// only VNS pays attention to this
PaintRefresh = false;
RubberBand = false;
DefaultSettingsValid = false;
memset(&Elevs, 0, sizeof (struct RasterWidgetData));
memset(&Gradient, 0, sizeof (struct RasterWidgetData));
memset(&Thumb, 0, sizeof (struct RasterWidgetData));
Elevs.Visible = 1.0;
Gradient.Visible = 1.0;
Thumb.Visible = 1.0;
Elevs.RDWin = this;
Gradient.RDWin = this;
Thumb.RDWin = this;
Elevs.OverRast = new Raster();
Gradient.MainRast = new Raster();
Gradient.OverRast = new Raster();
Thumb.OverRast = new Raster();
Gradient.MainRast->Cols = 16;
Gradient.MainRast->Rows = 256;
Gradient.MainRast->ByteBands = 3;
if (Gradient.MainRast->AllocByteBand(0))
	{
	Gradient.MainRast->ByteMap[1] = Gradient.MainRast->ByteMap[0];
	Gradient.MainRast->ByteMap[2] = Gradient.MainRast->ByteMap[0];
	ptr = Gradient.MainRast->ByteMap[0];
	for (y = 0; y < 256; y++)
		{
		val = 255 - (unsigned char)y;
		for (x = 0; x < 16; x++)
			{
			*ptr++ = val;
			} // for x
		} // for y
	} // if
Gradient.OverRast->Cols = 16;
Gradient.OverRast->Rows = 128;
Gradient.OverRast->ByteBands = 3;
if (Gradient.OverRast->AllocByteBand(0))
	{
	Gradient.OverRast->ClearByteBand(0);
	if (Gradient.OverRast->AllocByteBand(1))
		{
		Gradient.OverRast->ClearByteBand(1);
		if (Gradient.OverRast->AllocByteBand(2))
			{
			Gradient.OverRast->ClearByteBand(2);
			} // if
		} // if
	} // if

RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = -FLT_MAX;
RangeDefaults[2] = 1.0;
NorthADT.SetRangeDefaults(RangeDefaults);
NorthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
NorthADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
SouthADT.SetRangeDefaults(RangeDefaults);
SouthADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
SouthADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
WestADT.SetRangeDefaults(RangeDefaults);
WestADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
WestADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
EastADT.SetRangeDefaults(RangeDefaults);
EastADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
EastADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

if (EffectsSource && DBSource && ProjHost)
	{
	if (Active)
		{
		strcpy(OrigName, Active->GetBestName());
		sprintf(NameStr, "DEM Painter - %s", OrigName);
		}
	else
		strcpy(NameStr, "DEM Painter");
	SetTitle(NameStr);
	//Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	if (ActiveDEM = new DEM)
		{
		if (Active)
			{
			if (ActiveDEM->AttemptLoadDEM(Active, 1, ProjHost))
				{
				if (ActiveDEM->MoJoe && (MyAttr = (JoeCoordSys *)ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
					PaintCoords = MyAttr->Coord;
//				else
//					PaintCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
				CopyFromPrefsSettings();
				if (! DefaultSettingsValid)
					{
					ActiveBrush = 0;
					ForeElev = ActiveDEM->MaxEl();
					BackElev = ActiveDEM->MinEl();
					Tolerance = (ForeElev - BackElev) * 0.01;
					LastPaintMode = PaintMode = DPG_MODE_PAINT;
					Effect = DPG_EFFECT_ABS_RAISELOWER;
					GradMode = DPG_GRADMODE_LINEAR;
					BrushScale = 0.0f;
					} // if
				ElevCols =  ActiveDEM->LonEntries();
				ElevRows = ActiveDEM->LatEntries();
				MaxElev = ActiveDEM->MaxEl();
				MinElev = ActiveDEM->MinEl();
				MakeLUT();
				ModifierBaseMap = (float *)AppMem_Alloc(ElevCols * ElevRows * sizeof(float), 0L);
				if (!ModifierBaseMap)
					ConstructError = 1;
				ModifierMap = (float *)AppMem_Alloc(ElevCols * ElevRows * sizeof(float), 0L);
				if (!ModifierMap)
					ConstructError = 1;
				RevertMap = (float *)AppMem_Alloc(ElevCols * ElevRows * sizeof(float), 0L);
				if (RevertMap)
					memcpy(RevertMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));
				else
					ConstructError = 1;
				UndoMap = (float *)AppMem_Alloc(ElevCols * ElevRows * sizeof(float), 0L);
				if (UndoMap)
					memcpy(UndoMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));
				else
					ConstructError = 1;
				WorkMap = (float *)AppMem_Alloc(ElevCols * ElevRows * sizeof(float), 0L);
				if (!WorkMap)
					ConstructError = 1;
				MaskMap = (unsigned char *)AppMem_Alloc(ElevCols * ElevRows, APPMEM_CLEAR);
				if (! MaskMap)
					ConstructError = 1;
				TmpMaskMap = (unsigned char *)AppMem_Alloc(ElevCols * ElevRows, APPMEM_CLEAR);
				if (! TmpMaskMap)
					ConstructError = 1;
#ifdef WCS_BUILD_VNS
				NullMap = (unsigned char *)AppMem_Alloc(ElevCols * ElevRows, APPMEM_CLEAR);
				if (! NullMap)
					ConstructError = 1;
#endif // WCS_BUILD_VNS
				// 1 pixel border around normal area for DPG_MASKMODE_FREEHAND
				FloodMap = (unsigned char *)AppMem_Alloc((ElevCols + 2) * (ElevRows + 2), APPMEM_CLEAR);
				if (! FloodMap)
					ConstructError = 1;
				// 1 pixel border around centered brush
				BrushWorkArea = (float *)AppMem_Alloc((MAX_DPG_BRUSH_SIZE + 2) * (MAX_DPG_BRUSH_SIZE + 2) * sizeof(float), 0L);
				if (! BrushWorkArea)
					ConstructError = 1;
				BrushDef = (unsigned char *)AppMem_Alloc(MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float), 0L);
				if (! BrushDef)
					ConstructError = 1;
				BrushOutline = (unsigned char *)AppMem_Alloc(MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float), 0L);
				if (! BrushOutline)
					ConstructError = 1;
				BrushRegion = (unsigned char *)AppMem_Alloc(MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float) * 3, 0L);
				if (! BrushRegion)
					ConstructError = 1;
				DefaultSettingsValid = true;
				if (ActiveDEM->MoJoe)
					{
					JoeFlags = ActiveDEM->MoJoe->GetFlags();
					ActiveDEM->MoJoe->SetFlags(JoeFlags & ~WCS_JOEFLAG_DRAWENABLED);
					}
				} // if
			else
				ConstructError = 1;
			} // if
		} // if
	else
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // DEMPaintGUI::DEMPaintGUI

/*===========================================================================*/

DEMPaintGUI::~DEMPaintGUI()
{
NotifyTag Changes[2];

CopyToPrefsSettings();
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (BrushRegion)
	AppMem_Free(BrushRegion, MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float) * 3);
if (BrushOutline)
	AppMem_Free(BrushOutline, MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float));
if (BrushDef)
	AppMem_Free(BrushDef, MAX_DPG_BRUSH_SIZE * MAX_DPG_BRUSH_SIZE * sizeof(float));
if (BrushWorkArea)
	AppMem_Free(BrushWorkArea, (MAX_DPG_BRUSH_SIZE + 2) * (MAX_DPG_BRUSH_SIZE + 2) * sizeof(float));
if (FloodMap)
	AppMem_Free(FloodMap, (ElevCols + 2) * (ElevRows + 2));
if (TmpMaskMap)
	AppMem_Free(TmpMaskMap, ElevCols * ElevRows);
if (MaskMap)
	AppMem_Free(MaskMap, ElevCols * ElevRows);
if (NullMap)
	AppMem_Free(NullMap, ElevCols * ElevRows);
if (UndoMap)
	AppMem_Free(UndoMap, ElevCols * ElevRows * sizeof(float));
if (WorkMap)
	AppMem_Free(WorkMap, ElevCols * ElevRows * sizeof(float));
if (RevertMap)
	AppMem_Free(RevertMap, ElevCols * ElevRows * sizeof(float));
if (ModifierMap)
	AppMem_Free(ModifierMap, ElevCols * ElevRows * sizeof(float));
if (ModifierBaseMap)
	AppMem_Free(ModifierBaseMap, ElevCols * ElevRows * sizeof(float));
if (ActiveDEM)
	delete ActiveDEM;
if (Elevs.OverRast)
	delete Elevs.OverRast;
if (Gradient.OverRast)
	delete Gradient.OverRast;
if (Gradient.MainRast)
	{
	// these all point to the same raster, so don't free it thrice!
	Gradient.MainRast->ByteMap[1] = NULL;
	Gradient.MainRast->ByteMap[2] = NULL;
	delete Gradient.MainRast;
	}
if (Thumb.OverRast)
	delete Thumb.OverRast;
if (Elevs.MainRast)
	{
	// these all point to the same raster, so don't free it thrice!
	Elevs.MainRast->ByteMap[1] = NULL;
	Elevs.MainRast->ByteMap[2] = NULL;
	Thumb.MainRast = NULL;	// it's was shared with Elevs.MainRast
	delete Elevs.MainRast;
	}
DumpPreview();
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
Changes[1] = 0;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // DEMPaintGUI::~DEMPaintGUI()

/*===========================================================================*/

void DEMPaintGUI::CopyFromPrefsSettings(void)
{

if (ProjHost && ProjHost->Prefs.PaintDefaultsValid)
	{
	PaintMode = (DPG_MODE)ProjHost->Prefs.PaintMode;
	GradMode = (DPG_GRADMODE)ProjHost->Prefs.GradMode;
	ActiveBrush = ProjHost->Prefs.ActiveBrush;
	Effect = (DPG_EFFECT)ProjHost->Prefs.Effect;
	Tolerance = ProjHost->Prefs.Tolerance;
	ForeElev = ProjHost->Prefs.ForeElev;
	BackElev = ProjHost->Prefs.BackElev;
	BrushScale = ProjHost->Prefs.BrushScale;
	Opacity = ProjHost->Prefs.Opacity;
	DefaultSettingsValid = true;
	} // if

} // DEMPaintGUI::CopyFromPrefsSettings

/*===========================================================================*/

void DEMPaintGUI::CopyToPrefsSettings()
{

if (ProjHost && DefaultSettingsValid)
	{
	if (MaskMode == DPG_MASKMODE_NONE)
		ProjHost->Prefs.PaintMode = PaintMode;
	else
		ProjHost->Prefs.PaintMode = LastPaintMode;
	ProjHost->Prefs.GradMode = GradMode;
	ProjHost->Prefs.ActiveBrush = ActiveBrush;
	ProjHost->Prefs.Effect = Effect;
	ProjHost->Prefs.Tolerance = Tolerance;
	ProjHost->Prefs.ForeElev = ForeElev;
	ProjHost->Prefs.BackElev = BackElev;
	ProjHost->Prefs.BrushScale = BrushScale;
	ProjHost->Prefs.Opacity = Opacity;
	ProjHost->Prefs.PaintDefaultsValid = true;
	} // if

} // DEMPaintGUI::CopyToPrefsSettings

/*===========================================================================*/

void DEMPaintGUI::ConfigureOverlays(void)
{

Elevs.OverRast->Cols = ElevCols;
Elevs.OverRast->Rows = ElevRows;
Elevs.OverRast->ByteBands = 3;
if (Elevs.OverRast->AllocByteBand(0))
	{
	Elevs.OverRast->ClearByteBand(0);
	if (Elevs.OverRast->AllocByteBand(1))
		{
		Elevs.OverRast->ClearByteBand(1);
		if (Elevs.OverRast->AllocByteBand(2))
			{
			Elevs.OverRast->ClearByteBand(2);
			} // if
		} // if
	} // if

Thumb.OverRast->Rows = WidSize[1][0];
Thumb.OverRast->Cols = WidSize[1][1];
Thumb.OverRast->ByteBands = 3;
if (Thumb.OverRast->AllocByteBand(0))
	{
	Thumb.OverRast->ClearByteBand(0);
	if (Thumb.OverRast->AllocByteBand(1))
		{
		Thumb.OverRast->ClearByteBand(1);
		if (Thumb.OverRast->AllocByteBand(2))
			{
			Thumb.OverRast->ClearByteBand(2);
			} // if
		} // if
	} // if

} // DEMPaintGUI::ConfigureOverlays

/*===========================================================================*/

void DEMPaintGUI::ComputeThumbOverlay(void)
{
long Cols, Rows, X, Y, Zip;
long left, right, top, bottom;
unsigned char *Red, *Green, *Blue;

Rows = Thumb.OverRast->Rows;
Cols = Thumb.OverRast->Cols;
Red = Thumb.OverRast->ByteMap[0];
Green = Thumb.OverRast->ByteMap[1];
Blue = Thumb.OverRast->ByteMap[2];

Thumb.OverRast->ClearByteBand(0);
Thumb.OverRast->ClearByteBand(1);
Thumb.OverRast->ClearByteBand(2);

// see where we are in the big picture
left = (long)(Elevs.OffsetX * (Cols - 1));
right = (long)((Elevs.OffsetX + Elevs.Visible) * (Cols - 3));
top = (long)(Elevs.OffsetY * (Rows - 1));
bottom = (long)((Elevs.OffsetY + Elevs.Visible) * (Rows - 3));

// draw a box showing where we're looking
Zip = top * Cols + left;
for (X = left; X <= right; X++, Zip++)
	{
	Red[Zip] = (unsigned char)(127);
	Green[Zip] = (unsigned char)(255);
	Blue[Zip] = (unsigned char)(0);
	} // for
Zip = bottom * Cols + left;
for (X = left; X <= right; X++, Zip++)
	{
	Red[Zip] = (unsigned char)(127);
	Green[Zip] = (unsigned char)(255);
	Blue[Zip] = (unsigned char)(0);
	} // for
for (Y = top; Y <= bottom; Y++)
	{
	Zip = Y * Cols + left;
	Red[Zip] = (unsigned char)(127);
	Green[Zip] = (unsigned char)(255);
	Blue[Zip] = (unsigned char)(0);
	} // for
for (Y = top; Y <= bottom; Y++)
	{
	Zip = Y * Cols + right;
	Red[Zip] = (unsigned char)(127);
	Green[Zip] = (unsigned char)(255);
	Blue[Zip] = (unsigned char)(0);
	} // for

ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);

} // DEMPaintGUI::ComputeThumbOverlay

/*===========================================================================*/

void DEMPaintGUI::DrawAreaOverlays(void)
{
long dx, dy, xdir;
long bottom, left, right, top;
long Cols, Rows, Zip;
long i, tmp, x0, x1, y0, y1;
unsigned char *Red, *Green, *Blue;
unsigned char octant;

Cols = Elevs.OverRast->Cols;
Rows = Elevs.OverRast->Rows;
Elevs.OverRast->ClearByteBand(0);
Elevs.OverRast->ClearByteBand(1);
Elevs.OverRast->ClearByteBand(2);
Red = Elevs.OverRast->ByteMap[0];
Green = Elevs.OverRast->ByteMap[1];
Blue = Elevs.OverRast->ByteMap[2];


if ((MaskMode == DPG_MASKMODE_BOX) || (MaskMode == DPG_MASKMODE_FREEHAND) || (MaskMode == DPG_MASKMODE_WAND))
	{
	DrawMaskMap();
	} // if DPG_MASKMODE_FREEHAND or DPG_MASKMODE_WAND

if (DefiningMask && (MaskMode == DPG_MASKMODE_BOX))
	{
	left = (long)(min(boxx0, boxx1) * (Cols - 1) + 0.5f);
	right = (long)(max(boxx0, boxx1) * (Cols - 1) + 0.5f);
	top = (long)(min(boxy0, boxy1) * (Rows - 1) + 0.5f);
	bottom = (long)(max(boxy0, boxy1) * (Rows - 1) + 0.5f);
	Zip = top * Cols + left;
	for (i = left; i <= right; i++, Zip++)
		{
		Red[Zip] = (unsigned char)(0);
		Green[Zip] = (unsigned char)(127);
		Blue[Zip] = (unsigned char)(255);
		} // for
	Zip = bottom * Cols + left;
	for (i = left; i <= right; i++, Zip++)
		{
		Red[Zip] = (unsigned char)(0);
		Green[Zip] = (unsigned char)(127);
		Blue[Zip] = (unsigned char)(255);
		} // for
	for (i = top; i <= bottom; i++)
		{
		Zip = i * Cols + left;
		Red[Zip] = (unsigned char)(0);
		Green[Zip] = (unsigned char)(127);
		Blue[Zip] = (unsigned char)(255);
		} // for
	for (i = top; i <= bottom; i++)
		{
		Zip = i * Cols + right;
		Red[Zip] = (unsigned char)(0);
		Green[Zip] = (unsigned char)(127);
		Blue[Zip] = (unsigned char)(255);
		} // for
	} // if DPG_MASKMODE_BOX
else if (DefiningMask && (MaskMode == DPG_MASKMODE_FREEHAND))
	{
	DrawFreehandVerts();
	} // else if

if (RubberBand)
	{
	// basic Bresenham using 2 octants
	x0 = (long)(rbx0 * Cols + 0.5f);
	x1 = (long)(rbx1 * Cols + 0.5f);
	y0 = (long)(rby0 * Rows + 0.5f);
	y1 = (long)(rby1 * Rows + 0.5f);

	if (y0 > y1)
		{
		tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
		} // if
	dx = x1 - x0;
	dy = y1 - y0;
	if (dx > 0)
		{
		xdir = 1;
		if (dx > dy)
			octant = 0;
		else
			octant = 1;
		} // if dx
	else
		{
		dx = -dx;	// ie: absolute value
		xdir = -1;
		if (dx >dy)
			octant = 0;
		else
			octant = 1;
		} // else

	// draw the first pixel
	Zip = y0 * Cols + x0;
	Red[Zip] = (unsigned char)(255);
	Green[Zip] = (unsigned char)(79);
	Blue[Zip] = (unsigned char)(0);
	// now finish drawing line via octant logic
	if (octant == 0)
		{
		long DYx2, DYx2SubDXx2, Err;

		DYx2 = dy * 2;
		DYx2SubDXx2 = DYx2 - dx * 2;
		Err = DYx2 - dx;

		while (dx--)
			{
			if (Err >= 0)
				{
				y0++;
				Err += DYx2SubDXx2;
				} // if
			else
				Err += DYx2;
			x0 += xdir;
			Zip = y0 * Cols + x0;
			Red[Zip] = (unsigned char)(255);
			Green[Zip] = (unsigned char)(79);
			Blue[Zip] = (unsigned char)(0);
			} // while
		} // octant 0
	else
		{
		long DXx2, DXx2SubDYx2, Err;

		DXx2 = dx * 2;
		DXx2SubDYx2 = DXx2 - dy * 2;
		Err = DXx2 - dy;

		while (dy--)
			{
			if (Err >= 0)
				{
				x0 += xdir;
				Err += DXx2SubDYx2;
				}
			else
				Err += DXx2;
			y0++;
			Zip = y0 * Cols + x0;
			Red[Zip] = (unsigned char)(255);
			Green[Zip] = (unsigned char)(79);
			Blue[Zip] = (unsigned char)(0);
			} // while
		} // octant 1

	} // if RubberBand

#ifdef WCS_BUILD_VNS
Zip = 0;
for (y0 = 0; y0 < Rows; y0++)
	{
	for (x0 = 0; x0 < Cols; x0++, Zip++)
		{
		if (NullMap[Cols * y0 + x0])
			{
			Red[Zip] = (unsigned char)(255);
			Green[Zip] = (unsigned char)(0);
			Blue[Zip] = (unsigned char)(0);
			} // if
		} // for
	} // for
#endif // WCS_BUILD_VNS

ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);

} // DEMPaintGUI::DrawAreaOverlays

/*===========================================================================*/

void DEMPaintGUI::DumpPreview(void)
{
NotifyTag Changes[2];

if (MyDEM)
	MyDEM->Pristine = NULL;
if (PreviewJoe)
	{
	// remove from database
	if (PreviewJoe->RemoveMe(EffectsHost))
		{
		delete PreviewJoe;
		} // if
	// null the local pointer anyway because deletion is now in the hands of the 
	// database if it wasn't successfully removed
	PreviewJoe = NULL;
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
	Changes[1] = 0;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
MyDEM = NULL;

} // DEMPaintGUI::DumpPreview

/*===========================================================================*/

void DEMPaintGUI::SetPaintMode2(DPG_MODE Mode)
{
int ButtonID;

if (Mode == DPG_MODE_PAINT)
	ButtonID = IDC_PAINTTOOL_PAINT;
else if (Mode == DPG_MODE_EYEDROP)
	ButtonID = IDC_PAINTTOOL_EYEDROPPER;
else if (Mode == DPG_MODE_FILL)
	ButtonID = IDC_PAINTTOOL_FILL;
else if (Mode == DPG_MODE_GRADIENT)
	ButtonID = IDC_PAINTTOOL_GRADIENT;
else if (Mode == DPG_MODE_ERASE)
	ButtonID = IDC_PAINTTOOL_ERASE;
else if (Mode == DPG_MODE_SMOOTH)
	ButtonID = IDC_PAINTTOOL_SMOOTH;
else if (Mode == DPG_MODE_ROUGHEN)
	ButtonID = DPG_MODE_ROUGHEN;
else if (Mode == DPG_MODE_SMEAR)
	ButtonID = IDC_PAINTTOOL_SMEAR;
else
	ButtonID = IDC_PAINTTOOL_PAINT;	// catch-all - shouldn't hit this

SetPaintMode(ButtonID);

} // DEMPaintGUI::SetPaintMode2

/*===========================================================================*/

void DEMPaintGUI::SetPaintMode(int ButtonID)
{
bool SetOpts = true;

DefiningMask = false;
switch (ButtonID)
	{
	case IDC_PAINTTOOL_ERASE:
		LastPaintMode = PaintMode = DPG_MODE_ERASE;
		break;
	case IDC_PAINTTOOL_EYEDROPPER:
		LastPaintMode = PaintMode = DPG_MODE_EYEDROP;
		break;
	case IDC_PAINTTOOL_FILL:
		LastPaintMode = PaintMode = DPG_MODE_FILL;
		break;
	case IDC_PAINTTOOL_FREESELECT:
		if ((PaintMode == DPG_MODE_MASK) && (MaskMode == DPG_MASKMODE_FREEHAND))
			{
			PaintMode = LastPaintMode;
			MaskMode = DPG_MASKMODE_NONE;
			memset(MaskMap, 255, ElevCols * ElevRows);
			DrawAreaOverlays();
			} // if
		else
			{
			PaintMode = DPG_MODE_MASK;
			MaskMode = DPG_MASKMODE_FREEHAND;
			DefiningMask = true;
			} // else
		break;
	case IDC_PAINTTOOL_GRADIENT:
		LastPaintMode = PaintMode = DPG_MODE_GRADIENT;
		break;
	case IDC_PAINTTOOL_MOVEREGION:
		if ((PaintMode == DPG_MODE_MASK) && (MaskMode == DPG_MASKMODE_MOVE))
			{
			PaintMode = LastPaintMode;
			MaskMode = DPG_MASKMODE_NONE;
			memset(MaskMap, 255, ElevCols * ElevRows);
			DrawAreaOverlays();
			} // if
		else
			{
			PaintMode = DPG_MODE_MASK;
			MaskMode = DPG_MASKMODE_MOVE;
			DefiningMask = true;
			} // else
		break;
	case IDC_PAINTTOOL_PAINT:
		LastPaintMode = PaintMode = DPG_MODE_PAINT;
		break;
	case IDC_PAINTTOOL_REGION:
		if ((PaintMode == DPG_MODE_MASK) && (MaskMode == DPG_MASKMODE_BOX))
			{
			PaintMode = LastPaintMode;
			MaskMode = DPG_MASKMODE_NONE;
			memset(MaskMap, 255, ElevCols * ElevRows);
			DrawAreaOverlays();
			} // if
		else
			{
			PaintMode = DPG_MODE_MASK;
			MaskMode = DPG_MASKMODE_BOX;
			DefiningMask = true;
			} // else
		break;
	case IDC_PAINTTOOL_ROUGHEN:
		LastPaintMode = PaintMode = DPG_MODE_ROUGHEN;
		break;
	case IDC_PAINTTOOL_SELECT:
		if ((PaintMode == DPG_MODE_MASK) && (MaskMode == DPG_MASKMODE_WAND))
			{
			PaintMode = LastPaintMode;
			MaskMode = DPG_MASKMODE_NONE;
			memset(MaskMap, 255, ElevCols * ElevRows);
			DrawAreaOverlays();
			} // if
		else
			{
			PaintMode = DPG_MODE_MASK;
			MaskMode = DPG_MASKMODE_WAND;
			DefiningMask = true;
			} // else
		break;
	case IDC_PAINTTOOL_SMEAR:
		LastPaintMode = PaintMode = DPG_MODE_SMEAR;
		break;
	case IDC_PAINTTOOL_SMOOTH:
		LastPaintMode = PaintMode = DPG_MODE_SMOOTH;
		break;
	default:
		SetOpts = false;
		break;
	} // switch

if (SetOpts)
	SetOptions(ButtonID);

WidgetSetCheck(IDC_PAINTTOOL_REGION, MaskMode == DPG_MASKMODE_BOX);
WidgetSetCheck(IDC_PAINTTOOL_MOVEREGION, MaskMode == DPG_MASKMODE_MOVE);
WidgetSetCheck(IDC_PAINTTOOL_FREESELECT, MaskMode == DPG_MASKMODE_FREEHAND);
WidgetSetCheck(IDC_PAINTTOOL_SELECT, MaskMode == DPG_MASKMODE_WAND);

WidgetSetCheck(IDC_PAINTTOOL_PAINT, PaintMode == DPG_MODE_PAINT);
WidgetSetCheck(IDC_PAINTTOOL_ERASE, PaintMode == DPG_MODE_ERASE);
WidgetSetCheck(IDC_PAINTTOOL_SMOOTH, PaintMode == DPG_MODE_SMOOTH);
WidgetSetCheck(IDC_PAINTTOOL_ROUGHEN, PaintMode == DPG_MODE_ROUGHEN);
WidgetSetCheck(IDC_PAINTTOOL_FILL, PaintMode == DPG_MODE_FILL);
WidgetSetCheck(IDC_PAINTTOOL_EYEDROPPER, PaintMode == DPG_MODE_EYEDROP);
WidgetSetCheck(IDC_PAINTTOOL_SMEAR, PaintMode == DPG_MODE_SMEAR);
WidgetSetCheck(IDC_PAINTTOOL_GRADIENT, PaintMode == DPG_MODE_GRADIENT);

ConfigureToolsBrushes();

} // DEMPaintGUI::SetPaintMode

/*===========================================================================*/

void DEMPaintGUI::SetBrush(unsigned long BrushNum)
{
float size, hiweight, loweight, normsize;

loweight = (100.0f - BrushScale) / 100.0f;
hiweight = 1.0f - loweight;

ActiveBrush = BrushNum;
switch (BrushNum)
	{
	default:
	case 0:
		normsize =  1.0f;
		size = normsize * loweight + 7.0f * hiweight;
		break;
	case 1:
		normsize = 2.0f;
		size = normsize * loweight + 15.0f * hiweight;
		break;
	case 2:
		normsize = 3.0f;
		size = normsize * loweight + 31.0f * hiweight;
		break;
	case 3:
		normsize = 7.0f;
		size = normsize * loweight + 63.0f * hiweight;
		break;
	case 4:
		normsize = 15.0f;
		size = normsize * loweight + 127.0f * hiweight;
		break;
	case 5:
		normsize = 31.0f;
		size = normsize * loweight + 255.0f * hiweight;
		break;
	case 6:
		normsize = 7.0f;
		size = normsize * loweight + 63.0f * hiweight;
		break;
	case 7:
		normsize = 15.0f;
		size = normsize * loweight + 127.0f * hiweight;
		break;
	case 8:
		normsize = 31.0f;
		size = normsize * loweight + 255.0f * hiweight;
		break;
	case 9:
		normsize = 31.0f;
		size = normsize * loweight + 255.0f * hiweight;
		break;
	case 10:
		normsize = 31.0f;
		size = normsize * loweight + 255.0f * hiweight;
		break;
	} // switch
// common to all the case statements
BrushWidth = BrushHeight = (unsigned short)(size + 0.5f);
CreateBrushMap(ActiveBrush, BrushWidth);

// highlight active brush
WidgetSetCheck(IDC_BRUSH0, ActiveBrush == 0);
WidgetSetCheck(IDC_BRUSH1, ActiveBrush == 1);
WidgetSetCheck(IDC_BRUSH2, ActiveBrush == 2);
WidgetSetCheck(IDC_BRUSH3, ActiveBrush == 3);
WidgetSetCheck(IDC_BRUSH4, ActiveBrush == 4);
WidgetSetCheck(IDC_BRUSH5, ActiveBrush == 5);
WidgetSetCheck(IDC_BRUSH6, ActiveBrush == 6);
WidgetSetCheck(IDC_BRUSH7, ActiveBrush == 7);
WidgetSetCheck(IDC_BRUSH8, ActiveBrush == 8);
WidgetSetCheck(IDC_BRUSH9, ActiveBrush == 9);
WidgetSetCheck(IDC_BRUSH10, ActiveBrush == 10);
WidgetSetCheck(IDC_BRUSH11, ActiveBrush == 11);

ConfigureToolsBrushes();

} // DEMPaintGUI::SetBrush

/*===========================================================================*/

long DEMPaintGUI::HandleCloseWin(NativeGUIWin NW)
{

if (memcmp(ActiveDEM->Map(), RevertMap, ElevCols * ElevRows * sizeof(float)))
	{
	if (UserMessageYN("DEM Paint", "Would you like to save your modified DEM?"))
		{
		SaveDEM();
		} // if
	} // if the DEM was modified

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DPG, 0);

return(0);

} // DEMPaintGUI::HandleCloseWin

/*===========================================================================*/

long DEMPaintGUI::HandleKeyDown(int Key, char Alt, char Control, char Shift)
{
double range = (MaxElev - MinElev);
bool update = false;

switch (Key)
	{
	case '+':
		update = true;
		ForeElev += range * 0.01;
		break;
	case '-':
		update = true;
		ForeElev -= range * 0.01;
		break;
	} // switch Key

if (update)
	{
	AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue = ForeElev;
	WidgetSNSync(IDC_FOREGROUND, WP_FISYNC_NONOTIFY);
	DrawForegroundIndicator();
	} // if

return 0;

} // DEMPaintGUI::HandleKeyDown

/*===========================================================================*/

long DEMPaintGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_BRUSH0:
		SetBrush(0);
		break;
	case IDC_BRUSH1:
		SetBrush(1);
		break;
	case IDC_BRUSH2:
		SetBrush(2);
		break;
	case IDC_BRUSH3:
		SetBrush(3);
		break;
	case IDC_BRUSH4:
		SetBrush(4);
		break;
	case IDC_BRUSH5:
		SetBrush(5);
		break;
	case IDC_BRUSH6:
		SetBrush(6);
		break;
	case IDC_BRUSH7:
		SetBrush(7);
		break;
	case IDC_BRUSH8:
		SetBrush(8);
		break;
	case IDC_BRUSH9:
		SetBrush(9);
		break;
	case IDC_BRUSH10:
		SetBrush(10);
		break;
	case IDC_BRUSH11:
		SetBrush(11);
		break;
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
	case IDC_UNDO:
		{
		memcpy(ActiveDEM->Map(), UndoMap, ElevCols * ElevRows * sizeof(float));
		ActiveDEM->FindElMaxMin();
		MaxElev = ActiveDEM->MaxEl();
		MinElev = ActiveDEM->MinEl();
		MakeTable = true;
		ComputeTerrainView();
		ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
		ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);
		UpdatePreview();
		break;
		} // 
	case IDCANCEL:
		{
		ReloadDEM();
		ComputeTerrainView();
		ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
		ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);
		UpdatePreview();
		break;
		} // IDCANCEL
	case IDC_RELOAD:
		{
		ReloadDEM();
		ComputeTerrainView();
		ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
		ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);
		UpdatePreview();
		break;
		} // IDC_RELOAD
	case IDC_SAVE:
		{
		SaveDEM();
		break;
		} // IDC_SAVEDEM
	case IDC_TOOLS:
	case IDC_BRUSHES:
	case IDC_ELEVATIONS:
	case IDC_OPTIONS:
		{
		switch (ButtonID)
			{
			case IDC_TOOLS:
				{
				ShowPanel(2, ActivePanel = 0);
				break;
				} // 0
			case IDC_OPTIONS:
				{
				ShowPanel(2, ActivePanel = 1);
				break;
				} // 1
			case IDC_BRUSHES:
				{
				ShowPanel(2, ActivePanel = 2);
				break;
				} // 1
			case IDC_ELEVATIONS:
				{
				ShowPanel(2, ActivePanel = 3);
				break;
				} // 1
			default:
				{
				ShowPanel(2, ActivePanel = 0);
				break;
				} // 0
			} // switch
		// update mutex buttons
		WidgetSetCheck(IDC_TOOLS, ButtonID == IDC_TOOLS);
		WidgetSetCheck(IDC_OPTIONS, ButtonID == IDC_OPTIONS);
		WidgetSetCheck(IDC_BRUSHES, ButtonID == IDC_BRUSHES);
		WidgetSetCheck(IDC_ELEVATIONS, ButtonID == IDC_ELEVATIONS);
		break;
		} // Select toolbar contents
	case IDC_PAINTTOOL_REGION:
	case IDC_PAINTTOOL_MOVEREGION:
	case IDC_PAINTTOOL_FREESELECT:
	case IDC_PAINTTOOL_SELECT:
	case IDC_PAINTTOOL_PAINT:
	case IDC_PAINTTOOL_ERASE:
	case IDC_PAINTTOOL_FILL:
	case IDC_PAINTTOOL_EYEDROPPER:
	case IDC_PAINTTOOL_GRADIENT:
	case IDC_PAINTTOOL_SMOOTH:
	case IDC_PAINTTOOL_SMEAR:
	case IDC_PAINTTOOL_ROUGHEN:
		SetPaintMode(ButtonID);
		HandleCBChange(Handle, NW, IDC_CB_OPT1);
		break;
	case IDC_SWAP:
		double tmp;
		tmp = ForeElev;
		ForeElev = BackElev;
		BackElev = tmp;
		AnimPar[DPG_ANIMPAR_BACKGROUND].CurValue = BackElev;
		WidgetSNSync(IDC_BACKGROUND, WP_FISYNC_NONOTIFY);
		AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue = ForeElev;
		WidgetSNSync(IDC_FOREGROUND, WP_FISYNC_NONOTIFY);
		DrawForegroundIndicator();
		break;
	default:
		break;
	} // ButtonID
return(0);

} // DEMPaintGUI::HandleButtonClick

/*===========================================================================*/


long DEMPaintGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CB_OPT1:
		Effect = (DPG_EFFECT)WidgetCBGetCurSel(IDC_CB_OPT1);
		WidgetCBClear(IDC_CB_OPT1);
		WidgetCBAddEnd(IDC_CB_OPT1, "Absolute");
		WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Raise");
		WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Lower");
		if ((PaintMode != DPG_MODE_SMOOTH) && (PaintMode != DPG_MODE_ROUGHEN) && (PaintMode != DPG_MODE_SMEAR))
			{
			WidgetCBAddEnd(IDC_CB_OPT1, "Rel: Raise");
			WidgetCBAddEnd(IDC_CB_OPT1, "Rel: Lower");
			}
		if ((Effect == DPG_EFFECT_REL_RAISE) || (Effect == DPG_EFFECT_REL_LOWER))
			{
			if ((PaintMode == DPG_MODE_SMOOTH) || (PaintMode == DPG_MODE_ROUGHEN) || (PaintMode == DPG_MODE_SMEAR))
				{
				Effect = DPG_EFFECT_ABS_RAISELOWER;
				WidgetCBClear(IDC_CB_OPT1);
				WidgetCBAddEnd(IDC_CB_OPT1, "Absolute");
				WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Raise");
				WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Lower");
				} // if
			else
				{
				memset(ModifierMap, 0, ElevCols * ElevRows * sizeof(float));	// init all vals to 0.0f - which in IEEE is 4 zeros
				memcpy(ModifierBaseMap, ActiveDEM->RawMap, ElevCols * ElevRows * sizeof(float));
				} // else
			} // if
		WidgetCBSetCurSel(IDC_CB_OPT1, Effect);
		break;
	case IDC_CB_OPT2:
		if (PaintMode == DPG_MODE_GRADIENT)
			GradMode = (DPG_GRADMODE)WidgetCBGetCurSel(IDC_CB_OPT2);
		break;
	default:
		break;
	} // switch

return (0);

} // DEMPaintGUI::HandleCBChange

/*===========================================================================*/

long DEMPaintGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
short ZoomChanged = 0;

if (ScrollCode)
	{
	switch (CtrlID)
		{
		case IDC_DPG_ZOOM:
			{
			double CenterX, CenterY, NewOffsetX, NewOffsetY, MaxOffset;

			CenterX = Elevs.OffsetX + .5 * Elevs.Visible;
			CenterY = Elevs.OffsetY + .5 * Elevs.Visible;

			Elevs.Visible = ScrollPos * 0.01;

			NewOffsetX = Elevs.Visible < 1.0 ? (CenterX - Elevs.Visible * .5): 0.0;
			NewOffsetY = Elevs.Visible < 1.0 ? (CenterY - Elevs.Visible * .5): 0.0;

			MaxOffset = (1.0 - Elevs.Visible);
			Elevs.OffsetX = NewOffsetX > MaxOffset ? MaxOffset: NewOffsetX < 0.0 ? 0.0: NewOffsetX;
			Elevs.OffsetY = NewOffsetY > MaxOffset ? MaxOffset: NewOffsetY < 0.0 ? 0.0: NewOffsetY;

			ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetX / (1.0 - Elevs.Visible)): 0;
			WidgetSetScrollPos(IDC_DPG_HPOS, ScrollPos);
			ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetY / (1.0 - Elevs.Visible)): 0;
			WidgetSetScrollPos(IDC_DPG_VPOS, ScrollPos);

			ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
			ZoomChanged = 1;
			break;
			} // IDC_DPG_ZOOM
		case IDC_DPG_HPOS:
			{
			double HPos;

			HPos = ScrollPos * 0.01;

			HPos *= (1.0 - Elevs.Visible);
			Elevs.OffsetX = HPos;
			ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
			ZoomChanged = 1;
			break;
			} // IDC_DPG_HPOS
		case IDC_DPG_VPOS:
			{
			double VPos;

			VPos = ScrollPos * 0.01;

			VPos *= (1.0 - Elevs.Visible);
			Elevs.OffsetY = VPos;
			ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
			ZoomChanged = 1;
			break;
			} // IDC_DPG_VPOS
		case IDC_DPG_BRUSHSIZE:
			{
			BrushScale = (float)ScrollPos;
			SetBrush(ActiveBrush);
			break;
			} // IDC_DPG_BRUSHSIZE
		default:
			break;
		} // switch
	if (ZoomChanged)
		{
		DrawMaskMap();
		ComputeThumbOverlay();
		} // if
	return(0);
	} // if
else
	{
	return(5); // default scroll amount
	} // else

} // DEMPaintGUI::HandleScroll

/*===========================================================================*/

long DEMPaintGUI::HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift)
{
double HalfVis = Elevs.Visible * 0.5, tmpx, tmpy;
long px, py;
int ScrollPos;
SHORT spacebar;
bool ZoomChanged = false;

LMBDown = 1;

spacebar = GetAsyncKeyState(VK_SPACE) & 0x8000;

if (! spacebar)
	{
	switch (CtrlID)
		{
		case IDC_DPG_AREA:
			{
			if (PaintMode == DPG_MODE_MASK)
				{
				DefiningMask = true;
				if (Shift)
					MaskOpts = DPG_MASKOPT_ADD;
				else if (Alt)
					MaskOpts = DPG_MASKOPT_SUB;
				else
					MaskOpts = DPG_MASKOPT_NONE;
				}
			if (DefiningMask)
				{
				// get cursor position relative to widget
				tmpx = (double)X / WidSize[0][0];
				tmpy = (double)Y / WidSize[0][1];
				if (tmpx < 0.0)
					tmpx = 0.0;
				if (tmpx > 1.0)
					tmpx = 1.0;
				if (tmpy < 0.0)
					tmpy = 0.0;
				if (tmpy > 1.0)
					tmpy = 1.0;
				// now translate widget x/y to visible area
				tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
				tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
				if (MaskMode == DPG_MASKMODE_BOX)
					{
					// store result as first box corner
					boxx0 = (float)tmpx;
					boxy0 = (float)tmpy;
					// copy into 2nd box corner for safety's sake
					boxx1 = (float)tmpx;
					boxy1 = (float)tmpy;
					if (MaskOpts == DPG_MASKOPT_NONE)
						memset(MaskMap, 255, ElevCols * ElevRows);
					}
				else if (MaskMode == DPG_MASKMODE_WAND)
					{
					// now we can figure out where in the DEM we are
					px = (long)(tmpx * (ElevCols - 1));
					py = (long)(tmpy * (ElevRows - 1));
					MagicWand(px, py);
					}
				else if (MaskMode == DPG_MASKMODE_FREEHAND)
					{
					FreehandVerts[0][0] = (float)tmpx;
					FreehandVerts[0][1] = (float)tmpy;
					FreehandCount = 1;
					}
				else if (MaskMode == DPG_MASKMODE_MOVE)
					{
					MoveRefX = float(tmpx);
					MoveRefY = float(tmpy);
					memset(Elevs.OverRast->ByteMap[2], 0, ElevCols * ElevRows);
					MoveSelect(0.0f, 0.0f, true);
					}
				} // if DefiningMask
			else
				{
				if (PaintMode != DPG_MODE_EYEDROP)
					memcpy(UndoMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));
				if (PaintMode == DPG_MODE_GRADIENT)
					{
					RubberBand = true;
					// get cursor position relative to widget
					tmpx = (double)X / WidSize[0][0];
					tmpy = (double)Y / WidSize[0][1];
					if (tmpx < 0.0)
						tmpx = 0.0;
					if (tmpx > 1.0)
						tmpx = 1.0;
					if (tmpy < 0.0)
						tmpy = 0.0;
					if (tmpy > 1.0)
						tmpy = 1.0;
					// now translate widget x/y to visible area
					tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
					tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
					// now save as the rubber band start point
					rbx0 = (float)tmpx;
					rby0 = (float)tmpy;
					// copy for safety's sake
					rbx1 = (float)tmpx;
					rby1 = (float)tmpy;
					}
				if (PaintMode == DPG_MODE_FILL)
					SeedPaint(X, Y);
				else if (PaintMode == DPG_MODE_SMOOTH)
					Smooth(X, Y);
				else if (PaintMode == DPG_MODE_ROUGHEN)
					Roughen(X, Y);
				else if (PaintMode == DPG_MODE_SMEAR)
					Smear(X, Y, true);
				else
					Paint(X, Y);
				} // else Defining Mask
			DrawAreaOverlays();
			if (PaintRefresh)
				{
				ComputeTerrainView();
				ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
				} // if
			break;
			} // IDC_DPG_AREA
		case IDC_DPG_THUMB:
			{
			tmpx = (double)X / WidSize[1][0];
			tmpy = (double)Y / WidSize[1][1];
			if ((tmpx - HalfVis) < 0.0)
				tmpx = HalfVis;
			if ((tmpy - HalfVis) < 0.0)
				tmpy = HalfVis;
			if ((tmpx - HalfVis + Elevs.Visible) <= 1.0)
				{
				Elevs.OffsetX = tmpx - HalfVis;
				ZoomChanged = true;
				} // if
			if ((tmpy - HalfVis + Elevs.Visible) <= 1.0)
				{
				Elevs.OffsetY = tmpy - HalfVis;
				ZoomChanged = true;
				} // if
			ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);
			if (ZoomChanged)
				{
				ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
				ComputeThumbOverlay();
				ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetX / (1.0 - Elevs.Visible)): 0;
				WidgetSetScrollPos(IDC_DPG_HPOS, ScrollPos);
				ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetY / (1.0 - Elevs.Visible)): 0;
				WidgetSetScrollPos(IDC_DPG_VPOS, ScrollPos);
				} // if
			//Sampling = InitCameraMotion(1, (double)X / WidSize[1][0], (double)Y / WidSize[1][1]);
			break;
			} // IDC_DPG_THUMB
		case IDC_DPG_ELEVGRAD:
			{
			tmpy = (float)Y / WidSize[2][1];
			if (tmpy < 0.0f)
				tmpy = 0.0f;
			if (tmpy > 1.0f)
				tmpy = 1.0f;
			ForeElev = (MaxElev - MinElev) * (1.0f - tmpy) + MinElev;
			AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue = ForeElev;
			WidgetSNSync(IDC_FOREGROUND, WP_FISYNC_NONOTIFY);
			DrawForegroundIndicator();
			break;
			} // IDC_DPG_ELEVGRAD
		} // switch CtrlID
	} // if

return (0);

} // DEMPaintGUI::HandleLeftButtonDown

/*===========================================================================*/

long DEMPaintGUI::HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift)
{
double tmpx, tmpy;
long i, x, y, Zip;
//long bottom, left, right, top;

LMBDown = 0;

switch (CtrlID)
	{
	case IDC_DPG_AREA:
		{
		if (DefiningMask)
			{
			DefiningMask = false;
			if (MaskMode == DPG_MASKMODE_BOX)
				{
				// get cursor position relative to widget
				tmpx = (double)X / WidSize[0][0];
				tmpy = (double)Y / WidSize[0][1];
				if (tmpx < 0.0)
					tmpx = 0.0;
				if (tmpx > 1.0)
					tmpx = 1.0;
				if (tmpy < 0.0)
					tmpy = 0.0;
				if (tmpy > 1.0)
					tmpy = 1.0;
				// now translate widget x/y to visible area
				tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
				tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
				// now save as the 2nd box corner
				boxx1 = (float)tmpx;
				boxy1 = (float)tmpy;
				BoxMask();
				} // DPG_MASKMODE_BOX
			else if (MaskMode == DPG_MASKMODE_FREEHAND)
				{
				// this trick will keep us from having to write new floodfill code
				ElevCols += 2;
				ElevRows += 2;
				// set the floodmap to a filled state
				memset(FloodMap, 255, ElevCols * ElevRows);
				// draw the polygon into the floodmap
				DrawFreehandVerts2();
				// flood fill starting in the 1 pixel border region
				SeedFill(0, 0);
				ElevCols -= 2;
				ElevRows -= 2;
				if (MaskOpts != DPG_MASKOPT_NONE)
					memcpy(TmpMaskMap, MaskMap, ElevCols * ElevRows);
				// copy the computed mask over
				for (i = y = 0; y < ElevRows; y++)
					{
					Zip = (y + 1) * (ElevCols + 2) + 1;
					for (x = 0; x < ElevCols; x++)
						{
						MaskMap[i++] = FloodMap[Zip++];
						} // for x
					} // for y
				if (MaskOpts == DPG_MASKOPT_ADD)
					AddMasks();
				else if (MaskOpts == DPG_MASKOPT_SUB)
					SubMasks();
				} // DPG_MASKMODE_FREEHAND
			else if (MaskMode == DPG_MASKMODE_MOVE)
				{
				// get cursor position relative to widget
				tmpx = (double)X / WidSize[0][0];
				tmpy = (double)Y / WidSize[0][1];
				if (tmpx < 0.0)
					tmpx = 0.0;
				if (tmpx > 1.0)
					tmpx = 1.0;
				if (tmpy < 0.0)
					tmpy = 0.0;
				if (tmpy > 1.0)
					tmpy = 1.0;
				// now translate widget x/y to visible area
				tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
				tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
				MoveSelect(MoveRefX - (float)tmpx, MoveRefY - (float)tmpy, false);
				}
			} // if DefiningMask

		RubberBand = false;
		DrawAreaOverlays();

		if (PaintMode == DPG_MODE_GRADIENT)
			DoGradFill();
		if (PaintRefresh)
			{
			ComputeTerrainView();
			ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
			ConfigureRD(NativeWin, IDC_DPG_THUMB, &Thumb);
			UpdatePreview();
			PaintRefresh = false;
			} // if
		if ((Effect == DPG_EFFECT_REL_RAISE) || (Effect == DPG_EFFECT_REL_LOWER))
			{
			memset(ModifierMap, 0, ElevCols * ElevRows * sizeof(float));
			memcpy(ModifierBaseMap, ActiveDEM->RawMap, ElevCols * ElevRows * sizeof(float));
			} // if
		break;
		} // IDC_DPG_AREA
	default:
		break;
	} // switch

return (0);

} // DEMPaintGUI::HandleLeftButtonUp

/*===========================================================================*/

long DEMPaintGUI::HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
double HalfVis = Elevs.Visible * 0.5, tmpx, tmpy;
long ScrollPos;
SHORT lmb, spacebar;
static bool pan = false;
bool ZoomChanged = false;

lmb = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
spacebar = GetAsyncKeyState(VK_SPACE) & 0x8000;

if (lmb && spacebar)
	{
	long deltaX, deltaY;
	static long lastX = 0, lastY = 0;	// initialized since compiler warns about unitialized vars otherwise

	if (! pan)
		{
		pan = true;
		lastX = X;
		lastY = Y;
		} // if
	else
		{
		double panFactor = 0.001;	//(1.0 - Elevs.Visible) * 0.05;
		double maxOffset = (1.0 - Elevs.Visible);

		deltaX = X - lastX;
		deltaY = Y - lastY;
		lastX = X;
		lastY = Y;
		tmpx = Elevs.OffsetX + panFactor * deltaX;
		tmpy = Elevs.OffsetY + panFactor * deltaY;
		if (tmpx < 0.0)
			tmpx = 0.0;
		if (tmpx > maxOffset)
			tmpx = maxOffset;
		if (tmpy < 0.0)
			tmpy = 0.0;
		if (tmpy > maxOffset)
			tmpy = maxOffset;
		Elevs.OffsetX = tmpx;
		Elevs.OffsetY = tmpy;
		ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
		ComputeThumbOverlay();
		ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetX / (1.0 - Elevs.Visible)): 0;
		WidgetSetScrollPos(IDC_DPG_HPOS, ScrollPos);
		ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetY / (1.0 - Elevs.Visible)): 0;
		WidgetSetScrollPos(IDC_DPG_VPOS, ScrollPos);
		} // else
	} // if
else
	{
	pan = false;
	switch (CtrlID)
		{
		case IDC_DPG_AREA:
			{
			if (LMBDown)
				{
				if (DefiningMask)
					{
					// get cursor position relative to widget
					tmpx = (double)X / WidSize[0][0];
					tmpy = (double)Y / WidSize[0][1];
					if (tmpx < 0.0)
						tmpx = 0.0;
					if (tmpx > 1.0)
						tmpx = 1.0;
					if (tmpy < 0.0)
						tmpy = 0.0;
					if (tmpy > 1.0)
						tmpy = 1.0;
					// now translate widget x/y to visible area
					tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
					tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
					if (MaskMode == DPG_MASKMODE_BOX)
						{
						// now save as the 2nd box corner
						boxx1 = (float)tmpx;
						boxy1 = (float)tmpy;
						DrawAreaOverlays();
						}
					else if (MaskMode == DPG_MASKMODE_FREEHAND)
						{
						if (FreehandCount == 1023)
							FreehandCount = 0;	// overflowed - reset
						FreehandVerts[FreehandCount][0] = (float)tmpx;
						FreehandVerts[FreehandCount][1] = (float)tmpy;
						FreehandCount++;
						DrawAreaOverlays();
						}
					else if (MaskMode == DPG_MASKMODE_MOVE)
						{
						MoveSelect(MoveRefX - (float)tmpx, MoveRefY - (float)tmpy, true);
						}
					} // if DefiningMask
				else
					{
					if (PaintMode == DPG_MODE_SMOOTH)
						Smooth(X, Y);
					else if (PaintMode == DPG_MODE_ROUGHEN)
						Roughen(X, Y);
					else if (PaintMode == DPG_MODE_SMEAR)
						Smear(X, Y, false);
					else if (PaintMode != DPG_MODE_FILL)
						Paint(X, Y);
					if (PaintMode == DPG_MODE_GRADIENT)
						{
						// get cursor position relative to widget
						tmpx = (double)X / WidSize[0][0];
						tmpy = (double)Y / WidSize[0][1];
						if (tmpx < 0.0)
							tmpx = 0.0;
						if (tmpx > 1.0)
							tmpx = 1.0;
						if (tmpy < 0.0)
							tmpy = 0.0;
						if (tmpy > 1.0)
							tmpy = 1.0;
						// now translate widget x/y to visible area
						tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
						tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
						// now save as the rubber band end point
						rbx1 = (float)tmpx;
						rby1 = (float)tmpy;
						}
					DrawAreaOverlays();
					if (PaintRefresh)
						{
						ComputeTerrainView();
						ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
						} // if
					} // else Defining Mask
				} // if LMBDown
			else
				{
				// draw brush outline & restore/save brush region as necessary
				if (SavedRegion)
					RestoreBrushRegion();
				SaveBrushRegion(X, Y);
				ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
				} // else LMBDown
			break;
			} // IDC_DPG_AREA
		case IDC_DPG_THUMB:
			{
			if (LMBDown)
				{
				tmpx = (double)X / WidSize[1][0];
				tmpy = (double)Y / WidSize[1][1];
				if ((tmpx - HalfVis) < 0.0)
					tmpx = HalfVis;
				if ((tmpy - HalfVis) < 0.0)
					tmpy = HalfVis;
				if ((tmpx - HalfVis + Elevs.Visible) <= 1.0)
					{
					Elevs.OffsetX = tmpx - HalfVis;
					ZoomChanged = true;
					} // if
				if ((tmpy - HalfVis + Elevs.Visible) <= 1.0)
					{
					Elevs.OffsetY = tmpy - HalfVis;
					ZoomChanged = true;
					} // if
				if (ZoomChanged)
					{
					ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
					ComputeThumbOverlay();
					ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetX / (1.0 - Elevs.Visible)): 0;
					WidgetSetScrollPos(IDC_DPG_HPOS, ScrollPos);
					ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetY / (1.0 - Elevs.Visible)): 0;
					WidgetSetScrollPos(IDC_DPG_VPOS, ScrollPos);
					} // if ZoomChanged
				} // if LMBDown
			//Sampling = InitCameraMotion(1, (double)X / WidSize[1][0], (double)Y / WidSize[1][1]);
			break;
			} // IDC_DPG_THUMB
		default:
			break;
		} // switch CtrlID
	} // else

return (0);

} // DEMPaintGUI::HandleMouseMove

/*===========================================================================*/

long DEMPaintGUI::HandleMouseWheelVert(long x, long y, float amount, char alt, char control, char shift)
{
double CenterX, CenterY;
int ScrollPos;
bool zoomChanged = false;

CenterX = Elevs.OffsetX + .5 * Elevs.Visible;
CenterY = Elevs.OffsetY + .5 * Elevs.Visible;

if (amount < 0)
	{
	// zoom in
	Elevs.Visible *= 9.0 / 10.0;
	if (Elevs.Visible < 0.1)
		Elevs.Visible = 0.1;
	zoomChanged = true;
	} // if
else if (amount > 0)
	{
	// zoom out
	Elevs.Visible *= 10.0 / 9.0;
	if (Elevs.Visible > 1.0)
		Elevs.Visible = 1.0;
	zoomChanged = true;
	} // else if

if (zoomChanged)
	{
	double NewOffsetX, NewOffsetY, MaxOffset;

	NewOffsetX = Elevs.Visible < 1.0 ? (CenterX - Elevs.Visible * .5): 0.0;
	NewOffsetY = Elevs.Visible < 1.0 ? (CenterY - Elevs.Visible * .5): 0.0;

	MaxOffset = (1.0 - Elevs.Visible);
	Elevs.OffsetX = NewOffsetX > MaxOffset ? MaxOffset: NewOffsetX < 0.0 ? 0.0: NewOffsetX;
	Elevs.OffsetY = NewOffsetY > MaxOffset ? MaxOffset: NewOffsetY < 0.0 ? 0.0: NewOffsetY;

	ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetX / (1.0 - Elevs.Visible)): 0;
	WidgetSetScrollPos(IDC_DPG_HPOS, ScrollPos);
	ScrollPos = Elevs.Visible < 1.0 ? (long)(100.0 * Elevs.OffsetY / (1.0 - Elevs.Visible)): 0;
	WidgetSetScrollPos(IDC_DPG_VPOS, ScrollPos);
	WidgetSetScrollPos(IDC_DPG_ZOOM, (long)(Elevs.Visible * 100));

	ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
	DrawMaskMap();
	ComputeThumbOverlay();
	} // if

return(1);

} // DEMPaintGUI::HandleMouseWheelVert

/*===========================================================================*/

long DEMPaintGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SC_DPGPREVIEW:
	case IDC_SC_DPGSOLO:
		UpdatePreview();
		break;
	default:
		break;
	} // switch

return(0);

} // DEMPaintGUI::HandleSCChange

/*===========================================================================*/

long DEMPaintGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FOREGROUND:
		ForeElev = AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue;
		DrawForegroundIndicator();
		break;
	case IDC_BACKGROUND:
		BackElev = AnimPar[DPG_ANIMPAR_BACKGROUND].CurValue;
		break;
	default:
		break;
	} // switch

return(0);

} // DEMPaintGUI::HandleFIChange

/*===========================================================================*/

void DEMPaintGUI::HandleNotifyEvent(void)
{
DiagnosticData *Diagnostics;
NotifyTag *Changes, Interested[7];
int Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	} // if DEM count changed

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	} // if units changed

if (Active)
	{
	Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		DisableWidgets();
		Done = 1;
		} // if

	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
	Interested[1] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		if (Diagnostics = (DiagnosticData *)Activity->ChangeNotify->NotifyData)
			{
			if (Diagnostics->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Diagnostics->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				SelectPoints(Diagnostics->Value[WCS_DIAGNOSTIC_LATITUDE], Diagnostics->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			} // if
		Done = 1;
		} // if
	} // if

if (! Done)
	ConfigureWidgets();

} // DEMPaintGUI::HandleNotifyEvent()

/*===========================================================================*/


long DEMPaintGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT ControlRect, WindRect;
POINT TransUL, PaneTransUL;
NativeControl AreaPane, LeftScroll, RightScroll, BotScroll, RDW;
int NewW, NewH, RightScrollW, BotScrollH;

if (AreaPane = SubPanels[0][0])
	{
	LeftScroll = GetWidgetFromID(IDC_DPG_ZOOM);
	RightScroll = GetWidgetFromID(IDC_DPG_VPOS);
	BotScroll = GetWidgetFromID(IDC_DPG_HPOS);
	RDW = GetWidgetFromID(IDC_DPG_AREA);

	GetClientRect(NativeWin, &WindRect);

	// resize containing Area Pane
	GetWindowRect(AreaPane, &ControlRect);
	TransUL.x = ControlRect.left;
	TransUL.y = ControlRect.top;
	ScreenToClient(NativeWin, &TransUL);
	NewW = WindRect.right - TransUL.x;
	NewH = WindRect.bottom - TransUL.y;
	SetWindowPos(AreaPane, NULL, 0, 0, NewW, NewH, SWP_NOMOVE | SWP_NOZORDER);

	// get coords of UL of RasterDrawWidget, in AreaPane's Coordinate space
	GetWindowRect(RDW, &ControlRect);
	PaneTransUL.x = ControlRect.left;
	PaneTransUL.y = ControlRect.top;
	ScreenToClient(AreaPane, &PaneTransUL);

	// Get calculation dimensions
	GetWindowRect(RightScroll, &ControlRect); // RightScroll
	RightScrollW = ControlRect.right - ControlRect.left;

	GetWindowRect(BotScroll, &ControlRect); // BotScroll
	BotScrollH = ControlRect.bottom - ControlRect.top;

	// calculate new size of RasterDrawWidget
	NewW -= (RightScrollW + PaneTransUL.x);
	NewH -= (BotScrollH + PaneTransUL.y);

	if (NewW < 10) NewW = 10;
	if (NewH < 10) NewH = 10;

	WidSize[0][0] = NewW - 4;
	WidSize[0][1] = NewH - 4;
	//ComputeTerrainView();
	//ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);

	// move/size RightScroll
	SetWindowPos(RightScroll, NULL, PaneTransUL.x + NewW, PaneTransUL.y, RightScrollW, NewH, SWP_NOZORDER);

	// move/size BotScroll
	SetWindowPos(BotScroll, NULL, PaneTransUL.x, PaneTransUL.y + NewH, NewW, BotScrollH, SWP_NOZORDER);

	// resize RasterDrawWidget
	SetWindowPos(RDW, NULL, 0, 0, NewW, NewH, SWP_NOMOVE | SWP_NOZORDER);
	// force repaint
	InvalidateRect(RDW, NULL, NULL);

	} // if

return(0);
} // DEMPaintGUI::HandleReSized

/*===========================================================================*/

void DEMPaintGUI::ConfigureToolsBrushes(void)
{
int IconID;

// Tools
IconID = IDI_PAINTFREE; // default image
switch (PaintMode)
	{
	case DPG_MODE_MASK:
		{
		switch (MaskMode)
			{
			case DPG_MASKMODE_NONE:	default: IconID = IDI_REGION; break;
			case DPG_MASKMODE_BOX: IconID = IDI_REGION; break;
			case DPG_MASKMODE_MOVE: IconID = IDI_MOVEREGION; break;
			case DPG_MASKMODE_FREEHAND: IconID = IDI_FREESELECT; break;
			case DPG_MASKMODE_WAND: IconID = IDI_SELECT; break;
			} // MaskMode
		break;
		} // MASK
	case DPG_MODE_PAINT: IconID = IDI_PAINTFREE; break;
	case DPG_MODE_EYEDROP: IconID = IDI_EYEDROPPER; break;
	case DPG_MODE_FILL: IconID = IDI_FILL; break;
	case DPG_MODE_GRADIENT: IconID = IDI_GRADIENT; break;
	case DPG_MODE_ERASE: IconID = IDI_ERASE; break;
	case DPG_MODE_SMOOTH: IconID = IDI_PAINTSMOOTH; break;
	case DPG_MODE_ROUGHEN: IconID = IDI_ROUGHEN; break;
	case DPG_MODE_SMEAR: IconID = IDI_SMEAR; break;
	} // switch
ConfigureTB(NativeWin, IDC_TOOLS, IconID, NULL);

// Brushes
IconID = IDI_BRUSH1; // default image
if (ActiveBrush == 0) IconID = IDI_BRUSH1;
else if (ActiveBrush == 1) IconID = IDI_BRUSH2;
else if (ActiveBrush == 2) IconID = IDI_BRUSH3;
else if (ActiveBrush == 3) IconID = IDI_BRUSH4;
else if (ActiveBrush == 4) IconID = IDI_BRUSH5;
else if (ActiveBrush == 5) IconID = IDI_BRUSH6;
else if (ActiveBrush == 6) IconID = IDI_BRUSH7;
else if (ActiveBrush == 7) IconID = IDI_BRUSH8;
else if (ActiveBrush == 8) IconID = IDI_BRUSH9;
else if (ActiveBrush == 9) IconID = IDI_BRUSH10;
else if (ActiveBrush == 10) IconID = IDI_BRUSH11;
//else if (Brush == BrushDefs[11]) IconID = IDI_NOTEXTURE;
ConfigureTB(NativeWin, IDC_BRUSHES, IconID, NULL);

} // DEMPaintGUI::ConfigureToolsBrushes()

/*===========================================================================*/

void DEMPaintGUI::ConfigureWidgets(void)
{
char TextStr[256];

if (Active)
	sprintf(TextStr, "DEM Painter - %s", Active->GetBestName());
else
	strcpy(TextStr, "DEM Painter");
SetTitle(TextStr);

ConfigureSC(NativeWin, IDC_SC_DPGPREVIEW, &Previewing, SCFlag_Long, NULL, NULL);
ConfigureSC(NativeWin, IDC_SC_DPGSOLO, &Solo, SCFlag_Long, NULL, NULL);

ConfigureTB(NativeWin, IDC_UNDO, IDI_REVERT, NULL);
ConfigureTB(NativeWin, IDC_SAVEDEM, IDI_FILESAVE, NULL);

ConfigureTB(NativeWin, IDC_OPTIONS, IDI_PREFS, NULL);
ConfigureTB(NativeWin, IDC_ELEVATIONS, IDI_ELEVATIONS, NULL);

ConfigureToolsBrushes();

ConfigureTB(NativeWin, IDC_PAINTTOOL_PAINT, IDI_PAINTFREE, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_SMOOTH, IDI_PAINTSMOOTH, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_FILL, IDI_FILL, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_GRADIENT, IDI_GRADIENT, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_ERASE, IDI_ERASE, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_SELECT, IDI_SELECT, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_SMEAR, IDI_SMEAR, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_MOVEREGION, IDI_MOVEREGION, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_REGION, IDI_REGION, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_EYEDROPPER, IDI_EYEDROPPER, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_FREESELECT, IDI_FREESELECT, NULL);
ConfigureTB(NativeWin, IDC_PAINTTOOL_ROUGHEN, IDI_ROUGHEN, NULL);

ConfigureTB(NativeWin, IDC_BRUSH0, IDI_BRUSH1, NULL);
ConfigureTB(NativeWin, IDC_BRUSH1, IDI_BRUSH2, NULL);
ConfigureTB(NativeWin, IDC_BRUSH2, IDI_BRUSH3, NULL);
ConfigureTB(NativeWin, IDC_BRUSH3, IDI_BRUSH4, NULL);
ConfigureTB(NativeWin, IDC_BRUSH4, IDI_BRUSH5, NULL);
ConfigureTB(NativeWin, IDC_BRUSH5, IDI_BRUSH6, NULL);
ConfigureTB(NativeWin, IDC_BRUSH6, IDI_BRUSH7, NULL);
ConfigureTB(NativeWin, IDC_BRUSH7, IDI_BRUSH8, NULL);
ConfigureTB(NativeWin, IDC_BRUSH8, IDI_BRUSH9, NULL);
ConfigureTB(NativeWin, IDC_BRUSH9, IDI_BRUSH10, NULL);
ConfigureTB(NativeWin, IDC_BRUSH10, IDI_BRUSH11, NULL);
ConfigureTB(NativeWin, IDC_BRUSH11, IDI_NOTEXTURE, NULL);

WidgetSNConfig(IDC_FOREGROUND, &AnimPar[DPG_ANIMPAR_FOREGROUND]);
WidgetSNConfig(IDC_BACKGROUND, &AnimPar[DPG_ANIMPAR_BACKGROUND]);

ConfigureFI(NativeWin, IDC_FI_OPACITY, &Opacity, 1.0, 0.0, 100.0, FIOFlag_Float, NULL, NULL);
ConfigureFI(NativeWin, IDC_FI_TOLERANCE, &Tolerance, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);

WidgetSetText(IDC_STATIC_OPT1, "Effect");
WidgetCBClear(IDC_CB_OPT1);
WidgetCBAddEnd(IDC_CB_OPT1, "Absolute");
WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Raise");
WidgetCBAddEnd(IDC_CB_OPT1, "Abs: Lower");
if ((PaintMode != DPG_MODE_SMOOTH) && (PaintMode != DPG_MODE_ROUGHEN) && (PaintMode != DPG_MODE_SMEAR))
	{
	WidgetCBAddEnd(IDC_CB_OPT1, "Rel: Raise");
	WidgetCBAddEnd(IDC_CB_OPT1, "Rel: Lower");
	}
WidgetCBSetCurSel(IDC_CB_OPT1, Effect);
WidgetSetText(IDC_STATIC_OPT2, " ");

DisableWidgets();
HideWidgets();

ComputeThumbOverlay();

ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
ConfigureRD(NativeWin, IDC_DPG_ELEVGRAD, &Gradient);

} // DEMPaintGUI::ConfigureWidgets()

/*===========================================================================*/

void DEMPaintGUI::NewDEMFile(void)
{

if (Elevs.MainRast = new Raster)
	{
	if (Elevs.MainRast->ByteMap[0] = (unsigned char *)AppMem_Alloc(ElevCols * ElevRows, 0L))
		{
		Elevs.MainRast->Cols = ElevCols;
		Elevs.MainRast->Rows = ElevRows;
		Elevs.MainRast->ByteBands = 1;
		Elevs.MainRast->ByteBandSize = ElevCols * ElevRows;
		ComputeTerrainView();
		Elevs.MainRast->ByteMap[1] = Elevs.MainRast->ByteMap[0];
		Elevs.MainRast->ByteMap[2] = Elevs.MainRast->ByteMap[0];
		ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
		Thumb.MainRast = Elevs.MainRast;
		} // if ByteMap[0]
	} // if Elevs.MainRast

#ifdef WCS_BUILD_VNS
MakeNullMap();
#endif // WCS_BUILD_VNS

} // DEMPaintGUI::NewDEMFile

/*===========================================================================*/

void DEMPaintGUI::MakeLUT(void)
{
double delta, diagdist, dist1, dist2, x = 0.0;
float erange = MaxElev - MinElev;
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
#endif // WCS_BUILD_VNS

// compute the distance between two cells
#ifdef WCS_BUILD_VNS
if (ActiveDEM->MoJoe && (MyAttr = (JoeCoordSys *)ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) &&
	(MyAttr->Coord->Method.GCTPMethod != 0))	// Projected
	{
	dist1 = ActiveDEM->LatStep();
	dist2 = -ActiveDEM->LonStep();
	}
else
	{
	// Geographic
	dist1 = ActiveDEM->LatStep() * LatScale(GlobalApp->AppEffects->GetPlanetRadius());
	dist2 = ActiveDEM->LonStep() * LonScale(GlobalApp->AppEffects->GetPlanetRadius(), (ActiveDEM->Northest() + ActiveDEM->Southest()) * 0.5);
	}
#else // WCS_BUILD_VNS
dist1 = ActiveDEM->LatStep() * LatScale(GlobalApp->AppEffects->GetPlanetRadius());
dist2 = ActiveDEM->LonStep() * LonScale(GlobalApp->AppEffects->GetPlanetRadius(), (ActiveDEM->Northest() + ActiveDEM->Southest()) / 2.0);
#endif // WCS_BUILD_VNS
diagdist = sqrt(dist1 * dist1 + dist2 * dist2);

delta = erange / 256.0;
for (long i = 0; i < 256; x += delta, i++)
	{
	lut[i] = (float)(atan2(x, diagdist) * HalfPi * 127.0);
	} // for

MakeTable = false;

} // DEMPaintGUI::MakeLUT

/*===========================================================================*/

void DEMPaintGUI::MakeNullMap(void)
{
#ifdef WCS_BUILD_VNS
float *sptr;
long x, y;

if (ActiveDEM->NullReject)
	{
	for (y = 0; y < ElevRows; y++)
		{
		sptr = ActiveDEM->Map() + ElevRows - y - 1;
		for (x = 0; x < ElevCols; x++, sptr += ElevRows)
			{
			if (*sptr == ActiveDEM->NullValue())
				NullMap[ElevCols * y + x] = 1;
			} // for
		} // for
	} // if

#endif // WCS_BUILD_VNS
} // DEMPaintGUI::MakeNullMap

/*===========================================================================*/

void DEMPaintGUI::ComputeTerrainViewDamaged(void)
{
float erange, val;
//long i;
long x, y;
float *refptr, *sptr;
unsigned char *dptr;
//char msg[80];
#ifdef WCS_BUILD_VNS
float NullVal = ActiveDEM->NullValue();
short HasNulls = ActiveDEM->NullReject;
#endif //WCS_BUILD_VNS

if (MakeTable)
	MakeLUT();
erange = MaxElev - MinElev;
if (erange != 0.0f)
	{
	float inverange255 = 1.0f / erange * 255.0f;	// pre-calc for speed
	// draw shaded view
	for (y = DamageYMin; y <= DamageYMax; y++)
		{
		dptr = Elevs.MainRast->ByteMap[0] + ElevCols * y + DamageXMin;
		sptr = ActiveDEM->Map() + ElevRows - y - 1 + ElevRows * DamageXMin;
		for (x = DamageXMin; x <= DamageXMax; x++, sptr += ElevRows)
			{
			val = *sptr;
#ifdef WCS_BUILD_VNS
			/*** changed 04/25/03 in quest for speed - NULL mask on overlay will overwrite any garbage that we display
			//if ((HasNulls) && (val == NullVal))
			//	*dptr++ = 0;
			//else
			***/
			refptr = sptr - ElevRows - 1;
				{
				if ((x != 0) && (y != 0))
					{
					//double slope;
					float slope;
					float delta, shade;
					//refptr = sptr - ElevRows - 1;
					shade = (val - MinElev) * inverange255;		// shaded height
					/***
					slope = atan2(*refptr - val, diagdist);		// +/- 1 radian
					slope = slope * HalfPi * 127.0 + 127.0f;	// scale to +/- 127.0
					***/
					delta = *refptr - val;
					slope = (float)(fabs(delta) * inverange255);
					if (slope > 255.0f)
						slope = 255.0f;
					slope = lut[(unsigned char)slope];
					if (delta < 0)
						slope = -slope;
					slope += 127.0f;
					*dptr++ = (unsigned char)(shade * 0.75f + slope * 0.25f);
					refptr++;
					} // if
				else
					*dptr++ = (unsigned char)((val - MinElev) * inverange255);
				} // else
#else // WCS_BUILD_VNS
			// shade it if we have a cell to the NorthWest
			if ((x != 0) && (y != 0))
				{
				double slope;
				float delta, shade;
				refptr = sptr - ElevRows - 1;
				shade = (val - MinElev) * inverange255;		// shaded height
				/***
				slope = atan2(*refptr - val, diagdist);		// +/- 1 radian
				slope = slope * HalfPi * 127.0 + 127.0f;	// scale to +/- 127.0
				***/
				delta = *refptr - val;
				slope = fabs(delta) * inverange255;
				if (slope > 255.0)
					slope = 255.0;
				slope = lut[(unsigned char)slope];
				if (delta < 0)
					slope = -slope;
				slope += 127.0f;
				*dptr++ = (unsigned char)(shade * 0.75f + slope * 0.25f);
				} // if
			else
				*dptr++ = (unsigned char)((val - MinElev) * inverange255);
#endif // WCS_BUILD_VNS
			} // for x
		} // for y
	} // if erange
else
	{
	// flood fill to white if no range in data
	dptr = Elevs.MainRast->ByteMap[0];
	for (y = 0; y < ElevRows; y++)
		{
		for (x = 0; x < ElevCols; x++)
			{
			*dptr++ = 255;
			} // for x
		} // for y
	} // else erange

//sprintf(&msg[0], "Edit Range: %fm - %fm", MinElev, MaxElev);
//GlobalApp->MCP->SetCurrentStatusText((char *)msg);

// reset the damage region
DamageXMax = DamageYMax = 0;
DamageXMin = DamageYMin = LONG_MAX;

} // DEMPaintGUI::ComputeTerrainViewDamaged

/*===========================================================================*/

void DEMPaintGUI::ComputeTerrainView(void)
{
float erange, val;
//long i;
long x, y;
float *refptr, *sptr;
unsigned char *dptr;
//char msg[80];
#ifdef WCS_BUILD_VNS
float NullVal = ActiveDEM->NullValue();
short HasNulls = ActiveDEM->NullReject;
#endif //WCS_BUILD_VNS

if (MakeTable)
	MakeLUT();
sptr = ActiveDEM->Map();
erange = MaxElev - MinElev;
if (erange != 0.0f)
	{
	float inverange255 = 1.0f / erange * 255.0f;	// pre-calc for speed
	// draw shaded view
	dptr = Elevs.MainRast->ByteMap[0];
	for (y = 0; y < ElevRows; y++)
		{
		sptr = ActiveDEM->Map() + ElevRows - y - 1;
		for (x = 0; x < ElevCols; x++, sptr += ElevRows)
			{
			val = *sptr;
#ifdef WCS_BUILD_VNS
			/*** changed 04/25/03 in quest for speed - NULL mask on overlay will overwrite any garbage that we display
			//if ((HasNulls) && (val == NullVal))
			//	*dptr++ = 0;
			//else
			***/
			refptr = sptr - ElevRows - 1;
				{
				if ((x != 0) && (y != 0))
					{
					//double slope;
					float slope;
					float delta, shade;
					//refptr = sptr - ElevRows - 1;
					shade = (val - MinElev) * inverange255;		// shaded height
					/***
					slope = atan2(*refptr - val, diagdist);		// +/- 1 radian
					slope = slope * HalfPi * 127.0 + 127.0f;	// scale to +/- 127.0
					***/
					delta = *refptr - val;
					slope = (float)(fabs(delta) * inverange255);
					if (slope > 255.0f)
						slope = 255.0f;
					slope = lut[(unsigned char)slope];
					if (delta < 0)
						slope = -slope;
					slope += 127.0f;
					*dptr++ = (unsigned char)(shade * 0.75f + slope * 0.25f);
					refptr++;
					} // if
				else
					*dptr++ = (unsigned char)((val - MinElev) * inverange255);
				} // else
#else // WCS_BUILD_VNS
			// shade it if we have a cell to the NorthWest
			if ((x != 0) && (y != 0))
				{
				double slope;
				float delta, shade;
				refptr = sptr - ElevRows - 1;
				shade = (val - MinElev) * inverange255;		// shaded height
				/***
				slope = atan2(*refptr - val, diagdist);		// +/- 1 radian
				slope = slope * HalfPi * 127.0 + 127.0f;	// scale to +/- 127.0
				***/
				delta = *refptr - val;
				slope = fabs(delta) * inverange255;
				if (slope > 255.0)
					slope = 255.0;
				slope = lut[(unsigned char)slope];
				if (delta < 0)
					slope = -slope;
				slope += 127.0f;
				*dptr++ = (unsigned char)(shade * 0.75f + slope * 0.25f);
				} // if
			else
				*dptr++ = (unsigned char)((val - MinElev) * inverange255);
#endif // WCS_BUILD_VNS
			} // for x
		} // for y
	} // if erange
else
	{
	// flood fill to white if no range in data
	dptr = Elevs.MainRast->ByteMap[0];
	for (y = 0; y < ElevRows; y++)
		{
		for (x = 0; x < ElevCols; x++)
			{
			*dptr++ = 255;
			} // for x
		} // for y
	} // else erange

//sprintf(&msg[0], "Edit Range: %fm - %fm", MinElev, MaxElev);
//GlobalApp->MCP->SetCurrentStatusText((char *)msg);

} // DEMPaintGUI::ComputeTerrainView

/*===========================================================================*/

void DEMPaintGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_BRUSH11, 1);
WidgetShow(IDC_BRUSH11, false); // texture brush is not implemented
if (PaintMode != DPG_MODE_GRADIENT)
	WidgetSetDisabled(IDC_CB_OPT2, 1);

} // DEMPaintGUI::DisableWidgets

/*===========================================================================*/

void DEMPaintGUI::HideWidgets(void)
{
} // DEMPaintGUI::HideWidgets

/*===========================================================================*/

void DEMPaintGUI::SetNewElev(int CtrlID, long Row, long Col)
{
float NewElev;
char NewText[128];

// add scroll pos to Row & Col
Row += RowOffset;
Col += ColOffset;

if (WidgetGetModified(CtrlID))
	{
	WidgetGetText(CtrlID, 128, NewText);
	WidgetSetModified(CtrlID, false);
	NewElev = (float)atof(NewText);
	if (Row < (long)ActiveDEM->LatEntries() && Col < (long)ActiveDEM->LonEntries())
		{
		if (ActiveDEM->Map())
			{
			ActiveDEM->StoreElevation(ActiveDEM->LatEntries() - 1 - Row, Col, NewElev);
			ActiveModified = 1;
			} // if
		} // if
	} // if

} // DEMPaintGUI::SetNewElev

/*===========================================================================*/

void DEMPaintGUI::SelectPoints(double SelectLat, double SelectLon)
{
double dx, dy;
long X, Y;

// sanity check these
if ((SelectLat > Active->NWLat) || (SelectLat < Active->SELat) || (SelectLon > Active->NWLon) || (SelectLon < Active->SELon))
	return;

if (DefiningMask)
	return;

if (PaintMode != DPG_MODE_EYEDROP)
	memcpy(UndoMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));

dy = Active->NWLat - Active->SELat;
Y = (long)(((Active->NWLat - SelectLat) / dy * (ElevRows - 1)) + 0.5);

dx = Active->NWLon - Active->SELon;
X = (long)(((Active->NWLon - SelectLon) / dx * (ElevCols - 1)) + 0.5);

switch (PaintMode)
	{
	case DPG_MODE_PAINT:
		Paint(X, Y, 0);
		break;
	case DPG_MODE_FILL:
		SeedPaint(X, Y, 0);
		break;
	case DPG_MODE_SMOOTH:
		Smooth(X, Y, 0);
		break;
	case DPG_MODE_ROUGHEN:
		Roughen(X, Y, 0);
		break;
	case DPG_MODE_SMEAR:
		Smear(X, Y, true, 0);
		break;
	case DPG_MODE_MASK:
	case DPG_MODE_EYEDROP:
	case DPG_MODE_GRADIENT:
	case DPG_MODE_ERASE:
	default:
		break;
	} // switch

DrawAreaOverlays();
if (PaintRefresh)
	{
	ComputeTerrainView();
	ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
	} // if

} // DEMPaintGUI::SelectPoints

/*===========================================================================*/

void DEMPaintGUI::ReloadDEM(void)
{
NotifyTag Changes[2];
JoeCoordSys *MyAttr;

if (Active)
	{
	ActiveDEM->FreeRawElevs();
	if (ActiveDEM->AttemptLoadDEM(Active, 1, ProjHost))
		{
		if (ActiveDEM->MoJoe && (MyAttr = (JoeCoordSys *)ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
			PaintCoords = MyAttr->Coord;
//		else
//			PaintCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
		}
	} // if

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // DEMPaintGUI::ReloadDEM

/*===========================================================================*/

short DEMPaintGUI::DisableOrigDEM(char *BaseName)
{
short Found = 0;
Joe *Clip;

for (Clip = DBHost->GetFirst(); Clip ; Clip = DBHost->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (! stricmp(BaseName, Clip->FileName()))
			{
			Found = 1;
			break;
			} // 
		} // if
	} // for

if (Found)
	{
	Clip->ClearFlags(WCS_JOEFLAG_ACTIVATED);
	} // if

return (Found);

} // DEMPaintGUI::DisableOrigDEM

/*===========================================================================*/

void DEMPaintGUI::SaveDEM(void)
{
#ifndef WCS_BUILD_DEMO
static char LastPath[256] = "";
char filename[512], *FilePath;
char BaseName[256+32], SaveName[256+32], PathName[256+32];
char NameStr[256];
FileReq *FR;
NotifyTag Changes[2];
unsigned char Result;
Joe *Clip;
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_DEMO

UserMessageDemo("The DEM Painter doesn't save in the demo version.");

#else // WCS_BUILD_DEMO

if (Active)
	{
	Result = UserMessageCustom((char *)Active->GetBestName(),
		"Save DEM file? Changes to this DEM will affect all Projects that reference this DEM file.",
		"Overwrite", "Cancel", "Select New Name", 1);	// Overwrite = 1, Cancel = 0, Select = 2
	if (Result == 0)
		return;
	ActiveDEM->FindElMaxMin();
	MaxElev = ActiveDEM->MaxEl();
	MinElev = ActiveDEM->MinEl();
	MakeTable = true;
	ComputeTerrainView();
	DrawForegroundIndicator();
	ActiveDEM->ComputeRelEl();
	if (Result == 1)
		{
		if (FilePath = ActiveDEM->AttemptFindDEMPath((char *)Active->FileName(), ProjHost))
			{
			strmfp(filename, FilePath, (char *)Active->FileName());
			strcat(filename, ".elev");
			if (ActiveDEM->SaveDEM(filename, GlobalApp->StatusLog))
				{
				Active->RecheckBounds();
				UserMessageOK((char *)Active->FileName(), "DEM has been saved.");
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				memcpy(RevertMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));
				return;
				} // if
			} // if
		UserMessageOK((char *)Active->FileName(), "Error saving DEM.");
		} // Result == 1
	else
		{
		if (FR = new FileReq)
			{
			FR->SetTitle("DEM Paint");
			FR->SetDefPat(WCS_REQUESTER_WILDCARD);
			if (LastPath[0] == 0)
				FR->SetDefPath(GlobalApp->MainProj->dirname);
			else
				FR->SetDefPath(LastPath);
			if (FR->Request(WCS_REQUESTER_FILE_SAVE))
				{
				strcpy(SaveName, FR->GetFirstName());
				AddExtension(SaveName, "elev");
				BreakFileName(SaveName, PathName, sizeof(PathName), BaseName, sizeof(BaseName));
				StripExtension(BaseName);
				DBHost->AddDEMToDatabase("DEM Paint", BaseName, ActiveDEM, PaintCoords, ProjHost, EffectsHost);
				if (ActiveDEM->SaveDEM(SaveName, GlobalApp->StatusLog))
					{
					Active->RecheckBounds();
					UserMessageOK((char *)Active->FileName(), "DEM has been saved.");
					Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
					memcpy(RevertMap, ActiveDEM->Map(), ElevCols * ElevRows * sizeof(float));
					if (! GlobalApp->MainProj->DirList_ItemExists(GlobalApp->MainProj->DL, PathName))
						GlobalApp->MainProj->DirList_Add(GlobalApp->MainProj->DL, PathName, 0);
					DisableOrigDEM(OrigName);
					// update our Joe pointer
					for (Clip = DBHost->GetFirst(); Clip ; Clip = DBHost->GetNext(Clip))
						{
						if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
							{
							if (! stricmp(BaseName, Clip->FileName()))
								{
								Active = Clip;
								break;
								} // 
							} // if
						} // for
					strcpy(OrigName, Active->GetBestName());
					sprintf(NameStr, "DEM Painter - %s", OrigName);
					SetTitle(NameStr);
					} // if
				} // if
			delete FR;
			} // if
		} // Result == 2
	} // if

#endif // WCS_BUILD_DEMO

} // DEMPaintGUI::SaveDEM

/*===========================================================================*/

void DEMPaintGUI::SetSNDefaults(void)
{
long i;
double EffectDefault[DPG_NUMANIMPAR] =
	{
	ForeElev,		// Foreground
	BackElev		// Background
	};
double RangeDefaults[DPG_NUMANIMPAR][3] =
	{
	FLT_MAX, -FLT_MAX, 5.0,		// Foreground
	FLT_MAX, -FLT_MAX, 5.0		// Background
	};

for (i = 0; i < DPG_NUMANIMPAR; i++)
	{
	AnimPar[i].SetDefaults(NULL, (char)i, EffectDefault[i]);
	AnimPar[i].SetRangeDefaults(RangeDefaults[i]);
	};

AnimPar[DPG_ANIMPAR_FOREGROUND].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[DPG_ANIMPAR_BACKGROUND].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);

AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue = ForeElev;
AnimPar[DPG_ANIMPAR_BACKGROUND].CurValue = BackElev;

} // DEMPaintGUI::SetSNDefaults

/*===========================================================================*/

// true indicates in masked area / OK to draw
bool DEMPaintGUI::Mask(long X, long Y)
{

#ifdef WCS_BUILD_VNS
if (MaskNulls && (NullMap[ElevCols * Y + X]))
	return false;
#endif // WCS_BUILD_VNS

if (MaskMode == DPG_MASKMODE_NONE)
	{
	return true;
	} // if
else
	{
	if (MaskMap[ElevCols * Y + X])
		return true;
	else
		return false;
	} // else

} // DEMPaintGUI::Mask

/*===========================================================================*/

void DEMPaintGUI::Paint(long X, long Y, long raw)
{
double tmpx, tmpy;
float newval, opacity = Opacity * 0.01f, origval, rval;
float *fptr, *mbptr, *mptr, *rptr;
long bx, by, bcx, bcy, drawx, drawy, px, py;	// brush x/y, brush center x/y, DEM x/y to modify, DEM x/y event is centered on
unsigned char *bptr;

if (raw)
	{
	// get cursor position relative to widget
	tmpx = (double)X / WidSize[0][0];
	tmpy = (double)Y / WidSize[0][1];
	if (tmpx < 0.0)
		tmpx = 0.0;
	if (tmpx > 1.0)
		tmpx = 1.0;
	if (tmpy < 0.0)
		tmpy = 0.0;
	if (tmpy > 1.0)
		tmpy = 1.0;
	// now translate widget x/y to visible area
	tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
	tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
	// now we can figure out where in the DEM we are
	px = (long)(tmpx * (ElevCols - 1));
	py = (long)(tmpy * (ElevRows - 1));
	} // if
else
	{
	px = X; py = Y;
	} // else

if (PaintMode == DPG_MODE_GRADIENT)
	{
	// do nothing here
	} // if DPG_MODE_GRADIENT
else if (PaintMode == DPG_MODE_EYEDROP)
	{
	fptr = ActiveDEM->Map() + (px * ElevRows) + (ElevRows - py - 1);
	ForeElev = *fptr;
	AnimPar[DPG_ANIMPAR_FOREGROUND].CurValue = ForeElev;
	WidgetSNSync(IDC_FOREGROUND, WP_FISYNC_NONOTIFY);
	DrawForegroundIndicator();
	}
else
	{
	bcx = BrushWidth / 2;
	bcy = BrushHeight / 2;
	for (by = 0; by < BrushHeight; by++)
		{
		bptr = BrushDef + by * BrushHeight;
		// see if this brush row is in the drawing area
		if ((py + by - bcy) < 0)
			continue;
		if ((py + by - bcy) >= ElevRows)
			break;
		for (bx = 0; bx < BrushWidth; bx++, bptr++)
			{
			// see if this brush column is in the drawing area
			if ((px + bx - bcx) < 0)
				continue;
			if ((px + bx - bcx) >= ElevCols)
				break;
			drawx = px + bx - bcx;
			drawy = py + by - bcy;
			if (Mask(drawx, drawy))
				{
				float writeval = MinElev;
				PaintRefresh = 2; // deferred
				if (drawx < DamageXMin)
					DamageXMin = drawx;
				if (drawx > DamageXMax)
					DamageXMax = drawx;
				if (drawy < DamageYMin)
					DamageYMin = drawy;
				if (drawy > DamageYMax)
					DamageYMax = drawy;
				switch (PaintMode)
					{
					case DPG_MODE_PAINT:
						{
						fptr = ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1);
						mbptr = ModifierBaseMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
						mptr = ModifierMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
						origval = *fptr;
						switch (Effect)
							{
							case DPG_EFFECT_ABS_RAISELOWER:
								newval = origval * (255 - *bptr) / 255.0f + (float)ForeElev * (*bptr / 255.0f);
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_ABS_RAISE:
								newval = origval * (255 - *bptr) / 255.0f + (float)ForeElev * (*bptr / 255.0f);
								if (newval > origval)
									*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_ABS_LOWER:
								newval = origval * (255 - *bptr) / 255.0f + (float)ForeElev * (*bptr / 255.0f);
								if (newval < origval)
									*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_REL_RAISE:
								/***
								newval = (float)ForeElev * (*bptr / 255.0f);
								*fptr = (origval + newval) * opacity + origval * (1.0f - opacity);
								***/
								newval = (float)ForeElev * (*bptr / 255.0f) * opacity;
								if (newval > 0.0f)
									{
									if (newval > *mptr)
										{
										*mptr = newval;
										*fptr = writeval = *mbptr + *mptr;
										} // if
									} // if
								break;
							case DPG_EFFECT_REL_LOWER:
								/***
								newval = (float)ForeElev * (*bptr / 255.0f);
								*fptr = (origval - newval) * opacity + origval * (1.0f - opacity);
								***/
								newval = (float)ForeElev * (*bptr / 255.0f) * opacity;
								if (newval > 0.0f)
									{
									if (newval > *mptr)
										{
										*mptr = newval;
										*fptr = writeval = *mbptr - *mptr;
										} // if
									} // if
								break;
							default:
								break;
							} // switch Effect
						break;
						} // DPG_MODE_PAINT
					case DPG_MODE_ERASE:
						{
						fptr = ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1);
						mbptr = ModifierBaseMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
						mptr = ModifierMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
						origval = *fptr;
						rptr = RevertMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
						rval = *rptr;
						switch (Effect)
							{
							case DPG_EFFECT_ABS_RAISELOWER:
								newval = origval * (255 - *bptr) / 255.0f + rval * (*bptr / 255.0f);
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_ABS_RAISE:
								newval = origval * (255 - *bptr) / 255.0f + rval * (*bptr / 255.0f);
								if (newval > origval)
									*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_ABS_LOWER:
								newval = origval * (255 - *bptr) / 255.0f + rval * (*bptr / 255.0f);
								if (newval < origval)
									*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
								break;
							case DPG_EFFECT_REL_RAISE:
								//newval = rval * (*bptr / 255.0f);
								//*fptr = (origval + newval) * opacity + origval * (1.0f - opacity);
								if (newval >= 0.0f)
									{
									if (newval < *mptr)
										{
										*mptr = newval;
										*fptr = writeval = *mbptr + *mptr;
										} // if
									} // if
								break;
							case DPG_EFFECT_REL_LOWER:
								//newval = rval * (*bptr / 255.0f);
								//*fptr = (origval - newval) * opacity + origval * (1.0f - opacity);
								if (newval >= 0.0f)
									{
									if (newval < *mptr)
										{
										*mptr = newval;
										*fptr = writeval = *mbptr - *mptr;
										} // if
									} // if
								break;
							default:
								break;
							} // switch Effect
						break;
						} // DPG_MODE_ERASE
					case DPG_MODE_MASK:
					case DPG_MODE_EYEDROP:
					case DPG_MODE_FILL:
					case DPG_MODE_GRADIENT:
					case DPG_MODE_SMOOTH:
					case DPG_MODE_ROUGHEN:
					case DPG_MODE_SMEAR:
					default:
						break;
					} // switch PaintMode
				if (writeval > MaxElev)
					{
					MaxElev = writeval;
					MakeTable = true;
					} // if
				if (writeval < MinElev)
					{
					MinElev = writeval;
					MakeTable = true;
					} // if
				} // if Mask
			} // for bx
		} // for by
	ComputeTerrainViewDamaged();
	} // else

} // DEMPaintGUI::Paint

/*===========================================================================*/

void DEMPaintGUI::DoGradFill(void)
{
float denom, dx, dy, newval, opacity, origval, r, t, xj, xk, xl, yj, yk, yl, xkj, xlk, ykj, ylk;
float *fptr, *mbptr, *mptr;
unsigned long cols, rows, x, y;

cols = ActiveDEM->LonEntries();
rows = ActiveDEM->LatEntries();
opacity = Opacity * 0.01f;
// do any calcs here that don't need to be done in the main loop body
switch (GradMode)
	{
	case DPG_GRADMODE_LINEAR:
		xk = rbx0 * ElevCols;
		yk = rby0 * ElevRows;
		xl = rbx1 * ElevCols;
		yl = rby1 * ElevRows;
		break;
	case DPG_GRADMODE_RADIAL:
		dx = rbx0 - rbx1;
		dy = rby0 - rby1;
		r = (float)sqrt(dx * dx + dy * dy);
		break;
	case DPG_GRADMODE_BUMP:
		xk = rbx0 * ElevCols;
		yk = rby0 * ElevRows;
		xl = rbx1 * ElevCols;
		yl = rby1 * ElevRows;
		break;
	} // GradMode
for (y = 0; y < rows; y++)
	{
	for (x = 0; x < cols; x++)
		{
		if (Mask(x, y))
			{
			float writeval = MinElev;
			PaintRefresh = 2; // deferred
			fptr = ActiveDEM->Map() + (x * ElevRows) + (ElevRows - y - 1);
			mbptr = ModifierBaseMap + (x * ElevRows) + (ElevRows - y - 1);
			mptr = ModifierMap + (x * ElevRows) + (ElevRows - y - 1);
			origval = *fptr;
			switch (GradMode)
				{
				case DPG_GRADMODE_LINEAR:
					// distance from point to line segment using parametric equation, but we only care about the t parameter
					xj = (float)x;
					yj = (float)y;
					xkj = xk - xj;
					ykj = yk - yj;
					xlk = xl - xk;
					ylk = yl - yk;
					denom = xlk * xlk + ylk * ylk;
					if (denom < 1.0e-6)	// points coincide - pick an end
						t = 0.0f;
					else
						{
						t = -(xkj * xlk + ykj * ylk) / denom;
						t = (float)min(max(t, 0.0), 1.0);
						}
					newval = (1.0f - t) * (float)ForeElev + t * (float)BackElev;
					switch (Effect)
						{
						case DPG_EFFECT_ABS_RAISELOWER:
							*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_RAISE:
							if (newval > origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_LOWER:
							if (newval < origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_REL_RAISE:
							//*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr + *mptr;
									} // if
								} // if
							break;
						case DPG_EFFECT_REL_LOWER:
							//*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr - *mptr;
									} // if
								} // if
							break;
						default:
							break;
						} // switch Effect
					break;
				case DPG_GRADMODE_RADIAL:
					// compute distance to first point
					dx = rbx0 - x / (float)(ElevCols - 1);
					dy = rby0 - y / (float)(ElevRows - 1);
					t = (float)sqrt(dx * dx + dy * dy);
					// convert to parametric 0.0 to 1.0
					if (t > r)
						t = r;
					t = t / r;
					newval = (1.0f - t) * (float)ForeElev + t * (float)BackElev;
					switch (Effect)
						{
						case DPG_EFFECT_ABS_RAISELOWER:
							*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_RAISE:
							if (newval > origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_LOWER:
							if (newval < origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_REL_RAISE:
							//*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr + *mptr;
									} // if
								} // if
							break;
						case DPG_EFFECT_REL_LOWER:
							//*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr - *mptr;
									} // if
								} // if
							break;
						default:
							break;
						} // switch Effect
					break;
				case DPG_GRADMODE_BUMP:
					// distance from point to line segment using parametric equation, but we only care about the t parameter
					xk = rbx0 * ElevCols;
					yk = rby0 * ElevRows;
					xl = rbx1 * ElevCols;
					yl = rby1 * ElevRows;
					xj = (float)x;
					yj = (float)y;
					xkj = xk - xj;
					ykj = yk - yj;
					xlk = xl - xk;
					ylk = yl - yk;
					denom = xlk * xlk + ylk * ylk;
					if (denom < 1.0e-6)	// points coincide - pick an end
						t = 0.0f;
					else
						{
						t = -(xkj * xlk + ykj * ylk) / denom;
						t = (float)min(max(t, 0.0), 1.0);
						}
					// cause "reflection" around midpoint
					if (t > 0.5)
						t = 1.0f - t;
					t *= 2.0f;
					newval = (1.0f - t) * (float)BackElev + t * (float)ForeElev;
					switch (Effect)
						{
						case DPG_EFFECT_ABS_RAISELOWER:
							*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_RAISE:
							if (newval > origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_ABS_LOWER:
							if (newval < origval)
								*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
							break;
						case DPG_EFFECT_REL_RAISE:
							//*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr + *mptr;
									} // if
								} // if
							break;
						case DPG_EFFECT_REL_LOWER:
							//*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
							if (newval > 0.0f)
								{
								if (newval > *mptr)
									{
									*mptr = newval;
									*fptr = writeval = *mbptr - *mptr;
									} // if
								} // if
							break;
						default:
							break;
						} // switch Effect
					break;
				default:
					break;
				} // switch
			if (writeval > MaxElev)
				{
				MaxElev = writeval;
				MakeTable = true;
				}
			if (writeval < MinElev)
				{
				MinElev = writeval;
				MakeTable = true;
				}
			} // if Mask
		} // for x
	} // for y

} // DEMPaintGUI::DoGradFill

/*===========================================================================*/

void DEMPaintGUI::SetOptions(int ButtonID)
{

WidgetCBClear(IDC_CB_OPT2);
WidgetSetDisabled(IDC_CB_OPT2, 1);
WidgetSetText(IDC_STATIC_OPT2, " ");

switch (ButtonID)
	{
	case IDC_PAINTTOOL_EYEDROPPER:
		break;
	case IDC_PAINTTOOL_FILL:
		break;
	case IDC_PAINTTOOL_FREESELECT:
		break;
	case IDC_PAINTTOOL_GRADIENT:
		WidgetSetText(IDC_STATIC_OPT2, "Gradient");
		WidgetCBAddEnd(IDC_CB_OPT2, "Linear");
		WidgetCBAddEnd(IDC_CB_OPT2, "Radial");
		WidgetCBAddEnd(IDC_CB_OPT2, "Bump");
		WidgetCBSetCurSel(IDC_CB_OPT2, (long)GradMode);
		WidgetSetDisabled(IDC_CB_OPT2, 0);
		break;
	case IDC_PAINTTOOL_MOVEREGION:
		break;
	case IDC_PAINTTOOL_PAINT:
		break;
	case IDC_PAINTTOOL_REGION:
		break;
	case IDC_PAINTTOOL_ROUGHEN:
		break;
	case IDC_PAINTTOOL_SELECT:
		break;
	case IDC_PAINTTOOL_SMEAR:
		break;
	case IDC_PAINTTOOL_SMOOTH:
		break;
	default:
		break;
	} // switch

} // DEMPaintGUI::SetOptions

/*===========================================================================*/

void DEMPaintGUI::ClearMaskMap(void)
{
unsigned char *mem = MaskMap;

memset(mem, 0, ElevCols * ElevRows);

} // DEMPaintGUI::ClearMaskMap

/*===========================================================================*/

void DEMPaintGUI::DrawMaskMap(void)
{
double tmpx, tmpy;
long px, py, Zip;
int x, y;
unsigned char *Red, *Green, *Blue, val;

Red = Elevs.OverRast->ByteMap[0];
Green = Elevs.OverRast->ByteMap[1];
Blue = Elevs.OverRast->ByteMap[2];

for (y = 0; y < Elevs.OverRast->Rows; y++)
	{
	tmpy = (double)y / (Elevs.OverRast->Rows - 1);
	// now we can figure out where in the DEM we are
	py = (long)(tmpy * (ElevRows - 1));
	Zip = y * Elevs.OverRast->Cols;
	for (x = 0; x < Elevs.OverRast->Cols; x++, Zip++)
		{
		tmpx = (double)x / (Elevs.OverRast->Cols - 1);
		// now we can figure out where in the DEM we are
		px = (long)(tmpx * (ElevCols - 1));
		// see if there's a mask there
		if (MaskMap[ElevCols * py + px])
			{
			// make transparent to show what's underneath
			Red[Zip] = Green[Zip] = Blue[Zip] = 0;
			} // if
		else
			{
			// draw a checkerboard pattern of transparent & grays:
			if (x & 1)
				{
				if (y & 1)
					{
					// transparent
					val = 0;
					} // if
				else
					{
					// basic black
					val = 31;
					} // else
				} // if
			else
				{
				if (y & 1)
					{
					// dark gray
					val = 95;
					} // if
				else
					{
					// transparent
					val = 0;
					} // else
				} // else
			Red[Zip] = Green[Zip] = Blue[Zip] = val;
			}
		} // for x
	} // for y

} // DEMPaintGUI::DrawMaskMap

/*===========================================================================*/

void DEMPaintGUI::SeedPaint(long X, long Y, long raw)
{
double tmpx, tmpy;
float newval, opacity = Opacity * 0.01f, origval;
float *fptr, *mbptr, *mptr;
long px, py, x, y, Zip;

if (raw)
	{
	// get cursor position relative to widget
	tmpx = (double)X / WidSize[0][0];
	tmpy = (double)Y / WidSize[0][1];
	if (tmpx < 0.0)
		tmpx = 0.0;
	if (tmpx > 1.0)
		tmpx = 1.0;
	if (tmpy < 0.0)
		tmpy = 0.0;
	if (tmpy > 1.0)
		tmpy = 1.0;
	// now translate widget x/y to visible area
	tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
	tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
	// now we can figure out where in the DEM we are
	px = (long)(tmpx * (ElevCols - 1));
	py = (long)(tmpy * (ElevRows - 1));
	} // if
else
	{
	px = X; py = Y;
	} // else

SeedVal = *(ActiveDEM->Map() + (px * ElevRows) + (ElevRows - py - 1));

memset(FloodMap, 0, ElevCols * ElevRows);

if (Mask(px, py))
	{
	SeedFill(px, py);

	for (y = 0; y < ElevRows; y++)
		{
		Zip = y * ElevCols;
		fptr = ActiveDEM->Map() + ElevRows - y - 1;
		for (x = 0; x < ElevCols; x++, Zip++, fptr += ElevRows)
			{
			if (Mask(x, y) && (FloodMap[Zip]))
				{
				float writeval = MinElev;
				origval = *fptr;
				mbptr = ModifierBaseMap + (x * ElevRows) + (ElevRows - y - 1);
				mptr = ModifierMap + (x * ElevRows) + (ElevRows - y - 1);
				switch (Effect)
					{
					case DPG_EFFECT_ABS_RAISELOWER:
						newval = (float)ForeElev * opacity + origval * (1.0f - opacity);
						*fptr = writeval = newval;
						break;
					case DPG_EFFECT_ABS_RAISE:
						newval = (float)ForeElev * opacity + origval * (1.0f - opacity);
						if (newval > origval)
							*fptr = writeval = newval;
						break;
					case DPG_EFFECT_ABS_LOWER:
						newval = (float)ForeElev * opacity + origval * (1.0f - opacity);
						if (newval < origval)
							*fptr = writeval = newval;
						break;
					case DPG_EFFECT_REL_RAISE:
						//newval = writeval = origval + (float)ForeElev * opacity;
						//*fptr = newval;
						newval = (float)ForeElev * opacity + origval * (1.0f - opacity);
						if (newval > 0.0f)
							{
							if (newval > *mptr)
								{
								*mptr = newval;
								*fptr = writeval = *mbptr + *mptr;
								} // if
							} // if
						break;
					case DPG_EFFECT_REL_LOWER:
						//newval = writeval = origval - (float)ForeElev * opacity;
						//*fptr = newval;
						newval = (float)ForeElev * opacity + origval * (1.0f - opacity);
						if (newval > 0.0f)
							{
							if (newval > *mptr)
								{
								*mptr = newval;
								*fptr = writeval = *mbptr - *mptr;
								} // if
							} // if
						break;
					default:
						break;
					} // switch
				if (writeval > MaxElev)
					{
					MaxElev = writeval;
					MakeTable = true;
					} // if
				if (writeval < MinElev)
					{
					MinElev = writeval;
					MakeTable = true;
					} // if
				} // switch Effect
			} // for x
		} // for y

	} // if Mask

} // DEMPaintGUI::SeedPaint

/*===========================================================================*/

// if DPG_MASKMODE_FREEHAND, FloodMap is inited to 255, then poly is drawn with 254, then floodfilled with 0
// else FloodMap is inited to 0, filled areas end up with 255
bool DEMPaintGUI::FillWorthy(long Col, long Row)
{
float *fptr, origval;
long Zip;

Zip = Row * ElevCols + Col;
// return if we've been here before
if ((PaintMode == DPG_MODE_FILL) && (FloodMap[Zip]))
	return false;
else if ((MaskMode == DPG_MASKMODE_WAND) && (MaskMap[Zip]))
	return false;
else if ((MaskMode == DPG_MASKMODE_FREEHAND) && (FloodMap[Zip] == 0))
	return false;

if (MaskMode == DPG_MASKMODE_FREEHAND)
	{
	if (FloodMap[Zip] == 255)
		{
		FloodMap[Zip] = 0;
		return true;
		} // if
	else
		return false;
	} // if
else
	{
	fptr = ActiveDEM->Map() + (Col * ElevRows) + (ElevRows - Row - 1);
	origval = *fptr;

	if ((origval >= (SeedVal - Tolerance)) && (origval <= (SeedVal + Tolerance)))
		{
		if (PaintMode == DPG_MODE_FILL)
			FloodMap[Zip] = 255;
		else if (MaskMode == DPG_MASKMODE_WAND)
			MaskMap[Zip] = 255;
		return true;
		} // if
	else
		return false;
	} // else

} // DEMPaintGUI::FillWorthy

/*===========================================================================*/

#define DP_MAXFILLSTACK 40000
#define DP_PUSH(Y, XL, XR, DY)\
	if ((sp < (stack + DP_MAXFILLSTACK)) && (Y + (DY) >= 0) && (Y + (DY) < ElevRows))\
	{sp->y = Y; sp->xl = XL, sp->xr = XR; sp->dy = DY, sp++;}
#define DP_POP(Y, XL, XR, DY)\
	{sp--; Y = sp->y + (DY = sp->dy); XL = sp->xl; XR = sp->xr;}
typedef struct {long y, xl, xr, dy;} DP_Segment;

void DEMPaintGUI::SeedFill(long Col, long Row)
{
long l, x, y, x1, x2, dy;
DP_Segment stack[DP_MAXFILLSTACK], *sp = stack;

x = Col; y = Row;
DP_PUSH(y, x, x, 1);
DP_PUSH((y + 1), x, x, -1);
while (sp > stack)
	{
	DP_POP(y, x1, x2, dy);
	for (x = x1; (x >= 0) && FillWorthy(x, y); x--)
		{
		PaintRefresh = true;
		} // for
	if (x >= x1)
		goto skip;
	l = x + 1;
	if (l < x1)
		DP_PUSH(y, l, (x1 - 1), -dy);
	x = x1 + 1;
	do
		{
		for (; (x < ElevCols) && FillWorthy(x, y); x++)
			{
			PaintRefresh = true;
			} // for
		DP_PUSH(y, l, (x - 1), dy);
		if (x > (x2 +1))
			DP_PUSH(y, (x2 + 1), (x - 1), -dy);
skip:   for (x++; x <= x2 && !FillWorthy(x, y); x++)
			{
			}	// yes, this is truly empty
		l = x;
		} while (x <= x2);
	} // while

} // DEMPaintGUI::SeedFill

/*===========================================================================*/

void DEMPaintGUI::BoxMask(void)
{
long bottom, left, right, top, x, y, zip;

if (MaskOpts != DPG_MASKOPT_NONE)
	memcpy(TmpMaskMap, MaskMap, ElevCols * ElevRows);

memset(MaskMap, 0, ElevCols * ElevRows);

left = (long)(min(boxx0, boxx1) * (ElevCols - 1) + 0.5f);
right = (long)(max(boxx0, boxx1) * (ElevCols - 1) + 0.5f);
top = (long)(min(boxy0, boxy1) * (ElevRows - 1) + 0.5f);
bottom = (long)(max(boxy0, boxy1) * (ElevRows - 1) + 0.5f);

for (y = top; y <= bottom; y++)
	{
	zip = y * ElevCols;
	for (x = left, zip += x; x <= right; x++, zip++)
		{
		MaskMap[zip] = 255;
		} // for x
	} // for y

if (MaskOpts == DPG_MASKOPT_ADD)
	AddMasks();
else if (MaskOpts == DPG_MASKOPT_SUB)
	SubMasks();

} // DEMPaintGUI::BoxMask

/*===========================================================================*/

void DEMPaintGUI::MagicWand(long X, long Y)
{

SeedVal = *(ActiveDEM->Map() + (X * ElevRows) + (ElevRows - Y - 1));

if (MaskOpts != DPG_MASKOPT_NONE)
	memcpy(TmpMaskMap, MaskMap, ElevCols * ElevRows);

memset(MaskMap, 0, ElevCols * ElevRows);

SeedFill(X, Y);

if (MaskOpts == DPG_MASKOPT_ADD)
	AddMasks();
else if (MaskOpts == DPG_MASKOPT_SUB)
	SubMasks();

} // DEMPaintGUI::MagicWand

/*===========================================================================*/

void DEMPaintGUI::AddMasks(void)
{
long i;

for (i = 0; i < ElevCols * ElevRows; i++)
	MaskMap[i] += TmpMaskMap[i];

} // DEMPaintGUI::AddMasks

/*===========================================================================*/

void DEMPaintGUI::SubMasks(void)
{
long i;

for (i = 0; i < ElevCols * ElevRows; i++)
	{
	if (MaskMap[i])
		MaskMap[i] = 0;
	else
		MaskMap[i] = TmpMaskMap[i];
	} // for

} // DEMPaintGUI::SubMasks

/*===========================================================================*/

void DEMPaintGUI::CreateBrushMap(unsigned long BrushNum, long size)
{
double d, i, r, r1, r2;	// distance, intensity, radius, start radius, full on radius
double ri = 0.0;		// inner radius
float c, dx, dy;		// center in pixel coords
long bx, by;
unsigned char *bptr, *optr, val;

bptr = BrushDef;
optr = BrushOutline;

// the solid brush defs
if (BrushNum < 6)
	{
	r = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r > 2.5)
		ri = r - 2.5;
	c = (float)(r - 0.5f);
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r)
				val = 255;
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 0..5
// the airbrush defs
else if (BrushNum == 6)
	{
	r1 = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r1 > 2.5)
		ri = r1 - 2.5;
	c = (float)(r1 - 0.5f);
	r2 = r1 * 0.75;
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r2)
				val = 255;
			else if (d <= r1)
				{
				i = (r1 - d) / (r1 - r2);
				val = (unsigned char)(i * 255.0);
				}
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r1)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 6
else if (BrushNum == 7)
	{
	r1 = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r1 > 2.5)
		ri = r1 - 2.5;
	c = (float)(r1 - 0.5f);
	r2 = r1 * 0.67;
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r2)
				val = 255;
			else if (d <= r1)
				{
				i = (r1 - d) / (r1 - r2);
				val = (unsigned char)(i * 255.0);
				}
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r1)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 7
else if (BrushNum == 8)
	{
	r1 = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r1 > 2.5)
		ri = r1 - 2.5;
	c = (float)(r1 - 0.5f);
	r2 = r1 * 0.67;
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r2)
				val = 255;
			else if (d <= r1)
				{
				i = (r1 - d) / (r1 - r2);
				val = (unsigned char)(i * 255.0);
				}
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r1)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 8
else if (BrushNum == 9)
	{
	r1 = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r1 > 2.5)
		ri = r1 - 2.5;
	c = (float)(r1 - 0.5f);
	r2 = r1 * 0.33;
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r2)
				val = 255;
			else if (d <= r1)
				{
				i = (r1 - d) / (r1 - r2);
				val = (unsigned char)(i * 255.0);
				}
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r1)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 9
else if (BrushNum == 10)
	{
	r1 = size * 0.5;
	// outline brush thickness is 2.5 pixels
	if (r1 > 2.5)
		ri = r1 - 2.5;
	c = (float)(r1 - 0.5f);
	// r2 = 0.0
	for (by = 0; by < size; by++)
		{
		dy = c - by;
		for (bx = 0; bx < size; bx++)
			{
			dx = c - bx;
			d = sqrt(dx * dx + dy * dy);
			if (d <= r1)
				{
				i = (r1 - d) / r1;
				val = (unsigned char)(i * 255.0);
				}
			else
				val = 0;
			*bptr++ = val;
			// outline brush
			if (d >= ri && d <= r1)
				val = 255;
			else
				val = 0;
			*optr++ = val;
			} // for x
		} // for y
	} // Brush 10

} // DEMPaintGUI::CreateBrushMap

/*===========================================================================*/

void DEMPaintGUI::Smooth(long X, long Y, long raw)
{
double tmpx, tmpy;
float elevsum, *fptr, newval, opacity = Opacity * 0.01f, *sptr, origval;
long bx, by, bcx, bcy, count, drawx, drawy, dx, dy, px, py;	// brush x/y, brush center x/y, count, DEM x/y to modify, deltas, DEM x/y event is centered on
unsigned char *bptr;

if (raw)
	{
	// get cursor position relative to widget
	tmpx = (double)X / WidSize[0][0];
	tmpy = (double)Y / WidSize[0][1];
	if (tmpx < 0.0)
		tmpx = 0.0;
	if (tmpx > 1.0)
		tmpx = 1.0;
	if (tmpy < 0.0)
		tmpy = 0.0;
	if (tmpy > 1.0)
		tmpy = 1.0;
	// now translate widget x/y to visible area
	tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
	tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
	// now we can figure out where in the DEM we are
	px = (long)(tmpx * (ElevCols - 1));
	py = (long)(tmpy * (ElevRows - 1));
	} // if
else
	{
	px = X; py = Y;
	} // else

bcx = BrushWidth / 2;
bcy = BrushHeight / 2;

// compute a 5x5 lowpass filter for the area covered by the brush
fptr = BrushWorkArea;
for (by = 0; by < BrushHeight; by++)
	{
	for (bx = 0; bx < BrushWidth; bx++)
		{
		elevsum = 0.0f;
		count = 0;
		for (dy = -2; dy <= 2; dy++)
			{
			// see if we're in a valid source row
			if ((py + by - bcy + dy) < 0)
				continue;
			if ((py + by - bcy + dy) >= ElevRows)
				break;
			for (dx = -2; dx <= 2; dx++)
				{
				// see if we're in a valid source column
				if ((px + bx - bcx + dx) < 0)
					continue;
				if ((px + bx - bcx + dx) >= ElevCols)
					break;
				// draw is really the read position here
				drawx = px + bx - bcx + dx;
				drawy = py + by - bcy + dy;
				elevsum += *(ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1));
				count++;
				} // for dx
			} // for dy
		*fptr++ = elevsum / count;	// count will never be 0
		} // for bx
	} // for by

// apply the filter using the brush map
for (by = 0; by < BrushHeight; by++)
	{
	bptr = BrushDef + by * BrushHeight;
	// see if this brush row is in the drawing area
	if ((py + by - bcy) < 0)
		continue;
	if ((py + by - bcy) >= ElevRows)
		break;
	for (bx = 0; bx < BrushWidth; bx++, bptr++)
		{
		// see if this brush column is in the drawing area
		if ((px + bx - bcx) < 0)
			continue;
		if ((px + bx - bcx) >= ElevCols)
			break;
		drawx = px + bx - bcx;
		drawy = py + by - bcy;
		if (Mask(drawx, drawy))
			{
			float writeval = MinElev;
			PaintRefresh = 2; // deferred
			if (drawx < DamageXMin)
				DamageXMin = drawx;
			if (drawx > DamageXMax)
				DamageXMax = drawx;
			if (drawy < DamageYMin)
				DamageYMin = drawy;
			if (drawy > DamageYMax)
				DamageYMax = drawy;
			fptr = ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1);
			sptr = BrushWorkArea + by * BrushHeight + bx;
			origval = *fptr;
			switch (Effect)
				{
				case DPG_EFFECT_ABS_RAISELOWER:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_RAISE:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					if (newval > origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_LOWER:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					if (newval < origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				/***
				case DPG_EFFECT_REL_RAISE:
					newval = *sptr * (*bptr / 255.0f);
					*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_REL_LOWER:
					newval = *sptr * (*bptr / 255.0f);
					*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
					break;
				***/
				default:
				case DPG_EFFECT_REL_RAISE:
				case DPG_EFFECT_REL_LOWER:
					break;
				} // switch Effect
			if (writeval > MaxElev)
				{
				MaxElev = writeval;
				MakeTable = true;
				} // if
			if (writeval < MinElev)
				{
				MinElev = writeval;
				MakeTable = true;
				} // if
			} // if Mask
		} // for bx
	} // for by
ComputeTerrainViewDamaged();

} // DEMPaintGUI::Smooth

/*===========================================================================*/

void DEMPaintGUI::Smear(long X, long Y, bool Init, long raw)
{
double tmpx, tmpy;
float lastval, newval, opacity = Opacity * 0.01f * 0.5f, origval;
float *fptr, *last, *mbptr, *mptr;
static long lastx = 0, lasty = 0;
long bx, by, bcx, bcy, drawx, drawy, px, py;	// brush x/y, brush center x/y, DEM x/y to modify, DEM x/y event is centered on
unsigned char *bptr;

fptr = last = NULL;	// Lint fix

if (raw)
	{
	// get cursor position relative to widget
	tmpx = (double)X / WidSize[0][0];
	tmpy = (double)Y / WidSize[0][1];
	if (tmpx < 0.0)
		tmpx = 0.0;
	if (tmpx > 1.0)
		tmpx = 1.0;
	if (tmpy < 0.0)
		tmpy = 0.0;
	if (tmpy > 1.0)
		tmpy = 1.0;
	// now translate widget x/y to visible area
	tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
	tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
	// now we can figure out where in the DEM we are
	px = (long)(tmpx * (ElevCols - 1));
	py = (long)(tmpy * (ElevRows - 1));
	} // if
else
	{
	px = X; py = Y;
	} // else

if (Init)
	{
	lastx = px;
	lasty = py;
	return;
	}

bcx = BrushWidth / 2;
bcy = BrushHeight / 2;

// blend last area from DEM with this area from DEM
for (by = 0; by < BrushHeight; by++)
	{
	bptr = BrushDef + by * BrushHeight;
	// see if this brush row is in the source area
	if ((lasty + by - bcy) < 0)
		continue;
	if ((lasty + by - bcy) >= ElevRows)
		break;
	// see if this brush row is in the drawing area
	if ((py + by - bcy) < 0)
		continue;
	if ((py + by - bcy) >= ElevRows)
		break;
	for (bx = 0; bx < BrushWidth; bx++, bptr++, fptr++, last++)
		{
		// see if this brush column is in the source area
		if ((lastx + bx - bcx) < 0)
			continue;
		if ((lastx + bx - bcx) >= ElevCols)
			break;
		// see if this brush column is in the drawing area
		if ((px + bx - bcx) < 0)
			continue;
		if ((px + bx - bcx) >= ElevCols)
			break;
		drawx = px + bx - bcx;
		drawy = py + by - bcy;
		if (Mask(drawx, drawy))
			{
			float writeval = MinElev;
			PaintRefresh = 2; // deferred;
			if (drawx < DamageXMin)
				DamageXMin = drawx;
			if (drawx > DamageXMax)
				DamageXMax = drawx;
			if (drawy < DamageYMin)
				DamageYMin = drawy;
			if (drawy > DamageYMax)
				DamageYMax = drawy;
			last = ActiveDEM->Map() + ((lastx + bx - bcx) * ElevRows) + (ElevRows - (lasty + by - bcy) - 1);
			lastval = *last;
			fptr = ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1);
			mbptr = ModifierBaseMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
			mptr = ModifierMap + (drawx * ElevRows) + (ElevRows - drawy - 1);
			origval = *fptr;
			switch (Effect)
				{
				case DPG_EFFECT_ABS_RAISELOWER:
					newval = origval * (255 - *bptr) / 255.0f + lastval * (*bptr / 255.0f);
					*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_RAISE:
					newval = origval * (255 - *bptr) / 255.0f + lastval * (*bptr / 255.0f);
					if (newval > origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_LOWER:
					newval = origval * (255 - *bptr) / 255.0f + lastval * (*bptr / 255.0f);
					if (newval < origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				/***
				case DPG_EFFECT_REL_RAISE:
					//newval = lastval * (*bptr / 255.0f);
					//*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
					newval = origval * (255 - *bptr) / 255.0f + lastval * (*bptr / 255.0f);
					if (newval > 0.0f)
						{
						if (newval > *mptr)
							{
							*mptr = newval;
							*fptr = writeval = *mbptr + *mptr;
							} // if
						} // if
					break;
				case DPG_EFFECT_REL_LOWER:
					//newval = lastval * (*bptr / 255.0f);
					//*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
					newval = origval * (255 - *bptr) / 255.0f + lastval * (*bptr / 255.0f);
					if (newval > 0.0f)
						{
						if (newval > *mptr)
							{
							*mptr = newval;
							*fptr = writeval = *mbptr - *mptr;
							} // if
						} // if
					break;
				***/
				default:
				case DPG_EFFECT_REL_RAISE:
				case DPG_EFFECT_REL_LOWER:
					break;
				} // switch Effect
			if (writeval > MaxElev)
				{
				MaxElev = writeval;
				MakeTable = true;
				} // if
			if (writeval < MinElev)
				{
				MinElev = writeval;
				MakeTable = true;
				} // if
			} // if Mask
		} // for bx
	} // for by
ComputeTerrainViewDamaged();

lastx = px;
lasty = py;

} // DEMPaintGUI::Smear

/*===========================================================================*/

void DEMPaintGUI::Roughen(long X, long Y, long raw)
{
double maxel, minel, tmpx, tmpy;
float avg, elevsum, middle, newval, opacity = Opacity * 0.01f, origval;
long bx, by, bcx, bcy, count, drawx, drawy, dx, dy, px, py;	// brush x/y, brush center x/y, count, DEM x/y to modify, deltas, DEM x/y event is centered on
float *fptr, *sptr;
unsigned char *bptr;

maxel = 2 * MaxElev;
minel = MinElev * 0.5;

if (raw)
	{
	// get cursor position relative to widget
	tmpx = (double)X / WidSize[0][0];
	tmpy = (double)Y / WidSize[0][1];
	if (tmpx < 0.0)
		tmpx = 0.0;
	if (tmpx > 1.0)
		tmpx = 1.0;
	if (tmpy < 0.0)
		tmpy = 0.0;
	if (tmpy > 1.0)
		tmpy = 1.0;
	// now translate widget x/y to visible area
	tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
	tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;
	// now we can figure out where in the DEM we are
	px = (long)(tmpx * (ElevCols - 1));
	py = (long)(tmpy * (ElevRows - 1));
	} // if
else
	{
	px = X; py = Y;
	} // else

bcx = BrushWidth / 2;
bcy = BrushHeight / 2;

// compute a value for the area covered by the brush which magnifies differences
fptr = BrushWorkArea;
for (by = 0; by < BrushHeight; by++)
	{
	for (bx = 0; bx < BrushWidth; bx++)
		{
		elevsum = 0.0f;
		count = 0;
		for (dy = -1; dy <= 1; dy++)
			{
			// see if we're in a valid source row
			if ((py + by - bcy + dy) < 0)
				continue;
			if ((py + by - bcy + dy) >= ElevRows)
				break;
			for (dx = -1; dx <= 1; dx++)
				{
				// see if we're in a valid source column
				if ((px + bx - bcx + dx) < 0)
					continue;
				if ((px + bx - bcx + dx) >= ElevCols)
					break;
				// draw is really the read position here
				drawx = px + bx - bcx + dx;
				drawy = py + by - bcy + dy;
				elevsum += *(ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1));
				if ((dx == 0) && (dy == 0))
					middle = *(ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1));
				count++;
				} // for dx
			} // for dy
		avg = elevsum / count;
		newval = middle + (middle - avg) / 10.0f;
		// need to clamp to something, otherwise values can run off to infinity quickly
		if (newval > maxel)
			newval = (float)maxel;
		else if (newval < minel)
			newval = (float)minel;
		*fptr++ = newval;
		} // for bx
	} // for by

// apply the filter using the brush map
for (by = 0; by < BrushHeight; by++)
	{
	bptr = BrushDef + by * BrushHeight;
	// see if this brush row is in the drawing area
	if ((py + by - bcy) < 0)
		continue;
	if ((py + by - bcy) >= ElevRows)
		break;
	for (bx = 0; bx < BrushWidth; bx++, bptr++)
		{
		// see if this brush column is in the drawing area
		if ((px + bx - bcx) < 0)
			continue;
		if ((px + bx - bcx) >= ElevCols)
			break;
		drawx = px + bx - bcx;
		drawy = py + by - bcy;
		if (Mask(drawx, drawy))
			{
			float writeval = MinElev;
			PaintRefresh = 2; // deferred;
			if (drawx < DamageXMin)
				DamageXMin = drawx;
			if (drawx > DamageXMax)
				DamageXMax = drawx;
			if (drawy < DamageYMin)
				DamageYMin = drawy;
			if (drawy > DamageYMax)
				DamageYMax = drawy;
			fptr = ActiveDEM->Map() + (drawx * ElevRows) + (ElevRows - drawy - 1);
			sptr = BrushWorkArea + by * BrushHeight + bx;
			origval = *fptr;
			switch (Effect)
				{
				case DPG_EFFECT_ABS_RAISELOWER:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_RAISE:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					if (newval > origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_ABS_LOWER:
					newval = origval * (255 - *bptr) / 255.0f + *sptr * (*bptr / 255.0f);
					if (newval < origval)
						*fptr = writeval = newval * opacity + origval * (1.0f - opacity);
					break;
				/***
				case DPG_EFFECT_REL_RAISE:
					newval = *sptr * (*bptr / 255.0f);
					*fptr = writeval = (origval + newval) * opacity + origval * (1.0f - opacity);
					break;
				case DPG_EFFECT_REL_LOWER:
					newval = *sptr * (*bptr / 255.0f);
					*fptr = writeval = (origval - newval) * opacity + origval * (1.0f - opacity);
					break;
				***/
				default:
				case DPG_EFFECT_REL_RAISE:
				case DPG_EFFECT_REL_LOWER:
					break;
				} // switch Effect
			if (writeval > MaxElev)
				{
				MaxElev = writeval;
				MakeTable = true;
				} // if
			if (writeval < MinElev)
				{
				MinElev = writeval;
				MakeTable = true;
				} // if
			} // if Mask
		} // for bx
	} // for by
ComputeTerrainViewDamaged();

} // DEMPaintGUI::Roughen

/*===========================================================================*/

// draw vertices via basic Bresenham using 2 octants
void DEMPaintGUI::DrawFreehandVerts(void)
{
long Cols, Rows, Zip;
long dx, dy, i, lx, ly, tmp, x0, x1, xdir, y0, y1;
unsigned char *Red, *Green, *Blue, octant;

Cols = Elevs.OverRast->Cols;
Rows = Elevs.OverRast->Rows;
Red = Elevs.OverRast->ByteMap[0];
Green = Elevs.OverRast->ByteMap[1];
Blue = Elevs.OverRast->ByteMap[2];

x0 = (long)((Cols - 1) * FreehandVerts[0][0] + 0.5f);
y0 = (long)((Rows - 1) * FreehandVerts[0][1] + 0.5f);

for (i = 1; i < FreehandCount; i++)
	{
	lx = x1 = (long)((Cols - 1) * FreehandVerts[i][0] + 0.5f);
	ly = y1 = (long)((Rows - 1) * FreehandVerts[i][1] + 0.5f);

	if (y0 > y1)
		{
		tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
		} // if
	dx = x1 - x0;
	dy = y1 - y0;
	if (dx > 0)
		{
		xdir = 1;
		if (dx > dy)
			octant = 0;
		else
			octant = 1;
		} // if dx
	else
		{
		dx = -dx;	// ie: absolute value
		xdir = -1;
		if (dx >dy)
			octant = 0;
		else
			octant = 1;
		} // else

	// draw the first pixel
	Zip = y0 * Cols + x0;
	Red[Zip] = (unsigned char)(255);
	Green[Zip] = (unsigned char)(79);
	Blue[Zip] = (unsigned char)(0);
	// now finish drawing line via octant logic
	if (octant == 0)
		{
		long DYx2, DYx2SubDXx2, Err;

		DYx2 = dy * 2;
		DYx2SubDXx2 = DYx2 - dx * 2;
		Err = DYx2 - dx;

		while (dx--)
			{
			if (Err >= 0)
				{
				y0++;
				Err += DYx2SubDXx2;
				} // if
			else
				Err += DYx2;
			x0 += xdir;
			Zip = y0 * Cols + x0;
			Red[Zip] = (unsigned char)(255);
			Green[Zip] = (unsigned char)(79);
			Blue[Zip] = (unsigned char)(0);
			} // while
		} // octant 0
	else
		{
		long DXx2, DXx2SubDYx2, Err;

		DXx2 = dx * 2;
		DXx2SubDYx2 = DXx2 - dy * 2;
		Err = DXx2 - dy;

		while (dy--)
			{
			if (Err >= 0)
				{
				x0 += xdir;
				Err += DXx2SubDYx2;
				}
			else
				Err += DXx2;
			y0++;
			Zip = y0 * Cols + x0;
			Red[Zip] = (unsigned char)(255);
			Green[Zip] = (unsigned char)(79);
			Blue[Zip] = (unsigned char)(0);
			} // while
		} // octant 1

	// make last endpoint new startpoint
	x0 = lx;
	y0 = ly;

	} // for i


} // DEMPaintGUI::DrawFreehandVerts

/*===========================================================================*/

// draw vertices via basic Bresenham using 2 octants (remember there's a 1 pixel border around our true drawing area)
void DEMPaintGUI::DrawFreehandVerts2(void)
{
long Cols, Rows, Zip;
long dx, dy, i, lx, ly, tmp, x0, x1, xdir, y0, y1;
unsigned char fillval = 254, octant;

Cols = ElevCols;
Rows = ElevRows;

// first line will be drawn from last vertex to first vertex
x0 = (long)((Cols - 3) * FreehandVerts[FreehandCount - 1][0] + 0.5f) + 1;
y0 = (long)((Rows - 3) * FreehandVerts[FreehandCount - 1][1] + 0.5f) + 1;

for (i = 0; i < FreehandCount; i++)
	{
	lx = x1 = (long)((Cols - 3) * FreehandVerts[i][0] + 0.5f) + 1;
	ly = y1 = (long)((Rows - 3) * FreehandVerts[i][1] + 0.5f) + 1;

	if (y0 > y1)
		{
		tmp = x0;
		x0 = x1;
		x1 = tmp;
		tmp = y0;
		y0 = y1;
		y1 = tmp;
		} // if
	dx = x1 - x0;
	dy = y1 - y0;
	if (dx > 0)
		{
		xdir = 1;
		if (dx > dy)
			octant = 0;
		else
			octant = 1;
		} // if dx
	else
		{
		dx = -dx;	// ie: absolute value
		xdir = -1;
		if (dx >dy)
			octant = 0;
		else
			octant = 1;
		} // else

	// draw the first pixel (remember that 1 pixel border)
	Zip = y0 * Cols + x0;
	FloodMap[Zip] = fillval;
	// now finish drawing line via octant logic
	if (octant == 0)
		{
		long DYx2, DYx2SubDXx2, Err;

		DYx2 = dy * 2;
		DYx2SubDXx2 = DYx2 - dx * 2;
		Err = DYx2 - dx;

		while (dx--)
			{
			if (Err >= 0)
				{
				y0++;
				Err += DYx2SubDXx2;
				} // if
			else
				Err += DYx2;
			x0 += xdir;
			Zip = y0 * Cols + x0;
			FloodMap[Zip] = fillval;
			} // while
		} // octant 0
	else
		{
		long DXx2, DXx2SubDYx2, Err;

		DXx2 = dx * 2;
		DXx2SubDYx2 = DXx2 - dy * 2;
		Err = DXx2 - dy;

		while (dy--)
			{
			if (Err >= 0)
				{
				x0 += xdir;
				Err += DXx2SubDYx2;
				}
			else
				Err += DXx2;
			y0++;
			Zip = y0 * Cols + x0;
			FloodMap[Zip] = fillval;
			} // while
		} // octant 1

	// make last endpoint new startpoint
	x0 = lx;
	y0 = ly;

	} // for i

} // DEMPaintGUI::DrawFreehandVerts2

/*===========================================================================*/

void DEMPaintGUI::MoveSelect(float fdx, float fdy, bool ShowOnly)
{
long dx, dy, px, py, x, y;
float *dptr, *sptr;
unsigned char *rptr, *gptr;
unsigned char val;

dx = long(fdx * (ElevCols - 1) + 0.5f);
dy = long(fdy * (ElevRows - 1) + 0.5f);

if (ShowOnly)
	{
	rptr = Elevs.OverRast->ByteMap[0];
	gptr = Elevs.OverRast->ByteMap[1];

	for (y = 0; y < ElevRows; y++)
		{
		py = y + dy;
		for (x = 0, px = dx; x < ElevCols; x++, px++)
			{
			if ((px >= 0) && (py >= 0) && (px < ElevCols) && (py < ElevRows) && Mask(px, py))
				val = *(Elevs.MainRast->ByteMap[0] + py * ElevCols + px);
			else
				val = 0;
			*rptr++ = *gptr++ = val;
			} // for x
		} // for y

	ConfigureRD(NativeWin, IDC_DPG_AREA, &Elevs);
	} // if ShowOnly
else
	{
	sptr = ActiveDEM->Map();
	memcpy(WorkMap, sptr, ElevCols * ElevRows * sizeof(float));

	for (y = 0; y < ElevRows; y++)
		{
		dptr = ActiveDEM->Map() + ElevRows - y - 1;
		py = y + dy;
		for (x = 0, px = dx; x < ElevCols; x++, px++, dptr += ElevRows)
			{
			if ((px >= 0) && (py >= 0) && (px < ElevCols) && (py < ElevRows) && Mask(px, py))
				*dptr = *(WorkMap + (px * ElevRows) + (ElevRows - py - 1));
			} // for x
		} // for y

	PaintRefresh = true;
	} // else ShowOnly

} // DEMPaintGUI::MoveSelect

/*===========================================================================*/

void DEMPaintGUI::DrawForegroundIndicator(void)
{
double y;
long x, xmax, Zip;
unsigned char *rptr, *gptr, *bptr;
char heightstr[16];
unsigned char r = 63, g = 239, b = 0;

Gradient.OverRast->ClearByteBand(0);
Gradient.OverRast->ClearByteBand(1);
Gradient.OverRast->ClearByteBand(2);

if ((ForeElev > MaxElev) || (MaxElev == MinElev))
	{
	y = 0.0;
	sprintf(heightstr, "++ %8g ++", ForeElev);
	}
else if (ForeElev < MinElev)
	{
	y = 1.0;
	sprintf(heightstr, "-- %8g --", ForeElev);
	}
else
	{
	y = 1.0 - (ForeElev - MinElev) / (MaxElev - MinElev);
	sprintf(heightstr, "%8g", ForeElev);
	}

Zip = (long)(y * (Gradient.OverRast->Rows - 2) + 0.5) * Gradient.OverRast->Cols;
rptr = Gradient.OverRast->ByteMap[0] + Zip;
gptr = Gradient.OverRast->ByteMap[1] + Zip;
bptr = Gradient.OverRast->ByteMap[2] + Zip;

xmax = Gradient.OverRast->Cols * 2;	// draw two scan lines so we'll always be visible
for (x = 0; x < xmax; x++)
	{
	*rptr++ = r;
	*gptr++ = g;
	*bptr++ = b;
	} // for x

WidgetSetText(IDC_TEXT_FGPEN, heightstr);
ConfigureRD(NativeWin, IDC_DPG_ELEVGRAD, &Gradient);

} // DEMPaintGUI::DrawForegroundIndicator

/*===========================================================================*/

unsigned long DEMPaintGUI::InitPreview(void)
{

if (PreviewJoe)
	{
	PreviewJoe->ZeroUpTree();
	DBHost->ReBoundTree(WCS_DATABASE_DYNAMIC);
	} // if
else
	{
	if (PreviewJoe = new ("DEM Paint") Joe)
		{
		PreviewJoe->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
		PreviewJoe->SetLineWidth(1);
		PreviewJoe->SetLineStyle(4);
		PreviewJoe->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);

		if (! DBHost->AddJoe(PreviewJoe, WCS_DATABASE_DYNAMIC, ProjHost))
			{
			UserMessageOK("DEM Paint", "Could not add object to Database. Preview not available.");
			delete PreviewJoe;
			PreviewJoe = NULL;
			return (0);
			} // else

		if (MyDEM = new JoeDEM)
			{
			// Transfer DEM bounds values into Joe
			if (ActiveDEM->MoJoe)
				{
				PreviewJoe->NWLat = ActiveDEM->MoJoe->NWLat;
				PreviewJoe->NWLon = ActiveDEM->MoJoe->NWLon;
				PreviewJoe->SELon = ActiveDEM->MoJoe->SELon;
				PreviewJoe->SELat = ActiveDEM->MoJoe->SELat;
				MyDEM->MaxFract = 7;
				// <<<MAXMINELFLOAT>>>
				MyDEM->MaxEl = (short)ActiveDEM->MaxEl();
				MyDEM->MinEl = (short)ActiveDEM->MinEl();
				MyDEM->SumElDif = (float)ActiveDEM->SumElDif ();
				MyDEM->SumElDifSq = (float)ActiveDEM->SumElDifSq();
				MyDEM->ElScale = ActiveDEM->ElScale();
				MyDEM->ElDatum = ActiveDEM->ElDatum();
				MyDEM->Pristine = ActiveDEM;
				
				PreviewJoe->AddAttribute(MyDEM);
				} // if
			} // if
		else
			{
			UserMessageOK("DEM Paint", "Could not add DEM attribute. Preview not available.");
			// remove from database
			if (PreviewJoe->RemoveMe(EffectsHost))
				{
				delete PreviewJoe;
				} // if
			// null the local pointer anyway because deletion is now in the hands of the 
			// database if it wasn't successfully removed
			PreviewJoe = NULL;
			return (0);
			} // 
		DBHost->BoundUpTree(PreviewJoe);
		} // if
	} // else

return (1);

} // DEMPaintGUI::InitPreview

/*===========================================================================*/

void DEMPaintGUI::UpdatePreview(void)
{
NotifyTag Changes[4];

if (Previewing)
	{
	if (InitPreview())
		{
		// if InitPreview succeeded, PreviewJoe will exist
		Changes[0] = MAKE_ID(PreviewJoe->GetNotifyClass(), PreviewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		// Notify Views to regen dynamic entities
		Changes[1] = MAKE_ID(WCS_NOTIFYCLASS_VIEWGUI, WCS_NOTIFYSUBCLASS_NEWTERRAGEN, 0, 0);
		Changes[2] = 0;
		GlobalApp->AppEx->GenerateNotify(Changes, PreviewJoe->GetRAHostRoot());
		} // if initialized
	} // if
else
	DumpPreview();

} // DEMPaintGUI::UpdatePreview

/*===========================================================================*/

void DEMPaintGUI::RestoreBrushRegion(void)
{
long x, y, Zip;
unsigned char *Red, *Green, *Blue, *Save;

Red = Elevs.OverRast->ByteMap[0];
Green = Elevs.OverRast->ByteMap[1];
Blue = Elevs.OverRast->ByteMap[2];
Save = BrushRegion;
for (y = RegionYMin; y < RegionYMax; y++)
	{
	if ((y >= 0) && (y < ElevRows))
		{
		Zip = y * ElevCols + RegionXMin;
		for (x = RegionXMin; x < RegionXMax; x++, Zip++)
			{
			if ((x >= 0) && (x < ElevCols))
				{
				Red[Zip] = *Save++;
				Green[Zip] = *Save++;
				Blue[Zip] = *Save++;
				} // if x in raster
			} // for x
		} // if y in raster
	} // for y

SavedRegion = 0;

} // DEMPaintGUI::RestoreBrushRegion

/*===========================================================================*/

void DEMPaintGUI::SaveBrushRegion(long X, long Y)
{
double tmpx, tmpy;
long brushy, x, y, Zip, ZipB;
unsigned char *Red, *Green, *Blue, *Save;

SavedRegion = 1;

// get cursor position relative to widget
tmpx = (double)X / WidSize[0][0];
tmpy = (double)Y / WidSize[0][1];
if (tmpx < 0.0)
	tmpx = 0.0;
if (tmpx > 1.0)
	tmpx = 1.0;
if (tmpy < 0.0)
	tmpy = 0.0;
if (tmpy > 1.0)
	tmpy = 1.0;

// now translate widget x/y to visible area
tmpx = Elevs.OffsetX + tmpx * Elevs.Visible;
tmpy = Elevs.OffsetY + tmpy * Elevs.Visible;

// compute the copy region
RegionXMin = (long)(tmpx * ElevCols - BrushWidth * 0.5);
RegionXMax = (long)(tmpx * ElevCols + BrushWidth * 0.5);
RegionYMin = (long)(tmpy * ElevRows - BrushHeight * 0.5);
RegionYMax = (long)(tmpy * ElevRows + BrushHeight * 0.5);

Red = Elevs.OverRast->ByteMap[0];
Green = Elevs.OverRast->ByteMap[1];
Blue = Elevs.OverRast->ByteMap[2];
Save = BrushRegion;
for (brushy = 0, y = RegionYMin; y < RegionYMax; brushy++, y++)
	{
	if ((y >= 0) && (y < ElevRows))
		{
		Zip = y * ElevCols + RegionXMin;
		ZipB = brushy * BrushWidth;
		for (x = RegionXMin; x < RegionXMax; x++, Zip++, ZipB++)
			{
			if ((x >= 0) && (x < ElevCols))
				{
				*Save++ = Red[Zip];
				*Save++ = Green[Zip];
				*Save++ = Blue[Zip];
				// draw the brush outline in magenta
				Red[Zip] = BrushOutline[ZipB];
				Blue[Zip] = BrushOutline[ZipB];
				} // if x in raster
			} // for x
		} // if y in raster
	} // for y

} // DEMPaintGUI::SaveBrushRegion

/*===========================================================================*/
