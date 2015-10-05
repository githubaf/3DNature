// ScriptAction.cpp
// Code to actually "catch" script events and implement most
// common commands.
// Created from Scratch on May 29 1997 by Chris 'Xenon' Hanson
// Copyright 1997

#include "stdafx.h"
#include "Log.h"
#include "Script.h"
#include "ScriptAction.h"
#include "VocabTable.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "DataOpsDefs.h"
#include "DXF.h"
#include "AppMem.h"
#include "Project.h"
//#include "ProjectIO.h"
#include "Toolbar.h"
#include "requester.h"
#include "Render.h"
#include "DBFilterEvent.h"
#include "Database.h"
#include "Joe.h"

extern int AutoStartRender;
extern int suppress_rendernotify;
int NetScriptRender;


ScriptActor::ScriptActor(ScriptParser *ScriptMod)
{
ParseMod = ScriptMod;
DBFilter = CurFilter = NULL;

if(ParseMod)
	{
	ParseMod->AddHandler((WCSModule *)this);
	} // if

} // ScriptActor::ScriptActor

/*===========================================================================*/

ScriptActor::~ScriptActor()
{

while (DBFilter)
	{
	CurFilter = DBFilter;
	DBFilter = DBFilter->Next;
	delete CurFilter;
	} // while

if(ParseMod)
	{
	ParseMod->RemoveHandler((WCSModule *)this);
	} // if

} // ScriptActor::~ScriptActor

/*===========================================================================*/

static int GoAhead, TOneShot;

long ScriptActor::HandleEvent(void)
{
long CommandResult = -1;
#ifndef WCS_BUILD_DEMO
struct CmdContext *LocalScope;
time_t TCheck;

// net render script commands aren't restricted to SE
if(Activity->Type == WCS_APP_EVENTTYPE_SCRIPT)
	{
	LocalScope = (struct CmdContext *)Activity->GenericData;
	LocalApp = AppScope;
	if(LocalScope->SourceScript->IsNetwork())
		{
		switch(LocalScope->EventCode)
			{
			case WCS_SCRIPT_EVENT_PROJECT:
				{
				if((LocalScope->LastToken == VE_LOAD) || // These are the only permitted commands from PROJECT group
				 (LocalScope->LastToken == VE_WCSCONTENT) ||
				 (LocalScope->LastToken == VE_WCSFRAMES) ||
				 (LocalScope->LastToken == VE_WCSPROJECTS))
					{
					CommandResult = HandleProject(LocalScope);
					} // if
				break;
				} // PROJECT
			case WCS_SCRIPT_EVENT_RENDER:
				{
				NetScriptRender = 1;
				CommandResult = HandleRender(LocalScope);
				NetScriptRender = 0;
				break;
				} // RENDER
			case WCS_SCRIPT_EVENT_QUIT:
				{
				CommandResult = HandleQuit(LocalScope);
				break;
				} // QUIT
			} // switch
		return(CommandResult); // prevent checking for command restriction if successful 
		} // if
	} // if

if(!GoAhead)
	{
	GoAhead = GlobalApp->Sentinal->CheckAuthSE();
	} // if

if(GoAhead)
	{
	if(Activity->Type == WCS_APP_EVENTTYPE_SCRIPT)
		{
		LocalScope = (struct CmdContext *)Activity->GenericData;
		LocalApp = AppScope;
		(void)time(&TCheck);
		if((TCheck % 5) == 0)
			{
			if(TOneShot == 0)
				{
				GoAhead = 0;
				TOneShot = 1;
				} // if
			} // if
		else
			{
			TOneShot = 0;
			} // else
		switch(LocalScope->EventCode)
			{
			case WCS_SCRIPT_EVENT_PROJECT:
				{
				CommandResult = HandleProject(LocalScope);
				break;
				} // PROJECT
			case WCS_SCRIPT_EVENT_SCRIPT:
				{
				CommandResult = HandleScript(LocalScope);
				break;
				} // SCRIPT
			case WCS_SCRIPT_EVENT_RENDER:
				{
				CommandResult = HandleRender(LocalScope);
				break;
				} // RENDER
			case WCS_SCRIPT_EVENT_WINDOW:
				{
				CommandResult = HandleWindow(LocalScope);
				break;
				} // WINDOW
			case WCS_SCRIPT_EVENT_MAPVIEW:
				{
				CommandResult = HandleMapView(LocalScope);
				break;
				} // MAPVIEW
			case WCS_SCRIPT_EVENT_CAMVIEW:
				{
				CommandResult = HandleCamView(LocalScope);
				break;
				} // CAMVIEW
			case WCS_SCRIPT_EVENT_RENDSET:
				{
				CommandResult = HandleRendSet(LocalScope);
				break;
				} // RENDSET
			case WCS_SCRIPT_EVENT_PARAM:
				{
				CommandResult = HandleParam(LocalScope);
				break;
				} // RENDSET
			case WCS_SCRIPT_EVENT_EFFECT:
				{
				CommandResult = HandleEffect(LocalScope);
				break;
				} // RENDSET
			case WCS_SCRIPT_EVENT_DATABASE:
				{
				CommandResult = HandleDataBase(LocalScope);
				break;
				} // DATABASE
			case WCS_SCRIPT_EVENT_IMAGELIB:
				{
				CommandResult = HandleImageLib(LocalScope);
				break;
				} // IMAGELIB
			case WCS_SCRIPT_EVENT_GISIMPORT:
				{
				CommandResult = HandleGISImport(LocalScope);
				break;
				} // GISIMPORT
			case WCS_SCRIPT_EVENT_QUIT:
				{
				CommandResult = HandleQuit(LocalScope);
				break;
				} // QUIT
			} // switch
		} // if
	} // if
else
	{ // report non-authorized, and permit authorization
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Scripting not enabled.");
	GlobalApp->Sentinal->DoAuth('S');
	} // else
#endif // !WCS_BUILD_DEMO

return(CommandResult);
} // ScriptActor::HandleEvent

/*===========================================================================*/

