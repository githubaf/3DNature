// MaterialEditGUI.h
// Header file for Material Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_MATERIALEDITGUI_H
#define WCS_MATERIALEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class MaterialEffect;
class Object3DEditGUI;

#define WCS_MATERIALGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class MaterialEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	friend class Object3DEditGUI;

	private:
		EffectsLib *EffectsHost;
		MaterialEffect *Active, Backup;
		static char *TabNames[WCS_MATERIALGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		MaterialEditGUI(EffectsLib *EffectsSource, MaterialEffect *ActiveSource);
		~MaterialEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(short PanelID);
		MaterialEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};

	private:

		void ConfigureNumObjects(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void SelectShade(void);

	}; // class MaterialEditGUI

#endif // WCS_MATERIALEDITGUI_H
