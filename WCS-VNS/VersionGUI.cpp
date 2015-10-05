// VersionGUI.cpp
// Code for Startup window
// Built from VersionGUI.cpp on March 22 2002 by CHX
// Built from ProjectPrefsGUI.cpp on 6/15/96 by Chris "Xenon" Hanson
// Copyright 1996

#undef WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include "AppMem.h"
#include "VersionGUI.h" // will become StartupWin.h
#include "WCSVersion.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "AppHelp.h"
#include "Conservatory.h"
#include "Raster.h"
#include "Random.h"
#include "FontImage.h"
#include "FSSupport.h"
#include "resource.h"

//lint -save -e648

// Splash window image is now loaded from resource via JPEG
#include <setjmp.h>
extern "C" {
#include "jpeglib.h"
}

//long GlobalStartupPage;
long StartupWinOpenToPage;
long VersionGUI::ActiveSubPage;

#include "TipTable.h" // funny way to do this, but trust me.

#define WCS_VERSION_UPDATE_THRESH_DAYS 30

#define WCS_VERSION_BLANK_PANE 5
#define WCS_VERSION_TUT_PANE 1
#define WCS_VERSION_EXISTPROJ_PANE 2
#define WCS_VERSION_NEWPROJ_PANE 3
#define WCS_VERSION_SPLASH_SECONDS 7

char *VersionGUI::TabNames[WCS_STARTUPGUINEWPAGE_NUMTABS] = {"Summary", "Terrain", "Ground", "Environment", "Sky", "Clouds"};
char *FileExt[WCS_STARTUPGUINEWPAGE_NUMTYPES] = {".tgn", ".gnd", ".env", ".sky", ".cld"};
int EffectSubClass[WCS_STARTUPGUINEWPAGE_NUMTYPES] = {WCS_EFFECTSSUBCLASS_GENERATOR, WCS_EFFECTSSUBCLASS_GROUND, WCS_EFFECTSSUBCLASS_ENVIRONMENT, WCS_EFFECTSSUBCLASS_SKY, WCS_EFFECTSSUBCLASS_CLOUD};
char *FileCompPaths[WCS_STARTUPGUINEWPAGE_NUMTYPES] = {"WCSContent:Terrain", "WCSContent:LandCover", "WCSContent:LandCover", "WCSContent:Sky", "WCSContent:Sky"};

NativeGUIWin VersionGUI::Open(Project *Moi)
{
NativeGUIWin Success;
RECT Center;
if (Success = GUIFenetre::Open(Moi))
	{
	// center ourselves
	GetTime(OpenMoment);
	GetClientRect(NativeWin, &Center);
	SetWindowPos(NativeWin, NULL, (LocalWinSys()->InquireDisplayWidth() / 2) - (Center.right / 2), (LocalWinSys()->InquireDisplayHeight() / 2) - (Center.bottom / 2), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	GlobalApp->MCP->AddWindowToMenuList(this);
	PickAndSetTip(-1);
	InstallBG();
	} // if

return (Success);

} // VersionGUI::Open

/*===========================================================================*/

NativeGUIWin VersionGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_STARTUP_MAIN, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_STARTUP_TIPS, 0, 0, false);
	CreateSubWinFromTemplate(IDD_STARTUP_TUTORIALS, 0, 1, false);
	CreateSubWinFromTemplate(IDD_STARTUP_EXISTPROJ, 0, 2, false);
	CreateSubWinFromTemplate(IDD_STARTUP_NEWPROJ, 0, 3, false);
	CreateSubWinFromTemplate(IDD_STARTUP_USERS, 0, 4, false);
	CreateSubWinFromTemplate(IDD_STARTUP_BLANK, 0, 5, false);

	for (TabIndex = 0; TabIndex < WCS_STARTUPGUINEWPAGE_NUMTABS; TabIndex ++)
		{
		WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
		} // for

	if(NativeWin)
		{
		// setup a larger italic font
		TipTextFont = CreateFont(20, 0, 0, 0, FW_NORMAL, 1, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /* CLEARTYPE_QUALITY */ , DEFAULT_PITCH | FF_SWISS, "arial");
		SendMessage(GetWidgetFromID(IDC_TEXTBOX), WM_SETFONT, (WPARAM)TipTextFont, 1);

/*
// We'd have liked this to work:
    CTEXT           "",IDC_TEXTBOX,0,0,279,201,WS_BORDER|SS_CENTERIMAGE|SS_CENTER,WS_EX_TRANSPARENT

// but we settled on this:
    CTEXT           "",IDC_STATIC,0,0,279,201,WS_BORDER,
    CTEXT           "",IDC_TEXTBOX,15,65,249,120,SS_CENTER,WS_EX_TRANSPARENT
*/
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // VersionGUI::Construct

/*===========================================================================*/

VersionGUI::VersionGUI()
: GUIFenetre('VERS', this, "")
{
int Ct;
TipTextFont = NULL;
OpenMoment = 0;
MadeTutColumns = JustRecentConfigured = 0;
SerialStr[0] = NULL;
LogoRedBuf = LogoGrnBuf = LogoBluBuf = NULL;
LogoW = LogoH = 0;
#ifdef WCS_BUILD_DEMO
SerialStr[0] = 0;
#else // !WCS_BUILD_DEMO
if((GlobalApp->Sentinal) && (GlobalApp->Sentinal->CheckSerial()))
	{
	sprintf(SerialStr, "Serial: 0x%08x", GlobalApp->Sentinal->CheckSerial());
	} // if
#endif // !WCS_BUILD_DEMO

LastTip = -1;
UpdateTip = UpdateWarned = 0;
NameChanged = 0;
BackgroundInstalled = 0;
strcpy(DefaultExt, "proj");
//strcpy(ActiveDir, (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->projectpath[0]) ? GlobalApp->MainProj->projectpath: "WCSProjects:");
strcpy(ActiveDir, "WCSProjects:");

for (Ct = 0; Ct < WCS_STARTUPGUINEWPAGE_NUMTYPES; Ct ++)
	{
	CCD[Ct] = NULL;
	NumValidComps[Ct] = NULL;
	NumCompRanges[Ct] = NULL;
	CompActiveRange[Ct] = 0;
	} // for

NumItems = NumValidItems = 0;
NameList = NULL;
ActiveRange = NumRanges = ActiveItem = 0;
RasterList = CompRasterList = NULL;
BrowseList = NULL;
FirstItemInRange = 0;
LastItemInRange = 0;
ScrollPos = 0;
memset(&DisplayedItem[0], 0, WCS_VERSIONGUI_NUMNAILS * sizeof (long));

RasterList = new Raster[WCS_VERSIONGUI_NUMNAILS];

// GlobalStartupPage should be in project somewhere
ActivePage = (GlobalApp && GlobalApp->MainProj) ? GlobalApp->MainProj->Prefs.GlobalStartupPage: 0;
if(StartupWinOpenToPage != 0)
	{
	// Overridden by opener
	ActivePage = StartupWinOpenToPage;
	// will be cleared in first configurewidgets
	} // if

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP | WCS_FENETRE_WINMAN_SMTITLE);

CurTutorialNum = -1;
} // VersionGUI::VersionGUI

/*===========================================================================*/

VersionGUI::~VersionGUI()
{
long Ct;

ClearComponents();
ClearProjects();

for (Ct = 0; Ct < WCS_STARTUPGUINEWPAGE_NUMTYPES; Ct ++)
	{
	SelectedCompRaster[Ct].Thumb = NULL;
	} // for
if(CompRasterList)
	{
	for (Ct = 0; Ct < WCS_VERSIONGUI_NUMNAILS; Ct ++)
		{
		CompRasterList[Ct].Thumb = NULL;
		} // for
	} // if
if(CompRasterList)
	{
	delete [] CompRasterList;
	} // if
CompRasterList = NULL;

if (RasterList)
	{
	for (Ct = 0; Ct < WCS_VERSIONGUI_NUMNAILS; Ct ++)
		RasterList[Ct].Thumb = NULL;
	delete [] RasterList;
	} // if
RasterList = NULL;

UninstallBG(); // so it doesn't call us after we close...

IconImage.R = NULL;
IconImage.G = NULL;
IconImage.B = NULL;

UnloadLogo();

GlobalApp->MCP->RemoveWindowFromMenuList(this);

if(TipTextFont) DeleteObject(TipTextFont);

} // VersionGUI::~VersionGUI()

/*===========================================================================*/

void VersionGUI::ClearProjects(void)
{
long Ct;

if (NameList)
	{
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if (NameList[Ct])
			AppMem_Free(NameList[Ct], strlen(NameList[Ct]) + 1);
		} // for
	AppMem_Free(NameList, NumItems * sizeof (char *));
	NameList = NULL;
	} // if

if (BrowseList)
	delete [] BrowseList;
BrowseList = NULL;

NumItems = NumValidItems = 0;
NumRanges = 0;
FirstItemInRange = 0;
LastItemInRange = 0;
ActiveItem = 0;
ScrollPos = 0;
memset(&DisplayedItem[0], 0, WCS_VERSIONGUI_NUMNAILS * sizeof (long));

} // VersionGUI::ClearProjects

/*===========================================================================*/

void VersionGUI::ClearComponents(void)
{
long Ct;
CombinedComponentData *Scan, *NextScan = NULL;

// NULL out names
for (Ct = 0; Ct < WCS_STARTUPGUINEWPAGE_NUMTYPES; Ct ++)
	{
	SelectedCompNames[Ct][0] = NULL;

	// free linked lists of CombinedComponentData for each type
	for(Scan = CCD[Ct]; Scan; Scan = NextScan)
		{
		NextScan = Scan->Next;
		delete Scan;
		} // for
	CCD[Ct] = NULL;
	} // for

} // VersionGUI::ClearComponents

/*===========================================================================*/

long VersionGUI::HandleCloseWin(NativeGUIWin NW)
{

CaptureUserData();
LoadPrefs();
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_VER, 0);

return(0);

} // VersionGUI::HandleCloseWin

/*===========================================================================*/

long VersionGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case IDC_NEXTTIP:
		{
			DoOSVersion();
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT))
			{
			GlobalApp->HelpSys->OpenHelpTopic('TIPS');
			} // else
		else
			{
			PickAndSetTip(LastTip + 1);
			} // else
		break;
		} // IDC_NEXTTIP
	case IDC_MOREINFO:
		{
		if (LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) && LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_CONTROL))
			{
			DoOSVersion();
			} // else
		else
			{
			TipInfo(LastTip);
			} // else
		break;
		} // IDC_NEXTTIP
	case ID_RESUME:
		{
		CaptureUserData();
		ShowWindow(NativeWin, SW_HIDE);
		LoadPrefs();
		AppScope->MainProj->LoadResumeState(GlobalApp->GetProgDir());
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		break;
		} // RESUME
	case ID_OK:
	case ID_KEEP:
	case IDCANCEL:
		{
		CaptureUserData();
		LoadPrefs();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_VER, 0);
		break;
		} // 
	case IDC_TIPS:
		{
		SelectPage(0);
		break;
		} // IDC_TIPS
	case IDC_TUTORIALS:
		{
		SelectPage(1);
		break;
		} // IDC_TUTORIALS
	case IDC_EXISTPROJ:
		{
		SelectPage(2);
		break;
		} // IDC_EXISTPROJ
	case IDC_NEWPROJ:
		{
		SelectPage(3);
		break;
		} // IDC_NEWPROJ
	case IDC_USERS:
		{
		SelectPage(4);
		break;
		} // IDC_USERS
	case IDC_IMAGEWCS:
		{
		UninstallBG();
		HideSplash();
		break;
		} // IDC_IMAGEWCS
	case IDC_TNAIL1:
		{
		LaunchTutorial(CurTutorialNum);
		break;
		} // IDC_IMAGEWCS
	case IDC_EPTNAIL1:
		{
		SetActiveProject(0);
		break;
		} // 
	case IDC_EPTNAIL2:
		{
		SetActiveProject(1);
		break;
		} // 
	case IDC_EPTNAIL3:
		{
		SetActiveProject(2);
		break;
		} // 
	case IDC_EPTNAIL4:
		{
		SetActiveProject(3);
		break;
		} // 
	case IDC_EPTNAIL5:
		{
		SetActiveProject(4);
		break;
		} // 
	case IDC_EPTNAIL6:
		{
		SetActiveProject(5);
		break;
		} // 
	case IDC_NPTNAIL1:
		{
		if(ActiveSubPage == 0)
			{
			HandleSubPageChange(1); // go to terrain page
			} // if
		else
			{
			SetActiveComponentByIconNum(0);
			} // else
		break;
		} // 
	case IDC_NPTNAIL2:
		{
		if(ActiveSubPage == 0)
			{
			HandleSubPageChange(2); // go to ground page
			} // if
		else
			{
			SetActiveComponentByIconNum(1);
			} // else
		break;
		} // 
	case IDC_NPTNAIL3:
		{
		if(ActiveSubPage == 0)
			{
			HandleSubPageChange(3); // go to 
			} // if
		else
			{
			SetActiveComponentByIconNum(2);
			} // else
		break;
		} // 
	case IDC_NPTNAIL4:
		{
		if(ActiveSubPage == 0)
			{
			HandleSubPageChange(4); // go to
			} // if
		else
			{
			SetActiveComponentByIconNum(3);
			} // else
		break;
		} // 
	case IDC_NPTNAIL5:
		{
		if(ActiveSubPage == 0)
			{
			HandleSubPageChange(5); // go to
			} // if
		else
			{
			SetActiveComponentByIconNum(4);
			} // else
		break;
		} // 
	case IDC_NPTNAIL6:
		{
		if(ActiveSubPage == 0)
			{
			// can't do this
			} // if
		else
			{
			SetActiveComponentByIconNum(5);
			} // else
		break;
		} // 
	case IDC_PAGEUP:
		{
		NewProjectRange(1);
		break;
		} // IDC_PAGEUP
	case IDC_PAGEDOWN:
		{
		if(JustRecentConfigured)
			{
			// configure with all projects
			ConfigureProjects(0);
			} // if
		NewProjectRange(-1);
		break;
		} // IDC_PAGEDOWN
	case IDC_NPAGEUP:
		{
		NewComponentRange(ActiveSubPage - 1, 1);
		break;
		} // IDC_PAGEUP
	case IDC_NPAGEDOWN:
		{
		NewComponentRange(ActiveSubPage - 1, -1);
		break;
		} // IDC_PAGEDOWN
	case IDC_CREATE:
		{
		CreateNewProj();
		break;
		} // CREATE new project
	case IDC_RANDOMIZE:
		{
		RandomizeSelection();
		ConfigureSummaryImages();
		break;
		} // CREATE new project
	default:
		break;
	} // ButtonID

return(0);

} // VersionGUI::HandleButtonClick

/*===========================================================================*/

long VersionGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_EPTNAIL1:
		{
		CaptureActiveProject(0);
		break;
		} // 
	case IDC_EPTNAIL2:
		{
		CaptureActiveProject(1);
		break;
		} // 
	case IDC_EPTNAIL3:
		{
		CaptureActiveProject(2);
		break;
		} // 
	case IDC_EPTNAIL4:
		{
		CaptureActiveProject(3);
		break;
		} // 
	case IDC_EPTNAIL5:
		{
		CaptureActiveProject(4);
		break;
		} // 
	case IDC_EPTNAIL6:
		{
		CaptureActiveProject(5);
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // VersionGUI::HandleButtonDoubleClick

/*===========================================================================*/

long VersionGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_USERDROP:
		{
		SelectNewUser();
		break;
		}
	} // switch CtrlID

return (0);

} // VersionGUI::HandleCBChange

/*===========================================================================*/

long VersionGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKSHOWTHISPANE:
		{
		if(ShowThisPage)
			{
			GlobalApp->MainProj->Prefs.GlobalStartupPage = ActivePage;
			} // if
		else
			{
			GlobalApp->MainProj->Prefs.GlobalStartupPage = 0; // show Tips instead
			} // else
		break;
		} // IDC_CHECKSHOWTHISPANE
	case IDC_CHECKMULTIUSER:
		{
		if (GlobalApp->MainProj->Prefs.MultiUserMode)
			ConfigureMultiUser();
		DisableWidgets();
		break;
		} // 
	} // switch CtrlID

return(0);

} // VersionGUI::HandleSCChange

/*===========================================================================*/

long VersionGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

/*
switch(CtrlID)
	{
	case IDC_ACTIVEDIR:
		{
		ConfigureProjects();
		break;
		} // IDC_ACTIVEDIR
	default: break;
	} // ID
*/
return(0);

} // VersionGUI::HandleDDChange


/*===========================================================================*/

long VersionGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		HandleSubPageChange(NewPageID);
		break;
		}
	} // switch

return(0);

} // VersionGUI::HandlePageChange

/*===========================================================================*/

long VersionGUI::HandleSubPageChange(long NewPageID)
{

if(NewPageID == 0)
	{ //case 0: // summary
	WidgetShow(IDC_NPAGEUP, 0);
	WidgetShow(IDC_NPAGEDOWN, 0);
	WidgetShow(IDC_NPAGEUPLABEL, 0);
	WidgetShow(IDC_NPAGEDOWNLABEL, 0);

	WidgetShow(IDC_RANDOMIZE, 1);
	WidgetShow(IDC_NEWPROJNAMELABEL, 1);
	WidgetShow(IDC_NEWPROJNAMEEDIT, 1);
	WidgetShow(IDC_NPCOMMENT, 0);

	WidgetShow(IDC_NPTNAIL6, 0);
	WidgetShow(IDC_NPTITLE6, 0);
	} // 1
else
	{
	/*
	case 1: // Terrain
	case 2: // Ground
	case 3: // Environment
	case 4: // Sky
	case 5: // Clouds
	*/
	WidgetShow(IDC_NPAGEUP, 1);
	WidgetShow(IDC_NPAGEDOWN, 1);
	WidgetShow(IDC_NPAGEUPLABEL, 1);
	WidgetShow(IDC_NPAGEDOWNLABEL, 1);

	WidgetShow(IDC_RANDOMIZE, 0);
	WidgetShow(IDC_NEWPROJNAMELABEL, 0);
	WidgetShow(IDC_NEWPROJNAMEEDIT, 0);
	WidgetShow(IDC_NPCOMMENT, 1);

	WidgetShow(IDC_NPTNAIL6, 1);
	WidgetShow(IDC_NPTITLE6, 1);
	} // 1

WidgetTCSetCurSel(IDC_TAB1, NewPageID);
ActiveSubPage = NewPageID;

// clear comment field
WidgetSetText(IDC_NPCOMMENT, "");

if(ActiveSubPage == 0) // summary page
	{
	ConfigureSummaryImages();
	} // if
else
	{
	ConfigureComponentImages(ActiveSubPage - 1);
	} // else


return(0);

} // VersionGUI::HandleSubPageChange

/*===========================================================================*/


long VersionGUI::HandleBackgroundCrunch(int Siblings)
{
time_t NowMoment;

GetTime(NowMoment);
if(NowMoment - OpenMoment > WCS_VERSION_SPLASH_SECONDS)
	{ // Done
	HideSplash();
	return(1); // self-remove
	} // if

return(0);
} // VersionGUI::HandleBackgroundCrunch

/*===========================================================================*/

