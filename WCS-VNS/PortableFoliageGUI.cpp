// PortableFoliageGUI.cpp
// Code for foliage pages of numerous component editors
// Built from parts of EcosystemEditGUI on 4/15/08 by Gary R. Huber
// Copyright 2008 Questar Productions. All rights reserved.

#include <Windows.h>
#include <commctrl.h>
#include "stdafx.h"
#include "Application.h"
#include "Fenetre.h"
#include "PortableFoliageGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "GraphData.h"
#include "EffectsLib.h"
#include "Ecotype.h"
#include "Raster.h"
#include "resource.h"
#include "Useful.h"
#include "Requester.h"
#include "AppMem.h"
#include "Log.h"

extern WCSApp *GlobalApp;

PortableFoliageGUI::PortableFoliageGUI(GUIFenetre *FenetreSource, EffectsLib *EffectsSource,  
	ImageLib *ImageSource, AnimMaterialGradient *MatGradSource, Ecotype *EcotypeSource, long HostFolPageSource,
	long FolPanelSetSource, long EcotypePanelSource, long HostActivePageSource)
{

ConstructError = 0;
HostEdFolPage = HostFolPageSource;
FoliagePanelSet = FolPanelSetSource;
EcotypePanelSet = EcotypePanelSource;
HostActivePage = HostActivePageSource;
FenetreHost = FenetreSource;
EffectsHost = EffectsSource;
ImageHost = ImageSource;
MatGrad = MatGradSource;
ActiveGrad = NULL;
FoliagePanel = EcotypePanel = EcotypeSelection = WCS_FOLIAGE_PANEL_ECOTYPE_GENERAL;
FixedEcotype = EcotypeSource;
ActiveEcotype = NULL;
ActiveGroup = NULL;
ActiveFol = NULL;
LastSelected = NULL;
EcotypeTreeItem[1] = EcotypeTreeItem[0] = 0;

if (EffectsSource && (MatGradSource || FixedEcotype))
	{
	if (! (ObjRast = new Raster()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // PortableFoliageGUI::PortableFoliageGUI

/*===========================================================================*/

PortableFoliageGUI::~PortableFoliageGUI()
{

if (ObjRast)
	{
	ObjRast->Thumb = NULL;
	delete ObjRast;
	} // if
	
} // PortableFoliageGUI::~PortableFoliageGUI

/*===========================================================================*/

long PortableFoliageGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case IDC_LOADCOMPONENT3:
		{
		if (ActiveEcotype)
			ActiveEcotype->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT3:
		{
		if (ActiveEcotype)
			ActiveEcotype->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_LOADCOMPONENT2:
		{
		if (ActiveGroup)
			ActiveGroup->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT2:
		{
		if (ActiveGroup)
			ActiveGroup->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDECO:
		{
		AddEcotype();
		break;
		} // IDC_ADDECO
	case IDC_REMOVEECO:
		{
		RemoveEcotype();
		break;
		} // IDC_REMOVEECO
	case IDC_ADDGROUP:
		{
		AddFoliageGroup();
		break;
		} // IDC_ADDGROUP
	case IDC_REMOVEFOLIAGEGROUP:
		{
		RemoveFoliageGroup();
		break;
		} // IDC_REMOVEFOLIAGEGROUP
	case IDC_ADDFOLIAGE:
		{
		AddFoliage();
		break;
		} // IDC_ADDFOLIAGE
	case IDC_REMOVEFOLIAGE:
		{
		RemoveFoliage();
		break;
		} // IDC_REMOVEFOLIAGE
	case IDC_APPLYTOGROUP:
		{
		ApplySettingsToFoliageGroup();
		break;
		} // IDC_APPLYTOGROUP
	case IDC_EDITHTGRAPH:
		{
		if (ActiveEcotype)
			{
			if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
				ActiveEcotype->DBHCurve.EditRAHost();
			else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
				ActiveEcotype->AgeCurve.EditRAHost();
			} // if
		break;
		} // IDC_EDITHTGRAPH
	case IDC_EDITHTGRAPH2:
		{
		if (ActiveGroup && ActiveEcotype)
			{
			if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
				ActiveGroup->DBHCurve.EditRAHost();
			else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
				ActiveGroup->AgeCurve.EditRAHost();
			} // if
		break;
		} // IDC_EDITHTGRAPH2
	default:
		break;
	} // ButtonID

return(0);

} // PortableFoliageGUI::HandleButtonClick

/*===========================================================================*/

long PortableFoliageGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL:
		{
		if (ActiveFol)
			OpenFoliagePreview();
		break;
		} // 
	case IDC_TNAIL2:
		{
		if (ActiveFol)
			OpenObjectPreview();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // PortableFoliageGUI::HandleButtonDoubleClick

/*===========================================================================*/

long PortableFoliageGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_IMAGEDROP:
		{
		SelectNewFoliageImage(-1);
		break;
		}
	case IDC_OBJECTDROP:
		{
		SelectNewFoliageObject(-1);
		break;
		}
	case IDC_UNITSDROP:
		{
		SetFoliageDensityUnits();
		break;
		}
	case IDC_BAUNITSDROP:
	case IDC_BAUNITSDROP2:
		{
		SetFoliageBAUnits((unsigned short)CtrlID);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PortableFoliageGUI::HandleCBChange

/*===========================================================================*/

long PortableFoliageGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		RemoveFoliageItem((RasterAnimHost *)ItemData);
		break;
		} // IDC_GROUPLIST
	default:
		break;
	} // switch

return(0);

} // PortableFoliageGUI::HandleListDelItem

/*===========================================================================*/

long PortableFoliageGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		CopyFoliageItem((RasterAnimHost *)ItemData);
		break;
		}
	default:
		break;
	} // switch

return(0);

} // PortableFoliageGUI::HandleListCopyItem

/*===========================================================================*/

long PortableFoliageGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		PasteFoliageItem((RasterAnimHost *)ItemData);
		break;
		}
	default:
		break;
	} // switch

return(0);

} // PortableFoliageGUI::HandleListPasteItem

/*===========================================================================*/

long PortableFoliageGUI::HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		if(Derived)
			{
			// Pass along to RasterAnimHost for handling
			if(RAH)
				{
				RAH->HandlePopMenuSelection(ActionText);
				} // if
			} // if
		else
			{
			if(!stricmp(ActionText, "ENABLE"))
				{
				EnableFoliageItem(RAH, TreeItem, true);
				} // if
			else if(!stricmp(ActionText, "DISABLE"))
				{
				EnableFoliageItem(RAH, TreeItem, false);
				} // if
			else if(!stricmp(ActionText, "EDIT"))
				{
				EditFoliageItem(RAH, TreeItem);
				} // if
			else if((! stricmp(ActionText, "ADDOVERSTORY"))
				|| (! stricmp(ActionText, "ADDUNDERSTORY"))
				|| (! stricmp(ActionText, "ADDFOLIAGEGROUP"))
				|| (! stricmp(ActionText, "ADDFOLIAGE"))
				|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUP"))
				|| (! stricmp(ActionText, "ADDOVERFOLIAGE"))
				|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUP"))
				|| (! stricmp(ActionText, "ADDUNDERFOLIAGE")))
				{
				AddFoliageItem(TreeItem, 0, RAH, ActionText);
				} // if
			else if((!stricmp(ActionText, "ADDFOLIAGEGROUPGALLERY"))
				|| (! stricmp(ActionText, "ADDOVERGALLERY"))
				|| (! stricmp(ActionText, "ADDUNDERGALLERY"))
				|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUPGALLERY"))
				|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUPGALLERY")))
				{
				AddFoliageItem(TreeItem, 1, RAH, ActionText);
				} // if
			else if(!stricmp(ActionText, "COPY"))
				{
				CopyFoliageItem(RAH);
				} // if
			else if(!stricmp(ActionText, "PASTE"))
				{
				PasteFoliageItem(RAH);
				} // if
			else if(!stricmp(ActionText, "GALLERY"))
				{
				if(RAH) RAH->OpenGallery(EffectsHost);
				} // if
			else if(!stricmp(ActionText, "LOAD"))
				{
				if(RAH) RAH->LoadComponentFile(NULL);
				} // if
			else if(!stricmp(ActionText, "SIGNANDSAVE"))
				{
				if(RAH) RAH->OpenBrowseData(EffectsHost);
				} // if
			else if(!stricmp(ActionText, "DELETE"))
				{
				RemoveFoliageItem(RAH);
				} // if
			return(0);
			} // else
		break;
		}
	default:
		break;
	} // switch

return(0);
} // PortableFoliageGUI::HandleTreeMenuSelect

/*===========================================================================*/

long PortableFoliageGUI::HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		ChangeFoliageSelection(OldTreeItem, NewTreeItem, TreeItemData);
		break;
		} // IDC_FOLIAGETREE
	} // switch

return (0);

} // PortableFoliageGUI::HandleTreeChange

/*===========================================================================*/

long PortableFoliageGUI::HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand)
{

switch(CtrlID)
	{
	case IDC_FOLIAGETREE:
		{
		if (! Pre)
			ListItemExpanded(TreeItemData, Expand);
		break;
		} // IDC_FOLIAGETREE
	} // switch
	
return (0);

} // PortableFoliageGUI::HandleTreeExpand

/*===========================================================================*/

long PortableFoliageGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GROUPNAME:
		{
		FoliageGroupName();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // PortableFoliageGUI::HandleStringLoseFocus

/*===========================================================================*/

long PortableFoliageGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB3:
		{
		FoliagePanel = EcotypePanel = NewPageID;
		ShowPanels();
		} // 
	default:
		break;
	} // switch

return(0);

} // PortableFoliageGUI::HandlePageChange

/*===========================================================================*/

void PortableFoliageGUI::ShowPanels(void)
{

if (HostActivePage == HostEdFolPage)
	{
	FenetreHost->ShowPanel(FoliagePanelSet, FoliagePanel);
	if (FoliagePanel < WCS_FOLIAGE_PANEL_FOLIAGEGROUP)
		FenetreHost->ShowPanel(EcotypePanelSet, WCS_FOLIAGE_PANEL_ECOTYPE_GENERAL);
	else
		FenetreHost->ShowPanel(EcotypePanelSet, -1);
	} // if
else
	HidePanels();
	
} // PortableFoliageGUI::ShowPanels

/*===========================================================================*/

void PortableFoliageGUI::HidePanels(void)
{

FenetreHost->ShowPanel(FoliagePanelSet, -1);
FenetreHost->ShowPanel(EcotypePanelSet, -1);
	
} // PortableFoliageGUI::HidePanels

/*===========================================================================*/

long PortableFoliageGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKECOTYPEENABLED:
		{
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKECOTYPEENABLED
	case IDC_CHECKDISSOLVEENABLED:
	case IDC_CHECKRENDEROCCLUDED:
		{
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKDISSOLVEENABLED
	case IDC_CHECKGROUPENABLED:
		{
		if (ActiveGroup)
			{
			Changes[0] = MAKE_ID(ActiveGroup->GetNotifyClass(), ActiveGroup->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveGroup->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKGROUPENABLED
	case IDC_CHECKOBJENABLED:
	case IDC_CHECKIMGENABLED:
		{
		if (ActiveFol)
			{
			Changes[0] = MAKE_ID(ActiveFol->GetNotifyClass(), ActiveFol->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveFol->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKOBJENABLED
	case IDC_CHECKROTATEX:
	case IDC_CHECKROTATEY:
	case IDC_CHECKROTATEZ:
	case IDC_FLIPX:
	case IDC_3DSHADE:
		{
		if (ActiveFol)
			{
			Changes[0] = MAKE_ID(ActiveFol->GetNotifyClass(), ActiveFol->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveFol->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKROTATEX
	default:
		break;
	} // switch CtrlID

return(0);

} // PortableFoliageGUI::HandleSCChange

/*===========================================================================*/

long PortableFoliageGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOABSHTECOTYPE:
	case IDC_RADIOABSHTFOLGRP:
		{
		if (ActiveEcotype)
			{
			if (GlobalApp->ForestryAuthorized && ActiveEcotype->AbsHeightResident != ActiveEcotype->AbsDensResident)
				{
				if ((ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
					|| (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA))
					{
					ActiveEcotype->AbsDensResident = ActiveEcotype->AbsHeightResident;
					ActiveEcotype->ChangeAbsDensResident();
					} // if
				} // if
			ActiveEcotype->ChangeAbsHtResident();
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_RADIOABSDENSECOTYPE:
	case IDC_RADIOABSDENSFOLGRP:
		{
		if (ActiveEcotype)
			{
			if (GlobalApp->ForestryAuthorized && ActiveEcotype->AbsHeightResident != ActiveEcotype->AbsDensResident)
				{
				if ((ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
					|| (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA))
					{
					ActiveEcotype->AbsHeightResident = ActiveEcotype->AbsDensResident;
					ActiveEcotype->ChangeAbsHtResident();
					} // if
				} // if
			ActiveEcotype->ChangeAbsDensResident();
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_RADIOSCNDHTABS:
	case IDC_RADIOSCNDHTRELMIN:
	case IDC_RADIOSCNDHTRELRNG:
		{
		if (ActiveEcotype)
			{
			ActiveEcotype->ChangeSecondHtType(OldType);
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			OldType = ActiveEcotype->SecondHeightType;
			} // if
		break;
		} // 
	case IDC_RADIODENSAREA:
	case IDC_RADIODENSSTEMS:
	case IDC_RADIODENSBASALAREA:
	case IDC_RADIODENSCLOSURE:
		{
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_RADIOIMAGEOBJ:
	case IDC_RADIO3DOBJ:
	case IDC_RADIOIMAGEOBJ2:
	case IDC_RADIO3DOBJ2:
		{
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			BuildFoliageTree();
			} // if
		break;
		} // 
	case IDC_RADIOSIZEHEIGHT:
	case IDC_RADIOSIZEDBH:
	case IDC_RADIOSIZEAGE:
	case IDC_RADIOSIZECLOSURE:
		{
		if (ActiveEcotype)
			{
			ActiveEcotype->ChangeSizeMethod();
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_RADIODENSPOLYGON:
		{
		if (ActiveEcotype)
			{
			ActiveEcotype->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_RADIOBYPIXEL:
	case IDC_RADIOBYIMAGEHT:
		{
		if (! UserMessageOKCAN("Common Distance Dissolve", "Warning! This change will affect how all Foliage Effects and all Ecotype foliage is rendered."))
			{
			GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize = 1 - GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize;
			} // if
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PortableFoliageGUI::HandleSRChange

/*===========================================================================*/

long PortableFoliageGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_ROTATEX:
	case IDC_ROTATEY:
	case IDC_ROTATEZ:
		{
		if (ActiveFol)
			{
			// these float ints are configured for plain ol' doubles
			Changes[0] = MAKE_ID(ActiveFol->GetNotifyClass(), ActiveFol->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveFol->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_DISSOLVEREFHT:
		{
		if (ActiveEcotype)
			{
			Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PortableFoliageGUI::HandleFIChange

/*===========================================================================*/

void PortableFoliageGUI::ConfigureWidgets(void)
{

FenetreHost->WidgetTBConfig(IDC_ADDGROUP, IDI_ADDFOLGROUP, NULL);
FenetreHost->WidgetTBConfig(IDC_REMOVEFOLIAGEGROUP, IDI_DELETEFOLGROUP, NULL);
FenetreHost->WidgetTBConfig(IDC_ADDFOLIAGE, IDI_ADDFOLOBJ, NULL);
FenetreHost->WidgetTBConfig(IDC_REMOVEFOLIAGE, IDI_DELETEFOLOBJ, NULL);
FenetreHost->WidgetTBConfig(IDC_ADDECO, IDI_ADDECOTYPE, NULL);
FenetreHost->WidgetTBConfig(IDC_REMOVEECO, IDI_DELETEECOTYPE, NULL);
FenetreHost->WidgetTBConfig(IDC_LOADCOMPONENT2, IDI_GALLERY, NULL);
FenetreHost->WidgetTBConfig(IDC_SAVECOMPONENT2, IDI_FILESAVE, NULL);
FenetreHost->WidgetTBConfig(IDC_LOADCOMPONENT3, IDI_GALLERY, NULL);
FenetreHost->WidgetTBConfig(IDC_SAVECOMPONENT3, IDI_FILESAVE, NULL);

} // PortableFoliageGUI::ConfigureWidgets

/*===========================================================================*/

void PortableFoliageGUI::EcotypeTabSetup(void)
{
long Index;
char *EcotypePanelTabNames[3] = {"General", "Settings", "Parameters"};

for (Index = 0; Index < 3; Index ++)
	{
	FenetreHost->WidgetTCInsertItem(IDC_TAB3, Index, EcotypePanelTabNames[Index]);
	} // for
FenetreHost->WidgetTCSetCurSel(IDC_TAB3, EcotypePanel);

} // PortableFoliageGUI::EcotypeTabSetup

/*===========================================================================*/

void PortableFoliageGUI::BuildImageList(bool RebuildTree)
{
long Pos, CurPos = -1;
Raster *MyRast, *MatchRast;

MatchRast = (ActiveFol && ActiveFol->Img) ? ActiveFol->Img->GetRaster(): NULL;
FenetreHost->WidgetCBClear(IDC_IMAGEDROP);
FenetreHost->WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
	{
	Pos = FenetreHost->WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
	FenetreHost->WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
	if (MyRast == MatchRast)
		CurPos = Pos;
	} // for
FenetreHost->WidgetCBSetCurSel(IDC_IMAGEDROP, CurPos);
if (RebuildTree)
	BuildFoliageTree();

} // PortableFoliageGUI::BuildImageList

/*===========================================================================*/

void PortableFoliageGUI::ConfigureFoliageNail(void)
{
Raster *MatchRast;

MatchRast = (ActiveFol && ActiveFol->Img) ? ActiveFol->Img->GetRaster(): NULL;
DisableFoliageWidgets();
FenetreHost->WidgetTBConfigThumb(IDC_TNAIL, MatchRast);

} // PortableFoliageGUI::ConfigureFoliageNail

/*===========================================================================*/

void PortableFoliageGUI::BuildObjectList(bool RebuildTree)
{
long Pos, CurPos = -1;
GeneralEffect *MyEffect, *MatchEffect;

CurPos = -1;
MatchEffect = ActiveFol ? ActiveFol->Obj: NULL;
FenetreHost->WidgetCBClear(IDC_OBJECTDROP);
FenetreHost->WidgetCBInsert(IDC_OBJECTDROP, -1, "New 3D Object...");
for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
	{
	Pos = FenetreHost->WidgetCBInsert(IDC_OBJECTDROP, -1, MyEffect->GetName());
	FenetreHost->WidgetCBSetItemData(IDC_OBJECTDROP, Pos, MyEffect);
	if (MyEffect == MatchEffect)
		CurPos = Pos;
	} // for
FenetreHost->WidgetCBSetCurSel(IDC_OBJECTDROP, CurPos);
if (RebuildTree)
	BuildFoliageTree();

} // PortableFoliageGUI::BuildImageList

/*===========================================================================*/

void PortableFoliageGUI::FillUnitDrops(void)
{
int Index;
char  *Units[] = {"Hectare", "Acre", "Square Meter", "Square Foot"};
char  *BAUnits[] = {"Square Meters", "Square Feet"};
char  *BAUnits2[] = {"Sq Meters", "Sq Feet"};

for (Index = 0; Index < 4; Index ++)
	{
	FenetreHost->WidgetCBInsert(IDC_UNITSDROP, -1, Units[Index]);
	} // for
for (Index = 0; Index < 2; Index ++)
	{
	FenetreHost->WidgetCBInsert(IDC_BAUNITSDROP, -1, BAUnits[Index]);
	FenetreHost->WidgetCBInsert(IDC_BAUNITSDROP2, -1, BAUnits2[Index]);
	} // for

} // PortableFoliageGUI::FillUnitDrops

/*===========================================================================*/

void PortableFoliageGUI::ConfigureAllFoliage(void)
{

if (ActiveFoliageValid())
	OldType = ActiveEcotype->SecondHeightType;
ConfigureEcotype();
BuildFoliageTree();
	
} // PortableFoliageGUI::ConfigureAllFoliage

/*===========================================================================*/

void PortableFoliageGUI::ConfigureEcotype(void)
{
int DBHDisplay;

FenetreHost->WidgetSRConfig(IDC_RADIOBYPIXEL, IDC_RADIOBYPIXEL, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 0);
FenetreHost->WidgetSRConfig(IDC_RADIOBYPIXEL, IDC_RADIOBYIMAGEHT, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 1);

if (ActiveEcotype)
	{
	FenetreHost->WidgetSRConfig(IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTECOTYPE, &ActiveEcotype->AbsHeightResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSRConfig(IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTFOLGRP, &ActiveEcotype->AbsHeightResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);

	FenetreHost->WidgetSRConfig(IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSECOTYPE, &ActiveEcotype->AbsDensResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSRConfig(IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSFOLGRP, &ActiveEcotype->AbsDensResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);

	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTABS, &ActiveEcotype->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_MINABS);
	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELMIN, &ActiveEcotype->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_MINPCT);
	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELRNG, &ActiveEcotype->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_RANGEPCT);

	FenetreHost->WidgetSCConfig(IDC_CHECKECOTYPEENABLED, &ActiveEcotype->Enabled, SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKDISSOLVEENABLED, &ActiveEcotype->DissolveEnabled, SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKRENDEROCCLUDED, &ActiveEcotype->RenderOccluded, SCFlag_Char, NULL);
	FenetreHost->WidgetCBSetCurSel(IDC_UNITSDROP, ActiveEcotype->DensityUnits);
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP, ActiveEcotype->BasalAreaUnits);
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP2, ActiveEcotype->BasalAreaUnits);
	FenetreHost->WidgetSetDisabled(IDC_UNITSDROP, false);

	FenetreHost->WidgetSmartRAHConfig(IDC_DISSOLVECOLOR, &ActiveEcotype->DissolveColor, ActiveEcotype);

	FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMINHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT], ActiveEcotype);

	if (GlobalApp->ForestryAuthorized)
		{
		FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSSTEMS, &ActiveEcotype->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA);
		FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSBASALAREA, &ActiveEcotype->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_BASALAREA);
		FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSCLOSURE, &ActiveEcotype->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_CLOSURE);

		FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEHEIGHT, &ActiveEcotype->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_HEIGHT);
		FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEDBH, &ActiveEcotype->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_DBH);
		FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEAGE, &ActiveEcotype->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_AGE);
		FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZECLOSURE, &ActiveEcotype->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_CLOSURE);

		DBHDisplay = ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH || ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;

		if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT], ActiveEcotype);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DBH], ActiveEcotype);
			DBHDisplay = 0;
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], ActiveEcotype);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_AGE], ActiveEcotype);
			} // if

		if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY], ActiveEcotype);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_BASALAREA], ActiveEcotype);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], ActiveEcotype);
			} // if

		if (DBHDisplay)
			{
			FenetreHost->WidgetShow(IDC_DENSITY2, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
			FenetreHost->WidgetShow(IDC_ECOTYPEDENS2_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
			FenetreHost->WidgetSetDisabled(IDC_DENSITY2, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
			FenetreHost->WidgetSetDisabled(IDC_ECOTYPEDENS2_EXTRATEXT, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY2, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DBH], ActiveEcotype);
			} // if
		else
			{
			FenetreHost->WidgetSetText(IDC_DENSITY2, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS2_EXTRATEXT, "");
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY2, (RasterAnimHost *)NULL, NULL);
			FenetreHost->WidgetSetDisabled(IDC_DENSITY2, TRUE);
			FenetreHost->WidgetSetDisabled(IDC_ECOTYPEDENS2_EXTRATEXT, TRUE);
			FenetreHost->WidgetShow(IDC_DENSITY2, FALSE);
			FenetreHost->WidgetShow(IDC_ECOTYPEDENS2_EXTRATEXT, FALSE);
			} // else
		} // if
	else
		{
		FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT], ActiveEcotype);
		FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY], ActiveEcotype);
		} // else

	FenetreHost->WidgetFIConfig(IDC_DISSOLVEHEIGHT,
	 &ActiveEcotype->DissolvePixelHeight,
	  .5,
	   0.0,
		1000.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	FenetreHost->WidgetFIConfig(IDC_DISSOLVEREFHT,
	 &GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt,
	  1.0,
	   1.0,
		32767.0,
		 FIOFlag_Short,
		  NULL,
		   0);
	} // if
