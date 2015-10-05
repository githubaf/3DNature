// SceneImportGUI.cpp
// Code for Scene Import module
// Built from LWExportGUI on Apr. 9, 1997 by Gary Huber
// Copyright 1997 Questar Productions

//#define WCS_SUPPORT_3DS

#include "stdafx.h"
#include "SceneImportGUI.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Render.h"
#include "Useful.h"
#include "MathSupport.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "AppMem.h"

#ifdef WCS_SUPPORT_3DS
#include "3dsftk.h"
#endif // WCS_SUPPORT_3DS

#include "resource.h"

static const char *LWImportItems[] = {
//	"LoadObject",
	"CameraMotion",
	"AddLight",
	"LightMotion",
	"ZoomFactor",
	"ZenithColor",
	"SkyColor",
	"FogMinDist",
	"FogMaxDist",
	"FogColor",
//	"FieldRendering",
//	"Antialiasing",
//	"MotionBlur",
//	"Resolution",
//	"Letterbox",
	"PixelAspectRatio",
	"LightColor",
	"LgtIntensity",
	"FirstFrame",
	"LastFrame",
	"FrameStep",
	"AmbientColor",
	"AmbIntensity",
	"FrameSize", // won't find in pre-LW7
	NULL
};

static const short LWImportItemLength[] = {
//	10,		//LoadObject
	12,		//CameraMotion
	8,		//AddLight
	11,		//LightMotion
	10,		//ZoomFactor
	11,		//ZenithColor
	8,		//SkyColor
	10,		//FogMinDist
	10,		//FogMaxDist
	8,		//FogColor
//	14,		//FieldRendering
//	12,		//Antialiasing
//	10,		//MotionBlur
//	10,		//Resolution
//	9,		//Letterbox
	16,		//PixelAspectRatio
	10,		//LightColor
	12,		//LgtIntensity
	10,		//FirstFrame
	9,		//LastFrame
	9,		//FrameStep
	12,		//AmbientColor
	12,		//AmbIntensity
	9		//FrameSize
};

static const char *LW7ImportItems[] = {
//	"LoadObject",
	"AddCamera",
	"AddLight",
	"LightMotion",
	"ZoomFactor",
	"ZenithColor",
	"SkyColor",
	"FogMinDistance",
	"FogMaxDistance",
	"FogColor",
//	"FieldRendering",
//	"Antialiasing",
//	"MotionBlur",
//	"Resolution",
//	"Letterbox",
	"PixelAspect",
	"LightColor",
	"LightIntensity",
	"FirstFrame",
	"LastFrame",
	"FrameStep",
	"AmbientColor",
	"AmbientIntensity",
	"FrameSize", // won't find in pre-LW7
	NULL
};

static const short LW7ImportItemLength[] = {
//	10,		//LoadObject
	9,		//AddCamera
	8,		//AddLight
	11,		//LightMotion
	10,		//ZoomFactor
	11,		//ZenithColor
	8,		//SkyColor
	14,		//FogMinDistance
	14,		//FogMaxDistance
	8,		//FogColor
//	14,		//FieldRendering
//	12,		//Antialiasing
//	10,		//MotionBlur
//	10,		//Resolution
//	9,		//Letterbox
	11,		//PixelAspect
	10,		//LightColor
	14,		//LgtIntensity
	10,		//FirstFrame
	9,		//LastFrame
	9,		//FrameStep
	12,		//AmbientColor
	16,		//AmbientIntensity
	9		//FrameSize
};

static const char *TDSImportItems[] = {
	"LoadObject",
	"ObjectMotion",
	"Omnilight",
	"Spotlight",
	"OmnilightMotion",
	"SpotlightMotion",
	"Camera",
	"CameraMotion",
	"CameraZoom",
	"ZenithColor",
	"SkyColor",
	"FogMinDist",
	"FogMaxDist",
	"FogColor",
	"LightColor",
	"LightIntensity",
	"AnimLength",
	"AmbientColor",
	"AmbientIntensity",
	NULL
};

static const short TDSImportItemLength[] = {
	10,		//Load Object
	12,		//Object Motion
	9,		//Omnilight
	9,		//Spotlight
	15,		//Omnilight Motion
	15,		//Spotlight Motion
	6,		//Camera
	12,		//Camera Motion
	10,		//Camera Zoom
	11,		//Zenith Color
	8,		//Sky Color
	10,		//Fog Min Dist
	10,		//Fog Max Dist
	8,		//Fog Color
	10,		//Light Color
	14,		//Light Intensity
	10,		//Anim Length
	12,		//Ambient Color
	16		//Ambient Intensity
};

//static short GotAspect, GotAmbient, GotFirstFrame, GotHazeStart, GotHazeEnd, GotLastFrame, GotLetterbox, GotLight, GotResolution;
long Resolution, PicAspect, Letterbox, LastFrame;

NativeGUIWin SceneImportGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SceneImportGUI::Open

/*===========================================================================*/

NativeGUIWin SceneImportGUI::Construct(void)
{
long TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_SCENE_IMPORT, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		WidgetLBSetHorizExt(IDC_PARLIST, 256);
		// Set up Widget bindings
		WidgetCBAddEnd(IDC_FORMATDROP, "LightWave 3D");
		#ifdef WCS_SUPPORT_3DS
		WidgetCBAddEnd(IDC_FORMATDROP, "3D Studio");
		#endif // WCS_SUPPORT_3DS

		for(TabIndex = WCS_USEFUL_UNIT_MILLIMETER; TabIndex <= WCS_USEFUL_UNIT_MILE_US_STATUTE; TabIndex ++)
			{
			WidgetCBAddEnd(IDC_UNITSDROP, GetUnitName(TabIndex));
			} // for

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // SceneImportGUI::Construct

/*===========================================================================*/

SceneImportGUI::SceneImportGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource)
: GUIFenetre('SCIM', this, "Scene Import"), LWInfo(ProjSource) // Yes, I know...
{

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
CurLight = NULL;
CurCamera = NULL;
CurSky = NULL;
CurAtmo = NULL;
CurOpt = NULL;
DefCoords = NULL;

ClearCameras = ClearLights = ClearSkies = ClearAtmos = ClearOptions = 0;
PlanetRad = FrameRate = 1.0;	// just dummy values until it is time to import

RefLatADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
RefLatADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
RefLatADT.SetValue(LWInfo.RefLat);
RefLonADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
RefLonADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
RefLonADT.SetValue(LWInfo.RefLon);

ImportUnits = ProjHost->Prefs.HorDisplayUnits;

ImportItems = 0;
ImportFile[0] = 0;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // SceneImportGUI::SceneImportGUI

/*===========================================================================*/

SceneImportGUI::~SceneImportGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // SceneImportGUI::~SceneImportGUI()

/*===========================================================================*/

long SceneImportGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SIM, 0);

return(0);

} // SceneImportGUI::HandleCloseWin

/*===========================================================================*/

long SceneImportGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(080, 80);
switch(ButtonID)
	{
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SIM, 0);
		break;
		} // 
	case ID_IMPORT:
		{
		Import();
		break;
		} // 
	case IDC_SELECTALL:
		{
		SelectAll();
		break;
		} // 
	default: break;
	} // ButtonID

return(0);

} // SceneImportGUI::HandleButtonClick

/*===========================================================================*/

long SceneImportGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_INPUT_FILE:
		{
		GetInputFile();
		break;
		} // IDC_INPUT_FILE
	} // ID

return(0);

} // SceneImportGUI::HandleDDChange

/*===========================================================================*/

void SceneImportGUI::ConfigureWidgets(void)
{

WidgetCBSetCurSel(IDC_FORMATDROP, WCS_IMPORT_FORMAT_LIGHTWAVE_3);

WidgetSNConfig(IDC_REFLAT, &RefLatADT);
WidgetSNConfig(IDC_REFLON, &RefLonADT);

ConfigureDD(NativeWin, IDC_INPUT_FILE, ProjHost->altobjectpath, 255, LWInfo.Name, 31, NULL);

ConfigureSC(NativeWin, IDC_CHECKCLEARCAM, &ClearCameras, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLEARLIGHT, &ClearLights, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLEARSKY, &ClearSkies, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLEARATMO, &ClearAtmos, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLEAROPT, &ClearOptions, SCFlag_Char, NULL, 0);

WidgetCBSetCurSel(IDC_UNITSDROP, ImportUnits - WCS_USEFUL_UNIT_MILLIMETER);

} // SceneImportGUI::ConfigureWidgets()

/*===========================================================================*/