long VersionGUI::HandleEvent(void)
{
short ButtonID;
unsigned long Notify;

if(Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
	{
	switch(Activity->GUIMessage.message)
		{
		case WM_NOTIFY:
			{
			NMHDR *nmhdr;
			ButtonID  = LOWORD(Activity->GUIMessage.wParam);
			nmhdr    = (NMHDR *)Activity->GUIMessage.lParam;
			Notify    = nmhdr->code;

			//if(Activity->GUIMessage.hwnd == NativeWin)
				{
				if(ButtonID == IDC_TUTORIALTREE)
					{
					if ((Notify == NM_CLICK) || (Notify == NM_DBLCLK))
						{
						NativeControl ListViewWidget;
						if(ListViewWidget = GetWidgetFromID(IDC_TUTORIALTREE))
							{
							if(Notify == NM_CLICK)
								{
								SelectTutorial(SendMessage(ListViewWidget, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_ALL | LVNI_SELECTED));
								} // if
							else if(Notify == NM_DBLCLK)
								{
								LaunchTutorial(SendMessage(ListViewWidget, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_ALL | LVNI_SELECTED));
								} // if
							} // if
						return(0);
						} // if
					} // if
				} // if
			break;
			} // WM_NOTIFY
		} // switch
	} // else

return(0);

} // VersionGUI::HandleEvent

/*===========================================================================*/
void VersionGUI::SelectPage(int NewPage)
{

ActivePage = NewPage;
ShowPanel(0, NewPage);

if(NewPage == GlobalApp->MainProj->Prefs.GlobalStartupPage)
	{
	ShowThisPage = 1;
	} // if
else
	{
	ShowThisPage = 0;
	} // else

ShowPageWidgets(0);

WidgetShow(IDC_CHECKSHOWTHISPANE, !(NewPage == 0)); // only show if not tips page
ConfigureSC(NativeWin, IDC_CHECKSHOWTHISPANE, &ShowThisPage, SCFlag_Short, NULL, 0);

if(NewPage == WCS_VERSION_TUT_PANE) // tutorials
	{
	ScanTutorials();
	ConfigureTutorials();
	if(HFI.GetNumHelpFiles())
		{
		SelectTutorial(HFI.FindHighestPriorityNum());
		} // if
	else
		{
		SelectTutorial(-1); // clear fields
		} // else
	} // if
else if(NewPage == WCS_VERSION_EXISTPROJ_PANE)
	{
	ShowPageWidgets(1);
	ConfigureProjects(1); // just configure recent projects when first going to page
	} // else if
else if(NewPage == WCS_VERSION_NEWPROJ_PANE)
	{
	HandleSubPageChange(0); // show summary
	ConfigureNewProj();
	RandomizeSelection();
	ConfigureSummaryImages();

	} // else if

} // VersionGUI::SelectPage

/*===========================================================================*/

void VersionGUI::ShowPageWidgets(int ShowWidgets)
{

WidgetShow(IDC_PAGEUPLABEL, ShowWidgets);
WidgetShow(IDC_PAGEDOWNLABEL, ShowWidgets);
WidgetShow(IDC_PAGEUP, ShowWidgets);
WidgetShow(IDC_PAGEDOWN, ShowWidgets);

} // VersionGUI::ShowPageWidgets


/*===========================================================================*/


void VersionGUI::ConfigureWidgets(void)
{
# ifdef _WIN32
HWND MyTB;
struct ToolButtonConfig *SlotIcon;
RECT CheckSize;
int XDif, YDif;
# endif // _WIN32

#ifdef WCS_BUILD_VNS
WidgetShow(IDC_USERS, 1);
#endif // WCS_BUILD_VNS

LoadLogo();

# ifdef _WIN32
MyTB = GetWidgetFromID(IDC_IMAGEWCS);
/*
if(!MyTB)
	{ // try to allocate Splash image
	GetClientRect(NativeWin, &CheckSize);
	//MyTB = CreateWindow(APP_CLASSPREFIX ".ToolButton", "Splash", WS_TABSTOP | 0x6, 0, 0, CheckSize.right, CheckSize.bottom, NativeWin, (HMENU)IDC_IMAGEWCS, GlobalApp->WinSys->Instance(), NULL);
	} // if
*/
if(!MyTB) return;

GetClientRect(NativeWin, &CheckSize);
XDif = CheckSize.right - LogoW;
YDif = CheckSize.bottom - LogoH;
XDif /= 2;
YDif /= 2;
SetWindowPos(MyTB, HWND_TOP, XDif, YDif, LogoW, LogoH, NULL);

# endif // _WIN32

strcpy(VersionAppend, ExtLabelVersion);

#ifndef WCS_BUILD_DEMO
if(GlobalApp->Sentinal->CheckAuthSE())
	{
	strcat(VersionAppend, " SE");
	} // if
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_RTX
//strcat(VersionAppend, " RTX");
#endif // WCS_BUILD_RTX

strcpy(BuildStr, ExtAboutBuild);

// don't show splash if invoked as something else
if(StartupWinOpenToPage == 0)
	{
	CustomizeSplash(1);
	ShowSplash();
	} // if
else
	{
	StartupWinOpenToPage = 0; // clear it for next call
	HideSplash();
	} // else

# ifdef _WIN32
IconImage.Width = LogoW;
IconImage.Height = LogoH;


if(MyTB && LogoW && LogoH)
	{
	IconImage.R = (char *)LogoRedBuf;
	IconImage.G = (char *)LogoGrnBuf;
	IconImage.B = (char *)LogoBluBuf;
	if(SlotIcon = (struct ToolButtonConfig *)GetWindowLong(MyTB, 0))
		{
		SlotIcon->Normal = (HICON)&IconImage;
		SlotIcon->State = 1;
		} // if
	InvalidateRect(MyTB, NULL, NULL);
	} // if
else
	{
	if(SlotIcon = (struct ToolButtonConfig *)GetWindowLong(MyTB, 0))
		{
		SlotIcon->Normal = NULL;
		} // if
	} // else
# endif // _WIN32

ConfigureTB(NativeWin, IDC_PAGEUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_PAGEDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_NPAGEUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_NPAGEDOWN, IDI_ARROWDOWN, NULL);

// multi-user config
#ifdef WCS_BUILD_VNS
ConfigureSC(NativeWin, IDC_CHECKMULTIUSER, &GlobalApp->MainProj->Prefs.MultiUserMode, SCFlag_Short, NULL, 0);
if (GlobalApp->MainProj->Prefs.MultiUserMode)
	ConfigureMultiUser();
#endif // WCS_BUILD_VNS
// defer ConfigureProjects until we switch to that pane
//ConfigureProjects();
DisableWidgets();

} // VersionGUI::ConfigureWidgets()

/*===========================================================================*/

void VersionGUI::ConfigureMultiUser(void)
{
FILE *ffile;
char filename[512];
long Found, Pos;

//WidgetSetText(IDC_PASSWORD, GlobalApp->MainProj->Prefs.CurrentPassword);
//WidgetSetModified(IDC_PASSWORD, FALSE);
WidgetCBClear(IDC_USERDROP);
WidgetCBInsert(IDC_USERDROP, -1, "New User...");

strmfp(filename, GlobalApp->GetProgDir(), "WCSUsers.txt");

// open and read user name file
Found = -1;
if (ffile = fopen(filename, "r"))
	{
	while (1)	//lint !e716
		{
		// read a line from the file
		if (fgets(filename, 64, ffile))
			{
			if (filename[strlen(filename) - 1] == '\n')
				filename[strlen(filename) - 1] = 0;
			Pos = WidgetCBInsert(IDC_USERDROP, -1, filename);
			// add name to combo box
			if (! stricmp(filename, GlobalApp->MainProj->Prefs.CurrentUserName))
				Found = Pos;
			} // if
		else
			break;
		} // while
	fclose(ffile);
	} // if

WidgetCBSetCurSel(IDC_USERDROP, Found);
NameChanged = 0;

} // VersionGUI::ConfigureMultiUser


/*===========================================================================*/


#define WCS_VERSION_TUTORIAL_COLUMNS 5
char *TutorialColumnNames[] = {"Title", "Brief", "Level", "Product", "Set"};
char *TutorialColumnMinWidthStrs[] = {"Slightly Longer", "Sort of descriptive brief text", "Level", "Product", "Somewhat Longer"};

void VersionGUI::ConfigureTutorials(void)
{
int MaxTuts, SubItemCt, ItemCt, Width, MinWidth;
LV_COLUMN ColumnData;
LV_ITEM ItemData;
NativeControl TreeWidget;
HelpFileMetaData *Me;

ColumnData.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
ColumnData.cx = 20;

if(TreeWidget = GetWidgetFromID(IDC_TUTORIALTREE))
	{
	(void)ListView_DeleteAllItems(TreeWidget);
	if(!MadeTutColumns)
		{
		// delete any exsting columns
		for (SubItemCt = 0; SubItemCt <= WCS_VERSION_TUTORIAL_COLUMNS; SubItemCt ++)
			{
			(void)ListView_DeleteColumn(TreeWidget, SubItemCt);
			} // for
		// add columns
		for (SubItemCt = 0; SubItemCt < WCS_VERSION_TUTORIAL_COLUMNS; SubItemCt ++)
			{
			ColumnData.pszText = TutorialColumnNames[SubItemCt];
			ColumnData.iSubItem = SubItemCt;
			if ((Width = SendMessage(TreeWidget, LVM_GETSTRINGWIDTH, 0, (LPARAM)TutorialColumnNames[SubItemCt])) <= 0)
				Width = 20;
			if ((MinWidth = SendMessage(TreeWidget, LVM_GETSTRINGWIDTH, 0, (LPARAM)TutorialColumnMinWidthStrs[SubItemCt])) <= 0)
				MinWidth = 20;
			Width = max(Width, MinWidth);
			Width += 15;
			ColumnData.cx = Width;
			SendMessage(TreeWidget, LVM_INSERTCOLUMN, SubItemCt, (LPARAM)&ColumnData);
			} // for
		MadeTutColumns = 1;
		} // if

	// this makes adding large numbers of items more efficient
	SendMessage(TreeWidget, LVM_SETITEMCOUNT, HFI.GetNumHelpFiles(), 0);

	if(MaxTuts = HFI.GetNumHelpFiles())
		{
		if(Me = HFI.GetFirst())
			{
			for (ItemCt = 0; Me && ItemCt < MaxTuts; ItemCt ++)
				{
				ItemData.mask = LVIF_TEXT;
				ItemData.iItem = ItemCt;
				ItemData.iSubItem = 0;
				ItemData.lParam = ItemCt;
				ItemData.pszText = (char *)Me->GetTitle();
				SendMessage(TreeWidget, LVM_INSERTITEM, 0, (LPARAM)&ItemData);

				ItemData.iSubItem = 1;
				ItemData.pszText = (char *)Me->GetBrief();
				SendMessage(TreeWidget, LVM_SETITEMTEXT, ItemCt, (LPARAM)&ItemData);

				ItemData.iSubItem = 2;
				ItemData.pszText = (char *)Me->GetLevel();
				SendMessage(TreeWidget, LVM_SETITEMTEXT, ItemCt, (LPARAM)&ItemData);

				ItemData.iSubItem = 3;
				ItemData.pszText = (char *)Me->GetProduct();
				SendMessage(TreeWidget, LVM_SETITEMTEXT, ItemCt, (LPARAM)&ItemData);

				ItemData.iSubItem = 4;
				ItemData.pszText = (char *)Me->GetSet();
				SendMessage(TreeWidget, LVM_SETITEMTEXT, ItemCt, (LPARAM)&ItemData);

				Me = HFI.GetNext(Me);
				} // for
			} // if
		} // if
	} // if

} // VersionGUI::ConfigureTutorials

/*===========================================================================*/


int VersionGUI::SelectTutorial(int TutorialNum)
{
HelpFileMetaData *Selected = NULL;
int ClearAll = 1;

CurTutorialNum = TutorialNum;

if(TutorialNum != -1)
	{
	if(Selected = HFI.GetNum(TutorialNum))
		{
		WidgetSetText(IDC_EDIT1, Selected->GetAbstract());
		WidgetSetText(IDC_TITLE1, Selected->GetTitle());
		WidgetSetText(IDC_TNAIL1, Selected->GetBrief());
		ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, Selected->GetThumbRaster());
		ClearAll = 0;
		} // if
	} // if

if(ClearAll)
	{
	// clear selection
	WidgetSetText(IDC_EDIT1, "");
	WidgetSetText(IDC_TITLE1, "");
	WidgetSetText(IDC_TNAIL1, "");
	ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, NULL);
	} // if

return(0);
} // VersionGUI::SelectTutorial

/*===========================================================================*/

int VersionGUI::LaunchTutorial(int TutorialNum)
{
HelpFileMetaData *Selected = NULL;
int ClearAll = 1;

if(TutorialNum != -1)
	{
	if(Selected = HFI.GetNum(TutorialNum))
		{
		return(Selected->Launch());
		} // if
	} // if

return(0);
} // VersionGUI::LaunchTutorial

/*===========================================================================*/

class DirScanStackItem
	{
	public:
		DirScanStackItem *Next;
		char DirName[512];

		DirScanStackItem() {DirName[0] = NULL; Next = NULL;};
	}; // DirScanStackItem

