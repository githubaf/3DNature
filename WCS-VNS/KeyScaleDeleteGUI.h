// KeyScaleDeleteGUI.h
// Header for Key Scale and Delete GUI
// Built from KeyDeleteShortGUI.h on 1/13/00 by Gary R. Huber
// Copyright 2000 by Questar Productions. All rights reserved.

#ifndef WCS_KEYDELETESHORTGUI_H
#define WCS_KEYDELETESHORTGUI_H

class EffectsLib;
class RasterAnimHost;
class Project;

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

// Very important that we list the base classes in this order.
class KeyScaleDeleteGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		Project *ProjHost;
		RasterAnimHost *HostCritter, *RootObj;
		char SiblingsExist, FrameOp, Operation, ItemOp, HostCritterAnimated, RootObjAnimated;
		long RootObjClass;
		char HostCritterNameStr[256], RootCritterNameStr[256], ObjClassNameStr[256];
		double FrameRate, RootObjKeyRange[2], CurObjKeyRange[2], ProjectKeyRange[2], GroupKeyRange[2];
		AnimDoubleTime OldFrame[2], NewFrame[2];


	public:

		int ConstructError;

		KeyScaleDeleteGUI(Project *ProjSource, EffectsLib *EffectsSource, RasterAnimHost *CritterSupplied, int ScaleOrRemove);
		~KeyScaleDeleteGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DisableWidgets(void);
		void ResetTimes(void);
		void Operate(void);

	}; // class KeyScaleDeleteGUI

#endif // WCS_KEYDELETESHORTGUI_H
