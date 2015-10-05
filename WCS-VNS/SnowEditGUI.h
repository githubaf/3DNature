// SnowEditGUI.h
// Header file for Snow Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_SNOWEDITGUI_H
#define WCS_SNOWEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class SnowEffect;
class Project;
// material GUI
class PortableMaterialGUI;
class SnowEditGUI;
class SnowEditGUIPortableMaterialGUINotifyFunctor;

#define WCS_SNOWGUI_NUMTABS	4

// material GUI
class SnowEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		SnowEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // SnowEditGUIPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class SnowEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		SnowEffect *Active, Backup;
		GradientCritter *ActiveGrad;
		// material GUI
		PortableMaterialGUI *MatGUI;
		SnowEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;
		float MaxTerrainElev, MinTerrainElev;
		static char *TabNames[WCS_SNOWGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		SnowEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, SnowEffect *ActiveSource);
		~SnowEditGUI();

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
		SnowEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);
		// material GUI
		void ConfigureMaterial(void); // so we can call from SnowEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode) {ActiveGrad = NewNode;}; // so we can call from SnowEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);

	private:

		void ConfigureRules(void);
		void ConfigureColors(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);

	}; // class SnowEditGUI

#endif // WCS_SNOWEDITGUI_H
