// StreamEditGUI.cpp
// Code for Stream editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "StreamEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"
#include "Raster.h"
#include "PortableFoliageGUI.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


char *StreamEditGUI::TabNames[WCS_STREAMGUI_NUMTABS] = {"General", "Water", "Beach Height", "Beach", "Foliage"};
char *StreamEditGUI::WaterTabNames[4] = {"Water", "Depth", "Foam", "Waves"};

long StreamEditGUI::ActivePage;
long StreamEditGUI::ActivePanel;
// advanced
long StreamEditGUI::DisplayAdvanced;

#ifdef WCS_BUILD_DEMO
// foliage page authorization
extern int ForestryDemoWarned;
#endif // WCS_BUILD_DEMO

#define WCS_STREAMED_WATERMAT_PAGE	1
// foliage page defines
#define WCS_STREAMED_FOLIAGE_PAGE	4
#define WCS_STREAMED_FOLIAGE_PANELSET	2
#define WCS_STREAMED_FOLIAGE_ECOTYPESET	3
// material GUI
#define WCS_STREAMED_BEACH_MATGRADSET	4
#define WCS_STREAMED_WATER_MATGRADSET	6

NativeGUIWin StreamEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // StreamEditGUI::Open

/*===========================================================================*/

NativeGUIWin StreamEditGUI::Construct(void)
{
char *BlendStyles[] = {"Sharp Edge", "Soft Edge", "Quarter Blend", "Half Blend", "Full Blend", "Fast Increase", "Slow Increase", "S-Curve"};
int TabIndex;
#ifdef WCS_FORESTRY_WIZARD
// foliage page authorization
int InhibitForestry = 0;
#endif // WCS_FORESTRY_WIZARD

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_STREAM_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_STREAM_WATERMAT, 0, WCS_STREAMED_WATERMAT_PAGE);
	CreateSubWinFromTemplate(IDD_STREAM_BEACHGRAD, 0, 2);
	CreateSubWinFromTemplate(IDD_STREAM_BEACHMAT_VNS3, 0, 3);
	// foliage page resources
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_VNS3, 0, WCS_STREAMED_FOLIAGE_PAGE);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE1_FORESTRY, WCS_STREAMED_FOLIAGE_PANELSET, 0);
	// forestry
	#ifdef WCS_FORESTRY_WIZARD
	if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("inhibit_forestry"))
		{
		InhibitForestry = 1;
		GlobalApp->ForestryAuthorized = 0;
		} // if
	if (GlobalApp->ForestryAuthorized || (GlobalApp->ForestryAuthorized = ((GlobalApp->Sentinal->CheckAuthFieldForestry() 
		|| GlobalApp->Sentinal->CheckRenderEngineQuick()) && ! InhibitForestry) ? 1: 0))
		{
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE2_FORESTRY, WCS_STREAMED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE3_FORESTRY, WCS_STREAMED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP_FORESTRY, WCS_STREAMED_FOLIAGE_PANELSET, 3);
		#ifdef WCS_BUILD_DEMO
		if (! ForestryDemoWarned)
			UserMessageOK("Forestry Edition", "The Forestry Edition is an optional upgrade to the basic VNS program. It enables you to input foliage size and density in a suite of typical forestry style variables that are not available in the basic VNS version. Those extra features are enabled in this Demo version so you can see how they work. If you want these extra features be sure to specify that you want the Forestry Edition when you purchase VNS. It also includes the Forestry Wizard for expediting the setup of forestry projects.");
		ForestryDemoWarned = 1;
		#endif // WCS_BUILD_DEMO
		} // if
	// non-forestry
	else
	#endif // WCS_FORESTRY_WIZARD
		{
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE2, WCS_STREAMED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE3, WCS_STREAMED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP, WCS_STREAMED_FOLIAGE_PANELSET, 3);
		} // else
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGE, WCS_STREAMED_FOLIAGE_PANELSET, 4);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_3DOBJECT, WCS_STREAMED_FOLIAGE_PANELSET, 5);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPETAB, WCS_STREAMED_FOLIAGE_ECOTYPESET, 0);
	// end foliage resources

	CreateSubWinFromTemplate(IDD_STREAM_WATERPROP_TRANSP, 1, 0);
	CreateSubWinFromTemplate(IDD_STREAM_ELEVATION, 1, 1);
	CreateSubWinFromTemplate(IDD_STREAM_FOAM, 1, 2);
	CreateSubWinFromTemplate(IDD_STREAM_WAVES, 1, 3);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_STREAMGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		for (TabIndex = 0; TabIndex < 4; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB2, TabIndex, WaterTabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetTCSetCurSel(IDC_TAB2, ActivePanel);
		// foliage page setup
		FolGUI->EcotypeTabSetup();
		FolGUI->FillUnitDrops();
		FolGUI->BuildImageList(false);
		FolGUI->BuildObjectList(false);
		// Material GUI
		BeachMatGUI->Construct(WCS_STREAMED_BEACH_MATGRADSET, WCS_STREAMED_BEACH_MATGRADSET + 1);
		WaterMatGUI->Construct(WCS_STREAMED_WATER_MATGRADSET, WCS_STREAMED_WATER_MATGRADSET + 1);
		ShowPanels();
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // StreamEditGUI::Construct

/*===========================================================================*/

StreamEditGUI::StreamEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, StreamEffect *ActiveSource)
: GUIFenetre('STRM', this, "Stream Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_STREAM, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_WAVE, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_WAVE, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
			/* foliage page */	MAKE_ID(0xff, WCS_SUBCLASS_ECOTYPE, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
ProjHost = ProjSource;
Active = ActiveSource;
ActiveBeachGrad = ActiveWaterGrad = NULL;
// Material GUI
BeachMatGUI = WaterMatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Stream Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// foliage page initialization
	if (! (FolGUI = new PortableFoliageGUI(this, EffectsSource, ImageSource, &Active->BeachMat, NULL,
		WCS_STREAMED_FOLIAGE_PAGE, WCS_STREAMED_FOLIAGE_PANELSET, WCS_STREAMED_FOLIAGE_ECOTYPESET, ActivePage)))
		ConstructError = 1;
	// Material GUI
	if (BeachMatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->BeachMat, WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER, WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER)) // init ordinal 0
		{
		PopDropBeachMaterialNotifier.Host = this; // to be able to call notifications later
		BeachMatGUI->SetNotifyFunctor(&PopDropBeachMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	if (WaterMatGUI = new PortableMaterialGUI(1, this, EffectsSource, Active, &Active->WaterMat, WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER, WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER)) // init ordinal 1
		{
		PopDropWaterMaterialNotifier.Host = this; // to be able to call notifications later
		WaterMatGUI->SetNotifyFunctor(&PopDropWaterMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // StreamEditGUI::StreamEditGUI

/*===========================================================================*/

StreamEditGUI::~StreamEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (BeachMatGUI)
	delete BeachMatGUI;
if (WaterMatGUI)
	delete WaterMatGUI;

// foliage page destructor
if (FolGUI)
	delete FolGUI;

} // StreamEditGUI::~StreamEditGUI()

/*===========================================================================*/

long StreamEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SEG, 0);

return(0);

} // StreamEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long StreamEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{

DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);

} // StreamEditGUI::HandleShowAdvanced

