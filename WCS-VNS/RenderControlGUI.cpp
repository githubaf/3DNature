// RenderControlGUI.cpp
// Code for Atmosphere editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RenderControlGUI.h"
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
#include "resource.h"
#include "Script.h" // for network script feedback in BusyWin

extern int AutoStartRender, AutoQuitDone, RenderIconified;
extern int NetScriptRender;
extern NotifyTag ThawEvent[2];

NativeGUIWin RenderControlGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

if (AutoStartRender == 1) // 1 indicates open RenderControl state
	{
	AutoStartRender = 2; // 2 indicates render started state
	if (RenderIconified)
		{
		ShowWindow((HWND)GlobalApp->WinSys->GetRoot(), SW_SHOWMINNOACTIVE);
		} // if
	RenderGo();
	if (RenderIconified)
		{
		ShowWindow((HWND)GlobalApp->WinSys->GetRoot(), SW_SHOWNORMAL);
		} // if
	} // if

return (Success);

} // RenderControlGUI::Open

/*===========================================================================*/

NativeGUIWin RenderControlGUI::Construct(void)
{
char *PriorityLevels[] = {"Low", "Medium", "High"};
int Index;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_RENDERCONTROL1, NULL);

	if (NativeWin)
		{
		for (Index = 0; Index < 3; Index ++)
			{
			WidgetCBInsert(IDC_PRIORITYDROP, -1, PriorityLevels[Index]);
			} // for
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // RenderControlGUI::Construct

/*===========================================================================*/

RenderControlGUI::RenderControlGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, 
	Project *ProjectSource)
: GUIFenetre('RCTL', this, 
APP_TLA" Render Control"
) // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDERJOB, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
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

AText[0] = FText[0] = PText[0] = IText[0] = RText[0] = FRText[0] =
ProgCap[0] = PrevCap[0] = FrameTitle[0] =
	Pause = Run = 0;

PMaxSteps = PCurSteps = FMaxSteps = FCurSteps = AMaxSteps = ACurSteps = 0; // these are unsigned long
FStartSeconds = AStartSeconds = 0; // these are time_t

Preview = 0;
NetNotifyDone = 0;
RegenFDMs = GetRegenFDMs();

if (EffectsHost && DBHost)
	{
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_NODOCK);

} // RenderControlGUI::RenderControlGUI

/*===========================================================================*/

RenderControlGUI::~RenderControlGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

if (Rend)
	delete Rend;

} // RenderControlGUI::~RenderControlGUI()

/*===========================================================================*/

long RenderControlGUI::HandleCloseWin(NativeGUIWin NW)
{

if (Activity->Origin->FenID == 'REND')
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
			WCS_TOOLBAR_ITEM_RCG, 0);
		} // if
	else
		{
		Run = 0;
		} // else
	return(1);
	} // else

} // RenderControlGUI::HandleCloseWin

/*===========================================================================*/

long RenderControlGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
#ifdef _WIN32
MSG msg;
#endif // _WIN32

switch(ButtonID)
	{
	case ID_KEEP:
		{
		RenderStop();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_RCG, 0);
		break;
		} // 
	case IDC_EDIT1:
		{
		if (! Rendering && ActiveJob && ActiveJob->Cam)
			ActiveJob->Cam->EditRAHost();
		break;
		} //
	case IDC_EDIT2:
		{
		if (! Rendering && ActiveJob && ActiveJob->Options)
			ActiveJob->Options->EditRAHost();
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
		RenderGo();
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

} // RenderControlGUI::HandleButtonClick

/*===========================================================================*/

long RenderControlGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RenderControlGUI::HandleListSel

/*===========================================================================*/

long RenderControlGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
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

} // RenderControlGUI::HandleListDelItem

/*===========================================================================*/

long RenderControlGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RenderControlGUI::HandleListDoubleClick

/*===========================================================================*/

long RenderControlGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RenderControlGUI::HandleCBChange

/*===========================================================================*/

long RenderControlGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_DISPLAYPREV:
		{
		Preview = (char)WidgetGetCheck(IDC_DISPLAYPREV);
		UpdatePreviewState();
		break;
		} // 
	case IDC_CHECKREGENFDM:
		{
		SetRegenFDMs(RegenFDMs);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // RenderControlGUI::HandleSCChange

/*===========================================================================*/

void RenderControlGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[3];
int Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDERJOB, WCS_EFFECTSSUBCLASS_RENDERJOB, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_RENDERJOB, WCS_EFFECTSSUBCLASS_RENDERJOB, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildJobList();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	RegenFDMs = GetRegenFDMs();
	WidgetSetDisabled(IDC_CHECKREGENFDM, EffectsHost->TerrainParamBase.FractalMethod != WCS_FRACTALMETHOD_DEPTHMAPS);
	WidgetSCSync(IDC_CHECKREGENFDM, WP_SCSYNC_NONOTIFY);
	} // if

if (! Done)
	ConfigureWidgets();

} // RenderControlGUI::HandleNotifyEvent()

/*===========================================================================*/

void RenderControlGUI::ConfigureWidgets(void)
{

ConfigureTB(NativeWin, IDC_ADDJOB, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_REMOVEJOB, IDI_DISABLE, NULL);
ConfigureTB(NativeWin, IDC_MOVEJOBUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEJOBDOWN, IDI_ARROWDOWN, NULL);

ConfigureSC(NativeWin, IDC_CHECKREGENFDM, &RegenFDMs, SCFlag_Char, NULL, 0);
WidgetSetDisabled(IDC_CHECKREGENFDM, EffectsHost->TerrainParamBase.FractalMethod != WCS_FRACTALMETHOD_DEPTHMAPS);

WidgetSetCheck(IDC_DISPLAYPREV, Preview);

WidgetCBSetCurSel(IDC_PRIORITYDROP, ProjectHost->InquireRenderPri() + 1);

BuildJobList();
DisableWidgets();

} // RenderControlGUI::ConfigureWidgets()

