// VecProfExportGUI.cpp
// Code for Vector Profile Export window
// Built from scratch 10/31/02 Gary Huber
// Copyright 2002 Questar Productions. all rights reserved.

#include "stdafx.h"
#include "VecProfExportGUI.h"
#include "WCSVersion.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Illustrator.h"
#include "Requester.h"
#include "AppHelp.h"
#include "resource.h"

static char fontStr[320];

NativeGUIWin VecProfExportGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // VecProfExportGUI::Open

/*===========================================================================*/

NativeGUIWin VecProfExportGUI::Construct(void)
{
char *DistanceUnits[] = {"Miles", "Kilometers", "Meters", "Feet", "Inches", "Centimeters"};
char *ElevationUnits[] = {"Miles", "Kilometers", "Meters", "Feet", "Inches", "Centimeters"};
char *LayoutUnits[] = {"Inches", "Centimeters"};
char *VectorStyles[] = {"Solid", "Dashed", "Dotted", "Broken"};
int ListEntry;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_VECPROF_EXPORT, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		for (ListEntry = 0; ListEntry < 6; ListEntry ++)
			WidgetCBInsert(IDC_HORGRIDUNITSDROP, -1, DistanceUnits[ListEntry]);
		WidgetCBSetCurSel(IDC_HORGRIDUNITSDROP, ExportData->HorUnits);

		for (ListEntry = 0; ListEntry < 6; ListEntry ++)
			WidgetCBInsert(IDC_VERTGRIDUNITSDROP, -1, ElevationUnits[ListEntry]);
		WidgetCBSetCurSel(IDC_VERTGRIDUNITSDROP, ExportData->VertUnits);

		for (ListEntry = 0; ListEntry < 2; ListEntry ++)
			WidgetCBInsert(IDC_LAYOUTUNITSDROP, -1, LayoutUnits[ListEntry]);
		WidgetCBSetCurSel(IDC_LAYOUTUNITSDROP, ExportData->LayoutUnits);

		for (ListEntry = 0; ListEntry < 4; ListEntry ++)
			WidgetCBInsert(IDC_VECTORSTYLEDROP, -1, VectorStyles[ListEntry]);
		WidgetCBSetCurSel(IDC_VECTORSTYLEDROP, ExportData->VectorLineStyle);

		for (ListEntry = 0; ListEntry < 4; ListEntry ++)
			WidgetCBInsert(IDC_TERRAINSTYLEDROP, -1, VectorStyles[ListEntry]);
		WidgetCBSetCurSel(IDC_TERRAINSTYLEDROP, ExportData->TerrainLineStyle);

		for (ListEntry = 0; ListEntry < 4; ListEntry ++)
			WidgetCBInsert(IDC_HORGRIDSTYLEDROP, -1, VectorStyles[ListEntry]);
		WidgetCBSetCurSel(IDC_HORGRIDSTYLEDROP, ExportData->HorGridStyle);

		for (ListEntry = 0; ListEntry < 4; ListEntry ++)
			WidgetCBInsert(IDC_VERTGRIDSTYLEDROP, -1, VectorStyles[ListEntry]);
		WidgetCBSetCurSel(IDC_VERTGRIDSTYLEDROP, ExportData->VertGridStyle);

		for (ListEntry = 0; ListEntry < 4; ListEntry ++)
			WidgetCBInsert(IDC_GRAPHBOXSTYLEDROP, -1, VectorStyles[ListEntry]);
		WidgetCBSetCurSel(IDC_GRAPHBOXSTYLEDROP, ExportData->GraphOutlineStyle);


		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);

} // VecProfExportGUI::Construct

/*===========================================================================*/

VecProfExportGUI::VecProfExportGUI(Project *ProjSource, Joe *ActiveSource)
: GUIFenetre('VPEX', this, "Vector Profile Export") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED), 
								MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED), 
								0};
char NameStr[512];
char Ct;

ConstructError = 0;
Active = ActiveSource;
ExportData = NULL;
VecLen = 1.0;
VecMaxEl = VecMinEl = 0.0f;
ElevMinEl = ElevMaxEl = 0.0;
fHandle = NULL;

if (ProjSource && ActiveSource)
	{
	sprintf(NameStr, "%s (%s)", "Vector Profile Export", ActiveSource->GetBestName());
	SetTitle(NameStr);
	ExportData = &ProjSource->Prefs.VecExpData;
	if (! ExportData->PAF.Path[0])
		ExportData->PAF.SetPath(ProjSource->dirname);
	sprintf(NameStr, "%s", ActiveSource->GetBestName());
	strcat(NameStr, ".ai");
	ExportData->PAF.SetName(NameStr);
	CalcVecLenHeight();
	CalcScaleFromSize();
	for (Ct = 0; Ct < WCS_VECPROF_EXPORT_NUMCOLORS; Ct ++)
		{
		ColorStandIns[Ct].SetDefaults(NULL, Ct);
		ColorStandIns[Ct].SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		} // for
	ColorStandIns[0].SetValue3(ExportData->VectorColor[0] / 255.0, ExportData->VectorColor[1] / 255.0, ExportData->VectorColor[2] / 255.0);
	ColorStandIns[1].SetValue3(ExportData->TerrainColor[0] / 255.0, ExportData->TerrainColor[1] / 255.0, ExportData->TerrainColor[2] / 255.0);
	ColorStandIns[2].SetValue3(ExportData->HorGridColor[0] / 255.0, ExportData->HorGridColor[1] / 255.0, ExportData->HorGridColor[2] / 255.0);
	ColorStandIns[3].SetValue3(ExportData->VertGridColor[0] / 255.0, ExportData->VertGridColor[1] / 255.0, ExportData->VertGridColor[2] / 255.0);
	ColorStandIns[4].SetValue3(ExportData->HorTicColor[0] / 255.0, ExportData->HorTicColor[1] / 255.0, ExportData->HorTicColor[2] / 255.0);
	ColorStandIns[5].SetValue3(ExportData->VertTicColor[0] / 255.0, ExportData->VertTicColor[1] / 255.0, ExportData->VertTicColor[2] / 255.0);
	ColorStandIns[6].SetValue3(ExportData->HorLabelColor[0] / 255.0, ExportData->HorLabelColor[1] / 255.0, ExportData->HorLabelColor[2] / 255.0);
	ColorStandIns[7].SetValue3(ExportData->VertLabelColor[0] / 255.0, ExportData->VertLabelColor[1] / 255.0, ExportData->VertLabelColor[2] / 255.0);
	ColorStandIns[8].SetValue3(ExportData->GraphOutlineColor[0] / 255.0, ExportData->GraphOutlineColor[1] / 255.0, ExportData->GraphOutlineColor[2] / 255.0);
	ElevationADD.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	ElevationADD.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // VecProfExportGUI::VecProfExportGUI

/*===========================================================================*/

VecProfExportGUI::~VecProfExportGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // VecProfExportGUI::~VecProfExportGUI()

/*===========================================================================*/

long VecProfExportGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_VPX, 0);

return(0);

} // VecProfExportGUI::HandleCloseWin

/*===========================================================================*/

long VecProfExportGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(032, 32);
switch(ButtonID)
	{
	case IDC_EXPORT:
		{
		ExportProfile();
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VPX, 0);
		break;
		} // 
	case ID_COLORPOT1:
		{
		ColorStandIns[0].EditRAHost();
		break;
		} // 
	case ID_COLORPOT2:
		{
		ColorStandIns[1].EditRAHost();
		break;
		} // 
	case ID_COLORPOT3:
		{
		ColorStandIns[2].EditRAHost();
		break;
		} // 
	case ID_COLORPOT4:
		{
		ColorStandIns[3].EditRAHost();
		break;
		} // 
	case ID_COLORPOT5:
		{
		ColorStandIns[4].EditRAHost();
		break;
		} // 
	case ID_COLORPOT6:
		{
		ColorStandIns[5].EditRAHost();
		break;
		} // 
	case ID_COLORPOT7:
		{
		ColorStandIns[6].EditRAHost();
		break;
		} // 
	case ID_COLORPOT8:
		{
		ColorStandIns[7].EditRAHost();
		break;
		} // 
	case ID_COLORPOT9:
		{
		ColorStandIns[8].EditRAHost();
		break;
		} // 
	case IDC_CHOOSEFONT:
		{
		GetInputString("Enter a name for the new object", "", ExportData->PreferredFont);
		break;
		} // IDC_CHOOSEFONT
	default:
		break;
	} // ButtonID

return(0);

} // VecProfExportGUI::HandleButtonClick

/*===========================================================================*/

long VecProfExportGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_HORGRIDUNITSDROP:
		{
		ExportData->HorUnits = (char)WidgetCBGetCurSel(IDC_HORGRIDUNITSDROP);
		SetUnitsTexts();
		CalcScaleFromSize();
		break;
		} // date
	case IDC_VERTGRIDUNITSDROP:
		{
		ExportData->VertUnits = (char)WidgetCBGetCurSel(IDC_VERTGRIDUNITSDROP);
		SetUnitsTexts();
		CalcScaleFromSize();
		break;
		} // date
	case IDC_LAYOUTUNITSDROP:
		{
		ExportData->LayoutUnits = (char)WidgetCBGetCurSel(IDC_LAYOUTUNITSDROP);
		SetUnitsTexts();
		break;
		} // date
	case IDC_VECTORSTYLEDROP:
		{
		ExportData->VectorLineStyle = (char)WidgetCBGetCurSel(IDC_VECTORSTYLEDROP);
		break;
		} // date
	case IDC_TERRAINGRIDSTYLEDROP:
		{
		ExportData->TerrainLineStyle = (char)WidgetCBGetCurSel(IDC_TERRAINGRIDSTYLEDROP);
		break;
		} // date
	case IDC_HORGRIDSTYLEDROP:
		{
		ExportData->HorGridStyle = (char)WidgetCBGetCurSel(IDC_HORGRIDSTYLEDROP);
		break;
		} // date
	case IDC_VERTGRIDSTYLEDROP:
		{
		ExportData->VertGridStyle = (char)WidgetCBGetCurSel(IDC_VERTGRIDSTYLEDROP);
		break;
		} // date
	case IDC_GRAPHBOXSTYLEDROP:
		{
		ExportData->GraphOutlineStyle = (char)WidgetCBGetCurSel(IDC_GRAPHBOXSTYLEDROP);
		break;
		} // date
	default:
		break;
	} // switch CtrlID

return (0);

} // VecProfExportGUI::HandleCBChange

/*===========================================================================*/

long VecProfExportGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKVECTOR:
	case IDC_CHECKTERRAIN:
		{
		CalcScaleFromSize();
		break;
		} // IDC_CHECKTERRAIN
	default:
		break;
	} // switch CtrlID


return(0);

} // VecProfExportGUI::HandleSCChange

/*===========================================================================*/

long VecProfExportGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_HORSCALE:
		{
		CalcSizeFromScale();
		break;
		} // IDC_HORSCALE
	case IDC_VERTSCALE:
		{
		CalcSizeFromScale();
		break;
		} // IDC_VERTSCALE
	case IDC_HORSIZE:
		{
		CalcScaleFromSize();
		break;
		} // IDC_HORSIZE
	case IDC_VERTSIZE:
		{
		CalcScaleFromSize();
		break;
		} // IDC_VERTSIZE
	default:
		break;
	} // switch CtrlID

return(0);

} // VecProfExportGUI::HandleFIChange

/*===========================================================================*/

void VecProfExportGUI::HandleNotifyEvent(void)
{

FetchColors();
ConfigureWidgets();

} // VecProfExportGUI::HandleNotifyEvent()

/*===========================================================================*/

void VecProfExportGUI::ConfigureWidgets(void)
{

ConfigureSC(NativeWin, IDC_CHECKHORSCALELABEL, &ExportData->HorScaleLabel, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTSCALELABEL, &ExportData->VertScaleLabel, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVECTOR, &ExportData->DrawVector, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTERRAIN, &ExportData->DrawTerrain, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKHORGRID, &ExportData->DrawHorGrid, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTGRID, &ExportData->DrawVertGrid, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKHORTIC, &ExportData->DrawHorTics, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTTIC, &ExportData->DrawVertTics, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKHORLABEL, &ExportData->DrawHorLabels, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTLABEL, &ExportData->DrawVertLabels, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRAPHBOX, &ExportData->DrawGraphOutline, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKLAUNCH, &ExportData->LaunchIllustrator, SCFlag_Char, NULL, 0);

ConfigureFI(NativeWin, IDC_HORSCALE,
 &ExportData->HorScale,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORSIZE,
 &ExportData->HorSize,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTSCALE,
 &ExportData->VertScale,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTSIZE,
 &ExportData->VertSize,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VECTORWT,
 &ExportData->VectorLineWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_TERRAINWT,
 &ExportData->TerrainLineWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORGRIDWT,
 &ExportData->HorGridWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTGRIDWT,
 &ExportData->VertGridWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORTICWT,
 &ExportData->HorTicWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTTICWT,
 &ExportData->VertTicWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_GRAPHBOXWT,
 &ExportData->GraphOutlineWeight,
  .5,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORGRIDINTERVAL,
 &ExportData->HorGridInterval,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTGRIDINTERVAL,
 &ExportData->VertGridInterval,
  5.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORTICINTERVAL,
 &ExportData->HorTicInterval,
  1.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_VERTTICINTERVAL,
 &ExportData->VertTicInterval,
  5.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_HORLABELINTERVAL,
 &ExportData->HorLabelInterval,
  5.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_VERTLABELINTERVAL,
 &ExportData->VertLabelInterval,
  5.0,
   .000001,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_HORTICLEN,
 &ExportData->HorTicLength,
  1.0,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_VERTTICLEN,
 &ExportData->VertTicLength,
  1.0,
   0.0,
	100.0,
	 FIOFlag_Double,
	  NULL,
	   0);

SetColorPot(0, ExportData->VectorColor[0], ExportData->VectorColor[1], ExportData->VectorColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);	// 100 percentage filled, ..., color pot

SetColorPot(1, ExportData->TerrainColor[0], ExportData->TerrainColor[1], ExportData->TerrainColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);	// 100 percentage filled, ..., color pot

SetColorPot(2, ExportData->HorGridColor[0], ExportData->HorGridColor[1], ExportData->HorGridColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT3, 100, CBFlag_CustomColor, 2);	// 100 percentage filled, ..., color pot

SetColorPot(3, ExportData->VertGridColor[0], ExportData->VertGridColor[1], ExportData->VertGridColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT4, 100, CBFlag_CustomColor, 3);	// 100 percentage filled, ..., color pot

SetColorPot(4, ExportData->HorTicColor[0], ExportData->HorTicColor[1], ExportData->HorTicColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT5, 100, CBFlag_CustomColor, 4);	// 100 percentage filled, ..., color pot

SetColorPot(5, ExportData->VertTicColor[0], ExportData->VertTicColor[1], ExportData->VertTicColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT6, 100, CBFlag_CustomColor, 5);	// 100 percentage filled, ..., color pot

SetColorPot(6, ExportData->HorLabelColor[0], ExportData->HorLabelColor[1], ExportData->HorLabelColor[2], 0);
ConfigureCB(NativeWin, ID_COLORPOT7, 100, CBFlag_CustomColor, 6);	// 100 percentage filled, ..., color pot

SetColorPot(7, ExportData->VertLabelColor[0], ExportData->VertLabelColor[1], ExportData->VertLabelColor[2], 1);
ConfigureCB(NativeWin, ID_COLORPOT8, 100, CBFlag_CustomColor, 7);	// 100 percentage filled, ..., color pot

SetColorPot(8, ExportData->GraphOutlineColor[0], ExportData->GraphOutlineColor[1], ExportData->GraphOutlineColor[2], 1);
ConfigureCB(NativeWin, ID_COLORPOT9, 100, CBFlag_CustomColor, 8);	// 100 percentage filled, ..., color pot

ConfigureDD(NativeWin, IDC_OUTPUT, (char *)ExportData->PAF.GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)ExportData->PAF.GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_PROJ);

SetUnitsTexts();

} // VecProfExportGUI::ConfigureWidgets()

/*===========================================================================*/

