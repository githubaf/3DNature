// EcotypeEditGUI.cpp
// Code for Ecotype editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EcotypeEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "resource.h"

#ifdef WCS_BUILD_DEMO
extern int ForestryDemoWarned;
#endif // WCS_BUILD_DEMO

char *EcotypeEditGUI::TabNames[WCS_ECOTYPEGUI_NUMTABS] = {"General", "Ecotype", "Groups", "Objects"};
long EcotypeEditGUI::ActivePage;

/*===========================================================================*/

bool EcotypeEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_IS_A_COMPONENT_EDITOR:
	case WCS_FENETRE_WINCAP_CANUNDO:
	case WCS_FENETRE_WINCAP_CANLOAD:
	case WCS_FENETRE_WINCAP_CANSAVE:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // EcotypeEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin EcotypeEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // EcotypeEditGUI::Open

/*===========================================================================*/

NativeGUIWin EcotypeEditGUI::Construct(void)
{
Raster *MyRast;
GeneralEffect *MyEffect;
char  *Units[] = {"Hectare", "Acre", "Square Meter", "Square Foot"};
char  *BAUnits[] = {"Square Meters", "Square Feet"};
char  *BAUnits2[] = {"Sq Meters", "Sq Feet"};
int TabIndex;
#ifdef WCS_FORESTRY_WIZARD
int InhibitForestry = 0;
#endif // WCS_FORESTRY_WIZARD

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_FORESTRY_WIZARD
	if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("inhibit_forestry"))
		{
		InhibitForestry = 1;
		GlobalApp->ForestryAuthorized = 0;
		} // if
	if (GlobalApp->ForestryAuthorized || (GlobalApp->ForestryAuthorized = ((GlobalApp->Sentinal->CheckAuthFieldForestry() 
		|| GlobalApp->Sentinal->CheckRenderEngineQuick()) && ! InhibitForestry) ? 1: 0))
		{
		CreateSubWinFromTemplate(IDD_ECOTYPE_GENERAL_FORESTRY, 0, 0);
		CreateSubWinFromTemplate(IDD_ECOTYPE_BASIC_FORESTRY, 0, 1);
		CreateSubWinFromTemplate(IDD_ECOTYPE_FOLIAGEGROUP_FORESTRY, 0, 2);
		#ifdef WCS_BUILD_DEMO
		if (! ForestryDemoWarned)
			UserMessageOK("Forestry Edition", "The Forestry Edition is an optional upgrade to the basic VNS program. It enables you to input foliage size and density in a suite of typical forestry style variables that are not available in the basic VNS version. Those extra features are enabled in this Demo version so you can see how they work. If you want these extra features be sure to specify that you want the Forestry Edition when you purchase VNS. It also includes the Forestry Wizard for expediting the setup of forestry projects.");
		ForestryDemoWarned = 1;
		#endif // WCS_BUILD_DEMO
		} // if
	else
	#endif // WCS_FORESTRY_WIZARD
		{
		CreateSubWinFromTemplate(IDD_ECOTYPE_GENERAL, 0, 0);
		CreateSubWinFromTemplate(IDD_ECOTYPE_BASIC, 0, 1);
		CreateSubWinFromTemplate(IDD_ECOTYPE_FOLIAGEGROUP, 0, 2);
		} // else
	CreateSubWinFromTemplate(IDD_ECOTYPE_OBJECTLIST, 0, 3);
	CreateSubWinFromTemplate(IDD_ECOTYPE_FOLIAGE, 1, 0);
	CreateSubWinFromTemplate(IDD_ECOTYPE_3DOBJECT, 1, 1);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_ECOTYPEGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for (TabIndex = 0; TabIndex < 4; TabIndex ++)
			{
			WidgetCBInsert(IDC_UNITSDROP, -1, Units[TabIndex]);
			} // for
		for (TabIndex = 0; TabIndex < 2; TabIndex ++)
			{
			WidgetCBInsert(IDC_BAUNITSDROP, -1, BAUnits[TabIndex]);
			WidgetCBInsert(IDC_BAUNITSDROP2, -1, BAUnits2[TabIndex]);
			} // for
		WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
		for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
			{
			TabIndex = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
			WidgetCBSetItemData(IDC_IMAGEDROP, TabIndex, MyRast);
			} // for
		WidgetCBInsert(IDC_OBJECTDROP, -1, "New 3D Object...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_OBJECTDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_OBJECTDROP, TabIndex, MyEffect);
			} // for
		SelectPanel(ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // EcotypeEditGUI::Construct

/*===========================================================================*/

EcotypeEditGUI::EcotypeEditGUI(ImageLib *ImageSource, EffectsLib *EffectsSource, Ecotype *ActiveSource)
: GUIFenetre('ECTP', this, "Ecotype Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {0,
								0,
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
ImageHost = ImageSource;
EffectsHost = EffectsSource;
Active = ActiveSource;
ActiveGroup = NULL;
ActiveFol = NULL;

if (ImageSource && EffectsHost && ActiveSource)
	{
	OldType = Active->SecondHeightType;
	AllEvents[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, 0xff);
	AllEvents[1] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	sprintf(NameStr, "Ecotype Editor - %s ", Active->GetRAHostName());
	if (Active->RAParent)
		strcat(NameStr, Active->RAParent->GetCritterName(Active));
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	if (ActiveGroup = Active->FolGrp)
		{
		ActiveFol = ActiveGroup->Fol;
		} // if
	else
		ActiveFol = NULL;
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // EcotypeEditGUI::EcotypeEditGUI

/*===========================================================================*/

EcotypeEditGUI::~EcotypeEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // EcotypeEditGUI::~EcotypeEditGUI()

/*===========================================================================*/

long EcotypeEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_FEG, 0);

return(0);

} // EcotypeEditGUI::HandleCloseWin

/*===========================================================================*/

long EcotypeEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_FEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_GALLERY:
		{
		Active->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_LOAD:
		{
		Active->LoadComponentFile(NULL);
		break;
		} //
	case IDC_SAVE:
		{
		Active->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDGROUP:
		{
		AddGroup();
		break;
		} // IDC_ADDECO
	case IDC_REMOVEFOLIAGEGROUP:
		{
		RemoveGroup();
		break;
		} // IDC_REMOVEECO
	case IDC_MOVEGROUPUP:
		{
		ChangeGroupListPosition(1);
		break;
		} // IDC_MOVEECOUP
	case IDC_MOVEGROUPDOWN:
		{
		ChangeGroupListPosition(0);
		break;
		} // IDC_MOVEECODOWN
	case IDC_LOADCOMPONENT:
		{
		if (ActiveGroup)
			ActiveGroup->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveGroup)
			ActiveGroup->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDFOLIAGE:
		{
		AddFoliage();
		break;
		} // IDC_ADDECO
	case IDC_REMOVEFOLIAGE:
		{
		RemoveFoliage();
		break;
		} // IDC_REMOVEECO
	case IDC_MOVEFOLIAGEUP:
		{
		ChangeFoliageListPosition(1);
		break;
		} // IDC_MOVEECOUP
	case IDC_MOVEFOLIAGEDOWN:
		{
		ChangeFoliageListPosition(0);
		break;
		} // IDC_MOVEECODOWN
	case IDC_APPLYTOGROUP:
		{
		ApplyBackLightToGroup();
		break;
		} // IDC_APPLYTOGROUP
	case IDC_EDITHTGRAPH:
		{
		if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			Active->DBHCurve.EditRAHost();
		else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			Active->AgeCurve.EditRAHost();
		break;
		} // IDC_EDITHTGRAPH
	case IDC_EDITHTGRAPH2:
		{
		if (ActiveGroup)
			{
			if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
				ActiveGroup->DBHCurve.EditRAHost();
			else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
				ActiveGroup->AgeCurve.EditRAHost();
			} // if
		break;
		} // IDC_EDITHTGRAPH2
	default:
		break;
	} // ButtonID

return(0);

} // EcotypeEditGUI::HandleButtonClick

/*===========================================================================*/

long EcotypeEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL:
		{
		if (ActiveFol)
			OpenPreview();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // EcotypeEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long EcotypeEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_IMAGEDROP:
		{
		SelectNewImage(-1);
		break;
		}
	case IDC_OBJECTDROP:
		{
		SelectNewObject(-1);
		break;
		}
	case IDC_UNITSDROP:
		{
		SetDensityUnits();
		break;
		}
	case IDC_BAUNITSDROP:
	case IDC_BAUNITSDROP2:
		{
		SetBAUnits((unsigned short)CtrlID);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // EcotypeEditGUI::HandleCBChange

/*===========================================================================*/

long EcotypeEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(016, 16);
switch (CtrlID)
	{
	case IDC_GROUPLIST:
		{
		SetActiveGroup();
		break;
		} // IDC_GROUPLIST
	case IDC_FOLIAGELIST:
		{
		SetActiveFoliage();
		break;
		} // IDC_FOLIAGELIST
	default:
		break;
	} // switch CtrlID

return (0);

} // EcotypeEditGUI::HandleListSel

/*===========================================================================*/

long EcotypeEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_GROUPLIST:
		{
		RemoveGroup();
		break;
		} // IDC_GROUPLIST
	case IDC_FOLIAGELIST:
		{
		RemoveFoliage();
		break;
		} // IDC_FOLIAGELIST
	default:
		break;
	} // switch

return(0);

} // EcotypeEditGUI::HandleListDelItem

