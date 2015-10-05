// DEM.cpp
// Code to actually do stuff for DEM objects
// Built from scratch and with code from V1 DEM.c and other V1 files
// on 8/24/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Application.h"
#include "Project.h"
#include "DEM.h"
#include "AppMem.h"
#include "Useful.h"
#include "Requester.h"
#include "Log.h"
#include "Project.h"
#include "Joe.h"
#include "Useful.h"
#include "Render.h"
#include "ImageInputFormat.h"
#include "zlib.h"
#include "VectorPolygon.h"
#include "VectorNode.h"

//#define DEM_TRACE

#ifdef DEM_TRACE
#define CRUMB(x) x
#else
#define CRUMB(x)
#endif

// Causes each DEM cell's VectorPolygon data to be released before the DEM is deleted.
// If the DEM deletion is to be followed by QuantaAllocator freeing of the actual memory
// then the individual releasing of data is a waste of time.
//#define WCS_VECPOLY_FREEINDIVIDUAL_DEMCELLDATA

// Used in Enterbox relel computation
float weight[11][11];

void DEM::Copy(DEM *CopyFrom, short CopyPointers)
{

if (CopyPointers)
	{
	RelElMap = CopyFrom->RelElMap;
	RawMap = CopyFrom->RawMap;
	FractalMap = CopyFrom->FractalMap;
	} // if

Flags = CopyFrom->Flags;
XIndex = CopyFrom->XIndex;
YIndex = CopyFrom->YIndex;
ScreenWidth = CopyFrom->ScreenWidth;
ScreenHeight = CopyFrom->ScreenHeight;
RawCount = CopyFrom->RawCount;
pNorthWest.Lat = CopyFrom->pNorthWest.Lat;
pSouthEast.Lat = CopyFrom->pSouthEast.Lat;
pNorthWest.Lon = CopyFrom->pNorthWest.Lon;
pSouthEast.Lon = CopyFrom->pSouthEast.Lon;
pLatEntries = CopyFrom->pLatEntries;
pLonEntries = CopyFrom->pLonEntries;
pElSamples = CopyFrom->pElSamples;
pRelElSamples = CopyFrom->pRelElSamples;
pDSLatEntries = CopyFrom->pDSLatEntries;
pDSLonEntries = CopyFrom->pDSLonEntries;
pDSSamples = CopyFrom->pDSSamples;
pElMaxEl = CopyFrom->pElMaxEl;
pElMinEl = CopyFrom->pElMinEl;
pRelElMaxEl = CopyFrom->pRelElMaxEl;
pRelElMinEl = CopyFrom->pRelElMinEl;
pElSumElDif = CopyFrom->pElSumElDif;
pElSumElDifSq = CopyFrom->pElSumElDifSq;
pRelElSumElDif = CopyFrom->pRelElSumElDif;
pRelElSumElDifSq = CopyFrom->pRelElSumElDifSq;
pFileVersion = CopyFrom->pFileVersion;
pLatStep = CopyFrom->pLatStep;
pLonStep = CopyFrom->pLonStep;
pElScale = CopyFrom->pElScale;
pElDatum = CopyFrom->pElDatum;
pRelElScale = CopyFrom->pRelElScale;
pRelElDatum = CopyFrom->pRelElDatum;
Lr = CopyFrom->Lr;
Lc = CopyFrom->Lc;
pNullValue = CopyFrom->pNullValue;
NullReject = CopyFrom->NullReject;
RelativeEffectAdd = CopyFrom->RelativeEffectAdd;
RelativeEffectSubtract = CopyFrom->RelativeEffectSubtract;
AbsoluteEffectMax = CopyFrom->AbsoluteEffectMax;
AbsoluteEffectMin = CopyFrom->AbsoluteEffectMin;

#ifndef BUILD_LIB
InvLatStep = CopyFrom->InvLatStep;
InvLonStep = CopyFrom->InvLonStep;
#endif // !BUILD_LIB

} // DEM::Copy

/*===========================================================================*/

unsigned long DEM::LoadDEM(char *filename, short LoadRelEl, CoordSys **MyCoords)
{
FILE *InFile;
char InName[256];

CRUMB(UserMessageOK("DEM Debug", "if filename");)
if (filename)
	{
	CRUMB(UserMessageOK("DEM Debug", "if PROJ_fopen");)
	if (InFile = PROJ_fopen(filename, "rb"))
		{
		CRUMB(UserMessageOK("DEM Debug", "result = LoadDEM");)
		if (LoadDEM(InFile, LoadRelEl, MyCoords))
			{
			fclose(InFile);
			CRUMB(UserMessageOK("DEM Debug", "if RawMap");)
			if (RawMap)
				{
				CRUMB(UserMessageOK("DEM Debug", "if LoadRelEl");)
				if (LoadRelEl)
					{
					CRUMB(UserMessageOK("DEM Debug", "if ! RelElMap");)
					if (! RelElMap)
						{
						CRUMB(UserMessageOK("DEM Debug", "Save1");)
						SaveDEM(filename, GlobalApp->StatusLog);	// overwrite elev file with V3 DEM incl RelEl
						return (RelElMap ? 1: 0);	
						} // if
					} // if
				return (1);
				} // if
			return (0);
			} // if new chunk-based file format
		if (! fseek(InFile, 0, SEEK_SET))
			{
			if (LoadRawElevs(InFile))
				{
				fclose(InFile);
				InFile = NULL;
				if (LoadRelEl)
					{
					strcpy(InName, filename);
					StripExtension(InName);
					strcat(InName, ".relel");
					if (InFile = PROJ_fopen(InName, "rb"))
						{
						if (LoadRelElevs(InFile))
							{
							fclose(InFile);
							return (1);
							} // if relel loaded
						fclose(InFile);
						} // if
					CRUMB(UserMessageOK("DEM Debug", "Save2");)
					SaveDEM(filename, GlobalApp->StatusLog);	// overwrite elev file with V3 DEM incl RelEl
					return (RelElMap ? 1: 0);	
					} // if
				return (1);
				} // if
			} // if
		fclose(InFile);
		return(0);
		} // if
	} // if

return(0);

} // DEM::LoadDEM

/*===========================================================================*/

unsigned long DEM::LoadRawElevs(char *InName)
{
FILE *InFile;
unsigned long Result = 0;

if (InName)
	{
	if (InFile = PROJ_fopen(InName, "rb"))
		{
		Result = LoadRawElevs(InFile);
		fclose(InFile);
		} // if
	} // if

return(Result);

} // DEM::LoadRawElevs

/*===========================================================================*/

unsigned long DEM::LoadRawElevs(FILE *Input)
{
double ScaleFactor;
short *TempMap = NULL;
unsigned long FlipFlop;
short RecomputeMaxMin = 0;

if (RawMap)
	{
	RawCount++;
	return(1);
	} // if

if (pFileVersion == 0.0)
	{ // Need to read header info
	if (fread(&pFileVersion, 4, 1, Input))
		{
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip32F(&pFileVersion, &pFileVersion);
		#endif // BYTEORDER_LITTLEENDIAN
		// Version is of the form 1.0200
		if ((pFileVersion - WCS_DEM_CURVERSION) < .01 && (pFileVersion - WCS_DEM_CURVERSION) > -.01) // change in digit 1.00x
			{ // Should be a transparent version change
			if (!ReadV101Header(Input))
				{
				return(NULL);
				} // if
			} // if
		else
			{
			if ((pFileVersion - WCS_DEM_CURVERSION) < .1 && (pFileVersion - WCS_DEM_CURVERSION) > -.1) // change in digit 1.0x
				{ // May need some fields adjusted, but no size changes
				if (!ReadV101Header(Input))
					{
					return(NULL);
					} // if
				} // if
			else
				{
				if ((pFileVersion - WCS_DEM_CURVERSION) < 1 && (pFileVersion - WCS_DEM_CURVERSION) > -1) // change in digit 1.x
					{ // Change in size of header.
					return(0); // None known at this time
					} // if
				else // change in digit x.00
					{ // Major change in file format, currently unrecognised
					return(0);
					} // else
				} // else
			} // else
		} // if
	} // if (pFileVersion = 0.0)
else
	{ // Skip header, if version known
	if ((!(pFileVersion < 1.01)) && (!(pFileVersion > 1.02)))
		{
		if (fseek(Input, WCS_DEM_HEADERSIZE_V102, SEEK_CUR))
			{
			return(NULL);
			} // if
		} // if
	else
		{
		return(NULL);
		} // else
	} // else

// If we get here, we've got valid header info, and should be positioned
// at the beginning of the real data.

if ((TempMap = (short *)AppMem_Alloc(MapSize() * sizeof(short), APPMEM_CLEAR))
	&& (RawMap = (float *)AppMem_Alloc(MapSize() * sizeof(float), APPMEM_CLEAR)))
	{
	if (pElScale != ELSCALE_METERS)
		RecomputeMaxMin = 1;
	ScaleFactor = pElScale / ELSCALE_METERS;
	pElScale = ELSCALE_METERS;
	if (fread(TempMap, 1, MapSize() * 2, Input) == MapSize() * 2)
		{
		unsigned long SizeToFlop = MapSize();
		// Need to byte-swap
		for (FlipFlop = 0; FlipFlop < SizeToFlop; FlipFlop++)
			{
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16S(TempMap[FlipFlop], &TempMap[FlipFlop]);
			#endif // BYTEORDER_LITTLEENDIAN
			RawMap[FlipFlop] = (float)(TempMap[FlipFlop] * ScaleFactor);
			} // for
		AppMem_Free(TempMap, MapSize() * sizeof(short));
		RawCount++;
		if (RecomputeMaxMin)
			FindElMaxMin();			
		return(1); // Success
		} // if
	else
		{
		UserMessageOK("DEM Elevation Load", "IO Error.");
		} // else
	AppMem_Free(TempMap, MapSize() * sizeof(short));
	AppMem_Free(RawMap, MapSize() * sizeof(float));
	RawMap = NULL;
	return(NULL);
	} // if
else
	{
	if (TempMap)
		AppMem_Free(TempMap, MapSize() * sizeof(short));
	if (RawMap)
		AppMem_Free(RawMap, MapSize() * sizeof(float));
	RawMap = NULL;
	return(NULL);
	} // else

} // DEM::LoadRawElevs

/*===========================================================================*/

unsigned long DEM::LoadRelElevs(FILE *Input)
{
#ifdef BYTEORDER_LITTLEENDIAN
unsigned long FlipFlop;
#endif // BYTEORDER_LITTLEENDIAN
DEM Temp;

if (fread(&Temp.pFileVersion, 4, 1, Input) == 1)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32F(&Temp.pFileVersion, &Temp.pFileVersion);
	#endif // BYTEORDER_LITTLEENDIAN
	// Version is of the form 1.0200
	if ((Temp.pFileVersion - WCS_DEM_CURVERSION) < .01 && (Temp.pFileVersion - WCS_DEM_CURVERSION) > -.01) // change in digit 1.00x
		{ // Should be a transparent version change
		if (!Temp.ReadV101Header(Input))
			{
			return(NULL);
			} // if
		} // if
	else
		{
		if ((Temp.pFileVersion - WCS_DEM_CURVERSION) < .1 && (Temp.pFileVersion - WCS_DEM_CURVERSION) > -.1) // change in digit 1.0x
			{ // May need some fields adjusted, but no size changes
			if (!Temp.ReadV101Header(Input))
				{
				return(NULL);
				} // if
			} // if
		else
			{
			if ((Temp.pFileVersion - WCS_DEM_CURVERSION) < 1 && (Temp.pFileVersion - WCS_DEM_CURVERSION) > -1) // change in digit 1.x
				{ // Change in size of header.
				return(0); // None known at this time
				} // if
			else // change in digit x.00
				{ // Major change in file format, currently unrecognised
				return(0);
				} // else
			} // else
		} // else
	} // if
else
	{
	return (NULL);
	} // else

// If we get here, we've got valid header info, and should be positioned
// at the beginning of the real data.

//if (RawMap = new short [MapSize()])
if (Temp.MapSize() == MapSize())
	{
	if (RelElMap = (short *)AppMem_Alloc(MapSize() * sizeof(short), APPMEM_CLEAR))
		{
		if (fread(RelElMap, 1, MapSize() * sizeof(short), Input) == MapSize() * sizeof(short))
			{
			#ifdef BYTEORDER_LITTLEENDIAN
			unsigned long SizeToFlop = MapSize();
			// Need to byte-swap
			for (FlipFlop = 0; FlipFlop < SizeToFlop; FlipFlop++)
				{
				SimpleEndianFlip16S(RelElMap[FlipFlop], &RelElMap[FlipFlop]);
				} // for
			#endif // BYTEORDER_LITTLEENDIAN
			pRelElSamples = Temp.pElSamples;
			pRelElSumElDif = Temp.pElSumElDif;
			pRelElSumElDifSq = Temp.pElSumElDifSq;
			pRelElMaxEl = Temp.pElMaxEl;
			pRelElMinEl = Temp.pElMinEl;
			pRelElScale = ELSCALE_METERS;
			pRelElDatum = 0.0;
			return(1); // Success
			} // if
		else
			{
			UserMessageOK("Relative Elevation Load", "IO Error.");
			} // else
		//delete [] RawMap;
		AppMem_Free(RelElMap, MapSize() * sizeof(short));
		RelElMap = NULL;
		return(NULL);
		} // if
	else
		{
		return(NULL);
		} // else
	} // if
