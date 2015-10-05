// GUI.cpp
// Nice abstract layer over top of all that gooey GUI stuff
// Written on 23 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "GUI.h"
#include "AppMem.h"
#include "Useful.h"
#include "Random.h"
#include "Fenetre.h"
#include "Palette.h"
#include "WCSVersion.h"
#include "resource.h"
#include "Application.h"
#include "Requester.h"
#include "WCSWidgets.h"
#include "Security.h"
#include "VisualStylesXP.h"
#include "Project.h" // to use QueryConfigOpt...()

// Pick this up from whatever *Version.h/*Version.cpp files you're using
//extern char ExtTitle[];
//extern char ExtTLA[];

// for detecting WinXP Themes (abstracted from uxtheme.h so we don't need the whole XP Platform SDK)
#define UXP_ALLOW_NONCLIENT    0x01
//#define UXP_ALLOW_CONTROLS     0x02 // never used - FW2 10/12/05
//#define UXP_ALLOW_WEBCONTENT   0x04 // never used - FW2 10/12/05

typedef DECLSPEC_IMPORT DWORD (APIENTRY * UXTGETTHEMEAPPPROPERTIES)(void);

#ifdef _WIN32
//extern long WINAPI WndProc(NativeGUIWin hwnd, UINT message, UINT wParam, LONG lParam);
#endif // _WIN32

/*===========================================================================*/

