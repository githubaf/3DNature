#include <osg/Notify>
#include <osgUtil/IntersectVisitor>
#include <osgGA/GUIEventAdapter>
#include <osgProducer/Viewer>
#include <osg/PositionAttitudeTransform>

#include "Types.h"
#include "NVScene.h"
#include "PanoManipulator.h"
#include "NVMiscGlobals.h"
#include "NVTerrain.h"
#include "CameraSupport.h"
#include "Viewer.h"
#include "Navigation.h"
#include "PartialVertexDEM.h"
#include "UsefulMath.h"
#include "EventDispatcher.h"
#include "NVNodeMasks.h"
#include "NVQueryAction.h"
#include "ToolTips.h"

// these still live in main.cpp swith class Scene, for the moment
bool GetSceneGraphBuilt(void);
osg::Group *GetSceneRootNodeGlobal(void);

extern ToolTipSupport *GlobalTipSupport;


extern NVScene MasterScene;


// for debugging output on HUD
double DXG, DYG, MiscReadout, MiscReadoutTwo;


using namespace osg;
using namespace osgGA;


PanoManipulator::PanoManipulator()
{
_modelScale = 0.01f;
_velocity = 0.0f;

LastClickOriginX = LastClickOriginY = FLT_MAX;
TransitionDesiredLengthTime = 2.0;
StartMoment = 0.0;
LastMouseMoveMoment = 0.0;
SetMouseInactiveOneShot(false);
} // PanoManipulator::PanoManipulator


PanoManipulator::~PanoManipulator()
{
} // PanoManipulator::~PanoManipulator


void PanoManipulator::setNode(osg::Node* node)
{
_node = node;
/*
    if (_node.get())
    {
        const osg::BoundingSphere& boundingSphere=_node->getBound();
        _modelScale = boundingSphere._radius;
    }
*/
if(GetSceneRootNodeGlobal())
    {
    const osg::BoundingSphere& boundingSphere=GetSceneRootNodeGlobal()->getBound(); // don't do entire scenegraph, just 'displayed' scene
    _modelScale = boundingSphere._radius;
    } // if
} // PanoManipulator::setNode


const osg::Node* PanoManipulator::getNode() const
{
return _node.get();
} // PanoManipulator::getNode



osg::Node* PanoManipulator::getNode()
{
return _node.get();
} // PanoManipulator::getNode


