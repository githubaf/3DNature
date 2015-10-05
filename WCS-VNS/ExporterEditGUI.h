// ExporterEditGUI.h
// Header file for SceneExporter Editor
// Created from scratch on 4/18 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#ifndef WCS_EXPORTEREDITGUI_H
#define WCS_EXPORTEREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "SXQueryAction.h"
#include "EffectsLib.h"
#include "Tables.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class SceneExporter;
class DiagnosticData;
class EffectList;
class Project;

#define WCS_EXPORTERGUI_NUMTABS	8

// Very important that we list the base classes in this order.
class ExporterEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		ImageLib *ImageHost;
		SceneExporter *Active, Backup;
		SXQueryAction *ActiveAction;
		long LastOneDEMResX, LastOneDEMResY, LastOneTexResX, LastOneTexResY, LastSkyRes, LastFoliageRes, LastDEMResOption, LastTexResOption, JustOpening;
		static char *TabNames[WCS_EXPORTERGUI_NUMTABS];
		static char *ActionTypes[WCS_SXQUERYACTION_NUMACTIONTYPES];
		static int ActionTypeEnums[WCS_SXQUERYACTION_NUMACTIONTYPES];
		static long ActivePage, ActiveAdvancedPage;
		ExportTable ExpTable;

	public:

		int ConstructError, ReceivingDiagnostics, DEMSquare, ShowAdvanced;
		char ExportSkyFeatures, RestoreSkyFeatures, ExportSky, ExportCelest, ExportClouds, ExportStars, ExportAtmosphere, ExportVolumetrics;
		unsigned short ActiveCameraListID, ActiveLightListID, ActiveHazeListID;
		double LatEvent[2], LonEvent[2], PrevNorthValue, PrevSouthValue, PrevEastValue, PrevWestValue;
		char AltFolMethod[64];
		AnimDoubleTime NSDEMCellSize, WEDEMCellSize;

		ExporterEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, SceneExporter *ActiveSource);
		~ExporterEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChangeArrow(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void ConfigureWidgets(void);
		void ConfigAdvancedOptions(void);
		void ConfigureActiveAction(void);
		SceneExporter *GetActive(void) {return (Active);};

	private:

		void Cancel(void);
		void Name(void);
		void SyncFormatCombos(void);
		void BuildScenarioList(void);
		void BuildScenarioListEntry(char *ListName, GeneralEffect *Me);
		void AddScenario(void);
		void RemoveScenario(void);
		void EditScenario(void);
		void ChangeScenarioListPosition(short MoveUp);
		void BuildMiscList(unsigned short WidgetID, long EffectClass, EffectList *SelectedItems);
		void FillComboOptions(unsigned short ComboID, char *FieldName, char *TargetName);
		void SelectOutputFormat(void);
		void SelectImageFormat(void);
		void SelectFolImageFormat(void);
		void SelectExportItem(unsigned short WidgetID, long EffectClass);
		void ValidateSettings(long ActiveFormat);
		long ParseDefaultOption(long ActiveFormat, char *FieldName);
		long ParseDefaultValue(long ActiveFormat, char *FieldName);
		const char *ParseDefaultString(long ActiveFormat, char *FieldName, char *OptionName);
		void ExportNow(void);
		void SelectNewCoords(void);
		void SetBounds(DiagnosticData *Data);
		void FiddleDEMRes(void);
		void FiddleTexRes(void);
		void FiddleSkyRes(void);
		void FiddleFolRes(void);
		void ConfigDEMCellSize(void);
		void ConfigTexCellSize(void);
		void ComputeBoundsFromCellSize(void);
		void AdjustBoundsFromCellSize(int CtrlID);
		void ConfigBoundsMetric(void);
		void ConfigAdvanced(void);
		void ConfigNormal(void);
		void BuildActionItemList(void);
		void BuildActionItemListEntry(char *ListName, RasterAnimHost *Me);
		void ClearActionItemList(void);
		void SelectSTLUnits(void);
		void ComputeSTLScale(void);
		void HDILearnMore(void);
		void RecordActionTextChange(unsigned short WidgetID);
		void RecordNVETextChange(unsigned short WidgetID);
		void RecordFBXTextChange(unsigned short WidgetID);
		void RecordGETextChange(unsigned short WidgetID);
		void FillNVLogoInsertCombo(void);
		void FillActionTypeCombo(void);
		void FillActionItemCombo(void);
		void HideActionOptions(void);
		void InsertNVLogo(void);
		void InsertActionText(void);
		void InsertActionAttrib(void);
		void SelectActionType(void);
		char *TranslateActionName(char ActionType);
		void SelectActiveAction(void);
		void AddActionItem(void);
		void RemoveActionItem(void);
		void GrabActionAll(void);
		void GrabActionQuery(void);
		void BackupPrevBounds(void);

	}; // class ExporterEditGUI

#endif // WCS_EXPORTEREDITGUI_H
