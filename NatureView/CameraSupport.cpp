
#include <osgProducer/Viewer>
#include <osg/PositionAttitudeTransform>

#include "UsefulMath.h"
#include "CameraSupport.h"
#include "NVScene.h"
#include "Viewer.h"
#include "NVMathSupport.h"
#include "RequesterBasic.h"

extern NVScene MasterScene;

extern double DXG, DYG, MiscReadout, MiscReadoutTwo;

int GetCurCameraNum(void) {return(MasterScene.GetCurrentCameraNum());};

double GetCameraAnimLength(int CamNum)
{
if(MasterScene.CheckAnyCameras() && MasterScene.CheckCameraName())
	{
	NVAnimObject *CurCam;

	if(CurCam = MasterScene.GetCamera(CamNum))
		{
		return(CurCam->GetTimeOfEndKey());
		} // if
	} // if

return(0); // no cam info in file
} // GetCameraAnimLength


// should these be part of a CameraInterpreter class/object
static MiniKeyFrame *PrevKey, *NextKey;

void ClearCameraKeyCache(void)
{
PrevKey = NextKey = NULL;
} // ClearCameraKeyCache

int InterpretCameraForUse(NVAnimObject *Cam, double Moment, osg::Vec3 &EyePoint, osg::Quat &Orientation, float &HFOV)
{
int NumValid;

float HFOV_S, HFOV_N, HFOV_E;

NumValid = Cam->GetBracketingKeyFrames(Moment, PrevKey, NextKey, false); // don't use caching yet, we're not set up for it

if(NumValid)
	{
	if(NumValid == 2)
		{ // interpolate
		double KeyTimeDif, MomentTimeDif;

		//determine interpolation fraction
		KeyTimeDif = NextKey->GetTimeValue() - PrevKey->GetTimeValue();
		MomentTimeDif = Moment - PrevKey->GetTimeValue();

		if(KeyTimeDif > 0.0 && MomentTimeDif >= 0.0) // do rationality checking before a risky divide
			{
			double MomentFrac;
			osg::Vec3 EyePoint_S, EyePoint_E;
			osg::Quat Orientation_S, Orientation_E;

			MomentFrac = MomentTimeDif / KeyTimeDif; // divide by zero prevented by rationality checking above
			MiscReadout = MomentFrac;
			if(MomentFrac > 1.0)
				{
				Cam->GetBracketingKeyFrames(Moment, PrevKey, NextKey, false); // trace only
				} // if

			// HFOV
			HFOV_S = PrevKey->ChannelValues[NVAnimObject::CANONHANGLE];
			HFOV_E = NextKey->ChannelValues[NVAnimObject::CANONHANGLE];
			HFOV_N = flerp(MomentFrac, HFOV_S, HFOV_E);
			HFOV = HFOV_N;

			// create start and end eye/orient pairs
			InterpretCameraConfiguration(PrevKey, EyePoint_S, Orientation_S);
			InterpretCameraConfiguration(NextKey, EyePoint_E, Orientation_E);

			// Eye position
			EyePoint = EyePoint_S + ((EyePoint_E - EyePoint_S) * MomentFrac);

			// Orientation quat via slerp
			Orientation.slerp(MomentFrac, Orientation_S, Orientation_E);

			return(1);
			} // if
		else
			{ // drop back ten and punt by going with NumValid == 1 code below
			NumValid = 1;
			} // else
		} // if
	if(NumValid == 1) // not an else-if, this is a fail-safe case if the above NumValid=2 fails somehow
		{ // only have one, only use PrevKey, NextKey is identical and tweening would be a waste of time
		InterpretCameraConfiguration(PrevKey, EyePoint, Orientation);
		HFOV = PrevKey->ChannelValues[NVAnimObject::CANONHANGLE];
		} // else
	} // if
return(0);
} // InterpretCameraForUse


int InterpretCameraConfiguration(MiniKeyFrame *CurKF, osg::Vec3 &EyePoint, osg::Quat &Orientation)
{
return(InterpretCameraConfiguration(CurKF->ChannelValues[NVAnimObject::CANONY], CurKF->ChannelValues[NVAnimObject::CANONX], CurKF->ChannelValues[NVAnimObject::CANONZ],
 CurKF->ChannelValues[NVAnimObject::CANONAUXY], CurKF->ChannelValues[NVAnimObject::CANONAUXX], CurKF->ChannelValues[NVAnimObject::CANONAUXZ],
 CurKF->ChannelValues[NVAnimObject::CANONB], EyePoint, Orientation));
} // InterpretCameraConfiguration


int InterpretCameraConfiguration(double CLat, double CLon, float CElev, double TLat, double TLon, float TElev, float Bank, osg::Vec3 &EyePoint, osg::Quat &Orientation)
{
float CX, CY, CZ, TX, TY, TZ;

ConvertPosToCart(CX, CY, CZ, CLat, CLon, CElev);
ConvertPosToCart(TX, TY, TZ, TLat, TLon, TElev);

osg::Vec3 eye = osg::Vec3(CX,CY,CZ);
osg::Vec3 lv, upvec, rotupvec;
osg::Matrix mat;

upvec = osg::Vec3(0.0f,0.0f,1.0f);
lv = osg::Vec3(TX,TY,TZ) - eye;

mat.makeRotate( osg::DegreesToRadians(-Bank), lv );
rotupvec = upvec * mat;

ComputeOrientation(lv, rotupvec, Orientation);
EyePoint = eye;

return(0);
} // InterpretCameraConfiguration