void VecProfExportGUI::FetchColors(void)
{

ExportData->VectorColor[0] = (unsigned char)(ColorStandIns[0].CurValue[0] * 255);
ExportData->VectorColor[1] = (unsigned char)(ColorStandIns[0].CurValue[1] * 255);
ExportData->VectorColor[2] = (unsigned char)(ColorStandIns[0].CurValue[2] * 255);

ExportData->TerrainColor[0] = (unsigned char)(ColorStandIns[1].CurValue[0] * 255);
ExportData->TerrainColor[1] = (unsigned char)(ColorStandIns[1].CurValue[1] * 255);
ExportData->TerrainColor[2] = (unsigned char)(ColorStandIns[1].CurValue[2] * 255);

ExportData->HorGridColor[0] = (unsigned char)(ColorStandIns[2].CurValue[0] * 255);
ExportData->HorGridColor[1] = (unsigned char)(ColorStandIns[2].CurValue[1] * 255);
ExportData->HorGridColor[2] = (unsigned char)(ColorStandIns[2].CurValue[2] * 255);

ExportData->VertGridColor[0] = (unsigned char)(ColorStandIns[3].CurValue[0] * 255);
ExportData->VertGridColor[1] = (unsigned char)(ColorStandIns[3].CurValue[1] * 255);
ExportData->VertGridColor[2] = (unsigned char)(ColorStandIns[3].CurValue[2] * 255);

ExportData->HorTicColor[0] = (unsigned char)(ColorStandIns[4].CurValue[0] * 255);
ExportData->HorTicColor[1] = (unsigned char)(ColorStandIns[4].CurValue[1] * 255);
ExportData->HorTicColor[2] = (unsigned char)(ColorStandIns[4].CurValue[2] * 255);

ExportData->VertTicColor[0] = (unsigned char)(ColorStandIns[5].CurValue[0] * 255);
ExportData->VertTicColor[1] = (unsigned char)(ColorStandIns[5].CurValue[1] * 255);
ExportData->VertTicColor[2] = (unsigned char)(ColorStandIns[5].CurValue[2] * 255);

ExportData->HorLabelColor[0] = (unsigned char)(ColorStandIns[6].CurValue[0] * 255);
ExportData->HorLabelColor[1] = (unsigned char)(ColorStandIns[6].CurValue[1] * 255);
ExportData->HorLabelColor[2] = (unsigned char)(ColorStandIns[6].CurValue[2] * 255);

ExportData->VertLabelColor[0] = (unsigned char)(ColorStandIns[7].CurValue[0] * 255);
ExportData->VertLabelColor[1] = (unsigned char)(ColorStandIns[7].CurValue[1] * 255);
ExportData->VertLabelColor[2] = (unsigned char)(ColorStandIns[7].CurValue[2] * 255);

ExportData->GraphOutlineColor[0] = (unsigned char)(ColorStandIns[8].CurValue[0] * 255);
ExportData->GraphOutlineColor[1] = (unsigned char)(ColorStandIns[8].CurValue[1] * 255);
ExportData->GraphOutlineColor[2] = (unsigned char)(ColorStandIns[8].CurValue[2] * 255);

} // VecProfExportGUI::FetchColors

/*===========================================================================*/

void VecProfExportGUI::CalcVecLenHeight(void)
{

VecLen = Active->GetVecLength(0);
Active->GetElevRange(VecMaxEl, VecMinEl);
BuildElevProfile();
ElevationADD.GetMinMaxVal(ElevMinEl, ElevMaxEl);

} // VecProfExportGUI::CalcVecLenHeight

/*===========================================================================*/

void VecProfExportGUI::BuildElevProfile(void)
{
double SumDistance, TerrainElevation = 0.0, PlanetRad;
VertexDEM CurVert, NextVert;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VectorPoint *PLink;
GraphNode *CurNode = NULL, *ElevNode = NULL;
int ElevNULL = 0, ElevSkip = 0;

ElevationADD.ReleaseNodes();

if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();

if (Active && (PLink = Active->GetFirstRealPoint()))
	{
	SumDistance = 0.0;
	if (PLink->ProjToDefDeg(MyCoords, &CurVert))
		{
		if (! ElevSkip)
			{
			TerrainElevation = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(CurVert.Lat, CurVert.Lon, ElevNULL);
			if (! ElevNULL)
				{
				if (! (ElevNode = ElevationADD.AddNodeEnd(ElevNode, SumDistance, TerrainElevation, 0.0)))
					return;
				ElevNode->SetLinear(1);
				} // if elevation OK
			else
				{
				ElevSkip = 10;
				} // else
			} // if
		else
			{
			ElevSkip --;
			} // else
		PLink = PLink->Next;
		while (PLink)
			{
			if (PLink->ProjToDefDeg(MyCoords, &NextVert))
				{
				SumDistance += FindDistance(CurVert.Lat, CurVert.Lon, NextVert.Lat, NextVert.Lon, PlanetRad);
				if (! ElevSkip)
					{
					TerrainElevation = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(NextVert.Lat, NextVert.Lon, ElevNULL);
					if (! ElevNULL)
						{
						if (! (ElevNode = ElevationADD.AddNodeEnd(ElevNode, SumDistance, TerrainElevation, 0.0)))
							return;
						ElevNode->SetLinear(1);
						} // if elevation OK
					else
						{
						ElevSkip = 10;
						} // else
					} // if
				else
					{
					ElevSkip --;
					} // else
				CurVert.CopyLatLon(&NextVert);
				} // if
			else
				return;
			PLink = PLink->Next;
			} // while
		} // if
	} // if

} // VecProfExportGUI::BuildElevProfile

/*===========================================================================*/

void VecProfExportGUI::SetUnitsTexts(void)
{
char DistScale[32], ElevScale[32], DistSize[32], ElevSize[32];

strcpy(DistScale, "Scale ");
strcpy(ElevScale, "Scale ");
strcpy(DistSize, "Width ");
strcpy(ElevSize, "Height ");

switch (ExportData->HorUnits)
	{
	case 0:
		{
		strcat(DistScale, "(mi/");
		break;
		} // Miles
	case 1:
		{
		strcat(DistScale, "(km/");
		break;
		} // Kilometers
	case 2:
		{
		strcat(DistScale, "(m/");
		break;
		} // Meters
	case 3:
		{
		strcat(DistScale, "(ft/");
		break;
		} // Feet
	case 4:
		{
		strcat(DistScale, "(in/");
		break;
		} // Inches
	case 5:
		{
		strcat(DistScale, "(cm/");
		break;
		} // Centimeters
	} // if
switch (ExportData->VertUnits)
	{
	case 0:
		{
		strcat(ElevScale, "(mi/");
		break;
		} // Miles
	case 1:
		{
		strcat(ElevScale, "(km/");
		break;
		} // Kilometers
	case 2:
		{
		strcat(ElevScale, "(m/");
		break;
		} // Meters
	case 3:
		{
		strcat(ElevScale, "(ft/");
		break;
		} // Feet
	case 4:
		{
		strcat(ElevScale, "(in/");
		break;
		} // Inches
	case 5:
		{
		strcat(ElevScale, "(cm/");
		break;
		} // Centimeters
	} // if
switch (ExportData->LayoutUnits)
	{
	case 0:
		{
		strcat(DistScale, "in) ");
		strcat(ElevScale, "in) ");
		strcat(DistSize, "(in) ");
		strcat(ElevSize, "(in) ");
		break;
		} // Inches
	case 1:
		{
		strcat(DistScale, "cm) ");
		strcat(ElevScale, "cm) ");
		strcat(DistSize, "(cm) ");
		strcat(ElevSize, "(cm) ");
		break;
		} // Centimeters
	} // if

WidgetSetText(IDC_HORSCALE, DistScale);
WidgetSetText(IDC_VERTSCALE, ElevScale);
WidgetSetText(IDC_HORSIZE, DistSize);
WidgetSetText(IDC_VERTSIZE, ElevSize);

} // VecProfExportGUI::SetUnitsTexts

/*===========================================================================*/

void VecProfExportGUI::CalcSizeFromScale(void)
{
double VecLenLocal, VecRangeLocal;
int ToUnit;

if (ExportData->HorUnits == 0)
	ToUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
else if (ExportData->HorUnits == 1)
	ToUnit = WCS_USEFUL_UNIT_KILOMETER;
else if (ExportData->HorUnits == 2)
	ToUnit = WCS_USEFUL_UNIT_METER;
else if (ExportData->HorUnits == 3)
	ToUnit = WCS_USEFUL_UNIT_FEET;
else if (ExportData->HorUnits == 4)
	ToUnit = WCS_USEFUL_UNIT_INCH;
else if (ExportData->HorUnits == 5)
	ToUnit = WCS_USEFUL_UNIT_CENTIMETER;

VecLenLocal = ConvertFromMeters(VecLen, ToUnit);

if (ExportData->VertUnits == 0)
	ToUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
else if (ExportData->VertUnits == 1)
	ToUnit = WCS_USEFUL_UNIT_KILOMETER;
else if (ExportData->VertUnits == 2)
	ToUnit = WCS_USEFUL_UNIT_METER;
else if (ExportData->VertUnits == 3)
	ToUnit = WCS_USEFUL_UNIT_FEET;
else if (ExportData->VertUnits == 4)
	ToUnit = WCS_USEFUL_UNIT_INCH;
else if (ExportData->VertUnits == 5)
	ToUnit = WCS_USEFUL_UNIT_CENTIMETER;

if (ExportData->DrawVector && ExportData->DrawTerrain)
	VecRangeLocal = ConvertFromMeters(max(VecMaxEl, ElevMaxEl) - min(VecMinEl, ElevMinEl), ToUnit);
else if (ExportData->DrawTerrain)
	VecRangeLocal = ConvertFromMeters(ElevMaxEl - ElevMinEl, ToUnit);
else
	VecRangeLocal = ConvertFromMeters((double)VecMaxEl - VecMinEl, ToUnit);

ExportData->HorSize = VecLenLocal / ExportData->HorScale;
if (ExportData->HorSize < 0.000001)
	ExportData->HorSize = 0.000001;
ExportData->VertSize = VecRangeLocal / ExportData->VertScale;
if (ExportData->VertSize < 0.000001)
	ExportData->VertSize = 0.000001;
WidgetFISync(IDC_HORSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_VERTSIZE, WP_FISYNC_NONOTIFY);

} // VecProfExportGUI::CalcSizeFromScale

