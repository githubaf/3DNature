// DBEditGUI.cpp
// Code for Database editor editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary R. Huber
// Effects list added 5/97 by GRH
// Copyright 1996 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "AppMem.h"
#include "DBEditGUI.h"
#include "Project.h"
#include "ProjectIO.h"
#include "WCSWidgets.h"
#include "ProjectDispatch.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Conservatory.h"
#include "Database.h"
#include "Joe.h"
#include "Toolbar.h"
#include "Useful.h"
#include "DEM.h"
#include "Log.h"
#include "EffectsLib.h"
#include "Interactive.h"
#include "WCSVersion.h"
#include "DEMPaintGUI.h"
#include "Lists.h"
#include "FeatureConfig.h"
#include "resource.h"
#include "VisualStylesXP.h" // for theming detection for toolbar sizing
#include "GridWidget.h"


//#define DBEDIT_BUSYWIN_SPEEDTEST
//#define DBEDIT_BATCHRESAVE

#define NAMEBUFLEN 100
static char NameBuf[NAMEBUFLEN];

bool GIS_Name_Warned;	// need this global
long DBEditGUI::ActivePage;
bool DBEditGUI::ShowPropertiesPanel;

long FAR PASCAL DBGridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
extern WNDPROC ListViewWndProc;
extern unsigned long int ListViewOriginalWndExtra;


/*===========================================================================*/

bool DBEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANLOAD:
	case WCS_FENETRE_WINCAP_CANSAVE:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // DBEditGUI::InquireWindowCapabilities

/*===========================================================================*/

char *DBEditGUI::InquireWindowCapabilityCaption(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANLOAD:
		return("Load Database or .ELEV DEMs (+SHIFT to Append Database)");
	case WCS_FENETRE_WINCAP_CANSAVE:
		return("Save Database (+SHIFT to Export)");
	default:
		return(NULL);
	} // AskAbout
//lint -restore

} // DBEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin DBEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

/*
short X, Y, W, H;

Proj->InquireWindowCoords(FenID, X, Y, W, H);
SetWindowPos(NativeWin, NULL, 0, 0, W, H, SWP_NOMOVE | SWP_NOZORDER); // resize
*/
if (Success = GUIFenetre::Open(Moi))
	{
	// call HandlePageChange/HandleResized to adjust initial widget placement (after Open has setup initial size)
	HandlePageChange(NULL, NativeWin, IDC_TAB1, ActivePage);
	} // if

return (Success);

} // DBEditGUI::Open

/*===========================================================================*/

NativeGUIWin DBEditGUI::Construct(void)
{

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DB_EDIT1, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_DB_EDIT_PROPERTIES, 0, 0, true);
	CreateSubWinFromTemplate(IDD_DB_EDIT_EXTENTS_VEC, 0, 1, true);
	CreateSubWinFromTemplate(IDD_DB_EDIT_EXTENTS_DEM, 0, 2, true);
	CreateSubWinFromTemplate(IDD_DB_EDIT_EFFECT, 0, 3, true);
	CreateSubWinFromTemplate(IDD_DB_EDIT_LAYER, 0, 4, true);

	if (NativeWin)
		{
		// add top toolbar
		CreateTopToolbar(NativeWin, LocalWinSys()->Instance());
		// Set up Widget bindings

		WidgetTCInsertItem(IDC_TAB1, 0, "Properties");
		WidgetTCInsertItem(IDC_TAB1, 1, "Extent");
		WidgetTCInsertItem(IDC_TAB1, 2, "Comp");
		WidgetTCInsertItem(IDC_TAB1, 3, "Layer");
		#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
		WidgetTCInsertItem(IDC_TAB1, 4, "Attrib");
		#endif // WCS_SUPPORT_GENERIC_ATTRIBS
		WidgetTCInsertItem(IDC_TAB1, 5, "All");
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);

		PrepGridWidgetForJoeUsage(GetWidgetFromID(IDC_LIST1));
		GridWidgetInstance GWI;
		
		GWI.GWStyles = WCSW_GW_STYLE_JOE | WCSW_GW_STYLE_GREENBAR;
		GWI.GridWidgetCache = &GridWidgetCache;
		GWI.DBHost = Host;
		GWI.DBEHost = this;
		GWI.HostLib = HostLib;
		GWI.ProjHost = ProjHost;
		WidgetGWConfig(IDC_LIST1, &GWI);

		BuildList();

		ConfigureWidgets();
		UpdateTitle();
		} // if

	} // if
 
return(NativeWin);

} // DBEditGUI::Construct

/*===========================================================================*/

static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 0};

DBEditGUI::DBEditGUI(Database *DB, Project *ProjSource, EffectsLib *LibSource)
: GUIFenetre('DBED', this, "Database") // Yes, I know...
{
static NotifyTag AllAppExEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_FREEZE, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_WAVE, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_CLOUD, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_ILLUMINATION, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_RASTERTA, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_FOLIAGE, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_SHADOW, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_STREAM, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_GROUND, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_SNOW, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_FENCE, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
									MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff), // for embedded AnimColorTime
									MAKE_ID(0xff, WCS_SUBCLASS_LAYER, 0xff, 0xff), // Layer operations
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_JOE;
ConstructError = 0;
ImBusy = 0;
ButtonCount = 0;
hwndTB = NULL;

Host = DB;
HostLib = LibSource;
ProjHost = ProjSource;

NoOfObjects =
LayerName[0] = 0;
Frozen = 0;
DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

GridWidgetColumnsPrepped = false;

Host->RegisterClient(this, AllDBEvents);
GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
GlobalApp->AppEx->RegisterClient(this, AllAppExEvents);

VectorColor.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

ActiveColorEditItem = 0;

#ifdef WCS_BUILD_VNS
QueryFilter = NULL;
#endif // WCS_BUILD_VNS

SetWinManFlags(WCS_FENETRE_WINMAN_GUIFENSIZE);

} // DBEditGUI::DBEditGUI

/*===========================================================================*/

DBEditGUI::~DBEditGUI()
{

if (GlobalApp->GUIWins->DBO)
	{
	delete GlobalApp->GUIWins->DBO;
	GlobalApp->GUIWins->DBO = NULL;
	} // if
Host->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // DBEditGUI::~DBEditGUI()

/*===========================================================================*/

long DBEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch (CtrlID)
	{
	case IDC_LIST1:
		{
		DoRemove();
		break;
		} // IDC_LIST1
	case IDC_EFFECTLIST:
		{
		RemoveEffect();
		break;
		} // IDC_EFFECTLIST
	case IDC_LAYERLIST:
		{
		DoRemoveLayer();
		break;
		} // IDC_LAYERLIST
	default:
		break;
	} // switch

return(0);

} // DBEditGUI::HandleListDelItem

/*===========================================================================*/

long DBEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DBG, 0);

return(0);

} // DBEditGUI::HandleCloseWin

/*===========================================================================*/

long DBEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
Joe *CurrentJoe;
SECURITY_INLINE_CHECK(013, 13);

switch (ButtonID)
	{
	case IDC_SHOWPROPPANEL:
		{
		ShowPropertiesPanel = WidgetGetCheck(IDC_SHOWPROPPANEL) ? true : false;
		ConfigurePropertiesPanel(); // update panel visibility
		break;
		} // IDC_SHOWPROPPANEL
	case IDC_DBFILTER:
		{
		DoFilterPopup();
		break;
		} // IDC_DBFILTER
	case IDC_SEL1:
		{
		int Qualifier;

		Qualifier = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1:
			LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL) ? 2: 0;
		DoLayerSelect(Qualifier);
		break;
		} // 
	case IDC_ON1:
		{
		DoLayerOn();
		break;
		} // 
	case IDC_OFF1:
		{
		DoLayerOff();
		break;
		} // 
	case IDC_ADDLAYER:
		{
		if (ActivePage == 4)
			DoAddAttr();
		else
			SetLayerName();
		break;
		} // 
	case IDC_REMOVELAYER:
		{
		DoRemoveLayer();
		break;
		} // 
	case IDC_SETATTR:
		{
		DoAttribValue();
		break;
		} // 
	case IDI_DBTB_SEARCH:
		{
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT))
			{
			DoSearch(0);
			} // if
		else
			{
			DoSearch(1);
			} // else
		break;
		} // 
	case IDI_DBTB_REMOVE:
		{
		DoRemove();
		break;
		} // 
	case ID_SAVE:
	case IDC_SAVE:
		{
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1: 0)
			{
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
				WCS_TOOLBAR_ITEM_DBO, 0);
			} // if
		else
			{
			DoSave();
			} // else
		break;
		} // 
	case IDC_LOAD:
		{
		DoAppend();
		break;
		} // 
	case IDC_GALLERY:
		{
		DoLoad(TRUE);
		break;
		} // 
	case IDC_ADDEFFECT:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_EFG, 0);
		break;
		} // 
	case IDI_DBTB_EDIT:
		{
		OpenEditor();
		break;
		} // 
	case IDI_DBTB_CONFORM:
		{
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL) ? 1: 0)
			{
			if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1: 0)
				{
				// Do mega-rebound
				Host->MegaReBoundTree(WCS_DATABASE_STATIC);
				UserMessageOK("Database Editor", "Recomputed Boundaries for all groups.");
				} // if
			else
				{
				Host->ReBoundTree(WCS_DATABASE_STATIC);
				UserMessageOK("Database Editor", "Updated Boundaries for all groups.");
				} // else
			} // if
		else
			{
			ConformVector();
			} // else
		break;
		} // 
	case IDI_DBTB_SPLIT:
		{
		SplitVector();
		break;
		} // 
	case IDI_DBTB_JOIN:
		{
		JoinVectors();
		break;
		} // 
	case IDI_DEMPAINT:
		{
		if (CurrentJoe = GetJoeFromOBN(GetActiveObject()))
			{
			if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
				{ // paint
				if (GlobalApp->GUIWins->DPG && GlobalApp->GUIWins->DPG->GetActive() != CurrentJoe)
					{
					delete GlobalApp->GUIWins->DPG;
					GlobalApp->GUIWins->DPG = NULL;
					}
				if (! GlobalApp->GUIWins->DPG)
					GlobalApp->GUIWins->DPG = new DEMPaintGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->MainProj, CurrentJoe);
				if (GlobalApp->GUIWins->DPG)
					{
					if (GlobalApp->GUIWins->DPG->ConstructError)
						{
						delete GlobalApp->GUIWins->DPG;
						GlobalApp->GUIWins->DPG = NULL;
						} // if
					else
						GlobalApp->GUIWins->DPG->Open(GlobalApp->MainProj);
					}
				} // if
			} // if
		break;
		} // 
	case IDI_DBTB_PROFILE:
		{
		if (CurrentJoe = GetJoeFromOBN(GetActiveObject()))
			{
			if (!CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
				{ // profile
				AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
					WCS_TOOLBAR_ITEM_VPG, 0);
				} // if
			} // if
		break;
		} // 
	case IDC_MODIFYEFFECT:
		{
		ModifyEffect();
		break;
		} // 
	case IDC_REMOVEEFFECT:
		{
		RemoveEffect();
		break;
		} // 
	case IDC_ADDBYLAYER:
		{
		AddByLayer();
		break;
		} // 
	case IDC_ADDBYNAME:
		{
		AddByName();
		break;
		} // 
	default: break;
	} // ButtonID

UpdateTitle();

return(0);

} // DBEditGUI::HandleButtonClick

/*===========================================================================*/

long DBEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

// note that Handle is 0 if called from ConfigureWidgets
switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{ // extent
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), false);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 1;
				break;
				} // 1
			case 2:
				{ // Components
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), false);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 2;
				break;
				} // 2
			case 3:
				{ // layers
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), false);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 3;
				break;
				} // 3
			#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
			case 4:
				{ // Attributes
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), true);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 4;
				break;
				} // 4
			case 5:
			#else // !WCS_SUPPORT_GENERIC_ATTRIBS
			case 4:
			#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
				{ // All
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), true);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 1;
				NewPageID = 0;
				break;
				} // 4 (WCS) / 5 (VNS), All
			default:
				{ // properties
				WidgetShow(IDC_LIST1, 0);
				ShowPropertiesColumns(GetWidgetFromID(IDC_LIST1), true);
				ShowExtentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowComponentColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowLayerColumns(GetWidgetFromID(IDC_LIST1), false);
				ShowAttributeColumns(GetWidgetFromID(IDC_LIST1), false);
				WidgetShow(IDC_LIST1, 1);
				ActivePage = 0;
				NewPageID = 0;
				break;
				} // 0
			} // switch
		ConfigurePropertiesPanel();
		break;
		}
	} // switch

return(0);

} // DBEditGUI::HandlePageChange


void DBEditGUI::ConfigurePropertiesPanel(void)
{

switch (ActivePage)
	{
	case 1:
		{ // extent
		ShowPanel(0, -1);
		break;
		} // 1
	case 2:
		{ // Components
		if(ShowPropertiesPanel) ShowPanel(0, 3);
		else ShowPanel(0, -1);
		break;
		} // 2
	case 3:
		{ // layers
		if(ShowPropertiesPanel) ShowPanel(0, 4);
		else ShowPanel(0, -1);
		ConfigLayerSection(Host->ActiveObj, NoOfObjects);
		break;
		} // 3
	#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
	case 4:
		{ // Attributes
		if(ShowPropertiesPanel) ShowPanel(0, 4);
		else ShowPanel(0, -1);
		ConfigLayerSection(Host->ActiveObj, NoOfObjects);
		break;
		} // 4
	case 5:
	#else // !WCS_SUPPORT_GENERIC_ATTRIBS
	case 4:
	#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
		{ // All
		ShowPanel(0, -1);
		break;
		} // 4 (WCS) / 5 (VNS), All
	default:
		{ // properties
		ShowPanel(0, -1);
		break;
		} // 0
	} // switch
// call HandleResized to adjust widget placement
RECT WindowClient;
GetClientRect(NativeWin, &WindowClient);
HandleReSized(0, WindowClient.right, WindowClient.bottom);

} // DBEditGUI::ConfigurePropertiesPanel

/*===========================================================================*/

long DBEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(049, 49);
switch (CtrlID)
	{
	case IDC_LIST1:
		{
		// activating an item when control clicking to deselect it causes 
		// the item to become active and then reselects it
		if (! LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
			{
			DoActivate();
			} // if
		UpdateTitle();
		break;
		}
	} // switch CtrlID

return (0);

} // DBEditGUI::HandleListSel

/*===========================================================================*/

long DBEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_EFFECTLIST:
		{
		ModifyEffect();
		break;
		}
	} // switch CtrlID

return (0);

} // DBEditGUI::HandleListDoubleClick

/*===========================================================================*/

void DBEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, TestChange, Interested[7];
long OldActive, NewGuy, MatchClass;
char Redraw = 0, Rebuild = 0, ClearList = 0, Reconfig = 0, RebuildEffects = 0, Done = 0, TestLoop;

SECURITY_INLINE_CHECK(032, 32);

if (! NativeWin)
	return;

OldActive = GetActiveObject();

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	Freeze();
	return;
	} // if freeze
Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	UnFreeze();		// does not rebuild all
	PrepGridWidgetForJoeUsage(GetWidgetFromID(IDC_LIST1)); // rebuilds grid widget columns
	BuildList();
	ConfigureWidgets();
	BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);
	HandlePageChange(NULL, NativeWin, IDC_TAB1, ActivePage); // enforce visibility of proper stuff
	return;
	} // if thaw
if (Frozen)
	return;

Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[3] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	MatchClass = (TestChange & 0xff000000) >> 24;
	if (MatchClass > WCS_EFFECTSSUBCLASS_LAKE && MatchClass < WCS_MAXIMPLEMENTED_EFFECTS)
		{
		RebuildEffects = 1;
		Done = 1;
		} // if
	else if (MatchClass == WCS_RAHOST_OBJTYPE_VECTOR || MatchClass == WCS_RAHOST_OBJTYPE_DEM || MatchClass == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		Reconfig = 1;
		Redraw = 1;
		Done = 1;
		} // else if
	} // if thaw
Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[1] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	long MatchSubClass = (TestChange & 0x00ff0000) >> 16;
	MatchClass = (TestChange & 0xff000000) >> 24;
	if (MatchClass == WCS_RAHOST_OBJTYPE_VECTOR || MatchClass == WCS_RAHOST_OBJTYPE_DEM || MatchClass == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		RebuildEffects = 1;
		Done = 1;
		} // if
	else if (MatchSubClass == WCS_SUBCLASS_LAYER) // layer added or removed on Joe
		{
		Redraw = 1;
		Done = 1;
		} // else if
	} // if thaw
Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	long MatchSubClass = (TestChange & 0x00ff0000) >> 16;
	MatchClass = (TestChange & 0xff000000) >> 24;
	if (MatchClass == WCS_RAHOST_OBJTYPE_VECTOR || MatchClass == WCS_RAHOST_OBJTYPE_DEM || MatchClass == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		Rebuild = 1;
		RebuildEffects = 1;
		Done = 1;
		} // if
	else if (MatchSubClass == WCS_SUBCLASS_LAYER) // layer created or deleted
		{
		Rebuild = 1;
		} // else if
	} // if thaw
Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Interested[1] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	MatchClass = (TestChange & 0xff000000) >> 24;
	if (MatchClass > WCS_EFFECTSSUBCLASS_LAKE && MatchClass < WCS_MAXIMPLEMENTED_EFFECTS)
		{
		RebuildEffects = 1;
		Done = 1;
		} // if
	else if (MatchClass == WCS_RAHOST_OBJTYPE_VECTOR || MatchClass == WCS_RAHOST_OBJTYPE_DEM || MatchClass == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		Rebuild = 1;
		RebuildEffects = 1;
		Done = 1;
		} // else if
	} // if thaw
Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	Reconfig = 1;
	Done = 1;
	} // if thaw

Interested[0] = MAKE_ID(WCS_SUBCLASS_ANIMCOLORTIME, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, 0xff);
Interested[1] = NULL;
if (TestChange = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	// was it from our VectorColor?
	if (Activity->ChangeNotify->NotifyData == &VectorColor)
		{
		// color of vector is being changed
		UpdateVecColor((unsigned char)WCS_round(VectorColor.CurValue[0] * 255.0), (unsigned char)WCS_round(VectorColor.CurValue[1] * 255.0), (unsigned char)WCS_round(VectorColor.CurValue[2] * 255.0));
		Redraw = 1;
		} // if
	} // if


if (! Done)
	{
	for(TestLoop = 0; TestChange = Changes[TestLoop]; TestLoop ++)
		{
		if (((TestChange & 0xff000000) >> 24) ==  WCS_NOTIFYCLASS_DBASE)
			{
			switch ((TestChange & 0x00ff0000) >> 16)
				{
				case WCS_NOTIFYDBASE_PRELOAD:
					{
					ImBusy = 1;
					break;
					} // if
				case WCS_NOTIFYDBASE_NEW:
				case WCS_NOTIFYDBASE_LOAD:
					{
					ImBusy = 0;
					ClearList = 1;
					// no break, fall thru
					} // if
					//lint -fallthrough
				case WCS_NOTIFYDBASE_ADDOBJ:
				case WCS_NOTIFYDBASE_DELOBJ:
					{
					Rebuild = 1;
					break;
					} //
				case WCS_NOTIFYDBASE_CHANGEOBJ:
					{
					if (((TestChange & 0x0000ff00) >> 8) == WCS_NOTIFYDBASECHANGE_FLAGS
						&& (TestChange & 0x000000ff) == WCS_NOTIFYDBASECHANGE_FLAGS_SELSTATECLEAR)
						{
						DeselectAll();
						} // if
					else if (((TestChange & 0x0000ff00) >> 8) == WCS_NOTIFYDBASECHANGE_NAME
						|| ((TestChange & 0x0000ff00) >> 8) == WCS_NOTIFYDBASECHANGE_FLAGS
						|| ((TestChange & 0x0000ff00) >> 8) == WCS_NOTIFYDBASECHANGE_DRAWATTRIB
						|| ((TestChange & 0x0000ff00) >> 8) == WCS_NOTIFYDBASECHANGE_POINTS)
						{
						Reconfig = 1;
						Redraw = 1;
						} // if
					break;
					} //
				case WCS_NOTIFYDBASE_CHANGEACTIVE:
					{
					// Need to find OBN of Joe...
					if (!ImBusy) // This message occurs during a DBEditGUI delete operation, so ignore it.
						{
						if ((NewGuy =  GetOBNFromJoe((Joe *)Activity->ChangeNotify->NotifyData)) >= 0)
							{
							SetActiveObject(NewGuy);
							UpdateTitle();
							} // if
						} // if
					break;
					} //
				} // switch
			} // if
		} // for
	} // if

if (ImBusy)
	{
	return;
	} // if

if (ClearList)
	{
	OldActive = 0;
	} // if
if (Rebuild)
	{
	BuildList();
	if (OldActive < NoOfObjects)
		{
		SetActiveObject(OldActive);
		} // if
	else
		{
		SetActiveObject(0);
		} // else
	OldActive = 0;
	} // if
if (Rebuild || Reconfig)
	{
	ConfigureWidgets();
	PrepGridWidgetForJoeUsage(GetWidgetFromID(IDC_LIST1)); // // rebuilds grid widget columns
	HandlePageChange(NULL, NativeWin, IDC_TAB1, ActivePage); // enforce visibility of proper stuff
	} // if
if (Redraw && !(Rebuild))
	{
	ReDrawList();
	} 
if (RebuildEffects)
	{
	BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);
	// configure extents which might have changed when coord sys added or removed
	} // if
/*
Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_EFFECTS, WCS_EFFECTSSUBCLASS_GENERIC, WCS_EFFECTSGENERIC_NAME, 0xff);
Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_EFFECTS, WCS_EFFECTSSUBCLASS_GENERIC, WCS_EFFECTSGENERIC_ENABLED, 0xff);
Interested[2] = MAKE_ID(WCS_NOTIFYCLASS_EFFECTS, WCS_EFFECTSSUBCLASS_GENERIC, WCS_EFFECTSGENERIC_JOESADDED, 0xff);
Interested[3] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);
	} // if effect changed
*/

} // DBEditGUI::HandleNotifyEvent()


/*===========================================================================*/

void DBEditGUI::HardFreeze(void)
{

Freeze();

WidgetShow(IDC_LIST1, FALSE);

} // DBEditGUI::HardFreeze

/*===========================================================================*/

void DBEditGUI::HardUnFreeze(void)
{

UnFreeze();

WidgetShow(IDC_LIST1, TRUE);

// rebuild our display state from scratch
ConfigureWidgets();
ReDrawList();
BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);

} // DBEditGUI::HardUnFreeze

/*===========================================================================*/

static char Str[255];

void DBEditGUI::ConfigureWidgets(void)
{
Joe *CurrentJoe;
char Second = 0;

WidgetTBConfig(IDC_DBFILTER, IDI_DBEFILTER, NULL);
WidgetTBConfig(IDC_SHOWPROPPANEL, IDI_DBESHOWPROPPANEL, NULL);

if (CurrentJoe = GetJoeFromOBN(GetActiveObject()))
	{
	BuildLayerList(Host->ActiveObj, NoOfObjects);
	} // if

ConfigureToolbarButtons();

} // DBEditGUI::ConfigureWidgets()

/*===========================================================================*/

void DBEditGUI::ConfigureToolbarButtons(void)
{

if (Joe *Active = GetJoeFromOBN(GetActiveObject()))
	{
	bool IsDEM = Active->TestFlags(WCS_JOEFLAG_ISDEM) ? true : false;
	SetToolbarButtonDisabled(IDI_DBTB_PROFILE, IsDEM);
	SetToolbarButtonDisabled(IDI_DEMPAINT, !IsDEM);
	SetToolbarButtonDisabled(IDI_DBTB_SPLIT, IsDEM);
	SetToolbarButtonDisabled(IDI_DBTB_JOIN, IsDEM);
	SetToolbarButtonDisabled(IDI_DBTB_CONFORM, IsDEM);
	} // if

} // DBEditGUI::ConfigureToolbarButtons()


/*===========================================================================*/


void DBEditGUI::ConfigLayerSection(Joe *Active, long NumListItems)
{

BuildLayerList(Active, NumListItems);
if (ActivePage == 3)	// layers
	{
	WidgetShow(IDC_SETATTR, FALSE);
	WidgetShow(IDC_ON1, TRUE);
	WidgetShow(IDC_OFF1, TRUE);
	} // if
else
	{
	WidgetShow(IDC_ON1, FALSE);
	WidgetShow(IDC_OFF1, FALSE);
	WidgetShow(IDC_SETATTR, TRUE);
	} // else

} // DBEditGUI::ConfigLayerSection

/*===========================================================================*/

bool DBEditGUI::CreateSelectedLayer(char *name, bool DEMonly)
{
Joe *cur, *JoeBob;
LayerEntry *layerEntry;
long j;
bool success = false;

if (name && name[0] && (layerEntry = Host->DBLayers.MatchMakeLayer(name, 0)))
	{
	for (j = 0; j < NoOfObjects; j ++)
		{
		if (GetSelected(j))
			{
			if (cur = GetJoeFromOBN(j))
				{
				if (! cur->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					if (! DEMonly || cur->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						JoeBob = GetJoeFromOBN(j);
						if (! JoeBob->MatchLayer(layerEntry))
							{
							JoeBob->AddObjectToLayer(layerEntry);
							success = true;
							} // if
						} // if
					} // if
				} // if
			} // if
		} // for
	} // if

return(success);

} // DBEditGUI::CreateSelectedLayer

/*===========================================================================*/

long DBEditGUI::GetActiveObject(void)
{
long Test;

Test = ListView_GetNextItem(GetWidgetFromID(IDC_LIST1), -1, LVNI_FOCUSED);
if(Test == -1)
	{ // no focused, try first selected instead
	Test = ListView_GetNextItem(GetWidgetFromID(IDC_LIST1), -1, LVNI_SELECTED);
	} // if
return(Test);

} // DBEditGUI::GetActiveObject()

/*===========================================================================*/

short DBEditGUI::GetSelected(long OBN)
{
LVITEM LVI;
LVI.mask = LVIF_STATE;
LVI.stateMask = LVIS_SELECTED;
LVI.iItem = OBN;
if(ListView_GetItem(GetWidgetFromID(IDC_LIST1), &LVI))
	{
	if(LVI.state & LVIS_SELECTED) return(1);
	} // if
return(0);
} // DBEditGUI::GetSelected()

/*===========================================================================*/

void DBEditGUI::DoMaxFractal(int NewMaxFract)
{
DoPatch DP;
long j;

DP.DoOneShort = (void (*)(class Joe *,short))SetMaxFract;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewMaxFract, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			SetMaxFract(GetJoeFromOBN(j), NewMaxFract);
			} // if object selected
		} // for
	} // if

ImBusy = 0;
ConfigureWidgets();

} // DBEditGUI::DoMaxFractal()

/*===========================================================================*/

void DBEditGUI::DoRename(const char *NewName, const char *NewLabel)
{
DoPatch DP;
Joe *CurrentJoe;
long j;
NotifyTag Changes[2];

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

ImBusy = 1;
// Change stuff:
DP.DoProjTwoChar = (void (*)(class Joe *,Project *,const char *, const char *))ChangeNames;
if (NewLabel)
	{
	DP.One = (char *)NewLabel;
	DP.Two = NULL;
	} // if
else
	{
	DP.One = NULL;
	DP.Two = (char *)NewName;
	} // else

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (CurrentJoe = GetJoeFromOBN(j))
				{
				ChangeNames(CurrentJoe, ProjHost, DP.One, DP.Two);
				if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
//BuildList();
ConfigureWidgets();

} // DBEditGUI::DoRename()

/*===========================================================================*/

void DBEditGUI::DoLineWeight(int NewLineWeight)
{
long j;
DoPatch DP;

DP.DoOneShort = (void (*)(class Joe *,short))SetLineWeight;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewLineWeight, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			SetLineWeight(GetJoeFromOBN(j), NewLineWeight);
			} // if object selected
		} // for
	} // if

ImBusy = 0;
ConfigureWidgets();
} // DBEditGUI::DoLineWeight()

/*===========================================================================*/

