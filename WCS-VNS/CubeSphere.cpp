// CubeSphere.cpp
// Code to implement Cubic panoramic to spherical or hemispherical texture mapping
// Created from scratch Jul 17, 2003 by CXH
// Copyright 2003

#include "stdafx.h"
#include "CubeSphere.h"

int SphereToCube(double InLon, double InLat, unsigned char &OutCubeFace, double &OutX, double &OutY)
{
double XYZ[3], XYZMaxed[3];
//double Radius;
double Rsq, Rxz, Ysq, TempLon = InLon;
double XMag, YMag, ZMag;

//Radius = 1.0;
XYZ[1] = sin(InLat * 1.74532925199433E-002); // PiOver180 (no defines needed)
Rxz = (Rsq = 1.0) - (Ysq = XYZ[1] * XYZ[1]);
Rxz = (Rxz <= 0.0 ? 0.0 : sqrt(Rxz));
XYZ[0] = Rxz * sin(-InLon * 1.74532925199433E-002); // PiOver180 (no defines needed)
Rsq = Rsq - Ysq - XYZ[0] * XYZ[0];
XYZ[2] = (Rsq <= 0.0 ? 0.0 : sqrt(Rsq));

if (fabs(TempLon) < 90.0)
	XYZ[2] = -XYZ[2];

XMag = fabs(XYZ[STC_COORDINDEX_X]);
YMag = fabs(XYZ[STC_COORDINDEX_Y]);
ZMag = fabs(XYZ[STC_COORDINDEX_Z]);

// project unit sphere out to intersection on unit cube (MaximizeToAxis)
// determine cube face intersected
// translate 'global' coords into proper local coords on cube face per WCS/VNS convention
if((XMag >= YMag) && (XMag >= ZMag))
	{ // X wins, must be B (South) or D (North)
	MaximizeToXAxis(XYZ, XMag, XYZMaxed);
	if(XYZ[STC_COORDINDEX_X] > 0) // should never == 0
		{ // B (South)
		OutCubeFace = STC_CUBEFACENAME_REAR_SOUTH;
		OutX = -XYZMaxed[STC_COORDINDEX_Z];
		OutY = -XYZMaxed[STC_COORDINDEX_Y];
		} // if
	else
		{ // D (North)
		OutCubeFace = STC_CUBEFACENAME_FRONT_NORTH;
		OutX = XYZMaxed[STC_COORDINDEX_Z];
		OutY = -XYZMaxed[STC_COORDINDEX_Y];
		} // else
	} // if
else if((ZMag >= YMag) && (ZMag >= XMag))
	{ // Z wins, must be C (West) or A/E (East)
	MaximizeToZAxis(XYZ, ZMag, XYZMaxed);
	if(XYZ[STC_COORDINDEX_Z] > 0) // should never == 0
		{ // A/E (East)
		OutCubeFace = STC_CUBEFACENAME_RIGHT_EAST;
		OutX = XYZMaxed[STC_COORDINDEX_X];
		OutY = -XYZMaxed[STC_COORDINDEX_Y];
		} // if
	else
		{ // C (West)
		OutCubeFace = STC_CUBEFACENAME_LEFT_WEST;
		OutX = -XYZMaxed[STC_COORDINDEX_X];
		OutY = -XYZMaxed[STC_COORDINDEX_Y];
		} // else
	} // if
else if((YMag >= XMag) && (YMag >= ZMag))
	{ // Y wins, must be Top or Bottom
	MaximizeToYAxis(XYZ, YMag, XYZMaxed);
	if(XYZ[STC_COORDINDEX_Y] > 0) // should never == 0
		{ // Top
		OutCubeFace = STC_CUBEFACENAME_TOP;
		OutX = XYZMaxed[STC_COORDINDEX_Z];
		OutY = -XYZMaxed[STC_COORDINDEX_X];
		} // if
	else
		{ // Bottom
		OutCubeFace = STC_CUBEFACENAME_BOT;
		OutX = XYZMaxed[STC_COORDINDEX_Z];
		OutY = XYZMaxed[STC_COORDINDEX_X];
		} // else
	} // if

// Remap from -.1 ... +.1 domain to 0.0 ... 1.0
OutX = (OutX * .5) + .5;
OutY = (OutY * .5) + .5;

return(1);
} // SphereToCube


// Do not call with Mag = 0 -- crashes
void MaximizeToXAxis(double *XYZIn, double Mag, double *MaxOut)
{
double ScaleUpFac;

ScaleUpFac = 1.0 / Mag;

// for efficiency, we don't need to do X itself
MaxOut[STC_COORDINDEX_Y] = XYZIn[STC_COORDINDEX_Y] * ScaleUpFac;
MaxOut[STC_COORDINDEX_Z] = XYZIn[STC_COORDINDEX_Z] * ScaleUpFac;
} // MaximizeToXAxis


// Do not call with Mag = 0 -- crashes
void MaximizeToYAxis(double *XYZIn, double Mag, double *MaxOut)
{
double ScaleUpFac;

ScaleUpFac = 1.0 / Mag;

// for efficiency, we don't need to do X itself
MaxOut[STC_COORDINDEX_X] = XYZIn[STC_COORDINDEX_X] * ScaleUpFac;
MaxOut[STC_COORDINDEX_Z] = XYZIn[STC_COORDINDEX_Z] * ScaleUpFac;
} // MaximizeToYAxis


// Do not call with Mag = 0 -- crashes
void MaximizeToZAxis(double *XYZIn, double Mag, double *MaxOut)
{
double ScaleUpFac;

ScaleUpFac = 1.0 / Mag;

// for efficiency, we don't need to do X itself
MaxOut[STC_COORDINDEX_Y] = XYZIn[STC_COORDINDEX_Y] * ScaleUpFac;
MaxOut[STC_COORDINDEX_X] = XYZIn[STC_COORDINDEX_X] * ScaleUpFac;
} // MaximizeToZAxis
