// Noise.cpp
// Source for classic noise generators
//
// Created from NoiseTest.c on 18 Sep 1998 by CXH
// Some code is from/copyright Perlin and Musgrave et al.

#include "stdafx.h"
#include "Noise.h"

void normalize2(double v[2]);
void normalize3(double v[3]);

// static variables in PerlinNoise3 scope
int PerlinNoise3::ClassInit;
unsigned long PerlinNoise3::PerlinPseudoPermut[PERLIN_RANGE_B + PERLIN_RANGE_B + 2];
double PerlinNoise3::PerlinG3[PERLIN_RANGE_B + PERLIN_RANGE_B + 2][3];

static double tfloor;

void normalize2(double v[2])
{
double s;

s = sqrt(v[0] * v[0] + v[1] * v[1]);
v[0] = v[0] / s;
v[1] = v[1] / s;

} // normalize2

/*===========================================================================*/

void normalize3(double v[3])
{
double invs;

invs = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
v[0] = v[0] * invs;
v[1] = v[1] * invs;
v[2] = v[2] * invs;

} // normalize3

/*===========================================================================*/

/***
void normalize3f(float v[3])
{
float invs;

invs = (float)(1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
v[0] = v[0] * invs;
v[1] = v[1] * invs;
v[2] = v[2] * invs;

} // normalize3
***/

/*===========================================================================*/

// Good old-fashioned Perlin noise. Low-calorie and yummy.
PerlinNoise3::PerlinNoise3(double *Args, int NumArgs)
{
int i, j, k;

// One-time initialization
if(!ClassInit)
	{
	srand(3141567); // Bogus value.

	for(i = 0; i < PERLIN_RANGE_B; i++)
		{
		PerlinPseudoPermut[i] = i;

		for (j = 0 ; j < 3 ; j++)
			{
			PerlinG3[i][j] = (double)((rand() % (PERLIN_RANGE_B + PERLIN_RANGE_B)) - PERLIN_RANGE_B) / PERLIN_RANGE_B;
			} // for
		normalize3(PerlinG3[i]);
		} // for

	while(--i)
		{
		k = PerlinPseudoPermut[i];
		PerlinPseudoPermut[i] = PerlinPseudoPermut[j = rand() % PERLIN_RANGE_B];
		PerlinPseudoPermut[j] = k;
		} // while

	for (i = 0; i < PERLIN_RANGE_B + 2; i++)
		{
		PerlinPseudoPermut[PERLIN_RANGE_B + i] = PerlinPseudoPermut[i];
		for (j = 0 ; j < 3 ; j++)
			{
			PerlinG3[PERLIN_RANGE_B + i][j] = PerlinG3[i][j];
			} // for
		} // for
	ClassInit = 1;
	} // if

} // PerlinNoise3::PerlinNoise3

/*===========================================================================*/

//extern double TotalTime;

double PerlinNoise3::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double rx0, rx1, ry0, ry1, rz0, rz1, sy, sz, a, b, c, d, t, rVal;
double u[2], v[2];
double vec[3];
double *q[8];
unsigned long bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
unsigned long i, j;

//StartHiResTimer();

vec[0] = point.x;
vec[1] = point.y;
vec[2] = point.z;

PERLIN_SETUP(0, bx0,bx1, rx0,rx1);
PERLIN_SETUP(1, by0,by1, ry0,ry1);
PERLIN_SETUP(2, bz0,bz1, rz0,rz1);

i = PerlinPseudoPermut[ bx0 ];
j = PerlinPseudoPermut[ bx1 ];

b00 = PerlinPseudoPermut[ i + by0 ];
b10 = PerlinPseudoPermut[ j + by0 ];
b01 = PerlinPseudoPermut[ i + by1 ];
b11 = PerlinPseudoPermut[ j + by1 ];

t  = PERLIN_s_curve(rx0);
sy = PERLIN_s_curve(ry0);
sz = PERLIN_s_curve(rz0);

