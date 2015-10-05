// VectorScaleGUI.h
// Header file for Vector Elevation Scale GUI
// Built from KeyScaleGUI.h on 7/1/97 by Gary R. Huber
// Copyright 1997 by Questar Productions. All rights reserved.

#ifndef WCS_VECTORSCALEGUI_H
#define WCS_VECTORSCALEGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

class Joe;
class Database;
class Project;

// Very important that we list the base classes in this order.
class VectorScaleGUI : public WCSModule, public GUIFenetre
	{
	private:
		Database *HostDB;
		Project *ProjHost;
		Joe *Active, *Backup;
		AnimDoubleTime VSh;
		int ValueScale, ValueShift;
		double VScDbl, VShDbl;
		
	public:

		int ConstructError;
		struct KeyScaleInfo *SKInfo;

		VectorScaleGUI(Database *DBSource, Project *ProjSource, Joe *ActiveSource);
		~VectorScaleGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void ScaleKeys(void);

	}; // class VectorScaleGUI

#endif // WCS_VECTORSCALEGUI_H
