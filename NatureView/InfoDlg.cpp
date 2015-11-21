// InfoDlg.cpp
// Code for info dialog

#include <windows.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include "resource.h"
#include "InfoDlg.h"
#include "DriveDlg.h" // for centering/positioning relative to DriveDlg
#include "ToolTips.h"
#include "EventDispatcher.h"
#include "NVEventHandler.h"
#include "InstanceSupport.h"
#include "ResourceSupport.h"
#include "Viewer.h"

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include "SnazzyWidgetOSG.h"
#include "SnazzyWidgetJPGRC.h"

extern ToolTipSupport *GlobalTipSupport;
extern SnazzyWidgetContainerFactory *SWCF;


extern char ProgDir[500];

NativeGUIWin InfoDlg::DlgHandle = NULL;

InfoDlgSWEventObserver InfoDlg::CallbackObserver;
SnazzyTextWidget *InfoDlg::Text = NULL;
SnazzyWidgetContainer *InfoDlg::SWC = NULL;
double InfoDlg::LastUpdateMomentSeconds;

bool InfoDlg::HandleEvent(WIDGETID WidgetID)
{
bool Handled = false;

switch(WidgetID)
	{
	case IDCANCEL: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_INFO); return(true);
	} // switch ID

return(Handled);
} // InfoDlg::HandleEvent


void InfoDlg::Show(bool NewState)
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

} // InfoDlg::Show



InfoDlg::InfoDlg(bool Show)
{
DlgHandle = NULL;

UpdateLastMoment();

// $CH <<<>>> these should be elsewhere
SnazzyWidgetImageCollection *SWIC = NULL;
//char ImagePath[1024];

//osg::ref_ptr<osg::Image> Index;

//strcpy(ImagePath, ProgDir);
//strcat(ImagePath, "/TempResource/InfoPanelIdx.png");
//Index = osgDB::readImageFile(ImagePath);


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

LoadJPGImageToSnazzyWidgetNormal(IDR_INFOPANEL_NORMAL_JPEG, SWIC);
LoadJPGImageToSnazzyWidgetNormalHover(IDR_INFOPANEL_NORMAL_HOVER_JPEG, SWIC);

// Selected imagery looks just like Normal, so don't load it, widget will automaticaly use Normal instead
//LoadJPGImageToSnazzyWidgetSelected(IDR_INFOPANEL_SEL_JPEG, SWIC);
//LoadJPGImageToSnazzyWidgetSelectedHover(IDR_INFOPANEL_SEL_HOVER_JPEG, SWIC);
LoadPNGImageToSnazzyWidgetIndex(IDR_INFOPANEL_INDEX_PNG, SWIC);

//CopyImageToSnazzyWidgetIndex(Index.get(), SWIC);

if(SWC = SWCF->CreateSnazzyWidgetContainer(GetGlobalViewerHWND(), &CallbackObserver, SWIC))
	{
	SWC->CreateSnazzyPushWidget(52, IDCANCEL, LoadStringFromResource(IDS_CLOSETIP));
	SWC->CreateSnazzyDragWidget(51, IDC_DRAG, LoadStringFromResource(IDS_DRAGWINTIP));
	Text = SWC->CreateSnazzyTextWidget(50, 0, LoadStringFromResource(IDS_INFOTIP));

	DlgHandle = SWC->GetContainerNativeWin();

	// position window in lower left, just to left of DriveDlg
	int X, Y;
	RECT DlgRect, DriveRect;
	GetWindowRect(DlgHandle, &DlgRect);
	GetWindowRect(DriveDlg::GetDlgHandle(), &DriveRect);

	X = (DriveRect.left - (DlgRect.right - DlgRect.left)) - 1;
	Y = DriveRect.top;

	SetWindowPos(DlgHandle, NULL, X, Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	if(GlobalTipSupport)
		{
		GlobalTipSupport->AddCallbackWin(DlgHandle);
		} // if

	SWC->Show(Show);
	} // if

} // InfoDlg::InfoDlg()


InfoDlg::~InfoDlg()
{

if(DlgHandle) DestroyWindow(DlgHandle);
DlgHandle = NULL;

} // InfoDlg::~InfoDlg()


int InfoDlg::SetNewText(const char *NewText)
{
// suppress new updates that are too close in time to last update to prevent flicker
if(GetCurrentMoment() - LastUpdateMomentSeconds > NVW_INFODLG_UPDATE_TIME_MIN)
	{
	if(Text)
		{
		Text->SetText(NewText);
		} // if
	UpdateLastMoment();
	} // if
return(1);
} // InfoDlg::SetNewText



bool InfoDlg::IsShown(void)
{
if(DlgHandle)
	{
	if(IsWindowVisible(DlgHandle))
		{
		return(true);
		} // if
	} // if
return(false);
} // InfoDlg::IsShown


void InfoDlg::UpdateLastMoment(void)
{
LastUpdateMomentSeconds = GetCurrentMoment(); 
} // InfoDlg::UpdateLastMoment


double InfoDlg::GetCurrentMoment(void)
{
struct _timeb Now;
double Result;

_ftime(&Now);
Result = (double)Now.time + ((double)Now.millitm * 0.001); 
return(Result);
} // InfoDlg::GetCurrentMoment