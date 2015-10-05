// Toolbar.cpp
// Code for Toolbar
// Built from RenderStatus.cpp on 12/13/95 by Chris "Xenon" Hanson
// Effects Library GUI elements added 5/97 by Gary R. Huber
// Copyright 1995 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "VisualStylesXP.h"
#include "Security.h"
#include "Types.h"
#include "Toolbar.h"
#include "WCSVersion.h"
#include "Project.h"
#include "Database.h"
#include "Interactive.h"
#include "Conservatory.h"
#include "Notify.h"
#include "Useful.h"
#include "ProjectDispatch.h"
#include "ProjectPrefsGUI.h"
#include "AppHelp.h"
#include "SceneViewGUI.h"
#include "ViewGUI.h"
#include "Requester.h"
#include "NumericEntryGUI.h"
#include "AnimGraphGUI.h"
#include "AnimGraphGUI.h"
#include "DragnDropListGUI.h"
#include "Script.h"
#include "AppMem.h"
#include "resource.h"
#include "FSSupport.h"

#ifdef WCS_BUILD_RTX
#include "NatureViewCrypto.h"
#endif // WCS_BUILD_RTX

#ifdef WCS_BUILD_GARYSTOYS
#include "GarysGenie.h"
#endif // WCS_BUILD_GARYSTOYS


extern long StartupWinOpenToPage; // communicate desired page to StartupWin

// our own private Idaho -- to keep resource.h from eating them...
#define ID_BUSYWIN_COMPLETE             1004
#define ID_BUSYWIN_REMAIN               1007
#define IDC_FRAME                       5494
#define IDC_THISFRAME                   6269


/*
	WCS_PROJPREFS_TASKMODE_ALL = 0,
	WCS_PROJPREFS_TASKMODE_TERRAIN,
	WCS_PROJPREFS_TASKMODE_LANDCOVER,
	WCS_PROJPREFS_TASKMODE_WATER,
	WCS_PROJPREFS_TASKMODE_AIR,
	WCS_PROJPREFS_TASKMODE_LIGHT,
	WCS_PROJPREFS_TASKMODE_OBJECT,
	WCS_PROJPREFS_TASKMODE_VECTOR,
	WCS_PROJPREFS_TASKMODE_RENDER
*/
// This array needs to be terminated with a full block of 8 NULL values.
unsigned long TaskModeCompliantWinList[][8] = 
	{
	// 8 modes for each entry
	'ATEF', // "Area Terraffector Editor",
	'ECEF', // "Ecosystem Editor",
	'LKEF', // "Lake Editor",
	'CLOD', // "Cloud Model Editor",
	'ILEF', // "Illumination Editor",
	'MATE', // "Material Editor",
	'COSY',	// "Coordinate System Editor"
	'CAMP', // "Camera Editor"

	'TERF', // "Terraffector Editor",
	'ENVI', // "Environment Editor",
	'WVED', // "Wave Editor",
	'CELP', // "Celestial Object Editor",
	'SHEF', // "Shadow Effect Editor",
	'3DOE', // "3D Object Editor",
	'TBLG',	// "Table List"
	'RNDO', // "Render Options Editor",

	'PLAN', // "Planet Options Editor",
	'CMAP', // "Color Map Editor",
	'STRM', // "Stream Editor",
	'SKYP', // "Sky Editor",
	'SUNP', // "Light Editor",
	'FNCE',	// "Fence Editor",
	NULL,
	'RNDJ', // "Render Job Editor",

	'TERP', // "Terrain Parameter Editor",
	'EDFO', // "Foliage Effect Editor",
	NULL,
	'ATMG', // "Atmosphere Editor",
	'EPTS', // "Light Position by Time",
	'LABL',	// "Label Editor"
	NULL,
	'PPRC',	// "Post Process Editor"

	'WEDT', // "DEM Interpolater",
	'GRND', // "Ground Editor",
	NULL,
	'STAR', // "Starfield Editor",
	NULL,
	NULL,
	NULL,
	'SCEN',	// "Scenario Editor",

	'TGRD',	// "Terrain Gridder",
	'SNOW', // "Snow Editor",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	'RTXP',

	'TGEN',	// "Terrain Generator"
	'ECTP', // "Ecotype Editor",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	'COSY',	// "Coordinate System Editor"
	'MATS', // "Material Strata Editor",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	'TBLG',	// "Table List"
	'THEM', // "Thematic Map Editor",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	'DEMP',	// "DEM Painter"
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	'DMRG',	// "DEM Merger"
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL

// <<<>>> ADD_NEW_EFFECTS add the editor's ID - replace the NULL value found in the first block that 
// has a NULL value in the slot corresponding to the appropriate task mode. In other words each block has the 
// same number of items as there are task modes. Terrain task mode is the first item in each block, the second 
// is Land Cover, i.e. the same order as they appear at the top of S@G. Be sure there is a full block of NULL's
// terminating the list.
	};
unsigned long TaskModeIndependentWinList[] = 
	{
	'ANIM', // "Animation Preview Control"
	'BSIN', // "Component Browse Info"
	'CRDT', // "Credits"
	'DB3L', // "Database Fields"
	'DBED', // "Database Editor"
	'NNGR', // "Terrain Builder"
	'DOCV', // "DEM Converter"
	'DEMB', // "Terrain Designer"
	'DIAG', // "Diagnostic Data"
	'DIGO', // "Create Wizard"
	'EFFL', // "Component Library"
	'IMGL', // "Image Object Library"
	'IMVG', // "Image View"
	'IMWZ', // "Import Wizard"
	'LWMO', // "Scene Export"
	'PREF', // "Preferences"
	'PRON', // "New Project"
	'PRJU', // "Project Update Wizard"
	'RCTL', // "Render Control"
	'REND', // "Render Preview"
	'SCIM', // "Scene Import"
	'SAAG', // "Scene at a Glance"
	'VTEG', // "Vector Editor"
	'VPEF', // "Vector Profile Editor"
	'VESC', // "Scale Vector Elevations"
	'VERS', // "Version"
	'VEWT', // "View Toolbar"
	'VPRF', // "View Preferences"
	'SEQU', // "Search Queries"
	'TPLM', // "Template Manager"
	'PTHT', // "Path Transfer"
	'EDSS', // "EDSS"
	'VPEX', // "Vector Profile Export"
	'ECTL',	// "Export Control"
	'FOWZ',	// "Forestry Wizard"
	'GWIZ', // "Gridder Wizard"
	'MWIZ',	// "Merger Wizard"
	'DRIL',	// "DrillDownInfoGUI" aka "Info About Point"
	NULL
	};

unsigned long TaskModeDestroyWinList[] = 
	{
	'EDCO',	// color ed
	'TEXG',	// texture ed
	'ANGR',	// anim graph
	'GPLG', // "Effect List"
	'DNDL', // "Copy Object List"
	'LOUV', // "Component Gallery"
	'KDSG', // "Delete Key Frames"
	NULL
	};

NativeGUIWin Toolbar::Open(Project *Proj)
{
NativeGUIWin Ret;

Ret = GUIFenetre::Open(Proj);

if (Ret)
	{
	#ifdef _WIN32
	ShowWindow(NativeWin, SW_SHOWNORMAL);
	#endif // _WIN32
	} // if

return(Ret);
} // Toolbar::Open


// CreateTopToolbar creates a toolbar and adds a set of buttons to it.
// The function returns the handle to the toolbar if successful, 
// or NULL otherwise. 
// hwndParent is the handle to the toolbar's parent window. 
HWND Toolbar::CreateTopToolbar(HWND hwndParent, HINSTANCE g_hinst)
{
   HWND hwndTB = NULL, hwndRB = NULL;
   REBARBANDINFO rbBand;
   RECT          rc;

// Create a Rebar to contain the toolbar
   hwndRB = CreateWindowEx(0, REBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_AUTOSIZE | CCS_NORESIZE,
        0, 0, LocalWinSys()->InquireDisplayWidth(), 20, hwndParent, 
        NULL, g_hinst, NULL); 

	//The rebar control has been succesfully created 
	REBARINFO ri={0};
	//no imagelist to attach to rebar
	ri.cbSize=sizeof(REBARINFO); 
	SendMessage(hwndRB,RB_SETBARINFO,0,reinterpret_cast<LPARAM>(&ri));

   // Initialize structure members for band(s).
   rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
   rbBand.fMask  = RBBIM_TEXT | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE;
   rbBand.fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE;

// Create a toolbar. 
   hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 
        0, 0, LocalWinSys()->InquireDisplayWidth(), 20, hwndRB, 
        NULL, g_hinst, NULL); 

   SetWindowLong(hwndTB, GWL_STYLE, GetWindowLong(hwndTB, GWL_STYLE) | TBSTYLE_FLAT);

// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
// backward compatibility. 
   SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
   SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS); 
   LRESULT lResult;
   // Get the tooltip so we can twiddle it
   HWND TTip;
   TTip = (HWND)SendMessage(hwndTB, TB_GETTOOLTIPS, 0, 0);
   if (TTip)
	{
	SetWindowLong(TTip, GWL_STYLE, GetWindowLong(TTip, GWL_STYLE) | TTS_ALWAYSTIP);
	// we actually force this TOPMOST again during TBN_GETINFOTIP processing elsewhere
	SetWindowPos(TTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	} // if

   toolBarIL = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK , 38, 0 ); // Will need to destroy in cleanup
   lResult = SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)toolBarIL); 
       
// Fill the TBBUTTON array with button information, and add the 
// buttons to the toolbar.
   ButtonCount = 0;
   AddToolbarImageIcon(hwndTB, "Terrain Task Mode [CTRL+1]", IDI_TB_TM_TERRAIN, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Land Cover Task Mode [CTRL+2]", IDI_TB_TM_LANDCOVER, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Water Task Mode [CTRL+3]", IDI_TB_TM_WATER, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Sky Task Mode [CTRL+4]", IDI_TB_TM_SKY, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Light Task Mode [CTRL+5]", IDI_TB_TM_LIGHT, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "3D Object Task Mode [CTRL+6]", IDI_TB_TM_3DOBJ, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Vector Task Mode [CTRL+7]", IDI_TB_TM_VECTOR, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Render Task Mode [CTRL+8]", IDI_TB_TM_RENDER, ButtonCount++, true);
   
   AddToolbarDivider(hwndTB);
 
   AddToolbarImageIcon(hwndTB, "Preferences [CTRL+P]", IDI_TB_PREFS, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Database [ALT+D]", IDI_TB_DATABASE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Image Object Library [ALT+J]", IDI_TB_IMAGEOBJLIB, ButtonCount++, false);

   AddToolbarImageIcon(hwndTB, "Import Wizard [CTRL+I]", IDI_TB_IMPORTWIZ, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Render Control [ALT+R]", IDI_TB_RENDER, ButtonCount++, false);

   AddToolbarDivider(hwndTB);

   AddToolbarImageIcon(hwndTB, "Move Mode [1 or M]", IDI_TB_MOVE, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Rotate Mode [2 or R]", IDI_TB_ROTATE, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Scale or Zoom Mode [3 or S]", IDI_TB_SCALEZOOM, ButtonCount++, true);

   AddToolbarImageIcon(hwndTB, "Enable X Axis [4 or X]", IDI_TB_XAXIS, XAxisID = ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Enable Y Axis [5 or Y]", IDI_TB_YAXIS, YAxisID = ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Enable Z Axis [6 or Z]", IDI_TB_ZAXIS, ZAxisID = ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Enable Elevation Axis", IDI_TB_ELEVAXIS, ButtonCount++, true);
   AddToolbarDivider(hwndTB);
   AddToolbarImageIcon(hwndTB, "Select/Show Points", IDI_TB_POINTSMODE, ButtonCount++, true);

   AddToolbarImageIcon(hwndTB, "Create", IDI_TB_CREATE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Replace Vector or Control Points", IDI_TB_REPLACE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Append to Vector or Control Points", IDI_TB_APPEND, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Delete Vector or Control Points", IDI_TB_DELETE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Undo Last Vector Edit", IDI_TB_UNDO, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Measure Distance", IDI_TB_MEASURE, ButtonCount++, true);
   
   // need to preload alternate images for XYZ/HPB axis buttons
   ImageList_AddIcon (toolBarIL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_TB_HAXIS))); HAxisID = ButtonCount++;
   ImageList_AddIcon (toolBarIL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_TB_PAXIS))); PAxisID = ButtonCount++;
   ImageList_AddIcon (toolBarIL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_TB_BAXIS))); BAxisID = ButtonCount++;


   SIZE TBSize;
   lResult = SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);
   SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
   SetWindowPos(hwndTB, NULL, 0, 0, LocalWinSys()->InquireDisplayWidth(), TBSize.cy + 2, SWP_NOZORDER);
   int Win2KPadding = 0;
   if (!g_xpStyle.IsAppThemed())
     {
     Win2KPadding = 2;
     } // if
   SetWindowPos(hwndRB, NULL, 0, 0, LocalWinSys()->InquireDisplayWidth(), TBSize.cy + 2 + Win2KPadding, SWP_NOMOVE | SWP_NOZORDER);
   
   GetClientRect(hwndTB, &rc);
   rbBand.lpText     = NULL;
   rbBand.hwndChild  = hwndTB;
   rbBand.cxMinChild = 0;
   rbBand.cyMinChild = rc.bottom;
   rbBand.cx         = rc.right;

   // Add the band with the toolbar.
   SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

   ShowWindow(hwndRB, SW_SHOW); 
   Fenetre *Stash;
   Stash = this; // Convert to *Fenetre, not obvious
   SetWindowLong(hwndTB, GWL_USERDATA, (LONG)Stash);
   
   return hwndTB; 
} // Toolbar::CreateTopToolbar


