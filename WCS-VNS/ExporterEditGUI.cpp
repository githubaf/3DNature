// ExporterEditGUI.cpp
// Code for SceneExporter editor
// Built from scratch on 4/18/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ExporterEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Conservatory.h"
#include "ExportControlGUI.h"
#include "resource.h"
#include "AppMem.h"
#include "AppHelp.h"
#include "SXExtension.h"
#include "Lists.h"
#include "PixelManager.h"

#ifdef WCS_BUILD_DEMO
extern int SXDemoWarned;
#endif // WCS_BUILD_DEMO
extern char *Attrib_TextSymbol;

char *ExporterEditGUI::TabNames[WCS_EXPORTERGUI_NUMTABS] = {"General", "Terrain", "LOD", "Texture", "Foliage", "Sky", "Misc", "Misc2"};

long ExporterEditGUI::ActivePage;
long ExporterEditGUI::ActiveAdvancedPage;

/*===========================================================================*/

// used by GIS Exporter
static void CalcCellSizes(const SceneExporter *Active, double &cellX, double &cellY)
{

if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	cellX = Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
	cellX /= (Active->OneDEMResX - 1);
	cellY = Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
	cellY /= (Active->OneDEMResY - 1);
	} // if
else
	{
	cellX = Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
	cellX /= Active->OneDEMResX;
	cellY = Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
	cellY /= Active->OneDEMResY;
	} // else
cellX = fabs(cellX);

} // CalcCellSizes

/*===========================================================================*/

static void GISclamp(GeoRegister *gReg, long &Cols, long &Rows, const double cellX, const double cellY)
{
double val1, val2;

val1 = WCS_ceil(gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue / cellY);
gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetCurValue(val1 * cellY);

val2 = WCS_floor(gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue / cellY);
gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetCurValue(val2 * cellY);

Rows = quickftol(fabs(val1 - val2));

val1 = WCS_ceil(gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue / cellX);
gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetCurValue(val1 * cellX);

val2 = WCS_floor(gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue / cellX);
gReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetCurValue(val2 * cellX);

Cols = quickftol(fabs(val1 - val2));

} // GISclamp

/*===========================================================================*/

NativeGUIWin ExporterEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ExporterEditGUI::Open

/*===========================================================================*/

NativeGUIWin ExporterEditGUI::Construct(void)
{
#ifdef WCS_BUILD_VNS
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS
int TabIndex, SetTarget, ListLoc;
long NameFieldNum;
char Str[64];

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_RTX
	#ifdef WCS_RENDER_SCENARIOS
	CreateSubWinFromTemplate(IDD_EXPORTER_GENERAL_VNS, 0, 0);
	#else // WCS_RENDER_SCENARIOS
	CreateSubWinFromTemplate(IDD_EXPORTER_GENERAL, 0, 0);
	#endif // WCS_RENDER_SCENARIOS

	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_EXPORTER_TERRAIN_VNS, 0, 1);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_EXPORTER_TERRAIN, 0, 1);
	#endif // WCS_BUILD_VNS

	CreateSubWinFromTemplate(IDD_EXPORTER_LOD, 0, 2);
	CreateSubWinFromTemplate(IDD_EXPORTER_TEXTURE, 0, 3);
	CreateSubWinFromTemplate(IDD_EXPORTER_FOLIAGE, 0, 4);
	CreateSubWinFromTemplate(IDD_EXPORTER_SKY, 0, 5);
	CreateSubWinFromTemplate(IDD_EXPORTER_3DOBJECTS, 0, 6);
	CreateSubWinFromTemplate(IDD_EXPORTER_MISC, 0, 7);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_STL, 1, 0);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_NVE, 1, 1);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_NVE2, 1, 2);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_NVE3, 1, 3);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_OF, 1, 4);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_FBX, 1, 5);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_GE, 1, 6);
	CreateSubWinFromTemplate(IDD_EXPORTER_ADVANCED_GE2, 1, 7);
	#endif // WCS_BUILD_RTX

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_EXPORTERGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);

		SetTarget = -1;
		if ((NameFieldNum = ExpTable.FindFieldByName("NAME")) >= 0)
			{
			TabIndex = ListLoc = 0;
			while (ExpTable.FetchFieldValueStr(TabIndex, NameFieldNum, Str, 64))
				{
				// see if dongle is authorized
				if (ExportControlGUI::FormatSpecificAuth(Str))
					{
					WidgetCBInsert(IDC_TARGETDROP, ListLoc, Str);
					if (! stricmp(Str, Active->ExportTarget))
						SetTarget = ListLoc;
					ListLoc ++;
					} // if
				TabIndex ++;
				} // while
			} // if field number found
		WidgetCBSetCurSel(IDC_TARGETDROP, SetTarget);

		FillComboOptions(IDC_FORMATDROP, "IMAGEFILE", Active->ImageFormat);
		FillComboOptions(IDC_FOLFORMATDROP, "FOLIMGFILE", Active->FoliageImageFormat);
		FillNVLogoInsertCombo();
		FillActionTypeCombo();
		FillActionItemCombo();

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
		//if (stricmp("GIS", Active->ExportTarget) == 0)
		//	GISclamping();
		} // if
	} // if
 
return (NativeWin);

} // ExporterEditGUI::Construct

/*===========================================================================*/

ExporterEditGUI::ExporterEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, SceneExporter *ActiveSource)
: GUIFenetre('RTXP', this, "Scene Exporter Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_EXPORTER, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								#ifdef WCS_RENDER_SCENARIOS
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								#endif // WCS_RENDER_SCENARIOS
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								0};
#ifdef WCS_BUILD_RTX
long ActiveFormat;
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];
#endif // WCS_BUILD_RTX
double CellSizeDefaults[3] = {FLT_MAX, .0000001, 1.0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
ImageHost = ImageSource;
Active = ActiveSource;
ReceivingDiagnostics = 0;
LastOneDEMResX = LastOneDEMResY = LastOneTexResX = LastOneTexResY = 512;
LastDEMResOption = LastTexResOption = 0;
DEMSquare = 0;
JustOpening = 1;
ActiveCameraListID = IDC_CAMERALIST;
ActiveLightListID = IDC_LIGHTLIST;
ActiveHazeListID = IDC_HAZELIST;
ExportSky = ExportCelest = ExportClouds = ExportStars = ExportAtmosphere = ExportVolumetrics = ExportSkyFeatures = RestoreSkyFeatures = 0;
ShowAdvanced = 0;
ActiveAction = NULL;
strcpy(AltFolMethod, "Alt. Method");
NSDEMCellSize.SetRangeDefaults(CellSizeDefaults);
NSDEMCellSize.SetValue(1.0);
NSDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
NSDEMCellSize.SetMultiplier(1.0);
NSDEMCellSize.SetIncrement(1.0);
NSDEMCellSize.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
WEDEMCellSize.SetRangeDefaults(CellSizeDefaults);
WEDEMCellSize.SetValue(1.0);
WEDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
WEDEMCellSize.SetMultiplier(1.0);
WEDEMCellSize.SetIncrement(1.0);
WEDEMCellSize.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

#ifdef WCS_BUILD_RTX
if ((GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0)
	&& (EffectsSource && DBSource && ImageSource && ActiveSource))
	{
	sprintf(NameStr, "Scene Exporter Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	if ((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Active->ExportTarget)) >= 0)
		ValidateSettings(ActiveFormat);
	JustOpening = 0;
	if (Active->GeoReg.TestForDefaultBounds())
		Active->GeoReg.SnapToDatabaseBounds(DBHost);
	LastOneDEMResX = Active->OneDEMResX;
	LastOneDEMResY = Active->OneDEMResY;
	LastFoliageRes = Active->FoliageRes;
	LastOneTexResX = Active->OneTexResX;
	LastOneTexResY = Active->OneTexResY;
	LastSkyRes = Active->SkyRes;
	LastDEMResOption = Active->DEMResOption;
	LastTexResOption = Active->TexResOption;
	ExportSky = Active->ExportSky;
	ExportCelest = Active->ExportCelest;
	ExportClouds = Active->ExportClouds;
	ExportStars = Active->ExportStars;
	ExportAtmosphere = Active->ExportAtmosphere;
	ExportVolumetrics = Active->ExportVolumetrics;
	RestoreSkyFeatures = ExportSkyFeatures = (ExportSky || ExportCelest || ExportClouds || ExportStars || ExportAtmosphere || ExportVolumetrics);
	BackupPrevBounds();
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	#ifdef WCS_BUILD_DEMO
	if (! SXDemoWarned)
		{
		#ifdef WCS_BUILD_VNS
		UserMessageOK("Scene Exporter Editor", "Scene Express is an add-on product and is not part of the basic VNS program. It is enabled in this Demo version so you can see what it looks like but you can't actually save any exported files. Be sure to specify that you want Scene Express when you purchase VNS.");
		#else // WCS_BUILD_VNS
		UserMessageOK("Scene Exporter Editor", "Scene Express is an add-on product and is not part of the basic WCS program. It is enabled in this Demo version so you can see what it looks like but you can't actually save any exported files. Be sure to specify that you want Scene Express when you purchase WCS.");
		#endif // WCS_BUILD_VNS
		SXDemoWarned = 1;
		} // if
	#endif // WCS_BUILD_DEMO
	} // if
else
#endif // WCS_BUILD_RTX
	ConstructError = 1;

} // ExporterEditGUI::ExporterEditGUI

/*===========================================================================*/

ExporterEditGUI::~ExporterEditGUI()
{

if (Active)
	Active->SetActiveAction(NULL);

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ExporterEditGUI::~ExporterEditGUI()

/*===========================================================================*/

long ExporterEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_EXP, 0);

return(0);

} // ExporterEditGUI::HandleCloseWin

/*===========================================================================*/

long ExporterEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);
long ActiveFormat;
char checked;

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EXP, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDSCENARIO:
		{
		AddScenario();
		break;
		} // IDC_ADDSCENARIO
	case IDC_REMOVESCENARIO:
		{
		RemoveScenario();
		break;
		} // IDC_REMOVESCENARIO
	case IDC_MOVESCENARIOUP:
		{
		ChangeScenarioListPosition(1);
		break;
		} // IDC_MOVESCENARIOUP
	case IDC_MOVESCENARIODOWN:
		{
		ChangeScenarioListPosition(0);
		break;
		} // IDC_MOVESCENARIODOWN
	case IDC_EDITTERRAINPAR:
		{
		GeneralEffect *DefaultPar;

		if (DefaultPar = EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, NULL))
			DefaultPar->EditRAHost();
		break;
		} // IDC_EDITTERRAINPAR
	case IDC_EDITCOORDS:
		{
		if (Active->Coords)
			Active->Coords->EditRAHost();
		//if (stricmp("GIS", Active->ExportTarget) == 0)
		//	GISclamping();
		break;
		} // IDC_EDITCOORDS
	case IDC_EXPORTNOW:
		{
		ExportNow();
		break;
		} // IDC_EXPORTNOW
	case IDC_SHOWNORMAL:
	case IDC_SHOWNORMAL2:
	case IDC_SHOWNORMAL3:
	case IDC_SHOWNORMAL4:
	case IDC_SHOWNORMAL5:
	case IDC_SHOWNORMAL6:
	case IDC_SHOWNORMAL7:
		{
		ConfigNormal();
		break;
		} // IDC_EXPORTNOW
	case IDC_SHOWADVANCED:
		{
		if (ShowAdvanced)
			ConfigNormal();
		else
			ConfigAdvanced();
		break;
		} // IDC_EXPORTNOW
	case IDC_SETBOUNDS:
		{
		SetBounds(NULL);
		break;
		} // 
	case IDC_SNAPBOUNDS:
		{
		ReceivingDiagnostics = 0;
		if (Active)
			{
			double cellX, cellY;
			NotifyTag Changes[2];

			if (stricmp("GIS", Active->ExportTarget) == 0)
				{
				CalcCellSizes(Active, cellX, cellY);
				} // if
			Active->GeoReg.SnapToDBObjs(Active->Coords);
			BackupPrevBounds();
			if (((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Active->ExportTarget)) >= 0)
				&& (ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "EDGE")))
				Active->BoundsType = WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES;
			if (stricmp("GIS", Active->ExportTarget) == 0)
				{
				Active->RBounds.SetCoords(Active->Coords ? Active->Coords: EffectsHost->FetchDefaultCoordSys());
				GISclamp(&Active->GeoReg, Active->OneDEMResX, Active->OneDEMResY, cellX, cellY);
				if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
					{
					/***
					PixResX = RenderWidth / (ImageWidth - 1);
					RenderWidth += PixResX;
					PixResY = RenderHeight / (ImageHeight - 1);
					RenderHeight += PixResY;
					***/
					Active->RBounds.SetOutsideBoundsFromCenters(&Active->GeoReg, Active->OneDEMResY, Active->OneDEMResX);
					} // if
				else
					{
					/***
					PixResX = RenderWidth / ImageWidth;
					PixResY = RenderHeight / ImageHeight;
					***/
					Active->RBounds.SetOutsideBounds(&Active->GeoReg);
					} // else
				Active->RBounds.DeriveCoords(Active->OneDEMResY, Active->OneDEMResX);
				}
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_HDILEARNMORE:
		{
		HDILearnMore();
		break;
		} // 
	case IDC_NV_INSERT:
		{
		InsertNVLogo();
		break;
		} // INSERTLIST
	case IDC_ADDACTION:
		{
		ActiveAction = Active->AddQueryAction();
		ConfigureActiveAction();
		break;
		} // IDC_ADDACTION
	case IDC_REMOVEACTION:
		{
		if (ActiveAction)
			{
			ActiveAction = Active->RemoveQueryAction(ActiveAction);
			ConfigureActiveAction();
			} // if
		break;
		} // IDC_REMOVEACTION
	case IDC_NV_INSERT2:
		{
		InsertActionText();
		break;
		} // INSERTLIST2
	case IDC_NV_INSERT3:
		{
		InsertActionAttrib();
		break;
		} // INSERTLIST3
	case IDC_NV_ADDITEM:
		{
		AddActionItem();
		break;
		} // IDC_NV_ADDITEM
	case IDC_NV_REMOVEITEM:
		{
		RemoveActionItem();
		break;
		} // IDC_NV_REMOVEITEM
	case IDC_NV_GRABALL:
		{
		GrabActionAll();
		break;
		} // IDC_NV_GRABALL
	case IDC_NV_QUERYGRAB:
		{
		GrabActionQuery();
		break;
		} // IDC_NV_QUERYGRAB
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

} // ExporterEditGUI::HandleButtonClick

/*===========================================================================*/

long ExporterEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
unsigned short usCtrlID = (unsigned short)CtrlID;

switch (usCtrlID)
	{
	case IDC_CAMERALIST:
	case IDC_CAMERALISTSINGLESEL:
		{
		SelectExportItem(usCtrlID, WCS_EFFECTSSUBCLASS_CAMERA);
		break;
		}
	case IDC_LIGHTLIST:
	case IDC_LIGHTLISTSINGLESEL:
		{
		SelectExportItem(usCtrlID, WCS_EFFECTSSUBCLASS_LIGHT);
		break;
		}
	case IDC_HAZELIST:
	case IDC_HAZELISTSINGLESEL:
		{
		SelectExportItem(usCtrlID, WCS_EFFECTSSUBCLASS_ATMOSPHERE);
		break;
		}
	case IDC_ACTIONLIST:
		{
		SelectActiveAction();
		break;
		} // IDC_ACTIONLIST
	} // switch usCtrlID

return (0);

} // ExporterEditGUI::HandleListSel

/*===========================================================================*/

long ExporterEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_SCENARIOLIST:
		{
		RemoveScenario();
		break;
		} // IDC_SCENARIOLIST
	case IDC_ACTIONLIST:
		{
		if (ActiveAction)
			{
			ActiveAction = Active->RemoveQueryAction(ActiveAction);
			ConfigureActiveAction();
			} // if
		break;
		} // IDC_ACTIONLIST
	} // switch

return(0);

} // ExporterEditGUI::HandleListDelItem

/*===========================================================================*/

long ExporterEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SCENARIOLIST:
		{
		EditScenario();
		break;
		}
	} // switch CtrlID

return (0);

} // ExporterEditGUI::HandleListDoubleClick

/*===========================================================================*/

long ExporterEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_TARGETDROP:
		{
		SelectOutputFormat();
		break;
		} // 
	case IDC_FORMATDROP:
		{
		SelectImageFormat();
		break;
		} // 
	case IDC_FOLFORMATDROP:
		{
		SelectFolImageFormat();
		break;
		} // 
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		}
	case IDC_STL_UNITSDROP:
		{
		SelectSTLUnits();
		break;
		} // 
	case IDC_NV_ACTIONTYPE:
		{
		SelectActionType();
		break;
		} // 
	} // switch CtrlID

return (0);

} // ExporterEditGUI::HandleCBChange

/*===========================================================================*/

long ExporterEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
unsigned short usCtrlID = (unsigned short)CtrlID;

switch (usCtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_NVE_LOGOTEXT:
	case IDC_NVE_WATERMARK:
	case IDC_NVE_NAME:
	case IDC_NVE_COPYRIGHT:
	case IDC_NVE_AUTHOR:
	case IDC_NVE_EMAIL:
	case IDC_NVE_USER1:
	case IDC_NVE_USER2:
		{
		RecordNVETextChange(usCtrlID);
		break;
		} // 
	case IDC_NV_ACTIONTEXT:
		{
		RecordActionTextChange(usCtrlID);
		break;
		} // 
	case IDC_FBX_PASSWORD:
		{
		RecordFBXTextChange(usCtrlID);
		break;
		} // 
	case IDC_GE_MESSAGE:
		{
		RecordGETextChange(usCtrlID);
		break;
		} // 
	} // switch usCtrlID

return (0);

} // ExporterEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long ExporterEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 0:
			case 4:
			case 5:
			case 6:
			case 7:
				{
				ShowPanel(1, -1);
				ShowPanel(0, NewPageID);
				break;
				} // 0
			case 1:
				{
				if (ShowAdvanced && Active->FormatExtension)
					{
					ShowPanel(0, -1);
					if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
						ShowPanel(1, 0);
					else if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
						ShowPanel(1, 1);
					else if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
						ShowPanel(1, 4);
					else if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
						ShowPanel(1, 5);
					else if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
						ShowPanel(1, 6);
					else
						ShowPanel(1, -1);
					} // if
				else
					{
					ShowPanel(1, -1);
					ShowPanel(0, NewPageID);
					} // else
				break;
				} // 1
			case 2:
				{
				if (ShowAdvanced && Active->FormatExtension)
					{
					ShowPanel(0, -1);
					if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
						ShowPanel(1, 2);
					else if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
						ShowPanel(1, 3);
					else
						ShowPanel(1, -1);
					} // if
				else
					{
					ShowPanel(1, -1);
					ShowPanel(0, NewPageID);
					} // else
				break;
				} // 2
			case 3:
				{
				if (ShowAdvanced && Active->FormatExtension)
					{
					ShowPanel(0, -1);
					if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
						ShowPanel(1, 3);
					else
						ShowPanel(1, -1);
					} // if
				else
					{
					ShowPanel(1, -1);
					ShowPanel(0, NewPageID);
					} // else
				break;
				} // 3
			default:
				{
				ShowPanel(1, -1);
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

if (ShowAdvanced)
	ActiveAdvancedPage = NewPageID;
else
	ActivePage = NewPageID;

return(0);

} // ExporterEditGUI::HandlePageChange

/*===========================================================================*/

long ExporterEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
int Result;
NotifyTag Changes[2];

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_EXPORTSKYFEATURES:
		{
		Active->ExportSky = (ExportSky && ExportSkyFeatures);
		Active->ExportCelest = (ExportCelest && ExportSkyFeatures);
		Active->ExportClouds = (ExportClouds && ExportSkyFeatures);
		Active->ExportStars = (ExportStars && ExportSkyFeatures);
		Active->ExportAtmosphere = (ExportAtmosphere && ExportSkyFeatures);
		Active->ExportVolumetrics = (ExportVolumetrics && ExportSkyFeatures);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTSKY:
		{
		ExportSky = Active->ExportSky;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTCLOUDS:
		{
		ExportClouds = Active->ExportClouds;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTCELEST:
		{
		ExportCelest = Active->ExportCelest;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTSTARS:
		{
		ExportStars = Active->ExportStars;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTATMO:
		{
		ExportAtmosphere = Active->ExportAtmosphere;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_EXPORTVOLUME:
		{
		ExportVolumetrics = Active->ExportVolumetrics;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} //
	case IDC_SQUARECELLS:
		{
		double cellX, cellY;

		CalcCellSizes(Active, cellX, cellY);
		if (Active->SquareCells)
			{
			NSDEMCellSize.SetCurValue(WEDEMCellSize.CurValue);
#ifdef GORILLA
			if (cellX < cellY)
				{
				cellY = cellX;
				NSDEMCellSize.SetCurValue(cellY);
				WidgetSNSync(IDC_GISNSDEMGRIDSIZE, WP_FISYNC_NONOTIFY);
				} // if
			else
				{
				cellX = cellY;
				WEDEMCellSize.SetCurValue(cellX);
				WidgetSNSync(IDC_GISWEDEMGRIDSIZE, WP_FISYNC_NONOTIFY);
				} // else
			GISclamp(&Active->GeoReg, Active->OneDEMResX, Active->OneDEMResY, cellX, cellY);
#endif // GORILLA
			GISclamp(&Active->GeoReg, Active->OneDEMResX, Active->OneDEMResY, WEDEMCellSize.CurValue, NSDEMCellSize.CurValue);
			WidgetSNSync(IDC_GISNSDEMGRIDSIZE, WP_FISYNC_NONOTIFY);
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} //
	case IDC_TILEWALLTEX:
		{
		if (Active->BurnWallShading && Active->TileWallTex)
			{
			Result = UserMessageCustom("Wall Export", "You cannot have both Burned Shading and Tiled Textures for walls at the same time. Please choose which you prefer to be enabled.", "Burned Shading", "Tiled Textures", NULL, 0);
			if (Result == 1)
				Active->TileWallTex = 0;
			else
				Active->BurnWallShading = 0;
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_BURNWALLSHADING:
		{
		if (Active->BurnWallShading && Active->TileWallTex)
			{
			Result = UserMessageCustom("Wall Export", "You cannot have both Burned Shading and Tiled Textures for walls at the same time. Please choose which you prefer to be enabled.", "Burned Shading", "Tiled Textures", NULL, 1);
			if (Result == 1)
				Active->TileWallTex = 0;
			else
				Active->BurnWallShading = 0;
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECK_DEFAULTSPEED:
		{
		// if NV format
		if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
			{
			// if use default speed
			if (((SXExtensionNVE *)Active->FormatExtension)->NavUseDefaultSpeed)
				{
				// set default speed for user reference
				((SXExtensionNVE *)Active->FormatExtension)->NavSpeed = (long)Active->CalcDefaultSpeed();
				} // if
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECK_GE_REVERSE_NORMALS:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // default
	} // switch CtrlID

return(0);

} // ExporterEditGUI::HandleSCChange

/*===========================================================================*/

long ExporterEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_RADIODEMPOW2:
	case IDC_RADIODEMPOW2PLUS1:
		{
		FiddleDEMRes();
		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->DEMTilesX * Active->OneDEMResX > 2049)
			{
			Active->DEMTilesX = 2049 / Active->OneDEMResX;
			WidgetFISync(IDC_TILESX, WP_FISYNC_NONOTIFY);
			} // if
		if (Active->DEMTilesY * Active->OneDEMResY > 2049)
			{
			Active->DEMTilesY = 2049 / Active->OneDEMResY;
			WidgetFISync(IDC_TILESY, WP_FISYNC_NONOTIFY);
			} // if
		#endif // WCS_BUILD_VNS
		ConfigDEMCellSize();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOTEXPOW2:
	case IDC_RADIOTEXPOW2PLUS1:
		{
		FiddleTexRes();
		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->TexTilesX * Active->OneTexResX > 2049)
			{
			Active->TexTilesX = 2049 / Active->OneTexResX;
			WidgetFISync(IDC_TEXTILESX, WP_FISYNC_NONOTIFY);
			} // if
		if (Active->TexTilesY * Active->OneTexResY > 2049)
			{
			Active->TexTilesY = 2049 / Active->OneTexResY;
			WidgetFISync(IDC_TEXTILESY, WP_FISYNC_NONOTIFY);
			} // if
		#endif // WCS_BUILD_VNS
		ConfigTexCellSize();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOFOLPOW2:
		{
		FiddleFolRes();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOSKYPOW2:
		{
		FiddleSkyRes();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOCELLEDGES:
	case IDC_RADIOCELLCENTERS:
		{
		ConfigDEMCellSize();
		ConfigTexCellSize();
		ComputeSTLScale();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // bounds type
	case IDC_STL_BUILDMODE_TOFIT:
	case IDC_STL_BUILDMODE_TOSCALE:
		{
		ComputeSTLScale();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // bounds type
	default:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // default
	} // switch CtrlID

return(0);

} // ExporterEditGUI::HandleSRChange

/*===========================================================================*/

long ExporterEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_PRIORITY:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_DEMCOLS:
		{
		if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->OneDEMResX))
				{
				if (Active->OneDEMResX > LastOneDEMResX)
					Active->OneDEMResX = 2 * LastOneDEMResX;
				else if (Active->OneDEMResX < LastOneDEMResX)
					Active->OneDEMResX = LastOneDEMResX / 2;
				} // if
			if (Active->OneDEMResX < 2)
				Active->OneDEMResX = 2;
			if (Active->OneDEMResX > Active->MaxDEMRes)
				Active->OneDEMResX = Active->MaxDEMRes;
			// cut it down to size
			if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
				Active->OneDEMResX /= 2;
			} // if
		else if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			// if user directly entered a valid power of two+1, skip the increment/decrement code
			if (1 != CountBits((unsigned int)(Active->OneDEMResX - 1)))
				{
				if (Active->OneDEMResX > LastOneDEMResX)
					Active->OneDEMResX = 1 + 2 * (LastOneDEMResX - 1);
				else if (Active->OneDEMResX < LastOneDEMResX)
					Active->OneDEMResX = 1 + (LastOneDEMResX - 1) / 2;
				} // if
			if (Active->OneDEMResX < 3)
				Active->OneDEMResX = 3;
			if (Active->OneDEMResX > Active->MaxDEMRes)
				Active->OneDEMResX = Active->MaxDEMRes;
			// cut it down to size
			if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
				Active->OneDEMResX = Active->OneDEMResX / 2 + 1;
			} // if
		else if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
			Active->OneDEMResX = (Active->MaxDEMVerts) / Active->OneDEMResY;

		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->DEMTilesX * Active->OneDEMResX > 2049)
			Active->OneDEMResX = LastOneDEMResX;
		#endif // WCS_BUILD_VNS

		LastOneDEMResX = Active->OneDEMResX;
		if (DEMSquare)
			{
			Active->OneDEMResY = Active->OneDEMResX;
			LastOneDEMResY = LastOneDEMResX;
			WidgetFISync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);
			} // if

		if (! strcmp(Active->ExportTarget, "GIS"))
			ComputeBoundsFromCellSize(); // modify bounds using DEMCellSize

		ConfigDEMCellSize();
		//if (stricmp("GIS", Active->ExportTarget) == 0)
		//	GISclamping();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_DEMROWS:
		{
		if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->OneDEMResY))
				{
				if (Active->OneDEMResY > LastOneDEMResY)
					Active->OneDEMResY = 2 * LastOneDEMResY;
				else if (Active->OneDEMResY < LastOneDEMResY)
					Active->OneDEMResY = LastOneDEMResY / 2;
				} // if
			if (Active->OneDEMResY < 2)
				Active->OneDEMResY = 2;
			if (Active->OneDEMResY > Active->MaxDEMRes)
				Active->OneDEMResY = Active->MaxDEMRes;
			// cut it down to size
			if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
				Active->OneDEMResY /= 2;
			} // if
		else if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			// if user directly entered a valid power of two+1, skip the increment/decrement code
			if (1 != CountBits((unsigned int)(Active->OneDEMResY - 1)))
				{
				if (Active->OneDEMResY > LastOneDEMResY)
					Active->OneDEMResY = 1 + 2 * (LastOneDEMResY - 1);
				else if (Active->OneDEMResY < LastOneDEMResY)
					Active->OneDEMResY = 1 + (LastOneDEMResY - 1) / 2;
				} // if
			if (Active->OneDEMResY < 3)
				Active->OneDEMResY = 3;
			if (Active->OneDEMResY > Active->MaxDEMRes)
				Active->OneDEMResY = Active->MaxDEMRes;
			// cut it down to size
			if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
				Active->OneDEMResY = Active->OneDEMResY / 2 + 1;
			} // if
		else if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
			Active->OneDEMResY = (Active->MaxDEMVerts) / Active->OneDEMResX;

		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->DEMTilesY * Active->OneDEMResY > 2049)
			Active->OneDEMResY = LastOneDEMResY;
		#endif // WCS_BUILD_VNS

		LastOneDEMResY = Active->OneDEMResY;

		if (! strcmp(Active->ExportTarget, "GIS"))	
			ComputeBoundsFromCellSize(); // modify bounds using DEMCellSize

		ConfigDEMCellSize();
		//if (stricmp("GIS", Active->ExportTarget) == 0)
		//	GISclamping();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_FOLROWS:
		{
		if (Active->FolResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->FoliageRes))
				{
				if (Active->FoliageRes > LastFoliageRes)
					Active->FoliageRes = 2 * LastFoliageRes;
				else if (Active->FoliageRes < LastFoliageRes)
					Active->FoliageRes = LastFoliageRes / 2;
				} // if
			if (Active->FoliageRes < 2)
				Active->FoliageRes = 2;
			if (Active->FoliageRes > Active->MaxFolRes)
				Active->FoliageRes = Active->MaxFolRes;
			} // if
		LastFoliageRes = Active->FoliageRes;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_TEXCOLS:
		{
		if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->OneTexResX))
				{
				if (Active->OneTexResX > LastOneTexResX)
					Active->OneTexResX = 2 * LastOneTexResX;
				else if (Active->OneTexResX < LastOneTexResX)
					Active->OneTexResX = LastOneTexResX / 2;
				} // if
			if (Active->OneTexResX < 2)
				Active->OneTexResX = 2;
			if (Active->OneTexResX > Active->MaxTexRes)
				Active->OneTexResX = Active->MaxTexRes;
			} // if
		else if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			// if user directly entered a valid power of two+1, skip the increment/decrement code
			if (1 != CountBits((unsigned int)(Active->OneTexResX - 1)))
				{
				if (Active->OneTexResX > LastOneTexResX)
					Active->OneTexResX = 1 + 2 * (LastOneTexResX - 1);
				else if (Active->OneTexResX < LastOneTexResX)
					Active->OneTexResX = 1 + (LastOneTexResX - 1) / 2;
				} // if
			if (Active->OneTexResX < 3)
				Active->OneTexResX = 3;
			if (Active->OneTexResX > Active->MaxTexRes)
				Active->OneTexResX = Active->MaxTexRes;
			} // if

		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->TexTilesX * Active->OneTexResX > 2049)
			Active->OneTexResX = LastOneTexResX;
		#endif // WCS_BUILD_VNS

		LastOneTexResX = Active->OneTexResX;
		ConfigTexCellSize();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_TEXROWS:
		{
		if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->OneTexResY))
				{
				if (Active->OneTexResY > LastOneTexResY)
					Active->OneTexResY = 2 * LastOneTexResY;
				else if (Active->OneTexResY < LastOneTexResY)
					Active->OneTexResY = LastOneTexResY / 2;
				} // if
			if (Active->OneTexResY < 2)
				Active->OneTexResY = 2;
			if (Active->OneTexResY > Active->MaxTexRes)
				Active->OneTexResY = Active->MaxTexRes;
			} // if
		else if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			// if user directly entered a valid power of two+1, skip the increment/decrement code
			if (1 != CountBits((unsigned int)(Active->OneTexResY - 1)))
				{
				if (Active->OneTexResY > LastOneTexResY)
					Active->OneTexResY = 1 + 2 * (LastOneTexResY - 1);
				else if (Active->OneTexResY < LastOneTexResY)
					Active->OneTexResY = 1 + (LastOneTexResY - 1) / 2;
				} // if
			if (Active->OneTexResY < 3)
				Active->OneTexResY = 3;
			if (Active->OneTexResY > Active->MaxTexRes)
				Active->OneTexResY = Active->MaxTexRes;
			} // if

		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->TexTilesY * Active->OneTexResY > 2049)
			Active->OneTexResY = LastOneTexResY;
		#endif // WCS_BUILD_VNS

		LastOneTexResY = Active->OneTexResY;
		ConfigTexCellSize();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_SKYROWS:
		{
		if (Active->SkyResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
			{
			// if user directly entered a valid power of two, skip the increment/decrement code
			if (1 != CountBits((unsigned int)Active->SkyRes))
				{
				if (Active->SkyRes > LastSkyRes)
					Active->SkyRes = 2 * LastSkyRes;
				else if (Active->SkyRes < LastSkyRes)
					Active->SkyRes = LastSkyRes / 2;
				} // if
			if (Active->SkyRes < 2)
				Active->SkyRes = 2;
			if (Active->SkyRes > Active->MaxTexRes)
				Active->SkyRes = Active->MaxTexRes;
			} // if
		LastSkyRes = Active->SkyRes;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_NORTH:
	case IDC_WEST:
	case IDC_EAST:
	case IDC_SOUTH:
		{
		if (! strcmp(Active->ExportTarget, "GIS"))
			{
			GISclamp(&Active->GeoReg, Active->OneDEMResX, Active->OneDEMResY, WEDEMCellSize.CurValue, NSDEMCellSize.CurValue);
			WidgetFISync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_DEMCOLS, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_NORTH, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_WEST, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_EAST, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
			//	// modify bounds using DEMCellSize
			//	AdjustBoundsFromCellSize(CtrlID);
			} // if
		ConfigDEMCellSize();
		ConfigTexCellSize();
		ComputeSTLScale();
		//if (stricmp("GIS", Active->ExportTarget) == 0)
		//	GISclamping();
		break;
		} // geo bounds
	case IDC_TILESX:
	case IDC_TILESY:
		{
		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->DEMTilesX * Active->OneDEMResX > 2049)
			{
			Active->DEMTilesX = 2049 / Active->OneDEMResX;
			WidgetFISync(IDC_TILESX, WP_FISYNC_NONOTIFY);
			} // if
		if (Active->DEMTilesY * Active->OneDEMResY > 2049)
			{
			Active->DEMTilesY = 2049 / Active->OneDEMResY;
			WidgetFISync(IDC_TILESY, WP_FISYNC_NONOTIFY);
			} // if
		#endif // WCS_BUILD_VNS

		if (! strcmp(Active->ExportTarget, "GIS"))
			// modify bounds using DEMCellSize
			ComputeBoundsFromCellSize();

		ConfigDEMCellSize();
		if (Active->EqualTiles)
			{
			Active->TexTilesX = Active->DEMTilesX;
			Active->TexTilesY = Active->DEMTilesY;
			ConfigTexCellSize();
			WidgetFISync(IDC_TEXTILESY, WP_FISYNC_NONOTIFY);
			WidgetFISync(IDC_TEXTILESX, WP_FISYNC_NONOTIFY);
			} // if
		break;
		} // # DEM tiles
	case IDC_TEXTILESX:
	case IDC_TEXTILESY:
		{
		#ifndef WCS_BUILD_VNS
		// largest DEM and tex res in WCS is 2049
		if (Active->TexTilesX * Active->OneTexResX > 2049)
			{
			Active->TexTilesX = 2049 / Active->OneTexResX;
			WidgetFISync(IDC_TEXTILESX, WP_FISYNC_NONOTIFY);
			} // if
		if (Active->TexTilesY * Active->OneTexResY > 2049)
			{
			Active->TexTilesY = 2049 / Active->OneTexResY;
			WidgetFISync(IDC_TEXTILESY, WP_FISYNC_NONOTIFY);
			} // if
		#endif // WCS_BUILD_VNS

		ConfigTexCellSize();
		break;
		} // # texture tiles
	case IDC_TERLODNUM:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // detail levels
	case IDC_STL_BUILDSCALE:
		{
		if (Active->FormatExtension)
			{
			// STL and VRML-STL
			if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
				{
				((SXExtensionSTL *)Active->FormatExtension)->BuildMode = WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOSCALE;
				ComputeSTLScale();
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				} // if
			} // if
		break;
		} // detail levels
	case IDC_STL_MAXDIMX:
	case IDC_STL_MAXDIMY:
	case IDC_STL_MAXDIMZ:
		{
		ComputeSTLScale();
		break;
		} // STL dimensions
	case IDC_GISNSDEMGRIDSIZE:
	case IDC_GISWEDEMGRIDSIZE:
		{
		/***
		// modify bounds using DEMCellSize
		ComputeBoundsFromCellSize();
		***/
		if (Active->SquareCells)
			NSDEMCellSize.SetCurValue(WEDEMCellSize.CurValue);
		GISclamp(&Active->GeoReg, Active->OneDEMResX, Active->OneDEMResY, WEDEMCellSize.CurValue, NSDEMCellSize.CurValue);
		// reconfigure
		ConfigDEMCellSize();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_GISWEDEMGRIDSIZE
	case IDC_GE_DRAW_ORDER:
	case IDC_GE_LABEL_RESCALE:
	case IDC_GE_SCREENX:
	case IDC_GE_SCREENY:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // GoogleEarth controls
	} // switch CtrlID

BackupPrevBounds();

return(0);

} // ExporterEditGUI::HandleFIChange

/*===========================================================================*/

long ExporterEditGUI::HandleFIChangeArrow(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
double change, TopLeftAdjust, BottomRightAdjust;
long delta;

switch (CtrlID)
	{
	case IDC_NORTH:
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue > PrevNorthValue)
			delta = 1;
		else
			delta = -1;
		if ((Active->OneDEMResY + delta) > 1)
			{
			LastOneDEMResY = Active->OneDEMResY = Active->OneDEMResY + delta;
			change = delta * NSDEMCellSize.CurValue;
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->SetCurValue(PrevNorthValue + change);
			WidgetFISync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
			} // if
		else
			{
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->SetCurValue(PrevNorthValue);
			} // else
		WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
		break;
	case IDC_WEST:
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue > PrevWestValue)
			delta = 1;
		else
			delta = -1;
		change = delta * WEDEMCellSize.CurValue;
		if ((! Active->Coords) || Active->Coords->GetGeographic())
			delta = -delta;
		if ((Active->OneDEMResX - delta) > 1)
			{
			LastOneDEMResX = Active->OneDEMResX = Active->OneDEMResX - delta;
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->SetCurValue(PrevWestValue + change);
			WidgetFISync(IDC_DEMCOLS, WP_FISYNC_NONOTIFY);
			} // if
		else
			{
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->SetCurValue(PrevWestValue);
			} // else
		WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
		break;
	case IDC_EAST:
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue > PrevEastValue)
			delta = 1;
		else
			delta = -1;
		change = delta * WEDEMCellSize.CurValue;
		if ((! Active->Coords) || Active->Coords->GetGeographic())
			delta = -delta;
		if ((Active->OneDEMResX + delta) > 1)
			{
			LastOneDEMResX = Active->OneDEMResX = Active->OneDEMResX + delta;
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->SetCurValue(PrevEastValue + change);
			WidgetFISync(IDC_DEMCOLS, WP_FISYNC_NONOTIFY);
			} // if
		else
			{
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->SetCurValue(PrevEastValue);
			} // else
		WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
		break;
	case IDC_SOUTH:
		if (Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue > PrevSouthValue)
			delta = 1;
		else
			delta = -1;
		if ((Active->OneDEMResY - delta) > 1)
			{
			LastOneDEMResY = Active->OneDEMResY = Active->OneDEMResY - delta;
			change = delta * NSDEMCellSize.CurValue;
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->SetCurValue(PrevSouthValue + change);
			WidgetFISync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);
			} // if
		else
			{
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->SetCurValue(PrevSouthValue);
			} // else
		WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
		break;
	case IDC_DEMCOLS:
		delta = Active->OneDEMResX - LastOneDEMResX;
		LastOneDEMResX = Active->OneDEMResX;
		change = delta * WEDEMCellSize.CurValue;
		if ((! Active->Coords) || Active->Coords->GetGeographic())
			change = -change;
		if (WidgetGetCheck(IDC_TBTG_WLOCK))
			{
			TopLeftAdjust = 0.0;
			BottomRightAdjust = change;
			} // if
		else if (WidgetGetCheck(IDC_TBTG_ELOCK))
			{
			TopLeftAdjust = -change;
			BottomRightAdjust = 0.0;
			} // else if
		else
			{
			change *= 0.5;
			TopLeftAdjust = -change;
			BottomRightAdjust = change;
			} // else
		Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->SetCurValue(PrevWestValue + TopLeftAdjust);
		Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->SetCurValue(PrevEastValue + BottomRightAdjust);
		WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
		break;
	case IDC_DEMROWS:
		delta = Active->OneDEMResY - LastOneDEMResY;
		LastOneDEMResY = Active->OneDEMResY;
		change = delta * NSDEMCellSize.CurValue;
		if (WidgetGetCheck(IDC_TBTG_NLOCK))
			{
			TopLeftAdjust = 0.0;
			BottomRightAdjust = -change;
			} // if
		else if (WidgetGetCheck(IDC_TBTG_SLOCK))
			{
			TopLeftAdjust = change;
			BottomRightAdjust = 0.0;
			} // else if
		else
			{
			change *= 0.5;
			TopLeftAdjust = change;
			BottomRightAdjust = -change;
			} // else
		Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->SetCurValue(PrevNorthValue + TopLeftAdjust);
		Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->SetCurValue(PrevSouthValue + BottomRightAdjust);
		WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
		break;
	default:
		break;
	} // switch
	
BackupPrevBounds();

return(0);

} // ExporterEditGUI::HandleFIChangeArrow

/*===========================================================================*/

void ExporterEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Done = 0;
#ifdef WCS_BUILD_VNS
long Pos, CurPos;
GeneralEffect *MyEffect, *MatchEffect;
#endif // WCS_BUILD_VNS


if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// will be handled in HandleFIChange()
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	DisableWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		{
		if (((Changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	Done = 1;
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
	ConfigDEMCellSize();
	ConfigTexCellSize();
	ConfigBoundsMetric();
	ComputeSTLScale();
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

#ifdef WCS_RENDER_SCENARIOS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
Interested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_SCENARIO, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[3] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildScenarioList();
	} // if component added or name changed
#endif // WCS_RENDER_SCENARIOS

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[3] = MAKE_ID(WCS_EFFECTSSUBCLASS_LIGHT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[4] = MAKE_ID(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[5] = MAKE_ID(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[6] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildMiscList(ActiveCameraListID, WCS_EFFECTSSUBCLASS_CAMERA, Active->Cameras);
	BuildMiscList(ActiveLightListID, WCS_EFFECTSSUBCLASS_LIGHT, Active->Lights);
	BuildMiscList(ActiveHazeListID, WCS_EFFECTSSUBCLASS_ATMOSPHERE, Active->Haze);
	} // if component added or name changed

if (! Done)
	ConfigureWidgets();

} // ExporterEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void ExporterEditGUI::ConfigureWidgets(void)
{
unsigned long GISFlags;
char TextStr[256];
#ifdef WCS_BUILD_VNS
long ListPos, Ct, NumEntries;
CoordSys *TestCS;
#endif // WCS_BUILD_VNS

if (! strcmp(Active->ExportTarget, "GIS"))
	GISFlags = FIOFlag_ArrowNotify;
else
	GISFlags = NULL;

sprintf(TextStr, "Scene Exporter Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCOMPRESS, &Active->ZipItUp, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTDEM, &Active->ExportTerrain, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTVECTORS, &Active->ExportVectors, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTSKYFEATURES, &ExportSkyFeatures, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTSKY, &Active->ExportSky, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTCELEST, &Active->ExportCelest, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTCLOUDS, &Active->ExportClouds, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTSTARS, &Active->ExportStars, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTATMO, &Active->ExportAtmosphere, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTVOLUME, &Active->ExportVolumetrics, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTFOLIAGE, &Active->ExportFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTLABELS, &Active->ExportLabels, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTTEXTURE, &Active->ExportTexture, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORT3DOBJECTS, &Active->Export3DObjects, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORT3DOBJECTFOL, &Active->Export3DFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTWALLS, &Active->ExportWalls, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_BURNSHADOWS, &Active->BurnShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_BURNSHADING, &Active->BurnShading, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_BURNVECTORS, &Active->BurnVectors, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTCAMERAS, &Active->ExportCameras, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTLIGHTS, &Active->ExportLights, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_EXPORTHAZE, &Active->ExportHaze, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGAPFILL, &Active->LODFillGaps, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_TILEWALLTEX, &Active->TileWallTex, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_BURNWALLSHADING, &Active->BurnWallShading, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_SQUARECELLS, &Active->SquareCells, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOASLINES, IDC_RADIOASLINES, &Active->VectorExpType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOASLINES, IDC_RADIOASPOLY2, &Active->VectorExpType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOASLINES, IDC_RADIOASPOLY3, &Active->VectorExpType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIONOFOLTEX, IDC_RADIONOFOLTEX, &Active->TextureFoliageType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIONOFOLTEX, IDC_RADIOFOLTEXINONE, &Active->TextureFoliageType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIONOFOLTEX, IDC_RADIOFOLINTWO, &Active->TextureFoliageType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIONOFOLTEX, IDC_RADIOFOLTEXINTWO, &Active->TextureFoliageType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLEDGES, &Active->BoundsType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLCENTERS, &Active->BoundsType, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIODEMPOW2, IDC_RADIODEMPOW2, &Active->DEMResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIODEMPOW2, IDC_RADIODEMPOW2PLUS1, &Active->DEMResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIODEMPOW2, IDC_RADIODEMANYSIZE, &Active->DEMResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFOLPOW2, IDC_RADIOFOLPOW2, &Active->FolResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFOLPOW2, IDC_RADIOFOLANYSIZE, &Active->FolResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOTEXPOW2, IDC_RADIOTEXPOW2, &Active->TexResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTEXPOW2, IDC_RADIOTEXPOW2PLUS1, &Active->TexResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTEXPOW2, IDC_RADIOTEXANYSIZE, &Active->TexResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOSKYPOW2, IDC_RADIOSKYPOW2, &Active->SkyResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSKYPOW2, IDC_RADIOSKYANYSIZE, &Active->SkyResOption, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFOLCROSSBD, IDC_RADIOFOLCROSSBD, &Active->FoliageStyle, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFOLCROSSBD, IDC_RADIOFOLFLIPBD, &Active->FoliageStyle, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFOLCROSSBD, IDC_RADIOFOLOTHER, &Active->FoliageStyle, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOALPHAFOL, IDC_RADIOALPHAFOL, &Active->FolTransparencyStyle, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_ALPHA, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALPHAFOL, IDC_RADIOCLIPFOL, &Active->FolTransparencyStyle, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_CLIPMAP, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFDCONST, IDC_RADIOFDCONST, &Active->FractalMethod, SRFlag_Char, WCS_FRACTALMETHOD_CONSTANT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFDCONST, IDC_RADIOFDDEPTH, &Active->FractalMethod, SRFlag_Char, WCS_FRACTALMETHOD_DEPTHMAPS, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOCOPYOBJECTS, IDC_RADIOCOPYOBJECTS, &Active->ObjectTreatment, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCOPYOBJECTS, IDC_RADIOCREATEOBJECTS, &Active->ObjectTreatment, SRFlag_Char, WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_CREATEALL, NULL, NULL);

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_DEMROWS,
 &Active->OneDEMResY,
  1.0,
   2.0,
	(double)LONG_MAX,
	 FIOFlag_Long | GISFlags,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_DEMCOLS,
 &Active->OneDEMResX,
  1.0,
   2.0,
	(double)LONG_MAX,
	 FIOFlag_Long | GISFlags,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_FOLROWS,
 &Active->FoliageRes,
  1.0,
   2.0,
	(double)Active->MaxFolRes,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TILESY,
 &Active->DEMTilesY,
  1.0,
   1.0,
	(double)Active->MaxDEMTiles,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TILESX,
 &Active->DEMTilesX,
  1.0,
   1.0,
	(double)Active->MaxDEMTiles,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TEXROWS,
 &Active->OneTexResY,
  1.0,
   2.0,
	(double)LONG_MAX,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TEXCOLS,
 &Active->OneTexResX,
  1.0,
   2.0,
	(double)LONG_MAX,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TEXTILESY,
 &Active->TexTilesY,
  1.0,
   1.0,
	(double)Active->MaxTexTiles,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TEXTILESX,
 &Active->TexTilesX,
  1.0,
   1.0,
	(double)Active->MaxTexTiles,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_SKYROWS,
 &Active->SkyRes,
  1.0,
   2.0,
	(double)Active->MaxTexRes,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_TERLODNUM,
 &Active->LODLevels,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_3DOTEXSIZE,
 &Active->Max3DOTexSize,
  1.0,
   2.0,
	(double)Active->MaxTexRes,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_WALLTEXSIZE,
 &Active->MaxWallTexSize,
  1.0,
   2.0,
	(double)Active->MaxTexRes,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_NUMBOARDS,
 &Active->NumFoliageBoards,
  1.0,
   (double)Active->MinCrossBd,
	(double)Active->MaxCrossBd,
	 FIOFlag_Long,
	  NULL,
	   0);

if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
	{
	WidgetSetText(IDC_STARTTIME, "Start Frame ");
	} // if
else
	{
	WidgetSetText(IDC_STARTTIME, "Start Time ");
	} // else

WidgetSNConfig(IDC_STARTTIME, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME]);
WidgetSNConfig(IDC_MINFOLHT, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT]);
WidgetSNConfig(IDC_VECWIDTHMULT, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT]);
WidgetSNConfig(IDC_ADDELEV, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD]);
WidgetSNConfig(IDC_TERLODDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN]);
WidgetSNConfig(IDC_TERDISDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH]);
WidgetSNConfig(IDC_FOLDISDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH]);
WidgetSNConfig(IDC_3DOBBDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX]);
WidgetSNConfig(IDC_3DODISDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH]);
WidgetSNConfig(IDC_LABELDISDIST, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH]);
WidgetSNConfig(IDC_WALLBESTSCALE, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE]);
WidgetSNConfig(IDC_3DOBESTSCALE, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE]);
WidgetSNConfig(IDC_3DOTEXSTRETCH, &Active->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH]);
WidgetSNConfig(IDC_GISWEDEMGRIDSIZE, &WEDEMCellSize);
WidgetSNConfig(IDC_GISNSDEMGRIDSIZE, &NSDEMCellSize);

WidgetSNConfig(IDC_NORTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], GISFlags);
WidgetSNConfig(IDC_WEST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], GISFlags);
WidgetSNConfig(IDC_EAST, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], GISFlags);
WidgetSNConfig(IDC_SOUTH, &Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], GISFlags);

ConfigureDD(NativeWin, IDC_TEMPPATH, (char *)Active->TempPath.GetPath(), 255, NULL, 0, IDC_LABEL_TEMP);
ConfigureDD(NativeWin, IDC_SAVEPATH, (char *)Active->OutPath.GetPath(), 255, (char *)Active->OutPath.GetName(), 63, NULL);

#ifdef WCS_RENDER_SCENARIOS
ConfigureTB(NativeWin, IDC_ADDSCENARIO, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESCENARIO, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVESCENARIOUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVESCENARIODOWN, IDI_ARROWDOWN, NULL);
#endif // WCS_RENDER_SCENARIOS
ConfigureTB(NativeWin, IDC_ADDACTION, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEACTION, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_NV_ADDITEM, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_NV_REMOVEITEM, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_TBTG_NLOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_SLOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_ELOCK, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBTG_WLOCK, IDI_ICONLOCKINTERSM, NULL);

#ifdef WCS_BUILD_VNS
if (Active->Coords)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestCS = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)CB_ERR && TestCS == Active->Coords)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
#endif // WCS_BUILD_VNS

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);

ConfigDEMCellSize();
ConfigTexCellSize();
ConfigBoundsMetric();
BuildScenarioList();
BuildMiscList(ActiveCameraListID, WCS_EFFECTSSUBCLASS_CAMERA, Active->Cameras);
BuildMiscList(ActiveLightListID, WCS_EFFECTSSUBCLASS_LIGHT, Active->Lights);
BuildMiscList(ActiveHazeListID, WCS_EFFECTSSUBCLASS_ATMOSPHERE, Active->Haze);
SyncFormatCombos();
ConfigAdvancedOptions();
DisableWidgets();

} // ExporterEditGUI::ConfigureWidgets()

/*===========================================================================*/

void ExporterEditGUI::ConfigAdvancedOptions(void)
{
char *STLUnitsText[2] = {"Millimeters", "Inches"};
char *NVENavStylesText[1] = {"Standard"};
int Units;
char DimStr[64];

// NULL out anything that might blow up if a notify is generated before this function is complete
WidgetCBClear(IDC_NVE_NAVSTYLEDROP);
WidgetCBSetCurSel(IDC_NVE_NAVSTYLEDROP, -1);
ConfigureFI(NativeWin, IDC_NVE_FOLLOWHEIGHT, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_FOLLOWMAXHEIGHT, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_INERTIA, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_NAVSPEED, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_NAVACCEL, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_NAVFRICTION, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_MAXFOLIAGESTEMS, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_NVE_MINFEATURESIZE, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECK_CONSTRAIN, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECK_MAXHTCONSTRAIN, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECK_DEFAULTSPEED, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCOMPRESS_TERRAINTEX, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCOMPRESS_FOLTEX, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECK_OPTIMIZEMOVE, NULL, 0, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECK_SHOWMAP, NULL, 0, NULL, 0);
WidgetSetText(IDC_NVE_LOGOTEXT, ""); 
WidgetSetText(IDC_NVE_WATERMARK, ""); 
WidgetSetText(IDC_NVE_NAME, ""); 
WidgetSetText(IDC_NVE_COPYRIGHT, ""); 
WidgetSetText(IDC_NVE_AUTHOR, ""); 
WidgetSetText(IDC_NVE_USER1, ""); 
WidgetSetText(IDC_NVE_USER2, ""); 
WidgetSetText(IDC_NVE_EMAIL, ""); 
ConfigureDD(NativeWin, IDC_NVE_LOGOIMAGE, NULL, 0, NULL, 0, IDC_LABEL_CMAP);

WidgetSetText(IDC_STL_BUILDDIMX, "");
WidgetSetText(IDC_STL_BUILDDIMY, "");
WidgetSetText(IDC_STL_BUILDDIMZ, "");
ConfigureFI(NativeWin, IDC_STL_VERTEXAG, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_STL_MINTHICKNESS, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureSR(NativeWin, IDC_STL_BUILDMODE_TOFIT, IDC_STL_BUILDMODE_TOFIT, NULL, 0, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_STL_BUILDMODE_TOFIT, IDC_STL_BUILDMODE_TOSCALE, NULL, 0, 0, NULL, NULL);
WidgetCBSetCurSel(IDC_STL_UNITSDROP, -1);

ConfigureSC(NativeWin, IDC_CHECK_GE_REVERSE_NORMALS, NULL, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_GE_DRAW_ORDER, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_GE_LABEL_RESCALE, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_GE_SCREENX, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureFI(NativeWin, IDC_GE_SCREENY, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
ConfigureDD(NativeWin, IDC_GE_OVERLAY, NULL, 0, NULL, 0, IDC_GE_LABEL_ICON);
WidgetSetText(IDC_GE_MESSAGE, ""); 

if (Active->FormatExtension)
	{
	// OpenFlight
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
		{
		ConfigureSC(NativeWin, IDC_CHECK_OF_CREATEFOLIAGE, &((SXExtensionOF *)Active->FormatExtension)->CreateFoliage, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_OF_INDIVIDUALFOLLOD, &((SXExtensionOF *)Active->FormatExtension)->IndividualFolLOD, SCFlag_Char, NULL, 0);
		} // OpenFlight
	// FBX
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
		{
		ConfigureSC(NativeWin, IDC_CHECK_FBX_SAVEV5, &((SXExtensionFBX *)Active->FormatExtension)->SaveV5, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_FBX_SAVEBINARY, &((SXExtensionFBX *)Active->FormatExtension)->SaveBinary, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_FBX_EMBEDMEDIA, &((SXExtensionFBX *)Active->FormatExtension)->EmbedMedia, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_FBX_USEPASSWORD, &((SXExtensionFBX *)Active->FormatExtension)->UsePassword, SCFlag_Char, NULL, 0);
		WidgetSetText(IDC_FBX_PASSWORD, ((SXExtensionFBX *)Active->FormatExtension)->Password);
		} // FBX
	// GoogleEarth
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
		{
		ConfigureSC(NativeWin, IDC_CHECK_GE_REVERSE_NORMALS, &((SXExtensionGE *)Active->FormatExtension)->Reverse3DNormals, SCFlag_Char, NULL, 0);
		ConfigureFI(NativeWin, IDC_GE_DRAW_ORDER,
		 &((SXExtensionGE *)Active->FormatExtension)->DrawOrder,
		  1.0,
		   0.0,
			99.0,
			 FIOFlag_Char,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_GE_LABEL_RESCALE,
		 &((SXExtensionGE *)Active->FormatExtension)->LabelRescale,
		  1.0,
		   0.0,
			500.0,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_GE_SCREENX,
		 &((SXExtensionGE *)Active->FormatExtension)->overlayX,
		  0.05,
		   0.0,
			1.0,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_GE_SCREENY,
		 &((SXExtensionGE *)Active->FormatExtension)->overlayY,
		  0.05,
		   0.0,
			1.0,
			 FIOFlag_Double,
			  NULL,
			   0);
		WidgetSetText(IDC_GE_MESSAGE, ((SXExtensionGE *)Active->FormatExtension)->Message);
		ConfigureDD(NativeWin, IDC_GE_OVERLAY, ((SXExtensionGE *)Active->FormatExtension)->overlayFilename.Path, WCS_PATHANDFILE_PATH_LEN_MINUSONE, ((SXExtensionGE *)Active->FormatExtension)->overlayFilename.Name, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_GE_LABEL_ICON);
		} // GoogleEarth
	// STL and VRML-STL
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
		{
		ComputeSTLScale();

		WidgetCBClear(IDC_STL_UNITSDROP);
		for (Units = 0; Units < 2; Units ++)
			{
			WidgetCBAddEnd(IDC_STL_UNITSDROP, STLUnitsText[Units]);
			} // for
		WidgetCBSetCurSel(IDC_STL_UNITSDROP, ((SXExtensionSTL *)Active->FormatExtension)->UnitOfMeasure);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimX);
		WidgetSetText(IDC_STL_BUILDDIMX, DimStr);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimY);
		WidgetSetText(IDC_STL_BUILDDIMY, DimStr);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimZ);
		WidgetSetText(IDC_STL_BUILDDIMZ, DimStr);

		ConfigureFI(NativeWin, IDC_STL_MAXDIMX,
		 &((SXExtensionSTL *)Active->FormatExtension)->MaxDimX,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureFI(NativeWin, IDC_STL_MAXDIMY,
		 &((SXExtensionSTL *)Active->FormatExtension)->MaxDimY,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureFI(NativeWin, IDC_STL_MAXDIMZ,
		 &((SXExtensionSTL *)Active->FormatExtension)->MaxDimZ,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureFI(NativeWin, IDC_STL_BUILDSCALE,
		 &((SXExtensionSTL *)Active->FormatExtension)->BuildScale,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureFI(NativeWin, IDC_STL_VERTEXAG,
		 &((SXExtensionSTL *)Active->FormatExtension)->VertExag,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureFI(NativeWin, IDC_STL_MINTHICKNESS,
		 &((SXExtensionSTL *)Active->FormatExtension)->MinThickness,
		  1.0,
		   1.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);

		ConfigureSR(NativeWin, IDC_STL_BUILDMODE_TOFIT, IDC_STL_BUILDMODE_TOFIT, &((SXExtensionSTL *)Active->FormatExtension)->BuildMode, SRFlag_Char, WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOFIT, NULL, NULL);
		ConfigureSR(NativeWin, IDC_STL_BUILDMODE_TOFIT, IDC_STL_BUILDMODE_TOSCALE, &((SXExtensionSTL *)Active->FormatExtension)->BuildMode, SRFlag_Char, WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOSCALE, NULL, NULL);
		} // STL and VRML-STL
	// NatureView
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
		{
		ConfigureFI(NativeWin, IDC_NVE_FOLLOWHEIGHT,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavFollowTerrainHeight,
		  1.0,
		   0.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_FOLLOWMAXHEIGHT,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavFollowTerrainMaxHeight,
		  1.0,
		   0.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_INERTIA,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavInertia,
		  .01,
		   0.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_NAVSPEED,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavSpeed,
		  1.0,
		   0.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_NAVACCEL,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavAcceleration,
		  1.0,
		   0.0,
			(double)FLT_MAX,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_NAVFRICTION,
		 &((SXExtensionNVE *)Active->FormatExtension)->NavFriction,
		  .01,
		   0.0,
			1.0,
			 FIOFlag_Double,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_MAXFOLIAGESTEMS,
		 &((SXExtensionNVE *)Active->FormatExtension)->LODMaxFoliageStems,
		  1.0,
		   0.0,
			10000000.0,
			 FIOFlag_Long,
			  NULL,
			   0);
		ConfigureFI(NativeWin, IDC_NVE_MINFEATURESIZE,
		 &((SXExtensionNVE *)Active->FormatExtension)->LODMinFeatureSizePixels,
		  1.0,
		   0.0,
			100000.0,
			 FIOFlag_Long,
			  NULL,
			   0);

		ConfigureSC(NativeWin, IDC_CHECK_CONSTRAIN, &((SXExtensionNVE *)Active->FormatExtension)->NavConstrain, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_MAXHTCONSTRAIN, &((SXExtensionNVE *)Active->FormatExtension)->NavMaxHtConstrain, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_DEFAULTSPEED, &((SXExtensionNVE *)Active->FormatExtension)->NavUseDefaultSpeed, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECKCOMPRESS_TERRAINTEX, &((SXExtensionNVE *)Active->FormatExtension)->LODCompressTerrainTex, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECKCOMPRESS_FOLTEX, &((SXExtensionNVE *)Active->FormatExtension)->LODCompressFoliageTex, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_OPTIMIZEMOVE, &((SXExtensionNVE *)Active->FormatExtension)->LODOptimizeMove, SCFlag_Char, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECK_SHOWMAP, &((SXExtensionNVE *)Active->FormatExtension)->OverlayShowMap, SCFlag_Char, NULL, 0);

		WidgetCBClear(IDC_NVE_NAVSTYLEDROP);
		for (Units = 0; Units < 1; Units ++)
			{
			WidgetCBAddEnd(IDC_NVE_NAVSTYLEDROP, NVENavStylesText[Units]);
			} // for
		WidgetCBSetCurSel(IDC_NVE_NAVSTYLEDROP, ((SXExtensionNVE *)Active->FormatExtension)->NavStyle);

		WidgetSetText(IDC_NVE_LOGOTEXT, ((SXExtensionNVE *)Active->FormatExtension)->OverlayLogoText ?
			((SXExtensionNVE *)Active->FormatExtension)->OverlayLogoText: "");
		WidgetSetText(IDC_NVE_WATERMARK, ((SXExtensionNVE *)Active->FormatExtension)->WatermarkText ? 
			((SXExtensionNVE *)Active->FormatExtension)->WatermarkText: "");
		WidgetSetText(IDC_NVE_NAME, ((SXExtensionNVE *)Active->FormatExtension)->MetaName ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaName: "");
		WidgetSetText(IDC_NVE_COPYRIGHT, ((SXExtensionNVE *)Active->FormatExtension)->MetaCopyright ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaCopyright: "");
		WidgetSetText(IDC_NVE_AUTHOR, ((SXExtensionNVE *)Active->FormatExtension)->MetaAuthor ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaAuthor: "");
		WidgetSetText(IDC_NVE_USER1, ((SXExtensionNVE *)Active->FormatExtension)->MetaUser1 ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaUser1: "");
		WidgetSetText(IDC_NVE_USER2, ((SXExtensionNVE *)Active->FormatExtension)->MetaUser2 ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaUser2: "");
		WidgetSetText(IDC_NVE_EMAIL, ((SXExtensionNVE *)Active->FormatExtension)->MetaEmail ? 
			((SXExtensionNVE *)Active->FormatExtension)->MetaEmail: "");
		WidgetSetModified(IDC_NVE_LOGOTEXT, FALSE);
		WidgetSetModified(IDC_NVE_WATERMARK, FALSE);
		WidgetSetModified(IDC_NVE_NAME, FALSE);
		WidgetSetModified(IDC_NVE_COPYRIGHT, FALSE);
		WidgetSetModified(IDC_NVE_AUTHOR, FALSE);
		WidgetSetModified(IDC_NVE_EMAIL, FALSE);
		WidgetSetModified(IDC_NVE_USER1, FALSE);
		WidgetSetModified(IDC_NVE_USER2, FALSE);
		ConfigureDD(NativeWin, IDC_NVE_LOGOIMAGE, ((SXExtensionNVE *)Active->FormatExtension)->OverlayLogoFileName.Path, WCS_PATHANDFILE_PATH_LEN_MINUSONE, ((SXExtensionNVE *)Active->FormatExtension)->OverlayLogoFileName.Name, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
		} // NatureView
	} // if
ConfigureActiveAction();

} // ExporterEditGUI::ConfigAdvancedOptions

/*===========================================================================*/

void ExporterEditGUI::BuildActionItemList(void)
{
RasterAnimHostList *Current;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
RasterAnimHost **SelectedItems = NULL;

if (ActiveAction)
	{
	Current = ActiveAction->Items;
	NumListItems = WidgetLBGetCount(IDC_NV_ITEMLIST);

	for (TempCt = 0; TempCt < NumListItems; TempCt ++)
		{
		if (WidgetLBGetSelState(IDC_NV_ITEMLIST, TempCt))
			{
			NumSelected ++;
			} // if
		} // for

	if (NumSelected)
		{
		if (SelectedItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), 0))
			{
			for (TempCt = 0; TempCt < NumListItems; TempCt ++)
				{
				if (WidgetLBGetSelState(IDC_NV_ITEMLIST, TempCt))
					{
					SelectedItems[SelCt ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_NV_ITEMLIST, TempCt);
					} // if
				} // for
			} // if
		} // if

	while (Current || Ct < NumListItems)
		{
		CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_NV_ITEMLIST, Ct): NULL;
		
		if (Current)
			{
			if (Current->Me)
				{
				if (Current->Me == CurrentRAHost)
					{
					BuildActionItemListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_NV_ITEMLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_NV_ITEMLIST, Ct, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_NV_ITEMLIST, 1, Ct);
								break;
								} // if
							} // for
						} // if
					Ct ++;
					} // if
				else
					{
					FoundIt = 0;
					for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
						{
						if (Current->Me == (RasterAnimHost *)WidgetLBGetItemData(IDC_NV_ITEMLIST, TempCt))
							{
							FoundIt = 1;
							break;
							} // if
						} // for
					if (FoundIt)
						{
						BuildActionItemListEntry(ListName, Current->Me);
						WidgetLBReplace(IDC_NV_ITEMLIST, TempCt, ListName);
						WidgetLBSetItemData(IDC_NV_ITEMLIST, TempCt, Current->Me);
						if (SelectedItems)
							{
							for (SelCt = 0; SelCt < NumSelected; SelCt ++)
								{
								if (SelectedItems[SelCt] == Current->Me)
									{
									WidgetLBSetSelState(IDC_NV_ITEMLIST, 1, TempCt);
									break;
									} // if
								} // for
							} // if
						for (TempCt -- ; TempCt >= Ct; TempCt --)
							{
							WidgetLBDelete(IDC_NV_ITEMLIST, TempCt);
							NumListItems --;
							} // for
						Ct ++;
						} // if
					else
						{
						BuildActionItemListEntry(ListName, Current->Me);
						Place = WidgetLBInsert(IDC_NV_ITEMLIST, Ct, ListName);
						WidgetLBSetItemData(IDC_NV_ITEMLIST, Place, Current->Me);
						if (SelectedItems)
							{
							for (SelCt = 0; SelCt < NumSelected; SelCt ++)
								{
								if (SelectedItems[SelCt] == Current->Me)
									{
									WidgetLBSetSelState(IDC_NV_ITEMLIST, 1, Place);
									break;
									} // if
								} // for
							} // if
						NumListItems ++;
						Ct ++;
						} // else
					} // if
				} // if
			Current = Current->Next;
			} // if
		else
			{
			WidgetLBDelete(IDC_NV_ITEMLIST, Ct);
			NumListItems --;
			} // else
		} // while

	if (SelectedItems)
		AppMem_Free(SelectedItems, NumSelected * sizeof (RasterAnimHost *));
	} // if

} // ExporterEditGUI::BuildActionItemList

