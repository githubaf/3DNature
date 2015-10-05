// RenderOptEditGUI.h
// Header file for RenderOpt Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_RENDEROPTEDITGUI_H
#define WCS_RENDEROPTEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class RenderOpt;
class ImageOutputEvent;

#define WCS_RENDEROPTGUI_NUMTABS	6

struct RenderPreset
	{
	char *PSName;
	int Width, Height;
	double Aspect, FrameRate;
	}; // RenderPreset

// Very important that we list the base classes in this order.
class RenderOptEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		RenderOpt *Active, Backup;
		ImageOutputEvent *ActiveEvent;
		static char *TabNames[WCS_RENDEROPTGUI_NUMTABS];
		static long ActivePage;
		char CustomRes;


	public:

		int ConstructError;

		RenderOptEditGUI(EffectsLib *EffectsSource, RenderOpt *ActiveSource);
		~RenderOptEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RenderOpt *GetActive(void) {return (Active);};

	private:

		void BuildPostProcList(void);
		void BuildPostProcListEntry(char *ListName, GeneralEffect *Me);
		void ConfigureSizeDrop(void);
		void ConfigureOutputEvent(void);
		void BuildEventBufferList(void);
		void BuildEventCodecList(void);
		void BuildEventList(void);
		void BuildEventListEntry(char *ListName, ImageOutputEvent *Me);
		ImageOutputEvent *ActiveEventValid(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);
		void SelectNewResolution(void);
		void AddNewOutputEvent(void);
		void RemoveOutputEvent(void);
		void SetActiveOutputEvent(void);
		void SelectNewOutputFormat(void);
		void SelectOutputBuffer(void);
		void SelectOutputCodec(void);
		void SetConstrain(void);
		void ScaleSize(void);
		void AddPostProc(void);
		void RemovePostProc(void);
		void EditPostProc(void);
		void ChangePostProcListPosition(short MoveUp);

	}; // class RenderOptEditGUI

#endif // WCS_RENDEROPTEDITGUI_H
