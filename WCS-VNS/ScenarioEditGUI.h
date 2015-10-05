// ScenarioEditGUI.h
// Header file for Scenario Editor
// Created from scratch on 9/11/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#ifndef WCS_SCENARIOEDITGUI_H
#define WCS_SCENARIOEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphWidget.h"
#include "GraphData.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class ImageLib;
class RenderScenario;

#define WCS_SCENARIOGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class ScenarioEditGUI : public WCSModule, public GUIFenetre, public GraphDialog, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		ImageLib *ImageHost;
		RenderScenario *Active, Backup;
		AnimDoubleTime DistanceADT;
		bool BasicOnOff;
		static char *TabNames[WCS_SCENARIOGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;


	public:

		int ConstructError;
		struct GRData WidgetLocal;

		ScenarioEditGUI(EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource, RenderScenario *ActiveSource);
		~ScenarioEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RenderScenario *GetActive(void) {return (Active);};
		void RefreshGraph(void);
		void AddNode(void);
		void RemoveNode(void);
		void SelectAllNodes(void);
		void ClearSelectNodes(void);
		void ToggleSelectNodes(void);
		virtual void NewActiveNode(GraphNode *NewActive);
		void NewDistance(void);
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void BuildItemList(void);
		void BuildItemListEntry(char *ListName, RasterAnimHost *Me, char AddAsterisk);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void AddItem(void);
		void RemoveItem(void);
		void GrabAll(void);
		void EditItem(void);
		void Name(void);
		void SelectNewQuery(void);
		void FreeQuery(void);
		void Actionate(void);
		void ChangeBasicState(void);
		void ConfigureItem(void);
		void CaptureReverseCheck(void);

	}; // class ScenarioEditGUI

#endif // WCS_SCENARIOEDITGUI_H
