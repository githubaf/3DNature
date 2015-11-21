// NavDlg.h
// Interfaces to navigation dialog

// Black box...

#ifndef NVW_NAVDLG_H
#define NVW_NAVDLG_H

#include <windows.h>
#include "Types.h"
#include "SnazzyWidget.h"

class NavDlgSWEventObserver;


class NavDlg
	{
	private:
		
		static NativeGUIWin DlgHandle;
		static NavDlgSWEventObserver CallbackObserver;
		static SnazzyCheckWidget *Goto, *Follow, *Rotate, *Drive, *Slide, *Climb, *Query;
		static SnazzyWidgetContainer *SWC;

	public:

		NavDlg(bool Show = true);
		~NavDlg();

		int ProcessAndHandleEvents();
		// only called from NavDlgDialogProc
		static bool HandleEvent(WIDGETID WidgetID);
		static void Show(bool NewState = true);
		static bool IsShown(void);
		static void SyncWidgets(void);
		static NativeGUIWin GetDlgHandle(void) {return(DlgHandle);};
	}; // NavDlg


class NavDlgSWEventObserver : public SnazzyWidgetEventObserver
	{
	public:
		virtual void Handle(unsigned long int WidgetID) {NavDlg::HandleEvent((WIDGETID)WidgetID);};
	}; // NavDlgSWEventObserver 


#endif // !NVW_NAVDLG_H
