// RenderPreviewGUI.cpp
// Code for Render preview window
// Written from scratch 11/17/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RenderPreviewGUI.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Render.h"
#include "Notify.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Conservatory.h"
#include "RenderControlGUI.h"
#include "GUI.h"
#include "resource.h"

extern WCSApp *GlobalApp;

NativeDrwWin RenderPreviewGUI::Open(Project *Moi)
{
NativeDrwWin Success;

if (Success = DrawingFenetre::Open(Moi))
	{
	SetWinManFlags(WCS_FENETRE_WINMAN_ISRENDPREV);
	if (Rendering)
		GoModal();
	if(GlobalApp->GUIWins->RCG && GlobalApp->GUIWins->RCG->Rendering)
		{
		//AttachToDesktop();
		} // if
	SetupForDrawing();
	//GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // RenderPreviewGUI::Open

/*===========================================================================*/

void RenderPreviewGUI::AttachToRoot(void)
{
SetParent(NativeWin, LocalWinSys()->RootWin);
} // RenderPreviewGUI::AttachToRoot

void RenderPreviewGUI::AttachToDesktop(void)
{
SetParent(NativeWin, GetDesktopWindow());
} // RenderPreviewGUI::AttachToDesktop

/*===========================================================================*/

RenderPreviewGUI::RenderPreviewGUI(Renderer *RenderSource, int Running)
: DrawingFenetre('REND', this, "Render Preview") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, 0xff, 0xff, 0xff),
								0};

ConstructError = Rendering = 0;
RenderHost = RenderSource;
Sampling = 0;
LastX = LastY = 0;

if (RenderHost)
	{
	Rendering = Running;
	if (! RenderSource->Exporter)
		{
		if (Rendering)
			SetTitle("Rendering in Progress");
		else
			SetTitle("Rendering Complete");
		} // if
	else
		{
		if (Rendering)
			SetTitle("Export Render in Progress");
		else
			SetTitle("Export Render Complete");
		} // if
	} // if
else
	ConstructError = 1;

GlobalApp->AppEx->RegisterClient(this, AllEvents);
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_ISDIAG);

} // RenderPreviewGUI::RenderPreviewGUI

/*===========================================================================*/

RenderPreviewGUI::~RenderPreviewGUI()
{

GlobalApp->AppEx->RemoveClient(this);

if (Rendering)
	{
	CleanupFromDrawing();
	EndModal();
	} // if
//GlobalApp->AppEx->RemoveClient(this);
//GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // RenderPreviewGUI::~RenderPreviewGUI()

/*===========================================================================*/

long RenderPreviewGUI::HandleCloseWin(NativeGUIWin NW)
{
if (RenderHost)
	RenderHost->ClosePreview();
return(1);
} // RenderPreviewGUI::HandleCloseWin

/*===========================================================================*/

long RenderPreviewGUI::HandleLeftButtonDown(long int X, long int Y, char Alt, char Control, char Shift)
{

if (! Rendering)
	{
	Sampling = 1;
	RenderHost->SampleDiagnostics(X, Y, 0, 0);
	LastX = X;
	LastY = Y;
	} // if

return (0);

} // RenderPreviewGUI::HandleLeftButtonDown

/*===========================================================================*/

long RenderPreviewGUI::HandleLeftButtonDoubleClick(long int X, long int Y, char Alt, char Control, char Shift)
{

// suppress double-click-to-edit while digitizing
if(!GlobalApp->GUIWins->DIG) RenderHost->EditDiagnosticObject(X, Y);

return (0);

} // RenderPreviewGUI::HandleLeftButtonDoubleClick

/*===========================================================================*/

long RenderPreviewGUI::HandleLeftButtonUp(long int X, long int Y, char Alt, char Control, char Shift)
{

Sampling = 0;

return (0);

} // RenderPreviewGUI::HandleRightButtonDown

/*===========================================================================*/

long RenderPreviewGUI::HandleMouseMove(long int X, long int Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{

if (Sampling)
	{
	RenderHost->SampleDiagnostics(X, Y, X - LastX, Y - LastY);
	LastX = X;
	LastY = Y;
	} // if

return (0);

} // RenderPreviewGUI::HandleMouseMove

/*===========================================================================*/

long RenderPreviewGUI::HandlePopupMenuSelect(int MenuID)
{

switch(MenuID)
	{
/*
	case ID_WINMENU_DIAGNOSTICS:
		{
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
					WCS_TOOLBAR_ITEM_RDG, 0);
		break;
		} // 
*/
	case ID_WINMENU_SAVEIMAGE:
		{
		RenderHost->SaveDisplayedBuffers(0);
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // RenderPreviewGUI::HandlePopupMenuSelect

/*===========================================================================*/

void RenderPreviewGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	RenderHost->SetDisplayBuffer((unsigned char)(Changed & 0xff));
	} // if

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THRESHOLD, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	RenderHost->SetDisplayThreshold((unsigned char)((Changed & 0xff00) >> 8), (unsigned char)(Changed & 0xff));
	} // if

} // RenderPreviewGUI::HandleNotifyEvent()

/*===========================================================================*/

void RenderPreviewGUI::RenderDone(bool OpenDiagnostic)
{

CleanupFromDrawing();
EndModal();
Rendering = 0;

if (! RenderHost->Exporter)
	SetTitle("Rendering Complete");
else
	SetTitle("Export Rendering Complete");

if (OpenDiagnostic && ! GlobalApp->WinSys->InquireMinimized())
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_RDG, 1);

} // RenderPreviewGUI::RenderDone
