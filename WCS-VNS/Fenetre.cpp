// Fenetre.cpp
// Abstract window class
// Created from scratch on 5/16/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Application.h"
#include "Fenetre.h"
#include "GUI.h"
#include "WCSVersion.h"
#include "AppModule.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "WidgetSupport.h" // for some Draw3d stuff
#include "Useful.h"
#include "Log.h"
#include "AppMem.h"
#include "Requester.h"
#include "AppHelp.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "resource.h"
#include "Toolbar.h"
#include "Conservatory.h"
#include "ViewGUI.h" // so Docked window forced resize actions can trigger the View to redraw
#include "CommonComponentEditor.h" // to automatically forward InquireWindowCapabilities to CommonComponentEditor::RespondToInquireWindowCapabilities() when possible 
#include "VisualStylesXP.h"

//#define DEBUGGUIMSG 1
#ifdef DEBUGGUIMSG
static UINT GUImsg;
static char debugMsg[256];
#endif // DEBUGGUIMSG


// Not currently used
//#define USE_LINE_GL_BACKINGSTORE

// older SDKs don't have this yet...
#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE 0x08000000L
#endif // !WS_EX_NOACTIVATE

// New in Vista, but maybe not working...
#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER 92
#endif // !SM_CXPADDEDBORDER

// new to Vista, ref:
// http://www.winehq.org/pipermail/wine-cvs/2007-December/039089.html
#ifndef WM_GETTITLEBARINFOEX
#define WM_GETTITLEBARINFOEX	0x033F
#endif // !WM_GETTITLEBARINFOEX

// http://msdn2.microsoft.com/en-us/library/ms997498.aspx#mshrdwre_topic2
// These values must be defined to handle the WM_MOUSEHWHEEL on 
// Windows 2000 and Windows XP, the first two values will be defined 
// in the Longhorn SDK and the last value is a default value 
// that will not be defined in the Longhorn SDK but will be needed for 
// handling WM_MOUSEHWHEEL messages emulated by IntelliType Pro
// or IntelliPoint (if implemented in future versions).
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                            0x020E
#endif // !WM_MOUSEHWHEEL


#define WCS_FENETRE_DOCK_FUDGE_FACTOR	7
#define WCS_FENETRE_DOCK_FUDGE_W		WCS_FENETRE_DOCK_FUDGE_FACTOR
#define WCS_FENETRE_DOCK_FUDGE_H		WCS_FENETRE_DOCK_FUDGE_FACTOR

#define WCS_FENETRE_BARHEIGHT_BIGBUTTON_THRESHOLD		22
#define WCS_FENETRE_BARHEIGHT_BIGBUTTON_WIDTH			23
#define WCS_FENETRE_BARHEIGHT_BIGBUTTON_HEIGHT			23
#define WCS_FENETRE_BARHEIGHT_BIGBUTTON_XPAD			1
#define WCS_FENETRE_BARHEIGHT_BIGBUTTON_XADVANCE		(WCS_FENETRE_BARHEIGHT_BIGBUTTON_WIDTH + WCS_FENETRE_BARHEIGHT_BIGBUTTON_XPAD)

#define WCS_FENETRE_BARHEIGHT_SMBUTTON_WIDTH			16
#define WCS_FENETRE_BARHEIGHT_SMBUTTON_HEIGHT			16
#define WCS_FENETRE_BARHEIGHT_SMBUTTON_XPAD				2
#define WCS_FENETRE_BARHEIGHT_SMBUTTON_XADVANCE			(WCS_FENETRE_BARHEIGHT_SMBUTTON_WIDTH + WCS_FENETRE_BARHEIGHT_SMBUTTON_XPAD)

GUIContext *LocalWinSysCopy;
HWND LocalRootWin;

HGLRC GLDrawingFenetre::CurrentGDLF_GLRCs[WCS_FENETRE_MAX_SHARED_GLRCS];

static NativeBitmap CreateDIB24(HDC ExistingDC, int Width, int Height);
BOOL CALLBACK ShuffleChildren(HWND hwnd,LPARAM lParam);	
static HDWP ShuffleData;

// Hacks
int GLInhibitReadPixels, GLReadPixelsFront, GLEXTBGRA, GLClampEdgeAvailable;

// This is a scratch buffer for blitting.
// Thumbnails bigger than 2048 wide are not supported. Duh.
unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];

// This is messy
#ifdef _WIN32
extern WNDPROC EditWndProc, ButtonWndProc, ListWndProc, ComboWndProc, ToolbarWndProc, ListViewWndProc;
extern WNDPROC RealButtonWndProc;
long WINAPI ToolButtonWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI FloatIntWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SlightlyModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedTreeWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedListWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI GridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);

long WINAPI ColorBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI DiskDirWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SmartCheckWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SmartRadioWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
#endif // _WIN32

bool FindWindowOrigin(NativeDrwWin FenGuy, int &X, int &Y);

/*===========================================================================*/
//
//                            Common code section
//
/*===========================================================================*/

//lint -save -e1541
Fenetre::Fenetre(unsigned long WinID, WCSModule *Module,
 char *Title)
{
int Loop;

RealFenType = 0;
memset(VPID, 0, 4);
WinManagerFlags = 0;
ForceMoveSize = 0;
PopupMenu = NULL;
PopupX = PopupY = -1;
NominalWidth = NominalHeight = 0;
WinCaptionButtonBar = NULL;

for (Loop = 0; Loop < WCS_FENETRE_MAX_CUSTOM_LONG; Loop++)
	{
	CustomDataLong[Loop] = 0;
	} // for
for (Loop = 0; Loop < WCS_FENETRE_MAX_CUSTOM_VOID; Loop++)
	{
	CustomDataVoid[Loop] = NULL;
	} // for

if (Module && WinID)
	{
	FenID = WinID;
	strncpy(FenTitle, Title, 255); FenTitle[255] = NULL;
	Owner = Module;
	GCFenChain = NULL;
	CurrentPointer = PointerStore = ModalInhibit = 0;
	RangeBase = RangeSizeCheck = 0;
	OriX = OriY = SizeX = SizeY = 0;
	#ifdef _WIN32
	NewDC = NULL;
	_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_NONE;
	#endif // _WIN32
	// Clear out custom color-pot swatches
	for (Loop = 0; Loop < WCS_PALETTE_COLOR_SIZE; Loop++)
		{
		Swatches[Loop].peRed = Swatches[Loop].peGreen = Swatches[Loop].peBlue = 0;
		#ifdef _WIN32
		Swatches[Loop].peFlags = PC_RESERVED;
		#endif // _WIN32
		} // for
	} // if

CustomCloseButton = false;
#if (NTDDI_VERSION >= NTDDI_VISTA)
// for Vista window size-correction variables
SizeCorrectX = SizeCorrectY = 0;
#endif // NTDDI_VERSION >= NTDDI_VISTA

} // Fenetre::Fenetre
//lint -restore

/*===========================================================================*/

Fenetre::~Fenetre()
{
if (PopupMenu)
	DestroyMenu(PopupMenu);
PopupMenu = NULL;

if (WinCaptionButtonBar)
	DestroyWindow(WinCaptionButtonBar);
WinCaptionButtonBar = NULL;

} // Fenetre::~Fenetre

/*===========================================================================*/

void Fenetre::ResyncWinMan(unsigned long ChangedFlags)
{
HWND TheWin;
RECT SizeAdjust;
long WinStyle, WinExStyle;

if (TheWin = (HWND)GetNativeWin())
	{
	WinStyle = GetWindowLong(TheWin, GWL_STYLE);
	WinExStyle = GetWindowLong(TheWin, GWL_EXSTYLE);
	if (LocalWinSys()->InquireWindowStyle() || TestWinManFlags(WCS_FENETRE_WINMAN_SMTITLE))
		{
		if (!(WinExStyle & WS_EX_TOOLWINDOW))
			{ // only change if needed
			WinExStyle |= WS_EX_TOOLWINDOW;
			if (!TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))
				{
				// get current window size
				GetWindowRect(TheWin, &SizeAdjust);
				SizeAdjust.right -= SizeAdjust.left;
				SizeAdjust.bottom -= SizeAdjust.top;
				// Adjust smaller by difference in titlebar height
				SizeAdjust.bottom -= (GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYSMCAPTION));
				SetWindowPos(TheWin, NULL, 0, 0, SizeAdjust.right, SizeAdjust.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOREDRAW);
				} // if
			} // if
		} // if
	else
		{
		if (WinExStyle & WS_EX_TOOLWINDOW)
			{ // only change if needed
			WinExStyle &= ~(WS_EX_TOOLWINDOW);
			if (!TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))
				{
				// get current window size
				GetWindowRect(TheWin, &SizeAdjust);
				SizeAdjust.right -= SizeAdjust.left;
				SizeAdjust.bottom -= SizeAdjust.top;
				// Adjust bigger by difference in titlebar height
				SizeAdjust.bottom += (GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYSMCAPTION));
				SetWindowPos(TheWin, NULL, 0, 0, SizeAdjust.right, SizeAdjust.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOREDRAW);
				} // if
			} // if
		} // else	

	// WCS_FENETRE_WINMAN_ISHIDDEN is handled at bottom
	if (ChangedFlags & WCS_FENETRE_WINMAN_ISDOCKED)
		{ // changed docking
		if (TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))
			{
			if (!(LocalWinSys()->InquireBorderThemesEnabled()))
				{
				WinExStyle |= WS_EX_STATICEDGE; // THIS IS IT! THIS FREAKS WINXP VISUAL THEMES!
				} // if
			} // if
		else
			{
			WinExStyle &= ~WS_EX_STATICEDGE;
			} // else
		} // if

//	if (ChangedFlags & WCS_FENETRE_WINMAN_ONTOP)
		{ // ontop
		if (TestWinManFlags(WCS_FENETRE_WINMAN_ONTOP))
			{
			SetWindowPos(TheWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			WinExStyle |= WS_EX_TOPMOST;
			} // if
		else
			{
			SetWindowPos(TheWin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			WinExStyle &= ~WS_EX_TOPMOST;
			} // else
		} // if
	SetWindowLong(TheWin, GWL_STYLE, WinStyle);
	SetWindowLong(TheWin, GWL_EXSTYLE, WinExStyle);
	ShowWindow(TheWin, SW_HIDE);
	if (!TestWinManFlags(WCS_FENETRE_WINMAN_ISHIDDEN)) ShowWindow(TheWin, SW_SHOW);
	if (TestWinManFlags(WCS_FENETRE_WINMAN_ONTOP))
		{
		SetWindowPos(TheWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		} // if
	else
		{
		SetWindowPos(TheWin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		} // else
	} // if
} // Fenetre::ResyncWinMan

/*===========================================================================*/

void Fenetre::MoveFen(short X, short Y)
{
HWND TheWin;
POINT XLate;
unsigned long WStyle;

if (TheWin = (HWND)GetNativeWin())
	{
	ForceMoveSize = 1;
	WStyle = GetWindowLong(TheWin, GWL_STYLE);
	if (WStyle & WS_POPUP)
		{
		XLate.x = X;
		XLate.y = Y;
		ClientToScreen(LocalWinSys()->RootWin, &XLate);
		X = (short)XLate.x;
		Y = (short)XLate.y;
		} // if
	SetWindowPos(TheWin, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ForceMoveSize = 0;
	} // if

} // Fenetre::MoveFen

/*===========================================================================*/

void Fenetre::SizeFen(unsigned short Width, unsigned short Height)
{
HWND TheWin;

if (TheWin = (HWND)GetNativeWin())
	{
	ForceMoveSize = 1;
	SetWindowPos(TheWin, NULL, 0, 0, Width, Height, SWP_NOMOVE | SWP_NOZORDER);
	ForceMoveSize = 0;
	} // if

} // Fenetre::SizeFen

/*===========================================================================*/

void Fenetre::MoveAndSizeFen(short X, short Y, unsigned short Width, unsigned short Height)
{
HWND TheWin;
POINT XLate;
unsigned long WStyle;

if (TheWin = (HWND)GetNativeWin())
	{
	ForceMoveSize = 1;
	WStyle = GetWindowLong(TheWin, GWL_STYLE);
	if (WStyle & WS_POPUP)
		{
		XLate.x = X;
		XLate.y = Y;
		ClientToScreen(LocalWinSys()->RootWin, &XLate);
		X = (short)XLate.x;
		Y = (short)XLate.y;
		} // if
	SetWindowPos(TheWin, NULL, X, Y, Width, Height, SWP_NOZORDER);
	ForceMoveSize = 0;
	} // if

} // Fenetre::MoveAndSizeFen

/*===========================================================================*/

// No one but the GUIContext should call this.
void Fenetre::SetupLocalWinSys(GUIContext *InitWinSys)
{

LocalWinSysCopy = InitWinSys;

} // Fenetre::SetupLocalWinSys

/*===========================================================================*/

GUIContext *Fenetre::LocalWinSys(void)
{

return(LocalWinSysCopy);

} // Fenetre::LocalWinSys

/*===========================================================================*/

void Fenetre::GainFocus(void)
{

LocalWinSys()->ColorControl->Instantiate(this);

} // Fenetre::GainFocus

/*===========================================================================*/

void Fenetre::LoseFocus(void)
{

} // Fenetre::LoseFocus

/*===========================================================================*/

void Fenetre::SetFocusToMe(void)
{

SetFocus((HWND)GetNativeWin());

} // Fenetre::SetFocusToMe

/*===========================================================================*/

void Fenetre::PositionWinCaptionBar(int NewHostWinX, int NewHostWinY, HDWP DeferPos)
{

if (WinCaptionButtonBar)
	{
	int CaptionButtonWinWidth;
	int x, y, RightOffset, BarHeight, ButtonHeight = 21, XFrame, Adjust = 0, VOffset = 0;
	int YFrame, Pad, YEdge;
	RECT WinRect;
	long WinStyle, WinExStyle;
	bool BigButtons, ToolWindow = false;

#ifdef USE_VSSYM32
#if (NTDDI_VERSION >= NTDDI_VISTA)
	// for Vista support
	TITLEBARINFOEX ExtendedTitlebarInfo;
	ExtendedTitlebarInfo.cbSize = sizeof(TITLEBARINFOEX);
	bool UseExtended = false;

	if (LocalWinSys()->InquireIsVista())
		{
		SendMessage((HWND)GetNativeWin(), WM_GETTITLEBARINFOEX, 0, (LPARAM)&ExtendedTitlebarInfo);
		UseExtended = true;
		} // if
#endif // NTDDI_VERSION >= NTDDI_VISTA
#endif // USE_VSSYM32
	WinStyle = GetWindowLong((HWND)GetNativeWin(), GWL_STYLE);
	WinExStyle = GetWindowLong((HWND)GetNativeWin(), GWL_EXSTYLE);
	XFrame = GetSystemMetrics(SM_CXFRAME);
	if (WinStyle & WS_THICKFRAME)
		{
		YFrame = GetSystemMetrics(SM_CYFRAME);
		if (Adjust == 0)
			Adjust = -1;
		} // if
	else
		{
		YFrame = GetSystemMetrics(SM_CYFIXEDFRAME);
		} // else
	Pad = GetSystemMetrics(SM_CXPADDEDBORDER);
	YEdge = GetSystemMetrics(SM_CYEDGE);

	if (WinExStyle & WS_EX_TOOLWINDOW)
		{
		int XSizeSm;
		XSizeSm = GetSystemMetrics(SM_CXSMSIZE);
		BigButtons = false;
		RightOffset = XFrame + GetSystemMetrics(SM_CXPADDEDBORDER) + XSizeSm;
		ToolWindow = true;
		BarHeight = GetSystemMetrics(SM_CYSMCAPTION);
		Adjust = -2;

		VOffset = YFrame + YEdge + Adjust;
		CustomCloseButton = false;
		} // if
	else
		{
		int XSize;
		XSize = GetSystemMetrics(SM_CXSIZE);
		BigButtons = true;
		if (CustomCloseButton) // omit close button space
			{
			RightOffset = XFrame + GetSystemMetrics(SM_CXPADDEDBORDER);
			} // if
		else
			{
			RightOffset = XFrame + GetSystemMetrics(SM_CXPADDEDBORDER) + XSize;
			} // if
 		BarHeight = GetSystemMetrics(SM_CYCAPTION);
		VOffset = (2 * YFrame - YEdge) + Adjust;
		} // else
	if (BarHeight > WCS_FENETRE_BARHEIGHT_BIGBUTTON_THRESHOLD)
		{
		ButtonHeight = WCS_FENETRE_BARHEIGHT_BIGBUTTON_HEIGHT;
		} // if
	else
		{
		ButtonHeight = WCS_FENETRE_BARHEIGHT_SMBUTTON_HEIGHT;
		} // else
	
	GetClientRect(WinCaptionButtonBar, &WinRect);
	CaptionButtonWinWidth = WinRect.right;
	
	GetWindowRect((HWND)GetNativeWin(), &WinRect); // to get width, if nothing else
	if (NewHostWinX == LONG_MIN && NewHostWinY == LONG_MIN)
		{ // initial positioning, not during a move
		NewHostWinX = WinRect.left;
		NewHostWinY = WinRect.top;
		} // if	
#ifdef USE_VSSYM32
#if (NTDDI_VERSION >= NTDDI_VISTA)
	// for Vista support
	if (UseExtended)
		{
		VOffset = 0; // don't muck with the positioning we calculate here
		x = ExtendedTitlebarInfo.rgrect[5].left - (CaptionButtonWinWidth + XFrame);
		// Vista Basic or Toolwindows should track top of close button
		int CXSize, CXEdge, MetricCloseButtonWidth;
		CXSize = GetSystemMetrics(SM_CXSIZE);
		CXEdge = GetSystemMetrics(SM_CXEDGE);
		MetricCloseButtonWidth = CXSize - CXEdge;
		if (ToolWindow || (MetricCloseButtonWidth > (ExtendedTitlebarInfo.rgrect[5].right - ExtendedTitlebarInfo.rgrect[5].left))) // compare close button size to meterics to detect full Aero
			{
			// center the middle of our close button on the middle of the Windows close button
			y = ((ExtendedTitlebarInfo.rgrect[5].top + ExtendedTitlebarInfo.rgrect[5].bottom) / 2) - (ButtonHeight / 2);
			} // if
		else
			{ // Full Vista Aero should choose its own y pos, since close button is stuck to top of window
			y = ExtendedTitlebarInfo.rgrect[5].top + GetSystemMetrics(SM_CYFRAME); // SM_CYFRAME is arbitrary, but seems about right
			} // else
		} // if
	else
#endif // (NTDDI_VERSION >= NTDDI_VISTA)
#endif // USE_VSSYM32
		{
		x = NewHostWinX + ((WinRect.right - WinRect.left) - (RightOffset + CaptionButtonWinWidth));
		//y = NewHostWinY + 2 + ((BarHeight - ButtonHeight) / 2);
		// ref http://shellrevealed.com/photos/blog_images/images/4538/original.aspx
		// and http://shellrevealed.com/blogs/shellblog/archive/2006/10/12/Frequently-asked-questions-about-the-Aero-Basic-window-frame.aspx
		y = NewHostWinY + VOffset;
		} // else

	if (x < NewHostWinX + 10)
		{ // hide the button bar
		if (DeferPos)
			{
			DeferWindowPos(DeferPos, WinCaptionButtonBar, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
			} // if
		else
			{
			SetWindowPos(WinCaptionButtonBar, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
			} // else
		} // if
	else
		{
		if (DeferPos)
			{
			DeferWindowPos(DeferPos, WinCaptionButtonBar, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			} // if
		else
			{
			SetWindowPos(WinCaptionButtonBar, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			} // else
		} // else
	
	InvalidateRect(WinCaptionButtonBar, NULL, false);
	} // if

} // Fenetre::PositionWinCaptionBar

/*===========================================================================*/

bool Fenetre::CreateWinCaptionBar(void)
{
bool IsAView = false, IsAComponentEditor = false,
 CanDock = false, CanLoad = false, CanSave = false, CanNext = false, CanPrev = false, CanUndo = false, CanEmbed = false, CanAdv = false;
int ButtonCount = 0, ButtonPos = 0, BarHeight;
bool BigButtons = false;
int ButtonWidth, ButtonHeight, ButtonAdvance;
long WinExStyle;
HWND Buttons[20];

long Style = GetWindowLong((HWND)GetNativeWin(), GWL_STYLE);

IsAView = (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW) ? true : false);
CanDock = (TestWinManFlags(WCS_FENETRE_WINMAN_NODOCK) ? false : true);
CanLoad = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANLOAD);
CanSave = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANSAVE);
CanNext = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANNEXT);
CanPrev = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANPREV);
CanUndo     = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANUNDO);
CanEmbed    = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANEMBED);
CanAdv      = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANSHOWADV);

WinExStyle = GetWindowLong((HWND)GetNativeWin(), GWL_EXSTYLE);
if (WinExStyle & WS_EX_TOOLWINDOW)
	{
	BarHeight = GetSystemMetrics(SM_CYSMCAPTION);
	CustomCloseButton = false; // only on full-size windows that get SysMenu
	} // if
else
	{
	BarHeight = GetSystemMetrics(SM_CYCAPTION);
	} // else

if (BarHeight > WCS_FENETRE_BARHEIGHT_BIGBUTTON_THRESHOLD)
	{
	BigButtons = true;
	} // if
else
	{
	BigButtons = false;
	} // else

if (BigButtons)
	{
	ButtonWidth   = WCS_FENETRE_BARHEIGHT_BIGBUTTON_WIDTH;
	ButtonHeight  = WCS_FENETRE_BARHEIGHT_BIGBUTTON_HEIGHT;
	ButtonAdvance = WCS_FENETRE_BARHEIGHT_BIGBUTTON_XADVANCE;
	} // if
else
	{
	ButtonWidth   = WCS_FENETRE_BARHEIGHT_SMBUTTON_WIDTH;
	ButtonHeight  = WCS_FENETRE_BARHEIGHT_SMBUTTON_HEIGHT;
	ButtonAdvance = WCS_FENETRE_BARHEIGHT_SMBUTTON_XADVANCE;
	} // else