else
	{
	FenetreHost->WidgetFIConfig(IDC_DISSOLVEREFHT, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	FenetreHost->WidgetFIConfig(IDC_DISSOLVEHEIGHT, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY2, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMINHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_DISSOLVECOLOR, (RasterAnimHost *)NULL, NULL);

	FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSSTEMS, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSBASALAREA, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIODENSSTEMS, IDC_RADIODENSCLOSURE, NULL, 0, 0);

	FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEHEIGHT, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEDBH, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEAGE, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZECLOSURE, NULL, 0, 0);

	FenetreHost->WidgetSRConfig(IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTECOTYPE, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTFOLGRP, NULL, 0, 0);

	FenetreHost->WidgetSRConfig(IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSECOTYPE, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSFOLGRP, NULL, 0, 0);

	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTABS, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELMIN, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELRNG, NULL, 0, 0);

	FenetreHost->WidgetSCConfig(IDC_CHECKECOTYPEENABLED, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKDISSOLVEENABLED, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKRENDEROCCLUDED, NULL, 0, NULL, 0);

	FenetreHost->WidgetSetDisabled(IDC_BAUNITSDROP2, TRUE);
	FenetreHost->WidgetSetDisabled(IDC_BAUNITSDROP, TRUE);
	FenetreHost->WidgetSetDisabled(IDC_UNITSDROP, TRUE);

	FenetreHost->WidgetSetText(IDC_DENSITY2, "");
	FenetreHost->WidgetSetText(IDC_ECOTYPEDENS2_EXTRATEXT, "");
	FenetreHost->WidgetSetDisabled(IDC_ECOTYPEDENS2_EXTRATEXT, TRUE);
	FenetreHost->WidgetShow(IDC_DENSITY2, FALSE);
	FenetreHost->WidgetShow(IDC_ECOTYPEDENS2_EXTRATEXT, FALSE);
	} // else
	
ConfigureGroup();
DisableFoliageWidgets();

} // PortableFoliageGUI::ConfigureEcotype()

/*===========================================================================*/

Ecotype *PortableFoliageGUI::ActiveFoliageValid(void)
{
FoliageGroup *CurGrp;
Foliage *CurFol;
MaterialEffect *Mat;

if (FixedEcotype || ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing())))
	{
	if (FixedEcotype)
		ActiveEcotype = FixedEcotype;
	else if (ActiveEcotype != Mat->EcoFol[0] && ActiveEcotype != Mat->EcoFol[1])
		ActiveEcotype = Mat->EcoFol[0] ? Mat->EcoFol[0]: Mat->EcoFol[1];
	if (ActiveEcotype)
		{
		if (FixedEcotype || (ActiveEcotype == Mat->EcoFol[0]))
			EcotypeSelection = 0;
		else
			EcotypeSelection = 1;
		if (ActiveGroup)
			{
			// check to see if active foliage group is valid
			CurGrp = ActiveEcotype->FolGrp;
			while (CurGrp)
				{
				if (CurGrp == ActiveGroup)
					{
					// check to see if active foliage is valid
					if (ActiveFol)
						{
						// check to see if active foliage group is valid
						CurFol = ActiveGroup->Fol;
						while (CurFol)
							{
							if (CurFol == ActiveFol)
								{
								// check to see if active foliage is valid
								break;
								} // if found
							CurFol = CurFol->Next;
							} // while
						if (! CurFol)
							ActiveFol = ActiveGroup->Fol;
						} // if
					else
						ActiveFol = ActiveGroup->Fol;
					break;
					} // if found
				CurGrp = CurGrp->Next;
				} // while
			if (! CurGrp)
				{
				ActiveGroup = NULL;
				ActiveFol = NULL;
				} // if
			} // if
		if (! ActiveGroup)
			{
			ActiveGroup = ActiveEcotype->FolGrp;
			ActiveFol = NULL;
			} // if
		if (! ActiveFol && ActiveGroup)
			ActiveFol = ActiveGroup->Fol;
		} // if
	else
		{
		ActiveEcotype = NULL;
		ActiveGroup = NULL;
		ActiveFol = NULL;
		} // else
	} // if
else
	{
	ActiveEcotype = NULL;
	ActiveGroup = NULL;
	ActiveFol = NULL;
	} // else

return (ActiveEcotype);

} // PortableFoliageGUI::ActiveFoliageValid

/*===========================================================================*/

void PortableFoliageGUI::ConfigureGroup(void)
{
int DBHDisplay;

if (ActiveGroup)
	{
	FenetreHost->WidgetSetDisabled(IDC_GROUPNAME, 0);
	FenetreHost->WidgetSetModified(IDC_GROUPNAME, FALSE);
	FenetreHost->WidgetSetText(IDC_GROUPNAME, ActiveGroup->Name);

	FenetreHost->WidgetSCConfig(IDC_CHECKGROUPENABLED, &ActiveGroup->Enabled, SCFlag_Char, NULL);

	FenetreHost->WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT], ActiveGroup);

	if (GlobalApp->ForestryAuthorized)
		{
		DBHDisplay = ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH || ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;

		if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT || ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT], ActiveGroup);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH], ActiveGroup);
			DBHDisplay = 0;
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE], ActiveGroup);
			} // if

		if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA || ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY], ActiveGroup);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_BASALAREA], ActiveGroup);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
			} // if

		if (DBHDisplay)
			{
			FenetreHost->WidgetShow(IDC_GROUPDENSITY2, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			FenetreHost->WidgetShow(IDC_GROUPDENS2_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			FenetreHost->WidgetSetDisabled(IDC_GROUPDENSITY2, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			FenetreHost->WidgetSetDisabled(IDC_GROUPDENS2_EXTRATEXT, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY2, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH], ActiveGroup);
			} // if
		else
			{
			FenetreHost->WidgetSetText(IDC_GROUPDENSITY2, "");
			FenetreHost->WidgetSetText(IDC_GROUPDENS2_EXTRATEXT, "");
			FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY2, (RasterAnimHost *)NULL, NULL);
			FenetreHost->WidgetSetDisabled(IDC_GROUPDENSITY2, TRUE);
			FenetreHost->WidgetSetDisabled(IDC_GROUPDENS2_EXTRATEXT, TRUE);
			FenetreHost->WidgetShow(IDC_GROUPDENSITY2, FALSE);
			FenetreHost->WidgetShow(IDC_GROUPDENS2_EXTRATEXT, FALSE);
			} // else
		} // if
	else
		{
		FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT], ActiveGroup);
		FenetreHost->WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT], ActiveGroup);
		FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY], ActiveGroup);
		FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY2, (RasterAnimHost *)NULL, NULL);
		FenetreHost->WidgetSetDisabled(IDC_GROUPDENSITY2, TRUE);
		FenetreHost->WidgetSetDisabled(IDC_GROUPDENS2_EXTRATEXT, TRUE);
		FenetreHost->WidgetShow(IDC_GROUPDENSITY2, FALSE);
		FenetreHost->WidgetShow(IDC_GROUPDENS2_EXTRATEXT, FALSE);
		} // else
	} // if
else
	{
	FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY2, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKGROUPENABLED, NULL, 0, NULL, 0);
	FenetreHost->WidgetSetText(IDC_GROUPNAME, "");
	FenetreHost->WidgetSetDisabled(IDC_GROUPNAME, 1);
	FenetreHost->WidgetSetDisabled(IDC_GROUPDENSITY2, TRUE);
	FenetreHost->WidgetSetDisabled(IDC_GROUPDENS2_EXTRATEXT, TRUE);
	FenetreHost->WidgetShow(IDC_GROUPDENSITY2, FALSE);
	FenetreHost->WidgetShow(IDC_GROUPDENS2_EXTRATEXT, FALSE);
	} // else

ConfigureFoliage();

} // PortableFoliageGUI::ConfigureGroup

/*===========================================================================*/

