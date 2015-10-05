// MathSupport.h
// Copyright 1996, Questar Productions

#include "stdafx.h"

#ifndef WCS_MATHSUPPORT_H
#define WCS_MATHSUPPORT_H

#include "Types.h"
#include "UsefulMath.h"

class VectorPoint;
class Vertex3D;
class VertexDEM;
class VertexData;

struct GeneralLinearEquation
	{
	double CoefA, CoefB, CoefC;
	}; // struct GeneralLinearEquation

double findangle2(double pta, double ptb);
void FindPosVector(Vertex3D *OP, Vertex3D *EP, struct coords *SP);
void FindPosVector(Point3d OP, Point3d EP, Point3d SP);
void FindPosVector(Point3d OP, Point3d EP, struct coords *SP);
void VectorMagnitude(struct coords *TP);
void VectorMagnitude(Vertex3D *TP);
double VectorMagnitude(Point3d TP);
float VectorMagnitude(Point3f TP);
void VectorMagnitudeNosqrt(struct coords *TP);
void VectorMagnitudeNosqrt(Vertex3D *TP);
double VectorMagnitudeNosqrt(Point3d TP);
float VectorMagnitudeNosqrt(Point3f TP);
double PointDistance(Point3d EP, Point3d SP);
double PointDistanceNoSQRT(Point3d EP, Point3d SP);
void SurfaceNormal(struct coords *NP, struct coords *FP, struct coords *LP);
void SurfaceNormal(Point3d NP, Point3d FP, Point3d LP);
void SurfaceNormal(Point3d NP, Point3d A, Point3d B, Point3d C);
void UnitVector(struct coords *UV);
void UnitVector(Point3d UV);
void UnitVector(Point3f UV);
void UnitVectorCopy(Point3f UVTo, Point3d UV);
void UnitVectorCopy(Point3d UVTo, Point3d UV);
double UnitVectorMagnitude(Point3d UV);
float InvSqrt(float x);
double VectorAngle(struct coords *SN, struct coords *VP);
double VectorAngle(Point3d SN, struct coords *VP);
double VectorAngle(Point3d SN, Point3d VP);
double VectorAngleNoUnitize(Point3d SN, Point3d VP); // Assumes SN and VP are already unit-length
void VectorAverage(Point3d TN, Point3d SN, struct coords *VP);
short SurfaceVisible(struct coords *SN, struct coords *VP, short Face);
short SurfaceVisible(Point3d SN, Point3d VP, short Face);
int ComparePoint(Point3d A, Point3d B, double Tolerance);
int ComparePoint2d(Point2d A, Point2d B, double Tolerance);
int CompareLine2Point2d(Point2d A, Point2d B, Point2d C, double Tolerance, double &DistFromA);
void BuildRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx);
void BuildInvRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx);
void BuildTransformationMatrix4x4(double Tx, double Ty, double Tz, double Sx, double Sy, double Sz, double Rx, double Ry, double Rz, Matx4x4 RMatx);
void BuildInvTransformationMatrix4x4(double Tx, double Ty, double Tz, double Sx, double Sy, double Sz, double Rx, double Ry, double Rz, Matx4x4 RMatx);
void RotationMatrix3D(short m, double theta, Matx3x3 A);
void TranslationMatrix4x4(double tx, double ty, double tz, Matx4x4 A);
void ScaleMatrix4x4(double sx, double sy, double sz, Matx4x4 A);
void RotationMatrix4x4(int m, double Theta, Matx4x4 A);
void ZeroMatrix3x3(Matx3x3 A);
void CopyMatrix3x3(Matx3x3 A, Matx3x3 B);
void ZeroMatrix4x4(Matx4x4 A);
void CopyMatrix4x4(Matx4x4 A, Matx4x4 B);
void CopyPoint3d(Point3d To, Point3d From);
void CopyPoint3f(Point3d To, Point3d From);
void CopyPoint3f3d(Point3f To, Point3d From);
void ZeroCoords(struct coords *A);
void ZeroPoint3d(Point3d A);
void AddPoint3d(Point3d A, Point3d Add);
void AddPoint3f(Point3f A, Point3f Add);
void AddPoint3f3d(Point3f A, Point3d Add);
void DividePoint3d(Point3d A, double Div);
void MultiplyPoint3d(Point3d B, Point3d A, double Mult);
void NegateVector(struct coords *A);
void NegateVector(Point3d A);
void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C);
void Multiply4x4Matrices(Matx4x4 A, Matx4x4 B, Matx4x4 C);
void RotatePoint(struct coords *A, Matx3x3 M);
void RotatePoint(Vertex3D *A, Matx3x3 M);
void RotatePoint(double &X, double &Y, double &Z, Matx3x3 M);
void RotatePoint(double XYZ[3], Matx3x3 M);
void RotatePoint(Point2d, double Angle);
void RotateX(Point3d XYZ, double Angle);
void RotateY(Point3d XYZ, double Angle);
void RotateZ(Point3d XYZ, double Angle);
void Transform3DPoint(Vertex3D *A, Matx4x4 M);
double findangle(double pta, double ptb);
double findangle3(double pta, double ptb);
void rotate(double *pta, double *ptb, double rotangle, double angle);
void rotate2(double *pta, double *ptb, double rotangle, double angle);
int RaySphereIntersect(double Radius, Point3d VecOrigin, Point3d VecDirection, Point3d SphereCenter,
	Point3d Intersection1, double &Dist1, Point3d Intersection2, double &Dist2);
