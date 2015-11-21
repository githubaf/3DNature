// DataDlg.cpp
// Code for Data dialog

#include <stdio.h>
#include <windows.h>
#include <richedit.h>
#include <string>
#include "resource.h"
#include "DataDlg.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "MediaSupport.h"
#include "Viewer.h"
#include "NVMiscGlobals.h"

NativeGUIWin DataDlg::DlgHandle = NULL;
HMODULE DataDlg::RichEditLib;

static bool SizeSearchInProgress = false;
static int SizeReqX = 0, SizeReqY = 0;

BOOL CALLBACK DataDlgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

switch(uMsg)
	{
	case WM_SIZE:
		{
		unsigned long int DeltaX, DeltaY;
		signed int NewW, NewH;
		RECT WinClient, Widget;
		POINT WidgetOrigin;
		GetClientRect(hwndDlg, &WinClient);
		GetWindowRect(GetDlgItem(hwndDlg, IDC_DATATEXT), &Widget);
		WidgetOrigin.x = Widget.left;
		WidgetOrigin.y = Widget.top;
		ScreenToClient(hwndDlg, &WidgetOrigin);
		// calculate new widget size
		DeltaX = WidgetOrigin.x;
		DeltaY = WidgetOrigin.y;
		NewW = WinClient.right  - (DeltaX + NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_X);
		NewH = WinClient.bottom - (DeltaY + NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_Y);
		SetWindowPos(GetDlgItem(hwndDlg, IDC_DATATEXT), NULL, 0, 0, NewW, NewH, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		return(0);
		break;
		} // SIZE
	case WM_NOTIFY:
		{
		short ButtonID;
		unsigned long Notify;
		NMHDR *nmhdr;
		ButtonID  = LOWORD(wParam);
		nmhdr    = (NMHDR *)lParam;
		Notify    = nmhdr->code;
		switch(Notify)
			{
			case EN_LINK:
				{
				ENLINK *EL;
				EL = (ENLINK *)lParam;
				if(EL->msg == WM_LBUTTONDOWN)
					{ // fetch text, open as URL
					TEXTRANGE Dest;
					if((EL->chrg.cpMax - EL->chrg.cpMin) < 4000)
						{
						char URL[4096];
						// copy range to text request structure
						Dest.chrg.cpMin = EL->chrg.cpMin;
						Dest.chrg.cpMax = EL->chrg.cpMax;
						Dest.lpstrText = URL;
						if(SendDlgItemMessage(hwndDlg, IDC_DATATEXT, EM_GETTEXTRANGE, 0, (WPARAM)&Dest))
							{
							OpenURLExternally(URL);
							} // if
						} // if
					} // if
				break;
				} // LINK
			case EN_REQUESTRESIZE:
				{
				REQRESIZE *NewWidgetSize = (REQRESIZE *)lParam;
				if(SizeSearchInProgress)
					{
					SizeReqX = NewWidgetSize->rc.right - NewWidgetSize->rc.left;
					SizeReqY = NewWidgetSize->rc.bottom - NewWidgetSize->rc.top;
					} // if
				else
					{
/*
					int DeltaX, DeltaY, WidgetW, WidgetH;
					unsigned long int ViewerW = 0, ViewerH = 0;
					RECT CurSize;
					GetClientRect(NewWidgetSize->nmhdr.hwndFrom, &CurSize);
					WidgetW = 5 + NewWidgetSize->rc.right;
					WidgetH = 7 + NewWidgetSize->rc.bottom;
	
					if(GetGlobalViewerDims(ViewerW, ViewerH) && ViewerW != 0 && ViewerH != 0)
						{ // rationalize auto window/widget size
						double SizeRatio;
						// conform width to no more than 1/4 viewer width
						SizeRatio = (double)WidgetW / ((double)ViewerW / 4.0f);
						if(SizeRatio > 1.0)
							{
							WidgetW = ViewerW / 4;
							WidgetH = (unsigned long int)(WidgetH * SizeRatio);
							} // if

						// conform height to no more than 1/3 viewer width
						SizeRatio = (double)WidgetH / ((double)ViewerH / 3.0f);
						if(SizeRatio > 1.0)
							{
							WidgetH = ViewerH / 3;
							// nothing else to adjust, we already adjusted width
							} // if

						} // if

	
					SetWindowPos(NewWidgetSize->nmhdr.hwndFrom, NULL, 0, 0, WidgetW, WidgetH, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
					// resize window now...
					WINDOWINFO WInfo;
					WInfo.cbSize = sizeof(WINDOWINFO);
					GetWindowInfo(hwndDlg, &WInfo);
					DeltaX = (WInfo.rcClient.left - WInfo.rcWindow.left) + (WInfo.rcWindow.right - WInfo.rcClient.right);
					DeltaY = (WInfo.rcClient.top - WInfo.rcWindow.top) + (WInfo.rcWindow.bottom - WInfo.rcClient.bottom);
					SetWindowPos(hwndDlg, NULL, 0, 0, WidgetW + DeltaX, WidgetH + DeltaY, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);

					// Now turn on word wrap, after we've clamped the width if too large...
					//SendDlgItemMessage(NewWidgetSize->nmhdr.hwndFrom, IDC_DATATEXT, EM_SETTARGETDEVICE, NULL, 0); // 0=enable word wrap
*/
					} // else

				} // EN_REQUESTRESIZE
			default:
				{
				break;
				} // default;
			} // switch
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
} // DataDlgDialogProc


void DataDlg::Show(bool NewState)
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

} // DataDlg::Show


DataDlg::DataDlg(bool Show)
{
DlgHandle = NULL;

if(!(RichEditLib = GetModuleHandle("RICHED20.DLL")))
	{
	RichEditLib = LoadLibrary("RICHED20.DLL"); // we never unload until exit
	} // if

if(DlgHandle = CreateDialog(GetHInstance(), MAKEINTRESOURCE(IDD_DATAPANEL), GetGlobalViewerHWND(), DataDlgDialogProc))
	{
	SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_AUTOURLDETECT, true, NULL); // enable auto-URL processing

   	if(Show)
		{
		ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
		} // if
	} // if

} // DataDlg::DataDlg()


