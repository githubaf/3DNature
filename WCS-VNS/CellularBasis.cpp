// CellularBasis.cpp
// Cellular basis functions
// Created from scratch by Gary R. Huber on 10/22/98
// Copyright 1998 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CellularBasis.h"
#include "Useful.h"
#include "Random.h"
#include "Application.h"
#include "AppMem.h"

PRNGX CellularBasis::CellBasisRand;

#define Poisson(x, M, xF)	((pow((double)M, (double)x) * exp((double)-M)) / (double)xF)

// build extra cache-coherancy tests and asserts into debug builds
//#define WCS_CELLULARBASIS_CACHE_DEBUG

// try more aggressive cache preservation during RootCell changes
//#define WCS_CELLULARBASIS_CACHE_XFER

CellularBasis::CellularBasis(double *Args, int NumArgs)
{
long X, NumOctaves;

CBCO = NULL;

if (NumArgs > 3)
	PtsPerCellParam = Args[3];
else
	PtsPerCellParam = 1.0;
if (NumArgs > 2)
	PointsNeededParam = quicklongfloor(Args[2]);
else
	PointsNeededParam = 1;
if (NumArgs > 1)
	DimensionsParam = quicklongfloor(Args[1]);
else
	DimensionsParam = 3;
if (NumArgs > 0)
	NumOctaves = quicklongfloor(Args[0]);
else
	NumOctaves = 1;

if(DimensionsParam < 4)
	{ // caching only supported up to 3D
	CBCO = new CBCacheOctave[NumOctaves + 1];
	} // if

InitProbability(PtsPerCellParam);

for (X = 0; X < WCS_CELLULARBASIS_MAXFEATUREPTS; ++X)
	Dist[X] = FLT_MAX;

} // CellularBasis::CellularBasis

/*===========================================================================*/

CellularBasis::~CellularBasis()
{

if(CBCO)
	{
	delete [] CBCO;
	CBCO = NULL;
	} // if
} // CellularBasis::~CellularBasis

/*===========================================================================*/

void CellularBasis::InitProbability(double PtsPerCell)
{
unsigned long X, XFactorial = 1;

// assign probablilities for each possible number of feature points per cell
for (X = 0; X < WCS_CELLULARBASIS_MAXPTSPERCELL; ++X)
	{
	if (X > 1)
		XFactorial *= X;

	PtProb[X] = Poisson(X, PtsPerCell, XFactorial);

	// make the probablilities cumulative
	if (X > 0)
		{
		PtProb[X] += PtProb[X - 1];
		if (PtProb[X] > .99)
			{
			PtProb[X] = 1.0;
			break;
			} // if
		} // if
	} // for

// just to be sure we cover the whole range of random numbers
PtProb[WCS_CELLULARBASIS_MAXPTSPERCELL - 1] = 1.0;

} // CellularBasis::InitProbability

/*===========================================================================*/

// PRNBin is a bin of doubles equal to the maximum number of Point PRNs we
// could generate in one evaluate. This is used to batch the generation of
// the PRNs and prevent stack/functioncall operations for each one.
static double PRNBin[WCS_CELLULARBASIS_MAXDIM * WCS_CELLULARBASIS_MAXPTSPERCELL];

// Pt is an array of doubles that specify XYZT in cells (divided already by size of cell)
// Be sure that the number of points requested does not exceed the number of points allowed: WCS_CELLULARBASIS_MAXFEATUREPTS

//extern double TotalTime;

