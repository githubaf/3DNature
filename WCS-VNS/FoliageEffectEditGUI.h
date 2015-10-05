// FoliageEffectEditGUI.h
// Header file for FoliageEffect Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_FOLIAGEEFFECTEDITGUI_H
#define WCS_FOLIAGEEFFECTEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class ImageLib;
class Database;
class PortableFoliageGUI;

#define WCS_FOLIAGEEFFECTGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class FoliageEffectEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		ImageLib *ImageHost;
		EffectsLib *EffectsHost;
		Database *DBHost;
		FoliageEffect *Active, Backup;
		PortableFoliageGUI *FolGUI;
		static char *TabNames[WCS_FOLIAGEEFFECTGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		FoliageEffectEditGUI(EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource, FoliageEffect *ActiveSource);
		~FoliageEffectEditGUI();

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
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		FoliageEffect *GetActive(void) {return (Active);};
		void SetToFoliagePage(void);
		void ActivateFoliageItem(RasterAnimHost *ActivateMe);

	private:

		void ShowPanels(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);

	}; // class FoliageEffectEditGUI

#endif // WCS_FOLIAGEEFFECTEDITGUI_H
