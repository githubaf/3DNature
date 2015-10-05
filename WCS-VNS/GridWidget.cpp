// GridWidget.cpp
// Implementation of the Grid/Listview widget used by the Database Editor
// Built from scratch on 3/20/2008 by Chris 'Xenon' Hanson
// Copyright 2008

#include "stdafx.h"
#include "Types.h"
#include "AppMem.h"
#include "WCSWidgets.h"
#include "GridWidget.h"
#include "WidgetSupport.h"
#include "Application.h"
#include "Joe.h"
#include "WCSVersion.h"
#include "resource.h"
#include "Useful.h"
#include "Palette.h"
#include "Fenetre.h"
#include "Notify.h"
#include "Database.h"
#include "Conservatory.h"
#include "DBEditGUI.h" // for ENUM_DBEDITGUI_GRID_COLUMN

#ifdef _WIN32
#include <mmsystem.h>
#include "VisualStylesXP.h"
#endif // _WIN32

extern WCSApp *GlobalApp;
long WINAPI GridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
// this has been broken out to DBEditGUI.cpp
long WINAPI DBGridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
extern WNDPROC ListViewWndProc;
extern unsigned long int ListViewOriginalWndExtra;

/*===========================================================================*/

long WINAPI GridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
GridWidgetInstance *WThis;

WThis = (GridWidgetInstance *)GetWindowLong(hwnd, ListViewOriginalWndExtra);