GUIContext::GUIContext(NativeLoadHandle InitInstance, int StartupCode)
{
VirtualDisplayWidth = DisplayWidth = 1024;	// initialize these to keep Lint happy
VirtualDisplayHeight = DisplayHeight = 768;
RootWin = NULL;
SystemFont = NULL;
JoyCenterX[0] = JoyCenterX[1] = JoyCenterX[2] = JoyCenterX[3] = 0;
JoyCenterY[0] = JoyCenterY[1] = JoyCenterY[2] = JoyCenterY[3] = 0;
JoyCenterZ[0] = JoyCenterZ[1] = JoyCenterZ[2] = JoyCenterZ[3] = 0;

JoyMaxX[0] = JoyMaxX[1] = JoyMaxX[2] = JoyMaxX[3] = 1;
JoyMaxY[0] = JoyMaxY[1] = JoyMaxY[2] = JoyMaxY[3] = 1;
JoyMaxZ[0] = JoyMaxZ[1] = JoyMaxZ[2] = JoyMaxZ[3] = 1;

JoyMinX[0] = JoyMinX[1] = JoyMinX[2] = JoyMinX[3] = 1;
JoyMinY[0] = JoyMinY[1] = JoyMinY[2] = JoyMinY[3] = 1;
JoyMinZ[0] = JoyMinZ[1] = JoyMinZ[2] = JoyMinZ[3] = 1;
#ifdef _WIN32
WNDCLASS InquireRegister;
NativeGUIWin DeskWin;
RECT WorkArea;
HDC TempDC;
#endif // _WIN32

ColorControl = NULL;
ErrorStatus = 0;
FenList = NULL;
ModalLevel = 0;
BorderThemesEnabled = 0;
WL = NULL;

RootTitleBackup[0] = NULL;
WInstance = (HINSTANCE)InitInstance;

// WidgetLib has some brushes that RootWin needs, so init it first
if (!(WL = new WidgetLib(this)))
	{
	ErrorStatus = WCS_ERR_GUI_NOWIDGETS;
	} // if
else
	{
	if (WL->InquireInitError())
		{
		ErrorStatus = WCS_ERR_GUI_NOWIDGETS;
		} // if
	} // else

#ifdef _WIN32

if (!GetClassInfo((HINSTANCE)InitInstance, APP_CLASSPREFIX ".DrawingWin", &InquireRegister))
	{
	InquireRegister.style			= CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= (HINSTANCE)InitInstance;
	InquireRegister.hIcon			= NULL; // LoadIcon((HINSTANCE)InitInstance, MAKEINTRESOURCE(IDI_WDRAWINGFEN));
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".DrawingWin";
	RegisterClass(&InquireRegister);
	if (DeskWin = GetDesktopWindow())
		{
		if (TempDC = GetDC(DeskWin))
			{
			if (!SystemFont)
				{
				SystemFont = GetStockObject(ANSI_VAR_FONT);
				} // if
 			DisplayWidth = 0;
 			DisplayHeight = 0;
			//DisplayWidth  = GetDeviceCaps(TempDC, HORZRES);
			//DisplayHeight = GetDeviceCaps(TempDC, VERTRES);
			DisplayWidth  = GetSystemMetrics(SM_CXSCREEN);
			VirtualDisplayWidth  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
 			if (DisplayWidth  < 640) DisplayWidth  = 640;

			DisplayHeight = GetSystemMetrics(SM_CYSCREEN);
			VirtualDisplayHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
 			if (DisplayHeight < 400) DisplayHeight = 400;
			ColorDepth    = GetDeviceCaps(TempDC, BITSPIXEL);
			ReleaseDC(DeskWin, TempDC);
			} // if
		} // if
	} // if
else
	{
	ErrorStatus = WCS_ERR_GUI_WINDOWSETUP;
	} // else

if (!GetClassInfo((HINSTANCE)InitInstance, APP_CLASSPREFIX ".GLDrawingWin", &InquireRegister))
	{
	InquireRegister.style			= CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= (HINSTANCE)InitInstance;
	InquireRegister.hIcon			= LoadIcon((HINSTANCE)InitInstance, MAKEINTRESOURCE(IDI_TB_TM_RENDER));
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".GLDrawingWin";
	RegisterClass(&InquireRegister);
	} // if
else
	{
	ErrorStatus = WCS_ERR_GUI_WINDOWSETUP;
	} // else

if (!GetClassInfo((HINSTANCE)InitInstance, APP_CLASSPREFIX ".GLWin", &InquireRegister))
	{
	InquireRegister.style			= CS_OWNDC | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= (HINSTANCE)InitInstance;
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".GLWin";
	RegisterClass(&InquireRegister);
	} // if

if (!GetClassInfo((HINSTANCE)InitInstance, APP_CLASSPREFIX ".RootWin", &InquireRegister))
	{
	InquireRegister.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= (HINSTANCE)InitInstance;
	InquireRegister.hIcon			= LoadIcon((HINSTANCE)InitInstance, MAKEINTRESOURCE(IDI_WCSNOSHADOW));
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.hbrBackground	= WL->BackgroundGrey;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".RootWin";
	RegisterClass(&InquireRegister);
	} // if

#ifdef _WIN32
// Make our initial window open in the work area (outside of taskbars)
if (SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0))
	{
	// dual monitors under Win2k is reported as 1 double-wide monitor - dumb, dumb, dumb
	if (DisplayWidth > (DisplayHeight * (1920.0 / 1080.0)))	// chose an "exotic" valid mode for comparison
		{
		// Guessing it must be dual monitors under Win2k then...
		DisplayHeight = WorkArea.bottom - WorkArea.top;
		DisplayWidth = (WorkArea.right - WorkArea.left) / 2;
		} // if
	else
		{
		DisplayHeight = WorkArea.bottom - WorkArea.top;
		DisplayWidth = WorkArea.right - WorkArea.left;
		} // else
	} // if

#endif // _WIN32

RootWin = CreateWindow(APP_CLASSPREFIX ".RootWin",
		ExtTitle,
	    //WS_CLIPCHILDREN|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MAXIMIZEBOX,     // window style
		WS_CLIPCHILDREN|WS_OVERLAPPEDWINDOW,     // window style
	    WorkArea.left, WorkArea.top, DisplayWidth, DisplayHeight,
	    NULL,                    // parent/owner window handle
	    NULL,                    // window menu handle
	    (HINSTANCE)WInstance,    // program instance handle
		NULL) ;				     // creation parameters

// Are Windows XP Themes available?
HMODULE UXTHEMEHM;

if (UXTHEMEHM = LoadLibrary("uxtheme.dll"))
	{
	UXTGETTHEMEAPPPROPERTIES uxtGetThemeAppProperties;
	if (uxtGetThemeAppProperties = (UXTGETTHEMEAPPPROPERTIES)GetProcAddress(UXTHEMEHM, "GetThemeAppProperties"))
		{
		DWORD ThemeResult;
		ThemeResult = uxtGetThemeAppProperties();
		if (ThemeResult & UXP_ALLOW_NONCLIENT)
			{
			BorderThemesEnabled = 1;
			} // if
		} // if
	FreeLibrary(UXTHEMEHM); // got what we needed
	UXTHEMEHM = NULL;
	} // if

