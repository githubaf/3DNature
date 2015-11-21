// DriveDlg.cpp
// Code for navigation dialog

#include <windows.h>

#include <osg/PositionAttitudeTransform>


#include "resource.h"
#include "DriveDlg.h"
#include "ToolTips.h"
#include "EventDispatcher.h"
#include "WindowDefaults.h"
#include "CommCtrl.h"
#include "NVScene.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "ResourceSupport.h"
#include "NVMiscGlobals.h"
#include "Viewer.h"

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include "SnazzyWidgetOSG.h"
#include "SnazzyWidgetJPGRC.h"

extern ToolTipSupport *GlobalTipSupport;
extern SnazzyWidgetContainerFactory *SWCF;
extern NVScene MasterScene;

extern char ProgDir[500];

NativeGUIWin DriveDlg::DlgHandle = NULL;
DriveDlgSWEventObserver DriveDlg::CallbackObserver;
SnazzySlideYWidget *DriveDlg::Throttle = NULL;
SnazzyWidgetContainer *DriveDlg::SWC = NULL;
SnazzyHotPadWidget *DriveDlg::HotNav = NULL;
SnazzyHotPadWidget *DriveDlg::HotTurn = NULL;


#define NVW_DRIVEDLG_SPEEDSLIDER_QUANTA	20



/*
BOOL CALLBACK DriveDlgDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

int X = 0;

switch(uMsg)
	{
	case WM_VSCROLL:
		{
		int nScrollCode;
		short int nPos;
		HWND hwndScrollBar;
		int TickNum;
		double ThrottleFrac;

		nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
		nPos = (short int) HIWORD(wParam);   // scroll box position 
		hwndScrollBar = (HWND) lParam;       // handle to scroll bar
		if(nScrollCode == TB_THUMBPOSITION || nScrollCode == TB_THUMBTRACK)
			{
			TickNum = NVW_DRIVEDLG_SPEEDSLIDER_QUANTA - nPos; // stoopid top=0 style of slider...
			ThrottleFrac = (double)TickNum / (double)NVW_DRIVEDLG_SPEEDSLIDER_QUANTA;
			MasterScene.SetThrottleSpeed(MasterScene.GetMaxSpeed() * ThrottleFrac);

			} // if
		return(0);
		break;
		} // WM_VSCROLL
	} // switch

return(0);
} // DriveDlgDialogProc
*/


void DriveDlg::Show(bool NewState)
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

} // DriveDlg::Show




