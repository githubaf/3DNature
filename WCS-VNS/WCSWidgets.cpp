// WCSWidgets.cpp
// Application-supplied Widget/MuiCustomClass/CustomControl source
// Built from scratch on 1/21/96 by Chris 'Xenon' Hanson
// Copyright 1996

// CXH Note:
//  I'm not terribly proud of any of this code, nor the circumstances
// that required me to write it. I despise Microsoft Windows.

// FPW2 Note:
//    Lint agrees :)

//lint -save -e648

#include "stdafx.h"
#include "Types.h"
#include "AppMem.h"
#include "WCSWidgets.h"
#include "WidgetSupport.h"
#include "Application.h"
#include "Joe.h"
#include "WCSVersion.h"
#include "resource.h"
#include "Useful.h"
#include "Palette.h"
#include "Fenetre.h"
#include "Requester.h"
#include "Notify.h"

// hardwiring hack
#include "Conservatory.h"
#include "SceneViewGUI.h"

#include "Raster.h"

#define WCS_WIDGET_SRAH_SUPPORT_COORDS

class AnimDouble;
#include "GraphData.h"
#include "Project.h"
#include "Interactive.h"

#ifdef _WIN32
#include <mmsystem.h>
#include "VisualStylesXP.h"
#endif // _WIN32

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW       0x00020000
#endif //!CS_DROPSHADOW

#define WCS_WIDGET_GRADIENT_MID_BORDER		5
#define WCS_WIDGET_GRADIENT_LOWER_HEIGHT	15
#define WCS_WIDGET_GRADIENT_NEEDLE_HEIGHT	15
#define WCS_WIDGET_GRADIENT_SIDE_BORDER		5
#define WCS_WIDGET_GRADIENT_NODE_HALFWIDTH	WCS_WIDGET_GRADIENT_SIDE_BORDER
#define WCS_WIDGET_GRADIENT_NODE_WIDTH		(WCS_WIDGET_GRADIENT_NODE_HALFWIDTH + WCS_WIDGET_GRADIENT_NODE_HALFWIDTH)

// The following is used for reporting a Widget Debug error back to
// the user/betatester.
//static int WidgetDebug;
//static char WidDbgStr[200];

#define WCSW_SCRATCHSIZE 300
static char Scratch[WCSW_SCRATCHSIZE];

#define WCS_WIDGET_COMPBUF_SIZE 50
static char FloatIntCompBuf[WCS_WIDGET_COMPBUF_SIZE];
static char SNADCompBuf[WCS_WIDGET_COMPBUF_SIZE], SNADDeCompBufA[WCS_WIDGET_COMPBUF_SIZE];
//  SNADDeCompBufB[WCS_WIDGET_COMPBUF_SIZE], SNADDeCompBufC[WCS_WIDGET_COMPBUF_SIZE],
//  SNADDeCompBufD[WCS_WIDGET_COMPBUF_SIZE], SNADDeCompBufE[WCS_WIDGET_COMPBUF_SIZE];
static char LabelName[64];
FloatIntConfig Cfg;
static DiskDirConfig DDCfg;
static SmartCheckConfig SCCfg;
static SmartRadioConfig SRCfg;
static ToolButtonConfig TBCfg;

// Draw3D stuff
extern BOOL f3dDialogs;

// This should not be necessary...
#ifdef _WIN32
#ifdef STRICT
#define WPROC WNDPROC
#else // !STRICT
#define WPROC FARPROC
#endif // !STRICT
#endif // _WIN32

static char *FWT[] =
	{
	"0",
	"01",
	"012",
	"0123",
	"01234",
	"012345",
	"0123456",
	"01234567",
	"012345678",
	"0123456789", // 10
	"01234567890",
	"012345678901",
	"0123456789012",
	"01234567890123",
	"012345678901234" // 15
	}; /* FWT */

#ifdef _WIN32
long WINAPI ToolButtonWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI FloatIntWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI AnimGradientWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SlightlyModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI UnModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedScrollWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedTreeWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedListWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ModifiedTabWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ColumnedListViewWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI TextColumnMarkerWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);

long WINAPI WinCaptionButtonBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI StatusBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SplitBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI ColorBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI DiskDirWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SmartCheckWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI SmartRadioWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI AnimGraphWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI RasterWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI GridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
long WINAPI LinkWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
static NativeControl MasterTip;
#endif // _WIN32

void SNmSync(NativeControl Me, struct FloatIntConfig *FIC);
ULONG SNmStr(NativeControl Me, struct FloatIntConfig *FIC);
void SNADmSync(NativeControl Me, struct FloatIntConfig *FIC, AnimDouble *AD);
ULONG SNADmStr(NativeControl Me, struct FloatIntConfig *FIC, AnimDouble *AD);
double SNADmUnformatLinear(char *Text, AnimDouble *AD);
double SNADmUnformatVelocity(char *Text, AnimDouble *AD);
double SNADmUnformatLatLon(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent = NULL);
char *SNADParseNumberSuffix(char *Advance, char &SuffixChar, char *ValidSuffixes, double &NumberVal);
int SNCalculateFractionalDigits(double Val);
// this is safe to call even if it's not an SNAD, it'll just return 0.0
double SNADmGetCurValue(NativeControl Me, struct FloatIntConfig *FIC);


ULONG SNDoIncDec(NativeControl Me,  struct FloatIntConfig *FIC, int Action);
double SNCalcIncDec(double Quantity, struct FloatIntConfig *FIC, int Action);

AnimCritter *SRAHGetAssociatedAD(RasterAnimHost *Parent, AnimDouble *AD);
int SRAHDoPopup(NativeControl Me, struct FloatIntConfig *FIC, int X, int Y, unsigned long MenuClassFlags, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent);
void SRAHConfigButtons(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent);
unsigned long SRAHQueryButton(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent, int ButtonNum);
void SRAHConfigLabel(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent);

void DDmSync(NativeControl EditDude, int Width, char *Path, char *File);

void SCmSync(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID);
void SCGetCheck(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, char *State);
void SCSetCheck(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, char State);

void SRmSync(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long RadioVal);
void SRGetState(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long *State);
void SRSetState(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long State);


#ifdef _WIN32
WNDPROC EditWndProc, ButtonWndProc, ScrollWndProc, TreeWndProc, ListWndProc, ComboWndProc, ToolbarWndProc;
WNDPROC TabWndProc, RealButtonWndProc, ListViewWndProc;
unsigned long ListViewOriginalWndExtra;
static int ButtonWndExtra;
static TOOLINFO HCti;
#endif // _WIN32

/*===========================================================================*/

#ifdef _WIN32
WidgetLib::WidgetLib(GUIContext *GUI)
{
int WPenInit;
WNDCLASS InquireRegister;
HWND TempWindow;
ErrorStatus = 0;
INITCOMMONCONTROLSEX WhichControls;
unsigned char R, G, B;

XPGreyButtonDisabled = XPGreyButtonHot = XPGreyButtonNormal = XPGreyButtonPressed = NULL;
XPGreyButtonDisabledLg = XPGreyButtonHotLg = XPGreyButtonNormalLg = XPGreyButtonPressedLg = NULL;
XPCloseButtonHot = XPCloseButtonNormal = XPCloseButtonPressed = XPCloseButtonHotLg = XPCloseButtonNormalLg = XPCloseButtonPressedLg = NULL;

WhichControls.dwSize = sizeof(WhichControls);
WhichControls.dwICC = (ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | /* ICC_STANDARD_CLASSES */ 0x00004000 | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS);

BackgroundGrey = (HBRUSH)CreateSolidBrush(WINGDI_RGB(242, 241, 233));

InitCommonControlsEx(&WhichControls);
f3dDialogs = InternalDraw3dColorChange(TRUE);

XPGreyButtonDisabled = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_DISABLED");
XPGreyButtonHot = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_HOT");
XPGreyButtonNormal = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_NORMAL");
XPGreyButtonPressed = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_PRESSED");
XPGreyButtonDisabledLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_DISABLEDLG");
XPGreyButtonHotLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_HOTLG");
XPGreyButtonNormalLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_NORMALLG");
XPGreyButtonPressedLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPGREYBUTTON_PRESSEDLG");

XPCloseButtonHot = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_HOT");
XPCloseButtonNormal = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_NORMAL");
XPCloseButtonPressed = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_PRESSED");
XPCloseButtonHotLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_HOTLG");
XPCloseButtonNormalLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_NORMALLG");
XPCloseButtonPressedLg = LoadBitmap((HINSTANCE)GUI->Instance(), "IDB_XPCLOSEBUTTON_PRESSEDLG");

_BoldListBoxFont = _ItalicListBoxFont = NULL;

// The ToolButton Toolbar-Icon widget
//WidgetDebug = 0;
MasterTip = NULL;

for (WPenInit = 0; WPenInit < 8; WPenInit++)
	{
	switch (WPenInit)
		{
		case WCS_WIDGET_COLOR_BKG:
			{
			R = 0x88; G = 0x99; B = 0xbb;
			break;
			} // 
		case WCS_WIDGET_COLOR_BLACK:
			{
			R = 0; G = 0; B = 0;
			break;
			} // 
		case WCS_WIDGET_COLOR_WHITE:
			{
			R = 0xff; G = 0xff; B = 0xff;
			break;
			} // 
		case WCS_WIDGET_COLOR_RED:
			{
			R = 0xff; G = 0; B = 0;
			break;
			} // 
		case WCS_WIDGET_COLOR_DKBLUE:
			{
			R = 0x33; G = 0x44; B = 0x88;
			break;
			} // 
		case WCS_WIDGET_COLOR_GREEN:
			{
			R = 0; G = 0xff; B = 0;
			break;
			} // 
		case WCS_WIDGET_COLOR_LTBLUE:
			{
			R = 0x33; G = 0x77; B = 0xcc;
			break;
			} // 
		case WCS_WIDGET_COLOR_YELLOW:
			{
			R = 0xff; G = 0xff; B = 0x00;
			break;
			} // 
		default:
			break;
		} // switch

	if ((WidgetPens[WPenInit] = CreatePen(PS_SOLID, 1, WINGDI_RGB(R, G, B))) == 0)
		{
		// no error handling here
		} // if
	} // WPenInit

if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".ToolButton", &InquireRegister))
	{
	InquireRegister.style			= CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= ToolButtonWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL; // (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".ToolButton";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 1;
		} // if
	} // if

// The FloatInt Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".FloatInt", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.hInstance		= GUI->Instance();

	InquireRegister.lpfnWndProc		= FloatIntWndProc;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".FloatInt";

	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 16;
		} // if
	} // if

// The Link Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".Link", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.hInstance		= GUI->Instance();

	InquireRegister.lpfnWndProc		= LinkWidgetWndProc;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".Link";

	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 16;
		} // if
	} // if


// The AnimGradient Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".AnimGradient", &InquireRegister))
	{
	InquireRegister.style			= CS_DBLCLKS;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.hInstance		= GUI->Instance();

	InquireRegister.lpfnWndProc		= AnimGradientWndProc;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".AnimGradient";

	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 16;
		} // if
	} // if

// The ColorBar
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".ColorBar", &InquireRegister))
	{
	InquireRegister.style			= CS_PARENTDC;
	InquireRegister.lpfnWndProc		= ColorBarWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".ColorBar";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 32;
		} // if
	} // if

// The StatusBar
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".StatusBar", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= StatusBarWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".StatusBar";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		} // if
	} // if

// The WinCaptionButtonBar
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".WinCaptionButtonBar", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= WinCaptionButtonBarWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	//InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".WinCaptionButtonBar";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		} // if
	} // if



// The SplitBar
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".SplitBar", &InquireRegister))
	{
	InquireRegister.style			= CS_DBLCLKS;
	InquireRegister.lpfnWndProc		= SplitBarWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".SplitBar";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		} // if
	} // if


// The DiskDir Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".DiskDir", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= DiskDirWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".DiskDir";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 64;
		} // if
	} // if

// The TextColumnMarker
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".TextColumnMarker", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= TextColumnMarkerWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4;
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	//InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.hbrBackground	= NULL;
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".TextColumnMarker";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		} // if
	} // if


// The OriginalEdit Widget
if (GetClassInfo(NULL, "EDIT", &InquireRegister))
	{
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".OriginalEdit";
	InquireRegister.lpfnWndProc		= UnModifiedEditWndProc;
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 64;
		} // if
	} // if

