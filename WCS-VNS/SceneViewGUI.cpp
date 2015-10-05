// SceneViewGUI.cpp
// Code for Scene at a Glance
// Built from SceneViewGUI.cpp on 5/5/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "SceneViewGUI.h"
#include "EffectsLib.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Application.h"
#include "Conservatory.h"
#include "RasterAnimHost.h"
#include "Toolbar.h"
#include "Security.h"
#include "Useful.h"
#include "Raster.h"
#include "Database.h"
#include "Joe.h"
#include "Project.h"
#include "Layers.h"
#include "Requester.h"
#include "AppMem.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "resource.h"
#include "DrillDownInfoGUI.h"
#include "DBEditGUI.h" // for Database Editor Select by Layer

// What, did you _forget_?
#ifndef TVIS_FOCUSED
#define TVIS_FOCUSED 0x0001
#endif // !TVIS_FOCUSED

// For debugging
//#define WCS_SCENEVIEW_SHOW_GROUPJOES

#define WCS_SCENEVIEW_BOTCONTAIN_MARGIN_LEFT	5
#define WCS_SCENEVIEW_BOTCONTAIN_MARGIN_TOP		25
#define WCS_SCENEVIEW_BOTCONTAIN_MARGIN_RIGHT	6
#define WCS_SCENEVIEW_BOTCONTAIN_MARGIN_BOTTOM	6

static unsigned char IconFilterList_Terrain[] = {
	WCS_EFFECTSSUBCLASS_RASTERTA,
	WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	WCS_EFFECTSSUBCLASS_PLANETOPT,
	WCS_EFFECTSSUBCLASS_TERRAINPARAM,
	WCS_EFFECTSSUBCLASS_GRIDDER,
	WCS_EFFECTSSUBCLASS_GENERATOR,
	WCS_EFFECTSSUBCLASS_COORDSYS,
	WCS_EFFECTSSUBCLASS_DEMMERGER,
	WCS_RAHOST_OBJTYPE_DEM,
	WCS_RAHOST_OBJTYPE_CONTROLPT,
	WCS_SUBCLASS_LAYER,
	255
	};

static unsigned char IconFilterList_LandCover[] = {
	WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	WCS_EFFECTSSUBCLASS_CMAP,
	WCS_EFFECTSSUBCLASS_FOLIAGE,
	WCS_EFFECTSSUBCLASS_GROUND,
	WCS_EFFECTSSUBCLASS_SNOW,
	WCS_EFFECTSSUBCLASS_THEMATICMAP,
	WCS_RAHOST_OBJTYPE_RASTER,
	255
	};

static unsigned char IconFilterList_Water[] = {
	WCS_EFFECTSSUBCLASS_LAKE,
	WCS_EFFECTSSUBCLASS_WAVE,
	WCS_EFFECTSSUBCLASS_STREAM,
	255
	};

static unsigned char IconFilterList_Air[] = {
	WCS_EFFECTSSUBCLASS_CLOUD,
	WCS_EFFECTSSUBCLASS_CELESTIAL,
	WCS_EFFECTSSUBCLASS_STARFIELD,
	WCS_EFFECTSSUBCLASS_SKY,
	WCS_EFFECTSSUBCLASS_ATMOSPHERE,
	255
	};

static unsigned char IconFilterList_Light[] = {
	WCS_EFFECTSSUBCLASS_ILLUMINATION,
	WCS_EFFECTSSUBCLASS_SHADOW,
	WCS_EFFECTSSUBCLASS_LIGHT,
	255
	};

static unsigned char IconFilterList_Object[] = {
	WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_MATERIAL,
	WCS_EFFECTSSUBCLASS_FENCE,
	WCS_EFFECTSSUBCLASS_LABEL,
	255
	};

static unsigned char IconFilterList_Vector[] = {
	WCS_RAHOST_OBJTYPE_VECTOR,
	WCS_EFFECTSSUBCLASS_SEARCHQUERY,
	WCS_EFFECTSSUBCLASS_COORDSYS,
	WCS_SUBCLASS_LAYER,
	255
	};

static unsigned char IconFilterList_Render[] = {
	WCS_EFFECTSSUBCLASS_CAMERA,
	WCS_EFFECTSSUBCLASS_RENDERJOB,
	WCS_EFFECTSSUBCLASS_RENDEROPT,
	WCS_EFFECTSSUBCLASS_POSTPROC,
	WCS_EFFECTSSUBCLASS_SCENARIO,
	WCS_EFFECTSSUBCLASS_EXPORTER,
	255
	};
// <<<>>> ADD_NEW_EFFECTS add it to one of the task modes

/*===========================================================================*/

NativeGUIWin SceneViewGUI::Open(Project *Moi)
{
NativeGUIWin Vous;
RECT Wreck;
POINT Convert;

if (Vous = GUIFenetre::Open(Moi))
	{
	GetWindowRect(Vous, &Wreck);
	Wreck.right  -= Wreck.left;
	Wreck.bottom -= Wreck.top;
	// This no longer causes HandleReSized()
	MoveAndSizeFen(0, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN),
	 (unsigned short)Wreck.right, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
	GetClientRect(Vous, &Wreck);
	HandleReSized(0, Wreck.right, Wreck.bottom);
	GetWindowRect(Vous, &Wreck);
	Convert.x = Wreck.right;
	Convert.y = Wreck.bottom;
	ScreenToClient(LocalWinSys()->RootWin, &Convert);
	GlobalApp->MainProj->SetRootMarker(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN, (unsigned short)Convert.x, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN));
	GlobalApp->MainProj->SetRootMarker(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE, GlobalApp->WinSys->InquireRootWidth() - (unsigned short)Convert.x, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
	SetSAGBottomHtPct(GlobalApp->MainProj->Prefs.SAGBottomHtPct);
	// adjust Matrix windows to SAG size
	if (GlobalApp->WinSys)
		{
		GlobalApp->WinSys->UpdateDocking(1);
		} // 
	} // if

return(Vous);
} // SceneViewGUI::Open

/*===========================================================================*/

NativeGUIWin SceneViewGUI::Construct(void)
{
HBITMAP hbitmap, hbmask;
int ExpandSize;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_SCENE_VIEW, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		#ifdef _WIN32
		GetWindowRect(NativeWin, &TVDragRect);
		SVGStockX = (int)TVDragRect.right - TVDragRect.left;
		ExpandSize = GlobalApp->MainProj->Prefs.SAGExpanded;
		if (ExpandSize < 30) ExpandSize = 30;
		ExpandedX = SVGStockX + ExpandSize;

		StdCursor = GetCursor();
		NoDropCursor = LoadCursor((HINSTANCE)LocalWinSys()->Instance(), MAKEINTRESOURCE(IDC_NODROPCURSOR));
		TVImageList = ImageList_Create(18, 12, ILC_COLOR32 | ILC_MASK, WCS_RAHOST_NUMICONTYPES, 0);
        hbitmap = LoadBitmap((HINSTANCE)LocalWinSys()->Instance(), "IDB_TVICONS");
		hbmask  = LoadBitmap((HINSTANCE)LocalWinSys()->Instance(), "IDB_TVICMASK");
        ImageList_Add(TVImageList, hbitmap, hbmask);
		DeleteObject(hbitmap);
		DeleteObject(hbmask);
		// Assign Image IDs like they used to be
		Image[WCS_RAHOST_ICONTYPE_MISC] = 29;	// misc.
		Image[WCS_RAHOST_ICONTYPE_VECTOR] = 42;
		Image[WCS_RAHOST_ICONTYPE_DEM] = 14;
		Image[WCS_RAHOST_ICONTYPE_CONTROLPT] = 13;
		Image[WCS_RAHOST_ICONTYPE_MATERIAL] = 26;

		Image[WCS_RAHOST_ICONTYPE_TERRAINPAR] = 15;	// terrain params
		Image[WCS_RAHOST_ICONTYPE_CLOUD] = 1; // cloud
		Image[WCS_RAHOST_ICONTYPE_LIGHT] = 19;
		Image[WCS_RAHOST_ICONTYPE_LAKE] = 43;
		Image[WCS_RAHOST_ICONTYPE_ECOTYPE] = 17;

		Image[WCS_RAHOST_ICONTYPE_FOLIAGEGROUP] = 22;	// foliage group
		Image[WCS_RAHOST_ICONTYPE_FOLIAGE] = 21;
		Image[WCS_RAHOST_ICONTYPE_ROOTTEXTURE] = 34;
		Image[WCS_RAHOST_ICONTYPE_TEXTURE] = 41;
		Image[WCS_RAHOST_ICONTYPE_RASTER] = 31;

		Image[WCS_RAHOST_ICONTYPE_GEOREGISTER] = 23;	// georegister
		Image[WCS_RAHOST_ICONTYPE_WAVESOURCE] = 45;
		Image[WCS_RAHOST_ICONTYPE_ANIMDOUBLETIME] = 6;
		Image[WCS_RAHOST_ICONTYPE_ANIMDOUBLEDISTANCE] = 2;
		Image[WCS_RAHOST_ICONTYPE_ANIMCOLORTIME] = 12;

		Image[WCS_RAHOST_ICONTYPE_ANIMCOLORGRADIENT] = 10;	// animcolorgradient
		Image[WCS_RAHOST_ICONTYPE_ANIMMATERIALGRADIENT] = 27;
		Image[WCS_RAHOST_ICONTYPE_COLORTEXTURE] = 11;
		Image[WCS_RAHOST_ICONTYPE_RENDER] = 33;
		Image[WCS_RAHOST_ICONTYPE_ENVIRONMENT] = 18;

		Image[WCS_RAHOST_ICONTYPE_CMAP] = 9;	// cmap
		Image[WCS_RAHOST_ICONTYPE_FOLEFFECT] = 20;
		Image[WCS_RAHOST_ICONTYPE_3DOBJECT] = 0;
		Image[WCS_RAHOST_ICONTYPE_PLANET] = 30;
		Image[WCS_RAHOST_ICONTYPE_SNOW] = 37;

		Image[WCS_RAHOST_ICONTYPE_STAR] = 38;	// starfield
		Image[WCS_RAHOST_ICONTYPE_TERRAFFECTOR] = 40;
		Image[WCS_RAHOST_ICONTYPE_RASTERTA] = 32;
		Image[WCS_RAHOST_ICONTYPE_SHADOW] = 35;
		Image[WCS_RAHOST_ICONTYPE_WAVE] = 44;

		Image[WCS_RAHOST_ICONTYPE_STREAM] = 39;	// stream
		Image[WCS_RAHOST_ICONTYPE_ATMOSPHERE] = 7;
		Image[WCS_RAHOST_ICONTYPE_SKY] = 36;
		Image[WCS_RAHOST_ICONTYPE_CELESTIAL] = 8;
		Image[WCS_RAHOST_ICONTYPE_ILLUMINATION] = 24;

		Image[WCS_RAHOST_ICONTYPE_MATERIALSTRATA] = 28;	// material strata
		Image[WCS_RAHOST_ICONTYPE_GRADIENTPROFILE] = 4;
		Image[WCS_RAHOST_ICONTYPE_ENVELOPE] = 3;
		Image[WCS_RAHOST_ICONTYPE_CROSSSECTION] = 5;
		Image[WCS_RAHOST_ICONTYPE_ECOSYSTEM] = 16;	// ecosystem

		Image[WCS_RAHOST_ICONTYPE_FENCE] = 46;	// wall
		Image[WCS_RAHOST_ICONTYPE_LABEL] = 47;	// label
		
		// new icons for WCS7/VNS3 era
		Image[WCS_RAHOST_ICONTYPE_COORDSYS] = 48;	// Coordinate System
		Image[WCS_RAHOST_ICONTYPE_DEMMERGE] = 49;	// DEM Merger
		Image[WCS_RAHOST_ICONTYPE_THEMATIC] = 50;	// Thematic Map
		Image[WCS_RAHOST_ICONTYPE_RENDJOB] = 51;	// Render Job
		Image[WCS_RAHOST_ICONTYPE_RENDOPT] = 52;	// Render Options
		Image[WCS_RAHOST_ICONTYPE_RENDSCEN] = 53;	// Render Scenario
		Image[WCS_RAHOST_ICONTYPE_SCHQY] = 54;	// Search Query
		Image[WCS_RAHOST_ICONTYPE_TERRAINGEN] = 55;	// Terrain Generator
		Image[WCS_RAHOST_ICONTYPE_TERRAINGRID] = 56;	// Terrain Gridder
		Image[WCS_RAHOST_ICONTYPE_SCENEEXP] = 57;	// Scene Exporter
		Image[WCS_RAHOST_ICONTYPE_POSTPROC] = 58;	// Post Process
		Image[WCS_RAHOST_ICONTYPE_GROUND] = 59;	// Ground
		Image[WCS_RAHOST_ICONTYPE_LAYER] = 60;	// Ground
		
		Image[WCS_RAHOST_ICONTYPE_KEYOVERLAY] = 25; // key overlay

		ImageList_SetOverlayImage(TVImageList, Image[WCS_RAHOST_ICONTYPE_KEYOVERLAY], 1);
		SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)TVImageList);
		SendDlgItemMessage(NativeWin, IDC_PARLIST2, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)TVImageList);
		
		AddLowerPanelTab("S@G", WCS_SCENEVIEWGUI_TAB_SAG, GetWidgetFromID(IDC_PARLIST2));
		// open diagnostics to populate that tab
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_RDG, 0);
		SwitchToTab(WCS_SCENEVIEWGUI_TAB_SAG);
		#endif // _WIN32

		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // SceneViewGUI::Construct

/*===========================================================================*/

SceneViewGUI::SceneViewGUI(Database *DBSource, EffectsLib *EffectsSource, Project *ProjSource, ImageLib *ImageSource)
: GUIFenetre('SAAG', this, "Scene at a Glance") // Yes, I know...
{
static NotifyTag AllEvents[] = {MAKE_ID(0xff, 0xff, 0xff, 0xff), 0};

ConstructError = 0;
DBHost = DBSource;
EffectsHost = EffectsSource;
ProjHost = ProjSource;
ImageHost = ImageSource;
TVImageList = TVDragList = NULL;
HitList = DragList = NULL;
HitItem = DragItem = NULL;
DragHost = HitHost = NULL;
DragProp = NULL;
Dragging = Editing = 0;
SVGStockX = ExpandedX = 0;
AutoResize = 1;
SystemResize = 0;
ModeList = 0;
Frozen = 0;

memset(TVListItemExpanded, 0, sizeof (TVListItemExpanded));

GlobalApp->AppEx->RegisterClient(this, AllEvents);
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_NOWINLIST | WCS_FENETRE_WINMAN_SMTITLE);

} // SceneViewGUI::SceneViewGUI

/*===========================================================================*/

SceneViewGUI::~SceneViewGUI()
{
if (GlobalApp->AppEx)
	GlobalApp->AppEx->RemoveClient(this);

ImageList_Destroy(TVImageList);
if (DragProp)
	delete DragProp;

} // SceneViewGUI::~SceneViewGUI()

/*===========================================================================*/

//lint -save -e648
long SceneViewGUI::HandleEvent(void)
{
LPMINMAXINFO LPMMI;
unsigned long Notify;
short ButtonID;

if (Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
	{
	switch(Activity->GUIMessage.message)
		{
		case WM_GETMINMAXINFO:
			{
			if (Activity->GUIMessage.hwnd ==  NativeWin)
				{
				LPMMI = (LPMINMAXINFO)Activity->GUIMessage.lParam;
				LPMMI->ptMaxTrackSize.x = AppScope->WinSys->InquireRootWidth();
				LPMMI->ptMaxTrackSize.y = AppScope->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
				LPMMI->ptMinTrackSize.x = SVGStockX;
				LPMMI->ptMinTrackSize.y = AppScope->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE);
				return(0);
				} // if
			else
				{
				LPMMI = (LPMINMAXINFO)Activity->GUIMessage.lParam;
				LPMMI->ptMaxTrackSize.x = AppScope->WinSys->InquireDisplayWidth();
				LPMMI->ptMaxTrackSize.y = AppScope->WinSys->InquireDisplayHeight();
				LPMMI->ptMinTrackSize.x = 50;
				LPMMI->ptMinTrackSize.y = 50;
				} // else
			break;
			} // MINMAXINFO
		case WM_MOUSEMOVE:
			{
			if (! Frozen && Dragging)
				{
				DoDragItem(LOWORD(Activity->GUIMessage.lParam), HIWORD(Activity->GUIMessage.lParam));
				} // if
			break;
			} // WM_MOUSEMOVE
		case WM_LBUTTONUP:
			{
			if (! Frozen && Dragging)
				EndDrag();
			break;
			} // WM_LBUTTONUP
		case WM_NOTIFY:
			{
			NMHDR *nmhdr;
			ButtonID  = LOWORD(Activity->GUIMessage.wParam);
			nmhdr    = (NMHDR *)Activity->GUIMessage.lParam;
			if (nmhdr) // can sometimes be NULL, and that's bad to dereference
				{
				Notify    = nmhdr->code;
				} // if
			else
				{
				break; // can't process it if we don't get any DATA!
				} // else

			if (! Frozen && Activity->GUIMessage.hwnd == NativeWin)
				{
				if (Notify == TVN_ITEMEXPANDING)
					{
					ExpandListItem(ButtonID, ButtonID == IDC_PARLIST2, Activity->GUIMessage.lParam, ButtonID == IDC_PARLIST1);
					} // if
				else if (Notify == TVN_ITEMEXPANDED)
					{
					ListItemExpanded(ButtonID, ButtonID == IDC_PARLIST2, Activity->GUIMessage.lParam);
					} // if
/*				else if (Notify == TVN_SELCHANGING)
					{
					LPNMTREEVIEW pnmtv;
					pnmtv = (LPNMTREEVIEW) Activity->GUIMessage.lParam;
					if ((pnmtv->itemOld.hItem == NULL) && (pnmtv->action == 0x00001000))
						{
						return(1);
						} // if
					} // if
*/				else if (Notify == TVN_SELCHANGED)
					{
					NM_TREEVIEW *pnmtv;
					pnmtv = (NM_TREEVIEW *)Activity->GUIMessage.lParam;
					// you can get this message when an item is being deleted. 
					// itemNew will point to the item being deleted which is very, very bad. 
					// Why would that behaviour be useful to anyone? Bad Microsoft. Beat yourselves!
					if ((pnmtv->itemOld.hItem == NULL) && (pnmtv->action == 0x00001000))
						{
						// Bogus select -- use sledgehammer to unselect.
						SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL);
						return(0);
						} // if

					ChangeSelection(ButtonID, Activity->GUIMessage.lParam);
					} // if
				else if (Notify == TVN_BEGINDRAG)
					{
					BeginDrag(ButtonID, Activity->GUIMessage.lParam);
					} // if
				else if (Notify == TVN_BEGINLABELEDIT)
					{
					return (BeginLabelEdit(ButtonID, Activity->GUIMessage.lParam));
					} // if
				else if (Notify == TVN_ENDLABELEDIT)
					{
					EndLabelEdit(ButtonID, Activity->GUIMessage.lParam);
					} // if
				} // if
			break;
			} // WM_NOTIFY
		default:
			break;
		} // switch
	} // else