// 2nd pairs seemed to be fetched in time anyways - adding those prefetches slows things down
q[0] = PerlinG3[b11 + bz1];
_mm_prefetch((char *)q[0], _MM_HINT_T0);
q[1] = PerlinG3[b01 + bz1];
//_mm_prefetch((char *)q[1], _MM_HINT_T0);
q[2] = PerlinG3[b10 + bz1];
_mm_prefetch((char *)q[2], _MM_HINT_T0);
q[3] = PerlinG3[b00 + bz1];
//_mm_prefetch((char *)q[3], _MM_HINT_T0);
q[4] = PerlinG3[b11 + bz0];
_mm_prefetch((char *)q[4], _MM_HINT_T0);
q[5] = PerlinG3[b01 + bz0];
//_mm_prefetch((char *)q[5], _MM_HINT_T0);
q[6] = PerlinG3[b10 + bz0];
_mm_prefetch((char *)q[6], _MM_HINT_T0);
q[7] = PerlinG3[b00 + bz0];
//_mm_prefetch((char *)q[7], _MM_HINT_T0);

v[0] = ( rx1 * q[0][0] + ry1 * q[0][1] + rz1 * q[0][2] );
u[0] = ( rx0 * q[1][0] + ry1 * q[1][1] + rz1 * q[1][2] );
b = ((u[0]) + (t) * ((v[0]) - (u[0])));

v[1] = ( rx1 * q[2][0] + ry0 * q[2][1] + rz1 * q[2][2] );
u[1] = ( rx0 * q[3][0] + ry0 * q[3][1] + rz1 * q[3][2] );
a = ((u[1]) + (t) * ((v[1]) - (u[1])));

d = ((a) + (sy) * ((b) - (a)));

v[0] = ( rx1 * q[4][0] + ry1 * q[4][1] + rz0 * q[4][2] );
u[0] = ( rx0 * q[5][0] + ry1 * q[5][1] + rz0 * q[5][2] );
b = ((u[0]) + (t) * ((v[0]) - (u[0])));

v[1] = ( rx1 * q[6][0] + ry0 * q[6][1] + rz0 * q[6][2] );
u[1] = ( rx0 * q[7][0] + ry0 * q[7][1] + rz0 * q[7][2] );
a = ((u[1]) + (t) * ((v[1]) - (u[1])));

c = PERLIN_lerp(sy, a, b);

rVal = (PERLIN_lerp(sz, c, d));

//TotalTime += StopHiResTimerSecs();

return(rVal);

} // PerlinNoise3::EvaluateNoise

// Reference version of above
/***
double PerlinNoise3::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double rx0, rx1, ry0, ry1, rz0, rz1, sy, sz, a, b, c, d, t, u, v, rVal;
double vec[3];
double *q;
unsigned long bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
unsigned long i, j;

StartHiResTimer();

vec[0] = point.x;
vec[1] = point.y;
vec[2] = point.z;

PERLIN_SETUP(0, bx0,bx1, rx0,rx1);
PERLIN_SETUP(1, by0,by1, ry0,ry1);
PERLIN_SETUP(2, bz0,bz1, rz0,rz1);

i = PerlinPseudoPermut[ bx0 ];
j = PerlinPseudoPermut[ bx1 ];

b00 = PerlinPseudoPermut[ i + by0 ];
b10 = PerlinPseudoPermut[ j + by0 ];
b01 = PerlinPseudoPermut[ i + by1 ];
b11 = PerlinPseudoPermut[ j + by1 ];

t  = PERLIN_s_curve(rx0);
sy = PERLIN_s_curve(ry0);
sz = PERLIN_s_curve(rz0);

q = PerlinG3[ b11 + bz1 ] ; v = PERLIN_at3(rx1,ry1,rz1);
q = PerlinG3[ b01 + bz1 ] ; u = PERLIN_at3(rx0,ry1,rz1);
b = PERLIN_lerp(t, u, v);

q = PerlinG3[ b10 + bz1 ] ; v = PERLIN_at3(rx1,ry0,rz1);
q = PerlinG3[ b00 + bz1 ] ; u = PERLIN_at3(rx0,ry0,rz1);
a = PERLIN_lerp(t, u, v);

d = PERLIN_lerp(sy, a, b);

q = PerlinG3[ b11 + bz0 ] ; v = PERLIN_at3(rx1,ry1,rz0);
q = PerlinG3[ b01 + bz0 ] ; u = PERLIN_at3(rx0,ry1,rz0);
b = PERLIN_lerp(t, u, v);

q = PerlinG3[ b10 + bz0 ] ; v = PERLIN_at3(rx1,ry0,rz0);
q = PerlinG3[ b00 + bz0 ] ; u = PERLIN_at3(rx0,ry0,rz0);
a = PERLIN_lerp(t, u, v);

c = PERLIN_lerp(sy, a, b);

rVal = (PERLIN_lerp(sz, c, d));

TotalTime += StopHiResTimerSecs();

return(rVal);

} // PerlinNoise3::EvaluateNoise
***/

