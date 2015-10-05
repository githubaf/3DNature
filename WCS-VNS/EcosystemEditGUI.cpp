// EcosystemEditGUI.cpp
// Code for Ecosystem editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EcosystemEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Database.h"
#include "resource.h"
#include "Lists.h"
#include "Raster.h"
#include "AppMem.h"
#include "PortableFoliageGUI.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

char *EcosystemEditGUI::TabNames[WCS_ECOSYSTEMGUI_NUMTABS] = {"General", "Material", "Foliage", "Rules", "Color Map"};

long EcosystemEditGUI::ActivePage;
// advanced
long EcosystemEditGUI::DisplayAdvanced;

#ifdef WCS_BUILD_DEMO
// foliage page authorization
int ForestryDemoWarned;
#endif // WCS_BUILD_DEMO

// foliage page defines
#define WCS_ECOSYSED_FOLIAGE_PAGE	2
#define WCS_ECOSYSED_FOLIAGE_PANELSET	1
#define WCS_ECOSYSED_FOLIAGE_ECOTYPESET	2
// material GUI
#define WCS_ECOSYSED_MATGRADSET	3

NativeGUIWin EcosystemEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // EcosystemEditGUI::Open

/*===========================================================================*/

NativeGUIWin EcosystemEditGUI::Construct(void)
{
int TabIndex;
#ifdef WCS_FORESTRY_WIZARD
// foliage page authorization
int InhibitForestry = 0;
#endif // WCS_FORESTRY_WIZARD

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_GENERAL_VNS3, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_GENERAL, 0, 0);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_MATERIAL_VNS3, 0, 1);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_VNS3, 0, WCS_ECOSYSED_FOLIAGE_PAGE);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_RULES_VNS3, 0, 3);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_CMAP_VNS3, 0, 4);
	// foliage page resources
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE1_FORESTRY, WCS_ECOSYSED_FOLIAGE_PANELSET, 0);
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
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE2_FORESTRY, WCS_ECOSYSED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE3_FORESTRY, WCS_ECOSYSED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP_FORESTRY, WCS_ECOSYSED_FOLIAGE_PANELSET, 3);
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
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE2, WCS_ECOSYSED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE3, WCS_ECOSYSED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP, WCS_ECOSYSED_FOLIAGE_PANELSET, 3);
		} // else
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGE, WCS_ECOSYSED_FOLIAGE_PANELSET, 4);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_3DOBJECT, WCS_ECOSYSED_FOLIAGE_PANELSET, 5);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPETAB, WCS_ECOSYSED_FOLIAGE_ECOTYPESET, 0);
	// end foliage resources

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_ECOSYSTEMGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		// foliage page setup
		FolGUI->EcotypeTabSetup();
		FolGUI->FillUnitDrops();
		FolGUI->BuildImageList(false);
		FolGUI->BuildObjectList(false);
		// Material GUI
		MatGUI->Construct(WCS_ECOSYSED_MATGRADSET, WCS_ECOSYSED_MATGRADSET + 1);
		ShowPanels();
		ConfigureWidgets();
		UpdateWinCaptionBarState(WCS_FENETRE_WINSTATE_SHOWADV); // to synchronize Advanced state
		} // if
	} // if
 
return (NativeWin);

} // EcosystemEditGUI::Construct

/*===========================================================================*/

EcosystemEditGUI::EcosystemEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ImageLib *ImageSource, EcosystemEffect *ActiveSource)
: GUIFenetre('ECEF', this, "Ecosystem Editor"), Backup(WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, 0xff), 
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
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

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
ImageHost = ImageSource;
Active = ActiveSource;
ActiveGrad = NULL;
// Material GUI
MatGUI = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Ecosystem Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	// foliage page initialization
	if (! (FolGUI = new PortableFoliageGUI(this, EffectsSource, ImageSource, &Active->EcoMat, NULL,
		WCS_ECOSYSED_FOLIAGE_PAGE, WCS_ECOSYSED_FOLIAGE_PANELSET, WCS_ECOSYSED_FOLIAGE_ECOTYPESET, ActivePage)))
		ConstructError = 1;
	// Material GUI
	if (MatGUI = new PortableMaterialGUI(0, this, EffectsSource, Active, &Active->EcoMat, WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER, WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER)) // init ordinal 0
		{
		PopDropMaterialNotifier.Host = this; // to be able to call notifications later
		MatGUI->SetNotifyFunctor(&PopDropMaterialNotifier);
		} // if
	else
		{
		ConstructError = 1;	
		}
	GlobalApp->AppDB->GetDEMElevRange(MaxTerrainElev, MinTerrainElev);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // EcosystemEditGUI::EcosystemEditGUI