void DBEditGUI::DoAddLayer(LayerEntry *LayerToAdd)
{
DoPatch DP;
Joe *CurrentJoe;
long j;
NotifyTag Changes[2];

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

ImBusy = 1;
// Change stuff:
DP.DoOneLayer = (void (*)(Joe *, LayerEntry *))LayerAdd;
DP.DoLayer = LayerToAdd;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (CurrentJoe = GetJoeFromOBN(j))
				{
				LayerAdd(CurrentJoe, LayerToAdd);
				if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
//BuildList();
ConfigureWidgets();

} // DBEditGUI::DoAddLayer

/*===========================================================================*/

void DBEditGUI::DoRemoveLayer(LayerEntry *LayerToRemove)
{
DoPatch DP;
Joe *CurrentJoe;
long j;
NotifyTag Changes[2];

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

ImBusy = 1;
// Change stuff:
DP.DoOneLayer = (void (*)(Joe *, LayerEntry *))LayerRemove;
DP.DoLayer = LayerToRemove;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (CurrentJoe = GetJoeFromOBN(j))
				{
				LayerRemove(CurrentJoe, LayerToRemove);
				if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
//BuildList();
ConfigureWidgets();

} // DBEditGUI::DoRemoveLayer

/*===========================================================================*/

void DBEditGUI::DoEnable(bool NewState)
{
long j;
DoPatch DP;
NotifyTag Changes[2];
Joe *Me;

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

DP.DoOneShort = (void (*)(class Joe *,short))SetEnabled;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewState ? 1: 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (Me = GetJoeFromOBN(j))
				{
				SetEnabled(Me, NewState ? 1: 0);
				if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

//ReDrawList();
//ConfigureWidgets();

} // DBEditGUI::DoEnable()

/*===========================================================================*/

void DBEditGUI::DoDrawEnable(bool NewState)
{
long j;
DoPatch DP;
NotifyTag Changes[2];
Joe *Me;

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

DP.DoOneShort = (void (*)(class Joe *,short))SetDrawEnabled;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewState ? 1: 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (Me = GetJoeFromOBN(j))
				{
				SetDrawEnabled(Me, NewState ? 1: 0);
				if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

//ReDrawList();
//ConfigureWidgets();
} // DBEditGUI::DoDrawEnable()

/*===========================================================================*/

void DBEditGUI::DoRenderEnable(bool NewState)
{
long j;
DoPatch DP;
//NotifyTag Changes[2];
Joe *Me;

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

DP.DoOneShort = (void (*)(class Joe *,short))SetRenderEnabled;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewState ? 1: 0, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (Me = GetJoeFromOBN(j))
				{
				SetRenderEnabled(Me, NewState ? 1: 0);
				if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if object selected
		} // for
	} // if

ImBusy = 0;


ReDrawList();
ConfigureWidgets();
} // DBEditGUI::DoRenderEnable()

/*===========================================================================*/

/*
void DBEditGUI::DoToggleEnable(void)
{
long j;
DoPatch DP;
NotifyTag Changes[2];
Joe *Me;

DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;

Enabled = WidgetGetCheck(IDC_ENABLED) ? 0: 1;

DP.DoOneShort = (void (*)(class Joe *,short))SetEnabled;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, Enabled, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			if (Me = GetJoeFromOBN(j))
				{
				SetEnabled(Me, Enabled);
				SetDrawEnabled(Me, Enabled);
				SetRenderEnabled(Me, Enabled);
				if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
					DEMsChanged = 1;
				else if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL))
					ControlPtsChanged = 1;
				else
					VectorsChanged = 1;
				} // if
			} // if
		} // for
	} // if

ImBusy = 0;

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

//ReDrawList();
ConfigureWidgets();

} // DBEditGUI::DoToggleEnable()
*/

/*===========================================================================*/

void DBEditGUI::DoLayerSelect(LayerEntry *LayerToSelect, int Qualifier)
{
long j, Select;
Joe *CurJoe, *LastSelJoe = NULL;

if (LayerToSelect)
	{
	if (Qualifier == 0)
		{
		// deselect everything first
		DeselectAll();
		} // if

	Select = (Qualifier < 2);
	ImBusy = 1;
	for (j = 0; j < NoOfObjects; j ++)
		{
		if ((CurJoe = GetJoeFromOBN(j)) && CurJoe->MatchLayer(LayerToSelect))
			{
			if (! LastSelJoe && Select)
				{
				LastSelJoe = CurJoe;
				Host->SetActiveObj(LastSelJoe);
				} // if
			SetSelected(j, (short)Select);
			} /* if same layer */
		} // for
	ImBusy = 0;
	ReDrawList();
	ConfigureWidgets();
	} // if

if (LastSelJoe && Select)
	{
	UpdateTitle();
	} // if

} // DBEditGUI::DoLayerSelect()

/*===========================================================================*/

void DBEditGUI::DoLayerSelect(int Qualifier)
{
long Current;
LayerEntry *TargetLayer;

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	if ((TargetLayer = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && TargetLayer != (LayerEntry *)LB_ERR)
		{
		DoLayerSelect(TargetLayer, Qualifier);
		} // if
	} // if

} // DBEditGUI::DoLayerSelect()

/*===========================================================================*/

void DBEditGUI::DoLayerOn(void)
{
long Current;
LayerEntry *TargetLayer;

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	if ((TargetLayer = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && TargetLayer != (LayerEntry *)LB_ERR)
		{
		ImBusy = 1;
		TargetLayer->SetJoeFlags(WCS_JOEFLAG_ACTIVATED, false);
		ImBusy = 0;
		} // if
	} // if

ConfigureWidgets();

} // DBEditGUI::DoLayerOn()

/*===========================================================================*/

void DBEditGUI::DoLayerOff(void)
{
long Current;
LayerEntry *TargetLayer;

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	if ((TargetLayer = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && TargetLayer != (LayerEntry *)LB_ERR)
		{
		ImBusy = 1;
		TargetLayer->SetJoeFlags(WCS_JOEFLAG_ACTIVATED, true); // SetJoeFlags(clear) is ClearJoeFlags()
		ImBusy = 0;
		} // if
	} // if

ConfigureWidgets();

} // DBEditGUI::DoLayerOff()

/*===========================================================================*/

void DBEditGUI::ChangeAttribValue(int Item, int Column, const char *NewValue)
{
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
double IEEE;
long j, IsIEEE, IsCalculate = 0, IsMinCalc, IsMaxCalc, IsAvgCalc, IsLenCalc, IsAreaCalc, IsRandCalc;
LayerStub *Stub;
Joe *CurJoe;
NotifyTag Changes[2];
const char *DataInputBuf = NewValue;

Changes[1] = NULL;

time_t rawtime;
unsigned long seedtime;
time(&rawtime);
seedtime = (unsigned long int)rawtime;
xseed48(seedtime, ~seedtime);

int AttribNum = GetAttribNumFromColumn(Column);
LayerEntry *MyLayer = GetAttribColumn(AttribNum);

if(MyLayer)
	{
	if (! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		IsCalculate = ! strnicmp(DataInputBuf, "*calc", 5);
		if (IsCalculate)
			{
			IsMinCalc = ! strnicmp(DataInputBuf, "*calcmin", 8);
			IsMaxCalc = ! strnicmp(DataInputBuf, "*calcmax", 8);
			IsAvgCalc = ! strnicmp(DataInputBuf, "*calcavg", 8);
			IsLenCalc = ! strnicmp(DataInputBuf, "*calclen", 8);
			IsAreaCalc = ! strnicmp(DataInputBuf, "*calcarea", 9);
			IsRandCalc = ! strnicmp(DataInputBuf, "*calcrand", 9);
			} // if
		IEEE = atof(DataInputBuf);
		IsIEEE = (! MyLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE));
		for (j = 0; j < NoOfObjects; j ++)
			{
			if (GetSelected(j) && (CurJoe = GetJoeFromOBN(j)))
				{
				Stub = CurJoe->MatchEntryToStub(MyLayer);
				if (IsIEEE)
					{
					if (IsCalculate)
						{
						double VecCalcLength = 0.0, VecCalcArea = 0.0;
						float MaxEl = 0.0f, MinEl = 0.0f;
						double Random = 0.0;

						if (IsMinCalc || IsMaxCalc || IsAvgCalc) CurJoe->GetElevRange(MaxEl, MinEl);
						if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
							{
							if (IsLenCalc) VecCalcLength = GetVecLength(CurJoe, 0);
							if (IsAreaCalc)
								{
								double VecCalcNorth, VecCalcSouth, VecCalcWest, VecCalcEast;
								VecCalcNorth = GetNorth(CurJoe);
								VecCalcSouth = GetSouth(CurJoe);
								VecCalcWest = GetWest(CurJoe);
								VecCalcEast = GetEast(CurJoe);
								if (VecCalcWest <= 360.0 && VecCalcEast >= - 360.0 && VecCalcNorth <= 90.0 && VecCalcSouth >= -90.0)
									{
									VecCalcArea = CurJoe->ComputeAreaDegrees();
									// figure value in tenths of a kilometer so multiplied they give hectares
									VecCalcArea *= (LatScale(HostLib->GetPlanetRadius()) * .01);
									VecCalcArea *= (LonScale(HostLib->GetPlanetRadius(), (VecCalcNorth + VecCalcSouth) * .5) * .01);
									} // if
								else
									VecCalcArea = 0.0;
								} // if
							if (IsRandCalc) Random = (unsigned int)(xrand48()* 10.0);
							} // if
						IEEE = IsMinCalc ? MinEl: IsMaxCalc ? MaxEl: IsAvgCalc ? (float)((MaxEl + MinEl) * .5): IsLenCalc ? VecCalcLength : IsAreaCalc ? VecCalcArea : IsRandCalc ? Random : 0.0;
						} // if
					if (Stub)
						Stub->SetIEEEAttribVal(IEEE);
					else
						CurJoe->AddIEEEAttribute(MyLayer, IEEE);
					} // if
				else
					{
					if (Stub)
						Stub->SetTextAttribVal(DataInputBuf);
					else
						CurJoe->AddTextAttribute(MyLayer, DataInputBuf);
					} // else
				Changes[0] = MAKE_ID(CurJoe->GetNotifyClass(), CurJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, CurJoe->GetRAHostRoot());
				} // if
			} // for
		BuildLayerList(Host->ActiveObj, ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)));
		} // if
	} // if

#endif // WCS_SUPPORT_GENERIC_ATTRIBS
} // DBEditGUI::ChangeAttribValue

/*===========================================================================*/

void DBEditGUI::DoAttribValue(void)
{
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
double IEEE;
long Current, j, IsIEEE, IsCalculate = 0, IsMinCalc, IsMaxCalc, IsAvgCalc, IsLenCalc, IsAreaCalc, IsRandCalc;
LayerEntry *MyLayer = NULL;
LayerStub *Stub;
Joe *CurJoe;
NotifyTag Changes[2];
char DataInputBuf[200];

DataInputBuf[0] = NULL;
Changes[1] = NULL;

time_t rawtime;
unsigned long seedtime;
time(&rawtime);
seedtime = (unsigned long int)rawtime;
xseed48(seedtime, ~seedtime);

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	if ((MyLayer = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && MyLayer != (LayerEntry *)LB_ERR)
		{
		if (! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
			{
			if (GetInputString("Enter field contents. Type will be determined automatically.", "", DataInputBuf))
				{
				IsCalculate = ! strnicmp(DataInputBuf, "*calc", 5);
				if (IsCalculate)
					{
					IsMinCalc = ! strnicmp(DataInputBuf, "*calcmin", 8);
					IsMaxCalc = ! strnicmp(DataInputBuf, "*calcmax", 8);
					IsAvgCalc = ! strnicmp(DataInputBuf, "*calcavg", 8);
					IsLenCalc = ! strnicmp(DataInputBuf, "*calclen", 8);
					IsAreaCalc = ! strnicmp(DataInputBuf, "*calcarea", 9);
					IsRandCalc = ! strnicmp(DataInputBuf, "*calcrand", 9);
					} // if
				IEEE = atof(DataInputBuf);
				IsIEEE = (! MyLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE));
				for (j = 0; j < NoOfObjects; j ++)
					{
					if (GetSelected(j) && (CurJoe = GetJoeFromOBN(j)))
						{
						Stub = CurJoe->MatchEntryToStub(MyLayer);
						if (IsIEEE)
							{
							if (IsCalculate)
								{
								double VecCalcLength = 0.0, VecCalcArea = 0.0;
								float MaxEl = 0.0f, MinEl = 0.0f;
								double Random = 0.0;

								if (IsMinCalc || IsMaxCalc || IsAvgCalc) CurJoe->GetElevRange(MaxEl, MinEl);
								if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
									{
									if (IsLenCalc) VecCalcLength = GetVecLength(CurJoe, 0);
									if (IsAreaCalc)
										{
										double VecCalcNorth, VecCalcSouth, VecCalcWest, VecCalcEast;
										VecCalcNorth = GetNorth(CurJoe);
										VecCalcSouth = GetSouth(CurJoe);
										VecCalcWest = GetWest(CurJoe);
										VecCalcEast = GetEast(CurJoe);
										if (VecCalcWest <= 360.0 && VecCalcEast >= - 360.0 && VecCalcNorth <= 90.0 && VecCalcSouth >= -90.0)
											{
											VecCalcArea = CurJoe->ComputeAreaDegrees();
											// figure value in tenths of a kilometer so multiplied they give hectares
											VecCalcArea *= (LatScale(HostLib->GetPlanetRadius()) * .01);
											VecCalcArea *= (LonScale(HostLib->GetPlanetRadius(), (VecCalcNorth + VecCalcSouth) * .5) * .01);
											} // if
										else
											VecCalcArea = 0.0;
										} // if
									if (IsRandCalc) Random = (unsigned int)(xrand48()* 10.0);
									} // if
								IEEE = IsMinCalc ? MinEl: IsMaxCalc ? MaxEl: IsAvgCalc ? (float)((MaxEl + MinEl) * .5): IsLenCalc ? VecCalcLength : IsAreaCalc ? VecCalcArea : IsRandCalc ? Random : 0.0;
								} // if
							if (Stub)
								Stub->SetIEEEAttribVal(IEEE);
							else
								CurJoe->AddIEEEAttribute(MyLayer, IEEE);
							} // if
						else
							{
							if (Stub)
								Stub->SetTextAttribVal(DataInputBuf);
							else
								CurJoe->AddTextAttribute(MyLayer, DataInputBuf);
							} // else
						Changes[0] = MAKE_ID(CurJoe->GetNotifyClass(), CurJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
						GlobalApp->AppEx->GenerateNotify(Changes, CurJoe->GetRAHostRoot());
						} // if
					} // for
				} // if
			BuildLayerList(Host->ActiveObj, ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)));
			} // if
		} // if
	} // if

#endif // WCS_SUPPORT_GENERIC_ATTRIBS
} // DBEditGUI::DoAttribValue

/*===========================================================================*/

void DBEditGUI::DoAddAttr(void)
{
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
double IEEE;
long j, IsIEEE;
Joe *CurJoe;
NotifyTag Changes[2];
char FieldInputBuf[200], DataInputBuf[200];

Changes[1] = NULL;
FieldInputBuf[0] = DataInputBuf[0] = NULL;
if (GetInputString("Enter field name", "", FieldInputBuf))
	{
	if (GetInputString("Enter field contents. Type will be determined automatically.", "", DataInputBuf))
		{
		IEEE = atof(DataInputBuf);
		IsIEEE = 1;
		for(j = 0; DataInputBuf[j]; j++)
			{
			if (!(isdigit(DataInputBuf[j]) || (DataInputBuf[j] == '.') || (DataInputBuf[j] == '-') || (DataInputBuf[j] == '+') || (DataInputBuf[j] == 'e')))
				{
				IsIEEE = 0;
				break;
				} // if
			} // for
		for (j = 0; j < NoOfObjects; j ++)
			{
			if (GetSelected(j) && (CurJoe = GetJoeFromOBN(j)))
				{
				if (IsIEEE)
					{
					CurJoe->AddIEEEAttribute(FieldInputBuf, IEEE);
					} // if
				else
					{
					CurJoe->AddTextAttribute(FieldInputBuf, DataInputBuf);
					} // else
				Changes[0] = MAKE_ID(CurJoe->GetNotifyClass(), CurJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
				GlobalApp->AppEx->GenerateNotify(Changes, CurJoe->GetRAHostRoot());
				} // if
			} // for
		} // if
	} // if

BuildLayerList(Host->ActiveObj, ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)));

#endif // WCS_SUPPORT_GENERIC_ATTRIBS
} // DBEditGUI::DoAddAttr

/*===========================================================================*/

int DBEditGUI::MatchSelectedCoordSys(CoordSys *TestCoords, int DEMOnly)
{
long j;
int Found = 0;
Joe *Cur;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;

for (j = 0; j < NoOfObjects; j ++)
	{
	if (GetSelected(j))
		{
		if (Cur = GetJoeFromOBN(j))
			{
			if (! Cur->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (! DEMOnly || Cur->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					if (MyAttr = (JoeCoordSys *)Cur->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
						MyCoords = MyAttr->Coord;
					else
						MyCoords = NULL;
					if (MyCoords != TestCoords)
						return (0);
					Found ++;
					} // if
				} // if
			} // if
		} // if
	} // for j=0...

return (Found);

} // DBEditGUI::MatchSelectedCoordSys

/*===========================================================================*/

int DBEditGUI::GetSelectedBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly)
{
long j;
int Found = 0;
double Bounds;
Joe *Cur;

NewNorth = -FLT_MAX;
NewWest = -FLT_MAX;
NewSouth = FLT_MAX;
NewEast = FLT_MAX;

for (j = 0; j < NoOfObjects; j ++)
	{
	if (GetSelected(j))
		{
		if (Cur = GetJoeFromOBN(j))
			{
			if (! Cur->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (! DEMOnly || Cur->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					Found ++;
					if ((Bounds = Cur->GetNorth()) > NewNorth)
						NewNorth = Bounds;
					if ((Bounds = Cur->GetSouth()) < NewSouth)
						NewSouth = Bounds;
					if ((Bounds = Cur->GetWest()) > NewWest)
						NewWest = Bounds;
					if ((Bounds = Cur->GetEast()) < NewEast)
						NewEast = Bounds;
					} // if
				} // if
			} // if
		} // if
	} // for

if (! Found)
	{
	NewNorth = NewSouth = NewWest = NewEast = 0.0;
	} // if

return (Found);

} // DBEditGUI::GetSelectedBounds

/*===========================================================================*/

int DBEditGUI::GetSelectedNativeBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly)
{
long j;
int Found = 0;
double CurNorth, CurSouth, CurWest, CurEast;
Joe *Cur;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
DEM Topo;

NewNorth = -FLT_MAX;
NewSouth = FLT_MAX;

for (j = 0; j < NoOfObjects; j ++)
	{
	if (GetSelected(j))
		{
		if (Cur = GetJoeFromOBN(j))
			{
			if (! Cur->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (! DEMOnly || Cur->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					if (Found == 0)
						{
						if (MyAttr = (JoeCoordSys *)Cur->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
							MyCoords = MyAttr->Coord;
						else
							MyCoords = NULL;
						if ((! MyCoords) || MyCoords->GetGeographic())
							{
							NewWest = -FLT_MAX;
							NewEast = FLT_MAX;
							} // if
						else
							{
							NewWest = FLT_MAX;
							NewEast = -FLT_MAX;
							} // else
						} // if
					Found ++;
					// load DEM if necessary
					if (Cur->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (Topo.AttemptLoadDEM(Cur, 1, GlobalApp->MainProj))
							{
							CurNorth = Topo.Northest();
							CurSouth = Topo.Southest();
							CurWest = Topo.Westest();
							CurEast = Topo.Eastest();
							Topo.FreeRawElevs();
							} // if
						} // if DEM
					// otherwise fetch vector bounds from vertices
					else
						Cur->GetBoundsProjected(MyCoords, CurNorth, CurWest, CurSouth, CurEast, false);

					if (CurNorth > NewNorth)
						NewNorth = CurNorth;
					if (CurSouth < NewSouth)
						NewSouth = CurSouth;
					// if no CS or geographic
					if ((! MyCoords) || MyCoords->GetGeographic())
						{
						if (CurWest > NewWest)
							NewWest = CurWest;
						if (CurEast < NewEast)
							NewEast = CurEast;
						} // if
					else
						{
						if (CurWest < NewWest)
							NewWest = CurWest;
						if (CurEast > NewEast)
							NewEast = CurEast;
						} // else
					} // if
				} // if
			} // if
		} // if
	} // for

if (! Found)
	{
	NewNorth = NewSouth = NewWest = NewEast = 0.0;
	} // if

return (Found);

} // DBEditGUI::GetSelectedNativeBounds

/*===========================================================================*/


void DBEditGUI::DoSearch(char ClearFirst)
{
Joe *Examine;
Joe *CurrentJoe, *LastSelJoe = NULL;
const char *CurrentName, *OtherName, *CanName, *SecName;
long SetSel;
char Search[100], Cleared = 0, HasName;

Search[0] = 0;
if (CurrentJoe = GetJoeFromOBN(GetActiveObject()))
	{
	strncpy(Search, CurrentJoe->GetBestName(), 99);
	} // if
Search[99] = 0;

if (! GetInputString("Enter search string.", "", Search))
	return;
if (ClearFirst)
	{
	ClearSelected();
	} // if
if (strlen(Search) == 0)
	return;
strupr(Search);

for (long j=0; j<NoOfObjects; j++)
	{
	SetSel = 0;
	if (Examine = GetJoeFromOBN(j))
		{
		HasName = 0;
		CurrentName = Examine->Name();
		OtherName = Examine->FileName();
		CanName = Examine->CanonName();
		if (CurrentName)
			{
			strncpyupr(NameBuf, CurrentName, NAMEBUFLEN);
			if (strstr(NameBuf, Search))
				{
				SetSel = 1;
				} /* if match found */
			HasName = 1;
			} // if
		if (OtherName)
			{
			strncpyupr(NameBuf, OtherName, NAMEBUFLEN);
			if (strstr(NameBuf, Search))
				{
				SetSel = 1;
				} /* if match found */
			HasName = 1;
			} // if
		if (CanName)
			{
			strncpyupr(NameBuf, CanName, NAMEBUFLEN);
			if (strstr(NameBuf, Search))
				{
				SetSel = 1;
				} /* if match found */
			HasName = 1;
			} // if
		SecName = Examine->SecondaryName();
		if (SecName)
			{
			strncpyupr(NameBuf, SecName, NAMEBUFLEN);
			if (strstr(NameBuf, Search))
				{
				SetSel = 1;
				} /* if match found */
			HasName = 1;
			} // if
		if (!HasName)
			{
			strncpyupr(NameBuf, "Unnamed", NAMEBUFLEN);
			if (strstr(NameBuf, Search))
				{
				SetSel = 1;
				} /* if match found */
			} // if
		} // if
	if (SetSel)
		{
		if (!Cleared)
			{
			ClearSelected();
			Cleared = 1;
			} // if
		if (! LastSelJoe && SetSel)
			{
			LastSelJoe = Examine;
			Host->SetActiveObj(LastSelJoe);
			} // if
		SetSelected(j, TRUE);
		} // if
	} // for

if (LastSelJoe && SetSel)
	{
	UpdateTitle();
	} // if

} // DBEditGUI::DoSearch()

/*===========================================================================*/

void DBEditGUI::DoRemove(void)
{
short Remove, DEMSExist = 0, VectorsRemoved = 0, ControlPtsRemoved = 0, DEMsRemoved = 0;
long j;
unsigned long CountDEMs = 0, CountVecs = 0, CountCtrl = 0, CountTotal = 0;
NotifyTag Changes[2];
Joe *Cur;
BusyWin *BWDL = NULL;

ImBusy = 1;
GoModal();
for (j=NoOfObjects-1; j>=0; j--)
	{
	if (GetSelected(j))
		{
		if (Cur = GetJoeFromOBN(j))
			{
			if (!Cur->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Cur->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					DEMSExist = 1;
					CountDEMs++;
					DEMsRemoved = 1;
					} // if
				else if (Cur->TestFlags(WCS_JOEFLAG_ISCONTROL))
					{
					ControlPtsRemoved = 1;
					CountCtrl++;
					} // else if
				else
					{
					VectorsRemoved = 1;
					CountVecs++;
					} // else
				} // if
			} // if
		} // if
	} // for

if (DEMSExist)
	{
	sprintf(NameBuf, "Delete %d DEM elevation files from disk\nas well as remove %d other objects from the Database?", CountDEMs, CountCtrl + CountVecs);
	if ((Remove = UserMessageYNCAN("Database: Remove",
		NameBuf, 1)) == NULL)
		{
		EndModal();
		ImBusy = 0;
		return;
		} // if
	} // if
else if (CountCtrl + CountVecs > 0)
	{
	sprintf(NameBuf, "Delete %d objects from the Database?", CountCtrl + CountVecs);
	if ((Remove = UserMessageOKCAN("Database: Remove",
		NameBuf, 1)) == NULL)
		{
		EndModal();
		ImBusy = 0;
		return;
		} // if
	} // else
else
	{
	sprintf(NameBuf, "There are no objects selected.");
	UserMessageOK("Database: Remove", NameBuf);
	EndModal();
	ImBusy = 0;
	return;
	} // else

Changes[1] = NULL;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
CountTotal = CountCtrl + CountVecs + CountDEMs;
CountCtrl = 0; // reuse as loop counter
BWDL = new BusyWin("Deleting", CountTotal, 'BWDL', 0);
for (j=NoOfObjects-1; j>=0; j--)
	{
	if (GetSelected(j))
		{
		if (Cur = GetJoeFromOBN(j))
			{
			if (!Cur->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Cur->RemoveMe(HostLib))
					{
					if (Remove == 2)
						{
						AttemptDeleteDEMFiles((char *)Cur->FileName(), ProjHost);
						} // if
					delete Cur;
					Cur = NULL;
					} // if
				} // if
			} // if
		NoOfObjects --;
		if (BWDL && (CountCtrl & 0xff)) // only update every 256th time, for efficiency
			{
			BWDL->Update(CountCtrl);
			if (BWDL->CheckAbort())
				{
				break; // bail out of delete loop
				}
			} // if
		CountCtrl++;
		} // if
	} // for

if (BWDL)
	{
	// in case it didn't update recently
	BWDL->Update(CountCtrl);
	delete BWDL;
	} // if

Host->ReBoundTree(WCS_DATABASE_STATIC);
Host->RemoveUnusedLayers();
ImBusy = 0;
if (VectorsRemoved)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsRemoved)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (DEMsRemoved)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
EndModal();
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // DBEditGUI::DoRemove()

/*===========================================================================*/

void DBEditGUI::DoSave(void)
{

	ProjHost->Save(NULL, NULL, Host, HostLib, AppScope->AppImages,
	ProjHost->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
	WCS_PROJECT_IODETAILTAG_CHUNKID, "Database",
	WCS_PROJECT_IODETAILTAG_FLAGS, WCS_DATABASE_STATIC,
	WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
	WCS_PROJECT_IODETAILTAG_GROUP, WCS_PROJECT_LOAD_PATHS_DIRLIST,
	WCS_PROJECT_IODETAILTAG_DONE),
	WCS_DATABASE_STATIC);

} // DBEditGUI::DoSave()

/*===========================================================================*/

void DBEditGUI::DoLoad(int ClearFirst)
{
FileReq *FR;
char extension[32], filename[1024], loadpath[1024], loadfile[256];
long k;
DEM Topo;
NotifyTag Changes[2];
BusyWin *BWFI;

if (FR = new FileReq)
	{
	FR->SetDefPat(WCS_REQUESTER_PARTIALWILD("elev")";"WCS_REQUESTER_PARTIALWILD("db")";"WCS_REQUESTER_PARTIALWILD("proj"));
	FR->SetDefPath("WCSProjects:");
	if (FR->Request(WCS_REQUESTER_FILE_MULTI))
		{
		BWFI = new BusyWin("Files", FR->FilesSelected(), 'BWFI', 0);
		for (k = 0; k < (int)FR->FilesSelected(); k ++)
			{
			if (k == 0)
				{
				strcpy(filename, FR->GetFirstName());
				} // if
			else
				{
				strcpy(filename, FR->GetNextName());
				} // else

			BreakFileName((char *)filename, loadpath, 1024, loadfile, 256);
			stcgfe(extension, loadfile);
			
			// decide what to do with this file
			if ((!stricmp(extension, "db")) || (!stricmp(extension, "proj")))
				{ // load as a WCS/VNS project
				int ioFlags = WCS_DATABASE_STATIC;
				unsigned long dbFlags = WCS_DATABASE_STATIC;

				if (k==0 && ClearFirst) // only clear everything during load of first of potentially multiple database files
					{
					ioFlags |= WCS_DATABASE_LOAD_CLEAR;
					dbFlags |= WCS_DATABASE_LOAD_CLEAR;
					} // if

				// reassemble full path+name
				strmfp(filename, loadpath, loadfile);
				ProjHost->Load(NULL, filename, Host, NULL, NULL,
					ProjHost->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
					WCS_PROJECT_IODETAILTAG_CHUNKID, "Database",
					WCS_PROJECT_IODETAILTAG_FLAGS, ioFlags,
					WCS_PROJECT_IODETAILTAG_CHUNKID, "Paths",
					WCS_PROJECT_IODETAILTAG_GROUP, WCS_PROJECT_LOAD_PATHS_DIRLIST,
					WCS_PROJECT_IODETAILTAG_DONE),
					dbFlags);
				} // if
			else if (!stricmp(extension, "elev"))
				{ // load as a .ELEV DEM
				if (! Host->AddDEMToDatabase(loadpath, loadfile, ProjHost, HostLib))
					{
					if (! UserMessageOKCAN("Database Editor: Add Object",
						"Error reading elevation file!\nContinue with next file?", 0))
						{
						break;
						} // if not continue to next file 
					} // if
				} // else if
			else
				{
				GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_TYPE, loadfile);
				UserMessageOK(loadfile,
					"Error! File not a "APP_TLA" DEM\nOperation terminated.");
				} // else

			if (BWFI)
				{
				if (BWFI->CheckAbort())
					{
					break;
					}
				BWFI->Update(k + 1);
				} // if

			} // for k=0... 

		Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff,	WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);

		delete (BWFI);
		} // if
	delete FR;
	FR = NULL;
	} // if

// BuildList() is no longer necessary because we broadcast WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED above

} // DBEditGUI::DoLoad()

/*===========================================================================*/

void DBEditGUI::DoAppend(void)
{

DoLoad(NULL);

} // DBEditGUI::Append()


/*===========================================================================*/

void DBEditGUI::DoObjectClass(short NewClass)
{
long j;
DoPatch DP;
NotifyTag Changes[2];

short Class = NewClass;

DP.DoOneShort = (void (*)(class Joe *,short))SetClass;

ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, Class, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			SetClass(GetJoeFromOBN(j), Class);
			} // if
		} // for
	} // if
ImBusy = 0;

Changes[1] = NULL;
Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

ConfigureWidgets();

} // DBEditGUI::DoObjectClass()

/*===========================================================================*/

void DBEditGUI::DoLineStyle(short NewStyle)
{
long j;
DoPatch DP;

DP.DoOneShort = (void (*)(class Joe *,short))SetLineStyle;
ImBusy = 1;

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.
if (!DoSomethingToEach(&DP, NewStyle, 0, 0))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			SetLineStyle(GetJoeFromOBN(j), NewStyle);
			} // if
		} // for
	} // if
ImBusy = 0;
ConfigureWidgets();

} // DBEditGUI::DoLineStyle()

/*===========================================================================*/

void DBEditGUI::DoActivate(void)
{
unsigned long CurDude;
long TotNumSel;
Joe *TheDude;

// don't set new item active if multi-selecting because it causes other items to 
// become unselected when HandleNotifyEvent processes
if ((TotNumSel = ListView_GetSelectedCount(GetWidgetFromID(IDC_LIST1))) < 2)
	{
	CurDude = GetActiveObject();
	if(CurDude != -1)
		{
		TheDude = GetJoeFromOBN(CurDude);
		if (TheDude)
			{
			// avoid recursive activation-setting
			ImBusy = 1;
			Host->SetActiveObj(TheDude);
			ImBusy = 0;
			SetActiveObject(CurDude);
			} // if
		} // if
	} // if

} // DBEditGUI::DoActivate()

/*===========================================================================*/

void DBEditGUI::SetActiveObject(long OBN)
{
ClearSelected();
LVITEM LVI;
LVI.mask = LVIF_STATE;
LVI.iItem = OBN;
if(ListView_GetItem(GetWidgetFromID(IDC_LIST1), &LVI))
	{
	LVI.state |= LVIS_FOCUSED;
	ListView_SetItemState(GetWidgetFromID(IDC_LIST1), OBN, LVI.state, LVIS_FOCUSED);
	// LVIS_FOCUSED doesn't seem to really "set" when we do the above, so let's set it LVIS_SELECTED too
	ClearSelected();
	SetSelected(OBN, 1);
	} // if
ConfigureWidgets();
BuildEffectList(GetJoeFromOBN(OBN), NoOfObjects);
ConfigLayerSection(GetJoeFromOBN(OBN), NoOfObjects);
} // DBEditGUI::SetActiveObject()

/*===========================================================================*/

void DBEditGUI::SetSelected(long OBN, short Selected)
{
LVITEM LVI;
LVI.mask = LVIF_STATE;
LVI.iItem = OBN;
if(ListView_GetItem(GetWidgetFromID(IDC_LIST1), &LVI))
	{
	if(Selected) LVI.state |= LVIS_SELECTED;
	else LVI.state &= ~(LVIS_SELECTED);
	ListView_SetItemState(GetWidgetFromID(IDC_LIST1), OBN, LVI.state, LVIS_SELECTED);
	} // if

} // DBEditGUI::SetSelected()

/*===========================================================================*/
// Database Direct Access Functions for DBEditGUI.cpp

Joe *DBEditGUI::GetJoeFromOBN(unsigned long OBN)
{
Joe *Result = NULL;
if(OBN < GridWidgetCache.size())
	{
	Result = GridWidgetCache[OBN];
	} // if
return (Result);

} // DBEditGUI::GetJoeFromOBN()

/*===========================================================================*/

long DBEditGUI::GetOBNFromJoe(Joe *FindMe)
{
long Result = -1; // not found
std::vector<Joe *>::iterator FoundJoe;

FoundJoe = find(GridWidgetCache.begin(), GridWidgetCache.end(), FindMe);
if (FoundJoe != GridWidgetCache.end())
	{
	Result = FoundJoe - GridWidgetCache.begin();
	} // if

return (Result);

} // DBEditGUI::GetOBNFromJoe()

/*===========================================================================*/

void DBEditGUI::SetLayerName(void)
{
Joe *JoeBob;
LayerEntry *LE = NULL;
NotifyTag Changes[2];
long j;

LayerName[0] = 0;

if (GetInputString("Enter new Layer Name", "", LayerName))
	{
	if (LayerName[0])
		{
		if (LE = Host->DBLayers.MatchMakeLayer(LayerName, 0))
			{
			//Me->AddObjectToLayer(LE);
			for (j=0; j<NoOfObjects; j++)
				{
				if (GetSelected(j))
					{
					JoeBob = GetJoeFromOBN(j);
					if (! JoeBob->MatchLayer(LE))
						JoeBob->AddObjectToLayer(LE);
					Changes[0] = MAKE_ID(LE->GetNotifyClass(), LE->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, LE->GetRAHostRoot());
					} // if object selected
				} // for
			BuildLayerList(Host->ActiveObj, ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)));
			} // if
		} // if
	} // if

} // DBEditGUI::SetLayerName()

/*===========================================================================*/

void DBEditGUI::DoRemoveLayer(void)
{
LayerEntry *LE = NULL;
NotifyTag Changes[2];
long j, Current;

LayerName[0] = 0;

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	//WidgetLBGetText(IDC_LAYERLIST, Current, LayerName);
	//if (LayerName[0])
	if ((LE = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && LE != (LayerEntry *)LB_ERR)
		{
		//if (LE = Host->DBLayers.MatchLayer(LayerName))
		//	{
			for (j=0; j<NoOfObjects; j++)
				{
				if (GetSelected(j))
					{
					GetJoeFromOBN(j)->RemoveObjectFromLayer(TRUE, LE);
					Changes[0] = MAKE_ID(LE->GetNotifyClass(), LE->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, LE->GetRAHostRoot());
					} // if
				} // for
			// added 12/11/03 by GRH - try to remove the layer, if it is still in use removal will fail
			Host->RemoveRAHost(LE, ProjHost, HostLib, 1, 1);
			BuildLayerList(Host->ActiveObj, ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)));
		//	} // if
		} // if
	} // if

} // DBEditGUI::DoRemoveLayer()