if (!TestWinManFlags(WCS_FENETRE_WINMAN_NOPOPUP) || CanUndo || CustomCloseButton) // let some windows express undo if they ask to
	{
	unsigned long WStyle = WS_POPUP | WS_VISIBLE;
	if (WinCaptionButtonBar = CreateWindowEx(WS_EX_NOACTIVATE|WS_EX_LAYERED, APP_CLASSPREFIX ".WinCaptionButtonBar", "", WStyle,
	 0, 0, 34, ButtonHeight, (HWND)GetNativeWin(), NULL, LocalWinSys()->Instance(), NULL))
		{
		char *ButtonCaption = NULL;
		
		unsigned long BStyle = WS_CHILD | WCSW_TB_STYLE_NOFOCUS | WCSW_TB_STYLE_XPLOOK | WCSW_TB_STYLE_CAPTIONBUTTON;
		if (CanLoad)
			{
			// Load
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANLOAD)))
				{
				ButtonCaption = "Load Component (+SHIFT for File Requester)";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_LOAD, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_GALLERY);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanSave)
			{
			// Save
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANSAVE)))
				{
				ButtonCaption = "Save Component";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_SAVE, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_SAVE);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanEmbed)
			{
			// Embed
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANEMBED)))
				{
				ButtonCaption = "Embed Component";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_EMBED, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_EMBED);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanPrev)
			{
			// Prev
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANPREV)))
				{
				ButtonCaption = "Edit Previous Component of Same Type";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_PREV, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_PREV);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanNext)
			{
			// Next
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANNEXT)))
				{
				ButtonCaption = "Edit Next Component of Same Type";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_NEXT, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_NEXT);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanUndo)
			{
			// Undo
			if (!(ButtonCaption = InquireWindowCapabilityCaption(WCS_FENETRE_WINCAP_CANUNDO)))
				{
				ButtonCaption = "Undo Changes";
				} // if
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", ButtonCaption, BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_UNDO, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDC_WINUNDO);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (IsAView)
			{
			// Move
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Move View", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_MOVEVIEW, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_CAP_MOVEVIEW);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Rotate
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Rotate View", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ROTVIEW, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_CAP_ROTVIEW);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Zoom
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Zoom View", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ZOOMVIEW, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_CAP_ZOOMVIEW);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Auto-Zoom/Floating
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Default View Position (Set View's Camera Floating)", BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ZOOMAUTO, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_ZOOM);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Zoom Box
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Zoom View with Box", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ZOOMBOX, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_ZOOMBOX);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Zoom Way Out
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Zoom View Way Out", BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ZOOMWAYOUT, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_ZOOMWAYOUT);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Undo/Restore Zoom
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Restore View's Last Position", BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ZOOMUNDO, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_ZOOMRESTORE);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Render
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Render [F9] / Dismiss [F8] Preview ", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_RENDER, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_RENDERVIEW);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			// Render Constraint
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Toggle Constrained Render [F5]", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_RENDERCONSTRAINT, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, IDI_TB_LTDREGION);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanAdv)
			{
			// Advanced Features
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Show Advanced Features", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_ADV, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, ID_WINMENU_SHOWADVANCED);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if
		if (CanDock)
			{
			// Dock
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Toggle Dock", BStyle | WCSW_TB_STYLE_TOG,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, IDI_CAP_DOCK, NULL);
				SetWindowLong(Buttons[ButtonCount], GWL_ID, ID_WINMENU_DOCK);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if

		if (CustomCloseButton) // supply our own Close button
			{
			// Close
			ButtonPos += 5; // separate Close from other buttons
			if (Buttons[ButtonCount] = CreateWindowEx(NULL, APP_CLASSPREFIX ".ToolButton", "Close", BStyle,
			 ButtonPos, 0, ButtonWidth, ButtonHeight, WinCaptionButtonBar, NULL, LocalWinSys()->Instance(), NULL))
				{
				ConfigureTB(Buttons[ButtonCount], NULL, NULL, NULL); // middle NULL indicates to use built-in XP Close image
				SetWindowLong(Buttons[ButtonCount], GWL_ID, ID_WINMENU_CLOSE);
				SendMessage(Buttons[ButtonCount], WM_SETFONT, (WPARAM)NULL, 1); // necessary to trigger Tooltip adding in SETFONT handler
				ButtonCount++;
				ButtonPos += ButtonAdvance;
				} // if
			} // if

		SetWindowPos(WinCaptionButtonBar, HWND_TOP, 0, 0, ButtonPos, ButtonHeight, SWP_NOMOVE);
		SetLayeredWindowAttributes(WinCaptionButtonBar, WINGDI_RGB(192,192,192), 255, LWA_COLORKEY);

		} // if/
	} // if

if (CustomCloseButton)
	{ // suppress sysmenu
	Style &= ~(WS_SYSMENU);
	SetWindowLong((HWND)GetNativeWin(), GWL_STYLE, Style);
	SetWindowPos((HWND)GetNativeWin(), NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
	} // if

PositionWinCaptionBar(LONG_MIN, LONG_MIN);
UpdateWinCaptionBarState(~WCS_FENETRE_WINSTATE_EMBED); // update them all (except WCS_FENETRE_WINSTATE_EMBED), without having to keep track of each of them
// Show all buttons now
for (int i = 0; i < ButtonCount; i++)
	{
	ShowWindow(Buttons[i], SW_SHOW);
	} // for
return(true);

} // Fenetre::CreateWinCaptionBar

/*===========================================================================*/

void Fenetre::UpdateWinCaptionBarState(int Updating)
{
unsigned long ViewState = NULL;

if (WinCaptionButtonBar)
	{
	// collect view state, if it's a view
	if (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW) && GlobalApp->GUIWins->CVG)
		{
		ViewState = GlobalApp->GUIWins->CVG->QueryViewWindowState(GlobalApp->GUIWins->CVG->ViewNumFromFenetre(this));
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_DOCK)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, ID_WINMENU_DOCK);
		SendMessage(ButtonToUpdate, BM_SETCHECK, TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_SHOWADV)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, ID_WINMENU_SHOWADVANCED);
		SendMessage(ButtonToUpdate, BM_SETCHECK, TestWinManFlags(WCS_FENETRE_WINMAN_SHOWADV) ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_EMBED)
		{
		// removing a button currently means destroying and recreating caption button bar
		DestroyWindow(WinCaptionButtonBar);
		CreateWinCaptionBar(); // this will call us again, but without EMBED
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_PAN)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_CAP_MOVEVIEW);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_PANVIEW ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_ROTATE)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_CAP_ROTVIEW);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_ROTVIEW ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_ZOOM)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_CAP_ZOOMVIEW);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_ZOOMVIEW ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_ZOOMBOX)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_TB_ZOOMBOX);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_ZOOMBOX ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_RENDER)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_TB_RENDERVIEW);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_RENDERPREV ? true : false, 0);
		} // if
	if (Updating & WCS_FENETRE_WINSTATE_CONSTRAIN)
		{
		HWND ButtonToUpdate = GetDlgItem(WinCaptionButtonBar, IDI_TB_LTDREGION);
		SendMessage(ButtonToUpdate, BM_SETCHECK, ViewState & WCS_VIEWGUI_VIEWSTATE_CONSTRAIN ? true : false, 0);
		} // if
	InvalidateRect(WinCaptionButtonBar, NULL, false);
	} // if

} // Fenetre::UpdateWinCaptionBarState

/*===========================================================================*/

#ifdef _WIN32
void DrawingFenetre::HandleBackupContent(AppEvent *Activity)
{
int W, H;
NativeBitmap NewSize;
NativeDrawContext TempDC, CopyDC;
HGDIOBJ SpareBsh, SparePen;
WINDOWPLACEMENT WinInfo;
PAINTSTRUCT ClipZone;
int Left, Top;
GLDrawingFenetre *MeGLDF;

if ((Activity->GUIMessage.message == WM_SIZE) && (Activity->GUIMessage.hwnd == NativeWin))
	{
	// Resize the OffScreen Content Backup bitmap to new window size
	// Do NOT return from here, fall through to AppModule event processing
	W = LOWORD(Activity->GUIMessage.lParam); // width of client area
	H = HIWORD(Activity->GUIMessage.lParam); // height of client area
	//GetClientRect(Activity->GUIMessage.hwnd, &SizeRect);
	if ((W != OSCBWidth) || (H != OSCBHeight))
		{
		TempDC = GetDC(NativeWin);
		//if (NewSize = CreateCompatibleBitmap(TempDC, W, H))
		if (NewSize = CreateDIB24(TempDC, W, H))
			{
			if (OffScreen)
				{
				SelectObject(OffScreen, NewSize);
				} // if
			if (ContentBackup)
				{
				// Refresh new bitmap
				if (CopyDC = CreateCompatibleDC(OffScreen))
					{
					// Clear new bitmap
					SpareBsh = SelectObject(OffScreen, GetStockObject(BLACK_BRUSH));
					SparePen = SelectObject(OffScreen, GetStockObject(NULL_PEN));
					Rectangle(OffScreen, 0, 0, W + 1, H + 1);
					SelectObject(OffScreen, SpareBsh);
					SelectObject(OffScreen, SparePen);
					// Copy old bitmap to new one...
					SelectObject(CopyDC, ContentBackup);
//					if ((W > OSCBWidth) || (H > OSCBHeight))
						{
						BitBlt(OffScreen, 0, 0, OSCBWidth, OSCBHeight, CopyDC, 0, 0, SRCCOPY);
						} // if
					DeleteObject(CopyDC);
					} // if
				DeleteObject(ContentBackup);
				} // if
			if (RealFenType == 3)
				{
				//float *NewCBZ;
				MeGLDF = (GLDrawingFenetre *)this;
				//NewCBZ = (float *)AppMem_Alloc((W * H * sizeof(float)), APPMEM_CLEAR);
				//if (CBZ && NewCBZ)
					{
					// no intelligent way to copy...
					} // if
				if (MeGLDF->CBZ)
					{
					/*** F2 MOD
					char msg[132];
					sprintf(msg, "Line %d : Freeing %d * %d * 4\n", __LINE__, OSCBWidth, OSCBHeight);
					DEBUGOUT(msg);
					***/
					/***
					AppMem_Free(MeGLDF->CBZ, OSCBWidth * OSCBHeight * sizeof(float));
					MeGLDF->CBZ = NULL;
					***/
					delete MeGLDF->CBZ;
					MeGLDF->CBZ = NULL;
					} // if
				//CBZ = NewCBZ;
				} // if
			OSCBWidth = W;
			OSCBHeight = H;
			ContentBackup = NewSize;
			} // if
		ReleaseDC(NativeWin, TempDC);
		TempDC = NULL;
		} // if
	
	// Now tell project about new size
	if (FindWindowOrigin(NativeWin, Left, Top))
		{
		GlobalApp->MainProj->SetWindowCoords(FenID, (short)Left, (short)Top, (short)W, (short)H);
		} // if
	
	} // if
if (Activity->GUIMessage.message == WM_PAINT)
	{
	// Skip redraw if iconified
	WinInfo.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(NativeWin, &WinInfo);
	if (! ((WinInfo.showCmd == SW_MINIMIZE) ||
	 (WinInfo.showCmd == SW_SHOWMINIMIZED) || (WinInfo.showCmd == SW_SHOWMINNOACTIVE)) )
		{
		// Handle redrawing from offscreen buffer...
		if (OffScreen && ContentBackup)
			{
			TempDC = BeginPaint(NativeWin, &ClipZone);
			W = (unsigned short)ClipZone.rcPaint.left + 1;
			H = (unsigned short)ClipZone.rcPaint.top;
			BitBlt(TempDC, ClipZone.rcPaint.left, ClipZone.rcPaint.top,
			 1 + ClipZone.rcPaint.right - ClipZone.rcPaint.left,
			 1 + ClipZone.rcPaint.bottom - ClipZone.rcPaint.top,
			 OffScreen, ClipZone.rcPaint.left, ClipZone.rcPaint.top, SRCCOPY);
			EndPaint(NativeWin, &ClipZone);
			} // if
		} // if
	} // if

} // DrawingFenetre::HandleBackupContent

#endif // _WIN32

/*===========================================================================*/

void GLDrawingFenetre::HandleBackupContent(AppEvent *Activity)
{
#ifdef _WIN32
int W, H;

if ((Activity->GUIMessage.message == WM_SIZE) && (Activity->GUIMessage.hwnd == NativeWin))
	{
	// Resize the OffScreen Content Backup bitmap to new window size
	// Do NOT return from here, fall through to AppModule event processing
	W = LOWORD(Activity->GUIMessage.lParam); // width of client area
	H = HIWORD(Activity->GUIMessage.lParam); // height of client area
	if (FGLSubWin)
		{
		if ((W != OSCBWidth) || (H != OSCBHeight))
			{
			SetWindowPos(FGLSubWin, NULL, NULL, NULL, W, H, SWP_NOMOVE | SWP_NOZORDER);
			} // if
		ReGL();
		} // if
	} // if
#endif // _WIN32

DrawingFenetre::HandleBackupContent(Activity);

} // GLDrawingFenetre::HandleBackupContent

/*===========================================================================*/

long Fenetre::DispatchEvent(AppEvent *Activity, WCSApp *AppScope)
{
long Result = 0, EventHandled, WidgetID;
unsigned short TempW = 0, TempH = 0, Notify;
char ControlState, ShiftState, AltState, LeftState, MiddleState, RightState, MouseState;
static char ReentrantLock = 0;
GUIFenetre *GThis;
NativeDrawContext IconClearDC;
char IsDF;
#ifdef _WIN32
int Left, Top;
//unsigned short Check = 0;
//static char WinName[40];
HGDIOBJ SpareBsh, SparePen;
LPHELPINFO LPHI;
int DidHelp = 0;
#endif // _WIN32

#ifdef _WIN32

IsDF = !(DoYouEraseBG()); // Hint: GUIFens erase themselves

#ifdef DEBUGGUIMSG
if (WCS_APP_EVENTTYPE_MSWIN != Activity->Type)
	sprintf(debugMsg, "Dispatch: Type = %xh, Message = %xh\n", Activity->Type, Activity->GUIMessage.message);
else
	sprintf(debugMsg, "Dispatch: Message = %xh\n", Activity->GUIMessage.message);
OutputDebugString(debugMsg);
#endif // DEBUGGUIMSG

// This causes a losefocus event on SmartNumerics and Text fields when closing windows, so that
// any unprocesed changes are dealt with before the window is obliterated
// putting this above ReentrantLock allows us to process the losefocus even though we're already
// in the start of an event, but it is ok since we're not far into the event processing
if ((Activity->Type == WCS_APP_EVENTTYPE_MSWIN) && (Activity->GUIMessage.message == WM_CLOSE))
	{
	SetFocus(NULL);
	} // if

ReentrantLock++;

// Handle Help
if (Activity->GUIMessage.message == WM_HELP)
	{
	// Invoke Help
	LPHI = (LPHELPINFO)Activity->GUIMessage.lParam;
	if (GlobalApp->HelpSys)
		{
		if (LPHI->iContextType == HELPINFO_MENUITEM)
			{
			// Invoke Help for menus
			DidHelp = GlobalApp->HelpSys->OpenHelpTopic('MENU');
			} // if
		else
			{
			// Invoke Help for this window.
			DidHelp = GlobalApp->HelpSys->OpenHelpTopic(QueryHelpTopic());
			} // else
		} // if
	if (!DidHelp)
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, "Unable to open help file.");
		// help system already notifies user, no need to do it again.
		//UserMessageOK("Online Help", "Unable to open help file.");
		} // if
	ReentrantLock--;
	return(1);
	} // else if
#endif // _WIN32

#ifdef _WIN32
else if (Activity->GUIMessage.message == WM_SHOWWINDOW)
	{
	if ((int)Activity->GUIMessage.lParam == SW_PARENTOPENING)
		{
		ShowWindow(Activity->GUIMessage.hwnd, SW_SHOW);
		if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_SHOW);
		ReentrantLock--;
		return(1);
		} // if
	} // if
#endif // _WIN32

#ifdef _WIN32
else if (Activity->GUIMessage.message == WM_MOVE && Activity->GUIMessage.hwnd == (HWND)GetNativeWin())
	{
	if (FindWindowOrigin(Activity->GUIMessage.hwnd, Left, Top))
		{
		RECT SizeRect;
		GetClientRect(Activity->GUIMessage.hwnd, &SizeRect);
		GlobalApp->MainProj->SetWindowCoords(FenID, Left, Top, (short)SizeRect.right, (short)SizeRect.bottom);
		} // if
	} // if
#endif // _WIN32

#ifdef _WIN32
else if (Activity->GUIMessage.message == WM_ICONERASEBKGND)
	{
	// Clear background
	IconClearDC = (NativeDrawContext)Activity->GUIMessage.wParam;
	SpareBsh = SelectObject(IconClearDC, GetStockObject(WHITE_BRUSH));
	SparePen = SelectObject(IconClearDC, GetStockObject(NULL_PEN));
	Rectangle(IconClearDC, 0, 0, 64, 64);
	SelectObject(IconClearDC, SpareBsh);
	SelectObject(IconClearDC, SparePen);
	ReentrantLock--;
	return(1);
	} // if
#endif // _WIN32

#ifdef _WIN32
else if ((Activity->GUIMessage.message == WM_ERASEBKGND) && (!DoYouEraseBG()))
	{
	ReentrantLock--;
	return(1);
	} // if
#endif // _WIN32

#ifdef _WIN32

else if (Activity->GUIMessage.message == WM_NCPAINT)
	{
	long WinExStyle;
	HDC FrameDC;
	HGDIOBJ PutBack;
	RECT ClientZone, WindowZone;
	WinExStyle = GetWindowLong(Activity->GUIMessage.hwnd, GWL_EXSTYLE);
	if ((WinExStyle & (WS_EX_TOOLWINDOW|WS_EX_STATICEDGE)) == (WS_EX_TOOLWINDOW|WS_EX_STATICEDGE))
		{
		if (FrameDC = GetWindowDC(Activity->GUIMessage.hwnd))
			{
			GetClientRect(Activity->GUIMessage.hwnd, &ClientZone); // size of client area
			GetWindowRect(Activity->GUIMessage.hwnd, &WindowZone); // screen position of nonclient area
			// one pixel outside client area...
			ClientZone.left   -= 1;
			ClientZone.top    -= 1;
			//ClientZone.right  += 1;
			//ClientZone.bottom += 1;
			ClientToScreen(Activity->GUIMessage.hwnd, (LPPOINT)&ClientZone.left);
			ClientToScreen(Activity->GUIMessage.hwnd, (LPPOINT)&ClientZone.right);
			// Translate Screen coordinates of client-border into non-client window coordinates
			ClientZone.left   -= WindowZone.left;
			ClientZone.right  -= WindowZone.left;
			ClientZone.top    -= WindowZone.top;
			ClientZone.bottom -= WindowZone.top;

			// Draw the frame in
			PutBack = SelectObject(FrameDC, GetStockObject(BLACK_PEN));
			MoveToEx(FrameDC, ClientZone.left, ClientZone.top, NULL);
			LineTo(FrameDC, ClientZone.right, ClientZone.top);
			LineTo(FrameDC, ClientZone.right, ClientZone.bottom);
			LineTo(FrameDC, ClientZone.left, ClientZone.bottom);
			LineTo(FrameDC, ClientZone.left, ClientZone.top);
			DeleteObject(SelectObject(FrameDC, PutBack));
			
			ReleaseDC(Activity->GUIMessage.hwnd, FrameDC);
			} // if
		// Need to paint little bit of WS_EX_TOOLWINDOW frame that Windoze forgets to
		} // if
	} // if

else if ((Activity->GUIMessage.message == WM_SIZE) ||
 (Activity->GUIMessage.message == WM_PAINT))
	{
	// This only does something on DrawingFenetres...
	HandleBackupContent(Activity);
	// Update Size variables
	if (Activity->GUIMessage.message == WM_SIZE)
		{
		TempW = LOWORD(Activity->GUIMessage.lParam);
		TempH = HIWORD(Activity->GUIMessage.lParam);
		} // if
	// Don't return.
	} // if

else if (Activity->GUIMessage.message == WM_MOUSEMOVE)
	{
	InstallPointer(CurrentPointer);
	} // if

else if (Activity->GUIMessage.message == WM_ACTIVATE)
	{
	// auto-hide PopupNotifier during window focus changes.
	GlobalApp->Toaster->HideIfNeeded();
	if ((LOWORD(Activity->GUIMessage.wParam) == WA_ACTIVE) || (LOWORD(Activity->GUIMessage.wParam) == WA_CLICKACTIVE))
		{
		// We now have HandleGainFocus
		GainFocus();
		if (Owner && ReentrantLock < 2) // ReentrantLock prevents dispatching to a handler while we're already in one, trashing the CommonVars
			{
			Owner->SetCommonVars(Activity, AppScope, Activity->GUIMessage.hwnd, this);
			Owner->HandleGainFocus();
			} // if
		if (WinCaptionButtonBar) PositionWinCaptionBar(LONG_MIN, LONG_MIN);
		ReentrantLock--;
		return(0);
		} // if
	else
		{
		LoseFocus();
		if (Owner && ReentrantLock < 2) // ReentrantLock prevents dispatching to a handler while we're already in one, trashing the CommonVars
			{
			Owner->SetCommonVars(Activity, AppScope, Activity->GUIMessage.hwnd, this);
			Owner->HandleLoseFocus();
			} // if
		if (WinCaptionButtonBar) PositionWinCaptionBar(LONG_MIN, LONG_MIN);
		ReentrantLock--;
		return(0);
		} // else
	} // if

else if (Activity->GUIMessage.message == WM_CTLCOLORDLG)
	{
	ReentrantLock--;
	return(0);
	} // if

else if ((Activity->GUIMessage.message == WM_CTLCOLORSTATIC) ||
 (Activity->GUIMessage.message == WM_CTLCOLORBTN))
	{
	ReentrantLock--;
	// this crap is necessary to make STATIC (and hopefully CHECKBOX) widgets get
	// the proper XP background theme in tab containers with gradient backgrounds
    if ((Activity->GUIMessage.message == WM_CTLCOLORSTATIC) && g_xpStyle.IsAppThemed())
		{
		LONG Style;
		WNDPROC ControlType;
		Style = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_STYLE);
		ControlType = (WNDPROC)GetClassLong((HWND)Activity->GUIMessage.lParam, GCL_WNDPROC);
		LONG ID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
		
		if (ControlType == ComboWndProc)
			{
			// don't do transparent or theme background
			return((long)Draw3dCtlColor(0, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam));
			} // if
		else if ((ControlType == EditWndProc) || (ControlType == SlightlyModifiedEditWndProc) || (ControlType == ModifiedEditWndProc))
			{
			return(0);
			} // if
		else if ((ControlType == RealButtonWndProc || ControlType == ButtonWndProc))
			{ // if Button Class
			// Just do the transparent stuff, but don't draw the theme background
			bool DrawThemePBG = false;
			HTHEME hTheme = g_xpStyle.OpenThemeData((HWND)Activity->GUIMessage.lParam, L"BUTTON");
			if ((Style & 0x0000ffff) == BS_AUTOCHECKBOX)
				{
				// IsThemeBackgroundPartiallyTransparent doesn't seem to give us a useful answer WRT to BP_CHECKBOX/CBS_UNCHECKEDNORMAL so we force it
				//DrawThemePBG = (bool)(g_xpStyle.IsThemeBackgroundPartiallyTransparent(hTheme, BP_CHECKBOX, CBS_UNCHECKEDNORMAL) ? 1 : 0);
				if (g_xpStyle.IsAppThemed())
					{
					DrawThemePBG = true;
					} // if
				} // if
			else if ((Style & 0x0000ffff) == BS_GROUPBOX)
				{
				return((long)Draw3dCtlColor(0, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam));
				} // if
			g_xpStyle.CloseThemeData(hTheme);
			if (DrawThemePBG)
				{
				RECT ControlRECT;
				GetClientRect((HWND)Activity->GUIMessage.lParam, &ControlRECT);
				g_xpStyle.DrawThemeParentBackground((HWND)Activity->GUIMessage.lParam, (HDC)Activity->GUIMessage.wParam, &ControlRECT);
				SetBkMode((HDC)Activity->GUIMessage.wParam, TRANSPARENT); 
				return (long)GetStockObject(HOLLOW_BRUSH);
				} // if
			else
				{
				return((long)Draw3dCtlColor(0, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam));
				} // else
			} // if
		else
			{ // not sure who arrives here
			// Removed code 1/9/09 CXH, restored 1/28/09
			// if themes enabled and "hide underscores until ALT key pressed enabled, pressing ALT key the first time causes STATIC text to disappear during WM_CTLCOLORSTATIC
			// so we redraw our control to compensate. We have no way to know if it's the first time, so we do it always
			if(LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT) && g_xpStyle.IsAppThemed())
				{
				InvalidateRect((HWND)Activity->GUIMessage.lParam, NULL, TRUE);
				} // if
			else
				{
				RECT ControlRECT;
				GetClientRect((HWND)Activity->GUIMessage.lParam, &ControlRECT);
				g_xpStyle.DrawThemeParentBackground((HWND)Activity->GUIMessage.lParam, (HDC)Activity->GUIMessage.wParam, &ControlRECT);
				} // if
			} // else
        SetBkMode((HDC)Activity->GUIMessage.wParam, TRANSPARENT); 
        return (long)GetStockObject(HOLLOW_BRUSH);
		} // if
	else
		{
		return((long)Draw3dCtlColor(0, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam));
		} // else
	} // if
#endif // _WIN32

// After this, the if's are no longer else-if's, because some of them have conditions other than just
// the message value, and some are not exclusive to cases in the else-if's above

#ifdef _WIN32
if ((IsWindowVisible(Activity->GUIMessage.hwnd)) || (Activity->GUIMessage.message == WM_MEASUREITEM)
 || (Activity->GUIMessage.message == WM_INITDIALOG))