/*===========================================================================*/

/* "Variable Lacunarity Noise"  -or- VLNoise3()
 * A distorted variety of Perlin noise.
 *
 * Copyright 1994 F. Kenton Musgrave 
 */

PerlinVLNoise3::PerlinVLNoise3(double *Args, int NumArgs)
: PerlinNoise3(Args, NumArgs)
{

} // PerlinVLNoise3::PerlinVLNoise3

/*===========================================================================*/

// <<<>>> Needs some work, lacking VecNoise3() and AddVectors()
double PerlinVLNoise3::EvaluateNoise( const NoiseVector &point, double *Args, int NumArgs)
{
double distortion = 0;
NoiseVector offset;
// NoiseVector VecNoise3(), AddVectors();

if(NumArgs) (distortion = Args[0]);

offset.x = point.x + 0.5; // misregister domain
offset.y = point.y + 0.5;
offset.z = point.z + 0.5;

/* <<<>>> */ //offset = VecNoise3( offset ); // get a random vector

offset.x *= distortion; // scale the randomization
offset.y *= distortion;
offset.z *= distortion;

/* ``point'' is the domain; distort domain by adding ``offset'' */
/* <<<>>> */ // point = AddVectors( point, offset );

return (PerlinNoise3::EvaluateNoise(point)); /* distorted-domain noise */
} // PerlinVLNoise3::EvaluateNoise

/*===========================================================================*/

FractalNoise::FractalNoise(double *Args, int NumArgs)
: PerlinNoise3(Args, NumArgs)
{
double HParam = 0.5, lacunarityParam = 2.0, octavesParam = 6.0;

if (NumArgs > 0)
	{
	octavesParam = Args[0];
	if (NumArgs > 1)
		{
		lacunarityParam = Args[1];
		if (NumArgs > 2)
			HParam = Args[2];
		} // if
	} // if

HVal = HParam;
lacunarityVal = lacunarityParam;
octavesVal = min(octavesParam, PERLIN_NOISE_MAX_OCTAVES);

// precompute and store spectral weights
FractalNoise::InitExpArray(ExponentArray, HVal, lacunarityVal, octavesVal);

} // FractalNoise::FractalNoise

/*===========================================================================*/

void FractalNoise::InitExpArray(double *ExpArray, double HInit,
 double lacunarityInit, double octavesInit)
{
double frequency;
unsigned int i;

// precompute and store spectral weights
frequency = 1.0;
for (i = 0; i < octavesInit + 1.0; ++i)
	{
	// compute weight for each frequency
	ExpArray[i] = pow(frequency, -HInit);
	frequency *= lacunarityInit;
	} // for

} // FractalNoise::InitExpArray