void VersionGUI::ConfigureProjects(int JustRecent)
{
char NameStr[512], TempStr[512];
int Ct, Depth = 0, Pass, OneShot, DoneDirs;
WIN32_FIND_DATA FileData;
HANDLE Hand = NULL;

DirScanStackItem *DirStack = NULL, *EndStack = NULL, *ScanStack = NULL, *ThisStack;

ClearProjects();
ClearComponents();

NumItems = 0;

if (RasterList && ActiveDir[0] && DefaultExt[0])
	{
	for(Pass = 0; Pass < 2; Pass++)
		{
		ScanStack = ThisStack = NULL;
		DoneDirs = 0;
		// allocate on second pass
		if(Pass == 1)
			{
			if(NumItems > 0)
				{
				NameList = (char **)AppMem_Alloc(NumItems * sizeof (char *), APPMEM_CLEAR);
				BrowseList = new BrowseData[NumItems];
				if(!(NameList && BrowseList))
					{
					// bail out
					ClearProjects();
					break;
					} // if
				} // if
			else
				{
				break; // nothing to load, bail out of passes
				} // else
			} // if

		NumValidItems = 0;

		// add last six recent projects
		for (Ct = 0; Ct < 6; Ct ++)
			{
			if (GlobalApp->MainProj->LastProject[Ct][0])
				{
				if(Pass == 0)
					{
					// just count them
					NumItems ++;
					} // if
				else
					{ // add them to the lists
					strcpy(TempStr, GlobalApp->MainProj->LastProject[Ct]);
					//StripExtension(TempStr);
					if (NameList[NumValidItems] = (char *)AppMem_Alloc(strlen(TempStr) + 1, APPMEM_CLEAR))
						{
						strcpy(NameList[NumValidItems], TempStr);
						if (BrowseList[NumValidItems].LoadBrowseInfo(GlobalApp->MainProj->LastProject[Ct]))
							{
							NumValidItems ++;
							} // if
						else
							{
							AppMem_Free(NameList[NumValidItems], strlen(TempStr) + 1);
							NameList[NumValidItems] = NULL;
							} // else
						} // if
					else
						{
						break;
						} // else
					} // else
				} // if
			} // for

		if(!JustRecent)
			{
			// Heirarchical loop
			for(DoneDirs = 0; DoneDirs < 2;)
				{
				if(DoneDirs == 0)
					{
					strmfp(NameStr, GlobalApp->MainProj->MungPath(ActiveDir), "*");
					if(Pass == 1)
						{
						// setup for scanning
						ScanStack = DirStack;
						} // if
					DoneDirs++;
					} // if
				else
					{
					// are there listed dirs still available to scan?
					if(ScanStack)
						{
						Depth = 1; // stop adding dirs as we encounter them, as we're now using them
						strmfp(NameStr, GlobalApp->MainProj->MungPath(ActiveDir), ScanStack->DirName);
						ThisStack = ScanStack;
						// need trailing '*'
						strcat(NameStr, "/*");
						ScanStack = ScanStack->Next;
						} // if
					else
						{
						DoneDirs++; // hit 2 and stop
						} // else
					} // else
				// dirscan loop
				for(OneShot = 1; ; ) // exit via break
					{
					if(OneShot)
						{
						if(Hand != NULL)
							{ // close previous handle
							FindClose(Hand);
							Hand = NULL;
							} // if
						if ((Hand = FindFirstFile(NameStr, &FileData)) == INVALID_HANDLE_VALUE) //lint !e645
							{
							break; // bail out of dirscan loop
							} // if
						OneShot = 0;
						} // if
					else
						{
						if(!(FindNextFile(Hand, &FileData)))
							{
							break; // bail out of dirscan loop
							} // if
						} // else
					// Is it a directory or a file?
					if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{ // dir
						// ignore "." and ".." and anythign that begins with "."
						if(FileData.cFileName[0] != '.')
							{
							// limit recursion to one level, aka
							// WCSProjects:Blah.proj, WCSProjects:MyProject\MyProject.proj
							// WCSProjects:Demos/Foo.proj but not WCSProjects:Demos/Foo/Foo.proj
							if((Pass == 0) && (Depth == 0)) // only build list on first pass, just use it on second
								{
								DirScanStackItem *DSSI;
								if(DSSI = new DirScanStackItem)
									{
									strncpy(DSSI->DirName, FileData.cFileName, 500);
									DSSI->DirName[500] = NULL;
									
									// append to end of stack
									if(DirStack == NULL)
										{
										DirStack = DSSI;
										} // if
									if(EndStack != NULL)
										{
										EndStack->Next = DSSI;
										} // if
									EndStack = DSSI;
									ScanStack = DirStack;
									} // if
								} // if
							} // if
						} // if
					else
						{ // file
						// does it end in ".proj"?
						if((strlen(FileData.cFileName) > 5) && !stricmp(".proj", &FileData.cFileName[strlen(FileData.cFileName) - 5]))
							{
							if(Pass == 0)
								{ // just count
								NumItems ++;
								} // if
							else
								{
								char FullPathAssemble[1024];
								//strcpy(TempStr, FileData.cFileName);
								//StripExtension(TempStr);

								if(ThisStack)
									{
									// Need to add subdir to path
									strmfp(TempStr, ActiveDir, ThisStack->DirName);
									strmfp(FullPathAssemble, TempStr, FileData.cFileName);
									} // if
								else
									{
									strmfp(FullPathAssemble, ActiveDir, FileData.cFileName);
									} // else

								if (NameList[NumValidItems] = (char *)AppMem_Alloc(strlen(FullPathAssemble) + 1, APPMEM_CLEAR))
									{
									if(ThisStack)
										{
										strcpy(NameList[NumValidItems], FullPathAssemble);
										strcpy(NameStr, NameList[NumValidItems]);
										} // if
									else
										{
										strcpy(NameList[NumValidItems], FullPathAssemble);
										strmfp(NameStr, ActiveDir, FileData.cFileName);
										} // else
									if (BrowseList[NumValidItems].LoadBrowseInfo(NameStr))
										{
										//StripExtension(NameList[NumValidItems]);
										NumValidItems ++;
										} // if
									else
										{
										AppMem_Free(NameList[NumValidItems], strlen(FullPathAssemble) + 1);
										NameList[NumValidItems] = NULL;
										} // else
									} // if
								else
									{
									break;
									} // else
								} // else
							} // if
						} // else

					} // for
				} // for DoneDirs
			FindClose(Hand);
			Hand = NULL;
			} // if
		JustRecentConfigured = JustRecent;

		} // for Pass
	} // if

NumRanges = (NumValidItems) / 6 + ((NumValidItems) % 6 ? 1: 0);

// free up dirstack
if(DirStack)
	{
	while(DirStack)
		{
		// reuse EndStack as temp
		EndStack = DirStack->Next;
		delete DirStack;
		DirStack = EndStack;
		} // for
	} // if

ActiveRange = ActiveItem = 0;
ConfigureProjectImages();

//SetActiveProject(0);
//ConfigureDD(NativeWin, IDC_ACTIVEDIR, ActiveDir, 255, NULL, 0, IDC_LABEL_DEFDIR);
if(JustRecentConfigured)
	{
	// always enable pagedown if we haven't grabbed the actual file listing yet.
	WidgetSetDisabled(IDC_PAGEDOWN, 0);
	} // if
else
	{
	// enable it as necessary once we have the whole picture
	WidgetSetDisabled(IDC_PAGEDOWN, NumRanges < 2 || ActiveRange >= NumRanges - 1);
	} // else
WidgetSetDisabled(IDC_PAGEUP, NumRanges < 2 || ActiveRange <= 0);

} // VersionGUI::ConfigureProjects

/*===========================================================================*/

void VersionGUI::NewProjectRange(int MoveDir)
{

if (MoveDir < 0)
	{
	if (ActiveRange < NumRanges - 1)
		ActiveRange ++;
	ConfigureProjectImages();
	} // if
else
	{
	if (ActiveRange > 0)
		ActiveRange --;
	ConfigureProjectImages();
	} // else

WidgetSetDisabled(IDC_PAGEDOWN, NumRanges < 2 || ActiveRange >= NumRanges - 1);
WidgetSetDisabled(IDC_PAGEUP, NumRanges < 2 || ActiveRange <= 0);

} // VersionGUI::NewProjectRange

/*===========================================================================*/

char *VersionGUI::FormatWithPath(char *Input)
{
// if we already have path, just unmung and return
// if not, prepend path, unmung and return
int HasPathParts = 0;
int ScanLoop;
// temporary scope because we'll return the output of Unmungpath which
// has a longer-persistance buffer
char PathFormatBuffer[200];

for(ScanLoop = 0; Input[ScanLoop]; ScanLoop++)
	{
	if((Input[ScanLoop] == ':') || (Input[ScanLoop] == '\\') || (Input[ScanLoop] == '/'))
		{
		HasPathParts = 1;
		break;
		} // if
	} // if

if(HasPathParts)
	{
	return(GlobalApp->MainProj->UnMungPath(Input));
	} // if
else
	{
	strmfp(PathFormatBuffer, ActiveDir, Input);
	return(GlobalApp->MainProj->UnMungPath(PathFormatBuffer));
	} // else

// Lint points out that we can't ever reach the next line of code
//return(GlobalApp->MainProj->UnMungPath(Input));
} // VersionGUI::FormatWithPath

char *VersionGUI::FormatWithoutPath(char *Input)
{
// if we have path portions, strip them
// unmunging not necessary if path is removed
char *StartofName;
int ScanLoop;

StartofName = Input;
for(ScanLoop = 0; Input[ScanLoop]; ScanLoop++)
	{
	if((Input[ScanLoop] == ':') || (Input[ScanLoop] == '\\') || (Input[ScanLoop] == '/'))
		{
		StartofName = &Input[ScanLoop + 1];
		} // if
	} // if

return(StartofName);
} // VersionGUI::FormatWithoutPath


void VersionGUI::ConfigureProjectImages(void)
{
char *Title;
long Ct, Found = 0;
Raster *MyRast;
char IconCaption[200];

if (RasterList && BrowseList)
	{
	FirstItemInRange = ActiveRange * WCS_VERSIONGUI_NUMNAILS;
	Found = 0;
	for (Ct = FirstItemInRange; Found < WCS_VERSIONGUI_NUMNAILS; Ct ++)
		{
		if (Ct < NumValidItems && NameList[Ct])
			{
			Title = NameList[Ct];
			sprintf(IconCaption, "Double-click to load %s", FormatWithPath(Title));
			RasterList[Found].Thumb = BrowseList[Ct].Thumb;
			MyRast = BrowseList[Ct].Thumb ? &RasterList[Found]: NULL;
			DisplayedItem[Found] = Ct;
			} // switch
		else
			{
			Title = "";
			IconCaption[0] = NULL;
			MyRast = NULL;
			DisplayedItem[Found] = NumValidItems;
			} // else
		switch (Found)
			{
			case 0:
				{
				WidgetSetText(IDC_EPTITLE1, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL1, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL1, NULL, NULL, MyRast);
				break;
				} // case 0
			case 1:
				{
				WidgetSetText(IDC_EPTITLE2, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL2, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL2, NULL, NULL, MyRast);
				break;
				} // case 1
			case 2:
				{
				WidgetSetText(IDC_EPTITLE3, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL3, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL3, NULL, NULL, MyRast);
				break;
				} // case 2
			case 3:
				{
				WidgetSetText(IDC_EPTITLE4, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL4, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL4, NULL, NULL, MyRast);
				break;
				} // case 3
			case 4:
				{
				WidgetSetText(IDC_EPTITLE5, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL5, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL5, NULL, NULL, MyRast);
				break;
				} // case 4
			case 5:
				{
				WidgetSetText(IDC_EPTITLE6, FormatWithoutPath(Title));
				WidgetSetText(IDC_EPTNAIL6, IconCaption);
				ConfigureTB(NativeWin, IDC_EPTNAIL6, NULL, NULL, MyRast);
				break;
				} // case 5
			} // switch
		Found ++;
		} // for
	} // if

} // VersionGUI::ConfigureProjectImages

