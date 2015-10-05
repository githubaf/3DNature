// UsefulMath.cpp
// Math-related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulMath.h"

// Lots of stuff on net about how this works.  Search for either the floating point number or the power.
// Of course some of the info is wrong ;)  AMD and others have macros, but I thought using a union would
// make it easier to understand.  The macro might be faster though.
static double magicdnum = 6755399441055744.0;	// 2^52 * 1.5
static double magicdelta = 1.5e-8;
static double magicround = 0.5f - magicdelta;

/*===========================================================================*/

// like, below, but hard-coded to water Coef (1.33), which speeds calculations
double FresnelReflectionCoefWater(double CosAngle)
{
double G, GpC, GmC, T1, T2, T3, rVal;

if (CosAngle < 0.0)
	rVal = 0.0;
else
	{
	//G = RefrRatio * RefrRatio + CosAngle * CosAngle - 1.0;
	G = 0.7689 + CosAngle * CosAngle; // simplified equation without variable Coef
	//if (G >= 0.0 && CosAngle > 0.0)
	if (CosAngle > 0.0) // G is always > 0.0 (and, in fact > 0.7689 if Coef always = 1.33 (water)
		{
		G = sqrt(G);
		GpC = G + CosAngle;
		GmC = G - CosAngle;
		T1 = .5 * (GmC * GmC) / (GpC * GpC);
		T2 = CosAngle * GpC - 1.0;
		T3 = CosAngle * GmC + 1.0;
		rVal = T1 * (1.0 + (T2 * T2) / (T3 * T3));
		} // if
	else
		{
		rVal = 1.0;
		} // else
	} // else

return (rVal);

} // FresnelReflectionCoef

/*===========================================================================*/

double FresnelReflectionCoef(double CosAngle, double RefrRatio)
{
double G, GpC, GmC, T1, T2, T3, rVal;

if (CosAngle < 0.0)
	rVal = 0.0;
else
	{
	G = RefrRatio * RefrRatio + CosAngle * CosAngle - 1.0;
	if (G >= 0.0 && CosAngle > 0.0)
		{
		G = sqrt(G);
		GpC = G + CosAngle;
		GmC = G - CosAngle;
		T1 = .5 * (GmC * GmC) / (GpC * GpC);
		T2 = CosAngle * GpC - 1.0;
		T3 = CosAngle * GmC + 1.0;
		rVal = T1 * (1.0 + (T2 * T2) / (T3 * T3));
		} // if
	else
		{
		rVal = 1.0;
		} // else
	} // else

return (rVal);

} // FresnelReflectionCoef

/*===========================================================================*/


double Round(double InVal, short Precision)
{
double MultVal;

MultVal = pow(10.0, (int)Precision);

return (quickdblfloor(InVal * MultVal + .5) / MultVal);

} // Round

/*===========================================================================*/

static double NormalDistr[31];

void StandardNormalDistributionInit(void)
{
double Distr[31] = {
					.5, .4602, .4207, .3821, .3446, .3085, .2743, .2420, .2119, .1841,
					.1587, .1357, .1151, .0968, .0808, .0668, .0548, .0446, .0359, .0287,
					.0228, .0179, .0139, .0107, .0082, .0062, .0047, .0035, .0026, .0019, .0013
					};
unsigned long i;

for (i = 0; i < 31; i ++)
	NormalDistr[i] = Distr[i];

} // StandardNormalDistributionInit

/*===========================================================================*/

double StandardNormalDistribution(double K)
{
long I, Ip1;

if (K < 0.0 || K > 30.0)
	return (0.0);

I = quicklongfloor(K);
Ip1 = quicklongceil(K);

if (I == Ip1)
	return (NormalDistr[I]);

return (NormalDistr[I] + (K - I) * (NormalDistr[Ip1] - NormalDistr[I]));

} // StandardNormalDistribution

/*===========================================================================*/

// use the EPS value to change the "accuracy" of this function
// the default value sets it to about 2 places worth of precision
// e.g., 1.33 is evaluated as 1 1/3, but 1.3 isn't
double WCS_gcd(double m, double n, const double EPS)
{
double rem;

while (n > EPS)
	{
	// perform a % on floating-point numbers
	rem = quickdblfrac(m / n) * n;
	m = n;
	n = rem;
	} // while

return m;

} // WCS_gcd

/*===========================================================================*/

double WCS_lcm(double a, double b, const double EPS)
{

double res = (a * (b / WCS_gcd(a, b, EPS)));

return res;

} // WCS_lcm

/*===========================================================================*/