//char DebugMsg[1024];
//osg::Matrix debug_matrix;
//osg::Quat debug_quat;

void ComputeOrientation(const osg::Vec3& lv,const osg::Vec3& up, osg::Quat &rotationDest)
{
osg::Vec3 f(lv);
f.normalize();
osg::Vec3 s(f^up);
s.normalize();
osg::Vec3 u(s^f);
u.normalize();


{
__asm NOP
}


osg::Matrix rotation_matrix(s[0],     u[0],     -f[0],     0.0f,
                            s[1],     u[1],     -f[1],     0.0f,
                            s[2],     u[2],     -f[2],     0.0f,
                            0.0f,     0.0f,     0.0f,      1.0f);
rotationDest.set(rotation_matrix);
//debug_matrix = rotation_matrix;
//debug_quat = rotationDest;
rotationDest = rotationDest.inverse();

} // ComputeOrientation

/*
void DEBUGDUMP()
{

sprintf(DebugMsg, "%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f",
debug_matrix(0,0), debug_matrix(1,0), debug_matrix(2,0), debug_matrix(3,0),
debug_matrix(0,1), debug_matrix(1,1), debug_matrix(2,1), debug_matrix(3,1),
debug_matrix(0,2), debug_matrix(1,2), debug_matrix(2,2), debug_matrix(3,2),
debug_matrix(0,3), debug_matrix(1,3), debug_matrix(2,3), debug_matrix(3,3)
);

sprintf(DebugMsg, "%f, %f, %f, %f",
debug_quat[0], debug_quat[1], debug_quat[2], debug_quat[3]);
UserMessageOK("DEBUG", DebugMsg);
} // DEBUGDUMP
*/


void ConvertPosToCart(float &CX, float &CY, float &CZ, double Lat, double Lon, float Elev)
{
PartialVertexDEM TempLoc;

TempLoc.Lat  = Lat;
TempLoc.Lon  = Lon;
TempLoc.Elev = Elev;

MasterScene.DegToCart(TempLoc);

CX = TempLoc.XYZ[0];
CY = TempLoc.XYZ[1];
CZ = TempLoc.XYZ[2];
} // ConvertPosToCart


int GetCameraStartPos(int CamNum, float &CX, float &CY, float &CZ, float &TX, float &TY, float &TZ, float &HFOV, float &Bank)
{
/*
osg::Vec3 TempEye;
osg::Quat TempOrient;
float TempHFOV;
InterpretCameraForUse(MasterScene.GetCamera(CamNum), 1.0, TempEye, TempOrient, TempHFOV);
*/
if(MasterScene.CheckAnyCameras() && MasterScene.CheckCameraName())
	{
	ConvertPosToCart(CX, CY, CZ, MasterScene.GetCamPosLat(CamNum), MasterScene.GetCamPosLon(CamNum), MasterScene.GetCamPosElev(CamNum));
	ConvertPosToCart(TX, TY, TZ, MasterScene.GetTargPosLat(CamNum), MasterScene.GetTargPosLon(CamNum), MasterScene.GetTargPosElev(CamNum));

	HFOV = MasterScene.GetCamHFOV(CamNum);
	Bank = MasterScene.GetCamBank(CamNum);
	return(1);
	} // if

return(0); // no cam info in file

} // GetCameraStartPos

int GetCurCameraCurXYZ(float &CX, float &CY, float &CZ)
{

if(MasterScene.CheckAnyCameras() && MasterScene.CheckCameraName())
	{
	osg::Vec3 Eye, Center, Up;
	PartialVertexDEM CamLoc;
	GetGlobalViewer()->getSceneHandlerList()[0]->getSceneView()->getViewMatrixAsLookAt(Eye, Center, Up);
	CamLoc.XYZ[0] = Eye.x();
	CamLoc.XYZ[1] = Eye.y();
	CamLoc.XYZ[2] = Eye.z();
	MasterScene.CartToDeg(CamLoc);
	CX = CamLoc.Lon;
	CY = CamLoc.Lat;
	CZ = CamLoc.Elev;
	return(1);
	} // if

return(0); // no cam info in file

} // GetCurCameraCurXYZ

int GetCurCameraCurFov(float &HFOV, float &VFOV)
{
HFOV = GetGlobalViewer()->getLensHorizontalFov();
VFOV = GetGlobalViewer()->getLensVerticalFov();
return(1);
} // GetCurCameraFov


int GetCurCameraCurHeadingRadians(float &Heading)
{
osg::Vec3 Eye, Center, Up;
PartialVertexDEM CamLoc;
GetGlobalViewer()->getSceneHandlerList()[0]->getSceneView()->getViewMatrixAsLookAt(Eye, Center, Up);
CamLoc.XYZ[0] = Eye.x();
CamLoc.XYZ[1] = Eye.y();
CamLoc.XYZ[2] = Eye.z();
MasterScene.CartToDeg(CamLoc);

Heading = 0;

// calculate camera heading
// first, make a view direction unit normal from Eye and Center
osg::Vec3 ViewVec;
ViewVec = (Center - Eye);
if(fabs(ViewVec.z()) != 1.0) // heading is undefined when looking straight up
	{
	Heading = findangle3(-ViewVec.y(), ViewVec.x());
	} // if
return(1);
} // GetCurCameraCurHeadingRadians

