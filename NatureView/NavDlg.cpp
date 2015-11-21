// NavDlg.cpp
// Code for navigation dialog

#include <windows.h>
#include "resource.h"
#include "NavDlg.h"
#include "DriveDlg.h" // for centering/positioning relative to DriveDlg
#include "ToolTips.h"
#include "EventDispatcher.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "ResourceSupport.h"
#include "Navigation.h" // to query current nav mode
#include "NVMiscGlobals.h" // for CheckFollowTerrainEnabledGlobal()
#include "Viewer.h"

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include "SnazzyWidgetOSG.h"
#include "SnazzyWidgetJPGRC.h"

extern ToolTipSupport *GlobalTipSupport;
extern SnazzyWidgetContainerFactory *SWCF;
NativeAnyWin NavNativeWindow;

NativeGUIWin NavDlg::DlgHandle = NULL;
NavDlgSWEventObserver NavDlg::CallbackObserver;
SnazzyCheckWidget *NavDlg::Goto = NULL, *NavDlg::Follow = NULL, *NavDlg::Rotate = NULL, *NavDlg::Drive = NULL, *NavDlg::Slide = NULL, *NavDlg::Climb = NULL, *NavDlg::Query = NULL;
SnazzyWidgetContainer *NavDlg::SWC = NULL;

extern char ProgDir[500];


void NavDlg::Show(bool NewState)
{

if(DlgHandle)
	{
	if(NewState)
		{
		ShowWindow(DlgHandle, SW_SHOW);
		} // if
	else
		{
		ShowWindow(DlgHandle, SW_HIDE);
		} // else
	} // if

} // NavDlg::Show