/*===========================================================================*/

void RenderControlGUI::ConfigureJobWidgets(void)
{
long Ct;
char TextStr[256], NoMore = 0;

if (EffectsHost->IsEffectValid(ActiveJob, WCS_EFFECTSSUBCLASS_RENDERJOB, 0))
	{
	WidgetSetText(IDC_TITLE1, ActiveJob->Cam ? ActiveJob->Cam->Name: "No Camera");
	WidgetSetText(IDC_TITLE2, ActiveJob->Options ? ActiveJob->Options->Name: "No Render Options");
	if (ActiveJob->Cam)
		{
		switch (ActiveJob->Cam->CameraType)
			{
			case WCS_EFFECTS_CAMERATYPE_TARGETED:
				{
				strcpy(TextStr, "Targeted");
				if (ActiveJob->Cam->PanoCam)
					strcat(TextStr, " Panoramic");
				if (ActiveJob->Cam->Orthographic)
					strcat(TextStr, ", Orthographic");
				else
					strcat(TextStr, ", Perspective");
				break;
				}
			case WCS_EFFECTS_CAMERATYPE_UNTARGETED:
				{
				strcpy(TextStr, "Non-Targeted");
				if (ActiveJob->Cam->PanoCam)
					strcat(TextStr, " Panoramic");
				if (ActiveJob->Cam->Orthographic)
					strcat(TextStr, ", Orthographic");
				else
					strcat(TextStr, ", Perspective");
				break;
				}
			case WCS_EFFECTS_CAMERATYPE_ALIGNED:
				{
				strcpy(TextStr, "Aligned");
				if (ActiveJob->Cam->PanoCam)
					strcat(TextStr, " Panoramic");
				if (ActiveJob->Cam->Orthographic)
					strcat(TextStr, ", Orthographic");
				else
					strcat(TextStr, ", Perspective");
				break;
				}
			case WCS_EFFECTS_CAMERATYPE_OVERHEAD:
				{
				strcpy(TextStr, "Overhead");
				if (ActiveJob->Cam->Orthographic)
					strcat(TextStr, ", Orthographic");
				else
					strcat(TextStr, ", Perspective");
				break;
				}
			case WCS_EFFECTS_CAMERATYPE_PLANIMETRIC:
				{
				strcpy(TextStr, "Planimetric");
				break;
				}
			default:
				{
				TextStr[0] = 0;
				break;
				} // def
			} // switch
		WidgetSetText(IDC_TITLE3, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_TITLE3, "");
		} // else
	if (ActiveJob->Options)
		{
		sprintf(TextStr, "%d", (long)(ActiveJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * ActiveJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5));
		WidgetSetText(IDC_TITLE4, TextStr);
		sprintf(TextStr, "%d", (long)(ActiveJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * ActiveJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5));
		WidgetSetText(IDC_TITLE5, TextStr);
		sprintf(TextStr, "%d", ActiveJob->Options->FrameStep);
		WidgetSetText(IDC_TITLE6, TextStr);
		TextStr[0] = 0;
		if (ActiveJob->Options->OutputImages())
			{
			strcat(TextStr, "Images");
			} // if
		if (ActiveJob->Options->OutputZBuffer())
			{
			if (TextStr[0])
				strcat(TextStr, ", ");
			strcat(TextStr, "Z-Buffer");
			} // if
		if (ActiveJob->Options->OutputDiagnosticData())
			{
			if (TextStr[0])
				strcat(TextStr, ", ");
			strcat(TextStr, "Diagnostic Data");
			} // if
		WidgetSetText(IDC_TITLE7, TextStr);
		TextStr[0] = 0;
		if (! ActiveJob->Options->TerrainEnabled)
			strcat(TextStr, "Terrain");
		if (! ActiveJob->Options->FoliageEnabled)
			strcat(TextStr, TextStr[0] ? ", Eco Foliage": "Eco Foliage");
		if (! ActiveJob->Options->VectorsEnabled)
			strcat(TextStr, TextStr[0] ? ", Vectors": "Vectors");
		for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS; Ct ++)
			{
			if (! ActiveJob->Options->EffectEnabled[Ct] && EffectsHost->EffectTypeImplemented(Ct))
				{
				if (strlen(TextStr) + strlen(EffectsHost->GetEffectTypeName(Ct)) + 2 < 42)
					{
					if (TextStr[0])
						{
						strcat(TextStr, ", ");
						strcat(TextStr, EffectsHost->GetEffectTypeName(Ct));
						} // if
					else
						strcat(TextStr, EffectsHost->GetEffectTypeName(Ct));
					} // if
				else
					{
					strcat(TextStr, " && More");
					NoMore = 1;
					break;
					} // else
				} // if
			} // for
		if (! NoMore && ! ActiveJob->Options->ReflectionsEnabled)
			{
			if (strlen(TextStr) + strlen(", Reflections") < 42)
				strcat(TextStr, TextStr[0] ? ", Reflections": "Reflections");
			else
				{
				strcat(TextStr, " && More");
				NoMore = 1;
				} // else
			} // if
		if (! NoMore && ActiveJob->Options->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && ! ActiveJob->Options->ObjectShadowsEnabled)
			{
			if (strlen(TextStr) + strlen(", 3D Object Shadows") < 42)
				strcat(TextStr, TextStr[0] ? ", 3D Object Shadows": "3D Object Shadows");
			else
				{
				strcat(TextStr, " && More");
				NoMore = 1;
				} // else
			} // if
		if (! NoMore && ActiveJob->Options->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && ! ActiveJob->Options->CloudShadowsEnabled)
			{
			if (strlen(TextStr) + strlen(", Cloud Shadows") < 42)
				strcat(TextStr, TextStr[0] ? ", Cloud Shadows": "Cloud Shadows");
			else
				{
				strcat(TextStr, " && More");
				NoMore = 1;
				} // else
			} // if
		if (! NoMore && ! ActiveJob->Options->DepthOfFieldEnabled)
			{
			if (strlen(TextStr) + strlen(", Depth of Field") < 42)
				strcat(TextStr, TextStr[0] ? ", Depth of Field": "Depth of Field");
			else
				{
				strcat(TextStr, " && More");
				NoMore = 1;
				} // else
			} // if
		if (! NoMore && ! ActiveJob->Options->MultiPassAAEnabled)
			{
			if (strlen(TextStr) + strlen(", Multipass AA") < 42)
				strcat(TextStr, TextStr[0] ? ", Multipass AA": "Multipass AA");
			else
				{
				strcat(TextStr, " && More");
				NoMore = 1;
				} // else
			} // if
		WidgetSetText(IDC_TITLE8, TextStr);
		} // if
	else
		{
		WidgetSetText(IDC_TITLE4, "");
		WidgetSetText(IDC_TITLE5, "");
		WidgetSetText(IDC_TITLE6, "");
		WidgetSetText(IDC_TITLE7, "");
		WidgetSetText(IDC_TITLE8, "");
		} // else
	} // if