/*===========================================================================*/

void ExporterEditGUI::BuildActionItemListEntry(char *ListName, RasterAnimHost *Me)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;

Me->GetRAHostProperties(&Prop);
if (Prop.Name && Prop.Name[0])
	{
	strcpy(ListName, "* ");
	strcat(ListName, Prop.Name);
	if (Prop.Type && Prop.Type[0])
		{
		strcat(ListName, " ");
		strcat(ListName, Prop.Type);
		} // if
	} // if
else
	strcat(ListName, "  ");

} // ExporterEditGUI::BuildActionItemListEntry()

/*===========================================================================*/

void ExporterEditGUI::ClearActionItemList(void)
{

WidgetLBClear(IDC_NV_ITEMLIST);

} // ClearActionItemList

/*===========================================================================*/

char *NVOverlayLogoInserts[] = 
	{
	"Author Field",
	"Email Address Field",
	"Copyright Notice Field",
	"Note 1 Field",
	"Note 2 field",
	"Scene Exporter Name",
	"Viewer's Date and Time",
	"Project Name",
	"Camera Name",
	"Camera Bank (degrees)",
	"Camera Heading (degrees)",
	"Camera Hor. Field of View",
	"Camera X (longitude)",
	"Camera Y (latitude)",
	"Camera Z (absolute elev)",
	"Cam Elev above Ground",
	"New Line (line break)",
	"Ampersand (&)",
	NULL
	}; // NVOverlayLogoInserts

char *NVOverlayLogoInsertCharacters[] = 
	{
	"&UN",
	"&UE",
	"&UC",
	"&U1",
	"&U2",
	"&RN",
	"&RD",
	"&PN",
	"&CN",
	"&CB",
	"&CH",
	"&CF",
	"&CX",
	"&CY",
	"&CZ",
	"&CE",
	"&*",
	"&&",
	NULL
	}; // NVOverlayLogoInsertCharacters

char *QueryActionInserts[] = 
	{
	"New Line (line break)",
	"Object Name",
	"Vector Name or Label",
	"Vector Label",
	/*
	"Author Field",
	"Email Address Field",
	"Copyright Notice Field",
	"Note 1 Field",
	"Note 2 field",
	"Scene Exporter Name",
	"Viewer's Date and Time",
	"Project Name",
	*/
	"Ampersand (&)",
	NULL
	}; // QueryActionInserts

char *QueryActionInsertCharacters[] = 
	{
	"&*",
	"&ON",
	"&VN",
	"&VL",
	/*
	"&UN",
	"&UE",
	"&UC",
	"&U1",
	"&U2",
	"&RN",
	"&RD",
	"&PN",
	"&CN",
	"&CB",
	"&CH",
	"&CF",
	"&CX",
	"&CY",
	"&CZ",
	"&CE",
	*/
	"&&",
	NULL
	}; // QueryActionInsertCharacters

/*===========================================================================*/

void ExporterEditGUI::FillNVLogoInsertCombo(void)
{
long ItemCt;
LayerEntry *Entry;
const char *LayerName;

WidgetCBClear(IDC_NV_ITEMINSERT);
WidgetCBClear(IDC_NV_ITEMINSERT2);
WidgetCBClear(IDC_NV_ITEMINSERT3);

for (ItemCt = 0; NVOverlayLogoInserts[ItemCt]; ItemCt ++)
	{
	WidgetCBAddEnd(IDC_NV_ITEMINSERT, NVOverlayLogoInserts[ItemCt]);
	} // for
for (ItemCt = 0; QueryActionInserts[ItemCt]; ItemCt ++)
	{
	WidgetCBAddEnd(IDC_NV_ITEMINSERT2, QueryActionInserts[ItemCt]);
	} // for

// list of DB attributes
Entry = DBHost->DBLayers.FirstEntry();
while (Entry)
	{
	if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		LayerName = Entry->GetName();
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL || LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
			{
			ItemCt = WidgetCBInsert(IDC_NV_ITEMINSERT3, -1, &LayerName[1]);
			} // if an attribute layer
		} // if
	Entry = DBHost->DBLayers.NextEntry(Entry);
	} // while

} // ExporterEditGUI::FillNVLogoInsertCombo

/*===========================================================================*/

char *ExporterEditGUI::ActionTypes[WCS_SXQUERYACTION_NUMACTIONTYPES] = 
	{
	"Display Text as Non-persistent Overlay",
	"Display Text in Persistent Window",
	"View Text File in Persistent Window",
	"Display Image in Default Viewer",
	"Display Image in Scene Viewer",
	"Play Sound",
	"Display Web page in Default Browser",
	"Display Web page in Scene Viewer",
	"Play Media File in Default Media Player",
	"Play Media File in Scene Viewer",
	"Highlight Object",
	"Load New Scene File",
	"Run Shell Command",
	"Launch User-defined Plug-in",
	}; // ActionTypes

int ExporterEditGUI::ActionTypeEnums[WCS_SXQUERYACTION_NUMACTIONTYPES] = 
	{
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE,
	WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET,
	WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE,
	WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND,
	WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN
	}; // ActionTypeEnums

/*===========================================================================*/

char *ExporterEditGUI::TranslateActionName(char ActionType)
{
long ItemCt;

for (ItemCt = 0; ItemCt < WCS_SXQUERYACTION_NUMACTIONTYPES; ItemCt ++)
	{
	if (ActionTypeEnums[ItemCt] == ActionType)
		return (ActionTypes[ItemCt]);
	} // for

return (NULL);

} // ExporterEditGUI::TranslateActionName

/*===========================================================================*/

void ExporterEditGUI::FillActionTypeCombo(void)
{
long ItemCt, ItemPos;
char *QResult;

WidgetCBClear(IDC_NV_ACTIONTYPE);

// <<<>>>GE filter for GE formats only
for (ItemCt = 0; ItemCt < WCS_SXQUERYACTION_NUMACTIONTYPES; ItemCt ++)
	{
	if (ActionTypeEnums[ItemCt] == WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND || ActionTypeEnums[ItemCt] == WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN)
		{
		if (QResult = GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("nvx_options"))
			{
			if (tolower(QResult[0]) != 'y')
				{
				continue;
				} // 
			} // if
		else
			continue;
		} // if
	if (Active->ApproveActionAvailable(ActionTypeEnums[ItemCt]))
		{
		ItemPos = WidgetCBInsert(IDC_NV_ACTIONTYPE, -1, ActionTypes[ItemCt]);
		WidgetCBSetItemData(IDC_NV_ACTIONTYPE, ItemPos, (void *)ActionTypeEnums[ItemCt]);
		} // if
	} // for

} // ExporterEditGUI::FillActionTypeCombo

/*===========================================================================*/

static long ActionItems[] = {
	WCS_RAHOST_OBJTYPE_VECTOR,
	WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_FOLIAGE,
	WCS_EFFECTSSUBCLASS_FENCE,
	WCS_EFFECTSSUBCLASS_LABEL,
	0};

/*===========================================================================*/

void ExporterEditGUI::FillActionItemCombo(void)
{
long ItemCt, Ct;

if (Active->ApproveActionItemAvailable(ActionItems[0]))
	{
	ItemCt = WidgetCBInsert(IDC_NV_CLASSDROP, -1, "Vector");
	WidgetCBSetItemData(IDC_NV_CLASSDROP, ItemCt, (void *)ActionItems[0]);
	} // if

Ct = 1;
while (ActionItems[Ct] > 0)
	{
	if (Active->ApproveActionItemAvailable(ActionItems[Ct]))
		{
		ItemCt = WidgetCBInsert(IDC_NV_CLASSDROP, -1, EffectsHost->GetEffectTypeNameNonPlural(ActionItems[Ct]));
		WidgetCBSetItemData(IDC_NV_CLASSDROP, ItemCt, (void *)ActionItems[Ct]);
		} // if
	Ct ++;
	} // for
WidgetCBSetCurSel(IDC_NV_CLASSDROP, 0);
HideActionOptions();

} // ExporterEditGUI::FillActionItemCombo

/*===========================================================================*/

void ExporterEditGUI::HideActionOptions(void)
{

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction())
		{
		if (ActiveAction)
			{
			switch (ActiveAction->ActionType)
				{
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL:	//"Display Label",
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE:	//"Display Data Table",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Text");
					break;
					} // Display Label
				case WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE:	//"View Text File",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Text File");
					break;
					} // View Text File
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE:	//"Display Image",
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL:	//"Display Image in NV",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Image File");
					break;
					} // Display Image
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE:	//"Display Web Page",
				case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL:	//"Display Web Page",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Web Address");
					break;
					} // Display Web Page
				case WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE:	//"Play Media File",
				case WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL:	//"Play Media File",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Media File");
					break;
					} // Play Media File
				case WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET:	//"Highlight Object Set",
					{
					WidgetShow(IDC_TEXT_LABEL, FALSE);
					WidgetShow(IDC_NV_ACTIONTEXT, FALSE);
					break;
					} // Highlight Object Set
				case WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE:	//"Load New NatureView Scene File",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Scene File");
					break;
					} // Load New NatureView Scene File
				case WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND:	//"Run Shell Command",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Command");
					break;
					} // Run Shell Command
				case WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN:	//"Launch Plug-in",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Argument");
					break;
					} // Launch Plug-in
				case WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL:	//"Play Sound",
					{
					WidgetShow(IDC_TEXT_LABEL, TRUE);
					WidgetShow(IDC_NV_ACTIONTEXT, TRUE);
					WidgetSetText(IDC_TEXT_LABEL, "Sound File");
					break;
					} // Play Sound
				} // if
			} // if
		} // if
	} // if

} // ExporterEditGUI::HideActionOptions

/*===========================================================================*/

void ExporterEditGUI::InsertNVLogo(void)
{
HWND Wid;
int LabelInsert;

if ((LabelInsert = WidgetCBGetCurSel(IDC_NV_ITEMINSERT)) != -1)
	{
	if (Wid = GetWidgetFromID(IDC_NVE_LOGOTEXT))
		{
		SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)NVOverlayLogoInsertCharacters[LabelInsert]);
		RecordNVETextChange(IDC_NVE_LOGOTEXT);
		} // if
	} // if

} // ExporterEditGUI::InsertNVLogo

/*===========================================================================*/

void ExporterEditGUI::InsertActionText(void)
{
HWND Wid;
int LabelInsert;

if ((LabelInsert = WidgetCBGetCurSel(IDC_NV_ITEMINSERT2)) != -1)
	{
	if (Wid = GetWidgetFromID(IDC_NV_ACTIONTEXT))
		{
		SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)QueryActionInsertCharacters[LabelInsert]);
		RecordActionTextChange(IDC_NV_ACTIONTEXT);
		} // if
	} // if

} // ExporterEditGUI::InsertActionText

/*===========================================================================*/

void ExporterEditGUI::InsertActionAttrib(void)
{
HWND Wid;
int LabelInsert;
char AttribName[256], TextReplace[256];

if ((LabelInsert = WidgetCBGetCurSel(IDC_NV_ITEMINSERT3)) != -1)
	{
	if (Wid = GetWidgetFromID(IDC_NV_ACTIONTEXT))
		{
		WidgetCBGetText(IDC_NV_ITEMINSERT3, LabelInsert, AttribName);
		sprintf(TextReplace, "%s%s%s", Attrib_TextSymbol, AttribName, Attrib_TextSymbol);
		SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)TextReplace);
		RecordActionTextChange(IDC_NV_ACTIONTEXT);
		} // if
	} // if

} // ExporterEditGUI::InsertActionAttrib

/*===========================================================================*/

void ExporterEditGUI::SyncFormatCombos(void)
{
long SetTarget, Pos, Count;
char FetchText[256];

// texture format
SetTarget = -1;
Count = WidgetCBGetCount(IDC_FORMATDROP);

for (Pos = 0; Pos < Count; Pos ++)
	{
	WidgetCBGetText(IDC_FORMATDROP, Pos, FetchText);
	if (! stricmp(FetchText, Active->ImageFormat))
		SetTarget = Pos;
	} // for
WidgetCBSetCurSel(IDC_FORMATDROP, SetTarget);

// foliage format
SetTarget = -1;
Count = WidgetCBGetCount(IDC_FOLFORMATDROP);

for (Pos = 0; Pos < Count; Pos ++)
	{
	WidgetCBGetText(IDC_FOLFORMATDROP, Pos, FetchText);
	if (! stricmp(FetchText, Active->FoliageImageFormat))
		SetTarget = Pos;
	} // for
WidgetCBSetCurSel(IDC_FOLFORMATDROP, SetTarget);

// target format
SetTarget = -1;
Count = WidgetCBGetCount(IDC_TARGETDROP);

for (Pos = 0; Pos < Count; Pos ++)
	{
	WidgetCBGetText(IDC_TARGETDROP, Pos, FetchText);
	if (! stricmp(FetchText, Active->ExportTarget))
		SetTarget = Pos;
	} // for
WidgetCBSetCurSel(IDC_TARGETDROP, SetTarget);

} // ExporterEditGUI::SyncFormatCombos

/*===========================================================================*/

void ExporterEditGUI::BuildScenarioList(void)
{
#ifdef WCS_RENDER_SCENARIOS
EffectList *Current = Active->Scenarios;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_SCENARIOLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_SCENARIOLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (GeneralEffect **)AppMem_Alloc(NumSelected * sizeof (GeneralEffect *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildScenarioListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_SCENARIOLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_SCENARIOLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_SCENARIOLIST, 1, Ct);
							break;
							} // if
						} // for
					} // if
				Ct ++;
				} // if
			else
				{
				FoundIt = 0;
				for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
					{
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildScenarioListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_SCENARIOLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_SCENARIOLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SCENARIOLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_SCENARIOLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildScenarioListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_SCENARIOLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_SCENARIOLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_SCENARIOLIST, 1, Place);
								break;
								} // if
							} // for
						} // if
					NumListItems ++;
					Ct ++;
					} // else
				} // if
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_SCENARIOLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));
#endif // WCS_RENDER_SCENARIOS

} // ExporterEditGUI::BuildScenarioList

/*===========================================================================*/

void ExporterEditGUI::BuildScenarioListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // ExporterEditGUI::BuildScenarioListEntry()

/*===========================================================================*/

void ExporterEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCOMPRESS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTDEM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTVECTORS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTSKYFEATURES, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTSKY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTCELEST, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTCLOUDS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTSTARS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTATMO, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTVOLUME, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTFOLIAGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTLABELS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTTEXTURE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORT3DOBJECTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORT3DOBJECTFOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTWALLS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_BURNSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_BURNSHADING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_BURNVECTORS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTCAMERAS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTLIGHTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_EXPORTHAZE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGAPFILL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_TILEWALLTEX, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_BURNWALLSHADING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_SQUARECELLS, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOASLINES, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOASPOLY2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOASPOLY3, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIONOFOLTEX, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLTEXINONE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLINTWO, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLTEXINTWO, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCELLEDGES, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCELLCENTERS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIODEMPOW2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIODEMPOW2PLUS1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIODEMANYSIZE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLPOW2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLANYSIZE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOTEXPOW2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOTEXPOW2PLUS1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOTEXANYSIZE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSKYPOW2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSKYANYSIZE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLCROSSBD, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLFLIPBD, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFOLOTHER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALPHAFOL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCLIPFOL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFDCONST, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFDDEPTH, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCOPYOBJECTS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCREATEOBJECTS, WP_SRSYNC_NONOTIFY);

WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_DEMCOLS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FOLROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TILESY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TILESX, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TEXROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TEXCOLS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TEXTILESY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TEXTILESX, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_SKYROWS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TERLODNUM, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WALLTEXSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_3DOTEXSIZE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NUMBOARDS, WP_FISYNC_NONOTIFY);

WidgetSNSync(IDC_STARTTIME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINFOLHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VECWIDTHMULT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ADDELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TERLODDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TERDISDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOLDISDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_3DOBBDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_3DODISDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LABELDISDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WALLBESTSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_3DOBESTSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_3DOTEXSTRETCH, WP_FISYNC_NONOTIFY);

WidgetFISync(IDC_STL_BUILDSCALE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_STL_MAXDIMX, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_STL_MAXDIMY, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_STL_MAXDIMZ, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_STL_VERTEXAG, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_STL_MINTHICKNESS, WP_FISYNC_NONOTIFY);
WidgetSRSync(IDC_STL_BUILDMODE_TOFIT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_STL_BUILDMODE_TOSCALE, WP_SRSYNC_NONOTIFY);

WidgetFISync(IDC_NVE_FOLLOWHEIGHT, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_FOLLOWMAXHEIGHT, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_INERTIA, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_NAVSPEED, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_NAVACCEL, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_NAVFRICTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_MAXFOLIAGESTEMS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_NVE_MINFEATURESIZE, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECK_CONSTRAIN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_MAXHTCONSTRAIN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_DEFAULTSPEED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCOMPRESS_TERRAINTEX, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCOMPRESS_FOLTEX, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_OPTIMIZEMOVE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_SHOWMAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_OF_CREATEFOLIAGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_OF_INDIVIDUALFOLLOD, WP_SCSYNC_NONOTIFY);

WidgetSCSync(IDC_CHECK_FBX_SAVEV5, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_FBX_SAVEBINARY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_FBX_EMBEDMEDIA, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK_FBX_USEPASSWORD, WP_SCSYNC_NONOTIFY);

WidgetSCSync(IDC_CHECK_GE_REVERSE_NORMALS, WP_SCSYNC_NONOTIFY);
WidgetFISync(IDC_GE_DRAW_ORDER, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_GE_LABEL_RESCALE, WP_FISYNC_NONOTIFY);

} // ExporterEditGUI::SyncWidgets

/*===========================================================================*/

void ExporterEditGUI::DisableWidgets(void)
{
long ActiveFormat, IsItEnabled, IsTerrainEnabled;
char Str[256], disability, FormatIsGIS;

if ((ActiveFormat = WidgetCBGetCurSel(IDC_TARGETDROP)) != CB_ERR)
	{
	WidgetCBGetText(IDC_TARGETDROP, ActiveFormat, Str);

	if ((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Str)) >= 0)
		{
		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "ZIPRESULT");
		WidgetSetDisabled(IDC_CHECKCOMPRESS, ! IsItEnabled);

		if (ExpTable.IsFieldValueTrue(ActiveFormat, "DEMSQUARE"))
			{
			DEMSquare = 1;
			WidgetSetText(IDC_DEMCOLS, "Size ");
			} // if
		else
			{
			WidgetSetText(IDC_DEMCOLS, "Columns ");
			DEMSquare = 0;
			} // else
		IsItEnabled = IsTerrainEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "TERRAIN");
		WidgetSetDisabled(IDC_EXPORTDEM, ! IsItEnabled);
		WidgetSetDisabled(IDC_DEMCOLS, ! (Active->ExportTerrain && IsItEnabled));
		WidgetSetDisabled(IDC_DEMROWS, ! (Active->ExportTerrain && IsItEnabled && ! DEMSquare));
		WidgetSetDisabled(IDC_TILESX, ! (Active->ExportTerrain && IsItEnabled && Active->MaxDEMTiles > 1));
		WidgetSetDisabled(IDC_TILESY, ! (Active->ExportTerrain && IsItEnabled && Active->MaxDEMTiles > 1));
		WidgetSetDisabled(IDC_RADIOFDCONST, ! (Active->ExportTerrain && IsItEnabled));
		WidgetSetDisabled(IDC_RADIOFDDEPTH, ! (Active->ExportTerrain && IsItEnabled));
		WidgetSetDisabled(IDC_RADIODEMPOW2, ! (Active->ExportTerrain && IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "POW2"))));
		WidgetSetDisabled(IDC_RADIODEMPOW2PLUS1, ! (Active->ExportTerrain && IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "POW2P1"))));
		WidgetSetDisabled(IDC_RADIODEMANYSIZE, ! (Active->ExportTerrain && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "ANY")));
		WidgetSetDisabled(IDC_SQUARECELLS, ! (Active->ExportTerrain && IsItEnabled && ExpTable.IsFieldValueTrue(ActiveFormat, "SQUARECELL")));

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "TEXTURE");
		WidgetSetDisabled(IDC_EXPORTTEXTURE, ! IsItEnabled);
		WidgetSetDisabled(IDC_TEXCOLS, ! (Active->ExportTexture && IsItEnabled));
		WidgetSetDisabled(IDC_TEXROWS, ! (Active->ExportTexture && IsItEnabled));
		WidgetSetDisabled(IDC_TEXTILESX, ! (Active->ExportTexture && IsItEnabled && Active->MaxTexTiles > 1 && ! (Active->EqualTiles && IsTerrainEnabled && Active->ExportTerrain)));
		WidgetSetDisabled(IDC_TEXTILESY, ! (Active->ExportTexture && IsItEnabled && Active->MaxTexTiles > 1 && ! (Active->EqualTiles && IsTerrainEnabled && Active->ExportTerrain)));
		WidgetSetDisabled(IDC_FORMATDROP, ! 
			((Active->ExportTexture && IsItEnabled) || 
			Active->ExportWalls || 
			Active->Export3DObjects || 
			Active->Export3DFoliage || 
			ExportSkyFeatures));

		WidgetSetDisabled(IDC_BURNVECTORS, ! (ExpTable.IsFieldValueTrue(ActiveFormat, "BURNVECTOR") && Active->ExportTexture && IsItEnabled));
		WidgetSetDisabled(IDC_BURNSHADING, ! (Active->ExportTexture && IsItEnabled));
		WidgetSetDisabled(IDC_BURNSHADOWS, ! (Active->ExportTexture && Active->BurnShading && IsItEnabled));
		WidgetSetDisabled(IDC_RADIOTEXPOW2, ! (Active->ExportTexture && IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "POW2"))));
		WidgetSetDisabled(IDC_RADIOTEXPOW2PLUS1, ! (Active->ExportTexture && IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "POW2P1"))));
		WidgetSetDisabled(IDC_RADIOTEXANYSIZE, ! (Active->ExportTexture && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "ANY")));

		IsItEnabled = ((Active->BurnVectors && ExpTable.IsFieldValueTrue(ActiveFormat, "BURNVECTOR") && Active->ExportTexture && ExpTable.IsFieldValueTrue(ActiveFormat, "TEXTURE"))
			|| (Active->ExportVectors && ExpTable.IsFieldValueTrue(ActiveFormat, "VECTOR")));
		WidgetSetDisabled(IDC_VECWIDTHMULT, ! (IsItEnabled));
		WidgetEMSetReadOnly(IDC_VECWIDTHMULT, ! (IsItEnabled));
		WidgetSetDisabled(IDC_ADDELEV, ! (IsItEnabled));
		WidgetEMSetReadOnly(IDC_ADDELEV, ! (IsItEnabled));

		IsItEnabled = 
			(ExpTable.IsFieldValueTrue(ActiveFormat, "SKY") || 
			ExpTable.IsFieldValueTrue(ActiveFormat, "CLOUD") || 
			ExpTable.IsFieldValueTrue(ActiveFormat, "CELEST") || 
			ExpTable.IsFieldValueTrue(ActiveFormat, "STAR") || 
			(ExpTable.IsFieldValueTrue(ActiveFormat, "ATMOSPHERE") &&
			ExpTable.IsFieldValueTrue(ActiveFormat, "VOLUMETRIC")));
		WidgetSetDisabled(IDC_EXPORTSKYFEATURES, ! (IsItEnabled));
		IsItEnabled = (IsItEnabled && ExportSkyFeatures && (Active->ExportSky || Active->ExportClouds || Active->ExportCelest || Active->ExportStars || 
			(Active->ExportAtmosphere && Active->ExportVolumetrics)));
		WidgetSetDisabled(IDC_RADIOSKYPOW2, ! (IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "SKYRESOPT", "POW2"))));
		WidgetSetDisabled(IDC_RADIOSKYANYSIZE, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "SKYRESOPT", "ANY")));
		WidgetSetDisabled(IDC_SKYROWS, ! (IsItEnabled));
		WidgetEMSetReadOnly(IDC_SKYROWS, ! (IsItEnabled));
		WidgetSetDisabled(IDC_EXPORTSKY, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "SKY")));
		WidgetSetDisabled(IDC_EXPORTCLOUDS, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "CLOUD")));
		WidgetSetDisabled(IDC_EXPORTCELEST, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "CELEST")));
		WidgetSetDisabled(IDC_EXPORTSTARS, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "STAR")));
		WidgetSetDisabled(IDC_EXPORTATMO, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "ATMOSPHERE")));
		WidgetSetDisabled(IDC_EXPORTVOLUME, ! (ExportSkyFeatures && ExpTable.IsFieldValueTrue(ActiveFormat, "VOLUMETRIC")));

		WidgetSetDisabled(IDC_EXPORTVECTORS, ! ExpTable.IsFieldValueTrue(ActiveFormat, "VECTOR"));
		WidgetSetDisabled(IDC_EXPORT3DOBJECTS, ! ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJ"));
		WidgetSetDisabled(IDC_EXPORT3DOBJECTFOL, ! ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJ"));
		IsItEnabled = (ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJ") && (Active->Export3DObjects || Active->Export3DFoliage));
		WidgetSetDisabled(IDC_RADIOCOPYOBJECTS, ! (IsItEnabled && ExpTable.IsFieldValueTrue(ActiveFormat, "OBJCOPY")));
		WidgetSetDisabled(IDC_RADIOCREATEOBJECTS, ! IsItEnabled);
		WidgetSetDisabled(IDC_3DOTEXSIZE, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_3DOTEXSIZE, ! IsItEnabled);
		WidgetSetDisabled(IDC_3DOBESTSCALE, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_3DOBESTSCALE, ! IsItEnabled);
		WidgetSetDisabled(IDC_3DOTEXSTRETCH, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_3DOTEXSTRETCH, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "WALL");
		WidgetSetDisabled(IDC_EXPORTWALLS, ! IsItEnabled);
		WidgetSetDisabled(IDC_WALLBESTSCALE, ! (IsItEnabled && Active->ExportWalls));
		WidgetEMSetReadOnly(IDC_WALLBESTSCALE, ! (IsItEnabled && Active->ExportWalls));
		WidgetSetDisabled(IDC_WALLTEXSIZE, ! (IsItEnabled && Active->ExportWalls));
		WidgetEMSetReadOnly(IDC_WALLTEXSIZE, ! (IsItEnabled && Active->ExportWalls));
		WidgetSetDisabled(IDC_TILEWALLTEX, ! (IsItEnabled && Active->ExportWalls));
		WidgetSetDisabled(IDC_BURNWALLSHADING, ! (IsItEnabled && Active->ExportWalls));

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "FOLIAGE");
		WidgetSetDisabled(IDC_EXPORTFOLIAGE, ! IsItEnabled);
		WidgetSetDisabled(IDC_EXPORTLABELS, ! IsItEnabled);
		WidgetSetDisabled(IDC_MINFOLHT, ! (Active->ExportFoliage && IsItEnabled));
		WidgetEMSetReadOnly(IDC_MINFOLHT, ! (Active->ExportFoliage && IsItEnabled));
		WidgetSetDisabled(IDC_FOLFORMATDROP, ! (Active->ExportFoliage && IsItEnabled));
		WidgetSetDisabled(IDC_FOLROWS, ! (Active->ExportFoliage && IsItEnabled));
		WidgetEMSetReadOnly(IDC_FOLROWS, ! (Active->ExportFoliage && IsItEnabled));
		WidgetSetDisabled(IDC_RADIOFOLPOW2, ! (Active->ExportFoliage && IsItEnabled && (ExpTable.IsOptionSupported(ActiveFormat, "FOLRESOPT", "POW2"))));
		WidgetSetDisabled(IDC_RADIOFOLANYSIZE, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLRESOPT", "ANY")));
		WidgetSetDisabled(IDC_RADIOFOLFLIPBD, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "FLIP")));
		WidgetSetDisabled(IDC_RADIOFOLCROSSBD, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "CROSS")));
		WidgetSetDisabled(IDC_RADIOFOLOTHER, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "OTHER")));
		disability = (Active->ExportFoliage && IsItEnabled && Active->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "CROSS")
			&& Active->FoliageAsObj && Active->MaxCrossBd != Active->MinCrossBd);
		WidgetSetDisabled(IDC_NUMBOARDS, ! disability);
		WidgetEMSetReadOnly(IDC_NUMBOARDS, ! disability);
		WidgetSetDisabled(IDC_RADIOALPHAFOL, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLALPHA", "ALPHA")));
		WidgetSetDisabled(IDC_RADIOCLIPFOL, ! (Active->ExportFoliage && IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLALPHA", "CLIP")));

		WidgetShow(IDC_RADIOFOLOTHER, IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "OTHER"));
		WidgetShow(IDC_HDILEARNMORE, IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "OTHER"));

		IsItEnabled = (Active->ExportVectors && ExpTable.IsFieldValueTrue(ActiveFormat, "VECTOR"));
		WidgetSetDisabled(IDC_RADIOASLINES, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "LINES")));
		WidgetSetDisabled(IDC_RADIOASPOLY2, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "2PT")));
		WidgetSetDisabled(IDC_RADIOASPOLY3, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "3PT")));

		IsItEnabled = (Active->ExportTexture && ExpTable.IsFieldValueTrue(ActiveFormat, "TEXTURE"));
		WidgetSetDisabled(IDC_RADIONOFOLTEX, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "NOFOL")));
		WidgetSetDisabled(IDC_RADIOFOLTEXINONE, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "FOLIN1")));
		WidgetSetDisabled(IDC_RADIOFOLINTWO, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "FOLIN2")));
		WidgetSetDisabled(IDC_RADIOFOLTEXINTWO, ! (IsItEnabled && ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "BOTHIN2")));

		WidgetSetDisabled(IDC_RADIOCELLEDGES, ! ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "EDGE"));
		WidgetSetDisabled(IDC_RADIOCELLCENTERS, ! ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "CENTER"));

		IsItEnabled = (ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "MULTI"));
		WidgetSetDisabled(IDC_EXPORTCAMERAS, ! IsItEnabled);
		WidgetSetDisabled(IDC_CAMERALIST, ! IsItEnabled);
		WidgetSetDisabled(IDC_CAMERALISTSINGLESEL, ! IsItEnabled);

		IsItEnabled = (ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "MULTI"));
		WidgetSetDisabled(IDC_EXPORTLIGHTS, ! IsItEnabled);
		WidgetSetDisabled(IDC_LIGHTLIST, ! IsItEnabled);
		WidgetSetDisabled(IDC_LIGHTLISTSINGLESEL, ! IsItEnabled);

		IsItEnabled = (ExpTable.IsOptionSupported(ActiveFormat, "HAZE", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "HAZE", "MULTI"));
		WidgetSetDisabled(IDC_EXPORTHAZE, ! IsItEnabled);
		WidgetSetDisabled(IDC_HAZELIST, ! IsItEnabled);
		WidgetSetDisabled(IDC_HAZELISTSINGLESEL, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "TERLODNUM");
		WidgetSetDisabled(IDC_TERLODNUM, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_TERLODNUM, ! IsItEnabled);
		WidgetSetDisabled(IDC_CHECKGAPFILL, ! (IsItEnabled && ExpTable.IsFieldValueTrue(ActiveFormat, "LODGAPFILL") && Active->LODLevels > 1));

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "TERLODDIST");
		WidgetSetDisabled(IDC_TERLODDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_TERLODDIST, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "TERDISDIST");
		WidgetSetDisabled(IDC_TERDISDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_TERDISDIST, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "FOLDISDIST");
		WidgetSetDisabled(IDC_FOLDISDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_FOLDISDIST, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBBDIST");
		WidgetSetDisabled(IDC_3DOBBDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_3DOBBDIST, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "3DODISDIST");
		WidgetSetDisabled(IDC_3DODISDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_3DODISDIST, ! IsItEnabled);

		IsItEnabled = ExpTable.IsFieldValueTrue(ActiveFormat, "LBLDISDIST");
		WidgetSetDisabled(IDC_LABELDISDIST, ! IsItEnabled);
		WidgetEMSetReadOnly(IDC_LABELDISDIST, ! IsItEnabled);

		WidgetShow(IDC_HAZELIST, ActiveHazeListID == IDC_HAZELIST);
		WidgetShow(IDC_HAZELISTSINGLESEL, ActiveHazeListID == IDC_HAZELISTSINGLESEL);
		WidgetShow(IDC_CAMERALIST, ActiveCameraListID == IDC_CAMERALIST);
		WidgetShow(IDC_CAMERALISTSINGLESEL, ActiveCameraListID == IDC_CAMERALISTSINGLESEL);
		WidgetShow(IDC_LIGHTLIST, ActiveLightListID == IDC_LIGHTLIST);
		WidgetShow(IDC_LIGHTLISTSINGLESEL, ActiveLightListID == IDC_LIGHTLISTSINGLESEL);
		WidgetShow(IDC_EXPORTLABELS, EffectsHost->EffectTypeImplemented(WCS_EFFECTSSUBCLASS_LABEL));

		if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
			{
			WidgetSetDisabled(IDC_NVE_NAVSPEED, ((SXExtensionNVE *)Active->FormatExtension)->NavUseDefaultSpeed);
			WidgetSetDisabled(IDC_NVE_FOLLOWMAXHEIGHT, ! ((SXExtensionNVE *)Active->FormatExtension)->NavMaxHtConstrain);
			} // if
		if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
			{
			WidgetSetDisabled(IDC_CHECK_OF_INDIVIDUALFOLLOD, ! ((SXExtensionOF *)Active->FormatExtension)->CreateFoliage);
			} // if
		if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
			{
			WidgetSetDisabled(IDC_FBX_PASSWORD, ! ((SXExtensionFBX *)Active->FormatExtension)->UsePassword);
			//WidgetSetDisabled(IDC_CHECK_FBX_SAVEBINARY, ! ((SXExtensionFBX *)Active->FormatExtension)->SaveV5);
			} // if
		FormatIsGIS = ! strcmp(Active->ExportTarget, "GIS");
		WidgetShow(IDC_TBTG_NLOCK, FormatIsGIS);
		WidgetShow(IDC_TBTG_SLOCK, FormatIsGIS);
		WidgetShow(IDC_TBTG_ELOCK, FormatIsGIS);
		WidgetShow(IDC_TBTG_WLOCK, FormatIsGIS);
		//WidgetShow(IDC_WEDEMGRIDSIZETEXT_LABEL, ! FormatIsGIS);
		WidgetShow(IDC_WEDEMGRIDSIZETEXT, ! FormatIsGIS);
		//WidgetShow(IDC_NSDEMGRIDSIZETEXT_LABEL, ! FormatIsGIS);
		WidgetShow(IDC_NSDEMGRIDSIZETEXT, ! FormatIsGIS);
		WidgetShow(IDC_GISWEDEMGRIDSIZE, FormatIsGIS);
		WidgetShow(IDC_GISNSDEMGRIDSIZE, FormatIsGIS && ! Active->SquareCells);
		WidgetShow(IDC_WEDEMGRIDSIZETEXT_LABEL, FormatIsGIS && ! Active->SquareCells);
		WidgetShow(IDC_NSDEMGRIDSIZETEXT_LABEL, FormatIsGIS && ! Active->SquareCells);
		} // if
	} // if

WidgetSetDisabled(IDC_SHOWADVANCED, Active->FormatExtension ? FALSE: TRUE);
WidgetSetText(IDC_RADIOFOLOTHER, AltFolMethod);

} // ExporterEditGUI::DisableWidgets

/*===========================================================================*/

void ExporterEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);
ExportSkyFeatures = RestoreSkyFeatures;

if (ShowAdvanced && Active->FormatExtension)
	ConfigAdvanced();
else
	ConfigNormal();

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ExporterEditGUI::Cancel

/*===========================================================================*/

void ExporterEditGUI::AddScenario(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_SCENARIO);

} // ExporterEditGUI::AddScenario

/*===========================================================================*/

