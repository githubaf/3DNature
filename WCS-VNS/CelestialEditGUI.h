// CelestialEditGUI.h
// Header file for Celestial Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_CELESTIALEDITGUI_H
#define WCS_CELESTIALEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class CelestialEffect;

#define WCS_CELESTIALGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class CelestialEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		CelestialEffect *Active, Backup;
		static char *TabNames[WCS_CELESTIALGUI_NUMTABS];
		static long ActivePage;


	public:

		int ConstructError;

		CelestialEditGUI(EffectsLib *EffectsSource, CelestialEffect *ActiveSource);
		~CelestialEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		CelestialEffect *GetActive(void) {return (Active);};

	private:

		void BuildLightList(void);
		void BuildLightListEntry(char *ListName, GeneralEffect *Me);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void EditImage(void);
		void AddLight(void);
		void RemoveLight(void);
		void EditLight(void);
		void Name(void);
		void SelectNewImage(void);
		void SelectNewPhaseSource(void);
		void ElevationPreset(void);
		void RadiusPreset(void);

	}; // class CelestialEditGUI

#endif // WCS_CELESTIALEDITGUI_H
