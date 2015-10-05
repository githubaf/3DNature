// TableListGUI.cpp
// Code for RasterTA editor
// Built from scratch on 12/28/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "TableListGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Toolbar.h"
#include "EffectsLib.h"
#include "Useful.h"
#include "resource.h"

extern WCSApp *GlobalApp;

NativeGUIWin TableListGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TableListGUI::Open

/*===========================================================================*/

NativeGUIWin TableListGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_TABLE_LIST, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // TableListGUI::Construct

/*===========================================================================*/

TableListGUI::TableListGUI(CoordSys *ActiveSource, GenericTable *TableSource, char *FieldSource[4], 
	char ApplyToSource, char *DataTypeSource, long CurrentSelection)
: GUIFenetre('TBLG', this, "Coordinate System Selector") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff), 
								0};
char NameStr[256];

ConstructError = 0;
TableHost = TableSource;
ApplyTo = ApplyToSource;
Fields[0][0] = Fields[1][0] = Fields[2][0] = Fields[3][0] = 0;
if (FieldSource[0])
	strcpy(Fields[0], FieldSource[0]);
if (FieldSource[1])
	strcpy(Fields[1], FieldSource[1]);
if (FieldSource[2])
	strcpy(Fields[2], FieldSource[2]);
if (FieldSource[3])
	strcpy(Fields[3], FieldSource[3]);
FirstSelection = CurrentSelection;
Active = ActiveSource;
strcpy(DataType, DataTypeSource);
GoneModal = 0;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

if (ActiveSource && TableSource && Fields[0][0])
	{
	strcpy(NameStr, Active->GetName());
	SetTitle(NameStr);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	} // if
else
	ConstructError = 1;

} // TableListGUI::TableListGUI

/*===========================================================================*/

TableListGUI::~TableListGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (GoneModal)
	EndModal();

} // TableListGUI::~TableListGUI()

/*===========================================================================*/

long TableListGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TBG, 0);

return(0);

} // TableListGUI::HandleCloseWin

/*===========================================================================*/

long TableListGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case ID_KEEP:
		{
		ApplySelection();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TBG, 0);
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TBG, 0);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // TableListGUI::HandleButtonClick

/*===========================================================================*/

long TableListGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAMELIST:
		{
		ApplySelection();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TBG, 0);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // TableListGUI::HandleListDoubleClick

/*===========================================================================*/

long TableListGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAMELIST:
		{
		UpdateUsage();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // TableListGUI::HandleListSel

/*===========================================================================*/

void TableListGUI::HandleNotifyEvent(void)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TBG, 0);

} // TableListGUI::HandleNotifyEvent()

/*===========================================================================*/

void TableListGUI::ConfigureWidgets(void)
{
char Str[128];

sprintf(Str, "Select new %s", DataType);
WidgetSetText(IDC_ADD_TXT, Str);

BuildList();
WidgetLBSetCurSel(IDC_NAMELIST, FirstSelection);
UpdateUsage();

} // TableListGUI::ConfigureWidgets()

/*===========================================================================*/

void TableListGUI::BuildList(void)
{
long TabIndex;
long NameFieldNum;
char Str[256];

WidgetLBClear(IDC_NAMELIST);

if ((NameFieldNum = TableHost->FindFieldByName(Fields[0])) >= 0)
	{
	TabIndex = 0;
	while (TableHost->FetchFieldValueStr(TabIndex, NameFieldNum, Str, 256))
		{
		WidgetLBInsert(IDC_NAMELIST, TabIndex, Str);
		TabIndex ++;
		} // while
	} // if field number found

} // TableListGUI::BuildList()

/*===========================================================================*/

void TableListGUI::UpdateUsage(void)
{
long Current, UsageFieldNum, UsageCt;
char BigStr[1024], TextStr[256];

BigStr[0] = 0;
if ((Current = WidgetLBGetCurSel(IDC_NAMELIST)) != LB_ERR)
	{
	for (UsageCt = 0; UsageCt < 3; UsageCt ++)
		{
		if (Fields[UsageCt])
			{
			if ((UsageFieldNum = TableHost->FindFieldByName(Fields[UsageCt])) >= 0)
				{
				if (TableHost->FetchFieldValueStr(Current, UsageFieldNum, TextStr, 256))
					{
					strcat(BigStr, TextStr);
					strcat(BigStr, " ");
					} // if
				} // if
			} // if
		} // if
	} // if
WidgetSetText(IDC_USAGETXT, BigStr);

} // TableListGUI::UpdateUsage()

/*===========================================================================*/

void TableListGUI::ApplySelection(void)
{
long Current;

if ((Current = WidgetLBGetCurSel(IDC_NAMELIST)) != LB_ERR)
	Active->NewItemCallBack(Current, ApplyTo);

} // TableListGUI::ApplySelection

/*===========================================================================*/