return(0);

} // SceneViewGUI::HandleEvent
//lint -restore

/*===========================================================================*/

long SceneViewGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{
if (NW == NativeWin)
	{
	switch(CtrlID)
		{
		case IDC_PARLIST1:
		case IDC_PARLIST2:
			{
			RemoveObject(CtrlID);
			break;
			}
		default:
			break;
		} // switch
	} // if
return(0);
} // SceneViewGUI::HandleListDelItem

/*===========================================================================*/

long SceneViewGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{
if (NW == NativeWin)
	{
	switch(CtrlID)
		{
		case IDC_PARLIST1:
		case IDC_PARLIST2:
			{
			CopyObject(CtrlID);
			break;
			}
		default:
			break;
		} // switch
	} // if
return(0);
} // SceneViewGUI::HandleListCopyItem

/*===========================================================================*/

long SceneViewGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{
if (NW == NativeWin)
	{
	switch(CtrlID)
		{
		case IDC_PARLIST1:
		case IDC_PARLIST2:
			{
			PasteObject(CtrlID);
			break;
			}
		default:
			break;
		} // switch
	} // if
return(0);
} // SceneViewGUI::HandleListPasteItem

/*===========================================================================*/

long SceneViewGUI::HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{
//char DebugMessage[200], DebugName[200], DebugType[200];
//TV_ITEM TVItem;

if (NW == NativeWin)
	{
	switch(CtrlID)
		{
		case IDC_PARLIST1:
		case IDC_PARLIST2:
			{
			if (Derived)
				{
				// Pass along to RasterAnimHost for handling
				if (RAH)
					{
					RAH->HandlePopMenuSelection(ActionText);
					} // if
				} // if
			else
				{
				if (!stricmp(ActionText, "ENABLE"))
					{
					EnableObject(RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "DISABLE"))
					{
					DisableObject(RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "EMBED"))
					{
					EmbedObject(RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "EDIT"))
					{
					EditObject(CtrlID, RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "ADD"))
					{
					AddObject(CtrlID, TreeItem, 0, RAH);
					} // if
				else if (!stricmp(ActionText, "CLONE"))
					{
					AddObject(CtrlID, TreeItem, 0, RAH);
					} // if
				else if (!stricmp(ActionText, "CLONEVEC"))
					{
					AddObject(CtrlID, TreeItem, 0, RAH);
					} // else if
// these now appear in W6 builds
				else if (!strnicmp(ActionText, "CREATE", 6)) // or CREATETYPE or CREATE*
					{
					HandleTreeMenuCreate(Handle, NW, CtrlID, TreeItem, RAH, ActionText, Derived);
					} // if
				else if (!stricmp(ActionText, "ADDGALLERY"))
					{
					AddObject(CtrlID, TreeItem, 1, RAH);
					} // if
				else if (!stricmp(ActionText, "COPY"))
					{
					CopyObject(CtrlID, RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "PASTE"))
					{
					PasteObject(CtrlID, RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "GALLERY"))
					{
					if (RAH) RAH->OpenGallery(EffectsHost);
					} // if
				else if (!stricmp(ActionText, "LOAD"))
					{
					if (RAH) RAH->LoadComponentFile(NULL);
					} // if
				else if (!stricmp(ActionText, "SIGNANDSAVE"))
					{
					if (RAH) RAH->OpenBrowseData(EffectsHost);
					} // if
				else if (!stricmp(ActionText, "DELETE"))
					{
					RemoveObject(CtrlID, RAH, TreeItem);
					} // if
				else if (!stricmp(ActionText, "EDNUM"))
					{
					if (RAH) GlobalApp->MCP->NumericValue(RAH);
					} // if
				else if (!stricmp(ActionText, "EDVECPROF"))
					{
					if (RAH)
						{
						DBHost->SetActiveObj((Joe *)RAH);
						GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
							WCS_TOOLBAR_ITEM_VPG, 0);
						} // if
					} // if
				else if (!stricmp(ActionText, "PURGELAYERS"))
					{
					NotifyTag Changes[2];
					Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					DBHost->RemoveUnusedLayers();
					Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					} // if
				else if (!stricmp(ActionText, "SELECTLAYER"))
					{
					int Qualifier;
					RasterAnimHostProperties Prop;

					Qualifier = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1:
						LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL) ? 2: 0;
					Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
					Prop.FlagsMask = 0;
					RAH->GetRAHostProperties(&Prop);

					if (GlobalApp->GUIWins->DBE)
						{
						// safety checking
						if (Prop.TypeNumber == WCS_SUBCLASS_LAYER)
							{
							GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_DBG, 0);
							GlobalApp->GUIWins->DBE->DoLayerSelect((LayerEntry *)RAH, Qualifier);
							} // if
						} // if
					} // if
				return(0);
				} // else

			// unhandled event debugging code
/*			if (RAH)
				{
				if (RAH->GetRAHostName())
					{
					strcpy(DebugName, RAH->GetRAHostName());
					} // if
				if (RAH->GetRAHostTypeString())
					{
					strcpy(DebugType, RAH->GetRAHostTypeString());
					} // if
				} // if
			else
				{
				strcpy(DebugName, "(Unknown)"); // in case TVM_GETITEM fails
				TVItem.hItem = (HTREEITEM)TreeItem;
				TVItem.mask = (TVIF_HANDLE | TVIF_PARAM | TVIF_STATE | TVIF_TEXT);
				TVItem.stateMask = TVIS_SELECTED; 
				TVItem.pszText = &DebugName[0];
				TVItem.cchTextMax = 190;
				SendDlgItemMessage(NativeWin, CtrlID, TVM_GETITEM, 0, (LPARAM)&TVItem);
				sprintf(DebugType, "Category");
				} // else
			sprintf(DebugMessage, "%s %s: %s [%s]", DebugName, DebugType, ActionText, Derived ? "Derived" : "Base");
			UserMessageOK("S@G Menu Handler", DebugMessage);
*/
			break;
			}
		default:
			break;
		} // switch
	} // if
return(0);
} // SceneViewGUI::HandleTreeMenuSelect

/*===========================================================================*/

long SceneViewGUI::HandleTreeMenuCreate(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost, *RootHost;
RasterAnimHostProperties Prop;
long Category;

// these now appear in W6 builds
if (!strnicmp(ActionText, "CREATE", 6)) // or CREATETYPE or CREATE*
	{
	ListID = GetDlgItem(NativeWin, CtrlID);
	if (! Frozen && (TVItem.hItem = (HTREEITEM)TreeItem))
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditHost = (RasterAnimHost *)TVItem.lParam)
			{
			RootHost = EditHost->GetRAHostRoot();
			Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.TypeNumber = 0;
			RootHost->GetRAHostProperties(&Prop);
			Category = Prop.TypeNumber;
			RasterAnimHost::SetActiveRAHost(EditHost, 1); // We are S@G, so maybe not good to get recursive?
			GlobalApp->GUIWins->CVG->DoCreate(0);
			} // if 
		else
			{
			if ((Category = IdentifyCategory((unsigned long)TVItem.hItem, CtrlID == IDC_PARLIST2 ? 1: 0)) > 0)
				{
				// must force no Active object to get it to go by category
				RasterAnimHost::SetActiveRAHost(NULL, 1); // We are S@G, so maybe not good to get recursive?
				GlobalApp->GUIWins->CVG->DoCreate(0, Category);
				} // if
			/*
			for (Prop.TypeNumber = WCS_EFFECTSSUBCLASS_LAKE; Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS + 5; Prop.TypeNumber ++)
				{
				if (TVListItem[CtrlID == IDC_PARLIST2 ? 1: 0][Prop.TypeNumber] == TVItem.hItem)
					{
					if (Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
						{
						Category = Prop.TypeNumber;
						} // if effect
					else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS)
						{
						Category = WCS_RAHOST_OBJTYPE_VECTOR;
						} // else if vector
					else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 1)
						{
						Category = WCS_RAHOST_OBJTYPE_DEM;
						} // else if dem
					else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 2)
						{
						Category = WCS_RAHOST_OBJTYPE_CONTROLPT;
						} // else if control pt
					else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 3)
						{
						Category = WCS_RAHOST_OBJTYPE_RASTER;
						} // else if raster
					else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 4)
						{
						Category = WCS_SUBCLASS_LAYER;
						} // else if layer
					// must force no Active object to get it to go by category
					RasterAnimHost::SetActiveRAHost(NULL, 1); // We are S@G, so maybe not good to get recursive?
					GlobalApp->GUIWins->CVG->DoCreate(0, Category);
					break;
					} // if
				} // for
			*/
			} // else
		} // if
	Category = 0;
	} // if

return(0);
} // SceneViewGUI::HandleTreeMenuCreate

/*===========================================================================*/

long SceneViewGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARLIST1:
	case IDC_PARLIST2:
		{
		EditObject(CtrlID);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // SceneViewGUI::HandleListDoubleClick

/*===========================================================================*/

long SceneViewGUI::HandleCloseWin(NativeGUIWin NW)
{

if (NW ==  NativeWin)
	{
/*	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
		WCS_TOOLBAR_ITEM_SAG, 0);
*/
	GlobalApp->SetTerminate();
	} // if

return(0);

} // SceneViewGUI::HandleCloseWin

/*===========================================================================*/

long SceneViewGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case IDC_EDIT:
		{
		EditObject(IDC_PARLIST1);
		break;
		} // 
	case IDC_ADD:
		{
		AddObject(IDC_PARLIST1, 0, 1);
		break;
		} // 
	case IDC_REMOVE:
		{
		RemoveObject(IDC_PARLIST1);
		break;
		} // 
	case IDC_ENABLE:
		{
		EnableObject();
		break;
		} // 
	case IDC_DISABLE:
		{
		DisableObject();
		break;
		} // 
	case IDC_EXPAND:
		{
		DoWindowExpand(NW);
		break;
		} // 
	case IDC_FILTERENABLED:
		{
		GlobalApp->MainProj->Prefs.EnabledFilter = WidgetGetCheck(ButtonID);
		BuildList(1, IDC_PARLIST1, 0);
		break;
		} // 
	case IDC_FILTERANIMATED:
		{
		GlobalApp->MainProj->Prefs.AnimatedFilter = WidgetGetCheck(ButtonID);
		BuildList(1, IDC_PARLIST1, 0);
		break;
		} // 
	//case IDC_SHOWDBBYLAYER:
	//	{
	//	GlobalApp->MainProj->Prefs.ShowDBbyLayer = WidgetGetCheck(ButtonID);
	//	BuildList(1, IDC_PARLIST1, 0);
	//	break;
	//	} // 
	default:
		break;
	} // ButtonID

return(0);

} // SceneViewGUI::HandleButtonClick

/*===========================================================================*/

void SceneViewGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[4];
int Action, Group, SyncResolution;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	Freeze();
	return;
	} // if freeze
Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	UnFreeze();		// rebuilds lists
	return;
	} // if thaw
if (Frozen)
	return;

Changed = Changes[0];

if (Changed)
	{
	if (ParseNotifyEvent((unsigned char)((Changed & 0xff000000) >> 24), (unsigned char)((Changed & 0xff0000) >> 16), (unsigned char)((Changed & 0xff00) >> 8), (unsigned char)(Changed & 0xff),
		Activity->ChangeNotify->NotifyData, Action, Group))
		ProcessNotifyEvent(Action, Group, Activity->ChangeNotify->NotifyData);
	} // if

// kind of a hack, but don't know a better way of doing it
Interested[0] = MAKE_ID(WCS_SUBCLASS_LAYER, WCS_SUBCLASS_LAYER, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	ProcessNotifyEvent(WCS_SCENEVIEW_LISTACTION_BUILDLIST, (unsigned char)((Changed & 0xff000000) >> 24), Activity->ChangeNotify->NotifyData);
	} // 


Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[3] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncResolution = (((Changed & 0xff000000) >> 24) == WCS_RAHOST_OBJTYPE_DEM);
	GlobalApp->MainProj->Interactive->SyncFloaters(DBHost);
	GlobalApp->AppEffects->SyncFloaters(DBHost, ProjHost, SyncResolution);
	} // if thaw


} // SceneViewGUI::HandleNotifyEvent()

/*===========================================================================*/

void SceneViewGUI::ConfigureWidgets(void)
{

ConfigureTB(NativeWin, IDC_ENABLE, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_DISABLE, IDI_DISABLE, NULL);

ConfigureTB(NativeWin, IDC_ADD, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_EXPAND, IDI_ICONEXPANDX, NULL);
ConfigureTB(NativeWin, IDC_REMOVE, IDI_DELETE, NULL);

ConfigureTB(NativeWin, IDC_FILTERENABLED, IDI_SHOWENABLED, NULL);
ConfigureTB(NativeWin, IDC_FILTERANIMATED, IDI_SHOWANIMATED, NULL);
//ConfigureTB(NativeWin, IDC_SHOWDBBYLAYER, IDI_SHOWDBBYLAYER, NULL);

if (GlobalApp->MCP)
	{
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_TERRAIN, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_TERRAIN);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_LANDCOVER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_LANDCOVER);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_WATER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_WATER);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_SKY, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_AIR);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_LIGHT, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_LIGHT);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_3DOBJ, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_OBJECT);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_VECTOR, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_VECTOR);
	GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_RENDER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_RENDER);
	} // if

WidgetSetCheck(IDC_FILTERENABLED, GlobalApp->MainProj->Prefs.EnabledFilter);
WidgetSetCheck(IDC_FILTERANIMATED, GlobalApp->MainProj->Prefs.AnimatedFilter);
//WidgetSetCheck(IDC_SHOWDBBYLAYER, GlobalApp->MainProj->Prefs.ShowDBbyLayer);

BuildList(1, IDC_PARLIST1, 0);
BuildList(0, IDC_PARLIST2, 1);
SetSAGBottomHtPct(GlobalApp->MainProj->Prefs.SAGBottomHtPct);

} // SceneViewGUI::ConfigureWidgets()

/*===========================================================================*/

int SceneViewGUI::ParseNotifyEvent(unsigned char Class, unsigned char Subclass, unsigned char Item, unsigned char Comp,
	void *Data, int &Action, int &Group)
{

Action = 0;
Group = Class;

if (Class == 0xff)
	{
	Action = WCS_SCENEVIEW_LISTACTION_BUILDLIST;
	} // if
else if (Subclass == 0xff && ! Data)
	{
	Action = WCS_SCENEVIEW_LISTACTION_BUILDSECTION;
	} // if
else if (Subclass == Class && (Comp == WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED || ! Data))
	{
	Action = WCS_SCENEVIEW_LISTACTION_UPDATESECTION;
	} // if
else if (Data)
	{
	if (Subclass == 0xff)
		{
		Action = WCS_SCENEVIEW_LISTACTION_BUILDOBJECT;
		} // if
	else if (Comp == WCS_NOTIFYCOMP_OBJECT_NAMECHANGED)
		{
		Action = WCS_SCENEVIEW_LISTACTION_UPDATEOBJECT;
		} // if
	else if (Comp == WCS_NOTIFYCOMP_ANIM_NODEADDED || Comp == WCS_NOTIFYCOMP_ANIM_NODEREMOVED
		|| Comp == WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED || Comp == WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED
		|| Comp == WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED || Comp == WCS_NOTIFYCOMP_OBJECT_CHANGED)
		{
		Action = WCS_SCENEVIEW_LISTACTION_UPDATEOBJPLUSHDR;
		} // if
	} // if

return (Action);

} // SceneViewGUI::ParseNotifyEvent

/*===========================================================================*/

