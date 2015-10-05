// MathSupport.cpp
// Math functions ripped from MapUtil.c
// By Gary R. Huber
// Copyright 1996, Questar Productions
// 3/26/96 Added some stuff from RenderUtil.cpp -CXH
// 7/4/97 Added functions to determine distance to a vector -GRH
// 10/27/99 Added new 3-vertex form of SurfaceNormal

#include "stdafx.h"
#include "MathSupport.h"
#include "Render.h"
#include "Points.h"
#include "EffectsLib.h"
#include "Render.h"
#include "UsefulTime.h"

//union quadDouble
//{
//__m128d	xmmdata[2];
//Point4d data;
//};

inline void FindPosVector(Vertex3D *OP, Vertex3D *EP, struct coords *SP)
{

OP->XYZ[0] = EP->XYZ[0] - SP->x;
OP->XYZ[1] = EP->XYZ[1] - SP->y;
OP->XYZ[2] = EP->XYZ[2] - SP->z;

} // FindPosVector

/*===========================================================================*/

inline void FindPosVector(Point3d OP, Point3d EP, Point3d SP)
{

OP[0] = EP[0] - SP[0];
OP[1] = EP[1] - SP[1];
OP[2] = EP[2] - SP[2];

} // FindPosVector

/*===========================================================================*/

inline void FindPosVector(Point3d OP, Point3d EP, struct coords *SP)
{

OP[0] = EP[0] - SP->x;
OP[1] = EP[1] - SP->y;
OP[2] = EP[2] - SP->z;

} // FindPosVector

/*===========================================================================*/

void VectorMagnitude(struct coords *TP)
{

TP->q = sqrt(TP->x * TP->x + TP->y * TP->y + TP->z * TP->z);

} // VectorMagnitude

/*===========================================================================*/

void VectorMagnitude(Vertex3D *TP)
{

TP->ScrnXYZ[2] = sqrt(TP->XYZ[0] * TP->XYZ[0] + TP->XYZ[1] * TP->XYZ[1] + TP->XYZ[2] * TP->XYZ[2]);

} // VectorMagnitude

/*===========================================================================*/

double VectorMagnitude(Point3d TP)
{

return (sqrt(TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2]));

} // VectorMagnitude

/*===========================================================================*/

float VectorMagnitude(Point3f TP)
{

return ((float)sqrt((double)(TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2])));

} // VectorMagnitude

/*===========================================================================*/

void VectorMagnitudeNosqrt(struct coords *TP)
{

TP->q = (TP->x * TP->x + TP->y * TP->y + TP->z * TP->z);

} // VectorMagnitudeNosqrt

/*===========================================================================*/

void VectorMagnitudeNosqrt(Vertex3D *TP)
{

TP->ScrnXYZ[2] = (TP->XYZ[0] * TP->XYZ[0] + TP->XYZ[1] * TP->XYZ[1] + TP->XYZ[2] * TP->XYZ[2]);

} // VectorMagnitudeNosqrt

/*===========================================================================*/

double VectorMagnitudeNosqrt(Point3d TP)
{

return (TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2]);

} // VectorMagnitudeNosqrt

/*===========================================================================*/

float VectorMagnitudeNosqrt(Point3f TP)
{

return (TP[0] * TP[0] + TP[1] * TP[1] + TP[2] * TP[2]);

} // VectorMagnitudeNosqrt

/*===========================================================================*/

double PointDistance(Point3d EP, Point3d SP)
{
Point3d TempDist;

FindPosVector(TempDist, EP, SP);
return(VectorMagnitude(TempDist));

} // PointDistance

/*===========================================================================*/

double PointDistanceNoSQRT(Point3d EP, Point3d SP)
{
Point3d TempDist;

FindPosVector(TempDist, EP, SP);
return (TempDist[0] * TempDist[0] + TempDist[1] * TempDist[1] + TempDist[2] * TempDist[2]);

} // PointDistanceNoSQRT

/*===========================================================================*/

void SurfaceNormal(struct coords *NP, struct coords *FP, struct coords *LP)
{

NP->x = FP->y * LP->z - FP->z * LP->y;
NP->y = FP->z * LP->x - FP->x * LP->z;
NP->z = FP->x * LP->y - FP->y * LP->x;
VectorMagnitude(NP); // this calculates the stored q value
UnitVector(NP); // this uses the stored q, without recalculating it

} // SurfaceNormal

/*===========================================================================*/

void SurfaceNormal(Point3d NP, Point3d FP, Point3d LP)
{

NP[0] = FP[1] * LP[2] - FP[2] * LP[1];
NP[1] = FP[2] * LP[0] - FP[0] * LP[2];
NP[2] = FP[0] * LP[1] - FP[1] * LP[0];

UnitVector(NP);

} // SurfaceNormal

/*===========================================================================*/

void SurfaceNormal(Point3d NP, Point3d A, Point3d B, Point3d C)
{
static Point3d SNSide[2];

FindPosVector(SNSide[0], B, A);
FindPosVector(SNSide[1], C, A);
// the WCS renderer likes normals generated from Side[1], Side[0] order,
// because WCS assumes +Z is into the screen.
// OpenGL likes normals in Side[0], Side[1] order because it assumes +Z
// is out of the screen.
SurfaceNormal(NP, SNSide[1], SNSide[0]);

} // SurfaceNormal

/*===========================================================================*/

void UnitVector(struct coords *UV)
{
double Invert;

if (UV->q > 0.0)
	{
	Invert = 1.0 / UV->q;
	UV->x *= Invert;
	UV->y *= Invert;
	UV->z *= Invert;
	UV->q = 1.0;
	} // if

} // UnitVector

/*===========================================================================*/

void UnitVector(Point3d UV)
{
double Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0 / Len;
	UV[0] *= Invert;
	UV[1] *= Invert;
	UV[2] *= Invert;
	} // if
#ifdef DEBUG
else
	{
	//assert(Len > 0.0);
	} // else
#endif

} // UnitVector

/*===========================================================================*/

double UnitVectorMagnitude(Point3d UV)
{
double Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0 / Len;
	UV[0] *= Invert;
	UV[1] *= Invert;
	UV[2] *= Invert;
	} // if
#ifdef DEBUG
else
	{
	//assert(Len > 0.0);
	} // else
#endif

return(Len);
} // UnitVectorMagnitude

/*===========================================================================*/

void UnitVectorCopy(Point3f UVTo, Point3d UV)
{
double Len;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	double Invert;
	Invert = 1.0 / Len;
	UVTo[0] = (float)(UV[0] * Invert);
	UVTo[1] = (float)(UV[1] * Invert);
	UVTo[2] = (float)(UV[2] * Invert);
	} // if
#ifdef DEBUG
else
	{
	assert(Len > 0.0);
	} // else
#endif

} // UnitVectorCopy

/*===========================================================================*/

void UnitVectorCopy(Point3d UVTo, Point3d UV)
{
double Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0 / Len;
	UVTo[0] = (UV[0] * Invert);
	UVTo[1] = (UV[1] * Invert);
	UVTo[2] = (UV[2] * Invert);
	} // if
#ifdef DEBUG
else
	{
	assert(Len > 0.0);
	} // else
#endif

} // UnitVectorCopy

/*===========================================================================*/

void UnitVector(Point3f UV)
{
float Len, Invert;

Len = VectorMagnitude(UV);
if (Len > 0.0)
	{
	Invert = 1.0f / Len;
	UV[0] *= Invert;
	UV[1] *= Invert;
	UV[2] *= Invert;
	} // if

} // UnitVector

/*===========================================================================*/

double VectorAngle(struct coords *SN, struct coords *VP)
{

if (SN->q <= 0.0 || VP->q <= 0.0)
	return (1.0);

// returns the cosine of the angle

return ((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z) / (SN->q * VP->q));

} // VectorAngle

/*===========================================================================*/

double VectorAngle(Point3d SN, struct coords *VP)
{
double LenSN;

LenSN = VectorMagnitude(SN);

if (LenSN <= 0.0 || VP->q <= 0.0)
	return (1.0);

// returns the cosine of the angle

return ((SN[0] * VP->x + SN[1] * VP->y + SN[2] * VP->z) / (LenSN * VP->q));

} // VectorAngle

/*===========================================================================*/

//extern double TotalTime;