if (!RootWin)
	{
	ErrorStatus = WCS_ERR_GUI_WINDOWSETUP;
	} // if
else
	{
	ShowWindow(RootWin, StartupCode);
	} // else

#endif // _WIN32

// Copy a pointer to the GUIContext into a variable where all
// Fenetres can conveniently get at it.

Fenetre::SetupLocalWinSys(this);
SetupReqLocalGUI(this);

if (ErrorStatus)
	return;

if ((ColorControl = new PaletteMeister((GUIContext *)this)) == NULL)
	{
	ErrorStatus = WCS_ERR_GUI_NOPALETTE;
	} // if
else
	{
	if (ColorControl->InquireStatus())
		{
		ErrorStatus = WCS_ERR_GUI_NOPALETTE;
		} // if
	} // else


IsVista = false;

OSVERSIONINFOEX OSVIE;
ZeroMemory(&OSVIE, sizeof(OSVERSIONINFOEX));
OSVIE.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
GetVersionEx((LPOSVERSIONINFOA)&OSVIE);

IsVista = OSVIE.dwMajorVersion > 5; // 6 = The operating system is Windows Vista or Windows Server 2008.

#if (NTDDI_VERSION >= NTDDI_VISTA)
DWMHM = NULL;
DynamicDwmGetWindowAttribute = NULL;

// obtain dynamic access to dwmapi's DwmGetWindowAttribute if available
if (DWMHM = LoadLibrary("dwmapi.dll"))
	{
	// dwmapi
	DynamicDwmGetWindowAttribute = (DWMGETWINDOWATTRIBUTE)GetProcAddress(DWMHM, "DwmGetWindowAttribute");
	} // if

#endif // NTDDI_VERSION >= NTDDI_VISTA



} // GUIContext::GUIContext

/*===========================================================================*/

GUIContext::~GUIContext()
{
#ifdef _WIN32
WNDCLASS InquireRegister;
#endif // _WIN32

#if (NTDDI_VERSION >= NTDDI_VISTA)
if (DWMHM)
	{
	FreeLibrary(DWMHM);
	DWMHM = NULL;
	} // if
#endif // NTDDI_VERSION >= NTDDI_VISTA

if (WL)
	{ // Custom Widget library
	delete WL;
	WL = NULL;
	} // if

#ifdef _WIN32
if (RootWin)
	{
	// So we get DefWinProc
	SetWindowLong(RootWin, GWL_USERDATA, NULL);
	DestroyWindow(RootWin);
	RootWin = NULL;
	} // if

if (GetClassInfo((HINSTANCE)WInstance, APP_CLASSPREFIX ".RootWin", &InquireRegister))
	{
	DeleteObject(InquireRegister.hbrBackground);
	InquireRegister.hbrBackground = NULL;
	UnregisterClass(APP_CLASSPREFIX ".RootWin", (HINSTANCE)WInstance);
	} // if
if (GetClassInfo((HINSTANCE)WInstance, APP_CLASSPREFIX ".GLWin", &InquireRegister))
	{
	UnregisterClass(APP_CLASSPREFIX ".GLWin", (HINSTANCE)WInstance);
	} // if
if (GetClassInfo((HINSTANCE)WInstance, APP_CLASSPREFIX ".DrawingWin", &InquireRegister))
	{
	UnregisterClass(APP_CLASSPREFIX ".DrawingWin", (HINSTANCE)WInstance);
	} // if
if (GetClassInfo((HINSTANCE)WInstance, APP_CLASSPREFIX ".GLDrawingWin", &InquireRegister))
	{
	UnregisterClass(APP_CLASSPREFIX ".GLDrawingWin", (HINSTANCE)WInstance);
	} // if

#endif // _WIN32

if (ColorControl)
	{
	delete ColorControl;
	ColorControl = NULL;
	} // if

FenList = NULL;
#ifdef _WIN32
WInstance = NULL;
#endif // _WIN32
SystemFont = NULL;

} // GUIContext::~GUIContext

/*===========================================================================*/