void ExporterEditGUI::RemoveScenario(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
			{
			NumSelected ++;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					if (Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
						{
						//EffectsHost->RemoveRAHost(RemoveItems[Ct], 0);
						} // if
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Render Scenario", "There are no Render Scenarios selected to remove.");
		} // else
	} // if

} // ExporterEditGUI::RemoveScenario

/*===========================================================================*/

void ExporterEditGUI::EditScenario(void)
{
RasterAnimHost *EditMe;
long Ct, NumListEntries;

if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // ExporterEditGUI::EditScenario

/*===========================================================================*/

void ExporterEditGUI::ChangeScenarioListPosition(short MoveUp)
{
RasterAnimHost *MoveMe;
EffectList *Current, *PrevScenario = NULL, *PrevPrevScenario = NULL, *StashScenario;
NotifyTag Changes[2];
long Ct, NumListEntries, SendNotify = 0;

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_SCENARIOLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
					{
					Current = Active->Scenarios;
					while (Current->Me != MoveMe)
						{
						PrevPrevScenario = PrevScenario;
						PrevScenario = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (PrevScenario)
							{
							StashScenario = Current->Next;
							if (PrevPrevScenario)
								{
								PrevPrevScenario->Next = Current;
								Current->Next = PrevScenario;
								} // if
							else
								{
								Active->Scenarios = Current;
								Active->Scenarios->Next = PrevScenario;
								} // else
							PrevScenario->Next = StashScenario;
							SendNotify = 1;
							} // else if
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
		} // if
	else
		{
		for (Ct = NumListEntries - 1; Ct >= 0; Ct --)
			{
			if (WidgetLBGetSelState(IDC_SCENARIOLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_SCENARIOLIST, Ct))
					{
					Current = Active->Scenarios;
					while (Current->Me != MoveMe)
						{
						PrevPrevScenario = PrevScenario;
						PrevScenario = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (Current->Next)
							{
							StashScenario = Current->Next->Next;
							if (PrevScenario)
								{
								PrevScenario->Next = Current->Next;
								PrevScenario->Next->Next = Current;
								} // if
							else
								{
								Active->Scenarios = Current->Next;
								Active->Scenarios->Next = Current;
								} // else
							Current->Next = StashScenario;
							SendNotify = 1;
							} // if move down
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
		} // else
	} // if

// need to send a very general message that will cause SAG to completely rebuild
// Just updating the object will cause crash in SAG with NULL pointer
if (SendNotify)
	{
	Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // ExporterEditGUI::ChangeScenarioListPosition

/*===========================================================================*/

void ExporterEditGUI::BuildMiscList(unsigned short WidgetID, long EffectClass, EffectList *SelectedItems)
{
long Pos = 0;
GeneralEffect *CurEffect;
EffectList *CurList;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

WidgetLBClear(WidgetID);

if (CurEffect = EffectsHost->GetListPtr(EffectClass))
	{
	while (CurEffect)
		{
		BuildScenarioListEntry(ListName, CurEffect);
		WidgetLBInsert(WidgetID, Pos, ListName);
		WidgetLBSetItemData(WidgetID, Pos, CurEffect);
		CurList = SelectedItems;
		while (CurList)
			{
			if (CurList->Me == CurEffect)
				{
				if (WidgetID == IDC_CAMERALIST || WidgetID == IDC_LIGHTLIST || WidgetID == IDC_HAZELIST)
					WidgetLBSetSelState(WidgetID, TRUE, Pos);
				else
					WidgetLBSetCurSel(WidgetID, Pos);
				break;
				} // if
			CurList = CurList->Next;
			} // while
		CurEffect = CurEffect->Next;
		Pos ++;
		} // while
	} // if

} // ExporterEditGUI::BuildMiscList

/*===========================================================================*/

void ExporterEditGUI::FillComboOptions(unsigned short ComboID, char *FieldName, char *TargetName)
{
#ifdef WCS_BUILD_RTX
long ActiveFormat, CurPos, SetTarget = -1;
char OptionName[256];

WidgetCBClear(ComboID);
OptionName[0] = 0;

if ((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Active->ExportTarget)) >= 0)
	{
	while (ExpTable.FetchNextOption(ActiveFormat, FieldName, OptionName))
		{
		if (GlobalApp->Sentinal->CheckImageFormatRTX(OptionName))
			{
			CurPos = WidgetCBInsert(ComboID, -1, (char *)OptionName);
			if (! stricmp(OptionName, TargetName))
				SetTarget = CurPos;
			} // if
		} // while
	} // if

WidgetCBSetCurSel(ComboID, SetTarget);
#endif // WCS_BUILD_RTX

} // ExporterEditGUI::FillComboOptions

/*===========================================================================*/

void ExporterEditGUI::Name(void)
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

} // ExporterEditGUI::Name()

/*===========================================================================*/

void ExporterEditGUI::SelectOutputFormat(void)
{
long ActiveFormat;
NotifyTag Changes[2];
char Str[256];

if (ShowAdvanced)
	ConfigNormal();

if ((ActiveFormat = WidgetCBGetCurSel(IDC_TARGETDROP)) != CB_ERR)
	{
	WidgetCBGetText(IDC_TARGETDROP, ActiveFormat, Str);

	// see if dongle is authorized
	if (ExportControlGUI::FormatSpecificAuth(Str))
		{
		if ((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Str)) >= 0)
			Active->SetTarget(Str);

		FillComboOptions(IDC_FORMATDROP, "IMAGEFILE", Active->ImageFormat);
		FillComboOptions(IDC_FOLFORMATDROP, "FOLIMGFILE", Active->FoliageImageFormat);
		// enforce certain default values
		if (! stricmp(Active->ExportTarget, "LightWave"))
			Active->FolTransparencyStyle = WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_CLIPMAP;

		ValidateSettings(ActiveFormat);
			
		WidgetSetDisabled(IDC_TARGETDROP, TRUE);	// freakin' Windows! Why won't it repaint this flippin' widget by itself?!?
		WidgetSetDisabled(IDC_TARGETDROP, FALSE);

		#ifdef WCS_BUILD_DEMO
		#ifdef WCS_BUILD_VNS
		if (! stricmp(Active->ExportTarget, "OpenFlight") || ! stricmp(Active->ExportTarget, "VRML-STL") || ! stricmp(Active->ExportTarget, "STL"))
			UserMessageOK("Scene Exporter Editor", "OpenFlight and STL are add-on products and are not part of the basic Scene Express. They are enabled in this Demo version so you can see what they look like but you can't actually save any exported files. Be sure to specify that you want Scene Express with OpenFlight or STL when you purchase VNS and Scene Express.");
		#endif // WCS_BUILD_VNS
		#endif // WCS_BUILD_DEMO
		} // if

	// a fairly serious event that should cause a ConfigureWidgets so that floatint limits can be re-set
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // ExporterEditGUI::SelectOutputFormat

/*===========================================================================*/

void ExporterEditGUI::SelectImageFormat(void)
{
long ActiveFormat;
char Str[256];

if ((ActiveFormat = WidgetCBGetCurSel(IDC_FORMATDROP)) != CB_ERR)
	{
	WidgetCBGetText(IDC_FORMATDROP, ActiveFormat, Str);

	strcpy(Active->ImageFormat, Str);
	} // if

} // ExporterEditGUI::SelectImageFormat

/*===========================================================================*/

void ExporterEditGUI::SelectFolImageFormat(void)
{
long ActiveFormat;
char Str[256];

if ((ActiveFormat = WidgetCBGetCurSel(IDC_FOLFORMATDROP)) != CB_ERR)
	{
	WidgetCBGetText(IDC_FOLFORMATDROP, ActiveFormat, Str);

	strcpy(Active->FoliageImageFormat, Str);
	} // if

} // ExporterEditGUI::SelectFolImageFormat

/*===========================================================================*/

void ExporterEditGUI::SelectExportItem(unsigned short WidgetID, long EffectClass)
{
long NumItems, ItemCt;
GeneralEffect *Entry;
NotifyTag Changes[2];

NumItems = WidgetLBGetCount(WidgetID);

Active->DeleteExportItems(EffectClass);

for (ItemCt = 0; ItemCt < NumItems; ItemCt ++)
	{
	if (WidgetLBGetSelState(WidgetID, ItemCt))
		{
		if ((Entry = (GeneralEffect *)WidgetLBGetItemData(WidgetID, ItemCt)) && Entry != (GeneralEffect *)LB_ERR)
			{
			Active->AddExportItem(Entry, 0);
			} // if
		} // if
	} // for

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ExporterEditGUI::SelectExportItem

/*===========================================================================*/

void ExporterEditGUI::ValidateSettings(long ActiveFormat)
{
EffectList *CurEffect, *DelEffect;
char OptionName[256];

// search for any values that are innappropriate for the export target
if (Active->ZipItUp && ! ExpTable.IsFieldValueTrue(ActiveFormat, "ZIPRESULT"))
	Active->ZipItUp = 0;
if (Active->ExportTerrain && ! ExpTable.IsFieldValueTrue(ActiveFormat, "TERRAIN"))
	Active->ExportTerrain = 0;
if (Active->ExportTexture && ! ExpTable.IsFieldValueTrue(ActiveFormat, "TEXTURE"))
	Active->ExportTexture = 0;
if (Active->ExportVectors && ! ExpTable.IsFieldValueTrue(ActiveFormat, "VECTOR"))
	Active->ExportVectors = 0;
if (Active->Export3DObjects && ! ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJ"))
	Active->Export3DObjects = 0;
if (Active->Export3DFoliage && ! ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJ"))
	Active->Export3DFoliage = 0;
if (Active->ExportWalls && ! ExpTable.IsFieldValueTrue(ActiveFormat, "WALL"))
	Active->ExportWalls = 0;
if (Active->ExportSky && ! ExpTable.IsFieldValueTrue(ActiveFormat, "SKY"))
	Active->ExportSky = 0;
if (Active->ExportClouds && ! ExpTable.IsFieldValueTrue(ActiveFormat, "CLOUD"))
	Active->ExportClouds = 0;
if (Active->ExportCelest && ! ExpTable.IsFieldValueTrue(ActiveFormat, "CELEST"))
	Active->ExportCelest = 0;
if (Active->ExportStars && ! ExpTable.IsFieldValueTrue(ActiveFormat, "STAR"))
	Active->ExportStars = 0;
if (Active->ExportAtmosphere && ! ExpTable.IsFieldValueTrue(ActiveFormat, "ATMOSPHERE"))
	Active->ExportAtmosphere = 0;
if (Active->ExportVolumetrics && ! ExpTable.IsFieldValueTrue(ActiveFormat, "VOLUMETRIC"))
	Active->ExportVolumetrics = 0;
if (Active->ExportFoliage && ! ExpTable.IsFieldValueTrue(ActiveFormat, "FOLIAGE"))
	{
	Active->ExportFoliage = 0;
	Active->ExportLabels = 0;
	} // if
if (Active->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY && ! ExpTable.IsFieldValueTrue(ActiveFormat, "OBJCOPY"))
	Active->ObjectTreatment = WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_CREATEALL;
if (Active->SquareCells && ! ExpTable.IsFieldValueTrue(ActiveFormat, "SQUARECELL"))
	Active->SquareCells = 0;

ExportSky = Active->ExportSky;
ExportCelest = Active->ExportCelest;
ExportClouds = Active->ExportClouds;
ExportStars = Active->ExportStars;
ExportAtmosphere = Active->ExportAtmosphere;
ExportVolumetrics = Active->ExportVolumetrics;
ExportSkyFeatures = (ExportSky || ExportCelest || ExportClouds || ExportStars || ExportAtmosphere || ExportVolumetrics);

Active->TerrainAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "TERASOBJ");
Active->FoliageAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "FOLASOBJ");
Active->SkyAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "SKYASOBJ");
Active->VectorAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "VECASOBJ");
Active->ObjectAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "3DOBJASOBJ");
Active->WallAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "WALLASOBJ");
Active->AllowDEMNULL = ExpTable.IsFieldValueTrue(ActiveFormat, "ALLOWNULL");
Active->FoliageInstancesAsObj = ExpTable.IsFieldValueTrue(ActiveFormat, "FOLINSTOBJ");
Active->AlignFlipBoardsToCamera = ExpTable.IsFieldValueTrue(ActiveFormat, "ALIGNFLIP");
Active->PadFolImageBottom = ExpTable.IsFieldValueTrue(ActiveFormat, "PADIMGBOT");
Active->WallFloors = ExpTable.IsFieldValueTrue(ActiveFormat, "WALLFLOOR");
Active->WorldFile = ExpTable.IsFieldValueTrue(ActiveFormat, "WORLDFILE");
if (DEMSquare = ExpTable.IsFieldValueTrue(ActiveFormat, "DEMSQUARE"))
	{
	Active->OneDEMResY = Active->OneDEMResX;
	} // if
if (Active->EqualTiles = ExpTable.IsFieldValueTrue(ActiveFormat, "EQUALTILES"))
	{
	Active->TexTilesY = Active->DEMTilesY;
	Active->TexTilesX = Active->DEMTilesX;
	} // if

if (Active->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES && ! ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "LINES"))
	Active->VectorExpType = (char)ParseDefaultOption(ActiveFormat, "VECEXPTYP");
else if (Active->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS && ! ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "2PT"))
	Active->VectorExpType = (char)ParseDefaultOption(ActiveFormat, "VECEXPTYP");
else if (Active->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS && ! ExpTable.IsOptionSupported(ActiveFormat, "VECEXPTYP", "3PT"))
	Active->VectorExpType = (char)ParseDefaultOption(ActiveFormat, "VECEXPTYP");

if (Active->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "NOFOL"))
	Active->TextureFoliageType = (char)ParseDefaultOption(ActiveFormat, "TEXFOLTYP");
else if (Active->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "FOLIN1"))
	Active->TextureFoliageType = (char)ParseDefaultOption(ActiveFormat, "TEXFOLTYP");
else if (Active->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "FOLIN2"))
	Active->TextureFoliageType = (char)ParseDefaultOption(ActiveFormat, "TEXFOLTYP");
else if (Active->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXFOLTYP", "BOTHIN2"))
	Active->TextureFoliageType = (char)ParseDefaultOption(ActiveFormat, "TEXFOLTYP");

if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES && ! ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "EDGE"))
	Active->BoundsType = (char)ParseDefaultOption(ActiveFormat, "BOUNDSTYP");
else if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS && ! ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "CENTER"))
	Active->BoundsType = (char)ParseDefaultOption(ActiveFormat, "BOUNDSTYP");

if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2 && ! ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "POW2"))
	Active->DEMResOption = (char)ParseDefaultOption(ActiveFormat, "DEMRESOPT");
else if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1 && ! ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "POW2P1"))
	Active->DEMResOption = (char)ParseDefaultOption(ActiveFormat, "DEMRESOPT");
else if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE && ! ExpTable.IsOptionSupported(ActiveFormat, "DEMRESOPT", "ANY"))
	Active->DEMResOption = (char)ParseDefaultOption(ActiveFormat, "DEMRESOPT");

if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2 && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "POW2"))
	Active->TexResOption = (char)ParseDefaultOption(ActiveFormat, "TEXRESOPT");
else if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1 && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "POW2P1"))
	Active->TexResOption = (char)ParseDefaultOption(ActiveFormat, "TEXRESOPT");
else if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE && ! ExpTable.IsOptionSupported(ActiveFormat, "TEXRESOPT", "ANY"))
	Active->TexResOption = (char)ParseDefaultOption(ActiveFormat, "TEXRESOPT");

if (Active->SkyResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2 && ! ExpTable.IsOptionSupported(ActiveFormat, "SKYRESOPT", "POW2"))
	Active->SkyResOption = (char)ParseDefaultOption(ActiveFormat, "SKYRESOPT");
else if (Active->SkyResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE && ! ExpTable.IsOptionSupported(ActiveFormat, "SKYRESOPT", "ANY"))
	Active->SkyResOption = (char)ParseDefaultOption(ActiveFormat, "SKYRESOPT");

if (Active->FolResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2 && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLRESOPT", "POW2"))
	Active->FolResOption = (char)ParseDefaultOption(ActiveFormat, "FOLRESOPT");
else if (Active->FolResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLRESOPT", "ANY"))
	Active->FolResOption = (char)ParseDefaultOption(ActiveFormat, "FOLRESOPT");

if (Active->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "FLIP"))
	Active->FoliageStyle = (char)ParseDefaultOption(ActiveFormat, "FOLSTYLE");
else if (Active->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "CROSS"))
	Active->FoliageStyle = (char)ParseDefaultOption(ActiveFormat, "FOLSTYLE");
else if (Active->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLSTYLE", "OTHER"))
	Active->FoliageStyle = (char)ParseDefaultOption(ActiveFormat, "FOLSTYLE");

if (! Active->ImageFormat[0]
	|| (! stricmp(Active->ImageFormat, "PNG") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "PNG"))
	|| (! stricmp(Active->ImageFormat, "TIFF") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "TIFF"))
	|| (! stricmp(Active->ImageFormat, "JPEG") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "JPEG"))
	|| (! stricmp(Active->ImageFormat, "TARGA") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "TARGA"))
	|| (! stricmp(Active->ImageFormat, "BMP") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "BMP"))
	|| (! stricmp(Active->ImageFormat, "ECW") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "ECW"))
	|| (! stricmp(Active->ImageFormat, "IFF") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "IFF"))
	|| (! stricmp(Active->ImageFormat, "PICT") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "PICT"))
	|| (! stricmp(Active->ImageFormat, "SGI RGB") && ! ExpTable.IsOptionSupported(ActiveFormat, "IMAGEFILE", "SGI RGB")))
	{
	if (ParseDefaultString(ActiveFormat, "IMAGEFILE", OptionName))
		strcpy(Active->ImageFormat, OptionName);
	else
		Active->ImageFormat[0] = 0;
	} // if

if (! Active->FoliageImageFormat[0]
	|| (! stricmp(Active->FoliageImageFormat, "PNG") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "PNG"))
	|| (! stricmp(Active->FoliageImageFormat, "TIFF") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "TIFF"))
	|| (! stricmp(Active->FoliageImageFormat, "TARGA") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "TARGA"))
	|| (! stricmp(Active->FoliageImageFormat, "BMP") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "BMP"))
	|| (! stricmp(Active->FoliageImageFormat, "PICT") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "PICT"))
	|| (! stricmp(Active->FoliageImageFormat, "SGI RGB") && ! ExpTable.IsOptionSupported(ActiveFormat, "FOLIMGFILE", "SGI RGB")))
	{
	if (ParseDefaultString(ActiveFormat, "FOLIMGFILE", OptionName))
		strcpy(Active->FoliageImageFormat, OptionName);
	else
		Active->FoliageImageFormat[0] = 0;
	} // if

if (ParseDefaultString(ActiveFormat, "STYLELABEL", OptionName) && OptionName[0])
	strcpy(AltFolMethod, OptionName);
else
	strcpy(AltFolMethod, "Alt. Method");

if (ParseDefaultString(ActiveFormat, "SINGLEOPTS", OptionName))
	strcpy(Active->SingleOpt, OptionName);
else
	Active->SingleOpt[0] = 0;

if (ParseDefaultString(ActiveFormat, "MULTIOPTS", OptionName))
	strcpy(Active->MultiOpt, OptionName);
else
	Active->MultiOpt[0] = 0;

if (ParseDefaultString(ActiveFormat, "DEMFMT", OptionName))
	strcpy(Active->DEMFormat, OptionName);
else
	Active->DEMFormat[0] = 0;

if (ParseDefaultString(ActiveFormat, "OBJFMT", OptionName))
	strcpy(Active->ObjectFormat, OptionName);
else
	Active->ObjectFormat[0] = 0;

Active->MaxDEMTiles = ParseDefaultValue(ActiveFormat, "MAXDEMTILE");
Active->MaxTexTiles = ParseDefaultValue(ActiveFormat, "MAXTEXTILE");
Active->MaxFolRes = ParseDefaultValue(ActiveFormat, "MAXFOLRES");
Active->MaxTexRes = ParseDefaultValue(ActiveFormat, "MAXTEXRES");
Active->MaxDEMRes = ParseDefaultValue(ActiveFormat, "MAXDEMRES");
Active->MaxDEMVerts = ParseDefaultValue(ActiveFormat, "MAXDEMVERT");
Active->MaxDEMTileOverlap = ParseDefaultValue(ActiveFormat, "MAXDMTOLAP");
Active->MinDEMTileOverlap = ParseDefaultValue(ActiveFormat, "MINDMTOLAP");
Active->MaxTexTileOverlap = ParseDefaultValue(ActiveFormat, "MAXTXTOLAP");
Active->MinTexTileOverlap = ParseDefaultValue(ActiveFormat, "MINTXTOLAP");
Active->MaxCrossBd = ParseDefaultValue(ActiveFormat, "MAXCROSSBD");
Active->MinCrossBd = ParseDefaultValue(ActiveFormat, "MINCROSSBD");

if (Active->DEMTileOverlap > Active->MaxDEMTileOverlap)
	Active->DEMTileOverlap = Active->MaxDEMTileOverlap;
else if (Active->DEMTileOverlap < Active->MinDEMTileOverlap)
	Active->DEMTileOverlap = Active->MinDEMTileOverlap;

