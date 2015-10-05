// CoordsCalculatorGUI.h
// Header file for Coordinate system transformation calculator
// Created from scratch on 01/31/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#ifndef WCS_COORDSCALCULATORGUI_H
#define WCS_COORDSCALCULATORGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"

class CoordSys;

// Very important that we list the base classes in this order.
class CoordsCalculatorGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		CoordSys *Source, *Dest;
		AnimDoubleTime SourceNorth, SourceEast, SourceElev, 
			DestNorth, DestEast, DestElev, ReverseNorth, ReverseEast, ReverseElev;


	public:

		int ConstructError;

		CoordsCalculatorGUI(EffectsLib *EffectsSource, CoordSys *ActiveSource);
		~CoordsCalculatorGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);

	private:

		void SyncWidgets(void);
		void SelectNewSource(void);
		void SelectNewDest(void);
		void ComputeTransforms(void);

	}; // class CoordsCalculatorGUI

#endif // WCS_COORDSCALCULATORGUI_H
