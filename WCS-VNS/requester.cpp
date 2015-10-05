// requester.cpp
// Some common GUI requesters
// built from scratch on 11 Jun 1995 by Chris "Xenon" Hanson
// copyright 1995

#include "stdafx.h"

// This helps in inlining BusyWin::CheckAbort
#define WCS_REQUESTER_CPP
class GUIContext;
class WCSApp;
char BusyWinAbort;
extern int NetScriptRender;

GUIContext *ReqLocalGUI;
extern WCSApp *GlobalApp;

#include "Application.h"
#include "Project.h"
#include "GUI.h"
#include "Requester.h"
#include "Useful.h"
#include "WCSWidgets.h"
#include "Fenetre.h"
#include "AppHelp.h"
#include "resource.h"
#include "GUI.h"
#include "Toolbar.h"
#include "WCSVersion.h"
#include "WCSWidgets.h" // for SNADmUnformatTime(), used by GetInputTime()
#include "Script.h" // for network script feedback in BusyWin

#ifdef _WIN32
bool FindWindowOrigin(HWND FenGuy, int &X, int &Y);
// PrevUserMessageWin now in RequesterBasic.cpp
extern HWND PrevUserMessageWin;
UINT WINAPI FileOpenHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
void CopyHackHookData(char *Dest, int DestSize);
char *BrowseForFolder(LPCSTR InitialPath, LPCSTR Title, char *ResultBuffer);
#endif // _WIN32

BusyWin *BusyWin::BusyStack[WCS_REQUESTER_MAX_BUSYSTACK];

#ifdef _WIN32
struct UMCustomParam
	{
	char *Topic, *Message, *ButtonA, *ButtonB, *ButtonC, *ButtonD;
	unsigned char DefButton;
	int ResultButton;
	}; // UMCustomParam

struct UMCustomParam UMCP;
#endif // _WIN32

// These functions are only visible within this file
#ifdef _WIN32
long WINAPI MessageBoxDlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI GetInputDlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI RejectWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif // _WIN32

/*===========================================================================*/

void SetupReqLocalGUI(GUIContext *GUIInit)
{
ReqLocalGUI = GUIInit;
} // SetupReqLocalGUI

//////////////////////////////////////////////////////////////////////
//
//                             Common code
//
//////////////////////////////////////////////////////////////////////

FileReq::FileReq()
{

ClearStuffedNames();

#ifdef _WIN32
NativeReq.lStructSize		= sizeof(OPENFILENAME);
NativeReq.hwndOwner			= ReqLocalGUI->RootWin;
NativeReq.hInstance			= NULL;
NativeReq.lpstrFilter		= NULL;
NativeReq.lpstrCustomFilter	= NULL;
NativeReq.nMaxCustFilter	= 0;
NativeReq.nFilterIndex		= 0;
NativeReq.lpstrFile			= FinalCombo;
NativeReq.nMaxFile			= 16384; // 2k was current max limit in Windows, has this changed?
NativeReq.lpstrFileTitle	= NULL;
NativeReq.nMaxFileTitle		= 0;
NativeReq.lpstrInitialDir	= NULL;
NativeReq.lpstrTitle		= NULL; // redundant
NativeReq.Flags				= OFN_HIDEREADONLY | OFN_EXPLORER;
NativeReq.nFileOffset		= 0;
NativeReq.nFileExtension	= 0;
NativeReq.lpstrDefExt		= NULL;
NativeReq.lCustData			= NULL;
NativeReq.lpfnHook			= NULL;
NativeReq.lpTemplateName	= NULL;
#if (_WIN32_WINNT >= 0x0500)
	NativeReq.pvReserved = NULL;
	NativeReq.dwReserved = 0;
	NativeReq.FlagsEx = 0;
#endif // (_WIN32_WINNT >= 0x0500)
#endif // _WIN32

SetDefPat(WCS_REQUESTER_WILDCARD);
IDefFName[0] = IDefPath[0] = IDefPat[0] = Title[0] = CustPat[0] = 0;
NameIndex = NumFiles = 0;
FinalCombo[0] = 0;

} // FileReq::FileReq

/*===========================================================================*/

FileReq::~FileReq()
{
} // FileReq::~FileReq

/*===========================================================================*/

int FileReq::StuffNames(int NumNames, ...)
{
int DidNames = 0;
char *ThisName;
va_list VarA;

if (NumNames)
	{
	ClearStuffedNames();
	va_start(VarA, NumNames);

	while ((DidNames < WCS_REQUESTER_MAX_STUFFEDNAMES) && (ThisName = va_arg(VarA, char *)))
		{
		StuffedNames[DidNames] = ThisName;
		DidNames++;
		} // while

	va_end(VarA);
	} // if

return(DidNames);

} // FileReq::StuffNames

/*===========================================================================*/

char *FileReq::GetNextStuffedName(void)
{
char *NextStuff = NULL;

if (NameIndex < WCS_REQUESTER_MAX_STUFFEDNAMES)
	{
	if (NextStuff = StuffedNames[NameIndex])
		NameIndex++;
	} // if
return (NextStuff);

} // FileReq::GetNextStuffedName

/*===========================================================================*/

void FileReq::SetDefFile(char *DefFName)
{

if (DefFName)
	{
	strncpy(IDefFName, DefFName, 63);
	IDefFName[63] = NULL;
	} // if
else
	{
	IDefFName[0] = NULL;
	} // else

} // FileReq::SetDefFile

/*===========================================================================*/

void FileReq::SetDefPath(char *DefPath)
{

if (DefPath)
	{
	strncpy(IDefPath, DefPath, 255);
	IDefPath[255] = NULL;
	} // if
else
	{
	IDefPath[0] = NULL;
	} // else

} // FileReq::SetDefPath

/*===========================================================================*/

void FileReq::SetDefPat(char *DefPat)
{
#ifdef _WIN32
int NulLoop;
#endif // _WIN32

if (DefPat)
	{
	#ifdef _WIN32
	sprintf(IDefPat, "Default (%s)@%s@All Files ("WCS_REQUESTER_WILDCARD")@"WCS_REQUESTER_WILDCARD"@@", DefPat, DefPat);

	for (NulLoop = 0; IDefPat[NulLoop]; NulLoop++)
		{
		if (IDefPat[NulLoop] == '@')
			{
			IDefPat[NulLoop] = NULL;
			} // if
		} // if
	#else // !_WIN32
	strncpy(IDefPat, DefPat, 40);
	#endif // !_WIN32
	IDefPat[99] = NULL;
	} // if
else
	{
	IDefPat[0] = NULL;
	} // else

} // FileReq::SetDefPat

