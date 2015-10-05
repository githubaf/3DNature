// VectorPolygon.h
// Header file for VectorPolygon class
// Created by Gary R. Huber 1/24/06
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_VECTORPOLYGON_H
#define WCS_VECTORPOLYGON_H

class VectorNode;
class VectorPolygon;
class Joe;
class EffectJoeList;
class VectorPoint;
class GeneralEffect;
class VectorPolygonList;
class VectorPolygonListDouble;
class VectorNodeList;
class PolygonBoundingBox;
class TfxDetail;
class EcosystemEffect;
class CoordSys;
class EffectEval;
class Database;

#include "QuantaAllocator.h"

enum ENUM_TEST_POINT_CONTAINED
	{
	WCS_TEST_POINT_CONTAINED_INSIDE,
	WCS_TEST_POINT_CONTAINED_EDGE,
	WCS_TEST_POINT_CONTAINED_OUTSIDE
	}; // enum ENUM_TEST_POINT_CONTAINED

enum ENUM_BOUNDING_BOX_CONTAINED
	{
	WCS_BOUNDING_BOX_CONTAINED_INSIDE,
	WCS_BOUNDING_BOX_CONTAINED_EDGE,
	WCS_BOUNDING_BOX_CONTAINED_OUTSIDE,
	WCS_BOUNDING_BOX_CONTAINED_ENCLOSED
	}; // enum ENUM_BOUNDING_BOX_CONTAINED

class VectorPart
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorPart), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		VectorNode *MyNodes;
		VectorPart *NextPart;
		unsigned long NumNodes;
		char Tested, Irrelevant;

		VectorPart()	{MyNodes = NULL; NextPart = NULL; NumNodes = 0; Tested = Irrelevant = false;};
		~VectorPart();
		void RemoveNodes(void);
		VectorNode *FirstNode(void);
		VectorNode *SecondNode(void);
		VectorNode *ThirdNode(void);
		unsigned long CountNodes(void);
		VectorNode *InsertNode(VectorNode *InsertAfter, VectorNode *CloneNode);
		VectorNode *InsertNode(VectorNode *InsertAfter, double NewLat, double NewLon, double NewElev);
		double PartArea(void);
		bool ReverseDirection(void);
		bool CrossLink(VectorPart *LinkMe, VectorPolygon *ThisPolygon, VectorPolygon *TheOtherPolygon);
	}; // class VectorPart
	