DataDlg::~DataDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

if(RichEditLib)
	{
	FreeLibrary(RichEditLib);
	RichEditLib = NULL;
	} // if

} // DataDlg::~DataDlg()


int DataDlg::ProcessAndHandleEvents()
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
} // DataDlg::ProcessAndHandleEvents()



int DataDlg::SetDataText(const char *NewText)
{
SetWindowText(GetDlgItem(DlgHandle, IDC_DATATEXT), NewText);
ResizeToFit(); // shows window
return(1);
} // DataDlg::SetDataText



DWORD __stdcall EStreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
if(cb > 0)
	{
	FILE *InFile = (FILE *)dwCookie;
	size_t BytesRead;
	BytesRead = fread(pbBuff, 1, cb, InFile);
	if(BytesRead > 0)
		{
		*pcb = BytesRead;
		return(0);
		} // if
	} // if

return(1);
} // EStreamReadCallback

int DataDlg::SetDataTextFromFile(const char *Filename)
{
EDITSTREAM EStream;
FILE *InFile;
bool IsRTF = false, IsTxt = false;

if(!Filename) return(0);

if(strlen(Filename) > 4)
	{
	if(!stricmp(&Filename[strlen(Filename)] - 4, ".rtf"))
		{
		IsRTF = true;
		} // if
	if(!stricmp(&Filename[strlen(Filename)] - 4, ".txt"))
		{
		IsTxt = true;
		} // if
	if(!stricmp(&Filename[strlen(Filename)] - 4, ".csv"))
		{
		IsTxt = true;
		} // if
	if(!stricmp(&Filename[strlen(Filename)] - 4, ".asc"))
		{
		IsTxt = true;
		} // if
	} // if

if(!(IsTxt || IsRTF)) return(0);

if(InFile = fopen(Filename, "r"))
	{
	EStream.dwCookie = (DWORD_PTR)InFile;
	EStream.pfnCallback = &EStreamReadCallback;
	if(SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_STREAMIN, (IsRTF ? SF_RTF : SF_TEXT), (LPARAM)&EStream) > 0) // load file via callback
		{
		ResizeToFit(); // shows window
		} // if

	fclose(InFile);
	InFile = NULL;
	} // if