// returns the cosine of the angle
double VectorAngle(Point3d SN, Point3d VP)
{
double LenSN, LenVP;
double rVal = 1.0;
//double *ptr0 = &SN[0], *ptr1 = &SN[2], *ptr2 = &VP[0], *ptr3 = &VP[2];
//__m128d stuff0, stuff1, stuff2, stuff3, stuff4, stuff5;

//StartHiResTimer();

/***
__asm
	{
	mov		eax, DWORD PTR SN
	movupd	xmm0, QWORD PTR [eax]		; unaligned - slower, but doesn't segfault :)  See if data can be aligned somehow
	movsd	xmm1, DWORD PTR [eax+16]
	movapd	xmm4, xmm0
	movapd	xmm5, xmm1
	mov		eax, DWORD PTR VP
	movupd	xmm2, QWORD PTR [eax]
	movsd	xmm3, DWORD PTR [eax+16]
	movapd	xmm6, xmm2
	movapd	xmm7, xmm3
	mulpd	xmm0, xmm0
	mulpd	xmm1, xmm1
	mulpd	xmm2, xmm2
	mulpd	xmm3, xmm3
	addpd	xmm0, xmm1
	addpd	xmm2, xmm3
	; sse3 !
	haddpd xmm0, xmm2
	; LenSN & LenVP now in xmm0
	; add conditional testing here
	movapd	xmm1, xmm0
	shufpd	xmm1, xmm0, 1
	mulsd	xmm1, xmm0
	sqrtsd	xmm1, xmm1
	mulpd	xmm4, xmm6
	mulpd	xmm5, xmm7
	haddpd	xmm5, xmm4
	movapd	xmm4, xmm5
	shufpd	xmm4, xmm5, 1
	addsd	xmm4, xmm5
	divsd	xmm4, xmm1
	movupd	rVal, xmm4
	}
***/

/***
stuff0 = _mm_load_pd(ptr0);	// SN[0], SN[1]
stuff1 = _mm_load_sd(ptr1);	// 0.0, SN[2]
stuff2 = _mm_load_pd(ptr2);	// VP[0], VP[1]
stuff3 = _mm_load_pd(ptr3);	// 0.0, VP[2]
stuff0 = _mm_mul_pd(stuff0, stuff0);	// SN[0]^2, SN[1]^2
stuff1 = _mm_mul_pd(stuff1, stuff1);	// 0.0, SN[2]^2
stuff2 = _mm_mul_pd(stuff2, stuff2);	// VP[0]^2, VP[1]^2
stuff3 = _mm_mul_pd(stuff3, stuff3);	// 0.0, VP[2]^2
stuff4 = _mm_add_pd(stuff0, stuff1);	// SN[0]^2 + 0.0, SN[1]^2 + SN[2]^2
stuff5 = _mm_add_pd(stuff2, stuff3);
stuff0.m128d_f64[0] = stuff4.m128d_f64[1];
stuff1.m128d_f64[0] = stuff5.m128d_f64[1];
stuff4 = _mm_add_sd(stuff0, stuff1);
***/

//looky here
LenSN = VectorMagnitudeNosqrt(SN);
LenVP = VectorMagnitudeNosqrt(VP);

if (LenSN > 0.0 && LenVP > 0.0)
	rVal = ((SN[0] * VP[0] + SN[1] * VP[1] + SN[2] * VP[2]) / sqrt(LenSN * LenVP));

//TotalTime += StopHiResTimerSecs();

return (rVal);

/***
if (LenSN <= 0.0 || LenVP <= 0.0)
	return (1.0);

return ((SN[0] * VP[0] + SN[1] * VP[1] + SN[2] * VP[2])
		/ sqrt(LenSN * LenVP));
***/

} // VectorAngle

/*===========================================================================*/

// Assumes SN and VP are already unit-length
double VectorAngleNoUnitize(Point3d SN, Point3d VP)
{

// returns the cosine of the angle
return ((SN[0] * VP[0] + SN[1] * VP[1] + SN[2] * VP[2]));

} // VectorAngleNoUnitize

/*===========================================================================*/

void VectorAverage(Point3d TN, Point3d SN, struct coords *VP)
{

TN[0] = (SN[0] + VP->x) * 0.5;
TN[1] = (SN[1] + VP->y) * 0.5;
TN[2] = (SN[2] + VP->z) * 0.5;

} // VectorAverage

/*===========================================================================*/

short SurfaceVisible(struct coords *SN, struct coords *VP, short Face)
{
short rVal;

if (!Face)
	rVal = (short)
	((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z) >= -0.1);	// a little added insurance
else
	rVal = (short)
	((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z) <= 0.1);

return (rVal);

} // SurfaceVisible

/*===========================================================================*/

short SurfaceVisible(Point3d SN, Point3d VP, short Face)
{
short rVal;

if (!Face)
	rVal = (short)((SN[0] * VP[0] + SN[1] * VP[1] + SN[2] * VP[2]) >= -0.1);
else
	rVal = (short)((SN[0] * VP[0] + SN[1] * VP[1] + SN[2] * VP[2]) <= 0.1);

return (rVal);

} // SurfaceVisible

/*===========================================================================*/

int ComparePoint(Point3d A, Point3d B, double Tolerance)
{

return (fabs(A[0] - B[0]) <= Tolerance && fabs(A[1] - B[1]) <= Tolerance && fabs(A[2] - B[2]) <= Tolerance);

} // ComparePoint

/*===========================================================================*/

int ComparePoint2d(Point2d A, Point2d B, double Tolerance)
{

return (fabs(A[0] - B[0]) <= Tolerance && fabs(A[1] - B[1]) <= Tolerance);

} // ComparePoint2d

/*===========================================================================*/

int CompareLine2Point2d(Point2d A, Point2d B, Point2d C, double Tolerance, double &DistFromA)
{
double Cycalc, M;

if (A[0] != B[0])
	{
	M = (B[1] - A[1]) / (B[0] - A[0]);
	Cycalc = A[1] + M * (C[0] - A[0]);
	if (fabs(C[1] - Cycalc) <= Tolerance)
		{
		Cycalc -= A[1];
		M = C[0] - A[0];
		DistFromA = sqrt(M * M + Cycalc * Cycalc);
		return (1);
		} // if
	} // if
else if (A[1] != B[1])
	{
	if (fabs(C[0] - A[0]) <= Tolerance && ((C[1] >= A[1] && C[1] <= B[1]) || (C[1] <= A[1] && C[1] >= B[1])))
		{
		DistFromA = fabs(C[1] - A[1]);
		return (1);
		} // if
	} // else if
else if (fabs(C[0] - A[0]) <= Tolerance && fabs (C[1] - A[1]) <= Tolerance)
	{
	DistFromA = 0.0;	
	return 1;
	} // else

return (0);

} // ComparePoint2d

/*===========================================================================*/

// used for camera view matrices, texture rotations, scene rotations for export, 3D Object sfc normals
void BuildRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx)
{
Matx3x3 XRot, YRot, ZRot, Temp;

RotationMatrix3D(1, Rx, XRot);
RotationMatrix3D(2, Ry, YRot);
RotationMatrix3D(3, Rz, ZRot);
Multiply3x3Matrices(XRot, ZRot, Temp);
Multiply3x3Matrices(YRot, Temp, RMatx);

} // BuildRotationMatrix

/*===========================================================================*/

// used for camera view matrices
void BuildInvRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx)
{
Matx3x3 XRot, YRot, ZRot, Temp;

RotationMatrix3D(1, -Rx, XRot);
RotationMatrix3D(2, -Ry, YRot);
RotationMatrix3D(3, -Rz, ZRot);
Multiply3x3Matrices(XRot, YRot, Temp);
Multiply3x3Matrices(ZRot, Temp, RMatx);

} // BuildInvRotationMatrix

/*===========================================================================*/

// used for 3d objects
void BuildTransformationMatrix4x4(double Tx, double Ty, double Tz, double Sx, double Sy, double Sz, double Rx, double Ry, double Rz, Matx4x4 RMatx)
{
Matx4x4 Scale, XRot, YRot, ZRot, Trans, Temp1, Temp2, Temp3;

ScaleMatrix4x4(Sx, Sy, Sz, Scale);
RotationMatrix4x4(1, Rx, XRot);
RotationMatrix4x4(2, Ry, YRot);
RotationMatrix4x4(3, Rz, ZRot);
TranslationMatrix4x4(Tx, Ty, Tz, Trans);
Multiply4x4Matrices(ZRot, Scale, Temp1);
Multiply4x4Matrices(XRot, Temp1, Temp2);
Multiply4x4Matrices(YRot, Temp2, Temp3);
Multiply4x4Matrices(Trans, Temp3, RMatx);

} // BuildTransformationMatrix4x4

/*===========================================================================*/

// used for 3d objects
void BuildInvTransformationMatrix4x4(double Tx, double Ty, double Tz, double Sx, double Sy, double Sz, double Rx, double Ry, double Rz, Matx4x4 RMatx)
{
Matx4x4 Scale, XRot, YRot, ZRot, Trans, Temp1, Temp2, Temp3;

if (Sx != 0.0 && Sy != 0.0 && Sz != 0.0)
	ScaleMatrix4x4(1.0 / Sx, 1.0 / Sy, 1.0 / Sz, Scale);
else
	ScaleMatrix4x4(1.0, 1.0, 1.0, Scale);
RotationMatrix4x4(1, -Rx, XRot);
RotationMatrix4x4(2, -Ry, YRot);
RotationMatrix4x4(3, -Rz, ZRot);
TranslationMatrix4x4(-Tx, -Ty, -Tz, Trans);
Multiply4x4Matrices(YRot, Trans, Temp1);
Multiply4x4Matrices(XRot, Temp1, Temp2);
Multiply4x4Matrices(ZRot, Temp2, Temp3);
Multiply4x4Matrices(Scale, Temp3, RMatx);

} // BuildInvTransformationMatrix4x4

/*===========================================================================*/

// used for camera view matrices, texture rotations, scene rotations for export
void RotationMatrix3D(short m, double theta, Matx3x3 A)
{
double c, s;
short m1, m2;

ZeroMatrix3x3(A);
A[m - 1][m - 1] = 1.0;
m1 = (m % 3) + 1;
m2 = m1 % 3;
m1 --;
sincos(theta, &s, &c);

A[m1][m1] = c;
A[m1][m2] = s;
A[m2][m2] = c;
A[m2][m1] = -s;

} // RotationMatrix3D

/*===========================================================================*/

void TranslationMatrix4x4(double tx, double ty, double tz, Matx4x4 A)
{

ZeroMatrix4x4(A);
A[0][0] = 1.0;
A[1][1] = 1.0;
A[2][2] = 1.0;
A[3][3] = 1.0;
A[0][3] = -tx;
A[1][3] = -ty;
A[2][3] = -tz;

} // TranslationMatrix4x4

/*===========================================================================*/

void ScaleMatrix4x4(double sx, double sy, double sz, Matx4x4 A)
{

ZeroMatrix4x4(A);
A[0][0] = sx;
A[1][1] = sy;
A[2][2] = sz;
A[3][3] = 1.0;

} // ScaleMatrix4x4

/*===========================================================================*/

// used for 3d objects
void RotationMatrix4x4(int m, double Theta, Matx4x4 A)
{
double c, s;
int m1, m2;

ZeroMatrix4x4(A);
A[m - 1][m - 1] = 1.0;
A[3][3] = 1.0;
m1 = (m % 3) + 1;
m2 = (m1 % 3);
m1 --;
sincos(Theta, &s, &c);
A[m1][m1] = c;
A[m1][m2] = s;
A[m2][m2] = c;
A[m2][m1] = -s;

} // RotationMatrix4x4

