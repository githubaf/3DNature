// RasterTAEditGUI.h
// Header file for RasterTA Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_RASTERTAEDITGUI_H
#define WCS_RASTERTAEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class RasterTerraffectorEffect;
class Project;

#define WCS_RASTERTAGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class RasterTAEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		RasterTerraffectorEffect *Active, Backup;
		static char *TabNames[WCS_RASTERTAGUI_NUMTABS];
		static long ActivePage;


	public:

		int ConstructError;

		RasterTAEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, RasterTerraffectorEffect *ActiveSource);
		~RasterTAEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RasterTerraffectorEffect *GetActive(void) {return (Active);};

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);

	}; // class RasterTAEditGUI

#endif // WCS_RASTERTAEDITGUI_H