/*===========================================================================*/

long EcotypeEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GROUPLIST:
		{
		ActivePage = 2;
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		SelectPanel(ActivePage);
		break;
		} // IDC_FOLIAGELIST
	case IDC_FOLIAGELIST:
		{
		EditFoliageObject();
		break;
		} // IDC_FOLIAGELIST
	default:
		break;
	} // switch CtrlID

return (0);

} // EcotypeEditGUI::HandleListDoubleClick

/*===========================================================================*/

long EcotypeEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_GROUPLIST:
		{
		CopyMaterial(1);	// foliage group
		break;
		} // IDC_ECOMATLIST1
	case IDC_FOLIAGELIST:
		{
		CopyMaterial(0);	// foliage
		break;
		} // IDC_ECOMATLIST1
	default:
		break;
	} // switch

return(0);

} // EcotypeEditGUI::HandleListCopyItem

/*===========================================================================*/

long EcotypeEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_GROUPLIST:
		{
		PasteMaterial(1);	// foliage group
		break;
		} // IDC_ECOMATLIST1
	case IDC_FOLIAGELIST:
		{
		PasteMaterial(0);	// foliage
		break;
		} // IDC_ECOMATLIST1
	default:
		break;
	} // switch

return(0);

} // EcotypeEditGUI::HandleListPasteItem

/*===========================================================================*/

long EcotypeEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GROUPNAME:
		{
		GroupName();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // EcotypeEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long EcotypeEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				SelectPanel(1);
				break;
				} // 0
			case 2:
				{
				SelectPanel(2);
				break;
				} // 0
			case 3:
				{
				SelectPanel(3);
				break;
				} // 0
			default:
				{
				SelectPanel(0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // EcotypeEditGUI::HandlePageChange

/*===========================================================================*/

long EcotypeEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_CHECKENABLED
	case IDC_CHECKDISSOLVEENABLED:
	case IDC_CHECKRENDEROCCLUDED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
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
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKROTATEX
	default:
		break;
	} // switch CtrlID

return(0);

} // EcotypeEditGUI::HandleSCChange

/*===========================================================================*/

long EcotypeEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOABSHTECOTYPE:
	case IDC_RADIOABSHTFOLGRP:
		{
		if (GlobalApp->ForestryAuthorized && Active->AbsHeightResident != Active->AbsDensResident)
			{
			if ((Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
				|| (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA))
				{
				Active->AbsDensResident = Active->AbsHeightResident;
				Active->ChangeAbsDensResident();
				} // if
			} // if
		Active->ChangeAbsHtResident();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOABSDENSECOTYPE:
	case IDC_RADIOABSDENSFOLGRP:
		{
		if (GlobalApp->ForestryAuthorized && Active->AbsHeightResident != Active->AbsDensResident)
			{
			if ((Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
				|| (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA))
				{
				Active->AbsHeightResident = Active->AbsDensResident;
				Active->ChangeAbsHtResident();
				} // if
			} // if
		Active->ChangeAbsDensResident();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOSCNDHTABS:
	case IDC_RADIOSCNDHTRELMIN:
	case IDC_RADIOSCNDHTRELRNG:
		{
		Active->ChangeSecondHtType(OldType);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		OldType = Active->SecondHeightType;
		break;
		} // 
	case IDC_RADIOIMAGEOBJ:
	case IDC_RADIO3DOBJ:
	case IDC_RADIODENSAREA:
	case IDC_RADIODENSSTEMS:
	case IDC_RADIODENSBASALAREA:
	case IDC_RADIODENSCLOSURE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOSIZEHEIGHT:
	case IDC_RADIOSIZEDBH:
	case IDC_RADIOSIZEAGE:
	case IDC_RADIOSIZECLOSURE:
		{
		Active->ChangeSizeMethod();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIODENSPOLYGON:
		{
		Active->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOBYPIXEL:
	case IDC_RADIOBYIMAGEHT:
		{
		if (! UserMessageOKCAN("Common Distance Dissolve", "Warning! This change will affect how all Foliage Effects and all Ecotype foliage is rendered."))
			{
			GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize = 1 - GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize;
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // EcotypeEditGUI::HandleSRChange

/*===========================================================================*/

long EcotypeEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_ROTATEX:
	case IDC_ROTATEY:
	case IDC_ROTATEZ:
		{
		// these float ints are configured for plain ol' doubles
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_DISSOLVEREFHT:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // EcotypeEditGUI::HandleFIChange

/*===========================================================================*/

void EcotypeEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Pos, CurPos, Done = 0;
Raster *MyRast, *MatchRast;
GeneralEffect *MyEffect, *MatchEffect;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[5] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[6] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = ImageHost->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchRast = (ActiveFol && ActiveFol->Img) ? ActiveFol->Img->GetRaster(): NULL;
	WidgetCBClear(IDC_IMAGEDROP);
	WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
	for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
		{
		Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
		WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
		if (MyRast == MatchRast)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_IMAGEDROP, CurPos);
	BuildFoliageList();
	Done = 1;
	} // if image name changed
else
	{
	Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (Changed = GlobalApp->AppImages->MatchNotifyClass(Interested, Changes, 0))
		{
		MatchRast = (ActiveFol && ActiveFol->Img) ? ActiveFol->Img->GetRaster(): NULL;
		DisableWidgets();
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MatchRast);
		} // else
	} // else

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = ActiveFol ? ActiveFol->Obj: NULL;
	WidgetCBClear(IDC_OBJECTDROP);
	WidgetCBInsert(IDC_OBJECTDROP, -1, "New 3D Object...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_OBJECTDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_OBJECTDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_OBJECTDROP, CurPos);
	BuildFoliageList();
	Done = 1;
	} // if image name changed

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // EcotypeEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void EcotypeEditGUI::ConfigureWidgets(void)
{
int DBHDisplay;
char TextStr[256];

sprintf(TextStr, "Ecotype Editor - %s ", Active->GetRAHostName());
if (Active->RAParent)
	strcat(TextStr, Active->RAParent->GetCritterName(Active));
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);

ConfigureSR(NativeWin, IDC_RADIOBYPIXEL, IDC_RADIOBYPIXEL, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOBYPIXEL, IDC_RADIOBYIMAGEHT, &GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize, SRFlag_Short, 1, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTECOTYPE, &Active->AbsHeightResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_ECOTYPE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSHTECOTYPE, IDC_RADIOABSHTFOLGRP, &Active->AbsHeightResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_FOLGROUP, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSECOTYPE, &Active->AbsDensResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_ECOTYPE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSDENSECOTYPE, IDC_RADIOABSDENSFOLGRP, &Active->AbsDensResident, SRFlag_Char, WCS_ECOTYPE_ABSRESIDENT_FOLGROUP, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTABS, &Active->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_MINABS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELMIN, &Active->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_MINPCT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSCNDHTABS, IDC_RADIOSCNDHTRELRNG, &Active->SecondHeightType, SRFlag_Char, WCS_ECOTYPE_SECONDHT_RANGEPCT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIODENSPOLYGON, IDC_RADIODENSPOLYGON, &Active->ConstDensity, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIODENSPOLYGON, IDC_RADIODENSAREA, &Active->ConstDensity, SRFlag_Char, 1, NULL, NULL);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDISSOLVEENABLED, &Active->DissolveEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRENDEROCCLUDED, &Active->RenderOccluded, SCFlag_Char, NULL, 0);
WidgetCBSetCurSel(IDC_UNITSDROP, Active->DensityUnits);
WidgetCBSetCurSel(IDC_BAUNITSDROP, Active->BasalAreaUnits);
WidgetCBSetCurSel(IDC_BAUNITSDROP2, Active->BasalAreaUnits);

WidgetSmartRAHConfig(IDC_DISSOLVECOLOR, &Active->DissolveColor, Active);

WidgetSmartRAHConfig(IDC_ECOTYPEMINHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT], Active);

if (GlobalApp->ForestryAuthorized)
	{
	ConfigureSR(NativeWin, IDC_RADIODENSSTEMS, IDC_RADIODENSSTEMS, &Active->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIODENSSTEMS, IDC_RADIODENSBASALAREA, &Active->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_BASALAREA, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIODENSSTEMS, IDC_RADIODENSCLOSURE, &Active->DensityMethod, SRFlag_Char, WCS_FOLIAGE_DENSITYMETHOD_CLOSURE, NULL, NULL);

	ConfigureSR(NativeWin, IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEHEIGHT, &Active->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_HEIGHT, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEDBH, &Active->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_DBH, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZEAGE, &Active->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_AGE, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOSIZEHEIGHT, IDC_RADIOSIZECLOSURE, &Active->SizeMethod, SRFlag_Char, WCS_FOLIAGE_SIZEMETHOD_CLOSURE, NULL, NULL);

	DBHDisplay = Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH || Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;

	if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
		{
		WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT], Active);
		} // if
	else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_DBH], Active);
		DBHDisplay = 0;
		} // if
	else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
		{
		WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], Active);
		} // if
	else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_AGE], Active);
		} // if

	if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
		{
		WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY], Active);
		} // if
	else if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
		{
		WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_BASALAREA], Active);
		} // if
	else if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE], Active);
		} // if

	if (DBHDisplay)
		{
		WidgetShow(IDC_DENSITY2, Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		WidgetSetDisabled(IDC_DENSITY2, Active->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
		WidgetSmartRAHConfig(IDC_DENSITY2, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_DBH], Active);
		} // if
	else
		{
		WidgetSetText(IDC_DENSITY2, "");
		WidgetSmartRAHConfig(IDC_DENSITY2, (RasterAnimHost *)NULL, NULL);
		WidgetSetDisabled(IDC_DENSITY2, TRUE);
		WidgetShow(IDC_DENSITY2, FALSE);
		} // else
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_ECOTYPEMAXHEIGHT, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT], Active);
	WidgetSmartRAHConfig(IDC_DENSITY, &Active->AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY], Active);
	} // else

ConfigureFI(NativeWin, IDC_DISSOLVEHEIGHT,
 &Active->DissolvePixelHeight,
  .5,
   0.0,
	1000.0,
	 FIOFlag_Double,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_DISSOLVEREFHT,
 &GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt,
  1.0,
   1.0,
	32767.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureTB(NativeWin, IDC_ADDGROUP, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEFOLIAGEGROUP, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDFOLIAGE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEFOLIAGE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEGROUPUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEGROUPDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_MOVEFOLIAGEUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEFOLIAGEDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);

ConfigureGroup();
DisableWidgets();

} // EcotypeEditGUI::ConfigureWidgets()

/*===========================================================================*/

FoliageGroup *EcotypeEditGUI::ActiveFoliageValid(void)
{
FoliageGroup *CurGrp;
Foliage *CurFol;

if (ActiveGroup)
	{
	// check to see if active foliage group is valid
	CurGrp = Active->FolGrp;
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
	ActiveFol = NULL;
	ActiveGroup = Active->FolGrp;
	} // if
if (! ActiveFol && ActiveGroup)
	ActiveFol = ActiveGroup->Fol;
	
return (ActiveGroup);

} // EcotypeEditGUI::ActiveFoliageValid

/*===========================================================================*/

void EcotypeEditGUI::ConfigureGroup(void)
{
int DBHDisplay;

if (ActiveGroup = BuildGroupList())
	{
	WidgetSetDisabled(IDC_GROUPNAME, 0);
	WidgetSetModified(IDC_GROUPNAME, FALSE);
	WidgetSetText(IDC_GROUPNAME, ActiveGroup->Name);

	ConfigureSC(NativeWin, IDC_CHECKGROUPENABLED, &ActiveGroup->Enabled, SCFlag_Char, NULL, 0);

	WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT], ActiveGroup);

	if (GlobalApp->ForestryAuthorized)
		{
		DBHDisplay = Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH || Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA;

		if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT || Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
			{
			WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT], ActiveGroup);
			} // if
		else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH], ActiveGroup);
			DBHDisplay = 0;
			} // if
		else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
			} // if
		else if (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE], ActiveGroup);
			} // if

		if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA || Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
			{
			WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY], ActiveGroup);
			} // if
		else if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_BASALAREA], ActiveGroup);
			} // if
		else if (Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE], ActiveGroup);
			} // if

		if (DBHDisplay)
			{
			WidgetShow(IDC_GROUPDENSITY2, Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			WidgetSetDisabled(IDC_GROUPDENSITY2, Active->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
			WidgetSmartRAHConfig(IDC_GROUPDENSITY2, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH], ActiveGroup);
			} // if
		else
			{
			WidgetSetText(IDC_GROUPDENSITY2, "");
			WidgetSmartRAHConfig(IDC_GROUPDENSITY2, (RasterAnimHost *)NULL, NULL);
			WidgetSetDisabled(IDC_GROUPDENSITY2, TRUE);
			WidgetShow(IDC_GROUPDENSITY2, FALSE);
			} // else
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_GROUPHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT], ActiveGroup);
		WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT], ActiveGroup);
		WidgetSmartRAHConfig(IDC_GROUPDENSITY, &ActiveGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY], ActiveGroup);
		WidgetSmartRAHConfig(IDC_GROUPDENSITY2, (RasterAnimHost *)NULL, NULL);
		WidgetSetDisabled(IDC_GROUPDENSITY2, TRUE);
		WidgetShow(IDC_GROUPDENSITY2, FALSE);
		} // else
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_GROUPHEIGHT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_GROUPMINHEIGHT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_GROUPDENSITY, (RasterAnimHost *)NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGROUPENABLED, NULL, 0, NULL, 0);
	WidgetSetText(IDC_GROUPNAME, "");
	WidgetSetDisabled(IDC_GROUPNAME, 1);
	} // else