/*===========================================================================*/

void ZeroMatrix3x3(Matx3x3 A)
{

memset(A, 0, sizeof(Matx3x3));

} // ZeroMatrix3x3

/*===========================================================================*/

void CopyMatrix3x3(Matx3x3 A, Matx3x3 B)
{

memcpy(A, B, sizeof(Matx3x3));
/***
short i, j;

 for (i=0; i<3; i++)
  {
  for (j=0; j<3; j++)
   {
   A[i][j] = B[i][j];
   } // for j
  } // for i
***/

} // CopyMatrix3x3

/*===========================================================================*/

void ZeroMatrix4x4(Matx4x4 A)
{

memset(A, 0, sizeof(Matx4x4));

} // ZeroMatrix4x4

/*===========================================================================*/

void CopyMatrix4x4(Matx4x4 A, Matx4x4 B)
{

memcpy(A, B, sizeof(Matx4x4));
/***
short i, j;

 for (i=0; i<4; i++)
  {
  for (j=0; j<4; j++)
   {
   A[i][j] = B[i][j];
   } // for j
  } // for i
***/

} // CopyMatrix4x4

/*===========================================================================*/

void CopyPoint3d(Point3d To, Point3d From)
{

To[0] = From[0];
To[1] = From[1];
To[2] = From[2];

} // CopyPoint3d

/*===========================================================================*/

void CopyPoint3f(Point3f To, Point3f From)
{

To[0] = From[0];
To[1] = From[1];
To[2] = From[2];

} // CopyPoint3f

/*===========================================================================*/

void CopyPoint3f3d(Point3f To, Point3d From)
{

To[0] = (float)From[0];
To[1] = (float)From[1];
To[2] = (float)From[2];

} // CopyPoint3f3d

/*===========================================================================*/

void ZeroCoords(struct coords *A)
{

A->q = A->z = A->y = A->q = 0.0;

} // ZeroCoords

/*===========================================================================*/

void ZeroPoint3d(Point3d A)
{

memset(A, 0, sizeof(Point3d));

} // ZeroPoint3d

/*===========================================================================*/

void AddPoint3d(Point3d A, Point3d Add)
{

A[0] += Add[0];
A[1] += Add[1];
A[2] += Add[2];

} // AddPoint3d

/*===========================================================================*/

void AddPoint3f(Point3f A, Point3f Add)
{

A[0] += Add[0];
A[1] += Add[1];
A[2] += Add[2];

} // AddPoint3f

/*===========================================================================*/

void AddPoint3f3d(Point3f A, Point3d Add)
{

A[0] += (float)Add[0];
A[1] += (float)Add[1];
A[2] += (float)Add[2];

} // AddPoint3f3d

/*===========================================================================*/

void DividePoint3d(Point3d A, double Div)
{
double Invert;

if (Div > 0.0)
	{
	Invert = 1.0 / Div;
	A[0] *= Invert;
	A[1] *= Invert;
	A[2] *= Invert;
	} // if

} // DividePoint3d

/*===========================================================================*/

void MultiplyPoint3d(Point3d B, Point3d A, double Mult)
{

B[0] = A[0] * Mult;
B[1] = A[1] * Mult;
B[2] = A[2] * Mult;

} // MultiplyPoint3d

/*===========================================================================*/

void NegateVector(struct coords *A)
{

A->x = -A->x;
A->y = -A->y;
A->z = -A->z;

} // NegateVector

/*===========================================================================*/

void NegateVector(Point3d A)
{

A[0] = -A[0];
A[1] = -A[1];
A[2] = -A[2];

} // NegateVector

/*===========================================================================*/

void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C)
{
#ifdef WCS_OPTIMIZED_MATH

C[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0];
C[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1];
C[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2];
C[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0];
C[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1];
C[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2];
C[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0];
C[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1];
C[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2];

#else // WCS_OPTIMIZED_MATH

double ab;
short i, j, k;

for (i=0; i<3; i++)
	{
	for (j=0; j<3; j++)
		{
		ab = 0.0;
		for (k=0; k<3; k++)
			ab += A[i][k] * B[k][j];
		C[i][j] = ab;
		} // for
	} // for

#endif // WCS_OPTIMIZED_MATH

} // Multiply3x3Matrices

/*===========================================================================*/

void Multiply4x4Matrices(Matx4x4 A, Matx4x4 B, Matx4x4 C)
{
//#ifdef WCS_OPTIMIZED_MATH
//#else // WCS_OPTIMIZED_MATH
double ab;
short i, j, k;
//FILE *output;

//output = fopen("C:/Matrix4x4.Txt", "w");
for (i=0; i<4; i++)
	{
	for (j=0; j<4; j++)
		{
		ab = 0.0;
		for (k=0; k<4; k++)
//			{
			ab += A[i][k] * B[k][j];
//			fprintf(output, "A[%d][%d] * B[%d][%d]", i, k, k, j);
//			}
		C[i][j] = ab;
//		fprintf(output, "C[%d][%d]", i, j);
		} // for j
	} // for i
//#endif // WCS_OPTIMIZED_MATH
//printf("Foo");

} // Multiply4x4Matrices

/*===========================================================================*/

void RotatePoint(struct coords *A, Matx3x3 M)
{
struct coords B;

B.x = A->x * M[0][0] + A->y * M[0][1] + A->z * M[0][2];
B.y = A->x * M[1][0] + A->y * M[1][1] + A->z * M[1][2];
B.z = A->x * M[2][0] + A->y * M[2][1] + A->z * M[2][2];

A->x = B.x;
A->y = B.y;
A->z = B.z;
VectorMagnitude(A);

} // RotatePoint

/*===========================================================================*/

void RotatePoint(Vertex3D *A, Matx3x3 M)
{
Vertex3D B;

B.XYZ[0] = A->XYZ[0] * M[0][0] + A->XYZ[1] * M[0][1] + A->XYZ[2] * M[0][2];
B.XYZ[1] = A->XYZ[0] * M[1][0] + A->XYZ[1] * M[1][1] + A->XYZ[2] * M[1][2];
B.XYZ[2] = A->XYZ[0] * M[2][0] + A->XYZ[1] * M[2][1] + A->XYZ[2] * M[2][2];

A->XYZ[0] = B.XYZ[0];
A->XYZ[1] = B.XYZ[1];
A->XYZ[2] = B.XYZ[2];
VectorMagnitude(A);

} // RotatePoint

/*===========================================================================*/

void RotatePoint(double XYZ[3], Matx3x3 M)
{
double Temp[3];

Temp[0] = XYZ[0] * M[0][0] + XYZ[1] * M[0][1] + XYZ[2] * M[0][2];
Temp[1] = XYZ[0] * M[1][0] + XYZ[1] * M[1][1] + XYZ[2] * M[1][2];
Temp[2] = XYZ[0] * M[2][0] + XYZ[1] * M[2][1] + XYZ[2] * M[2][2];

XYZ[0] = Temp[0];
XYZ[1] = Temp[1];
XYZ[2] = Temp[2];

} // RotatePoint

/*===========================================================================*/

void RotatePoint(double &X, double &Y, double &Z, Matx3x3 M)
{
struct coords B;

B.x = X * M[0][0] + Y * M[0][1] + Z * M[0][2];
B.y = X * M[1][0] + Y * M[1][1] + Z * M[1][2];
B.z = X * M[2][0] + Y * M[2][1] + Z * M[2][2];

X = B.x;
Y = B.y;
Z = B.z;

} // RotatePoint

/*===========================================================================*/

void RotateX(Point3d XYZ, double Angle)
{
double NewY, NewZ, SinAngle, CosAngle;

Angle *= PiOver180;
sincos(Angle, &SinAngle, &CosAngle);
NewZ = XYZ[2] * CosAngle + XYZ[1] * SinAngle;
NewY = XYZ[1] * CosAngle - XYZ[2] * SinAngle;

XYZ[1] = NewY;
XYZ[2] = NewZ;

} // RotateX

/*===========================================================================*/

void RotateY(Point3d XYZ, double Angle)
{
double NewX, NewZ, SinAngle, CosAngle;

Angle *= PiOver180;
sincos(Angle, &SinAngle, &CosAngle);
NewX = XYZ[0] * CosAngle + XYZ[2] * SinAngle;
NewZ = XYZ[2] * CosAngle - XYZ[0] * SinAngle;

XYZ[0] = NewX;
XYZ[2] = NewZ;

} // RotateY

/*===========================================================================*/

void RotateZ(Point3d XYZ, double Angle)
{
double NewX, NewY, SinAngle, CosAngle;

Angle *= PiOver180;
sincos(Angle, &SinAngle, &CosAngle);
NewX = XYZ[0] * CosAngle + XYZ[1] * SinAngle;
NewY = XYZ[1] * CosAngle - XYZ[0] * SinAngle;

XYZ[0] = NewX;
XYZ[1] = NewY;

} // RotateZ

/*===========================================================================*/

void Transform3DPoint(Vertex3D *A, Matx4x4 M)
{
Vertex3D B;

B.XYZ[0] = A->XYZ[0] * M[0][0] + A->XYZ[1] * M[0][1] + A->XYZ[2] * M[0][2] + M[0][3];
B.XYZ[1] = A->XYZ[0] * M[1][0] + A->XYZ[1] * M[1][1] + A->XYZ[2] * M[1][2] + M[1][3];
B.XYZ[2] = A->XYZ[0] * M[2][0] + A->XYZ[1] * M[2][1] + A->XYZ[2] * M[2][2] + M[2][3];

A->XYZ[0] = B.XYZ[0];
A->XYZ[1] = B.XYZ[1];
A->XYZ[2] = B.XYZ[2];

} // Transform3DPoint

/*===========================================================================*/