// CreateBottomToolbar creates a toolbar and adds a set of buttons to it.
// The function returns the handle to the toolbar if successful, 
// or NULL otherwise. 
// hwndParent is the handle to the toolbar's parent window. 
HWND Toolbar::CreateBottomToolbar(HWND hwndParent, HINSTANCE g_hinst)
{
   HWND hwndTB = NULL, hwndRB = NULL;
   REBARBANDINFO rbBand;
   RECT          rc;

// Create a Rebar to contain the toolbar
   hwndRB = CreateWindowEx(0, REBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_AUTOSIZE | CCS_NORESIZE,
        0, 0, LocalWinSys()->InquireDisplayWidth(), 50, hwndParent, 
        NULL, g_hinst, NULL); 

	//The rebar control has been succesfully created so attach two bands, each with
	//a single standard control (combobox and button).
	REBARINFO ri={0};
	//no imagelist to attach to rebar
	ri.cbSize=sizeof(REBARINFO); 
	SendMessage(hwndRB,RB_SETBARINFO,0,reinterpret_cast<LPARAM>(&ri));

   // Initialize structure members for band(s).
   rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
   rbBand.fMask  = RBBIM_TEXT | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_STYLE;
   rbBand.fStyle = RBBS_NOGRIPPER | RBBS_HIDETITLE;

// Create a toolbar. 
   hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 
        0, 0, LocalWinSys()->InquireDisplayWidth(), 50, hwndRB, 
        NULL, g_hinst, NULL);

   SetWindowLong(hwndTB, GWL_STYLE, GetWindowLong(hwndTB, GWL_STYLE) | TBSTYLE_FLAT);

// Send the TB_BUTTONSTRUCTSIZE message, which is required for 
// backward compatibility. 
   SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
   SendMessage(hwndTB, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS); 
   LRESULT lResult;
   // Get the tooltip so we can twiddle it
   HWND TTip;
   TTip = (HWND)SendMessage(hwndTB, TB_GETTOOLTIPS, 0, 0);
   if (TTip)
	{
	SetWindowLong(TTip, GWL_STYLE, GetWindowLong(TTip, GWL_STYLE) | TTS_ALWAYSTIP);
	// we actually force this TOPMOST again during TBN_GETINFOTIP processing elsewhere
	SetWindowPos(TTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	} // if

   lResult = SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)toolBarIL); 
   SendMessage(hwndTB, TB_AUTOSIZE, 0, 0);
       
// Fill the TBBUTTON array with button information, and add the 
// buttons to the toolbar.
   AddToolbarImageIcon(hwndTB, "Key Frame Group Toggle", IDI_KEYGROUP, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Add Key Frame(s)", IDI_KEYADD, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Remove Key Frame(s)", IDI_KEYREMOVE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Scale Key Frames", IDI_KEYSCALE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Open TimeLine for Active Parameter", IDI_TIMELINE, ButtonCount++, false);
   AddToolbarImageIcon(hwndTB, "Go to Previous Key Frame", IDI_PREVKEY, ButtonCount++, false);

   // Frame SmartNumeric
   FrameScrub = AddToolbarFrameScrub(hwndTB); // Frame FloatInt
/*   EnableWindow(FrameScrub, 0);
   InvalidateRect(FrameScrub, NULL, true);*/

   AddToolbarImageIcon(hwndTB, "Go to Next Key Frame", IDI_NEXTKEY, NextKeyID = ButtonCount++, false);

   // Active Item Field
   ObjectText = AddToolbarStatus(hwndTB, 30, ID_BUSYWIN_COMPLETE, false, true, false);

   AddToolbarImageIcon(hwndTB, "Lock Active Item", IDI_ICONLOCKINTER, ButtonCount++, true);
   AddToolbarImageIcon(hwndTB, "Status Log [ALT+G]", IDI_STATUSLOG, StatusLogID = ButtonCount++, true);

   // set marker for left edge of status log
   SIZE TBSize;
   SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
   // TB_GETMAXSIZE returns useless results for width under pre-XP. TB_GETITEMRECT gives the right numbers though.
   RECT ControlRECT;
   SendMessage(hwndTB, TB_GETITEMRECT, 10, (LPARAM)&ControlRECT); // 10 = number of StatusLog icon, counting spacers
   TBSize.cx = ControlRECT.right;
   
   GlobalApp->MainProj->SetRootMarkerQuiet(WCS_PROJECT_ROOTMARKER_LOGSTATUS_LEFTEDGE, (short)TBSize.cx, 0);
   
   // Status/Progress field
   StatusText = AddToolbarStatus(hwndTB, 30, ID_LOG_TEXT, true, false, false);
   ProgressText = AddToolbarStatus(hwndTB, 15, ID_BUSYWIN_REMAIN, false, true, true);
   ProgressBar = AddToolbarProgress(hwndTB, 15, ID_BUSYWIN_PERCENT, true, true, true);
   AddToolbarImageIcon(hwndTB, "Abort Operation [ESC]", IDI_STOP, ButtonCount++, false, false);
   
   // bottom row of lower toolbar
   SendMessage(hwndTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
   // TB_GETMAXSIZE returns useless results for width under pre-XP. TB_GETITEMRECT gives the right numbers though.
   SendMessage(hwndTB, TB_GETITEMRECT, 13, (LPARAM)&ControlRECT); // 13 = number of Abort icon, counting spacers
   TBSize.cx = ControlRECT.right;

   // Time slider
   FrameSlider = AddToolbarFrameSlider(hwndTB, TBSize.cy); // Frame PropGadget
   // Play button
   PlayButton = AddToolbarPlayIcon(hwndTB, "Play Animation", FrameSliderHeight == WCS_TOOLBAR_FRAMESLIDERHEIGHTSM ? IDI_ANIMPLAY : IDI_ANIMPLAYLG, TBSize.cy + 1);


   int ToolHeight = (TBSize.cy + FrameSliderHeight + 2);
   GlobalApp->MainProj->SetRootMarkerQuiet(WCS_PROJECT_ROOTMARKER_ANIMBAR_LR, (short)(TBSize.cx + 2), ToolHeight);

   SetWindowPos(hwndTB, NULL, 0, 0, TBSize.cx, TBSize.cy, SWP_NOMOVE | SWP_NOZORDER);
   SetWindowPos(hwndRB, NULL, 0, 0, LocalWinSys()->InquireDisplayWidth(), ToolHeight, SWP_NOMOVE | SWP_NOZORDER);
   
   GetClientRect(hwndTB, &rc);
   rbBand.lpText     = NULL;
   rbBand.hwndChild  = hwndTB;
   rbBand.cxMinChild = 0;
   rbBand.cyMinChild = rc.bottom;
   rbBand.cx         = rc.right;

   // Add the band with the toolbar.
   SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

   ShowWindow(hwndRB, SW_SHOW); 
   Fenetre *Stash;
   Stash = this; // Convert to *Fenetre, not obvious
   SetWindowLong(hwndTB, GWL_USERDATA, (LONG)Stash);
   
   return hwndTB; 
} // Toolbar::CreateBottomToolbar


void Toolbar::SetToolbarButtonHidden(WORD IconID, bool NewHiddenState)
{
LRESULT lResult;
lResult = SendMessage((HWND)TopTB, (UINT) TB_HIDEBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (NewHiddenState, 0));
lResult = SendMessage((HWND)BotTB, (UINT) TB_HIDEBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (NewHiddenState, 0));
} // Toolbar::SetToolbarButtonHidden


void Toolbar::SetToolbarButtonPressed(WORD IconID, bool NewState)
{
LRESULT lResult;
lResult = SendMessage((HWND)TopTB, (UINT) TB_CHECKBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (NewState, 0));
lResult = SendMessage((HWND)BotTB, (UINT) TB_CHECKBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (NewState, 0));
} // Toolbar::SetToolbarButtonPressed

bool Toolbar::GetToolbarButtonPressed(WORD IconID)
{
bool Result = false;
LRESULT lResult;
lResult = SendMessage((HWND)TopTB, (UINT) TB_GETSTATE, (WPARAM) IconID, (LPARAM) 0);
if (lResult == -1)
	{
	lResult = SendMessage((HWND)BotTB, (UINT) TB_GETSTATE, (WPARAM) IconID, (LPARAM) 0);
	} // if
if (lResult == -1)
	{
	return(false);
	} // if
if (lResult & TBSTATE_CHECKED)
	Result = true;
return(Result);
} // Toolbar::GetToolbarButtonPressed


void Toolbar::SetToolbarButtonDisabled(WORD IconID, bool NewDisabledState)
{
LRESULT lResult;
lResult = SendMessage((HWND)TopTB, (UINT) TB_ENABLEBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (!NewDisabledState, 0));
lResult = SendMessage((HWND)BotTB, (UINT) TB_ENABLEBUTTON, (WPARAM) IconID, (LPARAM) MAKELONG (!NewDisabledState, 0));
} // Toolbar::SetToolbarButtonDisabled



void Toolbar::ConfigureToolbarAxis(bool RotationMode)
{
TBBUTTONINFO TBBI;
TBBI.cbSize = sizeof(TBBUTTONINFO);
if (RotationMode)
	{
	TBBI.pszText = "Enable Heading Rotation [4]";
	TBBI.dwMask = TBIF_TEXT | TBIF_IMAGE;
	TBBI.iImage = HAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_XAXIS, (LPARAM)&TBBI);
	TBBI.pszText = "Enable Pitch Rotation [5]";
	TBBI.iImage = PAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_YAXIS, (LPARAM)&TBBI);
	TBBI.pszText = "Enable Banking Rotation [6]";
	TBBI.iImage = BAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_ZAXIS, (LPARAM)&TBBI);	
	} // if
else
	{
	TBBI.pszText = "Enable X Axis [4 or X]";
	TBBI.dwMask = TBIF_TEXT | TBIF_IMAGE;
	TBBI.iImage = XAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_XAXIS, (LPARAM)&TBBI);
	TBBI.pszText = "Enable Y Axis [5 or Y]";
	TBBI.iImage = YAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_YAXIS, (LPARAM)&TBBI);
	TBBI.pszText = "Enable Z Axis [6 or Z]";
	TBBI.iImage = ZAxisID;
	SendMessage((HWND)TopTB, (UINT) TB_SETBUTTONINFO, (WPARAM)IDI_TB_ZAXIS, (LPARAM)&TBBI);
	} // else
} // Toolbar::ConfigureToolbarAxis



NativeGUIWin Toolbar::Construct(void)
{
#ifdef _WIN32
WNDCLASS InquireRegister;
Fenetre *Stash;


// We do a devious thing here. Our RootWin doesn't have a Fenetre
// associated with it to collect input events. We'll attach the
// RootWin to the ToolWin, so we can get at the close gadget on
// the RootWin
Stash = this; // Convert to *Fenetre, not obvious
SetWindowLong(LocalWinSys()->RootWin, GWL_USERDATA, (LONG)Stash);

if (!GetClassInfo(LocalWinSys()->Instance(), APP_CLASSPREFIX ".ToolWin", &InquireRegister))
	{
	InquireRegister.style			= 0; // Can't resize
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= LocalWinSys()->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	InquireRegister.hbrBackground	= (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".ToolWin";
	RegisterClass(&InquireRegister);
	} // if

if (!GetClassInfo(LocalWinSys()->Instance(), APP_CLASSPREFIX ".ToolDummyWin", &InquireRegister))
	{
	InquireRegister.style			= 0; // Can't resize
	InquireRegister.lpfnWndProc		= WndProc;
	InquireRegister.cbClsExtra		= 0;
	InquireRegister.cbWndExtra		= 0;
	InquireRegister.hInstance		= LocalWinSys()->Instance();
	InquireRegister.hIcon			= NULL;
	InquireRegister.hCursor			= LoadCursor(NULL, IDC_ARROW);
	InquireRegister.hbrBackground	= NULL; //(HBRUSH)GetStockObject(LTGRAY_BRUSH);
	InquireRegister.lpszMenuName 	= NULL;
	InquireRegister.lpszClassName	= APP_CLASSPREFIX ".ToolDummyWin";
	RegisterClass(&InquireRegister);
	} // if

#endif // _WIN32



if (!NativeWin)
	{
	#ifdef _WIN32
	SIZE TBSize;

	// Top toolbar background
	SubPanels[0][0] = CreateWindowEx(0,
		APP_CLASSPREFIX ".ToolWin",
	    FenTitle,
        /* WS_BORDER | */ WS_CHILD | WS_CLIPSIBLINGS,     // window style
        -1, 0, LocalWinSys()->InquireDisplayWidth(), 25, // 25 is resized later
        LocalWinSys()->RootWin,                    // parent/owner window handle
        NULL,                    // window menu handle
        LocalWinSys()->Instance(),    // program instance handle
	    NULL) ;				     // creation parameters


	// Top toolbar contents (rebar and toolbar widgets, and contents)
	TopTB = CreateTopToolbar(SubPanels[0][0], LocalWinSys()->Instance());

	// Bottom Animation+etc bar
	SubPanels[0][1] = CreateWindowEx(0,
		APP_CLASSPREFIX ".ToolWin",
	    FenTitle,
        /* WS_BORDER |  */ WS_CHILD | WS_CLIPSIBLINGS,     // window style
        -1, LocalWinSys()->InquireDisplayHeight() - 50, LocalWinSys()->InquireDisplayWidth(), 50,
        LocalWinSys()->RootWin,                    // parent/owner window handle
        NULL,                    // window menu handle
        LocalWinSys()->Instance(),    // program instance handle
	    NULL) ;				     // creation parameters

	// Bottom toolbar contents (rebar and toolbar widgets, and contents)
	BotTB = CreateBottomToolbar(SubPanels[0][1], LocalWinSys()->Instance());
	HideProgress();

	// Dummy bar
	NativeWin = CreateWindowEx(0,
		APP_CLASSPREFIX ".ToolDummyWin",
	    FenTitle,
        WS_CHILD,     // window style
        -1, -1, 0, 0,
        LocalWinSys()->RootWin,                    // parent/owner window handle
        NULL,                    // window menu handle
        LocalWinSys()->Instance(),    // program instance handle
	    NULL) ;				     // creation parameters

	if (SubPanels[0][0])
		{
		SetWindowLong(SubPanels[0][0], GWL_USERDATA, NULL);
		ShowWindow(SubPanels[0][0], SW_SHOWNORMAL);
		SetWindowLong(SubPanels[0][0], GWL_USERDATA, (LONG)Stash);
		} // if
	if (SubPanels[0][1])
		{
		SetWindowLong(SubPanels[0][1], GWL_USERDATA, NULL);
		ShowWindow(SubPanels[0][1], SW_SHOWNORMAL);
		SetWindowLong(SubPanels[0][1], GWL_USERDATA, (LONG)Stash);
		} // if
	
	#endif // _WIN32

	#ifdef _WIN32
	SendMessage(TopTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
	SetWindowPos(SubPanels[0][0], NULL, -1, 0, LocalWinSys()->InquireDisplayWidth() + 2, TBSize.cy + 2, SWP_NOZORDER);
	GlobalApp->MainProj->SetRootMarkerQuiet(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN, 0, (short)TBSize.cy + 2);
	GlobalApp->MainProj->SetRootMarkerQuiet(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE, GlobalApp->WinSys->InquireRootWidth(), GlobalApp->WinSys->InquireRootHeight() - (short)TBSize.cy + 2);
	#endif // _WIN32

	// ********************** Bottom **************************

	#ifdef _WIN32
	{
	SendMessage(BotTB, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
	int TopEdgePos, ToolHeight;
	ToolHeight = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_ANIMBAR_LR);
	TopEdgePos = GlobalApp->WinSys->InquireRootHeight() - ToolHeight;
	GlobalApp->MainProj->SetRootMarkerQuiet(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE, GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE), GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE) - (short)(ToolHeight));
	SetWindowPos(SubPanels[0][1], NULL, -1, TopEdgePos, LocalWinSys()->InquireRootWidth() + 2, ToolHeight, SWP_NOZORDER);
	ShowWindow(SubPanels[0][1], SW_SHOWNORMAL);
	}
	#endif // _WIN32
	
	if (NativeWin)
		{
		#ifdef _WIN32
		// Create menu and attach to root window
		#ifdef WCS_BUILD_VNS
		SetMenu(LocalWinSys()->RootWin, (HMENU)(MainMenu = (HMENU)LoadMenu(LocalWinSys()->Instance(), MAKEINTRESOURCE(IDR_MAINMENU))));
		#else // !WCS_BUILD_VNS
		SetMenu(LocalWinSys()->RootWin, (HMENU)(MainMenu = (HMENU)LoadMenu(LocalWinSys()->Instance(), MAKEINTRESOURCE(IDR_MAINMENU_WCS))));
		#endif // !WCS_BUILD_VNS
		KeyAbbrev = LoadAccelerators(LocalWinSys()->Instance(), MAKEINTRESOURCE(IDR_KEYABBREVS));
		#endif // _WIN32
		} // if
	} // if
 
return(NativeWin);
} // Toolbar::Construct

