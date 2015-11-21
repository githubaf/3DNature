// HelpDlg.cpp
// Code for help dialog

#include <windows.h>
#include <string>
#include "resource.h"
#include "HelpDlg.h"
#include "HelpSheet.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "Viewer.h"

NativeGUIWin HelpDlg::DlgHandle;

BOOL CALLBACK HelpDlgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

switch(uMsg)
	{
	case WM_KEYDOWN:
		{
		bool ControlQual, ShiftQual, AltQual;
		int nVirtKey = wParam;
		ControlQual = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 1 : 0);
		ShiftQual = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0);
		AltQual = ((GetAsyncKeyState(VK_MENU) & 0x8000) ? 1 : 0);
		return(NVEventHandler::HandleKey(nVirtKey, ControlQual, ShiftQual, AltQual));
		break;
		} // WM_INITDIALOG
	case WM_COMMAND:
		{
		WORD ID = LOWORD(wParam);
		if(HIWORD(wParam) == BN_CLICKED)
			{
			//if(ID == IDCANCEL)
				{
				//EndDialog(hwndDlg, 1);
				ShowWindow(hwndDlg, SW_HIDE); // don't delete, just hide
				} // else
			} // if
		return(1);
		break;
		} // WM_INITDIALOG
	case WM_INITDIALOG:
		{
		return(0);
		break;
		} // WM_INITDIALOG
	} // switch

return(0);
} // HelpDlgDialogProc


void HelpDlg::Show(bool NewState)
{

if(DlgHandle)
	{
	if(NewState)
		{
		ShowWindow(DlgHandle, SW_SHOW);
		} // if
	else
		{
		ShowWindow(DlgHandle, SW_HIDE);
		} // else
	} // if

} // HelpDlg::Show


HelpDlg::HelpDlg()
{
DlgHandle = NULL;
std::string HelpText;

if(DlgHandle = CreateDialog(GetHInstance(), MAKEINTRESOURCE(IDD_HELPPANEL), GetGlobalViewerHWND(), HelpDlgDialogProc))
	{
	// Configure text...
	HelpText.append("\n"); // blank line at top for spacing
	for(int HelpLine = 0; HelpSheet[HelpLine]; HelpLine++)
		{
		HelpText.append(HelpSheet[HelpLine]);
		HelpText.append("\n");
		} // for


	SetHelpText(HelpText.c_str());
	ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
	} // if

} // HelpDlg::HelpDlg()


HelpDlg::~HelpDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // HelpDlg::~HelpDlg()


int HelpDlg::ProcessAndHandleEvents()
{
int Status = 0;
/*
MSG msg;

while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	if(!IsDialogMessage(DlgHandle, &msg))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		} // else
	} // while
*/

return(Status);
} // HelpDlg::ProcessAndHandleEvents()


int HelpDlg::SetHelpText(const char *NewText)
{
SetWindowText(GetDlgItem(DlgHandle, IDC_HELPTEXT), NewText);
return(1);
} // HelpDlg::SetHelpText



