// GridderWizGUI.h
// Header file for Gridder Wizard GUI
// Created from GridderWizGUI.h on 03/20/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#ifndef WCS_GRIDDERWIZGUI_H
#define WCS_GRIDDERWIZGUI_H

#include <list>
#include "Application.h"
#include "Fenetre.h"
#include "GridderWiz.h"
#include "GraphData.h"
#include "EffectsLib.h"

using namespace std;

class EffectsLib;
class ImageLib;
class Database;
class Project;
class DEMMerger;

// Very important that we list the base classes in this order.
class GridderWizGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *effectsHost;
		ImageLib *imageHost;
		Database *dbHost;
		Project *projectHost;
		WizardlyPage *activePage;
		AnimDoubleTime NorthADT, SouthADT, EastADT, WestADT;
//#ifdef WCS_BUILD_VNS
		list<CoordSys *> coordsList;
//#endif // WCS_BUILD_VNS
		list<Joe *> gridList;
		GridderWiz wizzer;
		TerraGridder gridder;
		unsigned long totalPoints;
		bool	dbWasOpen, hasCP, hasVector, hasEnabled, hasDisabled, hasLines, hasPoints;
		char	layerName[32], haveData;

	public:
		double LatEvent[2], LonEvent[2];
		long ConstructError, ReceivingDiagnostics;

		GridderWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource);
		~GridderWizGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);

		void ConfigureWidgets(void);
		bool CreateLayers(void);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		//long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCloseWin(NativeGUIWin NW);
		//long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		//long HandleEvent(void);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		//long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);

	private:
		void ConfigureBounds(void);
		void CreateGridList(void);
		void DisplayPage(void);
		void DoCancel(void);
		void DoNext(void);
		void DoPrev(void);
		void InitBounds(void);
		void NameChange(void);
		void PlugItIn(void);
		void SelectNewCoords(void);
		void SelectPanel(unsigned short PanelID);
		void SetBounds(DiagnosticData *Data);

	}; // class GridderWizGUI

#endif // WCS_GRIDDERWIZGUI_H