/*===========================================================================*/

Toolbar::Toolbar(WCSApp *AppSource)
: GUIFenetre('TOOL', this, "Toolbar") // Yes, I know...
{
static NotifyTag AllMods[] = {MAKE_ID(WCS_TOOLBARCLASS_MODULES, 0xff, 0xff, 0xff), 0};
static NotifyTag AllProjects[] = {MAKE_ID(WCS_PROJECTCLASS_PATHS, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_PROJECTCLASS_PREFS, 0xff, 0xff, 0xff), 0};
static NotifyTag InterEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff), 0};
static NotifyTag AppExEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_FREEZE, 0xff, 0xff, 0xff), 
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED),
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED),
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED),
									MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
									MAKE_ID(WCS_NOTIFYCLASS_DELAYEDEDIT, WCS_NOTIFYSUBCLASS_ALL, WCS_NOTIFYITEM_ALL, WCS_NOTIFYCOMP_ALL),
									 0};
static NotifyTag MatrixEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0), 0};

Disabled_TimeLine = Disabled_KeyAdd = Disabled_PrevKey = Disabled_NextKey = false;

ProgressBar = ProgressText = ObjectText = StatusText = 0;
CurrentProgressText[0] = CurrentStatusText[0] = NULL;

BusyWinNest = 0;
toolBarIL = NULL;
CurrentlyPlaying = 0;
FrameSliderHeight = (LocalWinSys()->InquireDisplayHeight() > 768) ? WCS_TOOLBAR_FRAMESLIDERHEIGHTLG : WCS_TOOLBAR_FRAMESLIDERHEIGHTSM;

XAxisID = YAxisID = ZAxisID = HAxisID = PAxisID = BAxisID = NextKeyID = StatusLogID = 0;

MainMenu = NULL;
memset(&WindowsInMenuList[0], 0, sizeof (WindowsInMenuList));

#ifdef _WIN32
CommonFont = NULL;
FrameBorder = NULL;
#endif // _WIN32

RegisterClient((WCSModule *)this, AllMods);
AppSource->MainProj->RegisterClient((WCSModule *)this, AllProjects);
GlobalApp->MainProj->RegisterClient((WCSModule *)this, MatrixEvents);
GlobalApp->MainProj->Interactive->RegisterClient((WCSModule *)this, InterEvents);
GlobalApp->AppEx->RegisterClient((WCSModule *)this, AppExEvents);
#ifdef _WIN32
KeyAbbrev = NULL;
#endif // _WIN32

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOWINLIST | WCS_FENETRE_WINMAN_NOPOPUP);

} // Toolbar::Toolbar

/*===========================================================================*/

Toolbar::~Toolbar()
{
this->Close();

if (TopTB) DestroyWindow(TopTB);
if (BotTB) DestroyWindow(BotTB);
if (toolBarIL) ImageList_Destroy(toolBarIL);

RemoveClient((WCSModule *)this);
GlobalApp->MainProj->Interactive->RemoveClient((WCSModule *)this);
GlobalApp->AppEx->RemoveClient((WCSModule *)this);
if (GlobalApp->MainProj)
	GlobalApp->MainProj->RemoveClient(this);

} // Toolbar::~Toolbar

/*===========================================================================*/

#ifdef _WIN32
HWND Toolbar::FetchWinHandle(void)
{

return ((HWND)NativeWin);

} // FetchWinHandle()
#endif // _WIN32

/*===========================================================================*/

long Toolbar::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
RECT ToolInterior;
short TOriX, TOriY, DH, DW, RMY, WY, MW, MH;

if (BusyWinNest) return(0);

GetClientRect((HWND)GlobalApp->WinSys->GetRoot(), &ToolInterior);
DW  = (short)ToolInterior.right;
DH  = (short)ToolInterior.bottom;

if (GlobalApp->MainProj)
	{
	RMY = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_ANIMBAR_LR);
	WY  = DH - RMY;

	TOriX = GlobalApp->MainProj->InquireRootMarkerX(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
	TOriY = GlobalApp->MainProj->InquireRootMarkerY(WCS_PROJECT_ROOTMARKER_MATRIX_ORIGIN);
	MW = DW - TOriX;
	MH = DH - (TOriY + RMY);
	GlobalApp->MainProj->SetRootMarker(WCS_PROJECT_ROOTMARKER_MATRIX_SIZE,
	 MW,
	 MH);

	// update bottom toolbar
	SetWindowPos(SubPanels[0][1], NULL, -1, WY, DW, RMY, SWP_NOZORDER);
	} // if


if (GlobalApp->GUIWins && GlobalApp->GUIWins->SAG)
	{
	GlobalApp->GUIWins->SAG->HandleMatrixResize();
	} // if
if (GlobalApp->WinSys)
	{
	GlobalApp->WinSys->UpdateDocking(1);
	} // 

return(0);

} // Toolbar::HandleReSized

/*===========================================================================*/

long Toolbar::HandleBackgroundCrunch(int Siblings)
{
double End = 0.0, NewTime, NowTime;
// advance time

if (BusyWinNest) return(0);

if (End = GetRealMaxTime())
	{
	NewTime = GlobalApp->MainProj->Interactive->GetActiveTime();
	#ifdef WCS_BUILD_DEMO
	if (NewTime > End || NewTime >= 5.0)
	#else // WCS_BUILD_DEMO
	if (NewTime > End)
	#endif // WCS_BUILD_DEMO
		{ // reset to start
		LastFrameRedrawTime = GetSystemTimeFP();
		GlobalApp->MainProj->Interactive->SetActiveTime(0.0);
		} // if
	else
		{
		NowTime = GetSystemTimeFP();
		NewTime += (NowTime - LastFrameRedrawTime);
		LastFrameRedrawTime = NowTime;
		#ifdef WCS_BUILD_DEMO
		if (NewTime > 5.0)
			NewTime = 5.0;
		#endif // WCS_BUILD_DEMO
		GlobalApp->MainProj->Interactive->SetActiveTime(NewTime);
		} // else
	//GlobalApp->MainProj->Interactive->SetActiveFrame(GlobalApp->MainProj->Interactive->GetActiveFrame() + 1);
	} // if
else
	{
	GlobalApp->RemoveBGHandler(this);
	} // else
return(0);
} // Toolbar::HandleBackgroundCrunch

long Toolbar::HandleInquireScrollColor(NativeControl Widget) {return(0);}
long Toolbar::HandleCloseWin(NativeGUIWin NW)
{
#ifdef _WIN32
SetWindowLong(LocalWinSys()->RootWin, GWL_USERDATA, NULL);
#endif // _WIN32
GlobalApp->SetTerminate();
return(1);
}

long Toolbar::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

if (BusyWinNest) return(0);

switch(CtrlID)
	{
	case IDC_FRAME:
		{
		GlobalApp->MainProj->Interactive->SetActiveTime(FrameADT.CurValue);
		} // Start
	} // ID
return(1);
} // HandleFIChange

long Toolbar::HandleRepaint(NativeGUIWin NW, NativeDrawContext NDC)
{
long Ct, NumWindows, WinWidth, WinHeight, OffsetX, OffsetY;

if (NW == LocalWinSys()->RootWin)
	{
	HGDIOBJ Backup;
	// Draw Matrix cells
	NumWindows = GlobalApp->MainProj->ViewPorts.GetNumPanes(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT);

	if (!g_xpStyle.IsAppThemed())
		Backup = SelectObject(NDC, GetStockObject(BLACK_PEN));
	for (Ct = 0; Ct < NumWindows; Ct ++)
		{
		OffsetX   = GlobalApp->MainProj->ViewPorts.GetPaneX(Ct);
		OffsetY   = GlobalApp->MainProj->ViewPorts.GetPaneY(Ct);
		WinWidth  = GlobalApp->MainProj->ViewPorts.GetPaneW(Ct);
		WinHeight = GlobalApp->MainProj->ViewPorts.GetPaneH(Ct);

		if (g_xpStyle.IsAppThemed())
			{
			RECT Rect;
			Rect.left = OffsetX;
			Rect.right = OffsetX + WinWidth;
			Rect.top = OffsetY;
			Rect.bottom = OffsetY + WinHeight;
			HTHEME hTheme = g_xpStyle.OpenThemeData(NW, L"TRACKBAR");
			g_xpStyle.DrawThemeBackground(hTheme, NDC, TKP_TRACK, TRS_NORMAL, &Rect, 0);
			g_xpStyle.CloseThemeData(hTheme);
/*
			HTHEME hTheme = g_xpStyle.OpenThemeData(NW, L"WINDOW");
			g_xpStyle.DrawThemeBackground(hTheme, NDC, WP_MAXBUTTON, MAXBS_NORMAL, &Rect, 0);
			WCHAR szBkBitmapFilename[MAX_PATH];
			HBITMAP BGImageBM;
			g_xpStyle.GetThemeFilename(hTheme, WP_MAXBUTTON, MAXBS_NORMAL, TMT_IMAGEFILE, szBkBitmapFilename, MAX_PATH);
			BGImageBM = g_xpStyle.LoadBitmapFromTheme(szBkBitmapFilename);
			g_xpStyle.DrawThemeBackground(hTheme, NDC, WP_MAXBUTTON, MAXBS_NORMAL, &Rect, 0);
			g_xpStyle.CloseThemeData(hTheme);
*/			} // if
		else
			{
			// top
			MoveToEx(NDC, OffsetX, OffsetY, NULL);
			LineTo(NDC, OffsetX + WinWidth, OffsetY);
			
			// right
			SelectObject(NDC, GetStockObject(WHITE_PEN));
			LineTo(NDC, OffsetX + WinWidth, OffsetY + WinHeight);
			
			// bottom
			LineTo(NDC, OffsetX, OffsetY + WinHeight);
			
			// left
			SelectObject(NDC, GetStockObject(BLACK_PEN));
			LineTo(NDC, OffsetX, OffsetY);
			} // else
		} // for
	if (!g_xpStyle.IsAppThemed())
		SelectObject(NDC, Backup);
	return(1);
	} // if
return(0);
} // Toolbar::HandleRepaint

long Toolbar::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
if (BusyWinNest) return(0);

return(1);
}

long Toolbar::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

if (ButtonID == IDI_STOP)
	{
	BusyWinAbort = 1;
	return(0);
	} // STOP

if (BusyWinNest) return(0);

if (ButtonID >= WCS_WINDOWLIST_MINID && ButtonID < WCS_WINDOWLIST_MINID + 25)
	{
	WindowsInMenuList[ButtonID - WCS_WINDOWLIST_MINID]->Open(GlobalApp->MainProj);
	return (0);
	} // if

// moved this code out of switch/case statement to handle larger numbers of camera entities
if (ButtonID == ID_WINMENU_VIEW_EDITORS || (ButtonID >= ID_WINMENU_VIEW_CAM1 && ButtonID < ID_WINMENU_VIEW_CAM1 + WCS_VIEWGUI_VIEWS_CAMSINPOPUP_MAX))
	{
	int ViewPane, ItemIdx, Invalid = 0;
	GeneralEffect *MyEffect;
#ifndef WCS_BUILD_DEMO

	if (!GlobalApp->Sentinal->CheckDongle()) Invalid = 1;

#endif // !WCS_BUILD_DEMO
	if (!Invalid && (FenID == 'TOOL'))
		{
		POINT HitPoints;
		HitPoints.x = PopupX;
		HitPoints.y = PopupY;
		ScreenToClient(LocalWinSys()->RootWin, &HitPoints);
		ViewPane = GlobalApp->MainProj->ViewPorts.GetPaneFromXY((short)HitPoints.x, (short)HitPoints.y);
		if (ViewPane != -1)
			{
			if (ButtonID == ID_WINMENU_VIEW_EDITORS && GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
				{
				// Do stuff
				GlobalApp->MainProj->ViewPorts.ClearFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
				// <<<>>> Close View
				} // if
			else //if (!GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
				{
				GlobalApp->MainProj->ViewPorts.SetFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
				// identify the camera and open a view
				for (ItemIdx = 0, MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); MyEffect; MyEffect = MyEffect->Next)
					{
					if (ButtonID == (ID_WINMENU_VIEW_CAM1 + (ItemIdx++)))
						{
						if (GlobalApp->GUIWins->CVG)
							{
							GlobalApp->GUIWins->CVG->CreateNewView(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), (Camera *)MyEffect, NULL);
							} // if
						} // if
					} // for
				} // if
			} // if
		} // if
	} // View


