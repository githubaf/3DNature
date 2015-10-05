// Object3DEditGUI.h
// Header file for Object3D Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_OBJECT3DEDITGUI_H
#define WCS_OBJECT3DEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
#include "Raster.h"

class EffectsLib;
class Database;
class Object3DEffect;

#define WCS_OBJECT3DGUI_NUMTABS	7

// Very important that we list the base classes in this order.
class Object3DEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Object3DEffect *Active, Backup;
		Database *DBHost;
		MaterialEffect *ActiveMaterial;
		static char *TabNames[WCS_OBJECT3DGUI_NUMTABS];
		// advanced
		static long ActivePage, ActiveParam, DisplayAdvanced;
		Raster TempRast; // used for carrying component thumbnail from BrowseInfo into Thumbnail Toolbutton


	public:

		int ConstructError;

		Object3DEditGUI(EffectsLib *EffectsSource, Database *DBSource, Object3DEffect *ActiveSource);
		~Object3DEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Object3DEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void ConfigureDimensions(void);
		void ConfigureDetail(void);
		void ConfigureColors(MaterialEffect *Mat);
		void FillVectorComboBox(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void SetWidgetsVisible(void);
		void BuildMaterialList(void);
		void BuildMaterialReplaceList(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void EditMaterial(void);
		void EditMaterialColor(void);
		void ReplaceMaterial(void);
		void SelectMaterial(void);
		void ShowSizePos(short ShowWhat);
		void NewUnits(void);
		void NewAlignVec(void);
		void NewObject(void);

	}; // class Object3DEditGUI

#endif // WCS_OBJECT3DEDITGUI_H
