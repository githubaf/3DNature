// EffectEval.h
// Header for combining vectors and EffectsLib components
// Used in rendering and Views
// Built from scratch 2/24/06 by Gary R. Huber
// Copyright 2006 by 3D nature LLC. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_EFFECTEVAL_H
#define WCS_EFFECTEVAL_H

class EffectsLib;
class Database;
class VectorPolygonListDouble;
class RenderJoeList;
class Joe;
class VectorPolygon;
class VectorNode;
class PolygonBoundingBox;
class GeneralEffect;
class Renderer;
class Project;

class EffectEval
	{
	public:
		EffectsLib *EffectsHost;
		Database *DBHost;
		Renderer *Rend;
		Project *Proj;
		VectorPolygonListDouble *PolygonList;
		VectorNode **MergeOutputNodes;
		VectorPolygon **MergeOutputPolygons, **MergeNodeOwners;
		unsigned long MergeNumNodesAllocated, MergeNumPolysAllocated;


		EffectEval(EffectsLib *EffectsSource, Database *DBSource, Project *ProjectSource, Renderer *RendSource);
		~EffectEval();
		VectorPolygonListDouble *DestroyPolygonList(VectorPolygonListDouble *DestroyList);
		static void OutputVectorsToDatabase(VectorPolygonListDouble *CurVPList, char *BatchLayerName, PolygonBoundingBox *BoundLimits);
		static void OutputOneVectorToDatabase(VectorPolygon *CurVP, unsigned long Counter, bool PrintPointData);
		void OutputFailedListOfNodesToDatabase(VectorNode **MergeOutputNodes, unsigned long NodesUsed);
		bool InitToRender(int EffectClass, char *EffectEnabled, bool ElevationOnly, double MetersPerDegLat);
		bool UpdateGauge(unsigned long NewCurSteps);
		bool BuildPolygons(int EffectClass, char *EffectEnabled, bool ElevationOnly, double MetersPerDegLat, 
			unsigned long &NumPolygons);
		VectorPolygonListDouble *BuildPolygons(RenderJoeList *&ClassJoeList, unsigned long &NumPolygons);
		void SetOriginalVectorNodeFlags(void);
		bool GetPolygonRange(double &LowLat, double &LowLon, double &HighLat, double &HighLon);
		bool WeedOutOfBoundsPolygons(double &LowLat, double &LowLon, double &HighLat, double &HighLon, unsigned long &NumPolygons);
		void PrepareToMerge(VectorPolygonListDouble *ListToMerge);
		bool DisableSharedEdges(VectorPolygonListDouble *ListToDisable, bool PostProgress);
		bool CreateNodeLoop(VectorPolygon *StartPoly, VectorNode *StartNode, VectorNode **&OutputNodes, VectorPolygon **&NodeOwners, 
			VectorPolygon **&OutputPolygons, unsigned long &NodesUsed, unsigned long &PolygonsUsed,
			unsigned long &NumNodesAllocated, unsigned long &NumPolysAllocated, PolygonBoundingBox *TestBounds, 
			bool &PolyIsPure);
		bool CreateReverseNodeLoop(VectorPolygon *StartPoly, VectorNode *StartNode, VectorNode **&OutputNodes, 
			VectorPolygon **&NodeOwners, VectorPolygon **&OutputPolygons, unsigned long &NodesUsed, 
			unsigned long &PolygonsUsed, unsigned long &NumNodesAllocated, unsigned long &NumPolysAllocated, 
			PolygonBoundingBox *TestBounds);
		VectorPolygon *CreateNewPolygonAndList(VectorPolygonListDouble **&VPListPtr, VectorNode **InputNodes, 
			VectorPolygon **NodeOwners, unsigned long NodeListElements, VectorPolygon **InputPolygons);
		bool IntersectPolygons(unsigned long &NumPolygons);
		bool IntersectPolygonsWithTerrainCell(VectorPolygonListDouble *&MergeList);
		bool IntersectPolygonsWithMultipleTerrainCells(VectorPolygonListDouble *&TerrainPolygons, VectorPolygonListDouble *EffectPolygons);
		bool MergePolygons(VectorPolygonListDouble *&ListToMerge, double AreaLimit, GeneralEffect *RequiredEffect, 
			PolygonBoundingBox *TestBounds, bool SelfIntersect, bool PostProgress);
		RenderJoeList *SortByDiminishingArea(RenderJoeList *ListToSort);
		VectorPolygonListDouble *MatchListJoe(VectorPolygonListDouble *MatchList, Joe *MatchVector);
		bool RemoveHoles(VectorPolygon *HoleyCow, RenderJoeList *RJLCur);
		bool NormalizeDirection(VectorPolygonListDouble *NormalizeMe);
		VectorPolygonListDouble *CreateBoundedPolygonList(VectorPolygonListDouble *ListToTest, 
			PolygonBoundingBox *PolyBounds, bool &Success);
		VectorPolygonListDouble *MakeBoundingVectorPolygons(GeneralEffect *NewEffect, Joe *NewVector,
			VectorPolygon *PolyLiner, double MetersPerDegLat, bool &Success);
		VectorPolygonListDouble *MakeBoundingVectorPointPolygons(GeneralEffect *NewEffect, Joe *NewVector,
			double MetersPerDegLat, bool &Success);
		// only used for debugging
		void ComparePolygonsInLists(VectorPolygonListDouble *List1, VectorPolygonListDouble *List2);

	}; // class EffectEval

#endif // WCS_EFFECTEVAL_H