else
	{
	WidgetSetText(IDC_TITLE1, "");
	WidgetSetText(IDC_TITLE2, "");
	WidgetSetText(IDC_TITLE3, "");
	WidgetSetText(IDC_TITLE4, "");
	WidgetSetText(IDC_TITLE5, "");
	WidgetSetText(IDC_TITLE6, "");
	WidgetSetText(IDC_TITLE7, "");
	WidgetSetText(IDC_TITLE8, "");
	} // else

} // RenderControlGUI::ConfigureJobWidgets

/*===========================================================================*/

void RenderControlGUI::BuildJobList(void)
{
RenderJob *Current, **JobList;
long Done, Pos, Found = 0, JobCt = 0;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

WidgetLBClear(IDC_JOBLIST);

// build a temporary list of jobs that can be sorted by priority
// count jobs
for (Current = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); 
	Current; Current = (RenderJob *)Current->Next, JobCt ++);	//lint !e722

if (JobCt > 0)
	{
	if (JobList = (RenderJob **)AppMem_Alloc(JobCt * sizeof (RenderJob *), 0))
		{
		for (Pos = 0, Current = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); 
			Current; Current = (RenderJob *)Current->Next, Pos ++)
			{
			JobList[Pos] = Current;
			} // for

		Done = 0;
		while (! Done)
			{
			Done = 1;
			for (Pos = 1; Pos < JobCt; Pos ++)
				{
				if (JobList[Pos - 1]->Priority < JobList[Pos]->Priority)
					{
					swmem(&JobList[Pos - 1], &JobList[Pos], sizeof (RenderJob *));
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
		AppMem_Free(JobList, JobCt * sizeof (RenderJob *));
		} // if

	if (! Found)
		{
		if (ActiveJob = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB))
			{
			WidgetLBSetCurSel(IDC_JOBLIST, 0);
			}
		} // if
	} // if

ConfigureJobWidgets();

} // RenderControlGUI::BuildJobList

/*===========================================================================*/

void RenderControlGUI::BuildJobListEntry(char *ListName, RenderJob *Me)
{

if (Me->Enabled && Me->Cam && Me->Options)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->Name);

} // RenderControlGUI::BuildJobListEntry()

/*===========================================================================*/

void RenderControlGUI::DisableWidgets(void)
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

} // RenderControlGUI::DisableWidgets

/*===========================================================================*/

void RenderControlGUI::EnableJob(void)
{
NotifyTag Changes[2];

if (ActiveJob)
	{
	if (! ActiveJob->Cam)
		{
		if (UserMessageYN(ActiveJob->Name, "Selected Render Job does not have a Camera and can not be enabled.\nDo you wish to edit the Render Job?"))
			EditJob();
		} // if
	else if (! ActiveJob->Options)
		{
		if (UserMessageYN(ActiveJob->Name, "Selected Render Job does not have Render Options and can not be enabled.\nDo you wish to edit the Render Job?"))
			EditJob();
		} // if
	else
		{
		ActiveJob->Enabled = 1;
		Changes[0] = MAKE_ID(ActiveJob->GetNotifyClass(), ActiveJob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveJob->GetRAHostRoot());
		} // if
	} // if

} // RenderControlGUI::EnableJob

/*===========================================================================*/

void RenderControlGUI::DisableJob(void)
{
NotifyTag Changes[2];

if (ActiveJob)
	{
	ActiveJob->Enabled = 0;
	Changes[0] = MAKE_ID(ActiveJob->GetNotifyClass(), ActiveJob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveJob->GetRAHostRoot());
	} // if

} // RenderControlGUI::DisableJob

/*===========================================================================*/

void RenderControlGUI::ChangeJobPriority(short Direction)
{
RenderJob *NextItem;
long Current;
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
				if ((NextItem = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Current - 1)) && NextItem != (RenderJob *)LB_ERR)
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
				if ((NextItem = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Current + 1)) && NextItem != (RenderJob *)LB_ERR)
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

} // RenderControlGUI::ChangeJobPriority

/*===========================================================================*/

void RenderControlGUI::EditJob(void)
{
RasterAnimHost *EditMe;
long Ct, NumListEntries;

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

} // RenderControlGUI::EditJob