NavDlg::NavDlg(bool Show)
{
DlgHandle = NULL;
SWC = NULL;
//char ImagePath[1024];

SnazzyWidgetImageCollection *SWIC = NULL;

/*
osg::ref_ptr<osg::Image> Index;

strcpy(ImagePath, ProgDir);
strcat(ImagePath, "/TempResource/NavPanelIdx.png");
Index = osgDB::readImageFile(ImagePath);
*/
try
	{
	SWIC = new SnazzyWidgetImageCollection(204, 96); // can we get this dynamically somehow?
	} // try
catch(std::bad_alloc)
	{
	// $CH <<<>>> These should be done in cleanup too
	delete SWIC;
	return;
	} // catch

//CopyImageToSnazzyWidgetIndex(Index.get(), SWIC);
LoadPNGImageToSnazzyWidgetIndex(IDR_NAVPANEL_INDEX_PNG, SWIC);
LoadJPGImageToSnazzyWidgetNormal(IDR_NAVPANEL_NORMAL_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetNormalHover(IDR_NAVPANEL_NORMAL_HOVER_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetSelected(IDR_NAVPANEL_SEL_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetSelectedHover(IDR_NAVPANEL_SEL_HOVER_JPEG, SWIC);


if(SWC = SWCF->CreateSnazzyWidgetContainer(GetGlobalViewerHWND(), &CallbackObserver, SWIC))
	{

	SWC->CreateSnazzyPushWidget(15,  IDC_PREVVIEW, LoadStringFromResource(IDS_PREVVPTIP));
	SWC->CreateSnazzyPushWidget(16,  IDC_VIEWPOINTS, LoadStringFromResource(IDS_EYEPOINTSTIP));
	SWC->CreateSnazzyPushWidget(17,  IDC_NEXTVIEW, LoadStringFromResource(IDS_NEXTVPTIP));
	SWC->CreateSnazzyPushWidget(18,  IDC_HOME, LoadStringFromResource(IDS_HOMETIP));
	Goto   = SWC->CreateSnazzyCheckWidget(19,  IDC_GOTO, LoadStringFromResource(IDS_GOTOTIP));
	Follow = SWC->CreateSnazzyCheckWidget(20,  IDC_FOLLOW, LoadStringFromResource(IDS_FOLLOWTIP));
	Rotate = SWC->CreateSnazzyCheckWidget(21,  IDC_ROTATE, LoadStringFromResource(IDS_ROTATETIP));
	Drive  = SWC->CreateSnazzyCheckWidget(22,  IDC_DRIVE, LoadStringFromResource(IDS_DRIVETIP));
	Slide  = SWC->CreateSnazzyCheckWidget(23,  IDC_SLIDE, LoadStringFromResource(IDS_SLIDETIP));
	Climb  = SWC->CreateSnazzyCheckWidget(24, IDC_CLIMB, LoadStringFromResource(IDS_CLIMBTIP));
	SWC->CreateSnazzyPushWidget(25, IDC_UNDO, LoadStringFromResource(IDS_UNDOTIP));
	SWC->CreateSnazzyPushWidget(26, IDC_EXIT, LoadStringFromResource(IDS_EXITTIP));
	SWC->CreateSnazzyPushWidget(27, IDC_HELPBUTTON, LoadStringFromResource(IDS_HELPTIP));
	//SWC->CreateSnazzyPushWidget(28, IDC_CATAGORIES, LoadStringFromResource(IDS_ENABLECATTIP));
	SWC->CreateSnazzyDragWidget(29, IDC_DRAG, LoadStringFromResource(IDS_DRAGWINTIP));
	SWC->CreateSnazzyPushWidget(30, IDCANCEL, LoadStringFromResource(IDS_CLOSETIP));
	
	// New UI
	#ifdef NVW_SUPPORT_QUERYACTION
	Query  = SWC->CreateSnazzyCheckWidget(28, IDC_ACTIONMODE, LoadStringFromResource(IDS_ACTIONTIP));
	#else // !NVW_SUPPORT_QUERYACTION
	SWC->CreateSnazzyPushWidget(28, IDC_CREDITS, LoadStringFromResource(IDS_ABOUTTIP));
	#endif // !NVW_SUPPORT_QUERYACTION
	SWC->CreateSnazzyPushWidget(31, IDC_INFO, LoadStringFromResource(IDS_INFOPANELTIP));
	SWC->CreateSnazzyPushWidget(32, IDC_SHOWDRIVE, LoadStringFromResource(IDS_DRIVEPANELTIP));


	NavNativeWindow = DlgHandle = SWC->GetContainerNativeWin();

	// position window in lower center (right of and adjacent to the Drive Dlg)
	int X, Y;
	RECT DlgRect, DriveRect;
	GetWindowRect(DlgHandle, &DlgRect);
	GetWindowRect(DriveDlg::GetDlgHandle(), &DriveRect);

	X = DriveRect.right + 1;
	Y = DriveRect.top;

	SetWindowPos(DlgHandle, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	if(GlobalTipSupport)
		{
		GlobalTipSupport->AddCallbackWin(DlgHandle);
		} // if
	
	SyncWidgets();

	SWC->Show(Show);
	
	} // if


} // NavDlg::NavDlg()


NavDlg::~NavDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // NavDlg::~NavDlg()

int NavDlg::ProcessAndHandleEvents()
{
int Status = 0;
MSG msg;

while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	if(!IsDialogMessage(DlgHandle, &msg))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		} // else
	} // while


return(Status);
} // NavDlg::ProcessAndHandleEvents()


bool NavDlg::HandleEvent(WIDGETID WidgetID)
{
bool Handled = false;

switch(WidgetID)
	{
	case IDC_EXIT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_GLOBAL, EventDispatcher::NVW_ESC_GLOBAL_EXIT); return(true);
	case IDC_PREVVIEW: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_PREV); return(true);
	case IDC_VIEWPOINTS: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NONE); return(true);
	case IDC_NEXTVIEW: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NEXT); return(true);
	case IDC_HOME: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_HOME); return(true);
	case IDC_GOTO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_GOTO); SyncWidgets(); return(true);
	case IDC_FOLLOW: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TERRAINFOLLOW); SyncWidgets(); return(true);
	case IDC_ROTATE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_ROTATE); SyncWidgets(); return(true);
	case IDC_QUERY: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_QUERY); SyncWidgets(); return(true);
	case IDC_DRIVE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_DRIVE); SyncWidgets(); return(true);
	case IDC_SLIDE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_SLIDE); SyncWidgets(); return(true);
	case IDC_CLIMB: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_CLIMB); SyncWidgets(); return(true);
	case IDC_UNDO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_UNDO); return(true);
	//case IDC_CATAGORIES: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_CATMENU); return(true);
	//case IDC_OPTIONS: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT); return(true);
	case IDC_HELPBUTTON: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_HELP); return(true);
	case IDCANCEL: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_NAV); return(true);
	
	// New UI
	#ifdef NVW_SUPPORT_QUERYACTION
	case IDC_ACTIONMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_QUERY); return(true);
	#else // !NVW_SUPPORT_QUERYACTION
	case IDC_CREDITS:  EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_CREDITS); return(true);
	#endif // !NVW_SUPPORT_QUERYACTION
	case IDC_INFO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_INFO); return(true);
	case IDC_SHOWDRIVE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_DRIVE); return(true);
	} // switch ID

return(Handled);
} // NavDlg::HandleEvent

bool NavDlg::IsShown(void)
{
if(DlgHandle)
	{
	if(IsWindowVisible(DlgHandle))
		{
		return(true);
		} // if
	} // if
return(false);
} // NavDlg::IsShown

void NavDlg::SyncWidgets(void)
{

// defer redraw until all are reconfigured
#ifdef NVW_SUPPORT_QUERYACTION
if(Query) Query->SetSelectedSilent(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_QUERY);
#endif // NVW_SUPPORT_QUERYACTION
Rotate->SetSelectedSilent(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_ROTATE);
Drive->SetSelectedSilent(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_DRIVE);
Slide->SetSelectedSilent(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_SLIDE);
Climb->SetSelectedSilent(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_CLIMB);
Follow->SetSelectedSilent(CheckFollowTerrainEnabledGlobal());
Goto->SetSelectedSilent(CheckDelayedGoto());
SWC->ForceRepaintAll(); // repaint all widgets

} // NavDlg::SyncWidgets
