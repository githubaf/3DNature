


#ifndef NVW_CAMERASUPPORT_H
#define NVW_CAMERASUPPORT_H

#include <osg/Vec3>
#include <osg/Quat>
#include <osg/PositionAttitudeTransform>



class NVAnimObject;
class MiniKeyFrame;

double GetCameraAnimLength(int CamNum);
int GetCameraStartPos(int CamNum, float &CX, float &CY, float &CZ, float &TX, float &TY, float &TZ, float &HFOV, float &Bank);
int GetCurCameraNum(void);

int GetCurCameraCurXYZ(float &CX, float &CY, float &CZ);
int GetCurCameraCurFov(float &HFOV, float &VFOV);
int GetCurCameraCurHeadingRadians(float &Heading);

void ConvertPosToCart(float &CX, float &CY, float &CZ, double Lat, double Lon, float Elev);

// returns result in passed rotationDest var
void ComputeOrientation(const osg::Vec3& lv, const osg::Vec3& up, osg::Quat &rotationDest);


int InterpretCameraForUse(NVAnimObject *Cam, double Moment, osg::Vec3 &EyePoint, osg::Quat &Orientation, float &HFOV);
int InterpretCameraConfiguration(double CLat, double CLon, float CElev, double TLat, double TLon, float TElev, float Bank, osg::Vec3 &EyePoint, osg::Quat &Orientation);
int InterpretCameraConfiguration(MiniKeyFrame *CurKF, osg::Vec3 &EyePoint, osg::Quat &Orientation);

#endif // !NVW_CAMERASUPPORT_H

