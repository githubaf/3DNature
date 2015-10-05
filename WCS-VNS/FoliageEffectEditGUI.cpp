// FoliageEffectEditGUI.cpp
// Code for FoliageEffect editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "FoliageEffectEditGUI.h"
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
#include "Ecotype.h"
#include "resource.h"
#include "Lists.h"
#include "PortableFoliageGUI.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_DEMO
extern int ForestryDemoWarned;
#endif // WCS_BUILD_DEMO

char *FoliageEffectEditGUI::TabNames[WCS_FOLIAGEEFFECTGUI_NUMTABS] = {"General", "Foliage"};
long FoliageEffectEditGUI::ActivePage;

// foliage page defines
#define WCS_FOLEFFECTED_FOLIAGE_PAGE	1
#define WCS_FOLEFFECTED_FOLIAGE_PANELSET	1
#define WCS_FOLEFFECTED_FOLIAGE_ECOTYPESET	2

NativeGUIWin FoliageEffectEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // FoliageEffectEditGUI::Open

/*===========================================================================*/

NativeGUIWin FoliageEffectEditGUI::Construct(void)
{
Raster *MyRast;
GeneralEffect *MyEffect;
int TabIndex;
#ifdef WCS_FORESTRY_WIZARD
int InhibitForestry = 0;
#endif // WCS_FORESTRY_WIZARD

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_GENERAL_VNS3, 0, 0);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_VNS3, 0, WCS_FOLEFFECTED_FOLIAGE_PAGE);
	// foliage page resources
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPE1_FORESTRY, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 0);
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
		CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_FOLIAGE_ECOTYPE2_FORESTRY, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_FOLIAGE_ECOTYPE3_FORESTRY, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP_FORESTRY, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 3);
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
		CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_FOLIAGE_ECOTYPE2, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 1);
		CreateSubWinFromTemplate(IDD_FOLIAGEEFFECT_FOLIAGE_ECOTYPE3, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 2);
		CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGEGROUP, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 3);
		} // else
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_FOLIAGE, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 4);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_3DOBJECT, WCS_FOLEFFECTED_FOLIAGE_PANELSET, 5);
	CreateSubWinFromTemplate(IDD_ECOSYSTEM_FOLIAGE_ECOTYPETAB, WCS_FOLEFFECTED_FOLIAGE_ECOTYPESET, 0);
	// end foliage resources

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_FOLIAGEEFFECTGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
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
		// foliage page setup
		FolGUI->EcotypeTabSetup();
		FolGUI->FillUnitDrops();
		FolGUI->BuildImageList(false);
		FolGUI->BuildObjectList(false);
		ShowPanels();
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // FoliageEffectEditGUI::Construct

/*===========================================================================*/

FoliageEffectEditGUI::FoliageEffectEditGUI(EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource, FoliageEffect *ActiveSource)
: GUIFenetre('EDFO', this, "Foliage Effect Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {0,
								0,
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
			/* foliage page */	MAKE_ID(0xff, WCS_SUBCLASS_ECOTYPE, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
			/* foliage page */	MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
			/* foliage page */	MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
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
DBHost = DBSource;
Active = ActiveSource;

if (ImageSource && EffectsHost && ActiveSource)
	{
	AllEvents[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, 0xff);
	AllEvents[1] = MAKE_ID(0xff, Active->Ecotp.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	sprintf(NameStr, "FoliageEffect Editor - %s ", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	// foliage page initialization
	if (! (FolGUI = new PortableFoliageGUI(this, EffectsSource, ImageSource, NULL, &Active->Ecotp,
		WCS_FOLEFFECTED_FOLIAGE_PAGE, WCS_FOLEFFECTED_FOLIAGE_PANELSET, WCS_FOLEFFECTED_FOLIAGE_ECOTYPESET, ActivePage)))
		ConstructError = 1;
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // FoliageEffectEditGUI::FoliageEffectEditGUI

/*===========================================================================*/

FoliageEffectEditGUI::~FoliageEffectEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
// foliage page destructor
if (FolGUI)
	delete FolGUI;

} // FoliageEffectEditGUI::~FoliageEffectEditGUI()

/*===========================================================================*/

long FoliageEffectEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_FLG, 0);

return(0);

} // FoliageEffectEditGUI::HandleCloseWin

/*===========================================================================*/

long FoliageEffectEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_FLG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	default:
		break;
	} // ButtonID

// foliage page
FolGUI->HandleButtonClick(Handle, NW, ButtonID);

return(0);

} // FoliageEffectEditGUI::HandleButtonClick

/*===========================================================================*/

long FoliageEffectEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

// foliage page
FolGUI->HandleButtonDoubleClick(Handle, NW, ButtonID);

return(0);

} // FoliageEffectEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long FoliageEffectEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(016, 16);

// foliage page
FolGUI->HandleCBChange(Handle, NW, CtrlID);

return (0);

} // FoliageEffectEditGUI::HandleCBChange

/*===========================================================================*/

long FoliageEffectEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListDelItem(Handle, NW, CtrlID, ItemData);

return(0);

} // FoliageEffectEditGUI::HandleListDelItem

/*===========================================================================*/

long FoliageEffectEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListCopyItem(Handle, NW, CtrlID, ItemData);

return(0);

} // FoliageEffectEditGUI::HandleListCopyItem