if (!EditWndProc)
	{
	if (GetClassInfo(NULL, "EDIT", &InquireRegister))
		{
		EditWndProc = InquireRegister.lpfnWndProc;
		} // if
	// SlightlyModifiedEditWndProc
	if (TempWindow = CreateWindow("EDIT", NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		SetClassLong(TempWindow, GCL_WNDPROC, (long)SlightlyModifiedEditWndProc);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if
if (!ScrollWndProc)
	{
	if (GetClassInfo(NULL, "SCROLLBAR", &InquireRegister))
		{
		ScrollWndProc = InquireRegister.lpfnWndProc;
		} // if
	if (TempWindow = CreateWindow("SCROLLBAR", NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		SetClassLong(TempWindow, GCL_WNDPROC, (long)ModifiedScrollWndProc);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if


if (!TabWndProc)
	{
	if (GetClassInfo(NULL, WC_TABCONTROL, &InquireRegister))
		{
		TabWndProc = InquireRegister.lpfnWndProc;
		} // if
	if (TempWindow = CreateWindow(WC_TABCONTROL, NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		SetClassLong(TempWindow, GCL_WNDPROC, (long)ModifiedTabWndProc);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if

if (!ListViewWndProc)
	{
	// setup ColumnedListView
	if (GetClassInfo(GUI->Instance(), WC_LISTVIEW, &InquireRegister))
		{
		ListViewWndProc = InquireRegister.lpfnWndProc;
		InquireRegister.lpfnWndProc		= ColumnedListViewWndProc;
		InquireRegister.lpszClassName	= APP_CLASSPREFIX ".ColumnedListView";
		ListViewOriginalWndExtra		= InquireRegister.cbWndExtra;
		InquireRegister.cbWndExtra		+= 4;
		if (RegisterClass(&InquireRegister) == NULL)
			{
			//WidgetDebug |= 64;
			} // if
		} // if

	// setup Grid Widget
	if (GetClassInfo(GUI->Instance(), WC_LISTVIEW, &InquireRegister))
		{
		if (GetClassInfo(NULL, WC_LISTVIEW, &InquireRegister))
			{
			InquireRegister.lpszClassName	= APP_CLASSPREFIX ".GridWidget";
			InquireRegister.lpfnWndProc		= GridWidgetWndProc;
			InquireRegister.cbWndExtra		+= 4;
			if (RegisterClass(&InquireRegister) == NULL)
				{
				//WidgetDebug |= 64;
				} // if
			} // if
		} // if
	} // if


if (!TreeWndProc)
	{
	if (GetClassInfo(NULL, WC_TREEVIEW, &InquireRegister))
		{
		TreeWndProc = InquireRegister.lpfnWndProc;
		} // if
	if (TempWindow = CreateWindow(WC_TREEVIEW, NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		SetClassLong(TempWindow, GCL_WNDPROC, (long)ModifiedTreeWndProc);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if

if (!ListWndProc)
	{
	if (GetClassInfo(NULL, "LISTBOX", &InquireRegister))
		{
		ListWndProc = InquireRegister.lpfnWndProc;
		} // if
	if (TempWindow = CreateWindow("LISTBOX", NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		SetClassLong(TempWindow, GCL_WNDPROC, (long)ModifiedListWndProc);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if


// May need to identify real "themed" button wndproc on XP and later
if (!RealButtonWndProc)
	{
	if (TempWindow = CreateWindow("BUTTON", NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		RealButtonWndProc = (WNDPROC)GetClassLong(TempWindow, GCL_WNDPROC);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if

// same trick to pick up toolbar class WndProc
if (!ToolbarWndProc)
	{
	if (TempWindow = CreateWindow(TOOLBARCLASSNAME, NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		ToolbarWndProc = (WNDPROC)GetClassLong(TempWindow, GCL_WNDPROC);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if

// Ditto, ComboWndProc
if (!ComboWndProc)
	{
	if (TempWindow = CreateWindow("COMBOBOX", NULL, NULL,
	 0, 0, 0, 0, NULL, NULL, GUI->Instance(), NULL))
		{
		ComboWndProc = (WNDPROC)GetClassLong(TempWindow, GCL_WNDPROC);
		DestroyWindow(TempWindow);
		TempWindow = NULL;
		} // if
	} // if



// The SmartCheck Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".SmartCheck", &InquireRegister))
	{
	if (GetClassInfo(NULL, "BUTTON", &InquireRegister))
		{
		ButtonWndExtra = InquireRegister.cbWndExtra;
		ButtonWndProc = InquireRegister.lpfnWndProc;
		InquireRegister.cbWndExtra		+= 4;
		InquireRegister.hInstance		= GUI->Instance();
		InquireRegister.lpfnWndProc		= SmartCheckWndProc;
		InquireRegister.lpszClassName	= APP_CLASSPREFIX ".SmartCheck";
		if (RegisterClass(&InquireRegister) == NULL)
			{
			//WidgetDebug |= 128;
			} // if
		} // if
	} // if


// The SmartRadio Widget
if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".SmartRadio", &InquireRegister))
	{
	if (GetClassInfo(NULL, "BUTTON", &InquireRegister))
		{
		ButtonWndExtra = InquireRegister.cbWndExtra;
		InquireRegister.cbWndExtra		+= 4;
		InquireRegister.hInstance		= GUI->Instance();
		InquireRegister.lpfnWndProc		= SmartRadioWndProc;
		InquireRegister.lpszClassName	= APP_CLASSPREFIX ".SmartRadio";
		if (RegisterClass(&InquireRegister) == NULL)
			{
			//WidgetDebug |= 256;
			} // if
		} // if
	} // if

// The AnimGraph widget

if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".AnimGraph", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= AnimGraphWidgetWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4; // Pointer to real data
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".AnimGraph";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 2048;
		} // if
	} // if

// The RasterDraw widget

if (!GetClassInfo(GUI->Instance(), APP_CLASSPREFIX ".RasterDraw", &InquireRegister))
	{
	InquireRegister.style			= 0;
	InquireRegister.lpfnWndProc		= RasterWidgetWndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 4; // Pointer to real data
	InquireRegister.hInstance		= GUI->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= NULL;
	InquireRegister.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".RasterDraw";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 2048;
		} // if
	} // if

// The PopDrop panel class
if (GetClassInfo(NULL, "#32770", &InquireRegister)) // subclassing the standard Dialog Box class to add a dropshadow
	{
	InquireRegister.style			|= CS_DROPSHADOW; // add dropshadow
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".PopDropDialogClass";
	if (RegisterClass(&InquireRegister) == NULL)
		{
		//WidgetDebug |= 2048;
		} // if
	} // if


MasterTip = CreateWindow(TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP,
 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
 NULL, NULL, GUI->Instance(), NULL);
 SendMessage(MasterTip, TTM_ACTIVATE, 1, 0);

} // WidgetLib::WidgetLib
#endif // _WIN32

/*===========================================================================*/

#ifdef _WIN32

HMENU TVPopupMenu, SRAHPopupMenu;

WidgetLib::~WidgetLib()
{
int WPenClean;

DeleteObject(_BoldListBoxFont);
_BoldListBoxFont = NULL;
DeleteObject(_ItalicListBoxFont);
_ItalicListBoxFont = NULL;

DeleteObject(XPGreyButtonDisabled);
DeleteObject(XPGreyButtonHot);
DeleteObject(XPGreyButtonNormal);
DeleteObject(XPGreyButtonPressed);
DeleteObject(XPGreyButtonDisabledLg);
DeleteObject(XPGreyButtonHotLg);
DeleteObject(XPGreyButtonNormalLg);
DeleteObject(XPGreyButtonPressedLg);
XPGreyButtonDisabled = XPGreyButtonHot = XPGreyButtonNormal = XPGreyButtonPressed = NULL;
XPGreyButtonDisabledLg = XPGreyButtonHotLg = XPGreyButtonNormalLg = XPGreyButtonPressedLg = NULL;

DeleteObject(XPCloseButtonHot);
DeleteObject(XPCloseButtonNormal);
DeleteObject(XPCloseButtonPressed);
DeleteObject(XPCloseButtonHotLg);
DeleteObject(XPCloseButtonNormalLg);
DeleteObject(XPCloseButtonPressedLg);
XPCloseButtonHot = XPCloseButtonNormal = XPCloseButtonPressed = NULL;
XPCloseButtonHotLg = XPCloseButtonNormalLg = XPCloseButtonPressedLg = NULL;

if (TVPopupMenu) DestroyMenu(TVPopupMenu); TVPopupMenu = NULL;
if (SRAHPopupMenu) DestroyMenu(SRAHPopupMenu); SRAHPopupMenu = NULL;

if (BackgroundGrey) DeleteObject(BackgroundGrey);
BackgroundGrey = NULL;

if (MasterTip) DestroyWindow(MasterTip);
MasterTip = NULL;

ReleaseRastBlast();

UnregisterClass(APP_CLASSPREFIX ".Link", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".SmartRadio", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".SmartCheck", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".DiskDir", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".SplitBar", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".StatusBar", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".WinCaptionButtonBar", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".ColorBar", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".AnimGradient", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".FloatInt", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".ToolButton", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".RasterDraw", GlobalApp->WinSys->Instance());
UnregisterClass(APP_CLASSPREFIX ".PopDropDialogClass", GlobalApp->WinSys->Instance());

for (WPenClean = 0; WPenClean < 8; WPenClean++)
	{
	if (WidgetPens[WPenClean])
		{
		DeleteObject(WidgetPens[WPenClean]);
		WidgetPens[WPenClean] = NULL;
		} // if
	} // WPenClean

// Clean up Draw3d stuff
DeleteObjects();
} // WidgetLib::~WidgetLib
# endif // _WIN32

/*===========================================================================*/

// This is a big mess to implement Icon-based buttons
// Why this isn't built into Windoze, I don't understand.

void ConfigureTB(NativeControl Dest, int Target, int Normal, int Select, Raster *Thumb)
{
NativeControl MyTB;

# ifdef _WIN32
HICON NormalIcon = NULL, SelectIcon = NULL;
# endif // _WIN32

memset(&TBCfg, 0, sizeof(struct ToolButtonConfig));
MyTB = NULL;

if (Dest && Target)
	{
	//MyTB = GetDlgItem(Dest, Target);
	GUIFenetre *GF;

#       ifdef _WIN32
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		MyTB = GF->GetWidgetFromID(Target);
		} // if
#       endif // _WIN32
	} // if
else if (Dest)
	{
	MyTB = Dest;
	} // else if
	
if (MyTB)
	{
	if (Thumb)
		{
		if (! Thumb->GetPreppedStatus() && ! Thumb->ThumbnailValid())
			{
			//GlobalApp->MCP->GoModal();
			Thumb->LoadnPrepImage(FALSE, FALSE);
			//GlobalApp->MCP->EndModal();
			} // if
		TBCfg.Normal = (HICON)Thumb;
		TBCfg.Hilite = NULL;
		} // if

#       ifdef _WIN32
	else if (GetWindowLong(MyTB, GWL_STYLE) & WCSW_TB_STYLE_ISTHUMB)
		{
		TBCfg.Normal = (HICON)Thumb;
		TBCfg.Hilite = NULL;
		} // else if
	else
		{
		if (NormalIcon = (HICON)LoadImage((HINSTANCE)GetWindowLong(Dest, GWL_HINSTANCE), MAKEINTRESOURCE(Normal), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED))
			{
			TBCfg.Normal = NormalIcon;
			} // if
		if (Select)
			{
			if (SelectIcon = (HICON)LoadImage((HINSTANCE)GetWindowLong(Dest, GWL_HINSTANCE), MAKEINTRESOURCE(Select), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED))
				{
				TBCfg.Hilite = SelectIcon;
				} // if
			} // if
		} // else

	SendMessage(MyTB, WM_WCSW_TB_SETUP, 0, (LPARAM)&TBCfg);
#       endif

	} // if

} // ConfigureTB

/*===========================================================================*/

static char StatusBarString[100];
static char TooltipTransferBuf[4096]; // pain in the butt to dynamically reallocate in-situ

static DiagnosticData TNailData;
static NotifyTag TNailChanges[2];
# ifdef _WIN32
long WINAPI ToolButtonWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;

WORD ID, X, Y;
NativeControl Parent;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
NMTTDISPINFO *lpDispInfo;
char New = 0, OutBounds = 0;
TrueColorIcon *TCI;
unsigned long WStyle;
struct ToolButtonConfig *WThis, *TBC;
int nVirtKey;

Parent  = GetParent(hwnd);
ID      = (WORD) GetWindowLong(hwnd, GWL_ID);
WThis = (struct ToolButtonConfig *)GetWindowLong(hwnd, 0);

WStyle  = GetWindowLong(hwnd, GWL_STYLE);

TCI = NULL;
if (WStyle & WCSW_TB_STYLE_TRUECOLOR)
	{
	if (WThis)
		{
		TCI  = (TrueColorIcon *)WThis->Normal;
		} // if
	} // if


GetClientRect(hwnd, &Square);

switch (message)
	{
	case WM_NOTIFY:
		{
		if (lpDispInfo = (LPNMTTDISPINFO)lParam)
			{
			if (lpDispInfo->hdr.code == TTN_GETDISPINFO)
				{
				if (GetWindowTextLength(hwnd) < 80)
					{
					GetWindowText(hwnd, lpDispInfo->szText, 80);
					} // if
				else // longer strings
					{
					GetWindowText(hwnd, TooltipTransferBuf, 4095);
					lpDispInfo->lpszText = TooltipTransferBuf;
					} // else
				SendMessage(MasterTip, TTM_SETMAXTIPWIDTH, (WPARAM)0, (LPARAM)480);
				// if we're a conventional icon button display our icon, but omit a "title" (not the same as the tooltip text)
				if (!(WStyle & WCSW_TB_STYLE_TRUECOLOR) && !(WStyle & WCSW_TB_STYLE_ISTHUMB) && (WThis->Normal))
					{
					SendMessage(MasterTip, TTM_SETTITLE, (WPARAM)WThis->Normal, (LPARAM)" ");
					} // if
				} // if
			else if (lpDispInfo->hdr.code == TTN_SHOW)
				{
				SetWindowPos(MasterTip, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				} // if
			else if (lpDispInfo->hdr.code == TTN_POP)
				{
				SendMessage(MasterTip, TTM_SETTITLE, (WPARAM)TTI_INFO, (LPARAM)"");
				} // if
			} // if
		Handled = 1;
		break;
		} // NOTIFY
	case WM_SETFONT:
		{
		WThis->Font = (HFONT)wParam;
		Handled = 1;
		HCti.cbSize = sizeof(TOOLINFO);
		HCti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		HCti.lpszText = LPSTR_TEXTCALLBACK;
		HCti.hwnd = hwnd;
		HCti.uId = (UINT)hwnd;
		HCti.hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
		SendMessage(MasterTip, TTM_ADDTOOL, 0, (LPARAM)&HCti);
		break;
		} // SETFONT
/*
	case WM_ERASEBKGND:
		{
		return(1);
		} // WM_ERASEBKGND
*/
	case WM_WCSW_TB_SETUP:
		{
		TBC = (struct ToolButtonConfig *)lParam;
		WThis->Normal = TBC->Normal;
		WThis->Hilite = TBC->Hilite;
		InvalidateParentRect(hwnd);
		//InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // WM_WCSW_TB_SETUP
	case WM_KILLFOCUS:
		{
		InvalidateParentRect(hwnd);
		//InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // KILLFOCUS
	case WM_PAINT:
		{
		if (GetUpdateRect(hwnd, NULL, 0))
			{
			Canvas = BeginPaint(hwnd, &PS);
			if (WStyle & WCSW_TB_STYLE_ISTHUMB)
				{
				GlobalApp->WinSys->WL->DrawThumbNail(hwnd, (Raster *)WThis->Normal, WThis->State);
				} // if
			else
				{
				GlobalApp->WinSys->WL->DrawToolButtonImage(Canvas, hwnd, WThis->Normal, WThis->Hilite, TCI, WThis->State, (WStyle & WCSW_TB_STYLE_XPLOOK) ? true : false, WThis->Hot ? true : false);
				} // if
			if (!(WStyle & WCSW_TB_STYLE_NOCHANGE))
				DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
			EndPaint(hwnd, &PS);
			} // if
		Handled = 1;
		break;
		} // PAINT
	case WM_SETFOCUS:
		{
		SetFocus(hwnd);
		InvalidateParentRect(hwnd);
		//InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // SETFOCUS
	case WM_SHOWWINDOW:
		{
		long Result = DefWindowProc(hwnd, message, wParam, lParam);
		Handled = 1;
		break;
		// fall through to MOUSEMOVE below to process hot/capture conditions
		} // WM_SHOW
	case WM_MOUSEMOVE:
		{
		POINT CursorPos;
		GetCursorPos(&CursorPos);
		ScreenToClient(hwnd, &CursorPos);
		X = LOWORD(lParam); Y = HIWORD(lParam);
		X = (WORD)CursorPos.x; Y = (WORD)CursorPos.y;
		if (X > Square.right || X < 1 || Y > Square.bottom || Y < 1)
			{
			OutBounds = 1;
			if (WThis->Hot)
				{
				WThis->Hot = false;
				New = 1;
				} // if
			} // if
		else
			{
			if (!WThis->Hot)
				{
				WThis->Hot = true;
				New = 1;
				} // if
			OutBounds = 0;
			if (!WThis->Capture)
				{
				SetCapture(hwnd);
				WThis->Capture = 1;
				} // if
			if (!WThis->State && !(WStyle & WCSW_TB_STYLE_TOG))
				{
				if (wParam & MK_LBUTTON)
					{
					if ((!WThis->State) && (WThis->Trigger))
						{
						WThis->State = 1;
						New = 1;
						} // if
					} // if
				} // if
			} // else
		if (OutBounds)
			{
			WThis->Trigger = 0;
			if (!WThis->State || (WStyle & WCSW_TB_STYLE_TOG) || (WStyle & WCSW_TB_STYLE_NOCHANGE))
				{
				ReleaseCapture();

				WThis->Capture = 0;
				} // if
			if (WThis->State && !(WStyle & WCSW_TB_STYLE_TOG))
				{
				if (!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
					{
					WThis->State = 0;
					New = 1;
					} // if
				} // if
			} // if
		break;
		} // MOUSEMOVE
	case WM_CANCELMODE:
		{
		WThis->Trigger = 0;
		if (WThis->Capture)
			{
			ReleaseCapture();
			WThis->Capture = 0;
			} // if
		break;
		} // CANCELMODE
	case WM_ENABLE:
		{
		New = 1; // force a redraw: "The WM_ENABLE message is sent when an application changes the enabled state of a window. It is sent to the window whose enabled state is changing."
		break;
		} // WM_ENABLE
	case BM_GETCHECK:
		{
		Handled = 1;
		return(WThis->State);
		} // GETCHECK
	case BM_SETCHECK:
		{
		Handled = 1;
		if (WThis->State != (char)(wParam != 0))
			{
			WThis->State = (char)(wParam != 0);
			New = 1;
			} // if
		break;
		} // SETCHECK
	case WM_KEYDOWN:
		{
		nVirtKey = (int)wParam;
		if (nVirtKey != ' ')
			{
			break;
			}
		if (WStyle & WCSW_TB_STYLE_NOCHANGE)
			{
			// Button inhibited
			break;
			} // if
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{ // CTRL+SPACE = CTRL+CLICK
			PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_HILITE << 16), (LONG)hwnd);
			} // if
		else
			{
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				{ // SHIFT+SPACE = DBLCLICK
				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_DBLCLK << 16), (LONG)hwnd);
				} // if
			else
				{
				if (WStyle & WCSW_TB_STYLE_TOG)
					{
					WThis->State = !WThis->State; New = 1;
	 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // if
				else
					{
					Canvas = GetDC(hwnd);
					GlobalApp->WinSys->WL->DrawToolButtonImage(Canvas, hwnd, WThis->Normal, WThis->Hilite, TCI, 1, (WStyle & WCSW_TB_STYLE_XPLOOK) ? true : false, WThis->Hot ? true : false);
					if (!(WStyle & WCSW_TB_STYLE_NOCHANGE))
						DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
					Sleep(50);
					GlobalApp->WinSys->WL->DrawToolButtonImage(Canvas, hwnd, WThis->Normal, WThis->Hilite, TCI, 0, (WStyle & WCSW_TB_STYLE_XPLOOK) ? true : false, WThis->Hot ? true : false);
					if (!(WStyle & WCSW_TB_STYLE_NOCHANGE))
						DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
					ReleaseDC(hwnd, Canvas);
					PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // else
				} // else
			} // else
		break;
		} // WM_CHAR
	case WM_RBUTTONDOWN:
		{
		// fall through to LBUTTON code if RBUTTON permitted, else break
		if (!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
		} // RBUTTONDOWN
		//lint -fallthrough
	case WM_LBUTTONDOWN:
		{
		if (!(WStyle & WCSW_TB_STYLE_NOFOCUS)) SetFocus(hwnd);
		if (!(wParam & MK_CONTROL))
			{
			if (!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
				{
				if (WStyle & WCSW_TB_STYLE_TOG)
					{
					WThis->State = !WThis->State; New = 1;
					X = LOWORD(lParam); Y = HIWORD(lParam);
					if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
		 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // if
				else
					{
					WThis->State = 1; New = 1;
					WThis->Trigger = 1;
					} // else
				} // if
			else if (WStyle & WCSW_TB_STYLE_ISTHUMB)
				{
				X = LOWORD(lParam); Y = HIWORD(lParam);
				if (TNailData.ValueValid[WCS_DIAGNOSTIC_RGB] = GlobalApp->WinSys->WL->SampleTNailColor(hwnd, (Raster *)WThis->Normal, X, Y, TNailData.DataRGB[0], TNailData.DataRGB[1], TNailData.DataRGB[2]))
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, &TNailData);
					} // if
				if (WThis->Normal && ((Raster *)WThis->Normal)->Thumb)
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, ((Raster *)WThis->Normal)->Thumb);
					} // if
				} // else
			} // if
		break;
		} // LBUTTONDOWN
	case WM_CREATE:
		{
		if (WThis = (struct ToolButtonConfig *)AppMem_Alloc(sizeof(struct ToolButtonConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, 0, (long)WThis);
			return(0); // hunkidori
			} // if
		else
			{
			SetWindowLong(hwnd, 0, (long)NULL);
			return(-1); // bah, humbug
			} // else
		} // CREATE
	case WM_DESTROY:
		{
		if (WStyle & WCSW_TB_STYLE_TRUECOLOR)
			{
			if (TCI && TCI->BitArray)
				{
				AppMem_Free(TCI->BitArray, TCI->BASize);
				TCI->BitArray = NULL;
				TCI->BASize = NULL;
				} // if
			} // if
		HCti.hwnd = hwnd;
		HCti.uId = (UINT)hwnd;
		HCti.hinst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
		SendMessage(MasterTip, TTM_DELTOOL, 0, (LPARAM)&HCti);
		SetWindowLong(hwnd, 0, 0);
		AppMem_Free(WThis, sizeof(struct ToolButtonConfig));
		return(0);
		} // DESTROY
	case WM_RBUTTONDBLCLK:
		{
		// fall through to LBUTTON code if RBUTTON permitted, else break
		if (!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
		} // WM_RBUTTONDBLCLK
		//lint -fallthrough
	case WM_LBUTTONDBLCLK:
		{
		if (!(wParam & MK_CONTROL))
			{
			X = LOWORD(lParam); Y = HIWORD(lParam);
			if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
				{
				// send a single-click first, in case we're not paying attention to DBLCLICK
 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
				// Send the DBLCLICK
				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_DBLCLK << 16), (LONG)hwnd);
				} // if
			} // if
		break;
		} // WM_LBUTTONDBLCLK
	case WM_RBUTTONUP:
		{
		// fall through to LBUTTON code if RBUTTON permitted, else break
		if (!(WStyle & WCSW_TB_STYLE_ALLOWRIGHT))
			{
			break;
			} // if
		} // WM_RBUTTONUP
		//lint -fallthrough
	case WM_LBUTTONUP:
		{
		X = LOWORD(lParam); Y = HIWORD(lParam);
		if (wParam & MK_CONTROL)
			{ // Misuse the BN_HILITE notify
			if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
				{
				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_HILITE << 16), (LONG)hwnd);
				} // if
			} // if
		else if (!(WStyle & WCSW_TB_STYLE_TOG))
			{
			if (WThis->Capture)
				{
				ReleaseCapture();
				WThis->Capture = 0;
				if (!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
					{
					WThis->State = 0; New = 1;
					if (WThis->Trigger)
						{
						if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
							{
							WNDPROC ControlWndProc;
							ControlWndProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
							PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
							} // if
						WThis->Trigger = 0;
						} // if
					} // if
				} // if
			} // if
		break;
		} // LBUTTONUP
	} // switch

if (New)
	{
	InvalidateParentRect(hwnd);
	
	/*
	Canvas = GetDC(hwnd);
	if (WStyle & WCSW_TB_STYLE_ISTHUMB)
		{
		GlobalApp->WinSys->WL->DrawThumbNail(hwnd, (Raster *)WThis->Normal, WThis->State);
		} // if
	else
		{
		GlobalApp->WinSys->WL->DrawToolButtonImage(Canvas, hwnd, WThis->Normal, WThis->Hilite, TCI, WThis->State, (WStyle & WCSW_TB_STYLE_XPLOOK) ? true : false, WThis->Hot ? true : false);
		} // if
	if (!(WStyle & WCSW_TB_STYLE_NOCHANGE))
		DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
	ReleaseDC(hwnd, Canvas);
	*/
	New = 0;
	} // if


if (!Handled)
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // if
else
	{
	return(0);
	} // else

} // ToolButtonWndProc
# endif // _WIN32

# ifdef _WIN32

//int WarnNull;
//BITMAP BM;

/*===========================================================================*/

void WidgetLib::DrawToolButtonImage(NativeDrawContext ButtonDC, NativeControl ButtonWnd, HICON Norm, HICON Hi,
 TrueColorIcon *Img, char State, bool NewStyle, bool Hot)
{
HICON Picasso;
RECT Square;
RC Cola;
unsigned int NomW, NomH, XPaint;
signed int YPaint;
unsigned int Offset, OffsetDIB, LineLength;
unsigned char R, G, B, Shift = 0, GlyphOffset = 2;
LONG Style;
bool BigCaptionButtons = false;
bool AsClose = false;

Style   = GetWindowLong(ButtonWnd, GWL_STYLE);
LONG ID = GetWindowLong(ButtonWnd, GWL_ID);

if (ID == ID_WINMENU_CLOSE) AsClose = true;

Picasso =  Norm;

GetWindowRect(ButtonWnd, &Square);
Cola.xLeft  = 0;
Cola.yTop   = 0;
Cola.xRight = Square.right - Square.left;
Cola.yBot   = Square.bottom - Square.top;
NomW = Cola.xRight;
NomH = Cola.yBot;

if ((Style & WCSW_TB_STYLE_CAPTIONBUTTON) && (NomW > 16))
	{
	BigCaptionButtons = true;
	} // if


if (State)
	{
	if ((Style & WCSW_TB_STYLE_CAPTIONBUTTON) && g_xpStyle.IsAppThemed())
		{
		if (BigCaptionButtons)
			{
			DrawCaptionButtonImage(ButtonDC, ButtonWnd, State, Hot, (Style & WS_DISABLED) ? false : true, true, AsClose);
			} // if
		else
			{
			DrawCaptionButtonImage(ButtonDC, ButtonWnd, State, Hot, (Style & WS_DISABLED) ? false : true, false, AsClose);
			} // else
		} // if
	else
		{
		Draw3dButtonIn(ButtonWnd, ButtonDC, &Cola, 0x03, NewStyle, Hot, (Style & WS_DISABLED) ? false : true);
		} // else
	} // if
else
	{
	if ((Style & WCSW_TB_STYLE_CAPTIONBUTTON) && g_xpStyle.IsAppThemed())
		{
		if (BigCaptionButtons)
			{
			DrawCaptionButtonImage(ButtonDC, ButtonWnd, State, Hot, (Style & WS_DISABLED) ? false : true, true, AsClose);
			} // if
		else
			{
			DrawCaptionButtonImage(ButtonDC, ButtonWnd, State, Hot, (Style & WS_DISABLED) ? false : true, false, AsClose);
			} // else
		} // if
	else
		{
		Draw3dButtonOut(ButtonWnd, ButtonDC, &Cola, 0x03, NewStyle, Hot, (Style & WS_DISABLED) ? false : true);
		} // else
	} // else

if ((State) && (Hi))
	{
	Picasso = Hi;
	} // if

// turns out Offset=1 is probably correct for non-themed (Win2k) CAPTIONBUTTON style too
if (Style & WCSW_TB_STYLE_CAPTIONBUTTON)
	{
	if (BigCaptionButtons)
		{
		GlyphOffset = 5; // Button glyph image is designed to align with frame image this way
		} // if
	else
		{
		GlyphOffset = 1; // Button glyph image is designed to align with frame image this way
		} // else
	} // if

if (Img) // Truecolor icon image?
	{
	if (NomW > Img->Width)
		{
		NomW = Img->Width;
		} // if
	if (NomH > Img->Height)
		{
		NomH = Img->Height;
		} // if
	if (!Img->BitArray) //  && (GetDeviceCaps(ButtonDC, RASTERCAPS) & RC_DIBTODEV)
		{ // setup BitArray
		Img->BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		Img->BMI.bmiHeader.biWidth = NomW;
		Img->BMI.bmiHeader.biHeight = (signed)NomH;
		Img->BMI.bmiHeader.biPlanes = 1;
		Img->BMI.bmiHeader.biBitCount = 24;
		Img->BMI.bmiHeader.biCompression = BI_RGB;
		Img->BMI.bmiHeader.biSizeImage = 3 * NomW * NomH; // could actually be 0...
		Img->BMI.bmiHeader.biXPelsPerMeter = 2834; // 72 dpi
		Img->BMI.bmiHeader.biYPelsPerMeter = 2834;
		Img->BMI.bmiHeader.biClrUsed = 0;
		Img->BMI.bmiHeader.biClrImportant = 0;
		LineLength = (NomW * 3);
		LineLength = ROUNDUP(LineLength, 4);
		Img->BASize = LineLength * NomH;

		if (Img->BitArray = (unsigned char *)AppMem_Alloc(Img->BASize, NULL))
			{
			for (OffsetDIB = 0, YPaint = NomH - 1; YPaint >= 0; YPaint--, OffsetDIB += LineLength)
				{
				Offset = (YPaint * NomW);
				for (XPaint = 0; XPaint < NomW; XPaint++)
					{
					R = Img->R[Offset + XPaint];
					G = Img->G[Offset + XPaint];
					B = Img->B[Offset + XPaint];
					((char *)Img->BitArray)[OffsetDIB + (XPaint * 3)] = B;
					((char *)Img->BitArray)[OffsetDIB + (XPaint * 3) + 1] = G;
					((char *)Img->BitArray)[OffsetDIB + (XPaint * 3) + 2] = R;
					} // for
				} // for

			} // if
		} // if


	if (State == 1) Shift = 1;
	if (Style & (WCSW_TB_STYLE_TOG | WCSW_TB_STYLE_NOCHANGE))
		Shift = 0;
	if ((Style & WCSW_TB_STYLE_CAPTIONBUTTON) && g_xpStyle.IsAppThemed())
		{
		Shift = 0; // XP-style doesn't shift button glyph when pressed
		} // if

	if (Img->BitArray)
		{
		SetDIBitsToDevice(ButtonDC, GlyphOffset + Shift, GlyphOffset + Shift, NomW, NomH, 0, 0, 0,
		 NomH, Img->BitArray, &Img->BMI, DIB_RGB_COLORS);
		} //
	else
		{ // fallback to slow SetPixel plotting
		for (YPaint = 0; YPaint < (signed)NomH; YPaint++)
			{
			Offset = (YPaint * NomW);
			for (XPaint = 0; XPaint < NomW; XPaint++)
				{
				R = Img->R[Offset + XPaint];
				G = Img->G[Offset + XPaint];
				B = Img->B[Offset + XPaint];
				VSetPixel(ButtonDC, XPaint, YPaint, WINGDI_RGB(R, G, B));
				} // for
			} // for
		} // else
	} // if
else
	{
	if (Picasso)
		{
		// this handles greying for disabled state automatically
		DrawState(ButtonDC, NULL, NULL, (LPARAM)Picasso, NULL, GlyphOffset + Shift, GlyphOffset + Shift, 0, 0, DST_ICON | ((Style & WS_DISABLED) ? DSS_DISABLED : DSS_NORMAL));
		} // if
	} // else

if (!NewStyle) // NewStyle themes can overwrite interior, so don't redraw them
	{
	if (State)
		{
		Draw3dButtonIn(ButtonWnd, ButtonDC, &Cola, 0x01, NewStyle, Hot);
		} // if
	else
		{
		Draw3dButtonOut(ButtonWnd, ButtonDC, &Cola, 0x01, NewStyle, Hot);
		} // else
	} // if

// only use old halftone method for deprecated RGB-type icon images, which we're eliminating
if (Img && (Style & WS_DISABLED))
	{
	// Draw over the button with a grey checkerboard screen
	// <<<>>> Deprecated, remove
	UserMessageOK("DEBUG", "Attempting to draw a disable RGB-Truecolor icon. Deprecated.");
	} // if

} // WidgetLib::DrawToolButtonImage

/*===========================================================================*/

void WidgetLib::DrawCaptionButtonImage(NativeDrawContext ButtonDC, NativeControl ButtonWnd, char State, bool Hot, bool Enabled, bool LargeSize, bool AsClose)
{
int Width, Height;
HDC     hMemDC = CreateCompatibleDC(ButtonDC);
HBITMAP ChosenBitmap;

if (LargeSize)
	{
	ChosenBitmap = XPGreyButtonNormalLg;
	if (Hot) ChosenBitmap = XPGreyButtonHotLg;
	if (State) ChosenBitmap = XPGreyButtonPressedLg;
	if (!Enabled) ChosenBitmap = XPGreyButtonDisabledLg;
	if (AsClose)
		{
		ChosenBitmap = XPCloseButtonNormalLg;
		if (Hot) ChosenBitmap = XPCloseButtonHotLg;
		if (State) ChosenBitmap = XPCloseButtonPressedLg;
		} // if
	} // if
else
	{
	ChosenBitmap = XPGreyButtonNormal;
	if (Hot) ChosenBitmap = XPGreyButtonHot;
	if (State) ChosenBitmap = XPGreyButtonPressed;
	if (!Enabled) ChosenBitmap = XPGreyButtonDisabled;
	if (AsClose)
		{
		ChosenBitmap = XPCloseButtonNormal;
		if (Hot) ChosenBitmap = XPCloseButtonHot;
		if (State) ChosenBitmap = XPCloseButtonPressed;
		} // if
	} // if

if (LargeSize)
	{
	Width = 21;
	Height = 21;
	} // if
else
	{
	Width = 14;
	Height = 14;
	} // else

HGDIOBJ hOld   = SelectObject(hMemDC, ChosenBitmap);
TransparentBlt(ButtonDC, 1, 1, Width, Height, hMemDC, 0, 0, Width, Height, WINGDI_RGB(255,0,255)); // no one with taste will ever use this color in a UI button
SelectObject(hMemDC, hOld);
DeleteObject(hMemDC);

} // WidgetLib::DrawCaptionButtonImage

# endif //_WIN32

/*===========================================================================*/

// This is a scratch buffer for blitting.
// Thumbnails bigger than 2048 wide are not supported. Duh.
extern unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];

int WidgetLib::SampleTNailColor(NativeControl hwnd, Raster *Ras, int XC, int YC, unsigned char &ROut, unsigned char &GOut, unsigned char &BOut)
{
RECT XLate;
long W, H;
float X, Y, XInc, YInc;
unsigned long Idx, Index;
NativeControl Stamp;
Raster *Subject;
UBYTE *Red, *Green, *Blue, Gray;
ColorControlShell *ColorCtrl = NULL;
RasterAttribute *MyAttr;

Subject = Ras;
Stamp = hwnd;

X = (float)XC;
Y = (float)YC;

GetWindowRect(Stamp, &XLate);

XLate.right -= (XLate.left + 4);
XLate.bottom -= (XLate.top + 4);
XLate.left = 0;
XLate.top = 0;

if (Subject)
	{
	if (! Subject->GetPreppedStatus() && ! Subject->ThumbnailValid())
		{
		// Not safe to do here, bail out.
		return 0;
		} // if
	W = WCS_RASTER_TNAIL_SIZE;
	H = WCS_RASTER_TNAIL_SIZE;
	if (Subject->ColorControlEnabled && (MyAttr = Subject->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL)))
		{
		ColorCtrl = (ColorControlShell *)MyAttr->GetShell();
		} // if

	if (ColorCtrl && ColorCtrl->UseBandAssignment)
		{
		Red = Subject->Thumb->TNail[ColorCtrl->UseBandAs[0]];
		Green = Subject->Thumb->TNail[ColorCtrl->UseBandAs[1]];
		Blue = Subject->Thumb->TNail[ColorCtrl->UseBandAs[2]];
		} // if
	else
		{
		Red = Subject->Thumb->TNail[0];
		Green = Subject->Thumb->TNail[1];
		Blue = Subject->Thumb->TNail[2];
		} // else

	if (Red == NULL || Green == NULL || Blue == NULL)
		{
		return(0);
		} // if

	XInc = (float)W / (float)(XLate.right - XLate.left + 2);	// + 2 is added to fill the widget - GRH
	YInc = (float)H / (float)(XLate.bottom - XLate.top + 2);	// + 2 is added to fill the widget - GRH

	Idx = (unsigned long)((Y * YInc));
		if (Idx < (unsigned long)Subject->Thumb->TNailPadY || Idx >= (unsigned long)(H - Subject->Thumb->TNailPadY))
			{
			return(0);
			} // if
		else
			{
			Idx = (unsigned long)(Idx * W);
				{
				Index = (int)(X * XInc);
				if (Index < (unsigned long)Subject->Thumb->TNailPadX || Index >= (unsigned long)(W - Subject->Thumb->TNailPadX))
					{
					return(0);
					} // if
				else
					{
					Index = (int)(Idx + Index);
					if (ColorCtrl && ColorCtrl->ShowTransparency && 
							Red[Index] >= ColorCtrl->RGB[1][0] && Red[Index] <= ColorCtrl->RGB[0][0] &&
							Green[Index] >= ColorCtrl->RGB[1][1] && Green[Index] <= ColorCtrl->RGB[0][1] &&
							Blue[Index] >= ColorCtrl->RGB[1][2] && Blue[Index] <= ColorCtrl->RGB[0][2])
						{
						ROut = (unsigned char)ColorCtrl->RGB[2][0];
						GOut = (unsigned char)ColorCtrl->RGB[2][1];
						BOut = (unsigned char)ColorCtrl->RGB[2][2];
						return(1);
						} // if
					else
						{
						if (ColorCtrl && ! ColorCtrl->UseAsColor)
							{
							Gray = (Red[Index] + Green[Index] + Blue[Index]) / 3;
							ROut = Gray; GOut = Gray; BOut = Gray;
							return(1);
							} // if
						else
							{
							ROut = Red[Index]; GOut = Green[Index]; BOut = Blue[Index];
							return(1);
							} // else
						} // else
					} // else
				} // nothing
			} // else
	} // if

return(0);

} // WidgetLib::SampleTNailColor

/*===========================================================================*/

void WidgetLib::DrawThumbNail(NativeControl hwnd, Raster *Ras, char State)
{
float X, Y, XInc, YInc;
unsigned long Idx, Index;
unsigned short Wide;
long W, H;
UBYTE *Red, *Green, *Blue, Gray;
NativeControl Stamp;
RECT XLate;
#ifdef _WIN32
HDC Chalk = NULL;
RC Cola;
#endif // _WIN32
ColorControlShell *ColorCtrl = NULL;
RasterAttribute *MyAttr;
Raster *Subject;

Subject = Ras;
Stamp = hwnd;

#ifdef _WIN32
GetWindowRect(Stamp, &XLate);
Chalk = GetDC(Stamp);

Cola.xLeft  = 0;
Cola.yTop   = 0;
Cola.xRight = XLate.right - XLate.left;
Cola.yBot   = XLate.bottom - XLate.top;

if (State)
	{
	Draw3dButtonIn(hwnd, Chalk, &Cola, 0x01, false, false);
	} // if
else
	{
	Draw3dButtonOut(hwnd, Chalk, &Cola, 0x01, false, false);
	} // else


XLate.right -= (XLate.left + 4);
XLate.bottom -= (XLate.top + 4);
#endif // _WIN32

XLate.left = 0;
XLate.top = 0;

if (Subject)
	{
	if (! Subject->GetPreppedStatus() && ! Subject->ThumbnailValid())
		{
		// Not safe to do here, bail out.
		return;
		//GlobalApp->MCP->GoModal();
		//Subject->LoadnPrepImage(FALSE);
		//GlobalApp->MCP->EndModal();
		} // if
	W = WCS_RASTER_TNAIL_SIZE;
	H = WCS_RASTER_TNAIL_SIZE;
	if (Subject->ColorControlEnabled && (MyAttr = Subject->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL)))
		{
		ColorCtrl = (ColorControlShell *)MyAttr->GetShell();
		} // if

	if (ColorCtrl && ColorCtrl->UseBandAssignment)
		{
		Red = Subject->Thumb->TNail[ColorCtrl->UseBandAs[0]];
		Green = Subject->Thumb->TNail[ColorCtrl->UseBandAs[1]];
		Blue = Subject->Thumb->TNail[ColorCtrl->UseBandAs[2]];
		} // if
	else
		{
		Red = Subject->Thumb->TNail[0];
		Green = Subject->Thumb->TNail[1];
		Blue = Subject->Thumb->TNail[2];
		} // else

	if (Red == NULL || Green == NULL || Blue == NULL)
		{
#ifdef _WIN32
		if (Chalk)
			{
			ReleaseDC(Stamp, Chalk);
			} // if
#endif // _WIN32
		return;
		} // if


	XInc = (float)W / (float)(XLate.right - XLate.left + 2);	// + 2 is added to fill the widget - GRH
	YInc = (float)H / (float)(XLate.bottom - XLate.top + 2);	// + 2 is added to fill the widget - GRH

	for (Y = (float)XLate.top; Y < XLate.bottom; Y++)
		{
		Idx = (unsigned long)((Y * YInc));
		Wide = 0;
		if (Idx < (unsigned long)Subject->Thumb->TNailPadY || Idx >= (unsigned long)(H - Subject->Thumb->TNailPadY))
			{
			for (X = (float)XLate.left; X < XLate.right; X++)
				{
				ThumbNailR[(int)X] = 125; ThumbNailG[(int)X] = 125; ThumbNailB[(int)X] = 125; Wide = (unsigned short)X;
				//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(125, 125, 125));
				} // for
			} // if
		else
			{
			Idx = (unsigned long)(Idx * W);
			for (X = (float)XLate.left; X < XLate.right; X++)
				{
				Index = (int)(X * XInc);
				if (Index < (unsigned long)Subject->Thumb->TNailPadX || Index >= (unsigned long)(W - Subject->Thumb->TNailPadX))
					{
					ThumbNailR[(int)X] = 125; ThumbNailG[(int)X] = 125; ThumbNailB[(int)X] = 125;  Wide = (unsigned short)X;
					//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(125, 125, 125));
					} // if
				else
					{
					Index = (int)(Idx + Index);
					if (ColorCtrl && ColorCtrl->ShowTransparency && 
							Red[Index] >= ColorCtrl->RGB[1][0] && Red[Index] <= ColorCtrl->RGB[0][0] &&
							Green[Index] >= ColorCtrl->RGB[1][1] && Green[Index] <= ColorCtrl->RGB[0][1] &&
							Blue[Index] >= ColorCtrl->RGB[1][2] && Blue[Index] <= ColorCtrl->RGB[0][2])
						{
						ThumbNailR[(int)X] = (unsigned char)ColorCtrl->RGB[2][0];
						ThumbNailG[(int)X] = (unsigned char)ColorCtrl->RGB[2][1];
						ThumbNailB[(int)X] = (unsigned char)ColorCtrl->RGB[2][2];
						Wide = (unsigned short)X;
						//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(ColorCtrl->RGB[2][0], ColorCtrl->RGB[2][1], ColorCtrl->RGB[2][2]));
						} // if
					else
						{
						if (ColorCtrl && ! ColorCtrl->UseAsColor)
							{
							Gray = (Red[Index] + Green[Index] + Blue[Index]) / 3;
							ThumbNailR[(int)X] = Gray; ThumbNailG[(int)X] = Gray; ThumbNailB[(int)X] = Gray; Wide = (unsigned short)X;
							//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(Gray, Gray, Gray));
							} // if
						else
							{
							ThumbNailR[(int)X] = Red[Index]; ThumbNailG[(int)X] = Green[Index]; ThumbNailB[(int)X] = Blue[Index]; Wide = (unsigned short)X;
							//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(Red[Index], Green[Index], Blue[Index]));
							} // else
						} // else
					} // else
				} // for
			} // else
#ifdef _WIN32
		RastBlast(Chalk, (unsigned short)(XLate.left + 2), (unsigned short)(Y + 2), Wide,
		 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
#endif // _WIN32
		} // for
	} // if Subject
else
	{
	#ifdef _WIN32
	for (Y = (float)XLate.top; Y < XLate.bottom; Y++)
		{
		Wide = 0;
		for (X = (float)XLate.left; X < XLate.right; X++)
			{
			ThumbNailR[(int)X] = 175; ThumbNailG[(int)X] = 175; ThumbNailB[(int)X] = 175; Wide = (unsigned short)X;
			//VSetPixel(Chalk, (int)X + 2, (int)Y + 2, WINGDI_RGB(175, 175, 175));
			} // for
		RastBlast(Chalk, (unsigned short)(XLate.left + 2), (unsigned short)(Y + 2), Wide,
		 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
		} // for
	#endif // _WIN32
	} // else

#ifdef _WIN32
if (Chalk)
	{
	ReleaseDC(Stamp, Chalk);
	} // if
#endif // _WIN32

} // WidgetLib::DrawThumbNail

/*===========================================================================*/

// This is used as a quick non-dynamic conversion buffer by the
// FloatInt WndProc
static double FloatIntDummyBuf;

void ConfigureFI(NativeControl Dest, int Target, void *MV, double IDA, double Min, double Max, unsigned long Flags,
 SetCritter *Crit, unsigned long SetGetID)
{
memset(&Cfg, 0, sizeof(struct FloatIntConfig));
Cfg.MasterVariable = MV;
Cfg.AuxVariable = NULL;
if (fabs(IDA) < 1.0)
	{
	Cfg.SmallIDA = (long)(FI_SmallIDAScale * IDA);
	Flags |= FIOFlag_SmallIncDec;
	Cfg.IncDecAmount = 0.0f;
	} // if
else
	{
	Cfg.SmallIDA = 0;
	Cfg.IncDecAmount = IDA;
	} // else

Cfg.MaxAmount = Max;
Cfg.MinAmount = Min;

Cfg.CritID = SetGetID;
Cfg.FIFlags = Flags;
if ((MV == NULL))
	{
	Cfg.MasterVariable = Crit;
	Cfg.FIFlags |= FIOFlag_Critter;
	} // if

# ifdef _WIN32
if (Target)
	{
	//SendDlgItemMessage(Dest, Target, WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		SendMessage(GF->GetWidgetFromID(Target), WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
		} // if
	} // if
else
	{
	SendMessage(Dest, WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
	} // else
# endif // _WIN32

} // ConfigureFI

/*===========================================================================*/

void ConfigureAG(NativeGUIWin Dest, int Target, AnimGradient *AG)
{

# ifdef _WIN32
if (Target)
	{
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		SendMessage(GF->GetWidgetFromID(Target), WM_WCSW_AG_SETUP, 0, (LPARAM)AG);
		} // if
	} // if
else
	{
	SendMessage(Dest, WM_WCSW_AG_SETUP, 0, (LPARAM)AG);
	} // else
# endif // _WIN32

} // ConfigureAG

/*===========================================================================*/

static unsigned char FoolKeyStates[256];

# ifdef _WIN32

static double SmartNumericPreviousValue;

double GetSNPrevValue(void)
{
return(SmartNumericPreviousValue);
} // GetSNPrevValue

/*===========================================================================*/

// this is safe to call even if it's not an SNAD, it'll just return 0.0
double SNADmGetCurValue(NativeControl Me, struct FloatIntConfig *FIC)
{
double Result = 0.0;
AnimDouble *AD;

if (FIC->FIFlags & FIOFlag_Critter)
	{
	return(Result);
	} // if
else if (FIC->FIFlags & FIOFlag_AnimDouble)
	{
	if (AD = (AnimDouble *)FIC->MasterVariable)
		{
		Result = AD->GetCurValue();
		} // if
	} // else if

return(Result);

} // SNADmGetCurValue

/*===========================================================================*/

long WINAPI FloatIntWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD ExtraButtonWidth, ButtonWidth, EditWidth, LabelWidth, LabelHeight, FieldChars, ColorPot = 0, ColorWidth = 0, GenWidth, GenHeight,
 Notify, MyID;
NativeControl Parent;
LPCREATESTRUCT Create;
HINSTANCE MyInst;
HDC TextDC;
SIZE Ruler;
RECT WinSize;
HFONT TextStyle;
char ExtrasOK, LabelRight = 0, DState = 0;
int Delta;
struct FloatIntConfig *FIC, *NewConfig;
LONG Style;
LPNMHDR nmhdr;
NM_UPDOWN *UPD;
AnimDoubleTime *AD = NULL;
//LPDRAWITEMSTRUCT lpdis;
RasterAnimHost **IRAH = NULL, *RAH = NULL, *RAHParent = NULL;
POINT MouseTrans;

Parent  = GetParent(hwnd);
MyID    = (WORD)GetWindowLong(hwnd, GWL_ID);
FIC		= (struct FloatIntConfig *)GetWindowLong(hwnd, 0);

Style   = GetWindowLong(hwnd, GWL_STYLE);

if (FIC && (FIC->FIFlags & FIOFlag_AnimDouble))
	{
	AD = (AnimDoubleTime *)FIC->MasterVariable;
	} // if
if (FIC && (FIC->FIFlags & FIOFlag_RAH))
	{
	RAH       = (RasterAnimHost *)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if
if (FIC && (FIC->FIFlags & FIOFlag_IRAH))
	{
	IRAH      = (RasterAnimHost **)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if


switch (message)
	{
	case WM_DESTROY:
		{
		AppMem_Free(FIC, sizeof(struct FloatIntConfig));
		SetWindowLong(hwnd, 0, 0);
		return(0);
		} // DESTROY
	case WM_PAINT:
		{
		if (Style & WS_DISABLED) DState = 1;
		else DState = 0;
		if ((!FIC || !FIC->MasterVariable)) DState = 1;
		if (DState)
			{
			if (FIC)
				{
				if (FIC->Edit) 
					{
					if ((!FIC || !FIC->MasterVariable)) SetWindowText(FIC->Edit, "");
					} // if
				EnableWindow(FIC->Label, false);
				InvalidateRect(FIC->Label, NULL, TRUE);

				if (FIC->Edit)
					{
					EnableWindow(FIC->Edit, false);
					InvalidateRect(FIC->Edit, NULL, TRUE);
					} // if

				if (FIC->Less)
					{
					EnableWindow(FIC->Less, false);
					InvalidateRect(FIC->Less, NULL, TRUE);
					} // if
				} // if FIC
			} // if
		else
			{
			EnableWindow(FIC->Label, true);
			InvalidateRect(FIC->Label, NULL, TRUE);

			if (FIC->Edit)
				{
				EnableWindow(FIC->Edit, true);
				InvalidateRect(FIC->Edit, NULL, TRUE);
				} // if

			if (FIC->Less)
				{
				EnableWindow(FIC->Less, true);
				InvalidateRect(FIC->Less, NULL, TRUE);
				} // if

			} // else
		break;
		} // PAINT
	case WM_SETTEXT:
		{
		SendMessage(FIC->Label, WM_SETTEXT, wParam, lParam);
		break;
		} // SETTEXT
	case WM_ERASEBKGND:
		{
		RECT ControlRECT;
		GetClientRect(hwnd, &ControlRECT);
		g_xpStyle.DrawThemeParentBackground(hwnd, (HDC)wParam, &ControlRECT);
		return(1);
		} // ERASEBG
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
		{
		return(CallWindowProc((WPROC)GetWindowLong(Parent, GWL_WNDPROC), Parent,
		 message, wParam, (LPARAM)lParam));
		} // WM_CTLCOLOR...
	case WM_SETFONT:
		{
		TextStyle = (HFONT)wParam;
		TextDC = GetDC(hwnd);
		if (TextStyle) SelectObject(TextDC, TextStyle);

		if (GetWindowLong(hwnd, GWL_STYLE) & WCSW_FI_STYLE_LABELRIGHT)
			{
			LabelRight = 1;
			} // if

		// Measure length of label text and field width in current font
		GetWindowText(hwnd, LabelName, 63);

		// Translate spaces into n's, because TextExtent ignores spaces.
		for (GenWidth = FieldChars = 0; LabelName[FieldChars]; FieldChars++)
			{
			if (LabelName[FieldChars] == ' ')
				{
				LabelName[GenWidth++] = 'n';
				} // if
			else if (LabelName[FieldChars] == '&')
				{
				// Don't increment GenWidth
				} // else if if
			else
				{
				LabelName[GenWidth++] = LabelName[FieldChars];
				} // else
			} // for

		GetTextExtentPoint32(TextDC, LabelName, (int)strlen(LabelName), &Ruler);
		LabelWidth  = (unsigned short)Ruler.cx;
		LabelHeight = (unsigned short)Ruler.cy;

		EditWidth = 0;
		if (FIC->Edit)
			{
			GetWindowText(FIC->Edit, LabelName, 31);
			GetTextExtentPoint32(TextDC, LabelName, (int)strlen(LabelName), &Ruler);
			EditWidth = (unsigned short)Ruler.cx;
			SetWindowText(FIC->Edit, ""); // Clear out the edit field
			} // if

		ReleaseDC(hwnd, TextDC);
		TextDC = NULL;

		// Tell the children what font to use
		SendMessage(FIC->Label, WM_SETFONT, (WPARAM)TextStyle, 0);
		if (FIC->Edit) SendMessage(FIC->Edit, WM_SETFONT, (WPARAM)TextStyle, 0);
		if (FIC->Less) SendMessage(FIC->Less, WM_SETFONT, (WPARAM)TextStyle, 0);

		// Find current container window size
		GetClientRect(hwnd, &WinSize);
		GenWidth = (unsigned short)WinSize.right;
		GenHeight = (unsigned short)WinSize.bottom;

		ExtraButtonWidth = ButtonWidth = 0;
		// Are the buttons enabled?
		if (FIC->Less)
			{
			// And recalculate the ButtonWidth, cause Windows is stupid
			ButtonWidth = (unsigned short)(GenHeight * .76);
			} // if
		if (FIC->Opt1)
			{
			// Extra Button Width is 2 * 16
			ExtraButtonWidth = 16;
			SendMessage(FIC->Opt1, WM_SETFONT, wParam, lParam);
			} // if
		if (FIC->Opt2)
			{
			ExtraButtonWidth = 32;
			SendMessage(FIC->Opt2, WM_SETFONT, wParam, lParam);
			} // if
		if (FIC->Opt3)
			{
			// Extra Button Width is 3 * 16
			ExtraButtonWidth = 48;
			SendMessage(FIC->Opt3, WM_SETFONT, wParam, lParam);
			} // if
		if (FIC->Color)
			{
			ColorWidth = GenHeight + 2; // color well is same width and height right now, pad width by 2 for margin
			SendMessage(FIC->Color, WM_SETFONT, wParam, lParam);
			} // if

		// Move and size the children
		if (LabelRight)
			{
			if (FIC->Edit)
				{
				MoveWindow(FIC->Edit, 0, 0, EditWidth, GenHeight, 1);
				GetClientRect(FIC->Edit, &WinSize);
				SetWindowPos(FIC->Edit, NULL, 0, (GenHeight - WinSize.bottom) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				} // IF
			// Optional Colorpot should already be in the right place -- the left edge
			// Finally, place the label
			MoveWindow(FIC->Label, ColorWidth + EditWidth + ButtonWidth + ExtraButtonWidth,
			 (GenHeight - LabelHeight) / 2, LabelWidth, GenHeight, 1);
			} // if
		else
			{
			// If LABELRIGHT is not set, the < > buttons are created
			// in their correct positions to begin with...
			MoveWindow(FIC->Label, GenWidth - (LabelWidth + ColorWidth + EditWidth + ButtonWidth + ExtraButtonWidth),
			 (GenHeight - LabelHeight) / 2, LabelWidth, LabelHeight, 1);
			if (FIC->Edit)
				{
				MoveWindow(FIC->Edit, GenWidth - (ColorWidth + EditWidth + ButtonWidth + ExtraButtonWidth),
				 0, EditWidth, GenHeight, 1);
				SetWindowPos(FIC->Edit, NULL, GenWidth - (ColorWidth + EditWidth + ButtonWidth + ExtraButtonWidth),
				 (GenHeight - WinSize.bottom) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				} // if
			if (FIC->Color)
				{
				SetWindowPos(FIC->Color, NULL, GenWidth - (ColorWidth + EditWidth + ButtonWidth + ExtraButtonWidth),
				 (GenHeight - WinSize.bottom) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				} // if
			} // else
		// If we have the updown buttons, we need to find out where the
		// Edit is, and place the updown next to it regardless of the
		// state of Labelright.
		if (FIC->Less && FIC->Edit)
			{
			GetChildCoords(FIC->Edit, &WinSize);
			MoveWindow(FIC->Less, WinSize.right, WinSize.top, ButtonWidth - 1, WinSize.bottom - WinSize.top, 1);
			} // if

		break;
		} // SETFONT
	case WM_CREATE:
		{
		if (FIC = (struct FloatIntConfig *)AppMem_Alloc(sizeof(struct FloatIntConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, 0, (long)FIC);
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else

		MyInst = (HINSTANCE)GetClassLong(hwnd, GCL_HMODULE);
		Create = (LPCREATESTRUCT)lParam;
		Ruler.cx = 0;

		// Figure out desired field width from low bits of Style
		FieldChars = 4; // default
		switch (Create->style & 0x000f)
			{
			case WCSW_FI_STYLE_WIDTH2: FieldChars = 2; break;
			case WCSW_FI_STYLE_WIDTH3: FieldChars = 3; break;
			case WCSW_FI_STYLE_WIDTH4: FieldChars = 4; break;
			case WCSW_FI_STYLE_WIDTH5: FieldChars = 5; break;
			case WCSW_FI_STYLE_WIDTH6: FieldChars = 6; break;
			case WCSW_FI_STYLE_WIDTH7: FieldChars = 7; break;
			case WCSW_FI_STYLE_WIDTH8: FieldChars = 8; break;
			case WCSW_FI_STYLE_WIDTH9: FieldChars = 9; break;
			case WCSW_FI_STYLE_WIDTH10: FieldChars = 10; break;
			case WCSW_FI_STYLE_WIDTH11: FieldChars = 11; break;
			case WCSW_FI_STYLE_WIDTH12: FieldChars = 12; break;
			case WCSW_FI_STYLE_WIDTH13: FieldChars = 13; break;
			case WCSW_FI_STYLE_WIDTH14: FieldChars = 14; break;
			case WCSW_FI_STYLE_WIDTH15: FieldChars = 15; break;
			} // switch

		// do we need an edit field, or are we a colorpot, or what?
		if (Create->style & WCSW_FI_STYLE_SRAH_NOFIELD)
			{
			FieldChars = 0;
			} // if
		if (Create->style & WCSW_FI_STYLE_SRAH_COLOR)
			{
			FieldChars = 0;
			ColorPot = 1;
			} // if

		ExtraButtonWidth = ButtonWidth = 0;
		// Check to see if buttons are inhibited
		if (!(WCSW_FI_STYLE_NOBUTTONS & Create->style))
			{
			// Calculate button width as fraction of height
			ButtonWidth = (unsigned short)((Create->cy) * .76);
			} // if
		if (!FieldChars)
			{
			// Don't need updown either
			ButtonWidth = 0;
			} // if
		// Check to see if EXTRA buttons are inhibited
		if (WCSW_FI_STYLE_EXTRABUTTONS & Create->style)
			{
			// Extra Button Width is 2 * 16
			ExtraButtonWidth = 32;
			} // if
		if (Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
			{
			// Extra Button Width is 3 * 16
			ExtraButtonWidth = 48;
			if (Create->style & WCSW_FI_STYLE_SRAH_TWOBUTTON)
				{
				ExtraButtonWidth = 32;
				} // if
			else if (Create->style & WCSW_FI_STYLE_SRAH_ONEBUTTON)
				{
				ExtraButtonWidth = 16;
				} // else if
			else if (Create->style & WCSW_FI_STYLE_SRAH_NOBUTTON)
				{
				ExtraButtonWidth = 0;
				} // else if
			} // if

		// Create Label
		// (Label and edit will have dummy widths until we calculate
		// the real sizes when we get font info)
		if (Create->style & WCSW_FI_STYLE_LABELRIGHT)
			{
			FIC->Label = CreateWindow("STATIC", Create->lpszName, WS_CHILD | SS_LEFT,
			 0, 0, 15, Create->cy, hwnd, NULL, MyInst, NULL);
			} // if
		else
			{
			FIC->Label = CreateWindow("STATIC", Create->lpszName, WS_CHILD | SS_RIGHT,
			 0, 0, 15, Create->cy, hwnd, NULL, MyInst, NULL);
			} // else

		// Create color pot
		if (ColorPot)
			{
			if (FIC->Color = CreateWindow(APP_CLASSPREFIX ".ColorBar", "Click to Edit", WS_CHILD,
			 0, 0, Create->cy, Create->cy, hwnd, NULL, MyInst, NULL))
				{
				SetWindowLong(FIC->Color, GWL_ID, 63145);
				} // if
			} // if

		// Create edit field
		// (We'll fill it with a dummy string of the right width for
		// later, when we know the font size...)
		if (FieldChars)
			{
			FIC->Edit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", FWT[FieldChars], WS_CHILD | ES_AUTOHSCROLL,
			 15, 0, 15, Create->cy, hwnd, NULL, MyInst, NULL);
			SetWindowLong(FIC->Edit, GWL_WNDPROC, (long)ModifiedEditWndProc);
			} // if

		FIC->Less = NULL;
		// Do we need buttons?
		if (ButtonWidth)
			{
			// Create UPDOWN button
			FIC->Less = CreateWindowEx(NULL, UPDOWN_CLASS, "", WS_CHILD | UDS_HOTTRACK,
			 Create->cx - (ButtonWidth + 2 + ExtraButtonWidth), 0, ButtonWidth, Create->cy,
			 hwnd, (HMENU)63141, MyInst, NULL);
			SendMessage(FIC->Less, UDM_SETRANGE32, (WPARAM)INT_MIN, (LPARAM)INT_MAX);
			} // if
		if (ExtraButtonWidth)
			{
			// Create extra buttons
			if (FIC->Opt1 = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", "Animation Operations",
			 WS_CHILD | WS_VISIBLE | WCSW_TB_STYLE_ALLOWRIGHT | WCSW_TB_STYLE_XPLOOK, Create->cx - (2 + ExtraButtonWidth), 0, 16, 18, hwnd, NULL, MyInst, NULL))
				{
				SetWindowLong(FIC->Opt1, GWL_ID, 63142);
				if (Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
					{
					ShowWindow(FIC->Opt1, SW_HIDE); // Hidden until needed
					} // if
				else if (Create->style & WCSW_FI_STYLE_DELKEY)
					{
					SetWindowText(FIC->Opt1, "Delete Key");
					assert(false); // I don't think we ever use this case, and I want to find out if we do anywhere
					} // if
				else
					{
					ConfigureTB(FIC->Opt1, NULL, IDI_KEY, NULL);
					} // else
				} // if
			if (ExtraButtonWidth >= 32)
				{
				if (FIC->Opt2 = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", "Texture Operations",
				 WS_CHILD | WS_VISIBLE | WCSW_TB_STYLE_ALLOWRIGHT | WCSW_TB_STYLE_XPLOOK, Create->cx - (2 + (ExtraButtonWidth - 16)), 0, 16, 18, hwnd, NULL, MyInst, NULL))
					{
					SetWindowLong(FIC->Opt2, GWL_ID, 63143);
					if (Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
						{
						ShowWindow(FIC->Opt2, SW_HIDE); // Hidden until needed
						} // if
					else
						{
						//ConfigureTB(FIC->Opt2, NULL, IDI_TIMELINESM, NULL);
						assert(false); // I don't think we ever use this case, and I want to find out if we do anywhere
						} // if
					} // if
				} // if
			if ((ExtraButtonWidth >= 48) && (Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD)))
				{
				if (FIC->Opt3 = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", "Opt3",
				 WS_CHILD | WS_VISIBLE | WCSW_TB_STYLE_ALLOWRIGHT | WCSW_TB_STYLE_XPLOOK, Create->cx - (2 + ExtraButtonWidth - 32), 0, 16, 18, hwnd, NULL, MyInst, NULL))
					{
					SetWindowLong(FIC->Opt3, GWL_ID, 63144);
					ShowWindow(FIC->Opt3, SW_HIDE); // Hidden until needed
					} // if
				//SRAHConfigButtons(hwnd, FIC, NULL, NULL);
				} // if
			} // if
		
		ExtrasOK = 1;
		if ((ButtonWidth != 0) && (FIC->Less == NULL)) ExtrasOK = 0;
		if (ExtraButtonWidth != 0)
			{
			if ((ExtraButtonWidth >= 16) && (FIC->Opt1 == NULL)) ExtrasOK = 0;
			if ((ExtraButtonWidth >= 32) && (FIC->Opt2 == NULL)) ExtrasOK = 0;
			if ((ExtraButtonWidth >= 48) && (Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD)) && (FIC->Opt3 == NULL)) ExtrasOK = 0;
			} // if
		if (FIC->Color) 
			{
			ShowWindow(FIC->Color, SW_SHOW);
			} // if
		if ((FIC->Edit || !FieldChars) && FIC->Label && ExtrasOK) 
			{
			ShowWindow(FIC->Label, SW_SHOW);
			if (FIC->Edit) ShowWindow(FIC->Edit, SW_SHOW);
			if (ButtonWidth)
				{
				ShowWindow(FIC->Less, SW_SHOW);
				} // if
			if (ExtraButtonWidth)
				{
				if (!(Create->style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD)))
					{
					// leave hidden for now if SRAH
					ShowWindow(FIC->Opt1, SW_SHOW);
					ShowWindow(FIC->Opt2, SW_SHOW);
					} // if
				} // if
			SetWindowLong(hwnd, 0, (long)FIC);
			return(0);
			} // if
		else
			{
			if (FIC->Opt1) DestroyWindow(FIC->Opt1);
			if (FIC->Opt2) DestroyWindow(FIC->Opt2);
			if (FIC->Opt3) DestroyWindow(FIC->Opt3);
			if (FIC->Color) DestroyWindow(FIC->Color);
			if (FIC->Label) DestroyWindow(FIC->Label);
			if (FIC->Edit) DestroyWindow(FIC->Edit);
			if (FIC->Less) DestroyWindow(FIC->Less);
			AppMem_Free(FIC, sizeof(struct FloatIntConfig));
			return(-1);
			} // else
		} // CREATE
	case WM_NOTIFY:
		{
		if (((int)wParam) == 63141)
			{
			if (nmhdr = (LPNMHDR)lParam)
				{
				if (nmhdr->code == UDN_DELTAPOS)
					{
					SetFocus(NULL);
					UPD = (NM_UPDOWN *)lParam;
					Delta = UPD->iDelta;
					SNDoIncDec(hwnd, FIC, Delta);
					if (FIC->Edit)
						{
						SetFocus(FIC->Edit);
						SendMessage(FIC->Edit, EM_SETSEL, 0, (LPARAM)0);
						} // if
					} // if
				} // if
			} // if
		break;
		} // NOTIFY:UDN_DELTAPOS
	case WM_COMMAND:
		{
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case BN_CLICKED:
				{
				if (LOWORD(wParam) == 63142)
					{
					if (RAH || IRAH)
						{
						GetCursorPos(&MouseTrans);
						//SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, WCS_RAH_POPMENU_CLASS_ANIM, RAH, RAHParent);
						SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, SRAHQueryButton(hwnd, FIC, RAH, IRAH, RAHParent, 0), RAH, IRAH, RAHParent);
						} // if
					else
						{
						SendMessage(Parent, WM_WCSW_FI_OPT1, (WPARAM)(MyID), (LPARAM)hwnd);
						if (AD)
							{
							AD->AddNode();
							} // if
						} // else
					} // if
				if (LOWORD(wParam) == 63143)
					{
					if (RAH || IRAH)
						{
						GetCursorPos(&MouseTrans);
						//SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, WCS_RAH_POPMENU_CLASS_TEX, RAH, RAHParent);
						SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, SRAHQueryButton(hwnd, FIC, RAH, IRAH, RAHParent, 1), RAH, IRAH, RAHParent);
						} // if
					else
						{
						SendMessage(Parent, WM_WCSW_FI_OPT2, (WPARAM)(MyID), (LPARAM)hwnd);
						if (AD)
							{
							AD->OpenTimeline();
							} // if
						} // else
					} // if
				if (LOWORD(wParam) == 63144)
					{
					if (RAH || IRAH)
						{
						GetCursorPos(&MouseTrans);
						//SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, WCS_RAH_POPMENU_CLASS_THEME, RAH, RAHParent);
						SRAHDoPopup(hwnd, FIC, MouseTrans.x, MouseTrans.y, SRAHQueryButton(hwnd, FIC, RAH, IRAH, RAHParent, 2), RAH, IRAH, RAHParent);
						} // if
					} // if
				if (LOWORD(wParam) == 63145)
					{
					if (RAH)
						{
						RAH->EditRAHost();
						} // if
					if (IRAH && *IRAH)
						{
						(*IRAH)->EditRAHost();
						} // if
					} // if
				return(0);
				} // CLICKED
			case EN_KILLFOCUS:
				{
				// Hmmm. Need to validate contents and convert
				SNmStr(hwnd, FIC);
				//SendMessage(Parent, WM_WCSW_FI_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
				return(0);
				} // KILLFOCUS
			} // switch
		break;
		} // WM_COMMAND
	case WM_SETFOCUS:
		{
		LRESULT LineLen;
		char OldShiftState;
		if (FIC->Edit)
			{
			SetFocus(FIC->Edit);
			LineLen = SendMessage(FIC->Edit, EM_LINELENGTH, 0, (LPARAM)0);

			// Feed large amounts of bogus data and events to the Edit control to
			// get it to do what we want it to do.
			SendMessage(FIC->Edit, EM_SCROLLCARET, LineLen, (LPARAM)LineLen);
			SendMessage(FIC->Edit, WM_KEYDOWN, VK_END, (LPARAM)0x14f0001);
			SendMessage(FIC->Edit, WM_KEYUP, VK_END, (LPARAM)0xc14f0001);
			GetKeyboardState(FoolKeyStates);
			OldShiftState = FoolKeyStates[VK_SHIFT];
			FoolKeyStates[VK_SHIFT] = 0x80;
			SetKeyboardState(FoolKeyStates);

			SendMessage(FIC->Edit, WM_KEYDOWN, VK_HOME, (LPARAM)0x1470001);
			SendMessage(FIC->Edit, WM_KEYUP, VK_HOME, (LPARAM)0xc1470001);
			FoolKeyStates[VK_SHIFT] = OldShiftState;
			SetKeyboardState(FoolKeyStates);
			} // if
		else if (FIC->Opt1)
			{
			SetFocus(FIC->Opt1);
			} // else if
		return(0);
		} // WM_SETFOCUS
	case WM_WCSW_FI_SYNC:
		{
		SNmSync(hwnd, FIC);
		if (!(wParam & WP_FISYNC_NONOTIFY))
			{
			SmartNumericPreviousValue = SNADmGetCurValue(hwnd, FIC);
			SendMessage(Parent, WM_WCSW_FI_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // SYNC
	case WM_WCSW_FI_SETUP:
		{
		NewConfig = (struct FloatIntConfig *)lParam;
		// Copy field-by-field so as not to obliterate HWNDs
		if (NewConfig)
			{
			FIC->MasterVariable = NewConfig->MasterVariable;
			FIC->AuxVariable = NewConfig->AuxVariable;
			FIC->IncDecAmount = NewConfig->IncDecAmount;
			FIC->MaxAmount = NewConfig->MaxAmount;
			FIC->MinAmount = NewConfig->MinAmount;
			FIC->FIFlags = NewConfig->FIFlags;
			FIC->LabelText = NewConfig->LabelText;
			FIC->CritID = NewConfig->CritID;
			FIC->SmallIDA = NewConfig->SmallIDA;
			} // if
		SNmSync(hwnd, FIC);
		return(0);
		} // SETUP
	case WM_WCSW_FI_OPT1: // means get edit field contents
		{
		if (FIC->Edit)
			{
			return(GetWindowText(FIC->Edit, (LPTSTR)lParam, wParam));
			} // if
		else
			{
			return(0);
			} // else
		break;	//lint !e527
		} // WM_WCSW_FI_OPT1
	case EM_SETREADONLY: // control READONLY attribute of edit field
		{
		if (FIC->Edit)
			{
			if (FIC->Less) EnableWindow(FIC->Less, !wParam);
			return(SendMessage(FIC->Edit, EM_SETREADONLY, wParam, 0));
			} // if
		else
			{
			return(0);
			} // else
		break;	//lint !e527
		} // EM_SETREADONLY
	case WM_ENABLE:
		{
		// resync subwidgets: "The WM_ENABLE message is sent when an application changes the enabled state of a window. It is sent to the window whose enabled state is changing."
		if (FIC)
			{
			if (FIC->Less) EnableWindow(FIC->Less, wParam);
			if (FIC->Edit) EnableWindow(FIC->Edit, wParam);
			if (FIC->Label) EnableWindow(FIC->Label, wParam);
			if (FIC->Color) EnableWindow(FIC->Color, wParam);
			if (FIC->Opt1) EnableWindow(FIC->Opt1, wParam);
			if (FIC->Opt2) EnableWindow(FIC->Opt2, wParam);
			if (FIC->Opt3) EnableWindow(FIC->Opt3, wParam);

			if (!wParam)
				{
				if (Style & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
					{
					ShowWindow(FIC->Opt1, SW_HIDE);
					ShowWindow(FIC->Opt2, SW_HIDE);
					ShowWindow(FIC->Opt3, SW_HIDE);
					} // if
				} // if
			} // if
		InvalidateParentRect(hwnd);
		break;
		} // WM_ENABLE
	} // switch

return(DefWindowProc(hwnd, message, wParam, lParam));

} // FloatIntWndProc

/*===========================================================================*/

// Pass along
long WINAPI UnModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam));
} // UnModifiedEditWndProc

/*===========================================================================*/

// Intercept a few messages for dialog-tabbing
long WINAPI SlightlyModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD ShiftState;
NativeControl Parent, Focus;
LONG Style, ID;

ID = GetWindowLong(hwnd, GWL_ID); // 1148 = FileReq name field
// 1148 is the ID of the Name field in the Windows file dialog, AFAIK. We have to leave it alone.
// Our 1148 is ID_PROJECT_SAVEVIEW_4, a menu item
if (ID == 1148)
	return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam)); // do your own thing

switch (message)
	{
	case WM_GETDLGCODE:
		{
		return(DLGC_WANTALLKEYS);
		} // GETDLGCODE
	case WM_KEYUP:
	case WM_CHAR:
		{
/*		if ((int)wParam == VK_HOME)
			{
			sprintf(WidDbgStr, "HOME: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
		if ((int)wParam == VK_END)
			{
			sprintf(WidDbgStr, "END: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
		if ((int)wParam == VK_SHIFT)
			{
			sprintf(WidDbgStr, "SHIFT: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
			*/
		if (((int)wParam == VK_TAB) || ((int)wParam == VK_RETURN))
			{
			Style   = GetWindowLong(hwnd, GWL_STYLE);
			if (Style & ES_WANTRETURN) // make sure multiline text widgets still work
				{
				if ((int)wParam == VK_RETURN)
					{
					return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam)); // do your own thing
					} // if
				else if ((int)wParam == VK_TAB)
					{
					return(0); // to be handled as focus change
					} // else if
				} // if
			return(0);
			} // if
		break;
		} // TABEATER

	case WM_KEYDOWN:
		{
		if (((int)wParam == VK_TAB) || ((int)wParam == VK_RETURN))
			{
			Style   = GetWindowLong(hwnd, GWL_STYLE);
			if (Style & ES_WANTRETURN) // make sure multiline text widgets still work
				{
				if ((int)wParam == VK_RETURN)
					{
					return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam)); // do your own thing
					} // if
				else if ((int)wParam == VK_TAB)
					{
					// handled via focus change code, below
					} // else if
				} // if
			// Next/Prev Gadget via Tab
			Parent = GetParent(hwnd);
			//Grand = GetParent(Parent);
			ShiftState = 0;
			if ((int)wParam == VK_TAB)
				{ // SHIFT-RETURN doesn't mean anything
				ShiftState = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
				} // if

			if (ShiftState)
				{
				Focus = GetNextDlgTabItem(Parent, hwnd, 1);
				} // if
			else
				{
				Focus = GetNextDlgTabItem(Parent, hwnd, 0);
				} // else
			SendMessage(Parent, WM_COMMAND, (EN_KILLFOCUS << 16) | ((UINT)GetWindowLong(hwnd, GWL_ID) & 0xffff), (LONG)hwnd);
			SetFocus(Focus);
			return(0);
			} // if
		break;
		} // KEYDOWN

	} // switch

return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam));

} // SlightlyModifiedEditWndProc

/*===========================================================================*/

// Intercept a few messages for dialog-tabbing
long WINAPI ModifiedEditWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD ShiftState;
NativeControl Parent, Grand, Focus;

switch (message)
	{
	case WM_KILLFOCUS:
		{
		SendMessage(GetParent(hwnd), WM_KILLFOCUS, wParam, lParam);
		break;
		} // KILLFOCUS
	case WM_GETDLGCODE:
		{
		return(DLGC_WANTALLKEYS);
		} // GETDLGCODE
	case WM_KEYUP:
	case WM_CHAR:
		{
/*		if ((int)wParam == VK_HOME)
			{
			sprintf(WidDbgStr, "HOME: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
		if ((int)wParam == VK_END)
			{
			sprintf(WidDbgStr, "END: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
		if ((int)wParam == VK_SHIFT)
			{
			sprintf(WidDbgStr, "SHIFT: %x\n", lParam);
			OutputDebugStr(WidDbgStr);
			return(1);
			} // if
			*/
		if (((int)wParam == VK_TAB) || ((int)wParam == VK_RETURN))
			{
			return(0);
			} // if
		break;
		} // TABEATER
	case WM_KEYDOWN:
		{
		if (((int)wParam == VK_TAB) || ((int)wParam == VK_RETURN))
			{
			// Next/Prev Gadget via Tab
			Parent = GetParent(hwnd);
			Grand = GetParent(Parent);
			ShiftState = 0;
			if ((int)wParam == VK_TAB)
				{ // SHIFT-RETURN doesn't mean anything
				ShiftState = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
				} // if

			if (ShiftState)
				{
				Focus = GetNextDlgTabItem(Grand, Parent, 1);
				} // if
			else
				{
				Focus = GetNextDlgTabItem(Grand, Parent, 0);
				} // else
			SetFocus(Focus);
			return(0);
			} // if
		break;
		} // KEYDOWN
	} // switch

return(CallWindowProc((WPROC)EditWndProc, hwnd, message, wParam, lParam));

} // ModifiedEditWndProc

/*===========================================================================*/

long WINAPI ModifiedScrollWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{

switch (message)
	{
	case WM_LBUTTONDOWN:
		{
		SetFocus(hwnd);
		} // 
	} // switch

return(CallWindowProc((WPROC)ScrollWndProc, hwnd, message, wParam, lParam));

} // ModifiedScrollWndProc

/*===========================================================================*/

long WINAPI ModifiedTabWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{

switch (message)
	{
	case WM_LBUTTONDOWN:
		{
		SetFocus(hwnd);
		} // 
	} // switch

return(CallWindowProc((WPROC)TabWndProc, hwnd, message, wParam, lParam));

} // ModifiedTabWndProc

/*===========================================================================*/

long WINAPI ModifiedTreeWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
LRESULT ReturnMe;
long CtrlID, TVPopResult;
int ContextEnabled = 0, X, Y;
int Category;
//RasterAnimHost *RAH;
TV_ITEM TVItem;
TVHITTESTINFO TVHTI;
HTREEITEM HitItem;
POINT MouseTrans;
char ContextNameBuf[100];
RasterAnimHost *PopHost;
RasterAnimHostProperties HostProp;

if (message == WM_LBUTTONDBLCLK)
	{
	SendMessage(GetParent(hwnd), WM_COMMAND, (LBN_DBLCLK << 16), (LPARAM)hwnd);
	return (0);
	} // if

CtrlID = GetWindowLong(hwnd, GWL_ID);
// Context is only enabled for lists in S@G
if ((CtrlID == IDC_PARLIST1) || (CtrlID == IDC_PARLIST2) || (CtrlID == IDC_FOLIAGETREE))
	{
	ContextEnabled = 1;
	} // 

switch (message)
	{
	case WM_LBUTTONDOWN:
		{
		break; // don't react to LBUTTONDOWN on non-Mac, pass along
		} // RBUTTONDOWN
	case WM_RBUTTONDOWN:
		{
		// only try to display popup menus for S@G lists
		if (ContextEnabled)
			{
			X = LOWORD(lParam);
			Y = HIWORD(lParam);
			// invoke context popup menu

	/*		if (GlobalApp->WinSys->ModalLevel > 0)
				return(0); // no menus if modal
	*/		
			// remove any already-displayed popup menu
			if (TVPopupMenu) DestroyMenu(TVPopupMenu); TVPopupMenu = NULL;

			// See if they hit anything
			TVHTI.pt.x = X;
			TVHTI.pt.y = Y;
			if (HitItem = (HTREEITEM)CallWindowProc((WPROC)TreeWndProc, hwnd, TVM_HITTEST, 0, (LPARAM)&TVHTI))
				{
				TVItem.hItem = HitItem;
				TVItem.mask = (TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT);
				TVItem.stateMask = TVIS_SELECTED; 
				TVItem.pszText = &ContextNameBuf[0];
				TVItem.cchTextMax = 99;
				// Call real TreeView wndproc to retrieve Item info and name
				CallWindowProc((WPROC)TreeWndProc, hwnd, TVM_GETITEM, 0, (LPARAM)&TVItem);
				// Start the process
				if (TVPopupMenu = CreatePopupMenu())
					{
					char Header[255];
					PopMenuAdder Paddy(TVPopupMenu); // create new popmenuadder with our root PopupMenu
					if (PopHost = (RasterAnimHost *)TVItem.lParam)
						{
						HostProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
						HostProp.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
						PopHost->GetRAHostProperties(&HostProp);

						//if (HostProp.Name[0]) AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, HostProp.Name);
						//if (HostProp.Type[0]) AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, HostProp.Type);
						sprintf(Header, "%s %s", HostProp.Name, HostProp.Type);
						if (HostProp.Name[0] || HostProp.Type[0]) AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, Header);
						AppendMenu(TVPopupMenu, MF_SEPARATOR | MF_ENABLED, 58000, NULL);
						if (CtrlID == IDC_FOLIAGETREE)
							{
							PopHost->AddBasePopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_ECO);
							PopHost->AddDerivedPopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_ECO);
							} // if
						else
							{
							PopHost->AddBasePopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_GLOBAL);
							PopHost->AddDerivedPopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_GLOBAL);
							} // if
						} // if
					else
						{
						// Must be category
						//AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, ContextNameBuf);
						//AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, "(Category)");
						sprintf(Header, "%s %s", ContextNameBuf, "(Category)");
						AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, Header);
						AppendMenu(TVPopupMenu, MF_SEPARATOR | MF_ENABLED, 58000, NULL);
						if (CtrlID != IDC_FOLIAGETREE)
							{
							Category = GlobalApp->GUIWins->SAG->IdentifyCategory((unsigned long)TVItem.hItem, -1);
							Paddy.AddPopMenuItem("Enable all in Category", "ENABLE", 0);
							Paddy.AddPopMenuItem("Disable all in Category", "DISABLE", 0);
							if (Category != WCS_SUBCLASS_LAYER)
								Paddy.AddPopMenuItem("Edit first in Category", "EDIT", 0);
							// Don't want Create item to just pop into current W6 builds without documentation
							// we're now enabling it as we move forward to 6.3 and beyond
							if (Category > 0 && RasterAnimHost::IsDigitizeLegal(NULL, Category))
								Paddy.AddPopMenuItem("Create Component of this type with path or vector", "CREATETYPE", 0);
							if ((Category >= WCS_EFFECTSSUBCLASS_LAKE && Category < WCS_MAXIMPLEMENTED_EFFECTS)
								|| Category == WCS_RAHOST_OBJTYPE_VECTOR || Category == WCS_RAHOST_OBJTYPE_CONTROLPT)
								Paddy.AddPopMenuItem("Add Component of this type", "ADD", 0);
							if (Category >= WCS_EFFECTSSUBCLASS_LAKE && Category < WCS_MAXIMPLEMENTED_EFFECTS)
								Paddy.AddPopMenuItem("Add Component from Gallery", "ADDGALLERY", 0);
							//Paddy.AddPopMenuItem("Add Component from File", "LOAD", 0); // no easy way to implement for category
							if (Category == WCS_SUBCLASS_LAYER)
								{
								Paddy.AddPopMenuItem("Add new Layer", "ADD", 0);
								Paddy.AddPopMenuItem("Purge unused layers from Project", "PURGELAYERS", 0);
								} // if
							else
								Paddy.AddPopMenuItem("Delete all in Category", "DELETE", 0);
							} // if
						else
							{
							if (! stricmp(ContextNameBuf, "Overstory"))
								{
								Paddy.AddPopMenuItem("Add Overstory Ecotype", "ADDOVERSTORY", 0);
								Paddy.AddPopMenuItem("Add Overstory from Gallery", "ADDOVERGALLERY", 0);
								Paddy.AddPopMenuItem("Add Foliage Group", "ADDOVERFOLIAGEGROUP", 0);
								Paddy.AddPopMenuItem("Add Foliage Group from Gallery", "ADDOVERFOLIAGEGROUPGALLERY", 0);
								Paddy.AddPopMenuItem("Add Foliage", "ADDOVERFOLIAGE", 0);
								} // if
							else
								{
								Paddy.AddPopMenuItem("Add Understory Ecotype", "ADDUNDERSTORY", 0);
								Paddy.AddPopMenuItem("Add Understory from Gallery", "ADDUNDERGALLERY", 0);
								Paddy.AddPopMenuItem("Add Foliage Group", "ADDUNDERFOLIAGEGROUP", 0);
								Paddy.AddPopMenuItem("Add Foliage Group from Gallery", "ADDUNDERFOLIAGEGROUPGALLERY", 0);
								Paddy.AddPopMenuItem("Add Foliage", "ADDUNDERFOLIAGE", 0);
								} // else
							} // else
						} // else

					// window to screen?
					MouseTrans.x = X;
					MouseTrans.y = Y;

					ClientToScreen(hwnd, &MouseTrans);
					if (TVPopResult = TrackPopupMenu(TVPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, MouseTrans.x, MouseTrans.y, 0, hwnd, NULL))
						{
						int ActionID, Derived = 0;
						if (TVPopResult < 59000)
							{
							ActionID = TVPopResult - 58001;
							} // if
						else
							{
							Derived = 1;
							ActionID = TVPopResult - 59001;
							} // else
						if (ActionID > -1) // Dummy entries (58000) result in -1
							{
							PopMenuEvent PME;
							PME.ActionText = (void *)Paddy.GetAction(ActionID); // void cast needed to shoehorn into older code without extensive code changes elsewhere
							PME.RAH = PopHost;
							PME.TreeItem = (unsigned long)HitItem;
							PME.Derived = Derived;
							SendMessage(GetParent(hwnd), WM_WCSW_LIST_CONTEXTMENU, (WPARAM)(GetWindowLong(hwnd, GWL_ID)), (LPARAM)&PME);
							} // if
						} // if
					} // if
				} // if

			return(0); // we'va handled it, don't pass on to real TreeView
			} // if
		} // RBUTTONDOWN
	} // switch

