// FenetreWidget.cpp
// Widget-interaction abstraction layer for GUIFenetres.
// Implements many GUIFenetre methods found in Fenetre.h
// Built from scratch on 10/06/97 by Chris 'Xenon' Hanson
// Copyright 1997

#include "stdafx.h"
#include "Fenetre.h"
#include "WCSWidgets.h"

#ifdef _WIN32
long WINAPI FloatIntWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
#endif // _WIN32

// Here's the prototype from Fenetre.h:
// char WidgetSetDisabled(WIDGETID WidgetID, char Disabled, NativeGUIWin DestWin = NULL);
void GUIFenetre::WidgetSetDisabled(WIDGETID WidgetID, char Disabled, NativeGUIWin DestWin)
{
#ifdef _WIN32
char Result = 0;
HWND Wid;
unsigned long Style;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32

if(Wid = GetWidgetFromID(WidgetID))
	{
	Style = GetWindowLong(Wid, GWL_STYLE);
	Result = ((Style & WS_DISABLED) != 0);
	if(!Disabled)
		{
		EnableWindow(Wid, true);
		} // if
	else
		{
		EnableWindow(Wid, false);
		} // else
	if(Result != Disabled)
		{
		InvalidateRect(Wid, NULL, TRUE);
		} // if
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetDisabled

/*===========================================================================*/

void GUIFenetre::WidgetSetText(WIDGETID WidgetID, const char *Text, NativeGUIWin DestWin)
{
#ifdef _WIN32
//char Result = 0;
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

# ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_SETTEXT, NULL, (LPARAM)Text);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetText

/*===========================================================================*/

long GUIFenetre::WidgetGetText(WIDGETID WidgetID, int BufSize, char *Buffer, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, WM_GETTEXT, (WPARAM)BufSize, (LPARAM)Buffer);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetGetText

/*===========================================================================*/

void GUIFenetre::WidgetEMSetSelected(WIDGETID WidgetID, long StartSel, long EndSel, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, EM_SETSEL, StartSel, EndSel);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetEMSetSelected

/*===========================================================================*/

void GUIFenetre::WidgetEMSetReadOnly(WIDGETID WidgetID, int ReadOnly, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, EM_SETREADONLY, ReadOnly, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetEMSetReadOnly

/*===========================================================================*/

void GUIFenetre::WidgetEMScroll(WIDGETID WidgetID, long HScroll, long VScroll, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, EM_LINESCROLL, (WPARAM)HScroll, (LPARAM)VScroll);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetEMScroll

/*===========================================================================*/

void GUIFenetre::WidgetSetCheck(WIDGETID WidgetID, unsigned long State, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, BM_SETCHECK, State, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetCheck

/*===========================================================================*/

char GUIFenetre::WidgetGetCheck(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (char)SendMessage(Wid, BM_GETCHECK, 0, 0);
	} // if

#endif // _WIN32

return((char)Result);
} // GUIFenetre::WidgetGetCheck

/*===========================================================================*/

void GUIFenetre::WidgetSetStyle(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SetWindowLong(Wid, GWL_STYLE, Style);
	if(GetWindowLong(Wid, GWL_WNDPROC) == (long)FloatIntWndProc)
		{ // special handling...
		SendMessage(Wid, WM_WCSW_FI_SETUP, 0, (LPARAM)0);
		} // if
	WidgetRepaint(WidgetID, DestWin);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetStyle

/*===========================================================================*/

void GUIFenetre::WidgetSetStyleOn(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin)
{
#ifdef _WIN32
unsigned long Result = 0;
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = GetWindowLong(Wid, GWL_STYLE);
	Result |= Style;
	SetWindowLong(Wid, GWL_STYLE, Result);
	if(GetWindowLong(Wid, GWL_WNDPROC) == (long)FloatIntWndProc)
		{ // special handling...
		SendMessage(Wid, WM_WCSW_FI_SETUP, 0, (LPARAM)0);
		} // if
	WidgetRepaint(WidgetID, DestWin);
	} // if

#endif // _WIN32
} // GUIFenetre::WidgetSetStyleOn

/*===========================================================================*/

void GUIFenetre::WidgetSetStyleOff(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin)
{
#ifdef _WIN32
unsigned long Result = 0;
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = GetWindowLong(Wid, GWL_STYLE);
	Result &= ~Style;
	SetWindowLong(Wid, GWL_STYLE, Result);
	if(GetWindowLong(Wid, GWL_WNDPROC) == (long)FloatIntWndProc)
		{ // special handling...
		SendMessage(Wid, WM_WCSW_FI_SETUP, 0, (LPARAM)0);
		} // if
	WidgetRepaint(WidgetID, DestWin);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetStyleOff

/*===========================================================================*/

unsigned long GUIFenetre::WidgetGetStyle(WIDGETID WidgetID, NativeGUIWin DestWin)
{
unsigned long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = GetWindowLong(Wid, GWL_STYLE);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetGetStyle

/*===========================================================================*/

long GUIFenetre::WidgetSetScrollRange(WIDGETID WidgetID, long Min, long Max, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, SBM_SETRANGE, (WPARAM)Min, (LPARAM)Max);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetSetScrollRange

/*===========================================================================*/

void GUIFenetre::WidgetRepaint(WIDGETID WidgetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	InvalidateRect(DestWin, NULL, NULL);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetRepaint

/*===========================================================================*/

void GUIFenetre::WidgetShow(WIDGETID WidgetID, int Reveal, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	ShowWindow(Wid, Reveal ? SW_SHOW: SW_HIDE);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetShow

/*===========================================================================*/

int GUIFenetre::WidgetGetSize(WIDGETID WidgetID, int &Width, int &Height, NativeGUIWin DestWin)
{
int Success = 0;
#ifdef _WIN32
HWND Wid;
RECT Square;
RC Cola;
#endif // _WIN32

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	//ShowWindow(Wid, Reveal ? SW_SHOW: SW_HIDE);
	GetWindowRect(Wid, &Square);
	Cola.xLeft  = 0;
	Cola.yTop   = 0;
	Cola.xRight = Square.right - Square.left;
	Cola.yBot   = Square.bottom - Square.top;
	Width = Cola.xRight;
	Height = Cola.yBot;
	Success = 1;
	} // if

#endif // _WIN32

return(Success);

} // GUIFenetre::WidgetShow

/*===========================================================================*/

long GUIFenetre::WidgetSetScrollPos(WIDGETID WidgetID, long Pos, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, SBM_SETPOS, (WPARAM)Pos, (LPARAM)TRUE);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetSetScrollPos

/*===========================================================================*/

long GUIFenetre::WidgetSetScrollPosQuiet(WIDGETID WidgetID, long Pos, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, SBM_SETPOS, (WPARAM)Pos, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetSetScrollPosQuiet

/*===========================================================================*/

long GUIFenetre::WidgetGetScrollPos(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, SBM_GETPOS, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetGetScrollPos

/*===========================================================================*/

void GUIFenetre::WidgetFISync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_WCSW_FI_SYNC, Flags, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetFISync

/*===========================================================================*/

extern FloatIntConfig Cfg;

/*===========================================================================*/

void GUIFenetre::WidgetSmartRAHConfig(WIDGETID WidgetID, RasterAnimHost *ConfigRAH, RasterAnimHost *RAHParent, unsigned long Flags)
{

memset(&Cfg, 0, sizeof(struct FloatIntConfig));
Cfg.MasterVariable = ConfigRAH;
Cfg.AuxVariable = RAHParent;
Cfg.FIFlags = FIOFlag_RAH | FIOFlag_AnimDouble | Flags;
Cfg.MaxAmount = Cfg.MinAmount = Cfg.IncDecAmount = 0.0;
Cfg.CritID = NULL;

# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
# endif // _WIN32

} // GUIFenetre::WidgetSmartRAHConfig

/*===========================================================================*/

void GUIFenetre::WidgetSmartRAHConfig(WIDGETID WidgetID, RasterAnimHost **ConfigRAH, RasterAnimHost *RAHParent, unsigned long Flags)
{
memset(&Cfg, 0, sizeof(struct FloatIntConfig));
Cfg.MasterVariable = ConfigRAH;
Cfg.AuxVariable = RAHParent;
Cfg.FIFlags = FIOFlag_IRAH | FIOFlag_AnimDouble | Flags;
Cfg.MaxAmount = Cfg.MinAmount = Cfg.IncDecAmount = 0.0;
Cfg.CritID = NULL;

# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
# endif // _WIN32

} // GUIFenetre::WidgetSmartRAHConfig

/*===========================================================================*/

void GUIFenetre::WidgetFIConfig(WIDGETID WidgetID, void *MV, double IDA, double Min, double Max, unsigned long Flags, SetCritter *Crit, unsigned long SetGetID)
{
# ifdef _WIN32
HWND WidgetWND;

if(WidgetWND = GetWidgetFromID(WidgetID))
	{
	ConfigureFI(WidgetWND, NULL, MV, IDA, Min, Max, Flags, Crit, SetGetID);
	} // if
# endif // _WIN32
} // GUIFenetre::WidgetFIConfig

/*===========================================================================*/

void GUIFenetre::WidgetSNConfig(WIDGETID WidgetID, AnimDoubleTime *AD, unsigned long Flags)
{
// FIOFlag_AnimDouble

memset(&Cfg, 0, sizeof(struct FloatIntConfig));
Cfg.MasterVariable = AD;
Cfg.AuxVariable = NULL;
Cfg.FIFlags = FIOFlag_AnimDouble | Flags;
Cfg.MaxAmount = Cfg.MinAmount = Cfg.IncDecAmount = 0.0;
Cfg.CritID = NULL;

# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_FI_SETUP, 0, (LPARAM)&Cfg);
# endif // _WIN32

} // GUIFenetre::WidgetSNConfig

/*===========================================================================*/

long GUIFenetre::WidgetSNGetEditText(WIDGETID WidgetID, int BufSize, char *Buffer, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, WM_WCSW_FI_OPT1, (WPARAM)BufSize, (LPARAM)Buffer);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetSNGetEditText

/*===========================================================================*/

double GUIFenetre::WidgetSNGetPrevValue(void)
{

return(GetSNPrevValue());

} // GUIFenetre::WidgetSNGetPrevValue

/*===========================================================================*/

void GUIFenetre::WidgetAGConfig(WIDGETID WidgetID, AnimGradient *AG)
{

# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_AG_SETUP, 0, (LPARAM)AG);
# endif // _WIN32

} // WidgetAGConfig

/*===========================================================================*/

void GUIFenetre::WidgetAGSync(WIDGETID WidgetID)
{
#ifdef _WIN32
HWND Wid;
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_WCSW_AG_SYNC, 0, 0);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetAGSync

/*===========================================================================*/

void GUIFenetre::WidgetSCSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_WCSW_SC_SYNC, Flags, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSCSync

/*===========================================================================*/

void GUIFenetre::WidgetSCConfig(WIDGETID WidgetID, unsigned long Flags, SetCritter *Crit, unsigned long SetGetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureSC(Wid, NULL, NULL, Flags, Crit, SetGetID);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSCConfig

/*===========================================================================*/

void GUIFenetre::WidgetSCConfig(WIDGETID WidgetID, void *MV, unsigned long Flags, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureSC(Wid, NULL, MV, Flags, NULL, NULL);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSCConfig

/*===========================================================================*/

void GUIFenetre::WidgetLWConfig(WIDGETID WidgetID, GeneralEffect *Component, Database *DatabaseHost, EffectsLib *EffectsHost, unsigned long int NewQueryFlags)
{
LinkWidgetInstance Instance;
# ifdef _WIN32
Instance.DBHost = DatabaseHost;
Instance.ComponentHost = Component;
Instance.EffectsHost = EffectsHost;
Instance.NewQueryFlags = NewQueryFlags;
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_LW_SETUP, 0, (LPARAM)&Instance);
# endif // _WIN32
} // WidgetLWConfig

/*===========================================================================*/

void GUIFenetre::WidgetLWSync(WIDGETID WidgetID)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_LW_SYNC, 0, 0);
# endif // _WIN32
} // WidgetLWSync

/*===========================================================================*/

void GUIFenetre::WidgetGWConfig(WIDGETID WidgetID, GridWidgetInstance *Instance)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_GW_SETUP, 0, (LPARAM)Instance);
# endif // _WIN32
} // WidgetGWConfig

/*===========================================================================*/

void GUIFenetre::WidgetGWSetStyle(WIDGETID WidgetID, unsigned long GWStyle)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_GW_SETSTYLE, 0, (LPARAM)GWStyle);
# endif // _WIN32
} // WidgetGWConfig

/*===========================================================================*/

void GUIFenetre::WidgetTCConfig(WIDGETID WidgetID, TextColumnMarkerWidgetInstance *Instance)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_TC_SETUP, 0, (LPARAM)Instance);
# endif // _WIN32
} // WidgetTCConfig

/*===========================================================================*/

void GUIFenetre::WidgetTCSetText(WIDGETID WidgetID, const char *NewText)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_TC_SETTEXT, 0, (LPARAM)NewText);
# endif // _WIN32
} // WidgetTCSetText

