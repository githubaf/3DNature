// Application.cpp
// WCSApp code
// Created from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Application.h"
#include "Project.h"
#include "Interactive.h"
#include "Raster.h"
#include "EffectsLib.h"
#include "Database.h"
#include "AppModule.h"
#include "GUI.h"
#include "Fenetre.h"
#include "Toolbar.h"
#include "Notify.h"
#include "Script.h"

#include "typeinfo.h"

extern WCSApp *GlobalApp;

/*===========================================================================*/
//                                 Common Code
/*===========================================================================*/

WCSApp::WCSApp(NativeLoadHandle Instance, const char *FirstParam, int StartupCode)
{
#ifdef _WIN32
long FindSlash;
char ProgName[32];
char OpenCommand[300];
LPSTR Chomp;
SDFK = NULL;
#endif // _WIN32
Terminate = 0;
ErrorStatus = 0;
ReturnCode = 0;
ProgDir[0] = NULL;
Delayed = NULL;
InquireTempBuf[0] = 0;

IntInst = Instance;

// Clear sub-object pointers
WinSys = NULL; AppDB = NULL; MainProj = NULL; StatusLog = NULL;
Engine = NULL; MCP = NULL; HelpSys = NULL; GUIWins = NULL;
Sentinal = NULL; AppEffects = LoadToEffectsLib = CopyFromEffectsLib = CopyToEffectsLib = NULL;
AppImages = LoadToImageLib = CopyFromImageLib = CopyToImageLib = NULL;
SuperScript = NULL;
Bogart = NULL;
MathTables = NULL;
AppEx = NULL;
Toaster = NULL;
StartEDSS = NULL;
TemplateLoadInProgress = ComponentSaveInProgress = EDSSEnabled = SXAuthorized = ForestryAuthorized = 0;
DatabaseDisposalInProgress = ImageDisposalInProgress = EffectsDisposalInProgress = 0;

StartupParam = NULL;

#ifdef WCS_BUILD_ECW_SUPPORT
ECWSupport = NULL;
#endif // WCS_BUILD_ECW_SUPPORT


// Initialize Background Handler support
for (NumBackGroundHandlers = 0; NumBackGroundHandlers < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX; NumBackGroundHandlers++)
	{
	BackGroundHandlers[NumBackGroundHandlers] = NULL;
	} // for
NumBackGroundHandlers = 0;

// Enable Windows FP exceptions in debug builds
#ifdef _WIN32
#ifdef DEBUG
// Get the default control word.
int cw = _controlfp( 0,0 );

// Set the exception bits ON.
cw &=~(EM_OVERFLOW|EM_UNDERFLOW|/*EM_INEXACT|*/EM_ZERODIVIDE|EM_DENORMAL);

// Set the control word.
//#if !defined(WCS_BUILD_FRANK)	// Frank's OpenGL has become totally unstable in debug when this is set
_controlfp( cw, MCW_EM );
//#endif // WCS_BUILD_FRANK
#endif // DEBUG
#endif // _WIN32

if (FirstParam)
	{
	StartupParam = FirstParam;
	} // if

#ifdef _WIN32
ProgName[0] = NULL;
for (FindSlash = (long)strlen(_pgmptr) - 1; FindSlash > 0; FindSlash--)
	{
	if (_pgmptr[FindSlash] == '\\')
		{
		strcpy(ProgName, &_pgmptr[FindSlash + 1]);
		break;
		} // if
	} // for
if (ProgName[0])
	{
	if (ProgName[strlen(ProgName) - 4] != '.')
		{
		strcat(ProgName, ".exe");
		} // if
	} // if
SearchPath(NULL, ProgName, NULL, 254, ProgDir, &Chomp);
strcpy(OpenCommand, ProgDir);
*Chomp = NULL;
#endif // _WIN32

#ifdef _WIN32
#define _WIN32_DCOM
// needed for SHBrowseForFolder()
CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif // _WIN32


WinSys = new GUIContext(Instance, StartupCode);

if (!WinSys)
	{
	ErrorStatus = WCS_ERR_GUI_ALLOCATE;
	return;
	} // if

if (!(ErrorStatus = WinSys->InquireInitError()))
	{
	// Initialize application-defined members
	(void)AppSpecificInit(); // Sets ErrorStatus
	if (MainProj) MainProj->ViewPorts.SetCurrent(5);
	} // if

#ifdef _WIN32
// We'll associate our file extension with our
// executable if it isn't already taken...
	{	//lint !e539
	HKEY hkResult, ExtKey = NULL, AppKey = NULL, ShellKey = NULL, OpenKey = NULL, CommandKey = NULL;
	DWORD Dispos;
	const char *Descript = "WCS Project", *AppSig = "WCS.Project";

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, ".proj", 0, KEY_READ, &hkResult) != ERROR_SUCCESS)
		{ // No such extension exists, set us up...
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, AppSig, 0, "", REG_OPTION_NON_VOLATILE,
		 KEY_ALL_ACCESS, NULL, &AppKey, &Dispos) == ERROR_SUCCESS)
			{
			strcat(OpenCommand, " \"%1\"");
			RegSetValueEx(AppKey, NULL, 0, REG_SZ, (unsigned char *)Descript, (DWORD)(strlen(Descript) + 1));
			if (RegCreateKeyEx(AppKey, "shell", 0, "", REG_OPTION_NON_VOLATILE,
			 KEY_ALL_ACCESS, NULL, &ShellKey, &Dispos) == ERROR_SUCCESS)
				{
				if (RegCreateKeyEx(ShellKey, "open", 0, "", REG_OPTION_NON_VOLATILE,
				 KEY_ALL_ACCESS, NULL, &OpenKey, &Dispos) == ERROR_SUCCESS)
					{
					if (RegCreateKeyEx(OpenKey, "command", 0, "", REG_OPTION_NON_VOLATILE,
					 KEY_ALL_ACCESS, NULL, &CommandKey, &Dispos) == ERROR_SUCCESS)
						{
						RegSetValueEx(CommandKey, NULL, 0, REG_SZ, (unsigned char *)OpenCommand, (DWORD)(strlen(OpenCommand) + 1));
						RegCloseKey(CommandKey);
						if (RegCreateKeyEx(HKEY_CLASSES_ROOT, ".proj", 0, "", REG_OPTION_NON_VOLATILE,
						 KEY_ALL_ACCESS, NULL, &ExtKey, &Dispos) == ERROR_SUCCESS)
							{
							RegSetValueEx(ExtKey, NULL, 0, REG_SZ, (unsigned char *)AppSig, (DWORD)(strlen(AppSig) + 1));
							RegCloseKey(ExtKey);
							} // if
						} // if
					RegCloseKey(OpenKey);
					} // if
				RegCloseKey(ShellKey);
				} // if
			RegCloseKey(AppKey);
			} // if
		} // if
	else
		{
		RegCloseKey(hkResult);
		} // else
	} // local scope
