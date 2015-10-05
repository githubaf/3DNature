// DEFG.cpp
// Decent Extremely Fast Gridder
// Started August 16th, 2000 by Chris 'Xenon' Hanson
// Copyright 2000, Chris 'Xenon' Hanson and 3D Nature, LLC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h> // semi-accurate timing

#include "DEFG.h"
#include "DEFGSpline.h"

// For API compatability
class NNGrid;

// are we built in VNS or stand-alone?
#if defined(WCS_BUILD_VNS) || defined(WCS_BUILD_W6)
#define DEFG_BUILD_WCSVNS
#include "../AppMem.h"
#include "../Application.h"
#include "../Useful.h"
#include "../NNGrid.h"
#include "../Database.h"
#include "../Joe.h"
#include "../DEM.h"
#include "../Requester.h"
#include "../Render.h"
#include "../EffectsLib.h"
#include "../Log.h"
#include "../MathSupport.h"
#else //  !WCS_BUILD_VNS
#include "DEFGSupport.h"
#endif // !WCS_BUILD_VNS

#ifndef WCS_BUILD_VNS
#define DEFG_LIMIT_INPOINTS				6000
int InPointExcess;
double ProcessAccum = 0;
#endif // !WCS_BUILD_VNS

static double BeginTime, StartTime, EndTime, FinalTime, ElapsedTime;	//lint -e551

#ifdef DEFG_ENABLE_DIAG
static char StatusOut[1204];
#endif // DEFG_ENABLE_DIAG

// starting at VNS3 we no longer watermark the gridder, since we don't offer a WCS that can read VNS3's DEMs
// to prevent competition with
#ifdef WCS_BUILD_DEMO_OLD
// for Demo version watermarking
RootTexture *TexRoot;
float DEMRange;
#endif //WCS_BUILD_DEMO_OLD


// Stolen from WCS's MathSupport.cpp
/*===========================================================================*/
//      0
//      +a
// 270__|__+b 90
//      |
//     180
double DEFGfindangle3(double pta, double ptb)
{
double angle;

if (pta == 0.0)
	{
	angle = ptb > 0.0 ? DEFGHalfPi: DEFGOneAndHalfPi;
	} // if
else
	{
	angle = atan(ptb / pta);
	if (pta < 0.0) angle += DEFGPi;
} // else

return angle;

} // DEFGfindangle3

/*===========================================================================*/
/*===========================================================================*/

void DEFG::ClearDist(float Value)
{
int GX, GY;

// clear dist buffer to FLT_MAX
for (GY = 0; GY < DefgGridHeight; GY++)
	{
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		TempFloat[GY * DefgGridWidth + GX] = Value;
		} // for
	} // for

} // DEFG::ClearDist

/*===========================================================================*/

void DEFG::ClearFinalOutput(void)
{
int GX, GY;

for (GY = 0; GY < DefgGridHeight; GY++)
	{
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		FinalOutput[GY * DefgGridWidth + GX] = -FLT_MAX;
		} // for
	} // for

} // DEFG::ClearFinalOutput

/*===========================================================================*/

void DEFG::ClearEdgeBuf(void)
{
int GX, GY;

for (GY = 0; GY < DefgGridHeight; GY++)
	{
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		EdgeBuf[GY * DefgGridWidth + GX] = -FLT_MAX;
		} // for
	} // for

} // DEFG::ClearEdgeBuf

/*===========================================================================*/

void DEFG::ClearXBuffer(void)
{
int GX, GY;

for (GY = 0; GY < DefgGridHeight; GY++)
	{
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		XCoordBuf[GY * DefgGridWidth + GX] = -DBL_MAX;
		} // for
	} // for

} // DEFG::ClearXBuffer

/*===========================================================================*/

int DEFG::AutoBoundPoints(void)
{
int BoundLoop;

LowX  = LowY  = LowZ  =  DBL_MAX;
HighX = HighY = HighZ = -DBL_MAX;

for (BoundLoop = 0; BoundLoop < LoadedPoints; BoundLoop++)
	{
	if (Smoothing)
		{
		if (LowX  > SmoothedInPoints[BoundLoop].X) LowX  = SmoothedInPoints[BoundLoop].X;
		if (HighX < SmoothedInPoints[BoundLoop].X) HighX = SmoothedInPoints[BoundLoop].X;

		if (LowY  > SmoothedInPoints[BoundLoop].Y) LowY  = SmoothedInPoints[BoundLoop].Y;
		if (HighY < SmoothedInPoints[BoundLoop].Y) HighY = SmoothedInPoints[BoundLoop].Y;

		if (LowZ  > SmoothedInPoints[BoundLoop].Z) LowZ  = SmoothedInPoints[BoundLoop].Z;
		if (HighZ < SmoothedInPoints[BoundLoop].Z) HighZ = SmoothedInPoints[BoundLoop].Z;
		} // if
	else
		{
		if (LowX  > InPoints[BoundLoop].X) LowX  = InPoints[BoundLoop].X;
		if (HighX < InPoints[BoundLoop].X) HighX = InPoints[BoundLoop].X;

		if (LowY  > InPoints[BoundLoop].Y) LowY  = InPoints[BoundLoop].Y;
		if (HighY < InPoints[BoundLoop].Y) HighY = InPoints[BoundLoop].Y;

		if (LowZ  > InPoints[BoundLoop].Z) LowZ  = InPoints[BoundLoop].Z;
		if (HighZ < InPoints[BoundLoop].Z) HighZ = InPoints[BoundLoop].Z;
		} // else
	} // if

XRange = (HighX - LowX);
YRange = (HighY - LowY);

sprintf(StatusOut, "Lat:  %f to %f.\n", -HighY, -LowY); PrintStatus(StatusOut);
sprintf(StatusOut, "Lon:  %f to %f.\n", -HighX, -LowX); PrintStatus(StatusOut);
sprintf(StatusOut, "Elev: %f to %f.\n", HighZ, LowZ); PrintStatus(StatusOut);

return(BoundLoop);

} // DEFG::AutoBoundPoints

/*===========================================================================*/

void DEFG::SetupGridCoordX(int GridSize, double LowCoord, double HighCoord)
{

GridCoordLowX = LowCoord;
GridCoordRangeX = (HighCoord - LowCoord);
GridCoordSizeX = GridSize;
GridCoordSizeXmOneUnderRangeX = (GridCoordRangeX / (GridCoordSizeX - 1));

} // DEFG::SetupGridCoordX

/*===========================================================================*/

void DEFG::SetupGridCoordY(int GridSize, double LowCoord, double HighCoord)
{

GridCoordLowY = LowCoord;
GridCoordRangeY = (HighCoord - LowCoord);
GridCoordSizeY = GridSize;
GridCoordSizeYmOneUnderRangeY = (GridCoordRangeY / (GridCoordSizeY - 1));

} // DEFG::SetupGridCoordY

/*===========================================================================*/

int DEFG::GridFromCoordX(double Coord)
{
double Dif, Frac;
int GridPos;

Dif = (Coord - GridCoordLowX);
Frac = Dif / GridCoordRangeX;

GridPos = (int)((GridCoordSizeX - 1) * Frac);

return(GridPos);

} // DEFG::GridFromCoordX

/*===========================================================================*/

double DEFG::fGridFromCoordX(double Coord)
{
double Dif, Frac, GridPos;

Dif = (Coord - GridCoordLowX);
Frac = Dif / GridCoordRangeX;

GridPos = ((GridCoordSizeX - 1) * Frac);

return(GridPos);

} // DEFG::fGridFromCoordX

/*===========================================================================*/

int DEFG::GridFromCoordY(double Coord)
{
double Dif, Frac;
int GridPos;

Dif = (Coord - GridCoordLowY);
Frac = Dif / GridCoordRangeY;

GridPos = (int)((GridCoordSizeY - 1) * Frac);

return(GridPos);

} // DEFG::GridFromCoordY

/*===========================================================================*/

double DEFG::fGridFromCoordY(double Coord)
{
double Dif, Frac, GridPos;

Dif = (Coord - GridCoordLowY);
Frac = Dif / GridCoordRangeY;

GridPos = ((GridCoordSizeY - 1) * Frac);

return(GridPos);

} // DEFG::fGridFromCoordY

/*===========================================================================*/

int DEFG::GridFromCoord(int GridSize, double LowCoord, double HighCoord, double Coord)
{
double Range, Dif, Frac;
int GridPos;

Range = (HighCoord - LowCoord);
Dif = (Coord - LowCoord);
Frac = Dif / Range;

GridPos = (int)((GridSize - 1) * Frac);
//GridPos = (int)(GridSize * ((Coord - LowCoord) / Range));

return(GridPos);

} // DEFG::GridFromCoord

/*===========================================================================*/

double DEFG::fGridFromCoord(int GridSize, double LowCoord, double HighCoord, double Coord)
{
double Range, Dif, Frac, GridPos;

Range = (HighCoord - LowCoord);
Dif = (Coord - LowCoord);
Frac = Dif / Range;

GridPos = ((GridSize - 1) * Frac);
//GridPos = (int)(GridSize * ((Coord - LowCoord) / Range));

return(GridPos);

} // DEFG::fGridFromCoord

/*===========================================================================*/

double DEFG::CoordFromGrid(int GridSize, double LowCoord, double HighCoord, double Coord)
{
double Range, Result;

Range = (HighCoord - LowCoord);

Result = LowCoord + (Coord * (Range / (GridSize - 1)));

return(Result);

} // DEFG::CoordFromGrid

/*===========================================================================*/

inline double DEFG::CoordFromGridX(double Coord)
{
double Result;

//Result = GridCoordLowX + (Coord * (GridCoordRangeX / (GridCoordSizeX - 1)));
Result = GridCoordLowX + (Coord * GridCoordSizeXmOneUnderRangeX);

return(Result);

} // DEFG::CoordFromGridX

/*===========================================================================*/

inline double DEFG::CoordFromGridY(double Coord)
{
double Result;

//Result = GridCoordLowY + (Coord * (GridCoordRangeY / (GridCoordSizeY - 1)));
Result = GridCoordLowY + (Coord * GridCoordSizeYmOneUnderRangeY);

return(Result);

} // DEFG::CoordFromGridY

/*===========================================================================*/

double DEFG::EvalDistNoSq(double XA, double YA, double XB, double YB)
{
double DX, DY /*, DXs, DYs, DXsDYs */;

DX = (XA - XB);
DY = (YA - YB);

//return(sqrt(DX * DX + DY * DY));
return((DX * DX + DY * DY));

/*DXs = DX * DX;
DYs = DY * DY;
DXsDYs = (DXs + DYs);
return(DXsDYs);
*/

} // DEFG::EvalDistNoSq

/*===========================================================================*/

void DEFG::FlagInterps(void)
{
int GX, GY;

// use TempBuf to indicate which cells need to be extrapolated
memset(TempBuf, 0, DefgGridCells);
memset(TempFloat, 0, DefgGridCells * sizeof(float));
// we'll make you infectious below when we see you have a valid value.
memset(NonInfectious, 1, DefgGridCells);

for (GY = 0; GY < DefgGridHeight; GY++)
	{
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		if (FinalOutput[GY * DefgGridWidth + GX] == -FLT_MAX)
			{
			// we need to be interped, we're not infectious until we are interped
			TempBuf[GY * DefgGridWidth + GX] = 1;
			} // if
		else
			{
			// No need to be interped, but we're infectious
			NonInfectious[GY * DefgGridWidth + GX] = 0;
			} // else
		} // for
	} // for

} // DEFG::FlagInterps

/*===========================================================================*/

int DEFG::InfectCellVal(int GX, int GY, int XOff, unsigned long int Idx, float Weight)
{
//double DBoop;
double NewVal;
unsigned long DestCellIdx, SrcCellIdx;
//unsigned long boop = 0;

DestCellIdx = Idx + (GX + XOff);
SrcCellIdx = GY * DefgGridWidth + GX;

// flagged for extrapolation?
if (TempBuf[DestCellIdx])
	{
	NewVal = (Weight * FinalOutput[SrcCellIdx]);
	if (FinalOutput[SrcCellIdx] != 0.0 && (fabs(NewVal) < FLT_MIN))
		{
		NewVal = 0.0;
		} // if
	if (TempFloat[DestCellIdx] == 0.0)
		{
		FinalOutput[DestCellIdx] = (float)NewVal;
		} // if
	else
		{
		FinalOutput[DestCellIdx] += (float)NewVal;
		} // else
	TempFloat[DestCellIdx] += Weight;
	return(1);
	} // if

return(0);

} // DEFG::InfectCellVal

/*===========================================================================*/

void DEFG::NormalizeExtrap(void)
{
int GX, GY, NormGY, NormIdx;
//int Boop;

for (GY = 0; GY < DefgGridHeight; GY++)
	{
	NormGY = GY * DefgGridWidth;
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
/*
		if (GY == 37 && GX == 38)
			Boop = 1;
*/
		NormIdx = NormGY + GX;
		if (TempBuf[NormIdx] && TempFloat[NormIdx] > 0)
			{
			TempBuf[NormIdx] = 0;
			// complete normalizing
			if (TempFloat[NormIdx] != 0.0)
				{
/*
				if (FinalOutput[NormIdx] == -FLT_MAX)
					{
					Boop = 1;
					} // if
*/
				FinalOutput[NormIdx] /= TempFloat[NormIdx];
				TempFloat[NormIdx] = 0.0f;
				} // if
			// we have a valid value, now we become infectious ourselves
			NonInfectious[NormIdx] = 0;
			} // if
		} // for
	} // for

} // DEFG::NormalizeExtrap

/*===========================================================================*/

int DEFG::InfectCell(int GX, int GY, int XOff, unsigned long int Idx, unsigned long int CenterCellID, double CX, double CY, double RX, double RY)
{
InputPoint *OldPoint = NULL;
float Dist, OldDist;
unsigned long int DestCellIdx;

DestCellIdx = Idx + (GX + XOff);

OldDist = TempFloat[DestCellIdx];

if (ID[DestCellIdx] != CenterCellID)
	{
	if (ID[DestCellIdx])
		{
		OldPoint = (InputPoint *)ID[DestCellIdx];
		} // we're reinfecting
	Dist = (float)EvalDistNoSq(CX, CY, RX, RY);
	if (Dist < OldDist)
		{ // infect cell
		TempFloat[DestCellIdx]     = Dist;
		ID[DestCellIdx]            = CenterCellID;
		NonInfectious[DestCellIdx] = 0; // make our infected target infectuous itself
		return(1);
		} // if
	} // if

return(0);

} // DEFG::InfectCell