int RaySphereIntersect(double Radius, Point3d VecOrigin, Point3d VecDirection, Point3d SphereCenter,
	Point3d Intersection, double &Dist);
int RayQuadrilateralIntersect(Point3d VecOrigin, Point3d VecDirection, Point3d QuadNormal, double QuadVert[4][3],
	Point3d Intersection, double &Dist);
double DistPoint2Line(Point2d P, Point2d L1, Point2d L2);
double DistPoint2Point(Point2d P1, Point2d P2);
double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2, char *UsePoint, double &P1Offset, double &SegLength);
double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2);
double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2, Point2d Px);
short LineSegsIntersect(double Lat, double Lon, VectorPoint *P1, VectorPoint *P2);
short LineSegsIntersect(double Lat1, double Lon1, double Lat2, double Lon2, double LatPt, double LonPt);
int CalcGeneralLinearEquation(Point2d Pt1, Point2d Pt2, struct GeneralLinearEquation *Eq);
int CalcParallelLinearEquation(Point2d Pt1, struct GeneralLinearEquation *Input, struct GeneralLinearEquation *Output);
int PointContainedInSegment(Point2d SegStart, Point2d SegEnd, Point2d Pt);
//short PointEnclosedPoly3(VertexDEM *Vert[3], double LatPt, double LonPt);
int CalcLineIntersection(struct GeneralLinearEquation *LineA, struct GeneralLinearEquation *LineB, Point2d Intersection);
int CalcLineOrthogonal(Point2d Pt1, Point2d Pt2, int LastPt, struct GeneralLinearEquation *Eq);
short ComputePolygonInterpolants(double *XYZ, double *PolyX, double *PolyY, long PolyMinX, long PolyMinY, 
	short PolyMinXPt, long MinScrnX, long MinScrnY,
	short &MaxPt, short &MinPt, double &StartPt, double &IncX, double &IncY);
double SolveTriangleArea(VertexData *Vtx[3]);
double SolveTriangleArea2D(Point2d Pt1, Point2d Pt2, Point2d Pt3);
double SolveTriangleArea2D(VertexDEM *Vtx1, VertexDEM *Vtx2, VertexDEM *Vtx3);
//double bias(double a, double b);
//double gain(double a, double b);
void EulerToAxisAngle(Point3d EulerAngles, Point4d AxisAngle);
void EulerToQuaternion(Point3d EulerAngles, Point4d Quat);
void AxisAngleToQuaternion(Point4d AxisAngle, Point4d Quat);
void QuaternionToAxisAngle(Point4d AxisAngle, Point4d Quat);
void MultiplyQuaternions(Point4d QuatOut, Point4d Q1, Point4d Q2);


float MinPointLineSq (const eberlyPoint3& p, const Line3& line, float& t);

double inline Signum(double x) {return (x == 0.0 ? 0.0 : x < 0.0 ? -1.0 : 1.0);};	// returns sign of a number

//---------------------------------------------------------------------------
inline double SafePow(double Base, double Exponent)
{
int BaseExp;

(void)frexp(Base, &BaseExp);
BaseExp *= (int)Exponent;	// Exponent is base 10, BaseExp is base 2 (which is larger so result will be conservative)
if (BaseExp > DBL_MAX_10_EXP - 10)		// pad the test to be safe
	return (FLT_MAX);		// return a value that is small enough so that used in other math ops it won't overflow
if (BaseExp < DBL_MIN_10_EXP + 10)
	return (FLT_MIN);		// return a value that is small enough so that used in other math ops it won't underflow

return pow(Base, Exponent);

}
//---------------------------------------------------------------------------
inline eberlyPoint3 operator- (const eberlyPoint3& p, const eberlyPoint3& q)
{
    eberlyPoint3 sub;
    sub.x = p.x - q.x;
    sub.y = p.y - q.y;
    sub.z = p.z - q.z;
    return sub;
}
//---------------------------------------------------------------------------
inline eberlyPoint3 operator* (float t, const eberlyPoint3& p)
{
    eberlyPoint3 prod;
    prod.x = t*p.x;
    prod.y = t*p.y;
    prod.z = t*p.z;
    return prod;
}
//---------------------------------------------------------------------------
inline eberlyPoint3 operator* (const eberlyPoint3& p, float t)
{
    eberlyPoint3 prod;
    prod.x = t*p.x;
    prod.y = t*p.y;
    prod.z = t*p.z;
    return prod;
}
//---------------------------------------------------------------------------
inline float eberlyDot (const eberlyPoint3& p, const eberlyPoint3& q)
{
    return p.x*q.x + p.y*q.y + p.z*q.z;
}



#endif // WCS_MATHSUPPORT_H