switch(ButtonID)
	{
/*
	case ID_WINMENU_DIAGNOSTICS:
		{
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
					WCS_TOOLBAR_ITEM_RDG, 0);
		break;
		} // 
*/
	case ID_WINMENU_RENDERPREV:
		{
#ifndef WCS_BUILD_DEMO


		if (GlobalApp->GUIWins->CVG && GlobalApp->Sentinal->CheckDongle())


#else // WCS_BUILD_DEMO
		DONGLE_INLINE_CHECK()
#endif // WCS_BUILD_DEMO
			{
			GlobalApp->GUIWins->CVG->DoRender(-1); // forced focus loss occurs now in DoRender()
			} // if
		break;
		} // 
	case ID_WINMENU_RENDER_EDITMORE:
		{
#ifndef WCS_BUILD_DEMO

		if (GlobalApp->GUIWins->CVG && GlobalApp->Sentinal->CheckDongle())

#else // WCS_BUILD_DEMO
		DONGLE_INLINE_CHECK()
#endif // WCS_BUILD_DEMO
			{
			GlobalApp->GUIWins->CVG->DoPrefs(-1);
			} // if
		break;
		} // 
	case ID_WINMENU_VIEW_NEWPERSPEC:
	case ID_WINMENU_VIEW_NEWOVER:
	case ID_WINMENU_VIEW_NEWPLAN:
		{
		int ViewPane;
		char CamType = WCS_EFFECTS_CAMERATYPE_TARGETED;
		POINT HitPoints;
		HitPoints.x = PopupX;
		HitPoints.y = PopupY;
		ScreenToClient(LocalWinSys()->RootWin, &HitPoints);
#ifndef WCS_BUILD_DEMO


		if (!GlobalApp->Sentinal->CheckDongle()) break;


#endif // !WCS_BUILD_DEMO
		if (ButtonID == ID_WINMENU_VIEW_NEWOVER)
			{
			CamType = WCS_EFFECTS_CAMERATYPE_OVERHEAD;
			} // if
		if (ButtonID == ID_WINMENU_VIEW_NEWPLAN)
			{
			CamType = WCS_EFFECTS_CAMERATYPE_PLANIMETRIC;
			} // if
		ViewPane = GlobalApp->MainProj->ViewPorts.GetPaneFromXY((short)HitPoints.x, (short)HitPoints.y);
		if (GlobalApp->GUIWins->CVG && (ViewPane != -1))
			{
			GlobalApp->MainProj->ViewPorts.SetFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
			GlobalApp->GUIWins->CVG->CreateNewViewAndCamera(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), CamType);
			} // if
		break;
		} // 
	case IDI_TB_TM_TERRAIN:
	case IDI_TB_TM_LANDCOVER:
	case IDI_TB_TM_WATER:
	case IDI_TB_TM_SKY:
	case IDI_TB_TM_LIGHT:
	case IDI_TB_TM_3DOBJ:
	case IDI_TB_TM_VECTOR:
	case IDI_TB_TM_RENDER:
		{
		if (AppScope->GUIWins->SAG)
			{
			AppScope->GUIWins->SAG->SetTaskMode(ButtonID, GetToolbarButtonPressed(ButtonID));
			} // if
		break;
		} // Task modes
	// this set is the IDs used by the key accelerators in WCS.rc IDR_KEYABBREVS
	case ID_TASKMODE_TERRAIN:
	case ID_TASKMODE_LANDCOVER:
	case ID_TASKMODE_WATER:
	case ID_TASKMODE_SKY:
	case ID_TASKMODE_LIGHT:
	case ID_TASKMODE_OBJECT:
	case ID_TASKMODE_VECTOR:
	case ID_TASKMODE_RENDER:
		{
		if (AppScope->GUIWins->SAG)
			{
			AppScope->GUIWins->SAG->SetTaskMode(ButtonID, 1);
			} // if
		break;
		} // Task modes
	case IDI_TB_MOVE:
	case IDI_TB_ROTATE:
	case IDI_TB_SCALEZOOM:
	case IDI_TB_XAXIS:
	case IDI_TB_YAXIS:
	case IDI_TB_ZAXIS:
	case IDI_TB_ELEVAXIS:
	case IDI_TB_POINTSMODE:
	case IDI_TB_CREATE:
	case IDI_TB_REPLACE:
	case IDI_TB_APPEND:
	case IDI_TB_DELETE:
	case IDI_TB_UNDO:
	case IDI_TB_MEASURE:
	case IDI_TB_RENDERVIEW:
	case IDI_TB_LTDREGION: // needs to remain to drive F5 kb abbrev
		{
		if (GlobalApp->GUIWins->CVG)
			{
			GlobalApp->GUIWins->CVG->HandleButtonClick(Handle, NW, ButtonID);
			} // if
		break;
		} // View-events
	case IDI_TIMELINE:
		{
		OpenTimeLine();
		break;
		} // 
	case IDI_KEYGROUP:
		{
		KeyGroupMode();
		break;
		} // 
	case IDI_KEYADD:
		{
		KeyFrameAdd(LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1 : 0);
		break;
		} // 
	case IDI_KEYREMOVE:
		{
		KeyFrameRemove();	// argument was ignored in v5 anyway
		break;
		} // 
	case IDI_ANIMPLAY:
	case IDI_ANIMPLAYLG:
		{
		PlayMode();
		break;
		} // 
	case IDI_KEYSCALE:
		{
		KeyFrameScale();
		break;
		} // 
	case IDI_PREVKEY:
		{
		NextKeyFrame(-1);
		break;
		} // 
	case IDI_NEXTKEY:
		{
		NextKeyFrame(1);
		break;
		} // 
	case IDI_ICONLOCKINTER:
		{
		RasterAnimHost::SetActiveLock(GetToolbarButtonPressed(ButtonID));
		break;
		} // 
	case IDI_IMPORTWIZ:
	case ID_IMPORTWIZ:
	case IDI_TB_IMPORTWIZ:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_IWG, 0);
		return(1);
		} // ID_IMPORTWIZ
	case ID_INTERPDEM:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_IDG, 0);
		return(1);
		} // ID_INTERPDEM
	case ID_COMPONENTLIB:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_EFG, 0, 0);
		return(1);
		} // COMPONENT LIB
	case ID_IMAGELIB:
	case IDI_TB_IMAGEOBJLIB:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_ILG, 0, 0);
		return(1);
		} // IMAGEOBJ LIB
	case ID_TERRAINGEN:
		{
		// TerraGenerator component editor
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_TGN, 0);
		return(1);
		} // TERRAINGEN
	case ID_TERRAINGRID:
		{
		GeneralEffect *Pop;
		if (Pop = GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_GRIDDER, 1, GlobalApp->AppDB))
			{
			Pop->EditRAHost();
			} // if
		return(1);
		} // TERRAINGRID
	case ID_DATA_PATHTRANSFER:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_PTH, 0);
		return(1);
		} // ID_DATA_PATHTRANSFER
	case ID_PROJECT_OPENRESUME:
		{
		AppScope->MainProj->LoadResumeState(AppScope->GetProgDir());
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT ID_PROJECT_OPENRESUME
	case ID_PROJECT_OPEN:
		{
		AppScope->MainProj->Load(NULL, NULL, AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ1:
		{
		if (AppScope->MainProj->LastProject[0][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[0], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(1);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ2:
		{
		if (AppScope->MainProj->LastProject[1][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[1], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(2);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ3:
		{
		if (AppScope->MainProj->LastProject[2][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[2], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(3);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ4:
		{
		if (AppScope->MainProj->LastProject[3][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[3], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(4);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ5:
		{
		if (AppScope->MainProj->LastProject[4][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[4], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(5);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_FILE_PREVPROJ6:
		{
		if (AppScope->MainProj->LastProject[5][0])
			if (! AppScope->MainProj->Load(NULL, AppScope->MainProj->LastProject[5], AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
				AppScope->MainProj->DeleteFromRecentProjectList(6);
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // PROJECT OPEN
	case ID_PROJECT_SIGNSAVE:
		{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Projects cannot be saved.");
#else // !WCS_BUILD_DEMO
		if (AppScope->MainProj->ProjectLoaded)
			AppScope->MainProj->OpenBrowseData(AppScope->AppEffects);
		else
			UserMessageOK("Save Project", "There is no Project to save. You must load or create a Project first.");
#endif // !WCS_BUILD_DEMO
		return(1);
		} // ID_PROJECT_SIGNSAVE
	case ID_PROJECT_SAVEAS:
		{
		if (AppScope->MainProj->ProjectLoaded)
			{
			AppScope->MainProj->Save(NULL, NULL, AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff);
			#ifndef WCS_BUILD_DEMO
			AppScope->MainProj->SavePrefs(AppScope->GetProgDir());
			#endif // WCS_BUILD_DEMO
			} // else
		else
			UserMessageOK("Save Project", "There is no Project to save. You must load or create a Project first.");
		return(1);
		} // PROJECT SAVEAS
	case ID_PROJECT_SAVEASTEMPLATE:
		{
		char Backup, ProjPath0, FramePath0, ContPath0;
		if (AppScope->MainProj->ProjectLoaded)
			{
			if ((Backup = UserMessageYNCAN("Save As Template", "Do you wish to save the template file in such a way as to be portable to other machines? If you plan to use the template only on the current computer, answer \"no.\" If you answer \"yes\" then Master Paths will not be written to the file and Master Paths will be inherited from the destination machine's VNS.Prefs file.")) == 2)
				{
				// zero out first character so they don't get saved
				ProjPath0 = AppScope->MainProj->pcprojectpath[0];
				FramePath0 = AppScope->MainProj->pcframespath[0];
				ContPath0 = AppScope->MainProj->contentpath[0];
				AppScope->MainProj->pcprojectpath[0] = 0;
				AppScope->MainProj->pcframespath[0] = 0;
				AppScope->MainProj->contentpath[0] = 0;
				} // if
			else if (! Backup)	// user cancelled
				return (1);
			AppScope->MainProj->Save(NULL, NULL, AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff);
			#ifndef WCS_BUILD_DEMO
			if (Backup != 2)	// don't save prefs if master paths are NULLed
				AppScope->MainProj->SavePrefs(AppScope->GetProgDir());
			#endif // WCS_BUILD_DEMO
			if (Backup == 2)
				{
				AppScope->MainProj->pcprojectpath[0] = ProjPath0;
				AppScope->MainProj->pcframespath[0] = FramePath0;
				AppScope->MainProj->contentpath[0] = ContPath0;
				} // if
			} // else
		else
			UserMessageOK("Save Project", "There is no Project to save. You must load or create a Project first.");
		return(1);
		} // ID_PROJECT_SAVEASTEMPLATE
	case ID_PROJECT_SAVE:
		{
		char filename[256];
		if (AppScope->MainProj->ProjectLoaded)
			{
			strmfp(filename, AppScope->MainProj->projectpath, AppScope->MainProj->projectname);
			if (! AppScope->MainProj->Save(NULL, filename, AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages, NULL, 0xffffffff))
			#ifndef WCS_BUILD_DEMO
				{
				UserMessageOK("Project: Save", "An error occurred saving the file.\n File could not be opened for writing or an\n error occurred while writing the file. \nCheck disk access privileges and amount of free space.");
				} // if
			AppScope->MainProj->SavePrefs(AppScope->GetProgDir());
			#else // WCS_BUILD_DEMO
				{ // goes with the if, above, in Demo builds
				return(1);
				} // if
			#endif // WCS_BUILD_DEMO
			} // if
		else
			UserMessageOK("Save Project", "There is no Project to save. You must load or create a Project first.");
		return(1);
		} // PROJECT SAVE
	case ID_PROJECT_NEW:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PNG, 0);
		return(1);
		} // ID_PROJECT_NEW
	case ID_PROJECT_EDIT:
		{
		//GlobalApp->GUIWins->PPG->SetActivePage(5);
		ProjectPrefsGUI::SetActivePage(5);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_PROJECT_EDIT
	case ID_PROJECT_NEWUSER:
		{
		#ifndef WCS_BUILD_DEMO
		StartupWinOpenToPage = 4; // users page
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_VER, 0);
		#else // WCS_BUILD_DEMO
		UserMessageDemo("Multiple-user environment is not supported.");
		#endif // WCS_BUILD_DEMO
		return(1);
		} // ID_PROJECT_NEWUSER
	case ID_HELP_AUTH:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_AUT, 0);
		return(1);
		} // ID_HELP_AUTH
	case ID_WINMENU_FORESTWIZ:
		{
		#ifdef WCS_FORESTRY_WIZARD
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_FWZ, 0);
		#endif // WCS_FORESTRY_WIZARD
		return(1);
		} // ID_WINMENU_FORESTWIZ
	case ID_WINMENU_GRIDDERWIZ:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_GWZ, 0);
		return(1);
		} // ID_WINMENU_GRIDDERWIZ
	case ID_WINMENU_MERGERWIZ:
		{
		#ifdef WCS_BUILD_VNS
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_MWZ, 0);
		#endif // WCS_BUILD_VNS
		return(1);
		} // ID_WINMENU_FORESTWIZ
	case ID_PROJECT_VERSION:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // ID_PROJECT_VERSION
	case ID_PROJECT_CREDITS:
		{
		if (AppScope->HelpSys)
			AppScope->HelpSys->OpenCredits();
		return(1);
		} // ID_PROJECT_CREDITS
	case ID_PROJECT_TEMPLATEMAN:
		{
		#ifdef WCS_BUILD_VNS
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_TPM, 0);
		#endif // WCS_BUILD_VNS
		return(1);
		} // ID_PROJECT_TEMPLATEMAN
	case IDI_STATUSLOG:
	case ID_PROJECT_LOG:
		{
		if (AppScope->StatusLog && AppScope->StatusLog->IsLogOpen())
			{ // close it
			SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			 WCS_TOOLBAR_ITEM_STL, 0);
			} // if
		else
			{ // open it
			SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			 WCS_TOOLBAR_ITEM_STL, 0);
			} // else
		return(1);
		} // 
	case ID_RENDERCONTROL:
	case IDI_TB_RENDER:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_RCG, 0);
		return(1);
		} // RENDER
	case ID_DBEDIT:
	case IDI_DBEDIT:
	case IDI_TB_DATABASE:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_DBG, 0);
		return(1);
		} // ID_DBEDIT_OPEN
	case ID_PROJECT_IMPORTSCENE:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_SIM, 0);
		return(1);
		} // ID_PROJECT_IMPORTSCENE
	case ID_PROJECT_EXPORTSCENE:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_LWG, 0);
		return(1);
		} // ID_LWEXPORT_OPEN
#ifdef WCS_BUILD_RTX
	case ID_PROJECT_SXEXPORT:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_EXG, 0);
		return(1);
		} // ID_LWEXPORT_OPEN
	case ID_SX_SIGNFILE:
		{
		#ifdef WCS_BUILD_VNS
		RequestAndSignNVWFile(NVW_KEY_SX1_VNS_2);
		#else // WCS (!VNS)
		RequestAndSignNVWFile(NVW_KEY_SX1_WCS_6);
		#endif // WCS (!VNS)
		return(1);
		} // ID_LWEXPORT_OPEN
