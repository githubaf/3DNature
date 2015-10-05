// NumericEntryGUI.cpp
// Code for NumericEntry editor
// Built from scratch on 2/29/00 by Gary R. Huber
// Copyright 2000 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "NumericEntryGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "resource.h"

NativeGUIWin NumericEntryGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // NumericEntryGUI::Open

/*===========================================================================*/

NativeGUIWin NumericEntryGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_GETNUMERIC, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // NumericEntryGUI::Construct

/*===========================================================================*/

NumericEntryGUI::NumericEntryGUI(AnimDoubleTime *ActiveSource)
: GUIFenetre('NUME', this, "Numeric Input") // Yes, I know...
{
AnimDoubleTime *NextSib;

ConstructError = 0;
Active1 = ActiveSource;
Active2 = Active3 = NULL;

if (ActiveSource)
	{
	Active1->Copy(&Backup1, Active1);
	if (Active1->RAParent)
		{
		// find siblings
		NextSib = (AnimDoubleTime *)Active1->RAParent->GetNextGroupSibling(Active1);
		if (NextSib && NextSib != Active1)
			{
			Active2 = NextSib;
			Active2->Copy(&Backup2, Active2);
			NextSib = (AnimDoubleTime *)Active1->RAParent->GetNextGroupSibling(Active2);
			if (NextSib && NextSib != Active1)
				{
				Active3 = NextSib;
				Active3->Copy(&Backup3, Active3);
				} // if
			} // if
		} // if
	} // if
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

GoModal();

} // NumericEntryGUI::NumericEntryGUI

/*===========================================================================*/

NumericEntryGUI::~NumericEntryGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);
EndModal();

} // NumericEntryGUI::~NumericEntryGUI()

/*===========================================================================*/

long NumericEntryGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_NUM, 0);

return(0);

} // NumericEntryGUI::HandleCloseWin

/*===========================================================================*/

long NumericEntryGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case IDC_OK:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_NUM, 0);
		break;
		} // 
	case IDCANCEL:
		{
		Cancel();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_NUM, 0);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // NumericEntryGUI::HandleButtonClick

/*===========================================================================*/

void NumericEntryGUI::ConfigureWidgets(void)
{
RasterAnimHostProperties Prop;
int Found;
char LabelText[256], LabelText2[256], *Marker;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME;

WidgetSNConfig(Active1 && Active2 ? IDC_PARAM1: IDC_PARAM2, Active1);
Active1->GetRAHostProperties(&Prop);
if (Prop.Name && Prop.Name[0])
	{
	strcpy(LabelText, Prop.Name);
	Marker = &LabelText[strlen(LabelText) - 1];
	if (*Marker == ')')
		{
		Marker --;
		if (Marker > LabelText && *Marker != '%')
			{
			Found = 0;
			while (Marker > LabelText && ! Found)
				{
				if (*Marker == '(')
					Found = 1;
				*Marker = 0;
				Marker --;
				} // while
			TrimTrailingSpaces(LabelText);
			} // if
		} // if
	LabelText[19] = 0;
	sprintf(LabelText2, "%s ", LabelText);
	if (Active2)
		WidgetSetText(IDC_PARAM1, LabelText2);
	else
		WidgetSetText(IDC_PARAM2, LabelText2);
	} // if
if (Active2)
	{
	WidgetSNConfig(IDC_PARAM2, Active2);
	Active2->GetRAHostProperties(&Prop);
	if (Prop.Name && Prop.Name[0])
		{
		strcpy(LabelText, Prop.Name);
		Marker = &LabelText[strlen(LabelText) - 1];
		if (*Marker == ')')
			{
			Marker --;
			if (Marker > LabelText && *Marker != '%')
				{
				Found = 0;
				while (Marker > LabelText && ! Found)
					{
					if (*Marker == '(')
						Found = 1;
					*Marker = 0;
					Marker --;
					} // while
				TrimTrailingSpaces(LabelText);
				} // if
			} // if
		LabelText[19] = 0;
		sprintf(LabelText2, "%s ", LabelText);
		WidgetSetText(IDC_PARAM2, LabelText2);
		} // if
	} // if
if (Active3)
	{
	WidgetSNConfig(IDC_PARAM3, Active3);
	Active3->GetRAHostProperties(&Prop);
	if (Prop.Name && Prop.Name[0])
		{
		strcpy(LabelText, Prop.Name);
		Marker = &LabelText[strlen(LabelText) - 1];
		if (*Marker == ')')
			{
			Marker --;
			if (Marker > LabelText && *Marker != '%')
				{
				Found = 0;
				while (Marker > LabelText && ! Found)
					{
					if (*Marker == '(')
						Found = 1;
					*Marker = 0;
					Marker --;
					} // while
				TrimTrailingSpaces(LabelText);
				} // if
			} // if
		LabelText[19] = 0;
		sprintf(LabelText2, "%s ", LabelText);
		WidgetSetText(IDC_PARAM3, LabelText2);
		} // if
	} // if

ShowWidgets();

} // NumericEntryGUI::ConfigureWidgets()

/*===========================================================================*/

void NumericEntryGUI::ShowWidgets(void)
{

WidgetShow(IDC_PARAM1, Active1 && Active2 ? 1: 0);
WidgetShow(IDC_PARAM3, Active3 ? 1: 0);

} // NumericEntryGUI::ShowWidgets

/*===========================================================================*/

void NumericEntryGUI::Cancel(void)
{
NotifyTag Changes[2];

Active1->Copy(Active1, &Backup1);
if (Active2)
	Active2->Copy(Active2, &Backup2);
if (Active3)
	Active3->Copy(Active3, &Backup3);

Changes[0] = MAKE_ID(Active1->GetNotifyClass(), Active1->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active1->GetRAHostRoot());

} // NumericEntryGUI::Cancel

/*===========================================================================*/