void GUIContext::RegisterFen(Fenetre *Moi)
{

Moi->GCFenChain = FenList;
FenList = Moi;

} // GUIContext::RegisterFen

/*===========================================================================*/

void GUIContext::DeRegFen(Fenetre *Moi)
{
Fenetre *Me;

for (Me = FenList; Me; Me = Me->GCFenChain)
	{
	if (Me == Moi)
		{
		FenList = Moi->GCFenChain;
		break;
		} // if
	else if (Me->GCFenChain == Moi)
		{
		Me->GCFenChain = Me->GCFenChain->GCFenChain;
		break;
		} // if
	} // for

} // GUIContext::DeRegFen

/*===========================================================================*/

void GUIContext::UpdateDocking(int ResizeOnly)
{
Fenetre *Me;

for (Me = FenList; Me; Me = Me->GCFenChain)
	{
	Me->DockUpdate(ResizeOnly);
	} // if

} // GUIContext::UpdateDocking

/*===========================================================================*/

void GUIContext::GoModal(void)
{
Fenetre *Me;

++ModalLevel;
if (ModalLevel > 0)
	{
	for (Me = FenList; Me; Me = Me->GCFenChain)
		{
		Me->UpdateModal();
		} // if
	} // if

} // GUIContext::GoModal

/*===========================================================================*/

void GUIContext::EndModal(void)
{
Fenetre *Me;

if (ModalLevel > 0)
	{
	ModalLevel--;
	} // if
if (ModalLevel < 1)
	{
	for (Me = FenList; Me; Me = Me->GCFenChain)
		{
		Me->UpdateModal();
		} // if
	} // if

} // GUIContext::EndModal

/*===========================================================================*/

unsigned long GUIContext::InquireSignalMask(void)
{

#ifdef _WIN32
return(NULL);
#endif // _WIN32

} // GUIContext::InquireSignalMask

/*===========================================================================*/

#ifdef _WIN32
int GUIContext::CheckEvent(MSG *msg)
{

return(GetMessage(msg, NULL, 0, 0));

} // GUIContext::CheckEvent

/*===========================================================================*/

int GUIContext::CheckNoWait(MSG *msg)
{

return(PeekMessage(msg, NULL, 0, 0, PM_REMOVE));

} // GUIContext::CheckNoWait

#endif // _WIN32

/*===========================================================================*/

void GUIContext::RationalizeWinCoords(short &X, short &Y, short &W, short &H)
{

#ifdef _WIN32
if (W > VirtualDisplayWidth)
	{
	W = (unsigned short)(VirtualDisplayWidth); // - (RootWin->WBorLeft + RootWin->WBorRight + 10));
	} // if
if (H > VirtualDisplayHeight)
	{
	H = (unsigned short)(VirtualDisplayHeight); // - (RootWin->WBorTop + RootWin->WBorBottom +
	 //RootWin->Font->ta_YSize + 10));
	} // if
if ((X + W) < 25)
	{
	X = (25 - W);
	} // if
if (Y < GetSystemMetrics(SM_CYMENU))
	{
	Y = GetSystemMetrics(SM_CYMENU);
	} // if
if (X > ((unsigned short)VirtualDisplayWidth - 100))
	{
	X = (unsigned short)(VirtualDisplayWidth - (W + 100));
	} // if
if (Y > ((unsigned short)VirtualDisplayHeight - 100))
	{
	Y = (unsigned short)(VirtualDisplayHeight - (H + 100));
	} // if
#endif // _WIN32

} // GUIContext::RationalizeWinCoords

/*===========================================================================*/

int GUIContext::InquireRootWidth(void)
{
RECT Inq;

GetClientRect(RootWin, &Inq);
return(Inq.right);

} // 

/*===========================================================================*/

int GUIContext::InquireRootHeight(void)
{
RECT Inq;

GetClientRect(RootWin, &Inq);
return(Inq.bottom - GetSystemMetrics(SM_CYMENU));

} // 

/*===========================================================================*/

int GUIContext::InquireMinimized(void)
{

WINDOWPLACEMENT WP;
WP.length = sizeof(WINDOWPLACEMENT);
GetWindowPlacement(RootWin, &WP);
return(WP.showCmd == SW_SHOWMINIMIZED);

} // 

/*===========================================================================*/