void PortableFoliageGUI::ConfigureFoliage(void)
{
char TextStr[64];
Raster *MyRast, *TestRast;
Object3DEffect *TestObj;
long ListPos, Ct, NumEntries;

if (ActiveFol)
	{
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ, IDC_RADIOIMAGEOBJ, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_RASTER);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ, IDC_RADIO3DOBJ, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_OBJECT3D);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ2, IDC_RADIOIMAGEOBJ2, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_RASTER);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ2, IDC_RADIO3DOBJ2, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_OBJECT3D);

	FenetreHost->WidgetSCConfig(IDC_CHECKOBJENABLED, &ActiveFol->Enabled, SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKIMGENABLED, &ActiveFol->Enabled, SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_3DSHADE, &ActiveFol->Shade3D, SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEX, &ActiveFol->RandomRotate[0], SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEY, &ActiveFol->RandomRotate[1], SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEZ, &ActiveFol->RandomRotate[2], SCFlag_Char, NULL);
	FenetreHost->WidgetSCConfig(IDC_FLIPX, &ActiveFol->FlipX, SCFlag_Char, NULL);

	FenetreHost->WidgetSmartRAHConfig(IDC_FOLIAGEHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT], ActiveFol);
	FenetreHost->WidgetSmartRAHConfig(IDC_FOLIAGEDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY], ActiveFol);
	FenetreHost->WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING], ActiveFol);
	FenetreHost->WidgetSmartRAHConfig(IDC_REPLACECOLOR, &ActiveFol->Color, ActiveFol);

	FenetreHost->WidgetSmartRAHConfig(IDC_OBJECTHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT], ActiveFol);
	FenetreHost->WidgetSmartRAHConfig(IDC_OBJECTDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY], ActiveFol);

	FenetreHost->WidgetFIConfig(IDC_ROTATEX,
	 &ActiveFol->Rotate[0],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	FenetreHost->WidgetFIConfig(IDC_ROTATEY,
	 &ActiveFol->Rotate[1],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	FenetreHost->WidgetFIConfig(IDC_ROTATEZ,
	 &ActiveFol->Rotate[2],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	FenetreHost->WidgetSetDisabled(IDC_OBJECTDROP, 0);
	FenetreHost->WidgetSetDisabled(IDC_IMAGEDROP, 0);
	FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 0);
	FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ, 0);
	FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ2, 0);
	FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ2, 0);
	FenetreHost->WidgetSetDisabled(IDC_APPLYTOGROUP, 0);
	FenetreHost->WidgetSetDisabled(IDC_TNAIL, 0);
	FenetreHost->WidgetSetDisabled(IDC_TNAIL2, 0);

	if (ActiveFol->Img && (MyRast = ActiveFol->Img->GetRaster()))
		{
		sprintf(TextStr, "%d", MyRast->GetWidth());
		FenetreHost->WidgetSetText(IDC_IMAGEWIDTH, TextStr);
		sprintf(TextStr, "%d", MyRast->GetHeight());
		FenetreHost->WidgetSetText(IDC_IMAGEHEIGHT, TextStr);
		ListPos = -1;
		NumEntries = FenetreHost->WidgetCBGetCount(IDC_IMAGEDROP);
		for (Ct = 0; Ct < NumEntries; Ct ++)
			{
			if ((TestRast = (Raster *)FenetreHost->WidgetCBGetItemData(IDC_IMAGEDROP, Ct)) != (Raster *)LB_ERR && TestRast == MyRast)
				{
				ListPos = Ct;
				break;
				} // for
			} // for
		FenetreHost->WidgetCBSetCurSel(IDC_IMAGEDROP, ListPos);
		FenetreHost->WidgetTBConfigThumb(IDC_TNAIL, MyRast);
		} // if
	else
		{
		FenetreHost->WidgetSetText(IDC_IMAGEWIDTH, "");
		FenetreHost->WidgetSetText(IDC_IMAGEHEIGHT, "");
		FenetreHost->WidgetSetText(IDC_IMAGECOLORS, "");
		FenetreHost->WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
		FenetreHost->WidgetTBConfig(IDC_TNAIL, NULL, NULL, NULL);
		} // else

	if (ActiveFol->Obj)
		{
		sprintf(TextStr, "%d", ActiveFol->Obj->NumPolys);
		FenetreHost->WidgetSetText(IDC_POLYGONS, TextStr);
		sprintf(TextStr, "%d", ActiveFol->Obj->NumVertices);
		FenetreHost->WidgetSetText(IDC_VERTICES, TextStr);
		ListPos = -1;
		NumEntries = FenetreHost->WidgetCBGetCount(IDC_OBJECTDROP);
		for (Ct = 0; Ct < NumEntries; Ct ++)
			{
			if ((TestObj = (Object3DEffect *)FenetreHost->WidgetCBGetItemData(IDC_OBJECTDROP, Ct)) != (Object3DEffect *)LB_ERR && TestObj == ActiveFol->Obj)
				{
				ListPos = Ct;
				break;
				} // for
			} // for
		FenetreHost->WidgetCBSetCurSel(IDC_OBJECTDROP, ListPos);
		if (ActiveFol->Obj->BrowseInfo)
			{
			ObjRast->Thumb = ActiveFol->Obj->BrowseInfo->Thumb;
			FenetreHost->WidgetTBConfigThumb(IDC_TNAIL2, ObjRast);
			} // if
		else
			FenetreHost->WidgetTBConfig(IDC_TNAIL2, NULL, NULL, NULL);
		} // if
	else
		{
		FenetreHost->WidgetSetText(IDC_POLYGONS, "");
		FenetreHost->WidgetSetText(IDC_VERTICES, "");
		FenetreHost->WidgetCBSetCurSel(IDC_OBJECTDROP, -1);
		FenetreHost->WidgetTBConfig(IDC_TNAIL2, NULL, NULL, NULL);
		} // else
	if (FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE || FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT)
		{
		if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
		else
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT;
		} // if
	} // if
else
	{
	FenetreHost->WidgetSmartRAHConfig(IDC_FOLIAGEHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_FOLIAGEDENSITY, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_OBJECTHEIGHT, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_OBJECTDENSITY, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(IDC_REPLACECOLOR, (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetFIConfig(IDC_ROTATEX, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	FenetreHost->WidgetFIConfig(IDC_ROTATEY, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	FenetreHost->WidgetFIConfig(IDC_ROTATEZ, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ, IDC_RADIOIMAGEOBJ, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ, IDC_RADIO3DOBJ, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ2, IDC_RADIOIMAGEOBJ2, NULL, 0, 0);
	FenetreHost->WidgetSRConfig(IDC_RADIOIMAGEOBJ2, IDC_RADIO3DOBJ2, NULL, 0, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKOBJENABLED, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKIMGENABLED, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_3DSHADE, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEX, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEY, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_CHECKROTATEZ, NULL, 0, NULL, 0);
	FenetreHost->WidgetSCConfig(IDC_FLIPX, NULL, 0, NULL, 0);
	FenetreHost->WidgetSetDisabled(IDC_OBJECTDROP, 1);
	FenetreHost->WidgetSetDisabled(IDC_IMAGEDROP, 1);
	FenetreHost->WidgetSetDisabled(IDC_APPLYTOGROUP, 1);
	FenetreHost->WidgetSetDisabled(IDC_TNAIL, 1);
	FenetreHost->WidgetSetDisabled(IDC_TNAIL2, 1);
	} // else

} // PortableFoliageGUI::ConfigureFoliage

/*===========================================================================*/

void PortableFoliageGUI::SyncFoliageWidgets(void)
{

FenetreHost->WidgetSRSync(IDC_RADIOBYPIXEL, WP_SRSYNC_NONOTIFY);
FenetreHost->WidgetSRSync(IDC_RADIOBYIMAGEHT, WP_SRSYNC_NONOTIFY);

if (ActiveFoliageValid())
	{
	FenetreHost->WidgetSCSync(IDC_CHECKDISSOLVEENABLED, WP_SCSYNC_NONOTIFY);
	FenetreHost->WidgetSCSync(IDC_CHECKRENDEROCCLUDED, WP_SCSYNC_NONOTIFY);
	FenetreHost->WidgetSCSync(IDC_CHECKECOTYPEENABLED, WP_SCSYNC_NONOTIFY);
	FenetreHost->WidgetCBSetCurSel(IDC_UNITSDROP, ActiveEcotype->DensityUnits);
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP, ActiveEcotype->BasalAreaUnits);
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP2, ActiveEcotype->BasalAreaUnits);

	FenetreHost->WidgetSNSync(IDC_ECOTYPEMAXHEIGHT, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetSNSync(IDC_ECOTYPEMINHEIGHT, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetSNSync(IDC_DENSITY, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetSNSync(IDC_DISSOLVECOLOR, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetFISync(IDC_DISSOLVEHEIGHT, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetFISync(IDC_DISSOLVEREFHT, WP_FISYNC_NONOTIFY);
	FenetreHost->WidgetFISync(IDC_DISSOLVECOLOR, WP_FISYNC_NONOTIFY);

	FenetreHost->WidgetSRSync(IDC_RADIOABSHTECOTYPE, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOABSHTFOLGRP, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOABSDENSECOTYPE, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOABSDENSFOLGRP, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOSCNDHTABS, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOSCNDHTRELMIN, WP_SRSYNC_NONOTIFY);
	FenetreHost->WidgetSRSync(IDC_RADIOSCNDHTRELRNG, WP_SRSYNC_NONOTIFY);

	if (GlobalApp->ForestryAuthorized)
		{
		if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT], ActiveEcotype);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DBH], ActiveEcotype);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], ActiveEcotype);
			} // if
		else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_AGE], ActiveEcotype);
			} // if

		if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY], ActiveEcotype);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_BASALAREA], ActiveEcotype);
			} // if
		else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSmartRAHConfig(IDC_DENSITY, &ActiveEcotype->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], ActiveEcotype);
			} // if
		FenetreHost->WidgetSRSync(IDC_RADIODENSSTEMS, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIODENSBASALAREA, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIODENSCLOSURE, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIOSIZEHEIGHT, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIOSIZEDBH, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIOSIZEAGE, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSRSync(IDC_RADIOSIZECLOSURE, WP_SRSYNC_NONOTIFY);
		FenetreHost->WidgetSNSync(IDC_DENSITY2, WP_FISYNC_NONOTIFY);
		} // if

	if (ActiveGroup)
		{
		FenetreHost->WidgetSCSync(IDC_CHECKGROUPENABLED, WP_SCSYNC_NONOTIFY);
		FenetreHost->WidgetSNSync(IDC_GROUPHEIGHT, WP_FISYNC_NONOTIFY);
		FenetreHost->WidgetSNSync(IDC_GROUPMINHEIGHT, WP_FISYNC_NONOTIFY);
		FenetreHost->WidgetSNSync(IDC_GROUPDENSITY, WP_FISYNC_NONOTIFY);

		if (GlobalApp->ForestryAuthorized)
			{
			FenetreHost->WidgetSNSync(IDC_GROUPDENSITY2, WP_FISYNC_NONOTIFY);
			if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT || ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT], ActiveGroup);
				} // if
			else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH], ActiveGroup);
				} // if
			else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
				} // if
			else if (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE], ActiveGroup);
				} // if

			if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA || ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY], ActiveGroup);
				} // if
			else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_BASALAREA], ActiveGroup);
				} // if
			else if (ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
				{
				FenetreHost->WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
				} // if
			} // if

		if (ActiveFol)
			{
			FenetreHost->WidgetSRSync(IDC_RADIOIMAGEOBJ, WP_SRSYNC_NONOTIFY);
			FenetreHost->WidgetSRSync(IDC_RADIO3DOBJ, WP_SRSYNC_NONOTIFY);
			FenetreHost->WidgetSRSync(IDC_RADIOIMAGEOBJ2, WP_SRSYNC_NONOTIFY);
			FenetreHost->WidgetSRSync(IDC_RADIO3DOBJ2, WP_SRSYNC_NONOTIFY);

			FenetreHost->WidgetSCSync(IDC_CHECKOBJENABLED, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_CHECKIMGENABLED, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_3DSHADE, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_CHECKROTATEX, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_CHECKROTATEY, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_CHECKROTATEZ, WP_SCSYNC_NONOTIFY);
			FenetreHost->WidgetSCSync(IDC_FLIPX, WP_SCSYNC_NONOTIFY);

			FenetreHost->WidgetSNSync(IDC_FOLIAGEHEIGHT, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetSNSync(IDC_FOLIAGEDENSITY, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetSNSync(IDC_ORIENTATIONSHADING, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetSNSync(IDC_OBJECTHEIGHT, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetSNSync(IDC_OBJECTDENSITY, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetSNSync(IDC_REPLACECOLOR, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetFISync(IDC_ROTATEX, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetFISync(IDC_ROTATEY, WP_FISYNC_NONOTIFY);
			FenetreHost->WidgetFISync(IDC_ROTATEZ, WP_FISYNC_NONOTIFY);
			if (FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE || FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT)
				{
				if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
					FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
				else
					FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT;
				} // if
			ShowPanels();
			} // if
		} // if
	SyncFoliageTreeNames();
	} // if

} // PortableFoliageGUI::SyncFoliageWidgets

/*===========================================================================*/

void PortableFoliageGUI::BuildFoliageTree(void)
{
unsigned long Flags = 0;
long EcotypeCt;
Ecotype *CurEtp;
FoliageGroup *CurGrp;
Foliage *CurFol;
MaterialEffect *Mat;
void *Parent, *FolParent, *FolInsert;
char ListName[512], Expanded, Enabled, Selected;
RasterAnimHostProperties Prop;

FenetreHost->WidgetTVDeleteAll(IDC_FOLIAGETREE);

if (FixedEcotype || ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing())))
	{
	for (EcotypeCt = 0; EcotypeCt < (FixedEcotype ? 1: 2); ++EcotypeCt)
		{
		if (EcotypeCt == 0)
			{
			if (FixedEcotype)
				{
				Prop.PropMask =	WCS_RAHOST_MASKBIT_NAME;
				FixedEcotype->GetRAHostRoot()->GetRAHostProperties(&Prop);
				sprintf(ListName, "Foliage Effect (%s)", Prop.Name);
				CurEtp = FixedEcotype;
				} // if
			else
				{
				sprintf(ListName, "Overstory (%s)", Mat->GetName());
				CurEtp = Mat->EcoFol[0];
				} // else
			} // if
		else
			{
			sprintf(ListName, "Understory (%s)", Mat->GetName());
			CurEtp = Mat->EcoFol[1];
			} // else
		if (CurEtp)
			{
			CurEtp->GetExpansionFlags(WCS_RAHOST_FLAGBIT_EXPANDED1, Flags);
			Expanded = (Flags & WCS_RAHOST_FLAGBIT_EXPANDED1) ? 1: 0;
			Enabled = CurEtp->Enabled ? 1: 0;
			Selected = ActiveEcotype == CurEtp && FoliagePanel < WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
			} // if
		else
			Expanded = Enabled = Selected = 0;
		if (EcotypeTreeItem[EcotypeCt] = Parent = (void *)FenetreHost->WidgetTVInsertExpanded(IDC_FOLIAGETREE, ListName, CurEtp, NULL, (CurEtp && CurEtp->FolGrp) ? 1: 0, Expanded))
			{
			FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, Parent, Enabled);
			if (Selected)
				{
				FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, Parent, Selected);
				LastSelected = Parent;
				} // if
			if (CurEtp)
				{
				for (CurGrp = CurEtp->FolGrp; CurGrp; CurGrp = CurGrp->Next)
					{
					CurGrp->GetExpansionFlags(WCS_RAHOST_FLAGBIT_EXPANDED1, Flags);
					Expanded = (Flags & WCS_RAHOST_FLAGBIT_EXPANDED1) ? 1: 0;
					Enabled = CurGrp->Enabled ? 1: 0;
					Selected = ActiveGroup == CurGrp && FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
					if (FolParent = (void *)FenetreHost->WidgetTVInsertExpanded(IDC_FOLIAGETREE, CurGrp->Name, CurGrp, Parent, CurGrp->Fol ? 1: 0, Expanded))
						{
						FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, FolParent, Enabled);
						if (Selected)
							{
							FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, FolParent, Selected);
							LastSelected = FolParent;
							} // if
						for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
							{
							Selected = ActiveFol == CurFol && FoliagePanel > WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
							if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
								{
								if (CurFol->Img && CurFol->Img->Rast)
									{
									Enabled = CurFol->Enabled && CurFol->Img->GetRaster()->Enabled ? 1: 0;
									if (FolInsert = FenetreHost->WidgetTVInsert(IDC_FOLIAGETREE, CurFol->Img->GetRaster()->GetUserName(), CurFol, FolParent, 0))
										FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, FolInsert, Enabled);
									} // if
								else
									FolInsert = FenetreHost->WidgetTVInsert(IDC_FOLIAGETREE, "No Image Selected", CurFol, FolParent, 0);
								} // if
							else if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
								{
								if (CurFol->Obj)
									{
									Enabled = CurFol->Enabled && CurFol->Obj->Enabled ? 1: 0;
									if (FolInsert = FenetreHost->WidgetTVInsert(IDC_FOLIAGETREE, CurFol->Obj->GetName(), CurFol, FolParent, 0))
										FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, FolInsert, Enabled);
									} // if
								else
									FolInsert = FenetreHost->WidgetTVInsert(IDC_FOLIAGETREE, "No Object Selected", CurFol, FolParent, 0);
								} // else
							if (Selected && FolInsert)
								{
								FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, FolInsert, Selected);
								LastSelected = FolInsert;
								} // if
							} // for
						} // if
					} // for
				} // if
			} // if
		} // for
	} // if
	
} // PortableFoliageGUI::BuildFoliageTree

