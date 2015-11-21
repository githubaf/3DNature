
#include <osg/Switch>
#include <osg/PositionAttitudeTransform>

#include "KeyDefines.h"
#include "NVEventHandler.h"
#include "EventDispatcher.h"
#include "CreateHelpSplashWatermark.h"
#include "NavDlg.h" // to be able to syncwidgets after key actions that change widget state
#include "Viewer.h"
#include "NVScene.h"

extern NVScene MasterScene;


bool NVEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{

EventDispatcher::SetMousPosf(ea.getX(), ea.getY());

switch(ea.getEventType())
	{
	case(osgGA::GUIEventAdapter::SCROLL):
		{
		if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
			{
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_UP);
			return(true);
			} // if
		else if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
			{
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_DOWN);
			return(true);
			} // else if
		break;
		} // SCROLLUP

	case(osgGA::GUIEventAdapter::PUSH):
		{
		if(ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
			{
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_MAINPOPUP);
			// forge RMB-up message to clear EventHandler state
			PostMessage(GetGlobalViewerHWND(), WM_RBUTTONUP, 0, 0);
			return(true);
			} // if
		else if(ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			{
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_STOP);
			return(false); // Manipulator handler needs to receive this event too
			} // if
		else if(ea.getButtonMask() & osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
			if(MasterScene.Overlay.HandlePotentialMapClick(ea.getX(), ea.getY()))
				{
				return(true); // we handled it, Manipulator shouldn't
				} // if
			} // if
		break;
		} // PUSH
    case(osgGA::GUIEventAdapter::KEYDOWN):
        {
		bool ControlQual, ShiftQual, AltQual;
/*
// this demonstrates an event that uses mouse position as input, like point to locate
        if (ea.getKey()=='a')
	        {
            float x = ea.getXnormalized();
            float y = ea.getYnormalized();
            return (true);
	        } // if
*/

        ControlQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_CTRL ? true : false);
		ShiftQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_SHIFT ? true : false);
		AltQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_ALT ? true : false);

		return(HandleKey(ea.getKey(), ControlQual, ShiftQual, AltQual));
        } // KEYDOWN
    case(osgGA::GUIEventAdapter::KEYUP):
        {
		bool ControlQual, ShiftQual, AltQual;
/*
// this demonstrates an event that uses mouse position as input, like point to locate
        if (ea.getKey()=='a')
	        {
            float x = ea.getXnormalized();
            float y = ea.getYnormalized();
            return (true);
	        } // if
*/

        ControlQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_CTRL ? true : false);
		ShiftQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_SHIFT ? true : false);
		AltQual = (ea.getModKeyMask() && osgGA::GUIEventAdapter::MODKEY_ALT ? true : false);

		return(HandleKeyUp(ea.getKey(), ControlQual, ShiftQual, AltQual));
        } // KEYUP
    default: return false;
	} // switch getEventType
return(false);
} // NVEventHandler::handle

bool NVEventHandler::HandleKey(int Key, bool ControlQual, bool ShiftQual, bool AltQual)
{
// Hide Splash, if visible
if(GetSplashDisplayed())
	{
	EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_SPLASH);
	} // if