else
	{
	return (NULL);
	} // else

} // DEM::LoadRelElevs

/*===========================================================================*/

// ValidateInBounds ensures that the point remains within the DEM after whatever precision errors occur during coord transforms
// so that VertexDataPoint will find a valid elevation
int DEM::TransferToVerticesLatLon(int ValidateInBounds)
{
double CurLat, CurLon;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
unsigned long LonCt, LatCt, LastLatCt, LastLonCt, Ct, Iterations, TestMore;
int rVal = 0;

if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	MyCoords = MyAttr->Coord;
else
	MyCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (MyCoords && Vertices || AllocVertices())
	{
	LastLonCt = pLonEntries - 1;
	LastLatCt = pLatEntries - 1;
	CurLon = Westest();
	// F2_NOTE:  rewrite loops for OpenMP
	for (Ct = LonCt = 0; LonCt < pLonEntries; ++LonCt, CurLon -= pLonStep)
		{
		CurLat = Southest(); 
		for (LatCt = 0; LatCt < pLatEntries; ++Ct, ++LatCt, CurLat += pLatStep)
			{
			Vertices[Ct].xyz[1] = CurLat;
			Vertices[Ct].xyz[0] = CurLon;
			Vertices[Ct].xyz[2] = RawMap[Ct];
			if (MyCoords != GlobalApp->AppEffects->FetchDefaultCoordSys())
				{
				MyCoords->ProjToDefDeg(&Vertices[Ct]);
				// if we need to validate that the vertex falls within the bounds of the DEM...
				// this test is necessary to avoid thrashing hard drives and coming up with no DEM to sample
				// elevations from in processes like export DXF and realtime terraffector evaluation...
				// and the fact that unprojecting vertex coords and reprojecting them is an imprecise business
				// which sometimes results in vertices that fall ever so slightly outside the DEM when reprojected
				if (ValidateInBounds && (LatCt == 0 || LatCt == LastLatCt || LonCt == 0 || LonCt == LastLonCt))
					{
					Iterations = 0;
					do
						{
						TestMore = 0;
						MyCoords->DefDegToProj(&Vertices[Ct]);
						if (Northest() > Southest())
							{
							if (Vertices[Ct].xyz[1] > Northest())
								{
								Vertices[Ct].xyz[1] -= fabs(pLatStep) * .0001;
								TestMore = 1;
								} // if
							if (Vertices[Ct].xyz[1] < Southest())
								{
								Vertices[Ct].xyz[1] += fabs(pLatStep) * .0001;
								TestMore = 1;
								} // if
							} // if
						else
							{
							if (Vertices[Ct].xyz[1] < Northest())
								{
								Vertices[Ct].xyz[1] += fabs(pLatStep) * .0001;
								TestMore = 1;
								} // if
							if (Vertices[Ct].xyz[1] > Southest())
								{
								Vertices[Ct].xyz[1] -= fabs(pLatStep) * .0001;
								TestMore = 1;
								} // if
							} // else
						if (Westest() > Eastest())
							{
							if (Vertices[Ct].xyz[0] > Westest())
								{
								Vertices[Ct].xyz[0] -= fabs(pLonStep) * .0001;
								TestMore = 1;
								} // if
							if (Vertices[Ct].xyz[0] < Eastest())
								{
								Vertices[Ct].xyz[0] += fabs(pLonStep) * .0001;
								TestMore = 1;
								} // if
							} // if
						else
							{
							if (Vertices[Ct].xyz[0] < Westest())
								{
								Vertices[Ct].xyz[0] += fabs(pLonStep) * .0001;
								TestMore = 1;
								} // if
							if (Vertices[Ct].xyz[0] > Eastest())
								{
								Vertices[Ct].xyz[0] -= fabs(pLonStep) * .0001;
								TestMore = 1;
								} // if
							} // else
						Iterations ++;
						MyCoords->ProjToDefDeg(&Vertices[Ct]);
						} while (TestMore && Iterations < 10);
					} // if
				} // if
			else
				{
				Vertices[Ct].Lat = Vertices[Ct].xyz[1];
				Vertices[Ct].Lon = Vertices[Ct].xyz[0];
				Vertices[Ct].Elev = Vertices[Ct].xyz[2];
				} // else
			} // for
		} // for
	rVal = 1;
	} // if

return (rVal);

} // DEM::TransferToVerticesLatLon

/*===========================================================================*/


int DEM::TransferXYZtoLatLon(void)
{
double LonDiff;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
long LonCt;	// unsigned long type doesn't work with OMP parallel for loop
unsigned long Ct, LatCt, LonZip;
int rVal = 0;

if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	MyCoords = MyAttr->Coord;
else
	MyCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (MyCoords && Vertices || AllocVertices())
	{
	if (MyCoords != GlobalApp->AppEffects->FetchDefaultCoordSys())
		{
		#pragma omp for private(Ct, LatCt)
		for (LonCt = 0; LonCt < (long)pLonEntries; ++LonCt)
			{
			Ct = LonCt * pLatEntries;
			for (LatCt = 0; LatCt < pLatEntries; ++Ct, ++LatCt)
				{
				MyCoords->ProjToDefDeg(&Vertices[Ct]);
				} // for
			} // for
		// end #pragma omp for
		for (LonCt = 1, LonZip = pLatEntries; LonCt < (long)pLonEntries; ++LonCt, LonZip += pLatEntries)
			{
			if ((LonDiff = Vertices[LonZip].Lon - Vertices[LonZip - pLatEntries].Lon) > 180.0)	//lint !e530
				{
				for (Ct = LonZip, LatCt = 0; LatCt < pLatEntries; ++LatCt, ++Ct)
					Vertices[Ct].Lon -= 360.0;
				} // if
			else if (LonDiff < -180.0)
				{
				for (Ct = LonZip, LatCt = 0; LatCt < pLatEntries; ++LatCt, ++Ct)
					Vertices[Ct].Lon += 360.0;
				} // else if
			} // for
		} // if
	else
		{
		#pragma omp for private(Ct, LatCt)
		for (LonCt = 0; LonCt < (long)pLonEntries; ++LonCt)
			{
			Ct = LonCt * pLatEntries;
			for (LatCt = 0; LatCt < pLatEntries; ++Ct, ++LatCt)
				{
				Vertices[Ct].Lat = Vertices[Ct].xyz[1];
				Vertices[Ct].Lon = Vertices[Ct].xyz[0];
				Vertices[Ct].Elev = Vertices[Ct].xyz[2];
				} // for
			} // for
		// end #pragma omp for
		for (LonCt = 1, LonZip = pLatEntries; LonCt < (long)pLonEntries; ++LonCt, LonZip += pLatEntries)
			{
			if ((LonDiff = Vertices[LonZip].Lon - Vertices[LonZip - pLatEntries].Lon) > 180.0)	//lint !e530
				{
				for (Ct = LonZip, LatCt = 0; LatCt < pLatEntries; ++LatCt, ++Ct)
					Vertices[Ct].Lon -= 360.0;
				} // if
			else if (LonDiff < -180.0)
				{
				for (Ct = LonZip, LatCt = 0; LatCt < pLatEntries; ++LatCt, ++Ct)
					Vertices[Ct].Lon += 360.0;
				} // else if
			} // for
		} // else
	rVal = 1;
	} // if

return (rVal);

} // DEM::TransferXYZtoLatLon

/*===========================================================================*/

