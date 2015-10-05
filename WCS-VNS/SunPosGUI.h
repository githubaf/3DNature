// SunPosGUI.h
// Header file for ...Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Revised on 8/13/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#ifndef WCS_SUNPOSGUI_H
#define WCS_SUNPOSGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

class Light;
class Project;

// Very important that we list the base classes in this order.
class SunPosGUI : public WCSModule, public GUIFenetre
	{
	private:
		Light *Active;
		AnimDoubleTime RefLon, BackupLon, BackupLat;
		char AMPM, SunDate;

		void ReverseSeasons(void);
		void ChangeRefLon(void);
		void TimeChange(void);
		void MonthDateChange(void);
		void SetReverse(int Reverse);	// F2 NOTE: argument is never used
		void SetPosition(void);
		void Cancel(void);
		
	public:

		int ConstructError;

		SunPosGUI(Project *ProjSource, Light *ActiveSource);
		~SunPosGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Light *GetActive(void) {return (Active);};

	}; // class SunPosGUI

#endif // WCS_SUNPOSGUI_H
