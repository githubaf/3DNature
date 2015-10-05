// DEMMergeGUI.h
// Header DEM merging code & GUI
// Built from scratch on 08/30/02 by Frank Weed II
// Copyright 2002 3D Nature, LLC. All rights reserved.

#ifndef WCS_DEMMERGEGUI_H
#define WCS_DEMMERGEGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "CommonComponentEditor.h"

#define WCS_DEMMERGEGUI_NUMTABS	3

class EffectsLib;
class DEMMerger;

// Very important that we list the base classes in this order.
class DEMMergeGUI: public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:

		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		InterCommon *InteractHost;
		DEMMerger Backup, *Active;
		DEM *ActiveDEM;
		static char *TabNames[WCS_DEMMERGEGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:
		int ConstructError;

		DEMMergeGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, InterCommon *InteractSource,
			DEMMerger *ActiveSource);
		~DEMMergeGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);

		void AddSQ(void);
		void BuildSQList(void);
		void BuildSQListEntry(char *ListName, GeneralEffect *Me);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void ChangeSQListPosition(short MoveUp);
		void ComputeMemUse(void);
		void ConfigureWidgets(void);
		void DisableWidgets(void);
		void Display2ndRes(void);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void Name(void);
		void RemoveSQ(void);
		void EditSQ(void);
		void SelectNewCoords(void);
		void Merge(void);
		void MergeMultiRes(void);
		void SetMetrics(void);
		void SyncWidgets(void);
		DEMMerger *GetActive(void)	{return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	};	// class DEMMergeGUI

#endif // WCS_DEMMERGEGUI_H