// angles specified in degrees
void EulerToAxisAngle(Point3d EulerAngles, Point4d AxisAngle)
{
double Heading, Pitch, Bank, c1, c2, c3, s1, s2, s3;

Heading = EulerAngles[1] * PiOver180;
Pitch = EulerAngles[0] * PiOver180;
Bank = EulerAngles[2] * PiOver180;
sincos(Heading * 0.5, &c1, &s1);
sincos(Pitch * 0.5, &c2, &s2);
sincos(Bank * 0.5, &c3, &s3);
//AxisAngle[0] = c1 * c2 * s3 - s1 * s2 * c3;
//AxisAngle[1] = c1 * s2 * c3 + s1 * c2 * s3;
//AxisAngle[2] = s1 * c2 * c3 - c1 * s2 * s3;
AxisAngle[0] = c1 * s2 * c3 + s1 * c2 * s3;
AxisAngle[1] = s1 * c2 * c3 - c1 * s2 * s3;
AxisAngle[2] = c1 * c2 * s3 - s1 * s2 * c3;
AxisAngle[3] = c1 * c2 * c3 + s1 * s2 * s3;
AxisAngle[3] = 2.0 * PiUnder180 * acos(AxisAngle[3]);

UnitVector(AxisAngle);

} // EulerToAxisAngle

/*===========================================================================*/

void EulerToQuaternion(Point3d EulerAngles, Point4d Quat)
{
double Heading, Pitch, Bank, c1, c2, c3, s1, s2, s3;

Heading = EulerAngles[1] * PiOver180;
Pitch = EulerAngles[0] * PiOver180;
Bank = EulerAngles[2] * PiOver180;
sincos(Heading * 0.5, &c1, &s1);
sincos(Pitch * 0.5, &c2, &s2);
sincos(Bank * 0.5, &c3, &s3);

Quat[3] = c1 * c2 * c3 + s1 * s2 * s3;
//Quat[0] = c1 * c2 * s3 - s1 * s2 * c3;
//Quat[1] = c1 * s2 * c3 + s1 * c2 * s3;
//Quat[2] = s1 * c2 * c3 - c1 * s2 * s3;
// these are the equations that work in our axis system
Quat[0] = c1 * s2 * c3 + s1 * c2 * s3;
Quat[1] = s1 * c2 * c3 - c1 * s2 * s3;
Quat[2] = c1 * c2 * s3 - s1 * s2 * c3;

} // EulerToQuaternion

/*===========================================================================*/

void AxisAngleToQuaternion(Point4d AxisAngle, Point4d Quat)
{
double angle, c, s;

angle = AxisAngle[3] * PiOver180;
sincos(angle * 0.5, &s, &c);

Quat[0] = AxisAngle[0] * s;
Quat[1] = AxisAngle[1] * s;
Quat[2] = AxisAngle[2] * s;
Quat[3] = c;

} // AxisAngleToQuaternion

/*===========================================================================*/

void QuaternionToAxisAngle(Point4d AxisAngle, Point4d Quat)
{
double s;

AxisAngle[3] = (2.0 * acos(Quat[3])) * PiUnder180;
s = sqrt(1.0 - Quat[3] * Quat[3]);

if (fabs(s) < 0.000001)
	s = 1.0;
AxisAngle[0] = Quat[0] / s;
AxisAngle[1] = Quat[1] / s;
AxisAngle[2] = Quat[2] / s;

} // QuaternionToAxisAngle

/*===========================================================================*/

void MultiplyQuaternions(Point4d QuatOut, Point4d Q1, Point4d Q2)
{

QuatOut[3] = Q1[3] * Q2[3] - Q1[0] * Q2[0] - Q1[1] * Q2[1] - Q1[2] * Q2[2];
QuatOut[0] = (Q1[0] * Q2[3] + Q1[3] * Q2[0] + Q1[1] * Q2[2] - Q1[2] * Q2[1]);
QuatOut[1] = (Q1[3] * Q2[1] - Q1[0] * Q2[2] + Q1[1] * Q2[3] + Q1[2] * Q2[0]);
QuatOut[2] = (Q1[3] * Q2[2] + Q1[0] * Q2[1] - Q1[1] * Q2[0] + Q1[2] * Q2[3]);

} // MultiplyQuaternions

/*===========================================================================*/

//    180
//     +a
// 90__|__+b 270
//     |
//     0
double findangle(double pta, double ptb)
{
double angle;

if (pta == 0.0)
	{
	angle = ptb > 0.0 ? OneAndHalfPi: HalfPi;
	} // if
else
	{
	angle = atan(ptb / pta);
	if (pta > 0.0)
		angle += Pi;
	else if (ptb > 0.0)
		angle += TwoPi;
	} // else

return(angle);

} // findangle

/*===========================================================================*/
//      90
//      +a
// 180__|__+b 0
//      |
//     270
double findangle2(double pta, double ptb)
{
double angle;

if (ptb == 0.0)
	{
	angle = pta > 0.0 ? HalfPi: OneAndHalfPi;
	} // if
else
	{
	angle = atan(pta / ptb);
	if (ptb < 0.0)
		angle += Pi;
	} // else

return angle;

} // findangle2

/*===========================================================================*/
//      0
//      +a
// 270__|__+b 90
//      |
//     180
double findangle3(double pta, double ptb)
{
double angle;

if (pta == 0.0)
	{
	angle = ptb > 0.0 ? HalfPi: OneAndHalfPi;
	} // if
else
	{
	angle = atan(ptb / pta);
	if (pta < 0.0)
		angle += Pi;
	} // else

return angle;

} // findangle3

/*===========================================================================*/

void RotatePoint(Point2d XY, double Angle)
{
double NewX, NewY, SinAngle, CosAngle;

Angle *= PiOver180;
sincos(Angle, &SinAngle, &CosAngle);
NewX = XY[0] * CosAngle + XY[1] * SinAngle;
NewY = XY[1] * CosAngle - XY[0] * SinAngle;

XY[0] = NewX;
XY[1] = NewY;

} // rotate

/*===========================================================================*/

void rotate(double *pta, double *ptb, double rotangle, double angle)
{
double length, newangle, sine, cosine;

newangle = angle - rotangle;
sincos(angle, &sine, &cosine);
length = sqrt((*pta) * (*pta) + (*ptb) * (*ptb));
*pta = length * sine;
*ptb = length * cosine;

} // rotate

/*===========================================================================*/

void rotate2(double *pta, double *ptb, double rotangle, double angle)
{
double length, newangle, sine, cosine;

newangle = angle - rotangle;
sincos(angle, &sine, &cosine);
length = sqrt((*pta) * (*pta) + (*ptb) * (*ptb));
*pta = length * cosine;
*ptb = length * sine;

} // rotate2

/*===========================================================================*/

// VecDirection must already be unitized.
// Returns 0 if vector misses sphere but still attempts to find the point on the line closest to the sphere.
// Otherwise returns the number of positive roots, which are the intersection points in the direction indicated by
// DirectionVec. The smallest positive root's distance and intersection point is placed in Dist1 and Intersection1,
// the larger in Dist2, Intersection2.

int RaySphereIntersect(double Radius, Point3d VecOrigin, Point3d VecDirection, Point3d SphereCenter,
	Point3d Intersection1, double &Dist1, Point3d Intersection2, double &Dist2)
{
double Rsq, B, C, C4, VmS[3], Determinant, T1, T2;

VmS[0] = VecOrigin[0] - SphereCenter[0];
VmS[1] = VecOrigin[1] - SphereCenter[1];
VmS[2] = VecOrigin[2] - SphereCenter[2];

Rsq = Radius * Radius;	// this is the radius of the star sphere squared
C = (VmS[0] * VmS[0]) + (VmS[1] * VmS[1]) + (VmS[2] * VmS[2]) - Rsq;
C4 = C * 4.0;

B = 2.0 * ((VecDirection[0] * VmS[0]) + (VecDirection[1] * VmS[1]) + (VecDirection[2] * VmS[2]));

Determinant = B * B - C4;

if (Determinant < 0.0)
	{
	// find nearest to tangent point
	Dist1 = -B * .5;
	Intersection1[0] = VecOrigin[0] + Dist1 * VecDirection[0];
	Intersection1[1] = VecOrigin[1] + Dist1 * VecDirection[1];
	Intersection1[2] = VecOrigin[2] + Dist1 * VecDirection[2];
	return (0);
	} // if

// general quadratic equation: y = (-B +/- sqrt(B * B - 4 * A * C)) / (2 * A)
// A = 1 because direction vector is unitized
Determinant = sqrt(Determinant);
T1 = (-B + Determinant) * .5;
T2 = (-B - Determinant) * .5;

if (T1 > 0.0 && T2 > 0.0)
	{
	Dist1 = min(T1, T2);	// closest point to vector origin
	Dist2 = max(T1, T2);	// farthest point from vector origin
	Intersection1[0] = VecOrigin[0] + Dist1 * VecDirection[0];
	Intersection1[1] = VecOrigin[1] + Dist1 * VecDirection[1];
	Intersection1[2] = VecOrigin[2] + Dist1 * VecDirection[2];
	Intersection2[0] = VecOrigin[0] + Dist2 * VecDirection[0];
	Intersection2[1] = VecOrigin[1] + Dist2 * VecDirection[1];
	Intersection2[2] = VecOrigin[2] + Dist2 * VecDirection[2];
	} // if
else
	{
	Dist1 = T1;		// positive root
	Intersection1[0] = VecOrigin[0] + Dist1 * VecDirection[0];
	Intersection1[1] = VecOrigin[1] + Dist1 * VecDirection[1];
	Intersection1[2] = VecOrigin[2] + Dist1 * VecDirection[2];
	} // else

return ((T1 > 0.0) + (T2 > 0.0));

} // RaySphereIntersect

/*===========================================================================*/