/*===========================================================================*/

int DEFG::AddSpanToNode(InputPoint *AddNode, struct Span *AddSpan)
{
void *ReBlock;

if (!AddNode->SpanBlock)
	{
	AddNode->SpanBlock = (struct Span **)calloc(1, sizeof(struct Span *) * DEFG_SPANBLOCK_QUANTA);
	AddNode->MaxSpans  = DEFG_SPANBLOCK_QUANTA;
	AddNode->NumSpans  = 0;
	} // if
if (AddNode->SpanBlock)
	{
	// are we out of available slots?
	if (AddNode->NumSpans + 1 == AddNode->MaxSpans)
		{
		if (ReBlock = realloc(AddNode->SpanBlock, sizeof(struct Span *) * (AddNode->MaxSpans + DEFG_SPANBLOCK_QUANTA)))
			{
			AddNode->SpanBlock = (struct Span **)ReBlock;
			AddNode->MaxSpans += DEFG_SPANBLOCK_QUANTA;
			} // if
		} // if
	// are we good to go?
	if (AddNode->NumSpans + 1 != AddNode->MaxSpans)
		{
		// add a span
		AddNode->SpanBlock[AddNode->NumSpans] = AddSpan;
		AddNode->NumSpans ++;
		return(1); // success
		} // if
	} // if

return(0);

} // DEFG::AddSpanToNode

/*===========================================================================*/

int DEFG::SpanCell(int GX, int GY, int XOff, unsigned long int Idx, InputPoint *CenterCell)
{
InputPoint *ANode, *BNode;
struct Span *Moi;
unsigned long DestCellIdx, CenterCellID, OtherCellID;
signed long SearchSlot;
char FoundA, FoundB;
//Point3d FP /*, LP, NP */;
//int Boop;

DestCellIdx = Idx + (GX + XOff);

CenterCellID = (unsigned long int)CenterCell;
if (ID[DestCellIdx] != CenterCellID)
	{
	OtherCellID  = ID[DestCellIdx];

	if (Smoothing)
		{
		SmoothedEdges[TotalSpans].PointA = simplemin(CenterCellID, OtherCellID);
		SmoothedEdges[TotalSpans].PointB = simplemax(CenterCellID, OtherCellID);
		ANode = (InputPoint *)SmoothedEdges[TotalSpans].PointA;
		BNode = (InputPoint *)SmoothedEdges[TotalSpans].PointB;
		Moi = (struct Span *)&SmoothedEdges[TotalSpans]; // subset of SmoothedSpan
		} // if
	else
		{
		Edges[TotalSpans].PointA = simplemin(CenterCellID, OtherCellID);
		Edges[TotalSpans].PointB = simplemax(CenterCellID, OtherCellID);
		ANode = (InputPoint *)Edges[TotalSpans].PointA;
		BNode = (InputPoint *)Edges[TotalSpans].PointB;

		Moi = &Edges[TotalSpans];
		} // else
	FoundA = FoundB = 0;
	if (ANode->NumSpans && ANode->SpanBlock)
		{
		for (SearchSlot = 0; (SearchSlot < ANode->NumSpans) && (ANode->SpanBlock[SearchSlot]); SearchSlot++)
			{
			if ((ANode->SpanBlock[SearchSlot]->PointA == Moi->PointA) && (ANode->SpanBlock[SearchSlot]->PointB == Moi->PointB))
				{ // we're already linked on this node
				FoundA = 1;
				break;
				} // if
			} // if
		} // if
	if (BNode->NumSpans && BNode->SpanBlock)
		{
		for (SearchSlot = 0; (SearchSlot < BNode->NumSpans) && (BNode->SpanBlock[SearchSlot]); SearchSlot++)
			{
			if ((BNode->SpanBlock[SearchSlot]->PointA == Moi->PointA) && (BNode->SpanBlock[SearchSlot]->PointB == Moi->PointB))
				{ // we're already linked on this node
				FoundB = 1;
				break;
				} // if
			} // if
		} // if
	if (!(FoundA || FoundB))
		{
		AddSpanToNode(ANode, Moi);
		AddSpanToNode(BNode, Moi);
		if (Smoothing)
			{
/*			double fdY, fdX; */
			SmoothedEdges[TotalSpans].OrigConnect = SmoothedEdges[TotalSpans].TrisRef = 0;
/*
			// precalculate slope and normal
			// calculations would get really pissy here if A and B were at the same location
			fdX = fGridFromCoordX(BNode->X) - fGridFromCoordX(ANode->X);
			fdY = fGridFromCoordY(BNode->Y) - fGridFromCoordY(ANode->Y);
			SmoothedEdges[TotalSpans].ABSlope = (int)(DEFGPiUnder180 * DEFGfindangle3(fdY, fdX)); //(atan2(ANode.Y - BNode.Y, ANode.X - BNode.X)
			if (SmoothedEdges[TotalSpans].ABSlope < 0)
				SmoothedEdges[TotalSpans].ABSlope += 360;
			// we make line vector in X & Y pixel coords not original units, since we use it in pixel coords later
			FP[0] = fGridFromCoordX(BNode->X) - fGridFromCoordX(ANode->X);
			FP[1] = fGridFromCoordY(BNode->Y) - fGridFromCoordY(ANode->Y);
			FP[2] = BNode->Z - ANode->Z;
			UnitVector(FP);
*/
/*
			// LP is straight up vector
			LP[0] = 0.0;
			LP[1] = 0.0;
			LP[2] = 1.0;
			// get normal perpendicular to plane containing original line and up vector
			SurfaceNormal(NP, FP, LP);
			// get new "uppish" normal to original line by getting normal
			// of plane comprising original line and the NP vector.
			// We recycle LP as the destination because we don't need it anymore
			SurfaceNormal(LP, FP, NP);
			if (LP[2] < 0.0)
				{
				SmoothedEdges[TotalSpans].Normal[0] = (float)-LP[0];
				SmoothedEdges[TotalSpans].Normal[1] = (float)-LP[1];
				SmoothedEdges[TotalSpans].Normal[2] = (float)-LP[2];
				} // if
			else
				{
				SmoothedEdges[TotalSpans].Normal[0] = (float)LP[0];
				SmoothedEdges[TotalSpans].Normal[1] = (float)LP[1];
				SmoothedEdges[TotalSpans].Normal[2] = (float)LP[2];
				} // else
*/
/*			// record basic normal of line
				SmoothedEdges[TotalSpans].Normal[0] = (float)FP[0];
				SmoothedEdges[TotalSpans].Normal[1] = (float)FP[1];
				SmoothedEdges[TotalSpans].Normal[2] = (float)FP[2];
*/			} // if
		else
			{
			Edges[TotalSpans].OrigConnect = Edges[TotalSpans].TrisRef = 0;
			} // else

		TotalSpans++;
		} // if
	if (FoundA ^ FoundB)
		{
		sprintf(StatusOut, "Strange Error: Span mismatch.\n"); PrintStatus(StatusOut);
		} // if
	return(1);
	} // if

return(0);

} // DEFG::SpanCell

/*===========================================================================*/