ReturnMe = CallWindowProc((WPROC)TreeWndProc, hwnd, message, wParam, lParam);

switch (message)
	{
	case WM_KEYUP:
		{
		void *ItemData = NULL;
		TVITEMEX ItemInfo;
		HTREEITEM SelectedItem = TreeView_GetSelection(hwnd);
		if (SelectedItem)
			{
			ItemInfo.hItem = SelectedItem;
			TreeView_GetItem(hwnd, &ItemInfo);
			ItemData = (void *)ItemInfo.lParam;
			} // if
		if ((int)wParam == 'V')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_PASTE, (WPARAM)(ItemData), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		if ((int)wParam == 'C')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_COPY, (WPARAM)(ItemData), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		if ((int)wParam == VK_DELETE)
			{
			SendMessage(GetParent(hwnd), WM_WCSW_LIST_DELITEM, (WPARAM)(ItemData), (LPARAM)hwnd);
			} // if
		if (((int)wParam == VK_RETURN))
			{
			// Simulate doubleclick for RETURN
			SendMessage(GetParent(hwnd), WM_LBUTTONDBLCLK, wParam, (LPARAM)hwnd);
			} // if
		break;
		} // KEYUP
	case WM_HSCROLL:
	case WM_VSCROLL:
		{
		SendMessage(GetParent(hwnd), message, wParam, (LPARAM)hwnd);
		} // 
	} // switch