/*===========================================================================*/

void PortableFoliageGUI::SyncFoliageTreeNames(void)
{
void *Parent, *FolParent, *FolItem;
Ecotype *CurEco;
FoliageGroup *CurGrp;
Foliage *CurFol;
char NameTxt[WCS_EFFECT_MAXNAMELENGTH];
char Enabled;

for (Parent = FenetreHost->WidgetTVGetRoot(IDC_FOLIAGETREE); Parent; Parent = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, Parent))
	{
	if (CurEco = (Ecotype *)FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, Parent))
		{
		Enabled = CurEco->Enabled ? 1: 0;
		FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, Parent, Enabled);
		} // if
	for (FolParent = FenetreHost->WidgetTVGetChild(IDC_FOLIAGETREE, Parent); FolParent; FolParent = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, FolParent))
		{
		if (CurGrp = (FoliageGroup *)FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, FolParent))
			{
			Enabled = CurGrp->Enabled ? 1: 0;
			FenetreHost->WidgetTVGetItemText(IDC_FOLIAGETREE, FolParent, NameTxt);
			if (strcmp(CurGrp->Name, NameTxt))
				FenetreHost->WidgetTVSetItemText(IDC_FOLIAGETREE, FolParent, CurGrp->Name);
			FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, FolParent, Enabled);
			} // if
		for (FolItem = FenetreHost->WidgetTVGetChild(IDC_FOLIAGETREE, FolParent); FolItem; FolItem = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, FolItem))
			{
			if (CurFol = (Foliage *)FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, FolItem))
				{
				Enabled = CurFol->Enabled ? 1: 0;
				FenetreHost->WidgetTVSetBoldState(IDC_FOLIAGETREE, FolItem, Enabled);
				} // if
			} // for
		} // for
	} // for
	
} // PortableFoliageGUI::SyncFoliageTreeNames

/*===========================================================================*/

void PortableFoliageGUI::DisableFoliageWidgets(void)
{

FenetreHost->WidgetShow(IDC_ADDECO, ! FixedEcotype);
FenetreHost->WidgetShow(IDC_REMOVEECO, ! FixedEcotype);

if (ActiveEcotype)
	{
	FenetreHost->WidgetSetDisabled(IDC_DISSOLVEHEIGHT, ! ActiveEcotype->DissolveEnabled);
	FenetreHost->WidgetSetDisabled(IDC_DISSOLVEREFHT, ! GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize);
	FenetreHost->WidgetSetDisabled(IDC_DISSOLVECOLOR, ! ActiveEcotype->DissolveEnabled);
	FenetreHost->WidgetShow(IDC_DISSOLVECOLOR, ! FixedEcotype);

	FenetreHost->WidgetSetDisabled(IDC_ECOTYPEMAXHEIGHT, ActiveEcotype->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_ECOTYPESIZE_EXTRATEXT, ActiveEcotype->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_ECOTYPEMINHEIGHT, ActiveEcotype->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_ECOTYPESIZE2_EXTRATEXT, ActiveEcotype->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_GROUPMINHEIGHT, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_GROUPSIZE2_EXTRATEXT, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_DENSITY, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	FenetreHost->WidgetSetDisabled(IDC_ECOTYPEDENS2_EXTRATEXT, ActiveEcotype->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	
	if (GlobalApp->ForestryAuthorized)
		{
		FenetreHost->WidgetSetDisabled(IDC_BAUNITSDROP, ! (ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA));
		FenetreHost->WidgetSetDisabled(IDC_BAUNITSDROP2, ! (ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA));
		FenetreHost->WidgetShow(IDC_BAUNITSDROP_LABEL, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		FenetreHost->WidgetShow(IDC_BAUNITSDROP, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		FenetreHost->WidgetShow(IDC_BAUNITSDROP_LABEL2, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
		FenetreHost->WidgetShow(IDC_BAUNITSDROP2, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);

		FenetreHost->WidgetSetDisabled(IDC_RADIODENSBASALAREA, (ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && ActiveEcotype->AbsDensResident != ActiveEcotype->AbsHeightResident));
		FenetreHost->WidgetSetDisabled(IDC_RADIOSIZEDBH, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && ActiveEcotype->AbsDensResident != ActiveEcotype->AbsHeightResident);
		// crown closure is available only when size and density are in same level
		FenetreHost->WidgetSetDisabled(IDC_RADIODENSCLOSURE, ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || ActiveEcotype->AbsDensResident != ActiveEcotype->AbsHeightResident);
		FenetreHost->WidgetSetDisabled(IDC_RADIOSIZECLOSURE, ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE || ActiveEcotype->AbsDensResident != ActiveEcotype->AbsHeightResident);
		// basal area density and dbh size are available together only when size and density are in same level
		} // if

	if (GlobalApp->ForestryAuthorized && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
		{
		if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Basal Area ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(BA/Hectare)": "");
			} // if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Basal Area ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(BA/Acre)": "");
			} // else if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Basal Area ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(BA/Sq Meter)": "");
			} // else if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Basal Area ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(BA/Sq Foot)": "");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Basal Area ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(BA/Unit Area)": "");
			} // else
		} // if
	else if (GlobalApp->ForestryAuthorized && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		FenetreHost->WidgetSetText(IDC_DENSITY, "Crown Closure ");
		FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(%)": "");
		} // if
	else
		{
		if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Density ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(Stems/Hectare)": "");
			} // if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Density ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(Stems/Acre)": "");
			} // else if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Density ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(Stems/Sq Meter)": "");
			} // else if
		else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Density ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(Stems/Sq Foot)": "");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_DENSITY, "Density ");
			FenetreHost->WidgetSetText(IDC_ECOTYPEDENS_EXTRATEXT, ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE ? "(Stems/Unit Area)": "");
			} // else
		} // if

	if (ActiveEcotype->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
		{
		if (GlobalApp->ForestryAuthorized && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Basal Area ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(BA/Hectare)");
				} // if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Basal Area ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(BA/Acre)");
				} // else if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Basal Area ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(BA/Sq Meter)");
				} // else if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Basal Area ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(BA/Sq Foot)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Basal Area ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(BA/Unit Area)");
				} // else
			} // if
		else if (GlobalApp->ForestryAuthorized && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Crown Closure ");
			FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(%)");
			} // if
		else
			{
			if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(Stems/Hectare)");
				} // if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(Stems/Acre)");
				} // else if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(Stems/Sq Meter)");
				} // else if
			else if (ActiveEcotype->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(Stems/Sq Foot)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
				FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(Stems/Unit Area)");
				} // else
			} // else
		} // if
	else
		{
		FenetreHost->WidgetSetText(IDC_GROUPDENSITY, "Group Density ");
		FenetreHost->WidgetSetText(IDC_GROUPDENS_EXTRATEXT, "(% of Ecotype)");
		} // else

	if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum DBH ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum DBH ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "");
			} // if
		else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum DBH ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min DBH ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(% of Max)");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean DBH ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "DBH Range ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(+/-%)");
			} // else if
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH2, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
		FenetreHost->WidgetSetText(IDC_EDITHTGRAPH, "Edit DBH/Height Graph...");
		FenetreHost->WidgetSetText(IDC_EDITHTGRAPH2, "Edit DBH/Height Graph...");
		} // if
	else if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Age ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Age ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "");
			} // if
		else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Age ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Age ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(% of Max)");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Age ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Age Range ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(+/-%)");
			} // else if
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH2, ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
		FenetreHost->WidgetSetText(IDC_EDITHTGRAPH, "Edit Age/Height Graph...");
		FenetreHost->WidgetSetText(IDC_EDITHTGRAPH2, "Edit Age/Height Graph...");
		} // if
	else if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
		{
		if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Closure ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "(%)");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Closure ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(%)");
			} // if
		else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Closure ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "(%)");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Closure ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(% of Max)");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Closure ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "(%)");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Closure Range ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(+/-%)");
			} // else if
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH, FALSE);
		FenetreHost->WidgetShow(IDC_EDITHTGRAPH2, FALSE);
		} // if
	else
		{
		if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Height ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Height ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "");
			} // if
		else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Height ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Height ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(% of Max)");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Height ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE_EXTRATEXT, "");
			FenetreHost->WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Height Range ");
			FenetreHost->WidgetSetText(IDC_ECOTYPESIZE2_EXTRATEXT, "(+/-%)");
			} // else if
		if (GlobalApp->ForestryAuthorized)
			{
			FenetreHost->WidgetShow(IDC_EDITHTGRAPH, FALSE);
			FenetreHost->WidgetShow(IDC_EDITHTGRAPH2, FALSE);
			} // If
		} // if

	if (GlobalApp->ForestryAuthorized && ActiveEcotype->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA
		&& ActiveEcotype->SizeMethod != WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		FenetreHost->WidgetSetText(IDC_DENSITY2, "Diameter (DBH) ");
		FenetreHost->WidgetSetText(IDC_ECOTYPEDENS2_EXTRATEXT, "");
		FenetreHost->WidgetSetText(IDC_GROUPDENSITY2, "Diameter (DBH) ");
		FenetreHost->WidgetSetText(IDC_GROUPDENS2_EXTRATEXT, "");
		} // if
	else
		{
		FenetreHost->WidgetSetText(IDC_DENSITY2, "");
		FenetreHost->WidgetSetText(IDC_ECOTYPEDENS2_EXTRATEXT, "");
		FenetreHost->WidgetSetText(IDC_GROUPDENSITY2, "");
		FenetreHost->WidgetSetText(IDC_GROUPDENS2_EXTRATEXT, "");
		} // else
		
	if (ActiveEcotype->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
		{
		FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Height ");
		FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "(% of Ecotype)");
		if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Height ");
			FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "");
			} // if
		else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Height ");
			FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(% of Max)");
			} // else if
		else
			{
			FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Ht. Range ");
			FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(+/-%)");
			} // else if
		} // if
	else
		{
		if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. DBH ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. DBH ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "");
				} // if
			else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. DBH ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. DBH ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(% of Max)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Mean DBH ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group DBH Range ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(+/-%)");
				} // else if
			} // if
		else if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Age ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Age ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "");
				} // if
			else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Age ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Age ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(% of Max)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Age ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Age Range ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(+/-%)");
				} // else if
			} // else if
		else if (GlobalApp->ForestryAuthorized && ActiveEcotype->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Closure ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "(%)");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Closure ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(%)");
				} // if
			else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Closure ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "(%)");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Closure ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(% of Max)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Closure ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Closure Rng. ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(+/-%)");
				} // else if
			} // else if
		else
			{
			if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Height ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Height ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "");
				} // if
			else if (ActiveEcotype->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Max. Height ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min. Height ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(% of Max)");
				} // else if
			else
				{
				FenetreHost->WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Height ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE_EXTRATEXT, "");
				FenetreHost->WidgetSetText(IDC_GROUPMINHEIGHT, "Group Ht. Range ");
				FenetreHost->WidgetSetText(IDC_GROUPSIZE2_EXTRATEXT, "(+/-%)");
				} // else if
			} // else
		} // else

	if (ActiveFol)
		{
		FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 0);
		FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ, 0);
		FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ2, 0);
		FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ2, 0);
		} // if
	else
		{
		FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 1);
		FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ, 1);
		FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ2, 1);
		FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ2, 1);
		} // else
	} // if
