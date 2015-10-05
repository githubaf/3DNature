// AuthorizeGUI.h
// Header file for Authorize window
// built from VersionGUI.h on Oct 9 2003 by CXH
// Copyright 1996

#ifndef WCS_AUTHORIZEGUI_H
#define WCS_AUTHORIZEGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "WCSWidgets.h"
#include "Effectslib.h"


// Very important that we list the base classes in this order.
class AuthorizeGUI : public WCSModule, public GUIFenetre
	{
	private:
		char SerialStr[19];
		char KeyID[20], WCSAuth[50], VNSAuth[50], SXAuth[50],
		 SXFormats[500];

	
	public:

		AuthorizeGUI();
		~AuthorizeGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);

		void HandleNewAuth(void);

	}; // class AuthorizeGUI

#endif // WCS_AUTHORIZEGUI_H
