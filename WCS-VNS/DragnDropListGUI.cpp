// DragnDropListGUI.cpp
// Code for DragnDrop List editor
// Built from DirectoryListGUI.cpp on 5/8/99 Gary Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "DragnDropListGUI.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "RasterAnimHost.h"
#include "EffectsLib.h"
#include "Texture.h"
#include "GraphData.h"
#include "requester.h"
#include "AppMem.h"
#include "Useful.h"
#include "resource.h"

NativeGUIWin DragnDropListGUI::Open(Project *Moi)
{
return(GUIFenetre::Open(Moi));
} // DragnDropListGUI::Open

/*===========================================================================*/

NativeGUIWin DragnDropListGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DRAGNDROP_LIST, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // DragnDropListGUI::Construct

/*===========================================================================*/

DragnDropListGUI::DragnDropListGUI(unsigned char IDSource, RasterAnimHostProperties *PropSource, RasterAnimHost **ListSource, 
	long NumItems, RootTextureParent *RootTextureCallBackSource, Texture *TextureCallBackSource, long *ListItemSource)
: GUIFenetre('DNDL', this, "Copy Object List") // Yes, I know...
{
char NameStr[256];

ConstructError = 0;
MyListID = IDSource;
NumListItems = NumItems;
ListItems = NULL;
DropSource = NULL;

if (RAHostList = (RasterAnimHost **)AppMem_Alloc(NumListItems * sizeof (RasterAnimHost *), 0))
	{
	memcpy(RAHostList, ListSource, NumListItems * sizeof (RasterAnimHost *));
	if (ListItemSource)
		{
		if (ListItems = (long *)AppMem_Alloc(NumListItems * sizeof (long), 0))
			memcpy(ListItems, ListItemSource, NumListItems * sizeof (long));
		else
			ConstructError = 1;
		} // if
	RootTextureCallBackHost = RootTextureCallBackSource;
	TextureCallBackHost = TextureCallBackSource;
	sprintf(NameStr, "Copy %s %s", PropSource->Name, PropSource->Type);
	SetTitle(NameStr);
	SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
	if (DropSource = new RasterAnimHostProperties)
		{
		DropSource->Copy(PropSource);
		} // if
	else
		ConstructError = 1;
	GoModal();
	} // if
else
	ConstructError = 1;

} // DragnDropListGUI::DragnDropListGUI

/*===========================================================================*/

DragnDropListGUI::~DragnDropListGUI()
{

if (ListItems)
	AppMem_Free(ListItems, NumListItems * sizeof (long));
if (RAHostList)
	AppMem_Free(RAHostList, NumListItems * sizeof (RasterAnimHost *));
if (DropSource)
	delete DropSource;

EndModal();

} // DragnDropListGUI::~DragnDropListGUI()

/*===========================================================================*/

long DragnDropListGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DDL, MyListID);

return(0);

} // DragnDropListGUI::HandleCloseWin

/*===========================================================================*/

long DragnDropListGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case IDC_APPLY:
		{
		if (DoApply())
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_DDL, MyListID);
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DDL, MyListID);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // DragnDropListGUI::HandleButtonClick

/*===========================================================================*/

void DragnDropListGUI::HandleNotifyEvent(void)
{

} // DragnDropListGUI::HandleNotifyEvent()

/*===========================================================================*/

void DragnDropListGUI::ConfigureWidgets(void)
{

BuildList();

} // DragnDropListGUI::ConfigureWidgets()

/*===========================================================================*/