/*===========================================================================*/

long StreamEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_SEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveWaterGrad && ActiveWaterGrad->GetThing())
			((MaterialEffect *)ActiveWaterGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveWaterGrad && ActiveWaterGrad->GetThing())
			((MaterialEffect *)ActiveWaterGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_LOADBEACHCOMPONENT:
		{
		if (ActiveBeachGrad && ActiveBeachGrad->GetThing())
			((MaterialEffect *)ActiveBeachGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVEBEACHCOMPONENT:
		{
		if (ActiveBeachGrad && ActiveBeachGrad->GetThing())
			((MaterialEffect *)ActiveBeachGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDWAVE:
		{
		AddWave();
		break;
		} // IDC_ADDWAVE
	case IDC_REMOVEWAVE:
		{
		RemoveWave();
		break;
		} // IDC_REMOVEWAVE
	// material GUI
	case IDC_POPDROP0:
		{
		if (WidgetGetCheck(IDC_POPDROP0))
			{
			ShowWaterMaterialPopDrop(true);			} // if
		else
			{
			ShowWaterMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	case IDC_POPDROP1:
		{
		if (WidgetGetCheck(IDC_POPDROP1))
			{
			ShowBeachMaterialPopDrop(true);			} // if
		else
			{
			ShowBeachMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP1
	default: break;
	} // ButtonID

// foliage page
FolGUI->HandleButtonClick(Handle, NW, ButtonID);
// Material GUI
BeachMatGUI->HandleButtonClick(Handle, NW, ButtonID);
WaterMatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // StreamEditGUI::HandleButtonClick

/*===========================================================================*/

long StreamEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

// foliage page
FolGUI->HandleButtonDoubleClick(Handle, NW, ButtonID);

return(0);

} // StreamEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long StreamEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_WAVELIST:
		{
		RemoveWave();
		break;
		} // IDC_WAVELIST
	} // switch

// foliage page
FolGUI->HandleListDelItem(Handle, NW, CtrlID, ItemData);

return(0);

} // StreamEditGUI::HandleListDelItem

/*===========================================================================*/

long StreamEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WAVELIST:
		{
		EditWave();
		break;
		}
	} // switch CtrlID

return (0);

} // StreamEditGUI::HandleListDoubleClick

/*===========================================================================*/

long StreamEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListCopyItem(Handle, NW, CtrlID, ItemData);

return(0);

} // StreamEditGUI::HandleListCopyItem

/*===========================================================================*/

long StreamEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListPasteItem(Handle, NW, CtrlID, ItemData);

return(0);

} // StreamEditGUI::HandleListPasteItem

/*===========================================================================*/

long StreamEditGUI::HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{

// foliage page
FolGUI->HandleTreeMenuSelect(Handle, NW, CtrlID, TreeItem, RAH, ActionText, Derived);

return(0);
} // StreamEditGUI::HandleTreeMenuSelect

/*===========================================================================*/

long StreamEditGUI::HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData)
{

// foliage page
FolGUI->HandleTreeChange(Handle, NW, CtrlID, OldTreeItem, NewTreeItem, TreeItemData);

return (0);

} // StreamEditGUI::HandleTreeChange