/*===========================================================================*/

void RenderControlGUI::SetActiveJob(void)
{
RenderJob *NewJob;
long Current;

Current = WidgetLBGetCurSel(IDC_JOBLIST);
if ((NewJob = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Current, 0)) != (RenderJob *)LB_ERR && NewJob)
	{
	if (NewJob != ActiveJob)
		{
		ActiveJob = NewJob;
		ConfigureJobWidgets();
		} // if
	} // if
else
	ConfigureJobWidgets();

} // RenderControlGUI::SetActiveJob

/*===========================================================================*/

void RenderControlGUI::PriorityLevel(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_PRIORITYDROP);
ProjectHost->SetRenderPri((short)(Current - 1));

} // RenderControlGUI::PriorityLevel

/*===========================================================================*/

#define STARTTIME	CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue
#define ENDTIME		CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue
#define FRAMESTEP	(CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue >= CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue ? CurJob->Options->FrameStep: -CurJob->Options->FrameStep)
#define FRAMERATE	CurJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue

void RenderControlGUI::RenderGo(void)
{
char StashPreviewState, CurRenderAnim, ReqDigits, DigitsWarned = 0, RenderAnim = 0;
long Ct, NumListEntries, CurFrame, EndFrame, JobsEnabled = 0, DoneJobs = 0, LastFrameRendered = 0;
#ifdef WCS_RENDER_SCENARIOS
long FirstFrame;
time_t StartSecs, FirstSecs, FrameSecs, NowTime;
unsigned long FramesToRender, FramesRendered;
Thumbnail BackupThumb;
#endif // WCS_RENDER_SCENARIOS
double CurTime;
RenderJob *CurJob;
ImageOutputEvent *CurEvent;
#ifdef WCS_RENDER_TILES
NotifyTag Changes[2];
#endif // WCS_RENDER_TILES
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Rendering is somewhat slower in the demo version.");
#endif // WCS_BUILD_DEMO

// complain if a fractal map is not found
FrdWarned = 0;

if ((NumListEntries = WidgetLBGetCount(IDC_JOBLIST)) > 0)
	{
	// count enabled jobs so we know when we are done
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (CurJob = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Ct))
			{
			if (CurJob->Enabled && CurJob->Cam && CurJob->Options)
				{
				JobsEnabled ++;
				CurRenderAnim = 0;
				// check to see if this is an animated frame range and autodigits not enough
				CurFrame = (long)(STARTTIME * FRAMERATE + .5);
				EndFrame = (long)(ENDTIME * FRAMERATE + .5);
				if (CurFrame != EndFrame)
					CurRenderAnim = RenderAnim = 1;
				ReqDigits = EndFrame > 99999 ? 6: EndFrame > 9999 ? 5: EndFrame > 999 ? 4: EndFrame > 99 ? 3: EndFrame > 9 ? 2: EndFrame > 0 ? 1: 0;
				for (CurEvent = CurJob->Options->OutputEvents; CurEvent; CurEvent = CurEvent->Next)
					{
					if (CurRenderAnim && CurEvent->Enabled && CurEvent->AutoDigits < ReqDigits)
						{
						if (DigitsWarned || UserMessageYN("Output Auto-digits", "You are about to render an animation in which the frame numbers exceed the range suitable for the number of automatic digits specified in Image Output Events. Failure to provide enough digits may result in rendered frames overwriting each other. Would you like the number of digits increased to a suitable value?"))
							{
							DigitsWarned = 1;
							CurEvent->AutoDigits = ReqDigits;
							} // if
						} // if
					} // for
				} // if
			} // if
		} // for

	// warn user if creation rendered animation with variable frd
	if (RegenFDMs)
		{
		if (! EffectsHost->TerrainParamBase.CreateFractalMaps(FALSE))
			return;
		} // if
	else
		{
		if (EffectsHost->GetFractalMethod() == WCS_FRACTALMETHOD_VARIABLE && RenderAnim)
			{
			RenderAnim = UserMessageCustom("Variable Fractal Method", 
				"Warning! You are about to render an animation with the Fractal method set to Variable. That is not recommended. \nPlease select the method you wish to use by clicking the appropriate button below.", "Depth Maps", "Constant", "Variable", 0);
			if (RenderAnim == 0)
				EffectsHost->SetFractalMethod(WCS_FRACTALMETHOD_CONSTANT);
			else if (RenderAnim == 1)
				{
				if (! (RenderAnim = UserMessageYNCAN("Depth Maps", "Would you like to generate Fractal Depth Maps now?")))
					return;
				else if (RenderAnim == 2)
					{
					if (! EffectsHost->TerrainParamBase.CreateFractalMaps(FALSE))
						return;
					} // else if
				EffectsHost->SetFractalMethod(WCS_FRACTALMETHOD_DEPTHMAPS);
				} // else if
			} // if
		} // else

	// warn user if rendering with terraffectors and backface culling
	if (EffectsHost->GetBackFaceCull() && (EffectsHost->EnabledEffectExists(WCS_EFFECTSSUBCLASS_TERRAFFECTOR) || EffectsHost->EnabledEffectExists(WCS_EFFECTSSUBCLASS_RASTERTA)))
		{
		RenderAnim = UserMessageCustom("Back Face Culling", 
			"Warning! You are about to render Terraffectors using back-face polygon culling. That may produce holes in the terrain. \nPlease select the option you wish to use by clicking the appropriate button below.", "Back-face Culling", "No Back-face Culling", NULL, 0);
		if (RenderAnim == 0)
			{
			EffectsHost->SetBackFaceCull(0);
			if (EffectsHost->GetFractalMethod() == WCS_FRACTALMETHOD_DEPTHMAPS)
				{
				if (! (RenderAnim = UserMessageYNCAN("Depth Maps", "You have changed a Terrain Parameter setting that affects Fractal Depth Maps. Would you like to regenerate Fractal Depth Maps now?")))
					return;
				else if (RenderAnim == 2)
					{
					if (! EffectsHost->TerrainParamBase.CreateFractalMaps(FALSE))
						return;
					} // else if
				} // if depth maps
			} // else if
		} // if

	// warn if invalid output directory
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (CurJob = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Ct))
			{
			if (CurJob->Enabled && CurJob->Cam && CurJob->Options)	// do all enabled jobs
				{
				if (! CurJob->Options->ValidateOutputPaths())
					return;
				} // if
			} // if
		} // for

	#ifdef WCS_RENDER_TILES
	// warn if conflicting render settings
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (CurJob = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Ct))
			{
			if (CurJob->Enabled && CurJob->Cam && CurJob->Options)	// do all enabled jobs
				{
				if (CurJob->Options->TilingEnabled)
					{
					if (CurJob->Cam->PanoCam || CurJob->Cam->StereoCam || CurJob->Cam->AAPasses > 1 || CurJob->Cam->FieldRender)
						{
						if (! (RenderAnim = UserMessageCustom(CurJob->Name, "Camera settings in this Job are incompatible with \
Tiled image output. Do you wish to modify Camera settings or render without Tiled output? The illegal settings are: Panoramic or Stereo Cameras, \
Field Rendering, Multi-pass antialiasing and Motion Blur.", "Modify Camera", "Cancel", "No Tiled Output", 0)))
							return;
						else if (RenderAnim == 1)
							{
							// modify camera
							CurJob->Cam->PanoCam = 0;
							CurJob->Cam->StereoCam = 0;
							CurJob->Cam->AAPasses = 1;
							CurJob->Cam->MotionBlur = 0;
							CurJob->Cam->FieldRender = 0;
							Changes[0] = MAKE_ID(CurJob->Cam->GetNotifyClass(), CurJob->Cam->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, CurJob->Cam->GetRAHostRoot());
							} // else if
						else // (RenderAnim == 2)
							{
							// modify tiling
							CurJob->Options->TilingEnabled = 0;
							Changes[0] = MAKE_ID(CurJob->Options->GetNotifyClass(), CurJob->Options->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, CurJob->Options->GetRAHostRoot());
							} // else if
						} // if
					if (CurJob->Options->OutputImageHeight % CurJob->Options->TilesY || CurJob->Options->OutputImageWidth % CurJob->Options->TilesX)
						{
						if (! UserMessageYN(CurJob->Name, "This Job's image output width or height is not evenly divisible by the number of tiles. \
Image size will be reduced by a few pixels to make the width and height evenly divisible. Do you wish to continue?"))
							return;
						} // if
					} // if
				} // if
			} // if
		} // for
	#endif // WCS_RENDER_TILES

	Rendering = 1;
	Run = 1;
	DisableWidgets();

	for (Ct = 0; Ct < NumListEntries && Run; Ct ++)
		{
		if (CurJob = (RenderJob *)WidgetLBGetItemData(IDC_JOBLIST, Ct))
			{
			if (CurJob->Enabled && CurJob->Cam && CurJob->Options)	// do all enabled jobs
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
				if (Rend = new Renderer())
					{
					#ifdef WCS_RENDER_SCENARIOS
					FirstFrame = (long)(STARTTIME * FRAMERATE + .5);
					CurTime = FirstFrame / FRAMERATE;
					// init enabled states at first frame to be rendered before init effects
					EffectsHost->InitScenarios(CurJob->Scenarios, CurTime, DBHost);
					#endif // WCS_RENDER_SCENARIOS
					if (Rend->Init(CurJob, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, TRUE))
						{
						// only complain once per render session
						Rend->SetFrdWarned(FrdWarned);
						CurFrame = (long)(STARTTIME * FRAMERATE + .5);
						EndFrame = (long)(ENDTIME * FRAMERATE + .5);


						#ifdef WCS_BUILD_DEMO
						if (CurFrame > 150 || EndFrame > 150)
							{
							UserMessageDemo("Highest frame number you can render is 150.");
							if (CurFrame > 150)
								CurFrame = 150;
							if (EndFrame > 150)
								EndFrame = 150;
							} // if
						#endif // WCS_BUILD_DEMO

						for ( ; (FRAMESTEP > 0) ? CurFrame <= EndFrame: CurFrame >= EndFrame; CurFrame += FRAMESTEP)
							{
							CurTime = CurFrame / FRAMERATE;
							LastFrameRendered = CurFrame;

							#ifdef WCS_BUILD_DEMO
							if (CurTime > 5.0)
								{
								UserMessageDemo("Highest time you can render is 5 seconds.");
								CurTime = 5.0;
								} // if
							#endif // WCS_BUILD_DEMO

							#ifdef WCS_RENDER_SCENARIOS
							// update scenarios
							if (FirstFrame != CurFrame && EffectsHost->UpdateScenarios(CurJob->Scenarios, CurTime, DBHost))
								{
								StartSecs = Rend->StartSecs;
								FirstSecs = Rend->FirstSecs;
								FrameSecs = Rend->FrameSecs;
								NowTime = Rend->NowTime;
								FramesToRender = Rend->FramesToRender;
								FramesRendered = Rend->FramesRendered;
								BackupThumb.Copy(&BackupThumb, Rend->LastRast->Thumb);
								StashPreviewState = Preview;	// store to recall after renderer is deleted
								Rend->Cleanup(false, false, -1, ! ProjectHost->Prefs.PublicQueryConfigOptTrue("suppress_diagnosticwin"));	// -1 prevents certain actions which should only be done at final completion
								GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
								ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
								ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
								delete Rend;
								if (Rend = new Renderer())
									{
									SetPreview(StashPreviewState);	// restore this setting so preview window can open for next job
									if (! Rend->Init(CurJob, GlobalApp, EffectsHost, ImageHost, DBHost, ProjectHost, GlobalApp->StatusLog, this, NULL, FALSE))
										break;
									Rend->StartSecs = StartSecs;
									Rend->FirstSecs = FirstSecs;
									Rend->FrameSecs = FrameSecs;
									Rend->NowTime = NowTime;
									Rend->FramesToRender = FramesToRender;
									Rend->FramesRendered = FramesRendered;
									BackupThumb.Copy(Rend->LastRast->Thumb, &BackupThumb);
									} // if
								else
									break;
								} // if
							#endif // WCS_RENDER_SCENARIOS
							// render a frame
							if (! Rend->RenderFrame(CurTime, CurFrame) || ! Run)
								break;

							// update previous frame thumbnail
							UpdateLastFrame();
							// post time to status log
							Rend->LogElapsedTime(CurFrame, true, false, false); // frame time only, no job time or popup
							Rend->ReportPixelFragResources();
							Rend->ReportQuantaResources();
							Rend->RealTimeFoliageWrite = NULL;
							} // for
						GlobalApp->WinSys->ClearRenderTitle();
						// open message to tell final render time for frame
						GlobalApp->WinSys->DoBeep();
						DoneJobs ++;
						#ifdef WCS_RENDER_SCENARIOS
						EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
						#endif // WCS_RENDER_SCENARIOS
						if (Rend)
							{
							Rend->LogElapsedTime(LastFrameRendered, false, true, false); // job time, but not frame time or popup
							if (DoneJobs == JobsEnabled)
								Rend->LogElapsedTime(LastFrameRendered, false, false, true); // no frame/job time, just show the popup
							Rend->Cleanup(true, false, TRUE, ! ProjectHost->Prefs.PublicQueryConfigOptTrue("suppress_diagnosticwin"));
							GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
							// steal this from renderer in case another job follows
							FrdWarned = Rend->GetFrdWarned();
							} // if
						} // if
					else
						{
						#ifdef WCS_RENDER_SCENARIOS
						EffectsHost->RestoreScenarios(CurJob->Scenarios, DBHost);
						#endif // WCS_RENDER_SCENARIOS
						Rend->Cleanup(false, false, FALSE, false);	// need to delete anything created during failed init process.
						GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
						ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
						ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
						} // else
					} // if 
				} // if
			} // if
		} // for
	RenderStop();
	} // if