/*===========================================================================*/

EcosystemEditGUI::~EcosystemEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

// Material GUI
if (MatGUI)
	delete MatGUI;

// foliage page destructor
if (FolGUI)
	delete FolGUI;
	
} // EcosystemEditGUI::~EcosystemEditGUI()

/*===========================================================================*/

long EcosystemEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_ECG, 0);

return(0);

} // EcosystemEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long EcosystemEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{

DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);

} // EcosystemEditGUI::HandleShowAdvanced

/*===========================================================================*/

long EcosystemEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_ECG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveGrad && ActiveGrad->GetThing())
			((MaterialEffect *)ActiveGrad->GetThing())->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_EDITPROFILE:
		{
		Active->ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
	case IDC_FREEQUERY:
		{
		//FreeQuery();
		break;
		} // IDC_FREEQUERY
	case IDC_EDITQUERY:
		{
		//if (Active->Search)
		//	Active->Search->EditRAHost();
		break;
		} // IDC_EDITQUERY
	// material GUI
	case IDC_POPDROP0:
		{
		if (WidgetGetCheck(IDC_POPDROP0))
			{
			ShowMaterialPopDrop(true);			} // if
		else
			{
			ShowMaterialPopDrop(false);
			} // else
		break;
		} // IDC_POPDROP0
	default:
		break;
	} // ButtonID

// foliage page
FolGUI->HandleButtonClick(Handle, NW, ButtonID);
// Material GUI
MatGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // EcosystemEditGUI::HandleButtonClick

/*===========================================================================*/

long EcosystemEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

// foliage page
FolGUI->HandleButtonDoubleClick(Handle, NW, ButtonID);

return(0);

} // EcosystemEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long EcosystemEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// foliage page
FolGUI->HandleCBChange(Handle, NW, CtrlID);
// Material GUI
MatGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // EcosystemEditGUI::HandleCBChange

/*===========================================================================*/

long EcosystemEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListDelItem(Handle, NW, CtrlID, ItemData);

return(0);

} // EcosystemEditGUI::HandleListDelItem

/*===========================================================================*/

long EcosystemEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListCopyItem(Handle, NW, CtrlID, ItemData);

return(0);

} // EcosystemEditGUI::HandleListCopyItem

/*===========================================================================*/

long EcosystemEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListPasteItem(Handle, NW, CtrlID, ItemData);

return(0);

} // EcosystemEditGUI::HandleListPasteItem

/*===========================================================================*/

long EcosystemEditGUI::HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{

// foliage page
FolGUI->HandleTreeMenuSelect(Handle, NW, CtrlID, TreeItem, RAH, ActionText, Derived);

return(0);

} // EcosystemEditGUI::HandleTreeMenuSelect

/*===========================================================================*/

long EcosystemEditGUI::HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData)
{

// foliage page
FolGUI->HandleTreeChange(Handle, NW, CtrlID, OldTreeItem, NewTreeItem, TreeItemData);

return (0);

} // EcosystemEditGUI::HandleTreeChange

/*===========================================================================*/

long EcosystemEditGUI::HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand)
{

// foliage page
FolGUI->HandleTreeExpand(Handle, NW, CtrlID, TreeItem, TreeItemData, Pre, Expand);

return (0);

} // EcosystemEditGUI::HandleTreeExpand

/*===========================================================================*/

long EcosystemEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

// foliage page
FolGUI->HandleStringLoseFocus(Handle, NW, CtrlID);
// Material GUI
MatGUI->HandleStringLoseFocus(Handle, NW, CtrlID);

return (0);

} // EcosystemEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long EcosystemEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// Material GUI
ShowMaterialPopDrop(false);

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		ActivePage = NewPageID;
		FolGUI->SetActivePage(ActivePage);
		break;
		} // 
	default:
		break;
	} // switch

// foliage page
FolGUI->HandlePageChange(Handle, NW, CtrlID, NewPageID);
ShowPanels();

return(0);

} // EcosystemEditGUI::HandlePageChange

/*===========================================================================*/

