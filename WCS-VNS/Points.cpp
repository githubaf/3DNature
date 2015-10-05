// Points.cpp
// Code to manipulate points and pointblocks and pointallocators and other
// pointless things like that.
// Finally written from scratch on 21 Apr 1995 by Christopher Eric "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "Points.h"
#include "Render.h"
#include "EffectsLib.h"
#include "AppMem.h"
#include "Useful.h"

void PointAllocator::InitNewBlock(PointBlock *Me)
{
int Loop;

Me->PrevBlock = NULL;
Me->NextBlock = BlockList;
BlockList = Me;
Me->AvailableBlocks = WCS_POINTS_IN_A_BLOCK;
NumAvail += WCS_POINTS_IN_A_BLOCK;
Me->Flags = NULL;

for(Loop = 0; Loop < WCS_POINTS_IN_A_BLOCK; Loop++)
	{
	if(Loop == (WCS_POINTS_IN_A_BLOCK - 1))
		{
		Me->Points[Loop].Next = FreeList;
		} // if
	else
		{
		Me->Points[Loop].Next = &Me->Points[Loop + 1];
		} // else
	} // for

FreeList = &Me->Points[0];
} // PointAllocator::InitNewBlock

int PointAllocator::DisposeAll(void)
{

if(NumAllocated)
	{
	return(0); // Sorry, still in use
	} // if

return(DestroyAll());
} // PointAllocator::DisposeAll

int PointAllocator::DestroyAll(void)
{
PointBlock *Victim, *NextVictim;
int Carnage = 0;


for(Victim = BlockList; Victim;)
	{
	NextVictim = Victim->NextBlock;
	AppMem_Free(Victim, sizeof(PointBlock));
	Carnage++;
	Victim = NextVictim;
	} // for


BlockList = NULL;
FreeList = NULL;
NumAllocated = NULL;
NumAvail = NULL;

return(Carnage);
} //PointAllocator::DestroyAll



VectorPoint *PointAllocator::Allocate(unsigned long int NumPoints)
{
PointBlock *Fresh;
VectorPoint *Booty, *Snip;
unsigned long int SnipCount;

while(NumPoints > NumAvail)
	{
	Fresh = (PointBlock *) AppMem_Alloc(sizeof (PointBlock), APPMEM_CLEAR, "Database Vertex Allocation");
	if(Fresh == NULL)
		{
		return(NULL);
		} // if
	InitNewBlock(Fresh);
	// if you're asking for more than WCS_POINTS_IN_A_BLOCK points, we'll have to
	// go back and get more blocks...
	} // while

Booty = FreeList; // Take blocks from the head of the list

// Now, walk along the list until we've traversed NumPoints blocks, and
// make that the new beginning of the list

Snip = FreeList;
SnipCount = 1;

while(SnipCount != NumPoints)
	{
	if(Snip)
		{
		Snip = Snip->Next;
		} // if
	else
		{
		// Log some kind of PointList-Corrupt message, or would that be pointless?
		printf("Point allocation error!\n");
		break;
		} // else
	SnipCount++;
	} // while

if(Snip)
	{
	FreeList = Snip->Next;
	Snip->Next = NULL;
	NumAllocated += NumPoints;
	NumAvail     -= NumPoints;
	return(Booty); // Success
	} // if
else
	{
	return(NULL);
	} // else

} // PointAllocator::Allocate


void PointAllocator::DeAllocate(VectorPoint *Chain)
{
VectorPoint *Link;
int FreedPoints = 0;

if(Chain)
	{
	for(Link = Chain; Link; Link = Link->Next)
		{
		FreedPoints++;
		if(Link->Next == NULL)
			{
			break;
			} // if
		} // for
	Link->Next = FreeList;
	FreeList = Chain;
	NumAvail     += FreedPoints;
	NumAllocated -= FreedPoints;
	} // if

} // PointAllocator::DeAllocate

/*===========================================================================*/