// VecDirection must already be unitized.
// Returns 0 if vector misses sphere but still attempts to find the point on the line closest to the sphere.
// Otherwise returns the number of positive roots, which are the intersection points in the direction indicated by
// DirectionVec. The smallest positive root's distance and intersection point is placed in dist and Intersection.

int RaySphereIntersect(double Radius, Point3d VecOrigin, Point3d VecDirection, Point3d SphereCenter,
	Point3d Intersection, double &Dist)
{
double Rsq, B, C, C4, VmS[3], Determinant, T1, T2;

VmS[0] = VecOrigin[0] - SphereCenter[0];
VmS[1] = VecOrigin[1] - SphereCenter[1];
VmS[2] = VecOrigin[2] - SphereCenter[2];

Rsq = Radius * Radius;	// this is the radius of the star sphere squared
C = (VmS[0] * VmS[0]) + (VmS[1] * VmS[1]) + (VmS[2] * VmS[2]) - Rsq;
C4 = C * 4.0;

B = 2.0 * ((VecDirection[0] * VmS[0]) + (VecDirection[1] * VmS[1]) + (VecDirection[2] * VmS[2]));

Determinant = B * B - C4;

if (Determinant < 0.0)
	{
	// find nearest to tangent point
	Dist = -B * .5;
	Intersection[0] = VecOrigin[0] + Dist * VecDirection[0];
	Intersection[1] = VecOrigin[1] + Dist * VecDirection[1];
	Intersection[2] = VecOrigin[2] + Dist * VecDirection[2];
	return (0);
	} // if

// general quadratic equation: y = (-B +/- sqrt(B * B - 4 * A * C)) / (2 * A)
// A = 1 because direction vector is unitized
Determinant = sqrt(Determinant);
T1 = (-B + Determinant) * .5;
T2 = (-B - Determinant) * .5;

if (T1 > 0.0 && T2 > 0.0)
	{
	Dist = min(T1, T2);	// closest point to vector origin
	} // if
else
	Dist = T1;		// positive root

Intersection[0] = VecOrigin[0] + Dist * VecDirection[0];
Intersection[1] = VecOrigin[1] + Dist * VecDirection[1];
Intersection[2] = VecOrigin[2] + Dist * VecDirection[2];

return ((T1 > 0.0) + (T2 > 0.0));

} // RaySphereIntersect

/*===========================================================================*/

// calculate the intersection between a ray and a four-sided coplanar feature
// return 1 if intersection is within quadrilateral
// all values may be modified so don't pass anything that can't be changed
// VecDirection and QuadNormal must be unit vectors
int RayQuadrilateralIntersect(Point3d VecOrigin, Point3d VecDirection, Point3d QuadNormal, double QuadVert[4][3],
	Point3d Intersection, double &Dist)
{
Point3d Origin;
double Denom, Num, MaxCoord[3], MinCoord[3];
int InQuad = 0, NumCrossings = 0, IgnoreAxis, i;

Origin[0] = QuadVert[0][0];
Origin[1] = QuadVert[0][1];
Origin[2] = QuadVert[0][2];

// move everything to origin
VecOrigin[0] -= Origin[0];
VecOrigin[1] -= Origin[1];
VecOrigin[2] -= Origin[2];

QuadVert[1][0] -= Origin[0];
QuadVert[1][1] -= Origin[1];
QuadVert[1][2] -= Origin[2];

QuadVert[2][0] -= Origin[0];
QuadVert[2][1] -= Origin[1];
QuadVert[2][2] -= Origin[2];

QuadVert[3][0] -= Origin[0];
QuadVert[3][1] -= Origin[1];
QuadVert[3][2] -= Origin[2];

QuadVert[0][0] = QuadVert[0][1] = QuadVert[0][2] = 0.0;

// find distance to plane from vector origin
// Denom = ax1 + by1 + cz1
Denom = QuadNormal[0] * VecDirection[0] + QuadNormal[1] * VecDirection[1] + QuadNormal[2] * VecDirection[2];
if (Denom != 0.0)
	{
	Num = QuadNormal[0] * VecOrigin[0] + QuadNormal[1] * VecOrigin[1] + QuadNormal[2] * VecOrigin[2];
	Dist = -Num / Denom;

	// solve for intersection
	Intersection[0] = Dist * VecDirection[0] + VecOrigin[0];
	Intersection[1] = Dist * VecDirection[1] + VecOrigin[1];
	Intersection[2] = Dist * VecDirection[2] + VecOrigin[2];

	// ignore the axis with the smallest range of values
	MaxCoord[0] = MaxCoord[1] = MaxCoord[2] = -FLT_MAX;
	MinCoord[0] = MinCoord[1] = MinCoord[2] = FLT_MAX;
	for (i = 0; i < 4; i ++)
		{
		if (QuadVert[i][0] > MaxCoord[0])
			MaxCoord[0] = QuadVert[i][0];
		if (QuadVert[i][0] < MinCoord[0])
			MinCoord[0] = QuadVert[i][0];

		if (QuadVert[i][1] > MaxCoord[1])
			MaxCoord[1] = QuadVert[i][1];
		if (QuadVert[i][1] < MinCoord[1])
			MinCoord[1] = QuadVert[i][1];

		if (QuadVert[i][2] > MaxCoord[2])
			MaxCoord[2] = QuadVert[i][2];
		if (QuadVert[i][2] < MinCoord[2])
			MinCoord[2] = QuadVert[i][2];
		} // for
	if (MaxCoord[0] - MinCoord[0] > MaxCoord[1] - MinCoord[1])
		{
		IgnoreAxis = 1;
		if (MaxCoord[1] - MinCoord[1] > MaxCoord[2] - MinCoord[2])
			{
			IgnoreAxis = 2;
			} // if
		} // if
	else
		{
		IgnoreAxis = 0;
		if (MaxCoord[0] - MinCoord[0] > MaxCoord[2] - MinCoord[2])
			{
			IgnoreAxis = 2;
			} // if
		} // else
	// test intersection to see if it is in quad
	if (IgnoreAxis == 0)
		{
		NumCrossings += LineSegsIntersect(QuadVert[0][1], QuadVert[0][2], QuadVert[1][1], QuadVert[1][2], Intersection[1], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[1][1], QuadVert[1][2], QuadVert[2][1], QuadVert[2][2], Intersection[1], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[2][1], QuadVert[2][2], QuadVert[3][1], QuadVert[3][2], Intersection[1], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[3][1], QuadVert[3][2], QuadVert[0][1], QuadVert[0][2], Intersection[1], Intersection[2]);
		} // if
	else if (IgnoreAxis == 1)
		{
		NumCrossings += LineSegsIntersect(QuadVert[0][0], QuadVert[0][2], QuadVert[1][0], QuadVert[1][2], Intersection[0], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[1][0], QuadVert[1][2], QuadVert[2][0], QuadVert[2][2], Intersection[0], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[2][0], QuadVert[2][2], QuadVert[3][0], QuadVert[3][2], Intersection[0], Intersection[2]);
		NumCrossings += LineSegsIntersect(QuadVert[3][0], QuadVert[3][2], QuadVert[0][0], QuadVert[0][2], Intersection[0], Intersection[2]);
		} // if
	else
		{
		NumCrossings += LineSegsIntersect(QuadVert[0][0], QuadVert[0][1], QuadVert[1][0], QuadVert[1][1], Intersection[0], Intersection[1]);
		NumCrossings += LineSegsIntersect(QuadVert[1][0], QuadVert[1][1], QuadVert[2][0], QuadVert[2][1], Intersection[0], Intersection[1]);
		NumCrossings += LineSegsIntersect(QuadVert[2][0], QuadVert[2][1], QuadVert[3][0], QuadVert[3][1], Intersection[0], Intersection[1]);
		NumCrossings += LineSegsIntersect(QuadVert[3][0], QuadVert[3][1], QuadVert[0][0], QuadVert[0][1], Intersection[0], Intersection[1]);
		} // if

	// restore intersection to original coord space
	Intersection[0] += Origin[0];
	Intersection[1] += Origin[1];
	Intersection[2] += Origin[2];

	InQuad = NumCrossings % 2;
	} // if

return (InQuad);

} // RayQuadrilateralIntersect

/*===========================================================================*/

// This function computes the distance from a line extended to infinity and a point

double DistPoint2Line(Point2d P, Point2d L1, Point2d L2)
{
double A, C, Dx, Dy;

if (L2[0] != L1[0])
	{
	A = (L2[1] - L1[1]) / (L2[0] - L1[0]);
	C = L1[1] - A * L1[0];
//	B = -1.0;
//	return(fabs(A * P[0] + B * P[1] + C) / sqrt(A * A + B * B));
	return(fabs(A * P[0] - P[1] + C) / sqrt(A * A + 1.0));
	} // if
else if (L2[1] != L1[1])
	{
//	A = -1.0;
//	B = 0.0;
	C = L1[0];
//	return(fabs(A * P[0] + B * P[1] + C) / sqrt(A * A + B * B));
	return(fabs(- P[0] + C));
	} // else

Dx = P[0] - L1[0];
Dy = P[1] - L1[1];
return (sqrt(Dx * Dx + Dy * Dy));

} // DistPoint2Line

/*===========================================================================*/

double DistPoint2Point(Point2d P1, Point2d P2)
{
double Dx, Dy;

Dx = P1[0] - P2[0];
Dy = P1[1] - P2[1];
return (sqrt(Dx * Dx + Dy * Dy));

} // DistPoint2Point

/*===========================================================================*/

// This function does the same as the above except that if the point is not contained within the
// line segment the return value is the distance to the closest end point of the segment.

