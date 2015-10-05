// WidgetSupport.h
// Support for making custom classes/widgets more portable
// Built from parts of V2 TLWidget.cpp by CXH on 6/12/96
// Original code by CXH
// Copyright 1996

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
	{
	WCS_WIDGET_COLOR_BKG,
	WCS_WIDGET_COLOR_BLACK,
	WCS_WIDGET_COLOR_WHITE,
	WCS_WIDGET_COLOR_RED,
	WCS_WIDGET_COLOR_DKBLUE,
	WCS_WIDGET_COLOR_GREEN,
	WCS_WIDGET_COLOR_LTBLUE,
	WCS_WIDGET_COLOR_YELLOW
	}; // Widget drawing colors

#ifdef _WIN32
#define WCSW_WIDGET_ACTION message
#define WCSW_WIDGET_INFO wParam
#define WCSW_WIDGET_X LOWORD(lParam)
#define WCSW_WIDGET_Y HIWORD(lParam)
#define WCSW_WIDGET_FLAGS data->drawflags
#endif // _WIN32

// Button Focus Border sizes
#define WCSW_TOOLBUTTON_FOCUS_BORDER   2

// Call-translation layer to mangle widget-GUI calls into the
// apropriate OS-GUI calls for each platform.
void WCSW_SetWidgetDrawColor(void *DrawCon, int NewColor);

int WCSW_GetTextLength(void *DrawP, char *Text, int Len);
int WCSW_GetTextHeight(void *DrawP, char *Text, int Len);
void WCSW_Move(void *DrawP, long X, long Y);
void WCSW_Draw(void *DrawP, long X, long Y);
void WCSW_DotLine(void *DrawP, long XS, long YS, long XE, long YE);
void WCSW_Text(void *DrawP, const char *Str, int Len);
void WCSW_BlackOut(void *DrawP, long Left, long Top, long Right,
 long Bottom);

int GetChildCoords(NativeControl hwnd, RECT *Pos);
void DrawButtonFocus(HWND hwnd, HDC Canvas, unsigned char Border);
void InvalidateParentRect(HWND MeNotMyParent);
BOOL GetClientRectInParentCoords(HWND hWnd, LPRECT lpRect);
BOOL MoveWindowX(HWND hWnd, int X);
BOOL MoveWindowY(HWND hWnd, int Y);

// not currently used
/*
void SetControlText(NativeControl DestDlg, int Control, double Amount);
void SetControlText(NativeControl DestDlg, int Control, int Amount);
void SetControlText(NativeControl DestDlg, int Control, char *Text);
*/

/*-----------------------------------------------------------------------
|   Draw3d - Routines to help add 3D effects to Windows
-----------------------------------------------------------------------*/

BOOL InternalDraw3dColorChange(BOOL fForce);
static VOID DeleteObjectNull(HBRUSH *ph);
VOID DeleteObjects(VOID);
//VOID PatFill(HDC hdc, RC FAR *lprc);
static VOID DrawButtonBorder(NativeControl ButtonWnd, HDC hdc, RC FAR *lprc, int Fill = 1);

// Index Color Table
// WARNING: change mpicvSysColors if you change the icv order
typedef int ICV;
#define ICVBTNHILITE   0
#define ICVBTNFACE     1
#define ICVBTNSHADOW   2
#define ICVBRUSHMAX    3

#define ICVBTNTEXT     3
#define ICVWINDOW      4
#define ICVWINDOWTEXT  5
#define ICVGRAYTEXT    6
#define ICVWINDOWFRAME 7
#define ICVMAX         8

  
// DrawRec3d flags
#define DR3LEFT  0x0001
#define DR3TOP   0x0002
#define DR3RIGHT 0x0004
#define DR3BOT   0x0008
#define DR3HACKBOTRIGHT 0x1000  // code size is more important than aesthetics
#define DR3ALL    0x000f

typedef WORD DR3;     

HBRUSH Draw3dCtlColor(UINT wm, WPARAM wParam, LPARAM lParam);
BOOL   Draw3dColorChange(void);

VOID Draw3dRec(NativeControl ButtonWnd, NativeDrawContext hdc, RC FAR *lprc, ICV icvUpperLeft, ICV icvLowerRight, DR3 rdr3);
//VOID Draw3dInsetRect(NativeControl ButtonWnd, NativeDrawContext hdc, RC FAR *prc, DR3 dr3);
VOID Draw3dButtonIn(NativeControl ButtonWnd, NativeDrawContext hdc, RC FAR *prc, int Border, bool NewStyle, bool Hot, bool Enabled = true);
VOID Draw3dButtonOut(NativeControl ButtonWnd, NativeDrawContext hdc, RC FAR *prc, int Border, bool NewStyle, bool Hot, bool Enabled = true);
VOID Draw3dGreyBkgnd(NativeControl ButtonWnd, NativeDrawContext hdc, RC FAR *prc);
