// ForestWizGUI.h
// Header file for Forest Wizard
// Created from scratch on 1/30/04 by Gary R. Huber
// Copyright 2004 Questar Productions. All rights reserved.

#ifndef WCS_FORESTWIZGUI_H
#define WCS_FORESTWIZGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "ForestWiz.h"
#include "GraphData.h"

class EffectsLib;
class ImageLib;
class Database;
class Project;
class DiagnosticData;


// Very important that we list the base classes in this order.
class ForestWizGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		ImageLib *ImageHost;
		Database *DBHost;
		Project *ProjectHost;
		WizardlyPage *ActivePage;
		ForestWiz Wizzer;
		long CurrentColor, CurrentEcoData, CurrentEcoFolFile, CurrentClassData, CurrentClassFolFile, CurrentClassGroup, CurrentSpeciesData;
		char CMapMatchName[WCS_FORESTWIZ_MAXUNITNAMELEN], CurrentEcoName[WCS_FORESTWIZ_MAXUNITNAMELEN], PreferredAreaUnits;
		unsigned char CMapMatchRGB[3];
		ForestWizEcoData DisplayEcoData;
		ForestWizClassData DisplayClassData;
		AnimDoubleTime CMapHtADT;

	public:

		int ConstructError;

		ForestWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource);
		~ForestWizGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		//long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		//long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		//long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);

	private:
		void DisplayPage(void);
		void SelectPanel(unsigned short PanelID);
		void DoNext(void);
		void DoPrev(void);
		void DoCancel(void);
		void BuildCMapImageList(void);
		void BuildImageListEntry(char *ListName, Raster *Me);
		void SelectCMapImage(void);
		void ShowCMapImage(void);
		void SyncMatchColors(void);
		void ProcessMatchColor(int ClearColors, int Advance);
		void GoBackAMatchColor(void);
		void ChangeCMapName(unsigned short CtrlID);
		void RespondCmapColorNotify(DiagnosticData *Data);
		void SetFirstUnidentifiedCMapColor(long StartCt);
		void SyncCMapColors(void);
		void SyncCMapName(void);
		void DisableCMapButtons(void);
		void ConfigureCMapColors(void);
		void FillEcoGroundTexCombo(void);
		void FillEcoUnitsCombo(void);
		void ConfigureEcoData(int ConfigNULL);
		void UpdateEcoFoliage(void);
		void DisableEcoButtons(void);
		void ProcessEcoUnit(void);
		void GoBackAnEcoUnit(void);
		void ProcessEcoImageName(int AdvanceImage);
		void GoBackAnEcoImageName(void);
		void SelectNewEcoGroundTex(void);
		void SetEcoDensityUnits(void);
		void GrabEcoImages(void);
		void ConfigureClassData(int ConfigNULL);
		void UpdateClassFoliage(void);
		void DisableClassButtons(void);
		void ProcessClass(void);
		void GoBackAClass(void);
		void ProcessClassImageName(int AdvanceImage);
		void GoBackAClassImageName(void);
		void SelectNewClassGroundTex(void);
		void SetClassDensityUnits(void);
		void GrabClassImages(void);
		int FetchSpeciesData(long SpeciesCt, ForestWizClassData *ClassData);
		void ConfigureSpeciesData(int ConfigNULL);
		void ProcessSpecies(void);
		void GoBackASpecies(void);
		void SetSpeciesSize(void);
		void DisableSpeciesButtons(void);
		void ConfigureDBHData(int ConfigNULL);
		void ProcessDBH(void);
		void GoBackADBH(void);
		void SetSpeciesDBH(void);
		void DisableDBHButtons(void);
		void ConfigureSPDensData(int ConfigNULL);
		void ProcessSPDens(void);
		void GoBackASPDens(void);
		void SetSpeciesSPDens(void);
		void DisableSPDensButtons(void);
		void FillDBAttributeCombos(void);
		void SelectDBAttribute(unsigned short WidID);
		void SelectMultiDensAttribs(unsigned short WidID);
		void FillFolHeightCombos(void);
		void SelectFolHeightUnits(unsigned short WidID);
		void SelectFolDBHUnits(unsigned short WidID);
		void SelectFolDBHThemeUnits(unsigned short WidID);
		void SelectFolBasalAreaUnits(unsigned short WidID);
		void SetDBHUnitLabels(void);
		void DisableMinHtWidgets(void);

	}; // class ForestWizGUI

#endif // WCS_FORESTWIZGUI_H