/*===========================================================================*/

void GUIFenetre::WidgetTCGetData(WIDGETID WidgetID, std::vector<unsigned long int> *DestinationContainer)
{
# ifdef _WIN32
SendMessage(GetWidgetFromID(WidgetID), WM_WCSW_TC_GETDATA, 0, (LPARAM)DestinationContainer);
# endif // _WIN32
} // GUIFenetre::WidgetTCGetData

/*===========================================================================*/

void GUIFenetre::WidgetSRSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_WCSW_SR_SYNC, Flags, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSRSync

/*===========================================================================*/

void GUIFenetre::WidgetDDSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, WM_WCSW_DD_SYNC, Flags, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetDDSync

/*===========================================================================*/

void GUIFenetre::WidgetCBSetUIMode(WIDGETID WidgetID, char Mode, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_SETEXTENDEDUI, Mode, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetCBSetUIMode

/*===========================================================================*/

void GUIFenetre::WidgetCBClear(WIDGETID WidgetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetCBClear

/*===========================================================================*/

long GUIFenetre::WidgetCBInsert(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, CB_INSERTSTRING, (WPARAM)Entry, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetCBInsert

/*===========================================================================*/

long GUIFenetre::WidgetCBAddEnd(WIDGETID WidgetID, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, CB_ADDSTRING, 0, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetCBAddEnd

/*===========================================================================*/

void GUIFenetre::WidgetCBDelete(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_DELETESTRING, (WPARAM)Entry, (LPARAM)NULL);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetCBDelete

/*===========================================================================*/

long GUIFenetre::WidgetCBReplace(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_DELETESTRING, (WPARAM)Entry, (LPARAM)NULL);
	Result = (long)SendMessage(Wid, CB_INSERTSTRING, (WPARAM)Entry, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetCBReplace

/*===========================================================================*/

long GUIFenetre::WidgetCBStuffStrings(WIDGETID WidgetID, NativeGUIWin DestWin, ...)
{
long Result = 0;
va_list VarA;
char *StringArg;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	va_start(VarA, DestWin);
	while(StringArg = va_arg(VarA, char *))
		{
		Result = (long)SendMessage(Wid, CB_ADDSTRING, 0, (LPARAM)StringArg);
		} // while
	va_end(VarA);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetCBStuffStrings

/*===========================================================================*/

void GUIFenetre::WidgetCBSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
//char Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_SETCURSEL, (WPARAM)Current, (LPARAM)0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetCBSetCurSel

/*===========================================================================*/

long GUIFenetre::WidgetCBGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, CB_GETCURSEL, 0, 0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetCBGetCurSel

/*===========================================================================*/

void GUIFenetre::WidgetCBSetItemData(WIDGETID WidgetID, long Current, void *ItemData, NativeGUIWin DestWin)
{
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, CB_SETITEMDATA, (WPARAM)Current, (LPARAM)ItemData);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetCBSetItemData

/*===========================================================================*/

void *GUIFenetre::WidgetCBGetItemData(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
void *Result = NULL;
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if
#ifdef _WIN32

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (void *)SendMessage(Wid, CB_GETITEMDATA, Current, 0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetCBGetItemData

/*===========================================================================*/

long GUIFenetre::WidgetCBGetCount(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetCBGetCount

/*===========================================================================*/

long GUIFenetre::WidgetCBGetText(WIDGETID WidgetID, long Entry, char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, CB_GETLBTEXT, (WPARAM)Entry, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetCBGetText

/*===========================================================================*/

long GUIFenetre::WidgetLBInsert(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_INSERTSTRING, (WPARAM)Entry, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBInsert

/*===========================================================================*/

void GUIFenetre::WidgetLBDelete(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_DELETESTRING, (WPARAM)Entry, (LPARAM)NULL);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBDelete

/*===========================================================================*/

long GUIFenetre::WidgetLBReplace(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_DELETESTRING, (WPARAM)Entry, (LPARAM)NULL);
	Result = (long)SendMessage(Wid, LB_INSERTSTRING, (WPARAM)Entry, (LPARAM)String);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBReplace

/*===========================================================================*/

long GUIFenetre::WidgetLBGetSelItems(WIDGETID WidgetID, long ArraySize, LONG *SelItems,
                                     NativeGUIWin DestWin)
{
long Result = 0;
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

Wid = GetWidgetFromID(WidgetID);

#ifdef _WIN32
if (Wid)
   Result = (long)SendMessage(Wid, LB_GETSELITEMS, (WPARAM)ArraySize, (LPARAM)SelItems);
#endif // _WIN32

return Result;

} // GUIFenetre::WidgetLBGetSelItems

/*===========================================================================*/

long GUIFenetre::WidgetLBStuffStrings(WIDGETID WidgetID, NativeGUIWin DestWin, ...)
{
long Result = 0;
va_list VarA;
char *StringArg;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	va_start(VarA, DestWin);
	while(StringArg = va_arg(VarA, char *))
		{
		Result = (long)SendMessage(Wid, LB_ADDSTRING, 0, (LPARAM)StringArg);
		} // while
	va_end(VarA);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBStuffStrings

/*===========================================================================*/

long GUIFenetre::WidgetLBGetText(WIDGETID WidgetID, long Entry, char *Buffer, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_GETTEXT, (WPARAM)Entry, (LPARAM)Buffer);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetLBGetText

/*===========================================================================*/

void GUIFenetre::WidgetLBClear(WIDGETID WidgetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBClear

/*===========================================================================*/

long GUIFenetre::WidgetLBGetCount(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBGetCount

/*===========================================================================*/

long GUIFenetre::WidgetLBGetSelCount(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_GETSELCOUNT, (WPARAM)0, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBGetSelCount

/*===========================================================================*/

long GUIFenetre::WidgetLBSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_SETCURSEL, (WPARAM)Current, (LPARAM)NULL);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetLBSetCurSel

/*===========================================================================*/

void GUIFenetre::WidgetLBClearSel(WIDGETID WidgetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_SETSEL, (WPARAM)0, (LPARAM)-1);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBClearSel

/*===========================================================================*/

long GUIFenetre::WidgetLBGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (long)SendMessage(Wid, LB_GETCURSEL, 0, 0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBGetCurSel

/*===========================================================================*/

void GUIFenetre::WidgetLBSetSelState(WIDGETID WidgetID, long State, long Current, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_SETSEL, (LPARAM)State, (WPARAM)Current);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBGetSelState

/*===========================================================================*/

char GUIFenetre::WidgetLBGetSelState(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
char Result = 0;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (char)SendMessage(Wid, LB_GETSEL, (WPARAM)Current, (LPARAM)0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetLBGetSelState

/*===========================================================================*/

void GUIFenetre::WidgetLBSetItemData(WIDGETID WidgetID, long Current, void *ItemData, NativeGUIWin DestWin)
{
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_SETITEMDATA, (WPARAM)Current, (LPARAM)ItemData);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBSetItemData

/*===========================================================================*/

void *GUIFenetre::WidgetLBGetItemData(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
void *Result = NULL;
NativeControl Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if
#ifdef _WIN32

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (void *)SendMessage(Wid, LB_GETITEMDATA, Current, 0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetLBGetItemData

/*===========================================================================*/

void GUIFenetre::WidgetLBSetHorizExt(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, LB_SETHORIZONTALEXTENT, (WPARAM)Current, (LPARAM)NULL);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetLBSetHorizExt

/*===========================================================================*/

void GUIFenetre::WidgetSetModified(WIDGETID WidgetID, char Modified, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid, EM_SETMODIFY, (WPARAM)(int)Modified, 0);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetSetModified

/*===========================================================================*/

char GUIFenetre::WidgetGetModified(WIDGETID WidgetID, NativeGUIWin DestWin)
{
char Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (char)SendMessage(Wid, EM_GETMODIFY, 0, 0);
	} // if

#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetGetModified

/*===========================================================================*/

void GUIFenetre::WidgetTVDeleteAll(WIDGETID WidgetID, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	SendMessage(Wid,  TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
	} // if

#endif // _WIN32

} // GUIFenetre::WidgetTVDeleteAll

/*===========================================================================*/

long GUIFenetre::WidgetTVGetVisibleCount(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TVM_GETVISIBLECOUNT, 0, 0);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetVisibleCount

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetFirstVisible(WIDGETID WidgetID, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (void *)SendMessage(Wid, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, NULL);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetFirstVisible

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetNextVisible(WIDGETID WidgetID,  void *CurVis, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = (void *)SendMessage(Wid, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)CurVis);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetNextVisible

/*===========================================================================*/

void GUIFenetre::WidgetTVSetItemData(WIDGETID WidgetID, void *Item, void *ItemData, NativeGUIWin DestWin)
{
#ifdef _WIN32
TV_ITEM MyTVItem;

HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_PARAM;
	MyTVItem.hItem = (HTREEITEM)Item;

	SendMessage(Wid, TVM_SETITEM, 0, (LPARAM)&MyTVItem);
	} // if

#endif // _WIN32
} // GUIFenetre::WidgetTVSetItemData

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetItemData(WIDGETID WidgetID, void *Item, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
TV_ITEM MyTVItem;

HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_PARAM;
	MyTVItem.hItem = (HTREEITEM)Item;

	SendMessage(Wid, TVM_GETITEM, 0, (LPARAM)&MyTVItem);
	Result = (void *)MyTVItem.lParam;
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTVGetItemData

/*===========================================================================*/

void GUIFenetre::WidgetTVSetItemSelected(WIDGETID WidgetID, void *Item, char SelState, NativeGUIWin DestWin)
{
#ifdef _WIN32
TV_ITEM MyTVItem;
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_STATE;
	MyTVItem.stateMask = TVIS_SELECTED;
	MyTVItem.hItem = (HTREEITEM)Item;

	MyTVItem.state = SelState ? TVIS_SELECTED: 0;
	SendMessage(Wid, TVM_SETITEM, 0, (LPARAM)&MyTVItem);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetTVSetItemSelected

/*===========================================================================*/

char GUIFenetre::WidgetTVGetItemSelected(WIDGETID WidgetID, void *Item, NativeGUIWin DestWin)
{
char SelState = 0;
#ifdef _WIN32
TV_ITEM MyTVItem;

HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_STATE;
	MyTVItem.hItem = (HTREEITEM)Item;

	SendMessage(Wid, TVM_GETITEM, 0, (LPARAM)&MyTVItem);
	SelState = ((MyTVItem.state & TVIS_SELECTED) ? 1 : 0);
	} // if

#endif // _WIN32
return(SelState);

} // GUIFenetre::WidgetTVGetItemSelected

/*===========================================================================*/

void GUIFenetre::WidgetTVSetBoldState(WIDGETID WidgetID, void *Item, char BoldState, NativeGUIWin DestWin)
{
#ifdef _WIN32
TV_ITEM MyTVItem;
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_STATE;
	MyTVItem.stateMask = TVIS_BOLD;
	MyTVItem.hItem = (HTREEITEM)Item;

	MyTVItem.state = BoldState ? TVIS_BOLD: 0;
	SendMessage(Wid, TVM_SETITEM, 0, (LPARAM)&MyTVItem);
	} // if
#endif // _WIN32

} // GUIFenetre::WidgetTVSetBoldState

/*===========================================================================*/

void GUIFenetre::WidgetTVSetItemText(WIDGETID WidgetID, void *Item, char *ItemText, NativeGUIWin DestWin)
{
#ifdef _WIN32
TV_ITEM MyTVItem;
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_TEXT;
	MyTVItem.hItem = (HTREEITEM)Item;

	MyTVItem.pszText = ItemText;
	SendMessage(Wid, TVM_SETITEM, 0, (LPARAM)&MyTVItem);
	} // if
#endif // _WIN32

} // GUIFenetre::WidgetTVSetItemText

/*===========================================================================*/

long GUIFenetre::WidgetTVGetItemText(WIDGETID WidgetID, void *Item, char *TextBuffer, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
TV_ITEM MyTVItem;
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&MyTVItem, 0, sizeof MyTVItem);
	MyTVItem.mask = TVIF_TEXT;
	MyTVItem.hItem = (HTREEITEM)Item;

	MyTVItem.pszText = TextBuffer;
	MyTVItem.cchTextMax = 255;
	SendMessage(Wid, TVM_GETITEM, 0, (LPARAM)&MyTVItem);
	} // if
#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetItemText

/*===========================================================================*/

#ifdef _WIN32
TV_INSERTSTRUCT TVInsert;
#endif // _WIN32

/*===========================================================================*/

void *GUIFenetre::WidgetTVInsert(WIDGETID WidgetID, char *ItemText, void *ItemData, void *ItemParent, char Children, NativeGUIWin DestWin)
{
void *Result = NULL;

#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&TVInsert, 0, sizeof(TV_INSERTSTRUCT));
	TVInsert.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	TVInsert.item.stateMask = 0;
	TVInsert.item.state = 0;
	TVInsert.item.pszText = ItemText;
	TVInsert.hParent = (HTREEITEM)ItemParent;
	TVInsert.hInsertAfter = TVI_LAST;
	TVInsert.item.cChildren = Children;
	TVInsert.item.lParam = (long)ItemData;

	Result = (void *)SendMessage(Wid, TVM_INSERTITEM, 0, (LPARAM)&TVInsert);
	} // if
#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetTVInsert

/*===========================================================================*/

void *GUIFenetre::WidgetTVInsertExpanded(WIDGETID WidgetID, char *ItemText, void *ItemData, void *ItemParent, char Children, char Expanded, NativeGUIWin DestWin)
{
void *Result = NULL;

#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	memset(&TVInsert, 0, sizeof(TV_INSERTSTRUCT));
	TVInsert.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	TVInsert.item.stateMask = TVIF_STATE | TVIS_EXPANDED;
	TVInsert.item.state = Expanded ? TVIS_EXPANDED : 0;
	TVInsert.item.pszText = ItemText;
	TVInsert.hParent = (HTREEITEM)ItemParent;
	TVInsert.hInsertAfter = TVI_LAST;
	TVInsert.item.cChildren = Children;
	TVInsert.item.lParam = (long)ItemData;

	Result = (void *)SendMessage(Wid, TVM_INSERTITEM, 0, (LPARAM)&TVInsert);
	} // if
#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetTVInsertExpanded

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetRoot(WIDGETID WidgetID, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = TreeView_GetRoot(Wid);
	} // if
#endif // _WIN32

return(Result);
} // GUIFenetre::WidgetTVGetRoot

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetChild(WIDGETID WidgetID, void *CurItem, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = TreeView_GetChild(Wid, CurItem);
	} // if
#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetChild

/*===========================================================================*/

void *GUIFenetre::WidgetTVGetNextSibling(WIDGETID WidgetID, void *CurItem, NativeGUIWin DestWin)
{
void *Result = NULL;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = TreeView_GetNextSibling(Wid, CurItem);
	} // if
#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTVGetNextSibling

/*===========================================================================*/

bool GUIFenetre::WidgetTVEnsureVisible(WIDGETID WidgetID, void *CurVis, NativeGUIWin DestWin)
{
bool Result = false;
#ifdef _WIN32
HWND Wid;
#endif // _WIN32

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

#ifdef _WIN32
if(Wid = GetWidgetFromID(WidgetID))
	{
	// returns non-NULL if tree had to be scrolled to make the item visible
	Result = TreeView_EnsureVisible(Wid, CurVis) ? true: false;
	} // if
#endif // _WIN32

return (Result);
} // GUIFenetre::WidgetTVEnsureVisible

/*===========================================================================*/


long GUIFenetre::WidgetTCInsertItem(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
TC_ITEM TCItem;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	TCItem.mask = TCIF_TEXT;
	TCItem.pszText = (char *)String;
	Result = SendMessage(Wid, TCM_INSERTITEM, Entry, (LPARAM)&TCItem);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCInsertItem

/*===========================================================================*/

long GUIFenetre::WidgetTCSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TCM_SETCURSEL, Current, 0);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCSetCurSel

/*===========================================================================*/

long GUIFenetre::WidgetTCDeleteItem(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TCM_DELETEITEM, Entry, 0);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCDeleteItem

/*===========================================================================*/

long GUIFenetre::WidgetTCClear(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TCM_DELETEALLITEMS, 0, 0);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCClear

/*===========================================================================*/

long GUIFenetre::WidgetTCGetItemCount(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TCM_GETITEMCOUNT, 0, 0);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCGetItemCount

/*===========================================================================*/

long GUIFenetre::WidgetTCGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, TCM_GETCURSEL, 0, 0);
	} // if

#endif // _WIN32
return(Result);
} // GUIFenetre::WidgetTCGetCurSel

/*===========================================================================*/

long GUIFenetre::WidgetTCGetItemText(WIDGETID WidgetID, long Entry, char *String, long MaxTextLen, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;
TC_ITEM TCItem;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	TCItem.mask = TCIF_TEXT;
	TCItem.pszText = (char *)String;
	TCItem.cchTextMax = MaxTextLen;
	Result = SendMessage(Wid, TCM_GETITEM, Entry, (LPARAM)&TCItem);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetTCGetItemText

/*===========================================================================*/

long GUIFenetre::WidgetUDSetPos(WIDGETID WidgetID, long Current, NativeGUIWin DestWin)
{
long Result = 0;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = SendMessage(Wid, UDM_SETPOS, 0, MAKELONG((short)Current, 0));
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetUDSetPos

/*===========================================================================*/

void GUIFenetre::WidgetColumnedListViewSetActiveColumn(WIDGETID WidgetID, signed long ActiveCol, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	ColumnedListViewSetActiveColumn(Wid, ActiveCol);
	} // if

#endif // _WIN32
} // GUIFenetre::WidgetColumnedListViewSetActiveColumn

/*===========================================================================*/

long GUIFenetre::WidgetColumnedListViewGetActiveColumn(WIDGETID WidgetID, NativeGUIWin DestWin)
{
long Result = -1;
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	Result = ColumnedListViewGetActiveColumn(Wid);
	} // if

#endif // _WIN32

return(Result);

} // GUIFenetre::WidgetColumnedListViewGetActiveColumn

void GUIFenetre::WidgetTBConfig(WIDGETID WidgetID, int Normal, int Select, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureTB(Wid, NULL, Normal, Select);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetTBConfig


void GUIFenetre::WidgetTBConfigThumb(WIDGETID WidgetID, Raster *Thumb, NativeGUIWin DestWin)
{
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureTB(Wid, NULL, NULL, NULL, Thumb);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetTBConfigThumb


void GUIFenetre::WidgetSRConfig(WIDGETID GroupWidgetID, WIDGETID WidgetID, void *MV, unsigned long Flags, signed long SetVal, NativeGUIWin DestWin)
{ // GroupWidgetID is unused currently
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureSR(DestWin, GroupWidgetID, WidgetID, MV, Flags, SetVal);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetSRConfig


void GUIFenetre::WidgetSRConfigCritter(WIDGETID GroupWidgetID, WIDGETID WidgetID, unsigned long Flags, signed long SetVal, SetCritter *Crit, unsigned long SetGetID, NativeGUIWin DestWin)
{ // GroupWidgetID is unused currently
#ifdef _WIN32
HWND Wid;

if(DestWin == NULL)
	{
	DestWin = NativeWin;
	} // if

if(Wid = GetWidgetFromID(WidgetID))
	{
	ConfigureSR(DestWin, GroupWidgetID, WidgetID, NULL, Flags, SetVal, Crit, SetGetID);
	} // if
#endif // _WIN32
} // GUIFenetre::WidgetSRConfig