ConfigureFoliage();

} // EcotypeEditGUI::ConfigureGroup

/*===========================================================================*/

void EcotypeEditGUI::ConfigureFoliage(void)
{
char TextStr[64];
Raster *MyRast, *TestRast;
Object3DEffect *TestObj;
long ListPos, Ct, NumEntries;

if (ActiveFol = BuildFoliageList())
	{
	ConfigureSR(NativeWin, IDC_RADIOIMAGEOBJ, IDC_RADIOIMAGEOBJ, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_RASTER, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOIMAGEOBJ, IDC_RADIO3DOBJ, &ActiveFol->FoliageType, SRFlag_Char, WCS_FOLIAGE_TYPE_OBJECT3D, NULL, NULL);

	ConfigureSC(NativeWin, IDC_CHECKOBJENABLED, &ActiveFol->Enabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_3DSHADE, &ActiveFol->Shade3D, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEX, &ActiveFol->RandomRotate[0], SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEY, &ActiveFol->RandomRotate[1], SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEZ, &ActiveFol->RandomRotate[2], SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_FLIPX, &ActiveFol->FlipX, SCFlag_Char, NULL, 0);

	//WidgetSNConfig(IDC_FOLIAGEHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT]);
	//WidgetSNConfig(IDC_FOLIAGEDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY]);
	//WidgetSNConfig(IDC_ORIENTATIONSHADING, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING]);
	WidgetSmartRAHConfig(IDC_FOLIAGEHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT], ActiveFol);
	WidgetSmartRAHConfig(IDC_FOLIAGEDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY], ActiveFol);
	WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING], ActiveFol);
	WidgetSmartRAHConfig(IDC_REPLACECOLOR, &ActiveFol->Color, ActiveFol);

	//WidgetSNConfig(IDC_OBJECTHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT]);
	//WidgetSNConfig(IDC_OBJECTDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY]);
	WidgetSmartRAHConfig(IDC_OBJECTHEIGHT, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT], ActiveFol);
	WidgetSmartRAHConfig(IDC_OBJECTDENSITY, &ActiveFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY], ActiveFol);

	ConfigureFI(NativeWin, IDC_ROTATEX,
	 &ActiveFol->Rotate[0],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	ConfigureFI(NativeWin, IDC_ROTATEY,
	 &ActiveFol->Rotate[1],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	ConfigureFI(NativeWin, IDC_ROTATEZ,
	 &ActiveFol->Rotate[2],
	  1.0,
	   -180.0,
		180.0,
		 FIOFlag_Double,
		  NULL,
		   0);

	WidgetSetDisabled(IDC_OBJECTDROP, 0);
	WidgetSetDisabled(IDC_IMAGEDROP, 0);
	WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 0);
	WidgetSetDisabled(IDC_RADIO3DOBJ, 0);
	WidgetSetDisabled(IDC_APPLYTOGROUP, 0);

	if (ActiveFol->Img && (MyRast = ActiveFol->Img->GetRaster()))
		{
		sprintf(TextStr, "%d", MyRast->GetWidth());
		WidgetSetText(IDC_IMAGEWIDTH, TextStr);
		sprintf(TextStr, "%d", MyRast->GetHeight());
		WidgetSetText(IDC_IMAGEHEIGHT, TextStr);
		sprintf(TextStr, "%d", (MyRast->GetIsColor() ? 3: 1));
		if (MyRast->GetAlphaStatus())
			strcat(TextStr, " + Alpha");
		WidgetSetText(IDC_IMAGECOLORS, TextStr);
		ListPos = -1;
		NumEntries = WidgetCBGetCount(IDC_IMAGEDROP);
		for (Ct = 0; Ct < NumEntries; Ct ++)
			{
			if ((TestRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Ct)) != (Raster *)LB_ERR && TestRast == MyRast)
				{
				ListPos = Ct;
				break;
				} // for
			} // for
		WidgetCBSetCurSel(IDC_IMAGEDROP, ListPos);
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MyRast);
		} // if
	else
		{
		WidgetSetText(IDC_IMAGEWIDTH, "");
		WidgetSetText(IDC_IMAGEHEIGHT, "");
		WidgetSetText(IDC_IMAGECOLORS, "");
		WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
		} // else

	if (ActiveFol->Obj)
		{
		sprintf(TextStr, "%d", ActiveFol->Obj->NumPolys);
		WidgetSetText(IDC_POLYGONS, TextStr);
		sprintf(TextStr, "%d", ActiveFol->Obj->NumVertices);
		WidgetSetText(IDC_VERTICES, TextStr);
		sprintf(TextStr, "%d", ActiveFol->Obj->NumMaterials);
		WidgetSetText(IDC_MATERIALS, TextStr);
		ListPos = -1;
		NumEntries = WidgetCBGetCount(IDC_OBJECTDROP);
		for (Ct = 0; Ct < NumEntries; Ct ++)
			{
			if ((TestObj = (Object3DEffect *)WidgetCBGetItemData(IDC_OBJECTDROP, Ct)) != (Object3DEffect *)LB_ERR && TestObj == ActiveFol->Obj)
				{
				ListPos = Ct;
				break;
				} // for
			} // for
		WidgetCBSetCurSel(IDC_OBJECTDROP, ListPos);
		} // if
	else
		{
		WidgetSetText(IDC_POLYGONS, "");
		WidgetSetText(IDC_VERTICES, "");
		WidgetSetText(IDC_MATERIALS, "");
		WidgetCBSetCurSel(IDC_OBJECTDROP, -1);
		} // else
	if (ActivePage == 3)
		{
		if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
			ShowPanel(1, 0);
		else
			ShowPanel(1, 1);
		} // if
	} // if