/*===========================================================================*/

void VersionGUI::SetActiveProject(long NewActive)
{
char CommentText[4096];

if (BrowseList)
	{
	if ((NewActive = DisplayedItem[NewActive]) < NumValidItems && NameList[NewActive])
		{
		// capture any data the user may have entered
		ActiveItem = NewActive;
		strcpy(CommentText, FormatWithoutPath(NameList[ActiveItem]));
		if(BrowseList[ActiveItem].Comment)
			{
			strcat(CommentText, "\r\n");
			strncat(CommentText, BrowseList[ActiveItem].Comment, 4000 - strlen(CommentText));
			CommentText[4000] = NULL;
			} // if
		WidgetSetText(IDC_EPCOMMENT, CommentText);
		} // if
	else
		{
		ActiveItem = 0;
		WidgetSetText(IDC_EPCOMMENT, "No Project Selected");
		} // else
	} // if

} // VersionGUI::SetActiveProject

/*===========================================================================*/

void VersionGUI::CaptureActiveProject(long NewActive)
{
char filename[512];
long ItemStash = NewActive, Result, UseDir = 1;
char *cur;
RasterAnimHostProperties Prop;

if (BrowseList && GlobalApp->MainProj)
	{
	if ((NewActive = DisplayedItem[NewActive]) < NumValidItems && NameList[NewActive])
		{
		ActiveItem = NewActive;

		// determine if the directory needs to be used
		cur = NameList[ActiveItem];
		while (*cur)
			{
			if (*cur == '/' || *cur == ':' || *cur == '\\')
				{
				UseDir = 0;
				break;
				}
			cur ++;
			} // while
		if (UseDir)
			strmfp(filename, ActiveDir, NameList[ActiveItem]);
		else
			strcpy(filename, NameList[ActiveItem]);
		if (strlen(filename) >= 5 && ! stricmp(&filename[strlen(filename) - 5], ".proj"))
			StripExtension(filename);
		strcat(filename, ".");
		strcat(filename, DefaultExt);
		Prop.Path = filename;
		Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
		Prop.Queries = 0;
		if ((Result = GlobalApp->MainProj->LoadFilePrep(&Prop)) > 0)
			{
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_VER, 0);
			} // if
		} // if
	else
		{
		SetActiveProject(ItemStash);
		} // else
	} // if

} // VersionGUI::CaptureActiveProject

/*===========================================================================*/




void VersionGUI::ConfigureNewProj(void)
{ // load the browse data for the components in each of the component categories
int MyType;

ClearProjects();
ClearComponents();

NumItems = 0;
NumValidItems = 0;

for(MyType = WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN; MyType < WCS_STARTUPGUINEWPAGE_TYPE_MAX; MyType++)
	{
	ScanComponents(MyType);
	CompActiveRange[MyType] = 0;
	} // for


WidgetSetDisabled(IDC_PAGEDOWN, NumCompRanges[MyType] < 2 || ActiveRange >= NumCompRanges[MyType] - 1);
WidgetSetDisabled(IDC_PAGEUP, NumCompRanges[MyType] < 2 || ActiveRange <= 0);

return;
} // VersionGUI::ConfigureNewProj



/*===========================================================================*/
void VersionGUI::ScanComponents(int MyType)
{ // load the browse data for the components in the component category specified
char NameStr[512], TempStr[512];
int OneShot;
WIN32_FIND_DATA FileData;
HANDLE Hand = NULL;

strmfp(NameStr, GlobalApp->MainProj->MungPath(FileCompPaths[MyType]), "*");
// dirscan loop
for(OneShot = 1; ; ) // exit via break
	{
	if(OneShot)
		{
		if(Hand != NULL)
			{ // close previous handle
			FindClose(Hand);
			Hand = NULL;
			} // if
		if ((Hand = FindFirstFile(NameStr, &FileData)) == INVALID_HANDLE_VALUE)
			{
			break; // bail out of dirscan loop
			} // if
		OneShot = 0;
		} // if
	else
		{
		if(!(FindNextFile(Hand, &FileData)))
			{
			break; // bail out of dirscan loop
			} // if
		} // else
	// Is it a directory or a file?
	if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{ // dir, ignore it
		} // if
	else
		{ // file
		// does it end in ".proj"?
		if((strlen(FileData.cFileName) > 4) && !stricmp(FileExt[MyType], &FileData.cFileName[strlen(FileData.cFileName) - 4]))
			{
			CombinedComponentData *TestAlloc;

			strcpy(TempStr, FileData.cFileName);
			StripExtension(TempStr);

			if (TestAlloc = new CombinedComponentData)
				{
				strcpy(TestAlloc->ComponentName, TempStr);
				strmfp(NameStr, FileCompPaths[MyType], FileData.cFileName);
				if (TestAlloc->BD.LoadBrowseInfo(NameStr))
					{
					NumValidComps[MyType]++;
					TestAlloc->Next = CCD[MyType];
					CCD[MyType] = TestAlloc;
					} // if
				else
					{
					delete TestAlloc;
					TestAlloc = NULL;
					} // else
				} // if
			else
				{
				break;
				} // else
			} // if
		} // else

	} // for
FindClose(Hand);
Hand = NULL;

NumCompRanges[MyType] = (NumValidComps[MyType]) / 6 + ((NumValidComps[MyType]) % 6 ? 1: 0);


} // VersionGUI::ScanComponents

/*===========================================================================*/

void VersionGUI::CreateNewProj(void)
{ // do the deed of creating a new project with all the selected components
char NewName[512];
NewProject ProjNew(GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages);
RasterAnimHostProperties Prop;
RasterAnimHost *Active;
int MyType;
NotifyTag Changes[2];

#ifndef WCS_BUILD_DEMO

		if(!GlobalApp->Sentinal->CheckDongle()) return;

#endif // !WCS_BUILD_DEMO

// test code, fill in presets to try out creation
/*
strcpy(SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN], "HillsAndGullies");
strcpy(SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_GROUND], "Rocky Organic");
strcpy(SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_ENVIRON], "Generic Environment");
strcpy(SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_SKY], "Plum Pudding");
strcpy(SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_CLOUD], "Jetstream");
*/

WidgetGetText(IDC_NEWPROJNAMEEDIT, 500, NewName);
if(NewName[0])
	{
	ProjNew.ProjName.SetPathAndName("WCSProjects:", NewName);
	ProjNew.CloneType = 0;
	ProjNew.PlaceInSubDir = 1;
	ProjNew.Clone = 0;
	// ProjNew. Templates? = 0;

	ProjNew.Create();

	// load components, do ground first
	for(MyType = WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN; MyType < WCS_STARTUPGUINEWPAGE_TYPE_MAX; MyType++)
		{
		if(SelectedCompNames[MyType][0])
			{
			if(Active = GlobalApp->AppEffects->GetDefaultEffect(EffectSubClass[MyType], 1, GlobalApp->AppDB))
				{
				// find the existing item of this type in the new project, or create a 'blank' one
				// tell it to load the user-selected component
				strmfp(NewName, FileCompPaths[MyType], SelectedCompNames[MyType]);
				strcat(NewName, FileExt[MyType]);
				Prop.Path = NewName;
				Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
				Prop.Queries = 0;
				if ((Active->LoadFilePrep(&Prop)) > 0)
					{
					Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
					} // if
				} // if
			} // if
		if(MyType == WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN)
			{ // instantiate terrain so other components can float on it
			TerraGenerator *GeneratorComponent;
			time_t GenerateMoment;

			GeneratorComponent = (TerraGenerator *)Active;

			// set random seed
			GetTime(GenerateMoment);
			// conversion from time_t to unsigned long below does is not harmful as all we need are some unpredictable bits
			GeneratorComponent->TerraType.Seed = (unsigned long int)GenerateMoment % 100000;
			GeneratorComponent->DoSomethingConstructive(GlobalApp->MainProj, GlobalApp->AppDB);
			} // if
		} // for


	// clean up by nulling selected components entries
	SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN][0] = NULL;
	SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_GROUND][0] = NULL;
	SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_ENVIRON][0] = NULL;
	SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_SKY][0] = NULL;
	SelectedCompNames[WCS_STARTUPGUINEWPAGE_TYPE_CLOUD][0] = NULL;

	// save project now
	strmfp(NewName, GlobalApp->MainProj->projectpath, GlobalApp->MainProj->projectname);
	#ifndef WCS_BUILD_DEMO
	if (! GlobalApp->MainProj->Save(NULL, NewName, GlobalApp->AppDB, GlobalApp->AppEffects, GlobalApp->AppImages, NULL, 0xffffffff))
		UserMessageOK("Project: Save", "An error occurred saving the file.\n File could not be opened for writing or an\n error occurred while writing the file. \nCheck disk access privileges and amount of free space.");
	GlobalApp->MainProj->SavePrefs(GlobalApp->GetProgDir());
	#endif // WCS_BUILD_DEMO

	// Open up a camera in a View here
	if(1)
		{
		int ViewPane = 0;
		if(GlobalApp->GUIWins->CVG)
			{
			GlobalApp->MainProj->ViewPorts.SetFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW);
			GlobalApp->GUIWins->CVG->CreateNewViewAndCamera(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_EFFECTS_CAMERATYPE_TARGETED);
			} // if
		} // if

	HandleCloseWin(NULL);
	} // if

} // VersionGUI::CreateNewProj


/*===========================================================================*/

void VersionGUI::RandomizeSelection(void)
{ // select a random component from each category
time_t RandomMoment;
int MyType, RandRes, CompNum;

GetTime(RandomMoment);

// conversion from time_t to unsigned long (in srand, below) does is not harmful as all we need are some unpredictable bits
srand((unsigned long int)RandomMoment); // no need for strong PRNs
for(MyType = WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN; MyType < WCS_STARTUPGUINEWPAGE_TYPE_MAX; MyType++)
	{
	RandRes = rand();
	if(CCD[MyType] && (NumValidComps[MyType] > 0))
		{
		CompNum = RandRes % NumValidComps[MyType];
		SetActiveComponentByNum(MyType, CompNum);
		} // if
	} // for

} // VersionGUI::RandomizeSelection

/*===========================================================================*/

void VersionGUI::SetActiveComponentByIconNum(int IconNum)
{
int MyType, CompNum;

MyType = ActiveSubPage - 1;
CompNum = DisplayedItem[IconNum];

SetActiveComponentByNum(MyType, CompNum);

} // VersionGUI::SetActiveComponentByIconNum

/*===========================================================================*/

void VersionGUI::SetActiveComponentByNum(int MyType, int CompNum)
{
int CompCount;
CombinedComponentData *CompScan;

if(CCD[MyType] && CompNum < NumValidComps[MyType])
	{
	for(CompCount = 0, CompScan = CCD[MyType]; CompScan; CompScan = CompScan->Next)
		{
		if(CompCount == CompNum)
			{
			SetActiveComponentByCCD(MyType, CompScan);
			break;
			} // if
		CompCount++;
		} // for
	} // if


} // VersionGUI::SetActiveComponentByNum

