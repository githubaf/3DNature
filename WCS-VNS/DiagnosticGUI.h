// DiagnosticGUI.h
// Header file for Diagnostic Data Viewer
// Created from scratch on 11/17/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_DIAGNOSTICGUI_H
#define WCS_DIAGNOSTICGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

class RasterAnimHost;
class DiagnosticData;

// Very important that we list the base classes in this order.
class DiagnosticGUI : public WCSModule, public GUIFenetre
	{

	public:

		int ConstructError;
		unsigned char DisplayBuffer, Threshold[2];
		AnimDoubleTime DistanceADT, ElevationADT, LatitudeADT, LongitudeADT, AspectADT;
		int NextDiagnosticLineY;

		DiagnosticGUI(void);
		~DiagnosticGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(DiagnosticData *Data);
		void SyncScroll(DiagnosticData *Data);
		void SyncButtonMutexes(void);
	
	private:
		int AddDiagnosticLine(int ButtonID, int FieldID, int IconID, char *IconCaption);

	}; // class DiagnosticGUI

#endif // WCS_DIAGNOSTICGUI_H
