// InfoDlg.h
// Interfaces to info dialog

// Black box...

#ifndef NVW_INFODLG_H
#define NVW_INFODLG_H

#include <windows.h>
#include "Types.h"
#include "SnazzyWidget.h"

#define NVW_INFODLG_UPDATE_TIME_MIN	.1		// 10 times per second maximum info window text update rate to prevent insane flicker

class InfoDlgSWEventObserver;

class InfoDlg
	{
	private:
		
		static NativeGUIWin DlgHandle;
		static InfoDlgSWEventObserver CallbackObserver;
		static SnazzyTextWidget *Text;
		static SnazzyWidgetContainer *SWC;


		// for flicker-prevention
		static double LastUpdateMomentSeconds;

		static void UpdateLastMoment(void);
		static double GetCurrentMoment(void); // could be broken out somewhere else for reuse
	public:

		InfoDlg(bool Show = true);
		~InfoDlg();

		static bool HandleEvent(WIDGETID WidgetID);
		static int SetNewText(const char *NewText);
		static void Show(bool NewState = true);
		static bool IsShown(void);
		static NativeGUIWin GetDlgHandle(void) {return(DlgHandle);};
	}; // InfoDlg


class InfoDlgSWEventObserver : public SnazzyWidgetEventObserver
	{
	public:
		virtual void Handle(unsigned long int WidgetID) {InfoDlg::HandleEvent((WIDGETID)WidgetID);};
	}; // InfoDlgSWEventObserver 


#endif // NVW_INFODLG_H