// handles Viewpoint Tours
void PanoManipulator::InternalTour(void)
{
if(GetTransitionInProgress())
	{ // nothing to do, things are flowing nicely
	} // if
else
	{
	double CurrentTime, CurrentPauseTime;
	CurrentTime = GetCurrentMoment();
	CurrentPauseTime = CurrentTime - TourPauseMoment;

	if(GetAnimationInProgress())
		{ // stay here until end of animated viewpoint path
		return;
		} // if
	else
		{
		if(CurrentPauseTime > TourPauseTime) // pause is over, start moving again
			{
			// move on to next viewpoint
			SetTransStartFromCurrent();
			if(MasterScene.GetCurrentCameraNum() == MasterScene.GetNumCameras() - 1) // are we at last viewpoint?
				{ // yes, move to first
				MasterScene.SetCurrentCameraNum(0);
				MarkTransStartMoment();
				SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // if
			else // no, just move on to next camera
				{
				MasterScene.SelectNextCamera();
				MarkTransStartMoment();
				SetTransEndFromCurrent(); 
				SetTransitionInProgress();
				SetDelayedHome();
				} // else
			return;
			} // if
		else
			{
			// nothing to do at the moment, twiddle thumbs while the gapers gape
			} // else
		} // else
	} // else
} // PanoManipulator::InternalTour


void PanoManipulator::StartTour(double NewTourPauseTime)
{
SetTourInProgress(true);
TourPauseTime = NewTourPauseTime;
// make sure next viewpoint transition starts right away
TourPauseMoment = GetCurrentMoment() - TourPauseTime;

} // PanoManipulator::StartTour


void PanoManipulator::EndTour(void)
{
SetTourInProgress(false);
} // PanoManipulator::EndTour

void PanoManipulator::TourPauseHere(void)
{ // reset pause counter
TourPauseMoment = GetCurrentMoment();
} // PanoManipulator::TourPauseHere


// sets up for viewpoint animated transitions

void PanoManipulator::SetTransStartFromCurrent(void)
{
// get current HFOV
osgProducer::Viewer *_viewer;
double lensleft, lensright, lenstop, lensbottom, zNear, zFar;

_viewer = GetGlobalViewer();
_viewer->getLensParams(lensleft, lensright, lenstop, lensbottom, zNear, zFar);
TransHFOV_S = _viewer->getLensHorizontalFov();

TransCamPos_S = _eye;
TransCamOrient_S = _rotation;
} // PanoManipulator::SetTransStartFromCurrent

void PanoManipulator::SetTransEndFromCurrent(void)
{
float CX, CY, CZ, TX, TY, TZ, HFOV, Bank;

if(GetCameraStartPos(GetCurCameraNum(), CX, CY, CZ, TX, TY, TZ, HFOV, Bank))
	{
	SetTransEnd(CX, CY, CZ, TX, TY, TZ, HFOV, Bank);
	} // if

} // PanoManipulator::SetTransEndFromCurrent

void PanoManipulator::SetTransEnd(double CX, double CY, double CZ, double TX, double TY, double TZ, double HFOV, double Bank)
{
TransCamPos_E[0] = CX;
TransCamPos_E[1] = CY;
TransCamPos_E[2] = CZ;
TransTargPos_E[0] = TX;
TransTargPos_E[1] = TY;
TransTargPos_E[2] = TZ;

TransHFOV_E = HFOV;
TransBank_E = Bank;
} // PanoManipulator::SetTransEnd

void PanoManipulator::MarkTransStartMoment(void)
{
TransitionStartTime = GetCurrentMoment();
} // PanoManipulator::MarkTransStartMoment



// Handles each slice of the viewpoint transition

void PanoManipulator::InternalTrans(void)
{

double CurrentTime, CurrentTransTime;
CurrentTime = GetCurrentMoment();
CurrentTransTime = CurrentTime - TransitionStartTime;

if(CurrentTransTime > TransitionDesiredLengthTime)
	{
	InternalTrans(TransitionDesiredLengthTime); // hit final moment
	SetTransitionInProgress(false);
	ClearStartMoment(); // start animated camera at time=0
	if(GetTourInProgress())
		{ // begin counting pause time
		TourPauseHere();
		} // if
	return;
	} // if
else
	{
	InternalTrans(CurrentTransTime);
	} // else

} // PanoManipulator::InternalTrans



void PanoManipulator::InternalTrans(double Moment)
{
double TransFrac;
float HFOV_S, HFOV_N, HFOV_E;
float CX_E, CY_E, CZ_E, TX_E, TY_E, TZ_E, Bank_E;

if(TransitionDesiredLengthTime > 0)
	{
	// convert to proportion in 0...1 scale
	TransFrac = Moment / TransitionDesiredLengthTime;
	MiscReadout = TransFrac;
	// <<<>>> remap with ease in/out?
	} // if
else
	{
	// error, bail
	return;
	} // else

// make local copies -- kind of redundent, but hey...
HFOV_S = TransHFOV_S;

CX_E = TransCamPos_E[0];  CY_E = TransCamPos_E[1];  CZ_E = TransCamPos_E[2];
TX_E = TransTargPos_E[0]; TY_E = TransTargPos_E[1]; TZ_E = TransTargPos_E[2];
HFOV_E = TransHFOV_E; Bank_E = TransBank_E;


// interpolate present value for HFOV
HFOV_N = flerp(TransFrac, HFOV_S, HFOV_E);

// Determine position Vec3 and orientation Quat for destination
osg::Vec3 lv, upvec, rotupvec, DestEye;
osg::Matrix mat;
osg::Quat DestOrient;
DestEye = osg::Vec3(CX_E,CY_E,CZ_E);
lv = osg::Vec3(TX_E,TY_E,TZ_E) - DestEye;
// need to rotate upvec by "Bank" degrees (clockwise? counterclockwise?) around lv
// suggestions by D Burns, 2/25/04
upvec = osg::Vec3(0.0f,0.0f,1.0f);
mat.makeRotate( osg::DegreesToRadians(-Bank_E), lv );
rotupvec = upvec * mat;
ComputeOrientation(lv, rotupvec, DestOrient);

// create interpolated position and orientation
osg::Vec3 NowEye;
osg::Quat NowOrient;

NowEye = TransCamPos_S + ((DestEye - TransCamPos_S) * TransFrac);
// Quats can't be linearly interpolated, but we have a Spherical interpolation method just for this!
NowOrient.slerp(TransFrac, TransCamOrient_S, DestOrient);

// load new interpolated position and orientation
_eye = NowEye;
_rotation = NowOrient;


// set up FOV
osgProducer::Viewer *_viewer;
_viewer = GetGlobalViewer();
double lensleft, lensright, lenstop, lensbottom, zNear, zFar, lensaspect;
float fovx, fovy, newfovx, newfovy;

_viewer->getLensParams(lensleft, lensright, lenstop, lensbottom, zNear, zFar);
fovx = _viewer->getLensHorizontalFov();
fovy = _viewer->getLensVerticalFov();
lensaspect = _viewer->getCamera(0)->getLensAspectRatio();
if(fovy != 0.0) // safety: avoid divide by zero
	{
	lensaspect = fovx / fovy;
	} // if

if(lensaspect != 0.0 && HFOV_N > 1.0 && HFOV_N < 180.0)
	{
	newfovx = HFOV_N;
	newfovy = HFOV_N / lensaspect;
	_viewer->setLensPerspective(newfovx, newfovy, zNear, zFar);
	} // if

EnforceConform();
} // PanoManipulator::InternalTrans




// Handles each slice of the viewpoint transition

void PanoManipulator::InternalAnim(void)
{
double CurrentTime, CurrentAnimLen, CurrentAnimTime;
CurrentTime = GetCurrentMoment();

CurrentAnimLen = GetCameraAnimLength(GetCurCameraNum());
if(GetTourInProgress())
	{ // if at endpoint, finish this path so we can move on to the next viewpoint
	if(CurrentTime > CurrentAnimLen)
		{
		SetAnimationInProgress(false);
		InternalTour(); // jump-start tour resumption
		return;
		} // if
	} // if
// loop this path
CurrentAnimTime = fmod(CurrentTime, CurrentAnimLen);

InternalAnim(CurrentAnimTime);

} // PanoManipulator::InternalAnim



void PanoManipulator::InternalAnim(double Moment)
{
NVAnimObject *CurCam;
// for interpolated position and orientation
osg::Vec3 NowEye;
osg::Quat NowOrient;
float HFOV_N = 1.0;

if(CurCam = MasterScene.GetCamera(GetCurCameraNum()))
	{
	if(InterpretCameraForUse(CurCam, Moment, NowEye, NowOrient, HFOV_N))
		{
		// load new interpolated position and orientation
		_eye = NowEye;
		_rotation = NowOrient;

		// set up FOV
		osgProducer::Viewer *_viewer;
		_viewer = GetGlobalViewer();
		double lensleft, lensright, lenstop, lensbottom, zNear, zFar, lensaspect;
		float fovx, fovy, newfovx, newfovy;

		_viewer->getLensParams(lensleft, lensright, lenstop, lensbottom, zNear, zFar);
		fovx = _viewer->getLensHorizontalFov();
		fovy = _viewer->getLensVerticalFov();
		lensaspect = _viewer->getCamera(0)->getLensAspectRatio();
		if(fovy != 0.0) // safety: avoid divide by zero
			{
			lensaspect = fovx / fovy;
			} // if

		if(lensaspect != 0.0 && HFOV_N > 1.0 && HFOV_N < 180.0)
			{
			newfovx = HFOV_N;
			newfovy = HFOV_N / lensaspect;
			_viewer->setLensPerspective(newfovx, newfovy, zNear, zFar);
			} // if

		EnforceConform();
		} // if
	} // if

} // PanoManipulator::InternalAnim



// does the real work of home
void PanoManipulator::InternalHome(void)
{

if(!GetSceneGraphBuilt())
	{
	// don't set up a transition here, since it's our starting point
	SetDelayedHome(true); // keep homing until the scene is initialized
	return;
	} // if

if(_node.get())
    {
	float CX, CY, CZ, TX, TY, TZ, HFOV, Bank;

	if(GetCameraStartPos(GetCurCameraNum(), CX, CY, CZ, TX, TY, TZ, HFOV, Bank))
		{
		osgProducer::Viewer *_viewer;
		osg::Vec3 eye = osg::Vec3(CX,CY,CZ);

		osg::Vec3 lv, upvec;
		lv = osg::Vec3(TX,TY,TZ) - eye;

		// need to rotate upvec by "Bank" degrees (clockwise? counterclockwise?) around lv
		// suggestions by D Burns, 2/25/04
		/*
You can do what you are trying to do with quaternions (osg::Quat) or
matricies.  In either case, you will end up using a matrix.  You could
also do the matrix math inline and probably save yourself a couple of
redundant operations, but coding it is much more difficult than just using
a matrix.

Method one:

    // Given
    osg::Vec3 Normal(x,y,z);
    osg::Vec3 Axis(a,b,c);
    float radians = someRotation;
    osg::Matrix mat = osg::Matrix::makeRotate( radians, Axis );
    osg::Vec3 _Normal = Normal * mat;

Method two:
    float radians = someRotation;
    osg::Quat q( Axis[0] * sin(radians/2.0),
                 Axis[1] * sin(radians/2.0),
                 Axis[2] * sin(radians/2.0),
                 cos(radians/2.0));

    // Now it would be nice if we could just

    osg::Vec3 _Normal = normal * q;

    // ... perhaps after the Quat overhaul. For now

    osg::Matrix mat;
    q.get(mat);
    osg::Vec3 _Normal = Normal * mat;
*/

		upvec = osg::Vec3(0.0f,0.0f,1.0f);

		osg::Matrix mat;
		mat.makeRotate( osg::DegreesToRadians(-Bank), lv );
		osg::Vec3 rotupvec = upvec * mat;

		computePosition(eye, lv, rotupvec);

		// set up FOV
		_viewer = GetGlobalViewer();
		double lensleft, lensright, lenstop, lensbottom, zNear, zFar, lensaspect;
		float fovx, fovy, newfovx, newfovy;

		_viewer->getLensParams(lensleft, lensright, lenstop, lensbottom, zNear, zFar);
		fovx = _viewer->getLensHorizontalFov();
		fovy = _viewer->getLensVerticalFov();
		lensaspect = _viewer->getCamera(0)->getLensAspectRatio();
		if(fovy != 0.0) // safety: avoid divide by zero
			{
			lensaspect = fovx / fovy;
			} // if

		if(lensaspect != 0.0 && HFOV > 1.0 && HFOV < 180.0)
			{
			newfovx = HFOV;
			newfovy = HFOV / lensaspect;
			_viewer->setLensPerspective(newfovx, newfovy, zNear, zFar);
			} // if

		} // if
	else
		{
		const osg::BoundingSphere& boundingSphere=_node->getBound();
		osg::Vec3 eye = boundingSphere._center+osg::Vec3(-boundingSphere._radius*0.25f,-boundingSphere._radius*0.25f,+boundingSphere._radius*0.03f);

		computePosition(eye,
			osg::Vec3(1.0f,1.0f,-0.1f),
			osg::Vec3(0.0f,0.0f,1.0f));
		} // else

	_velocity = 0.0f;
    } // if

} // PanoManipulator::InternalHome


void PanoManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
if(_node.get())
    {
	InternalHome();
    us.requestRedraw();
	// warp pointer no longer needed
    //us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
    flushMouseEventStack();
    } // if

} // PanoManipulator::home