#endif // _WIN32

// Set up 'dunno' responses for GL config strings in case they aren't
// set later by GL init

PrivGLVendorString[0] = PrivGLRenderString[0] = PrivGLVersionString[0] = NULL;

} // WCSApp::WCSApp

/*===========================================================================*/

WCSApp::~WCSApp()
{
Terminate = 1;

AppSpecificCleanup();

if (WinSys)
	{
	delete WinSys;
	WinSys = NULL;
	} // if

#ifdef _WIN32
CoUninitialize();
#endif // _WIN32

} // WCSApp::~WCSApp

/*===========================================================================*/

void WCSApp::SetProcPri(signed char NewLevel)
{
#ifdef _WIN32
HANDLE Me;

Me = GetCurrentProcess();

if (NewLevel > 0)
	{
	SetPriorityClass(Me, HIGH_PRIORITY_CLASS);
	} // if
else if (NewLevel < 0)
	{
	SetPriorityClass(Me, IDLE_PRIORITY_CLASS);
	} // else if
else
	{
	SetPriorityClass(Me, NORMAL_PRIORITY_CLASS);
	} // else
#endif // _WIN32

} // WCSApp::SetProcPri

/*===========================================================================*/

unsigned char WCSApp::GetProgDir(char *Dest, unsigned char Len)
{

strncpy(Dest, ProgDir, Len);
return(Len);

} // WCSApp::GetProgDir

