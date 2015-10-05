// MaterialStrataEditGUI.h
// Header file for MaterialStrata Editor
// Created from scratch on 7/8/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_MATERIALSTRATAEDITGUI_H
#define WCS_MATERIALSTRATAEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"

class EffectsLib;
class MaterialStrata;

// Very important that we list the base classes in this order.
class MaterialStrataEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		MaterialStrata *Active, Backup;

	public:

		int ConstructError;

		MaterialStrataEditGUI(EffectsLib *EffectsSource, MaterialStrata *ActiveSource);
		~MaterialStrataEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		MaterialStrata *GetActive(void) {return (Active);};
		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);

	}; // class MaterialStrataEditGUI

#endif // WCS_MATERIALSTRATAEDITGUI_H