void PanoManipulator::halt(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
if(_node.get())
    {
	_velocity = 0.0f;
    us.requestRedraw();
	// warp pointer no longer needed
    //us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
    flushMouseEventStack();
    } // if

} // PanoManipulator::halt


void PanoManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
flushMouseEventStack();

us.requestContinuousUpdate(false);

_velocity = 0.0f;

if (ea.getEventType()!=GUIEventAdapter::RESIZE)
	{
	// warp pointer no longer needed
    //us.requestWarpPointer((ea.getXmin()+ea.getXmax())/2.0f,(ea.getYmin()+ea.getYmax())/2.0f);
	} // if
} // PanoManipulator::init

static bool PossibleQueryInitiating = false;

bool PanoManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& us)
{

if(StartMoment == 0.0) // only do this once at actual startup of drawing
	{
	ClearStartMoment();
	} // if

unsigned int buttonMask = ea.getButtonMask();

switch(ea.getEventType())
    {
    case(GUIEventAdapter::PUSH):
        {
		GlobalTipSupport->HideBalloonTip();
		if(GetGlobalViewer())
			{
			SetCapture(GetGlobalViewerHWND());
			} // if
		if((buttonMask & GUIEventAdapter::LEFT_MOUSE_BUTTON) && (Navigation::GetCurrentNavMode() == Navigation::NAV_NM_QUERY))
			{
			PossibleQueryInitiating = true;
			} // if
		else
			{
			if(buttonMask & GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
				{ // determine and record orbit point
				osg::Vec3 NewOrbitCenter;
				if(GetFirstTangibleHitLocation(ea.getX(), ea.getY(), NewOrbitCenter))
					{
					SetOrbitCenter(NewOrbitCenter);
					} // if
				else
					{
					ClearOrbitCenter();
					} // else
				} // if
			PossibleQueryInitiating = false;
			addMouseEvent(ea);
			us.requestContinuousUpdate(true);
			if(GetDelayedGoto())
				{
				Goto(ea.getX(), ea.getY());
				} // if
			else
				{
				if (calcMovement(ea, us)) us.requestRedraw();
				} // else
			} // if
        return true;
        } // PUSH
    case(GUIEventAdapter::RELEASE):
        {
        if(buttonMask & GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
			{
			ClearOrbitCenter();
			} // if
		if(GetGlobalViewer())
			{
			if(GetCapture() == GetGlobalViewerHWND())
				{
				ReleaseCapture();
				} // if
			} // if
        addMouseEvent(ea);
        us.requestContinuousUpdate(true);
        if(Navigation::GetCurrentNavMode() == Navigation::NAV_NM_QUERY)
			{
#ifdef NVW_SUPPORT_QUERYACTION
			// Query/Action fires on mouseUP, to no weird state is interfering with newly-opened windows
			if(PossibleQueryInitiating) PerformQueryAction(ea.getX(), ea.getY(), false);
#endif // NVW_SUPPORT_QUERYACTION
			} // if
		else
			{
			PossibleQueryInitiating = false;
	        if (calcMovement(ea, us)) us.requestRedraw();
	        } // else
        return true;
        } // RELEASE
    case(GUIEventAdapter::DRAG):
        {
        addMouseEvent(ea);
        us.requestContinuousUpdate(true);
        if (calcMovement(ea, us)) us.requestRedraw();
        return true;
        } // DRAG
    case(GUIEventAdapter::MOVE):
        {
        MarkLastMouseMoveMoment();
        addMouseEvent(ea);
        us.requestContinuousUpdate(true);
        if (calcMovement(ea, us)) us.requestRedraw();

        return true;
        } // MOVE
    case(GUIEventAdapter::KEYDOWN):
		{
        if (ea.getKey()==' ')
            {
			GlobalTipSupport->HideBalloonTip();
            EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_ZERO);
			flushMouseEventStack();
            home(ea,us);
            us.requestRedraw();
            us.requestContinuousUpdate(false);
            return true;
            } // if
        if (ea.getKey()=='.')
            {
			GlobalTipSupport->HideBalloonTip();
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_ZERO);
            flushMouseEventStack();
            halt(ea,us);
            us.requestRedraw();
            us.requestContinuousUpdate(false);
            return true;
            } // if
        return false;
		} // KEYDOWN
    case(GUIEventAdapter::FRAME):
		{
		if(!GetMouseInactiveOneShot() && GetMouseInactiveTime() > NVW_PANOMANIPULATOR_MOUSE_INACTIVE_TIME_SECONDS && (GetSystemTimeFP() - GetLastMovementMoment()) > NVW_PANOMANIPULATOR_MOUSE_INACTIVE_TIME_SECONDS)
			{
			SetMouseInactiveOneShot(true); // make it only fire once
			EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_ACTION, EventDispatcher::NVW_ESC_ACTION_QUERYHERE, EventDispatcher::NVW_PC_LESS);
			} // if
		GlobalTipSupport->HandleBalloonTipTimeout();
        addMouseEvent(ea);
		if(GetTourInProgress())
			{
			GlobalTipSupport->HideBalloonTip();
			InternalTour();
			// we don't redraw, as the animation part of the tour is handled by Transition code, below
			} // if
		if(GetTransitionInProgress())
			{
			GlobalTipSupport->HideBalloonTip();
			InternalTrans();
			us.requestRedraw();
			} // if
		else if(GetAnimationInProgress())
			{
			GlobalTipSupport->HideBalloonTip();
			InternalAnim();
			us.requestRedraw();
			} // if
		else
			{
			// handle HotNav disk
			if(MasterScene.GetHotNavFwdBackAmount() != 0.0 || MasterScene.GetHotNavSideAmount() != 0.0 )
				{
				GlobalTipSupport->HideBalloonTip();
				ExecuteMovement(MasterScene.GetHotNavSideAmount(), MasterScene.GetHotNavFwdBackAmount(), 0.0);
				us.requestRedraw();
				} // if
			// handle HotTurn disk
			if(MasterScene.GetHotTurnHAmount() != 0.0 || MasterScene.GetHotTurnVAmount() != 0.0 )
				{
				// hot turn is too hot without the constant scaling factor
				GlobalTipSupport->HideBalloonTip();
				ExecuteRotate(MasterScene.GetHotTurnHAmount() * .01, 0.0, MasterScene.GetHotTurnVAmount() * .01); 
				us.requestRedraw();
				} // if
	        if (calcMovement(ea, us)) us.requestRedraw();
			} // else
        return true;
		} // FRAME
    case(GUIEventAdapter::RESIZE):
		{
		GlobalTipSupport->HideBalloonTip();
        init(ea,us);
        us.requestRedraw();
        return true;
		} // RESIZE
    default:
        return false;
    } // switch
} // PanoManipulator::handle



