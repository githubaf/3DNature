// StarfieldEditGUI.h
// Header file for Starfield Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_STARFIELDEDITGUI_H
#define WCS_STARFIELDEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class StarFieldEffect;
// material GUI
class PortableMaterialGUI;
class StarfieldEditGUI;
class StarfieldEditGUIPortableMaterialGUINotifyFunctor;

#define WCS_STARFIELDGUI_NUMTABS	2

// material GUI
class StarfieldEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		StarfieldEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // StarfieldEditGUIPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class StarfieldEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		StarFieldEffect *Active, Backup;
		GradientCritter *ActiveColorGrad;
		ColorTextureThing *ActiveColorNode;
		// material GUI
		PortableMaterialGUI *MatGUI;
		StarfieldEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;

		static char *TabNames[WCS_STARFIELDGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		StarfieldEditGUI(EffectsLib *EffectsSource, StarFieldEffect *ActiveSource);
		~StarfieldEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(short PanelID);
		StarFieldEffect *GetActive(void) {return (Active);};
		// material GUI
		void ConfigureColors(void); // so we can call from StarfieldEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode); // so we can call from StarfieldEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowMaterialPopDrop(false);}; // hide popdrop when necessary

	private:

		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void EditImage(void);
		void ClearImage(void);
		void Name(void);
		void SelectNewImage(void);

	}; // class StarfieldEditGUI

#endif // WCS_STARFIELDEDITGUI_H