class VectorPolygon
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorPolygon), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		EffectJoeList *MyEffects;
		PolygonBoundingBox *BBox;
		VectorPolygon *CloneOfThis;
		unsigned long TotalNumNodes, PolyNumber;
		float Normals[4];
		char ImAnOriginalPolygon;
		VectorPart FirstPart;

		VectorPolygon(Database *DBHost, GeneralEffect *NewEffect, Joe *NewVector, bool NumPtsIgnore, bool &InsufficientNodes);
		VectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template);
		VectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template, bool AddLinkBack);
		VectorPolygon(VectorNode **Pts, unsigned long NumPts, VectorPolygon *Template);
		VectorPolygon(VectorPolygon * const CloneMe);
		VectorPolygon(VectorPolygon * const CloneMe, VectorNode *NewNodes, bool AddLinkBack);
		VectorPolygon(VectorPart * const ClonePart, bool AddLinkBack, VectorPolygon *PartOwner);
		~VectorPolygon();

		void RemoveNodes(void);
		VectorPart *GetFirstPart(void)	{return (&FirstPart);};
		VectorNode *CopyNodes(VectorPolygon *SourceToCopy, bool CopyLinks);
		VectorNode *CopyNodes(VectorPart *PartToCopy, bool CopyLinks, VectorPolygon *PartOwner);
		VectorPart *ClonePart(VectorPart *CloneMe);
		VectorPart *AddPart(VectorPart *AddMe);
		unsigned long CountNodes(void);
		unsigned long CountParts(void);
		VectorNode *PolyFirstNode(void);
		VectorNode *PolySecondNode(void);
		VectorNode *PolyThirdNode(void);
		VectorNode *PolyNextNode(VectorNode *CurNode);
		void PolySetFirstNode(VectorNode *SetNode)	{FirstPart.MyNodes = SetNode;};
		VectorNode *InsertNode(VectorPart *CurPart, VectorNode *InsertAfter, VectorNode *CloneNode);
		VectorNode *InsertNode(VectorPart *CurPart, VectorNode *InsertAfter, double NewLat, double NewLon, double NewElev);
		bool InsertNodeAddLinks(VectorNode *InsertAfter, VectorNode *CloneNode, VectorPolygon *CloneNodeOwner);
		Joe *GetFirstVector(void);
		void LinkBackwards(void);
		void SetPolyNumber(unsigned long NewNumber)	{PolyNumber = NewNumber;};
		bool CrossLink(VectorPolygon *LinkMe);
		EffectJoeList *SetSimpleEffect(GeneralEffect *NewEffect, Joe *NewJoe);
		EffectJoeList *SetEffect(EffectJoeList *EffectSource);
		bool TestForEffect(GeneralEffect *TestEffect);
		bool SupplementEffects(VectorPolygon *SourcePoly);
		void RemoveEffects(void);
		void RemoveEffect(GeneralEffect *RemoveMe);
		void RemoveTerrainEffectLinks(GeneralEffect *TerrainEffect);
		TfxDetail *SetTfxDetail(GeneralEffect *MatchEffect, Joe *MatchVector, unsigned long SetIndex, short SetSegment, 
			unsigned short SetFlags);
		PolygonBoundingBox *SetBoundingBox(void);
		double PolygonArea(void);
		double PolygonAreaOutsidePart(void);
		double FindNodeListArea(VectorNodeList *NodeList);
		bool ReverseDirection(void);
		void ClearIntersectionFlags(void);
		bool ConvertToDefGeo(void);
		void RemoveVestigialSegments(void);
		void RemoveNode(VectorNode *RemoveMe);
		bool ResolveSelfIntersections(bool &LinksAdded);
		bool CleanupPolygonRelationships(VectorPolygon *VP2);
		void RemoveRightSideLists(void);
		bool MergeAdjoiningNodes(bool &NodeReplicationExists);
		VectorPolygonListDouble *Triangulate(bool &Success);
		bool RemoveHoles(VectorPolygonListDouble *&InsideList);
		bool RemoveConnectingParts(void);
		double NodeToNodeDistanceSquared(VectorNode *Node1, VectorNode *Node2);
		double NodeToNodeDistanceSquaredQ(VectorNode *Node1, VectorNode *Node2);
		ENUM_TEST_POINT_CONTAINED TestPointContained(VectorNode *TestNode, double Tolerance);
		ENUM_TEST_POINT_CONTAINED TestPointContained(VectorNode **NodeLoop, unsigned long NumLoopNodes, 
			VectorNode *TestNode, double Tolerance);
		//ENUM_TEST_POINT_CONTAINED TestPointRelationToSegment(VectorNode *TestNode, VectorNode *CurNode, 
		//	VectorNode *NextNode, double Tolerance);
		ENUM_TEST_POINT_CONTAINED TestPointRelationToSegment(VectorNode *TestNode, VectorNode *CurNode, double Tolerance);
		bool TestNodeOnOrNearSegment(VectorNode *TestNode, VectorNode *CurNode, VectorNode *NextNode, 
			double &NodeLat, double &NodeLon, double Tolerance);
		void SetClonePointer(VectorPolygon *PointerToClone)	{CloneOfThis = PointerToClone;};
		VectorPolygon *GetClonePointer()	{return (CloneOfThis);};
		ENUM_BOUNDING_BOX_CONTAINED TestBoxContained(PolygonBoundingBox *IsItInsideMe);
		bool TrulyEncloses(VectorPolygon *AmIEnclosed);
		bool PolygonEnclosesPart(VectorPart *InnerPart, VectorPolygon *InnerPolygon);
		void SetLinkedPolygonClonesToNULL(void);
		void ReplaceNodeLinkPolygons(void);
		void ReplaceLinkedReferences(VectorPolygon *Replacement);
		bool CompareEffectsSame(EffectJoeList *CompareMe);
		bool GetNormals(CoordSys *PolyCoords, double PlanetRad, double *PolyNormal, double &PolyWt, 
			VectorNode *RefNode);
		bool GetWaterNormals(CoordSys *PolyCoords, double PlanetRad, double *PolyNormal, double &PolyWt,
			VectorNode *RefNode, double Elev1, double Elev2, double Elev3);
	}; // class VectorPolygon

