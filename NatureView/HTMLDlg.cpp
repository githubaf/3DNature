// HTMLDlg.cpp
// Code for Data dialog
// Created from DataDlg on 2/14/05 by CXH

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <sstream>
#include <algorithm>

#include "resource.h"
#include "HTMLDlg.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "MediaSupport.h"
#include "Viewer.h"
#include "NVMiscGlobals.h"
#include "Credits.h"
#include "NVScene.h"

extern "C" {
#include "InternalBrowserSupport.h"
} // C

#define NVW_HTMLDLG_TOOLBAR_EXTRAMARGIN_Y	3

extern NVScene MasterScene;

// The class name of our Window to host the browser. It can be anything of your choosing.
//static const TCHAR ClassName[] = "NV Internal Browser";

NativeGUIWin HTMLDlg::DlgHandle = NULL;
bool HTMLDlg::ToolbarVisible = true;
bool HTMLDlg::CenterOnOpen = false;
std::string HTMLDlg::ForceTitle;

BOOL CALLBACK HTMLDlgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

switch(uMsg)
	{
/*
	case WM_MOUSEMOVE:
		{
		int xPos, yPos;
		int ToolMargin = 0, ShowState = SW_HIDE;
		RECT WindowBounds, ToolbarBounds;

		xPos = GET_X_LPARAM(lParam); 
		yPos = GET_Y_LPARAM(lParam);

		GetClientRect(hwndDlg, &WindowBounds);
		if(HTMLDlg::GetToolbarBounds(&ToolbarBounds))
			{
			if(xPos >= 0 && xPos <= WindowBounds.right && yPos >= 0 && yPos <= (ToolbarBounds.bottom * 2))
				{
				ShowState = SW_SHOWNOACTIVATE;
				} // if
			else
				{
				ShowState = SW_HIDE;
				} // else
			} // if

		ShowWindow(GetDlgItem(hwndDlg, IDC_BACK), ShowState);
		ShowWindow(GetDlgItem(hwndDlg, IDC_FWD), ShowState);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STOP), ShowState);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REFRESH), ShowState);
		ShowWindow(GetDlgItem(hwndDlg, IDC_EDIT), ShowState);
		return(0);
		} // WM_MOUSEMOVE
*/
/*
	case WM_CREATE:
		{
		// Success
		return(0);
		} // CREATE
	case WM_DESTROY:
		{
		return(TRUE);
		} // DESTROY
*/
	case WM_SIZE:
		{
		HTMLDlg::HandleResize(LOWORD(lParam), HIWORD(lParam));
		return(0);
		} // SIZE
/*
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
			default:
				{
				break;
				} // default;
			} // switch
		break;
		} // WM_NOTIFY
*/
/*
	case WM_KEYUP:
		{
		bool ControlQual, ShiftQual, AltQual;
		int nVirtKey = wParam;
		ControlQual = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 1 : 0);
		ShiftQual = ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0);
		AltQual = ((GetAsyncKeyState(VK_MENU) & 0x8000) ? 1 : 0);
		
		switch(nVirtKey)
			{
			default:
				{
				nVirtKey = 0;
				} // default
			} // switch
		break;
		} // KEYUP
*/
	case WM_COMMAND:
		{
		WORD ID = LOWORD(wParam);
		if(HIWORD(wParam) == BN_CLICKED)
			{
			switch(ID)
				{
				case IDC_BACK:
					{
					DoPageAction(hwndDlg, NVW_INTERNALBROWSERSUPPORT_GOBACK);
					break;
					} // BACK
				case IDC_FWD:
					{
					DoPageAction(hwndDlg, NVW_INTERNALBROWSERSUPPORT_GOFORWARD);
					break;
					} // FWD
				case IDC_STOP:
					{
					DoPageAction(hwndDlg, NVW_INTERNALBROWSERSUPPORT_STOP);
					break;
					} // STOP
				case IDC_REFRESH:
					{
					DoPageAction(hwndDlg, NVW_INTERNALBROWSERSUPPORT_REFRESH);
					break;
					} // IDC_REFRESH
				case IDC_EDIT:
					{
					HTMLDlg::DoNewURL();
					break;
					} // IDC_REFRESH
				case IDCANCEL:
					{
					//EndDialog(hwndDlg, 1);
					//ShowWindow(hwndDlg, SW_HIDE); // don't delete, just hide
					HTMLDlg::Show(false);
					break;
					} // IDC_REFRESH
				} // control ID
			} // if
		else
			{
			ID = ID;
			} // else
		return(1);
		} // WM_COMMAND
	case WM_INITDIALOG:
		{
		return(0);
		} // WM_INITDIALOG
	} // switch

