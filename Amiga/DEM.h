// DEM.h
// DEM object, part of the database
// Created from scratch on 8/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_DEM_H
#define WCS_DEM_H

#include <stdio.h>
#include <math.h>
#include "Useful.h"

#define WCS_DEM_CURVERSION			1.02
#define WCS_DEM_HEADERSIZE_V102		64

class GeoPoint
	{
	public:
		double Lat, Lon;
	}; // GeoPoint

class DEM
	{
	private:
		unsigned long int Flags;  // MapAsSFC, etc
		short *RawMap;
		long *ScaledMap, *DSMap;
		float *ScreenX, *ScreenY, *ScreenZ;
		unsigned char RawCount;
		// CloudData *Cloud;
		// MotionKey *Motion;
		// ColorKey *Color;

		// Stuff that's public, but should be read-only so access via member
		GeoPoint pNorthWest, pSouthEast;
		// LonEntries used to be Rows, LatEntries was Columns
		unsigned long int pLonEntries, pLatEntries, pSamples;
		unsigned long int pDSLonEntries, pDSLatEntries, pDSSamples;
		signed short int pMaxEl, pMinEl;
		float pSumElDif, pSumElDifSq, pFileVersion;
		double pLatStep, pLonStep, pElScale;

		unsigned long int ReadV101Header(FILE *In);
		inline unsigned long int MapSize(void) {return((pLonEntries + 1) * pLatEntries);};

	public:
		DEM();
		~DEM();
		
		// These are just read-only inline interfaces to the private
		// variables above. They'll be optimized into an inline
		// access by the compiler, but this keeps them from being
		// written to from outside the object.

		// LonEntries used to be Rows, LatEntries was Columns
		#define Rows(a)		LonEntries(a)
		#define Columns(a)	LatEntries(a)
		inline unsigned long int LonEntries (void) {return(pLonEntries);};
		inline unsigned long int LatEntries (void) {return(pLatEntries);};
		inline double Northest  (void) {return(pNorthWest.Lat);};
		inline double Southest  (void) {return(pSouthEast.Lat);};
		inline double Eastest   (void) {return(pSouthEast.Lon);};
		inline double Westest   (void) {return(pNorthWest.Lon);};
		inline double LatStep   (void) {return(pLatStep);};
		inline double LonStep   (void) {return(pLonStep);};
		inline double ElScale   (void) {return(pElScale);};
		inline signed short int MaxEl (void) {return(pMaxEl);};
		inline signed short int MinEl (void) {return(pMinEl);};
		inline unsigned long int Samples (void) {return(pSamples);};
		inline double SumElDif  (void) {return(pSumElDif);};
		inline double SumElDifSq(void) {return(pSumElDifSq);};

		inline unsigned long int DSLonEntries (void) {return(pDSLonEntries);};
		inline unsigned long int DSLatEntries (void) {return(pDSLatEntries);};
		inline unsigned long int DSSamples (void) {return(pDSSamples);};

		inline unsigned long int CheckFlags(unsigned long int F) {return(F & Flags);};
		inline void SetFlags(unsigned long int F) {Flags |= F;};

		unsigned long int LoadRawElevs(char *InName);
		unsigned long int LoadRawElevs(FILE *Input);
		void FreeRawElevs(void);
		
		//inline unsigned long int ScaleElevs(void) {return(ScaleElevs(0, 1));};
		//unsigned long int ScaleElevs(double Datum, double VertExag);
		//void FreeScaledElevs(void);
		
		//unsigned long int DownSample(unsigned int Ratio);
		//void FreeDownSamElevs(void);
		
		// unsigned long int ScreenTransform();
		// void FreeScreenData(void);
		
		// See IterateTypes below for the ItType argument. Returns
		// number of output samples per horiztonal axis.
		//unsigned long int SetupIterate(char ItType, double LowVertAxis,
		// double HighVertAxis, double LowHorizAxis, double HighHorizAxis,
		// double VertAxisInc, double HorizAxisInc);
		//inline double NextSample(void);
		
		inline signed short int Sample(unsigned long int Horiz, unsigned long int Vert) {return(RawMap[Vert + (Horiz * pLatEntries)]);};
		inline signed short int SampleRaw(unsigned long int Offset) {return(RawMap[Offset]);};
		
		//double SampleCoords(double Lat, double Lon);

	}; // DEM

// Definitions for the flag bits in the DEM object, use SetFlags and
// CheckFlags

enum
	{
	WCS_DEM_FLAG_MAPASSFC
	}; // DEMFlags

enum
	{
	WCS_DEM_ITERATE_BYSAMPLE,
	WCS_DEM_ITERATE_BYCOORD
	}; // IterateTypes

#endif // WCS_DEM_H
