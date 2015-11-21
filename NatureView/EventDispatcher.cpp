// EventDispatcher.cpp

#include <osg/PositionAttitudeTransform>
#include <osgProducer/Viewer>
#include <osgProducer/ViewerEventHandler>
#include <osgGA/StateSetManipulator>
#include "ConfigDefines.h"

#include "EventDispatcher.h"
#include "NVScene.h"
#include "Viewer.h"
#include "PanoManipulator.h"
#include "SwitchBox.h"
#include "CreateHelpSplashWatermark.h"
#include "Navigation.h"
#include "Viewpoints.h"
#include "Category.h"
#include "NVMiscGlobals.h"
#include "DriveDlg.h"
#include "InfoDlg.h"
#include "NavDlg.h"
#include "MainPopupMenu.h"
#include "MediaSupport.h"
#include "NVQueryAction.h"
#include "Credits.h"
#include "HTMLDlg.h"
#include "HelpDlg.h"
#include "DataDlg.h"
#include "ImageDlg.h"


// picked up from main
void SignalApplicationExit(void);

extern NVScene MasterScene;
extern osg::ref_ptr<PanoManipulator> PM;
// picked up from NVSky.cpp
extern osg::ref_ptr<osg::ClearNode> clearNode;


extern bool MovementDisabled;
extern SwitchBox *MasterSwitches;

unsigned long int EventDispatcher::MousePosX = 0, EventDispatcher::MousePosY = 0;
float EventDispatcher::MousePosXf = 0.0f, EventDispatcher::MousePosYf = 0.0f;

bool EventDispatcher::DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor)
{

return(DispatchEvent(Major, Minor, NVW_PC_NONE, ""));

} // EventDispatcher::DispatchEvent



bool EventDispatcher::DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor, char *EventParam)
{

// <<<>>> Parse recognizable leading parts of EventParam, if possible

return(DispatchEvent(Major, Minor, NVW_PC_NONE, EventParam));

} // EventDispatcher::DispatchEvent



