// RenderPreviewGUI.h
// Header file for Render preview window
// Created from scratch on 11/17/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_RENDERPREVIEWGUI_H
#define WCS_RENDERPREVIEWGUI_H

#include "Application.h"
#include "Fenetre.h"

class Renderer;

// Very important that we list the base classes in this order.
class RenderPreviewGUI : public WCSModule, public DrawingFenetre
	{
	friend class Renderer;

	private:
		Renderer *RenderHost;
		long LastX, LastY;

	public:

		int ConstructError, Rendering, Sampling;

		RenderPreviewGUI(Renderer *RenderSource, int Running);
		~RenderPreviewGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift);
		long HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandlePopupMenuSelect(int MenuID);
		void HandleNotifyEvent(void);
		void AttachToRoot(void);
		void AttachToDesktop(void);

	private:
		void RenderDone(bool OpenDiagnostic);

	}; // class RenderPreviewGUI

#endif // WCS_RENDERPREVIEWGUI_H
