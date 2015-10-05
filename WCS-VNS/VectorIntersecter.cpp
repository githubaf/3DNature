// VectorIntersecter.cpp
// Header file for VectorIntersecter class
// Created by Gary R. Huber 2/3/06
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "VectorIntersecter.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "Lists.h"
#include "Points.h"
#include "UsefulMath.h"
#include "Useful.h"		// for DEBUGOUT
#include "Joe.h"

using namespace std;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

// enables intersector type counting
//#define DEBUG_INTERSECTOR_TYPE

#ifdef DEBUG_INTERSECTOR_TYPE
unsigned long int IntersectorTypeCt[WCS_NUM_INTERSECTOR_TYPES];
#endif // DEBUG_INTERSECTOR_TYPE

VectorIntersecter::VectorIntersecter()
{

#ifdef DEBUG_INTERSECTOR_TYPE
for (unsigned long int Ct = 0; Ct < WCS_NUM_INTERSECTOR_TYPES; ++Ct)
	IntersectorTypeCt[Ct] = 0;
#endif // DEBUG_INTERSECTOR_TYPE

} // VectorIntersecter::VectorIntersecter

/*===========================================================================*/

VectorIntersecter::~VectorIntersecter()
{

} // VectorIntersecter::~VectorIntersecter

/*===========================================================================*/

bool VectorIntersecter::TestIntersect(VectorPolygon *VP1, VectorPolygon *VP2, VectorPolygonListDouble *&AddedPolygons, bool InitialMerge)
{
bool Success = true;

// create new set of VectorPolygons if the two polygons overlap
// if there is no intersection return NULL and leave the original polygons as they are.
// intersection means segments that cross each other or portions of area that overlap.
// polygons that merely share some edges are not intersecting.
// if the two polygons are identical then only one polygon will be in the output set and the 
// original polygons' attributes will be combined.
// otherwise the output set will contain three or more polygons depending on the intersection geometry.
// the output polygons will not contain any overlapping regions but will contain all regions
// contained by the original polygons and no more.

if (TestBBoxOverlap(VP1, VP2, WCS_VECTORPOLYGON_NODECOORD_TOLERANCE))
	{
	Success = FindIntersectionsLinkNodes(VP1, VP2, AddedPolygons, InitialMerge);
	} // if

return (Success);

} // VectorIntersecter::TestIntersect

/*===========================================================================*/

bool VectorIntersecter::TestBBoxOverlap(VectorPolygon * const VP1, VectorPolygon * const VP2)
{

// first see if the bounding boxes of the two polygons overlap at all
if (! VP1->BBox)
	VP1->SetBoundingBox();
if (! VP2->BBox)
	VP2->SetBoundingBox();

if (! VP1->BBox || ! VP2->BBox)
	return (false);	// error

// elliminate the most trivial case of non-adjacency or overlap
if (VP1->BBox->MinX > VP2->BBox->MaxX)
	return (false);
if (VP2->BBox->MinX > VP1->BBox->MaxX)
	return (false);
if (VP1->BBox->MinY > VP2->BBox->MaxY)
	return (false);
if (VP2->BBox->MinY > VP1->BBox->MaxY)
	return (false);

return (true);

} // VectorIntersecter::TestBBoxOverlap

/*===========================================================================*/

bool VectorIntersecter::TestBBoxOverlap(VectorPolygon * const VP1, VectorPolygon * const VP2, double Tolerance)
{
bool rVal = true;

// first see if the bounding boxes of the two polygons overlap at all
if (! VP1->BBox)
	VP1->SetBoundingBox();
if (! VP2->BBox)
	VP2->SetBoundingBox();

if (! VP1->BBox || ! VP2->BBox)
	rVal = false;	// error
// elliminate the most trivial case of non-adjacency or overlap
else if (VP1->BBox->MinX > VP2->BBox->MaxX + Tolerance)
	rVal = false;
else if (VP2->BBox->MinX > VP1->BBox->MaxX + Tolerance)
	rVal = false;
else if (VP1->BBox->MinY > VP2->BBox->MaxY + Tolerance)
	rVal = false;
else if (VP2->BBox->MinY > VP1->BBox->MaxY + Tolerance)
	rVal = false;

return (rVal);

} // VectorIntersecter::TestBBoxOverlap

/*===========================================================================*/

bool VectorIntersecter::TestBBoxOverlap(VectorPolygon * const VP1, PolygonBoundingBox * const BoundLimits)
{
bool rVal = true;

// first see if the bounding boxes of the two polygons overlap at all
if (! VP1->BBox)
	VP1->SetBoundingBox();

if (! VP1->BBox || ! BoundLimits)
	rVal = false;	// error
// elliminate the most trivial case of non-adjacency or overlap
else if (VP1->BBox->MinX > BoundLimits->MaxX)
	rVal = false;
else if (BoundLimits->MinX > VP1->BBox->MaxX)
	rVal = false;
else if (VP1->BBox->MinY > BoundLimits->MaxY)
	rVal = false;
else if (BoundLimits->MinY > VP1->BBox->MaxY)
	rVal = false;

return (rVal);

} // VectorIntersecter::TestBBoxOverlap

/*===========================================================================*/

bool VectorIntersecter::TestBoundingBoxContained(VectorPolygon * const Smaller, VectorPolygon * const Larger)
{

return (Smaller->BBox->MinX > Larger->BBox->MinX && Smaller->BBox->MaxX < Larger->BBox->MaxX
	&& Smaller->BBox->MinY > Larger->BBox->MinY && Smaller->BBox->MaxY < Larger->BBox->MaxY);

} // VectorIntersecter::TestBoundingBoxContained

/*===========================================================================*/

//#define GORILLA

