// Random.h
// PRNG/xrand Pseudo Random Number Generator header
// Built by Chris "Xenon" Hanson from parts of Useful.h and some new code
// on 09 Nov 1998. Copyright 1998

#ifndef WCS_RANDOM_H
#define WCS_RANDOM_H

#define MAX_XRAND 100000



// *****************************************************
// ***** xrand

void xseed48(unsigned long NewSeedLo, unsigned long NewSeedHi);
double xrand48(void);
unsigned long int xrand48int(void);

// Don't try to include C++ objects in our basic C-style lib
#ifndef BUILD_LIB

// *****************************************************
// ***** PRNGX


class PRNGX
	{
	private:
		unsigned int PRNGSeed[3];
	public:
		PRNGX() {PRNGSeed[0] = PRNGSeed[1] = PRNGSeed[2] = 0;};
		void Seed64(unsigned long NewSeedLo, unsigned long NewSeedHi);
		void Seed3Val64(unsigned int *NewSeed)
			{PRNGSeed[0] = NewSeed[0]; PRNGSeed[1] = NewSeed[1]; PRNGSeed[2] = NewSeed[2];};
		void Seed64BitShift(unsigned long a, unsigned long b)	{Seed64(((a) >> 1), ((b) >> 1));};
		void Copy(PRNGX *CopyFrom)	{PRNGSeed[0] = CopyFrom->PRNGSeed[0]; PRNGSeed[1] = CopyFrom->PRNGSeed[1]; PRNGSeed[2] = CopyFrom->PRNGSeed[2];};

		// inline these?
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

// InitGauss and DoGauss are now histroy, use GaussRand
#define Newf3(PRNG,delta,x0,x1,x2) 	(((x0 + x1 + x2) * (1.0 / 3.0)) + delta * (PRNG)->GenGauss())
#define Newf4(PRNG,delta,x0,x1,x2,x3)	(((x0 + x1 + x2 + x3) * 0.25) + delta * (PRNG)->GenGauss())  // Optimized out division. "* 0.25" was / 4.0

#endif // !BUILD_LIB

#endif // WCS_RANDOM_H
