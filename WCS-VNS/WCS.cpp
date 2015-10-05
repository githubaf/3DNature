// WCS.cpp
// Used to be AppTest.cpp, 12/13/95-CXH
// AppTest.cpp
// Hack to test object-oriented application management
// written from scratch on 27 May 1995 by Chris "Xenon" Hanson
// (first functioned properly on 01 Jun 1995 at 01:33:39!)

#include "stdafx.h"

#define WCSCPP

#ifdef MS_MEM_DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // _MSC_VER
#endif // MS_MEM_DEBUG

#define TRACKMEM_MORE

#include "Joe.h"
#include "Requester.h"
#include "Application.h"
#include "Useful.h"
#include "AppMem.h"
#include "Render.h"
#include "Toolbar.h"
#include "WCSWidgets.h"
#include "Project.h"
#include "Database.h"
#include "Log.h"
#include "Notify.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "Script.h"
#include "ScriptAction.h"
#include "AppHelp.h"
#include "TrigTable.h"
#include "SceneViewGUI.h"
#include "ViewGUI.h"
#include "Security.h"
#include "WCSVersion.h"
#include "Tables.h"
#include "Texture.h"
#include "EDSSControl.h"
#include "ImageFormatConfig.h"
#ifdef WCS_BUILD_ECW_SUPPORT
#include "ImageInputFormat.h"
#endif // WCS_BUILD_ECW_SUPPORT
#include "QuantaAllocator.h"
#include "VectorNode.h"
#include "Lists.h"
#include "VectorPolygon.h"
#include "PixelManager.h"
#include "UsefulCPU.h"
#include "UnitTests.h"

#ifdef WCS_BUILD_FRANK
#if defined(_OPENMP)
#include <omp.h>
#endif // _OPENMP
#endif // WCS_BUILD_FRANK

//#ifdef WCS_BUILD_FRANK
//void *megaChunk[5];
//#endif // WCS_BUILD_FRANK

WCSApp *GlobalApp;

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
unsigned __int64 DevCounter[50];
#endif // FRANK or GARY

// Global flags to indicate instruction sets we can call
//unsigned long HasSSE2;
#ifdef DOMERGE
unsigned long DoMerge;
#endif // DOMERGE

// I'd like a SETCONFIGOPT command here too, maybe later

#ifdef DOMERGE
static char RipTemplate[300], *TemplateConst = "STARTUPSCRIPT PROJECT/DFLT RENDER AUTOQUIT ICONIFIED HELP EDSS NEWPROJECT TEMPLATE SHAPEFILE WCSPROJECTS WCSCONTENT WCSFRAMES SUPPRESSRENDERNOTIFY LOGFILE NOVIEWS ENGINEONLY PREFSFILE DOMERGE";
#else // DOMERGE
static char RipTemplate[300], *TemplateConst = "STARTUPSCRIPT PROJECT/DFLT RENDER AUTOQUIT ICONIFIED HELP EDSS NEWPROJECT TEMPLATE SHAPEFILE WCSPROJECTS WCSCONTENT WCSFRAMES SUPPRESSRENDERNOTIFY LOGFILE NOVIEWS ENGINEONLY PREFSFILE SOFTLICENSE";
#endif // !DOMERGE

char OverridePrefsFile[512];
int AutoStartRender, AutoQuitDone, RenderIconified, suppress_rendernotify, suppress_openviews, EngineOnly;

// As the name implies, this is part of the Application
// object, and gets called if all the generalized init code in
// the App object constructor succeeds.
//
// This method is implemented outside of Application.cpp, usually
// in the main or startup module of your application.
//
// Here, you perform application-specific initialization and
// allocation of object pointers found in the WCSApp object.
//
// It doesn't hurt to have lots of extraneous pointers in the
// Application object, as long as you don't use them (or init
// them in this method) and don't like with their code, they're
// just unused pointers and you can ignore them.
//
// In this way, we can share one (compiled) Application object
// with many different projects or applications without having
// to rebuild it every time we rebuild a different app.

/*===========================================================================*/