/*===========================================================================*/

void FileReq::SetTitle(char *NewTitle)
{

if (NewTitle)
	{
	strncpy(Title, NewTitle, 40);
	Title[40] = NULL;
	} // if
else
	{
	Title[0] = NULL;
	} // else

} // FileReq::SetTitle

/*===========================================================================*/

const char *FileReq::GetFirstName(void)
{
char *TestStuff = NULL;

NameIndex = 0;
if (TestStuff = GetNextStuffedName()) return(TestStuff);

#ifdef _WIN32
FNameList = FNameListStore;
#endif // _WIN32
return(GetNextName());

} // FileReq::GetFirstName

//////////////////////////////////////////////////////////////////////
//
//                              Windows code
//
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32

int FileReq::Request(int Flags)
{
int FileCount;

FNameListStore = FNameList = NULL;

ReqLocalGUI->GoModal();

if (Title[0])
	{
	NativeReq.lpstrTitle = Title;
	} // if
else
	{
	NativeReq.lpstrTitle = NULL;
	} // else

if (IDefFName[0])
	{
	strcpy(FinalCombo, GlobalApp->MainProj->MungPath(IDefFName));
	} // if
else
	{
	FinalCombo[0] = NULL;
	} // else

if (IDefPath[0])
	{
	NativeReq.lpstrInitialDir = strcpy(IDefPath, GlobalApp->MainProj->MungPath(IDefPath));
	} // if
else
	{
	NativeReq.lpstrInitialDir = NULL;
	} // else

if (Flags & WCS_REQUESTER_FILE_DIRONLY)
	{
	if (BrowseForFolder(NativeReq.lpstrInitialDir, NativeReq.lpstrTitle, FinalCombo))
		{
		ReqLocalGUI->EndModal();
		if (!(Flags & WCS_REQUESTER_FILE_NOMUNGE))
			{
			strcpy(FinalCombo, GlobalApp->MainProj->UnMungPath(FinalCombo));
			} // if
		FNameListStore = FNameList = FinalCombo;
		return(NumFiles = 1);
		} // if
	else
		{
		ReqLocalGUI->EndModal();
		return(0); // aborted
		} // else
	} // if

if (IDefPat[0])
	{
	NativeReq.lpstrFilter = IDefPat;
	} // if
else
	{
	NativeReq.lpstrFilter = "All Files ("WCS_REQUESTER_WILDCARD")\0"WCS_REQUESTER_WILDCARD"\0\0";
	} // else

//NativeReq.lpstrCustomFilter = CustPat;
//NativeReq.nMaxCustFilter = 40;	
NumFiles = NameIndex = 0;
NativeReq.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

if (Flags & WCS_REQUESTER_FILE_SAVE)
	{
	NativeReq.Flags |= OFN_NOTESTFILECREATE;
	if (GetSaveFileName(&NativeReq))
		{
		if (!(Flags & WCS_REQUESTER_FILE_NOMUNGE))
			{
			strcpy(FinalCombo, GlobalApp->MainProj->UnMungPath(FinalCombo));
			} // if
		ReqLocalGUI->EndModal();
		return(NumFiles = 1);
		} // if
	} // if
else
	{
	NativeReq.Flags |= (OFN_PATHMUSTEXIST | OFN_ENABLESIZING);
	if (Flags & WCS_REQUESTER_FILE_MULTI)
		{
		NativeReq.Flags |= (OFN_ALLOWMULTISELECT | OFN_ENABLEHOOK);
		NativeReq.lpfnHook = FileOpenHookProc;
		} // if
	else
		{
		NativeReq.Flags |= OFN_FILEMUSTEXIST;
		} // else
	if (GetOpenFileName(&NativeReq))
		{
		if (Flags & WCS_REQUESTER_FILE_MULTI)
			{
			CopyHackHookData(FinalCombo, 16384);
			} // if
		ReqLocalGUI->EndModal();
		if (!(Flags & WCS_REQUESTER_FILE_NOMUNGE))
			{
			if (Flags & WCS_REQUESTER_FILE_MULTI)
				{
				strdoublenullcopy(FinalCombo, GlobalApp->MainProj->UnMungNulledPath(FinalCombo));
				} // if
			else
				{
				strcpy(FinalCombo, GlobalApp->MainProj->UnMungPath(FinalCombo));
				} // else
			} // if
		if (Flags & WCS_REQUESTER_FILE_MULTI)
			{
			// Setup stuff for multifiles
			//if (strchr(FinalCombo, (char)' '))
			if (1)
				{
				// Count files
				for (FileCount = 0, NumFiles = 0; FinalCombo[FileCount] || FinalCombo[FileCount + 1]; FileCount++)
					{
					if (FinalCombo[FileCount] == NULL)
						{
						if (!FNameList)
							{
							FNameListStore = FNameList = &FinalCombo[FileCount + 1];
							FinalCombo[FileCount] = NULL;
							} // if
						NumFiles++;
						} // if
					} // for
				} // if
			else
				{
				NumFiles = 1;
				} // else
			if (NumFiles == 0) NumFiles = 1;
			return(NumFiles);
			} // if
		return(NumFiles = 1);
		} // if
	} // else

ReqLocalGUI->EndModal();
return(0);

} // FileReq::Request

/*===========================================================================*/

static char MultiFileCombine[256];

/*===========================================================================*/

const char *FileReq::GetNextName(void)
{
char *NextName, *TestStuff;

if (TestStuff = GetNextStuffedName())
	return(TestStuff);

if (NameIndex < NumFiles)
	{
	NameIndex++;
	if (NumFiles > 1)
		{
		if (NextName = SkipSpaces(FNameList))
			{
			strmfp(MultiFileCombine, FinalCombo, NextName);
			if (NextName[strlen(NextName) + 1])
				{
				FNameList = &NextName[strlen(NextName) + 1];
				} // if
			return(MultiFileCombine);
			} // if
		else
			{
			// error
			return(NULL);
			} // else
		} // if
	else
		{
		return(FinalCombo);
		} // else
	} // if

return(NULL);

} // FileReq::GetNextName

