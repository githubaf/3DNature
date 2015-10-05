// EffectsLibGUI.cpp
// Code for Effects Library Editor
// Built from scratch on 5/1/97 by Gary Huber
// Copyright 1997 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLibGUI.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Database.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Conservatory.h"
#include "DBEditGUI.h"
#include "Joe.h"
#include "Raster.h"
#include "DigitizeGUI.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"
#include "FeatureConfig.h"

NativeGUIWin EffectsLibGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // EffectsLibGUI::Open

/*===========================================================================*/

NativeGUIWin EffectsLibGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_EFFECTS_LIB1, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if
	} // if
 
return(NativeWin);

} // EffectsLibGUI::Construct

/*===========================================================================*/

EffectsLibGUI::EffectsLibGUI(EffectsLib *LibSource, Project *ProjSource, Database *DBSource)
: GUIFenetre('EFFL', this, "Component Library") // Yes, I know...
{
static NotifyTag AllEffectEvents[] = {
									MAKE_ID(WCS_NOTIFYCLASS_FREEZE, 0xff, 0xff, 0xff), 
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
									0};
static NotifyTag AllDBEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, 0xff, 0xff, 0xff), 
								0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
HostLib = LibSource;
HostProj = ProjSource;
HostDB = DBSource;
BrowseLib = new EffectsLib;
BrowseImageLib = new ImageLib;
Frozen = 0;

SECURITY_INLINE_CHECK(019, 19);

HostDB->RegisterClient(this, AllDBEvents);
GlobalApp->AppEx->RegisterClient(this, AllEffectEvents);

//SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // EffectsLibGUI::EffectsLibGUI

/*===========================================================================*/

EffectsLibGUI::~EffectsLibGUI()
{

delete BrowseImageLib;	// the order of deletion is critical
delete BrowseLib;

HostDB->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // EffectsLibGUI::~EffectsLibGUI()

/*===========================================================================*/

long EffectsLibGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{
	switch(CtrlID)
		{
		case IDC_PARLIST:
			{
			RemoveEffect();
			break;
			} // IDC_PARLIST
		default:
			break;
		} // switch

return(0);

} // EffectsLibGUI::HandleListDelItem

/*===========================================================================*/

long EffectsLibGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_EFG, 0);

return(0);

} // EffectsLibGUI::HandleCloseWin

/*===========================================================================*/

long EffectsLibGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
SECURITY_INLINE_CHECK(024, 24);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EFG, 0);
		break;
		} // 
	case IDC_CREATENEW:
		{
		SECURITY_INLINE_CHECK(029, 29);
		NewEffect();
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
	case IDC_APPLYTOACTIVE:
		{
		SECURITY_INLINE_CHECK(049, 49);
		ApplyToActive(NULL);
		break;
		} // 
	case IDC_ADDFILE:
		{
		BrowseFile();
		break;
		} // 
	case IDC_SELECTALL:
		{
		SelectAll();
		break;
		} // 
	case IDC_COPYTOPROJ:
		{
		CopyToProj();
		break;
		} // 
	case IDC_SORTEFFECTS:
		{
		SortEffects();
		break;
		} // 
	case IDC_OPENDBEDIT:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_DBG, 0);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // EffectsLibGUI::HandleButtonClick

/*===========================================================================*/

long EffectsLibGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		ModifyEffect();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // EffectsLibGUI::HandleListDoubleClick

/*===========================================================================*/

void EffectsLibGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
//short ListEntry;

if (! NativeWin)
	return;
SECURITY_INLINE_CHECK(024, 24);

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
	UnFreeze();		// rebuilds all
	return;
	} // if thaw
if (Frozen)
	return;