#endif // WCS_BUILD_RTX
	case ID_PROJECT_DIRLIST:
		{
		ProjectPrefsGUI::SetActivePage(6);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_DIRLIST_OPEN
	case ID_PROJECT_PREFERENCES:
	case IDI_TB_PREFS:
		{
		ProjectPrefsGUI::SetActivePage(0);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_PROJECT_PREFERENCES
	case ID_PROJECT_REMOVEUSER:
		{
		RemoveCurrentUser();
		return(1);
		} // ID_DIRLIST_OPEN
	case ID_VIEW_COMPONENTGAL:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_LVE, 0);
		return(1);
		} // IDI_GALLERY_TOOLBAR
	case ID_WINDOW_MATRIX:
		{
		ProjectPrefsGUI::SetActivePage(1);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_WINDOW_MATRIX
	case ID_WINDOW_CLOSEALL:
		{
		CloseAllWindowMenuWindows();
		return(1);
		} // ID_WINDOW_MATRIX
	case ID_FILE_PREFERENCES_PROJECT:
		{
		ProjectPrefsGUI::SetActivePage(2);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_FILE_PREFERENCES_PROJECT
	case ID_FILE_PREFERENCES_INTERACT:
		{
		ProjectPrefsGUI::SetActivePage(3);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_FILE_PREFERENCES_PROJECT
	case ID_FILE_PREFERENCES_UNITS:
		{
		ProjectPrefsGUI::SetActivePage(4);
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_PPG, 0);
		return(1);
		} // ID_FILE_PREFERENCES_UNITS
	case ID_VIEW_IMAGE_FROMDISK:
		{
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_IVG, 0);
		return(1);
		} // ID_VIEW_IMAGE_FROMDISK
	case ID_VIEW_IMAGE_LASTRENDERED:
		{
		#ifdef WCS_BUILD_DEMO
		UserMessageDemo("Demo version does not save rendered images.");
		#else // WCS_BUILD_DEMO
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_IVG, 1);
		#endif // WCS_BUILD_DEMO
		return(1);
		} // ID_VIEW_IMAGE_LASTRENDERED
	case ID_PROJECT_QUIT:
		{
		AppScope->SetTerminate();
		return(1);
		} // 
	case ID_ANIM_GOTOFRAME:
		{
		char FrameStr[64];
		FrameStr[0] = 0;
		if (GetInputString("Go to frame number:", WCS_REQUESTER_POSINTEGERS_ONLY, FrameStr))
			{
			GlobalApp->MainProj->Interactive->SetActiveFrame(atoi(FrameStr));
			} // if
		return(1);
		} // 
	case ID_ANIM_NUMERIC:
		{
		//#ifdef WCS_BUILD_GARYSTOYS
		//GarysGenie Genie;
		//Genie.Appear();
		//#endif // WCS_BUILD_GARYSTOYS
		NumericValue();
		return(1);
		} // 
	case ID_PROJECT_MATRIX_0:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 0, NULL); break;
	case ID_PROJECT_MATRIX_1:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 1, NULL); break;
	case ID_PROJECT_MATRIX_2:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 2, NULL); break;
	case ID_PROJECT_MATRIX_3:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 3, NULL); break;
	case ID_PROJECT_MATRIX_4:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 4, NULL); break;
	case ID_PROJECT_MATRIX_5:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 5, NULL); break;
	case ID_PROJECT_MATRIX_6:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 6, NULL); break;
	case ID_PROJECT_MATRIX_7:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 7, NULL); break;
	case ID_PROJECT_MATRIX_8:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 8, NULL); break;
	case ID_PROJECT_MATRIX_9:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 9, NULL); break;
	case ID_PROJECT_MATRIX_10:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 10, NULL); break;
	case ID_PROJECT_MATRIX_11:
		AppScope->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0, 11, NULL); break;
	case ID_HELP_TUTORIALS:
		{
		StartupWinOpenToPage = 1; // tutorials page
		SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		 WCS_TOOLBAR_ITEM_VER, 0);
		return(1);
		} // 
	case ID_HELP_ONLINE:
		{
		if (AppScope->HelpSys)
			AppScope->HelpSys->OpenHelpTopic(NULL);
		return(1);
		} // 
	case ID_HELP_HOMEPAGE:
		{
		if (AppScope->HelpSys)
			AppScope->HelpSys->OpenURLIndirect("http://WWW.3DNature.COM");
		return(1);
		} // 
	case ID_HELP_REPORT:
		{
		// going to reuse this item in demo version
		#ifdef WCS_BUILD_DEMO
		if (AppScope->HelpSys)
			AppScope->HelpSys->DoOnlinePurchase();
		#else // !WCS_BUILD_DEMO
		if (AppScope->HelpSys)
			AppScope->HelpSys->DoOnlineReport();
		#endif // !WCS_BUILD_DEMO
		return(1);
		} // 
	case ID_HELP_REGISTER:
		{
		if (AppScope->HelpSys)
			AppScope->HelpSys->DoOnlineRegister();
		return(1);
		} // 
	case ID_HELP_UPDATES:
		{
		if (AppScope->HelpSys)
			AppScope->HelpSys->DoOnlineUpdate();
		return(1);
		} // 
	case ID_SCRIPT_COMMAND:
		{
		char CommandStr[2000];
		CommandStr[0] = 0;
		if (GetInputString("Enter script command:", "", CommandStr))
			{
			GlobalApp->SuperScript->RunSingleCommand(CommandStr);
			} // if
		return(1);
		} // 
	case ID_SCRIPT_START:
		{
		FileReq *GetScript;
		if (GetScript = new FileReq)
			{
			char Ptrn[32];
			//GetScript->SetDefFile("WCSCRIPT.txt");
			strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("txt"));
			GetScript->SetDefPat(Ptrn);
			if (GetScript->Request(0))
				{
				GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_START_SCRIPT, (char *)GetScript->GetFirstName());
				GlobalApp->SuperScript->StartScript((char *)GetScript->GetFirstName());
				} // if
			delete GetScript;
			GetScript = NULL;
			} // if
		return(1);
		} //
	case ID_WINMENU_DEMMERGE:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_MRG, 0);
		return(1);
		} // ID_WINMENU_DEMMERGE
	default:
		break;
	} // switch ButtonID

return(0);

} // Toolbar::HandleButtonClick

/*===========================================================================*/

long Toolbar::HandlePopupMenuSelect(int MenuID) {return(HandleMenuSelect(MenuID));}

/*===========================================================================*/

long Toolbar::HandleMenuSelect(int MenuID) {return(HandleButtonClick(NULL, NULL, MenuID));}

/*===========================================================================*/

long Toolbar::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{

if (BusyWinNest) return(0);

// We're not ignoring SB_ENDSCROLL anymore
if (ScrollCode)
	{
	GlobalApp->MainProj->Interactive->SetActiveFrame(ScrollPos);
	return(0);
	} // if
else
	return(10);
} // Toolbar::HandleScroll

/*===========================================================================*/

void Toolbar::SetParam(int Notify, ...)
{
va_list VarA;
unsigned int ParClass, SubClass, Item, Component, Event;
ULONG NotifyChanges[5];
char DoAsDelayed = 0;

va_start(VarA, Notify);
for (Event = 0; Event < 1; Event++)
	{
	ParClass = va_arg(VarA, int);

	if (ParClass)
		{
		if (ParClass > 255)
			{
			NotifyChanges[Event] = ParClass;
			SubClass = (ParClass & 0xff0000) >> 16;
			Item = (ParClass & 0xff00) >> 8;
			Component = (ParClass & 0xff);
			ParClass >>= 24;
			} // if event passed as longword ID
		else
			{
			SubClass = va_arg(VarA, int);
			Item = va_arg(VarA, int);
			Component = va_arg(VarA, int);
			NotifyChanges[Event] = (ParClass << 24) + (SubClass << 16) + (Item << 8) + Component;
			} // else event passed as decomposed list of class, subclass, item and component

		if ((ParClass == WCS_TOOLBARCLASS_MODULES) && (SubClass != WCS_TOOLBAR_OPEN_MOD))
			{
			DoAsDelayed = 1;
			} // if

		// Nothing to do for real...
		} // if
	else
		{
		break;
		} // else
	} // for


NotifyChanges[Event] = NULL;

// Now all we do is generate a (possibly delayed) notify event...
if (Notify)
	{
	if (DoAsDelayed)
		{
		GenerateDelayedNotify(NotifyChanges);
		} // if
	else
		{
		GenerateNotify(NotifyChanges);
		} // else
	} // if

return;
} // Toolbar::SetParam

void Toolbar::GetParam(void *Value, ...)
{ // Not implemented in this class
return;
} // Toolbar::GetParam

bool Toolbar::AddToolbarImageIcon(NativeControl Bar, char *HelpCapt, WORD IconID, unsigned long int ImageNum, bool Tog, bool StartHidden)
{

ImageList_AddIcon (toolBarIL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IconID))); 

TBBUTTON tbb;
LRESULT lResult;

tbb.iBitmap = ImageNum; 
tbb.idCommand = IconID; 
tbb.fsState = TBSTATE_ENABLED | (StartHidden ? TBSTATE_HIDDEN : 0); 
tbb.fsStyle = Tog ? BTNS_CHECK : BTNS_BUTTON;
tbb.dwData = 0; 
tbb.iString = (INT_PTR)HelpCapt;
lResult = SendMessage(Bar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM) (LPTBBUTTON) &tbb); 

return(lResult ? true : false);

} // Toolbar::AddToolbarImageIcon

/*===========================================================================*/

bool Toolbar::AddToolbarTextButton(NativeControl Bar, char *String, char *HelpCapt, char Tog)
{
/*
TBBUTTON tbb; 
LRESULT lResult;

tbb[ButtonCount].iBitmap = ImageNum; 
tbb[ButtonCount].idCommand = IconID; 
tbb[ButtonCount].fsState = TBSTATE_ENABLED; 
tbb[ButtonCount].fsStyle = Tog ? BTNS_CHECK : BTNS_BUTTON;
tbb[ButtonCount].dwData = 0; 
tbb[ButtonCount].iString = (INT_PTR)HelpCapt;
lResult = SendMessage(Bar, TB_ADDBUTTONS, (WPARAM)ButtonCount, (LPARAM) (LPTBBUTTON) &tbb); 

return(lResult ? true : false);
*/
return(false);

} // Toolbar::AddToolbarTextButton

/*===========================================================================*/

bool Toolbar::AddToolbarDivider(NativeControl Bar, unsigned long int Width)
{
TBBUTTON tbb; 
LRESULT lResult;

tbb.iBitmap = Width; 
tbb.idCommand = 0; 
tbb.fsState = TBSTATE_ENABLED; 
tbb.fsStyle = BTNS_SEP;
tbb.dwData = 0; 
tbb.iString = 0;
lResult = SendMessage(Bar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM) (LPTBBUTTON) &tbb); 

return(lResult ? true : false);

} // Toolbar::AddToolbarDivider

/*===========================================================================*/

