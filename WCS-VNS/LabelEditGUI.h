// LabelEditGUI.h
// Header file for Label Editor
// Created from FoliageEffectEditGUI.h on 4/14/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#ifndef WCS_LABELEDITGUI_H
#define WCS_LABELEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class Database;

#define WCS_LABELGUI_NUMTABS	6

// Very important that we list the base classes in this order.
class LabelEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Label *Active, Backup;
		static char *TabNames[WCS_LABELGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		LabelEditGUI(EffectsLib *EffectsSource, Database *DBSource, Label *ActiveSource);
		~LabelEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Label *GetActive(void) {return (Active);};

	private:

		void BuildAttributeList(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);
		void OverlayText(void);
		void ScaleSizes(void);

	}; // class LabelEditGUI

#endif // WCS_LABELEDITGUI_H