if (AutoQuitDone)
	{
	GlobalApp->SetTerminate();
	} // if

} // RenderControlGUI::RenderGo

/*===========================================================================*/

void RenderControlGUI::RenderStop(void)
{

Rendering = 0;
Run = 0;
DisableWidgets();

} // RenderControlGUI::RenderStop

/*===========================================================================*/

void RenderControlGUI::SyncTexts(void)
{

if (NativeWin)
	{
	WidgetSetText(IDC_ANIMSTAT, AText);
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

} // RenderControlGUI::SyncTexts

/*===========================================================================*/

void RenderControlGUI::ClearAll(void)
{

AText[0] = FText[0] = PText[0] = 0;
IText[0] = RText[0] = FRText[0] = 0;
SyncTexts();

} // RenderControlGUI::ClearAll

/*===========================================================================*/

void RenderControlGUI::SetProcText(char *Text)
{

sprintf(PText, "[%s]", Text);
SyncTexts();

} // RenderControlGUI::SetProcText

/*===========================================================================*/

void RenderControlGUI::SetAnimText(char *Text)
{

strncpy(AText, Text, 80);
AText[80] = 0;
SyncTexts();

} // RenderControlGUI::SetAnimText

/*===========================================================================*/

void RenderControlGUI::SetFrameNum(char *FrameNum)
{

if (strlen(FrameNum) < 8)
	{
	sprintf(ProgCap, "%s: in progress", FrameNum);
	} // if
else
	{
	sprintf(ProgCap, "%s:", FrameNum);
	} // else
sprintf(FrameTitle, "Frame %s", FrameNum);
SyncTexts();

} // RenderControlGUI::SetFrameNum

/*===========================================================================*/

void RenderControlGUI::SetFrameText(char *Text)
{

sprintf(FText, "[%s]", Text);
FText[29] = NULL;
SyncTexts();

} // RenderControlGUI::SetFrameText

/*===========================================================================*/

void RenderControlGUI::SetImageText(char *Text)
{

strncpy(IText, Text, 80);
IText[80] = 0;
SyncTexts();

} // RenderControlGUI::SetImageText

/*===========================================================================*/

void RenderControlGUI::SetResText(char *Text)
{

strncpy(RText, Text, 80);
RText[80] = 0;
SyncTexts();

} // RenderControlGUI::SetResText

/*===========================================================================*/

void RenderControlGUI::SetFractText(char *Text)
{

strncpy(FRText, Text, 80);
FRText[80] = 0;
SyncTexts();

} // RenderControlGUI::SetFractText

/*===========================================================================*/

void RenderControlGUI::SetPreview(char PreviewOn)
{

Preview = PreviewOn;
WidgetSetCheck(IDC_DISPLAYPREV, Preview);

} // RenderControlGUI::SetPreview

/*===========================================================================*/

void RenderControlGUI::UpdateStamp(void)
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

} // RenderControlGUI::UpdateStamp

