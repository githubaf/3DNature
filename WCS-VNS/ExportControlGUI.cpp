// ExportControlGUI.cpp
// Code for ExportControlGUI
// Built from scratch on 4/23/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ExportControlGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Interactive.h"
#include "Render.h"
#include "Raster.h"
#include "AppMem.h"
#include "RenderPreviewGUI.h"
#include "WCSVersion.h"
#include "EDSSControlGUI.h"
#include "Conservatory.h"
#include "ImageOutputEvent.h"
#include "Realtime.h"
#include "RasterResampler.h"
#include "ExportFormat.h"
#include "ExporterEditGUI.h"
#include "FeatureConfig.h"
#include "ImageFormat.h"
#include "resource.h"
#include "Script.h" // for network script feedback in BusyWin
#include "Lists.h"

extern NotifyTag FreezeEvent[2];
extern NotifyTag ThawEvent[2];

#ifdef WCS_BUILD_DEMO
int SXDemoWarned;
#endif // WCS_BUILD_DEMO

NativeGUIWin ExportControlGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ExportControlGUI::Open

/*===========================================================================*/

NativeGUIWin ExportControlGUI::Construct(void)
{
#ifdef WCS_BUILD_RTX
int Index;
#endif // WCS_BUILD_RTX
char *PriorityLevels[] = {"Low", "Medium", "High"};

#ifdef WCS_BUILD_RTX
if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_EXPORTCONTROL, NULL);

	if(NativeWin)
		{
		for (Index = 0; Index < 3; Index ++)
			{
			WidgetCBInsert(IDC_PRIORITYDROP, -1, PriorityLevels[Index]);
			} // for
		ConfigureWidgets();
		} // if
	} // if
#endif // WCS_BUILD_RTX
 
return (NativeWin);

} // ExportControlGUI::Construct

/*===========================================================================*/

ExportControlGUI::ExportControlGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, 
	Project *ProjectSource)
: GUIFenetre('ECTL', this, 
APP_TLA" Export Control"
) // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_EXPORTER, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff),
								0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
ProjectHost = ProjectSource;
ActiveJob = NULL;
Rend = NULL;
Rendering = 0;
FrdWarned = 0;
RTFWriteConfig = NULL;
ExportFmt = NULL;

AText[0] = FText[0] = PText[0] = IText[0] = RText[0] = FRText[0] =
ProgCap[0] = PrevCap[0] = FrameTitle[0] =
	Pause = Run = 0;
strcpy(FrameTitle, "Operation:");

PMaxSteps = PCurSteps = FMaxSteps = FCurSteps = AMaxSteps = ACurSteps = 0; // these are unsigned long
FStartSeconds = AStartSeconds = 0; // these are time_t
Preview = 0;

#ifdef WCS_BUILD_RTX
if ((GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0) && EffectsHost && DBHost)
	{
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
#endif // WCS_BUILD_RTX
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_NODOCK);

#ifdef WCS_BUILD_DEMO
if (! SXDemoWarned)
	{
	#ifdef WCS_BUILD_VNS
	UserMessageOK("Export Controller", "Scene Express is an add-on product and is not part of the basic VNS program. It is enabled in this Demo version so you can see what it looks like but you can't actually save any exported files. Be sure to specify that you want Scene Express when you purchase VNS.");
	#else // WCS_BUILD_VNS
	UserMessageOK("Export Controller", "Scene Express is an add-on product and is not part of the basic WCS program. It is enabled in this Demo version so you can see what it looks like but you can't actually save any exported files. Be sure to specify that you want Scene Express when you purchase WCS.");
	#endif // WCS_BUILD_VNS
	SXDemoWarned = 1;
	} // if
#endif // WCS_BUILD_DEMO

} // ExportControlGUI::ExportControlGUI

/*===========================================================================*/

ExportControlGUI::~ExportControlGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
if (Rend)
	delete Rend;

} // ExportControlGUI::~ExportControlGUI()

/*===========================================================================*/

long ExportControlGUI::HandleCloseWin(NativeGUIWin NW)
{

if(Activity->Origin->FenID == 'REND')
	{
	Preview = 0;
	if (Rend)
		Rend->ClosePreview();
	WidgetSetCheck(IDC_DISPLAYPREV, Preview);
	return(1);
	} // if
else
	{
	if (! Run)
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EXG, 0);
		} // if
	else
		{
		Run = 0;
		} // else
	return(1);
	} // else

} // ExportControlGUI::HandleCloseWin

/*===========================================================================*/

long ExportControlGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
#ifdef _WIN32
MSG msg;
#endif // _WIN32

switch(ButtonID)
	{
	case ID_KEEP:
		{
		ExportStop();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EXG, 0);
		break;
		} // 
	case IDC_EDIT1:
		{
		if (! Rendering && ActiveJob)
			ActiveJob->EditRAHost();
		break;
		} //
	case IDC_ADDJOB:
		{
		if (! Rendering)
			EnableJob();
		break;
		} //
	case IDC_REMOVEJOB:
		{
		if (! Rendering)
			DisableJob();
		break;
		} //
	case IDC_MOVEJOBUP:
		{
		ChangeJobPriority(1);
		break;
		} //
	case IDC_MOVEJOBDOWN:
		{
		ChangeJobPriority(-1);
		break;
		} //
	case IDC_GO:
		{
		ExportGo();
		break;
		} //
	case IDC_DISPLAYPREV:
		{
		Preview = (char)WidgetGetCheck(IDC_DISPLAYPREV);
		UpdatePreviewState();
		break;
		} // 
	case IDC_PAUSE:
		{
		if (Run)
			{
			Pause = ! Pause;
			if (Pause)
				{
				WidgetSetText(IDC_PAUSE, "Resume");
				memcpy(PauseText, PText, sizeof(PauseText));
				//SetProcText("Paused");
				if (Rend->PlotFromFrags)
					{
					Rend->CollapseMap();
					} // if
				ProcUpdate(PCurSteps, "Paused");
				// Do pause stuff
				#ifdef _WIN32
				while (Run && Pause && GlobalApp->WinSys->CheckEvent(&msg))
					{
					GlobalApp->ProcessOSEvent(&msg);
					} // while
				#endif // _WIN32
				memcpy(PText, PauseText, sizeof(PauseText));
				// if rendering with fragments update bitmaps from fragments
				SyncTexts();
				} // if
			else
				{
				WidgetSetText(IDC_PAUSE, "Pause");
				} // else
			return(1);
			} // if
		break;
		} // PAUSE
	case IDC_STOPRENDER:
		{
		WidgetSetText(IDC_PAUSE, "Pause");
		Run = 0;
		return(1);
		} // STOP
	default:
		break;
	} // ButtonID

return(0);

} // ExportControlGUI::HandleButtonClick

/*===========================================================================*/

long ExportControlGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_JOBLIST:
		{
		SetActiveJob();
		break;
		} // IDC_JOBLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // ExportControlGUI::HandleListSel

/*===========================================================================*/

long ExportControlGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_JOBLIST:
		{
		if (! Rendering)
			DisableJob();
		break;
		} // IDC_JOBLIST
	default:
		break;
	} // switch

return(0);

} // ExportControlGUI::HandleListDelItem

/*===========================================================================*/

long ExportControlGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_JOBLIST:
		{
		if (! Rendering && ! GlobalApp->EDSSEnabled)
			EditJob();
		break;
		} // IDC_JOBLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // ExportControlGUI::HandleListDoubleClick

/*===========================================================================*/

long ExportControlGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_PRIORITYDROP:
		{
		PriorityLevel();
		break;
		}
	default:
		break;
	} // switch

return (0);

} // ExportControlGUI::HandleCBChange

/*===========================================================================*/

long ExportControlGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_DISPLAYPREV:
		{
		Preview = (char)WidgetGetCheck(IDC_DISPLAYPREV);
		UpdatePreviewState();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // ExportControlGUI::HandleSCChange

/*===========================================================================*/

void ExportControlGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[3];
int Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_EXPORTER, WCS_EFFECTSSUBCLASS_EXPORTER, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_EXPORTER, WCS_EFFECTSSUBCLASS_EXPORTER, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildJobList();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // ExportControlGUI::HandleNotifyEvent()

/*===========================================================================*/

void ExportControlGUI::ConfigureWidgets(void)
{

ConfigureTB(NativeWin, IDC_ADDJOB, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_REMOVEJOB, IDI_DISABLE, NULL);
ConfigureTB(NativeWin, IDC_MOVEJOBUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEJOBDOWN, IDI_ARROWDOWN, NULL);

WidgetSetCheck(IDC_DISPLAYPREV, Preview);

WidgetCBSetCurSel(IDC_PRIORITYDROP, ProjectHost->InquireRenderPri() + 1);

BuildJobList();
DisableWidgets();

} // ExportControlGUI::ConfigureWidgets()

/*===========================================================================*/

void ExportControlGUI::ConfigureJobWidgets(void)
{
char TextStr[512];

if (EffectsHost->IsEffectValid(ActiveJob, WCS_EFFECTSSUBCLASS_EXPORTER, 0))
	{
	WidgetSetText(IDC_TITLE1, ActiveJob->Name);
	strcpy(TextStr, ActiveJob->ExportTarget);
	if ((ActiveJob->ExportTexture || ActiveJob->Export3DObjects || ActiveJob->Export3DFoliage
		|| ActiveJob->ExportWalls || ActiveJob->ExportSky || ActiveJob->ExportCelest || ActiveJob->ExportClouds
		|| ActiveJob->ExportStars || ActiveJob->ExportAtmosphere) && ActiveJob->ImageFormat[0])
		{
		strcat(TextStr, ", Texture: ");
		strcat(TextStr, ActiveJob->ImageFormat);
		} // if
	if (ActiveJob->ExportFoliage && ActiveJob->FoliageImageFormat[0])
		{
		strcat(TextStr, ", Foliage: ");
		strcat(TextStr, ActiveJob->FoliageImageFormat);
		} // if
	#ifdef WCS_LABEL
	if (ActiveJob->ExportLabels && ActiveJob->FoliageImageFormat[0])
		{
		strcat(TextStr, ", Labels: ");
		strcat(TextStr, ActiveJob->FoliageImageFormat);
		} // if
	#endif // WCS_LABEL

	WidgetSetText(IDC_TITLE7, TextStr);

	TextStr[0] = 0;
	if (ActiveJob->ExportTerrain)
		strcat(TextStr, "Terrain, ");
	if (ActiveJob->ExportTexture)
		strcat(TextStr, "Texture, ");
	if (ActiveJob->ExportVectors)
		strcat(TextStr, "Vectors, ");
	if (ActiveJob->ExportFoliage)
		strcat(TextStr, "Foliage, ");
	#ifdef WCS_LABEL
	if (ActiveJob->ExportLabels)
		strcat(TextStr, "Labels, ");
	#endif // WCS_LABEL
	if (ActiveJob->Export3DObjects)
		strcat(TextStr, "3D Objects, ");
	if (ActiveJob->Export3DFoliage)
		strcat(TextStr, "3D Foliage, ");
	if (ActiveJob->ExportWalls)
		strcat(TextStr, "Walls, ");
	if (ActiveJob->ExportSky || ActiveJob->ExportCelest || ActiveJob->ExportClouds ||
		ActiveJob->ExportStars || ActiveJob->ExportAtmosphere)
		strcat(TextStr, "Sky Features, ");
	if (ActiveJob->ExportCameras && ActiveJob->Cameras)
		strcat(TextStr, "Cameras, ");
	if (ActiveJob->ExportLights && ActiveJob->Lights)
		strcat(TextStr, "Lights, ");
	if (ActiveJob->ExportHaze && ActiveJob->Haze)
		strcat(TextStr, "Haze, ");
	if (TextStr[0])
		TextStr[strlen(TextStr) - 2] = 0;
	WidgetSetText(IDC_TITLE3, TextStr);

	// options
	TextStr[0] = 0;
	if (ActiveJob->ExportTexture && ActiveJob->BurnShading)
		strcat(TextStr, "Burn Shading, ");
	if (ActiveJob->ExportTexture && ActiveJob->BurnShading && ActiveJob->BurnShadows)
		strcat(TextStr, "Burn Shadows, ");
	if (ActiveJob->ExportTexture && ActiveJob->BurnVectors)
		strcat(TextStr, "Burn Vectors, ");
	if (ActiveJob->ExportWalls && ActiveJob->BurnWallShading)
		strcat(TextStr, "Burn Wall Shadows, ");
	if (ActiveJob->ExportWalls && ActiveJob->TileWallTex)
		strcat(TextStr, "Tile Wall Textures, ");
	if (ActiveJob->ExportVectors && ActiveJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES)
		strcat(TextStr, "Vectors as Lines, ");
	else if (ActiveJob->ExportVectors && ActiveJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS)
		strcat(TextStr, "Vectors as 2-pt Polygons, ");
	else if (ActiveJob->ExportVectors && ActiveJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS)
		strcat(TextStr, "Vectors as 3-pt Polygons, ");
	if (ActiveJob->ExportVolumetrics)
		strcat(TextStr, "Volumetrics, ");
	if (ActiveJob->ZipItUp)
		strcat(TextStr, "Zip Output, ");
	if (TextStr[0])
		TextStr[strlen(TextStr) - 2] = 0;
	WidgetSetText(IDC_TITLE8, TextStr);
	} // if

} // ExportControlGUI::ConfigureJobWidgets

/*===========================================================================*/

void ExportControlGUI::BuildJobList(void)
{
SceneExporter *Current, **JobList;
long Done, Pos, Found = 0, JobCt = 0;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

WidgetLBClear(IDC_JOBLIST);

// build a temporary list of jobs that can be sorted by priority
// count jobs
for (Current = (SceneExporter *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_EXPORTER); 
	Current; Current = (SceneExporter *)Current->Next)	//lint !e722
	{
	if (FormatSpecificAuth(Current->ExportTarget))
		JobCt ++;
	else
		{
		if(Current->Enabled) // only nag about enabled jobs, otherwise we get really annoying
			{
			UserMessageOK(Current->ExportTarget, "An unauthorized Export Format has been detected. This format is not available to you at this time. Please contact 3D Nature to obtain authorization.");
			} // if
		} // else
	}

if (JobCt > 0)
	{
	if (JobList = (SceneExporter **)AppMem_Alloc(JobCt * sizeof (SceneExporter *), APPMEM_CLEAR))
		{
		for (Pos = 0, Current = (SceneExporter *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_EXPORTER); 
			Current; Current = (SceneExporter *)Current->Next)
			{
			if (FormatSpecificAuth(Current->ExportTarget))
				JobList[Pos ++] = Current;
			} // for

		Done = 0;
		while (! Done)
			{
			Done = 1;
			for (Pos = 1; Pos < JobCt; Pos ++)
				{
				if (JobList[Pos - 1]->Priority < JobList[Pos]->Priority)
					{
					swmem(&JobList[Pos - 1], &JobList[Pos], sizeof (SceneExporter *));
					Done = 0;
					} // if
				} // for
			} // while

		for (Pos = 0; Pos < JobCt; Pos ++)
			{
			BuildJobListEntry(ListName, JobList[Pos]);
			WidgetLBInsert(IDC_JOBLIST, -1, ListName);
			WidgetLBSetItemData(IDC_JOBLIST, Pos, JobList[Pos]);
			if (JobList[Pos] == ActiveJob)
				{
				Found = 1;
				WidgetLBSetCurSel(IDC_JOBLIST, Pos);
				} // if
			} // for
		AppMem_Free(JobList, JobCt * sizeof (SceneExporter *));
		} // if

	if (! Found)
		{
		if (ActiveJob = (SceneExporter *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_EXPORTER))
			{
			WidgetLBSetCurSel(IDC_JOBLIST, 0);
			}
		} // if
	} // if

ConfigureJobWidgets();

} // ExportControlGUI::BuildJobList

/*===========================================================================*/

void ExportControlGUI::BuildJobListEntry(char *ListName, SceneExporter *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->Name);

} // ExportControlGUI::BuildJobListEntry()

/*===========================================================================*/

void ExportControlGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_ADDJOB, Rendering || GlobalApp->EDSSEnabled);
WidgetSetDisabled(IDC_REMOVEJOB, Rendering || GlobalApp->EDSSEnabled);
WidgetSetDisabled(IDC_MOVEJOBUP, Rendering);
WidgetSetDisabled(IDC_MOVEJOBDOWN, Rendering);
WidgetSetDisabled(IDC_EDIT1, Rendering || GlobalApp->EDSSEnabled);
WidgetSetDisabled(IDC_EDIT2, Rendering || GlobalApp->EDSSEnabled);
WidgetSetDisabled(IDC_GO, Rendering);
WidgetSetDisabled(IDC_PAUSE, ! Rendering);
WidgetSetDisabled(IDC_STOPRENDER, ! Rendering);

} // ExportControlGUI::DisableWidgets

/*===========================================================================*/

void ExportControlGUI::EnableJob(void)
{
NotifyTag Changes[2];

if (ActiveJob)
	{
	ActiveJob->Enabled = 1;
	Changes[0] = MAKE_ID(ActiveJob->GetNotifyClass(), ActiveJob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveJob->GetRAHostRoot());
	} // if

} // ExportControlGUI::EnableJob

/*===========================================================================*/

void ExportControlGUI::DisableJob(void)
{
NotifyTag Changes[2];

if (ActiveJob)
	{
	ActiveJob->Enabled = 0;
	Changes[0] = MAKE_ID(ActiveJob->GetNotifyClass(), ActiveJob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveJob->GetRAHostRoot());
	} // if

} // ExportControlGUI::DisableJob

/*===========================================================================*/

void ExportControlGUI::ChangeJobPriority(short Direction)
{
long Current;
SceneExporter *NextItem;
NotifyTag Changes[2];

if (ActiveJob)
	{
	if ((Current = WidgetLBGetCurSel(IDC_JOBLIST)) != LB_ERR)
		{
		if (Direction > 0)
			{
			if (Current > 0)
				{
				// get priority of next job up in list
				if ((NextItem = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Current - 1)) && NextItem != (SceneExporter *)LB_ERR)
					{
					if (NextItem->Priority + 1 < SHRT_MAX)
						ActiveJob->Priority = NextItem->Priority + 1;
					} // if
				} // if
			} // if
		else
			{
			if (WidgetLBGetCount(IDC_JOBLIST) > Current + 1)
				{
				// get priority of next job down in list
				if ((NextItem = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Current + 1)) && NextItem != (SceneExporter *)LB_ERR)
					{
					if (NextItem->Priority - 1 > SHRT_MIN)
						ActiveJob->Priority = NextItem->Priority - 1;
					} // if
				} // if
			} // else
		Changes[0] = MAKE_ID(ActiveJob->GetNotifyClass(), ActiveJob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveJob->GetRAHostRoot());
		} // if
	} // if

} // ExportControlGUI::ChangeJobPriority

/*===========================================================================*/

void ExportControlGUI::EditJob(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_JOBLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_JOBLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_JOBLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // ExportControlGUI::EditJob

/*===========================================================================*/

void ExportControlGUI::SetActiveJob(void)
{
long Current;
SceneExporter *NewJob;

Current = WidgetLBGetCurSel(IDC_JOBLIST);
if ((NewJob = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Current, 0)) != (SceneExporter *)LB_ERR && NewJob)
	{
	if (NewJob != ActiveJob)
		{
		ActiveJob = NewJob;
		ConfigureJobWidgets();
		} // if
	} // if
else
	ConfigureJobWidgets();

} // ExportControlGUI::SetActiveJob

/*===========================================================================*/

void ExportControlGUI::PriorityLevel(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_PRIORITYDROP);
ProjectHost->SetRenderPri((short)(Current - 1));

} // ExportControlGUI::PriorityLevel

/*===========================================================================*/

void ExportControlGUI::ExportGo(void)
{
#ifdef WCS_BUILD_RTX
#ifndef WCS_BUILD_DEMO
long FractalMethodStash, BackfaceCullStash, Ct, NumListEntries;
SceneExporter *CurJob;
int Success = 1;
char StashPreviewState;

if (! (GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0))
	return;

Rendering = 1;
Run = 1;
DisableWidgets();

FractalMethodStash = EffectsHost->GetFractalMethod();
BackfaceCullStash = EffectsHost->GetBackFaceCull();

StashPreviewState = Preview;	// store to recall after renderer is deleted
if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	delete Rend;	// may have been created by previous rendering
	Rend = NULL;
	Preview = StashPreviewState;
	} // if

if ((NumListEntries = WidgetLBGetCount(IDC_JOBLIST)) > 0)
	{
	// back up paths
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if ((CurJob = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Ct)) && (CurJob != (SceneExporter *)LB_ERR))
			{
			if (CurJob->Enabled)
				{
				CurJob->BackupPath();
				} // if
			} // if
		} // for

	// sanity check each job
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if ((CurJob = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Ct)) && (CurJob != (SceneExporter *)LB_ERR))
			{
			if (CurJob->Enabled)
				{
				WidgetLBSetCurSel(IDC_JOBLIST, Ct);	// show current job in list
				SetActiveJob();		// this displays the job parameters in the interface
				if (! strnicmp(CurJob->ExportTarget, "VNS", 7))
					{
					// see if the user-defined path is available
					if (ValidateExportPaths(CurJob))
						{
						char TempStash[1024];
						// now add a project directory
						strmfp(TempStash, CurJob->OutPath.GetPath(), CurJob->OutPath.GetName());
						CurJob->OutPath.SetPath(TempStash);
						} // if
					else
						{
						Success = 0;
						break;
						} // else
					} // if
				// validate paths or revalidate if VNS and make sure all settings are legal.
				if (! JobTestSettings(CurJob) || ! ValidateExportPaths(CurJob))
					{
					Success = 0;
					break;
					} // if sanity check failed
				} // if enabled
			} // if
		} // for

	// best to update interface of SceneExporter in case sanity checking uncovered bad settings and corrected them
	if (GlobalApp->GUIWins->EXP)
		GlobalApp->GUIWins->EXP->SyncWidgets();

	// export each job
	for (Ct = 0; Success && Ct < NumListEntries; Ct ++)
		{
		if ((CurJob = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Ct)) && (CurJob != (SceneExporter *)LB_ERR))
			{
			if (FormatSpecificAuth(CurJob->ExportTarget))
				{
				if (CurJob->Enabled)
					{
					StashPreviewState = Preview;	// store to recall after renderer is deleted
					if (Rend)
						{
						ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
						ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
						delete Rend;	// may have been created by previous rendering
						Rend = NULL;
						} // if
					WidgetLBSetCurSel(IDC_JOBLIST, Ct);	// show current job in list
					SetActiveJob();		// this displays the job parameters in the interface
					SetPreview(StashPreviewState);	// restore this setting so preview window can open for next job
					if (! ExportAJob(CurJob))
						break;
					} // if enabled
				} // if
			} // if
		} // for

	// restore output paths if they were changed
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if ((CurJob = (SceneExporter *)WidgetLBGetItemData(IDC_JOBLIST, Ct)) && (CurJob != (SceneExporter *)LB_ERR))
			{
			if (CurJob->Enabled)
				{
				CurJob->RestorePath();
				} // if enabled
			} // if
		} // for
	} // if

EffectsHost->SetFractalMethod((char)FractalMethodStash);
EffectsHost->SetBackFaceCull((char)BackfaceCullStash);
ExportStop();

#else // WCS_BUILD_DEMO
UserMessageDemo("Exports cannot be generated and saved.");
#endif // WCS_BUILD_DEMO
#endif // WCS_BUILD_RTX
} // ExportControlGUI::ExportGo

/*===========================================================================*/

int ExportControlGUI::JobTestSettings(SceneExporter *CurJob)
{
int Success = 0;

if (AllocExportFormat(CurJob->ExportTarget, CurJob))
	{
	Success = ExportFmt->SanityCheck();
	DeleteExportFormat();
	} // if

return (Success);

} // ExportControlGUI::JobTestSettings

/*===========================================================================*/

int ExportControlGUI::ExportAJob(SceneExporter *CurJob)
{
int Success = 1;
#ifndef WCS_BUILD_DEMO	// extra insurance in case RenderGo is bypassed by a direct call in the future
double StashProjectTime;
NameList *FileNamesCreated = NULL, *DelList;

GlobalApp->AppEx->GenerateNotify(FreezeEvent, NULL);

StashProjectTime = ProjectHost->Interactive->GetActiveTime();
SetFrameTextA("Export Begun");
if (AllocExportFormat(CurJob->ExportTarget, CurJob))
	{
	ProjectHost->Interactive->SetActiveTime(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	// update application by time
	GlobalApp->UpdateProjectByTime();
	if (Success = CurJob->InitToExport())
		{
		Success = ValidateExportPaths(CurJob);

		// run scenarios
		#ifdef WCS_RENDER_SCENARIOS
		EffectsHost->InitScenarios(CurJob->Scenarios, CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue, DBHost);
		#endif // WCS_RENDER_SCENARIOS

		if (Success && (CurJob->ExportTerrain || CurJob->ExportTexture || CurJob->ExportFoliage || CurJob->ExportLabels || CurJob->Export3DFoliage))
			{
			Success = ExportTerrain(CurJob, &FileNamesCreated);
			} // if
		else
			{
			CurJob->SetupRBounds(DBHost, EffectsHost);
			} // else
		if (Success && (CurJob->ExportFoliage || CurJob->ExportLabels))
			{
			if (Success = ExportFoliageImages(CurJob, &FileNamesCreated))
				{
				// save foliage of specific image as an object
				if (CurJob->FoliageInstancesAsObj && CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER)
					{
					if (Success = ExportFoliageAsNonAligned3DObjects(CurJob, &FileNamesCreated))
						Success = ExportFoliageInstancesAs3DObjects(CurJob, &FileNamesCreated);
					} // if
				// save crossboards as objects 
				else if (CurJob->FoliageAsObj && CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS)
					Success = ExportFoliageAs3DObjects(CurJob, &FileNamesCreated, FALSE, FALSE);
				// save flipboards as objects if it is Lightwave
				else if (CurJob->FoliageAsObj && CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS
					&& CurJob->AlignFlipBoardsToCamera)
					Success = ExportFoliageAsAligned3DObjects(CurJob, &FileNamesCreated);
				} // if
			} // if
		if (Success && (CurJob->ExportSky || CurJob->ExportCelest || CurJob->ExportClouds || CurJob->ExportStars))
			{
			if (Success = ExportSky(CurJob, &FileNamesCreated))
				{
				if (CurJob->SkyAsObj)
					Success = ExportSkyAs3DObject(CurJob, &FileNamesCreated);
				} // if
			} // if
		if (Success && (CurJob->ExportVectors))
			{
			if (Success = ExportVectors(CurJob, &FileNamesCreated))
				{
				if ((CurJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS ||
					CurJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS) && CurJob->VectorAsObj)
					Success = ExportVectorsAs3DObjects(CurJob, &FileNamesCreated);
				} // if
			} // if
		if (Success && (CurJob->Export3DObjects || CurJob->Export3DFoliage))
			{
			Success = Export3DObjects(CurJob, &FileNamesCreated);
			} // if
		if (Success && (CurJob->ExportWalls))
			{
			Success = ExportWalls(CurJob, &FileNamesCreated);
			} // if
		if (Success)
			{
			Success = PrepExportItems(CurJob, &FileNamesCreated);
			} // if
		PurgeFoliageFiles(CurJob, &FileNamesCreated);
		if (Success)
			{
			if (Success = ResampleExports(CurJob, &FileNamesCreated))
				{
				if (CurJob->ExportTerrain && CurJob->TerrainAsObj)
					{
					Success = ExportTerrainAs3DObject(CurJob, &FileNamesCreated);
					} // if
				if (Success && FormatSpecificAuth(CurJob->ExportTarget))
					{
					SetFrameTextA("Finalizing Export");
					CurJob->RBounds.DeriveCoords(CurJob->DEMResY, CurJob->DEMResX);
					Success =  ConvertWCSGeoToExportCoords(CurJob, &FileNamesCreated);
					if (Success && (Success = ExportFmt->PackageExport(&FileNamesCreated)))
						{
						Success = ZipItAndShipIt(CurJob, &FileNamesCreated);
						} // if
					} // if
				} // if
			} // if

		#ifdef WCS_RENDER_SCENARIOS
		EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		#endif // WCS_RENDER_SCENARIOS

		while (FileNamesCreated)
			{
			DelList = FileNamesCreated;
			FileNamesCreated = FileNamesCreated->Next;
			delete DelList;
			} // while
		} // if init SceneExporter successful
	DeleteExportFormat();
	CurJob->CleanupFromExport();
	// reset time value just to be sure - some process might have changed it for some items and not reset it globally
	// since it might have been needed for finalizing export of items like cameras
	ProjectHost->Interactive->SetActiveTime(StashProjectTime);
	// update application by time
	GlobalApp->UpdateProjectByTime();
	} // if
ProcClear();
// return to program directory so that lock on created directories is freed
chdir(GlobalApp->GetProgDir());

if (Success)
	{
	SetFrameTextA("Export Completed");
	GlobalApp->WinSys->DoBeep();
	} // if
else
	SetFrameTextA("Export Aborted");

GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);

#endif // WCS_BUILD_DEMO

return (Success);

} // ExportControlGUI::ExportAJob

/*===========================================================================*/

ExportFormat *ExportControlGUI::AllocExportFormat(char *Target, SceneExporter *CurJob)
{

DeleteExportFormat();

if (! stricmp(Target, "NatureView"))
	{
	ExportFmt = new ExportFormatNV(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "VTP"))
	{
	ExportFmt = new ExportFormatVTP(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "3DS"))
	{
	ExportFmt = new ExportFormat3DS(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "VRML"))
	{
	ExportFmt = new ExportFormatVRML(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "VRML-STL"))
	{
	ExportFmt = new ExportFormatVRMLSTL(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "STL"))
	{
	ExportFmt = new ExportFormatSTL(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "OpenFlight"))
	{
	ExportFmt = new ExportFormatOpenFlight(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "LightWave"))
	{
	ExportFmt = new ExportFormatLW(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "COLLADA"))
	{
	ExportFmt = new ExportFormatCOLLADA(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
#ifdef WCS_BUILD_FBX
if (! stricmp(Target, "FBX"))
	{
	ExportFmt = new ExportFormatFBX(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
#endif // WCS_BUILD_FBX
if (! stricmp(Target, "GIS"))
	{
	ExportFmt = new ExportFormatGIS(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "WorldWind"))
	{
	ExportFmt = new ExportFormatWW(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "VNS"))
	{
	ExportFmt = new ExportFormatWCSVNS(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "GoogleEarth"))
	{
	ExportFmt = new ExportFormatKML(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if
if (! stricmp(Target, "Custom"))
	{
	ExportFmt = new ExportFormatCustom(CurJob, ProjectHost, EffectsHost, DBHost, ImageHost);
	} // if

return (ExportFmt);

} // ExportControlGUI::AllocExportFormat

/*===========================================================================*/

void ExportControlGUI::DeleteExportFormat(void)
{

if (ExportFmt)
	delete ExportFmt;
ExportFmt = NULL;

} // ExportControlGUI::DeleteExportFormat

/*===========================================================================*/

int ExportControlGUI::ExportTerrain(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double RenderWidth, RenderHeight, PlanetRad, PixResX, PixResY;
float DBMaxEl, DBMinEl;
long FractalDepth = 0, FractalDepthStash, ImageWidth, ImageHeight, ShadowObjects, Ct;
TerrainParamEffect *DefaultPar;
PlanetOpt *DefPlanetOpt;
Camera *CurCam;
RenderOpt *CurOpt;
RenderJob *CurRJ;
Object3DEffect *CurObj;
ShadowEffect *CurShadow;
Object3DEffect **ShadowObjectList = NULL;
ImageOutputEvent *CurEvent;
char *RawExtension;
int Success = 1, ShadowsToRender;
char FormatStr[256];


#ifndef WCS_BUILD_VNS
// largest DEM and tex res in WCS is 2049
// set so it blows up later
// shouldn't have gotten here unless something hacked.
if ((CurJob->DEMTilesX * CurJob->OneDEMResX > 2049) ||
	(CurJob->DEMTilesY * CurJob->OneDEMResY > 2049) ||
	(CurJob->TexTilesX * CurJob->OneTexResX > 2049) ||
	(CurJob->TexTilesY * CurJob->OneTexResY > 2049))
	{
	Success = 0;
	UserMessageOK("Export Error", "Too many rows or columns of either terrain or texture. WCS is limited to exports of 2049 cells in either direction.");
	return (0);
	} // if
#endif // WCS_BUILD_VNS

// set fractal method to automatic calculation or leave as is by user's choice
if (CurJob->FractalMethod == WCS_FRACTALMETHOD_CONSTANT)
	EffectsHost->SetFractalMethod(CurJob->FractalMethod);
EffectsHost->SetBackFaceCull(0);
SetFrameTextA("Exporting Terrain/Texture/Foliage");

RawExtension = ImageSaverLibrary::GetDefaultExtension("Raw");

// if exporting terrain or imagery, create a camera and render opts
if (DefaultPar = (TerrainParamEffect *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, NULL))
	{
	FractalDepthStash = DefaultPar->FractalDepth;
	if ((CurCam = new Camera) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob))
		{
		DBHost->GetDEMElevRange(DBMaxEl, DBMinEl);
		if (DefPlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
			{
			DBMaxEl = (float)CalcExag((double)DBMaxEl, DefPlanetOpt);
			DBMinEl = (float)CalcExag((double)DBMinEl, DefPlanetOpt);
			} // if
		CurJob->ReplaceNULLElev = (float)(DBMinEl - fabs((DBMaxEl - DBMinEl) * .1));
		PlanetRad = EffectsHost->GetPlanetRadius();
		//CurJob->GeoReg.GetCenterLatLon(CurJob->Coords, CamLat, CamLon);	// got the lat and lon in the export coord sys, not the default cs as needed
		CurJob->GeoReg.GetMetricWidthHeight(CurJob->Coords, PlanetRad, RenderWidth, RenderHeight);
		if (CurJob->ExportTexture)
			{
			ImageWidth = CurJob->TexResX;
			ImageHeight = CurJob->TexResY;
			if (CurJob->ExportTerrain)
				{
				ImageWidth = max(ImageWidth, CurJob->DEMResX);
				ImageHeight = max(ImageHeight, CurJob->DEMResY);
				} // if
			} // if
		else
			{
			ImageWidth = CurJob->DEMResX;
			ImageHeight = CurJob->DEMResY;
			} // else
		CurJob->RBounds.SetCoords(CurJob->Coords ? CurJob->Coords: EffectsHost->FetchDefaultCoordSys());
		// add a cell width and height if bounds are centers
		if (CurJob->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
			{
			PixResX = RenderWidth / (ImageWidth - 1);
			RenderWidth += PixResX;
			PixResY = RenderHeight / (ImageHeight - 1);
			RenderHeight += PixResY;
			CurJob->RBounds.SetOutsideBoundsFromCenters(&CurJob->GeoReg, ImageHeight, ImageWidth);
			} // if
		else
			{
			PixResX = RenderWidth / ImageWidth;
			PixResY = RenderHeight / ImageHeight;
			CurJob->RBounds.SetOutsideBounds(&CurJob->GeoReg);
			} // else
		CurJob->RBounds.DeriveCoords(ImageHeight, ImageWidth);
		CurJob->SetupExportReferenceData(PlanetRad, (double)DBMinEl, (double)DBMaxEl);
		// compute some settings
		if (CurJob->FractalMethod == WCS_FRACTALMETHOD_CONSTANT)
			{
			double CellSizeNS, CellSizeWE, MaxCellSize, MinPixRes;

			if (DBHost->GetMinDEMCellSizeMetersMaxCount(CellSizeNS, CellSizeWE, 100, ProjectHost)) // we only sample 100 total DEMs instead of all of them
				{
				// choose the largest cell size to test against pixel res
				// Use half the cell size so that the effective comparison is against the size of two pixels.
				// That seems optimal but might bear watching to see if it is excessive or not detailed enough.
				MaxCellSize = max(CellSizeNS, CellSizeWE) * .5;
				MinPixRes = min(PixResX, PixResY);
				while (MaxCellSize > MinPixRes && FractalDepth < 7)
					{
					FractalDepth ++;
					MaxCellSize *= .5;
					} // while
				} // if
			} // if
		// set camera settings
		CurCam->CameraType = WCS_EFFECTS_CAMERATYPE_PLANIMETRIC;
		if (CurJob->Coords)
			{
			CurCam->Projected = 1;
			CurCam->SetCoords(CurJob->Coords);
			} // if
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CurJob->ExportRefData.WCSRefLat);
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CurJob->ExportRefData.WCSRefLon);
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(max(5000.0, DBMaxEl + 600.0));
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(RenderWidth);
		// set render options
		CurOpt->RenderFromOverhead = 1;
		CurOpt->OutputImageWidth = ImageWidth;
		CurOpt->OutputImageHeight = ImageHeight;
		CurOpt->TilesX = (short)((ImageWidth / 513) + (ImageWidth % 513 ? 1: 0));
		CurOpt->TilesY = (short)((ImageHeight / 513) + (ImageHeight % 513 ? 1: 0));
		CurOpt->TilingEnabled = (CurOpt->TilesX > 0 || CurOpt->TilesY > 0);
		CurOpt->ConcatenateTiles = 1;
		CurOpt->ReflectionsEnabled = 0;
		CurOpt->CloudShadowsEnabled = CurOpt->ObjectShadowsEnabled = CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = (CurJob->BurnShading && CurJob->BurnShadows);
		CurOpt->RenderDiagnosticData = CurOpt->MultiPassAAEnabled = CurOpt->DepthOfFieldEnabled = 0;
		CurOpt->VectorsEnabled = CurJob->BurnVectors;
		CurOpt->FoliageEnabled = CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
			(CurJob->ExportFoliage || CurJob->Export3DFoliage || (CurJob->ExportTexture && (CurJob->TextureFoliageType != WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST))); 
		CurOpt->HazeVectors = 0;
		CurOpt->RenderFromOverhead = 1;
		CurOpt->LuminousVectors = 1;
		CurOpt->TransparentWaterEnabled = 1;
		CurOpt->FragmentRenderingEnabled = 1;
		CurOpt->FragmentDepth = 100;
		CurOpt->VolumetricsEnabled = 0;
		CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].CurValue);
		CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(PixResX / PixResY);
		CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
		CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = EffectsHost->EffectTypeImplemented(WCS_EFFECTSSUBCLASS_LABEL) ? CurJob->ExportLabels: 0;
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 0;
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = CurJob->BurnShading;
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = (CurJob->BurnShadows && CurJob->BurnShading);
		CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->TempPath);
		// turn on building foliage list

		if (CurJob->ExportFoliage || CurJob->ExportLabels || CurJob->Export3DFoliage)
			{
			if (RTFWriteConfig = new (RealTimeFoliageWriteConfig))
				{
				RTFWriteConfig->ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT].CurValue);
				RTFWriteConfig->ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].SetValue((double)FLT_MAX);
				RTFWriteConfig->ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].SetValue(0.0);
				RTFWriteConfig->ConfigParams[WCS_REALTIME_CONFIG_FARDIST].SetValue((double)FLT_MAX);
				RTFWriteConfig->NumFiles = 1;
				RTFWriteConfig->StemsPerCell = LONG_MAX;
				RTFWriteConfig->Include3DO = CurJob->Export3DFoliage ? 1: 0;
				RTFWriteConfig->IncludeImage = CurJob->ExportFoliage ? 1: 0;
				RTFWriteConfig->IncludeLabels = CurJob->ExportLabels && EffectsHost->EffectTypeImplemented(WCS_EFFECTSSUBCLASS_LABEL) ? 1: 0;
				strcpy(RTFWriteConfig->BaseName, CurJob->OutPath.GetName());
				strcat(RTFWriteConfig->BaseName, "_Fol");
				strcpy(RTFWriteConfig->DirName, CurJob->OutPath.GetPath());
				sprintf(FormatStr, "%s.dat", RTFWriteConfig->BaseName);
				AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX);
				sprintf(FormatStr, "%sFoliageList%d.dat", RTFWriteConfig->BaseName, 0);
				AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_FOLFILE);
				} // if
			} // if

		CurJob->FragmentCollapseType = CurJob->TextureFoliageType;

		// create and configure a image output events
		// need to save elevation, antialias, RGB terrain, RGB foliage + terrain (optional)
		if (CurJob->ExportTexture)
			{
			if (CurEvent = CurOpt->AddOutputEvent())
				{
				// set new file type
				strcpy(CurEvent->FileType, "Raw");

				for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
					{
					CurEvent->OutBuffers[Ct][0] = 0;
					CurEvent->BufNodes[Ct] = NULL;
					} // for

				// set new values for buffers and codec
				strcpy(CurEvent->OutBuffers[0], "RED");
				strcpy(CurEvent->Codec, "Full Channel Precision");
				strcpy(FormatStr, CurJob->OutPath.GetName());
				strcat(FormatStr, "_RGB.red");
				CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
				CurEvent->AutoDigits = 0;
				CurEvent->AutoExtension = 0;
				AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX1_RED);
				if (CurEvent = CurOpt->AddOutputEvent())
					{
					// set new file type
					strcpy(CurEvent->FileType, "Raw");

					for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
						{
						CurEvent->OutBuffers[Ct][0] = 0;
						CurEvent->BufNodes[Ct] = NULL;
						} // for

					// set new values for buffers and codec
					strcpy(CurEvent->OutBuffers[0], "GREEN");
					strcpy(CurEvent->Codec, "Full Channel Precision");
					strcpy(FormatStr, CurJob->OutPath.GetName());
					strcat(FormatStr, "_RGB.grn");
					CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
					CurEvent->AutoDigits = 0;
					CurEvent->AutoExtension = 0;
					AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN);
					if (CurEvent = CurOpt->AddOutputEvent())
						{
						// set new file type
						strcpy(CurEvent->FileType, "Raw");

						for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
							{
							CurEvent->OutBuffers[Ct][0] = 0;
							CurEvent->BufNodes[Ct] = NULL;
							} // for

						// set new values for buffers and codec
						strcpy(CurEvent->OutBuffers[0], "BLUE");
						strcpy(CurEvent->Codec, "Full Channel Precision");
						strcpy(FormatStr, CurJob->OutPath.GetName());
						strcat(FormatStr, "_RGB.blu");
						CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
						CurEvent->AutoDigits = 0;
						CurEvent->AutoExtension = 0;
						AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU);
						if (CurEvent = CurOpt->AddOutputEvent())
							{
							// set new file type
							strcpy(CurEvent->FileType, "Raw");

							for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
								{
								CurEvent->OutBuffers[Ct][0] = 0;
								CurEvent->BufNodes[Ct] = NULL;
								} // for

							// set new values for buffers and codec
							strcpy(CurEvent->OutBuffers[0], "ANTIALIAS");
							strcpy(CurEvent->Codec, "Full Channel Precision");
							strcpy(FormatStr, CurJob->OutPath.GetName());
							strcat(FormatStr, "_RGB.alp");
							CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
							CurEvent->AutoDigits = 0;
							CurEvent->AutoExtension = 0;
							AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA);
							} // if
						} // if
					} // if
				} // if

			if (CurJob->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND
				|| CurJob->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND)
				{
				if (CurEvent = CurOpt->AddOutputEvent())
					{
					// set new file type
					strcpy(CurEvent->FileType, "Raw");

					for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
						{
						CurEvent->OutBuffers[Ct][0] = 0;
						CurEvent->BufNodes[Ct] = NULL;
						} // for

					// set new values for buffers and codec
					strcpy(CurEvent->OutBuffers[0], "FOLIAGE RED");
					strcpy(CurEvent->Codec, "Full Channel Precision");
					strcpy(FormatStr, CurJob->OutPath.GetName());
					strcat(FormatStr, "_FolRGB.red");
					CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
					CurEvent->AutoDigits = 0;
					CurEvent->AutoExtension = 0;
					AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX2_RED);
					if (CurEvent = CurOpt->AddOutputEvent())
						{
						// set new file type
						strcpy(CurEvent->FileType, "Raw");

						for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
							{
							CurEvent->OutBuffers[Ct][0] = 0;
							CurEvent->BufNodes[Ct] = NULL;
							} // for

						// set new values for buffers and codec
						strcpy(CurEvent->OutBuffers[0], "FOLIAGE GREEN");
						strcpy(CurEvent->Codec, "Full Channel Precision");
						strcpy(FormatStr, CurJob->OutPath.GetName());
						strcat(FormatStr, "_FolRGB.grn");
						CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
						CurEvent->AutoDigits = 0;
						CurEvent->AutoExtension = 0;
						AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN);
						if (CurEvent = CurOpt->AddOutputEvent())
							{
							// set new file type
							strcpy(CurEvent->FileType, "Raw");

							for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
								{
								CurEvent->OutBuffers[Ct][0] = 0;
								CurEvent->BufNodes[Ct] = NULL;
								} // for

							// set new values for buffers and codec
							strcpy(CurEvent->OutBuffers[0], "FOLIAGE BLUE");
							strcpy(CurEvent->Codec, "Full Channel Precision");
							strcpy(FormatStr, CurJob->OutPath.GetName());
							strcat(FormatStr, "_FolRGB.blu");
							CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
							CurEvent->AutoDigits = 0;
							CurEvent->AutoExtension = 0;
							AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU);
							if (CurEvent = CurOpt->AddOutputEvent())
								{
								// set new file type
								strcpy(CurEvent->FileType, "Raw");

								for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
									{
									CurEvent->OutBuffers[Ct][0] = 0;
									CurEvent->BufNodes[Ct] = NULL;
									} // for

								// set new values for buffers and codec
								strcpy(CurEvent->OutBuffers[0], "FOLIAGE ANTIALIAS");
								strcpy(CurEvent->Codec, "Full Channel Precision");
								strcpy(FormatStr, CurJob->OutPath.GetName());
								strcat(FormatStr, "_FolRGB.alp");
								CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
								CurEvent->AutoDigits = 0;
								CurEvent->AutoExtension = 0;
								AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA);
								} // if
							} // if
						} // if
					} // if
				} // if put foliage in second texture
			} // if export texture

		if (CurJob->ExportTerrain)
			{
			if (CurEvent = CurOpt->AddOutputEvent())
				{
				// set new file type
				strcpy(CurEvent->FileType, "Raw");

				for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
					{
					CurEvent->OutBuffers[Ct][0] = 0;
					CurEvent->BufNodes[Ct] = NULL;
					} // for

				// set new values for buffers and codec
				strcpy(CurEvent->OutBuffers[0], "ELEVATION");
				strcpy(CurEvent->Codec, "Full Channel Precision");
				strcpy(FormatStr, CurJob->OutPath.GetName());
				strcat(FormatStr, "_Elev");
				CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
				CurEvent->AutoDigits = 0;
				if (RawExtension)
					strcat(FormatStr, RawExtension);
				AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_RAWELEV);
				} // if
			} // if

		// set objects to cast shadows only
		// if there is a shadow effect set to receive 3d object shadows, need to set all objects that are enabled
		// to render shadows only.
		// this applies only if 3d objects are supposed to export
		// set walls to render only if they are supposed to be exported and we need their shadows.
		if (CurJob->ExportTexture && CurJob->BurnShadows && CurJob->BurnShading)
			{
			if (CurJob->Export3DObjects)
				{
				ShadowsToRender = 0;
				ShadowObjects = 0;
				for (CurShadow = (ShadowEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); CurShadow; CurShadow = (ShadowEffect *)CurShadow->Next)
					{
					if (CurShadow->Enabled && CurShadow->ReceiveShadows3DObject)
						{
						ShadowsToRender = 1;
						break;
						} // if
					} // for
				if (ShadowsToRender)
					{
					for (CurObj = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurObj; CurObj = (Object3DEffect *)CurObj->Next)
						{
						if (CurObj->Enabled && CurObj->CastShadows && ! CurObj->ShadowsOnly)
							ShadowObjects ++;
						} // for
					} // if
				if (ShadowObjects)
					{
					if (ShadowObjectList = (Object3DEffect **)AppMem_Alloc(ShadowObjects * sizeof (Object3DEffect *), APPMEM_CLEAR))
						{
						for (Ct = 0, CurObj = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
							Ct < ShadowObjects && CurObj;
							CurObj = (Object3DEffect *)CurObj->Next)
							{
							if (CurObj->Enabled && CurObj->CastShadows && ! CurObj->ShadowsOnly)
								{
								ShadowObjectList[Ct] = CurObj;
								CurObj->ShadowsOnly = 1;
								Ct ++;
								} // if
							} // for
						} // if
					} // if
				else
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 0;
				} // if
			else
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 0;
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = CurJob->ExportWalls;
			} // if
		else
			{
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 0;
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 0;
			} // else

		// set maximum fractal depth based on export terrain resolution
		if (CurJob->FractalMethod == WCS_FRACTALMETHOD_CONSTANT)
			DefaultPar->FractalDepth = (char)FractalDepth;

		CurRJ->SetCamera(CurCam);
		CurRJ->SetRenderOpt(CurOpt);
		// no copying so be careful not to mess these scenarios up
		CurRJ->Scenarios = CurJob->Scenarios;
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);

		// restore objects whose enabled state was changed
		if (ShadowObjectList)
			{
			for (Ct = 0; Ct < ShadowObjects; Ct ++)
				{
				if (ShadowObjectList[Ct])
					{
					ShadowObjectList[Ct]->ShadowsOnly = 0;
					} // if
				} // for
			AppMem_Free(ShadowObjectList, ShadowObjects * sizeof (Object3DEffect *));
			} // if

		// clean up from rendering terrain and texture
		if (RTFWriteConfig)
			{
			delete RTFWriteConfig;
			RTFWriteConfig = NULL;
			} // if
		CurRJ->Scenarios = NULL;
		delete CurRJ;
		delete CurCam;
		delete CurOpt;
		} // if
	DefaultPar->FractalDepth = (char)FractalDepthStash;
	} // if

FrameTextClearA();
return (Success);

} // ExportControlGUI::ExportTerrain

/*===========================================================================*/

#define STARTTIME	CurRJ->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue
//#define ENDTIME		CurRJ->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue
//#define FRAMESTEP	(CurRJ->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue >= CurRJ->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue ? CurRJ->Options->FrameStep: -CurRJ->Options->FrameStep)
#define FRAMERATE	CurRJ->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue

int ExportControlGUI::RenderTerrain(SceneExporter *CurJob, RenderJob *CurRJ)
{
double CurTime;
long CurFrame;
//#ifdef WCS_RENDER_SCENARIOS
//long FirstFrame;
//#endif // WCS_RENDER_SCENARIOS
int Success = 0;
char TempPreview;

if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	TempPreview = Preview;
	delete Rend;
	Rend = NULL;
	SetPreview(TempPreview);
	} // if

// warn if invalid output directory
if (! CurRJ->Options->ValidateOutputPaths())
	return (0);

if (Rend = new Renderer())
	{
	Rend->RealTimeFoliageWrite = RTFWriteConfig;
	Rend->Exporter = CurJob;
//	#ifdef WCS_RENDER_SCENARIOS
//	FirstFrame = (long)(STARTTIME * FRAMERATE + .5);
//	CurTime = FirstFrame / FRAMERATE;
	// init enabled states at first frame to be rendered before init effects
//	EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
//	#endif // WCS_RENDER_SCENARIOS
	if (Rend->Init(CurRJ, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, TRUE))
		{
		// only complain once per render session
		Rend->SetFrdWarned(FrdWarned);
		CurFrame = (long)(STARTTIME * FRAMERATE + .5);

		#ifdef WCS_BUILD_DEMO
		if (CurFrame > 150)
			{
			UserMessageDemo("Highest frame number you can render is 150.");
			if (CurFrame > 150)
				CurFrame = 150;
			} // if
		#endif // WCS_BUILD_DEMO

		CurTime = CurFrame / FRAMERATE;

		#ifdef WCS_BUILD_DEMO
		if (CurTime > 5.0)
			{
			UserMessageDemo("Highest time you can render is 5 seconds.");
			CurTime = 5.0;
			} // if
		#endif // WCS_BUILD_DEMO

		// render a frame
		if ((Success = Rend->RenderFrame(CurTime, CurFrame)) && Run)
			{
			GlobalApp->WinSys->ClearRenderTitle();
			// update previous frame thumbnail
			UpdateLastFrame();
			// post time to status log
			Rend->LogElapsedTime(CurFrame, true, false, false); // frame time only
			// save the planet radius used for later use in exporting a sky cube to VNS
			CurJob->PlanetRadius = Rend->PlanetRad;
			// set these if it is the first time this is called which means it is called for saving terrain and 
			// texture map, not sky
			if (! CurJob->RenderSizesSet)
				{
				CurJob->FrameRendered = CurFrame;
				CurJob->TimeRendered = CurTime;
				CurJob->RowsRendered = Rend->Height * Rend->YTiles;
				CurJob->ColsRendered = Rend->Width * Rend->XTiles;
				CurJob->RenderSizesSet = 1;
				} // 
			} // if

		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		if (Rend)
			{
			Rend->Cleanup(true, false, TRUE, false);
			Rend->Opt->CleanupFromRender(TRUE);
			// steal this from renderer in case another job follows
			FrdWarned = Rend->GetFrdWarned();
			} // if
		} // if
	else
		{
		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(false, false, FALSE, false);	// need to delete anything created during failed init process.
		Rend->Opt->CleanupFromRender(TRUE);
		ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
		ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
		} // else
	Rend->RealTimeFoliageWrite = NULL;
	} // if 

return (Success);

} // ExportControlGUI::RenderTerrain

/*===========================================================================*/

// opens foliage index file and reads it, then opens and checks validity of foliage data file and returns pointer to file
// CountPos is used to rewrite the number of foliage entries in the index file
FILE *ExportControlGUI::PrepareFoliageFileForWalkthrough(SceneExporter *CurJob, RealtimeFoliageIndex *Index, 
	RealtimeFoliageCellData *RFCD, NameList **FileNamesCreated, long *CountPos, char *Mode)
{
long FileType;
const char *DataFileName, *OutputFilePath;
FILE *ffile;
char FileName[512], FullDataPath[512], TestFileVersion;

OutputFilePath = CurJob->OutPath.GetPath();

// files may not exist. If they don't then user must not have chosen foliage export features
if (ffile = OpenFoliageIndexFile(CurJob, FileNamesCreated, "rb"))
	{
	// read headers
	FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (DataFileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		// you've got the two file names that the Renderer wrote out.
		// combine them with the output file path to make a file that can be opened with PROJ_fopen()

		// read file descriptor, no need to keep it around unless you want to
		fgets(FileName, 256, ffile);
		// version
		fread((char *)&Index->FileVersion, sizeof (char), 1, ffile);
		// number of files
		fread((char *)&Index->NumCells, sizeof (long), 1, ffile);
		// reference XYZ
		fread((char *)&Index->RefXYZ[0], sizeof (double), 1, ffile);
		fread((char *)&Index->RefXYZ[1], sizeof (double), 1, ffile);
		fread((char *)&Index->RefXYZ[2], sizeof (double), 1, ffile);

		if (Index->NumCells > 0)
			{
			// only one cell data entry is provided
			if (Index->CellDat = RFCD)
				{
				// file name
				fgets(Index->CellDat->FileName, 64, ffile);
				// center XYZ
				fread((char *)&Index->CellDat->CellXYZ[0], sizeof (double), 1, ffile);
				fread((char *)&Index->CellDat->CellXYZ[1], sizeof (double), 1, ffile);
				fread((char *)&Index->CellDat->CellXYZ[2], sizeof (double), 1, ffile);
				// half cube cell dimension
				fread((char *)&Index->CellDat->CellRad, sizeof (double), 1, ffile);
				// number of trees in file
				if (CountPos)
					*CountPos = ftell(ffile);
				fread((char *)&Index->CellDat->DatCt, sizeof (long), 1, ffile);
				} // if
			} // if some cells to read
		fclose(ffile);
		ffile = NULL;

		if((Index->NumCells > 0) && (Index->CellDat->DatCt > 0))
			{
			strmfp(FullDataPath, OutputFilePath, DataFileName);
			if (ffile = PROJ_fopen(FullDataPath, Mode ? Mode: "rb"))
				{
				fgets(FileName, 64, ffile);
				// version
				fread((char *)&TestFileVersion, sizeof (char), 1, ffile);
				// Pointless version check -- we know we wrote it
				if (TestFileVersion == Index->FileVersion)
					{
					return (ffile);
					} // if
				} // if
			} // if
		} // if
	if (ffile)
		fclose(ffile);
	} // if
	
return (NULL);

} // ExportControlGUI::PrepareFoliageFileForWalkthrough

/*===========================================================================*/

FILE *ExportControlGUI::OpenFoliageIndexFile(SceneExporter *CurJob, NameList **FileNamesCreated, char *Mode)
{
long FileType;
const char *IndexFileName, *OutputFilePath;
FILE *ffile;
char FullIndexPath[512];

OutputFilePath = CurJob->OutPath.GetPath();

FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	// find and open the index file
	strmfp(FullIndexPath, OutputFilePath, IndexFileName);
	if (ffile = PROJ_fopen(FullIndexPath, Mode ? Mode: "rb"))
		{
		return (ffile);
		} // if
	} // if

return (NULL);

} // ExportControlGUI::OpenFoliageIndexFile

/*===========================================================================*/

// remove foliage files if no longer necessary
// this presumes that 3d objects have already been removed from them and any image foliage has been
// converted to 3d objects prior to this call.
void ExportControlGUI::PurgeFoliageFiles(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long FileType;
const char *OutputFilePath, *IndexFileName, *DataFileName;
char FullPath[512];

OutputFilePath = CurJob->OutPath.GetPath();

// see if an index file exists
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType);
// see if a data file exists
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
DataFileName = (*FileNamesCreated)->FindNameOfType(FileType);

if (IndexFileName || DataFileName)
	{
	if (! (CurJob->ExportFoliage || CurJob->ExportLabels) || (CurJob->FoliageAsObj && (CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS)) ||
		(CurJob->FoliageInstancesAsObj && CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER))
		{
		if (IndexFileName)
			{
			strmfp(FullPath, OutputFilePath, IndexFileName);
			PROJ_remove(FullPath);
			RemoveNameList(FileNamesCreated, (char *)IndexFileName, WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX);
			} // if
		if (DataFileName)
			{
			strmfp(FullPath, OutputFilePath, DataFileName);
			PROJ_remove(FullPath);
			RemoveNameList(FileNamesCreated, (char *)DataFileName, WCS_EXPORTCONTROL_FILETYPE_FOLFILE);
			} // if
		} // if
	} // if

} // ExportControlGUI::PurgeFoliageFiles

/*===========================================================================*/

// converts the foliage images into images with alpha channels standing in for background black transparency
int ExportControlGUI::ExportFoliageImages(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double CurTime;
long FirstFrame, ImageCt, RenderIt, ActualWidth;
RenderOpt *CurOpt = NULL;
RenderJob *CurRJ = NULL;
Camera *CurCam = NULL;
Light *CurLight = NULL;
BufferNode *CurBuf;
Raster *CurRast;
AnimColorTime *ReplaceRGB;
char *BuffersToSave[8], *FolExtension;
void *OutBuffers[4];
FILE *FolDataFile;
int Success = 1, Shade3D = 0, DoubleSample = 0, IsLabel = 0;
char TempPreview, FormatStr[256];
VertexDEM Vert;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
FolExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->FoliageImageFormat);
SetFrameTextA("Exporting Foliage Images");

if ((CurRJ = new RenderJob) && (CurOpt = new RenderOpt) && (CurCam = new Camera) && (CurLight = new Light))
	{
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->TempPath);
	CurOpt->RenderDiagnosticData = 0;
	CurRJ->Cam = CurCam;
	CurRJ->Options = CurOpt;

	FirstFrame = (long)(CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5);
	CurTime = FirstFrame / CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
	//#ifdef WCS_RENDER_SCENARIOS
	// init enabled states at export time
	//EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
	//#endif // WCS_RENDER_SCENARIOS

	FolDataFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated);

	// load to render any foliage being used as either ecotype or foliage effect
	// all effect types enabled
	ProcInit(ImageHost->GetImageCount(), "Processing Foliage");
	ImageCt = 0;
	CurRast = NULL;
	while (Success && (Success = ImageHost->InitFoliageRastersOneAtATime(CurTime, CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue,
		CurJob->ExportFoliage, CurJob->ExportFoliage, CurJob->ExportFoliage, CurJob->ExportFoliage, CurJob->ExportLabels, CurRast, ReplaceRGB, Shade3D)) && CurRast)
		{
		// find out if there is at least one instance of this foliage in bounds and of greater than minimum size
		RenderIt = 0;
		if (FolDataFile)
			{
			RenderIt = SearchForExportableFoliage(FolDataFile, CurJob, &Index, CurRast, IsLabel);
			} // if

		if (RenderIt)
			{
			FindOptimalFoliageRes(CurJob, CurRast->Rows, CurRast->Cols, CurOpt->OutputImageHeight, CurOpt->OutputImageWidth);

			// Lightwave HyperVoxel Sprite style
			ActualWidth = CurOpt->OutputImageWidth;

			// This was for support of Lightwave HyperVoxel sprites but it didn't work very well,
			// Also needed without the width adjustment for HD Instance plugin
			//if (CurJob->FoliageInstancesAsObj && CurJob->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_OTHER)
			//	{
			//	CurOpt->OutputImageHeight *= 2;
			//	if (CurOpt->OutputImageWidth < CurOpt->OutputImageHeight)
			//		CurOpt->OutputImageWidth = CurOpt->OutputImageHeight;
			//	DoubleSample = 1;
			//	} // if

			// Code modified for GE4
			if ((! strcmp(CurJob->ExportTarget, "GoogleEarth")) && IsLabel)
				{
				CurOpt->OutputImageHeight *= 2;
				CurOpt->OutputImageHeight += 2;
				CurOpt->OutputImageWidth += 2;
				DoubleSample = 1;
				} // if

			if (Rend)
				{
				ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
				ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
				TempPreview = Preview;
				delete Rend;
				Rend = NULL;
				SetPreview(TempPreview);
				} // if

			if (Rend = new Renderer())
				{
				// turn off anything that would take a while to initialize and does not affect foliage
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ENVIRONMENT] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
				CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 0;

				if (Rend->InitForProcessing(CurRJ, GlobalApp, EffectsHost, ImageHost, 
					DBHost, ProjectHost, GlobalApp->StatusLog, TRUE))	// true=elevations only, just need basic setup
					{
					Rend->Master = this;

					// turn things back on again that might make a difference to which foliage is processed
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
					CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 1;

					if (CurBuf = Rend->Buffers = new BufferNode("RED", WCS_RASTER_BANDSET_BYTE))
						{
						if (CurBuf = CurBuf->AddBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE))
							{
							if (CurBuf = CurBuf->AddBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE))
								{
								if (! CurBuf->AddBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE))
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;
						} // if
					else
						Success = 0;

					if (Success && Rend->AllocateBuffers())
						{
						// initialize everything
						CurCam->InitToRender(CurOpt, Rend->Buffers);
						CurOpt->InitToRender(Rend->Buffers, Rend->Width, Rend->Height, 1);
						CurLight->InitToRender(CurOpt, Rend->Buffers);

						// set positions
						// camera defaults to 0 HPB so will be pointed horizontally due north
						CurCam->CameraType = WCS_EFFECTS_CAMERATYPE_UNTARGETED;
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(45.0);
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(0.0);
						// tree will be scaled to 20m tall.
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(10.0);
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(53.13);	// atan(10/20)
						// light is a little west and above so gives more light on upper left
						CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetValue(20.0);
						CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetValue(20.0);
						
						Rend->SetDrawOffsets();
						Rend->LocalLog->GoModal();
						Rend->AppBase->SetProcPri((signed char)ProjectHost->InquireRenderPri());
						GUIGoModal();
						if (GetPreview())
							{
							Rend->OpenPreview();
							if (Rend->Pane)
								Rend->Pane->Clear();
							else
								SetPreview(0);
							} // if
						Rend->InitFrame(CurTime, FirstFrame, 1);
						CurCam->InitFrameToRender(EffectsHost, &Rend->RendData);
						CurOpt->InitFrameToRender(EffectsHost, &Rend->RendData);
						CurLight->InitFrameToRender(EffectsHost, &Rend->RendData);

						// needs to follow InitFrame
						Vert.Lat = 45.00018;	// aproximately 20 meters north of camera on normal earth-sized globe
						Vert.Lon = 0.0;
						Vert.Elev = 0.0;
						Vert.ScrnXYZ[0] = CurOpt->OutputImageWidth * .5;
						Vert.ScrnXYZ[1] = DoubleSample ? CurOpt->OutputImageHeight * .5: CurOpt->OutputImageHeight;
						Vert.ScrnXYZ[2] = 20.0;
						#ifdef WCS_BUILD_VNS
						Rend->DefCoords->DegToCart(&Vert);
						#else // WCS_BUILD_VNS
						Vert.DegToCart(Rend->PlanetRad);
						#endif // WCS_BUILD_VNS

						Rend->ProcessOneFoliageExport(&Vert, CurRast, ReplaceRGB, Shade3D, 20.0, DoubleSample ? -20.0 / (CurOpt->OutputImageHeight * .5): -20.0 / CurOpt->OutputImageHeight, 
							CurJob->FolTransparencyStyle == WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_CLIPMAP, CurLight, DoubleSample, ActualWidth);

						strcpy(FormatStr, CurRast->GetUserName());
						ImageSaverLibrary::StripImageExtension(FormatStr);
						strcat(FormatStr, "_Fol");
						ReplaceChar(FormatStr, '.', '_');

						BuffersToSave[0] = "RED";
						BuffersToSave[1] = "GREEN";
						BuffersToSave[2] = "BLUE";
						BuffersToSave[3] = "ANTIALIAS";
						BuffersToSave[4] = NULL;
						OutBuffers[0] = Rend->Bitmap[0];
						OutBuffers[1] = Rend->Bitmap[1];
						OutBuffers[2] = Rend->Bitmap[2];
						OutBuffers[3] = Rend->AABuf;

						if (Success = SaveBuffer(CurJob, CurOpt, NULL, CurJob->FoliageImageFormat, BuffersToSave, OutBuffers,
							(char *)CurJob->OutPath.GetPath(), FormatStr, FirstFrame, CurOpt->OutputImageHeight, 
							CurOpt->OutputImageWidth, false, 0.0f, 0.0f))
							{
							if (FolExtension)
								strcat(FormatStr, FolExtension);
							AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX);
							} // if
						// label images are created specifically for export and can be deleted after rendering
						if (IsLabel)
							{
							strmfp(FormatStr, CurRast->PAF.GetPath(), CurRast->GetUserName());
							PROJ_remove(FormatStr);
							} // if

						} // if buffers allocated
					else
						Success = 0;
					Rend->Cleanup(true, false, TRUE, false);
					CurOpt->CleanupFromRender(TRUE);
					} // if Renderer initialized
				else
					{
					Success = 0;
					ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
					ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
					} // else
				} // if Renderer created
			else
				Success = 0;
			} // if need to render it
		ProcUpdate(++ImageCt);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // while
	if (FolDataFile)
		fclose(FolDataFile);
	Index.CellDat = NULL;
	ProcClear();
	//#ifdef WCS_RENDER_SCENARIOS
	//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
	//#endif // WCS_RENDER_SCENARIOS
	} // if misc. components created
else
	Success = 0;

if (CurOpt)
	delete CurOpt;
if (CurCam)
	delete CurCam;
if (CurLight)
	delete CurLight;
if (CurRJ)
	delete CurRJ;

FrameTextClearA();
return (Success);

} // ExportControlGUI::ExportFoliageImages

/*===========================================================================*/

// screens foliage rasters by whether or not they show up in the foliage list with appropriate height and position
int ExportControlGUI::SearchForExportableFoliage(FILE *FolDataFile, SceneExporter *CurJob, RealtimeFoliageIndex *Index,
	Raster *SearchRast, int &IsLabel)
{
long FolObjCt;
char FileName[128], TestFileVersion;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;

IsLabel = 0;

// rewind file to position just past header
fseek(FolDataFile, 0, SEEK_SET);

fgets(FileName, 64, FolDataFile);
// version
fread((char *)&TestFileVersion, sizeof (char), 1, FolDataFile);

// step through records looking for item with height greater or equal to minimum and within bounds

for (FolObjCt = 0; FolObjCt < Index->CellDat->DatCt; FolObjCt ++)
	{
	// read and interpret foliage record
	if (FolData.ReadFoliageRecord(FolDataFile, TestFileVersion))
		{
		// only want image foliage
		if (FolData.ElementID > 0 && FolData.InterpretFoliageRecord(NULL, ImageHost, &PointData)) // don't need full decoding of 3dobjects, just height, etc
			{
			// is it the Raster we are intested in?
			if (PointData.CurRast == SearchRast)
				{
				// find out the size and image used
				if (PointData.Height >= CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT].CurValue)
					{
					if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL)
						IsLabel = 1;
					if (CurJob->RBounds.IsDefGeoPointBounded(FolData.XYZ[1] + Index->RefXYZ[1], FolData.XYZ[0] + Index->RefXYZ[0]))
						return (1);
					} // if
				} // if
			} // if
		} // if
	} // for

return (0);

} // ExportControlGUI::SearchForExportableFoliage

/*===========================================================================*/

// finds correct resolution for foliage images
void ExportControlGUI::FindOptimalFoliageRes(SceneExporter *CurJob, long SourceRows, long SourceCols, long &DestRows, long &DestCols)
{
long Res, ResDiff, BestDiff;

if (SourceRows > CurJob->FoliageRes)
	DestRows = CurJob->FoliageRes;
else if (CurJob->FolResOption != WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
	{
	DestRows = SourceRows;
	} // else if
else
	{
	// make sure it gets set to something
	DestRows = CurJob->FoliageRes;
	// initial difference
	BestDiff = abs(SourceRows - CurJob->FoliageRes);
	for (Res = 2; Res < CurJob->FoliageRes; Res *= 2)
		{
		ResDiff = SourceRows - Res;
		// weight it a bit towards up-sampling
		ResDiff = ResDiff < 0 ? -ResDiff: (ResDiff * 3) / 2;
		if (ResDiff < BestDiff)
			{
			DestRows = Res;
			BestDiff = ResDiff;
			} // if
		} // for
	} // else

if (SourceCols > CurJob->FoliageRes)
	DestCols = CurJob->FoliageRes;
else if (CurJob->FolResOption != WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2)
	{
	DestCols = SourceCols;
	} // else if
else
	{
	// make sure it gets set to something
	DestCols = CurJob->FoliageRes;
	// initial difference
	BestDiff = abs(SourceCols - CurJob->FoliageRes);
	for (Res = 2; Res < CurJob->FoliageRes; Res *= 2)
		{
		ResDiff = SourceCols - Res;
		// weight it a bit towards up-sampling
		ResDiff = ResDiff < 0 ? -ResDiff: (ResDiff * 3) / 2;
		if (ResDiff < BestDiff)
			{
			DestCols = Res;
			BestDiff = ResDiff;
			} // if
		} // for
	} // else

} // ExportControlGUI::FindOptimalFoliageRes

/*===========================================================================*/

int ExportControlGUI::ExportFoliageInstancesAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double CurWidth, CurHeight;
long TotalRasters, RastCt, VertCt, FolObjCt, CurFlipX;
long *NumInstancesList = NULL;
float *MaxHeightList = NULL;
Object3DInstance *FolObjectList = NULL, **InstancePtr, *CurInstance;
Object3DEffect *Object3D;
Raster *CurRast;
MaterialEffect *CurMat;
const char *OutputFilePath;
char *FolExtension;
FILE *FolDatFile;
int Success = 1;
VertexDEM BasePos;
RealtimeFoliageIndex Index;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;
RealtimeFoliageCellData RFCD;
//char FileName[512], FullOriginalPath[512], FullNewPath[512], TempName[256];

InstancePtr = &FolObjectList;

FolExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->FoliageImageFormat);
OutputFilePath = CurJob->OutPath.GetPath();

if ((TotalRasters = ImageHost->GetImageCount()) > 0)
	{
	TotalRasters ++;	// need to allow for fact that image ID's begin at 1
	if ((MaxHeightList = (float *)AppMem_Alloc(TotalRasters * sizeof (float), APPMEM_CLEAR))
		&& (NumInstancesList = (long *)AppMem_Alloc(TotalRasters * sizeof (long), APPMEM_CLEAR)))
		{
		if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated))
			{
			// for each raster, count the number of instances, note the maximum height
			for (FolObjCt = 0; FolObjCt < Index.CellDat->DatCt; FolObjCt ++)
				{
				// read and interpret foliage record
				if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
					{
					// only want image foliage
					if (FolData.ElementID > 0 && FolData.ElementID < TotalRasters && FolData.InterpretFoliageRecord(NULL, ImageHost, &PointData)) // don't need full decoding of 3dobjects, just height, etc
						{
						// find out the size and image used
						if (PointData.Height > MaxHeightList[FolData.ElementID])
							MaxHeightList[FolData.ElementID] = (float)PointData.Height;
						NumInstancesList[FolData.ElementID] ++;
						} // if
					} // if
				} // for
			fclose(FolDatFile);
			} // if FolDatFile
		Index.CellDat = NULL;

		// walk the image list and build an object for each raster with a non-zero count
		// create a weight map (a UV map which we will use only the u coordinate)
		for (RastCt = 1, CurRast = ImageHost->GetFirstRast(); CurRast && Success; RastCt ++, CurRast = ImageHost->GetNextRast(CurRast))
			{
			if (NumInstancesList[RastCt] > 0 && MaxHeightList[RastCt] > 0.0)
				{
				if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated))
					{
					// create an object
					if (Object3D = new Object3DEffect)
						{
						CurJob->Unique3DObjectInstances ++;
						CurJob->AddObjectToEphemeralList(Object3D);
						sprintf(Object3D->Name, "FO%06di", RastCt);
						strcpy(Object3D->ShortName, Object3D->Name);
						
						// create a new material
						if (CurMat = new MaterialEffect(NULL))
							{
							CurMat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
							CurMat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
							CurMat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
							// delete it later
							CurJob->AddObjectToEphemeralList(CurMat);
							// name it uniquely
							sprintf(CurMat->Name, "FM%06di", RastCt);
							strcpy(CurMat->ShortName, CurMat->Name);

							// where is the image we copied with the alpha channel?
							/*
							strcpy(FileName, CurRast->GetUserName());
							ImageSaverLibrary::StripImageExtension(FileName);
							strcat(FileName, "_Fol");
							ReplaceChar(FileName, '.', '_');
							if (FolExtension)
								AddExtension(FileName, FolExtension);
							// change file name of raster to material name which is unique, add extension
							/// this needs to change the image name to something short
							// needs file extension to match the one saved
							strmfp(FullOriginalPath, (char *)OutputFilePath, FileName);
							strcpy(TempName, CurMat->Name);
							if (FolExtension)
								AddExtension(TempName, FolExtension);
							strmfp(FullNewPath, (char *)OutputFilePath, TempName);
							PROJ_remove(FullNewPath);
							PROJ_rename(FullOriginalPath, FullNewPath);
							RemoveNameList(FileNamesCreated, FileName, WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX);
							AddNewNameList(FileNamesCreated, TempName, WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX);
							*/
							} // if new material
						else
							{
							Success = 0;
							break;
							} // else
						// set number of polygons and vertices
						Object3D->NumVertices = NumInstancesList[RastCt];
						Object3D->NumPolys = 0;
						Object3D->NumMaterials = 1;

						// allocate verts
						// create UV map
						if ((Object3D->Vertices = new Vertex3D[Object3D->NumVertices])&&
							(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials]) &&
							Object3D->AllocUVWMaps(1) && Object3D->UVWTable[0].AllocMap(Object3D->NumVertices))
							{
							Object3D->VertexUVWAvailable = 1;
							strcpy(Object3D->NameTable[0].Name, CurMat->Name);
							Object3D->NameTable[0].Mat = CurMat;
							Object3D->UVWTable[0].SetName(Object3D->Name);
							Object3D->UVWTable[0].UVMapType = WCS_OBJPERVERTMAP_MAPTYPE_WEIGHT;

							// walk the instance list and set the vertices in the object
							// set the height fraction in the Weight map
							for (VertCt = FolObjCt = 0; Success && FolObjCt < Index.CellDat->DatCt; FolObjCt ++)
								{
								// read and interpret foliage record
								if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
									{
									// only want image foliage
									if (FolData.ElementID > 0 && FolData.ElementID < TotalRasters && FolData.InterpretFoliageRecord(NULL, ImageHost, &PointData)) // don't need full decoding of 3dobjects, just height, etc
										{
										// is it an instance of this raster?
										if (PointData.CurRast == CurRast)
											{
											// find out the size and image used
											CurHeight = PointData.Height;
											CurWidth = PointData.Width;
											CurFlipX = PointData.FlipX;
											Object3D->Vertices[VertCt].xyz[0] = -(FolData.XYZ[0] + Index.RefXYZ[0] - CurJob->ExportRefData.WCSRefLon) * CurJob->ExportRefData.WCSLonScale;
											Object3D->Vertices[VertCt].xyz[2] = (FolData.XYZ[1] + Index.RefXYZ[1] - CurJob->ExportRefData.WCSRefLat) * CurJob->ExportRefData.WCSLatScale;
											Object3D->Vertices[VertCt].xyz[1] = (FolData.XYZ[2] + Index.RefXYZ[2] - CurJob->ExportRefData.RefElev) * CurJob->ExportRefData.ElevScale;

											Object3D->UVWTable[0].CoordsValid[VertCt] = 1;
											// HD instances don't invert when negative
											//Object3D->UVWTable[0].CoordsArray[0][VertCt] = CurFlipX ? -(float)(CurHeight / MaxHeightList[RastCt]): (float)(CurHeight / MaxHeightList[RastCt]);
											Object3D->UVWTable[0].CoordsArray[0][VertCt] = (float)(CurHeight / MaxHeightList[RastCt]);

											VertCt ++;
											} // if
										} // if
									} // if
								else
									Success = 0;
								} // for
							if (*InstancePtr = new Object3DInstance)
								{
								CurInstance = *InstancePtr;
								CurInstance->MyObj = Object3D;
								// WCS geographic coords
								CurInstance->WCSGeographic[0] = CurJob->ExportRefData.WCSRefLon;
								CurInstance->WCSGeographic[1] = CurJob->ExportRefData.WCSRefLat;
								CurInstance->WCSGeographic[2] = CurJob->ExportRefData.RefElev;
								CurInstance->Scale[0] = MaxHeightList[RastCt] * CurRast->Cols /  CurRast->Rows;
								CurInstance->Scale[1] = MaxHeightList[RastCt];
								CurInstance->Scale[2] = MaxHeightList[RastCt] * CurRast->Cols /  CurRast->Rows;
								//CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE;
								CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE;
								InstancePtr = &CurInstance->Next;
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							Object3D->GetObjectBounds();
							} // if alloc vertices
						else
							{
							Success = 0;
							break;
							} // else
						} // if object created
					else
						{
						Success = 0;
						break;
						} // else
					fclose(FolDatFile);
					} // if FolDatFile
				Index.CellDat = NULL;
				} // if
			} // for

		if (CurJob->ObjectInstanceList && FolObjectList)
			{
			CurInstance = CurJob->ObjectInstanceList;
			while (CurInstance->Next)
				{
				CurInstance = CurInstance->Next;
				} // if
			CurInstance->Next = FolObjectList;
			} // if
		else if (FolObjectList)
			{
			CurJob->ObjectInstanceList = FolObjectList;
			} // else if
		FolObjectList = NULL;
		} // if
	else
		Success = 0;
	if (MaxHeightList)
		AppMem_Free(MaxHeightList, TotalRasters * sizeof (float));
	if (NumInstancesList)
		AppMem_Free(NumInstancesList, TotalRasters * sizeof (long));
	} // if total rasters

return (Success);

} // ExportControlGUI::ExportFoliageInstancesAs3DObjects

/*===========================================================================*/

int ExportControlGUI::ExportFoliageAsAligned3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long TempNumBoards;
int Success;

TempNumBoards = CurJob->NumFoliageBoards;
CurJob->NumFoliageBoards = 1;

Success = ExportFoliageAs3DObjects(CurJob, FileNamesCreated, TRUE, FALSE);

CurJob->NumFoliageBoards = TempNumBoards;

return (Success);

} // ExportControlGUI::ExportFoliageAsAligned3DObjects

/*===========================================================================*/

int ExportControlGUI::ExportFoliageAsNonAligned3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long TempNumBoards;
int Success;

TempNumBoards = CurJob->NumFoliageBoards;
CurJob->NumFoliageBoards = 1;

Success = ExportFoliageAs3DObjects(CurJob, FileNamesCreated, FALSE, TRUE);

CurJob->NumFoliageBoards = TempNumBoards;

return (Success);

} // ExportControlGUI::ExportFoliageAsNonAligned3DObjects

/*===========================================================================*/

// convert image foliage entries to 3d object list entries
int ExportControlGUI::ExportFoliageAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated, int AlignToCamera, 
	int SingleObjectAtOrigin)
{
double CurWidth, CurHeight, BoardX, BoardZ, BoardAngle;
long TotalRasters, FolObjCt, RastCt, CurFlipX, PolyCt, VertRefBlockIdx, VertNum, PolyNum, 
	Board, FirstVert, FolMatNum = 0;
Object3DInstance *FolObjectList = NULL, **InstancePtr, *CurInstance;
Object3DEffect *Object3D, *MadeObj, **FoliageObjectList = NULL;
RootTexture *CurTexRoot;
UVImageTexture *CurUVTex;
Raster *CurRast, *UVRaster;
MaterialEffect *CurMat;
const char *OutputFilePath;
float *MaxHeightList = NULL;
char *FolExtension;
FILE *FolDatFile;
int Success = 1;
char FileName[512], FullOriginalPath[512], FullNewPath[512], TempName[256];
VertexDEM BasePos;
RealtimeFoliageIndex Index;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;
RealtimeFoliageCellData RFCD;

InstancePtr = &FolObjectList;

FolExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->FoliageImageFormat);
OutputFilePath = CurJob->OutPath.GetPath();

if (SingleObjectAtOrigin)
	{
	if ((TotalRasters = ImageHost->GetImageCount()) > 0)
		{
		TotalRasters ++;	// need to allow for fact that image ID's begin at 1
		if ((MaxHeightList = (float *)AppMem_Alloc(TotalRasters * sizeof (float), APPMEM_CLEAR)))
			{
			if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated))
				{
				// for each raster, count the number of instances, note the maximum height
				for (FolObjCt = 0; FolObjCt < Index.CellDat->DatCt; FolObjCt ++)
					{
					// read and interpret foliage record
					if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
						{
						// only want image foliage
						if (FolData.ElementID > 0 && FolData.ElementID < TotalRasters && FolData.InterpretFoliageRecord(NULL, ImageHost, &PointData)) // don't need full decoding of 3dobjects, just height, etc
							{
							// find out the size and image used
							if (PointData.Height > MaxHeightList[FolData.ElementID])
								MaxHeightList[FolData.ElementID] = (float)PointData.Height;
							} // if
						} // if
					} // for
				fclose(FolDatFile);
				} // if FolDatFile
			Index.CellDat = NULL;
			} // if
		else
			Success = 0;
		} // if
	} // if

if (Success && (TotalRasters = ImageHost->GetImageCount()) > 0)
	{
	TotalRasters ++;	// need to allow for fact that image ID's begin at 1
	if (FoliageObjectList = (Object3DEffect **)AppMem_Alloc(TotalRasters * sizeof (Object3DEffect *), APPMEM_CLEAR))
		{

		// walk the foliage list
		// build a 3d object for each unique element ID in the list, create one material per object
		// two polygons in variable number of planes with UV coords

		// walk the image list and build an object for each raster with a non-zero count
		// create a weight map (a UV map which we will use only the u coordinate)
		for (RastCt = 1, CurRast = ImageHost->GetFirstRast(); CurRast && Success; RastCt ++, CurRast = ImageHost->GetNextRast(CurRast))
			{
			if (! MaxHeightList || MaxHeightList[RastCt] > 0.0)
				{
				if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated))
					{
					for (FolObjCt = 0; Success && FolObjCt < Index.CellDat->DatCt; FolObjCt ++)
						{
						// read and interpret foliage record
						if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
							{
							// only want image foliage
							if (FolData.ElementID > 0 && FolData.ElementID < TotalRasters && FolData.InterpretFoliageRecord(NULL, ImageHost, &PointData) && PointData.Height > 0.0) // don't need full decoding of 3dobjects, just height, etc
								{
								if (CurRast == PointData.CurRast)
									{
									// find out the size and image used
									CurHeight = SingleObjectAtOrigin ? MaxHeightList[FolData.ElementID]: PointData.Height;
									CurWidth = SingleObjectAtOrigin ? CurHeight * PointData.Width / PointData.Height: PointData.Width;
									BasePos.Lon = SingleObjectAtOrigin ? CurJob->ExportRefData.WCSRefLon: FolData.XYZ[0] + Index.RefXYZ[0];
									BasePos.Lat = SingleObjectAtOrigin ? CurJob->ExportRefData.WCSRefLat: FolData.XYZ[1] + Index.RefXYZ[1];
									BasePos.Elev = SingleObjectAtOrigin ? CurJob->ExportRefData.RefElev: FolData.XYZ[2] + Index.RefXYZ[2];
									CurFlipX = SingleObjectAtOrigin ? 0: PointData.FlipX;

									MadeObj = NULL;
									// has object already been made for this foliage
									if ((Object3D = FoliageObjectList[FolData.ElementID]) || 
										(MadeObj = (Object3D = new Object3DEffect)))
										{
										if (MadeObj)
											{
											CurJob->Unique3DObjectInstances ++;
											FoliageObjectList[FolData.ElementID] = Object3D;
											CurJob->AddObjectToEphemeralList(Object3D);

											sprintf(Object3D->Name, "FO%06d", FolData.ElementID);
											strcpy(Object3D->ShortName, Object3D->Name);
											
											// create a new material
											if (CurMat = new MaterialEffect(NULL))
												{
												CurMat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
												CurMat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
												CurMat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
												// delete it later
												CurJob->AddObjectToEphemeralList(CurMat);
												// name it uniquely
												sprintf(CurMat->Name, "FM%06d", FolData.ElementID);
												strcpy(CurMat->ShortName, CurMat->Name);

												// set material properties
												CurMat->DoubleSided = 1;
												CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
												// in LightWave at least it seems good to have flipboards non-luminous,
												// maybe wherever flipboards are used. Changed 11/30/04 by GRH
												if (SingleObjectAtOrigin || CurJob->NumFoliageBoards <= 1)
													{
													CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY].SetValue(1.0);
													CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(0.0);
													} // if
												else
													CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
												// create texture
												UVRaster = NULL;
												if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
													{
													if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
														{
														CurUVTex->SetVertexMap(Object3D->Name);
														// where is the image we copied with the alpha channel?
														strcpy(FileName, CurRast->GetUserName());
														ImageSaverLibrary::StripImageExtension(FileName);
														strcat(FileName, "_Fol");
														ReplaceChar(FileName, '.', '_');
														if (FolExtension)
															AddExtension(FileName, FolExtension);
														// change file name of raster to material name which is unique, add extension
														/// this needs to change the image name to something short
														// needs file extension to match the one saved
														strmfp(FullOriginalPath, (char *)OutputFilePath, FileName);
														strcpy(TempName, CurMat->Name);
														if (FolExtension)
															AddExtension(TempName, FolExtension);
														strmfp(FullNewPath, (char *)OutputFilePath, TempName);
														PROJ_remove(FullNewPath);
														PROJ_rename(FullOriginalPath, FullNewPath);
														RemoveNameList(FileNamesCreated, FileName, WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX);
														AddNewNameList(FileNamesCreated, TempName, WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX);
														if (UVRaster = ImageHost->AddRaster((char *)OutputFilePath, TempName, 0, 1, 1))
															{
															UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
															CurUVTex->SetRaster(UVRaster);
															CurJob->AddObjectToEphemeralList(UVRaster);
															} // if
														else
															{
															Success = 0;
															break;
															} // else
														} // if
													} // if
												// make a bump map texture too
												if (UVRaster)
													{
													// set bump intensity to 200%
													CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].SetValue(2.0);
													// create bump map texture same as color
													if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
														{
														if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
															{
															CurUVTex->SetVertexMap(Object3D->Name);
															CurUVTex->SetRaster(UVRaster);
															} // if
														} // if
													} // if
												} // if new material
											else
												{
												Success = 0;
												break;
												} // else

											// set number of polygons and vertices
											Object3D->NumVertices = CurJob->NumFoliageBoards * 4;
											Object3D->NumPolys = CurJob->NumFoliageBoards * 2;
											Object3D->NumMaterials = 1;

											// allocate polys and verts
											if (CurMat && (Object3D->Vertices = new Vertex3D[Object3D->NumVertices]) && 
												(Object3D->Polygons = new Polygon3D[Object3D->NumPolys]) &&
												(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials]) &&
												Object3D->AllocUVWMaps(1) && Object3D->UVWTable[0].AllocMap(Object3D->NumVertices)
												&& Object3D->AllocVertRef(Object3D->NumPolys * 3))
												{
												VertRefBlockIdx = 0;
												Object3D->VertexUVWAvailable = 1;
												strcpy(Object3D->NameTable[0].Name, CurMat->Name);
												Object3D->NameTable[0].Mat = CurMat;
												Object3D->UVWTable[0].SetName(Object3D->Name);

												for (Board = VertNum = PolyNum = 0; Board < CurJob->NumFoliageBoards; Board ++)
													{
													double sinBA, cosBA;

													FirstVert = VertNum;
													BoardAngle = PiOver180 * Board * 180.0 / CurJob->NumFoliageBoards;
													//BoardZ = .5 * sin(BoardAngle);
													//BoardX = .5 * cos(BoardAngle);
													sincos(BoardAngle, &sinBA, &cosBA);
													BoardZ = .5 * sinBA;
													BoardX = .5 * cosBA;
													// vertex 0
													Object3D->Vertices[VertNum].xyz[0] = -BoardX;
													Object3D->Vertices[VertNum].xyz[1] = 1.0;
													Object3D->Vertices[VertNum].xyz[2] = -BoardZ;
													Object3D->UVWTable[0].CoordsValid[VertNum] = 1;
													Object3D->UVWTable[0].CoordsArray[0][VertNum] = CurFlipX ? 1.0f: 0.0f;
													Object3D->UVWTable[0].CoordsArray[1][VertNum ++] = 1.0f;	// bottom of image
													// vertex 1
													Object3D->Vertices[VertNum].xyz[0] = -BoardX;
													Object3D->Vertices[VertNum].xyz[1] = 0.0;
													Object3D->Vertices[VertNum].xyz[2] = -BoardZ;
													Object3D->UVWTable[0].CoordsValid[VertNum] = 1;
													Object3D->UVWTable[0].CoordsArray[0][VertNum] = CurFlipX ? 1.0f: 0.0f;
													Object3D->UVWTable[0].CoordsArray[1][VertNum ++] = 0.0f;	// top of image
													// vertex 2
													Object3D->Vertices[VertNum].xyz[0] = BoardX;
													Object3D->Vertices[VertNum].xyz[1] = 0.0;
													Object3D->Vertices[VertNum].xyz[2] = BoardZ;
													Object3D->UVWTable[0].CoordsValid[VertNum] = 1;
													Object3D->UVWTable[0].CoordsArray[0][VertNum] = CurFlipX ? 0.0f: 1.0f;
													Object3D->UVWTable[0].CoordsArray[1][VertNum ++] = 0.0f;	// top of image
													// vertex 3
													Object3D->Vertices[VertNum].xyz[0] = BoardX;
													Object3D->Vertices[VertNum].xyz[1] = 1.0;
													Object3D->Vertices[VertNum].xyz[2] = BoardZ;
													Object3D->UVWTable[0].CoordsValid[VertNum] = 1;
													Object3D->UVWTable[0].CoordsArray[0][VertNum] = CurFlipX ? 0.0f: 1.0f;
													Object3D->UVWTable[0].CoordsArray[1][VertNum ++] = 1.0f;	// bottom of image

													// allocate polygon reference numbers
													for (PolyCt = PolyNum; PolyCt < PolyNum + 2; PolyCt ++)
														{
														Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
														VertRefBlockIdx += 3; // We just used 3 longs
														Object3D->Polygons[PolyCt].NumVerts = 3;
														} // for
													// polygon 0
													Object3D->Polygons[PolyNum].VertRef[0] = FirstVert;
													Object3D->Polygons[PolyNum].VertRef[1] = FirstVert + 1;
													Object3D->Polygons[PolyNum].VertRef[2] = FirstVert + 2;
													Object3D->Polygons[PolyNum ++].Material = 0;
													// polygon 1
													Object3D->Polygons[PolyNum].VertRef[0] = FirstVert;
													Object3D->Polygons[PolyNum].VertRef[1] = FirstVert + 2;
													Object3D->Polygons[PolyNum].VertRef[2] = FirstVert + 3;
													Object3D->Polygons[PolyNum ++].Material = 0;
													} // for

												Object3D->GetObjectBounds();
												} // if allocated
											else
												{
												Success = 0;
												break;
												} // else
											} // if made new object
											
										if (*InstancePtr = new Object3DInstance)
											{
											CurInstance = *InstancePtr;
											CurInstance->MyObj = Object3D;
											// WCS geographic coords
											CurInstance->WCSGeographic[0] = BasePos.Lon;
											CurInstance->WCSGeographic[1] = BasePos.Lat;
											CurInstance->WCSGeographic[2] = BasePos.Elev;
											CurInstance->Scale[0] = CurWidth;
											CurInstance->Scale[1] = CurHeight;
											CurInstance->Scale[2] = CurWidth;
											if (AlignToCamera)
												CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_ALIGNTOCAMERA;
											if (CurJob->FolTransparencyStyle == WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_CLIPMAP)
												CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_CLIPMAP;
											if (SingleObjectAtOrigin)
												CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_UNSEENBYCAMERA;
											CurInstance->Flags |= WCS_OBJECT3DINSTANCE_FLAGBIT_FOLIAGE;
											InstancePtr = &CurInstance->Next;
											} // if
										else
											{
											Success = 0;
											break;
											} // else
										} // if object3d
									else
										{
										Success = 0;
										break;
										} // else
									if (SingleObjectAtOrigin)
										break;
									} // if correct raster
								} // if interpret record
							} // if read record
						else
							{
							Success = 0;
							break;
							} // else
						} // for

					fclose(FolDatFile);
					} // if FolDatFile
				} // if
			} // for each raster
		if (CurJob->ObjectInstanceList && FolObjectList)
			{
			CurInstance = CurJob->ObjectInstanceList;
			while (CurInstance->Next)
				{
				CurInstance = CurInstance->Next;
				} // if
			CurInstance->Next = FolObjectList;
			} // if
		else if (FolObjectList)
			{
			CurJob->ObjectInstanceList = FolObjectList;
			} // else if
		FolObjectList = NULL;
		} // if
	else
		Success = 0;
	if (FoliageObjectList)
		AppMem_Free(FoliageObjectList, TotalRasters * sizeof (Object3DEffect *));
	} // if total rasters

Index.CellDat = NULL;

if (MaxHeightList)
	AppMem_Free(MaxHeightList, TotalRasters * sizeof (float));

return (Success);

} // ExportControlGUI::ExportFoliageAs3DObjects

/*===========================================================================*/

// prepares to render sky cube
int ExportControlGUI::ExportSky(SceneExporter *CurJob, NameList **FileNamesCreated)
{
Camera *CurCam;
RenderOpt *CurOpt;
RenderJob *CurRJ;
ImageOutputEvent *CurEvent;
char *ImageExtension;
int Success = 1, Ct;
char FormatStr[256];

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);
SetFrameTextA("Exporting Sky");

// create a camera, render opt and render job
if ((CurCam = new Camera) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob))
	{
	// make the camera non-targeted panoramic
	CurCam->CameraType = WCS_EFFECTS_CAMERATYPE_UNTARGETED;
	CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CurJob->ExportRefData.WCSRefLat);
	CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CurJob->ExportRefData.WCSRefLon);
	CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurJob->ExportRefData.RefElev);
	CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(90.0);
	CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(0.0);

	// disable terrain and other stuff
	// set image size to be square with 1.0 aspect
	CurOpt->OutputImageWidth = CurOpt->OutputImageHeight = CurJob->SkyRes;
	CurOpt->ReflectionsEnabled = 0;
	CurOpt->CloudShadowsEnabled = CurOpt->ObjectShadowsEnabled = CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = (CurJob->BurnShading && CurJob->BurnShadows);
	CurOpt->RenderDiagnosticData = CurOpt->MultiPassAAEnabled = CurOpt->DepthOfFieldEnabled = 0;
	CurOpt->TerrainEnabled = 0;
	CurOpt->VectorsEnabled = 0;
	CurOpt->FoliageEnabled = CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] =  
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 0; 
	CurOpt->FragmentRenderingEnabled = 1;
	CurOpt->FragmentDepth = 100;
	CurOpt->VolumetricsEnabled = CurJob->ExportVolumetrics;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = CurJob->ExportClouds;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = CurJob->ExportCelest;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = CurJob->ExportStars;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = CurJob->ExportSky;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = CurJob->ExportAtmosphere;
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->TempPath);

	CurJob->FragmentCollapseType = WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST;

	// set up an output event
	if (CurEvent = CurOpt->AddOutputEvent())
		{
		// set new file type
		strcpy(CurEvent->FileType, CurJob->TextureImageFormat);

		for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
			{
			CurEvent->OutBuffers[Ct][0] = 0;
			CurEvent->BufNodes[Ct] = NULL;
			} // for

		// set new values for buffers and codec
		strcpy(CurEvent->OutBuffers[0], "RED");
		strcpy(CurEvent->OutBuffers[1], "GREEN");
		strcpy(CurEvent->OutBuffers[2], "BLUE");
		strcpy(FormatStr, CurJob->OutPath.GetName());
		strcat(FormatStr, "_SkyNorthRGB");
		CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
		CurEvent->AutoDigits = 0;
		if (ImageExtension)
			strcat(FormatStr, ImageExtension);
		AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYNORTH);
		} // if

	// render the panorama
	CurRJ->SetCamera(CurCam);
	CurRJ->SetRenderOpt(CurOpt);
	// no copying so be careful not to mess these scenarios up
	CurRJ->Scenarios = CurJob->Scenarios;
	CurCam->InitToRender(CurOpt, NULL);
	((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
	CurRJ->InitToRender(CurOpt, NULL);

	// render the image so that all tiles are rendered and concatenated
	Success = RenderTerrain(CurJob, CurRJ);

	if (Success)
		{
		// make the camera non-targeted, non-panoramic
		CurCam->PanoCam = 0;

		// set heading to east
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(90.0);

		// set up output event
		if (CurEvent)
			{
			strcpy(FormatStr, CurJob->OutPath.GetName());
			strcat(FormatStr, "_SkyEastRGB");
			CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
			if (ImageExtension)
				strcat(FormatStr, ImageExtension);
			AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYEAST);
			} // if

		// render image
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);
		} // if success so far

	if (Success)
		{
		// make the camera non-targeted, non-panoramic
		CurCam->PanoCam = 0;

		// set heading to east
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(180.0);

		// set up output event
		if (CurEvent)
			{
			strcpy(FormatStr, CurJob->OutPath.GetName());
			strcat(FormatStr, "_SkySouthRGB");
			CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
			if (ImageExtension)
				strcat(FormatStr, ImageExtension);
			AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH);
			} // if

		// render image
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);
		} // if success so far

	if (Success)
		{
		// make the camera non-targeted, non-panoramic
		CurCam->PanoCam = 0;

		// set heading to east
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(-90.0);

		// set up output event
		if (CurEvent)
			{
			strcpy(FormatStr, CurJob->OutPath.GetName());
			strcat(FormatStr, "_SkyWestRGB");
			CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
			if (ImageExtension)
				strcat(FormatStr, ImageExtension);
			AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYWEST);
			} // if

		// render image
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);
		} // if success so far

	if (Success)
		{
		// make the camera non-targeted, non-panoramic
		CurCam->PanoCam = 0;

		// set pitch to straight up
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(0.0);
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(-90.0);

		// set up output event
		if (CurEvent)
			{
			strcpy(FormatStr, CurJob->OutPath.GetName());
			strcat(FormatStr, "_SkyTopRGB");
			CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
			if (ImageExtension)
				strcat(FormatStr, ImageExtension);
			AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYTOP);
			} // if

		// render image
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);
		} // if success so far

	if (Success)
		{
		// set the pitch to straight down
		CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(90.0);

		// set up output event
		if (CurEvent)
			{
			strcpy(FormatStr, CurJob->OutPath.GetName());
			strcat(FormatStr, "_SkyBottomRGB");
			CurEvent->PAF.SetPathAndName((char *)CurJob->OutPath.GetPath(), FormatStr);
			CurEvent->AutoDigits = 0;
			if (ImageExtension)
				strcat(FormatStr, ImageExtension);
			AddNewNameList(FileNamesCreated, FormatStr, WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM);
			} // if

		// render image
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);

		// render the image so that all tiles are rendered and concatenated
		Success = RenderTerrain(CurJob, CurRJ);
		} // if success so far

	CurRJ->Scenarios = NULL;
	delete CurRJ;
	delete CurCam;
	delete CurOpt;
	} // if

FrameTextClearA();
return (Success);

} // ExportControlGUI::ExportSky

/*===========================================================================*/

// creates a list of vectors
int ExportControlGUI::ExportVectors(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double VecAddElev, NNorthing, WEasting, SNorthing, EEasting;
long VecCt;
Joe *CurJoe;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
PlanetOpt *DefPlanetOpt;
VectorPoint *PLinkA, *PLinkB;
int Success = 1;
VertexDEM Vert;

SetFrameTextA("Exporting Vectors");

// run scenarios
//#ifdef WCS_RENDER_SCENARIOS
//EffectsHost->InitScenarios(CurJob->Scenarios, CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue, DBHost);
//#endif // WCS_RENDER_SCENARIOS

if (DefPlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
	{
	VecAddElev = CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].CurValue;

	// walk database and for each enabled vector or control point object
	// count items
	CurJob->NumVecInstances = 0;
	for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (CurJoe->TestRenderFlags())
					{
					if (CurJoe->GetNumRealPoints() > 0)
						{
						if (CurJoe->GetLineWidth() > 0)
							{
							// test if it is in-bounds
							// convert joe bounds to RBounds
							// NW corner
							CurJob->RBounds.DefDegToRBounds(CurJoe->NWLat, CurJoe->NWLon, WEasting, NNorthing);
							// SE corner
							CurJob->RBounds.DefDegToRBounds(CurJoe->SELat, CurJoe->SELon, EEasting, SNorthing);
							if (CurJob->RBounds.TestBoundsOverlap(NNorthing, WEasting, SNorthing, EEasting))
								{
								CurJob->NumVecInstances ++;
								} // if
							} // if
						} // if
					} // if
				} // if
			} // if
		} // for

	if (CurJob->NumVecInstances > 0)
		{
		// create an array of VectorExportItems
		if (CurJob->VecInstanceList = new VectorExportItem[CurJob->NumVecInstances])
			{
			for (VecCt = 0, CurJoe = DBHost->GetFirst(); CurJoe && Success && VecCt < CurJob->NumVecInstances; CurJoe = DBHost->GetNext(CurJoe))
				{
				if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (CurJoe->TestRenderFlags())
							{
							if (CurJoe->GetNumRealPoints() > 0)
								{
								if (CurJoe->GetLineWidth() > 0)
									{
									// test if it is in-bounds
									// convert joe bounds to RBounds
									// NW corner
									CurJob->RBounds.DefDegToRBounds(CurJoe->NWLat, CurJoe->NWLon, WEasting, NNorthing);
									// SE corner
									CurJob->RBounds.DefDegToRBounds(CurJoe->SELat, CurJoe->SELon, EEasting, SNorthing);
									if (CurJob->RBounds.TestBoundsOverlap(NNorthing, WEasting, SNorthing, EEasting))
										{
										// stash Joe pointer and number of vertices
										CurJob->VecInstanceList[VecCt].MyJoe = CurJoe;
										CurJob->VecInstanceList[VecCt].NumPoints = CurJoe->GetNumRealPoints();	// good old label point
										// allocate VectorPoints
										if (CurJob->VecInstanceList[VecCt].Points = DBHost->MasterPoint.Allocate(CurJob->VecInstanceList[VecCt].NumPoints))
											{
											// is there an attached CoordSys
											if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
												MyCoords = MyAttr->Coord;
											else
												MyCoords = NULL;
											// for each vertex
											for (PLinkA = CurJoe->GetFirstRealPoint(), PLinkB = CurJob->VecInstanceList[VecCt].Points;
												PLinkA && PLinkB; PLinkA = PLinkA->Next, PLinkB = PLinkB->Next)
												{
												// copy vertices
												PLinkB->Latitude = PLinkA->Latitude;
												PLinkB->Longitude = PLinkA->Longitude;
												PLinkB->Elevation = (float)(VecAddElev + CalcExag((double)PLinkA->Elevation, DefPlanetOpt));
												// translate vertices into default lat/lon
												if (MyCoords)
													{
													PLinkB->ProjToDefDeg(MyCoords, &Vert);
													PLinkB->Latitude = Vert.Lat;
													PLinkB->Longitude = Vert.Lon;
													PLinkB->Elevation = (float)(Vert.Elev);
													} // if
												} // for
											} // if
										else
											Success = 0;
										VecCt ++;
										} // if
									} // if
								} // if
							} // if
						} // if
					} // if
				} // for
			} // if
		else
			Success = 0;
		} // if

	} // if
else
	Success = 0;

//#ifdef WCS_RENDER_SCENARIOS
//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
//#endif // WCS_RENDER_SCENARIOS

FrameTextClearA();
return (Success);

} // ExportControlGUI::ExportVectors

/*===========================================================================*/

// exports 3d objects, creates instance lists and combines them for 3d objects and foliage 3d objects
// removes 3d objects from foliage files
// renders textures for any 3d objects that need them
int ExportControlGUI::Export3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long Sum3DOs = 0, ObjCt;
Object3DEffect *CurObj;
Object3DInstance *FolObjectList = NULL, *NonFolObjectList = NULL, *CurInstance;
char TempPreview;
int Success = 1;

if (! CurJob->ObjectAsObj)
	return (1);

TempPreview = Preview;
SetFrameTextA("Exporting 3D Objects");

// for each instance in the 3d instance list, determine what the rotations and scaling would be for each
// calculate a geographic coordinate for each, including any transformations
// apply the transformation and back-calculate the lat/lon/elev
// write the data into a structure, one instance of an object per structure
// write out a file so other savers can access it

// set ShortName for all regular 3d objects
for (ObjCt = 0, CurObj = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurObj; CurObj = (Object3DEffect *)CurObj->Next, ObjCt ++)
	{
	sprintf(CurObj->ShortName, "OB%06d", ObjCt);
	} // for

// create a renderer, initiate it so that 3d objects get initialized
// be sure RBounds has been set by an ealier process, if not, set them
if (CurJob->Export3DObjects)
	Success = ExportObjects(CurJob, FileNamesCreated, &NonFolObjectList);
if (Success && CurJob->Export3DFoliage)
	Success = Export3DFoliageObjects(CurJob, FileNamesCreated, &FolObjectList);
if (Success)
	{
	// sum the foliage and 3do list instances, append them together
	// count the number of 3do instances in the list that fall within the output boundaries
	if (NonFolObjectList)
		Sum3DOs = NonFolObjectList->CountBoundedInstances(&CurJob->RBounds);
	if (FolObjectList)
		Sum3DOs += FolObjectList->CountBoundedInstances(&CurJob->RBounds);
	if (CurJob->ObjectInstanceList && NonFolObjectList)
		{
		CurInstance = CurJob->ObjectInstanceList;
		while (CurInstance->Next)
			{
			CurInstance = CurInstance->Next;
			} // if
		CurInstance->Next = NonFolObjectList;
		} // if
	else if (NonFolObjectList)
		{
		CurJob->ObjectInstanceList = NonFolObjectList;
		} // else if
	NonFolObjectList = NULL;
	if (CurJob->ObjectInstanceList && FolObjectList)
		{
		CurInstance = CurJob->ObjectInstanceList;
		while (CurInstance->Next)
			{
			CurInstance = CurInstance->Next;
			} // if
		CurInstance->Next = FolObjectList;
		} // if
	else if (FolObjectList)
		{
		CurJob->ObjectInstanceList = FolObjectList;
		} // else if
	FolObjectList = NULL;

	// render textures and substitute textured object for the original if needed
	Success = RenderExportObjects(CurJob, FileNamesCreated);
	} // if

SetPreview(TempPreview);
FrameTextClearA();

return (Success);

} // ExportControlGUI::Export3DObjects

/*===========================================================================*/

// creates an instance list of regular 3d objects, both geographic instances and vector placed
int ExportControlGUI::ExportObjects(SceneExporter *CurJob, NameList **FileNamesCreated, Object3DInstance **ObjectList)
{
double CurTime;
long FirstFrame, VertCt, MatCt;
RenderOpt *CurOpt = NULL;
RenderJob *CurRJ = NULL;
Camera *CurCam = NULL;
Object3DInstance **InstancePtr, *CurInstance;
int Success = 1;
char TempPreview;
VertexDEM ObjVert;
PolygonData Poly;

TempPreview = Preview;
InstancePtr = ObjectList;

if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	delete Rend;
	Rend = NULL;
	SetPreview(TempPreview);
	} // if

// create a camera, renderopt, render job
// create a renderer too
if ((Rend = new Renderer()) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob) && (CurCam = new Camera))
	{
	CurRJ->Cam = CurCam;
	CurRJ->Options = CurOpt;
	// turn off anything that would take a while to initialize and does not affect 3d objects
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ENVIRONMENT] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 0;
	// initialize it
	if (Rend->InitForProcessing(CurRJ, GlobalApp, EffectsHost, ImageHost, 
		DBHost, ProjectHost, GlobalApp->StatusLog, FALSE))	// false=not just elevations, need 3do's init'ed
		{
		Rend->Master = this;

		FirstFrame = (long)(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5);
		CurTime = CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue;
		CurCam->InitToRender(CurOpt, NULL);
		((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
		CurRJ->InitToRender(CurOpt, NULL);
		Rend->InitFrame(CurTime, FirstFrame, 1);
		// steal data from the list of 3d objects
		// are there any 3do's?
		if (EffectsHost->Object3DBase.VertList)
			{
			for (VertCt = 0; VertCt < EffectsHost->Object3DBase.VerticesToRender; VertCt ++)
				{
				// add a new 3DOInstance and fill in the data
				if (*InstancePtr = new Object3DInstance)
					{
					CurInstance = *InstancePtr;
					// find lat/lon position
					EffectsHost->Object3DBase.VertList[VertCt].Obj->InitToRender(CurOpt, NULL);
					EffectsHost->Object3DBase.VertList[VertCt].Obj->InitFrameToRender(EffectsHost, &Rend->RendData);
					ObjVert.Lat = EffectsHost->Object3DBase.VertList[VertCt].Lat;
					ObjVert.Lon = EffectsHost->Object3DBase.VertList[VertCt].Lon;
					// transfer data 
					EffectsHost->Object3DBase.VertList[VertCt].Obj->FindBasePosition(&Rend->RendData, &ObjVert, &Poly, EffectsHost->Object3DBase.VertList[VertCt].Vec, EffectsHost->Object3DBase.VertList[VertCt].Point);
					EffectsHost->Object3DBase.VertList[VertCt].Obj->CompleteInstanceInfo(&Rend->RendData, CurInstance, &Poly, 
						0.0, 0.0, 0.0, ObjVert.Lat, ObjVert.Lon, ObjVert.Elev, NULL, -1.0);
					// determine if it is inbounds
					if (CurJob->RBounds.IsDefGeoPointBounded(CurInstance->WCSGeographic[1], CurInstance->WCSGeographic[0]))
						{
						// null out the ShortName so it can be assigned uniquely by the exporter
						for (MatCt = 0; MatCt < EffectsHost->Object3DBase.VertList[VertCt].Obj->NumMaterials; MatCt ++)
							{
							if (EffectsHost->Object3DBase.VertList[VertCt].Obj->NameTable[MatCt].Mat)
								EffectsHost->Object3DBase.VertList[VertCt].Obj->NameTable[MatCt].Mat->ShortName[0] = 0;
							} // for
						#ifdef WCS_BUILD_SX2
						if (EffectsHost->Object3DBase.VertList[VertCt].Obj->IsClickQueryEnabled())
							CurInstance->ClickQueryObjectID = EffectsHost->Object3DBase.VertList[VertCt].Obj->GetRecordNumber(EffectsHost->Object3DBase.VertList[VertCt].Obj, Poly.Vector, FileNamesCreated);
						#endif // WCS_BUILD_SX2
						InstancePtr = &CurInstance->Next;
						} // if
					else
						{
						delete CurInstance;
						*InstancePtr = NULL;
						} // else
					} // if
				else
					{
					Success = 0;
					break;
					} // else
				} // for
			} // if
		} // if
	else
		Success = 0;
	// cleanup renderer
	Rend->Cleanup(true, false, TRUE, false);
	CurOpt->CleanupFromRender(TRUE);
	} // if objects created
else
	Success = 0;

if (CurCam)
	delete CurCam;
if (CurOpt)
	delete CurOpt;
if (CurRJ)
	delete CurRJ;

return (Success);

} // ExportControlGUI::ExportObjects

/*===========================================================================*/

// creates an instance list of any 3d objects placed as members of ecotypes (ecosystems and foliage effects)
int ExportControlGUI::Export3DFoliageObjects(SceneExporter *CurJob, NameList **FileNamesCreated, Object3DInstance **ObjectList)
{
double CurTime;
long DatPt, FirstFrame, MatCt;
RenderOpt *CurOpt = NULL;
RenderJob *CurRJ = NULL;
Camera *CurCam = NULL;
Object3DInstance *FolObjectList = NULL, *NonFolObjectList = NULL, **InstancePtr, *CurInstance;
const char *OutputFilePath;
char TempPreview;
FILE *FolDatFile;
int Success = 1;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;

// The directory where all the files should be created is:
OutputFilePath = CurJob->OutPath.GetPath();
InstancePtr = ObjectList;
TempPreview = Preview ? 1: 0;

// open the foliage data file if there is one and count the number of 3do instances in it.
if (CurJob->Export3DFoliage)
	{
	if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated))
		{
		if (Rend)
			{
			ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
			ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
			delete Rend;
			Rend = NULL;
			SetPreview(TempPreview);
			} // if

		// create a Renderer
		// do InitForProcessing
		if ((Rend = new Renderer()) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob) && (CurCam = new Camera))
			{
			CurRJ->Cam = CurCam;
			CurRJ->Options = CurOpt;
			CurCam->InitToRender(CurOpt, NULL);
			((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
			CurRJ->InitToRender(CurOpt, NULL);
			// turn off anything that would take a while to initialize and does not affect foliage
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ENVIRONMENT] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
			CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 0;
			if (Rend->InitForProcessing(CurRJ, GlobalApp, EffectsHost, ImageHost, 
				DBHost, ProjectHost, GlobalApp->StatusLog, TRUE))	// true= elevation only, just need basic init
				{
				Rend->Master = this;

				FirstFrame = (long)(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5);
				CurTime = CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue;
				Rend->InitFrame(CurTime, FirstFrame, 1);

				for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt ++)
					{
					if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
						{
						if (FolData.InterpretFoliageRecord(GlobalApp->AppEffects, NULL, &PointData)) // don't need full decoding of 3dobjects, just height, etc
							{
							if (PointData.Object3D)
								{
								// init this so the object saver knows to set the name sequentially
								// this has been initialized before this function was called so shouldn't be nulled here
								//PointData.Object3D->ShortName[0] = 0;
								for (MatCt = 0; MatCt < PointData.Object3D->NumMaterials; MatCt ++)
									{
									// find material
									if (PointData.Object3D->NameTable[MatCt].Mat ||
										(PointData.Object3D->NameTable[MatCt].Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, PointData.Object3D->NameTable[MatCt].Name)))
										{
										// init material ShortName[0] so it gets initialized uniquely during object save
										PointData.Object3D->NameTable[MatCt].Mat->ShortName[0] = 0;
										} // if
									} // for
								// we have an interpreted data record
								// add a new 3DOInstance and fill in the data
								if (*InstancePtr = new Object3DInstance)
									{
									CurInstance = *InstancePtr;
									// transfer data 
									PointData.Object3D->InitToRender(CurOpt, NULL);
									PointData.Object3D->InitFrameToRender(EffectsHost, &Rend->RendData);
									PointData.Object3D->CompleteInstanceInfo(&Rend->RendData, CurInstance, &Index, &FolData, &PointData);
									#ifdef WCS_BUILD_SX2
									if (FolData.MyEffect)
										{
										if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_FOLIAGE)
											CurInstance->ClickQueryObjectID = ((FoliageEffect *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
										else if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LABEL)
											CurInstance->ClickQueryObjectID = ((Label *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
										else if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D)
											CurInstance->ClickQueryObjectID = ((Object3DEffect *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
										} // if
									#endif // WCS_BUILD_SX2
									InstancePtr = &CurInstance->Next;
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								} // if
							} // if 
						} // if data record read
					} // for
				} // if
			Rend->Cleanup(true, false, TRUE, false);
			CurOpt->CleanupFromRender(TRUE);
			} // if
		fclose(FolDatFile);
		} // if data file opened
	Index.CellDat = NULL;
	// now can safely remove 3d objects from foliage files to avoid confusion downstream
	if (Success)
		Success = StripObjectsFromFoliageFiles(CurJob, FileNamesCreated);
	} // if export 3d foliage

if (CurCam)
	delete CurCam;
if (CurOpt)
	delete CurOpt;
if (CurRJ)
	delete CurRJ;

return (Success);

} // ExportControlGUI::Export3DFoliageObjects

/*===========================================================================*/

// removes 3d objects from foliage files so downstream process can assume that all records are strictly image foliage
int ExportControlGUI::StripObjectsFromFoliageFiles(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long FileType, DatPt, CountPos, EntitiesWritten = 0;
const char *OutputFilePath, *DataFileName, *IndexFileName;
FILE *FolDatFile, *rewritefile;
long NewCellDatCt;
int Success = 1;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;
char OrigFileName[512], NewFileName[512], FileDescriptor[256], MesgStr[600], TestFileVersion;

OutputFilePath = CurJob->OutPath.GetPath();
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
DataFileName = (*FileNamesCreated)->FindNameOfType(FileType);
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (DataFileName && (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType)))
	{
	strmfp(OrigFileName, OutputFilePath, DataFileName);

	if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, &RFCD, FileNamesCreated, &CountPos))
		{
		TestFileVersion = Index.FileVersion;
		strmfp(NewFileName, OutputFilePath, DataFileName);
		strcat(NewFileName, "_Stripped");
		if (rewritefile = PROJ_fopen(NewFileName, "wb"))
			{
			// write file header
			fwrite((char *)FileDescriptor, strlen(FileDescriptor) + 1, 1, rewritefile);
			fputc('\n', rewritefile);	// so variable length file descriptor can be read by fgets
			// file version number
			fwrite((char *)&TestFileVersion, sizeof (char), 1, rewritefile);

			NewCellDatCt = Index.CellDat->DatCt;
			for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt ++)
				{
				if (FolData.ReadFoliageRecord(FolDatFile, TestFileVersion))
					{
					if (FolData.InterpretFoliageRecord(GlobalApp->AppEffects, NULL, &PointData)) // don't need full decoding of 3dobjects, just height, etc
						{
						if (PointData.Object3D)
							{
							NewCellDatCt --;
							} // if
						else
							{
							// write out foliage record
							if (! FolData.WriteFoliageRecord(rewritefile))
								{
								Success = 0;
								ExportError("Strip Objects from Foliage Files", "Error writing foliage record.");
								} // if
							else
								{
								EntitiesWritten ++;
								} // else
							} // else
						} // if 
					} // if data record read
				else
					{
					Success = 0;
					ExportError("Strip Objects from Foliage Files", "Error reading foliage record.");
					} // else
				} // for
			fclose(rewritefile);
			} // if rewrite file opened
		else
			{
			Success = 0;
			sprintf(MesgStr, "Error opening Foliage Data file for rewrite: %s", NewFileName);
			ExportError("Strip Objects from Foliage Files", MesgStr);
			} // else

		fclose(FolDatFile);
		if (EntitiesWritten)
			{
			// delete original file
			PROJ_remove(OrigFileName);
			// rename new file to the old name
			PROJ_rename(NewFileName, OrigFileName);
			// rewrite index file with new number of entities
			strmfp(OrigFileName, OutputFilePath, IndexFileName);
			if (FolDatFile = PROJ_fopen(OrigFileName, "rb+"))
				{
				if (! fseek(FolDatFile, CountPos, SEEK_SET))
					fwrite(&NewCellDatCt, sizeof (long), 1, FolDatFile);
				fclose(FolDatFile);
				} // if
			else
				{
				Success = 0;
				ExportError("Strip Objects from Foliage Files", "Error reopening Foliage Index file for point count modification.");
				} // else
			} // if
		else
			{
			// delete rewritten file, no entities removed
			PROJ_remove(NewFileName);
			} // else
		} // if data file opened
	Index.CellDat = NULL;
	} // if index file name

return (Success);

} // ExportControlGUI::StripObjectsFromFoliageFiles

/*===========================================================================*/

// prepares to render 3d object textures as needed
int ExportControlGUI::RenderExportObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
Camera *CurCam;
RenderOpt *CurOpt;
RenderJob *CurRJ;
ImageOutputEvent *CurEvent;
char *ImageExtension, *DefBuf;
int Success = 1, Ct;

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);

// are there any objects?
if (! EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D))
	return (1);

// Best to try to do one wall at a time, using a similar process to what the Renderer does but controlling the flow
// so that one bitmap at a time is created rather than rendering the wall segments in Z-sorted order.

// A Renderer would perhaps make life easier but ViewGUI doesn't use one so it probably isn't necessary.
// How to do texture sampling and image plotting without using RenderPoly is perhaps less pleasant.


// How 'bout we create and init a Renderer with everything turned off except terraffectors and area tfx and walls.
// The list of fence parts is sorted by the fencs component and vector at that stage.
// We can walk that list, checking for either a vector or effect change which triggers new calculations and saves previous.

// create a camera, render opt and render job
if ((CurCam = new Camera) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob))
	{
	// make it an orthograhic camera so there is no perspective texture warping
	CurCam->Orthographic = 1;
	// disable stuff
	CurOpt->OutputImageWidth = CurOpt->OutputImageHeight = CurJob->Max3DOTexSize;	// temporary
	CurOpt->ReflectionsEnabled = 0;
	// might as well turn off shading and shadowing since we are exporting only one instance of an object that 
	// might be replicated in may different orientations.
	CurOpt->CloudShadowsEnabled = 
		CurOpt->ObjectShadowsEnabled = 
		CurOpt->TerrainEnabled = 
		CurOpt->FoliageEnabled = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 0;	//(CurJob->BurnShadows && CurJob->BurnShading);
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = 0;	//CurJob->BurnShading;
	CurOpt->RenderDiagnosticData = CurOpt->MultiPassAAEnabled = CurOpt->DepthOfFieldEnabled = 0;
	CurOpt->TerrainEnabled = 0;
	CurOpt->VectorsEnabled = 0;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] =  
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 0;
	CurOpt->FragmentRenderingEnabled = 1;
	CurOpt->FragmentDepth = 20;
	CurOpt->VolumetricsEnabled = 0;
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->TempPath);

	CurJob->FragmentCollapseType = WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST;

	// set up an output event
	if (CurEvent = CurOpt->AddOutputEvent())
		{
		// set new file type
		strcpy(CurEvent->FileType, CurJob->TextureImageFormat);

		for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
			{
			CurEvent->OutBuffers[Ct][0] = 0;
			CurEvent->BufNodes[Ct] = NULL;
			} // for
		if (DefBuf = ImageSaverLibrary::GetNextCodec(CurEvent->FileType, NULL))
			strcpy(CurEvent->Codec, DefBuf);

		// set new values for buffers and codec
		strcpy(CurEvent->OutBuffers[0], "RED");
		strcpy(CurEvent->OutBuffers[1], "GREEN");
		strcpy(CurEvent->OutBuffers[2], "BLUE");
		strcpy(CurEvent->OutBuffers[3], "ANTIALIAS");
		} // if

	// render the panorama
	CurRJ->SetCamera(CurCam);
	CurRJ->SetRenderOpt(CurOpt);
	// no copying so be careful not to mess these scenarios up
	CurRJ->Scenarios = CurJob->Scenarios;
	CurCam->InitToRender(CurOpt, NULL);
	((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
	CurRJ->InitToRender(CurOpt, NULL);

	// render the image so that all tiles are rendered and concatenated
	Success = RenderObjects(CurJob, CurRJ, FileNamesCreated);

	CurRJ->Scenarios = NULL;
	delete CurRJ;
	delete CurCam;
	delete CurOpt;
	} // if

FrameTextClearA();
return (Success);

} // ExportControlGUI::RenderExportObjects

/*===========================================================================*/

// renders 3d object textures as needed
int ExportControlGUI::RenderObjects(SceneExporter *CurJob, RenderJob *CurRJ, NameList **FileNamesCreated)
{
#ifdef WCS_BUILD_RTX
double AxisCenter[3], AxisWidth[3], ImageStart[6][2], ImageCenter[6][2], AxisScale[3][2], OptimalWidth, OptimalHeight,
	WidthScale, HeightScale, CurTime, TextureShrinkage;
Object3DEffect *CurEffect, *Object3D;
MaterialEffect *CurMat, *PolyMat;
Object3DInstance *CurInstance;
RootTexture *CurTexRoot;
ObjectPerVertexMap *UVTable;
Polygon3D *CurPoly;
Vertex3D *CurVert, *Vtx[3];
ImageOutputEvent *IOE;
UVImageTexture *CurUVTex;
Raster *UVRaster;
long *StradlingVertices;
char *StradlingPolygons, *ImageExtension;
int Success = 1;
int NeedRendered, NeedRenderedThis, TransparencyExists = 0;
long MatCt, NewMatNum, TexNum, Index, PolyAxis, FirstAxisGroup, PolyAxisGroup, AxisGroup, PlotAxisGroup, RenderSide, SumPolyVerts, UsedVertRefs, VertCt, 
	VertRefCt, PolyCt, PolyRefCt, TotalVertices, ImageWidth, ImageHeight, FirstFrame, UntexturedMaterials, poly, p0, p1, p2;
PolygonData Poly;
PathAndFile ImagePAF;
char TempName[256], BufBackup[WCS_MAX_BUFFERNODE_NAMELEN], TempPreview;
unsigned char PrevMapUsed;

// well here goes - a method to export a 3d object complete with a texture map
// if the object already has UV textures and only one UVW map then use it as is

// only render objects that have textures and that specify an image format
if (! CurJob->TextureImageFormat || strlen(CurJob->TextureImageFormat) == 0)
	return (Success);

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);
if (! ImageExtension || strlen(ImageExtension) == 0)
	return (Success);

if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	TempPreview = Preview;
	delete Rend;
	Rend = NULL;
	SetPreview(TempPreview);
	} // if

if (Rend = new Renderer())
	{
	//Rend->Exporter = CurJob;
	FirstFrame = (long)(STARTTIME * FRAMERATE + .5);
	CurTime = FirstFrame / FRAMERATE;
	#ifdef WCS_BUILD_DEMO
	if (FirstFrame > 150)
		{
		UserMessageDemo("Highest frame number you can render is 150.");
		if (FirstFrame > 150)
			FirstFrame = 150;
		} // if
	if (CurTime > 5.0)
		{
		UserMessageDemo("Highest time you can render is 5 seconds.");
		CurTime = 5.0;
		} // if
	#endif // WCS_BUILD_DEMO
	//#ifdef WCS_RENDER_SCENARIOS
	// init enabled states at first frame to be rendered before init effects
	//EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
	//#endif // WCS_RENDER_SCENARIOS
	if (Rend->Init(CurRJ, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, TRUE))
		{
		SetFrameTextA("Rendering Object Textures");
		// bitmaps are now allocated which we won't be using

		// only complain once per render session
		Rend->SetFrdWarned(FrdWarned);

		// render shadows - steal some code from RenderFrame
		GetTime(Rend->StartSecs);
		GetTime(Rend->FrameSecs);

		// initialize everything for this frame
		if (Rend->InitFrame(CurTime, FirstFrame, TRUE))
			{
			// walk the 3d object list in EffectsLib
			for (CurEffect = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurEffect; 
				CurEffect = (Object3DEffect *)CurEffect->Next)
				{
				// compare objects there to objects in the 3d object export list
				for (CurInstance = CurJob->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
					{
					// if the same object is found then create a replacement object and for each instance in the list, fill in the replacement
					if (CurInstance->MyObj == CurEffect)
						{
						CurJob->Unique3DObjectInstances ++;
						// there is a possibility that the object won't be needed: If CurJob->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY
						// and the object format is of the correct type for the format of export then the object will be used "as is."
						if (CurJob->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY)
							{
							if (ExportFmt->FindCopyable3DObjFile(CurEffect))
								// this object is not needed, don't look for any more instances of it
								break;
							} // if

						// test to see if all materials exist before trying to render it
						NeedRendered = 1;
						for (MatCt = 0; MatCt < CurEffect->NumMaterials; MatCt ++)
							{
							if (! ((CurMat = CurEffect->NameTable[MatCt].Mat) || 
								(CurMat = (CurEffect->NameTable[MatCt].Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CurEffect->NameTable[MatCt].Name)))))
								{
								// no material, don't render it
								NeedRendered = 0;
								} // if
							} // for 

						// at least one material is missing, skip this object, don't look for any more instances of it
						if (! NeedRendered)
							break;

						// we found a match, test for textures in each material
						NeedRendered = 0;
						UntexturedMaterials = 0;
						PrevMapUsed = 0;
						for (MatCt = 0; MatCt < CurEffect->NumMaterials; MatCt ++)
							{
							if ((CurMat = CurEffect->NameTable[MatCt].Mat) || 
								(CurMat = (CurEffect->NameTable[MatCt].Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CurEffect->NameTable[MatCt].Name))))
								{
								NeedRenderedThis = 0;
								if (CurMat->AreThereEnabledTextures())
									{
									// we'll need to know if there is transparency so we know to save alpha channel
									TransparencyExists = (TransparencyExists || CurMat->IsThereTransparentMaterial());
									// there is at least one texture so may need to render
									// check to see if it is a UV texture
									for (TexNum = 0; ! NeedRenderedThis && TexNum < CurMat->GetNumTextures(); TexNum ++)
										{
										if (CurTexRoot = CurMat->GetTexRootPtr(TexNum))
											{
											if (CurTexRoot->Enabled)
												{
												// if either textures are all disabled or it is a UV texture only, we don't need to render
												if (CurTexRoot->IsNeedRendered(CurEffect, CurJob->MultipleObjectUVMappingsSupported(), PrevMapUsed))
													NeedRendered = NeedRenderedThis = 1;
												} // if
											} // if
										} // for
									} // if
								if (! NeedRenderedThis)
									UntexturedMaterials ++;
								} // if
							// no material, don't render it
							else
								break;
							} // for 

						if (NeedRendered)
							{
							// has textures, process this object
							if (Object3D = new Object3DEffect)
								{
								CurJob->AddObjectToEphemeralList(Object3D);

								Object3D->Copy(Object3D, CurEffect);
								// delete material table
								Object3D->FreeDynamicStuff();
								// create new material table with one material
								// if there are untextured materials, add the number of those
								if (Object3D->NameTable = new ObjectMaterialEntry[UntexturedMaterials + 1])
									Object3D->NumMaterials = UntexturedMaterials + 1;
								// create single UVW table
								Object3D->AllocUVWMaps(1);
								UVTable = &Object3D->UVWTable[0];
								// create a material
								if (CurMat = new MaterialEffect(NULL))
									{
									CurMat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
									CurMat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
									CurMat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
									CurJob->AddObjectToEphemeralList(CurMat);
									// assign name to material, set properties
									strcpy(CurMat->Name, Object3D->ShortName[0] ? Object3D->ShortName: Object3D->Name);
									CurMat->ShortName[0] = 0;
									CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
									// not burning in shading since object may be replicated with different orientations.
									//if (CurJob->BurnShading)
									//	CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
									CurMat->DoubleSided = 1;

									// assign a map name
									UVTable->SetName(Object3D->Name);

									// load the original object
									if (Object3D->NameTable && Object3D->UVWTable && 
										((CurEffect->Vertices && CurEffect->Polygons && CurEffect->NameTable) || CurEffect->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
										{
										strcpy(Object3D->NameTable[0].Name, CurMat->Name);
										Object3D->NameTable[0].Mat = CurMat;
										StradlingPolygons = NULL;	// a number between 0 and 5 will be used to identify the group for each polygon
										StradlingVertices = NULL;	// 6 arrays will store the vertex numbers for use with each orientation
										if ((StradlingPolygons = (char *)AppMem_Alloc(CurEffect->NumPolys * sizeof (char), APPMEM_CLEAR))
											&& (StradlingVertices = (long *)AppMem_Alloc(6 * CurEffect->NumVertices * sizeof (long), APPMEM_CLEAR)))
											{
											for (VertCt = 0; VertCt < 6 * CurEffect->NumVertices; VertCt ++)
												StradlingVertices[VertCt] = -1;
											// find center and width of each axis
											AxisCenter[0] = (CurEffect->ObjectBounds[0] + CurEffect->ObjectBounds[1]) * .5;
											AxisWidth[0] = fabs(CurEffect->ObjectBounds[0] - CurEffect->ObjectBounds[1]);
											AxisCenter[1] = (CurEffect->ObjectBounds[2] + CurEffect->ObjectBounds[3]) * .5;
											AxisWidth[1] = fabs(CurEffect->ObjectBounds[2] - CurEffect->ObjectBounds[3]);
											AxisCenter[2] = (CurEffect->ObjectBounds[4] + CurEffect->ObjectBounds[5]) * .5;
											AxisWidth[2] = fabs(CurEffect->ObjectBounds[4] - CurEffect->ObjectBounds[5]);

											// copy vertex positions over to XYX array so polygons can be normalized
											for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
												{
												CurVert = &CurEffect->Vertices[VertCt];
												CurVert->XYZ[0] = CurVert->xyz[0];
												CurVert->XYZ[1] = CurVert->xyz[1];
												CurVert->XYZ[2] = CurVert->xyz[2];
												} // for VertCt

											// figure out which polygon group each polygon belongs using surface normals
											// groups are 0=+x, 1=-x, 2=+y, 3=-y, 4=+z, 5=-z
											SumPolyVerts = 0;
											for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
												{
												CurPoly = &CurEffect->Polygons[PolyCt];
												SumPolyVerts += CurPoly->NumVerts;
												// set polygon normalized flag = 0
												CurPoly->Normalized = 0;
												// normalize polygon
												CurPoly->Normalize(CurEffect->Vertices);
												// flip normals if necessary
												PolyMat = CurEffect->NameTable[CurPoly->Material].Mat;
												if (PolyMat->FlipNormal)
													NegateVector(CurPoly->Normal);
												
												if (fabs(CurPoly->Normal[0]) >= fabs(CurPoly->Normal[1]))
													{
													if (fabs(CurPoly->Normal[0]) >= fabs(CurPoly->Normal[2]))
														PolyAxis = 0;
													else
														PolyAxis = 2;
													} // if
												else
													{
													if (fabs(CurPoly->Normal[1]) >= fabs(CurPoly->Normal[2]))
														PolyAxis = 1;
													else
														PolyAxis = 2;
													} // else
												AxisGroup = 0;
												for (VertRefCt = 0; VertRefCt < CurPoly->NumVerts; VertRefCt ++)
													{
													CurVert = &CurEffect->Vertices[CurPoly->VertRef[VertRefCt]];
													AxisGroup += (CurVert->xyz[PolyAxis] - AxisCenter[PolyAxis] >= 0 ? 1: -1);
													} // if
												AxisGroup = (AxisGroup >= 0) ? 0: 1;	// 0 is on the pos end of axis, 1 on neg
												PolyAxisGroup = PolyAxis * 2 + AxisGroup;
												StradlingPolygons[PolyCt] = (char)PolyAxisGroup;
												} // for

											// determine how many replicate vertices will be required and which polygons get them
											TotalVertices = CurEffect->NumVertices;
											for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
												{
												CurVert = &CurEffect->Vertices[VertCt];
												FirstAxisGroup = -1;
												for (PolyRefCt = 0; PolyRefCt < CurVert->NumPolys; PolyRefCt ++)
													{
													PolyCt = CurVert->PolyRef[PolyRefCt];
													PolyAxisGroup = StradlingPolygons[PolyCt];
													// for any instances that are in the first group found, use original vertex
													if (FirstAxisGroup < 0)
														FirstAxisGroup = PolyAxisGroup;
													Index = PolyAxisGroup * CurEffect->NumVertices + VertCt;
													if (PolyAxisGroup == FirstAxisGroup)
														StradlingVertices[Index] = VertCt;
													else if (StradlingVertices[Index] == -1)
														StradlingVertices[Index] = TotalVertices ++;
													} // for
												} // for

											// allocate vertices and polygons including any duplicates
											Object3D->NumVertices = TotalVertices;
											Object3D->NumPolys = CurEffect->NumPolys;
											Object3D->Vertices = new Vertex3D[Object3D->NumVertices];
											Object3D->Polygons = new Polygon3D[Object3D->NumPolys];
											Object3D->AllocVertRef(SumPolyVerts);
											// allocate UV table with number of vertices
											UVTable->AllocMap(Object3D->NumVertices);

											if (UVTable->MapsValid() && Object3D->Vertices && Object3D->Polygons && Object3D->VertRefBlock)
												{
												Object3D->VertexUVWAvailable = 1;
												// copy the vertices and polygons
												UsedVertRefs = 0;
												for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
													{
													CurPoly = &Object3D->Polygons[PolyCt];
													PolyAxisGroup = StradlingPolygons[PolyCt];
													// copy CurEffect->Polygons[PolyCt] to Object3D->Polygons[PolyCt]
													CurPoly->Copy(CurPoly, &CurEffect->Polygons[PolyCt]);
													CurPoly->Material = 0;
													CurPoly->Normalized = 0;
													CurPoly->VertRef = &Object3D->VertRefBlock[UsedVertRefs];
													UsedVertRefs += CurPoly->NumVerts;
													// change polygon references to the new vertices as needed
													for (VertRefCt = 0; VertRefCt < CurPoly->NumVerts; VertRefCt ++)
														{
														VertCt = CurEffect->Polygons[PolyCt].VertRef[VertRefCt];
														Index = PolyAxisGroup * CurEffect->NumVertices + VertCt;
														VertCt = StradlingVertices[Index] >= 0 ? StradlingVertices[Index]: 0;
														CurPoly->VertRef[VertRefCt] = VertCt;
														} // for
													} // for
												for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
													{
													// copy CurEffect->Vertices[VertCt] to Object3D->Vertices[VertCt]
													Object3D->Vertices[VertCt].Copy(&Object3D->Vertices[VertCt], &CurEffect->Vertices[VertCt]);
													Object3D->Vertices[VertCt].NumPolys = 0;
													} // for
												// duplicate the vertices
												for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
													{
													for (VertRefCt = 0; VertRefCt < 6; VertRefCt ++)
														{
														Index = VertRefCt * CurEffect->NumVertices + VertCt;
														if (StradlingVertices[Index] > VertCt)
															{
															// copy CurEffect->Vertices[VertCt] to Object3D->Vertices[StradlingVertices[VertCt]]
															Object3D->Vertices[StradlingVertices[Index]].Copy(&Object3D->Vertices[StradlingVertices[Index]], &CurEffect->Vertices[VertCt]);
															Object3D->Vertices[StradlingVertices[Index]].NumPolys = 0;
															} // if
														} // for
													} // for

												// compute the bitmap size for the rendered texture image
												OptimalWidth = AxisWidth[0] + AxisWidth[2];
												OptimalHeight = 2 * AxisWidth[1] + 2 * AxisWidth[2];
												if (OptimalWidth > OptimalHeight)
													{
													ImageWidth = quickftol(OptimalWidth / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].CurValue) + 4;
													if (ImageWidth > CurJob->Max3DOTexSize)
														ImageWidth = CurJob->Max3DOTexSize;
													WidthScale = (ImageWidth - 4) / OptimalWidth;
													ImageHeight = quickftol(WidthScale * OptimalHeight) + 8;
													HeightScale = (ImageHeight - 8) / OptimalHeight;
													} // if
												else 
													{
													ImageHeight = quickftol(OptimalHeight / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].CurValue) + 8;
													if (ImageHeight > CurJob->Max3DOTexSize)
														ImageHeight = CurJob->Max3DOTexSize;
													HeightScale = (ImageHeight - 8) / OptimalHeight;
													ImageWidth = quickftol(HeightScale * OptimalWidth) + 4;
													WidthScale = (ImageWidth - 4) / OptimalWidth;
													} // if

												// create a special scale for each side of the cube so that
												// each face maps into integer number of pixels - helps texture seams
												AxisScale[0][0] = (AxisWidth[0] > 0.0 ? max(1.0, WCS_floor(AxisWidth[0] * WidthScale)) / AxisWidth[0]: 0.0);
												AxisScale[0][1] = (AxisWidth[0] > 0.0 ? max(1.0, WCS_floor(AxisWidth[0] * HeightScale)) / AxisWidth[0]: 0.0);
					
												AxisScale[1][0] = (AxisWidth[1] > 0.0 ? max(1.0, WCS_floor(AxisWidth[1] * WidthScale)) / AxisWidth[1]: 0.0);
												AxisScale[1][1] = (AxisWidth[1] > 0.0 ? max(1.0, WCS_floor(AxisWidth[1] * HeightScale)) / AxisWidth[1]: 0.0);

												AxisScale[2][0] = (AxisWidth[2] > 0.0 ? max(1.0, WCS_floor(AxisWidth[2] * WidthScale)) / AxisWidth[2]: 0.0);
												AxisScale[2][1] = (AxisWidth[2] > 0.0 ? max(1.0, WCS_floor(AxisWidth[2] * HeightScale)) / AxisWidth[2]: 0.0);

												ImageWidth = quickftol(WCS_ceil(AxisWidth[0] * AxisScale[0][0] + AxisWidth[2] * AxisScale[2][0]) + 4);
												ImageHeight = quickftol(2 * WCS_ceil(AxisWidth[1] * AxisScale[1][1]) + 2 * WCS_ceil(AxisWidth[2] * AxisScale[2][1]));

												// measured from bottom left of image
												ImageStart[0][0] = AxisWidth[0] * AxisScale[0][0] + 3.0;
												ImageStart[0][1] = 1.0;
												ImageCenter[0][0] = ImageStart[0][0] + .5 * AxisWidth[2] * AxisScale[2][0];
												ImageCenter[0][1] = ImageStart[0][1] + .5 * AxisWidth[1] * AxisScale[1][1];

												ImageStart[1][0] = AxisWidth[0] * AxisScale[0][0] + 3.0;
												ImageStart[1][1] = AxisWidth[1] * AxisScale[1][1] + 3.0;
												ImageCenter[1][0] = ImageStart[1][0] + .5 * AxisWidth[2] * AxisScale[2][0];
												ImageCenter[1][1] = ImageStart[1][1] + .5 * AxisWidth[1] * AxisScale[1][1];

												ImageStart[2][0] = 1.0;
												ImageStart[2][1] = 2.0 * AxisWidth[1] * AxisScale[1][1] + 5.0;
												ImageCenter[2][0] = ImageStart[2][0] + .5 * AxisWidth[0] * AxisScale[0][0];
												ImageCenter[2][1] = ImageStart[2][1] + .5 * AxisWidth[2] * AxisScale[2][1];

												ImageStart[3][0] = 1.0;
												ImageStart[3][1] = 2.0 * AxisWidth[1] * AxisScale[1][1] + AxisWidth[2] * AxisScale[2][1] + 7.0;
												ImageCenter[3][0] = ImageStart[3][0] + .5 * AxisWidth[0] * AxisScale[0][0];
												ImageCenter[3][1] = ImageStart[3][1] + .5 * AxisWidth[2] * AxisScale[2][1];

												ImageStart[4][0] = 1.0;
												ImageStart[4][1] = 1.0;
												ImageCenter[4][0] = ImageStart[4][0] + .5 * AxisWidth[0] * AxisScale[0][0];
												ImageCenter[4][1] = ImageStart[4][1] + .5 * AxisWidth[1] * AxisScale[1][1];

												ImageStart[5][0] = 1.0;
												ImageStart[5][1] = AxisWidth[1] * AxisScale[1][1] + 3.0;
												ImageCenter[5][0] = ImageStart[5][0] + .5 * AxisWidth[0] * AxisScale[0][0];
												ImageCenter[5][1] = ImageStart[5][1] + .5 * AxisWidth[1] * AxisScale[1][1];

												// compute and assign the UV coords of each vertex
												// some stretch of the texture assures that seams will be invisible.
												TextureShrinkage = 1.0 / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH].CurValue;
												for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
													{
													VertRefCt = 0;
													Index = VertCt;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[2] - AxisCenter[2]) * AxisScale[2][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[1] - AxisCenter[1]) * AxisScale[1][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[0] - AxisCenter[0]) / AxisWidth[0];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[0][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[0][1]) / ImageHeight);
														} // if
													VertRefCt = 1;
													Index += CurEffect->NumVertices;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[2] - AxisCenter[2]) * AxisScale[2][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[1] - AxisCenter[1]) * AxisScale[1][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[0] - AxisCenter[0]) / AxisWidth[0];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[1][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[1][1]) / ImageHeight);
														} // if
													VertRefCt = 2;
													Index += CurEffect->NumVertices;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[0] - AxisCenter[0]) * AxisScale[0][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[2] - AxisCenter[2]) * AxisScale[2][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[1] - AxisCenter[1]) / AxisWidth[1];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[2][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[2][1]) / ImageHeight);
														} // if
													VertRefCt = 3;
													Index += CurEffect->NumVertices;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[0] - AxisCenter[0]) * AxisScale[0][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[2] - AxisCenter[2]) * AxisScale[2][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[1] - AxisCenter[1]) / AxisWidth[1];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[3][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[3][1]) / ImageHeight);
														} // if
													VertRefCt = 4;
													Index += CurEffect->NumVertices;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[0] - AxisCenter[0]) * AxisScale[0][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[1] - AxisCenter[1]) * AxisScale[1][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[2] - AxisCenter[2]) / AxisWidth[2];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[4][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[4][1]) / ImageHeight);
														} // if
													VertRefCt = 5;
													Index += CurEffect->NumVertices;
													if (StradlingVertices[Index] >= VertCt)
														{
														CurVert = &Object3D->Vertices[StradlingVertices[Index]];
														CurVert->ScrnXYZ[0] = (CurVert->xyz[0] - AxisCenter[0]) * AxisScale[0][0];
														CurVert->ScrnXYZ[1] = (CurVert->xyz[1] - AxisCenter[1]) * AxisScale[1][1];
														CurVert->ScrnXYZ[2] = (CurVert->xyz[2] - AxisCenter[2]) / AxisWidth[2];
														UVTable->CoordsValid[StradlingVertices[Index]] = 1;
														UVTable->CoordsArray[0][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[0] * TextureShrinkage + ImageCenter[5][0]) / ImageWidth);
														UVTable->CoordsArray[1][StradlingVertices[Index]] = (float)((CurVert->ScrnXYZ[1] * TextureShrinkage + ImageCenter[5][1]) / ImageHeight);
														} // if
													} // for

												// count polygons associated with each vertex
												for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt ++)
													{
													CurPoly = &Object3D->Polygons[PolyCt];
													if (CurPoly->VertRef)
														{
														for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
															{
															Object3D->Vertices[CurPoly->VertRef[VertCt]].NumPolys ++;
															} // for
														} // for
													} // for

												// allocate PolyRef for each vertex
												for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt ++)
													{
													if (Object3D->Vertices[VertCt].NumPolys > 0)
														{
														Object3D->Vertices[VertCt].PolyRef = (long *)AppMem_Alloc(Object3D->Vertices[VertCt].NumPolys * sizeof (long), APPMEM_CLEAR);
														Object3D->Vertices[VertCt].NumPolys = 0;
														} // if
													} // for

												// fill in PolyRef for each vertex
												for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt ++)
													{
													CurPoly = &Object3D->Polygons[PolyCt];
													if (CurPoly->VertRef)
														{
														for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
															{
															CurVert = &Object3D->Vertices[CurPoly->VertRef[VertCt]];
															if (CurVert->PolyRef)
																{
																CurVert->PolyRef[CurVert->NumPolys] = PolyCt;
																CurVert->NumPolys ++;
																} // if
															} // for
														} // for
													} // for

												// normalize vertices for any formats that require normals precalculated
												for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt ++)
													{
													CurVert = &Object3D->Vertices[VertCt];
													if (CurVert->PolyRef)
														{
														CurVert->Normalized = 0;
														CurVert->Normalize(Object3D->Polygons, Object3D->Vertices, &Object3D->Polygons[CurVert->PolyRef[0]], Object3D->NameTable);
														} // if
													} // for

												// create a Renderer or whatever, alloc bitmaps or clear bitmaps
												// configure output
												ImagePAF.Copy(&ImagePAF, &CurJob->OutPath);
												strcpy(TempName, CurMat->Name);
												if (ImageExtension)
													AddExtension(TempName, ImageExtension);
												ImagePAF.SetName(TempName);

												Rend->Opt->OutputImageWidth = ImageWidth;
												Rend->Opt->OutputImageHeight = ImageHeight;

												IOE = Rend->Opt->OutputEvents;
												IOE->PAF.Copy(&IOE->PAF, &ImagePAF);
												IOE->AutoExtension = 0;
												IOE->AutoDigits = 0;

												// alloc new rasters
												if (Rend->SetupAndAllocBitmaps(ImageWidth, ImageHeight))
													{
													ProcInit(CurEffect->NumPolys, "Rendering 3D Object Texture");
													Poly.Object = CurEffect;
													Poly.BaseMat.Covg[0] = 1.0;
													Poly.ShadowFlags = 0;
													Poly.ReceivedShadowIntensity = 0.0;
													Poly.PlotOffset3DX = Poly.PlotOffset3DY = 0;
													Poly.RenderPassWeight = 1.0;

													// render each polygon of the original object into a bitmap
													ProcInit(CurEffect->NumPolys, "Rendering 3D Object Texture");
													for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
														{
														CurPoly = &CurEffect->Polygons[PolyCt];

														// skip invisible poly
														PolyMat = CurEffect->NameTable[CurEffect->Polygons[PolyCt].Material].Mat;
														if (PolyMat->Shading != WCS_EFFECT_MATERIAL_SHADING_INVISIBLE && PolyMat->Enabled)
															{
															// set basemat material
															Poly.BaseMat.Mat[0] = PolyMat;
															// copy normals to PolygonData
															Poly.Normal[0] = CurPoly->Normal[0];
															Poly.Normal[1] = CurPoly->Normal[1];
															Poly.Normal[2] = CurPoly->Normal[2];
															// calc vertex normals
															Poly.ShadeType = CurEffect->CalcNormals ? (char)PolyMat->Shading: WCS_EFFECT_MATERIAL_SHADING_PHONG;
															if (Poly.ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG && CurEffect->CalcNormals)
																{
																for (p0 = 0; p0 < CurPoly->NumVerts; p0 ++)
																	{
																	CurEffect->Vertices[CurPoly->VertRef[p0]].Normalized = 0;
																	CurEffect->Vertices[CurPoly->VertRef[p0]].Normalize(CurEffect->Polygons, CurEffect->Vertices, CurPoly, CurEffect->NameTable);
																	} // for
																} // if phong

															PolyAxisGroup = StradlingPolygons[PolyCt];
															PlotAxisGroup = (PolyAxisGroup % 2) ? PolyAxisGroup - 1: PolyAxisGroup;
															for (VertRefCt = 0; VertRefCt < CurPoly->NumVerts; VertRefCt ++)
																{
																VertCt = CurPoly->VertRef[VertRefCt];
																CurVert = &CurEffect->Vertices[VertCt];
																// copy screen coords
																// first render the positive side
																// get the basic offset coordinates
																Index = PolyAxisGroup * CurEffect->NumVertices + VertCt;
																if (StradlingVertices[Index] >= 0)
																	{
																	CurVert->ScrnXYZ[0] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[0];
																	CurVert->ScrnXYZ[1] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[1];
																	CurVert->ScrnXYZ[2] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[2];
																	} // if - failsafe
																// apply the correct offsets
																CurVert->ScrnXYZ[0] += ImageCenter[PlotAxisGroup][0];
																CurVert->ScrnXYZ[1] = ImageHeight - (CurVert->ScrnXYZ[1] + ImageCenter[PlotAxisGroup][1]);
																CurVert->ScrnXYZ[2] = 1 + .5 - CurVert->ScrnXYZ[2];
																} // for VertCt

															for (RenderSide = 0; RenderSide < 2; RenderSide ++)
																{
																p0 = 0;
																p1 = 1;
																p2 = CurPoly->NumVerts - 1;
																poly = 0;
																while (p2 > p1)
																	{
																	// assign vertex pointers
																	Vtx[0] = &CurEffect->Vertices[CurPoly->VertRef[p0]];
																	Vtx[1] = &CurEffect->Vertices[CurPoly->VertRef[p1]];
																	Vtx[2] = &CurEffect->Vertices[CurPoly->VertRef[p2]];
																	Poly.VtxNum[0] = CurPoly->VertRef[p0];
																	Poly.VtxNum[1] = CurPoly->VertRef[p1];
																	Poly.VtxNum[2] = CurPoly->VertRef[p2];
																	Poly.VertRefData[0] = CurPoly->FindVertRefDataHead(Poly.VtxNum[0]);
																	Poly.VertRefData[1] = CurPoly->FindVertRefDataHead(Poly.VtxNum[1]);
																	Poly.VertRefData[2] = CurPoly->FindVertRefDataHead(Poly.VtxNum[2]);

																	Rend->RenderPolygon(&Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_3DOBJECT);

																	if (poly %2)
																		{
																		p0 = p2;
																		p2 --;
																		} // if
																	else
																		{
																		p0 = p1;
																		p1 ++;
																		} // else
																	poly ++;
																	} // while p2 > p1

																if (RenderSide)
																	break;

																// set up the second set of screen coordinates, the negative axis
																PlotAxisGroup ++;
																for (VertRefCt = 0; VertRefCt < CurPoly->NumVerts; VertRefCt ++)
																	{
																	VertCt = CurPoly->VertRef[VertRefCt];
																	CurVert = &CurEffect->Vertices[VertCt];
																	// copy screen coords
																	Index = PolyAxisGroup * CurEffect->NumVertices + VertCt;
																	if (StradlingVertices[Index] >= 0)
																		{
																		CurVert->ScrnXYZ[0] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[0];
																		CurVert->ScrnXYZ[1] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[1];
																		CurVert->ScrnXYZ[2] = Object3D->Vertices[StradlingVertices[Index]].ScrnXYZ[2];
																		} // if - failsafe
																	// apply the correct offsets
																	CurVert->ScrnXYZ[0] += ImageCenter[PlotAxisGroup][0];
																	CurVert->ScrnXYZ[1] = ImageHeight - (CurVert->ScrnXYZ[1] + ImageCenter[PlotAxisGroup][1]);
																	CurVert->ScrnXYZ[2] = 1 + .5 + CurVert->ScrnXYZ[2];
																	} // for VertCt

																} // for
															} // if not invisible

														ProcUpdate(PolyCt + 1);
														if (! IsRunning())
															{
															Success = 0;
															break;
															} // if
														} // for PolyCt
													ProcClear();

													// NULL this in case it causes problems, Renderer normally does not set this in the first place
													// but it is needed in order to render 3d objects to the Renderer's fragment map
													Rend->Rast->rPixelBlock = NULL;
													// save bitmap and alpha
													// create a UV texture in the material for diffuse color
													if (Success)
														{
														Rend->CollapseMap();
														// update thumbnail
														UpdateStamp();
														// check abort causes thumbnail refresh
														CheckAbort();
														if (IsRunning())
															{
															// turn off saving alpha channel if it might cause problems
															strcpy(BufBackup, IOE->OutBuffers[3]);
															if (! TransparencyExists || ExportFmt->ProhibitAlphaSave() || (ExportFmt->CautionAlphaSave() && ! CheckAlphaWorthwhile (Rend->AABuf, Rend->Width * Rend->Height)))
																IOE->OutBuffers[3][0] = 0;
															// save image
															if (Rend->Opt->SaveImage(Rend, NULL, Rend->Buffers, Rend->Width, Rend->Height, 0, 0, 0, 0, 0, 0, 0, 0,
																1, 1, 1, 1, 1, 1, 0, FALSE))
																{
																AddNewNameList(FileNamesCreated, (char *)ImagePAF.GetName(), WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX);

																// create a texture for the diffuse
																if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
																	{
																	if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
																		{
																		CurUVTex->SetVertexMap(Object3D->Name);
																		if (UVRaster = ImageHost->AddRaster((char *)ImagePAF.GetPath(), (char *)ImagePAF.GetName(), 0, 1, 1))
																			{
																			UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
																			CurUVTex->SetRaster(UVRaster);
																			CurJob->AddObjectToEphemeralList(UVRaster);
																			CurMat->PixelTexturesExist = true;
																			} // if
																		else
																			Success = 0;
																		} // if
																	} // if
																} // if
															else
																Success = 0;
															strcpy(IOE->OutBuffers[3], BufBackup);
															} // if
														else
															Success = 0;
														} // if
													} // if
												else
													Success = 0;

												// replace object pointers with new object
												if (Success)
													{
													for (CurInstance = CurJob->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
														{
														if (CurInstance->MyObj == CurEffect)
															CurInstance->MyObj = Object3D;
														} // for
													} // if
												} // if
											else
												Success = 0;
											} // if
										if (StradlingPolygons)
											AppMem_Free(StradlingPolygons, CurEffect->NumPolys * sizeof (char));
										if (StradlingVertices)
											AppMem_Free(StradlingVertices, 6 * CurEffect->NumVertices * sizeof (long));
										} // if
									} // if
								else
									Success = 0;
								if (UntexturedMaterials)
									{
									NewMatNum = 1;	// first material is the rendered texture
									PrevMapUsed = 255;
									// make a new material for each untextured material
									// walk the object and replace material in polygons as necessary
									for (MatCt = 0; MatCt < CurEffect->NumMaterials; MatCt ++)
										{
										if (CurMat = CurEffect->NameTable[MatCt].Mat)
											{
											NeedRenderedThis = 0;
											if (CurMat->AreThereEnabledTextures())
												{
												// there is at least one texture so may need to render
												// check to see if it is a UV texture
												for (TexNum = 0; ! NeedRenderedThis && TexNum < CurMat->GetNumTextures(); TexNum ++)
													{
													if (CurTexRoot = CurMat->GetTexRootPtr(TexNum))
														{
														if (CurTexRoot->Enabled)
															{
															// if it is a UV texture we have already rendered it and created a new UV map for this object.
															// otherwise if it is textured we have already rendered it.
															// By setting PrevmapUsed to 255 (a number that will likely never get hit) and telling it that
															// multiple UV maps are not supported, all UV maps and all textured surfaces should 
															// return TRUE
															// if textures are all disabled it returns FALSE
															if (CurTexRoot->IsNeedRendered(CurEffect, FALSE, PrevMapUsed))
																NeedRenderedThis = 1;
															} // if
														} // if
													} // for
												} // if
											} // if
										if (! NeedRenderedThis)
											{
											strcpy(Object3D->NameTable[NewMatNum].Name, CurEffect->NameTable[MatCt].Name);
											if (Object3D->NameTable[NewMatNum].Mat = new MaterialEffect(NULL))
												{
												CurJob->AddObjectToEphemeralList(Object3D->NameTable[NewMatNum].Mat);
												// assign name to material, set properties
												if (CurEffect->NameTable[MatCt].Mat)
													Object3D->NameTable[NewMatNum].Mat->Copy(Object3D->NameTable[NewMatNum].Mat, CurEffect->NameTable[MatCt].Mat);
												// replace polygon materials
												for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt ++)
													{
													if (CurEffect->Polygons[PolyCt].Material == MatCt)
														Object3D->Polygons[PolyCt].Material = NewMatNum;
													} // for
												} // if
											NewMatNum ++;
											} // if
										} // if
									} // for
								} // if
							else
								Success = 0;
							} // if object has textures and they need to be rendered
						// this object has been rendered, don't look for any more instances of it
						break;
						} // if
					} // for each object instance in instance list
				} // for each 3do
			} // if init frame
		ProcClear();
		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(true, false, TRUE, false);
		Rend->Opt->CleanupFromRender(TRUE);
		// steal this from renderer in case another job follows
		FrdWarned = Rend->GetFrdWarned();
		} // if
	else
		{
		Success = 0;
		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(false, false, FALSE, false);	// need to delete anything created during failed init process.
		Rend->Opt->CleanupFromRender(TRUE);
		ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
		ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
		} // else
	} // if renderer
else
	Success = 0;
#endif // WCS_BUILD_RTX

return (Success);

} // ExportControlGUI::RenderObjects

/*===========================================================================*/
/*
int ExportControlGUI::RenderObjects(SceneExporter *CurJob, RenderJob *CurRJ, NameList **FileNamesCreated)
{
double LongAxisLen, LongAxisCenter, ShortAxisCenter[2], ShortAxisWidth[2], VertCoord[3], ThisAngle, MaxAngle, OptimalWidth, OptimalHeight,
	WidthScale, HeightScale, MaxAngleDegrees, MaxShortAxisLen, CurTime;
long MatCt, TexNum, LongAxis, ShortAxis[2], NumPolyStradlers, NumVertStradlers, SumPolyVerts, UsedVertRefs, VertCt, 
	PolyCt, FoundNeg, FoundPos, ImageWidth, ImageHeight, VertNum, PolyNum, FirstFrame, poly, p0, p1, p2;
Object3DEffect *CurEffect, *Object3D;
MaterialEffect *CurMat, *PolyMat;
Object3DInstance *CurInstance;
RootTexture *CurTexRoot;
ObjectPerVertexMap *UVTable;
Polygon3D *CurPoly;
Vertex3D *CurVert, *Vtx[3];
ImageOutputEvent *IOE;
UVImageTexture *CurUVTex;
Raster *UVRaster;
struct ObjPolySort *PolySortArray;
long *StradlingVertices;
char *StradlingPolygons, *ImageExtension;
int NeedRendered, Success = 1;
char TempName[256], TempPreview;
VertexDEM Vert;
PolygonData Poly;
PathAndFile ImagePAF;

// well here goes - a method to export a 3d object complete with a texture map
// if the object already has UV textures and only one UVW map then use it as is

// only render objects that have textures

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);

if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	TempPreview = Preview;
	delete Rend;
	Rend = NULL;
	SetPreview(TempPreview);
	} // if

if (Rend = new Renderer())
	{
	//Rend->Exporter = CurJob;
	FirstFrame = (long)(STARTTIME * FRAMERATE + .5);
	CurTime = FirstFrame / FRAMERATE;
	#ifdef WCS_BUILD_DEMO
	if (FirstFrame > 150)
		{
		UserMessageDemo("Highest frame number you can render is 150.");
		if (FirstFrame > 150)
			FirstFrame = 150;
		} // if
	if (CurTime > 5.0)
		{
		UserMessageDemo("Highest time you can render is 5 seconds.");
		CurTime = 5.0;
		} // if
	#endif // WCS_BUILD_DEMO
	#ifdef WCS_RENDER_SCENARIOS
	// init enabled states at first frame to be rendered before init effects
	EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
	#endif // WCS_RENDER_SCENARIOS
	if (Rend->Init(CurRJ, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, TRUE))
		{
		SetFrameTextA("Rendering Object Textures");
		// bitmaps are now allocated which we won't be using

		// only complain once per render session
		Rend->SetFrdWarned(FrdWarned);

		// render shadows - steal some code from RenderFrame
		GetTime(Rend->StartSecs);
		GetTime(Rend->FrameSecs);

		// initialize everything for this frame
		if (Rend->InitFrame(CurTime, FirstFrame, TRUE))
			{
			// walk the 3d object list in EffectsLib
			for (CurEffect = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurEffect; 
				CurEffect = (Object3DEffect *)CurEffect->Next)
				{
				// compare objects there to objects in the 3d object export list
				for (CurInstance = CurJob->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
					{
					// if the same object is found then create a replacement object and for each instance in the list, fill in the replacement
					if (CurInstance->MyObj == CurEffect)
						{
						// we found a match, test for textures in each material
						NeedRendered = 0;
						for (MatCt = 0; ! NeedRendered && MatCt < CurEffect->NumMaterials; MatCt ++)
							{
							if ((CurMat = CurEffect->NameTable[MatCt].Mat) || 
								(CurMat = (CurEffect->NameTable[MatCt].Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CurEffect->NameTable[MatCt].Name))))
								{
								if (CurMat->AreThereEnabledTextures())
									{
									// there is at least one texture so may need to render
									// check to see if it is a UV texture
									for (TexNum = 0; ! NeedRendered && TexNum < CurMat->GetNumTextures(); TexNum ++)
										{
										if (CurTexRoot = CurMat->GetTexRootPtr(TexNum))
											{
											if (CurTexRoot->Enabled)
												{
												// if either textures are all disabled or it is a UV texture only, we don't need to render
												if (CurTexRoot->IsNeedRendered(CurEffect->NumUVWMaps, CurJob->MultipleObjectUVMappingsSupported()))
													NeedRendered = 1;
												} // if
											} // if
										} // for
									} // if
								} // if
							} // for 

						if (NeedRendered)
							{
							// has textures, process this object
							if (Object3D = new Object3DEffect)
								{
								CurJob->AddObjectToEphemeralList(Object3D);

								Object3D->Copy(Object3D, CurEffect);
								// delete material table
								Object3D->FreeDynamicStuff();
								// create new material table with one material
								if (Object3D->NameTable = new ObjectMaterialEntry[1])
									Object3D->NumMaterials = 1;
								// create single UVW table
								Object3D->AllocUVWMaps(1);
								UVTable = &Object3D->UVWTable[0];
								// create a material
								if (CurMat = new MaterialEffect(NULL))
									{
									CurJob->AddObjectToEphemeralList(CurMat);
									// assign name to material, set properties
									strcpy(CurMat->Name, Object3D->ShortName[0] ? Object3D->ShortName: Object3D->Name);
									CurMat->ShortName[0] = 0;
									CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
									// not burning in shading since object may be replicated with different orientations.
									//if (CurJob->BurnShading)
									//	CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
									CurMat->DoubleSided = 1;

									// assign a map name
									UVTable->SetName(Object3D->Name);

									// load the original object
									if (Object3D->NameTable && Object3D->UVWTable && 
										((CurEffect->Vertices && CurEffect->Polygons && CurEffect->NameTable) || CurEffect->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
										{
										strcpy(Object3D->NameTable[0].Name, CurMat->Name);
										Object3D->NameTable[0].Mat = CurMat;
										StradlingPolygons = NULL;
										StradlingVertices = NULL;
										if ((StradlingPolygons = (char *)AppMem_Alloc(CurEffect->NumPolys * sizeof (char), APPMEM_CLEAR))
											&& (StradlingVertices = (long *)AppMem_Alloc(CurEffect->NumVertices * sizeof (long), APPMEM_CLEAR)))
											{
											// might need to create some duplicate vertices
											// assume seam at +z axis
											// find longest axis
											if (fabs(CurEffect->ObjectBounds[0] - CurEffect->ObjectBounds[1]) > fabs(CurEffect->ObjectBounds[2] - CurEffect->ObjectBounds[3]))
												{
												LongAxis = 0;
												LongAxisLen = fabs(CurEffect->ObjectBounds[0] - CurEffect->ObjectBounds[1]);
												LongAxisCenter = (CurEffect->ObjectBounds[0] + CurEffect->ObjectBounds[1]) * .5;
												} // if
											else
												{
												LongAxis = 1;
												LongAxisLen = fabs(CurEffect->ObjectBounds[2] - CurEffect->ObjectBounds[3]);
												LongAxisCenter = (CurEffect->ObjectBounds[2] + CurEffect->ObjectBounds[3]) * .5;
												} // else
											if (fabs(CurEffect->ObjectBounds[4] - CurEffect->ObjectBounds[5]) > LongAxisLen)
												{
												LongAxisCenter = (CurEffect->ObjectBounds[4] + CurEffect->ObjectBounds[5]) * .5;
												LongAxis = 2;
												LongAxisLen = fabs(CurEffect->ObjectBounds[4] - CurEffect->ObjectBounds[5]);
												} // if
											// find centroid of other two axes
											// if x is long axis
											if (LongAxis == 0)
												{
												ShortAxis[0] = 2;
												ShortAxisCenter[0] = (CurEffect->ObjectBounds[4] + CurEffect->ObjectBounds[5]) * .5;
												ShortAxisWidth[0] = fabs(CurEffect->ObjectBounds[4] - CurEffect->ObjectBounds[5]);
												ShortAxis[1] = 1;
												ShortAxisCenter[1] = (CurEffect->ObjectBounds[2] + CurEffect->ObjectBounds[3]) * .5;
												ShortAxisWidth[1] = fabs(CurEffect->ObjectBounds[2] - CurEffect->ObjectBounds[3]);
												} // if
											// if y is long axis
											if (LongAxis == 1)
												{
												ShortAxis[0] = 0;
												ShortAxisCenter[0] = (CurEffect->ObjectBounds[0] + CurEffect->ObjectBounds[1]) * .5;
												ShortAxisWidth[0] = fabs(CurEffect->ObjectBounds[0] - CurEffect->ObjectBounds[1]);
												ShortAxis[1] = 2;
												ShortAxisCenter[1] = (CurEffect->ObjectBounds[4] + CurEffect->ObjectBounds[5]) * .5;
												ShortAxisWidth[1] = fabs(CurEffect->ObjectBounds[4] - CurEffect->ObjectBounds[5]);
												} // if
											// if z is long axis
											if (LongAxis == 2)
												{
												ShortAxis[0] = 1;
												ShortAxisCenter[0] = (CurEffect->ObjectBounds[2] + CurEffect->ObjectBounds[3]) * .5;
												ShortAxisWidth[0] = fabs(CurEffect->ObjectBounds[2] - CurEffect->ObjectBounds[3]);
												ShortAxis[1] = 0;
												ShortAxisCenter[1] = (CurEffect->ObjectBounds[0] + CurEffect->ObjectBounds[1]) * .5;
												ShortAxisWidth[1] = fabs(CurEffect->ObjectBounds[0] - CurEffect->ObjectBounds[1]);
												} // if
											MaxShortAxisLen = max(ShortAxisWidth[0], ShortAxisWidth[1]);
											// find angle of all vertices from centroid to vertex
											// determine which polygons have vertices in both +x and -x space and +z
											// create a list of the affected polygons
											MaxAngle = 0.0;
											NumPolyStradlers = 0;
											SumPolyVerts = 0;
											for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
												{
												CurPoly = &CurEffect->Polygons[PolyCt];
												SumPolyVerts += CurPoly->NumVerts;
												FoundNeg = FoundPos = 0;
												for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
													{
													CurVert = &CurEffect->Vertices[CurPoly->VertRef[VertCt]];
													// find rotation angle of vertex around the principal axis
													VertCoord[0] = CurVert->xyz[ShortAxis[0]] - ShortAxisCenter[0];
													VertCoord[1] = CurVert->xyz[ShortAxis[1]] - ShortAxisCenter[1];
													if (VertCoord[1] > 0.0)
														{
														// we found a point in negative texture space
														if (VertCoord[0] < 0.0)
															{
															FoundNeg = 1;
															// how large is the angle of rotation from the seam axis
															ThisAngle = PiUnder180 * findangle3(VertCoord[1], VertCoord[0]);
															if (ThisAngle > 180.0)
																ThisAngle -= 360.0;
															if (ThisAngle < 0.0)
																{
																ThisAngle = -ThisAngle;
																if (ThisAngle > MaxAngle)
																	MaxAngle = ThisAngle;
																} // if
															} // if
														else if (VertCoord[0] > 0.0)
															FoundPos = 1;
														} // if
													} // for
												if (FoundNeg && FoundPos)
													{
													// add this polygon to a list of affected polygons
													StradlingPolygons[PolyCt] = 1;
													} // if
												} // for
											// create a list of such vertices that are in -x, +z
											// count the list, these are the extra vertices needed
											NumVertStradlers = 0;
											for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
												{
												if (StradlingPolygons[PolyCt])
													{
													CurPoly = &CurEffect->Polygons[PolyCt];
													for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
														{
														CurVert = &CurEffect->Vertices[CurPoly->VertRef[VertCt]];
														// find rotation angle of vertex around the principal axis
														VertCoord[0] = CurVert->xyz[ShortAxis[0]] - ShortAxisCenter[0];
														VertCoord[1] = CurVert->xyz[ShortAxis[1]] - ShortAxisCenter[1];
														if (VertCoord[1] > 0.0 && VertCoord[0] < 0.0)
															{
															if (StradlingVertices[CurPoly->VertRef[VertCt]] == 0)
																StradlingVertices[CurPoly->VertRef[VertCt]] = CurEffect->NumVertices + NumVertStradlers ++;
															} // if
														} // for
													} // if
												} // for
											// allocate vertices and polygons including any duplicates
											Object3D->NumVertices = CurEffect->NumVertices + NumVertStradlers;
											Object3D->NumPolys = CurEffect->NumPolys;
											Object3D->Vertices = new Vertex3D[Object3D->NumVertices];
											Object3D->Polygons = new Polygon3D[Object3D->NumPolys];
											Object3D->AllocVertRef(SumPolyVerts);
											// allocate UV table with number of vertices
											UVTable->AllocMap(Object3D->NumVertices);

											if (UVTable->MapsValid() && Object3D->Vertices && Object3D->Polygons && Object3D->VertRefBlock)
												{
												Object3D->VertexUVWAvailable = 1;
												// copy the vertices and polygons
												UsedVertRefs = 0;
												for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
													{
													CurPoly = &Object3D->Polygons[PolyCt];
													// copy CurEffect->Polygons[PolyCt] to Object3D->Polygons[PolyCt]
													CurPoly->Copy(CurPoly, &CurEffect->Polygons[PolyCt]);
													CurPoly->Material = 0;
													CurPoly->VertRef = &Object3D->VertRefBlock[UsedVertRefs];
													UsedVertRefs += CurPoly->NumVerts;
													for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
														{
														CurPoly->VertRef[VertCt] = CurEffect->Polygons[PolyCt].VertRef[VertCt];
														} // for
													} // for
												for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
													{
													// copy CurEffect->Vertices[VertCt] to Object3D->Vertices[VertCt]
													Object3D->Vertices[VertCt].Copy(&Object3D->Vertices[VertCt], &CurEffect->Vertices[VertCt]);
													} // for
												// duplicate the vertices in the -x, +z region
												for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
													{
													if (StradlingVertices[VertCt] > 0)
														{
														// copy CurEffect->Vertices[VertCt] to Object3D->Vertices[StradlingVertices[VertCt]]
														Object3D->Vertices[StradlingVertices[VertCt]].Copy(&Object3D->Vertices[StradlingVertices[VertCt]], &CurEffect->Vertices[VertCt]);
														} // if
													} // for
												// change polygon references to the new vertices as needed
												for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
													{
													if (StradlingPolygons[PolyCt])
														{
														CurPoly = &Object3D->Polygons[PolyCt];
														for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
															{
															CurVert = &Object3D->Vertices[CurPoly->VertRef[VertCt]];
															// find rotation angle of vertex around the principal axis
															VertCoord[0] = CurVert->xyz[ShortAxis[0]] - ShortAxisCenter[0];
															VertCoord[1] = CurVert->xyz[ShortAxis[1]] - ShortAxisCenter[1];
															if (VertCoord[1] > 0.0 && VertCoord[0] < 0.0)
																{
																CurPoly->VertRef[VertCt] = StradlingVertices[CurPoly->VertRef[VertCt]];
																} // if
															} // for
														} // if
													} // for

												// compute the bitmap size for the rendered texture image
												// scale largest side
												// use same scale to scale smaller side
												OptimalWidth = ((360.0 + MaxAngle) / 360.0) * Pi * MaxShortAxisLen;
												OptimalHeight = LongAxisLen;
												if (OptimalWidth > OptimalHeight)
													{
													ImageWidth = (long)(OptimalWidth / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].CurValue);
													if (ImageWidth > CurJob->Max3DOTexSize)
														ImageWidth = CurJob->Max3DOTexSize;
													WidthScale = ImageWidth / OptimalWidth;
													ImageHeight = (long)(WidthScale * OptimalHeight);
													HeightScale = ImageHeight / OptimalHeight;
													} // if
												else 
													{
													ImageHeight = (long)(OptimalHeight / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].CurValue);
													if (ImageHeight > CurJob->Max3DOTexSize)
														ImageHeight = CurJob->Max3DOTexSize;
													HeightScale = ImageHeight / OptimalHeight;
													ImageWidth = (long)(HeightScale * OptimalWidth);
													WidthScale = ImageWidth / OptimalWidth;
													} // if
												// compute and assign the UV coords of each vertex
												// we will map each vertex into UV space by projecting its coordinates onto an ellipsoid
												// Ellipsoid has a semimajor axis of OptimalHeight and semiminor axis of OptimalWidth
												MaxAngleDegrees = MaxAngle + 360.0;
												// treat dulpicated vertices as 360 degrees +
												for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt ++)
													{
													// project coordinates
													CurVert = &Object3D->Vertices[VertCt];
													Vert.XYZ[0] = CurVert->xyz[ShortAxis[0]] - ShortAxisCenter[0];
													Vert.XYZ[2] = CurVert->xyz[ShortAxis[1]] - ShortAxisCenter[1];
													Vert.XYZ[1] = CurVert->xyz[LongAxis] - LongAxisCenter;

													Vert.Lon = PiUnder180 * findangle3(Vert.XYZ[2], Vert.XYZ[0]);
													if (Vert.Lon < 0.0)
														Vert.Lon += 360.0;
													Vert.Lon = 360.0 - Vert.Lon;
													Vert.Lat = 180.0 * Vert.XYZ[1] / LongAxisLen;

													//Vert.CartToDeg(LongAxisLen, MaxShortAxisLen);
													//if (Vert.Lon < 0.0)
													//	Vert.Lon += 360.0;
													//Vert.Lon = 180.0 - Vert.Lon;

													if (VertCt >= CurEffect->NumVertices)
														Vert.Lon += 360.0;
													CurVert->ScrnXYZ[0] = Vert.Lon / MaxAngleDegrees;
													if (CurVert->ScrnXYZ[0] > 1.0)
														CurVert->ScrnXYZ[0] = 1.0;	// catch any overflow, don't expect any but roundoff in degree/radian conversion...
													CurVert->ScrnXYZ[1] = 1.0 - (Vert.Lat + 90.0) / 180.0;
													// use vertex distance from surrounding cylinder to determine drawing order
													CurVert->ScrnXYZ[2] = MaxShortAxisLen - sqrt(Vert.XYZ[0] * Vert.XYZ[0] + Vert.XYZ[2] * Vert.XYZ[2]);
													UVTable->CoordsValid[VertCt] = 1;
													UVTable->CoordsArray[0][VertCt] = (float)CurVert->ScrnXYZ[0];
													UVTable->CoordsArray[1][VertCt] = (float)(1.0 - CurVert->ScrnXYZ[1]);
													CurVert->ScrnXYZ[0] *= ImageWidth;
													CurVert->ScrnXYZ[1] *= ImageHeight;
													} // for

												// create an array of polygon pointers and sort them according to smallest to largest ScrnXYZ[2]
												if (PolySortArray = (struct ObjPolySort *)AppMem_Alloc(CurEffect->NumPolys * sizeof (struct ObjPolySort), 0))
													{
													// fill in sort data
													for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
														{
														PolySortArray[PolyCt].PolyNum = PolyCt;
														PolySortArray[PolyCt].Poly = &CurEffect->Polygons[PolyCt];
														PolySortArray[PolyCt].AxisDist = (float)MaxShortAxisLen;
														for (VertCt = 0; VertCt < CurEffect->NumVertices; VertCt ++)
															{
															// find smallest distance from axis using axis distances stored in Object3D vertices
															if ((float)Object3D->Vertices[VertCt].ScrnXYZ[2] < PolySortArray[PolyCt].AxisDist)
																PolySortArray[PolyCt].AxisDist = (float)CurVert->ScrnXYZ[2];
															} // for
														} // for
													// sort polygons
													qsort((void *)PolySortArray, (size_t)CurEffect->NumPolys, (size_t)(sizeof (struct ObjPolySort)), CompareObjPolySort);
													// create a Renderer or whatever, alloc bitmaps or clear bitmaps
													// configure output
													ImagePAF.Copy(&ImagePAF, &CurJob->OutPath);
													strcpy(TempName, CurMat->Name);
													if (ImageExtension)
														AddExtension(TempName, ImageExtension);
													ImagePAF.SetName(TempName);

													Rend->Opt->OutputImageWidth = ImageWidth;
													Rend->Opt->OutputImageHeight = ImageHeight;

													IOE = Rend->Opt->OutputEvents;
													IOE->PAF.Copy(&IOE->PAF, &ImagePAF);
													IOE->AutoExtension = 0;
													IOE->AutoDigits = 0;

													// alloc new rasters
													if (Rend->SetupAndAllocBitmaps(ImageWidth, ImageHeight))
														{
														ProcInit(CurEffect->NumPolys, "Rendering 3D Object Texture");
														//Rend->Rast->rPixelBlock = Rend->rPixelBlock;
														Poly.Object = CurEffect;
														//Poly.Plot3DRast = Rend->Rast;
														Poly.BaseMat.Covg[0] = 1.0;
														Poly.ShadowFlags = 0;
														Poly.ReceivedShadowIntensity = 0.0;
														Poly.PlotOffset3DX = Poly.PlotOffset3DY = 0;
														Poly.RenderPassWeight = 1.0;
														// render each polygon of the original object into a bitmap
														ProcInit(CurEffect->NumPolys, "Rendering 3D Object Texture");
														for (PolyCt = 0; PolyCt < CurEffect->NumPolys; PolyCt ++)
															{
															CurPoly = PolySortArray[PolyCt].Poly;
															PolyNum = PolySortArray[PolyCt].PolyNum;
															for (VertCt = 0; VertCt < CurPoly->NumVerts; VertCt ++)
																{
																VertNum = CurPoly->VertRef[VertCt];
																CurVert = &CurEffect->Vertices[VertNum];
																CurVert->XYZ[0] = CurVert->xyz[0];
																CurVert->XYZ[1] = CurVert->xyz[1];
																CurVert->XYZ[2] = CurVert->xyz[2];
																// copy screen coords
																// if it is a straddling polygon, use the wrapped coords if a duplicate vertex exists
																if (StradlingPolygons[PolyNum] && StradlingVertices[VertNum] > 0)
																	{
																	CurVert->ScrnXYZ[0] = Object3D->Vertices[StradlingVertices[VertNum]].ScrnXYZ[0];
																	CurVert->ScrnXYZ[1] = Object3D->Vertices[StradlingVertices[VertNum]].ScrnXYZ[1];
																	CurVert->ScrnXYZ[2] = Object3D->Vertices[StradlingVertices[VertNum]].ScrnXYZ[2];
																	//CurVert->ScrnXYZ[2] = 1.0;
																	} // if
																else
																	{
																	CurVert->ScrnXYZ[0] = Object3D->Vertices[VertNum].ScrnXYZ[0];
																	CurVert->ScrnXYZ[1] = Object3D->Vertices[VertNum].ScrnXYZ[1];
																	CurVert->ScrnXYZ[2] = Object3D->Vertices[VertNum].ScrnXYZ[2];
																	//CurVert->ScrnXYZ[2] = 1.0;
																	} // else
																} // for VertCt

															// transform vertices
															// cart to deg vertices
															// skip invisible poly
															PolyMat = CurEffect->NameTable[CurEffect->Polygons[PolyNum].Material].Mat;
															if (PolyMat->Shading != WCS_EFFECT_MATERIAL_SHADING_INVISIBLE && PolyMat->Enabled)
																{
																// set polygon normalized flag = 0
																CurPoly->Normalized = 0;
																// set basemat material
																Poly.BaseMat.Mat[0] = PolyMat;
																// normalize polygon
																CurPoly->Normalize(CurEffect->Vertices);
																// flip normals if necessary
																if (PolyMat->FlipNormal)
																	NegateVector(CurPoly->Normal);
																// copy normals to PolygonData
																Poly.Normal[0] = CurPoly->Normal[0];
																Poly.Normal[1] = CurPoly->Normal[1];
																Poly.Normal[2] = CurPoly->Normal[2];
																// calc vertex normals
																Poly.ShadeType = CurEffect->CalcNormals ? (char)PolyMat->Shading: WCS_EFFECT_MATERIAL_SHADING_PHONG;
																if (Poly.ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG && CurEffect->CalcNormals)
																	{
																	for (p0 = 0; p0 < CurPoly->NumVerts; p0 ++)
																		{
																		CurEffect->Vertices[CurPoly->VertRef[p0]].Normalized = 0;
																		CurEffect->Vertices[CurPoly->VertRef[p0]].Normalize(CurEffect->Polygons, CurEffect->Vertices, CurPoly, CurEffect->NameTable);
																		} // for
																	} // if phong

																p0 = 0;
																p1 = 1;
																p2 = CurPoly->NumVerts - 1;
																poly = 0;
																while (p2 > p1)
																	{
																	// assign vertex pointers
																	Vtx[0] = &CurEffect->Vertices[CurPoly->VertRef[p0]];
																	Vtx[1] = &CurEffect->Vertices[CurPoly->VertRef[p1]];
																	Vtx[2] = &CurEffect->Vertices[CurPoly->VertRef[p2]];
																	Poly.VtxNum[0] = CurPoly->VertRef[p0];
																	Poly.VtxNum[1] = CurPoly->VertRef[p1];
																	Poly.VtxNum[2] = CurPoly->VertRef[p2];
																	Poly.VertRefData[0] = CurPoly->FindVertRefDataHead(Poly.VtxNum[0]);
																	Poly.VertRefData[1] = CurPoly->FindVertRefDataHead(Poly.VtxNum[1]);
																	Poly.VertRefData[2] = CurPoly->FindVertRefDataHead(Poly.VtxNum[2]);

																	Rend->RenderPolygon(&Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_3DOBJECT);

																	if (poly %2)
																		{
																		p0 = p2;
																		p2 --;
																		} // if
																	else
																		{
																		p0 = p1;
																		p1 ++;
																		} // else
																	poly ++;
																	} // while p2 > p1
																} // if not invisible

															ProcUpdate(PolyCt + 1);
															if (! IsRunning())
																{
																Success = 0;
																break;
																} // if
															} // for PolyCt
														ProcClear();

														// NULL this in case it causes problems, Renderer normally does not set this in the first place
														// but it is needed in order to render 3d objects to the Renderer's fragment map
														Rend->Rast->rPixelBlock = NULL;
														// save bitmap and alpha
														// create a UV texture in the material for diffuse color
														if (Success)
															{
															Rend->CollapseMap();
															if (Rend->Opt->SaveImage(Rend, NULL, Rend->Buffers, Rend->Width, Rend->Height, 0, 0, 0, 0, 0, 0, 0, 0,
																1, 1, 1, 1, 1, 1, 0, FALSE))
																{
																AddNewNameList(FileNamesCreated, (char *)ImagePAF.GetName(), WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX);

																// create a texture for the diffuse
																if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
																	{
																	if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
																		{
																		CurUVTex->SetVertexMap(Object3D->Name);
																		if (UVRaster = ImageHost->AddRaster((char *)ImagePAF.GetPath(), (char *)ImagePAF.GetName(), 0, 1, 1))
																			{
																			UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
																			CurUVTex->SetRaster(UVRaster);
																			CurJob->AddObjectToEphemeralList(UVRaster);
																			} // if
																		else
																			Success = 0;
																		} // if
																	} // if
																} // if
															else
																{
																Success = 0;
																} // if
															} // if
														} // if
													else
														Success = 0;
													AppMem_Free(PolySortArray, CurEffect->NumPolys * sizeof (struct ObjPolySort));
													} // if
												else
													Success = 0;

												// replace object pointers with new object
												if (Success)
													{
													for (CurInstance = CurJob->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
														{
														if (CurInstance->MyObj == CurEffect)
															CurInstance->MyObj = Object3D;
														} // for
													} // if
												} // if
											else
												Success = 0;
											} // if
										if (StradlingPolygons)
											AppMem_Free(StradlingPolygons, CurEffect->NumPolys * sizeof (char));
										if (StradlingVertices)
											AppMem_Free(StradlingVertices, CurEffect->NumVertices * sizeof (long));
										} // if
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if object has textures and they need to be rendered
						break;
						} // if
					} // for
				} // for each 3do
			} // if init frame
		ProcClear();
		#ifdef WCS_RENDER_SCENARIOS
		EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(TRUE, FALSE, TRUE, FALSE);
		Rend->Opt->CleanupFromRender(TRUE);
		// steal this from renderer in case another job follows
		FrdWarned = Rend->GetFrdWarned();
		} // if
	else
		{
		Success = 0;
		#ifdef WCS_RENDER_SCENARIOS
		EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(FALSE, FALSE, FALSE, FALSE);	// need to delete anything created during failed init process.
		Rend->Opt->CleanupFromRender(TRUE);
		ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
		ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
		} // else
	} // if renderer
else
	Success = 0;

return (Success);

} // ExportControlGUI::RenderObjects
*/
/*===========================================================================*/

// prepares to export walls
int ExportControlGUI::ExportWalls(SceneExporter *CurJob, NameList **FileNamesCreated)
{
int Success = 1;
#ifdef WCS_BUILD_RTX
Camera *CurCam;
RenderOpt *CurOpt;
RenderJob *CurRJ;
ImageOutputEvent *CurEvent;
char *DefBuf;
int Ct;

SetFrameTextA("Exporting Walls");

if (! CurJob->WallAsObj)
	return (1);

// are there any walls?
if (! EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_FENCE))
	return (1);

// Best to try to do one wall at a time, using a similar process to what the Renderer does but controlling the flow
// so that one bitmap at a time is created rather than rendering the wall segments in Z-sorted order.

// A Renderer would perhaps make life easier but ViewGUI doesn't use one so it probably isn't necessary.
// How to do texture sampling and image plotting without using RenderPoly is perhaps less pleasant.


// How 'bout we create and init a Renderer with everything turned off except terraffectors and area tfx and walls.
// The list of fence parts is sorted by the fencs component and vector at that stage.
// We can walk that list, checking for either a vector or effect change which triggers new calculations and saves previous.

// create a camera, render opt and render job
if ((CurCam = new Camera) && (CurOpt = new RenderOpt) && (CurRJ = new RenderJob))
	{
	// disable stuff
	CurOpt->OutputImageWidth = CurOpt->OutputImageHeight = CurJob->MaxWallTexSize;	// temporary
	CurOpt->ReflectionsEnabled = 0;
	CurOpt->CloudShadowsEnabled = 
		CurOpt->ObjectShadowsEnabled = 
		CurOpt->TerrainEnabled = 
		CurOpt->FoliageEnabled = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = (CurJob->BurnWallShading);
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = CurJob->BurnWallShading;
	CurOpt->RenderDiagnosticData = CurOpt->MultiPassAAEnabled = CurOpt->DepthOfFieldEnabled = 0;
	CurOpt->TerrainEnabled = 0;
	CurOpt->VectorsEnabled = 0;
	CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] =  
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] = 
		CurOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 0;
	CurOpt->FragmentRenderingEnabled = 1;
	CurOpt->FragmentDepth = 2;
	CurOpt->VolumetricsEnabled = 0;
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
	CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->TempPath);

	CurJob->FragmentCollapseType = WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST;

	// set up an output event
	if (CurEvent = CurOpt->AddOutputEvent())
		{
		// set new file type
		strcpy(CurEvent->FileType, CurJob->TextureImageFormat);

		for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
			{
			CurEvent->OutBuffers[Ct][0] = 0;
			CurEvent->BufNodes[Ct] = NULL;
			} // for
		if (DefBuf = ImageSaverLibrary::GetNextCodec(CurEvent->FileType, NULL))
			strcpy(CurEvent->Codec, DefBuf);
		// set new values for buffers and codec
		strcpy(CurEvent->OutBuffers[0], "RED");
		strcpy(CurEvent->OutBuffers[1], "GREEN");
		strcpy(CurEvent->OutBuffers[2], "BLUE");
		strcpy(CurEvent->OutBuffers[3], "ANTIALIAS");
		} // if

	// render the panorama
	CurRJ->SetCamera(CurCam);
	CurRJ->SetRenderOpt(CurOpt);
	// no copying so be careful not to mess these scenarios up
	CurRJ->Scenarios = CurJob->Scenarios;
	CurCam->InitToRender(CurOpt, NULL);
	((GeneralEffect *)CurOpt)->InitToRender(CurOpt, NULL);
	CurRJ->InitToRender(CurOpt, NULL);

	// render the image so that all tiles are rendered and concatenated
	Success = RenderWalls(CurJob, CurRJ, FileNamesCreated);

	CurRJ->Scenarios = NULL;
	delete CurRJ;
	delete CurCam;
	delete CurOpt;
	} // if

FrameTextClearA();
#endif // WCS_BUILD_RTX
return (Success);

} // ExportControlGUI::ExportWalls

/*===========================================================================*/

// creates geometry for walls
int ExportControlGUI::RenderWalls(SceneExporter *CurJob, RenderJob *CurRJ, NameList **FileNamesCreated)
{
int Success = 0;
#ifdef WCS_BUILD_RTX
int MakeFloor;
double Area, CurTime, VecCenterNS, VecCenterEW, TopElev, BotElev, Temp1, Temp2, SegLen, RefElev;
#ifndef WCS_FENCE_LIMITED
double RoofElev;
#endif // !WCS_FENCE_LIMITED
long FirstFrame, NumWallSegs, NumRoofSegs, NumElementsThisObject, CurVert, CurPoly, StartVert, NumObjVerts, NumPolys, 
	NumObjPolys, NumRoofPolys, VertRefBlockIdx, WallObjNumber = 0, WallMatNum = 0;
unsigned long StashCurSteps, StashMaxSteps, PolyCt, TestCt, MainPolyCt, Flags;
time_t StashStartSecs;
Renderer *ShadowRend, *BackupRend;
FenceVertexList *ListElement, *TestElement, *FirstWallElement, *FirstRoofElement;
Joe *CurVec;
Fence *CurFence;
struct TerrainPolygonSort *PolyArray;
Object3DEffect *Object3D;
JoeCoordSys *MyAttr;
CoordSys *VecCoords;
GradientCritter *GradCrit;
Object3DInstance *WallObjectList = NULL, **InstancePtr, *CurInstance;
MaterialEffect *SourceMat;
char TempPreview, StashText[200];
VertexData Vtx[4];
PolygonData Poly;
int SpanMatNum, RoofMatNum;
#ifndef WCS_FENCE_LIMITED
double Angle, NorthMost, SouthMost, WestMost, EastMost;
int Clockwise, Legal, BreakToOuterLoop;
long NumRoofPts, FirstCt, NextCt, NextNextCt, NumToTest, Illegal;
VectorPoint *PLink;
VectorPoint **RoofPts = NULL, *PLinkPrev;
#endif // WCS_FENCE_LIMITED
#ifdef WCS_THEMATIC_MAP
double ThemeValue[3];
#endif // WCS_THEMATIC_MAP

InstancePtr = &WallObjectList;

#ifdef WCS_THEMATIC_MAP
ThemeValue[0] = ThemeValue[1] = ThemeValue[2] = 0.0;
#endif // WCS_THEMATIC_MAP

if (Rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	TempPreview = Preview;
	delete Rend;
	Rend = NULL;
	SetPreview(TempPreview);
	} // if

if (Rend = new Renderer())
	{
	//Rend->Exporter = CurJob;
	FirstFrame = (long)(STARTTIME * FRAMERATE + .5);
	CurTime = FirstFrame / FRAMERATE;
	#ifdef WCS_BUILD_DEMO
	if (FirstFrame > 150)
		{
		UserMessageDemo("Highest frame number you can render is 150.");
		if (FirstFrame > 150)
			FirstFrame = 150;
		} // if
	if (CurTime > 5.0)
		{
		UserMessageDemo("Highest time you can render is 5 seconds.");
		CurTime = 5.0;
		} // if
	#endif // WCS_BUILD_DEMO
	//#ifdef WCS_RENDER_SCENARIOS
	// init enabled states at first frame to be rendered before init effects
	//EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
	//#endif // WCS_RENDER_SCENARIOS
	if (Rend->Init(CurRJ, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, TRUE))
		{
		SetFrameTextA("Exporting Walls");
		// bitmaps are now allocated which we won't be using

		// only complain once per render session
		Rend->SetFrdWarned(FrdWarned);

		// render shadows - steal some code from RenderFrame
		GetTime(Rend->StartSecs);
		GetTime(Rend->FrameSecs);

		// initialize everything for this frame
		if (Rend->InitFrame(CurTime, FirstFrame, TRUE))
			{
			// Shadows
			if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] && 
				((Rend->Opt->ObjectShadowsEnabled  && Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D]) || 
				(Rend->Opt->CloudShadowsEnabled && Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD]) || 
				(Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && Rend->Opt->TerrainEnabled)))
				{
				if (ShadowRend = new Renderer())
					{
					BackupRend = Rend;
					GetFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, StashText);
					#ifdef WCS_VECPOLY_EFFECTS
					Success = ShadowRend->InitForShadows(Rend->AppBase, Rend->EffectsBase, Rend->ImageBase, 
						Rend->DBase, Rend->ProjectBase, Rend->LocalLog, this, Rend->Opt, Rend, FirstFrame, CurTime);
					#else // WCS_VECPOLY_EFFECTS
					Success = ShadowRend->InitForShadows(Rend->AppBase, Rend->EffectsBase, Rend->ImageBase, 
						Rend->DBase, Rend->ProjectBase, Rend->LocalLog, this, Rend->Opt, Rend, FirstFrame);
					#endif // WCS_VECPOLY_EFFECTS
					SetRenderer(BackupRend);
					SetPreview(Rend->Pane ? 1: 0);
					if (strlen(StashText) > 1)  //lint !e645
						{
						StashText[strlen(StashText) - 1] = 0;
						RestoreFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, &StashText[1]);
						} // if
					delete ShadowRend;
					// restore foliage, fence and 3d object vertex non-rendered status
					if (Success)
						Rend->EffectsBase->InitBaseToRender();
					} // if
				} // if
			else
				Success = 1;

			if (Success)
				{
				// render each wall one at a time
				// compute the number of possible foliage and 3D object and fence vertices
				if ((NumPolys = Rend->EffectsBase->CountRenderItemVertices(&Rend->RendData, Rend->DBase)) > 0)
					{
					// create the array of a size large enough to hold all the vertices
					if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort), 0))
						{
						PolyCt = 0;	// this will count the polygons as they are inserted into the array
						Rend->EffectsBase->FillRenderPolyArray(&Rend->RendData, PolyArray, PolyCt, CurJob->Coords, &CurJob->GeoReg);
						if (PolyCt > 0)
							{
							// render the polygons
							FrameGaugeInit(PolyCt);

							for (MainPolyCt = 0; MainPolyCt < PolyCt && Success; )	// Ct is incremented in busy-win update section
								{
								if (PolyArray[MainPolyCt].PolyType == WCS_POLYSORTTYPE_FENCE)
									{
									// render this fence and vector combination 
									ListElement = (FenceVertexList *)PolyArray[MainPolyCt].PolyNumber;
									CurVec = ListElement->Vec;
									CurFence = ListElement->Fnce;
									FirstWallElement = FirstRoofElement = NULL;
									RefElev = FLT_MAX;

									// free the existing bitmaps if they exist
									// compute the extents of the fence in length and height
									// find the ultimate length by using the Joe MinDistToPoint function on the last vertex
									// if the output is tiled then we only need to generate a short patch of texture
									// if the material has no texture on either color, diffuse intensity, transparency or bump
									// then we have only to create a small image of a constant color.

									// set the image size
									// allocate bitmaps for this wall
									// render all of the list entities that have the same wall and vector

									// count number of polygons
									NumWallSegs = NumRoofSegs = NumElementsThisObject = 0;
									NumRoofPolys = 0;
									MakeFloor = 0;
									// walk through remaining list of elements
									for (TestCt = MainPolyCt; TestCt < PolyCt; TestCt ++)
										{
										TestElement = (FenceVertexList *)PolyArray[TestCt].PolyNumber;
										if (CurVec != TestElement->Vec || CurFence != TestElement->Fnce)
											break;
										// if wall element
										if (TestElement->PieceType == WCS_FENCEPIECE_SPAN)
											{
											NumWallSegs ++;
											if (! FirstWallElement)
												FirstWallElement = TestElement;
											} // if
										else if (TestElement->PieceType == WCS_FENCEPIECE_ROOF)
											{
											NumRoofSegs ++;
											if (! FirstRoofElement)
												FirstRoofElement = TestElement;
											} // if
										NumElementsThisObject ++;
										} // for
									// if wall element
										// add 2 polygons, 4 vertices
									// if roof
										// add number of vector vertices - 1
									Area = CurVec->ComputeAreaDegrees();
									#ifndef WCS_FENCE_LIMITED
									if (NumRoofSegs > 0)
										{
										PLinkPrev = NULL;
										for (NumRoofPts = 0, PLink = CurVec->GetFirstRealPoint(); PLink; PLink = PLink->Next)
											{
											if (PLinkPrev && PLink->SamePoint(PLinkPrev))
												continue;
											if (! PLink->Next && PLink->SamePoint(CurVec->GetFirstRealPoint()))
												continue;
											NumRoofPts ++;
											PLinkPrev = PLink;
											} // for
										//NumRoofPts = ListElement->Vec->GetNumRealPoints();
										if (NumRoofPts >= 3 && (RoofPts = (VectorPoint **)AppMem_Alloc(NumRoofPts * sizeof (VectorPoint *), APPMEM_CLEAR)))
											{
											// find out if vector is clockwise or countercw
											Clockwise = (Area >= 0.0);
											// build list of vector points
											PLinkPrev = NULL;
											if (Clockwise)
												{
												for (TestCt = 0, PLink = CurVec->GetFirstRealPoint(); (long)TestCt < NumRoofPts && PLink; PLink = PLink->Next)
													{
													if (PLinkPrev && PLink->SamePoint(PLinkPrev))
														continue;
													if (! PLink->Next && PLink->SamePoint(CurVec->GetFirstRealPoint()))
														continue;
													RoofPts[TestCt ++] = PLink;
													PLinkPrev = PLink;
													} // for
												} // if
											else
												{
												for (TestCt = NumRoofPts - 1, PLink = CurVec->GetFirstRealPoint(); TestCt >= 0 && PLink; PLink = PLink->Next)
													{
													if (PLinkPrev && PLink->SamePoint(PLinkPrev))
														continue;
													if (! PLink->Next && PLink->SamePoint(CurVec->GetFirstRealPoint()))
														continue;
													RoofPts[TestCt --] = PLink;
													PLinkPrev = PLink;
													} // for
												} // else
											NumObjVerts = NumWallSegs * 4 + NumRoofPts;
											NumObjPolys = NumWallSegs * 2 + NumRoofPts - 2;
											// need both walls and roof to make an enclosed 3d object for STL
											if (NumWallSegs > 2 && CurJob->WallFloors)
												{
												NumObjVerts += NumRoofPts;
												NumObjPolys += NumRoofPts - 2;
												MakeFloor = 1;
												} // if need floor
											} // if
										else
											{ // if
											NumObjVerts = NumWallSegs * 4;
											NumObjPolys = NumWallSegs * 2;
											}
										} // if
									else
										{
										NumObjVerts = NumWallSegs * 4;
										NumObjPolys = NumWallSegs * 2;
										} // else
									#else // WCS_FENCE_LIMITED
									NumObjVerts = NumWallSegs * 4;
									NumObjPolys = NumWallSegs * 2;
									#endif // WCS_FENCE_LIMITED

									if (NumObjPolys > 0 && NumObjVerts > 0)
										{
										// make a 3D object
										if (Object3D = new Object3DEffect)
											{
											CurJob->Unique3DObjectInstances ++;
											// add to list of objects created and need to be destroyed later
											CurJob->AddObjectToEphemeralList(Object3D);
											// set object name
											sprintf(Object3D->Name, "WO%06d", WallObjNumber ++);
											strcpy(Object3D->ShortName, Object3D->Name);
											// temporarily copy ActionList to ephemeral -- must clear before deleting empemeral, or shared data will be destroyed.
											Object3D->ActionList = CurFence->ActionList;
											/*
											strcpy(Object3D->Name, CurFence->Name);
											if (strlen(Object3D->Name) > WCS_EFFECT_MAXNAMELENGTH - 6)
												Object3D->Name[WCS_EFFECT_MAXNAMELENGTH - 6] = 0;
											strcat(Object3D->Name, "_Wall");
											*/

											// set number of polygons and vertices
											Object3D->NumVertices = NumObjVerts;
											Object3D->NumPolys = NumObjPolys;
											#ifndef WCS_FENCE_LIMITED
											Object3D->NumMaterials = CurFence->RoofEnabled && CurFence->SpansEnabled ? 2: 1;
											SpanMatNum = 0;
											if (CurFence->SpansEnabled)
												RoofMatNum = 1;
											else
												RoofMatNum = 0;
											#else // WCS_FENCE_LIMITED
											Object3D->NumMaterials = 1;
											SpanMatNum = RoofMatNum = 0;
											#endif // WCS_FENCE_LIMITED

											// allocate polys and verts
											if ((Object3D->Vertices = new Vertex3D[Object3D->NumVertices]) && 
												(Object3D->Polygons = new Polygon3D[Object3D->NumPolys]) &&
												(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials])
												&& Object3D->AllocVertRef(Object3D->NumPolys * 3))
												{
												VertRefBlockIdx = 0;
												sprintf(Object3D->NameTable[SpanMatNum].Name, "WM%06d", WallMatNum ++);

												if (CurFence->SpansEnabled)
													{
													if (GradCrit = CurFence->SpanMat.GetNextNode(NULL))
														{
														SourceMat = (MaterialEffect *)GradCrit->GetThing();
														if (SourceMat && (Object3D->NameTable[SpanMatNum].Mat = new MaterialEffect(NULL)))
															{
															Object3D->NameTable[SpanMatNum].Mat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[SpanMatNum].Mat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[SpanMatNum].Mat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[SpanMatNum].Mat->Copy(Object3D->NameTable[SpanMatNum].Mat, SourceMat);
															Object3D->NameTable[SpanMatNum].Mat->DoubleSided = 1;
															Object3D->NameTable[SpanMatNum].Mat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
															
															strcpy(Object3D->NameTable[SpanMatNum].Mat->Name, Object3D->NameTable[SpanMatNum].Name);
															strcpy(Object3D->NameTable[SpanMatNum].Mat->ShortName, Object3D->NameTable[SpanMatNum].Name);
															CurJob->AddObjectToEphemeralList(Object3D->NameTable[SpanMatNum].Mat);
															} // if
														else
															{
															Success = 0;
															break;
															} // else
														} // if
													} // if
												#ifndef WCS_FENCE_LIMITED
												if (CurFence->RoofEnabled)
													{
													sprintf(Object3D->NameTable[RoofMatNum].Name, "RM%06d", WallMatNum ++);
													// roof material if it is different from panels, otherwise use panel material
													if (GradCrit = CurFence->SeparateRoofMat ? CurFence->RoofMat.GetNextNode(NULL): CurFence->SpanMat.GetNextNode(NULL))
														{
														SourceMat = (MaterialEffect *)GradCrit->GetThing();
														if (SourceMat && (Object3D->NameTable[RoofMatNum].Mat = new MaterialEffect(NULL)))
															{
															Object3D->NameTable[RoofMatNum].Mat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[RoofMatNum].Mat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[RoofMatNum].Mat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
															Object3D->NameTable[RoofMatNum].Mat->Copy(Object3D->NameTable[RoofMatNum].Mat, SourceMat);
															Object3D->NameTable[RoofMatNum].Mat->DoubleSided = 1;
															Object3D->NameTable[RoofMatNum].Mat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
															
															strcpy(Object3D->NameTable[RoofMatNum].Mat->Name, Object3D->NameTable[RoofMatNum].Name);
															strcpy(Object3D->NameTable[RoofMatNum].Mat->ShortName, Object3D->NameTable[RoofMatNum].Name);
															CurJob->AddObjectToEphemeralList(Object3D->NameTable[RoofMatNum].Mat);
															} // if
														else
															{
															Success = 0;
															break;
															} // else
														} // if
													} // if
												#endif // WCS_FENCE_LIMITED

												if (MyAttr = (JoeCoordSys *)CurVec->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
													VecCoords = MyAttr->Coord;
												else
													VecCoords = NULL;
												VecCenterNS = CurVec->GetNSCenter();
												VecCenterEW = CurVec->GetEWCenter();
												Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetValue(VecCenterNS);
												Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetValue(VecCenterEW);
												Vtx[0].Lat = VecCenterNS;
												Vtx[0].Lon = VecCenterEW;
												Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
													WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
												Rend->RendData.Interactive->VertexDataPoint(&Rend->RendData, &Vtx[0], &Poly, Flags);
												RefElev = Vtx[0].Elev;

												// calculate and store polygons and vertices
												CurVert = 0;
												CurPoly = 0;
												for ( ; MainPolyCt < PolyCt && CurVert < Object3D->NumVertices && CurPoly < Object3D->NumPolys; )	// MainPolyCt is incremented in busy-win update section
													{
													TestElement = (FenceVertexList *)PolyArray[MainPolyCt].PolyNumber;
													if (CurVec != TestElement->Vec || CurFence != TestElement->Fnce)
														break;

													// process the element, inserting vertices and polygons into 3dobj
													StartVert = CurVert;
													if (TestElement->PieceType == WCS_FENCEPIECE_SPAN)
														{
														TestElement->PointA->ProjToDefDeg(VecCoords, &Vtx[0]);
														Vtx[1].CopyLatLon(&Vtx[0]);
														TestElement->PointB->ProjToDefDeg(VecCoords, &Vtx[2]);
														Vtx[3].CopyLatLon(&Vtx[2]);

														TopElev = CurFence->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue;
														BotElev = CurFence->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue;
														#ifdef WCS_THEMATIC_MAP
														if (CurFence->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV) &&
															CurFence->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)->Eval(ThemeValue, TestElement->Vec))
															TopElev = ThemeValue[0];
														if (CurFence->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV) &&
															CurFence->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)->Eval(ThemeValue, TestElement->Vec))
															BotElev = ThemeValue[0];
														#endif // WCS_THEMATIC_MAP

														#ifdef WCS_FENCE_LIMITED
														if (TopElev > 5.0)
															TopElev = 5.0;
														if (BotElev > 5.0)
															BotElev = 5.0;
														if (TopElev < -10.0)
															TopElev = -10.0;
														if (BotElev < -10.0)
															BotElev = -10.0;
														#endif // WCS_FENCE_LIMITED

														if (CurFence->Absolute == WCS_EFFECT_RELATIVETOJOE)
															{
															Vtx[0].Elev = TopElev + Rend->RendData.Exaggerate(Vtx[0].Elev);
															Vtx[1].Elev = BotElev + Rend->RendData.Exaggerate(Vtx[1].Elev);
															Vtx[2].Elev = TopElev + Rend->RendData.Exaggerate(Vtx[2].Elev);
															Vtx[3].Elev = BotElev + Rend->RendData.Exaggerate(Vtx[3].Elev);
															} // if
														else if (CurFence->Absolute == WCS_EFFECT_ABSOLUTE)
															{
															Vtx[0].Elev = TopElev;
															Vtx[1].Elev = BotElev;
															Vtx[2].Elev = TopElev;
															Vtx[3].Elev = BotElev;
															} // else if
														else
															{
															Rend->RendData.Interactive->VertexDataPoint(&Rend->RendData, &Vtx[0], &Poly, Flags);
															Vtx[1].Elev = Vtx[0].Elev + BotElev;
															Vtx[0].Elev += TopElev;
															Rend->RendData.Interactive->VertexDataPoint(&Rend->RendData, &Vtx[2], &Poly, Flags);
															Vtx[3].Elev = Vtx[2].Elev + BotElev;
															Vtx[2].Elev += TopElev;
															} // else
														// convert lat/lon to xyz relative to center of vector
														// longitude = x, latitude = z, elev = y
														// set lat and lon for future reference
														// put panel dimensions in XYZ array.

														Temp1 = (Vtx[2].Lon - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale;
														Temp2 = (Vtx[2].Lat - Vtx[0].Lat) * CurJob->ExportRefData.WCSLatScale;
														SegLen = sqrt(Temp1 * Temp1 + Temp2 * Temp2);

														Object3D->Vertices[CurVert].Lon = Vtx[0].Lon;
														Object3D->Vertices[CurVert].Lat = Vtx[0].Lat;
														Object3D->Vertices[CurVert].Elev = Vtx[0].Elev;
														Object3D->Vertices[CurVert].XYZ[0] = 0.0;
														Object3D->Vertices[CurVert].XYZ[1] = 0.0;
														Object3D->Vertices[CurVert].xyz[0] = (VecCenterEW - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale;
														Object3D->Vertices[CurVert].xyz[2] = (Vtx[0].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
														Object3D->Vertices[CurVert ++].xyz[1] = Vtx[0].Elev - RefElev;

														Object3D->Vertices[CurVert].Lon = Vtx[1].Lon;
														Object3D->Vertices[CurVert].Lat = Vtx[1].Lat;
														Object3D->Vertices[CurVert].Elev = Vtx[1].Elev;
														Object3D->Vertices[CurVert].XYZ[0] = 0.0;
														Object3D->Vertices[CurVert].XYZ[1] = TopElev - BotElev;
														Object3D->Vertices[CurVert].xyz[0] = (VecCenterEW - Vtx[1].Lon) * CurJob->ExportRefData.WCSLonScale;
														Object3D->Vertices[CurVert].xyz[2] = (Vtx[1].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
														Object3D->Vertices[CurVert ++].xyz[1] = Vtx[1].Elev - RefElev;

														Object3D->Vertices[CurVert].Lon = Vtx[2].Lon;
														Object3D->Vertices[CurVert].Lat = Vtx[2].Lat;
														Object3D->Vertices[CurVert].Elev = Vtx[2].Elev;
														Object3D->Vertices[CurVert].XYZ[0] = SegLen;
														Object3D->Vertices[CurVert].XYZ[1] = 0.0;
														Object3D->Vertices[CurVert].xyz[0] = (VecCenterEW - Vtx[2].Lon) * CurJob->ExportRefData.WCSLonScale;
														Object3D->Vertices[CurVert].xyz[2] = (Vtx[2].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
														Object3D->Vertices[CurVert ++].xyz[1] = Vtx[2].Elev - RefElev;

														Object3D->Vertices[CurVert].Lon = Vtx[3].Lon;
														Object3D->Vertices[CurVert].Lat = Vtx[3].Lat;
														Object3D->Vertices[CurVert].Elev = Vtx[3].Elev;
														Object3D->Vertices[CurVert].XYZ[0] = SegLen;
														Object3D->Vertices[CurVert].XYZ[1] = TopElev - BotElev;
														Object3D->Vertices[CurVert].xyz[0] = (VecCenterEW - Vtx[3].Lon) * CurJob->ExportRefData.WCSLonScale;
														Object3D->Vertices[CurVert].xyz[2] = (Vtx[3].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
														Object3D->Vertices[CurVert ++].xyz[1] = Vtx[3].Elev - RefElev;

														Object3D->Polygons[CurPoly].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
														VertRefBlockIdx += 3; // We just used 3 longs
														Object3D->Polygons[CurPoly].NumVerts = 3;
														Object3D->Polygons[CurPoly + 1].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
														VertRefBlockIdx += 3; // We just used 3 longs
														Object3D->Polygons[CurPoly + 1].NumVerts = 3;
														if (Area > 0.0)
															{
															Object3D->Polygons[CurPoly].VertRef[0] = StartVert;
															Object3D->Polygons[CurPoly].VertRef[1] = StartVert + 1;
															Object3D->Polygons[CurPoly].VertRef[2] = StartVert + 2;
															Object3D->Polygons[CurPoly ++].Material = SpanMatNum;
															Object3D->Polygons[CurPoly].VertRef[0] = StartVert + 1;
															Object3D->Polygons[CurPoly].VertRef[1] = StartVert + 3;
															Object3D->Polygons[CurPoly].VertRef[2] = StartVert + 2;
															Object3D->Polygons[CurPoly ++].Material = SpanMatNum;
															} // if
														else
															{
															Object3D->Polygons[CurPoly].VertRef[0] = StartVert;
															Object3D->Polygons[CurPoly].VertRef[1] = StartVert + 2;
															Object3D->Polygons[CurPoly].VertRef[2] = StartVert + 1;
															Object3D->Polygons[CurPoly ++].Material = SpanMatNum;
															Object3D->Polygons[CurPoly].VertRef[0] = StartVert + 2;
															Object3D->Polygons[CurPoly].VertRef[1] = StartVert + 3;
															Object3D->Polygons[CurPoly].VertRef[2] = StartVert + 1;
															Object3D->Polygons[CurPoly ++].Material = SpanMatNum;
															} // else
														} // if
													#ifndef WCS_FENCE_LIMITED
													else if (TestElement->PieceType == WCS_FENCEPIECE_ROOF)
														{
														if (RoofPts)
															{
															// find object bounds
															NorthMost = -FLT_MAX;
															SouthMost = FLT_MAX;
															WestMost = -FLT_MAX;
															EastMost = FLT_MAX;
															for (FirstCt = 0; FirstCt < NumRoofPts; FirstCt ++)
																{
																RoofPts[FirstCt]->ProjToDefDeg(VecCoords, &Vtx[0]);
																if (Vtx[0].Lat > NorthMost)
																	NorthMost = Vtx[0].Lat;
																if (Vtx[0].Lon > WestMost)
																	WestMost = Vtx[0].Lon;
																if (Vtx[0].Lat < SouthMost)
																	SouthMost = Vtx[0].Lat;
																if (Vtx[0].Lon < EastMost)
																	EastMost = Vtx[0].Lon;
																} // for

															for (FirstCt = 0; FirstCt < NumRoofPts; FirstCt ++)
																{
																RoofPts[FirstCt]->ProjToDefDeg(VecCoords, &Vtx[0]);

																RoofElev = CurFence->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].CurValue;
																#ifdef WCS_THEMATIC_MAP
																if (CurFence->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV) &&
																	CurFence->GetTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)->Eval(ThemeValue, TestElement->Vec))
																	RoofElev = ThemeValue[0];
																#endif // WCS_THEMATIC_MAP

																if (CurFence->Absolute == WCS_EFFECT_RELATIVETOJOE)
																	{
																	Vtx[0].Elev = RoofElev + Rend->RendData.Exaggerate(Vtx[0].Elev);
																	} // if
																else if (CurFence->Absolute == WCS_EFFECT_ABSOLUTE)
																	{
																	Vtx[0].Elev = RoofElev;
																	} // else if
																else
																	{
																	Rend->RendData.Interactive->VertexDataPoint(&Rend->RendData, &Vtx[0], &Poly, Flags);
																	Vtx[0].Elev += RoofElev;
																	} // else

																Object3D->Vertices[CurVert].Lon = Vtx[0].Lon;
																Object3D->Vertices[CurVert].Lat = Vtx[0].Lat;
																Object3D->Vertices[CurVert].Elev = Vtx[0].Elev;
																Object3D->Vertices[CurVert].XYZ[0] = (WestMost - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale; 
																Object3D->Vertices[CurVert].XYZ[1] = (NorthMost - Vtx[0].Lat) * CurJob->ExportRefData.WCSLatScale;
																Object3D->Vertices[CurVert].xyz[0] = (VecCenterEW - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale;
																Object3D->Vertices[CurVert].xyz[2] = (Vtx[0].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
																Object3D->Vertices[CurVert].xyz[1] = Vtx[0].Elev - RefElev;

																if (MakeFloor)
																	{
																	RoofPts[FirstCt]->ProjToDefDeg(VecCoords, &Vtx[0]);
																	BotElev = CurFence->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue;
																	#ifdef WCS_THEMATIC_MAP
																	if (CurFence->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV) &&
																		CurFence->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)->Eval(ThemeValue, TestElement->Vec))
																		BotElev = ThemeValue[0];
																	#endif // WCS_THEMATIC_MAP

																	#ifdef WCS_FENCE_LIMITED
																	if (BotElev > 5.0)
																		BotElev = 5.0;
																	if (BotElev < -10.0)
																		BotElev = -10.0;
																	#endif // WCS_FENCE_LIMITED

																	if (CurFence->Absolute == WCS_EFFECT_RELATIVETOJOE)
																		{
																		Vtx[0].Elev = BotElev + Rend->RendData.Exaggerate(Vtx[0].Elev);
																		} // if
																	else if (CurFence->Absolute == WCS_EFFECT_ABSOLUTE)
																		{
																		Vtx[0].Elev = BotElev;
																		} // else if
																	else
																		{
																		Rend->RendData.Interactive->VertexDataPoint(&Rend->RendData, &Vtx[0], &Poly, Flags);
																		Vtx[0].Elev += BotElev;
																		} // else

																	Object3D->Vertices[CurVert + NumRoofPts].Lon = Vtx[0].Lon;
																	Object3D->Vertices[CurVert + NumRoofPts].Lat = Vtx[0].Lat;
																	Object3D->Vertices[CurVert + NumRoofPts].Elev = Vtx[0].Elev;
																	Object3D->Vertices[CurVert + NumRoofPts].XYZ[0] = (WestMost - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale; 
																	Object3D->Vertices[CurVert + NumRoofPts].XYZ[1] = (NorthMost - Vtx[0].Lat) * CurJob->ExportRefData.WCSLatScale;
																	Object3D->Vertices[CurVert + NumRoofPts].xyz[0] = (VecCenterEW - Vtx[0].Lon) * CurJob->ExportRefData.WCSLonScale;
																	Object3D->Vertices[CurVert + NumRoofPts].xyz[2] = (Vtx[0].Lat - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
																	Object3D->Vertices[CurVert + NumRoofPts].xyz[1] = Vtx[0].Elev - RefElev;
																	} // if
																CurVert ++;
																} // for
															if (MakeFloor)
																CurVert += NumRoofPts;

															NumToTest = NumRoofPts - 2;
															Illegal = 0;
															for (FirstCt = 0; Illegal < NumToTest && NumToTest > 0; )
																{
																BreakToOuterLoop = 0;
																if (RoofPts[FirstCt])
																	{
																	for (NextCt = FirstCt + 1 < NumRoofPts ? FirstCt + 1: 0; NextCt != FirstCt && ! BreakToOuterLoop; )
																		{
																		if (RoofPts[NextCt])
																			{
																			for (NextNextCt = NextCt + 1 < NumRoofPts ? NextCt + 1: 0; NextNextCt != FirstCt && NextNextCt != NextCt; )
																				{
																				if (RoofPts[NextNextCt])
																					{
																					Legal = 1;
																					// test to see if it is a legal triangle
																					// a roof triangle is legal and OK to draw if it fills clockwise space and
																					// if no other point causes the triangle to be partly outside the roof outline
																					// Is it clockwise?
																					RoofPts[FirstCt]->ProjToDefDeg(VecCoords, &Vtx[0]);
																					RoofPts[NextCt]->ProjToDefDeg(VecCoords, &Vtx[1]);
																					RoofPts[NextNextCt]->ProjToDefDeg(VecCoords, &Vtx[2]);
																					Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
																					Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
																					Vtx[2].XYZ[0] = -(Vtx[2].Lon - Vtx[0].Lon);
																					Vtx[2].XYZ[2] = Vtx[2].Lat - Vtx[0].Lat;
																					Angle = Vtx[1].FindAngleYfromZ();
																					Vtx[2].RotateY(-Angle);
																					if (Vtx[2].XYZ[0] >= 0.0)
																						{
																						// rotate back to original position
																						Vtx[2].RotateY(Angle);
																						Angle = Vtx[2].FindAngleYfromZ();
																						Vtx[2].RotateY(-Angle);
																						for (TestCt = NextNextCt + 1 < NumRoofPts ? NextNextCt + 1: 0; (long)TestCt != FirstCt && (long)TestCt != NextCt && (long)TestCt != NextNextCt; )
																							{
																							if (RoofPts[TestCt])
																								{
																								RoofPts[TestCt]->ProjToDefDeg(VecCoords, &Vtx[1]);
																								Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
																								Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
																								Vtx[1].RotateY(-Angle);
																								if (Vtx[1].XYZ[2] > 0.0 && Vtx[1].XYZ[2] < Vtx[2].XYZ[2] && Vtx[1].XYZ[0] < 0.0)
																									{
																									Legal = 0;
																									break;
																									} // if
																								} // if
																							TestCt ++;
																							if ((long)TestCt >= NumRoofPts)
																								TestCt = 0;
																							} // for
																						} // if clockwise triangle
																					else
																						Legal = 0;

																					// if legal then fill it
																					if (Legal)
																						{
																						Object3D->Polygons[CurPoly].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
																						VertRefBlockIdx += 3; // We just used 3 longs
																						Object3D->Polygons[CurPoly].NumVerts = 3;
																						Object3D->Polygons[CurPoly].VertRef[0] = FirstCt + StartVert;
																						Object3D->Polygons[CurPoly].VertRef[1] = NextCt + StartVert;
																						Object3D->Polygons[CurPoly].VertRef[2] = NextNextCt + StartVert;
																						Object3D->Polygons[CurPoly ++].Material = RoofMatNum;
																						NumRoofPolys ++;
																						if (MakeFloor)
																							{
																							Object3D->Polygons[CurPoly].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
																							VertRefBlockIdx += 3; // We just used 3 longs
																							Object3D->Polygons[CurPoly].NumVerts = 3;
																							Object3D->Polygons[CurPoly].VertRef[2] = FirstCt + StartVert + NumRoofPts;
																							Object3D->Polygons[CurPoly].VertRef[1] = NextCt + StartVert + NumRoofPts;
																							Object3D->Polygons[CurPoly].VertRef[0] = NextNextCt + StartVert + NumRoofPts;
																							Object3D->Polygons[CurPoly ++].Material = RoofMatNum;
																							NumRoofPolys ++;
																							} // if

																						RoofPts[NextCt] = NULL;
																						NumToTest --;
																						Illegal = 0;
																						BreakToOuterLoop = 1;
																						break;
																						} // if
																					else
																						{
																						Illegal ++;
																						BreakToOuterLoop = 1;
																						break;
																						} // else
																					} // if
																				NextNextCt ++;
																				if (NextNextCt >= NumRoofPts)
																					NextNextCt = 0;
																				} // for
																			} // if
																		NextCt ++;
																		if (NextCt >= NumRoofPts)
																			NextCt = 0;
																		} // for
																	} // if
																FirstCt ++; 
																if (FirstCt >= NumRoofPts)
																	FirstCt = 0;
																} // for

															AppMem_Free(RoofPts, NumRoofPts * sizeof (VectorPoint *));
															RoofPts = NULL;
															// we added NumRoofpts worth of vertices after the last CurVert
															} // if RoofPts
														} // else
													#endif // WCS_FENCE_LIMITED

													FrameGaugeUpdate(++ MainPolyCt);
													if (! IsRunning())
														{
														Success = 0;
														break;
														} // if
													} // for

												if (Success)
													{
													Object3D->GetObjectBounds();
													// add 3D object to object instance list
													// add a new 3DOInstance and fill in the data
													if (*InstancePtr = new Object3DInstance)
														{
														CurInstance = *InstancePtr;
														CurInstance->MyObj = Object3D;
														// WCS geographic default coord sys vector center
														CurInstance->WCSGeographic[0] = VecCenterEW;
														CurInstance->WCSGeographic[1] = VecCenterNS;
														CurInstance->WCSGeographic[2] = RefElev;
														#ifdef WCS_BUILD_SX2
														if (CurFence->IsClickQueryEnabled())
															CurInstance->ClickQueryObjectID = CurFence->GetRecordNumber(CurFence, CurVec, FileNamesCreated);
														#endif // WCS_BUILD_SX2
														InstancePtr = &CurInstance->Next;
														} // if
													else
														{
														Success = 0;
														break;
														} // else
												
													// here would be a good place to render all the polygons
													// wall polygons have material 0, roof are material 1
													// walls can all be done first, then roof
													// make different image for walls and roof

													if (NumWallSegs > 0)
														{
														if (! (Success = RenderWallSegments(CurJob, Rend, Object3D, FirstWallElement, NumWallSegs, TopElev - BotElev, Area < 0.0, SpanMatNum, FileNamesCreated)))
															break;
														} // if


													#ifndef WCS_FENCE_LIMITED
													if (NumRoofPolys > 0)
														{
														if (! (Success = RenderRoofSegments(CurJob, Rend, Object3D, FirstRoofElement, NumRoofPolys, (WestMost - EastMost) * CurJob->ExportRefData.WCSLonScale, (NorthMost - SouthMost) * CurJob->ExportRefData.WCSLatScale, RoofMatNum, FileNamesCreated)))
															break;
														} // if
													#endif // WCS_FENCE_LIMITED
													} // if
												// don't delete object
												} // if vertices and polygons and name table
											else
												{
												Success = 0;
												break;
												} // else
											} // if a 3D Object
										else
											{
											Success = 0;
											break;
											} // else
										} // if some polygons and vertices
									else
										{
										FrameGaugeUpdate(MainPolyCt += NumElementsThisObject);
										if (! IsRunning())
											{
											Success = 0;
											break;
											} // if
										} // else
									} // if a fence element
								else
									{
									FrameGaugeUpdate(++ MainPolyCt);
									if (! IsRunning())
										{
										Success = 0;
										break;
										} // if
									} // else
								} // for
							FrameGaugeClear();
							
							if (CurJob->ObjectInstanceList && WallObjectList)
								{
								CurInstance = CurJob->ObjectInstanceList;
								while (CurInstance->Next)
									{
									CurInstance = CurInstance->Next;
									} // if
								CurInstance->Next = WallObjectList;
								} // if
							else if (WallObjectList)
								{
								CurJob->ObjectInstanceList = WallObjectList;
								} // else if
							WallObjectList = NULL;
							} // if something to render
						AppMem_Free(PolyArray, NumPolys * sizeof (struct TerrainPolygonSort));
						} // if
					else
						Success = 0;
					} // if NumPolys
				} // if success so far
			} // if rend init frame
		else
			Success = 0;

		ProcClear();
		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(true, false, TRUE, false);
		Rend->Opt->CleanupFromRender(TRUE);
		// steal this from renderer in case another job follows
		FrdWarned = Rend->GetFrdWarned();
		} // if
	else
		{
		Success = 0;
		//#ifdef WCS_RENDER_SCENARIOS
		//EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
		//#endif // WCS_RENDER_SCENARIOS
		Rend->Cleanup(false, false, FALSE, false);	// need to delete anything created during failed init process.
		Rend->Opt->CleanupFromRender(TRUE);
		ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
		ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
		} // else
	} // if renderer
else
	Success = 0;

#endif // WCS_BUILD_RTX
return (Success);

} // ExportControlGUI::RenderWalls

/*===========================================================================*/

// renders textures for wall panels
int ExportControlGUI::RenderWallSegments(SceneExporter *CurJob, Renderer *Rend, Object3DEffect *Object3D, 
	FenceVertexList *ListElement, long NumWallSegs, double WallHeight, int FlipNormals, int SpanMatNum, NameList **FileNamesCreated)
{
int Success = 1;
#ifdef WCS_BUILD_RTX
double Area, SegLen, OptimalWidth, OptimalHeight, RemainingWidth, UsedWidth, MaxRowWidth, WidthScale, HeightScale, 
	ScaledRowHeight, SegStartX, SegEndX, SegStartY, SegEndY, FirstSegLen, TileWidth, TileHeight, TileCenterX, TileCenterY,
	SegStart, PolySide[2][3], TotalLength = 0, MaxSegLen = 0;
long WallPairCt, WallPolyCt, FirstPair, CurrentRow, SegsUsed, MaxSegsPerRow, CurrentCol, WidthPixelsAdd,
	HeightPixelsAdd, ImageWidth, ImageHeight, VtxNum, VertNum, TempImageSize, SegCt, VertCt, WallPolysRead;
WallPolygonPair *WallPolyPairs;
Vertex3D *Vert;
VertexData *VtxPtr[3];
Polygon3D *CurPoly, *Poly1, *Poly2;
ObjectPerVertexMap *UVTable;
MaterialEffect *CurMat;
RootTexture *CurTexRoot;
UVImageTexture *CurUVTex;
Raster *UVRaster;
ImageOutputEvent *IOE;
char *ImageExtension;
int TileTextures, TransparencyExists = 0;
char TempName[256], BufBackup[WCS_MAX_BUFFERNODE_NAMELEN];
PathAndFile ImagePAF;
PolygonData Poly;
VertexData Vtx[4];
VertexBase TempVert;

// don't render textures if no image format specified
if (! CurJob->TextureImageFormat || strlen(CurJob->TextureImageFormat) == 0)
	return (Success);
ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);
if (! ImageExtension || strlen(ImageExtension) == 0)
	return (Success);

// see if there is a need for UV mapped textures
if (ListElement->Fnce->SpanMat.AreThereEnabledTextures())
	{
	if (TileTextures = (CurJob->TileWallTex && ListElement->Fnce->SpanMat.AreTexturesTileable(TileWidth, TileHeight, TileCenterX, TileCenterY)))
		{
		FirstSegLen = 0.0;
		for (WallPolyCt = 0; WallPolyCt < Object3D->NumPolys; WallPolyCt ++)
			{
			if (Object3D->Polygons[WallPolyCt].Material == SpanMatNum)
				{
				CurPoly = &Object3D->Polygons[WallPolyCt];
				FirstSegLen = Object3D->Vertices[CurPoly->VertRef[0]].XYZ[0];
				FirstSegLen = max(FirstSegLen, Object3D->Vertices[CurPoly->VertRef[1]].XYZ[0]);
				FirstSegLen = max(FirstSegLen, Object3D->Vertices[CurPoly->VertRef[2]].XYZ[0]);
				break;
				} // if
			} // for
		if (FirstSegLen < TileWidth)
			TileTextures = 0;
		} // if

	if (Object3D->UVWTable || Object3D->AllocUVWMaps(1))
		{
		CurMat = Object3D->NameTable[SpanMatNum].Mat;
		// allocate maps
		UVTable = &Object3D->UVWTable[0];
		if (UVTable->MapsValid() || UVTable->AllocMap(Object3D->NumVertices))
			{
			// assign a map name
			UVTable->SetName(Object3D->Name);

			if (! TileTextures)
				{
				if (WallPolyPairs = (struct WallPolygonPair *)AppMem_Alloc(NumWallSegs * sizeof (struct WallPolygonPair), APPMEM_CLEAR))
					{
					// find total length and longest length
					for (WallPairCt = WallPolyCt = 0; WallPairCt < NumWallSegs; WallPolyCt ++)
						{
						if (Object3D->Polygons[WallPolyCt].Material == 0)
							{
							WallPolyPairs[WallPairCt].Poly1 = CurPoly = &Object3D->Polygons[WallPolyCt ++];
							WallPolyPairs[WallPairCt].Poly2 = &Object3D->Polygons[WallPolyCt];
							SegLen = Object3D->Vertices[CurPoly->VertRef[0]].XYZ[0];
							SegLen = max(SegLen, Object3D->Vertices[CurPoly->VertRef[1]].XYZ[0]);
							SegLen = max(SegLen, Object3D->Vertices[CurPoly->VertRef[2]].XYZ[0]);
							WallPolyPairs[WallPairCt].SegLen = SegLen;
							TotalLength += SegLen;
							if (SegLen > MaxSegLen)
								MaxSegLen = SegLen;
							if (WallPairCt == NumWallSegs - 1 && NumWallSegs > 1)
								WallPolyPairs[WallPairCt].LastSeg = 1;
							WallPairCt ++;
							} // if
						} // for

					// sort polygon pairs by decreasing length
					qsort((void *)WallPolyPairs, (size_t)NumWallSegs, (size_t)(sizeof (struct WallPolygonPair)), CompareWallPolygonPair);

					// find optimal length
					Area = TotalLength * WallHeight;
					OptimalWidth = sqrt(Area);
					if (OptimalWidth < MaxSegLen)
						OptimalWidth = MaxSegLen;

					// shuffle polygon pairs into slots working from largest to smallest
					FirstPair = 0;
					CurrentRow = 0;
					SegsUsed = 0;
					MaxSegsPerRow = 0;
					MaxRowWidth = 0.0;
					while (SegsUsed < NumWallSegs)
						{
						CurrentCol = 0;
						RemainingWidth = OptimalWidth + .001;	// a little margin for error so we dont' get in an endless loop
						UsedWidth = 0.0;
						for (SegCt = FirstPair; SegCt < NumWallSegs && RemainingWidth > 0.0; SegCt ++)
							{
							if (! WallPolyPairs[SegCt].PairPlaced)
								{
								if (WallPolyPairs[SegCt].SegLen <= RemainingWidth)
									{
									WallPolyPairs[SegCt].RowNum = CurrentRow;
									WallPolyPairs[SegCt].ColNum = CurrentCol;
									WallPolyPairs[SegCt].StartX = UsedWidth;
									WallPolyPairs[SegCt].PairPlaced = 1;
									UsedWidth += WallPolyPairs[SegCt].SegLen;
									RemainingWidth -= WallPolyPairs[SegCt].SegLen;
									CurrentCol ++;
									if (UsedWidth > MaxRowWidth)
										MaxRowWidth = UsedWidth;
									if (CurrentCol > MaxSegsPerRow)
										MaxSegsPerRow = CurrentCol;
									SegsUsed ++;
									} // if
								} // if
							if (RemainingWidth < .002)
								break;
							} // for
						while (FirstPair < NumWallSegs && WallPolyPairs[FirstPair].PairPlaced)
							FirstPair ++;
						CurrentRow ++;
						} // while

					// number of extra pixels to pad space between segments and at image edges
					WidthPixelsAdd = (MaxSegsPerRow - 1 ) * 3 + 2;
					HeightPixelsAdd = (CurrentRow - 1) * 3 + 2;

					OptimalWidth = MaxRowWidth;
					OptimalHeight = CurrentRow * WallHeight;

					// scale largest side
					// use same scale to scale smaller side
					if (OptimalWidth > OptimalHeight)
						{
						TempImageSize = (long)(OptimalWidth / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
						if (TempImageSize > CurJob->MaxWallTexSize)
							TempImageSize = CurJob->MaxWallTexSize;
						ImageWidth = WidthPixelsAdd < TempImageSize / 2 ? TempImageSize: TempImageSize / 2 + WidthPixelsAdd;
						WidthScale = (ImageWidth - WidthPixelsAdd) / OptimalWidth;
						ImageHeight = (long)(WidthScale * OptimalHeight);
						ImageHeight = HeightPixelsAdd < ImageHeight / 2 ? ImageHeight: ImageHeight / 2 + HeightPixelsAdd;
						HeightScale = (ImageHeight - HeightPixelsAdd) / OptimalHeight;
						} // if
					else 
						{
						TempImageSize = (long)(OptimalHeight / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
						if (TempImageSize > CurJob->MaxWallTexSize)
							TempImageSize = CurJob->MaxWallTexSize;
						ImageHeight = HeightPixelsAdd < TempImageSize / 2 ? TempImageSize: TempImageSize / 2 + HeightPixelsAdd;
						HeightScale = (ImageHeight - HeightPixelsAdd) / OptimalHeight;
						ImageWidth = (long)(HeightScale * OptimalWidth);
						ImageWidth = WidthPixelsAdd < ImageWidth / 2 ? ImageWidth: ImageWidth / 2 + WidthPixelsAdd;
						WidthScale = (ImageWidth - WidthPixelsAdd) / OptimalWidth;
						} // if

					// scale polygon pairs and add in spacers
					// transfer data to polygon vertices
					ScaledRowHeight = WallHeight * HeightScale;
					for (SegCt = 0; SegCt < NumWallSegs; SegCt ++)
						{
						WallPolyPairs[SegCt].SegLen *= WidthScale;
						WallPolyPairs[SegCt].StartX *= WidthScale;
						SegStartX = WallPolyPairs[SegCt].ColNum * 2 + 1 + WallPolyPairs[SegCt].StartX;
						SegEndX = SegStartX + WallPolyPairs[SegCt].SegLen;
						SegStartY = WallPolyPairs[SegCt].RowNum * 2 + 1 + WallPolyPairs[SegCt].RowNum * ScaledRowHeight;
						SegEndY = SegStartY + ScaledRowHeight;
						for (VtxNum = 0; VtxNum < 3; VtxNum ++)
							{
							VertNum = WallPolyPairs[SegCt].Poly1->VertRef[VtxNum];
							Vert = &Object3D->Vertices[VertNum];
							Vert->ScrnXYZ[0] = (Vert->XYZ[0] < .0000001) ? SegStartX: SegEndX;
							Vert->ScrnXYZ[1] = (Vert->XYZ[1] < .0000001) ? SegStartY: SegEndY;
							Vert->ScrnXYZ[2] = 1.0;
							UVTable->CoordsValid[VertNum] = 1;
							UVTable->CoordsArray[0][VertNum] = (float)(Vert->ScrnXYZ[0] / ImageWidth);
							UVTable->CoordsArray[1][VertNum] = (float)(1.0 - (Vert->ScrnXYZ[1] / ImageHeight));
							UVTable->CoordsArray[2][VertNum] = 0.0f;

							VertNum = WallPolyPairs[SegCt].Poly2->VertRef[VtxNum];
							Vert = &Object3D->Vertices[VertNum];
							Vert->ScrnXYZ[0] = (Vert->XYZ[0] < .0000001) ? SegStartX: SegEndX;
							Vert->ScrnXYZ[1] = (Vert->XYZ[1] < .0000001) ? SegStartY: SegEndY;
							Vert->ScrnXYZ[2] = 1.0;
							UVTable->CoordsValid[VertNum] = 1;
							UVTable->CoordsArray[0][VertNum] = (float)(Vert->ScrnXYZ[0] / ImageWidth);
							UVTable->CoordsArray[1][VertNum] = (float)(1.0 - (Vert->ScrnXYZ[1] / ImageHeight));
							UVTable->CoordsArray[2][VertNum] = 0.0f;
							} // for
						} // for
					Object3D->VertexUVWAvailable = 1;

					// configure output
					ImagePAF.Copy(&ImagePAF, &CurJob->OutPath);
					strcpy(TempName, CurMat->Name);
					if (ImageExtension)
						AddExtension(TempName, ImageExtension);
					ImagePAF.SetName(TempName);

					Rend->Opt->OutputImageWidth = ImageWidth;
					Rend->Opt->OutputImageHeight = ImageHeight;

					IOE = Rend->Opt->OutputEvents;
					IOE->PAF.Copy(&IOE->PAF, &ImagePAF);
					IOE->AutoExtension = 0;
					IOE->AutoDigits = 0;

					// alloc new rasters
					if (Rend->SetupAndAllocBitmaps(ImageWidth, ImageHeight))
						{
						ProcInit(NumWallSegs, "Rendering Wall Texture");
						// render the polygons
						for (SegCt = 0; SegCt < NumWallSegs; SegCt ++)
							{
							VertNum = WallPolyPairs[SegCt].Poly1->VertRef[0];
							Vtx[0].CopyVDEM(&Object3D->Vertices[VertNum]);
							VertNum = WallPolyPairs[SegCt].Poly1->VertRef[2];
							Vtx[1].CopyVDEM(&Object3D->Vertices[VertNum]);
							VertNum = WallPolyPairs[SegCt].Poly1->VertRef[1];
							Vtx[2].CopyVDEM(&Object3D->Vertices[VertNum]);
							VertNum = WallPolyPairs[SegCt].Poly2->VertRef[1];
							Vtx[3].CopyVDEM(&Object3D->Vertices[VertNum]);

							for (VertCt = 0; VertCt < 4; VertCt ++)
								{
								// convert degrees to cartesian and project to screen
								#ifdef WCS_BUILD_VNS
								Rend->RendData.DefCoords->DegToCart(&Vtx[VertCt]);
								#else // WCS_BUILD_VNS
								Vtx[VertCt].DegToCart(Rend->RendData.PlanetRad);
								#endif // WCS_BUILD_VNS
								} // for

							Poly.Vector = ListElement->Vec;
							Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
							Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
							// if not end connecting segment then PointA->Next will be PointB
							// if end connecting segment then PointA->Next will be NULL
							Poly.VectorType |= (WallPolyPairs[SegCt].LastSeg ? WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT: 0);
							Poly.Lat = (Vtx[0].Lat + Vtx[2].Lat) * .5;
							Poly.Lon = (Vtx[0].Lon + Vtx[2].Lon) * .5;
							Poly.Elev = (Vtx[1].Elev + Vtx[3].Elev) * .5;
							Poly.Z = 1.0;
							Poly.Q = 1.0;
							Poly.Slope = 90.0;

							Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
							Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

							// compute surface normal - it is same for both triangles
							FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
							FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
							SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);
							//if (FlipNormals)
							//	NegateVector(Poly.Normal);

							TempVert.XYZ[0] = Poly.Normal[0];
							TempVert.XYZ[1] = Poly.Normal[1];
							TempVert.XYZ[2] = Poly.Normal[2];
							TempVert.RotateY(-Poly.Lon);
							TempVert.RotateX(90.0 - Poly.Lat);
							Poly.Aspect = TempVert.FindAngleYfromZ();
							if (Poly.Aspect < 0.0)
								Poly.Aspect += 360.0;

							Poly.Beach = &ListElement->Fnce->SpanMat;
							Poly.Fnce = ListElement->Fnce;
							Poly.FenceType = WCS_FENCEPIECE_SPAN;
							if (Rend->ShadowsExist)
								Rend->EffectsBase->EvalShadows(&Rend->RendData, &Poly);

							VtxPtr[0] = &Vtx[0];
							VtxPtr[1] = &Vtx[2];
							VtxPtr[2] = &Vtx[1];
							for (VertCt = 0; VertCt < 2; VertCt ++)
								{
								if (! (Success = Rend->InstigateTerrainPolygon(&Poly, VtxPtr)))
									break;
								VtxPtr[0] = &Vtx[2];
								VtxPtr[1] = &Vtx[1];
								VtxPtr[2] = &Vtx[3];
								} // for
							ProcUpdate(SegCt + 1);
							if (! IsRunning())
								{
								Success = 0;
								break;
								} // if
							} // for
						ProcClear();
						} // if new bitmaps allocated
					else
						Success = 0;
					AppMem_Free(WallPolyPairs, NumWallSegs * sizeof (struct WallPolygonPair));
					} // if WallPolyPairs
				else
					Success = 0;
				} // if ! TileTextures
			else
				{
				// find optimal length, height in meters
				OptimalWidth = TileWidth;	// by restrictions placed near top of this function, tile width is always less than the first segment
				OptimalHeight = min(WallHeight, TileHeight);

				// render a texture patch which is the smaller of length and texture width / height and texture height
				// scale largest side
				// use same scale to scale smaller side
				if (OptimalWidth > OptimalHeight)
					{
					ImageWidth = (long)(OptimalWidth / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
					if (ImageWidth > CurJob->MaxWallTexSize)
						ImageWidth = CurJob->MaxWallTexSize;
					WidthScale = ImageWidth / OptimalWidth;
					ImageHeight = (long)(WidthScale * OptimalHeight);
					ImageHeight = ImageHeight;
					HeightScale = ImageHeight / OptimalHeight;
					} // if
				else 
					{
					ImageHeight = (long)(OptimalHeight / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
					if (ImageHeight > CurJob->MaxWallTexSize)
						ImageHeight = CurJob->MaxWallTexSize;
					HeightScale = ImageHeight / OptimalHeight;
					ImageWidth = (long)(HeightScale * OptimalWidth);
					ImageWidth = ImageWidth;
					WidthScale = ImageWidth / OptimalWidth;
					} // if

				// assign texture UV coordinates to object vertices based on multiples of the width and height
				SegStart = 0.0;
				for (WallPolyCt = WallPolysRead = 0; WallPolyCt < Object3D->NumPolys; WallPolyCt ++)
					{
					// test for Object3D->Polygons[WallPolyCt].VertRef because sometimes not all roof polygons are valid
					if (Object3D->Polygons[WallPolyCt].Material == 0 && Object3D->Polygons[WallPolyCt].VertRef)
						{
						CurPoly = &Object3D->Polygons[WallPolyCt];

						VertNum = CurPoly->VertRef[0];
						Vert = &Object3D->Vertices[VertNum];
						SegLen = Vert->XYZ[0];
						UVTable->CoordsValid[VertNum] = 1;
						UVTable->CoordsArray[0][VertNum] = (float)((Vert->XYZ[0] + SegStart) / OptimalWidth);
						UVTable->CoordsArray[1][VertNum] = (float)(Vert->XYZ[1] / OptimalHeight);
						UVTable->CoordsArray[2][VertNum] = 0.0f;

						VertNum = CurPoly->VertRef[1];
						Vert = &Object3D->Vertices[VertNum];
						SegLen = max(SegLen, Vert->XYZ[0]);
						UVTable->CoordsValid[VertNum] = 1;
						UVTable->CoordsArray[0][VertNum] = (float)((Vert->XYZ[0] + SegStart) / OptimalWidth);
						UVTable->CoordsArray[1][VertNum] = (float)(Vert->XYZ[1] / OptimalHeight);
						UVTable->CoordsArray[2][VertNum] = 0.0f;

						VertNum = CurPoly->VertRef[2];
						Vert = &Object3D->Vertices[VertNum];
						SegLen = max(SegLen, Vert->XYZ[0]);
						UVTable->CoordsValid[VertNum] = 1;
						UVTable->CoordsArray[0][VertNum] = (float)((Vert->XYZ[0] + SegStart) / OptimalWidth);
						UVTable->CoordsArray[1][VertNum] = (float)(Vert->XYZ[1] / OptimalHeight);
						UVTable->CoordsArray[2][VertNum] = 0.0f;
						
						if (WallPolysRead % 2)
							SegStart += SegLen;
						WallPolysRead ++;
						} // if
					} // for
				Object3D->VertexUVWAvailable = 1;

				// render the patch from coordinate position 0,0 to OptimalWidth, OptimalHeight

				// configure output
				ImagePAF.Copy(&ImagePAF, &CurJob->OutPath);
				strcpy(TempName, CurMat->Name);
				if (ImageExtension)
					AddExtension(TempName, ImageExtension);
				ImagePAF.SetName(TempName);

				Rend->Opt->OutputImageWidth = ImageWidth;
				Rend->Opt->OutputImageHeight = ImageHeight;

				IOE = Rend->Opt->OutputEvents;
				IOE->PAF.Copy(&IOE->PAF, &ImagePAF);
				IOE->AutoExtension = 0;
				IOE->AutoDigits = 0;

				TransparencyExists = ListElement->Fnce->SpanMat.IsThereTransparentMaterial();

				// alloc new rasters
				if (Rend->SetupAndAllocBitmaps(ImageWidth, ImageHeight))
					{
					ProcInit(2, "Rendering Wall Texture");
					// render two polygons
					// set up the first two polygons of the object as if they were the screen coords of the image

					Poly1 = Poly2 = NULL;
					for (WallPolyCt = 0; WallPolyCt < Object3D->NumPolys; WallPolyCt ++)
						{
						// test for Object3D->Polygons[WallPolyCt].VertRef because sometimes not all roof polygons are valid
						if (Object3D->Polygons[WallPolyCt].Material == 0 && Object3D->Polygons[WallPolyCt].VertRef)
							{
							if (! Poly1)
								Poly1 = &Object3D->Polygons[WallPolyCt];
							else
								{
								Poly2 = &Object3D->Polygons[WallPolyCt];
								break;
								} // else
							} // if
						} // for

					if (Poly1 && Poly2)
						{
						VertNum = Poly1->VertRef[0];
						Vtx[0].CopyVDEM(&Object3D->Vertices[VertNum]);
						VertNum = Poly1->VertRef[2];
						Vtx[1].CopyVDEM(&Object3D->Vertices[VertNum]);
						VertNum = Poly1->VertRef[1];
						Vtx[2].CopyVDEM(&Object3D->Vertices[VertNum]);
						VertNum = Poly2->VertRef[1];
						Vtx[3].CopyVDEM(&Object3D->Vertices[VertNum]);

						for (VertCt = 0; VertCt < 4; VertCt ++)
							{
							if (fabs(Vtx[VertCt].XYZ[0]) < .0001)	// start
								{
								Vtx[VertCt].ScrnXYZ[0] = 0.0;
								// no need to change lat/lon
								} // if
							else	// end of seg
								{
								Vtx[VertCt].ScrnXYZ[0] = ImageWidth;
								Vtx[VertCt].Lat = Vtx[0].Lat + (Vtx[VertCt].Lat - Vtx[0].Lat) * OptimalWidth / FirstSegLen;
								Vtx[VertCt].Lon = Vtx[0].Lon + (Vtx[VertCt].Lon - Vtx[0].Lon) * OptimalWidth / FirstSegLen;
								} // else
							if (fabs(Vtx[VertCt].XYZ[1]) < .0001)	// bottom
								{
								Vtx[VertCt].ScrnXYZ[1] = 0.0;
								Vtx[VertCt].Elev = 0.0;
								} // if
							else	// top
								{
								Vtx[VertCt].ScrnXYZ[1] = ImageHeight;
								Vtx[VertCt].Elev = OptimalHeight;
								} // else
							Vtx[VertCt].ScrnXYZ[2] = 1.0;
							// convert degrees to cartesian and project to screen
							#ifdef WCS_BUILD_VNS
							Rend->RendData.DefCoords->DegToCart(&Vtx[VertCt]);
							#else // WCS_BUILD_VNS
							Vtx[VertCt].DegToCart(Rend->RendData.PlanetRad);
							#endif // WCS_BUILD_VNS
							} // for

						Poly.Vector = ListElement->Vec;
						Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
						Poly.Lat = (Vtx[0].Lat + Vtx[2].Lat) * .5;
						Poly.Lon = (Vtx[0].Lon + Vtx[2].Lon) * .5;
						Poly.Elev = (Vtx[1].Elev + Vtx[3].Elev) * .5;
						Poly.Z = 1.0;
						Poly.Q = 1.0;
						Poly.Slope = 90.0;

						Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
						Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

						// compute surface normal - it is same for both triangles
						FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
						FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
						SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);
						//if (FlipNormals)
						//	NegateVector(Poly.Normal);

						TempVert.XYZ[0] = Poly.Normal[0];
						TempVert.XYZ[1] = Poly.Normal[1];
						TempVert.XYZ[2] = Poly.Normal[2];
						TempVert.RotateY(-Poly.Lon);
						TempVert.RotateX(90.0 - Poly.Lat);
						Poly.Aspect = TempVert.FindAngleYfromZ();
						if (Poly.Aspect < 0.0)
							Poly.Aspect += 360.0;

						Poly.Beach = &ListElement->Fnce->SpanMat;
						Poly.Fnce = ListElement->Fnce;
						Poly.FenceType = WCS_FENCEPIECE_SPAN;

						VtxPtr[0] = &Vtx[0];
						VtxPtr[1] = &Vtx[2];
						VtxPtr[2] = &Vtx[1];
						for (VertCt = 0; VertCt < 2; VertCt ++)
							{
							if (! (Success = Rend->InstigateTerrainPolygon(&Poly, VtxPtr)))
								break;
							VtxPtr[0] = &Vtx[2];
							VtxPtr[1] = &Vtx[1];
							VtxPtr[2] = &Vtx[3];
							ProcUpdate(VertCt + 1);
							if (! IsRunning())
								{
								Success = 0;
								} // if
							} // for
						} // if
					else
						Success = 0;
					ProcClear();
					} // if
				else
					Success = 0;
				} // else tile textures

			// should have a texture map by now
			if (Success)
				{
				Rend->CollapseMap();
				// turn off saving alpha channel if it might cause problems
				strcpy(BufBackup, IOE->OutBuffers[3]);
				if (! TransparencyExists || ExportFmt->ProhibitAlphaSave() || (ExportFmt->CautionAlphaSave() && ! CheckAlphaWorthwhile (Rend->AABuf, Rend->Width * Rend->Height)))
					IOE->OutBuffers[3][0] = 0;
				if (Rend->Opt->SaveImage(Rend, NULL, Rend->Buffers, Rend->Width, Rend->Height, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 0, FALSE))
					{
					AddNewNameList(FileNamesCreated, (char *)ImagePAF.GetName(), WCS_EXPORTCONTROL_FILETYPE_WALLTEX);
					// remove any material textures
					if (CurJob->BurnWallShading && ! TileTextures)
						CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
					// transparency will be handled by alpha channel in texture map
					if (CurMat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
						CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetValue(0.0);
					CurMat->DeleteAllTextures();

					// create a texture for the diffuse
					if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
						{
						if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
							{
							CurUVTex->SetVertexMap(Object3D->Name);
							if (UVRaster = ImageHost->AddRaster((char *)ImagePAF.GetPath(), (char *)ImagePAF.GetName(), 0, 0, 1))
								{
								UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
								CurUVTex->SetRaster(UVRaster);
								CurUVTex->TileWidth = CurUVTex->TileHeight = 1;
								CurJob->AddObjectToEphemeralList(UVRaster);
								CurMat->PixelTexturesExist = true;
								} // if
							else
								Success = 0;
							} // if
						} // if
					} // if
				else
					{
					Success = 0;
					} // if
				strcpy(IOE->OutBuffers[3], BufBackup);
				} // if
			} // if alloc
		else
			{
			Success = 0;
			Object3D->FreeUVWMaps();
			} // else
		} // if Object3D->UVWTable
	} // if wall textures

#endif // WCS_BUILD_RTX
return (Success);

} // ExportControlGUI::RenderWallSegments

/*===========================================================================*/

// renders textures for wall roofs
int ExportControlGUI::RenderRoofSegments(SceneExporter *CurJob, Renderer *Rend, Object3DEffect *Object3D, 
	FenceVertexList *ListElement, long NumRoofPolys, double RoofWidth, double RoofHeight, int RoofMatNum, NameList **FileNamesCreated)
{
double OptimalWidth, OptimalHeight, WidthScale, HeightScale, PolySide[2][3], TotalLength = 0, MaxSegLen = 0;
WallPolygonPair *WallPolyPairs;
Vertex3D *Vert;
VertexData *VtxPtr[3];
Polygon3D *CurPoly;
ObjectPerVertexMap *UVTable;
MaterialEffect *CurMat;
AnimMaterialGradient *RoofMatGrad;
RootTexture *CurTexRoot;
UVImageTexture *CurUVTex;
Raster *UVRaster;
ImageOutputEvent *IOE;
char *ImageExtension;
long WallPairCt, WallPolyCt, WidthPixelsAdd,
	HeightPixelsAdd, ImageWidth, ImageHeight, VtxNum, VertNum, TempImageSize, SegCt, VertCt;
int Success = 1, TransparencyExists = 0;
PathAndFile ImagePAF;
PolygonData Poly;
VertexData Vtx[4];
VertexBase TempVert;
char TempName[256], BufBackup[WCS_MAX_BUFFERNODE_NAMELEN];

// don't render textures if no image format specified
if (! CurJob->TextureImageFormat || strlen(CurJob->TextureImageFormat) == 0)
	return (Success);
ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);
if (! ImageExtension || strlen(ImageExtension) == 0)
	return (Success);
RoofMatGrad = ListElement->Fnce->SeparateRoofMat ? &ListElement->Fnce->RoofMat: &ListElement->Fnce->SpanMat;

// see if there is a need for UV mapped textures
if (RoofMatGrad->AreThereEnabledTextures())
	{
	if (Object3D->UVWTable || Object3D->AllocUVWMaps(1))
		{
		CurMat = Object3D->NameTable[RoofMatNum].Mat;
		// allocate maps
		UVTable = &Object3D->UVWTable[0];
		if (UVTable->MapsValid() || UVTable->AllocMap(Object3D->NumVertices))
			{
			// assign a map name
			UVTable->SetName(Object3D->Name);
			if (WallPolyPairs = (struct WallPolygonPair *)AppMem_Alloc(NumRoofPolys * sizeof (struct WallPolygonPair), APPMEM_CLEAR))
				{
				// find total length and longest length
				for (WallPairCt = WallPolyCt = 0; WallPairCt < NumRoofPolys; WallPolyCt ++)
					{
					if (Object3D->Polygons[WallPolyCt].Material == RoofMatNum)
						{
						WallPolyPairs[WallPairCt].Poly1 = CurPoly = &Object3D->Polygons[WallPolyCt];
						WallPairCt ++;
						} // if
					} // for

				OptimalWidth = RoofWidth;
				OptimalHeight = RoofHeight;

				// number of extra pixels to pad space between segments and at image edges
				WidthPixelsAdd = 2;
				HeightPixelsAdd = 2;

				// scale largest side to 512
				// use same scale to scale smaller side
				if (OptimalWidth > OptimalHeight)
					{
					TempImageSize = (long)(OptimalWidth / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
					if (TempImageSize > CurJob->MaxWallTexSize)
						TempImageSize = CurJob->MaxWallTexSize;
					ImageWidth = WidthPixelsAdd < TempImageSize / 2 ? TempImageSize: TempImageSize / 2 + WidthPixelsAdd;
					WidthScale = (ImageWidth - WidthPixelsAdd) / OptimalWidth;
					ImageHeight = (long)(WidthScale * OptimalHeight);
					ImageHeight = HeightPixelsAdd < ImageHeight / 2 ? ImageHeight: ImageHeight / 2 + HeightPixelsAdd;
					HeightScale = (ImageHeight - HeightPixelsAdd) / OptimalHeight;
					} // if
				else 
					{
					TempImageSize = (long)(OptimalHeight / CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].CurValue);
					if (TempImageSize > CurJob->MaxWallTexSize)
						TempImageSize = CurJob->MaxWallTexSize;
					ImageHeight = HeightPixelsAdd < TempImageSize / 2 ? TempImageSize: TempImageSize / 2 + HeightPixelsAdd;
					HeightScale = (ImageHeight - HeightPixelsAdd) / OptimalHeight;
					ImageWidth = (long)(HeightScale * OptimalWidth);
					ImageWidth = WidthPixelsAdd < ImageWidth / 2 ? ImageWidth: ImageWidth / 2 + WidthPixelsAdd;
					WidthScale = (ImageWidth - WidthPixelsAdd) / OptimalWidth;
					} // if

				// scale polygon pairs and add in spacers
				// transfer data to polygon vertices
				for (SegCt = 0; SegCt < NumRoofPolys; SegCt ++)
					{
					for (VtxNum = 0; VtxNum < 3; VtxNum ++)
						{
						VertNum = WallPolyPairs[SegCt].Poly1->VertRef[VtxNum];
						Vert = &Object3D->Vertices[VertNum];
						Vert->ScrnXYZ[0] = Vert->XYZ[0] * WidthScale;
						Vert->ScrnXYZ[1] = Vert->XYZ[1] * HeightScale;
						Vert->ScrnXYZ[2] = 1.0;
						UVTable->CoordsValid[VertNum] = 1;
						UVTable->CoordsArray[0][VertNum] = (float)(Vert->ScrnXYZ[0] / ImageWidth);
						UVTable->CoordsArray[1][VertNum] = (float)(1.0 - (Vert->ScrnXYZ[1] / ImageHeight));
						UVTable->CoordsArray[2][VertNum] = 0.0f;
						} // for
					} // for
				Object3D->VertexUVWAvailable = 1;

				// configure output
				ImagePAF.Copy(&ImagePAF, &CurJob->OutPath);
				strcpy(TempName, CurMat->Name);
				if (ImageExtension)
					AddExtension(TempName, ImageExtension);
				ImagePAF.SetName(TempName);

				Rend->Opt->OutputImageWidth = ImageWidth;
				Rend->Opt->OutputImageHeight = ImageHeight;

				IOE = Rend->Opt->OutputEvents;
				IOE->PAF.Copy(&IOE->PAF, &ImagePAF);
				IOE->AutoExtension = 0;
				IOE->AutoDigits = 0;

				TransparencyExists = RoofMatGrad->IsThereTransparentMaterial();

				// alloc new rasters
				if (Rend->SetupAndAllocBitmaps(ImageWidth, ImageHeight))
					{
					ProcInit(NumRoofPolys, "Rendering Roof Texture");
					// render the polygons
					for (SegCt = 0; SegCt < NumRoofPolys; SegCt ++)
						{
						VertNum = WallPolyPairs[SegCt].Poly1->VertRef[0];
						Vtx[0].CopyVDEM(&Object3D->Vertices[VertNum]);
						VertNum = WallPolyPairs[SegCt].Poly1->VertRef[1];
						Vtx[1].CopyVDEM(&Object3D->Vertices[VertNum]);
						VertNum = WallPolyPairs[SegCt].Poly1->VertRef[2];
						Vtx[2].CopyVDEM(&Object3D->Vertices[VertNum]);

						for (VertCt = 0; VertCt < 3; VertCt ++)
							{
							// convert degrees to cartesian and project to screen
							#ifdef WCS_BUILD_VNS
							Rend->RendData.DefCoords->DegToCart(&Vtx[VertCt]);
							#else // WCS_BUILD_VNS
							Vtx[VertCt].DegToCart(Rend->RendData.PlanetRad);
							#endif // WCS_BUILD_VNS
							} // for

						Poly.Vector = ListElement->Vec;
						Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
						Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
						// if not end connecting segment then PointA->Next will be PointB
						// if end connecting segment then PointA->Next will be NULL
						Poly.VectorType |= (WallPolyPairs[SegCt].LastSeg ? WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT: 0);
						Poly.Lat = ListElement->Lat;
						Poly.Lon = ListElement->Lon;
						Poly.Elev = ListElement->Elev;
						Poly.Z = 1.0;
						Poly.Q = 1.0;
						Poly.Slope = 0.0;

						Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
						Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

						// compute surface normal - it is same for both triangles
						FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
						FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
						SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);
						//if (FlipNormals)
							NegateVector(Poly.Normal);

						Poly.Beach = RoofMatGrad;
						Poly.Fnce = ListElement->Fnce;
						Poly.FenceType = WCS_FENCEPIECE_ROOF;
						if (Rend->ShadowsExist)
							Rend->EffectsBase->EvalShadows(&Rend->RendData, &Poly);

						VtxPtr[0] = &Vtx[0];
						VtxPtr[1] = &Vtx[2];
						VtxPtr[2] = &Vtx[1];
						if (! (Success = Rend->InstigateTerrainPolygon(&Poly, VtxPtr)))
							break;

						ProcUpdate(SegCt + 1);
						if (! IsRunning())
							{
							Success = 0;
							break;
							} // if
						} // for
					ProcClear();
					
					if (Success)
						{
						Rend->CollapseMap();
						// turn off saving alpha channel if it might cause problems
						strcpy(BufBackup, IOE->OutBuffers[3]);
						if (! TransparencyExists || ExportFmt->ProhibitAlphaSave() || (ExportFmt->CautionAlphaSave() && ! CheckAlphaWorthwhile (Rend->AABuf, Rend->Width * Rend->Height)))
							IOE->OutBuffers[3][0] = 0;
						if (Rend->Opt->SaveImage(Rend, NULL, Rend->Buffers, Rend->Width, Rend->Height, 0, 0, 0, 0, 0, 0, 0, 0,
							1, 1, 1, 1, 1, 1, 0, FALSE))
							{
							AddNewNameList(FileNamesCreated, (char *)ImagePAF.GetName(), WCS_EXPORTCONTROL_FILETYPE_WALLTEX);
							// remove any material textures
							if (CurJob->BurnWallShading)
								CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
							// transparency will be handled by alpha channel in texture map
							if (CurMat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
								CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetValue(0.0);
							CurMat->DeleteAllTextures();

							// create a texture for the diffuse
							if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
								{
								if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
									{
									CurUVTex->SetVertexMap(Object3D->Name);
									if (UVRaster = ImageHost->AddRaster((char *)ImagePAF.GetPath(), (char *)ImagePAF.GetName(), 0, 1, 1))
										{
										UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
										CurUVTex->SetRaster(UVRaster);
										CurJob->AddObjectToEphemeralList(UVRaster);
										CurMat->PixelTexturesExist = true;
										} // if
									else
										Success = 0;
									} // if
								} // if
							} // if
						else
							{
							Success = 0;
							} // if
						strcpy(IOE->OutBuffers[3], BufBackup);
						} // else
					} // if new bitmaps allocated
				else
					Success = 0;
				AppMem_Free(WallPolyPairs, NumRoofPolys * sizeof (struct WallPolygonPair));
				} // if WallPolyPairs
			else
				Success = 0;
			} // if alloc
		else
			{
			Success = 0;
			Object3D->FreeUVWMaps();
			} // else
		} // if Object3D->UVWTable
	} // if

return (Success);

} // ExportControlGUI::RenderRoofSegments

/*===========================================================================*/

// creates a 3d object for terrain if needed and adds it to the 3d object instance list
int ExportControlGUI::ExportTerrainAs3DObject(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double MapWidth, MapHeight, NSMapCenter, WEMapCenter, MapCellNS, MapCellWE, MapLatScale, MapLonScale, RefElev, u, v;
long DEMFileType, TEXFileType, RowSize, Row, Col, Zip, CurTileY, CurTileX, CurTexTileY, CurTexTileX, LowPt, VertRefBlockIdx,
	TerObjNumber = 0;
float *ElevBuf;
const char *OutputFilePath, *DEMFileNameOfKnownType = NULL, *TEXFileNameOfKnownType = NULL;
char *Ext;
MaterialEffect *CurMat = NULL;
Object3DEffect *Object3D;
RootTexture *CurTexRoot;
UVImageTexture *CurUVTex;
Raster *UVRaster;
Object3DInstance **InstancePtr, *CurInstance, *TerrainObjectList = NULL;
FILE *ffile;
int Success = 1;
char FullName[512], TempName[256], FullOriginalPath[512], FullNewPath[512];

// The directory where all the files should be created is:
OutputFilePath = CurJob->OutPath.GetPath();
RowSize = CurJob->OneDEMResX * sizeof (float);
InstancePtr = &TerrainObjectList;

// find each terrain tile
// a method can be called on the root NameList to get the file name of a particular type if it exists
DEMFileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
TEXFileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
// this should be a raw elevation file
while (Success && (DEMFileNameOfKnownType = (*FileNamesCreated)->FindNextNameOfType(DEMFileType, DEMFileNameOfKnownType)))
	{
	ParseTileNumbers(DEMFileNameOfKnownType, CurTileY, CurTileX);

	// see if there is a texture file to go with it
	TEXFileNameOfKnownType = NULL;

	while (TEXFileNameOfKnownType = (*FileNamesCreated)->FindNextNameOfType(TEXFileType, TEXFileNameOfKnownType))
		{
		ParseTileNumbers(TEXFileNameOfKnownType, CurTexTileY, CurTexTileX);
		if (CurTexTileY == CurTileY && CurTexTileX == CurTileX)
			break;
		} // while

	CurJob->RBounds.DeriveTileCoords(CurJob->DEMResY, CurJob->DEMResX, 
		CurJob->DEMTilesY, CurJob->DEMTilesX, CurTileY, CurTileX, CurJob->DEMTileOverlap);

	// map center will be in the prevailing export coords whether geographic or metric 
	// and will always be in GIS convention of lon positive to east
	// Exportlat and LonScales will match the coordinate space as will cell sizes
	CurJob->RBounds.FetchRegionCenterGIS(NSMapCenter, WEMapCenter);
	CurJob->RBounds.FetchCellSizeGIS(MapCellNS, MapCellWE);

	MapLatScale = CurJob->ExportRefData.ExportLatScale;
	MapLonScale = CurJob->ExportRefData.ExportLonScale;
	RefElev = CurJob->ExportRefData.RefElev;
	MapHeight = fabs(MapCellNS) * (CurJob->OneDEMResY - 1) * MapLatScale;
	MapWidth = fabs(MapCellWE) * (CurJob->OneDEMResX - 1) * MapLonScale;

	if (Object3D = new Object3DEffect)
		{
		CurJob->Unique3DObjectInstances ++;
		if (*InstancePtr = new Object3DInstance)
			{
			CurInstance = *InstancePtr;
			CurInstance->MyObj = Object3D;
			// coordinates may be geographic or metric, this will ensure that they are geographic in the default coord sys
			CurJob->RBounds.RBoundsToDefDeg(WEMapCenter, NSMapCenter, NSMapCenter, WEMapCenter);
			CurInstance->WCSGeographic[0] = WEMapCenter;
			CurInstance->WCSGeographic[1] = NSMapCenter;
			CurInstance->WCSGeographic[2] = RefElev;

			CurJob->AddObjectToEphemeralList(Object3D);

			sprintf(Object3D->Name, "TO%06d", TerObjNumber);
			strcpy(Object3D->ShortName, Object3D->Name);

			// make a material for it if there is a texture image, otherwise use the default material
			if (TEXFileNameOfKnownType || ! CurMat)
				{
				if (CurMat = new MaterialEffect(NULL))
					{
					CurMat->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
					CurMat->SpecularColor.SetValue3(1.0, 1.0, 1.0);
					CurMat->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
					CurJob->AddObjectToEphemeralList(CurMat);
					sprintf(CurMat->Name, "TM%06d", TerObjNumber);
					strcpy(CurMat->ShortName, CurMat->Name);
					CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
					if (CurJob->BurnShading)
						CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
					if (TEXFileNameOfKnownType)
						{
						// make diffuse UV texture
						if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
							{
							if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
								{
								CurUVTex->SetVertexMap(Object3D->Name);
								// change file name of raster to material name which is unique, add extension
								Ext = FindFileExtension((char *)TEXFileNameOfKnownType);
								strmfp(FullOriginalPath, (char *)OutputFilePath, (char *)TEXFileNameOfKnownType);
								strcpy(TempName, CurMat->Name);
								AddExtension(TempName, Ext);
								strmfp(FullNewPath, (char *)OutputFilePath, TempName);
								PROJ_remove(FullNewPath);
								PROJ_rename(FullOriginalPath, FullNewPath);
								RemoveNameList(FileNamesCreated, (char *)TEXFileNameOfKnownType, WCS_EXPORTCONTROL_FILETYPE_TEX1);
								AddNewNameList(FileNamesCreated, (char *)TempName, WCS_EXPORTCONTROL_FILETYPE_TEX1);
								if (UVRaster = ImageHost->AddRaster((char *)OutputFilePath, TempName, 0, 1, 1))
									{
									UVRaster->AlphaEnabled = UVRaster->AlphaAvailable && CurJob->TransparentPixelsExist;
									CurUVTex->SetRaster(UVRaster);
									CurJob->AddObjectToEphemeralList(UVRaster);
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								} // if
							} // if
						} // if
					} // if
				else
					{
					Success = 0;
					break;
					} // else
				} // if

			// allocate vertices and polygons and material table and UV map
			// set number of polygons and vertices
			Object3D->NumVertices = CurJob->OneDEMResX * CurJob->OneDEMResY;
			Object3D->NumPolys = (CurJob->OneDEMResX - 1) * (CurJob->OneDEMResY - 1) * 2;
			Object3D->NumMaterials = 1;

			// allocate polys and verts
			if (CurMat && (Object3D->Vertices = new Vertex3D[Object3D->NumVertices]) && 
				(Object3D->Polygons = new Polygon3D[Object3D->NumPolys]) &&
				(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials]) && 
				Object3D->AllocVertRef(Object3D->NumPolys * 3) && (! TEXFileNameOfKnownType ||
				(Object3D->AllocUVWMaps(1) && Object3D->UVWTable[0].AllocMap(Object3D->NumVertices))))
				{
				VertRefBlockIdx = 0;
				strcpy(Object3D->NameTable[0].Name, CurMat->Name);
				Object3D->NameTable[0].Mat = CurMat;
				if (Object3D->UVWTable)
					{
					Object3D->UVWTable[0].SetName(Object3D->Name);
					Object3D->VertexUVWAvailable = 1;
					} // if
				// open terrain file
				if (ElevBuf = (float *)AppMem_Alloc(RowSize, 0))
					{
					strmfp(FullName, OutputFilePath, DEMFileNameOfKnownType);
					if (ffile = PROJ_fopen(FullName, "rb"))
						{
						for (Row = Zip = 0; Row < CurJob->OneDEMResY; Row ++)
							{
							v = (double)Row / (CurJob->OneDEMResY - 1);
							if (fread((char *)ElevBuf, RowSize, 1, ffile) == 1)
								{
								for (Col = 0; Col < CurJob->OneDEMResX; Col ++, Zip ++)
									{
									u = (double)Col / (CurJob->OneDEMResX - 1);
									Object3D->Vertices[Zip].xyz[0] = (u - .5) * MapWidth;
									Object3D->Vertices[Zip].xyz[1] = ElevBuf[Col] - RefElev;
									Object3D->Vertices[Zip].xyz[2] = -(v - .5) * MapHeight;
									if (Object3D->UVWTable)
										{
										Object3D->UVWTable[0].CoordsValid[Zip] = 1; 
										Object3D->UVWTable[0].CoordsArray[0][Zip] = (float)u; 
										Object3D->UVWTable[0].CoordsArray[1][Zip] = (float)(1.0 - v);
										} // if
									} // for
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // for
						fclose(ffile);
						PROJ_remove(FullName);

						// parse out polygons
						for (Row = Zip = 0; Row < CurJob->OneDEMResY - 1; Row ++)
							{
							LowPt = Row * CurJob->OneDEMResX;
							for (Col = 0; Col < CurJob->OneDEMResX - 1; Col ++, LowPt ++, Zip += 2)
								{
								Object3D->Polygons[Zip].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
								VertRefBlockIdx += 3; // We just used 3 longs
								Object3D->Polygons[Zip].NumVerts = 3;
								Object3D->Polygons[Zip + 1].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
								VertRefBlockIdx += 3; // We just used 3 longs
								Object3D->Polygons[Zip + 1].NumVerts = 3;
								Object3D->Polygons[Zip].VertRef[0] = LowPt;
								Object3D->Polygons[Zip].VertRef[1] = LowPt + 1;
								Object3D->Polygons[Zip].VertRef[2] = LowPt + CurJob->OneDEMResX;
								Object3D->Polygons[Zip].Material = 0;
								Object3D->Polygons[Zip + 1].VertRef[0] = LowPt + 1;
								Object3D->Polygons[Zip + 1].VertRef[1] = LowPt + CurJob->OneDEMResX + 1;
								Object3D->Polygons[Zip + 1].VertRef[2] = LowPt + CurJob->OneDEMResX;
								Object3D->Polygons[Zip + 1].Material = 0;
								} // for
							} // for
						} // if
					AppMem_Free(ElevBuf, RowSize);
					} // if
				} // if allocate
			InstancePtr = &CurInstance->Next;
			} // if
		else
			{
			Success = 0;
			break;
			} // else
		} // if
	TerObjNumber ++;
	} // while
if (CurJob->ObjectInstanceList && TerrainObjectList)
	{
	CurInstance = CurJob->ObjectInstanceList;
	while (CurInstance->Next)
		{
		CurInstance = CurInstance->Next;
		} // if
	CurInstance->Next = TerrainObjectList;
	} // if
else if (TerrainObjectList)
	{
	CurJob->ObjectInstanceList = TerrainObjectList;
	} // else if
TerrainObjectList = NULL;

while ((*FileNamesCreated) && (DEMFileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN)))
	{
	RemoveNameList(FileNamesCreated, (char *)DEMFileNameOfKnownType, WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN);
	} // while

return (Success);

} // ExportControlGUI::ExportTerrainAs3DObject

/*===========================================================================*/

// creates a 3d object for sky if needed and adds it to the 3d object instance list
int ExportControlGUI::ExportSkyAs3DObject(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double CubeWidth, CubeHeight, CubeElev, CellWE, CellNS;
long PolyCt, SkyMatNum, VertRefBlockIdx;
MaterialEffect *SkyMat[6], *CurMat;
Object3DEffect *Object3D;
RootTexture *CurTexRoot;
UVImageTexture *CurUVTex;
Raster *UVRaster;
Object3DInstance **InstancePtr, *CurInstance, *SkyObjectList = NULL;
const char *SkyFileName[6], *OutputFilePath;
char *ImageExtension;
int Success = 1, SkyFileType[6];
char TempName[256], FullOriginalPath[512], FullNewPath[512];

SkyFileType[0] = WCS_EXPORTCONTROL_FILETYPE_SKYNORTH;
SkyFileType[1] = WCS_EXPORTCONTROL_FILETYPE_SKYEAST;
SkyFileType[2] = WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH;
SkyFileType[3] = WCS_EXPORTCONTROL_FILETYPE_SKYWEST;
SkyFileType[4] = WCS_EXPORTCONTROL_FILETYPE_SKYTOP;
SkyFileType[5] = WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM;

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->TextureImageFormat);
OutputFilePath = CurJob->OutPath.GetPath();
InstancePtr = &SkyObjectList;

// cell sizes will either be geographic or metric depending on the coord sys of the exporter.
// ExportLat and LonScale will match.
CurJob->RBounds.DeriveCoords(CurJob->DEMResY, CurJob->DEMResX);
if (CurJob->RBounds.FetchCellSizeGIS(CellNS, CellWE))
	{
	CubeWidth = fabs(CellWE * (CurJob->DEMResX - 1) * CurJob->ExportRefData.ExportLonScale);
	CubeHeight = fabs(CellNS * (CurJob->DEMResY - 1) * CurJob->ExportRefData.ExportLatScale);
	CubeElev = fabs(CurJob->ExportRefData.MaxElev - CurJob->ExportRefData.RefElev);
	CubeWidth = max(CubeWidth, CubeHeight);
	CubeWidth = max(CubeWidth, CubeElev);
	CubeWidth *= .55 * 100.0;	// .55 is just large enough to surround the scene, *100 makes it far enough away 
								// that clouds and celestial objects appear at the right position relative to foreground
	} // if
else
	{
	return (0);
	} // else

// create a cube object
if (Object3D = new Object3DEffect)
	{
	CurJob->Unique3DObjectInstances ++;
	SkyMatNum = 0;
	CurJob->AddObjectToEphemeralList(Object3D);

	strcpy(Object3D->Name, "Sky");
	strcpy(Object3D->ShortName, Object3D->Name);
	
	// some beneficial settings
	Object3D->Absolute = WCS_EFFECT_ABSOLUTE;
	Object3D->DrawEnabled = 0;
	Object3D->CastShadows = Object3D->ReceiveShadowsTerrain = Object3D->ReceiveShadowsFoliage = 
		Object3D->ReceiveShadows3DObject = Object3D->ReceiveShadowsCloudSM = Object3D->ReceiveShadowsVolumetric = 0;

	// set number of polygons and vertices
	Object3D->NumVertices = 24;
	Object3D->NumPolys = 12;
	Object3D->NumMaterials = 6;

	// allocate polys and verts
	if ((Object3D->Vertices = new Vertex3D[Object3D->NumVertices]) && 
		(Object3D->Polygons = new Polygon3D[Object3D->NumPolys]) &&
		(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials]) &&
		Object3D->AllocUVWMaps(1) && Object3D->UVWTable[0].AllocMap(Object3D->NumVertices)
		&& Object3D->AllocVertRef(Object3D->NumPolys * 3))
		{
		VertRefBlockIdx = 0;
		Object3D->VertexUVWAvailable = 1;

		for (SkyMatNum = 0; SkyMatNum < 6; SkyMatNum ++)
			{
			// sky files should all exist or there has been an error
			if (SkyFileName[SkyMatNum] = (*FileNamesCreated)->FindNameOfType(SkyFileType[SkyMatNum]))
				{
				if (SkyMat[SkyMatNum] = new MaterialEffect(NULL))
					{
					SkyMat[SkyMatNum]->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
					SkyMat[SkyMatNum]->SpecularColor.SetValue3(1.0, 1.0, 1.0);
					SkyMat[SkyMatNum]->SpecularColor2.SetValue3(1.0, 1.0, 1.0);
					CurMat = SkyMat[SkyMatNum];
					CurJob->AddObjectToEphemeralList(CurMat);
					// name it uniquely
					sprintf(CurMat->Name, "SM%06d", SkyMatNum);
					strcpy(CurMat->ShortName, CurMat->Name);
					strcpy(Object3D->NameTable[SkyMatNum].Name, CurMat->Name);
					Object3D->NameTable[SkyMatNum].Mat = CurMat;
					// set material properties
					CurMat->DoubleSided = 1;
					CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
					CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
					// create texture
					if (CurTexRoot = CurMat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
						{
						if(CurUVTex = (UVImageTexture *)CurTexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_UVW))
							{
							CurUVTex->SetVertexMap(Object3D->Name);
							// change file name of raster to material name which is unique, add extension
							/// this needs to change the image name to something short
							// needs file extension to match the one saved
							strmfp(FullOriginalPath, (char *)OutputFilePath, (char *)SkyFileName[SkyMatNum]);
							strcpy(TempName, CurMat->Name);
							if (ImageExtension)
								AddExtension(TempName, ImageExtension);
							strmfp(FullNewPath, (char *)OutputFilePath, TempName);
							PROJ_remove(FullNewPath);
							PROJ_rename(FullOriginalPath, FullNewPath);
							RemoveNameList(FileNamesCreated, (char *)SkyFileName[SkyMatNum], SkyFileType[SkyMatNum]);
							AddNewNameList(FileNamesCreated, TempName, SkyFileType[SkyMatNum]);
							if (UVRaster = ImageHost->AddRaster((char *)OutputFilePath, TempName, 0, 1, 1))
								{
								UVRaster->AlphaEnabled = UVRaster->AlphaAvailable;
								CurUVTex->SetRaster(UVRaster);
								CurJob->AddObjectToEphemeralList(UVRaster);
								} // if
							else
								{
								Success = 0;
								break;
								} // if
							} // if
						else
							{
							Success = 0;
							break;
							} // if
						} // if
					else
						{
						Success = 0;
						break;
						} // if
					} // if
				else
					{
					Success = 0;
					break;
					} // if
				} // if sky file
			else
				{
				Success = 0;
				break;
				} // if
			} // for

		if (Success)
			{
			Object3D->UVWTable[0].SetName(Object3D->Name);

			// north of cube
			// vertex 0
			Object3D->Vertices[0].xyz[0] = -CubeWidth;
			Object3D->Vertices[0].xyz[1] = -CubeWidth;
			Object3D->Vertices[0].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[0] = 1;
			Object3D->UVWTable[0].CoordsArray[0][0] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][0] = 0.0f;	// bottom of image
			// vertex 1
			Object3D->Vertices[1].xyz[0] = -CubeWidth;
			Object3D->Vertices[1].xyz[1] = CubeWidth;
			Object3D->Vertices[1].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[1] = 1;
			Object3D->UVWTable[0].CoordsArray[0][1] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][1] = 1.0f;	// top of image
			// vertex 2
			Object3D->Vertices[2].xyz[0] = CubeWidth;
			Object3D->Vertices[2].xyz[1] = CubeWidth;
			Object3D->Vertices[2].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[2] = 1;
			Object3D->UVWTable[0].CoordsArray[0][2] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][2] = 1.0f;	// top of image
			// vertex 3
			Object3D->Vertices[3].xyz[0] = CubeWidth;
			Object3D->Vertices[3].xyz[1] = -CubeWidth;
			Object3D->Vertices[3].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[3] = 1;
			Object3D->UVWTable[0].CoordsArray[0][3] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][3] = 0.0f;	// bottom of image

			// east of cube
			// vertex 4
			Object3D->Vertices[4].xyz[0] = CubeWidth;
			Object3D->Vertices[4].xyz[1] = -CubeWidth;
			Object3D->Vertices[4].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[4] = 1;
			Object3D->UVWTable[0].CoordsArray[0][4] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][4] = 0.0f;	// bottom of image
			// vertex 5
			Object3D->Vertices[5].xyz[0] = CubeWidth;
			Object3D->Vertices[5].xyz[1] = CubeWidth;
			Object3D->Vertices[5].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[5] = 1;
			Object3D->UVWTable[0].CoordsArray[0][5] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][5] = 1.0f;	// top of image
			// vertex 6
			Object3D->Vertices[6].xyz[0] = CubeWidth;
			Object3D->Vertices[6].xyz[1] = CubeWidth;
			Object3D->Vertices[6].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[6] = 1;
			Object3D->UVWTable[0].CoordsArray[0][6] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][6] = 1.0f;	// top of image
			// vertex 7
			Object3D->Vertices[7].xyz[0] = CubeWidth;
			Object3D->Vertices[7].xyz[1] = -CubeWidth;
			Object3D->Vertices[7].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[7] = 1;
			Object3D->UVWTable[0].CoordsArray[0][7] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][7] = 0.0f;	// bottom of image

			// south of cube
			// vertex 8
			Object3D->Vertices[8].xyz[0] = CubeWidth;
			Object3D->Vertices[8].xyz[1] = -CubeWidth;
			Object3D->Vertices[8].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[8] = 1;
			Object3D->UVWTable[0].CoordsArray[0][8] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][8] = 0.0f;	// bottom of image
			// vertex 9
			Object3D->Vertices[9].xyz[0] = CubeWidth;
			Object3D->Vertices[9].xyz[1] = CubeWidth;
			Object3D->Vertices[9].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[9] = 1;
			Object3D->UVWTable[0].CoordsArray[0][9] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][9] = 1.0f;	// top of image
			// vertex 10
			Object3D->Vertices[10].xyz[0] = -CubeWidth;
			Object3D->Vertices[10].xyz[1] = CubeWidth;
			Object3D->Vertices[10].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[10] = 1;
			Object3D->UVWTable[0].CoordsArray[0][10] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][10] = 1.0f;	// top of image
			// vertex 11
			Object3D->Vertices[11].xyz[0] = -CubeWidth;
			Object3D->Vertices[11].xyz[1] = -CubeWidth;
			Object3D->Vertices[11].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[11] = 1;
			Object3D->UVWTable[0].CoordsArray[0][11] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][11] = 0.0f;	// bottom of image

			// west of cube
			// vertex 12
			Object3D->Vertices[12].xyz[0] = -CubeWidth;
			Object3D->Vertices[12].xyz[1] = -CubeWidth;
			Object3D->Vertices[12].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[12] = 1;
			Object3D->UVWTable[0].CoordsArray[0][12] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][12] = 0.0f;	// bottom of image
			// vertex 13
			Object3D->Vertices[13].xyz[0] = -CubeWidth;
			Object3D->Vertices[13].xyz[1] = CubeWidth;
			Object3D->Vertices[13].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[13] = 1;
			Object3D->UVWTable[0].CoordsArray[0][13] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][13] = 1.0f;	// top of image
			// vertex 14
			Object3D->Vertices[14].xyz[0] = -CubeWidth;
			Object3D->Vertices[14].xyz[1] = CubeWidth;
			Object3D->Vertices[14].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[14] = 1;
			Object3D->UVWTable[0].CoordsArray[0][14] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][14] = 1.0f;	// top of image
			// vertex 15
			Object3D->Vertices[15].xyz[0] = -CubeWidth;
			Object3D->Vertices[15].xyz[1] = -CubeWidth;
			Object3D->Vertices[15].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[15] = 1;
			Object3D->UVWTable[0].CoordsArray[0][15] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][15] = 0.0f;	// bottom of image

			// top of cube
			// vertex 16
			Object3D->Vertices[16].xyz[0] = -CubeWidth;
			Object3D->Vertices[16].xyz[1] = CubeWidth;
			Object3D->Vertices[16].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[16] = 1;
			Object3D->UVWTable[0].CoordsArray[0][16] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][16] = 0.0f;	// bottom of image
			// vertex 17
			Object3D->Vertices[17].xyz[0] = -CubeWidth;
			Object3D->Vertices[17].xyz[1] = CubeWidth;
			Object3D->Vertices[17].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[17] = 1;
			Object3D->UVWTable[0].CoordsArray[0][17] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][17] = 1.0f;	// top of image
			// vertex 18
			Object3D->Vertices[18].xyz[0] = CubeWidth;
			Object3D->Vertices[18].xyz[1] = CubeWidth;
			Object3D->Vertices[18].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[18] = 1;
			Object3D->UVWTable[0].CoordsArray[0][18] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][18] = 1.0f;	// top of image
			// vertex 19
			Object3D->Vertices[19].xyz[0] = CubeWidth;
			Object3D->Vertices[19].xyz[1] = CubeWidth;
			Object3D->Vertices[19].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[19] = 1;
			Object3D->UVWTable[0].CoordsArray[0][19] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][19] = 0.0f;	// bottom of image

			// bottom of cube
			// vertex 20
			Object3D->Vertices[20].xyz[0] = -CubeWidth;
			Object3D->Vertices[20].xyz[1] = -CubeWidth;
			Object3D->Vertices[20].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[20] = 1;
			Object3D->UVWTable[0].CoordsArray[0][20] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][20] = 0.0f;	// bottom of image
			// vertex 21
			Object3D->Vertices[21].xyz[0] = -CubeWidth;
			Object3D->Vertices[21].xyz[1] = -CubeWidth;
			Object3D->Vertices[21].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[21] = 1;
			Object3D->UVWTable[0].CoordsArray[0][21] = 0.0f;
			Object3D->UVWTable[0].CoordsArray[1][21] = 1.0f;	// top of image
			// vertex 22
			Object3D->Vertices[22].xyz[0] = CubeWidth;
			Object3D->Vertices[22].xyz[1] = -CubeWidth;
			Object3D->Vertices[22].xyz[2] = CubeWidth;
			Object3D->UVWTable[0].CoordsValid[22] = 1;
			Object3D->UVWTable[0].CoordsArray[0][22] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][22] = 1.0f;	// top of image
			// vertex 23
			Object3D->Vertices[23].xyz[0] = CubeWidth;
			Object3D->Vertices[23].xyz[1] = -CubeWidth;
			Object3D->Vertices[23].xyz[2] = -CubeWidth;
			Object3D->UVWTable[0].CoordsValid[23] = 1;
			Object3D->UVWTable[0].CoordsArray[0][23] = 1.0f;
			Object3D->UVWTable[0].CoordsArray[1][23] = 0.0f;	// bottom of image


			// allocate polygon reference numbers
			for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt ++)
				{
				Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
				VertRefBlockIdx += 3; // We just used 3 longs
				Object3D->Polygons[PolyCt].NumVerts = 3;
				} // for
			// north
			// polygon 0
			Object3D->Polygons[0].VertRef[0] = 0;
			Object3D->Polygons[0].VertRef[1] = 1;
			Object3D->Polygons[0].VertRef[2] = 2;
			// polygon 1
			Object3D->Polygons[1].VertRef[0] = 0;
			Object3D->Polygons[1].VertRef[1] = 2;
			Object3D->Polygons[1].VertRef[2] = 3;

			// east
			// polygon 2
			Object3D->Polygons[2].VertRef[0] = 4;
			Object3D->Polygons[2].VertRef[1] = 5;
			Object3D->Polygons[2].VertRef[2] = 6;
			// polygon 3
			Object3D->Polygons[3].VertRef[0] = 4;
			Object3D->Polygons[3].VertRef[1] = 6;
			Object3D->Polygons[3].VertRef[2] = 7;

			// south
			// polygon 2
			Object3D->Polygons[4].VertRef[0] = 8;
			Object3D->Polygons[4].VertRef[1] = 9;
			Object3D->Polygons[4].VertRef[2] = 10;
			// polygon 3
			Object3D->Polygons[5].VertRef[0] = 8;
			Object3D->Polygons[5].VertRef[1] = 10;
			Object3D->Polygons[5].VertRef[2] = 11;

			// west
			// polygon 2
			Object3D->Polygons[6].VertRef[0] = 12;
			Object3D->Polygons[6].VertRef[1] = 13;
			Object3D->Polygons[6].VertRef[2] = 14;
			// polygon 3
			Object3D->Polygons[7].VertRef[0] = 12;
			Object3D->Polygons[7].VertRef[1] = 14;
			Object3D->Polygons[7].VertRef[2] = 15;

			// top
			// polygon 2
			Object3D->Polygons[8].VertRef[0] = 16;
			Object3D->Polygons[8].VertRef[1] = 17;
			Object3D->Polygons[8].VertRef[2] = 18;
			// polygon 3
			Object3D->Polygons[9].VertRef[0] = 16;
			Object3D->Polygons[9].VertRef[1] = 18;
			Object3D->Polygons[9].VertRef[2] = 19;

			// bottom
			// polygon 2
			Object3D->Polygons[10].VertRef[0] = 20;
			Object3D->Polygons[10].VertRef[1] = 21;
			Object3D->Polygons[10].VertRef[2] = 22;
			// polygon 3
			Object3D->Polygons[11].VertRef[0] = 20;
			Object3D->Polygons[11].VertRef[1] = 22;
			Object3D->Polygons[11].VertRef[2] = 23;

			Object3D->Polygons[0].Material = 0;
			Object3D->Polygons[1].Material = 0;
			Object3D->Polygons[2].Material = 1;
			Object3D->Polygons[3].Material = 1;
			Object3D->Polygons[4].Material = 2;
			Object3D->Polygons[5].Material = 2;
			Object3D->Polygons[6].Material = 3;
			Object3D->Polygons[7].Material = 3;
			Object3D->Polygons[8].Material = 4;
			Object3D->Polygons[9].Material = 4;
			Object3D->Polygons[10].Material = 5;
			Object3D->Polygons[11].Material = 5;

			Object3D->GetObjectBounds();
			
			if (*InstancePtr = new Object3DInstance)
				{
				CurInstance = *InstancePtr;
				CurInstance->MyObj = Object3D;
				// need the WCS geographic coords here
				CurInstance->WCSGeographic[0] = CurJob->ExportRefData.WCSRefLon;
				CurInstance->WCSGeographic[1] = CurJob->ExportRefData.WCSRefLat;
				CurInstance->WCSGeographic[2] = CurJob->ExportRefData.RefElev;
				InstancePtr = &CurInstance->Next;
				} // if
			else
				{
				Success = 0;
				} // else
			} // if allocation
		else
			{
			Success = 0;
			} // else
		} // if material success
	} // if object3d

if (CurJob->ObjectInstanceList && SkyObjectList)
	{
	CurInstance = CurJob->ObjectInstanceList;
	while (CurInstance->Next)
		{
		CurInstance = CurInstance->Next;
		} // if
	CurInstance->Next = SkyObjectList;
	} // if
else if (SkyObjectList)
	{
	CurJob->ObjectInstanceList = SkyObjectList;
	} // else if
SkyObjectList = NULL;

return (Success);

} // ExportControlGUI::ExportSkyAs3DObject

/*===========================================================================*/

// creates 3d objects for vectors if needed and adds them to the 3d object instance list
int ExportControlGUI::ExportVectorsAs3DObjects(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double MatColor[3], VecCenterNS, VecCenterEW, VecRefElev, VecNULL[2], VecBack[2], VecForward[2], LastVertxyz[2],
	VecSectrix[2], VecLen, VecLenBack, VecLenForward, VecWidth, BisectrixAngleHalved, Multiplier, Area;
long VecCt, MatCt, VertCt, PolyCt, VertRefBlockIdx, NumMatsMade = 0;
Joe *CurVec;
MaterialEffect *CurMat, *MadeMat, **MatList;
Object3DEffect *Object3D;
Object3DInstance *VecObjectList = NULL, **InstancePtr, *CurInstance;
VectorPoint *PLink;
int Success = 1, MakeLines, SkipPoint, ProblemSolved;

InstancePtr = &VecObjectList;
VecNULL[0] = VecNULL[1] = 0.0;

if (CurJob->VecInstanceList)
	{
	if (MatList = (MaterialEffect **)AppMem_Alloc(CurJob->NumVecInstances * sizeof (MaterialEffect *), APPMEM_CLEAR))
		{
		// for each vector list item
		// convert to 3d geometry
		for (VecCt = 0; VecCt < CurJob->NumVecInstances && Success; VecCt ++)
			{
			CurVec = CurJob->VecInstanceList[VecCt].MyJoe;
			// create 3d object
			if (Object3D = new Object3DEffect)
				{
				CurJob->Unique3DObjectInstances ++;
				// add to ephemeral list
				CurJob->AddObjectToEphemeralList(Object3D);

				sprintf(Object3D->Name, "VO%06d", VecCt);
				strcpy(Object3D->ShortName, Object3D->Name);

				MatColor[0] = CurVec->Red() / 255.0;
				MatColor[1] = CurVec->Green() / 255.0;
				MatColor[2] = CurVec->Blue() / 255.0;

				// check the vector material list to see if there is one already with the same color
				CurMat = MadeMat = NULL;
				MakeLines = 1;
				for (MatCt = 0; MatCt < NumMatsMade; MatCt ++)
					{
					if ((fabs(MatList[MatCt]->DiffuseColor.CurValue[0] - MatColor[0]) < .001)
						&& (fabs(MatList[MatCt]->DiffuseColor.CurValue[1] - MatColor[1]) < .001)
						&& (fabs(MatList[MatCt]->DiffuseColor.CurValue[2] - MatColor[2]) < .001))
						CurMat = MatList[MatCt];
					} // for
				
				// create a new material
				if (CurMat || (CurMat = (MadeMat = new MaterialEffect(NULL))))
					{
					if (MadeMat)
						{
						CurMat->DiffuseColor.SetValue3(MatColor[0], MatColor[1], MatColor[2]);
						CurMat->SpecularColor.SetValue3(MatColor[0], MatColor[1], MatColor[2]);
						CurMat->SpecularColor2.SetValue3(MatColor[0], MatColor[1], MatColor[2]);
						// add to ephemeral list
						CurJob->AddObjectToEphemeralList(CurMat);
						// name it uniquely
						sprintf(CurMat->Name, "VM%06d", NumMatsMade);
						strcpy(CurMat->ShortName, CurMat->Name);

						// set material properties
						CurMat->DoubleSided = 1;
						CurMat->Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
						CurMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue(1.0);
						MatList[NumMatsMade ++] = CurMat;
						} // if

					// allocate vertices and polygons and 1 material table entry
					Object3D->NumMaterials = 1;
					// for 2pt polygons
					Object3D->NumPolys = CurJob->VecInstanceList[VecCt].NumPoints - 1;
					Object3D->NumVertices = CurJob->VecInstanceList[VecCt].NumPoints;
					// for 3pt polygons there will be two vertices for every point
					if (CurVec->GetLineStyle() < 4 || CurJob->VecInstanceList[VecCt].NumPoints < 2)
						{
						MakeLines = 0;
						Object3D->NumVertices *= 4;
						Object3D->NumPolys += 1;
						Object3D->NumPolys *= 2;
						} // if
					else if (CurJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS)
						{
						Object3D->NumVertices *= 2;
						Object3D->NumPolys *= 2;
						} // if
					// find vector width
					VecWidth = .5 * CurVec->GetLineWidth() * CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT].CurValue;

					if ((Object3D->Vertices = new Vertex3D[Object3D->NumVertices]) && 
						(Object3D->Polygons = new Polygon3D[Object3D->NumPolys]) &&
						(Object3D->NameTable = new ObjectMaterialEntry[Object3D->NumMaterials]) &&
						Object3D->AllocVertRef(Object3D->NumPolys * (CurJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_3PTPOLYS ? 3: 2)))
						{
						VertRefBlockIdx = 0;
						strcpy(Object3D->NameTable[0].Name, CurMat->Name);
						Object3D->NameTable[0].Mat = CurMat;

						// create object instance and set its coordinates
						if (*InstancePtr = new Object3DInstance)
							{
							CurInstance = *InstancePtr;
							CurInstance->MyObj = Object3D;
							// WCS geographic coords
							PLink = CurJob->VecInstanceList[VecCt].Points;
							// find vector center
							VecCenterNS = CurVec->GetNSCenter();
							VecCenterEW = CurVec->GetEWCenter();
							VecRefElev = PLink->Elevation;
							CurInstance->WCSGeographic[0] = VecCenterEW;
							CurInstance->WCSGeographic[1] = VecCenterNS;
							CurInstance->WCSGeographic[2] = VecRefElev;
							#ifdef WCS_BUILD_SX2
							if (CurVec->IsSXClickQueryEnabled())
								CurInstance->ClickQueryObjectID = CurVec->GetSXQueryRecordNumber(FileNamesCreated);
							#endif // WCS_BUILD_SX2
							InstancePtr = &CurInstance->Next;
							Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT].SetValue(VecCenterNS);
							Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON].SetValue(VecCenterEW);
							// make vertices and polygons
							if (MakeLines)
								{
								if (CurJob->VectorExpType == WCS_EFFECTS_SCENEEXPORTER_VECTYPE_2PTPOLYS)
									{
									// copy vertices with correct offsets
									for (VertCt = 0; PLink && VertCt < Object3D->NumVertices; PLink = PLink->Next, VertCt ++)
										{
										// coords are in default lat/lon
										// convert to meters as offsets from center point
										Object3D->Vertices[VertCt].xyz[0] = (VecCenterEW - PLink->Longitude) * CurJob->ExportRefData.WCSLonScale;
										Object3D->Vertices[VertCt].xyz[1] = (PLink->Elevation - VecRefElev);
										Object3D->Vertices[VertCt].xyz[2] = (PLink->Latitude - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
										} // for
									// make polygons
									for (VertCt = PolyCt = 0; VertCt < CurJob->VecInstanceList[VecCt].NumPoints - 1; VertCt ++, PolyCt ++)
										{
										Object3D->Polygons[PolyCt].NumVerts = 2;
										Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
										VertRefBlockIdx += 2; // We just used 2 longs
										Object3D->Polygons[PolyCt].VertRef[0] = VertCt;
										Object3D->Polygons[PolyCt].VertRef[1] = VertCt + 1;
										Object3D->Polygons[PolyCt].Material = 0;
										} // for
									} // if
								else
									{
									// copy vertices with correct offsets
									for (VertCt = 0; PLink && VertCt < Object3D->NumVertices; PLink = PLink->Next, VertCt += 2)
										{
										// coords are in default lat/lon
										// convert to meters as offsets from center point
										Object3D->Vertices[VertCt].xyz[0] = (VecCenterEW - PLink->Longitude) * CurJob->ExportRefData.WCSLonScale;
										Object3D->Vertices[VertCt].xyz[1] = (PLink->Elevation - VecRefElev);
										Object3D->Vertices[VertCt].xyz[2] = (PLink->Latitude - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
										Object3D->Vertices[VertCt + 1].xyz[0] = Object3D->Vertices[VertCt].xyz[0];
										Object3D->Vertices[VertCt + 1].xyz[1] = Object3D->Vertices[VertCt].xyz[1];
										Object3D->Vertices[VertCt + 1].xyz[2] = Object3D->Vertices[VertCt].xyz[2];
										} // for
									// project each point in both directions along the bisectrix
									for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt += 2)
										{
										SkipPoint = ProblemSolved = 0;
										// coords are in meters
										// find bisectrix vector
										// from the current point, find the vector to the points on either side
										if (VertCt > 1)
											{
											VecBack[0] = LastVertxyz[0] - Object3D->Vertices[VertCt].xyz[0];
											VecBack[1] = LastVertxyz[1] - Object3D->Vertices[VertCt].xyz[2];
											} // if
										else
											{
											VecBack[0] = VecBack[1] = 0.0;
											} // else
										if (VertCt < Object3D->NumVertices - 2)
											{
											VecForward[0] = Object3D->Vertices[VertCt + 2].xyz[0] - Object3D->Vertices[VertCt].xyz[0];
											VecForward[1] = Object3D->Vertices[VertCt + 2].xyz[2] - Object3D->Vertices[VertCt].xyz[2];
											} // if
										else
											{
											VecForward[0] = VecForward[1] = 0.0;
											} // else
										// normalize those vectors
										if ((VecLenBack = sqrt(VecBack[0] * VecBack[0] + VecBack[1] * VecBack[1])) > 0.0001)
											{
											VecBack[0] /= VecLenBack;
											VecBack[1] /= VecLenBack;
											} // if
										if ((VecLenForward = sqrt(VecForward[0] * VecForward[0] + VecForward[1] * VecForward[1])) > 0.0001)
											{
											VecForward[0] /= VecLenForward;
											VecForward[1] /= VecLenForward;
											} // if
										// find their average and normalize it
										if (VecLenBack > 0.0 && VecLenForward > 0.0)
											{
											VecSectrix[0] = (VecBack[0] + VecForward[0]) * .5;
											VecSectrix[1] = (VecBack[1] + VecForward[1]) * .5;
											if ((VecLen = sqrt(VecSectrix[0] * VecSectrix[0] + VecSectrix[1] * VecSectrix[1])) > .0001)
												{
												VecSectrix[0] /= VecLen;
												VecSectrix[1] /= VecLen;
												BisectrixAngleHalved = acos(VecLen);
												Multiplier = sin(BisectrixAngleHalved);
												if (Multiplier > 0.0001)
													{
													Multiplier = 1.0 / Multiplier;
													Area = SolveTriangleArea2D(VecBack, VecNULL, VecSectrix);
													ProblemSolved = 1;
													if (Multiplier * VecWidth > VecLenForward)
														Multiplier = VecLenForward / VecWidth;
													} // if
												} // if
											} // if
										if (! ProblemSolved)
											{
											if (VecLenForward > 0.0)
												{
												VecSectrix[0] = VecForward[1];
												VecSectrix[1] = -VecForward[0];
												Area = SolveTriangleArea2D(VecNULL, VecForward, VecSectrix);
												Multiplier = 1.0;
												} // else if
											else if (VecLenBack > 0.0)
												{
												VecSectrix[0] = VecBack[1];
												VecSectrix[1] = -VecBack[0];
												Area = SolveTriangleArea2D(VecBack, VecNULL, VecSectrix);
												Multiplier = 1.0;
												} // else if
											else
												{
												// just skip this point
												SkipPoint = 1;
												} // else
											} // if
										LastVertxyz[0] = Object3D->Vertices[VertCt].xyz[0];
										LastVertxyz[1] = Object3D->Vertices[VertCt].xyz[2];
										if (! SkipPoint)
											{
											// make the bisectrix point to the right when looking from start to end of vector segment
											// which way does it point now?
											// form a triangle between last point, this point and bisectrix + this point
											if (Area < 0.0)
												{
												VecSectrix[0] = -VecSectrix[0];
												VecSectrix[1] = -VecSectrix[1];
												} // if
											// offset one set of vertices one direction and the other the opposite
											Object3D->Vertices[VertCt].xyz[0] += VecWidth * Multiplier * VecSectrix[0];
											Object3D->Vertices[VertCt].xyz[2] += VecWidth * Multiplier * VecSectrix[1];
											Object3D->Vertices[VertCt + 1].xyz[0] -= VecWidth * Multiplier * VecSectrix[0];
											Object3D->Vertices[VertCt + 1].xyz[2] -= VecWidth * Multiplier * VecSectrix[1];
											} // if
										} // for
									// make 2 polygons for each segment
									for (VertCt = PolyCt = 0; VertCt < Object3D->NumVertices && PolyCt < Object3D->NumPolys; VertCt += 2)
										{
										Object3D->Polygons[PolyCt].NumVerts = 3;
										Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
										VertRefBlockIdx += 3; // We just used 3 longs
										Object3D->Polygons[PolyCt].VertRef[0] = VertCt;
										Object3D->Polygons[PolyCt].VertRef[1] = VertCt + 2;
										Object3D->Polygons[PolyCt].VertRef[2] = VertCt + 1;
										Object3D->Polygons[PolyCt ++].Material = 0;

										Object3D->Polygons[PolyCt].NumVerts = 3;
										Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
										VertRefBlockIdx += 3; // We just used 3 longs
										Object3D->Polygons[PolyCt].VertRef[0] = VertCt + 3;
										Object3D->Polygons[PolyCt].VertRef[1] = VertCt + 1;
										Object3D->Polygons[PolyCt].VertRef[2] = VertCt + 2;
										Object3D->Polygons[PolyCt ++].Material = 0;
										} // for
									} // else
								} // if lines
							else
								{
								// copy vertices with correct offsets
								for (VertCt = 0; PLink && VertCt < Object3D->NumVertices; PLink = PLink->Next, VertCt += 4)
									{
									// coords are in default lat/lon
									// convert to meters as offsets from center point
									Object3D->Vertices[VertCt].xyz[0] = (VecCenterEW - PLink->Longitude) * CurJob->ExportRefData.WCSLonScale;
									Object3D->Vertices[VertCt].xyz[1] = (PLink->Elevation - VecRefElev);
									Object3D->Vertices[VertCt].xyz[2] = (PLink->Latitude - VecCenterNS) * CurJob->ExportRefData.WCSLatScale;
									Object3D->Vertices[VertCt + 1].xyz[0] = Object3D->Vertices[VertCt].xyz[0];
									Object3D->Vertices[VertCt + 1].xyz[1] = Object3D->Vertices[VertCt].xyz[1];
									Object3D->Vertices[VertCt + 1].xyz[2] = Object3D->Vertices[VertCt].xyz[2];
									Object3D->Vertices[VertCt + 2].xyz[0] = Object3D->Vertices[VertCt].xyz[0];
									Object3D->Vertices[VertCt + 2].xyz[1] = Object3D->Vertices[VertCt].xyz[1];
									Object3D->Vertices[VertCt + 2].xyz[2] = Object3D->Vertices[VertCt].xyz[2];
									Object3D->Vertices[VertCt + 3].xyz[0] = Object3D->Vertices[VertCt].xyz[0];
									Object3D->Vertices[VertCt + 3].xyz[1] = Object3D->Vertices[VertCt].xyz[1];
									Object3D->Vertices[VertCt + 3].xyz[2] = Object3D->Vertices[VertCt].xyz[2];
									Object3D->Vertices[VertCt].xyz[0] += VecWidth;
									Object3D->Vertices[VertCt].xyz[2] += VecWidth;
									Object3D->Vertices[VertCt + 1].xyz[0] -= VecWidth;
									Object3D->Vertices[VertCt + 1].xyz[2] += VecWidth;
									Object3D->Vertices[VertCt + 2].xyz[0] -= VecWidth;
									Object3D->Vertices[VertCt + 2].xyz[2] -= VecWidth;
									Object3D->Vertices[VertCt + 3].xyz[0] += VecWidth;
									Object3D->Vertices[VertCt + 3].xyz[2] -= VecWidth;
									} // for
								// make 2 polygons for each vertex
								for (VertCt = PolyCt = 0; VertCt < Object3D->NumVertices && PolyCt < Object3D->NumPolys; VertCt += 4)
									{
									Object3D->Polygons[PolyCt].NumVerts = 3;
									Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
									VertRefBlockIdx += 3; // We just used 3 longs
									Object3D->Polygons[PolyCt].VertRef[0] = VertCt;
									Object3D->Polygons[PolyCt].VertRef[1] = VertCt + 1;
									Object3D->Polygons[PolyCt].VertRef[2] = VertCt + 2;
									Object3D->Polygons[PolyCt ++].Material = 0;

									Object3D->Polygons[PolyCt].NumVerts = 3;
									Object3D->Polygons[PolyCt].VertRef = &Object3D->VertRefBlock[VertRefBlockIdx];
									VertRefBlockIdx += 3; // We just used 3 longs
									Object3D->Polygons[PolyCt].VertRef[0] = VertCt + 2;
									Object3D->Polygons[PolyCt].VertRef[1] = VertCt + 3;
									Object3D->Polygons[PolyCt].VertRef[2] = VertCt;
									Object3D->Polygons[PolyCt ++].Material = 0;
									} // for
								} // else
							} // if
						else
							Success = 0;
						} // if vertices, polygons, etc
					} // if material
				else
					Success = 0;
				} // if
			else
				Success = 0;
			} // for
		AppMem_Free(MatList, CurJob->NumVecInstances * sizeof (MaterialEffect *));
		if (CurJob->ObjectInstanceList && VecObjectList)
			{
			CurInstance = CurJob->ObjectInstanceList;
			while (CurInstance->Next)
				{
				CurInstance = CurInstance->Next;
				} // if
			CurInstance->Next = VecObjectList;
			} // if
		else if (VecObjectList)
			{
			CurJob->ObjectInstanceList = VecObjectList;
			} // else if
		VecObjectList = NULL;
		} // if
	else
		Success = 0;

	// delete vector list
	CurJob->DeleteVectorInstanceList();
	} // if

return (Success);

} // ExportControlGUI::ExportVectorsAs3DObjects

/*===========================================================================*/

// initializes items for export such as cameras, lights and atmospheres
// set items to export time before init.
int ExportControlGUI::PrepExportItems(SceneExporter *CurJob, NameList **FileNamesCreated)
{
EffectList *CurItem;
Camera *CurCamera, *CameraCreated = NULL;
int Success = 1;
RenderData RendData(NULL);

// see if there is a camera to use, otherwise create one
if (CurJob->Cameras)
	CurCamera = (Camera *)CurJob->Cameras->Me;
else
	CameraCreated = CurCamera = new Camera(NULL);

if (CurCamera)
	{
	if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, CurCamera, 320, 320))
		{
		if (CurJob->ExportHaze)
			{
			CurItem = CurJob->Haze;
			while (CurItem)
				{
				if (CurItem->Me)
					{
					((Atmosphere *)CurItem->Me)->SetToTime(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
					((Atmosphere *)CurItem->Me)->InitToRender(NULL, NULL);
					((Atmosphere *)CurItem->Me)->InitFrameToRender(GlobalApp->AppEffects, &RendData);
					} // if
				CurItem = CurItem->Next;
				} // while
			} // if
		if (CurJob->ExportLights)
			{
			CurItem = CurJob->Lights;
			while (CurItem)
				{
				if (CurItem->Me)
					{
					((Light *)CurItem->Me)->SetToTime(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
					((Light *)CurItem->Me)->InitToRender(NULL, NULL);
					((Light *)CurItem->Me)->InitFrameToRender(GlobalApp->AppEffects, &RendData);
					} // if
				CurItem = CurItem->Next;
				} // while
			} // if
		if (CurJob->ExportCameras)
			{
			CurItem = CurJob->Cameras;
			while (CurItem)
				{
				if (CurItem->Me)
					{
					((Camera *)CurItem->Me)->SetToTime(CurJob->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue);
					((Camera *)CurItem->Me)->InitToRender(NULL, NULL);
					((Camera *)CurItem->Me)->InitFrameToRender(GlobalApp->AppEffects, &RendData);
					} // if
				CurItem = CurItem->Next;
				} // while
			} // if

		if (CameraCreated)
			delete CameraCreated;
		} // if
	else
		{
		Success = 0;
		ExportError("Prep Items for Export", "Error: Could not initialize data.");
		} // else
	} // if
else
	{
	Success = 0;
	ExportError("Prep Items for Export", "Error: Could not create temporary camera.");
	} // else

return (Success);

} // ExportControlGUI::PrepExportItems

/*===========================================================================*/

// convert all WCS geographic coordinates into the coord system of the exporter
int ExportControlGUI::ConvertWCSGeoToExportCoords(SceneExporter *CurJob, NameList **FileNamesCreated)
{
double PointLat, PointLon, PointX, PointY, PointZ;
float fPointX, fPointY, fPointZ;
long CurRecordStart, CurRecordEnd, FolObjCt, NumCells, VecCt; 
Object3DInstance *CurInstance;
VectorPoint *PLinkB;
FILE *FolDatFile;
int Success = 1, FlipLon = 0;
char FileName[256], FileVersion;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;

if (CurJob->ObjectInstanceList)
	{
	CurInstance = CurJob->ObjectInstanceList;
	while (CurInstance)
		{
		CurJob->RBounds.DefDegToRBounds(CurInstance->WCSGeographic[1], CurInstance->WCSGeographic[0],
			CurInstance->ExportXYZ[0], CurInstance->ExportXYZ[1]);
		CurInstance->ExportXYZ[0] -= CurJob->ExportRefData.ExportRefLon;
		CurInstance->ExportXYZ[1] -= CurJob->ExportRefData.ExportRefLat;
		CurInstance->ExportXYZ[2] = CurInstance->WCSGeographic[2] - CurJob->ExportRefData.RefElev;
		CurInstance = CurInstance->Next;
		} // if
	} // if

// if export format is VNS and export coord sys is geographic
if (! strcmp(CurJob->ExportTarget, "VNS") && (! CurJob->Coords || CurJob->Coords->GetGeographic()))
	FlipLon = true;

// convert foliage files too

// open foliage files for walkthrough in rb+ mode
if (FolDatFile = PrepareFoliageFileForWalkthrough(CurJob, &Index, 
	&RFCD, FileNamesCreated, NULL, "rb+"))
	{
	CurRecordEnd = ftell(FolDatFile);
	for (FolObjCt = 0; Success && FolObjCt < Index.CellDat->DatCt; FolObjCt ++)
		{
		CurRecordStart = CurRecordEnd;
		// read and interpret foliage record
		if (FolData.ReadFoliageRecord(FolDatFile, Index.FileVersion))
			{
			CurRecordEnd = ftell(FolDatFile);
			if (FolData.ElementID > 0 && FolData.InterpretFoliageRecord(EffectsHost, ImageHost, &PointData))
				{
				PointLon = FolData.XYZ[0] + Index.RefXYZ[0];
				PointLat = FolData.XYZ[1] + Index.RefXYZ[1];
				PointZ = FolData.XYZ[2] + Index.RefXYZ[2];
				// convert WCS default geographic to export coords
				CurJob->RBounds.DefDegToRBounds(PointLat, PointLon, PointX, PointY);
				// remove the reference for this export
				fPointX = (float)(PointX - CurJob->ExportRefData.ExportRefLon);
				fPointY = (float)(PointY - CurJob->ExportRefData.ExportRefLat);
				fPointZ = (float)(PointZ - CurJob->ExportRefData.RefElev);
				// rewrite the positions
				if (FlipLon)
					fPointX = -fPointX;
				if (! fseek(FolDatFile, CurRecordStart + sizeof (short), SEEK_SET))
					{
					fwrite((char *)&fPointX, sizeof (float), 1, FolDatFile);
					fwrite((char *)&fPointY, sizeof (float), 1, FolDatFile);
					if (fwrite((char *)&fPointZ, sizeof (float), 1, FolDatFile) != 1)
						Success = 0;
					if (fseek(FolDatFile, CurRecordEnd, SEEK_SET))
						Success = 0;
					} // if
				else
					Success = 0;
				} // if
			} // if
		} // for

	fclose(FolDatFile);

	if (Success)
		{
		if (FolDatFile = OpenFoliageIndexFile(CurJob, FileNamesCreated, "rb+"))
			{
			fgets(FileName, 256, FolDatFile);
			// version
			fread((char *)&FileVersion, sizeof (char), 1, FolDatFile);
			// number of files
			fread((char *)&NumCells, sizeof (long), 1, FolDatFile);
			// need an fseek in order to write after read
			fseek(FolDatFile, 0, SEEK_CUR);
			// rewrite reference XYZ
			if (FlipLon)
				CurJob->ExportRefData.ExportRefLon = -CurJob->ExportRefData.ExportRefLon;
			fwrite((char *)&CurJob->ExportRefData.ExportRefLon, sizeof (double), 1, FolDatFile);
			fwrite((char *)&CurJob->ExportRefData.ExportRefLat, sizeof (double), 1, FolDatFile);
			fwrite((char *)&CurJob->ExportRefData.RefElev, sizeof (double), 1, FolDatFile);
			fclose(FolDatFile);
			} // if
		} // if
	} // if

Index.CellDat = NULL;

// convert vector vertices
if (CurJob->NumVecInstances > 0)
	{
	if (CurJob->VecInstanceList)
		{
		for (VecCt = 0; VecCt < CurJob->NumVecInstances; VecCt ++)
			{
			// for each vertex
			for (PLinkB = CurJob->VecInstanceList[VecCt].Points; PLinkB; PLinkB = PLinkB->Next)
				{
				CurJob->RBounds.DefDegToRBounds(PLinkB->Latitude, PLinkB->Longitude,
					PLinkB->Longitude, PLinkB->Latitude);
				PLinkB->Longitude -= CurJob->ExportRefData.ExportRefLon;
				PLinkB->Latitude -= CurJob->ExportRefData.ExportRefLat;
				PLinkB->Elevation = (float)(PLinkB->Elevation - CurJob->ExportRefData.RefElev);
				} // for
			} // for
		} // if
	} // if

return (Success);

} // ExportControlGUI::ConvertWCSGeoToExportCoords

/*===========================================================================*/

// This function takes the files generated by the first steps of the export process and makes the final output products.
int ExportControlGUI::ResampleExports(SceneExporter *CurJob, NameList **FileNamesCreated)
{
long FileType, CurTileX, CurTileY;
int Success = 1;
const char *OutputFilePath, *FileNameOfKnownType, *FileNameOfKnownTypeR, *FileNameOfKnownTypeG, *FileNameOfKnownTypeB, *FileNameOfKnownTypeA;
char *BuffersToSave[8], *ImageExtension, *DEMExtension;
char FullNewPath[512], FullNewPathR[512], FullNewPathG[512], FullNewPathB[512], FullNewPathA[512], 
	FullOriginalPath[512], FullOriginalPathR[512], FullOriginalPathG[512], FullOriginalPathB[512], FullOriginalPathA[512], 
	FilePart[256], FilePartR[256], FilePartG[256], FilePartB[256], FilePartA[256], PathPart[512], CurrentOutputFile[256];
RenderOpt *CurOpt;

ImageExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->ImageFormat);
DEMExtension = ImageSaverLibrary::GetDefaultExtension(CurJob->DEMFormat);
SetFrameTextA("Resampling");

if (CurOpt = new RenderOpt())
	{
	// all files saved so far are in OutPath, all files that are going to be created are in OutPath as well.
	CurOpt->TempPath.Copy(&CurOpt->TempPath, &CurJob->OutPath);

	// Use CurJob to tell you what needs to be created, sizes needed and tiles
	// Use FileNamesCreated to tell you what files were created in the initial export process
	// Create new files for final product.
	// If it is a new file call AddNewNameList() with FinalNamesCreated, a new file name and a file type.
	// If it is a file type that is not yet defined, define it in ExportControlGUI.h
	// See ExportControlGUI.h for list of currently defined file types

	// The directory where all the files should be created is:
	OutputFilePath = CurJob->OutPath.GetPath();

	// TERRAIN
	// a method can be called on the root NameList to get the file name of a particular type if it exists
	FileType = WCS_EXPORTCONTROL_FILETYPE_RAWELEV;
	if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		CurJob->RBounds.NullReject = 1;
		CurJob->RBounds.NullValue = -9999.0f;
		// do something with the file name, like open it and resample it
		// what size was terrain rendered?
		// what size is it supposed to be

		// create a new file name for it
		// this should be the name of the temp buffer that WCS would save for normal image file output
		CurOpt->MakeTempFileName(FullNewPath, "Export", "ELEVATION", CurJob->FrameRendered, 0);
		BreakFileName(FullNewPath, PathPart, 512, FilePart, 256);

		if (CurJob->RowsRendered == CurJob->DEMResY && CurJob->ColsRendered == CurJob->DEMResX && CurJob->DEMTilesX == 1 && CurJob->DEMTilesY == 1)
			{
			// rename the file, alles gut
			strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownType);
			PROJ_remove(FullNewPath);
			if (PROJ_rename(FullOriginalPath, FullNewPath))
				{
				UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
				Success = 0;
				} // if
			else
				{
				// use image saver routine to save in final format drawing one line at a time from raw file
				// full file name currently is FullNewPath, file part of name is FilePart
				// which buffers to save, NULL terminated list
				BuffersToSave[0] = "ELEVATION";
				BuffersToSave[1] = NULL;
				// file name to save under
				strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
				// save file
				Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->DEMFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
					CurJob->FrameRendered, CurJob->DEMResY, CurJob->DEMResX, false, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

				// remove old entry
				strcpy(FilePart, FileNameOfKnownType);
				RemoveNameList(FileNamesCreated, FilePart, WCS_EXPORTCONTROL_FILETYPE_RAWELEV);
				// remove old file
				PROJ_remove(FullNewPath);
				// add a new file entry
				if (DEMExtension)
					strcat(CurrentOutputFile, DEMExtension);
				AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN);
				} // else
			} // if no resampling necessary
		else
			{
			// resample, create multiple tiles if necessary
			strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownType);
			if (CurJob->DEMTilesX == 1 && CurJob->DEMTilesY == 1)
				{
				// resample
				if (Success = ResampleRaster(FullOriginalPath, FullNewPath, WCS_RESAMPLE_BANDTYPE_FLOAT, CurJob->RowsRendered, CurJob->ColsRendered,
					CurJob->DEMResY, CurJob->DEMResX, -9999.0f, TRUE))
					{
					// reset RasterBounds
					CurJob->RBounds.DeriveCoords(CurJob->DEMResY, CurJob->DEMResX);
					// use image saver routine to save in final format drawing one line at a time from raw file
					// full file name currently is FullNewPath, file part of name is FilePart
					// which buffers to save, NULL terminated list
					BuffersToSave[0] = "ELEVATION";
					BuffersToSave[1] = NULL;
					// file name to save under
					strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
					// save file
					Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->DEMFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
						CurJob->FrameRendered, CurJob->DEMResY, CurJob->DEMResX, false, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

					// remove old entry
					strcpy(FilePart, FileNameOfKnownType);
					RemoveNameList(FileNamesCreated, FilePart, WCS_EXPORTCONTROL_FILETYPE_RAWELEV);
					// remove old files
					PROJ_remove(FullOriginalPath);
					PROJ_remove(FullNewPath);
					// add a new file entry
					if (DEMExtension)
						strcat(CurrentOutputFile, DEMExtension);
					AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN);
					} // if
				else
					UserMessageOK(FullOriginalPath, "Error resampling this file. Export terminated.");
				} // if
			else
				{
				CurJob->ValidateTileRowsCols(CurJob->DEMResY, CurJob->DEMResX, CurJob->DEMTilesY, CurJob->DEMTilesX, CurJob->DEMTileOverlap, CurJob->OneDEMResY, CurJob->OneDEMResX, CurJob->DEMResOption);
				for (CurTileY = 0; Success && CurTileY < CurJob->DEMTilesY; CurTileY ++)
					{
					for (CurTileX = 0; Success && CurTileX < CurJob->DEMTilesX; CurTileX ++)
						{
						if (Success = ResampleRasterTile(FullOriginalPath, FullNewPath, WCS_RESAMPLE_BANDTYPE_FLOAT, CurJob->RowsRendered, CurJob->ColsRendered,
							CurJob->DEMResY, CurJob->DEMResX, CurJob->DEMTilesY, CurJob->DEMTilesX, CurTileY, CurTileX, CurJob->DEMTileOverlap, -9999.0f, TRUE))
							{
							// reset RasterBounds
							CurJob->RBounds.DeriveTileCoords(CurJob->DEMResY, CurJob->DEMResX, 
								CurJob->DEMTilesY, CurJob->DEMTilesX, CurTileY, CurTileX, CurJob->DEMTileOverlap);
							// use image saver routine to save in final format drawing one line at a time from raw file
							// full file name currently is FullNewPath, file part of name is FilePart
							// which buffers to save, NULL terminated list
							BuffersToSave[0] = "ELEVATION";
							BuffersToSave[1] = NULL;
							// file name to save under
							sprintf(CurrentOutputFile, "%s_%dy_%dx", CurJob->OutPath.GetName(), CurTileY, CurTileX);
							// save file
							Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->DEMFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
								CurJob->FrameRendered, CurJob->OneDEMResY, CurJob->OneDEMResX, false, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

							// add a new file entry
							if (DEMExtension)
								strcat(CurrentOutputFile, DEMExtension);
							AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN);
							} // if
						else
							UserMessageOK(FullOriginalPath, "Error tiling this file. Export terminated.");
						} // for
					} // for
				// remove old entry
				strcpy(FilePart, FileNameOfKnownType);
				RemoveNameList(FileNamesCreated, FilePart, WCS_EXPORTCONTROL_FILETYPE_RAWELEV);
				// remove old file
				PROJ_remove(FullOriginalPath);
				PROJ_remove(FullNewPath);
				} // else
			} // else need to resample
		} // if

	// TEXTURE 1
	if (Success && (FileNameOfKnownTypeR = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX1_RED))
		&& (FileNameOfKnownTypeG = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN))
		&& (FileNameOfKnownTypeB = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU))
		&& (FileNameOfKnownTypeA = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA)))
		{
		CurJob->RBounds.NullReject = 0;
		CurJob->RBounds.NullValue = 0.0f;
		// do something with the file name, like open it and resample it
		// what size was image rendered?
		// what size is it supposed to be

		if (CurJob->RowsRendered == CurJob->TexResY && CurJob->ColsRendered == CurJob->TexResX && CurJob->TexTilesX == 1 && CurJob->TexTilesY == 1)
			{
			// create a new file name for it
			// this should be the name of the temp buffer that WCS would save for normal image file output
			CurOpt->MakeTempFileName(FullNewPathR, "Export", "RED", CurJob->FrameRendered, 0);
			BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
			strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeR);
			// rename the file, alles gut
			PROJ_remove(FullNewPathR);
			if (PROJ_rename(FullOriginalPath, FullNewPathR))
				{
				UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
				Success = 0;
				} // if
			else
				{
				CurOpt->MakeTempFileName(FullNewPathG, "Export", "GREEN", CurJob->FrameRendered, 0);
				BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
				strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeG);
				PROJ_remove(FullNewPathG);
				if (PROJ_rename(FullOriginalPath, FullNewPathG))
					{
					UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
					Success = 0;
					} // if
				else
					{
					CurOpt->MakeTempFileName(FullNewPathB, "Export", "BLUE", CurJob->FrameRendered, 0);
					BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
					strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeB);
					PROJ_remove(FullNewPathB);
					if (PROJ_rename(FullOriginalPath, FullNewPathB))
						{
						UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
						Success = 0;
						} // if
					else
						{
						CurOpt->MakeTempFileName(FullNewPathA, "Export", "ANTIALIAS", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
						strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeA);
						PROJ_remove(FullNewPathA);
						if (PROJ_rename(FullOriginalPath, FullNewPathA))
							{
							UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
							Success = 0;
							} // if
						else
							{
							// set RasterBounds for image size
							CurJob->RBounds.DeriveCoords(CurJob->TexResY, CurJob->TexResX);
							// use image saver routine to save in final format drawing one line at a time from raw file
							// full file name currently is FullNewPath, file part of name is FilePart
							// which buffers to save, NULL terminated list
							BuffersToSave[0] = "RED";
							BuffersToSave[1] = "GREEN";
							BuffersToSave[2] = "BLUE";
							// turn off saving alpha channel if it might cause problems
							BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"ANTIALIAS": NULL;
							BuffersToSave[4] = NULL;
							// file name to save under
							strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
							strcat(CurrentOutputFile, "_RGB");
							// save file
							Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
								CurJob->FrameRendered, CurJob->TexResY, CurJob->TexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

							// remove old entry
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX1_RED);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA);
							// remove old file
							PROJ_remove(FullNewPathR);
							PROJ_remove(FullNewPathG);
							PROJ_remove(FullNewPathB);
							PROJ_remove(FullNewPathA);
							// add a new file entry
							if (ImageExtension)
								strcat(CurrentOutputFile, ImageExtension);
							AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX1);
							} // else
						} // else
					} // else
				} // else
			} // if no resampling necessary
		else
			{
			// resample, create multiple tiles if necessary
			if (CurJob->TexTilesX == 1 && CurJob->TexTilesY == 1)
				{
				// resample
				// create a new file name for it
				// this should be the name of the temp buffer that WCS would save for normal image file output
				CurOpt->MakeTempFileName(FullNewPathR, "Export", "RED", CurJob->FrameRendered, 0);
				BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
				strmfp(FullOriginalPathR, OutputFilePath, FileNameOfKnownTypeR);
				if (Success = ResampleRaster(FullOriginalPathR, FullNewPathR, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
					CurJob->TexResY, CurJob->TexResX))
					{
					CurOpt->MakeTempFileName(FullNewPathG, "Export", "GREEN", CurJob->FrameRendered, 0);
					BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
					strmfp(FullOriginalPathG, OutputFilePath, FileNameOfKnownTypeG);
					if (Success = ResampleRaster(FullOriginalPathG, FullNewPathG, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
						CurJob->TexResY, CurJob->TexResX))
						{
						CurOpt->MakeTempFileName(FullNewPathB, "Export", "BLUE", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
						strmfp(FullOriginalPathB, OutputFilePath, FileNameOfKnownTypeB);
						if (Success = ResampleRaster(FullOriginalPathB, FullNewPathB, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
							CurJob->TexResY, CurJob->TexResX))
							{
							CurOpt->MakeTempFileName(FullNewPathA, "Export", "ANTIALIAS", CurJob->FrameRendered, 0);
							BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
							strmfp(FullOriginalPathA, OutputFilePath, FileNameOfKnownTypeA);
							if (Success = ResampleRaster(FullOriginalPathA, FullNewPathA, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
								CurJob->TexResY, CurJob->TexResX))
								{
								// set RasterBounds for image size
								CurJob->RBounds.DeriveCoords(CurJob->TexResY, CurJob->TexResX);
								// use image saver routine to save in final format drawing one line at a time from raw file
								// full file name currently is FullNewPath, file part of name is FilePart
								// which buffers to save, NULL terminated list
								BuffersToSave[0] = "RED";
								BuffersToSave[1] = "GREEN";
								BuffersToSave[2] = "BLUE";
								// turn off saving alpha channel if it might cause problems
								BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"ANTIALIAS": NULL;
								BuffersToSave[4] = NULL;
								// file name to save under
								strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
								strcat(CurrentOutputFile, "_RGB");
								// save file
								Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
									CurJob->FrameRendered, CurJob->TexResY, CurJob->TexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

								// remove old entry
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX1_RED);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA);
								// remove old file
								PROJ_remove(FullOriginalPathR);
								PROJ_remove(FullOriginalPathG);
								PROJ_remove(FullOriginalPathB);
								PROJ_remove(FullOriginalPathA);
								PROJ_remove(FullNewPathR);
								PROJ_remove(FullNewPathG);
								PROJ_remove(FullNewPathB);
								PROJ_remove(FullNewPathA);
								// add a new file entry
								if (ImageExtension)
									strcat(CurrentOutputFile, ImageExtension);
								AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX1);
								} // if
							else
								UserMessageOK(FullOriginalPathA, "Error resampling this file. Export terminated.");
							} // if
						else
							UserMessageOK(FullOriginalPathB, "Error resampling this file. Export terminated.");
						} // if
					else
						UserMessageOK(FullOriginalPathG, "Error resampling this file. Export terminated.");
					} // if
				else
					UserMessageOK(FullOriginalPathR, "Error resampling this file. Export terminated.");
				} // if
			else
				{
				CurJob->ValidateTileRowsCols(CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurJob->TexTileOverlap, CurJob->OneTexResY, CurJob->OneTexResX, CurJob->TexResOption);
				for (CurTileY = 0; CurTileY < CurJob->TexTilesY; CurTileY ++)
					{
					for (CurTileX = 0; CurTileX < CurJob->TexTilesX; CurTileX ++)
						{
						// resample into tiles
						// create a new file name for it
						// this should be the name of the temp buffer that WCS would save for normal image file output
						CurOpt->MakeTempFileName(FullNewPathR, "Export", "RED", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
						strmfp(FullOriginalPathR, OutputFilePath, FileNameOfKnownTypeR);
						if (Success = ResampleRasterTile(FullOriginalPathR, FullNewPathR, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
							CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
							{
							CurOpt->MakeTempFileName(FullNewPathG, "Export", "GREEN", CurJob->FrameRendered, 0);
							BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
							strmfp(FullOriginalPathG, OutputFilePath, FileNameOfKnownTypeG);
							if (Success = ResampleRasterTile(FullOriginalPathG, FullNewPathG, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
								CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
								{
								CurOpt->MakeTempFileName(FullNewPathB, "Export", "BLUE", CurJob->FrameRendered, 0);
								BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
								strmfp(FullOriginalPathB, OutputFilePath, FileNameOfKnownTypeB);
								if (Success = ResampleRasterTile(FullOriginalPathB, FullNewPathB, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
									CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
									{
									CurOpt->MakeTempFileName(FullNewPathA, "Export", "ANTIALIAS", CurJob->FrameRendered, 0);
									BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
									strmfp(FullOriginalPathA, OutputFilePath, FileNameOfKnownTypeA);
									if (Success = ResampleRasterTile(FullOriginalPathA, FullNewPathA, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
										CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
										{
										// set RasterBounds for image size
										CurJob->RBounds.DeriveTileCoords(CurJob->TexResY, CurJob->TexResX, 
											CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap);
										// use image saver routine to save in final format drawing one line at a time from raw file
										// full file name currently is FullNewPath, file part of name is FilePart
										// which buffers to save, NULL terminated list
										BuffersToSave[0] = "RED";
										BuffersToSave[1] = "GREEN";
										BuffersToSave[2] = "BLUE";
										// turn off saving alpha channel if it might cause problems
										BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"ANTIALIAS": NULL;
										BuffersToSave[4] = NULL;
										// file name to save under
										sprintf(CurrentOutputFile, "%s_RGB_%dy_%dx", CurJob->OutPath.GetName(), CurTileY, CurTileX);
										// save file
										Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
											CurJob->FrameRendered, CurJob->OneTexResY, CurJob->OneTexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

										// add a new file entry
										if (ImageExtension)
											strcat(CurrentOutputFile, ImageExtension);
										AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX1);
										} // if
									else
										UserMessageOK(FullOriginalPathA, "Error tiling this file. Export terminated.");
									} // if
								else
									UserMessageOK(FullOriginalPathB, "Error tiling this file. Export terminated.");
								} // if
							else
								UserMessageOK(FullOriginalPathG, "Error tiling this file. Export terminated.");
							} // if
						else
							UserMessageOK(FullOriginalPathR, "Error tiling this file. Export terminated.");
						} // for
					} // for
				// remove old entry
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX1_RED);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX1_GRN);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX1_BLU);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX1_ALPHA);
				// remove old file
				PROJ_remove(FullOriginalPathR);
				PROJ_remove(FullOriginalPathG);
				PROJ_remove(FullOriginalPathB);
				PROJ_remove(FullOriginalPathA);
				PROJ_remove(FullNewPathR);
				PROJ_remove(FullNewPathG);
				PROJ_remove(FullNewPathB);
				PROJ_remove(FullNewPathA);
				} // else
			} // else need to resample
		} // if

	// TEXTURE 2
	// same as Texture 1 but with FOLIAGE RED... buffers and WCS_EXPORTCONTROL_FILETYPE_TEX2
	if (Success && (FileNameOfKnownTypeR = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX2_RED))
		&& (FileNameOfKnownTypeG = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN))
		&& (FileNameOfKnownTypeB = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU))
		&& (FileNameOfKnownTypeA = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA)))
		{
		CurJob->RBounds.NullReject = 0;
		CurJob->RBounds.NullValue = 0.0f;
		// do something with the file name, like open it and resample it
		// what size was image rendered?
		// what size is it supposed to be

		if (CurJob->RowsRendered == CurJob->TexResY && CurJob->ColsRendered == CurJob->TexResX && CurJob->TexTilesX == 1 && CurJob->TexTilesY == 1)
			{
			// create a new file name for it
			// this should be the name of the temp buffer that WCS would save for normal image file output
			CurOpt->MakeTempFileName(FullNewPathR, "Export", "FOLIAGE RED", CurJob->FrameRendered, 0);
			BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
			strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeR);
			// rename the file, alles gut
			PROJ_remove(FullNewPathR);
			if (PROJ_rename(FullOriginalPath, FullNewPathR))
				{
				UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
				Success = 0;
				} // if
			else
				{
				CurOpt->MakeTempFileName(FullNewPathG, "Export", "FOLIAGE GREEN", CurJob->FrameRendered, 0);
				BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
				strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeG);
				PROJ_remove(FullNewPathG);
				if (PROJ_rename(FullOriginalPath, FullNewPathG))
					{
					UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
					Success = 0;
					} // if
				else
					{
					CurOpt->MakeTempFileName(FullNewPathB, "Export", "FOLIAGE BLUE", CurJob->FrameRendered, 0);
					BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
					strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeB);
					PROJ_remove(FullNewPathB);
					if (PROJ_rename(FullOriginalPath, FullNewPathB))
						{
						UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
						Success = 0;
						} // if
					else
						{
						CurOpt->MakeTempFileName(FullNewPathA, "Export", "FOLIAGE ANTIALIAS", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
						strmfp(FullOriginalPath, OutputFilePath, FileNameOfKnownTypeA);
						PROJ_remove(FullNewPathA);
						if (PROJ_rename(FullOriginalPath, FullNewPathA))
							{
							UserMessageOK(FullOriginalPath, "Error renaming this file. Export terminated.");
							Success = 0;
							} // if
						else
							{
							// set RasterBounds for image size
							CurJob->RBounds.DeriveCoords(CurJob->TexResY, CurJob->TexResX);
							// use image saver routine to save in final format drawing one line at a time from raw file
							// full file name currently is FullNewPath, file part of name is FilePart
							// which buffers to save, NULL terminated list
							BuffersToSave[0] = "FOLIAGE RED";
							BuffersToSave[1] = "FOLIAGE GREEN";
							BuffersToSave[2] = "FOLIAGE BLUE";
							// turn off saving alpha channel if it might cause problems
							BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"FOLIAGE ANTIALIAS": NULL;
							BuffersToSave[4] = NULL;
							// file name to save under
							strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
							strcat(CurrentOutputFile, "_FolRGB");
							// save file
							Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
								CurJob->FrameRendered, CurJob->TexResY, CurJob->TexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

							// remove old entry
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX2_RED);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU);
							strcpy(FilePart, FileNameOfKnownTypeR);
							RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA);
							// remove old file
							PROJ_remove(FullNewPathR);
							PROJ_remove(FullNewPathG);
							PROJ_remove(FullNewPathB);
							PROJ_remove(FullNewPathA);
							// add a new file entry
							if (ImageExtension)
								strcat(CurrentOutputFile, ImageExtension);
							AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX2);
							} // else
						} // else
					} // else
				} // else
			} // if no resampling necessary
		else
			{
			// resample, create multiple tiles if necessary
			if (CurJob->TexTilesX == 1 && CurJob->TexTilesY == 1)
				{
				// resample
				// create a new file name for it
				// this should be the name of the temp buffer that WCS would save for normal image file output
				CurOpt->MakeTempFileName(FullNewPathR, "Export", "FOLIAGE RED", CurJob->FrameRendered, 0);
				BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
				strmfp(FullOriginalPathR, OutputFilePath, FileNameOfKnownTypeR);
				if (Success = ResampleRaster(FullOriginalPathR, FullNewPathR, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
					CurJob->TexResY, CurJob->TexResX))
					{
					CurOpt->MakeTempFileName(FullNewPathG, "Export", "FOLIAGE GREEN", CurJob->FrameRendered, 0);
					BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
					strmfp(FullOriginalPathG, OutputFilePath, FileNameOfKnownTypeG);
					if (Success = ResampleRaster(FullOriginalPathG, FullNewPathG, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
						CurJob->TexResY, CurJob->TexResX))
						{
						CurOpt->MakeTempFileName(FullNewPathB, "Export", "FOLIAGE BLUE", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
						strmfp(FullOriginalPathB, OutputFilePath, FileNameOfKnownTypeB);
						if (Success = ResampleRaster(FullOriginalPathB, FullNewPathB, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
							CurJob->TexResY, CurJob->TexResX))
							{
							CurOpt->MakeTempFileName(FullNewPathA, "Export", "FOLIAGE ANTIALIAS", CurJob->FrameRendered, 0);
							BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
							strmfp(FullOriginalPathA, OutputFilePath, FileNameOfKnownTypeA);
							if (Success = ResampleRaster(FullOriginalPathA, FullNewPathA, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
								CurJob->TexResY, CurJob->TexResX))
								{
								// set RasterBounds for image size
								CurJob->RBounds.DeriveCoords(CurJob->TexResY, CurJob->TexResX);
								// use image saver routine to save in final format drawing one line at a time from raw file
								// full file name currently is FullNewPath, file part of name is FilePart
								// which buffers to save, NULL terminated list
								BuffersToSave[0] = "FOLIAGE RED";
								BuffersToSave[1] = "FOLIAGE GREEN";
								BuffersToSave[2] = "FOLIAGE BLUE";
								// turn off saving alpha channel if it might cause problems
								BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"FOLIAGE ANTIALIAS": NULL;
								BuffersToSave[4] = NULL;
								// file name to save under
								strcpy(CurrentOutputFile, CurJob->OutPath.GetName());
								strcat(CurrentOutputFile, "_FolRGB");
								// save file
								Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
									CurJob->FrameRendered, CurJob->TexResY, CurJob->TexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

								// remove old entry
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX2_RED);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU);
								strcpy(FilePart, FileNameOfKnownTypeR);
								RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA);
								// remove old file
								PROJ_remove(FullOriginalPathR);
								PROJ_remove(FullOriginalPathG);
								PROJ_remove(FullOriginalPathB);
								PROJ_remove(FullOriginalPathA);
								PROJ_remove(FullNewPathR);
								PROJ_remove(FullNewPathG);
								PROJ_remove(FullNewPathB);
								PROJ_remove(FullNewPathA);
								// add a new file entry
								if (ImageExtension)
									strcat(CurrentOutputFile, ImageExtension);
								AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX2);
								} // if
							else
								UserMessageOK(FullOriginalPathA, "Error resampling this file. Export terminated.");
							} // if
						else
							UserMessageOK(FullOriginalPathB, "Error resampling this file. Export terminated.");
						} // if
					else
						UserMessageOK(FullOriginalPathG, "Error resampling this file. Export terminated.");
					} // if
				else
					UserMessageOK(FullOriginalPathR, "Error resampling this file. Export terminated.");
				} // if
			else
				{
				CurJob->ValidateTileRowsCols(CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurJob->TexTileOverlap, CurJob->OneTexResY, CurJob->OneTexResX, CurJob->TexResOption);
				for (CurTileY = 0; CurTileY < CurJob->TexTilesY; CurTileY ++)
					{
					for (CurTileX = 0; CurTileX < CurJob->TexTilesX; CurTileX ++)
						{
						// resample into tiles
						// create a new file name for it
						// this should be the name of the temp buffer that WCS would save for normal image file output
						CurOpt->MakeTempFileName(FullNewPathR, "Export", "FOLIAGE RED", CurJob->FrameRendered, 0);
						BreakFileName(FullNewPathR, PathPart, 512, FilePartR, 256);
						strmfp(FullOriginalPathR, OutputFilePath, FileNameOfKnownTypeR);
						if (Success = ResampleRasterTile(FullOriginalPathR, FullNewPathR, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
							CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
							{
							CurOpt->MakeTempFileName(FullNewPathG, "Export", "FOLIAGE GREEN", CurJob->FrameRendered, 0);
							BreakFileName(FullNewPathG, PathPart, 512, FilePartG, 256);
							strmfp(FullOriginalPathG, OutputFilePath, FileNameOfKnownTypeG);
							if (Success = ResampleRasterTile(FullOriginalPathG, FullNewPathG, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
								CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
								{
								CurOpt->MakeTempFileName(FullNewPathB, "Export", "FOLIAGE BLUE", CurJob->FrameRendered, 0);
								BreakFileName(FullNewPathB, PathPart, 512, FilePartB, 256);
								strmfp(FullOriginalPathB, OutputFilePath, FileNameOfKnownTypeB);
								if (Success = ResampleRasterTile(FullOriginalPathB, FullNewPathB, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
									CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
									{
									CurOpt->MakeTempFileName(FullNewPathA, "Export", "FOLIAGE ANTIALIAS", CurJob->FrameRendered, 0);
									BreakFileName(FullNewPathA, PathPart, 512, FilePartA, 256);
									strmfp(FullOriginalPathA, OutputFilePath, FileNameOfKnownTypeA);
									if (Success = ResampleRasterTile(FullOriginalPathA, FullNewPathA, WCS_RESAMPLE_BANDTYPE_UBYTE, CurJob->RowsRendered, CurJob->ColsRendered,
										CurJob->TexResY, CurJob->TexResX, CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap))
										{
										// set RasterBounds for image size
										CurJob->RBounds.DeriveTileCoords(CurJob->TexResY, CurJob->TexResX, 
											CurJob->TexTilesY, CurJob->TexTilesX, CurTileY, CurTileX, CurJob->TexTileOverlap);
										// use image saver routine to save in final format drawing one line at a time from raw file
										// full file name currently is FullNewPath, file part of name is FilePart
										// which buffers to save, NULL terminated list
										BuffersToSave[0] = "FOLIAGE RED";
										BuffersToSave[1] = "FOLIAGE GREEN";
										BuffersToSave[2] = "FOLIAGE BLUE";
										// turn off saving alpha channel if it might cause problems
										BuffersToSave[3] = (! ExportFmt->ProhibitAlphaSave()) ? (char*)"FOLIAGE ANTIALIAS": NULL;
										BuffersToSave[4] = NULL;
										// file name to save under
										sprintf(CurrentOutputFile, "%s_FolRGB_%dy_%dx", CurJob->OutPath.GetName(), CurTileY, CurTileX);
										// save file
										Success = SaveBuffer(CurJob, CurOpt, &CurJob->RBounds, CurJob->ImageFormat, BuffersToSave, NULL, (char *)OutputFilePath, CurrentOutputFile, 
											CurJob->FrameRendered, CurJob->OneTexResY, CurJob->OneTexResX, CurJob->WorldFile, CurJob->MaxRenderedElevation, CurJob->MinRenderedElevation);

										// add a new file entry
										if (ImageExtension)
											strcat(CurrentOutputFile, ImageExtension);
										AddNewNameList(FileNamesCreated, CurrentOutputFile, WCS_EXPORTCONTROL_FILETYPE_TEX2);
										} // if
									else
										UserMessageOK(FullOriginalPathA, "Error tiling this file. Export terminated.");
									} // if
								else
									UserMessageOK(FullOriginalPathB, "Error tiling this file. Export terminated.");
								} // if
							else
								UserMessageOK(FullOriginalPathG, "Error tiling this file. Export terminated.");
							} // if
						else
							UserMessageOK(FullOriginalPathR, "Error tiling this file. Export terminated.");
						} // for
					} // for
				// remove old entry
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartR, WCS_EXPORTCONTROL_FILETYPE_TEX2_RED);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartG, WCS_EXPORTCONTROL_FILETYPE_TEX2_GRN);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartB, WCS_EXPORTCONTROL_FILETYPE_TEX2_BLU);
				strcpy(FilePart, FileNameOfKnownTypeR);
				RemoveNameList(FileNamesCreated, FilePartA, WCS_EXPORTCONTROL_FILETYPE_TEX2_ALPHA);
				// remove old file
				PROJ_remove(FullOriginalPathR);
				PROJ_remove(FullOriginalPathG);
				PROJ_remove(FullOriginalPathB);
				PROJ_remove(FullOriginalPathA);
				PROJ_remove(FullNewPathR);
				PROJ_remove(FullNewPathG);
				PROJ_remove(FullNewPathB);
				PROJ_remove(FullNewPathA);
				} // else
			} // else need to resample
		} // if

	delete CurOpt;
	} // if
else
	Success = 0;

FrameTextClearA();
return (Success);

} // ExportControlGUI::ResampleExports

/*===========================================================================*/

// determines tile numbers from a file name
void ExportControlGUI::ParseTileNumbers(const char *FileName, long &CurTileY, long &CurTileX)
{
char FileNameCopy[256], *Underscore, *StartY = NULL, *StartX = NULL;

// copy file name
strcpy(FileNameCopy, FileName);
// strip extension
StripExtension(FileNameCopy);

// find the last underscore if there is one - x
Underscore = &FileNameCopy[strlen(FileNameCopy) - 1];
while (Underscore >= FileNameCopy)
	{
	if (*Underscore == '_')
		{
		if (StartX)
			{
			StartY = Underscore + 1;
			break;
			} // if
		else
			{
			StartX = Underscore + 1;
			} // else
		} // if
	Underscore --;
	} // while

// find next last underscore if there is one
if (StartX && StartY)
	{
	CurTileX = atoi(StartX);
	*StartX = 0;
	CurTileY = atoi(StartY);
	} // if
else
	{
	CurTileX = CurTileY = 0;
	} // else

} // ExportControlGUI::ParseTileNumbers

/*===========================================================================*/

// saves an image or terrain
int ExportControlGUI::SaveBuffer(SceneExporter *CurJob, RenderOpt *Opt, RasterBounds *RBounds, char *FileType, char **BuffersToSave, 
	void **Buffers, char *OutPath, char *OutFile, long Frame, long Height, long Width, int WorldFile, float MaxTerrainElev, float MinTerrainElev)
{
int Success = 0, BufCt;
char *DefBuf;
BufferNode *CurBuf, *RootBuf;
ImageOutputEvent IOEvent;

strcpy(IOEvent.FileType, FileType);
for (BufCt = 0; BuffersToSave[BufCt]; BufCt ++)
	{ 
	strcpy(IOEvent.OutBuffers[BufCt], BuffersToSave[BufCt]);
	} // for
// Special hack here for Google Earth KML export
// WCS/VNS makes premultiplied alpha, GE wants to see nonpremultiplied alpha
// Our PNG saver has a special option, implemented as a codec, to try to reverse the premultiplication
// We only want this done on GE terrain texture PNG images. So, we check to make sure the exporter format
// is GE/KML, the image format is PNG and the Max/Min terrain Elev are not 0.
if(!strcmp(CurJob->ExportTarget, "GoogleEarth") && !strcmp(FileType, "PNG") && MaxTerrainElev != 0.0 && MinTerrainElev != 0.0)
	{
	strcpy(IOEvent.Codec, "NonPremultAlpha");
	} // if
else if(!strcmp(CurJob->ExportTarget, "WorldWind") && !strcmp(FileType, "TIFF"))
	{
	strcpy(IOEvent.Codec, "WorldWind");
	} // if
else
	{
	if (DefBuf = ImageSaverLibrary::GetNextCodec(IOEvent.FileType, NULL))
		strcpy(IOEvent.Codec, DefBuf);
	} // else
IOEvent.PAF.SetPathAndName(OutPath, OutFile);
IOEvent.AutoExtension = 1;
IOEvent.AutoDigits = 0;

// questions
// How will a coordinate system be embedded in a WCS DEM when saved this way
// How will image files get their world file coordinates and the data to write out a PRJ file
// What will we pass data to the image savers in instead of a Renderer

// create a set of Buffer Nodes
// currently elevation is the only non-byte band
if (CurBuf = RootBuf = new BufferNode(IOEvent.OutBuffers[0], stricmp(IOEvent.OutBuffers[0], "ELEVATION") ? WCS_RASTER_BANDSET_BYTE: WCS_RASTER_BANDSET_FLOAT))
	{
	CurBuf->Buffer = Buffers ? Buffers[0]: NULL;	// data will be gotten from file (hopefully)
	for (BufCt = 1; IOEvent.OutBuffers[BufCt][0]; BufCt ++)
		{
		if (CurBuf = CurBuf->AddBufferNode(IOEvent.OutBuffers[BufCt], stricmp(IOEvent.OutBuffers[BufCt], "ELEVATION") ? WCS_RASTER_BANDSET_BYTE: WCS_RASTER_BANDSET_FLOAT))
			CurBuf->Buffer = Buffers ? Buffers[BufCt]: NULL;	// data will be gotten from file (hopefully)
		} // while
	// "WorldFile" translates to "With World File", which also controls insertion of GeoTIFF tags
	if (RBounds && WorldFile)
		strcpy(IOEvent.Codec, "With World File");

	// this sets up some necessary format-specific allocations
	if (IOEvent.InitSequence(RBounds, RootBuf, Width, Height))
		{
		// elevation range is needed for ECW DEM format
		IOEvent.SetDataRange(MaxTerrainElev, MinTerrainElev);

		// prep to save
		for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
			{
			CurBuf->PrepToSave(Opt, Frame, Width, 0);
			} // for

		// Save it
		Success = IOEvent.SaveImage(RBounds, RootBuf, Width, Height, 0, Opt);

		// Cleanup all prep work
		for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
			{
			CurBuf->CleanupFromSave();
			} // for

		while (RootBuf)
			{
			CurBuf = RootBuf->Next;
			delete RootBuf;
			RootBuf = CurBuf;
			} // if

		} // if
	else
		Success = 0;
	} // if

return (Success);

} // ExportControlGUI::SaveBuffer

/*===========================================================================*/

// compress the final files
int ExportControlGUI::ZipItAndShipIt(SceneExporter *CurJob, NameList **FileNamesCreated)
{
NameList *CurName;
const char *OutputFilePath;
int Success = 1;

if (CurJob->ZipItUp)
	{
	SetFrameTextA("Zipping Files");

	// If desired, all the created final output files can be zipped together or tarred and gzipped or whatever
	// That is what this function is for.
	// Use the NameList to identify all the files tha need to be collated and zipped.

	// The directory where all the files are found is:
	OutputFilePath = CurJob->OutPath.GetPath();

	for (CurName = *FileNamesCreated; CurName; CurName = CurName->Next)
		{
		if (CurName->Name)
			{
			// this is a file that needs to be processed
			} // if
		} // for
	FrameTextClearA();
	} // if

return (Success);

} // ExportControlGUI::ZipItAndShipIt

/*===========================================================================*/

// make sure all paths are valid before beginning
int ExportControlGUI::ValidateExportPaths(SceneExporter *CurJob)
{
char CurrentDirStash[1024];
_getcwd(CurrentDirStash, 1023);

if (PROJ_chdir(CurJob->OutPath.GetPath()))
	{
	if (UserMessageOKCAN((char *)CurJob->OutPath.GetPath(), "Path does not exist. Create a new directory?"))
		{
		if (PROJ_mkdir(CurJob->OutPath.GetPath()))
			{
			UserMessageOK((char *)CurJob->OutPath.GetPath(), "Directory could not be created. There may be a file of the same name as the directory or a permissions problem or more than one directory needs to be created in the same path. Try selecting a new Output Path in the Scene Exporter Editor or create the paths manually in the Output Path file selector.");
			PROJ_chdir(CurrentDirStash); // put it back so we don't hold a lock on it
			return (0);
			} // if
		} // if
	else
		{
		PROJ_chdir(CurrentDirStash); // put it back so we don't hold a lock on it
		return (0);
		} // else
	} // if
if (PROJ_chdir(CurJob->TempPath.GetPath()))
	{
	if (UserMessageOKCAN((char *)CurJob->TempPath.GetPath(), "Path does not exist. Create a new directory?"))
		{
		if (PROJ_mkdir(CurJob->TempPath.GetPath()))
			{
			UserMessageOK((char *)CurJob->TempPath.GetPath(), "Directory could not be created. There may be a file of the same name as the directory or a permissions problem or more than one directory needs to be created in the same path. Try selecting a new Temp. Path in the Scene Exporter Editor or create the paths manually in the Temp. Path file selector.");
			PROJ_chdir(CurrentDirStash); // put it back so we don't hold a lock on it
			return (0);
			} // if
		} // if
	else
		{
		PROJ_chdir(CurrentDirStash); // put it back so we don't hold a lock on it
		return (0);
		} // else
	} // if

PROJ_chdir(CurrentDirStash); // put it back so we don't hold a lock on it
return (1);

} // ExportControlGUI::ValidateExportPaths

/*===========================================================================*/

int ExportControlGUI::CheckAlphaWorthwhile (unsigned char *AABuf, long BufSize)
{
long Zip;

for (Zip = 0; Zip < BufSize; Zip ++)
	{
	if (AABuf[Zip] < 255)
		return (1);
	} // for

return (0);

} // ExportControlGUI::CheckAlphaWorthwhile

/*===========================================================================*/

NameList *ExportControlGUI::AddNewNameList(NameList **Names, char *NewName, long FileType)
{
NameList **ListPtr;

if (Names)
	{
	ListPtr = Names;
	while (*ListPtr)
		{
		if (! stricmp((*ListPtr)->Name, NewName) && FileType == (*ListPtr)->ItemClass)
			return (*ListPtr);
		ListPtr = &(*ListPtr)->Next;
		} // if
	return (*ListPtr = new NameList(NewName, FileType));
	} // if

return (NULL);

} // ExportControlGUI::AddNewNameList

/*===========================================================================*/

NameList *ExportControlGUI::RemoveNameList(NameList **Names, char *RemoveName, long FileType)
{
NameList **ListPtr, *DelPtr;

if (Names)
	{
	ListPtr = Names;
	while (*ListPtr)
		{
		if (! stricmp((*ListPtr)->Name, RemoveName) && FileType == (*ListPtr)->ItemClass)
			{
			DelPtr = *ListPtr;
			*ListPtr = (*ListPtr)->Next;
			delete DelPtr;
			break;
			} // if
		ListPtr = &(*ListPtr)->Next;
		} // if
	return (*Names);
	} // if

return (NULL);

} // ExportControlGUI::RemoveNameList

/*===========================================================================*/

void ExportControlGUI::ExportStop(void)
{

Rendering = 0;
Run = 0;
DisableWidgets();

} // ExportControlGUI::ExportStop

/*===========================================================================*/

void ExportControlGUI::SyncTexts(void)
{

if (NativeWin)
	{
	WidgetSetText(IDC_FRAMESTAT, FText);
	WidgetSetText(IDC_PROCSTAT, PText);
	WidgetSetText(IDC_IMAGEINFO, IText);
	WidgetSetText(IDC_RESINFO, RText);
	WidgetSetText(IDC_FRACTINFO, FRText);
	WidgetSetText(ID_CURFRAMENUM, ProgCap);
	WidgetSetText(ID_LASTFRAMENUM, PrevCap);
	WidgetSetText(IDC_ANIMSTATIC5, FrameTitle);
	UpdateStamp();
	} // if

} // ExportControlGUI::SyncTexts

/*===========================================================================*/

void ExportControlGUI::ClearAll(void)
{

AText[0] = /*FText[0] = */PText[0] = 0;
IText[0] = RText[0] = FRText[0] = 0;
SyncTexts();

} // ExportControlGUI::ClearAll

/*===========================================================================*/

void ExportControlGUI::SetProcText(char *Text)
{

sprintf(PText, "[%s]", Text);
SyncTexts();

} // ExportControlGUI::SetProcText

/*===========================================================================*/

void ExportControlGUI::SetAnimText(char *Text)
{

strncpy(AText, Text, 80);
AText[80] = 0;
SyncTexts();

} // ExportControlGUI::SetAnimText

/*===========================================================================*/

void ExportControlGUI::SetFrameNum(char *FrameNum)
{

if(strlen(FrameNum) < 8)
	{
	sprintf(ProgCap, "%s: in progress", FrameNum);
	} // if
else
	{
	sprintf(ProgCap, "%s:", FrameNum);
	} // else
SyncTexts();

} // ExportControlGUI::SetFrameNum

/*===========================================================================*/

void ExportControlGUI::SetFrameTextA(char *Text)
{

sprintf(FText, "[%s]", Text);
FText[80] = NULL;
SyncTexts();

} // ExportControlGUI::SetFrameTextA

/*===========================================================================*/

void ExportControlGUI::SetImageText(char *Text)
{

strncpy(IText, Text, 80);
IText[80] = 0;
SyncTexts();

} // ExportControlGUI::SetImageText

/*===========================================================================*/

void ExportControlGUI::SetResText(char *Text)
{

strncpy(RText, Text, 80);
RText[80] = 0;
SyncTexts();

} // ExportControlGUI::SetResText

/*===========================================================================*/

void ExportControlGUI::SetFractText(char *Text)
{

strncpy(FRText, Text, 80);
FRText[80] = 0;
SyncTexts();

} // ExportControlGUI::SetFractText

/*===========================================================================*/

void ExportControlGUI::SetPreview(char PreviewOn)
{

Preview = PreviewOn;
WidgetSetCheck(IDC_DISPLAYPREV, Preview);

} // ExportControlGUI::SetPreview

/*===========================================================================*/

void ExportControlGUI::UpdateStamp(void)
{

if (Rend && Rend->Rast)
	{
	if (Rend->MultiBuffer && ! Rend->PlotFromFrags)
		{
		Rend->Rast->CreateMergedThumbNail(Rend->Bitmap[0], Rend->Bitmap[1], Rend->Bitmap[2], Rend->ZBuf, 
			Rend->TreeBitmap[0], Rend->TreeBitmap[1], Rend->TreeBitmap[2], Rend->TreeZBuf);
		} // if
	else if (Rend->PlotFromFrags)
		{
		Rend->Rast->CreateFragThumbnail(Rend->rPixelFragMap);
		} // else
	else
		{
		Rend->Rast->CreateThumbNail(WCS_RASTER_BANDSET_BYTE, Rend->Bitmap[0], Rend->Bitmap[1], Rend->Bitmap[2], Rend->ExponentBuf);
		} // else
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, Rend->Rast);
	} // if

} // ExportControlGUI::UpdateStamp

/*===========================================================================*/

void ExportControlGUI::UpdateLastFrame(void)
{

Rend->Rast->Thumb->Copy(Rend->LastRast->Thumb, Rend->Rast->Thumb);

ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, Rend->LastRast);

} // ExportControlGUI::UpdateLastFrame

/*===========================================================================*/

void ExportControlGUI::AnimInit(unsigned long Steps, char *Text)
{

AMaxSteps = Steps;
AnimUpdate(0, Text);

ACurSteps = 0;
AStartSeconds = 0;
GetTime(AStartSeconds);

} // ExportControlGUI::AnimInit

/*===========================================================================*/

void ExportControlGUI::AnimUpdate(unsigned long NewSteps, char *Text)
{


} // ExportControlGUI::AnimUpdate

/*===========================================================================*/

void ExportControlGUI::AnimClear()
{

SyncTexts();

} // ExportControlGUI::AnimClear

/*===========================================================================*/

void ExportControlGUI::FrameTextInitA(char *Text)
{

if(Text)
	{
	sprintf(FText, "[%s]", Text);
	FText[80] = 0;
	SyncTexts();
	} // if

} // ExportControlGUI::FrameTextInitA

/*===========================================================================*/

void ExportControlGUI::FrameTextClearA()
{

FText[0] = 0;
SyncTexts();

} // ExportControlGUI::FrameTextClearA

/*===========================================================================*/

void ExportControlGUI::FrameGaugeInit(unsigned long Steps)
{

FMaxSteps = Steps;
FrameGaugeUpdate(0);
FCurSteps = 0;
FStartSeconds = 0;
GetTime(FStartSeconds);

} // ExportControlGUI::FrameGaugeInit

/*===========================================================================*/

void ExportControlGUI::FrameGaugeUpdate(unsigned long NewSteps)
{

DoUpdate(NewSteps, FMaxSteps, FStartSeconds, FCurSteps,
	IDC_FRAME_REMAIN, IDC_FRAME_ELAPSE, NULL, IDC_FRAME_PERCENT);

} // ExportControlGUI::FrameGaugeUpdate

/*===========================================================================*/

void ExportControlGUI::FrameGaugeClear()
{

DoUpdate(0, FMaxSteps, FStartSeconds, FCurSteps,
	IDC_FRAME_REMAIN, IDC_FRAME_ELAPSE, NULL, IDC_FRAME_PERCENT);
FMaxSteps = 0;
FCurSteps = 0;
FStartSeconds = 0;

} // ExportControlGUI::FrameGaugeClear

/*===========================================================================*/

void ExportControlGUI::ProcInit(unsigned long Steps, char *Text)
{

PMaxSteps = Steps;
ProcUpdate(0, Text);
PCurSteps = 0;

} // ExportControlGUI::ProcInit

/*===========================================================================*/

void ExportControlGUI::ProcUpdate(unsigned long NewSteps, char *Text)
{

if(Text)
	{
	sprintf(PText, "[%s]", Text);
	PText[80] = 0;
	SyncTexts();
	} // if
DoUpdate(NewSteps, PMaxSteps, (time_t)0, PCurSteps, NULL, NULL, NULL, IDC_PROC_PERCENT);

} // ExportControlGUI::ProcUpdate

/*===========================================================================*/

void ExportControlGUI::ProcClear()
{

DoUpdate(0, PMaxSteps, (time_t)0, PCurSteps, NULL, NULL, NULL, IDC_PROC_PERCENT);
PMaxSteps = 0;
PCurSteps = 0;

PText[0] = 0;
SyncTexts();

} // ExportControlGUI::ProcClear

/*===========================================================================*/

void ExportControlGUI::GetProcSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, char *StashText)
{

StashCurSteps = PCurSteps;
StashMaxSteps = PMaxSteps;
strcpy(StashText, PText);

} // ExportControlGUI::GetProcSetup

/*===========================================================================*/

void ExportControlGUI::RestoreProcSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, char *StashText)
{

PMaxSteps = StashMaxSteps;
PCurSteps = StashCurSteps;
ProcUpdate(PCurSteps, StashText);

} // ExportControlGUI::RestoreProcSetup

/*===========================================================================*/

void ExportControlGUI::GetFrameSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, time_t &StashStartSecs, char *StashText)
{

StashCurSteps = FCurSteps;
StashMaxSteps = FMaxSteps;
StashStartSecs = FStartSeconds;
strcpy(StashText, FText);

} // ExportControlGUI::GetFrameSetup

/*===========================================================================*/

void ExportControlGUI::RestoreFrameSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, time_t StashStartSecs, char *StashText)
{

FMaxSteps = StashMaxSteps;
FCurSteps = StashCurSteps;
FStartSeconds = StashStartSecs;
FrameGaugeUpdate(FCurSteps);
FrameTextInit(StashText);

} // ExportControlGUI::RestoreFrameSetup

/*===========================================================================*/

void ExportControlGUI::DoUpdate(unsigned long Step, unsigned long MaxSteps,
	time_t StartSeconds, unsigned long &CurSteps, int Rem, int Elap, int Comp, int GaugeID)
{
time_t NowSecs, Elapsed, Remain, Projected;
unsigned char ElapHrs, ElapMin, ElapSec, RemHrs, RemMin, RemSec;

if (CurSteps != Step)
	{
	ECRepaintFuelGauge(Step, MaxSteps, GaugeID);
	} // if

CurSteps = Step;
if (Step == 0)
	{
	if (Rem)
		WidgetSetText(Rem, "");
	if (Elap)
		WidgetSetText(Elap, "");
	} // if
if (StartSeconds && (Step != 0))
	{
	GetTime(NowSecs);
	Elapsed = NowSecs - StartSeconds;
	ElapSec = (unsigned char)(Elapsed % 60);
	ElapMin = (unsigned char)((Elapsed / 60) % 60);
	ElapHrs = (unsigned char)(Elapsed / 3600);

	// Replace this with the algorithm of your choice, Gary. ;)
	Remain = (Elapsed * MaxSteps / Step) - Elapsed; // Fixed for more accuracy
	RemSec = (unsigned char)(Remain % 60);
	RemMin = (unsigned char)((Remain / 60) % 60);
	RemHrs = (unsigned char)(Remain / 3600);
	Projected = NowSecs + Remain;
	ECPWDate[0] = ECToday[0] = NULL;
	if(GetTimeString(Projected) && GetTimeString(StartSeconds))
		{
		strcpy(ECPWDate, GetTimeString(Projected));
		strcpy(ECToday, GetTimeString(StartSeconds));
		} // if
	if(Elap && Rem)
		{

		ECPWDate[19] = NULL;
		if(strncmp(ECToday, ECPWDate, 10))
			{
			sprintf(ECProjDate, "Finish %s", ECPWDate);
			} // if
		else
			{
			sprintf(ECProjDate, "Finish %s", &ECPWDate[11]);
			} // if
		sprintf(ECPWDate, "Elapsed %02d:%02d:%02d", ElapHrs, ElapMin, ElapSec); // Elapsed
		sprintf(ECToday, "Remaining %02d:%02d:%02d", RemHrs, RemMin, RemSec); // Remaining

		if(Comp)
			{
			WidgetSetText(Comp, ECProjDate);
			} // if
		WidgetSetText(Rem, ECToday); WidgetRepaint(Rem);
		WidgetSetText(Elap, ECPWDate); WidgetRepaint(Elap);
		} // if	
	} // if

CheckAbort();

} // ExportControlGUI::DoUpdate

/*===========================================================================*/

int ExportControlGUI::CheckAbort(void)
{ // Briefly dispatch all events...
#ifdef _WIN32
MSG BusyEvent;
#endif // _WIN32

if (this && NativeWin)
	{
	// Special event checking
	#ifdef _WIN32
	while (LocalWinSys()->CheckNoWait(&BusyEvent))
		{
		GlobalApp->ProcessOSEvent(&BusyEvent);
		} // while
	#endif // _WIN32
	} // if

return (Run);

} // ExportControlGUI::CheckAbort

/*===========================================================================*/

void ExportControlGUI::StashFrame(unsigned long FrameTime)
{
int Chop;
unsigned char ElapHrs, ElapMin, ElapSec;

if (Rend->IsCamView)
	{
	return;
	} // if

for (Chop = 0; ProgCap[Chop]; Chop++)
	{
	if ((ProgCap[Chop] == ':') || (ProgCap[Chop] == ','))
		{
		ProgCap[Chop] = NULL;
		break;
		} // if
	} // for

// Calc time for rendering frame
ElapSec = (unsigned char)(FrameTime % 60);
ElapMin = (unsigned char)((FrameTime / 60) % 60);
ElapHrs = (unsigned char)(FrameTime / 3600);
sprintf(PrevCap, "%s [%02d:%02d:%02d]", ProgCap, ElapHrs, ElapMin, ElapSec);
ProgCap[0] = NULL;
SyncTexts();

} // ExportControlGUI::StashFrame

/*===========================================================================*/

void ExportControlGUI::ECRepaintFuelGauge(unsigned long CurSteps, unsigned long MaxSteps, int GaugeID)
{
NativeControl Me;
long Steps;
int Percent;


Steps = CurSteps;

Percent = 0;
Me = GetDlgItem(NativeWin, GaugeID);
if((Steps > 0) && (MaxSteps > 0))
	{
	Percent = (long)(100 * ((float)Steps / (float)MaxSteps));
	if((Percent == 0) && ((100 * ((float)Steps / (float)MaxSteps)) > 0))
		{
		Percent = 1;
		} // if
	} // if
#ifdef _WIN32
SendMessage(Me, WM_WCSW_CB_NEWVAL, 0, (LPARAM)Percent);
#endif // _WIN32
} // ExportControlGUI::RepaintFuelGauge

/*===========================================================================*/

void ExportControlGUI::UpdatePreviewState(void)
{

if (Preview)
	{
	if (Rend)
		{
		Rend->DrawPreview();
		} // if
	} // if
else
	{
	if (Rend)
		{
		Rend->ClosePreview();
		} // if
	} // else

} // ExportControlGUI::UpdatePreviewState

/*===========================================================================*/

void ExportControlGUI::SetPreviewCheck(int State)
{

WidgetSetCheck(IDC_DISPLAYPREV, State, NativeWin);

} // ExportControlGUI::SetPreviewCheck

/*===========================================================================*/

int ExportControlGUI::ResampleRaster(const char *InputPath, const char *OutputPath, int BandType, long InRows, long InCols,
	long OutRows, long OutCols, float NullValue, int HonorNull)
{
RasterResampler *Resamp = NULL;
int Success = 0;

switch (BandType)
	{
	case WCS_RESAMPLE_BANDTYPE_UBYTE:
		{
		Resamp = new UByteRasterResampler();
		break;
		} // WCS_RESAMPLE_BANDTYPE_UBYTE
	case WCS_RESAMPLE_BANDTYPE_FLOAT:
		{
		Resamp = new FloatRasterResampler();
		break;
		} // WCS_RESAMPLE_BANDTYPE_FLOAT
	default:
		{
		UserMessageOK("Raster Resample", "Unimplemented variable size detected. Resampling aborted.");
		return (0);
		} // default
	} // switch

if (Resamp)
	{
	if (HonorNull)
		Resamp->SetNull(NullValue);
	Success = Resamp->Resample(InputPath, OutputPath, InRows, InCols, OutRows, OutCols);
	delete Resamp;
	} // if

return (Success);

} // ExportControlGUI::ResampleRaster

/*===========================================================================*/

int ExportControlGUI::ResampleRasterTile(const char *InputPath, const char *OutputPath, int BandType, long InRows, long InCols,
	long OutRows, long OutCols, long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap, float NullValue, int HonorNull)
{
RasterResampler *Resamp = NULL;
int Success = 0;

switch (BandType)
	{
	case WCS_RESAMPLE_BANDTYPE_UBYTE:
		{
		Resamp = new UByteRasterResampler();
		break;
		} // WCS_RESAMPLE_BANDTYPE_UBYTE
	case WCS_RESAMPLE_BANDTYPE_FLOAT:
		{
		Resamp = new FloatRasterResampler();
		break;
		} // WCS_RESAMPLE_BANDTYPE_FLOAT
	default:
		{
		UserMessageOK("Raster Resample", "Unimplemented variable size detected. Resampling aborted.");
		return (0);
		} // default
	} // switch

if (Resamp)
	{
	if (HonorNull)
		Resamp->SetNull(NullValue);
	Success = Resamp->ResampleTile(InputPath, OutputPath, InRows, InCols, OutRows, OutCols, 
		TilesY, TilesX, CurTileY, CurTileX, Overlap);
	delete Resamp;
	} // if

return (Success);

} // ExportControlGUI::ResampleRasterTile

/*===========================================================================*/

void ExportControlGUI::ExportError(char *Title, char *Mesg)
{

UserMessageOK(Title, Mesg);

} // ExportControlGUI::ExportError

/*===========================================================================*/

int ExportControlGUI::FormatSpecificAuth(char *TargetName)
{
#ifdef WCS_BUILD_RTX
if (! stricmp(TargetName, "Custom"))
	return (1);
if (! stricmp(TargetName, "NatureView"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_NVE));
if (! stricmp(TargetName, "VRML"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VRML_WEB));
if (! stricmp(TargetName, "STL"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_STL));
if (! stricmp(TargetName, "VRML-STL"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VRMLSTL));
if (! stricmp(TargetName, "3DS"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_3DS));
if (! stricmp(TargetName, "LightWave"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_LW));
if (! stricmp(TargetName, "VTP"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_VTP));
if (! stricmp(TargetName, "TerraPage"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_TERRAPAGE));
if (! stricmp(TargetName, "SoftImage"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_SOFTIMAGE));
if (! stricmp(TargetName, "Maya"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_MAYA));
if (! stricmp(TargetName, "OpenFlight"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_OPENFLIGHT));
if (! stricmp(TargetName, "GIS"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_GIS));
if (! stricmp(TargetName, "VNS"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_WCSVNS));
if (! stricmp(TargetName, "COLLADA"))
	//return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_COLLADA));
	return(0); // removed for now, not implemented
if (! stricmp(TargetName, "FBX"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_FBX));
if (! stricmp(TargetName, "GoogleEarth"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_KML));
if (! stricmp(TargetName, "WorldWind"))
	return (GlobalApp->Sentinal->CheckFormatRTX(WCS_SECURITY_RTX_FORMAT_WW));

#endif // WCS_BUILD_RTX

return (0);

} // ExportControlGUI::FormatSpecificAuth

/*===========================================================================*/
/*===========================================================================*/

// sort in decreasing order
int CompareWallPolygonPair(const void *elem1, const void *elem2)
{

return (
	((struct WallPolygonPair *)elem1)->SegLen > ((struct WallPolygonPair *)elem2)->SegLen ? -1:
	(((struct WallPolygonPair *)elem1)->SegLen < ((struct WallPolygonPair *)elem2)->SegLen ? 1: 0)
	);

} // CompareWallPolygonPair

// sort in increasing order
int CompareObjPolySort(const void *elem1, const void *elem2)
{

return (
	((struct ObjPolySort *)elem1)->AxisDist > ((struct ObjPolySort *)elem2)->AxisDist ? 1:
	(((struct ObjPolySort *)elem1)->AxisDist < ((struct ObjPolySort *)elem2)->AxisDist ? -1: 0)
	);

} // CompareObjPolySort