return(0);
} // HTMLDlgDialogProc


void HTMLDlg::Show(bool NewState)
{

if(DlgHandle)
	{
	if(NewState)
		{
		if(CenterOnOpen)
			{
			int PrefWidth, PrefHeight, Left, Right, Top, Bottom;
			long X, Y;
			PrefWidth = GetGlobalViewerWidth() / 2;
			PrefHeight = GetGlobalViewerHeight() / 2;
			if(GetGlobalViewerCoords(X, Y))
				{
				Left = X + PrefWidth / 2;
				Top = Y + PrefHeight / 2;
				Right = X + (PrefWidth / 2) * 3;
				Bottom = Y + (PrefHeight / 2) * 3;
				// Move, Size and Show at once!
				SetWindowPos(DlgHandle, NULL, Left, Top, Right - Left, Bottom - Top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
				} // if
			SetCenterOnOpen(false);
			} // if
		ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
		} // if
	else
		{
		if(MasterScene.GetNoticeModal()) SetMovementLockoutGlobal(false); // ends a modal Notice window
		CleanupCredits(); // removes logo temporary files
		ShowWindow(DlgHandle, SW_HIDE);
		} // else
	} // if

} // HTMLDlg::Show


HTMLDlg::HTMLDlg(bool Show)
{
DlgHandle = NULL;

InitHTMLEmbed();

/*
ATOM WClass;
WNDCLASSEX		wc;

// Register the class of our window to host the browser. 'WindowProc' is our message handler
// and 'ClassName' is the class name. You can choose any class name you want.
ZeroMemory(&wc, sizeof(WNDCLASSEX));
wc.cbSize = sizeof(WNDCLASSEX);
wc.hInstance = GetHInstance();
wc.lpfnWndProc = (WNDPROC)HTMLDlgDialogProc;
wc.lpszClassName = &ClassName[0];
if(WClass = RegisterClassEx(&wc)) 
	{
	if(DlgHandle = CreateWindowEx(WS_EX_TOOLWINDOW,  MAKEINTATOM(WClass), "Web Browser", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, HWND_DESKTOP, NULL, GetHInstance(), 0))
*/
	if(DlgHandle = CreateDialog(GetHInstance(), MAKEINTRESOURCE(IDD_HTMLPANEL), GetGlobalViewerHWND(), HTMLDlgDialogProc))
		{
		SendDlgItemMessage(DlgHandle, IDC_BACK, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(GetHInstance(), MAKEINTRESOURCE(IDB_BACK)));
		SendDlgItemMessage(DlgHandle, IDC_FWD, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(GetHInstance(), MAKEINTRESOURCE(IDB_FWD)));
		SendDlgItemMessage(DlgHandle, IDC_STOP, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(GetHInstance(), MAKEINTRESOURCE(IDB_STOP)));
		SendDlgItemMessage(DlgHandle, IDC_REFRESH, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(GetHInstance(), MAKEINTRESOURCE(IDB_REFRESH)));
		// We'll do this later, so as not to steal focus on startup
		//CreateBrowserWidget();
   		if(Show)
			{
			//ShowWindow(DlgHandle, SW_SHOWNOACTIVATE);
			HTMLDlg::Show(true);
			} // if
		} // if

/*
	} // if
*/

} // HTMLDlg::HTMLDlg()


bool HTMLDlg::CreateBrowserWidget(void)
{
if(DlgHandle)
	{
	RECT WindowBounds, ToolbarBounds;
	GetClientRect(DlgHandle, &WindowBounds);
	if(GetToolbarBounds(&ToolbarBounds))
		{
		if(ToolbarVisible)
			{
			WindowBounds.top += ToolbarBounds.bottom + NVW_HTMLDLG_TOOLBAR_EXTRAMARGIN_Y;
			} // if
		} // if
	if(!EmbedBrowserObject(DlgHandle, &WindowBounds))
		{
		return(true);
		} // if
	} // if
return(false);
} // HTMLDlg::CreateBrowserWidget


void HTMLDlg::DestroyBrowserWidget(void)
{
if(DlgHandle)
	{
	UnEmbedBrowserObject(DlgHandle);
	SetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), "");
	} // if
} // HTMLDlg::DestroyBrowserWidget