unsigned long WCSApp::AppSpecificInit(void)
{
CPU_Caps cpu;

GlobalApp = this;

Sentinal = new Security;

if (! cpu.Has_SSE2())
	{
	AppHelp::OpenURLIndirect("http://en.wikipedia.org/wiki/SSE2#CPUs_supporting_SSE2");
	UserMessageOK("Unsupported CPU", "This program requires a CPU that has the SSE2 instruction set.");
	exit(123);
	} // if

CountBits = CountBits_Std;
#if _MSC_VER >= 1500
if (cpu.Has_POPCNT())
	CountBits = CountBits_POPCNT;
#endif // _MSC_VER

if (!(StatusLog = new MessageLog))
	{
	ErrorStatus = WCS_ERR_LOG_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(MathTables = new MathAccellerators))
	{
	ErrorStatus = WCS_ERR_MATH_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(Toaster = new PopupNotifier))
	{
	ErrorStatus = WCS_ERR_NOTIFY_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(AppEx = new NotifyEx))
	{
	ErrorStatus = WCS_ERR_NOTIFY_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(AppDB = new Database))
	{
	ErrorStatus = WCS_ERR_DB_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(MainProj = new Project))
	{
	ErrorStatus = WCS_ERR_PROJ_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(AppImages = new ImageLib()))
	{
	ErrorStatus = WCS_ERR_IMAGES_ALLOCATE;
	return(ErrorStatus);
	} // if
LoadToImageLib = CopyFromImageLib = CopyToImageLib = AppImages;

if (!(AppEffects = new EffectsLib()))
	{
	ErrorStatus = WCS_ERR_EFFECTS_ALLOCATE;
	return(ErrorStatus);
	} // if
if (!(AppEffects->FetchDefaultCoordSys()))
	{
	ErrorStatus = WCS_ERR_EFFECTS_ALLOCATE;
	return(ErrorStatus);
	} // if
LoadToEffectsLib = CopyFromEffectsLib = CopyToEffectsLib = AppEffects;

if (!(SuperScript = new ScriptParser(StatusLog)))
	{
	ErrorStatus = WCS_ERR_SCRIPT_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(Bogart = new ScriptActor(SuperScript)))
	{
	ErrorStatus = WCS_ERR_SCRIPT_ALLOCATE;
	return(ErrorStatus);
	} // if

if (! (MainProj->Interactive = new InterCommon(AppDB, MainProj)))
	{
	ErrorStatus = WCS_ERR_INTERCOMMON_ALLOCATE;
	return(ErrorStatus);
	} // if

if (!(MCP = new Toolbar(this)))
	{
	ErrorStatus = WCS_ERR_TOOLBAR_ALLOCATE;
	return(ErrorStatus);
	} // if
else
	{
	MCP->Open(NULL);
	} // else

if (! (GUIWins = new Conservatory))
	{
	ErrorStatus = WCS_ERR_GUIWINS_ALLOCATE;
	return(ErrorStatus);
	} // if

HelpSys = new AppHelp;

// Auto-Open Scene@Glance now that pointers are valid.
if (GUIWins->SAG = new SceneViewGUI(AppDB, AppEffects, MainProj, AppImages))
 	{
 	if (GUIWins->SAG->ConstructError)
 		{
 		delete GUIWins->SAG;
 		GUIWins->SAG = NULL;
		ErrorStatus = WCS_ERR_GUIWINS_ALLOCATE;
 		} // if
	else
		{
	 	GlobalApp->GUIWins->SAG->Open(GlobalApp->MainProj);
		} // else
 	} // if

// Auto-Open ViewGUI
if (GUIWins->CVG = new ViewGUI(GlobalApp->MainProj, AppDB, MainProj->Interactive))
 	{
 	if (GUIWins->CVG->ConstructError)
 		{
 		delete GUIWins->CVG;
 		GUIWins->CVG = NULL;
		ErrorStatus = WCS_ERR_GUIWINS_ALLOCATE;
 		} // if
	else
		{
	 	GlobalApp->GUIWins->CVG->Open(GlobalApp->MainProj);
		} // else
 	} // if

#ifdef WCS_BUILD_ECW_SUPPORT
// try to run-time init ECW support
if (ECWSupport = new ECWDLLAPI)
	{
	if (!ECWSupport->InitOk())
		{
		delete ECWSupport;
		ECWSupport = NULL;
		} // if
	} // if
#endif // WCS_BUILD_ECW_SUPPORT

// Do InquireSignalMask() on any special modules that may need it
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, cpu.GetCapString());