unsigned long DEM::ReadV101Header(FILE *In)
{
short TempShort;

if (!fread(&pLonEntries, 4, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32U(pLonEntries, &pLonEntries);
#endif // BYTEORDER_LITTLEENDIAN
pLonEntries++;

if (pLonEntries <= 1 || pLonEntries > SHRT_MAX)
	return (0);

if (!fread(&pLatEntries, 4, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32U(pLatEntries, &pLatEntries);
#endif // BYTEORDER_LITTLEENDIAN

if (pLatEntries <= 1 || pLatEntries > SHRT_MAX)
	return (0);

if (!fread(&pSouthEast.Lat, 8, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip64(&pSouthEast.Lat, &pSouthEast.Lat);
#endif // BYTEORDER_LITTLEENDIAN
//pNorthWest.Lat = pSouthEast.Lat;

// Contrary to popular belief, lolong is actually the High Longitude value...
if (!fread(&pNorthWest.Lon, 8, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip64(&pNorthWest.Lon, &pNorthWest.Lon);
#endif // BYTEORDER_LITTLEENDIAN
//pNorthWest.Lon = pSouthEast.Lon;

if (!fread(&pLatStep, 8, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip64(&pLatStep, &pLatStep);
#endif // BYTEORDER_LITTLEENDIAN

if (!fread(&pLonStep, 8, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip64(&pLonStep, &pLonStep);
#endif // BYTEORDER_LITTLEENDIAN

if (!fread(&pElScale, 8, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip64(&pElScale, &pElScale);
#endif // BYTEORDER_LITTLEENDIAN

if (!fread(&TempShort, 2, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip16S(TempShort, &TempShort);
#endif // BYTEORDER_LITTLEENDIAN
pElMaxEl = (float)TempShort;

if (!fread(&TempShort, 2, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip16S(TempShort, &TempShort);
#endif // BYTEORDER_LITTLEENDIAN
pElMinEl = (float)TempShort;


if (!fread(&pElSamples, 4, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32U(pElSamples, &pElSamples);
#endif // BYTEORDER_LITTLEENDIAN

if (!fread(&pElSumElDif, 4, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32F(&pElSumElDif, &pElSumElDif);
#endif // BYTEORDER_LITTLEENDIAN

if (!fread(&pElSumElDifSq, 4, 1, In))
	{
	return(0);
	} // if
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32F(&pElSumElDifSq, &pElSumElDifSq);
#endif // BYTEORDER_LITTLEENDIAN

// Do not be deceived, grasshopper by the name
// lolong, it is actually the highlong
pNorthWest.Lat = pSouthEast.Lat + pLatStep * (pLatEntries - 1);
pSouthEast.Lon = pNorthWest.Lon - pLonStep * (pLonEntries - 1);

pElDatum = 0.0;

// precalculate some common factors that we don't need to store
PrecalculateCommonFactors();

return(1);
} // DEM::ReadV101Header

/*===========================================================================*/

/*signed short DEM::Sample(unsigned long Horiz, unsigned long Vert)
{
//signed short Elev;
//unsigned long Offset;

//Offset = Vert + (Horiz * pLatEntries);
//Elev = RawMap[Offset];
//return(Elev);
return(RawMap[Vert + (Horiz * pLatEntries)]);
} // DEM::Sample
  */

/*===========================================================================*/

short DEM::ComputeRelEl(MessageLog *ErrorReport)
{
float *AltMap;
long RowBytes, ReLr, ReLc, zip;
short error = 0, boxsize;

if (ErrorReport == NULL)
	{
	ErrorReport = GlobalApp->StatusLog;
	} // if

if (RawMap)
	{
	if (RelElMap)
		FreeRelElMap();
	if (AllocRelElMap())
		{
		boxsize = EnterBoxWeights();
		if (AltMap = (float *)AppMem_Alloc((pLatEntries + 10) * (pLonEntries + 10) * sizeof(float), 0))
			{
			for (ReLr = 0, zip = 0; ReLr < (long)pLonEntries; ReLr ++)
				{
				for (ReLc = 0; ReLc < (long)pLatEntries; ReLc ++, zip ++)
					{
					RelElMap[zip] = (short)RawMap[zip];
					} // for
				} // for
			RowBytes = pLatEntries * sizeof(float);
			for (ReLr = 0, zip = 0; ReLr < (long)pLonEntries; ReLr ++)
				{
				memcpy(&AltMap[5 + (ReLr + 5) * (pLatEntries + 10)], &RawMap[ReLr * pLatEntries], RowBytes);
				} // for
			PadArray(AltMap);
			if (ComputeRelEl(AltMap, boxsize))
				{
				FindRelElMaxMin();
				} // if
			else
				{
				error = 1;
				FreeRelElMap();
				} // else
			AppMem_Free(AltMap, (pLatEntries + 10) * (pLonEntries + 10) * sizeof(float));
			AltMap = NULL;
			} // if
		else
			{
			error = 1;
			} // else
		} // if
	else
		{
		error = 1;
		} // else
	} // if
else
	{
	error = 1;
	} // else

if (error)
	{
	ErrorReport->PostStockError(WCS_LOG_ERR_OPEN_FAIL, "Relative elevation creation");
	} // if error opening output file 

return((short)(! error));

} // DEM::ComputeRelEl()

/*===========================================================================*/

short DEM::EnterBoxWeights(void)
{
double sum = 0.0;
short stdweight11[11][11] = {
		0,0,1,1,1, 2,1,1,1,0,0,
		0,1,1,2,3, 3,3,2,1,1,0,
		1,1,2,3,4, 4,4,3,2,1,1,
		1,2,3,4,5, 6,5,4,3,2,1,
		1,3,4,5,7, 8,7,5,4,3,1,
		2,3,4,6,8,10,8,6,4,3,2,
		1,3,4,5,7, 8,7,5,4,3,1,
		1,2,3,4,5, 6,5,4,3,2,1,
		1,1,2,3,4, 4,4,3,2,1,1,
		0,1,1,2,3, 3,3,2,1,1,0,
		0,0,1,1,1, 2,1,1,1,0,0 };
short boxsize = 11, i, j;

for (i=0; i<boxsize; i++)
	{
	for (j=0; j<boxsize; j++)
		{
		weight[i][j] = stdweight11[i][j];
		sum += weight[i][j];
		} // for j
	} // for i

if (sum)
	sum = 1.0 / sum;
for (i=0; i<boxsize; i++)
	{
	for (j=0; j<boxsize; j++)
		{
		weight[i][j] = (float)(weight[i][j] * sum);
		} // for j
	} // for i

return (boxsize);

} // DEM::EnterBoxWeights() 

/*===========================================================================*/

void DEM::PadArray(float *AltMap)
{
float *rowptr, *mapptr;
long rowsize, maprowsize, maprowbytes, PaLr, PaLc;

rowsize = pLatEntries + 10;
maprowsize = pLatEntries;
maprowbytes = maprowsize * sizeof(float);

rowptr = AltMap + 5 * rowsize + 5;
mapptr = AltMap + 5;
for (PaLr = 0; PaLr < 5; ++PaLr)
	{
	memcpy(mapptr, rowptr, maprowbytes);
	mapptr += rowsize;
	} // for

rowptr = AltMap + (pLonEntries + 4) * rowsize + 5;
mapptr = rowptr + rowsize;
for (PaLr = 0; PaLr < 5; ++PaLr)
	{
	memcpy(mapptr, rowptr, maprowbytes);
	mapptr += rowsize;
	} // for

rowptr = AltMap + 5;
for (PaLr = 0; PaLr < (long)pLonEntries + 10; ++PaLr)
	{
	mapptr = AltMap + PaLr * rowsize;
	for (PaLc = 0; PaLc < 5; ++PaLc)
		{
		*mapptr = *rowptr;
		mapptr ++;
		} // for
	rowptr += rowsize;
	} // for

rowptr = AltMap + 4 + maprowsize;
for (PaLr = 0; PaLr < (long)pLonEntries + 10; ++PaLr)
	{
	mapptr = AltMap + PaLr * rowsize + 5 + maprowsize;
	for (PaLc = 0; PaLc < 5; ++PaLc)
		{
		*mapptr = *rowptr;
		mapptr ++;
		} // for
	rowptr += rowsize;
	} // for

} //  DEM::PadArray() 

/*===========================================================================*/


short DEM::FindRelElMaxMin(void)
{
unsigned long Zip;
long FREMMLr, FREMMLc, LastCol, ElDif;

if (RelElMap == NULL || RawMap == NULL)
	return (0);

pRelElMaxEl = -1000000.0f;
pRelElMinEl = 1000000.0f;
pRelElSamples = 0;
pRelElSumElDif = pRelElSumElDifSq = 0.0f;
LastCol = pLatEntries - 1;

for (FREMMLr = 0,Zip = 0; FREMMLr < (long)pLonEntries; FREMMLr ++)
	{
	for (FREMMLc = 0; FREMMLc < (long)pLatEntries; FREMMLc ++, Zip ++)
		{
		if (! NullReject || RawMap[Zip] != pNullValue)
			{
			if (RelElMap[Zip] > pRelElMaxEl)
				{
				pRelElMaxEl = RelElMap[Zip];
				} // if 
			if (RelElMap[Zip] < pRelElMinEl)
				{
				pRelElMinEl = RelElMap[Zip];
				} // else if 
			if (FREMMLr != 0 && FREMMLc != LastCol)
				{
				if (! NullReject || RawMap[Zip - LastCol] != pNullValue)
					{
					ElDif = abs(RelElMap[Zip] - RelElMap[Zip - LastCol]);
					pRelElSumElDif += ElDif;
					pRelElSumElDifSq += (float)(long)(ElDif * ElDif);
					pRelElSamples ++;
					} // if
				} // if not first row 
			} // if
		} // for
	} // for

return (1);

} // DEM::FindRelElMaxMin() 

/*===========================================================================*/

short DEM::SaveDEM(char *filename, MessageLog *ErrorReport)
{
char FileType[12], StrBuf[12];
FILE *felev;
char Version = WCS_PROJECT_CURRENT_VERSION;
char Revision = WCS_PROJECT_CURRENT_REVISION;
ULONG ItemTag, ByteOrder = 0xaabbccdd, TotalWritten = 0;
long BytesWritten;
//float Version = (float)(DEM_CURRENT_VERSION);
#ifdef BYTEORDER_LITTLEENDIAN
//unsigned long i, FlopSize;
#endif // BYTEORDER_LITTLEENDIAN
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
#endif // WCS_BUILD_VNS

if (! RawMap)
	return (0);
FindElMaxMin();
if (! RelElMap)
	ComputeRelEl(ErrorReport);
if (RelElMap)
	FindRelElMaxMin();

if ((felev = PROJ_fopen (filename, "wb" /*IOFLAG_BINARY | IOFLAG_WRONLY | IOFLAG_TRUNC | IOFLAG_CREAT*/)) == NULL)
	{
	if (ErrorReport)
		ErrorReport->PostStockError(WCS_LOG_ERR_OPEN_FAIL, filename);
	goto WriteError;
	} // if error opening output file 

strcpy(FileType, "WCS File");

// no tags or sizes for first four items: file descriptor, version, revision & byte order
if ((BytesWritten = WriteBlock(felev, (char *)FileType,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = WriteBlock(felev, (char *)&Version,
	WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = WriteBlock(felev, (char *)&Revision,
	WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = WriteBlock(felev, (char *)&ByteOrder,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

#ifdef WCS_BUILD_VNS

// CoordSys
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	{
	MyCoords = MyAttr->Coord;
	strcpy(StrBuf, "CoordSys");
	if (BytesWritten = WriteBlock(felev, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(felev, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = MyCoords->Save(felev, false))
				{
				TotalWritten += BytesWritten;
				fseek(felev, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(felev, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(felev, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if CoordSys saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

#endif // WCS_BUILD_VNS

memset(StrBuf, 0, 9);
strcpy(StrBuf, "DEMData");

if (BytesWritten = WriteBlock(felev, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(felev, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(felev, filename))
			{
			TotalWritten += BytesWritten;
			fseek(felev, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(felev, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(felev, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Paths saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

fclose(felev);

/*
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32F(&Version, &Version);
#endif // BYTEORDER_LITTLEENDIAN
fwrite ((char *)&Version, 1, 4, fhelev);

#ifdef BYTEORDER_LITTLEENDIAN
// Endian flip...
SimpleEndianFlip16S(Map->MaxEl, &Map->MaxEl);
SimpleEndianFlip16S(Map->MinEl, &Map->MinEl);

SimpleEndianFlip32S(Map->rows, &Map->rows);
SimpleEndianFlip32S(Map->columns, &Map->columns);
SimpleEndianFlip32S(Map->Samples, &Map->Samples);

SimpleEndianFlip32F(&Map->SumElDif, &Map->SumElDif);
SimpleEndianFlip32F(&Map->SumElDifSq, &Map->SumElDifSq);

SimpleEndianFlip64(&Map->lolat, &Map->lolat);
SimpleEndianFlip64(&Map->lolong, &Map->lolong);
SimpleEndianFlip64(&Map->steplat, &Map->steplat);
SimpleEndianFlip64(&Map->steplong, &Map->steplong);
SimpleEndianFlip64(&Map->elscale, &Map->elscale);

#endif // BYTEORDER_LITTLEENDIAN
//fwrite ((char *)&Map, 1, ELEVHDRLENV101, fhelev);
fwrite((char *)&Map->rows, 1, 4, fhelev);
fwrite((char *)&Map->columns, 1, 4, fhelev);
fwrite((char *)&Map->lolat, 1, 8, fhelev);
fwrite((char *)&Map->lolong, 1, 8, fhelev);
fwrite((char *)&Map->steplat, 1, 8, fhelev);
fwrite((char *)&Map->steplong, 1, 8, fhelev);
fwrite((char *)&Map->elscale, 1, 8, fhelev);
fwrite((char *)&Map->MaxEl, 1, 2, fhelev);
fwrite((char *)&Map->MinEl, 1, 2, fhelev);
fwrite((char *)&Map->Samples, 1, 4, fhelev);
fwrite((char *)&Map->SumElDif, 1, 4, fhelev);
fwrite((char *)&Map->SumElDifSq, 1, 4, fhelev);

#ifdef BYTEORDER_LITTLEENDIAN
FlopSize = Map->size / 2;
for (i=0; i<FlopSize; i++)
	SimpleEndianFlip16S(Map->map[i], &AltArray[i]);

if (fwrite ((char *)AltArray, 1, Map->size, fhelev) != (unsigned)Map->size)
#else
if (fwrite ((char *)Map->map, 1, Map->size, fhelev) != (unsigned)Map->size)
#endif // BYTEORDER_LITTLEENDIAN
	{
	if (ErrorReport)
		ErrorReport->PostStockError(WCS_LOG_ERR_WRITE_FAIL, filename);
	fclose (fhelev);
	goto SaveError;
	} // if error writing file 
else
	{
	fclose(fhelev);
	} // else saved OK 

#ifdef BYTEORDER_LITTLEENDIAN
// Endian flip...
SimpleEndianFlip16S(Map->MaxEl, &Map->MaxEl);
SimpleEndianFlip16S(Map->MinEl, &Map->MinEl);

SimpleEndianFlip32S(Map->rows, &Map->rows);
SimpleEndianFlip32S(Map->columns, &Map->columns);
SimpleEndianFlip32S(Map->Samples, &Map->Samples);

SimpleEndianFlip32F(&Map->SumElDif, &Map->SumElDif);
SimpleEndianFlip32F(&Map->SumElDifSq, &Map->SumElDifSq);

SimpleEndianFlip64(&Map->lolat, &Map->lolat);
SimpleEndianFlip64(&Map->lolong, &Map->lolong);
SimpleEndianFlip64(&Map->steplat, &Map->steplat);
SimpleEndianFlip64(&Map->steplong, &Map->steplong);
SimpleEndianFlip64(&Map->elscale, &Map->elscale);

#endif // BYTEORDER_LITTLEENDIAN
*/
return (1);

WriteError:

if (felev)
	fclose(felev);
return (0);

} // DEM::SaveDEM

/*===========================================================================*/

short DEM::ComputeRelEl(float *AltMap, short boxsize)
{
double sum, ScaleFactor, VertScaleFact, DataVal, LatStepMeters, LonStepMeters, CurLatScale;
long datarowsize, DataPt;
BusyWin *BWRE;
float *firstpt, *firstptr, *dataptr;
short *mapptr;
short a, b, CRELr, CRELc, boxoffset, error = 0;

BWRE = new BusyWin("Computing RelEl", pLonEntries, 'BWRE', 0);
boxoffset = (boxsize - 1) / 2;
datarowsize = pLatEntries + 10;
mapptr = RelElMap;

// get dem cell size in meters
GetDEMCellSizeMeters(LatStepMeters, LonStepMeters);
// get size of a degree in meters
CurLatScale = LatScale(GlobalApp->AppEffects->GetPlanetRadius());

// scale RelEl by the ratio of a cell of 90 meter USGS data
ScaleFactor = (((1.0 / 1200.0) / (LatStepMeters / CurLatScale)) + ((1.0 / 1200.0) / (LonStepMeters / CurLatScale))) / 2.0;
//ScaleFactor = (((1.0 / 1200.0) / pLatStep) + ((1.0 / 1200.0) / pLonStep)) / 2.0;
VertScaleFact = pElScale / ELSCALE_METERS;

for (CRELr = 0; CRELr < (long)pLonEntries; CRELr ++)
	{
	firstpt = AltMap + (CRELr + 5 - boxoffset) * datarowsize + 5 - boxoffset;
	for (CRELc = 0; CRELc < (long)pLatEntries; CRELc ++)
		{
		DataPt = (CRELr + 5) * datarowsize + CRELc + 5;
		sum = 0.0;
		firstptr = firstpt;
		for (a=0; a<boxsize; a++)
			{
			dataptr = firstptr;
			for (b=0; b<boxsize; b++)
				{
				sum += (*dataptr * weight[a][b]);
				dataptr ++;
				} // for b=0... 
			firstptr += datarowsize;
			} // for a=0... 
		//   *mapptr = (short)(*mapptr - sum);
		//   *mapptr = (short)(*mapptr * ScaleFactor);
		DataVal = (AltMap[DataPt] - sum) * VertScaleFact;
		// created truncation which had bad results when working with high-res DEMs
		//DataVal = (*mapptr - sum) * VertScaleFact;
		*mapptr = (short)(DataVal * ScaleFactor);
		++mapptr;
		++firstpt;
		} // for

	if (BWRE)
		{
		if (BWRE->Update(CRELr + 1))
			{
			error = 1;
			break;
			} // if
		} // if
	} // for

if (BWRE)
	delete BWRE;

BWRE = NULL;

return ((short)(! error));

} // DEM::ComputeRelEl()

/*===========================================================================*/

unsigned long DEM::ScreenTransform(unsigned long Width, unsigned long Height)
{
unsigned long InitIdx;

FreeScreenData();

if (Width && Height)
	{
	XIndex = new unsigned long [Width];
	YIndex = new unsigned long [Height];
	} // if

if (XIndex && YIndex)
	{	// Initialize index arrays
	// First the YIndex
	for (InitIdx = 0; InitIdx < Height; InitIdx++)
		{
		YIndex[InitIdx] = (unsigned long int)( pLatEntries * ((double)((Height - 1) - InitIdx) / (double)Height) );
		} // for
	// Now the XIndex
	for (InitIdx = 0; InitIdx < Width; InitIdx++)
		{
		//Fract = ((double)InitIdx / (double)(Width - 1));
		//Fract = Fract * pLonEntries;
		//Test= (unsigned long int)( pLatEntries * (unsigned long int)Fract );
		if (Width > 1)
			{
			XIndex[InitIdx] = (unsigned long int)( pLatEntries * (unsigned long int)((pLonEntries - 1) * ((double)InitIdx / (double)(Width - 1))) );
			} // if
		else
			{
			XIndex[InitIdx] = 0;
			} // else
		} // for
	ScreenWidth = Width;
	ScreenHeight = Height;
//	XIMax = XIndex[InitIdx];
	return(1);
	} // if

return(0);
} // DEM::ScreenTransform

/*===========================================================================*/

void DEM::FreeScreenData(void)
{

ScreenWidth = ScreenHeight = 0;
delete [] YIndex;	YIndex = NULL;
delete [] XIndex;	XIndex = NULL;

} // DEM::FreeScreenData

/*===========================================================================*/

short DEM::GetRelElFromScreenCoord(long X, long Y, short Max, short Min)
{
short TempRelEl;

if (!RelElMap || X < 0 || Y < 0 || X >= (long)ScreenWidth || Y >= (long)ScreenHeight)
	return(0);
TempRelEl = RelElMap[XIndex[X] + YIndex[Y]];

if (TempRelEl > Max)
	TempRelEl = Max;
if (TempRelEl < Min)
	TempRelEl = Min;

return(TempRelEl);

} // DEM::GetRelElFromScreenCoord

/*===========================================================================*/

short DEM::GetFractalFromScreenCoord(long X, long Y)
{
long NewX, NewY;

if (FractalMap && X >= 0 && Y >= 0 && X < (long)ScreenWidth && Y < (long)ScreenHeight)
	{
	NewX = (long)(((double)(XIndex[X] / pLatEntries) / (double)pLonEntries) * (pLonEntries - 1)) * 2 * (pLatEntries - 1); //lint !e653 !e414
	NewY = (long)(((double)YIndex[Y] / (double)pLatEntries) * (pLatEntries - 1)) * 2;

	return((short)(FractalMap[NewX + NewY]));
	} // if

return(0);

} // DEM::GetFractalFromScreenCoord

/*===========================================================================*/

void DEM::SampleCellCorner(unsigned long PositionFlags, VectorNode *CurNode)
{
unsigned long FromWest, FromSouth;

// Lr = west edge of cell
// Lc = south edge of cell

// which corner
FromSouth = (PositionFlags == WCS_VECTORNODE_FLAG_SWCORNER || PositionFlags == WCS_VECTORNODE_FLAG_SECORNER) ? Lc: Lc + 1;
FromWest = (PositionFlags == WCS_VECTORNODE_FLAG_NWCORNER || PositionFlags == WCS_VECTORNODE_FLAG_SWCORNER) ? Lr: Lr + 1;

CurNode->Elev = Sample(FromWest, FromSouth);
CurNode->NodeData->RelEl = XYRelEl(FromWest, FromSouth);

} // DEM::SampleCellCorner

/*===========================================================================*/

bool DEM::SplineInternalCellValue(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode,
	bool CellUnknown)
{
double DistFromWestCellEdge, PtToWest, DistFromSouthCellEdge, PtToSouth;
unsigned long FromWest, FromSouth, RowCt, RowStart, RowEnd, TFSInit;
long TempFromSouth,  PreSample, PostSample;
float Samples[4], InterpElev[4], InterpRelElev[4], D1, D2, S1, S2, S3, h1, h2, h3, h4;
bool SampleSouthValid = false, SampleNorthValid = false;
VertexDEM Vert;

// Lr = west edge of cell
// Lc = south edge of cell

if (DEMCoords && NodeCoords)
	{
	Vert.Lat = CurNode->Lat;
	Vert.Lon = CurNode->Lon;
	Vert.Elev = CurNode->Elev;
	if (! NodeCoords->DegToCart(&Vert))
		return (false);
	if (! DEMCoords->CartToProj(&Vert))
		return (false);
	} // if

if (CellUnknown)
	{
	if (DEMCoords && NodeCoords)
		{
		// put the point in the range where it should be - to the east of the northwest point
		//if (DEMCoords->Geographic)
		//	{
		//	while (Vert.xyz[0] < pNorthWest.Lon)
		//		Vert.xyz[0] += 360.0;
		//	while (Vert.xyz[0] > pNorthWest.Lon)
		//		Vert.xyz[0] -= 360.0;
		//	} // if
		FromWest = (unsigned long)fabs((Vert.xyz[0] - pNorthWest.Lon) * InvLonStep);
		FromSouth = (unsigned long)fabs((Vert.xyz[1] - pSouthEast.Lat) * InvLatStep);
		} // if
	else
		{
		// put the point in the range where it should be - to the east of the northwest point
		double TempLon = CurNode->Lon;
		//while (TempLon < pNorthWest.Lon)
		//	TempLon += 360.0;
		//while (TempLon > pNorthWest.Lon)
		//	TempLon -= 360.0;
		FromWest = (unsigned long)fabs((TempLon - pNorthWest.Lon) * InvLonStep);
		FromSouth = (unsigned long)fabs((CurNode->Lat - pSouthEast.Lat) * InvLatStep);
		} // else
	//if (FromWest < 0 || FromWest > pLonEntries - 1 || FromSouth < 0 || FromSouth > pLatEntries - 1)
	if (FromWest > pLonEntries - 1 || FromSouth > pLatEntries - 1)	// unsigned #'s can't be less than zero, so don't need to test that
		{
		bool isErr = true;

		// Attempt to recover for geographic.  Avoid doing this above, as it unnecessarily slows down the majority of the calls
		if (DEMCoords->Geographic)
			{
#ifdef DEBUG
			double origVal = Vert.xyz[0];
#endif // DEBUG

			while (Vert.xyz[0] < pNorthWest.Lon)
				Vert.xyz[0] += 360.0;
			while (Vert.xyz[0] > pNorthWest.Lon)
				Vert.xyz[0] -= 360.0;
			FromWest = (unsigned long)fabs((Vert.xyz[0] - pNorthWest.Lon) * InvLonStep);
			if (FromWest > pLonEntries - 1 || FromSouth > pLatEntries - 1)
				isErr = true;	// not really needed
			else
				isErr = false;
			} // if
		if (isErr)
			{
			UserMessageOK("DEM projection error", "DEM sampling is occurring outside DEM bounds. Rendering will be aborted. Check all Coordinate Systems for correctness. Do not use WCS Back-compatible Sphere.");
			return (false);	//  not inside DEM bounds
			} // if
		} // if
	} // if
else
	{
	FromSouth = Lc;
	FromWest = Lr;
	} // else

PtToWest = pNorthWest.Lon - (FromWest * pLonStep);
PtToSouth = pSouthEast.Lat + (FromSouth * pLatStep);
if (DEMCoords && NodeCoords)
	{
	DistFromWestCellEdge = fabs((Vert.xyz[0] - PtToWest) * InvLonStep);
	DistFromSouthCellEdge = fabs((Vert.xyz[1] - PtToSouth) * InvLatStep);
	} // if
else
	{
	DistFromWestCellEdge = fabs((CurNode->Lon - PtToWest) * InvLonStep);
	DistFromSouthCellEdge = fabs((CurNode->Lat - PtToSouth) * InvLatStep);
	} // else

if (DistFromWestCellEdge < .0001 || FromWest >= pLonEntries - 1)
	DistFromWestCellEdge = 0.0;	

if (DistFromSouthCellEdge < .0001 || FromSouth >= pLatEntries - 1)
	DistFromSouthCellEdge = 0.0;


#ifdef _DEBUG
//if (DistFromWestCellEdge > 1.0)
//	printf("");
//if (DistFromSouthCellEdge > 1.0)
//	printf("");
#endif // _DEBUG

if (DistFromWestCellEdge > 0.0)
	{
	S1 = (float)DistFromWestCellEdge;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0f * S3 - 3.0f * S2 + 1.0f;
	h2 = -2.0f * S3 + 3.0f * S2;
	h3 = S3 - 2.0f * S2 + S1;
	h4 = S3 - S2;
	} // if

if (DistFromSouthCellEdge == 0.0)
	{ // optimized only for calculating InterpElev[1] and InterpRelElev[1]
	// limits sampling to one row of data
	RowStart = 1;
	TFSInit = FromSouth;
	RowEnd = 2;
	} // if
else
	{
	// sample four rows unless we're in the first or last row
	if (FromSouth > 0)
		{
		// we're not in the bottom tier of cells
		// if DistFromWestCellEdge > 0 then we will also sample the next lattice to the east
		if (TestNullReject(FromSouth - 1 + FromWest * pLatEntries) || (DistFromWestCellEdge > 0.0 && TestNullReject(FromSouth - 1 + (FromWest + 1) * pLatEntries)))
			{
			RowStart = 1;
			TFSInit = FromSouth;
			} // if
		else
			{
			RowStart = 0;
			TFSInit = FromSouth - 1;
			SampleSouthValid = true;
			} // else
		} // if
	else
		{
		RowStart = 1;
		TFSInit = FromSouth;
		} // else
	if (FromSouth < pLatEntries - 2)
		{
		// we're not in the top tier of cells
		// if DistFromWestCellEdge > 0 then we will also sample the next lattice to the east
		if (TestNullReject(FromSouth + 2 + FromWest * pLatEntries) || (DistFromWestCellEdge > 0.0 && TestNullReject(FromSouth + 2 + (FromWest + 1) * pLatEntries)))
			{
			RowEnd = 3;
			} // if
		else
			{
			RowEnd = 4;
			SampleNorthValid = true;
			} // else
		} // if
	else
		{
		RowEnd = 3;
		} // else
	} // else

for (RowCt = RowStart, TempFromSouth = TFSInit; RowCt < RowEnd; ++RowCt, ++TempFromSouth)
	{
	if (DistFromWestCellEdge == 0.0)
		{
		InterpElev[RowCt] = Sample(FromWest, TempFromSouth);
		InterpRelElev[RowCt] = XYRelEl(FromWest, TempFromSouth);
		} // if
	else
		{
		if (FromWest > 0)
			{
			PreSample = (long)(TempFromSouth + (FromWest - 1) * pLatEntries);
			if (TestNullReject((unsigned long)PreSample))
				PreSample = -1;
			} // if
		else
			PreSample = -1;

		if (FromWest < pLonEntries - 2)
			{
			PostSample = (long)(TempFromSouth + (FromWest + 2) * pLatEntries);
			if (TestNullReject((unsigned long)PostSample))
				PostSample = -1;
			} // if
		else
			PostSample = -1;

		// elevation
		if (PreSample >= 0)
			Samples[0] = SampleRaw((unsigned long)PreSample);
		Samples[1] = Sample(FromWest, TempFromSouth);
		Samples[2] = Sample(FromWest + 1, TempFromSouth);
		if (PostSample >= 0)
			Samples[3] = SampleRaw((unsigned long)PostSample);

		D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
			(Samples[2] - Samples[1]);

		D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
			(Samples[2] - Samples[1]);

		InterpElev[RowCt] = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;

		// relative el
		if (PreSample >= 0)
			Samples[0] = SampleRelEl((unsigned long)PreSample);
		Samples[1] = XYRelEl(FromWest, TempFromSouth);
		Samples[2] = XYRelEl(FromWest + 1, TempFromSouth);
		if (PostSample >= 0)
			Samples[3] = SampleRelEl((unsigned long)PostSample);

		// whattup?
		D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
			(Samples[2] - Samples[1]);

		D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
			(Samples[2] - Samples[1]);

		InterpRelElev[RowCt] = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;
		} // else
	} // for

if (DistFromSouthCellEdge == 0.0)
	{
	CurNode->Elev = InterpElev[1];
	CurNode->NodeData->RelEl = InterpRelElev[1];
	} // if
else
	{
	S1 = (float)DistFromSouthCellEdge;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0f * S3 - 3.0f * S2 + 1.0f;
	h2 = -2.0f * S3 + 3.0f * S2;
	h3 = S3 - 2.0f * S2 + S1;
	h4 = S3 - S2;

	// elevation
	D1 = SampleSouthValid ? (0.5f * (InterpElev[1] - InterpElev[0])) + (0.5f * (InterpElev[2] - InterpElev[1])):
		(InterpElev[2] - InterpElev[1]);

	D2 = SampleNorthValid ? (0.5f * (InterpElev[2] - InterpElev[1])) + (0.5f * (InterpElev[3] - InterpElev[2])):
		(InterpElev[2] - InterpElev[1]);

	CurNode->Elev = InterpElev[1] * h1 + InterpElev[2] * h2 + D1 * h3 + D2 * h4;

	// relative el
	D1 = SampleSouthValid ? (0.5f * (InterpRelElev[1] - InterpRelElev[0])) + (0.5f * (InterpRelElev[2] - InterpRelElev[1])):
		(InterpRelElev[2] - InterpRelElev[1]);

	D2 = SampleNorthValid ? (0.5f * (InterpRelElev[2] - InterpRelElev[1])) + (0.5f * (InterpRelElev[3] - InterpRelElev[2])):
		(InterpRelElev[2] - InterpRelElev[1]);

	CurNode->NodeData->RelEl = InterpRelElev[1] * h1 + InterpRelElev[2] * h2 + D1 * h3 + D2 * h4;
	} // else

return (true);

} // DEM::SplineInternalCellValue

/*===========================================================================*/

bool DEM::SplineCellEdgeNorthSouth(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode)
{
double DistFromWestCellEdge, PtToWest;
unsigned long FromWest, FromSouth;
long PreSample, PostSample;
float Samples[4], D1, D2, S1, S2, S3, h1, h2, h3, h4;
VertexDEM Vert;

// Lr = west edge of cell
// Lc = south edge of cell

// which edge
FromSouth = (PositionFlags == WCS_VECTORNODE_FLAG_SOUTHEDGE) ? Lc: Lc + 1;
FromWest = Lr;

PtToWest = pNorthWest.Lon - (FromWest * pLonStep);
if (DEMCoords && NodeCoords)
	{
	Vert.Lat = CurNode->Lat;
	Vert.Lon = CurNode->Lon;
	Vert.Elev = CurNode->Elev;
	if (! NodeCoords->DegToCart(&Vert))
		return (false);
	if (! DEMCoords->CartToProj(&Vert))
		return (false);
	DistFromWestCellEdge = fabs((Vert.xyz[0] - PtToWest) * InvLonStep);
	} // if
else
	DistFromWestCellEdge = fabs((CurNode->Lon - PtToWest) * InvLonStep);

if (DistFromWestCellEdge < .0001)
	DistFromWestCellEdge = 0.0;

#ifdef _DEBUG
//if (DistFromWestCellEdge > 1.0)
//	printf("");
#endif // _DEBUG

if (DistFromWestCellEdge > 0.0)
	{
	if (FromWest > 0)
		{
		PreSample = (long)(FromSouth + (FromWest - 1) * pLatEntries);
		if (TestNullReject((unsigned long)PreSample))
			PreSample = -1;
		} // if
	else
		PreSample = -1;

	if (FromWest < pLonEntries - 2)
		{
		PostSample = (long)(FromSouth + (FromWest + 2) * pLatEntries);
		if (TestNullReject((unsigned long)PostSample))
			PostSample = -1;
		} // if
	else
		PostSample = -1;
		
	S1 = (float)DistFromWestCellEdge;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0f * S3 - 3.0f * S2 + 1.0f;
	h2 = -2.0f * S3 + 3.0f * S2;
	h3 = S3 - 2.0f * S2 + S1;
	h4 = S3 - S2;

	// elevation
	if (PreSample >= 0)
		Samples[0] = SampleRaw((unsigned long)PreSample);
	Samples[1] = Sample(FromWest, FromSouth);
	Samples[2] = Sample(FromWest + 1, FromSouth);
	if (PostSample >= 0)
		Samples[3] = SampleRaw((unsigned long)PostSample);

	D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
		(Samples[2] - Samples[1]);

	D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
		(Samples[2] - Samples[1]);

	CurNode->Elev = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;

	// relative el
	if (PreSample >= 0)
		Samples[0] = SampleRelEl((unsigned long)PreSample);
	Samples[1] = XYRelEl(FromWest, FromSouth);
	Samples[2] = XYRelEl(FromWest + 1, FromSouth);
	if (PostSample >= 0)
		Samples[3] = SampleRelEl((unsigned long)PostSample);

	D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
		(Samples[2] - Samples[1]);

	D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
		(Samples[2] - Samples[1]);

	CurNode->NodeData->RelEl = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;
	} // if
else
	{
	CurNode->Elev = Sample(FromWest, FromSouth);
	CurNode->NodeData->RelEl = XYRelEl(FromWest, FromSouth);
	} // else

return (true);

} // DEM::SplineCellEdgeNorthSouth

/*===========================================================================*/

bool DEM::SplineCellEdgeEastWest(CoordSys *DEMCoords, CoordSys *NodeCoords, unsigned long PositionFlags, VectorNode *CurNode)
{
double DistFromSouthCellEdge, PtToSouth;
unsigned long FromWest, FromSouth, FromWestPreCalc;
long PreSample, PostSample;
float Samples[4], D1, D2, S1, S2, S3, h1, h2, h3, h4;
VertexDEM Vert;

// Lr = west edge of cell
// Lc = south edge of cell

// which edge
FromWest = (PositionFlags == WCS_VECTORNODE_FLAG_WESTEDGE) ? Lr: Lr + 1;
FromSouth = Lc;
FromWestPreCalc = FromWest * pLatEntries;

PtToSouth = pSouthEast.Lat + (FromSouth * pLatStep);
if (DEMCoords && NodeCoords)
	{
	Vert.Lat = CurNode->Lat;
	Vert.Lon = CurNode->Lon;
	Vert.Elev = CurNode->Elev;
	if (! NodeCoords->DegToCart(&Vert))
		return (false);
	if (! DEMCoords->CartToProj(&Vert))
		return (false);
	DistFromSouthCellEdge = fabs((Vert.xyz[1] - PtToSouth) * InvLatStep);
	} // if
else
	DistFromSouthCellEdge = fabs((CurNode->Lat - PtToSouth) * InvLatStep);

if (DistFromSouthCellEdge < .0001)
	DistFromSouthCellEdge = 0.0;

#ifdef _DEBUG
//if (DistFromSouthCellEdge > 1.0)
//	printf("");
#endif // _DEBUG

if (DistFromSouthCellEdge > 0.0)
	{
	if (FromSouth > 0)
		{
		PreSample = (long)(FromSouth - 1 + FromWestPreCalc);
		if (TestNullReject((unsigned long)PreSample))
			PreSample = -1;
		} // if
	else
		PreSample = -1;

	if (FromSouth < pLatEntries - 2)
		{
		PostSample = (long)(FromSouth + 2 + FromWestPreCalc);
		if (TestNullReject((unsigned long)PostSample))
			PostSample = -1;
		} // if
	else
		PostSample = -1;

	S1 = (float)DistFromSouthCellEdge;
	S2 = S1 * S1;
	S3 = S1 * S2;
	h1 = 2.0f * S3 - 3.0f * S2 + 1.0f;
	h2 = -2.0f * S3 + 3.0f * S2;
	h3 = S3 - 2.0f * S2 + S1;
	h4 = S3 - S2;

	// elevation
	if (PreSample >= 0)
		Samples[0] = SampleRaw((unsigned long)PreSample);
	Samples[1] = SampleRaw(FromWestPreCalc + FromSouth);
	Samples[2] = SampleRaw(FromWestPreCalc + FromSouth + 1);
	if (PostSample >= 0)
		Samples[3] = SampleRaw((unsigned long)PostSample);

	D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
		(Samples[2] - Samples[1]);

	D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
		(Samples[2] - Samples[1]);

	CurNode->Elev = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;

	// relative el
	if (PreSample >= 0)
		Samples[0] = SampleRelEl((unsigned long)PreSample);
	Samples[1] = SampleRelEl(FromWestPreCalc + FromSouth);
	Samples[2] = SampleRelEl(FromWestPreCalc + FromSouth + 1);
	if (PostSample >= 0)
		Samples[3] = SampleRelEl((unsigned long)PostSample);

	D1 = PreSample >= 0 ? (((float)(.5) * (Samples[1] - Samples[0])) + ((float)(.5) * (Samples[2] - Samples[1]))):
		(Samples[2] - Samples[1]);

	D2 = PostSample >= 0 ? (((float)(.5) * (Samples[2] - Samples[1])) + ((float)(.5) * (Samples[3] - Samples[2]))):
		(Samples[2] - Samples[1]);

	CurNode->NodeData->RelEl = Samples[1] * h1 + Samples[2] * h2 + D1 * h3 + D2 * h4;
	} // if
else
	{
	CurNode->Elev = Sample(FromWest, FromSouth);
	CurNode->NodeData->RelEl = XYRelEl(FromWest, FromSouth);
	} // else

return (true);

} // DEM::SplineCellEdgeEastWest

/*===========================================================================*/

char PathCombine[512], NewPathCombine[512];

char *DEM::AttemptFindDEMPath(char *ObjName, Project *OpenFrom)
{
DirList *Skim;
FILE *TestOpen;

if (OpenFrom && ObjName && ObjName[0])
	{
	for (Skim = OpenFrom->DL; Skim; Skim = Skim->Next)
		{
		MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			return(Skim->Name);
			} // if
		MakeCompletePath(PathCombine, Skim->Name, ObjName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			return(Skim->Name);
			} // if
		} // for
	} // if

return(NULL);

} // DEM::AttemptFindDEMPath

/*===========================================================================*/

unsigned long DEM::AttemptLoadDEM(Joe *MyJoe, short LoadRelEl, Project *OpenFrom)
{
DirList *Skim;
FILE *TestOpen;
char ObjName[256];

if (MyJoe && MyJoe->FileName())
	{
	strcpy(ObjName, MyJoe->FileName());

	if (OpenFrom && ObjName && ObjName[0])
		{
		for (Skim = OpenFrom->DL; Skim; Skim = Skim->Next)
			{
			MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".elev");
			if (TestOpen = PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				MoJoe = MyJoe;
				return(LoadDEM(PathCombine, LoadRelEl, NULL));
				} // if
			MakeCompletePath(PathCombine, Skim->Name, ObjName, ".elev");
			if (TestOpen = PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				MoJoe = MyJoe;
				return(LoadDEM(PathCombine, LoadRelEl, NULL));
				} // if
			} // for
		} // if
	} // if

return(0);

} // DEM::AttemptLoadDEM

/*===========================================================================*/

FILE *AttemptOpenObjFile(char *ObjName, Project *OpenFrom)
{
DirList *Skim;
FILE *TestOpen;

if (OpenFrom && ObjName && ObjName[0])
	{
	for (Skim = OpenFrom->DL; Skim; Skim = Skim->Next)
		{
		MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			return(TestOpen);
			} // if
		MakeCompletePath(PathCombine, Skim->Name, ObjName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			return(TestOpen);
			} // if
		} // for
	} // if

return(NULL);

} // DEM::AttemptOpenObjFile

/*===========================================================================*/

void AttemptDeleteDEMFiles(char *ObjName, Project *OpenFrom)
{
DirList *Skim;
FILE *TestOpen;
int Hit = 0, Result = -2;

if (OpenFrom && ObjName && ObjName[0])
	{
	for (Skim = OpenFrom->DL; Skim && !Hit; Skim = Skim->Next)
		{
		MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			// Kill it.
			Result = remove(GlobalApp->MainProj->MungPath(PathCombine));
			Hit = 1;
			} // if
		else
			{
			MakeCompletePath(PathCombine, Skim->Name, ObjName, ".elev");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// Kill it.
				Result = remove(GlobalApp->MainProj->MungPath(PathCombine));
				Hit = 1;
				} // if
			} // else
		// And your little dog too...
		if (Hit)
			{
			MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".relel");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// Kill it.
				remove(GlobalApp->MainProj->MungPath(PathCombine));
				} // if
			else
				{
				MakeCompletePath(PathCombine, Skim->Name, ObjName, ".relel");
				if (TestOpen= PROJ_fopen(PathCombine, "rb"))
					{
					fclose(TestOpen);
					// Kill it.
					remove(GlobalApp->MainProj->MungPath(PathCombine));
					} // if
				} // else
			} // if
		MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".frd");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			// Kill it.
			remove(GlobalApp->MainProj->MungPath(PathCombine));
			} // if
		else
			{
			MakeCompletePath(PathCombine, Skim->Name, ObjName, ".frd");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// Kill it.
				remove(GlobalApp->MainProj->MungPath(PathCombine));
				} // if
			} // else
		} // for
	} // if

if (Result < 0 && GlobalApp->MainProj->Prefs.PrivateQueryConfigOptTrue("debug_file_remove"))
	{
	if (Result == 0)
		sprintf(PathCombine, "DEM Debug: %s: File deleted.", ObjName);
	else if (Result < -1)
		sprintf(PathCombine, "DEM Debug: %s: File not found to delete.", ObjName);
	else
		sprintf(PathCombine, "DEM Debug: %s: Error %d: Unable to delete file.", ObjName, errno);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, PathCombine);
	} // if

} // DEM::AttemptDeleteDEMFiles

/*===========================================================================*/

void AttemptRenameDEMFiles(Joe *OldGuy, char *NewName, Project *From)
{
DirList *Skim;
FILE *TestOpen;
const char *OldName;
int Hit = 0;

OldName = OldGuy->FileName();
if (From && OldName && OldName[0] && NewName && NewName[0])
	{
	for (Skim = From->DL; Skim && !Hit; Skim = Skim->Next)
		{
		MakeNonPadPath(PathCombine, Skim->Name, (char *)OldName, ".elev");
		MakeNonPadPath(NewPathCombine, Skim->Name, NewName, ".elev");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			// rename it.
			// Note: MungPath uses a static buffer, we can't use two
			// calls to it inline...
			strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
			strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
			rename(PathCombine, NewPathCombine);
			Hit = 1;
			} // if
		else
			{
			MakeCompletePath(PathCombine, Skim->Name, (char *)OldName, ".elev");
			MakeCompletePath(NewPathCombine, Skim->Name, NewName, ".elev");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// rename it.
				// Note: MungPath uses a static buffer, we can't use two
				// calls to it inline...
				strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
				strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
				rename(PathCombine, NewPathCombine);
				Hit = 1;
				} // if
			} // else
		// Fractal depth file
		MakeNonPadPath(PathCombine, Skim->Name, (char *)OldName, ".frd");
		MakeNonPadPath(NewPathCombine, Skim->Name, NewName, ".frd");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			fclose(TestOpen);
			// rename it.
			// Note: MungPath uses a static buffer, we can't use two
			// calls to it inline...
			strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
			strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
			rename(PathCombine, NewPathCombine);
			Hit = 1;
			} // if
		else
			{
			MakeCompletePath(PathCombine, Skim->Name, (char *)OldName, ".frd");
			MakeCompletePath(NewPathCombine, Skim->Name, NewName, ".frd");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// rename it.
				// Note: MungPath uses a static buffer, we can't use two
				// calls to it inline...
				strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
				strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
				rename(PathCombine, NewPathCombine);
				Hit = 1;
				} // if
			} // else
		// And your little dog too...
		if (Hit)
			{
			MakeNonPadPath(PathCombine, Skim->Name, (char *)OldName, ".relel");
			MakeNonPadPath(NewPathCombine, Skim->Name, NewName, ".relel");
			if (TestOpen= PROJ_fopen(PathCombine, "rb"))
				{
				fclose(TestOpen);
				// rename it.
				// Note: MungPath uses a static buffer, we can't use two
				// calls to it inline...
				strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
				strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
				rename(PathCombine, NewPathCombine);
				} // if
			else
				{
				MakeCompletePath(PathCombine, Skim->Name, (char *)OldName, ".relel");
				MakeCompletePath(NewPathCombine, Skim->Name, NewName, ".relel");
				if (TestOpen= PROJ_fopen(PathCombine, "rb"))
					{
					fclose(TestOpen);
					// rename it.
					// Note: MungPath uses a static buffer, we can't use two
					// calls to it inline...
					strcpy(PathCombine, GlobalApp->MainProj->MungPath(PathCombine));
					strcpy(NewPathCombine, GlobalApp->MainProj->MungPath(NewPathCombine));
					rename(PathCombine, NewPathCombine);
					} // if
				} // else
			} // if
		} // for
	} // if

} // DEM::AttemptRenameDEMFiles

/*===========================================================================*/


FILE *AttemptOpenRelElFile(char *ObjName, Project *OpenFrom)
{
DirList *Skim;
FILE *TestOpen;

if (OpenFrom && ObjName && ObjName[0])
	{
	for (Skim = OpenFrom->DL; Skim; Skim = Skim->Next)
		{
		MakeNonPadPath(PathCombine, Skim->Name, ObjName, ".relel");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			return(TestOpen);
			} // if
		MakeCompletePath(PathCombine, Skim->Name, ObjName, ".relel");
		if (TestOpen= PROJ_fopen(PathCombine, "rb"))
			{
			return(TestOpen);
			} // if
		} // for
	} // if

return(NULL);

} // DEM::AttemptOpenRelElFile

/*===========================================================================*/

int DEM::AttemptLoadDownSampled(Joe *MyJoe, Project *OpenFrom, long GridSize)
{
float *NewMap = NULL;
unsigned long NewLonEntries, NewLatEntries, NewSize, ALDSLr, ALDSLc, RowZip, Dest;

if (AttemptLoadDEM(MyJoe, 0, OpenFrom))
	{
	if (GridSize < 1)
		GridSize = 10;
	NewLonEntries = 1 + (pLonEntries - 1) / (unsigned long)GridSize;
	NewLatEntries = 1 + (pLatEntries - 1) / (unsigned long)GridSize;
	NewSize = NewLonEntries * NewLatEntries * sizeof(float);
	if (NewMap = (float *)AppMem_Alloc(NewSize, 0))
		{
		for (ALDSLr = Dest = 0; ALDSLr < pLonEntries; ALDSLr += GridSize)
			{
			RowZip = ALDSLr * pLatEntries;
			for (ALDSLc = 0; ALDSLc < pLatEntries; ALDSLc += GridSize, Dest ++)
				{
				NewMap[Dest] = RawMap[RowZip + ALDSLc];
				} // for
			} // for
		AppMem_Free(RawMap, MapSize() * sizeof(float));
		pLonEntries = NewLonEntries;
		pLatEntries = NewLatEntries;
		pLatStep *= GridSize;
		pLonStep *= GridSize;
		RawMap = NewMap;

		// precalculate some common factors that we don't need to store
		PrecalculateCommonFactors();

		return (1);
		} // if 
	else
		{
		FreeRawElevs();
		} // else
	} // if

return(0);

} // DEM::AttemptLoadDownSampled

/*===========================================================================*/

int DEM::AttemptLoadBetterDownSampled(Joe *MyJoe, Project *OpenFrom, int MaxPolys)
{
double LatScan, LonScan, LatInc, LonInc, ALBDSLatRange, ALBDSLonRange, PolyFrac, southest, westest, theLon;
float *NewMap = NULL;
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
unsigned long NewLonEntries, NewLatEntries, NewSize, ALBDSLr, ALBDSLc, Dest, TotPolys;

if ((MaxPolys == 0)/* || (MaxPolys > TotalPolys) */)
	{ // load full-res dataset
	if (AttemptLoadDownSampled(MyJoe, OpenFrom, 1))
		{
		return(1);
		} // if
	else
		{
		return(0);
		} // else
	} // if

if (AttemptLoadDEM(MyJoe, 0, OpenFrom))
	{
	if (MyJoe && (MyAttr = (JoeCoordSys *)MyJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
		HostCoords = MyAttr->Coord;
	else
		HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

	TotPolys = 2 * (pLonEntries - 1) * (pLatEntries - 1);
	if ((TotPolys < 2) || (MaxPolys < 2)) return(0);
	NewLonEntries = pLonEntries;
	NewLatEntries = pLatEntries;

/*
	do
		{ // loop, decrementing rows & cols until we're under just under the limit
		if (NewLonEntries > 2) NewLonEntries--;
		if (NewLatEntries > 2) NewLatEntries--;
		TestPolys = 2 * (NewLonEntries - 1) * (NewLatEntries - 1);
		if ((NewLatEntries == 2) && (NewLonEntries == 2)) break;
		} // do
		while (TestPolys > (unsigned)MaxPolys);
*/

	if ((2 * (pLonEntries - 1) * (pLatEntries - 1)) > (unsigned)MaxPolys)
		{ // downsample
		PolyFrac = sqrt((double)MaxPolys / (double)TotPolys);
		NewLonEntries = (unsigned long)(PolyFrac * (double)pLonEntries);
		NewLatEntries = (unsigned long)(PolyFrac * (double)pLatEntries);
		if (NewLatEntries < 2) NewLatEntries = 2;
		if (NewLonEntries < 2) NewLonEntries = 2;
		} // if

	ALBDSLatRange = Northest() - Southest();
	ALBDSLonRange = Westest() - Eastest();
	LatInc = ALBDSLatRange / (NewLatEntries - 1);
	LonInc = ALBDSLonRange / (NewLonEntries - 1);
	NewSize = NewLonEntries * NewLatEntries * sizeof(float);
	southest = Southest();
	westest = Westest();
	if (NewMap = (float *)AppMem_Alloc(NewSize, 0))
		{
		LonScan = 0.0;
		Dest = 0;
		for (ALBDSLr= 0; ALBDSLr < NewLonEntries; ++ALBDSLr)
			{
			theLon = westest - LonScan;
			LatScan = 0.0;
			for (ALBDSLc = 0; ALBDSLc < NewLatEntries; ++ALBDSLc)
				{
				double preventUnderflow;

				preventUnderflow = DEMArrayPointExtract(HostCoords, southest + LatScan, theLon);
				#ifdef DEBUG
				if (preventUnderflow < FLT_MIN)
					preventUnderflow = 0.0f;
				#endif // DEBUG
				NewMap[Dest] = (float)preventUnderflow;
				++Dest;
				LatScan += LatInc;
				} // for
			LonScan += LonInc;
			} // for

/*
		for (ALBDSLr = Dest = 0; ALBDSLr < pLonEntries; ALBDSLr += GridSize)
			{
			RowZip = ALBDSLr * pLatEntries;
			for (ALBDSLc = 0; ALBDSLc < pLatEntries; ALBDSLc += GridSize, Dest ++)
				{
				NewMap[Dest] = RawMap[RowZip + ALBDSLc];
				} // for
			} // for
*/
		AppMem_Free(RawMap, MapSize() * sizeof(float));
		pLonEntries = NewLonEntries;
		pLatEntries = NewLatEntries;
		pLatStep = LatInc;
		pLonStep = LonInc;
		RawMap = NewMap;

		// precalculate some common factors that we don't need to store
		PrecalculateCommonFactors();

		return (1);
		} // if 
	else
		{
		FreeRawElevs();
		} // else
	} // if

return(0);

} // DEM::AttemptLoadBetterDownSampled

/*===========================================================================*/

double DEM::DEMRelElPointExtract(CoordSys *SourceCoords, double Lat, double Lon)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
#endif // WCS_BUILD_VNS
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = Lat;
Vert.Lon = Vert.xyz[0] = Lon;

#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

if (! TestPointNULL(Vert.xyz[0], Vert.xyz[1]))
	{
	return (ArrayPointExtract(RelEl(), Vert.xyz[0], Vert.xyz[1],
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries()));
	} // if

return (0.0);

} // DEM::DEMRelElPointExtract


/*===========================================================================*/

#ifdef WCS_MULTIFRD
int DEM::AttemptLoadFractalFile(char *ObjName, char *ParamName, Project *OpenFrom)
#else // WCS_MULTIFRD
int DEM::AttemptLoadFractalFile(char *ObjName, Project *OpenFrom)
#endif // WCS_MULTIFRD
{
FILE *TestOpen;

if (OpenFrom && ObjName && ObjName[0])
	{
	#ifdef WCS_MULTIFRD
	sprintf(NewPathCombine, "%s%s", ObjName, ParamName);
	MakeNonPadPath(PathCombine, OpenFrom->dirname, NewPathCombine, ".frd");
	#else // WCS_MULTIFRD
	MakeNonPadPath(PathCombine, OpenFrom->dirname, ObjName, ".frd");
	#endif // WCS_MULTIFRD
	if (TestOpen= PROJ_fopen(PathCombine, "rb"))
		{
		fclose(TestOpen);
		return(LoadFractalMap(PathCombine));
		} // if
	MakeCompletePath(PathCombine, OpenFrom->dirname, ObjName, ".frd");
	if (TestOpen= PROJ_fopen(PathCombine, "rb"))
		{
		fclose(TestOpen);
		return(LoadFractalMap(PathCombine));
		} // if
	} // if

return(0);

} // DEM::AttemptLoadFractalFile

/*===========================================================================*/

#ifdef WCS_MULTIFRD
int DEM::AttemptSaveFractalFile(char *ObjName, char *ParamName, Project *OpenFrom)
#else // WCS_MULTIFRD
int DEM::AttemptSaveFractalFile(char *ObjName, Project *OpenFrom)
#endif // WCS_MULTIFRD
{
FILE *TestOpen;

if (OpenFrom && ObjName && ObjName[0])
	{
	#ifdef WCS_MULTIFRD
	sprintf(NewPathCombine, "%s%s", ObjName, ParamName);
	MakeNonPadPath(PathCombine, OpenFrom->dirname, NewPathCombine, ".frd");
	#else // WCS_MULTIFRD
	MakeNonPadPath(PathCombine, OpenFrom->dirname, ObjName, ".frd");
	#endif // WCS_MULTIFRD
	if (TestOpen= PROJ_fopen(PathCombine, "wb"))
		{
		fclose(TestOpen);
		return(SaveFractalMap(PathCombine));
		} // if
	} // if

return(0);

} // DEM::AttemptSaveFractalFile

/*===========================================================================*/

int DEM::AttemptCheckExistColorMapFile(char *ObjName, Project *OpenFrom, char *ReturnPath, char *OptionalDir)
{
char *UseThisDir;

if (OpenFrom && ObjName && ObjName[0])
	{
	UseThisDir = OptionalDir ? OptionalDir: OpenFrom->colormappath;
	MakeCompletePath(PathCombine, UseThisDir, ObjName, "");
	if (CheckExistColorMap(PathCombine))
		{
		if (ReturnPath)
			strcpy(ReturnPath, PathCombine);
		return (1);
		} // if
	strmfp(PathCombine, UseThisDir, ObjName);
	if (CheckExistColorMap(PathCombine))
		{
		if (ReturnPath)
			strcpy(ReturnPath, PathCombine);
		return (1);
		} // if
	strmfp(PathCombine, UseThisDir, ObjName);
	TrimTrailingSpaces(PathCombine);
	if (CheckExistColorMap(PathCombine))
		{
		if (ReturnPath)
			strcpy(ReturnPath, PathCombine);
		return (1);
		} // if
	} // if

return(0);

} // DEM::AttemptCheckExistColorMapFile

/*===========================================================================*/

int DEM::CheckExistColorMap(char *InputName)
{

if (CheckExistUnknownImageExtension(InputName))
	return (1);

return (0);

} // DEM::CheckExistColorMap

/*===========================================================================*/

short DEM::AllocFractalMap(int SetToDefault)
{

if (FractalMap = (signed char *)AppMem_Alloc(FractalMapSize(), 0))
	{
	if (SetToDefault)
		memset(FractalMap, -1, FractalMapSize());
	return (1);
	} // if

return (0);

} // DEM::AllocFractalMap

/*===========================================================================*/

int DEM::LoadFractalMap(char *InputName)
{
int Success = 1;
FILE *fFrd;

if (AllocFractalMap(0))
	{
	if (fFrd = PROJ_fopen(InputName, "rb"))
		{
		if (fread(FractalMap, FractalMapSize(), 1, fFrd) != 1)
			{
			Success = 0;
			} // if read error 
		fclose(fFrd);
		} // if file exists 
	else
		{
		Success = 0;
		} // else 
	if (! Success)
		FreeFractalMap();
	//if (LoadRasterImage(InputName, 0, (unsigned char **)&FractalMap, (short)(2 * (pLatEntries - 1)), (short)(pLonEntries - 1), 0, &Dummy, &Dummy, &Dummy))
	//	return (1);
	} // if

return (Success);

} // DEM::LoadFractalMap

/*===========================================================================*/

int DEM::SaveFractalMap(char *OutputName)
{
int Success = 1;
FILE *fFrd;

if (FractalMap)
	{
	if (fFrd = PROJ_fopen(OutputName, "wb"))
		{
		if (fwrite(FractalMap, FractalMapSize(), 1, fFrd) != 1)
			{
			Success = 0;
			} // if write error 
		fclose(fFrd);
		} // if file exists 
	else
		{
		Success = 0;
		} // else 
	} // if

return (Success);

} // DEM::SaveFractalMap

/*===========================================================================*/

void DEM::FreeFractalMap(void)
{

if (FractalMap)
	AppMem_Free(FractalMap, FractalMapSize());
FractalMap = NULL;

} // FreeFractalMap

/*===========================================================================*/

short DEM::AllocRelElMap(void)
{

if (RelElMap = (short *)AppMem_Alloc(RelElMapSize(), APPMEM_CLEAR))
	return (1);

return (0);

} // AllocRelElMap

/*===========================================================================*/

void DEM::FreeRelElMap(void)
{

if (RelElMap)
	AppMem_Free(RelElMap, RelElMapSize());
RelElMap = NULL;

} // FreeFractalMap

/*===========================================================================*/
/*===========================================================================*/

unsigned long DEM::RelEl_Load(FILE *ffile, short ByteFlip)
{
int zlib_result;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, Ct, MaxCt = MapSize();
unsigned char *CompBuf;
union MultiVal MV;
char Precision = -1, Compression = -1;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_DEM_COVERAGE_DATA_PRECISION:
						{
						BytesRead = ReadBlock(ffile, (char *)&Precision, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_COMPRESSION:
						{
						BytesRead = ReadBlock(ffile, (char *)&Compression, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_DATUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElDatum, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_ELSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_MAXEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElMaxEl, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_MINEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElMinEl, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SAMPLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElSamples, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SUMELDIF:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElSumElDif, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SUMELDIFSQ:
						{
						BytesRead = ReadBlock(ffile, (char *)&pRelElSumElDifSq, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_DATABLOCK:
						{
						if (Compression == WCS_DEM_COMPRESSION_ZLIB)
							{
							CRUMB(UserMessageOK("DEM Debug", "Zlib Compression");)
							// only WCS_DEM_PRECISION_SHORT is supported here
							if (RelElMap = (short *)AppMem_Alloc(MapSize() * sizeof(short), 0))
								{
								if (CompBuf = (unsigned char *)AppMem_Alloc(Size, APPMEM_CLEAR))
									{
									if (BytesRead = ReadLongBlock(ffile, (char *)CompBuf, Size))
										{
										unsigned long OutSize = MapSize() * sizeof(short);

										zlib_result = uncompress((unsigned char *)RelElMap, &OutSize, CompBuf, Size);
										if (zlib_result != Z_OK)
											{
											CRUMB(UserMessageOK("DEM Debug", "Decompression Failure");)
											FreeRawElevs();
											} // else read error
										} // if
									else
										{
										CRUMB(UserMessageOK("DEM Debug", "Decompression Buffer Allocation Failure");)
										FreeRawElevs();
										} // else read error
									AppMem_Free(CompBuf, Size);
									CompBuf = NULL;
									} //if
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // zlib compression
						else
							{
							if (Precision == WCS_DEM_PRECISION_SHORT)
								{
								if (RelElMap = (short *)AppMem_Alloc(MapSize() * sizeof(short), 0))
									{
									if (BytesRead = ReadLongBlock(ffile, (char *)RelElMap, Size))
										{
										if (ByteFlip)
											{
											for (Ct = 0; Ct < MaxCt; Ct ++)
												{
												SimpleEndianFlip16S((short)RelElMap[Ct], (short *)&RelElMap[Ct]);
												} // for
											} // if
										} // if
									else
										{
										FreeRelElMap();
										} // else read error
									} // if memory
								else if (! fseek(ffile, Size, SEEK_CUR))
									BytesRead = Size;
								} // if float
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
								} // no compression
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

} // DEM::RelEl_Load

/*===========================================================================*/
/*===========================================================================*/

unsigned long DEM::Save(FILE *ffile, char *filename)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMDATA_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(filename) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)filename)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_DEMDATA_COVERAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Coverage_Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if lake effect saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // DEM::Save

/*===========================================================================*/

unsigned long DEM::Coverage_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char Compression, SaveRelEl = TRUE;

Compression = WCS_DEM_COMPRESSION_NONE;
if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("zcomp_elevs"))
	{
	Compression = WCS_DEM_COMPRESSION_ZLIB;
	} // if

if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("strip_relel"))
	{
	SaveRelEl = FALSE;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_ROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&pLonEntries)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_COLUMNS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&pLatEntries)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_NWLAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pNorthWest.Lat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_NWLON, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pNorthWest.Lon)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_SELAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pSouthEast.Lat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_SELON, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pSouthEast.Lon)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_LATSTEP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pLatStep)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_LONSTEP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pLonStep)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (RawMap)
	{
	ItemTag = WCS_DEM_COVERAGE_ELEVATION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Elevation_Save(ffile, Compression))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if lake effect saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if rawmap

if (RelElMap && SaveRelEl)
	{
	ItemTag = WCS_DEM_COVERAGE_RELEL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = RelEl_Save(ffile, Compression))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if lake effect saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if relel

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // DEM::Coverage_Save

/*===========================================================================*/

unsigned long DEM::Elevation_Save(FILE *ffile, char Compression)
{
int zlib_result;
ULONG CompressedSize, ItemTag, ReqSize, TotalWritten = 0;
long BytesWritten;
unsigned char *CompBuf = NULL;
char Precision = WCS_DEM_PRECISION_FLOAT;

if (Compression == WCS_DEM_COMPRESSION_ZLIB)
	{
	Compression = WCS_DEM_COMPRESSION_NONE;	// default back to uncompressed if we fail to allocate a buffer or zlib has an error
	// ReqSize is via this zlib note: destLen is the total size of the destination buffer, which must be at least 0.1% larger than sourceLen plus 12 bytes
	ReqSize = (unsigned long)(MapSize() * sizeof(float) * 1.02f + 16);
	if (CompBuf = (unsigned char *)AppMem_Alloc(ReqSize, APPMEM_CLEAR))
		{
		CompressedSize = ReqSize;
		zlib_result = compress(CompBuf, &CompressedSize, (const unsigned char *)RawMap, MapSize() * sizeof(float));
		if (zlib_result == Z_OK)
			{
			Compression = WCS_DEM_COMPRESSION_ZLIB;
			} // if
		} // if
	} // if asking for zlib compression
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_PRECISION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Precision)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_COMPRESSION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Compression)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_DATUM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pElDatum)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_ELSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pElScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_MAXEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pElMaxEl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_MINEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pElMinEl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SAMPLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&pElSamples)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SUMELDIF, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pElSumElDif)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SUMELDIFSQ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pElSumElDifSq)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_NULLREJECT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&NullReject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_NULLVALUE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pNullValue)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if (Compression == WCS_DEM_COMPRESSION_ZLIB)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, CompressedSize,
		WCS_BLOCKTYPE_CHAR, (char *)CompBuf)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if zlib compressed
else
	{
	//if (Precision == WCS_DEM_PRECISION_FLOAT)
	//	{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, MapSize() * sizeof(float),
			WCS_BLOCKTYPE_CHAR, (char *)RawMap)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
	//	} // if
	/***
	else if (Precision == WCS_DEM_PRECISION_SHORT)
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, MapSize() * sizeof(short),
			WCS_BLOCKTYPE_CHAR, (char *)RawMap)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	***/
	} // else store decompressed

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

WrapUp:

if (CompBuf)
	AppMem_Free(CompBuf, ReqSize);

return (TotalWritten);

WriteError:

TotalWritten = 0;
goto WrapUp;

} // DEM::Elevation_Save

/*===========================================================================*/

unsigned long DEM::RelEl_Save(FILE *ffile, char Compression)
{
int zlib_result;
ULONG CompressedSize, ItemTag, ReqSize, TotalWritten = 0;
long BytesWritten;
unsigned char *CompBuf = NULL;
char Precision = WCS_DEM_PRECISION_SHORT;

if (Compression == WCS_DEM_COMPRESSION_ZLIB)
	{
	Compression = WCS_DEM_COMPRESSION_NONE;	// default back to uncompressed if we fail to allocate a buffer or zlib has an error
	// ReqSize is via this zlib note: destLen is the total size of the destination buffer, which must be at least 0.1% larger than sourceLen plus 12 bytes
	ReqSize = (unsigned long)(MapSize() * sizeof(short) * 1.02f + 16);
	if (CompBuf = (unsigned char *)AppMem_Alloc(ReqSize, APPMEM_CLEAR))
		{
		CompressedSize = ReqSize;
		zlib_result = compress(CompBuf, &CompressedSize, (const unsigned char *)RelElMap, MapSize() * sizeof(short));
		if (zlib_result == Z_OK)
			{
			Compression = WCS_DEM_COMPRESSION_ZLIB;
			}
		} // if
	} // if asking for zlib compression
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_PRECISION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Precision)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_COMPRESSION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Compression)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_DATUM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pRelElDatum)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_ELSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&pRelElScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_MAXEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pRelElMaxEl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_MINEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pRelElMinEl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SAMPLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&pRelElSamples)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SUMELDIF, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pRelElSumElDif)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEM_COVERAGE_DATA_SUMELDIFSQ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&pRelElSumElDifSq)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if (Compression == WCS_DEM_COMPRESSION_ZLIB)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, CompressedSize,
		WCS_BLOCKTYPE_CHAR, (char *)CompBuf)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // zlib compressed
else
	{
	if (Precision == WCS_DEM_PRECISION_FLOAT)
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, MapSize() * sizeof(float),
			WCS_BLOCKTYPE_CHAR, (char *)RelElMap)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	else if (Precision == WCS_DEM_PRECISION_SHORT)
		{
		if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_DEM_COVERAGE_DATA_DATABLOCK, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, MapSize() * sizeof(short),
			WCS_BLOCKTYPE_CHAR, (char *)RelElMap)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // store uncompressed

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

WrapUp:

if (CompBuf)
	AppMem_Free(CompBuf, ReqSize);

return (TotalWritten);

WriteError:

TotalWritten = 0;
goto WrapUp;

} // DEM::RelEl_Save

/*===========================================================================*/

/***
// returns approximate area in square kilometers
double DEM::RoughArea(void)
{
double L1, L2, L3;

L1 = FindDistance(pNorthWest.Lat, pNorthWest.Lon, pNorthWest.Lat, pSouthEast.Lon, EARTHRAD);
L2 = FindDistance(pNorthWest.Lat, pNorthWest.Lon, pSouthEast.Lat, pNorthWest.Lon, EARTHRAD);
L3 = FindDistance(pSouthEast.Lat, pNorthWest.Lon, pSouthEast.Lat, pSouthEast.Lon, EARTHRAD);

return (L2 * (L1 + L3) * 0.5);

} // DEM::RoughArea
***/

/*===========================================================================*/

int DEM::GetDEMCellSizeMeters(double &CellSizeNS, double &CellSizeWE)
{
double Dist[3];
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
VertexDEM Vert1, Vert2, Vert3;

if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (HostCoords->GetGeographic())
	{
	CellSizeNS = (Northest() - Southest()) / (LatEntries() - 1);
	CellSizeWE = (Westest() - Eastest()) / (LonEntries() - 1);

	// set up three vertices near center of DEM, one cell apart on each axis
	Vert1.xyz[0] = Vert2.xyz[0] = Westest() - LonEntries() * .5 * CellSizeWE;
	Vert3.xyz[0] = Vert1.xyz[0] - CellSizeWE;

	Vert1.xyz[1] = Vert3.xyz[1] = Southest() + LatEntries() * .5 * CellSizeNS;
	Vert2.xyz[1] = Vert1.xyz[1] + CellSizeNS;

	if (! HostCoords->ProjToCart(&Vert1))
		return (0);
	if (! HostCoords->ProjToCart(&Vert2))
		return (0);
	if (! HostCoords->ProjToCart(&Vert3))
		return (0);

	Dist[0] = Vert1.XYZ[0] - Vert2.XYZ[0];
	Dist[1] = Vert1.XYZ[1] - Vert2.XYZ[1];
	Dist[2] = Vert1.XYZ[2] - Vert2.XYZ[2];
	CellSizeNS = sqrt(Dist[0] * Dist[0] + Dist[1] * Dist[1] + Dist[2] * Dist[2]);

	Dist[0] = Vert1.XYZ[0] - Vert3.XYZ[0];
	Dist[1] = Vert1.XYZ[1] - Vert3.XYZ[1];
	Dist[2] = Vert1.XYZ[2] - Vert3.XYZ[2];
	CellSizeWE = sqrt(Dist[0] * Dist[0] + Dist[1] * Dist[1] + Dist[2] * Dist[2]);
	} // if
else
	{
	CellSizeNS = fabs(LatStep());
	CellSizeWE = fabs(LonStep());
	} // else

return(1);

} // DEM::GetDEMCellSizeMeters


/*===========================================================================*/

void DEM::NullRejectSetFractalMap(void)
{
long VertNumber, PolyNumber;

if (NullReject && FractalMap && RawMap)
	{
	for (Lr = 0; Lr < (long)pLonEntries - 1; Lr ++)
		{
		VertNumber = Lr * pLatEntries;
		PolyNumber = (Lr * (pLatEntries - 1)) * 2;
		for (Lc = 0; Lc < (long)pLatEntries - 1; Lc ++, VertNumber ++, PolyNumber += 2)
			{
			if (TestNullValue(VertNumber) || TestNullValue(VertNumber + 1) || TestNullValue(VertNumber + pLatEntries) || TestNullValue(VertNumber + pLatEntries + 1))
				{
				FractalMap[PolyNumber] = FractalMap[PolyNumber + 1] = -1;
				} // if
			} // for
		} // for
	} // if

} // DEM::NullRejectSetFractalMap

/*===========================================================================*/

int DEM::GeographicPointContained(CoordSys *SourceCoords, double SourceLat, double SourceLon, int RejectNULL)
{
double X, Y;
double DEMLocalWest, DEMLocalEast; // these are for possible local wrap-around manipulation
unsigned long GeoCell;
int Contained = 1;
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
#endif // WCS_BUILD_VNS
CoordSys *HostCoords = NULL;
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = SourceLat;
Vert.Lon = Vert.xyz[0] = SourceLon;

#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

if (Northest() > Southest())
	{
	if (Vert.xyz[1] < Southest() || Vert.xyz[1] > Northest())
		Contained = 0;
	} // if
else
	{
	if (Vert.xyz[1] > Southest() || Vert.xyz[1] < Northest())
		Contained = 0;
	} // else

DEMLocalWest = Westest(); // may or may not be modified below
DEMLocalEast = Eastest();

if (Contained)
	{
	// we may employ special wrap-around longitude modulo compensation
	// if geographic (or no HostCoords, implying geographic)
	if ((! HostCoords) || HostCoords->GetGeographic())
		{ // we are geographic, may compensate both DEMLocalEast/West AND Vert.xyz[0]
		// we chose the range of -180 ... +180 so it is less likely to incur the performance penalty of compensation
		// during DEM merges of reasonable longitude ranges, though this means it will incur more often for foliage effects
		// and the like, which start with a range of -90 ... +270. These are less likely to be adversely affected.
		
		// We compensate the actual Vert.xyz[0] as well, so that other math (below this test) benefits from the compensation

		// compensate East into range of -180 ... +180
		if (DEMLocalEast < -180.0)
			DEMLocalEast += 360.0;
		else if (DEMLocalEast > 180.0)
			DEMLocalEast -= 360.0;

		// compensate West into range of -180 ... +180
		if (DEMLocalWest < -180.0)
			DEMLocalWest += 360.0;
		else if (DEMLocalWest > 180.0)
			DEMLocalWest -= 360.0;

		// compensate Source (now stored in Vert.xyz[0]) into range of -180 ... +180
		if (Vert.xyz[0] < -180.0)
			Vert.xyz[0] += 360.0;
		else if (Vert.xyz[0] > 180.0)
			Vert.xyz[0] -= 360.0;

		} // if
	// test is performed as usual, whether compensation (above) was necessary or not
	if (DEMLocalWest > DEMLocalEast)
		{
		if (Vert.xyz[0] < DEMLocalEast || Vert.xyz[0] > DEMLocalWest)
			Contained = 0;
		} // if
	else
		{
		if (Vert.xyz[0] > DEMLocalEast || Vert.xyz[0] < DEMLocalWest)
			Contained = 0;
		} // else
	} // if

if (Contained && RejectNULL)
	{
	// current cell
	Y = fabs(((Vert.xyz[1] - Southest()) / (Northest() - Southest())) * (pLatEntries - 1));
	X = fabs(((DEMLocalWest - Vert.xyz[0]) / (DEMLocalWest - DEMLocalEast)) * (pLonEntries - 1));
	GeoCell = (unsigned long)X * pLatEntries + (unsigned long)Y;
	if (TestNullReject(GeoCell))
		Contained = 0;
	// adjacent cells
	if (Contained && (X - (unsigned long)X > 0.0 && (unsigned long)X < pLonEntries - 1))
		{
		GeoCell = ((unsigned long)X + 1) * pLatEntries + (unsigned long)Y;
		if (TestNullReject(GeoCell))
			Contained = 0;
		} // if
	if (Contained && (Y - (unsigned long)Y > 0.0 && (unsigned long)Y < pLatEntries - 1))
		{
		GeoCell = (unsigned long)X * pLatEntries + (unsigned long)Y + 1;
		if (TestNullReject(GeoCell))
			Contained = 0;
		} // if
	if (Contained && ((Y - (unsigned long)Y > 0.0 && (unsigned long)Y < pLatEntries - 1)
		&& (X - (unsigned long)X > 0.0 && (unsigned long)X < pLonEntries - 1)))
		{
		GeoCell = ((unsigned long)X + 1) * pLatEntries + (unsigned long)Y + 1;
		if (TestNullReject(GeoCell))
			Contained = 0;
		} // if
	} // if

return (Contained);

} // DEM::GeographicPointContained

/*===========================================================================*/

int DEM::XYFromLatLon(CoordSys *SourceCoords, double SourceLat, double SourceLon, double &X, double &Y)
{
int Contained = 1;
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
#endif // WCS_BUILD_VNS
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = SourceLat;
Vert.Lon = Vert.xyz[0] = SourceLon;

#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

if (Northest() > Southest())
	{
	if (Vert.xyz[1] < Southest() || Vert.xyz[1] > Northest())
		Contained = 0;
	} // if
else
	{
	if (Vert.xyz[1] > Southest() || Vert.xyz[1] < Northest())
		Contained = 0;
	} // else
if (Contained)
	{
	Y = ((Vert.xyz[1] - Southest()) / (Northest() - Southest())) * (pLatEntries - 1) + .5;
	if (Westest() > Eastest())
		{
		if (Vert.xyz[0] < Eastest() || Vert.xyz[0] > Westest())
			Contained = 0;
		} // if
	else
		{
		if (Vert.xyz[0] > Eastest() || Vert.xyz[0] < Westest())
			Contained = 0;
		} // else
	if (Contained)
		X = ((Westest() - Vert.xyz[0]) / (Westest() - Eastest())) * (pLonEntries - 1) + .5;
	} // if

return (Contained);

} // DEM::XYFromLatLon

/*===========================================================================*/

void DEM::Disect(void)
{
long Pt, Size;

Size = (long)MapSize();

for (Pt = 0; Pt < Size; Pt ++)
	{
	RawMap[Pt] -= 0.25f * (pRelElMaxEl - RelElMap[Pt]);
	} // for

} // DEM::Disect

/*===========================================================================*/

void DEM::Erode(void)
{
long Pt, Size;

Size = (long)MapSize();

for (Pt = 0; Pt < Size; Pt ++)
	{
	RawMap[Pt] -= (float)(.25 * RelElMap[Pt]);
	} // for

} // DEM::Erode

/*===========================================================================*/

void DEM::SetBounds(double North, double South, double West, double East)
{

pNorthWest.Lat = North; pNorthWest.Lon = West; pSouthEast.Lat = South; pSouthEast.Lon = East;
pLatStep = (pNorthWest.Lat - pSouthEast.Lat) / (pLatEntries - 1);
pLonStep = (pNorthWest.Lon - pSouthEast.Lon) / (pLonEntries - 1);

// precalculate some common factors that we don't need to store
PrecalculateCommonFactors();

} // DEM::SetBounds

/*===========================================================================*/


// used for DEM merging - don't call unless you ensure there's valid Map & entry settings
// set 0 based column & row as seen in DEM Editor (note: Editor is 1 based)
void DEM::SetCell(unsigned long col, unsigned long row, float elev)
{

*(Map() + col * pLatEntries + pLatEntries - row - 1) = elev;

} // DEM::SetCell

/*===========================================================================*/

void DEM::FreeVertices(void)
{

if (Vertices)
	{
	delete [] Vertices;
	Vertices = NULL;
	} // if

} // DEM::FreeVertices

/*===========================================================================*/

short DEM::AllocVertices(void)
{
short rVal = 0;

if (MapSize() > 0)
	{
	if (Vertices = new VertexDEM[MapSize()])
		rVal = 1;
	} // if

return (rVal);

} // DEM::AllocVertices

/*===========================================================================*/

int DEM::TransferToVerticesXYZ(void)
{
double CurLat, CurLon;
unsigned long LonCt, LatCt, Ct;
int rVal = 0;

if (Vertices || AllocVertices())
	{
	CurLon = Westest();
	for (Ct = LonCt = 0; LonCt < pLonEntries; LonCt ++, CurLon -= pLonStep)
		{
		CurLat = Southest(); 
		for (LatCt = 0; LatCt < pLatEntries; Ct ++, LatCt ++, CurLat += pLatStep)
			{
			Vertices[Ct].xyz[1] = CurLat;
			Vertices[Ct].xyz[0] = CurLon;
			Vertices[Ct].xyz[2] = RawMap[Ct];
			} // for
		} // for
	rVal = 1;
	} // if

return (rVal);

} // DEM::TransferToVerticesXYZ

/*===========================================================================*/

// When range254 is true, values between two ref elevs map to 1..254.  Otherwise, they map to 0..255.
// Raster is dumped in traditional North up manner.
bool DEM::DumpRaster(const char *filename, float refMin, float refMax, bool range254)
{
FILE *fOut;
float elev, delta;
unsigned long x, y;
size_t wrote; 
unsigned char outVal;
bool success = false;

delta = refMax - refMin;

if ((delta != 0.0f) && (fOut = PROJ_fopen(filename, "wb")))
	{
	if (range254)
		{
		y = pLatEntries - 1;
		do
			{
			for (x = 0; x < pLonEntries; x++)
				{
				elev = Sample(x, y);
				if (elev < refMin)
					outVal = 0;
				else if (elev > refMax)
					outVal = 255;
				else
					outVal = (unsigned char)((elev - refMin) / delta * 253.0 + 1.0f);
				wrote = fwrite(&outVal, 1, 1, fOut);
				} // for x
			--y;
			} while (y < pLatEntries);	// when it wraps, we exit
		} // if range254
	else
		{
		y = pLatEntries - 1;
		do
			{
			for (x = 0; x < pLonEntries; x++)
				{
				elev = Sample(x, y);
				if (elev < refMin)
					outVal = 0;
				else if (elev > refMax)
					outVal = 255;
				else
					outVal = (unsigned char)((elev - refMin) / delta * 255.0);
				wrote = fwrite(&outVal, 1, 1, fOut);
				} // for x
			--y;
			} while (y < pLatEntries);	// when it wraps, we exit
		} // else range254
	if (wrote == 1)
		success = true;
	fclose(fOut);
	} // if

return(success);

} // DumpRaster

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS

short DEM::AllocVPData(void)
{
short rVal = 0;

if (VPDataMapEntries() > 0)
	{
	if (VPData = (VectorPolygonListDouble **)AppMem_Alloc(VPDataMapSize(), APPMEM_CLEAR))
		rVal = 1;
	LastTouchedTime = 0;	// initialize this so it is absolutely the oldest one on the DEMCue until it is actually used.
	} // if

return (rVal);

} // DEM::AllocVPData

/*===========================================================================*/

unsigned long DEM::FreeVPData(bool ForceFree)
{
unsigned long ItemsFreed = 0;

if (VPData)
	{
	#ifndef WCS_VECPOLY_FREEINDIVIDUAL_DEMCELLDATA
	if (ForceFree)
	#endif // WCS_VECPOLY_FREEINDIVIDUAL_DEMCELLDATA
		{
		unsigned long Size, Pt;
		VectorPolygonListDouble *CurPolyList;
		// This memory is actually freed when the QuantaAllocator releases its blocks.
		// But if the data is needed prior to final release then each individual item must be released.
		Size = (long)VPDataMapEntries();

		for (Pt = 0; Pt < Size; ++Pt)
			{
			for (CurPolyList = VPData[Pt]; CurPolyList; CurPolyList = VPData[Pt])
				{
				VPData[Pt] = (VectorPolygonListDouble *)VPData[Pt]->NextPolygonList;
				CurPolyList->DeletePolygon();
				delete CurPolyList;
				++ItemsFreed;
				} // for
			} // for
		} // if

	AppMem_Free(VPData, VPDataMapSize());
	VPData = NULL;
	LastTouchedTime = 0;
	} // if

return (ItemsFreed);

} // DEM::FreeVPData

/*===========================================================================*/

unsigned long DEM::CalcFullMemoryReq(void)
{
unsigned long rVal = 0;

if (pLatEntries > 1 && pLonEntries > 1)
	rVal = VPDataMapSize() + FractalMapSize() + RelElMapSize() + RawMapSize() + VertexMapSize();

return (rVal);

} // DEM::CalcFullMemoryReq

/*===========================================================================*/

unsigned long DEM::VertexMapSize(void)
{

return(MapSize() * sizeof(VertexDEM));

} // DEM::VertexMapSize

#endif // WCS_VECPOLY_EFFECTS
