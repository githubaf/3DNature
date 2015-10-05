// GroundEditGUI.h
// Header file for Ground Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_GROUNDEDITGUI_H
#define WCS_GROUNDEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class GroundEffect;
class Project;
// material GUI
class PortableMaterialGUI;
class GroundEditGUI;
class GroundEditGUIPortableMaterialGUINotifyFunctor;

#define WCS_GROUNDGUI_NUMTABS	2

// material GUI
class GroundEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		GroundEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // GroundEditGUIPortableMaterialGUINotifyFunctor


// Very important that we list the base classes in this order.
class GroundEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		GroundEffect *Active, Backup;
		GradientCritter *ActiveGrad;
		// material GUI
		PortableMaterialGUI *MatGUI;
		GroundEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;
		static char *TabNames[WCS_GROUNDGUI_NUMTABS];
		// advanced
		static long ActivePage, ActiveParam, DisplayAdvanced;

	public:

		int ConstructError;

		GroundEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, GroundEffect *ActiveSource);
		~GroundEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		GroundEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);
		// material GUI
		void ConfigureMaterial(void); // so we can call from GroundEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode) {ActiveGrad = NewNode;}; // so we can call from GroundEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);

		virtual void HideSubordinateWindows(void) {ShowMaterialPopDrop(false);}; // hide popdrop when necessary

	private:

		void ConfigureColors(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);

	}; // class GroundEditGUI

#endif // WCS_GROUNDEDITGUI_H