switch (message)
	{
	case WM_WCSW_GW_LVNELEREFLECT:
	case WM_WCSW_GW_LVNBLEREFLECT:
	case WM_WCSW_GW_LVNGDIREFLECT:
	case WM_WCSW_GW_LVNOCHREFLECT:
	case WM_WCSW_GW_LVNBSREFLECT:
	case WM_WCSW_GW_NMCDREFLECT:
	case WM_COMMAND:
	case WM_NOTIFY:
		{
		return(DBGridWidgetWndProc(hwnd, message, wParam, lParam));
		} // forward to DB GridWidget handling
	case WM_CREATE:
		{
		long ReturnValue = (CallWindowProc((WNDPROC)ListViewWndProc, hwnd, message, wParam, lParam));
		if (WThis = (GridWidgetInstance *)AppMem_Alloc(sizeof(GridWidgetInstance), APPMEM_CLEAR))
			{
			SetWindowLong(hwnd, ListViewOriginalWndExtra, (long)WThis);
			WThis->MyControl = hwnd;

			// tinker with the embedded Header widget
			HWND HeaderWnd = ListView_GetHeader(hwnd);
			LONG NewStyle = GetWindowLong(HeaderWnd, GWL_STYLE);
			NewStyle |= HDS_BUTTONS;
			SetWindowLong(HeaderWnd, GWL_STYLE, NewStyle);
			//g_xpStyle.SetWindowTheme(hwnd, (L" "), (L" ")); // disable theming so color control works
			WThis->CheckedIcon = (HICON)LoadImage((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_CHECKED), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
			WThis->UnCheckedIcon = (HICON)LoadImage((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_UNCHECKED), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
			WThis->DiasbledCheckIcon = (HICON)LoadImage((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDI_DISABLEDCHECK), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED);
			return(ReturnValue);
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else
		} // CREATE
	case WM_DESTROY:
		{
		if (WThis)
			{
			if (WThis->HeaderPopupMenu)
				{
				DestroyMenu(WThis->HeaderPopupMenu);
				} // if
			AppMem_Free(WThis, sizeof(GridWidgetInstance));
			} // if
		SetWindowLong(hwnd, ListViewOriginalWndExtra, NULL);
		break;
		} // DESTROY
	case WM_WCSW_GW_SETUP:
		{
		HICON CheckedIcon = WThis->CheckedIcon; // for backup
		HICON UnCheckedIcon = WThis->UnCheckedIcon; // for backup
		HICON DiasbledCheckIcon = WThis->DiasbledCheckIcon; // for backup
		HWND MyControl = WThis->MyControl; // for backup
		if (WThis && lParam)
			{ // copy config data over
			memcpy(WThis, (void *)lParam, sizeof(GridWidgetInstance));
			WThis->CheckedIcon = CheckedIcon;
			WThis->UnCheckedIcon = UnCheckedIcon;
			WThis->DiasbledCheckIcon = DiasbledCheckIcon;
			WThis->MyControl = MyControl;
			} // if
		return(1);
		} // WM_WCSW_GW_SETUP
	case WM_WCSW_GW_SETSTYLE:
		{
		if (WThis)
			{ // copy config data over
			WThis->GWStyles = lParam;
			// refresh with new styles
			InvalidateRect(hwnd, NULL, NULL);
			} // if
		return(1);
		} // WM_WCSW_GW_SETUP
	case WM_KEYDOWN:
		{
		if ((int)wParam == 'A' && GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
			{ // eat message, processed in KEYUP below
			return(0);
			} // if
		break;
		} // KEYDOWN
	case WM_KEYUP:
		{
		if ((int)wParam == 'A' && GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
			{
			ListView_SetItemState(hwnd, -1, LVIS_SELECTED, (LVIS_SELECTED | LVIS_FOCUSED | LVIS_DROPHILITED | LVIS_CUT)); // select all
			// forge LBN_SELCHANGE to make everyone take notice
			PostMessage(GetParent(hwnd), WM_COMMAND, ((UINT)GetWindowLong(hwnd, GWL_ID) & 0xffff) | (LBN_SELCHANGE << 16), (LONG)hwnd);
			return(0);
			} // if
		if ((int)wParam == VK_DELETE)
			{
			SendMessage(GetParent(hwnd), WM_WCSW_LIST_DELITEM, (WPARAM)(GetWindowLong(hwnd, GWL_ID)), (LPARAM)hwnd);
			return(0);
			} // if
		if ((int)wParam == 'V')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_PASTE, (WPARAM)(GetWindowLong(hwnd, GWL_ID)), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		if ((int)wParam == 'C')
			{
			if (GlobalApp->WinSys->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
				{
				SendMessage(GetParent(hwnd), WM_WCSW_LIST_COPY, (WPARAM)(GetWindowLong(hwnd, GWL_ID)), (LPARAM)hwnd);
				return(0);
				} // if
			} // if
		break;
		} // KEYUP
	case WM_LBUTTONDBLCLK:
		{ // no clipping, works even in name/label region
		return(DBGridWidgetWndProc(hwnd, message, wParam, lParam));
		} //
	case WM_LBUTTONDOWN:
		{
		// if it within the Name or Label area?
		if(WThis->GridWidgetCache->size()) // are there any items?
			{
			short xPos = LOWORD(lParam); 
			//short yPos = HIWORD(lParam);
			RECT ItemRect;
			ListView_GetItemRect(hwnd, ListView_GetTopIndex(hwnd), &ItemRect, LVIR_LABEL);	//lint !e522
			if(xPos >= ItemRect.left && xPos <= ItemRect.right)
				{ // forward to base class for selection operations
				return(CallWindowProc(ListViewWndProc, hwnd, message, wParam, (LPARAM)lParam));
				} // if
			else
				{ // pass along to widget internals to handle operations
				return(DBGridWidgetWndProc(hwnd, message, wParam, lParam));
				} // else
			} // if
		return(0); // if we didn't forward it, eat it
		} // WM_LBUTTONDOWN
	} // switch

return(CallWindowProc(ListViewWndProc, hwnd, message, wParam, (LPARAM)lParam));

} // GridWidgetWndProc

/*===========================================================================*/

int GridWidgetInstance::FetchColumnWidthPixels(int ColumnNum)
{

return(ListView_GetColumnWidth(MyControl, ColumnNum));

} // GridWidgetInstance::FetchColumnWidthPixels