long ScriptActor::HandleProject(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
long NumArgsParsed; 
unsigned long ProjectResult;
char PathPart[256], FilePart[32], ButcherableArg[1024];
int Count;
char *Result;
NewProject *NewProj;

switch(ArgScope->LastToken)
	{
	case VE_NEW:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if (NewProj = new NewProject(LocalApp->MainProj, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages))
				{
				int GotName = 0;
				ArgRipper GetRipped("NAME CLONE CLONEINMEMORY SAVETODIR");
				strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

				if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(0, Count)) // NAME
							{
							BreakFileName(Result, PathPart, 256, FilePart, 32);
							NewProj->ProjName.SetPath(PathPart);
							NewProj->ProjName.SetName(FilePart);
							GotName = 1;
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // CLONE
							{
							BreakFileName(Result, PathPart, 256, FilePart, 32);
							NewProj->CloneName.SetPath(PathPart);
							NewProj->CloneName.SetName(FilePart);
							NewProj->Clone = 1;
							NewProj->CloneType = 0;
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(2, Count)) // CLONEINMEMORY
							{
							if (! (NewProj->CloneType = ! TestBoolean(Result)))
								NewProj->Clone = 1;
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(3, Count)) // SAVETODIR
							{
							NewProj->PlaceInSubDir = TestBoolean(Result);
							break;
							} // if
						else Count++;
						} // for
					if (GotName && NewProj->Create())
						HandleResult = 1;
					} // if
				} // if
			} // else
		break;
		} // NEW
	case VE_LOAD:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if(ProjectResult = LocalApp->MainProj->Load(NULL, ArgScope->ArgStr, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages, NULL, 0xffffffff))
				HandleResult = ProjectResult;
			} // if
		else
			{
			if(ProjectResult = LocalApp->MainProj->Load(NULL, NULL, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages, NULL, 0xffffffff))
				HandleResult = ProjectResult;
			} // else
		break;
		} // LOAD
	case VE_SAVEAS:
		{
		if(strlen(ArgScope->ArgStr))
			{
			strcpy(ScriptActorArgBuf, ArgScope->ArgStr);
			BreakFileName(ScriptActorArgBuf, PathPart, 256, FilePart, 32);
			strcpy(LocalApp->MainProj->projectpath, PathPart);
			strcpy(LocalApp->MainProj->projectname, FilePart);
			//UpdateProjTitle(LocalApp->WinSys->RootWin, LocalApp->MainProj->projectname);
			LocalApp->WinSys->UpdateRootTitle(LocalApp->MainProj->projectname, LocalApp->MainProj->Prefs.CurrentUserName);
			if(ProjectResult = LocalApp->MainProj->Save(NULL, ScriptActorArgBuf, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages, NULL, 0xffffffff))
				HandleResult = ProjectResult;
			} // if
		else
			{
			if(ProjectResult = LocalApp->MainProj->Save(NULL, NULL, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages, NULL, 0xffffffff))
				HandleResult = ProjectResult;
			} // else
		break;
		} //
	case VE_SAVE:
		{
		if(strlen(ArgScope->ArgStr))
			{
			strcpy(ScriptActorArgBuf, ArgScope->ArgStr);
			BreakFileName(ScriptActorArgBuf, PathPart, 256, FilePart, 32);
			strcpy(LocalApp->MainProj->projectpath, PathPart);
			strcpy(LocalApp->MainProj->projectname, FilePart);
			//UpdateProjTitle(LocalApp->WinSys->RootWin, LocalApp->MainProj->projectname);
			LocalApp->WinSys->UpdateRootTitle(LocalApp->MainProj->projectname, LocalApp->MainProj->Prefs.CurrentUserName);
			} // if
		else
			{
			strmfp(ScriptActorArgBuf, LocalApp->MainProj->projectpath, LocalApp->MainProj->projectname);
			} // else
		if(ProjectResult = LocalApp->MainProj->Save(NULL, ScriptActorArgBuf, LocalApp->AppDB, LocalApp->AppEffects, LocalApp->AppImages, NULL, 0xffffffff))
			HandleResult = ProjectResult;
		break;
		} //
	case VE_WCSCONTENT:
		{
		if(strlen(ArgScope->ArgStr))
			{
			LocalApp->MainProj->SetContentPath(ArgScope->ArgStr);
			} // if
		break;
		} //
	case VE_WCSFRAMES:
		{
		if(strlen(ArgScope->ArgStr))
			{
			LocalApp->MainProj->SetFramesPath(ArgScope->ArgStr);
			} // if
		break;
		} //
	case VE_WCSPROJECTS:
		{
		if(strlen(ArgScope->ArgStr))
			{
			LocalApp->MainProj->SetProjectPath(ArgScope->ArgStr);
			} // if
		break;
		} //
	} // LastToken
#endif // !DEMO
return(HandleResult);

} // HandleProject

/*===========================================================================*/

long ScriptActor::HandleScript(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
unsigned long ScriptResult;
int NewNum;
char DebugLevel;

switch(ArgScope->LastToken)
	{
	case VE_RUN:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if(ScriptResult = ArgScope->SourceScript->GetHost()->StartScript(ArgScope->ArgStr, 1))
				HandleResult = ScriptResult;
			} // if
		break;
		} // LOAD
	case VE_END:
		{
		ArgScope->SourceScript->SetTerminate();
		HandleResult = 0;
		break;
		} // END
	case VE_COMMENT:
		{ // here's a useless command
		HandleResult = 0;
		break;
		} // COMMENT
	case VE_ECHO:
		{
		ArgScope->SourceScript->LogScriptError(WCS_LOG_SEVERITY_MSG, ArgScope->ArgStr);
		HandleResult = 0;
		break;
		} // ECHO
	case VE_DEBUG:
	case VE_LEVEL:
		{
		if(strlen(ArgScope->ArgStr))
			{
			NewNum = atoi(ArgScope->ArgStr);
			if(NewNum > SCHAR_MAX) NewNum = SCHAR_MAX;
			if(NewNum < SCHAR_MIN) NewNum = SCHAR_MIN;
			DebugLevel = (char)NewNum;
			ArgScope->SourceScript->SetDebugLevel(DebugLevel);
			HandleResult = 0;
			} // if
		break;
		} // DEBUG LEVEL
	} // LastToken
#endif // !DEMO
return(HandleResult);
} // HandleScript

long ScriptActor::HandleRender(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
char ButcherableArg[1024];
long NumArgsParsed;
int Count;
char *Result;
RenderJob *FoundJob = NULL;
unsigned long FrameToRender = (unsigned long)-1;
int OldSuppress;

switch(ArgScope->LastToken)
	{
	case VE_RENDER:
		{
		ArgRipper GetRipped("JOB FRAME PRI MINIMIZE");
		strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

		if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
			{
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = GetRipped.GetArg(0, Count)) // JOB
					{
					//ArgScope->SourceScript->ReportResult("JOB", Result);
					//Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
					//	HandleResult = 1;
					FoundJob = (RenderJob *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_RENDERJOB, Result);
					break;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = GetRipped.GetArg(1, Count)) // FRAME
					{
					FrameToRender = atoi(Result);
					//ArgScope->SourceScript->ReportResult("FRAME", Result);
					//if (LocalApp->AppEffects->AddEffect(EffectType, Result, Proto))
					//	HandleResult = 1;
					break;
					} // if
				else Count++;
				} // for
			if(FoundJob && FoundJob->Cam && FoundJob->Options && FrameToRender != 0xffffffff) // 0xffffffff used to be -1 but triggered type sign warnings
				{ // we are go for Render!
				GeneralEffect *JobList;

				ArgScope->SourceScript->ReportResult("RENDERJOB", FoundJob->GetName());
				ArgScope->SourceScript->ReportResult("RENDERFRAME", FrameToRender);

				FoundJob->SetEnabled(1);
				FoundJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetCurValue(0, (FrameToRender / FoundJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].GetCurValue()));
				FoundJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetCurValue(0, (FrameToRender / FoundJob->Options->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].GetCurValue()));

				for(JobList = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); JobList; JobList = JobList->Next)
					{
					if(JobList != FoundJob)
						{
						JobList->SetEnabled(0);
						} // if
					} // for

				AutoStartRender = 1;
				OldSuppress = suppress_rendernotify;
				suppress_rendernotify = 1;
				GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
				 WCS_TOOLBAR_ITEM_RCG, 0);
				suppress_rendernotify = OldSuppress;

				HandleResult = 1;
				} // if
			else
				{
				// report problem details
				if(FoundJob) ArgScope->SourceScript->ReportResult("RENDERFAIL", "Invalid Frame");
				else if(FrameToRender != 0xffffffff) ArgScope->SourceScript->ReportResult("RENDERFAIL", "Invalid Job");  // 0xffffffff used to be -1 but triggered type sign warnings
				else ArgScope->SourceScript->ReportResult("RENDERFAIL", "Invalid Job, Frame, Camera or Options");
				} // else
			} // if
		else
			{ // act like we used to before we added JOB/FRAME args, IE: start rendering all currently enabled jobs
			AutoStartRender = 1;
			OldSuppress = suppress_rendernotify;
			suppress_rendernotify = 1;
			GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			 WCS_TOOLBAR_ITEM_RCG, 0);
			suppress_rendernotify = OldSuppress;
			HandleResult = 1;
			} // else
		break;
		} // OPEN
	} // LastToken
#endif // !DEMO
return(HandleResult);
} // HandleRender


/*===========================================================================*/

