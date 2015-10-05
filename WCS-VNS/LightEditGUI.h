// LightEditGUI.h
// Header file for Light Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_LIGHTEDITGUI_H
#define WCS_LIGHTEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Light;

#define WCS_LIGHTGUI_NUMTABS	4

// Very important that we list the base classes in this order.
class LightEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Light *Active, Backup;
		static char *TabNames[WCS_LIGHTGUI_NUMTABS];
		// advanced
		static long ActivePage, ActiveSubPanel, DisplayAdvanced;

	public:

		int ConstructError;

		LightEditGUI(EffectsLib *EffectsSource, Light *ActiveSource);
		~LightEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Light *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void BuildItemList(void);
		void BuildItemListEntry(char *ListName, RasterAnimHost *Me);
		void FillClassDrop(void);
		void AddItem(void);
		void RemoveItem(void);
		void EditItem(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void SelectNewTarget(void);
		void FreeTarget(void);
		void ElevationPreset(void);
		void RadiusPreset(void);
		void SetPosByTime(void);

	}; // class LightEditGUI

#endif // WCS_LIGHTEDITGUI_H
