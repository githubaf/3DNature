// CameraEditGUI.h
// Header file for Camera Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_CAMERAEDITGUI_H
#define WCS_CAMERAEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class ImageLib;
class Camera;

#define WCS_CAMERAGUI_NUMTABS	6

// Very important that we list the base classes in this order.
class CameraEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		ImageLib *ImageHost;
		Camera *Active, Backup;
		char SyncLensFOV;
		static char *TabNames[WCS_CAMERAGUI_NUMTABS];
		// advanced
		static long ActivePage, ActiveSubPanel, DisplayAdvanced;


	public:

		int ConstructError;

		CameraEditGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Camera *ActiveSource);
		~CameraEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
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
		Camera *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:
		
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void EditImage(void);
		void FreeTarget(void);
		void SelectNewImage(void);
		void SelectNewTarget(void);
		void Name(void);
		void SyncLens(void);
		void SyncFOV(void);
		void SetSeparation(void);
		void SelectNewCoords(void);

	}; // class CameraEditGUI

#endif // WCS_CAMERAEDITGUI_H