long ScriptActor::HandleWindow(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
unsigned long WinID;
Fenetre *Candidate = NULL;
const char *NextArg;
short X, Y, W, H, A, B;
int GotArgs;

switch(ArgScope->LastToken)
	{
	case VE_OPEN:
		{
		LocalApp->MCP->OpenAWindow(MakeIDFromString(ArgScope->ArgStr));
		if(Candidate = FindWinFromArg(ArgScope->ArgStr))
			{ // did it open?
			HandleResult = 0;
			} // if
		break;
		} // OPEN
	case VE_CLOSE:
		{
		HandleResult = 0;
		LocalApp->MCP->RequestWindowClose(MakeIDFromString(ArgScope->ArgStr));
		if(Candidate = FindWinFromArg(ArgScope->ArgStr))
			{ // is it still open?
			HandleResult = -1;
			} // if
		break;
		} // CLOSE
	case VE_ORIGIN:
	case VE_POSITION:
	case VE_SIZE:
		{
		// Get WindowID
		if(NextArg = SkipPastNextSpace(ArgScope->ArgStr))
			{
			if(WinID = MakeIDFromString(ArgScope->ArgStr))
				{
				// Get Window position args
				if((GotArgs = GetNumericArgs(NextArg)) > 1) // 2 or more args
					{
					X = (short)NumericArgs[0];
					Y = (short)NumericArgs[1];
					if(GotArgs == 4)
						{
						W = (short)NumericArgs[2];
						H = (short)NumericArgs[3];
						} // if
					if((ArgScope->LastToken == VE_SIZE) && (GotArgs == 2))
						{
						W = X; H = Y;
						LocalApp->MainProj->InquireWindowCoords(WinID, X, Y, A, B);
						LocalApp->MainProj->SetWindowCoords(WinID, X, Y, W, H);
						HandleResult = 0;
						} // if
					else if((ArgScope->LastToken == VE_ORIGIN) && (GotArgs == 2))
						{
						LocalApp->MainProj->SetWindowCoords(WinID, X, Y, 0, 0);
						HandleResult = 0;
						} // if
					else if((ArgScope->LastToken == VE_POSITION) && (GotArgs == 4))
						{
						LocalApp->MainProj->SetWindowCoords(WinID, X, Y, W, H);
						HandleResult = 0;
						} // if
					} // if
				} // if
			} // if
		break;
		} // POSITION
	case VE_TITLE:
		{
		if(NextArg = SkipPastNextSpace(ArgScope->ArgStr))
			{
			HandleResult = 0;
			if(Candidate = FindWinFromArg(ArgScope->ArgStr))
				{
				Candidate->SetTitle(NextArg);
				} // if
			} // if
		break;
		} // SIZE
	case VE_BOTTOM:
	case VE_TOP:
		{
		if(strlen(ArgScope->ArgStr))
			{
			HandleResult = 0;
			if(Candidate = FindWinFromArg(ArgScope->ArgStr))
				{
				if(ArgScope->LastToken == VE_TOP) Candidate->JumpTop();
				if(ArgScope->LastToken == VE_BOTTOM) Candidate->JumpBottom();
				} // if
			} // if
		break;
		} // TOP / BOTTOM
	case VE_SAVE:
	case VE_CONTENTS:
		{
		if(NextArg = SkipPastNextSpace(ArgScope->ArgStr))
			{
			if(Candidate = FindWinFromArg(ArgScope->ArgStr))
				{
				if(Candidate->SaveContentsToFile(NextArg))
					{
					HandleResult = 0;
					} // if
				} // if
			} // if
		break;
		} // SAVE CONTENTS
	} // LastToken

#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleWindow

/*===========================================================================*/

long ScriptActor::HandleMapView(struct CmdContext *ArgScope)
{
long HandleResult = -1;
return(HandleResult);
} // ScriptActor::HandleMapView

/*===========================================================================*/

long ScriptActor::HandleCamView(struct CmdContext *ArgScope)
{
long HandleResult = -1;
return(HandleResult);
} // ScriptActor::HandleCamView

/*===========================================================================*/

long ScriptActor::HandleRendSet(struct CmdContext *ArgScope)
{
long HandleResult = -1;
return(HandleResult);
} // ScriptActor::HandleRendSet

/*===========================================================================*/

long ScriptActor::HandleParam(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
int GotArgs;

// 1: Set, 2: Keyframe
char ParamAction = 0;

// 1:Camera, 2:Target, 3:SunLight, 4:SunImage, 5:MoonImage
// 6: HazeStart, 7:HazeEnd, 8:CloudHazeStart, 9:CloudHazeEnd
char ParamType = 0;

switch(ArgScope->LastToken)
	{
	case VE_CAMERA: ParamType = 1; break;
	case VE_TARGET: ParamType = 2; break;
	case VE_LIGHT: ParamType = 3; break;
	case VE_IMAGE:
		{
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_SUN) != -1)
			{
			ParamType = 4;
			break;
			} // if
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_MOON) != -1)
			{
			ParamType = 5;
			break;
			} // if
		break;
		} // IMAGE
	case VE_START:
		{
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_CLOUD) != -1)
			{
			ParamType = 8;
			break;
			} // if
		else
			{
			ParamType = 6;
			break;
			} // else
		} // START
	case VE_END:
		{
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_CLOUD) != -1)
			{
			ParamType = 9;
			break;
			} // if
		else
			{
			ParamType = 7;
			break;
			} // else
		} // END
	} // switch

if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_SET) != -1)
	{
	ParamAction = 1;
	} // if
if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_KEYFRAME) != -1)
	{
	ParamAction = 2;
	} // if
	
if(ParamType && ParamAction)
	{
	if((GotArgs = GetNumericArgs(ArgScope->ArgStr)) > 1) // 2 or more args
		{
		if(GotArgs > 2)
			{
			#ifdef WCS_BUILD_V5
			LocalApp->ActivePar->SetParam(1, WCS_PARAMCLASS_MOTION, WCS_MOTION_VALUE, WCS_MOTION_CAMALT, 0, NumericArgs[2],
						WCS_PARAMCLASS_MOTION, WCS_MOTION_VALUE, WCS_MOTION_CAMLAT, 0, NumericArgs[0],
						WCS_PARAMCLASS_MOTION, WCS_MOTION_VALUE, WCS_MOTION_CAMLON, 0, NumericArgs[1], NULL);
			#endif // WCS_BUILD_V5
			HandleResult = 0;
			} // if
		else
			{
			#ifdef WCS_BUILD_V5
			LocalApp->ActivePar->SetParam(1, 
						WCS_PARAMCLASS_MOTION, WCS_MOTION_VALUE, WCS_MOTION_CAMLAT, 0, NumericArgs[0],
						WCS_PARAMCLASS_MOTION, WCS_MOTION_VALUE, WCS_MOTION_CAMLON, 0, NumericArgs[1], NULL);
			#endif // WCS_BUILD_V5
			HandleResult = 0;
			} // else
		} // if
	} // if
#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleParam

/*===========================================================================*/