return(ReturnMe);

} // ModifiedTreeWndProc

/*===========================================================================*/

void ColumnedListViewSetActiveColumn(NativeControl NC, signed long ActiveCol)
{

SetWindowLong(NC, ListViewOriginalWndExtra + WCSW_CLV_OFF_ACTIVECOL, ActiveCol);

} // ColumnedListViewSetActiveColumn

/*===========================================================================*/

signed long ColumnedListViewGetActiveColumn(NativeControl NC)
{
signed long RVal;

RVal = GetWindowLong(NC, ListViewOriginalWndExtra + WCSW_CLV_OFF_ACTIVECOL);
return(RVal);

} // ColumnedListViewGetActiveColumn

/*===========================================================================*/

long WINAPI ColumnedListViewWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{

switch (message)
	{
	case WM_CREATE:
		{
		long ReturnValue = (CallWindowProc((WPROC)ListViewWndProc, hwnd, message, wParam, lParam));
		// subclass the embedded Header widget
		HWND HeaderWnd = ListView_GetHeader(hwnd);
		
		SetWindowLong(hwnd, ListViewOriginalWndExtra + WCSW_CLV_OFF_ACTIVECOL, -1); // none selected
		LONG NewStyle = GetWindowLong(HeaderWnd, GWL_STYLE);
		NewStyle |= HDS_BUTTONS;
		SetWindowLong(HeaderWnd, GWL_STYLE, NewStyle);

		g_xpStyle.SetWindowTheme(HeaderWnd, (L" "), (L" ")); // disable theming so color control works
		return(ReturnValue);
		} // 
	case WM_NOTIFY: // from child Header control
		{
		LPNMCUSTOMDRAW pnm = (LPNMCUSTOMDRAW)lParam;
		if (pnm->hdr.code == NM_CUSTOMDRAW)
			{
			switch (pnm->dwDrawStage)
				{
				case CDDS_PREPAINT : //Before the paint cycle begins
					//request notifications for individual listview items
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT: //Before an item is drawn
					{
					LONG ActiveCol;
					ActiveCol = GetWindowLong(hwnd, ListViewOriginalWndExtra + WCSW_CLV_OFF_ACTIVECOL);
					if (pnm->dwItemSpec == ActiveCol)
						{
						SetTextColor(pnm->hdc, WINGDI_RGB(255, 255, 255));
						SetBkColor(pnm->hdc, WINGDI_RGB(0, 0, 0));
						return CDRF_NEWFONT;
						} // if
					else
						return CDRF_DODEFAULT;


/*					if (g_nCurrentItemNdx == (int)lplvcd->nmcd.dwItemSpec)
						{
						if (g_fSubItemCD && LV_VIEW_DETAILS == ListView_GetView(g_hwndLV))
							{
							//request notification for subitems
							return CDRF_NOTIFYSUBITEMDRAW;
							}

						//customize item appearance
						lplvcd->clrText   = RGB(255,255,255);
						lplvcd->clrTextBk = RGB(0,0,255);

						//To set a custom font:
						//SelectObject(lplvcd->nmcd.hdc, <your custom HFONT>);

						return CDRF_NEWFONT;
						}
*/
					//break;
					} // ITEMPREPAINT

				} // switch
			return CDRF_DODEFAULT;
			} // if
		else if (pnm->hdr.code == HDN_ITEMCLICK || pnm->hdr.code == HDN_ITEMCLICKW) // not sure why we get HDN_ITEMCLICKW, but we do
			{
			LPNMHEADER lpnmhdr = (LPNMHEADER)lParam;
			if (lpnmhdr)
				{
				// record new active column
				SetWindowLong(hwnd, ListViewOriginalWndExtra + WCSW_CLV_OFF_ACTIVECOL, lpnmhdr->iItem);
				// notify parent
				SendMessage(GetParent(hwnd), WM_WCSW_COLLISTVIEW_SELCOLUMN, (WPARAM)(GetWindowLong(hwnd, GWL_ID)), (LPARAM)hwnd);
				// Redraw
				InvalidateRect(hwnd, NULL, NULL);
				} // if
			} // else if HDN_ITEMCLICK
		} // WM_NOTIFY
	} // switch

return(CallWindowProc((WPROC)ListViewWndProc, hwnd, message, wParam, lParam));

} // ColumnedListViewWndProc

/*===========================================================================*/

// PopMenuAdder code. PopMenuAdder is defined in RasterAnimHost.h

int PopMenuAdder::AddPopMenuItem(const char *DisplayText, const char *Action, int Derived, int Enabled, int Checked)
{
int Result = 0;
int IDBase = 58001;

if (Derived) IDBase = 59001;

if (Result = AppendMenu((HMENU)MenuStack[MenuStackCurrent], MF_STRING | (Enabled ? MF_ENABLED : MF_GRAYED) | (Checked ? MF_CHECKED : MF_UNCHECKED), IDBase + CurItem, DisplayText))
	{
	//Actions[CurItem] = (void *)Action; // old pointer-based Action container, obsolete
	Actions.push_back(Action); // this will copy the string
	CurItem++;
	} // if

return(Result);

} // PopMenuAdder::AddPopMenuItem

/*===========================================================================*/

int PopMenuAdder::BeginPopSubMenu(const char *DisplayText, int Enabled, int Checked )
{
HMENU NewPop = NULL;

if ((MenuStackCurrent + 1) < WCS_RAH_POPMENU_MAX_STACK)
	{
	if (NewPop = CreatePopupMenu())
		{
		if (AppendMenu((HMENU)MenuStack[MenuStackCurrent], MF_POPUP | MF_STRING | (Enabled ? MF_ENABLED : 0) | (Checked ? MF_CHECKED : MF_UNCHECKED), (UINT)NewPop, DisplayText))
			{
			MenuStack[++MenuStackCurrent] = NewPop;
			return(1);
			} // if
		DestroyMenu(NewPop);
		} // if
	} // if

return(0);

} // PopMenuAdder::BeginPopSubMenu

/*===========================================================================*/

int PopMenuAdder::EndPopSubMenu(void)
{

if (MenuStackCurrent)
	{
	MenuStack[MenuStackCurrent--] = NULL;
	return(1);
	} // if

return(0);

} // PopMenuAdder::EndPopSubMenu

/*===========================================================================*/

int PopMenuAdder::AddPopMenuDivider(void)
{ // easy

return(AppendMenu((HMENU)MenuStack[MenuStackCurrent], MF_SEPARATOR | MF_ENABLED, 59000, NULL));

} // PopMenuAdder::AddPopMenuDivider

/*===========================================================================*/

long WINAPI ModifiedListWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
LRESULT ReturnMe;
unsigned long Style;
Style = GetWindowLong(hwnd, GWL_STYLE);

switch (message)
	{
	case WM_KEYDOWN:
		{
		if ((int)wParam == 'A' && GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL) && (Style & LBS_EXTENDEDSEL))
			{ // eat message, processed in KEYUP below
			return(0);
			} // if
		break;
		} // KEYDOWN
	case WM_KEYUP:
		{
		void *ItemData = NULL;
		long CurSel = 0;
		CurSel = (long)CallWindowProc((WPROC)ListWndProc, hwnd, LB_GETCURSEL, 0, 0);
		ItemData = (void *)CallWindowProc((WPROC)ListWndProc, hwnd, LB_GETITEMDATA, CurSel, 0);
		if ((int)wParam == 'A' && GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL) && (Style & LBS_EXTENDEDSEL))
			{
			SendMessage(hwnd, LB_SETSEL, (WPARAM)true, (LPARAM)-1); // select all
			// forge LBN_SELCHANGE to make everyone take notice
			PostMessage(GetParent(hwnd), WM_COMMAND, ((UINT)GetWindowLong(hwnd, GWL_ID) & 0xffff) | (LBN_SELCHANGE << 16), (LONG)hwnd);
			return(0);
			} // if
		if ((int)wParam == VK_DELETE)
			{
			SendMessage(GetParent(hwnd), WM_WCSW_LIST_DELITEM, (WPARAM)(ItemData), (LPARAM)hwnd);
			return(0);
			} // if
		if ((int)wParam == 'V')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_PASTE, (WPARAM)(ItemData), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		if ((int)wParam == 'C')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_COPY, (WPARAM)(ItemData), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		} // KEYUP
	} // switch

ReturnMe = CallWindowProc((WPROC)ListWndProc, hwnd, message, wParam, lParam);

return(ReturnMe);

} // ModifiedListWndProc

# endif // _WIN32

/*===========================================================================*/

AnimCritter *SRAHGetAssociatedAD(RasterAnimHost *Parent, AnimDouble *AD)
{
unsigned char MyMetType, SeekMetType = 0xff;
RasterAnimHost *ScanRAH;

MyMetType = AD->GetMetricType();
if (MyMetType == WCS_ANIMDOUBLE_METRIC_LONGITUDE) SeekMetType = WCS_ANIMDOUBLE_METRIC_LATITUDE;
if (MyMetType == WCS_ANIMDOUBLE_METRIC_LATITUDE)  SeekMetType = WCS_ANIMDOUBLE_METRIC_LONGITUDE;

for (ScanRAH = Parent->GetNextGroupSibling(AD); ScanRAH && ScanRAH != AD; ScanRAH = Parent->GetNextGroupSibling(ScanRAH))
	{
	if (((AnimCritter *)ScanRAH)->GetMetricType() == SeekMetType)
		{
		return((AnimCritter *)ScanRAH);
		} // if
	} // for

return(NULL);

} // SRAHGetAssociatedAD

/*===========================================================================*/

int SRAHDoPopup(NativeControl Me, struct FloatIntConfig *FIC, int X, int Y, unsigned long MenuClassFlags, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent)
{
RasterAnimHost *PopHost;
RasterAnimHostProperties HostProp;
long SRAHPopResult;

if (SRAHPopupMenu = CreatePopupMenu())
	{
	PopMenuAdder Paddy(SRAHPopupMenu); // create new popmenuadder with our root PopupMenu
	//if (RAH) PopHost = RAH;
	if (RAHParent) PopHost = RAHParent;
	if (PopHost)
		{
		HostProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		HostProp.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		PopHost->GetRAHostProperties(&HostProp);

		// temp code for identification
		/*
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_ANIM) Paddy.AddPopMenuItem("CLASS ANIM", "DUMMY", 0);
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_TEX) Paddy.AddPopMenuItem("CLASS TEX", "DUMMY", 0);
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_ECO) Paddy.AddPopMenuItem("CLASS ECO", "DUMMY", 0);
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_THEME) Paddy.AddPopMenuItem("CLASS THEME", "DUMMY", 0);
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_STRATA) Paddy.AddPopMenuItem("CLASS STRATA", "DUMMY", 0);
		if (MenuClassFlags & WCS_RAH_POPMENU_CLASS_FOAM) Paddy.AddPopMenuItem("CLASS FOAM", "DUMMY", 0);
		*/


		if (HostProp.Name[0]) AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, HostProp.Name);
		if (HostProp.Type[0]) AppendMenu(TVPopupMenu, MF_STRING | MF_DISABLED, 58000, HostProp.Type);
		AppendMenu(TVPopupMenu, MF_SEPARATOR | MF_ENABLED, 58000, NULL);
		PopHost->AddSRAHBasePopMenus(&Paddy, MenuClassFlags, RAH, IRAH);
		} // if

	if (SRAHPopResult = TrackPopupMenu(SRAHPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, X, Y, 0, Me, NULL))
		{
		int ActionID, Derived = 0;
		if (SRAHPopResult < 59000)
			{
			ActionID = SRAHPopResult - 58001;
			} // if
		else
			{
			Derived = 1;
			ActionID = SRAHPopResult - 59001;
			} // else
		if (ActionID > -1) // Dummy entries (58000) result in -1
			{
			if (Derived) // most will be
				{
				PopHost->HandleSRAHPopMenuSelection((void *)Paddy.GetAction(ActionID), RAH, IRAH); // void cast needed to shoehorn into older code without extensive code changes elsewhere
				} // if
			else
				{ // on the off chance that we'll have some event that
				// the widget itself is expected to deal with
				// currently just asks the object to deal with it
				PopHost->HandleSRAHPopMenuSelection((void *)Paddy.GetAction(ActionID), RAH, IRAH); // void cast needed to shoehorn into older code without extensive code changes elsewhere
				} // else
			} // if
		} // if
	} // if

return(0);

} // SRAHDoPopup

/*===========================================================================*/

unsigned long SRAHQueryButton(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent, int ButtonNum)
{
RasterAnimHostProperties HostProp;
int OptAssign, OptCheck;
unsigned long OptType;
NativeControl CurOpt;

if (RAHParent)
	{
	HostProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_POPCLASS;
	HostProp.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
	HostProp.ChildA = RAH;
	HostProp.ChildB = IRAH;
	RAHParent->GetRAHostProperties(&HostProp);

	for (OptCheck = OptAssign = 0; (OptAssign < 3) && (OptCheck < 6);)
		{
		CurOpt = NULL;
		if (OptAssign == 0) CurOpt = FIC->Opt1;
		if (OptAssign == 1) CurOpt = FIC->Opt2;
		if (OptAssign == 2) CurOpt = FIC->Opt3;
		for (; OptCheck < 6; OptCheck ++)
			{
			if (HostProp.PopClassFlags & (1 << OptCheck))
				{
				OptType = (1 << OptCheck);
				break;
				} // if
			} // for

		if (OptCheck < 6)
			{
			if (OptType == WCS_RAH_POPMENU_CLASS_ANIM)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_TEX)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_THEME)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_STRATA)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_FOAM)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_ECO)
				{
				if (OptAssign == ButtonNum) return(OptType);
				OptAssign++;
				} // if
			OptCheck ++;
			} // if
		} // for
	} // if

return(0);

} // SRAHQueryButton

/*===========================================================================*/

