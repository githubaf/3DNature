// CoordSysEditGUI.h
// Header file for CoordSys Editor
// Created from scratch on 12/28/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#ifndef WCS_COORDSYSEDITGUI_H
#define WCS_COORDSYSEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class CoordSys;

#define WCS_COORDSYSGUI_NUMTABS	5

// Very important that we list the base classes in this order.
class CoordSysEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		CoordSys *Active, Backup;
		static char *TabNames[WCS_COORDSYSGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;
		double EccentricitySq, InvFlattening;


	public:

		int ConstructError, GoneModal;

		CoordSysEditGUI(EffectsLib *EffectsSource, Database *DBSource, CoordSys *ActiveSource, int ModalSource);
		~CoordSysEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void FillZoneDrop(void);
		CoordSys *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};

	private:

		void SyncWidgets(void);
		void HideWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void SelectNewSystem(int NewSystem);
		void SelectNewZone(void);
		void SelectNewMethod(void);
		void SelectNewDatum(int NewDatum);
		void SelectNewEllipsoid(int NewEllipse);
		void ComputeEF(void);
		void ComputeFromEccSq(void);
		void ComputeFromInvFlattening(void);
		void OpenCoordsCalculator(void);

	}; // class CoordSysEditGUI

#endif // WCS_COORDSYSEDITGUI_H