long ScriptActor::HandleEffect(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
long NumArgsParsed, EffectType, LoadQuiet = 0; 
int Count;
char *Result;
GeneralEffect *Proto = NULL, *AnotherEffect;
Raster *Image = NULL;
FoliageGroup *CurFolGrp = NULL;
Foliage *CurFol = NULL;
Ecotype *Ecotp = NULL;
char PathPart[256], FilePart[64], ButcherableArg[1024];
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
time_t TCheck;

(void)time(&TCheck);
if((TCheck % 5) == 0)
	{
	if(TOneShot == 0)
		{
		GoAhead = 0;
		TOneShot = 1;
		} // if
	} // if
else
	{
	TOneShot = 0;
	} // else


switch(ArgScope->LastToken)
	{
	case VE_CREATE:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if ((EffectType = IdentifyEffectType(ArgScope->WordToken[1])) > 0)
				{
				ArgRipper GetRipped("NAME CLONE");
				strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

				if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // CLONE
							{
							Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(0, Count)) // NAME
							{
							if (LocalApp->AppEffects->AddEffect(EffectType, Result, Proto))
								HandleResult = 1;
							break;
							} // if
						else Count++;
						} // for
					} // if
				} // if
			} // if
		break;
		} // VE_CREATE
	case VE_RENAME:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if ((EffectType = IdentifyEffectType(ArgScope->WordToken[1])) > 0)
				{
				ArgRipper GetRipped("NAME NEWNAME");
				strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

				if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(0, Count)) // NAME
							{
							Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // NEWNAME
							{
							if (Proto)
								{
								Prop.PropMask = WCS_RAHOST_MASKBIT_NAME;
								Prop.Name = Result;
								if (Proto->SetRAHostProperties(&Prop))
									HandleResult = 1;
								} // if
							break;
							} // if
						else Count++;
						} // for
					} // if
				} // if
			} // if
		break;
		} // VE_RENAME
	case VE_LOAD:
	case VE_QUIET:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if ((EffectType = IdentifyEffectType(ArgScope->WordToken[1])) > 0)
				{
				ArgRipper GetRipped("NAME FILE");
				strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

				if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_QUIET) != -1)
					LoadQuiet = 1;
				if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(0, Count)) // NAME
							{
							Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
							break;
							} // if
						else Count++;
						} // for
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // FILE
							{
							if (Proto)
								{
								Prop.Path = Result;
								Prop.Queries = ! LoadQuiet;
								Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
								if (Proto->LoadFilePrep(&Prop) > 0)
									{
									Changes[0] = MAKE_ID(Proto->GetNotifyClass(), Proto->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
									Changes[1] = NULL;
									LocalApp->AppEx->GenerateNotify(Changes, Proto->GetRAHostRoot());
									HandleResult = 1;
									} // if
								} // if
							break;
							} // if
						else Count++;
						} // for
					} // if
				} // if
			} // if
		break;
		} // VE_LOAD
	case VE_SET:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if ((EffectType = IdentifyEffectType(ArgScope->WordToken[1])) > 0)
				{
				switch (EffectType)
					{
					case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
						{
						ArgRipper GetRipped("NAME MATCHRED MATCHGREEN MATCHBLUE CMAPMATCH ELEV ECOTYPE MAXHEIGHT MINHEIGHT DENSITY GROUP GROUPHEIGHT GROUPDENSITY FOLIAGE FOLIAGEHEIGHT FOLIAGEDENSITY");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // MATCHRED
										{
										((EcosystemEffect *)Proto)->MatchColor[0] = atoi(Result);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(2, Count)) // MATCHGREEN
										{
										((EcosystemEffect *)Proto)->MatchColor[1] = atoi(Result);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(3, Count)) // MATCHBLUE
										{
										((EcosystemEffect *)Proto)->MatchColor[2] = atoi(Result);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(4, Count)) // CMAPMATCH
										{
										((EcosystemEffect *)Proto)->CmapMatch = TestBoolean(Result);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(5, Count)) // ELEV
										{
										((EcosystemEffect *)Proto)->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(6, Count)) // ECOTYPE
										{
										GradientCritter *ActiveNode;
										if (ActiveNode = ((EcosystemEffect *)Proto)->EcoMat.GetNextNode(NULL))
											{
											MaterialEffect *Mat;
											if (Mat = (MaterialEffect *)ActiveNode->GetThing())
												{
												if (! stricmp(Result, "Overstory"))
													Ecotp = Mat->EcoFol[0];
												else
													Ecotp = Mat->EcoFol[1];
												} // if material
											} // if node
										break;
										} // if
									else Count++;
									} // for
								if (Ecotp)
									{
									for (Count = 0; Count < NumArgsParsed;)
										{
										if (Result = GetRipped.GetArg(7, Count)) // MAXHEIGHT
											{
											Ecotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetCurValue(atof(Result));
											HandleResult = 1;
											break;
											} // if
										else Count++;
										} // for
									for (Count = 0; Count < NumArgsParsed;)
										{
										if (Result = GetRipped.GetArg(8, Count)) // MINHEIGHT
											{
											Ecotp->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetCurValue(atof(Result));
											HandleResult = 1;
											break;
											} // if
										else Count++;
										} // for
									for (Count = 0; Count < NumArgsParsed;)
										{
										if (Result = GetRipped.GetArg(9, Count)) // DENSITY
											{
											Ecotp->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].SetCurValue(atof(Result));
											HandleResult = 1;
											break;
											} // if
										else Count++;
										} // for
									for (Count = 0; Count < NumArgsParsed;)
										{
										if (Result = GetRipped.GetArg(10, Count)) // GROUP
											{
											if (! (CurFolGrp = Ecotp->FindFoliageGroup(Result)))
												CurFolGrp = Ecotp->AddFoliageGroup(NULL, Result);
											HandleResult = 1;
											break;
											} // if
										else Count++;
										} // for
									if (CurFolGrp)
										{
										for (Count = 0; Count < NumArgsParsed;)
											{
											if (Result = GetRipped.GetArg(11, Count)) // GROUPHEIGHT
												{
												#if !defined (WCS_BUILD_W6) && !defined (WCS_BUILD_V2)
												CurFolGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetCurValue(atof(Result) / 100.0);
												#else
												CurFolGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetCurValue(atof(Result));
												#endif
												HandleResult = 1;
												break;
												} // if
											else Count++;
											} // for
										for (Count = 0; Count < NumArgsParsed;)
											{
											if (Result = GetRipped.GetArg(12, Count)) // GROUPDENSITY
												{
												#if !defined (WCS_BUILD_W6) && !defined (WCS_BUILD_V2)
												CurFolGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetCurValue(atof(Result) / 100.0);
												#else
												CurFolGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetCurValue(atof(Result));
												#endif
												HandleResult = 1;
												break;
												} // if
											else Count++;
											} // for
										for (Count = 0; Count < NumArgsParsed;)
											{
											if (Result = GetRipped.GetArg(13, Count)) // FOLIAGE
												{
												if (! (CurFol = CurFolGrp->FindFoliage(Result)))
													{
													if (! (Image = LocalApp->AppImages->FindByUserName(Result)))
														{
														BreakFileName(Result, PathPart, 256, FilePart, 64);
														Image = LocalApp->AppImages->FindByName(PathPart, FilePart);
														} // if not matched to user name
													if (Image)
														{
														if (CurFol = CurFolGrp->AddFoliage(NULL))
															{
															CurFol->SetRaster(Image);
															} // if
														} // if
													} // if
												HandleResult = 1;
												break;
												} // if
											else Count++;
											} // for
										if (CurFol)
											{
											for (Count = 0; Count < NumArgsParsed;)
												{
												if (Result = GetRipped.GetArg(14, Count)) // FOLIAGEHEIGHT
													{
													CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetCurValue(atof(Result) / 100.0);
													HandleResult = 1;
													break;
													} // if
												else Count++;
												} // for
											for (Count = 0; Count < NumArgsParsed;)
												{
												if (Result = GetRipped.GetArg(15, Count)) // FOLIAGEDENSITY
													{
													CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetCurValue(atof(Result) / 100.0);
													HandleResult = 1;
													break;
													} // if
												else Count++;
												} // for
											} // if
										} // if
									} // if
								} // if
							} // if
						break;
						} // eco
					case WCS_EFFECTSSUBCLASS_CMAP:
						{
						int PosLon;
						ArgRipper GetRipped("NAME POSLON NORTH SOUTH WEST EAST MATCHECO IMAGE ECOSYSTEM");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								RasterAttribute *MyAttr;
								GeoRefShell *Shell = NULL;

								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(7, Count)) // IMAGE
										{
										if (! (Image = LocalApp->AppImages->FindByUserName(Result)))
											{
											BreakFileName(Result, PathPart, 256, FilePart, 64);
											Image = LocalApp->AppImages->FindByName(PathPart, FilePart);
											} // if not matched to user name
										if (Image)
											{
											((CmapEffect *)Proto)->SetRaster(Image);
											if (((CmapEffect *)Proto)->Img)
												{
												if (! ((CmapEffect *)Proto)->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
													((CmapEffect *)Proto)->Img->GetRaster()->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF, NULL, NULL);
												} // if
											HandleResult = 1;
											} // if
										break;
										} // if
									else Count++;
									} // for
								if (((CmapEffect *)Proto)->Img && ((CmapEffect *)Proto)->Img->GetRaster())
									{
									if ((MyAttr = ((CmapEffect *)Proto)->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
										{
										Shell = (GeoRefShell *)MyAttr->GetShell();
										} // if
									} // if
								PosLon = 1;	// for back-compatibility defaults to west positive
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // POSLON
										{
										PosLon = TestBoolean(Result) ? 1: -1;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(2, Count)) // NORTH
										{
										if (Shell)
											{
											Shell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetCurValue(atof(Result));
											HandleResult = 1;
											} // if
										else
											HandleResult = 0;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(3, Count)) // SOUTH
										{
										if (Shell)
											{
											Shell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetCurValue(atof(Result));
											HandleResult = 1;
											} // if
										else
											HandleResult = 0;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(4, Count)) // WEST
										{
										if (Shell)
											{
											Shell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetCurValue(PosLon * atof(Result));
											HandleResult = 1;
											} // if
										else
											HandleResult = 0;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(5, Count)) // EAST
										{
										if (Shell)
											{
											Shell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetCurValue(PosLon * atof(Result));
											HandleResult = 1;
											} // if
										else
											HandleResult = 0;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(6, Count)) // MATCHECO
										{
										((CmapEffect *)Proto)->EvalByPixel = ! TestBoolean(Result);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(8, Count)) // ECOSYSTEM
										{
										if (AnotherEffect = LocalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, Result))
											{
											((CmapEffect *)Proto)->AddEcosystem(AnotherEffect);
											HandleResult = 1;
											} // if matched to ecosystem name
										break;
										} // if
									else Count++;
									} // for
								} // if Proto
							} // if
						break;
						} // cmap
					case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
						{
						ArgRipper GetRipped("NAME ECOSYSTEM");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // ECOSYSTEM
										{
										if (AnotherEffect = LocalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, Result))
											{
											((EnvironmentEffect *)Proto)->AddEcosystem(AnotherEffect);
											HandleResult = 1;
											} // if matched to ecosystem name
										break;
										} // if
									else Count++;
									} // for
								} // if Proto
							} // if
						break;
						} // environment
					case WCS_EFFECTSSUBCLASS_LAKE:
						{
						ArgRipper GetRipped("NAME ELEV");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // ELEV
										{
										((LakeEffect *)Proto)->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								} // if Proto
							} // if
						break;
						} // cmap
					case WCS_EFFECTSSUBCLASS_CAMERA:
						{
						ArgRipper GetRipped("NAME FLOATING INTERACTIVEFOLLOW TYPE PANORAMIC ORTHOGRAPHIC STEREO HFOV LATITUDE LONGITUDE ELEVATION HEADING PITCH BANK TARGETLATITUDE TARGETLONGITUDE TARGETELEV");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // FLOATING
										{
										((Camera *)Proto)->SetFloating(atoi(Result) ? 1: 0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(2, Count)) // INTERACTIVEFOLLOW
										{
										((Camera *)Proto)->InterElevFollow = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(3, Count)) // TYPE
										{
										if (! stricmp(Result, "TARGETED"))
											((Camera *)Proto)->CameraType = WCS_EFFECTS_CAMERATYPE_TARGETED;
										else if (! stricmp(Result, "NONTARGETED"))
											((Camera *)Proto)->CameraType = WCS_EFFECTS_CAMERATYPE_UNTARGETED;
										else if (! stricmp(Result, "OVERHEAD"))
											{
											((Camera *)Proto)->PanoCam = 0;
											((Camera *)Proto)->CameraType = WCS_EFFECTS_CAMERATYPE_OVERHEAD;
											} // else if
										else if (! stricmp(Result, "PLANIMETRIC"))
											{
											((Camera *)Proto)->CameraType = WCS_EFFECTS_CAMERATYPE_PLANIMETRIC;
											((Camera *)Proto)->Orthographic = 0;
											((Camera *)Proto)->LensDistortion = 0;
											((Camera *)Proto)->StereoCam = 0;
											((Camera *)Proto)->PanoCam = 0;
											} // else if
										else if (! stricmp(Result, "ALIGNTOPATH"))
											((Camera *)Proto)->CameraType = WCS_EFFECTS_CAMERATYPE_ALIGNED;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(4, Count)) // PANORAMIC
										{
										((Camera *)Proto)->PanoCam = atoi(Result) ? 1: 0;
										if (((Camera *)Proto)->PanoCam)
											{
											((Camera *)Proto)->Orthographic = 0;
											((Camera *)Proto)->StereoCam = 0;
											} // if
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(5, Count)) // ORTHOGRAPHIC
										{
										((Camera *)Proto)->Orthographic = atoi(Result) ? 1: 0;
										if (((Camera *)Proto)->Orthographic)
											((Camera *)Proto)->LensDistortion = 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(6, Count)) // STEREO
										{
										((Camera *)Proto)->StereoCam = atoi(Result) ? 1: 0;
										if (((Camera *)Proto)->StereoCam)
											{
											((Camera *)Proto)->Orthographic = 0;
											((Camera *)Proto)->PanoCam = 0;
											} // if
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(7, Count)) // HFOV
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(8, Count)) // LATITUDE
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(9, Count)) // LONGITUDE
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(10, Count)) // ELEVATION
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(11, Count)) // HEADING
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(12, Count)) // PITCH
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(13, Count)) // BANK
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(14, Count)) // TARGETLATITUDE
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(15, Count)) // TARGETLONGITUDE
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(16, Count)) // TARGETELEV
										{
										((Camera *)Proto)->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								} // if Proto
							} // if
						break;
						} // camera
					case WCS_EFFECTSSUBCLASS_LIGHT:
						{
						ArgRipper GetRipped("NAME FLOATING DISTANT TYPE FALLOFFEXP CONEANGLE SOFTEDGEANGLE LATITUDE LONGITUDE ELEVATION HEADING PITCH CASTSHADOWS AASHADOWS SOFTSHADOWS ILLUMINATEVOLUMETRICATMOSPHERES LIGHTRADIUS COLORRED COLORGREEN COLORBLUE COLORINTENSITY");
						strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

						if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
							{
							for (Count = 0; Count < NumArgsParsed;)
								{
								if (Result = GetRipped.GetArg(0, Count)) // NAME
									{
									Proto = LocalApp->AppEffects->FindByName(EffectType, Result);
									break;
									} // if
								else Count++;
								} // for
							if (Proto)
								{
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(1, Count)) // FLOATING
										{
										((Light *)Proto)->SetFloating(atoi(Result) ? 1: 0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(2, Count)) // DISTANT
										{
										((Light *)Proto)->Distant = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(3, Count)) // TYPE
										{
										if (! stricmp(Result, "PARALLEL"))
											((Light *)Proto)->LightType = WCS_EFFECTS_LIGHTTYPE_PARALLEL;
										else if (! stricmp(Result, "OMNI"))
											((Light *)Proto)->LightType = WCS_EFFECTS_LIGHTTYPE_OMNI;
										else if (! stricmp(Result, "SPOT"))
											((Light *)Proto)->LightType = WCS_EFFECTS_LIGHTTYPE_SPOT;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(4, Count)) // FALLOFFEXP
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(5, Count)) // CONEANGLE
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(6, Count)) // SOFTEDGEANGLE
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(7, Count)) // LATITUDE
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(8, Count)) // LONGITUDE
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(9, Count)) // ELEVATION
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(10, Count)) // HEADING
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(11, Count)) // PITCH
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(12, Count)) // CASTSHADOWS
										{
										((Light *)Proto)->CastShadows = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(13, Count)) // AASHADOWS
										{
										((Light *)Proto)->AAShadows = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(14, Count)) // SOFTSHADOWS
										{
										((Light *)Proto)->SoftShadows = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(15, Count)) // ILLUMINATEVOLUMETRICATMOSPHERES
										{
										((Light *)Proto)->IllumAtmosphere = atoi(Result) ? 2: 0;
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(16, Count)) // LIGHTRADIUS
										{
										((Light *)Proto)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].SetCurValue(atof(Result));
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(17, Count)) // COLORRED
										{
										((Light *)Proto)->Color.SetCurValue(0, atof(Result) / 255.0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(18, Count)) // COLORGREEN
										{
										((Light *)Proto)->Color.SetCurValue(1, atof(Result) / 255.0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(19, Count)) // COLORBLUE
										{
										((Light *)Proto)->Color.SetCurValue(2, atof(Result) / 255.0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								for (Count = 0; Count < NumArgsParsed;)
									{
									if (Result = GetRipped.GetArg(20, Count)) // COLORINTENSITY
										{
										((Light *)Proto)->Color.Intensity.SetCurValue(atof(Result) / 100.0);
										HandleResult = 1;
										break;
										} // if
									else Count++;
									} // for
								} // if Proto
							} // if
						break;
						} // light
					} // switch
				} // if
			} // if
		break;
		} // VE_SET
	} // switch