if (((Changes[0] & 0xff000000) >> 24) == 0xff || (((Changes[0] & 0xff000000) >> 24) >= WCS_EFFECTSSUBCLASS_LAKE && 
	((Changes[0] & 0xff000000) >> 24) < WCS_MAXIMPLEMENTED_EFFECTS))
	{
	Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[1] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_LOAD, 0xff, 0xff);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		BuildList(HostLib, IDC_PARLIST);
		} // if effects added changed

	Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
		{
		ResetListJoes();
		} // if joes added changed

	SECURITY_INLINE_CHECK(018, 18);

	Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
		{
		ResetListNames();
		} // if name changed

	Interested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
		{
		ResetListEnabled();
		} // if enabled status changed
	} // if an effect notify

} // EffectsLibGUI::HandleNotifyEvent()

/*===========================================================================*/

void EffectsLibGUI::ConfigureWidgets(void)
{

ConfigureTB(NativeWin, IDC_CREATENEW, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEEFFECT, IDI_DELETE, NULL);

BuildList(HostLib, IDC_PARLIST);

} // EffectsLibGUI::ConfigureWidgets()

/*===========================================================================*/

void EffectsLibGUI::BuildList(EffectsLib *SourceLib, WIDGETID ListView)
{
long Place;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 10];
GeneralEffect *Me;

WidgetLBClear(ListView);

Me = (GeneralEffect *)SourceLib->Material;
WidgetLBInsert(ListView, -1, "* 3D Materials");
while (Me)

	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Object3D;
WidgetLBInsert(ListView, -1, "* 3D Objects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Atmospheres;
WidgetLBInsert(ListView, -1, "* Atmospheres");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->RasterTA;
WidgetLBInsert(ListView, -1, "* Area Terraffectors");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Cameras;
WidgetLBInsert(ListView, -1, "* Cameras");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Celestial;
WidgetLBInsert(ListView, -1, "* Celestial Objects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Cloud;
WidgetLBInsert(ListView, -1, "* Cloud Models");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Cmap;
WidgetLBInsert(ListView, -1, "* Color Maps");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#ifdef WCS_COORD_SYSTEM
Me = (GeneralEffect *)SourceLib->Coord;
WidgetLBInsert(ListView, -1, "* Coordinate Systems");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#endif // WCS_COORD_SYSTEM
#ifdef WCS_DEM_MERGE
Me = (GeneralEffect *)SourceLib->Merger;
WidgetLBInsert(ListView, -1, "* DEM Mergers");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#endif // WCS_DEM_MERGE
Me = (GeneralEffect *)SourceLib->Ecosystem;
WidgetLBInsert(ListView, -1, "* Ecosystems");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Environment;
WidgetLBInsert(ListView, -1, "* Environments");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Foliage;
WidgetLBInsert(ListView, -1, "* Foliage Effects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Ground;
WidgetLBInsert(ListView, -1, "* Ground Effects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
/*
Me = (GeneralEffect *)SourceLib->Illumination;
WidgetLBInsert(ListView, -1, "* Illumination Effects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
*/
Me = (GeneralEffect *)SourceLib->Labels;
WidgetLBInsert(ListView, -1, "* Labels");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Lake;
WidgetLBInsert(ListView, -1, "* Lakes");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Lights;
WidgetLBInsert(ListView, -1, "* Lights");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->PlanetOpts;
WidgetLBInsert(ListView, -1, "* Planet Options");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->PostProc;
WidgetLBInsert(ListView, -1, "* Post Processes");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->RenderJobs;
WidgetLBInsert(ListView, -1, "* Render Jobs");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->RenderOpts;
WidgetLBInsert(ListView, -1, "* Render Options");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#ifdef WCS_RENDER_SCENARIOS
Me = (GeneralEffect *)SourceLib->Scenario;
WidgetLBInsert(ListView, -1, "* Render Scenarios");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS
#ifdef WCS_BUILD_RTX
if (GlobalApp->SXAuthorized)
	{
	Me = (GeneralEffect *)SourceLib->Exporter;
	WidgetLBInsert(ListView, -1, "* Scene Exporters");
	while (Me)
		{
		BuildListEntry(ListName, Me);
		Place = WidgetLBInsert(ListView, -1, ListName);
		WidgetLBSetItemData(ListView, Place, Me);
		Me = Me->Next;
		} // while
	} // if
#endif // WCS_BUILD_RTX
#ifdef WCS_SEARCH_QUERY
Me = (GeneralEffect *)SourceLib->Search;
WidgetLBInsert(ListView, -1, "* Search Queries");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#endif // WCS_SEARCH_QUERY
Me = (GeneralEffect *)SourceLib->Shadow;
WidgetLBInsert(ListView, -1, "* Shadows");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Skies;
WidgetLBInsert(ListView, -1, "* Skies");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Snow;
WidgetLBInsert(ListView, -1, "* Snow Effects");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->StarField;
WidgetLBInsert(ListView, -1, "* Starfields");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Stream;
WidgetLBInsert(ListView, -1, "* Streams");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Terraffector;
WidgetLBInsert(ListView, -1, "* Terraffectors");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->TerrainParam;
WidgetLBInsert(ListView, -1, "* Terrain Parameters");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Generator;
WidgetLBInsert(ListView, -1, "* Terrain Generators");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Gridder;
WidgetLBInsert(ListView, -1, "* Terrain Gridders");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#ifdef WCS_THEMATIC_MAP
Me = (GeneralEffect *)SourceLib->Theme;
WidgetLBInsert(ListView, -1, "* Thematic Maps");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
#endif // WCS_THEMATIC_MAP
Me = (GeneralEffect *)SourceLib->Fences;
WidgetLBInsert(ListView, -1, "* Walls");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while
Me = (GeneralEffect *)SourceLib->Wave;
WidgetLBInsert(ListView, -1, "* Waves");
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	Me = Me->Next;
	} // while

// <<<>>> ADD_NEW_EFFECTS add a block somewhere appropriate above to make the effect show up in the Component Library

} // EffectsLibGUI::BuildList()

/*===========================================================================*/

void EffectsLibGUI::BuildListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, "    ");
strcat(ListName, Me->Name);

} // EffectsLibGUI::BuildListEntry()