#endif // _WIN32

/*===========================================================================*/

short GetFileNamePtrn(long Mode, char *ReqName, char *PathName, char *FileName, char *Ptrn, int NameLen)
{
int Flags;
FileReq FR;

Flags = 0;
if (Mode & 0x01)
	Flags |= WCS_REQUESTER_FILE_SAVE;
if (Mode & 0x02)
	Flags |= WCS_REQUESTER_FILE_NOMUNGE;
FR.SetDefPath(PathName);
FR.SetTitle(ReqName);
FR.SetDefPat(Ptrn);
FR.SetDefFile(FileName);
if (FR.Request(Flags))
	{
	PathName[0] = 0;
	FileName[0] = 0;
	BreakFileName((char *)FR.GetNextName(), PathName, 255, FileName, NameLen - 1);
	return (1);
	} // if

return(0);

} // GetFileNamePtrn()

/*===========================================================================*/

void UserMessageDemo(char *Message)
{
char Str[512];

if ((Message == NULL) || (Message[0] == 0))
	{
	sprintf(Str, "This is a demo version. Some capabilities available in the full version have been disabled in this demo.\n\nTo order the full version go to the Help menu and select \"Buy "APP_TLA" Online\" or select the \"Buy "APP_TLA" Online\" button below.");
	} // if
else
	{
	sprintf(Str, "This is a demo version. Some capabilities available in the full version have been disabled in this demo.\n\n%s\n\nTo order the full version go to the Help menu and select \"Buy "APP_TLA" Online\" or select the \"Buy "APP_TLA" Online\" button below.", Message);
	} // else

#ifdef WCS_BUILD_VNS
if (UserMessageCustom("Visual Nature Studio Demo Version", Str, "Buy VNS Online", "OK", NULL, 1))
#else // !VNS
if (UserMessageCustom("World Construction Set Demo Version", Str, "Buy WCS Online", "OK", NULL, 1))
#endif // !VNS
	{
	if (GlobalApp->HelpSys)
		GlobalApp->HelpSys->DoOnlinePurchase();
	} // if

} // UserMessageDemo

/*===========================================================================*/