//#ifdef WCS_BUILD_FRANK
//megaChunk[0] = malloc(512 * 1024 * 1024);
//megaChunk[1] = malloc(256 * 1024 * 1024);
//megaChunk[2] = malloc(128 * 1024 * 1024);
//megaChunk[3] = malloc(64 * 1024 * 1024);
//megaChunk[4] = malloc(32 * 1024 * 1024);
//#endif // WCS_BUILD_FRANK

return(0);

} // WCSApp::AppSpecificInit

/*===========================================================================*/

// This cleans up from the above
void WCSApp::AppSpecificCleanup(void)
{

//#ifdef WCS_BUILD_FRANK
//if (megaChunk[4])
//	free(megaChunk[4]);
//if (megaChunk[3])
//	free(megaChunk[3]);
//if (megaChunk[2])
//	free(megaChunk[2]);
//if (megaChunk[1])
//	free(megaChunk[1]);
//if (megaChunk[0])
//	free(megaChunk[0]);
//#endif // WCS_BUILD_FRANK

// *_*_*_*_*_*_*_*_*_*_*_*_*_*   IMPORTANT   *_*_*_*_*_*_*_*_*_*_*_*_*_*
//
// These must be deleted in the reverse order of their construction
// in AppSpecificInit, above.

DatabaseDisposalInProgress = 1;
ImageDisposalInProgress = 1;
EffectsDisposalInProgress = 1;

#ifdef WCS_BUILD_ECW_SUPPORT
delete ECWSupport; ECWSupport = NULL;
#endif // WCS_BUILD_ECW_SUPPORT
delete HelpSys; HelpSys = NULL;
delete GUIWins; GUIWins = NULL;
delete Bogart; Bogart = NULL;
delete SuperScript; SuperScript = NULL;
delete MCP; MCP = NULL;
delete StartEDSS; StartEDSS = NULL;
delete AppImages; AppImages = NULL;
delete AppEffects; AppEffects = NULL;
delete MainProj; MainProj = NULL; // deletes InterCommon member "Interactive"
delete AppDB; AppDB = NULL;
delete AppEx; AppEx = NULL;
delete Toaster; Toaster = NULL;
delete MathTables; MathTables = NULL;
delete StatusLog; StatusLog = NULL;
delete Sentinal; Sentinal = NULL;
} // WCSApp::AppSpecificCleanup

/*===========================================================================*/
/*===========================================================================*/