void EcosystemEditGUI::ShowPanels(void)
{

ShowPanel(0, ActivePage);
// foliage page
if (ActivePage == WCS_ECOSYSED_FOLIAGE_PAGE)
	FolGUI->ShowPanels();
else
	FolGUI->HidePanels();
	
} // EcosystemEditGUI::ShowPanels

/*===========================================================================*/

long EcosystemEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKGRADFILL:
	case IDC_CHECKTRANSPARENT:
	case IDC_CHECKMATCHECO:
	case IDC_CHECKMATCHRANGE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

// foliage page
FolGUI->HandleSCChange(Handle, NW, CtrlID);

return(0);

} // EcosystemEditGUI::HandleSCChange

/*===========================================================================*/

long EcosystemEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
// foliage page
FolGUI->HandleSRChange(Handle, NW, CtrlID);

return(0);

} // EcosystemEditGUI::HandleSRChange

/*===========================================================================*/

long EcosystemEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_MATCHRED:
	case IDC_MATCHGREEN:
	case IDC_MATCHBLUE:
	case IDC_MATCHRED2:
	case IDC_MATCHGREEN2:
	case IDC_MATCHBLUE2:
		{
		// these float ints aren't really configured to anim colortimes but that's what everyone ese is listening for
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

// foliage page
FolGUI->HandleFIChange(Handle, NW, CtrlID);
// Material GUI
MatGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // EcosystemEditGUI::HandleFIChange

/*===========================================================================*/

void EcosystemEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
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
	if ((((Changed & 0x000000ff) != WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED)
		&& ((Changed & 0x000000ff) != WCS_NOTIFYCOMP_OBJECT_CHANGED))
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

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	DisableWidgets();
	// foliage page
	FolGUI->SyncFoliageWidgets();
	FolGUI->DisableFoliageWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), Active->EcoMat.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureMaterial();
	ConfigureColors();
	DisableWidgets();
	// foliage page
	FolGUI->DisableFoliageWidgets();
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// advanced
	DisplayAdvancedFeatures();
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

if (! Done)
	ConfigureWidgets();

} // EcosystemEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void EcosystemEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE);

/*
if (EffectsHost->EcosystemBase.AreThereEdges((GeneralEffect *)EffectsHost->Ecosystem))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->EcosystemBase.AreThereGradients((GeneralEffect *)EffectsHost->Ecosystem))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
/*ConfigureFI(NativeWin, IDC_RESOLUTION,
 &EffectsHost->EcosystemBase.Resolution,
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

sprintf(TextStr, "Ecosystem Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKHIRESEDGE, &Active->HiResEdge, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKOVERLAP, &EffectsHost->EcosystemBase.OverlapOK, SCFlag_Short, NULL, 0);
//ConfigureSC(NativeWin, IDC_CHECKFLOATING, &EffectsHost->EcosystemBase.Floating, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTRANSPARENT, &Active->Transparent, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPLOWSNOW, &Active->PlowSnow, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKMATCHECO, &Active->CmapMatch, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKMATCHRANGE, &Active->CmapMatchRange, SCFlag_Short, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKVECPOLY, &EffectsHost->GroupVecPolyEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM], SCFlag_Char, NULL, 0);

WidgetAGConfig(IDC_ANIMGRADIENT2, &Active->EcoMat);

// Material GUI
ConfigureTB(NativeWin, IDC_POPDROP0, IDI_EXPAND, IDI_CONTRACT);
ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
// foliage page
FolGUI->ConfigureWidgets();
// Material GUI
MatGUI->ConfigureWidgets();

// cmap matching
ConfigureFI(NativeWin, IDC_MATCHRED,
 &Active->MatchColor[0],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);
ConfigureFI(NativeWin, IDC_MATCHGREEN,
 &Active->MatchColor[1],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);
ConfigureFI(NativeWin, IDC_MATCHBLUE,
 &Active->MatchColor[2],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);
ConfigureFI(NativeWin, IDC_MATCHRED2,
 &Active->MatchColor[3],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);
ConfigureFI(NativeWin, IDC_MATCHGREEN2,
 &Active->MatchColor[4],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);
ConfigureFI(NativeWin, IDC_MATCHBLUE2,
 &Active->MatchColor[5],
  1.0,
   0.0,
	255.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);

ConfigureMaterial();
ConfigureColors();
ConfigureRules();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // EcosystemEditGUI::ConfigureWidgets()

/*===========================================================================*/