#else // !_WIN32
if (1)
#endif // !_WIN32
	{
	 // Need to handle Ownerdraw
#ifdef _WIN32
	if ((Activity->GUIMessage.message == WM_DRAWITEM) ||
	 (Activity->GUIMessage.message == WM_MEASUREITEM))
		{
		// we now handle this directly, with info provided in advance
		// by the GUIFenetre and fetched by GetOwnerdrawMode, rather
		// than handing off to the raw old HandleEvent, which nothing uses anymore
		if (!GetFrozen())
			{
			if (GetOwnerdrawMode() == WCS_FENETRE_OWNERDRAW_MODE_BASIC)
				{
				HandleOwnerDrawGook(Activity->GUIMessage.hwnd, Activity->GUIMessage.message, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam, 0);
				Result = 1;
				} // if
			else if (GetOwnerdrawMode() == WCS_FENETRE_OWNERDRAW_MODE_JOE)
				{ // currently, specific to DBEditGUI.cpp
				short ButtonID;
				ButtonID  = (short)Activity->GUIMessage.wParam;
				if (ButtonID == IDC_PARLIST)
					HandleOwnerDrawGook(Activity->GUIMessage.hwnd, Activity->GUIMessage.message, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam, 1);
				else
					HandleOwnerDrawGook(Activity->GUIMessage.hwnd, Activity->GUIMessage.message, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam, 0);
				Result = 1;
				} // if
			else
				{ // no ownerdraw
				Result = 0;
				} // else
			} // if
		else
			{ // frozen
			Result = 0;
			} // else
		} // if
#else // !_WIN32
	if (0)
		{
		// Dummy if
		} // if
#endif // !_WIN32
	else if (((LocalWinSys()->ModalLevel > 0) && !ModalInhibit) || (CurrentPointer == WCS_FENETRE_POINTER_WAIT))
		{
		// Prevent user interaction
		#ifdef _WIN32
		if ((Activity->GUIMessage.message == WM_CLOSE))
			{
			ReentrantLock--;
			return(1);
			} // if
		if ((Activity->GUIMessage.message == WM_KEYDOWN) && ((short)Activity->GUIMessage.wParam == 0x1b)) // ESC
			{
			if (BusyWinAbort == 0)
				{
				BusyWinAbort = 1;
				ReentrantLock--;
				return(1);
				} // if
			} // ESC
		#endif // _WIN32
		ReentrantLock--;
		return(0);
		} // if
	// next else takes care caption bar updating of ModalInhibit==1
	else if ((((LocalWinSys()->ModalLevel > 0)) || (CurrentPointer == WCS_FENETRE_POINTER_WAIT)) && (Activity->GUIMessage.message == WM_WINDOWPOSCHANGING))
		{
		// Prevent user interaction
		#ifdef _WIN32
		LPWINDOWPOS LPWP;
		LPWP = (LPWINDOWPOS)(Activity->GUIMessage.lParam);
		HideSubordinateWindows();
		if (WinCaptionButtonBar && (!(LPWP->flags & SWP_NOMOVE)))
			{
			PositionWinCaptionBar(LPWP->x, LPWP->y);
			} // if
		ReentrantLock--;
		return(1);
		#endif // _WIN32
		} // if
	else if (ReentrantLock < 2) // ReentrantLock prevents dispatching to a handler while we're already in one, trashing the CommonVars
		{
		// Here's where the REAL FUN occurs.
		EventHandled = 0;
		Owner->SetCommonVars(Activity, AppScope, Activity->GUIMessage.hwnd, this);
		#ifdef _WIN32

		if (Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
			{
			WNDPROC ControlWndProc;
			WidgetID = LOWORD(Activity->GUIMessage.wParam);
			Notify = HIWORD(Activity->GUIMessage.wParam);
			if ((Activity->GUIMessage.message == WM_COMMAND) &&
			 (Activity->GUIMessage.lParam == NULL) &&
			 ((Notify == 1) || (Notify == 0)) )
				{
				// is it from the WINMENU?
				if (WidgetID >= ID_THISISJUSTADUMMY)
					{
					switch (WidgetID)
						{
						case ID_WINMENU_CLOSE:
							{
							if (Owner != GlobalApp->MCP) // don't let us close the whole app by accident
								{
								EventHandled = Owner->HandleCloseWin(Activity->GUIMessage.hwnd);
								} // if
							else
								{ // try to determine what window is active and close it
								Fenetre *WideReceiver;

								if (WideReceiver = (Fenetre *)GetWindowLong(GetActiveWindow(), GWL_USERDATA))
									{
									if (WideReceiver->Owner)
										{
										EventHandled = WideReceiver->Owner->HandleCloseWin(Activity->GUIMessage.hwnd);
										} // if
									} // if
								} // else
							break;
							} // if
						case ID_WINMENU_DOCK:
							{
							SetDockState(!GetDockState());
							break;
							} // Dock
						case ID_WINMENU_SHOWADVANCED:
							{
							if (TestWinManFlags(WCS_FENETRE_WINMAN_SHOWADV))
								{
								ClearWinManFlags(WCS_FENETRE_WINMAN_SHOWADV);
								EventHandled = Owner->HandleShowAdvanced(Activity->GUIMessage.hwnd, false);
								} // if
							else
								{
								SetWinManFlags(WCS_FENETRE_WINMAN_SHOWADV);
								EventHandled = Owner->HandleShowAdvanced(Activity->GUIMessage.hwnd, true);
								} // else
							UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_SHOWADV);
							break;
							} // Dock
						case ID_WINMENU_HELP:
							{
							if (GlobalApp->HelpSys)
								{
								// Invoke Help for this window.
								DidHelp = GlobalApp->HelpSys->OpenHelpTopic(FenID);
								} // if
							if (!DidHelp)
								{
								GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, "Unable to open help file.");
								UserMessageOK("Online Help", "Unable to open help file.");
								} // if
							break;
							} // HELP
						default:
							{
							EventHandled = Owner->HandlePopupMenuSelect(WidgetID);
							} //
						} // switch
					// Clear popcoords
					PopupX = PopupY = -1;
					} // if
				else
					{
					EventHandled = Owner->HandleMenuSelect(WidgetID);
					} // else
				} // if
			else if (Activity->GUIMessage.message == WM_NOTIFY)
				{
				LPNMHDR lpnmhdrfoo;
				long NewPageID;
				lpnmhdrfoo = (LPNMHDR)Activity->GUIMessage.lParam;
				if (lpnmhdrfoo)
					{
					LPNM_TREEVIEW MyTV = (LPNM_TREEVIEW)Activity->GUIMessage.lParam;
					TV_DISPINFO *MyTVD = (TV_DISPINFO *)Activity->GUIMessage.lParam;
					//lint -save -e648
					switch (lpnmhdrfoo->code)
						{
						// Tree View
						case TVN_ITEMEXPANDING:
							{
							EventHandled = Owner->HandleTreeExpand(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTV->itemNew.hItem, (void *)MyTV->itemNew.lParam, 1, (MyTV->action == TVE_EXPAND));
							break;
							} // 
						case TVN_ITEMEXPANDED:
							{
							EventHandled = Owner->HandleTreeExpand(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTV->itemNew.hItem, (void *)MyTV->itemNew.lParam, 0, (MyTV->action == TVE_EXPAND));
							break;
							} // 
						case TVN_SELCHANGING:
							{
							EventHandled = Owner->HandleTreeChange(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTV->itemOld.hItem, (unsigned long)MyTV->itemNew.hItem, (void *)MyTV->itemNew.lParam);
							break;
							} // 
						case TVN_BEGINDRAG:
							{
							EventHandled = Owner->HandleTreeBeginDrag(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTV->itemNew.hItem, (void *)MyTV->itemNew.lParam, MyTV->ptDrag.x, MyTV->ptDrag.y);
							break;
							} // 
						case TVN_BEGINLABELEDIT:
							{
							EventHandled = Owner->HandleTreeBeginLabelEdit(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTVD->item.hItem, (void *)MyTVD->item.lParam);
							break;
							} // 
						case TVN_ENDLABELEDIT:
							{
							EventHandled = Owner->HandleTreeEndLabelEdit(lpnmhdrfoo->hwndFrom, Activity->GUIMessage.hwnd, WidgetID, (unsigned long)MyTVD->item.hItem, (void *)MyTVD->item.lParam, MyTVD->item.pszText);
							break;
							} // 
						// Tab Control
						case TCN_SELCHANGE:
							{
							NewPageID = TabCtrl_GetCurSel((HWND)(lpnmhdrfoo->hwndFrom));
							EventHandled = Owner->HandlePageChange((HWND)(lpnmhdrfoo->hwndFrom), Activity->GUIMessage.hwnd, WidgetID,
							 NewPageID);
							break;
							} // 
						case NM_CUSTOMDRAW:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_NMCDREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						case LVN_BEGINLABELEDIT:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_LVNBLEREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						case LVN_ENDLABELEDIT:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_LVNELEREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						case LVN_GETDISPINFO:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_LVNGDIREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						case LVN_ITEMACTIVATE: // from ListView/GridWidget
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = Owner->HandleListSel((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
								} // if
							break;
							} // 
						case LVN_ITEMCHANGED:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								LPNMLISTVIEW LPNMLV = (LPNMLISTVIEW)Activity->GUIMessage.lParam;
								if (LPNMLV->uChanged == LVIF_STATE)
									{
									if (LPNMLV->uNewState & (LVIS_SELECTED | LVIS_FOCUSED )) // new focused or selected
										{
										EventHandled = Owner->HandleListSel((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
										} // if
									} // if
								} // if 
							break;
							} // 
						case LVN_ODSTATECHANGED:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = Owner->HandleListSel((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
								} // if
							break;
							} // 
						case LVN_BEGINSCROLL:
							{ // WM_WCSW_GW_LVNBSREFLECT
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_LVNBSREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						case LVN_ODCACHEHINT:
							{
							ControlWndProc = (WNDPROC)GetClassLong((HWND)(lpnmhdrfoo->hwndFrom), GCL_WNDPROC);
							if (ControlWndProc == GridWidgetWndProc)
								{
								EventHandled = GridWidgetWndProc((HWND)(lpnmhdrfoo->hwndFrom), WM_WCSW_GW_LVNOCHREFLECT, Activity->GUIMessage.wParam, Activity->GUIMessage.lParam);
								} // if
							break;
							} // 
						} // switch
					//lint -restore
					} // if
				} // else if WM_NOTIFY
			else if ((Activity->GUIMessage.message == WM_COMMAND) && (Activity->GUIMessage.lParam))
				{
				ControlWndProc = (WNDPROC)GetClassLong((HWND)Activity->GUIMessage.lParam, GCL_WNDPROC);
				if ((ControlWndProc == SmartCheckWndProc) || (ControlWndProc == ButtonWndProc) ||
				 (ControlWndProc == ToolButtonWndProc) ||
				 (ControlWndProc == SmartRadioWndProc) || (ControlWndProc == ColorBarWndProc) ||
				 (ControlWndProc == RealButtonWndProc) || (ControlWndProc == ToolbarWndProc))
					{
					if (HIWORD(Activity->GUIMessage.wParam) == BN_CLICKED)
						{
						EventHandled = Owner->HandleButtonClick((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
						} // Button
					if (HIWORD(Activity->GUIMessage.wParam) == BN_DBLCLK)
						{
						EventHandled = Owner->HandleButtonDoubleClick((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
						} // Button
					} // if
				if ((ControlWndProc == ListWndProc) || (ControlWndProc == ModifiedListWndProc) || (ControlWndProc == ModifiedTreeWndProc) || (ControlWndProc == ListViewWndProc) || (ControlWndProc == GridWidgetWndProc))
					{
					switch (HIWORD(Activity->GUIMessage.wParam))
						{
						case LBN_SELCHANGE:
							{
							EventHandled = Owner->HandleListSel((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // 
						case LBN_DBLCLK:
							{
							WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
							EventHandled = Owner->HandleListDoubleClick((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // 
						} // switch
					} // if
				if ((ControlWndProc == EditWndProc) || (ControlWndProc == SlightlyModifiedEditWndProc))
					{
					switch (HIWORD(Activity->GUIMessage.wParam))
						{
						case EN_CHANGE:
							{
							EventHandled = Owner->HandleStringEdit((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // 
						case EN_KILLFOCUS:
							{
							EventHandled = Owner->HandleStringLoseFocus((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // 
						} // switch
					} // if
				if (ControlWndProc == ComboWndProc)
					{
					switch (HIWORD(Activity->GUIMessage.wParam))
						{
						//case CBN_SELCHANGE:
						case CBN_SELENDOK:
							{
							EventHandled = Owner->HandleCBChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);

							break;
							} // 
						} // switch
					} // if
				} // else if WM_COMMAND
			else if ((Activity->GUIMessage.message == WM_REQ_NOTIFY) || (Activity->GUIMessage.message == NIN_BALLOONTIMEOUT) || (Activity->GUIMessage.message == NIN_BALLOONUSERCLICK))
				{ // forward notifications from PopupNotifier
				// or, that was the plan. These events never seem to arrive. Leaving code in place in case I figure out the fix.
				GlobalApp->Toaster->HandleNotificationEvent(Activity, GlobalApp);
				} // else if WM_REQ_NOTIFY
			else if ((Activity->GUIMessage.message == WM_WCSW_DD_CHANGE) ||
			 (Activity->GUIMessage.message == WM_WCSW_SC_CHANGE) ||
			 (Activity->GUIMessage.message == WM_WCSW_SR_CHANGE) ||
			 (Activity->GUIMessage.message == WM_WCSW_FI_CHANGE) ||
			 (Activity->GUIMessage.message == WM_WCSW_FI_PRECHANGE) ||
			 (Activity->GUIMessage.message == WM_WCSW_FI_OPT1) ||
			 (Activity->GUIMessage.message == WM_WCSW_FI_OPT2) ||
			 (Activity->GUIMessage.message == WM_WCSW_FI_CHANGEARROW))
				{
				ControlWndProc = (WNDPROC)GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_WNDPROC);
				if (ControlWndProc == DiskDirWndProc)
					{
					EventHandled = Owner->HandleDDChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
					} // if
				if (ControlWndProc == SmartCheckWndProc)
					{
					EventHandled = Owner->HandleSCChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
					} // if
				if (ControlWndProc == SmartRadioWndProc)
					{
					EventHandled = Owner->HandleSRChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
					} // if
				if (ControlWndProc == FloatIntWndProc)
					{
					switch (Activity->GUIMessage.message)
						{
						case WM_WCSW_FI_CHANGE:
							{
							EventHandled = Owner->HandleFIChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // FI_CHANGE
						case WM_WCSW_FI_CHANGEARROW:
							{
							EventHandled = Owner->HandleFIChangeArrow((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // WM_WCSW_FI_CHANGEARROW
						case WM_WCSW_FI_PRECHANGE:
							{
							EventHandled = Owner->HandleFIPreChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // FI_PRECHANGE
						case WM_WCSW_FI_OPT1:
							{
							EventHandled = Owner->HandleFIOpt1((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // FI_OPT1
						case WM_WCSW_FI_OPT2:
							{
							EventHandled = Owner->HandleFIOpt2((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // FI_OPT2
						case WM_WCSW_FI_OPT3:
							{
							EventHandled = Owner->HandleFIOpt2((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
							break;
							} // FI_OPT2
						} // switch
					} // if
				} // else if WM_WCSW_*_CHANGE/PRECHANGE/OPT*/CHANGEARROW
			else if (Activity->GUIMessage.message == WM_WCSW_LIST_COPY)
				{
				// need to reconstruct WidgetID as we use wParam for ItemData
				WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
				EventHandled = Owner->HandleListCopyItem((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID, (void *)Activity->GUIMessage.wParam);
				} // else if WM_WCSW_LIST_COPY
			else if (Activity->GUIMessage.message == WM_WCSW_LIST_PASTE)
				{
				// need to reconstruct WidgetID as we use wParam for ItemData
				WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
				EventHandled = Owner->HandleListPasteItem((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID, (void *)Activity->GUIMessage.wParam);
				} // else if WM_WCSW_LIST_PASTE
			else if (Activity->GUIMessage.message == WM_WCSW_LIST_DELITEM)
				{
				// need to reconstruct WidgetID as we use wParam for ItemData
				WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
				EventHandled = Owner->HandleListDelItem((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID, (void *)Activity->GUIMessage.wParam);
				} // else if WM_WCSW_LIST_DELITEM
			else if (Activity->GUIMessage.message == WM_WCSW_LIST_CONTEXTMENU)
				{
				PopMenuEvent *PME;
				if (PME = (PopMenuEvent *)Activity->GUIMessage.lParam)
					{
					// treat Action as text, even if it isn't
					EventHandled = Owner->HandleTreeMenuSelect((HWND)Activity->GUIMessage.hwnd, Activity->GUIMessage.hwnd, WidgetID, PME->TreeItem, PME->RAH, (char *)PME->ActionText, PME->Derived);
					} // if
				} // else if WM_WCSW_LIST_CONTEXTMENU
			else if (Activity->GUIMessage.message == WM_WCSW_COLLISTVIEW_SELCOLUMN)
				{
				EventHandled = Owner->HandleListViewColSel((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
				} // else if WM_WCSW_LIST_DELITEM
			else if (Activity->GUIMessage.message == WM_WCSW_TC_DATACHANGED)
				{
				EventHandled = Owner->HandleTextColumnMarkerChange((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID, (long)Activity->GUIMessage.lParam);
				} // else if WM_WCSW_TC_DATACHANGED
			else
				{
				GThis = (GUIFenetre *)this;
				#ifdef DEBUGGUIMSG
				GUImsg = Activity->GUIMessage.message;
				sprintf(debugMsg, "GUImsg = %xh\n", GUImsg);
				#endif // DEBUGGUIMSG
				switch (Activity->GUIMessage.message)
					{
					case WM_HSCROLL:
					case WM_VSCROLL:
						{
						long ScrollInc, Pos;
						short CommandCode;

						CommandCode  = LOWORD(Activity->GUIMessage.wParam);
						WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
						// Inquire about scroll increment
						ScrollInc = Owner->HandleScroll(0, 0, (HWND)Activity->GUIMessage.lParam, WidgetID);

						Pos = GThis->WidgetGetScrollPos((WIDGETID)WidgetID, Activity->GUIMessage.hwnd);
						switch (CommandCode)
							{
							case SB_BOTTOM:
							case SB_TOP:
								{ // scroll to ends of range
								INT Min, Max;
								if (NativeControl ScrollWidget = GThis->GetWidgetFromID((WIDGETID)WidgetID))
									{
									if (GetScrollRange(ScrollWidget, SB_CTL, &Min, &Max))
										{
										if (CommandCode == SB_BOTTOM) Pos = Max;
										else if (CommandCode == SB_TOP) Pos = Min;
										GThis->WidgetSetScrollPos((WIDGETID)WidgetID, Pos, Activity->GUIMessage.hwnd);
										} // if
									} // if
								break;
								} // TOP/BOTTOM
							case SB_PAGELEFT:
								{
								GThis->WidgetSetScrollPos((WIDGETID)WidgetID, Pos - ScrollInc, Activity->GUIMessage.hwnd);
								break;
								}
							case SB_PAGERIGHT:
								{
								GThis->WidgetSetScrollPos((WIDGETID)WidgetID, Pos + ScrollInc, Activity->GUIMessage.hwnd);
								break;
								}
							case SB_LINELEFT:
								{
								GThis->WidgetSetScrollPos((WIDGETID)WidgetID, Pos - 1, Activity->GUIMessage.hwnd);
								break;
								}
							case SB_LINERIGHT:
								{
								GThis->WidgetSetScrollPos((WIDGETID)WidgetID, Pos + 1, Activity->GUIMessage.hwnd);
								break;
								}
							case SB_THUMBPOSITION:
								{
								GThis->WidgetSetScrollPos((WIDGETID)WidgetID, HIWORD(Activity->GUIMessage.wParam), Activity->GUIMessage.hwnd);
								break;
								}
							case SB_THUMBTRACK:
								{
								GThis->WidgetSetScrollPosQuiet((WIDGETID)WidgetID, HIWORD(Activity->GUIMessage.wParam), Activity->GUIMessage.hwnd);
								break;
								}
							} // switch
						Pos = GThis->WidgetGetScrollPos((WIDGETID)WidgetID, Activity->GUIMessage.hwnd);

						// Send Scroll event on to do its work
						if (Activity->GUIMessage.message == WM_HSCROLL) EventHandled = Owner->HandleScroll(1, Pos, (HWND)Activity->GUIMessage.lParam, WidgetID);
						else EventHandled = Owner->HandleScroll(2, Pos, (HWND)Activity->GUIMessage.lParam, WidgetID);
						break;
						} // WM_VSCROLL/WM_HSCROLL
					case WM_CLOSE:
						{
						// SetFocus is now done at beginning of DispatchEvent to avoid trashing variables.
						EventHandled = Owner->HandleCloseWin(Activity->GUIMessage.hwnd);
						break;
						} // WM_CLOSE
					case WM_SIZE:
						{
						EventHandled = Owner->HandleReSized(Activity->GUIMessage.wParam, LOWORD(Activity->GUIMessage.lParam), HIWORD(Activity->GUIMessage.lParam));
						break;
						} // WM_SIZE
					case WM_NCLBUTTONDOWN:
						{
						if (WinCaptionButtonBar) PositionWinCaptionBar(LONG_MIN, LONG_MIN);
						// do not mark this as handled or normal window nonclient interaction will stop
						break;
						} // spoof
					case WM_NCRBUTTONDOWN:
						{
						EventHandled = 1;
						break;
						} // spoof
					case WM_NCRBUTTONUP:
					case WM_NCLBUTTONDBLCLK:
						{
						if (Activity->GUIMessage.hwnd != LocalWinSys()->RootWin)
							{
							POINTS Hit;
							Hit = MAKEPOINTS(Activity->GUIMessage.lParam);
							if (!TestWinManFlags(WCS_FENETRE_WINMAN_NOPOPUP))
								{
								DoPopup(Hit.x, Hit.y, (Activity->GUIMessage.message == WM_NCLBUTTONDBLCLK));
								} // if
							EventHandled = 1;
							} // if
						break;
						} // WM_NCRBUTTONUP/WM_NCLBUTTONDBLCLK invoke winmenu
					case WM_WINDOWPOSCHANGING:
						{
						LPWINDOWPOS LPWP;
						int WinPosX, WinPosY;
						POINT XLate;
						RECT OldPos;
						unsigned long WStyle;
						LPWP = (LPWINDOWPOS)(Activity->GUIMessage.lParam);
						if (Activity->GUIMessage.hwnd == LocalWinSys()->RootWin)
							{
							// is it a move?
							if (!(LPWP->flags & SWP_NOMOVE))
								{
								GetWindowRect(Activity->GUIMessage.hwnd, &OldPos);
								XLate.x = LPWP->x - OldPos.left;
								XLate.y = LPWP->y - OldPos.top;
								LocalRootWin = LocalWinSys()->RootWin; // out of band variable passing
								ShuffleData = BeginDeferWindowPos(20);
								EnumThreadWindows(GetCurrentThreadId(), (WNDENUMPROC)ShuffleChildren, (LPARAM)(&XLate));
								EndDeferWindowPos(ShuffleData);
								ShuffleData = NULL;
								} // if
							break;
							} // if
						WStyle = GetWindowLong(Activity->GUIMessage.hwnd, GWL_STYLE);
/*
						if (!(LPWP->flags & SWP_NOZORDER))
							{
							if (!(TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED)))
								{
								// are we going behind a docked window?
								if (LPWP->hwndInsertAfter)
									{
									Fenetre *Sibling;
									if (Sibling = (Fenetre *)GetWindowLong(LPWP->hwndInsertAfter, GWL_USERDATA))
										{
										if (Sibling->TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))
											{
											// can't let that happen
											//LPWP->flags |= SWP_NOZORDER;
											} // if
										} // if
									} // if
								} // if
							} // if
*/
						if (!ForceMoveSize)
							{
							LeftState    = (Activity->GUIMessage.wParam & MK_LBUTTON ? 1 : 0);
							MiddleState  = (Activity->GUIMessage.wParam & MK_MBUTTON ? 1 : 0);
							RightState   = (Activity->GUIMessage.wParam & MK_RBUTTON ? 1 : 0);
							MouseState = (LeftState || MiddleState || RightState);
							if (TestWinManFlags(WCS_FENETRE_WINMAN_NOSIZE)) LPWP->flags |= SWP_NOSIZE;
							//if (TestWinManFlags(WCS_FENETRE_WINMAN_NOMOVE) || (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW) && TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))) LPWP->flags |= SWP_NOMOVE;
							else if (TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) && !(LPWP->flags & SWP_NOMOVE))
								{
								short MX, MY, MW, MH;
								POINT CursorPos;
								int ViewPane, OrigPane;
								GetCursorPos(&CursorPos);
								WinPosX = CursorPos.x;
								WinPosY = CursorPos.y;
								// change screen coords into faux child-window coords
								if (WStyle & WS_POPUP)
									{
									XLate.x = LPWP->x;
									XLate.y = LPWP->y;
									ScreenToClient(LocalWinSys()->RootWin, &XLate);
									LPWP->x = (short)XLate.x;
									LPWP->y = (short)XLate.y;

									XLate.x = WinPosX;
									XLate.y = WinPosY;
									ScreenToClient(LocalWinSys()->RootWin, &XLate);
									WinPosX = (short)XLate.x;
									WinPosY = (short)XLate.y;
									} // if
								MX = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
								MY = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
								MW = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
								MH = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);

								OrigPane = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, VPID);
								// Is the window being moved outside the matrix?
								if ((ViewPane = GlobalApp->MainProj->ViewPorts.GetPaneFromXY(WinPosX, WinPosY)) != -1)
									{ // no
									// Have we gone anywhere?
									if (ViewPane != OrigPane)
										{
										// avoid view cells
										if (GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
											{
											ViewPane = OrigPane;
											} // if
										else
											{
											if (IsDF && GlobalApp->MainProj->ViewPorts.Inhabitants(ViewPane))
												{
												ViewPane = OrigPane;
												} // if
											else
												{
												// Make sure we can fit here
												short CellWidth, CellHeight;
												CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(ViewPane);
												CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(ViewPane);
												if (IsDF || ((CellWidth + WCS_FENETRE_DOCK_FUDGE_W >= NominalWidth) && (CellHeight + WCS_FENETRE_DOCK_FUDGE_H >= NominalHeight)))
													{
													LPWP->x = GlobalApp->MainProj->ViewPorts.GetPaneX(ViewPane);
													LPWP->y = GlobalApp->MainProj->ViewPorts.GetPaneY(ViewPane);
													LPWP->cx = CellWidth;
													LPWP->cy = CellHeight;
													} // if
												else
													{
													ViewPane = OrigPane;
													} // else
												} // else
											} // else
										} // if
									} // if
								else
									{ // yes, put it back where it was
									ViewPane = OrigPane;
									} // else
								// Do we need to put it back where it was?
								if (ViewPane == OrigPane)
									{
									LPWP->x = GlobalApp->MainProj->ViewPorts.GetPaneX(ViewPane);
									LPWP->y = GlobalApp->MainProj->ViewPorts.GetPaneY(ViewPane);
									LPWP->cx = GlobalApp->MainProj->ViewPorts.GetPaneW(ViewPane);
									LPWP->cy = GlobalApp->MainProj->ViewPorts.GetPaneH(ViewPane);
									} // else
								else
									{ // might need to update its new cell
									if (ViewPane != OrigPane) // did the cell change?
										{ // update dock cell
										memcpy(VPID, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), 3);
										GlobalApp->MainProj->ViewPorts.MoveOut(OrigPane);
										if (IsDF)
											{
											GlobalApp->MainProj->ViewPorts.ClearFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, OrigPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
											} // if
										GlobalApp->MainProj->SetWindowCell(FenID, VPID, 0, 0, 0, 0);
										GlobalApp->MainProj->ViewPorts.MoveIn(ViewPane);
										if (IsDF)
											{
											GlobalApp->MainProj->ViewPorts.SetFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
											} // if
										VPID[2] = NULL;
										} // if
									} // else
								// Change faux child-window coords back to screen coords
								if (WStyle & WS_POPUP)
									{
									XLate.x = LPWP->x;
									XLate.y = LPWP->y;
									ClientToScreen(LocalWinSys()->RootWin, &XLate);
									LPWP->x = (short)XLate.x;
									LPWP->y = (short)XLate.y;
									} // if
								} // if
							// adjust for Vista window size goofiness here
							if(SizeCorrectX != 0 || SizeCorrectY != 0)
								{
								LPWP->x += (short)SizeCorrectX;
								LPWP->y += (short)SizeCorrectY;
								LPWP->cx -= (short)(SizeCorrectX * 2);
								LPWP->cy -= (short)(SizeCorrectY * 2);
								} // if
							} // if
						else
							{ // do autoclear of oneshot ForceMoveSize
							if (ForceMoveSize == 2) ForceMoveSize = 0;
							} // else
						if (WinCaptionButtonBar && (!(LPWP->flags & SWP_NOMOVE)))
							{ // this code is only for real movement, not z-order and such
							HideSubordinateWindows();
							if (WinCaptionButtonBar) PositionWinCaptionBar(LPWP->x, LPWP->y);
							} // if
						
						EventHandled = Owner->HandlePreReSize((void *)Activity->GUIMessage.lParam);
						break;
						} // WM_WINDOWPOSCHANGING
					case WM_MOVE:
						{
						if (WinCaptionButtonBar) PositionWinCaptionBar(LONG_MIN, LONG_MIN);
						HideSubordinateWindows();
						break;
						} // WM_MOVE
					case WM_PAINT:
						{
						PAINTSTRUCT PS;
						NativeDrawContext NDC;
						if (GetUpdateRect(Activity->GUIMessage.hwnd, NULL, 0))
							{
							NDC = BeginPaint(Activity->GUIMessage.hwnd, &PS);
							EventHandled = Owner->HandleRepaint(Activity->GUIMessage.hwnd, NDC);
							EndPaint(Activity->GUIMessage.hwnd, &PS);
							NDC = NULL;
							} // if
						break;
						} // 
					case WM_MOUSEWHEEL:
					case WM_MOUSEHWHEEL:
						{
						float Amount = 0;
						int pvParam;
						SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &pvParam, 0);

   						ControlState = (Activity->GUIMessage.wParam & MK_CONTROL ? 1 : 0);
						ShiftState   = (Activity->GUIMessage.wParam & MK_SHIFT ? 1 : 0);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);

						Amount = ((float)(GET_WHEEL_DELTA_WPARAM(Activity->GUIMessage.wParam)) / 120.0f) * ((float)pvParam * (1.0f / 30.0f)); // Expressed in units of WHEEL_DELTA, then multiplied by SPI_GETWHEELSCROLLLINES (default: 3) / 30
						if (Activity->GUIMessage.message == WM_MOUSEWHEEL) // vertical
							{
							EventHandled = Owner->HandleMouseWheelVert((short)GET_X_LPARAM(Activity->GUIMessage.lParam), (short)GET_Y_LPARAM(Activity->GUIMessage.lParam), Amount, AltState, ControlState, ShiftState);
							} // if
						else // WM_MOUSEHWHEEL
							{
							EventHandled = Owner->HandleMouseWheelHoriz((short)GET_X_LPARAM(Activity->GUIMessage.lParam), (short)GET_Y_LPARAM(Activity->GUIMessage.lParam), Amount, AltState, ControlState, ShiftState);
							} // else
						break;
						} // WM_MOUSEWHEEL
					case WM_MOUSEMOVE:
						{
						ControlState = (Activity->GUIMessage.wParam & MK_CONTROL ? 1 : 0);
						ShiftState   = (Activity->GUIMessage.wParam & MK_SHIFT ? 1 : 0);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);

						LeftState    = (Activity->GUIMessage.wParam & MK_LBUTTON ? 1 : 0);
						MiddleState  = (Activity->GUIMessage.wParam & MK_MBUTTON ? 1 : 0);
						RightState   = (Activity->GUIMessage.wParam & MK_RBUTTON ? 1 : 0);
						EventHandled = Owner->HandleMouseMove((short)LOWORD(Activity->GUIMessage.lParam), (short)HIWORD(Activity->GUIMessage.lParam), AltState, ControlState, ShiftState, LeftState, MiddleState, RightState);
						break;
						} // MOUSEMOVE
					case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
					case WM_MBUTTONDOWN:
					case WM_MBUTTONUP:
						{
						short Xc, Yc;
						short MX, MY, MW, MH;
						MX = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
						MY = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
						MW = MX + GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
						MH = MY + GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
						ControlState = (Activity->GUIMessage.wParam & MK_CONTROL ? 1 : 0);
						ShiftState   = (Activity->GUIMessage.wParam & MK_SHIFT ? 1 : 0);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);
						Xc = (short)LOWORD(Activity->GUIMessage.lParam);
						Yc = (short)HIWORD(Activity->GUIMessage.lParam);
						if (Activity->GUIMessage.hwnd == LocalWinSys()->RootWin
						 && Activity->GUIMessage.message == WM_RBUTTONUP
						 && Xc > MX && Xc < MW && Yc > MY && Yc < MH)
							{
							POINT Hit;
							Hit.x = Xc;
							Hit.y = Yc;
							ClientToScreen(LocalWinSys()->RootWin, &Hit);
							// Invoke rootwin RButtonClick
							DoPopup((short)Hit.x, (short)Hit.y, 0);
							} // if
						else
							{
							switch (Activity->GUIMessage.message)
								{
								case WM_LBUTTONDOWN: EventHandled = Owner->HandleLeftButtonDown(Xc, Yc, AltState, ControlState, ShiftState); break;
								case WM_LBUTTONUP: EventHandled = Owner->HandleLeftButtonUp(Xc, Yc, AltState, ControlState, ShiftState); break;
								case WM_RBUTTONDOWN: EventHandled = Owner->HandleRightButtonDown(Xc, Yc, AltState, ControlState, ShiftState); break;
								case WM_RBUTTONUP: EventHandled = Owner->HandleRightButtonUp(Xc, Yc, AltState, ControlState, ShiftState); break;
								case WM_MBUTTONDOWN: EventHandled = Owner->HandleMiddleButtonDown(Xc, Yc, AltState, ControlState, ShiftState); break;
								case WM_MBUTTONUP: EventHandled = Owner->HandleMiddleButtonUp(Xc, Yc, AltState, ControlState, ShiftState); break;
								} // switch
							} // else
						break;
						} // case WM_xBUTTONxx
					case WM_LBUTTONDBLCLK:
						{
						short MX, MY, MW, MH, Xc, Yc;

						MX = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
						MY = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
						MW = MX + GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
						MH = MY + GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
						Xc = (short)LOWORD(Activity->GUIMessage.lParam);
						Yc = (short)HIWORD(Activity->GUIMessage.lParam);
						if (Activity->GUIMessage.hwnd == LocalWinSys()->RootWin
						 && Activity->GUIMessage.message == WM_LBUTTONDBLCLK
						 && Xc > MX && Xc < MW && Yc > MY && Yc < MH)
							{
							// Invoke rootwin LButtonDoubleClick
							DoPopup(Xc, Yc, 1);
							} // if
						else
							{
							WidgetID = GetWindowLong((HWND)Activity->GUIMessage.lParam, GWL_ID);
							ControlState = (Activity->GUIMessage.wParam & MK_CONTROL ? 1 : 0);
							ShiftState   = (Activity->GUIMessage.wParam & MK_SHIFT ? 1 : 0);
							AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);
							EventHandled = Owner->HandleLeftButtonDoubleClick(Xc, Yc, AltState, ControlState, ShiftState);
							} // else

						break;
						} // 
					case WM_CHAR:
						{
						ControlState = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL);
						ShiftState   = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);
						EventHandled = Owner->HandleKeyPress((int)Activity->GUIMessage.wParam, AltState, ControlState, ShiftState);
						break;
						} // 
					case WM_KEYUP:
						{
						ControlState = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL);
						ShiftState   = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);
						EventHandled = Owner->HandleKeyUp((int)Activity->GUIMessage.wParam, AltState, ControlState, ShiftState);
						break;
						} // 
					case WM_KEYDOWN:
						{
						ControlState = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL);
						ShiftState   = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT);
						AltState     = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_ALT);
						EventHandled = Owner->HandleKeyDown((int)Activity->GUIMessage.wParam, AltState, ControlState, ShiftState);
						break;
						} // 
					default:
						{
						Activity->GUIMessage.message = Activity->GUIMessage.message;
						#ifdef DEBUGGUIMSG
						GUImsg = Activity->GUIMessage.message;
						switch (GUImsg)
							{
							case WM_ACTIVATEAPP:
								sprintf(debugMsg, "Ignored WM_ERASEBKGND\n");
								break;
							case WM_CAPTURECHANGED:
								sprintf(debugMsg, "Ignored WM_CAPTURECHANGED\n");
								break;
							case WM_CTLCOLOREDIT:
								sprintf(debugMsg, "Ignored WM_CTLCOLOREDIT\n");
								break;
							case WM_CTLCOLORSCROLLBAR:
								sprintf(debugMsg, "Ignored WM_CTLCOLORSCROLLBAR\n");
								break;
							case WM_ENTERIDLE:
								sprintf(debugMsg, "Ignored WM_ENTERIDLE\n");
								break;
							case WM_ENTERMENULOOP:
								sprintf(debugMsg, "Ignored WM_ENTERMENULOOP\n");
								break;
							case WM_ERASEBKGND:
								sprintf(debugMsg, "Ignored WM_ERASEBKGND\n");
								break;
							case WM_EXITMENULOOP:
								sprintf(debugMsg, "Ignored WM_EXITMENULOOP\n");
								break;
							case WM_INITMENU:
								sprintf(debugMsg, "Ignored WM_INITMENU\n");
								break;
							case WM_INITMENUPOPUP:
								sprintf(debugMsg, "Ignored WM_INITMENUPOPUP\n");
								break;
							case WM_KILLFOCUS:
								sprintf(debugMsg, "Ignored WM_KILLFOCUS\n");
								break;
							case WM_MENUSELECT:
								sprintf(debugMsg, "Ignored WM_MENUSELECT\n");
								break;
							case WM_NCACTIVATE:
								sprintf(debugMsg, "Ignored WM_NCACTIVATE\n");
								break;
							case WM_NCHITTEST:
								sprintf(debugMsg, "Ignored WM_NCHITTEST\n");
								break;
							case WM_NCMOUSELEAVE:
								sprintf(debugMsg, "Ignored WM_NCMOUSELEAVE\n");
								break;
							case WM_NCMOUSEMOVE:
								sprintf(debugMsg, "Ignored WM_NCMOUSEMOVE\n");
								break;
							case WM_NCPAINT:
								sprintf(debugMsg, "Ignored WM_NCPAINT\n");
								break;
							case WM_PRINTCLIENT:
								sprintf(debugMsg, "Ignored WM_PRINTCLIENT\n");
								break;
							case WM_SETCURSOR:
								sprintf(debugMsg, "Ignored WM_SETCURSOR\n");
								break;
							case WM_SETFOCUS:
								sprintf(debugMsg, "Ignored WM_SETFOCUS\n");
								break;
							case WM_SHOWWINDOW:
								sprintf(debugMsg, "Ignored WM_SHOWWINDOW\n");
								break;
							case WM_SYSCOMMAND:
								sprintf(debugMsg, "Ignored WM_SYSCOMMAND\n");
								break;
							case WM_UNINITMENUPOPUP:
								sprintf(debugMsg, "Ignored WM_UNINITMENUPOPUP\n");
								break;
							case WM_USER:
								sprintf(debugMsg, "Ignored WM_USER\n");
								break;
							case WM_WINDOWPOSCHANGED:
								sprintf(debugMsg, "Ignored WM_WINDOWPOSCHANGED\n");
								break;
							default:
								sprintf(debugMsg, "Ignored message %xh\n", GUImsg);
								break;
							} // switch
						OutputDebugString(debugMsg);
						#endif // DEBUGGUIMSG
						} // default
					} // switch
				} // else
			} // if
		else
			{ // what kind of event is this?
			Result = 0;
			} // else

		#endif // _WIN32
		if (EventHandled)
			{
			Result = EventHandled;
			} // if
		else
			{
			Result = Owner->HandleEvent();
			} // if
		} // else if
	else
		{ // prevent re-entrant events, except for buttonclick for abort
		Result = 0; // we didn't really do anything with it
		if (Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
			{
			WNDPROC ControlWndProc;
			WidgetID = LOWORD(Activity->GUIMessage.wParam);
			Notify = HIWORD(Activity->GUIMessage.wParam);
			if ((Activity->GUIMessage.message == WM_COMMAND) && (Activity->GUIMessage.lParam))
				{
				ControlWndProc = (WNDPROC)GetClassLong((HWND)Activity->GUIMessage.lParam, GCL_WNDPROC);
				if ((ControlWndProc == ToolButtonWndProc) || (ControlWndProc == ButtonWndProc) ||
				 (ControlWndProc == RealButtonWndProc) || (ControlWndProc == ToolbarWndProc))
					{
					if ((HIWORD(Activity->GUIMessage.wParam) == BN_CLICKED) && (WidgetID == IDI_STOP || WidgetID == ID_DB3KEEP || WidgetID == ID_DB3SELECTALL || WidgetID == ID_DB3CANCEL || WidgetID == IDC_PAUSE || WidgetID == IDC_STOPRENDER || WidgetID == IDC_DISPLAYPREV))
						{
						EventHandled = Owner->HandleButtonClick((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
						Result = 1;
						} // Button
					if ((HIWORD(Activity->GUIMessage.wParam) == BN_CLICKED) && (WidgetID == ID_WINMENU_CLOSE) && Activity->Origin && Activity->Origin->TestWinManFlags(WCS_FENETRE_WINMAN_ISRENDPREV))
						{
						EventHandled = Owner->HandleButtonClick((HWND)Activity->GUIMessage.lParam, Activity->GUIMessage.hwnd, WidgetID);
						Result = 1;
						} // Button
					} // if
				} // if WM_COMMAND
			else if ((Activity->GUIMessage.message == WM_KEYDOWN) && ((short)Activity->GUIMessage.wParam == 0x1b)) // ESC
				{
				if (BusyWinAbort == 0)
					{
					BusyWinAbort = 1;
					} // if
				} // else if WM_KEYDOWN -- ESC
			else if ((Activity->GUIMessage.message == WM_CLOSE) && Activity->Origin && Activity->Origin->TestWinManFlags(WCS_FENETRE_WINMAN_ISRENDPREV))
				{
				EventHandled = Owner->HandleCloseWin(Activity->GUIMessage.hwnd);
				} // if
			} // if
		} // else
	} // if
else
	{
	ReentrantLock--;
	return(0);
	} // else

#ifdef _WIN32
if (Activity->GUIMessage.message == WM_SIZE)
#endif // _WIN32
	{
	SizeX = TempW;
	SizeY = TempH;
	// if we allow the user to resize us, we should reset the nominal sizes
	// so docking will consider our current size
	if (GetWindowLong(Activity->GUIMessage.hwnd, GWL_STYLE) & WS_THICKFRAME)
		{
		NominalWidth  = TempW;
		NominalHeight = TempH;
		} //
	} // if

// this should prevent drawingfenetres from disappearing before
// I can snarf their info
if (Activity->GUIMessage.message == WM_CLOSE)
	{
	Result = 1;
	} // if

ReentrantLock--;
return(Result);

} // Fenetre::DispatchEvent

/*===========================================================================*/

void Fenetre::InternalDockUpdate(int ResizeOnly)
{
double CandidateFit[WCS_PROJECT_WINLAYOUT_MAX_WINDOWS], CandidateBonus[WCS_PROJECT_WINLAYOUT_MAX_WINDOWS];
double AreaFrac, BestRating, MatrixWidth, MatrixHeight;
HWND MyWin;
RECT MWR;
long NumWindows;
short BestFit = -1, TestFit, MatrixScan;
short X, Y, W, H;
short CellWidth, CellHeight, WinCenterX, WinCenterY;
char OldCell[4], TestCell[4];
char IsDF = 0, IsGF = 0, Bonus, ThisBonus;

OldCell[0] = OldCell[1] = OldCell[2] = 0;

IsGF = !(IsDF = !(DoYouEraseBG())); // Hint: GUIFens erase themselves

if (!TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED))
	{ // undocking
	if (VPID[0])
		{
		BestFit = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, VPID);
		if (BestFit != -1)
			{
			GlobalApp->MainProj->ViewPorts.MoveOut(BestFit);
			} // if
		VPID[0] = VPID[1] = NULL;
		} // if
	GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_UNDOCKED, WCS_FENTRACK_FLAGS_ENABLE);
	return;
	} // if

if (VPID[0] && (BestFit == -1))
	{
	// Figure out which cell number best corresponds with our last known Cell
	TestFit = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, VPID);
	if (TestFit != -1)
		{ // we still exist!
		if (DockFitOk(TestFit, ResizeOnly))
			{
			BestFit = TestFit;
			memcpy(VPID, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), 3);
			} // if
		} // if
	if (BestFit == -1)
		{
		// may not have found a good home, so fall through to "bonus" weighted general search, below
		OldCell[0] = VPID[0];
		OldCell[1] = VPID[1];
		OldCell[2] = VPID[2];
		VPID[0] = VPID[1] = VPID[2] = 0;
		} // if
	} // if

// general search for a home
// Might be weighted in favor of cells with letter or ordinal the same our old home cell
if (TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) && (VPID[0] == 0) && (BestFit == -1))
	{
	Bonus = 0;
	if (BestFit == -1)
		{
		MyWin = (HWND)GetNativeWin();
		GetWindowRect(MyWin, &MWR);
		MWR.right  -= MWR.left;
		MWR.bottom -= MWR.top;
		ScreenToClient((HWND)LocalWinSys()->GetRoot(), (LPPOINT)&MWR.left);
		WinCenterX = (short)(MWR.left + (MWR.right  / 2));
		WinCenterY = (short)(MWR.top  + (MWR.bottom / 2));
		NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT);

		MatrixWidth   = (double)GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
		MatrixHeight  = (double)GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);

		for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
			{
			CandidateFit[MatrixScan] = DBL_MAX;
			CandidateBonus[MatrixScan] = 0.0;
			} // for

		AreaFrac = ((double)NominalWidth / MatrixWidth) * ((double)NominalHeight / MatrixHeight);
		// evaluate all cells
		for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
			{
			ThisBonus = 0;
			// Prevent docking GUIFenetres in a 'View' cell
			if (IsDF || !GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
				{
				CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(MatrixScan);
				CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(MatrixScan);
				if (IsDF || ((CellWidth + WCS_FENETRE_DOCK_FUDGE_W >= NominalWidth) && (CellHeight + WCS_FENETRE_DOCK_FUDGE_H >= NominalHeight)))
					{
					memcpy(TestCell, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan), 3);
					CandidateFit[MatrixScan] = GlobalApp->MainProj->ViewPorts.GetPercentArea(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan) - AreaFrac;
					if (IsDF)
						{ // we like cells that are smaller or larger, as long as they're close
						CandidateFit[MatrixScan] = fabs(CandidateFit[MatrixScan]);
						} // if
					// Avoid or penalize occupied cells, bonus cells we like
					if (GlobalApp->MainProj->ViewPorts.Inhabitants(MatrixScan))
						{
						if (IsGF)
							{
							CandidateBonus[MatrixScan] += -1.0;
							} // if
						else
							{
							CandidateFit[MatrixScan] = DBL_MAX; // can't fit here
							} // else
						} // if
					if (toupper(TestCell[0]) == toupper(OldCell[0]))
						{
						CandidateBonus[MatrixScan] += 4.0;
						} // if
					if (toupper(TestCell[1]) == toupper(OldCell[1]))
						{
						CandidateBonus[MatrixScan] += 2.0;
						} // if
					// bonus cell that our midpoint is over
					if (GlobalApp->MainProj->ViewPorts.GetPaneFromXY(WinCenterX, WinCenterY) == MatrixScan)
						{
						CandidateBonus[MatrixScan] += 0.5;
						} // if
					} // if
				} // if
			} // for

		// decide on best cell
		BestRating = DBL_MAX;
		for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
			{
			double Rating; // lower is better
			Rating = CandidateFit[MatrixScan] - (CandidateBonus[MatrixScan] * 0.1);
			if (Rating < BestRating)
				{
				BestRating = Rating;
				BestFit = MatrixScan;
				} // if
			} // for

		} // if
	if (BestFit != -1)
		{
		memcpy(VPID, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), 3);
		} // if
	} // if

if (TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) && VPID[0] && (BestFit != -1))
	{
	X = GlobalApp->MainProj->ViewPorts.GetPaneX(BestFit);
	Y = GlobalApp->MainProj->ViewPorts.GetPaneY(BestFit);
	W = GlobalApp->MainProj->ViewPorts.GetPaneW(BestFit);
	H = GlobalApp->MainProj->ViewPorts.GetPaneH(BestFit);
	if (!ResizeOnly) GlobalApp->MainProj->ViewPorts.MoveIn(BestFit);
	GlobalApp->MainProj->SetWindowCell(FenID, VPID, X, Y, W, H);
	GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_UNDOCKED, WCS_FENTRACK_FLAGS_DISABLE);
	MoveAndSizeFen(X, Y, W, H);
	// MoveAndSizeFen doesn't invoke HandleResized(), so we must do it ourselves
	RECT NewSize;
	GetClientRect((HWND)GetNativeWin(), &NewSize);
	// we have to forge a complete, valid AppEvent here, since we may already be within one.
	AppEvent NewActivity, *ActivityBackup;
	NewActivity.Origin = this;
	NewActivity.Type = WCS_APP_EVENTTYPE_MSWIN;
	NewActivity.GUIMessage.hwnd = (HWND)GetNativeWin();
	NewActivity.GUIMessage.message = WM_SIZE;
	NewActivity.GUIMessage.wParam = 0;
	NewActivity.GUIMessage.lParam = 0; // hope no one relies on it...
	ActivityBackup = Owner->GetEvent();
	Owner->SetEvent(&NewActivity);
	Owner->HandleReSized(SIZE_RESTORED, NewSize.right, NewSize.bottom);
	Owner->SetEvent(ActivityBackup);
	if (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW) && RealFenType == 3)
		{
		// extract the ViewContext here, identify the ViewNum, and tell ViewGUI to redraw it.
		ViewContext *OriVC = NULL;
		OriVC = (ViewContext *)CustomDataVoid[0];
		GlobalApp->GUIWins->CVG->DrawImmediately(OriVC->GetVCNum());
		} // if
	} // if
else
	{
	ClearWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
	GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_UNDOCKED, WCS_FENTRACK_FLAGS_ENABLE);
	} // else

if (WinCaptionButtonBar)
	{
	PositionWinCaptionBar(LONG_MIN, LONG_MIN);
	UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_DOCK);
	} // if

} // Fenetre::InternalDockUpdate

/*===========================================================================*/

char Fenetre::DockFitOk(int Pane, int ResizeOnly)
{
short CellWidth, CellHeight;
char IsDF = 0, IsGF = 0, DidFit = 0;

IsGF = !(IsDF = !(DoYouEraseBG())); // Hint: GUIFens erase themselves

if ((!ResizeOnly) && (IsDF && GlobalApp->MainProj->ViewPorts.Inhabitants(Pane)))
	{
	// keep looking...
	} // if
else
	{ // looks good, or we're a GUIFenetre who doesn't care
	// GUIFenetres do care if they fit though...
	if (IsGF)
		{
		// do we fit?
		CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(Pane);
		CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(Pane);
		if ((CellWidth >= NominalWidth) && (CellHeight >= NominalHeight))
			{
			DidFit = 1;
			} // if
		} // if
	else
		{ // DFs fit anywhere
		DidFit = 1;
		} // else
	} // else

return(DidFit);

} // Fenetre::DockFitOk

/*===========================================================================*/

// Don't think this can be called...
void Fenetre::DockUpdate(int ResizeOnly)
{

InternalDockUpdate(ResizeOnly);

} // Fenetre::DockUpdate

/*===========================================================================*/

void GUIFenetre::DockUpdate(int ResizeOnly)
{

InternalDockUpdate(ResizeOnly);

} // GUIFenetre::DockUpdate

/*===========================================================================*/

void DrawingFenetre::DockUpdate(int ResizeOnly)
{

InternalDockUpdate(ResizeOnly);

} // DrawingFenetre::DockUpdate

/*===========================================================================*/

// This one is private, and is called only from GoPointer/EndPointer
// or from the GUIContext GoModal/EndModal.
void Fenetre::InstallPointer(unsigned char PointerType)
{

CurrentPointer = PointerType;
#ifdef _WIN32
HCURSOR TempPointer = NULL;
// Set the pointer for this window...

switch (PointerType)
	{
	case WCS_FENETRE_POINTER_WAIT:
		{
		TempPointer = LoadCursor(NULL, IDC_WAIT);
		EnableWindow((NativeDrwWin)GetNativeWin(), 0);
		break;
		} // WAIT
	case WCS_FENETRE_POINTER_NORMAL:
		{
		TempPointer = LoadCursor(NULL, IDC_ARROW);
		EnableWindow((NativeDrwWin)GetNativeWin(), 1);
		break;
		} // NORMAL
	case WCS_FENETRE_POINTER_COPY:
		{
		TempPointer = LoadCursor(LocalWinSys()->Instance(), MAKEINTRESOURCE(IDC_CURSORCOPY));
		EnableWindow((NativeDrwWin)GetNativeWin(), 1);
		break;
		} // COPY
	case WCS_FENETRE_POINTER_SWAP:
		{
		TempPointer = LoadCursor(LocalWinSys()->Instance(), MAKEINTRESOURCE(IDC_CURSORSWAP));
		EnableWindow((NativeDrwWin)GetNativeWin(), 1);
		break;
		} // SWAP
	} // switch
SetCursor(TempPointer);
#endif // _WIN32

} // Fenetre::InstallPointer

/*===========================================================================*/

void Fenetre::GoPointer(unsigned char PointerType)
{

PointerStore = PointerType;
InstallPointer(PointerType);

#ifdef _WIN32
//ShowCursor(0);
//ShowCursor(1);
#endif // _WIN32

} // Fenetre::GoPointer

/*===========================================================================*/

void Fenetre::EndPointer(void)
{

// Reset pointer to default
if ((LocalWinSys()->ModalLevel > 0) && !ModalInhibit)
	{
	InstallPointer(WCS_FENETRE_POINTER_WAIT);
	} // if
else
	{
	InstallPointer(WCS_FENETRE_POINTER_NORMAL);
	} // else

#ifdef _WIN32
//ShowCursor(0);
//ShowCursor(1);
#endif // _WIN32

} // Fenetre::EndPointer

/*===========================================================================*/

void Fenetre::SetColorPot(unsigned char PotNum,
 unsigned char PotRed, unsigned char PotGreen, unsigned char PotBlue,
 int Immediate)
{
if (PotNum < WCS_PALETTE_COLOR_SIZE)
	{
	Swatches[PotNum].peRed   = SHIFT32(PotRed);
	Swatches[PotNum].peGreen = SHIFT32(PotGreen);
	Swatches[PotNum].peBlue  = SHIFT32(PotBlue);
	#ifdef _WIN32
	// This should already be set...
	Swatches[PotNum].peFlags = PC_RESERVED;
	#endif // _WIN32
	} // if

} // Fenetre::SetColorPot

/*===========================================================================*/

void Fenetre::JumpTop(void)
{

#ifdef _WIN32
if (GetNativeWin())
	{
	SetWindowPos((NativeDrwWin)GetNativeWin(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	if (!IsIconic(LocalWinSys()->RootWin))
		{
		ShowWindow((NativeDrwWin)GetNativeWin(), SW_SHOWNORMAL);
		if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_SHOW);
		} // if
	} // if
#endif // _WIN32

} // Fenetre::JumpTop

/*===========================================================================*/

void Fenetre::JumpBottom(void)
{

// <<<>>> Never worked.

} // Fenetre::JumpBottom

/*===========================================================================*/

void Fenetre::JumpVPane(char *VP)
{

JumpVPane(GlobalApp->MainProj->ViewPorts.Matrices[GlobalApp->MainProj->Prefs.GUIConfiguration].PaneFromID(VP));

} // Fenetre::JumpVPane

/*===========================================================================*/

void Fenetre::JumpVPane(int Pane)
{
int ConfigNo;

ConfigNo = GlobalApp->MainProj->Prefs.GUIConfiguration;
//NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(ConfigNo);

} // Fenetre::JumpVPane

/*===========================================================================*/

void Fenetre::Hide(void)
{
} // Fenetre::Hide

/*===========================================================================*/

void Fenetre::Show(void)
{
} // Fenetre::Show

/*===========================================================================*/

void DrawingFenetre::Hide(void)
{

if (NativeWin) ShowWindow(NativeWin, SW_HIDE);
if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_HIDE);

} // DrawingFenetre::Hide

/*===========================================================================*/

void DrawingFenetre::Show(void)
{

if (NativeWin) ShowWindow(NativeWin, SW_SHOW);
if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_SHOW);

} // DrawingFenetre::Show

/*===========================================================================*/

void GUIFenetre::Hide(void)
{

if (NativeWin) ShowWindow(NativeWin, SW_HIDE);
if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_HIDE);
HideSubordinateWindows();

} // GUIFenetre::Hide

/*===========================================================================*/

void GUIFenetre::Show(void)
{

if (NativeWin) ShowWindow(NativeWin, SW_SHOW);
if (WinCaptionButtonBar) ShowWindow(WinCaptionButtonBar, SW_SHOW);

} // GUIFenetre::Show

/*===========================================================================*/

GUIFenetre::GUIFenetre(unsigned long WinID, WCSModule *Module, char *Title)
 : Fenetre(WinID, Module, Title)
{

StockX = StockY = 0;
RealFenType = 1;
NativeWin = NULL;
for (int PanelClear = 0; PanelClear < WCS_FENETRE_MAX_SUBPANELS; PanelClear++)
	{
	for (int PaneClear = 0; PaneClear < WCS_FENETRE_MAX_SUBPANES; PaneClear++)
		{
		SubPanels[PanelClear][PaneClear] = NULL;
		} // for
	} // for

SetWinManFlags(0);

} // GUIFenetre::GUIFenetre

/*===========================================================================*/

DrawingFenetre::DrawingFenetre(unsigned long WinID, WCSModule *Module, char *Title)
 : Fenetre(WinID, Module, Title)
{

RealFenType = 2;
NativeWin = NULL;

ClientSizeX = 0;
ClientSizeY = 0;

#ifdef _WIN32
hdc = NULL;
Paint = 0;
PixelCol = NULL;
OffScreen = NULL;
ContentBackup = NULL;
OSCBHeight = OSCBWidth = 0;
CTBitArray = NULL;
CTBASize = 0;
PutBack = NULL;

#endif // _WIN32

StoredPat = NULL;
SetWinManFlags(0);

} // DrawingFenetre::DrawingFenetre

/*===========================================================================*/

DrawingFenetre::~DrawingFenetre()
{

this->Close();

#ifdef _WIN32

if (PutBack)
	{
	DeleteObject(SelectObject(OffScreen, PutBack));
	} // if

if (OffScreen)
	{
	DeleteDC(OffScreen);
	OffScreen = NULL;
	} // if

if (ContentBackup)
	{
	DeleteObject(ContentBackup);
	ContentBackup = NULL;
	} // if

if (CTBitArray && CTBASize)
	{
	AppMem_Free(CTBitArray, CTBASize);
	CTBASize = 0;
	CTBitArray = NULL;
	} // if

#endif // _WIN32

} // DrawingFenetre::~DrawingFenetre

/*===========================================================================*/

GLDrawingFenetre::GLDrawingFenetre(unsigned long WinID, WCSModule *Module, char *Title)
 : DrawingFenetre(WinID, Module, Title)
{

RealFenType = 3;

#ifdef _WIN32
{ // test for glreadpixels workaround
GLInhibitReadPixels = 0;
if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("avoid_glreadpixels"))
	{
	GLInhibitReadPixels = 1;
	} // if
} // test for glreadpixels workaround

CBZ = NULL;

FGLSubWin = NULL;
FGLHDC = NULL;
FGLRC = NULL;
PixelFormat = NULL;
GLUHM = OGLHM = NULL;
GLON = GLSetup = 0;

dglMakeCurrent = NULL;
dglCreateContext = NULL;
dglDeleteContext = NULL;

dglLookAt = NULL;
dglPerspective = NULL;
dglBegin = NULL;
dglClear = NULL;
dglClearColor = NULL;
dglClearDepth = NULL;
dglColor3fv = NULL;
dglColor3f = NULL;
dglColorMaterial = NULL;
dglDepthFunc = NULL;
dglDrawBuffer = NULL;
dglEnable = NULL;
dglDisable = NULL;
dglEnd = NULL;
dglFlush = NULL;
dglFogf = NULL;
dglFogfv = NULL;
dglFogi = NULL;
dglLightfv = NULL;
dglLightModeli = NULL;
dglLoadIdentity = NULL;
dglMaterialfv = NULL;
dglMatrixMode = NULL;
dglPushMatrix = NULL;
dglPopMatrix = NULL;
dglFrustum = NULL;
dglViewport = NULL;
dglNormal3fv = NULL;
dglNormal3d = NULL;
dglRotated = NULL;
dglVertex3dv = NULL;
dglVertex3d = NULL;
#endif // _WIN32

} // GLDrawingFenetre::GLDrawingFenetre

/*===========================================================================*/

GLDrawingFenetre::~GLDrawingFenetre()
{

if (CBZ) delete CBZ; CBZ = NULL;

#ifdef _WIN32
if (FGLSubWin)
	{
	fglCleanup();
	DestroyWindow(FGLSubWin);
	FGLSubWin = NULL;
	} // if

if (GLUHM)
	{
	FreeLibrary(GLUHM);
	GLUHM = NULL;
	} // if
if (OGLHM)
	{
	FreeLibrary(OGLHM);
	OGLHM = NULL;
	} // if
#endif // _WIN32

} // GLDrawingFenetre::~GLDrawingFenetre

/*===========================================================================*/

GUIFenetre::~GUIFenetre()
{

if (!TestWinManFlags(WCS_FENETRE_WINMAN_NOWINLIST))
	{
	GlobalApp->MCP->RealRemoveWindowFromMenuList(this);
	} // if
this->Close();

} // GUIFenetre::~GUIFenetre

/*===========================================================================*/

NativeDrwWin DrawingFenetre::Open(Project *Proj)
{
double MatrixWidth, MatrixHeight, FitError;
short X, Y, W, H;
short DX, DY, DW, DH, MatrixScan, DidDock = 0, BestFit;
//short CellWidth, CellHeight;
long NumWindows;
//double FitCheck, AreaFrac;
Fenetre *Stash;
unsigned long ProjFlag = NULL;
#ifdef _WIN32
NativeDrawContext TempDC;
RECT Client;
#endif // _WIN32
char CheckID[4];

if (NativeWin)
	{
	JumpTop();
	} // if
else
	{
	if (Proj)
		{
		BestFit = 0;
		FitError = DBL_MAX;
		DidDock = 0;
		Proj->InquireWindowCoords(FenID, DX, DY, DW, DH);
		if ((DW == 0) || (DH == 0))
			{
			DW = 200;
			DH = 150;
			} // if
		//GetClientRect(NativeWin, &Pos);
		//DW = (short)Pos.right;
		//DH = (short)Pos.bottom;
		NominalWidth  = DW;
		NominalHeight = DH;
		GlobalApp->MainProj->InquireWindowFlags(FenID, ProjFlag);
		if (!(ProjFlag & WCS_FENTRACK_FLAGS_UNDOCKED) && (!(TestWinManFlags(WCS_FENETRE_WINMAN_NODOCK))) && (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW)))
			{
			MatrixWidth   = (double)GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
			MatrixHeight  = (double)GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);

			CheckID[0] = NULL;
			GlobalApp->MainProj->InquireWindowCell(FenID, CheckID);
			if (CheckID[0])
				{
				if ((BestFit = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, CheckID)) != -1)
					{
					// Prevent returning to a non-'View' Cell
					if (GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
						{
						// Avoid occupied cells
						if (!GlobalApp->MainProj->ViewPorts.Inhabitants(BestFit))
							{
							DidDock = 1;
							} // if
						} // if
					} // if
				else
					{
					// look for apropriate dock cell
					// fall through for now...
					} // else
				} // if
			if (!DidDock)
				{
				NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT);

				//AreaFrac = ((double)DW / MatrixWidth) * ((double)DH / MatrixHeight);
				for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
					{
					// Prefer docking in a 'View' cell
					if (GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
						{
						// Avoid occupied cells
						if (!GlobalApp->MainProj->ViewPorts.Inhabitants(MatrixScan))
							{
							// Now we take the first available 'view' cell regardless of size
							//CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(MatrixScan);
							//CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(MatrixScan);
							//if ((CellWidth >= DW) && (CellHeight >= DH))
								{
								//FitCheck = GlobalApp->MainProj->ViewPorts.GetPercentArea(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan) - AreaFrac;
								//if (FitCheck < FitError)
									{
									//FitError = FitCheck;
									BestFit = MatrixScan;
									DidDock = 1;
									break;
									} // if
								} // if
							} // if
						} // if
					} // for
				} // else
			if (DidDock)
				{
				memcpy(VPID, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), 3);
				VPID[2] = NULL;
				X = GlobalApp->MainProj->ViewPorts.GetPaneX(BestFit);
				Y = GlobalApp->MainProj->ViewPorts.GetPaneY(BestFit);
				W = GlobalApp->MainProj->ViewPorts.GetPaneW(BestFit);
				H = GlobalApp->MainProj->ViewPorts.GetPaneH(BestFit);
				GlobalApp->MainProj->ViewPorts.SetFlag(VPID, WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
				GlobalApp->MainProj->ViewPorts.MoveIn(BestFit);
				GlobalApp->MainProj->SetWindowCell(FenID, VPID, 0, 0, 0, 0);
				SetWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
				SizeFen(W, H);
				} // if
			} // if
		if (!DidDock)
			{
			memset(VPID, 0, 3);
			ClearWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
			Proj->InquireWindowCoords(FenID, X, Y, W, H);
			} // if
		} // if
	else
		{
		X = 0;
		#ifdef _WIN32
		Y = 0;
		W = 630;
		H = 480;
		#endif // _WIN32
		} // else
	#ifdef _WIN32
	if (!DidDock)
		{
		// Keep windows from disappearing offscreen forever
		LocalWinSys()->RationalizeWinCoords(X, Y, W, H);
		} // if

	{
	POINT XLate;
	XLate.x = X;
	XLate.y = Y;
	char *WindowClassName;
	if (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW))
		{ // use a camera-like icon
		WindowClassName = APP_CLASSPREFIX ".GLDrawingWin";
		} // if
	else
		{
		WindowClassName = APP_CLASSPREFIX ".DrawingWin";
		} // else
	ClientToScreen(LocalWinSys()->RootWin, &XLate);
     NativeWin = CreateWindowEx(LocalWinSys()->InquireWindowStyle(), WindowClassName,
	    FenTitle,
        WS_POPUP | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,     // window style
        XLate.x, XLate.y, W, H,
        LocalWinSys()->RootWin,   // parent/owner window handle
        NULL,                    // window menu handle
        (HINSTANCE)LocalWinSys()->Instance(), // program instance handle
	    NULL) ;				     // creation parameters
	} // temp scope
	// Adjust to preferred actual size
	#endif // _WIN32
	if (NativeWin)
		{
		if ((ClientSizeX != 0) && (ClientSizeY != 0))
			{
			SetDrawingAreaSize(ClientSizeX, ClientSizeY);
			} // if
		OriX = X; OriY = Y;
		SizeX = W; SizeY = H;
		Stash = this; // Convert to *Fenetre, not obvious
		#ifdef _WIN32
		SetWindowLong(NativeWin, GWL_USERDATA, (LONG)Stash);
		TempDC = GetDC(NativeWin);
		GetClientRect(NativeWin, &Client);
		//if (ContentBackup = CreateCompatibleBitmap(TempDC, Client.right, Client.bottom))
		if (ContentBackup = CreateDIB24(TempDC, Client.right, Client.bottom))
			{
			if (OffScreen = CreateCompatibleDC(TempDC))
				{
				SelectObject(OffScreen, ContentBackup);
				OSCBWidth = W;
				OSCBHeight = H;
				} // if
			} // if
		ReleaseDC(NativeWin, TempDC);
		TempDC = NULL;
		ResyncWinMan(~((unsigned long)0));
		if (!IsIconic(LocalWinSys()->RootWin))
			{
			ShowWindow(NativeWin, SW_SHOWNORMAL);
			if (!DidDock)
				{
				JumpTop();
				} // if
			} // if
		if (OffScreen)
			{
			// Clear the Bitmap...
			SelectObject(OffScreen, GetStockObject(BLACK_BRUSH));
			SelectObject(OffScreen, GetStockObject(NULL_PEN));
			Rectangle(OffScreen, 0, 0, W + 1, H + 1);
			} // if
		UpdateWindow(NativeWin); // sends WM_PAINT
		#endif // _WIN32
		LocalWinSys()->RegisterFen(this);
		if (Proj)
			Proj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_ENABLE);
		CreateWinCaptionBar();
		} // if
	} // else