/*===========================================================================*/


void VersionGUI::SetActiveComponentByCCD(int MyType, CombinedComponentData *NewActiveComponent)
{
if(NewActiveComponent && MyType < WCS_STARTUPGUINEWPAGE_TYPE_MAX)
	{
	SelectedCompData[MyType].Copy(&SelectedCompData[MyType], &NewActiveComponent->BD);
	strcpy(SelectedCompNames[MyType], NewActiveComponent->ComponentName);
	SelectedCompRaster[MyType].Thumb = NewActiveComponent->BD.Thumb;

	if(ActiveSubPage != 0)
		{
		WidgetSetText(IDC_NPCOMMENT, NewActiveComponent->BD.Comment);
		} // if
	} // if

} // VersionGUI::SetActiveComponentByCCD


/*===========================================================================*/

void VersionGUI::NewComponentRange(int MyType, int MoveDir)
{

if (MoveDir < 0)
	{
	if (CompActiveRange[MyType] < NumCompRanges[MyType] - 1)
		CompActiveRange[MyType] ++;
	ConfigureComponentImages(MyType);
	} // if
else
	{
	if (CompActiveRange[MyType] > 0)
		CompActiveRange[MyType] --;
	ConfigureComponentImages(MyType);
	} // else

WidgetSetDisabled(IDC_NPAGEDOWN, NumCompRanges[MyType] < 2 || CompActiveRange[MyType] >= NumCompRanges[MyType] - 1);
WidgetSetDisabled(IDC_NPAGEUP, NumCompRanges[MyType] < 2 || CompActiveRange[MyType] <= 0);

} // VersionGUI::NewComponentRange

/*===========================================================================*/


void VersionGUI::ConfigureComponentImages(int MyType)
{ // do what ConfigureProjectImages() did, but for Components
char *Title;
long Ct, Found = 0, CaptionScan;
Raster *MyRast;
char IconCaption[400];
CombinedComponentData *CCDScan = NULL;

if(!CompRasterList)
	{
	CompRasterList = new Raster [WCS_VERSIONGUI_NUMNAILS];
	} // if

if(!CompRasterList) return;

FirstItemInRange = CompActiveRange[MyType] * WCS_VERSIONGUI_NUMNAILS;
Found = 0;
for (Ct = 0, CCDScan = CCD[MyType]; (Found < WCS_VERSIONGUI_NUMNAILS); CCDScan = (CCDScan ? CCDScan->Next : NULL), Ct++)
	{
	if(Ct >= FirstItemInRange)
		{
		if (CCDScan)
			{
			Title = CCDScan->ComponentName;
			IconCaption[0] = NULL;
			//sprintf(IconCaption, "%50s: %200s", CCDScan->ComponentName, CCDScan->BD.Comment);
			if(CCDScan->ComponentName && CCDScan->ComponentName[0])
				{
				strncpy(IconCaption, CCDScan->ComponentName, 50);
				strcat(IconCaption, ": ");
				} // if
			if(CCDScan->BD.Comment && CCDScan->BD.Comment[0])
				{
				strncat(IconCaption, CCDScan->BD.Comment, 200);
				} // if
			for(CaptionScan = 0; IconCaption[CaptionScan]; CaptionScan++)
				{
				// convert all whitespace, tabs, CRs to spaces for tooltip.
				if(isspace(IconCaption[CaptionScan]))
					{
					IconCaption[CaptionScan] = ' ';
					} // if
				} // for
			CompRasterList[Found].Thumb = CCDScan->BD.Thumb;
			MyRast = CCDScan->BD.Thumb ? &CompRasterList[Found]: NULL;
			DisplayedItem[Found] = Ct;
			} // switch
		else
			{
			Title = "";
			IconCaption[0] = NULL;
			MyRast = NULL;
			DisplayedItem[Found] = NumValidItems;
			} // else
		switch (Found)
			{
			case 0:
				{
				WidgetSetText(IDC_NPTITLE1, Title);
				WidgetSetText(IDC_NPTNAIL1, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL1, NULL, NULL, MyRast);
				break;
				} // case 0
			case 1:
				{
				WidgetSetText(IDC_NPTITLE2, Title);
				WidgetSetText(IDC_NPTNAIL2, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL2, NULL, NULL, MyRast);
				break;
				} // case 1
			case 2:
				{
				WidgetSetText(IDC_NPTITLE3, Title);
				WidgetSetText(IDC_NPTNAIL3, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL3, NULL, NULL, MyRast);
				break;
				} // case 2
			case 3:
				{
				WidgetSetText(IDC_NPTITLE4, Title);
				WidgetSetText(IDC_NPTNAIL4, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL4, NULL, NULL, MyRast);
				break;
				} // case 3
			case 4:
				{
				WidgetSetText(IDC_NPTITLE5, Title);
				WidgetSetText(IDC_NPTNAIL5, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL5, NULL, NULL, MyRast);
				break;
				} // case 4
			case 5:
				{
				WidgetSetText(IDC_NPTITLE6, Title);
				WidgetSetText(IDC_NPTNAIL6, IconCaption);
				ConfigureTB(NativeWin, IDC_NPTNAIL6, NULL, NULL, MyRast);
				break;
				} // case 5
			} // switch
		Found ++;
		} // if
	else
		{
		// do nothing but keep advancing
		} // else
	} // for

} // VersionGUI::ConfigureComponentImages



/*===========================================================================*/

void VersionGUI::ConfigureSummaryImages(void)
{ // do what ConfigureProjectImages() did, but for Components


char *Title;
long Ct, Found = 0, CaptionScan;
Raster *MyRast;
char IconCaption[400];

for (Ct = 0; Ct < WCS_STARTUPGUINEWPAGE_TYPE_MAX; Ct ++)
	{
	if(SelectedCompNames[Ct][0])
		{
		Title = SelectedCompNames[Ct];
		IconCaption[0] = NULL;
		//sprintf(IconCaption, "%50s: %200s", Title, SelectedCompData[Ct].Comment);
		if(Title && Title[0])
			{
			strncpy(IconCaption, Title, 50);
			strcat(IconCaption, ": ");
			} // if
		if(SelectedCompData[Ct].Comment && SelectedCompData[Ct].Comment[0])
			{
			strncat(IconCaption, SelectedCompData[Ct].Comment, 200);
			} // if
		for(CaptionScan = 0; IconCaption[CaptionScan]; CaptionScan++)
			{
			// convert all whitespace, tabs, CRs to spaces for tooltip.
			if(isspace(IconCaption[CaptionScan]))
				{
				IconCaption[CaptionScan] = ' ';
				} // if
			} // for
		SelectedCompRaster[Found].Thumb = SelectedCompData[Ct].Thumb;
		MyRast = SelectedCompData[Ct].Thumb ? &SelectedCompRaster[Found]: NULL;
		} // switch
	else
		{
		Title = TabNames[Ct + 1];
		sprintf(IconCaption, "No %s component selected.", TabNames[Ct + 1]);
		MyRast = NULL;
		} // else
	switch (Ct)
		{
		case 0:
			{
			WidgetSetText(IDC_NPTITLE1, Title);
			WidgetSetText(IDC_NPTNAIL1, IconCaption);
			ConfigureTB(NativeWin, IDC_NPTNAIL1, NULL, NULL, MyRast);
			break;
			} // case 0
		case 1:
			{
			WidgetSetText(IDC_NPTITLE2, Title);
			WidgetSetText(IDC_NPTNAIL2, IconCaption);
			ConfigureTB(NativeWin, IDC_NPTNAIL2, NULL, NULL, MyRast);
			break;
			} // case 1
		case 2:
			{
			WidgetSetText(IDC_NPTITLE3, Title);
			WidgetSetText(IDC_NPTNAIL3, IconCaption);
			ConfigureTB(NativeWin, IDC_NPTNAIL3, NULL, NULL, MyRast);
			break;
			} // case 2
		case 3:
			{
			WidgetSetText(IDC_NPTITLE4, Title);
			WidgetSetText(IDC_NPTNAIL4, IconCaption);
			ConfigureTB(NativeWin, IDC_NPTNAIL4, NULL, NULL, MyRast);
			break;
			} // case 3
		case 4:
			{
			WidgetSetText(IDC_NPTITLE5, Title);
			WidgetSetText(IDC_NPTNAIL5, IconCaption);
			ConfigureTB(NativeWin, IDC_NPTNAIL5, NULL, NULL, MyRast);
			break;
			} // case 4
		} // switch
	Found++;
	} // for

} // VersionGUI::ConfigureSummaryImages



/*===========================================================================*/

void VersionGUI::DisableWidgets(void)
{

#ifndef WCS_BUILD_DEMO
WidgetSetDisabled(IDC_USERDROP, ! GlobalApp->MainProj->Prefs.MultiUserMode);
//WidgetSetDisabled(IDC_PASSWORD, ! GlobalApp->MainProj->Prefs.MultiUserMode);
#else // WCS_BUILD_DEMO
WidgetSetDisabled(IDC_CHECKMULTIUSER, TRUE);
WidgetSetDisabled(IDC_USERDROP, TRUE);
//WidgetSetDisabled(IDC_PASSWORD, TRUE);
#endif // WCS_BUILD_DEMO

} // VersionGUI::DisableWidgets

/*===========================================================================*/