#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleEffect

/*===========================================================================*/

long ScriptActor::HandleDataBase(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
long NumArgsParsed; 
char ButcherableArg[1024], PathPart[256], FilePart[256];
int Count;
char *Result;
DBFilterEvent *NewFilter;

switch(ArgScope->LastToken)
	{
	case VE_ADD:
		{
		if(strlen(ArgScope->ArgStr))
			{
			ArgRipper GetRipped("FILE");
			strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

			if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
				{
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(0, Count)) // FILE
						{
						BreakFileName(Result, PathPart, 256, FilePart, 256);
						if (LocalApp->AppDB->AddDEMToDatabase(PathPart, FilePart, LocalApp->MainProj, LocalApp->AppEffects))
							HandleResult = 1;
						break;
						} // if
					else Count++;
					} // for
				} // if
			} // if
		break;
		} // ADD
	case VE_REMOVE:
		{
		if(strlen(ArgScope->ArgStr))
			{
			} // if
		break;
		} // REMOVE
	case VE_IMPORT:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_DXF) != -1)
				{
				struct ImportData *DXFData;
				FILE *ffile;
				char DXFFileName[512];

				if (DXFData = (struct ImportData *)AppMem_Alloc(sizeof (struct ImportData), APPMEM_CLEAR))
					{
					ArgRipper GetRipped("FILE POSLON UTM ZONE CONTROLPT");
					strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;
					DXFFileName[0] = 0;
					DXFData->OutputStyle = WCS_IMPORTDATA_OUTPUTSTYLE_VECTOR;
					DXFData->ReverseX = 1;
					DXFData->UTMZone = 10;
					if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
						{
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(0, Count)) // FILE
								{
								strcpy(DXFFileName, Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(1, Count)) // POSLON
								{
								DXFData->ReverseX = ! atoi(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(2, Count)) // UTM
								{
								DXFData->UTMUnits = atoi(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(3, Count)) // ZONE
								{
								DXFData->UTMZone = atoi(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(4, Count)) // CONTROLPT
								{
								DXFData->OutputStyle = atoi(Result) > 0 ? WCS_IMPORTDATA_OUTPUTSTYLE_CONTROL: WCS_IMPORTDATA_OUTPUTSTYLE_VECTOR;
								break;
								} // if
							else Count++;
							} // for
						} // if
					if (DXFFileName[0])
						{
						if (ffile = PROJ_fopen(DXFFileName, "r"))
							{
							if (ImportGISDXF(ffile, GlobalApp->AppDB->StaticRoot, NULL, DXFData, NULL, NULL))
								HandleResult = 1;
							fclose(ffile);
							} // if
						} // if
					AppMem_Free(DXFData, sizeof (struct ImportData));
					} // if DXFData
				} // if DXF
			} // if
		break;
		} // IMPORT
	case VE_FILTER:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_ADD) != -1)
				{
				if (NewFilter = AddFilter())
					{
					CurFilter = NewFilter;
					HandleResult = 1;
					} // if
				} // if
			else if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_REMOVE) != -1)
				{
				RemoveFilter();
				if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_ALL) != -1)
					{
					while (CurFilter)
						RemoveFilter();
					} // if
				HandleResult = 1;
				} // if
			else if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_CLEAR) != -1)
				{
				if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_ALL) != -1)
					{
					for (NewFilter = DBFilter; NewFilter; NewFilter = NewFilter->Next)
						{
						NewFilter->SetScriptDefaults();
						} // for
					} // if
				else if (CurFilter)
					CurFilter->SetScriptDefaults();
				if (CurFilter)
					HandleResult = 1;
				} // if
			else if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_SET) != -1)
				{
				if (CurFilter)
					{//					0		1	  2		3			4		5		  6		       7		8		9				10			11			12		13			14			15		16			17			18			19			20
					ArgRipper GetRipped("ADD ENABLED DEMS VECTORS CONTROLPTS ENABLEDOBJ DISABLEDOBJ LINEOBJ POINTOBJ LAYEREQUALS LAYERSIMILAR LAYERNUMERIC LAYERNOT NAMEEQUALS NAMESIMILAR NAMENUMERIC NAMENOT LABELEQUALS LABELSIMILAR LABELNUMERIC LABELNOT");
					strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

					if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
						{
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(0, Count)) // ADD
								{
								CurFilter->EventType = TestBoolean(Result) ? WCS_DBFILTER_EVENTTYPE_ADD: WCS_DBFILTER_EVENTTYPE_SUB;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(1, Count)) // ENABLED
								{
								CurFilter->Enabled = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(2, Count)) // DEMS
								{
								CurFilter->PassDEM = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(3, Count)) // VECTORS
								{
								CurFilter->PassVector = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(4, Count)) // CONTROLPTS
								{
								CurFilter->PassControlPt = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(5, Count)) // ENABLEDOBJ
								{
								CurFilter->PassEnabled = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(6, Count)) // DISABLEDOBJ
								{
								CurFilter->PassDisabled = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(7, Count)) // LINEOBJ
								{
								CurFilter->PassLine = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(8, Count)) // POINTOBJ
								{
								CurFilter->PassPoint = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(17, Count)) // LAYEREQUALS
								{
								if (Result[0])
									CurFilter->NewLayer(Result);
								CurFilter->LayerEquals = Result[0] ? 1: 0;
								CurFilter->LayerSimilar = CurFilter->LayerEquals ? 0: CurFilter->LayerSimilar;
								CurFilter->LayerNumeric = CurFilter->LayerEquals ? 0: CurFilter->LayerNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(18, Count)) // LAYERSIMILAR
								{
								if (Result[0])
									CurFilter->NewLayer(Result);
								CurFilter->LayerSimilar = Result[0] ? 1: 0;
								CurFilter->LayerEquals = CurFilter->LayerSimilar ? 0: CurFilter->LayerEquals;
								CurFilter->LayerNumeric = CurFilter->LayerSimilar ? 0: CurFilter->LayerNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(19, Count)) // LAYERNUMERIC
								{
								CurFilter->LayerNumeric = TestBoolean(Result);
								CurFilter->LayerEquals = CurFilter->LayerNumeric ? 0: CurFilter->LayerEquals;
								CurFilter->LayerSimilar = CurFilter->LayerNumeric ? 0: CurFilter->LayerSimilar;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(20, Count)) // LAYERNOT
								{
								CurFilter->LayerNot = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(13, Count)) // NAMEEQUALS
								{
								if (Result[0])
									CurFilter->NewName(Result);
								CurFilter->NameEquals = Result[0] ? 1: 0;
								CurFilter->NameSimilar = CurFilter->NameEquals ? 0: CurFilter->NameSimilar;
								CurFilter->NameNumeric = CurFilter->NameEquals ? 0: CurFilter->NameNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(14, Count)) // NAMESIMILAR
								{
								if (Result[0])
									CurFilter->NewName(Result);
								CurFilter->NameSimilar = Result[0] ? 1: 0;
								CurFilter->NameEquals = CurFilter->NameSimilar ? 0: CurFilter->NameEquals;
								CurFilter->NameNumeric = CurFilter->NameSimilar ? 0: CurFilter->NameNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(15, Count)) // NAMENUMERIC
								{
								CurFilter->NameNumeric = TestBoolean(Result);
								CurFilter->NameEquals = CurFilter->NameNumeric ? 0: CurFilter->NameEquals;
								CurFilter->NameSimilar = CurFilter->NameNumeric ? 0: CurFilter->NameSimilar;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(16, Count)) // NAMENOT
								{
								CurFilter->NameNot = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(17, Count)) // LABELEQUALS
								{
								if (Result[0])
									CurFilter->NewLabel(Result);
								CurFilter->LabelEquals = Result[0] ? 1: 0;
								CurFilter->LabelSimilar = CurFilter->LabelEquals ? 0: CurFilter->LabelSimilar;
								CurFilter->LabelNumeric = CurFilter->LabelEquals ? 0: CurFilter->LabelNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(18, Count)) // LABELSIMILAR
								{
								if (Result[0])
									CurFilter->NewLabel(Result);
								CurFilter->LabelSimilar = Result[0] ? 1: 0;
								CurFilter->LabelEquals = CurFilter->LabelSimilar ? 0: CurFilter->LabelEquals;
								CurFilter->LabelNumeric = CurFilter->LabelSimilar ? 0: CurFilter->LabelNumeric;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(19, Count)) // LABELNUMERIC
								{
								CurFilter->LabelNumeric = TestBoolean(Result);
								CurFilter->LabelEquals = CurFilter->LabelNumeric ? 0: CurFilter->LabelEquals;
								CurFilter->LabelSimilar = CurFilter->LabelNumeric ? 0: CurFilter->LabelSimilar;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(20, Count)) // LABELNOT
								{
								CurFilter->LabelNot = TestBoolean(Result);
								break;
								} // if
							else Count++;
							} // for
						HandleResult = 1;
						} // if
					} // if
				} // if
			} // if
		break;
		} // FILTER
	case VE_PROPERTIES:
		{
		if(strlen(ArgScope->ArgStr))
			{
			if (ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_SET) != -1)
				{
				if (DBFilter)
					{//						0		1			2			3		4		5	6	7		8		9		10
					ArgRipper GetRipped("ENABLED DRAWENABLED RENDERENABLED SELECTED STYLE RED GREEN BLUE WEIGHT EFFECTTYPE EFFECTNAME");
					strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

					if ((NumArgsParsed = GetRipped.Rip(ButcherableArg)) > 0)
						{
						JoeProperties JoeProp;
						// set properties to change
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(0, Count)) // ENABLED
								{
								JoeProp.Enabled = TestBoolean(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_ENABLED;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(1, Count)) // DRAWENABLED
								{
								JoeProp.DrawEnabled = TestBoolean(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_DRAWENABLED;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(2, Count)) // RENDERENABLED
								{
								JoeProp.RenderEnabled = TestBoolean(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_RENDERENABLED;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(3, Count)) // SELECTED
								{
								JoeProp.Selected = TestBoolean(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_SELECTED;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(4, Count)) // STYLE
								{
								JoeProp.LineStyle = Joe::GetLineStyleFromString(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_LINESTYLE;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(5, Count)) // RED
								{
								JoeProp.Red = atoi(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_RED;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(6, Count)) // GREEN
								{
								JoeProp.Green = atoi(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_GREEN;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(7, Count)) // BLUE
								{
								JoeProp.Blue = atoi(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_BLUE;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(8, Count)) // WEIGHT
								{
								JoeProp.LineWeight = atoi(Result);
								JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_LINEWEIGHT;
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(9, Count)) // EFFECTTYPE
								{
								JoeProp.EffectType = IdentifyEffectType(Result);
								break;
								} // if
							else Count++;
							} // for
						for (Count = 0; Count < NumArgsParsed;)
							{
							if (Result = GetRipped.GetArg(10, Count)) // EFFECTNAME
								{
								if (JoeProp.Effect = LocalApp->AppEffects->FindByName(JoeProp.EffectType, Result))
									JoeProp.Mask |= WCS_JOEPROPERTIES_MASKBIT_EFFECT;
								break;
								} // if
							else Count++;
							} // for
						// operate on database
						if (LocalApp->AppDB->SetJoeProperties(DBFilter, &JoeProp))
							HandleResult = 1;
						} // if
					} // if filters exist
				} // if set properties
			} // if
		break;
		} // PROPERTIES
	} // switch
#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleDataBase

/*===========================================================================*/

long ScriptActor::HandleImageLib(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
long NumArgsParsed; 
char PathPart[256], FilePart[64], ButcherableArg[1024];
int Count;
char *Result;
Raster *Image = NULL;
NotifyTag Changes[2];

switch(ArgScope->LastToken)
	{
	case VE_ADD:
		{
		if(strlen(ArgScope->ArgStr))
			{
			ArgRipper GetRipped("FILE NAME");
			strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

			if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
				{
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(0, Count)) // FILE
						{
						BreakFileName(Result, PathPart, 256, FilePart, 64);
						if ((Image = LocalApp->AppImages->AddRaster(PathPart, FilePart, 1, 1, 1)) && Image != (Raster *)WCS_IMAGE_ERR)
							HandleResult = 1;
						else if (Image == (Raster *)WCS_IMAGE_ERR)
							Image = NULL;
						break;
						} // if
					else Count++;
					} // for
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(1, Count)) // NAME
						{
						if (Image)
							Image->SetUserName(Result);
						break;
						} // if
					else Count++;
					} // for
				if (Image)
					{
					// send notification image added
					Changes[0] = MAKE_ID(Image->GetNotifyClass(), Image->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					LocalApp->AppEx->GenerateNotify(Changes, Image);
					} // if
				} // if
			} // if
		break;
		} // ADD
	case VE_REMOVE:
		{
		if(strlen(ArgScope->ArgStr))
			{
			ArgRipper GetRipped("FILE NAME");
			strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

			if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
				{
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(0, Count)) // FILE
						{
						BreakFileName(Result, PathPart, 256, FilePart, 64);
						if (Image = LocalApp->AppImages->FindByName(PathPart, FilePart))
							{
							LocalApp->AppImages->RemoveRaster(Image, LocalApp->AppEffects);
							HandleResult = 1;
							Changes[0] = MAKE_ID(Image->GetNotifyClass(), Image->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
							Changes[1] = NULL;
							LocalApp->AppEx->GenerateNotify(Changes, Image);
							} // if
						break;
						} // if
					else Count++;
					} // for
				if (! Image)
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // NAME
							{
							if (Image = LocalApp->AppImages->FindByUserName(Result))
								{
								LocalApp->AppImages->RemoveRaster(Image, LocalApp->AppEffects);
								HandleResult = 1;
								Changes[0] = MAKE_ID(Image->GetNotifyClass(), Image->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
								Changes[1] = NULL;
								LocalApp->AppEx->GenerateNotify(Changes, Image);
								} // if
							break;
							} // if
						else Count++;
						} // for
					} // if not already removed
				} // if
			} // if
		break;
		} // REMOVE
	case VE_RENAME:
		{
		if(strlen(ArgScope->ArgStr))
			{
			ArgRipper GetRipped("FILE NAME NEWNAME");
			strncpy(ButcherableArg, ArgScope->ArgStr, 1000); ButcherableArg[1000] = NULL;

			if (NumArgsParsed = GetRipped.Rip(ButcherableArg))
				{
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(0, Count)) // FILE
						{
						BreakFileName(Result, PathPart, 256, FilePart, 64);
						Image = LocalApp->AppImages->FindByName(PathPart, FilePart);
						break;
						} // if
					else Count++;
					} // for
				if (! Image)
					{
					for (Count = 0; Count < NumArgsParsed;)
						{
						if (Result = GetRipped.GetArg(1, Count)) // NAME
							{
							Image = LocalApp->AppImages->FindByUserName(Result);
							break;
							} // if
						else Count++;
						} // for
					} // if
				for (Count = 0; Count < NumArgsParsed;)
					{
					if (Result = GetRipped.GetArg(2, Count)) // NEWNAME
						{
						if (Image)
							Image->SetUserName(Result);
						Changes[0] = MAKE_ID(Image->GetNotifyClass(), Image->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
						Changes[1] = NULL;
						LocalApp->AppEx->GenerateNotify(Changes, Image);
						HandleResult = 1;
						break;
						} // if
					else Count++;
					} // for
				} // if
			} // if
		break;
		} // RENAME
	} // switch
