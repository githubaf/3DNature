// SkyEditGUI.h
// Header file for Sky Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_SKYEDITGUI_H
#define WCS_SKYEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// Material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Sky;
class Raster;
class TextureSampleData;
// Material GUI
class PortableMaterialGUI;
class SkyEditGUI;
class SkyEditGUISkyPortableMaterialGUINotifyFunctor;
class SkyEditGUILightPortableMaterialGUINotifyFunctor;

#define WCS_SKYGUI_NUMTABS	2

// Material GUI
class SkyEditGUISkyPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		SkyEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // SkyEditGUISkyPortableMaterialGUINotifyFunctor

// Material GUI
class SkyEditGUILightPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		SkyEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // SkyEditGUILightPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class SkyEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Sky *Active, Backup;
		GradientCritter *ActiveSkyGrad, *ActiveLightGrad;
		ColorTextureThing *ActiveSkyNode, *ActiveLightNode;
		Raster *Rast;
		TextureSampleData *SampleData;
		// Material GUI
		PortableMaterialGUI *SkyMatGUI, *LightMatGUI;
		SkyEditGUISkyPortableMaterialGUINotifyFunctor PopDropSkyMaterialNotifier;
		SkyEditGUILightPortableMaterialGUINotifyFunctor PopDropLightMaterialNotifier;

		static char *TabNames[WCS_SKYGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;
		unsigned char BackgroundInstalled;

	public:
		int ConstructError;

		SkyEditGUI(EffectsLib *EffectsSource, Sky *ActiveSource);
		~SkyEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleBackgroundCrunch(int Siblings);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		Sky *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(DisplayAdvanced ? true : false);};
		// Material GUI
		void ConfigureColors(void); // so we can call from SkyEditGUISkyPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveSkyGrad(GradientCritter *NewNode) {ActiveSkyGrad = NewNode;}; // so we can call from SkyEditGUISkyPortableMaterialGUINotifyFunctor without being a friend
		void ShowSkyMaterialPopDrop(bool ShowState);
		void SetNewActiveLightGrad(GradientCritter *NewNode) {ActiveLightGrad = NewNode;}; // so we can call from SkyEditGUILightPortableMaterialGUINotifyFunctor without being a friend
		void ShowLightMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowLightMaterialPopDrop(false); ShowSkyMaterialPopDrop(false);}; // hide popdrop when necessary

	private:

		void ConfigureWidgets(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void UpdateThumbnail(void);

	}; // class SkyEditGUI

#endif // WCS_SKYEDITGUI_H
