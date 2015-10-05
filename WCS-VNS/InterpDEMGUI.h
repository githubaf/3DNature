// InterpDEMGUI.h
// Header file for Interpolate DEM Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_INTERPDEMGUI_H
#define WCS_INTERPDEMGUI_H

class Project;
class Database;
class EffectsLib;

struct elmapheaderV101;

#include "Application.h"
#include "Fenetre.h"

// Very important that we list the base classes in this order.
class InterpDEMGUI : public WCSModule, public GUIFenetre
	{
	private:
		Project *HostProj;
		Database *DBHost;
		EffectsLib *EffectsHost;

		FileReq *FR;
		char 	elevpath[256],
				elevfile[256];
		double ElVar, FlatMax;

	public:

		int ConstructError;

		InterpDEMGUI(Project *Moi, Database *DB, EffectsLib *EffectsSource);
		~InterpDEMGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DoSelect(void);
		void DoInterp(void);


		short InterpDEM(void);
		short SplineMap(float *map, short Xrows, short Ycols, double elvar, double flatmax, 
			short NullReject, float NullValue);
		short DisableOrigDEM(char *BaseName);

	}; // class InterpDEMGUI

#endif // WCS_INTERPDEMGUI_H