bool GUIContext::InquireRemoteSession(void) const
{

return(GetSystemMetrics( SM_REMOTESESSION ) ? true : false);

} // GUIContext::InquireRemoteSession

/*===========================================================================*/

bool GUIContext::InquireIsVista(void) const
{

return(IsVista);

} // GUIContext::InquireIsVista

/*===========================================================================*/

bool GUIContext::InquireTrueWindowRect(HWND Window, RECT *DestRECT) const
{
#ifndef DWMWA_EXTENDED_FRAME_BOUNDS
#define DWMWA_EXTENDED_FRAME_BOUNDS 9
#endif // !DWMWA_EXTENDED_FRAME_BOUNDS

#if (NTDDI_VERSION >= NTDDI_VISTA)
if(DynamicDwmGetWindowAttribute)
	{
	HRESULT hr = S_OK;
	hr = DynamicDwmGetWindowAttribute(Window, DWMWA_EXTENDED_FRAME_BOUNDS, (PVOID)DestRECT, sizeof(*DestRECT));
	return(SUCCEEDED(hr) ? true : false);
	} // if
else
#endif // NTDDI_VERSION >= NTDDI_VISTA
	{
	return(GetWindowRect(Window, DestRECT) ? true : false);
	} // else

return(false); // just in case we get here
} // GUIContext::InquireTrueWindowRect

/*===========================================================================*/

void GUIContext::DoBeep(void)
{

#ifdef _WIN32
MessageBeep(MB_ICONASTERISK);
#endif // _WIN32

} // GUIContext::DoBeep

/*===========================================================================*/

Fenetre *GUIContext::FindOpenFen(unsigned long Culprit)
{
Fenetre *LookingGlass;

for (LookingGlass = FenList; LookingGlass; LookingGlass = LookingGlass->GCFenChain)
	{
	if (LookingGlass->FenID == Culprit)
		{
		return(LookingGlass);
		} // if
	} // for

return(NULL);

} // GUIContext::FindOpenFen

/*===========================================================================*/

void GUIContext::UpdateRootTitle(char *NewTitle, char *NewUser)
{
char NewProjTitle[256];

#ifdef WCS_BUILD_DEMO
DONGLE_INLINE_CHECK()
#else // !WCS_BUILD_DEMO


if (GlobalApp->Sentinal->CheckDongle())
	{
#endif // !WCS_BUILD_DEMO
	if (NewTitle)
		{
		if (NewUser && NewUser[0])
			{
			sprintf(NewProjTitle, "%s %s (%s) - [%s]", ExtTitle, ExtVersion, NewUser, NewTitle);
			} // if
		else
			{
			sprintf(NewProjTitle, "%s %s - [%s]", ExtTitle, ExtVersion, NewTitle);
			} // else
		} // iif
	else
		{
		if (NewUser && NewUser[0])
			{
			sprintf(NewProjTitle, "%s %s (%s)", ExtTitle, ExtVersion, NewUser);
			} // if
		else
			{
			sprintf(NewProjTitle, "%s %s", ExtTitle, ExtVersion);
			} // else
		} // else
#ifndef WCS_BUILD_DEMO
	} // if
else
	{
	if (NewTitle)
		{
		if (NewUser && NewUser[0])
			{
			sprintf(NewProjTitle, "%s%s Render Engine (%s) - [%s]", ExtTLA, ExtVersion, NewUser, NewTitle);
			} // if
		else
			{
			sprintf(NewProjTitle, "%s%s Render Engine - [%s]", ExtTLA, ExtVersion, NewTitle);
			} // else
		} // iif
	else
		{
		if (NewUser && NewUser[0])
			{
			sprintf(NewProjTitle, "%s%s Render Engine (%s)", ExtTLA, ExtVersion, NewUser);
			} // if
		else
			{
			sprintf(NewProjTitle, "%s%s Render Engine", ExtTLA, ExtVersion);
			} // else
		} // else
	} // else
#endif // !WCS_BUILD_DEMO


#ifdef _WIN32
SetWindowText(RootWin, NewProjTitle);
#endif // _WIN32

} // GUIContext::UpdateRootTitle

/*===========================================================================*/