long SceneViewGUI::IdentifyCategory(unsigned long HitItem, long ListNum)
{
long CategoryCt, Category = 0, Repeat = 0;

if (ListNum < 0 || ListNum > 1)
	{
	Repeat = 1;
	ListNum = 0;
	} // if

RepeatTest:

for (CategoryCt = WCS_EFFECTSSUBCLASS_LAKE; CategoryCt < WCS_MAXIMPLEMENTED_EFFECTS + 5; CategoryCt ++)
	{
	if (TVListItem[ListNum][CategoryCt] == (HTREEITEM)HitItem)
		{
		if (CategoryCt < WCS_MAXIMPLEMENTED_EFFECTS)
			{
			Category = CategoryCt;
			} // if effect
		else if (CategoryCt == WCS_MAXIMPLEMENTED_EFFECTS)
			{
			Category = WCS_RAHOST_OBJTYPE_VECTOR;
			} // else if vector
		else if (CategoryCt == WCS_MAXIMPLEMENTED_EFFECTS + 1)
			{
			Category = WCS_RAHOST_OBJTYPE_DEM;
			} // else if dem
		else if (CategoryCt == WCS_MAXIMPLEMENTED_EFFECTS + 2)
			{
			Category = WCS_RAHOST_OBJTYPE_CONTROLPT;
			} // else if control pt
		else if (CategoryCt == WCS_MAXIMPLEMENTED_EFFECTS + 3)
			{
			Category = WCS_RAHOST_OBJTYPE_RASTER;
			} // else if raster
		else if (CategoryCt == WCS_MAXIMPLEMENTED_EFFECTS + 4)
			{
			Category = WCS_SUBCLASS_LAYER;
			} // else if layer
		break;
		} // if
	} // for

if (Repeat)
	{
	ListNum = 1;
	Repeat = 0;
	goto RepeatTest;
	} // if

return (Category);

} // SceneViewGUI::IdentifyCategory

/*===========================================================================*/

void SceneViewGUI::BuildList(short FilterList, short ListID, short ListNum)
{
RasterAnimHost *CurHost;
RasterAnimHostProperties *Prop;
long Group, Animated[3], Enabled[3], AnimEnabled[3], Children[3], SearchType;
unsigned long ObjLimitCount = 0, ObjCount = 0,
 SortLimit, DisplayLimit;
TV_INSERTSTRUCT *TVInsert = NULL;
HTREEITEM SortType = TVI_SORT;
void *Parent, *MyHandle;
Joe *Local;
LayerEntry *Entry;

SortLimit = GlobalApp->MainProj->Prefs.MaxSortedSAGDBEntries;
DisplayLimit = GlobalApp->MainProj->Prefs.MaxSAGDBEntries;

// Clearing the list can be a real bugger because it sends messages that selection is changing
// which triggers other undesirable actions like trying to access methods on the data
// pointer stored with whatever it thinks is the new active item. God, it seems
// obvious that you wouldn't want to hear about that when the list is being cleared
// or are they just that stupid?
Frozen = 1;
WidgetTVDeleteAll(ListID);
Frozen = 0;

SetModeListPtr();

if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
	{
	TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	TVInsert->item.stateMask = TVIS_SELECTED | TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
	TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
	TVInsert->item.cchTextMax = 256;
	TVInsert->hParent = TVI_ROOT;
	TVInsert->hInsertAfter = TVI_SORT;

	if (Prop = new RasterAnimHostProperties())
		{
		TVListItem[ListNum][WCS_EFFECTSSUBCLASS_GENERIC] = NULL; 
		// insert EffectsLib categories
		for (Group = WCS_EFFECTSSUBCLASS_COMBO; Group < WCS_MAXIMPLEMENTED_EFFECTS; Group ++)
			{
			if (EffectsHost->EffectTypeImplemented(Group) && (! FilterList || TaskModeFilter(Group)))
				{
				sprintf(TVInsert->item.pszText, "%s", EffectsHost->GetEffectTypeName(Group));
				TVInsert->item.cChildren = (CurHost = EffectsHost->GetListPtr(Group)) ? 1: 0;
				Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
				Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED | WCS_RAHOST_FLAGBIT_ENABLED;
				Animated[0] = Enabled[0] = AnimEnabled[0] = 0;
				while (CurHost)
					{
					CurHost->GetRAHostProperties(Prop);
					if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
						{
						Animated[0] = 1;
						} // if
					if (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED)
						{
						if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
							AnimEnabled[0] = 1;
						Enabled[0] = 1;
						} // if
					CurHost = ((GeneralEffect *)CurHost)->Next;
					} // while
				if (! FilterList || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
					&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
					&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
					{
					Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE);
					Prop->FlagsMask = (unsigned long)~0;
					TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
					if (Animated[0])
						TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
					if (TVListItemExpanded[ListNum][Group] && TVInsert->item.cChildren)
						TVInsert->item.state |= TVIS_EXPANDED;
					TVInsert->item.lParam = NULL;
					TVInsert->item.iImage = Image[EffectsHost->GetIconType(Group)];
					TVInsert->item.iSelectedImage = TVInsert->item.iImage;
					Parent = (void *)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
					TVListItem[ListNum][Group] = Parent; 
					if (CurHost = EffectsHost->GetListPtr(Group))
						{
						while (CurHost)
							{
							InsertRAHost(CurHost, Prop, ListID, ListNum, Parent, TVInsert, FilterList);
							TVInsert->hInsertAfter = TVI_SORT;
							CurHost = ((GeneralEffect *)CurHost)->Next;
							} // while
						} // if
					TVInsert->hParent = TVI_ROOT;
					} // if
				else
					TVListItem[ListNum][Group] = NULL; 
				} // if
			else
				TVListItem[ListNum][Group] = NULL; 
			} // for

		// vectors, dems, control pts
		DBHost->ResetGeoClip();

		// build category headers for vectors, dems and control points
		for (Group = WCS_MAXIMPLEMENTED_EFFECTS; Group < WCS_MAXIMPLEMENTED_EFFECTS + 3; Group ++)
			{
			if (Group == WCS_MAXIMPLEMENTED_EFFECTS)
				{
				SearchType = WCS_RAHOST_OBJTYPE_VECTOR;
				sprintf(TVInsert->item.pszText, "%s", "Vectors");
				TVInsert->item.iImage = Image[WCS_RAHOST_ICONTYPE_VECTOR];
				} // if
			else if (Group == WCS_MAXIMPLEMENTED_EFFECTS + 1)
				{
				SearchType = WCS_RAHOST_OBJTYPE_DEM;
				sprintf(TVInsert->item.pszText, "%s", "DEMs");
				TVInsert->item.iImage = Image[WCS_RAHOST_ICONTYPE_DEM];
				} // else if
			else
				{
				SearchType = WCS_RAHOST_OBJTYPE_CONTROLPT;
				sprintf(TVInsert->item.pszText, "%s", "Control Points");
				TVInsert->item.iImage = Image[WCS_RAHOST_ICONTYPE_CONTROLPT];
				} // else
			if (! FilterList || TaskModeFilter(SearchType))
				{
				Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER);
				Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
				Animated[0] = Enabled[0] = AnimEnabled[0] = Children[0] = 0;
				CurHost = (RasterAnimHost *)DBHost->GetFirst();
				while (CurHost)
					{
					Local = (Joe *)CurHost;
					//const char *TempViewName;
					#ifdef WCS_SCENEVIEW_SHOW_GROUPJOES
					//TempViewName = Local->GetBestName();
					if (1)
					#else // !WCS_SCENEVIEW_SHOW_GROUPJOES
					if (! Local->TestFlags(WCS_JOEFLAG_HASKIDS))
					#endif // !WCS_SCENEVIEW_SHOW_GROUPJOES
						{
						ObjLimitCount++;
						CurHost->GetRAHostProperties(Prop);
						if (Prop->TypeNumber == SearchType)
							{
							Children[0] = 1;
							if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
								{
								Animated[0] = 1;
								} // if
							if (CurHost->GetRAEnabled())
								{
								if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
									AnimEnabled[0] = 1;
								Enabled[0] = 1;
								} // if
							} // if
						} // if
					// We use Utility to store Parent TreeNode handle temporarily
					Local->Utility = NULL;
					CurHost = (RasterAnimHost *)DBHost->GetNext(Local);
					} // while
				if (! FilterList || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
					&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
					&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
					{
					TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
					if (Animated[0])
						TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
					if (TVListItemExpanded[ListNum][Group] && Children[0])
						TVInsert->item.state |= TVIS_EXPANDED;
					TVInsert->item.cChildren = Children[0];
					TVInsert->item.lParam = NULL;
					TVInsert->item.iSelectedImage = TVInsert->item.iImage;
					TVInsert->hInsertAfter = TVI_SORT;
					Parent = (void *)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
					TVListItem[ListNum][Group] = Parent; 
					} // if
				else
					TVListItem[ListNum][Group] = NULL; 
				} // if
			else
				TVListItem[ListNum][Group] = NULL; 
			} // for

		#ifdef WCS_SCENEVIEW_SHOW_GROUPJOES
		if (1) // disable sorting with group joes
		#else // !WCS_SCENEVIEW_SHOW_GROUPJOES
		if (ObjLimitCount > SortLimit)
		#endif // !WCS_SCENEVIEW_SHOW_GROUPJOES
			{ // disable sorting
			SortType = TVI_LAST;
			} // if
		else
			{
			SortType = TVI_SORT;
			} // else
		// add Joes and Layers to their respective categories
		if (! FilterList || (TaskModeFilter(WCS_RAHOST_OBJTYPE_VECTOR) || TaskModeFilter(WCS_RAHOST_OBJTYPE_DEM) || TaskModeFilter(WCS_RAHOST_OBJTYPE_CONTROLPT)))
			{
			// not display by layer
			//if (! FilterList || ! GlobalApp->MainProj->Prefs.ShowDBbyLayer)
				{
				DBHost->StaticRoot->Utility = (unsigned long int)TVListItem[ListNum][WCS_MAXIMPLEMENTED_EFFECTS];
				if (CurHost = (RasterAnimHost *)DBHost->GetFirst())
					{
					ObjCount = 0;
					while (CurHost && (ObjCount < DisplayLimit))
						{
						ObjCount++;
						Local = ((Joe *)CurHost);
						#ifdef WCS_SCENEVIEW_SHOW_GROUPJOES
						if (ObjCount < DisplayLimit)
						#else // !WCS_SCENEVIEW_SHOW_GROUPJOES
						if (! Local->TestFlags(WCS_JOEFLAG_HASKIDS))
						#endif // !WCS_SCENEVIEW_SHOW_GROUPJOES
							{
							Prop->PropMask = (WCS_RAHOST_MASKBIT_TYPENUMBER);
							CurHost->GetRAHostProperties(Prop);
							if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
								{
								#ifdef WCS_SCENEVIEW_SHOW_GROUPJOES
								//const char *TempViewName;
								//TempViewName = Local->GetBestName();
								if (Local->Parent && Local->Parent->Utility)
									{
									Parent = (void *)(Local->Parent->Utility);
									} // if
								else
									{
									Parent = TVListItem[ListNum][WCS_MAXIMPLEMENTED_EFFECTS];
									} // else
								#else // !WCS_SCENEVIEW_SHOW_GROUPJOES
								Parent = TVListItem[ListNum][WCS_MAXIMPLEMENTED_EFFECTS];
								#endif // !WCS_SCENEVIEW_SHOW_GROUPJOES
								} // if
							else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_DEM)
								Parent = TVListItem[ListNum][WCS_MAXIMPLEMENTED_EFFECTS + 1];
							else
								Parent = TVListItem[ListNum][WCS_MAXIMPLEMENTED_EFFECTS + 2];
							if (Parent)
								{
								Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE);
								Prop->FlagsMask = (unsigned long)~0;
								MyHandle = InsertRAHost(CurHost, Prop, ListID, ListNum, Parent, TVInsert, FilterList);
								#ifdef WCS_SCENEVIEW_SHOW_GROUPJOES
								if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
									{
									if (Local->TestFlags(WCS_JOEFLAG_HASKIDS))
										{
										Local->Utility = (unsigned long int)MyHandle;
										} // if
									else
										{
										Local->Utility = NULL;
										} // else
									} // if
								#endif // WCS_SCENEVIEW_SHOW_GROUPJOES
								TVInsert->hInsertAfter = SortType; //TVInsert->hInsertAfter = TVI_SORT;
								} // if
							} // if
						CurHost = (RasterAnimHost *)DBHost->GetNext(Local);
						} // while
					} // if
				} // if
			// else display by layer
			/*
			else
				{
				Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE);
				Prop->FlagsMask = (unsigned long)~0;
				// for each database type
				for (Group = WCS_MAXIMPLEMENTED_EFFECTS; Group < WCS_MAXIMPLEMENTED_EFFECTS + 3; Group ++)
					{
					// check to see if this group is displayed
					if (Parent = TVListItem[ListNum][Group])
						{
						if (Group == WCS_MAXIMPLEMENTED_EFFECTS)
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
						else if (Group == WCS_MAXIMPLEMENTED_EFFECTS + 1)
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
						else
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
						// for each layer entry in the database
						Entry = DBHost->DBLayers.FirstEntry();
						while (Entry)
							{
							if (! Entry->TestFlags(WCS_LAYER_ISATTRIBUTE))
								{
								InsertRAHost(Entry, Prop, ListID, ListNum, Parent, TVInsert, FilterList);
								TVInsert->hInsertAfter = SortType;//TVInsert->hInsertAfter = TVI_SORT;
								} // if
							Entry = DBHost->DBLayers.NextEntry(Entry);
							} // while
						} // if
					} // for
				Prop->ChildTypeFilter = 0;
				} // else show DB by layer
			*/
			TVInsert->hParent = TVI_ROOT;
			} // if display database items

		// build category headers for layers
		Group = WCS_MAXIMPLEMENTED_EFFECTS + 4;
		SearchType = WCS_SUBCLASS_LAYER;
		sprintf(TVInsert->item.pszText, "%s", "Layers");
		TVInsert->item.iImage = Image[WCS_RAHOST_ICONTYPE_LAYER];
		if (! FilterList || TaskModeFilter(SearchType))
			{
			LayerStub *CurStub;
			
			Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER);
			Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
			Prop->ChildTypeFilter = 0;
			Animated[0] = Enabled[0] = AnimEnabled[0] = Children[0] = 0;
			// walk the layer list and the items in each layer.
			// for each layer entry in the database
			Entry = DBHost->DBLayers.FirstEntry();
			while (Entry)
				{
				if (! Entry->TestFlags(WCS_LAYER_ISATTRIBUTE))
					{
					for (CurStub = Entry->FirstStub(); CurStub; CurStub = CurStub->NextObjectInLayer())
						{
						CurHost = CurStub->MyObject();
						CurHost->GetRAHostProperties(Prop);
						if (TaskModeFilter(Prop->TypeNumber))
							{
							Children[0] = 1;
							if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
								{
								Animated[0] = 1;
								} // if
							if (CurHost->GetRAEnabled())
								{
								if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
									AnimEnabled[0] = 1;
								Enabled[0] = 1;
								} // if
							} // if
						} // for
					} // if
				Entry = DBHost->DBLayers.NextEntry(Entry);
				} // while
			
			if (! FilterList || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
				&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
				&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
				{
				TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
				if (Animated[0])
					TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
				if (TVListItemExpanded[ListNum][Group] && Children[0])
					TVInsert->item.state |= TVIS_EXPANDED;
				TVInsert->item.cChildren = Children[0];
				TVInsert->item.lParam = NULL;
				TVInsert->item.iSelectedImage = TVInsert->item.iImage;
				TVInsert->hInsertAfter = TVI_SORT;
				Parent = (void *)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
				TVListItem[ListNum][Group] = Parent; 

				// for each layer entry in the database
				Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
				Prop->FlagsMask = (unsigned long)~0;
				Entry = DBHost->DBLayers.FirstEntry();
				while (Entry)
					{
					if (! Entry->TestFlags(WCS_LAYER_ISATTRIBUTE))
						{
						Prop->ChildTypeFilter = 0;
						InsertRAHost(Entry, Prop, ListID, ListNum, Parent, TVInsert, FilterList);
						TVInsert->hInsertAfter = TVI_SORT;
						} // if
					Entry = DBHost->DBLayers.NextEntry(Entry);
					} // while
				TVInsert->hParent = TVI_ROOT;
				} // if
			else
				TVListItem[ListNum][Group] = NULL; 
			} // if
		else
			TVListItem[ListNum][Group] = NULL; 

		// imagelib
		Group = WCS_MAXIMPLEMENTED_EFFECTS + 3;
		if (! FilterList || TaskModeFilter(WCS_RAHOST_OBJTYPE_RASTER))
			{
			sprintf(TVInsert->item.pszText, "%s", "Image Objects");
			TVInsert->item.cChildren = (CurHost = ImageHost->GetFirstRast()) ? 1: 0;
			Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
			Animated[0] = Enabled[0] = AnimEnabled[0] = 0;
			while (CurHost)
				{
				CurHost->GetRAHostProperties(Prop);
				if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
					{
					Animated[0] = 1;
					} // if
				if (CurHost->GetRAEnabled())
					{
					if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
						AnimEnabled[0] = 1;
					Enabled[0] = 1;
					} // if
				CurHost = ((Raster *)CurHost)->Next;
				} // while
			if (! FilterList || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
				&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
				&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
				{
				Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE);
				Prop->FlagsMask = (unsigned long)~0;
				TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
				if (Animated[0])
					TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
				if (TVListItemExpanded[ListNum][Group] && TVInsert->item.cChildren)
					TVInsert->item.state |= TVIS_EXPANDED;
				TVInsert->item.lParam = NULL;
				TVInsert->item.iImage = Image[WCS_RAHOST_ICONTYPE_RASTER];
				TVInsert->item.iSelectedImage = TVInsert->item.iImage;
				TVInsert->hInsertAfter = TVI_SORT;
				Parent = (void *)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
				TVListItem[ListNum][Group] = Parent; 
				if (CurHost = ImageHost->GetFirstRast())
					{
					while (CurHost)
						{
						InsertRAHost(CurHost, Prop, ListID, ListNum, Parent, TVInsert, FilterList);
						TVInsert->hInsertAfter = TVI_SORT;
						CurHost = ((Raster *)CurHost)->Next;
						} // while
					} // if
				TVInsert->hParent = TVI_ROOT;
				} // if
			else
				TVListItem[ListNum][Group] = NULL; 
			} // if
		else
			TVListItem[ListNum][Group] = NULL; 

		delete Prop;
		} // if
	AppMem_Free(TVInsert->item.pszText, 256);
	AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
	} // if

} // SceneViewGUI::BuildList()