unsigned char UserMessageNOMEMLargeDEM(char *Topic, unsigned long ReqSize, unsigned long AvailSize, int Width, int Height)
{
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
int ReplyStatus;
char StuffBuff[510];

// if you change this message, check the buffer size above...
#ifdef _WIN32
sprintf(StuffBuff, "Could not allocate %lu bytes of memory.\nWarning: The Renderer detected one or more large DEMs.\nThis may cause excessive memory usage. Consider tiling DEMs.", ReqSize);
#else // !_WIN32
sprintf(StuffBuff, "Could not allocate\n%lu bytes of memory.\nLargest block available\nis %lu bytes.",
 ReqSize, AvailSize);
#endif // !_WIN32

if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if
#ifdef _WIN32
//ReplyStatus = MessageBox(ReqLocalGUI->RootWin, StuffBuff, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
PrevUserMessageWin = GetActiveWindow();
ReplyStatus = MessageBox(ParentWin, StuffBuff, Topic, MB_RETRYCANCEL | MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32

if (ReqLocalGUI) ReqLocalGUI->EndModal();

#ifdef _WIN32
if (ReplyStatus == IDRETRY)  return(1);
else /* (ReplyStatus == IDCANCEL) */ return(0);
#endif // _WIN32

} // UserMessageNOMEMLargeDEM

/*===========================================================================*/

unsigned char UserMessageCustom(char *Topic, char *Message,
 char *ButtonA, char *ButtonB, char *ButtonC, unsigned char DefButton)
{ // Button A is on the left, B is on the right, C (optional,
  // invisible if not used) is in the center
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32

if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if

#ifdef _WIN32

UMCP.Message = Message;

UMCP.Topic = Topic;
UMCP.ButtonA = ButtonA;
UMCP.ButtonB = ButtonB;
UMCP.ButtonC = ButtonC;
UMCP.ButtonD = NULL;
UMCP.DefButton = DefButton;
UMCP.ResultButton = -1;

PrevUserMessageWin = GetActiveWindow();
DialogBoxParam((HINSTANCE)ReqLocalGUI->Instance(), MAKEINTRESOURCE(IDD_MESSAGEBOX), ParentWin, (DLGPROC)MessageBoxDlgProc, (long)&UMCP);
if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32

if (ReqLocalGUI) ReqLocalGUI->EndModal();

#ifdef _WIN32
if (UMCP.ResultButton == IDOK) return(1);
if (UMCP.ResultButton == IDYES) return(2);
#endif // _WIN32

return(0);

} // UserMessageCustom

/*===========================================================================*/

unsigned char UserMessageCustomQuad(char *Topic, char *Message,
 char *ButtonA, char *ButtonB, char *ButtonC, char *ButtonD, unsigned char DefButton)
{ // Button A (Ok = 1) is on the left, B (Cancel = 0) is on the right, C (Yes = 2, optional,
  // invisible if not used) is left of center, D (Other = 3) is right of center
#ifdef _WIN32
HWND ParentWin = NULL;
#endif // _WIN32
unsigned char rVal = 0;

if (ReqLocalGUI)
	{
	ReqLocalGUI->GoModal();
	#ifdef _WIN32
	ParentWin = ReqLocalGUI->RootWin;
	#endif // _WIN32
	} // if

#ifdef _WIN32

UMCP.Message = Message;

UMCP.Topic = Topic;
UMCP.ButtonA = ButtonA;
UMCP.ButtonB = ButtonB;
UMCP.ButtonC = ButtonC;
UMCP.ButtonD = ButtonD;
UMCP.DefButton = DefButton;
UMCP.ResultButton = -1;

PrevUserMessageWin = GetActiveWindow();
DialogBoxParam((HINSTANCE)ReqLocalGUI->Instance(), MAKEINTRESOURCE(IDD_MESSAGEBOXQUAD), ParentWin, (DLGPROC)MessageBoxDlgProc, (long)&UMCP);
if (PrevUserMessageWin) SetActiveWindow(PrevUserMessageWin);
PrevUserMessageWin = NULL;
#endif // _WIN32

if (ReqLocalGUI) ReqLocalGUI->EndModal();

#ifdef _WIN32
if (UMCP.ResultButton == IDOK)
	rVal = 1;
else if (UMCP.ResultButton == IDYES)
	rVal = 2;
else if (UMCP.ResultButton == IDOTHER)
	rVal = 3;
#endif // _WIN32

return(rVal);

} // UserMessageCustomQuad

//////////////////////////////////////////////////////////////////////
//
//                            BusyWin common code
//
//////////////////////////////////////////////////////////////////////

//char PWDate[26], Today[26], ProjDate[26];

// I think I can use an embedded object in the initializer for the base
// class. The base class isn't going to _use_ the pointer until later,
// so it should be safe.

BusyWin::BusyWin(char *Title, unsigned long Steps, unsigned long Section, int TimeEst)
: GUIFenetre(Section, this, Title)
{
int StackSearch;

TitleStore = Title;
MaxSteps = Steps;
LastTickCount = CurPos = 0;
CurSteps = 1;
StartSeconds = 0;

#ifdef _WIN32
PrevActive = NULL;
#endif // _WIN32

if (TimeEst)
	{
	(void)time((time_t *)&StartSeconds);
	} // if

#ifdef _WIN32
//PrevActive = GetActiveWindow();
#endif // _WIN32

BusyWinAbort = 0;

//if (!GlobalApp->MCP->BusyWinNest)
	{
	//GUIFenetre::Open(GlobalApp->MainProj);
	GUIFenetre::GoModal();
	} // if
if (GlobalApp->MCP)
	{
	GlobalApp->MCP->IncBusy();
	GlobalApp->MCP->GoModal();
	GlobalApp->MCP->SetCurrentProgressText(Title);
	GlobalApp->MCP->ShowProgress();
	} // if

for (StackSearch = 0; StackSearch < WCS_REQUESTER_MAX_BUSYSTACK; StackSearch++)
	{
	if (!BusyStack[StackSearch])
		{
		BusyStack[StackSearch] = this;
		break;
		} // if
	} // for

} // BusyWin::BusyWin

/*===========================================================================*/

BusyWin::~BusyWin()
{
int StackSearch;

for (StackSearch = 0; StackSearch < WCS_REQUESTER_MAX_BUSYSTACK; StackSearch++)
	{
	if (BusyStack[StackSearch] == this)
		{
		BusyStack[StackSearch] = NULL;
		break;
		} // if
	} // for

for (StackSearch = WCS_REQUESTER_MAX_BUSYSTACK - 1; StackSearch >= 0; StackSearch--)
	{
	if (BusyStack[StackSearch])
		{
		GlobalApp->MCP->SetCurrentProgressText(BusyStack[StackSearch]->GetReqTitle());
		BusyStack[StackSearch]->Update(BusyStack[StackSearch]->GetCurSteps());
		break;
		} // if
	} // for


#ifdef _WIN32
//if (PrevActive) SetActiveWindow(PrevActive);
#endif // _WIN32

if (GlobalApp->MCP)
	{
	GlobalApp->MCP->DecBusy();
	GlobalApp->MCP->EndModal();
	if (GlobalApp->MCP->LookBusy() > 0)
		{
		GlobalApp->MCP->ShowProgress();
		} // if
	else
		{
		GlobalApp->MCP->HideProgress();
		} // if
	GlobalApp->MCP->ClearProgress();
	} // if

//if (!GlobalApp->MCP->BusyWinNest)
	{
	GUIFenetre::EndModal();
	} // if

if (!GlobalApp->MCP->LookBusy())
	{
	BusyWinAbort = 0;
	} // if

} // BusyWin::~BusyWin

/*===========================================================================*/

NativeGUIWin BusyWin::Construct(void)
{

return(NativeWin);

} // BusyWin::Construct

//#define WCS_REQUESTER_INEFFICIENT

/*===========================================================================*/

void BusyWin::SetNewTitle(char *NewTitle)
{

TitleStore = NewTitle;
GlobalApp->MCP->SetCurrentProgressText(NewTitle);

} // BusyWin::SetNewTitle

/*===========================================================================*/

void BusyWin::Reconfigure(char *NewTitle, unsigned long NewMaxSteps, unsigned long NewCurStep)
{

MaxSteps = NewMaxSteps;
Update(NewCurStep);
TitleStore = NewTitle;
GlobalApp->MCP->SetCurrentProgressText(NewTitle);

} // BusyWin::Reconfigure

/*===========================================================================*/

char NetStatus[100];

#ifdef SPEED_MOD
int FASTCALL BusyWin::Update(unsigned long Step)
#else // SPEED_MOD
int BusyWin::Update(unsigned long Step)
#endif // SPEED_MOD
{
int AbortStatus = 0;
//unsigned long CurTickCount;
/*unsigned long NowSecs, Elapsed, Remain, Projected;
unsigned char ElapHrs, ElapMin, ElapSec,
 RemHrs, RemMin, RemSec; */

//if (this && NativeWin)
if (this)
	{
	if (Step == 0)
		{
		Step = 1; // Prevent / by 0
		} // if

#ifdef WCS_REQUESTER_INEFFICIENT
	if (CurSteps != Step)
		{
		CurSteps = Step;
		} // if
	GlobalApp->MCP->SetCurrentProgressAmount(ProgAmount = (float)(((float)CurSteps / (float)MaxSteps)));
	AbortStatus = CheckAbort();
#else // WCS_REQUESTER_INEFFICIENT
	if (CurSteps != Step)
		{
		CurSteps = Step;
		// timing should limit updates and abort checks to every 63 milliseconds or 15 times per second max
		//CurTickCount = GetTickCount(); // counter in milliseconds, coarse-grained to about 20ms
		//CurTickCount &= 0xffffffc0; // mask off lowest 6 bits as insignificant
		//if (CurTickCount != LastTickCount)
			{
			//LastTickCount = CurTickCount;
			long NewPos;
			NewPos = quickftol(((float)CurSteps / (float)MaxSteps) * 100.0);
			if (GlobalApp->MCP)
				{
				if (NewPos != CurPos) // discourage unnecessary updates of less than 1%
					{
					CurPos = NewPos;
					GlobalApp->MCP->SetCurrentProgressAmount((int)NewPos);
					} // if
				} // if
			if (NetScriptRender && NewPos != CurPos)
				{ // report ongoing progress to net script client
				sprintf(NetStatus, "PROGRESS=\"%s: %d%% Complete\"\n", TitleStore, NewPos);
				GlobalApp->SuperScript->NetSendString(NetStatus);
				} // if
			} // if
		AbortStatus = CheckAbort();
		} // if
#endif // !WCS_REQUESTER_INEFFICIENT
	
	} // if

return(AbortStatus);
} // BusyWin::Update()

/*===========================================================================*/

#ifdef _WIN32
void BusyWin::RepaintFuelGauge(void)
{
/*
HWND Me;
long Steps;
int Percent;

Steps = CurSteps;

Percent = 0;
Me = GetDlgItem(NativeWin, ID_BUSYWIN_PERCENT);
if ((Steps > 0) && (MaxSteps > 0))
	{
	Percent = (long)(100 * ((float)Steps / (float)MaxSteps));
	if ((Percent == 0) && ((100 * ((float)Steps / (float)MaxSteps)) > 0))
		{
		Percent = 1;
		} // if
	} // if
SendMessage(Me, WM_WCSW_CB_NEWVAL, 0, (LPARAM)Percent);
*/

} // BusyWin::RepaintFuelGauge
#endif // _WIN32

/*===========================================================================*/

static int KeepGetting;
static int rCancel;
static char *RejectString;
static char LocalReject[255];
#ifdef _WIN32
WNDPROC OrigEditWndProc;
#endif // _WIN32

/*===========================================================================*/

double GetInputTime(char *Message)
{
double Time = 0.0;
long Frame;
char MessageStr[256];

// find out what time or frame
Time = GlobalApp->MainProj->Interactive->GetActiveTime();
if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
	{
	Frame = (long)(Time * GlobalApp->MainProj->Interactive->GetFrameRate() + .5);
	sprintf(MessageStr, "%df", Frame);
	if (GetInputString(Message, NULL, MessageStr))
		{
		Time = SNADmUnformatTime(MessageStr, NULL);
		} // if
	else
		return (0.0);
	} // if
else
	{
	sprintf(MessageStr, "%fs", Time);
	TrimZeros(MessageStr);
	if (GetInputString(Message, NULL, MessageStr))
		{
		Time = SNADmUnformatTime(MessageStr, NULL);
		} // if
	else
		return (0.0);
	} // else

return(Time);

} // GetInputTime

/*===========================================================================*/

// Make sure String can hold 255 chars
short GetInputString(char *Message, char *Reject, char *String)
{
int Loop, Result = 0;
int Left, Top;
short X, Y, W, H;
#ifdef _WIN32
HWND Main, Edit;
MSG msg;

KeepGetting = 1;
rCancel = 0;

LocalReject[0] = 0;
if (Reject)
	{
	strncpy(LocalReject, Reject, 250);
	LocalReject[251] = 0;
	} // if
ReqLocalGUI->GoModal();
if (Message && String)
	{
	RejectString = LocalReject;
	if (Main = CreateDialog((HINSTANCE)ReqLocalGUI->Instance(), MAKEINTRESOURCE(IDD_GETINPUT), ReqLocalGUI->RootWin, (DLGPROC)GetInputDlgProc))
		{
		SetWindowText(Main, "Input Request");
		SendDlgItemMessage(Main, ID_MESSAGE, WM_SETTEXT, 0, (LPARAM)Message);
		SendDlgItemMessage(Main, IDC_EDIT, WM_SETTEXT, 0, (LPARAM)String);
		SendDlgItemMessage(Main, IDC_EDIT, EM_SETSEL, 0, (LPARAM)-1);
		if (LocalReject[0])
			{
			// Bump letters to uppercase
			for (Loop = 0; LocalReject[Loop]; Loop++)
				{
				LocalReject[Loop] = toupper(LocalReject[Loop]);
				} // for
			// Need to abduct the WndProc for the Edit, to implement reject
			if (Edit = GetDlgItem(Main, IDC_EDIT))
				{
				OrigEditWndProc = (WNDPROC)SetWindowLong(Edit, GWL_WNDPROC, (long)RejectWndProc);
				} // if
			} // if
		// Move to preferred location
		GlobalApp->MainProj->InquireWindowCoords('INPT', X, Y, W, H);
		ReqLocalGUI->RationalizeWinCoords(X, Y, W, H);
		SetWindowPos(Main, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		if (!IsIconic(ReqLocalGUI->RootWin))
			{
			ShowWindow(Main, SW_SHOW);
			} // if
		// Loop and dispatch
		while (KeepGetting)
			{
			ReqLocalGUI->CheckEvent(&msg);
			if (!IsDialogMessage(Main, &msg))
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				} // if
			} // while
		if (rCancel == 0)
			{
			SendDlgItemMessage(Main, IDC_EDIT, WM_GETTEXT, 255, (LPARAM)String);
			Result = 1;
			} // if

		// Get current position of window, and stash
		//GetWindowRect(Main, &Pos);

		FindWindowOrigin(Main, Left, Top);
		GlobalApp->MainProj->SetWindowCoords('INPT', Left, Top, 0, 0);
		//GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_DISABLE);

		DestroyWindow(Main);
		Main = NULL;
		} // if
	} // if
ReqLocalGUI->EndModal();

#endif // _WIN32
return(Result);

} // GetInputString()

/*===========================================================================*/

int GetInputValue(char *Message, AnimDoubleTime *ADT)
{
int Result = 0;
int Left, Top;
short X, Y, W, H;
char String[256];
#ifdef _WIN32
HWND Main;
MSG msg;

KeepGetting = 1;
rCancel = 0;

ReqLocalGUI->GoModal();
if (Message && ADT)
	{
	if (Main = CreateDialog((HINSTANCE)ReqLocalGUI->Instance(), MAKEINTRESOURCE(IDD_GETINPUT), ReqLocalGUI->RootWin, (DLGPROC)GetInputDlgProc))
		{
		SetWindowText(Main, "Value Request");
		SendDlgItemMessage(Main, ID_MESSAGE, WM_SETTEXT, 0, (LPARAM)Message);

		SNADmFormat(String, ADT, ADT->CurValue);
		SendDlgItemMessage(Main, IDC_EDIT, WM_SETTEXT, 0, (LPARAM)String);
		SendDlgItemMessage(Main, IDC_EDIT, EM_SETSEL, 0, (LPARAM)-1);

		// Move to preferred location
		GlobalApp->MainProj->InquireWindowCoords('INPT', X, Y, W, H);
		ReqLocalGUI->RationalizeWinCoords(X, Y, W, H);
		SetWindowPos(Main, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		if (!IsIconic(ReqLocalGUI->RootWin))
			{
			ShowWindow(Main, SW_SHOW);
			} // if
		// Loop and dispatch
		while (KeepGetting)
			{
			ReqLocalGUI->CheckEvent(&msg);
			if (!IsDialogMessage(Main, &msg))
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				} // if
			} // while
		if (rCancel == 0)
			{
			SendDlgItemMessage(Main, IDC_EDIT, WM_GETTEXT, 255, (LPARAM)String);
			ADT->SetValue(SNADmUnformat(String, ADT));
			Result = 1;
			} // if

		// Get current position of window, and stash
		//GetWindowRect(Main, &Pos);

		FindWindowOrigin(Main, Left, Top);
		GlobalApp->MainProj->SetWindowCoords('INPT', Left, Top, 0, 0);
		//GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_DISABLE);

		DestroyWindow(Main);
		Main = NULL;
		} // if
	} // if
ReqLocalGUI->EndModal();

#endif // _WIN32
return(Result);

} // GetInputValue()

/*===========================================================================*/

#ifdef _WIN32
long WINAPI MessageBoxDlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
short ID, Notify;
struct UMCustomParam *MBCP;
long NuStyle;
RECT MyBox;

MBCP = (struct UMCustomParam *)GetWindowLong(hwnd, DWL_USER);

switch (message)
	{
	case WM_INITDIALOG:
		{
		MBCP = (struct UMCustomParam *)lParam;
		SetWindowLong(hwnd, DWL_USER, (long)MBCP);
		// Init
		if (MBCP->ButtonA)
			{
			SetWindowText(GetDlgItem(hwnd, IDOK), MBCP->ButtonA);
			} // if
		if (MBCP->ButtonB)
			{
			SetWindowText(GetDlgItem(hwnd, IDCANCEL), MBCP->ButtonB);
			} // if
		if (MBCP->ButtonC)
			{
			SetWindowText(GetDlgItem(hwnd, IDYES), MBCP->ButtonC);
			ShowWindow(GetDlgItem(hwnd, IDYES), SW_SHOW);
			} // if
		if (MBCP->ButtonD)
			{
			SetWindowText(GetDlgItem(hwnd, IDOTHER), MBCP->ButtonD);
			ShowWindow(GetDlgItem(hwnd, IDOTHER), SW_SHOW);
			} // if
		if (MBCP->Topic)
			{
			SetWindowText(hwnd, MBCP->Topic);
			} // if
		if (MBCP->Message)
			{
			SetWindowText(GetDlgItem(hwnd, ID_MESSAGE), MBCP->Message);
			} // if
		if (MBCP->DefButton != 0)
			{
			NuStyle = GetWindowLong(GetDlgItem(hwnd, IDCANCEL), GWL_STYLE);
			NuStyle &= (~BS_DEFPUSHBUTTON);
			SetWindowLong(GetDlgItem(hwnd, IDCANCEL), GWL_STYLE, NuStyle);
			} // if
		if (MBCP->DefButton == 0)
			{
			SetFocus(GetDlgItem(hwnd, IDCANCEL));
			} // if
		if (MBCP->DefButton == 1)
			{
			NuStyle = GetWindowLong(GetDlgItem(hwnd, IDOK), GWL_STYLE);
			NuStyle |= (BS_DEFPUSHBUTTON);
			SetWindowLong(GetDlgItem(hwnd, IDOK), GWL_STYLE, NuStyle);
			SetFocus(GetDlgItem(hwnd, IDOK));
			} // if
		if (MBCP->DefButton == 2)
			{
			NuStyle = GetWindowLong(GetDlgItem(hwnd, IDYES), GWL_STYLE);
			NuStyle |= (BS_DEFPUSHBUTTON);
			SetWindowLong(GetDlgItem(hwnd, IDYES), GWL_STYLE, NuStyle);
			SetFocus(GetDlgItem(hwnd, IDYES));
			} // if
		if (MBCP->DefButton == 3)
			{
			NuStyle = GetWindowLong(GetDlgItem(hwnd, IDOTHER), GWL_STYLE);
			NuStyle |= (BS_DEFPUSHBUTTON);
			SetWindowLong(GetDlgItem(hwnd, IDOTHER), GWL_STYLE, NuStyle);
			SetFocus(GetDlgItem(hwnd, IDOTHER));
			} // if
		// Center it...
		GetWindowRect(hwnd, &MyBox);
		MyBox.right  -= MyBox.left;
		MyBox.bottom -= MyBox.top;
		MyBox.left = ((ReqLocalGUI->InquireDisplayWidth()  / 2) - (MyBox.right / 2));
		MyBox.top  = ((ReqLocalGUI->InquireDisplayHeight() / 2) - (MyBox.bottom / 2));
		SetWindowPos(hwnd, NULL, MyBox.left, MyBox.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return(FALSE);
		} // WM_INITDIALOG
	case WM_COMMAND:
		{
		ID     = LOWORD(wParam);
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case BN_CLICKED:
				{
				switch (ID)
					{
					case IDOK:
						{
						if (MBCP)
							{
							MBCP->ResultButton = IDOK;
							} // if
						EndDialog(hwnd, 1);
						return(1);
						} // OK
					case IDYES:
						{
						if (MBCP)
							{
							MBCP->ResultButton = IDYES;
							} // if
						EndDialog(hwnd, 1);
						return(1);
						} // YES
					case IDOTHER:
						{
						if (MBCP)
							{
							MBCP->ResultButton = IDOTHER;
							} // if
						EndDialog(hwnd, 1);
						return(1);
						} // OTHER
					case IDCANCEL:
						{
						if (MBCP)
							{
							MBCP->ResultButton = IDCANCEL;
							} // if
						EndDialog(hwnd, 1);
						return(1);
						} // CANCEL
					} // ID
				} // CLICKED
			} // Notify
		break;
		} // COMMAND
	} // message

return(0);
} // MessageBoxDlgProc

