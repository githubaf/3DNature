// Noise.h
//
// Created from scratch and bits of NoiseTest.cpp on 18 Sep 1998 by CXH
// Copyright 1998

#include "stdafx.h"

#ifndef WCS_NOISE_H
#define WCS_NOISE_H

#include "Useful.h"

#ifndef max
#define   max(a,b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define   min(a,b)    ((a) <= (b) ? (a) : (b))
#endif

#define PERLIN_SETUP(i,b0,b1,r0,r1)\
	t = vec[i] + PERLIN_N_MASK;\
	tfloor = floor(t);\
	b0 = quickftol(tfloor) & PERLIN_BITMASK;\
	b1 = (b0+1) & PERLIN_BITMASK;\
	r0 = t - tfloor;\
	r1 = r0 - 1.;

#define PERLIN_at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

//#define PERLIN_clamp(x, a, b) min(max((x), (a)), (b))		// Bad idea! Perlin. Now in-line func in Useful.cpp

#define PERLIN_RANGE_B 0x100
#define PERLIN_BITMASK 0xff

#define PERLIN_N_MASK 0x1000
#define PERLIN_NOISE_MAX_OCTAVES 10

double inline SmoothstepRange(double x, double a, double b, double r)
	{
	double c, rVal;

	if (x <= a)
		rVal = 0.0;
	else if (x >= b)
		rVal = 1.0;
	else
		{
		c = (x - a) / r;
		rVal = PERLIN_s_curve(c);
		} // else

	return (rVal);
	};

double inline SmoothstepRangeReverse(double x, double a, double b, double r)
	{
	double c, rVal;

	if (x <= a)
		rVal = 1.0;
	else if (x >= b)
		rVal = 0.0;
	else
		{
		c = (b - x) / r;
		rVal = PERLIN_s_curve(c);
		} // else

	return (rVal);
	};

double inline SmoothPulse(double x, double s, double w)
	{
	double hw = w * .5, rVal;

	//x -= floor(x);
	x = quickdblfrac(x);

	if (x > 1.0 - w)
		rVal = SmoothstepRangeReverse(x, 1.0 - w, 1.0, w);
	else
		rVal = SmoothstepRange(x, s - hw, s + hw, w);

	return (rVal);
	};

double inline InvertSmoothPulse(double x, double s, double w)
	{
	double hw = w * .5, rVal;

	//x -= floor(x);
	x = quickdblfrac(x);

	if (x > 1.0 - w)
		rVal = 1.0 - SmoothstepRangeReverse(x, 1.0 - w, 1.0, w);
	else
		rVal = 1.0 - SmoothstepRange(x, s - hw, s + hw, w);

	return (rVal);
	};

class NoiseVector
	{
	public:
		double x;
		double y;
		double z;
	}; // NoiseVector

class GenericNoise
	{
	public:
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0) = 0;
	}; // GenericNoise


/* Copy-Paste Template for classes derived from GenericNoise

class Noise : public GenericNoise
	{
	public:
		Noise(double *Args = NULL, int NumArgs = 0);
		~Noise();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0) = 0;
		virtual NoiseVector EvaluateVecNoise(NoiseVector point, double *Args = NULL, int NumArgs = 0) = 0;
	}; // Noise

*/

// No real params for this type of noise
class PerlinNoise3 : public GenericNoise
	{
	private:
		static int ClassInit;
		static unsigned long PerlinPseudoPermut[PERLIN_RANGE_B + PERLIN_RANGE_B + 2];
		static double PerlinG3[PERLIN_RANGE_B + PERLIN_RANGE_B + 2][3];
	public:
		PerlinNoise3(double *Args = NULL, int NumArgs = 0);
		//~PerlinNoise3();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
		//virtual NoiseVector EvaluateVecNoise(NoiseVector point, double *Args = NULL, int NumArgs = 0);
	}; // PerlinNoise3

class PerlinVLNoise3 : public PerlinNoise3
	{
	public:
		PerlinVLNoise3(double *Args = NULL, int NumArgs = 0);
		//~PerlinVLNoise3();
		// Args: distortion
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // PerlinVLNoise3


// Derives from PerlinNoise3, adds Exponent Array common to all our fractal noises
// Purely virtual, unusable unless subclassed.
class FractalNoise: public PerlinNoise3
	{
	protected:
		double HVal, lacunarityVal, octavesVal;
		double ExponentArray[PERLIN_NOISE_MAX_OCTAVES + 1],
		 TempExponentArray[PERLIN_NOISE_MAX_OCTAVES + 1];
		inline void InitExpArray(double *ExpArray, double HInit, double lacunartyInit, double octavesInit);
	public:
		FractalNoise(double *Args = NULL, int NumArgs = 0);
		//~FractalNoise();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0) = 0;
		//virtual NoiseVector EvaluateVecNoise(NoiseVector point, double *Args = NULL, int NumArgs = 0) = 0;
	}; // FractalNoise
/*
class Turbulence: public PerlinNoise3
	{
	public:
		Turbulence(double *Args = NULL, int NumArgs = 0);
		//~Turbulence();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // Turbulence
*/
class fBmNoise : public FractalNoise
	{
	public:
		fBmNoise(double *Args = NULL, int NumArgs = 0);
		//~fBmNoise();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // fBmNoise

class fBmTurbulentNoise : public FractalNoise
	{
	public:
		fBmTurbulentNoise(double *Args = NULL, int NumArgs = 0);
		//~fBmTurbulentNoise();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // fBmTurbulentNoise

class MultiFractal : public FractalNoise
	{
	protected:
		double offsetVal;
	public:
		MultiFractal(double *Args = NULL, int NumArgs = 0);
		//~MultiFractal();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // MultiFractal

class HybridMultiFractal : public MultiFractal
	{
	public:
		HybridMultiFractal(double *Args = NULL, int NumArgs = 0);
		//~HybridMultiFractal();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // HybridMultiFractal

class HeteroTerrain : public MultiFractal
	{
	public:
		HeteroTerrain(double *Args = NULL, int NumArgs = 0);
		//~HeteroTerrain();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // HeteroTerrain

class RidgedMultiFractal : public MultiFractal
	{
	protected:
		double gainVal;
	public:
		RidgedMultiFractal(double *Args = NULL, int NumArgs = 0);
		//~RidgedMultiFractal();
		virtual double EvaluateNoise(const NoiseVector &point, double *Args = NULL, int NumArgs = 0);
	}; // RidgedMultiFractal


#endif // WCS_NOISE_H
