// SearchQueryEditGUI.cpp
// Code for Search Query editor
// Built from scratch on 12/22/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "SearchQueryEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "GraphData.h"
#include "DBFilterEvent.h"
#include "Interactive.h"
#include "Conservatory.h"
#include "DBEditGUI.h"
#include "AppMem.h"
#include "resource.h"

char *SearchQueryEditGUI::TabNames[WCS_SEARCHQUERYGUI_NUMTABS] = {"General", "Filter Criteria", "Geographic Criteria"};

long SearchQueryEditGUI::ActivePage;
// advanced
long SearchQueryEditGUI::DisplayAdvanced;

NativeGUIWin SearchQueryEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // SearchQueryEditGUI::Open

/*===========================================================================*/

NativeGUIWin SearchQueryEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_SEARCHQUERY_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_SEARCHQUERY_FILTER, 0, 1);
	CreateSubWinFromTemplate(IDD_SEARCHQUERY_GEOBNDS, 0, 2);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_SEARCHQUERYGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		ShowPanel(0, ActivePage);
		FillAttributeCBs();
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // SearchQueryEditGUI::Construct

/*===========================================================================*/

SearchQueryEditGUI::SearchQueryEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, SearchQuery *ActiveSource)
: GUIFenetre('SEQU', this, "Search Query"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								MAKE_ID(WCS_SUBCLASS_LAYER, WCS_SUBCLASS_LAYER, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, WCS_RAHOST_OBJTYPE_DEM, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, WCS_RAHOST_OBJTYPE_VECTOR, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
DBHost = DBSource;
ProjHost = ProjSource;
EffectsHost = EffectsSource;
Active = ActiveSource;
ActiveFilter = NULL;
AttrExists = AttrEquals = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
AttrConfigured = 0;
ReceivingDiagnostics = ReceivingPoint = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;

if (DBSource && ActiveSource)
	{
	sprintf(NameStr, "Search Query - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);

	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // SearchQueryEditGUI::SearchQueryEditGUI

/*===========================================================================*/

SearchQueryEditGUI::~SearchQueryEditGUI()
{
SetTitle("Closing window, please wait...");
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // SearchQueryEditGUI::~SearchQueryEditGUI()

/*===========================================================================*/

long SearchQueryEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_SQU, 0);

return(0);

} // SearchQueryEditGUI::HandleCloseWin

/*===========================================================================*/

long SearchQueryEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // SearchQueryEditGUI::HandleShowAdvanced

/*===========================================================================*/

long SearchQueryEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDFILTER:
		{
		AddFilter();
		break;
		} // IDC_ADDFILTER
	case IDC_SELECTNOW:
		{
		SelectDBItems();
		break;
		} // IDC_ADDFILTER
	case IDC_REMOVEFILTER:
		{
		RemoveFilter();
		break;
		} // IDC_REMOVEFILTER
	case IDC_MOVEFILTERUP:
		{
		ChangeFilterListPosition(1);
		break;
		} // IDC_MOVEFILTERUP
	case IDC_MOVEFILTERDOWN:
		{
		ChangeFilterListPosition(0);
		break;
		} // IDC_MOVEFILTERDOWN
	case IDC_SETBOUNDS:
		{
		SetBounds(NULL);
		break;
		} // 
	case IDC_SNAPBOUNDS:
		{
		ReceivingDiagnostics = ReceivingPoint = 0;
		if (ActiveFilter)
			{
			ActiveFilter->GeoBnds.SnapToDBObjs(NULL);
			ConfigureWidgets();
			} // if
		break;
		} // 
	case IDC_SETPT:
		{
		SetPoint(NULL);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // SearchQueryEditGUI::HandleButtonClick

/*===========================================================================*/

long SearchQueryEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_FILTERLIST:
		{
		RemoveFilter();
		break;
		} // IDC_FILTERLIST
	default:
		break;
	} // switch

return(0);

} // SearchQueryEditGUI::HandleListDelItem

/*===========================================================================*/

long SearchQueryEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(018, 18);
switch (CtrlID)
	{
	case IDC_FILTERLIST:
		{
		SetActiveFilter();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // SearchQueryEditGUI::HandleListSel

/*===========================================================================*/

long SearchQueryEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_FILTERLIST:
		{
		WidgetTCSetCurSel(IDC_TAB1, 1);
		ShowPanel(0, 1);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // SearchQueryEditGUI::HandleListDoubleClick

/*===========================================================================*/

long SearchQueryEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_NAMEMATCH:
		{
		NewMatchName();
		break;
		} // 
	case IDC_LABELMATCH:
		{
		NewMatchLabel();
		break;
		} // 
	case IDC_ATTRVALUE:
		{
		NewAttributeValue();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // SearchQueryEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long SearchQueryEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ATTRIBDROP:
		{
		NewMatchAttribute();
		break;
		}
	case IDC_LAYERDROP:
		{
		NewMatchLayer();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // SearchQueryEditGUI::HandleCBChange

/*===========================================================================*/

long SearchQueryEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				ShowPanel(0, 1);
				break;
				} // 1
			case 2:
				{
				ShowPanel(0, 2);
				break;
				} // 2
			default:
				{
				ShowPanel(0, 0);
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

} // SearchQueryEditGUI::HandlePageChange

/*===========================================================================*/

long SearchQueryEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

if (ActiveFilter)
	{
	switch (CtrlID)
		{
		case IDC_CHECKLAYEREQUALS:
			{
			if (ActiveFilter->LayerEquals)
				ActiveFilter->LayerSimilar = ActiveFilter->LayerNumeric = 0;
			break;
			} // 
		case IDC_CHECKLAYERSIMILAR:
			{
			if (ActiveFilter->LayerSimilar)
				ActiveFilter->LayerEquals = ActiveFilter->LayerNumeric = 0;
			break;
			} // 
		case IDC_CHECKLAYERNUMERIC:
			{
			if (ActiveFilter->LayerNumeric)
				ActiveFilter->LayerEquals = ActiveFilter->LayerSimilar = 0;
			break;
			} // 
		case IDC_CHECKNAMEEQUALS:
			{
			if (ActiveFilter->NameEquals)
				ActiveFilter->NameSimilar = ActiveFilter->NameNumeric = 0;
			break;
			} // 
		case IDC_CHECKNAMESIMILAR:
			{
			if (ActiveFilter->NameSimilar)
				ActiveFilter->NameEquals = ActiveFilter->NameNumeric = 0;
			break;
			} // 
		case IDC_CHECKNAMENUMERIC:
			{
			if (ActiveFilter->NameNumeric)
				ActiveFilter->NameEquals = ActiveFilter->NameSimilar = 0;
			break;
			} // 
		case IDC_CHECKLABELEQUALS:
			{
			if (ActiveFilter->LabelEquals)
				ActiveFilter->LabelSimilar = ActiveFilter->LabelNumeric = 0;
			break;
			} // 
		case IDC_CHECKLABELSIMILAR:
			{
			if (ActiveFilter->LabelSimilar)
				ActiveFilter->LabelEquals = ActiveFilter->LabelNumeric = 0;
			break;
			} // 
		case IDC_CHECKLABELNUMERIC:
			{
			if (ActiveFilter->LabelNumeric)
				ActiveFilter->LabelEquals = ActiveFilter->LabelSimilar = 0;
			break;
			} // 
		case IDC_CHECKATTREXISTS:
			{
			if (AttrExists)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EXISTS;
				AttrEquals = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTREQUALS:
			{
			if (AttrEquals)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
				AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTRGREATER:
			{
			if (AttrGreater)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_GREATER;
				AttrExists = AttrEquals = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTRGREATEREQUALS:
			{
			if (AttrGreaterEquals)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS;
				AttrExists = AttrGreater = AttrEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTRLESS:
			{
			if (AttrLess)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_LESS;
				AttrExists = AttrGreater = AttrGreaterEquals = AttrEquals = AttrLessEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTRLESSEQUALS:
			{
			if (AttrLessEquals)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_LESSEQUALS;
				AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrEquals = AttrSimilar = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKATTRSIMILAR:
			{
			if (AttrSimilar)
				{
				ActiveFilter->AttributeTest = WCS_DBFILTER_ATTRIBUTE_SIMILAR;
				AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrEquals = 0;
				} // if
			else
				ActiveFilter->AttributeTest = 0;
			break;
			} // 
		case IDC_CHECKGEOBNDSINSIDE:
			{
			if (ActiveFilter->GeoBndsInside)
				ActiveFilter->GeoBndsOutside = 0;
			break;
			} // 
		case IDC_CHECKGEOBNDSOUTSIDE:
			{
			if (ActiveFilter->GeoBndsOutside)
				ActiveFilter->GeoBndsInside = 0;
			break;
			} // 
		case IDC_CHECKGEOPTCONTAINED:
			{
			if (ActiveFilter->GeoPtContained)
				ActiveFilter->GeoPtUncontained = 0;
			break;
			} // 
		case IDC_CHECKGEOPTUNCONTAINED:
			{
			if (ActiveFilter->GeoPtUncontained)
				ActiveFilter->GeoPtContained = 0;
			break;
			} // 
		default:
			break;
		} // switch CtrlID

	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

return(0);

} // SearchQueryEditGUI::HandleSCChange

/*===========================================================================*/

long SearchQueryEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = NULL;

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

return(0);

} // SearchQueryEditGUI::HandleSRChange

/*===========================================================================*/

void SearchQueryEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7], UIInterested[7];
int Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;

// Layer update
UIInterested[0] = MAKE_ID(WCS_SUBCLASS_LAYER, WCS_SUBCLASS_LAYER, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
// Attribute updates
UIInterested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, WCS_RAHOST_OBJTYPE_DEM, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[2] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[3] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, WCS_RAHOST_OBJTYPE_VECTOR, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[4] = NULL;

if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		{
		if (((Changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	else if (ReceivingPoint)
		{
		if (((Changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			SetPoint((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	Done = 1;
	} // if

if (GlobalApp->AppEx->MatchNotifyClass(UIInterested, Changes, 0))
	{ // must be a layer/attribute update
	FillAttributeCBs();
	ConfigureWidgets();
	Done = 1;
	}

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureFilter();
	// advanced
	DisplayAdvancedFeatures();
	BuildFilterList();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // SearchQueryEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void SearchQueryEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Search Query - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureTB(NativeWin, IDC_ADDFILTER, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEFILTER, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEFILTERUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEFILTERDOWN, IDI_ARROWDOWN, NULL);

BuildFilterList();
ConfigureFilter();
// advanced
DisplayAdvancedFeatures();

} // SearchQueryEditGUI::ConfigureWidgets()

/*===========================================================================*/

void SearchQueryEditGUI::FillAttributeCBs(void)
{
LayerEntry *Entry;
const char *LayerName;
long ItemCt;

WidgetCBClear(IDC_ATTRIBDROP);
WidgetCBClear(IDC_LAYERDROP);

Entry = DBHost->DBLayers.FirstEntry();
while (Entry)
	{
	if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		LayerName = Entry->GetName();
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL || LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
			{
			ItemCt = WidgetCBInsert(IDC_ATTRIBDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_ATTRIBDROP, ItemCt, Entry);
			} // if an attribute layer
		else
			{
			ItemCt = WidgetCBInsert(IDC_LAYERDROP, -1, LayerName);
			WidgetCBSetItemData(IDC_LAYERDROP, ItemCt, Entry);
			} // else not attribute
		} // if
	Entry = DBHost->DBLayers.NextEntry(Entry);
	} // while

} // SearchQueryEditGUI::FillAttributeCBs

/*===========================================================================*/

void SearchQueryEditGUI::BuildFilterList(void)
{
DBFilterEvent *Current = Active->Filters, *CurrentFilter;
long Ct = 0, TempCt, Place, NumListItems, FoundIt;
unsigned MaxLen = 10;
char ListName[512];

NumListItems = WidgetLBGetCount(IDC_FILTERLIST);

ActiveFilterValid();

while (Current || Ct < NumListItems)
	{
	CurrentFilter = Ct < NumListItems ? (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == (DBFilterEvent *)CurrentFilter)
			{
			BuildFilterListEntry(ListName, Current);
			if (strlen(ListName) > MaxLen)
				MaxLen = (unsigned int)strlen(ListName);
			WidgetLBReplace(IDC_FILTERLIST, Ct, ListName);
			WidgetLBSetItemData(IDC_FILTERLIST, Ct, Current);
			if (Current == ActiveFilter)
				WidgetLBSetCurSel(IDC_FILTERLIST, Ct);
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current == (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildFilterListEntry(ListName, Current);
				if (strlen(ListName) > MaxLen)
					MaxLen = (unsigned int)strlen(ListName);
				WidgetLBReplace(IDC_FILTERLIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_FILTERLIST, TempCt, Current);
				if (Current == ActiveFilter)
					WidgetLBSetCurSel(IDC_FILTERLIST, TempCt);
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_FILTERLIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildFilterListEntry(ListName, Current);
				if (strlen(ListName) > MaxLen)
					MaxLen = (unsigned int)strlen(ListName);
				Place = WidgetLBInsert(IDC_FILTERLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_FILTERLIST, Place, Current);
				if (Current == ActiveFilter)
					WidgetLBSetCurSel(IDC_FILTERLIST, Place);
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_FILTERLIST, Ct);
		NumListItems --;
		} // else
	} // while

WidgetLBSetHorizExt(IDC_FILTERLIST, 5 * MaxLen);

} // SearchQueryEditGUI::BuildFilterList

/*===========================================================================*/

void SearchQueryEditGUI::BuildFilterListEntry(char *ListName, DBFilterEvent *Me)
{
int AddComma = 0, AddAmpersand = 0;

if (Me->Enabled && (((Me->PassControlPt || Me->PassVector) && (Me->PassLine || Me->PassPoint)) || Me->PassDEM) && (Me->PassEnabled || Me->PassDisabled))
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
	{
	strcat(ListName, "Add ");
	} // if
else
	{
	strcat(ListName, "Subtract ");
	} // else
if (Me->PassControlPt)
	{
	strcat(ListName, "Control Points");
	AddComma = AddAmpersand = 1;
	} // if
if (Me->PassVector)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Vectors");
	AddComma = AddAmpersand = 1;
	} // if
if (Me->PassDEM)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "DEMs");
	AddComma = 1;
	} // if
if (Me->PassEnabled)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Enabled");
	AddComma = AddAmpersand = 1;
	} // if
else
	AddAmpersand = 0;
if (Me->PassDisabled)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Disabled");
	AddComma = 1;
	} // if
if (Me->PassLine)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Lines");
	AddComma = AddAmpersand = 1;
	} // if
else
	AddAmpersand = 0;
if (Me->PassPoint)
	{
	if (AddAmpersand)
		strcat(ListName, " & ");
	else if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Points");
	AddComma = 1;
	} // if
if (((Me->LayerEquals || Me->LayerSimilar) && Me->Layer) || Me->LayerNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Layer ");
	if (Me->LayerNot)
		strcat(ListName, "Not ");
	if (Me->LayerEquals)
		strcat(ListName, "Equals ");
	else if (Me->LayerSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->LayerNumeric)
		strcat(ListName, "Numeric");
	if (! Me->LayerNumeric)
		strcat(ListName, Me->Layer);
	AddComma = 1;
	} // if
if (((Me->NameEquals || Me->NameSimilar) && Me->Name) || Me->NameNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Name ");
	if (Me->NameNot)
		strcat(ListName, "Not ");
	if (Me->NameEquals)
		strcat(ListName, "Equals ");
	else if (Me->NameSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->NameNumeric)
		strcat(ListName, "Numeric");
	if (! Me->NameNumeric)
		strcat(ListName, Me->Name);
	AddComma = 1;
	} // if
if (((Me->LabelEquals || Me->LabelSimilar) && Me->Label) || Me->LabelNumeric)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Label ");
	if (Me->LabelNot)
		strcat(ListName, "Not ");
	if (Me->LabelEquals)
		strcat(ListName, "Equals ");
	else if (Me->LabelSimilar)
		strcat(ListName, "Similar to ");
	else if (Me->LabelNumeric)
		strcat(ListName, "Numeric");
	if (! Me->LabelNumeric)
		strcat(ListName, Me->Label);
	AddComma = 1;
	} // if
if (Me->AttributeTest && Me->Attribute && Me->AttributeValue)
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Attribute Test");
	AddComma = 1;
	} // if
if ((Me->GeoPtContained || Me->GeoPtUncontained) || (Me->GeoBndsInside || Me->GeoBndsOutside))
	{
	if (AddComma)
		strcat(ListName, ", ");
	strcat(ListName, "Geographic Test");
	} // if

} // SearchQueryEditGUI::BuildFilterListEntry()

/*===========================================================================*/

DBFilterEvent *SearchQueryEditGUI::ActiveFilterValid(void)
{
DBFilterEvent *CurFilt;

if (ActiveFilter)
	{
	CurFilt = Active->Filters;
	while (CurFilt)
		{
		if (CurFilt == ActiveFilter)
			{
			return (ActiveFilter);
			} // if
		CurFilt = CurFilt->Next;
		} // while
	} // if

return (ActiveFilter = Active->Filters);

} // SearchQueryEditGUI::ActiveFilterValid

/*===========================================================================*/

void SearchQueryEditGUI::ConfigureFilter(void)
{
LayerEntry *Entry;
long ItemCt, NumEntries, MatchFound;

if (ActiveFilter = ActiveFilterValid())
	{
	AttrExists = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_EXISTS;
	AttrEquals = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_EQUALS;
	AttrGreater = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_GREATER;
	AttrGreaterEquals = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS;
	AttrLess = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_LESS;
	AttrLessEquals = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_LESSEQUALS;
	AttrSimilar = ActiveFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_SIMILAR;

	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOADD, &ActiveFilter->EventType, SRFlag_Char, WCS_DBFILTER_EVENTTYPE_ADD, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOSUB, &ActiveFilter->EventType, SRFlag_Char, WCS_DBFILTER_EVENTTYPE_SUB, NULL, NULL);

	ConfigureSC(NativeWin, IDC_CHECKFILTERENABLED, &ActiveFilter->Enabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCONTROLPT, &ActiveFilter->PassControlPt, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKVECTORS, &ActiveFilter->PassVector, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKDEMS, &ActiveFilter->PassDEM, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSENABLED, &ActiveFilter->PassEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSDISABLED, &ActiveFilter->PassDisabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSLINES, &ActiveFilter->PassLine, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSPOINTS, &ActiveFilter->PassPoint, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYEREQUALS, &ActiveFilter->LayerEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERSIMILAR, &ActiveFilter->LayerSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNUMERIC, &ActiveFilter->LayerNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNOT, &ActiveFilter->LayerNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMEEQUALS, &ActiveFilter->NameEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMESIMILAR, &ActiveFilter->NameSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENUMERIC, &ActiveFilter->NameNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENOT, &ActiveFilter->NameNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELEQUALS, &ActiveFilter->LabelEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELSIMILAR, &ActiveFilter->LabelSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNUMERIC, &ActiveFilter->LabelNumeric, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNOT, &ActiveFilter->LabelNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRNOT, &ActiveFilter->AttributeNot, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTREQUALS, &AttrEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRGREATER, &AttrGreater, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRGREATEREQUALS, &AttrGreaterEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRLESS, &AttrLess, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRLESSEQUALS, &AttrLessEquals, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRSIMILAR, &AttrSimilar, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTREXISTS, &AttrExists, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSINSIDE, &ActiveFilter->GeoBndsInside, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSOUTSIDE, &ActiveFilter->GeoBndsOutside, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSCOMPLETELY, &ActiveFilter->GeoBndsCompletely, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOPTCONTAINED, &ActiveFilter->GeoPtContained, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOPTUNCONTAINED, &ActiveFilter->GeoPtUncontained, SCFlag_Char, NULL, NULL);

	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	if (ActiveFilter->Name)
		WidgetSetText(IDC_NAMEMATCH, ActiveFilter->Name);
	else
		WidgetSetText(IDC_NAMEMATCH, "");

	WidgetSetModified(IDC_LABELMATCH, FALSE);
	if (ActiveFilter->Label)
		WidgetSetText(IDC_LABELMATCH, ActiveFilter->Label);
	else
		WidgetSetText(IDC_LABELMATCH, "");

	WidgetSetModified(IDC_ATTRVALUE, FALSE);
	if (ActiveFilter->AttributeValue)
		WidgetSetText(IDC_ATTRVALUE, ActiveFilter->AttributeValue);
	else
		WidgetSetText(IDC_ATTRVALUE, "");

	MatchFound = -1;
	if (ActiveFilter->Layer)
		{
		NumEntries = WidgetCBGetCount(IDC_LAYERDROP);
		for (ItemCt = 0; ItemCt < NumEntries; ItemCt ++)
			{
			if ((Entry = (LayerEntry *)WidgetCBGetItemData(IDC_LAYERDROP, ItemCt)) != (LayerEntry *)CB_ERR && Entry)
				{
				if (! strcmp((char *)Entry->GetName(), ActiveFilter->Layer))
					{
					MatchFound = ItemCt;
					break;
					} // if
				} // if an attribute layer
			} // for
		} // if
	WidgetCBSetCurSel(IDC_LAYERDROP, MatchFound);
	MatchFound = -1;
	if (ActiveFilter->Attribute)
		{
		NumEntries = WidgetCBGetCount(IDC_ATTRIBDROP);
		for (ItemCt = 0; ItemCt < NumEntries; ItemCt ++)
			{
			if ((Entry = (LayerEntry *)WidgetCBGetItemData(IDC_ATTRIBDROP, ItemCt)) != (LayerEntry *)CB_ERR && Entry)
				{
				if (! strcmp((char *)Entry->GetName(), ActiveFilter->Attribute))
					{
					MatchFound = ItemCt;
					break;
					} // if
				} // if an attribute layer
			} // for
		} // if
	WidgetCBSetCurSel(IDC_ATTRIBDROP, MatchFound);

	WidgetSmartRAHConfig(IDC_NORTH, &ActiveFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &ActiveFilter->GeoBnds);
	WidgetSmartRAHConfig(IDC_WEST, &ActiveFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &ActiveFilter->GeoBnds);
	WidgetSmartRAHConfig(IDC_EAST, &ActiveFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], &ActiveFilter->GeoBnds);
	WidgetSmartRAHConfig(IDC_SOUTH, &ActiveFilter->GeoBnds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], &ActiveFilter->GeoBnds);
	WidgetSmartRAHConfig(IDC_NORTH2, &ActiveFilter->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &ActiveFilter->GeoBnds);
	WidgetSmartRAHConfig(IDC_WEST2, &ActiveFilter->GeoPt.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &ActiveFilter->GeoBnds);

	WidgetSetDisabled(IDC_LAYERDROP, ! (ActiveFilter->LayerEquals || ActiveFilter->LayerSimilar));
	WidgetSetDisabled(IDC_NAMEMATCH, ! (ActiveFilter->NameEquals || ActiveFilter->NameSimilar));
	WidgetSetDisabled(IDC_LABELMATCH, ! (ActiveFilter->LabelEquals || ActiveFilter->LabelSimilar));
	WidgetSetDisabled(IDC_ATTRVALUE, ! (ActiveFilter->Attribute && (AttrEquals || AttrSimilar || AttrExists || AttrGreater || AttrLess || AttrGreaterEquals || AttrLessEquals)));
	WidgetSetDisabled(IDC_CHECKLAYERNOT, ! (ActiveFilter->LayerEquals || ActiveFilter->LayerSimilar || ActiveFilter->LayerNumeric));
	WidgetSetDisabled(IDC_CHECKNAMENOT, ! (ActiveFilter->NameEquals || ActiveFilter->NameSimilar || ActiveFilter->NameNumeric));
	WidgetSetDisabled(IDC_CHECKLABELNOT, ! (ActiveFilter->LabelEquals || ActiveFilter->LabelSimilar || ActiveFilter->LabelNumeric));
	WidgetSetDisabled(IDC_CHECKATTREXISTS, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTREQUALS, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRGREATER, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRGREATEREQUALS, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRLESS, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRLESSEQUALS, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRSIMILAR, ! (ActiveFilter->Attribute));
	WidgetSetDisabled(IDC_CHECKATTRNOT, ! (ActiveFilter->Attribute && (AttrEquals || AttrSimilar || AttrExists || AttrGreater || AttrLess || AttrGreaterEquals || AttrLessEquals)));
	WidgetSetDisabled(IDC_SETBOUNDS, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_SNAPBOUNDS, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_NORTH, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_WEST, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_EAST, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_SOUTH, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_CHECKGEOBNDSCOMPLETELY, ! (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside));
	WidgetSetDisabled(IDC_NORTH2, ! (ActiveFilter->GeoPtContained || ActiveFilter->GeoPtUncontained));
	WidgetSetDisabled(IDC_WEST2, ! (ActiveFilter->GeoPtContained || ActiveFilter->GeoPtUncontained));

	WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
	WidgetSetCheck(IDC_SETPT, ReceivingPoint);
	} // if
else
	{
	ReceivingDiagnostics = 0;
	ReceivingPoint = 0;
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOADD, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADD, IDC_RADIOSUB, NULL, 0, 0, NULL, NULL);

	ConfigureSC(NativeWin, IDC_CHECKFILTERENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCONTROLPT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKVECTORS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKDEMS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSDISABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSLINES, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKPASSPOINTS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYEREQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERSIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLAYERNOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMEEQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMESIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKNAMENOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELEQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELSIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNUMERIC, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKLABELNOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRNOT, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTREQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRGREATER, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRGREATEREQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRLESS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRLESSEQUALS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTRSIMILAR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKATTREXISTS, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSINSIDE, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSOUTSIDE, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOBNDSCOMPLETELY, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOPTCONTAINED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKGEOPTUNCONTAINED, NULL, 0, NULL, NULL);

	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	WidgetSetText(IDC_NAMEMATCH, "");

	WidgetSetModified(IDC_LABELMATCH, FALSE);
	WidgetSetText(IDC_LABELMATCH, "");

	WidgetSetModified(IDC_ATTRVALUE, FALSE);
	WidgetSetText(IDC_ATTRVALUE, "");

	WidgetCBSetCurSel(IDC_LAYERDROP, -1);
	WidgetCBSetCurSel(IDC_ATTRIBDROP, -1);

	WidgetSetDisabled(IDC_LAYERDROP, TRUE);
	WidgetSetDisabled(IDC_NAMEMATCH, TRUE);
	WidgetSetDisabled(IDC_LABELMATCH, TRUE);
	WidgetSetDisabled(IDC_ATTRIBDROP, TRUE);
	WidgetSetDisabled(IDC_ATTRVALUE, TRUE);
	WidgetSetDisabled(IDC_SETBOUNDS, TRUE);
	WidgetSetDisabled(IDC_SNAPBOUNDS, TRUE);

	WidgetSmartRAHConfig(IDC_NORTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_WEST, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_EAST, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SOUTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_NORTH2, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_WEST2, (RasterAnimHost *)NULL, NULL);
	} // else

} // SearchQueryEditGUI::ConfigureFilter

/*===========================================================================*/

// advanced
void SearchQueryEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_GEOPOINT_BOX, true);
	WidgetShow(IDC_GEOBOUNDS_BOX, true);
	WidgetShow(IDC_CHECKGEOBNDSINSIDE, true);
	WidgetShow(IDC_CHECKGEOBNDSOUTSIDE, true);
	WidgetShow(IDC_CHECKGEOBNDSCOMPLETELY, true);
	WidgetShow(IDC_NORTH, true);
	WidgetShow(IDC_WEST, true);
	WidgetShow(IDC_EAST, true);
	WidgetShow(IDC_SOUTH, true);
	WidgetShow(IDC_SETBOUNDS, true);
	WidgetShow(IDC_SNAPBOUNDS, true);
	WidgetShow(IDC_CHECKGEOPTCONTAINED, true);
	WidgetShow(IDC_CHECKGEOPTUNCONTAINED, true);
	WidgetShow(IDC_NORTH2, true);
	WidgetShow(IDC_WEST2, true);
	WidgetShow(IDC_SETPT, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_GEOPOINT_BOX, false);
	WidgetShow(IDC_GEOBOUNDS_BOX, false);
	WidgetShow(IDC_CHECKGEOBNDSINSIDE, false);
	WidgetShow(IDC_CHECKGEOBNDSOUTSIDE, false);
	WidgetShow(IDC_CHECKGEOBNDSCOMPLETELY, false);
	WidgetShow(IDC_NORTH, false);
	WidgetShow(IDC_WEST, false);
	WidgetShow(IDC_EAST, false);
	WidgetShow(IDC_SOUTH, false);
	WidgetShow(IDC_SETBOUNDS, false);
	WidgetShow(IDC_SNAPBOUNDS, false);
	WidgetShow(IDC_CHECKGEOPTCONTAINED, false);
	WidgetShow(IDC_CHECKGEOPTUNCONTAINED, false);
	WidgetShow(IDC_NORTH2, false);
	WidgetShow(IDC_WEST2, false);
	WidgetShow(IDC_SETPT, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // SearchQueryEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void SearchQueryEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // SearchQueryEditGUI::Cancel

/*===========================================================================*/

void SearchQueryEditGUI::Name(void)
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

} // SearchQueryEditGUI::Name()

/*===========================================================================*/

void SearchQueryEditGUI::NewMatchLayer(void)
{
LayerEntry *NewObj;
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_LAYERDROP);
if ((NewObj = (LayerEntry *)WidgetCBGetItemData(IDC_LAYERDROP, Current, 0)) != (LayerEntry *)LB_ERR && NewObj)
	{
	ActiveFilter->NewLayer((char *)NewObj->GetName());
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // SearchQueryEditGUI::NewMatchLayer()

/*===========================================================================*/

void SearchQueryEditGUI::NewMatchName(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAMEMATCH))
	{
	WidgetGetText(IDC_NAMEMATCH, 512, NewName);
	WidgetSetModified(IDC_NAMEMATCH, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewName(NewName);
		Active->SetFloating(1);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // SearchQueryEditGUI::NewMatchName()

/*===========================================================================*/

void SearchQueryEditGUI::NewMatchLabel(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_LABELMATCH))
	{
	WidgetGetText(IDC_LABELMATCH, 512, NewName);
	WidgetSetModified(IDC_LABELMATCH, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewLabel(NewName);
		Active->SetFloating(1);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // SearchQueryEditGUI::NewMatchLabel()

/*===========================================================================*/

void SearchQueryEditGUI::NewMatchAttribute(void)
{
LayerEntry *NewObj;
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_ATTRIBDROP);
if ((NewObj = (LayerEntry *)WidgetCBGetItemData(IDC_ATTRIBDROP, Current, 0)) != (LayerEntry *)LB_ERR && NewObj)
	{
	ActiveFilter->NewAttribute((char *)NewObj->GetName());
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // SearchQueryEditGUI::NewMatchLabel()

/*===========================================================================*/

void SearchQueryEditGUI::NewAttributeValue(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_ATTRVALUE))
	{
	WidgetGetText(IDC_ATTRVALUE, 512, NewName);
	WidgetSetModified(IDC_ATTRVALUE, FALSE);
	if (ActiveFilter)
		{
		ActiveFilter->NewAttributeValue(NewName);
		} // if
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // SearchQueryEditGUI::NewAttributeValue()

/*===========================================================================*/

void SearchQueryEditGUI::AddFilter(void)
{

ReceivingDiagnostics = ReceivingPoint = 0;
ActiveFilter = Active->AddFilter(NULL);
BuildFilterList();
ConfigureFilter();

} // SearchQueryEditGUI::AddFilter

/*===========================================================================*/

void SearchQueryEditGUI::RemoveFilter(void)
{
long RemoveItem;
DBFilterEvent *RemoveMe;

if ((RemoveItem = WidgetLBGetCurSel(IDC_FILTERLIST)) != LB_ERR)
	{
	if ((RemoveMe = (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, RemoveItem)) != (DBFilterEvent *)LB_ERR && RemoveMe)
		{
		ReceivingDiagnostics = ReceivingPoint = 0;
		Active->RemoveFilter(RemoveMe);
		BuildFilterList();
		ConfigureFilter();
		} // if
	else
		{
		UserMessageOK("Remove Filter", "There are no Filters selected to remove.");
		} // else
	} // if
else
	{
	UserMessageOK("Remove Filter", "There are no Filters selected to remove.");
	} // else

} // SearchQueryEditGUI::RemoveFilter

/*===========================================================================*/

void SearchQueryEditGUI::SetActiveFilter(void)
{
long Current;

ReceivingDiagnostics = ReceivingPoint = 0;
Current = WidgetLBGetCurSel(IDC_FILTERLIST);
Current = (long)WidgetLBGetItemData(IDC_FILTERLIST, Current);
if (Current != LB_ERR)
	ActiveFilter = (DBFilterEvent *)Current;
ConfigureFilter();

} // SearchQueryEditGUI::SetActiveFilter()

/*===========================================================================*/

void SearchQueryEditGUI::ChangeFilterListPosition(short MoveUp)
{
long MoveItem, NumListEntries, SendNotify = 0;
DBFilterEvent *MoveMe, *Current, *PrevFilt = NULL, *PrevPrevFilt = NULL, *StashFilt;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_FILTERLIST)) > 0)
	{
	if ((MoveItem = WidgetLBGetCurSel(IDC_FILTERLIST)) != LB_ERR)
		{
		if ((MoveMe = (DBFilterEvent *)WidgetLBGetItemData(IDC_FILTERLIST, MoveItem)) != (DBFilterEvent *)LB_ERR && MoveMe)
			{
			if (MoveUp && MoveItem > 0)
				{
				Current = Active->Filters;
				while (Current != MoveMe)
					{
					PrevPrevFilt = PrevFilt;
					PrevFilt = Current;
					Current = Current->Next;
					} // while
				if (Current)
					{
					if (PrevFilt)
						{
						StashFilt = Current->Next;
						if (PrevPrevFilt)
							{
							PrevPrevFilt->Next = Current;
							Current->Next = PrevFilt;
							} // if
						else
							{
							Active->Filters = Current;
							Active->Filters->Next = PrevFilt;
							} // else
						PrevFilt->Next = StashFilt;
						SendNotify = 1;
						} // else if
					} // if
				} // if
			else if (! MoveUp && MoveItem < NumListEntries - 1)
				{
				Current = Active->Filters;
				while (Current != MoveMe)
					{
					PrevPrevFilt = PrevFilt;
					PrevFilt = Current;
					Current = Current->Next;
					} // while
				if (Current)
					{
					if (Current->Next)
						{
						StashFilt = Current->Next->Next;
						if (PrevFilt)
							{
							PrevFilt->Next = Current->Next;
							PrevFilt->Next->Next = Current;
							} // if
						else
							{
							Active->Filters = Current->Next;
							Active->Filters->Next = Current;
							} // else
						Current->Next = StashFilt;
						SendNotify = 1;
						} // if move down
					} // if
				} // else if
			} // if
		} // if
	} // if

if (SendNotify)
	{
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // SearchQueryEditGUI::ChangeFilterListPosition

/*===========================================================================*/

void SearchQueryEditGUI::SelectDBItems(void)
{
// this code moved to SearchQuery::ActionNow
Active->ActionNow();
} // SearchQueryEditGUI::SelectDBItems

/*===========================================================================*/



void SearchQueryEditGUI::SetBounds(DiagnosticData *Data)
{

if (ActiveFilter)
	{
	if (ReceivingDiagnostics == 0)
		{
		ReceivingPoint = 0;
		if (UserMessageOKCAN("Set Geographic Bounds", "The next two points clicked in any View\n will become this Search Query's new bounds.\n\nPoints may be selected in any order."))
			{
			ReceivingDiagnostics = 1;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingDiagnostics == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				ReceivingDiagnostics = 2;
				} // if
			} // if
		else
			ReceivingDiagnostics = 0;
		} // else if
	else if (ReceivingDiagnostics == 2)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				ActiveFilter->SetBounds(LatEvent, LonEvent);
				ReceivingDiagnostics = 0;
				ConfigureWidgets();
				} // if
			} // if
		else
			ReceivingDiagnostics = 0;
		} // else if
	else
		ReceivingDiagnostics = 0;

	WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
	} // if

} // SearchQueryEditGUI::SetBounds

/*===========================================================================*/

void SearchQueryEditGUI::SetPoint(DiagnosticData *Data)
{

if (ActiveFilter)
	{
	if (ReceivingPoint == 0)
		{
		ReceivingDiagnostics = 0;
		if (UserMessageOKCAN("Set Geographic Bounds", "The next point clicked in any View\n will become this Search Query's new point."))
			{
			ReceivingPoint = 1;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingPoint == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				ActiveFilter->SetGeoPoint(LatEvent[0], LonEvent[0]);
				ReceivingPoint = 0;
				ConfigureWidgets();
				} // if
			} // if
		else
			ReceivingPoint = 0;
		} // else if
	else
		ReceivingPoint = 0;

	WidgetSetCheck(IDC_SETPT, ReceivingPoint);
	} // if

} // SearchQueryEditGUI::SetPoint

/*===========================================================================*/

bool SearchQueryEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || (ActiveFilter && (ActiveFilter->GeoBndsInside || ActiveFilter->GeoBndsOutside || 
	ActiveFilter->GeoPtContained || ActiveFilter->GeoPtUncontained)) ? true : false);
	
} // SearchQueryEditGUI::QueryLocalDisplayAdvancedUIVisibleState