/*===========================================================================*/

int WCSApp::CheckToolsDir(char *ProgDir)
{
char TestTools[512];
int Result;

strmfp(TestTools, ProgDir, "Tools");
Result = chdir(TestTools);

chdir(ProgDir);

return (Result == 0);

} // WCSApp::CheckToolsDir

/*===========================================================================*/

void WCSApp::UpdateProjectByTime(void)
{
double Time, Fraction;
long Frame;

Time = MainProj->Interactive->GetActiveTime();
Fraction = Time * MainProj->Interactive->GetFrameRate();
Frame = (long)Fraction;
Fraction = Fraction - Frame;

MainProj->ApplicationSetTime(Time, Frame, Fraction);
AppImages->ApplicationSetTime(Time, Frame, Fraction);
AppEffects->ApplicationSetTime(Time);
AppDB->ApplicationSetTime(Time, Frame, Fraction);

} // WCSApp::UpdateProjectByTime

/*===========================================================================*/

void WCSApp::LoopAndWait(void)
{
NativeMessage GUIEvent;
int CheckAgain;
unsigned int BGHandler, BGCalled, LocalNumHandlers;

while (!Terminate)
	{
	CheckAgain = 0;
	#ifdef _WIN32
	if (WinSys->CheckNoWait(&GUIEvent))
		{
		ProcessOSEvent(&GUIEvent);
		CheckAgain = 1;
		} // if
	#endif // _WIN32
	// Deal with delayed-notifies only when event queue is empty
	if (Delayed && !CheckAgain)
		{
		Delayed->ReGenerateDelayedNotify();
		Delayed = NULL;
		CheckAgain = 1;
		} // if
	if (!CheckAgain) // We want to empty all other events before doing script events
		{
		if (SuperScript->CheckScriptEvent())
			{ // CheckScriptEvent processes the event for us...
			CheckAgain = 1;
			} // if
		} // if
	if (!CheckAgain) // We want to empty all other events before doing netscript events
		{
		if (SuperScript->CheckNetScriptEvent())
			{ // CheckScriptEvent processes the event for us...
			CheckAgain = 1;
			} // if
		} // if
	if (!CheckAgain)
		{ // dispatch background processing
		if (LocalNumHandlers = NumBackGroundHandlers)
			{
			for (BGCalled = 0, BGHandler = 0;
			 BGHandler < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX && BGCalled < LocalNumHandlers;
			 BGHandler++)
				{
				if (BackGroundHandlers[BGHandler])
					{
					if (BackGroundHandlers[BGHandler]->HandleBackgroundCrunch((int)NumBackGroundHandlers))
						{
						// process complete, remove ourselves
						if (BackGroundHandlers[BGHandler] != NULL)
							{
							BackGroundHandlers[BGHandler] = NULL;
							NumBackGroundHandlers--;
							} // if
						} // if
					BGCalled++;
					CheckAgain = 1;
					} // if
				} // for
			} // if
		} // if
	if (!CheckAgain)
		{
		#ifdef _WIN32
		//WinSys->CheckEvent(&GUIEvent);
		(void)WaitMessage();
		#endif // _WIN32
		} // if
	} // while

} // WCSApp::LoopAndWait

/*===========================================================================*/
//                                  Windows Code
/*===========================================================================*/

#ifdef _WIN32

void WCSApp::ProcessOSEvent(MSG *msg)
{
HWND Daddy;
int Handled = 0;

if (msg)
	{
	// See if it's an accelerator...
	if (!Handled && MCP && MCP->KeyAbbrev)
		{
		if (TranslateAccelerator(MCP->FetchWinHandle(), MCP->KeyAbbrev, msg))
			{
			Handled = 1;
			} // if
		} // if
		
	if (!Handled)
		{
		// determine if it's coming from a GUIWindow, if so we have to
		// hand it to IsDialogMessage for processing
		if (Daddy = (HWND)GetWindowLong(msg->hwnd, GWL_HWNDPARENT))
			{
			if (GetWindowLong(Daddy, GWL_USERDATA))
				{
				Handled = IsDialogMessage(Daddy, msg);
				} // if
			} // if
		} // if

	if (!Handled)
		{
		TranslateMessage(msg);
		DispatchMessage(msg);
		} // else
	} // if

} // WCSApp::ProcessOSEvent