/*===========================================================================*/

void SetEnabled(Joe *Me, short Status)
{
if (Status) Me->SetFlags  (WCS_JOEFLAG_ACTIVATED);
else       Me->ClearFlags(WCS_JOEFLAG_ACTIVATED);
} // SetEnabled()

/*===========================================================================*/

void SetDrawEnabled(Joe *Me, short Status)
{
if (Status) Me->SetFlags  (WCS_JOEFLAG_DRAWENABLED);
else       Me->ClearFlags(WCS_JOEFLAG_DRAWENABLED);
} // SetDrawEnabled()

/*===========================================================================*/

void SetRenderEnabled(Joe *Me, short Status)
{
if (Status) Me->SetFlags  (WCS_JOEFLAG_RENDERENABLED);
else       Me->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
} // SetRenderEnabled()

/*===========================================================================*/

void __cdecl SetMaxFract(Joe *Me, short MaxFract)
{
JoeDEM *JD;

if (JD = (JoeDEM *)Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
	{
	JD->MaxFract = (char)MaxFract;
	} // if
} // SetMaxFract()

/*===========================================================================*/

void SetRGBPen(Joe *Me, short Red, short Green, short Blue)
{
Me->SetRGB((unsigned char)Red, (unsigned char)Green, (unsigned char)Blue);
} // SetRGBPen()

/*===========================================================================*/

void SetLineStyle(Joe *Me, short Style)
{
Me->SetLineStyle(Style);
return;
} // SetLineStyle()

/*===========================================================================*/

void ChangeNames(Joe *Me, Project *TheProj, const char *O, const char *T)
{

if (Me->TestFlags(WCS_JOEFLAG_ISDEM) && Me->FileName() && T)
	{
	// Try to rename the DEM file.
	AttemptRenameDEMFiles(Me, (char *)T, TheProj);
	} // if

Me->SetNewNames(O, T);
} // ChangeNames

/*===========================================================================*/

void SetLineWeight(Joe *Me, short Weight)
{
Me->SetLineWidth((unsigned char)Weight);
return;
} // SetLineWeight()

/*===========================================================================*/


void LayerAdd(Joe *Me, LayerEntry *AddLayer)
{
Me->AddObjectToLayer(AddLayer);
} // LayerAdd

/*===========================================================================*/

void LayerRemove(Joe *Me, LayerEntry *RemoveLayer)
{
Me->RemoveObjectFromLayer(0, RemoveLayer);
} // LayerRemove


/*===========================================================================*/

void SetClass(Joe *Me, short Class)
{
// 1=Vector, 2=Control Points

if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
	{
	return; // no longer implemented
	} // if
else
	{
	if (Class == 1) // Control, !Illum, !Seg
		{
		Me->SetFlags(WCS_JOEFLAG_ISCONTROL);
		Me->ClearFlags(WCS_JOEFLAG_VEC_AS_SEGMENTED);
		Me->ClearFlags(WCS_JOEFLAG_VEC_ILLUMINATED);
		} // else if
	else //(Class == 0) Normal vector, !Control, !Illum, !Seg
		{
		Me->ClearFlags(WCS_JOEFLAG_VEC_ILLUMINATED);
		Me->ClearFlags(WCS_JOEFLAG_VEC_AS_SEGMENTED);
		Me->ClearFlags(WCS_JOEFLAG_ISCONTROL);
		} // else
	} // else

return;
} // SetClass()

/*===========================================================================*/

#ifdef BUTCHERY
// inlined
const char *DBEditGUI::GetName(Joe *Me)
{
return(Me->Name());
} // DBEditGUI::GetName()
#endif // BUTCHERY

/*===========================================================================*/
/*
const char *DBEditGUI::GetLayerName(Joe *Me, short Layer)
{
LayerStub *LS;
LayerEntry *LE;

if (LS = Me->FirstLayer())
	{
	if (Layer == 1)
		{
		LS = Me->NextLayer(LS);
		} // if
	if (LS)
		{
		if (LE = LS->MyLayer())
			{
			return(LE->GetName());
			} // if
		} // if
	} // if

return(NULL);
} // DBEditGUI::GetLayerName()
*/
/*===========================================================================*/

#ifdef BUTCHERY
// inlined
short DBEditGUI::GetEnabled(Joe *Me)
{
return(Me->TestFlags(WCS_JOEFLAG_ACTIVATED));
} // DBEditGUI::GetEnabled()
#endif // BUTCHERY

/*===========================================================================*/

short DBEditGUI::GetMaxFract(Joe *Me)
{
// Returns 0 if not a DEM-class object. So there.
JoeDEM *JD;

if (JD = (JoeDEM *)Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
	{
	return(JD->MaxFract);
	} // if

return(0);
} // DBEditGUI::GetMaxFract()

/*===========================================================================*/

short DBEditGUI::GetClass(Joe *Me)
{
// V4: 0=Topo, 1=Surface, 2=DryLand, 3=Vector, 4=Illum Vec, 5=Seg Vec, 6=Illum Seg Vec, 7=Control Points
// V5: 0=Topo, 1=Vector, 2=Control Points
// V6/VNS: 0=Vector, 1=Control Points

if (Me->TestFlags(WCS_JOEFLAG_ISDEM))
	{
	return(0); // Plain ol' DEM, no rational value to return
	} // if
if (Me->TestFlags(WCS_JOEFLAG_ISCONTROL))
	{
	return(1);
	} // if
else
	{
	return(0);
	} // if

} // DBEditGUI::GetClass()

/*===========================================================================*/


long DBEditGUI::ValidateCachedELEVData(void)
{
long CountCached = 0;
BusyWin *BWCH = NULL;

Joe *Iterate = NULL;

for(Iterate = Host->GetFirst(); Iterate; Iterate = Host->GetNext(Iterate))
	{
	if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM) && Iterate->TestFlags(WCS_JOEFLAG_ACTIVATED))
		{
		JoeDEM *JD;
		if (JD = (JoeDEM *)Iterate->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
			{
			if(!JD->QueryCachedELEVDataValid())
				{
				if(!JD->UpdateCachedELEVData(Iterate, ProjHost))
					{
					Iterate->ClearFlags(WCS_JOEFLAG_ACTIVATED); // disable it, file is missing
					} // if
				else
					{
					CountCached++;
					if(!BWCH)
						{
						BWCH = new BusyWin("Conforming", 10, 'BWCF', 0);
						} // if
					if(BWCH)
						{
						BWCH->Update(5); // we have no way to judge real progress
						if(BWCH->CheckAbort())
							{
							Iterate = NULL;
							break;
							} // if
						} // if
					} // else
				} // if
			} // if
		} // if
	} // for

if(BWCH) delete BWCH;
BWCH = NULL;

return(CountCached);
} // DBEditGUI::ValidateCachedELEVData

/*===========================================================================*/


long DBEditGUI::BuildList(void)
{
HWND NewListView;
Joe *Iterate = NULL;
long Place, CurListPos = 0;
LayerEntry *MatchLayer = NULL;

GridWidgetCache.clear(); // empty the cache

ValidateCachedELEVData();

// validate Layer/Query filtermodes
if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER)
	{
	if(!(MatchLayer = Host->DBLayers.MatchLayer(GlobalApp->MainProj->Prefs.DBELayerFilterName)))
		{ // failed
		GlobalApp->MainProj->Prefs.DBELayerFilterName[0] = NULL; // clear it
		GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE;
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY)
	{
	if(!QueryFilter)
		{
		SearchQuery *MyEffect;
		for (MyEffect = (SearchQuery *)HostLib->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY); MyEffect; MyEffect = (SearchQuery *)MyEffect->Next)
			{
			if(!stricmp(MyEffect->GetName(), GlobalApp->MainProj->Prefs.DBESearchQueryFilterName))
				{
				QueryFilter = MyEffect;
				break;
				} // if
			} // for
		if(!QueryFilter)
			{
			GlobalApp->MainProj->Prefs.DBESearchQueryFilterName[0] = NULL; // clear it
			GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE;
			} // if
		} // if
	} // else if
#endif // WCS_BUILD_VNS

if (NewListView = GetWidgetFromID(IDC_LIST1))
	{
	Host->ResetGeoClip();
	ListView_DeleteAllItems(NewListView);
	for(Iterate = Host->GetFirst(); Iterate; Iterate = Host->GetNext(Iterate))
		{
		if (!Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
// Show Groups for debugging
//		if (1)
			{
			bool FilterApproved = true;
			// filtering
			if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC)
				{
				if(Iterate->TestFlags(WCS_JOEFLAG_ISDEM)) FilterApproved = false;
				} // if
			else if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM)
				{
				if(!Iterate->TestFlags(WCS_JOEFLAG_ISDEM)) FilterApproved = false;
				} // else if
			else if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER && MatchLayer)
				{
				if(!Iterate->MatchLayer(MatchLayer))
					{
					FilterApproved = false;
					} // if
				} // else if
#ifdef WCS_BUILD_VNS
			else if(GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY && QueryFilter)
				{
				if(!QueryFilter->ApproveJoe(Iterate))
					{
					FilterApproved = false;
					} // if
				} // else if
#endif // WCS_BUILD_VNS
			if(FilterApproved)
				{
				// Add to list
				GridWidgetCache.push_back(Iterate);
				Place = GridWidgetCache.size() - 1;
				if (Iterate == Host->ActiveObj)
					CurListPos = Place;
				} // if
			} // if
		} // for
	ReDrawList();
	SetActiveObject(CurListPos);

	ListView_SetItemCount(NewListView, GridWidgetCache.size());

	if (! Host->ActiveObj)
		Host->SetActiveObj(GetJoeFromOBN(CurListPos));
	BuildEffectList(Host->ActiveObj, GridWidgetCache.size());
	ConfigLayerSection(Host->ActiveObj, GridWidgetCache.size());
	} // if

return(NoOfObjects = GridWidgetCache.size());
} // DBEditGUI::BuildList()

/*===========================================================================*/

void DBEditGUI::BuildEffectList(Joe *Active, long NumListItems)
{
JoeAttribute *MyTurn;
long Current, Items = 0;

if ((Current = WidgetLBGetCurSel(IDC_EFFECTLIST)) == LB_ERR)
	Current = 0;
WidgetLBClear(IDC_EFFECTLIST);

if (NumListItems > 0 && Active)
	{
	if (!Active->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (MyTurn = Active->GetFirstAttribute())
			{
			while (MyTurn)
				{
				if (MyTurn->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && MyTurn->MinorAttrib() != WCS_JOE_ATTRIB_INTERNAL_DEM)
					{
					AddEffectToList(MyTurn);
					Items ++;
					} // if
				MyTurn = MyTurn->GetNextAttribute();
				} // while
			if (Items)
				{
				WidgetLBSetCurSel(IDC_EFFECTLIST, min(Current, Items - 1));
				} //. if
			} // if
		} // if
	} // if

} // DBEditGUI::BuildEffectList

/*===========================================================================*/

void DBEditGUI::BuildLayerList(Joe *Active, long NumListItems)
{
LayerStub *MyTurn;
LayerEntry *MyLayer;
const char *MyName;
long Current, Items = 0, IsAttribute, ListPos;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
char LNameTest[256];
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) == LB_ERR)
	Current = 0;
WidgetLBClear(IDC_LAYERLIST);

if (ActivePage == 4)
	{
	WidgetSetText(IDC_LAYERTEXT, "Attributes of the active Object");
	} // if
else
	{
	WidgetSetText(IDC_LAYERTEXT, "Layers to which active Object belongs");
	} // else

	if (NumListItems > 0 && Active)
		{
		if (MyTurn = Active->FirstLayer())
			{
			while (MyTurn)
				{
				if (MyLayer = MyTurn->MyLayer())
					{
					if (MyName = MyLayer->GetName())
						{
						IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
						#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
						if (IsAttribute && ActivePage == 4)
							{
							if (! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
								{
								if (MyLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
									{
									sprintf(LNameTest, "%s=%s", &MyName[1], MyTurn->GetTextAttribVal());
									if ((ListPos = WidgetLBInsert(IDC_LAYERLIST, -1, LNameTest)) != LB_ERR)
										{
										Items ++;
										WidgetLBSetItemData(IDC_LAYERLIST, ListPos, MyLayer);
										} // if
									} // if
								else
									{
									sprintf(LNameTest, "%s=%g", &MyName[1], MyTurn->GetIEEEAttribVal());
									//TrimZeros(LNameTest);
									if ((ListPos = WidgetLBInsert(IDC_LAYERLIST, -1, LNameTest)) != LB_ERR)
										{
										Items ++;
										WidgetLBSetItemData(IDC_LAYERLIST, ListPos, MyLayer);
										} // if
									} // else
								} // if not link
							} // if
						else if (! IsAttribute && ActivePage == 3)
							{
							if ((ListPos = WidgetLBInsert(IDC_LAYERLIST, -1, MyName)) != LB_ERR)
								{
								Items ++;
								WidgetLBSetItemData(IDC_LAYERLIST, ListPos, MyLayer);
								} // if
							} // else
						#else // !WCS_SUPPORT_GENERIC_ATTRIBS
						if (! IsAttribute)
							{
							if ((ListPos = WidgetLBInsert(IDC_LAYERLIST, -1, MyName)) != LB_ERR)
								{
								Items ++;
								WidgetLBSetItemData(IDC_LAYERLIST, ListPos, MyLayer);
								} // if
							} // if
						#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
						} // if
					} // if
				MyTurn = Active->NextLayer(MyTurn);
				} // while
			if (Items)
				{
				WidgetLBSetCurSel(IDC_LAYERLIST, min(Current, Items - 1));
				} //. if
			} // if
		} // if

} // DBEditGUI::BuildLayerList

/*===========================================================================*/

const char *DBEditGUI::ParseEffectNameClass(JoeAttribute *Me, bool DecorateWithPrefix)
{
long Place = 0;
static char EffectNameClass[512];
RasterAnimHostProperties HostProp;

if (Me)
	{
	GeneralEffect *Effect = NULL;
	HostProp.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;

	switch (Me->MinorAttrib())
		{
		case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		/*
		case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		*/
		case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		#ifdef WCS_THEMATIC_MAP
		case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		#endif // WCS_THEMATIC_MAP
		#ifdef WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		#endif // WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		/*
		case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		*/
			{
			Effect = Me->GetGeneralEffect();
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CMAP
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
		} // switch
	if(Effect)
		{
		char ListName[256];
		ListName[0] = NULL;
		if(DecorateWithPrefix)
			{
			if (Effect->Enabled)
				strcpy(ListName, "* ");
			else
				strcpy(ListName, "  ");
			if (Effect->Joes && Effect->Joes->Next)
				strcat(ListName, "+ ");
			else
				strcat(ListName, "  ");
			} // if
		Effect->GetRAHostProperties(&HostProp);
		sprintf(EffectNameClass, "%s%s %s", ListName, HostProp.Name, HostProp.Type);
		return(EffectNameClass);
		} // if
	} // if

return(NULL);
} // DBEditGUI::ParseEffectNameClass()

/*===========================================================================*/

long DBEditGUI::AddEffectToList(JoeAttribute *Me)
{
long Place = 0;

if (Me)
	{
	switch (Me->MinorAttrib())
		{
		case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		/*
		case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		*/
		case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		#ifdef WCS_THEMATIC_MAP
		case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		#endif // WCS_THEMATIC_MAP
		#ifdef WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		#endif // WCS_COORD_SYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		/*
		case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		*/
			{
			const char *CombinedString = ParseEffectNameClass(Me, true);
			if ((Place = WidgetLBInsert(IDC_EFFECTLIST, -1, CombinedString)) != LB_ERR)
				{
				WidgetLBSetItemData(IDC_EFFECTLIST, Place, Me->GetGeneralEffect());
				} // if
			break;
			} // case for all
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
		} // switch
	} // if

return(Place);
} // DBEditGUI::AddEffectToList()

/*===========================================================================*/

void DBEditGUI::ReDrawList(void)
{
WidgetRepaint(IDC_LIST1);
} // DBEditGUI::ReDrawList


void DBEditGUI::ChangeSelection(Joe *NewSel, char NewState)
{
long NewGuy;

NewGuy = GetOBNFromJoe(NewSel);
if (NewGuy != -1)
	{
	SetSelected(NewGuy, NewState);
	} // if
} // DBEditGUI::ChangeSelection


int DBEditGUI::DoSomethingToEach(DoPatch *MyDP, short ArgA, short ArgB, short ArgC)
{
unsigned long int DoCount;
std::vector<long> SelArray;
HWND JoeListWnd = GetWidgetFromID(IDC_LIST1);
Joe *Dude;

for(long Current = -1;;)
	{
	if((Current = ListView_GetNextItem(JoeListWnd, Current, LVNI_SELECTED)) != -1)
		{
		SelArray.push_back(Current);
		} // if
	else
		{
		break;
		} // else
	} // for
	
			
for(DoCount = 0; DoCount < SelArray.size(); DoCount++)
	{
	Dude = GridWidgetCache[SelArray[DoCount]];
	if (MyDP->DoThreeShort) MyDP->DoThreeShort(Dude, ArgA, ArgB, ArgC);
	if (MyDP->DoOneShort) MyDP->DoOneShort(Dude, ArgA);
	if (MyDP->DoProjTwoChar) MyDP->DoProjTwoChar(Dude, ProjHost, MyDP->One, MyDP->Two);
	if (MyDP->DoOneLayer) MyDP->DoOneLayer(Dude, MyDP->DoLayer);
	if (Dude->TestFlags(WCS_JOEFLAG_ISDEM))
		DEMsChanged = 1;
	else if (Dude->TestFlags(WCS_JOEFLAG_ISCONTROL))
		ControlPtsChanged = 1;
	else
		VectorsChanged = 1;
	} // for
return(true);
} // DBEditGUI::DoSomethingToEach


void DBEditGUI::ClearSelected(void)
{
ListView_SetItemState(GetWidgetFromID(IDC_LIST1), -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
} // DBEditGUI::ClearSelected


static char NewTitleBuffer[50];

void DBEditGUI::UpdateTitle(void)
{
int TotNumObjs, TotNumSel;
char FilterDescString[100];

TotNumObjs = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1));
TotNumSel = ListView_GetSelectedCount(GetWidgetFromID(IDC_LIST1));
FilterDescString[0] = NULL;
if(GlobalApp->MainProj->Prefs.DBEFilterMode != WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE)
	{
	switch(GlobalApp->MainProj->Prefs.DBEFilterMode)
		{
		case WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC:
			{
			sprintf(FilterDescString, " [Filter: Vectors]");
			break;
			} // WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC
		case WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM:
			{
			sprintf(FilterDescString, " [Filter: DEMs]");
			break;
			} // WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM
		case WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER:
			{
			sprintf(FilterDescString, " [Filter: Layer=%s]", GlobalApp->MainProj->Prefs.DBELayerFilterName);
			break;
			} // WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER
		case WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY:
			{
			sprintf(FilterDescString, " [Filter: Query \"%s\"]", GlobalApp->MainProj->Prefs.DBESearchQueryFilterName);
			break;
			} // WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY
		}
	} // if
sprintf(NewTitleBuffer, "Database Editor: %d of %d%s", TotNumSel, TotNumObjs, FilterDescString);
SetTitle(NewTitleBuffer);
} // DBEditGUI::UpdateTitle

void DBEditGUI::SetRegionSel(int NewSelState, int Inclusive,
 double NewNorth, double NewSouth, double NewEast, double NewWest)
{
Joe *Ca;
long j;

if (Host)
	{
	Host->SetGeoClip(NewNorth, NewSouth, NewEast, NewWest);
	for (j=0; j<NoOfObjects; j++)
		{
		if (Inclusive)
			{
			if (Ca = GetJoeFromOBN(j))
				{
				if (Host->GeoClipMatch(Ca))
					{
					SetSelected(j, NewSelState);
					} // if
				} // if
			} // if
		else
			{
			}
		} // for
	} // if
} // DBEditGUI::SetRegionSel

/*===========================================================================*/

void DBEditGUI::ModifyEffect(void)
{
long Active;
GeneralEffect *Test;

if ((Active = WidgetLBGetCurSel(IDC_EFFECTLIST)) != LB_ERR)
	{
	if ((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_EFFECTLIST, Active)) != (GeneralEffect *)LB_ERR && Test)
		{
		Test->EditRAHost();
		} // if
	} // if

} // DBEditGUI::ModifyEffect

/*===========================================================================*/

void DBEditGUI::RemoveEffect(void)
{
long NumEffectItems, EffectItem, NumDBItems, DBItem;
int DoRemoveAll = 0;
GeneralEffect *Test;
Joe *MyGuy;
//JoeAttribute *MyAttr;

if ((NumDBItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1))) && NumDBItems)
	{
	if ((NumEffectItems = WidgetLBGetCount(IDC_EFFECTLIST)) != LB_ERR && NumEffectItems)
		{
		for (EffectItem = 0; EffectItem < NumEffectItems; EffectItem ++)
			{
			if (WidgetLBGetSelState(IDC_EFFECTLIST, EffectItem))
				{
				if ((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_EFFECTLIST, EffectItem)) != (GeneralEffect *)LB_ERR && Test)
					{
					for (DBItem = 0; DBItem < NumDBItems; DBItem ++)
						{
						if (GetSelected(DBItem))
							{
							if (MyGuy = GetJoeFromOBN(DBItem))
								{
								MyGuy->FindnRemoveRAHostChild(Test, DoRemoveAll);
								//if (MyAttr = MyGuy->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)Test->EffectType, Test))
								//	delete MyAttr;
								//Test->RemoveFromJoeList(MyGuy);
								} // if
							} // if
						} // for
					} // if
				} // if
			} // for
		} // if
	} // if

} // DBEditGUI::RemoveEffect

/*===========================================================================*/

void DBEditGUI::ApplyEffectToSelected(GeneralEffect *Effect)
{
long NumItems, Item, ApplyToAll = 0;
Joe *ThisGuy;

if (Effect)
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (ThisGuy = GetJoeFromOBN(Item))
					{
					if (ThisGuy->AddEffect(Effect, ApplyToAll) == 2)
						ApplyToAll = 1;
					} // if
				} // if
			} // for
		} // if
	} // if


} // DBEditGUI::ApplyEffectToSelected

/*===========================================================================*/

Joe *DBEditGUI::GetNextSelected(long &LastOBN)
{
long NumItems;
Joe *ThisGuy;

if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
	{
	for (LastOBN = LastOBN >= 0 ? LastOBN + 1: 0; LastOBN < NumItems; LastOBN ++)
		{
		if (GetSelected(LastOBN))
			{
			if (ThisGuy = GetJoeFromOBN(LastOBN))
				{
				return (ThisGuy);
				} // if
			} // if
		} // for
	} // if

return (NULL);

} // DBEditGUI::GetNextSelected

/*===========================================================================*/

void DBEditGUI::SelectByJoeList(JoeList *List)
{
long NumItems, Item;
Joe *ThisGuy;
JoeList *ListItem;

if (List)
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (! GetSelected(Item))
				{
				if (ThisGuy = GetJoeFromOBN(Item))
					{
					ListItem = List;
					while (ListItem)
						{
						if (ListItem->Me == ThisGuy)
							{
							SetSelected(Item, TRUE);
							break;
							} // if
						ListItem = ListItem->Next;
						} // while
					} // if
				} // if
			} // for
		} // if
	} // if

} // DBEditGUI::SelectByJoeList

/*===========================================================================*/

void DBEditGUI::DeselectAll(void)
{
long NumItems, Item;

if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
	{
	for (Item = 0; Item < NumItems; Item ++)
		{
		if (GetSelected(Item))
			{
			SetSelected(Item, FALSE);
			} // if
		} // for
	} // if

} // DBEditGUI::DeselectAll

/*===========================================================================*/

void DBEditGUI::AddByLayer(void)
{

if (Host->AddEffectsByLayer(HostLib))
	BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);

} // DBEditGUI::AddByLayer

/*===========================================================================*/

void DBEditGUI::AddByName(void)
{

if (Host->AddEffectsByName(HostLib))
	BuildEffectList(GetJoeFromOBN(GetActiveObject()), NoOfObjects);

} // DBEditGUI::AddByName

/*===========================================================================*/

/*===========================================================================*/

void DBEditGUI::OpenEditor(void)
{
Joe *CurrentJoe;

if (CurrentJoe = GetJoeFromOBN(GetActiveObject()))
	{
	CurrentJoe->EditRAHost();
	} // if

} // DBEditGUI::OpenEditor

/*===========================================================================*/

int DBEditGUI::ConfirmActiveObject(Joe *ConfirmMe)
{

return (ConfirmMe == GetJoeFromOBN(GetActiveObject()));

} // DBEditGUI::ConfirmActiveObject

/*===========================================================================*/

int DBEditGUI::ConfirmSelectedObject(Joe *ConfirmMe)
{
long ObjNum;

if ((ObjNum = GetOBNFromJoe(ConfirmMe)) >= 0)
	return (GetSelected(ObjNum));
return (FALSE);

} // DBEditGUI::ConfirmSelectedObject

/*===========================================================================*/

int DBEditGUI::ConfirmEnabledObject(Joe *ConfirmMe)
{

return (GetEnabled(ConfirmMe));

} // DBEditGUI::ConfirmEnabledObject

/*===========================================================================*/

int DBEditGUI::ConfirmRenderEnabledObject(Joe *ConfirmMe)
{

return (GetRenderEnabled(ConfirmMe));

} // DBEditGUI::ConfirmRenderEnabledObject

/*===========================================================================*/
/*===========================================================================*/

// Database Export GUI

JoeApproveHook LocalJoeApproveHook;

NativeGUIWin DBExportGUI::Open(Project *Moi)
{
return(GUIFenetre::Open(Moi));
} // DBExportGUI::Open

/*===========================================================================*/

NativeGUIWin DBExportGUI::Construct(void)
{
int ListEntry;
#ifdef WCS_BUILD_VNS
GeneralEffect *MyEffect;
#endif // WCS_BUILD_VNS
char *FormatList[] = {"DXF" , "Arc Shape"};
char *DEMsAsList[] = {"Points", "Polyline", "Polygon Mesh", "Polyface Mesh", "3D Faces"};
char *XYUnitsList[] = {"Meter", "Feet", "US Survey Feet", "Inch", "Decimeter", "Centimeter", "Millimeter"};
char *ZUnitsList[] = {"Meter", "Feet", "US Survey Feet", "Inch", "Decimeter", "Centimeter", "Millimeter"};

if (!NativeWin)
	{

	#ifdef WCS_BUILD_VNS
	NativeWin = CreateWinFromTemplate(IDD_DB_EXPORT_VNS, LocalWinSys()->RootWin);
	#else // WCS_BUILD_VNS
	NativeWin = CreateWinFromTemplate(IDD_DB_EXPORT, LocalWinSys()->RootWin);
	#endif // WCS_BUILD_VNS

	if (NativeWin)
		{
		// Set up Widget bindings

		for (ListEntry=0; ListEntry<2; ListEntry++)
			WidgetCBInsert(IDC_FORMATDROP, -1, FormatList[ListEntry]);
		for (ListEntry=0; ListEntry<5; ListEntry++)
			WidgetCBInsert(IDC_DEMSASDROP, -1, DEMsAsList[ListEntry]);
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			ListEntry = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, ListEntry, MyEffect);
			} // for
		for (ListEntry=0; ListEntry<3; ListEntry++)
			WidgetCBInsert(IDC_XYUNITS, -1, XYUnitsList[ListEntry]);
		for (ListEntry=0; ListEntry<3; ListEntry++)
			WidgetCBInsert(IDC_ZUNITS, -1, ZUnitsList[ListEntry]);
		#endif // WCS_BUILD_VNS

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // DBExportGUI::Construct

/*===========================================================================*/

DBExportGUI::DBExportGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, DBEditGUI *DBEditorSource)
: GUIFenetre('DBOU', this, "Export Database") // Yes, I know...
{
#ifdef WCS_BUILD_VNS
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								0};
#endif // WCS_BUILD_VNS

ConstructError = 0;

DBHost = DBSource;
ProjHost = ProjSource;
EffectsHost = EffectsSource;
ExportCoords = NULL;
LocalJoeApproveHook.ObjectThis = DBEditorSource;
LocalJoeApproveHook.Approve = DBEditJoeApprove;

ExportFormat = WCS_DBEXPORT_FORMAT_DXF;
LocalJoeApproveHook.ExportDEMsAs = WCS_DBEXPORT_DEMSAS_POLYFACEMESH;
LocalJoeApproveHook.ExportObjects = WCS_DBEXPORT_OBJECTS_ENABLED;
LocalJoeApproveHook.ExportClasses = WCS_DBEXPORT_CLASS_VECTORS;

ExportLandscapes = (LocalJoeApproveHook.ExportClasses & WCS_DBEXPORT_CLASS_LANDSCAPES);
ExportVectors = (LocalJoeApproveHook.ExportClasses & WCS_DBEXPORT_CLASS_VECTORS);
ExportControlPts = (LocalJoeApproveHook.ExportClasses & WCS_DBEXPORT_CLASS_CONTROLPTS);

ExportXYUnits = WCS_DBEXPORT_XYUNITS_METERS;	
ExportZUnits = WCS_DBEXPORT_ZUNITS_METERS;
ExportXYScale = 1.0;
ExportZScale = 1.0;

#ifdef WCS_BUILD_VNS
GlobalApp->AppEx->RegisterClient(this, AllEvents);
#endif // WCS_BUILD_VNS
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

if (! DBSource || ! DBEditorSource)
	ConstructError = 1;

} // DBExportGUI::DBExportGUI

/*===========================================================================*/

DBExportGUI::~DBExportGUI()
{

#ifdef WCS_BUILD_VNS
GlobalApp->AppEx->RemoveClient(this);
#endif // WCS_BUILD_VNS
LocalJoeApproveHook.ObjectThis = NULL;

} // DBEditGUI::~DBEditGUI()

/*===========================================================================*/

long DBExportGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DBO, 0);
return(0);

} // DBExportGUI::HandleCloseWin

/*===========================================================================*/

long DBExportGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
switch (CtrlID)
	{
	case IDC_CHECKLANDSCAPES:
	case IDC_CHECKSURFACES:
	case IDC_CHECKVECTORS:
	case IDC_CHECKCONTROLPTS:
		{
		DoClassCheck();
		break;
		} //
	} // ButtonID
return(0);
} // DBExportGUI::HandleSCChange

/*===========================================================================*/

long DBExportGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch (ButtonID)
	{
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DBO, 0);
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (ExportCoords)
			ExportCoords->EditRAHost();
		break;
		} // IDC_EDITCOORDS
	case IDC_EXPORT:
		{
		#ifdef WCS_BUILD_DEMO
		UserMessageDemo("Database Export is disabled.");
		#else // !WCS_BUILD_DEMO
		DoExport();
		#endif // !WCS_BUILD_DEMO
		break;
		} // 
	} // switch
return(0);

} // DBExportGUI::HandleButtonClick

/*===========================================================================*/

long DBExportGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
switch (CtrlID)
	{
	case IDC_FORMATDROP:
		{
		DoFormat();
		break;
		} // day
	case IDC_DEMSASDROP:
		{
		DoDEMsAs();
		break;
		} // day
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		} // day
	case IDC_XYUNITS:
		switch (WidgetCBGetCurSel(IDC_XYUNITS))
			{
			default:
			case WCS_DBEXPORT_XYUNITS_METERS:
				ExportXYScale = 1.0;
				break;
			case WCS_DBEXPORT_XYUNITS_DECI:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_DECIMETER);
				break;
			case WCS_DBEXPORT_XYUNITS_CENTI:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_CENTIMETER);
				break;
			case WCS_DBEXPORT_XYUNITS_MILLI:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_MILLIMETER);
				break;
			case WCS_DBEXPORT_XYUNITS_USFEET:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
				break;
			case WCS_DBEXPORT_XYUNITS_FEET:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_FEET);
				break;
			case WCS_DBEXPORT_XYUNITS_INCH:
				ExportXYScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_INCH);
				break;
			}
		break;
	case IDC_ZUNITS:
		switch (WidgetCBGetCurSel(IDC_ZUNITS))
			{
			default:
			case WCS_DBEXPORT_ZUNITS_METERS:
				ExportZScale = 1.0;
				break;
			case WCS_DBEXPORT_ZUNITS_DECI:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_DECIMETER);
				break;
			case WCS_DBEXPORT_ZUNITS_CENTI:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_CENTIMETER);
				break;
			case WCS_DBEXPORT_ZUNITS_MILLI:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_MILLIMETER);
				break;
			case WCS_DBEXPORT_ZUNITS_FEET:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_FEET);
				break;
			case WCS_DBEXPORT_ZUNITS_USFEET:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
				break;
			case WCS_DBEXPORT_ZUNITS_INCH:
				ExportZScale = ConvertFromMeters(1.0, WCS_USEFUL_UNIT_INCH);
				break;
			}
		break;
	} // ButtonID