double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2, char *UsePoint, double &P1Offset, double &SegLength)
{
double A1, C1, A2, C2, Dist1, Dist2, X1, X2, Y1, Y2, XDif, YDif;
Point2d Px, Min, Max;

if (L1[0] > L2[0])
	{
	Min[0] = L2[0];
	Max[0] = L1[0];
	} // if
else
	{
	Min[0] = L1[0];
	Max[0] = L2[0];
	} // else
if (L1[1] > L2[1])
	{
	Min[1] = L2[1];
	Max[1] = L1[1];
	} // if
else
	{
	Min[1] = L1[1];
	Max[1] = L2[1];
	} // else


if (L2[0] != L1[0])		// x not equal
	{
	XDif = L2[0] - L1[0];
	A1 = (L2[1] - L1[1]) / (XDif);
	C1 = L1[1] - A1 * L1[0];
//	B = -1.0;

//	point-slope form of line is y = A1x + C1

	if (A1 != 0.0)
		{
		A2 = - 1.0 / A1;	//	orthogonal line has negative inverse slope
		C2 = P[1] - A2 * P[0];
		Px[0] = (C2 - C1) / (A1 - A2);		// no need to worry about divide by zero
		Px[1] = A1 * Px[0] + C1;
		YDif = L2[1] - L1[1];
		SegLength = sqrt(XDif * XDif + YDif * YDif);
		} // if
	else
		{
		Px[0] = P[0];
		Px[1] = C1;
		SegLength = fabs(XDif);
		} // else

	if (Px[0] >= Min[0] && Px[0] <= Max[0] && Px[1] >= Min[1] && Px[1] <= Max[1])
		{
		UsePoint[0] = UsePoint[1] = 1;
		P1Offset = (Px[0] - L1[0]) / XDif;
		return(fabs(A1 * P[0] - P[1] + C1) / sqrt(A1 * A1 + 1.0));
//		return(fabs(A1 * P[0] + B * P[1] + C1) / sqrt(A1 * A1 + B * B));
		} // if contained
//	else find distance to closest end point of line segment
	X1 = P[0] - L1[0];
	X2 = P[0] - L2[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist1 = X1 * X1 + Y1 * Y1;
	Dist2 = X2 * X2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		UsePoint[0] = 1;
		UsePoint[1] = 0;
		P1Offset = 0.0;
		return (sqrt(Dist1));
		} // if
	P1Offset = 1.0;
	UsePoint[1] = 1;
	UsePoint[0] = 0;
	return (sqrt(Dist2));
	} // if not vertical line
else							// x equal - vertical line or point
	{
//	A1 = -1.0;
//	B = 0.0;
	C1 = L1[0];

//	point-slope form of line is x = C1 vertical line

	Px[0] = C1;		//	orthogonal is horizontal line
	Px[1] = P[1];
	SegLength = Max[1] - Min[1];

	if (Px[1] >= Min[1] && Px[1] <= Max[1])
		{
//		return(fabs(A1 * P[0] + B * P[1] + C1) / sqrt(A1 * A1 + B * B));
		if (L1[1] != L2[1])
			{
			UsePoint[0] = UsePoint[1] = 1;
			P1Offset = (Px[1] - L1[1]) / (L2[1] - L1[1]);
			} // if
		else
			{
			UsePoint[0] = 1;
			UsePoint[1] = 0;
			P1Offset = 0.0;
			} // else the segment is a degenerate line
		return(fabs(- P[0] + C1));
		} // if contained
	X1 = P[0] - L1[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist2 = X1 * X1;		// temp storage
	Dist1 = Dist2 + Y1 * Y1;
	Dist2 = Dist2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		UsePoint[0] = 1;
		UsePoint[1] = 0;
		P1Offset = 0.0;
		return (sqrt(Dist1));
		} // if
	UsePoint[1] = 1;
	UsePoint[0] = 0;
	P1Offset = 1.0;
	return (sqrt(Dist2));
	} // else vertical line

} // DistPoint2LineContained

/*===========================================================================*/

double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2)
{
double A1, C1, A2, C2, Dist1, Dist2, X1, X2, Y1, Y2;
Point2d Px, Min, Max;

if (L1[0] > L2[0])
	{
	Min[0] = L2[0];
	Max[0] = L1[0];
	} // if
else
	{
	Min[0] = L1[0];
	Max[0] = L2[0];
	} // else
if (L1[1] > L2[1])
	{
	Min[1] = L2[1];
	Max[1] = L1[1];
	} // if
else
	{
	Min[1] = L1[1];
	Max[1] = L2[1];
	} // else


if (L2[0] != L1[0])		// x not equal
	{
	A1 = (L2[1] - L1[1]) / (L2[0] - L1[0]);
	C1 = L1[1] - A1 * L1[0];
//	B = -1.0;

//	point-slope form of line is y = A1x + C1

	if (A1 != 0.0)
		{
		A2 = - 1.0 / A1;	//	orthogonal line has negative inverse slope
		C2 = P[1] - A2 * P[0];
		Px[0] = (C2 - C1) / (A1 - A2);	// no need to worry about divide by zero
		Px[1] = A1 * Px[0] + C1;
		} // if
	else
		{
		Px[0] = P[0];
		Px[1] = C1;
		} // else

	if (Px[0] >= Min[0] && Px[0] <= Max[0] && Px[1] >= Min[1] && Px[1] <= Max[1])
		{
		return(fabs(A1 * P[0] - P[1] + C1) / sqrt(A1 * A1 + 1.0));
		} // if contained
//	else find distance to closest end point of line segment
	X1 = P[0] - L1[0];
	X2 = P[0] - L2[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist1 = X1 * X1 + Y1 * Y1;
	Dist2 = X2 * X2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		return (sqrt(Dist1));
		} // if
	return (sqrt(Dist2));
	} // if not vertical line
else	// x equal - vertical line or point
	{
//	A1 = -1.0;
//	B = 0.0;
	C1 = L1[0];

//	point-slope form of line is x = C1 vertical line

	Px[0] = C1;		//	orthogonal is horizontal line
	Px[1] = P[1];

	if (Px[1] >= Min[1] && Px[1] <= Max[1])
		{
		return(fabs(- P[0] + C1));
		} // if contained
	X1 = P[0] - L1[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist2 = X1 * X1;		// temp storage
	Dist1 = Dist2 + Y1 * Y1;
	Dist2 = Dist2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		return (sqrt(Dist1));
		} // if
	return (sqrt(Dist2));
	} // else vertical line

} // DistPoint2LineContained

/*===========================================================================*/

// fills in the coordinates of the closest pointon the line
double DistPoint2LineContained(Point2d P, Point2d L1, Point2d L2, Point2d Px)
{
double A1, C1, A2, C2, Dist1, Dist2, X1, X2, Y1, Y2;
Point2d Min, Max;

if (L1[0] > L2[0])
	{
	Min[0] = L2[0];
	Max[0] = L1[0];
	} // if
else
	{
	Min[0] = L1[0];
	Max[0] = L2[0];
	} // else
if (L1[1] > L2[1])
	{
	Min[1] = L2[1];
	Max[1] = L1[1];
	} // if
else
	{
	Min[1] = L1[1];
	Max[1] = L2[1];
	} // else


if (L2[0] != L1[0])		// x not equal
	{
	A1 = (L2[1] - L1[1]) / (L2[0] - L1[0]);
	C1 = L1[1] - A1 * L1[0];
//	B = -1.0;

//	point-slope form of line is y = A1x + C1

	if (A1 != 0.0)
		{
		A2 = - 1.0 / A1;	//	orthogonal line has negative inverse slope
		C2 = P[1] - A2 * P[0];
		Px[0] = (C2 - C1) / (A1 - A2);		// no need to worry about divide by zero
		Px[1] = A1 * Px[0] + C1;
		} // if
	else
		{
		Px[0] = P[0];
		Px[1] = C1;
		} // else

	if (Px[0] >= Min[0] && Px[0] <= Max[0] && Px[1] >= Min[1] && Px[1] <= Max[1])
		{
		return(fabs(A1 * P[0] - P[1] + C1) / sqrt(A1 * A1 + 1.0));
		} // if contained
//	else find distance to closest end point of line segment
	X1 = P[0] - L1[0];
	X2 = P[0] - L2[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist1 = X1 * X1 + Y1 * Y1;
	Dist2 = X2 * X2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		Px[0] = L1[0];
		Px[1] = L1[1];
		return (sqrt(Dist1));
		} // if
	Px[0] = L2[0];
	Px[1] = L2[1];
	return (sqrt(Dist2));
	} // if not vertical line
else	// x equal - vertical line or point
	{
//	A1 = -1.0;
//	B = 0.0;
	C1 = L1[0];

//	point-slope form of line is x = C1 vertical line

	Px[0] = C1;		//	orthogonal is horizontal line
	Px[1] = P[1];

	if (Px[1] >= Min[1] && Px[1] <= Max[1])
		{
		return(fabs(- P[0] + C1));
		} // if contained
	X1 = P[0] - L1[0];
	Y1 = P[1] - L1[1];
	Y2 = P[1] - L2[1];
	Dist2 = X1 * X1;		// temp storage
	Dist1 = Dist2 + Y1 * Y1;
	Dist2 = Dist2 + Y2 * Y2;
	if (Dist1 <= Dist2)
		{
		Px[0] = L1[0];
		Px[1] = L1[1];
		return (sqrt(Dist1));
		} // if
	Px[0] = L2[0];
	Px[1] = L2[1];
	return (sqrt(Dist2));
	} // else vertical line

} // DistPoint2LineContained

/*===========================================================================*/

// this takes a point and extrapolates a line to infinity in one direction (west = left)
// then determines if it intersects a line segment.
// The function is used on each segment of a polygon to test whether the point is contained.
// note that we only consider the equality of one end of the line but that has to be
// a consistent end based on the input order.
// The one place it will give different results depending on whether the vector is clockwise or countercw
// is if the point in consideration exactly lies on one of the vertices. If all objects are either
// clockwise or counterclockwise then it won't really be a problem.