/*===========================================================================*/

void VecProfExportGUI::CalcScaleFromSize(void)
{
double VecLenLocal, VecRangeLocal;
int ToUnit;

if (ExportData->HorUnits == 0)
	ToUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
else if (ExportData->HorUnits == 1)
	ToUnit = WCS_USEFUL_UNIT_KILOMETER;
else if (ExportData->HorUnits == 2)
	ToUnit = WCS_USEFUL_UNIT_METER;
else if (ExportData->HorUnits == 3)
	ToUnit = WCS_USEFUL_UNIT_FEET;
else if (ExportData->HorUnits == 4)
	ToUnit = WCS_USEFUL_UNIT_INCH;
else if (ExportData->HorUnits == 5)
	ToUnit = WCS_USEFUL_UNIT_CENTIMETER;

VecLenLocal = ConvertFromMeters(VecLen, ToUnit);

if (ExportData->VertUnits == 0)
	ToUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
else if (ExportData->VertUnits == 1)
	ToUnit = WCS_USEFUL_UNIT_KILOMETER;
else if (ExportData->VertUnits == 2)
	ToUnit = WCS_USEFUL_UNIT_METER;
else if (ExportData->VertUnits == 3)
	ToUnit = WCS_USEFUL_UNIT_FEET;
else if (ExportData->VertUnits == 4)
	ToUnit = WCS_USEFUL_UNIT_INCH;
else if (ExportData->VertUnits == 5)
	ToUnit = WCS_USEFUL_UNIT_CENTIMETER;

if (ExportData->DrawVector && ExportData->DrawTerrain)
	VecRangeLocal = ConvertFromMeters(max(VecMaxEl, ElevMaxEl) - min(VecMinEl, ElevMinEl), ToUnit);
else if (ExportData->DrawTerrain)
	VecRangeLocal = ConvertFromMeters(ElevMaxEl - ElevMinEl, ToUnit);
else
	VecRangeLocal = ConvertFromMeters((double)VecMaxEl - VecMinEl, ToUnit);

ExportData->HorScale = VecLenLocal / ExportData->HorSize;
if (ExportData->HorScale < 0.000001)
	ExportData->HorScale = 0.000001;
ExportData->VertScale = VecRangeLocal / ExportData->VertSize;
if (ExportData->VertScale < 0.000001)
	ExportData->VertScale = 0.000001;
WidgetFISync(IDC_HORSCALE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_VERTSCALE, WP_FISYNC_NONOTIFY);

} // VecProfExportGUI::CalcScaleFromSize

/*===========================================================================*/

#define WCS_STANDARD_ENDCAP		0
#define WCS_ROUND_ENDCAP		1

// order = solid, dashed, dotted, broken
int EndCapValues[4] = {WCS_STANDARD_ENDCAP, WCS_STANDARD_ENDCAP, WCS_ROUND_ENDCAP, WCS_STANDARD_ENDCAP};
char *LinePatternValues[4] = {"[]0", "[3 2]0", "[0 2]0", "[9 2 2 2]0"};