#ifndef _WIN32
int main(int Count, char *Vector[]);
#else // !_WIN32
extern "C" {
int WINAPI WinMain(NativeLoadHandle Instance, NativeLoadHandle PrevInst, LPSTR Param,
	int Show);
#endif // !_WIN32

#ifndef _WIN32
int main(int Count, char *Vector[])
{
NativeLoadHandle Instance = NULL;
const char *Param = Vector[0];
#else // !_WIN32
int WINAPI WinMain(NativeLoadHandle Instance, NativeLoadHandle PrevInst, LPSTR Param,
	int Show)
{ // } matching bracket to balance brace-match
#endif // !_WIN32
unsigned long ErrorLevel, RanScript = 0, AuthValue = 0;
unsigned short Reply, LoadedPrefs = 0;
char ScriptName[255];
const char *CurFName = NULL, *ScriptArg;
FileReq *ProjLoad = NULL;
#ifdef _WIN32
int Count;
#endif // _WIN32
int NumArgsParsed, AutoValidate = 1;
char *Result;

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
for (unsigned long DC = 0; DC < 50; DC++)
	DevCounter[DC] = 0;
#endif // FRANK or GARY

#ifdef MS_MEM_DEBUG
#ifdef _MSC_VER
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _MSC_VER
#endif // MS_MEM_DEBUG

// enable to test memory allocation tracking routines
//void *err = malloc(123);

GlobalApp = new WCSApp (Instance, (const char *)Param, Show = SW_SHOWMAXIMIZED);

// F2_NOTE: We should put up a requester & exit gracefully if SSE2 isn't found also

if (!GlobalApp)
	{
	return(-1);
	} // if
else if (ErrorLevel = GlobalApp->InquireInitError())
	{
 	char ErrorReport[80];
 	sprintf(ErrorReport, "Startup failed, unintelligble number = %d.", ErrorLevel);
 	UserMessageOK("Application Initialization", ErrorReport, REQUESTER_ICON_STOP);
	delete GlobalApp;
	return(ErrorLevel);
	} // if


	#ifdef DEBUG
	//GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_NULL, GlobalApp->GetStartupArgs());
	#endif // DEBUG

/*
#ifdef WCS_BUILD_DEMO
{
time_t Now;

(void)time(&Now);
if (Now > (1031351857 + 2419200)) // four weeks from Sept08-2002
	{
 	UserMessageOK("Application Initialization", "Time limited build expired.", REQUESTER_ICON_STOP);
	Now = 0;
	delete GlobalApp;
	return(ErrorLevel);
	} // if
}
#endif // WCS_BUILD_DEMO
*/

	// find Tools directory, otherwise quit
	if (! GlobalApp->CheckToolsDir(GlobalApp->GetProgDir()))
		{
		UserMessageOK("Tools Check", "The Tools directory could not be found.\nThere must be a Tools directory in the same directory as the program executable.\n\
Place the program executable file in the WCS directory created by the installer on your hard drive.\nThe program will exit when the OK button is pressed.", REQUESTER_ICON_STOP);
		goto EndRun;
		} // if no Tools dir


#ifdef WCS_BUILD_FRANK
#if defined(_OPENMP)
	int omp_var;

	omp_var = omp_get_num_procs();
	omp_var = omp_get_max_threads();
	omp_var = _OPENMP;
#endif // _OPENMP
#endif // WCS_BUILD_FRANK

	if (ScriptArg = GlobalApp->GetStartupArgs())
		{
		char ButcherableArg[1024];

		strncpy(ButcherableArg, ScriptArg, 1000); ButcherableArg[1000] = NULL;
		strcpy(RipTemplate, TemplateConst);
		ArgRipper JackThe(RipTemplate);

		if (NumArgsParsed = JackThe.Rip(ButcherableArg))
			{
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(5, Count)) // HELP
					{
					// This code doesn't work, but I'm leaving it in in case I figure out
					// how to make it work someday.
					_cprintf("%s\n%s\n%s\n\nUsage: %s\n", APP_TITLE, ExtAmiVersionTag, ExtCopyrightFull, TemplateConst);
					} // if
				else Count++;
				} // for
			// SOFTLICENSE (used for both old-style and new-style NLM)
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(18, Count)) // 18=SOFTLICENSE
					{
#ifndef WCS_BUILD_DEMO
					GlobalApp->Sentinal->SetAttemptNLM(1);
					GlobalApp->Sentinal->SetPreferredNLMHost(Result); // pass our internal equivalent of NSP_HOST
#endif // !WCS_BUILD_DEMO
					} // if
				else Count++;
				} // for
			} // if

// do dongle-checking now that we've potentially handled softlicense arg
#ifndef WCS_BUILD_DEMO
		if (!GlobalApp->Sentinal->CheckDongle())
			{
			GlobalApp->WinSys->UpdateRootTitle(NULL, NULL);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR + 1, "Hardware Key not found. Configuring as Render Engine.");
			#ifndef WCS_BUILD_DEMO
			GlobalApp->Sentinal->SetRenderEngineCached(1);
			#endif // !WCS_BUILD_DEMO
			} // if
#endif // !WCS_BUILD_DEMO

#ifdef WCS_BUILD_RTX
		GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0;
#endif // WCS_BUILD_RTX
#ifdef WCS_FORESTRY_WIZARD
		GlobalApp->ForestryAuthorized = GlobalApp->Sentinal->CheckAuthFieldForestry() ? 1: 0;