/*===========================================================================*/

long FoliageEffectEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

// foliage page
FolGUI->HandleListPasteItem(Handle, NW, CtrlID, ItemData);

return(0);

} // FoliageEffectEditGUI::HandleListPasteItem

/*===========================================================================*/

long FoliageEffectEditGUI::HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{

// foliage page
FolGUI->HandleTreeMenuSelect(Handle, NW, CtrlID, TreeItem, RAH, ActionText, Derived);

return(0);
} // FoliageEffectEditGUI::HandleTreeMenuSelect

/*===========================================================================*/

long FoliageEffectEditGUI::HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData)
{

// foliage page
FolGUI->HandleTreeChange(Handle, NW, CtrlID, OldTreeItem, NewTreeItem, TreeItemData);

return (0);

} // FoliageEffectEditGUI::HandleTreeChange

/*===========================================================================*/

long FoliageEffectEditGUI::HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand)
{

// foliage page
FolGUI->HandleTreeExpand(Handle, NW, CtrlID, TreeItem, TreeItemData, Pre, Expand);

return (0);

} // FoliageEffectEditGUI::HandleTreeExpand

/*===========================================================================*/

long FoliageEffectEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

return (0);

} // FoliageEffectEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long FoliageEffectEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		ActivePage = NewPageID;
		FolGUI->SetActivePage(ActivePage);
		break;
		}
	default:
		break;
	} // switch

// foliage page
FolGUI->HandlePageChange(Handle, NW, CtrlID, NewPageID);
ShowPanels();

return(0);

} // FoliageEffectEditGUI::HandlePageChange

/*===========================================================================*/

void FoliageEffectEditGUI::ShowPanels(void)
{

ShowPanel(0, ActivePage);
// foliage page
if (ActivePage == WCS_FOLEFFECTED_FOLIAGE_PAGE)
	FolGUI->ShowPanels();
else
	FolGUI->HidePanels();
	
} // FoliageEffectEditGUI::ShowPanels

/*===========================================================================*/

long FoliageEffectEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKPREVIEWENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // IDC_CHECKPREVIEWENABLED
	default:
		break;
	} // switch CtrlID

// foliage page
FolGUI->HandleSCChange(Handle, NW, CtrlID);

return(0);

} // FoliageEffectEditGUI::HandleSCChange

/*===========================================================================*/

long FoliageEffectEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOABSOLUTE:
	case IDC_RADIORELATIVEGRND:
	case IDC_RADIORELATIVEJOE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

// foliage page
FolGUI->HandleSRChange(Handle, NW, CtrlID);

return(0);

} // FoliageEffectEditGUI::HandleSRChange

/*===========================================================================*/

long FoliageEffectEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// foliage page
FolGUI->HandleFIChange(Handle, NW, CtrlID);

return(0);

} // FoliageEffectEditGUI::HandleFIChange

/*===========================================================================*/

void FoliageEffectEditGUI::HandleNotifyEvent(void)
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
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	DisableWidgets();
	// foliage page
	FolGUI->DisableFoliageWidgets();
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

} // FoliageEffectEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void FoliageEffectEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

sprintf(TextStr, "Foliage Effect Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPREVIEWENABLED, &Active->PreviewEnabled, SCFlag_Char, NULL, 0);
WidgetSmartRAHConfig(IDC_ELEV, &Active->AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV], Active);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIOABSOLUTE, &Active->Absolute, SRFlag_Short, WCS_EFFECT_ABSOLUTE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEGRND, &Active->Absolute, SRFlag_Short, WCS_EFFECT_RELATIVETOGROUND, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOABSOLUTE, IDC_RADIORELATIVEJOE, &Active->Absolute, SRFlag_Short, WCS_EFFECT_RELATIVETOJOE, NULL, NULL);

// foliage page
FolGUI->ConfigureWidgets();

DisableWidgets();
// foliage page
FolGUI->ConfigureAllFoliage();

} // FoliageEffectEditGUI::ConfigureWidgets()

/*===========================================================================*/

void FoliageEffectEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKPREVIEWENABLED, WP_SCSYNC_NONOTIFY);

WidgetSNSync(IDC_ELEV, WP_FISYNC_NONOTIFY);

} // FoliageEffectEditGUI::SyncWidgets

/*===========================================================================*/

void FoliageEffectEditGUI::DisableWidgets(void)
{

} // FoliageEffectEditGUI::DisableWidgets

/*===========================================================================*/

void FoliageEffectEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // FoliageEffectEditGUI::Cancel

/*===========================================================================*/

void FoliageEffectEditGUI::Name(void)
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

} // FoliageEffectEditGUI::Name()

/*===========================================================================*/

void FoliageEffectEditGUI::SetToFoliagePage(void)
{

ActivePage = WCS_FOLEFFECTED_FOLIAGE_PAGE;
FolGUI->SetActivePage(ActivePage);
WidgetTCSetCurSel(IDC_TAB1, ActivePage);
ShowPanels();

} // FoliageEffectEditGUI::SetToFoliagePage

/*===========================================================================*/

void FoliageEffectEditGUI::ActivateFoliageItem(RasterAnimHost *ActivateMe)
{

FolGUI->EditFoliageItem(ActivateMe, 0);

} // FoliageEffectEditGUI::ActivateFoliageItem

/*===========================================================================*/