void DragnDropListGUI::BuildList(void)
{
RasterAnimHostProperties *Prop;
int Ct;
char NameStr[256];

WidgetLBClear(IDC_PARLIST);

if (Prop = new RasterAnimHostProperties())
	{
	Prop->PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
	for (Ct = 0; Ct < NumListItems; Ct ++)
		{
		if (RAHostList[Ct])
			{
			RAHostList[Ct]->GetRAHostProperties(Prop);
			sprintf(NameStr, "%s %s", Prop->Name, Prop->Type);
			} // if
		else if (RootTextureCallBackHost && ListItems)
			sprintf(NameStr, "%s %s", RootTextureCallBackHost->GetTextureName(ListItems[Ct]), "(Texture)");
		else if (RootTextureCallBackHost)
			sprintf(NameStr, "%s %s", RootTextureCallBackHost->GetTextureName(Ct), "(Texture)");
		else if (TextureCallBackHost && ListItems && DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
			sprintf(NameStr, "%s %s", TextureCallBackHost->GetTextureName(ListItems[Ct]), "(Texture)");
		else if (TextureCallBackHost && DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
			sprintf(NameStr, "%s %s", TextureCallBackHost->GetTextureName(Ct), "(Texture)");
		else
			break;
		if (WidgetLBInsert(IDC_PARLIST, -1, NameStr) >= 0)
			WidgetLBSetItemData(IDC_PARLIST, Ct, RAHostList[Ct]);
		} // for
	delete Prop;
	} // if

} // DragnDropListGUI::BuildList()

/*===========================================================================*/

int DragnDropListGUI::DoApply(void)
{
char NameStr[256];
int Ct, Done = 0;
NotifyTag Changes[2];
RootTexture *NewRootTex;
Texture *NewTex;
RasterAnimHost *Data;

EndModal();

for (Ct = 0; Ct < NumListItems; Ct ++)
	{
	if (WidgetLBGetSelState(IDC_PARLIST, Ct))
		{
		Data = (RasterAnimHost *)WidgetLBGetItemData(IDC_PARLIST, Ct);
		if (Data)
			Data->SetRAHostProperties(DropSource);
		else if (RootTextureCallBackHost && ListItems)
			{
			if (NewRootTex = RootTextureCallBackHost->NewRootTexture(ListItems[Ct]))
				{
				((RasterAnimHost *)NewRootTex)->SetRAHostProperties(DropSource);
				} // if
			} // else if
		else if (RootTextureCallBackHost)
			{
			if (NewRootTex = RootTextureCallBackHost->NewRootTexture(Ct))
				{
				((RasterAnimHost *)NewRootTex)->SetRAHostProperties(DropSource);
				} // if
			} // else if
		else if (TextureCallBackHost && ListItems && DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
			{
			NameStr[0] = 0;
			WidgetLBGetText(IDC_PARLIST, Ct, NameStr);
			if (NameStr[0] && NameStr[0] != ' ')
				{
				if (NewTex = TextureCallBackHost->NewTexture(ListItems[Ct], (Texture *)DropSource->DropSource, ((Texture *)DropSource->DropSource)->GetTexType()))
					{
					((RasterAnimHost *)NewTex)->Copy(NewTex, (Texture *)DropSource->DropSource);
					Changes[0] = MAKE_ID(NewTex->GetNotifyClass(), NewTex->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NewTex->GetRAHostRoot());
					} // if
				} // if
			else
				UserMessageOK("Copy Texture", "Textures can only be copied to named parameters.");
			} // else if
		else if (TextureCallBackHost && DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
			{
			NameStr[0] = 0;
			WidgetLBGetText(IDC_PARLIST, Ct, NameStr);
			if (NameStr[0] && NameStr[0] != ' ')
				{
				if (NewTex = TextureCallBackHost->NewTexture(Ct, (Texture *)DropSource->DropSource, ((Texture *)DropSource->DropSource)->GetTexType()))
					{
					((RasterAnimHost *)NewTex)->Copy(NewTex, (Texture *)DropSource->DropSource);
					Changes[0] = MAKE_ID(NewTex->GetNotifyClass(), NewTex->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NewTex->GetRAHostRoot());
					} // if
				} // if
			else
				UserMessageOK("Copy Texture", "Textures can only be copied to named parameters.");
			} // else if
		else
			break;
		Done = 1;
		} // if
	} // for

if (! Done)
	{
	Done = ! UserMessageCustom("Copy Object", "No items are selected.\nWould you like to select some\n or cancel the copy operation?", "Select", "Cancel", NULL, 0);
	} // if

GoModal();
return (Done);

} // DragnDropListGUI::DoApply()

/*===========================================================================*/
