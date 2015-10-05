// TerrainParamEditGUI.h
// Header file for TerrainParam Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_TERRAINPARAMEDITGUI_H
#define WCS_TERRAINPARAMEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class TerrainParamEffect;

#define WCS_TERRAINPARAMGUI_NUMTABS	1

// Very important that we list the base classes in this order.
class TerrainParamEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		TerrainParamEffect *Active, Backup;
		static char *TabNames[WCS_TERRAINPARAMGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		TerrainParamEditGUI(EffectsLib *EffectsSource, Database *DBSource, TerrainParamEffect *ActiveSource);
		~TerrainParamEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(short PanelID);
		TerrainParamEffect *GetActive(void) {return (Active);};

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);

	}; // class TerrainParamEditGUI

#endif // WCS_TERRAINPARAMEDITGUI_H