/*===========================================================================*/

void EffectsLibGUI::NewEffect(void)
{
char Category[WCS_EFFECT_MAXNAMELENGTH + 10];
long Selected, EffectType;
GeneralEffect *Test, *TheNewEffect;

	if ((Selected = WidgetLBGetCurSel(IDC_PARLIST)) != -1)
		{
		EffectType = GetEffectClassFromListEntry(Selected, Test, Category);

		if (HostLib->EffectTypeImplemented(EffectType))
			{
			if (Test)
				strcpy(Category, Test->GetName());
			if (TheNewEffect = HostLib->AddEffect(EffectType, Test ? Category: NULL, Test))
				{
				TheNewEffect->Edit();
				} // if
			} // if

		} // if an item selected
	else
		{
		UserMessageOK("Effects Library: New", "You must first select an Effect or Category in the list to replicate!");
		} // else no selection

} // EffectsLibGUI::NewEffect()

/*===========================================================================*/

void EffectsLibGUI::ModifyEffect(void)
{
long Selected;
GeneralEffect *Test;

	Selected = WidgetLBGetCurSel(IDC_PARLIST);

	if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
		{
		Test->Edit();
		} // if its an existing effect
	else if (! Test)
		{
		NewEffect();
		} // else

} // EffectsLibGUI::ModifyEffect()

/*===========================================================================*/