#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleImageLib

/*===========================================================================*/

long ScriptActor::HandleGISImport(struct CmdContext *ArgScope)
{
long HandleResult = -1;
#ifndef WCS_BUILD_DEMO
char Val = -1;

switch(ArgScope->LastToken)
	{
	case VE_LOW:
	case VE_MEDIUM:
	case VE_HIGH:
		{
		// Determine Level
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_LOW) != -1)
			{
			Val = 0;
			} // if
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_MEDIUM) != -1)
			{
			Val = 1;
			} // if
		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_HIGH) != -1)
			{
			Val = 2;
			} // if

		if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_QUALITY) != -1)
			{
			if(Val != -1)
				{
				HandleResult = 0;
				switch(Val)
					{
					//case 0: UserMessageOK("GIS Import", "Quality = Low"); break;
					//case 1: UserMessageOK("GIS Import", "Quality = Medium"); break;
					//case 2: UserMessageOK("GIS Import", "Quality = High"); break;
					case -1:
					default: HandleResult = -1; break;
					} // switch
				} // if
			} // if
		else if(ArgScope->SourceScript->IsWordInSentance(ArgScope, VE_SPEED) != -1)
			{
			if(Val != -1)
				{
				switch(Val)
					{
					//case 0: UserMessageOK("GIS Import", "Speed = Low"); break;
					//case 1: UserMessageOK("GIS Import", "Speed = Medium"); break;
					//case 2: UserMessageOK("GIS Import", "Speed = High"); break;
					case -1:
					default: HandleResult = -1; break;
					} // switch
				HandleResult = 0;
				} // if
			} // if
		break;
		} // QUALITY/SPEED