/*===========================================================================*/

void *SceneViewGUI::InsertRAHost(RasterAnimHost *Insert, RasterAnimHostProperties *Prop, short ListID, short ListNum, void *Parent, void *InsertStruct, short FilterList)
{
TV_INSERTSTRUCT *TVInsert = (TV_INSERTSTRUCT *)InsertStruct;
RasterAnimHost *CurChild;
int Expanded, ExpandedOK, InsertTypeNumber, ResetChildType;

Insert->GetRAHostProperties(Prop);

InsertTypeNumber = Prop->TypeNumber;

if (FilterList && ((GlobalApp->MainProj->Prefs.EnabledFilter && ! (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED))
	|| (GlobalApp->MainProj->Prefs.AnimatedFilter && ! (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED))))
	return (NULL);

if (Prop->Name && Prop->Name[0])
	{
	strcpy(TVInsert->item.pszText, Prop->Name);
	strcat(TVInsert->item.pszText, " ");
	if (Insert->TemplateItem)
		strcat(TVInsert->item.pszText, "(Templated) ");
	if (Prop->Type)
		strcat(TVInsert->item.pszText, Prop->Type);
	} // if
else if (Prop->Type)
	{
	strcpy(TVInsert->item.pszText, Prop->Type);
	} // else if
else
	{
	strcpy(TVInsert->item.pszText, "Unnamed");
	} // else

ExpandedOK = ! HaltImperialistExpansionism(Insert, Parent, ListID);
Expanded = ((Prop->Flags & WCS_RAHOST_FLAGBIT_CHILDREN) && ((! ListNum && (Prop->Flags & WCS_RAHOST_FLAGBIT_EXPANDED1))
	|| (ListNum && (Prop->Flags & WCS_RAHOST_FLAGBIT_EXPANDED2))) && ExpandedOK);
TVInsert->hParent = (HTREEITEM)Parent;
TVInsert->item.cChildren = (Prop->Flags & WCS_RAHOST_FLAGBIT_CHILDREN) && ExpandedOK ? 1: 0;
if (InsertTypeNumber == WCS_SUBCLASS_LAYER)
	{
	if (FilterList)
		{
		if (! TaskModeFilter(WCS_RAHOST_OBJTYPE_DEM))
			Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
		else if (! TaskModeFilter(WCS_RAHOST_OBJTYPE_VECTOR))
			Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
		CurChild = NULL;
		if (! (CurChild = Insert->GetRAHostChild(CurChild, Prop->ChildTypeFilter)))
			{
			// layer filtering complicates things a bunch. Need to repeat for control points if doing the terrain task mode layer list
			if (Prop->ChildTypeFilter == WCS_RAHOST_OBJTYPE_DEM)
				{
				Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
				CurChild = NULL;
				if (! (CurChild = Insert->GetRAHostChild(CurChild, Prop->ChildTypeFilter)))
					{
					Expanded = 0;
					TVInsert->item.cChildren = 0;
					Prop->Flags &= ~WCS_RAHOST_FLAGBIT_ENABLED;
					} // if
				Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
				} // if
			else
				{
				Expanded = 0;
				TVInsert->item.cChildren = 0;
				Prop->Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
				Prop->Flags &= ~WCS_RAHOST_FLAGBIT_ENABLED;
				} // if
			} // while
		} // if
	} // if
TVInsert->item.state = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? TVIS_BOLD: 0;
if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
	TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
if (Expanded)
	TVInsert->item.state |= TVIS_EXPANDED;
TVInsert->item.iImage = Image[Prop->Flags & WCS_RAHOST_FLAGBITS_ICONTYPE];
TVInsert->item.iSelectedImage = TVInsert->item.iImage;
TVInsert->item.lParam = (long)Insert;
Parent = (void *)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
if (Expanded)
	{
	TVInsert->hInsertAfter = TVI_LAST;
	if (InsertTypeNumber == WCS_SUBCLASS_LAYER)
		ResetChildType = Prop->ChildTypeFilter;
	CurChild = NULL;
	while (CurChild = Insert->GetRAHostChild(CurChild, Prop->ChildTypeFilter))
		{
		InsertRAHost(CurChild, Prop, ListID, ListNum, Parent, InsertStruct, FilterList);
		if (InsertTypeNumber == WCS_SUBCLASS_LAYER)
			Prop->ChildTypeFilter = ResetChildType;
		} // while
	// layer filtering complicates things a bunch. Need to repeat for control points if doing the terrain task mode layer list
	if (InsertTypeNumber == WCS_SUBCLASS_LAYER && Prop->ChildTypeFilter == WCS_RAHOST_OBJTYPE_DEM)
		{
		Prop->ChildTypeFilter = ResetChildType = WCS_RAHOST_OBJTYPE_CONTROLPT;
		CurChild = NULL;
		while (CurChild = Insert->GetRAHostChild(CurChild, Prop->ChildTypeFilter))
			{
			InsertRAHost(CurChild, Prop, ListID, ListNum, Parent, InsertStruct, FilterList);
			if (InsertTypeNumber == WCS_SUBCLASS_LAYER)
				Prop->ChildTypeFilter = ResetChildType;
			} // while
		} // if
	} // if

return (Parent);

} // SceneViewGUI::InsertRAHost

/*===========================================================================*/

void SceneViewGUI::UpdateRAHost(RasterAnimHost *Insert, RasterAnimHostProperties *Prop, short ListID, short ListNum, void *Parent, void *InsertStruct, short FilterList)
{
int Expanded, ExpandedOK, OldExpanded, CurChildProcessed, TVItemProcessed;
TV_INSERTSTRUCT *TVInsert = (TV_INSERTSTRUCT *)InsertStruct;
RasterAnimHost *CurChild, *TempChild;
void *InsertAfterStash, *hItemStash, *NextItem, *NewInsertAfter;
RasterAnimHostProperties *TempProp;

Frozen = 1;

Insert->GetRAHostProperties(Prop);

if (Prop->Name && Prop->Name[0])
	{
	strcpy(TVInsert->item.pszText, Prop->Name);
	strcat(TVInsert->item.pszText, " ");
	if (Insert->TemplateItem)
		strcat(TVInsert->item.pszText, "(Templated) ");
	if (Prop->Type)
		strcat(TVInsert->item.pszText, Prop->Type);
	} // if
else if (Prop->Type)
	{
	strcpy(TVInsert->item.pszText, Prop->Type);
	} // else if
else
	{
	strcpy(TVInsert->item.pszText, "Unnamed");
	} // else


ExpandedOK = ! HaltImperialistExpansionism(Insert, Parent, ListID);
TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
OldExpanded = (TVInsert->item.state & TVIS_EXPANDED) ? 1: 0;
Expanded = ((Prop->Flags & WCS_RAHOST_FLAGBIT_CHILDREN) && OldExpanded);
TVInsert->item.cChildren = (Prop->Flags & WCS_RAHOST_FLAGBIT_CHILDREN) && ExpandedOK ? 1: 0;
TVInsert->item.state = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? TVIS_BOLD: 0;
if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
	TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
if (Expanded)
	TVInsert->item.state |= TVIS_EXPANDED;
TVInsert->item.iImage = Image[Prop->Flags & WCS_RAHOST_FLAGBITS_ICONTYPE];
TVInsert->item.iSelectedImage = TVInsert->item.iImage;
TVInsert->item.lParam = (long)Insert;
if (OldExpanded && ! Expanded)
	{
	SendDlgItemMessage(NativeWin, ListID, TVM_EXPAND, (TVE_COLLAPSE | TVE_COLLAPSERESET), (LPARAM)TVInsert->item.hItem);
	if (TempProp = new RasterAnimHostProperties())
		{
		TempProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS;
		TempProp->FlagsMask = (ListNum ? WCS_RAHOST_FLAGBIT_EXPANDED2: WCS_RAHOST_FLAGBIT_EXPANDED1);
		TempProp->Flags = 0;
		Insert->SetRAHostProperties(TempProp);
		delete TempProp;
		} // if
	} // else
SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVInsert->item);
if (Expanded)
	{
	Parent = TVInsert->item.hItem;
	TVInsert->hInsertAfter = TVI_FIRST;
	TVInsert->item.mask = (TVIF_PARAM | TVIF_HANDLE | TVIF_STATE | TVIF_CHILDREN);
	CurChild = Insert->GetRAHostChild(NULL, Prop->ChildTypeFilter);
	TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVInsert->item.hItem);
	while (CurChild || TVInsert->item.hItem)
		{
		CurChildProcessed = TVItemProcessed = 0;
		InsertAfterStash = TVInsert->hInsertAfter;
		hItemStash = TVInsert->item.hItem;
		if (TVInsert->item.hItem)
			SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVInsert->item);
		if (! TVInsert->item.hItem)
			{
			// insert CurChild
			if (NewInsertAfter = (HTREEITEM)InsertRAHost(CurChild, Prop, ListID, ListNum, Parent, InsertStruct, FilterList))
				TVInsert->hInsertAfter = (HTREEITEM)NewInsertAfter;
			TVInsert->item.hItem = (HTREEITEM)hItemStash;
			CurChildProcessed = 1;
			} // if
		else if (CurChild == (RasterAnimHost *)TVInsert->item.lParam)
			{
			// update CurChild
			UpdateRAHost(CurChild, Prop, ListID, ListNum, Parent, InsertStruct, FilterList);
			Frozen = 1;
			TVInsert->hInsertAfter = (HTREEITEM)InsertAfterStash;
			TVInsert->item.hItem = (HTREEITEM)hItemStash;
			CurChildProcessed = TVItemProcessed = 1;
			} // if
		else
			{
			if (TempChild = CurChild)
				{
				while (TempChild = Insert->GetRAHostChild(TempChild, Prop->ChildTypeFilter))
					{
					if (TempChild == (RasterAnimHost *)TVInsert->item.lParam)
						{
						// insert CurChild
						if (NewInsertAfter = (HTREEITEM)InsertRAHost(CurChild, Prop, ListID, ListNum, Parent, InsertStruct, FilterList))
							TVInsert->hInsertAfter = (HTREEITEM)NewInsertAfter;
						TVInsert->item.hItem = (HTREEITEM)hItemStash;
						CurChildProcessed = 1;
						break;
						} // if
					} // while
				} // if
			if (! CurChildProcessed)
				{
				// delete TV item
				NextItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVInsert->item.hItem);
				SendDlgItemMessage(NativeWin, ListID, TVM_DELETEITEM, 0, (LPARAM)TVInsert->item.hItem);
				TVInsert->item.hItem = (HTREEITEM)NextItem;
				} // if
			} // else
		if (CurChildProcessed)
			CurChild = Insert->GetRAHostChild(CurChild, Prop->ChildTypeFilter);
		if (TVItemProcessed)
			{
			TVInsert->hInsertAfter = TVInsert->item.hItem;
			TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVInsert->item.hItem);
			} // if
		} // while
	} // if

Frozen = 0;

} // SceneViewGUI::UpdateRAHost

/*===========================================================================*/

int SceneViewGUI::HaltImperialistExpansionism(RasterAnimHost *Insert, void *TreeItem, short ListID)
{
TV_ITEM TVItem;

TVItem.hItem = (HTREEITEM)TreeItem;
TVItem.mask = (TVIF_HANDLE | TVIF_PARAM);

while (TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)TVItem.hItem))
	{
	SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
	if (Insert == (RasterAnimHost *)TVItem.lParam)
		return (1);		// stop !, don't auto-expand this bugger or it will cause looping
	} // while

return (0);

} // SceneViewGUI::HaltImperialistExpansionism

/*===========================================================================*/

void SceneViewGUI::ExpandListItem(short ListID, short ListNum, long Something, short FilterList)
{
LPNM_TREEVIEW MyTV = (LPNM_TREEVIEW)Something;
RasterAnimHost *CurHost, *CurChild, *LastChild;
RasterAnimHostProperties *Prop;
TV_INSERTSTRUCT *TVInsert = NULL;
long NumItems = 0;

if (MyTV->itemNew.hItem)
	{
	if (CurHost = (RasterAnimHost *)MyTV->itemNew.lParam)
		{
		if (MyTV->action == TVE_EXPAND)
			{
			if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
				{
				TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
				TVInsert->item.cchTextMax = 256;
				TVInsert->hInsertAfter = TVI_LAST;

				if (Prop = new RasterAnimHostProperties())
					{
					// <<<>>> find out if it is a DB layer Entry
					Prop->PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
					CurHost->GetRAHostProperties(Prop);

					// if it is a layer find out which DB category by looking at the TV parent name
					if (Prop->TypeNumber == WCS_SUBCLASS_LAYER)
						{
						// figure out whether the list is filtered by task mode
						// set the appropriate Child filter
						if (FilterList)
							{
							if (! TaskModeFilter(WCS_RAHOST_OBJTYPE_DEM))
								Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
							else if (! TaskModeFilter(WCS_RAHOST_OBJTYPE_VECTOR))
								Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
							} // if
						/* removed 5/13/08 GRH due to changing the way layers are displayed
						TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVInsert->item.hItem);
						TVInsert->item.mask = TVIF_TEXT;
						SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVInsert->item);
						if (! stricmp(TVInsert->item.pszText, "Vectors"))
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
						else if (! stricmp(TVInsert->item.pszText, "DEMs"))
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
						else if (! stricmp(TVInsert->item.pszText, "Control Points"))
							Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
						*/
						} // if

					TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
					TVInsert->item.hItem = NULL;
					Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
					Prop->FlagsMask = (unsigned long)~0;
					CurChild = LastChild = NULL;
					while ((CurChild = CurHost->GetRAHostChild(CurChild, Prop->ChildTypeFilter)) && CurChild != LastChild && NumItems < GlobalApp->MainProj->Prefs.MaxSAGDBEntries)
						{
						NumItems ++;
						InsertRAHost(CurChild, Prop, ListID, ListNum, MyTV->itemNew.hItem, TVInsert, FilterList);
						LastChild = CurChild;
						} // while

					// DEMs and control points are displayed in the same filtered list
					// layer filtering complicates things a bunch. Need to repeat for control points if doing the terrain task mode layer list
					if (Prop->ChildTypeFilter == WCS_RAHOST_OBJTYPE_DEM)
						{
						Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
						CurChild = LastChild = NULL;
						while ((CurChild = CurHost->GetRAHostChild(CurChild, Prop->ChildTypeFilter)) && CurChild != LastChild && NumItems < GlobalApp->MainProj->Prefs.MaxSAGDBEntries)
							{
							NumItems ++;
							InsertRAHost(CurChild, Prop, ListID, ListNum, MyTV->itemNew.hItem, TVInsert, FilterList);
							LastChild = CurChild;
							} // while
						} // if

					Prop->PropMask = WCS_RAHOST_MASKBIT_FLAGS;
					Prop->FlagsMask = Prop->Flags = ListNum ? WCS_RAHOST_FLAGBIT_EXPANDED2: WCS_RAHOST_FLAGBIT_EXPANDED1;
					CurHost->SetRAHostProperties(Prop);
					delete Prop;
					} // if
				AppMem_Free(TVInsert->item.pszText, 256);
				AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
				} // if
			} // if
		} // for
	} // if

} // SceneViewGUI::ExpandListItem

/*===========================================================================*/

void SceneViewGUI::ListItemExpanded(short ListID, short ListNum, long Something)
{
LPNM_TREEVIEW MyTV = (LPNM_TREEVIEW)Something;
RasterAnimHost *CurHost;
RasterAnimHostProperties *Prop;
long Ct;

if (MyTV->itemNew.hItem)
	{
	if (CurHost = (RasterAnimHost *)MyTV->itemNew.lParam)
		{
		if (MyTV->action != TVE_EXPAND)
			{
			SendDlgItemMessage(NativeWin, ListID, TVM_EXPAND, (TVE_COLLAPSE | TVE_COLLAPSERESET), (LPARAM)MyTV->itemNew.hItem);
			if (Prop = new RasterAnimHostProperties())
				{
				Prop->PropMask = WCS_RAHOST_MASKBIT_FLAGS;
				Prop->FlagsMask = (ListNum ? WCS_RAHOST_FLAGBIT_EXPANDED2: WCS_RAHOST_FLAGBIT_EXPANDED1);
				Prop->Flags = 0;
				CurHost->SetRAHostProperties(Prop);
				delete Prop;
				} // if
			} // if
		} // for
	else
		{
		for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS + 5; Ct ++)
			{
			if (MyTV->itemNew.hItem == TVListItem[ListNum][Ct])
				{
				TVListItemExpanded[ListNum][Ct] = MyTV->action == TVE_EXPAND;
				break;
				} // if
			} // for
		} // else a root item
	} // if

} // SceneViewGUI::ListItemExpanded