else
	{
	//WidgetSNConfig(IDC_FOLIAGEHEIGHT, NULL);
	//WidgetSNConfig(IDC_FOLIAGEDENSITY, NULL);
	//WidgetSNConfig(IDC_ORIENTATIONSHADING, NULL);
	//WidgetSNConfig(IDC_OBJECTHEIGHT, NULL);
	//WidgetSNConfig(IDC_OBJECTDENSITY, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGEHEIGHT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGEDENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_OBJECTHEIGHT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_OBJECTDENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REPLACECOLOR, (RasterAnimHost *)NULL, NULL);
	ConfigureFI(NativeWin, IDC_ROTATEX, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_ROTATEY, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	ConfigureFI(NativeWin, IDC_ROTATEZ, NULL, 1.0, 0.0, 0.0, 0, NULL, 0);
	ConfigureSR(NativeWin, IDC_RADIOIMAGEOBJ, IDC_RADIOIMAGEOBJ, NULL, 0, 0, NULL, 0);
	ConfigureSR(NativeWin, IDC_RADIOIMAGEOBJ, IDC_RADIO3DOBJ, NULL, 0, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKOBJENABLED, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_3DSHADE, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEX, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEY, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKROTATEZ, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_FLIPX, NULL, 0, NULL, 0);
	WidgetSetDisabled(IDC_OBJECTDROP, 1);
	WidgetSetDisabled(IDC_IMAGEDROP, 1);
	WidgetSetDisabled(IDC_APPLYTOGROUP, 1);
	} // else

} // EcotypeEditGUI::ConfigureFoliage

/*===========================================================================*/

void EcotypeEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKDISSOLVEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRENDEROCCLUDED, WP_SCSYNC_NONOTIFY);
WidgetCBSetCurSel(IDC_UNITSDROP, Active->DensityUnits);
WidgetCBSetCurSel(IDC_BAUNITSDROP, Active->BasalAreaUnits);
WidgetCBSetCurSel(IDC_BAUNITSDROP2, Active->BasalAreaUnits);

WidgetSNSync(IDC_ECOTYPEMAXHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ECOTYPEMINHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DENSITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_DISSOLVECOLOR, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_DISSOLVEHEIGHT, WP_FISYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOABSHTECOTYPE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOABSHTFOLGRP, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOABSDENSECOTYPE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOABSDENSFOLGRP, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSCNDHTABS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSCNDHTRELMIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSCNDHTRELRNG, WP_SRSYNC_NONOTIFY);

if (GlobalApp->ForestryAuthorized)
	{
	WidgetSRSync(IDC_RADIODENSSTEMS, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIODENSBASALAREA, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIODENSCLOSURE, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOSIZEHEIGHT, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOSIZEDBH, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOSIZEAGE, WP_SRSYNC_NONOTIFY);
	WidgetSRSync(IDC_RADIOSIZECLOSURE, WP_SRSYNC_NONOTIFY);
	WidgetSNSync(IDC_DENSITY2, WP_FISYNC_NONOTIFY);
	} // if

if (ActiveFoliageValid())
	{
	WidgetSNSync(IDC_GROUPHEIGHT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_GROUPMINHEIGHT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_GROUPDENSITY, WP_FISYNC_NONOTIFY);

	if (GlobalApp->ForestryAuthorized)
		{
		WidgetSNSync(IDC_GROUPDENSITY2, WP_FISYNC_NONOTIFY);
		} // if

	if (ActiveFol)
		{
		WidgetSRSync(IDC_RADIOIMAGEOBJ, WP_SRSYNC_NONOTIFY);
		WidgetSRSync(IDC_RADIO3DOBJ, WP_SRSYNC_NONOTIFY);

		WidgetSCSync(IDC_3DSHADE, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_CHECKROTATEX, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_CHECKROTATEY, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_CHECKROTATEZ, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_FLIPX, WP_SCSYNC_NONOTIFY);

		WidgetSNSync(IDC_FOLIAGEHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FOLIAGEDENSITY, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_ORIENTATIONSHADING, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_OBJECTHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_OBJECTDENSITY, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_REPLACECOLOR, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_ROTATEX, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_ROTATEY, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_ROTATEZ, WP_FISYNC_NONOTIFY);
		} // if
	} // if

} // EcotypeEditGUI::SyncWidgets

/*===========================================================================*/

FoliageGroup *EcotypeEditGUI::BuildGroupList(void)
{
long Found = 0, Pos;
FoliageGroup *CurGrp = Active->FolGrp;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

WidgetLBClear(IDC_GROUPLIST);

while (CurGrp)
	{
	BuildGroupListEntry(ListName, CurGrp);
	Pos = WidgetLBInsert(IDC_GROUPLIST, -1, ListName);
	WidgetLBSetItemData(IDC_GROUPLIST, Pos, CurGrp);
	if (CurGrp == ActiveGroup)
		{
		Found = 1;
		WidgetLBSetCurSel(IDC_GROUPLIST, Pos);
		} // if
	CurGrp = CurGrp->Next;
	} // while

if (! Found)
	{
	if (ActiveGroup = Active->FolGrp)
		{
		WidgetLBSetCurSel(IDC_GROUPLIST, 0);
		ActiveFol = ActiveGroup->Fol;
		}
	} // if

return (ActiveGroup);

} // EcotypeEditGUI::BuildGroupList

/*===========================================================================*/

void EcotypeEditGUI::BuildGroupListEntry(char *ListName, FoliageGroup *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Fol)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // EcotypeEditGUI::BuildGroupListEntry()

/*===========================================================================*/

Foliage *EcotypeEditGUI::BuildFoliageList(void)
{
long Found = 0, Pos;
Foliage *CurFol;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

WidgetLBClear(IDC_FOLIAGELIST);

if (ActiveFoliageValid() && ActiveFol)
	{
	CurFol = ActiveGroup->Fol;
	while (CurFol)
		{
		BuildFoliageListEntry(ListName, CurFol);
		Pos = WidgetLBInsert(IDC_FOLIAGELIST, -1, ListName);
		WidgetLBSetItemData(IDC_FOLIAGELIST, Pos, CurFol);
		if (CurFol == ActiveFol)
			{
			Found = 1;
			WidgetLBSetCurSel(IDC_FOLIAGELIST, Pos);
			} // if
		CurFol = CurFol->Next;
		} // while

	if (! Found)
		{
		if (ActiveFol = ActiveGroup->Fol)
			{
			WidgetLBSetCurSel(IDC_FOLIAGELIST, 0);
			}
		} // if
	} // if

return (ActiveFol);

} // EcotypeEditGUI::BuildFoliageList

/*===========================================================================*/