return(0);
} // DBExportGUI::HandleCBChange

/*===========================================================================*/

void DBExportGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes;
#ifdef WCS_BUILD_VNS
NotifyTag Interested[7];
long CurPos, Pos;
GeneralEffect *MyEffect, *MatchEffect;
#endif // WCS_BUILD_VNS

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

#ifdef WCS_BUILD_VNS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPos = -1;
	MatchEffect = ExportCoords;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	if (CurPos < 0)
		ExportCoords = NULL;
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

DisableWidgets();

} // DBExportGUI::HandleNotifyEvent()

/*===========================================================================*/

void DBExportGUI::ConfigureWidgets(void)
{
#ifdef WCS_BUILD_VNS
long Ct, ListPos, NumEntries;
CoordSys *TestObj;
#endif // WCS_BUILD_VNS

WidgetCBSetCurSel(IDC_FORMATDROP, ExportFormat);
WidgetCBSetCurSel(IDC_DEMSASDROP, LocalJoeApproveHook.ExportDEMsAs);
WidgetCBSetCurSel(IDC_XYUNITS, ExportXYUnits);
WidgetCBSetCurSel(IDC_ZUNITS, ExportZUnits);
ConfigureSR(NativeWin, IDC_RADIOACTIVE, IDC_RADIOACTIVE, &LocalJoeApproveHook.ExportObjects, SRFlag_Long, WCS_DBEXPORT_OBJECTS_ACTIVE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOACTIVE, IDC_RADIOSELECTED, &LocalJoeApproveHook.ExportObjects, SRFlag_Long, WCS_DBEXPORT_OBJECTS_SELECTED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOACTIVE, IDC_RADIOENABLED, &LocalJoeApproveHook.ExportObjects, SRFlag_Long, WCS_DBEXPORT_OBJECTS_ENABLED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOACTIVE, IDC_RADIOALL, &LocalJoeApproveHook.ExportObjects, SRFlag_Long, WCS_DBEXPORT_OBJECTS_ALL, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKLANDSCAPES, &ExportLandscapes, SCFlag_Long, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKVECTORS, &ExportVectors, SCFlag_Long, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKCONTROLPTS, &ExportControlPts, SCFlag_Long, NULL, NULL);
WidgetSetCheck(IDC_RADIOLONPOSEAST, 1);

#ifdef WCS_BUILD_VNS
if (ExportCoords)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestObj == ExportCoords)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	if (ListPos < 0)
		ExportCoords = NULL;
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
#endif // WCS_BUILD_VNS

DisableWidgets();

} // DBExportGUI::ConfigureWidgets()

/*===========================================================================*/

void DBExportGUI::DoClassCheck(void)
{

LocalJoeApproveHook.ExportClasses = (ExportLandscapes ? WCS_DBEXPORT_CLASS_LANDSCAPES: 0) +
				(ExportLandscapes ? WCS_DBEXPORT_CLASS_SURFACES: 0) +	// surfaces were an old WCS V3- feature
				(ExportVectors ? WCS_DBEXPORT_CLASS_VECTORS: 0) +
				(ExportControlPts ? WCS_DBEXPORT_CLASS_CONTROLPTS: 0);
DisableWidgets();

} // DBExportGUI::DoClassCheck

/*===========================================================================*/

void DBExportGUI::DoFormat(void)
{

ExportFormat = WidgetCBGetCurSel(IDC_FORMATDROP);
WidgetCBSetCurSel(IDC_FORMATDROP, ExportFormat);

} // DBExportGUI::DoFormat

/*===========================================================================*/

void DBExportGUI::DoDEMsAs(void)
{

LocalJoeApproveHook.ExportDEMsAs = WidgetCBGetCurSel(IDC_DEMSASDROP);
WidgetCBSetCurSel(IDC_DEMSASDROP, LocalJoeApproveHook.ExportDEMsAs);

} // DBExportGUI::DoDEMsAs

/*===========================================================================*/

void DBExportGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj, *MadeObj = NULL;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (MadeObj = NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	ExportCoords = NewObj;
	ConfigureWidgets();
	if (MadeObj)
		MadeObj->EditRAHost();
	} // if
#endif // WCS_BUILD_VNS

} // DBExportGUI::SelectNewCoords

/*===========================================================================*/

void DBExportGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOLONPOSWEST, ExportCoords && ExportCoords->Method.GCTPMethod);
WidgetSetDisabled(IDC_RADIOLONPOSEAST, ExportCoords && ExportCoords->Method.GCTPMethod);
WidgetSetDisabled(IDC_DEMSASDROP, (LocalJoeApproveHook.ExportClasses & WCS_DBEXPORT_CLASS_LANDSCAPES) ? 0: 1);
WidgetSetDisabled(IDC_XYUNITS, !(ExportCoords && ExportCoords->Method.GCTPMethod));
WidgetSetDisabled(IDC_ZUNITS, !(ExportCoords && ExportCoords->Method.GCTPMethod));

} // DBExportGUI::DisableWidgets

/*===========================================================================*/

void DBExportGUI::DoExport(void)
{
int FlipLon = 0;

if (! ExportCoords || ! ExportCoords->Method.GCTPMethod)
	{
	if (WidgetGetCheck(IDC_RADIOLONPOSEAST))
		{
		FlipLon = 1;
		} // if
	} // if

if (LocalJoeApproveHook.ObjectThis)
	{
	switch (ExportFormat)
		{
		case WCS_DBEXPORT_FORMAT_DXF:
			{
			DBHost->ExportDXF(ProjHost, &LocalJoeApproveHook, ExportCoords, ExportXYScale, ExportZScale, FlipLon);
			break;
			} // WCS_DBEXPORT_FORMAT_DXF
		case WCS_DBEXPORT_FORMAT_SHAPE:
			{
			GIS_Name_Warned = false;
			DBHost->ExportShape(ProjHost, &LocalJoeApproveHook, ExportCoords, ExportXYScale, ExportZScale, FlipLon);
			break;
			} // WCS_DBEXPORT_FORMAT_SHAPE
		} // switch
	} // if

} // DBExportGUI::DoExport

/*===========================================================================*/

int DBEditJoeApprove(JoeApproveHook *JAH)
{
int Confirmed = 0;

switch (JAH->ExportObjects)
	{
	case WCS_DBEXPORT_OBJECTS_ACTIVE:
		{
		Confirmed = ((DBEditGUI *)JAH->ObjectThis)->ConfirmActiveObject(JAH->ApproveMe);
		break;
		} // WCS_DBEXPORT_OBJECTS_ACTIVE
	case WCS_DBEXPORT_OBJECTS_SELECTED:
		{
		Confirmed = ((DBEditGUI *)JAH->ObjectThis)->ConfirmSelectedObject(JAH->ApproveMe);
		break;
		} // WCS_DBEXPORT_OBJECTS_SELECTED
	case WCS_DBEXPORT_OBJECTS_ENABLED:
		{
		Confirmed = ((DBEditGUI *)JAH->ObjectThis)->ConfirmEnabledObject(JAH->ApproveMe);
		break;
		} // WCS_DBEXPORT_OBJECTS_ENABLED
	case WCS_DBEXPORT_OBJECTS_RENDERENABLED:
		{
		Confirmed = ((DBEditGUI *)JAH->ObjectThis)->ConfirmRenderEnabledObject(JAH->ApproveMe);
		break;
		} // WCS_DBEXPORT_OBJECTS_RENDERENABLED
	case WCS_DBEXPORT_OBJECTS_ALL:
		{
		Confirmed = TRUE;
		break;
		} // WCS_DBEXPORT_OBJECTS_ALL
	default:
		break;
	} // switch

if (Confirmed)
	{
	if (JAH->ApproveMe->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (JAH->ApproveMe->TestFlags(WCS_JOEFLAG_DEM_AS_SURFACE))
			{
			return (JAH->ExportClasses & WCS_DBEXPORT_CLASS_SURFACES);
			} // if surface
		else
			{
			return (JAH->ExportClasses & WCS_DBEXPORT_CLASS_LANDSCAPES);
			} // else just a landscape
		} // if
	else if (JAH->ApproveMe->TestFlags(WCS_JOEFLAG_ISCONTROL))
		{
		return (JAH->ExportClasses & WCS_DBEXPORT_CLASS_CONTROLPTS);
		} // else if control points
	else
		{
		return (JAH->ExportClasses & WCS_DBEXPORT_CLASS_VECTORS);
		} // else if control points
	} // if confirmed

return (0);

} // DBEditJoeApprove

/*===========================================================================*/

void DBEditGUI::ConformVector(void)
{
HMENU ConformPopupMenu;

if(ConformPopupMenu = CreatePopupMenu())
	{
	POINT CursorPos;
	long ConformPopResult;
	GetCursorPos(&CursorPos);
	AppendMenu(ConformPopupMenu, MF_STRING | MF_DISABLED, 58000, "Conform to:");
	AppendMenu(ConformPopupMenu, MF_STRING | MF_ENABLED, ID_DBEDIT_CONFORM_MENU_TOPO, "Terrain");
	AppendMenu(ConformPopupMenu, MF_STRING | MF_ENABLED, ID_DBEDIT_CONFORM_MENU_NAME, "Name");
	AppendMenu(ConformPopupMenu, MF_STRING | MF_ENABLED, ID_DBEDIT_CONFORM_MENU_LABEL, "Label");
	AppendMenu(ConformPopupMenu, MF_STRING | MF_ENABLED, ID_DBEDIT_CONFORM_MENU_LAYER, "Layer");
	#ifdef WCS_BUILD_VNS
	AppendMenu(ConformPopupMenu, MF_STRING | MF_ENABLED, ID_DBEDIT_CONFORM_MENU_ATTRIB, "Attribute");
	#endif // WCS_BUILD_VNS

	if(ConformPopResult = TrackPopupMenu(ConformPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, CursorPos.x, CursorPos.y, 0, NativeWin, NULL))
		{
		switch (ConformPopResult)
			{
			case ID_DBEDIT_CONFORM_MENU_TOPO:
				{
				ConformTopo();
				break;
				} // terrain
			case ID_DBEDIT_CONFORM_MENU_NAME:
				{
				ConformName();
				break;
				} // name
			case ID_DBEDIT_CONFORM_MENU_LABEL:
				{
				ConformLabel();
				break;
				} // label
			case ID_DBEDIT_CONFORM_MENU_LAYER:
				{
				ConformLayer();
				break;
				} // layer
#ifdef WCS_BUILD_VNS
			case ID_DBEDIT_CONFORM_MENU_ATTRIB:
				{
				ConformAttribute();
				break;
				} // layer
#endif // WCS_BUILD_VNS
			default:
				break;
			} // switch
		} // if
	DestroyMenu(ConformPopupMenu);
	ConformPopupMenu = NULL;
	} // if

} // DBEditGUI::ConformVector

/*===========================================================================*/

void DBEditGUI::ConformTopo(void)
{
BusyWin *BWCF = NULL;
Joe *JoeBob, *LastDone;
NotifyTag Changes[2];
long DoneObjs = 0, NumItems, Item;
char MessageStr[128];

if (UserMessageOKCAN("Conform to Terrain", "Conform all selected Vectors and Control Points to terrain elevations?"))
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		BWCF = new BusyWin("Conforming", NumItems, 'BWCF', 0);
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (JoeBob = GetJoeFromOBN(Item))
					{
					if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (ConformOneObject(JoeBob))
							{
							LastDone = JoeBob;
							DoneObjs ++;
							} // if
						} // if not DEM
					} // if
				} // if enabled
			if (Item % 100 == 1) // don't update excessively
				{
				BWCF->Update(Item);
				if (BWCF->CheckAbort())
					{
					break; // bail out of for loop
					}
				} // if
			} // for
		if (DoneObjs == 1)
			UserMessageOK((char *)LastDone->GetBestName(), "Selected object has been conformed to the terrain.");
		else if (DoneObjs > 1)
			{
			sprintf(MessageStr, "%d selected Vector and Control Point objects have been conformed to the terrain.", DoneObjs);
			UserMessageOK("Conform to Terrain", MessageStr);
			} // else if
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			Host->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			Host->GenerateNotify(Changes);
			} // else
		Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			GlobalApp->AppEx->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			GlobalApp->AppEx->GenerateNotify(Changes);
			} // else
		} // if
	} // if

if (BWCF)
	delete BWCF;
BWCF = NULL;

} // DBEditGUI::ConformTopo

/*===========================================================================*/

void DBEditGUI::ConformName(void)
{
Joe *JoeBob, *LastDone;
long DoneObjs = 0, NumItems, Item;
char MessageStr[128];
NotifyTag Changes[2];
BusyWin *BWCF = NULL;

if (UserMessageOKCAN("Conform to Name", "Conform all selected Vectors and Control Point elevations to their names?"))
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		BWCF = new BusyWin("Conforming", NumItems, 'BWCF', 0);
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (JoeBob = GetJoeFromOBN(Item))
					{
					if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (ConformOneObjectName(JoeBob))
							{
							LastDone = JoeBob;
							DoneObjs ++;
							} // if
						} // if not DEM
					} // if
				} // if enabled
			if (Item % 100 == 1) // don't update excessively
				{
				BWCF->Update(Item);
				if (BWCF->CheckAbort())
					{
					break; // bail out of for loop
					}
				} // if
			} // for
		if (DoneObjs == 1)
			UserMessageOK((char *)LastDone->GetBestName(), "Selected object's elevation has been conformed to its name.");
		else if (DoneObjs > 1)
			{
			sprintf(MessageStr, "All %d selected Vector and Control Point elevations have been conformed to their names.", DoneObjs);
			UserMessageOK("Conform to Name", MessageStr);
			} // else if
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			Host->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			Host->GenerateNotify(Changes);
			} // else
		Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			GlobalApp->AppEx->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			GlobalApp->AppEx->GenerateNotify(Changes);
			} // else
		} // if
	} // if

if (BWCF)
	delete BWCF;
BWCF = NULL;

} // DBEditGUI::ConformName

/*===========================================================================*/

void DBEditGUI::ConformLabel(void)
{
Joe *JoeBob, *LastDone;
long DoneObjs = 0, NumItems, Item;
char MessageStr[128];
NotifyTag Changes[2];
BusyWin *BWCF = NULL;

if (UserMessageOKCAN("Conform to Label", "Conform all selected Vectors and Control Point elevations to their labels?"))
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		BWCF = new BusyWin("Conforming", NumItems, 'BWCF', 0);
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (JoeBob = GetJoeFromOBN(Item))
					{
					if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (ConformOneObjectLabel(JoeBob))
							{
							LastDone = JoeBob;
							DoneObjs ++;
							} // if
						} // if not DEM
					} // if
				} // if enabled
			if (Item % 100 == 1) // don't update excessively
				{
				BWCF->Update(Item);
				if (BWCF->CheckAbort())
					{
					break; // bail out of for loop
					}
				} // if
			} // for
		if (DoneObjs == 1)
			UserMessageOK((char *)LastDone->GetBestName(), "Selected object's elevation has been conformed to its label.");
		else if (DoneObjs > 1)
			{
			sprintf(MessageStr, "%d selected Vector and Control Point elevations have been conformed to their labels.", DoneObjs);
			UserMessageOK("Conform to Label", MessageStr);
			} // else if
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			Host->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			Host->GenerateNotify(Changes);
			} // else
		Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			GlobalApp->AppEx->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			GlobalApp->AppEx->GenerateNotify(Changes);
			} // else
		} // if
	} // if

if (BWCF)
	delete BWCF;
BWCF = NULL;


} // DBEditGUI::ConformLabel

/*===========================================================================*/

void DBEditGUI::ConformLayer(void)
{
Joe *JoeBob, *LastDone;
long DoneObjs = 0, NumItems, Item;
char MessageStr[128];
NotifyTag Changes[2];
BusyWin *BWCF = NULL;

if (UserMessageOKCAN("Conform to Layer", "Conform all selected Vectors and Control Point elevations to their layer names?"))
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		BWCF = new BusyWin("Conforming", NumItems, 'BWCF', 0);
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (JoeBob = GetJoeFromOBN(Item))
					{
					if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (ConformOneObjectLayer(JoeBob))
							{
							LastDone = JoeBob;
							DoneObjs ++;
							} // if
						} // if not DEM
					} // if
				} // if enabled
			if (Item % 100 == 1) // don't update excessively
				{
				BWCF->Update(Item);
				if (BWCF->CheckAbort())
					{
					break; // bail out of for loop
					}
				} // if
			} // for
		if (DoneObjs == 1)
			UserMessageOK((char *)LastDone->GetBestName(), "Selected object's elevation has been conformed to its layer name.");
		else if (DoneObjs > 1)
			{
			sprintf(MessageStr, "%d selected Vector and Control Point elevations have been conformed to their layer names.", DoneObjs);
			UserMessageOK("Conform to Layer", MessageStr);
			} // else if
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			Host->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			Host->GenerateNotify(Changes);
			} // else
		Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
		Changes[1] = NULL;
		if (DoneObjs == 1)
			{
			GlobalApp->AppEx->GenerateNotify(Changes, LastDone);
			} // if
		else
			{
			GlobalApp->AppEx->GenerateNotify(Changes);
			} // else
		} // if
	} // if

if (BWCF)
	delete BWCF;
BWCF = NULL;

} // DBEditGUI::ConformLayer

/*===========================================================================*/

void DBEditGUI::ConformAttribute(void)
{
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
long DoneObjs = 0, NumItems, Item;
long Current;
char DataInputBuf[200], MessageStr[256];
Joe *JoeBob, *LastDone;
LayerEntry *MyLayer = NULL;
NotifyTag Changes[2];
BusyWin *BWCF = NULL;

DataInputBuf[0] = NULL;

// test to see if attribute table is displayed
// read selected attribute and test to see if it is text or value
if ((Current = WidgetLBGetCurSel(IDC_LAYERLIST)) != LB_ERR)
	{
	if ((MyLayer = (LayerEntry *)WidgetLBGetItemData(IDC_LAYERLIST, Current)) && MyLayer != (LayerEntry *)LB_ERR)
		{
		if (MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE) && ! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
			{
			if (! MyLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
				{
				strcpy(DataInputBuf, MyLayer->GetName());

				if (UserMessageOKCAN(&DataInputBuf[1], "Conform all selected Vectors and Control Point elevations to the selected attribute's values?"))
					{
					if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
						{
						BWCF = new BusyWin("Conforming", NumItems, 'BWCF', 0);
						for (Item = 0; Item < NumItems; Item ++)
							{
							if (GetSelected(Item))
								{
								if (JoeBob = GetJoeFromOBN(Item))
									{
									if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
										{
										if (ConformOneObjectAttribute(JoeBob, MyLayer))
											{
											LastDone = JoeBob;
											DoneObjs ++;
											} // if
										} // if not DEM
									} // if
								} // if enabled
							if (Item % 100 == 1) // don't update excessively
								{
								BWCF->Update(Item);
								if (BWCF->CheckAbort())
									{
									break; // bail out of for loop
									}
								} // if
							} // for
						if (DoneObjs == 1)
							{
							sprintf(MessageStr, "Selected object's elevation has been conformed to the attribute %s.", &DataInputBuf[1]);
							UserMessageOK((char *)LastDone->GetBestName(), MessageStr);
							} // if
						else if (DoneObjs > 1)
							{
							sprintf(MessageStr, "%d selected Vector and Control Point elevations have been conformed to the attribute %s.", DoneObjs, &DataInputBuf[1]);
							UserMessageOK("Conform to Layer", MessageStr);
							} // else if
						Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, 0xff);
						Changes[1] = NULL;
						if (DoneObjs == 1)
							{
							Host->GenerateNotify(Changes, LastDone);
							} // if
						else
							{
							Host->GenerateNotify(Changes);
							} // else
						Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
						Changes[1] = NULL;
						if (DoneObjs == 1)
							{
							GlobalApp->AppEx->GenerateNotify(Changes, LastDone);
							} // if
						else
							{
							GlobalApp->AppEx->GenerateNotify(Changes);
							} // else
						} // if
					} // if
				} // if
			else
				UserMessageOK("Conform to Attribute", "Selected attribute is a text attribute. Attribute must be numeric.");
			} // if
		else
			UserMessageOK("Conform to Attribute", "A layer is selected, not an attribute. Please select the attribute you wish to conform to.");
		} // if
	} // if
else
	UserMessageOK("Conform to Attribute", "Please select the attribute you wish to conform to in the attribute list.");

if (BWCF)
	delete BWCF;
BWCF = NULL;
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

} // DBEditGUI::ConformAttribute

/*===========================================================================*/

unsigned long DBEditGUI::ConformOneObject(Joe *BillyJoe)
{

return (BillyJoe->ConformToTopo());

} // DBEditGUI::ConformOneObject

/*===========================================================================*/

unsigned long DBEditGUI::ConformOneObjectName(Joe *BillyJoe)
{
char *LName, *Temp;
VectorPoint *Point;
float PointElev;
int Found = 0;

if (LName = (char *)BillyJoe->FileName())
	{
	for (Temp = &LName[strlen(LName) - 1]; Temp >= LName; Temp --)
		{
		if (isdigit(*Temp))
			{
			Found = 1;
			break;
			} // if
		} // for
	if (Found)
		{
		PointElev = (float)atof(LName);
		if (BillyJoe->Points() && BillyJoe->NumPoints() > 0)
			{
			for (Point = BillyJoe->Points(); Point; Point = Point->Next)
				{
				Point->Elevation = PointElev;
				} // for
			return (BillyJoe->NumPoints());
			} // if points exist
		} // if
	} // if

return (0);

} // DBEditGUI::ConformOneObjectName

/*===========================================================================*/

unsigned long DBEditGUI::ConformOneObjectLabel(Joe *BillyJoe)
{
VectorPoint *Point;
float PointElev;
char *LName, *Temp;
int Found = 0;

if (LName = (char *)BillyJoe->Name())
	{
	for (Temp = &LName[strlen(LName) - 1]; Temp >= LName; Temp --)
		{
		if (isdigit(*Temp))
			{
			Found = 1;
			break;
			} // if
		} // for
	if (Found)
		{
		PointElev = (float)atof(LName);
		if (BillyJoe->Points() && BillyJoe->NumPoints() > 0)
			{
			for (Point = BillyJoe->Points(); Point; Point = Point->Next)
				{
				Point->Elevation = PointElev;
				} // for
			return (BillyJoe->NumPoints());
			} // if points exist
		} // if
	} // if

return (0);

} // DBEditGUI::ConformOneObjectLabel

/*===========================================================================*/

unsigned long DBEditGUI::ConformOneObjectLayer(Joe *BillyJoe)
{
VectorPoint *Point;
float PointElev;
char *LName, *Temp;
int Found = 0;
LayerStub *Stub;

for (Stub = BillyJoe->FirstLayer(); Stub && ! Found; Stub = BillyJoe->NextLayer(Stub))
	{
	if (Stub->MyLayer() && (LName = (char *)Stub->MyLayer()->GetName()))
		{
		for (Temp = &LName[strlen(LName) - 1]; Temp >= LName; Temp --)
			{
			if (isdigit(*Temp))
				{
				Found = 1;
				break;
				} // if
			} // for
		} // if
	} // for

if (Found)
	{
	PointElev = (float)atof(LName);
	if (BillyJoe->Points() && BillyJoe->NumPoints() > 0)
		{
		for (Point = BillyJoe->Points(); Point; Point = Point->Next)
			{
			Point->Elevation = PointElev;
			} // for
		return (BillyJoe->NumPoints());
		} // if points exist
	} // if

return (0);

} // DBEditGUI::ConformOneObjectLayer

/*===========================================================================*/

unsigned long DBEditGUI::ConformOneObjectAttribute(Joe *BillyJoe, LayerEntry *MyLayer)
{
VectorPoint *Point;
float PointElev;
LayerStub *Stub;

if (Stub = BillyJoe->MatchLayer(MyLayer))
	{
	PointElev = (float)Stub->GetIEEEAttribVal();
	if (BillyJoe->Points() && BillyJoe->NumPoints() > 0)
		{
		for (Point = BillyJoe->Points(); Point; Point = Point->Next)
			{
			Point->Elevation = PointElev;
			} // for
		return (BillyJoe->NumPoints());
		} // if points exist
	} // if

return (0);

} // DBEditGUI::ConformOneObjectAttribute

/*===========================================================================*/