short LineSegsIntersect(double Lat, double Lon, VectorPoint *P1, VectorPoint *P2)
{
/* bogus
if (P1->Longitude < Lon && P2->Longitude <= Lon)
	return (0);

if (P1->Latitude == Lat)
	return (1);

if ((P1->Latitude > Lat && P2->Latitude >= Lat) || (P1->Latitude < Lat && P2->Latitude <= Lat))
	return (0);

if (P1->Longitude >= Lon && P2->Longitude >= Lon)
	return (1);

if (((P2->Latitude - P1->Latitude) / (P2->Longitude - P1->Longitude)) * (Lon - P1->Longitude) + P1->Latitude >= Lat)
	return (1);
*/
// try again
// both points right
if (P1->Longitude < Lon && P2->Longitude <= Lon)
	return (0);
// at least one point to left or pt1 equal

// both points above or below
if ((P1->Latitude > Lat && P2->Latitude >= Lat) || (P1->Latitude < Lat && P2->Latitude <= Lat))
	return (0);
// at least one point above, one below or pt1 equal

// pt1 on the projected line
if (P1->Latitude == Lat && P1->Longitude >= Lon)
	return (1);
// any points left of pt are either above or below

// both points to left or equal and one above, one below
if (P1->Longitude >= Lon && P2->Longitude >= Lon)
	return (1);
// not a single point line equal to point
// not a vertical line

// horizontal line with pt1 right of pt
if (P1->Latitude == Lat)
	return (0);
// not horizontal line

// test to see if y intersection would be left or right or equal to pt
if (((P2->Longitude - P1->Longitude) / (P2->Latitude - P1->Latitude)) * (Lat - P1->Latitude) + P1->Longitude >= Lon)
	return (1);

return (0);

} // LineSegsIntersect

/*===========================================================================*/

short LineSegsIntersect(double Lat1, double Lon1, double Lat2, double Lon2, double LatPt, double LonPt)
{
/*
// <<<>>> these rules may be bogus
if (Lon1 < LonPt && Lon2 <= LonPt)//
	return (0);

if (Lat1 == LatPt)
	return (1);

if ((Lat1 > LatPt && Lat2 >= LatPt) || (Lat1 < LatPt && Lat2 <= LatPt))//
	return (0);

if (Lon1 >= LonPt && Lon2 >= LonPt)//
	return (1);

if (((Lat2 - Lat1) / (Lon2 - Lon1)) * (LonPt - Lon1) + Lat1 >= LatPt)
	return (1);
*/
// try again
// both points right
if (Lon1 < LonPt && Lon2 <= LonPt)
	return (0);
// at least one point to left or pt1 equal

// both points above or below
if ((Lat1 > LatPt && Lat2 >= LatPt) || (Lat1 < LatPt && Lat2 <= LatPt))
	return (0);
// at least one point above, one below or pt1 equal

// pt1 on the projected line
if (Lat1 == LatPt && Lon1 >= LonPt)
	return (1);
// any points left of pt are either above or below

// both points to left or equal and one above, one below
if (Lon1 >= LonPt && Lon2 >= LonPt)
	return (1);
// not a single point line equal to point
// not a vertical line

// horizontal line with pt1 right of pt
if (Lat1 == LatPt)
	return (0);
// not horizontal line

// test to see if y intersection would be left or right or equal to pt
if (((Lon2 - Lon1) / (Lat2 - Lat1)) * (LatPt - Lat1) + Lon1 >= LonPt)
	return (1);

return (0);

} // LineSegsIntersect

/*===========================================================================*/

// this function returns an error if the two points are the same
int CalcGeneralLinearEquation(Point2d Pt1, Point2d Pt2, struct GeneralLinearEquation *Eq)
{
double m, b;

// given two points Pt1 and Pt2 in which [0] is x and [1] is y

// General equation for a line is Ax + By + C = 0

// if all values are non-zero
/* this seems to be more complex and less general than below
if (Pt1[0] != 0.0 && Pt2[0] != 0.0 && Pt1[1] != 0.0 && Pt2[1] != 0.0 && Pt1[0] / Pt1[1] != Pt2[0] / Pt2[1])
	{
	// solve for A in terms of C
	// A = C * (1 / y1 - 1 / y0) / (x0 / y0 - x1 / y1)
	Eq->CoefA = (1 / Pt2[1] - 1 / Pt1[1]) / (Pt1[0] / Pt1[1] - Pt2[0] / Pt2[1]);

	// then for B in terms of C
	// B = C * (1 / x1 - 1 / x0) / (y0 / x0 - y1 / x1)
	Eq->CoefB = (1 / Pt2[0] - 1 / Pt1[0]) / (Pt1[1] / Pt1[0] - Pt2[1] / Pt2[0]);

	// substitute and divide by C
	// Ax + By + C = 0
	// Eq->CoefA * C * x + Eq->CoefB * C * y + C = 0
	// Eq->CoefA * x + Eq->CoefB * y + 1 = 0

	Eq->CoefC = 1.0;
	return (1);
	} // if
else
*/
	{
	if (Pt1[0] != Pt2[0])
		{
		// y = mx + b
		// mx - y + b = 0
		// b = y - mx
		// A = m; B = -1; C = b
		m = (Pt2[1] - Pt1[1]) / (Pt2[0] - Pt1[0]);
		b = Pt2[1] - m * Pt2[0];
		Eq->CoefA = m;
		Eq->CoefB = -1.0;
		Eq->CoefC = b;
		return (1);
		} // if
	else if (Pt1[1] != Pt2[1])
		{
		// x = my + b
		// my - x + b = 0
		// b = x - my
		// A = -1; B = m; C = b
		m = (Pt2[0] - Pt1[0]) / (Pt2[1] - Pt1[1]);
		b = Pt2[0] - m * Pt2[1];
		Eq->CoefA = -1.0;
		Eq->CoefB = m;
		Eq->CoefC = b;
		return (1);
		} // if
	} // else

// both points are the same
Eq->CoefA = Eq->CoefB = 0.0;
Eq->CoefC = 1.0;
return (0);

} // CalcGeneralLinearEquation

/*===========================================================================*/

int CalcParallelLinearEquation(Point2d Pt1, struct GeneralLinearEquation *Input, struct GeneralLinearEquation *Output)
{
double m;
Point2d Pt2;

// Ax + By + C = 0
// y = mx + b
// mx - y + b = 0
// scale coefficients so that B becomes -1, A becomes slope

// B * ? = -1;
// ? = -1/B

// for vertical lines B = 0
if (Input->CoefB != 0.0)
	{
	m = Input->CoefA * (-1.0 / Input->CoefB);
	Pt2[0] = Pt1[0] + 1;
	Pt2[1] = Pt1[1] + m;
	} // if
else
	{
	Pt2[0] = Pt1[0];
	Pt2[1] = Pt1[1] + 1;
	} // else

return (CalcGeneralLinearEquation(Pt1, Pt2, Output));

} // CalcParallelLinearEquation

/*===========================================================================*/

// returns an error if lines do not intersect
int CalcLineIntersection(struct GeneralLinearEquation *LineA, struct GeneralLinearEquation *LineB, Point2d Intersection)
{
double Denom, invDenom;
int rVal = 1;

// coordinates of intersection: x = (B1 * C2 - B2 * C1) / (A1 * B2 - A2 * B1)
//								y = (C1 * A2 - C2 * A1) / (A1 * B2 - A2 * B1)

if ((Denom = LineA->CoefA * LineB->CoefB - LineB->CoefA * LineA->CoefB) != 0.0)
	{
	invDenom = 1.0 / Denom;

	Intersection[0] = (LineA->CoefB * LineB->CoefC - LineB->CoefB * LineA->CoefC) * invDenom;
	Intersection[1] = (LineA->CoefC * LineB->CoefA - LineB->CoefC * LineA->CoefA) * invDenom;
	} // if
else	// lines are parallel or the same line
	{
	Intersection[1] = Intersection[0] = FLT_MAX;
	rVal = 0;
	} // else

return (rVal);

} // CalcLineIntersection

/*===========================================================================*/

// returns an error if both points are equal
int CalcLineOrthogonal(Point2d Pt1, Point2d Pt2, int LastPt, struct GeneralLinearEquation *Eq)
{
struct GeneralLinearEquation SegEq;

if (Pt1[1] != Pt2[1])	// if not horizontal
	{
	if (CalcGeneralLinearEquation(Pt1, Pt2, &SegEq))
		{
		if (SegEq.CoefA != 0.0)
			{
			Eq->CoefA = -SegEq.CoefB / SegEq.CoefA;
			Eq->CoefB = 1.0;
			if (LastPt)
				{
				Eq->CoefC = -Pt2[1] + (SegEq.CoefB / SegEq.CoefA) * Pt2[0];
				} // if
			else
				{
				Eq->CoefC = -Pt1[1] + (SegEq.CoefB / SegEq.CoefA) * Pt1[0];
				} // else
			return (1);
			} // if
		} // if
	else
		return (0);
	} // if
// original line was horizontal so orthogonal is vertical
if (LastPt)
	{
	Pt1[0] = Pt2[0];		// same X
	Pt1[1] = Pt2[1] + 1;	// offset Y
	} // if
else
	{
	Pt2[0] = Pt1[0];
	Pt2[1] = Pt1[1] + 1;
	} // if

return (CalcGeneralLinearEquation(Pt1, Pt2, Eq));

} // CalcLineOrthogonal

/*===========================================================================*/

int PointContainedInSegment(Point2d SegStart, Point2d SegEnd, Point2d Pt)
{
double HighX, LowX, HighY, LowY;

HighX = max(SegStart[0], SegEnd[0]);
LowX = min(SegStart[0], SegEnd[0]);
HighY = max(SegStart[1], SegEnd[1]);
LowY = min(SegStart[1], SegEnd[1]);

if (HighX - LowX < .0000001)
	{
	return (Pt[1] >= LowY && Pt[1] <= HighY && fabs(HighX - Pt[0]) < .0000001);
	} // if
else if (HighY - LowY < .0000001)
	{
	return (Pt[0] >= LowX && Pt[0] <= HighX && fabs(HighY - Pt[1]) < .0000001);
	} // else if

return (Pt[0] >= LowX && Pt[0] <= HighX && Pt[1] >= LowY && Pt[1] <= HighY);

} // PointContainedInSegment