/*===========================================================================*/

long WINAPI GetInputDlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
short ID, Notify;

if (message == WM_INITDIALOG)
	{
	return(TRUE);
	} // if

switch (message)
	{
	case WM_COMMAND:

		{
		ID     = LOWORD(wParam);
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case BN_CLICKED:
				{
				switch (ID)
					{
					case IDOK:
						{
						rCancel = 0; KeepGetting = 0;
						return(1);
						} // OK
					case IDCANCEL:
						{
						rCancel = 1; KeepGetting = 0;
						return(1);
						} // CANCEL
					} // ID
				} // CLICKED
			} // Notify
		break;
		} // COMMAND
	case WM_CLOSE:
		{
		rCancel = 1; KeepGetting = 0;
		return(1);
		} // CLOSE
	} // message

return(0);

} // GetInputDlgProc

/*===========================================================================*/

long WINAPI RejectWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
char Key;

switch (message)
	{
	case WM_CHAR:
		{
		Key = (char)wParam;
		Key = toupper(Key);
		if (strchr(RejectString, (char)Key))
			{
			Handled = 1;
			} // if
		break;
		} // PAINT
	} // switch

if (Handled)
	{
	return(0);
	} // if
else
	{
	#ifdef STRICT
	return(CallWindowProc((WNDPROC)OrigEditWndProc, hwnd, message, wParam, lParam));
	#else // !STRICT
	return(CallWindowProc((FARPROC)OrigEditWndProc, hwnd, message, wParam, lParam));
	#endif // !STRICT
	} // else

} // RejectWndProc