bool PanoManipulator::DoAction(PM_ActionClass ActionType, float Magnitude)
{

GlobalTipSupport->HideBalloonTip();

switch(ActionType)
	{
	case PM_AC_STARTTOUR:
		{
		StartTour(NVW_PANOMANIPULATOR_TOUR_DELAY_TIME_SECONDS);
		return(true);
		break;
		} // 
	case PM_AC_ENDTOUR:
		{
		EndTour();
		return(true);
		break;
		} // 
	case PM_AC_UNDO:
		{
		RestoreFromUndo();
		return(true);
		break;
		} // 
	case PM_AC_HOME:
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_ZERO);
		InternalHome();
		return(true);
		break;
		} // 
	case PM_AC_HALT:
		{
		_velocity = 0.0f;
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_ZERO);
		return(true);
		break;
		} // 
	case PM_AC_MOVEFORWARD:
		{
		ExecuteMovement(0.0, 0.25, 0.0);
		return(true);
		break;
		} // 
	case PM_AC_MOVEBACKWARD:
		{
		ExecuteMovement(0.0, -0.25, 0.0);
		return(true);
		break;
		} // 
	case PM_AC_MOVELEFT:
		{
		ExecuteMovement(-0.25, 0.0, 0.0);
		return(true);
		break;
		} // 
	case PM_AC_MOVERIGHT:
		{
		ExecuteMovement(0.25, 0.0, 0.0);
		return(true);
		break;
		} // 
	case PM_AC_MOVEUP:
		{
		ExecuteMovement(0.0, 0.0, 0.25);
		return(true);
		break;
		} // 
	case PM_AC_MOVEDOWN:
		{
		ExecuteMovement(0.0, 0.0, -0.25);
		return(true);
		break;
		} // 
	case PM_AC_TURNLEFT:
		{
		ExecuteRotate(-0.005, 0.0, 0.0); // 1% is about 3.6 degrees
		return(true);
		break;
		} // 
	case PM_AC_TURNRIGHT:
		{
		ExecuteRotate(0.005, 0.0, 0.0); // 1% is about 3.6 degrees
		return(true);
		break;
		} // 
	case PM_AC_TILTUP:
		{
		ExecuteRotate(0.0, 0.0, 0.001);
		return(true);
		break;
		} // 
	case PM_AC_TILTDOWN:
		{
		ExecuteRotate(0.0, 0.0, -0.001);
		return(true);
		break;
		} // 
	case PM_AC_BANKCLOCKWISE:
		{
		ExecuteRotate(0.0, 0.015, 0.0); // 1.5% is about 5 degrees
		return(true);
		break;
		} // 
	case PM_AC_BANKCOUNTERCLOCKWISE:
		{
		ExecuteRotate(0.0, -0.015, 0.0); // 1.5% is about 5 degrees
		return(true);
		break;
		} // 
	}; // ActionType