double WCS_array_lcm( double* a, int count, const double EPS) {
// Not data safe.  Assumes array count is accurate
double res;

--count; // set count for direct array references
if (count)
	res = a[count];
while (count)
	{
	res = WCS_lcm(res, a[count-1], EPS);
	--count;
	} // while

return res;

} // WCS_array_lcm

/*===========================================================================*/
// Super efficient routines via magic numbers (see header for more info)
/*===========================================================================*/

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

long quickftol(double val)
{

//++DevCounter[10];	// Quadview Railway = 851026809 times

// SSE2 load scalar double & truncate to int
return((long)_mm_cvttsd_si32(_mm_load_sd(&val)));

/***
union wtf tricky;

tricky.d = val;
tricky.d += (val > 0) ? -magicround : magicround;
tricky.d += magicdnum;

//assert((long)val == tricky.l[0]);

return (tricky.l[0]);
***/

}; // quickftol

/*===========================================================================*/

int quickintround(double val)
{
union wtf tricky;

//++DevCounter[11];	// Quadview Railway = 0 times

tricky.d = val + magicdelta;

#ifdef WCS_BUILD_FRANK
assert((int)(val + 0.5) == tricky.i[0]);
#endif // WCS_BUILD_FRANK

return (tricky.i[0]);

}; // quickintround

/*===========================================================================*/

int quickintceil(double val)
{
union wtf tricky;

//++DevCounter[12];	// Quadview Railway = 25324508 times

tricky.d = (val + magicround) + magicdnum;

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
int official = (int)ceil(val);
assert(official == tricky.i[0]);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.i[0]);

}; // quickintceil

/*===========================================================================*/

int quickintfloor(double val)
{
union wtf tricky;

//++DevCounter[13];	// Quadview Railway = 7816386 times

tricky.d = (val - magicround) + magicdnum;

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
// good way to examine bit changes in debugger
double step1 = val - magicround;
double step2 = step1 + magicdnum;
int official = (int)floor(val);
assert(official == tricky.i[0]);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.i[0]);

}; // quickintfloor

/*===========================================================================*/

long quicklonground(double val)
{
union wtf tricky;

//++DevCounter[14];	// Quadview Railway = 0 times

tricky.d = val + magicdelta;

#ifdef WCS_BUILD_FRANK
assert((long)(val + 0.5) == tricky.l[0]);
#endif // WCS_BUILD_FRANK

return (tricky.l[0]);

}; // quicklonground

/*===========================================================================*/

long quicklongceil(double val)
{
union wtf tricky;

//++DevCounter[15];	// Quadview Railway = 0 times

tricky.d = (val + magicround) + magicdnum;

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
long official = (long)ceil(val);
assert(official == tricky.l[0]);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.l[0]);

}; // quicklongceil

/*===========================================================================*/

long quicklongfloor(double val)
{
long rVal;

//++DevCounter[16];	// Quadview Railway = 6975571 times

if (val >= 0)
	rVal = (long)_mm_cvttsd_si32(_mm_load_sd(&val));	// this instruction truncates (rounds) to zero, not towards -infinity
else
	{
	rVal = (long)floor(val);
	} // else

return rVal;

}; // quicklongfloor

/*===========================================================================*/

double quickdblceil(double val)
{
union wtf tricky;

//++DevCounter[17];	// Quadview Railway = 1091575 times

tricky.d = (val + magicround) + magicdnum;
tricky.d = tricky.l[0];

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
double official = ceil(val);
assert(official == tricky.d);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.d);

}; // quickdblceil

/*===========================================================================*/

double quickdblfloor(double val)
{
union wtf tricky;

//++DevCounter[18];	// Quadview Railway = 149679608 times

tricky.d = (val - magicround) + magicdnum;
tricky.d = tricky.l[0];

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
double step1 = val - magicround;
double step2 = step1 + magicdnum;
double official = floor(val);
assert(official == tricky.d);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.d);

}; // quickdblfloor

/*===========================================================================*/

// Returns fractional portion of number.  Use where you do x = x - floor(x).
double quickdblfrac(double val)
{
union wtf tricky;

//++DevCounter[19];	// Quadview Railway = 156555 times

tricky.d = (val - magicround) + magicdnum;
tricky.d = val - tricky.l[0];

#ifdef WCS_BUILD_FRANK
#ifdef DEBUG
double official = val - floor(val);
assert(official == tricky.d);
#endif // DEBUG
#endif // WCS_BUILD_FRANK

return (tricky.d);

}; // quickdblfrac
