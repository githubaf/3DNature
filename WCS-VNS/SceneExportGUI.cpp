// SceneExportGUI.cpp
// Code for Scene Exporter
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary R. Huber
// Modified by GRH on 3/29/00.
// Copyright 1996-2000 Questar Productions

//#define WCS_SUPPORT_3DS

#include "stdafx.h"
#include "SceneExportGUI.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Render.h"
#include "Interactive.h"
#include "MathSupport.h"
#include "Effectslib.h"
#include "resource.h"

NativeGUIWin SceneExportGUI::Open(Project *Proj)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Proj))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SceneExportGUI::Open

/*===========================================================================*/

NativeGUIWin SceneExportGUI::Construct(void)
{
long TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_LW_EXPORT, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		// Set up Widget bindings
		WidgetCBAddEnd(IDC_FORMATDROP, "LightWave 3D");
		WidgetCBAddEnd(IDC_FORMATDROP, "LightWave 3D 7.x");
		#ifdef WCS_SUPPORT_3DS
		WidgetCBAddEnd(IDC_FORMATDROP, "3D Studio");
		#endif // WCS_SUPPORT_3DS

		for (TabIndex = WCS_USEFUL_UNIT_MILLIMETER; TabIndex <= WCS_USEFUL_UNIT_MILE_US_STATUTE; TabIndex ++)
			{
			WidgetCBAddEnd(IDC_UNITSDROP, GetUnitName(TabIndex));
			} // for

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // SceneExportGUI::Construct

/*===========================================================================*/

SceneExportGUI::SceneExportGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource)
: GUIFenetre('LWMO', this, "Scene Export"), LWInfo(ProjSource) // Yes, I know...
{

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;

RefLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
RefLatADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
RefLatADT.SetValue(LWInfo.RefLat);
RefLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
RefLonADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
RefLonADT.SetValue(LWInfo.RefLon);

ExportUnits = ProjHost->Prefs.HorDisplayUnits;

if (! (RendData = new RenderData(NULL)))
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // SceneExportGUI::SceneExportGUI

/*===========================================================================*/

SceneExportGUI::~SceneExportGUI()
{

if (RendData)
	delete RendData;
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // SceneExportGUI::~SceneExportGUI()

/*===========================================================================*/

long SceneExportGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_LWG, 0);

return(0);

} // SceneExportGUI::HandleCloseWin

/*===========================================================================*/

long SceneExportGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(021, 21);
switch(ButtonID)
	{
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_LWG, 0);
		break;
		} // 
	case ID_EXPORT:
		{
		#ifdef WCS_BUILD_DEMO
		UserMessageDemo("Demo version does not export scene files.");
		#else // WCS_BUILD_DEMO
		Export();
		#endif // WCS_BUILD_DEMO
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // SceneExportGUI::HandleButtonClick

/*===========================================================================*/

void SceneExportGUI::ConfigureWidgets(void)
{
char Str[24];

WidgetCBSetCurSel(IDC_FORMATDROP, WCS_EXPORT_FORMAT_LIGHTWAVE_7);

WidgetSNConfig(IDC_REFLAT, &RefLatADT);
WidgetSNConfig(IDC_REFLON, &RefLonADT);

sprintf(Str, "%d", LWInfo.MaxPolys);
WidgetSetText(IDC_MAXPOLY, Str);
sprintf(Str, "%d", LWInfo.MaxVerts);
WidgetSetText(IDC_MAXVERT, Str);
sprintf(Str, "%d", LWInfo.KeyFrameInt);
WidgetSetText(IDC_KEYFRAMEINT, Str);
WidgetSetCheck(IDC_BATHY, LWInfo.Bathymetry);

SECURITY_INLINE_CHECK(054, 54);

WidgetSetCheck(IDC_SCENEONLY, (LWInfo.ExportItem == 0));
WidgetSetCheck(IDC_SCENEDEM, (LWInfo.ExportItem == 1));
WidgetSetCheck(IDC_DEMONLY, (LWInfo.ExportItem == 2));
WidgetSetCheck(IDC_MOTIONONLY, (LWInfo.ExportItem == 3));
ConfigureDD(NativeWin, IDC_OUTPUT, ProjHost->altobjectpath, 255, NULL, 0, IDC_LABEL_TEMP);

WidgetCBSetCurSel(IDC_UNITSDROP, ExportUnits - WCS_USEFUL_UNIT_MILLIMETER);

BuildJobList();

} // SceneExportGUI::ConfigureWidgets()

/*===========================================================================*/

void SceneExportGUI::BuildJobList(void)
{
RenderJob *Current;
long Pos;

for (Pos = 0, Current = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); 
	Current; Current = (RenderJob *)Current->Next)
	{
	if (Current->Cam && Current->Options && Current->Cam->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && ! Current->Cam->Orthographic)
		{
		WidgetLBInsert(IDC_CAMLIST, Pos, Current->GetName());
		WidgetLBSetItemData(IDC_CAMLIST, Pos ++, Current);
		} // if
	} // for

if (Pos == 1)
	WidgetLBSetCurSel(IDC_CAMLIST, 0);
else if (Pos == 0)
	UserMessageOK("Scene Export", "A valid Render Job must exist in order to export the scene.\n There are no Render Jobs suitable for export.\n Render Jobs must have a valid Camera and Render Options.\n The Camera may not be planimetric nor Orthographic.");

} // SceneExportGUI::BuildJobList