return(false);
} // PanoManipulator::DoAction


void PanoManipulator::getUsage(osg::ApplicationUsage& usage) const
{
//usage.addKeyboardMouseBinding("Pano: Space","Reset the viewing position to home");
//usage.addKeyboardMouseBinding("Pano: Period","Stop all movement and rotation");
} // PanoManipulator::getUsage

void PanoManipulator::flushMouseEventStack()
{
_ga_t1 = NULL;
_ga_t0 = NULL;
} // PanoManipulator::flushMouseEventStack


void PanoManipulator::addMouseEvent(const GUIEventAdapter& ea)
{
_ga_t1 = _ga_t0;
_ga_t0 = &ea;
} // PanoManipulator::addMouseEvent


void PanoManipulator::setByMatrix(const osg::Matrix& matrix)
{
_eye = matrix.getTrans();
_rotation.set(matrix);
} // PanoManipulator::setByMatrix

osg::Matrix PanoManipulator::getMatrix() const
{
return osg::Matrix::rotate(_rotation)*osg::Matrix::translate(_eye);
} // PanoManipulator::getMatrix

osg::Matrix PanoManipulator::getInverseMatrix() const
{
return osg::Matrix::translate(-_eye)*osg::Matrix::rotate(_rotation.inverse());
} // PanoManipulator::getInverseMatrix

//void DEBUGDUMP();

void PanoManipulator::computePosition(const osg::Vec3& eye,const osg::Vec3& lv,const osg::Vec3& up)
{
ComputeOrientation(lv, up, _rotation); // do the loading by passing internal variables
//DEBUGDUMP();
_eye = eye;
} // PanoManipulator::computePosition






bool PanoManipulator::calcMovement(const GUIEventAdapter& ea,GUIActionAdapter& us)
{
osg::Matrix rotation_matrix;
osg::Vec3 lv, up, sv;
float pitch, yaw, bank;

pitch = bank = yaw = 0.0;

rotation_matrix.makeRotate(_rotation);

// determine local state necessary to process any movement operations
lv = osg::Vec3(0.0f,0.0f,-1.0f) * rotation_matrix;
up = osg::Vec3(0.0f,1.0f,0.0) * rotation_matrix;
sv = lv^up;
sv.normalize();

// return if less then two events have been added.
if (_ga_t0.get()==NULL || _ga_t1.get()==NULL) return false;

if(GetMovementDisabledGlobal() || GetMovementLockoutGlobal()) return(false);

// handle camera changed through delayed home
if(GetDelayedHome())
	{
	UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
	halt(ea, us); // stop all motion
	InternalHome();
	EnforceConform();
	UpdateUndo();
	return(true);
	} // if

double dt = _ga_t0->time()-_ga_t1->time();
if (dt<0.0f) dt = 0.0f;

unsigned int buttonMask = _ga_t1->getButtonMask();
unsigned int modMask = _ga_t1->getModKeyMask();


if(GetAsyncKeyState(VK_MENU) & 0x8000) modMask |= GUIEventAdapter::MODKEY_ALT;


float DeadZone = .035, InvDeadZone;
float fdx, fdy;
float dx = _ga_t0->getXnormalized();
float dy = _ga_t0->getYnormalized();

if (buttonMask & (GUIEventAdapter::LEFT_MOUSE_BUTTON|GUIEventAdapter::MIDDLE_MOUSE_BUTTON)) // either left or middle button
	{
	if(LastClickOriginX == FLT_MAX) // we aren't in a click-drag operation yet
		{ // note the coordinates for click-drag operation
		LastClickOriginX = dx;
		LastClickOriginY = dy;
		dx = dy = 0.0; // we are now centered
		if(_velocity == 0.0)
			{
			UpdateUndo(); // if we're at rest, update the undo state
			} // if
		} // if
	else
		{
		// make coordinate relative to beginning of click/drag operation
		dx -= LastClickOriginX;
		dy -= LastClickOriginY;

		fdx = fabs(dx);
		fdy = fabs(dy);


		InvDeadZone = 1.0 - DeadZone;
		if(fdx < DeadZone)
			{
			dx = 0.0f; // clamp below DeadZone
			} // if
		else
			{
			if(dx < 0)
				{
				dx += DeadZone;
				} // if
			else
				{
				dx -= DeadZone;
				} // else
			dx /= InvDeadZone; // re-range 0 to InvDeadZone range to 0 to 1.0
			} // else

		if(fdy < DeadZone)
			{
			dy = 0.0f; // clamp below DeadZone
			} // if
		else
			{
			if(dy < 0)
				{
				dy += DeadZone;
				} // if
			else
				{
				dy -= DeadZone;
				} // else
			dy /= InvDeadZone; // re-range 0 to InvDeadZone range to 0 to 1.0
			} // else

		} // else

	// for debugging output on HUD
	DXG = dx;
	DYG = dy;

	} // if
else
	{
	LastClickOriginX = LastClickOriginY = FLT_MAX;
	} // if left button not down




double LocalMoveSpeed;
LocalMoveSpeed = GetMovementSpeedGlobal() * 10;
if(LocalMoveSpeed < 0) // makes no sense, fix it
	{
	LocalMoveSpeed = _modelScale*0.05f;
	} // if

if(MasterScene.GetThrottleSpeed() > 0.0)
	{
	_velocity = MasterScene.GetThrottleSpeed();
	} // if


if((Navigation::GetCurrentNavMode() == Navigation::NAV_NM_DRIVE) && !(modMask & GUIEventAdapter::MODKEY_CTRL) && !(modMask & GUIEventAdapter::MODKEY_ALT) && !(modMask & GUIEventAdapter::MODKEY_SHIFT))
	{
	double LocalInertialBraking;

	if (buttonMask & GUIEventAdapter::LEFT_MOUSE_BUTTON)
		{
		EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_NAVIGATION, EventDispatcher::NVW_ESC_NAVIGATION_THROTTLE, EventDispatcher::NVW_PC_ZERO);
		// process forward/back movement
		_velocity = dy * LocalMoveSpeed;
		// process horizontal mouse-axis (yaw) rotation
		yaw   = -inDegrees(dx*50.0f*dt);
		if ((dx != 0.0) || (dy != 0.0))
			{
			UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
			} // if
		} // if left button down
	else // no buttons, slowly decelerate
		{
		if(!(MasterScene.GetThrottleSpeed() > 0.0))
			{
			if (GetMovementInertiaGlobal() <= 0.0) // instant stop, no inertia
				{
				if(_velocity != 0.0)
					{
					UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
					_velocity = 0.0f; // screeeeeech
					} // if
				} // if
			else // normal range
				{
				if(GetMovementSpeedGlobal() != 0.0)
					{
					LocalInertialBraking = GetMovementSpeedGlobal() * 2;
					} // if
				else
					{
					LocalInertialBraking = _modelScale * 0.02f;
					} // else
				if(_velocity != 0.0)
					{
					float InertiaMult = 1.0 / GetMovementInertiaGlobal();
					UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
					if(_velocity > 0.0)
						{
						_velocity -= InertiaMult * dt * LocalInertialBraking;
						if(_velocity < 0.0f) _velocity = 0.0f; // no reverse
						} // if
					else
						{
						_velocity += InertiaMult * dt * LocalInertialBraking;
						if(_velocity > 0.0f) _velocity = 0.0f; // no switch to forward
						} // else
					} // if
				} // else
			} // else
		} // else

	// finish processing movement
	lv *= (_velocity*dt);
	_eye += lv;
	} // if
