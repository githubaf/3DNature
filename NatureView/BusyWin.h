// BusyWin.h
// New-generation version of BusyWin, for NatureView

#ifndef NVW_BUSYWIN_H
#define NVW_BUSYWIN_H

enum BusyStyle
	{
	NVW_BUSYWIN_STYLE_DEFAULT = 0,
	NVW_BUSYWIN_STYLE_POPUP,
	NVW_BUSYWIN_STYLE_WINDOWEDGE,
	NVW_BUSYWIN_STYLE_MAX // no further items
	}; // BusyWin styles


#include <windows.h>
#include "Types.h"

// BusyWins cannot be nested, only one can exist at a time.

class BusyWin
	{
	private:
		
		static NativeGUIWin DlgHandle;
		static bool Aborted;

	public:

		BusyWin(char *Title, char *InitialText, bool WithCancel, bool WithProgress, BusyStyle Style = NVW_BUSYWIN_STYLE_DEFAULT);
		~BusyWin();

		int ProcessAndHandleEvents();
		// only called from BusyWinDialogProc
		static bool HandleEvent(WIDGETID WidgetID);
		int UpdateAndCheckAbort(float Percent);
		int CheckAbort(void);
		static void Abort(void) {Aborted = true;};
		void SetText(char *NewText);
	}; // BusyWin




#endif // !NVW_BUSYWIN_H
