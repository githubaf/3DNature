// ThematicMapEditGUI.h
// Header file for ThematicMap Editor
// Created from scratch on 12/26/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#ifndef WCS_THEMATICMAPEDITGUI_H
#define WCS_THEMATICMAPEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class ThematicMap;
class Project;

#define WCS_THEMATICMAPGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class ThematicMapEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		ThematicMap *Active, Backup;
		static char *TabNames[WCS_THEMATICMAPGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;
		char AttrExists, AttrEquals, AttrGreater, AttrGreaterEquals, AttrLess, AttrLessEquals, AttrSimilar;

	public:

		int ConstructError;
		static char *MultPresetNames[];
		static double MultPresetValues[];

		ThematicMapEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ThematicMap *ActiveSource);
		~ThematicMapEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		ThematicMap *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};

	private:

		void FillAttributeCBs(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void HideWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void SelectNewAttribute(int WidID, int AttribNum);
		void SelectNewMultPreset(int WidID);
		void SelectPresetName(void);
		void NewAttributeValue(void);

	}; // class ThematicMapEditGUI

#endif // WCS_THEMATICMAPEDITGUI_H