void VersionGUI::SelectNewUser(void)
{
long Current, NumItems;
char InputStr[512], NewName[256];
#ifndef WCS_BUILD_DEMO
FILE *ffile;
#endif // !WCS_BUILD_DEMO

if ((Current = WidgetCBGetCurSel(IDC_USERDROP)) != CB_ERR)
	{
	// save the current user setup
	#ifndef WCS_BUILD_DEMO
	GlobalApp->MainProj->SavePrefs(GlobalApp->GetProgDir());
	#endif // WCS_BUILD_DEMO

	// void out the current password
	GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
	//WidgetSetText(IDC_PASSWORD, GlobalApp->MainProj->Prefs.CurrentPassword);
	//WidgetSetModified(IDC_PASSWORD, FALSE);

	NewName[0] = 0;
	// if user selected "new user"
	if (Current == 0)
		{
		// get user-supplied name
		if (GetInputString("Enter new user name. Maximum length is 63 characters.", ":;*/?`#%", NewName) && NewName[0])
			{
			NewName[63] = 0;
			// check to see if new name is unique
			Current = 1;
			if ((NumItems = WidgetCBGetCount(IDC_USERDROP)) > 1)
				{
				for (Current = 1; Current < NumItems; Current ++)
					{
					if (WidgetCBGetText(IDC_USERDROP, Current, InputStr) != CB_ERR)
						{
						if (! stricmp(InputStr, NewName))
							break;
						} // if
					} // for
				} // if
			if (Current >= NumItems)
				{
				// add new name to combo box and select it
				Current = WidgetCBInsert(IDC_USERDROP, -1, NewName);
				WidgetCBSetCurSel(IDC_USERDROP, Current);

				#ifndef WCS_BUILD_DEMO
				// write out user name file
				if ((NumItems = WidgetCBGetCount(IDC_USERDROP)) > 1)
					{
					strmfp(InputStr, GlobalApp->GetProgDir(), "WCSUsers.txt");

					// open and write user name file
					if (ffile = fopen(InputStr, "w"))
						{
						for (Current = 1; Current < NumItems; Current ++)
							{
							if (WidgetCBGetText(IDC_USERDROP, Current, InputStr) != CB_ERR)
								{
								fputs(InputStr, ffile);
								fputc('\n', ffile);
								} // if
							} // for
						fclose(ffile);
						} // if
					} // if
				#endif // WCS_BUILD_DEMO
				} // if
			else
				{
				WidgetCBSetCurSel(IDC_USERDROP, Current);
				UserMessageOK(NewName, "User name already exists.");
				} // else
			} // if new name supplied
		else
			WidgetCBSetCurSel(IDC_USERDROP, -1);
		} // if time for a new name
	else
		{
		if ((Current = WidgetCBGetCurSel(IDC_USERDROP)) != CB_ERR)
			{
			if (WidgetCBGetText(IDC_USERDROP, Current, NewName) == CB_ERR)
				NewName[0] = 0;
			} // if
		} // else
	NameChanged = 1;
	GlobalApp->WinSys->UpdateRootTitle(GlobalApp->MainProj->ProjectLoaded ? GlobalApp->MainProj->projectname: (char*)"", NewName);
	// ConfigureProjects doesn't reflect the new selection of user because prefs have not been reloaded.
	// see if this causes havoc or fixes the problem
	CaptureUserData();
	LoadPrefs();
	strcpy(ActiveDir, (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->projectpath[0]) ? 
		GlobalApp->MainProj->projectpath: "WCSProjects:");
	// defer ConfigureProjects until we switch to that pane
	//ConfigureProjects();
	} // if

} // VersionGUI::SelectNewUser

/*===========================================================================*/

void VersionGUI::CaptureUserData(void)
{
long Current;
char NewName[256];

if (GlobalApp->MainProj->Prefs.MultiUserMode)
	{
	if (NameChanged)
		{
		GlobalApp->MainProj->Prefs.CurrentUserName[0] = 0;
		GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
		if ((Current = WidgetCBGetCurSel(IDC_USERDROP)) != CB_ERR)
			{
			if (WidgetCBGetText(IDC_USERDROP, Current, NewName) != CB_ERR)
				{
				strncpy(GlobalApp->MainProj->Prefs.CurrentUserName, NewName, 63);
				GlobalApp->MainProj->Prefs.CurrentUserName[63] = 0;
				} // if
			} // if
		} // if
	if (GlobalApp->MainProj->Prefs.CurrentUserName[0])
		{
		// user may or may not have entered a password
		//if (WidgetGetModified(IDC_PASSWORD))
		//	{
		//	WidgetGetText(IDC_PASSWORD, 64, GlobalApp->MainProj->Prefs.CurrentPassword);
		//	} // if
		} // if user name
	else
		{
		GlobalApp->MainProj->Prefs.MultiUserMode = 0;
		GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
		} // else no user, no password
	} // if
else
	{
	GlobalApp->MainProj->Prefs.CurrentUserName[0] = 0;
	GlobalApp->MainProj->Prefs.CurrentPassword[0] = 0;
	} // else
GlobalApp->WinSys->UpdateRootTitle(GlobalApp->MainProj->ProjectLoaded ? GlobalApp->MainProj->projectname: (char*)"", GlobalApp->MainProj->Prefs.CurrentUserName);

} // VersionGUI::CaptureUserData

/*===========================================================================*/

void VersionGUI::LoadPrefs(void)
{
char PrefsName[256];
NotifyTag Changes[2];

if (GlobalApp->MainProj->Prefs.CurrentUserName[0] && (NameChanged || ! GlobalApp->MainProj->ProjectLoaded))
	{
	// remove old project elements
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	GlobalApp->GUIWins->CVG->CloseViewsForIO();
	if (GlobalApp->MainProj->BrowseInfo)
		GlobalApp->MainProj->BrowseInfo->FreeAll();
	GlobalApp->AppImages->DeleteAll();
	GlobalApp->AppEffects->DeleteAll(TRUE);
	GlobalApp->MainProj->Interactive->ClearAllPts();
	GlobalApp->MainProj->ClearPiers();
	GlobalApp->AppDB->DestroyAll();
	Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	GlobalApp->MainProj->pcprojectpath[0] = 0;
	GlobalApp->MainProj->pcframespath[0] = 0;
	GlobalApp->MainProj->contentpath[0] = 0;
	GlobalApp->MainProj->LastProject[0][0] = 0;
	GlobalApp->MainProj->PrefsLoaded = GlobalApp->MainProj->ProjectLoaded = 0;
	strcpy(PrefsName, GlobalApp->MainProj->Prefs.CurrentUserName);
	strcat(PrefsName, ".prefs");
	GlobalApp->MainProj->LoadPrefs(GlobalApp->GetProgDir(), PrefsName);
	} // if

GlobalApp->WinSys->UpdateRootTitle(GlobalApp->MainProj->ProjectLoaded ? GlobalApp->MainProj->projectname: (char*)"", GlobalApp->MainProj->Prefs.CurrentUserName);

} // VersionGUI::LoadPrefs

/*===========================================================================*/

void VersionGUI::SetTip(char *Tip)
{
WidgetSetText(IDC_TEXTBOX, Tip);
} // VersionGUI::SetTip

static char TempTipString[256];

/*===========================================================================*/

void VersionGUI::PickAndSetTip(int WhichTip)
{
double UpdateThres, Elaps;
long int TipNum, MaxTips;
time_t NowTime, LastTime;
struct _stat ProgramStat;
LPTSTR whoAmI;

UpdateTip = 0;
MaxTips = sizeof(TipTable) / sizeof(char *);
time(&NowTime);
UpdateThres = (WCS_VERSION_UPDATE_THRESH_DAYS * (60 * 60 * 24));

LastTime = GlobalApp->MainProj->Prefs.LastUpdateDate; // <<<>>> TIME64 issue, should use time_t for future-proofing

whoAmI = GetCommandLine();
// we get double quotes - how special!
strcpy(TempTipString, &whoAmI[1]);
//GlobalApp->GetProgDir(TempTipString, 240);
//strcat(TempTipString, APP_TLA".exe");
if(_stat(TempTipString, &ProgramStat) == 0)
	{
	LastTime = max(LastTime, max(ProgramStat.st_ctime, ProgramStat.st_mtime));
	//LastTime = max(LastTime, ProgramStat.st_ctime);
	} // if

//sprintf(TempTipString, "Time modified : %s", ctime( &ProgramStat.st_ctime ) );
Elaps = difftime(NowTime, LastTime);
//sprintf(TempTipString, "Diff: %f", Elaps);
//UserMessageOK("Update", TempTipString);

#ifndef WCS_BUILD_DEMO
if ((!UpdateWarned) && ((GlobalApp->MainProj->Prefs.LastUpdateDate == 0) || (Elaps > UpdateThres)))
	{
	if(GlobalApp->MainProj->Prefs.LastUpdateDate == 0)
		{
		GlobalApp->MainProj->Prefs.LastUpdateDate = (long)NowTime; // <<<>>> TIME64 issue, should use time_t for future-proofing
		} // if
	UpdateTip = 1;
	UpdateWarned = 1;
	DoUpdateMessage();
	} // if
else
#endif // !WCS_BUILD_DEMO
	{
	if(WhichTip == -1)
		{
		// conversion from time_t to unsigned long below does is not harmful as all we need are some unpredictable bits
		TipNum = (unsigned long int)NowTime % (unsigned long)MaxTips;
		} // if
	else
		{
		TipNum = WhichTip % MaxTips;
		} // else

	WidgetShow(IDC_MOREINFO, 0);
	if(TipNum < 0) TipNum = 0;
	SetTip(TipTable[TipNum]);
	LastTip = TipNum;
	} // else


} // VersionGUI::PickAndSetTip

/*===========================================================================*/

static char TipFileBuf[30];
void VersionGUI::TipInfo(int WhichTip)
{
long int MaxTips;

MaxTips = sizeof(TipTable) / sizeof(char *);

if(UpdateTip)
	{
	if (AppScope->HelpSys)
		AppScope->HelpSys->DoOnlineUpdate();
	} // if
else
	{
	if((WhichTip >= 0) && (WhichTip < MaxTips))
		{
		sprintf(TipFileBuf, "tip%03d", WhichTip);
		GlobalApp->HelpSys->OpenHelpFile(TipFileBuf);
		} // if
	} // else

} // VersionGUI::TipInfo

/*===========================================================================*/

void VersionGUI::DoOSVersion(void)
{
OSVERSIONINFO osvi;

memset(&osvi, 0, sizeof(OSVERSIONINFO));
osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

GetVersionEx(&osvi);

//sprintf(TempTipString, "osver: %d, winmajor: %d, winminor: %d, winver: %d.\n", _osver, _winmajor, _winminor, _winver);
sprintf(TempTipString, "osbuild: %d, winmajor: %d, winminor: %d, version: %s.\n",
		osvi.dwBuildNumber, osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion);
SetTip(TempTipString);

} // VersionGUI::DoOSVersion

/*===========================================================================*/

void VersionGUI::DoUpdateMessage(void)
{
WidgetShow(IDC_MOREINFO, 1);
#ifdef WCS_BUILD_VNS
SetTip("\n\rIt appears it has been over 30 days since you last obtained an online update of Visual Nature Studio. Click the More Info button to download the latest version from the 3D Nature Web Site.");
#else // !VNS
SetTip("\n\rIt appears it has been over 30 days since you last obtained an online update of World Construction Set. Click the More Info button to download the latest version from the 3D Nature Web Site.");
#endif // !VNS
UpdateTip = 1;
} // VersionGUI::DoUpdateMessage


// *********************************************************************
//
// Code to support loading and decompressing a JPEG out of resource data
//
// *********************************************************************


struct jpeg_source_mgr JPEGMemLoader;
static void *LogoJPGRsc;
static int LogoJPGSize;

#ifdef WCS_BUILD_JPEG_SUPPORT

void mem_init_source(j_decompress_ptr cinfo)
{
jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

// We reset the empty-input-file flag for each image,
// but we don't clear the input buffer.
// This is correct behavior for reading a series of images from one source.
src->next_input_byte = (unsigned char *)LogoJPGRsc;
src->bytes_in_buffer = LogoJPGSize;
} // mem_init_source()


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

// X: I don't believe this will ever be called as we fill the entire buffer in init_source above
boolean mem_fill_input_buffer(j_decompress_ptr cinfo)
{
  jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

  src->next_input_byte = (unsigned char *)LogoJPGRsc;
  src->bytes_in_buffer = LogoJPGSize;

  return TRUE;
} // mem_fill_input_buffer()


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

static void mem_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
jpeg_source_mgr *src = (jpeg_source_mgr *) cinfo->src;

src->next_input_byte += (size_t) num_bytes;
src->bytes_in_buffer -= (size_t) num_bytes;
} // mem_skip_input_data


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */

static unsigned char mem_jpeg_resync_to_restart (j_decompress_ptr cinfo, int desired)
{
return(jpeg_resync_to_restart(cinfo, desired));
} // mem_jpeg_resync_to_restart 


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

static void mem_term_source(j_decompress_ptr cinfo)
{
  /* no work necessary here */
} // mem_term_source

#endif // WCS_BUILD_JPEG_SUPPORT

/*
 * Here's the routine that will replace the standard error_exit method:
 */

struct VGUI_my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct VGUI_my_error_mgr * my_error_ptr;

#ifdef WCS_BUILD_JPEG_SUPPORT

METHODDEF(void)
mem_my_JPEGLOADER_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  //(*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

#endif // WCS_BUILD_JPEG_SUPPORT


int VersionGUI::LoadLogo(void)
{
int Success = 0;
HRSRC  ResHandle = NULL;
struct jpeg_decompress_struct cinfo;
//struct error_handler_data;
struct VGUI_my_error_mgr jerr;
int row_stride = 0;
unsigned long int InScan, PixelCol;
short Cols, Rows;
unsigned char *InterleaveBuf = NULL, *RBuf, *GBuf, *BBuf;
WORD LOGOID;

if(LogoW) // are we already loaded?
	{
	return(1);
	} // if

// ensure deallocated
UnloadLogo();

#ifdef WCS_BUILD_VNS
LOGOID = IDR_VNSSTARTUPWINLOGO_JPG;
#else // !WCS_BUILD_VNS
LOGOID = IDR_STARTUPWINLOGO_JPG;
#endif // !WCS_BUILD_VNS

// passing NULL as Instance means 'me you moron'
if(LogoJPGRsc = LockResource(LoadResource(NULL, ResHandle = FindResource(NULL, MAKEINTRESOURCE(LOGOID), "JPEGIMAGE"))))
	{
	LogoJPGSize = SizeofResource(NULL, ResHandle);

	#ifdef WCS_BUILD_JPEG_SUPPORT
	cinfo.err = jpeg_std_error((struct jpeg_error_mgr *)&jerr);
	jerr.pub.error_exit = mem_my_JPEGLOADER_error_exit;

	if (setjmp(jerr.setjmp_buffer))
		{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		if(InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
		if(!Success)
			{
			UnloadLogo();
			} // if
		jpeg_destroy_decompress(&cinfo);
		return(Success);
		} // if

	jpeg_create_decompress(&cinfo);
	//jpeg_stdio_src(&cinfo, fh);
	cinfo.src = &JPEGMemLoader;
	cinfo.src->init_source = mem_init_source;
	cinfo.src->fill_input_buffer = mem_fill_input_buffer;
	cinfo.src->skip_input_data = mem_skip_input_data;
	cinfo.src->resync_to_restart = mem_jpeg_resync_to_restart; /* use default method */
	cinfo.src->term_source = mem_term_source;
	cinfo.src->bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	cinfo.src->next_input_byte = NULL; /* until buffer loaded */

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	cinfo.dct_method = JDCT_FLOAT;
	jpeg_start_decompress(&cinfo);

	LogoW = Cols = (short)cinfo.output_width;
	LogoH = Rows = (short)cinfo.output_height;
	#else // !WCS_BUILD_JPEG_SUPPORT
	// without JPEG support, must fake it with blank black image of proper size
	LogoW = Cols = 533;
	LogoH = Rows = 400;
	#endif // !WCS_BUILD_JPEG_SUPPORT
	LogoRedBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Startup Window");
	LogoGrnBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Startup Window");
	LogoBluBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Startup Window");
	row_stride = cinfo.output_width * 3;
	InterleaveBuf = (UBYTE *)AppMem_Alloc(row_stride, 0, "JPG DeInterleave Buffer");
	if(InterleaveBuf && LogoRedBuf && LogoGrnBuf && LogoBluBuf) // everything ok?
		{
		// Clear bitmaps
		memset(LogoRedBuf, 0, Cols * Rows);
		memset(LogoGrnBuf, 0, Cols * Rows);
		memset(LogoBluBuf, 0, Cols * Rows);

		#ifdef WCS_BUILD_JPEG_SUPPORT
		while (cinfo.output_scanline < cinfo.output_height)
			{
			RBuf = &LogoRedBuf[cinfo.output_scanline * Cols];
			GBuf = &LogoGrnBuf[cinfo.output_scanline * Cols];
			BBuf = &LogoBluBuf[cinfo.output_scanline * Cols];
			if(jpeg_read_scanlines(&cinfo, &InterleaveBuf, 1) != 1)
				{
				jpeg_abort_decompress(&cinfo);
				break;
				} // if
			else
				{ // deinterleave
				for(InScan = PixelCol = 0; PixelCol < (unsigned long)(int)Cols; PixelCol++)
					{
					RBuf[PixelCol] = InterleaveBuf[InScan++];
					GBuf[PixelCol] = InterleaveBuf[InScan++];
					BBuf[PixelCol] = InterleaveBuf[InScan++];
					} // for
				} // else
			} // while

		if(cinfo.output_scanline == cinfo.output_height)
			{
			Success = 1;
			jpeg_finish_decompress(&cinfo);
			} // if
		#endif // WCS_BUILD_JPEG_SUPPORT
		} // if

	#ifdef WCS_BUILD_JPEG_SUPPORT
	jpeg_destroy_decompress(&cinfo);
	#endif // WCS_BUILD_JPEG_SUPPORT

	if(InterleaveBuf) AppMem_Free(InterleaveBuf, row_stride); InterleaveBuf = NULL;
	} // if
else
	{
	// setup blank image for now so we don't blow up
	LogoW = Cols = 533;
	LogoH = Rows = 400;
	LogoRedBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, APPMEM_CLEAR, "Startup Window");
	LogoGrnBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, APPMEM_CLEAR, "Startup Window");
	LogoBluBuf = (UBYTE *)AppMem_Alloc(Cols * Rows, APPMEM_CLEAR, "Startup Window");
	} // else


if(LogoJPGRsc)
	{
	FreeResource(LogoJPGRsc);
	LogoJPGRsc = NULL;
	} // if


return(Success);
} // VersionGUI::LoadLogo

void VersionGUI::UnloadLogo(void)
{
if(LogoW && LogoH)
	{
	if(LogoRedBuf) AppMem_Free(LogoRedBuf, LogoW * LogoH); LogoRedBuf = NULL;
	if(LogoGrnBuf) AppMem_Free(LogoGrnBuf, LogoW * LogoH); LogoGrnBuf = NULL;
	if(LogoBluBuf) AppMem_Free(LogoBluBuf, LogoW * LogoH); LogoBluBuf = NULL;
	LogoW = LogoH = 0;
	} // if
} // VersionGUI::UnloadLogo


int VersionGUI::InstallBG(void)
{
if (! BackgroundInstalled)
	{
	if (GlobalApp->AddBGHandler(this))
		BackgroundInstalled = 1;
	} // if

return(1);
} // VersionGUI::InstallBG


int VersionGUI::UninstallBG(void)
{
if (BackgroundInstalled)
	{
	GlobalApp->RemoveBGHandler(this);
	BackgroundInstalled = 0;
	} // if

return(1);
} // VersionGUI::UninstallBG

void VersionGUI::HideSplash(void)
{
WidgetShow(IDC_IMAGEWCS, 0);
SelectPage(ActivePage); // tips
WidgetShow(IDC_TIPS, 1);
WidgetShow(IDC_TUTORIALS, 1);
WidgetShow(IDC_EXISTPROJ, 1);
WidgetShow(IDC_NEWPROJ, 1);
WidgetShow(IDC_CHECKSHOWTHISPANE, (ActivePage != 0));
#ifdef WCS_BUILD_VNS
WidgetShow(IDC_USERS, 1);
#endif // WCS_BUILD_VNS
//SetActiveWindow(GetWidgetFromID(ID_RESUME));
DestroyWindow(SubPanels[0][WCS_VERSION_BLANK_PANE]);
SubPanels[0][WCS_VERSION_BLANK_PANE] = NULL;
} // VersionGUI::HideSplash


void VersionGUI::ShowSplash(void)
{
ShowPanel(0, WCS_VERSION_BLANK_PANE);
ShowPageWidgets(0);
WidgetShow(IDC_TIPS, 0);
WidgetShow(IDC_TUTORIALS, 0);
WidgetShow(IDC_EXISTPROJ, 0);
WidgetShow(IDC_NEWPROJ, 0);
WidgetShow(IDC_CHECKSHOWTHISPANE, 0);
WidgetShow(IDC_USERS, 0);
WidgetShow(IDC_IMAGEWCS, 1);
SetWindowPos(GetWidgetFromID(IDC_IMAGEWCS), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
} // VersionGUI::ShowSplash


void VersionGUI::CustomizeSplash(int WithSerNum)
{
FontImage FI;
InternalRasterFont IRF(1);
int YPos, YHeight;
char SplashString[200];

#ifdef WCS_BUILD_RTX
char SXNote[50];
SXNote[0] = 0;
#ifdef WCS_BUILD_DEMO
// Demo now shows off SX UI
sprintf(SXNote, "with Scene Express Demo");
#else // !WCS_BUILD_DEMO
if((GlobalApp->Sentinal) && (GlobalApp->SXAuthorized))
	{
	sprintf(SXNote, "with Scene Express");
	} // if
#endif // !WCS_BUILD_DEMO

#endif // RTX

YHeight = IRF.InternalFontHeight / 16 + 2;


FI.SetFont(IRF.FontDataPointer, (USHORT)IRF.InternalFontWidth, (USHORT)IRF.InternalFontHeight);
FI.SetOutput(LogoW, LogoH, LogoRedBuf, LogoGrnBuf, LogoBluBuf, NULL);

#ifdef WCS_BUILD_VNS
YPos = 310;
#else // WCS6
YPos = 310;
#endif // WCS_BUILD_VNS

#ifdef WCS_BUILD_RTX
FI.PrintText(1, YPos + 1, SXNote, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
FI.PrintText(0, YPos, SXNote, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
YPos += YHeight;
#endif // RTX

sprintf(SplashString, "%s", SerialStr);
FI.PrintText(1, YPos + 1, SplashString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
FI.PrintText(0, YPos, SplashString, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
YPos += YHeight;

sprintf(SplashString, "Version %s %s", VersionAppend, BuildStr);
FI.PrintText(1, YPos + 1, SplashString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
FI.PrintText(0, YPos, SplashString, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
YPos += YHeight;

sprintf(SplashString, "%s %s", ExtCopyrightFull, ExtPublisher);
FI.PrintText(1, YPos + 1, SplashString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
FI.PrintText(0, YPos, SplashString, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
YPos += YHeight;

} // VersionGUI::CustomizeSplash


int VersionGUI::ScanTutorials(void)
{
//char TutPath[1000];
//sprintf(TutPath, "%s%s", GlobalApp->GetProgDir(), WCS_APPHELP_PATH_TUTORIALS);
chdir(GlobalApp->GetProgDir());

HFI.BuildFileIndex(WCS_APPHELP_PATH_TUTORIALS);

return(HFI.GetNumHelpFiles());
} // VersionGUI::ScanTutorials

//lint -restore