void CellularBasis::Evaluate(int OctaveNum, double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double PtDist, D, CellDist[4][2], ThisPt[4], xOffset, yOffset, zOffset, tOffset, ClosestEdge;
unsigned long Pass;
long Seed[4], RootCell[4], EvalCell[4], NumPtsThisCell, PtsFound = 0, Ct, TestCt, MoveCt,
	BaseCt = 0, NumCellsToCheck = 1, CellCheck, Dimensions = DimensionsParam, PointsNeeded = PointsNeededParam;
int PrnBinIDX, MaxCells, CacheSlot;
int RootMatch = 0;
#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
int CellValid, StashNumPoints;
#endif // WCS_CELLULARBASIS_CACHE_DEBUG
char CellsToCheck[81][4], x, y, z, t, LastPoint, SearchXrange, SearchYrange, SearchZrange, SearchTrange;

//StartHiResTimer();

if (NumArgs > 3)
	{
	InitProbability(Args[3]);
	} // if
if (NumArgs > 2)
	{
	PointsNeeded = quicklongfloor(Args[2]);
	} // if
if (NumArgs > 1)
	{
	Dimensions = quicklongfloor(Args[1]);
	} // if

// initialize distances
for (Ct = 0; Ct < PointsNeeded; ++Ct)
	{
	Dist[Ct] = FLT_MAX;
	} // for
memset(ThisPt, 0, sizeof(ThisPt));

// set initial seed values - need at least one dimension
for (Pass = 0; Pass < 2; ++Pass)
	{
	for (CellCheck = 0; CellCheck < NumCellsToCheck; ++CellCheck)
		{
		int GenCell = 1;

		MaxCells = 3;
		if (Pass > 0)
			{
			EvalCell[0] = RootCell[0] + CellsToCheck[CellCheck][0];
			Seed[0] = EvalCell[0] % USHRT_MAX;
			CellDist[0][0] = Pt[0] - EvalCell[0];
			CellDist[0][1] = 1.0 - CellDist[0][0];
			if (Dimensions > 1)
				{
				MaxCells = 9;
				EvalCell[1] = RootCell[1] + CellsToCheck[CellCheck][1];
				Seed[1] = EvalCell[1] % USHRT_MAX;
				CellDist[1][0] = Pt[1] - EvalCell[1];
				CellDist[1][1] = 1.0 - CellDist[1][0];
				if (Dimensions > 2)
					{
					MaxCells = 27;
					EvalCell[2] = RootCell[2] + CellsToCheck[CellCheck][2];
					Seed[2] = EvalCell[2] % USHRT_MAX;
					CellDist[2][0] = Pt[2] - EvalCell[2];
					CellDist[2][1] = 1.0 - CellDist[2][0];
					if (Dimensions > 3)
						{
						//MaxCells = 81;
						EvalCell[3] = RootCell[3] + CellsToCheck[CellCheck][3];
						Seed[3] = EvalCell[3] % USHRT_MAX;
						CellDist[3][0] = Pt[3] - EvalCell[3];
						CellDist[3][1] = 1.0 - CellDist[3][0];
						} // if
					else
						{
						Seed[3] = 0;
						} // else
					} // if
				else
					{
					Seed[3] = Seed[2] = 0;
					} // else
				} // if
			else
				{
				Seed[3] = Seed[2] = Seed[1] = 0;
				} // else
			} // if
		else
			{
			LastPoint = (char)(PointsNeeded - 1);
			EvalCell[0] = RootCell[0] = quicklongfloor(Pt[0]);
			Seed[0] = EvalCell[0] % USHRT_MAX;
			CellDist[0][0] = Pt[0] - EvalCell[0];
			CellDist[0][1] = 1.0 - CellDist[0][0];
			SearchXrange = 1;
			if (Dimensions > 1)
				{
				MaxCells = 9;
				EvalCell[1] = RootCell[1] = quicklongfloor(Pt[1]);
				Seed[1] = EvalCell[1] % USHRT_MAX;
				CellDist[1][0] = Pt[1] - EvalCell[1];
				CellDist[1][1] = 1.0 - CellDist[1][0];
				SearchYrange = 1;
				if (Dimensions > 2)
					{
					MaxCells = 27;
					EvalCell[2] = RootCell[2] = quicklongfloor(Pt[2]);
					Seed[2] = EvalCell[2] % USHRT_MAX;
					CellDist[2][0] = Pt[2] - EvalCell[2];
					CellDist[2][1] = 1.0 - CellDist[2][0];
					SearchZrange = 1;
					if (Dimensions > 3)
						{
						//MaxCells = 81;
						EvalCell[3] = RootCell[3] = quicklongfloor(Pt[3]);
						Seed[3] = EvalCell[3] % USHRT_MAX;
						CellDist[3][0] = Pt[3] - EvalCell[3];
						CellDist[3][1] = 1.0 - CellDist[3][0];
						SearchTrange = 1;
						} // if
					else
						{
						Seed[3] = 0;
						SearchTrange = 0;
						} // else
					} // if
				else
					{
					Seed[3] = Seed[2] = 0;
					SearchZrange = 0;
					} // else
				} // if
			else
				{
				Seed[3] = Seed[2] = Seed[1] = 0;
				SearchYrange = 0;
				} // else

			// determine if the cache could contain any valid cells keyed to this root cell
			if (CBCO && CBCO[OctaveNum].CellIDValid)
				{
				if (CBCO[OctaveNum].RootCellID[0] == RootCell[0])
					{
					if (Dimensions > 1)
						{
						if (CBCO[OctaveNum].RootCellID[1] == RootCell[1])
							{
							if (Dimensions > 2)
								{
								if (CBCO[OctaveNum].RootCellID[2] == RootCell[2])
									{
									// we have a RootCell match, deal with it below
									RootMatch = 1; // match
									} // if
								else
									{
									RootMatch = 0; // invalidate or transfer below
									} // else
								} // if
							} // if
						else
							{
							RootMatch = 0; // invalidate or transfer below
							} // else
						} // if
					} // if
				else
					{
					RootMatch = 0; // invalidate or transfer below
					} // else
				} // if
			if (RootMatch == 0)
				{
				// didn't get a match, invalidate all Cells within this CacheOctave and write our RootCellID
				int CellInval;

				// Could we possibly win by transferring some existing cells to new positions?
				#ifdef WCS_CELLULARBASIS_CACHE_XFER
				#endif // WCS_CELLULARBASIS_CACHE_XFER

				for (CellInval = 0; CellInval < MaxCells; ++CellInval)
					{
					CBCO[OctaveNum].CellCache[CellInval].Valid = 0;
					//  for debugging
					#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
					CBCO[OctaveNum].CellCache[CellInval].NumPoints = 0;
					#endif // WCS_CELLULARBASIS_CACHE_DEBUG
					} // for
				// we copy all three RootCell coords for ease, even though we might compare fewer later
				CBCO[OctaveNum].RootCellID[0] = RootCell[0];
				CBCO[OctaveNum].RootCellID[1] = RootCell[1];
				CBCO[OctaveNum].RootCellID[2] = RootCell[2];
				CBCO[OctaveNum].CellIDValid = 1;
				RootMatch = 2; // was mismatch, now fully invalidated
				} // if
			if (RootMatch == 1)
				{
				// we may be able to get a cache hit, figure out which cache entry
				// corresponds with the cell we're looking for
				} // else
			} // else

		// determine where in the cache the desired cell might be
		if (Pass == 0)
			{
			CacheSlot = 1; // Center Cell in 1D [0,*1*,2]
			if (Dimensions > 1)
				CacheSlot = 4; // Center Cell in 2D
			if (Dimensions > 2)
				CacheSlot = 13; // Center Cell in 3D
			} // if
		else
			{
			CacheSlot = (1 + CellsToCheck[CellCheck][0]);
			if (Dimensions > 1)
				CacheSlot += (3 * (1 + CellsToCheck[CellCheck][1]));
			if (Dimensions > 2)
				CacheSlot += (9 * (1 + CellsToCheck[CellCheck][2]));
			} // if
		
		// Debug cache testing
		#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
		CellValid = StashNumPoints = 0;
		#endif // WCS_CELLULARBASIS_CACHE_DEBUG

		// Do we need to make new RNGs or do we have the desired cell already en-cache?
		if (CBCO && RootMatch > 0)
			{
			// look in the cache, and see if the entry in that slot is valid.
			if (CBCO[OctaveNum].CellCache[CacheSlot].Valid)
				{
				GenCell = 0;
				NumPtsThisCell = CBCO[OctaveNum].CellCache[CacheSlot].NumPoints;

				// Debug cache testing
				#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
				CellValid = 1;
				StashNumPoints = NumPtsThisCell;
				#endif // WCS_CELLULARBASIS_CACHE_DEBUG
				} // if
			} // if

		// Begin generating the contents of the cell, if required
		if (GenCell)
			{
			//CellBasisRand.Seed64((unsigned long)(Seed[0] + (Seed[2] << 16)), (unsigned long)(Seed[1] + (Seed[3] << 16)));
			//PtDist = CellBasisRand.GenPRN();
			PtDist = CellBasisRand.Seed64GenPRN((unsigned long)(Seed[0] + (Seed[2] << 16)), (unsigned long)(Seed[1] + (Seed[3] << 16)));

			NumPtsThisCell = 0;
			while (PtDist > PtProb[NumPtsThisCell])
				++NumPtsThisCell;

			if (CBCO && RootMatch > 0)
				{ // Store NumPtsThisCell, flag cell as valid since we'll definitely fill it in a moment

				// Debug cache testing
				#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
				if(CellValid)
					{
					assert(StashNumPoints != NumPtsThisCell)
					} // if
				#endif // WCS_CELLULARBASIS_CACHE_DEBUG

				CBCO[OctaveNum].CellCache[CacheSlot].NumPoints = (char)NumPtsThisCell;
				CBCO[OctaveNum].CellCache[CacheSlot].Valid = 1;
				} // if

			CellBasisRand.GenMultiPRN(Dimensions * NumPtsThisCell, PRNBin);
			PrnBinIDX = 0;
			} // if GenCell

		for (Ct = 0; Ct < NumPtsThisCell; ++Ct)
			{
			if (GenCell)
				{
				// generate point
				D = Pt[0] - (ThisPt[0] = (PRNBin[PrnBinIDX++] + EvalCell[0]));
				// Debug cache checking
				#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
				if (CBCO && CellValid)
					assert(CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[0] != ThisPt[0])
				#endif // WCS_CELLULARBASIS_CACHE_DEBUG
				if (CBCO)
					CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[0] = ThisPt[0];
				PtDist = (D * D);
				if (Dimensions > 1)
					{
					D = Pt[1] - (ThisPt[1] = (PRNBin[PrnBinIDX++] + EvalCell[1]));
					PtDist += (D * D);
					// Debug cache checking
					#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
					if (CBCO && CellValid)
						assert(CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[1] != ThisPt[1])
					#endif // WCS_CELLULARBASIS_CACHE_DEBUG
					if (CBCO)
						CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[1] = ThisPt[1];
					if (Dimensions > 2)
						{
						D = Pt[2] - (ThisPt[2] = (PRNBin[PrnBinIDX++] + EvalCell[2]));
						PtDist += (D * D);
						// Debug cache checking
						#ifdef WCS_CELLULARBASIS_CACHE_DEBUG
						if (CBCO && CellValid)
							assert(CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[2] != ThisPt[2])
						#endif // WCS_CELLULARBASIS_CACHE_DEBUG
						if (CBCO)
							CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct].Coords[2] = ThisPt[2];
						if (Dimensions > 3)
							{
							D = Pt[3] - (ThisPt[3] = (PRNBin[PrnBinIDX++] + EvalCell[3]));
							PtDist += (D * D);
							} // if
						} // if
					} // if
				} // if
			else
				{
				// load point from cache
				// generate point
				CBCellPoint *cbcp = &CBCO[OctaveNum].CellCache[CacheSlot].Points[Ct];

				D = Pt[0] - (ThisPt[0] = cbcp->Coords[0]);
				PtDist = (D * D);
				if (Dimensions > 1)
					{
					D = Pt[1] - (ThisPt[1] = cbcp->Coords[1]);
					PtDist += (D * D);
					if (Dimensions > 2)
						{
						D = Pt[2] - (ThisPt[2] = cbcp->Coords[2]);
						PtDist += (D * D);
						if (Dimensions > 3)
							{
							// won't work for 4d, but we shouldn't get here at all for 4d
							//D = Pt[3] - (ThisPt[3] = (PRNBin[PrnBinIDX++] + Cell[3]));
							PtDist += (D * D);
							} // if
						} // if
					} // if
				} // else
			// test against already generated points
			for (TestCt = BaseCt; TestCt < PointsNeeded; ++TestCt)
				{
				if (PtDist < Dist[TestCt])
					{
					for (MoveCt = TestCt + 1; MoveCt < PtsFound && MoveCt < PointsNeeded; ++MoveCt)
						{
						Dist[MoveCt] = Dist[MoveCt - 1];
						} // for
					Dist[TestCt] = PtDist;
					if (ClosePtStash && ! TestCt)
						{
						ClosePtStash[0] = ThisPt[0];
						ClosePtStash[1] = ThisPt[1];
						ClosePtStash[2] = ThisPt[2];
						ClosePtStash[3] = ThisPt[3];
						} // if
					++PtsFound;
					break;
					} // if
				} // for
			} // for
		} // for CheckCell

	// test to see if additional cells must be inspected
	if (Pass == 0)
		{
		NumCellsToCheck = 0;
		for (x = -SearchXrange; x <= SearchXrange; ++x)
			{
			xOffset = x > 0 ? CellDist[0][1] + x - 1.0: x < 0 ? CellDist[0][0] - x - 1.0: 0.0;
			xOffset *= xOffset;
			ClosestEdge =xOffset;
			if (ClosestEdge < Dist[LastPoint])
				{ 
				if (Dimensions > 1)
					{
					for (y = -SearchYrange; y <= SearchYrange; ++y)
						{
						yOffset = y > 0 ? CellDist[1][1] + y - 1.0: y < 0 ? CellDist[1][0] - y - 1.0: 0.0;
						yOffset *= yOffset;
						ClosestEdge = xOffset + yOffset;
						if (ClosestEdge < Dist[LastPoint])
							{
							if (Dimensions > 2)
								{
								for (z = -SearchZrange; z <= SearchZrange; ++z)
									{
									zOffset = z > 0 ? CellDist[2][1] + z - 1.0: z < 0 ? CellDist[2][0] - z - 1.0: 0.0;
									zOffset *= zOffset;
									ClosestEdge = xOffset + yOffset + zOffset;
									if (ClosestEdge < Dist[LastPoint])
										{
										if (Dimensions > 3)
											{
											for (t = -SearchTrange; t <= SearchTrange; ++t)
												{
												tOffset = t > 0 ? CellDist[3][1] + t - 1.0: t < 0 ? CellDist[3][0] - t - 1.0: 0.0;
												tOffset *= tOffset;
												ClosestEdge = xOffset + yOffset + zOffset + tOffset;
												if (ClosestEdge < Dist[LastPoint])
													{
													if (x == 0 && y == 0 && z == 0 && t == 0)
														continue;
													CellsToCheck[NumCellsToCheck][0] = x;
													CellsToCheck[NumCellsToCheck][1] = y;
													CellsToCheck[NumCellsToCheck][2] = z;
													CellsToCheck[NumCellsToCheck][3] = t;
													++NumCellsToCheck;
													} // if
												} // for
											} // if
										else
											{
											if (x == 0 && y == 0 && z == 0)
												continue;
											CellsToCheck[NumCellsToCheck][0] = x;
											CellsToCheck[NumCellsToCheck][1] = y;
											CellsToCheck[NumCellsToCheck][2] = z;
											++NumCellsToCheck;
											} // else
										} // if
									} // for
								} // if
							else
								{
								if (x == 0 && y == 0)
									continue;
								CellsToCheck[NumCellsToCheck][0] = x;
								CellsToCheck[NumCellsToCheck][1] = y;
								++NumCellsToCheck;
								} // else
							} // if
						} // for
					} // if
				else
					{
					if (x == 0)
						continue;
					CellsToCheck[NumCellsToCheck][0] = x;
					++NumCellsToCheck;
					} // else
				} // if
			} // for
		} // if
	} // for Pass

