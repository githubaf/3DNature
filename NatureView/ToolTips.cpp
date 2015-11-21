// ToolTips.cpp
// Support code to enable and handle ToolTips

#include <sstream>
#include <windows.h>
#include <commctrl.h>
#include "ToolTips.h"
#include "InstanceSupport.h"
#include "Viewer.h"


ToolTipSupport::ToolTipSupport()
{
INITCOMMONCONTROLSEX ICCEX;

BalloonWidget = NULL;

ICCEX.dwSize = sizeof(INITCOMMONCONTROLSEX);
ICCEX.dwICC = ICC_BAR_CLASSES;
InitCommonControlsEx(&ICCEX);

TTWidget = CreateWindow(TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP | TTS_NOPREFIX ,
 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
 NULL, NULL, GetHInstance(), NULL);
SetWindowPos(TTWidget, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
SendMessage(TTWidget, TTM_ACTIVATE, 1, 0);

} // ToolTipSupport::ToolTipSupport



ToolTipSupport::~ToolTipSupport()
{

if(TTWidget) DestroyWindow(TTWidget);
TTWidget = NULL;

if(BalloonWidget) DestroyWindow(BalloonWidget);
BalloonWidget = NULL;

} // ToolTipSupport::~ToolTipSupport




int ToolTipSupport::AddTool(NativeAnyWin Control, unsigned long int StringID)
{
TOOLINFO HCti;

HCti.cbSize = sizeof(TOOLINFO);
HCti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
HCti.hwnd = Control;
HCti.uId = (UINT)Control;
HCti.hinst = GetHInstance();
HCti.lpszText = (char *)StringID;
SendMessage(TTWidget, TTM_ADDTOOL, 0, (LPARAM)&HCti);
return(1);
} // ToolTipSupport::AddTool


int ToolTipSupport::AddCallbackWin(NativeAnyWin Control)
{
TOOLINFO HCti;

HCti.cbSize = sizeof(TOOLINFO);
HCti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
HCti.hwnd = Control;
HCti.uId = (UINT)Control;
HCti.hinst = GetHInstance();
HCti.lpszText = (char *)LPSTR_TEXTCALLBACK;
SendMessage(TTWidget, TTM_ADDTOOL, 0, (LPARAM)&HCti);
return(1);
} // ToolTipSupport::AddCallbackWin



int ToolTipSupport::RemoveTool(NativeAnyWin Control)
{
TOOLINFO HCti;

HCti.cbSize = sizeof(TOOLINFO);
HCti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
HCti.hwnd = Control;
HCti.uId = (UINT)Control;
HCti.hinst = GetHInstance();
SendMessage(TTWidget, TTM_DELTOOL, 0, (LPARAM)&HCti);
return(1);
} // ToolTipSupport::RemoveTool



void ToolTipSupport::SetEnabled(bool NewEnabled)
{
SendMessage(TTWidget, TTM_ACTIVATE, NewEnabled, 0);
} // ToolTipSupport::SetEnabled




int ToolTipSupport::ShowBalloonTip(unsigned long int X, unsigned long int Y, const char *TipText, double DisplayTime)
{
int Success = 0;
TOOLINFO ti;

DisplayLength = DisplayTime / BalloonTimer.getSecondsPerTick(); // record time
StartMoment = BalloonTimer.tick();

BalloonText = TipText;

if(BalloonWidget)
	{
	// Destroy it
	if(BalloonWidget) DestroyWindow(BalloonWidget);
	BalloonWidget = NULL;
	} // if

if(!BalloonWidget)
	{
	BalloonWidget = CreateWindow(TOOLTIPS_CLASS, "", TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOANIMATE | TTS_NOFADE,
	 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetHInstance(), NULL);
	// sets a maximum width for the tool tip window (else it won't wrap lines at all)
	SendMessage (BalloonWidget, TTM_SETMAXTIPWIDTH, 0, 300);
	} // if
if (BalloonWidget)
	{
	memset (&ti, 0, sizeof (ti)); // mama says: reset your structure before touching it.
	ti.cbSize = sizeof (ti);
	ti.uFlags = TTF_TRACK | TTF_ABSOLUTE; // TTF_CENTERTIP;
	ti.hwnd = GetGlobalViewerHWND();
	ti.lpszText = (char *) BalloonText.c_str();
	GetClientRect (ti.hwnd, &ti.rect);
	SendMessage (BalloonWidget, TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM) MAKELONG(0, 0)); // show immed
	//SendMessage (BalloonWidget, TTM_SETDELAYTIME, TTDT_RESHOW, (LPARAM) MAKELONG(10000, 0)); // no reshow
	//SendMessage (BalloonWidget, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM) MAKELONG(5000, 0)); // hold for 5 seconds if nothing dismisses it
	//SendMessage (BalloonWidget, TTM_SETTITLE, 1, (LPARAM)"Info");
	// send an addtool message to the tooltip control window
	SendMessage (BalloonWidget, TTM_ADDTOOL, 0, (long) &ti);
	// I dunno why, but I must call this to display the window (it wont work without)
	SendMessage (BalloonWidget, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(X, Y));
	SendMessage (BalloonWidget, TTM_TRACKACTIVATE, TRUE, (long) &ti);
	} // if

return(Success);
} // ToolTipSupport::ShowBalloonTip


int ToolTipSupport::HideBalloonTip(void)
{
if(BalloonWidget)
	{
	DestroyWindow(BalloonWidget);
	BalloonWidget = NULL;
	} // if
return(0);
} // ToolTipSupport::HideBalloonTip 

int ToolTipSupport::HandleBalloonTipTimeout(void)
{
if(BalloonWidget)
	{
	osg::Timer_t NowMoment;
	NowMoment = BalloonTimer.tick();
	if(NowMoment - StartMoment > DisplayLength)
		{
		HideBalloonTip();
		} // if
	} // if
return(0);
} // ToolTipSupport::HandleBalloonTipTimeout