return(NativeWin);

} // DrawingFenetre::Open

/*===========================================================================*/

NativeDrwWin GLDrawingFenetre::Open(Project *Proj, GLDrawingFenetre *ShareWith)
{
NativeDrwWin ParentWin;
Fenetre *Stash;
int GLW, GLH;
RECT Size;

Stash = this; // Convert to *Fenetre, not obvious

if (ParentWin = DrawingFenetre::Open(Proj))
	{
	GLW = SizeX;
	GLH = SizeY;
	#ifdef _WIN32
	if (NativeWin)
		{
		GetClientRect(NativeWin, &Size);
		GLW = Size.right;
		GLH = Size.bottom;
		} // if
	FGLSubWin = CreateWindow(APP_CLASSPREFIX ".GLWin",
	 NULL,
	 WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,     // window style
	 0, 0, GLW, GLH,
	 ParentWin,   // parent/owner window handle
	 NULL,                    // window menu handle
	 (HINSTANCE)LocalWinSys()->Instance(), // program instance handle
	 NULL) ;				     // creation parameters

	if (FGLSubWin)
		{
		SetWindowLong(FGLSubWin, GWL_USERDATA, (LONG)Stash);
		} // if
	#endif // _WIN32
	} // if

return(ParentWin);

} // GLDrawingFenetre::Open