/*===========================================================================*/

// This blows up real good if it's a C++ function
long WINAPI WndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
AppEvent ForwardPass;
Fenetre *WideReceiver;
long Handled = 0;

// Yes, this seems awkward
ForwardPass.Type = WCS_APP_EVENTTYPE_MSWIN;
ForwardPass.GUIMessage.hwnd = hwnd;
ForwardPass.GUIMessage.message = message;
ForwardPass.GUIMessage.wParam = wParam;
ForwardPass.GUIMessage.lParam = lParam;

WideReceiver = (Fenetre *)GetWindowLong(hwnd, GWL_USERDATA);
if (WideReceiver && GlobalApp && !GlobalApp->Terminate)
	{
	ForwardPass.Origin = WideReceiver;
	Handled = WideReceiver->DispatchEvent(&ForwardPass, GlobalApp);
	} // if

if (!Handled)
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // if
else
	{
	return(Handled);
	} // else

} // WndProc

/*===========================================================================*/

// Virtually identical to above, but no DefWindowProc call...
long WINAPI DlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
AppEvent ForwardPass;
Fenetre *WideReceiver;

// Yes, this seems awkward
ForwardPass.Type = WCS_APP_EVENTTYPE_MSWIN;
ForwardPass.GUIMessage.hwnd = hwnd;
ForwardPass.GUIMessage.message = message;
ForwardPass.GUIMessage.wParam = wParam;
ForwardPass.GUIMessage.lParam = lParam;

if (message == WM_INITDIALOG)
	{
	return(TRUE);
	} // if

WideReceiver = (Fenetre *)GetWindowLong(hwnd, GWL_USERDATA);

if (WideReceiver)
	{
	Fenetre *TestCast = NULL;
	TestCast = dynamic_cast<Fenetre *>(WideReceiver);
	assert(TestCast != NULL);
	} // if

if ((GlobalApp->SDFK) && (message == WM_MEASUREITEM))
	{
	if (!WideReceiver)
		{
		// This is a hack.
		WideReceiver = GlobalApp->SDFK;
		} // if
	} // if

if (WideReceiver && GlobalApp && !GlobalApp->Terminate)
	{
	ForwardPass.Origin = WideReceiver;
	return(WideReceiver->DispatchEvent(&ForwardPass, GlobalApp));
	} // if

return(0);

} // DlgProc

#endif // _WIN32

/*===========================================================================*/
/*===========================================================================*/

// Background Handler stuff
int WCSApp::AddBGHandler(WCSModule *NewHandler)
{
int HLoop;

// prevent multiple adds
if (IsBGHandler(NewHandler)) return(1);

if (NewHandler && (NumBackGroundHandlers < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX))
	{
	for (HLoop = 0; HLoop < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX; HLoop++)
		{
		if (BackGroundHandlers[HLoop] == NULL)
			{
			BackGroundHandlers[HLoop] = NewHandler;
			NumBackGroundHandlers++;
			return(1);
			} // if
		} // if
	} // if

return(0);

} // WCSApp::AddBGHandler

/*===========================================================================*/

void WCSApp::RemoveBGHandler(WCSModule *RemHandler)
{
int HLoop;

if (RemHandler)
	{
	for (HLoop = 0; HLoop < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX; HLoop++)
		{
		if (BackGroundHandlers[HLoop] == RemHandler)
			{
			BackGroundHandlers[HLoop] = NULL;
			NumBackGroundHandlers--;
			return;
			} // if
		} // if
	} // if

} // WCSApp::RemoveBGHandler

/*===========================================================================*/

void WCSApp::RemoveBGHandlers(WCSModule *RemHandler)
{
int HLoop;

if (RemHandler)
	{
	for (HLoop = 0; HLoop < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX; HLoop++)
		{
		if (BackGroundHandlers[HLoop] == RemHandler)
			{
			BackGroundHandlers[HLoop] = NULL;
			NumBackGroundHandlers--;
			} // if
		} // if
	} // if

} // WCSApp::RemoveBGHandlers