switch(Key)
	{
	case NV_KEY_HIDEWINTOGGLE:
		{ // hide all windows
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_HIDEALLWIN);
		return true;
		} // NV_KEY_RETURNHOME2
	case NV_KEY_STATSTOGGLE:
	case NV_KEY_STATSTOGGLE2:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_FRAMESTATS);
		return true;
		} // NV_KEY_RETURNHOME2
	case NV_KEY_RETURNHOME2:
		{ // jump to Home position
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_HOME);
		return true;
		} // NV_KEY_RETURNHOME2
	case NV_KEY_DISABLETOGGLE:
		{ // toggle Movement on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_PAUSE);
		return true;
		} // NV_KEY_DISABLETOGGLE
	case NV_KEY_MOVEOPT:
		{ // toggle Movement Optimization on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_OPTIMIZEMOVE);
		return true;
		} // NV_KEY_MOVEOPT
	case NV_KEY_HELPTOGGLE:
	case NV_KEY_HELPTOGGLE2:
		{ // toggle help screen on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_UI, EventDispatcher::NVW_ESC_UI_HELP);
		return true;
		} // NV_KEY_HELPTOGGLE
	case NV_KEY_ALTDRAPETOGGLE:
		{ // toggle alternate drape on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_ALTDRAPE);
		return true;
		} // NV_KEY_ALTDRAPETOGGLE
	case NV_KEY_VECTOGGLE:
		{ // toggle vector objects on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_VECTOR);
		return true;
		} // NV_KEY_VECTOGGLE
	case NV_KEY_OBJECTTOGGLE:
		{ // toggle 3d objects on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OBJECT);
		return true;
		} // NV_KEY_OBJECTTOGGLE
	case NV_KEY_FOLIAGETOGGLE:
		{ // toggle foliage on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_FOLIAGE);
		return true;
		} // NV_KEY_FOLIAGETOGGLE
	case NV_KEY_LABELTOGGLE:
		{ // toggle 3d objects on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_LABEL);
		return true;
		} // NV_KEY_LABELTOGGLE
	case NV_KEY_OCEANTOGGLE:
		{ // toggle ocean on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OCEAN);
		return true;
		} // NV_KEY_OCEANTOGGLE
	case NV_KEY_TERRAINTOGGLE:
		{ // toggle terrain on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_TERRAIN);
		return true;
		} // NV_KEY_TERRAINTOGGLE
	case NV_KEY_OVERLAYTOGGLE:
		{ // toggle overlay on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OVERLAY);
		return true;
		} // NV_KEY_OVERLAYTOGGLE
	case NV_KEY_TERRAINFOLLOW:
		{ // toggle terrain following on and off
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TERRAINFOLLOW);
		NavDlg::SyncWidgets();
		return true;
		} // NV_KEY_TERRAINFOLLOW
	case NV_KEY_ZOOMOUT:
	case NV_KEY_ZOOMOUT2:
	case NV_KEY_ZOOMOUT3:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_ZOOM, EventDispatcher::NVW_PC_OUT);
		return true;
		} // NV_KEY_ZOOMOUT
	case NV_KEY_ZOOMIN:
	case NV_KEY_ZOOMIN2:
	case NV_KEY_ZOOMIN3:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_ZOOM, EventDispatcher::NVW_PC_IN);
		return true;
		} // NV_KEY_ZOOMIN
	case NV_KEY_NEXTCAM:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NEXT);
		return true;
		} // NV_KEY_NEXTCAM
	case NV_KEY_PREVCAM:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_PREV);
		return true;
		} // NV_KEY_PREVCAM
	case NV_KEY_SELCAM:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_NONE);
		return true;
		} // NV_KEY_SELCAM
	case NV_KEY_SELCAT:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_CATMENU, EventDispatcher::NVW_PC_NONE);
		return true;
		} // NV_KEY_SELCAM

	// throttle
	case NV_KEY_THROTTLEUP:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_UP);
		return true;
		} // NV_KEY_THROTTLEUP
	case NV_KEY_THROTTLEDN:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_DOWN);
		return true;
		} // NV_KEY_THROTTLEDN

	// LOD
	case NV_KEY_LODMORE:
	case NV_KEY_LODMOREB:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_LOD, EventDispatcher::NVW_ESC_LOD_DETAIL, EventDispatcher::NVW_PC_MORE);
		return true;
		} // NV_KEY_LODMORE
	case NV_KEY_LODLESS:
	case NV_KEY_LODLESSB:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_LOD, EventDispatcher::NVW_ESC_LOD_DETAIL, EventDispatcher::NVW_PC_LESS);
		return true;
		} // NV_KEY_LODLESS

	case NV_KEY_UNDO:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_UNDO);
		return true;
		} // z
/*
	case NV_KEY_TESTTWO:
		{ // increase minimum feature size
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_LOD, EventDispatcher::NVW_ESC_LOD_MINFEATURESIZE, EventDispatcher::NVW_PC_MORE);
		return true;
		} // NV_KEY_TESTTWO
*/

	case NV_KEY_TESTTWO:
		{ // toggle maintain framerate
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_LOD, EventDispatcher::NVW_ESC_LOD_MAINTAINRATE, EventDispatcher::NVW_PC_TOGGLE);
		return true;
		} // NV_KEY_TESTTWO
	case NV_KEY_TESTNINE:
		{ // toggle wireframe
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_DEBUG, EventDispatcher::NVW_ESC_DEBUG_WIREFRAME, EventDispatcher::NVW_PC_TOGGLE);
		return true;
		} // NV_KEY_TESTNINE
	case NV_KEY_TESTTEN:
		{ // toggle textures
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_DEBUG, EventDispatcher::NVW_ESC_DEBUG_TEXTURES, EventDispatcher::NVW_PC_TOGGLE);
		return true;
		} // NV_KEY_TESTTEN