/*===========================================================================*/

void RenderControlGUI::UpdateLastFrame(void)
{

Rend->Rast->Thumb->Copy(Rend->LastRast->Thumb, Rend->Rast->Thumb);

ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, Rend->LastRast);

} // RenderControlGUI::UpdateLastFrame

/*===========================================================================*/

void RenderControlGUI::AnimInit(unsigned long Steps, char *Text)
{

AMaxSteps = Steps;
AnimUpdate(0, Text);

ACurSteps = 0;
AStartSeconds = 0;
GetTime(AStartSeconds);

} // RenderControlGUI::AnimInit

/*===========================================================================*/

void RenderControlGUI::AnimUpdate(unsigned long NewSteps, char *Text)
{

if (Text)
	{
	sprintf(AText, "[%s]", Text);
	AText[80] = 0;
	SyncTexts();
	} // if
DoUpdate(NewSteps, AMaxSteps, AStartSeconds, ACurSteps, IDC_ANIM_REMAIN,
	IDC_ANIM_ELAPSE, NULL, IDC_ANIM_PERCENT);

} // RenderControlGUI::AnimUpdate

/*===========================================================================*/

void RenderControlGUI::AnimClear()
{

DoUpdate(0, AMaxSteps, AStartSeconds, ACurSteps, IDC_ANIM_REMAIN,
	IDC_ANIM_ELAPSE, NULL, IDC_ANIM_PERCENT);
AMaxSteps = 0;
ACurSteps = 0;
AStartSeconds = 0;

AText[0] = 0;
SyncTexts();

} // RenderControlGUI::AnimClear