/*===========================================================================*/

//lint -save -e648
static char HookHackDataBuf[4096];
UINT WINAPI FileOpenHookProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
LPOFNOTIFY lpon;
    switch (msg)
		{
        case WM_INITDIALOG:
            return TRUE;
        case WM_NOTIFY:
			{
			lpon = (LPOFNOTIFY) lParam;
            if (lpon->hdr.code == CDN_FILEOK)
				{
				GetWindowText(GetDlgItem(lpon->hdr.hwndFrom, 0x047c), HookHackDataBuf, 4000);
	            return FALSE; /* Allow standard processing. */
				} // if
            break;
			} // COMMAND
        default:
            return FALSE;
		} /* end switch */
	return FALSE; 
} // FileOpenHookProc
//lint -restore

/*===========================================================================*/

void CopyHackHookData(char *Dest, int DestSize)
{
int /* SourceSize, */ Process, Write;
char InQuote, *Output, DidNull;

if (Dest && DestSize)
	{
	// Determine if there really is more than one file
	// InQuote counts single NULLs
	for (Write = Process = InQuote = DidNull = 0; Write < 2; Process++)
		{
		if (Dest[Process] == NULL)
			{
			InQuote++;
			if (DidNull) break;
			DidNull = 1;
			} // if
		else
			{
			DidNull = 0;
			} // else
		} // for

	if (InQuote < 3) return; // Not multifile

	// Copy as much data as will fit.
	//SourceSize = strlen(HookHackDataBuf) + 1;
	Output = &Dest[strlen(Dest) + 1]; // We're going to write after the existing NULL
	memset(Output, 0, DestSize - (strlen(Dest) + 1));

	// Break it up
	for (Write = Process = InQuote = 0; Write < DestSize; Process++)
		{
		if ((HookHackDataBuf[Process] == ' ') || (HookHackDataBuf[Process] == NULL))
			{
			if (!InQuote)
				{
				if (HookHackDataBuf[Process] == NULL)
					{
					// Add a doublenull
					Output[Write++] = NULL;
					Output[Write++] = NULL;
					break;
					} // if
				else
					{
					//HookHackDataBuf[Process] = NULL;
					Output[Write++] = NULL;
					} // else
				} // if
			else
				{
				Output[Write++] = HookHackDataBuf[Process];
				} // else
			} // if
		else if (HookHackDataBuf[Process] == '\"')
			{
			// Don't bother copying
			InQuote = !InQuote;
			} // else if
		else
			{
			Output[Write++] = HookHackDataBuf[Process];
			} // else
		} // for
	Dest[DestSize - 1] = Dest[DestSize - 2] = NULL; // Make sure we have a doublenull
	} // if

} // CopyHackHookData

