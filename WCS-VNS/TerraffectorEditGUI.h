// TerraffectorEditGUI.h
// Header file for Terraffector Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_TERRAFFECTOREDITGUI_H
#define WCS_TERRAFFECTOREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class TerraffectorEffect;
class Project;

#define WCS_TERRAFFECTORGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class TerraffectorEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		TerraffectorEffect *Active, Backup;
		static char *TabNames[WCS_TERRAFFECTORGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		TerraffectorEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraffectorEffect *ActiveSource);
		~TerraffectorEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(short PanelID);
		TerraffectorEffect *GetActive(void) {return (Active);};

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);
		void SelectNewEco(void);
		void FreeEcosystem(void);

	}; // class TerraffectorEditGUI

#endif // WCS_TERRAFFECTOREDITGUI_H