bool EventDispatcher::DispatchEvent(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

POINT MousePos;
if(GetCursorPos(&MousePos))
	{
	MousePosX = MousePos.x;
	MousePosY = MousePos.y;
	} // if

switch(Major)
	{
	case NVW_EC_GLOBAL: return(DispatchEventGLOBAL(Major, Minor, ParamCode, EventParam));
	case NVW_EC_SCENE: return(DispatchEventSCENE(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_NAVIGATION: return(DispatchEventNAVIGATION(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_UI: return(DispatchEventUI(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_LOD: return(DispatchEventLOD(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_ACTION: return(DispatchEventACTION(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_MEDIA: return(DispatchEventMEDIA(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_DEBUG: return(DispatchEventDEBUG(Major, Minor, ParamCode, EventParam)); break;
	case NVW_EC_MISC: return(DispatchEventMISC(Major, Minor, ParamCode, EventParam)); break;
	} // switch Major

return(Success);
} // EventDispatcher::DispatchEvent




bool EventDispatcher::DispatchEventGLOBAL(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_GLOBAL_EXIT: SignalApplicationExit(); return(true);
	} // switch Minor

return(Success);
} // EventDispatcher::DispatchEventGLOBAL


bool EventDispatcher::DispatchEventSCENE(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_SCENE_ALTDRAPE:
		{
		return(true);
		} // NVW_ESC_SCENE_ALTDRAPE
	case NVW_ESC_SCENE_VECTOR: MasterScene.SceneLOD.ToggleVecs(); return(true);
	case NVW_ESC_SCENE_OBJECT: MasterScene.SceneLOD.ToggleObjects(); return(true);
	case NVW_ESC_SCENE_FOLIAGE: MasterScene.SceneLOD.ToggleFoliage(); return(true);
	case NVW_ESC_SCENE_LABEL: MasterScene.SceneLOD.ToggleLabels(); return(true);
	case NVW_ESC_SCENE_OCEAN: MasterScene.SceneLOD.ToggleOcean(); return(true);
	case NVW_ESC_SCENE_TERRAIN: MasterScene.SceneLOD.ToggleTerrain(); return(true);
	case NVW_ESC_SCENE_OVERLAY: MasterScene.Overlay.ToggleOverlayState(); MasterSwitches->EnableHUD(MasterScene.Overlay.GetOverlayOn()); return(true);
	case NVW_ESC_SCENE_SPLASH: MasterSwitches->EnableSplash(false); SetSplashDisplayed(false); return(true);
	case NVW_ESC_SCENE_CATMENU:
		{
		if(SelectCategoryViaMenu(MousePosX, MousePosY) != -1)
			{
			//nothin' to do
			} // if
		return(true);
		} // NVW_ESC_SCENE_CATMENU
	} // switch Minor

return(Success);
} // EventDispatcher::DispatchEventSCENE



bool EventDispatcher::DispatchEventNAVIGATION(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_NAVIGATION_THROTTLE:
		{
		switch(ParamCode)
			{
			case NVW_PC_UP:
				{
				MasterScene.IncreaseThrottleSpeed();
				DriveDlg::SyncWidgets();
				break;
				} // UP
			case NVW_PC_DOWN:
				{
				MasterScene.DecreaseThrottleSpeed();
				DriveDlg::SyncWidgets();
				break;
				} // DOWN
			case NVW_PC_FULL:
				{
				MasterScene.SetThrottleSpeed(MasterScene.GetMaxSpeed());
				DriveDlg::SyncWidgets();
				break;
				} // FULL
			case NVW_PC_STOP:
			case NVW_PC_ZERO:
				{
				MasterScene.SetThrottleSpeed(0.0f);
				DriveDlg::SyncWidgets();
				break;
				} // ZERO
			} // switch
		break;
		} // NVW_ESC_NAVIGATION_THROTTLE
	case NVW_ESC_NAVIGATION_GOTO: ToggleDelayedGoto(); return(true);
	case NVW_ESC_NAVIGATION_OPTIMIZEMOVE: MasterScene.SceneLOD.ToggleOptimizeMove(); return(true);
	case NVW_ESC_NAVIGATION_TERRAINFOLLOW: MasterScene.ToggleFollowTerrainEnabled(); return(true);
	case NVW_ESC_NAVIGATION_HOME: PM->DoAction(PanoManipulator::PM_AC_HOME); return(true);
	case NVW_ESC_NAVIGATION_PAUSE: SetMovementLockoutGlobal(!GetMovementLockoutGlobal()); return(true);
	case NVW_ESC_NAVIGATION_VIEWPOINT:
		{
		if(ParamCode == NVW_PC_NEXT)
			{
			if(MasterScene.SelectNextCamera())
				{
				PM->SetTransStartFromCurrent(); 
				PM->MarkTransStartMoment();
				PM->SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // if
			} // if
		else if(ParamCode == NVW_PC_PREV)
			{
			if(MasterScene.SelectPrevCamera())
				{
				PM->SetTransStartFromCurrent(); 
				PM->MarkTransStartMoment();
				PM->SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // if
			} // else
		else if(ParamCode == NVW_PC_NONE)
			{
			if(SelectNewCameraViaMenu(MousePosX, MousePosY) != -1)
				{
				PM->SetTransStartFromCurrent(); 
				PM->MarkTransStartMoment();
				PM->SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // if
			} // else
		else if(ParamCode == NVW_PC_START)
			{
			PM->DoAction(PanoManipulator::PM_AC_STARTTOUR);
			} // if
		else if(ParamCode == NVW_PC_STOP)
			{
			PM->DoAction(PanoManipulator::PM_AC_ENDTOUR);
			} // if
		else if(ParamCode == NVW_PC_TOGGLE)
			{
			if(GetTourInProgress())
				{
				PM->DoAction(PanoManipulator::PM_AC_ENDTOUR);
				} // if
			else
				{
				PM->DoAction(PanoManipulator::PM_AC_STARTTOUR);
				} // else
			} // if
		return(true);
		} // VIEWPOINT
	case NVW_ESC_NAVIGATION_ZOOM:
		{
		double lensleft, lensright, lenstop, lensbottom, zNear, zFar;
		float fovx, fovy, newfovx, newfovy;
		GetGlobalViewer()->getLensParams(lensleft, lensright, lenstop, lensbottom, zNear, zFar);
		fovx = GetGlobalViewer()->getLensHorizontalFov();
		fovy = GetGlobalViewer()->getLensVerticalFov();

		if(ParamCode == NVW_PC_IN)
			{
			newfovx = fovx / NVW_ZOOM_RATE_FACTOR;
			newfovy = fovy / NVW_ZOOM_RATE_FACTOR;
			if(newfovx > 1 && newfovy > 1)
				{
				fovx = newfovx;
				fovy = newfovy;
				} // if
			} // if
		else if(ParamCode == NVW_PC_OUT)
			{
			newfovx = fovx * NVW_ZOOM_RATE_FACTOR;
			newfovy = fovy * NVW_ZOOM_RATE_FACTOR;
			if(newfovx < 180 && newfovy < 180)
				{
				fovx = newfovx;
				fovy = newfovy;
				} // if
			} // else
		GetGlobalViewer()->setLensPerspective(fovx, fovy, zNear, zFar);
		return(true);
		} // NVW_ESC_NAVIGATION_ZOOM
	case NVW_ESC_NAVIGATION_MOVEIMMED:
		{
		switch(ParamCode)
			{
			case NVW_PC_FORWARD: PM->DoAction(PanoManipulator::PM_AC_MOVEFORWARD); return(true);
			case NVW_PC_BACKWARD: PM->DoAction(PanoManipulator::PM_AC_MOVEBACKWARD); return(true);
			case NVW_PC_LEFT: PM->DoAction(PanoManipulator::PM_AC_MOVELEFT); return(true);
			case NVW_PC_RIGHT: PM->DoAction(PanoManipulator::PM_AC_MOVERIGHT); return(true);
			case NVW_PC_UP: PM->DoAction(PanoManipulator::PM_AC_MOVEUP); return(true);
			case NVW_PC_DOWN: PM->DoAction(PanoManipulator::PM_AC_MOVEDOWN); return(true);
			} // switch
		} // NVW_ESC_NAVIGATION_MOVEIMMED
	case NVW_ESC_NAVIGATION_TURNIMMED:
		{
		switch(ParamCode)
			{
			case NVW_PC_CLOCKWISE: PM->DoAction(PanoManipulator::PM_AC_BANKCLOCKWISE); return(true);
			case NVW_PC_COUNTERCLOCKWISE: PM->DoAction(PanoManipulator::PM_AC_BANKCOUNTERCLOCKWISE); return(true);
			case NVW_PC_LEFT: PM->DoAction(PanoManipulator::PM_AC_TURNLEFT); return(true);
			case NVW_PC_RIGHT: PM->DoAction(PanoManipulator::PM_AC_TURNRIGHT); return(true);
			case NVW_PC_UP: PM->DoAction(PanoManipulator::PM_AC_TILTUP); return(true);
			case NVW_PC_DOWN: PM->DoAction(PanoManipulator::PM_AC_TILTDOWN); return(true);
			} // switch
		} // NVW_ESC_NAVIGATION_TURNIMMED
	case NVW_ESC_NAVIGATION_STOP: PM->DoAction(PanoManipulator::PM_AC_HALT); return(true);
	case NVW_ESC_NAVIGATION_UNDO: PM->DoAction(PanoManipulator::PM_AC_UNDO); return(true);
	case NVW_ESC_NAVIGATION_DRIVE: Navigation::SetCurrentNavMode(Navigation::NAV_NM_DRIVE); NavDlg::SyncWidgets(); return(true);
	case NVW_ESC_NAVIGATION_SLIDE: Navigation::SetCurrentNavMode(Navigation::NAV_NM_SLIDE); NavDlg::SyncWidgets(); return(true);
	case NVW_ESC_NAVIGATION_CLIMB: Navigation::SetCurrentNavMode(Navigation::NAV_NM_CLIMB); NavDlg::SyncWidgets(); return(true);
	case NVW_ESC_NAVIGATION_ROTATE: Navigation::SetCurrentNavMode(Navigation::NAV_NM_ROTATE); NavDlg::SyncWidgets(); return(true);
	case NVW_ESC_NAVIGATION_QUERY: Navigation::SetCurrentNavMode(Navigation::NAV_NM_QUERY); NavDlg::SyncWidgets(); return(true);
	} // switch Minor


return(Success);
} // EventDispatcher::DispatchEventNAVIGATION



bool EventDispatcher::DispatchEventUI(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_UI_CREDITS: DisplayCredits(); return(true);
	case NVW_ESC_UI_HELP: MasterScene.SceneLOD.ToggleHelp(); return(true);
	case NVW_ESC_UI_MAINPOPUP:
		{
		if(InvokeMainPopupMenu(MousePosX, MousePosY) != -1)
			{
			//nothin' to do
			} // if
		return(true);
		} // NVW_ESC_UI_MAINPOPUP
	case NVW_ESC_UI_NAV: NavDlg::Show(!NavDlg::IsShown()); return(true);
	case NVW_ESC_UI_INFO: InfoDlg::Show(!InfoDlg::IsShown()); return(true);
	case NVW_ESC_UI_DRIVE: DriveDlg::Show(!DriveDlg::IsShown()); return(true);
	case NVW_ESC_UI_HIDEALLWIN:
		{
		if(NavDlg::IsShown() || InfoDlg::IsShown() || DriveDlg::IsShown()) // are any visible? Hide them!
			{
			// hide all windows
			NavDlg::Show(0);
			InfoDlg::Show(0);
			DriveDlg::Show(0);
			HTMLDlg::Show(0);
			HelpDlg::Show(0);
			DataDlg::Show(0);
			ImageDlg::Show(0);
			} // if
		else
			{
			// show all windows
			NavDlg::Show(1);
			InfoDlg::Show(1);
			DriveDlg::Show(1);
			} // if
		} // NVW_ESC_UI_HIDEALLWIN
	case NVW_ESC_UI_FRAMESTATS:
		{
		// toggle framerate display
		MasterScene.SceneLOD.SetShowFramerate(!MasterScene.SceneLOD.GetShowFramerate());
		// toggle OSG frame stats
		for(osgProducer::Viewer::EventHandlerList::iterator itr = GetGlobalViewer()->getEventHandlerList().begin();
		 itr != GetGlobalViewer()->getEventHandlerList().end(); ++itr)
			{
			osgProducer::ViewerEventHandler* viewerEventHandler = dynamic_cast<osgProducer::ViewerEventHandler*>(itr->get());
			if (viewerEventHandler)
				{
				viewerEventHandler->setFrameStatsMode(MasterScene.SceneLOD.GetShowFramerate() ? osgProducer::ViewerEventHandler::SCENE_STATS : osgProducer::ViewerEventHandler::NO_STATS);
				} // if
			} // for

		} // NVW_ESC_UI_FRAMESTATS
	} // switch Minor

return(Success);
} // EventDispatcher::DispatchEventUI

bool EventDispatcher::DispatchEventLOD(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_LOD_MINFEATURESIZE:
		{
		if(ParamCode == NVW_PC_MORE)
			{
			if(MasterScene.SceneLOD.GetMinFeatureSizePixels() == 0)
				{
				MasterScene.SceneLOD.SetMinFeatureSizePixels(1.0);
				} // if
			else
				{
				MasterScene.SceneLOD.SetMinFeatureSizePixels(MasterScene.SceneLOD.GetMinFeatureSizePixels() * 1.5);
				} // else
			osgProducer::OsgSceneHandler* scenehandler = GetGlobalViewer()->getSceneHandlerList()[0].get();
			osgUtil::SceneView* sv = scenehandler->getSceneView();
			sv->setSmallFeatureCullingPixelSize(MasterScene.SceneLOD.GetMinFeatureSizePixels());
			} // if
		else if(ParamCode == NVW_PC_LESS)
			{
			if(MasterScene.SceneLOD.GetMinFeatureSizePixels() > 0)
				{
				MasterScene.SceneLOD.SetMinFeatureSizePixels(MasterScene.SceneLOD.GetMinFeatureSizePixels() * .6666667);
				} // if
			osgProducer::OsgSceneHandler* scenehandler = GetGlobalViewer()->getSceneHandlerList()[0].get();
			osgUtil::SceneView* sv = scenehandler->getSceneView();
			sv->setSmallFeatureCullingPixelSize(MasterScene.SceneLOD.GetMinFeatureSizePixels());
			} // else
		return(true);
		} // NVW_ESC_LOD_MINFEATURESIZE
	case NVW_ESC_LOD_DETAIL:
		{
		if(ParamCode == NVW_PC_MORE)
			{
			if(MasterScene.SceneLOD.GetLODScalingFactor() > 0.0)
				{
				MasterScene.SceneLOD.SetLODScalingFactor(MasterScene.SceneLOD.GetLODScalingFactor() * .5);
				} // if
			GetGlobalViewer()->setLODScale(MasterScene.SceneLOD.GetLODScalingFactor() * MasterScene.SceneLOD.GetLODAutoFactor());
			} // if
		else if(ParamCode == NVW_PC_LESS)
			{
			if(MasterScene.SceneLOD.GetLODScalingFactor() == 0.0f)
				{
				MasterScene.SceneLOD.SetLODScalingFactor(1.0f);
				} // if
			else
				{
				MasterScene.SceneLOD.SetLODScalingFactor(MasterScene.SceneLOD.GetLODScalingFactor() * 2.0);
				} // else
			GetGlobalViewer()->setLODScale(MasterScene.SceneLOD.GetLODScalingFactor() * MasterScene.SceneLOD.GetLODAutoFactor());
			} // else
		return(true);
		} // NVW_ESC_LOD_DETAIL
	case NVW_ESC_LOD_MAINTAINRATE:
		{
		if(ParamCode == NVW_PC_ON)
			{
			MasterScene.SceneLOD.SetFramerateMaintain(true);
			} // if
		else if(ParamCode == NVW_PC_OFF)
			{
			MasterScene.SceneLOD.SetFramerateMaintain(false);
			MasterScene.SceneLOD.SetLODAutoFactor(1.0f); // reset to default
			GetGlobalViewer()->setLODScale(MasterScene.SceneLOD.GetLODScalingFactor() * MasterScene.SceneLOD.GetLODAutoFactor()); // load up current values
			} // else
		else if(ParamCode == NVW_PC_TOGGLE)
			{
			MasterScene.SceneLOD.SetFramerateMaintain(!MasterScene.SceneLOD.GetFramerateMaintain());
			if(!MasterScene.SceneLOD.GetFramerateMaintain())
				{
				MasterScene.SceneLOD.SetLODAutoFactor(1.0f); // reset to default
				GetGlobalViewer()->setLODScale(MasterScene.SceneLOD.GetLODScalingFactor() * MasterScene.SceneLOD.GetLODAutoFactor()); // load up current values
				} // if
			} // else
		return(true);
		} // NVW_ESC_LOD_MAINTAINRATE
	} // switch Minor

return(Success);
} // EventDispatcher::DispatchEventLOD

bool EventDispatcher::DispatchEventACTION(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_ACTION_QUERYHERE:
		{
		if(ParamCode == NVW_PC_LESS)
			{
			PerformQueryAction(MousePosXf, MousePosYf, true); // non-intrusive results only
			} // if
		else // NVW_PC_FULL
			{
			PerformQueryAction(MousePosXf, MousePosYf, false); // full query/action
			} // else
		return(true);
		} // NVW_ESC_ACTION_QUERYHERE
	} // switch Minor


return(Success);
} // EventDispatcher::DispatchEventACTION


bool EventDispatcher::DispatchEventMEDIA(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_MEDIA_CANCELSOUNDS:
		{
		CancelSounds();
		return(true);
		} // NVW_EC_MEDIA_CANCELSOUNDS
	} // switch Minor


return(Success);
} // EventDispatcher::DispatchEventMEDIA

bool EventDispatcher::DispatchEventDEBUG(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

switch(Minor)
	{
	case NVW_ESC_DEBUG_TEXTURES:
		{
		// toggle OSG texturing
		for(osgProducer::Viewer::EventHandlerList::iterator itr = GetGlobalViewer()->getEventHandlerList().begin();
		 itr != GetGlobalViewer()->getEventHandlerList().end(); ++itr)
			{
			osgGA::StateSetManipulator* stateSetManipulator = dynamic_cast<osgGA::StateSetManipulator*>(itr->get());
			if (stateSetManipulator)
				{
				stateSetManipulator->setTextureEnabled(!stateSetManipulator->getTextureEnabled());
				} // if
			} // for
		return(true);
		} // NVW_ESC_DEBUG_TEXTURES
	case NVW_ESC_DEBUG_WIREFRAME:
		{
		// toggle OSG wireframe
		for(osgProducer::Viewer::EventHandlerList::iterator itr = GetGlobalViewer()->getEventHandlerList().begin();
		 itr != GetGlobalViewer()->getEventHandlerList().end(); ++itr)
			{
			osgGA::StateSetManipulator* stateSetManipulator = dynamic_cast<osgGA::StateSetManipulator*>(itr->get());
			if (stateSetManipulator)
				{
				stateSetManipulator->setPolygonMode(stateSetManipulator->getPolygonMode() == osg::PolygonMode::FILL ? osg::PolygonMode::LINE : osg::PolygonMode::FILL);
				MasterScene.SceneLOD.EnableSky(stateSetManipulator->getPolygonMode() == osg::PolygonMode::FILL ? true : false); // sky off in wireframe
				clearNode->setRequiresClear(stateSetManipulator->getPolygonMode() == osg::PolygonMode::FILL ? false : true); // turn on clearing when in wireframe
				} // if
			} // for
		return(true);
		} // NVW_ESC_DEBUG_WIREFRAME
	} // switch Minor


return(Success);
} // EventDispatcher::DispatchEventDEBUG


bool EventDispatcher::DispatchEventMISC(NVW_EventClass Major, NVW_EventSubClass Minor, NVW_ParamCode ParamCode, char *EventParam)
{
bool Success = false;

return(Success);
} // EventDispatcher::DispatchEventMISC