void DEFG::SmoothRasterizeSpanElevPoints(InputPoint *IPA, InputPoint *IPB)
{
double LenFrac, DeltaX, DeltaY, DeltaZ, XPos, YPos, ZPos, dZA, dZB, dXA, dXB;
double LineLen, DPos;
double TdX, TSlope, HorizSlopeA, HorizSlopeB, HorizSlope, EdgeSlope, DeltaHS;
Point3d PS, PE, PL, PN, PV, PJ;
SmoothedInputPoint *PA, *PB;
SimpleGraphNode FourNode[4];
int XS, XE, YS, YE, IDeltaX, IDeltaY, LoopStep, IX, IY;
//int Boop;

PA = (SmoothedInputPoint *)IPA;
PB = (SmoothedInputPoint *)IPB;
//RasterizeSpanElevPoints(IPA, IPB); return;

DeltaX = PB->X - PA->X;
DeltaY = PB->Y - PA->Y;
DeltaZ = PB->Z - PA->Z;


XS = GridFromCoordX(PA->X);
XE = GridFromCoordX(PB->X);
YS = GridFromCoordY(PA->Y);
YE = GridFromCoordY(PB->Y);

PS[0] = fGridFromCoordX(PA->X);
PS[1] = fGridFromCoordY(PA->Y);
// set Z=0 to measure XY length of line only
PS[2] = 0.0;

PE[0] = fGridFromCoordX(PB->X);
PE[1] = fGridFromCoordY(PB->Y);
// set Z=0 to measure XY length of line only
PE[2] = 0.0;

// calculate length of line, which becomes length of control vectors
LineLen = PointDistance(PE, PS);

// fill in actual Z values now for normalling
PS[2] = PA->Z;
PE[2] = PB->Z;

// Endpoint A
// determine slope of tangents at Start/End nodes in plane of line
// (reuse PS and PE entities)
PL[0] = PE[0] - PS[0];
PL[1] = PE[1] - PS[1];
PL[2] = PE[2] - PS[2];
UnitVector(PL);
PV[0] = PA->Normal[0];
PV[1] = PA->Normal[1];
PV[2] = PA->Normal[2];
// get sfc normal (PN) perpendicular to Node summed normal (PV) and vector of line (PL)
SurfaceNormal(PN, PV, PL);
// get sfc normal (PE) perpendicular to (PN ^that result^) and original Normal (PV)
SurfaceNormal(PJ, PN, PV);
// result in PJ should be normalized (length=1.0) so Z component can now be interpreted
// as slope of line-wise component of node's summed surface normal.
dZA = PJ[2]; // this is used as 'Y' in the 2D space of the spline, the vertical plane the original edge is contained in 

// need to calculate linewise 'run' component to go with 'rise' component (delta X, A or B)
if (PJ[0] != 0.0 && PJ[1] != 0.0)
	dXA = sqrt(PJ[0] * PJ[0] + PJ[1] * PJ[1]);  // SQRT=EVIL! I wish I could avoid this!
else
	dXA = fabs(PJ[0]) + fabs(PJ[1]);


// Calculate the Y value at X=0 for the line passing through PB with slope dZA/dXA
// This line is parallel to the tangent surface at PA, and thus should never be vertical,
// therefore, dXA should never be 0.
TSlope = dZA / dXA; // tangent slope
TdX = -(LineLen + LineLen); // tangent delta x -- delta X between PB (X=2*LineLen) and first node at X=0

// load coords into spline data structure, where PA is at X=LineLen
// and Node 0 is always at 0
FourNode[0].Distance = 0;
FourNode[0].Value    = PE[2] + TdX * TSlope;
//FourNode[0].Value    = PS[2] - (PE[2] - PS[2]);

// calculate horizontal slope (rise/run of tangent surface where run never = 0)
// at PA for linear interpolation during span rasterization
// HorizSlope can = 0
//HorizSlopeA = dXA / -dZA;
HorizSlopeA = PA->Normal[0] / PA->Normal[2];


// Endpoint B
// determine slope of tangents at Start/End nodes in plane of line
// (reuse PS and PE entities)
// (PL is still valid from above)
//PL[0] = PE[0] - PS[0];
//PL[1] = PE[1] - PS[1];
//PL[2] = PE[2] - PS[2];
//UnitVector(PL);
PV[0] = PB->Normal[0];
PV[1] = PB->Normal[1];
PV[2] = PB->Normal[2];
// get sfc normal (PN) perpendicular to Node summed normal (PV) and vector of line (PL)
SurfaceNormal(PN, PV, PL);
// get sfc normal (PE) perpendicular to (PN ^that result^) and original Normal (PV)
SurfaceNormal(PJ, PN, PV);
// result in PJ should be normalized (length=1.0) so Z component can now be interpreted
// as slope of line-wise component of node's summed surface normal.
dZB = PJ[2]; // this is used as 'Y' in the 2D space of the spline, the vertical plane the original edge is contained in 
// need to calculate linewise 'run' component to go with 'rise' component (delta X, A or B)
if (PJ[0] != 0.0 && PJ[1] != 0.0)
	dXB = sqrt(PJ[0] * PJ[0] + PJ[1] * PJ[1]); // SQRT=EVIL! I wish I could avoid this!
else
	dXB = fabs(PJ[0]) + fabs(PJ[1]);

// Calculate the Y value at X=0 for the line passing through PB with slope dZA/dXA
// This line is parallel to the tangent surface at PA, and thus should never be vertical,
// therefore, dXA should never be 0.
TSlope = dZB / dXB; // tangent slope
TdX = LineLen + LineLen; // tangent delta x -- delta X between PA (X=LineLen) and last node at X=3*LineLen

// load coords into spline data structure, where PB is at X = 2 * LineLen
// and Node 3 is always at 3*LineLen
FourNode[3].Distance = LineLen + LineLen + LineLen;
FourNode[3].Value    = PS[2] + TdX * TSlope;
//FourNode[3].Value    = PE[2] + (PE[2] - PS[2]);

// calculate horizontal slope (rise/run of tangent surface where run never = 0)
// at PB for linear interpolation during span rasterization
// HorizSlope can = 0
//HorizSlopeB = dXB / -dZB;
HorizSlopeB = PB->Normal[0] / PB->Normal[2];


//HorizSlopeB = HorizSlopeA = 0.0;
//FourNode[0].Distance = 0;
//FourNode[0].Value    = PB->Z; // flat at nodes for testing
FourNode[1].Distance = LineLen;
FourNode[1].Value    = PA->Z;
FourNode[2].Distance = LineLen + LineLen;
FourNode[2].Value    = PB->Z;
//FourNode[3].Distance = LineLen + LineLen + LineLen;
//FourNode[3].Value    = PA->Z; // flat at nodes for testing
#ifdef DEFG_SPLINE_LOCALTCB
FourNode[0].TCB[0] = Tension;
FourNode[1].TCB[0] = Tension;
FourNode[2].TCB[0] = Tension;
FourNode[3].TCB[0] = Tension;
#endif // DEFG_SPLINE_LOCALTCB


// number of loop steps
IDeltaY = (YE - YS);
IDeltaX = (XE - XS);

if (IDeltaY != 0)
	{
	DeltaHS = HorizSlopeB - HorizSlopeA;
	// scan more or less vertical
	if (YS < YE) LoopStep = 1;
	else LoopStep = -1;
	IY = YS; // start at endpoint
	while(1)	//lint !e716
		{
		// starting with known grid Y, solve for coord Y, coord X and Z
		YPos = IY;
		LenFrac = ((YS - YPos) / -IDeltaY);
		if (LenFrac >= 0.0 && LenFrac <= 1.0)
			{
			XPos = XS + (LenFrac * IDeltaX);
			//ZPos = PA->Z + (LenFrac * DeltaZ);
			HorizSlope = HorizSlopeA + (LenFrac * DeltaHS);
			// DPos is distance from Node 1 to Node 2, not total distance
			DPos = LenFrac * LineLen;
			ZPos = GetSplineValueNoTCB(DPos, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#ifdef DEFG_SPLINE_LOCALTCB
			//ZPos = GetSplineValueTOnly(DPos, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#else // !DEFG_SPLINE_LOCALTCB
			//ZPos = GetSplineValueGlobalT(DPos, 1.0, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#endif // !DEFG_SPLINE_LOCALTCB

			// determine grid X from coord X
			IX = (int)XPos;

			// plot coord X value into X coord buf at grid X position
			EdgeSlope = SNX[IY * DefgGridWidth + IX];

			XCoordBuf[IY * DefgGridWidth + IX] = CoordFromGridX(XPos);

			// plot horizontal slope value into Surface Normal X buffer for later use during H rasterization
			SNX[IY * DefgGridWidth + IX] = (float)HorizSlope;
			//SNX[IY * DefgGridWidth + IX] = 0.0f;
			// plot Z elev into final output at grid X and Y pos
			//FinalOutput[IY * DefgGridWidth + IX] = (float)ZPos;
			if (fabs(ZPos) < FLT_MIN)
				{
				ZPos = 0.0;
				} // if
			EdgeBuf[IY * DefgGridWidth + IX] = (float)ZPos;
			} // if

		if (IY == YE) break; // after final pixel. Test before loop increment
		IY += LoopStep;
		} // while
	} // if

} // SmoothRasterizeSpanElevPoints

/*===========================================================================*/

void DEFG::RasterizeSpanElevPoints(InputPoint *PA, InputPoint *PB)
{
double LenFrac, DeltaX, DeltaY, DeltaZ, XPos, YPos, ZPos;
int XS, XE, YS, YE, IDeltaX, IDeltaY, LoopStep, IX, IY;
//int Boop;

DeltaX = PB->X - PA->X;
DeltaY = PB->Y - PA->Y;
DeltaZ = PB->Z - PA->Z;


XS = GridFromCoordX(PA->X);
XE = GridFromCoordX(PB->X);
YS = GridFromCoordY(PA->Y);
YE = GridFromCoordY(PB->Y);

// number of loop steps
IDeltaY = (YE - YS);
IDeltaX = (XE - XS);

if (IDeltaY != 0)
	{
	// scan more or less vertical
	if (YS < YE) LoopStep = 1;
	else LoopStep = -1;
	IY = YS; // start at endpoint
	while(1)	//lint !e716
		{
		// starting with known grid Y, solve for coord Y, coord X and Z
		YPos = IY;
		LenFrac = ((YS - YPos) / (double)(-IDeltaY));
		if (LenFrac >= 0.0 && LenFrac <= 1.0)
			{
			XPos = XS + (LenFrac * IDeltaX);
			ZPos = PA->Z + (LenFrac * DeltaZ);

			// determine grid X from coord X
			IX = (int)XPos;

			// plot coord X value into X coord buf at grid X position

			XCoordBuf[IY * DefgGridWidth + IX] = CoordFromGridX(XPos);
			// plot Z elev into final output at grid X and Y pos
			//FinalOutput[IY * DefgGridWidth + IX] = (float)ZPos;
			EdgeBuf[IY * DefgGridWidth + IX] = (float)ZPos;
			} // if

		if (IY == YE) break; // after final pixel. Test before loop increment
		IY += LoopStep;
		} // while
	} // if

} // DEFG::RasterizeSpanElevPoints

/*===========================================================================*/

#define DEFG_TAN_MULT	100.0

void DEFG::RasterizeSpanElevGridHor(int XS, double DXS, int YS, double ZS, int XE, double DXE, float ZE, double SNXS, float SNXE)
{
double DeltaX, DeltaZ, DeltaZStep, ZPos, LZPos, SZPos, LenFrac, XPos;
double LineLen, DPos, TanLen;
//double dZA, dZB, dXA, dXB;
//double TdX, TSlope;
//Point3d PS, PE, PL, PN;
SimpleGraphNode FourNode[4];
int LoopCoord, LoopStep;

//SNXS = SNXE = 1.0;

//int Boop = 0;
/*
if (YS == 37)
	{
	if (XS <= 14 && XE >= 14)
		Boop = 1;
	} // if
*/
DeltaX = DXS - DXE;
DeltaZ = ZS - ZE;

DeltaZStep = DeltaZ / DeltaX;

if (XS < XE) LoopStep = 1;
else LoopStep = -1;
if (Smoothing && SmoothPass == 2)
	{
/*
	if ((fabs(SNXE) > .1) || (fabs(SNXS) > .1))
		{
		Boop = 1;
		} // if
*/
	// calculate length of line, which becomes length of control vectors
	LineLen = fabs(DeltaX);
	TanLen = LineLen * DEFG_TAN_MULT;

	// remember SNXS and SNXE can = 0, because they are normalized rise component for run = 1.0
	//dZA = SNXS; // this is used as 'Y' in the 2D space of the spline, the vertical plane the original edge is contained in 
	//dXA = 1.0;

	// Calculate the Y value at X=0 for the line passing through PB with slope dZA/dXA
	// This line is parallel to the tangent surface at PA, and thus should never be vertical,
	// therefore, dXA should never be 0.
	//TdX = (-(LineLen + LineLen)); // tangent delta x -- delta X between PB (X=2*LineLen) and first node at X=0

	// load coords into spline data structure, where PA is at X=LineLen
	FourNode[0].Distance = 0;
	FourNode[0].Value    = ZE + (-(LineLen + TanLen)) * SNXS;

	// remember SNXS and SNXE can = 0, because they are normalized rise component for run = 1.0
	//dZB = SNXE; // this is used as 'Y' in the 2D space of the spline, the vertical plane the original edge is contained in 
	//dXB = 1.0;

	// Calculate the Y value at X=0 for the line passing through PB with slope dZA/dXA
	// This line is parallel to the tangent surface at PA, and thus should never be vertical,
	// therefore, dXA should never be 0.
	//TdX = LineLen + LineLen; // tangent delta x -- delta X between PA (X=LineLen) and last node at X=3*LineLen

	// load coords into spline data structure, where PB is at X = 2 * LineLen
	FourNode[3].Distance = TanLen + LineLen + TanLen;
	FourNode[3].Value    = ZS + (TanLen + LineLen) * SNXE;

	//FourNode[0].Distance = 0;
	//FourNode[0].Value    = ZE; // flat at nodes for testing
	FourNode[1].Distance = TanLen;
	FourNode[1].Value    = ZS;
	FourNode[2].Distance = TanLen + LineLen;
	FourNode[2].Value    = ZE;
	//FourNode[3].Distance = LineLen + LineLen + LineLen;
	//FourNode[3].Value    = ZS; // flat at nodes for testing
	#ifdef DEFG_SPLINE_LOCALTCB
	FourNode[0].TCB[0] = Tension;
	FourNode[1].TCB[0] = Tension;
	FourNode[2].TCB[0] = Tension;
	FourNode[3].TCB[0] = Tension;
	#endif // DEFG_SPLINE_LOCALTCB
	} // if smoothing

LoopCoord = XS;
while(1)	//lint !e716
	{
	XPos = CoordFromGridX((double)LoopCoord);
	LenFrac = ((DXS - XPos) / DeltaX);
	if (LenFrac >= 0.0 && LenFrac <= 1.0)
		{
		// we need linear value either way
		LZPos = ZS - (LenFrac * DeltaZ);
		//if (Smoothing && SmoothPass == 2)
		if (0)
			{
			// DPos is distance from Node 1 to Node 2, not total distance
			DPos = LenFrac * LineLen;
			SZPos = GetSplineValueNoTCB(DPos, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#ifdef DEFG_SPLINE_LOCALTCB
			//ZPos = GetSplineValueTOnly(DPos, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#else // !DEFG_SPLINE_LOCALTCB
			//ZPos = GetSplineValueGlobalT(DPos, 1.0, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#endif // !DEFG_SPLINE_LOCALTCB
			ZPos = SSmoothFrac * SZPos + LSmoothFrac * LZPos;
			} // if
		else
			{
			ZPos = LZPos;
			} // else
		// debugging range checks
		/*if (ZPos > simplemax(ZS, ZE) || ZPos < simplemin(ZS, ZE))
			Boop = 1;
		if (XPos > simplemax(DXS, DXE) || XPos < simplemin(DXS, DXE))
			Boop = 1;
		*/
/*
		if (ZPos < -9999)
			Boop = 1;
*/
		if (fabs(ZPos) < FLT_MIN)
			{
			ZPos = 0.0;
			} // if
		FinalOutput[YS * DefgGridWidth + LoopCoord] = (float)ZPos;
		} // if
	if (LoopCoord == XE) break; // before writing final pixel. Test before loop increment
	LoopCoord += LoopStep;
	} // for

} // DEFG::RasterizeSpanElevGridHor

/*===========================================================================*/

void DEFG::RasterizeSpanElev(struct Span *RasterSpan)
{
InputPoint *PA, *PB;

PA = (InputPoint *)RasterSpan->PointA;
PB = (InputPoint *)RasterSpan->PointB;

RasterizeSpanElevPoints(PA, PB);
} // DEFG::RasterizeSpanElev

/*===========================================================================*/

void DEFG::SmoothRasterizeSpanElev(struct SmoothedSpan *RasterSpan)
{
InputPoint *PA, *PB;

PA = (InputPoint *)RasterSpan->PointA;
PB = (InputPoint *)RasterSpan->PointB;

SmoothRasterizeSpanElevPoints(PA, PB);
} // DEFG::SmoothRasterizeSpanElev

/*===========================================================================*/

void DEFG::CalcSpanNormals(struct SmoothedSpan *RasterSpan)
{
InputPoint *PA, *PB;

PA = (InputPoint *)RasterSpan->PointA;
PB = (InputPoint *)RasterSpan->PointB;

CalcSpanNormalsPoints(PA, PB);
} // DEFG::CalcSpanNormals

/*===========================================================================*/

void DEFG::CalcSpanNormalsPoints(InputPoint *IPA, InputPoint *IPB)
{
double LenFrac, DeltaX, DeltaY, XPos, YPos;
double LSlope, RSlope, EdgeSlope;
double CenterZ, LeftZ, RightZ, SumCount;
SmoothedInputPoint *PA, *PB;
int XS, XE, YS, YE, IDeltaX, IDeltaY, LoopStep, IX, IY;
//int Boop;

PA = (SmoothedInputPoint *)IPA;
PB = (SmoothedInputPoint *)IPB;

DeltaX = PB->X - PA->X;
DeltaY = PB->Y - PA->Y;
//DeltaZ = PB->Z - PA->Z;

XS = GridFromCoordX(PA->X);
XE = GridFromCoordX(PB->X);
YS = GridFromCoordY(PA->Y);
YE = GridFromCoordY(PB->Y);

// number of loop steps
IDeltaY = (YE - YS);
IDeltaX = (XE - XS);

if (IDeltaY != 0)
	{
	// scan more or less vertical
	if (YS < YE) LoopStep = 1;
	else LoopStep = -1;
	IY = YS; // start at endpoint
	while(1)	//lint !e716
		{
		EdgeSlope = SumCount = LSlope = RSlope = 0.0;
		LeftZ = RightZ = -FLT_MAX;
		// starting with known grid Y, solve for coord Y, coord X and Z
		YPos = IY;
		LenFrac = ((YS - YPos) / (double)(-IDeltaY));
		if (LenFrac >= 0.0 && LenFrac <= 1.0)
			{
			XPos = XS + (LenFrac * (double)IDeltaX);
			// determine grid X from coord X
			IX = (int)(XPos);
/*
			if (IY == 5 && IX == 79)
				{
				Boop = 1;
				} // if
*/
			CenterZ = FinalOutput[IY * DefgGridWidth + IX];
			if (CenterZ != -FLT_MAX)
				{
				if (IX > 0)
					{
					LeftZ   = FinalOutput[IY * DefgGridWidth + (IX - 1)];
					} // if
				if (IX < DefgGridWidth - 1)
					{
					RightZ  = FinalOutput[IY * DefgGridWidth + (IX + 1)];
					} // if
				if (LeftZ != -FLT_MAX)
					{
					LSlope = CenterZ - LeftZ;
					SumCount++;
					} // if
				if (RightZ != -FLT_MAX)
					{
					RSlope = RightZ - CenterZ;
					SumCount++;
					} // if
				} // if center value valid
	
			if (SumCount > 0)
				{
				EdgeSlope = (LSlope + RSlope) / SumCount;
				} // if

			// plot coord X value into X coord buf at grid X position
			//XCoordBuf[IY * DefgGridWidth + IX] = XPos;

			// plot horizontal slope value into Surface Normal X buffer for later use during H rasterization
/*
			if (EdgeSlope > 100000)
				{
				Boop = 1;
				} // if
*/
			SNX[IY * DefgGridWidth + IX] = (float)EdgeSlope;
			// plot Z elev into final output at grid X and Y pos
			//FinalOutput[IY * DefgGridWidth + IX] = (float)ZPos;
			//EdgeBuf[IY * DefgGridWidth + IX] = (float)ZPos;
			} // if

		if (IY == YE) break; // after final pixel. Test before loop increment
		IY += LoopStep;
		} // while
	} // if

} // CalcSpanNormalsPoints

/*===========================================================================*/

int DEFG::Grid(void)
{
double CX, CY, RY, RX;
int Abort = 0, Loop, GX, GY, XOff, YOff;
//int SpanSearchA, SpanSearchB;
unsigned long int GYm1Idx, GY0Idx, GYp1Idx, GYIdx, Counter, Iterations, CenterCellID, DisabledPoints = 0, UsablePoints = 0;
InputPoint *WorkPoint;
#ifdef DEFG_BUILD_WCSVNS
BusyWin *BWRB = NULL;
int PointDisabled;
//int Boop;
#endif // DEFG_BUILD_WCSVNS
#ifdef WCS_BUILD_DEMO_OLD
TextureData TexData;
VertexDEM Vert;
#endif // WCS_BUILD_DEMO_OLD


//sprintf(StatusOut, "Beginning processing.\n"); PrintStatus(StatusOut);

if (!AllocRasterBufs(DefgGridWidth, DefgGridHeight))
	return(0);


BeginTime = StartTime = GetSystemTimeFP();

SetupGridCoordX(DefgGridWidth,  LowX, HighX);
SetupGridCoordY(DefgGridHeight, LowY, HighY);

memset(ID, 0, DefgGridCells * sizeof(unsigned long int));
ClearDist(0.0f);
ClearEdgeBuf();
ClearXBuffer();
// <<<>>> remove when done debugging
memset(TempBuf, 0, DefgGridCells);

//sprintf(StatusOut, "Merging points.\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
BWRB = new BusyWin("Merging Points", LoadedPoints, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS
for (Loop = 0; Loop < LoadedPoints; Loop++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(Loop))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	if (Smoothing)
		{
		GX = GridFromCoordX(SmoothedInPoints[Loop].X);
		GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
		} // if
	else
		{
		GX = GridFromCoordX(InPoints[Loop].X);
		GY = GridFromCoordY(InPoints[Loop].Y);
		} // else
	// point averaging here.
	if (ID[GY * DefgGridWidth + GX] == 0)
		{
		if (Smoothing)
			{
			ID[GY * DefgGridWidth + GX] = (unsigned long int)&SmoothedInPoints[Loop];
			} // if
		else
			{
			ID[GY * DefgGridWidth + GX] = (unsigned long int)&InPoints[Loop];
			} // else
		TempFloat[GY * DefgGridWidth + GX] = 1.0f;
		} // if
	else
		{
		WorkPoint = (InputPoint *)ID[GY * DefgGridWidth + GX];
		if (Smoothing)
			{
			WorkPoint->X += SmoothedInPoints[Loop].X;
			WorkPoint->Y += SmoothedInPoints[Loop].Y;
			WorkPoint->Z += SmoothedInPoints[Loop].Z;
			SmoothedInPoints[Loop].Disabled = 1;
			} // if
		else
			{
			WorkPoint->X += InPoints[Loop].X;
			WorkPoint->Y += InPoints[Loop].Y;
			WorkPoint->Z += InPoints[Loop].Z;
			InPoints[Loop].Disabled = 1;
			} // else
		TempFloat[GY * DefgGridWidth + GX] += 1.0f;
		DisabledPoints++;
		} // else
	} // for


//sprintf(StatusOut, "Normalizing merged points.\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Normalizing Merged Points", DefgGridHeight, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS
// normalize to complete averaging
for (GY = 0; GY < DefgGridHeight; GY++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(GY))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		if (TempFloat[GY * DefgGridWidth + GX] > 1.0)
			{
			WorkPoint = (InputPoint *)ID[GY * DefgGridWidth + GX];
			WorkPoint->X /= TempFloat[GY * DefgGridWidth + GX];
			WorkPoint->Y /= TempFloat[GY * DefgGridWidth + GX];
			WorkPoint->Z /= TempFloat[GY * DefgGridWidth + GX];
			} // if
		} // for
	} // for


// re-rasterize with conflict-resolved pointset only
memset(ID,  0, DefgGridCells * sizeof(unsigned long int));
memset(NonInfectious, 0, DefgGridCells);

ClearDist(FLT_MAX);


// Plot cardinal points

//DumpPoints("c:\\tmp\\merged.xyz");

//sprintf(StatusOut, "Processing merged points.\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Processing Merged Points", LoadedPoints, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS
for (Loop = 0; Loop < LoadedPoints; Loop++)
	{
/*
	if (Loop == 10435)
		{
		Boop = 1;
		} // if
*/
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(Loop))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
#ifdef DEFG_LIMIT_INPOINTS
	if (InPointExcess > 0)
		{
		for (int ProcessWorkCounter = 0; ProcessWorkCounter < InPointExcess; ProcessWorkCounter++)
			{
			ProcessAccum += sqrt((double)(ProcessWorkCounter & 0xff));
			ProcessAccum = fmod(ProcessAccum, 100.0);
			} // for
		} // if
#endif // DEFG_LIMIT_INPOINTS

	if (Smoothing)
		{
		PointDisabled = SmoothedInPoints[Loop].Disabled;
		} // if
	else
		{
		PointDisabled = InPoints[Loop].Disabled;
		} // else
	if (!PointDisabled)
		{
		UsablePoints++;
		if (Smoothing)
			{
			GX = GridFromCoordX(SmoothedInPoints[Loop].X);
			GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
			ID[GY * DefgGridWidth + GX] = (unsigned long int)&SmoothedInPoints[Loop];
			//FinalOutput[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
			EdgeBuf[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
/*
			if (GY == 37 && GX == 13)
				Boop = 1;
*/
			XCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].X;
			//YCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].Y;
			} // if
		else
			{
			GX = GridFromCoordX(InPoints[Loop].X);
			GY = GridFromCoordY(InPoints[Loop].Y);
			ID[GY * DefgGridWidth + GX] = (unsigned long int)&InPoints[Loop];
			//FinalOutput[GY * DefgGridWidth + GX] = (float)InPoints[Loop].Z;
			EdgeBuf[GY * DefgGridWidth + GX] = (float)InPoints[Loop].Z;
/*
			if (GY == 37 && GX == 13)
				Boop = 1;
*/
			XCoordBuf[GY * DefgGridWidth + GX] = InPoints[Loop].X;
			//YCoordBuf[GY * DefgGridWidth + GX] = InPoints[Loop].Y;
			} // else
		TempFloat[GY * DefgGridWidth + GX] = 0.0f;
		} // if
	} // for