void DBEditGUI::SplitVector(void)
{
long SplitHow, Success = 1;
unsigned long USplitPt, SplitPt, NumPts1, NumPts2, PtCt, IsAttribute;
VectorPoint *PLink, *DestLink, *LastLink, *NewPoints1 = NULL, *NewPoints2 = NULL;
Joe *SplitObj,  *JoeBob;
JoeAttribute *CurAttr;
GeneralEffect *CurEffect;
LayerEntry *CurLayer;
LayerStub *CurStub;
NotifyTag Changes[2];
char SplitPtTxt[256];

// find active joe
if (SplitObj = Host->ActiveObj)
	{
	// is it a vector
	if (! SplitObj->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		// find split point
		// find first currently selected point in 0-based notation
		if (SplitObj->GetNumRealPoints() > 1)
			{
			USplitPt = 0;
			GlobalApp->MainProj->Interactive->GetFirstSelectedPt(USplitPt);
			SplitPt = USplitPt;
			// remove label point, still 0-based
			if (SplitPt >= Joe::GetFirstRealPtNum())
				SplitPt -= Joe::GetFirstRealPtNum();	// subtract label point
			if (SplitPt == 0)
				SplitPt = (SplitObj->GetNumRealPoints()) / 2;

			// ask for vertex number to split at in 1-based notation
			sprintf(SplitPtTxt, "%d", SplitPt + 1);
			if (GetInputString("Enter the vertex number at which to split the Database Object", WCS_REQUESTER_POSINTEGERS_ONLY, SplitPtTxt))
				{
				SplitPt = atoi(SplitPtTxt);
				// return to 0-based notation
				if (SplitPt > 0)
					SplitPt --;
				if (SplitPt < SplitObj->GetNumRealPoints())
					{
					// restore label point, 0-based
					SplitPt += Joe::GetFirstRealPtNum();	// restore label point
					// ask if split at, after or before split vertex
					// point is not label point nor first vertex, nor last vertex
					if (SplitPt > Joe::GetFirstRealPtNum() && SplitPt < SplitObj->NumPoints() - 1)
						{
						SplitHow = UserMessageCustom((char *)SplitObj->GetBestName(), "Split object at, after or before the chosen point?", "At Point", "Before Point", "After Point", 0);
						} // if
					else if (SplitPt == SplitObj->NumPoints() - 1)
						{
						SplitHow = UserMessageCustom((char *)SplitObj->GetBestName(), "Split object at or before the chosen point?", "At Point", "Before Point", NULL, 0);
						} // else if
					else
						{
						SplitHow = UserMessageCustom((char *)SplitObj->GetBestName(), "Split object at or after the chosen point?", "At Point", "After Point", NULL, 0);
						SplitHow = SplitHow ? 1: 2;
						} // else
					// create two new sets of vertices
					// add 1 to correct number for 0-basedness of SplitPt
					NumPts1 = SplitPt + 1;
					// add label point
					NumPts2 = SplitObj->NumPoints() - SplitPt + Joe::GetFirstRealPtNum();
					if (SplitHow == 0)	// before
						NumPts1 --;
					else if (SplitHow == 2)	// after
						NumPts2 --;
						
					if (NumPts1 > 0 && NumPts2 > 0)
						{
						if ((NewPoints1 = Host->MasterPoint.Allocate(NumPts1)) && (NewPoints2 = Host->MasterPoint.Allocate(NumPts2)))
							{
							// copy points
							LastLink = NULL;
							for (PtCt = 0, PLink = SplitObj->Points(), DestLink = NewPoints1; PLink && DestLink && PtCt < NumPts1; PtCt ++, PLink = PLink->Next, DestLink = DestLink->Next)
								{
								DestLink->Latitude = PLink->Latitude;
								DestLink->Longitude = PLink->Longitude;
								DestLink->Elevation = PLink->Elevation;
								LastLink = PLink;
								} // for
							PLink = SplitHow == 1 ? LastLink: PLink;
							for (PtCt = 0, DestLink = (Joe::GetFirstRealPtNum() > 0 ? NewPoints2->Next: NewPoints2); PLink && DestLink && PtCt < NumPts2; PtCt ++, PLink = PLink->Next, DestLink = DestLink->Next)
								{
								DestLink->Latitude = PLink->Latitude;
								DestLink->Longitude = PLink->Longitude;
								DestLink->Elevation = PLink->Elevation;
								} // for
							#ifdef WCS_JOE_LABELPOINTEXISTS
							NewPoints2->Latitude = NewPoints2->Next->Latitude;
							NewPoints2->Longitude = NewPoints2->Next->Longitude;
							NewPoints2->Elevation = NewPoints2->Next->Elevation;
							#endif // WCS_JOE_LABELPOINTEXISTS

							// create two new vectors
							for (PtCt = 0; PtCt < 2; PtCt ++)
								{
								strcpy(SplitPtTxt, SplitObj->GetBestName());
								strcat(SplitPtTxt, PtCt == 0 ? "_1": "_2");
								if (JoeBob = Host->NewObject(ProjHost, SplitPtTxt))
									{
									// add new points
									JoeBob->Points(PtCt == 0 ? NewPoints1: NewPoints2);
									JoeBob->NumPoints(PtCt == 0 ? NumPts1: NumPts2);
									
									// ensure split children have same enable/view/render flag state as original
									if (SplitObj->TestFlags(WCS_JOEFLAG_ACTIVATED))
										JoeBob->SetFlags(WCS_JOEFLAG_ACTIVATED);
									else
										JoeBob->ClearFlags(WCS_JOEFLAG_ACTIVATED);

									if (SplitObj->TestFlags(WCS_JOEFLAG_DRAWENABLED))
										JoeBob->SetFlags(WCS_JOEFLAG_DRAWENABLED);
									else
										JoeBob->ClearFlags(WCS_JOEFLAG_DRAWENABLED);

									if (SplitObj->TestFlags(WCS_JOEFLAG_RENDERENABLED))
										JoeBob->SetFlags(WCS_JOEFLAG_RENDERENABLED);
									else
										JoeBob->ClearFlags(WCS_JOEFLAG_RENDERENABLED);


									// add attributes, layers and effects
									for (CurAttr = SplitObj->GetFirstAttribute(); CurAttr; CurAttr = CurAttr->GetNextAttribute())
										{
										if (CurAttr->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && CurAttr->MinorAttrib() >= WCS_EFFECTSSUBCLASS_LAKE &&
											CurAttr->MinorAttrib() < WCS_MAXIMPLEMENTED_EFFECTS)
											{
											if (CurEffect = SplitObj->GetAttributeEffect(CurAttr))
												{
												// already added CoordSys above
												JoeBob->AddEffect(CurEffect, 1);
												} // if
											} // if
										} // for
									for (CurStub = SplitObj->FirstLayer(); CurStub; CurStub = SplitObj->NextLayer(CurStub))
										{
										if (CurLayer = CurStub->MyLayer())
											{
											IsAttribute = CurLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
											#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
											if (IsAttribute)
												{
												if (! CurLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
													{
													if (CurLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
														{
														JoeBob->AddTextAttribute(CurLayer, CurStub->GetTextAttribVal());
														} // if
													else
														{
														JoeBob->AddIEEEAttribute(CurLayer, CurStub->GetIEEEAttribVal());
														} // else
													} // if not link
												} // if
											#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
											if (! IsAttribute)
												{
												JoeBob->AddObjectToLayer(CurLayer);
												} // if
											} // if
										} // for

									// set new object active
									Changes[0] = MAKE_ID(JoeBob->GetNotifyClass(), JoeBob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
									Changes[1] = NULL;
									GlobalApp->AppEx->GenerateNotify(Changes, JoeBob);
									RasterAnimHost::SetActiveRAHost(JoeBob);
									// send points changed message
									Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
									Host->GenerateNotify(Changes);
									Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
									GlobalApp->AppEx->GenerateNotify(Changes);
									} // if
								else
									{
									Success = 0;
									UserMessageOK("Split Vector", "Unable to create new object. Operation terminated.");
									break;
									} // else
								} // for
							// disable original vector
							if (Success && UserMessageYN("Split Vector", "Disable original object?"))
								{
								SplitObj->ClearFlags(WCS_JOEFLAG_ACTIVATED);
								if (SplitObj->TestFlags(WCS_JOEFLAG_ISCONTROL))
									Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
								else
									Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
								GlobalApp->AppEx->GenerateNotify(Changes, NULL);
								} // if
							} // if
						else
							Success = 0;
						} // if
					else
						UserMessageOK("Split Vector", "Method chosen results in 0 points in one of the new objects. Operation terminated.");
					} // if
				else
					UserMessageOK((char *)SplitObj->GetBestName(), "Point chosen is outside the range of points in the object.");
				} // if
			} // if
		else
			UserMessageOK((char *)SplitObj->GetBestName(), "Currently active Database Object does not have enough points to be split.");
		} // if
	else
		UserMessageOK((char *)SplitObj->GetBestName(), "Currently active Database Object is a DEM and can not be split.");
	} // if
else
	UserMessageOK("Split Vector", "There is no currently active Database Object.");

if (! Success)
	{
	if (NewPoints1)
		Host->MasterPoint.DeAllocate(NewPoints1);
	if (NewPoints2)
		Host->MasterPoint.DeAllocate(NewPoints2);
	} // if

} // DBEditGUI::SplitVector

/*===========================================================================*/

void DBEditGUI::JoinVectors(void)
{

Joe *JoeBob, **JoinList = NULL, **JoinList2 = NULL;
long JoinObjs = 0, JoinCt, NumVerts, PtCt, NumItems, Item, PointsToMake, Success = 1, TempFound1, TempFound2, 
	MoreToDo, ControlPtsChanged, VectorsChanged, *OrderList = NULL;
unsigned long IsAttribute;
double Dist, MaxDist, MinDist, LocalMinDist1, LocalMinDist2, PlanetRad;
char *FlagList = NULL, TempDir1, TempDir2;
VectorPoint **LastPoint = NULL, **FirstPoint = NULL,  **LastPoint2 = NULL, **FirstPoint2 = NULL, **VertList = NULL,
	*PLink, *DestLink, *NewPoints;
JoeCoordSys *MyAttr;
JoeAttribute *CurAttr;
CoordSys *HostCoords, *SourceCoords;
GeneralEffect *CurEffect;
LayerEntry *CurLayer;
LayerStub *CurStub;
VertexDEM Vert, Vert2;
NotifyTag Changes[2];

if (UserMessageOKCAN("Join Vectors", "Join all selected Vectors or Control Point objects and form a new one?"))
	{
	if (NumItems = ListView_GetItemCount(GetWidgetFromID(IDC_LIST1)))
		{
		for (Item = 0; Item < NumItems; Item ++)
			{
			if (GetSelected(Item))
				{
				if (JoeBob = GetJoeFromOBN(Item))
					{
					if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (JoeBob->GetNumRealPoints() > 0)
							JoinObjs ++;
						} // if not DEM
					} // if
				} // if enabled
			} // for
		if (JoinObjs > 0 && (JoinList = (Joe **)AppMem_Alloc(JoinObjs * sizeof (Joe *), APPMEM_CLEAR))
			&& (JoinList2 = (Joe **)AppMem_Alloc(JoinObjs * sizeof (Joe *), APPMEM_CLEAR))
			&& (FlagList = (char *)AppMem_Alloc(JoinObjs, APPMEM_CLEAR))
			&& (LastPoint = (VectorPoint **)AppMem_Alloc(JoinObjs * sizeof (VectorPoint *), APPMEM_CLEAR))
			&& (FirstPoint = (VectorPoint **)AppMem_Alloc(JoinObjs * sizeof (VectorPoint *), APPMEM_CLEAR))
			&& (LastPoint2 = (VectorPoint **)AppMem_Alloc(JoinObjs * sizeof (VectorPoint *), APPMEM_CLEAR))
			&& (FirstPoint2 = (VectorPoint **)AppMem_Alloc(JoinObjs * sizeof (VectorPoint *), APPMEM_CLEAR))
			&& (OrderList = (long *)AppMem_Alloc(JoinObjs * sizeof (long), APPMEM_CLEAR)))
			{
			for (JoinCt = Item = 0; Item < NumItems && JoinCt < JoinObjs; Item ++)
				{
				if (GetSelected(Item))
					{
					if (JoeBob = GetJoeFromOBN(Item))
						{
						if (! JoeBob->TestFlags(WCS_JOEFLAG_ISDEM))
							{
							if (JoeBob->GetNumRealPoints() > 0)
								{
								JoinList2[JoinCt] = JoeBob;
								FirstPoint2[JoinCt] = JoeBob->GetFirstRealPoint();
								PLink = FirstPoint2[JoinCt];
								while (PLink->Next)
									PLink = PLink->Next;
								LastPoint2[JoinCt] = PLink;
								JoinCt ++;
								} // if
							} // if not DEM
						} // if
					} // if enabled
				} // for

			// determine order of connection
			// find first vector and its direction by looking for the end vertex farthest from all other end vertices
			MaxDist = -FLT_MAX;
			PlanetRad = HostLib->GetPlanetRadius();
			for (JoinCt = 0; JoinCt < JoinObjs; JoinCt ++)
				{
				if (MyAttr = (JoeCoordSys *)JoinList2[JoinCt]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					HostCoords = MyAttr->Coord;
				else
					HostCoords = NULL;
				LocalMinDist1 = LocalMinDist2 = FLT_MAX;
				for (PtCt = 0; PtCt < JoinObjs; PtCt ++)
					{
					if (PtCt == JoinCt)
						continue;
					if (MyAttr = (JoeCoordSys *)JoinList2[PtCt]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
						SourceCoords = MyAttr->Coord;
					else
						SourceCoords = NULL;

					// test first point to first point
					FirstPoint2[JoinCt]->ProjToDefDeg(HostCoords, &Vert);					
					FirstPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);					
					Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);
					if (Dist < LocalMinDist1)
						{
						LocalMinDist1 = Dist;
						TempFound1 = JoinCt;
						TempDir1 = 1;
						} // if
					// test first point to last point
					if (LastPoint2[PtCt] != FirstPoint2[PtCt])
						{
						LastPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);					
						Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);
						if (Dist < LocalMinDist1)
							{
							LocalMinDist1 = Dist;
							TempFound1 = JoinCt;
							TempDir1 = 1;
							} // if
						} // if

					// test last point to first point
					LastPoint2[JoinCt]->ProjToDefDeg(HostCoords, &Vert);					
					FirstPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);					
					Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);
					if (Dist < LocalMinDist2)
						{
						LocalMinDist2 = Dist;
						TempFound2 = JoinCt;
						TempDir2 = 0;
						} // if
					// test last point to last point
					if (LastPoint2[PtCt] != FirstPoint2[PtCt])
						{
						LastPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);					
						Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);
						if (Dist < LocalMinDist2)
							{
							LocalMinDist2 = Dist;
							TempFound2 = JoinCt;
							TempDir2 = 0;
							} // if
						} // if
					} // for
				if (LocalMinDist1 >= LocalMinDist2)
					{
					if (LocalMinDist1 > MaxDist || (LocalMinDist1 == MaxDist && TempDir1 && ! FlagList[0]))
						{
						MaxDist = LocalMinDist1;
						JoinList[0] = JoinList2[TempFound1];
						OrderList[0] = TempFound1;
						FlagList[0] = TempDir1;
						} // if found a new largest minimum distance
					} // if first point is closest to another vector
				else
					{
					if (LocalMinDist2 > MaxDist || (LocalMinDist2 == MaxDist && TempDir2 && ! FlagList[0]))
						{
						MaxDist = LocalMinDist2;
						JoinList[0] = JoinList2[TempFound2];
						OrderList[0] = TempFound2;
						FlagList[0] = TempDir2;
						} // if found a new largest minimum distance
					} // else if last point is closest to another vector
				} // for

			JoinList2[OrderList[0]] = NULL;

			// find order of rest of vectors
			MoreToDo = JoinObjs - 1;
			for (JoinCt = 1; JoinCt < JoinObjs && MoreToDo; JoinCt ++)
				{
				MoreToDo = 0;
				if (MyAttr = (JoeCoordSys *)JoinList[JoinCt - 1]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					HostCoords = MyAttr->Coord;
				else
					HostCoords = NULL;
				if (FlagList[JoinCt - 1])
					LastPoint2[OrderList[JoinCt - 1]]->ProjToDefDeg(HostCoords, &Vert);					
				else
					FirstPoint2[OrderList[JoinCt - 1]]->ProjToDefDeg(HostCoords, &Vert);
				MinDist = FLT_MAX;				
				for (PtCt = 0; PtCt < JoinObjs; PtCt ++)
					{
					if (! JoinList2[PtCt])
						continue;
					if (MyAttr = (JoeCoordSys *)JoinList2[PtCt]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
						SourceCoords = MyAttr->Coord;
					else
						SourceCoords = NULL;

					MoreToDo ++;
					FirstPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);	
					Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);				
					if (Dist < MinDist)
						{
						MinDist = Dist;
						JoinList[JoinCt] = JoinList2[PtCt];
						OrderList[JoinCt] = PtCt;
						FlagList[JoinCt] = 1;
						} // if
					if (LastPoint2[PtCt] != FirstPoint2[PtCt])
						{
						LastPoint2[PtCt]->ProjToDefDeg(SourceCoords, &Vert2);
						Dist = FindDistance(Vert.Lat, Vert.Lon, Vert2.Lat, Vert2.Lon, PlanetRad);					
						if (Dist < MinDist)
							{
							MinDist = Dist;
							JoinList[JoinCt] = JoinList2[PtCt];
							OrderList[JoinCt] = PtCt;
							FlagList[JoinCt] = 0;
							} // if
						} // if
					} // for
				MoreToDo --;	// subtract the one just linked
				JoinList2[OrderList[JoinCt]] = NULL;
				} // for

			// sort JoinList according to joining order
			for (JoinCt = 0; JoinCt < JoinObjs; JoinCt ++)
				{
				// copy list pointers
				FirstPoint[JoinCt] = FirstPoint2[OrderList[JoinCt]];
				LastPoint[JoinCt] = LastPoint2[OrderList[JoinCt]];
				} // for

			// count points to create and remove duplicates at vector ends
			PointsToMake = JoinList[0]->GetNumRealPoints();
			for (JoinCt = 1; JoinCt < JoinObjs; JoinCt ++)
				{
				PointsToMake += JoinList[JoinCt]->GetNumRealPoints();
				if (FlagList[JoinCt])	// forward
					{
					if (FlagList[JoinCt - 1])	// forward
						{
						// compare last point of last vector to first point of this vector
						if (LastPoint[JoinCt - 1]->SamePoint(FirstPoint[JoinCt]))
							{
							FirstPoint[JoinCt] = FirstPoint[JoinCt]->Next;
							PointsToMake --;
							} // if
						} // if same direction
					else	// reverse
						{
						// compare first point of last vector to first point of this vector
						if (FirstPoint[JoinCt - 1]->SamePoint(FirstPoint[JoinCt]))
							{
							FirstPoint[JoinCt] = FirstPoint[JoinCt]->Next;
							PointsToMake --;
							} // if
						} // if
					} // if
				else	// reverse
					{
					if (FlagList[JoinCt - 1])	// forward
						{
						// compare last point of last vector to last point of this vector
						if (LastPoint[JoinCt - 1]->SamePoint(LastPoint[JoinCt]))
							{
							PLink = FirstPoint[JoinCt];
							while (PLink->Next && PLink->Next != LastPoint[JoinCt])
								PLink = PLink->Next;
							LastPoint[JoinCt] = PLink;
							PointsToMake --;
							} // if
						} // if same direction
					else	// reverse
						{
						// compare first point of last vector to last point of this vector
						if (FirstPoint[JoinCt - 1]->SamePoint(LastPoint[JoinCt]))
							{
							PLink = FirstPoint[JoinCt];
							while (PLink->Next && PLink->Next != LastPoint[JoinCt])
								PLink = PLink->Next;
							LastPoint[JoinCt] = PLink;
							PointsToMake --;
							} // if
						} // if
					} // else
				} // for

			// create points
			if (PointsToMake > 0 && (NewPoints = Host->MasterPoint.Allocate(PointsToMake + Joe::GetFirstRealPtNum())))
				{
				// find coord sys for first vector
				if (MyAttr = (JoeCoordSys *)JoinList[0]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					HostCoords = MyAttr->Coord;
				else
					HostCoords = NULL;
				// copy points to new array

				#ifdef WCS_JOE_LABELPOINTEXISTS
				DestLink = NewPoints->Next;
				#else // WCS_JOE_LABELPOINTEXISTS
				DestLink = NewPoints;
				#endif // WCS_JOE_LABELPOINTEXISTS

				for (JoinCt = 0; JoinCt < JoinObjs; JoinCt ++)
					{
					// if FirstPoint is NULL then this vector was elliminated by duplicate point reduction above
					if (FirstPoint[JoinCt])
						{
						// find coord sys for current vector
						if (MyAttr = (JoeCoordSys *)JoinList[JoinCt]->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
							SourceCoords = MyAttr->Coord;
						else
							SourceCoords = NULL;

						if (FlagList[JoinCt])	// forward
							{
							for (PLink = FirstPoint[JoinCt]; PLink && DestLink; PLink = PLink->Next, DestLink = DestLink->Next)
								{
								DestLink->Latitude = PLink->Latitude;
								DestLink->Longitude = PLink->Longitude;
								DestLink->Elevation = PLink->Elevation;
								// coordinate transformation
								if (HostCoords != SourceCoords)
									{
									DestLink->ProjToDefDeg(SourceCoords, &Vert);
									DestLink->DefDegToProj(HostCoords, &Vert);
									} // if
								if (PLink == LastPoint[JoinCt])
									{
									DestLink = DestLink->Next;
									break;
									} // if
								} // for
							} // if
						else	// reverse
							{
							// count vertices
							for (NumVerts = 0, PLink = FirstPoint[JoinCt]; PLink; PLink = PLink->Next)
								{
								NumVerts ++;
								if (PLink == LastPoint[JoinCt])
									break;
								} // for
							// create an array of vertices in reverse order
							if (VertList = (VectorPoint **)AppMem_Alloc(NumVerts * sizeof (VectorPoint *), 0))
								{
								// copy vertex pointers to new list
								for (PtCt = NumVerts - 1, PLink = FirstPoint[JoinCt]; PLink && PtCt >= 0; PtCt --, PLink = PLink->Next)
									{
									VertList[PtCt] = PLink;
									if (PLink == LastPoint[JoinCt])
										{
										break;
										} // if
									} // for
								// copy vertices to destination linked list
								for (PtCt = 0; PtCt < NumVerts && DestLink; PtCt ++, DestLink = DestLink->Next)
									{
									DestLink->Latitude = VertList[PtCt]->Latitude;
									DestLink->Longitude = VertList[PtCt]->Longitude;
									DestLink->Elevation = VertList[PtCt]->Elevation;
									// coordinate transformation
									if (HostCoords != SourceCoords)
										{
										DestLink->ProjToDefDeg(SourceCoords, &Vert);
										DestLink->DefDegToProj(HostCoords, &Vert);
										} // if
									} // for
								AppMem_Free(VertList, NumVerts * sizeof (VectorPoint *));
								} // if
							else
								{
								UserMessageOK("Join Vectors", "Unable to allocate temporary point list. Operation terminated.");
								Success = 0;
								break;
								} // else
							} // else
						} // if
					} // for

				#ifdef WCS_JOE_LABELPOINTEXISTS
				// set label point
				NewPoints->Latitude = NewPoints->Next->Latitude;
				NewPoints->Longitude = NewPoints->Next->Longitude;
				NewPoints->Elevation = NewPoints->Next->Elevation;
				#endif // WCS_JOE_LABELPOINTEXISTS

				// create new object
				if (JoeBob = Host->NewObject(ProjHost))
					{
					// add new points
					JoeBob->Points(NewPoints);
					JoeBob->NumPoints(PointsToMake + Joe::GetFirstRealPtNum());

					if (HostCoords)
						{
						JoeBob->AddEffect(HostCoords, 1);
						} // if
					else
						{
						JoeBob->ClearFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
						JoeBob->RecheckBounds();
						JoeBob->ZeroUpTree();
						Host->ReBoundTree(WCS_DATABASE_STATIC);
						} // else
					// add attributes, layers and effects
					for (CurAttr = JoinList[0]->GetFirstAttribute(); CurAttr; CurAttr = CurAttr->GetNextAttribute())
						{
						if (CurAttr->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && CurAttr->MinorAttrib() >= WCS_EFFECTSSUBCLASS_LAKE &&
							CurAttr->MinorAttrib() < WCS_MAXIMPLEMENTED_EFFECTS)
							{
							if (CurEffect = JoinList[0]->GetAttributeEffect(CurAttr))
								{
								// already added CoordSys above
								if (CurEffect->EffectType != WCS_EFFECTSSUBCLASS_COORDSYS)
									JoeBob->AddEffect(CurEffect, 1);
								} // if
							} // if
						} // for
					for (CurStub = JoinList[0]->FirstLayer(); CurStub; CurStub = JoinList[0]->NextLayer(CurStub))
						{
						if (CurLayer = CurStub->MyLayer())
							{
							IsAttribute = CurLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
							#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
							if (IsAttribute)
								{
								if (! CurLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
									{
									if (CurLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
										{
										JoeBob->AddTextAttribute(CurLayer, CurStub->GetTextAttribVal());
										} // if
									else
										{
										JoeBob->AddIEEEAttribute(CurLayer, CurStub->GetIEEEAttribVal());
										} // else
									} // if not link
								} // if
							#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
							if (! IsAttribute)
								{
								JoeBob->AddObjectToLayer(CurLayer);
								} // if
							} // if
						} // for

					// set new object active
					Changes[0] = MAKE_ID(JoeBob->GetNotifyClass(), JoeBob->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, JoeBob);
					RasterAnimHost::SetActiveRAHost(JoeBob);
					// send points changed message
					Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_POINTS, WCS_NOTIFYDBASECHANGE_POINTS_NOCONFORM);
					Host->GenerateNotify(Changes);
					Changes[0] = MAKE_ID(WCS_INTERCLASS_VECTOR, WCS_INTERVEC_SUBCLASS_OPERATE, WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV, 0);
					GlobalApp->AppEx->GenerateNotify(Changes);

					// disable original vectors
					if (UserMessageYN("Join Vectors", "Disable original objects?"))
						{
						ControlPtsChanged = VectorsChanged = 0;
						for (JoinCt = 0; JoinCt < JoinObjs; JoinCt ++)
							{
							JoinList[JoinCt]->ClearFlags(WCS_JOEFLAG_ACTIVATED);
							if (JoinList[JoinCt]->TestFlags(WCS_JOEFLAG_ISCONTROL))
								ControlPtsChanged = 1;
							else
								VectorsChanged = 1;
							} // for
						if (ControlPtsChanged)
							{
							Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
							GlobalApp->AppEx->GenerateNotify(Changes, NULL);
							} // if
						if (VectorsChanged)
							{
							Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
							GlobalApp->AppEx->GenerateNotify(Changes, NULL);
							} // if
						} // if
					} // if
				else
					{
					Host->MasterPoint.DeAllocate(NewPoints);
					UserMessageOK("Join Vectors", "Unable to create new object. Operation terminated.");
					} // else
				} // if
			else
				UserMessageOK("Join Vectors", "Unable to allocate new point list. Operation terminated.");
			} // if
		if (JoinList)
			AppMem_Free(JoinList, JoinObjs * sizeof (Joe *));
		if (JoinList2)
			AppMem_Free(JoinList2, JoinObjs * sizeof (Joe *));
		if (FirstPoint)
			AppMem_Free(FirstPoint, JoinObjs * sizeof (Joe *));
		if (LastPoint)
			AppMem_Free(LastPoint, JoinObjs * sizeof (Joe *));
		if (FirstPoint2)
			AppMem_Free(FirstPoint2, JoinObjs * sizeof (Joe *));
		if (LastPoint2)
			AppMem_Free(LastPoint2, JoinObjs * sizeof (Joe *));
		if (FlagList)
			AppMem_Free(FlagList, JoinObjs);
		if (OrderList)
			AppMem_Free(OrderList, JoinObjs * sizeof (long));
		} // if
	} // if


} // DBEditGUI::JoinVectors

/*===========================================================================*/
/*===========================================================================*/

int Database::ExportShape(Project *Proj, JoeApproveHook *JAH, CoordSys *OutCoords, double ExportXYScale, double ExportZScale, long FlipLon,
						  long ViaSX, long Force2D, double AddedElev, RasterBounds *RBounds, long SXType, char *sxpath, char *sxname)
{
double NBound, SBound, EBound, WBound;
double MaxVal[3], MinVal[3], Zero = 0.0;
double dWriteVal, LonDir;
int Success = 1;
Joe *CurrentJoe;
long Clipping = ViaSX;
BOOL GeoOutput = false;
DEM Topo;
LayerEntry *CurrentLayer;
VectorPoint *PLink, VP;
FILE *fDBase = NULL, *fIndex = NULL, *fPrj = NULL, *fShape = NULL;
long ObjectsFound = 0;
long ShapeType;
unsigned long db3recs = 0, RecordNum = 1, DB3_Fields = 0;
long db3recbytes, bound_pos, lWriteVal, numpts, rec_start, rec_size;
unsigned short FieldLength = 1;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM Vert;
BusyWin *BWDX = NULL, *BWDY = NULL;
int NumWrite = 0;
short Proceed;
struct DB3_Table_Field_Descriptor *ExportFields = NULL, *LastField = NULL, *ScanFields;
unsigned char headertemplate[] = 
	{
	0x00, 0x00, 0x27, 0x0A,	// File Code (Big-Endian 9994)
	0x00, 0x00, 0x00, 0x00,	// unused
	0x00, 0x00, 0x00, 0x00,	// unused
	0x00, 0x00, 0x00, 0x00,	// unused
	0x00, 0x00, 0x00, 0x00,	// unused
	0x00, 0x00, 0x00, 0x00,	// unused
	0x00, 0x00, 0x00, 0x00,	// File Length in words (fixed later)
	0xE8, 0x03, 0x00, 0x00,	// Version (Little-Endian 1000)
	0x00, 0x00, 0x00, 0x00	// Shape Type (fixed later)
	};
unsigned char shape3d;
//const char *objname;
BOOL changedNames = FALSE, FoundLines = FALSE, FoundPoints = FALSE, WriteLines = FALSE;
char OutPath[256], OutName[64], Ptrn[32], filename[256];

MaxVal[0] = MaxVal[1] = MaxVal[2] = -FLT_MAX;
MinVal[0] = MinVal[1] = MinVal[2] = FLT_MAX;
if (FlipLon)
	LonDir = -1.0;
else
	LonDir = 1.0;

strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("shp"));
if (Proj->dirname[0])
	strcpy(OutPath, Proj->dirname);
else if (Proj->projectpath[0])
	strcpy(OutPath, Proj->projectpath);
else
	strcpy(OutPath, "WCSProjects:");
if (Proj->projectname[0])
	strcpy(OutName, Proj->projectname);
else
	strcpy(OutName, "NoName");
(void)StripExtension(OutName);

if (OutCoords->GetGeographic())
	{
	// change to WCS convention
	GeoOutput = true;
	} // if

if (ViaSX)
	{
	if (Force2D)
		{
		shape3d = 0;
		} // if
	else
		{
		shape3d = 1;
		} // else
	Proceed = 1;
	strcpy(OutPath, sxpath);
	strcpy(OutName, sxname);
	strmfp(filename, OutPath, OutName);

	// set up export clipping bounds
	VP.Longitude = RBounds->ULcenter.x;
	VP.Latitude = RBounds->ULcenter.y;
	VP.Elevation = 0.0f;
	VP.ProjToDefDeg(OutCoords, &Vert);
	NBound = Vert.Lat;
	WBound = Vert.Lon;

	VP.Longitude = RBounds->LRcenter.x;
	VP.Latitude = RBounds->LRcenter.y;
	VP.Elevation = 0.0f;
	VP.ProjToDefDeg(OutCoords, &Vert);
	SBound = Vert.Lat;
	EBound = Vert.Lon;

	if (OutCoords->GetGeographic())
		{
		// change to WCS convention
		WBound *= -1.0;
		EBound *= -1.0;
		} // if
	} // if
else
	{
	shape3d = UserMessageCustom("Export Shapefile",
							"Shape type?", "3d (PolyLineZ/PointZ)", "2d (PolyLine/Point)", NULL, 1);	// 3d = 1, 2d = 0
	Proceed = GetFileNamePtrn(1, "Export Shapefile", OutPath, OutName, Ptrn, 64);
	} // else

if (Proceed)
	{
	if ( !ViaSX)
		{
		char *ext;

		ext = FindFileExtension(OutName);
		if (ext)
			{
			if (stricmp(ext, "shp") == 0)
				StripExtension(OutName);
			} // if ext
		strmfp(filename, OutPath, OutName);
		} // if !ViaSX
	strcat(filename, ".shx");
	fIndex = PROJ_fopen(filename, "wb");
	strmfp(filename, OutPath, OutName);
	strcat(filename, ".dbf");
	fDBase = PROJ_fopen(filename, "wb");
	strmfp(filename, OutPath, OutName);
	strcat(filename, ".shp");
	fShape = PROJ_fopen(filename, "wb");
	if (fIndex && fDBase && fShape)
		{
		strcpy(Proj->dbasepath, OutPath);
		strcpy(Proj->dbasename, OutName);
		(void)StripExtension(Proj->dbasename);
		ResetGeoClip();
		// first determine the object extents for the header
		for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
			{
			if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				JAH->ApproveMe = CurrentJoe;
				if (JAH->Approve(JAH))
					{
					NumWrite++;
					//objname = CurrentJoe->GetBestName();
					if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						} // if
					else
						{
						ObjectsFound++;
						if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
							{
							FoundPoints = TRUE;
							} // if
						else if (CurrentJoe->Points() && CurrentJoe->GetNumRealPoints() > 0)
							{
							if (CurrentJoe->GetLineStyle() >= 4)
								FoundLines = TRUE;
							else
								FoundPoints = TRUE;
							} // else if
						} // else
					} // if
				} // if
			} // for CurrentJoe

		if (ObjectsFound || ViaSX)	// need to write correct header for empty Shapefiles
			{
			if (ViaSX)
				{
				WriteLines = SXType;
				} // if
			else
				{
				if (FoundLines && FoundPoints)
					{
					WriteLines = UserMessageCustom("Export Shapefile", "Both points and lines were found in the database.\r\rChoose 1 type for export.",
						"Lines", "Points", NULL, 1);	// lines = 1, points = 0
					} // if
				else if (FoundLines)
					{
					WriteLines = TRUE;
					} // else if
				} // else
			BWDX = new BusyWin("Writing", NumWrite, 'BWDX', 0);
			NumWrite = 0;
			if (FlipLon)
				{
				MaxVal[0] *= -1.0;
				MinVal[0] *= -1.0;
				Swap64(MaxVal[0], MinVal[0]);
				} // if
			// write shape file header to both shapefile & index file
			fwrite((UBYTE *)headertemplate, 1, 36, fShape);	// Mixed Endian!
			fwrite((UBYTE *)headertemplate, 1, 36, fIndex);	// Mixed Endian!
			// rest of header is Little Endian
			PutL64(&MinVal[0], fShape);	// Xmin
			PutL64(&MinVal[0], fIndex);	// Xmin
			PutL64(&MinVal[1], fShape);	// Ymin
			PutL64(&MinVal[1], fIndex);	// Ymin
			PutL64(&MaxVal[0], fShape);	// Xmax
			PutL64(&MaxVal[0], fIndex);	// Xmax
			PutL64(&MaxVal[1], fShape);	// Ymax
			PutL64(&MaxVal[1], fIndex);	// Ymax
			if (shape3d)
				{
				PutL64(&MinVal[2], fShape);	// Zmin
				PutL64(&MinVal[2], fIndex);	// Zmin
				PutL64(&MaxVal[2], fShape);	// Zmax
				PutL64(&MaxVal[2], fIndex);	// Zmax
				} // if shape3d
			else
				{
				PutL64(&Zero, fShape);	// Zmin
				PutL64(&Zero, fIndex);	// Zmin
				PutL64(&Zero, fShape);	// Zmax
				PutL64(&Zero, fIndex);	// Zmax
				} // else shape3d
			PutL64(&Zero, fShape);		// Mmin
			PutL64(&Zero, fIndex);		// Mmin
			PutL64(&Zero, fShape);		// Mmax
			PutL64(&Zero, fIndex);		// Mmax
			} // if

		if (ObjectsFound || ViaSX)	// need a valid dbf for empty Shapefiles too
			{
			// make a DB3 usable list of Layers & Attributes
#ifdef WCS_BUILD_VNS
			ExportFields = (struct DB3_Table_Field_Descriptor *)AppMem_Alloc(sizeof(DB3_Table_Field_Descriptor), APPMEM_CLEAR);
			if (! ExportFields)
				{
				Success = 0;
				} // if
			LastField = ExportFields;
			strcpy(LastField->Name, "VectName");
			LastField->FieldType = 'C';
			LastField->FieldLength = 40;
			FieldLength += 40;
			DB3_Fields++;
			for (CurrentLayer = DBLayers.FirstEntry(); CurrentLayer; CurrentLayer = DBLayers.NextEntry(CurrentLayer))
				{
				long fndx;
				char db3_name[16],fixedName[12];
				char badChars[] = "(!@#$%^&*()-+={}[]|\\\"':;?/>.<,) ";
				BOOL Found = FALSE;

				if (CurrentLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE)) continue; // skip internal multipart polygon linkage

				strncpy(db3_name, CurrentLayer->GetCleanName(), 11);	// usable name is in chars 0..9
				db3_name[11] = 0;

				// dBase field name requires 10 character or less names
				// Arc requires only ASCII letters or underscores in the field name
				// Arc also requires a character as the first character, and no spaces
				// Excel seems to think that uppercase only is required in dBase field names
				memset(fixedName, 0, sizeof(fixedName));
				strncpy(fixedName, db3_name, 11);
				for (fndx = 0; fndx < (long)(sizeof(badChars) - 1); fndx++)
					{
					char* offender;
					char badChar = badChars[fndx];

					if (offender = strchr(fixedName, badChar))
						{
						*offender = '_';
						} // if
					} // for
				fndx = 0;
				while ((!(isalpha(fixedName[fndx]))) && (fndx < 10))
					{
					fndx++;
					} // while

				memset(db3_name, 0, sizeof(db3_name));
				strncpy(db3_name, &fixedName[fndx], 10 - fndx);

				// make a log message showing how names were transformed
				if (strcmp(db3_name, CurrentLayer->GetCleanName()) && !GIS_Name_Warned)
					{
					char xformMsg[512];

					sprintf(xformMsg, "Layer name '%s' changed to field name '%s' in dBase file.", CurrentLayer->GetCleanName(), db3_name);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, xformMsg);
					changedNames = true;
					} // if
					
				for (ScanFields = ExportFields; ScanFields; ScanFields = ScanFields->Next)
					{
					// see if name is found
					if (strncmp(fixedName, &db3_name[1], 10) == 0)
						{
						// if it's an attribute, it could be in the layer list twice - as both a number and a string
						if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
							{
							if ((CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE)) && (ScanFields->FieldType == 'C'))
								Found = true;
							else if ((! CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE)) && (ScanFields->FieldType == 'N'))
								Found = true;
							} // if
						else
							Found = true;	// it's a layer name
						break;
						} // if
					} // for
				if (! Found)	// we either have a new name, or an attribute with a new type
					{
					LastField->Next = (struct DB3_Table_Field_Descriptor *)AppMem_Alloc(sizeof(DB3_Table_Field_Descriptor), APPMEM_CLEAR);
					LastField = LastField->Next;
					LastField->Next = NULL;
					/***
					if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
						strncpy(LastField->Name, &db3_name[1], 10);
					else
						strncpy(LastField->Name, db3_name, 10);
					***/
					strncpy(LastField->Name, db3_name, 10);
					if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
						{
						if (CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
							{	// Text Attribute
							LastField->FieldType = 'C';		// characters
							LastField->FieldLength = 40;	// field width
							FieldLength += 40;
							} // if Text Attribute
						else
							{	// Numeric Attribute
							LastField->FieldType = 'N';		// numeric
							LastField->FieldLength = 12;	// field width
							LastField->FieldCount = 3;		// # decimal places
							FieldLength += 12;
							} // else Text Attribute
						} // if Attribute
					else
						{	// Layer name
						LastField->FieldType = 'L';		// logical
						LastField->FieldLength = 1;		// field width
						LastField->FieldCount = 0;		// #decimal places
						FieldLength += 1;
						} // else Attribute
					DB3_Fields++;
					} // if !Found
				} // for CurrentLayer

