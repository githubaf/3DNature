// ThematicMapEditGUI.cpp
// Code for Cmap editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ThematicMapEditGUI.h"
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
#include "Interactive.h"
#include "Conservatory.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS


#ifdef WCS_EXPRESSIONS
#include "StatementParser.h"
#endif // WCS_EXPRESSIONS

char *ThematicMapEditGUI::TabNames[WCS_THEMATICMAPGUI_NUMTABS] = {"General", "Data"};
char *ThematicMapEditGUI::MultPresetNames[] = {"Inches to Meters", "Feet to Meters", "Yards to Meters", "Miles to Meters", "Decimal to Percent", "Percent to Decimal", "Radians to Degrees", "Building Stories to Meters", NULL};
double ThematicMapEditGUI::MultPresetValues[] = {.0254, .3048, .9144, 1609.344, 100.0, .01, 57.29577951, 3.65};

long ThematicMapEditGUI::ActivePage;
// advanced
long ThematicMapEditGUI::DisplayAdvanced;

NativeGUIWin ThematicMapEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ThematicMapEditGUI::Open

/*===========================================================================*/

NativeGUIWin ThematicMapEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_THEMATICMAP_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_THEMATICMAP_DATA, 0, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_THEMATICMAPGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		TabIndex = 0;
		while (MultPresetNames[TabIndex])
			{
			WidgetCBAddEnd(IDC_MULTPRESETS, MultPresetNames[TabIndex]);
			TabIndex ++;
			} // while
		SelectPresetName();
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // ThematicMapEditGUI::Construct

/*===========================================================================*/

ThematicMapEditGUI::ThematicMapEditGUI(EffectsLib *EffectsSource, Project *ProjSource, Database *DBSource, ThematicMap *ActiveSource)
: GUIFenetre('THEM', this, "Thematic Map Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, 0xff), 
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
EffectsHost = EffectsSource;
DBHost = DBSource;
ProjHost = ProjSource;
Active = ActiveSource;
AttrExists = AttrEquals = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
if (ActivePage == 2)
	ActivePage = 0;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Thematic Map Editor - %s", Active->GetName());
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

} // ThematicMapEditGUI::ThematicMapEditGUI

/*===========================================================================*/

ThematicMapEditGUI::~ThematicMapEditGUI()
{
SetTitle("Closing window, please wait...");
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ThematicMapEditGUI::~ThematicMapEditGUI()

/*===========================================================================*/

long ThematicMapEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_THM, 0);

return(0);

} // ThematicMapEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long ThematicMapEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // ThematicMapEditGUI::HandleShowAdvanced

/*===========================================================================*/

long ThematicMapEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_THM, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
/*
// <<<>>> left here only as a crumb demonstrating how WCS_EXPRESSIONS was testing, for future use
	case :
		{
		#ifdef WCS_EXPRESSIONS
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT))
			{
			char InputStr[256];
			Expression *Expr;

			InputStr[0] = 0;
			if (GetInputString("Enter Expression", "", InputStr))
				{
				if (Expr = new Expression())
					{
					Expr->CreateChain(InputStr);
					sprintf(InputStr, "The answer is %f", Expr->Evaluate());
					UserMessageOK("Expression Evaluator", InputStr);
					delete Expr;
					} // if
				} // if
			} // if
		else if (UserMessageOKCAN(Active->GetName(), "Are you sure you wish to embed this component into the current Project? The button you just clicked will disappear if you do."))
		#else // WCS_EXPRESSIONS
		if (UserMessageOKCAN(Active->GetName(), "Are you sure you wish to embed this component into the current Project? The button you just clicked will disappear if you do."))
		#endif // WCS_EXPRESSIONS
			Active->Embed();
		break;
		} //
*/
	default: break;
	} // ButtonID
return(0);

} // ThematicMapEditGUI::HandleButtonClick

/*===========================================================================*/

long ThematicMapEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ATTRIBDROP:
		{
		SelectNewAttribute(CtrlID, 0);
		break;
		}
	case IDC_ATTRIBDROP2:
		{
		SelectNewAttribute(CtrlID, 1);
		break;
		}
	case IDC_ATTRIBDROP3:
		{
		SelectNewAttribute(CtrlID, 2);
		break;
		}
	case IDC_ATTRIBDROP4:
		{
		SelectNewAttribute(CtrlID, 3);
		break;
		}
	case IDC_MULTPRESETS:
		{
		SelectNewMultPreset(CtrlID);
		break;
		}
	} // switch CtrlID

return (0);

} // ThematicMapEditGUI::HandleCBChange

/*===========================================================================*/

long ThematicMapEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_ATTRVALUE:
		{
		NewAttributeValue();
		break;
		} // 
	} // switch CtrlID

return (0);

} // ThematicMapEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long ThematicMapEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 0:
				{
				ShowPanel(0, 0);
				break;
				} // 0
			case 1:
				{
				ShowPanel(0, 1);
				break;
				} // 1
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

ActivePage = NewPageID;

return(0);

} // ThematicMapEditGUI::HandlePageChange

/*===========================================================================*/

long ThematicMapEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKUSEDEFAULTS:
	case IDC_CHECKCONDITIONENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTREXISTS:
		{
		if (AttrExists)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_EXISTS;
			AttrEquals = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTREQUALS:
		{
		if (AttrEquals)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_EQUALS;
			AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTRGREATER:
		{
		if (AttrGreater)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_GREATER;
			AttrExists = AttrEquals = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTRGREATEREQUALS:
		{
		if (AttrGreaterEquals)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS;
			AttrExists = AttrGreater = AttrEquals = AttrLess = AttrLessEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTRLESS:
		{
		if (AttrLess)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_LESS;
			AttrExists = AttrGreater = AttrGreaterEquals = AttrEquals = AttrLessEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTRLESSEQUALS:
		{
		if (AttrLessEquals)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_LESSEQUALS;
			AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrEquals = AttrSimilar = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKATTRSIMILAR:
		{
		if (AttrSimilar)
			{
			Active->Condition.AttributeTest = WCS_DBFILTER_ATTRIBUTE_SIMILAR;
			AttrExists = AttrGreater = AttrGreaterEquals = AttrLess = AttrLessEquals = AttrEquals = 0;
			} // if
		else
			Active->Condition.AttributeTest = 0;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // ThematicMapEditGUI::HandleSCChange

/*===========================================================================*/

long ThematicMapEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOOUTCHANNELS1:
	case IDC_RADIOOUTCHANNELS3:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // ThematicMapEditGUI::HandleSRChange

/*===========================================================================*/

long ThematicMapEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_MULT1:
	case IDC_MULT2:
	case IDC_MULT3:
		{
		SelectPresetName();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // ThematicMapEditGUI::HandleFIChange

/*===========================================================================*/

void ThematicMapEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7], UIInterested[7];
long Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

// Layer update
UIInterested[0] = MAKE_ID(WCS_SUBCLASS_LAYER, WCS_SUBCLASS_LAYER, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
// Attribute updates
UIInterested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, WCS_RAHOST_OBJTYPE_DEM, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[2] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[3] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, WCS_RAHOST_OBJTYPE_VECTOR, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_VALUECHANGED);
UIInterested[4] = NULL;

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	SyncWidgets();
	DisableWidgets();
	// advanced
	DisplayAdvancedFeatures();
	HideWidgets();
	Done = 1;
	} // if

if (GlobalApp->AppEx->MatchNotifyClass(UIInterested, Changes, 0))
	{ // must be a layer/attribute update
	ConfigureWidgets();
	}

if (! Done)
	ConfigureWidgets();

} // ThematicMapEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void ThematicMapEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Thematic Map Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureFI(NativeWin, IDC_DEFAULT1,
 &Active->NullConstant[0],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_DEFAULT2,
 &Active->NullConstant[1],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_DEFAULT3,
 &Active->NullConstant[2],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_MULT1,
 &Active->AttribFactor[0],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_MULT2,
 &Active->AttribFactor[1],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureFI(NativeWin, IDC_MULT3,
 &Active->AttribFactor[2],
  1.0,
   (double)-FLT_MAX,
	(double)FLT_MAX,
	 FIOFlag_Double,
	  NULL,
	   NULL);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKUSEDEFAULTS, &Active->NullTreatment, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCONDITIONENABLED, &Active->ConditionEnabled, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOOUTCHANNELS1, IDC_RADIOOUTCHANNELS1, &Active->OutputChannels, SRFlag_Char, WCS_THEMATICMAP_OUTCHANNELS_ONE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOOUTCHANNELS1, IDC_RADIOOUTCHANNELS3, &Active->OutputChannels, SRFlag_Char, WCS_THEMATICMAP_OUTCHANNELS_THREE, NULL, NULL);

AttrExists = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_EXISTS;
AttrEquals = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_EQUALS;
AttrGreater = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_GREATER;
AttrGreaterEquals = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_GREATEREQUALS;
AttrLess = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_LESS;
AttrLessEquals = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_LESSEQUALS;
AttrSimilar = Active->Condition.AttributeTest == WCS_DBFILTER_ATTRIBUTE_SIMILAR;

ConfigureSC(NativeWin, IDC_CHECKATTRNOT, &Active->Condition.AttributeNot, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTREQUALS, &AttrEquals, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTRGREATER, &AttrGreater, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTRGREATEREQUALS, &AttrGreaterEquals, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTRLESS, &AttrLess, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTRLESSEQUALS, &AttrLessEquals, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTRSIMILAR, &AttrSimilar, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKATTREXISTS, &AttrExists, SCFlag_Char, NULL, NULL);

WidgetSetModified(IDC_ATTRVALUE, FALSE);
if (Active->Condition.AttributeValue)
	WidgetSetText(IDC_ATTRVALUE, Active->Condition.AttributeValue);
else
	WidgetSetText(IDC_ATTRVALUE, "");

DisableWidgets();
HideWidgets();
FillAttributeCBs();
// advanced
DisplayAdvancedFeatures();

} // ThematicMapEditGUI::ConfigureWidgets()

/*===========================================================================*/

void ThematicMapEditGUI::FillAttributeCBs(void)
{
LayerEntry *Entry;
const char *LayerName;
long ItemCt, Found[4] = {-1, -1, -1, -1};

WidgetCBClear(IDC_ATTRIBDROP);
WidgetCBClear(IDC_ATTRIBDROP2);
WidgetCBClear(IDC_ATTRIBDROP3);
WidgetCBClear(IDC_ATTRIBDROP4);

Entry = DBHost->DBLayers.FirstEntry();
while (Entry)
	{
	if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		LayerName = Entry->GetName();
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
			{
			ItemCt = WidgetCBInsert(IDC_ATTRIBDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_ATTRIBDROP, ItemCt, Entry);
			if (Active->AttribField[0][0] && ! strcmp(LayerName, Active->AttribField[0]))
				Found[0] = ItemCt;
			ItemCt = WidgetCBInsert(IDC_ATTRIBDROP2, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_ATTRIBDROP2, ItemCt, Entry);
			if (Active->AttribField[1][0] && ! strcmp(LayerName, Active->AttribField[1]))
				Found[1] = ItemCt;
			ItemCt = WidgetCBInsert(IDC_ATTRIBDROP3, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_ATTRIBDROP3, ItemCt, Entry);
			if (Active->AttribField[2][0] && ! strcmp(LayerName, Active->AttribField[2]))
				Found[2] = ItemCt;
			} // if an attribute layer
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL ||
			LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
			{
			ItemCt = WidgetCBInsert(IDC_ATTRIBDROP4, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_ATTRIBDROP4, ItemCt, Entry);
			if (Active->Condition.Attribute && ! strcmp(LayerName, Active->Condition.Attribute))
				Found[3] = ItemCt;
			} // if
		} // if
	Entry = DBHost->DBLayers.NextEntry(Entry);
	} // while

if (Found[0] >= 0)
	WidgetCBSetCurSel(IDC_ATTRIBDROP, Found[0]);
if (Found[1] >= 0)
	WidgetCBSetCurSel(IDC_ATTRIBDROP2, Found[1]);
if (Found[2] >= 0)
	WidgetCBSetCurSel(IDC_ATTRIBDROP3, Found[2]);
if (Found[3] >= 0)
	WidgetCBSetCurSel(IDC_ATTRIBDROP4, Found[3]);

} // ThematicMapEditGUI::FillAttributeCBs

/*===========================================================================*/

void ThematicMapEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRNOT, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTREQUALS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRGREATER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRGREATEREQUALS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRLESS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRLESSEQUALS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTRSIMILAR, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATTREXISTS, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOOUTCHANNELS1, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOOUTCHANNELS3, WP_SRSYNC_NONOTIFY);

SelectPresetName();

} // ThematicMapEditGUI::SyncWidgets