// allocate span storage now that we have a better idea of how many
// points (and therefore spans) we really need space for

if (!InitSpans(UsablePoints * DEFG_SPAN_MULT)) return(0);

//DumpID("c:\\tmp\\startdump.raw");
//CreateDumpEdgeBuf("c:\\tmp\\CardinalPoints.raw");

//sprintf(StatusOut, "Processing network.\n"); PrintStatus(StatusOut);

// iterate convolution-kernal trick

#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Processing Network", 1 + (int)sqrt((double)(DefgGridWidth * DefgGridWidth + DefgGridHeight * DefgGridHeight)), 'BWSE', 0); //lint !e790
#endif // DEFG_BUILD_WCSVNS
Iterations = 0;
do
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(Iterations))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	Counter = 0;
	for (GY = 0; GY < DefgGridHeight; GY++)
		{
		GYm1Idx = (GY - 1) * DefgGridWidth;
		GY0Idx  = (GY + 0) * DefgGridWidth;
		GYp1Idx = (GY + 1) * DefgGridWidth;
		for (GX = 0; GX < DefgGridWidth; GX++)
			{
#ifdef DEFG_LIMIT_INPOINTS
	if (InPointExcess > 0)
		{
		for (int ProcessWorkCounter = 0; ProcessWorkCounter < InPointExcess; ProcessWorkCounter++)
			{
			ProcessAccum += sqrt((double)(ProcessWorkCounter & 0xff));
			ProcessAccum = fmod(ProcessAccum, 100.0);
			} // for
		} // if
#endif // DEFG_LIMIT_INPOINTS

			// <<<>>> OPT
			//unsigned long int GY0IdxPlusGX;
			//GY0IdxPlusGX = GY0Idx + GX;
			//if ((CenterCellID = ID[GY0IdxPlusGX]) && (!NonInfectious[GY0IdxPlusGX])) // empty cells can't spread virally
			if ((CenterCellID = ID[GY0Idx + GX]) && (!NonInfectious[GY0Idx + GX])) // empty cells can't spread virally
				{
				WorkPoint = (InputPoint *)CenterCellID;
				CX = WorkPoint->X;
				CY = WorkPoint->Y;
				if (GY > 0)
					{ // look up one line
					YOff  = -1;
					GYIdx = GYm1Idx;
					RY    = CoordFromGridY((double)(GY + YOff));
					if (GX > 0)
						{ // look left
						XOff      = -1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					if (GX < DefgGridWidth - 1)
						{ // look right
						XOff      = 1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					// always look above on your column
					if (1) // preserve consistant indent
						{
						XOff      = 0;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					} // if
				if (GY < DefgGridHeight - 1)
					{ // look down one line
					YOff  = 1;
					GYIdx = GYp1Idx;
					RY    = CoordFromGridY((double)(GY + YOff));
					if (GX > 0)
						{ // look left
						XOff      = -1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					if (GX < DefgGridWidth - 1)
						{ // look right
						XOff      = 1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					// always look below on your column
					if (1) // preserve consistant indent
						{
						XOff      = 0;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					} // if
				if (1) // preserve consistant indent
					{ // always try left/right on your own line
					YOff  = 0;
					GYIdx = GY0Idx;
					RY    = CoordFromGridY((double)(GY + YOff));
					if (GX > 0)
						{ // look left
						XOff      = -1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					if (GX < DefgGridWidth - 1)
						{ // look right
						XOff      = 1;
						RX        = CoordFromGridX((double)(GX + XOff));
						Counter  += InfectCell(GX, GY, XOff, GYIdx, CenterCellID, CX, CY, RX, RY);
						} // if
					} // if
				NonInfectious[GY0Idx + GX] = 1;
				// <<<>>> OPT
				//NonInfectious[GY0IdxPlusGX] = 1;
				} // if
			} // for
		} // for
	Iterations++;

	//DumpID("c:\\tmp\\dump.raw");

	} while (Counter);


EndTime = GetSystemTimeFP();
ElapsedTime = (EndTime - StartTime);

//sprintf(StatusOut, "Total Points processed: %d.\n", (LoadedPoints - DisabledPoints)); PrintStatus(StatusOut);
//sprintf(StatusOut, "Total Iterations: %d.\n", Iterations); PrintStatus(StatusOut);
//sprintf(StatusOut, "Elapsed NN computation time: %f seconds.\n", ElapsedTime); PrintStatus(StatusOut);

//DumpID("c:\\tmp\\ID.raw");
//CreateDumpTempDist("c:\\tmp\\dist.raw");


// Create Span Lists

//sprintf(StatusOut, "Spanning Cells...\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Spanning Cells", DefgGridHeight, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS

StartTime = GetSystemTimeFP();

for (GY = 0; GY < DefgGridHeight; GY++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(GY))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS

#ifdef DEFG_LIMIT_INPOINTS
	if (InPointExcess > 0)
		{
		for (int ProcessWorkCounter = 0; ProcessWorkCounter < InPointExcess; ProcessWorkCounter++)
			{
			ProcessAccum += sqrt((double)(ProcessWorkCounter & 0xff));
			ProcessAccum = fmod(ProcessAccum, 100.0);
			} // for
		} // if
#endif // DEFG_LIMIT_INPOINTS

	GY0Idx  = (GY + 0) * DefgGridWidth;
	GYp1Idx = (GY + 1) * DefgGridWidth;
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		CenterCellID = ID[GY0Idx + GX];
		WorkPoint = (InputPoint *)CenterCellID;
		GYIdx = GY0Idx;
		if (GX < DefgGridWidth - 1)
			{ // UL to UR
			SpanCell(GX, GY, 1, GYIdx, WorkPoint);
			} // else if
		if (GY < DefgGridHeight - 1)
			{
			GYIdx = GYp1Idx;
			// UL to LL
			SpanCell(GX, GY, 0, GYIdx, WorkPoint);
			if (GX < DefgGridWidth - 1)
				{
				// Check UL to LR
				if (!SpanCell(GX, GY, 1, GYIdx, WorkPoint))
					{ // need to ensure we don't do double diagonal crossover
					// Check LL to UR (note: we reassign GYIdx and WorkPoint/CenterCellID!)
					GYIdx        = GY0Idx;
					CenterCellID = ID[GYp1Idx + GX];
					WorkPoint    = (InputPoint *)CenterCellID;
					SpanCell(GX, GY, 1, GYIdx, WorkPoint);
					} // if
				} // if
			} // else if
		} // for
	} // for

EndTime = GetSystemTimeFP();
ElapsedTime = (EndTime - StartTime);

//sprintf(StatusOut, "Total Spans created: %d.\n", TotalSpans); PrintStatus(StatusOut);
//sprintf(StatusOut, "Elapsed Span computation time: %f seconds.\n", ElapsedTime); PrintStatus(StatusOut);

//DumpEdges(DEFG_OUTFILE);


//CreateDumpTempElev("c:\\tmp\\A.raw");


//sprintf(StatusOut, "Computing Span and Node normals.\n"); PrintStatus(StatusOut);

// ID buf now becomes FinalOutput from now on
ClearFinalOutput();


//sprintf(StatusOut, "Rasterizing Spans\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Rasterizing Spans", TotalSpans, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS
for (Loop = 0; Loop < TotalSpans; Loop++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(Loop))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	if (Smoothing)
		{
/*
		if (Loop == 15)
			Boop = 1;
*/
		RasterizeSpanElev((struct Span *)&SmoothedEdges[Loop]);
		} // if
	else
		{
		RasterizeSpanElev(&Edges[Loop]);
		} // else
	} // for

//DumpElev("c:\\tmp\\LinearElevSpan.raw");
//CreateDumpEdgeBuf("c:\\tmp\\LinearElevSpanChar.raw");


//sprintf(StatusOut, "Horizontal Linear Polygon scan-filling..\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Polygon Filling", DefgGridHeight, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS

for (GY = 0; GY < DefgGridHeight; GY++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(GY))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	int FoundPointA = 0, TPAX, TPAY;
	double TPAZ, DTPAX;
	GY0Idx  = GY * DefgGridWidth;
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		if (EdgeBuf[GY0Idx + GX] != -FLT_MAX)
			{
			if (FoundPointA)
				{ // must be a point B
				if (GX - TPAX) //lint !e530 // don't scanfill if there's no space between.
					{
					RasterizeSpanElevGridHor(TPAX, DTPAX, TPAY, TPAZ, GX, XCoordBuf[GY0Idx + GX], EdgeBuf[GY0Idx + GX], 0.0, 0.0f); //lint !e530 
					} // if
				TPAX = GX;
				DTPAX = XCoordBuf[GY0Idx + GX];
				TPAY = GY;
				TPAZ = EdgeBuf[GY0Idx + GX];
				} // if
			else
				{
				// transfer current location into TPA for next operation
				TPAX = GX;
				DTPAX = XCoordBuf[GY0Idx + GX];
				TPAY = GY;
				TPAZ = EdgeBuf[GY0Idx + GX];
				} // else
			FoundPointA = 1;
			} // if
		} // for
	} // for


// begin smoothing process
if (Smoothing)
	{
	SmoothPass = 1;

	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Processing Smoothed Points", LoadedPoints, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < LoadedPoints; Loop++)
		{
	#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
	#endif // DEFG_BUILD_WCSVNS
		PointDisabled = SmoothedInPoints[Loop].Disabled;
		if (!PointDisabled)
			{
			GX = GridFromCoordX(SmoothedInPoints[Loop].X);
			GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
			//ID[GY * DefgGridWidth + GX] = (unsigned long int)&SmoothedInPoints[Loop];
			FinalOutput[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
			//EdgeBuf[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
			//XCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].X;
			//YCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].Y;
			//TempFloat[GY * DefgGridWidth + GX] = 0.0f;
			} // if
		} // for
	} // if

// examine each original point and assemble a new surface normal from
// that point's relationship with the (up to) eight surrounding points.

if (Smoothing)
	{
	Point3d CurNorm, NodeNorm, OffsetPoint, MiddlePoint;
	int XOff, YOff, MXOff, MYOff;
	double CenterZ, OffsetZ, MidZ, SumCount;
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Smoothing Nodes", LoadedPoints, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < LoadedPoints; Loop++)
		{
		NodeNorm[0] = NodeNorm[1] = NodeNorm[2] = 0.0;
/*
		if (Loop == 27108)
			Boop = 1;
*/
	#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
	#endif // DEFG_BUILD_WCSVNS
		PointDisabled = SmoothedInPoints[Loop].Disabled;
		if (!PointDisabled)
			{
			SumCount = 0.0;
			GX = GridFromCoordX(SmoothedInPoints[Loop].X);
			GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
			CenterZ = FinalOutput[GY * DefgGridWidth + GX];
/*
			if (CenterZ == -FLT_MAX)
				{
				Boop = 1;
				} // if
*/
			// column to the left, 1 above, 2 below
			if (GX > 0)
				{
				MXOff = XOff = -1;
				MYOff = 0;
				MidZ = FinalOutput[(GY + MYOff) * DefgGridWidth + (GX + MXOff)];
				if (MidZ != -FLT_MAX)
					{
					MiddlePoint[0] = MXOff;
					MiddlePoint[1] = MYOff;
					MiddlePoint[2] = MidZ - CenterZ;
					UnitVector(MiddlePoint);
					// above row (1)
					if (GY > 0)
						{
						YOff = -1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, MiddlePoint, OffsetPoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					// below row (2)
					if (GY < DefgGridHeight - 1)
						{
						YOff = 1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, MiddlePoint, OffsetPoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					} // if
				} // if

			// column to the right, 3 above, 4 below
			if (GX < DefgGridWidth - 1)
				{
				MXOff = XOff = 1;
				MYOff = 0;
				MidZ = FinalOutput[(GY + MYOff) * DefgGridWidth + (GX + MXOff)];
				if (MidZ != -FLT_MAX)
					{
					MiddlePoint[0] = MXOff;
					MiddlePoint[1] = MYOff;
					MiddlePoint[2] = MidZ - CenterZ;
					UnitVector(MiddlePoint);
					// above row (3)
					if (GY > 0)
						{
						YOff = -1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, MiddlePoint, OffsetPoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					// below row (4)
					if (GY < DefgGridHeight - 1)
						{
						YOff = 1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, MiddlePoint, OffsetPoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					} // if
				} // if


			// row above, 5 left, 6 right
			if (GY > 0)
				{
				MYOff = YOff = -1;
				MXOff = 0;
				MidZ = FinalOutput[(GY + MYOff) * DefgGridWidth + (GX + MXOff)];
				if (MidZ != -FLT_MAX)
					{
					MiddlePoint[0] = MXOff;
					MiddlePoint[1] = MYOff;
					MiddlePoint[2] = MidZ - CenterZ;
					UnitVector(MiddlePoint);
					// left (5)
					if (GX > 0)
						{
						XOff = -1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, OffsetPoint, MiddlePoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					// right (6)
					if (GX < DefgGridWidth - 1)
						{
						XOff = 1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, OffsetPoint, MiddlePoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					} // if
				} // if


			// row below, 7 left, 8 right
			if (GY < DefgGridHeight - 1)
				{
				MYOff = YOff = 1;
				MXOff = 0;
				MidZ = FinalOutput[(GY + MYOff) * DefgGridWidth + (GX + MXOff)];
				if (MidZ != -FLT_MAX)
					{
					MiddlePoint[0] = MXOff;
					MiddlePoint[1] = MYOff;
					MiddlePoint[2] = MidZ - CenterZ;
					// left (7)
					if (GX > 0)
						{
						XOff = -1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, OffsetPoint, MiddlePoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					// right (8)
					if (GX < DefgGridWidth - 1)
						{
						XOff = 1;
						OffsetZ = FinalOutput[(GY + YOff) * DefgGridWidth + (GX + XOff)];
						if (OffsetZ != -FLT_MAX)
							{
							OffsetPoint[0] = YOff;
							OffsetPoint[1] = XOff;
							OffsetPoint[2] = OffsetZ - CenterZ;
							UnitVector(OffsetPoint);
							SurfaceNormal(CurNorm, OffsetPoint, MiddlePoint);
							if (CurNorm[2] < 0)
								NegateVector(CurNorm);
							AddPoint3d(NodeNorm, CurNorm);
							SumCount++;
							} // if
						} // if
					} // if
				} // if

			if (SumCount > 0 && (NodeNorm[0] != 0.0 || NodeNorm[1] != 0.0 || NodeNorm[2] != 0.0))
			//if (0)
				{
				//double CheckLen;
				Point3d Fixup;
				Fixup[0] = (NodeNorm[0] / SumCount);
				Fixup[1] = (NodeNorm[1] / SumCount);
				Fixup[2] = (NodeNorm[2] / SumCount);

				// deal with float underflow
				if (fabs(Fixup[0]) < FLT_MIN)
					{
					if (Fixup[0] < 0)
						{
						Fixup[0] = -FLT_MIN;
						} // if
					else
						{
						Fixup[0] = FLT_MIN;
						} // else
					} // if
				if (fabs(Fixup[1]) < FLT_MIN)
					{
					if (Fixup[1] < 0)
						{
						Fixup[1] = -FLT_MIN;
						} // if
					else
						{
						Fixup[1] = FLT_MIN;
						} // else
					} // if
				if (fabs(Fixup[2]) < FLT_MIN)
					{
					if (Fixup[2] < 0)
						{
						Fixup[2] = -FLT_MIN;
						} // if
					else
						{
						Fixup[2] = FLT_MIN;
						} // else
					} // if
				UnitVector(Fixup);

				SmoothedInPoints[Loop].Normal[0] = (float)(Fixup[0]);
				SmoothedInPoints[Loop].Normal[1] = (float)(Fixup[1]);
				SmoothedInPoints[Loop].Normal[2] = (float)(Fixup[2]);

				// <<<>>> Debug code
				/*
				CheckLen = sqrt((double)(SmoothedInPoints[Loop].Normal[0] * SmoothedInPoints[Loop].Normal[0] + 
				 SmoothedInPoints[Loop].Normal[1] * SmoothedInPoints[Loop].Normal[1] + 
				 SmoothedInPoints[Loop].Normal[2] * SmoothedInPoints[Loop].Normal[2]));
				if (CheckLen > 1.001)
					{
					Boop = 1;
					} // if
				*/
				} // if
			else
				{
/*				CurNorm[0] = 0.0;
				CurNorm[1] = 1.0;
				CurNorm[2] = 1.0;
				UnitVector(CurNorm);
				SmoothedInPoints[Loop].Normal[0] = (float)CurNorm[0];
				SmoothedInPoints[Loop].Normal[1] = (float)CurNorm[1];
				SmoothedInPoints[Loop].Normal[2] = (float)CurNorm[2]; */
				SmoothedInPoints[Loop].Normal[0] = 0.0f;
				SmoothedInPoints[Loop].Normal[1] = 0.0f;
				SmoothedInPoints[Loop].Normal[2] = 1.0f;
				} // else
			} // if
		} // for
	
	
	ClearFinalOutput();
	ClearEdgeBuf();


	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Processing Smoothed Points", LoadedPoints, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < LoadedPoints; Loop++)
		{
	#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
	#endif // DEFG_BUILD_WCSVNS
		PointDisabled = SmoothedInPoints[Loop].Disabled;
		if (!PointDisabled)
			{
			GX = GridFromCoordX(SmoothedInPoints[Loop].X);
			GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
			//ID[GY * DefgGridWidth + GX] = (unsigned long int)&SmoothedInPoints[Loop];
			FinalOutput[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
/*
			if (GY == 37 && GX == 13)
				Boop = 1;
*/
			EdgeBuf[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
			XCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].X;
			//YCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].Y;
			//TempFloat[GY * DefgGridWidth + GX] = 0.0f;
			} // if
		} // for


	//sprintf(StatusOut, "Rasterizing Spans\n"); PrintStatus(StatusOut);
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Rasterizing Smoothed Spans", TotalSpans, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < TotalSpans; Loop++)
		{
/*
		if (Loop == 5)
			Boop = 1;
*/
		#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
		#endif // DEFG_BUILD_WCSVNS
		SmoothRasterizeSpanElev(&SmoothedEdges[Loop]);
		} // for

//DumpElev("c:\\tmp\\LinearElevSpan.raw");
//CreateDumpEdgeBuf("c:\\tmp\\LinearElevSpanChar.raw");

SmoothPass = 2;

	//sprintf(StatusOut, "Horizontal Linear Polygon scan-filling..\n"); PrintStatus(StatusOut);
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Calculating Smoothed Polygon Edges", DefgGridHeight, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS

	for (GY = 0; GY < DefgGridHeight; GY++)
		{
		double SNXS = 0.0, SNXE = 0.0;
		#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(GY))
			{
			Abort = 1;
			break;
			} // if
		#endif // DEFG_BUILD_WCSVNS
		int FoundPointA = 0, TPAX, TPAY;
		double TPAZ, DTPAX;
		GY0Idx  = GY * DefgGridWidth;
		for (GX = 0; GX < DefgGridWidth; GX++)
			{
			if (EdgeBuf[GY0Idx + GX] != -FLT_MAX)
				{
				if (FoundPointA)
					{ // must be a point B
					if (GX - TPAX) //lint !e530 // don't scanfill if there's no space between.
						{
						RasterizeSpanElevGridHor(TPAX, DTPAX, TPAY, TPAZ, GX, XCoordBuf[GY0Idx + GX], EdgeBuf[GY0Idx + GX], SNXS, SNX[GY0Idx + GX]); //lint !e530 
						} // if
					TPAX = GX;
					DTPAX = XCoordBuf[GY0Idx + GX];
					TPAY = GY;
					TPAZ = EdgeBuf[GY0Idx + GX];
					SNXS = SNX[GY0Idx + GX];
					} // if
				else
					{
					// transfer current location into TPA for next operation
					TPAX = GX;
					DTPAX = XCoordBuf[GY0Idx + GX];
					TPAY = GY;
					TPAZ = EdgeBuf[GY0Idx + GX];
					SNXS = SNX[GY0Idx + GX];
/*
					if (SNXS == FLT_MAX)
						Boop = 1;
*/
					} // else
				FoundPointA = 1;
				} // if
			} // for
		} // for


	// calculate new smoothed HSlopes and put into SNX buffer

	//sprintf(StatusOut, "Rasterizing Spans\n"); PrintStatus(StatusOut);
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Calculating Smoothed Polygons", TotalSpans, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < TotalSpans; Loop++)
		{
/*
		if (Loop == 5)
			Boop = 1;
*/
		#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
		#endif // DEFG_BUILD_WCSVNS
		CalcSpanNormals(&SmoothedEdges[Loop]);
		} // for

	ClearFinalOutput();

	SmoothPass = 2;

	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Processing Smoothed Points", LoadedPoints, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < LoadedPoints; Loop++)
		{
	#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
	#endif // DEFG_BUILD_WCSVNS
		PointDisabled = SmoothedInPoints[Loop].Disabled;
		if (!PointDisabled)
			{
			GX = GridFromCoordX(SmoothedInPoints[Loop].X);
			GY = GridFromCoordY(SmoothedInPoints[Loop].Y);
			//ID[GY * DefgGridWidth + GX] = (unsigned long int)&SmoothedInPoints[Loop];
			FinalOutput[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
			EdgeBuf[GY * DefgGridWidth + GX] = (float)SmoothedInPoints[Loop].Z;
/*
			if (GY == 37 && GX == 13)
				Boop = 1;
*/
			XCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].X;
			//YCoordBuf[GY * DefgGridWidth + GX] = SmoothedInPoints[Loop].Y;
			//TempFloat[GY * DefgGridWidth + GX] = 0.0f;
			} // if
		} // for


	//sprintf(StatusOut, "Rasterizing Spans\n"); PrintStatus(StatusOut);
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Rasterizing Smoothed Spans", TotalSpans, 'BWSE', 0);
	#endif // DEFG_BUILD_WCSVNS
	for (Loop = 0; Loop < TotalSpans; Loop++)
		{
/*
		if (Loop == 5)
			Boop = 1;
*/
		#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Loop))
			{
			Abort = 1;
			break;
			} // if
		#endif // DEFG_BUILD_WCSVNS
		SmoothRasterizeSpanElev(&SmoothedEdges[Loop]);
		} // for

//DumpElev("c:\\tmp\\LinearElevSpan.raw");
//CreateDumpEdgeBuf("c:\\tmp\\LinearElevSpanChar.raw");


	//sprintf(StatusOut, "Horizontal Linear Polygon scan-filling..\n"); PrintStatus(StatusOut);
	#ifdef DEFG_BUILD_WCSVNS
	if (BWRB) delete BWRB; BWRB = NULL;
	if (Abort) return(0);
	BWRB = new BusyWin("Smooth Polygon Filling", DefgGridHeight, 'BWSE', 0); // "Calculating Smoothed Polygons"
	#endif // DEFG_BUILD_WCSVNS

	for (GY = 0; GY < DefgGridHeight; GY++)
		{
		double SNXS = 0.0, SNXE = 0.0;
		#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(GY))
			{
			Abort = 1;
			break;
			} // if
		#endif // DEFG_BUILD_WCSVNS
		int FoundPointA = 0, TPAX, TPAY;
		double TPAZ, DTPAX;
		GY0Idx  = GY * DefgGridWidth;
		for (GX = 0; GX < DefgGridWidth; GX++)
			{
			if (EdgeBuf[GY0Idx + GX] != -FLT_MAX)
				{
				if (FoundPointA)
					{ // must be a point B
					if (GX - TPAX) //lint !e530 // don't scanfill if there's no space between.
						{
						RasterizeSpanElevGridHor(TPAX, DTPAX, TPAY, TPAZ, GX, XCoordBuf[GY0Idx + GX], EdgeBuf[GY0Idx + GX], SNXS, SNX[GY0Idx + GX]); //lint !e530 
						} // if
					TPAX = GX;
					DTPAX = XCoordBuf[GY0Idx + GX];
					TPAY = GY;
					TPAZ = EdgeBuf[GY0Idx + GX];
					SNXS = SNX[GY0Idx + GX];
					} // if
				else
					{
					// transfer current location into TPA for next operation
					TPAX = GX;
					DTPAX = XCoordBuf[GY0Idx + GX];
					TPAY = GY;
					TPAZ = EdgeBuf[GY0Idx + GX];
					SNXS = SNX[GY0Idx + GX];
/*
					if (SNXS == FLT_MAX)
						Boop = 1;
*/
					} // else
				FoundPointA = 1;
				} // if
			} // for
		} // for


	} // if Smoothing
//CreateDumpSNX("c:\\tmp\\SNX.raw");


// for efficiency we can dump our set of points now
if (Smoothing)
	{
	if (SmoothedInPoints)	AppMem_Free(SmoothedInPoints, MaxPoints * sizeof(struct SmoothedInputPoint));
	SmoothedInPoints = NULL;
	} // if
else
	{
	if (InPoints)	AppMem_Free(InPoints, MaxPoints * sizeof(struct InputPoint));
	InPoints = NULL;
	} // else

// for stingyness we can dump the edges list at this point
if (Edges && MaxSpans)			AppMem_Free(Edges, MaxSpans * sizeof(struct Span));
Edges = NULL;
if (SmoothedEdges && MaxSpans)	AppMem_Free(SmoothedEdges, MaxSpans * sizeof(struct SmoothedSpan));
SmoothedEdges = NULL;



if (Extrapolate)
	{
	//sprintf(StatusOut, "Post gap-filling and extrapolating.\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Extrapolating", DefgGridHeight + DefgGridWidth, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS

	FlagInterps();

	Iterations = 0;
	do
		{
#ifdef DEFG_BUILD_WCSVNS
		if (BWRB && BWRB->Update(Iterations))
			{
			Abort = 1;
			break;
			} // if
#endif // DEFG_BUILD_WCSVNS
		Counter = 0;
/*
		if (Iterations == 52)
			{
			Boop = 1;
			} // if
*/
		for (GY = 0; GY < DefgGridHeight; GY++)
			{
			GYm1Idx = (GY - 1) * DefgGridWidth;
			GY0Idx  = (GY + 0) * DefgGridWidth;
			GYp1Idx = (GY + 1) * DefgGridWidth;
			for (GX = 0; GX < DefgGridWidth; GX++)
				{
				if ((!NonInfectious[GY0Idx + GX]) && (FinalOutput[GY0Idx + GX] != -FLT_MAX))
					{
					if (GY > 0)
						{ // look up one line
						YOff  = -1;
						GYIdx = GYm1Idx;
						if (GX > 0)
							{ // look left
							XOff      = -1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_DIAG_WEIGHT);
							} // if
						if (GX < DefgGridWidth - 1)
							{ // look right
							XOff      = 1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_DIAG_WEIGHT);
							} // if
						// always look above on your column
						if (1) // preserve consistant indent
							{
							XOff      = 0;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_VERT_WEIGHT);
							} // if
						} // if
					if (GY < DefgGridHeight - 1)
						{ // look down one line
						YOff  = 1;
						GYIdx = GYp1Idx;
						if (GX > 0)
							{ // look left
							XOff      = -1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_DIAG_WEIGHT);
							} // if
						if (GX < DefgGridWidth - 1)
							{ // look right
							XOff      = 1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_DIAG_WEIGHT);
							} // if
						// always look below on your column
						if (1) // preserve consistant indent
							{
							XOff      = 0;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_VERT_WEIGHT);
							} // if
						} // if
					if (1) // preserve consistant indent
						{ // always try left/right on your own line
						YOff  = 0;
						GYIdx = GY0Idx;
						if (GX > 0)
							{ // look left
							XOff      = -1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_HORIZ_WEIGHT);
							} // if
						if (GX < DefgGridWidth - 1)
							{ // look right
							XOff      = 1;
							Counter  += InfectCellVal(GX, GY, XOff, GYIdx, DEFG_GAPFILL_HORIZ_WEIGHT);
							} // if
						} // if
					NonInfectious[GY0Idx + GX] = 1;
					} // if
				} // for
			} // for
		NormalizeExtrap();
		Iterations++;
		} while (Counter);
	//sprintf(StatusOut, "Finished gap-filling, %d passes.\n", Iterations); PrintStatus(StatusOut);
	} // if

//sprintf(StatusOut, "Writing NullValue.\n"); PrintStatus(StatusOut);
#ifdef DEFG_BUILD_WCSVNS
if (BWRB) delete BWRB; BWRB = NULL;
if (Abort) return(0);
BWRB = new BusyWin("Nulling", DefgGridHeight, 'BWSE', 0);
#endif // DEFG_BUILD_WCSVNS

#ifdef WCS_BUILD_DEMO_OLD
if (!TexRoot)
	{
	return(0);
	} // if
else
	{
	float DEMMax = -FLT_MAX, DEMMin = FLT_MAX;
	double MetersPerDegLat, MetersPerDegLon, PlanetRad;
	// sample elevation range for watermarking
	for (GY = 0; GY < DefgGridHeight; GY++)
		{
		GY0Idx  = (GY + 0) * DefgGridWidth;
		for (GX = 0; GX < DefgGridWidth; GX++)
			{
			if (FinalOutput[GY0Idx + GX] != -FLT_MAX)
				{
				if (FinalOutput[GY0Idx + GX] > DEMMax)
					{
					DEMMax = FinalOutput[GY0Idx + GX];
					} // if
				if (FinalOutput[GY0Idx + GX] < DEMMin)
					{
					DEMMin = FinalOutput[GY0Idx + GX];
					} // if
				} // if
			} // for
		} // for
	DEMRange = 100.0f;
	if ((DEMMax != -FLT_MAX) && (DEMMin != FLT_MAX))
		{
		DEMRange = (DEMMax - DEMMin) * 0.2f;
		} // if

	TexData.VDEM[0] = &Vert;
	TexData.TexRefLat = 0.0; // CenterLat
	TexData.TexRefLon = 0.0; // CenterLon;
	PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
	MetersPerDegLat = LatScale(PlanetRad);
	TexData.MetersPerDegLat = MetersPerDegLat;
	MetersPerDegLon = LonScale(PlanetRad, TexData.TexRefLat);
	TexData.MetersPerDegLon = MetersPerDegLon;
	} // else
#endif //WCS_BUILD_DEMO_OLD

// convert -FLT_MAX values to Null value
for (GY = 0; GY < DefgGridHeight; GY++)
	{
#ifdef DEFG_BUILD_WCSVNS
	if (BWRB && BWRB->Update(GY))
		{
		Abort = 1;
		break;
		} // if
#endif // DEFG_BUILD_WCSVNS
	GY0Idx  = (GY + 0) * DefgGridWidth;
	for (GX = 0; GX < DefgGridWidth; GX++)
		{
		if (FinalOutput[GY0Idx + GX] == -FLT_MAX)
			{
			FinalOutput[GY0Idx + GX] = NullVal;
			} // if
#ifdef WCS_BUILD_DEMO_OLD
		// Do Demo version watermarking
		else
			{
			if (!TexRoot)
				{
				Abort = 1;
				} // if
			else
				{
				double Displace = 0.0, Value[3], TexX, TexY;
				// eval texture here
				if ((GX + GY) & 8)
					{
					Displace = 1.0;

					// fill in texture data
					TexX = (double)GX / DefgGridWidth;
					TexY = (double)GY / DefgGridHeight;
					Vert.xyz[0] = Vert.XYZ[0] = TexX; // CalcX + SeedXOffset;
					Vert.xyz[1] = Vert.XYZ[1] = TexX; // CalcY + SeedYOffset;
					//TexData.LowX = TexData.HighX = CalcX + SeedXOffset;
					//TexData.LowY = TexData.HighY = CalcY + SeedYOffset; 
					TexData.Latitude = Vert.Lat = TexX;
					TexData.Longitude = Vert.Lon = TexY;
					// evaluate texture
					Value[0] = 0.0;
					TexRoot->Eval(Value, &TexData);
					Displace = Value[0];
					} // if
				//Displace = 0.0;
				FinalOutput[GY0Idx + GX] += DEMRange * (float)Displace;
				} // else
			} // else
#endif //WCS_BUILD_DEMO_OLD
/*
		if ((FinalOutput[GY0Idx + GX] != 0.0) && (fabs(FinalOutput[GY0Idx + GX]) < .0001)) // one-tenth of a mm is minimum epsilon value to appease OpenGL
			{
			FinalOutput[GY0Idx + GX] = 0.0f;
			} // if
*/
		} // for
	} // for

FinalTime = GetSystemTimeFP();

//CreateDumpTempElev("c:\\tmp\\Filled.raw");
//DumpElev("c:\\tmp\\Final.raw");

ElapsedTime = (FinalTime - BeginTime);

#ifdef DEFG_BUILD_WCSVNS
if (BWRB)
	{
	delete BWRB;
	BWRB = NULL;
	} // if
if (Abort)
	return(0);
#endif // DEFG_BUILD_WCSVNS

//sprintf(StatusOut, "Total Elapsed Time: %f seconds.\n", ElapsedTime); PrintStatus(StatusOut);

return(1);

} // DEFG::Grid

/*===========================================================================*/

DEFG::DEFG()
{
densify = false;
TempBuf = NULL;
TempFloat = FinalOutput = EdgeBuf = NULL;
SNX = NULL;
XCoordBuf = NULL;
ID = NULL;
NonInfectious = NULL;

InPoints = NULL;
SmoothedInPoints = NULL;
Edges = NULL;
SmoothedEdges = NULL;

LoadedPoints = TotalSpans = 0;

MaxPoints = MaxSpans = 0;

NullVal = -9999.0f;
SmoothVal = 0.0f;
Smoothing = 0;
SmoothPass = 0;

// very permissive bounds
BoundLowX  = -DBL_MAX;
BoundHighX =  DBL_MAX;
BoundLowY  = -DBL_MAX;
BoundHighY =  DBL_MAX;

} // DEFG::DEFG

/*===========================================================================*/

int DEFG::AllocRasterBufs(int FullGridWidth, int FullGridHeight)
{

// <<<>>> TempBuf only needs to be used when extrapolate is on
if (!(TempBuf = (unsigned char *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(unsigned char), APPMEM_CLEAR)))
	return(0);
if (!(TempFloat = (float *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(float), APPMEM_CLEAR)))
	return(0);
if (!(FinalOutput = (float *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(float), APPMEM_CLEAR)))
	return(0);
if (!(EdgeBuf = (float *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(float), APPMEM_CLEAR)))
	return(0);
if (!(XCoordBuf = (double *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(double), APPMEM_CLEAR)))
	return(0);
// ID buf now occupies same block as FinalOutput as they are never used at the same time
/*if (!(ID = (unsigned long int *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(unsigned long int), APPMEM_CLEAR)))
	return(0);*/
if (!(NonInfectious = (unsigned char *)AppMem_Alloc(FullGridWidth * FullGridHeight * sizeof(unsigned char), APPMEM_CLEAR)))
	return(0);

ID = (unsigned long *)FinalOutput;

// SNX is a temporary masquerade of TempFloat
SNX = TempFloat;

return(1);

} // DEFG::AllocRasterBufs

/*===========================================================================*/

int DEFG::InitSizes(int FinalGridWidth, int FinalGridHeight, double HOver, double VOver, int NewMaxPoints)
{
int FullGridHeight, FullGridWidth;
int ExtraGridHeight, ExtraGridWidth;
int InitSmoothing = 0;

if (NewMaxPoints == 0)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "No points selected.");
	return(0);
	} // if

if (SmoothVal > 0.0) InitSmoothing = 1;

WriteGridWidth  = FinalGridWidth;
WriteGridHeight = FinalGridHeight;

ExtraGridWidth  = (int)((double)FinalGridWidth  * (HOver / 100.0));
ExtraGridHeight = (int)((double)FinalGridHeight * (VOver / 100.0));

FullGridWidth  = ExtraGridWidth  + ExtraGridWidth  + FinalGridWidth;
FullGridHeight = ExtraGridHeight + ExtraGridHeight + FinalGridHeight;

WriteGridOffX = ExtraGridWidth;
WriteGridOffY = ExtraGridHeight;

// defer raster alloc until gridding begins because if we're not tiled,
// we might be able to free the initial control point array by then
/*if (!AllocRasterBufs(FullGridWidth, FullGridHeight))
	return(0);
*/

if (InitSmoothing)
	{
	if (!(SmoothedInPoints = (struct SmoothedInputPoint *)AppMem_Alloc(NewMaxPoints * sizeof(struct SmoothedInputPoint), APPMEM_CLEAR)))
		return(0);
	} // if
else
	{
	if (!(InPoints = (struct InputPoint *)AppMem_Alloc(NewMaxPoints * sizeof(struct InputPoint), APPMEM_CLEAR)))
		return(0);
	} // else


MaxPoints = NewMaxPoints;
DefgGridWidth = FullGridWidth;
DefgGridHeight = FullGridHeight;
DefgGridCells = DefgGridWidth * DefgGridHeight;

LoadedPoints = 0;

return(1);

} // DEFG::InitSizes

