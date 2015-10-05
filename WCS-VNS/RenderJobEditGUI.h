// RenderJobEditGUI.h
// Header file for RenderJob Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_RENDERJOBEDITGUI_H
#define WCS_RENDERJOBEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class RenderJob;

#define WCS_RENDERJOBGUI_NUMTABS	1

// Very important that we list the base classes in this order.
class RenderJobEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		RenderJob *Active, Backup;
		static char *TabNames[WCS_RENDERJOBGUI_NUMTABS];
		static long ActivePage;

	public:

		int ConstructError;

		RenderJobEditGUI(EffectsLib *EffectsSource, RenderJob *ActiveSource);
		~RenderJobEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RenderJob *GetActive(void) {return (Active);};

	private:

		void Cancel(void);
		void Name(void);
		void SelectNewCamera(void);
		void SelectNewRenderOpt(void);
		void BuildScenarioList(void);
		void BuildScenarioListEntry(char *ListName, GeneralEffect *Me);
		void AddScenario(void);
		void RemoveScenario(void);
		void EditScenario(void);
		void ChangeScenarioListPosition(short MoveUp);
		void ActionateScenarios(void);

	}; // class RenderJobEditGUI

#endif // WCS_RENDERJOBEDITGUI_H