void HTMLDlg::ShowToolbar(bool NewState)
{

if(DlgHandle)
	{
	RECT ClientRect;
	int ShowState = SW_HIDE;
	ToolbarVisible = NewState;
	if(ToolbarVisible)
		{
		ShowState = SW_SHOWNOACTIVATE;
		} // if
	else
		{
		ShowState = SW_HIDE;
		} // else

	ShowWindow(GetDlgItem(DlgHandle, IDC_BACK), ShowState);
	ShowWindow(GetDlgItem(DlgHandle, IDC_FWD), ShowState);
	ShowWindow(GetDlgItem(DlgHandle, IDC_STOP), ShowState);
	ShowWindow(GetDlgItem(DlgHandle, IDC_REFRESH), ShowState);
	ShowWindow(GetDlgItem(DlgHandle, IDC_EDIT), ShowState);

	GetClientRect(DlgHandle, &ClientRect);
	HandleResize(ClientRect.right, ClientRect.bottom);
	} // if

} // HTMLDlg::ShowToolbar


void HTMLDlg::HandleResize(int ClientW, int ClientH)
{
if(DlgHandle)
	{
	int ToolMargin = 0;
	HWND EditWnd = GetDlgItem(DlgHandle, IDC_EDIT);
	RECT WindowBounds, ToolbarBounds, EditBounds, BrowserWidget;
	POINT MapSet;
	GetClientRect(DlgHandle, &WindowBounds);
	GetWindowRect(EditWnd, &EditBounds); // GetClientRect gives us too small of dimensions on an EDIT
	if(GetToolbarBounds(&ToolbarBounds))
		{
		if(ToolbarVisible)
			{
			ToolMargin += ToolbarBounds.bottom + NVW_HTMLDLG_TOOLBAR_EXTRAMARGIN_Y;
			} // if
		} // if
	BrowserWidget.left =  0;
	BrowserWidget.top = ToolMargin;
	BrowserWidget.right = ClientW;
	BrowserWidget.bottom = ClientH - ToolMargin;
	ResizeBrowserRECT(DlgHandle, BrowserWidget);
	MapSet.x = 0; MapSet.y = 0;
	MapWindowPoints(EditWnd, DlgHandle, &MapSet, 1);
	SetWindowPos(EditWnd, NULL, 0, 0, WindowBounds.right - MapSet.x, EditBounds.bottom - EditBounds.top, SWP_NOMOVE|SWP_NOZORDER);
	} // if
} // HTMLDlg::HandleResize



HTMLDlg::~HTMLDlg()
{

if(DlgHandle)
	{
	DestroyBrowserWidget();
	DestroyWindow(DlgHandle);
	} // if
DlgHandle = NULL;

CleanupHTMLEmbed();
} // HTMLDlg::~HTMLDlg()