void EcosystemEditGUI::ConfigureRules(void)
{

WidgetSmartRAHConfig(IDC_ELEVLINE, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE], Active);
WidgetSmartRAHConfig(IDC_ELEVSKEW, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW], Active);
WidgetSmartRAHConfig(IDC_SKEWAZIMUTH, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ], Active);
WidgetSmartRAHConfig(IDC_RELELEFFECT, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL], Active);
WidgetSmartRAHConfig(IDC_MAXRELEL, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL], Active);
WidgetSmartRAHConfig(IDC_MINRELEL, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL], Active);
WidgetSmartRAHConfig(IDC_MAXSLOPE, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE], Active);
WidgetSmartRAHConfig(IDC_MINSLOPE, &Active->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE], Active);

} // EcosystemEditGUI::ConfigureRules

/*===========================================================================*/

void EcosystemEditGUI::ConfigureMaterial(void)
{
MaterialEffect *Mat;
char GroupWithMatName[200];

if ((ActiveGrad = Active->EcoMat.GetActiveNode()) && (Mat = (MaterialEffect *)ActiveGrad->GetThing()))
	{
	WidgetSmartRAHConfig(IDC_LUMINOSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY], Mat);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULARITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY], Mat);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP], Mat);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY], Mat);
	WidgetSmartRAHConfig(IDC_INTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, &Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY], Mat);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)Mat->GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP), Mat);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)&Mat->Strata, Mat);
	WidgetSmartRAHConfig(IDC_FOLIAGE1, (RasterAnimHost **)&Mat->EcoFol[0], Mat);
	WidgetSmartRAHConfig(IDC_FOLIAGE2, (RasterAnimHost **)&Mat->EcoFol[1], Mat);

	sprintf(GroupWithMatName, "Selected Material (%s)", Mat->Name);
	WidgetSetText(IDC_MATERIALS, GroupWithMatName);
	} // if
else
	{
	// configure everything to NULL
	WidgetSmartRAHConfig(IDC_LUMINOSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_TRANSPARENCY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULARITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SPECULAREXP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_REFLECTIVITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMPINTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_BUMP, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STRATA, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGE1, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FOLIAGE2, (RasterAnimHost **)NULL, NULL);

	WidgetSetText(IDC_MATERIALS, "Selected Material");
	} // else
// foliage page
FolGUI->ConfigureAllFoliage();

// Material GUI
MatGUI->ConfigureMaterial();

} // EcosystemEditGUI::ConfigureMaterial

/*===========================================================================*/

void EcosystemEditGUI::ConfigureColors(void)
{
unsigned char Red, Grn, Blu;

Red = (unsigned char)(Active->MatchColor[0]);
Grn = (unsigned char)(Active->MatchColor[1]);
Blu = (unsigned char)(Active->MatchColor[2]);
SetColorPot(0, Red, Grn, Blu, 1);
ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);

Red = (unsigned char)(Active->MatchColor[3]);
Grn = (unsigned char)(Active->MatchColor[4]);
Blu = (unsigned char)(Active->MatchColor[5]);
SetColorPot(1, Red, Grn, Blu, 1);
ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);

WidgetFISync(IDC_MATCHRED, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MATCHGREEN, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MATCHBLUE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MATCHRED2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MATCHGREEN2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MATCHBLUE2, WP_FISYNC_NONOTIFY);

// this is harmless to call even if there is no active gradient node, it will cause 
// a valid node to be set if there is one..
WidgetAGSync(IDC_ANIMGRADIENT2);
// Material GUI
MatGUI->SyncWidgets();
ActiveGrad = Active->EcoMat.GetActiveNode();

} // EcosystemEditGUI::ConfigureColors

/*===========================================================================*/