/*===========================================================================*/
/*===========================================================================*/

PopupNotifier::PopupNotifier()
{

Visible = Added = false;
NID.cbSize = sizeof(NID);

// setup for Shell Version 5 and later bahviour (required for notifications)
NID.uVersion = NOTIFYICON_VERSION;

NID.uID = 100;
NID.uFlags = NIF_ICON | NIF_TIP;
NID.uCallbackMessage = WM_REQ_NOTIFY; // from WCSWidgets.h
NID.hIcon = (HICON)LoadImage((HINSTANCE)GlobalApp->WinSys->Instance(), MAKEINTRESOURCE(IDI_WCSNOSHADOW), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED);
//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, "Visual Nature Studio", -1, NID.szTip, 64);
strcpy(NID.szTip, ExtTitle);
NID.dwState = NULL;
NID.dwStateMask = NULL;

LastDisplayedTime = 0;

} // PopupNotifier::PopupNotifier

/*===========================================================================*/

PopupNotifier::~PopupNotifier()
{

if (Added)
	{
	Shell_NotifyIcon(NIM_DELETE, &NID);
	} // if

} // PopupNotifier::~PopupNotifier

/*===========================================================================*/

void PopupNotifier::HandleNotificationEvent(AppEvent *Activity, WCSApp *AppScope)
{

// no matter how hard I try, this doesn't work.
switch (Activity->GUIMessage.lParam)
	{
	case NIN_BALLOONTIMEOUT:
	case NIN_BALLOONUSERCLICK:
		{
		HideNotification();
		break;
		} // hiding
	} // switch

} // PopupNotifier::HandleNotificationEvent