return(0);
} // DataDlg::SetDataTextFromFile


int DataDlg::ResizeToFit(void)
{
// perform sizing based on technique from http://www.codeproject.com/richedit/richeditsize.asp
// Of course, that page was full of errors...
SizeReqX = 0;
SizeReqY = 0;
SizeSearchInProgress = true;

SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_SETEVENTMASK, 0, ENM_LINK | ENM_REQUESTRESIZE);

unsigned long int ViewerW = 0, ViewerH = 0;
if(!(GetGlobalViewerDims(ViewerW, ViewerH) && ViewerW != 0 && ViewerH != 0))
	{
	ViewerW = GetSystemMetrics( SM_CXFULLSCREEN );
	} // if
int cyMin = 0, LastGoodX, LastGoodY;
// Start at quarter-size wide
int cxLast = ViewerW / 4;

while(1)
	{
	// Testing this guess
	RECT rc;
	rc.left = rc.top = 0;
	rc.right = cxLast;
	rc.bottom = 1;

	SetWindowPos(GetDlgItem(DlgHandle, IDC_DATATEXT), NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE);
	SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_SETRECT, 1, (LPARAM)&rc); // resize formatting rectangle
	SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_REQUESTRESIZE, 0, 0); // request resize

	// If it's the first time, take the result anyway.
	// This is the minimum height the control needs
	if(cyMin == 0)
		cyMin = SizeReqY;
		
	if(cxLast <= 0) // error safety
		{
		LastGoodX = 30;
		break;
		} // if

	// Iterating
	if(SizeReqY <= cyMin) // If the control didn't require a larger height, make it narrower until it does
		{
		LastGoodX = cxLast;
		LastGoodY = SizeReqY;
		cxLast -= 10;
		} // if
	else
		{
		break;
		} // else
	} // do

SizeSearchInProgress = false;

unsigned long int FinalWidgetW, FinalWidgetH;

FinalWidgetW = LastGoodX + NVW_DATADLG_WIDGET_INSIDE_MARGIN_X;
FinalWidgetH = LastGoodY + NVW_DATADLG_WIDGET_INSIDE_MARGIN_Y;
if(FinalWidgetH > ViewerH / 4)
	{
	FinalWidgetH = ViewerH / 4; // limit to 1/4 height initially
	} // if

// Resize Widget (which will destroy the SizeReq vars)
SetWindowPos(GetDlgItem(DlgHandle, IDC_DATATEXT), NULL, NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_X, NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_Y, FinalWidgetW, FinalWidgetH, SWP_NOZORDER|SWP_NOACTIVATE);

// resize window now...
WINDOWINFO WInfo;
int DeltaX, DeltaY;
WInfo.cbSize = sizeof(WINDOWINFO);
GetWindowInfo(DlgHandle, &WInfo);
DeltaX = (WInfo.rcClient.left - WInfo.rcWindow.left) + (WInfo.rcWindow.right - WInfo.rcClient.right);
DeltaY = (WInfo.rcClient.top - WInfo.rcWindow.top) + (WInfo.rcWindow.bottom - WInfo.rcClient.bottom);
SetWindowPos(DlgHandle, NULL, 0, 0, FinalWidgetW + DeltaX + NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_X,
 FinalWidgetH + DeltaY + NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_Y, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);


Show(true);
SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_SETSEL, -1, -1); // deselect all
SendDlgItemMessage(DlgHandle, IDC_DATATEXT, EM_HIDESELECTION, 1, 0);
return(1);
} // DataDlg::ResizeToFit