void GUIContext::DisplayRenderTitle(char *NewTitle)
{

if (!RootTitleBackup[0])
	{
	GetWindowText(RootWin, RootTitleBackup, 255);
	} // if
SetWindowText(RootWin, NewTitle);

} // GUIContext::DisplayRenderTitle

/*===========================================================================*/

void GUIContext::ClearRenderTitle(void)
{

if (RootTitleBackup[0])
	{
	SetWindowText(RootWin, RootTitleBackup);
	RootTitleBackup[0] = 0; // wipe out the backup
	} // if

} // GUIContext::ClearRenderTitle

/*===========================================================================*/

char GUIContext::CheckQualifier(unsigned long KeyCode)
{
#ifdef _WIN32
int VKey = 0;
char Test = 0;

// There is no VK_WIN, meaning "either VK_LWIN or VK_RWIN so we fake it.
if (KeyCode == WCS_GUI_KEYCODE_WINDOWS)
	{
	Test += ((GetAsyncKeyState(VK_LWIN) & 0x8000) ? 1 : 0);
	Test += ((GetAsyncKeyState(VK_RWIN) & 0x8000) ? 1 : 0);
	return(Test ? 1 : 0);
	} // if

switch(KeyCode)
	{
	case WCS_GUI_KEYCODE_LSHIFT: VKey = VK_LSHIFT; break;
	case WCS_GUI_KEYCODE_RSHIFT: VKey = VK_RSHIFT; break;
	case WCS_GUI_KEYCODE_SHIFT: VKey = VK_SHIFT; break;
	case WCS_GUI_KEYCODE_LALT: VKey = VK_LMENU; break;
	case WCS_GUI_KEYCODE_RALT: VKey = VK_RMENU; break;
	case WCS_GUI_KEYCODE_ALT: VKey = VK_MENU; break;
	case WCS_GUI_KEYCODE_CONTROL: VKey = VK_CONTROL; break;
	case WCS_GUI_KEYCODE_CAPSLOCK: VKey = VK_CAPITAL; break;
	case WCS_GUI_KEYCODE_LWINDOWS: VKey = VK_LWIN; break;
	case WCS_GUI_KEYCODE_RWINDOWS: VKey = VK_RWIN; break;
	case WCS_GUI_KEYCODE_WINMENU: VKey = VK_APPS; break;
	case WCS_GUI_KEYCODE_LMOUSE: VKey = VK_LBUTTON; break;
	case WCS_GUI_KEYCODE_MMOUSE: VKey = VK_MBUTTON; break;
	case WCS_GUI_KEYCODE_RMOUSE: VKey = VK_RBUTTON; break;
	case WCS_GUI_KEYCODE_Z: VKey = 'Z'; break;
	default: return(0);
	} // switch

return((GetAsyncKeyState(VKey) & 0x8000) ? 1 : 0);
#endif // _WIN32

} // GUIContext::CheckQualifier

/*===========================================================================*/

int GUIContext::InitInputController(int Controller)
{
#ifdef _WIN32
UINT JoyID = 0;
JOYINFO JoyData;
JOYCAPS JoyCaps;

if (Controller == 1) JoyID = JOYSTICKID1;
if (Controller == 2) JoyID = JOYSTICKID2;
if (joyGetPos(JoyID, &JoyData) == JOYERR_NOERROR)
	{
	JoyCenterX[Controller - 1] = JoyData.wXpos;
	JoyCenterY[Controller - 1] = JoyData.wYpos;
	JoyCenterZ[Controller - 1] = JoyData.wZpos;
	joyGetDevCaps(JoyID, &JoyCaps, sizeof(JoyCaps));
	JoyMinX[Controller - 1] = JoyCaps.wXmin;
	JoyMaxX[Controller - 1] = JoyCaps.wXmax;
	JoyMinY[Controller - 1] = JoyCaps.wYmin;
	JoyMaxY[Controller - 1] = JoyCaps.wYmax;
	JoyMinZ[Controller - 1] = JoyCaps.wZmin;
	JoyMaxZ[Controller - 1] = JoyCaps.wZmax;
	return(1);
	} // if
#endif // _WIN32
return(0);

} // GUIContext::InitInputController

/*===========================================================================*/