void HTMLDlg::DoNewURL(void)
{
char URLBuffer[2048];

if(DlgHandle)
	{
	GetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), URLBuffer, 2047);
	if(URLBuffer[0])
		{
		if(ValidateURL(URLBuffer))
			{
			DisplayHTMLPage(DlgHandle, (LPTSTR)URLBuffer);
			} // if
		} // if
	} // if

} // HTMLDlg::DoNewURL




int HTMLDlg::SetHTMLText(const char *NewText)
{
// clear any previous contents
DestroyBrowserWidget();
CreateBrowserWidget();
// Don't clear ForceTitle, we're probably using it

if(!NewText) return(0);

ShowToolbar(false);

DisplayHTMLStr(DlgHandle, NewText);

Show(true);
return(1);
} // HTMLDlg::SetHTMLText



int HTMLDlg::SetHTMLTextFromFile(const char *Filename)
{
//FILE *InFile;
bool IsHTML = false, IsTxt = false;
int Success = 0;

// clear any previous contents
DestroyBrowserWidget();
CreateBrowserWidget();
ClearForceTitle();

if(!Filename) return(0);

if(strlen(Filename) > 4)
	{
	if(ValidateURL(Filename))
		{
		IsHTML = true;
		} // if
	else if(!stricmp(&Filename[strlen(Filename)] - 4, ".txt"))
		{
		IsTxt = true;
		} // if
	} // if

if(!(IsTxt || IsHTML)) return(0);

//if(InFile = fopen(Filename, "r"))
	{
	//ResizeToFit(); // shows window
	//fclose(InFile);
	//InFile = NULL;
	if(IsHTML)
		{
		SetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), Filename);
		ShowToolbar(true);
		if(!DisplayHTMLPage(DlgHandle, (LPTSTR)Filename))
			{
			Success = 1;
			} // if
		} // if
	else if(IsTxt)
		{
		// do something that will ignore tags inline?
		SetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), Filename);
		if(!DisplayHTMLPage(DlgHandle, (LPTSTR)Filename))
			{
			Success = 1;
			} // if
		} // else if
	Show(true);
	} // if

return(Success);
} // HTMLDlg::SetHTMLTextFromFile



int HTMLDlg::SetImageFromFile(const char *Filename)
{
bool IsJPG = false, IsPNG = false, IsGIF = false;
int Success = 0;
std::string Path;

if(!DlgHandle)
	return(Success);

// clear any previous contents
DestroyBrowserWidget();
CreateBrowserWidget();
ClearForceTitle();

if(!Filename) return(0);

if(strlen(Filename) > 4)
	{
	// <<<>>> Could be dangerous
	//if(ValidateURL(Filename, true))
	if(!strnicmp(Filename, "file:///", 8))
		{
		// convert local relative paths to file:/// urls
		Path = Filename;
		} // if
	else
		{
		std::ostringstream TempImage;
		TempImage << "file:///" << MasterScene.GetSceneFileWorkingPath() << "/" << Filename;
		Path = TempImage.str();
		replace(Path.begin(), Path.end(), '\\', '/'); // change backslashes to forward slashes
		} // else
	if(1)
		{
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".jpg") || !stricmp(&Filename[strlen(Filename)] - 5, ".jpeg"))
			{
			IsJPG = true;
			} // if
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".png"))
			{
			IsPNG = true;
			} // if
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".gif"))
			{
			IsGIF = true;
			} // if
		} // if
	} // if

if(!(IsJPG || IsPNG || IsGIF)) return(0);

//if(InFile = fopen(Filename, "r"))
	{
	//ResizeToFit(); // shows window
	//fclose(InFile);
	//InFile = NULL;
	SetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), "");
	ShowToolbar(false);
	if(!DisplayHTMLPage(DlgHandle, (LPTSTR)Path.c_str())) // really displays image
		{
		Success = 1;
		} // if
	Show(true);
	} // if

return(Success);
} // HTMLDlg::SetImageFromFile