/*===========================================================================*/

void SceneViewGUI::BuildListSection(int Group, void *Data)
{

BuildList(1, IDC_PARLIST1, 0);
BuildList(0, IDC_PARLIST2, 1);

} // SceneViewGUI::BuildListSection

/*===========================================================================*/

void SceneViewGUI::UpdateListSection(int Group, void *Data)
{
RasterAnimHost *CurHost, *CurChild;
RasterAnimHostProperties *Prop;
long Animated[3], Enabled[3], AnimEnabled[3], CurChildProcessed, TVItemProcessed;
int ListNum;
short ListID;
TV_INSERTSTRUCT *TVInsert = NULL;
void *ListItem[2], *hItemStash, *NextItem, *Parent;

Frozen = 1;

if ((Group >= WCS_EFFECTSSUBCLASS_GENERIC && Group < WCS_MAXIMPLEMENTED_EFFECTS)
	|| (Group >= WCS_RAHOST_OBJTYPE_VECTOR && Group <= WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION))
	{
	// find the item in both lists that corresponds with this root data class
	if (IdentifyListRootItem(Group, ListItem[0], ListItem[1]))
		{
		if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
			{
			TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
			TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
			TVInsert->item.cchTextMax = 256;
			TVInsert->hParent = TVI_ROOT;
			TVInsert->hInsertAfter = TVI_SORT;

			if (Prop = new RasterAnimHostProperties())
				{
				for (ListNum = 0, ListID = IDC_PARLIST1; ListNum < 2; ListNum ++, ListID = IDC_PARLIST2)
					{
					if (ListNum || TaskModeFilter(Group))
						{
						if (Group < WCS_MAXIMPLEMENTED_EFFECTS)
							{
							sprintf(TVInsert->item.pszText, "%s", EffectsHost->GetEffectTypeName(Group));
							TVInsert->item.cChildren = (CurHost = EffectsHost->GetListPtr(Group)) ? 1: 0;
							Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
							Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
							Animated[0] = Enabled[0] = AnimEnabled[0] = 0;
							while (CurHost)
								{
								CurHost->GetRAHostProperties(Prop);
								if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
									{
									Animated[0] = 1;
									} // if
								if (CurHost->GetRAEnabled())
									{
									if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
										AnimEnabled[0] = 1;
									Enabled[0] = 1;
									} // if
								CurHost = ((GeneralEffect *)CurHost)->Next;
								} // while
							if (ListNum || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
								&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
								&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
								{
								Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
								Prop->FlagsMask = (unsigned long)~0;
								TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
								if (Animated[0])
									TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
								if (TVListItemExpanded[ListNum][Group] && TVInsert->item.cChildren)
									TVInsert->item.state |= TVIS_EXPANDED;
								TVInsert->item.lParam = NULL;
								TVInsert->item.iImage = Image[EffectsHost->GetIconType(Group)];
								TVInsert->item.iSelectedImage = TVInsert->item.iImage;
								if (ListItem[ListNum])
									{
									TVInsert->item.hItem = (HTREEITEM)ListItem[ListNum];
									SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVInsert->item);
									} // if
								else
									{
									TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
									TVListItem[ListNum][Group] = TVInsert->item.hItem; 
									ListItem[ListNum] = TVInsert->item.hItem;
									} // else

								// update objects
								if (Parent = TVInsert->item.hItem)
									{
									CurChild = EffectsHost->GetListPtr(Group);

									while (CurChild)
										{
										TVInsert->item.hItem = (HTREEITEM)Parent;
										TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVInsert->item.hItem);
										CurChildProcessed = 0;
										while (TVInsert->item.hItem)
											{
											TVInsert->item.mask = (TVIF_PARAM | TVIF_HANDLE | TVIF_STATE | TVIF_CHILDREN);
											SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVInsert->item);
											TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
											TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
											if (TVInsert->item.lParam == (long)CurChild)
												{
												hItemStash = TVInsert->item.hItem;
												UpdateRAHost(CurChild, Prop, ListID, ListNum, TVInsert->item.hItem, TVInsert, ListNum == 0);
												Frozen = 1;
												TVInsert->item.hItem = (HTREEITEM)hItemStash;
												CurChildProcessed = 1;
												break;
												} // if
											TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVInsert->item.hItem);
											} // while
										if (! CurChildProcessed)
											{
											InsertRAHost(CurChild, Prop, ListID, ListNum, Parent, TVInsert, ListNum == 0);
											} // if
										TVInsert->hInsertAfter = TVI_SORT;
										CurChild = ((GeneralEffect *)CurChild)->Next;
										} // while

									TVInsert->item.mask = (TVIF_PARAM | TVIF_HANDLE | TVIF_STATE | TVIF_CHILDREN);
									TVInsert->item.hItem = (HTREEITEM)Parent;
									TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVInsert->item.hItem);
									while (TVInsert->item.hItem)
										{
										SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVInsert->item);
										CurChild = EffectsHost->GetListPtr(Group);
										TVItemProcessed = 0;
										while (CurChild)
											{
											if (TVInsert->item.lParam == (long)CurChild)
												{
												TVItemProcessed = 1;
												TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVInsert->item.hItem);
												break;
												} // if
											CurChild = ((GeneralEffect *)CurChild)->Next;
											} // while
										if (! TVItemProcessed)
											{
											NextItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVInsert->item.hItem);
											SendDlgItemMessage(NativeWin, ListID, TVM_DELETEITEM, 0, (LPARAM)TVInsert->item.hItem);
											TVInsert->item.hItem = (HTREEITEM)NextItem;
											} // if
										} // while
									} // if
								} // if
							else if (ListItem[ListNum])
								{
								SendDlgItemMessage(NativeWin, ListID, TVM_DELETEITEM, 0, (LPARAM)ListItem[ListNum]);
								TVListItem[ListNum][Group] = NULL;
								} // else
							} // if
						else if (Group == WCS_RAHOST_OBJTYPE_VECTOR ||
							Group == WCS_RAHOST_OBJTYPE_DEM ||
							Group == WCS_RAHOST_OBJTYPE_CONTROLPT ||
							Group == WCS_RAHOST_OBJTYPE_RASTER)
							{
							// <<<>>>gh not an effect
							// This should work until something better
							BuildList(! ListNum, ListID, ListNum);
							} // else
						} // if
					} // for
				delete Prop;
				} // if
			AppMem_Free(TVInsert->item.pszText, 256);
			AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
			} // if
		} // if
	} // if

Frozen = 0;

} // SceneViewGUI::UpdateListSection

/*===========================================================================*/

void SceneViewGUI::BuildListObject(int Group, void *Data)
{
} // SceneViewGUI::BuildListObject

/*===========================================================================*/

void SceneViewGUI::UpdateListHeader(int Group, void *Data)
{
RasterAnimHost *CurHost;
RasterAnimHostProperties *Prop;
long Animated[3], Enabled[3], AnimEnabled[3];
int ListNum;
short ListID;
TV_INSERTSTRUCT *TVInsert = NULL;
void *ListItem[2];

Frozen = 1;

if ((Group >= WCS_EFFECTSSUBCLASS_GENERIC && Group < WCS_MAXIMPLEMENTED_EFFECTS)
	|| (Group >= WCS_RAHOST_OBJTYPE_VECTOR && Group <= WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION))
	{
	// find the item in both lists that corresponds with this root data class
	if (IdentifyListRootItem(Group, ListItem[0], ListItem[1]))
		{
		if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
			{
			TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
			TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
			TVInsert->item.cchTextMax = 256;
			TVInsert->hParent = TVI_ROOT;
			TVInsert->hInsertAfter = TVI_SORT;

			if (Prop = new RasterAnimHostProperties())
				{
				for (ListNum = 0, ListID = IDC_PARLIST1; ListNum < 2; ListNum ++, ListID = IDC_PARLIST2)
					{
					if (ListNum || TaskModeFilter(Group))
						{
						if (Group < WCS_MAXIMPLEMENTED_EFFECTS)
							{
							sprintf(TVInsert->item.pszText, "%s", EffectsHost->GetEffectTypeName(Group));
							TVInsert->item.cChildren = (CurHost = EffectsHost->GetListPtr(Group)) ? 1: 0;
							Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
							Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
							Animated[0] = Enabled[0] = AnimEnabled[0] = 0;
							while (CurHost)
								{
								CurHost->GetRAHostProperties(Prop);
								if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
									{
									Animated[0] = 1;
									} // if
								if (CurHost->GetRAEnabled())
									{
									if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
										AnimEnabled[0] = 1;
									Enabled[0] = 1;
									} // if
								CurHost = ((GeneralEffect *)CurHost)->Next;
								} // while
							if (ListNum || ((Enabled[0] || ! GlobalApp->MainProj->Prefs.EnabledFilter)
								&& (Animated[0] || ! GlobalApp->MainProj->Prefs.AnimatedFilter)
								&& (AnimEnabled[0] || (! GlobalApp->MainProj->Prefs.EnabledFilter || ! GlobalApp->MainProj->Prefs.AnimatedFilter))))
								{
								Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
								Prop->FlagsMask = (unsigned long)~0;
								TVInsert->item.state = Enabled[0] ? TVIS_BOLD: 0;
								if (Animated[0])
									TVInsert->item.state |= (INDEXTOOVERLAYMASK(1));
								if (TVListItemExpanded[ListNum][Group] && TVInsert->item.cChildren)
									TVInsert->item.state |= TVIS_EXPANDED;
								TVInsert->item.lParam = NULL;
								TVInsert->item.iImage = Image[EffectsHost->GetIconType(Group)];
								TVInsert->item.iSelectedImage = TVInsert->item.iImage;
								if (ListItem[ListNum])
									{
									TVInsert->item.hItem = (HTREEITEM)ListItem[ListNum];
									SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVInsert->item);
									} // if
								else
									{
									TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_INSERTITEM, 0, (LPARAM)TVInsert);
									TVListItem[ListNum][Group] = TVInsert->item.hItem; 
									ListItem[ListNum] = TVInsert->item.hItem;
									} // else
								} // if
							else if (ListItem[ListNum])
								{
								SendDlgItemMessage(NativeWin, ListID, TVM_DELETEITEM, 0, (LPARAM)ListItem[ListNum]);
								TVListItem[ListNum][Group] = NULL;
								} // else
							} // if
						else if (Group == WCS_RAHOST_OBJTYPE_VECTOR ||
							Group == WCS_RAHOST_OBJTYPE_DEM ||
							Group == WCS_RAHOST_OBJTYPE_CONTROLPT ||
							Group == WCS_RAHOST_OBJTYPE_RASTER)
							{
							// <<<>>>gh not an Effect
							// This should work until something better
							// Actually when this is enabled group actions like enable and disable don't work 
							// because the list gets rebuilt after the first object is changed and then further 
							// attempts to get tree view siblings fails
							//BuildList(! ListNum, ListID, ListNum);
							} // else
						} // if
					} // for
				delete Prop;
				} // if
			AppMem_Free(TVInsert->item.pszText, 256);
			AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
			} // if
		} // if
	} // if

Frozen = 0;

} // SceneViewGUI::UpdateListHeader

/*===========================================================================*/

void SceneViewGUI::UpdateListObject(int Group, void *Data)
{
void *ListItem[2];
int ListNum;
TV_ITEM TVItem;
short ListID;
RasterAnimHostProperties *Prop;
TV_INSERTSTRUCT *TVInsert = NULL;

// check if Data is valid
if (Data)
	{
	if ((Group >= WCS_EFFECTSSUBCLASS_GENERIC && Group < WCS_MAXIMPLEMENTED_EFFECTS)
		|| (Group >= WCS_RAHOST_OBJTYPE_VECTOR && Group <= WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION))
		{
		if (Group >= WCS_EFFECTSSUBCLASS_GENERIC && Group < WCS_MAXIMPLEMENTED_EFFECTS)
			{
			// bad news to try and update an object while it is being destroyed
			if (! EffectsHost->IsEffectValid((GeneralEffect *)Data, Group, 0))
				return;
			} // if
		if (Group >= WCS_RAHOST_OBJTYPE_VECTOR && Group <= WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			// bad news to try and update an object while it is being destroyed
			if (! DBHost->ValidateJoe((RasterAnimHost *)Data))
				return;
			} // if
		// find the item in both lists that corresponds with this root data class
		if (IdentifyListRootItem(Group, ListItem[0], ListItem[1]))
			{
			if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
				{
				TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
				TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
				TVInsert->item.cchTextMax = 256;
				TVInsert->hParent = TVI_ROOT;
				TVInsert->hInsertAfter = TVI_SORT;

				if (Prop = new RasterAnimHostProperties())
					{
					if (Group == WCS_RAHOST_OBJTYPE_VECTOR)
						Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
					else if (Group == WCS_RAHOST_OBJTYPE_DEM)
						Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
					else if (Group == WCS_RAHOST_OBJTYPE_CONTROLPT)
						Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
					for (ListNum = 0, ListID = IDC_PARLIST1; ListNum < 2; ListNum ++, ListID = IDC_PARLIST2)
						{
						if (ListItem[ListNum])
							{
							bool FoundItem = false;
							// get children of ListItem, examine each one for its user value to match Data
							TVItem.mask = (TVIF_PARAM | TVIF_HANDLE | TVIF_STATE | TVIF_CHILDREN);
							TVItem.hItem = (HTREEITEM)ListItem[ListNum];
							TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVItem.hItem);
							while (TVItem.hItem)
								{
								SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
								if (TVItem.lParam == (long)Data)
									{
									// process this item
									TVInsert->item.state = TVItem.state;
									TVInsert->item.hItem = TVItem.hItem;
									Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
									Prop->FlagsMask = (unsigned long)~0;
									UpdateRAHost((RasterAnimHost *)Data, Prop, ListID, ListNum, TVItem.hItem, TVInsert, ListNum == 1);
									FoundItem = true;
									break;
									} // if
								TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVItem.hItem);
								} // if
							if (! FoundItem)
								{
								Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
								Prop->FlagsMask = (unsigned long)~0;
								InsertRAHost((RasterAnimHost *)Data, Prop, ListID, ListNum, ListItem[ListNum], TVInsert, ListNum == 1);
								} // if
							} // if
						} // for
					delete Prop;
					} // if
				AppMem_Free(TVInsert->item.pszText, 256);
				AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
				} // if
			} // if
		} // if
	} // if

} // SceneViewGUI::UpdateListObject

/*===========================================================================*/

void SceneViewGUI::FindAndUpdateAllOtherInstances(int Group, void *Data)
{
int ListNum, Ct;
short ListID;
RasterAnimHostProperties *Prop;
TV_INSERTSTRUCT *TVInsert = NULL;

if (Data)
	{
	if ((Group >= WCS_EFFECTSSUBCLASS_GENERIC && Group < WCS_MAXIMPLEMENTED_EFFECTS)
		|| (Group >= WCS_RAHOST_OBJTYPE_VECTOR && Group <= WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION))
		{
		if (TVInsert = (TV_INSERTSTRUCT *)AppMem_Alloc(sizeof (TV_INSERTSTRUCT), APPMEM_CLEAR))
			{
			TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
			TVInsert->item.pszText = (char *)AppMem_Alloc(256, APPMEM_CLEAR);
			TVInsert->item.cchTextMax = 256;
			TVInsert->hParent = TVI_ROOT;
			TVInsert->hInsertAfter = TVI_SORT;

			if (Prop = new RasterAnimHostProperties())
				{
				if (Group == WCS_RAHOST_OBJTYPE_VECTOR)
					Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_VECTOR;
				else if (Group == WCS_RAHOST_OBJTYPE_DEM)
					Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_DEM;
				else if (Group == WCS_RAHOST_OBJTYPE_CONTROLPT)
					Prop->ChildTypeFilter = WCS_RAHOST_OBJTYPE_CONTROLPT;
				for (ListNum = 0, ListID = IDC_PARLIST1; ListNum < 2; ListNum ++, ListID = IDC_PARLIST2)
					{
					for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS + 5; Ct ++)
						{
						if (TVListItem[ListNum][Ct] && Ct != Group)
							{
							RecurseFindAndUpdate(Data, Prop, ListID, ListNum, TVListItem[ListNum][Ct], TVInsert);
							} // if
						} // for
					} // for
				delete Prop;
				} // if
			AppMem_Free(TVInsert->item.pszText, 256);
			AppMem_Free(TVInsert, sizeof (TV_INSERTSTRUCT));
			} // if
		} // if
	} // if

} // SceneViewGUI::FindAndUpdateAllOtherInstances

/*===========================================================================*/