if (Active->TexTileOverlap > Active->MaxTexTileOverlap)
	Active->TexTileOverlap = Active->MaxTexTileOverlap;
else if (Active->TexTileOverlap < Active->MinTexTileOverlap)
	Active->TexTileOverlap = Active->MinTexTileOverlap;

if (Active->DEMTilesX > Active->MaxDEMTiles)
	Active->DEMTilesX = Active->MaxDEMTiles;
if (Active->DEMTilesY > Active->MaxDEMTiles)
	Active->DEMTilesY = Active->MaxDEMTiles;

if (Active->TexTilesX > Active->MaxTexTiles)
	Active->TexTilesX = Active->MaxTexTiles;
if (Active->TexTilesY > Active->MaxTexTiles)
	Active->TexTilesY = Active->MaxTexTiles;

if (Active->FoliageRes > Active->MaxFolRes)
	Active->FoliageRes = Active->MaxFolRes;

if (Active->OneDEMResX > Active->MaxDEMRes)
	Active->OneDEMResX = Active->MaxDEMRes;
if (Active->OneDEMResY > Active->MaxDEMRes)
	Active->OneDEMResY = Active->MaxDEMRes;
if (Active->OneDEMResY * Active->OneDEMResX > Active->MaxDEMVerts)
	Active->OneDEMResY = Active->MaxDEMVerts / Active->OneDEMResX;

if (Active->OneTexResX > Active->MaxTexRes)
	Active->OneTexResX = Active->MaxTexRes;
if (Active->OneTexResY > Active->MaxTexRes)
	Active->OneTexResY = Active->MaxTexRes;
if (Active->SkyRes > Active->MaxTexRes)
	Active->SkyRes = Active->MaxTexRes;
if (Active->MaxWallTexSize > Active->MaxTexRes)
	Active->MaxWallTexSize = Active->MaxTexRes;
if (Active->Max3DOTexSize > Active->MaxTexRes)
	Active->Max3DOTexSize = Active->MaxTexRes;
if (Active->NumFoliageBoards > Active->MaxCrossBd)
	Active->NumFoliageBoards = Active->MaxCrossBd;
if (Active->NumFoliageBoards < Active->MinCrossBd)
	Active->NumFoliageBoards = Active->MinCrossBd;

if (ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "MULTI"))
	{
	if (! ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "MULTI"))
		{
		// delete all but first item
		if (CurEffect = Active->Cameras)
			{
			CurEffect = CurEffect->Next;
			Active->Cameras->Next = NULL;
			while (CurEffect)
				{
				DelEffect = CurEffect;
				CurEffect = CurEffect->Next;
				delete DelEffect;
				} // while
			} // if at least one
		} // if
	} // if
else
	{
	Active->ExportCameras = 0;
	Active->DeleteExportItems(WCS_EFFECTSSUBCLASS_CAMERA);
	} // else

if (ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "MULTI"))
	{
	if (! ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "MULTI"))
		{
		// delete all but first item
		if (CurEffect = Active->Lights)
			{
			CurEffect = CurEffect->Next;
			Active->Lights->Next = NULL;
			while (CurEffect)
				{
				DelEffect = CurEffect;
				CurEffect = CurEffect->Next;
				delete DelEffect;
				} // while
			} // if at least one
		} // if
	} // if
else
	{
	Active->ExportLights = 0;
	Active->DeleteExportItems(WCS_EFFECTSSUBCLASS_LIGHT);
	} // else

if (ExpTable.IsOptionSupported(ActiveFormat, "HAZE", "SINGLE") || ExpTable.IsOptionSupported(ActiveFormat, "HAZE", "MULTI"))
	{
	if (! ExpTable.IsOptionSupported(ActiveFormat, "HAZE", "MULTI"))
		{
		// delete all but first item
		if (CurEffect = Active->Haze)
			{
			CurEffect = CurEffect->Next;
			Active->Haze->Next = NULL;
			while (CurEffect)
				{
				DelEffect = CurEffect;
				CurEffect = CurEffect->Next;
				delete DelEffect;
				} // while
			} // if at least one
		} // if
	} // if
else
	{
	Active->ExportHaze = 0;
	Active->DeleteExportItems(WCS_EFFECTSSUBCLASS_ATMOSPHERE);
	} // else

if (ExpTable.IsOptionSupported(ActiveFormat, "CAMERA", "MULTI"))
	ActiveCameraListID = IDC_CAMERALIST;
else
	ActiveCameraListID = IDC_CAMERALISTSINGLESEL;

if (ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "MULTI"))
	ActiveLightListID = IDC_LIGHTLIST;
else
	ActiveLightListID = IDC_LIGHTLISTSINGLESEL;

if (ExpTable.IsOptionSupported(ActiveFormat, "LIGHT", "MULTI"))
	ActiveHazeListID = IDC_HAZELIST;
else
	ActiveHazeListID = IDC_HAZELISTSINGLESEL;

FiddleDEMRes();
FiddleTexRes();
FiddleSkyRes();
FiddleFolRes();

} // ExporterEditGUI::ValidateSettings

/*===========================================================================*/

long ExporterEditGUI::ParseDefaultOption(long ActiveFormat, char *FieldName)
{
char OptionName[256];

if (ExpTable.FetchDefaultOption(ActiveFormat, FieldName, OptionName))
	{
	if (! stricmp(FieldName, "VECEXPTYP"))
		{
		if (! stricmp(OptionName, "LINES"))
			return (WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES);
		else if (! stricmp(OptionName, "2PT"))
			return (WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS);
		} // if
	else if (! stricmp(FieldName, "TEXFOLTYP"))
		{
		if (! stricmp(OptionName, "NOFOL"))
			return (WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST);
		else if (! stricmp(OptionName, "FOLIN1"))
			return (WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST);
		else if (! stricmp(OptionName, "FOLIN2"))
			return (WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND);
		} // if
	else if (! stricmp(FieldName, "BOUNDSTYP"))
		{
		if (! stricmp(OptionName, "EDGE"))
			return (WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS);
		} // if
	else if (! stricmp(FieldName, "DEMRESOPT"))
		{
		if (! stricmp(OptionName, "POW2"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2);
		else if (! stricmp(OptionName, "POW2P1"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE);
		} // if
	else if (! stricmp(FieldName, "TEXRESOPT"))
		{
		if (! stricmp(OptionName, "POW2"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2);
		else if (! stricmp(OptionName, "POW2P1"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE);
		} // if
	else if (! stricmp(FieldName, "SKYRESOPT"))
		{
		if (! stricmp(OptionName, "POW2"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE);
		} // if
	else if (! stricmp(FieldName, "FOLRESOPT"))
		{
		if (! stricmp(OptionName, "POW2"))
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE);
		} // if
	else if (! stricmp(FieldName, "FOLSTYLE"))
		{
		if (! stricmp(OptionName, "FLIP"))
			return (WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS);
		else
			return (WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS);
		} // if
	} // if

return (0);

} // ExporterEditGUI::ParseDefaultOption

/*===========================================================================*/

long ExporterEditGUI::ParseDefaultValue(long ActiveFormat, char *FieldName)
{
char OptionValue[256];

if (ExpTable.FetchDefaultOption(ActiveFormat, FieldName, OptionValue))
	{
	return (atoi(OptionValue));
	} // if

if (! stricmp(FieldName, "MAXDEMTILE"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXTEXTILE"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXFOLRES"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXTEXRES"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXDEMRES"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXDEMVERT"))
	return (LONG_MAX);
if (! stricmp(FieldName, "MAXDMTOLAP"))
	return (1);
if (! stricmp(FieldName, "MINDMTOLAP"))
	return (1);
if (! stricmp(FieldName, "MAXTXTOLAP"))
	return (1);
if (! stricmp(FieldName, "MINTXTOLAP"))
	return (1);
if (! stricmp(FieldName, "MAXCROSSBD"))
	return (2);
if (! stricmp(FieldName, "MINCROSSBD"))
	return (2);

return (0);

} // ExporterEditGUI::ParseDefaultValue

/*===========================================================================*/

const char *ExporterEditGUI::ParseDefaultString(long ActiveFormat, char *FieldName, char *OptionName)
{

if (ExpTable.FetchDefaultOption(ActiveFormat, FieldName, OptionName))
	{
	return (OptionName);
	} // if

return (NULL);

} // ExporterEditGUI::ParseDefaultString

/*===========================================================================*/

void ExporterEditGUI::ExportNow(void)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_EXG, 0);

} // ExporterEditGUI::ExportNow

/*===========================================================================*/

void ExporterEditGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current, ActiveFormat;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)CB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Active->SetCoords(NewObj);

	// get database bounds
	// try first with DEMs only, then with all DB objects
	if (Active->GeoReg.SnapToBounds(DBHost, ProjHost, Active->Coords, TRUE) || Active->GeoReg.SnapToBounds(DBHost, ProjHost, Active->Coords, FALSE))
		{
		BackupPrevBounds();
		if (((ActiveFormat = ExpTable.FindRecordByFieldStr("NAME", Active->ExportTarget)) >= 0)
			&& (ExpTable.IsOptionSupported(ActiveFormat, "BOUNDSTYP", "EDGE")))
			Active->BoundsType = WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		} // if
	ConfigDEMCellSize();
	ConfigTexCellSize();
	ConfigBoundsMetric();
	ComputeSTLScale();
	//if (stricmp("GIS", Active->ExportTarget) == 0)
	//	GISclamping();
	} // if
#endif // WCS_BUILD_VNS

} // ExporterEditGUI::SelectNewCoords

/*===========================================================================*/

void ExporterEditGUI::SetBounds(DiagnosticData *Data)
{

if (ReceivingDiagnostics == 0)
	{
	if (UserMessageOKCAN("Set Geographic Bounds", "The next two points clicked in any View\n will become this Scene Exporter's new bounds.\n\nPoints may be selected in any order."))
		{
		ReceivingDiagnostics = 1;
		GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		} // if
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
			Active->GeoReg.SetBounds(Active->Coords, LatEvent, LonEvent);
			BackupPrevBounds();
			ReceivingDiagnostics = 0;
			ConfigureWidgets();
			//if (stricmp("GIS", Active->ExportTarget) == 0)
			//	GISclamping();
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);

} // ExporterEditGUI::SetBounds

/*===========================================================================*/

void ExporterEditGUI::FiddleDEMRes(void)
{
long Res;

if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
	Active->OneDEMResY = Active->MaxDEMVerts / Active->OneDEMResX;

if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
	return;

if (LastDEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1 || JustOpening)
	{
	Active->OneDEMResX --;
	Active->OneDEMResY --;
	} // if

// find next highest res for both X and Y
for (Res = 2; Res <= 262144; Res *= 2)
	{
	if (Res >= Active->OneDEMResX || Res == 262144)
		{
		Active->OneDEMResX = Res;
		// cut it down to size
		if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
			Active->OneDEMResX /= 2;
		break;
		} // for
	} // for
for (Res = 2; Res <= 262144; Res *= 2)
	{
	if (Res >= Active->OneDEMResY || Res == 262144)
		{
		Active->OneDEMResY = Res;
		// cut it down to size
		if (Active->OneDEMResX * Active->OneDEMResY > Active->MaxDEMVerts)
			Active->OneDEMResY /= 2;
		break;
		} // for
	} // for

if (Active->DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
	{
	Active->OneDEMResX ++;
	Active->OneDEMResY ++;
	} // if
LastDEMResOption = Active->DEMResOption;
LastOneDEMResX = Active->OneDEMResX;
LastOneDEMResY = Active->OneDEMResY;

} // ExporterEditGUI::FiddleDEMRes

/*===========================================================================*/

void ExporterEditGUI::FiddleTexRes(void)
{
long Res;

if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
	return;

if (LastTexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1 || JustOpening)
	{
	Active->OneTexResX --;
	Active->OneTexResY --;
	} // if

// find next highest res for both X and Y
for (Res = 2; Res <= 262144; Res *= 2)
	{
	if (Res >= Active->OneTexResX || Res == 262144)
		{
		Active->OneTexResX = Res;
		break;
		} // for
	} // for
for (Res = 2; Res <= 262144; Res *= 2)
	{
	if (Res >= Active->OneTexResY || Res == 262144)
		{
		Active->OneTexResY = Res;
		break;
		} // for
	} // for

if (Active->TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
	{
	Active->OneTexResX ++;
	Active->OneTexResY ++;
	} // if
LastTexResOption = Active->TexResOption;
LastOneTexResX = Active->OneTexResX;
LastOneTexResY = Active->OneTexResY;

} // ExporterEditGUI::FiddleTexRes

/*===========================================================================*/

void ExporterEditGUI::FiddleSkyRes(void)
{
long Res;

if (Active->SkyResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
	return;

// find next highest res for both X and Y
for (Res = 2; Res <= 16384; Res *= 2)
	{
	if (Res >= Active->SkyRes || Res == 16384)
		{
		Active->SkyRes = Res;
		break;
		} // for
	} // for

LastSkyRes = Active->SkyRes;

} // ExporterEditGUI::FiddleSkyRes

/*===========================================================================*/

void ExporterEditGUI::FiddleFolRes(void)
{
long Res;

if (Active->FolResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
	return;

// find next highest res for both X and Y
for (Res = 2; Res <= 16384; Res *= 2)
	{
	if (Res >= Active->FoliageRes || Res == 16384)
		{
		Active->FoliageRes = Res;
		break;
		} // for
	} // for

LastFoliageRes = Active->FoliageRes;

} // ExporterEditGUI::FiddleFolRes

/*===========================================================================*/

void ExporterEditGUI::AdjustBoundsFromCellSize(int CtrlID)
{
double OldBoundsValue, BoundsDiff, NumNewCells, PlanetRad, RenderWidth, RenderHeight;
AnimDoubleTime *ADTChanged, *ADTNotChanged;
int NorthChanged = 0, SouthChanged = 0, EastChanged = 0, WestChanged = 0, RoundUp = 0;

// which axis changed, which bounds? the others remain fixed
if (NorthChanged = (CtrlID == IDC_NORTH))
	{
	ADTChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH);
	ADTNotChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH);
	OldBoundsValue = PrevNorthValue;
	} // if
else if (SouthChanged = (CtrlID == IDC_SOUTH))
	{
	ADTChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH);
	ADTNotChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH);
	OldBoundsValue = PrevSouthValue;
	} // if
else if (EastChanged = (CtrlID == IDC_EAST))
	{
	ADTChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST);
	ADTNotChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST);
	OldBoundsValue = PrevEastValue;
	} // if
else if (WestChanged = (CtrlID == IDC_WEST))
	{
	ADTChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST);
	ADTNotChanged = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST);
	OldBoundsValue = PrevWestValue;
	} // if
else
	return;

// get the old bounds
//OldBoundsValue = WidgetSNGetPrevValue();

// find out if it is an increase in size or decrease
BoundsDiff = ADTChanged->CurValue - OldBoundsValue;

// interpret the bounds difference based on which value it is and which units
if (NorthChanged)
	{
	if (BoundsDiff > 0.0)
		RoundUp = 1;
	} // if
else if (SouthChanged)
	{
	if (BoundsDiff < 0.0)
		RoundUp = 1;
	} // if
else if (EastChanged)
	{
	if (BoundsDiff > 0.0)
		RoundUp = 1;
	} // if
else	// WestChanged
	{
	if (BoundsDiff < 0.0)
		RoundUp = 1;
	} // if
// geographic longitude is backwards
if (EastChanged || WestChanged)
	{
	if (! Active->Coords || Active->Coords->GetGeographic())
		{
		RoundUp = ! RoundUp;
		} // if
	} // if

// find out the new dimensions in meters
PlanetRad = EffectsHost->GetPlanetRadius();
if (Active->Coords && ! Active->Coords->GetGeographic())
	Active->GeoReg.GetMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight);
else
	{
	RenderWidth = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
	RenderHeight = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
	} // else

// find out how many cells result using DEMCellSize.CurValue
if (NorthChanged || SouthChanged)
	{
	NumNewCells = fabs(RenderHeight / NSDEMCellSize.CurValue);
	// round up or down the number of cells
	if (RoundUp)
		{
		NumNewCells = WCS_ceil(NumNewCells);
		NumNewCells += (Active->DEMTilesY - 1);
		} // if
	else
		{
		NumNewCells = WCS_floor(NumNewCells);
		NumNewCells -= (Active->DEMTilesY - 1);
		} // else
	if (NorthChanged)
		ADTChanged->SetValue(ADTNotChanged->CurValue + NumNewCells * NSDEMCellSize.CurValue);
	else
		ADTChanged->SetValue(ADTNotChanged->CurValue - NumNewCells * NSDEMCellSize.CurValue);
	Active->ComputeRowsFromNumCells((long)NumNewCells);
	} // if
else
	{
	NumNewCells = fabs(RenderWidth / WEDEMCellSize.CurValue);
	// round up or down the number of cells
	if (RoundUp)
		{
		NumNewCells = WCS_ceil(NumNewCells);
		NumNewCells += (Active->DEMTilesX - 1);
		} // if
	else
		{
		NumNewCells = WCS_floor(NumNewCells);
		NumNewCells -= (Active->DEMTilesX - 1);
		} // else
	// recalculate the dimensions and re-set the bounds
	if (Active->Coords && ! Active->Coords->GetGeographic())
		{
		// positive east
		if (EastChanged)
			ADTChanged->SetValue(ADTNotChanged->CurValue + NumNewCells * WEDEMCellSize.CurValue);
		else
			ADTChanged->SetValue(ADTNotChanged->CurValue - NumNewCells * WEDEMCellSize.CurValue);
		Active->ComputeColsFromNumCells((long)NumNewCells);
		} // if
	else
		{
		// positive west
		if (WestChanged)
			ADTChanged->SetValue(ADTNotChanged->CurValue + NumNewCells * WEDEMCellSize.CurValue);
		else
			ADTChanged->SetValue(ADTNotChanged->CurValue - NumNewCells * WEDEMCellSize.CurValue);
		} // if
	} // else
BackupPrevBounds();
WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DEMCOLS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DEMROWS, WP_FISYNC_NONOTIFY);

} // ExporterEditGUI::AdjustBoundsFromCellSize

/*===========================================================================*/

void ExporterEditGUI::ComputeBoundsFromCellSize(void)
{
double RenderWidth, RenderHeight, PlanetRad, PixResX, PixResY;
long ImageWidth, ImageHeight;
int FixedNorth, FixedSouth, FixedEast, FixedWest;

PlanetRad = EffectsHost->GetPlanetRadius();
if (Active->Coords && ! Active->Coords->GetGeographic())
	Active->GeoReg.GetMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight);
else
	{
	RenderWidth = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
	RenderHeight = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
	} // else
Active->CalcFullResolutions();

ImageWidth = Active->DEMResX;
ImageHeight = Active->DEMResY;
PixResX = WEDEMCellSize.CurValue;
PixResY = NSDEMCellSize.CurValue;

if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	RenderWidth = PixResX * (ImageWidth - 1);
	RenderHeight = PixResY * (ImageHeight - 1);
	} // if
else
	{
	RenderWidth = PixResX * ImageWidth;
	RenderHeight = PixResY * ImageHeight;
	} // else

FixedNorth = WidgetGetCheck(IDC_TBTG_NLOCK);
FixedSouth = WidgetGetCheck(IDC_TBTG_SLOCK);
FixedEast = WidgetGetCheck(IDC_TBTG_ELOCK);
FixedWest = WidgetGetCheck(IDC_TBTG_WLOCK);

PlanetRad = EffectsHost->GetPlanetRadius();
if (Active->Coords && (! Active->Coords->GetGeographic()))
	Active->GeoReg.SetBoundsFromMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight,
		FixedNorth, FixedSouth, FixedEast, FixedWest);
else
	Active->GeoReg.SetBoundsFromGeographicWidthHeight(RenderWidth, RenderHeight,
		FixedNorth, FixedSouth, FixedEast, FixedWest);
BackupPrevBounds();
WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);

} // ExporterEditGUI::ComputeBoundsFromCellSize

/*===========================================================================*/

void ExporterEditGUI::ConfigDEMCellSize(void)
{
double RenderWidth, RenderHeight, PlanetRad, PixResX, PixResY;
long ImageWidth, ImageHeight;
char Textology[64];

PlanetRad = EffectsHost->GetPlanetRadius();
Active->GeoReg.GetMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight);
Active->CalcFullResolutions();
if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
	{
	// if use default speed
	if (((SXExtensionNVE *)Active->FormatExtension)->NavUseDefaultSpeed)
		{
		// set default speed for user reference
		((SXExtensionNVE *)Active->FormatExtension)->NavSpeed = (long)Active->CalcDefaultSpeed();
		} // if
	} // if
ImageWidth = Active->DEMResX;
ImageHeight = Active->DEMResY;

///*** Frank changed with Xenon's consultation 06/10/05
// add a cell width and height if bounds are centers
if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	PixResX = RenderWidth / (ImageWidth - 1);
	PixResY = RenderHeight / (ImageHeight - 1);
	} // if
else
	{
	PixResX = RenderWidth / ImageWidth;
	PixResY = RenderHeight / ImageHeight;
	} // else