for (Ct = PointsNeeded - 1; Ct >= 0; --Ct)
	{
	if ((Dist[Ct] = sqrt(Dist[Ct])) > 1.0)
		Dist[Ct] = 1.0;
	} // for

//TotalTime += StopHiResTimerSecs();

} // CellularBasis::Evaluate

/*===========================================================================*/

void CellularBasis::EvaluateManhattan(int OctaveNum, double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double PtDist, D, CellDist[4][2], ThisPt[4], xOffset, yOffset, zOffset, tOffset, ClosestEdge;
unsigned long CellCheck, Pass;
long Seed[4], RootCell[4], EvalCell[4], NumPtsThisCell, PtsFound = 0, Ct, TestCt, MoveCt,
	BaseCt = 0, NumCellsToCheck = 1, Dimensions = DimensionsParam, PointsNeeded = PointsNeededParam;
char CellsToCheck[81][4], x, y, z, t, LastPoint, SearchXrange, SearchYrange, SearchZrange, SearchTrange;
int PrnBinIDX;

if (NumArgs > 3)
	{
	InitProbability(Args[3]);
	} // if
if (NumArgs > 2)
	{
	PointsNeeded = quicklongfloor(Args[2]);
	} // if
if (NumArgs > 1)
	{
	Dimensions = quicklongfloor(Args[1]);
	} // if

// initialize distances
for (Ct = 0; Ct < PointsNeeded; ++Ct)
	{
	Dist[Ct] = FLT_MAX;
	} // for
memset(ThisPt, 0, sizeof(ThisPt));

// set initial seed values - need at least one dimension
for (Pass = 0; Pass < 2; ++Pass)
	{
	for (CellCheck = 0; CellCheck < (unsigned long)NumCellsToCheck; ++CellCheck)
		{
		if (Pass > 0)
			{
			EvalCell[0] = RootCell[0] + CellsToCheck[CellCheck][0];
			Seed[0] = EvalCell[0] % USHRT_MAX;
			CellDist[0][0] = Pt[0] - EvalCell[0];
			CellDist[0][1] = 1.0 - CellDist[0][0];
			if (Dimensions > 1)
				{
				EvalCell[1] = RootCell[1] + CellsToCheck[CellCheck][1];
				Seed[1] = EvalCell[1] % USHRT_MAX;
				CellDist[1][0] = Pt[1] - EvalCell[1];
				CellDist[1][1] = 1.0 - CellDist[1][0];
				if (Dimensions > 2)
					{
					EvalCell[2] = RootCell[2] + CellsToCheck[CellCheck][2];
					Seed[2] = EvalCell[2] % USHRT_MAX;
					CellDist[2][0] = Pt[2] - EvalCell[2];
					CellDist[2][1] = 1.0 - CellDist[2][0];
					if (Dimensions > 3)
						{
						EvalCell[3] = RootCell[3] + CellsToCheck[CellCheck][3];
						Seed[3] = EvalCell[3] % USHRT_MAX;
						CellDist[3][0] = Pt[3] - EvalCell[3];
						CellDist[3][1] = 1.0 - CellDist[3][0];
						} // if
					else
						{
						Seed[3] = 0;
						} // else
					} // if
				else
					{
					Seed[3] = Seed[2] = 0;
					} // else
				} // if
			else
				{
				Seed[3] = Seed[2] = Seed[1] = 0;
				} // else
			} // if
		else
			{
			LastPoint = (char)(PointsNeeded - 1);
			EvalCell[0] = RootCell[0] = quicklongfloor(Pt[0]);
			Seed[0] = EvalCell[0] % USHRT_MAX;
			CellDist[0][0] = Pt[0] - EvalCell[0];
			CellDist[0][1] = 1.0 - CellDist[0][0];
			SearchXrange = 1;
			if (Dimensions > 1)
				{
				EvalCell[1] = RootCell[1] = quicklongfloor(Pt[1]);
				Seed[1] = EvalCell[1] % USHRT_MAX;
				CellDist[1][0] = Pt[1] - EvalCell[1];
				CellDist[1][1] = 1.0 - CellDist[1][0];
				SearchYrange = 1;
				if (Dimensions > 2)
					{
					EvalCell[2] = RootCell[2] = quicklongfloor(Pt[2]);
					Seed[2] = EvalCell[2] % USHRT_MAX;
					CellDist[2][0] = Pt[2] - EvalCell[2];
					CellDist[2][1] = 1.0 - CellDist[2][0];
					SearchZrange = 1;
					if (Dimensions > 3)
						{
						EvalCell[3] = RootCell[3] = quicklongfloor(Pt[3]);
						Seed[3] = EvalCell[3] % USHRT_MAX;
						CellDist[3][0] = Pt[3] - EvalCell[3];
						CellDist[3][1] = 1.0 - CellDist[3][0];
						SearchTrange = 1;
						} // if
					else
						{
						Seed[3] = 0;
						SearchTrange = 0;
						} // else
					} // if
				else
					{
					Seed[3] = Seed[2] = 0;
					SearchZrange = 0;
					} // else
				} // if
			else
				{
				Seed[3] = Seed[2] = Seed[1] = 0;
				SearchYrange = 0;
				} // else
			} // else

		//CellBasisRand.Seed64((unsigned long)(Seed[0] + (Seed[2] << 16)), (unsigned long)(Seed[1] + (Seed[3] << 16)));
		//PtDist = CellBasisRand.GenPRN();
		PtDist = CellBasisRand.Seed64GenPRN((unsigned long)(Seed[0] + (Seed[2] << 16)), (unsigned long)(Seed[1] + (Seed[3] << 16)));

		NumPtsThisCell = 0;
		while (PtDist > PtProb[NumPtsThisCell])
			++NumPtsThisCell;

		CellBasisRand.GenMultiPRN(Dimensions * NumPtsThisCell, PRNBin);
		PrnBinIDX = 0;

		for (Ct = 0; Ct < NumPtsThisCell; ++Ct)
			{
			// generate point
			D = Pt[0] - (ThisPt[0] = (PRNBin[PrnBinIDX++] + EvalCell[0]));
			PtDist = fabs(D);
			if (Dimensions > 1)
				{
				D = Pt[1] - (ThisPt[1] = (PRNBin[PrnBinIDX++] + EvalCell[1]));
				D = fabs(D);
				//PtDist = max(PtDist, D);	// this is the Chebychev basis
				PtDist += D;	// this is the Manhattan basis
				if (Dimensions > 2)
					{
					D = Pt[2] - (ThisPt[2] = (PRNBin[PrnBinIDX++] + EvalCell[2]));
					D = fabs(D);
					//PtDist = max(PtDist, D);
					PtDist += D;
					if (Dimensions > 3)
						{
						D = Pt[3] - (ThisPt[3] = (PRNBin[PrnBinIDX++] + EvalCell[3]));
						D = fabs(D);
						//PtDist = max(PtDist, D);
						PtDist += D;
						} // if
					} // if
				} // if
			// test against already generated points
			for (TestCt = BaseCt; TestCt < PointsNeeded; ++TestCt)
				{
				if (PtDist < Dist[TestCt])
					{
					for (MoveCt = TestCt + 1; MoveCt < PtsFound && MoveCt < PointsNeeded; ++MoveCt)
						{
						Dist[MoveCt] = Dist[MoveCt - 1];
						} // for
					Dist[TestCt] = PtDist;
					if (ClosePtStash && ! TestCt)
						{
						ClosePtStash[0] = ThisPt[0];
						ClosePtStash[1] = ThisPt[1];
						ClosePtStash[2] = ThisPt[2];
						ClosePtStash[3] = ThisPt[3];
						} // if
					++PtsFound;
					break;
					} // if
				} // for
			} // for
		} // for CheckCell

	// test to see if additional cells must be inspected
	if (Pass == 0)
		{
		NumCellsToCheck = 0;
		for (x = -SearchXrange; x <= SearchXrange; ++x)
			{
			ClosestEdge = xOffset = x > 0 ? CellDist[0][1] + x - 1.0: x < 0 ? CellDist[0][0] - x - 1.0: 0.0;
			//ClosestEdge = 0.0;	// to enable all surrounding cells
			if (ClosestEdge < Dist[LastPoint])
				{ 
				if (Dimensions > 1)
					{
					for (y = -SearchYrange; y <= SearchYrange; ++y)
						{
						yOffset = y > 0 ? CellDist[1][1] + y - 1.0: y < 0 ? CellDist[1][0] - y - 1.0: 0.0;
						//ClosestEdge = max(ClosestEdge, yOffset);	// this is the Chebychev basis
						ClosestEdge = xOffset + yOffset;	// this is the Manhattan basis
						if (ClosestEdge < Dist[LastPoint])
							{
							if (Dimensions > 2)
								{
								for (z = -SearchZrange; z <= SearchZrange; ++z)
									{
									zOffset = z > 0 ? CellDist[2][1] + z - 1.0: z < 0 ? CellDist[2][0] - z - 1.0: 0.0;
									//ClosestEdge = max(ClosestEdge, zOffset);
									ClosestEdge = xOffset + yOffset + zOffset;
									if (ClosestEdge < Dist[LastPoint])
										{
										if (Dimensions > 3)
											{
											for (t = -SearchTrange; t <= SearchTrange; ++t)
												{
												tOffset = t > 0 ? CellDist[3][1] + t - 1.0: t < 0 ? CellDist[3][0] - t - 1.0: 0.0;
												//ClosestEdge = max(ClosestEdge, tOffset);
												ClosestEdge = xOffset + yOffset + zOffset + tOffset;
												if (ClosestEdge < Dist[LastPoint])
													{
													if (x == 0 && y == 0 && z == 0 && t == 0)
														continue;
													CellsToCheck[NumCellsToCheck][0] = x;
													CellsToCheck[NumCellsToCheck][1] = y;
													CellsToCheck[NumCellsToCheck][2] = z;
													CellsToCheck[NumCellsToCheck][3] = t;
													++NumCellsToCheck;
													} // if
												} // for
											} // if
										else
											{
											if (x == 0 && y == 0 && z == 0)
												continue;
											CellsToCheck[NumCellsToCheck][0] = x;
											CellsToCheck[NumCellsToCheck][1] = y;
											CellsToCheck[NumCellsToCheck][2] = z;
											++NumCellsToCheck;
											} // else
										} // if
									} // for
								} // if
							else
								{
								if (x == 0 && y == 0)
									continue;
								CellsToCheck[NumCellsToCheck][0] = x;
								CellsToCheck[NumCellsToCheck][1] = y;
								++NumCellsToCheck;
								} // else
							} // if
						} // for
					} // if
				else
					{
					if (x == 0)
						continue;
					CellsToCheck[NumCellsToCheck][0] = x;
					++NumCellsToCheck;
					} // else
				} // if
			} // for
		} // if
	} // for Pass
/*
for (Ct = PointsNeeded - 1; Ct >= 0; Ct --)
	{
	//if (Dist[Ct] > 1.0)
	if ((Dist[Ct] = Dist[Ct] / Dimensions) > 1.0)
		Dist[Ct] = 1.0;
	} // for
*/
} // CellularBasis::EvaluateManhattan