#else // !WCS_BUILD_VNS (WCS)
			for (CurrentLayer = DBLayers.FirstEntry(); CurrentLayer; CurrentLayer = DBLayers.NextEntry(CurrentLayer))
				{
				char db3_name[16];
				if (!CurrentLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
					{
					if (!ExportFields)
						{
						if (!(ExportFields = (struct DB3_Table_Field_Descriptor *)AppMem_Alloc(sizeof(DB3_Table_Field_Descriptor), APPMEM_CLEAR)))
							{
							/*** F2 NOTE: Fix This ***/
							printf("Ack!");
							} // if
						else
							{
							strncpy(db3_name, CurrentLayer->GetName(), 11);	// usable name is in chars 1..10
							LastField = ExportFields;
							if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
								strncpy(ExportFields->Name, &db3_name[1], 10);
							else
								strncpy(ExportFields->Name, db3_name, 10);
							if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
								{
								if (CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
									{	// Text Attribute
									LastField->FieldType = 'C';		// characters
									LastField->FieldLength = 40;	// field width
									FieldLength += 40;
									} // if Text Attribute
								else
									{	// Numeric Attribute
									LastField->FieldType = 'N';		// numeric
									LastField->FieldLength = 12;	// field width
									LastField->FieldCount = 3;		// # decimal places
									FieldLength += 12;
									} // else Text Attribute
								} // if Attribute
							else
								{	// Layer name
								LastField->FieldType = 'L';	// logical
								LastField->FieldLength = 6;	// field width
								FieldLength += 6;
								} // else Attribute
							DB3_Fields = 1;
							} // else
						} // if !ExportFields
					else
						{
						BOOL Found = FALSE;
						strncpy(db3_name, CurrentLayer->GetName(), 11);	// usable name is in chars 1..10
						for (ScanFields = ExportFields; ScanFields; ScanFields = ScanFields->Next)
							{
							if (strncmp(ScanFields->Name, &db3_name[1], 10) == 0)
								{
								Found = TRUE;
								break;
								} // if
							if (!Found)
								{
								LastField->Next = (struct DB3_Table_Field_Descriptor *)AppMem_Alloc(sizeof(DB3_Table_Field_Descriptor), APPMEM_CLEAR);
								LastField = LastField->Next;
								LastField->Next = NULL;
								if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
									strncpy(LastField->Name, &db3_name[1], 10);
								else
									strncpy(LastField->Name, db3_name, 10);
								if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
									{
									if (CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
										{	// Text Attribute
										LastField->FieldType = 'C';		// characters
										LastField->FieldLength = 40;	// field width
										FieldLength += 40;
										} // if Text Attribute
									else
										{	// Numeric Attribute
										LastField->FieldType = 'N';		// numeric
										LastField->FieldLength = 12;	// field width
										LastField->FieldCount = 3;		// # decimal places
										FieldLength += 12;
										} // else Text Attribute
									} // if Attribute
								else
									{	// Layer name
									LastField->FieldType = 'L';	// logical
									LastField->FieldLength = 6;	// field width
									FieldLength += 6;
									} // else Attribute
								DB3_Fields++;
								break;
								} // if !Found
							} // for ScanFields
						} // else !ExportFields
					} // if !LinkAttribute

				/***
				if (!CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
					{
					for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
						{
						if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
							{
							JAH->ApproveMe = CurrentJoe;
							if (JAH->Approve(JAH))
								{
								if (CurrentJoe->MatchEntryToStub(CurrentLayer))
									{
									const char *FieldName;
									FieldName = CurrentLayer->GetName();
									// don't need to test any more Joes for this layer
									break;
									} // if object is in current layer
								} // if approved for output
							} // if not group object
						} // for each object in database
					} // if not an attribute
				***/
				} // for each layer in database
#endif // WCS_BUILD_VNS

			// now write our db3 fields to the database file
			time_t rawtime;
			struct tm *timeinfo;

			time(&rawtime);
			timeinfo = localtime(&rawtime);
			lWriteVal = 0x03000000;							// indicate DB3 format
			// add in the date as YYMMDD (straight binary - 8 bits per value, Y2K compliant :)
			lWriteVal += (timeinfo->tm_year << 16) + ((timeinfo->tm_mon + 1) << 8) + timeinfo->tm_mday;
			PutB32S(lWriteVal, fDBase);						// save in big-endian because I created this value that way
			PutL32S(ObjectsFound, fDBase);
			//lWriteVal = 32 + DB3_Fields * 32 + 1 + 96;	// # bytes in header
			lWriteVal = 32 + DB3_Fields * 32 + 1;			// # bytes in header
			PutL16U((unsigned short)lWriteVal, fDBase);
			PutL16U(FieldLength, fDBase);					// # bytes in record
			lWriteVal = 0;
			for (unsigned short i = 0; i < 20; i++)
				{
				fwrite((UBYTE *)&lWriteVal, 1, 1, fDBase);
				} // for
			db3recbytes = 1;
			LastField = ExportFields;
			while (LastField)
				{
				#ifdef DEBUG
				char logmsg[80];
				#endif // DEBUG

				db3recbytes += LastField->FieldLength;

				#ifdef DEBUG
				sprintf(logmsg, "db3recbytes = %d\n", db3recbytes);
				DEBUGOUT(logmsg);
				sprintf(logmsg, "Name = ");
				strcat(logmsg, LastField->Name);
				strcat(logmsg, "\n");
				DEBUGOUT(logmsg);
				sprintf(logmsg, "Type = %c\n", LastField->FieldType);
				DEBUGOUT(logmsg);
				sprintf(logmsg, "Length = %d\n\n", LastField->FieldLength);
				DEBUGOUT(logmsg);
				#endif // DEBUG

				fwrite((UBYTE *)&LastField->Name, 1, 11, fDBase);
				fwrite((UBYTE *)&LastField->FieldType, 1, 1, fDBase);
				fwrite((UBYTE *)&LastField->FieldAddress, 1, 4, fDBase);
				fwrite((UBYTE *)&LastField->FieldLength, 1, 1, fDBase);
				fwrite((UBYTE *)&LastField->FieldCount, 1, 1, fDBase);
				fwrite((UBYTE *)&LastField->reserved1, 1, 2, fDBase);
				fwrite((UBYTE *)&LastField->WorkID, 1, 1, fDBase);
				fwrite((UBYTE *)&LastField->reserved2, 1, 2, fDBase);
				fwrite((UBYTE *)&LastField->SetFields, 1, 1, fDBase);
				fwrite((UBYTE *)&LastField->reserved3, 1, 8, fDBase);
				LastField = LastField->Next;

				} // while
			lWriteVal = 0x0d;
			fwrite((UBYTE *)&lWriteVal, 1, 1, fDBase);	// write the Table File Header Terminator
			} // if

		if (ObjectsFound)
			{
			for (CurrentJoe = GetFirst(); CurrentJoe; CurrentJoe = GetNext(CurrentJoe))
				{
				if (! CurrentJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					JAH->ApproveMe = CurrentJoe;
					if (JAH->Approve(JAH))
						{
						BWDX->Update(NumWrite);
						NumWrite++;
						if (BWDX->CheckAbort())
							{
							break; // bail out of entities loop, but still write end of file
							} // if
						if (CurrentJoe->TestFlags(WCS_JOEFLAG_ISDEM))
							{
							} // if DEM
						else
							{
							if (CurrentJoe->GetFirstRealPoint() && CurrentJoe->GetNumRealPoints() > 0)	// probably not what it appears to be: 1st point = label point?
								{
								BOOL Need2Convert;	// If coords need to be converted
								BOOL ProcessIt = 0;	// true if passed bounds test or not clipping
								
								// identify if there is a coordsys attached to this object
								if (MyAttr = (JoeCoordSys *)CurrentJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
									MyCoords = MyAttr->Coord;
								else
									MyCoords = NULL;

								if ((MyCoords == NULL) && (OutCoords != NULL))
									Need2Convert = 1;
								else if ((OutCoords == NULL) && (MyCoords != NULL))
									Need2Convert = 1;
								else
									Need2Convert = MyCoords && (! OutCoords->Equals(MyCoords));

								if (!Clipping)
									ProcessIt = 1;
								else
									{
									// see if some part of this Joe is in our export area
									if ((((CurrentJoe->NWLon <= WBound) && (CurrentJoe->NWLon >= EBound)) ||
										((CurrentJoe->SELon <= WBound) && (CurrentJoe->SELon >= EBound))) &&
										(((CurrentJoe->NWLat <= NBound) && (CurrentJoe->NWLat >= SBound)) ||
										((CurrentJoe->SELat <= NBound) && (CurrentJoe->SELat >= SBound))))
										ProcessIt = 1;
									} // else

								if (ProcessIt)
									{
									double xmax = -FLT_MAX, xmin = FLT_MAX, ymax = -FLT_MAX, ymin = FLT_MAX, zmax = -FLT_MAX, zmin = FLT_MAX;
									double z;

									if (WriteLines && (CurrentJoe->GetLineStyle() >= 4))	// if line style
										{	// write PolyLineZ or PolyLine

										rec_start = ftell(fShape) / 2;	// offset in words
										PutB32S(rec_start, fIndex);		// write it to the Index file

										// write Record Number & {dummy} Content Length (in words) in Motorola format
										PutB32S(RecordNum, fShape);
										PutB32S(0L, fShape);

										RecordNum++;
										if (shape3d)
											ShapeType = 13;	// PolyLineZ
										else
											ShapeType = 3;	// PolyLine
										PutL32S(ShapeType, fShape);

										// write Box (bounds in Intel order)
										// *** just write 0.0 & rewrite correct values later ***
										bound_pos = ftell(fShape);
										dWriteVal = 0.0;
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										// write NumParts (Intel 1)
										PutL32S(1L, fShape);
										// write dummy number of points
										PutL32S(0L, fShape);
										// write Parts array (1 value of 0 since we're only writing one part)
										PutL32S(0L, fShape);
										// write Points (xy pairs in Intel order)
										numpts = 0;
										for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
											{
											numpts++;
											VP.Longitude = PLink->Longitude;
											VP.Latitude = PLink->Latitude;
											VP.Elevation = (float)(PLink->Elevation + AddedElev);
											if (Need2Convert)
												{
												// convert between the coordinate systems
												VP.ProjToDefDeg(MyCoords, &Vert);
												VP.DefDegToProj(OutCoords, &Vert);
												} // if
											// scale projected data
											if (OutCoords && (! GeoOutput))
												{
												VP.Longitude *= ExportXYScale;
												VP.Latitude *= ExportXYScale;
												VP.Elevation *= (float)ExportZScale;
												} // if
											else
												VP.Longitude *= LonDir;

											// compute the bounds for this shape
											if (VP.Longitude > xmax)
												xmax = VP.Longitude;
											if (VP.Longitude < xmin)
												xmin = VP.Longitude;
											if (VP.Latitude > ymax)
												ymax = VP.Latitude;
											if (VP.Latitude < ymin)
												ymin = VP.Latitude;
											if (VP.Elevation > zmax)
												zmax = VP.Elevation;
											if (VP.Elevation < zmin)
												zmin = VP.Elevation;

											PutL64(&VP.Longitude, fShape);
											PutL64(&VP.Latitude, fShape);
											} // for PLink

										// update the bounds for all the shapes
										if (xmax > MaxVal[0])
											MaxVal[0] = xmax;
										if (xmin < MinVal[0])
											MinVal[0] = xmin;
										if (ymax > MaxVal[1])
											MaxVal[1] = ymax;
										if (ymin < MinVal[1])
											MinVal[1] = ymin;
										if (zmax > MaxVal[2])
											MaxVal[2] = zmax;
										if (zmin < MinVal[2])
											MinVal[2] = zmin;
										if (shape3d)
											{
											// write Zmin, then Zmax (Intel order)
											PutL64(&zmin, fShape);
											PutL64(&zmax, fShape);
											// write Zarray (z values in Intel order)
											for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
												{
												z = PLink->Elevation * ExportZScale;
												PutL64(&z, fShape);
												} // for
											// don't bother with the optional measure fields
											} // if shape3d

										db3recs++;
										// now fix the Content Length (# words, Motorola format)
										rec_size = (ftell(fShape) - rec_start * 2) / 2 - 4;
										fseek(fShape, rec_start * 2 + 4, SEEK_SET);
										PutB32S(rec_size, fShape);
										PutB32S(rec_size, fIndex);	// append to Index file also
										// now fix this shapes bounds
										fseek(fShape, bound_pos, SEEK_SET);
										PutL64(&xmin, fShape);
										PutL64(&ymin, fShape);
										PutL64(&xmax, fShape);
										PutL64(&ymax, fShape);
										// now fix the number of points
										fseek(fShape, 4, SEEK_CUR);
										PutL32S(numpts, fShape);
										// put us back at the end again
										fseek(fShape, 0, SEEK_END);
										// now write the DB3 fields
										fwrite((const void *)" ", 1, 1, fDBase);	// an undeleted record

	#ifdef WCS_BUILD_VNS
										char vname[40];

										memset(vname, 0, 40);
										strncpy(vname, CurrentJoe->GetBestName(), 39);
										// hack to remove string terminators
										for (long i = 0; i < 40; i++)
											{
											if (vname[i] == 0)
												vname[i] = ' ';
											} // for
										fwrite((UBYTE *)vname, 1, 40, fDBase);
	#endif // WCS_BUILD_VNS
										for (CurrentLayer = DBLayers.FirstEntry(); CurrentLayer; CurrentLayer = DBLayers.NextEntry(CurrentLayer))
											{
											LayerStub *lstub;
											size_t writenum;
											char field[44];

											if (CurrentLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE)) continue; // skip internal multipart polygon linkage

											memset(field, 0, sizeof(field));

	#ifdef WCS_BUILD_VNS
											UBYTE i;

											// Attribute processing (VNS only)
											if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
												{
												lstub = CurrentJoe->MatchEntryToStub(CurrentLayer);
												if (CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
													{
													if (lstub)
														sprintf(field, "%.40s", CurrentJoe->GetTextAttributeValue(lstub));
													else
														sprintf(field, "%40s", " ");
													writenum = 40;
													} // if
												else
													{
													if (lstub)
														sprintf(field, "%12.3f", CurrentJoe->GetIEEEAttributeValue(lstub));
													else
														sprintf(field, "%12s", " ");
													writenum = 12;
													} // else
												// hack to remove string terminators
												for (i = 0; i < sizeof(field); i++)
													{
													if (field[i] == 0)
														field[i] = ' ';
													} // for
												fwrite((UBYTE *)field, 1, writenum, fDBase);
												} // if Attribute
	#endif // WCS_BUILD_VNS

											// Layer processing
											if (!CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
												{
												lstub = CurrentJoe->MatchEntryToStub(CurrentLayer);
												if (lstub)
													sprintf(field, "Y");	// write a logic true
												else
													sprintf(field, "N");	// write a logic false
												writenum = 1;
												/***
												// hack to remove string terminators
												for (i = 0; i < sizeof(field); i++)
													{
													if (field[i] == 0)
														field[i] = ' ';
													} // for
												***/
												fwrite((UBYTE *)field, 1, writenum, fDBase);
												} // if !LinkAttribute

											} // for CurrentLayer
										} // if line style
									else if ((CurrentJoe->TestFlags(WCS_JOEFLAG_ISCONTROL) || (!WriteLines && (CurrentJoe->GetLineStyle() < 4))))
										{	// write MultiPointZ or MultiPoint

										rec_start = ftell(fShape) / 2;	// offset in words
										PutB32S(rec_start, fIndex);		// write it to the Index file

										// write Record Number & {dummy} Content Length (in words) in Motorola format
										PutB32S(RecordNum, fShape);
										PutB32S(0L, fShape);

										RecordNum++;
										if (shape3d)
											ShapeType = 18;	// MultiPointZ
										else
											ShapeType = 1;	// MultiPoint
										PutL32S(ShapeType, fShape);

										// write Box (bounds in Intel order)
										// *** just write 0.0 & rewrite correct values later ***
										bound_pos = ftell(fShape);
										dWriteVal = 0.0;
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										PutL64(&dWriteVal, fShape);
										// write dummy number of points
										PutL32S(0L, fShape);
										// write Points (xy pairs in Intel order)
										numpts = 0;
										for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
											{
											numpts++;
											VP.Longitude = PLink->Longitude;
											VP.Latitude = PLink->Latitude;
											VP.Elevation = (float)(PLink->Elevation + AddedElev);
											if (Need2Convert)
												{
												// convert between the coordinate systems
												VP.ProjToDefDeg(MyCoords, &Vert);
												VP.DefDegToProj(OutCoords, &Vert);
												} // if
											// scale projected data
											if (OutCoords && (! GeoOutput))
												{
												VP.Longitude *= ExportXYScale;
												VP.Latitude *= ExportXYScale;
												VP.Elevation *= (float)ExportZScale;
												} // if
											else
												VP.Longitude *= LonDir;

											// compute the bounds for this shape
											if (VP.Longitude > xmax)
												xmax = VP.Longitude;
											if (VP.Longitude < xmin)
												xmin = VP.Longitude;
											if (VP.Latitude > ymax)
												ymax = VP.Latitude;
											if (VP.Latitude < ymin)
												ymin = VP.Latitude;
											if (VP.Elevation > zmax)
												zmax = VP.Elevation;
											if (VP.Elevation < zmin)
												zmin = VP.Elevation;

											PutL64(&VP.Longitude, fShape);
											PutL64(&VP.Latitude, fShape);
											} // for PLink

										// update the bounds for all the shapes
										if (xmax > MaxVal[0])
											MaxVal[0] = xmax;
										if (xmin < MinVal[0])
											MinVal[0] = xmin;
										if (ymax > MaxVal[1])
											MaxVal[1] = ymax;
										if (ymin < MinVal[1])
											MinVal[1] = ymin;
										if (zmax > MaxVal[2])
											MaxVal[2] = zmax;
										if (zmin < MinVal[2])
											MinVal[2] = zmin;
										if (shape3d)
											{
											// write Zmin, then Zmax (Intel order)
											PutL64(&zmin, fShape);
											PutL64(&zmax, fShape);
											// write Zarray (z values in Intel order)
											for (PLink = CurrentJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
												{
												z = PLink->Elevation * ExportZScale;
												PutL64(&z, fShape);
												} // for
											// don't bother with the optional measure fields
											} // if shape3d

										db3recs++;
										// now fix the Content Length (# words, Motorola format)
										rec_size = (ftell(fShape) - rec_start * 2) / 2 - 4;
										fseek(fShape, rec_start * 2 + 4, SEEK_SET);
										PutB32S(rec_size, fShape);
										PutB32S(rec_size, fIndex);	// append to Index file also
										// now fix this shapes bounds
										fseek(fShape, bound_pos, SEEK_SET);
										PutL64(&xmin, fShape);
										PutL64(&ymin, fShape);
										PutL64(&xmax, fShape);
										PutL64(&ymax, fShape);
										// now fix the number of points
										PutL32S(numpts, fShape);
										// put us back at the end again
										fseek(fShape, 0, SEEK_END);
										// now write the DB3 fields
										fwrite((const void *)" ", 1, 1, fDBase);	// an undeleted record

#ifdef WCS_BUILD_VNS
										char vname[40];

										memset(vname, 0, 40);
										strncpy(vname, CurrentJoe->GetBestName(), 39);
										// hack to remove string terminators
										for (long i = 0; i < 40; i++)
											{
											if (vname[i] == 0)
												vname[i] = ' ';
											} // for
										fwrite((UBYTE *)vname, 1, 40, fDBase);
#endif // WCS_BUILD_VNS
										for (CurrentLayer = DBLayers.FirstEntry(); CurrentLayer; CurrentLayer = DBLayers.NextEntry(CurrentLayer))
											{
											LayerStub *lstub;
											size_t writenum;
											char field[44];
											UBYTE i;

											if (CurrentLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE)) continue; // skip internal multipart polygon linkage

											memset(field, 0, sizeof(field));

	#ifdef WCS_BUILD_VNS
											// Attribute processing (VNS only)
											if (CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
												{
												lstub = CurrentJoe->MatchEntryToStub(CurrentLayer);
												if (CurrentLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
													{
													if (lstub)
														sprintf(field, "%.40s", CurrentJoe->GetTextAttributeValue(lstub));
													else
														sprintf(field, "%40s", " ");
													writenum = 40;
													} // if
												else
													{
													if (lstub)
														sprintf(field, "%12.3f", CurrentJoe->GetIEEEAttributeValue(lstub));
													else
														sprintf(field, "%12s", " ");
													writenum = 12;
													} // else
												// hack to remove string terminators
												for (i = 0; i < sizeof(field); i++)
													{
													if (field[i] == 0)
														field[i] = ' ';
													} // for
												fwrite((UBYTE *)field, 1, writenum, fDBase);
												} // if Attribute
	#endif // WCS_BUILD_VNS

											// Layer processing
											if (!CurrentLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
												{
												lstub = CurrentJoe->MatchEntryToStub(CurrentLayer);
												if (lstub)
													sprintf(field, "Y");	// write a logic true
												else
													sprintf(field, "N");	// write a logic false
												writenum = 1;
												/***
												// hack to remove string terminators
												for (i = 0; i < sizeof(field); i++)
													{
													if (field[i] == 0)
														field[i] = ' ';
													} // for
												***/
												fwrite((UBYTE *)field, 1, writenum, fDBase);
												} // if !LinkAttribute

											} // for CurrentLayer
										} // else point style
									} // if ProcessIt
								} // if points exist
							} // else
						} // if
					} // if not group
				if (BWDY)
					{
					delete BWDY;
					BWDY = NULL;
					}
				} // for Current
			} // if objects found
		else if (! ViaSX)	// empty shapefiles are allowed for GIS export
			{
			UserMessageOK("Export Shape", "No objects were found!\nOperation terminated.");
			Success = 0;
			} // else
//FileError:
		if (! Success)
			{
			UserMessageOK("Export Shape", "An error occurred while writing to file!\nNot all files were saved correctly.");
			} // if
		strmfp(filename, OutPath, OutName);
		strcat(filename, ".prj");
		if (OutCoords && (fPrj = PROJ_fopen(filename, "w")))
			{
			OutCoords->SaveToArcPrj(fPrj);
			fclose(fPrj);
			} // if
		} // if files opened
	else
		{
		char errmsg[300], ext[8];
		if (fIndex == NULL)
			strcpy(ext, ".shx");
		else if (fDBase == NULL)
			strcpy(ext, ".dbf");
		else
			strcpy(ext, ".shp");
		strmfp(filename, OutPath, OutName);
		strcat(filename, ext);
		sprintf(errmsg, "Error opening file for output (%s)!\nOperation terminated.", filename);
		UserMessageOK("Export Shape", errmsg);
		Success = 0;
		} // else couldn't open a file
	} // if Proceed
else
	Success = 0;

if (BWDX)
	{
	delete BWDX;
	BWDX = NULL;
	} // if

if (fIndex)
	{
	long fpos;
	// write Big Endian file length in words
	fseek(fIndex, 0, SEEK_END);	// make sure we're back at the end of the file
	fpos = ftell(fIndex) / 2;
	fseek(fIndex, 24, SEEK_SET);
	PutB32S(fpos, fIndex);
	if (fShape)
		{
		// write Big Endian file length in words
		fseek(fShape, 0, SEEK_END);	// make sure we're back at the end of the file
		fpos = ftell(fShape) / 2;
		fseek(fShape, 24, SEEK_SET);
		PutB32S(fpos, fShape);
		// write Little Endian ShapeType
		fseek(fShape, 32, SEEK_SET);
		fseek(fIndex, 32, SEEK_SET);
		if (WriteLines && shape3d)
			lWriteVal = 13;
		else if (WriteLines)
			lWriteVal = 3;
		else if (shape3d)
			lWriteVal = 11;
		else
			lWriteVal = 1;
		PutL32S(lWriteVal, fShape);
		PutL32S(lWriteVal, fIndex);
		// fix the files bounding box
		fseek(fShape, 36, SEEK_SET);
		fseek(fIndex, 36, SEEK_SET);
		PutL64(&MinVal[0], fShape);	// Xmin
		PutL64(&MinVal[0], fIndex);	// Xmin
		PutL64(&MinVal[1], fShape);	// Ymin
		PutL64(&MinVal[1], fIndex);	// Ymin
		PutL64(&MaxVal[0], fShape);	// Xmax
		PutL64(&MaxVal[0], fIndex);	// Xmax
		PutL64(&MaxVal[1], fShape);	// Ymax
		PutL64(&MaxVal[1], fIndex);	// Ymax
		if (shape3d)
			{
			PutL64(&MinVal[2], fShape);	// Zmin
			PutL64(&MinVal[2], fIndex);	// Zmin
			PutL64(&MaxVal[2], fShape);	// Zmax
			PutL64(&MaxVal[2], fIndex);	// Zmax
			} // if shape3d
		else
			{
			PutL64(&Zero, fShape);	// Zmin
			PutL64(&Zero, fIndex);	// Zmin
			PutL64(&Zero, fShape);	// Zmax
			PutL64(&Zero, fIndex);	// Zmax
			} // else shape3d
		PutL64(&Zero, fShape);		// Mmin
		PutL64(&Zero, fIndex);		// Mmin
		PutL64(&Zero, fShape);		// Mmax
		PutL64(&Zero, fIndex);		// Mmax
		} // if fShape
	fclose(fIndex);
	} // if fIndex
if (fDBase)
	{
	lWriteVal = 0x1a1a1a1a;	// any endian value
	fwrite((UBYTE *)&lWriteVal, 1, 1, fDBase);		// write EOF marker
	fseek(fDBase, 4, SEEK_SET);
	PutL32U(db3recs, fDBase);	// fix up number of records
	fseek(fDBase, 2, SEEK_CUR);
	PutL16U((unsigned short)db3recbytes, fDBase);
	fclose(fDBase);
	} // if fDBase
if (fShape)
	fclose(fShape);

ScanFields = ExportFields;
while (ScanFields)
	{
	struct DB3_Table_Field_Descriptor *Destroy;

	Destroy = ScanFields;
	ScanFields = ScanFields->Next;
	AppMem_Free(Destroy , sizeof(DB3_Table_Field_Descriptor));
	} // for

if (Success)
	{
	char msg[80];

	sprintf(msg, "Export complete: %d records written", RecordNum - 1);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	} // if

if (changedNames && !GIS_Name_Warned)
	{
	UserMessageOK("Shapefile Alert", "Status log contains Layer name changes made for DBF export.");
	GIS_Name_Warned = true;
	} // if

return (Success);

} // Database::ExportShape

/*===========================================================================*/

void DBEditGUI::UpdateVecColor(unsigned char Red, unsigned char Green, unsigned char Blue)
{
DoPatch DP;
long j;

DP.DoThreeShort = (void (*)(class Joe *,short,short,short))SetRGBPen;

ImBusy = 1;

// do the Active Edit item first
SetRGBPen(GetJoeFromOBN(ActiveColorEditItem), Red, Green, Blue);

// The surgeon-general has determined that looking too hard at the
// following code may cause headaches and cancer in rats.

if (!DoSomethingToEach(&DP, Red, Green, Blue))
	{
	for (j=0; j<NoOfObjects; j++)
		{
		if (GetSelected(j))
			{
			SetRGBPen(GetJoeFromOBN(j), Red, Green, Blue);
			} // if object selected
		} // for
	} // if

ImBusy = 0;
} // DBEditGUI::UpdateVecColor

/*===========================================================================*/

long DBEditGUI::HandleEvent(void)
{
LPMINMAXINFO LPMMI;

if (Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
	{
	switch (Activity->GUIMessage.message)
		{
		case WM_GETMINMAXINFO:
			{
			LPMMI = (LPMINMAXINFO)Activity->GUIMessage.lParam;
			LPMMI->ptMaxTrackSize.x = AppScope->WinSys->InquireDisplayWidth();
			LPMMI->ptMaxTrackSize.y = AppScope->WinSys->InquireDisplayHeight();
			LPMMI->ptMinTrackSize.x = StockX;
			LPMMI->ptMinTrackSize.y = StockY;
			return(0);
			} // MINMAXINFO
		default:
			break;
		} // switch
	} // else if

return(0);

} // DBEditGUI::HandleEvent

/*===========================================================================*/
long DBEditGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT OriginalWidgetRect;
long OriginalWidgetWidth, OriginalWidgetHeight, NewWidgetWidth, NewWidgetHeight;
POINT ConvertCoord;

// update the project with our current size
short Left, Top, W, H;
GlobalApp->MainProj->InquireWindowCoords(FenID, Left, Top, W, H); // fetch recorded position we don't want to mess up
GlobalApp->MainProj->SetWindowCoords(FenID, Left, Top, (short)NewWidth, (short)NewHeight);


// get original Grid widget position
GetWindowRect(GetWidgetFromID(IDC_LIST1), &OriginalWidgetRect);
// calculate and record current widget size
OriginalWidgetWidth = (OriginalWidgetRect.right - OriginalWidgetRect.left);
OriginalWidgetHeight = (OriginalWidgetRect.bottom - OriginalWidgetRect.top);
// translate widget UL corner position into window client coordinates
ConvertCoord.x = OriginalWidgetRect.left;
ConvertCoord.y = OriginalWidgetRect.top;
ScreenToClient(NativeWin, &ConvertCoord);

// calculate new width & height
if((ActivePage == 1) || (ActivePage == 0) || (!ShowPropertiesPanel)) // extent, properties
	{ // no right-side panel
	NewWidgetWidth = (NewWidth - ConvertCoord.x);
	NewWidgetHeight = (NewHeight - ConvertCoord.y);
	} // if
else
	{ // acount for right-side properties panel
	GetClientRect(SubPanels[0][4], &OriginalWidgetRect); // How big is the Layers sub-panel?
	NewWidgetWidth = (NewWidth - (ConvertCoord.x + OriginalWidgetRect.right + 8)); // subtract that much off, 8 pixel margin on right
	NewWidgetHeight = (NewHeight - ConvertCoord.y);

	// move the sub-panels
	int SubPanelX, SubPanelY;
	SubPanelX = ConvertCoord.x + NewWidgetWidth;
	SubPanelY = ConvertCoord.y;
	SetWindowPos(SubPanels[0][0], NULL, SubPanelX, SubPanelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // properties
	SetWindowPos(SubPanels[0][1], NULL, SubPanelX, SubPanelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // extent vec
	SetWindowPos(SubPanels[0][2], NULL, SubPanelX, SubPanelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // extent dem
	SetWindowPos(SubPanels[0][3], NULL, SubPanelX, SubPanelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // effect
	SetWindowPos(SubPanels[0][4], NULL, SubPanelX, SubPanelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // layers
	} // else

// resize the Grid widget
SetWindowPos(GetWidgetFromID(IDC_LIST1), NULL, 0, 0, NewWidgetWidth - 5, NewWidgetHeight - 5, SWP_NOMOVE | SWP_NOZORDER);

// Tab Widget
// get original Tab widget position
GetWindowRect(GetWidgetFromID(IDC_TAB1), &OriginalWidgetRect);
// calculate and record current widget size
OriginalWidgetWidth = (OriginalWidgetRect.right - OriginalWidgetRect.left);
OriginalWidgetHeight = (OriginalWidgetRect.bottom - OriginalWidgetRect.top);
// translate widget UL corner position into window client coordinates
ConvertCoord.x = OriginalWidgetRect.left;
ConvertCoord.y = OriginalWidgetRect.top;
ScreenToClient(NativeWin, &ConvertCoord);

// calculate new width & height
NewWidgetWidth = (NewWidth - ConvertCoord.x);
NewWidgetHeight = (NewHeight - ConvertCoord.y);

// resize the Tab widget
SetWindowPos(GetWidgetFromID(IDC_TAB1), NULL, 0, 0, NewWidgetWidth, NewWidgetHeight, SWP_NOMOVE | SWP_NOZORDER);

// Filter/Properties buttons
// get original Tab widget position
GetWindowRect(GetWidgetFromID(IDC_TAB1), &OriginalWidgetRect);
// translate widget UL corner position into window client coordinates
ConvertCoord.x = OriginalWidgetRect.right;
ConvertCoord.y = OriginalWidgetRect.top;
ScreenToClient(NativeWin, &ConvertCoord);

// move the Filter button
SetWindowPos(GetWidgetFromID(IDC_DBFILTER), NULL, ConvertCoord.x - 42, ConvertCoord.y - 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
// move the Properties button
SetWindowPos(GetWidgetFromID(IDC_SHOWPROPPANEL), NULL, ConvertCoord.x - 22, ConvertCoord.y - 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
WidgetRepaint(IDC_SHOWPROPPANEL);
WidgetRepaint(IDC_DBFILTER);

return(0);
} // DBEditGUI::HandleReSized

int DBEditGUI::PrepGridWidgetForJoeUsage(NativeControl GridWidget)
{
WidgetShow(IDC_LIST1, 0);

if(!GridWidgetColumnsPrepped)
	{
	ListView_SetExtendedListViewStyle(GridWidget, /* LVS_EX_DOUBLEBUFFER */ 0x00010000 | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_SUBITEMIMAGES | LVS_EX_ONECLICKACTIVATE);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME, "Name", 110);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL, "Label", 110);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED, "Enabled", 55);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW, "View", 55);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER, "Render", 55);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR, "Color", 40);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS, "Class", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE, "Style", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT, "Weight", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD, "Max Fractal Depth", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA, "Area", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS, "Points", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH, "Length", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS, "Rows", 45);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS, "Columns", 45);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL, "Maximum Elevation", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL, "Minimum Elevation", 50);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS, "NS Grid", 60);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW, "EW Grid", 60);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH, "North", 60);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH, "South", 60);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST, "East", 60);
	AddColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST, "West", 60);

	AddComponentColumn(GridWidget, "Components", NULL); // we don't have one column per component as originally envisioned
	} // if