void EffectsLibGUI::RemoveEffect(void)
{
char NameStr[256];
long NumItems, Item, FoundIt = 0, RemoveAll = 0, *SelItems;
GeneralEffect *Test;
NotifyTag Changes[2];

Changes[1] = NULL;

// get number of items in list
if ((NumItems = WidgetLBGetSelCount(IDC_PARLIST)) != -1 && NumItems)
	{
	if (SelItems = (long *)AppMem_Alloc(NumItems * sizeof (long), APPMEM_CLEAR))
		{
		// get a list of selected items
		if (WidgetLBGetSelItems(IDC_PARLIST, NumItems, SelItems))
			{
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			for (Item = 0; Item < NumItems; Item ++)
				{
				if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, SelItems[Item])) != (GeneralEffect *)-1 && Test)
					{
					if (HostLib->IsEffectValid(Test, 0))
						{
						sprintf(NameStr, "%s %s", Test->Name, Test->GetRAHostTypeString());
						if (! FoundIt && (RemoveAll == 2 || (
							(NumItems == 1 && UserMessageOKCAN(NameStr, "OK to remove this Component from the library?")) ||
							(NumItems > 1 && (RemoveAll = UserMessageCustom(NameStr, "OK to remove this Component from the library?",

							"OK", "Skip", "Remove All"))))))
							{
							HostLib->RemoveRAHost(Test, RemoveAll == 2);
							} // else if
						} // if its a valid effect
					} //  if its an effect
				} // for each item selected
			AppMem_Free(SelItems, NumItems * sizeof (long));
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if items retrieved
		} // if memory for list allocated
	} // if items selected
else
	{
	UserMessageOK("Components Library: Remove", "There are no items selected to remove!");
	} // else

} // EffectsLibGUI::RemoveEffect()

/*===========================================================================*/