NativeControl Toolbar::AddToolbarStatus(NativeControl Bar, char MaxLetters, WORD Id, bool MaximizeRemainingWidth, bool AutoPadToolbar, bool StartHidden)
{
int Width, MaxWidth, StatusX, StatusHeight;
# ifdef _WIN32
NativeControl Button;
Width = MaxLetters * 10;
SIZE TBSize;
int ButtonIdx;
HWND Rebar = GetParent(Bar);
HWND SubWin = GetParent(Rebar);

SendMessage(Bar, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
if (ButtonCount == NextKeyID + 1)
	{
	ButtonIdx = 7; // after IDI_NEXTKEY
	} // if
else
	{
	ButtonIdx = 10; // after IDI_STATUSLOG
	} // else
// TB_GETMAXSIZE returns useless results for width under pre-XP. TB_GETITEMRECT gives the right numbers though.
RECT ControlRECT;
SendMessage(Bar, TB_GETITEMRECT, ButtonIdx, (LPARAM)&ControlRECT); // ButtonIdx is the magic
TBSize.cx = ControlRECT.right;
StatusX = TBSize.cx;
if (MaximizeRemainingWidth)
	{
	MaxWidth = LocalWinSys()->InquireDisplayWidth() - (StatusX + 100);
	Width = MaxWidth;
	} // if
StatusHeight = TBSize.cy - 3;

if (Button = CreateWindowEx(0, APP_CLASSPREFIX ".StatusBar", "", (StartHidden ? 0 : WS_VISIBLE) | WS_CHILD,
 StatusX, 2, Width, StatusHeight, SubWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	SetWindowLong(Button, GWL_ID, Id);
	if (AutoPadToolbar)
		{
		// add blank space to toolbar
		AddToolbarDivider(Bar, Width);
		} // if
	SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return(Button);
	} // if
return(NULL);
# endif // _WIN32
} // Toolbar::AddToolbarStatus

/*===========================================================================*/

NativeControl Toolbar::AddToolbarProgress(NativeControl Bar, char MaxLetters, WORD Id, bool MaximizeRemainingWidth, bool AutoPadToolbar, bool StartHidden)
{
int Width, MaxWidth, ProgressX, ProgressHeight;
# ifdef _WIN32
NativeControl Button;
Width = MaxLetters * 10;
SIZE TBSize;
HWND Rebar = GetParent(Bar);
HWND SubWin = GetParent(Rebar);

SendMessage(Bar, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
// TB_GETMAXSIZE returns useless results for width under pre-XP. TB_GETITEMRECT gives the right numbers though.
RECT ControlRECT;
SendMessage(Bar, TB_GETITEMRECT, 11, (LPARAM)&ControlRECT); // 11 is the magic number -- after our own Status
TBSize.cx = ControlRECT.right;
ProgressX = TBSize.cx;

if (MaximizeRemainingWidth)
	{
	MaxWidth = LocalWinSys()->InquireDisplayWidth() - (ProgressX + 100);
	Width = MaxWidth;
	} // if
ProgressHeight = TBSize.cy - 3;

if (Button = CreateWindowEx(0, PROGRESS_CLASS, "Status", (StartHidden ? 0 : WS_VISIBLE) | WS_CHILD,
 ProgressX, 2, Width, ProgressHeight, SubWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	SetWindowLong(Button, GWL_ID, Id);
	if (AutoPadToolbar)
		{
		// add blank space to toolbar
		AddToolbarDivider(Bar, Width);
		} // if
	SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return(Button);
	} // if
return(NULL);
# endif // _WIN32
} // Toolbar::AddToolbarProgress

/*===========================================================================*/

NativeControl Toolbar::AddToolbarFrameScrub(NativeControl Bar)
{
double RangeDefaults[3];
HWND Rebar = GetParent(Bar);
HWND SubWin = GetParent(Rebar);

FrameADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
RangeDefaults[0] = FLT_MAX;
RangeDefaults[1] = 0.0;
RangeDefaults[2] = 1.0 / 30.0;
FrameADT.SetRangeDefaults(RangeDefaults);
FrameADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

# ifdef _WIN32
NativeControl Button;
int IconHeight, IconWidth;
SIZE TBSize;

SendMessage(Bar, TB_GETMAXSIZE, 0, (LPARAM)&TBSize);
// TB_GETMAXSIZE returns useless results for width under pre-XP. TB_GETITEMRECT gives the right numbers though.
RECT ControlRECT;
SendMessage(Bar, TB_GETITEMRECT, 5, (LPARAM)&ControlRECT); // 5 = number of PrevKey icon, counting spacers
TBSize.cx = ControlRECT.right;

IconHeight = TBSize.cy - 2;
IconWidth  = 66;

if (Button = CreateWindowEx(0, APP_CLASSPREFIX ".FloatInt", NULL, WS_VISIBLE | WS_CHILD | 0x5,
 TBSize.cx, 1, IconWidth, IconHeight, SubWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	SetWindowLong(Button, GWL_ID, IDC_FRAME);
	// add corresponding blank space to toolbar
	AddToolbarDivider(Bar, IconWidth);
	SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SendMessage(Button, WM_SETFONT, (WPARAM)CommonFont, 1);
	return(Button);
	} // if/
return(NULL);
# endif // _WIN32
} // Toolbar::AddToolbarFrameScrub

/*===========================================================================*/

NativeControl Toolbar::AddToolbarFrameSlider(NativeControl Bar, int Y)
{
NativeControl Button;
int Height, Width;
HWND Rebar = GetParent(Bar);
HWND SubWin = GetParent(Rebar);

Height = FrameSliderHeight;
Width  = LocalWinSys()->InquireDisplayWidth() - 100;

if (Button = CreateWindowEx(0, "SCROLLBAR", NULL, WS_VISIBLE | WS_CHILD | SBS_HORZ,
 2, Y, Width, Height, SubWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowLong(Button, GWL_ID, IDC_THISFRAME);
	return(Button);
	} // if/
return(NULL);

} // Toolbar::AddToolbarFrameSlider

/*===========================================================================*/

NativeControl Toolbar::AddToolbarPlayIcon(NativeControl Bar, char *HelpCapt, WORD Icon, int Y)
{
NativeControl Button;

# ifdef _WIN32
int IconHeight, IconWidth, X;
unsigned long int WStyle = WS_VISIBLE | WS_CHILD | WCSW_TB_STYLE_NOFOCUS | WCSW_TB_STYLE_TOG | WCSW_TB_STYLE_XPLOOK;
HWND Rebar = GetParent(Bar);
HWND SubWin = GetParent(Rebar);

IconHeight = FrameSliderHeight - 1;
IconWidth  = 16;
X = 2 + LocalWinSys()->InquireDisplayWidth() - 100;

if (Button = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", HelpCapt, WStyle,
 X, Y, IconWidth, IconHeight, SubWin, NULL, LocalWinSys()->Instance(), NULL))
	{
	ConfigureTB(Button, NULL, Icon, NULL);
	SetWindowLong(Button, GWL_ID, Icon);
	SendMessage(Button, WM_SETFONT, (WPARAM)CommonFont, 1);
	SetWindowPos(Button, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return(Button);
	} // if
return(0);

#endif // _WIN32
} // Toolbar::AddToolbarPlayIcon

/*===========================================================================*/

void Toolbar::UpdateStatus(NativeControl Status, WORD Id, char *StatText)
{

if (Status)
	{
	SendMessage(Status, WM_SETTEXT, 0, (LPARAM)StatText);
	} // 

} // Toolbar::UpdateStatus 

/*===========================================================================*/

void Toolbar::UpdateProgress(NativeControl Progress, WORD Id, float Percent)
{

if (Progress)
	{
	SendMessage(Progress, PBM_SETPOS, (WPARAM)(Percent * 100.0f), 0);
	} // if

} // Toolbar::UpdateProgress

/*===========================================================================*/

void Toolbar::UpdateProgress(NativeControl Progress, WORD Id, int Percent)
{

if (Progress)
	{
	SendMessage(Progress, PBM_SETPOS, (WPARAM)(Percent), 0);
	} // if

} // Toolbar::UpdateProgress

/*===========================================================================*/

void Toolbar::SetCurrentProgressText(char *NewText)
{

UpdateStatus(ProgressText, NULL, NewText);

} // Toolbar::SetCurrentProgressText

/*===========================================================================*/

void Toolbar::SetCurrentProgressAmount(float Val)
{

UpdateProgress(ProgressBar, NULL, Val);

} // Toolbar::SetCurrentProgressAmount

/*===========================================================================*/

void Toolbar::SetCurrentProgressAmount(int Val)
{

UpdateProgress(ProgressBar, NULL, Val);

} // Toolbar::SetCurrentProgressAmount

/*===========================================================================*/

void Toolbar::ClearProgress(void)
{

UpdateStatus(ProgressText, NULL, "");
SetCurrentProgressAmount(0.0f);

} // Toolbar::ClearProgress

/*===========================================================================*/

void Toolbar::ShowProgress(void)
{

ShowWindow(StatusText, SW_HIDE);
ShowWindow(ProgressText, SW_SHOW);
ShowWindow(ProgressBar, SW_SHOW);
SetToolbarButtonHidden(IDI_STOP, false);

} // Toolbar::ShowProgress

/*===========================================================================*/

void Toolbar::HideProgress(void)
{

ShowWindow(StatusText, SW_SHOW);
ShowWindow(ProgressText, SW_HIDE);
ShowWindow(ProgressBar, SW_HIDE);
SetToolbarButtonHidden(IDI_STOP, true);

} // Toolbar::HideProgress

/*===========================================================================*/

static char ToolbarStatusFilter[201];
void Toolbar::SetCurrentStatusText(const char *NewText)
{
int FilterScan;

// Try to filter out bad characters.
for (FilterScan = 0; (FilterScan < 200) && (NewText[FilterScan]); FilterScan++)
	{
	if (!isprint(NewText[FilterScan]))
		{
		ToolbarStatusFilter[FilterScan] = ' ';
		} // if
	else
		{
		ToolbarStatusFilter[FilterScan] = NewText[FilterScan];
		} // else
	} // for
ToolbarStatusFilter[FilterScan] = NULL;

UpdateStatus(StatusText, NULL, ToolbarStatusFilter);

} // Toolbar::SetCurrentStatusText

/*===========================================================================*/

void Toolbar::ClearStatus(void)
{

UpdateStatus(StatusText, NULL, "");

} // Toolbar::ClearStatus

/*===========================================================================*/

void Toolbar::SetCurrentNoteText(char *NewText)
{

UpdateStatus(StatusText, NULL, NewText);

} // Toolbar::SetCurrentStatusText

/*===========================================================================*/

void Toolbar::ClearNote(void)
{

UpdateStatus(StatusText, NULL, "");

} // Toolbar::ClearStatus

/*===========================================================================*/

void Toolbar::UpdateBusy(char MoreBusy)
{

if (BusyWinNest)
	{
	EnableWindow(SubPanels[0][0], 0);

	SetToolbarButtonDisabled(IDI_KEYGROUP, true);
	SetToolbarButtonDisabled(IDI_KEYADD, true);
	SetToolbarButtonDisabled(IDI_KEYREMOVE, true);
	SetToolbarButtonDisabled(IDI_KEYSCALE, true);
	SetToolbarButtonDisabled(IDI_TIMELINE, true);
	SetToolbarButtonDisabled(IDI_PREVKEY, true);
	EnableWindow(FrameScrub, false);
	SetToolbarButtonDisabled(IDI_NEXTKEY, true);
	EnableWindow(ObjectText, false);
	SetToolbarButtonDisabled(IDI_ICONLOCKINTER, true);
	SetToolbarButtonDisabled(IDI_STATUSLOG, true);
	EnableWindow(FrameSlider, false);
	EnableWindow(PlayButton, false);
	} // if
else
	{
	EnableWindow(SubPanels[0][0], 1);

	SetToolbarButtonDisabled(IDI_KEYGROUP, false);
	SetToolbarButtonDisabled(IDI_KEYADD, Disabled_KeyAdd);
	SetToolbarButtonDisabled(IDI_KEYREMOVE, false);
	SetToolbarButtonDisabled(IDI_KEYSCALE, false);
	SetToolbarButtonDisabled(IDI_TIMELINE, Disabled_TimeLine);
	SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey);
	EnableWindow(FrameScrub, true);
	SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey);
	EnableWindow(ObjectText, true);
	SetToolbarButtonDisabled(IDI_ICONLOCKINTER, false);
	SetToolbarButtonDisabled(IDI_STATUSLOG, false);
	EnableWindow(FrameSlider, true);
	EnableWindow(PlayButton, true);
	} // else

} // Toolbar::UpdateBusy

/*===========================================================================*/

void Toolbar::SetCurrentObject(RasterAnimHost *NewActive, int CheckSAG)
{
double CurTime;
RasterAnimHostProperties Prop;
char ObjDescr[256], *NameTest = NULL;

ObjDescr[0] = 0;

// notify ViewGUI to update its portion of the toolbar
if (GlobalApp->GUIWins->CVG)
	{
	GlobalApp->GUIWins->CVG->ConfigureWidgets();
	GlobalApp->GUIWins->CVG->Draw();
	} // if

if (NewActive)
	{
	if (NewActive->GetRAHostName())
		{
		strcpy(ObjDescr, NewActive->GetRAHostName());
		} // if
	if (NewActive->RAParent)
		{
		if (NewActive->RAParent->RAParent && (NameTest = NewActive->RAParent->RAParent->GetCritterName(NewActive->RAParent)) && NameTest[0])
			{
			if (strcmp(NameTest, ObjDescr))
				{
				strcat(ObjDescr, " ");
				strcat(ObjDescr, NameTest);
				} // if 
			} // if
		NameTest = NewActive->RAParent->GetCritterName(NewActive);
		if (NameTest && NameTest[0])
			{
			strcat(ObjDescr, " ");
			strcat(ObjDescr, NameTest);
			} // if
		} // if
	if ((! NameTest || ! NameTest[0]) && NewActive->GetRAHostTypeString())
		{
		strcat(ObjDescr, " ");
		strcat(ObjDescr, NewActive->GetRAHostTypeString());
		} // if
	// anim button activation
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_KEYRANGE;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
	NewActive->GetRAHostProperties(&Prop);
	SetToolbarButtonDisabled(IDI_TIMELINE, Disabled_TimeLine = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATABLE) ? 0: 1);
	SetToolbarButtonDisabled(IDI_KEYADD, Disabled_KeyAdd = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATABLE) ? 0: 1);

	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
		SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = (CurTime <= Prop.KeyNodeRange[0]));
		SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = (CurTime >= Prop.KeyNodeRange[1]));
		} // if animated
	else
		{
		SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = true);
		SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = true);
		} // else

	} // if
else if (CheckSAG)
	{
	if (GlobalApp->GUIWins->SAG)
		{
		GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(Prop.TypeNumber);
		if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
			Prop.ItemOperator = WCS_KEYOPERATION_OBJCLASS;
		else
			Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
		} // if
	else
		Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
	Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
	GlobalApp->AppEffects->GetRAHostProperties(&Prop);
	CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
	SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = (CurTime <= Prop.KeyNodeRange[0]));
	SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = (CurTime >= Prop.KeyNodeRange[1]));
	SetToolbarButtonDisabled(IDI_TIMELINE, Disabled_TimeLine = true);
	SetToolbarButtonDisabled(IDI_KEYADD, Disabled_KeyAdd = true);
	} // else
else
	{
	SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = true);
	SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = true);
	SetToolbarButtonDisabled(IDI_TIMELINE, Disabled_TimeLine = true);
	SetToolbarButtonDisabled(IDI_KEYADD, Disabled_KeyAdd = true);
	} // else

SetCurrentObjectText(ObjDescr);

} // Toolbar::SetCurrentObject

/*===========================================================================*/

void Toolbar::SetCurrentObjectText(char *NewText)
{

UpdateStatus(ObjectText, NULL, NewText);

} // Toolbar::SetCurrentStatusText

/*===========================================================================*/

void Toolbar::ClearObject(void)
{

UpdateStatus(ObjectText, NULL, "");

} // Toolbar::ClearStatus

/*===========================================================================*/

void Toolbar::ConfigureAnimWidgets(void)
{

SetToolbarButtonPressed(IDI_KEYGROUP, GlobalApp->MainProj->GetKeyGroupMode() ? true : false);
FrameNumber();
FrameScroll();

} // Toolbar::ConfigureAnimWidgets