/*
Turbulence::Turbulence(double *Args, int NumArgs)
: PerlinNoise3(Args, NumArgs)
{

} // Turbulence::Turbulence
*/

/*
// <<<>>>
double turbulence(double *v, double freq)
{
	double t;
	NoiseVector point;

	for (t = 0. ; freq >= 1. ; freq /= 2) {
		point.x = freq * v[0];
		point.y = freq * v[1];
		point.z = freq * v[2];
		t += fabs(Noise3(point)) / freq;
	}
	return t;
}
*/


/*
 * Procedural fBm evaluated at "point"; returns value stored in "value".
 *
 * Copyright 1994 F. Kenton Musgrave 
 * 
 * Parameters:
 *    ``H''  is the fractal increment parameter -- default .5
 *    ``lacunarity''  is the gap between successive frequencies -- default 2
 *    ``octaves''  is the number of frequencies in the fBm -- default 6
 */

fBmNoise::fBmNoise(double *Args, int NumArgs)
: FractalNoise(Args, NumArgs)
{
} // fBmNoise::fBmNoise

/*===========================================================================*/

double fBmNoise::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal;
double value, remainder;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

// initialize vars to proper values
value = 0.0;

localpoint = point; // make modifiable local copy

// inner loop of spectral construction
/* this is actually slower than single-threaded
#ifdef _OPENMP
i = 0;
#pragma omp parallel for firstprivate(localpoint) lastprivate(i) reduction(+: value)
for (i = 0; i < (int)octaves; i++)
	{
	double LacFactor;
	LacFactor = pow(lacunarity, i);
	localpoint.x *= LacFactor;
	localpoint.y *= LacFactor;
	localpoint.z *= LacFactor;
	value = PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
	} // for
#else // _OPENMP
*/
for (i = 0; i < (int)octaves; i++)
	{
	value += PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;
	} // for
/*
#endif // _OPENMP
*/

remainder = octaves - (int)octaves;
if (remainder)
	{
/*
	#ifdef _OPENMP
	double LacFactor;
	LacFactor = pow(lacunarity, i);
	localpoint.x *= LacFactor;
	localpoint.y *= LacFactor;
	localpoint.z *= LacFactor;
	#endif // _OPENMP
*/
	/* add in ``octaves'' remainder */
   /* ``i''  and spatial freq. are preset in loop above */
   value += remainder * PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
   } // if

return(PERLIN_clamp(value, -1.0, 1.0));
} // fBmNoise::EvaluateNoise

// This is Peachey's version of turbulence - maybe not as good as vector valued turbulence

fBmTurbulentNoise::fBmTurbulentNoise(double *Args, int NumArgs)
: FractalNoise(Args, NumArgs)
{
} // fBmTurbulentNoise::fBmTurbulentNoise

/*===========================================================================*/

double fBmTurbulentNoise::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal;
double value, remainder;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

// initialize vars to proper values
value = 0.0;

localpoint = point; // make modifiable local copy

// inner loop of spectral construction
for (i = 0; i < (int)octaves; i++)
	{
	value += fabs(PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i]);
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;
	} // for

remainder = octaves - (int)octaves;
if (remainder)
	{
	/* add in ``octaves'' remainder */
   /* ``i''  and spatial freq. are preset in loop above */
   value += remainder * PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
   } // if

return(PERLIN_clamp(value, 0.0, 1.5));
} // fBmTurbulentNoise::EvaluateNoise

/*===========================================================================*/

/*
 * Procedural multifractal evaluated at "point"; 
 * returns value stored in "value".
 *
 * Copyright 1994 F. Kenton Musgrave 
 * 
 * Parameters:
 *    ``H''  determines the highest fractal dimension
 *    ``lacunarity''  is gap between successive frequencies
 *    ``octaves''  is the number of frequencies in the fBm
 *    ``offset''  is the zero offset, which determines multifractality
 */

