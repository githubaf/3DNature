// CmapEditGUI.h
// Header file for Cmap Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_CMAPEDITGUI_H
#define WCS_CMAPEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class CmapEffect;
class EcosystemEffect;
class DiagnosticData;

#define WCS_CMAPGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class CmapEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		CmapEffect *Active, Backup;
		EcosystemEffect *ActiveEco;
		double LatEvent[2], LonEvent[2];
		char ResponseEnabled, MatchColorsChanged, PickColorType, FirstPick;
		unsigned char ReceivingDiagnostics;

		static char *TabNames[WCS_CMAPGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;


	public:

		int ConstructError;

		CmapEditGUI(EffectsLib *EffectsSource, Database *DBSource, CmapEffect *ActiveSource);
		~CmapEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		CmapEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};

	private:

		void ConfigureMatchColors(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void BuildEcoList(void);
		void BuildEcoListEntry(char *ListName, GeneralEffect *Me);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void EditImage(void);
		void OpenPreview(void);
		void AddEco(void);
		void RemoveEco(void);
		void EditEco(void);
		void Name(void);
		void ChangeEcoListPosition(short MoveUp);
		void SetActiveEco(void);
		void SelectNewImage(void);
		int ValidateGeoRefImage(void);
		void RespondColorNotify(DiagnosticData *Data);
		//void SetBounds(DiagnosticData *Data);

	}; // class CmapEditGUI

#endif // WCS_CMAPEDITGUI_H
