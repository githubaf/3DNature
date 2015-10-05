// DEFGDiag.cpp
// Diagnostic routines for DEFG
// Built from DEFG.cpp on
// Fri Aug 24, 2001 by CXH

#include "../stdafx.h"

#include "DEFG.h"
#include "DEFGSpline.h"

#ifdef DEFG_ENABLE_DIAG

#ifdef WCS_BUILD_VNS
#include "../Application.h"
#include "../Toolbar.h"
extern WCSApp *GlobalApp;
#else //  !WCS_BUILD_VNS
#endif // !WCS_BUILD_VNS


void DoSplineTest(double AX, double AY, double BX, double BY, double CX, double CY, double DX, double DY, char *FN, double Tension)
{
int Step, TotSteps = 100;
double DSplineStep, DPos, InterpVal, OneMinusT, /* DeltaX, */ DeltaYStep, DeltaY;
FILE *out;
SimpleGraphNode FourNode[4];

out = fopen(FN, "w");
if(!out) return;

FourNode[0].Distance = AX;
FourNode[0].Value    = AY;
FourNode[1].Distance = BX;
FourNode[1].Value    = BY;
FourNode[2].Distance = CX;
FourNode[2].Value    = CY;
FourNode[3].Distance = DX;
FourNode[3].Value    = DY;
#ifdef DEFG_SPLINE_LOCALTCB
FourNode[0].TCB[0] = Tension;
FourNode[1].TCB[0] = Tension;
FourNode[2].TCB[0] = Tension;
FourNode[3].TCB[0] = Tension;
#endif // DEFG_SPLINE_LOCALTCB

OneMinusT = 1.0 - Tension;

DSplineStep = (CX - BX) / (double)TotSteps;

// write left approach vector
//DeltaX = FourNode[1].Distance - FourNode[0].Distance;
DeltaY = FourNode[1].Value - FourNode[0].Value;
DeltaYStep = DeltaY / (double)TotSteps;
for(DPos = 0.0, Step = 0; Step < TotSteps; Step++)
	{
	InterpVal = FourNode[0].Value + (DeltaYStep * (double)Step);
	fprintf(out, "%f\n", InterpVal);
	DPos += DSplineStep;
	} // for

for(DPos = 0.0, Step = 0; Step < TotSteps; Step++)
	{
#ifdef DEFG_SPLINE_LOCALTCB
	InterpVal = GetSplineValueTOnly(DPos, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#else // !DEFG_SPLINE_LOCALTCB
	InterpVal = GetSplineValueGlobalT(DPos, OneMinusT, &FourNode[0], &FourNode[1], &FourNode[2], &FourNode[3]);
#endif // !DEFG_SPLINE_LOCALTCB
	fprintf(out, "%f\n", InterpVal);
	DPos += DSplineStep;
	} // for

// write right approach vector
//DeltaX = FourNode[3].Distance - FourNode[2].Distance;
DeltaY = FourNode[3].Value - FourNode[2].Value;
DeltaYStep = DeltaY / (double)TotSteps;
for(DPos = 0.0, Step = 0; Step < TotSteps; Step++)
	{
	InterpVal = FourNode[2].Value + (DeltaYStep * (double)Step);
	fprintf(out, "%f\n", InterpVal);
	DPos += DSplineStep;
	} // for


fclose(out);
} // DoSplineTest


void DEFG::DumpPoints(char *FileName)
{
FILE *Dump;
int Loop;

if(Dump = fopen(FileName, "w"))
	{
	for(Loop = 0; Loop < LoadedPoints; Loop++)
		{
		if(!InPoints[Loop].Disabled)
			{
			fprintf(Dump, "%f, %f, %f\n", InPoints[Loop].X, -InPoints[Loop].Y, InPoints[Loop].Z);
			} // if
		} // for
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpPoints


void DEFG::DumpID(char *FileName)
{
FILE *Dump;

if(Dump = fopen(FileName, "wb"))
	{
	fwrite(ID, DefgGridWidth * DefgGridHeight * sizeof(unsigned long int), 1, Dump);
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpID


void DEFG::DumpTempBuf(char *FileName)
{
FILE *Dump;

if(Dump = fopen(FileName, "wb"))
	{
	fwrite(TempBuf, DefgGridWidth * DefgGridHeight * sizeof(unsigned char), 1, Dump);
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpTempBuf


void DEFG::DumpDist(char *FileName)
{
FILE *Dump;

if(Dump = fopen(FileName, "wb"))
	{
	fwrite(TempFloat, DefgGridWidth * DefgGridHeight * sizeof(float), 1, Dump);
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpDist

void DEFG::DumpElev(char *FileName)
{
FILE *Dump;

if(Dump = fopen(FileName, "wb"))
	{
	fwrite(FinalOutput, DefgGridWidth * DefgGridHeight * sizeof(float), 1, Dump);
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpElev


void DEFG::DumpEdges(char *FileName)
{
FILE *Dump;
struct InputPoint *PA, *PB;
long int CurSpan;

if(Dump = fopen(FileName, "w"))
	{
	for(CurSpan = 0; CurSpan < TotalSpans; CurSpan++)
		{
		PA = (struct InputPoint *)Edges[CurSpan].PointA;
		PB = (struct InputPoint *)Edges[CurSpan].PointB;
		fprintf(Dump, "%f %f %f 1\n", PA->X, PA->Y, PA->Z);
		fprintf(Dump, "%f %f %f 0\n", PB->X, PB->Y, PB->Z);
		} // for
	fclose(Dump);
	Dump = NULL;
	} // if

} // DEFG::DumpEdges


void DEFG::CreateDumpEdgeBuf(char *FileName)
{
float ZVal;
int GX, GY;
unsigned char TempElevVal;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		ZVal = EdgeBuf[GY * DefgGridWidth + GX];
		TempElevVal = 1;
		if(ZVal >= LowZ && ZVal <= HighZ && ZVal != DEFG_NULL_VALUE)
			{
			TempElevVal = GridFromCoord(254, LowZ - 1, HighZ + 1, (double)ZVal);
			} // if
		TempBuf[GY * DefgGridWidth + GX] = TempElevVal;
		} // for
	} // for

DumpTempBuf(FileName);

} // DEFG::CreateDumpTempElev


void DEFG::CreateDumpTempElev(char *FileName)
{
float ZVal;
int GX, GY;
unsigned char TempElevVal;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		ZVal = FinalOutput[GY * DefgGridWidth + GX];
		TempElevVal = 1;
		if(ZVal >= LowZ && ZVal <= HighZ && ZVal != DEFG_NULL_VALUE)
			{
			TempElevVal = GridFromCoord(254, LowZ - 1, HighZ + 1, (double)ZVal);
			} // if
		TempBuf[GY * DefgGridWidth + GX] = TempElevVal;
		} // for
	} // for

DumpTempBuf(FileName);

} // DEFG::CreateDumpTempElev


void DEFG::CreateDumpTempDist(char *FileName)
{
float ZVal, MaxDist = 0.0f;
int GX, GY;
unsigned char TempElevVal;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		if(TempFloat[GY * DefgGridWidth + GX] > MaxDist)
			{
			MaxDist = TempFloat[GY * DefgGridWidth + GX];
			} // if
		} // for
	} // for


for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		ZVal = TempFloat[GY * DefgGridWidth + GX];
		TempElevVal = 254;
		if(ZVal >= 0.0 && ZVal <= 254)
			{
			TempElevVal = (unsigned char)GridFromCoord(254, 0.0, MaxDist / 50.0, (double)ZVal);
			} // if
		TempBuf[GY * DefgGridWidth + GX] = TempElevVal;
		} // for
	} // for

DumpTempBuf(FileName);

} // DEFG::CreateDumpTempDist


void DEFG::CreateDumpSNX(char *FileName)
{
int GX, GY;
unsigned char TempElevVal;
float Val, Max = -FLT_MAX, Min = FLT_MAX, ValRange;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		if(TempFloat[GY * DefgGridWidth + GX] != FLT_MAX)
			{
			if(TempFloat[GY * DefgGridWidth + GX] > Max)
				{
				Max = TempFloat[GY * DefgGridWidth + GX];
				} // if
			if(TempFloat[GY * DefgGridWidth + GX] < Min)
				{
				Min = TempFloat[GY * DefgGridWidth + GX];
				} // if
			} // if
		} // for
	} // for

if(Max == -FLT_MAX) return; // nothing?

ValRange = Max - Min;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		Val = TempFloat[GY * DefgGridWidth + GX];
		if(Val != FLT_MAX)
			{
			TempElevVal = (unsigned char)GridFromCoord(250, (double)Min, (double)Max, (double)Val);
			} // if
		else
			{
			TempElevVal = 0;
			} // else
		TempBuf[GY * DefgGridWidth + GX] = TempElevVal;
		} // for
	} // for

DumpTempBuf(FileName);

} // DEFG::CreateDumpSNX


void DEFG::CheckForDBLMAX(void)
{
double XCB;
float ZVal;
int GX, GY;
int Boop = 0;

for(GY = 0; GY < DefgGridHeight; GY++)
	{
	for(GX = 0; GX < DefgGridWidth; GX++)
		{
		ZVal = FinalOutput[GY * DefgGridWidth + GX];
		XCB = XCoordBuf[GY * DefgGridWidth + GX];
		if(ZVal != -FLT_MAX && XCB == -DBL_MAX)
			{
			Boop = 1;
			} // if
		} // for
	} // for

} // DEFG::CheckForDBLMAX


void DEFG::PrintStatus(char *Status)
{

#ifdef WCS_BUILD_VNS
GlobalApp->MCP->SetCurrentStatusText(Status);
#else //  !WCS_BUILD_VNS
printf("%s\n", Status);
#endif // !WCS_BUILD_VNS

} // DEFG::PrintStatus


#endif // DEFG_ENABLE_DIAG