//***/
//PixResX = RenderWidth / (ImageWidth - 1);
//PixResY = RenderHeight / (ImageHeight - 1);

sprintf(Textology, "%f", PixResX);
WidgetSetText(IDC_WEDEMGRIDSIZETEXT, Textology);
sprintf(Textology, "%f", PixResY);
WidgetSetText(IDC_NSDEMGRIDSIZETEXT, Textology);

if (Active->Coords && (! Active->Coords->GetGeographic()))
	{
	NSDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	WEDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	NSDEMCellSize.SetValue(PixResY);
	WEDEMCellSize.SetValue(PixResX);
	} // if
else
	{
	NSDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	WEDEMCellSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	RenderWidth = fabs(Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
	if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
		{
		PixResX = RenderWidth / (ImageWidth - 1);
		} // if
	else
		{
		PixResX = RenderWidth / ImageWidth;
		} // else
	RenderHeight = Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Active->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
	if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
		{
		PixResY = RenderHeight / (ImageHeight - 1);
		} // if
	else
		{
		PixResY = RenderHeight / ImageHeight;
		} // else
	NSDEMCellSize.SetValue(PixResY);
	WEDEMCellSize.SetValue(PixResX);
	} // else geographic
WidgetSNSync(IDC_GISNSDEMGRIDSIZE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_GISWEDEMGRIDSIZE, WP_FISYNC_NONOTIFY);

} // ExporterEditGUI::ConfigDEMCellSize

/*===========================================================================*/

void ExporterEditGUI::ConfigTexCellSize(void)
{
double GeoResX, GeoResY, RenderWidth, RenderHeight, PlanetRad, PixResX, PixResY;
long ImageWidth, ImageHeight;
char Textology[64];

PlanetRad = EffectsHost->GetPlanetRadius();
Active->GeoReg.GetMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight);
Active->CalcFullResolutions();
ImageWidth = Active->TexResX;
ImageHeight = Active->TexResY;

// add a cell width and height if bounds are centers
if (Active->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	PixResX = RenderWidth / (ImageWidth - 1);
	PixResY = RenderHeight / (ImageHeight - 1);
	if ((! Active->Coords) || Active->Coords->GetGeographic())
		{
		GeoResX = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->CurValue -
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->CurValue;
		GeoResX /= (ImageWidth - 1);
		GeoResY = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->CurValue -
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->CurValue;
		GeoResY /= (ImageHeight - 1);
		} // if
	} // if
else
	{
	PixResX = RenderWidth / ImageWidth;
	PixResY = RenderHeight / ImageHeight;
	if ((! Active->Coords) || Active->Coords->GetGeographic())
		{
		GeoResX = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->CurValue -
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->CurValue;
		GeoResX /= ImageWidth;
		GeoResY = Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->CurValue -
			Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->CurValue;
		GeoResY /= ImageHeight;
		} // if
	} // else

if (! strcmp(Active->ExportTarget, "GIS"))
	{
	if ((! Active->Coords) || Active->Coords->GetGeographic())
		{
		sprintf(Textology, "%.5f  %.1fm", GeoResX, PixResX);	//lint !e644
		WidgetSetText(IDC_WETEXGRIDSIZETEXT, Textology);
		sprintf(Textology, "%.5f  %.1fm", GeoResY, PixResY);	//lint !e644
		WidgetSetText(IDC_NSTEXGRIDSIZETEXT, Textology);
		} // if
	else
		{
		sprintf(Textology, "%.1fm", PixResX);
		WidgetSetText(IDC_WETEXGRIDSIZETEXT, Textology);
		sprintf(Textology, "%.1fm", PixResY);
		WidgetSetText(IDC_NSTEXGRIDSIZETEXT, Textology);
		} // else
	} // if
else
	{
	sprintf(Textology, "%f", PixResX);
	WidgetSetText(IDC_WETEXGRIDSIZETEXT, Textology);
	sprintf(Textology, "%f", PixResY);
	WidgetSetText(IDC_NSTEXGRIDSIZETEXT, Textology);
	} // else

} // ExporterEditGUI::ConfigTexCellSize

/*===========================================================================*/

void ExporterEditGUI::ConfigBoundsMetric(void)
{

Active->GeoReg.SetMetricType(Active->Coords);
WidgetSNSync(IDC_NORTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_WEST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EAST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SOUTH, WP_FISYNC_NONOTIFY);

} // ExporterEditGUI::ConfigBoundsMetric

/*===========================================================================*/

void ExporterEditGUI::ConfigAdvanced(void)
{

WidgetTCClear(IDC_TAB1);
ShowAdvanced = 1;

WidgetTCInsertItem(IDC_TAB1, 0, TabNames[0]);
ActiveAdvancedPage = 1;
ShowPanel(0, -1);
if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
	{
	WidgetTCInsertItem(IDC_TAB1, 1, "Advanced STL Options");
	ShowPanel(1, 0);
	} // if
else if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
	{
	WidgetTCInsertItem(IDC_TAB1, 1, "NatureView Options");
	WidgetTCInsertItem(IDC_TAB1, 2, "Options 2");
	if (GlobalApp->Sentinal->CheckAuthFieldSX2())
		WidgetTCInsertItem(IDC_TAB1, 3, "Options 3");
	ShowPanel(1, 1);
	} // if
else if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
	{
	WidgetTCInsertItem(IDC_TAB1, 1, "Advanced OpenFlight Options");
	ShowPanel(1, 4);
	} // if
else if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
	{
	WidgetTCInsertItem(IDC_TAB1, 1, "Advanced FBX Options");
	ShowPanel(1, 5);
	} // if
else if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
	{
	WidgetTCInsertItem(IDC_TAB1, 1, "Advanced GoogleEarth Options");
	WidgetTCInsertItem(IDC_TAB1, 2, "Options 2");
	ShowPanel(1, 6);
	} // if
else
	ShowPanel(1, -1);
WidgetTCSetCurSel(IDC_TAB1, 1);
WidgetSetText(IDC_SHOWADVANCED, "Show Normal Options");

} // ExporterEditGUI::ConfigAdvanced

/*===========================================================================*/

void ExporterEditGUI::ConfigNormal(void)
{
long TabIndex;

WidgetTCClear(IDC_TAB1);
ShowAdvanced = 0;

for (TabIndex = 0; TabIndex < WCS_EXPORTERGUI_NUMTABS; TabIndex ++)
	{
	WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
	} // for
WidgetTCSetCurSel(IDC_TAB1, ActivePage);
ShowPanel(1, -1);
ShowPanel(0, -1);
ShowPanel(0, ActivePage);
WidgetSetText(IDC_SHOWADVANCED, "Show Advanced Options");

} // ExporterEditGUI::ConfigNormal

/*===========================================================================*/

void ExporterEditGUI::SelectSTLUnits(void)
{
long Current;

if (Active->FormatExtension)
	{
	// STL and VRML-STL
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
		{
		if ((Current = WidgetCBGetCurSel(IDC_STL_UNITSDROP)) != CB_ERR)
			{
			((SXExtensionSTL *)Active->FormatExtension)->UnitOfMeasure = (char)Current;
			ComputeSTLScale();
			} // if
		} // if
	} // if

} // ExporterEditGUI::SelectSTLUnits

/*===========================================================================*/

void ExporterEditGUI::ComputeSTLScale(void)
{
double offset = 0.001, RenderWidth, RenderHeight, PlanetRad, scalefactor, xdim, xfactor, ydim, yfactor;
char DimStr[64], buildScaleMode;

if (Active->FormatExtension)
	{
	// STL and VRML-STL
	if (Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
		{
		PlanetRad = EffectsHost->GetPlanetRadius();
		Active->GeoReg.GetMetricWidthHeight(Active->Coords, PlanetRad, RenderWidth, RenderHeight);
		((SXExtensionSTL *)Active->FormatExtension)->ActualDimZ = 0.0;	// the GUI has no idea of the terrain range at this point
		if (((SXExtensionSTL *)Active->FormatExtension)->BuildMode == WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOFIT)
			{
			// calculate the scale from terrain dimensions and Max dimensions
			// RenderWidth, RenderHeight are the actual sizes in meters of the terrain
			xdim = (((SXExtensionSTL *)Active->FormatExtension)->MaxDimX * Active->DEMTilesX) - offset;
			ydim = (((SXExtensionSTL *)Active->FormatExtension)->MaxDimY * Active->DEMTilesY) - offset;
			if (((SXExtensionSTL *)Active->FormatExtension)->UnitOfMeasure == WCS_EFFECTS_SXEXTENSION_BUILDUNIT_INCHES)
				{
				xdim *= 25.4;	// convert inches to millimeters
				ydim *= 25.4;
				} // if
			xfactor = RenderWidth / xdim;
			yfactor = RenderHeight / ydim;
			if (xfactor >= yfactor)
				scalefactor = xfactor;
			else
				scalefactor = yfactor;
			xdim = RenderWidth / scalefactor;
			ydim = RenderHeight / scalefactor;
			if (((SXExtensionSTL *)Active->FormatExtension)->UnitOfMeasure == WCS_EFFECTS_SXEXTENSION_BUILDUNIT_INCHES)
				{
				xdim /= 25.4;	// convert millimeters to inches
				ydim /= 25.4;
				} // if
			((SXExtensionSTL *)Active->FormatExtension)->ActualDimX = xdim;
			((SXExtensionSTL *)Active->FormatExtension)->ActualDimY = ydim;
			((SXExtensionSTL *)Active->FormatExtension)->BuildScale = scalefactor * 1000.0;
			buildScaleMode = 1;
			} // if
		else	// build to scale
			{
			// calculate the actual size from scale and terrain dimensions and somehow include the Max dimensions
			// RenderWidth, RenderHeight are the actual sizes in meters of the terrain
			scalefactor = ((SXExtensionSTL *)Active->FormatExtension)->BuildScale / 1000.0;
			xdim = RenderWidth / scalefactor;
			ydim = RenderHeight / scalefactor;
			if (((SXExtensionSTL *)Active->FormatExtension)->UnitOfMeasure == WCS_EFFECTS_SXEXTENSION_BUILDUNIT_INCHES)
				{
				xdim /= 25.4;	// convert millimeters to inches
				ydim /= 25.4;
				} // if
			((SXExtensionSTL *)Active->FormatExtension)->ActualDimX = xdim;
			((SXExtensionSTL *)Active->FormatExtension)->ActualDimY = ydim;
			buildScaleMode = 0;
			} // else
		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimX);
		WidgetSetText(IDC_STL_BUILDDIMX, DimStr);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimY);
		WidgetSetText(IDC_STL_BUILDDIMY, DimStr);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->ActualDimZ);
		WidgetSetText(IDC_STL_BUILDDIMZ, DimStr);

		sprintf(DimStr, "%g", ((SXExtensionSTL *)Active->FormatExtension)->BuildScale);
		WidgetSetText(IDC_STL_BUILDSCALE, DimStr);
		SyncWidgets();

		WidgetSetDisabled(IDC_STL_BUILDSCALE, buildScaleMode);
		} // if
	} // if

} // ExporterEditGUI::ComputeSTLScale

/*===========================================================================*/

void ExporterEditGUI::HDILearnMore(void)
{

if (UserMessageYN("HD Instance", "HD Instance is a plugin for LightWave 3D 7.5 and higher that lets you render thousands of object instances without using up lots of memory. It is the best way represent large numbers of trees in LightWave. The LW scene file otherwise can become so large that LW will not load it and even if it does, it will be very slow to render. If you have more than a few hundred trees in your scene you should seriously consider buying HD Instance.\n\nClick \"Yes\" to learn more about the HD Instance plugin from Happy Digital, Ltd."))
	{
	GlobalApp->HelpSys->OpenURLIndirect("http://www.happy-digital.com/instance.asp");
	} // if

} // ExporterEditGUI::HDILearnMore

/*===========================================================================*/

void ExporterEditGUI::RecordActionTextChange(unsigned short WidgetID)
{
NotifyTag Changes[2];
char NewString[2048];

if (WidgetGetModified(WidgetID))
	{
	WidgetGetText(WidgetID, 2048, NewString);
	WidgetSetModified(WidgetID, FALSE);
	if (Active->FormatExtension && Active->FormatExtension->GetSupportsQueryAction())
		{
		if (WidgetID == IDC_NV_ACTIONTEXT && ActiveAction)
			ActiveAction->CreateString(NewString);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // ExporterEditGUI::RecordActionTextChange

/*===========================================================================*/

void ExporterEditGUI::RecordNVETextChange(unsigned short WidgetID)
{
NotifyTag Changes[2];
char NewString[2048];

if (WidgetGetModified(WidgetID))
	{
	WidgetGetText(WidgetID, 2048, NewString);
	WidgetSetModified(WidgetID, FALSE);
	if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
		{
		if (WidgetID == IDC_NVE_LOGOTEXT)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->OverlayLogoText, NewString);
		if (WidgetID == IDC_NVE_WATERMARK)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->WatermarkText, NewString);
		if (WidgetID == IDC_NVE_NAME)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaName, NewString);
		if (WidgetID == IDC_NVE_COPYRIGHT)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaCopyright, NewString);
		if (WidgetID == IDC_NVE_AUTHOR)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaAuthor, NewString);
		if (WidgetID == IDC_NVE_EMAIL)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaEmail, NewString);
		if (WidgetID == IDC_NVE_USER1)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaUser1, NewString);
		if (WidgetID == IDC_NVE_USER2)
			((SXExtensionNVE *)Active->FormatExtension)->CreateString(&((SXExtensionNVE *)Active->FormatExtension)->MetaUser2, NewString);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // ExporterEditGUI::RecordNVETextChange

/*===========================================================================*/

void ExporterEditGUI::RecordFBXTextChange(unsigned short WidgetID)
{
NotifyTag Changes[2];
char NewString[2048];

if (WidgetGetModified(WidgetID))
	{
	WidgetGetText(WidgetID, 128, NewString);
	WidgetSetModified(WidgetID, FALSE);
	if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
		{
		if (WidgetID == IDC_FBX_PASSWORD)
			((SXExtensionFBX *)Active->FormatExtension)->SetPassword(NewString);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // ExporterEditGUI::RecordFBXTextChange

/*===========================================================================*/

void ExporterEditGUI::RecordGETextChange(unsigned short WidgetID)
{
NotifyTag Changes[2];
char NewString[2048];

if (WidgetGetModified(WidgetID))
	{
	WidgetGetText(WidgetID, 128, NewString);
	WidgetSetModified(WidgetID, FALSE);
	if (Active->FormatExtension && Active->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
		{
		if (WidgetID == IDC_GE_MESSAGE)
			((SXExtensionGE *)Active->FormatExtension)->SetMessage(NewString);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // ExporterEditGUI::RecordGETextChange

/*===========================================================================*/

void ExporterEditGUI::ConfigureActiveAction(void)
{
SXQueryAction *CurAction;
long InsertPos, FoundPos, ActionCt;
char ActionName[256];

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && (ActiveAction = Active->VerifyActiveAction(ActiveAction)))
		{
		Active->SetActiveAction(ActiveAction);
		// build list of actions
		// highlight the active action in the list - part of building list
		WidgetLBClear(IDC_ACTIONLIST);
		CurAction = Active->GetActionList();
		FoundPos = -1;
		while (CurAction)
			{
			sprintf(ActionName, "* %s", TranslateActionName(CurAction->ActionType));
			InsertPos = WidgetLBInsert(IDC_ACTIONLIST, -1, ActionName);
			WidgetLBSetItemData(IDC_ACTIONLIST, InsertPos, (void *)CurAction);
			if (CurAction == ActiveAction)
				FoundPos = InsertPos;
			CurAction = CurAction->Next;
			} // while
		WidgetLBSetCurSel(IDC_ACTIONLIST, FoundPos);
		// select the right action in the combo box
		for (ActionCt = 0; ActionCt < WCS_SXQUERYACTION_NUMACTIONTYPES; ActionCt ++)
			{
			if (ActiveAction->ActionType == (char)WidgetCBGetItemData(IDC_NV_ACTIONTYPE, ActionCt))
				{
				WidgetCBSetCurSel(IDC_NV_ACTIONTYPE, ActionCt);
				break;
				} // if
			} // for
		// place action text in text edit field
		if (ActiveAction->ActionText)
			WidgetSetText(IDC_NV_ACTIONTEXT, ActiveAction->ActionText);
		else
			WidgetSetText(IDC_NV_ACTIONTEXT, "");
		// hide/show appropriate widgets
		HideActionOptions();
		BuildActionItemList();
		return;
		} // if
	} // if
// in all other cases
Active->SetActiveAction(NULL);
WidgetLBClear(IDC_ACTIONLIST);
WidgetSetText(IDC_NV_ACTIONTEXT, "");
WidgetCBSetCurSel(IDC_NV_ACTIONTYPE, -1);
ClearActionItemList();

} // ExporterEditGUI::ConfigureActiveAction

/*===========================================================================*/

void ExporterEditGUI::SelectActionType(void)
{
long Current;
char NewType;

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && ActiveAction)
		{
		if ((Current = WidgetCBGetCurSel(IDC_NV_ACTIONTYPE)) != CB_ERR)
			{
			NewType = (char)WidgetCBGetItemData(IDC_NV_ACTIONTYPE, Current);
			ActiveAction->SetActionType(NewType);
			ConfigureActiveAction();
			} // if
		} // if
	} // if

} // ExporterEditGUI::SelectActionType

/*===========================================================================*/

void ExporterEditGUI::SelectActiveAction(void)
{
SXQueryAction *NewAction;
long Current;

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction())
		{
		if ((Current = WidgetLBGetCurSel(IDC_ACTIONLIST)) != LB_ERR)
			{
			if ((NewAction = (SXQueryAction *)WidgetLBGetItemData(IDC_ACTIONLIST, Current)) != (SXQueryAction *)LB_ERR)
				{
				ActiveAction = NewAction;
				ConfigureActiveAction();
				} // if
			} // if
		} // if
	} // if

} // ExporterEditGUI::SelectActiveAction

/*===========================================================================*/

void ExporterEditGUI::AddActionItem(void)
{
long ItemData, Current;

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && ActiveAction)
		{
		Current = WidgetCBGetCurSel(IDC_NV_CLASSDROP);
		ItemData = (long)WidgetCBGetItemData(IDC_NV_CLASSDROP, Current, 0);

		if (ItemData >= WCS_EFFECTSSUBCLASS_LAKE && ItemData < WCS_MAXIMPLEMENTED_EFFECTS)
			EffectsHost->AddAttributeByList(Active, ItemData);
		else if (ItemData == WCS_RAHOST_OBJTYPE_VECTOR)
			{
			ActiveAction->AddDBItem(Active);
			} // else if
		} // if
	} // if

} // ExporterEditGUI::AddActionItem

/*===========================================================================*/

void ExporterEditGUI::RemoveActionItem(void)
{
RasterAnimHost **RemoveItems;
long Ct, Found, NumListEntries, NumSelected = 0;
//int RemoveAll = 0;

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && ActiveAction)
		{
		if ((NumListEntries = WidgetLBGetCount(IDC_NV_ITEMLIST)) > 0)
			{
			for (Ct = 0; Ct < NumListEntries; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_NV_ITEMLIST, Ct))
					{
					NumSelected ++;
					} // if
				} // for
			if (NumSelected)
				{
				if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
					{
					for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
						{
						if (WidgetLBGetSelState(IDC_NV_ITEMLIST, Ct))
							{
							RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_NV_ITEMLIST, Ct);
							} // if
						} // for
					for (Ct = 0; Ct < NumSelected; Ct ++)
						{
						if (RemoveItems[Ct])
							{
							ActiveAction->RemoveRAHost(RemoveItems[Ct], Active);
							} // if
						} // for
					AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
					} // if
				} // if
			else
				{
				UserMessageOK("Remove Item", "There are no Items selected to remove.");
				} // else
			} // if
		} // if
	} // if

} // ExporterEditGUI::RemoveActionItem

/*===========================================================================*/

void ExporterEditGUI::GrabActionAll(void)
{
long ItemData, Current;

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && ActiveAction)
		{
		Current = WidgetCBGetCurSel(IDC_NV_CLASSDROP);
		ItemData = (long)WidgetCBGetItemData(IDC_NV_CLASSDROP, Current, 0);

		ActiveAction->AddItemsByClass(EffectsHost, DBHost, ItemData, Active);
		} // if
	} // if

} // ExporterEditGUI::GrabActionAll

/*===========================================================================*/

void ExporterEditGUI::GrabActionQuery(void)
{

if (Active->FormatExtension)
	{
	if (Active->FormatExtension->GetSupportsQueryAction() && ActiveAction)
		{
		EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_SEARCHQUERY);
		} // if
	} // if

} // ExporterEditGUI::GrabActionQuery

/*===========================================================================*/

void ExporterEditGUI::BackupPrevBounds(void)
{

PrevNorthValue =  Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH)->CurValue;
PrevSouthValue =  Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH)->CurValue;
PrevEastValue =  Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST)->CurValue;
PrevWestValue =  Active->GeoReg.GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST)->CurValue;

} // ExporterEditGUI::BackupPrevBounds
