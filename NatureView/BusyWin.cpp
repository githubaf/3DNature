// BusyWin.cpp
// New-generation version of BusyWin, for NatureView

#include <windows.h>


#include "resource.h"
#include "BusyWin.h"
#include "CommCtrl.h"
#include "ToolTips.h"
#include "InstanceSupport.h"

extern NativeAnyWin ViewerNativeWindow;
extern ToolTipSupport *GlobalTipSupport;


NativeGUIWin BusyWin::DlgHandle = NULL;
bool BusyWin::Aborted = false;

BOOL CALLBACK BusyWinDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

switch(uMsg)
	{
	case WM_COMMAND:
		{
		WORD ID = LOWORD(wParam);
		if(HIWORD(wParam) == BN_CLICKED)
			{
			BusyWin::HandleEvent(ID); // so we can handle IDCANCEL before default handler does, below
			if(ID == IDCANCEL)
				{
				BusyWin::Abort();
				EndDialog(hwndDlg, 1);
				} // if
			} // if
		return(1);
		break;
		} // WM_INITDIALOG
	case WM_INITDIALOG:
		{
		int X, Y;
		RECT ViewerRect, DlgRect;
		HWND FullWindow;

		if(ViewerNativeWindow) FullWindow = ViewerNativeWindow;
		FullWindow = GetDesktopWindow();
		
		GetWindowRect(FullWindow, &ViewerRect);
		GetWindowRect(hwndDlg, &DlgRect);

		// Center
		X = ((ViewerRect.right - ViewerRect.left) / 2) - ((DlgRect.right - DlgRect.left) / 2);
		Y = ((ViewerRect.bottom - ViewerRect.top) / 2) - ((DlgRect.bottom - DlgRect.top) / 2);

		SetWindowPos(hwndDlg, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		return(0);
		break;
		} // WM_INITDIALOG
	} // switch

return(0);
} // BusyWinDialogProc





BusyWin::BusyWin(char *Title, char *InitialText, bool WithCancel, bool WithProgress, BusyStyle Style)
{
DlgHandle = NULL;
LPTSTR RCID;

Aborted = 0;

// Style is currently ignored

if(WithCancel)
	{
	RCID = MAKEINTRESOURCE(IDD_BUSYDIALOGCANCEL);
	} // if
else
	{
	RCID = MAKEINTRESOURCE(IDD_BUSYDIALOG);
	} // else

if(DlgHandle = CreateDialog(GetHInstance(), RCID, ViewerNativeWindow, BusyWinDialogProc))
	{
	SetWindowText(DlgHandle, Title);
	if(InitialText)
		{
		SetText(InitialText);
		} // if

	if(WithProgress)
		{
		SendMessage(GetDlgItem(DlgHandle, IDC_BUSYPROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(GetDlgItem(DlgHandle, IDC_BUSYPROGRESS), PBM_SETSTEP, (WPARAM) 1, 0);
		} // if
	else
		{
		ShowWindow(GetDlgItem(DlgHandle, IDC_BUSYPROGRESS), SW_HIDE);
		} // else

	if(GlobalTipSupport)
		{
		//GlobalTipSupport->AddTool(GetDlgItem(DlgHandle, IDC_HOME), IDS_HOMETIP);
		} // if

	ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
	} // if

ProcessAndHandleEvents();

} // BusyWin::BusyWin()


BusyWin::~BusyWin()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // BusyWin::~BusyWin()


int BusyWin::ProcessAndHandleEvents()
{
int Status = 0;
MSG msg;

while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	if(!IsDialogMessage(DlgHandle, &msg))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		} // else
	} // while


return(Status);
} // BusyWin::ProcessAndHandleEvents()


bool BusyWin::HandleEvent(WIDGETID WidgetID)
{
bool Handled = false;

switch(WidgetID)
	{
	case IDC_CANCELBUTTON: Aborted = 1; return(true);
	} // switch ID

return(Handled);
} // BusyWin::HandleEvent


int BusyWin::UpdateAndCheckAbort(float Percent)
{
int Step;

Step = (int)(100.0 * Percent);

SendMessage(GetDlgItem(DlgHandle, IDC_BUSYPROGRESS), PBM_SETPOS, (WPARAM) Step, 0);

return(CheckAbort());

} // BusyWin::UpdateAndCheckAbort

int BusyWin::CheckAbort(void)
{

return(Aborted);

} // BusyWin::CheckAbort

void BusyWin::SetText(char *NewText)
{
if(DlgHandle && NewText)
	{
	SetWindowText(GetDlgItem(DlgHandle, IDC_BUSYTEXT), NewText);
	} // if

} // BusyWin::SetText