/*===========================================================================*/

void ThematicMapEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_DEFAULT1, Active->NullTreatment == WCS_THEMATICMAP_NULLTREATMENT_IGNORE);
WidgetSetDisabled(IDC_DEFAULT2, Active->NullTreatment == WCS_THEMATICMAP_NULLTREATMENT_IGNORE);
WidgetSetDisabled(IDC_DEFAULT3, Active->NullTreatment == WCS_THEMATICMAP_NULLTREATMENT_IGNORE);
WidgetSetDisabled(IDC_ATTRVALUE, ! (Active->Condition.Attribute && (AttrEquals || AttrSimilar || AttrExists || AttrGreater || AttrLess || AttrGreaterEquals || AttrLessEquals)));
WidgetSetDisabled(IDC_CHECKATTREXISTS, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTREQUALS, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRGREATER, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRGREATEREQUALS, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRLESS, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRLESSEQUALS, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRSIMILAR, ! (Active->ConditionEnabled && Active->Condition.Attribute));
WidgetSetDisabled(IDC_CHECKATTRNOT, ! (Active->ConditionEnabled && Active->Condition.Attribute && (AttrEquals || AttrSimilar || AttrExists || AttrGreater || AttrLess || AttrGreaterEquals || AttrLessEquals)));
WidgetSetDisabled(IDC_ATTRIBDROP4, ! Active->ConditionEnabled);

} // ThematicMapEditGUI::DisableWidgets