void SceneImportGUI::GetInputFile(void)
{
char filename[255];
long Format;
FILE *fh = NULL;
#ifdef WCS_SUPPORT_3DS
file3ds *File3ds = NULL;
#endif // WCS_SUPPORT_3DS

Format = WidgetCBGetCurSel(IDC_FORMATDROP);

strmfp(filename, ProjHost->altobjectpath, LWInfo.Name);

if (Format == WCS_IMPORT_FORMAT_LIGHTWAVE_3)
	{
	if ((fh = PROJ_fopen(filename, "r")) == 0)
		{
		UserMessageOK(LWInfo.Name, "Unable to open file for input!");
		return;
		} /* if open error */
	} // if
#ifdef WCS_SUPPORT_3DS
else if (Format == 1) // Don't use WCS_IMPORT_FORMAT_3DS -- 3ds is second item in list, because LW7 doesn't have a different ID
	{
	if ((File3ds = OpenFile3ds((const char3ds *)filename, "r")) == 0)
		{
		UserMessageOK(LWInfo.Name, "Unable to open file for input!");
		CloseAllFiles3ds();
		return;
		} /* if open error */
	} // if
#endif // WCS_SUPPORT_3DS
else
	return;

WidgetLBClear(IDC_PARLIST);

switch (Format)
	{
	case WCS_IMPORT_FORMAT_LIGHTWAVE_3:
		{
		ScanLightWave(fh);
		break;
		} // WCS_IMPORT_FORMAT_LIGHTWAVE_3
	#ifdef WCS_SUPPORT_3DS
	case 1: // Don't use WCS_IMPORT_FORMAT_3DS -- 3ds is second item in list, because LW7 doesn't have a different ID
		{
		Import3DS(File3ds, FALSE, &LWInfo);
		break;
		} // WCS_IMPORT_FORMAT_3DS
	#endif // WCS_SUPPORT_3DS
	} // switch format

if (fh)
	fclose(fh);
#ifdef WCS_SUPPORT_3DS
else if (File3ds)
	CloseAllFiles3ds();
#endif // WCS_SUPPORT_3DS

} // SceneImportGUI::GetInputFile

/*===========================================================================*/

void SceneImportGUI::SetImportUnitScale(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_UNITSDROP);

ImportUnits = Current + WCS_USEFUL_UNIT_MILLIMETER;

LWInfo.UnitScale = ConvertFromMeters(1.0, ImportUnits);
LWInfo.UnitScale = 1.0 / LWInfo.UnitScale;

} // SceneImportGUI::SetImportUnitScale

/*===========================================================================*/

// set up reference point in cartesian space
void SceneImportGUI::SetupForImport(ImportInfo *LWInfo)
{
VertexDEM Vert;

Vert.Elev = 0;
Vert.Lat = LWInfo->RefLat;
Vert.Lon = 0.0;
BuildRotationMatrix((90.0 - LWInfo->RefLat) * PiOver180, 0.0, 0.0, LWInfo->RotMatx);
DefCoords->DegToCart(&Vert);
LWInfo->RefPt[0] = Vert.XYZ[0];
LWInfo->RefPt[1] = Vert.XYZ[1];
LWInfo->RefPt[2] = Vert.XYZ[2];

} // SceneImportGUI::SetupForImport

/*===========================================================================*/

