// HelpDlg.h
// Interfaces to help dialog

// Black box...

#ifndef NVW_HELPDLG_H
#define NVW_HELPDLG_H

#include <windows.h>
#include "Types.h"


class HelpDlg
	{
	private:
		
		static NativeGUIWin DlgHandle;
	public:

		HelpDlg();
		~HelpDlg();

		int ProcessAndHandleEvents();
		static int SetHelpText(const char *NewText);
		static void Show(bool NewState = true);
	}; // HelpDlg


#endif // !NVW_HELPDLG_H