/*===========================================================================*/

int DEFG::InitSpans(int NewMaxSpans)
{

if (Smoothing)
	{
	if (!(SmoothedEdges = (struct SmoothedSpan *)AppMem_Alloc(NewMaxSpans * sizeof(struct SmoothedSpan), APPMEM_CLEAR)))
		return(0);
	} // if
else
	{
	if (!(Edges = (struct Span *)AppMem_Alloc(NewMaxSpans * sizeof(struct Span), APPMEM_CLEAR)))
		return(0);
	} // else

MaxSpans = NewMaxSpans;

return(1);

} // DEFG::InitSpans

/*===========================================================================*/

DEFG::~DEFG()
{

if (TempBuf)		AppMem_Free(TempBuf, DefgGridCells * sizeof(unsigned char));
if (TempFloat)		AppMem_Free(TempFloat, DefgGridCells * sizeof(float));
if (FinalOutput)	AppMem_Free(FinalOutput, DefgGridCells * sizeof(float));
if (EdgeBuf)		AppMem_Free(EdgeBuf, DefgGridCells * sizeof(float));
if (XCoordBuf)		AppMem_Free(XCoordBuf, DefgGridCells * sizeof(double));
// ID buf now occupies same block as FinalOutput as they are never used at the same time
//if (ID)			AppMem_Free(ID, DefgGridCells * sizeof(unsigned long int));
if (NonInfectious)	AppMem_Free(NonInfectious, DefgGridCells * sizeof(unsigned char));

// SNX is a masquerades of TempFloat and doesn't get freed

if (InPoints)			AppMem_Free(InPoints, MaxPoints * sizeof(struct InputPoint));
if (SmoothedInPoints)	AppMem_Free(SmoothedInPoints, MaxPoints * sizeof(struct SmoothedInputPoint));
if (Edges)				AppMem_Free(Edges, MaxSpans * sizeof(struct Span));
if (SmoothedEdges)		AppMem_Free(SmoothedEdges, MaxSpans * sizeof(struct SmoothedSpan));

InPoints = NULL;
SmoothedInPoints = NULL;
Edges = NULL;
SmoothedEdges = NULL;

MaxPoints = MaxSpans = 0;

TempBuf = NULL;
TempFloat = FinalOutput = EdgeBuf = NULL;
XCoordBuf = NULL;
ID = NULL;
NonInfectious = NULL;

} // DEFG::~DEFG

