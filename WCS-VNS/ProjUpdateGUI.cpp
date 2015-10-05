// ProjUpdateGUI.cpp
// Code for ProjUpdateGUI window
// Built from VersionGUI.cpp on 3/10/08 by CXH

#undef WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include "AppMem.h"
#include "ProjUpdateGUI.h"
#include "WCSVersion.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Conservatory.h"
#include "resource.h"

//lint -save -e648

NativeGUIWin ProjUpdateGUI::Open(Project *Moi)
{
NativeGUIWin Success;
RECT Center;
if (Success = GUIFenetre::Open(Moi))
	{
	// center ourselves
	GetClientRect(NativeWin, &Center);
	SetWindowPos(NativeWin, NULL, (LocalWinSys()->InquireDisplayWidth() / 2) - (Center.right / 2), (LocalWinSys()->InquireDisplayHeight() / 2) - (Center.bottom / 2), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GlobalApp->MCP->AddWindowToMenuList(this);
	ShowIfNeeded(); // update messages
	} // if

return (Success);

} // ProjUpdateGUI::Open

/*===========================================================================*/

NativeGUIWin ProjUpdateGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_PROJ_UPDATE, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		// setup a larger italic font
		TipTextFont = CreateFont(14, 0, 0, 0, FW_NORMAL, 1, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /* CLEARTYPE_QUALITY */ , DEFAULT_PITCH | FF_SWISS, "arial");
		SendMessage(GetWidgetFromID(IDC_TEXTBOX), WM_SETFONT, (WPARAM)TipTextFont, 1);
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // ProjUpdateGUI::Construct

/*===========================================================================*/

ProjUpdateGUI::ProjUpdateGUI()
: GUIFenetre('PRJU', this, "Project Updated")
{
TipTextFont = NULL;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_SMTITLE);
_WarnAboutV5Displacement = _WarnAboutV6Displacement = _WarnAboutV6FractalDepth = _WarnAboutDensByPolygon = false;

} // ProjUpdateGUI::ProjUpdateGUI

/*===========================================================================*/

ProjUpdateGUI::~ProjUpdateGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);

if(TipTextFont) DeleteObject(TipTextFont);

} // ProjUpdateGUI::~ProjUpdateGUI()

/*===========================================================================*/


long ProjUpdateGUI::HandleCloseWin(NativeGUIWin NW)
{
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_PUW, 0);
return(0);

} // ProjUpdateGUI::HandleCloseWin

/*===========================================================================*/

long ProjUpdateGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_OK:
	case ID_KEEP:
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_PUW, 0);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // ProjUpdateGUI::HandleButtonClick

/*===========================================================================*/

void ProjUpdateGUI::ConfigureWidgets(void)
{
} // ProjUpdateGUI::ConfigureWidgets()

/*===========================================================================*/

bool ProjUpdateGUI::SetTerrainMessagesVisible(bool WarnAboutV5Displacement, bool WarnAboutV6Displacement, bool WarnAboutV6FractalDepth)
{
bool AnyChanges = false;
if (_WarnAboutV5Displacement != WarnAboutV5Displacement
	|| _WarnAboutV6Displacement != WarnAboutV6Displacement
	|| _WarnAboutV6FractalDepth != WarnAboutV6FractalDepth)
	AnyChanges = true;
_WarnAboutV5Displacement = WarnAboutV5Displacement;
_WarnAboutV6Displacement = WarnAboutV6Displacement;
_WarnAboutV6FractalDepth = WarnAboutV6FractalDepth;
return (AnyChanges);
} // ProjUpdateGUI::SetTerrainMessagesVisible

/*===========================================================================*/

bool ProjUpdateGUI::SetEcotypeMessagesVisible(bool WarnAboutDensByPolygon)
{
bool AnyChanges = false;
if (_WarnAboutDensByPolygon != WarnAboutDensByPolygon)
	AnyChanges = true;
_WarnAboutDensByPolygon = WarnAboutDensByPolygon;
return (AnyChanges);
} // ProjUpdateGUI::SetEcotypeMessagesVisible

/*===========================================================================*/


void ProjUpdateGUI::ShowIfNeeded(void)
{
char FullMessage[4000];

FullMessage[0] = 0;
if(_WarnAboutV5Displacement || _WarnAboutV5Displacement || _WarnAboutV6Displacement || _WarnAboutDensByPolygon)
	{
	strcat(FullMessage, "Some parameters in your project have changed in functionality. This Wizard helps explain \
these changes and advises you on how to adapt to them.\n\n");
	} // if


if(_WarnAboutV5Displacement)
	{
	strcat(FullMessage, "Displacement Change:\n The method used to compute fractal terrain vertical displacement \
has been changed since this file was created. You may need \
to adjust the Displacement value in the Terrain Parameters \
Editor to compensate. To figure the new Displacement value \
divide the old value by the smaller of the DEM cell dimensions.\n\n");
	} // if

if(_WarnAboutV6Displacement)
	{
	strcat(FullMessage, "Terrain Displacement:\n The method used to compute fractal terrain vertical displacement \
has been changed since this file was created. You need to adjust \
the Displacement value in the Terrain Parameters Editor to compensate. \
The old value was a percentage based on cell dimensions. \
The new value is an absolute amount of terrain height \
variation in meters. You will also need to add a texture to drive the amount of displacement.\n\n");
	} // if

if(_WarnAboutV6FractalDepth)
	{
	strcat(FullMessage, "Fractal Depth:\n Fractal Depth does not need to be as high as it did \
when this file was created. Fractal Depth is being reduced for you \
to a more appropriate value. You can make additional adjustments in \
the Terrain Parameters Editor if you wish more or less subdivision. \
The old value was higher to mask terrain facets and to improve edge \
sharpness of Terraffectors. Those are not issues any longer so to \
save rendering time and memory the smaller value for Fractal Depth \
is usually more appropriate. Bump Maps on Ground and Ecosystem \
materials can achieve with greater efficiency the same gritty look \
as high Fractal Depth used to create.\n\n");
	} // if

if(_WarnAboutDensByPolygon)
	{
	strcat(FullMessage, "Foliage:\n Foliage density can no longer be specified by polygon \
as it could be when this file was created. Foliage density can be \
specified by unit area and various units can be selected. Until you \
set the density units and adjust density to a more appropriate value \
you may notice that foliage is either too dense or not dense enough. \
Additional rendering time may result from high densities of vegetation. \
You may wish to use distant foliage dissove to shorten rendering times.");
	} // if

WidgetSetText(IDC_TEXTBOX, FullMessage);

} // ProjUpdateGUI::ShowIfNeeded