/*===========================================================================*/

void ThematicMapEditGUI::HideWidgets(void)
{

WidgetShow(IDC_CHANNEL1TXT, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_CHANNEL1TXT2, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_ATTRIBDROP2, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_DEFAULT2, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_MULT2, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_CHANNEL2TXT, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_CHANNEL2TXT2, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_ATTRIBDROP3, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_DEFAULT3, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);
WidgetShow(IDC_MULT3, Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE);

} // ThematicMapEditGUI::HideWidgets

/*===========================================================================*/

// advanced
void ThematicMapEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_CHECKCONDITIONENABLED, true);
	WidgetShow(IDC_ATTRDROP_LABEL, true);
	WidgetShow(IDC_ATTRIBDROP4, true);
	WidgetShow(IDC_CHECKATTREXISTS, true);
	WidgetShow(IDC_CHECKATTRNOT, true);
	WidgetShow(IDC_ORVALUE_LABEL, true);
	WidgetShow(IDC_CHECKATTREQUALS, true);
	WidgetShow(IDC_CHECKATTRGREATER, true);
	WidgetShow(IDC_CHECKATTRGREATEREQUALS, true);
	WidgetShow(IDC_CHECKATTRLESS, true);
	WidgetShow(IDC_CHECKATTRLESSEQUALS, true);
	WidgetShow(IDC_CHECKATTRSIMILAR, true);
	WidgetShow(IDC_ATTRVALUE_LABEL, true);
	WidgetShow(IDC_ATTRVALUE, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_CHECKCONDITIONENABLED, false);
	WidgetShow(IDC_ATTRDROP_LABEL, false);
	WidgetShow(IDC_ATTRIBDROP4, false);
	WidgetShow(IDC_CHECKATTREXISTS, false);
	WidgetShow(IDC_CHECKATTRNOT, false);
	WidgetShow(IDC_ORVALUE_LABEL, false);
	WidgetShow(IDC_CHECKATTREQUALS, false);
	WidgetShow(IDC_CHECKATTRGREATER, false);
	WidgetShow(IDC_CHECKATTRGREATEREQUALS, false);
	WidgetShow(IDC_CHECKATTRLESS, false);
	WidgetShow(IDC_CHECKATTRLESSEQUALS, false);
	WidgetShow(IDC_CHECKATTRSIMILAR, false);
	WidgetShow(IDC_ATTRVALUE_LABEL, false);
	WidgetShow(IDC_ATTRVALUE, false);
	} // else


SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // ThematicMapEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void ThematicMapEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ThematicMapEditGUI::Cancel

/*===========================================================================*/

void ThematicMapEditGUI::Name(void)
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

} // ThematicMapEditGUI::Name()

/*===========================================================================*/

void ThematicMapEditGUI::SelectNewAttribute(int WidID, int AttribNum)
{
LayerEntry *NewObj;
long Current;

Current = WidgetCBGetCurSel(WidID);
if ((NewObj = (LayerEntry *)WidgetCBGetItemData(WidID, Current, 0)) != (LayerEntry *)LB_ERR && NewObj)
	{
	Active->SetAttribute(NewObj, AttribNum);
	} // if

} // ThematicMapEditGUI::SelectNewAttribute

/*===========================================================================*/

void ThematicMapEditGUI::SelectNewMultPreset(int WidID)
{
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(WidID);
Active->AttribFactor[0] = Active->AttribFactor[1] = Active->AttribFactor[2] = MultPresetValues[Current];
WidgetFISync(IDC_MULT1, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MULT2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_MULT3, WP_FISYNC_NONOTIFY);
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // ThematicMapEditGUI::SelectNewMultPreset

/*===========================================================================*/

void ThematicMapEditGUI::SelectPresetName(void)
{
long Current, Found = 0;

if (Active->AttribFactor[0] != 0.0 && (Active->OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_ONE || ((Active->AttribFactor[1] == 0.0 && Active->AttribFactor[2] == 0.0)
	|| (fabs(Active->AttribFactor[0] - Active->AttribFactor[1]) < .00001 &&
	fabs(Active->AttribFactor[0] - Active->AttribFactor[2]) < .00001))))
	{
	Current = 0;
	while (MultPresetNames[Current])
		{
		if (fabs(Active->AttribFactor[0] - MultPresetValues[Current]) < .00001)
			{
			Found = 1;
			WidgetCBSetCurSel(IDC_MULTPRESETS, Current);
			break;
			} // if
		Current ++;
		} // while
	} // if

if (! Found)
	WidgetCBSetCurSel(IDC_MULTPRESETS, -1);

} // ThematicMapEditGUI::SelectPresetName

/*===========================================================================*/

void ThematicMapEditGUI::NewAttributeValue(void)
{
char NewName[512];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_ATTRVALUE))
	{
	WidgetGetText(IDC_ATTRVALUE, 512, NewName);
	WidgetSetModified(IDC_ATTRVALUE, FALSE);
	Active->Condition.NewAttributeValue(NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // ThematicMapEditGUI::NewAttributeValue()

/*===========================================================================*/

