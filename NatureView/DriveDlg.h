// DriveDlg.h
// Interfaces to drive dialog

// Black box...

#ifndef NVW_DRIVEDLG_H
#define NVW_DRIVEDLG_H

#include <windows.h>
#include "Types.h"
#include "SnazzyWidget.h"

class DriveDlgSWEventObserver;

class DriveDlg
	{
	private:
		
		static NativeGUIWin DlgHandle;
		static DriveDlgSWEventObserver CallbackObserver;
		static SnazzySlideYWidget *Throttle;
		static SnazzyHotPadWidget *HotNav, *HotTurn;
		static SnazzyWidgetContainer *SWC;

	public:

		DriveDlg(bool Show = true);
		~DriveDlg();

		//int ProcessAndHandleEvents();
		// only called from DriveDlgDialogProc
		static bool HandleEvent(WIDGETID WidgetID);
		static void Show(bool NewState = true);
		static bool IsShown(void);
		static void SyncWidgets(void);
		static NativeGUIWin GetDlgHandle(void) {return(DlgHandle);};
	}; // DriveDlg


class DriveDlgSWEventObserver : public SnazzyWidgetEventObserver
	{
	public:
		virtual void Handle(unsigned long int WidgetID) {DriveDlg::HandleEvent((WIDGETID)WidgetID);};
	}; // DriveDlgSWEventObserver 


#endif // !NVW_DRIVEDLG_H
