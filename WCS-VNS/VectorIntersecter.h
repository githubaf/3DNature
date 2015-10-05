// VectorIntersecter.h
// Header file for VectorIntersecter class
// Created by Gary R. Huber 2/3/06
// Copyright 2006 3D Nature LLC. All rights reserved.


#ifndef WCS_VECTORINTERSECTER_H
#define WCS_VECTORINTERSECTER_H

class VectorPolygonListDouble;
class VectorPolygon;
class VectorPart;
class VectorNode;
class GeneralEffect;
class PolygonBoundingBox;

enum ENUM_INTERSECTOR_TYPE
	{
	WCS_INTERSECTOR_TYPE_NOINTERSECTION,
	WCS_INTERSECTOR_TYPE_SHAREDEDGES,
	WCS_INTERSECTOR_TYPE_SHAREDEDGE_1ST_OUTSIDE,
	WCS_INTERSECTOR_TYPE_SHAREDEDGE_2ND_OUTSIDE,
	WCS_INTERSECTOR_TYPE_CONTAINED_1ST_OUTSIDE,
	WCS_INTERSECTOR_TYPE_CONTAINED_2ND_OUTSIDE,
	WCS_INTERSECTOR_TYPE_IDENTICAL,
	WCS_INTERSECTOR_TYPE_IDENTICAL_PTMISMATCH,
	WCS_INTERSECTOR_TYPE_OVERLAPPING,
	WCS_NUM_INTERSECTOR_TYPES
	}; // intersecion types

enum ENUM_SEGMENT_INTERSECTOR_TYPE
	{
	WCS_SEGMENT_INTERSECTOR_TYPE_NOINTERSECTION,
	WCS_SEGMENT_INTERSECTOR_TYPE_SHAREDEDGE,
	WCS_SEGMENT_INTERSECTOR_TYPE_SHAREDVERTEX,
	WCS_SEGMENT_INTERSECTOR_TYPE_INTERSECTING,
	WCS_NUM_SEGMENT_INTERSECTOR_TYPES
	}; // segment intersecion types

// this is the distance used to determine of a cross-link should be made during the initial proximity test
#define WCS_VECTORPOLYGON_NODECOORD_TOLERANCE	(1.0E-11)
// this is the tolerance used to detrmine if nodes should be merged into one. It is larger than the initial tolerance
// so that in theory all the points that are flagged in the first pass will be sure to get included in the second pass
// no matter what order the testing is done. Sounds good in theory anyway.
#define WCS_VECTORPOLYGON_NODECOORD_MERGETOLERANCE	(2.0E-11)
#define WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE	(1.0E-9)
#define WCS_VECTORPOLYGON_NODECOORD_CLEANUPTOLERANCE	(1.0E-9)

class VectorIntersecter
	{
	public:
		VectorIntersecter();
		~VectorIntersecter();

		bool TestIntersect(VectorPolygon *VP1, VectorPolygon *VP2, VectorPolygonListDouble *&AddedPolygons, bool InitialMerge);

		bool TestBBoxOverlap(VectorPolygon * const VP1, PolygonBoundingBox * const BoundLimits);
		bool TestBBoxOverlap(VectorPolygon * const VP1, VectorPolygon * const VP2);
		bool TestBBoxOverlap(VectorPolygon * const VP1, VectorPolygon * const VP2, double Tolerance);

		bool TestBoundingBoxContained(VectorPolygon * const Smaller, VectorPolygon * const Larger);

		bool FindIntersectionsLinkNodes(VectorPolygon *VP1, VectorPolygon *VP2, VectorPolygonListDouble *&AddedPolygons, bool InitialMerge);

		bool IntersectSegmentsLinkNodes(VectorPolygon * const Poly1, VectorPolygon * const Poly2, 
			VectorPart * const Part1, VectorPart * const Part2,
			VectorNode * const Node1, VectorNode * const Node2, bool &ContactFound);

		VectorNode *FindIntersectionMakeNode(VectorNode *Node1, VectorNode *Node2);

		bool FindIntersectionContained(VectorNode *Node1, VectorNode *Node2,
			VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
			VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
			double &NodeLat, double &NodeLon);

		bool FindIntersectionContained(VectorNode *Node1, VectorNode *Node1Next, 
			VectorNode *Node2, VectorNode *Node2Next,
			VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
			VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
			double &NodeLat, double &NodeLon);

		bool FindIntersectionExtended(VectorNode *Node1, VectorNode *Node1Next, 
			VectorNode *Node2, VectorNode *Node2Next, 
			double &NodeLat, double &NodeLon);
			
		bool FindIntersectionContiguous(VectorNode *Node1, VectorNode *Node2,
			VectorNode *Min1XNode, VectorNode *Max1XNode, VectorNode *Min2XNode, VectorNode *Max2XNode, 
			VectorNode *Min1YNode, VectorNode *Max1YNode, VectorNode *Min2YNode, VectorNode *Max2YNode, 
			double &NodeLat, double &NodeLon);
	
		bool TestSegmentsTouch(VectorPolygon * const Poly1, VectorNode * const Node1, VectorNode * const Node2,
			VectorNode *Max1XNode, VectorNode *Min1XNode, VectorNode *Max1YNode, VectorNode *Min1YNode, 
			VectorNode *Max2XNode, VectorNode *Min2XNode, VectorNode *Max2YNode, VectorNode *Min2YNode);

		bool TestSegmentsTouchAbsolute(VectorPolygon * const Poly1, VectorNode * const Seg1Node1, VectorNode * const Seg1Node2, 
			VectorNode * const Seg2Node1, VectorNode * const Seg2Node2);

		bool ReplicateNodeOnSharedSegments(VectorNode *NodeToReplicate, 
			VectorNode *SegStartNode, VectorNode *SegEndNode, VectorPolygon *NodeToReplicateOwner, 
			VectorPolygon *AnotherPolygonToSkip);

	}; // class VectorIntersecter

#endif // WCS_VECTORINTERSECTER_H
