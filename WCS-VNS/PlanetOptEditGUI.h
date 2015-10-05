// PlanetOptEditGUI.h
// Header file for PlanetOpt Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_PLANETOPTEDITGUI_H
#define WCS_PLANETOPTEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class PlanetOpt;

#define WCS_PLANETOPTGUI_NUMTABS	1

// Very important that we list the base classes in this order.
class PlanetOptEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		PlanetOpt *Active, Backup;
		static char *TabNames[WCS_PLANETOPTGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		PlanetOptEditGUI(EffectsLib *EffectsSource, PlanetOpt *ActiveSource);
		~PlanetOptEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		PlanetOpt *GetActive(void) {return (Active);};

	private:

		void SyncWidgets(void);
		void Cancel(void);
		void Name(void);
		void RadiusPreset(void);
		void SelectNewCoords(void);

	}; // class PlanetOptEditGUI

#endif // WCS_PLANETOPTEDITGUI_H
