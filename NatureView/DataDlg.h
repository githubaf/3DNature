// DataDlg.h
// Interfaces to data dialog

// Black box...

#ifndef NVW_DATADLG_H
#define NVW_DATADLG_H

#include <windows.h>
#include "Types.h"

// margin in pixels on all sides of Rich Text widget
#define NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_X	2
#define NVW_DATADLG_WIDGET_OUTSIDE_MARGIN_Y	2
#define NVW_DATADLG_WIDGET_INSIDE_MARGIN_X	4
#define NVW_DATADLG_WIDGET_INSIDE_MARGIN_Y	5

DWORD __stdcall EStreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);


class DataDlg
	{
	private:
		static NativeGUIWin DlgHandle;
		static HMODULE RichEditLib;
	public:

		DataDlg(bool Show);
		~DataDlg();

		static int ProcessAndHandleEvents();
		static int SetDataText(const char *NewText);
		static int SetDataTextFromFile(const char *Filename);
		static int ResizeToFit(void);
		static void Show(bool NewState = true);
	}; // DataDlg


#endif // !NVW_DATADLG_H
