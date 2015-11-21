#include <windows.h>
#include <osg/PositionAttitudeTransform>

#include "Types.h"
#include "KeyDefines.h"
#include "EventDispatcher.h"
#include "PanoManipulator.h"
#include "MainPopupMenu.h"
#include "NVScene.h"
#include "Navigation.h"
#include "Category.h"
#include "Viewpoints.h"

#include "InfoDlg.h"
#include "NavDlg.h"
#include "DriveDlg.h"
#include "NVMiscGlobals.h"

extern NVScene MasterScene;
extern NativeAnyWin NavNativeWindow;

extern osg::ref_ptr<PanoManipulator> PM;

enum
	{
	NVW_MAINPOPUPMENU_IDBASE = 60000,
	NVW_MAINPOPUPMENU_QUERYACTION,

	NVW_MAINPOPUPMENU_WIN_DRIVE,
	NVW_MAINPOPUPMENU_WIN_NAV,
	NVW_MAINPOPUPMENU_WIN_INFO,
	NVW_MAINPOPUPMENU_WIN_HELP,

	NVW_MAINPOPUPMENU_NAV_DRIVEMODE,
	NVW_MAINPOPUPMENU_NAV_SLIDEMODE,
	NVW_MAINPOPUPMENU_NAV_ROTMODE,
	NVW_MAINPOPUPMENU_NAV_QUERYMODE,
	NVW_MAINPOPUPMENU_NAV_PAUSE,
	NVW_MAINPOPUPMENU_NAV_STOP,
	NVW_MAINPOPUPMENU_NAV_HOME,
	NVW_MAINPOPUPMENU_NAV_GOTO,
	NVW_MAINPOPUPMENU_NAV_UNDO,
	NVW_MAINPOPUPMENU_NAV_FOLLOW,
	NVW_MAINPOPUPMENU_NAV_OPTMOVE,

	NVW_MAINPOPUPMENU_CREDITS,
	NVW_MAINPOPUPMENU_OPTIONS,
	NVW_MAINPOPUPMENU_EXIT,

	NVW_MAINPOPUPMENU_MAX // no further items
	}; // menu IDs