MultiFractal::MultiFractal(double *Args, int NumArgs)
: FractalNoise(Args, NumArgs)
{
double offsetParam = 0.7;
if(NumArgs > 3) offsetParam = Args[3];

offsetVal = offsetParam;
} // MultiFractal::MultiFractal

/*===========================================================================*/

double MultiFractal::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal, offset = offsetVal;
double value, remainder;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 3) offset = Args[3];
if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

// initialize vars to proper values
value = 0.0;
/* frequency = 1.0; */

localpoint = point; // make modifiable local copy

// inner loop of spectral construction
for (i = 0; i < (int)octaves; i++)
	{
	// Frequency is hardcoded to 1.0, whassup wi 'dat?
	/* value *= offset * frequency * PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i]; */
	//value *= offset * PerlinNoise3::EvaluateNoise(localpoint);	// could this be the problem? * ExpArray[i];
	value += offset * PerlinNoise3::EvaluateNoise(localpoint);	// could this be the problem? * ExpArray[i];
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;
	} // for

remainder = octaves - (int)octaves;
if (remainder)
	{
	/* add in ``octaves'' remainder */
   /* ``i''  and spatial freq. are preset in loop above */
   value += remainder * PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
   } // if

return(PERLIN_clamp(value, -1.0, 1.0));
} // MultiFractal::EvaluateNoise

/*===========================================================================*/

/* Hybrid additive/multiplicative multifractal terrain model.
 *
 * Copyright 1994 F. Kenton Musgrave 
 *
 * Some good parameter values to start with:
 *
 *      H:           0.25
 *      offset:      0.7
 */

HybridMultiFractal::HybridMultiFractal(double *Args, int NumArgs)
: MultiFractal(Args, NumArgs)
{

} // HybridMultiFractal::HybridMultiFractal

/*===========================================================================*/

double HybridMultiFractal::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal, offset = offsetVal;
double remainder, value, signal, weight;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 3) offset = Args[3];
if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

localpoint = point; // make modifiable local copy

// get first octave of function
value = (PerlinNoise3::EvaluateNoise(localpoint) + offset) * ExpArray[0];
weight = value;

/* increase frequency */
localpoint.x *= lacunarity;
localpoint.y *= lacunarity;
localpoint.z *= lacunarity;

/* spectral construction inner loop, where the fractal is built */
for (i = 1; i < (int)octaves; i++)
	{
	/* prevent divergence */
	weight = min(weight, 1.0);

	/* get next higher frequency */
	signal = (PerlinNoise3::EvaluateNoise(localpoint) + offset) * ExpArray[i];
	
	/* add it in, weighted by previous freq's local value */
	value += weight * signal;

	/* update the (monotonically decreasing) weighting value */
	/* (this is why H must specify a high fractal dimension) */
	weight *= signal;

	/* increase frequency */
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;
	} /* for */

/* take care of remainder in ``octaves''  */
remainder = octaves - (int)octaves;
if ( remainder )
	{
	/* ``i''  and spatial freq. are preset in loop above */
	value += remainder * PerlinNoise3::EvaluateNoise(localpoint) * ExpArray[i];
	} // if

return(PERLIN_clamp(value, 0.0, 3.0));
} // HybridMultiFractal::EvaluateNoise

/*===========================================================================*/

/*
 * Heterogeneous procedural terrain function: stats by altitude method.
 * Evaluated at "point"; returns value stored in "value".
 *
 * Copyright 1994 F. Kenton Musgrave 
 * 
 * Parameters:
 *       ``H''  determines the fractal dimension of the roughest areas
 *       ``lacunarity''  is the gap between successive frequencies
 *       ``octaves''  is the number of frequencies in the fBm
 *       ``offset''  raises the terrain from `sea level'
 */

HeteroTerrain::HeteroTerrain(double *Args, int NumArgs)
: MultiFractal(Args, NumArgs)
{

} // HeteroTerrain::HeteroTerrain

/*===========================================================================*/

