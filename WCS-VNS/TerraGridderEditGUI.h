// TerraGridderEditGUI.h
// Header file for TerraGridder Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_TERRAGRIDDEREDITGUI_H
#define WCS_TERRAGRIDDEREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class RenderJob;
class DBFilterEvent;
class Database;
class Project;

#define WCS_TERRAGRIDDERGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class TerraGridderEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		TerraGridder *Active, Backup;
		DBFilterEvent *ActiveFilter;
		static char *TabNames[WCS_TERRAGRIDDERGUI_NUMTABS];
		static long ActivePage;
		AnimDoubleTime NorthADT, SouthADT, EastADT, WestADT;
		unsigned char ReceivingDiagnostics;
		double LatEvent[2], LonEvent[2];

	public:

		int ConstructError;

		TerraGridderEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraGridder *ActiveSource);
		~TerraGridderEditGUI();

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
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		TerraGridder *GetActive(void) {return (Active);};

	private:

		void Cancel(void);
		void Name(void);
		void SetGridSizeText(void);
		void BuildFilterList(void);
		void BuildFilterListEntry(char *ListName, DBFilterEvent *Me);
		DBFilterEvent *ActiveFilterValid(void);
		void ConfigureFilter(void);
		void NewMatchLayer(void);
		void NewMatchName(void);
		void NewMatchLabel(void);
		void AddFilter(void);
		void RemoveFilter(void);
		void SetActiveFilter(void);
		void ChangeFilterListPosition(short MoveUp);
		void DoNewOutputName(void);
		void SetBounds(DiagnosticData *Data);
		void SelectNewCoords(void);

	}; // class TerraGridderEditGUI

#endif // WCS_TERRAGRIDDEREDITGUI_H
