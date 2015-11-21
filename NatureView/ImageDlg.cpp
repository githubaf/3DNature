// ImageDlg.cpp
// Code for image-display dialog

#include <windows.h>
#include <string>
#include "resource.h"
#include "ImageDlg.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "MediaSupport.h"
#include "Viewer.h"

NativeGUIWin ImageDlg::DlgHandle = NULL;

BOOL CALLBACK ImageDlgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

switch(uMsg)
	{
	case WM_NOTIFY:
		{
		short ButtonID;
		unsigned long Notify;
		NMHDR *nmhdr;
		ButtonID  = LOWORD(wParam);
		nmhdr    = (NMHDR *)lParam;
		Notify    = nmhdr->code;
/*
		switch(Notify)
			{
			default:
				{
				break;
				} // default;
			} // switch
*/
		break;
		} // WM_NOTIFY
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
} // ImageDlgDialogProc


void ImageDlg::Show(bool NewState)
{

if(DlgHandle)
	{
	if(NewState)
		{
		ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
		} // if
	else
		{
		ShowWindow(DlgHandle, SW_HIDE);
		} // else
	} // if

} // ImageDlg::Show


ImageDlg::ImageDlg(bool Show)
{
DlgHandle = NULL;

if(DlgHandle = CreateDialog(GetHInstance(), MAKEINTRESOURCE(IDD_DATAPANEL), GetGlobalViewerHWND(), ImageDlgDialogProc))
	{
	if(Show)
		{
		ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
		} // if
	} // if

} // ImageDlg::ImageDlg()


ImageDlg::~ImageDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // ImageDlg::~ImageDlg()


int ImageDlg::ProcessAndHandleEvents()
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
} // ImageDlg::ProcessAndHandleEvents()


int ImageDlg::SetDataText(const char *NewText)
{
SetWindowText(GetDlgItem(DlgHandle, IDC_DATATEXT), NewText);
Show(true);
return(1);
} // ImageDlg::SetDataText