bool VectorIntersecter::FindIntersectionsLinkNodes(VectorPolygon *VP1, VectorPolygon *VP2, VectorPolygonListDouble *&AddedPolygons, bool InitialMerge)
{
#ifdef GORILLA
double iMaxX, iMaxY, iMinX, iMinY;
#endif // GORILLA
double Tolerance, NodeLat, NodeLon, NodeElev, Min1X, Min1Y, Max1X, Max1Y;
#ifdef GORILLA
VectorNode *outNodes[65535];
VectorPart *outParts[65535];
#endif // GORILLA
VectorNode *VP1Node, *VP2Node, *NewNode, *NewNode2, *NodeStash, *NodeStash2, *VP1NextNodeStash;
VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
VectorPart *VP1Part, *VP2Part;
#ifdef GORILLA
VectorPart *lastPart = NULL;
#endif // GORILLA
VectorPart *NewPart;
VectorPolygon *SmallerPoly, *LargerPoly;
VectorPolygonListDouble *InsideList, **NewList;
unsigned long VP1NodeCt, VP2NodeCt, Num1Parts, Num2Parts, Part1Ct, Part2Ct, *Part1Contacts, *Part2Contacts;
#ifdef GORILLA
unsigned long outCount = 0;
#endif // GORILLA
bool Success = true, SomeContactFound = false, SegmentContactFound, PartContactFound;

Tolerance = WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
Num1Parts = VP1->GetFirstPart()->NextPart ? VP1->CountParts(): 1;
Num2Parts = VP2->GetFirstPart()->NextPart ? VP2->CountParts(): 1;
NewList = &AddedPolygons;

if (Num1Parts > 1)
	{
	Part1Contacts = new unsigned long[Num1Parts];
	memset(Part1Contacts, 0, Num1Parts * sizeof (unsigned long));
	} // if
else
	Part1Contacts = NULL;
if (Num2Parts > 1)
	{
	Part2Contacts = new unsigned long[Num2Parts];
	memset(Part2Contacts, 0, Num2Parts * sizeof (unsigned long));
	} // if
else
	Part2Contacts = NULL;
	
#ifdef GORILLA
// determine region of intersection
iMaxX = _MAX(VP1->BBox->MaxX, VP2->BBox->MaxX); //+ WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMinX = _MIN(VP1->BBox->MinX, VP2->BBox->MinX); //- WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMaxY = _MAX(VP1->BBox->MaxY, VP2->BBox->MaxY); //+ WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
iMinY = _MIN(VP1->BBox->MinY, VP2->BBox->MinY); //- WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

// make a list of only those nodes that create a segment in the intersection region
for (VP2Part = VP2->GetFirstPart(); VP2Part; VP2Part = VP2Part->NextPart)
	{
	for (VP2Node = VP2Part->FirstNode(), VP2NodeCt = 0; VP2NodeCt < VP2Part->NumNodes; ++VP2NodeCt, VP2Node = VP2Node->NextNode)
		{
		if (((VP2Node->Lon >= iMinX) && (VP2Node->Lon <= iMaxX)) || ((VP2Node->NextNode->Lon >= iMinX) && (VP2Node->NextNode->Lon <= iMaxX)) ||
			((VP2Node->Lat >= iMinY) && (VP2Node->Lat <= iMaxY)) || ((VP2Node->NextNode->Lat >= iMinY) && (VP2Node->NextNode->Lat <= iMaxY)))
			{
			outParts[outCount] = VP2Part;
			outNodes[outCount] = VP2Node;
			outCount++;
			} // if
		} // for
	} // for
#endif // GORILLA

// for each segment in one polygon, test to see if it intersects with the other polygon's segments or
// if the vertices are shared.

for (Part1Ct = 0, VP1Part = VP1->GetFirstPart(); VP1Part && Success; VP1Part = VP1Part->NextPart, ++Part1Ct)
	{
	for (VP1Node = VP1Part->FirstNode(), VP1NodeCt = 0; VP1NodeCt < VP1Part->NumNodes && Success; ++VP1NodeCt, VP1Node = VP1Node->NextNode)
		{
		VP1NextNodeStash = VP1Node->NextNode;
		Max1XNode = VP1Node->Lon > VP1NextNodeStash->Lon ? VP1Node: VP1NextNodeStash;
		Min1XNode = Max1XNode == VP1Node ? VP1NextNodeStash: VP1Node;
		Max1YNode = VP1Node->Lat > VP1NextNodeStash->Lat ? VP1Node: VP1NextNodeStash;
		Min1YNode = Max1YNode == VP1Node ? VP1NextNodeStash: VP1Node;
		Min1X = Min1XNode->Lon - Tolerance;
		Max1X = Max1XNode->Lon + Tolerance;
		Min1Y = Min1YNode->Lat - Tolerance;
		Max1Y = Max1YNode->Lat + Tolerance;
		#ifdef GORILLA
		for (unsigned long outloop = 0; Success && (outloop < outCount); outloop++)
			{
			VP2Part = outParts[outloop];
			VP2Node = outNodes[outloop];
			if (VP2Part != lastPart)
				{
				PartContactFound = false;
				lastPart = VP2Part;
				} // if
		#else // GORILLA
		for (Part2Ct = 0, VP2Part = VP2->GetFirstPart(); VP2Part && Success; VP2Part = VP2Part->NextPart, ++Part2Ct)
			{
			PartContactFound = false;
			for (VP2Node = VP2Part->FirstNode(), VP2NodeCt = 0; VP2NodeCt < VP2Part->NumNodes; ++VP2NodeCt, VP2Node = VP2Node->NextNode)
				{
				#endif // GORILLA
				Max2XNode = VP2Node->Lon > VP2Node->NextNode->Lon ? VP2Node: VP2Node->NextNode;
				Min2XNode = Max2XNode == VP2Node ? VP2Node->NextNode: VP2Node;
				if (Min2XNode->Lon <= Max1X && Max2XNode->Lon >= Min1X)
					{
					Max2YNode = VP2Node->Lat > VP2Node->NextNode->Lat ? VP2Node: VP2Node->NextNode;
					Min2YNode = Max2YNode == VP2Node ? VP2Node->NextNode: VP2Node;
					if (Min2YNode->Lat <= Max1Y && Max2YNode->Lat >= Min1Y)
						{
						SegmentContactFound = false;
						// are any of the vertices the same?
						if (VP1Node->SamePointLatLonOrLinked(VP2Node) && (VP1Node != VP2Node))
							{
							// link VP1Node and VP2Node
							Success = VP1Node->AddCrossLinks(VP2Node, VP1, VP2);
							SegmentContactFound = true;
							} // if
						else if (VP1Node->SamePointLatLonOrLinked(VP2Node->NextNode) && (VP1Node != VP2Node->NextNode))
							{
							// link VP1Node and VP2Node->NextNode
							Success = VP1Node->AddCrossLinks(VP2Node->NextNode, VP1, VP2);
							SegmentContactFound = true;
							} // else if
						// tricky: test to see if the second nodes are the same in which case the segments do not intersect even though they might appear to
						// using the imprecise slopes generated using the finite precision of computers
						else if (! VP2Node->SamePointLatLonOrLinked(VP1Node->NextNode) && ! VP1Node->NextNode->SamePointLatLonOrLinked(VP2Node->NextNode) 
							&& Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
							&& FindIntersectionContained(VP1Node, VP2Node, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
							Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
							{
							// intersection falls between end nodes on both segments
							if (VP1->TestNodeOnOrNearSegment(VP2Node, VP1Node, VP1Node->NextNode, NodeLat, NodeLon, Tolerance))
								{
								NodeStash = VP1Node->NextNode;
								// create node on Poly 1 at NodeLat, NodeLon and link with Node 2
								if (NewNode = VP1->InsertNode(VP1Part, VP1Node, NodeLat, NodeLon, VP2Node->Elev))
									{
									NewNode->InheritBoundaryFlagsFromParents(VP1Node, NodeStash);
									NewNode->InheritBoundaryFlagsFromParent(VP2Node);
									Success = (NewNode->AddCrossLinks(VP2Node, VP1, VP2) &&
										ReplicateNodeOnSharedSegments(NewNode, VP1Node, NodeStash, VP1, VP2));
									SegmentContactFound = true;
									#ifdef GORILLA
									outParts[outCount] = VP2Part;
									outNodes[outCount] = NewNode;
									outCount++;
									#endif // GORILLA
									} // if
								else
									Success = false;
								} // if
							else if (VP1->TestNodeOnOrNearSegment(VP2Node->NextNode, VP1Node, VP1Node->NextNode, NodeLat, NodeLon, Tolerance))
								{
								NodeStash = VP1Node->NextNode;
								// create node on Poly 1 at NodeLat, NodeLon and link with Node 2 next node
								if (NewNode = VP1->InsertNode(VP1Part, VP1Node, NodeLat, NodeLon, VP2Node->NextNode->Elev))
									{
									NewNode->InheritBoundaryFlagsFromParents(VP1Node, NodeStash);
									NewNode->InheritBoundaryFlagsFromParent(VP2Node->NextNode);
									Success = (NewNode->AddCrossLinks(VP2Node->NextNode, VP1, VP2) &&
										ReplicateNodeOnSharedSegments(NewNode, VP1Node, NodeStash, VP1, VP2));
									SegmentContactFound = true;
									#ifdef GORILLA
									outParts[outCount] = VP2Part;
									outNodes[outCount] = NewNode;
									outCount++;
									#endif // GORILLA
									} // if
								else
									Success = false;
								} // if
							else
								{
								// create two nodes and link
								NodeStash = VP1Node->NextNode;
								NodeStash2 = VP2Node->NextNode;
								// intersection falls on the line segment
								if (VP1Node->Lon != VP1Node->NextNode->Lon)
									NodeElev = VP1Node->Elev + (VP1Node->NextNode->Elev - VP1Node->Elev) * (NodeLon - VP1Node->Lon) / (VP1Node->NextNode->Lon - VP1Node->Lon);
								else
									NodeElev = VP1Node->Elev + (VP1Node->NextNode->Elev - VP1Node->Elev) * (NodeLat - VP1Node->Lat) / (VP1Node->NextNode->Lat - VP1Node->Lat);
				
								// do we need a new vertex on VP1?
								// how about on VP2? The intersection may fall very close to the end of either segment
								// and adding nodes creates nodes too close together which then causes redundant points when
								// when consolidated later.
								// Either create a new node on VP1 or assign the node to link with
								if (VP1Node->SimilarPointLatLon(NodeLat, NodeLon, Tolerance))
									NewNode = VP1Node;
								else if	(VP1Node->NextNode->SimilarPointLatLon(NodeLat, NodeLon, Tolerance))
									NewNode = VP1Node->NextNode;
								else
									{
									if (NewNode = VP1->InsertNode(VP1Part, VP1Node, NodeLat, NodeLon, NodeElev))
										{
										NewNode->InheritBoundaryFlagsFromParents(VP1Node, NodeStash);
										NewNode->InheritBoundaryFlagsFromParents(VP2Node, NodeStash2);
										Success = ReplicateNodeOnSharedSegments(NewNode, VP1Node, NodeStash, VP1, VP2);
										} // if
									else
										Success = false;
									} // else
								if (VP2Node->SimilarPointLatLon(NodeLat, NodeLon, Tolerance))
									NewNode2 = VP2Node;
								else if (VP2Node->NextNode->SimilarPointLatLon(NodeLat, NodeLon, Tolerance))
									NewNode2 = VP2Node->NextNode;
								else
									{
									if (NewNode2 = VP2->InsertNode(VP2Part, VP2Node, NodeLat, NodeLon, NodeElev))
										{
										NewNode2->InheritBoundaryFlagsFromParents(VP1Node, NodeStash);
										NewNode2->InheritBoundaryFlagsFromParents(VP2Node, NodeStash2);
										Success = ReplicateNodeOnSharedSegments(NewNode2, VP2Node, NodeStash2, VP2, VP1);
										} // if
									else
										Success = false;
									} // else
								if (Success)
									Success = NewNode->AddCrossLinks(NewNode2, VP1, VP2);
								SegmentContactFound = true;
								#ifdef GORILLA
								outParts[outCount] = VP2Part;
								outNodes[outCount] = NewNode2;
								outCount++;
								#endif // GORILLA
								} // else
							} // else if
						else if (VP1Node != VP2Node && VP1Node != VP2Node->NextNode && VP2Node != VP1Node->NextNode)
							{
							if (VP1->TestNodeOnOrNearSegment(VP1Node, VP2Node, VP2Node->NextNode, NodeLat, NodeLon, Tolerance))
								{
								NodeStash2 = VP2Node->NextNode;
								// create node on Poly 2 at NodeLat, NodeLon and link with Node 1
								if (NewNode = VP2->InsertNode(VP2Part, VP2Node, NodeLat, NodeLon, VP1Node->Elev))
									{
									NewNode->InheritBoundaryFlagsFromParents(VP2Node, NodeStash2);
									NewNode->InheritBoundaryFlagsFromParent(VP1Node);
									Success = (VP1Node->AddCrossLinks(NewNode, VP1, VP2) &&
										ReplicateNodeOnSharedSegments(NewNode, VP2Node, NodeStash2, VP2, VP1));
									SegmentContactFound = true;
									#ifdef GORILLA
									outParts[outCount] = VP2Part;
									outNodes[outCount] = NewNode;
									outCount++;
									#endif // GORILLA
									} // if
								else
									Success = false;
								} // if
							else if (VP1->TestNodeOnOrNearSegment(VP2Node, VP1Node, VP1Node->NextNode, NodeLat, NodeLon, Tolerance))
								{
								NodeStash = VP1Node->NextNode;
								// create node on Poly 2 at NodeLat, NodeLon and link with Node 1
								if (NewNode = VP1->InsertNode(VP1Part, VP1Node, NodeLat, NodeLon, VP2Node->Elev))
									{
									NewNode->InheritBoundaryFlagsFromParents(VP1Node, NodeStash);
									NewNode->InheritBoundaryFlagsFromParent(VP2Node);
									Success = (NewNode->AddCrossLinks(VP2Node, VP1, VP2) &&
										ReplicateNodeOnSharedSegments(NewNode, VP1Node, NodeStash, VP1, VP2));
									SegmentContactFound = true;
									#ifdef GORILLA
									outParts[outCount] = VP2Part;
									outNodes[outCount] = NewNode;
									outCount++;
									#endif // GORILLA
									} // if
								else
									Success = false;
								} // else if
							else if (VP1Node->SimilarPointLatLon(VP2Node, Tolerance))
								{
								// link Node 1 with Node 2
								Success = VP1Node->AddCrossLinks(VP2Node, VP1, VP2);
								SegmentContactFound = true;
								} // else if
							} // else if
						if (! Success)
							{
							break;
							} // if
						
						if (SegmentContactFound)
							{
							if (VP1NextNodeStash != VP1Node->NextNode)
								{
								VP1NextNodeStash = VP1Node->NextNode;
								Max1XNode = VP1Node->Lon > VP1NextNodeStash->Lon ? VP1Node: VP1NextNodeStash;
								Min1XNode = Max1XNode == VP1Node ? VP1NextNodeStash: VP1Node;
								Max1YNode = VP1Node->Lat > VP1NextNodeStash->Lat ? VP1Node: VP1NextNodeStash;
								Min1YNode = Max1YNode == VP1Node ? VP1NextNodeStash: VP1Node;
								Min1X = Min1XNode->Lon - Tolerance;
								Max1X = Max1XNode->Lon + Tolerance;
								Min1Y = Min1YNode->Lat - Tolerance;
								Max1Y = Max1YNode->Lat + Tolerance;
								} // if
							PartContactFound = true;
							SomeContactFound = true;
							} // if
						} // if
					} // if
			#ifndef GORILLA
				} // for
			#endif // !GORILLA
			if (PartContactFound)
				{
				// might need to remove one part from the other
				if (Part1Contacts)
					Part1Contacts[Part1Ct] = 1;
				if (Part2Contacts)
					Part2Contacts[Part2Ct] = 1;
				} // if
			} // for
		} // for
	} // for

if (Success)
	{
	if (! SomeContactFound)
		{
		// polygons do not touch but they do have overlapping bounding boxes.
		// test to see if one polygon is totally inside the other
		// Since there were not common nodes or crossing segments, a simple test is if a node from the smaller of the two
		// polygons is inside the larger
		SmallerPoly = VP1->PolygonArea() < VP2->PolygonArea() ? VP1: VP2;
		LargerPoly = SmallerPoly == VP1 ? VP2: VP1;
		if (LargerPoly->TestPointContained(SmallerPoly->PolyFirstNode(), 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE)
			{
			// make a copy of SmallerPoly and reverse its direction
			// Remove it as a hole from larger poly
			// add effects of larger poly to smaller poly
			if (InsideList = new VectorPolygonListDouble())
				{
				// copies the vector, nodes and node links but does not cross-link the polygon with the clone
				if (InsideList->MakeVectorPolygon(SmallerPoly, NULL, true))
					{
					// ClonePointer was set but isn't needed here and in fact can make for trouble
					SmallerPoly->SetClonePointer(NULL);
					// adds cross-links
					if (SmallerPoly->CrossLink(InsideList->GetPolygon()))
						{
						// makes a negative out of a positive
						if (InsideList->ReverseDirection())
							{
							// adds a hole and resolves the cross-linked polygon references
							if (LargerPoly->RemoveHoles(InsideList))
								{
								// add outside polygon's effects to inside polygon
								if (! SmallerPoly->SupplementEffects(LargerPoly))
									Success = false;
								} // if
							else
								Success = false;
							} // if
						else
							Success = false;
						} // if
					else
						Success = false;
					} // if
				else
					Success = false;
				if (InsideList)
					{
					InsideList->DeletePolygon();
					delete InsideList;
					} // if
				} // if
			else
				Success = false;
			} // if
		} // if
	else
		{
		if (Part1Contacts)
			{
			for (Part1Ct = 1, VP1Part = VP1->GetFirstPart()->NextPart; Part1Ct < Num1Parts; VP1Part = VP1Part->NextPart, ++Part1Ct)
				{
				if (! Part1Contacts[Part1Ct])
					{
					// test if part is inside poly 2
					if (VP2->TestPointContained(VP1Part->FirstNode(), 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE)
						{
						// make a copy of VP1Part
						// add it as a part to VP2 poly
						if (*NewList = new VectorPolygonListDouble())
							{
							// make a copy of the inside part and add it to VP2
							if (NewPart = VP2->ClonePart(VP1Part))
								{
								// crosslink the parts
								if (VP1Part->CrossLink(NewPart, VP1, VP2))
									{
									// make a new polygon as a copy of VP1Part inverted
									// copies the part, nodes and node links but does not cross-link the polygon with the clone
									if ((*NewList)->MakeVectorPolygon(VP1Part, true, VP1))
										{
										// adds cross-links
										if (VP1Part->CrossLink((*NewList)->GetPolygon()->GetFirstPart(), VP1, (*NewList)->GetPolygon()))
											{
											// makes a positive out of a negative
											if ((*NewList)->ReverseDirection())
												{
												// add VP2's effects to new polygon
												if (! (*NewList)->GetPolygon()->SupplementEffects(VP2))
													Success = false;
												} // if
											else
												Success = false;
											} // if
										else
											Success = false;
										} // if
									else
										Success = false;
									} // if
								else
									Success = false;
								} // if
							else
								Success = false;
							NewList = (VectorPolygonListDouble **)&(*NewList)->NextPolygonList;
							} // if
						else
							Success = false;
						} // if
					} // if
				} // for
			} // if
		if (Part2Contacts)
			{
			for (Part2Ct = 1, VP2Part = VP2->GetFirstPart()->NextPart; Part2Ct < Num2Parts; VP2Part = VP2Part->NextPart, ++Part2Ct)
				{
				if (! Part2Contacts[Part2Ct])
					{
					// test if part is inside poly 1
					if (VP1->TestPointContained(VP2Part->FirstNode(), 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE)
						{
						// make a copy of VP2Part
						// add it as a part to VP1 poly
						if (*NewList = new VectorPolygonListDouble())
							{
							// make a copy of the inside part and add it to VP1
							if (NewPart = VP1->ClonePart(VP2Part))
								{
								// crosslink the parts
								if (VP2Part->CrossLink(NewPart, VP2, VP1))
									{
									// make a new polygon as a copy of VP2Part inverted
									// copies the part, nodes and node links but does not cross-link the polygon with the clone
									if ((*NewList)->MakeVectorPolygon(VP2Part, true, VP2))
										{
										// adds cross-links
										if (VP2Part->CrossLink((*NewList)->GetPolygon()->GetFirstPart(), VP2, (*NewList)->GetPolygon()))
											{
											// makes a positive out of a negative
											if ((*NewList)->ReverseDirection())
												{
												// add VP1's effects to new polygon
												if (! (*NewList)->GetPolygon()->SupplementEffects(VP1))
													Success = false;
												} // if
											else
												Success = false;
											} // if
										else
											Success = false;
										} // if
									else
										Success = false;
									} // if
								else
									Success = false;
								} // if
							else
								Success = false;
							NewList = (VectorPolygonListDouble **)&(*NewList)->NextPolygonList;
							} // if
						else
							Success = false;
						} // if
					} // if
				} // for
			} // if
		} // else
	} // if
	
if (Part1Contacts)
	delete [] Part1Contacts;
if (Part2Contacts)
	delete [] Part2Contacts;
	
// so now we have two vector polygons with any intersections cross-noded on both polygons
return (Success);

} // VectorIntersecter::FindIntersectionsLinkNodes

/*===========================================================================*/

bool VectorIntersecter::IntersectSegmentsLinkNodes(VectorPolygon * const Poly1, VectorPolygon * const Poly2, 
	VectorPart * const Part1, VectorPart * const Part2, VectorNode * const Node1, VectorNode * const Node2,
	bool &ContactFound)
{
double NodeLat, NodeLon, NodeElev, Tolerance;
VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode, 
	*NewNode, *NewNode2, *NodeStash, *NodeStash2;
bool Success = true;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//DevCounter[3]++;
//#endif // FRANK or GARY

Tolerance = WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

// test bounding box overlap
Max1XNode = Node1->Lon > Node1->NextNode->Lon ? Node1: Node1->NextNode;
Max2XNode = Node2->Lon > Node2->NextNode->Lon ? Node2: Node2->NextNode;
Min1XNode = Max1XNode == Node1 ? Node1->NextNode: Node1;
Min2XNode = Max2XNode == Node2 ? Node2->NextNode: Node2;

if (Min2XNode->Lon <= Max1XNode->Lon + Tolerance && Max2XNode->Lon >= Min1XNode->Lon - Tolerance)
	{
	//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
	//DevCounter[4]++;
	//#endif // FRANK or GARY
	Max1YNode = Node1->Lat > Node1->NextNode->Lat ? Node1: Node1->NextNode;
	Max2YNode = Node2->Lat > Node2->NextNode->Lat ? Node2: Node2->NextNode;
	Min1YNode = Max1YNode == Node1 ? Node1->NextNode: Node1;
	Min2YNode = Max2YNode == Node2 ? Node2->NextNode: Node2;
	if (Min2YNode->Lat <= Max1YNode->Lat + Tolerance && Max2YNode->Lat >= Min1YNode->Lat - Tolerance)
		{
		//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
		//DevCounter[5]++;
		//#endif // FRANK or GARY
		// are any of the vertices the same?
		if (Node1->SamePointLatLon(Node2) && (Node1 != Node2))
			{
			// link Node1 and Node2
			Success = Node1->AddCrossLinks(Node2, Poly1, Poly2);
			ContactFound = true;
			} // if
		else if (Node1->SamePointLatLon(Node2->NextNode) && (Node1 != Node2->NextNode))
			{
			// link Node1 and Node2->NextNode
			Success = Node1->AddCrossLinks(Node2->NextNode, Poly1, Poly2);
			ContactFound = true;
			} // else if
		// tricky: test to see if the second nodes are the same in which case the segments do not intersect even though they might appear to
		// using the imprecise slopes generated using the finite precision of computers
		else if (! Node2->SamePointLatLon(Node1->NextNode) && ! Node1->NextNode->SamePointLatLon(Node2->NextNode) 
			&& Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
			&& FindIntersectionContained(Node1, Node2, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
			Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
			{
			// intersection falls between end nodes on both segments

			if (Poly1->TestNodeOnOrNearSegment(Node2, Node1, Node1->NextNode, NodeLat, NodeLon, Tolerance))
				{
				NodeStash = Node1->NextNode;
				// create node on Poly 1 at NodeLat, NodeLon and link with Node 2
				if (NewNode = Poly1->InsertNode(Part1, Node1, NodeLat, NodeLon, Node2->Elev))
					{
					NewNode->InheritBoundaryFlagsFromParents(Node1, NodeStash);
					NewNode->InheritBoundaryFlagsFromParent(Node2);
					Success = (NewNode->AddCrossLinks(Node2, Poly1, Poly2) &&
						ReplicateNodeOnSharedSegments(NewNode, Node1, NodeStash, Poly1, Poly2));
					ContactFound = true;
					} // if
				else
					Success = false;
				} // if
			else if (Poly1->TestNodeOnOrNearSegment(Node2->NextNode, Node1, Node1->NextNode, NodeLat, NodeLon, Tolerance))
				{
				NodeStash = Node1->NextNode;
				// create node on Poly 1 at NodeLat, NodeLon and link with Node 2 next node
				if (NewNode = Poly1->InsertNode(Part1, Node1, NodeLat, NodeLon, Node2->NextNode->Elev))
					{
					NewNode->InheritBoundaryFlagsFromParents(Node1, NodeStash);
					NewNode->InheritBoundaryFlagsFromParent(Node2->NextNode);
					Success = (NewNode->AddCrossLinks(Node2->NextNode, Poly1, Poly2) &&
						ReplicateNodeOnSharedSegments(NewNode, Node1, NodeStash, Poly1, Poly2));
					ContactFound = true;
					} // if
				else
					Success = false;
				} // if
			else
				{
				// create two nodes and link
				NodeStash = Node1->NextNode;
				NodeStash2 = Node2->NextNode;
				// intersection falls on the line segment
				if (Node1->Lon != Node1->NextNode->Lon)
					NodeElev = Node1->Elev + (Node1->NextNode->Elev - Node1->Elev) * (NodeLon - Node1->Lon) / (Node1->NextNode->Lon - Node1->Lon);
				else
					NodeElev = Node1->Elev + (Node1->NextNode->Elev - Node1->Elev) * (NodeLat - Node1->Lat) / (Node1->NextNode->Lat - Node1->Lat);
				if (NewNode = Poly1->InsertNode(Part1, Node1, NodeLat, NodeLon, NodeElev))
					{
					NewNode->InheritBoundaryFlagsFromParents(Node1, NodeStash);
					NewNode->InheritBoundaryFlagsFromParents(Node2, NodeStash2);
					if (NewNode2 = Poly2->InsertNode(Part2, Node2, NewNode))
						{
						// Are the new nodes also on other segments that may now need to be updated with new nodes?
						// See if Node1 and NodeStash have linked nodes that go to the same polygon and are 
						// prev/next members of each other
						Success = (NewNode->AddCrossLinks(NewNode2, Poly1, Poly2) &&
							ReplicateNodeOnSharedSegments(NewNode, Node1, NodeStash, Poly1, Poly2) &&
							ReplicateNodeOnSharedSegments(NewNode2, Node2, NodeStash2, Poly2, Poly1));
						ContactFound = true;
						} // if
					else
						Success = false;
					} // if
				else
					Success = false;
				} // else
			} // else if
		else if (Node1 != Node2 && Node1 != Node2->NextNode && Node2 != Node1->NextNode)
			{
			if (Poly1->TestNodeOnOrNearSegment(Node1, Node2, Node2->NextNode, NodeLat, NodeLon, Tolerance))
				{
				NodeStash2 = Node2->NextNode;
				// create node on Poly 2 at NodeLat, NodeLon and link with Node 1
				if (NewNode = Poly2->InsertNode(Part2, Node2, NodeLat, NodeLon, Node1->Elev))
					{
					NewNode->InheritBoundaryFlagsFromParents(Node2, NodeStash2);
					NewNode->InheritBoundaryFlagsFromParent(Node1);
					Success = (Node1->AddCrossLinks(NewNode, Poly1, Poly2) &&
						ReplicateNodeOnSharedSegments(NewNode, Node2, NodeStash2, Poly2, Poly1));
					ContactFound = true;
					} // if
				else
					Success = false;
				} // else if
			else if (Poly1->TestNodeOnOrNearSegment(Node2, Node1, Node1->NextNode, NodeLat, NodeLon, Tolerance))
				{
				NodeStash = Node1->NextNode;
				// create node on Poly 2 at NodeLat, NodeLon and link with Node 1
				if (NewNode = Poly1->InsertNode(Part1, Node1, NodeLat, NodeLon, Node2->Elev))
					{
					NewNode->InheritBoundaryFlagsFromParents(Node1, NodeStash);
					NewNode->InheritBoundaryFlagsFromParent(Node2);
					Success = (NewNode->AddCrossLinks(Node2, Poly1, Poly2) &&
						ReplicateNodeOnSharedSegments(NewNode, Node1, NodeStash, Poly1, Poly2));
					ContactFound = true;
					} // if
				else
					Success = false;
				} // else if
			else if (Node1->SimilarPointLatLon(Node2, Tolerance))
				{
				// link Node 1 with Node 2
				Success = Node1->AddCrossLinks(Node2, Poly1, Poly2);
				ContactFound = true;
				} // else if
			} // else if
		} // if latitude range overlap
	} // if longitude range overlap

return (Success);

} // VectorIntersecter::IntersectSegmentsLinkNodes

/*===========================================================================*/

VectorNode *VectorIntersecter::FindIntersectionMakeNode(VectorNode *Node1, VectorNode *Node2)
{
double NodeLat, NodeLon;
VectorNode *NewNode = NULL, *Min1XNode, *Max1XNode, *Min2XNode, *Max2XNode, 
	*Min1YNode, *Max1YNode, *Min2YNode, *Max2YNode;

Max1XNode = Node1->Lon > Node1->NextNode->Lon ? Node1: Node1->NextNode;
Max2XNode = Node2->Lon > Node2->NextNode->Lon ? Node2: Node2->NextNode;
Min1XNode = Max1XNode == Node1 ? Node1->NextNode: Node1;
Min2XNode = Max2XNode == Node2 ? Node2->NextNode: Node2;
Max1YNode = Node1->Lat > Node1->NextNode->Lat ? Node1: Node1->NextNode;
Max2YNode = Node2->Lat > Node2->NextNode->Lat ? Node2: Node2->NextNode;
Min1YNode = Max1YNode == Node1 ? Node1->NextNode: Node1;
Min2YNode = Max2YNode == Node2 ? Node2->NextNode: Node2;

if (FindIntersectionContained(Node1, Node2, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
	Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon))
	{
	if (NewNode = new VectorNode(Node1))
		{
		NewNode->Lat = NodeLat;
		NewNode->Lon = NodeLon;
		} // if
	} // if

return (NewNode);

} // VectorIntersecter::FindIntersectionMakeNode

/*===========================================================================*/

bool VectorIntersecter::FindIntersectionContained(VectorNode *Node1, VectorNode *Node2,
	VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
	VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
	double &NodeLat, double &NodeLon)
{
// return true if intersection falls between end nodes on both segments
double m1, m2, X1;
bool Seg1Vertical, Seg2Vertical;

Seg1Vertical = Node1->Lon == Node1->NextNode->Lon;
Seg2Vertical = Node2->Lon == Node2->NextNode->Lon;

if (! Seg1Vertical && ! Seg2Vertical)
	{
	// neither segment vertical
	// solve for intersection on X axis and see if it lies on both segments
	m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
	m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
	if (m1 != m2)
		{
		// solve for X value of intersection
		X1 = (m2 * (Node1->Lon - Node2->Lon) + Node2->Lat - Node1->Lat) / (m1 - m2);
		NodeLon = Node1->Lon + X1;

		// find location on both segments
		if (NodeLon >= Min1XNode->Lon && NodeLon <= Max1XNode->Lon && NodeLon >= Min2XNode->Lon && NodeLon <= Max2XNode->Lon)
			{
			// need Y position of hypothetical intersection to test for
			if (Node1->Lat == Node1->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node1->Lat;
				if (NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lon != NodeLon) &&
						(Node1->NextNode->Lon != NodeLon) &&
						(Node2->Lat != NodeLat || Node2->Lon != NodeLon) &&
						(Node2->NextNode->Lat != NodeLat || Node2->NextNode->Lon != NodeLon))
						return (true);
					} // if
				} // if
			else if (Node2->Lat == Node2->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node2->Lat;
				if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lat != NodeLat || Node1->Lon != NodeLon) &&
						(Node1->NextNode->Lat != NodeLat || Node1->NextNode->Lon != NodeLon) &&
						(Node2->Lon != NodeLon) &&
						(Node2->NextNode->Lon != NodeLon))
						return (true);
					} // if
				} // if
			else
				{
				// neither segment horizontal
				NodeLat = m1 * X1 + Node1->Lat;
				if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lat != NodeLat || Node1->Lon != NodeLon) &&
						(Node1->NextNode->Lat != NodeLat || Node1->NextNode->Lon != NodeLon) &&
						(Node2->Lat != NodeLat || Node2->Lon != NodeLon) &&
						(Node2->NextNode->Lat != NodeLat || Node2->NextNode->Lon != NodeLon))
						return (true);
					} // if
				} // else
			} // if
		} // if
	} // if
else if (! Seg2Vertical)
	{
	// only segment 1 is vertical
	NodeLon = Node1->Lon;
	if (NodeLon > Min2XNode->Lon &&  NodeLon < Max2XNode->Lon)
		{
		if (Node2->Lat == Node2->NextNode->Lat)
			// segment 2 is horizontal
			NodeLat = Node2->Lat;
		else
			{
			// segment 2 is not horizontal
			m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
			NodeLat = m2 * (NodeLon - Node2->Lon) + Node2->Lat;
			} // else
		if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if
else if (! Seg1Vertical)
	{
	// only segment 2 is vertical
	NodeLon = Node2->Lon;
	if (NodeLon > Min1XNode->Lon &&  NodeLon < Max1XNode->Lon)
		{
		if (Node1->Lat == Node1->NextNode->Lat)
			// segment 1 is horizontal
			NodeLat = Node1->Lat;
		else
			{
			// segment 1 is not horizontal
			m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
			NodeLat = m1 * (NodeLon - Node1->Lon) + Node1->Lat;
			} // else
		if (NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if

return (false);

} // VectorIntersecter::FindIntersectionContained

/*===========================================================================*/
/* Obsolete 6/20/08
bool VectorIntersecter::FindIntersectionContained(VectorNode *Node1, VectorNode *Node2,
	VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
	VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
	double &NodeLat, double &NodeLon)
{
// return true if intersection falls between end nodes on both segments
double m1, m2, X1;
bool Seg1Vertical, Seg2Vertical, SegNearVertical, SegNearHorizontal;

Seg1Vertical = Node1->Lon == Node1->NextNode->Lon;
Seg2Vertical = Node2->Lon == Node2->NextNode->Lon;

if (! Seg1Vertical && ! Seg2Vertical)
	{
	// neither segment vertical
	// solve for intersection on X axis and see if it lies on both segments
	m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
	m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
	if (m1 != m2)
		{
		// solve for X value of intersection
		X1 = (m2 * (Node1->Lon - Node2->Lon) + Node2->Lat - Node1->Lat) / (m1 - m2);
		NodeLon = Node1->Lon + X1;
		SegNearVertical = SegNearHorizontal = false;

		// find location on both segments
		// unless one of the segments is nearly vertical or horizontal which might suffer from loss of precision,
		// the intersection must lie inside the intersection of the two bounding boxes.
		//  SegNearVertical and SegNearHorizontal identify the cases where there might be an allowable
		// exception and that exception is only valid if the resulting intersection does not equal one of
		// the input nodes.
		if ((NodeLon > Min1XNode->Lon && NodeLon < Max1XNode->Lon && NodeLon > Min2XNode->Lon && NodeLon < Max2XNode->Lon) ||
			((SegNearVertical = (fabs(Node1->Lon - Node1->NextNode->Lon) <= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE || fabs(Node2->Lon - Node2->NextNode->Lon) <= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE)) && (NodeLon >= Min1XNode->Lon && NodeLon <= Max1XNode->Lon && NodeLon >= Min2XNode->Lon && NodeLon <= Max2XNode->Lon)))
			{
			// need Y position of hypothetical intersection to test for
			if (Node1->Lat == Node1->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node1->Lat;
				if (NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat)
					return (true);
				} // if
			else if (Node2->Lat == Node2->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node2->Lat;
				if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
					return (true);
				} // if
			else
				{
				// neither segment horizontal
				NodeLat = m1 * X1 + Node1->Lat;
				if ((NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat && NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat) ||
					((SegNearHorizontal = (fabs(Node1->Lat - Node1->NextNode->Lat) <= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE || fabs(Node2->Lat - Node2->NextNode->Lat) <= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE)) && (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)))
					{
					if (SegNearVertical || SegNearHorizontal)
						{
						if (! ((NodeLat == Node1->Lat && NodeLon == Node1->Lon) ||
							(NodeLat == Node1->NextNode->Lat && NodeLon == Node1->NextNode->Lon) ||
							(NodeLat == Node2->Lat && NodeLon == Node2->Lon) ||
							(NodeLat == Node2->NextNode->Lat && NodeLon == Node2->NextNode->Lon)))
							return (true);
						} // if
					else
						return (true);
					} // if
				} // else
			} // if
		} // if
	} // if
else if (! Seg2Vertical)
	{
	// only segment 1 is vertical
	NodeLon = Node1->Lon;
	if (Node2->Lat == Node2->NextNode->Lat)
		{
		// segment 2 is horizontal
		NodeLat = Node2->Lat;
		if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
			{
			return (true);
			} // if
		} // if
	else
		{
		// segment 2 is not horizontal
		m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
		NodeLat = m2 * (NodeLon - Node2->Lon) + Node2->Lat;
		if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
			{
			return (true);
			} // if
		} // else
	} // else if
else if (! Seg1Vertical)
	{
	// only segment 2 is vertical
	NodeLon = Node2->Lon;
	if (Node1->Lat == Node1->NextNode->Lat)
		{
		// segment 1 is horizontal
		NodeLat = Node1->Lat;
		if (NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	else
		{
		// segment 1 is not horizontal
		m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
		NodeLat = m1 * (NodeLon - Node1->Lon) + Node1->Lat;
		if (NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if

return (false);

} // VectorIntersecter::FindIntersectionContained
*/
/*===========================================================================*/

bool VectorIntersecter::FindIntersectionContained(VectorNode *Node1, VectorNode *Node1Next, 
	VectorNode *Node2, VectorNode *Node2Next,
	VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
	VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
	double &NodeLat, double &NodeLon)
{
// return true if intersection falls between end nodes on both segments
double m1, m2, X1;
bool Seg1Vertical, Seg2Vertical;

Seg1Vertical = Node1->Lon == Node1Next->Lon;
Seg2Vertical = Node2->Lon == Node2Next->Lon;

if (! Seg1Vertical && ! Seg2Vertical)
	{
	// neither segment vertical
	// solve for intersection on X axis and see if it lies on both segments
	m1 = (Node1Next->Lat - Node1->Lat) / (Node1Next->Lon - Node1->Lon);
	m2 = (Node2Next->Lat - Node2->Lat) / (Node2Next->Lon - Node2->Lon);
	if (m1 != m2)
		{
		// solve for X value of intersection
		X1 = (m2 * (Node1->Lon - Node2->Lon) + Node2->Lat - Node1->Lat) / (m1 - m2);
		NodeLon = Node1->Lon + X1;

		// find location on both segments
		if (NodeLon >= Min1XNode->Lon && NodeLon <= Max1XNode->Lon && NodeLon >= Min2XNode->Lon && NodeLon <= Max2XNode->Lon)
			{
			// need Y position of hypothetical intersection to test for
			if (Node1->Lat == Node1Next->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node1->Lat;
				if (NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lon != NodeLon) &&
						(Node1Next->Lon != NodeLon) &&
						(Node2->Lat != NodeLat || Node2->Lon != NodeLon) &&
						(Node2Next->Lat != NodeLat || Node2Next->Lon != NodeLon))
						return (true);
					} // if
				} // if
			else if (Node2->Lat == Node2Next->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node2->Lat;
				if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lat != NodeLat || Node1->Lon != NodeLon) &&
						(Node1Next->Lat != NodeLat || Node1Next->Lon != NodeLon) &&
						(Node2->Lon != NodeLon) &&
						(Node2Next->Lon != NodeLon))
						return (true);
					} // if
				} // if
			else
				{
				// neither segment horizontal
				NodeLat = m1 * X1 + Node1->Lat;
				if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					{
					// now we need to make sure it isn't equal to one of the nodes
					if ((Node1->Lat != NodeLat || Node1->Lon != NodeLon) &&
						(Node1Next->Lat != NodeLat || Node1Next->Lon != NodeLon) &&
						(Node2->Lat != NodeLat || Node2->Lon != NodeLon) &&
						(Node2Next->Lat != NodeLat || Node2Next->Lon != NodeLon))
						return (true);
					} // if
				} // else
			} // if
		} // if
	} // if
else if (! Seg2Vertical)
	{
	// only segment 1 is vertical
	NodeLon = Node1->Lon;
	if (NodeLon > Min2XNode->Lon &&  NodeLon < Max2XNode->Lon)
		{
		if (Node2->Lat == Node2Next->Lat)
			// segment 2 is horizontal
			NodeLat = Node2->Lat;
		else
			{
			// segment 2 is not horizontal
			m2 = (Node2Next->Lat - Node2->Lat) / (Node2Next->Lon - Node2->Lon);
			NodeLat = m2 * (NodeLon - Node2->Lon) + Node2->Lat;
			} // else
		if (NodeLat > Min1YNode->Lat && NodeLat < Max1YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if
else if (! Seg1Vertical)
	{
	// only segment 2 is vertical
	NodeLon = Node2->Lon;
	if (NodeLon > Min1XNode->Lon &&  NodeLon < Max1XNode->Lon)
		{
		if (Node1->Lat == Node1Next->Lat)
			// segment 1 is horizontal
			NodeLat = Node1->Lat;
		else
			{
			// segment 1 is not horizontal
			m1 = (Node1Next->Lat - Node1->Lat) / (Node1Next->Lon - Node1->Lon);
			NodeLat = m1 * (NodeLon - Node1->Lon) + Node1->Lat;
			} // else
		if (NodeLat > Min2YNode->Lat && NodeLat < Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if

return (false);

} // VectorIntersecter::FindIntersectionContained

/*===========================================================================*/

bool VectorIntersecter::FindIntersectionContiguous(VectorNode *Node1, VectorNode *Node2,
	VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
	VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
	double &NodeLat, double &NodeLon)
{
// return true if intersection falls between end nodes on both segments or on any of the end nodes
// but not two nodes equal
double m1, m2, X1;
bool Seg1Vertical, Seg2Vertical;

if (Node1->SamePointLatLon(Node2) || Node1->SamePointLatLon(Node2->NextNode) ||
	Node2->SamePointLatLon(Node1->NextNode) || Node2->NextNode->SamePointLatLon(Node1->NextNode))
	return (false);

Seg1Vertical = Node1->Lon == Node1->NextNode->Lon;
Seg2Vertical = Node2->Lon == Node2->NextNode->Lon;
	
if (! Seg1Vertical && ! Seg2Vertical)
	{
	// neither segment vertical
	// solve for intersection on X axis and see if it lies on both segments
	m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
	m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
	if (m1 != m2)
		{
		// solve for X value of intersection
		X1 = (m2 * (Node1->Lon - Node2->Lon) + Node2->Lat - Node1->Lat) / (m1 - m2);
		NodeLon = Node1->Lon + X1;

		// find location on both segments
		// unless one of the segments is nearly vertical or horizontal which might suffer from loss of precision,
		// the intersection must lie inside the intersection of the two bounding boxes.
		//  SegNearVertical and SegNearHorizontal identify the cases where there might be an allowable
		// exception and that exception is only valid if the resulting intersection does not equal one of
		// the input nodes.
		if (NodeLon >= Min1XNode->Lon && NodeLon <= Max1XNode->Lon && NodeLon >= Min2XNode->Lon && NodeLon <= Max2XNode->Lon)
			{
			// need Y position of hypothetical intersection to test for
			if (Node1->Lat == Node1->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node1->Lat;
				if (NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					return (true);
				} // if
			else if (Node2->Lat == Node2->NextNode->Lat)
				{
				// Seg 1 horizontal, m1 should be 0, test only on Seg 2
				NodeLat = Node2->Lat;
				if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat)
					return (true);
				} // if
			else
				{
				// neither segment horizontal
				NodeLat = m1 * X1 + Node1->Lat;
				if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
					{
					return (true);
					} // if
				} // else
			} // if
		} // if
	} // if
else if (! Seg2Vertical)
	{
	// only segment 1 is vertical
	NodeLon = Node1->Lon;
	if (Node2->Lat == Node2->NextNode->Lat)
		{
		// segment 2 is horizontal
		NodeLat = Node2->Lat;
		if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat)
			{
			return (true);
			} // if
		} // if
	else
		{
		// segment 2 is not horizontal
		m2 = (Node2->NextNode->Lat - Node2->Lat) / (Node2->NextNode->Lon - Node2->Lon);
		NodeLat = m2 * (NodeLon - Node2->Lon) + Node2->Lat;
		if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
			{
			return (true);
			} // if
		} // else
	} // else if
else if (! Seg1Vertical)
	{
	// only segment 2 is vertical
	NodeLon = Node2->Lon;
	if (Node1->Lat == Node1->NextNode->Lat)
		{
		// segment 1 is horizontal
		NodeLat = Node1->Lat;
		if (NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	else
		{
		// segment 1 is not horizontal
		m1 = (Node1->NextNode->Lat - Node1->Lat) / (Node1->NextNode->Lon - Node1->Lon);
		NodeLat = m1 * (NodeLon - Node1->Lon) + Node1->Lat;
		if (NodeLat >= Min1YNode->Lat && NodeLat <= Max1YNode->Lat && NodeLat >= Min2YNode->Lat && NodeLat <= Max2YNode->Lat)
			{
			return (true);
			} // if
		} // if
	} // else if

return (false);

} // VectorIntersecter::FindIntersectionContiguous

/*===========================================================================*/

bool VectorIntersecter::FindIntersectionExtended(VectorNode *Node1, VectorNode *Node1Next, 
	VectorNode *Node2, VectorNode *Node2Next, double &NodeLat, double &NodeLon)
{
// return true if lines are not parallel and an intersecting point is found
double m1, m2, X1;
bool Seg1Vertical, Seg2Vertical;

Seg1Vertical = Node1->Lon == Node1Next->Lon;
Seg2Vertical = Node2->Lon == Node2Next->Lon;

if (! Seg1Vertical && ! Seg2Vertical)
	{
	// neither segment vertical
	// solve for intersection on X axis and see if it lies on both segments
	m1 = (Node1Next->Lat - Node1->Lat) / (Node1Next->Lon - Node1->Lon);
	m2 = (Node2Next->Lat - Node2->Lat) / (Node2Next->Lon - Node2->Lon);
	if (m1 != m2)
		{
		// solve for X value of intersection
		X1 = (m2 * (Node1->Lon - Node2->Lon) + Node2->Lat - Node1->Lat) / (m1 - m2);
		NodeLon = Node1->Lon + X1;

		// find location on both segments
		// need Y position of hypothetical intersection to test for
		if (Node1->Lat == Node1Next->Lat)
			// Seg 1 horizontal, m1 should be 0, test only on Seg 2
			NodeLat = Node1->Lat;
		else if (Node2->Lat == Node2Next->Lat)
			// Seg 1 horizontal, m1 should be 0, test only on Seg 2
			NodeLat = Node2->Lat;
		else
			// neither segment horizontal
			NodeLat = m1 * X1 + Node1->Lat;
		return (true);
		} // if
	} // if
else if (! Seg2Vertical)
	{
	// only segment 1 is vertical
	NodeLon = Node1->Lon;
	if (Node2->Lat == Node2Next->Lat)
		// segment 2 is horizontal
		NodeLat = Node2->Lat;
	else
		{
		// segment 2 is not horizontal
		m2 = (Node2Next->Lat - Node2->Lat) / (Node2Next->Lon - Node2->Lon);
		NodeLat = m2 * (NodeLon - Node2->Lon) + Node2->Lat;
		} // else
	return (true);
	} // else if
else if (! Seg1Vertical)
	{
	// only segment 2 is vertical
	NodeLon = Node2->Lon;
	if (Node1->Lat == Node1Next->Lat)
		// segment 1 is horizontal
		NodeLat = Node1->Lat;
	else
		{
		// segment 1 is not horizontal
		m1 = (Node1Next->Lat - Node1->Lat) / (Node1Next->Lon - Node1->Lon);
		NodeLat = m1 * (NodeLon - Node1->Lon) + Node1->Lat;
		} // else
	return (true);
	} // else if

return (false);

} // VectorIntersecter::FindIntersectionExtended

/*===========================================================================*/

bool VectorIntersecter::TestSegmentsTouch(VectorPolygon * const Poly1, VectorNode * const Node1, VectorNode * const Node2,
	VectorNode *Max1XNode, VectorNode *Min1XNode, VectorNode *Max1YNode, VectorNode *Min1YNode, 
	VectorNode *Max2XNode, VectorNode *Min2XNode, VectorNode *Max2YNode, VectorNode *Min2YNode)
{
double NodeLat, NodeLon, Tolerance;
//VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
bool ContactFound = false;

Tolerance = WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;

// NOTE: Bounding box testing needs to be done prior to calling this function now - Frank2
// test bounding box overlap
//Max1XNode = Node1->Lon > Node1->NextNode->Lon ? Node1: Node1->NextNode;
//Max2XNode = Node2->Lon > Node2->NextNode->Lon ? Node2: Node2->NextNode;
//Min1XNode = Max1XNode == Node1 ? Node1->NextNode: Node1;
//Min2XNode = Max2XNode == Node2 ? Node2->NextNode: Node2;

//if (Min2XNode->Lon <= Max1XNode->Lon + Tolerance && Max2XNode->Lon >= Min1XNode->Lon - Tolerance)
	//{
//	Max1YNode = Node1->Lat > Node1->NextNode->Lat ? Node1: Node1->NextNode;
//	Max2YNode = Node2->Lat > Node2->NextNode->Lat ? Node2: Node2->NextNode;
//	Min1YNode = Max1YNode == Node1 ? Node1->NextNode: Node1;
//	Min2YNode = Max2YNode == Node2 ? Node2->NextNode: Node2;
	//if (Min2YNode->Lat <= Max1YNode->Lat + Tolerance && Max2YNode->Lat >= Min1YNode->Lat - Tolerance)
		//{
		// are any of the vertices the same?
		if (Node1->SamePointLatLon(Node2))
			{
			ContactFound = true;
			} // if
		else if (Node2->SamePointLatLon(Node1->NextNode))
			{
			ContactFound = true;
			} // else if
		else if (Node1->SamePointLatLon(Node2->NextNode))
			{
			ContactFound = true;
			} // else if
		else if (Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
			&& FindIntersectionContained(Node1, Node2, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
			Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
			{
			// intersection falls between end nodes on both segments
			ContactFound = true;
			} // else if
		else if (Poly1->TestNodeOnOrNearSegment(Node1, Node2, Node2->NextNode, NodeLat, NodeLon, Tolerance))
			{
			ContactFound = true;
			} // else if
		else if (Poly1->TestNodeOnOrNearSegment(Node2, Node1, Node1->NextNode, NodeLat, NodeLon, Tolerance))
			{
			ContactFound = true;
			} // else if
		else if (Node1->SimilarPointLatLon(Node2, Tolerance))
			{
			ContactFound = true;
			} // else if
		//} // if latitude range overlap
	//} // if longitude range overlap

return (ContactFound);

} // VectorIntersecter::TestSegmentsTouch

/*===========================================================================*/

bool VectorIntersecter::TestSegmentsTouchAbsolute(VectorPolygon * const Poly1, VectorNode * const Seg1Node1, VectorNode * const Seg1Node2, 
	VectorNode * const Seg2Node1, VectorNode * const Seg2Node2)
{
double NodeLat, NodeLon;
VectorNode *Max1XNode, *Min1XNode, *Max1YNode, *Min1YNode, *Max2XNode, *Min2XNode, *Max2YNode, *Min2YNode;
bool ContactFound = false;

// test bounding box overlap
Max1XNode = Seg1Node1->Lon > Seg1Node2->Lon ? Seg1Node1: Seg1Node2;
Max2XNode = Seg2Node1->Lon > Seg2Node2->Lon ? Seg2Node1: Seg2Node2;
Min1XNode = Max1XNode == Seg1Node1 ? Seg1Node2: Seg1Node1;
Min2XNode = Max2XNode == Seg2Node1 ? Seg2Node2: Seg2Node1;

if (Min2XNode->Lon <= Max1XNode->Lon && Max2XNode->Lon >= Min1XNode->Lon)
	{
	Max1YNode = Seg1Node1->Lat > Seg1Node2->Lat ? Seg1Node1: Seg1Node2;
	Max2YNode = Seg2Node1->Lat > Seg2Node2->Lat ? Seg2Node1: Seg2Node2;
	Min1YNode = Max1YNode == Seg1Node1 ? Seg1Node2: Seg1Node1;
	Min2YNode = Max2YNode == Seg2Node1 ? Seg2Node2: Seg2Node1;
	if (Min2YNode->Lat <= Max1YNode->Lat && Max2YNode->Lat >= Min1YNode->Lat)
		{
		// are any of the vertices the same?
		if (Seg1Node1->SamePointLatLon(Seg2Node1))
			{
			ContactFound = true;
			} // if
		else if (Seg1Node2->SamePointLatLon(Seg2Node2))
			{
			ContactFound = true;
			} // else if
		else if (Seg1Node1->SamePointLatLon(Seg2Node2))
			{
			ContactFound = true;
			} // else if
		else if (Seg1Node2->SamePointLatLon(Seg2Node1))
			{
			ContactFound = true;
			} // else if
		else if (Min2XNode->Lon < Max1XNode->Lon && Max2XNode->Lon > Min1XNode->Lon && Min2YNode->Lat < Max1YNode->Lat && Max2YNode->Lat > Min1YNode->Lat
			&& FindIntersectionContained(Seg1Node1, Seg1Node2, Seg2Node1, Seg2Node2, Min1XNode, Max1XNode, Min2XNode, Max2XNode, 
			Min1YNode, Max1YNode, Min2YNode, Max2YNode, NodeLat, NodeLon)) 
			{
			// intersection falls between end nodes on both segments
			ContactFound = true;
			} // else if
		else if (Poly1->TestNodeOnOrNearSegment(Seg1Node1, Seg2Node1, Seg2Node2, NodeLat, NodeLon, 0.0))
			{
			ContactFound = true;
			} // else if
		else if (Poly1->TestNodeOnOrNearSegment(Seg2Node1, Seg1Node1, Seg1Node2, NodeLat, NodeLon, 0.0))
			{
			ContactFound = true;
			} // else if
		} // if latitude range overlap
	} // if longitude range overlap

return (ContactFound);

} // VectorIntersecter::TestSegmentsTouch

/*===========================================================================*/

// This function tests to see if the new node is also on other segments that may now 
// need to be updated with replicates?
bool VectorIntersecter::ReplicateNodeOnSharedSegments(VectorNode *NodeToReplicate, 
	VectorNode *SegStartNode, VectorNode *SegEndNode, VectorPolygon *NodeToReplicateOwner, VectorPolygon *AnotherPolygonToSkip)
{
VectorNodeLink *OneLinkedNode, *TwoLinkedNode;
bool Success = true;

// See if SegStartNode and SegEndNode have linked nodes that go to the same polygon and are 
// prev/next members of each other
for (OneLinkedNode = SegStartNode->LinkedNodes; OneLinkedNode && Success; OneLinkedNode = (VectorNodeLink *)OneLinkedNode->NextNodeList)
	{
	if (OneLinkedNode->LinkedPolygon != NodeToReplicateOwner && OneLinkedNode->LinkedPolygon != AnotherPolygonToSkip)
		{
		for (TwoLinkedNode = SegEndNode->LinkedNodes; TwoLinkedNode; TwoLinkedNode = (VectorNodeLink *)TwoLinkedNode->NextNodeList)
			{
			// are the nodes linked to the same polygon
			if (OneLinkedNode->LinkedPolygon == TwoLinkedNode->LinkedPolygon)
				{
				// are the linked nodes linked to each other in either direction
				if (OneLinkedNode->MyNode->NextNode == TwoLinkedNode->MyNode)
					{
					// add a node between the linked nodes and link it to all the other nodes the 
					// replicated node is linked to
					if (! (Success = OneLinkedNode->LinkedPolygon->InsertNodeAddLinks(OneLinkedNode->MyNode, NodeToReplicate, NodeToReplicateOwner)))
						break;
					} // if
				else if (OneLinkedNode->MyNode == TwoLinkedNode->MyNode->NextNode)
					{
					// add a node between the linked nodes and link it to all the other nodes the 
					// replicated node is linked to
					if (! (Success = OneLinkedNode->LinkedPolygon->InsertNodeAddLinks(TwoLinkedNode->MyNode, NodeToReplicate, NodeToReplicateOwner)))
						break;
					} // if
				} // if
			} // for
		} // if
	} // for

return (Success);

} // VectorIntersecter::ReplicateNodeOnSharedSegments

/*===========================================================================*/