/*===========================================================================*/
/*===========================================================================*/

F1CellBasis3::F1CellBasis3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 3
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F1CellBasis3::F1CellBasis3

/*===========================================================================*/

double F1CellBasis3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < quickftol(Octaves); ++Ct)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[0] * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - quickftol(Octaves)) > 0.0)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[0] * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F1CellBasis3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F2CellBasis3::F2CellBasis3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 4
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F2CellBasis3::F2CellBasis3

/*===========================================================================*/

double F2CellBasis3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < quickftol(Octaves); ++Ct)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[1] * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - (int)Octaves) > 0.0)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[1] * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F2CellBasis3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F2mF1CellBasis3::F2mF1CellBasis3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 4
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F2mF1CellBasis3::F2mF1CellBasis3

/*===========================================================================*/

double F2mF1CellBasis3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < Octaves; ++Ct)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[1] - Dist[0]) * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - quickftol(Octaves)) > 0.0)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[1] - Dist[0]) * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F2mF1CellBasis3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F3mF1CellBasis3::F3mF1CellBasis3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 6
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F3mF1CellBasis3::F3mF1CellBasis3

/*===========================================================================*/

double F3mF1CellBasis3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < (int)Octaves; ++Ct)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[2] - Dist[0]) * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - (int)Octaves) > 0.0)
		{
		CellularBasis::Evaluate(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[2] - Dist[0]) * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F3mF1CellBasis3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F1Manhattan3::F1Manhattan3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 3
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F1Manhattan3::F1Manhattan3

/*===========================================================================*/

double F1Manhattan3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < quickftol(Octaves); ++Ct)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[0] * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - (int)Octaves) > 0.0)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[0] * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F1Manhattan3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F2Manhattan3::F2Manhattan3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 4
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F2Manhattan3::F2Manhattan3