/*
	case VE_RELATIONS:
		{
		//UserMessageOK("GIS Import", "Please confirm Ecosystem relations.");
		HandleResult = 0;
		break;
		} // CONFIRM ECO RELATIONS
	case VE_VIEW:
		{
		if(UserMessageYN("GIS Import", "Is this Viewpoint Acceptible?"))
			{
			HandleResult = 0;
			} // if
		break;
		} // CONFIRM CAMERA AND VIEW
*/
	} // LastToken
#endif // !DEMO
return(HandleResult);
} // ScriptActor::HandleGISImport

/*===========================================================================*/

long ScriptActor::HandleQuit(struct CmdContext *ArgScope)
{
LocalApp->SetTerminate();
return(0);
} // ScriptActor::HandleQuit

/*===========================================================================*/

Fenetre *ScriptActor::FindWinFromArg(const char *WinArg)
{
#ifndef WCS_BUILD_DEMO
if(WinArg && strlen(WinArg))
	{
	return(LocalApp->WinSys->FindOpenFen(MakeIDFromString(WinArg)));
	} // if
#endif // !DEMO
return(NULL);
} // ScriptActor::FindWinFromArg

/*===========================================================================*/

int ScriptActor::GetNumericArgs(const char *NumericString)
{
int ArgCount = 0;
#ifndef WCS_BUILD_DEMO
char *NextArg;

strncpy(ScriptActorNumericArgBuf, NumericString, 255);
ScriptActorNumericArgBuf[255] = NULL;
NextArg = strtok(ScriptActorNumericArgBuf, " \t");
while(NextArg != NULL)
	{
	if(ArgCount < 10)
		{
		NumericArgs[ArgCount++] = atof(NextArg);
		NextArg = strtok(NULL, " \t");
		} // if
	} // while
#endif // !DEMO
return(ArgCount);
} // ScriptActor::FindWinFromArg

