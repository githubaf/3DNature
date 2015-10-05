// MergerWizGUI.h
// Header file for Merger Wizard GUI
// Created from ForestWizGUI.h on 01/18/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#ifndef WCS_MERGERWIZGUI_H
#define WCS_MERGERWIZGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "MergerWiz.h"
#include "GraphData.h"
#include "EffectsLib.h"

class EffectsLib;
class ImageLib;
class Database;
class Project;
class DEMMerger;

// Very important that we list the base classes in this order.
class MergerWizGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		ImageLib *ImageHost;
		Database *DBHost;
		Project *ProjectHost;
		WizardlyPage *ActivePage;
		MergerWiz Wizzer;
		DEMMerger Merger;
		SearchQuery *sq;
		bool addedLayer;
		char boundType, haveCS, haveDEMs, resAuto;

	public:
		double LatEvent[2], LonEvent[2];
		long ConstructError, ReceivingDiagnostics;

		MergerWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource);
		~MergerWizGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);

		void AddSQ(void);
		void BuildSQList(void);
		void BuildSQListEntry(char *ListName, GeneralEffect *Me);
		void ChangeSQListPosition(short MoveUp);
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
		void SetActiveSQ(void);

	private:
		void DisplayPage(void);
		void DoCancel(void);
		void DoNext(void);
		void DoPrev(void);
		void NameChange(void);
		void PlugItIn(void);
		void RemoveSQ(void);
		void SelectNewCoords(void);
		void SelectPanel(unsigned short PanelID);
		void SetBounds(DiagnosticData *Data);

	}; // class MergerWizGUI

#endif // WCS_MERGERWIZGUI_H
