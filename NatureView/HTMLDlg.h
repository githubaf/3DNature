// HTMLDlg.h
// Interfaces to HTML dialog

// Black box...

#ifndef NVW_HTMLDLG_H
#define NVW_HTMLDLG_H

#include <windows.h>
#include <string>
#include "Types.h"

class HTMLDlg
	{
	private:
		static NativeGUIWin DlgHandle;
		static HMODULE RichEditLib;
		static bool ToolbarVisible;
		static bool CenterOnOpen;
		static std::string ForceTitle;
		
	public:

		HTMLDlg(bool Show);
		~HTMLDlg();

		static int SetHTMLText(const char *NewText);
		static int SetHTMLTextFromFile(const char *Filename);
		// these next two are temporary stop-gaps
		static int SetImageFromFile(const char *Filename);
		static int SetMediaFromFile(const char *Filename);

		static void Show(bool NewState = true);
		static void ShowToolbar(bool NewState = true);
		
		static void SetForceTitle(char *NewTitle) {ForceTitle = NewTitle;};
		static void ClearForceTitle(void) {ForceTitle = "";};
		static const char *GetForceTitle(void) {if(ForceTitle.length()) return(ForceTitle.c_str()); else return(NULL);};
		
		static void SetCenterOnOpen(bool NewValue) {CenterOnOpen = NewValue;};

		// public because it's used by WM_SIZE processing
		static bool GetToolbarBounds(RECT *BoundsOutput);
		static bool CreateBrowserWidget(void);
		static void DestroyBrowserWidget(void);
		static HWND GetHWND(void) {return(DlgHandle);};
		static void DoNewURL(void);
		static void HandleResize(int ClientW, int ClientH);
	}; // HTMLDlg


#endif // !NVW_HTMLDLG_H