else if(((Navigation::GetCurrentNavMode() == Navigation::NAV_NM_SLIDE) || (modMask & GUIEventAdapter::MODKEY_ALT))  && !(modMask & GUIEventAdapter::MODKEY_CTRL)  && !(modMask & GUIEventAdapter::MODKEY_SHIFT))
	{
	if ((dx != 0.0) || (dy != 0.0))
		{
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if

	if (buttonMask & GUIEventAdapter::LEFT_MOUSE_BUTTON)
		{
		// complex system to map the most logical nearly-horizontal axes to the appropriate forward/back and left/right directions
		osg::Vec3 ForwardBack, LeftRight;

		// Vec3 inits components to 0.0, we rely on this below
		osg::Vec3 LVH_UVH_FB, LVH_UVH_LR, LVH_UVV_FB, LVH_UVV_LR, LVV_FB, LVV_LR;
		double LVH_UVH_Weight, LVH_UVV_Weight, LVV_Weight;

		// LV is more vertical
		LVV_FB.set(up.x(), up.y(), 0);
		LVV_LR.set(sv.x(), sv.y(), 0);
		LVV_FB.normalize(); // safe: nop if length=0
		LVV_LR.normalize(); // safe: nop if length=0

		// LV is more horizontal
		// UV is more vertical
		LVH_UVV_FB.set(lv.x(), lv.y(), 0);
		LVH_UVV_LR.set(sv.x(), sv.y(), 0);
		LVH_UVV_FB.normalize(); // safe: nop if length=0
		LVH_UVV_LR.normalize(); // safe: nop if length=0

		// LV is more horizontal
		// UV is more horizontal
		LVH_UVH_FB.set(lv.x(), lv.y(), 0);
		LVH_UVH_LR.set(up.x(), up.y(), 0);
		LVH_UVH_FB.normalize(); // safe: nop if length=0
		LVH_UVH_LR.normalize(); // safe: nop if length=0

		// calculate slide motion blending factors based on axis dominance
		// (these should approximately sum to 1.0)
		double LVH_Rescale_Weight, Overall_Rescale_Weight;
		LVH_UVV_Weight = (fabs(1.0f - lv.z()) * .5) + (fabs(up.z()) * .5); // average to prevent non-linearity
		LVH_UVH_Weight = (fabs(1.0f - lv.z()) * .5) + (fabs(1.0f - up.z()) * .5); // average to prevent non-linearity
		LVV_Weight = fabs(lv.z());

		// balance LVH_UVW and LVH_UVV against each other, should be weighted equally
		LVH_Rescale_Weight = LVH_UVV_Weight + LVH_UVH_Weight;
		if(LVH_Rescale_Weight > 0.0f)
			{
			LVH_UVV_Weight /= LVH_Rescale_Weight;
			LVH_UVH_Weight /= LVH_Rescale_Weight;
			} // if

		// balance (LVH_UVW+LVH_UVV) against LVV, should be weighted equally
		Overall_Rescale_Weight = (LVH_UVV_Weight + LVH_UVH_Weight) + LVV_Weight;
		if(Overall_Rescale_Weight > 0.0f)
			{
			LVH_UVV_Weight /= Overall_Rescale_Weight;
			LVH_UVH_Weight /= Overall_Rescale_Weight;
			LVV_Weight /= Overall_Rescale_Weight;
			} // if

		// now lerp between these guys, based upon the factors
		ForwardBack += (LVH_UVV_FB * (dy * LocalMoveSpeed * dt)) * LVH_UVV_Weight;
		LeftRight   += (LVH_UVV_LR * (dx * LocalMoveSpeed * dt)) * LVH_UVV_Weight;

		ForwardBack += (LVV_FB * (dy * LocalMoveSpeed * dt)) * LVV_Weight;
		LeftRight   += (LVV_LR * (dx * LocalMoveSpeed * dt)) * LVV_Weight;

		// dx and dy ought to be transposed here to feel normal, but that would be weird too
		ForwardBack += (LVH_UVH_FB * (dy * LocalMoveSpeed * dt)) * LVH_UVH_Weight;
		LeftRight   += (LVH_UVH_LR * (dx * LocalMoveSpeed * dt)) * LVH_UVH_Weight;

		_eye += ForwardBack;
		_eye += LeftRight;
		} // if left button down

	// Handle cruise velocity
	if(MasterScene.GetThrottleSpeed() > 0.0)
		{
		lv *= (MasterScene.GetThrottleSpeed()*dt);
		_eye += lv;
		} // if
	} // else if
else if(((Navigation::GetCurrentNavMode() == Navigation::NAV_NM_ROTATE) || (modMask & GUIEventAdapter::MODKEY_CTRL)) && !(modMask & GUIEventAdapter::MODKEY_SHIFT)  && !(modMask & GUIEventAdapter::MODKEY_ALT))
	{
	if (buttonMask & GUIEventAdapter::LEFT_MOUSE_BUTTON)
		{
		if ((dx != 0.0) || (dy != 0.0))
			{
			UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
			} // if
		// process horizontal mouse-axis (yaw) rotation
		pitch =  inDegrees(dy*75.0f*dt);
		yaw   = -inDegrees(dx*50.0f*dt);
		} // if left button down
	// Handle cruise velocity
	if(MasterScene.GetThrottleSpeed() > 0.0)
		{
		lv *= (MasterScene.GetThrottleSpeed()*dt);
		_eye += lv;
		} // if
	} // else if
else if((Navigation::GetCurrentNavMode() == Navigation::NAV_NM_CLIMB) || (modMask & GUIEventAdapter::MODKEY_SHIFT))
	{
	if ((dx != 0.0) || (dy != 0.0))
		{
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if

	if (buttonMask & GUIEventAdapter::LEFT_MOUSE_BUTTON)
		{
		osg::Vec3 UpDown, GlobalUp;
		GlobalUp = osg::Vec3(0.0f,0.0f,1.0f);
		UpDown = GlobalUp * (dy * LocalMoveSpeed * dt);
		_eye += UpDown;
		} // if left button down

	// Handle cruise velocity
	if(MasterScene.GetThrottleSpeed() > 0.0)
		{
		lv *= (MasterScene.GetThrottleSpeed()*dt);
		_eye += lv;
		} // if
	} // else if
else if((Navigation::GetCurrentNavMode() == Navigation::NAV_NM_QUERY))
	{
	// no movement -- action-invocation is handled elsewhere
	} // else if

// handle 'orbit' functionality which is not mode-specific
osg::Vec3 OrbitCenter;
if(GetOrbitCenter(OrbitCenter))
	{
	if (buttonMask & GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
		{
		if ((dx != 0.0) || (dy != 0.0))
			{
			UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
			} // if
		// We need to counter-rotate the view the same angles that we orbit around the OrbitCenter, so these are negated
		pitch = -inDegrees(dy*75.0f*dt);
		yaw   =  inDegrees(dx*50.0f*dt);
		
		// move OrbitCenter to Origin (f0r rotation around origin)
		_eye -= OrbitCenter;
		
		// spin around Origin/OrbitCenter
		osg::Quat orbit_LR_rotate, orbit_UD_rotate;
		orbit_UD_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
		orbit_LR_rotate.makeRotate(yaw,0.0f,0.0f,1.0f); // yaw on world up vector
		
		// combine all the fancy camera-position rotations and apply them
		_eye =  orbit_UD_rotate * orbit_LR_rotate * _eye;
		
		// move OrbitCenter back
		_eye += OrbitCenter;
		
		} // if middle button down
	} // else if


EnforceConform(); 

osg::Quat delta_rotate;
if (pitch != 0.0 || bank != 0.0 || yaw != 0.0)
	{
	//double YawQLen;
	// process rotation
	osg::Quat pitch_rotate, yaw_rotate, bank_rotate;

	pitch_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
	yaw_rotate.makeRotate(yaw,0.0f,0.0f,1.0f); // yaw on world up vector
	//YawQLen = yaw_rotate.length();
	/*
	if(YawQLen > 0.0)
		{ // normalize
		yaw_rotate /= YawQLen;
		} // if
	*/
	//yaw_rotate.makeRotate(yaw,up.x(),up.y(),up.z()); // don't yaw around local up vector
	bank_rotate.makeRotate(bank,lv.x(),lv.y(),lv.z());

	//delta_rotate = pitch_rotate*yaw_rotate;
	delta_rotate = pitch_rotate*yaw_rotate*bank_rotate;

	_rotation = _rotation*delta_rotate;
	} // if

return true;
} // PanoManipulator::calcMovement





bool PanoManipulator::ExecuteMovement(double MX, double MY, double MZ)
{

if(GetMovementDisabledGlobal()) return(false);

if(MX == 0.0 && MY == 0.0 && MZ == 0.0) return(false);

double dt = 0.2;
double ExecuteVelocity = GetMovementSpeedGlobal();

UpdateLastMovementMoment(); // make note of time of last movement for movement optimization

osg::Vec3 MoveDelta(MX, MY, MZ);

osg::Matrix rotation_matrix;
rotation_matrix.makeRotate(_rotation);

osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * rotation_matrix;
osg::Vec3 lv = osg::Vec3(0.0f,0.0f,-1.0f) * rotation_matrix;
osg::Vec3 lr = osg::Vec3(1.0f,0.0f,0.0f) * rotation_matrix;
osg::Vec3 FwdBack, LeftRight, UpDown;

MoveDelta *= (ExecuteVelocity * dt);
// move on X is left/right, positive = right (I think)
LeftRight = osg::Vec3(lr.x() * MoveDelta.x(), lr.y() * MoveDelta.x(), lr.z() * MoveDelta.x());
_eye += LeftRight;

// move on Y is forward/backward, positive = forward
FwdBack = osg::Vec3(lv.x() * MoveDelta.y(), lv.y() * MoveDelta.y(), lv.z() * MoveDelta.y());
_eye += FwdBack;

// move on Z is up/down, positive = up (I think)
UpDown = osg::Vec3(up.x() * MoveDelta.z(), up.y() * MoveDelta.z(), up.z() * MoveDelta.z());
_eye += UpDown;


EnforceConform(); 

return true;
} // PanoManipulator::ExecuteMovement


bool PanoManipulator::ExecuteRotate(double RX, double RY, double RZ)
{

if(GetMovementDisabledGlobal()) return(false);

if(RX == 0.0 && RY == 0.0 && RZ == 0.0) return(false);

double dt = 1.0;

UpdateLastMovementMoment(); // make note of time of last movement for movement optimization

osg::Vec3 RotDelta(RX, RY, RZ);

RotDelta *= dt;

osg::Matrix rotation_matrix;
rotation_matrix.makeRotate(_rotation);

osg::Vec3 up = osg::Vec3(0.0f,1.0f,0.0) * rotation_matrix;
osg::Vec3 lv = osg::Vec3(0.0f,0.0f,-1.0f) * rotation_matrix;

osg::Vec3 sv = lv^up;
sv.normalize();

float yaw   = -inDegrees(360.0f*RotDelta.x());
float bank  =  inDegrees(360.0f*RotDelta.y());
float pitch =  inDegrees(360.0f*RotDelta.z());

osg::Quat delta_rotate;

osg::Quat pitch_rotate;
pitch_rotate.makeRotate(pitch,sv.x(),sv.y(),sv.z());
//delta_rotate = pitch_rotate;

osg::Quat yaw_rotate;
yaw_rotate.makeRotate(yaw,0.0f,0.0f,1.0f);

osg::Quat bank_rotate;
bank_rotate.makeRotate(bank,lv.x(),lv.y(),lv.z());

//delta_rotate = delta_rotate*yaw_rotate;
//delta_rotate = pitch_rotate*yaw_rotate;
delta_rotate = pitch_rotate*yaw_rotate*bank_rotate;

_rotation = _rotation*delta_rotate;

return true;
} // PanoManipulator::ExecuteRotate


void PanoManipulator::EnforceConform(void)
{

if(CheckFollowTerrainEnabledGlobal()) // enforce minimum height over terrain
	{
	double CurrentElev = 0.0;
	float CurX, CurY, CurZ;
	// Do something with FollowTerrainHeight
	float FollowHeight = GetFollowTerrainHeightGlobal();

	if(GetCurCameraCurXYZ(CurX, CurY, CurZ))
		{
		if(FetchTerrainHeight(NULL, CurX, CurY, CurrentElev))
			{
			//_eye.set(_eye.x(), _eye.y(), _eye.z());
			//_eye.set(_eye.x(), _eye.y(), max(CurrentElev + FollowHeight, _eye.z()));
			if(CurrentElev + FollowHeight > _eye.z())
				{ // perform lifting operation
				_eye.set(_eye.x(), _eye.y(), max(CurrentElev + FollowHeight, _eye.z()));
				UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
				if(GetMovementFrictionGlobal() > 0.0f)
					{
					_velocity *= (1.0 - GetMovementFrictionGlobal()); // slow down by inverse of friction factor
					} // if
				} // if
			if(CheckFollowTerrainHeightMaxGlobal()) // enforce maximum height over terrain
				{
				FollowHeight = GetFollowTerrainHeightMaxGlobal();
				if(CurrentElev + FollowHeight < _eye.z())
					{
					UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
					_eye.set(_eye.x(), _eye.y(), min(CurrentElev + FollowHeight, _eye.z()));
					} // if
				} // if
			} // if
		} // if

	} // if
if(GetMovementConstrainGlobal())
	{ // keep within DEM playpen
	float CurX, CurY;
	CurX = _eye.x();
	CurY = _eye.y();
	if(CurX < GetMovementXMinGlobal())
		{
		_eye.set(GetMovementXMinGlobal(), _eye.y(), _eye.z());
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if
	if(CurX > GetMovementXMaxGlobal())
		{
		_eye.set(GetMovementXMaxGlobal(), _eye.y(), _eye.z());
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if
	if(CurY < GetMovementYMinGlobal())
		{
		_eye.set(_eye.x(), GetMovementYMinGlobal(), _eye.z());
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if
	if(CurY > GetMovementYMaxGlobal())
		{
		_eye.set(_eye.x(), GetMovementYMaxGlobal(), _eye.z());
		UpdateLastMovementMoment(); // make note of time of last movement for movement optimization
		} // if
	} // if

} // PanoManipulator::EnforceConform



void PanoManipulator::UpdateUndo(void)
{
_eyeUndo = _eye;
_rotationUndo = _rotation;
} // PanoManipulator::UpdateUndo


void PanoManipulator::RestoreFromUndo(void)
{ // move current position into Undo, so we can un-Undo, and then DO the Undo
osg::Vec3   _eyeTemp;
osg::Quat   _rotationTemp;

// move current position into Temp
_eyeTemp = _eye;
_rotationTemp = _rotation;

// move Undo position into current position
_eye = _eyeUndo;
_rotation = _rotationUndo;

// move Temp position (formerly current position) into Undo
_eyeUndo = _eyeTemp;
_rotationUndo = _rotationTemp;

} // PanoManipulator::RestoreFromUndo


void PanoManipulator::Goto(float X, float Y)
{
osg::Vec3 IntersectPoint;
if (GetFirstTangibleHitLocation(X, Y, IntersectPoint))
	{
	PartialVertexDEM IntersectLoc;
	IntersectLoc.XYZ[0] = IntersectPoint.x();
	IntersectLoc.XYZ[1] = IntersectPoint.y();
	IntersectLoc.XYZ[2] = IntersectPoint.z();
	MasterScene.CartToDeg(IntersectLoc);

	_eye += ((IntersectPoint - _eye) * .9); // take us 90% of the way towards the intersection point
	EnforceConform(); // stay inside bounds if necessary
	} // if
} // PanoManipulator::Goto

// returns true if recorded a hit
bool PanoManipulator::GetFirstTangibleHitLocation(float X, float Y, osg::Vec3 &IntersectPoint)
{

osgUtil::IntersectVisitor::HitList hlist;
osgProducer::Viewer *_viewer;

osg::Node::NodeMask traversalMask = NVW_NODEMASK_TANGIBLE;

_viewer = GetGlobalViewer();

if (_viewer->computeIntersections(X, Y, hlist, traversalMask))
	{
    for(osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin(); hitr!=hlist.end(); ++hitr)
	    {
        IntersectPoint = hitr->getWorldIntersectPoint();
		return(true); // only do first hit item
		} // for
	} // if
return(false);
} // PanoManipulator::GetFirstTangibleHitLocation

void PanoManipulator::JumpTo(osg::Vec3 NewSpatialPos)
{
PartialVertexDEM JumpLoc;
JumpLoc.Lat = NewSpatialPos.y();
JumpLoc.Lon = NewSpatialPos.x();
JumpLoc.Elev = NewSpatialPos.z();
MasterScene.DegToCart(JumpLoc);

_eye[0] = JumpLoc.XYZ[0];
_eye[1] = JumpLoc.XYZ[1];
_eye[2] = JumpLoc.XYZ[2];
} // PanoManipulator::JumpTo

void PanoManipulator::JumpTo(double SpatialX, double SpatialY)
{
PartialVertexDEM JumpLoc;
JumpLoc.Lat = SpatialY;
JumpLoc.Lon = SpatialX;
JumpLoc.Elev = 0;
MasterScene.DegToCart(JumpLoc);

_eye[0] = JumpLoc.XYZ[0];
_eye[1] = JumpLoc.XYZ[1];
} // PanoManipulator::JumpTo