else
	{ // clear layers/attribs columns for re-adding
	for(unsigned long int ColumnToClear = (WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size() + AttribColumns.size()) - 1; 
	 ColumnToClear >= WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size(); ColumnToClear--)
		{
		ListView_DeleteColumn(GridWidget, ColumnToClear);
		} // for
	LayerColumns.clear();
	AttribColumns.clear();
	} // else


// list Layers
LayerEntry *MyLayer;
const char *MyName;
long IsAttribute;
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
char LNameTest[256];
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

if (MyLayer = Host->DBLayers.FirstEntry())
	{
	while (MyLayer)
		{
		if (MyName = MyLayer->GetName())
			{
			IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
			if (! IsAttribute)
				{
				AddLayerColumn(GridWidget, MyName, MyLayer);
				} // else
			} // if
		MyLayer = Host->DBLayers.NextEntry(MyLayer);
		} // while
	} // if

// list Attributes
if (MyLayer = Host->DBLayers.FirstEntry())
	{
	while (MyLayer)
		{
		if (MyName = MyLayer->GetName())
			{
			IsAttribute = MyLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
			if (IsAttribute)
				{
				if (! MyLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
					{
					sprintf(LNameTest, "%s", &MyName[1]);
					AddAttributeColumn(GridWidget, LNameTest, MyLayer);
					} // if not link
				} // if
			} // if
		MyLayer = Host->DBLayers.NextEntry(MyLayer);
		} // while
	} // if

WidgetShow(IDC_LIST1, 1);

GridWidgetColumnsPrepped = true;

return(1);
} // DBEditGUI::PrepGridWidgetForJoeUsage

int DBEditGUI::AddColumn(NativeControl GridWidget, int ColumnIndex, const char *ColumnText, int ColumnWidth)
{
LVCOLUMN LVC;
LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
LVC.fmt = LVCFMT_LEFT;

if (ColumnWidth == -1) // auto-size
	{
	LVC.cx = ListView_GetStringWidth(GridWidget, ColumnText);
	LVC.cx += 20;
	} // if
else
	{
	LVC.cx = ColumnWidth;
	} // else
LVC.iSubItem = ColumnIndex;
LVC.pszText = (LPSTR)ColumnText;
ListView_InsertColumn(GridWidget, ColumnIndex, &LVC);
ColumnWidths.push_back(LVC.cx);

return(1);
} // DBEditGUI::AddColumn


int DBEditGUI::AddComponentColumn(NativeControl GridWidget, const char *ColumnText, GeneralEffect *Component)
{
int DestColumn = 0;
LVCOLUMN LVC;
LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
LVC.fmt = LVCFMT_LEFT;
LVC.cx = 300;

DestColumn = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size();
ComponentColumns.push_back(Component);
LVC.pszText = (LPSTR)ColumnText;
ColumnWidths.push_back(LVC.cx);
return(ListView_InsertColumn(GridWidget, DestColumn, &LVC));
} // DBEditGUI::AddComponentColumn


int DBEditGUI::AddLayerColumn(NativeControl GridWidget, const char *ColumnText, LayerEntry *Layer)
{
int DestColumn = 0, ResultColumn;
LVCOLUMN LVC;
LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
LVC.fmt = LVCFMT_LEFT;
HWND HeaderWnd;

LVC.cx = ListView_GetStringWidth(GridWidget, ColumnText);
//LVC.cx += 20;
LVC.cx += 36;

DestColumn = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size();
LayerColumns.push_back(Layer);				
LVC.pszText = (LPSTR)ColumnText;
ColumnWidths.push_back(LVC.cx);
ResultColumn = ListView_InsertColumn(GridWidget, DestColumn, &LVC);
if (ResultColumn != -1)
	{
	if (HeaderWnd = ListView_GetHeader(GridWidget))
		{
		HDITEM HeaderItem;
		if (Header_GetItem(HeaderWnd, ResultColumn, &HeaderItem))
			{
			HeaderItem.mask = (HDI_BITMAP | HDI_FORMAT);
			HeaderItem.fmt |= (HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
			HeaderItem.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), "IDB_RIGHTTRIANGLE", IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
			Header_SetItem(HeaderWnd, ResultColumn, &HeaderItem);
			} // if
		} // if
	} // if

return(ResultColumn);
} // DBEditGUI::AddLayerColumn


int DBEditGUI::AddAttributeColumn(NativeControl GridWidget, const char *ColumnText, LayerEntry *Layer)
{
int DestColumn = 0;
LVCOLUMN LVC;
LVC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
LVC.fmt = LVCFMT_LEFT;

LVC.cx = ListView_GetStringWidth(GridWidget, ColumnText);
LVC.cx += 20;

DestColumn = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size() + AttribColumns.size();
AttribColumns.push_back(Layer);
LVC.pszText = (LPSTR)ColumnText;
ColumnWidths.push_back(LVC.cx);
return(ListView_InsertColumn(GridWidget, DestColumn, &LVC));
} // DBEditGUI::AddAttributeColumn


void DBEditGUI::ShowPropertiesColumns(NativeControl GridWidget, bool ShowState)
{
LVCOLUMN LVC;
LVC.mask = LVCF_WIDTH;
if (!ShowState)
	{
	LVC.cx = 0;
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD, &LVC);
	} // if
else
	{
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD, &LVC);
	} // else
} // DBEditGUI::ShowPropertiesColumns

void DBEditGUI::ShowExtentColumns(NativeControl GridWidget, bool ShowState)
{
LVCOLUMN LVC;
LVC.mask = LVCF_WIDTH;
if (!ShowState)
	{
	LVC.cx = 0;
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST, &LVC);
	ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST, &LVC);
	} // if
else
	{
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST, &LVC);
	LVC.cx = ColumnWidths[WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST]; ListView_SetColumn(GridWidget, WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST, &LVC);
	} // else
} // DBEditGUI::ShowExtentColumns

void DBEditGUI::ShowComponentColumns(NativeControl GridWidget, bool ShowState)
{
LVCOLUMN LVC;
LVC.mask = LVCF_WIDTH;

for(unsigned int i = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX; i < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size(); i++)
	{
	if (!ShowState)
		{
		LVC.cx = 0;
		ListView_SetColumn(GridWidget, i, &LVC);
		} // if
	else
		{
		LVC.cx = ColumnWidths[i]; ListView_SetColumn(GridWidget, i, &LVC);
		} // else
	} // for
} // DBEditGUI::ShowComponentColumns

void DBEditGUI::ShowLayerColumns(NativeControl GridWidget, bool ShowState)
{
LVCOLUMN LVC;
LVC.mask = LVCF_WIDTH;

for(unsigned int i = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size(); i < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size(); i++)
	{
	if (!ShowState)
		{
		LVC.cx = 0;
		ListView_SetColumn(GridWidget, i, &LVC);
		} // if
	else
		{
		LVC.cx = ColumnWidths[i]; ListView_SetColumn(GridWidget, i, &LVC);
		} // else
	} // for
} // DBEditGUI::ShowLayerColumns

void DBEditGUI::ShowAttributeColumns(NativeControl GridWidget, bool ShowState)
{
LVCOLUMN LVC;
LVC.mask = LVCF_WIDTH;

for(unsigned int i = WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size();
 i < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + ComponentColumns.size() + LayerColumns.size() + AttribColumns.size(); i++)
	{
	if (!ShowState)
		{
		LVC.cx = 0;
		ListView_SetColumn(GridWidget, i, &LVC);
		} // if
	else
		{
		LVC.cx = ColumnWidths[i]; ListView_SetColumn(GridWidget, i, &LVC);
		} // else
	} // for
} // DBEditGUI::ShowAttributeColumns

NativeControl DBEditGUI::CreateTopToolbar(HWND hwndParent, HINSTANCE g_hinst)
{
   HWND hwndRB = NULL;
   REBARBANDINFO rbBand;
   RECT          rc;

// Create a Rebar to contain the toolbar
   hwndRB = CreateWindowEx(0, REBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_AUTOSIZE | CCS_NORESIZE,
        0, 0, LocalWinSys()->InquireDisplayWidth(), 20, hwndParent, 
        NULL, g_hinst, NULL); 

	//The rebar control has been succesfully created 
	REBARINFO ri={0};
	//no imagelist to attach to rebar
	ri.cbSize=sizeof(REBARINFO); 
	SendMessage(hwndRB,RB_SETBARINFO,0,reinterpret_cast<LPARAM>(&ri));

   // Initialize structure members for band(s).
   rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
   rbBand.fMask  = RBBIM_TEXT | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE;
   rbBand.fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE;

// Create a toolbar. 
   hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_LIST, 
        0, 0, LocalWinSys()->InquireDisplayWidth(), 20, hwndRB, 
        NULL, g_hinst, NULL); 

// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
// backward compatibility. 
   SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
   SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS); 
   LRESULT lResult;
   // Get the tooltip so we can twiddle it
   HWND TTip;
   TTip = (HWND)SendMessage(hwndTB, TB_GETTOOLTIPS, 0, 0);
   if (TTip)
	{
	SetWindowLong(TTip, GWL_STYLE, GetWindowLong(TTip, GWL_STYLE) | TTS_ALWAYSTIP);
	// we actually force this TOPMOST again during TBN_GETINFOTIP processing elsewhere
	SetWindowPos(TTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	} // if

   toolBarIL = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK , 38, 0 ); // Will need to destroy in cleanup
   lResult = SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)toolBarIL); 
       
// Fill the TBBUTTON array with button information, and add the 
// buttons to the toolbar.

   ButtonCount = 0;

   AddToolbarImageIcon("Remove/Delete Database Object", IDI_DBTB_REMOVE, ButtonCount++, false);
   AddToolbarImageIcon("Search for Database Objects", IDI_DBTB_SEARCH, ButtonCount++, false);
   AddToolbarImageIcon("Edit Database Object", IDI_DBTB_EDIT, ButtonCount++, false);
   AddToolbarImageIcon("DEM Painter", IDI_DEMPAINT, ButtonCount++, false);
   AddToolbarImageIcon("Edit Vector Profile", IDI_DBTB_PROFILE, ButtonCount++, false);
   AddToolbarImageIcon("Split Database Object", IDI_DBTB_SPLIT, ButtonCount++, false);
   AddToolbarImageIcon("Join Database Objects", IDI_DBTB_JOIN, ButtonCount++, false);
   AddToolbarImageIcon("Conform...", IDI_DBTB_CONFORM, ButtonCount++, false);

   SIZE TBSize;
   lResult = SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);
   SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
   SetWindowPos(hwndTB, NULL, 0, 0, LocalWinSys()->InquireDisplayWidth(), TBSize.cy + 2, SWP_NOZORDER);
   int Win2KPadding = 0;
   if (!g_xpStyle.IsAppThemed())
     {
     Win2KPadding = 2;
     } // if
   SetWindowPos(hwndRB, NULL, 0, 0, LocalWinSys()->InquireDisplayWidth(), TBSize.cy + 2 + Win2KPadding, SWP_NOMOVE | SWP_NOZORDER);
   
   GetClientRect(hwndTB, &rc);
   rbBand.lpText     = NULL;
   rbBand.hwndChild  = hwndTB;
   rbBand.cxMinChild = 0;
   rbBand.cyMinChild = rc.bottom;
   rbBand.cx         = rc.right;

   // Add the band with the toolbar.
   SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

   ShowWindow(hwndRB, SW_SHOW); 
   Fenetre *Stash;
   Stash = this; // Convert to *Fenetre, not obvious
   SetWindowLong(hwndTB, GWL_USERDATA, (LONG)Stash);
   
   return hwndTB; 
} // DBEditGUI::CreateTopToolbar

bool DBEditGUI::AddToolbarImageIcon(char *HelpCapt, WORD IconID, unsigned long int ImageNum, bool Tog, bool StartHidden)
{

ImageList_AddIcon (toolBarIL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IconID))); 

TBBUTTON tbb;
LRESULT lResult;

tbb.iBitmap = ImageNum; 
tbb.idCommand = IconID; 
tbb.fsState = TBSTATE_ENABLED | (StartHidden ? TBSTATE_HIDDEN : 0); 
tbb.fsStyle = Tog ? BTNS_CHECK : BTNS_BUTTON;
tbb.dwData = 0; 
tbb.iString = (INT_PTR)HelpCapt;
lResult = SendMessage(hwndTB, TB_ADDBUTTONS, (WPARAM)1, (LPARAM) (LPTBBUTTON) &tbb); 

return(lResult ? true : false);
} // DBEditGUI::AddToolbarImageIcon

void DBEditGUI::SetToolbarButtonDisabled(WORD IconID, bool NewDisabledState)
{
SendMessage(hwndTB, (UINT) TB_ENABLEBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (!NewDisabledState, 0));
} // DBEditGUI::SetToolbarButtonDisabled


/*===========================================================================*/

void DBEditGUI::DoFilterPopup(void)
{
HMENU FilterPopupMenu = NULL, LayerSub = NULL, SQSub = NULL;

if(FilterPopupMenu = CreatePopupMenu())
	{
	POINT CursorPos;
	long ConformPopResult;
	GetCursorPos(&CursorPos);
	AppendMenu(FilterPopupMenu, MF_STRING | MF_DISABLED, 58000, "Filter Database by:");
	AppendMenu(FilterPopupMenu, MF_STRING | MF_ENABLED | (GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE ? MF_CHECKED : 0), ID_DBEDIT_FILTER_MENU_NONE, "None");
	AppendMenu(FilterPopupMenu, MF_STRING | MF_ENABLED | (GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC ? MF_CHECKED : 0), ID_DBEDIT_FILTER_MENU_VEC, "Vectors");
	AppendMenu(FilterPopupMenu, MF_STRING | MF_ENABLED | (GlobalApp->MainProj->Prefs.DBEFilterMode == WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM ? MF_CHECKED : 0), ID_DBEDIT_FILTER_MENU_DEM, "DEMs");
	if(LayerSub = CreatePopupMenu())
		{
		for(unsigned int LayerLoop = 0; LayerLoop < LayerColumns.size(); LayerLoop++)
			{
			bool Matched = false;
			if(!stricmp(GetLayerColumn(LayerLoop)->GetName(), GlobalApp->MainProj->Prefs.DBELayerFilterName))
				{
				Matched = true;
				} // if
			AppendMenu(LayerSub, MF_STRING | MF_ENABLED | (Matched ? MF_CHECKED : 0), ID_DBEDIT_FILTER_MENU_LAYERBASE + LayerLoop,
			 GetLayerColumn(LayerLoop)->GetName());
			} // for
		AppendMenu(FilterPopupMenu, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)LayerSub, "Layer");
		} // if
	#ifdef WCS_BUILD_VNS
	if(SQSub = CreatePopupMenu())
		{
		SearchQuery *MyEffect;
		AppendMenu(FilterPopupMenu, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)SQSub, "Query");
		int Count = 0;
		for (MyEffect = (SearchQuery *)HostLib->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY); MyEffect; MyEffect = (SearchQuery *)MyEffect->Next)
			{
			bool Matched = false;
			if(!stricmp(MyEffect->GetName(), GlobalApp->MainProj->Prefs.DBESearchQueryFilterName))
				{
				Matched = true;
				} // if
			AppendMenu(SQSub, MF_STRING | MF_ENABLED | (Matched ? MF_CHECKED : NULL), ID_DBEDIT_FILTER_MENU_QUERYBASE + Count++, MyEffect->GetName());
			} // for
		} // if
	#endif // WCS_BUILD_VNS

	if(ConformPopResult = TrackPopupMenu(FilterPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, CursorPos.x, CursorPos.y, 0, NativeWin, NULL))
		{
		switch (ConformPopResult)
			{
			case ID_DBEDIT_FILTER_MENU_NONE:
				{
				GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE;
				break;
				} // ID_DBEDIT_FILTER_MENU_NONE
			case ID_DBEDIT_FILTER_MENU_VEC:
				{
				GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC;
				break;
				} // ID_DBEDIT_FILTER_MENU_VEC
			case ID_DBEDIT_FILTER_MENU_DEM:
				{
				GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM;
				break;
				} // ID_DBEDIT_FILTER_MENU_DEM
			default:
				{ // could be Layer or Query
				if(ConformPopResult >= ID_DBEDIT_FILTER_MENU_LAYERBASE && ConformPopResult < ID_DBEDIT_FILTER_MENU_QUERYBASE)
					{ // layer
					int LayerNum = ConformPopResult - ID_DBEDIT_FILTER_MENU_LAYERBASE;
					strncpy(GlobalApp->MainProj->Prefs.DBELayerFilterName, GetLayerColumn(LayerNum)->GetName(), WCS_EFFECT_MAXNAMELENGTH);
					GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER;
					} // if
				else if(ConformPopResult >= ID_DBEDIT_FILTER_MENU_QUERYBASE && ConformPopResult < ID_DBEDIT_FILTER_MENU_QUERYMAX)
					{ // layer
					int QueryNum = ConformPopResult - ID_DBEDIT_FILTER_MENU_QUERYBASE;
					SearchQuery *MyEffect;
					int Count = 0;
					for (MyEffect = (SearchQuery *)HostLib->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY); MyEffect; MyEffect = (SearchQuery *)MyEffect->Next)
						{
						if(Count == QueryNum)
							{
							strncpy(GlobalApp->MainProj->Prefs.DBESearchQueryFilterName, MyEffect->GetName(), WCS_EFFECT_MAXNAMELENGTH);
							QueryFilter = MyEffect;
							GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY;
							break;
							} // if
						Count++;
						} // for

					} // if
				else
					{
					GlobalApp->MainProj->Prefs.DBEFilterMode = WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE;
					} // else
				break;
				} // default
			} // switch
		BuildList(); // rebuild with new filter settings
		} // if
	DestroyMenu(FilterPopupMenu);
	FilterPopupMenu = NULL;
	} // if

} // DBEditGUI::DoFilterPopup


bool DBEditGUI::IsColumnALayer(int ColumnNumber)
{
if(ColumnNumber >= WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize() &&
 ColumnNumber < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize() + GetLayerColumnSize())
	{
	return(true);
	} // if

return(false);
} // DBEditGUI::IsColumnALayer

bool DBEditGUI::IsColumnAnAttribute(int ColumnNumber)
{
if(ColumnNumber >= WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize() + GetLayerColumnSize() &&
 ColumnNumber < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize() + GetLayerColumnSize() + GetAttribColumnSize())
	{
	return(true);
	} // if

return(false);
} // DBEditGUI::IsColumnAnAttribute


int DBEditGUI::GetAttribNumFromColumn(int ColumnNumber)
{
return(ColumnNumber - (WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize() + GetLayerColumnSize()));
} // DBEditGUI::GetAttribNumFromColumn


int DBEditGUI::GetLayerNumFromColumn(int ColumnNumber)
{
return(ColumnNumber - (WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + GetComponentColumnSize()));
} // DBEditGUI::GetLayerNumFromColumn



GridWidgetInstance::GridWidgetInstance()
{
GWStyles = NULL;
HeaderPopupMenu = NULL;
DBHost = NULL;
DBEHost = NULL;
HostLib = NULL;
ProjHost = NULL;
CheckedIcon = UnCheckedIcon = DiasbledCheckIcon = NULL;

MyControl = EditControl = ComboControl = NULL;
ActiveEditColumn = WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE;
ActiveEditItem = 0;

DistanceADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
DistanceADT.SetMultiplier(1.0);
DistanceADT.SetIncrement(1.0);

ElevationADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
ElevationADT.SetMultiplier(1.0);
ElevationADT.SetIncrement(1.0);

LatitudeADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
LatitudeADT.SetMultiplier(1.0);
LatitudeADT.SetIncrement(1.0);

LongitudeADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
LongitudeADT.SetMultiplier(1.0);
LongitudeADT.SetIncrement(1.0);

} // GridWidgetInstance::GridWidgetInstance


bool GridWidgetInstance::CreateEdit(int Item, int WhichColumn, bool ReadOnly, const char *InitialString)
{
DWORD ExtraStyleBits = NULL;
int X, Y, W, H;
RECT CellRect;

DestroySubControls();

if(ReadOnly) ExtraStyleBits = ES_READONLY;

ListView_GetSubItemRect(MyControl, Item, WhichColumn, LVIR_BOUNDS, &CellRect);
if(WhichColumn == 0)
	{ // ListView_GetSubItemRect doesn't work on column 0
	RECT TheRest;
	ListView_GetSubItemRect(MyControl, Item, 1, LVIR_BOUNDS, &TheRest);
	CellRect.right = TheRest.left - 1; // chop off anything after column 0
	} // if
X = CellRect.left - 1;
Y = CellRect.top - 2;
W = 3 + CellRect.right - CellRect.left;
H = 3 + CellRect.bottom - CellRect.top;
if(EditControl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", InitialString, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ExtraStyleBits, X, Y, W, H, MyControl, (HMENU)IDC_EDIT, GlobalApp->WinSys->Instance(), NULL))
	{
	// set font
	SendMessage(EditControl, WM_SETFONT, SendMessage(MyControl, WM_GETFONT, 0, 0), 0);
	SetFocus(EditControl);
	Edit_SetModify(EditControl, false);
	ActiveEditColumn = WhichColumn;
	ActiveEditItem = Item;
	} // if

return(false);
} // GridWidgetInstance::CreateEdit



bool GridWidgetInstance::CreateCombo(int Item, int WhichColumn, std::vector<std::string> &ComboChoices, int CurSel)
{
int X, Y, W, H;
RECT CellRect;
DestroySubControls();

ListView_GetSubItemRect(MyControl, Item, WhichColumn, LVIR_BOUNDS, &CellRect);
X = CellRect.left - 1;
Y = CellRect.top - 2;
W = 3 + CellRect.right - CellRect.left;
H = 3 + CellRect.bottom - CellRect.top;
if(ComboControl = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, X, Y, W, H, MyControl, (HMENU)IDC_DBEDGRIDDROP, GlobalApp->WinSys->Instance(), NULL))
	{
	// set font
	HDC ComboDC;
	HFONT ComboFont;
	int MaxTextWidth = 0;
	SIZE Ruler;

	SendMessage(ComboControl, WM_SETFONT, (WPARAM)(ComboFont = (HFONT)SendMessage(MyControl, WM_GETFONT, 0, 0)), 0);
	ComboDC = GetDC(ComboControl);
	SetFocus(ComboControl);
	// populate
	for(std::vector<std::string>::iterator ComboChoiceWalk = ComboChoices.begin(); ComboChoiceWalk != ComboChoices.end(); ComboChoiceWalk++)
		{
		ComboBox_AddString(ComboControl, (*ComboChoiceWalk).c_str());
		GetTextExtentPoint32(ComboDC, (*ComboChoiceWalk).c_str(), (int)strlen((*ComboChoiceWalk).c_str()), &Ruler);
		if(Ruler.cx > MaxTextWidth) MaxTextWidth = Ruler.cx;
		} // for
	// set width
	SendMessage(ComboControl, CB_SETDROPPEDWIDTH, MaxTextWidth, 0);
	// set current selection
	ComboBox_SetCurSel(ComboControl, CurSel);
	// activate
	ComboBox_ShowDropdown(ComboControl, true);
	ActiveEditColumn = WhichColumn;
	ActiveEditItem = Item;
	} // if

return(false);
} // GridWidgetInstance::CreateCombo


void GridWidgetInstance::DestroySubControls(void)
{
if(ActiveEditColumn != WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE)
	{
	ActiveEditColumn = WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE;
	if(EditControl)
		{
		DestroyWindow(EditControl);
		EditControl = NULL;
		} // if
	if(ComboControl)
		{
		DestroyWindow(ComboControl);
		ComboControl = NULL;
		} // if
	} // if

} // GridWidgetInstance::DestroySubControls

