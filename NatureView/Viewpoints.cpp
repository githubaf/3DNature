
#include <windows.h>
#include <osg/PositionAttitudeTransform>

#include "Types.h"
#include "KeyDefines.h"
#include "Viewpoints.h"
#include "NVScene.h"
#include "NVMiscGlobals.h"
#include "EventDispatcher.h"


extern NVScene MasterScene;
extern NativeAnyWin NavNativeWindow;


void BuildViewpointMenu(void *MenuHandle)
{
HMENU ViewpointMenu;
int NumViewpoints;

if(ViewpointMenu = (HMENU)MenuHandle)
	{
	int VPID = NVW_VIEWPOINTMENU_IDBASE + 1;
	AppendMenu(ViewpointMenu, MF_STRING | MF_DISABLED, NVW_VIEWPOINTMENU_IDBASE, "Viewpoints");
	AppendMenu(ViewpointMenu, MF_SEPARATOR, NULL, NULL);
	if(NumViewpoints = MasterScene.GetNumCameras())
		{
		for(int CurVP = 0; CurVP < NumViewpoints; CurVP++)
			{
			NVAnimObject *CurCam;
			CurCam = MasterScene.GetCamera(CurVP);
			if(CurCam->GetIsAnimated())
				{
				char CamName[200];
				sprintf(CamName, "\xBB %s", CurCam->GetName());
				AppendMenu(ViewpointMenu, MF_STRING, VPID++, CamName);
				} // if
			else
				{
				AppendMenu(ViewpointMenu, MF_STRING, VPID++, CurCam->GetName());
				} // else
			} // for
		} // if
	else
		{
		AppendMenu(ViewpointMenu, MF_STRING | MF_DISABLED, NVW_VIEWPOINTMENU_IDBASE, "(none available)");
		} // else

	AppendMenu(ViewpointMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(ViewpointMenu, MF_STRING, NVW_VIEWPOINTMENU_VPNEXT, "Next Viewpoint\t"NV_KEY_NEXTCAMDESC);
	AppendMenu(ViewpointMenu, MF_STRING, NVW_VIEWPOINTMENU_VPPREV, "Prev Viewpoint\t"NV_KEY_PREVCAMDESC);
	AppendMenu(ViewpointMenu, MF_STRING | (GetTourInProgress() ? MF_CHECKED : 0), NVW_VIEWPOINTMENU_VPTOUR, "Viewpoint Tour\t"NV_KEY_VIEWTOURDESC);
	} // if

} // BuildViewpointMenu

int HandleViewpointMenuSelection(int MenuID)
{
int NewViewpoint;

if(MenuID > NVW_VIEWPOINTMENU_IDBASE)
	{
	if(MenuID == NVW_VIEWPOINTMENU_VPTOUR)
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_TOGGLE);
		} // if
	else if(MenuID == NVW_VIEWPOINTMENU_VPNEXT)
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NEXT);
		} // if
	else if(MenuID == NVW_VIEWPOINTMENU_VPPREV)
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_PREV);
		} // if
	NewViewpoint = MenuID - (NVW_VIEWPOINTMENU_IDBASE + 1); // (NVW_VIEWPOINTMENU_IDBASE + 1) = VP0
	if(NewViewpoint < MasterScene.GetNumCameras())
		{
		MasterScene.SetCurrentCameraNum(NewViewpoint); // BAM!
		return(NewViewpoint);
		} // if
	} // if

return(-1);
} // HandleViewpointMenuSelection




int SelectNewCameraViaMenu(int XCoord, int YCoord)
{
HMENU ViewpointMenu;
int TrackResult;

if(ViewpointMenu = CreatePopupMenu())
	{
	BuildViewpointMenu(ViewpointMenu);

	if(TrackResult = TrackPopupMenu(ViewpointMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_VCENTERALIGN | TPM_RIGHTBUTTON, XCoord, YCoord, 0, NavNativeWindow, NULL))
		{
		if(TrackResult > NVW_VIEWPOINTMENU_IDBASE)
			{
			return(HandleViewpointMenuSelection(TrackResult));
			} // if
		} // if
	} // if

return(-1);
} // SelectNewCameraViaMenu

