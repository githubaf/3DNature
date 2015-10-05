// EcosystemEditGUI.h
// Header file for Ecosystem Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_ECOSYSTEMEDITGUI_H
#define WCS_ECOSYSTEMEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class EcosystemEffect;
class Project;
class Raster;
// foliage page
class PortableFoliageGUI;
// material GUI
class PortableMaterialGUI;
class EcosystemEditGUI;
class EcosystemEditGUIPortableMaterialGUINotifyFunctor;

#define WCS_ECOSYSTEMGUI_NUMTABS	5

// material GUI
class EcosystemEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		EcosystemEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // EcosystemEditGUIPortableMaterialGUINotifyFunctor


// Very important that we list the base classes in this order.
class EcosystemEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		ImageLib *ImageHost;
		EcosystemEffect *Active, Backup;
		GradientCritter *ActiveGrad;
		// foliage page
		PortableFoliageGUI *FolGUI;
		// material GUI
		PortableMaterialGUI *MatGUI;
		EcosystemEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;
		float MaxTerrainElev, MinTerrainElev;

		static char *TabNames[WCS_ECOSYSTEMGUI_NUMTABS];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		EcosystemEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, EcosystemEffect *ActiveSource);
		~EcosystemEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived);
		long HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData);
		long HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		EcosystemEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);
		// material GUI
		void ConfigureMaterial(void); // so we can call from EcosystemEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode) {ActiveGrad = NewNode;}; // so we can call from EcosystemEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowMaterialPopDrop(false);}; // hide popdrop when necessary
		// foliage page
		void SetToFoliagePage(void);
		void ActivateFoliageItem(RasterAnimHost *ActivateMe);

	private:

		void ShowPanels(void);
		void ConfigureRules(void);
		void ConfigureColors(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void ChangeActiveParam(int NewActive);

	}; // class EcosystemEditGUI

#endif // WCS_ECOSYSTEMEDITGUI_H