/*===========================================================================*/

void RenderControlGUI::FrameTextInit(char *Text)
{

if (Text)
	{
	sprintf(FText, "[%s]", Text);
	FText[80] = 0;
	SyncTexts();
	} // if

} // RenderControlGUI::FrameTextInit

/*===========================================================================*/

void RenderControlGUI::FrameTextClear()
{

FText[0] = 0;
SyncTexts();

} // RenderControlGUI::FrameTextClear

/*===========================================================================*/

void RenderControlGUI::FrameGaugeInit(unsigned long Steps)
{

FMaxSteps = Steps;
FrameGaugeUpdate(0);
FCurSteps = 0;
FStartSeconds = 0;
GetTime(FStartSeconds);

} // RenderControlGUI::FrameGaugeInit

/*===========================================================================*/

void RenderControlGUI::FrameGaugeUpdate(unsigned long NewSteps)
{

DoUpdate(NewSteps, FMaxSteps, FStartSeconds, FCurSteps,
	IDC_FRAME_REMAIN, IDC_FRAME_ELAPSE, NULL, IDC_FRAME_PERCENT);

} // RenderControlGUI::FrameGaugeUpdate

/*===========================================================================*/

void RenderControlGUI::FrameGaugeClear()
{

DoUpdate(0, FMaxSteps, FStartSeconds, FCurSteps,
	IDC_FRAME_REMAIN, IDC_FRAME_ELAPSE, NULL, IDC_FRAME_PERCENT);
FMaxSteps = 0;
FCurSteps = 0;
FStartSeconds = 0;

} // RenderControlGUI::FrameGaugeClear

/*===========================================================================*/

void RenderControlGUI::ProcInit(unsigned long Steps, char *Text)
{

PMaxSteps = Steps;
ProcUpdate(0, Text);
PCurSteps = 0;

} // RenderControlGUI::ProcInit

/*===========================================================================*/

void RenderControlGUI::ProcUpdate(unsigned long NewSteps, char *Text)
{

if (Text)
	{
	sprintf(PText, "[%s]", Text);
	PText[80] = 0;
	SyncTexts();
	} // if
DoUpdate(NewSteps, PMaxSteps, (time_t)0, PCurSteps, NULL, NULL, NULL, IDC_PROC_PERCENT);

} // RenderControlGUI::ProcUpdate

/*===========================================================================*/

void RenderControlGUI::ProcClear()
{

DoUpdate(0, PMaxSteps, (time_t)0, PCurSteps, NULL, NULL, NULL, IDC_PROC_PERCENT);
PMaxSteps = 0;
PCurSteps = 0;

PText[0] = 0;
SyncTexts();

} // RenderControlGUI::ProcClear

/*===========================================================================*/

void RenderControlGUI::GetProcSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, char *StashText)
{

StashCurSteps = PCurSteps;
StashMaxSteps = PMaxSteps;
strcpy(StashText, PText);

} // RenderControlGUI::GetProcSetup

/*===========================================================================*/

void RenderControlGUI::RestoreProcSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, char *StashText)
{

PMaxSteps = StashMaxSteps;
PCurSteps = StashCurSteps;
ProcUpdate(PCurSteps, StashText);

} // RenderControlGUI::RestoreProcSetup

/*===========================================================================*/

void RenderControlGUI::GetFrameSetup(unsigned long &StashCurSteps, unsigned long &StashMaxSteps, time_t &StashStartSecs, char *StashText)
{

StashCurSteps = FCurSteps;
StashMaxSteps = FMaxSteps;
StashStartSecs = FStartSeconds;
strcpy(StashText, FText);

} // RenderControlGUI::GetFrameSetup

/*===========================================================================*/

void RenderControlGUI::RestoreFrameSetup(unsigned long StashCurSteps, unsigned long StashMaxSteps, time_t StashStartSecs, char *StashText)
{

FMaxSteps = StashMaxSteps;
FCurSteps = StashCurSteps;
FStartSeconds = StashStartSecs;
FrameGaugeUpdate(FCurSteps);
FrameTextInit(StashText);

} // RenderControlGUI::RestoreFrameSetup

/*===========================================================================*/

extern char NetStatus[100];
static time_t NetTimeout;

