// TerraGeneratorEditGUI.h
// Header file for TerraGenerator Editor
// Created from scratch on 12/14/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#ifndef WCS_TERRAGENERATOREDITGUI_H
#define WCS_TERRAGENERATOREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class TerraGenerator;
class TerraGenSampleData;

#define WCS_TERRAGENERATORGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class TerraGeneratorEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		TerraGenerator *Active, Backup;
		static char *TabNames[WCS_TERRAGENERATORGUI_NUMTABS];
		TerraGenSampleData *SampleData;
		static long ActivePage;
		double AreaWidth, AreaHeight, LonWidth, LatHeight;
		long TempPreviewSize;
		unsigned char BackgroundInstalled;
		AnimDoubleTime WidthADT, HeightADT, CellSizeADT;


	public:

		int ConstructError;

		TerraGeneratorEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, TerraGenerator *ActiveSource);
		~TerraGeneratorEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		long HandleBackgroundCrunch(int Siblings);
		void ConfigureWidgets(void);
		TerraGenerator *GetActive(void) {return (Active);};
		double GetMinDimension(void);
		int CreatingInitialSetup(void);

	private:

		void SyncWidgets(void);
		void DisplayGridSize(void);
		void DisableWidgets(void);
		void Cancel(void);
		void Name(void);
		void DEMName(void);
		void CalcArea(void);
		void SelectNewArea(void);
		void SetNewArea(void);
		void SetNewBounds(long VarChanged);
		void CreateDEM(void);
		void UpdateThumbnail(void);

	}; // class TerraGeneratorEditGUI

#endif // WCS_TERRAGENERATOREDITGUI_H