/*===========================================================================*/

bool PopupNotifier::ShowNotification(const char *Title, const char *Message, int Icon, bool CloseButton)
{
bool Success = false;

if (!Added)
	{
	NID.hWnd = GlobalApp->WinSys->RootWin; // where to send notifications, except that they never arrive.
	if (Shell_NotifyIcon (NIM_ADD, &NID))
		{
		Shell_NotifyIcon(NIM_SETVERSION, &NID);
		Added = true;
		} // if
	} // if
if (Added)
	{
	NID.dwState = 0;
	NID.uFlags = NIF_INFO | NIF_STATE | NIF_TIP;
	NID.dwState = 0; // removing NIS_HIDDEN
	NID.dwStateMask = NIS_HIDDEN;
	strcpy(NID.szInfo, Message);
	sprintf(NID.szTip, "%s: %s", ExtTitle, Message);
	//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Message, -1, NID.szInfo, 256);
	NID.uTimeout = 10000; // 10s
	strcpy(NID.szInfoTitle, Title);
	//MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Title, -1, NID.szInfoTitle, 64);
	NID.dwInfoFlags = NIIF_INFO;
	Success = Shell_NotifyIcon(NIM_MODIFY, &NID) ? true : false;
	Visible = true;
	LastDisplayedTime = GetSystemTimeFP();
	} // if

return(Success);

} // PopupNotifier::ShowNotification

/*===========================================================================*/

bool PopupNotifier::HideNotification(void)
{

if (!Added) return(false);
if (!Visible) return(false);
NID.uFlags = NIF_STATE;
NID.dwState = NIS_HIDDEN;
NID.dwStateMask = NIS_HIDDEN;
Shell_NotifyIcon(NIM_MODIFY, &NID);
Visible = false;
LastDisplayedTime = 0;
return(true);

} // PopupNotifier::HideNotification

/*===========================================================================*/

void PopupNotifier::HideIfNeeded(void)
{
double Tdif;

if (!QueryIsVisible()) return;

Tdif = GetSystemTimeFP() - LastDisplayedTime;
if (Tdif >= WCS_REQUESTER_POPUPNOTIFIER_AUTOHIDEDELAY)
	{
	HideNotification();
	} // if

} // PopupNotifier::HideIfNeeded

#endif // _WIN32

/*===========================================================================*/

// from http://www.codeproject.com/KB/files/browsepf.aspx
// now not currently used, but handy to keep around
/*
LPITEMIDLIST ConvertPathToLpItemIdList(const char *pszPath)
{
LPITEMIDLIST  pidl;
LPSHELLFOLDER pDesktopFolder;
OLECHAR       olePath[MAX_PATH];
ULONG         chEaten;
ULONG         dwAttributes;
HRESULT       hr;

if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, olePath, MAX_PATH);
    hr = pDesktopFolder->ParseDisplayName(NULL,NULL,olePath,&chEaten, &pidl,&dwAttributes);
    pDesktopFolder->Release();
	} // if
return pidl;
}
*/
/*===========================================================================*/


// from http://www.codeproject.com/KB/dialog/XBrowseForFolder.aspx

struct FOLDER_PROPS
{
	LPCTSTR lpszTitle;
	LPCTSTR lpszInitialFolder;
	UINT ulFlags;
};

///////////////////////////////////////////////////////////////////////////////
// BrowseCallbackProc - SHBrowseForFolder callback function
static int CALLBACK BrowseCallbackProc(HWND hWnd,		// Window handle to the browse dialog box
									   UINT uMsg,		// Value identifying the event
									   LPARAM lParam,	// Value dependent upon the message 
									   LPARAM lpData)	// Application-defined value that was 
														// specified in the lParam member of the 
														// BROWSEINFO structure
{
switch (uMsg)
	{
	case BFFM_INITIALIZED:		// sent when the browse dialog box has finished initializing. 
		{
		// remove context help button from dialog caption
		LONG lStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		lStyle &= ~DS_CONTEXTHELP;
		::SetWindowLong(hWnd, GWL_STYLE, lStyle);
		lStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
		lStyle &= ~WS_EX_CONTEXTHELP;
		::SetWindowLong(hWnd, GWL_EXSTYLE, lStyle);

		FOLDER_PROPS *fp = (FOLDER_PROPS *) lpData;
		if (fp)
			{
			if (fp->lpszInitialFolder && (fp->lpszInitialFolder[0] != '\0'))
				{
				// set initial directory
				::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)fp->lpszInitialFolder);
				} // if

			if (fp->lpszTitle && (fp->lpszTitle[0] != '\0'))
				{
				// set window caption
				::SetWindowText(hWnd, fp->lpszTitle);
				} // if
			} // if

		//SizeBrowseDialog(hWnd, fp);
		break;
		} // BFFM_INITIALIZED
	} // switch uMsg

return 0;

} // BrowseCallbackProc

/*===========================================================================*/

char *BrowseForFolder(LPCSTR InitialPath, LPCSTR Title, char *ResultBuffer)
{
char *Result = NULL;
FOLDER_PROPS fp;

// based on example from http://vcfaq.mvps.org/sdk/20.htm

BROWSEINFO bi = { 0 };
TCHAR path[MAX_PATH];
bi.lpszTitle = Title;
bi.pidlRoot = NULL;
bi.lpfn = BrowseCallbackProc;
fp.lpszInitialFolder = InitialPath;
fp.lpszTitle = NULL; // not used currently
fp.ulFlags = NULL; // not used by us
bi.lParam = (LPARAM)&fp;
bi.ulFlags = BIF_USENEWUI;
LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
if ( pidl != 0 )
	{
    // get the name of the folder
	SHGetPathFromIDList( pidl, path );
    strcpy(ResultBuffer, path);
    if (ResultBuffer[strlen(ResultBuffer)] != '\\')
		{
		strcat(ResultBuffer, "\\"); // BreakFileName wants to see a trailing \ on dirctory paths or it thinks the leaf directory name is a filename
		} // if
    Result = ResultBuffer;
    // free memory used
    IMalloc * imalloc = 0;
    if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
		{
        imalloc->Free ( pidl );
        imalloc->Release ( );
		} // if
	} // if

return(Result);

} // BrowseForFolder