/*===========================================================================*/

long StreamEditGUI::HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand)
{

// foliage page
FolGUI->HandleTreeExpand(Handle, NW, CtrlID, TreeItem, TreeItemData, Pre, Expand);

return (0);

} // StreamEditGUI::HandleTreeExpand

/*===========================================================================*/

long StreamEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// foliage page
FolGUI->HandleCBChange(Handle, NW, CtrlID);
// Material GUI
BeachMatGUI->HandleCBChange(Handle, NW, CtrlID);
WaterMatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // StreamEditGUI::HandleCBChange

/*===========================================================================*/

long StreamEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	} // switch CtrlID

// foliage page
FolGUI->HandleStringLoseFocus(Handle, NW, CtrlID);
// Material GUI
BeachMatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);
WaterMatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);

return (0);

} // StreamEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long StreamEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// Material GUI
ShowBeachMaterialPopDrop(false);
ShowWaterMaterialPopDrop(false);

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		ActivePage = NewPageID;
		FolGUI->SetActivePage(ActivePage);
		break;
		}
	case IDC_TAB2:
		{
		ActivePanel = NewPageID;
		break;
		}
	} // switch

// foliage page
FolGUI->HandlePageChange(Handle, NW, CtrlID, NewPageID);
ShowPanels();

return(0);

} // StreamEditGUI::HandlePageChange

/*===========================================================================*/

void StreamEditGUI::ShowPanels(void)
{

ShowPanel(0, ActivePage);
ShowPanel(1, ActivePage == WCS_STREAMED_WATERMAT_PAGE ? ActivePanel: -1);
// foliage page
if (ActivePage == WCS_STREAMED_FOLIAGE_PAGE)
	FolGUI->ShowPanels();
else
	FolGUI->HidePanels();
	
} // StreamEditGUI::ShowPanels

/*===========================================================================*/

long StreamEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
		} // 
	//case IDC_CHECKOVERLAP:
	case IDC_CHECKSPLINE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	//case IDC_CHECKFLOATING:
	//	{
	//	EffectsHost->StreamBase.SetFloating(EffectsHost->StreamBase.Floating, ProjHost);		// this sends the valuechanged message
	//	break;
	//	} // 
	} // switch CtrlID

// foliage page
FolGUI->HandleSCChange(Handle, NW, CtrlID);

return(0);

} // StreamEditGUI::HandleSCChange

/*===========================================================================*/

long StreamEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// foliage page
FolGUI->HandleSRChange(Handle, NW, CtrlID);

return(0);

} // StreamEditGUI::HandleSRChange

/*===========================================================================*/

long StreamEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// foliage page
FolGUI->HandleFIChange(Handle, NW, CtrlID);
// Material GUI
BeachMatGUI->HandleFIChange(Handle, NW, CtrlID);
WaterMatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // StreamEditGUI::HandleFIChange

/*===========================================================================*/

void StreamEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Changed, Interested[8];
long Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[6] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[7] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
	FolGUI->DisableFoliageWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->BeachMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), Active->BeachMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureBeach();
	ConfigureWater();
	ConfigureColors();
	DisableWidgets();
	// foliage page
	FolGUI->DisableFoliageWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
	ConfigureColors();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ROOTTEXTURE, 0xff, 0xff);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_TEXTURE, 0xff, 0xff);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ECOTYPE, 0xff, 0xff);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_FOLIAGEGRP, 0xff, 0xff);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_FOLIAGE, 0xff, 0xff);
Interested[5] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (((Changed & 0x000000ff) != WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED)
		|| ((Changed & 0x0000ff00) == (WCS_SUBCLASS_ROOTTEXTURE << 8))
		|| ((Changed & 0x0000ff00) == (WCS_EFFECTSSUBCLASS_THEMATICMAP << 8)))
		{
		SyncWidgets();
		// foliage page
		FolGUI->SyncFoliageWidgets();
		ConfigureColors();
		Done = 1;
		} // if
	} // if

// foliage page
Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = ImageHost->MatchNotifyClass(Interested, Changes, 0))
	{
	FolGUI->BuildImageList(true);
	Done = 1;
	} // if image name changed
else
	{
	Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (Changed = GlobalApp->AppImages->MatchNotifyClass(Interested, Changes, 0))
		{
		FolGUI->ConfigureFoliageNail();
		Done = 1;
		} // if
	} // else

// foliage page
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	FolGUI->BuildObjectList(true);
	Done = 1;
	} // if image name changed

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
	Done = 1;
	} // if

#ifdef WCS_BUILD_VNS
// query drop
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	WidgetLWSync(IDC_VECLINKAGE);
	Done = 1;
	} // if query changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // StreamEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void StreamEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->StreamBase.Resolution,
  1.0,
   0.00001,
	1000000.0,
	 FIOFlag_Float,
	  NULL,
	   NULL);*/

ConfigureFI(NativeWin, IDC_PRIORITY,
 &Active->Priority,
  1.0,
   -99.0,
	99.0,
	 FIOFlag_Short,
	  NULL,
	   0);

sprintf(TextStr, "Stream Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->StreamBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->StreamBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSPLINE, &Active->Splined, SCFlag_Char, NULL, 0);

WidgetSNConfig(IDC_RADIUS, &Active->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS]);
WidgetSmartRAHConfig(IDC_BEACHHTMIN, &Active->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT], Active);
WidgetSmartRAHConfig(IDC_BEACHHTMAX, &Active->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR], Active);

WidgetAGConfig(IDC_ANIMGRADIENT2, &Active->BeachMat);
WidgetAGConfig(IDC_WATERANIMGRADIENT, &Active->WaterMat);

// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_POPDROP1, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_ADDWAVE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEWAVE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_LOADBEACHCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVEBEACHCOMPONENT, IDI_FILESAVE, NULL);
// foliage page
FolGUI->ConfigureWidgets();
// Material GUI
BeachMatGUI->ConfigureWidgets();
WaterMatGUI->ConfigureWidgets();

//BuildWaveList();
ConfigureBeach();
ConfigureWater();
ConfigureColors();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // StreamEditGUI::ConfigureWidgets()

/*===========================================================================*/

void StreamEditGUI::ConfigureBeach(void)
{
MaterialEffect *Mat;
char GroupWithMatName[200];

if ((ActiveBeachGrad = Active->BeachMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveBeachGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_BEACHLUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHTRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHSPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHSPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_BEACHREFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHBUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BEACHBUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_FOLIAGE1, (RasterAnimHost **)&Mat->EcoFol[0], Mat);
	WidgetSmartRAHConfig(IDC_FOLIAGE2, (RasterAnimHost **)&Mat->EcoFol[1], Mat);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)&Mat->Strata, Mat);
	WidgetSmartRAHConfig(IDC_BEACHDIFFUSECOLOR, &Mat->DiffuseColor, Mat);

	sprintf(GroupWithMatName, "Selected Beach Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS, GroupWithMatName);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_BEACHLUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHTRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHSPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHSPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHREFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHBUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHBUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGE1, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGE2, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BEACHDIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);

	WidgetSetText(IDC_MATERIALS, "Selected Beach Material");
	} // else
// foliage page
FolGUI->ConfigureAllFoliage();
// Material GUI
BeachMatGUI->ConfigureMaterial();

} // StreamEditGUI::ConfigureBeach

/*===========================================================================*/