float GUIContext::CheckInputControllerX(int Controller)
{
#ifdef _WIN32
UINT JoyID = 0;
JOYINFO JoyData;
signed long JoyCentered;

if (Controller == 1) JoyID = JOYSTICKID1;
if (Controller == 2) JoyID = JOYSTICKID2;
if (joyGetPos(JoyID, &JoyData) == JOYERR_NOERROR)
	{
	JoyCentered = JoyData.wXpos - JoyCenterX[Controller - 1];
	if (JoyCentered > 0)
		return(((float)JoyCentered / (float)(JoyMaxX[Controller - 1] - JoyCenterX[Controller - 1])));
	else
		return(((float)JoyCentered / (float)(JoyMinX[Controller - 1] - JoyCenterX[Controller - 1])));
	} // if
#endif // _WIN32
return(0.0f);

} // GUIContext::CheckInputControllerX

/*===========================================================================*/

float GUIContext::CheckInputControllerY(int Controller)
{
#ifdef _WIN32
UINT JoyID = 0;
JOYINFO JoyData;
signed long JoyCentered;

if (Controller == 1) JoyID = JOYSTICKID1;
if (Controller == 2) JoyID = JOYSTICKID2;
if (joyGetPos(JoyID, &JoyData) == JOYERR_NOERROR)
	{
	JoyCentered = JoyData.wYpos - JoyCenterY[Controller - 1];
	if (JoyCentered > 0)
		return(((float)JoyCentered / (float)(JoyMaxY[Controller - 1] - JoyCenterY[Controller - 1])));
	else
		return(((float)JoyCentered / (float)(JoyMinY[Controller - 1] - JoyCenterY[Controller - 1])));
	} // if
#endif // _WIN32

return(0.0f);

} // GUIContext::CheckInputControllerY

/*===========================================================================*/

float GUIContext::CheckInputControllerZ(int Controller)
{
#ifdef _WIN32
UINT JoyID = 0;
JOYINFO JoyData;
long JoyCentered;

if (Controller == 1) JoyID = JOYSTICKID1;
if (Controller == 2) JoyID = JOYSTICKID2;
if (joyGetPos(JoyID, &JoyData) == JOYERR_NOERROR)
	{
	JoyCentered = JoyData.wZpos - JoyCenterZ[Controller - 1];
	if (JoyCentered > 0)
		return(((float)JoyCentered / (float)(JoyMaxZ[Controller - 1] - JoyCenterZ[Controller - 1])));
	else
		return(((float)JoyCentered / (float)(JoyMinZ[Controller - 1] - JoyCenterZ[Controller - 1])));
	} // if
#endif // _WIN32

return(0.f);

} // GUIContext::CheckInputControllerZ

/*===========================================================================*/

unsigned long GUIContext::CheckInputControllerButtons(int Controller, unsigned long ButtonMask)
{
unsigned long rval = 0;

#ifdef _WIN32
UINT JoyID = 0;
JOYINFO JoyData;
unsigned long ButtonCodes = 0;

if (Controller == 1) JoyID = JOYSTICKID1;
if (Controller == 2) JoyID = JOYSTICKID2;
if (joyGetPos(JoyID, &JoyData) == JOYERR_NOERROR)
	{
	if (JoyData.wButtons & JOY_BUTTON1) ButtonCodes |= 0x01;
	if (JoyData.wButtons & JOY_BUTTON2) ButtonCodes |= 0x02;
	if (JoyData.wButtons & JOY_BUTTON3) ButtonCodes |= 0x04;
	if (JoyData.wButtons & JOY_BUTTON4) ButtonCodes |= 0x08;
	} // if
rval = ButtonCodes & ButtonMask;
#endif // _WIN32

return(rval);

} // GUIContext::CheckInputControllerButtons

/*===========================================================================*/

DWORD GUIContext::InquireWindowStyle(void)
{
bool UseToolwinStyle = false;

if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("wintitle_decoration_small"))
	{
	UseToolwinStyle = true;
	} // if
else if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptFalse("wintitle_decoration_small"))
	{
	UseToolwinStyle = false;
	} // if
else
	{ // assume "auto"
	if (InquireRootHeight() > 768)
		{
		UseToolwinStyle = false;
		} // if
	else
		{
		UseToolwinStyle = true;
		} // else
	} // else

return(UseToolwinStyle ? WS_EX_TOOLWINDOW : NULL);

} // GUIContext::InquireWindowStyle
