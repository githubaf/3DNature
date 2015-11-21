// NVWidgets.cpp
// Oh, shocker, I have to write custom widgets to get around limitations in Windows' basic widget set
// Created originally from parts of WCSWidgets on 2/15/05 by CXH

#include <windows.h>
#include "NVWidgets.h"
#include "IdentityDefines.h"

long FAR PASCAL ModifiedEditWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);

static WNDPROC EditWndProc;

// Intercept a few messages for dialog-tabbing
long FAR PASCAL ModifiedEditWndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{

switch(message)
	{
	case WM_GETDLGCODE:
		{
		return(DLGC_WANTALLKEYS);
		} // GETDLGCODE
	case WM_KEYUP:
	case WM_CHAR:
		{
		if((int)wParam == VK_RETURN)
			{
			return(0);
			} // if
		} // RETURN-EATER
	case WM_KEYDOWN:
		{
		if((int)wParam == VK_RETURN)
			{
			// forge a BN_CLICKED
			PostMessage(GetParent(hwnd), WM_COMMAND, ((UINT)GetWindowLong(hwnd, GWL_ID) & 0xffff) | (BN_CLICKED << 16), (LONG)hwnd);
			SetFocus(NULL);
			return(0);
			} // if
		} // KEYDOWN
	} // switch

return(CallWindowProc((WNDPROC)EditWndProc, hwnd, message, wParam, lParam));
} // ModifiedEditWndProc


int RegisterNVWidgets(HINSTANCE Instance)
{
WNDCLASS InquireRegister;
int Successes = 0;

if(GetClassInfo(NULL, "EDIT", &InquireRegister))
	{
	EditWndProc = InquireRegister.lpfnWndProc;
	InquireRegister.lpszClassName	= NVW_NATUREVIEW_CLASSPREFIX ".ModEdit";
	InquireRegister.lpfnWndProc = ModifiedEditWndProc;
	RegisterClass(&InquireRegister);
	Successes++;
	} // if

return(Successes);
} // RegisterNVWidgets

void UnRegisterNVWidgets(HINSTANCE Instance)
{
UnregisterClass(NVW_NATUREVIEW_CLASSPREFIX ".ModEdit", Instance);

} // UnRegisterNVWidgets
