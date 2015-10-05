// WidgetSupport.cpp
// Support code for making custom classes/widgets more portable
// Built from parts of V2 TLWidget.cpp by CXH on 6/12/96
// Original code by CXH
// Copyright 1996

#include "stdafx.h"
#include "Application.h"
#include "GUI.h"
#include "WCSWidgets.h"
#include "WidgetSupport.h"
#include "VisualStylesXP.h"

extern WCSApp *GlobalApp;

/*===========================================================================*/

//static unsigned char WidgetDrawColRed, WidgetDrawColGrn, WidgetDrawColBlu;

void WCSW_SetWidgetDrawColor(void *DrawCon, int NewColor)
{
#ifdef _WIN32
unsigned char R = 0, G = 0, B = 0;
switch(NewColor)
	{
	case WCS_WIDGET_COLOR_BKG:
		{
		R = 0x88; G = 0x99; B = 0xbb;
		break;
		} // 
	case WCS_WIDGET_COLOR_BLACK:
		{
		break;
		} // 
	case WCS_WIDGET_COLOR_WHITE:
		{
		R = 0xff; G = 0xff; B = 0xff;
		break;
		} // 
	case WCS_WIDGET_COLOR_RED:
		{
		R = 0xff;
		break;
		} // 
	case WCS_WIDGET_COLOR_DKBLUE:
		{
		R = 0x33; G = 0x44; B = 0x88;
		break;
		} // 
	case WCS_WIDGET_COLOR_GREEN:
		{
		G = 0xff;
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

//WidgetDrawColRed = R;
//WidgetDrawColGrn = G;
//WidgetDrawColBlu = B;

SelectObject((HDC)DrawCon, GlobalApp->WinSys->WL->WidgetPens[NewColor]);
SetTextColor((HDC)DrawCon, RGB(R, G, B));

#endif // _WIN32
} // WCSW_SetWidgetDrawColor()

/*===========================================================================*/

int WCSW_GetTextLength(void *DrawP, char *Text, int Len)
{
#ifdef _WIN32
SIZE SZ;
GetTextExtentPoint32((HDC)DrawP, Text, Len, &SZ);
return(SZ.cx);
#endif // _WIN32
} // WCSW_GetTextLength

/*===========================================================================*/

int WCSW_GetTextHeight(void *DrawP, char *Text, int Len)
{
#ifdef _WIN32
SIZE SZ;
GetTextExtentPoint32((HDC)DrawP, Text, Len, &SZ);
return(SZ.cy);
#endif // _WIN32
} // WCSW_GetTextHeight

/*===========================================================================*/

#ifdef _WIN32
static int WCSW_Move_StoreX, WCSW_Move_StoreY;
#endif // _WIN32

void WCSW_Move(void *DrawP, long X, long Y)
{
#ifdef _WIN32
WCSW_Move_StoreX = X;
WCSW_Move_StoreY = Y;
MoveToEx((HDC)DrawP, X, Y, NULL);
#endif // _WIN32
} // WCSW_Move()

/*===========================================================================*/

void WCSW_Draw(void *DrawP, long X, long Y)
{
#ifdef _WIN32
LineTo((HDC)DrawP, X, Y);
#endif // _WIN32
} // WCSW_Draw()

/*===========================================================================*/

void WCSW_Text(void *DrawP, const char *Str, int Len)
{
#ifdef _WIN32
SetTextColor((HDC)DrawP, RGB(0x33,0x77,0xcc));
SetBkColor((HDC)DrawP, RGB(0,0,0));
ExtTextOut((HDC)DrawP, WCSW_Move_StoreX, WCSW_Move_StoreY, NULL, NULL,
 (char *)Str, Len, (int far *) NULL);
#endif // _WIN32
} // WCSW_Text()

/*===========================================================================*/

void WCSW_BlackOut(void *DrawP, long Left, long Top, long Right,
 long Bottom)
{
#ifdef _WIN32
HBRUSH SpareBsh;
HPEN SparePen;
SpareBsh = (HBRUSH)SelectObject((HDC)DrawP, GetStockObject(BLACK_BRUSH));
SparePen = (HPEN)SelectObject((HDC)DrawP, GetStockObject(NULL_PEN));
Rectangle((HDC)DrawP, Left, Top, Right + 1, Bottom + 1);
SelectObject((HDC)DrawP, SpareBsh);
SelectObject((HDC)DrawP, SparePen);
#endif // _WIN32
} // WCSW_BlackOut()

//#ifdef _WIN32
//LOGPEN ColorLookUp;
//#endif // _WIN32

void WCSW_DotLine(void *DrawP, long XS, long YS, long XE, long YE)
{
#ifdef _WIN32
//HPEN Old = NULL, New;
//if(New = CreatePen(PS_SOLID, 0, RGB(WidgetDrawColRed / 2, WidgetDrawColGrn / 2, WidgetDrawColBlu / 2)))
//	{
//	Old = SelectObject((HDC)DrawP, New);
//	} // if
MoveToEx((HDC)DrawP, XS, YS, NULL);
LineTo((HDC)DrawP, XE, YE);
//if(Old && New)
//	{
//	SelectObject((HDC)DrawP, Old);
//	DeleteObject(New);
//	} // if
#endif // _WIN32
} // WCSW_DotLine




#ifdef _WIN32
static WINDOWPLACEMENT PrivateWidgetInquiry;

int GetChildCoords(NativeControl hwnd, RECT *Pos)
{
PrivateWidgetInquiry.length = sizeof(WINDOWPLACEMENT);
if(GetWindowPlacement(hwnd, &PrivateWidgetInquiry))
	{
	Pos->left    = PrivateWidgetInquiry.rcNormalPosition.left;
	Pos->top     = PrivateWidgetInquiry.rcNormalPosition.top;
	Pos->right   = PrivateWidgetInquiry.rcNormalPosition.right;
	Pos->bottom  = PrivateWidgetInquiry.rcNormalPosition.bottom;
	return(1);
	} // if
return(0);
} // GetChildCoords


void DrawButtonFocus(HWND hwnd, HDC Canvas, unsigned char Border)
{
RECT Square;

if(GetFocus() == hwnd) // do we have the focus?
	{
	// Ignore widgets in toolbar
	if(GetWindowLong(GetParent(hwnd), GWL_WNDPROC) != (long)WndProc)
		{
		GetClientRect(hwnd, &Square);
		Square.left    += Border;
		Square.right   -= Border;
		Square.top     += Border;
		Square.bottom  -= Border;
		DrawFocusRect(Canvas, &Square);
		} // if
	} // if
} // DrawButtonFocus

void InvalidateParentRect(HWND MeNotMyParent)
{
HWND Parent = GetParent(MeNotMyParent);

RECT ParentRect;
POINT PointCvt;
GetWindowRect(MeNotMyParent, &ParentRect); // in screen coords
PointCvt.x = ParentRect.left;
PointCvt.y = ParentRect.top;
ScreenToClient(Parent, &PointCvt);
ParentRect.left = PointCvt.x;
ParentRect.top = PointCvt.y;
PointCvt.x = ParentRect.right;
PointCvt.y = ParentRect.bottom;
ScreenToClient(Parent, &PointCvt);
ParentRect.right = PointCvt.x;
ParentRect.bottom = PointCvt.y;
InvalidateRect(Parent, &ParentRect, TRUE);

} // InvalidateParentRect



BOOL GetClientRectInParentCoords(HWND hWnd, LPRECT lpRect)
{
RECT ScreenRect;
POINT UL, LR;
GetWindowRect(hWnd, &ScreenRect);
UL.x = ScreenRect.left;
UL.y = ScreenRect.top;
LR.x = ScreenRect.right;
LR.y = ScreenRect.bottom;
ScreenToClient(GetParent(hWnd), &UL);
ScreenToClient(GetParent(hWnd), &LR);
lpRect->left = UL.x;
lpRect->top = UL.y;
lpRect->right = LR.x;
lpRect->bottom = LR.y;
return(FALSE);
} // GetClientRectInParentCoords

BOOL MoveWindowX(HWND hWnd, int X)
{
int Y = 0;
RECT WindowRect;
GetClientRectInParentCoords(hWnd, &WindowRect);
Y = WindowRect.top;
return(SetWindowPos(hWnd, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
} // MoveWindowX


BOOL MoveWindowY(HWND hWnd, int Y)
{
int X = 0;
RECT WindowRect;
GetClientRectInParentCoords(hWnd, &WindowRect);
X = WindowRect.left;
return(SetWindowPos(hWnd, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
} // MoveWindowY




#endif // _WIN32

// Not currently used
/*
static char Transfer[30];
void SetControlText(NativeControl DestDlg, int Control, double Amount)
{
sprintf(Transfer, "%#f", Amount);
TrimDecimalZeros(Transfer);
SendDlgItemMessage(DestDlg, Control, WM_SETTEXT, 0, (LPARAM)Transfer);

} // SetControlText(double)

void SetControlText(NativeControl DestDlg, int Control, int Amount)
{
sprintf(Transfer, "%d", Amount);
SendDlgItemMessage(DestDlg, Control, WM_SETTEXT, 0, (LPARAM)Transfer);
} // SetControlText(int)

void SetControlText(NativeControl DestDlg, int Control, char *Text)
{
SendDlgItemMessage(DestDlg, Control, WM_SETTEXT, 0, (LPARAM)Text);
} // SetControlText(int)
*/


/*-----------------------------------------------------------------------
 |   Draw3d stuff
 -----------------------------------------------------------------------*/


typedef COLORREF CV;
// CoLoR Table
typedef struct
    {
    CV rgcv[ICVMAX];
    } CLRT;



// Brush Table
typedef struct
    {
    HBRUSH mpicvhbr[ICVBRUSHMAX];
    } BRT;

/*-----------------------------------------------------------------------
 |   Draw3d Global ( Static ) Variables
 -----------------------------------------------------------------------*/
static CLRT clrt;
static BRT brt;    
BOOL f3dDialogs;



// ************************** Draw3d code ****************************


//
// Sample 3D Drawing routines
//  

int mpicvSysColor[] =
    {
    COLOR_BTNHIGHLIGHT,
    COLOR_BTNFACE,
    COLOR_BTNSHADOW,
    COLOR_BTNTEXT,
    COLOR_WINDOW,
    COLOR_WINDOWTEXT,
    COLOR_GRAYTEXT,
    COLOR_WINDOWFRAME
    };



 
/*-----------------------------------------------------------------------
|   Draw3d Utility routines
-----------------------------------------------------------------------*/
static VOID DeleteObjectNull(HBRUSH *ph)
    {
    if (*ph != NULL)
        {
        DeleteObject(*ph);
        *ph = NULL;
        }
    }

VOID DeleteObjects(VOID)
    {
    int icv;

    for(icv = 0; icv < ICVBRUSHMAX; icv++)
        DeleteObjectNull(&brt.mpicvhbr[icv]);
    }


//static VOID PatFill(HDC hdc, RC FAR *lprc)
//    {
//    PatBlt(hdc, lprc->xLeft, lprc->yTop, lprc->xRight-lprc->xLeft, lprc->yBot-lprc->yTop, PATCOPY);
//    }

       
/*-----------------------------------------------------------------------
|   Draw3dRec
|
|   Arguments:
|       HDC hdc:    
|       RC FAR *lprc:   
|       LONG cvUpperLeft:  
|       LONG cvLowerRight:  
|       WORD grbit;
|       
|   Returns:
|       
-----------------------------------------------------------------------*/

VOID Draw3dRec(NativeControl ButtonWnd, HDC hdc, RC FAR *lprc, ICV icvUpperLeft, ICV icvLowerRight, DR3 dr3)
    {
    COLORREF cvSav;
    RC rc;

    cvSav = SetBkColor(hdc, clrt.rgcv[icvUpperLeft]);

    // top
    rc = *lprc;
    rc.yBot = rc.yTop+1;
    if (dr3 & DR3TOP)
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    // left
    rc.yBot = lprc->yBot;
    rc.xRight = rc.xLeft+1;
    if (dr3 & DR3LEFT)
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    if (icvUpperLeft != icvLowerRight)
        SetBkColor(hdc, clrt.rgcv[icvLowerRight]);

    // right
    rc.xRight = lprc->xRight;
    rc.xLeft = rc.xRight-1;
    if (dr3 & DR3RIGHT)
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    // bot
    if (dr3 & DR3BOT)
        {
        rc.xLeft = lprc->xLeft;
        rc.yTop = rc.yBot-1;
        if (dr3 & DR3HACKBOTRIGHT)
            rc.xRight -=2;
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);
        }

    SetBkColor(hdc, cvSav);
    }

static VOID DrawButtonBorder(NativeControl ButtonWnd, HDC hdc, RC FAR *lprc, int Fill)
    {
    COLORREF cvSav;
    RC rc;

    cvSav = SetBkColor(hdc, clrt.rgcv[ICVBTNFACE]);
	
    // top
	rc.yBot   = lprc->yTop + 1;
	rc.yTop   = lprc->yTop;
	rc.xLeft  = lprc->xLeft;
	rc.xRight = lprc->xRight - 1;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    // left
	rc.yBot   = lprc->yBot - 1;
	rc.yTop   = lprc->yTop + 1;
	rc.xLeft  =	lprc->xLeft;
	rc.xRight = lprc->xLeft + 1;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    SetBkColor(hdc, clrt.rgcv[ICVWINDOWFRAME]);

    // right
	rc.yBot   = lprc->yBot;
	rc.yTop   = lprc->yTop + 1;
	rc.xLeft  = lprc->xRight - 1;	
	rc.xRight = lprc->xRight;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    // bot
	rc.yBot   = lprc->yBot;
	rc.yTop   = lprc->yBot - 1;
	rc.xLeft  = lprc->xLeft;
	rc.xRight = lprc->xRight - 1;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) &rc, 
            (char far *) NULL, 0, (int far *) NULL);

    SetBkColor(hdc, cvSav);

	if(Fill)
		{
		rc.xLeft = lprc->xLeft + 1;
		rc.yTop = lprc->yTop + 1;
		rc.xRight = lprc->xRight - 1;
		rc.yBot = lprc->yBot - 1;
		Draw3dGreyBkgnd(ButtonWnd, hdc, &rc);
		} // if

    } // DrawButtonBorder

void Draw3dGreyBkgnd(NativeControl ButtonWnd, HDC hdc, RC FAR *prc)
{
COLORREF cvSav;

cvSav = SetBkColor(hdc, clrt.rgcv[ICVBTNFACE]);
ExtTextOut(hdc, 0, 0, ETO_OPAQUE, (LPRECT) prc, 
        (char far *) NULL, 0, (int far *) NULL);
SetBkColor(hdc, cvSav);

} // Draw3dGreyBkgnd


/*
VOID Draw3dInsetRect(NativeControl ButtonWnd, HDC hdc, RC FAR *prc, DR3 dr3)
    {
    RC rc;

    rc = *prc;
    Draw3dRec(ButtonWnd, hdc, &rc, ICVWINDOWFRAME, ICVBTNFACE, (WORD)(dr3 & DR3ALL));
    rc.xLeft--;
    rc.yTop--;
    rc.xRight++;
    rc.yBot++;
    Draw3dRec(ButtonWnd, hdc, &rc, ICVBTNSHADOW, ICVBTNHILITE, dr3);
    }
*/

VOID Draw3dButtonOut(NativeControl ButtonWnd, HDC hdc, RC FAR *prc, int Border, bool NewStyle, bool Hot, bool Enabled)
    {
    RC rc;
    RECT Rect;
    rc = *prc;
    Rect.bottom = rc.yBot;
    Rect.left   = rc.xLeft;
    Rect.right  = rc.xRight;
    Rect.top    = rc.yTop;

    if (NewStyle && g_xpStyle.IsAppThemed())
		{
        HTHEME hTheme = g_xpStyle.OpenThemeData(ButtonWnd, L"BUTTON");
        g_xpStyle.DrawThemeBackground(hTheme, hdc, BP_PUSHBUTTON, Enabled ? Hot ? PBS_HOT : PBS_NORMAL : PBS_DISABLED, &Rect, 0);
        g_xpStyle.CloseThemeData(hTheme);
		} // if
	else
		{
		if(Border)
			{
			DrawButtonBorder(ButtonWnd, hdc, prc, (Border & 0x02));

			rc.xLeft++;
			rc.yTop++;
			rc.xRight--;
			rc.yBot--;
			} // if
		Draw3dRec(ButtonWnd, hdc, &rc, ICVBTNHILITE, ICVBTNSHADOW, DR3ALL);
		} // else
    } // Draw3dButtonOut

VOID Draw3dButtonIn(NativeControl ButtonWnd, HDC hdc, RC FAR *prc, int Border, bool NewStyle, bool Hot, bool Enabled)
    {
    RC rc;

    RECT Rect;
    rc = *prc;
    Rect.bottom = rc.yBot;
    Rect.left   = rc.xLeft;
    Rect.right  = rc.xRight;
    Rect.top    = rc.yTop;
    if (NewStyle && g_xpStyle.IsAppThemed())
		{
        HTHEME hTheme = g_xpStyle.OpenThemeData(ButtonWnd, L"BUTTON");
        g_xpStyle.DrawThemeBackground(hTheme, hdc, BP_PUSHBUTTON, PBS_PRESSED | (Enabled ? (Hot ? PBS_HOT : 0) : PBS_DISABLED), &Rect, 0);
        g_xpStyle.CloseThemeData(hTheme);
		} // if
	else
		{
		if(Border)
			{
			DrawButtonBorder(ButtonWnd, hdc, prc, (Border & 0x02));
			Draw3dRec(ButtonWnd, hdc, &rc, ICVWINDOWFRAME, ICVBTNFACE, DR3LEFT | DR3TOP);
			Draw3dRec(ButtonWnd, hdc, &rc, ICVBTNFACE, ICVBTNFACE, DR3RIGHT | DR3BOT);

			rc.xLeft++;
			rc.yTop++;
			rc.xRight--;
			rc.yBot--;
			} // if

		Draw3dRec(ButtonWnd, hdc, &rc, ICVBTNSHADOW, ICVBTNHILITE, DR3ALL);
		} // else
    } // Draw3dButtonIn

/*-----------------------------------------------------------------------
|   Draw3dCtlColor
|
|       Common CTL_COLOR processor for 3d UITF dialogs & alerts.
|
|   Arguments:
|
|   Returns:
|       appropriate brush if f3dDialogs.  Returns FALSE otherwise
|
-----------------------------------------------------------------------*/
HBRUSH Draw3dCtlColor(UINT wm, WPARAM wParam, LPARAM lParam)
{
    if(f3dDialogs) {
            SetTextColor((HDC) wParam, clrt.rgcv[ICVBTNTEXT]);
            SetBkColor((HDC) wParam, clrt.rgcv[ICVBTNFACE]);
            return brt.mpicvhbr[ICVBTNFACE];
    }
    return (HBRUSH) FALSE;
}


/*-----------------------------------------------------------------------
|   Draw3dColorChange
|   
|       App calls this when it gets a WM_SYSCOLORCHANGE message
|       
|   Returns:
|       TRUE if successful.
|       
-----------------------------------------------------------------------*/
BOOL Draw3dColorChange(VOID)
    {
    return InternalDraw3dColorChange(FALSE);
    }


BOOL InternalDraw3dColorChange(BOOL fForce)
    {
    ICV icv;
    static CLRT clrtNew;
    static BRT brtNew;

    if (!f3dDialogs && !fForce)
        return FALSE;

    for (icv = 0; icv < ICVMAX; icv++)
        clrtNew.rgcv[icv] = GetSysColor(mpicvSysColor[icv]);

    //if (clrtNew.rgcv[ICVGRAYTEXT] == 0L || clrtNew.rgcv[ICVGRAYTEXT] == clrtNew.rgcv[ICVBTNFACE])
    //    clrtNew.rgcv[ICVGRAYTEXT] = WINGDI_RGB(0x80, 0x80, 0x80);
    //if (clrtNew.rgcv[ICVGRAYTEXT] == clrtNew.rgcv[ICVBTNFACE])
    //    clrtNew.rgcv[ICVGRAYTEXT] = 0L;

    if (fForce || memcmp(&clrt, &clrtNew, sizeof(CLRT))) {
        for (icv = 0; icv < ICVBRUSHMAX; icv++)
            brtNew.mpicvhbr[icv] = CreateSolidBrush(clrtNew.rgcv[icv]);

        for (icv = 0; icv < ICVBRUSHMAX; icv++) {
            if (brtNew.mpicvhbr[icv] == NULL) {
               for (icv = 0; icv < ICVBRUSHMAX; icv++)
                  DeleteObjectNull(&brtNew.mpicvhbr[icv]);
               return FALSE;
            }
        }

        DeleteObjects();
        brt = brtNew;
        clrt = clrtNew;
        return TRUE;
    }
        
    return TRUE;
}