else
	{
	FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 1);
	FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ, 1);
	FenetreHost->WidgetSetDisabled(IDC_RADIOIMAGEOBJ2, 1);
	FenetreHost->WidgetSetDisabled(IDC_RADIO3DOBJ2, 1);
	} // else

} // PortableFoliageGUI::DisableFoliageWidgets

/*===========================================================================*/

void PortableFoliageGUI::SelectNewFoliageImage(long Current)
{
Raster *NewRast, *MadeRast = NULL;
Raster **ManyRasters = NULL;
long NumMany = 0, ManyCt;

if (ActiveFol)
	{
	if (Current < 0)
		Current = FenetreHost->WidgetCBGetCurSel(IDC_IMAGEDROP);
	if (((NewRast = (Raster *)FenetreHost->WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)LB_ERR && NewRast)
		|| (MadeRast = NewRast = ImageHost->AddRequestRaster(0, &ManyRasters, &NumMany)))
		{
		ActiveFol->SetRaster(NewRast);
		if (MadeRast)
			{
			ImageHost->SetActive(MadeRast);
			} // if
		if (ManyRasters && ActiveGroup)
			{
			Foliage *NewFol;
			for (ManyCt = 0; ManyCt < NumMany; ++ManyCt)
				{
				if ((NewRast = ManyRasters[ManyCt]) != MadeRast)
					{
					// add new foliage object
					if (NewFol = ActiveGroup->AddFoliage(NULL))
						NewFol->SetRaster(NewRast);
					} // if
				} // for
			AppMem_Free(ManyRasters, NumMany * sizeof (Raster *));
			} // if
		} // if
	} // if

} // PortableFoliageGUI::SelectNewFoliageImage

/*===========================================================================*/

void PortableFoliageGUI::SelectNewFoliageObject(long Current)
{
Object3DEffect *OldObj, *NewObj, *MadeObj = NULL;
NotifyTag Changes[2];

if (ActiveFol)
	{
	if (Current < 0)
		Current = FenetreHost->WidgetCBGetCurSel(IDC_OBJECTDROP);
	if (((NewObj = (Object3DEffect *)FenetreHost->WidgetCBGetItemData(IDC_OBJECTDROP, Current, 0)) != (Object3DEffect *)LB_ERR && NewObj)
		|| (MadeObj = NewObj = (Object3DEffect *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, NULL, NULL)))
		{
		OldObj = ActiveFol->Obj;
		if (MadeObj)
			{
			if (! MadeObj->OpenInputFileRequest())
				{
				Changes[0] = MAKE_ID(MadeObj->GetNotifyClass(), MadeObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				Changes[1] = NULL;
				EffectsHost->RemoveEffect(MadeObj);
				MadeObj = NULL;
				NewObj = OldObj;
				GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			} // if
		ActiveFol->SetObject(NewObj);
		} // if
	} // if

} // PortableFoliageGUI::SelectNewFoliageObject

/*===========================================================================*/

void PortableFoliageGUI::EditFoliageObject(void)
{

if (ActiveFoliageValid() && ActiveFol)
	{
	if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
		{
		if (! ActiveFol->Img || ! ActiveFol->Img->GetRaster())
			{
			SelectNewFoliageImage(0);
			}
		if (ActiveFol->Img && ActiveFol->Img->GetRaster())
			ActiveFol->Img->GetRaster()->EditRAHost();
		} // if
	else if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
		{
		if (! ActiveFol->Obj)
			{
			SelectNewFoliageObject(0);
			}
		if (ActiveFol->Obj)
			ActiveFol->Obj->EditRAHost();
		} // if
	} // if
else
	ConfigureFoliage();

} // PortableFoliageGUI::EditFoliageObject

/*===========================================================================*/

void PortableFoliageGUI::SetFoliageDensityUnits(void)
{
long Current;
NotifyTag Changes[2];

if (ActiveFoliageValid())
	{
	Current = FenetreHost->WidgetCBGetCurSel(IDC_UNITSDROP);
	ActiveEcotype->DensityUnits = (unsigned char)Current;
	FenetreHost->WidgetCBSetCurSel(IDC_UNITSDROP, ActiveEcotype->DensityUnits);
	Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
	} // if
	
} // PortableFoliageGUI::SetFoliageDensityUnits()

/*===========================================================================*/

void PortableFoliageGUI::SetFoliageBAUnits(unsigned short WidID)
{
long Current;
NotifyTag Changes[2];

if (ActiveFoliageValid())
	{
	Current = FenetreHost->WidgetCBGetCurSel(WidID);
	ActiveEcotype->BasalAreaUnits = (unsigned char)Current;
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP, ActiveEcotype->BasalAreaUnits);
	FenetreHost->WidgetCBSetCurSel(IDC_BAUNITSDROP2, ActiveEcotype->BasalAreaUnits);
	Changes[0] = MAKE_ID(ActiveEcotype->GetNotifyClass(), ActiveEcotype->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveEcotype->GetRAHostRoot());
	} // if

} // PortableFoliageGUI::SetFoliageBAUnits()

/*===========================================================================*/

void PortableFoliageGUI::FoliageGroupName(void)
{
char Name[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (ActiveFoliageValid() && ActiveGroup && FenetreHost->WidgetGetModified(IDC_GROUPNAME))
	{
	FenetreHost->WidgetGetText(IDC_GROUPNAME, WCS_EFFECT_MAXNAMELENGTH, Name);
	FenetreHost->WidgetSetModified(IDC_GROUPNAME, FALSE);
	strncpy(ActiveGroup->Name, Name, WCS_EFFECT_MAXNAMELENGTH);
	ActiveGroup->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Changes[0] = MAKE_ID(ActiveGroup->GetNotifyClass(), ActiveGroup->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveGroup->GetRAHostRoot());
	} // if 

} // PortableFoliageGUI::FoliageGroupName()

/*===========================================================================*/

void PortableFoliageGUI::RemoveEcotype(void)
{
Ecotype *EcotypeToRemove;
MaterialEffect *Mat;
int RemoveAll = 0;

if (! FixedEcotype)
	{
	if ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing()))
		{
		FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, LastSelected, false);
		if (ActiveFoliageValid())
			{
			EcotypeToRemove = ActiveEcotype;
			if (ActiveEcotype == Mat->EcoFol[0] && Mat->EcoFol[1])
				{
				ActiveEcotype = Mat->EcoFol[1];
				EcotypeSelection = 1;
				} // else if
			else
				{
				ActiveEcotype = Mat->EcoFol[0];
				EcotypeSelection = 0;
				} // else
			ActiveGroup = NULL;
			ActiveFol = NULL;
			FoliagePanel = EcotypePanel;
			ShowPanels();
			Mat->FindnRemoveRAHostChild(EcotypeToRemove, RemoveAll);
			ActiveFoliageValid();
			FoliagePanel = EcotypePanel;
			ShowPanels();
			BuildFoliageTree();
			} // if
		} // if
	} // if
	
} // PortableFoliageGUI::RemoveEcotype

/*===========================================================================*/

void PortableFoliageGUI::RemoveFoliageGroup(void)
{
FoliageGroup *GroupToRemove;
int RemoveAll = 0;

if (ActiveFoliageValid() && ActiveGroup)
	{
	GroupToRemove = ActiveGroup;
	if (! (ActiveGroup = ActiveGroup->Next))
		ActiveGroup = ActiveEcotype->FolGrp;
	if (ActiveGroup)
		FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
	else
		FoliagePanel = EcotypePanel;
	ShowPanels();
	ActiveEcotype->FindnRemoveRAHostChild(GroupToRemove, RemoveAll);
	} // if

} // PortableFoliageGUI::RemoveFoliageGroup