/*===========================================================================*/

static char RecentProjectStr[500];
void Toolbar::UpdateRecentProjects(void)
{
HMENU FileMenu;
MENUITEMINFO MenuItem;
unsigned long ItemID, Ct;

if (MainMenu)
	{
	if (FileMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_FILE))
		{
		MenuItem.fMask = MIIM_TYPE;
		MenuItem.fType = MFT_STRING;

		for (Ct = 0; Ct < 6; Ct ++)
			{
			if (Ct == 0)
				ItemID = ID_FILE_PREVPROJ1;
			else if (Ct == 1)
				ItemID = ID_FILE_PREVPROJ2;
			else if (Ct == 2)
				ItemID = ID_FILE_PREVPROJ3;
			else if (Ct == 3)
				ItemID = ID_FILE_PREVPROJ4;
			else if (Ct == 4)
				ItemID = ID_FILE_PREVPROJ5;
			else
				ItemID = ID_FILE_PREVPROJ6;

			sprintf(RecentProjectStr, "&%d %s\tALT+%d", Ct + 1, GlobalApp->MainProj->UnMungPath(GlobalApp->MainProj->LastProject[Ct]), Ct + 1);
			MenuItem.dwTypeData = RecentProjectStr;
			MenuItem.cbSize = sizeof(MENUITEMINFO);
			SetMenuItemInfo(FileMenu, ItemID, 0, &MenuItem);
			} // for
		} // if
	} // if

} // Toolbar::UpdateRecentProjects

/*===========================================================================*/

void Toolbar::AddWindowToMenuList(GUIFenetre *AddMe)
{

} // Toolbar::AddWindowToMenuList

/*===========================================================================*/

static int TBTestPilotAdded, TBScriptMenuAdded, HelpVersionCredits, RTXAdded;
static int ForestryWizardMenuAdded, GridderWizardMenuAdded, MergerWizardMenuAdded;

void Toolbar::RealAddWindowToMenuList(GUIFenetre *AddMe)
{
HMENU WindowMenu;
MENUITEMINFO MenuItem;
int FirstAvailable = -1, Ct;
char Title[40];

if (AddMe && AddMe->GetTitle(Title, 39))
	{
	if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_WINDOW))
		{
		for (Ct = 0; Ct < WCS_WINDOWLIST_MAXNUMWINS; Ct ++)
			{
			if (AddMe == WindowsInMenuList[Ct])
				{
				return;
				} // if already in list
			else if (FirstAvailable < 0 && ! WindowsInMenuList[Ct])
				FirstAvailable = Ct;
			} // for
		if (FirstAvailable >= 0)
			{
			MenuItem.cbSize = sizeof(MENUITEMINFO);
			MenuItem.fMask = MIIM_TYPE | MIIM_ID;
			MenuItem.fType = MFT_STRING;
			MenuItem.wID = WCS_WINDOWLIST_MINID + FirstAvailable;
			MenuItem.dwTypeData = Title;
			InsertMenuItem(WindowMenu, 3 /*ID_WINDOW_WINLIST*/, 1, &MenuItem);
			WindowsInMenuList[FirstAvailable] = AddMe;
			DrawMenuBar(LocalWinSys()->RootWin);
			} // if
		} // if
	} // if


if (!RTXAdded)
	{
	// not a high-security check
	if (GlobalApp->SXAuthorized)
		{
		HMENU FileMenu;
		if (FileMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_FILE))
			{
			EnableMenuItem(FileMenu, ID_PROJECT_SXEXPORT, MF_ENABLED);
			} // if
		} // if

	// don't display Auth window in Render Engine
#ifndef WCS_BUILD_DEMO
	if (GlobalApp->Sentinal->CheckDongle())
		{
		// Demo Version doesn't have Authorization window

		// Add authorization item to Help menu
		if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_HELP))
			{
			MenuItem.cbSize = sizeof(MENUITEMINFO);
			MenuItem.fMask = MIIM_TYPE | MIIM_ID;
			MenuItem.fType = MFT_STRING;
			MenuItem.wID = ID_HELP_AUTH;
			MenuItem.dwTypeData = APP_TLA " &Authorization";
			AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

			DrawMenuBar(LocalWinSys()->RootWin);
			} // if



		// not a high-security check
		if (GlobalApp->SXAuthorized)
			{
			// Add Scene Express Utilities item to Data menu
			if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_DATA))
				{
				AppendMenu(WindowMenu, MF_STRING, ID_SX_SIGNFILE, "Scene Express: Sign NatureView File...");
				DrawMenuBar(LocalWinSys()->RootWin);
				} // if
			} // if

		} // if
#endif // !WCS_BUILD_DEMO
	// even if we didn't add it above (Render Engine), set this so we stop trying
	RTXAdded = 1;
	} // if

// Add Help/Version and Help/Credits menus
if (!HelpVersionCredits)
	{
	if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_HELP))
		{
		// append spacer line
		AppendMenu(WindowMenu, MF_SEPARATOR, 0, 0);

		MenuItem.cbSize = sizeof(MENUITEMINFO);
		MenuItem.fMask = MIIM_TYPE | MIIM_ID;
		MenuItem.fType = MFT_STRING;
		MenuItem.wID = ID_PROJECT_VERSION;
		MenuItem.dwTypeData = APP_TLA " &Version";
		AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

		MenuItem.fMask = MIIM_TYPE | MIIM_ID;
		MenuItem.fType = MFT_STRING;
		MenuItem.wID = ID_PROJECT_CREDITS;
		MenuItem.dwTypeData = APP_TLA " &Credits";
		AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

		DrawMenuBar(LocalWinSys()->RootWin);
		HelpVersionCredits = 1;
		} // if
	} // if

// Add secret beta-tester bug-report menu
// (or, in demo version, ghost useless menus and add Buy menu)
if (!TBTestPilotAdded)
	{
	// Unconditionally add this item in Demo version
	#ifndef WCS_BUILD_DEMO
	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("testpilot"))
	#endif // !WCS_BUILD_DEMO
		{
		if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_HELP))
			{
			MenuItem.cbSize = sizeof(MENUITEMINFO);
			MenuItem.fMask = MIIM_TYPE | MIIM_ID;
			MenuItem.fType = MFT_STRING;
			MenuItem.wID = ID_HELP_REPORT;
			#ifdef WCS_BUILD_DEMO
			MenuItem.dwTypeData = "&Buy "APP_TLA" Online!";
			#else // !WCS_BUILD_DEMO
			MenuItem.dwTypeData = "&File Report";
			#endif // !WCS_BUILD_DEMO
			InsertMenuItem(WindowMenu, 2 /*ID_WINDOW_WINLIST*/, 1, &MenuItem);
			DrawMenuBar(LocalWinSys()->RootWin);
			TBTestPilotAdded = 1;
			} // if

		#ifdef WCS_BUILD_DEMO
		if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_HELP))
			{
			EnableMenuItem(WindowMenu, ID_HELP_UPDATES, MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(WindowMenu, ID_HELP_REGISTER, MF_GRAYED | MF_BYCOMMAND);
			} // if
		#endif // !WCS_BUILD_DEMO
		} // if
	} // if

// Add SE script menu
if (!TBScriptMenuAdded)
	{
	// Never add this item in Demo version
	#ifndef WCS_BUILD_DEMO
		{
		if (GlobalApp->Sentinal->CheckAuthSE()) // is scripting enabled?
			{
			if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_DATA))
				{
				MenuItem.cbSize = sizeof(MENUITEMINFO);
				MenuItem.fMask = MIIM_TYPE | MIIM_ID;
				MenuItem.fType = MF_SEPARATOR;
				MenuItem.wID = 0;
				MenuItem.dwTypeData = NULL;
				AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

				MenuItem.fMask = MIIM_TYPE | MIIM_ID;
				MenuItem.fType = MFT_STRING;
				MenuItem.wID = ID_SCRIPT_COMMAND;
				MenuItem.dwTypeData = "&Execute Script Command";
				AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

				MenuItem.fMask = MIIM_TYPE | MIIM_ID;
				MenuItem.fType = MFT_STRING;
				MenuItem.wID = ID_SCRIPT_START;
				MenuItem.dwTypeData = "&Start Script";
				AppendMenu(WindowMenu, MF_STRING, MenuItem.wID, MenuItem.dwTypeData);

				DrawMenuBar(LocalWinSys()->RootWin);
				TBScriptMenuAdded = 1;
				} // if
			} // if
		} // if
	#endif // !WCS_BUILD_DEMO
	} // if

if (! ForestryWizardMenuAdded)
	{
	ForestryWizardMenuAdded = 1;
	#ifdef WCS_FORESTRY_WIZARD
	// Add Forestry Wizard item to Data menu
	if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_DATA))
		{
		MenuItem.cbSize = sizeof(MENUITEMINFO);
		MenuItem.fMask = MIIM_TYPE | MIIM_ID;
		MenuItem.fType = MFT_STRING;
		MenuItem.wID = ID_WINMENU_FORESTWIZ;
		MenuItem.dwTypeData = "Forestry Wizard...";

		InsertMenuItem(WindowMenu, 4, 1, &MenuItem);

		DrawMenuBar(LocalWinSys()->RootWin);
		} // if
	#endif // WCS_FORESTRY_WIZARD
	} // if

if (! GridderWizardMenuAdded)
	{
	GridderWizardMenuAdded = 1;
	// Add Merger Wizard item to Data menu
	if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_DATA))
		{
		MenuItem.cbSize = sizeof(MENUITEMINFO);
		MenuItem.fMask = MIIM_TYPE | MIIM_ID;
		MenuItem.fType = MFT_STRING;
		MenuItem.wID = ID_WINMENU_GRIDDERWIZ;
		MenuItem.dwTypeData = "Gridder Wizard...";

		InsertMenuItem(WindowMenu, 5, 1, &MenuItem);

		DrawMenuBar(LocalWinSys()->RootWin);
		} // if
	} // if

#ifdef WCS_BUILD_VNS
if (! MergerWizardMenuAdded)
	{
	MergerWizardMenuAdded = 1;
	// Add Merger Wizard item to Data menu
	if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_DATA))
		{
		MenuItem.cbSize = sizeof(MENUITEMINFO);
		MenuItem.fMask = MIIM_TYPE | MIIM_ID;
		MenuItem.fType = MFT_STRING;
		MenuItem.wID = ID_WINMENU_MERGERWIZ;
		MenuItem.dwTypeData = "Merger Wizard...";

		InsertMenuItem(WindowMenu, 6, 1, &MenuItem);

		DrawMenuBar(LocalWinSys()->RootWin);
		} // if
	} // if
#endif // WCS_BUILD_VNS

} // Toolbar::RealAddWindowToMenuList

/*===========================================================================*/

void Toolbar::RemoveWindowFromMenuList(GUIFenetre *RemoveMe)
{

RealRemoveWindowFromMenuList(RemoveMe);

} // Toolbar::RemoveWindowFromMenuList

/*===========================================================================*/

void Toolbar::RealRemoveWindowFromMenuList(GUIFenetre *RemoveMe)
{
HMENU WindowMenu;
int Ct;

if (WindowMenu = GetSubMenu((HMENU)MainMenu, WCS_MAINMENUNUMBER_WINDOW))
	{
	for (Ct = 0; Ct < WCS_WINDOWLIST_MAXNUMWINS; Ct ++)
		{
		if (RemoveMe == WindowsInMenuList[Ct])
			{
			DeleteMenu(WindowMenu, WCS_WINDOWLIST_MINID + Ct, MF_BYCOMMAND);
			WindowsInMenuList[Ct] = NULL;
			DrawMenuBar(LocalWinSys()->RootWin);
			break;
			} // if it's in list
		} // for
	} // if

} // Toolbar::RealRemoveWindowFromMenuList

/*===========================================================================*/

void Toolbar::OpenTimeLine(void)
{
RasterAnimHost *CurActive;

if (CurActive = RasterAnimHost::GetActiveRAHost())
	CurActive->EditRAHost();

} // Toolbar::OpenTimeLine

/*===========================================================================*/

void Toolbar::KeyGroupMode(void)
{
GlobalApp->MainProj->SetParam(1, WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_KEYGROUPMODE, 0, GetToolbarButtonPressed(IDI_KEYGROUP), NULL);
} // Toolbar::KeyGroupMode

/*===========================================================================*/

void Toolbar::KeyFrameUpdate(int UpdateAll)
{

} // Toolbar::KeyFrameUpdate

/*===========================================================================*/

// SilentNow indicates don't ank for time, use now moment
void Toolbar::KeyFrameAdd(int SilentNow)
{
RasterAnimHost *CurActive;
RasterAnimHostProperties Prop;

if (CurActive = RasterAnimHost::GetActiveRAHost())
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE;
	CurActive->GetRAHostProperties(&Prop);

	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATABLE)
		{
		double Time, TimeRange;
		if (SilentNow)
			{
			Time = GlobalApp->MainProj->Interactive->GetActiveTime();
			TimeRange = .05 / GlobalApp->MainProj->Interactive->GetFrameRate();
			} // if

		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
			{
			if (SilentNow)
				((AnimDoubleTime *)CurActive)->AddNode(Time, TimeRange);
			else
				((AnimDoubleTime *)CurActive)->AddNode();
			} // if
		else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
			{
			if (SilentNow)
				((AnimColorTime *)CurActive)->AddNode(Time, TimeRange);
			else
				((AnimColorTime *)CurActive)->AddNode();
			} // else
		} // if
	} // if

} // Toolbar::KeyFrameAdd

/*===========================================================================*/

void Toolbar::KeyFrameRemove(void)
{
SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_ITEM_DKG, 0);
} // Toolbar::KeyFrameRemove

/*===========================================================================*/

void Toolbar::PlayMode(void)
{

if (SendMessage(PlayButton, BM_GETCHECK, 0, 0))
	{
	LastFrameRedrawTime = GetSystemTimeFP();
	GlobalApp->AddBGHandler(this);
	CurrentlyPlaying = 1;
	} // if
else
	{
	GlobalApp->RemoveBGHandler(this);
	CurrentlyPlaying = 0;
	} // else

} // Toolbar::PlayMode

/*===========================================================================*/