long EffectsLibGUI::GetEffectClassFromListEntry(long Selected, GeneralEffect *&Test, char *Category)
{
long EffectType = 0;

if (Selected >= 0)
	{
	if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
		{
		EffectType = Test->EffectType;
		} // if
	else
		{
		WidgetLBGetText(IDC_PARLIST, Selected, Category);
		if (! strcmp(Category, "* Lakes"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_LAKE;
		else if (! strcmp(Category, "* Area Terraffectors"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR;
		else if (! strcmp(Category, "* Ecosystems"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM;
		else if (! strcmp(Category, "* Illumination Effects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION;
		else if (! strcmp(Category, "* Terraffectors"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR;
		else if (! strcmp(Category, "* Shadows"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_SHADOW;
		else if (! strcmp(Category, "* Foliage Effects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_FOLIAGE;
		else if (! strcmp(Category, "* Streams"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_STREAM;
		else if (! strcmp(Category, "* 3D Objects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_OBJECT3D;
		else if (! strcmp(Category, "* 3D Materials"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_MATERIAL;
		else if (! strcmp(Category, "* Background Images"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_BACKGROUND;
		else if (! strcmp(Category, "* Celestial Objects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_CELESTIAL;
		else if (! strcmp(Category, "* Starfields"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_STARFIELD;
		else if (! strcmp(Category, "* Color Maps"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_CMAP;
		else if (! strcmp(Category, "* Planet Options"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_PLANETOPT;
		else if (! strcmp(Category, "* Terrain Parameters"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM;
		else if (! strcmp(Category, "* Ground Effects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_GROUND;
		else if (! strcmp(Category, "* Snow Effects"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_SNOW;
		else if (! strcmp(Category, "* Skies"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_SKY;
		else if (! strcmp(Category, "* Atmospheres"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE;
		else if (! strcmp(Category, "* Lights"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_LIGHT;
		else if (! strcmp(Category, "* Cameras"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_CAMERA;
		else if (! strcmp(Category, "* Render Jobs"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDERJOB;
		else if (! strcmp(Category, "* Render Options"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDEROPT;
		else if (! strcmp(Category, "* Environments"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT;
		else if (! strcmp(Category, "* Cloud Models"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_CLOUD;
		else if (! strcmp(Category, "* Waves"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_WAVE;
		else if (! strcmp(Category, "* Terrain Gridders"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_GRIDDER;
		else if (! strcmp(Category, "* Terrain Generators"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_GENERATOR;
		else if (! strcmp(Category, "* Search Queries"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY;
		else if (! strcmp(Category, "* Thematic Maps"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP;
		else if (! strcmp(Category, "* Coordinate Systems"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_COORDSYS;
		else if (! strcmp(Category, "* Walls"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_FENCE;
		else if (! strcmp(Category, "* Post Processes"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_POSTPROC;
		else if (! strcmp(Category, "* Render Scenarios"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_SCENARIO;
		else if (! strcmp(Category, "* DEM Mergers"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_DEMMERGER;
		else if (! strcmp(Category, "* Scene Exporters"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_EXPORTER;
		else if (! strcmp(Category, "* Labels"))
			EffectType = WCS_JOE_ATTRIB_INTERNAL_LABEL;
// <<<>>> ADD_NEW_EFFECTS the asterisk makes the text bold
		} // else
	} // if

return (EffectType);

} // EffectsLibGUI::GetEffectClassFromListEntry

/*===========================================================================*/

void EffectsLibGUI::SortEffects(void)
{
long Selected, EffectType;
GeneralEffect *Test;
char Category[WCS_EFFECT_MAXNAMELENGTH + 10];

if ((Selected = WidgetLBGetCurSel(IDC_PARLIST)) != -1)
	{
	if ((EffectType = GetEffectClassFromListEntry(Selected, Test, Category)) >= 0)
		{
		if (HostLib->EffectTypeImplemented(EffectType))
			HostLib->SortEffectsByAlphabet(EffectType);
		} // if
	} // if

} // EffectsLibGUI::SortEffects

/*===========================================================================*/

void EffectsLibGUI::CloseDangerousWindows(long EffectClass)
{
// <<<>>> Nothing to do now that we've removed all that WCS_REMOVE_V5 stuff
} // EffectsLibGUI::CloseDangerousWindows

/*===========================================================================*/

long EffectsLibGUI::FindInList(GeneralEffect *FindMe, WIDGETID ListID)
{
long NumItems, Ct;
GeneralEffect *Test;

	NumItems = WidgetLBGetCount(IDC_PARLIST);
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Ct)) != (GeneralEffect *)-1 && Test)
			{
			if (Test == FindMe)
				return (Ct);
			} // if
		} // for

return (-1);

} // EffectsLibGUI::FindInList

/*===========================================================================*/

void EffectsLibGUI::ResetListNames(void)
{
char Category[WCS_EFFECT_MAXNAMELENGTH + 10];
long Selected, Found = 0;
GeneralEffect *Test;

	Selected = WidgetLBGetCurSel(IDC_PARLIST);

	if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
		{
		WidgetLBGetText(IDC_PARLIST, Selected, Category);
		if (strcmp(Test->Name, &Category[8]))
			{
			BuildListEntry(Category, Test);
			WidgetLBDelete(IDC_PARLIST, Selected);
			WidgetLBInsert(IDC_PARLIST, Selected, Category);
			WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
			WidgetLBSetCurSel(IDC_PARLIST, Selected);
			Found = 1;
			} // if its the active list item
		} // if it could bethe active list item
	if (! Found)
		{
		if ((Found = WidgetLBGetCount(IDC_PARLIST)) != -1)
			{
			for (Selected = 0; Selected < Found; Selected ++)
				{
				if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
					{
					WidgetLBGetText(IDC_PARLIST, Selected, Category);
					if (strcmp(Test->Name, &Category[8]))
						{
						BuildListEntry(Category, Test);
						WidgetLBDelete(IDC_PARLIST, Selected);
						WidgetLBInsert(IDC_PARLIST, Selected, Category);
						WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
						WidgetLBSetCurSel(IDC_PARLIST, Selected);
						} // if its the active list item
					} // if it could be the active list item
				} // for
			} // if some items in the list
		} // if

} // EffectsLibGUI::ResetListNames

/*===========================================================================*/

void EffectsLibGUI::ResetListEnabled(void)
{
char Category[WCS_EFFECT_MAXNAMELENGTH + 10];
long Selected, Found = 0;
GeneralEffect *Test;

	Selected = WidgetLBGetCurSel(IDC_PARLIST);

	if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
		{
		WidgetLBGetText(IDC_PARLIST, Selected, Category);
		if (Test->Enabled && Category[0] == ' ' || ! Test->Enabled && Category[0] == '*')
			{
			BuildListEntry(Category, Test);
			WidgetLBDelete(IDC_PARLIST, Selected);
			WidgetLBInsert(IDC_PARLIST, Selected, Category);
			WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
			WidgetLBSetCurSel(IDC_PARLIST, Selected);
			Found = 1;
			} // if its the active list item
		} // if it could bethe active list item
	if (! Found)
		{
		if ((Found = WidgetLBGetCount(IDC_PARLIST)) != -1)
			{
			for (Selected = 0; Selected < Found; Selected ++)
				{
				if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
					{
					WidgetLBGetText(IDC_PARLIST, Selected, Category);
					if (Test->Enabled && Category[0] == ' ' || ! Test->Enabled && Category[0] == '*')
						{
						BuildListEntry(Category, Test);
						WidgetLBDelete(IDC_PARLIST, Selected);
						WidgetLBInsert(IDC_PARLIST, Selected, Category);
						WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
						WidgetLBSetCurSel(IDC_PARLIST, Selected);
						} // if its the active list item
					} // if it could be the active list item
				} // for
			} // if some items in the list
		} // if

} // EffectsLibGUI::ResetListEnabled

/*===========================================================================*/

void EffectsLibGUI::ResetListJoes(void)
{
char Category[WCS_EFFECT_MAXNAMELENGTH + 10];
long Selected, Found = 0;
GeneralEffect *Test;

	Selected = WidgetLBGetCurSel(IDC_PARLIST);

	if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
		{
		WidgetLBGetText(IDC_PARLIST, Selected, Category);
		if (Test->Joes && Category[2] == ' ' || ! Test->Joes && Category[2] == '+')
			{
			BuildListEntry(Category, Test);
			WidgetLBDelete(IDC_PARLIST, Selected);
			WidgetLBInsert(IDC_PARLIST, Selected, Category);
			WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
			WidgetLBSetCurSel(IDC_PARLIST, Selected);
			Found = 1;
			} // if its the active list item
		} // if it could bethe active list item
	if (! Found)
		{
		if ((Found = WidgetLBGetCount(IDC_PARLIST)) != -1)
			{
			for (Selected = 0; Selected < Found; Selected ++)
				{
				if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (GeneralEffect *)-1 && Test)
					{
					WidgetLBGetText(IDC_PARLIST, Selected, Category);
					if (Test->Joes && Category[2] == ' ' || ! Test->Joes && Category[2] == '+')
						{
						BuildListEntry(Category, Test);
						WidgetLBDelete(IDC_PARLIST, Selected);
						WidgetLBInsert(IDC_PARLIST, Selected, Category);
						WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
						WidgetLBSetCurSel(IDC_PARLIST, Selected);
						} // if its the active list item
					} // if it could be the active list item
				} // for
			} // if some items in the list
		} // if

} // EffectsLibGUI::ResetListJoes

/*===========================================================================*/

long EffectsLibGUI::ApplyToActive(Joe *NewVec)
{
long Current, Ct, NumItems, EffectsAdded = 0;
GeneralEffect *Test;

if (GlobalApp->GUIWins->DBE || NewVec)
	{
		{ // used to be an if
		Current = WidgetLBGetCurSel(IDC_PARLIST);
		NumItems = WidgetLBGetCount(IDC_PARLIST);

		if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Current)) != (GeneralEffect *)-1 && Test)
			{
			if (NewVec)
				EffectsAdded += NewVec->AddEffect(Test, -1);
			else
				GlobalApp->GUIWins->DBE->ApplyEffectToSelected(Test);
			} // if
		for (Ct = 0; Ct < NumItems; Ct ++)
			{
			if (Ct != Current)
				{
				if (WidgetLBGetSelState(IDC_PARLIST, Ct))
					{
					if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_PARLIST, Ct)) != (GeneralEffect *)-1 && Test)
						{
						if (NewVec)
							EffectsAdded += NewVec->AddEffect(Test, -1);
						else
							GlobalApp->GUIWins->DBE->ApplyEffectToSelected(Test);
						} // if
					} // if
				} // if
			} // for
		} // if
	} // if

return (EffectsAdded);

} // EffectsLibGUI::ApplyToActive

/*===========================================================================*/

void EffectsLibGUI::BrowseFile(void)
{

if (BrowseLib)
	{
	HostProj->Load(NULL, NULL, HostDB, BrowseLib, BrowseImageLib,
		HostProj->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Images",
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Effects",
		WCS_PROJECT_IODETAILTAG_FLAGS, WCS_EFFECTS_LOAD_CLEAR,
		WCS_PROJECT_IODETAILTAG_DONE),
		0x0110ffff);
	BuildList(BrowseLib, IDC_EFFECTLIST);
	} // if

} // EffectsLibGUI::BrowseFile()

/*===========================================================================*/

void EffectsLibGUI::SelectAll(void)
{
long Ct, NumItems;
GeneralEffect *Test;

if (BrowseLib)
	{
		NumItems = WidgetLBGetCount(IDC_EFFECTLIST);
		for (Ct = 0; Ct < NumItems; Ct ++)
			{
			if((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_EFFECTLIST, Ct)) != (GeneralEffect *)-1 && Test)
				{
				WidgetLBSetSelState(IDC_EFFECTLIST, TRUE, Ct);
				} // if
			} // for
	} // if

} // EffectsLibGUI::SelectAll

/*===========================================================================*/

void EffectsLibGUI::CopyToProj(void)
{
long Ct, Selected, EffectType, NumItems;
NotifyTag Changes[2];
GeneralEffect *Test;

Changes[1] = NULL;

if (BrowseLib)
	{
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	NumItems = WidgetLBGetCount(IDC_EFFECTLIST);

	GlobalApp->CopyFromImageLib = BrowseImageLib;
	GlobalApp->CopyFromEffectsLib = BrowseLib;
	// copy effects into the effects lib
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if ((Selected = WidgetLBGetSelState(IDC_EFFECTLIST, Ct)) != -1 && Selected)
			{
			if ((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_EFFECTLIST, Ct)) != (GeneralEffect *)-1 && Test)
				{
				EffectType = Test->EffectType;
				if (HostLib->EffectTypeImplemented(EffectType))
					{
					//lint -save -e522
					switch (EffectType)
						{
						case WCS_JOE_ATTRIB_INTERNAL_LAKE:
							{
							new LakeEffect(NULL, HostLib, (LakeEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_LAKE
						case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
							{
							new EcosystemEffect(NULL, HostLib, (EcosystemEffect *)Test, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
						case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
							{
							new RasterTerraffectorEffect(NULL, HostLib, (RasterTerraffectorEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
						case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
							{
							new TerraffectorEffect(NULL, HostLib, (TerraffectorEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
						case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
							{
							new ShadowEffect(NULL, HostLib, (ShadowEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
						case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
							{
							new FoliageEffect(NULL, HostLib, (FoliageEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
						case WCS_JOE_ATTRIB_INTERNAL_STREAM:
							{
							new StreamEffect(NULL, HostLib, (StreamEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_STREAM
						case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
							{
							new Object3DEffect(NULL, HostLib, (Object3DEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
						case WCS_JOE_ATTRIB_INTERNAL_MATERIAL:
							{
							new MaterialEffect(NULL, HostLib, (MaterialEffect *)Test, WCS_EFFECTS_MATERIALTYPE_OBJECT3D);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_MATERIAL
						case WCS_JOE_ATTRIB_INTERNAL_CELESTIAL:
							{
							new CelestialEffect(NULL, HostLib, (CelestialEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_CELESTIAL
						case WCS_JOE_ATTRIB_INTERNAL_STARFIELD:
							{
							new StarFieldEffect(NULL, HostLib, (StarFieldEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_STARFIELD
						case WCS_JOE_ATTRIB_INTERNAL_CMAP:
							{
							new CmapEffect(NULL, HostLib, (CmapEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_CMAP
						case WCS_JOE_ATTRIB_INTERNAL_PLANETOPT:
							{
							new PlanetOpt(NULL, HostLib, (PlanetOpt *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_PLANETOPT
						case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
							{
							new TerrainParamEffect(NULL, HostLib, (TerrainParamEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
						case WCS_JOE_ATTRIB_INTERNAL_GROUND:
							{
							new GroundEffect(NULL, HostLib, (GroundEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_GROUND
						case WCS_JOE_ATTRIB_INTERNAL_SNOW:
							{
							new SnowEffect(NULL, HostLib, (SnowEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_SNOW
						case WCS_JOE_ATTRIB_INTERNAL_SKY:
							{
							new Sky(NULL, HostLib, (Sky *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_SKY
						case WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE:
							{
							new Atmosphere(NULL, HostLib, (Atmosphere *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE
						case WCS_JOE_ATTRIB_INTERNAL_LIGHT:
							{
							new Light(NULL, HostLib, (Light *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_LIGHT
						case WCS_JOE_ATTRIB_INTERNAL_CAMERA:
							{
							new Camera(NULL, HostLib, (Camera *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_CAMERA
						case WCS_JOE_ATTRIB_INTERNAL_RENDERJOB:
							{
							new RenderJob(NULL, HostLib, (RenderJob *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_RENDERJOB
						case WCS_JOE_ATTRIB_INTERNAL_RENDEROPT:
							{
							new RenderOpt(NULL, HostLib, (RenderOpt *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_RENDEROPT
						case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
							{
							new EnvironmentEffect(NULL, HostLib, (EnvironmentEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
						case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
							{
							new CloudEffect(NULL, HostLib, (CloudEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
						case WCS_JOE_ATTRIB_INTERNAL_WAVE:
							{
							new WaveEffect(NULL, HostLib, (WaveEffect *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_WAVE
						case WCS_JOE_ATTRIB_INTERNAL_GRIDDER:
							{
							new TerraGridder(NULL, HostLib, (TerraGridder *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_GRIDDER
						case WCS_JOE_ATTRIB_INTERNAL_GENERATOR:
							{
							new TerraGenerator(NULL, HostLib, (TerraGenerator *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_GRIDDER
						case WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY:
							{
							new SearchQuery(NULL, HostLib, (SearchQuery *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY
						case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
							{
							new ThematicMap(NULL, HostLib, (ThematicMap *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
						case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
							{
							new CoordSys(NULL, HostLib, (CoordSys *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
						case WCS_JOE_ATTRIB_INTERNAL_FENCE:
							{
							new Fence(NULL, HostLib, (Fence *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_FENCE
						case WCS_JOE_ATTRIB_INTERNAL_POSTPROC:
							{
							new PostProcess(NULL, HostLib, (PostProcess *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_POSTPROC
						case WCS_JOE_ATTRIB_INTERNAL_SCENARIO:
							{
							new RenderScenario(NULL, HostLib, (RenderScenario *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_SCENARIO
						case WCS_JOE_ATTRIB_INTERNAL_DEMMERGER:
							{
							new DEMMerger(NULL, HostLib, (DEMMerger *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_DEMMERGER
						case WCS_JOE_ATTRIB_INTERNAL_EXPORTER:
							{
							new SceneExporter(NULL, HostLib, (SceneExporter *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_EXPORTER
						case WCS_JOE_ATTRIB_INTERNAL_LABEL:
							{
							new Label(NULL, HostLib, (Label *)Test);
							break;
							} // WCS_JOE_ATTRIB_INTERNAL_LABEL
			// <<<>>> ADD_NEW_EFFECTS add a block for every effect type
						default:
							break;
						} // switch
					//lint -restore
					} // if
				} // if its really an effect
			} // if an item selected
		} // for
	GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
	GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

} // EffectsLibGUI::CopyToProj

/*===========================================================================*/