/*===========================================================================*/

NativeGUIWin GUIFenetre::Open(Project *Proj)
{
double FitCheck, MatrixWidth, MatrixHeight, AreaFrac, FitError;
Fenetre *Stash;
long NumWindows;
unsigned long ProjFlag;
#ifdef _WIN32
RECT Pos;
#endif // _WIN32
short X, Y, W, H;
short DW, DH, CellWidth, CellHeight, MatrixScan, DidDock = 0, BestFit;
char CheckID[4];

if (NativeWin)
	{
	JumpTop();
	} // if
else
	{
	NativeWin = Construct();
	if (NativeWin)
		{
		if (!(TestWinManFlags(WCS_FENETRE_WINMAN_ISSUBDIALOG)))
			{ // skip all app-toplevel window work
			// figure out if we'll need a Custom Close Button
			long Style = GetWindowLong((HWND)GetNativeWin(), GWL_STYLE);

			if (g_xpStyle.IsAppThemed() && !LocalWinSys()->InquireIsVista()) // this excludes Win2k, XP-non-themed and all Vista
				{
				if (Style & WS_SYSMENU) // do we even want a Close button?
					{
					CustomCloseButton = true;
					} // if
				} // if

			//Proj->InquireWindowCoords(FenID, DX, DY, DW, DH);
			GetWindowRect(NativeWin, &Pos);
			NominalWidth  = (short)(Pos.right - Pos.left);
			NominalHeight = (short)(Pos.bottom - Pos.top);
			GetClientRect(NativeWin, &Pos);
			DW = (short)(Pos.right - Pos.left);
			DH = (short)(Pos.bottom - Pos.top);
			if (Proj)
				{
				BestFit = 0;
				FitError = DBL_MAX;
				DidDock = 0;
				GlobalApp->MainProj->InquireWindowFlags(FenID, ProjFlag);
				if (!(ProjFlag & WCS_FENTRACK_FLAGS_UNDOCKED) && !(TestWinManFlags(WCS_FENETRE_WINMAN_NODOCK)) )
					{
					MatrixWidth   = (double)GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
					MatrixHeight  = (double)GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);

					CheckID[0] = NULL;
					GlobalApp->MainProj->InquireWindowCell(FenID, CheckID);
					if (CheckID[0])
						{
						if ((BestFit = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, CheckID)) != -1)
							{
							// Prevent returning to a 'View' Cell
							if (!GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
								{
								// Avoid occupied cells
								if (!GlobalApp->MainProj->ViewPorts.Inhabitants(BestFit))
									{
									DidDock = 1;
									} // if
								} // if
							} // if
						else
							{
							// look for apropriate dock cell
							// fall through for now...
							} // else
						} // if
					if (!DidDock)
						{
						NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT);

						AreaFrac = ((double)DW / MatrixWidth) * ((double)DH / MatrixHeight);
						for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
							{
							// Prevent docking in a 'View' cell
							if (!GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
								{
								CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(MatrixScan);
								CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(MatrixScan);
								if ((CellWidth >= DW) && (CellHeight >= DH))
									{
									FitCheck = GlobalApp->MainProj->ViewPorts.GetPercentArea(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan) - AreaFrac;
									if (FitCheck < FitError)
										{
										// Avoid occupied cells
										if (!GlobalApp->MainProj->ViewPorts.Inhabitants(MatrixScan))
											{
											FitError = FitCheck;
											BestFit = MatrixScan;
											DidDock = 1;
											} // if
										} // if
									} // if
								} // if
							} // for
						} // if
					if (!DidDock) // try again, occupied cells ok this time
						{
						NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT);

						AreaFrac = ((double)DW / MatrixWidth) * ((double)DH / MatrixHeight);
						for (MatrixScan = 0; MatrixScan < NumWindows; MatrixScan++)
							{
							// Prevent docking in a 'View' cell
							if (!GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
								{
								CellWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(MatrixScan);
								CellHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(MatrixScan);
								if ((CellWidth >= DW) && (CellHeight >= DH))
									{
									FitCheck = GlobalApp->MainProj->ViewPorts.GetPercentArea(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, MatrixScan) - AreaFrac;
									if (FitCheck < FitError)
										{
										// Avoid occupied cells
										//if (!GlobalApp->MainProj->ViewPorts.Inhabitants(MatrixScan))
											{
											FitError = FitCheck;
											BestFit = MatrixScan;
											DidDock = 1;
											} // if
										} // if
									} // if
								} // if
							} // for
						} // if
					if (DidDock)
						{
						memcpy(VPID, GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, BestFit), 3);
						VPID[2] = NULL;
						X = GlobalApp->MainProj->ViewPorts.GetPaneX(BestFit);
						Y = GlobalApp->MainProj->ViewPorts.GetPaneY(BestFit);
						W = GlobalApp->MainProj->ViewPorts.GetPaneW(BestFit);
						H = GlobalApp->MainProj->ViewPorts.GetPaneH(BestFit);
						GlobalApp->MainProj->ViewPorts.MoveIn(BestFit);
						MoveAndSizeFen(X, Y, W, H);
						// vista window-frame "Glass" padding size compensation
#if (NTDDI_VERSION >= NTDDI_VISTA)
							{
							RECT RealRECT;
							if(LocalWinSys()->InquireTrueWindowRect((HWND)GetNativeWin(), &RealRECT))
								{
								POINT CoordinateTranslateUL, CoordinateTranslateLR;
								LONG RealWidth, RealHeight;
								CoordinateTranslateUL.x = RealRECT.left;
								CoordinateTranslateUL.y = RealRECT.top;
								CoordinateTranslateLR.x = RealRECT.right;
								CoordinateTranslateLR.y = RealRECT.bottom;
								ScreenToClient(LocalWinSys()->RootWin, &CoordinateTranslateUL);
								ScreenToClient(LocalWinSys()->RootWin, &CoordinateTranslateLR);
								RealWidth  = (CoordinateTranslateLR.x - CoordinateTranslateUL.x);
								RealHeight = (CoordinateTranslateLR.y - CoordinateTranslateUL.y);
								if(X != CoordinateTranslateUL.x || Y != CoordinateTranslateUL.y || W != RealWidth || H != RealHeight)
									{ // mismatch due to Vista adjustments, re-adjust with the difference and re-size
									SizeCorrectX = (X - CoordinateTranslateUL.x);
									SizeCorrectY = (Y - CoordinateTranslateUL.y);
									X += (short)SizeCorrectX;
									Y += (short)SizeCorrectY;
									W -= (short)(SizeCorrectX * 2);
									H -= (short)(SizeCorrectY * 2);
									MoveAndSizeFen(X, Y, W, H); // apply adjusted dimensions
									} // if
								} // if
							} // Vista-only code scope
#endif // NTDDI_VERSION >= NTDDI_VISTA
						GlobalApp->MainProj->SetWindowCell(FenID, VPID, X, Y, W, H);
						SetWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
						} // if
					} // if
				if (!DidDock)
					{
					memset(VPID, 0, 3);
					ClearWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
					SetWindowLong(NativeWin, GWL_EXSTYLE, GetWindowLong(NativeWin, GWL_EXSTYLE) | LocalWinSys()->InquireWindowStyle());
					Proj->InquireWindowCoords(FenID, X, Y, W, H);
					} // if

				} // if
			else
				{
				X = 0; Y = 0;
				W = 500; H = 300;
				} // else
			} // if not SUBDIALOG

		// *Fenetre is not same as *GUIFenetre, this converts for us
		Stash = this;

		#ifdef _WIN32
		// Vitally important to SetWindowLong...
		SetWindowLong(NativeWin, GWL_USERDATA, (LONG)Stash);
		//SetClassLong(NativeWin, GCL_HICON, (long)LoadIcon((HINSTANCE)LocalWinSys()->Instance(), MAKEINTRESOURCE(IDI_WGUIFEN)));
		//SetClassLong(NativeWin, GCL_HCURSOR, NULL);
		#endif // _WIN32

		if (!(TestWinManFlags(WCS_FENETRE_WINMAN_ISSUBDIALOG)))
			{ // skip all app-toplevel window work
			#ifdef _WIN32
			SetWindowText(NativeWin, FenTitle);
			GetWindowRect(NativeWin, &Pos);
			short MoveW = (short)(Pos.right - Pos.left);
			short MoveH = (short)(Pos.bottom - Pos.top);
			LocalWinSys()->RationalizeWinCoords(X, Y, MoveW, MoveH);
			MoveFen(X, Y);
			ResyncWinMan(~((unsigned long)0));
			// record initial size for possible use as minimum during resize
			RECT InitialSizeRect;
			GetWindowRect(NativeWin, &InitialSizeRect);
			StockX = (int)(InitialSizeRect.right - InitialSizeRect.left);
			StockY = (int)(InitialSizeRect.bottom - InitialSizeRect.top);
			if (!DidDock && TestWinManFlags(WCS_FENETRE_WINMAN_GUIFENSIZE) && W != 0 && H != 0) // should we obey and restore stored size?
				{
				// adjust stored client size to full nonclient size that SetWindowPos wants
				RECT TransformRect;
				TransformRect.left = X;
				TransformRect.top = Y;
				TransformRect.right = X + W;
				TransformRect.bottom = Y + H;
				long Style = GetWindowLong(NativeWin, GWL_STYLE);
				long ExStyle = GetWindowLong(NativeWin, GWL_EXSTYLE);
				AdjustWindowRectEx(&TransformRect, Style, false, ExStyle);
				SetWindowPos(NativeWin, NULL, 0, 0, TransformRect.right - TransformRect.left, TransformRect.bottom - TransformRect.top, SWP_NOMOVE | SWP_NOZORDER);
				Owner->HandleReSized(SIZE_RESTORED, W, H);
				} // if
			else
				{
				// Invoke HandleResized(), so that resizable windows adjust to their current initial size
				RECT NewSize;
				GetClientRect((HWND)GetNativeWin(), &NewSize);
				Owner->HandleReSized(SIZE_RESTORED, NewSize.right, NewSize.bottom);
				} // else
			if (!IsIconic(LocalWinSys()->RootWin))
				{
	//			if (GetDlgItem(NativeWin, ID_BUSYWIN_COMPLETE))
	//				{
	//				ShowWindow(NativeWin, SW_SHOWNOACTIVATE);
	//				} // if
	//			else
	//				{
					ShowWindow(NativeWin, SW_SHOWNORMAL);
	//				} // if
				} // if
			else
				{
				ShowWindow(NativeWin, SW_SHOWMINNOACTIVE);
				} // if
			#endif // _WIN32
			LocalWinSys()->RegisterFen(this);
			if (Proj)
				Proj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_ENABLE);
			CreateWinCaptionBar();
			JumpTop();
			InvalidateRect(NativeWin, NULL, 1);

			if (!TestWinManFlags(WCS_FENETRE_WINMAN_NOWINLIST))
				{
				GlobalApp->MCP->RealAddWindowToMenuList(this);
				} // if
			} // if not SUBDIALOG
		} // if
	} // else