void StreamEditGUI::ConfigureWater(void)
{
MaterialEffect *Mat;

if ((ActiveWaterGrad = Active->WaterMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveWaterGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE], Mat);
	WidgetSmartRAHConfig(IDC_TRANSLUMEXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP], Mat);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_DISPLACEMENT, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT], Mat);
	WidgetSmartRAHConfig(IDC_WATERDEPTH, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH], Mat);
	WidgetSmartRAHConfig(IDC_FOAMCOVERAGE, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE], Mat);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH], Mat);
	WidgetSmartRAHConfig(IDC_INTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	WidgetSmartRAHConfig(IDC_FOAM, (RasterAnimHost **)&Mat->Foam, Mat);

	if (Mat->Foam)
		{
		WidgetSmartRAHConfig(IDC_FOAMLUMINOSITY, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMSPECULARITY, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMSPECULAREXP, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMTRANSLUMINANCE, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMTRANSLUMEXP, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMREFLECTIVITY, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMINTENSITY, &Mat->Foam->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat->Foam);
		WidgetSmartRAHConfig(IDC_FOAMDIFFUSECOLOR, &Mat->Foam->DiffuseColor, Mat->Foam);
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_FOAMLUMINOSITY, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMSPECULARITY, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMSPECULAREXP, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMTRANSLUMINANCE, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMTRANSLUMEXP, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMREFLECTIVITY, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMINTENSITY, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FOAMDIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
		} // else
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSLUMINANCE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSLUMEXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DISPLACEMENT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_WATERDEPTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMCOVERAGE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAM, (RasterAnimHost **)NULL, NULL);

	WidgetSmartRAHConfig(IDC_FOAMLUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMSPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMSPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMTRANSLUMINANCE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMTRANSLUMEXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMREFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOAMDIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	} // else

BuildWaveList();

// Material GUI
WaterMatGUI->ConfigureMaterial();

} // StreamEditGUI::ConfigureWater

/*===========================================================================*/

void StreamEditGUI::BuildWaveList(void)
{
EffectList *Current;
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;
MaterialEffect *Mat;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

if ((ActiveWaterGrad = Active->WaterMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveWaterGrad->GetThing()))
	{
	Current = Mat->Waves;

	NumListItems = WidgetLBGetCount(IDC_WAVELIST);

	for (TempCt = 0; TempCt < NumListItems; TempCt ++)
		{
		if (WidgetLBGetSelState(IDC_WAVELIST, TempCt))
			{
			NumSelected ++;
			} // if
		} // for

	if (NumSelected)
		{
		if (SelectedItems = (GeneralEffect **)AppMem_Alloc(NumSelected * sizeof (GeneralEffect *), 0))
			{
			for (TempCt = 0; TempCt < NumListItems; TempCt ++)
				{
				if (WidgetLBGetSelState(IDC_WAVELIST, TempCt))
					{
					SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_WAVELIST, TempCt);
					} // if
				} // for
			} // if
		} // if

	while (Current || Ct < NumListItems)
		{
		CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_WAVELIST, Ct): NULL;
		
		if (Current)
			{
			if (Current->Me)
				{
				if (Current->Me == (GeneralEffect *)CurrentRAHost)
					{
					BuildWaveListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_WAVELIST, Ct, ListName);
					WidgetLBSetItemData(IDC_WAVELIST, Ct, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_WAVELIST, 1, Ct);
								break;
								} // if
							} // for
						} // if
					Ct ++;
					} // if
				else
					{
					FoundIt = 0;
					for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
						{
						if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_WAVELIST, TempCt))
							{
							FoundIt = 1;
							break;
							} // if
						} // for
					if (FoundIt)
						{
						BuildWaveListEntry(ListName, Current->Me);
						WidgetLBReplace(IDC_WAVELIST, TempCt, ListName);
						WidgetLBSetItemData(IDC_WAVELIST, TempCt, Current->Me);
						if (SelectedItems)
							{
							for (SelCt = 0; SelCt < NumSelected; SelCt ++)
								{
								if (SelectedItems[SelCt] == Current->Me)
									{
									WidgetLBSetSelState(IDC_WAVELIST, 1, TempCt);
									break;
									} // if
								} // for
							} // if
						for (TempCt -- ; TempCt >= Ct; TempCt --)
							{
							WidgetLBDelete(IDC_WAVELIST, TempCt);
							NumListItems --;
							} // for
						Ct ++;
						} // if
					else
						{
						BuildWaveListEntry(ListName, Current->Me);
						Place = WidgetLBInsert(IDC_WAVELIST, Ct, ListName);
						WidgetLBSetItemData(IDC_WAVELIST, Place, Current->Me);
						if (SelectedItems)
							{
							for (SelCt = 0; SelCt < NumSelected; SelCt ++)
								{
								if (SelectedItems[SelCt] == Current->Me)
									{
									WidgetLBSetSelState(IDC_WAVELIST, 1, Place);
									break;
									} // if
								} // for
							} // if
						NumListItems ++;
						Ct ++;
						} // else
					} // if
				} // if
			Current = Current->Next;
			} // if
		else
			{
			WidgetLBDelete(IDC_WAVELIST, Ct);
			NumListItems --;
			} // else
		} // while

	if (SelectedItems)
		AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));
	} // if
