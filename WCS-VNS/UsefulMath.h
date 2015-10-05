// UsefulMath.h
// Math-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#include "stdafx.h"

#include <cmath>

#ifndef WCS_USEFULMATH_H
#define WCS_USEFULMATH_H

// moved here from Defines.h on 2/20/06 by CXH since it makes more sense here
#define Pi     3.1415926535898
#define HalfPi 1.5707963267949
#define TwoPi  6.2831853071796
#define OneAndHalfPi 4.7123889803847
#define PiOver180 1.74532925199433E-002
#define PiUnder180 5.72957795130823E+001

#define LOG2 .69314718
#define INVLOG2 (1.0 / LOG2)

#define DEG2RAD(d)	(d * PiOver180)
#define RAD2DEG(r)	(r * PiUnder180)

#define DOUBLE_MASK_1(x)    (*x & 0xFFFFFFFFFFFFFFFELL)
#define DOUBLE_MASK_2(x)    (*x & 0xFFFFFFFFFFFFFFFCLL)
#define DOUBLE_MASK_3(x)    (*x & 0xFFFFFFFFFFFFFFF8LL)
#define DOUBLE_MASK_4(x)    (*x & 0xFFFFFFFFFFFFFFF0LL)
#define DOUBLE_MASK_5(x)    (*x & 0xFFFFFFFFFFFFFFE0LL)
#define DOUBLE_MASK_6(x)    (*x & 0xFFFFFFFFFFFFFFC0LL)
#define DOUBLE_MASK_7(x)    (*x & 0xFFFFFFFFFFFFFF80LL)
#define DOUBLE_MASK_8(x)    (*x & 0xFFFFFFFFFFFFFF00LL)
#define DOUBLE_MASK_9(x)    (*x & 0xFFFFFFFFFFFFFE00LL)

#define DOUBLE_MASK_VALUE(x)	DOUBLE_MASK_9(x)
#define DOUBLE_MASK_ANGLE(x)	DOUBLE_MASK_9(x)


#ifndef ROUNDUP
        // Do NOT make b a zero, that would be dumb.
        #define ROUNDUP(a,b)    ((b) * (((a) + ((b) - 1)) / (b)))

#endif // ROUNDUP

// Handy DPFP clamp from Perlin via Huber
double inline PERLIN_clamp(double x, double a, double b) {return (x <= a ? a: x >= b ? b: x);};

#define PERLIN_lerp(t, a, b) ((a) + (t) * ((b) - (a)))
#define PERLIN_s_curve(t) ((t) * (t) * (3. - 2. * (t)))

// Macro form of linear interpolation.
#define lerp(t, a, b) ( (a) + (t) * ((b) - (a)) )

// inline function form of double-precision version
inline double flerp(double t, double a, double b) {return( a + t * (b - a) );}

// Comparison of three items
#define MAX3(a, b, c) ((a)>(b) ? ((a)>(c) ? (a): (c)) : ((b)>(c) ? (b): (c)))
#define MIN3(a, b, c) ((a)<(b) ? ((a)<(c) ? (a): (c)) : ((b)<(c) ? (b): (c)))
#define MID3(a, b, c) ((a)<(b) ? ((a)>(c) ? (a): ((b)<(c) ? (b): (c))) : ((a)<(c) ? (a): ((c)<(b) ? (b): (c))))

#ifdef SPEED_MOD
#define WCS_floor(x)	quickdblfloor(x)
#define WCS_ceil(x)		quickdblceil(x)
//inline double WCS_floor(double x)	{int ix = (int)x; return((double)(ix - (int)(x < 0.0 && x != ix)  ));};
//inline double WCS_ceil(double x)	{return((double)((int)x + (int)(x > 0.0)));};
#define WCS_round(x)    ((x) >= 0.0 ? floor((x) + .5): ceil((x) - .5))
#define WCS_qround(x)    ((x) >= 0.0 ? WCS_floor((x) + .5): WCS_ceil((x) - .5))
#else // SPEED_MOD
inline double WCS_floor(double x)	{return((double)((int)(x) - (int)((x) < 0.0 && (x) != (int)(x))  ));};
inline double WCS_ceil(double x)	{return((double)((int)(x) + (int)((x) > 0.0)  ));};
inline double WCS_round(double x)	{return((x) >= 0.0 ? floor((x) + .5): ceil((x) - .5));};
#endif // SPEED_MOD
inline double WCS_max(double x, double y)	{return((x) > (y) ? (x): (y));};
inline double WCS_min(double x, double y)	{return((x) < (y) ? (x): (y));};

//float FASTCALL FresnelReflectionCoef(float CosAngle, float RefrRatio);
double FresnelReflectionCoef(double CosAngle, double RefrRatio);
double FresnelReflectionCoefWater(double CosAngle);

// From http://www.lightsoft.co.uk/PD/stu/StuChat36.html
// and http://lists.apple.com/archives/PerfOptimization-dev/2005/Jan/msg00051.html
// approximate to some number of decimal places
// |error| < 0.005
inline float fastatanf( float z ) { return(z  < 1.0f ? (z / (1.0f + 0.28f * (z * z))) :  1.5707963f - z / (z * z + 0.28f) );};

double Round(double InVal, short Precision);
void StandardNormalDistributionInit(void);
double StandardNormalDistribution(double K);

double WCS_gcd(double m, double n, const double EPS=0.1);
double WCS_lcm(double a, double b, const double EPS=0.1);
double WCS_array_lcm( double* a, int count, const double EPS=0.1 );

union wtf
	{
	double d;
	long l[2];
	int  i[2];
	};

long quickftol(double val);
double quickdblceil(double val);
double quickdblfloor(double val);
double quickdblfrac(double val);
int quickintround(double val);
int quickintceil(double val);
int quickintfloor(double val);
long quicklonground(double val);
long quicklongceil(double val);
long quicklongfloor(double val);

#endif // WCS_USEFULMATH_H