/*===========================================================================*/

bool DEFG::AddPoint(double X, double Y, double Z)
{
bool rVal = false;

if (X >= BoundLowX && X <= BoundHighX && Y >= BoundLowY && Y <= BoundHighY)
	{
	if (Smoothing)
		{
		SmoothedInPoints[LoadedPoints].X = X;
		SmoothedInPoints[LoadedPoints].Y = Y;
		SmoothedInPoints[LoadedPoints].Z = Z;
		SmoothedInPoints[LoadedPoints].PointNum = LoadedPoints;
		SmoothedInPoints[LoadedPoints].NumSpans = SmoothedInPoints[LoadedPoints].MaxSpans = 0;
		SmoothedInPoints[LoadedPoints].SpanBlock = NULL;
		SmoothedInPoints[LoadedPoints].Normal[0] = SmoothedInPoints[LoadedPoints].Normal[1] = SmoothedInPoints[LoadedPoints].Normal[2] = 0.0f;
		} // if
	else
		{
		InPoints[LoadedPoints].X = X;
		InPoints[LoadedPoints].Y = Y;
		InPoints[LoadedPoints].Z = Z;
		InPoints[LoadedPoints].PointNum = LoadedPoints;
		InPoints[LoadedPoints].NumSpans = InPoints[LoadedPoints].MaxSpans = 0;
		InPoints[LoadedPoints].SpanBlock = NULL;
		} // if
	rVal = true;
	LoadedPoints++;
	} // if

return(rVal);

} // DEFG::AddPoint