void EcosystemEditGUI::SyncWidgets(void)
{

if (Active->EcoMat.GetActiveNode() != ActiveGrad)
	{
	ConfigureWidgets();
	return;
	} // if
	
/*
char TextStr[32];

if (EffectsHost->EcosystemBase.AreThereEdges((GeneralEffect *)EffectsHost->Ecosystem))
	strcpy(TextStr, "Hi-res Edges Exist");
else
	strcpy(TextStr, "No Hi-res Edges");
WidgetSetText(IDC_EDGESEXIST, TextStr);

if (EffectsHost->EcosystemBase.AreThereGradients((GeneralEffect *)EffectsHost->Ecosystem))
	strcpy(TextStr, "Profiles Exist");
else
	strcpy(TextStr, "No Profiles");
WidgetSetText(IDC_GRADIENTSEXIST, TextStr);
*/
//WidgetFISync(IDC_RESOLUTION, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_PRIORITY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVLINE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEVSKEW, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SKEWAZIMUTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_RELELEFFECT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXRELEL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINRELEL, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAXSLOPE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MINSLOPE, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKECOTYPEENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTRANSPARENT, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKMATCHECO, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKMATCHRANGE, WP_SCSYNC_NONOTIFY);

if (ActiveGrad = Active->EcoMat.GetActiveNode())
	{
	WidgetSNSync(IDC_LUMINOSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_TRANSPARENCY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULARITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SPECULAREXP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_REFLECTIVITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMPINTENSITY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_BUMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_DIFFUSECOLOR, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_STRATA, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOLIAGE1, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_FOLIAGE2, WP_FISYNC_NONOTIFY);
	} // if

// Material GUI
MatGUI->SyncWidgets();

} // EcosystemEditGUI::SyncWidgets

/*===========================================================================*/

void EcosystemEditGUI::DisableWidgets(void)
{

// eco base
WidgetSetDisabled(IDC_COMPAREDROP, ! EffectsHost->EcosystemBase.OverlapOK);

// color match
WidgetSetDisabled(IDC_MATCHRED, ! Active->CmapMatch);
WidgetSetDisabled(IDC_MATCHGREEN, ! Active->CmapMatch);
WidgetSetDisabled(IDC_MATCHBLUE, ! Active->CmapMatch);
WidgetSetDisabled(IDC_MATCHRED2, ! (Active->CmapMatch && Active->CmapMatchRange));
WidgetSetDisabled(IDC_MATCHGREEN2, ! (Active->CmapMatch && Active->CmapMatchRange));
WidgetSetDisabled(IDC_MATCHBLUE2, ! (Active->CmapMatch && Active->CmapMatchRange));
WidgetSetDisabled(IDC_CHECKMATCHRANGE, ! Active->CmapMatch);

WidgetSetDisabled(IDC_EDITPROFILE, ! Active->UseGradient);

WidgetSetText(IDC_MINTEXT, Active->CmapMatch && Active->CmapMatchRange ? "Minimum": "Values");
WidgetSetDisabled(IDC_MAXTEXT, ! (Active->CmapMatch && Active->CmapMatchRange));

// Material GUI
MatGUI->DisableWidgets();

} // EcosystemEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void EcosystemEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced || Active->CmapMatch)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_MATCHRED, true);
	WidgetShow(IDC_MATCHGREEN, true);
	WidgetShow(IDC_MATCHBLUE, true);
	WidgetShow(IDC_MATCHRED2, true);
	WidgetShow(IDC_MATCHGREEN2, true);
	WidgetShow(IDC_MATCHBLUE2, true);
	WidgetShow(IDC_CHECKMATCHRANGE, true);
	WidgetShow(IDC_MINTEXT, true);
	WidgetShow(IDC_MAXTEXT, true);
	WidgetShow(IDC_MATCHCOLORTXT, true);
	WidgetShow(IDC_RTXT, true);
	WidgetShow(IDC_GTXT, true);
	WidgetShow(IDC_BTXT, true);
	WidgetShow(ID_COLORPOT1, true);
	WidgetShow(ID_COLORPOT2, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_MATCHRED, false);
	WidgetShow(IDC_MATCHGREEN, false);
	WidgetShow(IDC_MATCHBLUE, false);
	WidgetShow(IDC_MATCHRED2, false);
	WidgetShow(IDC_MATCHGREEN2, false);
	WidgetShow(IDC_MATCHBLUE2, false);
	WidgetShow(IDC_CHECKMATCHRANGE, false);
	WidgetShow(IDC_MINTEXT, false);
	WidgetShow(IDC_MAXTEXT, false);
	WidgetShow(IDC_MATCHCOLORTXT, false);
	WidgetShow(IDC_RTXT, false);
	WidgetShow(IDC_GTXT, false);
	WidgetShow(IDC_BTXT, false);
	WidgetShow(ID_COLORPOT1, false);
	WidgetShow(ID_COLORPOT2, false);
	} // else