DriveDlg::DriveDlg(bool Show)
{
DlgHandle = NULL;
// $CH <<<>>> these should be elsewhere
SnazzyWidgetImageCollection *SWIC = NULL;
SnazzyWidgetImage *SWIK = NULL;
//char ImagePath[1024];

//osg::ref_ptr<osg::Image> Index, SlideKnob;
/*
strcpy(ImagePath, ProgDir);
strcat(ImagePath, "/TempResource/DrivePanelIdx.png");
Index = osgDB::readImageFile(ImagePath);
strcpy(ImagePath, ProgDir);
strcat(ImagePath, "/TempResource/Widget14Knob.png");
SlideKnob = osgDB::readImageFile(ImagePath);
*/
try
	{
	SWIC = new SnazzyWidgetImageCollection(260, 96); // can we get this dynamically somehow?
	SWIK = new SnazzyWidgetImage(21, 11); // can we get this dynamically somehow?
	} // try
catch(std::bad_alloc)
	{
	// $CH <<<>>> These should be done in cleanup too
	delete SWIC;
	delete SWIK;
	return;
	} // catch

//CopyImageToSnazzyWidgetIndex(Index.get(), SWIC);
//CopyImageToSnazzyWidgetNormal(SlideKnob.get(), SWIK);
LoadPNGImageToSnazzyWidgetNormal(IDR_SLIDE_KNOB_PNG, SWIK);
LoadPNGImageToSnazzyWidgetIndex(IDR_DRIVEPANEL_INDEX_PNG, SWIC);

LoadJPGImageToSnazzyWidgetNormal(IDR_DRIVEPANEL_NORMAL_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetNormalHover(IDR_DRIVEPANEL_NORMAL_HOVER_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetSelected(IDR_DRIVEPANEL_SEL_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetSelectedHover(IDR_DRIVEPANEL_SEL_HOVER_JPEG, SWIC);


if(SWC = SWCF->CreateSnazzyWidgetContainer(GetGlobalViewerHWND(), &CallbackObserver, SWIC))
	{
	SWC->CreateSnazzyPushWidget(1,  IDC_CATAGORIES, LoadStringFromResource(IDS_ENABLECATTIP));
	SWC->CreateSnazzyCheckWidget(2,  IDC_PAUSE, LoadStringFromResource(IDS_PAUSETIP));
	SWC->CreateSnazzyPushWidget(3,  IDC_STOP, LoadStringFromResource(IDS_STOPTIP));
	//SWC->CreateSnazzyPushWidget(4,  IDC_ROTLT, LoadStringFromResource(IDS_ROTLTTIP));
	//SWC->CreateSnazzyPushWidget(5,  IDC_MVLT, LoadStringFromResource(IDS_MVLTTIP));
	SWC->CreateSnazzyPushWidget(6,  IDC_EYEPOINTS, LoadStringFromResource(IDS_EYEPOINTSTIP));
	//SWC->CreateSnazzyPushWidget(7,  IDC_MVFWD, LoadStringFromResource(IDS_MVFWDTIP));
	SWC->CreateSnazzyPushWidget(8,  IDC_HOME, LoadStringFromResource(IDS_HOMETIP));
	//SWC->CreateSnazzyPushWidget(9,  IDC_MVBK, LoadStringFromResource(IDS_MVBKTIP));
	//SWC->CreateSnazzyPushWidget(10, IDC_ROTRT, LoadStringFromResource(IDS_ROTRTTIP));
	//SWC->CreateSnazzyPushWidget(11, IDC_MVRT, LoadStringFromResource(IDS_MVRTTIP));
	//SWC->CreateSnazzyPushWidget(12, IDC_ROTUP, LoadStringFromResource(IDS_ROTUPTIP));
	//SWC->CreateSnazzyPushWidget(13, IDC_ROTDN, LoadStringFromResource(IDS_ROTDNTIP));
	Throttle = SWC->CreateSnazzySlideYWidget(14, IDC_SPEEDSLIDE, SWIK, LoadStringFromResource(IDS_THROTTLETIP));
	SWC->CreateSnazzyDragWidget(15, IDC_DRAG, LoadStringFromResource(IDS_DRAGWINTIP));
	SWC->CreateSnazzyPushWidget(16, IDCANCEL, LoadStringFromResource(IDS_CLOSETIP));
	HotNav = SWC->CreateSnazzyHotPadWidget(17, IDC_HOTNAV, NULL, LoadStringFromResource(IDS_HOTNAVTIP));
	HotNav->SetExponential(true);
	
	// New UI only
	HotTurn = SWC->CreateSnazzyHotPadWidget(4, IDC_HOTTURN, NULL, LoadStringFromResource(IDS_HOTTURNTIP));
	HotTurn->SetExponential(true);
	SWC->CreateSnazzyPushWidget(9,  IDC_INFO, LoadStringFromResource(IDS_INFOPANELTIP)); // Info
	SWC->CreateSnazzyPushWidget(10, IDC_HELPBUTTON, LoadStringFromResource(IDS_HELPTIP)); // Help
	SWC->CreateSnazzyPushWidget(11, IDC_NAV, LoadStringFromResource(IDS_NAVPANELTIP)); // Nav

	DlgHandle = SWC->GetContainerNativeWin();

	// position window in lower center
	RECT ViewerRect, DlgRect;
	int X, Y;

	GetWindowRect(GetGlobalViewerHWND(), &ViewerRect);
	GetWindowRect(DlgHandle, &DlgRect);
	
	// determine positioning of Drive window that Nav/Info windows anchor to by default
	if(GetToolWindowLocation() == Top)
		{
		X = ((ViewerRect.right - ViewerRect.left) / 2) - ((DlgRect.right - DlgRect.left) / 2);
		Y = 0;
		} // if
	else
		{ // bottom
		X = ((ViewerRect.right - ViewerRect.left) / 2) - ((DlgRect.right - DlgRect.left) / 2);
		Y = (ViewerRect.bottom - ViewerRect.top) - (DlgRect.bottom - DlgRect.top);
		} // else
	SetWindowPos(DlgHandle, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	if(GlobalTipSupport)
		{
		GlobalTipSupport->AddCallbackWin(DlgHandle);
		} // if

	SWC->Show(Show);
	
	} // if

} // DriveDlg::DriveDlg()


DriveDlg::~DriveDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // DriveDlg::~DriveDlg()

bool DriveDlg::HandleEvent(WIDGETID WidgetID)
{
bool Handled = false;

switch(WidgetID)
	{
	case IDC_PAUSE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_PAUSE); return(true);
	case IDC_STOP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_STOP); return(true);
	case IDC_HOME: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_HOME); return(true);

	case IDC_MVFWD: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_FORWARD); return(true);
	case IDC_MVBK: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_BACKWARD); return(true);
	case IDC_MVLT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_LEFT); return(true);
	case IDC_MVRT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_RIGHT); return(true);

	case IDC_EYEPOINTS: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NONE); return(true);
	case IDC_CATAGORIES: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_CATMENU); return(true);

	case IDC_ROTUP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_UP); return(true);
	case IDC_ROTDN: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_DOWN); return(true);
	case IDC_ROTLT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_LEFT); return(true);
	case IDC_ROTRT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_RIGHT); return(true);
	
	case IDC_SPEEDSLIDE: MasterScene.SetThrottleSpeed(MasterScene.GetMaxSpeed() * Throttle->GetPositionY()); return(true);
	
	case IDC_HOTNAV: MasterScene.SetHotNavSideAmount(HotNav->GetPositionX()); MasterScene.SetHotNavFwdBackAmount(HotNav->GetPositionY()); return(true);
	
	// New UI
	case IDC_HOTTURN: MasterScene.SetHotTurnHAmount(HotTurn->GetPositionX()); MasterScene.SetHotTurnVAmount(HotTurn->GetPositionY()); return(true);
	case IDC_INFO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_INFO); return(true);
	case IDC_HELPBUTTON: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_HELP); return(true);
	case IDC_NAV: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_NAV); return(true);

	case IDCANCEL: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_DRIVE); return(true);
	} // switch ID

return(Handled);
} // DriveDlg::HandleEvent


void DriveDlg::SyncWidgets(void)
{
float ThrottleFrac;

if(DlgHandle && Throttle)
	{
	if(MasterScene.GetMaxSpeed() > 0.0)
		{
		ThrottleFrac = (float)MasterScene.GetThrottleSpeed() / (float)MasterScene.GetMaxSpeed();
		Throttle->SetPositionY(ThrottleFrac);
		} // if
	else
		{
		Throttle->SetPositionY(0.0f);
		} // else
	} // if
} // DriveDlg::SyncWidgets


bool DriveDlg::IsShown(void)
{
if(DlgHandle)
	{
	if(IsWindowVisible(DlgHandle))
		{
		return(true);
		} // if
	} // if
return(false);
} // DriveDlg::IsShown
