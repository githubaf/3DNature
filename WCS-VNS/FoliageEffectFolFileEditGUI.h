// FoliageEffectFolFileEditGUI.h
// Header file for FoliageEffect Editor
// Created from FoliageEffectEditGUI on 8/24/05 by Gary R. Huber
// Copyright 2005 Questar Productions. All rights reserved.

#ifndef WCS_FOLIAGEEFFECTFOLFILEEDITGUI_H
#define WCS_FOLIAGEEFFECTFOLFILEEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"

#define WCS_FOLIAGEEFFECTFOLFILEGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class FoliageEffectFolFileEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		FoliageEffect *Active, Backup;
		static char *TabNames[WCS_FOLIAGEEFFECTFOLFILEGUI_NUMTABS];
		static char *TabNamesFE[WCS_FOLIAGEEFFECTFOLFILEGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		FoliageEffectFolFileEditGUI(EffectsLib *EffectsSource, FoliageEffect *ActiveSource);
		~FoliageEffectFolFileEditGUI();

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
		FoliageEffect *GetActive(void) {return (Active);};

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	private:

		FoliageGroup *ActiveFoliageValid(void);
		void ConfigureGroup(void);
		void ConfigureFoliage(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void SelectPanel(long PanelID);
		void Cancel(void);
		void Name(void);
		void NewFoliageFile(void);

	}; // class FoliageEffectFolFileEditGUI

#endif // WCS_FOLIAGEEFFECTFOLFILEEDITGUI_H