// Rules of nature are displayed if the Ecosystem is in an Environment or CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced || EffectsHost->FindEcoInEnvironment(Active))
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_ELEVLINE, true);
	WidgetShow(IDC_ELEVSKEW, true);
	WidgetShow(IDC_SKEWAZIMUTH, true);
	WidgetShow(IDC_RELELEFFECT, true);
	WidgetShow(IDC_MAXRELEL, true);
	WidgetShow(IDC_MINRELEL, true);
	WidgetShow(IDC_MAXSLOPE, true);
	WidgetShow(IDC_MINSLOPE, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_ELEVLINE, false);
	WidgetShow(IDC_ELEVSKEW, false);
	WidgetShow(IDC_SKEWAZIMUTH, false);
	WidgetShow(IDC_RELELEFFECT, false);
	WidgetShow(IDC_MAXRELEL, false);
	WidgetShow(IDC_MINRELEL, false);
	WidgetShow(IDC_MAXSLOPE, true);
	WidgetShow(IDC_MINSLOPE, true);
	} // else
	
// All the material properties are displayed if CompositeDisplayAdvanced is checked.
if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_SPECULARITY, true);
	WidgetShow(IDC_SPECULAREXP, true);
	WidgetShow(IDC_REFLECTIVITY, true);
	WidgetShow(IDC_TRANSPARENCY, true);
	WidgetShow(IDC_LUMINOSITY, true);
	WidgetShow(IDC_ANIMGRADIENT2, ! MatGUI->QueryIsDisplayed());
	// Material GUI
	WidgetShow(IDC_POPDROP0, true);
	} 
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_SPECULARITY, false);
	WidgetShow(IDC_SPECULAREXP, false);
	WidgetShow(IDC_REFLECTIVITY, false);
	WidgetShow(IDC_TRANSPARENCY, false);
	WidgetShow(IDC_LUMINOSITY, false);
	WidgetShow(IDC_ANIMGRADIENT2, false);
	// Material GUI
	WidgetShow(IDC_POPDROP0, false);
	ShowMaterialPopDrop(false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // EcosystemEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void EcosystemEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // EcosystemEditGUI::Cancel

/*===========================================================================*/

void EcosystemEditGUI::Name(void)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // EcosystemEditGUI::Name()

/*===========================================================================*/

// Material GUI
void EcosystemEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial(void)
{

if (Host) Host->ConfigureMaterial();

} // EcosystemEditGUIPortableMaterialGUINotifyFunctor::HandleConfigureMaterial

/*===========================================================================*/

// Material GUI
void EcosystemEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad(GradientCritter *NewNode)
{

if (Host) Host->SetNewActiveGrad(NewNode);

} // EcosystemEditGUIPortableMaterialGUINotifyFunctor::HandleNewActiveGrad

/*===========================================================================*/

void EcosystemEditGUI::SetToFoliagePage(void)
{

ActivePage = WCS_ECOSYSED_FOLIAGE_PAGE;
FolGUI->SetActivePage(ActivePage);
WidgetTCSetCurSel(IDC_TAB1, ActivePage);
// Material GUI
ShowMaterialPopDrop(false);
ShowPanels();

} // EcosystemEditGUI::SetToFoliagePage

/*===========================================================================*/

void EcosystemEditGUI::ActivateFoliageItem(RasterAnimHost *ActivateMe)
{

FolGUI->EditFoliageItem(ActivateMe, 0);

} // EcosystemEditGUI::ActivateFoliageItem

/*===========================================================================*/

// material GUI
void EcosystemEditGUI::ShowMaterialPopDrop(bool ShowState)
{

if (ShowState)
	{
	// position and show
	if (MatGUI)
		{
		ShowPanelAsPopDrop(IDC_POPDROP0, MatGUI->GetPanel(), 0, SubPanels[0][1]);
		WidgetShow(IDC_ANIMGRADIENT2, 0); // hide master gradient widget since it looks weird otherwise
		WidgetSetCheck(IDC_POPDROP0, true);
		} // if
	} // if
else
	{
	if (MatGUI)
		{
		ShowPanel(MatGUI->GetPanel(), -1); // hide
		WidgetShow(IDC_ANIMGRADIENT2, QueryDisplayAdvancedUIVisibleState() ? true : false); // show master gradient widget
		WidgetSetCheck(IDC_POPDROP0, false);
		} // if
	} // else

} // EcosystemEditGUI::ShowMaterialPopDrop

/*===========================================================================*/

bool EcosystemEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->EcoMat.CountNodes() > 1 ? true : false);

} // EcosystemEditGUI::QueryLocalDisplayAdvancedUIVisibleState