/*===========================================================================*/

int WCSApp::IsBGHandler(WCSModule *CheckHandler)
{
int HLoop;

if (CheckHandler)
	{
	for (HLoop = 0; HLoop < WCS_APPLICATION_BACKGROUND_HANDLERS_MAX; HLoop++)
		{
		if (BackGroundHandlers[HLoop] == CheckHandler)
			{
			return(1);
			} // if
		} // if
	} // if

return(0);

} // WCSApp::AddBGHandler

/*===========================================================================*/

int WCSApp::InquireNumCPUS(void)
{
SYSTEM_INFO SysInfo;

int NumCPUs = 1;

GetSystemInfo(&SysInfo);
#ifdef _M_IX86 // intel
NumCPUs = (int)SysInfo.dwNumberOfProcessors;
#endif // _M_IX86

return(NumCPUs);

} // WCSApp::InquireNumCPUS

/*===========================================================================*/

unsigned long WCSApp::GetSystemProcessID(void)
{
unsigned long MyProcessID = 1;

MyProcessID = GetCurrentProcessId();

return(MyProcessID);

} // WCSApp::GetSystemProcessID

/*===========================================================================*/

char *WCSApp::InquireOSDescString(char *StrBuf, int StrBufLen)
{
const char *Plat;
OSVERSIONINFO osvi;

if ((!StrBuf) || (StrBufLen == 0)) return(NULL);

Plat = " ";

memset(&osvi, 0, sizeof(OSVERSIONINFO));
osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
(void)GetVersionEx(&osvi);
if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) Plat = "95/98 ";
if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) Plat = "NT/2k/XP/etc ";
#ifdef _M_IX86 // intel
sprintf(InquireTempBuf, "X86 Windows %sVersion %d.%d Build: %d.", Plat, osvi.dwBuildNumber, osvi.dwMinorVersion, osvi.dwBuildNumber);
#endif // _M_IX86

strncpy(StrBuf, InquireTempBuf, (unsigned)StrBufLen - 2);
StrBuf[StrBufLen - 1] = NULL;

return(StrBuf);

} // WCSApp::InquireOSDescString

/*===========================================================================*/

char *WCSApp::InquireCPUDescString(char *StrBuf, int StrBufLen)
{
SYSTEM_INFO SysInfo;

if ((!StrBuf) || (StrBufLen == 0)) return(NULL);

GetSystemInfo(&SysInfo);
#ifdef _M_IX86 // intel
sprintf(InquireTempBuf, "X86 CPUs: %d, Family: %d, Model: %d, Step: %d.", SysInfo.dwNumberOfProcessors, SysInfo.wProcessorLevel,
 SysInfo.wProcessorRevision >> 8, SysInfo.wProcessorRevision & 0x00ff);
#endif // _M_IX86

strncpy(StrBuf, InquireTempBuf, (unsigned)StrBufLen - 2);
StrBuf[StrBufLen - 1] = NULL;

return(StrBuf);

} // WCSApp::InquireCPUDescString

/*===========================================================================*/

char *WCSApp::InquireSystemNameString(char *StrBuf, int StrBufLen)
{
DWORD SysNameSize;

if ((!StrBuf) || (StrBufLen == 0)) return(NULL);

StrBuf[0] = 0;
StrBuf[StrBufLen - 1] = NULL;
SysNameSize = (unsigned)StrBufLen;
(void)GetComputerName(StrBuf, &SysNameSize);
return(StrBuf);
} // WCSApp::InquireSystemNameString

/*===========================================================================*/

char *WCSApp::InquireDisplayResString(char *StrBuf, int StrBufLen)
{

if ((!StrBuf) || (StrBufLen == 0)) return(NULL);

sprintf(InquireTempBuf, "Display Resolution: %dx%d, %dbpp", GlobalApp->WinSys->InquireDisplayWidth(), GlobalApp->WinSys->InquireDisplayHeight(), GlobalApp->WinSys->InquireDepth());

strncpy(StrBuf, InquireTempBuf, (unsigned)StrBufLen - 2);
StrBuf[StrBufLen - 1] = NULL;
return(StrBuf);

} // WCSApp::InquireDisplayResString
