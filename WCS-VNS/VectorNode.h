// VectorNode.h
// Header file for VectorNode class
// Created by Gary R. Huber 1/24/06
// Copyright 2006 3D Nature LLC. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_VECTORNODE_H
#define WCS_VECTORNODE_H

//#define VNA_FAST_DELETE	-> needs more debugging before using

class TriangleBoundingBox;
class VNAccelBin;
class VNAccelerator;
class VectorNode;
class VectorPoint;
class VectorPolygon;
class CoordSys;
class VertexDEM;
class MaterialEffect;
class GeneralEffect;
class EnvironmentEffect;
class Joe;
class VectorPolygonList;
class PolygonEdgeList;

#include "QuantaAllocator.h"

#define VNACCELMINNODES 500
#define VNBINSPERSIDE 12
#define VNNUMACCELBINS (VNBINSPERSIDE * VNBINSPERSIDE)

class MatListWaterProperties
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(MatListWaterProperties), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		char BeachOwner;
		float WaveHeight, WaterDepth, WaterNormal[3];	
		MatListWaterProperties();

	}; // class MatListWaterProperties
	
class MatListVectorProperties
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(MatListVectorProperties), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		char VectorType;
		float VectorSlope, VecOffsets[3];	
		MatListVectorProperties();

	}; // class MatListVectorProperties

class MaterialList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(MaterialList), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		float MatCoverage;	
		MatListWaterProperties *WaterProp;
		MatListVectorProperties *VectorProp;
		Joe *MyVec;
		MaterialEffect *MyMat;
		GeneralEffect *MyEffect;
		MaterialList *NextMaterial;
		
		MaterialList();
		MaterialList(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage);
		~MaterialList();
		MatListWaterProperties *AddWaterProperties(void);
		MatListVectorProperties *AddVectorProperties(void);
		void RemoveWaterProperties(void);
		void RemoveVectorProperties(void);
		
	}; // class MaterialList

class MaterialTable
	{
	public:
		double AuxValue[3];
		MaterialEffect *Material;
		MaterialList *MatListPtr[3];
		
		MaterialTable()	{Material = NULL; MatListPtr[0] = MatListPtr[1] = MatListPtr[2] = NULL;
			AuxValue[0] = AuxValue[1] = AuxValue[2] = 1.0;};
	}; // class MaterialTable
		
class VectorNodeRenderData
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorNodeRenderData), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		MaterialList *Materials;
		//unsigned long LatSeed, LonSeed;
		float RelEl, Normal[3], Slope, Aspect, TexDisplacement, TfxDisplacement, BeachLevel;
		EnvironmentEffect *NodeEnvironment;
		bool MaterialListOwned;

		VectorNodeRenderData();
		~VectorNodeRenderData();
		MaterialList *AddMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage);
		MaterialList *AddWaterMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage);
		MaterialList *AddVectorMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage);
		MaterialList *AddWaterVectorMaterial(MaterialEffect *NewMat, GeneralEffect *NewEffect, Joe *NewJoe, float NewCoverage);
		MaterialList *TestAddMaterial(MaterialEffect *TestMat, GeneralEffect *TestEffect, Joe *TestJoe, float TestCoverage);
		MaterialList *FindFirstEffect(long EffectType);
		MaterialList *FindNextEffect(MaterialList *LastMatList, long EffectType);
		MaterialList *FindFirstMaterial(long MaterialType);
		MaterialList *FindNextMaterial(MaterialList *LastMatList, long MaterialType);
		void CopyElevationData(VectorNodeRenderData *CopyFrom);
		void CopyNormalData(VectorNodeRenderData *CopyFrom);
		void CopyNonElevationNormalData(VectorNodeRenderData *CopyFrom);
	}; // class VectorNodeRenderData

class VectorNodeList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorNodeList), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		VectorNode *MyNode;
		VectorNodeList *NextNodeList;

		VectorNodeList()	{MyNode = 0L; NextNodeList = 0L;};
		VectorNodeList(VectorNode *NewNode)	{MyNode = NewNode; NextNodeList = 0L;};
	}; // class VectorNodeList

#define WCS_VECTORNODELINK_FLAG_TEMPORARY	(1 << 0)
#define WCS_VECTORNODELINK_FLAG_MERGEIMMINENT	(1 << 1)
#define WCS_VECTORNODELINK_FLAG_USETOMERGE	(1 << 2)