#endif // WCS_FORESTRY_WIZARD

		// prefs loading could be affected by dongle state?
		LoadedPrefs = GlobalApp->MainProj->LoadPrefs(GlobalApp->GetProgDir());


		// Advanced config
		// publish names of known hacks
		// this must be done AFTER loading prefs as prefs won't load configopts if any are already present
		if (!GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("avoid_glreadpixels"))
			{
			GlobalApp->MainProj->Prefs.SetConfigOpt("avoid_glreadpixels", "no");
			} // if

		if (!GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("suppress_rendernotify"))
			{
			GlobalApp->MainProj->Prefs.SetConfigOpt("suppress_rendernotify", "no");
			} // if

		if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("log_file"))
			{
			GlobalApp->StatusLog->OpenLogFile(GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("log_file"));
			} // if

#ifdef WCS_BUILD_FRANK
HANDLE hProcess, hReal;
PDWORD pam;
PDWORD sam;
//size_t minWSS;
//size_t maxWSS;

if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOptTrue("single_core"))
	{
	hProcess = GetCurrentProcess();
	if (DuplicateHandle(GetCurrentProcess(), hProcess, GetCurrentProcess(), &hReal, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
		if (GetProcessAffinityMask(hReal, (PDWORD_PTR)&pam, (PDWORD_PTR)&sam))
			{
			if (SetProcessAffinityMask(hReal, 0x1))
				UserMessageOK("Dev Test", "Single Core Mode Enabled");
			} // if
		} // if
	//if (GetProcessWorkingSetSize(hProcess, (PSIZE_T)&minWSS, (PSIZE_T)&maxWSS))
	//	{
	//	printf("here");
	//	} // if
	} // if
#endif // WCS_BUILD_FRANK

		// handle other args that might want dongle state resolve first
		if (NumArgsParsed)
			{
			// PREFSFILE -- do this early in case it affects later options
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(17, Count)) // PREFSFILE
					{
					// Picked up via extern in Project.cpp
					strncpy(OverridePrefsFile, Result, 500);
					OverridePrefsFile[500] = NULL;
					LoadedPrefs = GlobalApp->MainProj->LoadPrefs(GlobalApp->GetProgDir());
					} // if
				else Count++;
				} // for
			// ENGINEONLY SUPPRESSRENDERNOTIFY LOGFILE NOVIEWS -- do these early in case they affect later options
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(16, Count)) // ENGINEONLY
					{
					// Picked up via extern in render.cpp
					EngineOnly = 1;
					#ifndef WCS_BUILD_DEMO
					GlobalApp->Sentinal->SetRenderEngineCached(1);
					#endif // !WCS_BUILD_DEMO
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(13, Count)) // SUPPRESSRENDERNOTIFY
					{
					// Picked up via extern in render.cpp
					suppress_rendernotify = 1;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(14, Count)) // LOGFILE
					{
					// Set Master Path
					GlobalApp->StatusLog->OpenLogFile(Result);
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(15, Count)) // NOVIEWS
					{
					// Picked up via extern in ViewGUI.cpp
					suppress_openviews = 1;
					} // if
				else Count++;
				} // for
			// WCSPROJECTS WCSCONTENT WCSFRAMES
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(10, Count)) // WCSPROJECTS
					{
					// Set Master Path
					GlobalApp->MainProj->SetProjectPath(Result);
					// we assume we're running under batch control and it's better to fail
					// than to block, so we won't try autovalidating if you specify a path this way
					AutoValidate = 0;
					continue;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(11, Count)) // WCSCONTENT
					{
					// Set Master Path
					GlobalApp->MainProj->SetContentPath(Result);
					// we assume we're running under batch control and it's better to fail
					// than to block, so we won't try autovalidating if you specify a path this way
					AutoValidate = 0;
					continue;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(12, Count)) // WCSFRAMES
					{
					// Set Master Path
					GlobalApp->MainProj->SetFramesPath(Result);
					// we assume we're running under batch control and it's better to fail
					// than to block, so we won't try autovalidating if you specify a path this way
					AutoValidate = 0;
					continue;
					} // if
				else Count++;
				} // for

			// if necessary, autovalidate master paths before processing further args
			if (AutoValidate)
				{
				// To avoid much user confusion we'll make them establish valid assign paths or quit
				AutoValidate = 0; // don't do it again later
				if (! GlobalApp->MainProj->ValidateAssignPaths())
					{
					goto EndRun;
					} // if
				} // if


			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(1, Count)) // PROJECT
					{
					// Not exactly sure how to do this call, but here's the filename anyway...
					GlobalApp->MainProj->Load(NULL, Result, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL, 0xffffffff);
					continue;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(0, Count)) // STARTUPSCRIPT
					{
					strcpy(ScriptName, Result);
					GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_START_SCRIPT, ScriptName);
					RanScript = GlobalApp->SuperScript->StartScript(ScriptName);
					continue;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(3, Count)) // AUTOQUIT
					{
					AutoQuitDone = 1;
					} // if
				else Count++;
				} // for
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(4, Count)) // ICONIFIED
					{
					RenderIconified = 1;
					} // if
				else Count++;
				} // for
			#ifdef WCS_BUILD_EDSS
			#ifndef WCS_BUILD_DEMO
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(6, Count)) // EDSS
					{
					if (GlobalApp->Sentinal->CheckAuthField('E', NULL, WCS_SECURITY_AUTH_FIELD_B))
						{
						if (GlobalApp->StartEDSS = new EDSSControl(&JackThe, NumArgsParsed))
							{
							GlobalApp->EDSSEnabled = 1;
							if (! GlobalApp->StartEDSS->InitError)
								GlobalApp->StartEDSS->InstantiateGUI();
							else
								{
								UserMessageOK("EDSS", "Incorrect arguments on command line. EDSS Interface can not be launched.");
								delete GlobalApp->StartEDSS;
								GlobalApp->StartEDSS = NULL;
								GlobalApp->SetTerminate();
								} // else
							} // if
						} // if
					else
						{
						// Prompt for Authorization
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "EDSS not enabled.");
						GlobalApp->Sentinal->DoAuth('E', WCS_SECURITY_AUTH_FIELD_B);
						break;
						} // else
					} // if
				else Count++;
				} // for
			#endif // !WCS_BUILD_DEMO
			#endif // WCS_BUILD_EDSS
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(2, Count)) // RENDER
					{
					AutoStartRender = 1;
					GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
					 WCS_TOOLBAR_ITEM_RCG, 0);
					} // if
				else Count++;
				} // for

