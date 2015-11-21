// UsefulMath.h
// Math-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULMATH_H
#define WCS_USEFULMATH_H

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

inline double WCS_floor(double x)	{return((double)((int)(x) - (int)((x) < 0.0 && (x) != (int)(x))  ));};
inline double WCS_ceil(double x)	{return((double)((int)(x) + (int)((x) > 0.0)  ));};
inline double WCS_round(double x)	{return(WCS_floor(x + .5));};
#ifdef SPEED_MOD
#define WCS_fabs(x)	fabs(x)
#else // SPEED_MOD
inline double WCS_fabs(double x)	{return(x < 0.0 ? -x: x);};
#endif // SPEED_MOD
inline double WCS_max(double x, double y)	{return((x) > (y) ? (x): (y));};
inline double WCS_min(double x, double y)	{return((x) < (y) ? (x): (y));};

//float FASTCALL FresnelReflectionCoef(float CosAngle, float RefrRatio);
double FresnelReflectionCoef(double CosAngle, double RefrRatio);

double Round(double InVal, short Precision);
void StandardNormalDistributionInit(void);
double StandardNormalDistribution(double K);

double WCS_gcd(double m, double n, const double EPS=0.1);
double WCS_lcm(double a, double b, const double EPS=0.1);
double WCS_array_lcm( double* a, int count, const double EPS=0.1 );

#endif // WCS_USEFULMATH_H