else
	{
	WidgetLBClear(IDC_WAVELIST);
	} // else

} // StreamEditGUI::BuildWaveList

/*===========================================================================*/

void StreamEditGUI::BuildWaveListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // StreamEditGUI::BuildWaveListEntry()

/*===========================================================================*/

void StreamEditGUI::ConfigureColors(void)
{

// this is harmless to call even if there is no active gradient node, it will cause 
// a valid node to be set if there is one..
WidgetAGSync(IDC_ANIMGRADIENT2);

ActiveBeachGrad = Active->BeachMat.GetActiveNode();
// Material GUI
BeachMatGUI->SyncWidgets();

// this is harmless to call even if there is no active gradient node, it will cause 
// a valid node to be set if there is one..
WidgetAGSync(IDC_WATERANIMGRADIENT);

ActiveWaterGrad = Active->WaterMat.GetActiveNode();
// Material GUI
WaterMatGUI->SyncWidgets();

} // StreamEditGUI::ConfigureColors

/*===========================================================================*/

void StreamEditGUI::SyncWidgets(void)
{

if ((Active->BeachMat.GetActiveNode() != ActiveBeachGrad)
	|| (Active->WaterMat.GetActiveNode() != ActiveWaterGrad))
	{
	ConfigureWidgets();
	return;
	} // if

//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKOVERLAP, WP_SCSYNC_NONOTIFY);
//WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSPLINE, WP_SCSYNC_NONOTIFY);

WidgetSNSync(IDC_BEACHHTMIN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BEACHHTMAX, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RADIUS, WP_FISYNC_NONOTIFY);

if (ActiveBeachGrad = Active->BeachMat.GetActiveNode())
	{
	WidgetSNSync(IDC_BEACHLUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHTRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHSPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHSPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHREFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHBUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHBUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOLIAGE1, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOLIAGE2, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_STRATA, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BEACHDIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	} // if
if (ActiveWaterGrad = Active->WaterMat.GetActiveNode())
	{
	WidgetSNSync(IDC_LUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSLUMINANCE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSLUMEXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DISPLACEMENT, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_WATERDEPTH, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMCOVERAGE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAM, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMLUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMSPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMSPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMTRANSLUMINANCE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMTRANSLUMEXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMREFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOAMDIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	} // if

// Material GUI
BeachMatGUI->SyncWidgets();
WaterMatGUI->SyncWidgets();

} // StreamEditGUI::SyncWidgets

/*===========================================================================*/

void StreamEditGUI::DisableWidgets(void)
{
MaterialEffect *Mat;

//WidgetSetDisabled(IDC_PRIORITY, ! Active->Joes);

// beach material

// water material
Mat = ActiveWaterGrad ? (MaterialEffect *)ActiveWaterGrad->GetThing(): NULL;
WidgetSetDisabled(IDC_FOAMCOVERAGE, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMLUMINOSITY, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMSPECULARITY, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMSPECULAREXP, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMTRANSLUMINANCE, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMTRANSLUMEXP, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMREFLECTIVITY, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMINTENSITY, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);
WidgetSetDisabled(IDC_FOAMDIFFUSECOLOR, ! ActiveWaterGrad || ! Mat || ! Mat->Foam);

WidgetSetDisabled(IDC_LUMINOSITY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_SPECULARITY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_SPECULAREXP, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_TRANSLUMINANCE, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_TRANSLUMEXP, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_REFLECTIVITY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_DISPLACEMENT, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_WATERDEPTH, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_TRANSPARENCY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_INTENSITY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_BUMPINTENSITY, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_BUMP, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_DIFFUSECOLOR, ! ActiveWaterGrad || ! Mat);
WidgetSetDisabled(IDC_FOAM, ! ActiveWaterGrad || ! Mat);

Mat = ActiveBeachGrad ? (MaterialEffect *)ActiveBeachGrad->GetThing(): NULL;
WidgetSetDisabled(IDC_BEACHLUMINOSITY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHTRANSPARENCY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHSPECULARITY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHSPECULAREXP, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHREFLECTIVITY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHINTENSITY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHBUMPINTENSITY, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHBUMP, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_FOLIAGE1, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_FOLIAGE2, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_STRATA, ! ActiveBeachGrad || ! Mat);
WidgetSetDisabled(IDC_BEACHDIFFUSECOLOR, ! ActiveBeachGrad || ! Mat);

#ifdef WCS_BUILD_VNS
WidgetShow(IDC_CHECKSPLINE, true);
#else // WCS_BUILD_VNS
WidgetShow(IDC_CHECKSPLINE, false);
#endif // WCS_BUILD_VNS

// Material GUI
BeachMatGUI->DisableWidgets();
WaterMatGUI->DisableWidgets();

} // StreamEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void StreamEditGUI::DisplayAdvancedFeatures(void)
{
bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

// All the material properties are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced)
	{
	// beach
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_BEACHSPECULARITY, true);
	WidgetShow(IDC_BEACHSPECULAREXP, true);
	WidgetShow(IDC_BEACHREFLECTIVITY, true);
	WidgetShow(IDC_BEACHTRANSPARENCY, true);
	WidgetShow(IDC_BEACHLUMINOSITY, true);
	WidgetShow(IDC_ANIMGRADIENT2, ! BeachMatGUI->QueryIsDisplayed());
	WidgetShow(IDC_POPDROP1, true);
	// water
	WidgetShow(IDC_HIDDENCONTROLMSG5, false);
	WidgetShow(IDC_HIDDENCONTROLMSG8, false);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_TRANSLUMINANCE, true);
	WidgetShow(IDC_TRANSLUMEXP, true);
	WidgetShow(IDC_WATERANIMGRADIENT, ! WaterMatGUI->QueryIsDisplayed());
	WidgetShow(IDC_POPDROP0, true);
	// foam
	WidgetShow(IDC_HIDDENCONTROLMSG6, false);
	WidgetShow(IDC_FOAMLUMINOSITY, true);
	WidgetShow(IDC_FOAMSPECULARITY, true);
	WidgetShow(IDC_FOAMSPECULAREXP, true);
	WidgetShow(IDC_FOAMTRANSLUMINANCE, true);
	WidgetShow(IDC_FOAMTRANSLUMEXP, true);
	WidgetShow(IDC_FOAMREFLECTIVITY, true);
	} 
else
	{
	// beach
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_BEACHSPECULARITY, false);
	WidgetShow(IDC_BEACHSPECULAREXP, false);
	WidgetShow(IDC_BEACHREFLECTIVITY, false);
	WidgetShow(IDC_BEACHTRANSPARENCY, false);
	WidgetShow(IDC_BEACHLUMINOSITY, false);
	WidgetShow(IDC_ANIMGRADIENT2, false);
	WidgetShow(IDC_POPDROP1, false);
	// water
	WidgetShow(IDC_HIDDENCONTROLMSG5, true);
	WidgetShow(IDC_HIDDENCONTROLMSG8, true);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_TRANSLUMINANCE, false);
	WidgetShow(IDC_TRANSLUMEXP, false);
	WidgetShow(IDC_WATERANIMGRADIENT, false);
	WidgetShow(IDC_POPDROP0, false);
	// foam
	WidgetShow(IDC_HIDDENCONTROLMSG6, true);
	WidgetShow(IDC_FOAMLUMINOSITY, false);
	WidgetShow(IDC_FOAMSPECULARITY, false);
	WidgetShow(IDC_FOAMSPECULAREXP, false);
	WidgetShow(IDC_FOAMTRANSLUMINANCE, false);
	WidgetShow(IDC_FOAMTRANSLUMEXP, false);
	WidgetShow(IDC_FOAMREFLECTIVITY, false);
	// Material GUI
	ShowBeachMaterialPopDrop(false);
	ShowWaterMaterialPopDrop(false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // StreamEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void StreamEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // StreamEditGUI::Cancel

/*===========================================================================*/

void StreamEditGUI::AddWave(void)
{
MaterialEffect *Mat;

if ((ActiveWaterGrad = Active->WaterMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveWaterGrad->GetThing()))
	EffectsHost->AddAttributeByList(Mat, WCS_EFFECTSSUBCLASS_WAVE);

} // StreamEditGUI::AddWave

/*===========================================================================*/

void StreamEditGUI::RemoveWave(void)
{
RasterAnimHost **RemoveItems;
MaterialEffect *Mat;
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;

if ((ActiveWaterGrad = Active->WaterMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveWaterGrad->GetThing()))
	{
	if ((NumListEntries = WidgetLBGetCount(IDC_WAVELIST)) > 0)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_WAVELIST, Ct))
				{
				NumSelected ++;
				} // if
			} // for
		if (NumSelected)
			{
			if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
				{
				for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
					{
					if (WidgetLBGetSelState(IDC_WAVELIST, Ct))
						{
						RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_WAVELIST, Ct);
						} // if
					} // for
				for (Ct = 0; Ct < NumSelected; Ct ++)
					{
					if (RemoveItems[Ct])
						{
						if (Mat->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
							{
							//EffectsHost->RemoveRAHost(RemoveItems[Ct], 0);
							} // if
						} // if
					} // for
				AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
				} // if
			} // if
		else
			{
			UserMessageOK("Remove Wave", "There are no Waves selected to remove.");
			} // else
		} // if
	} // if

} // StreamEditGUI::RemoveWave

