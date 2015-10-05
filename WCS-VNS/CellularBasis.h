// CellularBasis.h
// Header file for cellular basis functions
// Created from scratch by Gary R. Huber on 10/22/98
// Copyright 1998 by Questar Productions. All rights reserved.

#ifndef WCS_CELLULARBASIS_H
#define WCS_CELLULARBASIS_H

class PRNGX;

#define WCS_CELLULARBASIS_MAXPTSPERCELL		10
#define WCS_CELLULARBASIS_MAXFEATUREPTS		5

// Maximum supported dimensions is 4, though we currently never use more than 3
#define WCS_CELLULARBASIS_MAXDIM	4

// Caching technique currently only works up to 3 dimensions
#define WCS_CELLULARBASIS_CACHE_MAXDIMS		3

// One point in a Cell
class CBCellPoint
	{
	public:
		double Coords[WCS_CELLULARBASIS_CACHE_MAXDIMS];
	}; // CBCellPoint

// One cell in the cache
class CBCacheCell
	{
	public:
		CBCellPoint Points[WCS_CELLULARBASIS_MAXPTSPERCELL];
		char NumPoints, Valid;

		CBCacheCell() {NumPoints = 0; Valid = 0;};
	}; // CBCacheCell

class CBCacheOctave
	{
	public:
		// C++ array initialization makes it difficult to size
		// these arrays dymically at runtime, so we just go with
		// the maximum of 3 all the time and waste a little memory
		// when we're less than three.
		CBCacheCell CellCache[27];
		int CellIDValid;
		long RootCellID[WCS_CELLULARBASIS_CACHE_MAXDIMS];

	CBCacheOctave() {CellIDValid = 0;};
	}; // CBCacheOctave

class CellularBasis
	{
	public:
		double PtProb[WCS_CELLULARBASIS_MAXPTSPERCELL];
		double Dist[WCS_CELLULARBASIS_MAXFEATUREPTS];		// this is F1, F2, F3... ala Worley
		double PtsPerCellParam;
		long DimensionsParam, PointsNeededParam;
		static PRNGX CellBasisRand;

		CBCacheOctave *CBCO; // Can be up to 10, currently, but I don't know where this define is

		CellularBasis(double *Args, int NumArgs);
		~CellularBasis();
		void InitProbability(double PtsPerCell);
		void Evaluate(int OctaveNum, double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);
		void EvaluateManhattan(int OctaveNum, double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class CellularBasis

class F1CellBasis3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F1CellBasis3(double *Args, int NumArgs);
		// ~F1CellBasis3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F1CellBasis3

class F2CellBasis3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F2CellBasis3(double *Args, int NumArgs);
		// ~F2CellBasis3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F2CellBasis3

class F2mF1CellBasis3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F2mF1CellBasis3(double *Args, int NumArgs);
		// ~F2mF1CellBasis3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F2mF1CellBasis3

class F3mF1CellBasis3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F3mF1CellBasis3(double *Args, int NumArgs);
		// ~F3mF1CellBasis3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F3mF1CellBasis3

class F1Manhattan3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F1Manhattan3(double *Args, int NumArgs);
		// ~F1Manhattan3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F1Manhattan3

class F2Manhattan3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F2Manhattan3(double *Args, int NumArgs);
		// ~F2Manhattan3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F2Manhattan3

class F2mF1Manhattan3 : public CellularBasis
	{
	public:
		long OctavesParam;

		F2mF1Manhattan3(double *Args, int NumArgs);
		// ~F2mF1Manhattan3()
		double Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4] = 0);

	}; // class F2mF1Manhattan3

#endif // WCS_CELLULARBASIS_H
