// ShadowEditGUI.h
// Header file for Shadow Editor
// Created from scratch on 8/13/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_SHADOWEDITGUI_H
#define WCS_SHADOWEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class ShadowEffect;
class Project;

#define WCS_SHADOWGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class ShadowEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		ShadowEffect *Active, Backup;
		static char *TabNames[WCS_SHADOWGUI_NUMTABS];
		static long ActivePage;


	public:

		int ConstructError;

		ShadowEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ShadowEffect *ActiveSource);
		~ShadowEditGUI();

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
		ShadowEffect *GetActive(void) {return (Active);};

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);

	}; // class ShadowEditGUI

#endif // WCS_SHADOWEDITGUI_H