/*===========================================================================*/

void SceneExportGUI::SetExportUnitScale(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_UNITSDROP);

ExportUnits = Current + WCS_USEFUL_UNIT_MILLIMETER;

LWInfo.UnitScale = ConvertFromMeters(1.0, ExportUnits);

} // SceneExportGUI::SetExportUnitScale

/*===========================================================================*/

void SceneExportGUI::Export(void)
{
char Value[24];
long Format, CurItem;
RenderJob *SelectedJob;

strcpy(LWInfo.Path, ProjHost->altobjectpath);

LWInfo.RefLat = RefLatADT.CurValue;
LWInfo.RefLon = RefLonADT.CurValue;
WidgetGetText(IDC_MAXPOLY, 24, Value);
LWInfo.MaxPolys = atoi(Value);
WidgetGetText(IDC_MAXVERT, 24, Value);
LWInfo.MaxVerts = atoi(Value);
WidgetGetText(IDC_KEYFRAMEINT, 24, Value);
LWInfo.KeyFrameInt = atoi(Value);
// key frame int of 0 will mean only export key frames
LWInfo.Bathymetry = WidgetGetCheck(IDC_BATHY) ? 1: 0;

Format = WidgetCBGetCurSel(IDC_FORMATDROP);

if (WidgetGetCheck(IDC_SCENEONLY))
	LWInfo.ExportItem = 0;
else if (WidgetGetCheck(IDC_SCENEDEM))
	LWInfo.ExportItem = 1;
else if (WidgetGetCheck(IDC_DEMONLY))
	LWInfo.ExportItem = 2;
else
	LWInfo.ExportItem = 3;

SetExportUnitScale();

// find out which job is selected
if ((CurItem = WidgetLBGetCurSel(IDC_CAMLIST)) != LB_ERR)
	{
	if ((SelectedJob = (RenderJob *)WidgetLBGetItemData(IDC_CAMLIST, CurItem)) != (RenderJob *)LB_ERR && SelectedJob)
		{
		if (EffectsHost->IsEffectValid(SelectedJob, WCS_EFFECTSSUBCLASS_RENDERJOB, 0))
			{
			// check to be sure there is a camera and render options
			if (SelectedJob->Options && SelectedJob->Cam)
				{
				if (RendData->InitToView(EffectsHost, ProjHost, DBHost, ProjHost->Interactive, SelectedJob->Options, SelectedJob->Cam, SelectedJob->Options->OutputImageWidth, SelectedJob->Options->OutputImageWidth))
					{
					switch (LWInfo.ExportItem)
						{
						case 0:
							{
							LWInfo.SaveDEMs = 0;
							switch (Format)
								{
								case WCS_EXPORT_FORMAT_LIGHTWAVE_3:
								case WCS_EXPORT_FORMAT_LIGHTWAVE_7:
									{
									LWScene_Export(&LWInfo, RendData, Format);
									break;
									} // LightWave 3D
								case WCS_EXPORT_FORMAT_3DS:
									{
									#ifdef WCS_SUPPORT_3DS
									ThreeDSScene_Export(&LWInfo, RendData);
									#endif // WCS_SUPPORT_3DS
									break;
									} // 3D Studio r4
								default:
									break;
								} // switch
							break;
							} // scene only
						case 1:
							{
							LWInfo.SaveDEMs = 1;
							switch (Format)
								{
								case WCS_EXPORT_FORMAT_LIGHTWAVE_3:
								case WCS_EXPORT_FORMAT_LIGHTWAVE_7:
									{
									LWScene_Export(&LWInfo, RendData, Format);
									break;
									} // LightWave 3D
								case WCS_EXPORT_FORMAT_3DS:
									{
									#ifdef WCS_SUPPORT_3DS
									ThreeDSScene_Export(&LWInfo, RendData);
									#endif // WCS_SUPPORT_3DS
									break;
									} // 3D Studio r4
								default:
									break;
								} // switch
							break;
							} // scene + DEM 
						case 2:
							{
							switch (Format)
								{
								case WCS_EXPORT_FORMAT_LIGHTWAVE_3:
								case WCS_EXPORT_FORMAT_LIGHTWAVE_7:
									{
									LWOB_Export(NULL, NULL, NULL, 1, -LWInfo.RefLon, LWInfo.RefLat - 90.0, &LWInfo, RendData, Format);
									break;
									} // LightWave 3D
								case WCS_EXPORT_FORMAT_3DS:
									{
									#ifdef WCS_SUPPORT_3DS
									ThreeDSScene_Export(&LWInfo, RendData);
									#endif // WCS_SUPPORT_3DS
									break;
									} // 3D Studio r4
								default:
									break;
								} // switch
							break;
							} // DEMs only
						case 3:
							{
							switch (Format)
								{
								case WCS_EXPORT_FORMAT_LIGHTWAVE_3:
								case WCS_EXPORT_FORMAT_LIGHTWAVE_7:
									{
									ExportWave(&LWInfo, NULL, RendData, Format);
									break;
									} // LightWave 3D
								case WCS_EXPORT_FORMAT_3DS:
									{
									#ifdef WCS_SUPPORT_3DS
									ThreeDSScene_Export(&LWInfo, RendData);
									#endif // WCS_SUPPORT_3DS
									break;
									} // 3D Studio r4
								default:
									break;
								} // switch
							break;
							} // camera motion only
						default:
							break;
						} // switch
					} // if
				else
					UserMessageOK("Scene Export", "Error initializing for export.");
				} // if
			else
				UserMessageOK("Scene Export", "Selected Render Job does not have a valid Camera or Render Options.");
			} // if
		else
			UserMessageOK("Scene Export", "Selected Render Job is no longer part of the Project.");
		} // if
	else
		UserMessageOK("Scene Export", "Selected Render job is invalid.");
	} // if
else
	UserMessageOK("Scene Export", "No Render job is selected for export.");

} // SceneExportGUI::Export()

