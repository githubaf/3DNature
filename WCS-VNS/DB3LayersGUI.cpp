// DB3LayersGUI.cpp
// Code for Wave editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "DB3LayersGUI.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "resource.h"

extern GUIContext *ReqLocalGUI;
extern WCSApp *GlobalApp;

NativeGUIWin DB3LayersGUI::Open(Project *Moi)
{
return(GUIFenetre::Open(Moi));
} // DB3LayersGUI::Open

/*===========================================================================*/

NativeGUIWin DB3LayersGUI::Construct(void)
{
long Item;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DB3_LAYERS, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		// Set up Widget bindings
		for (Item = 0; Item < NumItems; Item ++)
			WidgetLBInsert(IDC_PARLIST, -1, (char *)&LayerList[Item]);
		} // if
	} // if
 
return(NativeWin);
} // DB3LayersGUI::Construct

/*===========================================================================*/

DB3LayersGUI::DB3LayersGUI(DB3LayerList *ListSource, long ItemSource, long *SelectedSource)
: GUIFenetre('DB3L', this, "Database Fields") // Yes, I know...
{

ConstructError = 0;
LayerList = ListSource;
FieldsSelected = SelectedSource;
NumItems = ItemSource;
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
GUIFenetre::Open(GlobalApp->MainProj);
GUIFenetre::GoModal();
*FieldsSelected= 0;
AbortStatus = 0;
ProcessData = 0;

} // DB3LayersGUI::DB3LayersGUI

/*===========================================================================*/

DB3LayersGUI::~DB3LayersGUI()
{
char *LayerDude;
long Selected, StartByte = 0, Item;

// process selected fields
if (ProcessData)
	{
	for (Item = 0; Item < NumItems; Item ++)
		{
		LayerDude = (char *)&LayerList[Item];
		Selected = WidgetLBGetSelState(IDC_PARLIST, Item);
		if (Selected)
			{
			memcpy(&LayerDude[24], &StartByte, sizeof (long));
			LayerDude[31] = 1;
			*FieldsSelected += 1;
			} // if
		else
			LayerDude[31] = 0;
		StartByte += (unsigned char)LayerDude[16];
		} // for
	} // if
GUIFenetre::EndModal();

} // DB3LayersGUI::~DB3LayersGUI()

/*===========================================================================*/

long DB3LayersGUI::HandleCloseWin(NativeGUIWin NW)
{

ProcessData = 1;
AbortStatus = 1;

return(0);

} // DB3LayersGUI::HandleCloseWin

/*===========================================================================*/

long DB3LayersGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
char *LayerDude;
long StartByte = 0, Item;

switch(ButtonID)
	{
	case ID_DB3KEEP:
		{
		ProcessData = 1;
		AbortStatus = 1;
		return(1);
		} // OK
	case ID_DB3CANCEL:
		{
		AbortStatus = 1;
		return(1);
		} // CANCEL
	case ID_DB3SELECTALL:
		{
		*FieldsSelected = 0;
		for (Item = 0; Item < NumItems; Item ++)
			{
			LayerDude = (char *)&LayerList[Item];
			WidgetLBSetSelState(IDC_PARLIST, 1, Item);
			memcpy(&LayerDude[24], &StartByte, sizeof (long));
			LayerDude[31] = 1;
			*FieldsSelected += 1;
			StartByte += (unsigned char)LayerDude[16];
			} // for
		return(1);
		} // SELECTALL
	default:
		break;
	} // ButtonID

return(0);

} // DB3LayersGUI::HandleButtonClick

/*===========================================================================*/

int DB3LayersGUI::CheckAbort(void)
{
#ifdef _WIN32
MSG BusyEvent;
#endif // _WIN32

if(this && NativeWin)
	{
	// Special event checking
	#ifdef _WIN32
	while(ReqLocalGUI->CheckNoWait(&BusyEvent))
		{
		GlobalApp->ProcessOSEvent(&BusyEvent);
		} // while
	#endif // _WIN32

	return(AbortStatus);
	} // if

return(0);

} // DB3LayersGUI::CheckAbort