/*
	case NV_KEY_TESTTHREE:
		{ // toggle stereo
		GetGlobalViewer()->getDisplaySettings()->setStereoMode(osg::DisplaySettings::ANAGLYPHIC);
		GetGlobalViewer()->getDisplaySettings()->setStereo(!GetGlobalViewer()->getDisplaySettings()->getStereo());
		return true;
		} // NV_KEY_TESTTHREE
	case NV_KEY_TESTFOUR:
		{ // adjust stereo eye sep
		GetGlobalViewer()->getDisplaySettings()->setEyeSeparation(GetGlobalViewer()->getDisplaySettings()->getEyeSeparation() + .001);
		return true;
		} // NV_KEY_TESTFOUR
	case NV_KEY_TESTFIVE:
		{ // adjust stereo eye sep
		GetGlobalViewer()->getDisplaySettings()->setEyeSeparation(GetGlobalViewer()->getDisplaySettings()->getEyeSeparation() - .001);
		return true;
		} // NV_KEY_TESTFIVE
*/

	case NV_KEY_VIEWTOUR:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_VIEWPOINT, EventDispatcher::NVW_PC_TOGGLE);
		return true;
		} // NV_KEY_VIEWTOUR


	// NavMode switching
	case NV_KEY_MOVEMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_DRIVE); NavDlg::SyncWidgets(); return true;
	case NV_KEY_ROTMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_ROTATE); NavDlg::SyncWidgets(); return true;
	case NV_KEY_SLIDEMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_SLIDE); NavDlg::SyncWidgets(); return true;
#ifdef NVW_SUPPORT_QUERYACTION
	case NV_KEY_QUERYMODE: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_QUERY); NavDlg::SyncWidgets(); return true;
#endif // NVW_SUPPORT_QUERYACTION

	// QUAKE-style navigation keys
	// move
	case NV_KEY_MOVEFWD:
	case NV_KEY_MOVEFWD2: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_FORWARD); return true;
	case NV_KEY_MOVEBACK:
	case NV_KEY_MOVEBACK2: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_BACKWARD); return true;
	case NV_KEY_MOVELEFT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_LEFT); return true;
	case NV_KEY_MOVERIGHT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_RIGHT); return true;
	case NV_KEY_MOVEUP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_UP); return true;
	case NV_KEY_MOVEDOWN: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_MOVEIMMED, EventDispatcher::NVW_PC_DOWN); return true;

	// turn
	case NV_KEY_TURNLEFT:
	case NV_KEY_TURNLEFT2: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_LEFT); return true;
	case NV_KEY_TURNRIGHT2:
	case NV_KEY_TURNRIGHT: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_RIGHT); return true;
	case NV_KEY_TURNUP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_UP); return true;
	case NV_KEY_TURNDOWN: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_DOWN); return true;
	case NV_KEY_TURNCLOCK: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_CLOCKWISE); return true;
	case NV_KEY_TURNCCLOCK: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_TURNIMMED, EventDispatcher::NVW_PC_COUNTERCLOCKWISE); return true;
	
	// Misc
	case NV_KEY_SOUNDSTOP: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_MEDIA, EventDispatcher::NVW_ESC_MEDIA_CANCELSOUNDS, EventDispatcher::NVW_PC_NONE); return true;
	// NV_KEY_QUERY fires on keyup
	//case NV_KEY_QUERY: 

	} // switch
return false;

} // NVEventHandler::HandleKey



bool NVEventHandler::HandleKeyUp(int Key, bool ControlQual, bool ShiftQual, bool AltQual)
{
// Hide Splash, if visible
if(GetSplashDisplayed())
	{
	EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_SPLASH);
	} // if

switch(Key)
	{
	// Misc
	case NV_KEY_QUERY: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_ACTION, EventDispatcher::NVW_ESC_ACTION_QUERYHERE, EventDispatcher::NVW_PC_NONE); return true;
	} // switch
return false;

} // NVEventHandler::HandleKeyUp