/*===========================================================================*/

// adds pt1 and num2add interpolated points but NOT pt2
bool DEFG::AddPoints(Point3d &pt1, Point3d &pt2, unsigned long num2add)
{
bool rVal = false;

if (pt1[0] >= BoundLowX && pt1[0] <= BoundHighX && pt1[1] >= BoundLowY && pt1[1] <= BoundHighY)
	{
	if (Smoothing)
		{
		SmoothedInPoints[LoadedPoints].X = pt1[0];
		SmoothedInPoints[LoadedPoints].Y = pt1[1];
		SmoothedInPoints[LoadedPoints].Z = pt1[2];
		SmoothedInPoints[LoadedPoints].PointNum = LoadedPoints;
		SmoothedInPoints[LoadedPoints].NumSpans = SmoothedInPoints[LoadedPoints].MaxSpans = 0;
		SmoothedInPoints[LoadedPoints].SpanBlock = NULL;
		SmoothedInPoints[LoadedPoints].Normal[0] = SmoothedInPoints[LoadedPoints].Normal[1] = SmoothedInPoints[LoadedPoints].Normal[2] = 0.0f;
		} // if
	else
		{
		InPoints[LoadedPoints].X = pt1[0];
		InPoints[LoadedPoints].Y = pt1[1];
		InPoints[LoadedPoints].Z = pt1[2];
		InPoints[LoadedPoints].PointNum = LoadedPoints;
		InPoints[LoadedPoints].NumSpans = InPoints[LoadedPoints].MaxSpans = 0;
		InPoints[LoadedPoints].SpanBlock = NULL;
		} // if
	LoadedPoints++;
	} // if

// now add num2add interpolated points
double CurX = pt1[0];
double CurY = pt1[1];
double CurZ = pt1[2];
double DeltaX = (pt2[0] - pt1[0]) / (num2add + 1);
double DeltaY = (pt2[1] - pt1[1]) / (num2add + 1);
double DeltaZ = (pt2[2] - pt1[2]) / (num2add + 1);

for(unsigned long AddCount = 0; AddCount < num2add; AddCount++)
	{
	CurX += DeltaX;
	CurY += DeltaY;
	CurZ += DeltaZ;

	if (CurX >= BoundLowX && CurX <= BoundHighX && CurY >= BoundLowY && CurY <= BoundHighY)
		{
		if (Smoothing)
			{
			SmoothedInPoints[LoadedPoints].X = CurX;
			SmoothedInPoints[LoadedPoints].Y = CurY;
			SmoothedInPoints[LoadedPoints].Z = CurZ;
			SmoothedInPoints[LoadedPoints].PointNum = LoadedPoints;
			SmoothedInPoints[LoadedPoints].NumSpans = SmoothedInPoints[LoadedPoints].MaxSpans = 0;
			SmoothedInPoints[LoadedPoints].SpanBlock = NULL;
			SmoothedInPoints[LoadedPoints].Normal[0] = SmoothedInPoints[LoadedPoints].Normal[1] = SmoothedInPoints[LoadedPoints].Normal[2] = 0.0f;
			} // if
		else
			{
			InPoints[LoadedPoints].X = CurX;
			InPoints[LoadedPoints].Y = CurY;
			InPoints[LoadedPoints].Z = CurZ;
			InPoints[LoadedPoints].PointNum = LoadedPoints;
			InPoints[LoadedPoints].NumSpans = InPoints[LoadedPoints].MaxSpans = 0;
			InPoints[LoadedPoints].SpanBlock = NULL;
			} // if
		LoadedPoints++;
		} // if
	} // for

return(true);

} // DEFG::AddPoints

/*===========================================================================*/

void DEFG::SetBounds(double NewMinX, double NewMaxX, double NewMinY, double NewMaxY)
{

BoundLowX = simplemin(NewMinX, NewMaxX);
BoundHighX = simplemax(NewMinX, NewMaxX);
BoundLowY = simplemin(NewMinY, NewMaxY);
BoundHighY = simplemax(NewMinY, NewMaxY);

XRange = (BoundHighX - BoundLowX);
YRange = (BoundHighY - BoundLowY);

//sprintf(StatusOut, "Lat:  %f to %f.\n", -HighY, -LowY); PrintStatus(StatusOut);
//sprintf(StatusOut, "Lon:  %f to %f.\n", -HighX, -LowX); PrintStatus(StatusOut);
//sprintf(StatusOut, "Elev: %f to %f.\n", HighZ, LowZ); PrintStatus(StatusOut);

} // DEFG::SetBounds

/*===========================================================================*/