// VectorPolygonListDouble flag values
#define WCS_VECTORPOLYGONLIST_FLAG_INSIDE		(1 << 0)
#define WCS_VECTORPOLYGONLIST_FLAG_EDGE			(1 << 1)
#define WCS_VECTORPOLYGONLIST_FLAG_ENCLOSED		(1 << 2)
#define WCS_VECTORPOLYGONLIST_FLAG_DESTROY		(1 << 3)
#define WCS_VECTORPOLYGONLIST_FLAG_RIGHTSHARE	(1 << 4)
#define WCS_VECTORPOLYGONLIST_FLAG_INSUFFICIENTNODES	(1 << 5)

class VectorPolygonList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorPolygonList), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		VectorPolygon *MyPolygon;
		VectorPolygonList *NextPolygonList;
		unsigned char Flags;

		VectorPolygonList()	{MyPolygon = 0L; NextPolygonList = 0L; Flags = 0;};
		VectorPolygon *MakeVectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template);
		VectorPolygon *MakeVectorPolygon(VectorNode *Pt1, VectorNode *Pt2, VectorNode *Pt3, VectorPolygon *Template, bool AddLinkBack);
		VectorPolygon *MakeVectorPolygon(Database *DBHost, GeneralEffect *NewEffect, Joe *NewVector);
		VectorPolygon *MakeVectorPolygon(VectorNode **Pts, unsigned long NumPts, VectorPolygon *Template);
		VectorPolygon *MakeVectorPolygon(VectorPolygon * const CloneMe, VectorNode *NewNodes, bool AddLinkBack);
		VectorPolygon *MakeVectorPolygon(VectorPart * const ClonePart, bool AddLinkBack, VectorPolygon *PartOwner);
		void DeletePolygon(void);
		Joe *GetFirstVector(void)	{return (MyPolygon ? MyPolygon->GetFirstVector(): NULL);};
		EffectJoeList *SetSimpleEffect(GeneralEffect *NewEffect, Joe *NewJoe)	{return (MyPolygon ? MyPolygon->SetSimpleEffect(NewEffect, NewJoe): NULL);};
		VectorPolygon *GetPolygon(void)	{return (MyPolygon);};
		bool ReverseDirection(void)	{return (MyPolygon ? MyPolygon->ReverseDirection(): false);};
		bool ConvertToDefGeo(void);
		void RemoveVestigialSegments(void);
		bool MergeAdjoiningNodes(bool &NodeReplicationExists);
		void RemoveRightSideLists(void)	{MyPolygon->RemoveRightSideLists();};
		void FlagSet(unsigned char NewFlags)	{Flags |= NewFlags;};
		void FlagClear(unsigned char ClearFlags)	{Flags &= ~ClearFlags;};
		unsigned char FlagCheck(unsigned char CheckFlags)	{return (Flags & CheckFlags);};

	}; // class VectorPolygonList