void SceneImportGUI::Import(void)
{
FILE *fh = NULL;
#ifdef WCS_SUPPORT_3DS
file3ds *File3ds = NULL;
#endif // WCS_SUPPORT_3DS
NotifyTag Changes[2];
long Format;
char filename[255];

DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

strcpy(LWInfo.Path, ProjHost->altobjectpath);

LWInfo.RefLat = RefLatADT.CurValue;
LWInfo.RefLon = RefLonADT.CurValue;

Format = WidgetCBGetCurSel(IDC_FORMATDROP);
SetImportUnitScale();
PlanetRad = EffectsHost->GetPlanetRadius();
FrameRate = ProjHost->Interactive->GetFrameRate();
if (FrameRate <= 0.0)
	FrameRate = 30.0;
SetupForImport(&LWInfo);

strmfp(filename, ProjHost->altobjectpath, LWInfo.Name);

// freeze
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

if (ClearCameras)
	{
	EffectsHost->DeleteGroup(WCS_EFFECTSSUBCLASS_CAMERA);
	} // if
if (ClearLights)
	{
	EffectsHost->DeleteGroup(WCS_EFFECTSSUBCLASS_LIGHT);
	} // if
if (ClearSkies)
	{
	EffectsHost->DeleteGroup(WCS_EFFECTSSUBCLASS_SKY);
	} // if
if (ClearAtmos)
	{
	EffectsHost->DeleteGroup(WCS_EFFECTSSUBCLASS_ATMOSPHERE);
	} // if
if (ClearOptions)
	{
	EffectsHost->DeleteGroup(WCS_EFFECTSSUBCLASS_RENDEROPT);
	} // if

if (Format == WCS_IMPORT_FORMAT_LIGHTWAVE_3)
	{
	if ((fh = PROJ_fopen(filename, "r")) == 0)
		{
		UserMessageOK(LWInfo.Name, "Unable to open file for input!");
		return;
		} /* if open error */
	} // if
#ifdef WCS_SUPPORT_3DS
else if (Format == 1) // Don't use WCS_IMPORT_FORMAT_3DS -- 3ds is second item in list, because LW7 doesn't have a different ID
	{
	if ((File3ds = OpenFile3ds((const char3ds *)filename, "r")) == 0)
		{
		UserMessageOK(LWInfo.Name, "Unable to open file for input!");
		return;
		} /* if open error */
	} // if
#endif // WCS_SUPPORT_3DS
else
	return;

switch (Format)
	{
	case WCS_IMPORT_FORMAT_LIGHTWAVE_3:
		{
		ImportLightWave(fh, &LWInfo);
		break;
		} // WCS_IMPORT_FORMAT_LIGHTWAVE_3
	#ifdef WCS_SUPPORT_3DS
	case 1: // Don't use WCS_IMPORT_FORMAT_3DS -- 3ds is second item in list, because LW7 doesn't have a different ID
		{
		Import3DS(File3ds, TRUE, &LWInfo);
		break;
		} // WCS_IMPORT_FORMAT_3DS
	#endif // WCS_SUPPORT_3DS
	} // switch format

if (fh)
	fclose(fh);
#ifdef WCS_SUPPORT_3DS
else if (File3ds)
	CloseAllFiles3ds();
#endif // WCS_SUPPORT_3DS

// in case they import again
CurLight = NULL;
CurCamera = NULL;
CurSky = NULL;
CurAtmo = NULL;
CurOpt = NULL;

GlobalApp->UpdateProjectByTime();

// thaw
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // SceneImportGUI::Import()

/*===========================================================================*/

int SceneImportGUI::ScanLightWave(FILE *fh)
{
char InputBuf[256];
short i;
long Place;
int ImportVersion;
HWND MyListView;

ImportItems = 0;
if(MyListView = GetDlgItem(NativeWin, IDC_PARLIST))
	{
	InputBuf[0] = NULL;
	fgets(InputBuf, 256, fh);
	if(strncmp(InputBuf, "LWSC", 4))
		{ // not a LW Scene file
		return(ImportItems);
		} // if
	InputBuf[0] = NULL;
	fgets(InputBuf, 256, fh);
	if(InputBuf[0] == '1')
		{
		ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_3;
		} // if
	else if(InputBuf[0] == '3')
		{
		ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_7;
		} // else if
	else
		{ // not a format we understand
		return(ImportItems);
		} // else
		
	while (fgets(InputBuf, 256, fh))
		{
		i = 0;
		if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
			{
			while (LW7ImportItems[i])
				{
				if (! strnicmp(InputBuf, LW7ImportItems[i], LW7ImportItemLength[i]))
					{
					InputBuf[strlen(InputBuf) - 1] = 0;
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					break;
					} // if
				i ++;
				} // while
			} // if
		else
			{ // WCS_IMPORT_FORMAT_LIGHTWAVE_3
			while (LWImportItems[i])
				{
				if (! strnicmp(InputBuf, LWImportItems[i], LWImportItemLength[i]))
					{
					InputBuf[strlen(InputBuf) - 1] = 0;
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					break;
					} // if
				i ++;
				} // while
			} // else
		} // while
	} // if

return ImportItems;

} // SceneImportGUI::ScanLightWave

/*===========================================================================*/

long SceneImportGUI::ImportLightWave(FILE *fh, ImportInfo *LWInfo)
{
char MatchThis[256], InputBuf[256];
long DoCount, SelItems, *SelArray, ImportedItems = 0, MatchLength, ImportTheme, ThemeFound;
int ImportVersion, ItemLength;
HWND ItemList;

//GotResolution = GotAspect = GotLetterbox = GotFirstFrame = GotLastFrame = GotHazeStart = GotHazeEnd = GotLight = GotAmbient = 0;

InputBuf[0] = NULL;
fgets(InputBuf, 256, fh);
if(strncmp(InputBuf, "LWSC", 4))
	{ // not a LW Scene file
	return(ImportedItems);
	} // if
InputBuf[0] = NULL;
fgets(InputBuf, 256, fh);
if(InputBuf[0] == '1')
	{
	ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_3;
	} // if
else if(InputBuf[0] == '3')
	{
	ImportVersion = WCS_IMPORT_FORMAT_LIGHTWAVE_7;
	} // else if
else
	{ // not a format we understand
	return(ImportedItems);
	} // else

if(ItemList = GetDlgItem(NativeWin, IDC_PARLIST))
	{
	if(SelItems = SendMessage(ItemList, LB_GETSELCOUNT, 0, 0))
		{
		if (SelItems > 0)
			{
			if(SelArray = (long *)AppMem_Alloc(sizeof(long) * SelItems, 0))
				{
				SendMessage(ItemList, LB_GETSELITEMS, (WPARAM)SelItems, (LPARAM)SelArray);
				for(DoCount = 0; DoCount < SelItems; DoCount++)
					{
					MatchLength = WidgetLBGetText(IDC_PARLIST, SelArray[DoCount], MatchThis);
					while (fgets(InputBuf, 256, fh))
						{
						if (! strnicmp(InputBuf, MatchThis, MatchLength))
							{
							ImportTheme = ThemeFound = 0;
							if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
								{
								while (LW7ImportItems[ImportTheme])
									{
									if (! strnicmp(InputBuf, LW7ImportItems[ImportTheme], LW7ImportItemLength[ImportTheme]))
										{
										ThemeFound = 1;
										break;
										} // if
									ImportTheme ++;
									} // while
								} // if
							else
								{
								while (LWImportItems[ImportTheme])
									{
									if (! strnicmp(InputBuf, LWImportItems[ImportTheme], LWImportItemLength[ImportTheme]))
										{
										ThemeFound = 1;
										break;
										} // if
									ImportTheme ++;
									} // while
								} // if
							if (ThemeFound)
								{
								if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
									{
									ItemLength = LW7ImportItemLength[ImportTheme];
									} // if
								else
									{
									ItemLength = LWImportItemLength[ImportTheme];
									} // else
								switch (ImportTheme)
									{
//									case WCS_LWIMPORT_THEME_LOADOBJECT:
//										{
//										break;
//										} // 
									case WCS_LWIMPORT_THEME_CAMERAMOTION:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_CAMERAMOTION, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_ADDLIGHT:
										{
										break;
										} // 
									case WCS_LWIMPORT_THEME_LIGHTMOTION:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_LIGHTMOTION, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_LIGHTCOLOR:
										{
										ImportLWColor(WCS_LWIMPORT_THEME_LIGHTCOLOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_LGTINTENSITY:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_LGTINTENSITY, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_AMBIENTCOLOR:
										{
										ImportLWColor(WCS_LWIMPORT_THEME_AMBIENTCOLOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_AMBINTENSITY:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_AMBINTENSITY, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_ZOOMFACTOR:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_ZOOMFACTOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_ZENITHCOLOR:
										{
										ImportLWColor(WCS_LWIMPORT_THEME_ZENITHCOLOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_SKYCOLOR:
										{
										ImportLWColor(WCS_LWIMPORT_THEME_SKYCOLOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_FOGMINDIST:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_FOGMINDIST, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_FOGMAXDIST:
										{
										ImportLWMotion(WCS_LWIMPORT_THEME_FOGMAXDIST, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_FOGCOLOR:
										{
										ImportLWColor(WCS_LWIMPORT_THEME_FOGCOLOR, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), InputBuf, ImportVersion);
										break;
										} // 
//									case WCS_LWIMPORT_THEME_FIELDRENDERING:
//										{
//										ImportLWSetting(WCS_STNG_FIELDRENDER, fh, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
//										break;
//										} // 
//									case WCS_LWIMPORT_THEME_ANTIALIASING:
//										{
//										ImportLWSetting(WCS_STNG_MULTIPASSAA, fh, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
//										break;
//										} // 
//									case WCS_LWIMPORT_THEME_MOTIONBLUR:
//										{
//										ImportLWSetting(WCS_STNG_MOTIONBLUR, fh, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
//										break;
//										} // 
									case WCS_LWIMPORT_THEME_FIRSTFRAME:
										{
										ImportLWSetting(WCS_LWIMPORT_THEME_FIRSTFRAME, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_LASTFRAME:
										{
										ImportLWSetting(WCS_LWIMPORT_THEME_LASTFRAME, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_FRAMESTEP:
										{
										ImportLWSetting(WCS_LWIMPORT_THEME_FRAMESTEP, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
										break;
										} // 
									case WCS_LWIMPORT_THEME_FRAMESIZE:
										{
										ImportLWSetting(WCS_LWIMPORT_THEME_FRAMESIZE, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
										break;
										} // 
//									case WCS_LWIMPORT_THEME_RESOLUTION:
//										{
//										ImportLWSetting(WCS_STNG_SCRNWIDTH, fh, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
//										break;
//										} // 
//									case WCS_LWIMPORT_THEME_LETTERBOX:
//										{
//										ImportLWSetting(WCS_STNG_OVERSCAN, fh, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
//										break;
//										} // 
									case WCS_LWIMPORT_THEME_PIXELASPECTRATIO:
										{
										ImportLWSetting(WCS_LWIMPORT_THEME_PIXELASPECTRATIO, fh, LWInfo, AdvanceToNext(&InputBuf[ItemLength]), ImportVersion);
										break;
										} // 
									} // switch
								ImportedItems ++;
								} // if
							break;
							} // if
						} // while
					} // for
				AppMem_Free(SelArray, sizeof(long) * SelItems);
				SelArray = NULL;
				} // if
			} // if
		} // if
	} // if

return (ImportedItems);

} // SceneImportGUI::ImportLightWave

/*===========================================================================*/

void SceneImportGUI::ImportLWMotion(int MotionItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, char *InputBuf, int ImportVersion)
{
char Linear;
long NumItems, NumKeys, FrameCt, i, Frame;
float TCB[3];
double Value[9], Heading, Pitch, Bank, FrameTime;
VertexDEM VP;
AnimDoubleTime ADT[9];
GraphNode *GrNode;
char ObjectName[WCS_EFFECT_MAXNAMELENGTH];
int LW7NumChannels = 0, LW7Channel;

ObjectName[0] = NULL;
switch (MotionItem)
	{
	case WCS_LWIMPORT_THEME_CAMERAMOTION:
		{
		if (CurCamera = (Camera *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, NULL, NULL))
			{
			ADT[0].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT]);
			ADT[1].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON]);
			ADT[2].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV]);

			ADT[3].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]);
			ADT[4].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]);
			ADT[5].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]);

			ADT[6].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING]);
			ADT[7].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH]);
			ADT[8].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_LIGHTMOTION:
		{
		if (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NULL, NULL))
			{
			ADT[0].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
			ADT[1].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
			ADT[2].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_LGTINTENSITY:
		{
		if (CurLight || (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NULL, NULL)))
			{
			ADT[0].CopyRangeDefaults(&CurLight->Color.Intensity);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_AMBINTENSITY:
		{
		if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
			{
			ADT[0].CopyRangeDefaults(&CurAtmo->TopAmbientColor.Intensity);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_ZOOMFACTOR:
		{
		if (CurCamera || (CurCamera = (Camera *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, NULL, NULL)))
			{
			if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
				{
				ADT[0].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV]);

				// if possible, can we pre-read the Resolution & Aspect settings from the cached copies in the
				// list widget, as in actual processing (later) we won't encounter them before encountering
				// the ZoomFactor code, and therefore our ZoomFactor conversion will be wrong.
				if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
					{
					AttemptPreReadResAspect(LWInfo);
					} // if
				} // if
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_FOGMINDIST:
		{
		if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
			{
			ADT[0].CopyRangeDefaults(&CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_FOGMAXDIST:
		{
		if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
			{
			ADT[0].CopyRangeDefaults(&CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE]);
			} // if
		break;
		} // if
	} // if

ADT[0].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[1].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[2].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[3].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[4].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[5].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[6].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[7].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[8].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

if (HasEnvelope(Buffer) || MotionItem == WCS_LWIMPORT_THEME_CAMERAMOTION || MotionItem == WCS_LWIMPORT_THEME_LIGHTMOTION)
	{
	if (fgets(InputBuf, 256, fh))
		{
		if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
			{ // handle [object]Name and Show[object]
			AnimDoubleTime UnSplitChannels[9]; // X, Y, Z, SX, SY, SZ, H, P B
			for(;InputBuf[0];fgets(InputBuf, 256, fh))
				{
				if((!strnicmp(InputBuf, "CameraName", 10)) || (!strnicmp(InputBuf, "LightName", 9)))
					{
					Buffer = AdvanceToNext(InputBuf);
					Buffer[strlen(Buffer) - 1] = NULL; // chop EOL
					strncpy(ObjectName, Buffer, WCS_EFFECT_MAXNAMELENGTH - 2);
					ObjectName[WCS_EFFECT_MAXNAMELENGTH - 1] = NULL;
					} // if
				else if ((!strnicmp(InputBuf, "ShowCamera", 10)) || (!strnicmp(InputBuf, "ShowLight", 9)))
					{ // we don't care, skip it
					} // else
				else if ((!strnicmp(InputBuf, "CameraMotion", 10)) || (!strnicmp(InputBuf, "LightMotion", 9)))
					{ // ignore it
					} // else
				else if (!strnicmp(InputBuf, "NumChannels", 11))
					{ // grab it
					Buffer = AdvanceToNext(InputBuf);
					LW7NumChannels = atoi(Buffer);
					break;
					} // else
				InputBuf[0] = 0; // so a failed fgets will abort
				} // for

			for(LW7Channel = 0; LW7Channel < LW7NumChannels; LW7Channel++)
				{
				int ChannelKeys, KeyLoop;
				double KeyVal, KeyTime, KeyTens, KeyCont, KeyBias;
				int SpanType;
				// read "Channel n"
				fgets(InputBuf, 256, fh);

				// read "{ Envelope"
				fgets(InputBuf, 256, fh);

				// read ChannelKeys
				fgets(InputBuf, 256, fh);
				Buffer = AdvanceToFirst(InputBuf);
				ChannelKeys = atoi(Buffer);

				// read key lines
				for(KeyLoop = 0; KeyLoop < ChannelKeys; KeyLoop++)
					{
					// read Key line
					fgets(InputBuf, 256, fh);

					// parse key line
					Buffer = AdvanceToFirst(InputBuf);
					// skip over "Key"
					Buffer = AdvanceToNext(Buffer);
					// read Value
					KeyVal = atof(Buffer);
					Buffer = AdvanceToNext(Buffer);
					// read Time
					KeyTime = atof(Buffer);
					FrameTime = KeyTime;
					Buffer = AdvanceToNext(Buffer);
					// read SpanType
					SpanType = atoi(Buffer);
					Buffer = AdvanceToNext(Buffer);
					// read Tension
					KeyTens = atof(Buffer);
					Buffer = AdvanceToNext(Buffer);
					// read Continuity
					KeyCont = atof(Buffer);
					Buffer = AdvanceToNext(Buffer);
					// read Bias
					KeyBias = atof(Buffer);

					// make a key
					switch (MotionItem)
						{
						case WCS_LWIMPORT_THEME_CAMERAMOTION:
							{
							// We'll UnSet after all channels are done
							//UnSet_LWM(LWInfo, Value, &VP, Heading, Pitch, Bank);
							if (GrNode = UnSplitChannels[LW7Channel].AddNode(FrameTime, KeyVal, 0.0))
								{
								GrNode->SetTension(KeyTens);
								GrNode->SetContinuity(KeyCont);
								GrNode->SetBias(KeyBias);
								GrNode->SetLinear(SpanType == 3);
								} // if
							break;
							} // 
						case WCS_LWIMPORT_THEME_LIGHTMOTION:
							{
							// we'll UnSet and transfer the real keys after all channels are done
							//UnSet_LWMLight(LWInfo, Value, &VP);
							if (GrNode = UnSplitChannels[LW7Channel].AddNode(FrameTime, KeyVal, 0.0))
								{
								GrNode->SetTension(KeyTens);
								GrNode->SetContinuity(KeyCont);
								GrNode->SetBias(KeyBias);
								GrNode->SetLinear(SpanType == 3);
								} // if
							break;
							} // 
						case WCS_LWIMPORT_THEME_LGTINTENSITY:
						case WCS_LWIMPORT_THEME_AMBINTENSITY:
						case WCS_LWIMPORT_THEME_FOGMINDIST:
							{
							if (GrNode = ADT[0].AddNode(FrameTime, KeyVal, 0.0))
								{
								GrNode->SetTension(KeyTens);
								GrNode->SetContinuity(KeyCont);
								GrNode->SetBias(KeyBias);
								GrNode->SetLinear(SpanType == 3);
								} // if
							break;
							} // 
						case WCS_LWIMPORT_THEME_ZOOMFACTOR:
							{
							if (GrNode = ADT[0].AddNode(FrameTime, 
								2.0 * (atan((CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * CurOpt->OutputImageWidth / CurOpt->OutputImageHeight) / KeyVal) / PiOver180), 0.0))
								{
								GrNode->SetTension(KeyTens);
								GrNode->SetContinuity(KeyCont);
								GrNode->SetBias(KeyBias);
								GrNode->SetLinear(SpanType == 3);
								} // if
							break;
							} // 
						case WCS_LWIMPORT_THEME_FOGMAXDIST:
							{
							Value[1] = CurAtmo ? CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetValue(0, FrameTime): 0.0;
							if (GrNode = ADT[0].AddNode(FrameTime, KeyVal - Value[1] > 0.0 ? KeyVal - Value[1]: 1.0, 0.0))
								{
								GrNode->SetTension(KeyTens);
								GrNode->SetContinuity(KeyCont);
								GrNode->SetBias(KeyBias);
								GrNode->SetLinear(SpanType == 3);
								} // if
							break;
							} // 
						} // switch
					} // for

				// read Behaviour
				fgets(InputBuf, 256, fh);

				// read "}"
				fgets(InputBuf, 256, fh);
				} // for

			// handle unsplitting Light or Camera channels
			switch (MotionItem)
				{
				case WCS_LWIMPORT_THEME_CAMERAMOTION:
					{
					GraphNode *ScanNode;
					// Now we'll UnSet and transfer the real keys
					for(LW7Channel = 0; LW7Channel < 6; LW7Channel++)
						{
						for(ScanNode = UnSplitChannels[LW7Channel].GetFirstNode(0); ScanNode; ScanNode = ScanNode->Next)
							{
							Value[0] = UnSplitChannels[0].GetValue(0, ScanNode->Distance);
							Value[1] = UnSplitChannels[1].GetValue(0, ScanNode->Distance);
							Value[2] = UnSplitChannels[2].GetValue(0, ScanNode->Distance);
							// angles must be converted to degrees!
							Value[3] = UnSplitChannels[3].GetValue(0, ScanNode->Distance) * PiUnder180;
							Value[4] = UnSplitChannels[4].GetValue(0, ScanNode->Distance) * PiUnder180;
							Value[5] = UnSplitChannels[5].GetValue(0, ScanNode->Distance) * PiUnder180;
							UnSet_LWM(LWInfo, Value, &VP, Heading, Pitch, Bank);

							if (GrNode = ADT[0].AddNode(ScanNode->Distance, VP.Lat, 0.0))
								{
								if(LW7Channel == 0)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[1].AddNode(ScanNode->Distance, VP.Lon, 0.0))
								{
								if(LW7Channel == 1)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[2].AddNode(ScanNode->Distance, VP.Elev, 0.0))
								{
								if(LW7Channel == 2)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if

							if (GrNode = ADT[6].AddNode(ScanNode->Distance, Heading, 0.0))
								{
								if(LW7Channel == 0)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[7].AddNode(ScanNode->Distance, Pitch, 0.0))
								{
								if(LW7Channel == 1)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[8].AddNode(ScanNode->Distance, Bank, 0.0))
								{
								if(LW7Channel == 2)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if


							} // for
						} // for
					break;
					} // 
				case WCS_LWIMPORT_THEME_LIGHTMOTION:
					{
					GraphNode *ScanNode;
					// Now we'll UnSet and transfer the real keys, channel by channel, key by key
					for(LW7Channel = 0; LW7Channel < 3; LW7Channel++)
						{
						for(ScanNode = UnSplitChannels[LW7Channel].GetFirstNode(0); ScanNode; ScanNode = ScanNode->Next)
							{
							Value[0] = UnSplitChannels[0].GetValue(0, ScanNode->Distance);
							Value[1] = UnSplitChannels[1].GetValue(0, ScanNode->Distance);
							Value[2] = UnSplitChannels[2].GetValue(0, ScanNode->Distance);
							UnSet_LWMLight(LWInfo, Value, &VP);

							if (GrNode = ADT[0].AddNode(ScanNode->Distance, VP.Lat, 0.0))
								{
								if(LW7Channel == 0)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[1].AddNode(ScanNode->Distance, VP.Lon, 0.0))
								{
								if(LW7Channel == 1)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if
							if (GrNode = ADT[2].AddNode(ScanNode->Distance, VP.Elev, 0.0))
								{
								if(LW7Channel == 2)
									{
									GrNode->SetTension(ScanNode->TCB[0]);
									GrNode->SetContinuity(ScanNode->TCB[1]);
									GrNode->SetBias(ScanNode->TCB[2]);
									GrNode->SetLinear(ScanNode->Linear);
									} // if
								} // if

							} // for
						} // for
					break;
					} // 
				} // switch
			} // if
		else
			{ // pre-LW6/7
			Buffer = AdvanceToFirst(InputBuf);
			NumItems = atoi(Buffer);

			Buffer = AdvanceToNext(Buffer);
			if (Buffer[0] == 0 && fgets(InputBuf, 256, fh))
				{
				Buffer = AdvanceToFirst(InputBuf);
				} // if
			NumKeys = atoi(Buffer);
			Buffer = AdvanceToNext(Buffer);
			for (FrameCt = 0; FrameCt < NumKeys; FrameCt ++)
				{
				for (i = 0; i < NumItems && i < 9; i ++)
					{
					if (Buffer[0] == 0 && fgets(InputBuf, 256, fh))
						{
						Buffer = AdvanceToFirst(InputBuf);
						} // if
					Value[i] = atof(Buffer);
					Buffer = AdvanceToNext(Buffer);
					} // for
				if (fgets(InputBuf, 256, fh))
					{
					Buffer = AdvanceToFirst(InputBuf);
					Frame = atoi(Buffer);
					FrameTime = Frame / FrameRate;
					Linear = atoi(Buffer = AdvanceToNext(Buffer));
					Buffer = AdvanceToNext(Buffer);
					for (i = 0; i < 3; i ++)
						{
						if (Buffer[0] == 0 && fgets(InputBuf, 256, fh))
							Buffer = AdvanceToFirst(InputBuf);
						TCB[i] = (float)atof(Buffer);
						Buffer = AdvanceToNext(Buffer);
						} // for
					} // if
				switch (MotionItem)
					{
					case WCS_LWIMPORT_THEME_CAMERAMOTION:
						{
						UnSet_LWM(LWInfo, Value, &VP, Heading, Pitch, Bank);
						if (GrNode = ADT[0].AddNode(FrameTime, VP.Lat, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[1].AddNode(FrameTime, VP.Lon, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[2].AddNode(FrameTime, VP.Elev, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if

						if (GrNode = ADT[6].AddNode(FrameTime, Heading, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[7].AddNode(FrameTime, Pitch, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[8].AddNode(FrameTime, Bank, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						break;
						} // 
					case WCS_LWIMPORT_THEME_LIGHTMOTION:
						{
						UnSet_LWMLight(LWInfo, Value, &VP);
						if (GrNode = ADT[0].AddNode(FrameTime, VP.Lat, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[1].AddNode(FrameTime, VP.Lon, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						if (GrNode = ADT[2].AddNode(FrameTime, VP.Elev, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						break;
						} // 
					case WCS_LWIMPORT_THEME_LGTINTENSITY:
					case WCS_LWIMPORT_THEME_AMBINTENSITY:
					case WCS_LWIMPORT_THEME_FOGMINDIST:
						{
						if (GrNode = ADT[0].AddNode(FrameTime, Value[0], 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						break;
						} // 
					case WCS_LWIMPORT_THEME_ZOOMFACTOR:
						{
						if (GrNode = ADT[0].AddNode(FrameTime, 
							2.0 * (atan((CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * CurOpt->OutputImageWidth / CurOpt->OutputImageHeight) / Value[0]) / PiOver180), 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						break;
						} // 
					case WCS_LWIMPORT_THEME_FOGMAXDIST:
						{
						Value[1] = CurAtmo ? CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].GetValue(0, FrameTime): 0.0;
						if (GrNode = ADT[0].AddNode(FrameTime, Value[0] - Value[1] > 0.0 ? Value[0] - Value[1]: 1.0, 0.0))
							{
							GrNode->SetTension((double)TCB[0]);
							GrNode->SetContinuity((double)TCB[1]);
							GrNode->SetBias((double)TCB[2]);
							GrNode->SetLinear(Linear);
							} // if
						break;
						} // 
					} // switch
				} // for
			} // else pre-LW6/7
		} // if
	} // if
else
	{ // read a single simple value, easy!
	Value[0] = atof(Buffer);
	switch (MotionItem)
		{
		case WCS_LWIMPORT_THEME_LGTINTENSITY:
		case WCS_LWIMPORT_THEME_AMBINTENSITY:
		case WCS_LWIMPORT_THEME_FOGMINDIST:
			{
			ADT[0].SetValue(Value[0]);
			break;
			} // 
		case WCS_LWIMPORT_THEME_ZOOMFACTOR:
			{
			if (CurOpt)
				ADT[0].SetValue( 
					2.0 * (atan((CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * CurOpt->OutputImageWidth / CurOpt->OutputImageHeight) / Value[0]) / PiOver180));
			break;
			} // 
		case WCS_LWIMPORT_THEME_FOGMAXDIST:
			{
			Value[1] = CurAtmo ? CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue: 0.0;
			ADT[0].SetValue(Value[0] - Value[1] > 0.0 ? Value[0] - Value[1]: 1.0);
			break;
			} // 
		} // switch
	} // else

ADT[0].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[1].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[2].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[3].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[4].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[5].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[6].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[7].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADT[8].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

switch (MotionItem)
	{
	case WCS_LWIMPORT_THEME_CAMERAMOTION:
		{
		if (CurCamera)
			{
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], &ADT[0]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], &ADT[1]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], &ADT[2]);

			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], &ADT[3]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], &ADT[4]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], &ADT[5]);

			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING], &ADT[6]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH], &ADT[7]);
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK], &ADT[8]);

			CurCamera->CameraType = WCS_EFFECTS_CAMERATYPE_UNTARGETED;
			CurCamera->SetFloating(0);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_LIGHTMOTION:
		{
		if (CurLight)
			{
			CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &ADT[0]);
			CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &ADT[1]);
			CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ADT[2]);

			CurLight->SetFloating(0);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_LGTINTENSITY:
		{
		if (CurLight)
			{
			CurLight->Color.Intensity.Copy(&CurLight->Color.Intensity, &ADT[0]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_AMBINTENSITY:
		{
		if (CurAtmo)
			{
			CurAtmo->TopAmbientColor.Intensity.Copy(&CurAtmo->TopAmbientColor.Intensity, &ADT[0]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_ZOOMFACTOR:
		{
		if (CurCamera && CurOpt)
			{
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV], &ADT[0]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_FOGMINDIST:
		{
		if (CurAtmo)
			{
			CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].Copy(&CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART], &ADT[0]);
			} // if
		break;
		} // if
	case WCS_LWIMPORT_THEME_FOGMAXDIST:
		{
		if (CurAtmo)
			{
			CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].Copy(&CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE], &ADT[0]);
			} // if
		break;
		} // if
	} // if

} // SceneImportGUI::ImportLWMotion

/*===========================================================================*/

void SceneImportGUI::ImportLWColor(int ColorItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, char *InputBuf, int ImportVersion)
{
unsigned char Red, Green, Blue;
double RedF, GreenF, BlueF;
GradientCritter *CurGrad;
AnimColorTime *CurColor, ACT;

if(ImportVersion == WCS_IMPORT_FORMAT_LIGHTWAVE_7)
	{
	RedF = atof(Buffer);
	GreenF = atof(Buffer = AdvanceToNext(Buffer));
	BlueF = atof(Buffer = AdvanceToNext(Buffer));
	} // if
else
	{
	Red = (unsigned char)atoi(Buffer);
	RedF = Red / 255.0;
	Green = (unsigned char)atoi(Buffer = AdvanceToNext(Buffer));
	GreenF = Green / 255.0;
	Blue = (unsigned char)atoi(Buffer = AdvanceToNext(Buffer));
	BlueF = Blue / 255.0;
	} // else

switch (ColorItem)
	{
	case WCS_LWIMPORT_THEME_LIGHTCOLOR:
		{
		if (CurLight || (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NULL, NULL)))
			{
			ACT.CopyRangeDefaults(&CurLight->Color);
			ACT.SetValue3(RedF, GreenF, BlueF);
			CurLight->Color.Copy(&CurLight->Color, &ACT);
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_AMBIENTCOLOR:
		{
		if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
			{
			ACT.CopyRangeDefaults(&CurAtmo->TopAmbientColor);
			ACT.SetValue3(RedF, GreenF, BlueF);
			CurAtmo->TopAmbientColor.Copy(&CurAtmo->TopAmbientColor, &ACT);
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_ZENITHCOLOR:
		{
		if (CurSky || (CurSky = (Sky *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_SKY, NULL, NULL)))
			{
			if ((CurGrad = CurSky->SkyGrad.GetNextNode(NULL)) && CurGrad->GetThing())
				{
				if (CurColor = &((ColorTextureThing *)CurGrad->GetThing())->Color)
					{
					ACT.CopyRangeDefaults(CurColor);
					ACT.SetValue3(RedF, GreenF, BlueF);
					CurColor->Copy(CurColor, &ACT);
					} // if
				} // if
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_SKYCOLOR:
		{
		if (CurSky || (CurSky = (Sky *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_SKY, NULL, NULL)))
			{
			if (CurGrad = CurSky->SkyGrad.GetNextNode(NULL))
				{
				if ((CurGrad = CurSky->SkyGrad.GetNextNode(CurGrad)) && CurGrad->GetThing())
					{
					if (CurColor = &((ColorTextureThing *)CurGrad->GetThing())->Color)
						{
						ACT.CopyRangeDefaults(CurColor);
						ACT.SetValue3(RedF, GreenF, BlueF);
						CurColor->Copy(CurColor, &ACT);
						} // if
					} // if
				} // if
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_FOGCOLOR:
		{
		if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
			{
			ACT.CopyRangeDefaults(&CurAtmo->HazeColor);
			ACT.SetValue3(RedF, GreenF, BlueF);
			CurAtmo->HazeColor.Copy(&CurAtmo->HazeColor, &ACT);
			} // if
		break;
		} // 
	} // switch

} // SceneImportGUI::ImportLWColor

/*===========================================================================*/

void SceneImportGUI::ImportLWSetting(int SettingItem, FILE *fh, ImportInfo *LWInfo, char *Buffer, int ImportVersion)
{

switch (SettingItem)
	{
	case WCS_LWIMPORT_THEME_FIRSTFRAME:
		{
		if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
			{
			CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(atof(Buffer) / FrameRate);
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_LASTFRAME:
		{
		if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
			{
			CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(atof(Buffer) / FrameRate);
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_FRAMESTEP:
		{
		if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
			{
			CurOpt->FrameStep = atoi(Buffer);
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_FRAMESIZE:
		{
		if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
			{
			char *NextVal = NULL;
			CurOpt->OutputImageWidth = atoi(Buffer);
			if(NextVal = AdvanceToNext(Buffer))
				{
				CurOpt->OutputImageHeight = atoi(NextVal);
				} // if
			} // if
		break;
		} // 
	case WCS_LWIMPORT_THEME_PIXELASPECTRATIO:
		{
		if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
			{
			CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(atof(Buffer));
			} // if
		break;
		} // 
	} // switch

} // SceneImportGUI::ImportLWSetting

/*===========================================================================*/


void SceneImportGUI::AttemptPreReadResAspect(ImportInfo *LWInfo)
{
// attempt to read cached copies of Resolution and Aspect values from list widget before we
// actually encounter them in the file scan, as we need them prior to their occurrance.
char MatchThis[256];
long MatchLength, SelItems, DoCount;
HWND ItemList;

if(ItemList = GetDlgItem(NativeWin, IDC_PARLIST))
	{
	if(SelItems = SendMessage(ItemList, LB_GETCOUNT, 0, 0))
		{
		for(DoCount = 0; DoCount < SelItems; DoCount++)
			{
			MatchLength = WidgetLBGetText(IDC_PARLIST, DoCount, MatchThis);
			if (! strnicmp("FrameSize", MatchThis, LW7ImportItemLength[WCS_LWIMPORT_THEME_FRAMESIZE]))
				{
				// ImportLWSetting doesn't use second (file handle) arg anyway
				ImportLWSetting(WCS_LWIMPORT_THEME_FRAMESIZE, NULL, LWInfo, AdvanceToNext(&MatchThis[LW7ImportItemLength[WCS_LWIMPORT_THEME_FRAMESIZE]]), WCS_IMPORT_FORMAT_LIGHTWAVE_7);
				} // if
			if (! strnicmp("PixelAspect", MatchThis, LW7ImportItemLength[WCS_LWIMPORT_THEME_PIXELASPECTRATIO]))
				{
				// ImportLWSetting doesn't use second (file handle) arg anyway
				ImportLWSetting(WCS_LWIMPORT_THEME_PIXELASPECTRATIO, NULL, LWInfo, AdvanceToNext(&MatchThis[LW7ImportItemLength[WCS_LWIMPORT_THEME_PIXELASPECTRATIO]]), WCS_IMPORT_FORMAT_LIGHTWAVE_7);
				} // if
			} // for
		} // if
	} // if

} // SceneImportGUI::AttemptPreReadResAspect


/*===========================================================================*/

char *SceneImportGUI::AdvanceToNext(char *Buffer)
{
short i = 0;

while (Buffer[i])
	{
	if (Buffer[i] != ' ')
		{
		i ++;
		} // if
	else
		break;
	} // while

while (Buffer[i])
	{
	if (Buffer[i] == ' ')
		{
		i ++;
		} // if
	else
		break;
	} // while

return (&Buffer[i]);

} // SceneImportGUI::AdvanceToNext

/*===========================================================================*/

char *SceneImportGUI::AdvanceToFirst(char *Buffer)
{
short i = 0;

while (Buffer[i])
	{
	if (Buffer[i] == ' ')
		{
		i ++;
		} // if
	else
		break;
	} // while

return (&Buffer[i]);

} // SceneImportGUI::AdvanceToNext

/*===========================================================================*/

long SceneImportGUI::HasEnvelope(char *Buffer)
{
short i = 0;

while (Buffer[i])
	{
	if (Buffer[i] == ' ')
		{
		i ++;
		} // if
	else
		break;
	} // while

return (! strnicmp(&Buffer[i], "(envelope)", 10));

} // SceneImportGUI::AdvanceToNext

/*===========================================================================*/

void SceneImportGUI::SelectAll(void)
{
long i = 0;

for (i = 0; i < ImportItems; i ++)
	WidgetLBSetSelState(IDC_PARLIST, TRUE, i);

} // SceneImportGUI::SelectAll

/*===========================================================================*/
#ifdef WCS_SUPPORT_3DS

void SceneImportGUI::Import3DS(file3ds *File3ds, short ImportIt, ImportInfo *LWInfo)
{
dbtype3ds DBType3ds;
ulong3ds Omnilights, Spotlights, Cameras, OmnilightNodes, SpotlightNodes, CameraNodes, ObjectNodes;
database3ds *Database3ds = NULL;
meshset3ds *MeshSet3ds = NULL;
atmosphere3ds *Atmosphere3ds = NULL;
light3ds *Light3ds = NULL;
kfsets3ds *KFSets3ds = NULL;
kfmesh3ds *KFMesh3ds = NULL;
camera3ds *Camera3ds = NULL;
kfspot3ds *KFSpot3ds = NULL;
kfomni3ds *KFOmni3ds = NULL;
kfcamera3ds *KFCamera3ds = NULL;
namelist3ds *NameList3ds = NULL;

long Place, Count, NumKeys, KeyCt;
VertexDEM VP, FP;
char InputBuf[256];
HWND MyListView;
double ViewAngle, Bank, FrameTime;
AnimDoubleTime ADT[9];
AnimColorTime ACT;
GraphNode *GrNode;

if(MyListView = GetDlgItem(NativeWin, IDC_PARLIST))
	{
	InitDatabase3ds(&Database3ds);
	if (Database3ds && ! ftkerr3ds)
		{
		CreateDatabase3ds(File3ds, Database3ds);
		DBType3ds = GetDatabaseType3ds(Database3ds);
		if (DBType3ds == MeshFile && ! ftkerr3ds)
			{
			DisconnectDatabase3ds(Database3ds);
			if (ftkerr3ds)
				ftkerr3ds = 0;
			// object settings
			GetMeshSet3ds(Database3ds, &MeshSet3ds);
			if (MeshSet3ds && ! ftkerr3ds)
				{
				if (ImportIt)
					{
					if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_AMBIENTCOLOR, NULL))
						{
						if (CurAtmo || (CurAtmo = (Atmosphere *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, NULL, NULL)))
							{
							ACT.CopyRangeDefaults(&CurAtmo->TopAmbientColor);
							ACT.ReleaseNodes();

							ACT.SetValue3((double)MeshSet3ds->ambientlight.r, (double)MeshSet3ds->ambientlight.g, (double)MeshSet3ds->ambientlight.b);
							CurAtmo->TopAmbientColor.Copy(&CurAtmo->TopAmbientColor, &ACT);
							} // if
						} // if
					} // if
				else
					{
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)TDSImportItems[WCS_3DSIMPORT_THEME_AMBIENTCOLOR]);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					} // else
				ReleaseMeshSet3ds(&MeshSet3ds);
				} // if
			else
				{
				if (MeshSet3ds)
					ReleaseMeshSet3ds(&MeshSet3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to read 3DS Mesh Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error reading 3DS Mesh Settings chunk!\nOperation terminated.");
				goto EndIt;
				}

			// atmosphere settings
			// these don't seem to be read properly - just get default values
			// they seem to be written correctly by our exporter
			/*
			GetAtmosphere3ds(Database3ds, &Atmosphere3ds);
			if (Atmosphere3ds && ! ftkerr3ds)
				{
				if (ImportIt)
					{
					if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_FOGMINDIST, NULL))
						{
						Host->SetMoPar(WCS_MOTION_HAZESTRT, (double)(Atmosphere3ds->fog.nearplane / 1000.0));
						} // if
					if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_FOGMAXDIST, NULL))
						{
						Host->SetMoPar(WCS_MOTION_HAZERNG, (double)((Atmosphere3ds->fog.farplane - Atmosphere3ds->fog.nearplane) / 1000.0));
						} // if
					if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_FOGCOLOR, NULL))
						{
						Host->SetCoPar(WCS_COLOR_HAZE, 0, (short)(Atmosphere3ds->fog.fogcolor.r * 255.0));
						Host->SetCoPar(WCS_COLOR_HAZE, 1, (short)(Atmosphere3ds->fog.fogcolor.g * 255.0));
						Host->SetCoPar(WCS_COLOR_HAZE, 2, (short)(Atmosphere3ds->fog.fogcolor.b * 255.0));
						} // if
					} // if
				else
					{
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)TDSImportItems[WCS_3DSIMPORT_THEME_FOGMINDIST]);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)TDSImportItems[WCS_3DSIMPORT_THEME_FOGMAXDIST]);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)TDSImportItems[WCS_3DSIMPORT_THEME_FOGCOLOR]);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					} // else
				ReleaseAtmosphere3ds(&Atmosphere3ds);
				} // if
			else
				{
				if (Atmosphere3ds)
					ReleaseAtmosphere3ds(&Atmosphere3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to read 3DS Atmosphere Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error reading 3DS Atmosphere Settings chunk!\nOperation terminated.");
				goto EndIt;
				}
			*/
			// animation settings
			GetKfSets3ds(Database3ds, &KFSets3ds);
			if (KFSets3ds && ! ftkerr3ds)
				{
				if (ImportIt)
					{
					if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_ANIMLENGTH, NULL))
						{
						if (CurOpt || (CurOpt = (RenderOpt *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, NULL, NULL)))
							{
							CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(KFSets3ds->anim.length / FrameRate);
							} // if
						} // if
					} // if
				else
					{
					Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)TDSImportItems[WCS_3DSIMPORT_THEME_ANIMLENGTH]);
					if(Place != LB_ERR)
						{
						ImportItems ++;
						} // if
					} // else
				ReleaseKfSets3ds(&KFSets3ds);
				} // if
			else
				{
				if (KFSets3ds)
					ReleaseKfSets3ds(&KFSets3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Keyframe Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Keyframe Settings chunk!\nOperation terminated.");
				goto EndIt;
				} // if

			// light
			Omnilights = GetOmnilightCount3ds(Database3ds);
			Spotlights = GetSpotlightCount3ds(Database3ds);
			Cameras = GetCameraCount3ds(Database3ds);
			CameraNodes = GetCameraNodeCount3ds(Database3ds);
			OmnilightNodes = GetOmnilightNodeCount3ds(Database3ds);
			SpotlightNodes = GetSpotlightNodeCount3ds(Database3ds);
			ObjectNodes = GetObjectNodeCount3ds(Database3ds);
			if (Omnilights > 0)
				{
				GetOmnilightNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_ADDOMNILIGHT, NameList3ds->list[Count].name))
								{
								GetOmnilightByName3ds(Database3ds, NameList3ds->list[Count].name, &Light3ds);
								if (Light3ds && ! ftkerr3ds)
									{
									if (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name, NULL))
										{
										ADT[0].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
										ADT[1].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
										ADT[2].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();

										VP.XYZ[0] = (double)Light3ds->pos.x;
										VP.XYZ[1] = (double)Light3ds->pos.z;
										VP.XYZ[2] = (double)Light3ds->pos.y;
										UnSet_3DSPoint(LWInfo, &VP);
										ADT[0].SetValue(VP.Lat);
										ADT[1].SetValue(VP.Lon);
										ADT[2].SetValue(VP.Elev);

										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &ADT[0]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &ADT[1]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ADT[2]);

										ACT.CopyRangeDefaults(&CurLight->Color);
										ACT.ReleaseNodes();

										ACT.SetValue3((double)Light3ds->color.r, (double)Light3ds->color.g, (double)Light3ds->color.b);
										CurLight->Color.Copy(&CurLight->Color, &ACT);
										CurLight->SetFloating(0);
										} // if
									} // if
								if (Light3ds)
									{
									ReleaseLight3ds(&Light3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_ADDOMNILIGHT], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (OmnilightNodes > 0)
				{
				GetOmnilightNodeNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_OMNILIGHTMOTION, NameList3ds->list[Count].name))
								{
								GetOmnilightMotionByName3ds(Database3ds, NameList3ds->list[Count].name, &KFOmni3ds);
								if (KFOmni3ds && ! ftkerr3ds)
									{
									if ((CurLight = (Light *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name))
										|| (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name, NULL)))
										{
										// position keys
										ADT[0].Copy(&ADT[0], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
										ADT[1].Copy(&ADT[1], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
										ADT[2].Copy(&ADT[2], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();
										ADT[0].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

										NumKeys = (long)KFOmni3ds->npkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFOmni3ds->pkeys[KeyCt].time / FrameRate;
											VP.XYZ[0] = (double)KFOmni3ds->pos[KeyCt].x;
											VP.XYZ[1] = (double)KFOmni3ds->pos[KeyCt].z;
											VP.XYZ[2] = (double)KFOmni3ds->pos[KeyCt].y;
											UnSet_3DSPoint(LWInfo, &VP);
											if (GrNode = ADT[0].AddNode(FrameTime, VP.Lat, 0.0))
												{
												GrNode->SetTension((double)KFOmni3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFOmni3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFOmni3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[1].AddNode(FrameTime, VP.Lon, 0.0))
												{
												GrNode->SetTension((double)KFOmni3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFOmni3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFOmni3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[2].AddNode(FrameTime, VP.Elev, 0.0))
												{
												GrNode->SetTension((double)KFOmni3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFOmni3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFOmni3ds->pkeys[KeyCt].bias);
												} // if
											} // for
										ADT[0].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &ADT[0]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &ADT[1]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ADT[2]);

										ACT.Copy(&ACT, &CurLight->Color);
										ACT.ReleaseNodes();

										ACT.SetValue3((double)Light3ds->color.r, (double)Light3ds->color.g, (double)Light3ds->color.b);

										NumKeys = (long)KFOmni3ds->nckeys;
										ACT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFOmni3ds->ckeys[KeyCt].time / FrameRate;
											ACT.SetValue3((double)KFOmni3ds->color[KeyCt].r, (double)KFOmni3ds->color[KeyCt].g, (double)KFOmni3ds->color[KeyCt].b);
											if (GrNode = ACT.AddNode(FrameTime, 0.0))
												{
												GrNode->SetTension((double)KFOmni3ds->ckeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFOmni3ds->ckeys[KeyCt].continuity);
												GrNode->SetBias((double)KFOmni3ds->ckeys[KeyCt].bias);
												} // if
											} // for
										ACT.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										CurLight->Color.Copy(&CurLight->Color, &ACT);
										CurLight->SetFloating(0);
										} // if
									} // if
								if (KFOmni3ds)
									{
									ReleaseOmnilightMotion3ds(&KFOmni3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_OMNILIGHTMOTION], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (Spotlights > 0)
				{
				GetSpotlightNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_ADDSPOTLIGHT, NameList3ds->list[Count].name))
								{
								GetSpotlightByName3ds(Database3ds, NameList3ds->list[Count].name, &Light3ds);
								if (Light3ds && ! ftkerr3ds)
									{
									if (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name, NULL))
										{
										ADT[0].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
										ADT[1].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
										ADT[2].CopyRangeDefaults(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();

										VP.XYZ[0] = (double)Light3ds->pos.x;
										VP.XYZ[1] = (double)Light3ds->pos.z;
										VP.XYZ[2] = (double)Light3ds->pos.y;
										UnSet_3DSPoint(LWInfo, &VP);
										ADT[0].SetValue(VP.Lat);
										ADT[1].SetValue(VP.Lon);
										ADT[2].SetValue(VP.Elev);

										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &ADT[0]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &ADT[1]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ADT[2]);

										ACT.CopyRangeDefaults(&CurLight->Color);
										ACT.ReleaseNodes();

										ACT.SetValue3((double)Light3ds->color.r, (double)Light3ds->color.g, (double)Light3ds->color.b);
										CurLight->Color.Copy(&CurLight->Color, &ACT);
										CurLight->SetFloating(0);
										} // if
									} // if
								if (Light3ds)
									{
									ReleaseLight3ds(&Light3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_ADDSPOTLIGHT], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (SpotlightNodes > 0)
				{
				GetSpotlightNodeNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_SPOTLIGHTMOTION, NameList3ds->list[Count].name))
								{
								GetSpotlightMotionByName3ds(Database3ds, NameList3ds->list[Count].name, &KFSpot3ds);
								if (KFSpot3ds && ! ftkerr3ds)
									{
									if ((CurLight = (Light *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name))
										|| (CurLight = (Light *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_LIGHT, NameList3ds->list[Count].name, NULL)))
										{
										// position keys
										ADT[0].Copy(&ADT[0], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT]);
										ADT[1].Copy(&ADT[1], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON]);
										ADT[2].Copy(&ADT[2], &CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV]);
										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();
										ADT[0].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

										NumKeys = (long)KFSpot3ds->npkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFSpot3ds->pkeys[KeyCt].time / FrameRate;
											VP.XYZ[0] = (double)KFSpot3ds->pos[KeyCt].x;
											VP.XYZ[1] = (double)KFSpot3ds->pos[KeyCt].z;
											VP.XYZ[2] = (double)KFSpot3ds->pos[KeyCt].y;
											UnSet_3DSPoint(LWInfo, &VP);
											if (GrNode = ADT[0].AddNode(FrameTime, VP.Lat, 0.0))
												{
												GrNode->SetTension((double)KFSpot3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFSpot3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFSpot3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[1].AddNode(FrameTime, VP.Lon, 0.0))
												{
												GrNode->SetTension((double)KFSpot3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFSpot3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFSpot3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[2].AddNode(FrameTime, VP.Elev, 0.0))
												{
												GrNode->SetTension((double)KFSpot3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFSpot3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFSpot3ds->pkeys[KeyCt].bias);
												} // if
											} // for
										ADT[0].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT], &ADT[0]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON], &ADT[1]);
										CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].Copy(&CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV], &ADT[2]);

										ACT.Copy(&ACT, &CurLight->Color);
										ACT.ReleaseNodes();

										NumKeys = (long)KFSpot3ds->nckeys;
										ACT.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFSpot3ds->ckeys[KeyCt].time / FrameRate;
											ACT.SetValue3((double)KFSpot3ds->color[KeyCt].r, (double)KFSpot3ds->color[KeyCt].g, (double)KFSpot3ds->color[KeyCt].b);
											if (GrNode = ACT.AddNode(FrameTime, 0.0))
												{
												GrNode->SetTension((double)KFSpot3ds->ckeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFSpot3ds->ckeys[KeyCt].continuity);
												GrNode->SetBias((double)KFSpot3ds->ckeys[KeyCt].bias);
												} // if
											} // for
										ACT.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										CurLight->Color.Copy(&CurLight->Color, &ACT);
										CurLight->SetFloating(0);
										} // if
									} // if
								if (KFSpot3ds)
									{
									ReleaseSpotlightMotion3ds(&KFSpot3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_SPOTLIGHTMOTION], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (Cameras > 0)
				{
				GetCameraNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_ADDCAMERA, NameList3ds->list[Count].name))
								{
								GetCameraByName3ds(Database3ds, NameList3ds->list[Count].name, &Camera3ds);
								if (Camera3ds && ! ftkerr3ds)
									{
									if (CurCamera = (Camera *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, NameList3ds->list[Count].name, NULL))
										{
										ADT[0].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT]);
										ADT[1].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON]);
										ADT[2].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV]);

										ADT[3].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]);
										ADT[4].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]);
										ADT[5].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]);

										ADT[6].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK]);
										ADT[7].CopyRangeDefaults(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV]);

										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();
										ADT[3].ReleaseNodes();
										ADT[4].ReleaseNodes();
										ADT[5].ReleaseNodes();
										ADT[6].ReleaseNodes();
										ADT[7].ReleaseNodes();

										VP.XYZ[0] = (double)Camera3ds->position.x;
										VP.XYZ[1] = (double)Camera3ds->position.z;
										VP.XYZ[2] = (double)Camera3ds->position.y;
										FP.XYZ[0] = (double)Camera3ds->target.x;
										FP.XYZ[1] = (double)Camera3ds->target.z;
										FP.XYZ[2] = (double)Camera3ds->target.y;
										Bank = (double)(Camera3ds->roll);
										ViewAngle = (double)Camera3ds->fov;
										UnSet_3DSPoint(LWInfo, &VP);
										UnSet_3DSPoint(LWInfo, &FP);

										ADT[0].SetValue(VP.Lat);
										ADT[1].SetValue(VP.Lon);
										ADT[2].SetValue(VP.Elev);

										ADT[3].SetValue(FP.Lat);
										ADT[4].SetValue(FP.Lon);
										ADT[5].SetValue(FP.Elev);

										ADT[6].SetValue(Bank);
										ADT[7].SetValue(ViewAngle);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], &ADT[0]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], &ADT[1]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], &ADT[2]);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], &ADT[3]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], &ADT[4]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], &ADT[5]);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK], &ADT[6]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV], &ADT[7]);
										CurCamera->CameraType = WCS_EFFECTS_CAMERATYPE_TARGETED;
										CurCamera->SetFloating(0);
										} // if
									} // if
								if (Camera3ds)
									{
									ReleaseCamera3ds(&Camera3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_ADDCAMERA], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (CameraNodes > 0)
				{
				GetCameraNodeNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_CAMERAMOTION, NameList3ds->list[Count].name))
								{
								GetCameraMotionByName3ds(Database3ds, NameList3ds->list[Count].name, &KFCamera3ds);
								if (KFCamera3ds && ! ftkerr3ds)
									{
									if ((CurCamera = (Camera *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, NameList3ds->list[Count].name))
										|| (CurCamera = (Camera *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_CAMERA, NameList3ds->list[Count].name, NULL)))
										{
										// position keys
										ADT[0].Copy(&ADT[0], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT]);
										ADT[1].Copy(&ADT[1], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON]);
										ADT[2].Copy(&ADT[2], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV]);

										ADT[3].Copy(&ADT[3], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]);
										ADT[4].Copy(&ADT[4], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]);
										ADT[5].Copy(&ADT[5], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]);

										ADT[6].Copy(&ADT[6], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK]);
										ADT[7].Copy(&ADT[7], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV]);

										ADT[0].ReleaseNodes();
										ADT[1].ReleaseNodes();
										ADT[2].ReleaseNodes();
										ADT[3].ReleaseNodes();
										ADT[4].ReleaseNodes();
										ADT[5].ReleaseNodes();
										ADT[6].ReleaseNodes();
										ADT[7].ReleaseNodes();
										ADT[0].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[3].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[4].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[5].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[6].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[7].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

										NumKeys = (long)KFCamera3ds->npkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFCamera3ds->pkeys[KeyCt].time / FrameRate;
											VP.XYZ[0] = (double)KFCamera3ds->pos[KeyCt].x;
											VP.XYZ[1] = (double)KFCamera3ds->pos[KeyCt].z;
											VP.XYZ[2] = (double)KFCamera3ds->pos[KeyCt].y;
											UnSet_3DSPoint(LWInfo, &VP);
											if (GrNode = ADT[0].AddNode(FrameTime, VP.Lat, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[1].AddNode(FrameTime, VP.Lon, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[2].AddNode(FrameTime, VP.Elev, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->pkeys[KeyCt].bias);
												} // if
											} // for

										NumKeys = (long)KFCamera3ds->ntkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFCamera3ds->tkeys[KeyCt].time / FrameRate;
											FP.XYZ[0] = (double)KFCamera3ds->tpos[KeyCt].x;
											FP.XYZ[1] = (double)KFCamera3ds->tpos[KeyCt].z;
											FP.XYZ[2] = (double)KFCamera3ds->tpos[KeyCt].y;
											UnSet_3DSPoint(LWInfo, &FP);
											if (GrNode = ADT[3].AddNode(FrameTime, FP.Lat, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->tkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->tkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->tkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[4].AddNode(FrameTime, FP.Lon, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->tkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->tkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->tkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[5].AddNode(FrameTime, FP.Elev, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->tkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->tkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->tkeys[KeyCt].bias);
												} // if
											} // for

										NumKeys = (long)KFCamera3ds->nrkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFCamera3ds->rkeys[KeyCt].time / FrameRate;
											Bank = (double)KFCamera3ds->roll[KeyCt];
											if (GrNode = ADT[6].AddNode(FrameTime, Bank, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->rkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->rkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->rkeys[KeyCt].bias);
												} // if
											} // for

										NumKeys = (long)KFCamera3ds->nfkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFCamera3ds->fkeys[KeyCt].time / FrameRate;
											ViewAngle = (double)KFCamera3ds->fov[KeyCt];
											if (GrNode = ADT[7].AddNode(FrameTime, ViewAngle, 0.0))
												{
												GrNode->SetTension((double)KFCamera3ds->fkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFCamera3ds->fkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFCamera3ds->fkeys[KeyCt].bias);
												} // if
											} // for

										ADT[0].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[1].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[2].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[3].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[4].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[5].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[6].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[7].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], &ADT[0]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], &ADT[1]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], &ADT[2]);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], &ADT[3]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], &ADT[4]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], &ADT[5]);

										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK], &ADT[6]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV], &ADT[7]);
										CurCamera->CameraType = WCS_EFFECTS_CAMERATYPE_TARGETED;
										CurCamera->SetFloating(0);
										} // if
									} // if
								if (KFCamera3ds)
									{
									ReleaseCameraMotion3ds(&KFCamera3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_CAMERAMOTION], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			if (ObjectNodes > 0)
				{
				GetObjectNodeNameList3ds(Database3ds, &NameList3ds);
				if (NameList3ds->list && NameList3ds->count > 0)
					{
					if (ImportIt)
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							if (Match3DSImport(MyListView, WCS_3DSIMPORT_THEME_OBJECTMOTION, NameList3ds->list[Count].name))
								{
								GetObjectMotionByName3ds(Database3ds, NameList3ds->list[Count].name, &KFMesh3ds);
								if (KFMesh3ds && ! ftkerr3ds)
									{
									if (CurCamera)
										{
										ADT[3].Copy(&ADT[3], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT]);
										ADT[4].Copy(&ADT[4], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON]);
										ADT[5].Copy(&ADT[5], &CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV]);

										ADT[3].ReleaseNodes();
										ADT[4].ReleaseNodes();
										ADT[5].ReleaseNodes();
										ADT[3].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[4].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[5].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

										NumKeys = (long)KFMesh3ds->npkeys;
										for (KeyCt = 0; KeyCt < NumKeys; KeyCt ++)
											{
											FrameTime = KFMesh3ds->pkeys[KeyCt].time / FrameRate;
											FP.XYZ[0] = (double)KFMesh3ds->pos[KeyCt].x;
											FP.XYZ[1] = (double)KFMesh3ds->pos[KeyCt].z;
											FP.XYZ[2] = (double)KFMesh3ds->pos[KeyCt].y;
											UnSet_3DSPoint(LWInfo, &FP);
											if (GrNode = ADT[3].AddNode(FrameTime, FP.Lat, 0.0))
												{
												GrNode->SetTension((double)KFMesh3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFMesh3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFMesh3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[4].AddNode(FrameTime, FP.Lon, 0.0))
												{
												GrNode->SetTension((double)KFMesh3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFMesh3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFMesh3ds->pkeys[KeyCt].bias);
												} // if
											if (GrNode = ADT[5].AddNode(FrameTime, FP.Elev, 0.0))
												{
												GrNode->SetTension((double)KFMesh3ds->pkeys[KeyCt].tension);
												GrNode->SetContinuity((double)KFMesh3ds->pkeys[KeyCt].continuity);
												GrNode->SetBias((double)KFMesh3ds->pkeys[KeyCt].bias);
												} // if
											} // for

										ADT[3].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[4].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										ADT[5].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], &ADT[3]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], &ADT[4]);
										CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Copy(&CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], &ADT[5]);
										CurCamera->CameraType = WCS_EFFECTS_CAMERATYPE_TARGETED;
										CurCamera->SetFloating(0);
										} // if
									} // if
								if (KFMesh3ds)
									{
									ReleaseObjectMotion3ds(&KFMesh3ds);
									} // if
								} // if
							} // for
						} // if
					else
						{
						for (Count = 0; Count < (long)NameList3ds->count; Count ++)
							{
							sprintf(InputBuf, "%s - %s", TDSImportItems[WCS_3DSIMPORT_THEME_OBJECTMOTION], NameList3ds->list[Count].name);
							Place = SendMessage(MyListView, LB_ADDSTRING, 0, (LPARAM)InputBuf);
							if(Place != LB_ERR)
								{
								ImportItems ++;
								} // if
							} // for
						} // else
					} // if
				if (NameList3ds)
					ReleaseNameList3ds(&NameList3ds);
				} // if
			} // if database read and correct type
		else
			{
			UserMessageOK("3D Studio Scene Import", "Incorrect 3DS file type! Must be a .3ds file.\nOperation terminated.");
			} // if
	EndIt:
		ReleaseDatabase3ds(&Database3ds);
		} // if
	else
		{
		if (Database3ds)
			ReleaseDatabase3ds(&Database3ds);
		UserMessageOK("3D Studio Scene Import", "Unable to initialize 3DS Database!\nOperation terminated.");
		} // else
	} // if list view

} // SceneImportGUI::Import3DS

/*===========================================================================*/

short SceneImportGUI::Match3DSImport(HWND ListView, int ItemMatch, char *SecondMatch)
{
long ItemCount, Item;
char ItemText[256], *TextPtr;

if ((ItemCount = SendMessage(ListView, LB_GETCOUNT, 0, 0)) != LB_ERR && ItemCount > 0)
	{
	for (Item = 0; Item < ItemCount; Item ++)
		{
		if (SendMessage(ListView, LB_GETSEL, Item, 0))
			{
			if (SendMessage(ListView, LB_GETTEXT, Item, (LPARAM)ItemText))
				{
				if (! strncmp(ItemText, TDSImportItems[ItemMatch], TDSImportItemLength[ItemMatch]))
					{
					if (SecondMatch)
						{
						TextPtr = AdvanceToNext(AdvanceToNext(ItemText));
						if (! strcmp(TextPtr, SecondMatch))
							return (1);
						} // if
					else
						return (1);
					} // if
				} // if
			} // if
		} // for
	} // if

return (0);

} // SceneImportGUI::Match3DSImport

#endif // WCS_SUPPORT_3DS