return(NativeWin);

} // GUIFenetre::Open

/*===========================================================================*/

void GUIFenetre::Close(void)
{
int Left, Top;
int Width = 0, Height = 0;
short MyCell;

if (NativeWin)
	{
	#ifdef _WIN32
	FindWindowOrigin(NativeWin, Left, Top);
	#endif // _WIN32
	if (GlobalApp->MainProj)
		{
		if (VPID[0])
			{
			// Move out
			MyCell = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, VPID);
			GlobalApp->MainProj->ViewPorts.MoveOut(MyCell);
			// Preserve dock cell
			GlobalApp->MainProj->SetWindowCell(FenID, VPID, Left, Top, Width, Height);
			VPID[0] = NULL;
			} // if
		//GlobalApp->MainProj->SetWindowCoords(FenID, Left, Top, Width, Height);
		GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_DISABLE);
		// VPID
		} // if

	LocalWinSys()->DeRegFen(this);
	#ifdef _WIN32
	// So that DefWinProc will get called, and not perhaps an invalid AppModule
	SetWindowLong(NativeWin, GWL_USERDATA, NULL);
	#endif // _WIN32
	Destruct();
	} // if
} // GUIFenetre::Close

/*===========================================================================*/

void GUIFenetre::Destruct(void)
{
if (NativeWin)
	{
	#ifdef _WIN32
	int KillPanel, KillPane;

	// Ensure proper KillFocus
	SetFocus(NULL);
	// Destroy (possibly unlinked) subpanels
	for (KillPanel = 0; KillPanel < WCS_FENETRE_MAX_SUBPANELS; KillPanel++)
		{
		for (KillPane = 0; KillPane < WCS_FENETRE_MAX_SUBPANES; KillPane++)
			{
			if (SubPanels[KillPanel][KillPane])
				{
				DestroyWindow(SubPanels[KillPanel][KillPane]);
				SubPanels[KillPanel][KillPane] = NULL;
				} //if
			} // for
		} // for

	if (WinCaptionButtonBar)
		DestroyWindow(WinCaptionButtonBar);
	WinCaptionButtonBar = NULL;

	DestroyWindow(NativeWin);
	NativeWin = NULL;
	#endif // _WIN32
	} // if

} // GUIFenetre::Destruct

/*===========================================================================*/

void DrawingFenetre::Close(void)
{
#ifdef _WIN32
NativeDrawContext TempDC;
#endif // _WIN32
int Left, Top;
unsigned short FW, FH;
short MyCell;

if (NativeWin)
	{
	// Update Win coords with project...
	GetDrawingAreaSize(FW, FH);
	#ifdef _WIN32
	FindWindowOrigin(NativeWin, Left, Top);
	#endif // _WIN32
	// Move out
	if (VPID[0])
		{
		MyCell = GlobalApp->MainProj->ViewPorts.GetPaneFromID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, VPID);
		GlobalApp->MainProj->ViewPorts.MoveOut(MyCell);
		GlobalApp->MainProj->SetWindowCell(FenID, VPID, Left, Top, FW, FH);
		VPID[0] = NULL;
		} // if
	//GlobalApp->MainProj->SetWindowCoords(FenID, Left, Top, FW, FH);
	GlobalApp->MainProj->SetWindowFlags(FenID, WCS_FENTRACK_FLAGS_OPEN, WCS_FENTRACK_FLAGS_DISABLE);
	LocalWinSys()->DeRegFen(this);
	#ifdef _WIN32
	TempDC = GetDC(NativeWin);
	ReleaseDC(NativeWin, TempDC);

	if (WinCaptionButtonBar)
		DestroyWindow(WinCaptionButtonBar);
	WinCaptionButtonBar = NULL;


	DestroyWindow(NativeWin);
	NativeWin = NULL;
	#endif // _WIN32
	} // if

} // DrawingFenetre::Close

/*===========================================================================*/

void DrawingFenetre::CaptureInput(void)
{

if (NativeWin)
	{
	SetCapture(NativeWin);
	} // if

} // DrawingFenetre::CaptureInput

/*===========================================================================*/

void DrawingFenetre::ReleaseInput(void)
{

ReleaseCapture();

} // DrawingFenetre::ReleaseInput

/*===========================================================================*/

void DrawingFenetre::Clear(void)
{
#ifdef _WIN32
RECT ClearArea;
HGDIOBJ ClearBsh, OldBsh, OldPen, NoPen;
GetClientRect(NativeWin, &ClearArea);

ClearBsh = GetStockObject(BLACK_BRUSH);
NoPen = GetStockObject(NULL_PEN);
OldBsh = SelectObject(hdc, ClearBsh);
SelectObject(OffScreen, ClearBsh);
OldPen = SelectObject(hdc, NoPen);
SelectObject(OffScreen, NoPen);
Rectangle(hdc, 0, 0, ClearArea.right + 1, ClearArea.bottom + 1);
Rectangle(OffScreen, 0, 0, ClearArea.right + 1, ClearArea.bottom + 1);
SelectObject(hdc, OldBsh);
SelectObject(OffScreen, OldBsh);
SelectObject(hdc, OldPen);
SelectObject(OffScreen, OldPen);
#endif // _WIN32

} // DrawingFenetre::Clear

/*===========================================================================*/

void DrawingFenetre::ClearFG(unsigned short X, unsigned short Y, short Width, short Height)
{
#ifdef _WIN32
RECT ClearArea;
HGDIOBJ ClearBsh, OldBsh, OldPen, NoPen;
GetClientRect(NativeWin, &ClearArea);
#endif // _WIN32

if ((X == 0) && (Y == 0) && (Width == -1) && (Height == -1))
    {
#ifdef _WIN32
	X = 0;
	Y = 0;
	Width  = (short)(ClearArea.right + 1);
	Height = (short)(ClearArea.bottom + 1);
#endif // _WIN32
	} // else

#ifdef _WIN32
ClearBsh = GetStockObject(BLACK_BRUSH);
NoPen = GetStockObject(NULL_PEN);
OldBsh = SelectObject(hdc, ClearBsh);
OldPen = SelectObject(hdc, NoPen);
Rectangle(hdc, X, Y, Width, Height);
SelectObject(hdc, OldBsh);
SelectObject(hdc, OldPen);
#endif // _WIN32

} // DrawingFenetre::ClearFG

/*===========================================================================*/

void DrawingFenetre::ClearBackup(void)
{
#ifdef _WIN32
RECT ClearArea;
HGDIOBJ ClearBsh, OldBsh, OldPen, NoPen;
GetClientRect(NativeWin, &ClearArea);


ClearBsh = GetStockObject(BLACK_BRUSH);
NoPen = GetStockObject(NULL_PEN);
OldBsh = SelectObject(OffScreen, ClearBsh);
OldPen = SelectObject(OffScreen, NoPen);
Rectangle(OffScreen, 0, 0, ClearArea.right + 1, ClearArea.bottom + 1);
SelectObject(OffScreen, OldBsh);
SelectObject(OffScreen, OldPen);
#endif // _WIN32

} // DrawingFenetre::ClearBackup

/*===========================================================================*/

void DrawingFenetre::GetDrawingAreaSize(unsigned short &Width, unsigned short &Height)
{
#ifdef _WIN32
RECT Bounds;

GetClientRect(NativeWin, &Bounds);

Width  = (unsigned short)Bounds.right;
Height = (unsigned short)Bounds.bottom;

#endif // _WIN32

} // DrawingFenetre::GetDrawingAreaSize

/*===========================================================================*/

void DrawingFenetre::SetDrawingAreaSize(unsigned short Width, unsigned short Height)
{
unsigned short OldWidth, OldHeight;
signed short NewWidth, NewHeight;
#ifdef _WIN32
RECT Bounds;

if (NativeWin)
	{

	GetWindowRect(NativeWin, &Bounds);

	GetDrawingAreaSize(OldWidth, OldHeight);
	NewWidth = (signed short)Width - (signed short)OldWidth;
	NewHeight = (signed short)Height - (signed short)OldHeight;

	NewWidth  += (unsigned short)(Bounds.right  - Bounds.left);
	NewHeight += (unsigned short)(Bounds.bottom - Bounds.top );

	SetWindowPos(NativeWin, NULL, 0, 0, NewWidth, NewHeight, SWP_NOMOVE | SWP_NOZORDER);
	#endif // _WIN32
	} // if
else
	{
	ClientSizeX = Width;
	ClientSizeY = Height;
	} // else

} // DrawingFenetre::SetDrawingAreaSize

/*===========================================================================*/

void DrawingFenetre::SetTitle(const char *NewTitle)
{

strncpy(FenTitle, NewTitle, 255); FenTitle[255] = NULL;
if (NativeWin)
	{
	#ifdef _WIN32
	SetWindowText(NativeWin, FenTitle);
	#endif // _WIN32
	} // if

} // DrawingFenetre::SetTitle

/*===========================================================================*/

int DrawingFenetre::GetTitle(char *FetchTitle, int FetchLen)
{

if (NativeWin)
	{
	#ifdef _WIN32
	// supposedly returns the length of the string copied
	return (GetWindowText(NativeWin, FetchTitle, FetchLen));
	#endif // _WIN32
	} // if
return (0);

} // DrawingFenetre::GetTitle

/*===========================================================================*/

void GUIFenetre::SetTitle(const char *NewTitle)
{

strncpy(FenTitle, NewTitle, 255); FenTitle[255] = NULL;
if (NativeWin)
	{
	#ifdef _WIN32
	SetWindowText(NativeWin, FenTitle);
	#endif // _WIN32
	} // if

} // GUIFenetre::SetTitle

/*===========================================================================*/

int GUIFenetre::GetTitle(char *FetchTitle, int FetchLen)
{

if (NativeWin)
	{
	#ifdef _WIN32
	// supposedly returns the length of the string copied
	return (GetWindowText(NativeWin, FetchTitle, FetchLen));
	#endif // _WIN32
	} // if
return (0);

} // GUIFenetre::GetTitle

/*===========================================================================*/

void GUIFenetre::ShowPanel(int Panel, int Pane)
{
// -1 for Pane number means hide all panes
#ifdef _WIN32
for (int PaneLoop = 0; PaneLoop < WCS_FENETRE_MAX_SUBPANES; PaneLoop++)
	{
	if (SubPanels[Panel][PaneLoop])
		{
		if (PaneLoop == Pane)
			{
			ShowWindow(SubPanels[Panel][PaneLoop], SW_SHOWNORMAL);
			} // if
		else
			{
			ShowWindow(SubPanels[Panel][PaneLoop], SW_HIDE);
			} // else
		} // if
	else
		{
		break;
		} // else
	} // for
#endif // _WIN32
} // GUIFenetre::ShowPanel

/*===========================================================================*/

void GUIFenetre::ShowPanelAsPopDrop(WIDGETID WidgetID, int Panel, int Pane, NativeGUIWin Host)
{
RECT HostWinRECT, InvokingWidgetRECT, PopDropRECT;
int NewPopDropX, NewPopDropY;

if (Host == NULL)
	{
	Host = NativeWin;
	} // if

// figure out where everyone is
GetWindowRect(Host, &HostWinRECT);
GetWindowRect(GetWidgetFromID(WidgetID), &InvokingWidgetRECT);
GetClientRect(SubPanels[Panel][Pane], &PopDropRECT);

// position horizontally centered on host win, vertically below invoking widget
// we're working in screen coordinates, since we're a WS_POPUP and not a WS_CHILD
NewPopDropX = ((HostWinRECT.right + HostWinRECT.left) / 2) - (PopDropRECT.right / 2);
NewPopDropY = InvokingWidgetRECT.bottom;

SetWindowPos(SubPanels[Panel][Pane], HWND_TOP, NewPopDropX, NewPopDropY, 0, 0, SWP_NOSIZE);

// show it
ShowPanel(Panel, Pane);
} // GUIFenetre::ShowPanelAsPopDrop

/*===========================================================================*/

bool GUIFenetre::IsPaneDisplayed(int Panel, int Pane)
{
if (SubPanels[Panel][Pane])
	{
	long Style;
	Style = GetWindowLong(SubPanels[Panel][Pane], GWL_STYLE);
	if (Style & WS_VISIBLE)
		{
		return(true);
		} // if
	} // if

return(false);
} // GUIFenetre::IsPaneDisplayed

/*===========================================================================*/

void Fenetre::CueRedraw(void)
{
#ifdef _WIN32
RECT Invalid;

GetClientRect((HWND)GetNativeWin(), &Invalid);
InvalidateRect((HWND)GetNativeWin(), &Invalid, 1);
#endif // _WIN32

} // Fenetre::CueRedraw

/*===========================================================================*/

void DrawingFenetre::SetupForDrawing(void)
{
#ifdef _WIN32
if (!hdc)
	{
	hdc = GetDC(NativeWin);
	SetBkMode(hdc, TRANSPARENT);
	} // if

LocalWinSys()->ColorControl->WInstantiate(this);

#endif // _WIN32

} // DrawingFenetre::SetupForDrawing

/*===========================================================================*/

void DrawingFenetre::CleanupFromDrawing(void)
{

#ifdef _WIN32
if (hdc)
	{
	ReleaseDC(NativeWin, hdc);
	hdc = NULL;
	} // if
#endif // _WIN32

} // DrawingFenetre::CleanupFromDrawing

/*===========================================================================*/

void GLDrawingFenetre::SetupForDrawing(void)
{

// If in GL mode, Switch to GDI and blit GL contents forward
if (GLON)
	{
	GoGDI();
	} // if

DrawingFenetre::SetupForDrawing();

} // GLDrawingFenetre::SetupForDrawing

/*===========================================================================*/

void GLDrawingFenetre::CleanupFromDrawing(void)
{

DrawingFenetre::CleanupFromDrawing();

} // GLDrawingFenetre::CleanupFromDrawing

/*===========================================================================*/

void GUIFenetre::UpdateModal(void)
{
if ((LocalWinSys()->ModalLevel > 0) && !ModalInhibit)
	{
	#ifdef _WIN32
	InstallPointer(WCS_FENETRE_POINTER_WAIT);
	#endif // _WIN32
	} // if
else
	{
	#ifdef _WIN32
	InstallPointer(PointerStore);
	if (GlobalApp && GlobalApp->MCP && ((GUIFenetre *)GlobalApp->MCP == this))
		{
		InvalidateRect(GlobalApp->WinSys->RootWin, NULL, 1);
		} // if
	#endif // _WIN32
	} // else

} // GUIFenetre::UpdateModal

/*===========================================================================*/

void DrawingFenetre::UpdateModal(void)
{

if ((LocalWinSys()->ModalLevel > 0) && !ModalInhibit)
	{
	InstallPointer(WCS_FENETRE_POINTER_WAIT);
	} // if
else
	{
	InstallPointer(PointerStore);
	} // else

} // DrawingFenetre::UpdateModal

/*===========================================================================*/

void Fenetre::GoModal(void)
{
// increment local modal count, gomodal if it was 0
if (!(ModalInhibit++))
	{
	LocalWinSys()->GoModal();
	}
} // Fenetre::GoModal

/*===========================================================================*/

void Fenetre::EndModal(void)
{
// Decrement local modal count and endmodal if 0
if (ModalInhibit) ModalInhibit--;
if (!ModalInhibit) LocalWinSys()->EndModal();
} // Fenetre::EndModal

/*===========================================================================*/

void DrawingFenetre::InstallColorHook(PenThing NewColor)
{
#ifdef _WIN32
if (OffScreen)
	{
	SelectObject(OffScreen, NewColor);
	} // if
if (hdc)
	{
	SelectObject(hdc, NewColor);
	} // if
return;
#endif // _WIN32
} // DrawingFenetre::InstallColorHook

/*===========================================================================*/

void GUIFenetre::InstallColorHook(PenThing NewColor)
{
return;
} // GUIFenetre::InstallColorHook

/*===========================================================================*/

void *DrawingFenetre::GetNativeWin(void) const
{
return((void *)NativeWin);
} // DrawingFenetre::GetNativeWin

/*===========================================================================*/

void *GUIFenetre::GetNativeWin(void) const
{
#ifdef _WIN32
return((void *)NativeWin);
#endif // _WIN32
} // GUIFenetre::GetNativeWin

/*===========================================================================*/

NativeControl GUIFenetre::GetWidgetFromID(WIDGETID WidgetID)
{

#ifdef _WIN32
NativeControl FoundControl = NULL;
int PanelLoop, PaneLoop;

if (!NativeWin) return(NULL);

if (!(FoundControl = GetDlgItem(NativeWin, WidgetID)))
	{
	for (PanelLoop = 0; ! FoundControl && PanelLoop < WCS_FENETRE_MAX_SUBPANELS; PanelLoop++)
		{
		for (PaneLoop = 0; PaneLoop < WCS_FENETRE_MAX_SUBPANES; PaneLoop++)
			{
			if (SubPanels[PanelLoop][PaneLoop])
				{
				if (FoundControl = GetDlgItem(SubPanels[PanelLoop][PaneLoop], WidgetID))
					{
					break;
					} // if 
				} // if
			else
				{
				break;
				} // else
			} // for
		} // for
	} // if

return(FoundControl);
#endif // _WIN32

} // GUIFenetre::GetWidgetFromID

/*===========================================================================*/

void GUIFenetre::InstallPointer(unsigned char PointerType)
{
Fenetre::InstallPointer(PointerType);
} // GUIFenetre::InstallPointer

#ifdef UNUSED_CODE

//lint -save -e747
void DrawingFenetre::DrawTempEllipse12(VectorClipper *VC, int CX, int CY,
 unsigned short RX, unsigned short RY, short Erase, unsigned char DefPenDesc)
{
int ASizeX, BSizeX, CSizeX;
int ASizeY, BSizeY, CSizeY;

ASizeX = (int)((float)RX * .38268343);
BSizeX = (int)((float)RX * .70710678);
CSizeX = (int)((float)RX * .92387953);

if (RY == RX)
	{
	ASizeY = ASizeX;
	BSizeY = BSizeX;
	CSizeY = CSizeX;
	} // if
else
	{
	ASizeY = (int)((float)RY * .38268343);
	BSizeY = (int)((float)RY * .70710678);
	CSizeY = (int)((float)RY * .92387953);
	} // if

/* 12 O'Clock to 3 O'Clock */
TempLineClip(VC, CX, CY - RY, CX + ASizeX, CY - CSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + ASizeX, CY - CSizeY, CX + BSizeX, CY - BSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + BSizeX, CY - BSizeY, CX + CSizeX, CY - ASizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + CSizeX, CY - ASizeY, CX + RX, CY, Erase, DefPenDesc);


/* 3 O'Clock to 6 O'Clock */
TempLineClip(VC, CX + RX, CY, CX + CSizeX, CY + ASizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + CSizeX, CY + ASizeY, CX + BSizeX, CY + BSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + BSizeX, CY + BSizeY, CX + ASizeX, CY + CSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX + ASizeX, CY + CSizeY, CX, CY + RY, Erase, DefPenDesc);


/* 6 O'Clock to 9 O'Clock */
TempLineClip(VC, CX, CY + RY, CX - ASizeX, CY + CSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - ASizeX, CY + CSizeY, CX - BSizeX, CY + BSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - BSizeX, CY + BSizeY, CX - CSizeX, CY + ASizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - CSizeX, CY + ASizeY, CX - RX, CY, Erase, DefPenDesc);


/* 9 O'Clock to 12 O'Clock */
TempLineClip(VC, CX - RX, CY, CX - CSizeX, CY - ASizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - CSizeX, CY - ASizeY, CX - BSizeX, CY - BSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - BSizeX, CY - BSizeY, CX - ASizeX, CY - CSizeY, Erase, DefPenDesc);
TempLineClip(VC, CX - ASizeX, CY - CSizeY, CX, CY - RY, Erase, DefPenDesc);

} // DrawingFenetre::DrawTempEllipse12
//lint -restore