/*===========================================================================*/

void PortableFoliageGUI::RemoveFoliage(void)
{
Foliage *RemoveFol;
int RemoveAll = 0;

if (ActiveFoliageValid() && ActiveFol)
	{
	RemoveFol = ActiveFol;
	if (! (ActiveFol = ActiveFol->Next))
		ActiveFol = ActiveGroup->Fol;
	if (ActiveFol && (FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE || FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT))
		{
		if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
		else
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT;
		} // if
	else if (ActiveGroup && FoliagePanel >= WCS_FOLIAGE_PANEL_FOLIAGEGROUP)
		FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
	else
		FoliagePanel = EcotypePanel;
	ShowPanels();
	ActiveEcotype->FindnRemoveRAHostChild(RemoveFol, RemoveAll);
	} // if

} // PortableFoliageGUI::RemoveFoliage

/*===========================================================================*/

void PortableFoliageGUI::AddEcotype(void)
{
MaterialEffect *Mat;

if (! FixedEcotype)
	{
	if ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing()))
		{
		ValidateEcotypeSelection(Mat);
		FoliagePanel = EcotypePanel;
		ConfigureEcotype();
		BuildFoliageTree();
		ShowPanels();
		} // if
	} // if
	
} // PortableFoliageGUI::AddEcotype

/*===========================================================================*/

void PortableFoliageGUI::AddFoliageGroup(void)
{
FoliageGroup *NewGroup;
char NewName[WCS_EFFECT_MAXNAMELENGTH];
RasterAnimHostProperties Prop;
MaterialEffect *Mat;

if (FixedEcotype || ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing())))
	{
	if (! FixedEcotype)
		ValidateEcotypeSelection(Mat);
	if (ActiveFoliageValid())
		{
		NewName[0] = 0;
		if (GetInputString("Enter name for new Foliage Group", "", NewName) && NewName[0])
			{
			if (NewGroup = ActiveEcotype->AddFoliageGroup(NULL, NewName))
				{
				ActiveGroup = NewGroup;
				FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
				Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_EXPANDED1;
				Prop.Flags = WCS_RAHOST_FLAGBIT_EXPANDED1;
				ActiveEcotype->SetRAHostProperties(&Prop);
				ConfigureGroup();
				BuildFoliageTree();
				ShowPanels();
				} // if
			} // if
		} // if
	} // if
	
} // PortableFoliageGUI::AddFoliageGroup

/*===========================================================================*/

void PortableFoliageGUI::AddFoliage(void)
{
Foliage *NewFol;
RasterAnimHostProperties Prop;
MaterialEffect *Mat = NULL;

if (FixedEcotype || ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing())))
	{
	ValidateFoliageGroupSelection(Mat);
	if (ActiveFoliageValid() && ActiveGroup)
		{
		if (NewFol = ActiveGroup->AddFoliage(NULL))
			{
			ActiveFol = NewFol;
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_EXPANDED1;
			Prop.Flags = WCS_RAHOST_FLAGBIT_EXPANDED1;
			ActiveGroup->SetRAHostProperties(&Prop);
			ConfigureFoliage();
			BuildFoliageTree();
			ShowPanels();
			} // if
		} // if
	} // if
	
} // PortableFoliageGUI::AddFoliage

/*===========================================================================*/

void PortableFoliageGUI::OpenFoliagePreview(void)
{

if (ActiveFol->Img && ActiveFol->Img->GetRaster())
	{
	ActiveFol->Img->GetRaster()->EditRAHost();
	} // if

} // PortableFoliageGUI::OpenFoliagePreview()

/*===========================================================================*/

void PortableFoliageGUI::OpenObjectPreview(void)
{

if (ActiveFol->Obj)
	{
	ActiveFol->Obj->EditRAHost();
	} // if

} // PortableFoliageGUI::OpenObjectPreview()

/*===========================================================================*/

void PortableFoliageGUI::ApplySettingsToFoliageGroup(void)
{
Foliage *CurFol;

if (ActiveGroup && ActiveFol)
	{
	CurFol = ActiveGroup->Fol;
	while (CurFol)
		{
		if (CurFol != ActiveFol)
			{
			CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING].Copy(&CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING], &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING]);
			CurFol->FlipX = ActiveFol->FlipX;
			CurFol->Shade3D = ActiveFol->Shade3D;
			} // if
		CurFol = CurFol->Next;
		} // while
	} // if

} // PortableFoliageGUI::ApplySettingsToFoliageGroup

/*===========================================================================*/

void PortableFoliageGUI::CopyFoliageItem(RasterAnimHost *EditTarget)
{
int IsGroup = 0, IsEcotype = 0, IsFoliage = 0;
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost = NULL;
char CopyMsg[256];

IsEcotype = FoliagePanel < WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
IsGroup = FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
IsFoliage = FoliagePanel > WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
if (CopyProp = new RasterAnimHostProperties())
	{
	if (EditTarget)
		{
		CopyProp->PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		EditTarget->GetRAHostProperties(CopyProp);
		if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
			{
			IsEcotype = 1;
			CopyHost = EditTarget;
			} // if
		else if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP)
			{
			IsGroup = 1;
			CopyHost = EditTarget;
			} // else if
		else if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
			{
			IsFoliage = 1;
			CopyHost = EditTarget;
			} // else if
		} // if
	else
		{
		IsEcotype = FoliagePanel < WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		IsGroup = FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		IsFoliage = FoliagePanel > WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		} // else
	if (ActiveFoliageValid())
		{
		if ((IsEcotype && (CopyHost || (CopyHost = ActiveEcotype)))
			|| (IsGroup && (CopyHost || (CopyHost = ActiveGroup)))
			|| (IsFoliage && (CopyHost || (CopyHost = ActiveFol))))
			{
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			CopyHost->GetRAHostProperties(CopyProp);
			if (CopyProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
				{
				RasterAnimHost::SetCopyOfRAHost(CopyHost);
				sprintf(CopyMsg, "%s %s copied to clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK("Copy", "Selected item cannot be copied.");
				} // else
			} // if
		} // if
	delete CopyProp;
	} // if

} // PortableFoliageGUI::CopyFoliageItem

/*===========================================================================*/

void PortableFoliageGUI::PasteFoliageItem(RasterAnimHost *EditTarget)
{
int IsGroup = 0, IsEcotype = 0, IsFoliage = 0;
RasterAnimHostProperties *CopyProp;
RasterAnimHost *PasteHost = NULL, *CopyHost, *TempHost;
char CopyMsg[256], CopyIllegal = 0;

if (CopyProp = new RasterAnimHostProperties())
	{
	if (EditTarget)
		{
		CopyProp->PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		EditTarget->GetRAHostProperties(CopyProp);
		if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
			{
			IsEcotype = 1;
			PasteHost = EditTarget;
			} // if
		else if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP)
			{
			IsGroup = 1;
			PasteHost = EditTarget;
			} // else if
		else if (CopyProp->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
			{
			IsFoliage = 1;
			PasteHost = EditTarget;
			} // else if
		} // if
	else
		{
		IsEcotype = FoliagePanel < WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		IsGroup = FoliagePanel == WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		IsFoliage = FoliagePanel > WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
		} // else
	if (ActiveFoliageValid())
		{
		if ((IsEcotype && (PasteHost || (PasteHost = ActiveEcotype)))
			|| (IsGroup && (PasteHost || (PasteHost = ActiveGroup)))
			|| (IsFoliage && (PasteHost || (PasteHost = ActiveFol))))
			{
			if (CopyHost = RasterAnimHost::GetCopyOfRAHost())
				{
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
				CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
				CopyHost->GetRAHostProperties(CopyProp);
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
				PasteHost->GetRAHostProperties(CopyProp);
				if (CopyProp->DropOK)
					{
					CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					CopyProp->DropSource = CopyHost;
					// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
					//  eg. still in progress through a DragnDropListGUI
					// determine if copying is legal - can't copy from a texture to a child of itself or a parent of itself
					TempHost = PasteHost;
					while (TempHost)
						{
						if (CopyHost == TempHost)
							{
							CopyIllegal = 1;
							break;
							} // if
						TempHost = TempHost->RAParent;
						} // while
					TempHost = CopyHost;
					while (TempHost)
						{
						if (PasteHost == TempHost)
							{
							CopyIllegal = 1;
							break;
							} // if
						TempHost = TempHost->RAParent;
						} // while
					if (! CopyIllegal)
						{
						PasteHost->SetRAHostProperties(CopyProp);
						sprintf(CopyMsg, "%s %s pasted from clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
						} // if
					else
						{
						UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Can't copy from parent to child or child to its parent or copy texture to itself.");
						} // else
					} // if
				else
					{
					UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
					} // else
				} // if
			else
				{
				UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
				} // else
			} // if
		} // if
	delete CopyProp;
	} // if

} // PortableFoliageGUI::PasteFoliageItem

/*===========================================================================*/

void PortableFoliageGUI::ChangeFoliageSelection(unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData)
{
RasterAnimHost *RAThing;

if (NewTreeItem && NewTreeItem != OldTreeItem)
	{
	FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, (void *)OldTreeItem, false);
	if (LastSelected && LastSelected != (void *)OldTreeItem && LastSelected != (void *)NewTreeItem)
		FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, LastSelected, false);
	LastSelected = (void *)NewTreeItem;
	if (TreeItemData)
		{
		RAThing = (RasterAnimHost *)TreeItemData;
		RasterAnimHost::SetActiveRAHost(RAThing, 1);
		if (RAThing->GetNotifySubclass() == WCS_SUBCLASS_ECOTYPE)
			{
			ActiveEcotype = (Ecotype *)RAThing;
			if (ActiveFoliageValid() && ActiveEcotype)
				{
				FoliagePanel = EcotypePanel;
				ConfigureEcotype();
				ShowPanels();
				} // if
			} // if
		else if (RAThing->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGEGRP)
			{
			if (ActiveGroup = (FoliageGroup *)RAThing)
				{
				if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
					{
					if (ActiveFoliageValid() && ActiveGroup)
						{
						FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
						ConfigureEcotype();
						ShowPanels();
						} // if
					} // if
				} // if
			} // else if
		else if (RAThing->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGE)
			{
			if (ActiveFol = (Foliage *)RAThing)
				{
				if (ActiveGroup = (FoliageGroup *)ActiveFol->RAParent)
					{
					if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
						{
						if (ActiveFoliageValid() && ActiveFol)
							{
							ConfigureEcotype();
							if (ActiveFol && ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
								FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT;
							else
								FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
							ShowPanels();
							} // if
						} // if
					} // if
				} // if
			} // else if
		} // if
	else
		{
		if ((void *)NewTreeItem == EcotypeTreeItem[0])
			EcotypeSelection = 0;
		else
			EcotypeSelection = 1;
		ActiveEcotype = NULL;
		ActiveGroup = NULL;
		ActiveFol = NULL;
		FoliagePanel = EcotypePanel;
		ConfigureEcotype();
		ShowPanels();
		} // else
	} // if

} // PortableFoliageGUI::ChangeFoliageSelection

/*===========================================================================*/

void PortableFoliageGUI::ListItemExpanded(void *TreeItemData, char Expand)
{
RasterAnimHost *CurHost = (RasterAnimHost *)TreeItemData;
RasterAnimHostProperties Prop;

if (CurHost)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_EXPANDED1;
	Prop.Flags = Expand ? WCS_RAHOST_FLAGBIT_EXPANDED1: 0;
	CurHost->SetRAHostProperties(&Prop);
	} // for

} // PortableFoliageGUI::ListItemExpanded