void VecProfExportGUI::ExportProfile(void)
{
#ifndef WCS_BUILD_DEMO
double SumDist = 0.0, HDist, Radius, DPIfac, VecRangeLocal, LastPtLat, LastPtLon, FirstGrid, GridInt, xpos, ypos, 
	LeftGraphOffset, BottomGraphOffset, AbsMinEl, AbsMaxEl, FirstGridInUserUnits;
long screenwidth, screenheight, err = 0, FromUnit, LineCap, IntLabels, PrintStartBlock, LabelPlotted;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VectorPoint *PLinkA, *PLinkB;
GraphNode *CurNode;
VertexDEM MyVert;
struct AIRGBcolor rgb;
struct AICMYKcolor cmyk;
char DashPattern[32], *HVUnits, *LayoutUnits;

// create an image bounds large enough to contain the graph plus any margin stuff like labels and tics
LeftGraphOffset = BottomGraphOffset = WCS_ILLUSTRATOR_APPROX_RASTER_DPI;	// one inch border around graph
DPIfac = WCS_ILLUSTRATOR_APPROX_RASTER_DPI;
if (ExportData->LayoutUnits == 1)	// centimeters: convert to inches
	DPIfac /= 2.54;
screenwidth = (long)(2 * LeftGraphOffset + ExportData->HorSize * DPIfac);
screenheight = (long)(2 * BottomGraphOffset + ExportData->VertSize * DPIfac);
if (ExportData->DrawVector && ExportData->DrawTerrain)
	{
	VecRangeLocal = max(VecMaxEl, ElevMaxEl) - min(VecMinEl, ElevMinEl);
	AbsMinEl = min(VecMinEl, ElevMinEl);
	AbsMaxEl = max(VecMaxEl, ElevMaxEl);
	} // if
else if (ExportData->DrawTerrain)
	{
	VecRangeLocal = ElevMaxEl - ElevMinEl;
	AbsMinEl = ElevMinEl;
	AbsMaxEl = ElevMaxEl;
	} // else if
else
	{
	VecRangeLocal = VecMaxEl - VecMinEl;
	AbsMinEl = VecMinEl;
	AbsMaxEl = VecMaxEl;
	} // else

// IllustratorInit returns 0 for no error
if (! IllustratorInit(WCS_ILLUSTRATOR_DEFAULT_DPI, WCS_ILLUSTRATOR_APPROX_RASTER_DPI, screenwidth, screenheight))
	{
	// box around graph
	if (ExportData->DrawGraphOutline)
		{
		rgb.r = ExportData->GraphOutlineColor[0];
		rgb.g = ExportData->GraphOutlineColor[1];
		rgb.b = ExportData->GraphOutlineColor[2];
		LineCap = EndCapValues[ExportData->GraphOutlineStyle];
		if (ExportData->GraphOutlineStyle == 2)	// special space treatment for dotted pattern
			sprintf(DashPattern, "[0 %.4lf]0", ExportData->GraphOutlineWeight + 2);
		else
			strcpy(DashPattern, LinePatternValues[ExportData->GraphOutlineStyle]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->GraphOutlineWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Graph Outline");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		fprintf(fHandle, "%.4lf %.4lf m\n", LeftGraphOffset, BottomGraphOffset);
		fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset, BottomGraphOffset + DPIfac * ExportData->VertSize);
		fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset + DPIfac * ExportData->HorSize, BottomGraphOffset + DPIfac * ExportData->VertSize);
		fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset + DPIfac * ExportData->HorSize, BottomGraphOffset);
		fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset, BottomGraphOffset);
		if (err = (fprintf(fHandle, "S\n") <= 0))
			goto EndPlot;
		} // if

	// elevation grid
	if (ExportData->DrawVertGrid && VecRangeLocal > 0.0)
		{
		// find grid interval in meters
		if (ExportData->VertUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->VertUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->VertUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->VertUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->VertUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->VertUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->VertGridInterval, FromUnit);
		// find first grid line elevation > AbsMinEl
		FirstGrid = WCS_floor(AbsMinEl / GridInt) * GridInt + GridInt;
		// draw grid lines until elevation >= AbsMaxEl
		rgb.r = ExportData->VertGridColor[0];
		rgb.g = ExportData->VertGridColor[1];
		rgb.b = ExportData->VertGridColor[2];
		LineCap = EndCapValues[ExportData->VertGridStyle];
		if (ExportData->VertGridStyle == 2)	// special space treatment for dotted pattern
			sprintf(DashPattern, "[0 %.4lf]0", ExportData->VertGridWeight + 2);
		else
			strcpy(DashPattern, LinePatternValues[ExportData->VertGridStyle]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->VertGridWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Elevation Grid");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		while (FirstGrid < AbsMaxEl)
			{
			ypos = BottomGraphOffset + ((FirstGrid - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
			fprintf(fHandle, "%.4lf %.4lf m\n", LeftGraphOffset, ypos);
			fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset + DPIfac * ExportData->HorSize, ypos);
			if (err = (fprintf(fHandle, "S\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			} // while
		} // elevation grid

	// distance grid
	if (ExportData->DrawHorGrid && VecLen > 0.0)
		{
		// find grid interval in meters
		if (ExportData->HorUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->HorUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->HorUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->HorUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->HorUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->HorUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->HorGridInterval, FromUnit);
		FirstGrid = GridInt;
		// draw grid lines until distance >= VecLen
		rgb.r = ExportData->HorGridColor[0];
		rgb.g = ExportData->HorGridColor[1];
		rgb.b = ExportData->HorGridColor[2];
		LineCap = EndCapValues[ExportData->HorGridStyle];
		if (ExportData->HorGridStyle == 2)	// special space treatment for dotted pattern
			sprintf(DashPattern, "[0 %.4lf]0", ExportData->HorGridWeight + 2);
		else
			strcpy(DashPattern, LinePatternValues[ExportData->HorGridStyle]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->HorGridWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Distance Grid");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		while (FirstGrid < VecLen)
			{
			xpos = LeftGraphOffset + (FirstGrid / VecLen) * ExportData->HorSize * DPIfac;
			fprintf(fHandle, "%.4lf %.4lf m\n", xpos, BottomGraphOffset);
			fprintf(fHandle, "%.4lf %.4lf L\n", xpos, BottomGraphOffset + DPIfac * ExportData->VertSize);
			if (err = (fprintf(fHandle, "S\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			} // while
		} // distance grid

	// elevation tics
	if (ExportData->DrawVertTics && VecRangeLocal > 0.0)
		{
		// find grid interval in meters
		if (ExportData->VertUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->VertUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->VertUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->VertUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->VertUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->VertUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->VertTicInterval, FromUnit);
		// find first grid line elevation > AbsMinEl
		FirstGrid = WCS_floor(AbsMinEl / GridInt) * GridInt;
		if (FirstGrid < AbsMinEl)
			FirstGrid += GridInt;
		// draw grid lines until elevation > AbsMaxEl
		rgb.r = ExportData->VertTicColor[0];
		rgb.g = ExportData->VertTicColor[1];
		rgb.b = ExportData->VertTicColor[2];
		LineCap = EndCapValues[WCS_LINESTYLE_SOLID];
		strcpy(DashPattern, LinePatternValues[WCS_LINESTYLE_SOLID]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->VertTicWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Elevation Tic");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		while (FirstGrid <= AbsMaxEl)
			{
			ypos = BottomGraphOffset + ((FirstGrid - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
			fprintf(fHandle, "%.4lf %.4lf m\n", LeftGraphOffset, ypos);
			fprintf(fHandle, "%.4lf %.4lf L\n", LeftGraphOffset - ExportData->VertTicLength, ypos);
			if (err = (fprintf(fHandle, "S\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			} // while
		} // elevation tics

	// distance tics
	if (ExportData->DrawHorTics && VecLen > 0.0)
		{
		// find grid interval in meters
		if (ExportData->HorUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->HorUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->HorUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->HorUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->HorUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->HorUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->HorTicInterval, FromUnit);
		FirstGrid = 0.0;
		// draw grid lines until distance > VecLen
		rgb.r = ExportData->HorTicColor[0];
		rgb.g = ExportData->HorTicColor[1];
		rgb.b = ExportData->HorTicColor[2];
		LineCap = EndCapValues[WCS_LINESTYLE_SOLID];
		strcpy(DashPattern, LinePatternValues[WCS_LINESTYLE_SOLID]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->HorTicWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Distance Tic");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		while (FirstGrid <= VecLen)
			{
			xpos = LeftGraphOffset + (FirstGrid / VecLen) * ExportData->HorSize * DPIfac;
			fprintf(fHandle, "%.4lf %.4lf m\n", xpos, BottomGraphOffset);
			fprintf(fHandle, "%.4lf %.4lf L\n", xpos, BottomGraphOffset - ExportData->HorTicLength);
			if (err = (fprintf(fHandle, "S\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			} // while
		} // vertical tics

	// terrain
	if (ExportData->DrawTerrain && VecLen > 0.0)
		{
		rgb.r = ExportData->TerrainColor[0];
		rgb.g = ExportData->TerrainColor[1];
		rgb.b = ExportData->TerrainColor[2];
		LineCap = EndCapValues[ExportData->TerrainLineStyle];
		if (ExportData->TerrainLineStyle == 2)	// special space treatment for dotted pattern
			sprintf(DashPattern, "[0 %.4lf]0", ExportData->TerrainLineWeight + 2);
		else
			strcpy(DashPattern, LinePatternValues[ExportData->TerrainLineStyle]);
		// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
		fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->TerrainLineWeight, DashPattern);
		fprintf(fHandle, "%%AI3_Note: %s\n", "Terrain Profile");
		cmyk = ToCMYK(rgb);
		// set stroke color - values are 0..1 with 4 decimal places
		fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

		SumDist = 0.0;
		if (CurNode = ElevationADD.GetFirstNode(0))
			{
			xpos = LeftGraphOffset + (CurNode->GetDistance() / VecLen) * ExportData->HorSize * DPIfac;
			if (VecRangeLocal > 0.0)
				ypos = BottomGraphOffset + ((CurNode->GetValue() - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
			else
				ypos = BottomGraphOffset;
			fprintf(fHandle, "%.4lf %.4lf m\n", xpos, ypos);
			while (CurNode = ElevationADD.GetNextNode(0, CurNode))
				{
				xpos = LeftGraphOffset + (CurNode->GetDistance() / VecLen) * ExportData->HorSize * DPIfac;
				if (VecRangeLocal > 0.0)
					ypos = BottomGraphOffset + ((CurNode->GetValue() - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
				else
					ypos = BottomGraphOffset;
				fprintf(fHandle, "%.4lf %.4lf L\n", xpos, ypos);
				} // for
			if (err = (fprintf(fHandle, "S\n") <= 0))
				goto EndPlot;
			} // if
		} // terrain

	// vector
	if (ExportData->DrawVector && VecLen > 0.0)
		{
		Radius = GlobalApp->AppEffects->GetPlanetRadius();
		if (MyAttr = (JoeCoordSys *)Active->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;
		if (PLinkA = Active->GetFirstRealPoint())
			{
			rgb.r = ExportData->VectorColor[0];
			rgb.g = ExportData->VectorColor[1];
			rgb.b = ExportData->VectorColor[2];
			LineCap = EndCapValues[ExportData->VectorLineStyle];
			if (ExportData->VectorLineStyle == 2)	// special space treatment for dotted pattern
				sprintf(DashPattern, "[0 %.4lf]0", ExportData->VectorLineWeight + 2);
			else
				strcpy(DashPattern, LinePatternValues[ExportData->VectorLineStyle]);
			// set poly attributes (linecap, linejoin, linewidth, miterlimit, dash pattern)
			fprintf(fHandle, "%d J 0 j %.4lf w 4 M %s d\n", LineCap, ExportData->VectorLineWeight, DashPattern);
			fprintf(fHandle, "%%AI3_Note: %s\n", "Vector Profile");
			cmyk = ToCMYK(rgb);
			// set stroke color - values are 0..1 with 4 decimal places
			fprintf(fHandle, "%.4f %.4f %.4f %.4f K\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);

			SumDist = 0.0;
			//PLinkA = Active->GetFirstRealPoint();	// replaced above
			if (PLinkA->ProjToDefDeg(MyCoords, &MyVert))
				{
				xpos = LeftGraphOffset;
				if (VecRangeLocal > 0.0)
					ypos = BottomGraphOffset + ((PLinkA->Elevation - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
				else
					ypos = BottomGraphOffset;
				fprintf(fHandle, "%.4lf %.4lf m\n", xpos, ypos);
				for (PLinkB = PLinkA->Next; PLinkB; PLinkB = PLinkB->Next)
					{
					LastPtLat = MyVert.Lat;
					LastPtLon = MyVert.Lon;
					if (PLinkB->ProjToDefDeg(MyCoords, &MyVert))
						{
						HDist = FindDistance(LastPtLat, LastPtLon, MyVert.Lat, MyVert.Lon, Radius);
						SumDist += HDist;
						xpos = LeftGraphOffset + (SumDist / VecLen) * ExportData->HorSize * DPIfac;
						if (VecRangeLocal > 0.0)
							ypos = BottomGraphOffset + ((PLinkB->Elevation - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
						else
							ypos = BottomGraphOffset;
						fprintf(fHandle, "%.4lf %.4lf L\n", xpos, ypos);
						} // if
					else
						break;
					PLinkA = PLinkB;
					} // for
				if (err = (fprintf(fHandle, "S\n") <= 0))
					goto EndPlot;
				} // if
			} // if
		} // if vector

	// elevation labels
	if (ExportData->DrawVertLabels && VecRangeLocal > 0.0)
		{
		rgb.r = ExportData->VertLabelColor[0];
		rgb.g = ExportData->VertLabelColor[1];
		rgb.b = ExportData->VertLabelColor[2];
		cmyk = ToCMYK(rgb);
		PrintStartBlock = 0;
		fprintf(fHandle, "0 To\n");
		fprintf(fHandle, "0 Tr\n");
		fprintf(fHandle, "0 O\n");
		fprintf(fHandle, "%.4f %.4f %.4f %.4f k\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);
		fprintf(fHandle, "800 Ar\n");
		fprintf(fHandle, "0 J 0 j 1 w 4 M []0 d\n");
		fprintf(fHandle, "%%AI3_Note:\n");
		fprintf(fHandle, "0 D\n");
		fprintf(fHandle, "0 XR\n");
		sprintf(fontStr, "/_%s 12 Tf\n", ExportData->PreferredFont);
		fprintf(fHandle, fontStr);
		fprintf(fHandle, "0 Ts\n");
		fprintf(fHandle, "100 Tz\n");
		fprintf(fHandle, "0 Tt\n");
		fprintf(fHandle, "1 TA\n");
		fprintf(fHandle, "%%_ 0 XL\n");
		fprintf(fHandle, "36 0 Xb\n");
		fprintf(fHandle, "XB\n");
		fprintf(fHandle, "0 0 5 TC\n");
		fprintf(fHandle, "100 100 200 TW\n");
		fprintf(fHandle, "0 0 0 Ti\n");
		fprintf(fHandle, "2 Ta\n");
		fprintf(fHandle, "0 0 2 2 3 Th\n");
		fprintf(fHandle, "0 Tq\n");
		fprintf(fHandle, "0 0 Tl\n");
		fprintf(fHandle, "0 Tc\n");
		fprintf(fHandle, "0 Tw\n");
		// find grid interval in meters
		if (ExportData->VertUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->VertUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->VertUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->VertUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->VertUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->VertUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->VertLabelInterval, FromUnit);
		// find first grid line elevation > AbsMinEl
		FirstGrid = WCS_floor(AbsMinEl / GridInt) * GridInt;
		if (FirstGrid < AbsMinEl)
			FirstGrid += GridInt;
		// draw grid lines until elevation > AbsMaxEl
		IntLabels = (fabs(ExportData->VertLabelInterval - (int)ExportData->VertLabelInterval) < .01);
		LabelPlotted = 0;

		while (FirstGrid <= AbsMaxEl)
			{
			FirstGridInUserUnits = ConvertFromMeters(FirstGrid, FromUnit);
			xpos = LeftGraphOffset - 2 - (ExportData->DrawVertTics ? ExportData->VertTicLength: 0);
			// 4 is 2/3 the font height
			ypos = BottomGraphOffset - 4 + ((FirstGrid - AbsMinEl) / VecRangeLocal) * ExportData->VertSize * DPIfac;
			if (PrintStartBlock)
				fprintf(fHandle, "0 To\n");
			else
				PrintStartBlock = 1;
			fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", xpos, ypos);
			fprintf(fHandle, "0 Tv\n");
			fprintf(fHandle, "TP\n");
			if (IntLabels)
				fprintf(fHandle, "(%d) Tx\n", (long)FirstGridInUserUnits);
			else
				fprintf(fHandle, "(%.1lf) Tx\n", FirstGridInUserUnits);
			if (err = (fprintf(fHandle, "TO\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			LabelPlotted = 1;
			} // while
		if (! LabelPlotted)
			{
			xpos = LeftGraphOffset - 2 - (ExportData->DrawVertTics ? ExportData->VertTicLength: 0);
			ypos = BottomGraphOffset - 4;
			fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", xpos, ypos);
			fprintf(fHandle, "0 Tv\n");
			fprintf(fHandle, "TP\n");
			fprintf(fHandle, "(\\r) TX\n");
			if (err = (fprintf(fHandle, "TO\n") <= 0))
				goto EndPlot;
			} // if
		} // if

	// distance labels
	if (ExportData->DrawHorLabels && VecLen > 0.0)
		{
		rgb.r = ExportData->HorLabelColor[0];
		rgb.g = ExportData->HorLabelColor[1];
		rgb.b = ExportData->HorLabelColor[2];
		cmyk = ToCMYK(rgb);
		PrintStartBlock = 0;
		fprintf(fHandle, "0 To\n");
		fprintf(fHandle, "0 Tr\n");
		fprintf(fHandle, "0 O\n");
		fprintf(fHandle, "%.4f %.4f %.4f %.4f k\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);
		fprintf(fHandle, "800 Ar\n");
		fprintf(fHandle, "0 J 0 j 1 w 4 M []0 d\n");
		fprintf(fHandle, "%%AI3_Note:\n");
		fprintf(fHandle, "0 D\n");
		fprintf(fHandle, "0 XR\n");
		sprintf(fontStr, "/_%s 12 Tf\n", ExportData->PreferredFont);
		fprintf(fHandle, fontStr);
		fprintf(fHandle, "0 Ts\n");
		fprintf(fHandle, "100 Tz\n");
		fprintf(fHandle, "0 Tt\n");
		fprintf(fHandle, "1 TA\n");
		fprintf(fHandle, "%%_ 0 XL\n");
		fprintf(fHandle, "36 0 Xb\n");
		fprintf(fHandle, "XB\n");
		fprintf(fHandle, "0 0 5 TC\n");
		fprintf(fHandle, "100 100 200 TW\n");
		fprintf(fHandle, "0 0 0 Ti\n");
		fprintf(fHandle, "1 Ta\n");
		fprintf(fHandle, "0 0 2 2 3 Th\n");
		fprintf(fHandle, "0 Tq\n");
		fprintf(fHandle, "0 0 Tl\n");
		fprintf(fHandle, "0 Tc\n");
		fprintf(fHandle, "0 Tw\n");
		// find grid interval in meters
		if (ExportData->HorUnits == 0)
			FromUnit = WCS_USEFUL_UNIT_MILE_US_STATUTE;
		else if (ExportData->HorUnits == 1)
			FromUnit = WCS_USEFUL_UNIT_KILOMETER;
		else if (ExportData->HorUnits == 2)
			FromUnit = WCS_USEFUL_UNIT_METER;
		else if (ExportData->HorUnits == 3)
			FromUnit = WCS_USEFUL_UNIT_FEET;
		else if (ExportData->HorUnits == 4)
			FromUnit = WCS_USEFUL_UNIT_INCH;
		else if (ExportData->HorUnits == 5)
			FromUnit = WCS_USEFUL_UNIT_CENTIMETER;

		GridInt = ConvertToMeters(ExportData->HorLabelInterval, FromUnit);
		FirstGrid = 0.0;
		// draw grid lines until distance > VecLen
		IntLabels = (fabs(ExportData->VertLabelInterval - (int)ExportData->VertLabelInterval) < .01);
		LabelPlotted = 0;

		ypos = BottomGraphOffset - 12 - (ExportData->DrawHorTics ? ExportData->HorTicLength: 0);
		while (FirstGrid <= VecLen)
			{
			FirstGridInUserUnits = ConvertFromMeters(FirstGrid, FromUnit);
			// 12 is half the font height
			xpos = LeftGraphOffset + (FirstGrid / VecLen) * ExportData->HorSize * DPIfac;
			if (PrintStartBlock)
				fprintf(fHandle, "0 To\n");
			else
				PrintStartBlock = 1;
			fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", xpos, ypos);
			fprintf(fHandle, "0 Tv\n");
			fprintf(fHandle, "TP\n");
			if (IntLabels)
				fprintf(fHandle, "(%d) Tx\n", (long)FirstGridInUserUnits);
			else
				fprintf(fHandle, "(%.1lf) Tx\n", FirstGridInUserUnits);
			if (err = (fprintf(fHandle, "TO\n") <= 0))
				goto EndPlot;
			FirstGrid += GridInt;
			LabelPlotted = 1;
			} // while
		if (! LabelPlotted)
			{
			xpos = LeftGraphOffset;
			fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", xpos, ypos);
			fprintf(fHandle, "0 Tv\n");
			fprintf(fHandle, "TP\n");
			fprintf(fHandle, "(\\r) TX\n");
			if (err = (fprintf(fHandle, "TO\n") <= 0))
				goto EndPlot;
			} // if
		} // if

	if (ExportData->VertScaleLabel)
		{
		rgb.r = ExportData->VertLabelColor[0];
		rgb.g = ExportData->VertLabelColor[1];
		rgb.b = ExportData->VertLabelColor[2];
		cmyk = ToCMYK(rgb);
		fprintf(fHandle, "0 To\n");
		fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", LeftGraphOffset, BottomGraphOffset - 45);
		fprintf(fHandle, "0 Tv\n");
		fprintf(fHandle, "TP\n");
		fprintf(fHandle, "0 Tr\n");
		fprintf(fHandle, "0 O\n");
		fprintf(fHandle, "%.4f %.4f %.4f %.4f k\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);
		fprintf(fHandle, "800 Ar\n");
		fprintf(fHandle, "0 J 0 j 1 w 4 M []0 d\n");
		fprintf(fHandle, "%%AI3_Note:\n");
		fprintf(fHandle, "0 D\n");
		fprintf(fHandle, "0 XR\n");
		sprintf(fontStr, "/_%s 12 Tf\n", ExportData->PreferredFont);
		fprintf(fHandle, fontStr);
		fprintf(fHandle, "0 Ts\n");
		fprintf(fHandle, "100 Tz\n");
		fprintf(fHandle, "0 Tt\n");
		fprintf(fHandle, "1 TA\n");
		fprintf(fHandle, "%%_ 0 XL\n");
		fprintf(fHandle, "36 0 Xb\n");
		fprintf(fHandle, "XB\n");
		fprintf(fHandle, "0 0 5 TC\n");
		fprintf(fHandle, "100 100 200 TW\n");
		fprintf(fHandle, "0 0 0 Ti\n");
		fprintf(fHandle, "0 Ta\n");
		fprintf(fHandle, "0 0 2 2 3 Th\n");
		fprintf(fHandle, "0 Tq\n");
		fprintf(fHandle, "0 0 Tl\n");
		fprintf(fHandle, "0 Tc\n");
		fprintf(fHandle, "0 Tw\n");
		if (ExportData->VertUnits == 0)
			HVUnits = "mi";
		else if (ExportData->VertUnits == 1)
			HVUnits = "km";
		else if (ExportData->VertUnits == 2)
			HVUnits = "m";
		else if (ExportData->VertUnits == 3)
			HVUnits = "ft";
		else if (ExportData->VertUnits == 4)
			HVUnits = "in";
		else if (ExportData->VertUnits == 5)
			HVUnits = "cm";
		if (ExportData->LayoutUnits == 0)
			LayoutUnits = "in";
		else
			LayoutUnits = "cm";
		IntLabels = (fabs(ExportData->VertScale - (int)ExportData->VertScale) < .01);
		if (IntLabels)
			fprintf(fHandle, "(Elevation Scale %d %s/%s) Tx\n", (long)ExportData->VertScale, HVUnits, LayoutUnits);
		else
			fprintf(fHandle, "(Elevation Scale %.4lf %s/%s) Tx\n", ExportData->VertScale, HVUnits, LayoutUnits);
		if (err = (fprintf(fHandle, "TO\n") <= 0))
			goto EndPlot;
		} // if

	if (ExportData->HorScaleLabel)
		{
		rgb.r = ExportData->HorLabelColor[0];
		rgb.g = ExportData->HorLabelColor[1];
		rgb.b = ExportData->HorLabelColor[2];
		cmyk = ToCMYK(rgb);
		fprintf(fHandle, "0 To\n");
		fprintf(fHandle, "1 0 0 1 %.4lf %.4lf 0 Tp\n", LeftGraphOffset, BottomGraphOffset - 30);
		fprintf(fHandle, "0 Tv\n");
		fprintf(fHandle, "TP\n");
		fprintf(fHandle, "0 Tr\n");
		fprintf(fHandle, "0 O\n");
		fprintf(fHandle, "%.4f %.4f %.4f %.4f k\n", cmyk.c / 255.0, cmyk.m / 255.0, cmyk.y / 255.0, cmyk.k / 255.0);
		fprintf(fHandle, "800 Ar\n");
		fprintf(fHandle, "0 J 0 j 1 w 4 M []0 d\n");
		fprintf(fHandle, "%%AI3_Note:\n");
		fprintf(fHandle, "0 D\n");
		fprintf(fHandle, "0 XR\n");
		sprintf(fontStr, "/_%s 12 Tf\n", ExportData->PreferredFont);
		fprintf(fHandle, fontStr);
		fprintf(fHandle, "0 Ts\n");
		fprintf(fHandle, "100 Tz\n");
		fprintf(fHandle, "0 Tt\n");
		fprintf(fHandle, "1 TA\n");
		fprintf(fHandle, "%%_ 0 XL\n");
		fprintf(fHandle, "36 0 Xb\n");
		fprintf(fHandle, "XB\n");
		fprintf(fHandle, "0 0 5 TC\n");
		fprintf(fHandle, "100 100 200 TW\n");
		fprintf(fHandle, "0 0 0 Ti\n");
		fprintf(fHandle, "0 Ta\n");
		fprintf(fHandle, "0 0 2 2 3 Th\n");
		fprintf(fHandle, "0 Tq\n");
		fprintf(fHandle, "0 0 Tl\n");
		fprintf(fHandle, "0 Tc\n");
		fprintf(fHandle, "0 Tw\n");
		if (ExportData->HorUnits == 0)
			HVUnits = "mi";
		else if (ExportData->HorUnits == 1)
			HVUnits = "km";
		else if (ExportData->HorUnits == 2)
			HVUnits = "m";
		else if (ExportData->HorUnits == 3)
			HVUnits = "ft";
		else if (ExportData->HorUnits == 4)
			HVUnits = "in";
		else if (ExportData->HorUnits == 5)
			HVUnits = "cm";
		if (ExportData->LayoutUnits == 0)
			LayoutUnits = "in";
		else
			LayoutUnits = "cm";
		IntLabels = (fabs(ExportData->HorScale - (int)ExportData->HorScale) < .01);
		if (IntLabels)
			fprintf(fHandle, "(Distance Scale %d %s/%s) Tx\n", (long)ExportData->HorScale, HVUnits, LayoutUnits);
		else
			fprintf(fHandle, "(Distance Scale %.4lf %s/%s) Tx\n", ExportData->HorScale, HVUnits, LayoutUnits);
		if (err = (fprintf(fHandle, "TO\n") <= 0))
			goto EndPlot;
		} // if

	EndPlot:
	if (IllustratorEnd() || err)
		UserMessageOK("Export Vector Profile", "Error writing profile file.");
	else
		UserMessageOK("Export Vector Profile", "Profile export completed.");
	} // if
else
	UserMessageOK("Export Vector Profile", "Profile export failed. File could not be opened for writing.");

if (! err && ExportData->LaunchIllustrator)
	LaunchIllustrator();
#else // WCS_BUILD_DEMO
UserMessageDemo("Vector Profiles cannot be saved.");
#endif // WCS_BUILD_DEMO

} // VecProfExportGUI::ExportProfile()

/*===========================================================================*/

// possible parameters to pass: page size, page resolution, others?
long VecProfExportGUI::IllustratorInit(double VecDPI, double RasDPI, long screenwidth, long screenheight)
{
time_t now;
int err = 0, WriteRaster = 0;
char *timebuf;
long  ipagewidth, ipageheight, imagewidth, imageheight;
double pagewidth, pageheight;
char CompleteOutputPath[512];

ExportData->PAF.GetPathAndName(CompleteOutputPath);
if (strlen(CompleteOutputPath) < 3 || stricmp(&CompleteOutputPath[strlen(CompleteOutputPath) - 3], ".ai"))
	strcat(CompleteOutputPath, ".ai");

if (fHandle = PROJ_fopen(CompleteOutputPath, "wb+"))
	{
	imagewidth = screenwidth;
	imageheight = screenheight;
	fprintf(fHandle, "%%!PS-Adobe-3.0\n");
	fprintf(fHandle, "%%%%Creator: " APP_TITLE " "APP_VERS "\n");
	/*** The next 3 lines are optional ***/
	// We should save the user name & company here
	fprintf(fHandle, "%%%%For: (%s %s)\n", GlobalApp->MainProj->UserName, GlobalApp->MainProj->UserEmail);
	// The title of this document should be saved here - ie: vector name?
	fprintf(fHandle, "%%%%Title: (%s)\n", Active->GetBestName());
	// The creation date is saved here
	(void)time(&now);
	timebuf = ctime(&now);
	timebuf[strlen(timebuf) - 1] = NULL;
	fprintf(fHandle, "%%%%CreationDate: (%s)\n", timebuf);
	// Don't know what the bounding box should really be - setting to 8.5" x 11 at this DPI"
	pagewidth = imagewidth;
	ipagewidth = quicklongceil(pagewidth);	// integer version
	pageheight = imageheight;
	ipageheight = quicklongceil(pageheight);
	fprintf(fHandle, "%%%%BoundingBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%%%HiResBoundingBox: 0.0000 0.0000 %.4lf %.4lf\n", pagewidth, pageheight);
	fprintf(fHandle, "%%%%DocumentProcessColors: Cyan Magenta Yellow Black\n");
	fprintf(fHandle, "%%%%DocumentFonts: ");
	sprintf(fontStr, "%s\n", ExportData->PreferredFont);
	fprintf(fHandle, fontStr);
	fprintf(fHandle, "%%%%DocumentNeededFonts: ");
	sprintf(fontStr, "%s\n", ExportData->PreferredFont);
	fprintf(fHandle, fontStr);
	fprintf(fHandle, "%%%%DocumentNeededResources: procset Adobe_level2_AI5 1.2 0\n");
	fprintf(fHandle, "%%%%+ procset Adobe_ColorImage_AI6 1.1 0\n");
	fprintf(fHandle, "%%%%+ procset Adobe_Illustrator_AI5 1.0 0\n");
	fprintf(fHandle, "%%AI5_FileFormat 2.0\n");
	fprintf(fHandle, "%%AI3_ColorUsage: Color\n");
	fprintf(fHandle, "%%%%AI6_ColorSeparationSet: 1 1 (AI6 Default Color Separation Set)\n");
	fprintf(fHandle, "%%%%+ Options: 1 16 0 1 0 1 1 1 0 1 1 1 1 18 0 0 0 0 0 0 0 0 -1 -1\n");
	fprintf(fHandle, "%%%%+ PPD: 1 21 0 0 60 45 2 2 1 0 0 1 0 0 0 0 0 0 0 0 0 0 ()\n");
	fprintf(fHandle, "%%AI3_TemplateBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI3_TileBox: 0 0 %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI3_DocumentPreview: None\n");
	fprintf(fHandle, "%%AI5_ArtSize: %ld %ld\n", ipagewidth, ipageheight);
	fprintf(fHandle, "%%AI5_RulerUnits: 2\n");	// 0 = in, 1 = mm, 2 = pt, others unknown
	fprintf(fHandle, "%%AI5_ArtFlags: 0 0 0 1 0 0 1 1 0\n");
	fprintf(fHandle, "%%AI5_TargetResolution: %ld\n", (long)VecDPI);
	fprintf(fHandle, "%%AI5_NumLayers: 1\n");
	// viewable coordinates when doc is opened lowx, highx, ???
	// give 1/2" border
	fprintf(fHandle, "%%AI5_OpenToView: -269 800 -1.5 1137 819 18 0 1 7 43 0 0\n");
	fprintf(fHandle, "%%AI5_OpenViewLayers: 7\n");
	fprintf(fHandle, "%%%%EndComments\n");
	fprintf(fHandle, "%%%%BeginProlog\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_level2_AI5 1.2 0\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_ColorImage_AI6 1.1 0\n");
	fprintf(fHandle, "%%%%IncludeResource: procset Adobe_Illustrator_AI5 1.0 0\n");
	fprintf(fHandle, "%%%%EndProlog\n");
	fprintf(fHandle, "%%%%BeginSetup\n");
	fprintf(fHandle, "%%%%IncludeFont: ");
	sprintf(fontStr, "%s\n", ExportData->PreferredFont);
	fprintf(fHandle, fontStr);
	fprintf(fHandle, "Adobe_level2_AI5 /initialize get exec\n");
	fprintf(fHandle, "Adobe_ColorImage_AI6 /initialize get exec\n");
	fprintf(fHandle, "Adobe_Illustrator_AI5 /initialize get exec\n");
	// No clue for the following - just cutting & pasting
	fprintf(fHandle, "[\n");
	fprintf(fHandle, "39/quotesingle 96/grave 130/quotesinglbase/florin/quotedblbase/ellipsis\n");
	fprintf(fHandle, "/dagger/daggerdbl/circumflex/perthousand/Scaron/guilsinglleft/OE 145/quoteleft\n");
	fprintf(fHandle, "/quoteright/quotedblleft/quotedblright/bullet/endash/emdash/tilde/trademark\n");
	fprintf(fHandle, "/scaron/guilsinglright/oe/dotlessi 159/Ydieresis /space 164/currency 166/brokenbar\n");
	fprintf(fHandle, "168/dieresis/copyright/ordfeminine 172/logicalnot/hyphen/registered/macron/ring\n");
	fprintf(fHandle, "/plusminus/twosuperior/threesuperior/acute/mu 183/periodcentered/cedilla\n");
	fprintf(fHandle, "/onesuperior/ordmasculine 188/onequarter/onehalf/threequarters 192/Agrave\n");
	fprintf(fHandle, "/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla/Egrave/Eacute\n");
	fprintf(fHandle, "/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex/Idieresis/Eth/Ntilde\n");
	fprintf(fHandle, "/Ograve/Oacute/Ocircumflex/Otilde/Odieresis/multiply/Oslash/Ugrave\n");
	fprintf(fHandle, "/Uacute/Ucircumflex/Udieresis/Yacute/Thorn/germandbls/agrave/aacute\n");
	fprintf(fHandle, "/acircumflex/atilde/adieresis/aring/ae/ccedilla/egrave/eacute/ecircumflex\n");
	fprintf(fHandle, "/edieresis/igrave/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute\n");
	fprintf(fHandle, "/ocircumflex/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex\n");
	fprintf(fHandle, "/udieresis/yacute/thorn/ydieresis\n");
	fprintf(fHandle, "TE\n");
	fprintf(fHandle, "%%AI55J_Tsume: None\n");
	fprintf(fHandle, "%%AI3_BeginEncoding: ");	// compiler generates garbage if this string is in the sprint below
	sprintf(fontStr, "_%s %s\n", ExportData->PreferredFont, ExportData->PreferredFont);
	fprintf(fHandle, fontStr);
	// F2_NOTE: Looks like this should be "[/_%s/%s 0 0 0 TZ\n" if a TrueType font is used.
	sprintf(fontStr, "[/_%s/%s 0 0 1 TZ\n", ExportData->PreferredFont, ExportData->PreferredFont);
	fprintf(fHandle, fontStr);
	// F2_NOTE: AdobeType should really only be used on Adobe and OpenType fonts.  TrueType should be used on Windows fonts.
	fprintf(fHandle, "%%AI3_EndEncoding AdobeType\n");

	fprintf(fHandle, "%%AI5_BeginPalette\n");
	fprintf(fHandle, "0 0 Pb\n");
	fprintf(fHandle, "1 1 1 1 k\n");
	fprintf(fHandle, "Pc\n");
	fprintf(fHandle, "PB\n");
	fprintf(fHandle, "%%AI5_EndPalette\n");
	fprintf(fHandle, "%%%%EndSetup\n");

	fprintf(fHandle, "%%AI5_BeginLayer\n");
	fprintf(fHandle, "1 1 1 1 0 0 0 0 79 128 255 0 100 Lb\n");	// vis preview, enabled, print, dimmed, multimask, index, r, g, b, ? ?
	fprintf(fHandle, "(Vectors) Ln\n");
	fprintf(fHandle, "0 A\n");							// edit lock - OFF
	fprintf(fHandle, "1 Ap\n");							// ???
	fprintf(fHandle, "0 R\n");							// overprinting on stroke path = OFF
	fprintf(fHandle, "0 0 0 1 K\n");					// stroke setcmykcolor
	fprintf(fHandle, "%ld Ar\n", (long)VecDPI);						// output res
	fprintf(fHandle, "0 D\n");							// winding # for fills
	err = (fprintf(fHandle, "0 XR\n") <= 0);			// ???

	} // if
else
	err = 1;

return (long)(err);

} // VecProfExportGUI::IllustratorInit

/*===========================================================================*/

long VecProfExportGUI::IllustratorEnd(void)
{
int err;

fprintf(fHandle, "LB\n");
fprintf(fHandle, "%%AI5_EndLayer--\n");
fprintf(fHandle, "%%%%PageTrailer\n");
fprintf(fHandle, "gsave annotatepage grestore showpage\n");
fprintf(fHandle, "%%%%Trailer\n");
fprintf(fHandle, "Adobe_Illustrator_AI5 /terminate get exec\n");
fprintf(fHandle, "Adobe_ColorImage_AI6 /terminate get exec\n");
fprintf(fHandle, "Adobe_level2_AI5 /terminate get exec\n");
err = (fprintf(fHandle, "%%EOF\n") <= 0);
fclose(fHandle);

return (long)err;

} // VecProfExportGUI::IllustratorEnd

/*===========================================================================*/

void VecProfExportGUI::LaunchIllustrator(void)
{
char CompleteOutputPath[512];

// rebuild file path used to save file
ExportData->PAF.GetPathAndName(CompleteOutputPath);
if (strlen(CompleteOutputPath) < 3 || stricmp(&CompleteOutputPath[strlen(CompleteOutputPath) - 3], ".ai"))
	strcat(CompleteOutputPath, ".ai");

GlobalApp->HelpSys->ShellOpenHelpFile(GlobalApp->MainProj->MungPath(CompleteOutputPath));

} // VecProfExportGUI::LaunchIllustrator

/*===========================================================================*/