int VectorPoint::ProjToDefDeg(CoordSys *SourceCoords, VertexDEM *Vert)
{

#ifdef WCS_BUILD_VNS
// translate projected coords to global cartesian and then to default degrees
if (SourceCoords)
	{
	Vert->xyz[0] = Longitude;
	Vert->xyz[1] = Latitude;
	Vert->xyz[2] = Elevation;
	if (SourceCoords->ProjToDefDeg(Vert))
		return (1);
	} // if
else
	{
#endif // WCS_BUILD_VNS
	// coordinates are already in default degrees
	Vert->Lon = Longitude;
	Vert->Lat = Latitude;
	Vert->Elev = Elevation;
	return (1);
#ifdef WCS_BUILD_VNS
	} // else

return (0);
#endif // WCS_BUILD_VNS

} // VectorPoint::ProjToDefDeg

/*===========================================================================*/

int VectorPoint::DefDegToProj(CoordSys *SourceCoords, VertexDEM *Vert)
{

#ifdef WCS_BUILD_VNS
// translate default degrees to global cartesian then to projected coords
if (SourceCoords)
	{
	if (SourceCoords->DefDegToProj(Vert))
		{
		Longitude = Vert->xyz[0];
		Latitude = Vert->xyz[1];
		Elevation = (float)Vert->xyz[2];
		return (1);
		} // if
	} // if
else
	{
#endif // WCS_BUILD_VNS
	// coordinates are already in default degrees
	Longitude = Vert->Lon;
	Latitude = Vert->Lat;
	Elevation = (float)Vert->Elev;
	return (1);
#ifdef WCS_BUILD_VNS
	} // else

return (0);
#endif // WCS_BUILD_VNS

} // VectorPoint::DefDegToProj

/*===========================================================================*/

int VectorPoint::DefDegToProjElev(CoordSys *SourceCoords, VertexDEM *Vert)
{

// by definition elevation doesn't change between different projections and datums
Elevation = (float)Vert->Elev;
return (1);

/* if elevations were changed by translating between datums
#ifdef WCS_BUILD_VNS
// translate default degrees to global cartesian then to projected coords
if (SourceCoords)
	{
	if (SourceCoords->DefDegToProj(Vert))
		{
		Elevation = (float)Vert->xyz[2];
		return (1);
		} // if
	} // if
else
	{
#endif // WCS_BUILD_VNS
	// coordinates are already in default degrees
	Elevation = (float)Vert->Elev;
	return (1);
#ifdef WCS_BUILD_VNS
	} // else

return (0);
#endif // WCS_BUILD_VNS
*/
} // VectorPoint::DefDegToProj

/*===========================================================================*/

int VectorPoint::SamePoint(VectorPoint *CompareMe)
{
double Tolerance;

// uses a tenth of a meter elevation tolerance
if (fabs(Elevation - CompareMe->Elevation) < .1)
	{
	// lat-lon tolerance works out to a tenth of a meter if geographic coords
	// if latitude is greater than 100 we assume we are dealing with units of meters
	// in either case the tolerance is about a centimeter
	Tolerance = (fabs(Latitude) < 100.0) ? 10.0E-8: .01;
	if (fabs(Latitude - CompareMe->Latitude) < Tolerance)
		{
		if (fabs(Longitude - CompareMe->Longitude) < Tolerance)
			{
			return (1);
			} // if
		} // if
	} // if

return (0);

} // VectorPoint::SamePoint

/*===========================================================================*/

int VectorPoint::SamePointLatLon(VectorPoint *CompareMe)
{
double Tolerance;

// lat-lon tolerance works out to a tenth of a meter if geographic coords
// if latitude is greater than 100 we assume we are dealing with units of meters
// in either case the tolerance is about a centimeter
Tolerance = (fabs(Latitude) < 100.0) ? 10.0E-8: .01;
if (fabs(Latitude - CompareMe->Latitude) < Tolerance)
	{
	if (fabs(Longitude - CompareMe->Longitude) < Tolerance)
		{
		return (1);
		} // if
	} // if

return (0);

} // VectorPoint::SamePoint