/*===========================================================================*/

/*** No longer used - FPW2 09/27/07
short PointEnclosedPoly3(VertexDEM *Vert[3], double LatPt, double LonPt)
{
short Contained = 0, Pt, NextPt;

for (Pt = 0; Pt < 3; Pt ++)
	{
	NextPt = Pt < 2 ? Pt + 1: 0;
	Contained += LineSegsIntersect(Vert[Pt]->Lat, Vert[Pt]->Lon, Vert[NextPt]->Lat, Vert[NextPt]->Lon, LatPt, LonPt);
	} // for

return (Contained % 2);

} // PointEnclosedPoly3
***/

/*===========================================================================*/

short ComputePolygonInterpolants(double *XYZ, double *PolyX, double *PolyY, long PolyMinX, long PolyMinY, 
	short PolyMinXPt, long MinScrnX, long MinScrnY,
	short &MaxPt, short &MinPt, double &StartPt, double &IncX, double &IncY)
{
double Temp, X5, Y5, Y4, X3, El3;
long MidPt;

if (XYZ[2] > XYZ[0])
	{
	if (XYZ[2] > XYZ[1])
		{
		MaxPt = 2;
		MinPt = XYZ[1] < XYZ[0] ? 1: 0;
		MidPt = MinPt == 1 ? 0: 1;
		} // if 
	else
		{
		MaxPt = 1;
		MinPt = XYZ[2] < XYZ[0] ? 2: 0;
		MidPt = MinPt == 2 ? 0: 2;
		} // else 
	} // if 
else
	{
	if (XYZ[0] > XYZ[1])
		{
		MaxPt = 0;
		MinPt = XYZ[2] < XYZ[1] ? 2: 1;
		MidPt = MinPt == 2 ? 1: 2;
		} // if 
	else
		{
		MaxPt = 1;
		MinPt = XYZ[2] < XYZ[0] ? 2: 0;
		MidPt = MinPt == 2 ? 0: 2;
		} // else 
	} // else 

if (XYZ[MaxPt] == XYZ[MinPt])
	{
	IncX = 0.0;
	IncY = 0.0;
	} // that was easy 
else
	{
	Temp = (XYZ[MidPt] - XYZ[MinPt]) /
		(XYZ[MaxPt] - XYZ[MinPt]);
	X5 = Temp * (PolyX[MaxPt] - PolyX[MinPt]) + PolyX[MinPt];
	Y5 = Temp * (PolyY[MaxPt] - PolyY[MinPt]) + PolyY[MinPt];
	if (PolyY[MaxPt] == PolyY[MinPt])
		{
		IncY = IncX = 0.0;
		StartPt = XYZ[MidPt];
		return (0);
		//    X3 = Y4 = infinity;
		} // if 
	X3 = PolyX[MinPt] + (PolyY[MidPt] - PolyY[MinPt]) *
		(PolyX[MaxPt] - PolyX[MinPt]) /
		(PolyY[MaxPt] - PolyY[MinPt]);
	if (X5 == PolyX[MidPt])
		{
		IncY = IncX = 0.0;
		StartPt = XYZ[MidPt];
		return (0);
		//    Y4 = infinity;
		} // if 
	Y4 = PolyY[MidPt] + (X3 - PolyX[MidPt]) *
		(Y5 - PolyY[MidPt]) / (X5 - PolyX[MidPt]);
	if (PolyX[MaxPt] != PolyX[MinPt])
		El3 = XYZ[MinPt] + (X3 - PolyX[MinPt]) *
			(XYZ[MaxPt] - XYZ[MinPt]) /
			(PolyX[MaxPt] - PolyX[MinPt]);
	else if (PolyY[MaxPt] != PolyY[MinPt])
		El3 = XYZ[MinPt] + (PolyY[MidPt] - PolyY[MinPt]) *
			(XYZ[MaxPt] - XYZ[MinPt]) /
			(PolyY[MaxPt] - PolyY[MinPt]);
	else
		{
		IncY = IncX = 0.0;
		StartPt = XYZ[MidPt];
		return (0);
		} // else 

	if (X3 != PolyX[MidPt] && Y4 != PolyY[MidPt])
		{
		IncX = (El3 - XYZ[MidPt]) / (X3 - PolyX[MidPt]);
		IncY = (El3 - XYZ[MidPt]) / (PolyY[MidPt] - Y4);
		} // if 
	else
		{
		IncY = IncX = 0.0;
		StartPt = XYZ[MidPt];
		return (0);
		} // else 
	} // else not so easy 
StartPt = XYZ[MidPt] + ((PolyX[PolyMinXPt] - PolyX[MidPt]) + (MinScrnX - PolyMinX)) * IncX;
StartPt += ((PolyY[0] - PolyY[MidPt]) + (MinScrnY - PolyMinY)) * IncY;

return (1);

} // ComputePolygonInterpolants

/*===========================================================================*/

// Heron's Formula for area of triangle
// Side lengths must be >= 0 and sum of any two sides must be greater than the third side.
// Could also use MinPointLineSq() which uses dot products (see below).
double SolveTriangleArea(VertexData *Vtx[3])
{
double Len[3], X, Y, Z, S;

X = Vtx[0]->XYZ[0] - Vtx[1]->XYZ[0];
Y = Vtx[0]->XYZ[1] - Vtx[1]->XYZ[1];
Z = Vtx[0]->XYZ[2] - Vtx[1]->XYZ[2];
Len[0] = sqrt(X * X + Y * Y + Z * Z);

X = Vtx[0]->XYZ[0] - Vtx[2]->XYZ[0];
Y = Vtx[0]->XYZ[1] - Vtx[2]->XYZ[1];
Z = Vtx[0]->XYZ[2] - Vtx[2]->XYZ[2];
Len[1] = sqrt(X * X + Y * Y + Z * Z);

X = Vtx[2]->XYZ[0] - Vtx[1]->XYZ[0];
Y = Vtx[2]->XYZ[1] - Vtx[1]->XYZ[1];
Z = Vtx[2]->XYZ[2] - Vtx[1]->XYZ[2];
Len[2] = sqrt(X * X + Y * Y + Z * Z);

S = .5 * (Len[0] + Len[1] + Len[2]);
S = S * (S - Len[0]) * (S - Len[1]) * (S - Len[2]);

return (S > 0.0 ? sqrt(S): 0.0);

} // SolveTriangleArea

/*===========================================================================*/

double SolveTriangleArea2D(Point2d Pt1, Point2d Pt2, Point2d Pt3)
{
double OriginY, Area;

OriginY = Pt1[1];

Area = ((Pt2[1] - Pt1[1]) * (Pt2[0] - Pt1[0]) * .5);
Area += ((Pt3[1] - Pt2[1]) * (Pt3[0] - Pt2[0]) * .5 + (Pt2[1] - OriginY) * (Pt3[0] - Pt2[0]));
Area += ((Pt1[1] - Pt3[1]) * (Pt1[0] - Pt3[0]) * .5 + (Pt3[1] - OriginY) * (Pt1[0] - Pt3[0]));

return (Area);

} // SolveTriangleArea

/*===========================================================================*/

double SolveTriangleArea2D(VertexDEM *Vtx1, VertexDEM *Vtx2, VertexDEM *Vtx3)
{
double OriginY, Area;

OriginY = Vtx1->Lat;

Area = ((Vtx2->Lat - Vtx1->Lat) * (Vtx2->Lon - Vtx1->Lon) * .5);
Area += ((Vtx3->Lat - Vtx2->Lat) * (Vtx3->Lon - Vtx2->Lon) * .5 + (Vtx2->Lat - OriginY) * (Vtx3->Lon - Vtx2->Lon));
Area += ((Vtx1->Lat - Vtx3->Lat) * (Vtx1->Lon - Vtx3->Lon) * .5 + (Vtx3->Lat - OriginY) * (Vtx1->Lon - Vtx3->Lon));

return (Area);

} // SolveTriangleArea

/*===========================================================================*/

// The following function, MinPointLine
// is covered by the comments below:

// MAGIC Software
// http://www.cs.unc.edu/~eberly
//
// This free software is supplied under the following terms:
// 1. You may distribute the original source code to others at no charge.
// 2. You may modify the original source code and distribute it to others at
//    no charge.  The modified code must be documented to indicate that it is
//    not part of the original package.
// 3. You may use this code for non-commercial purposes.  You may also
//    incorporate this code into commercial packages.  However, you may not
//    sell any of your source code which contains my original and/or modified
//    source code (see items 1 and 2).  In such a case, you would need to
//    factor out my code and freely distribute it.
// 4. The original code comes with absolutely no warranty and no guarantee is
//    made that the code is bug-free.

#define eberlyDISTSq(x) (eberlyDot(x,x))

//---------------------------------------------------------------------------

float MinPointLineSq (const eberlyPoint3& p, const Line3& line, float& t)
{

eberlyPoint3 diff = p - line.b;
t = eberlyDot(line.m,diff)/eberlyDot(line.m,line.m);
diff = diff - t*line.m;

return eberlyDISTSq(diff);

}

// From Perlin

// 12/10/01 - I don't find these functions referenced from anywhere. - CXH
/*** 06/06/07 - commented out - FW2

double bias(double a, double b)
{
	return pow(a, log(b) / log(0.5));
}

***/

/*===========================================================================*/

/***

double gain(double a, double b)
{
	double p = log(1. - b) / log(0.5);

	if (a < .001)
		return 0.;
	else if (a > .999)
		return 1.;
	if (a < 0.5)
		return pow(2 * a, p) * 0.5;
	else
		return 1. - pow(2 * (1. - a), p) * 0.5;
}

***/
