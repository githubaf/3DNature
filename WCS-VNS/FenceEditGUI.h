// FenceEditGUI.h
// Header file for Fence Editor
// Created from scratch on 3/16/01 by Gary R. Huber
// Copyright 2001 Questar Productions. All rights reserved.

#ifndef WCS_FENCEEDITGUI_H
#define WCS_FENCEEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class Fence;
// material GUI
class PortableMaterialGUI;
class FenceEditGUI;
class FenceEditGUIRoofPortableMaterialGUINotifyFunctor;
class FenceEditGUIPanelPortableMaterialGUINotifyFunctor;

#define WCS_FENCEGUI_NUMTABS	3

// material GUI
class FenceEditGUIRoofPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		FenceEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // FenceEditGUIRoofPortableMaterialGUINotifyFunctor

// material GUI
class FenceEditGUIPanelPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		FenceEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // FenceEditGUIPanelPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class FenceEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Fence *Active, Backup;
		GradientCritter *ActiveSpanGrad, *ActiveRoofGrad;
		// material GUI
		PortableMaterialGUI *RoofMatGUI, *PanelMatGUI;
		FenceEditGUIRoofPortableMaterialGUINotifyFunctor PopDropRoofMaterialNotifier;
		FenceEditGUIPanelPortableMaterialGUINotifyFunctor PopDropPanelMaterialNotifier;

		static char *TabNames[WCS_FENCEGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;


	public:

		int ConstructError;

		FenceEditGUI(EffectsLib *EffectsSource, Database *DBSource, Fence *ActiveSource);
		~FenceEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Fence *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);
		// material GUI
		void ConfigureRoof(void); // so we can call from FenceEditGUIPortableRoofMaterialGUINotifyFunctor without being a friend
		void SetNewActiveRoofGrad(GradientCritter *NewNode) {ActiveRoofGrad = NewNode;}; // so we can call from FenceEditGUIPortableRoofMaterialGUINotifyFunctor without being a friend
		void ShowRoofMaterialPopDrop(bool ShowState);
		void ConfigurePanel(void); // so we can call from FenceEditGUIPortablePanelMaterialGUINotifyFunctor without being a friend
		void SetNewActivePanelGrad(GradientCritter *NewNode) {ActiveRoofGrad = NewNode;}; // so we can call from FenceEditGUIPortablePanelMaterialGUINotifyFunctor without being a friend
		void ShowPanelMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowPanelMaterialPopDrop(false); ShowRoofMaterialPopDrop(false);}; // hide popdrop when necessary

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);

	}; // class FenceEditGUI

#endif // WCS_FENCEEDITGUI_H