void Toolbar::KeyFrameScale(void)
{

SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_ITEM_KSG, 0);

} // Toolbar::KeyFrameScale

/*===========================================================================*/

void Toolbar::NextKeyFrame(short Direction)
{
RasterAnimHost *CurActive, *Sib;
RasterAnimHostProperties Prop;
int SiblingsExist;

Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
Prop.NewKeyNodeRange[0] = -DBL_MAX;
Prop.NewKeyNodeRange[1] = DBL_MAX;

if (CurActive = RasterAnimHost::GetActiveRAHost())
	{
	SiblingsExist = (CurActive->RAParent && (Sib = CurActive->RAParent->GetNextGroupSibling(CurActive))
		&& (Sib != CurActive));
	Prop.ItemOperator = SiblingsExist && GlobalApp->MainProj->GetKeyGroupMode() ? WCS_KEYOPERATION_CUROBJGROUP: WCS_KEYOPERATION_CUROBJ;
	CurActive->GetRAHostProperties(&Prop);
	} // if
else
	{
	if (GlobalApp->GUIWins->SAG)
		{
		GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(Prop.TypeNumber);
		if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
			Prop.ItemOperator = WCS_KEYOPERATION_OBJCLASS;
		else
			Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
		} // if
	else
		Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
	GlobalApp->AppEffects->GetRAHostProperties(&Prop);
	} // else

if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
	Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
	Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
if (Direction > 0)
	{
	if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
		GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
	} // if
else
	{
	if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
		GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
	} // else

} // Toolbar::NextKeyFrame

/*===========================================================================*/

void Toolbar::FrameNumber(void)
{
RasterAnimHost *CurActive;
RasterAnimHostProperties Prop;
double CurTime, FrameRate;

FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
FrameADT.SetIncrement(FrameRate > 0.0 ? 1.0 / FrameRate: 1.0 / 30.0);
FrameADT.SetValue(GlobalApp->MainProj->Interactive->GetActiveTime());

ConfigureFI(FrameScrub, NULL, &FrameADT, 1.0, 0.0, 10000000.0, FIOFlag_AnimDouble, NULL, NULL);

# ifdef _WIN32
SendMessage(FrameSlider, SBM_SETPOS, (WPARAM)(GlobalApp->MainProj->Interactive->GetActiveFrame()), (LPARAM)TRUE);
# endif // _WIN32

if (CurActive = RasterAnimHost::GetActiveRAHost())
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_KEYRANGE;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	CurActive->GetRAHostProperties(&Prop);

	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
		SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = (CurTime <= Prop.KeyNodeRange[0]));
		SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = (CurTime >= Prop.KeyNodeRange[1]));
		} // if animated
	else
		{
		SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = true);
		SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = true);
		} // else
	} // if
else
	{
	if (GlobalApp->GUIWins->SAG)
		{
		GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(Prop.TypeNumber);
		if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
			Prop.ItemOperator = WCS_KEYOPERATION_OBJCLASS;
		else
			Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
		} // if
	else
		Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
	Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
	GlobalApp->AppEffects->GetRAHostProperties(&Prop);
	CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
	SetToolbarButtonDisabled(IDI_PREVKEY, Disabled_PrevKey = (CurTime <= Prop.KeyNodeRange[0]));
	SetToolbarButtonDisabled(IDI_NEXTKEY, Disabled_NextKey = (CurTime >= Prop.KeyNodeRange[1]));
	} // else

} // Toolbar::FrameNumber

/*===========================================================================*/

void Toolbar::FrameScroll(void)
{
long LastKey;

LastKey = GetMaxFrame();
#ifdef _WIN32
SendMessage(FrameSlider, SBM_SETRANGE, (WPARAM)0, (LPARAM)(LastKey > 90 ? LastKey: 90));
SendMessage(FrameSlider, SBM_SETPOS, (WPARAM)(GlobalApp->MainProj->Interactive->GetActiveFrame()), (LPARAM)TRUE);
# endif // _WIN32

} // Toolbar::FrameScroll

/*===========================================================================*/

long Toolbar::GetMaxFrame(void)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
GlobalApp->AppEffects->GetRAHostProperties(&Prop);
return ((long)(max(3.0, Prop.KeyNodeRange[1]) * GlobalApp->MainProj->Interactive->GetFrameRate() + .5));	// .5 to round

} // Toolbar::GetMaxFrame

/*===========================================================================*/

double Toolbar::GetRealMaxTime(void)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
GlobalApp->AppEffects->GetRAHostProperties(&Prop);
return(Prop.KeyNodeRange[1]);
} // Toolbar::GetRealMaxTime

/*===========================================================================*/

char Toolbar::GetAvailableGraphNumber(void)
{
char Ct;

for (Ct = 0; Ct < 10; Ct ++)
	{
	if (! GlobalApp->GUIWins->GRG[Ct])
		return (Ct);
	} // for

return (-1);

} // Toolbar::GetAvailableGraphNumber

/*===========================================================================*/

char Toolbar::GetAvailableDragnDropListNumber(void)
{
char Ct;

for (Ct = 0; Ct < 5; Ct ++)
	{
	if (! GlobalApp->GUIWins->DDL[Ct])
		return (Ct);
	} // for

return (-1);

} // Toolbar::GetAvailableDragnDropListNumber

/*===========================================================================*/

void Toolbar::EnforceTaskModeWindowCompliance(long NewMode)
{
long IDCt, WinCt, Compliant, DestroyIt, Instance, TaskModeEnabled;

TaskModeEnabled = GlobalApp->MainProj->Prefs.MatrixTaskModeEnabled;

if (NewMode && TaskModeEnabled)
	{
	for (WinCt = 0; WinCt < WCS_WINDOWLIST_MAXNUMWINS; WinCt ++)
		{
		if (WindowsInMenuList[WinCt])
			{
			Compliant = DestroyIt = 0;
			for (IDCt = 0; TaskModeCompliantWinList[IDCt][NewMode - 1]; IDCt ++)
				{
				if (WindowsInMenuList[WinCt]->FenID == TaskModeCompliantWinList[IDCt][NewMode - 1])
					Compliant = 1;
				} // for
			if (! Compliant)
				{
				for (IDCt = 0; TaskModeIndependentWinList[IDCt]; IDCt ++)
					{
					if (WindowsInMenuList[WinCt]->FenID == TaskModeIndependentWinList[IDCt])
						Compliant = 1;
					} // for
				} // if
			if (! Compliant)
				{
				for (IDCt = 0; TaskModeDestroyWinList[IDCt]; IDCt ++)
					{
					if (WindowsInMenuList[WinCt]->FenID == TaskModeDestroyWinList[IDCt])
						DestroyIt = 1;
					} // for
				if (DestroyIt)
					{
					Instance = 0;
					if (NotifyFromID(WindowsInMenuList[WinCt]->FenID) == WCS_TOOLBAR_ITEM_GRG)
						{
						for ( ; Instance < 9; Instance ++)
							{
							if (WindowsInMenuList[WinCt] == (GUIFenetre *)GlobalApp->GUIWins->GRG[Instance])
								break;
							} // for
						} // if
					else if (NotifyFromID(WindowsInMenuList[WinCt]->FenID) == WCS_TOOLBAR_ITEM_DDL)
						{
						for ( ; Instance < 4; Instance ++)
							{
							if (WindowsInMenuList[WinCt] == (GUIFenetre *)GlobalApp->GUIWins->DDL[Instance])
								break;
							} // for
						} // else if
					HandleModule((unsigned char)NotifyFromID(WindowsInMenuList[WinCt]->FenID), 0, (unsigned char)Instance);
					} // if
				else
					WindowsInMenuList[WinCt]->Hide();
				} // if
			} // if
		} // for
	} // if
for (WinCt = 0; WinCt < WCS_WINDOWLIST_MAXNUMWINS; WinCt ++)
	{
	if (WindowsInMenuList[WinCt])
		{
		Compliant = ! NewMode || ! TaskModeEnabled;
		if (! Compliant)
			{
			for (IDCt = 0; TaskModeCompliantWinList[IDCt][NewMode - 1]; IDCt ++)
				{
				if (WindowsInMenuList[WinCt]->FenID == TaskModeCompliantWinList[IDCt][NewMode - 1])
					Compliant = 1;
				} // for
			} // if
		if (Compliant)
			{
			WindowsInMenuList[WinCt]->Show();
			} // else
		} // if
	} // for

} // Toolbar::EnforceTaskModeWindowCompliance

/*===========================================================================*/

void Toolbar::CloseAllWindowMenuWindows(void)
{
long WinCt, Instance;

for (WinCt = 0; WinCt < WCS_WINDOWLIST_MAXNUMWINS; WinCt ++)
	{
	if (WindowsInMenuList[WinCt])
		{
		Instance = 0;
		if (NotifyFromID(WindowsInMenuList[WinCt]->FenID) == WCS_TOOLBAR_ITEM_GRG)
			{
			for ( ; Instance < 9; Instance ++)
				{
				if (WindowsInMenuList[WinCt] == (GUIFenetre *)GlobalApp->GUIWins->GRG[Instance]->GetThySelf())
					break;
				} // for
			} // if
		else if (NotifyFromID(WindowsInMenuList[WinCt]->FenID) == WCS_TOOLBAR_ITEM_DDL)
			{
			for ( ; Instance < 4; Instance ++)
				{
				if (WindowsInMenuList[WinCt] == (GUIFenetre *)GlobalApp->GUIWins->DDL[Instance]->GetThySelf())
					break;
				} // for
			} // else if
		HandleModule((unsigned char)NotifyFromID(WindowsInMenuList[WinCt]->FenID), 0, (unsigned char)Instance);
		} // if
	} // for

} // Toolbar::CloseAllWindowMenuWindows

/*===========================================================================*/

void Toolbar::NumericValue(RasterAnimHost *RAH)
{
RasterAnimHost *CurActive;
RasterAnimHostProperties Prop;

if (RAH)
	{
	CurActive = RAH;
	} // if
else
	{
	CurActive = RasterAnimHost::GetActiveRAHost();
	} // else

if (CurActive)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	CurActive->GetRAHostProperties(&Prop);

	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
		{
		DONGLE_INLINE_CHECK()
		if (GlobalApp->GUIWins->NUM)
			{
			delete GlobalApp->GUIWins->NUM;
			}
		GlobalApp->GUIWins->NUM = new NumericEntryGUI((AnimDoubleTime *)CurActive);
		if (GlobalApp->GUIWins->NUM)
			{
			GlobalApp->GUIWins->NUM->Open(GlobalApp->MainProj);
			}
		} // if
	} // if

} // Toolbar::NumericValue

/*===========================================================================*/

void Toolbar::RemoveCurrentUser(void)
{
#ifndef WCS_BUILD_DEMO
long NumItems = 0, ItemCt = 0;
char *NameList, filename[512];
FILE *ffile;

if (GlobalApp->MainProj->Prefs.CurrentUserName[0])
	{
	if (UserMessageYN(GlobalApp->MainProj->Prefs.CurrentUserName, "Remove this User from the list of available User Profiles?\nWARNING: This command can not be \"undone!\""))
		{
		strmfp(filename, GlobalApp->GetProgDir(), "WCSUsers.txt");

		// open and count user name file
		if (ffile = fopen(filename, "r"))
			{
			while (1)	//lint !e716
				{
				// read a line from the file
				if (fgets(filename, 64, ffile))
					{
					NumItems ++;
					} // if
				else
					break;
				} // while
			fclose(ffile);
			} // if

		if (NumItems > 0)
			{
			// allocate a name list
			if (NameList = (char *)AppMem_Alloc(NumItems * 64, APPMEM_CLEAR))
				{
				strmfp(filename, GlobalApp->GetProgDir(), "WCSUsers.txt");

				// open and read user name file
				if (ffile = fopen(filename, "r"))
					{
					while (1)	//lint !e716
						{
						// read a line from the file
						if (fgets(&NameList[ItemCt * 64], 64, ffile))
							{
							if (NameList[ItemCt * 64 + strlen(&NameList[ItemCt * 64]) - 1] == '\n')
								NameList[ItemCt * 64 + strlen(&NameList[ItemCt * 64]) - 1] = 0;
							ItemCt ++;
							} // if
						else
							break;
						} // while
					fclose(ffile);
					} // if

				// write the names back out without the removed one
				// open and write user name file
				if (ffile = fopen(filename, "w"))
					{
					for (ItemCt = 0; ItemCt < NumItems; ItemCt ++)
						{
						if (stricmp(&NameList[ItemCt * 64], GlobalApp->MainProj->Prefs.CurrentUserName))
							{
							fputs(&NameList[ItemCt * 64], ffile);
							fputc('\n', ffile);
							} // if
						} // for
					fclose(ffile);
					} // if

				// delete the prefs file
				strmfp(filename, GlobalApp->GetProgDir(), GlobalApp->MainProj->Prefs.CurrentUserName);
				strcat(filename, ".prefs");
				remove(filename);

				// set the current user and multi-user mode
				GlobalApp->MainProj->Prefs.CurrentUserName[0] = 0;
				GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
				GlobalApp->MainProj->Prefs.MultiUserMode = 0;
				} // if
			else
				{
				UserMessageOK("Remove User Profile", "Unable to allocate name list memory.");
				} // else
			} // if at least one name exists
		else
			{
			// delete the prefs file
			strmfp(filename, GlobalApp->GetProgDir(), GlobalApp->MainProj->Prefs.CurrentUserName);
			strcat(filename, ".prefs");
			remove(filename);
			
			// set the current user and multi-user mode
			GlobalApp->MainProj->Prefs.CurrentUserName[0] = 0;
			GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
			GlobalApp->MainProj->Prefs.MultiUserMode = 0;
			} // else no names but remove prefs file anyway
		} // if
	} // if
else
	{
	UserMessageOK("Remove User Profile", "User must be logged on in order for their profile to be removed.");
	} // else

// update title bar
GlobalApp->WinSys->UpdateRootTitle(GlobalApp->MainProj->ProjectLoaded ? GlobalApp->MainProj->projectname: (char*)"", GlobalApp->MainProj->Prefs.CurrentUserName);

#else // WCS_BUILD_DEMO
UserMessageDemo("Multiple-user environment is not supported.");
#endif // WCS_BUILD_DEMO

} // Toolbar::RemoveCurrentUser