#endif // UNUSED_CODE

/*===========================================================================*/

void DrawingFenetre::LineClip(VectorClipper *VC, double XS, double YS, double XE, double YE)
{

if (VC->ClipSeg(XS, YS, XE, YE))
	{
	DrawLine((unsigned short)XS, (unsigned short)YS, (unsigned short)XE, (unsigned short)YE);
	} // if

} // DrawingFenetre::LineClip()

/*===========================================================================*/

void DrawingFenetre::TempLineClip(VectorClipper *VC, double XS, double YS, double XE, double YE, short Erase, unsigned char DefPenDesc)
{
int DrawIt = 1;

if (VC)
	{
	DrawIt = VC->ClipSeg(XS, YS, XE, YE);
	} // 

if (DrawIt)
	{
	if (Erase)
		EraseTempLine((unsigned short)XS, (unsigned short)YS, (unsigned short)XE, (unsigned short)YE);
	else
		DrawTempLine((unsigned short)XS, (unsigned short)YS, (unsigned short)XE, (unsigned short)YE, DefPenDesc);
	} // if

} // DrawingFenetre::TempLineClip()

/*===========================================================================*/

#ifdef UNUSED_CODE

//lint -save -e747
void DrawingFenetre::DrawEllipse12(VectorClipper *VC, int CX, int CY,
 unsigned short RX, unsigned short RY)
{
int ASizeX, BSizeX, CSizeX;
int ASizeY, BSizeY, CSizeY;

ASizeX = (int)((float)RX * .38268343F);
BSizeX = (int)((float)RX * .70710678F);
CSizeX = (int)((float)RX * .92387953F);

if (RY == RX)
	{
	ASizeY = ASizeX;
	BSizeY = BSizeX;
	CSizeY = CSizeX;
	} // if
else
	{
	ASizeY = (int)((float)RY * .38268343F);
	BSizeY = (int)((float)RY * .70710678F);
	CSizeY = (int)((float)RY * .92387953F);
	} // if

/* 12 O'Clock to 3 O'Clock */
LineClip(VC, CX, CY - RY, CX + ASizeX, CY - CSizeY);
LineClip(VC, CX + ASizeX, CY - CSizeY, CX + BSizeX, CY - BSizeY);
LineClip(VC, CX + BSizeX, CY - BSizeY, CX + CSizeX, CY - ASizeY);
LineClip(VC, CX + CSizeX, CY - ASizeY, CX + RX, CY);


/* 3 O'Clock to 6 O'Clock */
LineClip(VC, CX + RX, CY, CX + CSizeX, CY + ASizeY);
LineClip(VC, CX + CSizeX, CY + ASizeY, CX + BSizeX, CY + BSizeY);
LineClip(VC, CX + BSizeX, CY + BSizeY, CX + ASizeX, CY + CSizeY);
LineClip(VC, CX + ASizeX, CY + CSizeY, CX, CY + RY);


/* 6 O'Clock to 9 O'Clock */
LineClip(VC, CX, CY + RY, CX - ASizeX, CY + CSizeY);
LineClip(VC, CX - ASizeX, CY + CSizeY, CX - BSizeX, CY + BSizeY);
LineClip(VC, CX - BSizeX, CY + BSizeY, CX - CSizeX, CY + ASizeY);
LineClip(VC, CX - CSizeX, CY + ASizeY, CX - RX, CY);


/* 9 O'Clock to 12 O'Clock */
LineClip(VC, CX - RX, CY, CX - CSizeX, CY - ASizeY);
LineClip(VC, CX - CSizeX, CY - ASizeY, CX - BSizeX, CY - BSizeY);
LineClip(VC, CX - BSizeX, CY - BSizeY, CX - ASizeX, CY - CSizeY);
LineClip(VC, CX - ASizeX, CY - CSizeY, CX, CY - RY);

} // DrawingFenetre::DrawEllipse12
//lint -restore

#endif // UNUSED_CODE

/*===========================================================================*/
//
//                        Windows-specific code section
//
/*===========================================================================*/

#ifdef _WIN32
NativeDrawContext DrawingFenetre::GetNativeDC(void)
{
if (hdc)
	{
	return(hdc);
	} // if
else
	{
	return(NewDC = GetDC(NativeWin));
	} // else
} // DrawingFenetre::GetNativeDC

/*===========================================================================*/

void DrawingFenetre::ReleaseNativeDC()
{
if (NewDC)
	{
	ReleaseDC(NativeWin, NewDC);
	NewDC = NULL;
	} // if
} // DrawingFenetre::ReleaseNativeDC

NativeDrawContext GUIFenetre::GetNativeDC(void)
{
return(NewDC = GetDC(NativeWin));
} // GUIFenetre::GetNativeDC

/*===========================================================================*/

void GUIFenetre::ReleaseNativeDC()
{
if (NewDC)
	{
	ReleaseDC(NativeWin, NewDC);
	NewDC = NULL;
	} // if
} // GUIFenetre::ReleaseNativeDC

/*===========================================================================*/
/*===========================================================================*/

void DrawingFenetre::SyncBackground(int X, int Y, int W, int H, int XOff, int YOff)
{
NativeDrawContext TempDC;

if (OffScreen)
	{
	if (TempDC = GetDC(NativeWin))
		{
 		if ((XOff != 0) || (YOff != 0))
 			{
 			BitBlt(TempDC, X + XOff, Y + YOff, W, H, OffScreen, X, Y, SRCCOPY);
 			} // if
 		else
 			{
 			BitBlt(TempDC, X, Y, W, H, OffScreen, X, Y, SRCCOPY);
 			} // else
		ReleaseDC(NativeWin, TempDC);
		TempDC = NULL;
		} // if
	} // if
} // DrawingFenetre::SyncBackground

/*===========================================================================*/

#ifdef UNUSED_CODE

void DrawingFenetre::SetFillCol(unsigned short FillColor)
{
// <<<>>>
//SetBPen(TempRP, FillColor);
} // DrawingFenetre::SetFillCol

void DrawingFenetre::UnFilledBox(unsigned short ULX, unsigned short ULY,
 unsigned short LRX, unsigned short LRY)
{
MoveToEx(hdc, ULX, ULY, NULL);
LineTo(hdc, LRX, ULY);
LineTo(hdc, LRX, LRY);
LineTo(hdc, ULX, LRY);
LineTo(hdc, ULX, ULY);
MoveToEx(OffScreen, ULX, ULY, NULL);
LineTo(OffScreen, LRX, ULY);
LineTo(OffScreen, LRX, LRY);
LineTo(OffScreen, ULX, LRY);
LineTo(OffScreen, ULX, ULY);
} // DrawingFenetre::UnFilledBox

void DrawingFenetre::FilledBox(unsigned short ULX, unsigned short ULY,
 unsigned short LRX, unsigned short LRY)
{
HGDIOBJ OldStuff, NewStuff;

NewStuff = CreateSolidBrush(PixelCol);
OldStuff = SelectObject(hdc, NewStuff);
DeleteObject(OldStuff);
OldStuff = SelectObject(OffScreen, NewStuff);
if ((const unsigned long)OldStuff != 0x0000001) DeleteObject(OldStuff);
Rectangle(hdc, ULX, ULY, LRX, LRY);
Rectangle(OffScreen, ULX, ULY, LRX, LRY);
} // DrawingFenetre::FilledBox

void DrawingFenetre::FilledCircle(unsigned short CX, unsigned short CY, unsigned short R)
{
HGDIOBJ OldStuff, NewStuff;

NewStuff = CreateSolidBrush(PixelCol);
OldStuff = SelectObject(hdc, NewStuff);
DeleteObject(OldStuff);
OldStuff = SelectObject(OffScreen, NewStuff);
DeleteObject(OldStuff);
Ellipse(hdc, CX - R, CY - R, CX + R, CY + R);
Ellipse(OffScreen, CX - R, CY - R, CX + R, CY + R);
} // DrawingFenetre::FilledCircle


void DrawingFenetre::UnFilledCircle(int CX, int CY, unsigned short R)
{
HGDIOBJ OldStuff, NewStuff;

NewStuff = GetStockObject(NULL_BRUSH);
OldStuff = SelectObject(hdc, NewStuff);
DeleteObject(OldStuff);
OldStuff = SelectObject(OffScreen, NewStuff);
DeleteObject(OldStuff);
Ellipse(hdc, CX - R, CY - R, CX + R, CY + R);
Ellipse(OffScreen, CX - R, CY - R, CX + R, CY + R);
} // DrawingFenetre::UnFilledCircle

#endif // UNUSED_CODE

/*===========================================================================*/

int DrawingFenetre::SetupPolyDraw(unsigned long MaxPoints, unsigned long CacheSize)
{
HGDIOBJ OldStuff, NewStuff;

// Sets fill color to be same as current drawing pen...
// Only does anything to the background DC
NewStuff = CreateSolidBrush(PixelCol);
OldStuff = SelectObject(OffScreen, NewStuff);
if (PutBack == NULL)
	{
	PutBack = OldStuff;
	} // if
else
	{
	DeleteObject(OldStuff);
	} // else
return(1); // Success!
} // DrawingFenetre::SetupPolyDraw

int DrawingFenetre::DrawNGon(unsigned long Points, PolyPoint *PointList)
{
Polygon(OffScreen, (POINT *)PointList, Points);
return(1); // success
} // DrawingFenetre::DrawNGon

void DrawingFenetre::CleanupPolyDraw(void)
{
unsigned short DAW, DAH;

GetDrawingAreaSize(DAW, DAH);
SyncBackground(0, 0, DAW, DAH);
} // DrawingFenetre::DrawNGon

/*===========================================================================*/

NativeGUIWin GUIFenetre::FenCreateDialog(HINSTANCE a, LPCTSTR b, NativeGUIWin c, DLGPROC d, WCSApp *Ap, Fenetre *Dest)
{
NativeGUIWin Result;
if (Ap) Ap->SetupDlgFenetreKludge(Dest);
Result = CreateDialog(a, b, c, d);
if (Ap) Ap->SetupDlgFenetreKludge(NULL);
return(Result);
} // GUIFenetre::FenCreateDialog

#endif // _WIN32

/*===========================================================================*/

void DrawingFenetre::DrawTempLine(unsigned short XS, unsigned short YS,
 unsigned short XE, unsigned short YE, unsigned char DefPenDesc)
{
SelectDefPen(DefPenDesc);

#ifdef _WIN32
MoveToEx(hdc, XS, YS, NULL);
LineTo(hdc, XE, YE);
#endif // _WIN32
} // DrawingFenetre::DrawTempLine

/*===========================================================================*/

void DrawingFenetre::EraseTempLine(unsigned short XS, unsigned short YS,
 unsigned short XE, unsigned short YE)
{
unsigned short Flipper;

if (XS > XE) {Flipper = XS; XS = XE; XE = Flipper;}
if (YS > YE) {Flipper = YS; YS = YE; YE = Flipper;}

// Erase using data from backupcontent
SyncBackground(XS, YS, (XE - XS) + 1, (YE - YS) + 1);

} // DrawingFenetre::EraseTempLine

/*===========================================================================*/

bool FindWindowOrigin(NativeDrwWin FenGuy, int &X, int &Y)
{
#ifdef _WIN32
RECT Pos;
POINT StW;
//DWORD WinStyle;

//WinStyle = (DWORD)GetWindowLong(FenGuy, GWL_STYLE);
GetWindowRect(FenGuy, &Pos);
StW.x = Pos.left;
StW.y = Pos.top;

if (ScreenToClient((HWND)GlobalApp->WinSys->GetRoot(), &StW))
	{
	if ((StW.y > GetSystemMetrics(SM_CYVIRTUALSCREEN)) || (StW.x > GetSystemMetrics(SM_CXVIRTUALSCREEN))
	 || (StW.y < -GetSystemMetrics(SM_CYVIRTUALSCREEN)) || (StW.x < -GetSystemMetrics(SM_CXVIRTUALSCREEN)))
		{
		//assert(!(StW.x > GetSystemMetrics(SM_CXSCREEN)));
		//assert(!(StW.y > GetSystemMetrics(SM_CYSCREEN)));
		return(false);
		} // if
	else
		{
		X = StW.x;
		Y = StW.y;
		return(true);
		} // else
	} // if

/*
GetClientRect(FenGuy, &Pos);

AdjustWindowRect(&Pos, WinStyle, (int)GetMenu(FenGuy));

X -= Pos.left;
Y -= Pos.top;
*/
/*
WinStyle = WS_OVERLAPPEDWINDOW;
GetWindowRect(FenGuy, &Pos);
X = Pos.left;
Y = Pos.top;

GetClientRect(FenGuy, &Pos);

AdjustWindowRect(&Pos, WinStyle, (int)GetMenu(FenGuy));

X -= Pos.left;
Y -= Pos.top;
*/
#endif // _WIN32
return(false);
} // FindWindowOrigin

/*===========================================================================*/

#ifdef _WIN32

static unsigned long pbmiSize;

static NativeBitmap CreateDIB24(HDC ExistingDC, int Width, int Height)
{
NativeBitmap NB = NULL;

#ifdef USE_LINE_GL_BACKINGSTORE
NB = CreateCompatibleBitmap(ExistingDC, Width, Height);
#else // !USE_LINE_GL_BACKINGSTORE
BITMAPINFO Setup;
void *BitPointer = NULL;

Setup.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
Setup.bmiHeader.biWidth = Width;
Setup.bmiHeader.biHeight = Height;
Setup.bmiHeader.biPlanes = 1;
Setup.bmiHeader.biBitCount = 24;
Setup.bmiHeader.biCompression = BI_RGB;
Setup.bmiHeader.biSizeImage = 0;
Setup.bmiHeader.biXPelsPerMeter = 2834;	// 2834.65 pixels-per-meter is 72 dots per inch.
Setup.bmiHeader.biYPelsPerMeter = 2834;	// 2834.65 pixels-per-meter is 72 dots per inch.
Setup.bmiHeader.biClrUsed = 0;
Setup.bmiHeader.biClrImportant = 0;

NB = CreateDIBSection(ExistingDC, &Setup, DIB_RGB_COLORS, &BitPointer, NULL, NULL);
#endif // !USE_LINE_GL_BACKINGSTORE

return(NB);
} // CreateDIB24

/*===========================================================================*/

// These next two are adapted from stock code in the Win32 SDK
static PBITMAPINFO CreateBitmapInfoStruct(NativeBitmap hBmp)
{
BITMAP bmp;
PBITMAPINFO pbmi;
WORD    cClrBits; 

// Retrieve the bitmap's color format, width, and height.
if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	return(NULL);

// Convert the color format to a count of bits.
cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 

if (cClrBits == 1) 
    cClrBits = 1; 
else if (cClrBits <= 4) 
    cClrBits = 4; 
else if (cClrBits <= 8) 
    cClrBits = 8; 
else if (cClrBits <= 16) 
    cClrBits = 16; 
else if (cClrBits <= 24) 
    cClrBits = 24; 
else 
    cClrBits = 32; 

// Allocate memory for the BITMAPINFO structure. (This structure 
// contains a BITMAPINFOHEADER structure and an array of RGBQUAD data 
// structures.) 
if (cClrBits != 24)
	{
	pbmiSize = (sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (2^cClrBits));
	pbmi = (PBITMAPINFO) AppMem_Alloc(pbmiSize, APPMEM_CLEAR);
	} // if
else
	{ // There is no RGBQUAD array for the 24-bit-per-pixel format.
	pbmiSize = (sizeof(BITMAPINFOHEADER));
	pbmi = (PBITMAPINFO) AppMem_Alloc(pbmiSize, APPMEM_CLEAR);
	} // else

if (!pbmi)
	{
	pbmiSize = NULL;
	return(NULL);
	} // if

// Initialize the fields in the BITMAPINFO structure.
pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
pbmi->bmiHeader.biWidth = bmp.bmWidth; 
pbmi->bmiHeader.biHeight = bmp.bmHeight; 
pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
if (cClrBits < 24)
	{
	pbmi->bmiHeader.biClrUsed = 2^cClrBits;
	} // if

// If the bitmap is not compressed, set the BI_RGB flag.
pbmi->bmiHeader.biCompression = BI_RGB; 

// Compute the number of bytes in the array of color 
// indices and store the result in biSizeImage. 
pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 
 * pbmi->bmiHeader.biHeight * cClrBits; 

// Set biClrImportant to 0, indicating that all of the 
// device colors are important. 
pbmi->bmiHeader.biClrImportant = 0; 

return(pbmi);
} // CreateBitmapInfoStruct

static void DestroyBitmapInfoStruct(PBITMAPINFO Nuke)
{
if (pbmiSize)
	{
	AppMem_Free(Nuke, pbmiSize);
	pbmiSize = NULL;
	} // if

} // DestroyBitmapInfoStruct

/*===========================================================================*/

static int CreateBMPFile(FILE *Stream, PBITMAPINFO pbi, NativeBitmap hBMP, NativeDrawContext hDC) 
{
BITMAPFILEHEADER hdr;
PBITMAPINFOHEADER pbih;
LPBYTE lpBits;
DWORD dwTotal;
DWORD cb;
BYTE *hp;
int BytesWritten = 0, BytesAlloc = 0;

pbih = (PBITMAPINFOHEADER) pbi;
BytesAlloc = pbih->biSizeImage;
lpBits = (LPBYTE) AppMem_Alloc(BytesAlloc, NULL);
if (!lpBits)
	{
	return(0);
	} // if

// Retrieve the color table (RGBQUAD array) and the bits 
// (array of palette indices) from the DIB. 

if (GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
	{
	hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"

	// Compute the size of the entire file.
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize +
	 pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0; 
	hdr.bfReserved2 = 0; 

	// Compute the offset to the array of color indices.
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize +
	 pbih->biClrUsed * sizeof(RGBQUAD); 

	// Copy the BITMAPFILEHEADER into the .BMP file.
	BytesWritten += (int)fwrite((LPVOID)&hdr, 1, sizeof(BITMAPFILEHEADER), Stream);

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	BytesWritten += (int)fwrite((LPVOID)pbih, 1, sizeof(BITMAPINFOHEADER) +
	 pbih->biClrUsed * sizeof(RGBQUAD), Stream);

	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage; 
	hp = lpBits; 
	//while (cb > MAXWRITE)
	//	{ 
    //    fwrite((LPSTR)hp, 1, (int)MAXWRITE, Stream));
    //    cb-= MAXWRITE;
    //    hp += MAXWRITE;
	//	} // while
	//fwrite((LPSTR)hp, 1, (int)cb, Stream);
	BytesWritten += (int)fwrite((LPSTR)hp, 1, (int)cb, Stream);

	} // if

// Free memory.
AppMem_Free(lpBits, BytesAlloc);
return(BytesWritten);
} // CreateBMPFile

/*===========================================================================*/

// Caveat: This saves the BackingStore, and Temp objects will therefore
// not appear in the image.
int DrawingFenetre::SaveContentsToFile(const char *OutName)
{
int Success = 1;

FILE *OutFile;

if (OutFile = PROJ_fopen(OutName, "wb"))
	{
	if (! SaveContentsToFile(OutFile))
		{
		//OutputDebugStr("Save choke.\n");
		Success = 0;
		} // if
	fclose(OutFile);
	} // if
else
	{
	//OutputDebugStr("File choke.\n");
	} // else

return (Success);
} // DrawingFenetre::SaveContentsToFile

/*===========================================================================*/

int DrawingFenetre::SaveContentsToFile(FILE *Out)
{
PBITMAPINFO ThatThing;
int BMPResult = 0, Cleanup = 0;

if (Out)
	{
	if (!hdc)
		{
		SetupForDrawing();
		Cleanup = 1;
		} // if
	if (ThatThing = CreateBitmapInfoStruct(ContentBackup))
		{
		if (BMPResult = CreateBMPFile(Out, ThatThing, ContentBackup, OffScreen))
			{
			} // if
		else
			{
			//OutputDebugString("Choke Bytes..\n");
			} // else
		DestroyBitmapInfoStruct(ThatThing);
		ThatThing = NULL;
		} // if
	else
		{
		//OutputDebugString("Choke.\n");
		} // 
	if (Cleanup)
		{
		CleanupFromDrawing();
		} // if
	} // if

return(BMPResult);
} // DrawingFenetre::SaveContentsToFile

/*===========================================================================*/

void DrawingFenetre::BGDrawLine24(int X, int Y, unsigned short W, unsigned char *R, unsigned char *G, unsigned char *B)
{
unsigned int LineLength;
unsigned short NomW, XPaint;
unsigned char RT, GT, BT;

NomW = W;

if (R && G && B)
	{
	if (CTBitArray)
		{ // see if existing DIBSECTION is suitable
		if (BMI.bmiHeader.biWidth != (signed)NomW)
			{
			AppMem_Free(CTBitArray, CTBASize);
			CTBitArray = NULL;
			CTBASize = 0;
			} // if
		} // if
	if (!CTBitArray)
		{
		BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BMI.bmiHeader.biWidth = NomW;
		BMI.bmiHeader.biHeight = 1; // used to be -1
		BMI.bmiHeader.biPlanes = 1;
		BMI.bmiHeader.biBitCount = 24;
		BMI.bmiHeader.biCompression = BI_RGB;
		BMI.bmiHeader.biSizeImage = 0;
		BMI.bmiHeader.biXPelsPerMeter = 2835; // 72 dpi
		BMI.bmiHeader.biYPelsPerMeter = 2835;
		BMI.bmiHeader.biClrUsed = 0;
		BMI.bmiHeader.biClrImportant = 0;
		LineLength = (NomW * 3);
		LineLength = ROUNDUP(LineLength, 4);
		CTBASize = LineLength;
		CTBitArray = (unsigned char *)AppMem_Alloc(CTBASize, NULL);
		} // if
	if (CTBitArray)
		{
		for (XPaint = 0; XPaint < NomW; XPaint++)
			{
			RT = R[XPaint];
			GT = G[XPaint];
			BT = B[XPaint];
			((char *)CTBitArray)[(XPaint * 3)] = BT;
			((char *)CTBitArray)[(XPaint * 3) + 1] = GT;
			((char *)CTBitArray)[(XPaint * 3) + 2] = RT;
			} // for
		SetDIBitsToDevice(OffScreen, X, Y,
		 W, 1, 0, 0, 0, 1, CTBitArray, &BMI, DIB_RGB_COLORS);
		} // if
	} // if

} // DrawingFenetre::BGDrawLine24

/*===========================================================================*/

// RastBlast support code
#ifdef _WIN32
BITMAPINFO BMIBlast;
static unsigned char *BlastCTBitArray = NULL;
unsigned long BlastCTBASize;
#endif // _WIN32

void ReleaseRastBlast(void)
{

if (BlastCTBitArray)
	AppMem_Free(BlastCTBitArray, BlastCTBASize);
BlastCTBitArray = NULL;

} // ReleaseRastBlast

void RastBlast(NativeDrawContext BlastDest, unsigned short X, unsigned short Y,
 unsigned short W, unsigned char *R, unsigned char *G, unsigned char *B)
{
#ifdef _WIN32
unsigned short NomW, XPaint;
unsigned char RT, GT, BT;
unsigned int LineLength;

NomW = W;

if (R && G && B)
	{
	if (BlastCTBitArray)
		{ // see if existing DIBSECTION is suitable
		if (BMIBlast.bmiHeader.biWidth != (signed)NomW)
			{
			AppMem_Free(BlastCTBitArray, BlastCTBASize);
			BlastCTBitArray = NULL;
			BlastCTBASize = 0;
			} // if
		} // if
	if (!BlastCTBitArray)
		{
		BMIBlast.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BMIBlast.bmiHeader.biWidth = NomW;
		BMIBlast.bmiHeader.biHeight = 1; // used to be -1
		BMIBlast.bmiHeader.biPlanes = 1;
		BMIBlast.bmiHeader.biBitCount = 24;
		BMIBlast.bmiHeader.biCompression = BI_RGB;
		BMIBlast.bmiHeader.biSizeImage = 0;
		BMIBlast.bmiHeader.biXPelsPerMeter = 2835; // 72 dpi
		BMIBlast.bmiHeader.biYPelsPerMeter = 2835;
		BMIBlast.bmiHeader.biClrUsed = 0;
		BMIBlast.bmiHeader.biClrImportant = 0;
		LineLength = (NomW * 3);
		LineLength = ROUNDUP(LineLength, 4);
		BlastCTBASize = LineLength;
		BlastCTBitArray = (unsigned char *)AppMem_Alloc(BlastCTBASize, NULL);
		} // if
	if (BlastCTBitArray)
		{
		for (XPaint = 0; XPaint < NomW; XPaint++)
			{
			RT = R[XPaint];
			GT = G[XPaint];
			BT = B[XPaint];
			((char *)BlastCTBitArray)[(XPaint * 3)] = BT;
			((char *)BlastCTBitArray)[(XPaint * 3) + 1] = GT;
			((char *)BlastCTBitArray)[(XPaint * 3) + 2] = RT;
			} // for
		SetDIBitsToDevice(BlastDest, X, Y,
		 W, 1, 0, 0, 0, 1, BlastCTBitArray, &BMIBlast, DIB_RGB_COLORS);
		} // if
	} // if

#endif // _WIN32
} // RastBlast

/*===========================================================================*/

// FGL support code

int GLDrawingFenetre::fglInit(void)
{
PIXELFORMATDESCRIPTOR pfd = 
	{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_SWAP_COPY,
	PFD_TYPE_RGBA,
	24,
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0, 0, 0, 0,
	16,
	//32,
	0,

	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
	};

if (GLSetup)
	{
	return(1);
	} // if

// Obtain linkage to opengl32.dll and glu32.dll
if (OGLHM = LoadLibrary("opengl32.dll"))
	{
	if (GLUHM = LoadLibrary("glu32.dll"))
		{
		// glu
		dglLookAt = (DGLLOOKAT)GetProcAddress(GLUHM, "gluLookAt");
		dglPerspective = (DGLPERSPECTIVE)GetProcAddress(GLUHM, "gluPerspective");
		if (dglLookAt && dglPerspective)
			{
			dglBegin = (DGLBEGIN)GetProcAddress(OGLHM, "glBegin");
			dglClear = (DGLCLEAR)GetProcAddress(OGLHM, "glClear");
			dglClearColor = (DGLCLEARCOLOR)GetProcAddress(OGLHM, "glClearColor");
			dglClearDepth = (DGLCLEARDEPTH)GetProcAddress(OGLHM, "glClearDepth");
			dglColor3fv = (DGLCOLOR3FV)GetProcAddress(OGLHM, "glColor3fv");
			dglColor3f = (DGLCOLOR3F)GetProcAddress(OGLHM, "glColor3f");
			dglColorMaterial = (DGLCOLORMATERIAL)GetProcAddress(OGLHM, "glColorMaterial");
			dglDepthFunc = (DGLDEPTHFUNC)GetProcAddress(OGLHM, "glDepthFunc");
			dglDrawBuffer = (DGLDRAWBUFFER)GetProcAddress(OGLHM, "glDrawBuffer");
			dglEnable = (DGLENABLE)GetProcAddress(OGLHM, "glEnable");
			dglDisable = (DGLENABLE)GetProcAddress(OGLHM, "glDisable");
			dglEnd = (DGLEND)GetProcAddress(OGLHM, "glEnd");
			dglFlush = (DGLFLUSH)GetProcAddress(OGLHM, "glFlush");
			dglFogf = (DGLFOGF)GetProcAddress(OGLHM, "glFogf");
			dglFogfv = (DGLFOGFV)GetProcAddress(OGLHM, "glFogfv");
			dglFogi = (DGLFOGI)GetProcAddress(OGLHM, "glFogi");
			dglLightfv = (DGLLIGHTFV)GetProcAddress(OGLHM, "glLightfv");
			dglLightModeli = (DGLLIGHTMODELI)GetProcAddress(OGLHM, "glLightModeli");
			dglLoadIdentity = (DGLLOADIDENTITY)GetProcAddress(OGLHM, "glLoadIdentity");
			dglMaterialfv = (DGLMATERIALFV)GetProcAddress(OGLHM, "glMaterialfv");
			dglMatrixMode = (DGLMATRIXMODE)GetProcAddress(OGLHM, "glMatrixMode");
			dglPushMatrix = (DGLPUSHMATRIX)GetProcAddress(OGLHM, "glPushMatrix");
			dglPopMatrix = (DGLPOPMATRIX)GetProcAddress(OGLHM, "glPopMatrix");
			dglFrustum = (DGLFRUSTUM)GetProcAddress(OGLHM, "glFrustum");
			dglViewport = (DGLVIEWPORT)GetProcAddress(OGLHM, "glViewport");
			dglNormal3fv = (DGLNORMAL3FV)GetProcAddress(OGLHM, "glNormal3fv");
			dglNormal3d = (DGLNORMAL3D)GetProcAddress(OGLHM, "glNormal3d");
			dglRotated = (DGLROTATED)GetProcAddress(OGLHM, "glRotated");
			dglVertex3dv = (DGLVERTEX3DV)GetProcAddress(OGLHM, "glVertex3dv");
			dglVertex3d = (DGLVERTEX3D)GetProcAddress(OGLHM, "glVertex3d");

			dglMakeCurrent = (DGLMAKECURRENT)GetProcAddress(OGLHM, "wglMakeCurrent");
			dglCreateContext = (DGLCREATECONTEXT)GetProcAddress(OGLHM, "wglCreateContext");
			dglDeleteContext = (DGLDELETECONTEXT)GetProcAddress(OGLHM, "wglDeleteContext");

			if (dglBegin && dglClear && dglClearColor && dglClearDepth && dglColor3fv && dglColor3f && dglColorMaterial &&
			 dglDepthFunc && dglDrawBuffer && dglEnable && dglDisable && dglEnd && dglFlush && dglFogf && dglFogfv && dglFogi &&
			 dglLightfv && dglLightModeli && dglLoadIdentity && dglMaterialfv && dglMatrixMode &&
			 dglPushMatrix && dglPopMatrix && dglFrustum && dglViewport &&
			 dglNormal3fv && dglNormal3d && dglRotated && dglVertex3dv && dglVertex3d && dglMakeCurrent && dglCreateContext && dglDeleteContext)
				{
				// Use Subwindow DC 
				ShowWindow(FGLSubWin, SW_SHOW);
				if (FGLHDC = GetDC(FGLSubWin))
					{
					PixelFormat = ChoosePixelFormat(FGLHDC, &pfd);

					DescribePixelFormat(FGLHDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
					if (!(pfd.dwFlags & PFD_SWAP_COPY))
						{ // no SWAP_COPY, read from front instead
						GLReadPixelsFront = 1;
						} // if

					if (SetPixelFormat(FGLHDC, PixelFormat, &pfd))
						{
						if (FGLHDC && (FGLRC = fglCreateContext(FGLHDC)))
							{
							ShareGLRC(FGLRC);
							InsertGLRC(FGLRC);
							if (fglMakeCurrent(FGLHDC, FGLRC))
								{
								const char *Exts;

								GLSetup = 1;
								ShowWindow(FGLSubWin, SW_HIDE);
								GLON = 0;

								// Determine if GL_EXT_bgra is available
								if (Exts = (const char *)glGetString(GL_EXTENSIONS))
									{
									if (strstr(Exts, "GL_EXT_bgra"))
										{
										// Dagnabbit, this extension appears to actually be SLOWER
										// than doing it ourselves. We'll leave it off for now.
										GLEXTBGRA = 1;
										} // if
									// GLClampEdgeAvailable
									if (strstr(Exts, "GL_EXT_texture_edge_clamp"))
										{
										GLClampEdgeAvailable = 1;
										} // if
									if (strstr(Exts, "GL_SGIS_texture_edge_clamp"))
										{
										GLClampEdgeAvailable = 1;
										} // if
									} // if

								return (1);
								} // if
							} // if
						} // if
					ReleaseDC(FGLSubWin, FGLHDC);
					FGLHDC = NULL;
					} // if
				} // if
			else
				{
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to map functions in opengl32.dll .");
				} // else
			} // if
		else
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to map functions in glu32.dll .");
			} // else
		FreeLibrary(GLUHM);
		GLUHM = NULL;
		} // if
	else
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "glu32.dll Load Failed.");
		} // else
	FreeLibrary(OGLHM);
	OGLHM = NULL;
	} // if
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "opengl32.dll Load Failed.");
	} // else


GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "GL Init Failed.");

return (NULL);

} // GLDrawingFenetre::fglInit

/*===========================================================================*/

void GLDrawingFenetre::InsertGLRC(HGLRC Ins)
{
int LoopScan;

for (LoopScan = 0; LoopScan < WCS_FENETRE_MAX_SHARED_GLRCS; LoopScan++)
	{
	if (CurrentGDLF_GLRCs[LoopScan] == NULL)
		{
		CurrentGDLF_GLRCs[LoopScan] = Ins;
		return;
		} // if
	} // for

} // GLDrawingFenetre::InsertGLRC

/*===========================================================================*/

void GLDrawingFenetre::RemoveGLRC(HGLRC Rem)
{
int LoopScan;

for (LoopScan = 0; LoopScan < WCS_FENETRE_MAX_SHARED_GLRCS; LoopScan++)
	{
	if (CurrentGDLF_GLRCs[LoopScan] == Rem)
		{
		CurrentGDLF_GLRCs[LoopScan] = NULL;
		return;
		} // if
	} // for

} // GLDrawingFenetre::RemoveGLRC

/*===========================================================================*/

void GLDrawingFenetre::ShareGLRC(HGLRC Share)
{
int LoopScan;

for (LoopScan = 0; LoopScan < WCS_FENETRE_MAX_SHARED_GLRCS; LoopScan++)
	{
	if (CurrentGDLF_GLRCs[LoopScan])
		{
		wglShareLists(CurrentGDLF_GLRCs[LoopScan], Share);
		return;
		} // if
	} // for

} // GLDrawingFenetre::ShareGLRC

/*===========================================================================*/

int GLDrawingFenetre::LastOneOut(void)
{
int LoopScan;

for (LoopScan = 0; LoopScan < WCS_FENETRE_MAX_SHARED_GLRCS; LoopScan++)
	{
	if (CurrentGDLF_GLRCs[LoopScan])
		{
		return(0);
		} // if
	} // for

return(1);

} // GLDrawingFenetre::LastOneOut

/*===========================================================================*/

void GLDrawingFenetre::fglHide(void)
{

GoGDI();

} // GLDrawingFenetre::fglHide

/*===========================================================================*/

void GLDrawingFenetre::fglShow(void)
{

GoGL();

} // GLDrawingFenetre::fglShow

/*===========================================================================*/

void GLDrawingFenetre::fglCleanup(void)
{

if (FGLRC)
	{
	fglMakeCurrent(FGLHDC, NULL);
	RemoveGLRC(FGLRC);
	fglDeleteContext(FGLRC);
	FGLRC = NULL;
	} // if
if (FGLHDC)
	{
	ReleaseDC(FGLSubWin, FGLHDC);
	FGLHDC = NULL;
	} // if

} // GLDrawingFenetre::fglCleanup

/*===========================================================================*/

void GLDrawingFenetre::GoGL(void)
{
GLON = 1;
ShowWindow(FGLSubWin, SW_SHOW);
fglMakeCurrent(FGLHDC, FGLRC);
} // GLDrawingFenetre::GoGL

/*===========================================================================*/

void GLDrawingFenetre::ReGL(void)
{
unsigned short VWidth, VHeight;
if (FGLHDC && FGLRC)
	{
	fglMakeCurrent(FGLHDC, NULL);
	//fglDeleteContext(FGLRC);
	//FGLRC = fglCreateContext(FGLHDC);
	fglMakeCurrent(FGLHDC, FGLRC);
	GetDrawingAreaSize(VWidth, VHeight);
	glViewport(0, 0, VWidth, VHeight);
	} // if
} // GLDrawingFenetre::ReGL

/*===========================================================================*/

void GLDrawingFenetre::MakeCurrentGL(void)
{
if (FGLHDC && FGLRC)
	{
	fglMakeCurrent(FGLHDC, NULL);
	fglMakeCurrent(FGLHDC, FGLRC);
	} // if
} // GLDrawingFenetre::MakeCurrentGL

#endif // _WIN32

/*===========================================================================*/

double GLDrawingFenetre::GLGetPixelZ(int X, int Y)
{
float Z;
double DZ;
MakeCurrentGL();
glReadPixels(X, Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &Z);
DZ = Z;
/*DZ *= (FarClip - NearClip);
DZ += NearClip; */
return(DZ);
} // GLDrawingFenetre::GLGetPixelZ

/*===========================================================================*/

void GLDrawingFenetre::GLGetPixelRGBA(int X, int Y, unsigned char &R, unsigned char &G, unsigned char &B, unsigned char &A)
{
unsigned char RGBA[4];
MakeCurrentGL();
glPixelStorei(GL_PACK_ALIGNMENT, 4);
glPixelStorei(GL_PACK_ROW_LENGTH, 0);
glPixelStorei(GL_PACK_SKIP_ROWS, 0);
glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
glReadPixels(X, Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, RGBA);
R = RGBA[0];
G = RGBA[1];
B = RGBA[2];
A = RGBA[3];
return;
} // GLDrawingFenetre::GLGetPixelRGBA

/*===========================================================================*/

void GLDrawingFenetre::GoGDI(void)
{
GLON = 0;
#ifdef _WIN32
FreezeRGBZ();
ShowWindow(FGLSubWin, SW_HIDE);
#endif // _WIN32
} // GLDrawingFenetre::GoGDI

/*===========================================================================*/

void GLDrawingFenetre::RestoreRGBZ(void)
{
long SwapX, SwapY;
long RoundWidth;
GLubyte *rgb, *RGBbuf;
BITMAP bmp;

MakeCurrentGL();
//glGetIntegerv(GL_VIEWPORT, vp);
glFinish();
glPixelStorei(GL_PACK_ALIGNMENT, 4);
glPixelStorei(GL_PACK_ROW_LENGTH, 0);
glPixelStorei(GL_PACK_SKIP_ROWS, 0);
glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

if (CBZ)
	{
	glClearDepth(1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// we have to flip these stupid pixels both before and then after

	RoundWidth = OSCBWidth * 3;
	RoundWidth = (RoundWidth + 3) & ~3;
	if (GetObject(ContentBackup, sizeof(BITMAP), (LPSTR)&bmp))
		{
		RGBbuf = (GLubyte *)bmp.bmBits;
		for (SwapY = 0; SwapY < OSCBHeight; SwapY++)
			{
			rgb = &RGBbuf[SwapY * RoundWidth];
			for (SwapX = 0; SwapX < OSCBWidth; SwapX++, rgb += 3)
				{
				GLubyte temp;
				temp = rgb[0];
				rgb[0] = rgb[2];
				rgb[2] = temp;
				} // for
			} // for

		glRasterPos3d(0.0, 0.0, 1.0); // write image data at 0,0 and furthest
		//glDrawPixels(OSCBWidth, OSCBHeight, GL_RGB, GL_UNSIGNED_BYTE, CBZ);
		glDrawPixels(OSCBWidth, OSCBHeight, GL_DEPTH_COMPONENT, GL_FLOAT, CBZ);

		// put 'em back
		for (SwapY = 0; SwapY < OSCBHeight; SwapY++)
			{
			rgb = &RGBbuf[SwapY * RoundWidth];
			for (SwapX = 0; SwapX < OSCBWidth; SwapX++, rgb += 3)
				{
				GLubyte temp;
				temp = rgb[0];
				rgb[0] = rgb[2];
				rgb[2] = temp;
				} // for
			} // for
		} // if

	} // if

} // GLDrawingFenetre::RestoreRGBZ

/*===========================================================================*/

void GLDrawingFenetre::FreezeRGBZ(void)
{
#ifdef _WIN32
BITMAP bmp;
long SwapX, SwapY;
GLubyte *rgb, *RGBbuf, temp;
//GLint vp[4];
long RoundWidth;
#ifdef USE_LINE_GL_BACKINGSTORE
BITMAPINFO LBMI;
#endif // USE_LINE_GL_BACKINGSTORE

MakeCurrentGL();
//glGetIntegerv(GL_VIEWPORT, vp);
glFinish();
glPixelStorei(GL_PACK_ALIGNMENT, 4);
glPixelStorei(GL_PACK_ROW_LENGTH, 0);
glPixelStorei(GL_PACK_SKIP_ROWS, 0);
glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

// Copy GL data to GDI buffer
if (!GLInhibitReadPixels)
	{
	// Retrieve the bitmap's color format, width, and height.
	if (GetObject(ContentBackup, sizeof(BITMAP), (LPSTR)&bmp))
		{
		//if (GLReadPixelsFront)
		if (0) // ViewGUI hopefully takes care of ensuring a valid BG buffer now
			{
			glReadBuffer(GL_FRONT);
			} // if
		else
			{
			glReadBuffer(GL_BACK);
			} // else
		if (bmp.bmBits) // safer failure mode
			{
			if (GLEXTBGRA)
				{
				// using the hardcoded 0x80e0 instead of the define BGR_EXT allows this code
				// to compile on systems that don't support BGR_EXT and it will safely fail.
				glReadPixels(0, 0, bmp.bmWidth, bmp.bmHeight, 0x80e0 /* BGR_EXT */, GL_UNSIGNED_BYTE, bmp.bmBits);
				} // if
			else
				{
				glReadPixels(0, 0, bmp.bmWidth, bmp.bmHeight, GL_RGB, GL_UNSIGNED_BYTE, bmp.bmBits);
				} // else
			RGBbuf = (GLubyte *)bmp.bmBits;
			RoundWidth = bmp.bmWidth * 3;
			RoundWidth = (RoundWidth + 3) & ~3;
			// No need to swap if GL can provide for us in BGR order
			if (!GLEXTBGRA)
				{
				for (SwapY = 0; SwapY < bmp.bmHeight; SwapY++)
					{
					rgb = &RGBbuf[SwapY * RoundWidth];
					for (SwapX = 0; SwapX < bmp.bmWidth; SwapX++, rgb += 3)
						{
						temp = rgb[0];
						rgb[0] = rgb[2];
						rgb[2] = temp;
						} // for
					} // for
				} // if

			if (!CBZ)
				{
				CBZ = new float[bmp.bmWidth * bmp.bmHeight];
				} // 

			if (CBZ)
				{
				glReadPixels(0, 0, bmp.bmWidth, bmp.bmHeight, GL_DEPTH_COMPONENT, GL_FLOAT, CBZ);
				} // if
			} // if
		else
			{
			// fallback if bits not available through GetObject
			#ifdef USE_LINE_GL_BACKINGSTORE

			LBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			LBMI.bmiHeader.biWidth = bmp.bmWidth;
			LBMI.bmiHeader.biHeight = 1; // used to be -1
			LBMI.bmiHeader.biPlanes = 1;
			LBMI.bmiHeader.biBitCount = 24;
			LBMI.bmiHeader.biCompression = BI_RGB;
			LBMI.bmiHeader.biSizeImage = 0;
			LBMI.bmiHeader.biXPelsPerMeter = 2835; // 72 dpi
			LBMI.bmiHeader.biYPelsPerMeter = 2835;
			LBMI.bmiHeader.biClrUsed = 0;
			LBMI.bmiHeader.biClrImportant = 0;

			for (SwapY = 0; SwapY < bmp.bmHeight; SwapY++)
				{
				// use the large ThumbNailR buffer as a line buffer space
				glReadPixels(0, bmp.bmHeight - (SwapY + 1), bmp.bmWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, ThumbNailR);
				rgb = &ThumbNailR[0];
				for (SwapX = 0; SwapX < bmp.bmWidth; SwapX++, rgb += 3)
					{
					temp = rgb[0];
					rgb[0] = rgb[2];
					rgb[2] = temp;
					} // for
				// Blit into backingstore bitmap
				SetDIBitsToDevice(OffScreen, 0, SwapY, bmp.bmWidth, 1, 0, 0, 0, 1, ThumbNailR, &LBMI, DIB_RGB_COLORS);
				} // for
			#else // !USE_LINE_GL_BACKINGSTORE
			GLInhibitReadPixels = 1;
			#endif // !USE_LINE_GL_BACKINGSTORE
			} // else
		} // if
	} // if !

if (GLInhibitReadPixels)
	{
	BitBlt(GetBackupDC(), 0, 0, SizeX, SizeY, FGLHDC, 0, 0, SRCCOPY);
	} // if

#endif // _WIN32
} // GLDrawingFenetre::FreezeRGBZ

/*===========================================================================*/

#ifdef _WIN32
NativeGUIWin GUIFenetre::CreateWinFromTemplate(unsigned long TemplateID, NativeGUIWin Parent, bool TabAppearance)
{
NativeGUIWin Result = NULL;

Fenetre *Stash;
Stash = this;

if (Parent == NULL)
	{
	Parent = LocalWinSys()->RootWin;
	} // if

if (Result = FenCreateDialog((HINSTANCE)LocalWinSysCopy->Instance(), MAKEINTRESOURCE((WORD)TemplateID), Parent, (DLGPROC)DlgProc, GlobalApp, Stash))
	{
	SetWindowLong(Result, GWL_USERDATA, (LONG)Stash);
    // determine if this is a tab widget window, and set the theming differently for the background
    if (TabAppearance && g_xpStyle.IsAppThemed())
		{
        g_xpStyle.EnableThemeDialogTexture(Result, ETDT_ENABLETAB);
		} // if
	} // if

return(Result);

} // GUIFenetre::CreateWinFromTemplate()
#endif // _WIN32

/*===========================================================================*/

NativeGUIWin GUIFenetre::CreateSubWinFromTemplate(unsigned long TemplateID, int Panel, int Pane, bool TabAppearance)
{
NativeGUIWin Result = NULL;

#ifdef _WIN32
Fenetre *Stash;
Stash = this;
if (Result = FenCreateDialog((HINSTANCE)LocalWinSysCopy->Instance(), MAKEINTRESOURCE((WORD)TemplateID), NativeWin, (DLGPROC)DlgProc, GlobalApp, Stash))
	{
	SetWindowLong(Result, GWL_USERDATA, (LONG)Stash);
    // determine if this is a tab widget subwindow, and set the theming differently for the background
    if (TabAppearance && g_xpStyle.IsAppThemed())
		{
        g_xpStyle.EnableThemeDialogTexture(Result, ETDT_ENABLETAB);
		} // if
	} // if

SubPanels[Panel][Pane] = Result;
#endif // _WIN32

return(Result);

} // GUIFenetre::CreateSubWinFromTemplate

/*===========================================================================*/

BOOL CALLBACK ShuffleChildren(HWND hwnd, LPARAM lParam)
{
POINT *ShuffleDelta;
RECT CurPos;
unsigned long WStyle;
Fenetre *Card;

if (ShuffleDelta = (POINT *)lParam)
	{
	WStyle = GetWindowLong(hwnd, GWL_STYLE);
	if (GetParent(hwnd) == LocalRootWin)
		{
		if (WStyle & WS_POPUP)
			{
			if (Card = (Fenetre *)GetWindowLong(hwnd, GWL_USERDATA))
				{
				Card->ForceMoveSize = 2;
				GetWindowRect(hwnd, &CurPos);
				CurPos.left  += ShuffleDelta->x;
				CurPos.top   += ShuffleDelta->y;
				//SetWindowPos(hwnd, NULL, CurPos.left, CurPos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				DeferWindowPos(ShuffleData, hwnd, NULL, CurPos.left, CurPos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				if (Card->WinCaptionButtonBar) Card->PositionWinCaptionBar(CurPos.left, CurPos.top, ShuffleData);
				} // if
			} // if
		} // if
	} // if

return(TRUE);

} // ShuffleChildren

/*===========================================================================*/

bool Fenetre::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{
CommonComponentEditor *TestProbe = NULL;

TestProbe = dynamic_cast<CommonComponentEditor *>(this);
if (TestProbe) // is it a CommonComponentEditor?
	{
	return(TestProbe->RespondToInquireWindowCapabilities(AskAbout));
	} // if

return(false);

} // Fenetre::InquireWindowCapabilities

/*===========================================================================*/

void Fenetre::SetDockState(bool NewState)
{

if (!TestWinManFlags(WCS_FENETRE_WINMAN_NODOCK))
	{
	if (NewState)
		{
		SetWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
		} // if
	else
		{
		ClearWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED);
		} // else
	DockUpdate(0);
	} // if

} // Fenetre::SetDockState

/*===========================================================================*/

bool Fenetre::GetDockState(void) const
{

return(TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) ? true : false);

} // Fenetre::GetDockState

/*===========================================================================*/

bool Fenetre::QueryDisplayAdvancedUIVisibleState(void)
{

if (GlobalApp->MainProj->Prefs.GlobalAdvancedEnabled) // check global setting here
	{
	return(true); // regardless of local setting
	} // if
else
	{
	return(QueryLocalDisplayAdvancedUIVisibleState()); // go by the local state, if any
	} // else

} // Fenetre::QueryDisplayAdvancedUIVisibleState

/*===========================================================================*/

void Fenetre::SetDisplayAdvancedUIVisibleStateFlag(bool NewState)
{

if (NewState) SetWinManFlags(WCS_FENETRE_WINMAN_SHOWADV);
else ClearWinManFlags(WCS_FENETRE_WINMAN_SHOWADV);
UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_SHOWADV);

} // Fenetre::SetDisplayAdvancedUIVisibleStateFlag