double HeteroTerrain::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal, offset = offsetVal;
double remainder, value, increment;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 3) offset = Args[3];
if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

localpoint = point; // make modifiable local copy

// first unscaled octave of function; later octaves are scaled
value = PerlinNoise3::EvaluateNoise(localpoint) + offset;
localpoint.x *= lacunarity;
localpoint.y *= lacunarity;
localpoint.z *= lacunarity;

// spectral construction inner loop, where the fractal is built
for (i = 1; i < (int)octaves; i++)
	{
	// obtain displaced noise value
	increment = PerlinNoise3::EvaluateNoise(localpoint) + offset;
	// scale amplitude appropriately for this frequency
	increment *= ExpArray[i];
	// scale increment by current `altitude' of function
	increment *= value;
	// add increment to ``value''
	value += increment;
	// raise spatial frequency
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;
	} // for

// take care of remainder in ``octaves''
remainder = octaves - (int)octaves;
if (remainder)
	{
	/* ``i''  and spatial freq. are preset in loop above
	** note that the main loop code is made shorter here
	** you may want to that loop more like this */
	increment = (PerlinNoise3::EvaluateNoise(localpoint) + offset) * ExpArray[i];
	value += remainder * increment * value;
	} // if

return(PERLIN_clamp(value, 0.0, 3.0));
} // HeteroTerrain::EvaluateNoise

/*===========================================================================*/

/* Ridged multifractal terrain model.
 *
 * Copyright 1994 F. Kenton Musgrave 
 *
 * Some good parameter values to start with:
 *
 *      H:           1.0
 *      offset:      1.0
 *      gain:        2.0
 */

RidgedMultiFractal::RidgedMultiFractal(double *Args, int NumArgs)
: MultiFractal(Args, NumArgs)
{
double gainParam = 0.7;
if(NumArgs > 4) gainParam = Args[4];

gainVal = gainParam;
} // RidgedMultiFractal::RidgedMultiFractal

/*===========================================================================*/

double RidgedMultiFractal::EvaluateNoise(const NoiseVector &point, double *Args, int NumArgs)
{
double H = HVal, lacunarity = lacunarityVal, octaves = octavesVal,
 offset = offsetVal, gain = gainVal;
double value, signal, weight;
double *ExpArray = ExponentArray;
int i;
NoiseVector localpoint;

if(NumArgs > 4) gain = Args[4];
if(NumArgs > 3) offset = Args[3];
if(NumArgs > 0) octaves = Args[0];
if(NumArgs > 2) H = Args[2];
if(NumArgs > 1) 
	{
	lacunarity = Args[1];
	InitExpArray(TempExponentArray, H, lacunarity, octaves);
	ExpArray = TempExponentArray;
	} // if

localpoint = point; // make modifiable local copy

// get first octave
signal = PerlinNoise3::EvaluateNoise(localpoint);
// get absolute value of signal (this creates the ridges)
if ( signal < 0.0 ) signal = -signal;
// invert and translate (note that "offset" should be ~= 1.0)
signal = offset - signal;
// square the signal, to increase "sharpness" of ridges
signal *= signal;
// assign initial values
value = signal;
weight = 1.0;

for(i = 1; i < (int)octaves; i++)
	{
	/* increase the frequency */
	localpoint.x *= lacunarity;
	localpoint.y *= lacunarity;
	localpoint.z *= lacunarity;

	/* weight successive contributions by previous signal */
	weight = signal * gain;
	weight = PERLIN_clamp(weight, 0.0, 1.0);
	signal = PerlinNoise3::EvaluateNoise(localpoint);
	if (signal < 0.0) signal = -signal;
	signal = offset - signal;
	signal *= signal;
	/* weight the contribution */
	signal *= weight;
	value += signal * ExpArray[i];
	} // for

return(PERLIN_clamp(value, -1.0, 1.0));

} // RidgedMultiFractal::EvaluateNoise
