// EffectListGUI.cpp
// Code for GradientProfile List GUI
// Built from EffectListGUI.cpp on 6/17/97 by Gary Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectListGUI.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "resource.h"

NativeGUIWin EffectListGUI::Open(Project *Moi)
{
return(GUIFenetre::Open(Moi));
} // EffectListGUI::Open

/*===========================================================================*/

NativeGUIWin EffectListGUI::Construct(void)
{
char NameStr[256];

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_EFFECT_LIST, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		BuildList();
		if (CamToVec)
			{
			strcpy(NameStr, "Select a Camera to copy path from.");
			WidgetSetText(ID_KEEP, "Copy Path");
			} // if
		else if (Themes)
			{
			sprintf(NameStr, "Select a %s to add.", HostLib->GetEffectTypeName(EffectType));
			} // if
		else
			{
			sprintf(NameStr, "Select one or more %s to add.", HostLib->GetEffectTypeName(EffectType));
			} // if
		WidgetSetText(IDC_ADD_TXT, NameStr);
		GoModal();
		} // if

	} // if
 
return(NativeWin);

} // EffectListGUI::Construct

/*===========================================================================*/

EffectListGUI::EffectListGUI(EffectsLib *LibSource, RasterAnimHost *ActiveSource, long EffectTypeSource, ThematicOwner *ThemeOwnerSource, long ThemeNumberSource)
: GUIFenetre('GPLG', this, "Effect List") // Yes, I know...
{
char NameStr[256];

ConstructError = 0;
HostLib = LibSource;
ActiveHost = ActiveSource;
EffectType = EffectTypeSource;
HostList = LibSource->GetListPtr(EffectType);
CamToVec = Themes = 0;
ThemeNumber = ThemeNumberSource;
ThemeOwner = ThemeOwnerSource;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

if (HostList)
	{
	sprintf(NameStr, "%s %s", ActiveHost->GetRAHostName(), ActiveHost->GetRAHostTypeString());
	SetTitle(NameStr);
	if (EffectTypeSource == WCS_EFFECTSSUBCLASS_CAMERA && (ActiveHost->GetNotifyClass() == WCS_RAHOST_OBJTYPE_VECTOR
		|| ActiveHost->GetNotifyClass() == WCS_RAHOST_OBJTYPE_CONTROLPT))
		{
		CamToVec = 1;
		} // if
	else if (EffectTypeSource == WCS_EFFECTSSUBCLASS_THEMATICMAP && ThemeOwner)
		{
		Themes = 1;
		} // if
	} // if
else
	{
	sprintf(NameStr, "There are no %s to add.", LibSource->GetEffectTypeName(EffectType));
	UserMessageOK("Add Item", NameStr);
	ConstructError = 1;
	} // else


} // EffectListGUI::EffectListGUI

/*===========================================================================*/

EffectListGUI::~EffectListGUI()
{

if (! ConstructError)
	EndModal();

} // EffectListGUI::EffectListGUI

/*===========================================================================*/

long EffectListGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_EFL, 0);

return(0);

} // EffectListGUI::HandleCloseWin

/*===========================================================================*/

long EffectListGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
		{
		Apply();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EFL, 0);
		break;
		} // SAVE
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EFL, 0);
		break;
		} // SAVE
	} // ButtonID

return(0);

} // EffectListGUI::HandleButtonClick

/*===========================================================================*/

long EffectListGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_EFFECTLIST:
		{
		Apply();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_EFL, 0);
		break;
		}
	} // switch CtrlID

return (0);

} // EffectListGUI::HandleListDoubleClick

/*===========================================================================*/

void EffectListGUI::HandleNotifyEvent(void)
{

} // EffectListGUI::HandleNotifyEvent()

/*===========================================================================*/

void EffectListGUI::BuildList(void)
{
long Place;
GeneralEffect *Me;

WidgetLBClear(IDC_EFFECTLIST);

Me = HostList;
while (Me)
	{
	Place = WidgetLBInsert(IDC_EFFECTLIST, -1, Me->Name);
	WidgetLBSetItemData(IDC_EFFECTLIST, Place, Me);
	Me = Me->Next;
	} // while

} // EffectListGUI::BuildList

/*===========================================================================*/

void EffectListGUI::Apply(void)
{
long Ct, NumItems;
GeneralEffect *Test;
RasterAnimHostProperties Prop;

if ((NumItems = WidgetLBGetCount(IDC_EFFECTLIST)) > 0)
	{
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_EFFECTLIST, Ct))
			{
			if ((Test = (GeneralEffect *)WidgetLBGetItemData(IDC_EFFECTLIST, Ct)) != (GeneralEffect *)LB_ERR && Test)
				{
				if (Themes)
					{
					ThemeOwner->SetTheme(ThemeNumber, (ThematicMap *)Test);
					} // if
				else
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
					Test->GetRAHostProperties(&Prop);
					Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					Prop.DropSource = Test;
					ActiveHost->SetRAHostProperties(&Prop);
					} // else
				} // if
			} // if
		} // for
	} // if

} // EffectListGUI::Apply