void EcotypeEditGUI::BuildFoliageListEntry(char *ListName, Foliage *Me)
{

if (Me->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
	{
	if (Me->Img && Me->Img->GetRaster() && Me->Img->GetRaster()->GetEnabled() && Me->Enabled)
		strcpy(ListName, "* ");
	else
		strcpy(ListName, "  ");
	if (Me->Img && Me->Img->GetRaster())
		strcat(ListName, Me->Img->GetRaster()->GetUserName());
	else
		strcpy(ListName, "  No Image Selected");
	} // if
else if (Me->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
	{
	if (Me->Obj && Me->Obj->Enabled && Me->Enabled)
		strcpy(ListName, "* ");
	else
		strcpy(ListName, "  ");
	if (Me->Obj)
		strcat(ListName, Me->Obj->GetName());
	else
		strcpy(ListName, "  No Object Selected");
	} // if
else
	strcpy(ListName, "  No Image Selected");

} // EcotypeEditGUI::BuildFoliageListEntry()

/*===========================================================================*/

void EcotypeEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_UNITSDROP, ! Active->ConstDensity);
WidgetSetDisabled(IDC_DISSOLVEHEIGHT, ! Active->DissolveEnabled);
WidgetSetDisabled(IDC_DISSOLVEREFHT, ! GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize);
WidgetSetDisabled(IDC_DISSOLVECOLOR, ! Active->DissolveEnabled);

WidgetSetDisabled(IDC_ECOTYPEMAXHEIGHT, Active->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
WidgetSetDisabled(IDC_ECOTYPEMINHEIGHT, Active->AbsHeightResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
WidgetSetDisabled(IDC_GROUPMINHEIGHT, Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
WidgetSetDisabled(IDC_DENSITY, Active->AbsDensResident != WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);

if (GlobalApp->ForestryAuthorized)
	{
	WidgetSetDisabled(IDC_BAUNITSDROP, ! (Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA));
	WidgetSetDisabled(IDC_BAUNITSDROP2, ! (Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA));
	WidgetShow(IDC_BAUNITSDROP, Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	WidgetShow(IDC_BAUNITSDROP2, Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);

	WidgetSetDisabled(IDC_RADIODENSSTEMS, ! Active->ConstDensity);
	WidgetSetDisabled(IDC_RADIODENSBASALAREA, ! Active->ConstDensity 
		|| (Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH && Active->AbsDensResident != Active->AbsHeightResident));
	WidgetSetDisabled(IDC_RADIOSIZEDBH, Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA && Active->AbsDensResident != Active->AbsHeightResident);
	// crown closure is available only when size and density are in same level
	WidgetSetDisabled(IDC_RADIODENSCLOSURE, ! Active->ConstDensity || Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE || Active->AbsDensResident != Active->AbsHeightResident);
	WidgetSetDisabled(IDC_RADIOSIZECLOSURE, Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE || Active->AbsDensResident != Active->AbsHeightResident);
	// basal area density and dbh size are available together only when size and density are in same level
	} // if

if (Active->ConstDensity)
	{
	if (GlobalApp->ForestryAuthorized && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
		{
		if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			WidgetSetText(IDC_DENSITY, "Basal Area (BA/Hectare) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			WidgetSetText(IDC_DENSITY, "Basal Area (BA/Acre) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			WidgetSetText(IDC_DENSITY, "Basal Area (BA/Sq Meter) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			WidgetSetText(IDC_DENSITY, "Basal Area (BA/Sq Foot) ");
		else
			WidgetSetText(IDC_DENSITY, "Basal Area (BA/Unit Area) ");
		} // if
	else if (GlobalApp->ForestryAuthorized && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		WidgetSetText(IDC_DENSITY, "Crown Closure (%) ");
		} // if
	else
		{
		if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			WidgetSetText(IDC_DENSITY, "Density (Stems/Hectare) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			WidgetSetText(IDC_DENSITY, "Density (Stems/Acre) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			WidgetSetText(IDC_DENSITY, "Density (Stems/Sq Meter) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			WidgetSetText(IDC_DENSITY, "Density (Stems/Sq Foot) ");
		else
			WidgetSetText(IDC_DENSITY, "Density (Stems/Unit Area) ");
		} // if
	} // if
else
	WidgetSetText(IDC_DENSITY, "Density (% of Polygons) ");

if (Active->ConstDensity && Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	{
	if (GlobalApp->ForestryAuthorized && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
		{
		if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			WidgetSetText(IDC_GROUPDENSITY, "Basal Area (BA/Hectare) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			WidgetSetText(IDC_GROUPDENSITY, "Basal Area (BA/Acre) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			WidgetSetText(IDC_GROUPDENSITY, "Basal Area (BA/Sq Meter) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			WidgetSetText(IDC_GROUPDENSITY, "Basal Area (BA/Sq Foot) ");
		else
			WidgetSetText(IDC_GROUPDENSITY, "Basal Area (BA/Unit Area) ");
		} // if
	else if (GlobalApp->ForestryAuthorized && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		WidgetSetText(IDC_GROUPDENSITY, "Crown Closure (%) ");
		} // if
	else
		{
		if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_HECTARE)
			WidgetSetText(IDC_GROUPDENSITY, "Group Density (Stems/Hectare) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_ACRE)
			WidgetSetText(IDC_GROUPDENSITY, "Group Density (Stems/Acre) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQMETER)
			WidgetSetText(IDC_GROUPDENSITY, "Group Density (Stems/Sq Meter) ");
		else if (Active->DensityUnits == WCS_FOLIAGE_DENSITY_SQFOOT)
			WidgetSetText(IDC_GROUPDENSITY, "Group Density (Stems/Sq Foot) ");
		else
			WidgetSetText(IDC_GROUPDENSITY, "Group Density (Stems/Unit Area) ");
		} // else
	} // if
else if (Active->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	WidgetSetText(IDC_GROUPDENSITY, "Group Density (% of Polygons) ");
else
	WidgetSetText(IDC_GROUPDENSITY, "Group Density (% of Ecotype) ");

if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
	{
	if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum DBH ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum DBH ");
		} // if
	else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum DBH ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min DBH (% of Max) ");
		} // else if
	else
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean DBH ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "DBH Range (+/-%) ");
		} // else if
	WidgetShow(IDC_EDITHTGRAPH, Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	WidgetShow(IDC_EDITHTGRAPH2, Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
	WidgetSetText(IDC_EDITHTGRAPH, "Edit DBH/Height Graph...");
	WidgetSetText(IDC_EDITHTGRAPH2, "Edit DBH/Height Graph...");
	} // if
else if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
	{
	if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Age ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Age ");
		} // if
	else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Age ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Age (% of Max) ");
		} // else if
	else
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Age ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Age Range (+/-%) ");
		} // else if
	WidgetShow(IDC_EDITHTGRAPH, Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE);
	WidgetShow(IDC_EDITHTGRAPH2, Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP);
	WidgetSetText(IDC_EDITHTGRAPH, "Edit Age/Height Graph...");
	WidgetSetText(IDC_EDITHTGRAPH2, "Edit Age/Height Graph...");
	} // if
else if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
	{
	if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Closure (%) ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Closure (%) ");
		} // if
	else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Closure (%) ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Closure (% of Max) ");
		} // else if
	else
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Closure ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Closure Range (+/-%) ");
		} // else if
	WidgetShow(IDC_EDITHTGRAPH, FALSE);
	WidgetShow(IDC_EDITHTGRAPH2, FALSE);
	} // if
else
	{
	if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Height ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Minimum Height ");
		} // if
	else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Maximum Height ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Min Height (% of Max) ");
		} // else if
	else
		{
		WidgetSetText(IDC_ECOTYPEMAXHEIGHT, "Mean Height ");
		WidgetSetText(IDC_ECOTYPEMINHEIGHT, "Height Range (+/-%) ");
		} // else if
	if (GlobalApp->ForestryAuthorized)
		{
		WidgetShow(IDC_EDITHTGRAPH, FALSE);
		WidgetShow(IDC_EDITHTGRAPH2, FALSE);
		} // If
	} // if

if (GlobalApp->ForestryAuthorized && Active->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA
	&& Active->SizeMethod != WCS_FOLIAGE_SIZEMETHOD_DBH)
	{
	WidgetSetText(IDC_DENSITY2, "Diameter (DBH) ");
	WidgetSetText(IDC_GROUPDENSITY2, "Diameter (DBH) ");
	} // if

if (Active->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	WidgetSetText(IDC_GROUPHEIGHT, "Group Height (% of Ecotype) ");
	if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		WidgetSetText(IDC_GROUPMINHEIGHT, "Group Minimum Height ");
		} // if
	else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min Height (% of Max) ");
		} // else if
	else
		{
		WidgetSetText(IDC_GROUPMINHEIGHT, "Group Ht Range (+/-%) ");
		} // else if
	} // if
else
	{
	if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum DBH ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Minimum DBH ");
			} // if
		else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum DBH ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min DBH (% of Max) ");
			} // else if
		else
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Mean DBH ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group DBH Range (+/-%) ");
			} // else if
		} // if
	else if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Age ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Minimum Age ");
			} // if
		else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Age ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min Age (% of Max) ");
			} // else if
		else
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Age ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Age Range (+/-%) ");
			} // else if
		} // else if
	else if (GlobalApp->ForestryAuthorized && Active->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
		{
		if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Closure (%) ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Minimum Closure (%) ");
			} // if
		else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Closure (%) ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min Closure (% of Max) ");
			} // else if
		else
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Closure ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Closure Range (+/-%) ");
			} // else if
		} // else if
	else
		{
		if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Height ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Minimum Height ");
			} // if
		else if (Active->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Maximum Height ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Min Height (% of Max) ");
			} // else if
		else
			{
			WidgetSetText(IDC_GROUPHEIGHT, "Group Mean Height ");
			WidgetSetText(IDC_GROUPMINHEIGHT, "Group Height Range (+/-%) ");
			} // else if
		} // else
	} // else