int InvokeMainPopupMenu(int XCoord, int YCoord)
{
HMENU MainPopMenu, SubWindow, SubNavigation, SubCats, SubViews;
int TrackResult;

if(MainPopMenu = CreatePopupMenu())
	{
	SubWindow     = CreatePopupMenu();
	SubNavigation = CreatePopupMenu();
	SubCats       = CreatePopupMenu();
	SubViews      = CreatePopupMenu();

	#ifdef NVW_SUPPORT_QUERYACTION
	AppendMenu(MainPopMenu, MF_STRING, NVW_MAINPOPUPMENU_QUERYACTION, "Query/Action Here"); // leave off abbrev as it's too long
	#endif // NVW_SUPPORT_QUERYACTION

	AppendMenu(SubWindow, MF_STRING | (DriveDlg::IsShown() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_WIN_DRIVE, "Drive");
	AppendMenu(SubWindow, MF_STRING | (NavDlg::IsShown() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_WIN_NAV, "Navigation");
	AppendMenu(SubWindow, MF_STRING | (InfoDlg::IsShown() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_WIN_INFO, "Info");
	AppendMenu(SubWindow, MF_STRING | (MasterScene.SceneLOD.CheckHelpEnabled() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_WIN_HELP, "Help\t"NV_KEY_HELPDESC);

	AppendMenu(MainPopMenu, MF_STRING | MF_POPUP, (unsigned int)SubWindow, "Windows");

	AppendMenu(SubNavigation, MF_STRING | (Navigation::GetCurrentNavMode() == Navigation::NAV_NM_DRIVE ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_DRIVEMODE, "Drive Mode\t"NV_KEY_MOVEMODEDESC);
	AppendMenu(SubNavigation, MF_STRING | (Navigation::GetCurrentNavMode() == Navigation::NAV_NM_SLIDE ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_SLIDEMODE, "Slide Mode\t"NV_KEY_SLIDEMODEDESC);
	AppendMenu(SubNavigation, MF_STRING | (Navigation::GetCurrentNavMode() == Navigation::NAV_NM_ROTATE ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_ROTMODE, "Rotate Mode\t"NV_KEY_ROTMODEDESC);
	#ifdef NVW_SUPPORT_QUERYACTION
	AppendMenu(SubNavigation, MF_STRING | (Navigation::GetCurrentNavMode() == Navigation::NAV_NM_QUERY ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_QUERYMODE, "Action Mode\t"NV_KEY_QUERYMODEDESC);
	#endif // NVW_SUPPORT_QUERYACTION
	AppendMenu(SubNavigation, MF_STRING, NVW_MAINPOPUPMENU_NAV_PAUSE, "Pause\t"NV_KEY_DISABLETOGGLEDESC);
	AppendMenu(SubNavigation, MF_STRING, NVW_MAINPOPUPMENU_NAV_STOP, "Stop\t"NV_KEY_STOPMOVINGDESC);
	AppendMenu(SubNavigation, MF_STRING, NVW_MAINPOPUPMENU_NAV_HOME, "Home\t"NV_KEY_RETURNHOMEDESC);
	AppendMenu(SubNavigation, MF_STRING, NVW_MAINPOPUPMENU_NAV_GOTO, "Goto");
	AppendMenu(SubNavigation, MF_STRING, NVW_MAINPOPUPMENU_NAV_UNDO, "Undo\t"NV_KEY_UNDODESC);
	AppendMenu(SubNavigation, MF_STRING | (MasterScene.CheckFollowTerrainEnabled() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_FOLLOW, "Follow Terrain\t"NV_KEY_TERRAINFOLLOWDESC);
	AppendMenu(SubNavigation, MF_STRING | (MasterScene.SceneLOD.CheckOptimizeMove() ? MF_CHECKED : 0), NVW_MAINPOPUPMENU_NAV_OPTMOVE, "Optimized Movement\t"NV_KEY_MOVEOPTDESC);

	AppendMenu(MainPopMenu, MF_STRING | MF_POPUP, (unsigned int)SubNavigation, "Navigation");

	BuildCategoryMenu(SubCats);

	AppendMenu(MainPopMenu, MF_STRING | MF_POPUP, (unsigned int)SubCats, "Categories");

	BuildViewpointMenu(SubViews);

	AppendMenu(MainPopMenu, MF_STRING | MF_POPUP, (unsigned int)SubViews, "Viewpoints");

	AppendMenu(MainPopMenu, MF_STRING, NVW_MAINPOPUPMENU_CREDITS, "About");
	//AppendMenu(MainPopMenu, MF_STRING | MF_DISABLED | MF_GRAYED, NVW_MAINPOPUPMENU_OPTIONS, "Options");
	AppendMenu(MainPopMenu, MF_STRING, NVW_MAINPOPUPMENU_EXIT, "Exit\t"NV_KEY_EXITDESC);

	if(TrackResult = TrackPopupMenu(MainPopMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_CENTERALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, XCoord, YCoord, 0, NavNativeWindow, NULL))
		{
		if(TrackResult > NVW_MAINPOPUPMENU_IDBASE && TrackResult < NVW_MAINPOPUPMENU_MAX)
			{
			// do something
			switch(TrackResult)
				{
				case NVW_MAINPOPUPMENU_QUERYACTION: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_ACTION, EventDispatcher::NVW_ESC_ACTION_QUERYHERE, EventDispatcher::NVW_PC_NONE); break;
				case NVW_MAINPOPUPMENU_WIN_DRIVE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_DRIVE); break;
				case NVW_MAINPOPUPMENU_WIN_NAV: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_NAV); break;
				case NVW_MAINPOPUPMENU_WIN_INFO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_INFO); break;
				case NVW_MAINPOPUPMENU_WIN_HELP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_HELP); break;
				// Nav
				case NVW_MAINPOPUPMENU_NAV_DRIVEMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_DRIVE); break;
				case NVW_MAINPOPUPMENU_NAV_SLIDEMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_SLIDE); break;
				case NVW_MAINPOPUPMENU_NAV_ROTMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_ROTATE); break;
				case NVW_MAINPOPUPMENU_NAV_QUERYMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_QUERY); break;
				case NVW_MAINPOPUPMENU_NAV_PAUSE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_PAUSE); break;
				case NVW_MAINPOPUPMENU_NAV_STOP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_STOP); break;
				case NVW_MAINPOPUPMENU_NAV_HOME: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_HOME); break;
				case NVW_MAINPOPUPMENU_NAV_GOTO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_GOTO); break;
				case NVW_MAINPOPUPMENU_NAV_UNDO: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_UNDO); break;
				case NVW_MAINPOPUPMENU_NAV_FOLLOW: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TERRAINFOLLOW); break;
				case NVW_MAINPOPUPMENU_NAV_OPTMOVE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_OPTIMIZEMOVE); break;

				//case NVW_MAINPOPUPMENU_OPTIONS: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_OPTIMIZEMOVE); break;
				case NVW_MAINPOPUPMENU_CREDITS: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_CREDITS); break;
				case NVW_MAINPOPUPMENU_EXIT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_GLOBAL, EventDispatcher::NVW_ESC_GLOBAL_EXIT); break;

				} // SelectedCat
			} // if
		else if(TrackResult > NVW_CATEGORYMENU_IDBASE && TrackResult < NVW_CATEGORYMENU_MAX) // category menu
			{
			HandleCategoryMenuSelection(TrackResult);
			} // else if
		else if(TrackResult > NVW_VIEWPOINTMENU_IDBASE && TrackResult < NVW_VIEWPOINTMENU_MAX) // viewpoint menu
			{
			PM->SetTransStartFromCurrent();
			if(HandleViewpointMenuSelection(TrackResult) != -1)
				{
				PM->MarkTransStartMoment();
				PM->SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // if
			} // else if
		} // if
	DestroyMenu(SubWindow);
	DestroyMenu(SubNavigation);
	DestroyMenu(SubCats);
	DestroyMenu(SubViews);
	DestroyMenu(MainPopMenu);
	} // if

return(0);
} // InvokeMainPopupMenu