void SceneViewGUI::RecurseFindAndUpdate(void *Data, RasterAnimHostProperties *Prop, short ListID, short ListNum, void *hItem, void *InsertStruct)
{
TV_INSERTSTRUCT *TVInsert = (TV_INSERTSTRUCT *)InsertStruct;

if (TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem))
	{
	while (hItem = TVInsert->item.hItem)
		{
		TVInsert->item.mask = TVIF_PARAM | TVIF_CHILDREN;
		SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVInsert->item);
		TVInsert->item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		TVInsert->item.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_OVERLAYMASK;
		if (TVInsert->item.lParam == (long)Data)
			{
			// process update
			Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER);
			Prop->FlagsMask = (unsigned long)~0;
			UpdateRAHost((RasterAnimHost *)Data, Prop, ListID, ListNum, TVInsert->item.hItem, TVInsert, ListNum == 1);
			} // if
		else if (TVInsert->item.cChildren)
			{
			// search children
			RecurseFindAndUpdate(Data, Prop, ListID, ListNum, TVInsert->item.hItem, TVInsert);
			} // else
		TVInsert->item.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
		} // while
	} // if

} // SceneViewGUI::RecurseFindAndUpdate

/*===========================================================================*/

void SceneViewGUI::ProcessNotifyEvent(int Action, int Group, void *Data)
{

switch (Action)
	{
	case WCS_SCENEVIEW_LISTACTION_BUILDLIST:
		{
		BuildList(1, IDC_PARLIST1, 0);
		BuildList(0, IDC_PARLIST2, 1);
		break;
		} // Build entire list
	case WCS_SCENEVIEW_LISTACTION_BUILDSECTION:
		{
		BuildListSection(Group, Data);
		break;
		} // Build entire category of objects
	case WCS_SCENEVIEW_LISTACTION_UPDATESECTION:
		{
		UpdateListSection(Group, Data);
		FindAndUpdateAllOtherInstances(Group, Data);
		break;
		} // just update a single category
	case WCS_SCENEVIEW_LISTACTION_BUILDOBJECT:
		{
		BuildListObject(Group, Data);
		break;
		} // add or replace an entire object
	case WCS_SCENEVIEW_LISTACTION_UPDATEOBJPLUSHDR:
		{
		UpdateListHeader(Group, Data);
		} // update a single object and its children
		//lint -fallthrough
	case WCS_SCENEVIEW_LISTACTION_UPDATEOBJECT:
		{
		UpdateListObject(Group, Data);
		FindAndUpdateAllOtherInstances(Group, Data);
		break;
		} // just update a single object's children
	} // switch

} // SceneViewGUI::ProcessNotifyEvent

/*===========================================================================*/

int SceneViewGUI::IdentifyListRootItem(unsigned char Class, void *&Item1, void *&Item2)
{

if (Class < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	Item1 = TVListItem[0][Class];
	Item2 = TVListItem[1][Class];
	} // if
else if (Class == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Item1 = TVListItem[0][WCS_MAXIMPLEMENTED_EFFECTS];
	Item2 = TVListItem[1][WCS_MAXIMPLEMENTED_EFFECTS];
	} // else if
else if (Class == WCS_RAHOST_OBJTYPE_DEM)
	{
	Item1 = TVListItem[0][WCS_MAXIMPLEMENTED_EFFECTS + 1];
	Item2 = TVListItem[1][WCS_MAXIMPLEMENTED_EFFECTS + 1];
	} // else if
else if (Class == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{
	Item1 = TVListItem[0][WCS_MAXIMPLEMENTED_EFFECTS + 2];
	Item2 = TVListItem[1][WCS_MAXIMPLEMENTED_EFFECTS + 2];
	} // else if
else if (Class == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Item1 = TVListItem[0][WCS_MAXIMPLEMENTED_EFFECTS + 3];
	Item2 = TVListItem[1][WCS_MAXIMPLEMENTED_EFFECTS + 3];
	} // else if

if (Item1 || Item2)
	return (1);

return (0);

} // SceneViewGUI::IdentifyListRootItem

/*===========================================================================*/

void SceneViewGUI::BeginDrag(short ListID, long Something)
{
LPNM_TREEVIEW MyTV = (LPNM_TREEVIEW)Something;
POINT ConvertCoord;

if (MyTV->itemNew.hItem && (DragHost = (RasterAnimHost *)MyTV->itemNew.lParam))
	{
	if (DragProp || (DragProp = new RasterAnimHostProperties()))
		{
		DragProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		DragProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		DragHost->GetRAHostProperties(DragProp);
		if (DragProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			DragList = GetDlgItem(NativeWin, ListID);
			DragItem = MyTV->itemNew.hItem;
			SendMessage(DragList, TVM_SELECTITEM, TVGN_CARET, (LPARAM)DragItem);
			TVDragList = (HIMAGELIST)SendMessage(DragList, TVM_CREATEDRAGIMAGE, 0, (LPARAM)DragItem);
			ImageList_BeginDrag(TVDragList, 0, 0, -12);
	//		ShowCursor(FALSE);
			GetWindowRect(DragList, &TVDragRect);
			ConvertCoord.x = TVDragRect.left;
			ConvertCoord.y = TVDragRect.top;
			ScreenToClient(GetParent(DragList), &ConvertCoord);
			SetCapture(GetParent(DragList));
			ImageList_DragEnter(GetParent(DragList), MyTV->ptDrag.x + ConvertCoord.x, MyTV->ptDrag.y + ConvertCoord.y);
			Dragging = TRUE;
			HitList = NULL;
			HitItem = NULL;
			} // if daggable
		else
			DragHost = NULL;
		} // if
	else
		DragHost = NULL;
	} // if

} // SceneViewGUI::BeginDrag

/*===========================================================================*/

void SceneViewGUI::DoDragItem(long X, long Y)
{
HTREEITEM hitTarget;
TV_HITTESTINFO tvht;
HWND ItemList;
POINT ConvertCoord;
long Ct;
TV_ITEM TVItem;

ImageList_DragMove(X, Y);

for (ItemList = GetDlgItem(NativeWin, IDC_PARLIST1), Ct = 0; Ct < 2; Ct ++, ItemList = SubPanels[0][WCS_SCENEVIEWGUI_TAB_SAG])
	{
	GetWindowRect(ItemList, &TVDragRect);
	ConvertCoord.x = TVDragRect.left;
	ConvertCoord.y = TVDragRect.top;
	ScreenToClient(GetParent(ItemList), &ConvertCoord);
	tvht.pt.x = X - ConvertCoord.x;
	tvht.pt.y = Y - ConvertCoord.y;
	if (((hitTarget = TreeView_HitTest(ItemList, &tvht)) != NULL))
		{
		if (hitTarget != DragItem)
			{
			if (hitTarget != HitItem)
				{
				TVItem.lParam = 0;
				TVItem.mask = (TVIF_PARAM | TVIF_HANDLE);
				TVItem.hItem = hitTarget;
				SendMessage(ItemList, TVM_GETITEM, 0, (LPARAM)&TVItem);
				if ((HitHost = (RasterAnimHost *)TVItem.lParam) && (HitHost != DragHost))
					{
					DragProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
					HitHost->GetRAHostProperties(DragProp);
					if (DragProp->DropOK)
						{
		//				TreeView_SelectDropTarget(ItemList, hitTarget);
						HitList = ItemList;
						HitItem = hitTarget;
						SetCursor(StdCursor);
						} // if
					else
						{
						HitList = NULL;
						HitItem = NULL;
						HitHost = NULL;
						} // else
					} // if
				else
					{
					HitList = NULL;
					HitItem = NULL;
					HitHost = NULL;
					} // else
				} // if
			} // if
		else
			{
			HitList = NULL;
			HitItem = NULL;
			HitHost = NULL;
			} // else
		break;
		} // if
	else
		{
		HitList = NULL;
		HitItem = NULL;
		HitHost = NULL;
		} // else
	} // else

if (! HitItem)
	{
	if (GetCursor() != NoDropCursor)
		SetCursor(NoDropCursor);
	} // if

} // SceneViewGUI::DoDragItem

/*===========================================================================*/

void SceneViewGUI::EndDrag(void)
{
int Result;

SetCursor(StdCursor);
ImageList_EndDrag();
ImageList_DragLeave(GetParent(GetDlgItem(NativeWin, IDC_PARLIST1)));
ReleaseCapture();
//ShowCursor(TRUE);
Dragging = FALSE;
if (HitList && HitItem && DragHost && HitHost)
	{
	SendMessage(HitList, TVM_SELECTITEM, TVGN_CARET, (LPARAM)HitItem);

	DragProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
	DragProp->DropSource = DragHost;
	// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
	//  eg. still in progress through a DragnDropListGUI
	Result = HitHost->SetRAHostProperties(DragProp);
	} // if
else
	{
	SendMessage(DragList, TVM_SELECTITEM, TVGN_CARET, (LPARAM)DragItem);
	} // else

HitItem = NULL;
HitList = NULL;
HitHost = NULL;
DragList = NULL;
DragItem = NULL;
DragHost = NULL;

} // SceneViewGUI::EndDrag

/*===========================================================================*/

void SceneViewGUI::ChangeSelection(short ListID, long Something)
{
LPNM_TREEVIEW MyTV = (LPNM_TREEVIEW)Something;
//TV_ITEM TVItem;
RasterAnimHostProperties Prop;

if (MyTV->itemNew.hItem && MyTV->itemNew.hItem != MyTV->itemOld.hItem)
	{
	//TVItem.mask = (TVIF_STATE | TVIF_HANDLE);
	//TVItem.hItem = MyTV->itemNew.hItem;
	//TVItem.stateMask = (TVIS_SELECTED | TVIS_FOCUSED);
	//TVItem.state = (TVIS_SELECTED | TVIS_FOCUSED);
	//SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVItem);
//	if (ListID == IDC_PARLIST1)
		{
		if (MyTV->itemNew.lParam)
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DELETABLE;
			((RasterAnimHost *)MyTV->itemNew.lParam)->GetRAHostProperties(&Prop);
			WidgetSetDisabled(IDC_ENABLE, ! (Prop.Flags & WCS_RAHOST_FLAGBIT_DELETABLE));
			WidgetSetDisabled(IDC_DISABLE, ! (Prop.Flags & WCS_RAHOST_FLAGBIT_DELETABLE));
			WidgetSetDisabled(IDC_REMOVE, ! (Prop.Flags & WCS_RAHOST_FLAGBIT_DELETABLE));
			WidgetSetDisabled(IDC_ADD, Prop.TypeNumber >= WCS_MAXIMPLEMENTED_EFFECTS
				&& ((RasterAnimHost *)MyTV->itemNew.lParam)->RAParent);
			RasterAnimHost::SetActiveRAHost((RasterAnimHost *)MyTV->itemNew.lParam, 1);
			} // if
		else
			{
			WidgetSetDisabled(IDC_ENABLE, FALSE);
			WidgetSetDisabled(IDC_DISABLE, FALSE);
			WidgetSetDisabled(IDC_REMOVE, FALSE);
			WidgetSetDisabled(IDC_ADD, FALSE);
			RasterAnimHost::SetActiveRAHost(NULL, 1);
			} // else
		} // if
	} // if

} // SceneViewGUI::ChangeSelection

/*===========================================================================*/

int SceneViewGUI::BeginLabelEdit(short ListID, long Something)
{
TV_DISPINFO *MyTV = (TV_DISPINFO *)Something;

if (MyTV->item.hItem && (DragHost = (RasterAnimHost *)MyTV->item.lParam))
	{
	if (DragProp || (DragProp = new RasterAnimHostProperties()))
		{
		DragProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME;
		DragProp->FlagsMask = WCS_RAHOST_FLAGBIT_EDITNAME;
		DragHost->GetRAHostProperties(DragProp);
		if (DragProp->Flags & WCS_RAHOST_FLAGBIT_EDITNAME)
			{
			Editing = 1;
			DragHost = NULL;
			return (0);	// allows editing
			} // if daggable
		} // if
	} // if
SendDlgItemMessage(NativeWin, ListID, TVM_ENDEDITLABELNOW, 1, 0);

DragHost = NULL;
return (1);	// cancels editing

} // SceneViewGUI::BeginLabelEdit

/*===========================================================================*/

void SceneViewGUI::EndLabelEdit(short ListID, long Something)
{
TV_DISPINFO *MyTV = (TV_DISPINFO *)Something;
TV_ITEM TVItem;
long Ct = 0;
char BuildName[256];

// pszText should be 0 if user cancelled editing, no need to do anything
if (Editing && MyTV->item.hItem && MyTV->item.pszText && (DragHost = (RasterAnimHost *)MyTV->item.lParam))
	{
	if (DragProp || (DragProp = new RasterAnimHostProperties()))
		{
		DragProp->PropMask = WCS_RAHOST_MASKBIT_NAME;
		DragProp->FlagsMask = 0;
		strcpy(BuildName, MyTV->item.pszText);
		DragProp->Name = BuildName;
		while (BuildName[Ct] && BuildName[Ct] != '(')
			++Ct;
		if (Ct > 0 && BuildName[Ct])
			BuildName[Ct - 1] = 0;
		DragHost->SetRAHostProperties(DragProp);
		DragProp->PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
		DragHost->GetRAHostProperties(DragProp);
		if (DragProp->Name && DragProp->Name[0])
			{
			strcpy(BuildName, DragProp->Name);
			strcat(BuildName, " ");
			if (DragProp->Type)
				strcat(BuildName, DragProp->Type);
			} // if
		else if (DragProp->Type)
			{
			strcpy(BuildName, DragProp->Type);
			} // else if
		else
			{
			strcpy(BuildName, "Unnamed");
			} // else
		TVItem.mask = TVIF_HANDLE | TVIF_TEXT;
		TVItem.pszText = BuildName;
		TVItem.hItem = MyTV->item.hItem;
		SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVItem);
		} // if
	} // if

DragHost = NULL;
Editing = 0;

} // SceneViewGUI::EndLabelEdit

/*===========================================================================*/

void SceneViewGUI::DoWindowExpand(NativeGUIWin NW)
{
RECT Wreck;

if (WidgetGetCheck(IDC_EXPAND))
	{
	AutoResize = 1;
	//SetWinManFlags(WCS_FENETRE_WINMAN_ONTOP); // removed 030608 CXH
	SizeFen(ExpandedX, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
	} // if
else
	{
	AutoResize = 1;
	//ClearWinManFlags(WCS_FENETRE_WINMAN_ONTOP); // removed 030608 CXH
	SizeFen(SVGStockX, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
	} // else

GetClientRect(NativeWin, &Wreck);
HandleReSized(0, Wreck.right, Wreck.bottom);


} // SceneViewGUI::DoWindowExpand

/*===========================================================================*/

void SceneViewGUI::HandleMatrixResize(void)
{
RECT Wreck;

GetWindowRect(NativeWin, &Wreck);
Wreck.right  -= Wreck.left;
Wreck.bottom -= Wreck.top;
// This no longer causes HandleReSized()
SystemResize = 1;
MoveAndSizeFen(0, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN),
 (unsigned short)Wreck.right, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
GetClientRect(NativeWin, &Wreck);
HandleReSized(0, Wreck.right, Wreck.bottom);
SystemResize = 0;

} // SceneViewGUI::HandleMatrixResize

/*===========================================================================*/

long SceneViewGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT ListRect1;
POINT TransUL;
NativeControl ListControl1, ListControl2, Splitter, TabControl, Diag, Point; //, Container;
int TopBorder, Width, TotHeight, Height1, MidMargin = 5, Margin = 1;
long BotHeight = 200;
long NewWinWide;

NewWinWide = NewWidth;
SetWindowPos(NativeWin, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
if (!SystemResize)
	{
	if (! AutoResize)
		{
		ExpandedX = NewWinWide;
		GlobalApp->MainProj->Prefs.SAGExpanded = ExpandedX - SVGStockX;
		} // if
	AutoResize = 0;
	} // if
	
// update Expanded button to reflect if we're expanded or not
RECT CurrentTotalWindowDims;
GetWindowRect(NativeWin, &CurrentTotalWindowDims);
WidgetSetCheck(IDC_EXPAND, ((CurrentTotalWindowDims.right - CurrentTotalWindowDims.left) != SVGStockX) ? 1 : 0);

if (ListControl1 = GetWidgetFromID(IDC_PARLIST1))
	{
	// Because our border visually appears to be part of our
	// margin/spacing, we need to take it into account...
	ListControl2 = SubPanels[0][WCS_SCENEVIEWGUI_TAB_SAG];
	Diag = SubPanels[0][WCS_SCENEVIEWGUI_TAB_DIAG];
	Point = SubPanels[0][WCS_SCENEVIEWGUI_TAB_POINT];
	Splitter = GetWidgetFromID(IDC_SCENESPLIT);
	TabControl = GetWidgetFromID(IDC_TAB1);

	GetWindowRect(ListControl1, &ListRect1);

	TransUL.x = ListRect1.left;
	TransUL.y = ListRect1.top;
	ScreenToClient(NativeWin, &TransUL);
	TopBorder = TransUL.y;
	TotHeight = NewHeight - (TopBorder + Margin);
	Width = NewWidth - (Margin + Margin);
	if (BotHeight + BotHeight > TotHeight) BotHeight = TotHeight / 2;
	Height1 = TotHeight - (BotHeight + 5);

	SetWindowPos(ListControl1, NULL, Margin, TopBorder, Width, Height1, SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(ListControl2, NULL, 0, 0, Width - 6, BotHeight - 6, SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(Diag, NULL, 0, 0, Width - 6, BotHeight - 6, SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(Point, NULL, 0, 0, Width - 6, BotHeight - 6, SWP_NOZORDER|SWP_NOMOVE);
	SetWindowPos(TabControl, NULL, Margin, TopBorder + Height1 + MidMargin, Width, BotHeight, SWP_NOZORDER);
	SetWindowPos(Splitter, NULL, 0, 0, Width, Height1, SWP_NOZORDER|SWP_NOMOVE);
	SetSAGBottomHtPct(GlobalApp->MainProj->Prefs.SAGBottomHtPct);
	} // if

return(0);
} // SceneViewGUI::HandleReSized

/*===========================================================================*/

long SceneViewGUI::HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{

if (Left)
	{
	} // if

return(0);
} // SceneViewGUI::HandleMouseMove


/*===========================================================================*/

void SceneViewGUI::EditObject(short WidID, RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost;

ListID = GetDlgItem(NativeWin, WidID);

if (TreeItem)
	{
	TVItem.hItem = (HTREEITEM)TreeItem;
	} // if
else
	{
	TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
	} // else

if (TVItem.hItem)
	{
	TVItem.lParam = NULL;
	TVItem.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_CHILDREN;
	SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
	if (EditTarget)
		{
		EditHost = EditTarget;
		} // if
	else
		{
		EditHost = (RasterAnimHost *)TVItem.lParam;
		} // else
	if (EditHost)
		{
		EditHost->EditRAHost();
		RasterAnimHost::SetActiveRAHost(EditHost);
		} // if
	else
		{
		if (TVItem.cChildren)
			{
			if (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVItem.hItem))
				{
				TVItem.lParam = NULL;
				SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
				if (EditHost = (RasterAnimHost *)TVItem.lParam)
					{
					EditHost->EditRAHost();
					RasterAnimHost::SetActiveRAHost(EditHost);
					} // if
				} // if
			} // if
		else
			AddObject(WidID, (long)TVItem.hItem, 0);
		} // else
	} // if

} // SceneViewGUI::EditObject

/*===========================================================================*/

void SceneViewGUI::EnableObject(RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost;
RasterAnimHostProperties *Prop;

ListID = GetDlgItem(NativeWin, IDC_PARLIST1);

if (TreeItem)
	{
	TVItem.hItem = (HTREEITEM)TreeItem;
	} // if
else
	{
	TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
	} // else

if (TVItem.hItem)
	{
	if (Prop = new RasterAnimHostProperties())
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			EditHost = EditTarget;
			} // if
		else
			{
			EditHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (EditHost)
			{
			Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop->Flags = WCS_RAHOST_FLAGBIT_ENABLED;
			EditHost->SetRAHostProperties(Prop);
			} // if
		else
			{
			if (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVItem.hItem))
				{
				while (TVItem.hItem)
					{
					SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
					if (EditHost = (RasterAnimHost *)TVItem.lParam)
						{
						Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
						Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
						Prop->Flags = WCS_RAHOST_FLAGBIT_ENABLED;
						EditHost->SetRAHostProperties(Prop);
						} // if
					TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVItem.hItem);
					} // while
				} // if
			} // else must be a heading
		delete Prop;
		} // if
	} // if

} // SceneViewGUI::EnableObject

/*===========================================================================*/

void SceneViewGUI::DisableObject(RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost;
RasterAnimHostProperties *Prop;

ListID = GetDlgItem(NativeWin, IDC_PARLIST1);

if (TreeItem)
	{
	TVItem.hItem = (HTREEITEM)TreeItem;
	} // if
else
	{
	TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
	} // else

if (TVItem.hItem)
	{
	if (Prop = new RasterAnimHostProperties())
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			EditHost = EditTarget;
			} // if
		else
			{
			EditHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (EditHost)
			{
			Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop->Flags = 0;
			EditHost->SetRAHostProperties(Prop);
			} // if
		else
			{
			if (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)TVItem.hItem))
				{
				while (TVItem.hItem)
					{
					SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
					if (EditHost = (RasterAnimHost *)TVItem.lParam)
						{
						Prop->PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
						Prop->FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
						Prop->Flags = 0;
						EditHost->SetRAHostProperties(Prop);
						} // if
					TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)TVItem.hItem);
					} // while
				} // if
			} // else must be a heading
		delete Prop;
		} // if
	} // if

} // SceneViewGUI::DisableObject

