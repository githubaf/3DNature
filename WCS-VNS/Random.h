// Random.h
// PRNG/xrand Pseudo Random Number Generator header
// Built by Chris "Xenon" Hanson from parts of Useful.h and some new code
// on 09 Nov 1998. Copyright 1998

#ifndef WCS_RANDOM_H
#define WCS_RANDOM_H


// *****************************************************
// ***** xrand

void xseed48(unsigned long NewSeedLo, unsigned long NewSeedHi);
double xrand48(void);
unsigned long xrand48int(void);

// These variants use the older, slower, VNS2/WCS6 code, to be compatible with what
// NatureView Express expects
void xseed48backcompat(unsigned long NewSeedLo, unsigned long NewSeedHi);
unsigned long xrand48intbackcompat(void);

// Don't try to include C++ objects in our basic C-style lib
#ifndef BUILD_LIB

// *****************************************************
// ***** PRNGX

class PRNGX
	{
	private:
		UINT64 rnum;
	public:
		PRNGX() {rnum = 0;};
		void Seed64(unsigned long NewSeedLo, unsigned long NewSeedHi);
		void Seed64BitShift(unsigned long a, unsigned long b)	{Seed64(((a) >> 1), ((b) >> 1));};
		void Copy(PRNGX *CopyFrom)	{rnum = CopyFrom->rnum;};
		double GenPRN(void);
		double Seed64GenPRN(unsigned long NewSeedLo, unsigned long NewSeedHi);
		void   GenMultiPRN(unsigned int NumDesired, double *OutputBin);
		double GenGauss(void);
	}; // PRNGX

#endif // !BUILD_LIB


// *****************************************************
// ***** Gaussian

// GaussRand came from V1 Fractal.c/V2 RenderFractal.cpp
double GaussRand(void);

// Don't try to include C++ objects in our basic C-style lib
#ifndef BUILD_LIB

// InitGauss and DoGauss are now history, use GaussRand
//#define Newf3(PRNG,delta,x0,x1,x2) 	(((x0 + x1 + x2) * (1.0 / 3.0)) + delta * (PRNG)->GenGauss())
//#define Newf4(PRNG,delta,x0,x1,x2,x3)	(((x0 + x1 + x2 + x3) * 0.25) + delta * (PRNG)->GenGauss())  // Optimized out division. "* 0.25" was / 4.0

#endif // !BUILD_LIB

#endif // WCS_RANDOM_H
