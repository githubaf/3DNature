// PostProcEditGUI.h
// Header file for PostProc Editor
// Created from scratch on 2/18/02 by Gary R. Huber
// Copyright 2002 3D Nature LLC. All rights reserved.

#ifndef WCS_POSTPROCEDITGUI_H
#define WCS_POSTPROCEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class PostProcess;
class PostProcessEvent;

#define WCS_POSTPROCGUI_NUMTABS	3

// Very important that we list the base classes in this order.
class PostProcEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		PostProcess *Active, Backup;
		PostProcessEvent *ActiveEvent;
		static char *TabNames[WCS_POSTPROCGUI_NUMTABS];
		static long ActivePage;


	public:

		int ConstructError;
		char PreviewEnabled;

		PostProcEditGUI(EffectsLib *EffectsSource, PostProcess *ActiveSource);
		~PostProcEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		PostProcess *GetActive(void) {return (Active);};

	private:

		long HandleDetailPageChange(long NewPageID);
		void ConfigureEvent(void);
		void ConfigureSpecificEvent(PostProcessEvent *ConfigEvent);
		PostProcessEvent *ActiveEventValid(void);
		PostProcessEvent *BuildEventList(void);
		void BuildEventListEntry(char *ListName, PostProcessEvent *Me);
		void SyncWidgets(void);
		void SyncSpecificWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);
		void EventName(void);
		void AddEvent(void);
		void RemoveEvent(void);
		void ChangeEventListPosition(short MoveUp);
		void SetActiveEvent(void);
		void SelectNewEventType(void);
		void SelectNewImage(void);
		void SelectNewCompositeImage(void);
		void UpdatePreviews(void);
		void OverlayText(void);
		void EditImage(void);
		void OpenPreview(void);

	}; // class PostProcEditGUI

#endif // WCS_POSTPROCEDITGUI_H
