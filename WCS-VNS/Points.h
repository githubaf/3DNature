// Points.h
// Stuff for allocating, tracking and using Point objects
// Pulled out of Joe.cpp on 4/4/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_POINTS_H
#define WCS_POINTS_H

class Database;

class VectorPoint;
class PointAllocator;
class PointBlock;
class CoordSys;
class VertexDEM;

// ~64k-80k bytes per block, depending on architecture padding and alignment
#define WCS_POINTS_IN_A_BLOCK 2730

#define WCS_POINTS_FLAG_MARKEDFORDEATH			1 << 0; // Used to indicate expunge

// VectorPoint (24 bytes)
class VectorPoint
{
friend class PointAllocator; // This has little meaning when there are no
                             // private members, but I thought I'd mention it.
public:
	VectorPoint *Next;
	double Latitude, Longitude;
	float Elevation;

	int ProjToDefDeg(CoordSys *SourceCoords, VertexDEM *Vert);
	int DefDegToProj(CoordSys *SourceCoords, VertexDEM *Vert);
	int DefDegToProjElev(CoordSys *SourceCoords, VertexDEM *Vert);
	int SamePoint(VectorPoint *CompareMe);
	int SamePointLatLon(VectorPoint *CompareMe);
}; // VectorPoint

// PointBlock (16 + (WCS_POINTS_IN_A_BLOCK * sizeof(VectorPoint))) bytes)
//             16 + (2730              * 24) = 65536
class PointBlock
{
friend class PointAllocator; // Tho we have no private members...

// This whole shebang should pad out to 64k.
// Why? I'dunno, seemed like a good idea.
PointBlock *NextBlock, *PrevBlock;
unsigned long AvailableBlocks, Flags;
VectorPoint Points[WCS_POINTS_IN_A_BLOCK];
}; // PointBlock

// PointAllocator
class PointAllocator
{
friend class Database;
private:
	VectorPoint *FreeList;
	PointBlock *BlockList;
	unsigned long NumAllocated, NumAvail;

	void InitNewBlock(PointBlock *Me);
public:
	int DestroyAll(void);

//public:
	inline PointAllocator() {FreeList = NULL; BlockList = NULL; NumAllocated = NumAvail = 0;};
	~PointAllocator() {(void)DestroyAll();FreeList = NULL; BlockList = NULL;};
	VectorPoint *Allocate(unsigned long NumPoints);
	void DeAllocate(VectorPoint *Chain);
	int DisposeAll(void);
	inline unsigned long HowManyAllocated(void) {return(NumAllocated);};

}; // PointAllocator

#endif // WCS_POINTS_H
