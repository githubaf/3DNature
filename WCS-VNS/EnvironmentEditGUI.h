// EnvironmentEditGUI.h
// Header file for Environment Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_ENVIRONMENTEDITGUI_H
#define WCS_ENVIRONMENTEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class EnvironmentEffect;
class Project;

#define WCS_ENVIRONMENTGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class EnvironmentEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		EnvironmentEffect *Active, Backup;
		static char *TabNames[WCS_ENVIRONMENTGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;


	public:

		int ConstructError;

		EnvironmentEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, EnvironmentEffect *ActiveSource);
		~EnvironmentEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		EnvironmentEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};

	private:

		void BuildEcoList(void);
		void BuildEcoListEntry(char *ListName, GeneralEffect *Me);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void AddEco(void);
		void RemoveEco(void);
		void EditEco(void);
		void Name(void);
		void ChangeEcoListPosition(short MoveUp);

	}; // class EnvironmentEditGUI

#endif // WCS_ENVIRONMENTEDITGUI_H