if (ActiveFoliageValid())
	{
	if (ActiveFol)
		{
		WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 0);
		WidgetSetDisabled(IDC_RADIO3DOBJ, 0);
		} // if
	else
		{
		WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 1);
		WidgetSetDisabled(IDC_RADIO3DOBJ, 1);
		} // else
	} // if
else
	{
	WidgetSetDisabled(IDC_RADIOIMAGEOBJ, 1);
	WidgetSetDisabled(IDC_RADIO3DOBJ, 1);
	} // else

} // EcotypeEditGUI::DisableWidgets

/*===========================================================================*/

void EcotypeEditGUI::SelectPanel(long PanelID)
{

switch (PanelID)
	{
	case 1:
		{
		ShowPanel(1, -1);
		ShowPanel(0, 1);
		break;
		} // 1
	case 2:
		{
		ShowPanel(1, -1);
		ShowPanel(0, 2);
		break;
		} // 1
	case 3:
		{
		ShowPanel(0, 3);
		if (ActiveGroup && ActiveFol)
			{
			if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
				ShowPanel(1, 0);
			else
				ShowPanel(1, 1);
			} // if
		break;
		} // 1
	default:
		{
		ShowPanel(1, -1);
		ShowPanel(0, 0);
		break;
		} // None
	} // PanelID

} // EcotypeEditGUI::SelectPanel

/*===========================================================================*/

void EcotypeEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // EcotypeEditGUI::Cancel

/*===========================================================================*/

void EcotypeEditGUI::SelectNewImage(long Current)
{
Raster *NewRast, *MadeRast = NULL;

if (ActiveFol)
	{
	if (Current < 0)
		Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
	if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)LB_ERR && NewRast)
		|| (MadeRast = NewRast = ImageHost->AddRequestRaster()))
		{
		ActiveFol->SetRaster(NewRast);
		if (MadeRast)
			{
			ImageHost->SetActive(MadeRast);
			} // if
		} // if
	} // if

} // EcotypeEditGUI::SelectNewImage

/*===========================================================================*/

