// DB3OneLayerGUI.cpp
// Code for DB3 single layer selection editor
// stolen from DB3LayersGUI.cpp 02/17/00 FPW2
// Copyright 2000 3D Nature

#include "stdafx.h"
#include "DB3OneLayerGUI.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "resource.h"

extern GUIContext *ReqLocalGUI;
extern WCSApp *GlobalApp;

NativeGUIWin DB3OneLayerGUI::Open(Project *Moi)
{

return(GUIFenetre::Open(Moi));

} // DB3OneLayerGUI::Open

/*===========================================================================*/

NativeGUIWin DB3OneLayerGUI::Construct(void)
{
long Item;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_DB3_ELEV, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		// Set up Widget bindings
		for (Item = 0; Item < NumItems; Item ++)
			WidgetLBInsert(IDC_PARLIST, -1, (char *)&LayerList[Item]);

		} // if
	} // if
 
return(NativeWin);

} // DB3OneLayerGUI::Construct

/*===========================================================================*/

DB3OneLayerGUI::DB3OneLayerGUI(DB3LayerList *ListSource, long ItemSource, long *SelectedSource)
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

} // DB3OneLayerGUI::DB3OneLayerGUI

/*===========================================================================*/

DB3OneLayerGUI::~DB3OneLayerGUI()
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

} // DB3OneLayerGUI::~DB3OneLayerGUI()

/*===========================================================================*/

long DB3OneLayerGUI::HandleCloseWin(NativeGUIWin NW)
{

ProcessData = 1;
AbortStatus = 1;

return(0);

} // DB3OneLayerGUI::HandleCloseWin

/*===========================================================================*/

long DB3OneLayerGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_DB3KEEP:
		{
		ProcessData = 1;
		AbortStatus = 1;
		return(1);
		} // SAVE
	case ID_DB3CANCEL:
		{
		AbortStatus = 1;
		return(1);
		} // SAVE
	default:
		break;
	} // ButtonID

return(0);

} // DB3OneLayerGUI::HandleButtonClick

/*===========================================================================*/

int DB3OneLayerGUI::CheckAbort(void)
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
} // DB3OneLayerGUI::CheckAbort

/*===========================================================================*/

void DB3OneLayerGUI::SetCaption(char *Label)
{

WidgetSetText(IDC_DB3_ELEVTEXT, Label);

} // DB3OneLayerGUI::SetCaption