#ifdef DOMERGE
			// DOMERGE
			for (Count = 0; Count < NumArgsParsed;)
				{
				if (Result = JackThe.GetArg(18, Count)) // DOMERGE
					{
					// Picked up via extern in DEMMergeGUI.cpp
					DoMerge = 1;
					// find first merger and call edit on it, DoMerge will cause it to autorun
					if (GlobalApp->AppEffects->Merger)
						{
						GlobalApp->AppEffects->Merger->Edit();
						} // if
					} // if
				else Count++;
				} // for
#endif // DOMERGE

			} // if
		} // if
	// if we didn't previously, autovalidate master paths before going on
	if (AutoValidate)
		{
		// To avoid much user confusion we'll make them establish valid assign paths or quit
		AutoValidate = 0; // don't do it again later
		if (! GlobalApp->MainProj->ValidateAssignPaths())
			{
			goto EndRun;
			} // if
		} // if

	if (!RanScript)
		{
// Let's be persnickity and check the dongle again here, since logging is now available
#ifndef WCS_BUILD_DEMO
if (!GlobalApp->Sentinal->CheckDongle())
	{
	int NotifyNoAuth = 1;
	if (!EngineOnly) // if we're LM-enabled and run as ENGINEONLY, don't try to grab a network license
		{
		if (NotifyNoAuth)
			{
			GlobalApp->WinSys->UpdateRootTitle(NULL, NULL);
			#ifndef WCS_BUILD_DEMO
			GlobalApp->Sentinal->SetRenderEngineCached(1);
			#endif // !WCS_BUILD_DEMO

			// dongle-specific error message
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR + 1, "Hardware Key not found. Configuring as Render Engine.");

			//if (!UserMessageCustom("Security Check", "Hardware Key not found.\nConfiguring as Render Engine.", "Ok", "Help", NULL, 1))
			UserMessageCustom("Security Check", "Hardware Key not found.\nConfiguring as Render Engine.", "Ok", "Help", NULL, 1);
				{ // no longer conditional, display this tip everytime we can't find dongle
				UserMessageOK("Hardware Key Tip", "Ensure hardware key is connected to computer.\nIf problem persists, download and install\nlatest drivers from www.SetupMyKey.com.");
				} // if
			} // if

		} // if
	} // if