void EcotypeEditGUI::SelectNewObject(long Current)
{
Object3DEffect *OldObj, *NewObj, *MadeObj = NULL;
NotifyTag Changes[2];

if (ActiveFol)
	{
	if (Current < 0)
		Current = WidgetCBGetCurSel(IDC_OBJECTDROP);
	if (((NewObj = (Object3DEffect *)WidgetCBGetItemData(IDC_OBJECTDROP, Current, 0)) != (Object3DEffect *)LB_ERR && NewObj)
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

} // EcotypeEditGUI::SelectNewObject

/*===========================================================================*/

void EcotypeEditGUI::SetActiveGroup(void)
{
long Current;
FoliageGroup *NewGrp;

Current = WidgetLBGetCurSel(IDC_GROUPLIST);
if ((NewGrp = (FoliageGroup *)WidgetLBGetItemData(IDC_GROUPLIST, Current, 0)) != (FoliageGroup *)LB_ERR && NewGrp)
	{
	if (NewGrp != ActiveGroup)
		{
		ActiveGroup = NewGrp;
		ConfigureGroup();
		} // if
	} // if
else
	ConfigureGroup();

} // EcotypeEditGUI::SetActiveGroup

/*===========================================================================*/

void EcotypeEditGUI::SetActiveFoliage(void)
{
long Current;
Foliage *NewFol;

if (ActiveFoliageValid())
	{
	Current = WidgetLBGetCurSel(IDC_FOLIAGELIST);
	if ((NewFol = (Foliage *)WidgetLBGetItemData(IDC_FOLIAGELIST, Current, 0)) != (Foliage *)LB_ERR && NewFol)
		{
		if (NewFol != ActiveFol)
			{
			ActiveFol = NewFol;
			ConfigureFoliage();
			} // if
		} // if
	else
		ConfigureFoliage();
	} // if
else
	ConfigureFoliage();

} // EcotypeEditGUI::SetActiveFoliage

/*===========================================================================*/

void EcotypeEditGUI::EditFoliageObject(void)
{

if (ActiveFoliageValid() && ActiveFol)
	{
	if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
		{
		if (! ActiveFol->Img || ! ActiveFol->Img->GetRaster())
			{
			SelectNewImage(0);
			}
		if (ActiveFol->Img && ActiveFol->Img->GetRaster())
			ActiveFol->Img->GetRaster()->EditRAHost();
		} // if
	else if (ActiveFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
		{
		if (! ActiveFol->Obj)
			{
			SelectNewObject(0);
			}
		if (ActiveFol->Obj)
			ActiveFol->Obj->EditRAHost();
		} // if
	} // if
else
	ConfigureFoliage();

} // EcotypeEditGUI::EditFoliageObject

/*===========================================================================*/

void EcotypeEditGUI::SetDensityUnits(void)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_UNITSDROP);
Active->DensityUnits = (unsigned char)Current;
WidgetCBSetCurSel(IDC_UNITSDROP, Active->DensityUnits);
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // EcotypeEditGUI::SetDensityUnits()

/*===========================================================================*/

void EcotypeEditGUI::SetBAUnits(unsigned short WidID)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(WidID);
Active->BasalAreaUnits = (unsigned char)Current;
WidgetCBSetCurSel(IDC_BAUNITSDROP, Active->BasalAreaUnits);
WidgetCBSetCurSel(IDC_BAUNITSDROP2, Active->BasalAreaUnits);
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // EcotypeEditGUI::SetBAUnits()

/*===========================================================================*/

void EcotypeEditGUI::GroupName(void)
{
char Name[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (ActiveFoliageValid() && WidgetGetModified(IDC_GROUPNAME))
	{
	WidgetGetText(IDC_GROUPNAME, WCS_EFFECT_MAXNAMELENGTH, Name);
	WidgetSetModified(IDC_GROUPNAME, FALSE);
	strncpy(ActiveGroup->Name, Name, WCS_EFFECT_MAXNAMELENGTH);
	ActiveGroup->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Changes[0] = MAKE_ID(ActiveGroup->GetNotifyClass(), ActiveGroup->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, ActiveGroup->GetRAHostRoot());
	} // if 

} // EcotypeEditGUI::GroupName()

/*===========================================================================*/

void EcotypeEditGUI::RemoveGroup(void)
{
FoliageGroup *GroupToRemove;
int RemoveAll = 0;

if (ActiveFoliageValid())
	{
	GroupToRemove = ActiveGroup;
	ActiveGroup = ActiveGroup->Next;
	Active->FindnRemoveRAHostChild(GroupToRemove, RemoveAll);
	} // if

} // EcotypeEditGUI::RemoveGroup

/*===========================================================================*/

void EcotypeEditGUI::RemoveFoliage(void)
{
Foliage *RemoveFol;
int RemoveAll = 0;

if (ActiveFoliageValid() && ActiveFol)
	{
	RemoveFol = ActiveFol;
	ActiveFol = ActiveFol->Next;
	Active->FindnRemoveRAHostChild(RemoveFol, RemoveAll);
	} // if

} // EcotypeEditGUI::RemoveFoliage

/*===========================================================================*/

void EcotypeEditGUI::AddGroup(void)
{
FoliageGroup *NewGroup;
char NewName[WCS_EFFECT_MAXNAMELENGTH];

NewName[0] = 0;
if (GetInputString("Enter name for new Foliage Group", "", NewName) && NewName[0])
	{
	if (NewGroup = Active->AddFoliageGroup(NULL, NewName))
		{
		ActiveGroup = NewGroup;
		ConfigureGroup();
		} // if
	} // if

} // EcotypeEditGUI::AddGroup

/*===========================================================================*/

void EcotypeEditGUI::AddFoliage(void)
{
Foliage *NewFol;

if (ActiveFoliageValid())
	{
	if (NewFol = ActiveGroup->AddFoliage(NULL))
		{
		ActiveFol = NewFol;
		ConfigureFoliage();
		} // if
	} // if

} // EcotypeEditGUI::AddFoliage

/*===========================================================================*/

void EcotypeEditGUI::ChangeGroupListPosition(short MoveUp)
{
long SendNotify = 0;
RasterAnimHost *MoveMe;
FoliageGroup *Current, *PrevGroup = NULL, *PrevPrevGroup = NULL, *StashGroup;
NotifyTag Changes[2];

// don't send notification until all changes are done
if (ActiveFoliageValid())
	{
	MoveMe = ActiveGroup;
	if (MoveUp)
		{
		Current = Active->FolGrp;
		while (Current != MoveMe)
			{
			PrevPrevGroup = PrevGroup;
			PrevGroup = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (PrevGroup)
				{
				StashGroup = Current->Next;
				if (PrevPrevGroup)
					{
					PrevPrevGroup->Next = Current;
					Current->Next = PrevGroup;
					} // if
				else
					{
					Active->FolGrp = Current;
					Active->FolGrp->Next = PrevGroup;
					} // else
				PrevGroup->Next = StashGroup;
				SendNotify = 1;
				} // else if
			} // if
		} // if
	else
		{
		Current = Active->FolGrp;
		while (Current != MoveMe)
			{
			PrevPrevGroup = PrevGroup;
			PrevGroup = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (Current->Next)
				{
				StashGroup = Current->Next->Next;
				if (PrevGroup)
					{
					PrevGroup->Next = Current->Next;
					PrevGroup->Next->Next = Current;
					} // if
				else
					{
					Active->FolGrp = Current->Next;
					Active->FolGrp->Next = Current;
					} // else
				Current->Next = StashGroup;
				SendNotify = 1;
				} // if move down
			} // if
		} // else
	} // if

// need to send a very general message that will cause SAG to completely rebuild
// Just updating the object will cause crash in SAG with NULL pointer
if (SendNotify)
	{
	Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // EcotypeEditGUI::ChangeGroupListPosition

/*===========================================================================*/

void EcotypeEditGUI::ChangeFoliageListPosition(short MoveUp)
{
long SendNotify = 0;
RasterAnimHost *MoveMe;
Foliage *Current, *PrevFol = NULL, *PrevPrevFol = NULL, *StashFol;
NotifyTag Changes[2];

// don't send notification until all changes are done
if (ActiveFoliageValid() && ActiveFol)
	{
	MoveMe = ActiveFol;
	if (MoveUp)
		{
		Current = ActiveGroup->Fol;
		while (Current != MoveMe)
			{
			PrevPrevFol = PrevFol;
			PrevFol = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (PrevFol)
				{
				StashFol = Current->Next;
				if (PrevPrevFol)
					{
					PrevPrevFol->Next = Current;
					Current->Next = PrevFol;
					} // if
				else
					{
					ActiveGroup->Fol = Current;
					ActiveGroup->Fol->Next = PrevFol;
					} // else
				PrevFol->Next = StashFol;
				SendNotify = 1;
				} // else if
			} // if
		} // if
	else
		{
		Current = ActiveGroup->Fol;
		while (Current != MoveMe)
			{
			PrevPrevFol = PrevFol;
			PrevFol = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (Current->Next)
				{
				StashFol = Current->Next->Next;
				if (PrevFol)
					{
					PrevFol->Next = Current->Next;
					PrevFol->Next->Next = Current;
					} // if
				else
					{
					ActiveGroup->Fol = Current->Next;
					ActiveGroup->Fol->Next = Current;
					} // else
				Current->Next = StashFol;
				SendNotify = 1;
				} // if move down
			} // if
		} // else
	} // if

// need to send a very general message that will cause SAG to completely rebuild
// Just updating the object will cause crash in SAG with NULL pointer
if (SendNotify)
	{
	Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // EcotypeEditGUI::ChangeFoliageListPosition

/*===========================================================================*/

void EcotypeEditGUI::OpenPreview(void)
{

if (ActiveFol->Img && ActiveFol->Img->GetRaster())
	{
	ActiveFol->Img->GetRaster()->OpenPreview(FALSE);
	} // if

} // EcotypeEditGUI::OpenPreview()

/*===========================================================================*/

void EcotypeEditGUI::ApplyBackLightToGroup(void)
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
			} // if
		CurFol = CurFol->Next;
		} // while
	} // if

} // EcotypeEditGUI::ApplyBackLightToGroup

/*===========================================================================*/

void EcotypeEditGUI::CopyMaterial(int IsGroup)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost;
char CopyMsg[256];

if (CopyProp = new RasterAnimHostProperties())
	{
	if ((IsGroup && ActiveFoliageValid() && (CopyHost = ActiveGroup))
		|| (! IsGroup && ActiveFoliageValid() && (CopyHost = ActiveFol)))
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
	delete CopyProp;
	} // if

} // EcotypeEditGUI::CopyMaterial

/*===========================================================================*/

void EcotypeEditGUI::PasteMaterial(int IsGroup)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *PasteHost, *CopyHost, *TempHost;
char CopyMsg[256], CopyIllegal = 0;

if (CopyProp = new RasterAnimHostProperties())
	{
	if ((IsGroup && ActiveFoliageValid() && (PasteHost = ActiveGroup))
		|| (! IsGroup && ActiveFoliageValid() && (PasteHost = ActiveFol)))
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
	delete CopyProp;
	} // if

} // EcotypeEditGUI::PasteMaterial

/*===========================================================================*/