static char *StyleList[] = {"Point", "Circle", "Square", "Cross", "Solid", "Dotted", "Dashed", "Broken", NULL};
char ScratchFormatBuffer[2048]; // used for returning text from FetchTextForColumn
const char *GridWidgetInstance::FetchTextForColumn(int Item, int WhichColumn)
{
const char *ResultText = "";
double North, South, East, West;
Joe *JoeItem = NULL;
char AssembleBuffer[200];

// extract the Joe
if (GridWidgetCache)
	{
	JoeItem = (*(GridWidgetCache))[Item];
	} // if
	
if (JoeItem)
	{
	North = JoeItem->GetNorth();
	South = JoeItem->GetSouth();
	West = JoeItem->GetWest();
	East = JoeItem->GetEast();
	
	switch(WhichColumn)
		{
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
			{
			// Item text (Name)
			ResultText = JoeItem->FileName();
			if (ResultText == NULL)
				{
				ResultText = JoeItem->CanonName();
				} // if
			if (ResultText == NULL)
				{
				ResultText = JoeItem->SecondaryName();
				} // if
			if (ResultText == NULL)
				{ // "Unnamed"
				if (JoeItem->Name() == NULL)
					{
					ResultText = "Unnamed";
					} // if
				else
					{ // we'll have some kind of name in the Label field
					ResultText = "";
					} // else				
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL:
			{
			ResultText = JoeItem->Name();
			if (ResultText == NULL)
				{
				ResultText = "";
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED:
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW:
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER:
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR:
			{ // no text
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
			{
			if (JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				ResultText = "DEM";
				} // if
			else if (JoeItem->TestFlags(WCS_JOEFLAG_ISCONTROL))
				{
				ResultText = "Control Points";
				} // if
			else
				{
				ResultText = "Vector";
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
			{
			if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				ResultText = StyleList[JoeItem->GetLineStyle()];
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT:
			{
			if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				sprintf(ScratchFormatBuffer, "%d", JoeItem->GetLineWidth());
				ResultText = ScratchFormatBuffer;
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD:
			{
			JoeDEM *JD;
			if (JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				sprintf(ScratchFormatBuffer, "%d", JD->MaxFract);
				ResultText = ScratchFormatBuffer;
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA:
			{
			char HorDistUnitsMetric;
			double Area = 0.0;
			char *UnitSuffix = "";
			if (GlobalApp->MainProj->Prefs.HorDisplayUnits <= WCS_USEFUL_UNIT_MEGAMETER)
				HorDistUnitsMetric = 1;
			else
				HorDistUnitsMetric = 0;
			if (JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				Area = 0.0;
				if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
					{
					if(JD->QueryCachedELEVDataValid())
						{
						Area = JD->DEMGridNS * (JD->pLatEntries - 1) * JD->DEMGridWE * (JD->pLonEntries - 1) / 10000.0;
						} // if
					} // if
				} // if
			else
				{
				if (West <= 360.0 && East >= - 360.0 && North <= 90.0 && South >= -90.0)
					{
					Area = JoeItem->ComputeAreaDegrees();
					// figure value in tenths of a kilometer so multiplied they give hectares
					Area *= (LatScale(HostLib->GetPlanetRadius()) * .01);
					Area *= (LonScale(HostLib->GetPlanetRadius(), (North + South) * .5) * .01);
					} // if
				else
					Area = 0.0;
				} // else


			if (HorDistUnitsMetric)
				{
				if (Area < 100.0)
					{
					//Area in hectares
					UnitSuffix = "ha";
					} // if
				else
					{
					//Area in sq kilom
					UnitSuffix = "sq Km";
					Area *= .01;
					} // else
				} // if
			else
				{
				// hectares to acres
				Area *= 2.47;
				if (Area < 100.0)
					{
					// Area in acres
					UnitSuffix = "ac";
					} // if
				else
					{
					// Area in sq Mi
					UnitSuffix = "sq Mi";
					Area *= .0015625;
					} // else
				} // else
			sprintf(AssembleBuffer, "%.3f", Area);
			TrimZeros(AssembleBuffer);
			sprintf(ScratchFormatBuffer, "%s %s", AssembleBuffer, UnitSuffix);
			ResultText = ScratchFormatBuffer;
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS:
			{
			if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				sprintf(ScratchFormatBuffer, "%d", JoeItem->GetNumRealPoints());
				ResultText = ScratchFormatBuffer;
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH:
			{
			double VecLength;
			if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				VecLength = JoeItem->GetVecLength(0);
				FormatAsPreferredUnit(ScratchFormatBuffer, &DistanceADT, VecLength);
				ResultText = ScratchFormatBuffer;
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					sprintf(ScratchFormatBuffer, "%d", JD->pLonEntries);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					sprintf(ScratchFormatBuffer, "%d", JD->pLatEntries);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					FormatAsPreferredUnit(ScratchFormatBuffer, &ElevationADT, (double)JD->MaxEl);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					FormatAsPreferredUnit(ScratchFormatBuffer, &ElevationADT, (double)JD->MinEl);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH:
			{
			if (North > (DBL_MAX / 100)) ResultText = "~DBL_MAX";
			else if (North < -(DBL_MAX / 100)) ResultText = "~-DBL_MAX";
			else if (North > (FLT_MAX / 100)) ResultText = "~FLT_MAX";
			else if (North < -(FLT_MAX / 100)) ResultText = "~-FLT_MAX";
			else if (North > 100000) ResultText = "OVERFLOW";
			else
				{
				FormatAsPreferredUnit(ScratchFormatBuffer, &LatitudeADT, North);
				ResultText = ScratchFormatBuffer;
				} // else
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH:
			{
			if (South > (DBL_MAX / 100)) ResultText = "~DBL_MAX";
			else if (South < -(DBL_MAX / 100)) ResultText = "~-DBL_MAX";
			else if (South > (FLT_MAX / 100)) ResultText = "~FLT_MAX";
			else if (South < -(FLT_MAX / 100)) ResultText = "~-FLT_MAX";
			else if (South > 100000) ResultText = "OVERFLOW";
			else
				{
				FormatAsPreferredUnit(ScratchFormatBuffer, &LatitudeADT, South);
				ResultText = ScratchFormatBuffer;
				} // else
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST:
			{
			if (East > (DBL_MAX / 100)) ResultText = "~DBL_MAX";
			else if (East < -(DBL_MAX / 100)) ResultText = "~-DBL_MAX";
			else if (East > (FLT_MAX / 100)) ResultText = "~FLT_MAX";
			else if (East < -(FLT_MAX / 100)) ResultText = "~-FLT_MAX";
			else if (East > 100000) ResultText = "OVERFLOW";
			else
				{
				FormatAsPreferredUnit(ScratchFormatBuffer, &LongitudeADT, East);
				ResultText = ScratchFormatBuffer;
				} // else
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST:
			{
			if (West > (DBL_MAX / 100)) ResultText = "~DBL_MAX";
			else if (West < -(DBL_MAX / 100)) ResultText = "~-DBL_MAX";
			else if (West > (FLT_MAX / 100)) ResultText = "~FLT_MAX";
			else if (West < -(FLT_MAX / 100)) ResultText = "~-FLT_MAX";
			else if (West > 100000) ResultText = "OVERFLOW";
			else
				{
				FormatAsPreferredUnit(ScratchFormatBuffer, &LongitudeADT, West);
				ResultText = ScratchFormatBuffer;
				} // else
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					FormatAsPreferredUnit(ScratchFormatBuffer, &DistanceADT, JD->DEMGridNS);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW:
			{
			if (JoeDEM *JD = (JoeDEM *)JoeItem->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				if(JD->QueryCachedELEVDataValid())
					{
					FormatAsPreferredUnit(ScratchFormatBuffer, &DistanceADT, JD->DEMGridWE);
					ResultText = ScratchFormatBuffer;
					} // if
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW
		default:
			{
			if(DBEHost->IsColumnAnAttribute(WhichColumn))
				{
				RECT CellRect; //, CheckRect;
				ListView_GetSubItemRect(MyControl, Item, WhichColumn, LVIR_BOUNDS, &CellRect);
				if(CellRect.right - CellRect.left > 0) // ignore draw notifies when our cell isn't visible, duh.
					{
					int AttribNum = DBEHost->GetAttribNumFromColumn(WhichColumn);
					LayerEntry *ThisLayer = DBEHost->GetAttribColumn(AttribNum);
					if(ThisLayer)
						{
						LayerStub *MyTurn = JoeItem->MatchLayer(ThisLayer);
						if(MyTurn)
							{
							#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
							//const char *MyName = ThisLayer->GetName(); // for debugging
							if (ThisLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
								{
								if (! ThisLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
									{
									if (ThisLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
										{
										//sprintf(ScratchFormatBuffer, "%s:%s", &MyName[1], MyTurn->GetTextAttribVal());
										sprintf(ScratchFormatBuffer, "%s", MyTurn->GetTextAttribVal());
										} // if
									else
										{
										//sprintf(ScratchFormatBuffer, "%s:%g", &MyName[1], MyTurn->GetIEEEAttribVal());
										sprintf(ScratchFormatBuffer, "%g", MyTurn->GetIEEEAttribVal());
										} // else
									ResultText = ScratchFormatBuffer;
									} // if not link
								} // if
							#else // !WCS_SUPPORT_GENERIC_ATTRIBS
							#endif // !WCS_SUPPORT_GENERIC_ATTRIBS
							} // if
						} // if
					} // if visible
				} // if
			else if(WhichColumn == WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX) // Component
				{
				JoeAttribute *MyTurn;
				if (JoeItem)
					{
					if (MyTurn = JoeItem->GetFirstAttribute())
						{
						bool OneAdded = false;
						std::string StrAssemble;
						while (MyTurn)
							{
							if (MyTurn->MajorAttrib() == WCS_JOE_ATTRIB_INTERNAL && MyTurn->MinorAttrib() != WCS_JOE_ATTRIB_INTERNAL_DEM)
								{
								const char *EffectNameClass;
								if(EffectNameClass = DBEHost->ParseEffectNameClass(MyTurn, false))
									{
									if(OneAdded) StrAssemble += ", ";
									StrAssemble += EffectNameClass;
									OneAdded = true;
									} // if
								} // if
							MyTurn = MyTurn->GetNextAttribute();
							} // while
						if(StrAssemble.length())
							{
							strcpy(ScratchFormatBuffer, StrAssemble.c_str());
							ResultText = ScratchFormatBuffer;
							} // if
						} // if
					} // if
				} // else if
			break;
			} // default
		} // WhichColumn
	} // if
return(ResultText);
} // GridWidgetInstance::FetchTextForColumn

bool GridWidgetInstance::FillColumnChoices(int Item, int WhichColumn, std::vector<std::string> &ComboChoices)
{
Joe *JoeItem = NULL;

// extract the Joe
if (GridWidgetCache)
	{
	JoeItem = (*(GridWidgetCache))[Item];
	} // if
	
if (JoeItem)
	{
	switch(WhichColumn)
		{
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
			{
			if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				ComboChoices.push_back("Vector");
				ComboChoices.push_back("Control Points");
				} // if
			return(true);
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
			{
			char *StyleAdd;
			int Loop = 0;
			for(StyleAdd = StyleList[Loop]; StyleList[Loop]; Loop++)
				{
				ComboChoices.push_back(StyleList[Loop]);
				} // for
			return(true);
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE
		default:
			{
			break;
			} // default
		} // WhichColumn
	} // if
return(false);
} // GridWidgetInstance::FillColumnChoices

int GridWidgetInstance::QueryActiveColumnChoice(int Item, int WhichColumn)
{
Joe *JoeItem = NULL;

// extract the Joe
if (GridWidgetCache)
	{
	JoeItem = (*(GridWidgetCache))[Item];
	} // if
	
if (JoeItem)
	{
	switch(WhichColumn)
		{
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
			{
			if (JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				return(0); // N/A
				} // if
			else if (JoeItem->TestFlags(WCS_JOEFLAG_ISCONTROL))
				{
				return(0);
				} // if
			else
				{
				return(1);
				} // if
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS
		case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
			{
			return(JoeItem->GetLineStyle());
			break;
			} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE
		default:
			{
			break;
			} // default
		} // WhichColumn
	} // if
return(0);
} // GridWidgetInstance::QueryActiveColumnChoice

bool GridWidgetInstance::HandleNewTextForColumn(int Item, int WhichColumn, const char *NewText)
{
switch(WhichColumn)
	{
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
		{
		ChangeNames(DBEHost->GetJoeFromOBN(Item), ProjHost, NULL, NewText); // do the item we clicked
		DBEHost->DoRename(NewText, NULL); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL:
		{
		ChangeNames(DBEHost->GetJoeFromOBN(Item), ProjHost, NewText, NULL); // do the item we clicked
		DBEHost->DoRename(NULL, NewText); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT:
		{
		int NewWeight = atoi(NewText);
		SetLineWeight(DBEHost->GetJoeFromOBN(Item), NewWeight); // do the item we clicked
		DBEHost->DoLineWeight(NewWeight); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD:
		{
		int NewFRD = atoi(NewText);
		SetMaxFract(DBEHost->GetJoeFromOBN(Item), NewFRD); // do the item we clicked
		DBEHost->DoMaxFractal(NewFRD); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX: // component, should be readonly and we won't hit this, but...
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD
	default:
		{
		// handle attributes
		if(DBEHost->IsColumnAnAttribute(WhichColumn))
			{
			// ChangeAttribValue handles the changed item and selection set in one whack
			DBEHost->ChangeAttribValue(Item, WhichColumn, NewText);
			} // if
		break;
		} // default
	} // WhichColumn
return(false);
} // GridWidgetInstance::HandleNewTextForColumn

bool GridWidgetInstance::HandleNewChoiceForColumn(int Item, int WhichColumn, int NewChoice)
{
switch(WhichColumn)
	{
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
		{
		SetClass(DBEHost->GetJoeFromOBN(Item), NewChoice); // do the operated item
		DBEHost->DoObjectClass(NewChoice); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
		{
		SetLineStyle(DBEHost->GetJoeFromOBN(Item), NewChoice); // do the operated item
		DBEHost->DoLineStyle(NewChoice); // do the rest of the selection set
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE
	default:
		{
		break;
		} // default
	} // WhichColumn
return(false);
} // GridWidgetInstance::HandleNewChoiceForColumn


/*
switch(WhichColumn)
	{
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS
	case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW:
		{
		break;
		} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW
	default:
		{
		break;
		} // default
	} // WhichColumn
*/


long FAR PASCAL DBGridWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
GridWidgetInstance *WThis;

WThis = (GridWidgetInstance *)GetWindowLong(hwnd, ListViewOriginalWndExtra);

switch (message)
	{
	case WM_WCSW_GW_LVNELEREFLECT: // LVN_ENDLABELEDIT reflected from parent window for subclassing
		{
		if (WThis && WThis->GWStyles & WCSW_GW_STYLE_JOE)
			{
			Joe *JoeItem = NULL;
			NMLVDISPINFO *pdi = (NMLVDISPINFO *) lParam;
			JoeItem = (Joe *)(pdi->item.lParam);
			} // if
		break;
		} // WM_WCSW_GW_LVNELEREFLECT
	case WM_WCSW_GW_LVNBLEREFLECT: // LVN_BEGINLABELEDIT reflected from parent window for subclassing
		{
		if (WThis && WThis->GWStyles & WCSW_GW_STYLE_JOE)
			{
			Joe *JoeItem = NULL;
			NMLVDISPINFO *pdi = (NMLVDISPINFO *) lParam;
			JoeItem = (Joe *)(pdi->item.lParam);
			SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)TRUE);
			return(true);
			} // if
		break;
		} // WM_WCSW_GW_LVNBLEREFLECT
	case WM_WCSW_GW_LVNGDIREFLECT: // LVN_GETDISPINFO reflected from parent window for subclassing
		{
		if (WThis && WThis->GWStyles & WCSW_GW_STYLE_JOE)
			{
			NMLVDISPINFO *pdi = (NMLVDISPINFO*)lParam;
			
			if (1)
				{
				// is the list looking for text?
				if (pdi->item.mask & LVIF_TEXT)
					{
					const char *ResultText = "";
					// Which column?
					switch (pdi->item.iSubItem)
						{
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL: // subitem 1
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST:
							{
							ResultText = WThis->FetchTextForColumn(pdi->item.iItem, pdi->item.iSubItem);
							break;
							} // item
						// high-cost items that should be bounds-checked before fetching
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA:
						case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH:
							{
							RECT CellRect;
							ListView_GetSubItemRect(hwnd, pdi->item.iItem, pdi->item.iSubItem, LVIR_BOUNDS, &CellRect);
							if(CellRect.right - CellRect.left > 0) // ignore draw notifies when our cell isn't visible, duh.
								{
								ResultText = WThis->FetchTextForColumn(pdi->item.iItem, pdi->item.iSubItem);
								} // if
							break;
							} // high cost item
						default:
							{
							// Attribs, Components, Layers
							ResultText = WThis->FetchTextForColumn(pdi->item.iItem, pdi->item.iSubItem);
							break;
							} // default
						} // switch

					//Maximum number of characters is in pItem->cchTextMax
					strncpy(pdi->item.pszText, ResultText, pdi->item.cchTextMax);
					} // if
				} // if
			} // if
		break;
		} // WM_WCSW_GW_LVNGDIREFLECT
	case WM_WCSW_GW_LVNOCHREFLECT: // LVN_ODCACHEHINT reflected from parent window for subclassing
		{
		break;
		} // WM_WCSW_GW_LVNOCHREFLECT
	case WM_WCSW_GW_LVNBSREFLECT:
		{
		WThis->DestroySubControls(); // cancel edit in progress
		break;
		} // WM_WCSW_GW_LVNBSREFLECT
	case WM_WCSW_GW_NMCDREFLECT: // NM_CUSTOMDRAW reflected from parent window for subclassing
		{
		LPNMLVCUSTOMDRAW CDraw = (LPNMLVCUSTOMDRAW) lParam;
		switch (CDraw->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				{
				SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NOTIFYITEMDRAW);
				return(true);
				} // CDDS_PREPAINT
			case CDDS_ITEMPREPAINT:
				{
				SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NOTIFYSUBITEMDRAW);
				return(true);
				} // CDDS_ITEMPREPAINT
			case CDDS_SUBITEM | CDDS_ITEMPOSTPAINT:
				{
				Joe *JoeItem = NULL;
				if (WThis && WThis->GWStyles & WCSW_GW_STYLE_JOE)
					{
					// extract the Joe
					if (WThis->GridWidgetCache)
						{
						JoeItem = (*(WThis->GridWidgetCache))[CDraw->nmcd.dwItemSpec];
						} // if
					if (JoeItem)
						{
						switch (CDraw->iSubItem)
							{
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED:
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW:
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER:
								{
								long FlagTest = WCS_JOEFLAG_ACTIVATED;
								HICON State;
								RECT CellRect; //, CheckRect;
								if(CDraw->iSubItem == WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW) FlagTest = WCS_JOEFLAG_DRAWENABLED;
								else if(CDraw->iSubItem == WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER) FlagTest = WCS_JOEFLAG_RENDERENABLED;
								ListView_GetSubItemRect(hwnd, CDraw->nmcd.dwItemSpec, CDraw->iSubItem, LVIR_BOUNDS, &CellRect);
								if(CellRect.right - CellRect.left > 0) // ignore draw notifies when our cell isn't visible, duh.
									{
									State = JoeItem->TestFlags(FlagTest) ? WThis->CheckedIcon : WThis->UnCheckedIcon;
									if(CDraw->iSubItem != WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED && !JoeItem->TestFlags(WCS_JOEFLAG_ACTIVATED))
										State = WThis->DiasbledCheckIcon;
									DrawState(CDraw->nmcd.hdc, NULL, NULL, (LPARAM)State, NULL, ((CellRect.left + CellRect.right) / 2) - 7, CellRect.top, 16, 16, DST_ICON | DSS_NORMAL);
									} // if
								break;
								} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED/VIEW/RENDER
							default:
								{
								if(WThis->DBEHost->IsColumnALayer(CDraw->iSubItem))
									{
									// custom post-draw for Layer checkboxes
									int State;
									int LayerNum = WThis->DBEHost->GetLayerNumFromColumn(CDraw->iSubItem);
									LayerEntry *ThisLayer = WThis->DBEHost->GetLayerColumn(LayerNum);
									RECT CellRect; //, CheckRect;
									ListView_GetSubItemRect(hwnd, CDraw->nmcd.dwItemSpec, CDraw->iSubItem, LVIR_BOUNDS, &CellRect);
									if(CellRect.right - CellRect.left > 0) // ignore draw notifies when our cell isn't visible, duh.
										{
										State = JoeItem->MatchLayer(ThisLayer) ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
										if(State == CBS_CHECKEDNORMAL)
											{
											DrawState(CDraw->nmcd.hdc, NULL, NULL, (LPARAM)WThis->CheckedIcon, NULL, ((CellRect.left + CellRect.right) / 2) - 7, CellRect.top, 16, 16, DST_ICON | DSS_NORMAL);
											} // if
										} // if
									} // if
								break;
								} // other
							} // iSubItem
						} // if JoeItem
					} // if
				return(true);
				} // CDDS_SUBITEM | CDDS_ITEMPOSTPAINT
			case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
				{
				Joe *JoeItem = NULL;
				bool Selected = false;
				// this stuff is to implement retention of the blue highlight color when the widget loses focus
				// something that Windows doesn't offer by itself.
				// we can't trust the CDraw->nmcd.uItemState to tell if the item is selected, since the
				// widget doesn't flag that when not focused, so we have to look it up ourselves.
				LVITEM LVI;
				LVI.mask = LVIF_STATE;
				LVI.stateMask = LVIS_SELECTED;
				LVI.iItem = CDraw->nmcd.dwItemSpec;
				if(ListView_GetItem(hwnd, &LVI))
					{
					if(LVI.state & LVIS_SELECTED) Selected = true;
					} // if

				if (WThis->GWStyles & WCSW_GW_STYLE_GREENBAR) // actually light-blue bar
					{
					// change color if necessary
					if (Selected)
						{ // draw ourselves in the (normally blue) highlight color in case we don't have the focus
						CDraw->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
						SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
						} // if
					else
						{
						if (CDraw->nmcd.dwItemSpec % 2)
							{
							CDraw->clrTextBk = WINGDI_RGB(240, 240, 255); // light blue
							SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
							} // if
						else
							{
							CDraw->clrTextBk = CLR_DEFAULT;
							SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
							} // else
						} // else
					} // if
				if (WThis && WThis->GWStyles & WCSW_GW_STYLE_JOE)
					{
					// extract the Joe
					if (WThis->GridWidgetCache)
						{
						JoeItem = (*(WThis->GridWidgetCache))[CDraw->nmcd.dwItemSpec];
						} // if
					if (JoeItem)
						{
						switch (CDraw->iSubItem)
							{
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL:
								{
								if(!Selected) // selected color (especially when widget unfocused) has to override custom colors
									{
									if(!JoeItem->TestFlags(WCS_JOEFLAG_ACTIVATED))
										{
										CDraw->clrTextBk = WINGDI_RGB(140, 140, 140);
										SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
										} // if
									else if(!JoeItem->TestFlags(WCS_JOEFLAG_DRAWENABLED))
										{
										if(!JoeItem->TestFlags(WCS_JOEFLAG_RENDERENABLED))
											{ // both disabled, same as disabled above
											CDraw->clrTextBk = WINGDI_RGB(140, 140, 140);
											SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
											} // if
										else
											{ // View disabled, lighter grey
											CDraw->clrTextBk = WINGDI_RGB(230, 230, 230);
											SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
											} // else
										} // else if
									else if(!JoeItem->TestFlags(WCS_JOEFLAG_RENDERENABLED))
										{ // Render disabled, sorta lighter grey
										CDraw->clrTextBk = WINGDI_RGB(200, 200, 200);
										SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
										} // else if
									} // if
								break;
								} // name
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED:
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW:
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER:
								{
								// custom post-draw for checkboxes
								SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NOTIFYPOSTPAINT);
								break;
								} // WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER
							case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR:
								{
								if(!Selected) // selected color (especially when widget unfocused) has to override custom colors
									{
									// will over-ride greenbar handling, above
									if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM))
										{
										CDraw->clrTextBk = WINGDI_RGB(JoeItem->Red(), JoeItem->Green(), JoeItem->Blue());
										SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NEWFONT);
										} // if
									} // if
								break;
								} // color
							default:
								{
								if(CDraw->iSubItem >= WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + WThis->DBEHost->GetComponentColumnSize() &&
								 CDraw->iSubItem < WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + WThis->DBEHost->GetComponentColumnSize() + WThis->DBEHost->GetLayerColumnSize())
									{
									// custom post-draw for layer checkboxes
									SetWindowLong(GetParent(hwnd), DWL_MSGRESULT, (LONG)CDRF_NOTIFYPOSTPAINT);
									} // if
								break;
								} // other
							} // iSubItem
						} // if JoeItem
					} // if
				return(true);
				} // CDDS_SUBITEM | CDDS_ITEMPREPAINT
			} // switch dwDrawStage
		return(1);
		} // WM_WCSW_GW_NMCDREFLECT
	case WM_COMMAND: // from child combobox or edit etc controls
		{
		WORD Notify = HIWORD(wParam);
		WORD ControlID = LOWORD(wParam);
		switch(Notify)
			{
			case EN_KILLFOCUS:
				{
				// fetch changed value (if modified)
				if(WThis->ActiveEditColumn != WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE && WThis->EditControl && Edit_GetModify(WThis->EditControl))
					{
					char TextBuffer[512];
					Edit_GetText(WThis->EditControl, TextBuffer, 511);
					WThis->HandleNewTextForColumn(WThis->ActiveEditItem, WThis->ActiveEditColumn, TextBuffer);
					} // if
				WThis->DestroySubControls();
				break;
				} // KILLFOCUS
			case CBN_KILLFOCUS:
			case CBN_SELENDCANCEL:
				{ // ignore changed value
				WThis->DestroySubControls();
				break;
				} // CBN_SELENDOK / CBN_KILLFOCUS
			case CBN_SELENDOK:
				{
				// fetch changed value
				if(WThis->ActiveEditColumn != WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE && WThis->ComboControl)
					{
					int Choice;
					Choice = ComboBox_GetCurSel(WThis->ComboControl);
					if(Choice != CB_ERR)
						{
						WThis->HandleNewChoiceForColumn(WThis->ActiveEditItem, WThis->ActiveEditColumn, Choice);
						} // if
					} // if
				WThis->DestroySubControls();
				break;
				} // CBN_SELENDOK / CBN_KILLFOCUS
			} // notify
		break;
		} // WM_COMMAND
	case WM_NOTIFY: // from child Header control
		{
		bool ItemValid = false;
		int ItemNum = 0;
		LPNMHDR pnm = (LPNMHDR)lParam;
		POINT CursorPos;
		HDITEM NotifyItem;
		GetCursorPos(&CursorPos);
		if (pnm->code == HDN_ITEMCLICK || pnm->code == HDN_ITEMCLICKW) // not sure why we get HDN_ITEMCLICKW, but we do
			{ // LBM action gives us the item info for free
			LPNMHEADER lpnmheader = (LPNMHEADER)lParam;
			if (lpnmheader)
				{
				ItemNum = lpnmheader->iItem;
				ItemValid = true;
				} // if
			} // if
		else if (pnm->code == NM_RCLICK)
			{ // from a RMB, we need to identify the header item ourselves
			for(int ItemHit = 0; ; ItemHit++)
				{
				RECT HeaderRECT;
				POINT ClientCursorPos;
				ClientCursorPos.x = CursorPos.x;
				ClientCursorPos.y = CursorPos.y;
				ScreenToClient(pnm->hwndFrom, &ClientCursorPos);
				if (Header_GetItemRect(pnm->hwndFrom, ItemHit, &HeaderRECT))
					{
					if(ClientCursorPos.x > HeaderRECT.left && ClientCursorPos.x < HeaderRECT.right && ClientCursorPos.y > HeaderRECT.top && ClientCursorPos.y < HeaderRECT.bottom)
						{
						ItemNum = ItemHit;
						ItemValid = true;
						break;
						} // if
					} // if
				else
					{
					break;
					} // else
				} // for
			} // if NM_RCLICK
		if(ItemValid)
			{
			if(1) // to preserve indent for now
				{
				if (Header_GetItem(pnm->hwndFrom, ItemNum, &NotifyItem))
					{
					int LayerNum = 0;
					LayerEntry *PopLayer = NULL;
					RasterAnimHost *PopHost = NULL;
					LayerNum = ItemNum - (WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX + WThis->DBEHost->GetComponentColumnSize());
					if(LayerNum >= 0 && LayerNum < WThis->DBEHost->GetLayerColumnSize()) PopLayer = WThis->DBEHost->GetLayerColumn(LayerNum);
					if (PopHost = PopLayer)
						{
						long HeaderPopResult;
						char Header[255];
						RasterAnimHostProperties HostProp;
						HostProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
						HostProp.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
						PopHost->GetRAHostProperties(&HostProp);

						sprintf(Header, "%s %s", HostProp.Name, HostProp.Type);

						WThis->HeaderPopupMenu = CreatePopupMenu();
						PopMenuAdder Paddy(WThis->HeaderPopupMenu); // create new popmenuadder with our root PopupMenu
						if (HostProp.Name[0] || HostProp.Type[0]) AppendMenu(WThis->HeaderPopupMenu, MF_STRING | MF_DISABLED, 58000, Header);
						AppendMenu(WThis->HeaderPopupMenu, MF_SEPARATOR | MF_ENABLED, 58000, NULL);
						PopHost->AddBasePopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_GLOBAL);
						PopHost->AddDerivedPopMenus(&Paddy, (unsigned long)WCS_RAH_POPMENU_CLASS_GLOBAL);

						HeaderPopResult = TrackPopupMenu(WThis->HeaderPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, CursorPos.x, CursorPos.y, 0, hwnd, NULL);
						if (HeaderPopResult)
							{
							int ActionID, Derived = 0;
							if (HeaderPopResult < 59000)
								{
								ActionID = HeaderPopResult - 58001;
								} // if
							else
								{
								Derived = 1;
								ActionID = HeaderPopResult - 59001;
								} // else
							if (ActionID > -1) // Dummy entries (58000) result in -1
								{
								const char *ActionText = (const char *)Paddy.GetAction(ActionID);
								if (Derived)
									{
									} // if
								else
									{
									if (!stricmp(ActionText, "ENABLE"))
										{
										RasterAnimHostProperties Prop;
										Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
										Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
										Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
										PopHost->SetRAHostProperties(&Prop);
										} // if
									else if (!stricmp(ActionText, "DISABLE"))
										{
										RasterAnimHostProperties Prop;
										Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
										Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
										Prop.Flags = 0;
										PopHost->SetRAHostProperties(&Prop);
										} // if
									else if (!stricmp(ActionText, "PURGELAYERS"))
										{
										NotifyTag Changes[2];
										Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
										Changes[1] = NULL;
										GlobalApp->AppEx->GenerateNotify(Changes, NULL);
										WThis->DBHost->RemoveUnusedLayers();
										Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
										Changes[1] = NULL;
										GlobalApp->AppEx->GenerateNotify(Changes, NULL);
										} // if
									else if (!stricmp(ActionText, "DELETE"))
										{
										WThis->DBHost->RemoveRAHost((LayerEntry *)PopHost, WThis->ProjHost, WThis->HostLib, 1, 1);
										} // if
									else if (!stricmp(ActionText, "SELECTLAYER"))
										{
										int Qualifier = 0;
										RasterAnimHostProperties Prop;

										Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
										Prop.FlagsMask = 0;
										PopHost->GetRAHostProperties(&Prop);

										if (Prop.TypeNumber == WCS_SUBCLASS_LAYER)
											{
											GlobalApp->GUIWins->DBE->DoLayerSelect((LayerEntry *)PopHost, Qualifier);
											} // if
										} // if
									} // else
								} // if
							} // if

						} // if
					} // if
				} // if
			} // if ItemValid
		break;
		} // WM_NOTIFY
	case WM_LBUTTONDBLCLK:
		{
		LVHITTESTINFO HitTest;
		HitTest.pt.x = LOWORD(lParam);
		HitTest.pt.y = HIWORD(lParam);
		int HitItem = ListView_SubItemHitTest(hwnd, &HitTest);
		if(HitItem != -1)
			{
			Joe *JoeItem = NULL;
			if (WThis->GridWidgetCache)
				{
				JoeItem = (*(WThis->GridWidgetCache))[HitItem];
				} // if
			if (JoeItem)
				{
				if(HitTest.iSubItem == WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME || HitTest.iSubItem == WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL)
					{
					std::string TempString = WThis->FetchTextForColumn(HitItem, HitTest.iSubItem);
					WThis->CreateEdit(HitItem, HitTest.iSubItem, false, TempString.c_str());
					} // iSubItem
				} // if
			} // if
		return(0); // so we don't pass along
		} // WM_LBUTTONDBLCLK
	case WM_LBUTTONDOWN:
		{
		LVHITTESTINFO HitTest;
		HitTest.pt.x = LOWORD(lParam);
		HitTest.pt.y = HIWORD(lParam);
		int HitItem = ListView_SubItemHitTest(hwnd, &HitTest);
		if(HitItem != -1)
			{
			Joe *JoeItem = NULL;
			if (WThis->GridWidgetCache)
				{
				JoeItem = (*(WThis->GridWidgetCache))[HitItem];
				} // if
			if (JoeItem)
				{
				switch(HitTest.iSubItem)
					{
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED:
						{
						// get current state of item and toggle
						bool NewState = (JoeItem->TestFlags(WCS_JOEFLAG_ACTIVATED) ? false : true);
						if(NewState) JoeItem->SetFlags(WCS_JOEFLAG_ACTIVATED);
						else JoeItem->ClearFlags(WCS_JOEFLAG_ACTIVATED);
						WThis->DBEHost->DoEnable(NewState);
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW:
						{
						// get current state of item and toggle
						bool NewState = (JoeItem->TestFlags(WCS_JOEFLAG_DRAWENABLED) ? false : true);
						if(NewState) JoeItem->SetFlags(WCS_JOEFLAG_DRAWENABLED);
						else JoeItem->ClearFlags(WCS_JOEFLAG_DRAWENABLED);
						WThis->DBEHost->DoDrawEnable(NewState);
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER:
						{
						// get current state of item and toggle
						bool NewState = (JoeItem->TestFlags(WCS_JOEFLAG_RENDERENABLED) ? false : true);
						if(NewState) JoeItem->SetFlags(WCS_JOEFLAG_RENDERENABLED);
						else JoeItem->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
						WThis->DBEHost->DoRenderEnable(NewState);
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR:
						{
						// edit color
						WThis->DBEHost->VectorColor.SetValue3(JoeItem->Red() / 255.0, JoeItem->Green() / 255.0, JoeItem->Blue() / 255.0);
						WThis->DBEHost->VectorColor.ValueChanged();
						WThis->DBEHost->VectorColor.EditRAHost();
						WThis->DBEHost->ActiveColorEditItem = HitItem;
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS:
						{
						if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM)) // can't change style on DEM
							{
							std::vector<std::string> ComboChoices;
							if(WThis->FillColumnChoices(HitItem, HitTest.iSubItem, ComboChoices))
								{
								WThis->CreateCombo(HitItem, HitTest.iSubItem, ComboChoices, 0);
								} // if
							} // if
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE:
						{
						if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM)) // can't change style on DEM
							{
							std::vector<std::string> ComboChoices;
							if(WThis->FillColumnChoices(HitItem, HitTest.iSubItem, ComboChoices))
								{
								WThis->CreateCombo(HitItem, HitTest.iSubItem, ComboChoices, WThis->QueryActiveColumnChoice(HitItem, HitTest.iSubItem));
								} // if
							} // if
						break;
						} // 
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT:
						{ // conditionally editable text
						if (!JoeItem->TestFlags(WCS_JOEFLAG_ISDEM)) // not editable on DEMs
							{
							std::string TempString = WThis->FetchTextForColumn(HitItem, HitTest.iSubItem);
							WThis->CreateEdit(HitItem, HitTest.iSubItem, false, TempString.c_str());
							break;
							} // if
						break;
						} // text
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD:
						{ // conditionally editable text
						if (JoeItem->TestFlags(WCS_JOEFLAG_ISDEM)) // only editable on DEMs
							{
							std::string TempString = WThis->FetchTextForColumn(HitItem, HitTest.iSubItem);
							WThis->CreateEdit(HitItem, HitTest.iSubItem, false, TempString.c_str());
							break;
							} // if
						break;
						} // text
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL:
						{ // Handled in DOUBLECLICK, above
						break;
						} // text
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW:
					case WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX: // component
						{ // read-only text
						std::string TempString = WThis->FetchTextForColumn(HitItem, HitTest.iSubItem);
						if(!TempString.empty())
							{
							WThis->CreateEdit(HitItem, HitTest.iSubItem, true, TempString.c_str());
							} // if
						break;
						} // text
					default:
						{
						if(WThis->DBEHost->IsColumnALayer(HitTest.iSubItem))
							{
							NotifyTag Changes[2];
							LayerStub *Matched;
							LayerEntry *ColumnLayer = WThis->DBEHost->GetLayerColumn(WThis->DBEHost->GetLayerNumFromColumn(HitTest.iSubItem));
							if(Matched = JoeItem->MatchLayer(ColumnLayer))
								{
								JoeItem->RemoveObjectFromLayer(0, NULL, Matched); // does the one we clicked, passing LayerStub is faster
								WThis->DBEHost->DoRemoveLayer(ColumnLayer); // does the rest of the selection set if needed
								} // if
							else
								{
								JoeItem->AddObjectToLayer(ColumnLayer); // does the one we clicked
								WThis->DBEHost->DoAddLayer(ColumnLayer); // does the rest of the selection set if needed
								} // else
							Changes[0] = MAKE_ID(ColumnLayer->GetNotifyClass(), ColumnLayer->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, ColumnLayer->GetRAHostRoot());
							WThis->DBEHost->BuildLayerList(WThis->DBHost->ActiveObj, ListView_GetItemCount(hwnd));
							} // if
						else if(WThis->DBEHost->IsColumnAnAttribute(HitTest.iSubItem))
							{
							// editable text (attributes)
							std::string TempString = WThis->FetchTextForColumn(HitItem, HitTest.iSubItem);
							WThis->CreateEdit(HitItem, HitTest.iSubItem, false, TempString.c_str());
							} // else if
						break;
						} // text
					} // iSubItem
				} // if
			} // if
		return(0); // so we don't pass along
		} // WM_LBUTTONDOWN
	} // switch

return(CallWindowProc(ListViewWndProc, hwnd, message, wParam, (LPARAM)lParam));

} // DBGridWidgetWndProc