/*===========================================================================*/

double F2Manhattan3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < quickftol(Octaves); ++Ct)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[1] * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - (int)Octaves) > 0.0)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += (Dist[1] * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F2Manhattan3::Evaluate

/*===========================================================================*/
/*===========================================================================*/

F2mF1Manhattan3::F2mF1Manhattan3(double *Args, int NumArgs)
: CellularBasis(Args, NumArgs)	// avg pts = 4
{

if (NumArgs > 0)
	OctavesParam = quicklongfloor(Args[0]);
else
	OctavesParam = 1;

} // F2mF1Manhattan3::F2mF1Manhattan3

/*===========================================================================*/

double F2mF1Manhattan3::Evaluate(double *Pt, double *Args, int NumArgs, double ClosePtStash[4])
{
double Remainder, Value = 0.0, Scale = 1.0, MaxVal = 0.0, Octaves = OctavesParam, rVal = 0.0;
long Ct, Dimensions = DimensionsParam;

if (NumArgs > 1)
	Dimensions = quicklongfloor(Args[1]);
if (NumArgs > 0)
	Octaves = Args[0];

if (Octaves > 0.0)
	{
	for (Ct = 0; Ct < Octaves; ++Ct)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[1] - Dist[0]) * Scale);
		MaxVal += (1.0 * Scale);
		Pt[0] *= 2.0;
		Pt[0] += 100;
		if (Dimensions > 1)
			{
			Pt[1] *= 2.0;
			Pt[1] += 100;
			if (Dimensions > 2)
				{
				Pt[2] *= 2.0;
				Pt[2] += 100;
				if (Dimensions > 3)
					{
					Pt[3] *= 2.0;
					Pt[3] += 100;
					} // if
				} // if
			} // if
		Scale *= 0.5;
		} // for
	if ((Remainder = Octaves - (int)Octaves) > 0.0)
		{
		CellularBasis::EvaluateManhattan(Ct, Pt, Args, NumArgs, ClosePtStash);
		Value += ((Dist[1] - Dist[0]) * Scale) * Remainder;
		MaxVal += (1.0 * Scale) * Remainder;
		} // if
	rVal = Value / MaxVal;
	} // if

return(rVal);

} // F2mF1Manhattan3::Evaluate
