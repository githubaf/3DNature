// AtmosphereEditGUI.h
// Header file for Atmosphere Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_ATMOSPHEREEDITGUI_H
#define WCS_ATMOSPHEREEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Atmosphere;

#define WCS_ATMOSPHEREGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class AtmosphereEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Atmosphere *Active, Backup;
		AtmosphereComponent *ActiveComponent;
		static char *TabNames[WCS_ATMOSPHEREGUI_NUMTABS], *VolTabNames[3];
		// advanced
		static long ActivePage, ActiveSubPanel, DisplayAdvanced;


	public:

		int ConstructError;

		AtmosphereEditGUI(EffectsLib *EffectsSource, Atmosphere *ActiveSource);
		~AtmosphereEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(long PanelID);
		Atmosphere *GetActive(void) {return (Active);};
		static void SetActivePage(long NewActive) {ActivePage = NewActive;};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void BuildComponentList(void);
		void BuildComponentListEntry(char *ListName, AtmosphereComponent *Me);
		AtmosphereComponent *ActiveComponentValid(void);
		void ConfigureComponent(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void ComponentName(void);
		void AddComponent(void);
		void RemoveComponent(void);
		void SetActiveComponent(void);
		void ParticleTypePreset(void);

	}; // class AtmosphereEditGUI

#endif // WCS_ATMOSPHEREEDITGUI_H