class VectorNodeLink: public VectorNodeList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPoolD1; // we don't clear MyPoolD1 in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPoolD1 = QA->CreatePool(sizeof(VectorNodeLink), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPoolD1 != NULL);};
		static void CleanupAllocation(void) {MyPoolD1 = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPoolD1->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPoolD1->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPoolD1->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPoolD1->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		VectorPolygon *LinkedPolygon;
		unsigned char Flags;

		VectorNodeLink();
		VectorNodeLink(VectorNode *NewNode, VectorPolygon *NewPolygon, unsigned char NewFlags);
		~VectorNodeLink();
		void FlagSet(unsigned char NewFlags)	{Flags |= NewFlags;};
		void FlagClear(unsigned char ClearFlags)	{Flags &= ~ClearFlags;};
		unsigned char FlagCheck(unsigned char CheckFlags)	{return (Flags & CheckFlags);};
	}; // class VectorNodeLink

// vector node flag values
//#define WCS_VECTORNODE_FLAG_HOLEREMOVALSHARE	(1 << 0)
//#define WCS_VECTORNODE_FLAG_SHAREDVERTEX		(1 << 0)
#define WCS_VECTORNODE_FLAG_VECTORNODE		(1 << 0)
#define WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT		(1 << 1)
#define WCS_VECTORNODE_FLAG_INTERSECT_SHAREDEDGE		(1 << 2)
#define WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED		(1 << 3)
#define WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED		(1 << 4)
#define WCS_VECTORNODE_FLAG_INTERSECTFLAGS		(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT |\
	WCS_VECTORNODE_FLAG_INTERSECT_SHAREDEDGE | WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED |\
	WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED)
#define WCS_VECTORNODE_FLAG_DIRECTIONFLAGS		(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDEDGE |\
	WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED | WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED)
#define WCS_VECTORNODE_FLAG_EASTEDGE		(1 << 5)
#define WCS_VECTORNODE_FLAG_WESTEDGE		(1 << 6)
#define WCS_VECTORNODE_FLAG_NORTHEDGE		(1 << 7)
#define WCS_VECTORNODE_FLAG_SOUTHEDGE		(1 << 8)
#define WCS_VECTORNODE_FLAG_NWCORNER		(WCS_VECTORNODE_FLAG_NORTHEDGE | WCS_VECTORNODE_FLAG_WESTEDGE)
#define WCS_VECTORNODE_FLAG_SWCORNER		(WCS_VECTORNODE_FLAG_SOUTHEDGE | WCS_VECTORNODE_FLAG_WESTEDGE)
#define WCS_VECTORNODE_FLAG_NECORNER		(WCS_VECTORNODE_FLAG_NORTHEDGE | WCS_VECTORNODE_FLAG_EASTEDGE)
#define WCS_VECTORNODE_FLAG_SECORNER		(WCS_VECTORNODE_FLAG_SOUTHEDGE | WCS_VECTORNODE_FLAG_EASTEDGE)
#define WCS_VECTORNODE_FLAG_BOUNDARYNODE	(WCS_VECTORNODE_FLAG_NWCORNER | WCS_VECTORNODE_FLAG_SECORNER)
#define WCS_VECTORNODE_FLAG_ADJOININGLINKSMADE		(1 << 9)
#define WCS_VECTORNODE_FLAG_ELEVEVALUATED			(1 << 10)
#define WCS_VECTORNODE_FLAG_NORMALSEVALUATED		(1 << 11)
#define WCS_VECTORNODE_FLAG_EFFECTSEVALUATED		(1 << 12)
#define WCS_VECTORNODE_FLAG_BEACHEVALUATED			(1 << 13)
#define WCS_VECTORNODE_FLAG_WATEREVALUATED			(1 << 14)
#define WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED	(1 << 15)
#define WCS_VECTORNODE_FLAG_TFXECOEVALUATED			(1 << 16)
#define WCS_VECTORNODE_FLAG_NONELEVATIONITEMSREQUIRED		(WCS_VECTORNODE_FLAG_EFFECTSEVALUATED |\
	WCS_VECTORNODE_FLAG_WATEREVALUATED |\
	WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED |\
	WCS_VECTORNODE_FLAG_TFXECOEVALUATED)
#define WCS_VECTORNODE_FLAG_NONELEVATIONITEMSALL		(WCS_VECTORNODE_FLAG_EFFECTSEVALUATED |\
	WCS_VECTORNODE_FLAG_WATEREVALUATED |\
	WCS_VECTORNODE_FLAG_BEACHEVALUATED |\
	WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED |\
	WCS_VECTORNODE_FLAG_TFXECOEVALUATED)