else
	{
// be aware of similar code in Security::ErrorCheckNewAuthorizationValue() that needs updating if you change this
#ifdef WCS_BUILD_VNS
	if (!GlobalApp->Sentinal->CheckAuthField(0, (unsigned char *)"EW", WCS_SECURITY_AUTH_FIELD_B) || GlobalApp->Sentinal->CheckHL())
		{
		GlobalApp->Sentinal->DoAuth(0, WCS_SECURITY_AUTH_FIELD_B);
		} // if
#else // !WCS_BUILD_VNS
	if (!GlobalApp->Sentinal->CheckAuthField(0, (unsigned char *)"S") || GlobalApp->Sentinal->CheckHL())
		{
		GlobalApp->Sentinal->DoAuth(0);
		} // if
#endif // !WCS_BUILD_VNS

	} // else
		if (!GlobalApp->GetTerminate())
			{ // prevent running via Terminal Services/Remote Desktop/Citrix desktop sharing (which is prohibited by license)
			if (!GlobalApp->Sentinal->CheckRenderEngineQuick() && GlobalApp->WinSys->InquireRemoteSession())
				{
				UserMessageOK("Remote Desktop Prohibited", "Use of this software via desktop sharing\ntechnologies is prohibited per the license.\nExiting.");
				GlobalApp->SetTerminate();
				} // if
			} // if
#endif // WCS_BUILD_DEMO
		if (!GlobalApp->GetTerminate() && ! GlobalApp->EDSSEnabled)
			{
			if (!GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("suppress_startupwin"))
				{
				GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
				 WCS_TOOLBAR_ITEM_VER, 0);
				} // if
			} // if
		} // if

// RunUnitTests(); // run oddball in-environment tests if needed

if (!GlobalApp->GetTerminate())
	{
	if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("netscript_permit_addr"))
		{
		int PreferredPort = 0;

		if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("netscript_port_num"))
			{
			PreferredPort = atoi(GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("netscript_port_num"));
			} // if
		if (!PreferredPort) PreferredPort = 4242; // default to 4242 to save setup work and spelling mistakes
		GlobalApp->SuperScript->SetupNetworkScript(PreferredPort, GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("netscript_permit_addr"));
		} // if
	GlobalApp->LoopAndWait();
	} // if

EndRun:

if (GlobalApp->GetTerminate() == 2)
	{
	UserMessageOK("Security Check", "Hardware key not responding. Exiting.", REQUESTER_ICON_STOP);
	} // if
Reply = GlobalApp->ResultCode();

#ifndef WCS_BUILD_DEMO
GlobalApp->MainProj->SavePrefs(GlobalApp->GetProgDir());
#endif // WCS_BUILD_DEMO

CoordSys::ProjSysTable.FreeAll();
GeoDatum::DatmTable.FreeAll();
GeoEllipsoid::EllipseTable.FreeAll();
ProjectionMethod::MethodTable.FreeAll();
ProjectionMethod::ParamTable.FreeAll();
RootTexture::DestroyPreviewSubjects();

delete GlobalApp;
GlobalApp = NULL;

#ifdef TRACKMEM_MORE
{
AppMem_ReportUsage();
AppMem_ReportTrackLeftovers();
}
#endif // TRACKMEM_MORE

#ifdef MS_MEM_DEBUG
_CrtDumpMemoryLeaks();
#endif // MS_MEM_DEBUG

return(Reply);
} // main / WinMain

} // extern "C"