class VectorPolygonListDouble: public VectorPolygonList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPoolD1; // we don't clear MyPoolD1 in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPoolD1 = QA->CreatePool(sizeof(VectorPolygonListDouble), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(double)); return(MyPoolD1 != NULL);};
		static void CleanupAllocation(void) {MyPoolD1 = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPoolD1->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPoolD1->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPoolD1->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPoolD1->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		double DoubleVal;
		
		VectorPolygonListDouble()	{DoubleVal = 0.0;};
		double CalculateArea(void)	{return (DoubleVal = MyPolygon ? MyPolygon->PolygonArea(): 0.0);};
		double CalculateAreaOutsidePart(void)	{return (DoubleVal = MyPolygon ? MyPolygon->PolygonAreaOutsidePart(): 0.0);};
		bool NormalizeDirection(void);
		VectorPolygonListDouble *ResolveSelfIntersections(bool &Success, bool &LinksAdded, EffectEval *Evaluator);
		VectorPolygonListDouble *Triangulate(bool &Success);
	}; // class VectorPolygonListDouble

class TriangleBoundingBox
	{
	public:
		double MaxX, MaxY, MinX, MinY;
		VectorNode *Pt1, *Pt2, *Pt3;

		TriangleBoundingBox();
		void SetTriangle(VectorNode *Pt1Source, VectorNode *Pt2Source, VectorNode *Pt3Source);
		double TriangleArea(void);
		int TestPointInBox(VectorNode *TestPt);
		int TestPointInTriangle(VectorNode *TestPt);
		int SameSide(VectorNode *pOrigin, VectorNode *pSide, VectorNode *pTest, VectorNode *pConfirm);
		bool PartIntersectsTriangle(VectorPolygon *Poly, VectorPart *TestPart);
		bool PartIntersectsTriangleExcludeNode(VectorPolygon *Poly, VectorPart *TestPart, VectorNode *ExcludeMe);
	}; // class TriangleBoundingBox

class TriangleBoundingBoxVector
	{
	public:
		double X, Y, Z;

		TriangleBoundingBoxVector()	{X = Y = Z = 0.0;};
		TriangleBoundingBoxVector *SetXY(VectorNode *pSide, VectorNode *pOrigin);
		TriangleBoundingBoxVector *SetXY(double SideLon, double SideLat, VectorNode *pOrigin);
		TriangleBoundingBoxVector *SetXY(VectorPoint *pSide, VectorPoint *pOrigin);
		bool TestVecNonZero(void)	{return (X != 0.0 || Y != 0.0);};
		double CrossProductXY(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2);
		double DotProductXY(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2);
		double DotProductZ(TriangleBoundingBoxVector *V2);
		double LengthXYSquared(void);
		double FindRelativeAngle(TriangleBoundingBoxVector *V1, TriangleBoundingBoxVector *V2);
		void ScaleXY(double XFactor, double YFactor)	{X *= XFactor; Y*= YFactor;};
	}; // 

class PolygonBoundingBox
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(PolygonBoundingBox), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		double MaxX, MaxY, MinX, MinY;

		PolygonBoundingBox(VectorPolygon *MyPoly);
		PolygonBoundingBox()	{MaxX = MaxY = MinX = MinY = 0.0;};
		void Copy(PolygonBoundingBox *CopyFrom);
		void UpdateBounds(VectorPolygon *MyPoly);
		void UpdateBounds(VectorNode **NodeList, unsigned long NumNodes);
		bool TestPointInBox(double X, double Y);
		bool TestPointInBox(VectorNode *TestNode);
		ENUM_BOUNDING_BOX_CONTAINED TestBoxContained(PolygonBoundingBox *IsItInsideMe);
	}; // class PolygonBoundingBox

class PolygonEdgeList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(PolygonEdgeList), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

	VectorNode *TrailingNode, *LeadingNode;
	VectorPart *LeadingPart;
	PolygonEdgeList *NextEdgeList, *NextEdgeThisNode;
	bool Done;

	PolygonEdgeList(VectorNode *NewTrailer, VectorNode *NewLeader, VectorPart *NewLeadingPart)	
		{TrailingNode = NewTrailer; LeadingNode = NewLeader; NextEdgeList = NextEdgeThisNode = NULL; LeadingPart = NewLeadingPart; 
			Done = false;};
	}; // class PolygonEdgeList
	
#endif // WCS_VECTORPOLYGON_H