/*===========================================================================*/

void PortableFoliageGUI::ValidateEcotypeSelection(MaterialEffect *Mat)
{
NotifyTag Changes[2];

if (! ActiveEcotype)
	{
	ActiveEcotype = Mat->NewEcotype(EcotypeSelection);
	Changes[0] = MAKE_ID(Mat->GetNotifyClass(), Mat->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Mat->GetRAHostRoot());
	FoliagePanel = EcotypePanel;
	ConfigureEcotype();
	BuildFoliageTree();
	ShowPanels();
	} // if

} // PortableFoliageGUI::ValidateEcotypeSelection

/*===========================================================================*/

void PortableFoliageGUI::ValidateFoliageGroupSelection(MaterialEffect *Mat)
{
FoliageGroup *NewGroup;
char NewName[WCS_EFFECT_MAXNAMELENGTH];
RasterAnimHostProperties Prop;

if (! FixedEcotype)
	ValidateEcotypeSelection(Mat);

if (ActiveEcotype && ! ActiveGroup)
	{
	NewName[0] = 0;
	if (GetInputString("Enter name for new Foliage Group", "", NewName) && NewName[0])
		{
		if (NewGroup = ActiveEcotype->AddFoliageGroup(NULL, NewName))
			{
			ActiveGroup = NewGroup;
			FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_EXPANDED1;
			Prop.Flags = WCS_RAHOST_FLAGBIT_EXPANDED1;
			ActiveEcotype->SetRAHostProperties(&Prop);
			ConfigureGroup();
			BuildFoliageTree();
			ShowPanels();
			} // if
		} // if
	} // if

} // PortableFoliageGUI::ValidateFoliageGroupSelection

/*===========================================================================*/

void PortableFoliageGUI::EnableFoliageItem(RasterAnimHost *EditTarget, unsigned long TreeItem, bool EnabledState)
{
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Changes[1] = 0;

if (EditTarget)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	EditTarget->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
		{
		((Ecotype *)EditTarget)->Enabled = EnabledState ? 1: 0;
		} // if
	else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP)
		{
		((FoliageGroup *)EditTarget)->Enabled = EnabledState ? 1: 0;
		} // else if
	else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
		{
		((Foliage *)EditTarget)->Enabled = EnabledState ? 1: 0;
		} // else if
	Changes[0] = MAKE_ID(EditTarget->GetNotifyClass(), EditTarget->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, EditTarget->GetRAHostRoot());
	} // if

} // PortableFoliageGUI::EnableFoliageItem

/*===========================================================================*/

void PortableFoliageGUI::EditFoliageItem(RasterAnimHost *EditTarget, unsigned long TreeItem)
{
void *MyTreeItem = (void *)TreeItem;
void *TestData, *EcotypeLevel, *GroupLevel, *FoliageLevel;

if (! MyTreeItem && EditTarget)
	{
	// find the tree item that corresponds
	for (EcotypeLevel = FenetreHost->WidgetTVGetRoot(IDC_FOLIAGETREE); EcotypeLevel && ! MyTreeItem; EcotypeLevel = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, EcotypeLevel))
		{
		if ((TestData = FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, EcotypeLevel)) && TestData == EditTarget)
			{
			MyTreeItem = EcotypeLevel;
			break;
			} // if
		for (GroupLevel = FenetreHost->WidgetTVGetChild(IDC_FOLIAGETREE, EcotypeLevel); GroupLevel && ! MyTreeItem; GroupLevel = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, GroupLevel))
			{
			if ((TestData = FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, GroupLevel)) && TestData == EditTarget)
				{
				MyTreeItem = GroupLevel;
				break;
				} // if
			for (FoliageLevel = FenetreHost->WidgetTVGetChild(IDC_FOLIAGETREE, GroupLevel); FoliageLevel; FoliageLevel = FenetreHost->WidgetTVGetNextSibling(IDC_FOLIAGETREE, FoliageLevel))
				{
				if ((TestData = FenetreHost->WidgetTVGetItemData(IDC_FOLIAGETREE, FoliageLevel)) && TestData == EditTarget)
					{
					MyTreeItem = FoliageLevel;
					break;
					} // if
				} // for
			} // for
		} // for 
	} // if
if (MyTreeItem)
	{
	if (LastSelected && LastSelected != MyTreeItem)
		FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, LastSelected, false);
	FenetreHost->WidgetTVSetItemSelected(IDC_FOLIAGETREE, MyTreeItem, true);
	FenetreHost->WidgetTVEnsureVisible(IDC_FOLIAGETREE, MyTreeItem);
	LastSelected = MyTreeItem;
	if (EditTarget)
		{
		RasterAnimHost::SetActiveRAHost(EditTarget, 1);
		if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_ECOTYPE)
			{
			ActiveEcotype = (Ecotype *)EditTarget;
			if (ActiveFoliageValid() && ActiveEcotype)
				{
				FoliagePanel = EcotypePanel;
				ConfigureEcotype();
				ShowPanels();
				} // if
			} // if
		else if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGEGRP)
			{
			if (ActiveGroup = (FoliageGroup *)EditTarget)
				{
				if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
					{
					if (ActiveFoliageValid() && ActiveGroup)
						{
						FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGEGROUP;
						ConfigureEcotype();
						ShowPanels();
						} // if
					} // if
				} // if
			} // else if
		else if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGE)
			{
			if (ActiveFol = (Foliage *)EditTarget)
				{
				if (ActiveGroup = (FoliageGroup *)ActiveFol->RAParent)
					{
					if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
						{
						if (ActiveFoliageValid() && ActiveFol)
							{
							ConfigureEcotype();
							if (ActiveFol && ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
								FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_OBJECT;
							else
								FoliagePanel = WCS_FOLIAGE_PANEL_FOLIAGE_IMAGE;
							ShowPanels();
							} // if
						} // if
					} // if
				} // if
			} // else if
		} // if
	else
		{
		if (MyTreeItem == (void *)EcotypeTreeItem[0])
			EcotypeSelection = 0;
		else
			EcotypeSelection = 1;
		ActiveEcotype = NULL;
		ActiveGroup = NULL;
		ActiveFol = NULL;
		FoliagePanel = EcotypePanel;
		ConfigureEcotype();
		ShowPanels();
		} // else
	} // if

} // PortableFoliageGUI::EditFoliageItem

/*===========================================================================*/

RasterAnimHost *PortableFoliageGUI::AddFoliageItem(long Item, int OpenGallery, RasterAnimHost *EditTarget, char *ActionText)
{
bool MakeEcotype, MakeGroup, MakeFoliage, LoadFromGallery;
RasterAnimHost *Added = NULL;
RasterAnimHostProperties Prop;
MaterialEffect *Mat = NULL;

MakeEcotype = MakeGroup = MakeFoliage = LoadFromGallery = 0;

if (FixedEcotype || ((ActiveGrad = MatGrad->GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing())))
	{
	// overstory
	if ((! stricmp(ActionText, "ADDOVERSTORY"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUP"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGE"))
		|| (! stricmp(ActionText, "ADDOVERGALLERY"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUPGALLERY")))
		{
		EcotypeSelection = 0;
		if (! FixedEcotype)
			ActiveEcotype = NULL;
		ActiveGroup = NULL;
		ActiveFol = NULL;
		} // if
	// understory
	else if ((! stricmp(ActionText, "ADDUNDERSTORY"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUP"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGE"))
		|| (! stricmp(ActionText, "ADDUNDERGALLERY"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUPGALLERY")))
		{
		EcotypeSelection = 1;
		if (! FixedEcotype)
			ActiveEcotype = NULL;
		ActiveGroup = NULL;
		ActiveFol = NULL;
		} // if
	// indeterminate
	else if (EditTarget)
		{
		// find the object type and walk up the tree identifying its parents
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		EditTarget->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
			{
			ActiveEcotype = (Ecotype *)EditTarget;
			} // if
		else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP)
			{
			ActiveGroup = (FoliageGroup *)EditTarget;
			ActiveEcotype = (Ecotype *)ActiveGroup->RAParent;
			} // if
		else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
			{
			ActiveFol = (Foliage *)EditTarget;
			ActiveGroup = (FoliageGroup *)ActiveFol->RAParent;
			ActiveEcotype = (Ecotype *)ActiveGroup->RAParent;
			} // if
		if (FixedEcotype || (ActiveEcotype == Mat->EcoFol[0]))
			EcotypeSelection = 0;
		else
			EcotypeSelection = 1;
		} // else if
		
	// ecotype
	if ((! stricmp(ActionText, "ADDOVERSTORY"))
		|| (! stricmp(ActionText, "ADDUNDERSTORY"))
		|| (! stricmp(ActionText, "ADDOVERGALLERY"))
		|| (! stricmp(ActionText, "ADDUNDERGALLERY")))
		MakeEcotype = true;
	//foliage group
	else if ((! stricmp(ActionText, "ADDFOLIAGEGROUP"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUP"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUP"))
		|| (!stricmp(ActionText, "ADDFOLIAGEGROUPGALLERY"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUPGALLERY"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUPGALLERY")))
		MakeGroup = true;
	//foliage
	else if ((! stricmp(ActionText, "ADDFOLIAGE"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGE"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGE")))
		MakeFoliage = true;

	//gallery
	if ((!stricmp(ActionText, "ADDFOLIAGEGROUPGALLERY"))
		|| (! stricmp(ActionText, "ADDOVERGALLERY"))
		|| (! stricmp(ActionText, "ADDUNDERGALLERY"))
		|| (! stricmp(ActionText, "ADDOVERFOLIAGEGROUPGALLERY"))
		|| (! stricmp(ActionText, "ADDUNDERFOLIAGEGROUPGALLERY")))
		LoadFromGallery = 1;

	if (MakeEcotype && ! FixedEcotype)
		{
		ActiveEcotype = NULL;
		ValidateEcotypeSelection(Mat);
		Added = ActiveEcotype;
		} // if
	else if (MakeGroup)
		{
		ActiveGroup = NULL;
		ValidateFoliageGroupSelection(Mat);
		Added = ActiveGroup;
		} // else if
	else if (MakeFoliage)
		{
		ActiveFol = NULL;
		ValidateFoliageGroupSelection(Mat);
		AddFoliage();
		Added = ActiveFol;
		} // else if

	if (Added && LoadFromGallery)
		Added->OpenGallery(EffectsHost);
	} // if
		
return (Added);

} // PortableFoliageGUI::AddFoliageItem

/*===========================================================================*/

void PortableFoliageGUI::RemoveFoliageItem(RasterAnimHost *EditTarget)
{

if (EditTarget)
	{
	if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_ECOTYPE)
		{
		ActiveEcotype = (Ecotype *)EditTarget;
		RemoveEcotype();
		} // if
	else if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGEGRP)
		{
		if (ActiveGroup = (FoliageGroup *)EditTarget)
			{
			if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
				{
				RemoveFoliageGroup();
				} // if
			} // if
		} // else if
	else if (EditTarget->GetNotifySubclass() == WCS_SUBCLASS_FOLIAGE)
		{
		if (ActiveFol = (Foliage *)EditTarget)
			{
			if (ActiveGroup = (FoliageGroup *)ActiveFol->RAParent)
				{
				if (ActiveEcotype = (Ecotype *)ActiveGroup->RAParent)
					{
					RemoveFoliage();
					} // if
				} // if
			} // if
		} // else if
	} // if

} // PortableFoliageGUI::RemoveFoliageItem

/*===========================================================================*/