int HTMLDlg::SetMediaFromFile(const char *Filename)
{
bool IsAVI = false, IsMOV = false, IsMPG = false;
int Success = 0;

// clear any previous contents
DestroyBrowserWidget();
CreateBrowserWidget();
ClearForceTitle();

if(!Filename) return(0);

if(strlen(Filename) > 4)
	{
	// <<<>>> Could be dangerous
	//if(ValidateURL(Filename, true))
		{
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".mpg") || !stricmp(&Filename[strlen(Filename)] - 5, ".mpeg"))
			{
			IsMPG = true;
			} // if
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".mov"))
			{
			IsMOV = true;
			} // if
		if(!stricmp(&Filename[strlen(Filename)] - 4, ".avi"))
			{
			IsAVI = true;
			} // if
		} // if
	} // if

if(!(IsMPG || IsMOV || IsAVI)) return(0);

//if(InFile = fopen(Filename, "r"))
	{
	//ResizeToFit(); // shows window
	//fclose(InFile);
	//InFile = NULL;
	SetWindowText(GetDlgItem(DlgHandle, IDC_EDIT), "");
	ShowToolbar(false);
	if(!DisplayHTMLPage(DlgHandle, (LPTSTR)Filename)) // really displays image
		{
		Success = 1;
		} // if
	Show(true);
	} // if

return(Success);
} // HTMLDlg::SetMediaFromFile





bool HTMLDlg::GetToolbarBounds(RECT *BoundsOutput)
{
RECT LR;
if(DlgHandle && BoundsOutput)
	{
	GetClientRect(GetDlgItem(DlgHandle, IDC_BACK), BoundsOutput); // get UL
	GetClientRect(GetDlgItem(DlgHandle, IDC_EDIT), &LR); // get LR
	BoundsOutput->right = LR.right;
	BoundsOutput->bottom = LR.bottom;
	return(true);
	} // if

return(false);
} // HTMLDlg::GetToolbarBounds

// called only from StatusTextUpdateCallback, we're bridging between C and C++
// Ugly
HWND GetHTMLDlgHWND(void)
{
return(HTMLDlg::GetHWND());
} // GetHTMLDlgHWND

const char *GetHTMLDlgForceTitle(void)
{
return(HTMLDlg::GetForceTitle());
} // GetHTMLDlgForceTitle



// called from InternalBrowserSupport.c
// Fetches Title and status, and creates window title text
extern "C" {
void StatusTextUpdateCallback(LPCOLESTR pszStatusText)
{
char TitleBuffer[512], StatusText[512], FullTitle[1024], URLBuffer[1024];
char *DefaultTitle = "Web Browser";
int UseStatusText = 1;

TitleBuffer[0] = StatusText[0] = FullTitle[0] = URLBuffer[0] = NULL;

if(GetHTMLDlgForceTitle())
	{
	strcpy(FullTitle, GetHTMLDlgForceTitle());
	} // if
else
	{
	FetchTitle(GetHTMLDlgHWND(), TitleBuffer, 512);
	FetchURL(GetHTMLDlgHWND(), URLBuffer, 1024);
	WideCharToMultiByte(CP_ACP, 0, pszStatusText, -1, StatusText, 512, NULL, NULL);

	if(!strcmp(StatusText, "Done"))
		{
		UseStatusText = 0;
		} // if

	if(TitleBuffer[0])
		{
		if(UseStatusText)
			{
			sprintf(FullTitle, "%.500s: %.500s", TitleBuffer, StatusText);
			} // if
		else
			{
			sprintf(FullTitle, "%.500s", TitleBuffer);
			} // else
		} // if
	else
		{
		if(UseStatusText)
			{
			sprintf(FullTitle, "%.500s: %.500s", DefaultTitle, StatusText);
			} // if
		else
			{
			sprintf(FullTitle, "%.500s", DefaultTitle);
			} // else
		} // else
	} // else

SetWindowText(GetHTMLDlgHWND(), FullTitle);
SetWindowText(GetDlgItem(GetHTMLDlgHWND(), IDC_EDIT), URLBuffer);

return;
} // StatusTextUpdateCallback

} // extern "C"