/*===========================================================================*/

void SceneViewGUI::EmbedObject(RasterAnimHost *EditTarget, unsigned long TreeItem)
{

if (EditTarget)
	{
	EditTarget->Embed();
	} // if

} // SceneViewGUI::EmbedObject

/*===========================================================================*/

RasterAnimHost *SceneViewGUI::AddObject(short WidID, long Item, int OpenGallery, RasterAnimHost *EditTarget)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost, *RootHost, *CurHost;
RasterAnimHostProperties *Prop, *TempProp;
GeneralEffect *NewEffect;
Joe *NewJoe;
Raster *NewImage;
char NameStr[WCS_EFFECT_MAXNAMELENGTH + 10];
NotifyTag Changes[2];

ListID = GetDlgItem(NativeWin, WidID);

if ((TVItem.hItem = (HTREEITEM)Item) || (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0)))
	{
	if (Prop = new RasterAnimHostProperties())
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			EditHost = EditTarget;
			} // if
		else
			{
			EditHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (EditHost)
			{
			Prop->PropMask = (WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_NAME);
			Prop->TypeNumber = 0;
			EditHost->GetRAHostProperties(Prop);
			if (Prop->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
				{
				strcpy(NameStr, Prop->Name);
				if (! Prop->Name[0] && (RootHost = EditHost->GetRAHostRoot()))
					{
					if (TempProp = new RasterAnimHostProperties())
						{
						TempProp->PropMask = (WCS_RAHOST_MASKBIT_NAME);
						RootHost->GetRAHostProperties(TempProp);
						strcpy(NameStr, TempProp->Name);
						delete TempProp;
						} // if
					} // else
				if (NewEffect = GlobalApp->AppEffects->AddEffect(Prop->TypeNumber, NameStr[0] ? NameStr: NULL, (GeneralEffect *)EditHost))
					{
					NewEffect->EditRAHost();
					RasterAnimHost::SetActiveRAHost(NewEffect);
					delete Prop;
					return (NewEffect);
					} // if
				} // if effect
			else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR || Prop->TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				if (NewJoe = DBHost->NewObject(ProjHost, (char *)((Joe *)EditHost)->GetBestName()))
					{
					NewJoe->CopyPoints(NewJoe, (Joe *)EditHost, 1, 1);
					NewJoe->ClearFlags(NewJoe->GetFlags());
					NewJoe->SetFlags(((Joe *)EditHost)->GetFlags()  & (WCS_JOEFLAG_ISCONTROL | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED));
					// add effect attributes
					if (TempProp = new RasterAnimHostProperties())
						{
						CurHost = NULL; 
						while (CurHost = EditHost->GetRAHostChild(CurHost, 0))
							{
							TempProp->PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
							CurHost->GetRAHostProperties(TempProp);
							if (TempProp->TypeNumber >= WCS_JOE_ATTRIB_INTERNAL_LAKE && TempProp->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
								NewJoe->AddEffect((GeneralEffect *)CurHost, -1);
							} // while 
						delete TempProp;
						} // if
					// needs to be done after points added for undo functions
					Changes[0] = MAKE_ID(NewJoe->GetNotifyClass(), NewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NewJoe);
					DBHost->SetActiveObj(NewJoe);	// calls RasterAnimHost::SetActiveRAHost
					//RasterAnimHost::SetActiveRAHost(NewJoe);
					delete Prop;
					return (NewJoe);
					} // if
				} // else if vector
			else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_DEM)
				{
				} // else if dem
			else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
				{
				} // else if raster
			} // if clone an object
		else
			{
			for (Prop->TypeNumber = WCS_EFFECTSSUBCLASS_LAKE; Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS + 5; Prop->TypeNumber ++)
				{
				if (TVListItem[WidID == IDC_PARLIST2 ? 1: 0][Prop->TypeNumber] == TVItem.hItem)
					{
					if (Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
						{
						if (Prop->TypeNumber == WCS_EFFECTSSUBCLASS_DEMMERGER)
							{
							int Result;
							Result = UserMessageYNCAN("Create DEM Merger", "Would you like to use the DEM Merger Wizard?", 0, REQUESTER_ICON_QUESTION);
							if (!Result)
								return(NULL);
							else if (Result == 2) // yes
								{
								GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_MWZ, 0);
								return(NULL);
								} // else if
							// else will fall through to if (NewEffect... code below
							} // if
						else if (Prop->TypeNumber == WCS_EFFECTSSUBCLASS_GRIDDER)
							{
							int Result;
							Result = UserMessageYNCAN("Create Terrain Gridder", "Would you like to use the Terrain Gridder Wizard?", 0, REQUESTER_ICON_QUESTION);
							if (!Result)
								return(NULL);
							else if (Result == 2) // yes
								{
								GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_GWZ, 0);
								return(NULL);
								} // else if
							// else will fall through to if (NewEffect... code below
							} // else if
						if (NewEffect = GlobalApp->AppEffects->AddEffect(Prop->TypeNumber, NULL, NULL))
							{
							NewEffect->EditRAHost();
							if (OpenGallery)
								NewEffect->OpenGallery(EffectsHost);
							RasterAnimHost::SetActiveRAHost(NewEffect);
							delete Prop;
							return (NewEffect);
							} // if
						} // if effect
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS)
						{
						if (NewJoe = DBHost->NewObject(NULL))
							{
							NewJoe->ClearFlags(WCS_JOEFLAG_ISCONTROL);
							Changes[0] = MAKE_ID(NewJoe->GetNotifyClass(), NewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, NewJoe);
							RasterAnimHost::SetActiveRAHost(NewJoe);
							delete Prop;
							return (NewJoe);
							} // if
						} // else if vector
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 1)
						{
						} // else if dem
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 2)
						{
						if (NewJoe = DBHost->NewObject(NULL))
							{
							NewJoe->SetFlags(WCS_JOEFLAG_ISCONTROL);
							Changes[0] = MAKE_ID(NewJoe->GetNotifyClass(), NewJoe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
							Changes[1] = NULL;
							GlobalApp->AppEx->GenerateNotify(Changes, NewJoe);
							RasterAnimHost::SetActiveRAHost(NewJoe);
							delete Prop;
							return (NewJoe);
							} // if
						} // else if control pt
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 3)
						{
						if (NewImage = ImageHost->AddRequestRaster())
							{
							NewImage->EditRAHost();
							RasterAnimHost::SetActiveRAHost(NewImage);
							delete Prop;
							return (NewImage);
							} // if
						} // else if raster
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 4)
						{
						NameStr[0] = 0;

						if (GetInputString("Enter new Layer Name", "", NameStr))
							{
							if (NameStr[0])
								{
								DBHost->DBLayers.MatchMakeLayer(NameStr, 0);
								} // if
							} // if
						} // else if layer
					break;
					} // if
				} // for
			} // else
		delete Prop;
		} // if
	} // if

return (NULL);

} // SceneViewGUI::AddObject

/*===========================================================================*/

void SceneViewGUI::RemoveObject(short WidgetID, RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *RemoveHost, *ParentHost = NULL;
RasterAnimHostProperties *Prop, *ParentProp;
char RemoveText[256];
int RemoveAll = 1, DoRemoveAll = 0;

ListID = GetDlgItem(NativeWin, WidgetID);

if (TreeItem)
	{
	TVItem.hItem = (HTREEITEM)TreeItem;
	} // if
else
	{
	TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
	} // else

if (TVItem.hItem)
	{
	NotifyTag Changes[3];
	
	// Lock S@G (and other UI) while loading complex stuff
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);

	if ((Prop = new RasterAnimHostProperties()) && (ParentProp = new RasterAnimHostProperties()))
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			RemoveHost = EditTarget;
			} // if
		else
			{
			RemoveHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (RemoveHost)
			{
			Prop->PropMask = (WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_FLAGS);
			Prop->FlagsMask = WCS_RAHOST_FLAGBIT_DELETABLE;
			Prop->TypeNumber = 0;
			RemoveHost->GetRAHostProperties(Prop);
			if (Prop->Flags & WCS_RAHOST_FLAGBIT_DELETABLE)
				{
				if (RemoveHost->RAParent)
					RemoveAll = 0;
				if (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)TVItem.hItem))
					{
					SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
					ParentHost = (RasterAnimHost *)TVItem.lParam;
					} // if
				if (ParentHost)
					{
					if (RemoveAll)
						RemoveAll = ParentHost->FindnRemoveRAHostChild(RemoveHost, DoRemoveAll);
					else
						ParentHost->FindnRemoveRAHostChild(RemoveHost, DoRemoveAll);
					} // if
				else if (RemoveAll)
					{
					if (Prop->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
						{
						EffectsHost->RemoveRAHost(RemoveHost, 0);
						} // if effect
					else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR || Prop->TypeNumber == WCS_RAHOST_OBJTYPE_DEM ||
						Prop->TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT || Prop->TypeNumber == WCS_SUBCLASS_LAYER)
						{
						DBHost->RemoveRAHost(RemoveHost, ProjHost, EffectsHost, 0, 0);
						} // else if control pt
					else if (Prop->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
						{
						ImageHost->RemoveRAHost(RemoveHost, EffectsHost);
						} // else if raster
					} // if
				} // if deletable
			} // if single object
		else
			{
			for (Prop->TypeNumber = WCS_EFFECTSSUBCLASS_LAKE; Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS + 5; Prop->TypeNumber ++)
				{
				if (TVListItem[WidgetID == IDC_PARLIST2 ? 1: 0][Prop->TypeNumber] == TVItem.hItem)
					{
					if (Prop->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
						{
						sprintf(RemoveText, "Remove all %s from the current Project.", EffectsHost->GetEffectTypeName(Prop->TypeNumber));
						if (UserMessageOKCAN("Remove Effects", RemoveText))
							{
							EffectsHost->DeleteGroup(Prop->TypeNumber);
							} // if 
						} // if effect
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS)
						{
						sprintf(RemoveText, "Remove all %s from the current Project.", "Vectors");
						if (UserMessageOKCAN("Remove Vectors", RemoveText))
							{
							DBHost->RemoveAll(ProjHost, EffectsHost, WCS_RAHOST_OBJTYPE_VECTOR);
							} // if 
						} // else if vector
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 1)
						{
						sprintf(RemoveText, "Remove all %s from the current Project.", "DEMs");
						if (UserMessageOKCAN("Remove DEMs", RemoveText))
							{
							DBHost->RemoveAll(ProjHost, EffectsHost, WCS_RAHOST_OBJTYPE_DEM);
							DBHost->RemoveUnusedLayers();
							} // if 
						} // else if dem
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 2)
						{
						sprintf(RemoveText, "Remove all %s from the current Project.", "Control Points");
						if (UserMessageOKCAN("Remove Control Points", RemoveText))
							{
							DBHost->RemoveAll(ProjHost, EffectsHost, WCS_RAHOST_OBJTYPE_CONTROLPT);
							} // if 
						} // else if control pt
					else if (Prop->TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 3)
						{
						sprintf(RemoveText, "Remove all %s from the current Project.", "Image Objects");
						if (UserMessageOKCAN("Remove Image Objects", RemoveText))
							{
							ImageHost->RemoveAll(EffectsHost, FALSE);
							} // if 
						} // else if raster
					break;
					} // if
				} // for
			} // else
		delete Prop;
		delete ParentProp;
		} // if
	// Unlock S@G/UI
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);

	} // if

} // SceneViewGUI::RemoveObject

/*===========================================================================*/

// Returns the actual object, but the category of its root.
RasterAnimHost *SceneViewGUI::GetSelectedObjectAndCategory(long &Category)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *EditHost, *RootHost;
RasterAnimHostProperties Prop;

ListID = GetDlgItem(NativeWin, IDC_PARLIST1);

if (! Frozen && (TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0)))
	{
	TVItem.lParam = NULL;
	TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
	SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
	if (EditHost = (RasterAnimHost *)TVItem.lParam)
		{
		RootHost = EditHost->GetRAHostRoot();
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.TypeNumber = 0;
		RootHost->GetRAHostProperties(&Prop);
		Category = Prop.TypeNumber;
		return (EditHost);
		} // if 
	else
		{
		Category = IdentifyCategory((unsigned long)TVItem.hItem, 0);
		return (NULL);
		/*
		for (Prop.TypeNumber = WCS_EFFECTSSUBCLASS_LAKE; Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS + 5; Prop.TypeNumber ++)
			{
			if (TVListItem[0][Prop.TypeNumber] == TVItem.hItem)
				{
				if (Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
					{
					Category = Prop.TypeNumber;
					return (NULL);
					} // if effect
				else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS)
					{
					Category = WCS_RAHOST_OBJTYPE_VECTOR;
					return (NULL);
					} // else if vector
				else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 1)
					{
					Category = WCS_RAHOST_OBJTYPE_DEM;
					return (NULL);
					} // else if dem
				else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 2)
					{
					Category = WCS_RAHOST_OBJTYPE_CONTROLPT;
					return (NULL);
					} // else if control pt
				else if (Prop.TypeNumber == WCS_MAXIMPLEMENTED_EFFECTS + 3)
					{
					Category = WCS_RAHOST_OBJTYPE_RASTER;
					return (NULL);
					} // else if raster
				break;
				} // if
			} // for
		*/
		} // else
	} // if

Category = 0;
return (NULL);

} // SceneViewGUI::GetSelectedCategory

/*===========================================================================*/

