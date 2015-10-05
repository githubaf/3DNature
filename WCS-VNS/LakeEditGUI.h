// LakeEditGUI.h
// Header file for Lake Editor
// Created from StreamEditGUI on 1/31/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#ifndef WCS_LAKEEDITGUI_H
#define WCS_LAKEEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class ImageLib;
class LakeEffect;
class Project;
class PortableFoliageGUI;
// material GUI
class PortableMaterialGUI;
class LakeEditGUI;
class LakeEditGUIBeachPortableMaterialGUINotifyFunctor;
class LakeEditGUIWaterPortableMaterialGUINotifyFunctor;

#define WCS_LAKEGUI_NUMTABS	5

// material GUI
class LakeEditGUIBeachPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		LakeEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // LakeEditGUIBeachPortableMaterialGUINotifyFunctor

// material GUI
class LakeEditGUIWaterPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		LakeEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // LakeEditGUIWaterPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class LakeEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		ImageLib *ImageHost;
		LakeEffect *Active, Backup;
		GradientCritter *ActiveBeachGrad, *ActiveWaterGrad;
		// foliage page
		PortableFoliageGUI *FolGUI;
		// material GUI
		PortableMaterialGUI *BeachMatGUI, *WaterMatGUI;
		LakeEditGUIBeachPortableMaterialGUINotifyFunctor PopDropBeachMaterialNotifier;
		LakeEditGUIWaterPortableMaterialGUINotifyFunctor PopDropWaterMaterialNotifier;

		static char *TabNames[WCS_LAKEGUI_NUMTABS];
		static char *WaterTabNames[4];
		// advanced
		static long ActivePage, ActivePanel, DisplayAdvanced;

	public:

		int ConstructError;

		LakeEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, LakeEffect *ActiveSource);
		~LakeEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived);
		long HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData);
		long HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		LakeEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);
		// material GUI
		void ConfigureBeach(void); // so we can call from LakeEditGUIPortableBeachMaterialGUINotifyFunctor without being a friend
		void SetNewActiveBeachGrad(GradientCritter *NewNode) {ActiveBeachGrad = NewNode;}; // so we can call from LakeEditGUIPortableBeachMaterialGUINotifyFunctor without being a friend
		void ShowBeachMaterialPopDrop(bool ShowState);
		void ConfigureWater(void); // so we can call from LakeEditGUIPortableWaterMaterialGUINotifyFunctor without being a friend
		void SetNewActiveWaterGrad(GradientCritter *NewNode) {ActiveBeachGrad = NewNode;}; // so we can call from LakeEditGUIPortableWaterMaterialGUINotifyFunctor without being a friend
		void ShowWaterMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowWaterMaterialPopDrop(false); ShowBeachMaterialPopDrop(false);}; // hide popdrop when necessary
		// foliage page
		void SetToFoliagePage(void);
		void ActivateFoliageItem(RasterAnimHost *ActivateMe);

	private:

		void ShowPanels(void);
		void BuildWaveList(void);
		void BuildWaveListEntry(char *ListName, GeneralEffect *Me);
		void ConfigureColors(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void AddWave(void);
		void RemoveWave(void);
		void EditWave(void);
		void Name(void);

	}; // class LakeEditGUI

#endif // WCS_LAKEEDITGUI_H
