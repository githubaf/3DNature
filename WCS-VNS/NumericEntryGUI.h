// NumericEntryGUI.h
// Header file for NumericEntry Editor
// Created from scratch on 7/8/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_NUMERICENTRYGUI_H
#define WCS_NUMERICENTRYGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

// Very important that we list the base classes in this order.
class NumericEntryGUI : public WCSModule, public GUIFenetre
	{
	private:
		AnimDoubleTime *Active1, *Active2, *Active3, Backup1, Backup2, Backup3;

	public:

		int ConstructError;

		NumericEntryGUI(AnimDoubleTime *ActiveSource);
		~NumericEntryGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		void ConfigureWidgets(void);

	private:

		void ShowWidgets(void);
		void Cancel(void);

	}; // class NumericEntryGUI

#endif // WCS_NUMERICENTRYGUI_H