void SceneViewGUI::SetModeListPtr(void)
{

switch (GlobalApp->MainProj->Prefs.TaskMode)
	{
	case WCS_PROJPREFS_TASKMODE_ALL:
		{
		ModeList = NULL;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_TERRAIN:
		{
		ModeList = IconFilterList_Terrain;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_LANDCOVER:
		{
		ModeList = IconFilterList_LandCover;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_WATER:
		{
		ModeList = IconFilterList_Water;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_AIR:
		{
		ModeList = IconFilterList_Air;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_LIGHT:
		{
		ModeList = IconFilterList_Light;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_OBJECT:
		{
		ModeList = IconFilterList_Object;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_VECTOR:
		{
		ModeList = IconFilterList_Vector;
		break;
		} // 
	case WCS_PROJPREFS_TASKMODE_RENDER:
		{
		ModeList = IconFilterList_Render;
		break;
		} // 
	} // switch

} // SceneViewGUI::SetModeListPtr

/*===========================================================================*/

int SceneViewGUI::TaskModeFilter(long Group)
{
long Ct = 0;

if (! ModeList)
	return (1);
while (ModeList[Ct] < 255)
	{
	if (ModeList[Ct] == Group)
		return (1);
	Ct ++;
	} // while

return (0);

} // SceneViewGUI::TaskModeFilter

/*===========================================================================*/

void SceneViewGUI::SetTaskMode(int ButtonID, int Selected)
{

if (Selected)
	{
	switch (ButtonID)
		{ // the ID_ variants (ID_TASKMODE_TERRAIN) are used by the key accelerators in WCS.rc IDR_KEYABBREVS
		case IDI_TB_TM_TERRAIN:
		case ID_TASKMODE_TERRAIN:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_TERRAIN;
			break;
			} // 
		case ID_TASKMODE_LANDCOVER:
		case IDI_TB_TM_LANDCOVER:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_LANDCOVER;
			break;
			} // 
		case ID_TASKMODE_WATER:
		case IDI_TB_TM_WATER:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_WATER;
			break;
			} // 
		case ID_TASKMODE_SKY:
		case IDI_TB_TM_SKY:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_AIR;
			break;
			} // 
		case ID_TASKMODE_LIGHT:
		case IDI_TB_TM_LIGHT:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_LIGHT;
			break;
			} // 
		case ID_TASKMODE_OBJECT:
		case IDI_TB_TM_3DOBJ:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_OBJECT;
			break;
			} // 
		case ID_TASKMODE_VECTOR:
		case IDI_TB_TM_VECTOR:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_VECTOR;
			break;
			} // 
		case ID_TASKMODE_RENDER:
		case IDI_TB_TM_RENDER:
			{
			GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_RENDER;
			break;
			} // 
		} // switch

	if (GlobalApp->MCP)
		{
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_TERRAIN, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_TERRAIN);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_LANDCOVER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_LANDCOVER);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_WATER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_WATER);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_SKY, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_AIR);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_LIGHT, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_LIGHT);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_3DOBJ, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_OBJECT);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_VECTOR, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_VECTOR);
		GlobalApp->MCP->SetToolbarButtonPressed(IDI_TB_TM_RENDER, GlobalApp->MainProj->Prefs.TaskMode == WCS_PROJPREFS_TASKMODE_RENDER);
		} // if

	} // if
else
	GlobalApp->MainProj->Prefs.TaskMode = WCS_PROJPREFS_TASKMODE_ALL;

if (GlobalApp->MCP)
	GlobalApp->MCP->EnforceTaskModeWindowCompliance(GlobalApp->MainProj->Prefs.TaskMode);	// a long name for a tedious task

BuildList(1, IDC_PARLIST1, 0);

} // SceneViewGUI::SetTaskMode

/*===========================================================================*/

long SceneViewGUI::HandleSplitToggle(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
SetSAGBottomHtPct(-GlobalApp->MainProj->Prefs.SAGBottomHtPct);
return(0);
} // SceneViewGUI::HandleSplitToggle

/*===========================================================================*/

// We don't appear to have a resizer widget anymore
/*
long SceneViewGUI::HandleResizerToggle(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
// toggle expand button, like a user would
WidgetSetCheck(IDC_EXPAND, !WidgetGetCheck(IDC_EXPAND));
// call expand-handling code under same assumptions
DoWindowExpand(NW);
return(0);
} // SceneViewGUI::HandleResizerToggle
*/

/*===========================================================================*/

long SceneViewGUI::HandleSplitMove(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
double Pct;
NativeControl TopTree, BotTree, TabControl;
RECT TopRect, BotRect;
int SplitY, TotHeight;
POINT Mouse;
int Percent;

TopTree     = GetWidgetFromID(IDC_PARLIST1);
BotTree     = SubPanels[0][WCS_SCENEVIEWGUI_TAB_SAG];
TabControl  = GetWidgetFromID(IDC_TAB1);

//  calculate size of windows
GetWindowRect(TopTree, &TopRect);
GetWindowRect(TabControl, &BotRect);

TotHeight = BotRect.bottom - TopRect.top;
Mouse.x = X;
Mouse.y = Y;

ClientToScreen(Handle, (LPPOINT)&Mouse);
ScreenToClient(NativeWin, (LPPOINT)&Mouse);
ScreenToClient(NativeWin, (LPPOINT)&TopRect);

// do the math
SplitY    = (Mouse.y + 2) - TopRect.top;

Pct = 1.0 - ((double)SplitY / (double)TotHeight);
Percent = (int)(Pct * 10000.0);
SetSAGBottomHtPct(Percent);

return(0);
} // SceneViewGUI::HandleSplitMove

/*===========================================================================*/

// we don't appear to have a resizer widget anymore
/*
long SceneViewGUI::HandleResizer(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{
NativeControl Sizer;
RECT SizeRect;
POINT Mouse;

Sizer = GetWidgetFromID(IDC_SCENESIZE);

//  calculate size of windows
GetWindowRect(Sizer, &SizeRect);

Mouse.x = X;
Mouse.y = Y;

ClientToScreen(Handle, (LPPOINT)&Mouse);
ScreenToClient(NativeWin, (LPPOINT)&Mouse);

// do the math
if (Mouse.x < SVGStockX) Mouse.x = SVGStockX;


MoveAndSizeFen(0, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN),
 (unsigned short)Mouse.x, GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE));
GetClientRect(NativeWin, &SizeRect);
HandleReSized(0, Wreck.right, Wreck.bottom);

return(0);
} // SceneViewGUI::HandleResizer
*/

/*===========================================================================*/

void SceneViewGUI::SetSAGBottomHtPct(int Percent)
{
double Pct;
NativeControl TopTree, BotTree, Splitter, TabControl, Diag, Point; //, Container;
RECT TopRect, BotRect;
int AllWidth, AllX, TopHeight, BotHeight, BotY, SplitY, TotHeight, SplitHeight = 8;
int ContainerLeft, ContainerRight, ContainerTop, ContainerBottom; //, ContainerWidth, ContainerHeight;

//if (Percent < 34) Percent = 3300;

GlobalApp->MainProj->Prefs.SAGBottomHtPct = Percent;
Pct = (double)Percent / 10000.0;

if (Pct < 0.0)
	{
	Pct = 0.0;
	} // if

TopTree   = GetWidgetFromID(IDC_PARLIST1);
BotTree   = SubPanels[0][WCS_SCENEVIEWGUI_TAB_SAG];
Diag = SubPanels[0][WCS_SCENEVIEWGUI_TAB_DIAG];
Point = SubPanels[0][WCS_SCENEVIEWGUI_TAB_POINT];
Splitter  = GetWidgetFromID(IDC_SCENESPLIT);
TabControl = GetWidgetFromID(IDC_TAB1);

//  do resize of windows
GetWindowRect(TopTree, &TopRect);
AllWidth = TopRect.right - TopRect.left;
GetWindowRect(TabControl, &BotRect);

TotHeight = BotRect.bottom - TopRect.top;

ScreenToClient(NativeWin, (LPPOINT)&TopRect);
AllX = TopRect.left;

// do the math
BotHeight = (int)((double)TotHeight * Pct);
TopHeight = TotHeight - BotHeight;

// Prevent total collapse
if (TopHeight < 20)
	{
	TopHeight = 20;
	BotHeight = (TotHeight - TopHeight);
	} // if

SplitY    = TopRect.top + TopHeight - SplitHeight;
BotY      = TopRect.top + TopHeight;

//ContainerWidth
//ContainerHeight
ContainerLeft   = AllX + WCS_SCENEVIEW_BOTCONTAIN_MARGIN_LEFT;
ContainerRight  = AllWidth - (WCS_SCENEVIEW_BOTCONTAIN_MARGIN_RIGHT + WCS_SCENEVIEW_BOTCONTAIN_MARGIN_LEFT);
ContainerTop    = BotY + WCS_SCENEVIEW_BOTCONTAIN_MARGIN_TOP;
ContainerBottom = BotHeight - (WCS_SCENEVIEW_BOTCONTAIN_MARGIN_BOTTOM + WCS_SCENEVIEW_BOTCONTAIN_MARGIN_TOP);

SetWindowPos(TopTree, NULL, 0, 0, AllWidth, TopHeight - SplitHeight, SWP_NOMOVE | SWP_NOZORDER);
SetWindowPos(BotTree, NULL, ContainerLeft, ContainerTop, ContainerRight, ContainerBottom, SWP_NOZORDER);
if (Diag)
	{
	SetWindowPos(Diag, NULL, ContainerLeft, ContainerTop, ContainerRight, ContainerBottom, SWP_NOZORDER);
	} // if
if (Point)
	{
	SetWindowPos(Point, NULL, ContainerLeft, ContainerTop, ContainerRight, ContainerBottom, SWP_NOZORDER);
	// for some reason the above SetWindowPos isn't firing the HandleReSized properly, so we force it
	if (GlobalApp->GUIWins->DRL) GlobalApp->GUIWins->DRL->HandleReSized(0, ContainerRight, ContainerBottom);
	} // if
SetWindowPos(TabControl, NULL, AllX, BotY, AllWidth, BotHeight, SWP_NOZORDER);
SetWindowPos(Splitter, NULL, AllX, SplitY, AllWidth, SplitHeight, SWP_NOZORDER);

} // SceneViewGUI::SetSAGBottomHtPct

/*===========================================================================*/

void SceneViewGUI::ActivateActiveItem(void)
{
int ListCt, ListID, WhatToGet, UseSledge = 1;
RasterAnimHost *CurActive;
TV_ITEM TVItem;

CurActive = RasterAnimHost::GetActiveRAHost();

if (!CurActive)
	{
	if (TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0))
		{
		TVItem.mask = (TVIF_HANDLE | TVIF_PARAM);
		SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (TVItem.lParam == NULL)
			{
			return;
			} // if
		} // if
	else
		{
		return;
		} // else
	} // if
else
	{
	if (TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0))
		{
		TVItem.mask = (TVIF_HANDLE | TVIF_PARAM);
		SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (TVItem.lParam == (long)CurActive)
			{
			return;
			} // if
		} // if
	} // else

// search displayed scene view items in both lists and
//		if item ! selected && item == active && active != NULL
//			set selected
//		else if item selected && item != active
//			set deselected

// Use big sledgehammer to unconditionally unselect the S@G
SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL);

// walk the top list reading each item displayed
for (ListCt = 0, ListID = IDC_PARLIST1; ListCt < 2; ListCt ++, ListID = IDC_PARLIST2)
	{
	TVItem.hItem = 0;
	WhatToGet = TVGN_FIRSTVISIBLE;
	while (TVItem.hItem = (HTREEITEM)SendDlgItemMessage(NativeWin, ListID, TVM_GETNEXTITEM, (WPARAM)WhatToGet, (LPARAM)TVItem.hItem))
		{
		TVItem.mask = (TVIF_HANDLE | TVIF_PARAM | TVIF_STATE);
		TVItem.stateMask = TVIS_SELECTED; 
		SendDlgItemMessage(NativeWin, ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (! (TVItem.state & (TVIS_SELECTED)) && CurActive == (RasterAnimHost *)TVItem.lParam && CurActive != NULL)
			{
			// set selected
			TVItem.mask = (TVIF_HANDLE | TVIF_STATE);
			TVItem.stateMask = TVItem.state = TVIS_SELECTED | TVIS_FOCUSED; 
			SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVItem);
			SendDlgItemMessage(NativeWin, ListID, TVM_SELECTITEM, TVGN_CARET, (LPARAM)TVItem.hItem);
			UseSledge = 0;
			} // if
		else if ((TVItem.state & TVIS_SELECTED) && CurActive != (RasterAnimHost *)TVItem.lParam)
			{
			// set deselected
			TVItem.mask = (TVIF_HANDLE | TVIF_STATE);
			TVItem.stateMask = TVIS_SELECTED | TVIS_FOCUSED;
			TVItem.state = 0;
			SendDlgItemMessage(NativeWin, ListID, TVM_SETITEM, 0, (LPARAM)&TVItem);
			} // else if
		WhatToGet = TVGN_NEXTVISIBLE;
		} // while
	} // for

// big hammer to clear all selected states
// This trick didn't work, so we do it unconditionally at the beginning of the for loop.
if (UseSledge)
	{
	//SendDlgItemMessage(NativeWin, IDC_PARLIST1, TVM_SELECTITEM, TVGN_CARET, (LPARAM)NULL);
	} // if

} // SceneViewGUI::ActivateActiveItem

/*===========================================================================*/

void SceneViewGUI::CopyObject(short WidgetID, RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost;
char CopyMsg[256];

ListID = GetDlgItem(NativeWin, WidgetID);

if (CopyProp = new RasterAnimHostProperties())
	{
	if (TreeItem)
		{
		TVItem.hItem = (HTREEITEM)TreeItem;
		} // if
	else
		{
		TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
		} // else
	if (TVItem.hItem)
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			CopyHost = EditTarget;
			} // if
		else
			{
			CopyHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (CopyHost)
			{
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			CopyHost->GetRAHostProperties(CopyProp);
			if (CopyProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
				{
				RasterAnimHost::SetCopyOfRAHost(CopyHost);
				sprintf(CopyMsg, "%s %s copied to clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK("Copy", "Selected item cannot be copied.");
				} // else
			} // if
		else
			{
			UserMessageOK("Copy", "Selected category cannot be copied.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // SceneViewGUI::CopyObject

/*===========================================================================*/

void SceneViewGUI::PasteObject(short WidgetID, RasterAnimHost *EditTarget, unsigned long TreeItem)
{
TV_ITEM TVItem;
HWND ListID;
RasterAnimHost *PasteHost, *CopyHost;
RasterAnimHostProperties *CopyProp;
char CopyMsg[256];

ListID = GetDlgItem(NativeWin, WidgetID);

if (CopyProp = new RasterAnimHostProperties())
	{
	if (TreeItem)
		{
		TVItem.hItem = (HTREEITEM)TreeItem;
		} // if
	else
		{
		TVItem.hItem = (HTREEITEM)SendMessage(ListID, TVM_GETNEXTITEM, TVGN_CARET, (LPARAM)0);
		} // else
	if (TVItem.hItem)
		{
		TVItem.lParam = NULL;
		TVItem.mask = TVIF_PARAM | TVIF_HANDLE;
		SendMessage(ListID, TVM_GETITEM, 0, (LPARAM)&TVItem);
		if (EditTarget)
			{
			PasteHost = EditTarget;
			} // if
		else
			{
			PasteHost = (RasterAnimHost *)TVItem.lParam;
			} // else
		if (PasteHost)
			{
			if (CopyHost = RasterAnimHost::GetCopyOfRAHost())
				{
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
				CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
				CopyHost->GetRAHostProperties(CopyProp);
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
				PasteHost->GetRAHostProperties(CopyProp);
				if (CopyProp->DropOK)
					{
					CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					CopyProp->DropSource = CopyHost;
					// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
					//  eg. still in progress through a DragnDropListGUI
					PasteHost->SetRAHostProperties(CopyProp);
					sprintf(CopyMsg, "%s %s pasted from clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
					} // else
				} // if
			else
				{
				UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "Selected category cannot be pasted.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // SceneViewGUI::PasteObject

/*===========================================================================*/

long SceneViewGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 2:
				{
				SwitchToTab(WCS_SCENEVIEWGUI_TAB_POINT);
				break;
				} // 1
			case 1:
				{
				SwitchToTab(WCS_SCENEVIEWGUI_TAB_DIAG);
				break;
				} // 1
			default:
				{
				SwitchToTab(WCS_SCENEVIEWGUI_TAB_SAG);
				break;
				} // 0
			} // switch
		break;
		}
	} // switch
return(0);

} // SceneViewGUI::HandlePageChange

/*===========================================================================*/

int SceneViewGUI::AddLowerPanelTab(char *TabName, WCS_SCENEVIEWGUI_TAB Slot, NativeGUIWin TabContents)
{
WidgetTCInsertItem(IDC_TAB1, Slot, TabName);
SubPanels[0][Slot] = TabContents;
SetParent(TabContents, NativeWin);
ShowWindow(TabContents, SW_HIDE); // will show via SwitchToTab when we're ready
RECT Wreck;
GetClientRect(NativeWin, &Wreck);
HandleReSized(0, Wreck.right, Wreck.bottom); // cause size management on both axes
InvalidateRect(NativeWin, NULL, TRUE);
return(1);
} // SceneViewGUI::AddLowerPanelTab 

/*===========================================================================*/

int SceneViewGUI::SwitchToTab(WCS_SCENEVIEWGUI_TAB WhichTab)
{
WidgetTCSetCurSel(IDC_TAB1, WhichTab);
ShowPanel(0, WhichTab);
return(1);
} // SceneViewGUI::SwitchToTab