/*===========================================================================*/

long ScriptActor::IdentifyEffectType(int Token)
{
#ifndef WCS_BUILD_DEMO

switch (Token)
	{
	case VE_ENVIRONMENT:	return (WCS_EFFECTSSUBCLASS_ENVIRONMENT);
	case VE_ECOSYSTEM:	return (WCS_EFFECTSSUBCLASS_ECOSYSTEM);
	case VE_COLORMAP:	return (WCS_EFFECTSSUBCLASS_CMAP);
	case VE_LAKE:	return (WCS_EFFECTSSUBCLASS_LAKE);
	case VE_CAMERA:	return (WCS_EFFECTSSUBCLASS_CAMERA);
	} // switch
#endif // !DEMO
return (0);

} // ScriptActor::IdentifyEffectType

/*===========================================================================*/

long ScriptActor::IdentifyEffectType(char *Test)
{
#ifndef WCS_BUILD_DEMO

if (!stricmp(Test, "ENVIRONMENT"))	return (WCS_EFFECTSSUBCLASS_ENVIRONMENT);
if (!stricmp(Test, "ECOSYSTEM"))	return (WCS_EFFECTSSUBCLASS_ECOSYSTEM);
if (!stricmp(Test, "COLORMAP"))	return (WCS_EFFECTSSUBCLASS_CMAP);
if (!stricmp(Test, "LAKE"))	return (WCS_EFFECTSSUBCLASS_LAKE);
if (!stricmp(Test, "CAMERA"))	return (WCS_EFFECTSSUBCLASS_CAMERA);
if (!stricmp(Test, "LIGHT"))	return (WCS_EFFECTSSUBCLASS_LIGHT);
if (!stricmp(Test, "TERRAFFECTOR"))	return (WCS_EFFECTSSUBCLASS_TERRAFFECTOR);
if (!stricmp(Test, "AREATERRAFFECTOR"))	return (WCS_EFFECTSSUBCLASS_RASTERTA);
if (!stricmp(Test, "STREAM"))	return (WCS_EFFECTSSUBCLASS_STREAM);
if (!stricmp(Test, "GROUND"))	return (WCS_EFFECTSSUBCLASS_GROUND);
if (!stricmp(Test, "SNOW"))	return (WCS_EFFECTSSUBCLASS_SNOW);
#endif // !DEMO
return (0);

} // ScriptActor::IdentifyEffectType

/*===========================================================================*/

int ScriptActor::TestBoolean(char *TestStr)
{
#ifndef WCS_BUILD_DEMO

if (atoi(TestStr))
	return (1);
if (TestStr[0] == 't' || TestStr[0] == 'T')
	return (1);
if (TestStr[0] == 'y' || TestStr[0] == 'Y')
	return (1);
#endif // !DEMO
return (0);

} // ScriptActor::TestBoolean

/*===========================================================================*/

DBFilterEvent *ScriptActor::AddFilter(void)
{
DBFilterEvent **CurPtr = &DBFilter, *NewFilter = NULL;
#ifndef WCS_BUILD_DEMO

while (*CurPtr)
	CurPtr = &(*CurPtr)->Next;

if (*CurPtr = new DBFilterEvent())
	{
	NewFilter = *CurPtr;
	NewFilter->SetScriptDefaults();
	} // if
#endif // !DEMO
return (NewFilter);

} // ScriptActor::AddFilter

/*===========================================================================*/

void ScriptActor::RemoveFilter(void)
{
#ifndef WCS_BUILD_DEMO
DBFilterEvent *RemFilter = DBFilter;

while (RemFilter && RemFilter->Next && RemFilter->Next->Next)
	RemFilter = RemFilter->Next;

if (RemFilter)
	{
	if (RemFilter->Next)
		{
		delete RemFilter->Next;
		RemFilter->Next = NULL;
		CurFilter = RemFilter;
		} // if
	else
		{
		delete RemFilter;
		DBFilter = CurFilter = NULL;
		} // else only one filter
	} // if
#endif // !DEMO
} // ScriptActor::RemoveFilter

/*===========================================================================*/