#define WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE	(1 << 17)

class VectorNode
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(VectorNode), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(double)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		double Lat, Lon, Elev;
		VectorNodeLink *LinkedNodes;
		VectorNodeRenderData *NodeData;
		VectorNode *NextNode, *PrevNode;
		VectorPolygonList *RightSideList;
		PolygonEdgeList *ForwardList;
		
		#ifdef VNA_FAST_DELETE
		unsigned long VNAbinNum;	// for use by VNAccelerator only
		unsigned long VNAslotNum;	// for use by VNAccelerator only
		#endif // VNA_FAST_DELETE_
		unsigned long Flags;

		VectorNode();
		VectorNode(VectorPoint *SetPoint);
		VectorNode(VectorNode *Template);
		~VectorNode();
		bool AddRightSideMember(VectorPolygon *NewSideMember, bool EdgeIsShared);
		void RemoveRightSideList(void);
		void AddForwardEdge(PolygonEdgeList *NewEdge);
		void RemoveEdgeList(PolygonEdgeList *RemoveEdge);
		void RemoveLinkedNodes(void);
		void RemoveLink(VectorNode *TheOtherNode);
		void DeleteLinksNoCrossCheck(void);
		void FlagSet(unsigned long NewFlags)	{Flags |= NewFlags;};
		void FlagClear(unsigned long ClearFlags)	{Flags &= ~ClearFlags;};
		unsigned long FlagCheck(unsigned long CheckFlags)	{return (Flags & CheckFlags);};
		bool SamePoint(VectorNode *CompareMe);
		bool SamePointLatLon(VectorNode *CompareMe);
		bool SamePointLatLonOrLinked(VectorNode *CompareMe);
		bool SimilarPointLatLon(VectorNode *CompareMe, double Tolerance);
		bool SimilarPointLatLon(double CompareLat, double CompareLon, double Tolerance);
		bool ProjToDefDeg(CoordSys *SourceCoords, VertexDEM *Vert);
		bool AddCrossLinks(VectorNode *LinkMe, VectorPolygon *SourcePolyThis, VectorPolygon *SourcePolyThat);
		bool AddLink(VectorNode *LinkMe, VectorPolygon *SourcePoly, unsigned char NewFlags = 0);
		//bool AddTempLink(VectorNode *LinkMe, VectorPolygon *SourcePoly)	{return (AddLink(LinkMe, SourcePoly, WCS_VECTORNODELINK_FLAG_TEMPORARY));};
		VectorNode *FindLink(VectorPolygon *FindMe);
		void RemoveTempLinks(void);
		VectorNodeRenderData *AddNodeData(void);
		VectorNodeRenderData *CopyElevationData(VectorNode *CopyFrom);
		VectorNodeRenderData *CopyNormalData(VectorNode *CopyFrom);
		VectorNodeRenderData *CopyNonElevationNormalData(VectorNode *CopyFrom);
		void InheritBoundaryFlagsFromParents(VectorNode *Parent1, VectorNode *Parent2);
		void InheritBoundaryFlagsFromParent(VectorNode *Parent1);
		VectorNodeLink **MakeLinkedNodeList(VectorNodeLink *NodeList, VectorNodeLink **NewListPtr, VectorPolygon *NodeOwner);
	}; // class VectorNode

class VNAccelerator
	{
	public:
		VNAccelBin** accelBins;

		VNAccelerator();
		~VNAccelerator();
		void BinAllPoints(VectorNode* firstVN);
		void CreateBins(unsigned long numNodes);
		void ComputeBinBounds(VectorNode* firstVN);
		void ClearBins(void);
		void DeleteNode(VectorNode* deleteMe);
		void FreeAll(void);
		void Initialize(void);
		int TestPointsInTriangle(TriangleBoundingBox* triBounds, VectorNode* firstNode, VectorNode* lastNode);
	}; // class VNAccelerator

class VNAccelBin
	{
	public:
		double latMin, latMax, lonMin, lonMax; // bin bounds
		unsigned long binCapacity;
		unsigned long numNodes;
		VectorNode** binNodes;

		VNAccelBin(unsigned long binSize);
		~VNAccelBin();
		void AddNode(VectorNode *newNode);
		void VNAccelMinBinSize(unsigned long binSize);
	}; // class VNAccelBin

#endif // WCS_VECTORNODE_H