void RenderControlGUI::DoUpdate(unsigned long Step, unsigned long MaxSteps,
	time_t StartSeconds, unsigned long &CurSteps, int Rem, int Elap, int Comp, int GaugeID)
{
time_t NowSecs, Elapsed, Remain, Projected;
unsigned char ElapHrs, ElapMin, ElapSec,
	RemHrs, RemMin, RemSec;
static char NetCount = 0;	// because Lint prefers this
char NewNetCount, AbortFromNet = 0;

if (CurSteps != Step)
	{
	ECRepaintFuelGauge(Step, MaxSteps, GaugeID);
	} // if
if (NetScriptRender && CurSteps != Step)
	{ // report ongoing progress to net script client
	if (MaxSteps != 0)
		{
		NewNetCount = (char)(((float)Step / (float)MaxSteps) * 100.0);
		if (NetCount != NewNetCount)
			{
			// limit net feedback to once per second max
			time_t NetTimeNow;
			time(&NetTimeNow);
			if (NetTimeNow != NetTimeout)
				{
				NetTimeout = NetTimeNow;
				sprintf(NetStatus, "PROGRESS=\"%s: %d%% Complete\"\r\n", PText, NetCount);
				GlobalApp->SuperScript->NetSendString(NetStatus);
				} // if
			NetCount = NewNetCount;
			} // if
		} // if
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
	if (GetTimeString(Projected) && GetTimeString(StartSeconds))
		{
		strcpy(ECPWDate, GetTimeString(Projected));
		strcpy(ECToday, GetTimeString(StartSeconds));
		} // if
	if (Elap && Rem)
		{

		ECPWDate[19] = NULL;
		if (strncmp(ECToday, ECPWDate, 10))
			{
			sprintf(ECProjDate, "Finish %s", ECPWDate);
			} // if
		else
			{
			sprintf(ECProjDate, "Finish %s", &ECPWDate[11]);
			} // if
		sprintf(ECPWDate, "Elapsed %02d:%02d:%02d", ElapHrs, ElapMin, ElapSec); // Elapsed
		sprintf(ECToday, "Remaining %02d:%02d:%02d", RemHrs, RemMin, RemSec); // Remaining

		if (Comp)
			{
			WidgetSetText(Comp, ECProjDate);
			} // if
		WidgetSetText(Rem, ECToday); WidgetRepaint(Rem);
		WidgetSetText(Elap, ECPWDate); WidgetRepaint(Elap);
		} // if	
	} // if

CheckAbort();
if (GlobalApp->SuperScript->NetCheckAbort())
	{
	AbortFromNet = 1;
	Run = 0; // stop process
	} // if
if (!Run)
	{
	// report abort to socket
	if (!NetNotifyDone)
		{
		GlobalApp->SuperScript->NetSendString("PROGRESS=\"Aborted\"\r\n");
		NetNotifyDone = 1; // one-shot enforcement
		} // if
	} // if
if (AbortFromNet)
	{ // pull our own plug
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
		WCS_TOOLBAR_ITEM_RCG, 0);
	} // if

} // RenderControlGUI::DoUpdate

/*===========================================================================*/

int RenderControlGUI::CheckAbort(void)
{ // Briefly dispatch all events...
#ifdef _WIN32
MSG BusyEvent;
#endif // _WIN32

if (this && NativeWin)
	{
	// Special event checking
	#ifdef _WIN32
	while(LocalWinSys()->CheckNoWait(&BusyEvent))
		{
		GlobalApp->ProcessOSEvent(&BusyEvent);
		} // while
	#endif // _WIN32

	} // if

return (Run);

} // RenderControlGUI::CheckAbort

/*===========================================================================*/

void RenderControlGUI::StashFrame(unsigned long FrameTime)
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

} // RenderControlGUI::StashFrame

/*===========================================================================*/

void RenderControlGUI::ECRepaintFuelGauge(unsigned long CurSteps, unsigned long MaxSteps, int GaugeID)
{
NativeControl Me;
long Steps;
int Percent;

Steps = CurSteps;

Percent = 0;
Me = GetDlgItem(NativeWin, GaugeID);
if ((Steps > 0) && (MaxSteps > 0))
	{
	Percent = (long)(100 * ((float)Steps / (float)MaxSteps));
	if ((Percent == 0) && ((100 * ((float)Steps / (float)MaxSteps)) > 0))
		{
		Percent = 1;
		} // if
	} // if
#ifdef _WIN32
SendMessage(Me, WM_WCSW_CB_NEWVAL, 0, (LPARAM)Percent);
#endif // _WIN32

} // RenderControlGUI::RepaintFuelGauge

/*===========================================================================*/

void RenderControlGUI::UpdatePreviewState(void)
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

} // RenderControlGUI::UpdatePreviewState

/*===========================================================================*/

void RenderControlGUI::SetPreviewCheck(int State)
{

WidgetSetCheck(IDC_DISPLAYPREV, State, NativeWin);

} // RenderControlGUI::SetPreviewCheck

/*===========================================================================*/

char RenderControlGUI::GetRegenFDMs(void)
{
char Regen = 0;

Regen = EffectsHost->TerrainParamBase.RegenFDMsEachRender && EffectsHost->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_DEPTHMAPS;

return (Regen);

} // RenderControlGUI::GetRegenFDMs

/*===========================================================================*/

void RenderControlGUI::SetRegenFDMs(char Regen)
{
TerrainParamEffect *DefTerPar;
NotifyTag Changes[2];

Changes[1] = 0;

if (DefTerPar = (TerrainParamEffect *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, DBHost))
	{
	EffectsHost->TerrainParamBase.RegenFDMsEachRender = Regen;
	Changes[0] = MAKE_ID(DefTerPar->GetNotifyClass(), DefTerPar->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, DefTerPar->GetRAHostRoot());
	} // if

} // RenderControlGUI::SetRegenFDMs
