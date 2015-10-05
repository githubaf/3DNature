// PortableFoliageGUI.h
// Header file for foliage pages of numerous component editors
// Created from parts of EcosystemEditGUI on 4/15/08 by Gary R. Huber
// Copyright 2008 Questar Productions. All rights reserved.

#ifndef WCS_PORTABLEFOLIAGEGUI_H
#define WCS_PORTABLEFOLIAGEGUI_H

class EffectsLib;
class GUIFenetre;
class ImageLib;
class GradientCritter;
class Ecotype;
class FoliageGroup;
class Foliage;
class Raster;
class AnimMaterialGradient;
class MaterialEffect;

enum
	{
	WCS_FOLIAGE_PANEL_ECOTYPE_GENERAL,
	WCS_FOLIAGE_PANEL_ECOTYPE_SETTINGS,
	WCS_FOLIAGE_PANEL_ECOTYPE_PARAMS,
	WCS_FOLIAGE_PANEL_FOLIAGEGROUP,
	WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE,
	WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT
	}; 
	
class PortableFoliageGUI
	{
	private:
	public:
		EffectsLib *EffectsHost;
		GUIFenetre *FenetreHost;
		ImageLib *ImageHost;
		GradientCritter *ActiveGrad;
		AnimMaterialGradient *MatGrad;
		Ecotype *ActiveEcotype, *FixedEcotype;
		FoliageGroup *ActiveGroup;
		Foliage *ActiveFol;
		void *LastSelected, *EcotypeTreeItem[2];
		Raster *ObjRast;
		long FoliagePanel, EcotypePanel, EcotypeSelection, HostEdFolPage, FoliagePanelSet, EcotypePanelSet,
			HostActivePage;
		char OldType;
		int ConstructError;

		PortableFoliageGUI(GUIFenetre *FenetreSource, EffectsLib *EffectsSource, 
			ImageLib *ImageSource, AnimMaterialGradient *MatGradSource, Ecotype *EcotypeSource, long HostFolPageSource,
			long FolPanelSetSource, long EcotypePanelSource, long HostActivePageSource);
		~PortableFoliageGUI();

		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived);
		long HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData);
		long HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);
		void EcotypeTabSetup(void);
		void BuildImageList(bool RebuildTree);
		void ConfigureFoliageNail(void);
		void BuildObjectList(bool RebuildTree);
		void FillUnitDrops(void);
		void ShowPanels(void);
		void HidePanels(void);
		void SetActivePage(long NewActivePage)	{HostActivePage = NewActivePage;};

		void ConfigureAllFoliage(void);
		void ConfigureEcotype(void);
		Ecotype *ActiveFoliageValid(void);
		void ConfigureGroup(void);
		void ConfigureFoliage(void);
		void SyncFoliageWidgets(void);
		void BuildFoliageTree(void);
		void SyncFoliageTreeNames(void);
		void DisableFoliageWidgets(void);
		void SelectNewFoliageImage(long Current);
		void SelectNewFoliageObject(long Current);
		void EditFoliageObject(void);
		void SetFoliageDensityUnits(void);
		void SetFoliageBAUnits(unsigned short WidID);
		void FoliageGroupName(void);
		void AddEcotype(void);
		void RemoveEcotype(void);
		void RemoveFoliageGroup(void);
		void RemoveFoliage(void);
		void AddFoliageGroup(void);
		void AddFoliage(void);
		void OpenFoliagePreview(void);
		void OpenObjectPreview(void);
		void ApplySettingsToFoliageGroup(void);
		void CopyFoliageItem(RasterAnimHost *EditTarget);
		void PasteFoliageItem(RasterAnimHost *EditTarget);
		void ChangeFoliageSelection(unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData);
		void ListItemExpanded(void *TreeItemData, char Expand);
		void ValidateEcotypeSelection(MaterialEffect *Mat);
		void ValidateFoliageGroupSelection(MaterialEffect *Mat);
		void EnableFoliageItem(RasterAnimHost *EditTarget, unsigned long TreeItem, bool EnabledState);
		void EditFoliageItem(RasterAnimHost *EditTarget, unsigned long TreeItem);
		RasterAnimHost *AddFoliageItem(long Item, int OpenGallery, RasterAnimHost *EditTarget, char *ActionText);
		void RemoveFoliageItem(RasterAnimHost *EditTarget);

	}; // class PortableFoliageGUI

#endif // WCS_PORTABLEFOLIAGEGUI_H