/*===========================================================================*/

void StreamEditGUI::EditWave(void)
{
RasterAnimHost *EditMe;
long Ct, NumListEntries;

if ((NumListEntries = WidgetLBGetCount(IDC_WAVELIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_WAVELIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_WAVELIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // StreamEditGUI::EditWave

/*===========================================================================*/

void StreamEditGUI::Name(void)
{
NotifyTag Changes[2];
char NewName[WCS_EFFECT_MAXNAMELENGTH];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // StreamEditGUI::Name()

/*===========================================================================*/

void StreamEditGUI::SetToFoliagePage(void)
{

ActivePage = WCS_STREAMED_FOLIAGE_PAGE;
FolGUI->SetActivePage(ActivePage);
WidgetTCSetCurSel(IDC_TAB1, ActivePage);
ShowPanels();

} // StreamEditGUI::SetToFoliagePage

/*===========================================================================*/

void StreamEditGUI::ActivateFoliageItem(RasterAnimHost *ActivateMe)
{

FolGUI->EditFoliageItem(ActivateMe, 0);

} // StreamEditGUI::ActivateFoliageItem

/*===========================================================================*/

// Material GUI
void StreamEditGUIBeachPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{

if (Host) Host->ConfigureBeach();

} // StreamEditGUIBeachPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void StreamEditGUIBeachPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{

if (Host) Host->SetNewActiveBeachGrad(NewNode);

} // StreamEditGUIBeachPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// Material GUI
void StreamEditGUIWaterPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{

if (Host) Host->ConfigureWater();

} // StreamEditGUIWaterPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void StreamEditGUIWaterPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{

if (Host) Host->SetNewActiveWaterGrad(NewNode);

} // StreamEditGUIWaterPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

// material GUI
void StreamEditGUI::ShowBeachMaterialPopDrop(bool ShowState)
{

if (ShowState)
	{
	// position and show
	if (BeachMatGUI)
		{
		ShowPanelAsPopDrop(IDC_POPDROP1, BeachMatGUI->GetPanel(), 0, SubPanels[0][1]);
		WidgetShow(IDC_ANIMGRADIENT2, 0); // hide master gradient widget since it looks weird otherwise
		WidgetSetCheck(IDC_POPDROP1, true);
		} // if
	} // if
else
	{
	if (BeachMatGUI)
		{
		ShowPanel(BeachMatGUI->GetPanel(), -1); // hide
		WidgetShow(IDC_ANIMGRADIENT2, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
		WidgetSetCheck(IDC_POPDROP1, false);
		} // if
	} // else

} // StreamEditGUI::ShowBeachMaterialPopDrop

/*===========================================================================*/

void StreamEditGUI::ShowWaterMaterialPopDrop(bool ShowState)
{
if (ShowState)
	{
	// position and show
	if (WaterMatGUI)
		{
		ShowPanelAsPopDrop(IDC_POPDROP0, WaterMatGUI->GetPanel(), 0, SubPanels[0][1]);
		WidgetShow(IDC_WATERANIMGRADIENT, 0); // hide master gradient widget since it looks weird otherwise
		WidgetSetCheck(IDC_POPDROP0, true);
		} // if
	} // if
else
	{
	if (WaterMatGUI)
		{
		ShowPanel(WaterMatGUI->GetPanel(), -1); // hide
		WidgetShow(IDC_WATERANIMGRADIENT, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
		WidgetSetCheck(IDC_POPDROP0, false);
		} // if
	} // else

} // StreamEditGUI::ShowWaterMaterialPopDrop

/*===========================================================================*/

bool StreamEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->BeachMat.CountNodes() > 1 || Active->WaterMat.CountNodes() > 1 ? true : false);

} // StreamEditGUI::QueryLocalDisplayAdvancedUIVisibleState