void SRAHConfigButtons(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent)
{
RasterAnimHostProperties HostProp;
int OptAssign, OptCheck;
unsigned long OptType;
NativeControl CurOpt;

if (RAHParent)
	{
	HostProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_POPCLASS;
	HostProp.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
	HostProp.ChildA = RAH;
	HostProp.ChildB = IRAH;
	RAHParent->GetRAHostProperties(&HostProp);

	for (OptCheck = OptAssign = 0; (OptAssign < 3) && (OptCheck < 6);)
		{
		int Shown = 0;
		CurOpt = NULL;
		if (OptAssign == 0) CurOpt = FIC->Opt1;
		if (OptAssign == 1) CurOpt = FIC->Opt2;
		if (OptAssign == 2) CurOpt = FIC->Opt3;
		if (! CurOpt) break;
		for (; OptCheck < 6; OptCheck ++)
			{
			if (HostProp.PopClassFlags & (1 << OptCheck))
				{
				OptType = (1 << OptCheck);
				break;
				} // if
			} // for

		if (CurOpt && (OptCheck < 6))
			{
			if (OptType == WCS_RAH_POPMENU_CLASS_ANIM)
				{
				SetWindowText(CurOpt, "Animation Operations");
/*				if (HostProp.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
					ConfigureTB(CurOpt, NULL, IDI_KEYEXIST, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_KEY, NULL);
*/
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_ANIM)
					ConfigureTB(CurOpt, NULL, IDI_KEYEXIST, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_ANIM)
					ConfigureTB(CurOpt, NULL, IDI_KEYEXIST, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_KEY, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_TEX)
				{
				SetWindowText(CurOpt, "Texture Operations");
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_TEX)
					ConfigureTB(CurOpt, NULL, IDI_TEXTURE_SMALL, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_TEX)
					ConfigureTB(CurOpt, NULL, IDI_TEXTUREDISABLED_SMALL, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_NOTEXTURE_SMALL, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_THEME)
				{
				SetWindowText(CurOpt, "Thematic Operations");
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_THEME)
					ConfigureTB(CurOpt, NULL, IDI_THEME_SMALL, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_THEME)
					ConfigureTB(CurOpt, NULL, IDI_THEMEDISABLED_SMALL, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_NOTHEME_SMALL, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_STRATA)
				{
				SetWindowText(CurOpt, "Strata Operations");
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_STRATA)
					ConfigureTB(CurOpt, NULL, IDI_STRATA_SMALL, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_STRATA)
					ConfigureTB(CurOpt, NULL, IDI_STRATADISABLED_SMALL, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_NOSTRATA_SMALL, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_FOAM)
				{
				SetWindowText(CurOpt, "Foam Operations");
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_FOAM)
					ConfigureTB(CurOpt, NULL, IDI_FOAM_SMALL, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_FOAM)
					ConfigureTB(CurOpt, NULL, IDI_FOAMDISABLED_SMALL, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_NOFOAM_SMALL, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			if (OptType == WCS_RAH_POPMENU_CLASS_ECO)
				{
				SetWindowText(CurOpt, "Ecotype Operations");
				if (HostProp.PopEnabledFlags & WCS_RAH_POPMENU_CLASS_ECO)
					ConfigureTB(CurOpt, NULL, IDI_FOLIAGE_SMALL, NULL);
				else if (HostProp.PopExistsFlags & WCS_RAH_POPMENU_CLASS_ECO)
					ConfigureTB(CurOpt, NULL, IDI_FOLIAGEDISABLED_SMALL, NULL);
				else
					ConfigureTB(CurOpt, NULL, IDI_NOFOLIAGE_SMALL, NULL);
				OptAssign++;
				Shown = 1;
				ShowWindow(CurOpt, SW_SHOW);
				} // if
			OptCheck ++;
			} // if
		if (CurOpt && !Shown)
			{
			ShowWindow(CurOpt, SW_HIDE);
			} // if
		} // for
	// Hide any unassigned buttons
	while(OptAssign < 3)
		{
		CurOpt = NULL;
		if (OptAssign == 0) CurOpt = FIC->Opt1;
		if (OptAssign == 1) CurOpt = FIC->Opt2;
		if (OptAssign == 2) CurOpt = FIC->Opt3;
		if (CurOpt)
			{
			ShowWindow(CurOpt, SW_HIDE);
			} // if
		OptAssign++;
		} // while

	SRAHConfigLabel(Me, FIC, RAH, IRAH, RAHParent);
	} // if

} // SRAHConfigButtons

/*===========================================================================*/

void SRAHConfigLabel(NativeControl Me, struct FloatIntConfig *FIC, RasterAnimHost *RAH, RasterAnimHost **IRAH, RasterAnimHost *RAHParent)
{
RasterAnimHostProperties HostProp;
unsigned long StyleRefresh;
RasterAnimHost *TestRAH = NULL;
AnimDoubleTime *ADT;
unsigned char MetType;
const char *NewLabelStr = NULL;
CoordSys *DefCS;
PlanetOpt *DefPO;
GeneralEffect *CurEffect;

StyleRefresh = GetWindowLong(Me, GWL_STYLE);

if ((GlobalApp->MainProj->Prefs.DisplayGeoUnitsProjected) && (StyleRefresh & WCSW_FI_STYLE_SRAH_AUTOLABEL))
	{
	HostProp.ChildA = NULL;
	HostProp.ChildB = NULL;
	HostProp.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	if (RAH)
		{
		TestRAH = RAH;
		} // if
	else if (IRAH && *IRAH)
		{
		TestRAH = *IRAH;
		} // if
	if (TestRAH)
		{
		TestRAH->GetRAHostProperties(&HostProp);
		if (HostProp.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
			{
			ADT = (AnimDoubleTime *)TestRAH; // Yes, it definitiely is. See above.
			MetType = ADT->GetMetricType();

			DefPO = NULL;
			if (CurEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
				{
				while (CurEffect)
					{
					if (CurEffect->Enabled)
						{
						DefPO = (PlanetOpt *)CurEffect;
						}// if
					CurEffect = CurEffect->Next;
					} // while
				} // if

			if (DefPO && (DefCS = DefPO->Coords))
				{
				switch (MetType)
					{
					case WCS_ANIMDOUBLE_METRIC_LONGITUDE:
						{
						NewLabelStr = DefCS->GetXUnitName();
						//NewLabelStr = "Lon/X ";
						break;
						} // Lon
					case WCS_ANIMDOUBLE_METRIC_LATITUDE:
						{
						NewLabelStr = DefCS->GetYUnitName();
						//NewLabelStr = "Lat/Y ";
						break;
						} // Lat
					} // switch
				} // if
			if (NewLabelStr && FIC->Label)
				{
				SetWindowText(FIC->Label, NewLabelStr);
				} // if
			} // if
		} // if
	} // if

} // SRAHConfigLabel

/*===========================================================================*/

void SNmSync(NativeControl Me, struct FloatIntConfig *FIC)
{
double DConv;
float FConv;
unsigned long ULConv, StyleRefresh;
signed long LConv;
void *MasterVariable = NULL;
SetCritter *Crit = NULL;
AnimDouble *AD = NULL;
RasterAnimHost **IRAH = NULL, *RAH = NULL, *RAHParent = NULL;
RasterAnimHostProperties HostProp;

if (FIC && (FIC->FIFlags & FIOFlag_RAH))
	{
	RAH       = (RasterAnimHost *)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if
if (FIC && (FIC->FIFlags & FIOFlag_IRAH))
	{
	IRAH       = (RasterAnimHost **)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if

if (FIC->Color)
	{
	if (RAH)
		{
		HostProp.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		RAH->GetRAHostProperties(&HostProp);
		if (HostProp.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
			{
			ConfigureCB(FIC->Color, 0, 100, 0, 0, (AnimColorTime *)RAH);
			} // if
		} // if
	else if (IRAH && *IRAH)
		{
		HostProp.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		(*IRAH)->GetRAHostProperties(&HostProp);
		if (HostProp.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
			{
			ConfigureCB(FIC->Color, 0, 100, 0, 0, (AnimColorTime *)(*IRAH));
			} // if
		} // else if
	} // if

if (FIC->FIFlags & FIOFlag_Critter)
	{
	if (Crit = (SetCritter *)FIC->MasterVariable)
		{
		MasterVariable = (void *)&FloatIntDummyBuf;
		Crit->GetParam(MasterVariable, FIC->CritID);
		} // if
	} // if
else if (FIC->FIFlags & FIOFlag_AnimDouble)
	{
	AD = (AnimDouble *)FIC->MasterVariable;
	SNADmSync(Me, FIC, AD);
	return;
	} // else if
else
	{
	MasterVariable = FIC->MasterVariable;
	} // else


Scratch[0] = NULL;

# ifdef _WIN32
StyleRefresh = GetWindowLong(Me, GWL_STYLE);
if (FIC->Opt1)
	{
	if (StyleRefresh & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
		{
		// nothing to do now
		} // if
	else if (StyleRefresh & WCSW_FI_STYLE_DELKEY)
		{
		assert(false); // I don't think we ever use this case, and I want to find out if we do anywhere
		} // if
	else
		{
		ConfigureTB(FIC->Opt1, NULL, IDI_KEY, NULL);
		} // else
	} // if

if (StyleRefresh & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
	{
	SRAHConfigButtons(Me, FIC, RAH, IRAH, RAHParent);
	} // if


# endif // _WIN32

# ifdef _WIN32
// Cause a Full-Widget repaint to sort out disabled states
InvalidateRect(Me, NULL, TRUE);
# endif // _WIN32        

if (FIC->Edit)
	{
	if (FIC->FIFlags & FIOFlag_Float)
		{
		FConv = *(float *)MasterVariable;
		DConv = (double)FConv;
		//sprintf(Scratch, WCSW_FP_CONVERT_STRING, DConv);
		sprintf(Scratch, "%.*f", SNCalculateFractionalDigits(DConv), DConv);
		TrimDecimalZeros(Scratch);
		} /* if */
	else if (FIC->FIFlags & FIOFlag_Double)
		{
		DConv = *(double *)MasterVariable;
		//sprintf(Scratch, WCSW_FP_CONVERT_STRING, DConv);
		sprintf(Scratch,  "%.*f", SNCalculateFractionalDigits(DConv), DConv);
		TrimDecimalZeros(Scratch);
		} /* if */
	else if (FIC->FIFlags & FIOFlag_Char)
		{
		if (FIC->FIFlags & FIOFlag_Unsigned)
			{
			ULConv = *(unsigned char *)MasterVariable;
			sprintf(Scratch, "%u", ULConv);
			} /* if */
		else
			{
			LConv = *(char *)MasterVariable;
			sprintf(Scratch, "%d", LConv);
			} /* else */
		} /* if */
	else if (FIC->FIFlags & FIOFlag_Short)
		{
		if (FIC->FIFlags & FIOFlag_Unsigned)
			{
			ULConv = *(unsigned short *)MasterVariable;
			sprintf(Scratch, "%u", ULConv);
			} /* if */
		else
			{
			LConv = *(signed short *)MasterVariable;
			sprintf(Scratch, "%d", LConv);
			} /* else */
		} /* if */
	else if (FIC->FIFlags & FIOFlag_Long)
		{
		if (FIC->FIFlags & FIOFlag_Unsigned)
			{
			ULConv = *(unsigned long *)MasterVariable;
			sprintf(Scratch, "%u", ULConv);
			} /* if */
		else
			{
			LConv = *(long *)MasterVariable;
			sprintf(Scratch, "%d", LConv);
			} /* else */
		} /* if */

	# ifdef _WIN32
	SetWindowText(FIC->Edit, Scratch);
	SendMessage(FIC->Edit, EM_SETSEL, 0, (LPARAM)0);
	# endif // _WIN32
	} // if

} // SNmSync

/*===========================================================================*/

void SNADmSync(NativeControl Me, struct FloatIntConfig *FIC, AnimDouble *AD)
{
unsigned long StyleRefresh;
RasterAnimHost **IRAH = NULL, *RAH = NULL, *RAHParent = NULL;

if (!AD) return;

if (FIC && (FIC->FIFlags & FIOFlag_RAH))
	{
	RAH       = (RasterAnimHost *)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if
if (FIC && (FIC->FIFlags & FIOFlag_IRAH))
	{
	IRAH       = (RasterAnimHost **)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if

Scratch[0] = NULL;

# ifdef _WIN32
StyleRefresh = GetWindowLong(Me, GWL_STYLE);
if (StyleRefresh & WCSW_FI_STYLE_EXTRABUTTONS)
	{
	if (StyleRefresh & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
		{
		// nothing to do anymore
		} // if
	else
		{
		if (AD->GetNumNodes(0))
			{
			ConfigureTB(FIC->Opt1, NULL, IDI_KEYEXIST, NULL);
			} // else
		else
			{
			ConfigureTB(FIC->Opt1, NULL, IDI_KEY, NULL);
			} // else
		} // else
	} // if
# endif // _WIN32

if (StyleRefresh & (WCSW_FI_STYLE_SRAH | WCSW_FI_STYLE_SRAH_COLOR | WCSW_FI_STYLE_SRAH_NOFIELD))
	{
	SRAHConfigButtons(Me, FIC, RAH, IRAH, RAHParent);
	} // if


# ifdef _WIN32
// Cause a Full-Widget repaint to sort out disabled states
InvalidateRect(Me, NULL, TRUE);
# endif // _WIN32        

if (FIC->Edit)
	{
	if (FIC->FIFlags & FIOFlag_AnimDouble)
		{
		SNADmFormat(Scratch, AD, AD->GetCurValue(), RAHParent);
		} /* if */

	# ifdef _WIN32
	SetWindowText(FIC->Edit, Scratch);
	SendMessage(FIC->Edit, EM_SETSEL, 0, (LPARAM)0);
	# endif // _WIN32
	} // if

} // SNADmSync

/*===========================================================================*/

ULONG SNADmStr(NativeControl Me, struct FloatIntConfig *FIC, AnimDouble *AD)
{
double MasterDouble, DR /* IncDecAmount */;
short MyID;
char ReSync = 0, Notify = 0;
char *Str;
NativeGUIWin Parent;
RasterAnimHost **IRAH = NULL, *RAH = NULL, *RAHParent = NULL;

if (!AD) return(0);
if (!FIC->Edit) return(0); // can't do anything w/o edit field

if (FIC && (FIC->FIFlags & FIOFlag_RAH))
	{
	RAH       = (RasterAnimHost *)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if
if (FIC && (FIC->FIFlags & FIOFlag_IRAH))
	{
	IRAH       = (RasterAnimHost **)FIC->MasterVariable; 
	RAHParent = (RasterAnimHost *)FIC->AuxVariable; 
	} // if

# ifdef _WIN32
Parent = (NativeControl) GetWindowLong(Me, GWL_HWNDPARENT);
MyID = (short) GetWindowLong(Me, GWL_ID);
# endif // _WIN32

//IncDecAmount = AD->GetIncrement();

# ifdef _WIN32
GetWindowText(FIC->Edit, Scratch, 50);
# endif // _WIN32

Str = Scratch;


SmartNumericPreviousValue = MasterDouble = AD->GetCurValue();

DR = SNADmUnformat(Str, AD, RAHParent);
if (DR > AD->GetMaxVal()) {DR = AD->GetMaxVal(); ReSync = 1;}
if (DR < AD->GetMinVal()) {DR = AD->GetMinVal(); ReSync = 1;}
SNADmFormat(FloatIntCompBuf, AD, MasterDouble, RAHParent);
if (strcmp(FloatIntCompBuf, Str))
	{
	Notify = 1;
	AD->SetCurValue(DR);
	} // if
	
if (1) // always resync for units conversion
//if (ReSync)
	{
	/* Update string gadget to reflect max/min/nonzero limiting */
	SNADmSync(Me, FIC, AD);
	} /* if */
/* This will trigger notifies */
if (Notify)
	{
#	ifdef _WIN32
	SendMessage(Parent, WM_WCSW_FI_CHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#	endif // _WIN32
	} // if
return(0);

} // SNADmStr

/*===========================================================================*/

ULONG SNmStr(NativeControl Me, struct FloatIntConfig *FIC)
{
char ReSync = 0, Notify = 0;

double *D, DR;
float *F, FR;
char *C, CR, *Str;
unsigned char *UC, UCR;
short *S, SR, MyID;
unsigned short *US, USR;
long *L, LR;
unsigned long *UL, ULR;
void *MasterVariable = NULL;
NativeGUIWin Parent;
SetCritter *Crit = NULL;
unsigned long CritID;
double IncDecAmount;
AnimDouble *AD = NULL;

# ifdef _WIN32
Parent = (NativeControl) GetWindowLong(Me, GWL_HWNDPARENT);
MyID = (short) GetWindowLong(Me, GWL_ID);
# endif // _WIN32
Scratch[0] = NULL;

if (FIC->FIFlags & FIOFlag_Critter)
	{
	Crit = (SetCritter *)FIC->MasterVariable;
	CritID = (unsigned long)FIC->CritID;
	} // if
else if (FIC->FIFlags & FIOFlag_AnimDouble)
	{
	AD = (AnimDouble *)FIC->MasterVariable;
	SNADmStr(Me, FIC, AD);
	return(0);
	} // else if
else
	{
	MasterVariable = (void *)FIC->MasterVariable;
	} // else

SmartNumericPreviousValue = 0.0; // not implemented except for SNAD widgets


if (FIC->FIFlags & FIOFlag_SmallIncDec)
	{
	IncDecAmount = ((double)(FIC->SmallIDA) / (double)FI_SmallIDAScale);
	} // if
else
	{
	IncDecAmount = FIC->IncDecAmount;
	} // else

if (!FIC->Edit) return(0);
# ifdef _WIN32
GetWindowText(FIC->Edit, Scratch, 50);
# endif // _WIN32

Str = Scratch;

if (Crit)
	{
	Crit->GetParam(&FloatIntDummyBuf, CritID);
	MasterVariable = (void *)&FloatIntDummyBuf;
	} // if

if (FIC->FIFlags & FIOFlag_Float)
	{
	F = (float *)MasterVariable;
	FR = (float)atof(Str);
	if (FR > FIC->MaxAmount) {FR = (float)FIC->MaxAmount; ReSync = 1;}
	if (FR < FIC->MinAmount) {FR = (float)FIC->MinAmount; ReSync = 1;}
	if ((FR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
		{
		FR = (float)IncDecAmount;
		ReSync = 1;
		} // if
	//if (*F != FR)
	//if (memcmp(F, &FR, sizeof(float)))
	//sprintf(FloatIntCompBuf, WCSW_FP_CONVERT_STRING, *F);
	sprintf(FloatIntCompBuf,  "%.*f", SNCalculateFractionalDigits((double)*F), *F);
	TrimDecimalZeros(FloatIntCompBuf);
	if (strcmp(FloatIntCompBuf, Str))
		{
		*F = FR;
		Notify = 1;
		if (Crit)
			{
#			ifdef _WIN32
			SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#			endif // _WIN32
			Crit->SetParam(1, CritID, FR, 0);
			} // if
		} // if
	} /* if */
else if (FIC->FIFlags & FIOFlag_Double)
	{
	D = (double *)MasterVariable;
	DR = atof(Str);
	if (DR > FIC->MaxAmount) {DR = (double)FIC->MaxAmount; ReSync = 1;}
	if (DR < FIC->MinAmount) {DR = (double)FIC->MinAmount; ReSync = 1;}
	if ((DR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
		{
		DR = (double)IncDecAmount;
		ReSync = 1;
		} // if
	//if (*D != DR)
	//if (memcmp(D, &DR, sizeof(double)))
	//sprintf(FloatIntCompBuf, WCSW_FP_CONVERT_STRING, *D);
	sprintf(FloatIntCompBuf,  "%.*f", SNCalculateFractionalDigits(*D), *D);
	TrimDecimalZeros(FloatIntCompBuf);
	if (strcmp(FloatIntCompBuf, Str))
		{
		*D = DR;
		Notify = 1;
		if (Crit)
			{
#			ifdef _WIN32
			SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#			endif // _WIN32
			Crit->SetParam(1, CritID, DR, 0);
			} // if
		} // if
	} /* if */
else if (FIC->FIFlags & FIOFlag_Char)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		UC = (unsigned char *)MasterVariable;
		UCR = (unsigned char)atoi(Str);
		if (UCR > FIC->MaxAmount) {UCR = (unsigned char)FIC->MaxAmount; ReSync = 1;}
		if (UCR < FIC->MinAmount) {UCR = (unsigned char)FIC->MinAmount; ReSync = 1;}
		if ((UCR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			UCR = (unsigned char)IncDecAmount;
			ReSync = 1;
			} // if
		if (*UC != UCR)
			{
			*UC = UCR;
			Notify = 1;
			if (Crit)
				{
#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, UCR, 0);
				} // if
			} // if
		} /* else */
	else
		{
		C = (char *)MasterVariable;
		CR = (char)atoi(Str);
		if (CR > FIC->MaxAmount) {CR = (char)FIC->MaxAmount; ReSync = 1;}
		if (CR < FIC->MinAmount) {CR = (char)FIC->MinAmount; ReSync = 1;}
		if ((CR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			CR = (char)IncDecAmount;
			ReSync = 1;
			} // if
		if (*C != CR)
			{
			*C = CR;
			Notify = 1;
			if (Crit)
				{

#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, CR, 0);
				} // if
			} // if
		} /* if */
	} /* if */
else if (FIC->FIFlags & FIOFlag_Short)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		US = (unsigned short *)MasterVariable;
		USR = (unsigned short)atoi(Str);
		if (USR > FIC->MaxAmount) {USR = (unsigned short)FIC->MaxAmount; ReSync = 1;}
		if (USR < FIC->MinAmount) {USR = (unsigned short)FIC->MinAmount; ReSync = 1;}
		if ((USR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			USR = (unsigned short)IncDecAmount;
			ReSync = 1;
			} // if
		if (*US != USR)
			{
			*US = USR;
			Notify = 1;
			if (Crit)
				{
#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, USR, 0);
				} // if
			} // if
		} /* else */
	else
		{
		S = (short *)MasterVariable;
		SR = (short)atoi(Str);
		if (SR > FIC->MaxAmount) {SR = (short)FIC->MaxAmount; ReSync = 1;}
		if (SR < FIC->MinAmount) {SR = (short)FIC->MinAmount; ReSync = 1;}
		if ((SR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			SR = (short)IncDecAmount;
			ReSync = 1;
			} // if
		if (*S != SR)
			{
			*S = SR;
			Notify = 1;
			if (Crit)
				{
#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, SR, 0);
				} // if
			} // if
		} /* if */
	} /* if */
else if (FIC->FIFlags & FIOFlag_Long)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		UL = (unsigned long *)MasterVariable;
		ULR = (unsigned long)atol(Str);
		if (ULR > FIC->MaxAmount) {ULR = (unsigned long)FIC->MaxAmount; ReSync = 1;}
		if (ULR < FIC->MinAmount) {ULR = (unsigned long)FIC->MinAmount; ReSync = 1;}
		if ((ULR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			ULR = (unsigned long)IncDecAmount;
			ReSync = 1;
			} // if
		if (*UL != ULR)
			{
			*UL = ULR;
			Notify = 1;
			if (Crit)
				{
#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, ULR, 0);
				} // if
			} // if
		} /* else */
	else
		{
		L = (long *)MasterVariable;
		LR = (long)atol(Str);
		if (LR > FIC->MaxAmount) {LR = (long)FIC->MaxAmount; ReSync = 1;}
		if (LR < FIC->MinAmount) {LR = (long)FIC->MinAmount; ReSync = 1;}
		if ((LR == 0) && (FIC->FIFlags & FIOFlag_NonZero))
			{
			LR = (long)IncDecAmount;
			ReSync = 1;
			} // if
		if (*L != LR)
			{
			*L = LR;
			Notify = 1;
			if (Crit)
				{
#				ifdef _WIN32
				SendMessage(Parent, WM_WCSW_FI_PRECHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#				endif // _WIN32
				Crit->SetParam(1, CritID, LR, 0);
				} // if
			} // if
		} /* if */
	} /* if */

if (ReSync)
	{
	/* Update string gadget to reflect max/min/nonzero limiting */
	SNmSync(Me, FIC);
	} /* if */
/* This will trigger notifies */
if (Notify)
	{
#	ifdef _WIN32
	SendMessage(Parent, WM_WCSW_FI_CHANGE, (WPARAM)(int)(MyID), (LPARAM)Me);
#	endif // _WIN32
	} // if
return(0);

} // SNmStr

/*===========================================================================*/

char *FormatAsPreferredUnit(char *Text, unsigned char MetricType, double TheD, double Increment, AnimDouble *AD, RasterAnimHost *RAHParent)
{
double ModD;
int PreferredUnit, WasNegative = 0;
short DisplayUnit;

if (((MetricType == WCS_ANIMDOUBLE_METRIC_LONGITUDE) || (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE))
 && (GlobalApp->MainProj->Prefs.LatLonSignDisplay == WCS_PROJPREFS_LATLONSIGN_ALPHABETIC))
	{
	// Remove negative sign, to be replaced by S or E/W as apropriate
	if (TheD < 0.0)
		{
		WasNegative = 1;
		} // if
	TheD = fabs(TheD);
	} // if

if ((MetricType == WCS_ANIMDOUBLE_METRIC_LONGITUDE)
 && (GlobalApp->MainProj->Prefs.LatLonSignDisplay == WCS_PROJPREFS_LATLONSIGN_NUMERIC)
 && (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST))
	{
	TheD = -TheD;
	} // if

//sprintf(SNADCompBuf, WCSW_FP_CONVERT_STRING, TheD);
sprintf(SNADCompBuf,  "%.*f", SNCalculateFractionalDigits(TheD), TheD);
if (strchr(SNADCompBuf, '.'))
	{
	TrimDecimalZeros(SNADCompBuf);
	} // if

if ((MetricType == WCS_ANIMDOUBLE_METRIC_HEIGHT) || (MetricType == WCS_ANIMDOUBLE_METRIC_DISTANCE))
	{
	if (MetricType == WCS_ANIMDOUBLE_METRIC_HEIGHT)
		{
		DisplayUnit = GlobalApp->MainProj->Prefs.VertDisplayUnits;
		} // if
	else
		{
		DisplayUnit = GlobalApp->MainProj->Prefs.HorDisplayUnits;
		} // else
	// Convert to Prefs Display Units
	PreferredUnit = GetNormalizedUnit(DisplayUnit, TheD);
	ModD = ConvertFromMeters(TheD, PreferredUnit);
//	sprintf(SNADCompBuf, WCSW_FP_CONVERT_STRING, ModD);
	sprintf(SNADCompBuf,  "%.*f", SNCalculateFractionalDigits(ModD), ModD);
	if (strchr(SNADCompBuf, '.'))
		{
		TrimDecimalZeros(SNADCompBuf);
		} // if
	strcat(SNADCompBuf, GetUnitSuffix(PreferredUnit));
	strcpy(Text, SNADCompBuf);
	} // if
else
	{
	switch (MetricType)
		{
		case WCS_ANIMDOUBLE_METRIC_TIME:
			{
			if ((GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES) &&
			 (GlobalApp->MainProj->Interactive->GetFrameRate() > 0))
				{
				long TempF;
				double Rate, MyInc, TempFrac;
				MyInc = Increment;
				if (MyInc == 0.0)
					{
					Rate = GlobalApp->MainProj->Interactive->GetFrameRate();
					} // if
				else
					{
					Rate = 1.0 / MyInc;
					} // else
				TempF = (long)(TheD * Rate);
				TempFrac = (TheD * Rate);
				//sprintf(SNADCompBuf, "%df", TempF);
				sprintf(SNADCompBuf, "%.2ff", TempFrac);
				} // else
			else
				{
				double TempS;
				long TempM;
				TempS = TheD;
				TempM = (long)(TempS / 60);
				if (TempM)
					{
					TempS -= (double)(long)(TempM * 60);
					//sprintf(SNADCompBuf, "%d:"WCSW_FP_CONVERT_STRING"", TempM, TempS);
					sprintf(SNADCompBuf, "%d:%.*f", TempM, SNCalculateFractionalDigits(TempS), TempS);
					} // if
				else
					{
					strcat(SNADCompBuf, "s");
					} // else
				} // if
			strcpy(Text, SNADCompBuf);
			break;
			} // 
		case WCS_ANIMDOUBLE_METRIC_VELOCITY:
			{
			if ((GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES) &&
			 (GlobalApp->MainProj->Interactive->GetFrameRate() > 0))
				{
				double Rate, TempD;
				Rate = GlobalApp->MainProj->Interactive->GetFrameRate();
				TempD = (TheD / Rate);
				//sprintf(SNADCompBuf, WCSW_FP_CONVERT_STRING, TempD);
				sprintf(SNADCompBuf,  "%.*f", SNCalculateFractionalDigits(TempD), TempD);
				if (strchr(SNADCompBuf, '.'))
					{
					TrimDecimalZeros(SNADCompBuf);
					} // if
				strcat(SNADCompBuf, "m/f");
				} // else
			else
				{
				strcat(SNADCompBuf, "m/s");
				} // if
			strcpy(Text, SNADCompBuf);
			break;
			} // 
		case WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS:
			{
			// do nothing.
			strcpy(Text, SNADCompBuf);
			break;
			} // 
		case WCS_ANIMDOUBLE_METRIC_ANGLE:
			{
			strcat(SNADCompBuf, WCS_SYMBOL_DEGREE);
			strcpy(Text, SNADCompBuf);
			break;
			} // 
		case WCS_ANIMDOUBLE_METRIC_LATITUDE:
		case WCS_ANIMDOUBLE_METRIC_LONGITUDE:
			{
			char NoDMS = 0, Projected = 0;
			#ifdef WCS_WIDGET_SRAH_SUPPORT_COORDS
			CoordSys *DefCS = NULL;
			PlanetOpt *DefPO = NULL;
			GeneralEffect *CurEffect;
			AnimCritter *Associate = NULL;

			if (AD && RAHParent)
				{
				DefPO = NULL;
				if (CurEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
					{
					while (CurEffect)
						{
						if (CurEffect->Enabled)
							{
							DefPO = (PlanetOpt *)CurEffect;
							}// if
						CurEffect = CurEffect->Next;
						} // while
					} // if

				if (DefPO && (DefCS = DefPO->Coords))
					{
					if ((DefCS->Method.GCTPMethod != WCS_PROJECTIONCODE_GEO) && (GlobalApp->MainProj->Prefs.DisplayGeoUnitsProjected))
						{
						if (Associate = SRAHGetAssociatedAD(RAHParent, AD))
							{
							VertexDEM Projector;
							// This code ignores the Multiplier field, which I believe is undefined for Lat/Lon values anyway
							Projector.Elev = 0.0; // projection is 2d only for now.
							if (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE)
								{
								Projector.Lat = AD->GetCurValue();
								Projector.Lon = Associate->GetCurValue(0);
								} // if
							else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
								{
								Projector.Lat = Associate->GetCurValue(0);
								Projector.Lon = AD->GetCurValue();
								} // 
							DefCS->DefDegToProj(&Projector);
							// testing code -- reverse transform to verify
							//Projector.Lat = Projector.Lon = 0.0;
							//DefCS->ProjToDefDeg(&Projector);
							if (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE)
								{
								TheD = Projector.xyz[1];
								} // if
							else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
								{
								TheD = Projector.xyz[0];
								} // else

							// Convert projected value to preferred horizontal units
							Projected = 1;
							} // if
						} // if
					} // if
				} // if
			#endif // WCS_WIDGET_SRAH_SUPPORT_COORDS
			if (TheD == (double)(int)TheD) NoDMS = 1;
			if (Projected)
				{
				PreferredUnit = GetNormalizedUnit(GlobalApp->MainProj->Prefs.HorDisplayUnits, TheD);
				ModD = ConvertFromMeters(TheD, GlobalApp->MainProj->Prefs.HorDisplayUnits);
				sprintf(SNADCompBuf,  "%.*f", SNCalculateFractionalDigits(ModD), ModD);
				if (strchr(SNADCompBuf, '.'))
					{
					TrimDecimalZeros(SNADCompBuf);
					} // if
				strcat(SNADCompBuf, GetUnitSuffix(GlobalApp->MainProj->Prefs.HorDisplayUnits));
				} // if
			else if ((GlobalApp->MainProj->Prefs.AngleDisplayUnits == WCS_PROJPREFS_ANGLEUNITS_DECDEG) || NoDMS)
				{
				strcat(SNADCompBuf, WCS_SYMBOL_DEGREE);
				} // if
			else // if (GlobalApp->MainProj->Prefs.AngleDisplayUnits == WCS_PROJPREFS_ANGLEUNITS_DMS)
				{
				double TempD, TempM, TempS;
				DecimalToDMS(TheD, TempD, TempM, TempS);
				//sprintf(SNADCompBuf, "%.0f"WCS_SYMBOL_DEGREE"%.0f\'"WCSW_FP_CONVERT_STRING"\"", TempD, TempM, TempS);
				sprintf(SNADCompBuf, "%.0f"WCS_SYMBOL_DEGREE"%.0f\'%.*f\"", TempD, TempM, SNCalculateFractionalDigits(TempS), TempS);
				//  , , 
				} // else
			if (GlobalApp->MainProj->Prefs.LatLonSignDisplay == WCS_PROJPREFS_LATLONSIGN_NUMERIC)
				{
				sprintf(Text, "%s", SNADCompBuf);
				/*
				if (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE)
					{
					sprintf(Text, "%s", SNADCompBuf);
					} // if
				else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
					{
					sprintf(Text, "%s", SNADCompBuf);
					} // else
				*/
				} // if
			else if (DefCS && ((DefCS->Method.GCTPMethod != WCS_PROJECTIONCODE_GEO) && (GlobalApp->MainProj->Prefs.DisplayGeoUnitsProjected))) // Alphabetic, but might fall back if we're projected
				{
				// format without NSEW decorations
				sprintf(Text, "%s", SNADCompBuf);
				} // else if
			else // WCS_PROJPREFS_LATLONSIGN_ALPHABETIC
				{
				if ((TheD < 0.0) || WasNegative)
					{
					if (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE)
						{
						sprintf(Text, "S%s", SNADCompBuf);
						} // if
					else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
						{
						sprintf(Text, "E%s", SNADCompBuf);
						} // else
					} // if
				else if (TheD > 0.0)
					{
					if (MetricType == WCS_ANIMDOUBLE_METRIC_LATITUDE)
						{
						sprintf(Text, "N%s", SNADCompBuf);
						} // if
					else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
						{
						sprintf(Text, "W%s", SNADCompBuf);
						} // else
					} // else if
				else
					{
					sprintf(Text, "%s", SNADCompBuf);
					} // else
				} // else
			break;
			} // 
		} // MetricType
	} // if
return(Text);

} // FormatAsPreferredUnit

/*===========================================================================*/

char *FormatAsPreferredUnit(char *Text, AnimDouble *AD, double TheD, RasterAnimHost *RAHParent)
{

if (AD->GetMultiplier() != 0.0)
	{
	TheD *= AD->GetMultiplier();
	} // if

FormatAsPreferredUnit(Text, AD->GetMetricType(), TheD, AD->GetIncrement(), AD, RAHParent);

return(Text);

} // FormatAsPreferredUnit

/*===========================================================================*/

char *FormatAsPreferredUnit(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent)
{
double TheD;

TheD = AD->GetCurValue();

if (AD->GetMultiplier() != 0.0)
	{
	TheD *= AD->GetMultiplier();
	} // if

FormatAsPreferredUnit(Text, AD->GetMetricType(), TheD, AD->GetIncrement(), AD, RAHParent);
return(Text);

} // FormatAsPreferredUnit

/*===========================================================================*/

void SNADmFormat(char *Text, AnimDouble *AD, double TheD, RasterAnimHost *RAHParent)
{

if (AD->GetMultiplier() != 0.0)
	{
	TheD *= AD->GetMultiplier();
	} // if

FormatAsPreferredUnit(Text, AD->GetMetricType(), TheD, AD->GetIncrement(), AD, RAHParent);

} // SNADmFormat

/*===========================================================================*/

int SNCalculateFractionalDigits(double Val)
{
int IntVal, IntDigits, FracDigits;
// Frank Weed suggested this neat trick to calculate the number of
// digits necessary to represent a number in a particular base,
// here base 10.
IntVal = abs((int)Val);
if (IntVal == 0)
	{
	IntDigits = 0;
	} // if
else
	{
	IntDigits = (int)((log10((double)IntVal) /* / log(base) */) + 1.0);
	} // else
FracDigits = GlobalApp->MainProj->Prefs.SignificantDigits - IntDigits;

//if (Val < 0) FracDigits--; // if < 0, we have a '-' character too
if (fabs(Val) < 1.0) FracDigits--; // if abs(Val) < 1, we have a leading 0, as in 0.5 or -0.5

// we always have a decimal point to account for
//FracDigits--;

if (FracDigits < 0) FracDigits = 0;

return(FracDigits);

} // SNCalculateFractionalDigits

/*===========================================================================*/

double SNADmUnformat(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent)
{
double NewVal;
char *TempText = NULL, EmptyText[5];

if ((!AD) || (!Text)) return(0.0);

EmptyText[0] = NULL;

if (isspace((unsigned char)Text[0]))
	{
	TempText = SkipPastNextSpace(Text);
	if (TempText) Text = TempText;
	else Text = EmptyText;
	} // if
TrimTrailingSpaces(Text);

NewVal = atof(Text);

// Units conversion
if ((AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_HEIGHT) || (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_DISTANCE))
	{
	NewVal = (SNADmUnformatLinear(Text, AD));
	} // if
else
	{
	switch (AD->GetMetricType())
		{
		case WCS_ANIMDOUBLE_METRIC_VELOCITY:
			{
			NewVal = (SNADmUnformatVelocity(Text, AD));
			break;
			} // Velocity
		case WCS_ANIMDOUBLE_METRIC_TIME:
			{
			NewVal = (SNADmUnformatTime(Text, AD));
			break;
			} // TIME
		case WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS:
		case WCS_ANIMDOUBLE_METRIC_ANGLE:
			{
			// do nothing.
			break;
			} // 
		case WCS_ANIMDOUBLE_METRIC_LATITUDE:
		case WCS_ANIMDOUBLE_METRIC_LONGITUDE:
			{
			NewVal = (SNADmUnformatLatLon(Text, AD, RAHParent));
			break;
			} // 
		} // MetricType
	} // else

if (AD->GetMultiplier() != 0.0)
	{
	NewVal = NewVal / AD->GetMultiplier();
	} // if

return(NewVal);

} // SNADmUnformat

/*===========================================================================*/

double SNADmUnformatLinear(char *Text, AnimDouble *AD)
{
double NewVal = 0.0, UnitVal = 0.0;
char *Advance, SuffixUnit, NumElement, NotIdentified = 1, Negate = 0;

	memset(SNADDeCompBufA, 0, WCS_WIDGET_COMPBUF_SIZE);
	Advance = Text;

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	// Process negative
	if (Advance[0] == '-')
		{
		Negate = 1;
		Advance++;
		} // if

    // read contiguous digits
	for (NumElement = 0; isdigit((unsigned char)Advance[0]) || Advance[0] == '.'; Advance++)
		{
		SNADDeCompBufA[NumElement++] = Advance[0];
		} // 
	SNADDeCompBufA[NumElement] = NULL;
	if (NumElement)
		{
		UnitVal = atof(SNADDeCompBufA);
		} // if

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	// Look for unit
	memset(SNADDeCompBufA, 0, WCS_WIDGET_COMPBUF_SIZE);
	for (NumElement = 0; Advance[0]; Advance++)
		{
		// ignore infix or trailing spaces
		if (!isspace((unsigned char)Advance[0]))
			{
			SNADDeCompBufA[NumElement++] = Advance[0];
			} // if
		} // 
	SNADDeCompBufA[NumElement] = NULL;
	if (NumElement)
		{ // identify unit
		SuffixUnit = MatchUnitSuffix(SNADDeCompBufA);
		if (SuffixUnit != -1)
			{
			NewVal = ConvertToMeters(UnitVal, SuffixUnit);
			NotIdentified = 0;
			} // if
		else
			{ // unidentified unit suffix, ignore
			NotIdentified = 0;
			} // else
		} // if
	if (NotIdentified)
		{ // assume default unit
		if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_HEIGHT)
			{
			NewVal = ConvertToMeters(UnitVal, GlobalApp->MainProj->Prefs.VertDisplayUnits);
			} // if
		else
			{
			NewVal = ConvertToMeters(UnitVal, GlobalApp->MainProj->Prefs.HorDisplayUnits);
			} // else
		NotIdentified = 0;
		} // if

if (Negate) NewVal = -NewVal;

return(NewVal);

} // SNADmUnformatLinear

/*===========================================================================*/

double SNADmUnformatVelocity(char *Text, AnimDouble *AD)
{
double NewVal = 0.0, UnitVal = 0.0;
char *Advance, SuffixUnit, NumElement, TimeNotIdentified = 1, DistNotIdentified = 1,
 IsFrames = 0, IsSec = 0, IsMin = 0, IsHour = 0, IsNeg = 0;

	memset(SNADDeCompBufA, 0, WCS_WIDGET_COMPBUF_SIZE);
	Advance = Text;

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	// deal with possible minus sign
	if (Advance[0] == '-')
		{
		IsNeg = 1;
		Advance++;
		} // if

    // read contiguous digits
	for (NumElement = 0; isdigit((unsigned char)Advance[0]) || Advance[0] == '.'; Advance++)
		{
		SNADDeCompBufA[NumElement++] = Advance[0];
		} // 
	SNADDeCompBufA[NumElement] = NULL;
	if (NumElement)
		{
		UnitVal = atof(SNADDeCompBufA);
		} // if

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	// Look for unit
	memset(SNADDeCompBufA, 0, WCS_WIDGET_COMPBUF_SIZE);
	for (NumElement = 0; Advance[0]; Advance++)
		{
		// ignore infix or trailing spaces
		if (!isspace((unsigned char)Advance[0]))
			{
			SNADDeCompBufA[NumElement++] = Advance[0];
			} // if
		} // 
	SNADDeCompBufA[NumElement] = NULL;
	if (NumElement)
		{ // identify time unit
		if (strlen(SNADDeCompBufA) > 2)
			{
			if ((SNADDeCompBufA[strlen(SNADDeCompBufA) - 2] == '/') || (tolower((unsigned char)SNADDeCompBufA[strlen(SNADDeCompBufA) - 2]) == 'p'))
				{
				switch (tolower((unsigned char)SNADDeCompBufA[strlen(SNADDeCompBufA) - 1]))
					{
					case 'f':
						{
						IsFrames = 1;
						TimeNotIdentified = 0;
						break;
						} // frames
					case 's':
						{
						IsSec = 1;
						TimeNotIdentified = 0;
						break;
						} // seconds
					case 'm':
						{
						IsMin = 1;
						TimeNotIdentified = 0;
						break;
						} // minutes
					case 'h':
						{
						IsHour = 1;
						TimeNotIdentified = 0;
						break;
						} // hours
					} // switch
				// chop off slash and timeunit
				SNADDeCompBufA[strlen(SNADDeCompBufA) - 2] = NULL;
				} // if
			} // if

		// identify distance unit
		SuffixUnit = MatchUnitSuffix(SNADDeCompBufA);
		if (SuffixUnit != -1)
			{
			NewVal = ConvertToMeters(UnitVal, SuffixUnit);
			DistNotIdentified = 0;
			} // if
		else
			{ // unidentified unit suffix, ignore
			DistNotIdentified = 1;
			} // else
		} // if
	if (DistNotIdentified)
		{ // assume default HORIZONTAL unit
		NewVal = ConvertToMeters(UnitVal, GlobalApp->MainProj->Prefs.HorDisplayUnits);
		DistNotIdentified = 0;
		} // else
	if (TimeNotIdentified)
		{
		if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES) IsFrames = 1;
		else IsSec = 1;
		TimeNotIdentified = 0;
		} // else

	// Convert from m/f to m/s if necessary
	if (IsFrames)
		{
		NewVal *= GlobalApp->MainProj->Interactive->GetFrameRate();
		} // 
	else if (IsMin)
		{
		NewVal /= 60.0;
		} // 
	if (IsHour)
		{
		NewVal /= 3600.0;
		} // 

	if (IsNeg)
		{
		NewVal = -NewVal;
		} // if

return(NewVal);

} // SNADmUnformatVelocity

/*===========================================================================*/

double SNADmUnformatTime(char *Text, AnimDouble *AD)
{
double NewVal = 0.0, Rate, MyInc = 0.0;
int IsFrames = 0, HasMin = 0;

IsFrames = ((GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES) ? 1 : 0);
if (strchr(Text, 'f')) IsFrames = 1;
if (strchr(Text, 's')) IsFrames = 0;
if (strchr(Text, ':')) {IsFrames = 0; HasMin = 1;}

NewVal = atof(Text);

if (IsFrames)
	{
	if(AD) // can be NULL
		{
		MyInc = AD->GetIncrement();
		} // if
	if (MyInc == 0.0)
		{
		Rate = GlobalApp->MainProj->Interactive->GetFrameRate();
		} // if
	else
		{
		Rate = 1.0 / MyInc;
		} // else
	if (Rate > 0)
		{
		NewVal = NewVal / Rate;
		} // if
	else
		{
		NewVal = 0;
		} // else
	} // if
else
	{
	if (HasMin)
		{
		long TempM;
		char *TempStrS;
		// SNADDeCompBufA
		if (TempStrS = strchr(Text, ':'))
			{
			TempStrS[0] = NULL;
			TempStrS = &TempStrS[1];
			TempM = atoi(Text);
			NewVal = atof(TempStrS);
			NewVal += (double)(long)(TempM * 60);
			} // if
		} // if
	} // else

return(NewVal);

} // SNADmUnformatTime

/*===========================================================================*/

double SNADmUnformatLatLon(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent)
{
// Valid Lat/Lon sentance format: (trailing [NnSsEeWw] no longer allowed.)
// [NnSsEeWw][+-]122.75[:d° ]59.22[m' ]54.72[s" ]
//        N112°59'36.043"
//        -112.75
//        s112.13d59m36.043s
double NewVal = 0.0, ValD = 0.0, ValM = 0.0, ValS = 0.0, TempVal;
//int IsDMS = 0;
char ElementMap[] = "dms";
char *Advance, LatDir, LonDir, SuffixChar,
 FlipDir = 0, TestSym, NumElement,
 FormatError = 0, AtEnd = 0, MayBeProjected = 0, Projected = 0;

#ifdef WCS_WIDGET_SRAH_SUPPORT_COORDS
CoordSys *DefCS;
PlanetOpt *DefPO;
GeneralEffect *CurEffect;
AnimCritter *Associate = NULL;
#endif // WCS_WIDGET_SRAH_SUPPORT_COORDS

enum
	{
	WCS_WIDGET_LATDIR_NOT_SPEC	= -1,
	WCS_WIDGET_LATDIR_SOUTH		= 0,
	WCS_WIDGET_LATDIR_NORTH		= 1
	};

enum
	{
	WCS_WIDGET_LONDIR_NOT_SPEC	= -1,
	WCS_WIDGET_LONDIR_EAST		= 0,
	WCS_WIDGET_LONDIR_WEST		= 1
	};

LatDir = WCS_WIDGET_LATDIR_NOT_SPEC;
LonDir = WCS_WIDGET_LONDIR_NOT_SPEC;


#ifdef WCS_WIDGET_SRAH_SUPPORT_COORDS
if (AD && RAHParent)
	{
	DefPO = NULL;
	if (CurEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
		{
		while (CurEffect)
			{
			if (CurEffect->Enabled)
				{
				DefPO = (PlanetOpt *)CurEffect;
				}// if
			CurEffect = CurEffect->Next;
			} // while
		} // if

	if (DefPO && (DefCS = DefPO->Coords))
		{
		if ((DefCS->Method.GCTPMethod != WCS_PROJECTIONCODE_GEO) && (GlobalApp->MainProj->Prefs.DisplayGeoUnitsProjected))
			{
			MayBeProjected = 1;
			} // if
		} // if
	} // if
#endif // WCS_WIDGET_SRAH_SUPPORT_COORDS

if (1) // IsDMS may be useless now that we're a full parser...
	{
	// Clear buffers
	memset(SNADDeCompBufA, 0, WCS_WIDGET_COMPBUF_SIZE);
	Advance = Text;

	// Parse any leading NSEW symbols
	TestSym = toupper(Advance[0]);
	if (TestSym == 'N') {LatDir = WCS_WIDGET_LATDIR_NORTH; Advance++;}
	if (TestSym == 'S') {LatDir = WCS_WIDGET_LATDIR_SOUTH; Advance++;}
	if (TestSym == 'E') {LonDir = WCS_WIDGET_LONDIR_EAST; Advance++;}
	if (TestSym == 'W') {LonDir = WCS_WIDGET_LONDIR_WEST; Advance++;}

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	// Process any leading sign
	if (Advance[0] == '+') {FlipDir = 0; Advance++;}
	if (Advance[0] == '-') {FlipDir = 1; Advance++;}

	// Skip any whitespace
	for (;isspace((unsigned char)Advance[0]); Advance++);	//lint !e722

	
	for (TempVal = 0.0, SuffixChar = NULL, NumElement = 0; NumElement < 3; /*NumElement++*/)
		{
		SuffixChar = NULL;
		if (MayBeProjected)
			{
			// limited set of suffix identifiers so as not to conflict with linear unit suffixes
			if (!(Advance = SNADParseNumberSuffix(Advance, SuffixChar, WCS_SYMBOL_DEGREE":\'\"", TempVal)))
				{
				// try linear form
				TempVal = SNADmUnformatLinear(Text, AD);
				Advance = &Text[strlen(Text)]; // ensure it is non-null so we go forward
				} // if
			} // if
		else
			{
			Advance = SNADParseNumberSuffix(Advance, SuffixChar, WCS_SYMBOL_DEGREE":DdMm\'Ss\"", TempVal);
			} // else
		if (Advance)
			{
			if (SuffixChar == NULL)
				{
				if (MayBeProjected)
					{
					Projected = 1;
					//TempVal = SNADmUnformatLinear(Advance, AD);
					if ((AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LONGITUDE))
						{
						SuffixChar = 'x';
						} // if
					else // LATITUDE
						{
						SuffixChar = 'y';
						} // else
					} // if
				else
					{
					SuffixChar = ElementMap[NumElement];
					} // else
				} // determine next apropriate element
			switch (SuffixChar)
				{
				case 'x':
				case 'y':
					{
					Projected = 1;
					ValD = TempVal;
					NumElement = 3; // can't handle multi-element strings in projected, so stop element loop
					break;
					} // degree
				case ':':
				case 'd':
				case 'D':
				case (signed char)WCS_SYMBOL_DEGREE_CHAR:
					{
					ValD = TempVal;
					NumElement++;
					break;
					} // degree
				case 'm':
				case 'M':
				case '\'':
					{
					ValM = TempVal / 60;
					NumElement++;
					break;
					} // minute
				case 's':
				case 'S':
				case '\"':
					{
					ValS = TempVal / 3600;
					NumElement++;
					break;
					} // second
				} // suffix
			} // if
		else
			{
			if (NumElement)
				{
				break;
				} // if
			else
				{
				FormatError = 1;
				return(0.0);
				} // else
			} // else
		} // for

	NewVal = ValD + ValM + ValS;
	} // if

if (!Projected)
	{
	// Don't try to use NS prefixes with Longitude fields and EW with latitude, you fool.
	if ((LatDir != WCS_WIDGET_LATDIR_NOT_SPEC) && (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LONGITUDE))
		{
		FormatError = 1; return(0.0);
		} // if
	if ((LonDir != WCS_WIDGET_LONDIR_NOT_SPEC) && (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LATITUDE))
		{
		FormatError = 1; return(0.0);
		} // if

	// Account for East=Positive notation if you didn't specify the longitude direction
	if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LONGITUDE)
		{
		if (LonDir == WCS_WIDGET_LONDIR_NOT_SPEC)
			{
			if ((GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST))
				{
				FlipDir = !FlipDir;
				} // if
			} // if
		else if (LonDir == WCS_WIDGET_LONDIR_EAST)
			{
			FlipDir = !FlipDir;
			} // else if
		} // if

	if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LATITUDE)
		{
		if (LatDir == WCS_WIDGET_LATDIR_NOT_SPEC)
			{
			// assume north -- do nothing further
			} // if
		else if (LatDir == WCS_WIDGET_LATDIR_SOUTH)
			{
			FlipDir = !FlipDir;
			} // else if
		} // if

	if (FlipDir) NewVal = -NewVal;
	} // if
else
	{ // projected
	if ((LatDir != WCS_WIDGET_LATDIR_NOT_SPEC) || (LonDir != WCS_WIDGET_LONDIR_NOT_SPEC))
		{
		// these don't make any consistent sense for projected coords
		// <<<>>> Should we silently ignore them, or return an error?
		FormatError = 1; return(0.0);
		} // if
	// convert back to lat/lon


#ifdef WCS_WIDGET_SRAH_SUPPORT_COORDS
	if (Associate = SRAHGetAssociatedAD(RAHParent, AD))
		{
		VertexDEM Projector;

		Projector.Elev = 0.0; // projection is 2d only for now.
		// we have to project the old values to recover the projected version of our associate
		if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LATITUDE)
			{
			Projector.Lat = AD->GetCurValue();
			Projector.Lon = Associate->GetCurValue(0);
			// 11/17/06 CXH:
			// I don't believe flipping this makes sense.
			// For projected, a negative value negates the input Latitude, which borks the projection
			// We never reach this code if we're already geographic
			//if (FlipDir) Projector.Lat = -Projector.Lat;
			} // if
		else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
			{
			Projector.Lat = Associate->GetCurValue(0);
			Projector.Lon = AD->GetCurValue();
			// 11/17/06 CXH:
			// I don't believe flipping this makes sense.
			// For projected, a negative value negates the input Longitude, which borks the projection
			// We never reach this code if we're already geographic
			//if (FlipDir) Projector.Lon = -Projector.Lon;
			} // 
		DefCS->DefDegToProj(&Projector);

		// This code ignores the Multiplier field, which I believe is undefined for Lat/Lon values anyway
		Projector.Elev = 0.0; // projection is 2d only for now.
		// pop in our altered projected value so we can now unproject
		if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LATITUDE)
			{
			Projector.xyz[1] = NewVal;
			} // if
		else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
			{
			Projector.xyz[0] = NewVal;
			} // else

		DefCS->ProjToDefDeg(&Projector);
		if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_LATITUDE)
			{
			NewVal = Projector.Lat;
			Associate->SetCurValue(0, Projector.Lon); // I don't think causes a resync
			} // if
		else // WCS_ANIMDOUBLE_METRIC_LONGITUDE
			{
			Associate->SetCurValue(0, Projector.Lat); // I don't think causes a resync
			NewVal = Projector.Lon;
			} // 
		} // if
#endif // WCS_WIDGET_SRAH_SUPPORT_COORDS
	} // else Projected

return(NewVal);

} // SNADmUnformatLatLon

/*===========================================================================*/

char *SNADParseNumberSuffix(char *Advance, char &SuffixChar, char *ValidSuffixes, double &NumberVal)
{
char *NumberMark, NumDec, NumDigits, FormatError = 0, AtEnd = 0;

for (NumberMark = NULL, NumDec = NumDigits = 0; ((isdigit((unsigned char)Advance[0]) ) || (Advance[0] == '.')); Advance++)
	{
	if (Advance[0] == '.')
		{
		if (NumDec)
			{
			FormatError = 1;
			return(NULL);
			} // if
		NumDec++;
		} // if
	if (!NumDigits)
		{
		NumberMark = &Advance[0];
		} // if
	NumDigits++;
	} // for

if ((Advance[0] == NULL) || isspace((unsigned char)Advance[0]) || MatchChar(Advance[0], ValidSuffixes))
	{
	if (NumberMark)
		{
		if (NumDigits)
			{
			memcpy(SNADDeCompBufA, NumberMark, NumDigits);
			SNADDeCompBufA[NumDigits] = NULL;
			NumberVal = atof(SNADDeCompBufA);
			if (!((Advance[0] == NULL) || isspace((unsigned char)Advance[0])))
				{
				SuffixChar = Advance[0];
				Advance++;
				} // if
			return(Advance);
			} // if
		else
			{
			FormatError = 0;
			return(NULL);
			} // else
		} // if
	if (Advance[0] == NULL)
		{
		AtEnd = 0;
		} // if
	} // if
else
	{
	FormatError = 1;
	return(NULL);
	} // else

return(NULL);

} // SNADParseNumberSuffix

/*===========================================================================*/

ULONG SNDoIncDec(NativeControl Me,  struct FloatIntConfig *FIC, int Action)
{
double *D;
float *F;
char *C;
unsigned char *UC;
short *S;
unsigned short *US;
long *L;
unsigned long *UL;
void *MasterVariable = NULL;
SetCritter *Crit = NULL;
NativeGUIWin MyParent;
AnimDouble *AD = NULL;

SmartNumericPreviousValue = 0.0;
if (FIC->FIFlags & FIOFlag_Critter)
	{
	Crit = (SetCritter *)FIC->MasterVariable;
	} // if
else if (FIC->FIFlags & FIOFlag_AnimDouble)
	{
	if (AD = (AnimDouble *)FIC->MasterVariable)
		{
		SmartNumericPreviousValue = AD->GetCurValue();
		} // if
	} // else if
else
	{
	MasterVariable = (void *)FIC->MasterVariable;
	} // else

if (Crit)
	{
	MasterVariable = (void *)&FloatIntDummyBuf;
	Crit->GetParam(MasterVariable, FIC->CritID);
	} // if

if (AD)
	{ // A Cast of Thousands, or thousands of casts...
	MasterVariable = (void *)&FloatIntDummyBuf;
	D = (double *)MasterVariable;
	*D = AD->GetCurValue();
	} // if

if (!MasterVariable) return(0);
#ifdef _WIN32
MyParent = GetParent(Me);
#endif // _WIN32

if (FIC->FIFlags & FIOFlag_Float)
	{
	F = (float *)MasterVariable;
	*F = (float)SNCalcIncDec((double)*F, FIC, Action);
#		ifdef _WIN32
		SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#		endif // _WIN32
	if (Crit)
		{
		Crit->SetParam(1, FIC->CritID, *F, 0);
		} // if
	} /* if */
else if ((FIC->FIFlags & FIOFlag_Double) || (FIC->FIFlags & FIOFlag_AnimDouble))
	{
	D = (double *)MasterVariable;
	*D = SNCalcIncDec((double)*D, FIC, Action);
#		ifdef _WIN32
		SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#		endif // _WIN32
	if (Crit)
		{
		Crit->SetParam(1, FIC->CritID, *D, 0);
		} // if
	else if (AD)
		{
		AD->SetCurValue(*D);
		} // if
	} /* if */
else if (FIC->FIFlags & FIOFlag_Char)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		UC = (unsigned char *)MasterVariable;
		*UC = (unsigned char)SNCalcIncDec((double)*UC, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *UC, 0);
			} // if
		} /* if */
	else
		{
		C = (char *)MasterVariable;
		*C = (char)SNCalcIncDec((double)*C, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *C, 0);
			} // if
		} /* else */
	} /* if */
else if (FIC->FIFlags & FIOFlag_Short)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		US = (unsigned short *)MasterVariable;
		*US = (unsigned short)SNCalcIncDec((double)*US, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *US, 0);
			} // if
		} /* if */
	else
		{
		S = (short *)MasterVariable;
		*S = (short)SNCalcIncDec((double)*S, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *S, 0);
			} // if
		} /* else */
	} /* if */
else if (FIC->FIFlags & FIOFlag_Long)
	{
	if (FIC->FIFlags & FIOFlag_Unsigned)
		{
		UL = (unsigned long *)MasterVariable;
		*UL = (unsigned long)SNCalcIncDec((double)*UL, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *UL, 0);
			} // if
		} /* if */
	else
		{
		L = (long *)MasterVariable;
		*L = (long)SNCalcIncDec((double)*L, FIC, Action);
#			ifdef _WIN32
			SendMessage(MyParent, WM_WCSW_FI_PRECHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
#			endif // _WIN32
		if (Crit)
			{
			Crit->SetParam(1, FIC->CritID, *L, 0);
			} // if
		} /* else */
	} /* if */

/* Resync the string gadget */
SNmSync(Me, FIC);
#			ifdef _WIN32
			if (FIC->FIFlags & FIOFlag_ArrowNotify)
				{
				SendMessage(MyParent, WM_WCSW_FI_CHANGEARROW, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
				}
			else
				{
				SendMessage(MyParent, WM_WCSW_FI_CHANGE, (WPARAM)(GetWindowLong(Me, GWL_ID)), (LPARAM)Me);
				} // else
#			endif // _WIN32
return(0);

} // SNDoIncDec

/*===========================================================================*/

double SNCalcIncDec(double Quantity, struct FloatIntConfig *FIC, int Action)
{
double Step, Result;
double Rate, IncDecAmount, MaxAmount, MinAmount;
unsigned int ActionLoop;
AnimDouble *AD = NULL;

if (FIC->FIFlags & FIOFlag_SmallIncDec)
	{
	IncDecAmount = ((double)(FIC->SmallIDA) / (double)FI_SmallIDAScale);
	MaxAmount = FIC->MaxAmount;
	MinAmount = FIC->MinAmount;
	} // if
else if (FIC->FIFlags & FIOFlag_AnimDouble)
	{
	AD = (AnimDouble *)FIC->MasterVariable;
	IncDecAmount = AD->GetIncrement();
	if (IncDecAmount == 0.0)
		{
		if (AD->GetMetricType() == WCS_ANIMDOUBLE_METRIC_TIME)
			{
			Rate = GlobalApp->MainProj->Interactive->GetFrameRate();
			if (Rate != 0.0)
				{
				IncDecAmount = 1.0 / Rate;
				} // if
			} // if
		} // if
	MaxAmount = AD->GetMaxVal();
	MinAmount = AD->GetMinVal();
	} // else if
else
	{
	IncDecAmount = FIC->IncDecAmount;
	MaxAmount = FIC->MaxAmount;
	MinAmount = FIC->MinAmount;
	} // else

for (ActionLoop = 0; ActionLoop < (unsigned int)abs(Action); ActionLoop++)
	{
	Result = Quantity;

	if (FIC->FIFlags & FIOFlag_Frac)
		{
		if (Action > 0)
			{
			Step = fabs(Quantity - ((1 / IncDecAmount) * Quantity));
			}
		else
			{
			Step = Quantity * IncDecAmount;
			}
		} // if
	else
		{
		Step = IncDecAmount;
		} // else

	if (Action > 0)
		{
		if (((Quantity + Step) <= MaxAmount) && ((Quantity + Step) >= MinAmount))
			{
			Result = Quantity + Step;
			} // if

		else
			{
			if ((Quantity + Step) > MaxAmount)
				{
				Result = MaxAmount;
				} // if
			if ((Quantity + Step) < MinAmount)
				{
				Result = MinAmount;
				} // if
			} // else
		} // if
	else // if (Action == 0)
		{
		if (((Quantity - Step) <= MaxAmount) && ((Quantity - Step) >= MinAmount))
			{
			Result = Quantity - Step;
			} // if
		else
			{
			if ((Quantity - Step) > MaxAmount)
				{
				Result = MaxAmount;
				} // if
			if ((Quantity - Step) < MinAmount)
				{
				Result = MinAmount;
				} // if
			} // else
		} // if

	if (FIC->FIFlags & FIOFlag_NonZero)
		{
		if (Result == 0)
			{
			Result = IncDecAmount;
			} // if
		} // if
	Quantity = Result;
	} // for

return(Result);

} // SNCalcIncDec

/*===========================================================================*/

static struct ColorBarConfig CBCfg;

void ConfigureCB(NativeControl Dest, int Target, int Percent, unsigned long Flags,
				 unsigned char ColorPotNum, AnimColorTime *ACT)
{
CBCfg.ACT = ACT;
CBCfg.PotNum = ColorPotNum;
CBCfg.Flags = Flags;
CBCfg.Percent = Percent;

# ifdef _WIN32
if (Target)
	{
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		SendMessage(GF->GetWidgetFromID(Target), WM_WCSW_CB_SETUP, 0, (LPARAM)&CBCfg);
		} // if
	} // if
else
	{
	SendMessage(Dest, WM_WCSW_CB_SETUP, 0, (LPARAM)&CBCfg);
	} // else
# endif // _WIN32

} // ConfigureCB

/*===========================================================================*/

# ifdef _WIN32
long WINAPI ColorBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
RC Cola;
//long Vert;
COLORREF Old;
//COLORREF TestPal;
Fenetre *RangeInquiry = NULL;
struct ColorBarConfig *WThis;
HBRUSH NewBsh = NULL, OldBsh = NULL;
//HPEN NewPen = NULL, OldPen = NULL;

WThis = (struct ColorBarConfig *)GetWindowLong(hwnd, 0);

switch (message)
	{
	case WM_LBUTTONDOWN:
		{
		PostMessage(GetParent(hwnd), WM_COMMAND, ((UINT)GetWindowLong(hwnd, GWL_ID) & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
		break;
		} // WM_LBUTTONDOWN
	case WM_CREATE:
		{
		if (WThis = (struct ColorBarConfig *)AppMem_Alloc(sizeof(struct ColorBarConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, 0, (long)WThis);
			return(0); // hunkidori
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else
		} // CREATE
	case WM_DESTROY:
		{
		AppMem_Free(WThis, sizeof(struct ColorBarConfig));
		SetWindowLong(hwnd, 0, NULL);
		return(0);
		} // DESTROY
	case WM_WCSW_CB_NEWVAL:
		{
		WThis->Percent = (int)lParam;
		GetWindowRect(hwnd, &Square);
		// Force a repaint
		Square.right -= Square.left;
		Square.bottom -= Square.top;
		Square.left = Square.top = 0;
		Square.top++; Square.bottom--;
		Square.left++; Square.right--;
		InvalidateRect(hwnd, &Square, NULL);
		// necessary because Win2k and later seem to deprioritize these redraws, and we want it NOW
		CallWindowProc((WPROC)ColorBarWndProc, hwnd, WM_PAINT, 0, (LPARAM)0);
		break;
		} // WM_WCSW_CB_NEWVAL
	case WM_WCSW_CB_SETUP:
		{
		memcpy(WThis, (struct ColorBarConfig *)lParam, sizeof(struct ColorBarConfig));
		GetWindowRect(hwnd, &Square);
		// Force a repaint
		Square.right -= Square.left;
		Square.bottom -= Square.top;
		Square.left = Square.top = 0;
		Square.top++; Square.bottom--;
		Square.left++; Square.right--;
		InvalidateRect(hwnd, &Square, NULL);
		// necessary because Win2k and later seem to deprioritize these redraws, and we want it NOW
		CallWindowProc((WPROC)ColorBarWndProc, hwnd, WM_PAINT, 0, (LPARAM)0);
		break;
		} // SETUP
	case WM_PAINT:
		{
		if (WThis->Flags & CBFlag_CustomColor)
			{
			RangeInquiry = (Fenetre *)GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			} // if
		if (GetUpdateRect(hwnd, &Square, 0))
			{
			GetWindowRect(hwnd, &Square);
			Canvas = BeginPaint(hwnd, &PS);
			Old = GetBkColor(Canvas); // so we can set it back later, if needed
			if (RangeInquiry)
				{
				// <<<>>> I still think this sucks:
				if (NewBsh = CreateSolidBrush(WINGDI_RGB(RangeInquiry->FetchColPotRed(WThis->PotNum),
				 RangeInquiry->FetchColPotGrn(WThis->PotNum),
				 RangeInquiry->FetchColPotBlu(WThis->PotNum))))
					{
					OldBsh = (HBRUSH)SelectObject(Canvas, NewBsh);
					} // if
				} // if
			else if (WThis->ACT)
				{
				if (NewBsh = CreateSolidBrush(WINGDI_RGB(WThis->ACT->GetClampedCompleteValue(0) * 255,
				 WThis->ACT->GetClampedCompleteValue(1) * 255,
				 WThis->ACT->GetClampedCompleteValue(2) * 255)))
					{
					OldBsh = (HBRUSH)SelectObject(Canvas, NewBsh);
					} // if
				} // if
			else
				{
				Old = SetBkColor(Canvas, GetSysColor(COLOR_ACTIVECAPTION));
				} // else
			Cola.xLeft  = 0;
			Cola.yTop   = 0;
			Cola.xRight = 1 + Square.right - Square.left;
			Cola.yBot   = Square.bottom - Square.top;
			if (WThis->Percent > 100)
				WThis->Percent = 100;
			if (WThis->Percent < 0)
				WThis->Percent = 0;
			//if (Percent != 0)
				{
				Cola.xRight = (long)(((float)Cola.xRight * (float)WThis->Percent) / 100);
				if (RangeInquiry || WThis->ACT)
					{
					Rectangle(Canvas, Cola.xLeft, Cola.yTop, Cola.xRight, Cola.yBot);
					} // if
				else
					{
				    ExtTextOut(Canvas, 0, 0, ETO_OPAQUE, (LPRECT) &Cola,
    		         (char far *) NULL, 0, (int far *) NULL);
					} // else
				SetBkColor(Canvas, GetSysColor(COLOR_BTNFACE));
				Cola.xLeft = Cola.xRight;
				Cola.xRight = Square.right - Square.left;
				if (Cola.xLeft != Cola.xRight)
					{
				    ExtTextOut(Canvas, 0, 0, ETO_OPAQUE, (LPRECT) &Cola,
    		         (char far *) NULL, 0, (int far *) NULL);
					} // if
				Cola.xLeft  = 0;
				} // if
			Draw3dButtonIn(hwnd, Canvas, &Cola, 0, false, false);
			SetBkColor(Canvas, Old);
			if (OldBsh)
				{
				SelectObject(Canvas, OldBsh);
				OldBsh = NULL;
				} // if
			if (NewBsh)
				{
				if (!DeleteObject(NewBsh))
					{
					UserMessageOK("ColorPot:", "Failed to delete brush.");
					} // if
				NewBsh = NULL;
				} // if
			EndPaint(hwnd, &PS);
			Canvas = NULL;
			} // if
		Handled = 1;
		break;
		} // PAINT
	} // switch

if (Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // ColorBarWndProc

/*===========================================================================*/

long WINAPI WinCaptionButtonBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;

HWND Parent = GetParent(hwnd);

switch (message)
	{
	case WM_ACTIVATE:
		{
		if ((LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam) == WA_CLICKACTIVE))
			{
			// re-activate our parent
			SetActiveWindow(Parent);
			} // if
		else
			{
			} // else
		break;
		} // WM_ACTIVATE
	case WM_COMMAND:
		{
		WORD Notify;
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case BN_CLICKED:
				{
				// forward to our parent/owner
				LONG ID = GetWindowLong((HWND)lParam, GWL_ID);
				if (ID >= ID_THISISJUSTADUMMY)
					{ // send it looking like a popup menu event, because that's what its code says it should be handled like
					PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff), (LONG)NULL); // send like a Menu event of the proper ID
					} // if
				else
					{ // send it looking like a real button event
					if (ID == IDC_GALLERY && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
						{ // SHIFT+GALLERY = LOAD
						PostMessage(Parent, WM_COMMAND, ((UINT)IDC_LOAD & 0xffff), lParam); // send like a real button event of the proper ID, with the proper lParam
						} // if
					else
						{
						PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff), lParam); // send like a real button event of the proper ID, with the proper lParam
						} // else
					} // else
				SetActiveWindow(Parent);
				break;
				} // CLICKED
			} // switch Notify
		break;
		} // COMMAND
	} // switch

if (Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // WinCaptionButtonBarWndProc

/*===========================================================================*/

long WINAPI StatusBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;

switch (message)
	{
	case WM_SETTEXT:
		{
		Canvas = GetDC(hwnd);
		
		SelectObject(Canvas, (HGDIOBJ)(GlobalApp->WinSys->GetSystemFont()));

		GetClientRect(hwnd, &Square);
		DrawStatusText(Canvas, &Square, (LPCTSTR)lParam, NULL);
		ReleaseDC(hwnd, Canvas);
		Canvas = NULL;

		break;
		} //
	case WM_PAINT:
		{
		Canvas = BeginPaint(hwnd, &PS);

		SelectObject(Canvas, (HGDIOBJ)(GlobalApp->WinSys->GetSystemFont()));

		GetClientRect(hwnd, &Square);
		GetWindowText(hwnd, StatusBarString, 99);
		DrawStatusText(Canvas, &Square, StatusBarString, NULL);
		EndPaint(hwnd, &PS);
		Canvas = NULL;

		Handled = 1;
		break;
		} // PAINT
	} // switch

if (Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // StatusBarWndProc

/*===========================================================================*/

long WINAPI SplitBarWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0, Style;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
RC Cola;
WORD ID;
short X, Y;
HWND Parent;
char ControlState, ShiftState, AltState, LeftState, MiddleState, RightState;

Parent  = GetParent(hwnd);
ID      = (WORD) GetWindowLong(hwnd, GWL_ID);
Style   = GetWindowLong(hwnd, GWL_STYLE);

switch (message)
	{
	case WM_PAINT:
		{
		Canvas = BeginPaint(hwnd, &PS);
		GetClientRect(hwnd, &Square);

		Cola.xLeft  = 0;
		Cola.yTop   = 0;
		Cola.xRight = Square.right - Square.left;
		Cola.yBot   = Square.bottom - Square.top;
		if (GetCapture() == hwnd)
			{
			Draw3dButtonIn(hwnd, Canvas, &Cola, 0x03, true, false);
			} // if
		else
			{
			Draw3dButtonOut(hwnd, Canvas, &Cola, 0x03, true, false);
			} // else
		
		// draw gripper (what about Win2k theme?)
		HTHEME hTheme = g_xpStyle.OpenThemeData(hwnd, L"REBAR");
		RECT GripRect, ClipRect;
		int PartID = RP_GRIPPER; // RP_GRIPPERVERT
		ClipRect.top    = Cola.yTop + 1;
		ClipRect.bottom = Cola.yBot - 1;
		ClipRect.left   = Cola.xLeft + ((Cola.xRight - Cola.xLeft) / 4);
		ClipRect.right  = Cola.xRight - ((Cola.xRight - Cola.xLeft) / 4);
		GripRect.top    = Cola.yTop;
		GripRect.bottom = Cola.yBot;
		GripRect.left   = Cola.xLeft;
		GripRect.right  = Cola.xRight;
		g_xpStyle.DrawThemeBackground(hTheme, Canvas, PartID, 0, &GripRect, &ClipRect);
		g_xpStyle.CloseThemeData(hTheme);

		EndPaint(hwnd, &PS);
		Canvas = NULL;

		Handled = 1;
		break;
		} // PAINT
	case WM_LBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		{
		X = LOWORD(lParam); Y = HIWORD(lParam);
		ControlState = (wParam & MK_CONTROL ? 1 : 0);
		ShiftState   = (wParam & MK_SHIFT ? 1 : 0);
		AltState     = GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_ALT);
		LeftState    = (wParam & MK_LBUTTON ? 1 : 0);
		MiddleState  = (wParam & MK_MBUTTON ? 1 : 0);
		RightState   = (wParam & MK_RBUTTON ? 1 : 0);
		if (message == WM_LBUTTONDBLCLK)
			{
			if (GlobalApp->GUIWins->SAG)
				{
				if (Style & 0x01) // is it a resizer?
					{ // we don't appear to use any of these currently
					//GlobalApp->GUIWins->SAG->HandleResizerToggle(hwnd, Parent, ID, X, Y, AltState, ControlState, ShiftState, LeftState, MiddleState, RightState);
					} // if
				else
					{ // splitter
					GlobalApp->GUIWins->SAG->HandleSplitToggle(hwnd, Parent, ID, X, Y, AltState, ControlState, ShiftState, LeftState, MiddleState, RightState);
					} // else
				} // if
			} // if
		else
			{
			if (GetCapture() == hwnd)
				{
				// hardwired to S@G for now
				if (GlobalApp->GUIWins->SAG)
					{
					if (Style & 0x01) // is it a resizer?
						{ // we don't appear to use any of these currently
						//GlobalApp->GUIWins->SAG->HandleResizer(hwnd, Parent, ID, X, Y, AltState, ControlState, ShiftState, LeftState, MiddleState, RightState);
						} // if
					else
						{ // splitter
						GlobalApp->GUIWins->SAG->HandleSplitMove(hwnd, Parent, ID, X, Y, AltState, ControlState, ShiftState, LeftState, MiddleState, RightState);
						} // else
					} // if
				} // if
			} // else
		break;
		} // 
	case WM_LBUTTONDOWN:
		{
		SetCapture(hwnd);
		InvalidateRect(hwnd, NULL, NULL);
/*		if (!(wParam & MK_CONTROL))
			{
			if (!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
				{
				if (WStyle & WCSW_TB_STYLE_TOG)
					{
					WThis->State = !WThis->State; New = 1;
					X = LOWORD(lParam); Y = HIWORD(lParam);
					if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
		 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // if
				else
					{
					WThis->State = 1; New = 1;
					WThis->Trigger = 1;
					} // else
				} // if
			else if (WStyle & WCSW_TB_STYLE_ISTHUMB)
				{
				X = LOWORD(lParam); Y = HIWORD(lParam);
				if (TNailData.ValueValid[WCS_DIAGNOSTIC_RGB] = GlobalApp->WinSys->WL->SampleTNailColor(hwnd, (Raster *)WThis->Normal, X, Y, TNailData.RGB[0], TNailData.RGB[1], TNailData.RGB[2]))
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, &TNailData);
					} // if
				} // else
			} // if
*/
		break;
		} // LBUTTONDOWN
	case WM_LBUTTONUP:
		{
		ReleaseCapture();
		InvalidateRect(hwnd, NULL, NULL);
/*		if (!(wParam & MK_CONTROL))
			{
			if (!(WStyle & WCSW_TB_STYLE_NOCHANGE)) // inhibited
				{
				if (WStyle & WCSW_TB_STYLE_TOG)
					{
					WThis->State = !WThis->State; New = 1;
					X = LOWORD(lParam); Y = HIWORD(lParam);
					if (!(X > Square.right || X < 1 || Y > Square.bottom || Y < 1))
		 				PostMessage(Parent, WM_COMMAND, ((UINT)ID & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
					} // if
				else
					{
					WThis->State = 1; New = 1;
					WThis->Trigger = 1;
					} // else
				} // if
			else if (WStyle & WCSW_TB_STYLE_ISTHUMB)
				{
				X = LOWORD(lParam); Y = HIWORD(lParam);
				if (TNailData.ValueValid[WCS_DIAGNOSTIC_RGB] = GlobalApp->WinSys->WL->SampleTNailColor(hwnd, (Raster *)WThis->Normal, X, Y, TNailData.RGB[0], TNailData.RGB[1], TNailData.RGB[2]))
					{
					TNailChanges[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0, 0);
					TNailChanges[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(TNailChanges, &TNailData);
					} // if
				} // else
			} // if
*/
		break;
		} // LBUTTONUP
	} // switch

if (Handled)
	{
	return(0);
	} // if
else
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // else

} // SplitBarWndProc

# endif // _WIN32

/*===========================================================================*/

void ConfigureDD(NativeControl Dest, int Target, char *PBuf, int PBSize,
	char *FBuf, int FBSize, int LabelID)
{
DDCfg.Path = PBuf;
DDCfg.File = FBuf;
DDCfg.PSize = PBSize;
DDCfg.FSize = FBSize;
DDCfg.DDLabelID = LabelID;

# ifdef _WIN32
if (Target)
	{
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		SendMessage(GF->GetWidgetFromID(Target), WM_WCSW_DD_SETUP, 0, (LPARAM)&DDCfg);
		} // if
	} // if
else
	{
	SendMessage(Dest, WM_WCSW_DD_SETUP, 0, (LPARAM)&DDCfg);
	} // else
# endif // _WIN32

} // ConfigureDD

/*===========================================================================*/

# ifdef _WIN32
long WINAPI DiskDirWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD ButtonWidth, Notify, MyID;
NativeControl Parent;
LPCREATESTRUCT Create;
HINSTANCE MyInst;
HFONT TextStyle;
HDC TextDC;
RECT Square, FieldBounds;
SIZE Ruler;
struct DiskDirConfig *WThis, *DDC;
char EndTest;
long EditWidth, FieldChars;
FileReq *GetNew;
unsigned long RFlags;
LONG Style;

Parent  = GetParent(hwnd);
MyID    = (WORD)GetWindowLong(hwnd, GWL_ID);
WThis   = (struct DiskDirConfig *)GetWindowLong(hwnd, 0);
Style   = GetWindowLong(hwnd, GWL_STYLE);

switch (message)
	{
	// CreateWindow
	case WM_PAINT:
		{
		if (Style & WS_DISABLED)
			{
			EnableWindow(WThis->Text, false);
			} // if
		else
			{
			EnableWindow(WThis->Text, true);
			} // else
		break;
		} // PAINT
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		{
		return(CallWindowProc((WPROC)GetWindowLong(Parent, GWL_WNDPROC), Parent,
		 message, wParam, (LPARAM)hwnd));
		} // WM_CTLCOLORn
	case WM_SETFONT:
		{
		TextStyle = (HFONT)wParam;
		TextDC = GetDC(hwnd);
		SelectObject(TextDC, TextStyle);
		GetWindowRect(hwnd, &Square);
		EditWidth = Square.right - Square.left;

		for (FieldChars = 0; FieldChars < 98; FieldChars++)
			{
			Scratch[FieldChars] = 'n';
			Scratch[FieldChars + 1] = NULL;

			GetTextExtentPoint32(TextDC, Scratch, FieldChars + 1, &Ruler);
			if (Ruler.cx > EditWidth)
				{
				break;
				} // if
			} // for
		WThis->FWidth= FieldChars;
		WThis->Font = (HFONT)wParam;
		ReleaseDC(hwnd, TextDC);
		TextDC = NULL;

		// Tell the children what font to use
		SendMessage(WThis->Text, WM_SETFONT, wParam, 0);
		// Center textfield vertically within widget bounds,
		// in case it is not full-height, a la Mac.
		GetWindowRect(WThis->Text, &FieldBounds);
		// Misuse EditWidth 'cause it's handy...
		EditWidth = ((Square.bottom - Square.top) - (FieldBounds.bottom - FieldBounds.top)) / 2;
		SetWindowPos(WThis->Text, NULL, 0, EditWidth, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
		break;
		} // SETFONT
	case WM_DESTROY:
		{
		AppMem_Free(WThis, sizeof(struct DiskDirConfig));
		SetWindowLong(hwnd, 0, NULL);
		return(0);
		} // DESTROY
	case WM_CREATE:
		{
		if (WThis = (struct DiskDirConfig *)AppMem_Alloc(sizeof(struct DiskDirConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, 0, (long)WThis);
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else
		MyInst = (HINSTANCE)GetClassLong(hwnd, GCL_HMODULE);
		Create = (LPCREATESTRUCT)lParam;

		ButtonWidth = 20;

		// Create Disk Icon
		WThis->Disk = CreateWindow(APP_CLASSPREFIX ".ToolButton", "Opens a File Requester", WS_VISIBLE | WS_CHILD | WCSW_TB_STYLE_XPLOOK,
		 Create->cx - ButtonWidth, 0, ButtonWidth, Create->cy, hwnd, NULL, MyInst, NULL);
		if (WThis->Text = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		 0, 0, Create->cx - ButtonWidth, Create->cy - 1, hwnd, NULL, MyInst, NULL))
			{
			SetWindowLong(WThis->Text, GWL_WNDPROC, (long)ModifiedEditWndProc);
			} // if 

		WThis->Frame = NULL;

		if (WThis->Text && WThis->Disk)
			{
			ConfigureTB(WThis->Disk, NULL, IDI_CHOOSEFILE, NULL);
			ShowWindow(WThis->Disk, SW_SHOW);
			ShowWindow(WThis->Text, SW_SHOW);
			return(0);
			} // if
		else
			{
			if (WThis->Disk) DestroyWindow(WThis->Disk);
			if (WThis->Text) DestroyWindow(WThis->Text);
			AppMem_Free(WThis, sizeof(struct DiskDirConfig));
			return(-1);
			} // else
		} // CREATE
	case WM_COMMAND:
		{
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case EN_SETFOCUS:
				{
				Scratch[0] = NULL;
				if (WThis->Path)
					{
					if (WThis->Path[0])
						{
						strcpy(Scratch, WThis->Path);
						if (WThis->File)
							{
							EndTest = Scratch[strlen(Scratch) - 1];
							if (!((EndTest == ':') || (EndTest == '/') || (EndTest == '\\')))
								{
								strcat(Scratch, "\\");
								} // if
							} // if
						} // if
					} // if
				if (WThis->File)
					{
					strcat(Scratch, WThis->File);
					} // if
				SetWindowText(WThis->Text, Scratch);
				SendMessage(WThis->Text, EM_SETMODIFY, 0, 0);

				return(0);
				} // SETFOCUS
			case EN_KILLFOCUS:
				{
				if (SendMessage(WThis->Text, EM_GETMODIFY, 0, 0))
					{
					SendMessage(WThis->Text, WM_GETTEXT, WCSW_SCRATCHSIZE, (long)Scratch);
					if (WThis->File && WThis->Path)
						{
						BreakFileName(Scratch, WThis->Path, WThis->PSize, WThis->File, WThis->FSize);
						} // if
					else
						{
						if (WThis->Path)
							{
							strncpy(WThis->Path, Scratch, WThis->PSize);
							} // if
						if (WThis->File)
							{
							strncpy(WThis->File, Scratch, WThis->FSize);
							} // if
						} // if
					} // if
				DDmSync(WThis->Text, WThis->FWidth, WThis->Path, WThis->File);
				SendMessage(WThis->Text, EM_SETMODIFY, 0, 0);
				SendMessage(Parent, WM_WCSW_DD_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
				return(0);
				} // KILLFOCUS
			case BN_CLICKED:
				{
				RFlags = NULL;
				if (WThis->Disk == (NativeControl)lParam)
					{
					if (GetNew = new FileReq)
						{
						GetNew->SetDefPath(WThis->Path);
						GetNew->SetDefFile(WThis->File);
						if (WThis->DDLabelID)
							{
							LabelName[0] = NULL;
							SendDlgItemMessage(Parent, WThis->DDLabelID, WM_GETTEXT, (WPARAM)30, (LPARAM)LabelName);
							if (LabelName[0])
								{
								GetNew->SetTitle(LabelName);
								} // if
							} // if
						if (GetWindowLong(hwnd, GWL_STYLE) & WCSW_DD_STYLE_SAVETYPE)
							{
							RFlags |= WCS_REQUESTER_FILE_SAVE;
							} // if
						if (GetWindowLong(hwnd, GWL_STYLE) & WCSW_DD_STYLE_NOMUNGE)
							{
							RFlags |= WCS_REQUESTER_FILE_NOMUNGE;
							} // if
						if (GetWindowLong(hwnd, GWL_STYLE) & WCSW_DD_STYLE_DIRONLY)
							{
							RFlags |= WCS_REQUESTER_FILE_DIRONLY;
							} // if
						if (GetNew->Request(RFlags))
							{
							if (WThis->File)
								{
								BreakFileName((char *)GetNew->GetFirstName(), WThis->Path, WThis->PSize, WThis->File, WThis->FSize);
								} // if
							else
								{
								BreakFileName((char *)GetNew->GetFirstName(), WThis->Path, WThis->PSize, Scratch, 100);
								} // else
							SendMessage(Parent, WM_WCSW_DD_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
							DDmSync(WThis->Text, WThis->FWidth, WThis->Path, WThis->File);
							} // if
						delete GetNew;
						GetNew = NULL;
						} // if
					return(0);
					} // if
				return(0);
				} // CLICKED
			} // switch
		break;
		} // WM_COMMAND
	case WM_SETFOCUS:
		{
		SetFocus(WThis->Text);
		return(0);
		} // WM_SETFOCUS
	case WM_WCSW_DD_SYNC:
		{
		DDmSync(WThis->Text, WThis->FWidth, WThis->Path, WThis->File);
		if (!(wParam & WP_DDSYNC_NONOTIFY))
			{
			SendMessage(Parent, WM_WCSW_DD_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // SYNC
	case WM_WCSW_DD_SETUP:
		{
		DDC = (struct DiskDirConfig *)lParam;
		WThis->Path = DDC->Path;
		WThis->PSize = DDC->PSize;
		WThis->File = DDC->File;
		WThis->FSize = DDC->FSize;
		WThis->DDLabelID = DDC->DDLabelID;
		DDmSync(WThis->Text, WThis->FWidth, WThis->Path, WThis->File);
		return(0);
		} // SETUP
	} // switch

return(DefWindowProc(hwnd, message, wParam, lParam));

} // DiskDirWndProc
# endif // _WIN32

/*===========================================================================*/

void DDmSync(NativeControl EditDude, int Width, char *Path, char *File)
{
unsigned int Partial, PLen;

if (Path && Width && EditDude)
	{
	PLen = (unsigned int)strlen(Path);
	if (File)
		{
		// Eliminate possible illegal characters in filename getopenfilename
		for (Partial = 0; File[Partial]; Partial++)
			{
			if (File[Partial] == ':') File[Partial] = NULL;
			} // if
		if (strlen(File) + 1 + PLen < (unsigned int)Width)
			{
			strcpy(Scratch, Path);
			if (!((Path[PLen - 1] == ':') || (Path[PLen - 1] == '\\') || (Path[PLen - 1] == '/')))
				{
				strcat(Scratch, "\\");
				} // if
			strcat(Scratch, File);
			} // if
		else
			{
			Partial = (Width - 3) / 2;
			// Limited buffer size...
			if (Partial > 48) Partial = 48;

			if (Partial > 5)
				{
				strncpy(Scratch, Path, Partial);
				Scratch[Partial] = Scratch[48] = NULL;
				strcat(Scratch, "...");
				if (strlen(File) < Partial)
					{
					strcat(Scratch, File);
					} // if
				else
					{
					strncat(Scratch, &File[strlen(File) - Partial], Partial);
					} // else
				} // if
			} // else
		} // if
	else
		{
		strncpy(Scratch, Path, Width);
		Scratch[Width] = NULL;
		} // else

#	ifdef _WIN32
	SetWindowText(EditDude, Scratch);
#	endif // _WIN32
	} // if

} // DDmSync

// ************************** SmartCheckCode ****************************

void ConfigureSC(NativeControl Dest, int Target, void *MV, unsigned long Flags,
 SetCritter *Crit, unsigned long SetGetID)
{
NativeControl Widget = NULL;

SCCfg.MasterVariable = MV;
SCCfg.Crit = Crit;
SCCfg.CritID = SetGetID;
SCCfg.SCFlags = Flags;

# ifdef _WIN32

if (Target)
	{
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		Widget = GF->GetWidgetFromID(Target);
		} // if
	} // if
else
	{
	Widget = Dest;
	} // else

if (Widget)
	{
	// Don't know why we're calling the WndProc directly.
	SmartCheckWndProc(Widget, WM_WCSW_SC_SETUP, 0, (LPARAM)&SCCfg);
	} // if
/*if (Widget = GetDlgItem(Dest, Target))
	{
	SmartCheckWndProc(Widget, WM_WCSW_SC_SETUP, 0, (LPARAM)&SCCfg);
	} // else
*/
//SendDlgItemMessage(Dest, Target, WM_WCSW_SC_SETUP, 0, (LPARAM)&SCCfg);
# endif // _WIN32

} // ConfigureSC()

/*****************************************************************************
 * Dispatcher for the smartcheck class                                       *
 *****************************************************************************/

# ifdef _WIN32
long WINAPI SmartCheckWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD MyID;
NativeControl Parent;
struct SmartCheckConfig *WThis, *SCC;
void *MasterVariable = NULL;
unsigned long NuState, OldState;

Parent  = GetParent(hwnd);
MyID    = (WORD)GetWindowLong(hwnd, GWL_ID);
WThis	= (struct SmartCheckConfig *)GetWindowLong(hwnd, ButtonWndExtra);

if (WThis)
	{
	if (!(WThis->Crit && WThis->CritID))
		{
		MasterVariable = (void *)WThis->MasterVariable;
		} // if
	} // if

switch (message)
	{
	case WM_CREATE:
		{
		if (WThis = (struct SmartCheckConfig *)AppMem_Alloc(sizeof(struct SmartCheckConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, ButtonWndExtra, (long)WThis);
			// Must pass along to superclass, so don't return, let
			// ButtonWndProc do it's thing...
			//return(0); // hunkidori
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else
		break;
		} // CREATE
	case WM_DESTROY:
		{
		AppMem_Free(WThis, sizeof(struct SmartCheckConfig));
		SetWindowLong(hwnd, ButtonWndExtra, 0);
		return(0);
		} // DESTROY
	case WM_LBUTTONUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
		// Determine old check state
		OldState = CallWindowProc((WPROC)ButtonWndProc, hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
		// Forward to real checkbox
		CallWindowProc((WPROC)ButtonWndProc, hwnd, message, wParam, lParam);
		// Find out new check state
		NuState = CallWindowProc((WPROC)ButtonWndProc, hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
		if (OldState != NuState)
			{
			// Set our variable/invoke SetCritter notifies
			SCSetCheck(hwnd, MasterVariable, WThis->SCFlags, WThis->Crit, WThis->CritID, (char)NuState);
			// Notify our parent window
			SendMessage(Parent, WM_WCSW_SC_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // BUTTONUP
	case WM_WCSW_SC_SYNC:
		{
		SCmSync(hwnd, MasterVariable, WThis->SCFlags, WThis->Crit, WThis->CritID);
		if (!(wParam & WP_SCSYNC_NONOTIFY))
			{
			SendMessage(Parent, WM_WCSW_SC_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // SYNC
	case WM_WCSW_SC_SETUP:
		{
		SCC = (struct SmartCheckConfig *)lParam;
		WThis->MasterVariable = MasterVariable = SCC->MasterVariable;
		WThis->SCFlags = SCC->SCFlags;
		WThis->Crit = SCC->Crit;
		WThis->CritID = SCC->CritID;
		SCmSync(hwnd, MasterVariable, WThis->SCFlags, WThis->Crit, WThis->CritID);
		if ((!WThis->MasterVariable) && (!WThis->Crit) && (!WThis->CritID))
			{ // disable
			EnableWindow(hwnd, 0);
			} // if
		else
			{ // enable
			EnableWindow(hwnd, 1);
			} // else
		return(0);
		} // SETUP
	} // switch

return(CallWindowProc((WPROC)ButtonWndProc, hwnd, message, wParam, lParam));
} // SmartCheckWndProc
# endif // _WIN32

void SCGetCheck(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, char *State)
{
char CurState = 0;
long  LState;
short SState;
char  CState;
char  *CT;
short *ST;
long  *LT;

if (MyCrit && CritID)
	{
	if (Flags & SCFlag_Char)
		{
		MyCrit->GetParam(&CState, CritID);
		CurState = (char)CState;
		} // if
	else if (Flags & SCFlag_Short)
		{
		MyCrit->GetParam(&SState, CritID);

		CurState = (char)SState;
		} // if
	else if (Flags & SCFlag_Long)
		{
		MyCrit->GetParam(&LState, CritID);
		CurState = (char)LState;
		} // if
	} // if
else if (MasterVariable)
	{
	// Visual raises an exception in this code in debug builds (stored in smaller size)
	if (Flags & SCFlag_Char)
		{
		CT = (char *)MasterVariable;
		CurState = *CT;
		} // if
	else if (Flags & SCFlag_Short)
		{
		ST = (short *)MasterVariable;
		CurState = (char)*ST;
		} // if
	else if (Flags & SCFlag_Long)
		{
		LT = (long *)MasterVariable;
		CurState = (char)*LT;
		} // if
	} // else

if (State)
	{
	*State = CurState;
	} // if
} // SCGetCheck

void SCSetCheck(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, char State)
{
char  *CT;
short *ST;

long  *LT;
short SState;
long  LState;

if (MyCrit && CritID)
	{
	if (Flags & SCFlag_Char)
		{
		MyCrit->SetParam(1, CritID, State, 0);
		} // if
	else if (Flags & SCFlag_Short)
		{
		SState = State;
		MyCrit->SetParam(1, CritID, SState, 0);
		} // if
	else if (Flags & SCFlag_Long)
		{
		LState = State;
		MyCrit->SetParam(1, CritID, LState, 0);
		} // if
	} // if
else if (MasterVariable)
	{
	if (Flags & SCFlag_Char)
		{
		CT = (char *)MasterVariable;
		*CT = State;
		} // if
	else if (Flags & SCFlag_Short)
		{
		ST = (short *)MasterVariable;
		*ST = State;
		} // if
	else if (Flags & SCFlag_Long)
		{
		LT = (long *)MasterVariable;
		*LT = State;
		} // if
	} // else

} // SCSetCheck

void SCmSync(NativeControl Check, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID)
{
char NuState;
//unsigned long Style;

if (!MyCrit && !CritID && !MasterVariable)
	{
	return;
	} // if

SCGetCheck(Check, MasterVariable, Flags, MyCrit, CritID, &NuState);

# ifdef _WIN32
SendMessage(Check, BM_SETCHECK, (WPARAM)(int)NuState, (LPARAM)0);
# endif // _WIN32

} // SCmSync


// ************************** SmartRadioCode ****************************

void ConfigureSR(NativeControl Dest, int MasterTarget, int Target, void *MV, unsigned long Flags, signed long SetVal,
 SetCritter *Crit, unsigned long SetGetID)
{
// MasterTarget is not used in Win32 implementation, but is used to
// specify mutex-grouping under other OSes.

NativeControl Widget = NULL;

SRCfg.MasterVariable = MV;
SRCfg.Crit = Crit;
SRCfg.CritID = SetGetID;
SRCfg.RadioSetVal = SetVal;
SRCfg.SRFlags = Flags;

# ifdef _WIN32
if (Target)
	{
	GUIFenetre *GF;
	if (GF = (GUIFenetre *)GetWindowLong(Dest, GWL_USERDATA))
		{
		Widget = GF->GetWidgetFromID(Target);
		} // if
	} // if
else
	{
	Widget = Dest;
	} // else

if (Widget)
	{
	// Don't know why we're calling the WndProc directly.
	SmartRadioWndProc(Widget, WM_WCSW_SR_SETUP, 0, (LPARAM)&SRCfg);
	} // if

//SendDlgItemMessage(Dest, Target, WM_WCSW_SR_SETUP, 0, (LPARAM)&SRCfg);
# endif // _WIN32

} // ConfigureSR()

# ifdef _WIN32
long WINAPI SmartRadioWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD MyID;
NativeControl Parent, Step;
WNDPROC MyWP;
struct SmartRadioConfig *WThis, *SRC;
void *MasterVariable = NULL;
unsigned long NuState, OldState;



Parent  = GetParent(hwnd);
MyID    = (WORD)GetWindowLong(hwnd, GWL_ID);
WThis   = (struct SmartRadioConfig *)GetWindowLong(hwnd, ButtonWndExtra);


if (WThis)
	{
	if (!(WThis->Crit && WThis->CritID))
		{
		MasterVariable = (void *)WThis->MasterVariable;
		} // if
	} // if


switch (message)
	{
	case WM_CREATE:
		{
		if (WThis = (struct SmartRadioConfig *)AppMem_Alloc(sizeof(struct SmartRadioConfig), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, ButtonWndExtra, (long)WThis);
			// Must pass along to superclass, so don't return, let
			// ButtonWndProc do it's thing...
			//return(0); // hunkidori
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else
		break;
		} // CREATE
	case WM_DESTROY:
		{
		AppMem_Free(WThis, sizeof(struct SmartRadioConfig));
		SetWindowLong(hwnd, ButtonWndExtra, 0);
		return(0);
		} // DESTROY
	case WM_LBUTTONUP:
		{
		OldState = CallWindowProc((WPROC)ButtonWndProc, hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
		// Forward to real widget
		CallWindowProc((WPROC)ButtonWndProc, hwnd, message, wParam, lParam);
		// Find out new state
		NuState = CallWindowProc((WPROC)ButtonWndProc, hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
		// Set our variable/invoke SetCritter notifies
		if (NuState && !OldState) // prevent re-firing of event when we were already the selected radiobutton
			{
			SRSetState(hwnd, MasterVariable, WThis->SRFlags, WThis->Crit, WThis->CritID, WThis->RadioSetVal);
			// loop forward and backward, un-checking our brethern
			MyWP = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
			for (Step = GetNextWindow(hwnd, GW_HWNDPREV); Step; Step = GetNextWindow(Step, GW_HWNDPREV))
				{
				if (MyWP == (WNDPROC)GetWindowLong(Step, GWL_WNDPROC))
					{
					SendMessage(Step, BM_SETCHECK, 0, 0);
					} // if
				else
					{
					break;
					} // else
				} // for
			for (Step = GetNextWindow(hwnd, GW_HWNDNEXT); Step; Step = GetNextWindow(Step, GW_HWNDNEXT))
				{
				if (MyWP == (WNDPROC)GetWindowLong(Step, GWL_WNDPROC))
					{
					SendMessage(Step, BM_SETCHECK, 0, 0);
					} // if
				else
					{
					break;
					} // else
				} // for
			// Notify our parent window
			SendMessage(Parent, WM_WCSW_SR_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // WM_COMMAND
	case WM_WCSW_SR_SYNC:
		{
		SRmSync(hwnd, MasterVariable, WThis->SRFlags, WThis->Crit, WThis->CritID, WThis->RadioSetVal);
		if (!(wParam & WP_SRSYNC_NONOTIFY))
			{
			SendMessage(Parent, WM_WCSW_SR_CHANGE, (WPARAM)(MyID), (LPARAM)hwnd);
			} // if
		return(0);
		} // SYNC
	case WM_WCSW_SR_SETUP:
		{
		SRC = (struct SmartRadioConfig *)lParam;
		WThis->MasterVariable = MasterVariable = SRC->MasterVariable;
		WThis->SRFlags = SRC->SRFlags;
		WThis->RadioSetVal = SRC->RadioSetVal;
		WThis->Crit = SRC->Crit;
		WThis->CritID = SRC->CritID;
		SRmSync(hwnd, MasterVariable, WThis->SRFlags, WThis->Crit, WThis->CritID, WThis->RadioSetVal);
		if ((!WThis->MasterVariable) && (!WThis->Crit) && (!WThis->CritID))
			{ // disable
			EnableWindow(hwnd, 0);
			} // if
		else
			{ // enable
			EnableWindow(hwnd, 1);
			} // else
		return(0);
		} // SETUP

	} // switch

return(CallWindowProc((WPROC)ButtonWndProc, hwnd, message, wParam, lParam));

} // SmartRadioWndProc
# endif // _WIN32


void SRGetState(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long *State)
{
long  LState = 0;
short SState = 0;
char  CState = 0;
char  *CT;
short *ST;
long  *LT;

if (MyCrit && CritID)
	{
	if (Flags & SCFlag_Char)
		{
		MyCrit->GetParam(&CState, CritID);
		LState = (long)CState;
		} // if
	else if (Flags & SCFlag_Short)
		{
		MyCrit->GetParam(&SState, CritID);
		LState = (long)SState;
		} // if
	else if (Flags & SCFlag_Long)
		{

		MyCrit->GetParam(&LState, CritID);
		} // if
	} // if
else if (MasterVariable)
	{
	if (Flags & SCFlag_Char)
		{
		CT = (char *)MasterVariable;
		LState = *CT;
		} // if
	else if (Flags & SCFlag_Short)
		{
		ST = (short *)MasterVariable;
		LState = *ST;
		} // if
	else if (Flags & SCFlag_Long)
		{
		LT = (long *)MasterVariable;
		LState = *LT;
		} // if
	} // else

if (State)
	{
	*State = LState;
	} // if
} // SRGetState

void SRSetState(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long State)
{
char  *CT;
short *ST;
long  *LT;
char CState;
short SState;
long  LState;

if (MyCrit && CritID)
	{
	if (Flags & SCFlag_Char)
		{
		CState = (char)State;
		MyCrit->SetParam(1, CritID, CState, 0);
		} // if
	else if (Flags & SCFlag_Short)
		{
		SState = (short)State;
		MyCrit->SetParam(1, CritID, SState, 0);
		} // if
	else if (Flags & SCFlag_Long)
		{
		LState = State;
		MyCrit->SetParam(1, CritID, LState, 0);
		} // if
	} // if
else if (MasterVariable)
	{
	if (Flags & SCFlag_Char)
		{
		CT = (char *)MasterVariable;
		*CT = (char)State;
		} // if
	else if (Flags & SCFlag_Short)
		{
		ST = (short *)MasterVariable;
		*ST = (short)State;
		} // if
	else if (Flags & SCFlag_Long)
		{
		LT = (long *)MasterVariable;
		*LT = State;
		} // if
	} // else

} // SRSetState

void SRmSync(NativeControl Radio, void *MasterVariable, unsigned long Flags, SetCritter *MyCrit, NotifyTag CritID, long RadioVal)
{
long NuState;
//unsigned long Style;

if (!MyCrit && !CritID && !MasterVariable)
	{
	return;
	} // if

SRGetState(Radio, MasterVariable, Flags, MyCrit, CritID, &NuState);

# ifdef _WIN32
SendMessage(Radio, BM_SETCHECK, (WPARAM)(NuState == RadioVal), (LPARAM)0);
# endif // _WIN32

} // SRmSync


# ifdef _WIN32
long WINAPI AnimGradientWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD MyID;
NativeControl Parent;
HDC Canvas;
RECT Square;
char LabelRight = 0, DState = 0;
int GradWidth, WidgetWidth, GradLoop, NodePos, NodeWide, InNode, NeedleTop, MouseX, MouseY;
AnimGradient *AG;
LONG Style;
PAINTSTRUCT PS;
//LPDRAWITEMSTRUCT lpdis;
COLORREF Old;
RC Cola, ButtonCola;
unsigned char GradRed, GradGrn, GradBlu, DoRedraw = 0;
double GradPos, KnobFrac;
GradientCritter *GradNode, *ActNode;

Parent		= GetParent(hwnd);
MyID		= (WORD)GetWindowLong(hwnd, GWL_ID);
AG			= (AnimGradient *)GetWindowLong(hwnd, 0);
Style		= GetWindowLong(hwnd, GWL_STYLE);


switch (message)
	{
	case WM_CREATE:
		{
		return(0); // Hunkidori.
		} // WM_CREATE
	case WM_DESTROY:
		{
		SetWindowLong(hwnd, 0, 0);
		return(0);
		} // DESTROY
	case WM_WCSW_AG_SETUP:
		{
		SetWindowLong(hwnd, 0, lParam);
		AG = (AnimGradient *)lParam;
		// fall through to resync
		} // SETUP
		//lint -fallthrough
	case WM_WCSW_AG_SYNC:
		{
		// resync
		// just repaint (below)
		} // SYNC
		//lint -fallthrough
	case WM_PAINT:
		{
		DoRedraw = 0;
		if ((message == WM_PAINT) && (GetUpdateRect(hwnd, &Square, 0)))
			{
			Canvas = BeginPaint(hwnd, &PS);
			DoRedraw = 1;
			} // if
		else
			{
			Canvas = GetDC(hwnd);
			DoRedraw = 1;
			} // else

		if (DoRedraw)
			{
			GetClientRect(hwnd, &Square);
			
			// Widget bounds
			Cola.xLeft  = Square.left;
			Cola.yTop   = Square.top;
			Cola.xRight = Square.right;
			Cola.yBot   = Square.bottom;

			// Widget Outline
			Draw3dButtonIn(hwnd, Canvas, &Cola, 0, false, false);

			// Nudge borders in a smidgen
			Square.left++;  Square.top++;
			Square.right--; Square.bottom--;

			// Seems we need to erase bottom
			Cola.yTop   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
			Cola.yBot   = Square.bottom;
			Cola.xLeft  = Square.left;
			Cola.xRight = Square.right;
			Old = SetBkColor(Canvas, GetSysColor(COLOR_3DFACE));
			ExtTextOut(Canvas, 0, 0, ETO_OPAQUE, (LPRECT) &Cola,
    		 (char far *) NULL, 0, (int far *) NULL);
			SetBkColor(Canvas, Old);

			if (AG)
				{
				int Middle, Inside, Outside;
				WidgetWidth = Square.right - Square.left;
				GradWidth   = WidgetWidth - (WCS_WIDGET_GRADIENT_SIDE_BORDER + WCS_WIDGET_GRADIENT_SIDE_BORDER);

				Cola.yTop   = Square.top;
				Cola.xLeft  = Square.left + WCS_WIDGET_GRADIENT_SIDE_BORDER;
				Cola.xRight = Square.right - WCS_WIDGET_GRADIENT_SIDE_BORDER;


				// Gradient Bar
				Cola.yBot   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
				NeedleTop	= Cola.yBot - WCS_WIDGET_GRADIENT_NEEDLE_HEIGHT;

				// Draw prevnode button
				ButtonCola.xLeft  = Square.left;
				ButtonCola.xRight = Square.left + WCS_WIDGET_GRADIENT_SIDE_BORDER;
				ButtonCola.yTop   = Cola.yTop;
				ButtonCola.yBot   = Cola.yBot;
				Middle			  = (ButtonCola.yBot + ButtonCola.yTop) / 2;
				Inside			  = ButtonCola.xRight - 1;
				Outside			  = ButtonCola.xLeft  + 1;
				Draw3dButtonOut(hwnd, Canvas, &ButtonCola, 0, false, false);
				WCSW_DotLine(Canvas, Outside, Middle,     Inside, Middle - 3);
				WCSW_DotLine(Canvas, Outside, Middle,     Inside, Middle + 3);

				// Draw nextnode button
				ButtonCola.xLeft  = Square.right - WCS_WIDGET_GRADIENT_SIDE_BORDER;
				ButtonCola.xRight = Square.right;
				ButtonCola.yTop   = Cola.yTop;
				ButtonCola.yBot   = Cola.yBot;
				Outside			  = ButtonCola.xRight - 1;
				Inside			  = ButtonCola.xLeft  + 1;
				Draw3dButtonOut(hwnd, Canvas, &ButtonCola, 0, false, false);
				WCSW_DotLine(Canvas, Outside, Middle,     Inside, Middle - 3);
				WCSW_DotLine(Canvas, Outside, Middle,     Inside, Middle + 3);


				if (GradWidth > 0)
					{
					for (GradLoop = 0; GradLoop < GradWidth; GradLoop++)
						{
						GradPos = (double)GradLoop / (double)GradWidth;
						AG->GetBasicColor(GradRed, GradGrn, GradBlu, GradPos);
						ThumbNailR[GradLoop] = GradRed;
						ThumbNailG[GradLoop] = GradGrn;
						ThumbNailB[GradLoop] = GradBlu;
						} // for
					// Tile gradient pixelstrip vertically to fill band
					for (GradLoop = Cola.yTop; GradLoop < Cola.yBot; GradLoop++)
						{
						RastBlast(Canvas, Cola.xLeft, GradLoop, GradWidth,
						 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
						} // for

					// Node strip
					Cola.yTop   = Cola.yBot + WCS_WIDGET_GRADIENT_MID_BORDER;
					Cola.yBot   = Square.bottom;

					ButtonCola.yTop   = Cola.yTop;
					ButtonCola.yBot   = Cola.yBot;
					ActNode = AG->GetActiveNode();
					for (GradNode = NULL; GradNode = AG->GetNextNode(GradNode);)
						{
						NodePos = Cola.xLeft + (int)(AG->GetNodePosition(GradNode) * GradWidth);
						// Draw Node needle
						WCSW_SetWidgetDrawColor(Canvas, WCS_WIDGET_COLOR_BLACK);
						WCSW_DotLine(Canvas, NodePos, NeedleTop, NodePos, ButtonCola.yTop + 2);
						if (GradNode == ActNode)
							{
							// Draw Triangle on Active Node Needle
							for (int TriX = NodePos - (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 2); TriX <= NodePos + (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 2); TriX++)
								{
								WCSW_DotLine(Canvas, TriX, ButtonCola.yTop - 1, NodePos, Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT);
								} // for
							/* 
							WCSW_DotLine(Canvas, NodePos - (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 1), ButtonCola.yTop - 1, NodePos, Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT);
							WCSW_DotLine(Canvas, NodePos + (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 1), ButtonCola.yTop - 1, NodePos, Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT);
							WCSW_DotLine(Canvas, NodePos - (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 1), ButtonCola.yTop - 1, NodePos + (WCS_WIDGET_GRADIENT_NODE_HALFWIDTH - 1), ButtonCola.yTop - 1);
							*/
							} // id

						// Draw Node swatch
						AG->GetNodeColor(GradRed, GradGrn, GradBlu, GradNode);
						for (InNode = 0, NodeWide = NodePos - WCS_WIDGET_GRADIENT_NODE_HALFWIDTH; NodeWide <= NodePos + WCS_WIDGET_GRADIENT_NODE_HALFWIDTH; NodeWide++, InNode++)
							{
							ThumbNailR[InNode] = GradRed;
							ThumbNailG[InNode] = GradGrn;
							ThumbNailB[InNode] = GradBlu;
							} // for

						// Tile node pixelstrip vertically to fill band
						// Not the most efficient way...
						for (GradLoop = Cola.yTop; GradLoop < Cola.yBot; GradLoop++)
							{
							RastBlast(Canvas, NodePos - WCS_WIDGET_GRADIENT_NODE_HALFWIDTH, GradLoop, WCS_WIDGET_GRADIENT_NODE_WIDTH,
							 (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
							} // for
						// Draw node decorations
						ButtonCola.xLeft  = NodePos - WCS_WIDGET_GRADIENT_NODE_HALFWIDTH;
						ButtonCola.xRight = NodePos + WCS_WIDGET_GRADIENT_NODE_HALFWIDTH;
						Draw3dButtonOut(hwnd, Canvas, &ButtonCola, 0, false, false);
						if (AG->GetSpecialDataExists(GradNode))
							{
							WCSW_BlackOut(Canvas, NodePos - 1, ButtonCola.yTop + 3, NodePos + 1, ButtonCola.yBot - 3);
							} // if
						} // for
					} // if
				} // if configured
			} // if

		if (Canvas)
			{
			if (message == WM_PAINT)
				{
				EndPaint(hwnd, &PS); //lint !e645
				} // if
			else
				{
				ReleaseDC(hwnd, Canvas);
				} // else
			Canvas = NULL;
			} // if

		return(0);
		} // PAINT
	case WM_LBUTTONUP:
		{
		ReleaseCapture();
		SetFocus(hwnd);
		break;
		} // LBUTTONUP
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
		{
		SetFocus(hwnd);
		if (AG)
			{
			MouseX = LOWORD(lParam);
			MouseY = HIWORD(lParam);
			GetClientRect(hwnd, &Square);
			Square.left++;  Square.top++;
			Square.right--; Square.bottom--;
			WidgetWidth = Square.right - Square.left;
			// Cola will hold gradient bar bounds
			Cola.yTop   = Square.top;
			Cola.yBot   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
			Cola.xLeft  = Square.left   + WCS_WIDGET_GRADIENT_SIDE_BORDER;
			Cola.xRight = Square.right  - WCS_WIDGET_GRADIENT_SIDE_BORDER;
			// ButtonCola will hold node strip bounds
			ButtonCola.yTop   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
			ButtonCola.yBot   = Square.bottom;
			ButtonCola.xLeft  = Square.left   + WCS_WIDGET_GRADIENT_SIDE_BORDER;
			ButtonCola.xRight = Square.right  - WCS_WIDGET_GRADIENT_SIDE_BORDER;

			GradWidth   = Cola.xRight - Cola.xLeft;
			KnobFrac = ((double)WCS_WIDGET_GRADIENT_NODE_HALFWIDTH) / (double)GradWidth;
			if ((MouseX >= Cola.xLeft) && (MouseX <= Cola.xRight))
				{ // in main gradient region
				GradPos = ((double)(MouseX - Cola.xLeft)) / (double)GradWidth;
				if (MouseY < Cola.yBot)
					{ //in gradient bar
					if (GradNode = AG->FindNode(GradPos, 3.0 / (double)GradWidth))
						{
						if (message == WM_LBUTTONDOWN)
							{
							AG->SetActiveNode(GradNode);
							InvalidateRect(hwnd, NULL, TRUE);
							} // if
						} // if
					else
						{
						if (message == WM_LBUTTONDOWN)
							{
							//ControlState = (wParam & MK_CONTROL ? 1 : 0);
							AG->SetActiveNode(AG->AddNodeNotify(GradPos, (wParam & MK_CONTROL ? 1 : 0)));
							InvalidateRect(hwnd, NULL, TRUE);
							} // if
						} // else
					} // if
				else
					{ // in Node strip
					// 0.1 should be calculated...?
					if (GradNode = AG->FindNode(GradPos, KnobFrac))
						{
						if (GradNode != AG->GetActiveNode())
							{
							if (message == WM_LBUTTONDOWN)
								{
								AG->SetActiveNode(GradNode);
								} // if
							} // if
						else
							{
							if (message == WM_LBUTTONDBLCLK)
								{
								AG->EditNodeColor(GradNode);
								} // if
							else
								{
								SetCapture(hwnd);
								} // else
							} // else
						InvalidateRect(hwnd, NULL, TRUE);
						} // if
					} // else
				} // if
			else if (MouseY > ButtonCola.yTop)
				{ // Might be on node in nodestrip border area
				if (MouseX <= WCS_WIDGET_GRADIENT_SIDE_BORDER) // is it first node?
					{
					GradNode = AG->GetNextNode(NULL);
					if (GradNode != AG->GetActiveNode())
						{
						if (message == WM_LBUTTONDOWN)
							{
							AG->SetActiveNode(GradNode);
							} // if
						} // if
					else
						{
						SetCapture(hwnd);
						} // else
					InvalidateRect(hwnd, NULL, TRUE);
					} // if
				else if (MouseX >= ButtonCola.xRight) // is it last node?
					{
					// Dunno if this trick will work
					GradNode = AG->FindNode(1.0);
					if (GradNode != AG->GetActiveNode())
						{
						if (message == WM_LBUTTONDOWN)
							{
							AG->SetActiveNode(GradNode);
							} // if
						} // if
					else
						{
						SetCapture(hwnd);
						} // if
					InvalidateRect(hwnd, NULL, TRUE);
					} // else if
				} // else if
			else if (MouseX < Cola.xLeft)
				{ // prev button
				for (GradNode = AG->GetNextNode(NULL); GradNode; GradNode = AG->GetNextNode(GradNode))
					{
					if (AG->GetNextNode(GradNode) == AG->GetActiveNode())
						{
						if (message == WM_LBUTTONDOWN)
							{
							AG->SetActiveNode(GradNode);
							} // if
						} // if
					} // if
				} // else if
			else if (MouseX > Cola.xRight)
				{ // next button
				if (message == WM_LBUTTONDOWN)
					{
					AG->SetActiveNode(AG->GetNextNode(AG->GetActiveNode()));
					} // if
				} // else if
			} // if
		break;
		} // LBUTTONDOWN
	case WM_MOUSEMOVE:
		{
		if (AG)
			{
			if (wParam & MK_LBUTTON)
				{
				if (GetCapture() == hwnd)
					{
					MouseX = LOWORD(lParam);
					MouseY = HIWORD(lParam);

					GetClientRect(hwnd, &Square);
					Square.left++;  Square.top++;
					Square.right--; Square.bottom--;
					WidgetWidth = Square.right - Square.left;
					// Cola will hold gradient bar bounds
					Cola.yTop   = Square.top;
					Cola.yBot   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
					Cola.xLeft  = Square.left   + WCS_WIDGET_GRADIENT_SIDE_BORDER;
					Cola.xRight = Square.right  - WCS_WIDGET_GRADIENT_SIDE_BORDER;
					// ButtonCola will hold node strip bounds
					ButtonCola.yTop   = Square.bottom - WCS_WIDGET_GRADIENT_LOWER_HEIGHT;
					ButtonCola.yBot   = Square.bottom;
					ButtonCola.xLeft  = Square.left   + WCS_WIDGET_GRADIENT_SIDE_BORDER;
					ButtonCola.xRight = Square.right  - WCS_WIDGET_GRADIENT_SIDE_BORDER;

					GradWidth   = Cola.xRight - Cola.xLeft;

					if ((MouseX >= Cola.xLeft) && (MouseX <= Cola.xRight))
						{ // in main gradient region
						GradPos = ((double)(MouseX - Cola.xLeft)) / (double)GradWidth;
						if (GradNode = AG->GetActiveNode())
							{
							AG->SetNodeDistance(GradNode, GradPos);
							InvalidateRect(hwnd, NULL, TRUE);
							} // if
						} // if
					} // we have the ball
				} // if
			} // if
		break;
		} // MOUSEMOVE
	case WM_CHAR:
		{
		TCHAR Code;
		Code = (TCHAR)wParam;
		if ((Code == ',') || (Code == '['))
			{
			// Previous Node
			for (GradNode = AG->GetNextNode(NULL); GradNode; GradNode = AG->GetNextNode(GradNode))
				{
				if (AG->GetNextNode(GradNode) == AG->GetActiveNode())
					{
					AG->SetActiveNode(GradNode);
					} // if
				} // if
			return(0);
			} // if
		else if ((Code == '.') || (Code == ']'))
			{
			// Next Node
			AG->SetActiveNode(AG->GetNextNode(AG->GetActiveNode()));
			return(0);
			} // if
		return(0);
		} // KEYDOWN
	case WM_SETFOCUS:
		{
		return(0);
		} // WM_SETFOCUS
	} // switch

return(DefWindowProc(hwnd, message, wParam, lParam));
} // AnimGradientWndProc

/*===========================================================================*/

// returns the result in ScrollRect, in widget client coordinates
// TextColumnMarkerRect is in Widget Client coordinates, which means x & y are 0
void CalcTextColumnMarkerVScrollRect(RECT *ScrollRect, const RECT *TextColumnMarkerRect)
{

ScrollRect->top    = TextColumnMarkerRect->top;
ScrollRect->left   = TextColumnMarkerRect->right - GetSystemMetrics(SM_CYVTHUMB);
ScrollRect->right  = TextColumnMarkerRect->right;
ScrollRect->bottom = TextColumnMarkerRect->bottom - GetSystemMetrics(SM_CXHTHUMB); // subtract enough height to account for bottom HScroll

} // CalcTextColumnMarkerVScrollRect

/*===========================================================================*/

void CalcTextColumnMarkerHScrollRect(RECT *ScrollRect, const RECT *TextColumnMarkerRect)
{
ScrollRect->top     = TextColumnMarkerRect->bottom - GetSystemMetrics(SM_CXHTHUMB);
ScrollRect->left    = TextColumnMarkerRect->left;
ScrollRect->right   = TextColumnMarkerRect->right - GetSystemMetrics(SM_CYVTHUMB);
ScrollRect->bottom  = TextColumnMarkerRect->bottom;
} // CalcTextColumnMarkerHScrollRect

/*===========================================================================*/

void CalcTextColumnMarkerTextRect(RECT *TextRect, const RECT *TextColumnMarkerRect)
{
TextRect->top     = TextColumnMarkerRect->top;
TextRect->left    = TextColumnMarkerRect->left;
TextRect->right   = TextColumnMarkerRect->right - GetSystemMetrics(SM_CYVTHUMB);
TextRect->bottom  = TextColumnMarkerRect->bottom - GetSystemMetrics(SM_CXHTHUMB);
} // CalcTextColumnMarkerHScrollRect

/*===========================================================================*/

long WINAPI TextColumnMarkerWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
long Handled = 0;
WORD ID;
WORD X, Y;
NativeControl Parent;
RECT Square;
HDC Canvas;
PAINTSTRUCT PS;
char New = 0, OutBounds = 0;
unsigned long WStyle;
TextColumnMarkerWidgetInstance *WThis, *TCMWI;

Parent  = GetParent(hwnd);
ID      = (WORD) GetWindowLong(hwnd, GWL_ID);
WThis = (TextColumnMarkerWidgetInstance *)GetWindowLong(hwnd, 0);
WStyle  = GetWindowLong(hwnd, GWL_STYLE);

GetClientRect(hwnd, &Square);

switch (message)
	{
	case WM_SETFONT:
		{
		// setup monospaced font based on existing dialog font
		LOGFONT SetupFontAttrs;
		if (GetObject((HFONT)wParam, sizeof(SetupFontAttrs), &SetupFontAttrs))
			{
			// change the face
			strcpy(SetupFontAttrs.lfFaceName, "Lucida Console");
			WThis->_MonospacedFont = CreateFontIndirect(&SetupFontAttrs);
			} // if
		Handled = 1;
		break;
		} // SETFONT
	case WM_WCSW_TC_SETUP:
		{
		TCMWI = (TextColumnMarkerWidgetInstance *)lParam;
		WThis->Text = TCMWI->Text;
		WThis->ColumnMarkers = TCMWI->ColumnMarkers;
		//InvalidateParentRect(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // WM_WCSW_TC_SETUP
	case WM_WCSW_TC_GETDATA:
		{
		std::vector<unsigned long> *DestinationContainer;
		DestinationContainer = (std::vector<unsigned long> *)lParam;
		*DestinationContainer = WThis->ColumnMarkers; // This copies the contents of the container
		break;
		} // WM_WCSW_TC_GETDATA
	case WM_WCSW_TC_SETTEXT:
		{
		WThis->Text = (char *)lParam;
		//InvalidateParentRect(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // WM_WCSW_TC_SETUP
	case WM_KILLFOCUS:
		{
		//InvalidateParentRect(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // KILLFOCUS
	case WM_HSCROLL:
	case WM_VSCROLL:
		{
		// query and use scrollbar info
		SCROLLINFO ScrollerSetup;
		ScrollerSetup.cbSize = sizeof(ScrollerSetup);
		ScrollerSetup.fMask = SIF_ALL;

		if ((HWND)lParam == WThis->_HScroll)
			{
			// determine X scroll position to calculate how many chars (multiplied into pixels) to skip at left of body
			GetScrollInfo(WThis->_HScroll, SB_CTL, &ScrollerSetup);
			switch (LOWORD(wParam))
				{
				case SB_THUMBTRACK:
					{
					WThis->_ScrollOffsetX = ScrollerSetup.nTrackPos;
					break;
					} // SB_THUMBTRACK
				case SB_THUMBPOSITION:
					{
					WThis->_ScrollOffsetX = ScrollerSetup.nPos;
					break;
					} // SB_THUMBPOSITION
				case SB_LINEUP:
					{
					if (WThis->_ScrollOffsetX > ScrollerSetup.nMin)
						{
						WThis->_ScrollOffsetX--;
						} // if
					break;
					} // SB_LINEUP
				case SB_LINEDOWN:
					{
					if (WThis->_ScrollOffsetX < ScrollerSetup.nMax)
						{
						WThis->_ScrollOffsetX++;
						} // if
					break;
					} // SB_LINEDOWN
				case SB_PAGEUP:
					{
					if (WThis->_ScrollOffsetX - (int)ScrollerSetup.nPage >= ScrollerSetup.nMin)
						{
						WThis->_ScrollOffsetX -= ScrollerSetup.nPage;
						} // if
					break;
					} // SB_PAGEUP
				case SB_PAGEDOWN:
					{
					if (WThis->_ScrollOffsetX + (int)ScrollerSetup.nPage <= ScrollerSetup.nMax)
						{
						WThis->_ScrollOffsetX += ScrollerSetup.nPage;
						} // if
					break;
					} // SB_PAGEDOWN
				case SB_BOTTOM:
					{
					WThis->_ScrollOffsetX = ScrollerSetup.nMax;
					break;
					} // SB_BOTTOM
				case SB_TOP:
					{
					WThis->_ScrollOffsetX = ScrollerSetup.nMin;
					break;
					} // SB_TOP
				} // switch

			ScrollerSetup.fMask = SIF_POS;
			ScrollerSetup.nPos = WThis->_ScrollOffsetX;
			SetScrollInfo(WThis->_HScroll, SB_CTL, &ScrollerSetup, 1);
			} // if

		if ((HWND)lParam == WThis->_VScroll)
			{
			// determine Y scroll position to calculate how many lines to skip at top of body
			GetScrollInfo(WThis->_VScroll, SB_CTL, &ScrollerSetup);


			switch (LOWORD(wParam))
				{
				case SB_THUMBTRACK:
					{
					WThis->_ScrollOffsetY = ScrollerSetup.nTrackPos;
					break;
					} // SB_THUMBTRACK
				case SB_THUMBPOSITION:
					{
					WThis->_ScrollOffsetY = ScrollerSetup.nPos;
					break;
					} // SB_THUMBPOSITION
				case SB_LINEUP:
					{
					if (WThis->_ScrollOffsetY > ScrollerSetup.nMin)
						{
						WThis->_ScrollOffsetY--;
						} // if
					break;
					} // SB_LINEUP
				case SB_LINEDOWN:
					{
					if (WThis->_ScrollOffsetY < ScrollerSetup.nMax)
						{
						WThis->_ScrollOffsetY++;
						} // if
					break;
					} // SB_LINEDOWN
				case SB_PAGEUP:
					{
					if (WThis->_ScrollOffsetY - (int)ScrollerSetup.nPage >= ScrollerSetup.nMin)
						{
						WThis->_ScrollOffsetY -= ScrollerSetup.nPage;
						} // if
					break;
					} // SB_PAGEUP
				case SB_PAGEDOWN:
					{
					if (WThis->_ScrollOffsetY + (int)ScrollerSetup.nPage <= ScrollerSetup.nMax)
						{
						WThis->_ScrollOffsetY += ScrollerSetup.nPage;
						} // if
					break;
					} // SB_PAGEDOWN
				case SB_BOTTOM:
					{
					WThis->_ScrollOffsetY = ScrollerSetup.nMax;
					break;
					} // SB_BOTTOM
				case SB_TOP:
					{
					WThis->_ScrollOffsetY = ScrollerSetup.nMin;
					break;
					} // SB_TOP
				} // switch

			ScrollerSetup.fMask = SIF_POS;
			ScrollerSetup.nPos = WThis->_ScrollOffsetY;
			SetScrollInfo(WThis->_VScroll, SB_CTL, &ScrollerSetup, 1);
			} // if
		InvalidateRect(hwnd, NULL, NULL); // repaint fetches current scroll positions and uses them
		} // scroll
	case WM_PAINT:
		{
		if (GetUpdateRect(hwnd, NULL, 0))
			{
			Canvas = BeginPaint(hwnd, &PS);

			// draw background of Text area
			bool Enabled = (WStyle & WS_DISABLED) ? false : true;
			RECT ControlRect, SubControlRect;
			GetClientRect(hwnd, &ControlRect);
			CalcTextColumnMarkerTextRect(&SubControlRect, &ControlRect); // calculate extent of Text area
			if (g_xpStyle.IsAppThemed())
				{
				HTHEME hTheme = g_xpStyle.OpenThemeData(hwnd, L"EDIT");
				g_xpStyle.DrawThemeBackground(hTheme, Canvas, EP_EDITTEXT, Enabled ? ETS_NORMAL : ETS_DISABLED, &SubControlRect, 0);
				g_xpStyle.CloseThemeData(hTheme);
				} // if
			else
				{
				RC WhyNotUseARECT;
				WhyNotUseARECT.xLeft  = SubControlRect.left;
				WhyNotUseARECT.xRight = SubControlRect.right;
				WhyNotUseARECT.yTop   = SubControlRect.top;
				WhyNotUseARECT.yBot   = SubControlRect.bottom;
				Draw3dButtonIn(hwnd, Canvas, &WhyNotUseARECT, 0x03, false, false, (WStyle & WS_DISABLED) ? false : true);
				} // else
			//DrawButtonFocus(hwnd, Canvas, WCSW_TOOLBUTTON_FOCUS_BORDER);
			
			// adjust SubControlRect for margin
			SubControlRect.left   += 2;
			SubControlRect.right  -= 2;
			SubControlRect.top    += 2;
			SubControlRect.bottom -= 2;
			
			// draw header text
			SIZE TextExtent;
			int YPosition = 0, XPosition = 0, PixelWidthOfChar;
			HFONT PrevFont = NULL;
			PrevFont = (HFONT)SelectObject(Canvas, WThis->_MonospacedFont);
			SetTextAlign(Canvas, TA_TOP | TA_LEFT); // flow text top-down, from left
			
			// determine size of a single letter
			GetTextExtentPoint32(Canvas, "m", 1, &TextExtent); // get output text bounding box
			WThis->_StoredPixelWidthOfChar = PixelWidthOfChar = TextExtent.cx;
			
			// use scrollbar info
			// use X scroll position to calculate how many chars (multiplied into pixels) to skip at left of body
			XPosition = WThis->_ScrollOffsetX * PixelWidthOfChar;

			// use Y scroll position to calculate how many lines to skip at top of body
			signed long LinesToSkip, LinesOnPage = 0;
			LinesToSkip = WThis->_ScrollOffsetY;
			
			// there's an extra leading space here to align the text number over the proper mark
			char *HeaderText = "         10        20        30        40        50        60        70        80        90        100       110       120        130        140        150        160";
			int BkMode = SetBkMode(Canvas, TRANSPARENT); // inhibit rasterization behind text
			ExtTextOut(Canvas, SubControlRect.left - XPosition, SubControlRect.top + YPosition, ETO_CLIPPED, &SubControlRect, HeaderText, strlen(HeaderText), NULL);
			SetBkMode(Canvas, BkMode); // put it back
			GetTextExtentPoint32(Canvas, HeaderText, strlen(HeaderText), &TextExtent); // get output text bounding box
			YPosition += TextExtent.cy; // for the text line
			YPosition += 6; // 6 pixel vertical margin for scale
			
			// draw header scale
			// draw bottom line
			MoveToEx(Canvas, SubControlRect.left, YPosition, NULL);
			LineTo(Canvas, SubControlRect.right, YPosition);
			
			int TickX;
			// draw individual tick marks
			for (TickX = SubControlRect.left - XPosition; TickX < SubControlRect.right; TickX += PixelWidthOfChar)
				{
				MoveToEx(Canvas, TickX, YPosition, NULL);
				LineTo(Canvas, TickX, YPosition - 2); // 2-pixel high individual ticks
				} // for

			// draw 5-char tick marks
			for (TickX = SubControlRect.left - XPosition; TickX < SubControlRect.right; TickX += PixelWidthOfChar * 5)
				{
				MoveToEx(Canvas, TickX, YPosition, NULL);
				LineTo(Canvas, TickX, YPosition - 4); // 4-pixel high 5-char ticks
				} // for
			
			// draw 10-char tick marks (skip over first one)
			for (TickX = (SubControlRect.left - XPosition) + (PixelWidthOfChar * 10); TickX < SubControlRect.right; TickX += PixelWidthOfChar * 10)
				{ // 10-char ticks are 3 pixels wide
				MoveToEx(Canvas, TickX - 1, YPosition, NULL);
				LineTo(Canvas, TickX - 1, YPosition - 5); // 5-pixel high 5-char ticks
				MoveToEx(Canvas, TickX, YPosition, NULL);
				LineTo(Canvas, TickX, YPosition - 5); // 5-pixel high 5-char ticks
				MoveToEx(Canvas, TickX + 1, YPosition, NULL);
				LineTo(Canvas, TickX + 1, YPosition - 5); // 5-pixel high 5-char ticks
				} // for

			// draw markers
			YPosition += 4; // 4 pixel vertical margin for markers
			unsigned long MarkerYPos = YPosition; // drawn after text, below

			// draw body text
			WThis->_MaxTextLengthPixels = WThis->_NumLinesInBody = 0; // recalculated during each redraw
			if (const char *StringBegin = WThis->Text)
				{
				unsigned long StringLen, OutputLen = 0;
				const char *OutputString, *StringScan;
				for (bool KeepGoing = true; KeepGoing;)
					{
					// cruise forward until we meet a NULL or linebreak
					StringLen = 0;
					for (StringScan = StringBegin; StringScan[0] != NULL && StringScan[0] != '\n' && StringScan[0] != '\r'; StringScan++)
						{
						StringLen++;
						} // for
					if (StringScan[0] == NULL)
						{
						KeepGoing = false; // this will break us out of the while loop at the end of the loop, after rasterizing any text
						} // if
					else
						{ // must br CR or LF, skip ahead over them
						// we need to treat CR,LF or LF,CR as a single line break
						if (StringScan[0] == '\r')
							{
							if (StringScan[1] == '\n')
								{
								StringScan++; // skip an extra input char
								} // if
							StringScan++;
							} // if
						else if (StringScan[0] == '\n')
							{
							if (StringScan[1] == '\r')
								{
								StringScan++; // skip an extra input char
								} // if
							StringScan++;
							} // else if
						} // else
					if (StringLen == 0)
						{ // we probably need a dummy space char to make line advance height work properly
						OutputString = " ";
						OutputLen = 1;
						} // if
					else
						{
						OutputString = StringBegin;
						OutputLen = StringLen;
						} // else
					StringBegin = StringScan; // begin next scan at the end of this scan
					GetTextExtentPoint32(Canvas, OutputString, OutputLen, &TextExtent); // get output text bounding box
					if (!(WThis->_NumLinesInBody < LinesToSkip)) // should we skip the line or draw it?
						{
						BkMode = SetBkMode(Canvas, TRANSPARENT); // inhibit rasterization behind text
						ExtTextOut(Canvas, SubControlRect.left - XPosition, SubControlRect.top + YPosition, ETO_CLIPPED, &SubControlRect, OutputString, OutputLen, NULL);
						SetBkMode(Canvas, BkMode); // put it back
						YPosition += TextExtent.cy + 2; // 2 pixel vertical margin
						} // if
					WThis->_NumLinesInBody++;
					if (YPosition < SubControlRect.bottom)
						{
						LinesOnPage = WThis->_NumLinesInBody; // we'll stop updating when text flows off bottom
						} // if
					if (TextExtent.cx > WThis->_MaxTextLengthPixels)
						{
						WThis->_MaxTextLengthPixels = TextExtent.cx;
						} // if
					} // for
				} // if
			
			// Draw markers
			std::vector<unsigned long>::iterator MarkerWalk;
			for (MarkerWalk = WThis->ColumnMarkers.begin(); MarkerWalk != WThis->ColumnMarkers.end(); MarkerWalk++)
				{
				int MarkerX = SubControlRect.left + (*MarkerWalk * PixelWidthOfChar) - XPosition; // XPosition accounts for scrolling
				// marker line
				MoveToEx(Canvas, MarkerX, MarkerYPos, NULL);
				LineTo(Canvas, MarkerX, SubControlRect.bottom);
				
				// marker triangle
				MoveToEx(Canvas, MarkerX, MarkerYPos - 4, NULL);
				LineTo(Canvas, MarkerX-2, MarkerYPos); // 4-pixel high marker triangles
				LineTo(Canvas, MarkerX+3, MarkerYPos); // LineTo never draws the last pixel of the line...
				MoveToEx(Canvas, MarkerX, MarkerYPos - 4, NULL);
				LineTo(Canvas, MarkerX+3, MarkerYPos);
				} // for

			SelectObject(Canvas, PrevFont); // restore original font
			EndPaint(hwnd, &PS);
			
			static bool RangeSetup = 0; // init only fires once. Learn something new every day.
			
			if (!RangeSetup) {
			// set scrollbar ranges
			SCROLLINFO ScrollerSetup;
			ScrollerSetup.cbSize = sizeof(ScrollerSetup);
			
			// vertical scrollbar
			if (LinesOnPage > 0 && WThis->_NumLinesInBody > LinesOnPage) // is Vert scrollbar even needed? (avoid division by zero below)
				{
				ScrollerSetup.fMask = SIF_RANGE | SIF_PAGE;
				ScrollerSetup.nMin = 0;
				ScrollerSetup.nMax = WThis->_NumLinesInBody - LinesOnPage; // units of lines
				ScrollerSetup.nPage = (UINT)((float)ScrollerSetup.nMax * ((float)LinesOnPage / (float)(WThis->_NumLinesInBody)));
				if (ScrollerSetup.nPage < 1) ScrollerSetup.nPage = 1;
				} // if
			else
				{ // disable it
				ScrollerSetup.fMask = SIF_DISABLENOSCROLL;
				} // else
			SetScrollInfo(WThis->_VScroll, SB_CTL, &ScrollerSetup, true);

			// horizontal scrollbar
			if (WThis->_MaxTextLengthPixels > 0 && WThis->_MaxTextLengthPixels > SubControlRect.right) // is Horiz scrollbar even needed? (avoid division by zero below)
				{
				ScrollerSetup.fMask = SIF_RANGE | SIF_PAGE;
				ScrollerSetup.nMin = 0;
				ScrollerSetup.nMax = ((WThis->_MaxTextLengthPixels - SubControlRect.right) / PixelWidthOfChar) + 1; // units of lines, add one to round up in case excess width is not a whole char
				//ScrollerSetup.nPage = (UINT)((float)ScrollerSetup.nMax * ((float)SubControlRect.right / (float)(WThis->_MaxTextLengthPixels)));
				ScrollerSetup.nPage = 1; // <<<>>> this needs some work
				if (ScrollerSetup.nPage < 1) ScrollerSetup.nPage = 1;
				} // if
			else
				{ // disable it
				ScrollerSetup.fMask = SIF_DISABLENOSCROLL;
				} // else
			SetScrollInfo(WThis->_HScroll, SB_CTL, &ScrollerSetup, true);
			RangeSetup = true;
			} // if !RangeSetup

			} // if
		Handled = 1;
		break;
		} // PAINT
	case WM_SETFOCUS:
		{
		SetFocus(hwnd);
		//InvalidateParentRect(hwnd);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
		} // SETFOCUS
	case WM_ENABLE:
		{
		New = 1; // force a redraw: "The WM_ENABLE message is sent when an application changes the enabled state of a window. It is sent to the window whose enabled state is changing."
		break;
		} // WM_ENABLE
	case WM_LBUTTONDOWN:
		{
		RECT ControlRect, SubControlRect;

		GetClientRect(hwnd, &ControlRect);
		CalcTextColumnMarkerTextRect(&SubControlRect, &ControlRect); // calculate extent of Text area
		// adjust SubControlRect for margin
		SubControlRect.left   += 2;
		SubControlRect.right  -= 2;
		SubControlRect.top    += 2;
		SubControlRect.bottom -= 2;

		SetFocus(hwnd);
		X = LOWORD(lParam); Y = HIWORD(lParam);
		if (!(X > SubControlRect.right || X < SubControlRect.left || Y > SubControlRect.bottom || Y < SubControlRect.top))
			{
			unsigned long Column;
			signed long Action = 0;
			int XPosition = 0;
			std::vector<unsigned long>::iterator FoundLocation;
			XPosition = WThis->_ScrollOffsetX * WThis->_StoredPixelWidthOfChar;
			Column = (int)((X + XPosition + (WThis->_StoredPixelWidthOfChar / 2) - SubControlRect.left) / (float)(WThis->_StoredPixelWidthOfChar));
			if ((FoundLocation = std::find(WThis->ColumnMarkers.begin(), WThis->ColumnMarkers.end(), Column)) != WThis->ColumnMarkers.end())
				{ // found one, remove it
				WThis->ColumnMarkers.erase(FoundLocation);
				Action = -1; // remove
				} // if
			else
				{ // didn't find one, add it
				WThis->ColumnMarkers.push_back(Column);
				Action = 1; // Add
				} // else
			SendMessage(Parent, WM_WCSW_TC_DATACHANGED, (WPARAM)(ID), (LPARAM)Action);
			InvalidateRect(hwnd, NULL, NULL); // redraw
			} // if
		break;
		} // LBUTTONDOWN
	case WM_CREATE:
		{
		if (WThis = (TextColumnMarkerWidgetInstance *)AppMem_Alloc(sizeof(TextColumnMarkerWidgetInstance), APPMEM_CLEAR))
			{
			HINSTANCE MyInst = (HINSTANCE)GetClassLong(hwnd, GCL_HMODULE);
			RECT ControlRect, SubControlRect;
			SetWindowLong(hwnd, 0, (long)WThis);
			GetClientRect(hwnd, &ControlRect);
			CalcTextColumnMarkerVScrollRect(&SubControlRect, &ControlRect); // calculate extent of VScroll
			WThis->_VScroll = CreateWindowEx(NULL, "SCROLLBAR", "VScroll", WS_CHILD | WS_VISIBLE | SBS_VERT, SubControlRect.left, SubControlRect.top, SubControlRect.right - SubControlRect.left, SubControlRect.bottom - SubControlRect.top, hwnd, NULL, MyInst, NULL);
			CalcTextColumnMarkerHScrollRect(&SubControlRect, &ControlRect); // calculate extent of VScroll
			WThis->_HScroll = CreateWindowEx(NULL, "SCROLLBAR", "HScroll", WS_CHILD | WS_VISIBLE | SBS_HORZ, SubControlRect.left, SubControlRect.top, SubControlRect.right - SubControlRect.left, SubControlRect.bottom - SubControlRect.top, hwnd, NULL, MyInst, NULL);
			return(0); // hunkidori
			} // if
		else
			{
			SetWindowLong(hwnd, 0, (long)NULL);
			return(-1); // bah, humbug
			} // else
		} // CREATE
	case WM_DESTROY:
		{
		SetWindowLong(hwnd, 0, 0);
		AppMem_Free(WThis, sizeof(TextColumnMarkerWidgetInstance));
		return(0);
		} // DESTROY
	} // switch

if (New)
	{
	InvalidateRect(hwnd, NULL, NULL); // redraw ourselves
	New = 0;
	} // if

if (!Handled)
	{
	return(DefWindowProc(hwnd, message, wParam, lParam));
	} // if
else
	{
	return(0);
	} // else
} // TextColumnMarkerWndProc



# endif // _WIN32





#define OWNERDRAWNAMEBUFLEN 300
static char OwnerDrawNameBuf[OWNERDRAWNAMEBUFLEN];


void HandleOwnerDrawGook(NativeControl hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int Mode)
{
LPDRAWITEMSTRUCT lpdis;
LPMEASUREITEMSTRUCT lpmis;
TEXTMETRIC tm;
HDC TextCheck;
Joe *Moi = NULL;
char *RealText;
const char *TempName, *FName;
int Remain, y;

if (uMsg == WM_MEASUREITEM)
	{
	lpmis = (LPMEASUREITEMSTRUCT)lParam;
	TextCheck = GetDC(hwnd);
	GetTextMetrics(TextCheck, &tm);
	ReleaseDC(hwnd, TextCheck);
	// Calculate height from font size
	lpmis->itemHeight = tm.tmAscent;
	} // if
else if (uMsg == WM_DRAWITEM)
	{
	HFONT CurFont = NULL;
	lpdis = (LPDRAWITEMSTRUCT)lParam;

	HFONT BoldListBoxFont = NULL;
	HFONT ItalicListBoxFont = NULL;
	if (!(BoldListBoxFont = (HFONT)GlobalApp->WinSys->WL->FetchBoldListBoxFont()))
		{
		LOGFONT SetupFontAttrs;
		// Need to create BoldListBoxFont from normal listbox font
		if (CurFont = (HFONT)GetCurrentObject(lpdis->hDC, OBJ_FONT))
			{
			if (GetObject(CurFont, sizeof(SetupFontAttrs), &SetupFontAttrs))
				{
				// make it bold
				LONG OriginalWeight = SetupFontAttrs.lfWeight;
				SetupFontAttrs.lfWeight = FW_BOLD;
				if (BoldListBoxFont = CreateFontIndirect(&SetupFontAttrs))
					{
					GlobalApp->WinSys->WL->StoreBoldListBoxFont(BoldListBoxFont);
					} // if
				// make it italic
				SetupFontAttrs.lfWeight = OriginalWeight;
				SetupFontAttrs.lfItalic = TRUE;
				if (ItalicListBoxFont = CreateFontIndirect(&SetupFontAttrs))
					{
					GlobalApp->WinSys->WL->StoreItalicListBoxFont(ItalicListBoxFont);
					} // if
				} // if
			} // if
		} // if
	else
		{ // italic should be there too
		ItalicListBoxFont = (HFONT)GlobalApp->WinSys->WL->FetchItalicListBoxFont();
		} // else
		
	// If there are no list box items, skip this message.
	if (lpdis->itemID == 0xffffffff) return; // used to be -1 as per MS docs, but this caused type sign mismatch warnings. Duh.

	// Draw the text for the list box item.

	switch (lpdis->itemAction)
		{
		case ODA_FOCUS:
			{
            DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
			break;
			} // ODA_FOCUS
	    case ODA_SELECT:
	    case ODA_DRAWENTIRE:
			{
	        // Display the text associated with the item.

			// Pad text out to the right
	        memset(Scratch, ' ', WCSW_SCRATCHSIZE - 1);
			Scratch[WCSW_SCRATCHSIZE - 1] = NULL;
			
			// Given the item ID (lpdis->itemID), obtain the
			// text associated with it and put it into Scratch
	        SendMessage(lpdis->hwndItem, LB_GETTEXT,
	            lpdis->itemID, (LPARAM)Scratch);
			if (Mode == 1)
				{
				Moi = (Joe *)lpdis->itemData;
				//RealText = Scratch;
				if (Moi)
					{
					OwnerDrawNameBuf[0] = 0;

					// Only used if Groups are shown
					//if (Moi->TestFlags(WCS_JOEFLAG_HASKIDS))
					//	{
					//	strcat(OwnerDrawNameBuf, ">");
					//	} // if

					if (TempName = Moi->Name())
						{
						// limit the length of data copied...
						strncat(OwnerDrawNameBuf, TempName, OWNERDRAWNAMEBUFLEN - 2);
						} // if
					if (FName = Moi->FileName())
						{
						if (FName[0])
							{
							// <<<>>> Should limit the length of data copied...
							if (TempName) strcat(OwnerDrawNameBuf, " (");
							strcat(OwnerDrawNameBuf, FName);
							if (TempName) strcat(OwnerDrawNameBuf, ")");
							} // if
						} // if
					if (TempName = Moi->CanonName())
						{
						Remain = (OWNERDRAWNAMEBUFLEN - strlen(OwnerDrawNameBuf));
						strncat(OwnerDrawNameBuf, TempName, Remain - 2);
						} // if
					if (TempName = Moi->SecondaryName())
						{
						Remain = (OWNERDRAWNAMEBUFLEN - (strlen(OwnerDrawNameBuf) + 2));
						strcat(OwnerDrawNameBuf, ", ");
						strncat(OwnerDrawNameBuf, TempName, Remain - 2);
						} // if
					if ((OwnerDrawNameBuf[0] == NULL)/* || (OwnerDrawNameBuf[1] == NULL) */)
						{
						strcpy(OwnerDrawNameBuf, "Unnamed");
						if (Moi->TestFlags(WCS_JOEFLAG_HASKIDS))
							{
							strcat(OwnerDrawNameBuf, " Group");
							} // if
						} // if

					OwnerDrawNameBuf[OWNERDRAWNAMEBUFLEN - 1] = NULL;
					RealText = OwnerDrawNameBuf;
					} // if

				} // if
			else
				{
				RealText = &Scratch[2];
				} // else

	        GetTextMetrics(lpdis->hDC, &tm);

	        y = (lpdis->rcItem.bottom + lpdis->rcItem.top -
	            tm.tmHeight) / 2;

	        // Is the item selected?
			if (lpdis->itemState & ODS_SELECTED)
				{
				SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
				SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				} // if
			else
				{
				SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
				SetTextColor(lpdis->hDC, GetSysColor(COLOR_BTNTEXT));
				} // else

	        if (Mode == 1)
				{
		        if (Moi)
					{
					// if the item is not 'activated', use a different color scheme
					if (!Moi->TestFlags(WCS_JOEFLAG_ACTIVATED))
						{
						if (lpdis->itemState & ODS_SELECTED)
							{
							SetTextColor(lpdis->hDC, GetSysColor(COLOR_BTNFACE));
							} // if
						else
							{
							SetTextColor(lpdis->hDC, GetSysColor(COLOR_BTNSHADOW));
							} // else
						} // if
					} // if
				} // if
			else
				{
				// if the item is not 'activated', use a different color scheme
				if ((Scratch[1] == '3') || (Scratch[0] == '*'))
					{ // selected
					} // if
				else if ((Scratch[0] == '#') && BoldListBoxFont)
					{ // bolded
					CurFont = (HFONT)SelectObject(lpdis->hDC, BoldListBoxFont); // We'll put back the original font below, after ExtTextOut
					} // else if
				else if ((Scratch[0] == '/') && ItalicListBoxFont)
					{ // bolded
					CurFont = (HFONT)SelectObject(lpdis->hDC, ItalicListBoxFont); // We'll put back the original font below, after ExtTextOut
					} // else if
				else // not selected
					{
					if (lpdis->itemState & ODS_SELECTED)
						{
						SetTextColor(lpdis->hDC, GetSysColor(COLOR_BTNFACE));
						} // if
					else
						{
						SetTextColor(lpdis->hDC, GetSysColor(COLOR_BTNSHADOW));
						} // else
					} // if
				} // else

	        // Draw the text
	        ExtTextOut(lpdis->hDC,
	            0,

	            y,
				ETO_OPAQUE,
				&lpdis->rcItem,
	            RealText,
	            (UINT)strlen(RealText), NULL);
	        if (CurFont) SelectObject(lpdis->hDC, CurFont); // put back the original font, just in case we echanged it earlier
	        break;
			} // case
		} // switch
	} // if

} // HandleOwnerDrawGook

/*===========================================================================*/

# ifdef _WIN32
TrueColorIcon::TrueColorIcon()
{
BitArray = NULL;
BASize = NULL;
R = G = B = NULL;
Width = Height = 0;
} // TrueColorIcon::TrueColorIcon

TrueColorIcon::~TrueColorIcon()
{
if (R)
	{
	delete[] R;
	R = NULL;
	} // if
if (G)
	{
	delete[] G;
	G = NULL;
	} // if
if (B)
	{
	delete[] B;
	B = NULL;
	} // if

#ifdef _WIN32
if (BitArray)
	{
	AppMem_Free(BitArray, BASize);
	BitArray = NULL;
	BASize = NULL;
	} // if
#endif // _WIN32

} // TrueColorIcon::~TrueColorIcon
# endif

//lint -restore