// API used to be compatible with nngridr
int DEFG::DoGrid(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, TerraGridder *TG, Joe **TCJoeList, int JoeCount, int TCCount, NNGrid *nng,
	double xstart, double xterm, double ystart, double yterm, CoordSys *MyCS)
{
double minx, maxx, miny, maxy, LapX, LapY, VX, VY, VZ, Horilap = 0.0, Vertlap = 0.0;
double add_x, add_y, cell_x, cell_y, delta_x, delta_y, last_x, last_y, last_z;
VertexDEM CSConvert;
BusyWin *BWRB = NULL;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VectorPoint *PLink;
unsigned long adding;
int InitWidth, InitHeight, InitMaxPoints, NOOSMaxPoints, PointLoop, SomeOverscan = 0, JoeLoop = 0;
bool first_point;
//int Boop;

// steal relevant values fron NNGrid object for our use
InitWidth  = nng->x_nodes;
InitHeight = nng->y_nodes;

// x & y nodes are already adjusted for overlap
XStart = xstart;
XTerm  = xterm;
YStart = ystart;
YTerm  = yterm;

if (densify)
	{
	cell_x = (XTerm - XStart) / (InitWidth - 1);
	cell_y = (YTerm - YStart) / (InitHeight - 1);
	} // if

Horilap = nng->horilap;
Vertlap = nng->vertlap;

LapX = fabs(Horilap * (XTerm - XStart) / 100.0);
LapY = fabs(Vertlap * (YTerm - YStart) / 100.0);

if (Horilap != 0.0 || Vertlap != 0.0)
	{
	SomeOverscan = 1;
	} // if

LowX  = minx = XStart - LapX;
HighX = maxx = XTerm + LapX;
LowY  = miny = YStart - LapY;
HighY = maxy = YTerm + LapY;

// enumerate total points
InitMaxPoints = NOOSMaxPoints = 0;
#ifdef DEFG_LIMIT_INPOINTS
if (TCCount > DEFG_LIMIT_INPOINTS)
	{
	InPointExcess = TCCount - DEFG_LIMIT_INPOINTS;
	if (InPointExcess < 0)
		{
		InPointExcess = 0;
		} // if
	} // if
#endif // DEFG_LIMIT_INPOINTS
BWRB = new BusyWin("Counting Points in Tile", JoeCount, 'BWSE', 0);
for (PointLoop = 0, JoeLoop = 0; JoeLoop < JoeCount; JoeLoop++)
	{
	Joe *CurJoe;

	CurJoe = TCJoeList[JoeLoop];
	if (!CurJoe) break; // encounter NULL entry before expected end, shouldn't happen

	if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;

	first_point = true;
	adding = 0;
	for (PLink = CurJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
		{
		VertexDEM CurVert;

		if (PLink->ProjToDefDeg(MyCoords, &CurVert))
			{
			//CurVert.Lon;
			//CurVert.Lat;
			VZ = CurVert.Elev;

			if (MyCS)
				{
				CSConvert.Lon = CurVert.Lon;
				CSConvert.Lat = CurVert.Lat;
				MyCS->DefDegToProj(&CSConvert);

				if (MyCS->Method.GCTPMethod == 0) // geographic
					{ // flip
					VX = -CSConvert.xyz[0];
					VY = CSConvert.xyz[1];
					} // if
				else
					{
					VX = CSConvert.xyz[0];
					VY = CSConvert.xyz[1];
					} // else
				} // if
			else
				{ // geographic, flip
				VX = -CurVert.Lon;
				VY = CurVert.Lat;
				} // else

			if (densify)
				{
				if (! first_point)
					{
					add_y = add_x = 0;
					adding = 0;
					delta_x = fabs(VX - last_x);
					delta_y = fabs(VY - last_y);
					if (delta_x > cell_x)
						{
						add_x = ceil(delta_x / cell_x) - 1.0;
						} // if
					if (delta_y > cell_y)
						{
						add_y = ceil(delta_y / cell_y) - 1.0;
						} // if
					adding += max((unsigned long)add_x, (unsigned long)add_y);
					} // if
				else
					{
					first_point = false;
					} // else
				last_x = VX;
				last_y = VY;
				} // if

			if (VX >= minx && VX <= maxx && VY >= miny && VY <= maxy)
				{
				InitMaxPoints++;
				InitMaxPoints += adding;
				// if overscan, count points within non-overscan region
				if (SomeOverscan && VX >= XStart && VX <= XTerm && VY >= YStart && VY <= YTerm)
					{
					NOOSMaxPoints++;
					NOOSMaxPoints += adding;
					} // if
				} // if values in grid region
			else
				{
				//Boop = 1;
				} // else
			} // if
		} // for

	if (BWRB && BWRB->Update(JoeLoop))
		{
		return(0);
		} // if
	} // for
if (BWRB) delete BWRB;
BWRB = NULL;

// would default overscan values be ineffective here?
if (SomeOverscan && InitMaxPoints == NOOSMaxPoints && Vertlap == 100.0 && Horilap == 100.0)
	{ // disable overscan
	// reset all values internally to non-overscan versions
	Vertlap = Horilap = 0.0;
	LapX = LapY = 0.0;
	LowX  = minx = XStart - LapX;
	HighX = maxx = XTerm + LapX;
	LowY  = miny = YStart - LapY;
	HighY = maxy = YTerm + LapY;
	} // if

SetExtrap(nng->extrap);
SetNullVal((float)nng->nuldat);
// must set smooth val before initsizes so we can allocate smoothing data structures
SetSmoothVal((float)nng->bI);
if (InitSizes(InitWidth, InitHeight, Horilap, Vertlap, InitMaxPoints))
	{
	SetBounds(minx, maxx, miny, maxy);
	// transfer points to DEFG internal
	BWRB = new BusyWin("Clipping Points in Tile", JoeCount, 'BWSE', 0);
	for (PointLoop = 0, JoeLoop = 0; JoeLoop < JoeCount; JoeLoop++)
		{
		Joe *CurJoe;

		CurJoe = TCJoeList[JoeLoop];
		if (!CurJoe) break; // encounter NULL entry before expected end, shouldn't happen

		if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;

		first_point = true;
		adding = 0;
		for (PLink = CurJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
			{
			VertexDEM CurVert;

			if (PLink->ProjToDefDeg(MyCoords, &CurVert))
				{
				//CurVert.Lon;
				//CurVert.Lat;
				VZ = CurVert.Elev;

				if (MyCS)
					{
					CSConvert.Lon = CurVert.Lon;
					CSConvert.Lat = CurVert.Lat;
					MyCS->DefDegToProj(&CSConvert);

					if (MyCS->Method.GCTPMethod == 0) // geographic
						{ // flip
						VX = -CSConvert.xyz[0];
						VY = CSConvert.xyz[1];
						} // if
					else
						{
						VX = CSConvert.xyz[0];
						VY = CSConvert.xyz[1];
						} // else
					} // if
				else
					{ // geographic, flip
					VX = -CurVert.Lon;
					VY = CurVert.Lat;
					} // else

				if (densify)
					{
					if (! first_point)
						{
						add_y = add_x = 0;
						adding = 0;
						delta_x = fabs(VX - last_x);
						delta_y = fabs(VY - last_y);
						if (delta_x > cell_x)
							{
							add_x = ceil(delta_x / cell_x) - 1.0;
							} // if
						if (delta_y > cell_y)
							{
							add_y = ceil(delta_y / cell_y) - 1.0;
							} // if
						adding += max((unsigned long)add_x, (unsigned long)add_y);
						} // if
					else
						{
						first_point = false;
						} // else
					last_x = VX;
					last_y = VY;
					last_z = VZ;
					} // if

				if (VX >= minx && VX <= maxx && VY >= miny && VY <= maxy)
					{
					if (densify && adding)
						{
						Point3d thisPt, lastPt;

						thisPt[0] = VX;
						thisPt[1] = VY;
						thisPt[2] = VZ;
						lastPt[0] = last_x;
						lastPt[1] = last_y;
						lastPt[2] = last_z;
						AddPoints(thisPt, lastPt, adding);
						} // if
					else
						{
						AddPoint(VX, VY, VZ);
						}
					} // if values in grid region 
				else
					{
					//Boop = 1;
					} // else
				} // if
			} // for
		if (BWRB && BWRB->Update(JoeLoop))
			{
			return(0);
			} // if
		} // for
	if (BWRB) delete BWRB;
	BWRB = NULL;

	// if we're not tiled, we could dump the initial points array here, as we've
	// already made our own internal copy.
	if (!(TG->EWMaps || TG->NSMaps))
		{
		TG->FreeControlPts();
		} // if

	// Prep for demo version texture burn-in
	#ifdef WCS_BUILD_DEMO_OLD
	TexRoot = new RootTexture(NULL, 0, 0, 0);
	if (!TexRoot)
		{
		return(0);
		} // if
	else
		{
		Texture *Tex;
		// set texture type to planar image
		if (Tex = TexRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_FRACTALNOISE))
			{
			// Don't set image yet
			Tex->SetMiscDefaults();
			//Tex->SetCoordSpace(WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ, TRUE);
			//Tex->SelfOpacity = 1; // to allow for Alpha-driven transparency
			} // if
		} // else

	#endif //WCS_BUILD_DEMO_OLD

	if (Grid())
		{
		// cleanup from demo version texture burn-in
		#ifdef WCS_BUILD_DEMO_OLD
		delete TexRoot;
		TexRoot = NULL;
		#endif //WCS_BUILD_DEMO_OLD
		return(SaveDEM(MyCS, DBHost, ProjHost, EffectsHost, nng));
		} // if
	} // if

return(0);

} // DEFG::DoGrid

/*===========================================================================*/

int DEFG::SaveDEM(CoordSys *MyCS, Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, NNGrid *nng)
{

#ifdef DEFG_BUILD_WCSVNS

// much code stolen from Gary's DEM saver in NNGrid.cpp -- NNGrid::MakeGrid()
double GeoFlipMult = -1.0;
//JoeDEM *MyDEM;
DEM TempDEM;
long SaveSize;
float *GridPtr, *SavePtr;
Joe *Clip, *Added = NULL;
//NotifyTag ChangeEvent[2];
LayerEntry *LE = NULL;
long x, y, SubsetIdx;
int Success = 0;
int Found = 0;
char filename[256], ObjName[64], BaseName[64] /*, NameStr[64] */;

// if we're geographic coords, we must flip lon values into WCS convention before writing.
if (MyCS && MyCS->Method.GCTPMethod) GeoFlipMult = 1.0;

// save DEM 
TempDEM.pLonEntries = WriteGridWidth;
TempDEM.pLatEntries = WriteGridHeight;
SaveSize = WriteGridWidth * WriteGridHeight * sizeof(float);

TempDEM.pLatStep = (YTerm - YStart) / (WriteGridHeight - 1); // Y step
TempDEM.pLonStep = (-1.0 * GeoFlipMult) * ((XTerm - XStart) / (WriteGridWidth - 1)); // X step
TempDEM.pNorthWest.Lon = GeoFlipMult * XStart;
TempDEM.pSouthEast.Lat = YStart;
TempDEM.pNorthWest.Lat = TempDEM.pSouthEast.Lat + (TempDEM.pLatEntries - 1) * TempDEM.pLatStep;
TempDEM.pSouthEast.Lon = (TempDEM.pNorthWest.Lon - (TempDEM.pLonEntries - 1) * TempDEM.pLonStep);
TempDEM.pElScale = ELSCALE_METERS;
TempDEM.pElDatum = 0.0;
TempDEM.SetNullValue(NullVal);
TempDEM.SetNullReject(1);
TempDEM.PrecalculateCommonFactors();

// need to subset grid in-place?
if (WriteGridOffX || WriteGridOffY)
	{
	SubsetIdx = 0;
	for (y = WriteGridOffY; y < DefgGridHeight - WriteGridOffY; y++)
		{
		for (x = WriteGridOffX; x < DefgGridWidth - WriteGridOffX; x++)
			{
			FinalOutput[SubsetIdx++] = FinalOutput[x + y * DefgGridWidth];
			} // for
		} // for
	} // if

if (RowUnify && ColUnify)
	{ // need to store and restore certain samples for edge unification
	float *OutputStrip;
	unsigned long int FullNumRows, FullNumCols;
	
	FullNumRows = (UnifyNSTiles * (WriteGridHeight - 1)) + 1;
	FullNumCols = (UnifyEWTiles * (WriteGridWidth - 1)) + 1;

	// first we write candidate value into edge buffers if none exist there currently
	// after this, we'll come back and read values back, picking up the value we wrote
	// if we were the ones that put them there, or that previous tiles wrote if we didn't
	// put them there.


	// top edge into RowUnify (add 0 for top edge, 1 for bottom edge)
	OutputStrip = &RowUnify[(FullNumCols * (UnifyNSTileNum + 0)) + (UnifyEWTileNum * (WriteGridWidth - 1))];
	y = 0; // top row
	for (x = 0; x < WriteGridWidth; x++)
		{
		if (OutputStrip[x] == -FLT_MAX)
			{
			OutputStrip[x] = FinalOutput[x + y * WriteGridWidth];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	// bottom edge into RowUnify (add 0 for top edge, 1 for bottom edge)
	OutputStrip = &RowUnify[(FullNumCols * (UnifyNSTileNum + 1)) + (UnifyEWTileNum * (WriteGridWidth - 1))];
	y = WriteGridHeight - 1; // bottom row
	for (x = 0; x < WriteGridWidth; x++)
		{
		if (OutputStrip[x] == -FLT_MAX)
			{
			OutputStrip[x] = FinalOutput[x + y * WriteGridWidth];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	// left edge into ColUnify (add 0 for left edge, 1 for right edge)
	OutputStrip = &ColUnify[(FullNumRows * (UnifyEWTileNum + 0)) + (UnifyNSTileNum * (WriteGridHeight - 1))];
	x = 0; // leftmost col
	for (y = 0; y < WriteGridHeight; y++)
		{
		if (OutputStrip[y] == -FLT_MAX)
			{
			OutputStrip[y] = FinalOutput[x + y * WriteGridWidth];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	// right edge into ColUnify (add 0 for left edge, 1 for right edge)
	OutputStrip = &ColUnify[(FullNumRows * (UnifyEWTileNum + 1)) + (UnifyNSTileNum * (WriteGridHeight - 1))];
	x = WriteGridWidth - 1; // rightmost col
	for (y = 0; y < WriteGridHeight; y++)
		{
		if (OutputStrip[y] == -FLT_MAX)
			{
			OutputStrip[y] = FinalOutput[x + y * WriteGridWidth];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for


	// Ok, now we turn around an do reads from the edge coherency buffers, reading back any slot that
	// has a valid (non -FLT_MAX) values. Someone else might have just put those values there, in which
	// case we're conforming our edge to theirs (first guy always wins). Or, we might have just put those
	// values there ourselves, in which case nothing changes and the next guys conform to our edge (we're
	// first, we win).

	// top edge from RowUnify (add 0 for top edge, 1 for bottom edge)

	OutputStrip = &RowUnify[(FullNumCols * (UnifyNSTileNum + 0)) + (UnifyEWTileNum * (WriteGridWidth - 1))];
	y = 0; // top row
	for (x = 0; x < WriteGridWidth; x++)
		{
		if (OutputStrip[x] != -FLT_MAX)
			{
			FinalOutput[x + y * WriteGridWidth] = OutputStrip[x];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	// bottom edge from RowUnify (add 0 for top edge, 1 for bottom edge)
	OutputStrip = &RowUnify[(FullNumCols * (UnifyNSTileNum + 1)) + (UnifyEWTileNum * (WriteGridWidth - 1))];
	y = WriteGridHeight - 1; // bottom row
	for (x = 0; x < WriteGridWidth; x++)
		{
		if (OutputStrip[x] != -FLT_MAX)
			{
			FinalOutput[x + y * WriteGridWidth] = OutputStrip[x];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	// left edge from ColUnify (add 0 for left edge, 1 for right edge)
	OutputStrip = &ColUnify[(FullNumRows * (UnifyEWTileNum + 0)) + (UnifyNSTileNum * (WriteGridHeight - 1))];
	x = 0; // leftmost col
	for (y = 0; y < WriteGridHeight; y++)
		{
		if (OutputStrip[y] != -FLT_MAX)
			{
			FinalOutput[x + y * WriteGridWidth] = OutputStrip[y];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for


	// right edge from ColUnify (add 0 for left edge, 1 for right edge)
	OutputStrip = &ColUnify[(FullNumRows * (UnifyEWTileNum + 1)) + (UnifyNSTileNum * (WriteGridHeight - 1))];
	x = WriteGridWidth - 1; // rightmost col
	for (y = 0; y < WriteGridHeight; y++)
		{
		if (OutputStrip[y] != -FLT_MAX)
			{
			FinalOutput[x + y * WriteGridWidth] = OutputStrip[y];
			} // if
		else
			{
			// if there's a valid value there already, leave it alone, we'll read it later for unification
			} // else
		} // for

	} // if


if ((TempDEM.RawMap = (float *)AppMem_Alloc(SaveSize, 0)) != NULL)
	{
	SavePtr = TempDEM.RawMap;
	for (x = 0; x < WriteGridWidth; x ++)
		{
		GridPtr = FinalOutput + x;
		for (y = 1; y <= WriteGridHeight; y ++, SavePtr ++, GridPtr += WriteGridWidth)
			{
			*SavePtr = (float)(*GridPtr);
			} // for y
		} // for x
	strcpy(ObjName, nng->grd_file);
	strcpy(BaseName, ObjName);
	strcat(ObjName, ".elev");
	strmfp(filename, nng->grd_dir, ObjName);
	TempDEM.FindElMaxMin();
	Clip = DBHost->AddDEMToDatabase("Terrain Gridder", BaseName, &TempDEM, MyCS, ProjHost, EffectsHost);
	if (TempDEM.SaveDEM(filename, GlobalApp->StatusLog))
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DEM_SAVE, ObjName);
		Success = 1;
		} // if
	else
		UserMessageOK("Terrain Gridder", "Error saving DEM! Check available drive space.");

	TempDEM.FreeRawElevs();
	} // if memory OK 

#endif // DEFG_BUILD_WCSVNS
return(Success);

} // DEFG::SaveDEM

/*===========================================================================*/

void DEFG::SetSmoothVal(float NewSmoothVal)
{

SmoothVal = NewSmoothVal;

if (SmoothVal < 0.0)
	SmoothVal = 0.0f;

if (SmoothVal > 0.0)
	Smoothing = 1;
else
	Smoothing = 0;

SSmoothFrac = SmoothVal / 100.0;
LSmoothFrac = 1.0 - SSmoothFrac;

} // DEFG::SetSmoothVal
