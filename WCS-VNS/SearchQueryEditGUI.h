// SearchQueryEditGUI.h
// Header file for SearchQuery Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_SEARCHQUERYEDITGUI_H
#define WCS_SEARCHQUERYEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class DBFilterEvent;
class Project;

#define WCS_SEARCHQUERYGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class SearchQueryEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		SearchQuery *Active, Backup;
		Database *DBHost;
		EffectsLib *EffectsHost;
		Project *ProjHost;
		DBFilterEvent *ActiveFilter;
		double LatEvent[2], LonEvent[2];
		char AttrExists, AttrEquals, AttrGreater, AttrGreaterEquals, AttrLess, AttrLessEquals, AttrSimilar, AttrConfigured,
			ReceivingDiagnostics, ReceivingPoint;
		static char *TabNames[WCS_SEARCHQUERYGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		SearchQueryEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, SearchQuery *ActiveSource);
		~SearchQueryEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		SearchQuery *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void BuildFilterList(void);
		void BuildFilterListEntry(char *ListName, DBFilterEvent *Me);
		DBFilterEvent *ActiveFilterValid(void);
		void ConfigureFilter(void);
		void NewMatchLayer(void);
		void NewMatchName(void);
		void NewMatchLabel(void);
		void NewMatchAttribute(void);
		void NewAttributeValue(void);
		void AddFilter(void);
		void RemoveFilter(void);
		void SetActiveFilter(void);
		void ChangeFilterListPosition(short MoveUp);
		void SelectDBItems(void);
		void FillAttributeCBs(void);
		void SetBounds(DiagnosticData *Data);
		void SetPoint(DiagnosticData *Data);

	}; // class SearchQueryEditGUI

#endif // WCS_SEARCHQUERYEDITGUI_H
