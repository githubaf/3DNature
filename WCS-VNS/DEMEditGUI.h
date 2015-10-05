// DEMEditGUI.h
// Header file for DEM Editor
// Created from scratch on 1/27/01 by Gary R. Huber
// Copyright 2001 Questar Productions. All rights reserved.

#ifndef WCS_DEMEDITGUI_H
#define WCS_DEMEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"

class EffectsLib;
class Database;
class Project;
class Joe;
class DEM;

#define WCS_DEMGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class DEMEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		Joe *Active;
		DEM *ActiveDEM;
		static char *TabNames[WCS_DEMGUI_NUMTABS];
		static long ActivePage, ActivePanel;
		long RowOffset, ColOffset, ActiveModified;
		float NULLValue;
		AnimDoubleTime NorthADT, SouthADT, WestADT, EastADT;

	public:

		int ConstructError;

		DEMEditGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, Joe *ActiveSource);
		~DEMEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Joe *GetActive(void) {return (Active);};
		void SelectPoints(double SelectLat, double SelectLon);

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	private:

		void ConfigureCoords(void);
		void NewDEMFile(void);
		void SetRowColText(void);
		void FillSpreadSheet(void);
		int GetElevWidgetFromRowCol(long Row, long Col);
		void GetRowColFromElevWidget(int WidID, long &Row, long &Col);
		void DisableWidgets(void);
		void HideWidgets(void);
		void SetNewElev(int CtrlID, long Row, long Col);
		void ReloadDEM(void);
		void SaveDEM(void);
		void NextDEM(void);
		void PrevDEM(void);
		void SelectNewCoords(void);

	}; // class DEMEditGUI

#endif // WCS_DEMEDITGUI_H