/*===========================================================================*/

ImportInfo::ImportInfo(Project *ProjSource)
{

MaxPolys = 60000;
MaxVerts = 32767;
Bathymetry = 0;
ExportItem = 1;
SaveDEMs = 1;
KeyFrameInt = 0;
UnitScale = 1.0;
RefPt[0] = RefPt[1] = RefPt[2] = 0.0;
RefLat = ProjSource->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
RefLon = ProjSource->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
strcpy(Path, ProjSource->altobjectpath);
Name[0] = 0;
ZeroMatrix3x3(RotMatx);

} // ImportInfo::ImportInfo

/*===========================================================================*/

LightWaveMotion::LightWaveMotion()
{

XYZ[2] = XYZ[1] = XYZ[0] = 0.0; 
HPB[2] = HPB[1] = HPB[0] = 0.0; 
SCL[2] = SCL[1] = SCL[0] = 1.0; 
TCB[2] = TCB[1] = TCB[0] = 0.0; 
Frame = Linear = 0;

} // LightWaveMotion::LightWaveMotion

/*===========================================================================*/

ThreeDSMotion::ThreeDSMotion()
{

CamXYZ[2] = CamXYZ[1] = CamXYZ[0] = 0.0; 
TargetXYZ[2] = TargetXYZ[1] = TargetXYZ[0] = 0.0; 
Bank = FOV = 0.0; 
Frame = 0;

} // ThreeDSMotion::ThreeDSMotion
