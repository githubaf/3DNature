// EffectEval.cpp
// Code for combining vectors and EffectsLib components
// Used in rendering and Views
// Built from scratch 2/24/06 by Gary R. Huber
// Copyright 2006 by 3D nature LLC. All rights reserved.

#include "stdafx.h"
#include "EffectEval.h"
#include "EffectsLib.h"
#include "Database.h"
#include "Project.h"
#include "Joe.h"
#include "VectorPolygon.h"
#include "VectorIntersecter.h"
#include "VectorNode.h"
#include "Lists.h"
#include "Application.h"
#include "Render.h"
#include "AppMem.h"
#include "Requester.h"
#include "Log.h"

// see also build settings defines or FeatureConfig.h
// see also defined values in RenderDEM.cpp
#ifdef WCS_VECPOLY_EFFECTS
//#define DEBUG_POLYGONS_TO_VECTOR1	// creates a list of vectors resulting from InitToRender
//#define DEBUG_POLYGONS_TO_VECTOR2	// creates a list of vectors resulting from MergePolygons
//#define DEBUG_POLYGONS_TO_VECTOR3	// creates a list of vectors input to MergePolygons and lists data about them in output window
//#define DEBUG_POLYGONS_TO_VECTOR4	// creates a list of vectors created by each merger step until a limit set in code
//#define DEBUG_POLYGONS_TO_VECTOR5	// creates a list of polygons built in first step of init
//#define WCS_DEBUGPOLY_SHOWPOINTERS	// adds polygon pointers to the output when debugging
//#define WCS_OUTPUT_TFXDETAIL
#define MAKENOCLONES
#endif // WCS_VECPOLY_EFFECTS

//extern char DebugStr[24];
//VectorNode *DebugNode;

#if defined DEBUG_POLYGONS_TO_VECTOR1 || defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3 || defined DEBUG_POLYGONS_TO_VECTOR4
extern WCSApp *GlobalApp;
extern bool PrintToVector;
#elif _DEBUG // DEBUG_POLYGONS_TO_VECTOR1 || defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3 || defined DEBUG_POLYGONS_TO_VECTOR4
extern WCSApp *GlobalApp;
#endif // DEBUG_POLYGONS_TO_VECTOR1 || defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3 || defined DEBUG_POLYGONS_TO_VECTOR4

EffectEval::EffectEval(EffectsLib *EffectsSource, Database *DBSource, Project *ProjectSource, Renderer *RendSource)
{

EffectsHost = EffectsSource;
DBHost = DBSource;
Proj = ProjectSource;
Rend = RendSource;
PolygonList = NULL;
MergeOutputNodes = NULL;
MergeOutputPolygons = NULL;
MergeNodeOwners = NULL;
MergeNumNodesAllocated = 5000;
MergeNumPolysAllocated = 50;

} // EffectEval::EffectEval

/*===========================================================================*/

EffectEval::~EffectEval()
{

DestroyPolygonList(PolygonList);
if (MergeOutputNodes)
	AppMem_Free(MergeOutputNodes, MergeNumNodesAllocated * sizeof (VectorNode *));
if (MergeNodeOwners)
	AppMem_Free(MergeNodeOwners, MergeNumNodesAllocated * sizeof (VectorPolygon *));
if (MergeOutputPolygons)
	AppMem_Free(MergeOutputPolygons, MergeNumPolysAllocated * sizeof (VectorPolygon *));

} // EffectEval::~EffectEval

/*===========================================================================*/

VectorPolygonListDouble *EffectEval::DestroyPolygonList(VectorPolygonListDouble *DestroyList)
{
bool SetToNULL = false;

if (! DestroyList)
	{
	DestroyList = PolygonList;
	SetToNULL = true;
	} // if

for (VectorPolygonListDouble *CurList = DestroyList; DestroyList; CurList = DestroyList)
	{
	DestroyList = (VectorPolygonListDouble *)DestroyList->NextPolygonList;
	CurList->DeletePolygon();
	delete CurList;
	} // for

if (SetToNULL)
	PolygonList = NULL;

return (NULL);

} // EffectEval::DestroyPolygonList

/*===========================================================================*/

void EffectEval::OutputVectorsToDatabase(VectorPolygonListDouble *CurVPList, char *BatchLayerName, PolygonBoundingBox *BoundLimits)
{
char VecName[128];
int PartCounter;
unsigned long NodeCt;
VectorNode *CurNode;
VectorNodeLink *CurLinkedNode;
VectorPart *CurPart;
TfxDetail *CurTfxDetail;
VectorIntersecter VI;

if (BatchLayerName && BatchLayerName[0])
	{
	sprintf(VecName, "%s\n", BatchLayerName);
	OutputDebugString(VecName);
	} // if
	
for (; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
	{
	if (! BoundLimits || VI.TestBBoxOverlap(CurVPList->MyPolygon, BoundLimits))
		{
		for (PartCounter = 0, CurPart = CurVPList->MyPolygon->GetFirstPart(); CurPart; ++PartCounter, CurPart = CurPart->NextPart)
			{
			Joe *NewJoe;

			//if (CurVPList->MyPolygon->PolyNumber != 750 && CurVPList->MyPolygon->PolyNumber != 702)
			//	continue;
			// output first effect name on the polygon
			#ifdef WCS_DEBUGPOLY_SHOWPOINTERS
			sprintf(VecName, "%p %p %d %d/%d %s\n", CurVPList, CurVPList->MyPolygon, CurVPList->MyPolygon->TotalNumNodes, CurVPList->MyPolygon->PolyNumber, PartCounter, CurVPList->MyPolygon->MyEffects ? CurVPList->MyPolygon->MyEffects->MyEffect->Name: "Unnamed");
			#else // WCS_DEBUGPOLY_SHOWPOINTERS
			sprintf(VecName, "%d %d/%d %s\n", CurVPList->MyPolygon->TotalNumNodes, CurVPList->MyPolygon->PolyNumber, PartCounter, CurVPList->MyPolygon->MyEffects ? CurVPList->MyPolygon->MyEffects->MyEffect->Name: "Unnamed");
			#endif // WCS_DEBUGPOLY_SHOWPOINTERS
			OutputDebugString(VecName);
			for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
				{
				// output node number, forward used flag, backward used flag, coords,
				sprintf(VecName, "   %d flags %x ff=%d bf=%d sh=%d lat=%.6f lon=%.6f\n", NodeCt, CurNode->Flags,
					CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED),
					CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED), 
					CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT), CurNode->Lat * 10E5, CurNode->Lon * 10E5);
				OutputDebugString(VecName);
				// list of right side members
				for (VectorPolygonList *CurSideMember = CurNode->RightSideList; CurSideMember; CurSideMember = CurSideMember->NextPolygonList)
					{
					#ifdef WCS_DEBUGPOLY_SHOWPOINTERS
					sprintf(VecName, "    Right Side= %d %s %p\n", CurSideMember->MyPolygon->PolyNumber, CurSideMember->MyPolygon->MyEffects ? CurSideMember->MyPolygon->MyEffects->MyEffect->Name: "Unnamed", CurSideMember->MyPolygon);
					#else // WCS_DEBUGPOLY_SHOWPOINTERS
					sprintf(VecName, "    Right Side= %d %s\n", CurSideMember->MyPolygon->PolyNumber, CurSideMember->MyPolygon->MyEffects ? CurSideMember->MyPolygon->MyEffects->MyEffect->Name: "Unnamed");
					#endif // WCS_DEBUGPOLY_SHOWPOINTERS
					OutputDebugString(VecName);
					} // for
				// list of the first effect of the linked polygons
				for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
					{
					#ifdef WCS_DEBUGPOLY_SHOWPOINTERS
					sprintf(VecName, "    Linked Poly= %d %s %p\n", CurLinkedNode->LinkedPolygon->PolyNumber, CurLinkedNode->LinkedPolygon->MyEffects ? CurLinkedNode->LinkedPolygon->MyEffects->MyEffect->Name: "Unnamed", CurLinkedNode->LinkedPolygon);
					#else // WCS_DEBUGPOLY_SHOWPOINTERS
					sprintf(VecName, "    Linked Poly= %d %s\n", CurLinkedNode->LinkedPolygon->PolyNumber, CurLinkedNode->LinkedPolygon->MyEffects ? CurLinkedNode->LinkedPolygon->MyEffects->MyEffect->Name: "Unnamed");
					#endif // WCS_DEBUGPOLY_SHOWPOINTERS
					OutputDebugString(VecName);
					} // for
				} // for

			#ifdef WCS_DEBUGPOLY_SHOWPOINTERS
			sprintf(VecName, "%p %p %d/%d %s", CurVPList, CurVPList->MyPolygon, CurVPList->MyPolygon->PolyNumber, PartCounter, CurVPList->MyPolygon->MyEffects ? CurVPList->MyPolygon->MyEffects->MyEffect->Name: "Unnamed");
			#else // WCS_DEBUGPOLY_SHOWPOINTERS
			sprintf(VecName, "%d/%d %s", CurVPList->MyPolygon->PolyNumber, PartCounter, CurVPList->MyPolygon->MyEffects ? CurVPList->MyPolygon->MyEffects->MyEffect->Name: "Unnamed");
			#endif // WCS_DEBUGPOLY_SHOWPOINTERS
			if (NewJoe = GlobalApp->AppDB->NewObject(GlobalApp->MainProj, VecName))
				{
				if (NewJoe->TransferPoints(CurPart, true))
					{
					if (BatchLayerName && BatchLayerName[0])
						{
						if (LayerEntry *LE = GlobalApp->AppDB->DBLayers.MatchMakeLayer(BatchLayerName))
							{
							NewJoe->AddObjectToLayer(LE);
							} // if
						} // if
					for (EffectJoeList *CurEffect = CurVPList->MyPolygon->MyEffects; CurEffect; CurEffect = CurEffect->Next)
						{
						if (CurTfxDetail = CurEffect->GetTfxDetail())
							{
							for (; CurTfxDetail; CurTfxDetail = CurTfxDetail->Next)
								{
								sprintf(VecName, "%s %d/%d", CurEffect->MyEffect->Name, CurTfxDetail->Index, CurTfxDetail->XSectSegment);
								if (LayerEntry *LE = GlobalApp->AppDB->DBLayers.MatchMakeLayer(VecName))
									{
									NewJoe->AddObjectToLayer(LE);
									} // if
								} // for
							} // if
						else if (LayerEntry *LE = GlobalApp->AppDB->DBLayers.MatchMakeLayer(CurEffect->MyEffect->Name))
							{
							NewJoe->AddObjectToLayer(LE);
							} // if
						} // for
					} // if
				} // if
			} // for
		} // if
	} // for

} // EffectEval::OutputVectorsToDatabase

/*===========================================================================*/

void EffectEval::OutputOneVectorToDatabase(VectorPolygon *CurVP, unsigned long Counter, bool PrintPointData)
{
Joe *NewJoe;
VectorPart *CurPart;
VectorNode *CurNode;
unsigned long NodeCt;
int PartCounter;
char ItemName[128];

for (PartCounter = 0, CurPart = CurVP->GetFirstPart(); CurPart; ++PartCounter, CurPart = CurPart->NextPart)
	{
	if (PrintPointData)
		{
		#ifdef WCS_DEBUGPOLY_SHOWPOINTERS
		sprintf(ItemName, "%p %p %d %d/%d %s\n", CurVP, CurVP, CurVP->TotalNumNodes, CurVP->PolyNumber, PartCounter, CurVP->MyEffects ? CurVP->MyEffects->MyEffect->Name: "Unnamed");
		#else // WCS_DEBUGPOLY_SHOWPOINTERS
		sprintf(ItemName, "%d %d/%d %s\n", CurVP->TotalNumNodes, CurVP->PolyNumber, PartCounter, CurVP->MyEffects ? CurVP->MyEffects->MyEffect->Name: "Unnamed");
		#endif // WCS_DEBUGPOLY_SHOWPOINTERS
		OutputDebugString(ItemName);
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			// output node number, forward used flag, backward used flag, coords,
			sprintf(ItemName, "   %d lat=%.6f lon=%.6f\n", NodeCt, CurNode->Lat * 10E5, CurNode->Lon * 10E5);
			OutputDebugString(ItemName);
			} // for
		} // if
		
	sprintf(ItemName, "%d/%d - %s", Counter, PartCounter, CurVP->MyEffects->MyEffect->Name);
	if (NewJoe = GlobalApp->AppDB->NewObject(GlobalApp->MainProj, ItemName))
		{
		if (NewJoe->TransferPoints(CurPart, true))
			{
			for (EffectJoeList *CurEffect = CurVP->MyEffects; CurEffect; CurEffect = CurEffect->Next)
				{
				if (LayerEntry *LE = GlobalApp->AppDB->DBLayers.MatchMakeLayer(CurEffect->MyEffect->Name))
					{
					NewJoe->AddObjectToLayer(LE);
					} // if
				} // for
			} // if
		} // if
	} // for
	
} // EffectEval::OutputOneVectorToDatabase

/*===========================================================================*/

void EffectEval::OutputFailedListOfNodesToDatabase(VectorNode **MergeOutputNodes, unsigned long NodesUsed)
{
Joe *NewJoe;
VectorNode *CurNode;
VectorPoint *CurPt;
unsigned long NodeCt, TotalPoints;
char ItemName[64];

strcpy(ItemName, "Merge Failure Result");
if (NewJoe = GlobalApp->AppDB->NewObject(GlobalApp->MainProj, ItemName))
	{
	TotalPoints = NodesUsed + Joe::GetFirstRealPtNum();
	if (NewJoe->Points(GlobalApp->AppDB->MasterPoint.Allocate(TotalPoints)))
		{
		NewJoe->NumPoints(TotalPoints);
		CurPt = NewJoe->Points();
		CurNode = MergeOutputNodes[0];
		if (Joe::GetFirstRealPtNum())
			{
			CurPt->Latitude = CurNode->Lat;
			CurPt->Longitude = CurNode->Lon;
			CurPt->Elevation = (float)CurNode->Elev;
			CurPt = CurPt->Next;
			--TotalPoints;
			} // if
		for (NodeCt = 0; NodeCt < TotalPoints; ++NodeCt, CurPt = CurPt->Next)
			{
			CurNode = MergeOutputNodes[NodeCt];
			CurPt->Latitude = CurNode->Lat;
			CurPt->Longitude = CurNode->Lon;
			CurPt->Elevation = (float)CurNode->Elev;
			} // for
		} // if
	} // if
	
} // EffectEval::OutputFailedListOfNodesToDatabase

/*===========================================================================*/

bool EffectEval::InitToRender(int EffectClass, char *EffectEnabled, bool ElevationOnly, double MetersPerDegLat)
{
bool Success = true;
unsigned long NumPolygons;

PolygonList = DestroyPolygonList(PolygonList);

// failure occurs if out of memory in all functions or if a topology error occurs in merging polygons
NumPolygons = DBHost->HowManyObjs();
Rend->InitRemoteGauge("Building Polygons", NumPolygons, 0);
if ((Success = BuildPolygons(EffectClass, EffectEnabled, ElevationOnly, MetersPerDegLat, NumPolygons)) && PolygonList)
	{
	#ifdef DEBUG_POLYGONS_TO_VECTOR5
	long PolyCt;
	VectorPolygonListDouble *CurList;
	for (PolyCt = 0, CurList = PolygonList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList, ++PolyCt)
		{
		CurList->GetPolygon()->SetPolyNumber(PolyCt);
		} // for
	OutputVectorsToDatabase(PolygonList, "Built polygons", NULL);
	if (Success = false)	// just to stop rendering after outputting polygons
	#endif // DEBUG_POLYGONS_TO_VECTOR5
		{
		Rend->InitRemoteGauge("Intersecting Polygons", NumPolygons * 6, 0);
		if ((Success = IntersectPolygons(NumPolygons)) && PolygonList)
			{
			Rend->InitRemoteGauge("Merging Polygons", NumPolygons, 0);
			#if defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3
			PrintToVector = true;
			#endif // DEBUG_POLYGONS_TO_VECTOR2 || DEBUG_POLYGONS_TO_VECTOR3
			Success = MergePolygons(PolygonList, 0.0, NULL, NULL, false, true);
			SetOriginalVectorNodeFlags();
			} // if
		} // if
	} // if
Rend->RemoveRemoteGauge();

#ifdef DEBUG_POLYGONS_TO_VECTOR1
OutputVectorsToDatabase(PolygonList, "Init output", NULL);
#endif // DEBUG_POLYGONS_TO_VECTOR1

// if an error, clean up here
if (! Success)
	PolygonList = DestroyPolygonList(PolygonList);

return (Success);

} // EffectEval::InitToRender

/*===========================================================================*/

bool EffectEval::UpdateGauge(unsigned long NewCurSteps)
{

return (Rend->UpdateRemoteGauge(NewCurSteps));

} // EffectEval::UpdateGauge

/*===========================================================================*/

// Build a list of VectorPolygons to represent the coverage of all the enabled database vectors and effects.
// List should be purged of duplicate polygons.
// All polygons should be clockwise in orientation and have holes removed
bool EffectEval::BuildPolygons(int EffectClass, char *EffectEnabled, bool ElevationOnly, 
	double MetersPerDegLat, unsigned long &NumPolygons)
{
bool Success = true, VectorIsClean, LinksAdded;
int StartClass, EndClass;
unsigned long PolysBuilt = 0, JoesConsidered = 0, StashNumNodes;
VectorPolygonListDouble **VPLPtr, *TriVPList, *CurVPList, *PrevVPList = NULL, *ListEnd;
RenderJoeList *ClassJoeList, *RJLCur;

if (EffectClass >= WCS_EFFECTSSUBCLASS_LAKE && EffectClass < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	StartClass = EffectClass;
	EndClass = StartClass + 1;
	} // if specific class
else
	{
	StartClass = WCS_EFFECTSSUBCLASS_LAKE;
	EndClass = WCS_MAXIMPLEMENTED_EFFECTS;
	} // else all classes

VPLPtr = &PolygonList;

for (int CurClass = StartClass; Success && CurClass < EndClass && Success; ++CurClass)
	{
	// test class implementation
	if (EffectEnabled[CurClass] && EffectsHost->GetEffectTypePrecedence(CurClass) >= 0)
		{
		// area effects
		if (EffectsHost->TestInitToRender(CurClass, ElevationOnly, true, false))
			{
			// create a RenderJoeList of all the vectors for this class
			if (ClassJoeList = DBHost->CreateRenderJoeList(EffectsHost, CurClass))
				{
				// sort by diminishing absolute value of area to facilitate hole removal
				ClassJoeList = SortByDiminishingArea(ClassJoeList);
				// for each JoeList entry
				for (RJLCur = ClassJoeList; RJLCur && Success; RJLCur = (RenderJoeList *)RJLCur->Next)
					{
					bool ConsiderVector = RJLCur->Area != 0.0;
					bool MultiPart = false;

					// see if it is an inside or outside polygon
					#if defined WCS_ISLAND_EFFECTS
					// find out if there are multiparts and if this is a positive or negative area
					if (RJLCur->GetVector()->GetMultiPartLayer())
						{
						if (RJLCur->Area <= 0.0)
							ConsiderVector = 0;
						MultiPart = true;
						} // if
					#endif // WCS_ISLAND_EFFECTS

					if (ConsiderVector)
						{
						VectorIsClean = RJLCur->GetVector()->TestFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED) ? true: false;
						// test to see if a VectorPolygon has already been created for any of the items in the list
						// <<<>>> GRH MatchListJoe could get time-consuming for large numbers of vectors
						if (PolygonList && (CurVPList = MatchListJoe(PolygonList, RJLCur->GetVector())))
							{
							if (! CurVPList->SetSimpleEffect(RJLCur->GetEffect(), RJLCur->GetVector()))
								Success = false;
							} // if VectorPolygon already exists
						else
							{
							// make a new list entry
							if (*VPLPtr = new VectorPolygonListDouble())
								{
								CurVPList = *VPLPtr;
								// make a VectorPolygon
								if (CurVPList->MakeVectorPolygon(DBHost, RJLCur->GetEffect(), RJLCur->GetVector()))
									{
									++PolysBuilt;
									// if multi part vector make a list of the inside parts and remove the holes
									if (MultiPart)	// can only be true if WCS_ISLAND_EFFECTS is defined, see above
										{
										if (Success = RemoveHoles(CurVPList->GetPolygon(), RJLCur))
											Success = CurVPList->GetPolygon()->RemoveConnectingParts();
										} // if
									} // if
								else if (CurVPList->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_INSUFFICIENTNODES))
									{
									// not enough nodes remaining in vector once redundant points removed
									delete CurVPList;
									*VPLPtr = NULL;
									goto UpdateGauge;
									} // else if
								else
									Success = false;

								if (Success && (Success = CurVPList->ConvertToDefGeo()))
									{
									CurVPList->CalculateArea();
									// this will be used to determine if the polygon is built for an area effect
									CurVPList->MyPolygon->ImAnOriginalPolygon = true;
									if (Success = NormalizeDirection(CurVPList))
										{
										// Self-intersetion testing is very slow - try to do a pre-validation step before rendering
										// Since these are user-supplied vectors we cannot assume they are without
										// topological transgressions such as crossing segments or vertices coincident
										// with a segment, IOW, self-intersections that must be removed
										if (! VectorIsClean)
											{
											StashNumNodes = CurVPList->MyPolygon->TotalNumNodes;
											LinksAdded = false;
											TriVPList = CurVPList->ResolveSelfIntersections(Success, LinksAdded, this);
											if (! CurVPList->MyPolygon)
												{										
												--PolysBuilt;
												if (PrevVPList)
													PrevVPList->NextPolygonList = TriVPList;
												else
													PolygonList = TriVPList;
												delete CurVPList;
												CurVPList = PrevVPList;
												} // if
											else if (StashNumNodes == CurVPList->MyPolygon->TotalNumNodes && ! LinksAdded)
												RJLCur->GetVector()->SetFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
											if (TriVPList)
												{
												++PolysBuilt;
												TriVPList->MyPolygon->ImAnOriginalPolygon = true;
												for (ListEnd = TriVPList; ListEnd->NextPolygonList; ListEnd = (VectorPolygonListDouble *)ListEnd->NextPolygonList)
													{
													// this will be used to determine if the polygon is built for an area effect
													ListEnd->MyPolygon->ImAnOriginalPolygon = true;
													++PolysBuilt;
													} // for
												CurVPList = ListEnd;
												} // if
											} // if
										} // if
									} // if
								PrevVPList = CurVPList;
								VPLPtr = CurVPList ? (VectorPolygonListDouble **)&CurVPList->NextPolygonList: &PolygonList;
								} // if
							else
								Success = false;
							} // else
						} // if
					UpdateGauge:
					++JoesConsidered;
					if (JoesConsidered > NumPolygons)
						JoesConsidered = 1;
					if (! Success)
						{
						char ErrStr[256];
						sprintf(ErrStr, "An error was encountered creating a polygon from the vector \"%s\" which has the %s \"%s\" applied to it and contains %d points.",
							RJLCur->GetVector()->GetBestName(), EffectsHost->GetEffectTypeNameNonPlural(RJLCur->GetEffect()->EffectType), RJLCur->GetEffect()->GetName(), RJLCur->GetVector()->GetNumRealPoints());
						UserMessageOK("Topology Error", ErrStr);
						} // if
					if (! UpdateGauge(JoesConsidered))
						Success = false;
					} // for
				// clear JoeList
				for (RJLCur = ClassJoeList; ClassJoeList; RJLCur = ClassJoeList)
					{
					ClassJoeList = (RenderJoeList *)ClassJoeList->Next;
					delete RJLCur;
					} // for
				} // if
			} // if area effects
		// linear effects
		else if (EffectsHost->TestInitToRender(CurClass, ElevationOnly, false, true))
			{
			// create a RenderJoeList of all the vectors for this class
			if (ClassJoeList = DBHost->CreateRenderJoeList(EffectsHost, CurClass))
				{
				long RJLNum = -1;
				for (RJLCur = ClassJoeList; RJLCur && Success; RJLCur = (RenderJoeList *)RJLCur->Next)
					{
					++RJLNum;
					// make a new list entry
					// if a point terraffector is involved use a different function
					if ((RJLCur->GetVector()->GetLineStyle() < 4
						|| RJLCur->GetVector()->GetNumRealPoints() == 1))
						{
						if (*VPLPtr = MakeBoundingVectorPointPolygons(RJLCur->GetEffect(), RJLCur->GetVector(),
							MetersPerDegLat, Success))
							{
							while (*VPLPtr)
								{
								VPLPtr = (VectorPolygonListDouble **)&(*VPLPtr)->NextPolygonList;
								++PolysBuilt;
								} // while
							} // if
						else
							{
							Success = false;
							} // if
						} // if
					else if (*VPLPtr = MakeBoundingVectorPolygons(RJLCur->GetEffect(), RJLCur->GetVector(),
						NULL, MetersPerDegLat, Success))
						{
						while (*VPLPtr)
							{
							VPLPtr = (VectorPolygonListDouble **)&(*VPLPtr)->NextPolygonList;
							++PolysBuilt;
							} // while
						} // if
					else
						{
						Success = false;
						} // if
					++JoesConsidered;
					if (JoesConsidered > NumPolygons)
						JoesConsidered = 1;
					if (! Success)
						{
						char ErrStr[256];
						sprintf(ErrStr, "An error was encountered creating polygons from the vector \"%s\" which has the %s \"%s\" applied to it and contains %d points.",
							RJLCur->GetVector()->GetBestName(), EffectsHost->GetEffectTypeNameNonPlural(RJLCur->GetEffect()->EffectType), RJLCur->GetEffect()->GetName(), RJLCur->GetVector()->GetNumRealPoints());
						UserMessageOK("Topology Error", ErrStr);
						} // if
					if (! UpdateGauge(JoesConsidered))
						Success = false;
					} // for
				// clear JoeList
				for (RJLCur = ClassJoeList; ClassJoeList; RJLCur = ClassJoeList)
					{
					ClassJoeList = (RenderJoeList *)ClassJoeList->Next;
					delete RJLCur;
					} // for
				} // if
			} // else if line effects
		} // if
	} // for

NumPolygons = PolysBuilt;
return (Success);

} // EffectEval::BuildPolygons

/*===========================================================================*/

// This version builds a polygon list from a RenderJoeList and returns the list built. Does not add anything to
// EffectEval's built-in list *PolygonList
VectorPolygonListDouble *EffectEval::BuildPolygons(RenderJoeList *&ClassJoeList, unsigned long &NumPolygons)
{
bool Success = true, VectorIsClean, LinksAdded;
unsigned long PolysBuilt = 0, StashNumNodes;
VectorPolygonListDouble **VPLPtr, *TriVPList, *CurVPList, *PrevVPList = NULL, *BuiltList = NULL, *ListEnd, *TempVPList;
RenderJoeList *RJLCur;

// list is already sorted in the order desired. For hole removal to work there has to be a largest to smallest sort.
VPLPtr = &BuiltList;

// for each JoeList entry
for (RJLCur = ClassJoeList; RJLCur && Success; RJLCur = (RenderJoeList *)RJLCur->Next)
	{
	bool ConsiderVector = RJLCur->Area != 0.0;
	bool MultiPart = false;

	// see if it is an inside or outside polygon
	#if defined WCS_ISLAND_EFFECTS
	// find out if there are multiparts and if this is a positive or negative area
	if (RJLCur->GetVector()->GetMultiPartLayer())
		{
		if (RJLCur->Area <= 0.0)
			ConsiderVector = 0;
		MultiPart = true;
		} // if
	#endif // WCS_ISLAND_EFFECTS

	if (ConsiderVector)
		{
		VectorIsClean = RJLCur->GetVector()->TestFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED) ? true: false;
		// make a new list entry
		if (TempVPList = new VectorPolygonListDouble())
			{
			// make a VectorPolygon
			if (TempVPList->MakeVectorPolygon(DBHost, RJLCur->GetEffect(), RJLCur->GetVector()))
				{
				++PolysBuilt;
				*VPLPtr = TempVPList;
				CurVPList = *VPLPtr;
				// if multi part vector make a list of the inside parts and remove the holes
				if (MultiPart)	// can only be true if WCS_ISLAND_EFFECTS is defined, see above
					{
					Success = RemoveHoles(CurVPList->GetPolygon(), RJLCur);
					} // if
				} // if
			else
				{
				delete TempVPList;
				TempVPList = NULL;
				} // else no polygon

			if (TempVPList && (Success = CurVPList->ConvertToDefGeo()))
				{
				CurVPList->CalculateArea();
				if (Success = NormalizeDirection(CurVPList))
					{
					// Self-intersetion testing is very slow - try to do a pre-validation step before rendering
					// Since these are user-supplied vectors we cannot assume they are without
					// topological transgressions such as crossing segments or vertices coincident
					// with a segment, IOW, self-intersections that must be removed
					if (! VectorIsClean)
						{
						StashNumNodes = CurVPList->MyPolygon->TotalNumNodes;
						LinksAdded = false;
						TriVPList = CurVPList->ResolveSelfIntersections(Success, LinksAdded, this);
						if (! CurVPList->MyPolygon)
							{										
							--PolysBuilt;
							if (PrevVPList)
								PrevVPList->NextPolygonList = TriVPList;
							else
								BuiltList = TriVPList;
							delete CurVPList;
							CurVPList = PrevVPList;
							} // if
						else if (StashNumNodes == CurVPList->MyPolygon->TotalNumNodes && ! LinksAdded)
							RJLCur->GetVector()->SetFlags(WCS_JOEFLAG_TOPOLOGYVALIDATED);
						if (TriVPList)
							{
							for (ListEnd = TriVPList; ListEnd->NextPolygonList; ListEnd = (VectorPolygonListDouble *)ListEnd->NextPolygonList)
								++PolysBuilt;
							CurVPList = ListEnd;
							} // if
						} // if
					} // if
				} // if
			PrevVPList = CurVPList;
			VPLPtr = CurVPList ? (VectorPolygonListDouble **)&CurVPList->NextPolygonList: &BuiltList;
			} // if
		else
			Success = false;
		} // if
	} // for

NumPolygons = PolysBuilt;
return (BuiltList);

} // EffectEval::BuildPolygons

/*===========================================================================*/

void EffectEval::SetOriginalVectorNodeFlags(void)
{

unsigned long NodeCt;
VectorNode *CurNode;
VectorPart *CurPart;
VectorPolygonListDouble *CurPoly;

for (CurPoly = PolygonList; CurPoly; CurPoly = (VectorPolygonListDouble *)CurPoly->NextPolygonList)
	{
	CurPoly->MyPolygon->ImAnOriginalPolygon = 1;
	// we should be able to take it for granted that MyPolygon exists
	for (CurPart = CurPoly->MyPolygon->GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
		{
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes; ++NodeCt, CurNode = CurNode->NextNode)
			{
			CurNode->FlagSet(WCS_VECTORNODE_FLAG_ORIGINALVECTORNODE | WCS_VECTORNODE_FLAG_VECTORNODE);
			} // for
		} // for
	} // for
	
} // EffectEval::SetOriginalVectorNodeFlags

/*===========================================================================*/

bool EffectEval::GetPolygonRange(double &LowLat, double &LowLon, double &HighLat, double &HighLon)
{
VectorPolygonListDouble *CurList;
bool Success = true;

LowLat = LowLon = FLT_MAX;
HighLat = HighLon = -FLT_MAX;

for (CurList = PolygonList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
	{
	if (! CurList->MyPolygon->BBox)
		CurList->MyPolygon->SetBoundingBox();
	if (! CurList->MyPolygon->BBox)
		{
		Success = false;
		break;
		} // if
	if (CurList->MyPolygon->BBox->MaxX > HighLon)
		HighLon = CurList->MyPolygon->BBox->MaxX;
	if (CurList->MyPolygon->BBox->MaxY > HighLat)
		HighLat = CurList->MyPolygon->BBox->MaxY;
	if (CurList->MyPolygon->BBox->MinX < LowLon)
		LowLon = CurList->MyPolygon->BBox->MinX;
	if (CurList->MyPolygon->BBox->MinY < LowLat)
		LowLat = CurList->MyPolygon->BBox->MinY;
	} // for

return (Success);
	
} // EffectEval::GetPolygonRange

/*===========================================================================*/

bool EffectEval::WeedOutOfBoundsPolygons(double &LowLat, double &LowLon, double &HighLat, double &HighLon, unsigned long &NumPolygons)
{
VectorPolygonListDouble *CurList, *PrevList = NULL;
bool Success = true;

for (CurList = PolygonList; CurList && NumPolygons > 0; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: PolygonList)
	{
	if (! CurList->MyPolygon->BBox)
		CurList->MyPolygon->SetBoundingBox();
	if (! CurList->MyPolygon->BBox)
		{
		Success = false;
		break;
		} // if
	if ((CurList->MyPolygon->BBox->MinX > HighLon)
		|| (CurList->MyPolygon->BBox->MinY > HighLat)
		|| (CurList->MyPolygon->BBox->MaxX < LowLon)
		|| (CurList->MyPolygon->BBox->MaxY < LowLat))
		{
		if (PrevList)
			PrevList->NextPolygonList = CurList->NextPolygonList;
		else
			PolygonList = (VectorPolygonListDouble *)CurList->NextPolygonList;
		CurList->DeletePolygon();
		delete CurList;
		--NumPolygons;
		CurList = PrevList;
		} // if
	PrevList = CurList;
	} // for

return (Success);

} // EffectEval::WeedOutOfBoundsPolygons

/*===========================================================================*/

bool EffectEval::IntersectPolygons(unsigned long &NumPolygons)
{
double LowLat, LowLon, HighLat, HighLon, NumBlocks, LatBlockSize, LonBlockSize;
#ifdef WCS_TEST_EARLY_TERRAIN_INIT
double TerrainLowLat, TerrainLowLon, TerrainHighLat, TerrainHighLon;
#endif // WCS_TEST_EARLY_TERRAIN_INIT
long NumLatBlocks, NumLonBlocks, CurLatBlock, CurLonBlock;
unsigned long PolyCt;
bool InitInBlocks, InitNecessary, TerrainBoundsValid, Success = true;
VectorPolygonListDouble *CurList, *PrevCurList, *TestList, *LastList = NULL, *LocalLastList, *ShortPolyList, 
	*FirstAddedPolygon, *LastOriginalPolygon, *AddedPolygons = NULL;
VectorPolygon *BlockPoly = NULL;
VectorNode *BlockNodePtrs[4];
VectorIntersecter VI;
VectorNode BlockNodes[4];

#ifdef _DEBUG
for (PolyCt = 0, CurList = PolygonList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList, ++PolyCt)
	{
	CurList->GetPolygon()->SetPolyNumber(PolyCt);
	} // for
#ifdef DEBUG_POLYGONS_TO_VECTOR1
//OutputVectorsToDatabase(PolygonList, "Polygons built", NULL);
//return (false);
#endif // DEBUG_POLYGONS_TO_VECTOR1
#endif // _DEBUG

InitInBlocks = false;
InitNecessary = true;
TerrainBoundsValid = false;
PolyCt = 0;

#ifdef WCS_TEST_EARLY_TERRAIN_INIT
Rend->LocalLog->PostError(WCS_LOG_SEVERITY_MSG, "Terrain bounds limit applied to effect initilization.");
if (Rend->DEMCueReady())
	{
	if (Rend->DEMCueGetBounds(TerrainLowLat, TerrainLowLon, TerrainHighLat, TerrainHighLon))
		TerrainBoundsValid = true;
	else
		InitNecessary = false;
	} // if
#endif // WCS_TEST_EARLY_TERRAIN_INIT

if (InitNecessary && Proj->Prefs.PublicQueryConfigOptTrue("effect_block_init"))
	{
	Rend->LocalLog->PostError(WCS_LOG_SEVERITY_MSG, "Effect block initialization enabled.");
	NumLatBlocks = NumLonBlocks = 1;

	if (NumPolygons > 1000)
		{
		if (Success = GetPolygonRange(LowLat, LowLon, HighLat, HighLon))
			{
			#ifdef WCS_TEST_EARLY_TERRAIN_INIT
			if (TerrainBoundsValid)
				{
				// reduce size of bloc by terrain bounds
				if (LowLat >= TerrainHighLat || HighLat <= TerrainLowLat || LowLon >= TerrainHighLon || HighLon <= TerrainLowLon)
					InitNecessary = false;
				else
					{
					bool BoundsChanged = false;
					if (TerrainHighLat < HighLat)
						{
						HighLat = TerrainHighLat;
						BoundsChanged = true;
						} // if
					if (TerrainHighLon < HighLon)
						{
						HighLon = TerrainHighLon;
						BoundsChanged = true;
						} // if
					if (TerrainLowLat > LowLat)
						{
						LowLat = TerrainLowLat;
						BoundsChanged = true;
						} // if
					if (TerrainLowLon > LowLon)
						{
						LowLon = TerrainLowLon;
						BoundsChanged = true;
						} // if
					if (BoundsChanged)
						{
						Success = WeedOutOfBoundsPolygons(LowLat, LowLon, HighLat, HighLon, NumPolygons);
						if (NumPolygons == 0)
							InitNecessary = false;
						} // if
					} // else
				} // if
			#endif // WCS_TEST_EARLY_TERRAIN_INIT
			if (Success && InitNecessary)
				{
				if ((NumBlocks = ceil(NumPolygons / 1000.0)) > 1)
					{
					NumLatBlocks = (long)ceil(sqrt(NumBlocks * (HighLat - LowLat) / (HighLat - LowLat + HighLon - LowLon)));
					NumLonBlocks = (long)ceil(sqrt(NumBlocks * (HighLon - LowLon) / (HighLat - LowLat + HighLon - LowLon)));
					LatBlockSize = (HighLat - LowLat) / NumLatBlocks;
					LonBlockSize = (HighLon - LowLon) / NumLonBlocks;
					if (NumLatBlocks > 1 || NumLonBlocks > 1)
						InitInBlocks = true;
					} // if
				} // if
			} // if
		} // if
	} // if

if (Success)
	{
	if (InitNecessary)
		{
		if (InitInBlocks)
			{
			BlockNodePtrs[0] = &BlockNodes[0];
			BlockNodePtrs[1] = &BlockNodes[1];
			BlockNodePtrs[2] = &BlockNodes[2];
			BlockNodePtrs[3] = &BlockNodes[3];
			// make a polygon from the nodes
			if (BlockPoly = new VectorPolygon(BlockNodePtrs, 4, NULL))
				{
				for (CurLatBlock = 0; CurLatBlock < NumLatBlocks; ++CurLatBlock)
					{
					for (CurLonBlock = 0; CurLonBlock < NumLonBlocks; ++CurLonBlock)
						{
						bool LocalSuccess = true;
						// set coordinates in four nodes
						// NW
						BlockNodes[0].Lat = (CurLatBlock + 1) * LatBlockSize + LowLat;
						BlockNodes[0].Lon = (CurLonBlock + 1) * LonBlockSize + LowLon;
						// NE
						BlockNodes[1].Lat = BlockNodes[0].Lat;
						BlockNodes[1].Lon = CurLonBlock * LonBlockSize + LowLon;
						// SE
						BlockNodes[2].Lat = CurLatBlock * LatBlockSize + LowLat;
						BlockNodes[2].Lon = BlockNodes[1].Lon;
						// SW
						BlockNodes[3].Lat = BlockNodes[2].Lat;
						BlockNodes[3].Lon = BlockNodes[0].Lon;
						BlockPoly->SetBoundingBox();
						ShortPolyList = CreateBoundedPolygonList(NULL, BlockPoly->BBox, LocalSuccess);
						LocalLastList = FirstAddedPolygon = NULL;
						
						// prequel - align any points that are pretty darned close
						// testing only area effect polygons
						// we want to move only area effects but test against tfx or stream vertices that
						// are on the original vector.
						for (CurList = ShortPolyList; CurList && CurList->NextPolygonList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
							{
							if (CurList->MyPolygon->ImAnOriginalPolygon)
								{
								for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
									{
									// this is going to be replaced by the line below once testing is done
									// As originally implemented cleanup is only done if both polygons are original.
									// Modification conceived as doing the cleanup even if the second polygon is not original and modifying the original polygon if the second poly is not original
									// I don't recall the reasoning behind that distinction as of 2/10/11 - GRH
									if (TestList->MyPolygon->ImAnOriginalPolygon)
										{
										// false default argument added 2/10/11 to make the function compliant with new syntax but keep original behavior
										// Polygon that will be modified is the one with the lesser number of links to other nodes.
										CurList->MyPolygon->CleanupPolygonRelationships(TestList->MyPolygon);
										} // if
									//CurList->MyPolygon->CleanupPolygonRelationships(TestList->MyPolygon, ! TestList->MyPolygon->ImAnOriginalPolygon);
									} // for
								} // if
							if (! UpdateGauge(++PolyCt))
								Success = false;
							} // for
							
						// first phase
						// if there are tfx or stream polygons link them first to those of the same vector/effect
						for (CurList = ShortPolyList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
							{
							if (! CurList->MyPolygon->ImAnOriginalPolygon)
								{
								for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
									{
									if (! TestList->MyPolygon->ImAnOriginalPolygon && CurList->MyPolygon->MyEffects->MyEffect == TestList->MyPolygon->MyEffects->MyEffect
										&& CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe)
										{
										Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
										if (AddedPolygons)
											{
											if (! LocalLastList)
												{
												FirstAddedPolygon = AddedPolygons;
												for (LocalLastList = TestList; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
													;
												LastOriginalPolygon = LocalLastList;
												} // if
											LocalLastList->NextPolygonList = AddedPolygons;
											for (LocalLastList = AddedPolygons; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
												;
											AddedPolygons = NULL;
											} // if
										if (! Success)
											break;
										} // if
									else
										break;
									} // for
								} // if
							if (! UpdateGauge(++PolyCt))
								Success = false;
							} // for

						// second phase
						// if there are tfx or stream polygons link them next to those that share an end vertex where tfx or stream segments join
						for (CurList = ShortPolyList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
							{
							if (! CurList->MyPolygon->ImAnOriginalPolygon)
								{
								for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
									{
									if (! TestList->MyPolygon->ImAnOriginalPolygon &&
										((CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
										CurList->MyPolygon->MyEffects->MyEffect != TestList->MyPolygon->MyEffects->MyEffect) ||
										(CurList->MyPolygon->MyEffects->MyJoe != TestList->MyPolygon->MyEffects->MyJoe && 
										CurList->MyPolygon->MyEffects->MyJoe->MatchEndPoint(TestList->MyPolygon->MyEffects->MyJoe))))
										{
										Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
										if (AddedPolygons)
											{
											if (! LocalLastList)
												{
												FirstAddedPolygon = AddedPolygons;
												for (LocalLastList = TestList; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
													;
												LastOriginalPolygon = LocalLastList;
												} // if
											LocalLastList->NextPolygonList = AddedPolygons;
											for (LocalLastList = AddedPolygons; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
												;
											AddedPolygons = NULL;
											} // if
										if (! Success)
											break;
										} // if
									} // for
								} // if
							if (! UpdateGauge(++PolyCt))
								Success = false;
							} // for
						
						// follow with intersection of all non-genetically related individuals
						// find all the intersections between polygons and insert nodes at the intersecting points
						// remove vestigial segments or degenerate polygons resulting from insertion of nodes as each polygon is treated
						// Create links between polygons as you go.
						for (CurList = ShortPolyList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
							{
							// walk all the following polygons and intersect them with the current polygon
							// nodes that are closer than a tolerance value will be linked to each other for possible merging
							// into the same coordinates
							for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
								{
								// Those that have already been done need not be done again
								if (CurList->MyPolygon->ImAnOriginalPolygon || TestList->MyPolygon->ImAnOriginalPolygon ||
									! ((CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
									CurList->MyPolygon->MyEffects->MyEffect == TestList->MyPolygon->MyEffects->MyEffect) ||
									(CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
									CurList->MyPolygon->MyEffects->MyEffect != TestList->MyPolygon->MyEffects->MyEffect) ||
									(CurList->MyPolygon->MyEffects->MyJoe != TestList->MyPolygon->MyEffects->MyJoe && 
									CurList->MyPolygon->MyEffects->MyJoe->MatchEndPoint(TestList->MyPolygon->MyEffects->MyJoe))))
									{
									Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
									if (AddedPolygons)
										{
										if (! LocalLastList)
											{
											FirstAddedPolygon = AddedPolygons;
											for (LocalLastList = TestList; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
												;
											LastOriginalPolygon = LocalLastList;
											} // if
										LocalLastList->NextPolygonList = AddedPolygons;
										// so that all the necessary polygon comparisons get made down at the end of the list
										if (CurList->MyPolygon->ImAnOriginalPolygon || TestList->MyPolygon->ImAnOriginalPolygon)
											{
											for (LocalLastList = AddedPolygons; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
												LocalLastList->MyPolygon->ImAnOriginalPolygon = true;
											} // if
										else
											{
											for (LocalLastList = AddedPolygons; LocalLastList->NextPolygonList; LocalLastList = (VectorPolygonListDouble *)LocalLastList->NextPolygonList)
												;
											} // if
										AddedPolygons = NULL;
										} // if
									if (! Success)
										break;
									} // if
								} // for
							if (! UpdateGauge(++PolyCt))
								Success = false;
							} // for

						if (FirstAddedPolygon)
							{
							if (! LastList)
								{
								for (LastList = PolygonList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									;
								} // if
							LastList->NextPolygonList = FirstAddedPolygon;
							for (LastList = FirstAddedPolygon; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
								;
							LastOriginalPolygon->NextPolygonList = NULL;
							} // if
						for (VectorPolygonListDouble *Temp = ShortPolyList; Temp; Temp = ShortPolyList)
							{
							ShortPolyList = (VectorPolygonListDouble *)ShortPolyList->NextPolygonList;
							delete Temp;
							} // for
						} // for
					} // for
				BlockPoly->FirstPart.MyNodes = NULL;
				BlockPoly->FirstPart.NumNodes = 0;
				delete BlockPoly;
				} // if
			else
				Success = false;
			} // if
		else
			{
			// prequel - align any points that are pretty darned close
			for (CurList = PolygonList; CurList && CurList->NextPolygonList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (CurList->MyPolygon->ImAnOriginalPolygon)
					{
					for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
						{
						// this is going to be replaced by the line below once testing is done
						// As originally implemented cleanup is only done if both polygons are original.
						// Modification conceived as doing the cleanup even if the second polygon is not original and modifying the original polygon if the second poly is not original
						// I don't recall the reasoning behind that distinction as of 2/10/11 - GRH
						if (TestList->MyPolygon->ImAnOriginalPolygon)
							{
							// false default argument added 2/10/11 to make the function compliant with new syntax but keep original behavior
							// Polygon that will be modified is the one with the lesser number of links to other nodes.
							CurList->MyPolygon->CleanupPolygonRelationships(TestList->MyPolygon);
							} // if
						//CurList->MyPolygon->CleanupPolygonRelationships(TestList->MyPolygon, ! TestList->MyPolygon->ImAnOriginalPolygon);
						} // for
					} // if
				if (! UpdateGauge(++PolyCt))
					Success = false;
				} // for

			// first phase
			// if there are tfx or stream polygons link them first to those of the same vector/effect
			for (CurList = PolygonList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (! CurList->MyPolygon->ImAnOriginalPolygon)
					{
					for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
						{
						if (! TestList->MyPolygon->ImAnOriginalPolygon && CurList->MyPolygon->MyEffects->MyEffect == TestList->MyPolygon->MyEffects->MyEffect
							&& CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe)
							{
							Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
							if (AddedPolygons)
								{
								if (! LastList)
									{
									for (LastList = TestList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
										;
									} // if
								LastList->NextPolygonList = AddedPolygons;
								for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									;
								AddedPolygons = NULL;
								} // if
							if (! Success)
								break;
							} // if
						else
							break;
						} // for
					} // if
				if (! UpdateGauge(++PolyCt))
					Success = false;
				} // for

			// second phase
			// if there are tfx or stream polygons link them next to those that share an end vertex where tfx or stream segments join
			for (CurList = PolygonList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (! CurList->MyPolygon->ImAnOriginalPolygon)
					{
					for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
						{
						if (! TestList->MyPolygon->ImAnOriginalPolygon &&
							((CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
							CurList->MyPolygon->MyEffects->MyEffect != TestList->MyPolygon->MyEffects->MyEffect) ||
							(CurList->MyPolygon->MyEffects->MyJoe != TestList->MyPolygon->MyEffects->MyJoe && 
							CurList->MyPolygon->MyEffects->MyJoe->MatchEndPoint(TestList->MyPolygon->MyEffects->MyJoe))))
							{
							Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
							if (AddedPolygons)
								{
								if (! LastList)
									{
									for (LastList = TestList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
										;
									} // if
								LastList->NextPolygonList = AddedPolygons;
								for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									;
								AddedPolygons = NULL;
								} // if
							if (! Success)
								break;
							} // if
						} // for
					} // for
				if (! UpdateGauge(++PolyCt))
					Success = false;
				} // for

			// follow with intersection of all non-genetically related individuals
			// find all the intersections between polygons and insert nodes at the intersecting points
			// remove vestigial segments or degenerate polygons resulting from insertion of nodes as each polygon is treated
			// Create links between polygons as you go.
			for (CurList = PolygonList; CurList && CurList->NextPolygonList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				// walk all the following polygons and intersect them with the current polygon
				// nodes that are closer than a tolerance value will be linked to each other for possible merging
				// into the same coordinates
				for (TestList = (VectorPolygonListDouble *)CurList->NextPolygonList; TestList; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
					{
					// Those that have already been done need not be done again
					if (CurList->MyPolygon->ImAnOriginalPolygon || TestList->MyPolygon->ImAnOriginalPolygon ||
						! ((CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
						CurList->MyPolygon->MyEffects->MyEffect == TestList->MyPolygon->MyEffects->MyEffect) ||
						(CurList->MyPolygon->MyEffects->MyJoe == TestList->MyPolygon->MyEffects->MyJoe && 
						CurList->MyPolygon->MyEffects->MyEffect != TestList->MyPolygon->MyEffects->MyEffect) ||
						(CurList->MyPolygon->MyEffects->MyJoe != TestList->MyPolygon->MyEffects->MyJoe && 
						CurList->MyPolygon->MyEffects->MyJoe->MatchEndPoint(TestList->MyPolygon->MyEffects->MyJoe))))
						{
						Success = VI.TestIntersect(CurList->MyPolygon, TestList->MyPolygon, AddedPolygons, true);
						if (AddedPolygons)
							{
							if (! LastList)
								{
								for (LastList = TestList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									;
								} // if
							LastList->NextPolygonList = AddedPolygons;
							// so that all the necessary polygon comparisons get made down at the end of the list
							if (CurList->MyPolygon->ImAnOriginalPolygon || TestList->MyPolygon->ImAnOriginalPolygon)
								{
								for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									LastList->MyPolygon->ImAnOriginalPolygon = true;
								} // if
							else
								{
								for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
									;
								} // else
							AddedPolygons = NULL;
							} // if
						if (! Success)
							break;
						} // if
					} // for
				if (! UpdateGauge(++PolyCt))
					Success = false;
				} // for
			} // else

		if (Success)
			{
			bool NodeRepExists = false;
			// walk the polygon list and merge nodes that are linked to each other if they are closer than a tolerance amount
			for (CurList = PolygonList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (! (Success = CurList->MergeAdjoiningNodes(NodeRepExists)))
					break;
				if (! UpdateGauge(++PolyCt))
					{
					Success = false;
					break;
					}
				} // for
			if (Success && NodeRepExists)
				{
				for (PrevCurList = NULL, CurList = PolygonList; CurList && Success; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: PolygonList)
					{
					if (CurList->MyPolygon)
						CurList->RemoveVestigialSegments();
					// if merge or vestigial functions functions deleted the current polygon
					if (! CurList->MyPolygon)
						{
						if (PrevCurList)
							{
							PrevCurList->NextPolygonList = CurList->NextPolygonList;
							delete CurList;
							CurList = PrevCurList;
							} // if
						else
							{
							PolygonList = (VectorPolygonListDouble *)CurList->NextPolygonList;
							delete CurList;
							CurList = NULL;
							} // else
						} // if
					PrevCurList = CurList;
					if (! UpdateGauge(++PolyCt))
						{
						Success = false;
						break;
						}
					} // for
				} // if
			} // if
		} // if
	else
		{
		// delete polygon list
		DestroyPolygonList(NULL);
		} // else
	} // if
	
return (Success);

} // EffectEval::IntersectPolygons

/*===========================================================================*/

bool EffectEval::IntersectPolygonsWithTerrainCell(VectorPolygonListDouble *&MergeList)
{
bool Success = true;
VectorPolygonListDouble *CurList, *TestList, *LastList = NULL, *AddedPolygons = NULL;
VectorPolygonListDouble *PrevCurList;
VectorIntersecter VI;

// find all the intersections between polygons and insert nodes at the intersecting points
// Create links between polygons as you go.
for (TestList = (VectorPolygonListDouble *)MergeList->NextPolygonList; TestList && Success; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
	{
	// walk all the following polygons and intersect them with the current polygon
	// nodes that are closer than a tolerance value will be linked to each other for possible merging
	// into the same coordinates
	Success = VI.TestIntersect(MergeList->MyPolygon, TestList->MyPolygon, AddedPolygons, false);
	if (AddedPolygons)
		{
		if (! LastList)
			{
			for (LastList = TestList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
				;
			} // if
		LastList->NextPolygonList = AddedPolygons;
		for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
			;
		AddedPolygons = NULL;
		} // if
	if (! Success)
		break;
	} // for

if (Success)
	{
	bool NodeRepExists = false;
	
	if (Success = MergeList->MergeAdjoiningNodes(NodeRepExists))
		{
		// Terrain cell is just a box so I don't think it needs vestigial segments removed
		// if merge deleted the current polygon
		if (! MergeList->MyPolygon)
			{
			CurList = MergeList;
			MergeList = (VectorPolygonListDouble *)MergeList->NextPolygonList;
			delete CurList;
			} // if
		} // if
	if (Success && NodeRepExists)
		{
		// walk the polygon list and merge nodes that are linked to each other if they are closer than a tolerance amount
		for (PrevCurList = NULL, CurList = MergeList; CurList; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: MergeList)
			{
			if (CurList->MyPolygon)
				CurList->RemoveVestigialSegments();
			// if vestigial functions functions deleted the current polygon
			if (! CurList->MyPolygon)
				{
				if (PrevCurList)
					{
					PrevCurList->NextPolygonList = CurList->NextPolygonList;
					delete CurList;
					CurList = PrevCurList;
					} // if
				else
					{
					MergeList = (VectorPolygonListDouble *)CurList->NextPolygonList;
					delete CurList;
					CurList = NULL;
					} // else
				} // if
			PrevCurList = CurList;
			} // for
		} // if
	} // if

return (Success);

} // EffectEval::IntersectPolygonsWithTerrainCell

/*===========================================================================*/

bool EffectEval::IntersectPolygonsWithMultipleTerrainCells(VectorPolygonListDouble *&TerrainPolygons, VectorPolygonListDouble *EffectPolygons)
{
bool Success = true;
VectorPolygonListDouble *CurList, *PrevCurList, *TestList, *TerrainList, *LastList = NULL, *AddedPolygons = NULL;
VectorIntersecter VI;

// for debugging
#if defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3
if (PrintToVector)
	{
	unsigned long PolyCt;
	for (PolyCt = 0, CurList = TerrainPolygons; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList, ++PolyCt)
		{
		CurList->GetPolygon()->SetPolyNumber(PolyCt);
		} // for
	} // if
#endif // DEBUG_POLYGONS_TO_VECTOR2

// find all the intersections between polygons and insert nodes at the intersecting points
// Create links between polygons as you go. Link only the ones with effects with ones that don't have effects.
// The ones with no effects are the subdivided terrain.

for (TerrainList = TerrainPolygons; TerrainList != EffectPolygons && Success; TerrainList = (VectorPolygonListDouble *)TerrainList->NextPolygonList)
	{
	for (TestList = (VectorPolygonListDouble *)EffectPolygons; TestList && Success; TestList = (VectorPolygonListDouble *)TestList->NextPolygonList)
		{
		// walk all the following polygons and intersect them with the current polygon
		// nodes that are closer than a tolerance value will be linked to each other for possible merging
		// into the same coordinates
		Success = VI.TestIntersect(TerrainList->MyPolygon, TestList->MyPolygon, AddedPolygons, false);
		if (AddedPolygons)
			{
			if (! LastList)
				{
				for (LastList = TestList; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
					;
				} // if
			LastList->NextPolygonList = AddedPolygons;
			for (LastList = AddedPolygons; LastList->NextPolygonList; LastList = (VectorPolygonListDouble *)LastList->NextPolygonList)
				;
			AddedPolygons = NULL;
			} // if
		if (! Success)
			break;
		} // for
	} // for

if (Success)
	{
	bool NodeRepExists = false;
	#if defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3
	if (PrintToVector)
		{
		//OutputVectorsToDatabase(TerrainPolygons, "Multi-cell intersect output pre-node merge", NULL);
		} // if
	#endif // DEBUG_POLYGONS_TO_VECTOR2
	// walk the polygon list and merge nodes that are linked to each other if they are closer than a tolerance amount
	for (CurList = TerrainPolygons; CurList /*&& CurList != EffectPolygons*/; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
		{
		if (! (Success = CurList->MergeAdjoiningNodes(NodeRepExists)))
			break;
		} // for
	if (Success && NodeRepExists)
		{
		for (PrevCurList = NULL, CurList = TerrainPolygons; CurList; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: TerrainPolygons)
			{
			if (CurList->MyPolygon)
				CurList->RemoveVestigialSegments();
			// if merge or vestigial functions functions deleted the current polygon
			if (! CurList->MyPolygon)
				{
				if (PrevCurList)
					{
					PrevCurList->NextPolygonList = CurList->NextPolygonList;
					delete CurList;
					CurList = PrevCurList;
					} // if
				else
					{
					TerrainPolygons = (VectorPolygonListDouble *)CurList->NextPolygonList;
					delete CurList;
					CurList = NULL;
					} // else
				} // if
			PrevCurList = CurList;
			} // for
		} // if
	} // if

return (Success);

} // EffectEval::IntersectPolygonsWithMultipleTerrainCells

/*===========================================================================*/

void EffectEval::PrepareToMerge(VectorPolygonListDouble *ListToMerge)
{
unsigned long PolyCt;
VectorPolygonListDouble *CurList;
VectorPolygon *CurPoly;

for (PolyCt = 0, CurList = ListToMerge; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList, ++PolyCt)
	{
	CurPoly = CurList->GetPolygon();
	CurPoly->SetPolyNumber(PolyCt);
	CurPoly->LinkBackwards();
	CurPoly->ClearIntersectionFlags();
	#ifdef MAKENOCLONES
	if (! CurPoly->CloneOfThis)
		CurPoly->SetClonePointer((VectorPolygon *)~0);
	#endif // MAKENOCLONES
	} // for
	
} // EffectEval::PrepareToMerge

/*===========================================================================*/

// Mark all shared edges so that each edge can only be used once in each direction.
// In the process, note which polygons lie to the right of each node. If a segment lies inside
// another polygon, that polygon should be reflected in the "right side polygon" list for a node.
// A segment is inside if any point along it is inside. Since the end points might be coincident with nodes
// of the other polygon, the nodes themselves cannot be used for testing.
// Polygons that lie completely inside are dealt with in the intersection stage so need not be addressed here.
// Polygons that partly overlap each other will intersect at some point.
bool EffectEval::DisableSharedEdges(VectorPolygonListDouble *ListToDisable, bool PostProgress)
{
unsigned long NodeCt, PolyCt;
VectorPolygonListDouble *CurList;
VectorPolygonList *InsidePolygonList, *CurInsidePolygon, *PrevInsidePolygon, **NextPolygonListPtr;
VectorNode *CurNode, *LastNodeAddedToInsideList;
VectorPart *CurPart;
VectorNodeLink *CurLinkedNode, *NextLinkedNode;
VectorPolygon *CurPoly;
bool Success = true, SegmentShared, SegmentIsInside, FoundInList;
VectorNode TestNode;

for (PolyCt = 0, CurList = ListToDisable; CurList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
	{
	CurPoly = CurList->GetPolygon();
	for (CurPart = CurPoly->GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
		{
		InsidePolygonList = NULL;
		LastNodeAddedToInsideList = NULL;
		for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes && Success; ++NodeCt, CurNode = CurNode->NextNode)
			{
			// F2_NOTE: vvv CPI ~= 2.9
			for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode && Success; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
				{
				CurLinkedNode->FlagSet(WCS_VECTORNODELINK_FLAG_USETOMERGE);
				#ifdef MAKENOCLONES
				// only polygons with CloneOfThis set to non-NULL are to be considered in any merge activity
				// F2_NOTE: vvv CPI ~= 3.75
				if (CurLinkedNode->LinkedPolygon->CloneOfThis)
				#endif // MAKENOCLONES
					{
					SegmentShared = false;
					// see if the linked polygon matches a linked polygon at the next node
					// F2_NOTE: vvv CPI = 3
					for (NextLinkedNode = CurNode->NextNode->LinkedNodes; NextLinkedNode; NextLinkedNode = (VectorNodeLink *)NextLinkedNode->NextNodeList)
						{
						if (NextLinkedNode->LinkedPolygon == CurLinkedNode->LinkedPolygon)
							{
							// see if the forward link is to the other node
							// F2_NOTE: vvv CPI ~= 3.5
							if (CurLinkedNode->MyNode->NextNode == NextLinkedNode->MyNode)
								{
								// if a segment is already flagged as used then all shared segments will already be so marked as well
								// If there are no links for the next node then the current segment can not be a shared one
								if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
									{
									// skip it if the linked polygon is a lower number, it's already been checked
									if (CurLinkedNode->LinkedPolygon->PolyNumber >= CurPoly->PolyNumber)
										{
										// disable the forward link for the linked node 
										CurLinkedNode->MyNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED);
										// and add the polygon to the "right side" list for this node
										// true indicates the edge is shared, not that the edge is inside the linked polygon
										if (! CurNode->AddRightSideMember(CurLinkedNode->LinkedPolygon, true))
											{
											Success = false;
											break;
											} // if
										// disable the backward link for the linked node
										CurLinkedNode->MyNode->NextNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED);
										} // if
									} // if
								SegmentShared = true;
								} // if
							// see if the backward link is to the other node
							else if (CurLinkedNode->MyNode->PrevNode == NextLinkedNode->MyNode)
								{
								// if a segment is already flagged as used then all shared segments will already be so marked as well
								// If there are no links for the next node then the current segment can not be a shared one
								if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
									{
									// skip it if the linked polygon is a lower number, it's already been checked
									if (CurLinkedNode->LinkedPolygon->PolyNumber >= CurPoly->PolyNumber)
										{
										// disable the backward link for the both the linked node and the next node in the current polygon
										CurLinkedNode->MyNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED);
										CurNode->NextNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED);
										} // if
									} // if
								SegmentShared = true;
								} // else if
							} // if
						} // if
					if (! SegmentShared)
						{
						// since it isn't a shared segment with the linked polygon, it is possible that
						// the following segment is inside the linked polygon.
						// If it IS inside then the polygon needs to be added to a list of polygons that
						// should be tested again when another link is found to that polygon and, until then,
						// each segment should have that polygon added to its right side list
						// Test the members of the list to see if it is already in the list
						TestNode.Lat = (CurNode->Lat + CurNode->NextNode->Lat) * .5;
						TestNode.Lon = (CurNode->Lon + CurNode->NextNode->Lon) * .5;
						SegmentIsInside = (CurLinkedNode->LinkedPolygon->TestPointContained(&TestNode, 0.0) == WCS_TEST_POINT_CONTAINED_INSIDE);
						FoundInList = false;
						PrevInsidePolygon = NULL;
						for (CurInsidePolygon = InsidePolygonList; CurInsidePolygon; CurInsidePolygon = CurInsidePolygon->NextPolygonList)
							{
							if (CurInsidePolygon->MyPolygon == CurLinkedNode->LinkedPolygon)
								{
								FoundInList = true;
								break;
								} // if
							PrevInsidePolygon = CurInsidePolygon;
							} // for
						if (FoundInList)
							{
							// has been inside up to this point
							if (! SegmentIsInside)
								{
								// no longer inside so remove from list to test
								if (PrevInsidePolygon)
									PrevInsidePolygon->NextPolygonList = CurInsidePolygon->NextPolygonList;
								else
									InsidePolygonList = CurInsidePolygon->NextPolygonList;
								delete CurInsidePolygon;
								} // if
							} // if
						else
							{
							// is not inside up to this point
							if (SegmentIsInside)
								{
								// add the linked polygon to the inside list
								if (PrevInsidePolygon)
									NextPolygonListPtr = &PrevInsidePolygon->NextPolygonList;
								else
									NextPolygonListPtr = &InsidePolygonList;
								if (*NextPolygonListPtr = new VectorPolygonList())
									(*NextPolygonListPtr)->MyPolygon = CurLinkedNode->LinkedPolygon;
								else
									{
									Success = false;
									break;
									} // else
								} // if
							else
								LastNodeAddedToInsideList = CurNode;
							} // else
						} // if
					else
						{
						// segment is shared so if the polygon was listed as inside before, it needs to be removed
						FoundInList = false;
						PrevInsidePolygon = NULL;
						for (CurInsidePolygon = InsidePolygonList; CurInsidePolygon; CurInsidePolygon = CurInsidePolygon->NextPolygonList)
							{
							if (CurInsidePolygon->MyPolygon == CurLinkedNode->LinkedPolygon)
								{
								FoundInList = true;
								break;
								} // if
							PrevInsidePolygon = CurInsidePolygon;
							} // for
						if (FoundInList)
							{
							// has been inside up to this point
							// no longer inside so remove from list to test
							if (PrevInsidePolygon)
								PrevInsidePolygon->NextPolygonList = CurInsidePolygon->NextPolygonList;
							else
								InsidePolygonList = CurInsidePolygon->NextPolygonList;
							delete CurInsidePolygon;
							} // if
						} // else
					} // if
				} // for CurLinkedNode
			// the list of polygons that this node is inside is complete
			// but no use adding right side members if the segment will never be used
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
				{
				for (CurInsidePolygon = InsidePolygonList; CurInsidePolygon; CurInsidePolygon = CurInsidePolygon->NextPolygonList)
					{
					// false indicates that the edge is inside the the other polygon, not a shared edge
					if (! (Success = CurNode->AddRightSideMember(CurInsidePolygon->MyPolygon, false)))
						break;
					} // for
				} // if
			} // for CurNode
		if (InsidePolygonList && LastNodeAddedToInsideList)
			{
			// walk the remainder of the nodes and add right side members until the point is hit where the last of the
			// items in the check list have been dealt with. The check list will be depleted whenever a node is encountered
			// that is linked to the item in the list.
			for (; InsidePolygonList && CurNode != LastNodeAddedToInsideList && Success; CurNode = CurNode->NextNode)
				{
				PrevInsidePolygon = NULL;
				for (CurInsidePolygon = InsidePolygonList; CurInsidePolygon; CurInsidePolygon = CurInsidePolygon ? CurInsidePolygon->NextPolygonList: InsidePolygonList)
					{
					// see if there is a link to the polygon at the current node
					FoundInList = false;
					// F2_NOTE: vvv CPI ~= 2.5
					for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
						{
						if (CurInsidePolygon->MyPolygon == CurLinkedNode->LinkedPolygon)
							{
							FoundInList = true;
							break;
							} // if
						} // for
					if (FoundInList)
						{
						// remove polygon from check list
						if (PrevInsidePolygon)
							PrevInsidePolygon->NextPolygonList = CurInsidePolygon->NextPolygonList;
						else
							InsidePolygonList = CurInsidePolygon->NextPolygonList;
						delete CurInsidePolygon;
						CurInsidePolygon = PrevInsidePolygon;
						} // if
					else
						{
						// apply to right side list
						// false indicates that the edge is inside the the other polygon, not a shared edge
						if (! (Success = CurNode->AddRightSideMember(CurInsidePolygon->MyPolygon, false)))
							break;
						} // else
					PrevInsidePolygon = CurInsidePolygon;
					} // for
				} // for
			for (CurInsidePolygon = InsidePolygonList; CurInsidePolygon; CurInsidePolygon = InsidePolygonList)
				{
				InsidePolygonList = InsidePolygonList->NextPolygonList;
				delete CurInsidePolygon;
				} // for
			} // if
		} // for CurPart
	if (PostProgress)
		{
		if (! UpdateGauge(++PolyCt))
			Success = false;
		} // if
	} // for CurList

return (Success);

} // EffectEval::DisableSharedEdges

/*===========================================================================*/

bool EffectEval::CreateNodeLoop(VectorPolygon *StartPoly, VectorNode *StartNode, VectorNode **&OutputNodes, 
	VectorPolygon **&NodeOwners, VectorPolygon **&OutputPolygons, unsigned long &NodesUsed, 
	unsigned long &PolygonsUsed, unsigned long &NumNodesAllocated, unsigned long &NumPolysAllocated,
	PolygonBoundingBox *TestBounds, bool &PolyIsPure)
{
double BestAngle, TestAngle;
unsigned long PolyCheck;
int CurDirection, NewDirection;
VectorNode *PrevNode, *CurNode, *NewCurNode, *BestNode;
VectorNodeLink *LinkedNode;
VectorPolygon *BestPoly;
VectorPolygonList *RightSideLoop;
bool BreakOut, PolygonFound, BoundsFailed, Success = true;
TriangleBoundingBoxVector TBxFrom, TBxTo, AngleFinder;

PolyIsPure = true;
NodesUsed = 0;
PolygonsUsed = 0;

if (! TestBounds || TestBounds->TestPointInBox(StartNode->NextNode))
	{
	// add StartPoly to the list of polygons represented in this new polygon
	// Also add any polygons in StartNode's "right side" list
	// Add the first segment to the new node list
	NodeOwners[NodesUsed] = StartPoly;
	OutputNodes[NodesUsed ++] = StartNode;
	PrevNode = NULL;
	NewCurNode = CurNode = StartNode;
	BestNode = StartNode->NextNode;
	NewDirection = CurDirection = WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED;
	CurNode->FlagSet(CurDirection);
	BestPoly = StartPoly;
	BreakOut = false;
	while (! BreakOut)
		{
		// Allow room to grow and for NULL entry to be appended to each list
		if (NodesUsed >= NumNodesAllocated - 2)
			{
			VectorNode **TempOutputNodes;
			VectorPolygon **TempOutputOwners;
			
			if (TempOutputNodes = (VectorNode **)AppMem_Alloc((NumNodesAllocated + 1000) * sizeof (VectorNode *), 0))
				{
				memcpy(TempOutputNodes, OutputNodes, NumNodesAllocated * sizeof (VectorNode *));
				AppMem_Free(OutputNodes, NumNodesAllocated * sizeof (VectorNode *));
				OutputNodes = TempOutputNodes;
				} // if
			if (TempOutputOwners = (VectorPolygon **)AppMem_Alloc((NumNodesAllocated + 1000) * sizeof (VectorPolygon *), 0))
				{
				memcpy(TempOutputOwners, NodeOwners, NumNodesAllocated * sizeof (VectorPolygon *));
				AppMem_Free(NodeOwners, NumNodesAllocated * sizeof (VectorPolygon *));
				NodeOwners = TempOutputOwners;
				} // if
			NumNodesAllocated += 1000;
			} // if
		if (PolygonsUsed >= NumPolysAllocated - 2)
			{
			VectorPolygon **TempOutputPolygons;
			
			if (TempOutputPolygons = (VectorPolygon **)AppMem_Alloc((NumPolysAllocated + 1000) * sizeof (VectorPolygon *), 0))
				{
				memcpy(TempOutputPolygons, OutputPolygons, NumPolysAllocated * sizeof (VectorPolygon *));
				AppMem_Free(OutputPolygons, NumPolysAllocated * sizeof (VectorPolygon *));
				OutputPolygons = TempOutputPolygons;
				NumPolysAllocated += 1000;
				} // if
			} // if
		NodeOwners[NodesUsed] = BestPoly ? BestPoly: NodeOwners[NodesUsed - 1];
		OutputNodes[NodesUsed ++] = BestNode;
		CurNode = NewCurNode;
		CurNode->FlagSet(NewDirection);
		if (NewDirection == WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED)
			{
			if (BestPoly)
				{
				// Add BestPoly to the inside list
				PolygonFound = false;
				for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
					{
					if (OutputPolygons[PolyCheck] == BestPoly)
						{
						PolygonFound = true;
						break;
						} // if
					} // for
				if (! PolygonFound)
					{
					OutputPolygons[PolygonsUsed ++] = BestPoly;
					} // if
				// Add any "right side" polygons to the inside list
				for (RightSideLoop = CurNode->RightSideList; RightSideLoop; RightSideLoop = RightSideLoop->NextPolygonList)
					{
					PolygonFound = false;
					for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
						{
						if (OutputPolygons[PolyCheck] == RightSideLoop->MyPolygon)
							{
							PolygonFound = true;
							break;
							} // if
						} // for
					if (! PolygonFound)
						{
						OutputPolygons[PolygonsUsed ++] = RightSideLoop->MyPolygon;
						} // if
					} // for
				} // if
			} // if
		PrevNode = CurNode;
		CurNode = BestNode;
		CurDirection = NewDirection;
		// set sufficiently large
		BestAngle = 4.0;
		BestNode = NULL;
		BoundsFailed = false;
		// this remains NULL until another polygon is found
		BestPoly = NULL;
		TBxFrom.SetXY(CurNode, PrevNode);
		// find the next node
		// test the next node in the current chain to see if the segment is used in the current direction
		if (CurDirection == WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED)
			{
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
				{
				//TestAngle = angle from CurNode to CurNode->NextNode
				TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(CurNode->NextNode, CurNode));
				if (! TestBounds || TestBounds->TestPointInBox(CurNode->NextNode))
					{
					if (TestAngle < BestAngle)
						{
						NewCurNode = CurNode;
						BestNode = CurNode->NextNode;
						BestAngle = TestAngle;
						NewDirection = WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED;
						} // if
					} // if
				else if (TestAngle < BestAngle)
					{
					BestAngle = TestAngle;
					BoundsFailed = true;
					} // else if best angle so far
				} // if
			} // if
		else
			{
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED) && CurNode->PrevNode)
				{
				//TestAngle = angle from CurNode to CurNode->PrevNode
				TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(CurNode->PrevNode, CurNode));
				if (! TestBounds || TestBounds->TestPointInBox(CurNode->PrevNode))
					{
					if (TestAngle < BestAngle)
						{
						NewCurNode = CurNode;
						BestNode = CurNode->PrevNode;
						BestAngle = TestAngle;
						NewDirection = WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED;
						} // if
					} // if
				else if (TestAngle < BestAngle)
					{
					BestAngle = TestAngle;
					BoundsFailed = true;
					} // else if best angle so far
				} // if
			} // else
		// test its angle with the previous segment
		// if there are linked nodes test each of them in both the forward and backward direction
		for (LinkedNode = CurNode->LinkedNodes; LinkedNode && ! BreakOut; LinkedNode = (VectorNodeLink *)LinkedNode->NextNodeList)
			{
			// test to see if we've come to the end of the links that were established prior to the actual merging
			// Additional links may have been added during merging and formation of new merged polygons.
			// Those links are to the new polygons and should not be tested here.
			#ifdef MAKENOCLONES
			if (! LinkedNode->LinkedPolygon->CloneOfThis)
				continue;
			#endif // MAKENOCLONES
			if (! LinkedNode->FlagCheck(WCS_VECTORNODELINK_FLAG_USETOMERGE))
				break;
			if (! LinkedNode->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
				{
				//TestAngle = angle from CurNode to LinkedNode->MyNode->NextNode
				TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(LinkedNode->MyNode->NextNode, CurNode));
				if (! TestBounds || TestBounds->TestPointInBox(LinkedNode->MyNode->NextNode))
					{
					if (TestAngle < BestAngle)
						{
						NewCurNode = LinkedNode->MyNode;
						BestNode = LinkedNode->MyNode->NextNode;
						BestAngle = TestAngle;
						BestPoly = LinkedNode->LinkedPolygon;
						NewDirection = WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED;
						PolyIsPure = false;
						BoundsFailed = false;
						} // if
					} // if
				else if (TestAngle < BestAngle)
					{
					BestAngle = TestAngle;
					BoundsFailed = true;
					BestNode = NULL;
					} // else if best angle so far
				} // if
			if (! LinkedNode->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED) && LinkedNode->MyNode->PrevNode)
				{
				//TestAngle = angle from CurNode to LinkedNode->MyNode->PrevNode
				TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(LinkedNode->MyNode->PrevNode, CurNode));
				if (! TestBounds || TestBounds->TestPointInBox(LinkedNode->MyNode->PrevNode))
					{
					if (TestAngle < BestAngle)
						{
						NewCurNode = LinkedNode->MyNode;
						BestNode = LinkedNode->MyNode->PrevNode;
						BestAngle = TestAngle;
						BestPoly = LinkedNode->LinkedPolygon;
						NewDirection = WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED;
						PolyIsPure = false;
						BoundsFailed = false;
						} // if
					} // if
				else if (TestAngle < BestAngle)
					{
					BestAngle = TestAngle;
					BoundsFailed = true;
					BestNode = NULL;
					} // else if best angle so far
				} // if
			} // for
		if (BestNode)
			{
			// test to see if BestNode or any of its linked nodes are the StartNode
			if (BestNode == StartNode)
				BreakOut = true;
			else
				{
				for (LinkedNode = BestNode->LinkedNodes; LinkedNode; LinkedNode = (VectorNodeLink *)LinkedNode->NextNodeList)
					{
					if (LinkedNode->MyNode == StartNode)
						{
						BreakOut = true;
						break;
						} // if
					} // for
				} // else
			if (BreakOut)
				{
				NewCurNode->FlagSet(NewDirection);
				if (BestPoly && NewDirection == WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED)
					{
					// Add BestPoly to the inside list
					PolygonFound = false;
					for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
						{
						if (OutputPolygons[PolyCheck] == BestPoly)
							{
							PolygonFound = true;
							break;
							} // if
						} // for
					if (! PolygonFound)
						{
						OutputPolygons[PolygonsUsed ++] = BestPoly;
						} // if
					// Add any "right side" polygons to the inside list
					for (RightSideLoop = NewCurNode->RightSideList; RightSideLoop; RightSideLoop = RightSideLoop->NextPolygonList)
						{
						PolygonFound = false;
						for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
							{
							if (OutputPolygons[PolyCheck] == RightSideLoop->MyPolygon)
								{
								PolygonFound = true;
								break;
								} // if
							} // for
						if (! PolygonFound)
							{
							OutputPolygons[PolygonsUsed ++] = RightSideLoop->MyPolygon;
							} // if
						} // for
					} // if
				} // if
			} // if
		else
			{
			// Bad news - something wrong
			if (BoundsFailed)
				{
				NodesUsed = PolygonsUsed = 0;
				BreakOut = true;
				} // if
			else
				{
				Success = false;
				BreakOut = true;
				} // else
			} // else
		} // while
	} // if
	
OutputNodes[NodesUsed] = NULL;
NodeOwners[NodesUsed] = NULL;
OutputPolygons[PolygonsUsed] = NULL;

return (Success);

} // EffectEval::CreateNodeLoop

/*===========================================================================*/

bool EffectEval::CreateReverseNodeLoop(VectorPolygon *StartPoly, VectorNode *StartNode, VectorNode **&OutputNodes, 
	VectorPolygon **&NodeOwners, VectorPolygon **&OutputPolygons, unsigned long &NodesUsed, 
	unsigned long &PolygonsUsed, unsigned long &NumNodesAllocated, unsigned long &NumPolysAllocated,
	PolygonBoundingBox *TestBounds)
{
double BestAngle, TestAngle;
unsigned long PolyCheck;
int CurDirection, NewDirection;
VectorNode *PrevNode, *CurNode, *NewCurNode, *BestNode;
VectorNodeLink *LinkedNode;
VectorPolygon *BestPoly;
VectorPolygonList *RightSideLoop;
bool BreakOut, PolygonFound, Success = true;
TriangleBoundingBoxVector TBxFrom, TBxTo, AngleFinder;

// add StartPoly to the list of polygons represented in this new polygon
// Also add any polygons in StartNode's "right side" list
// Add the first segment to the new node list
NodesUsed = 0;
PolygonsUsed = 0;

if (StartNode->PrevNode)
	{
	if (! TestBounds || TestBounds->TestPointInBox(StartNode->PrevNode))
		{
		NodeOwners[NodesUsed] = StartPoly;
		OutputNodes[NodesUsed ++] = StartNode;
		PrevNode = NULL;
		NewCurNode = CurNode = StartNode;
		BestNode = StartNode->PrevNode;
		NewDirection = CurDirection = WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED;
		CurNode->FlagSet(CurDirection);
		BestPoly = StartPoly;
		BreakOut = false;
		while (! BreakOut)
			{
			// Allow room to grow and for NULL entry to be appended to each list
			if (NodesUsed >= NumNodesAllocated - 2)
				{
				VectorNode **TempOutputNodes;
				VectorPolygon **TempOutputOwners;
				
				if (TempOutputNodes = (VectorNode **)AppMem_Alloc((NumNodesAllocated + 1000) * sizeof (VectorNode *), 0))
					{
					memcpy(TempOutputNodes, OutputNodes, NumNodesAllocated * sizeof (VectorNode *));
					AppMem_Free(OutputNodes, NumNodesAllocated * sizeof (VectorNode *));
					OutputNodes = TempOutputNodes;
					} // if
				if (TempOutputOwners = (VectorPolygon **)AppMem_Alloc((NumNodesAllocated + 1000) * sizeof (VectorPolygon *), 0))
					{
					memcpy(TempOutputOwners, NodeOwners, NumNodesAllocated * sizeof (VectorPolygon *));
					AppMem_Free(NodeOwners, NumNodesAllocated * sizeof (VectorPolygon *));
					NodeOwners = TempOutputOwners;
					} // if
				NumNodesAllocated += 1000;
				} // if
			if (PolygonsUsed >= NumPolysAllocated - 2)
				{
				VectorPolygon **TempOutputPolygons;
				
				if (TempOutputPolygons = (VectorPolygon **)AppMem_Alloc((NumPolysAllocated + 1000) * sizeof (VectorPolygon *), 0))
					{
					memcpy(TempOutputPolygons, OutputPolygons, NumPolysAllocated * sizeof (VectorPolygon *));
					AppMem_Free(OutputPolygons, NumPolysAllocated * sizeof (VectorPolygon *));
					OutputPolygons = TempOutputPolygons;
					NumPolysAllocated += 1000;
					} // if
				} // if
			NodeOwners[NodesUsed] = BestPoly ? BestPoly: NodeOwners[NodesUsed - 1];
			OutputNodes[NodesUsed ++] = BestNode;
			CurNode = NewCurNode;
			CurNode->FlagSet(NewDirection);
			// Add any "right side" polygons to the inside list
			for (RightSideLoop = BestNode->RightSideList; RightSideLoop; RightSideLoop = RightSideLoop->NextPolygonList)
				{
				if (! RightSideLoop->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_RIGHTSHARE))
					{
					PolygonFound = false;
					for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
						{
						if (OutputPolygons[PolyCheck] == RightSideLoop->MyPolygon)
							{
							PolygonFound = true;
							break;
							} // if
						} // for
					if (! PolygonFound)
						{
						OutputPolygons[PolygonsUsed ++] = RightSideLoop->MyPolygon;
						} // if
					} // if
				} // for
			PrevNode = CurNode;
			CurNode = BestNode;
			CurDirection = NewDirection;
			// set sufficiently large
			BestAngle = 4.0;
			BestNode = NULL;
			// this remains NULL until another polygon is found
			BestPoly = NULL;
			TBxFrom.SetXY(CurNode, PrevNode);
			// find the next node
			// test the next node in the current chain to see if the segment is used in the current direction
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED) && CurNode->PrevNode)
				{
				if (! TestBounds || TestBounds->TestPointInBox(CurNode->PrevNode))
					{
					//TestAngle = angle from CurNode to CurNode->PrevNode
					TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(CurNode->PrevNode, CurNode));
					if (TestAngle < BestAngle)
						{
						NewCurNode = CurNode;
						BestNode = CurNode->PrevNode;
						BestAngle = TestAngle;
						NewDirection = WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED;
						} // if
					} // if
				} // if
			// test its angle with the previous segment
			// if there are linked nodes test each of them in both the forward and backward direction
			for (LinkedNode = CurNode->LinkedNodes; LinkedNode && ! BreakOut; LinkedNode = (VectorNodeLink *)LinkedNode->NextNodeList)
				{
				// test to see if we've come to the end of the links that were established prior to the actual merging
				// Additional links may have been added during merging and formation of new merged polygons.
				// Those links are to the new polygons and should not be tested here.
				#ifdef MAKENOCLONES
				if (! LinkedNode->LinkedPolygon->CloneOfThis)
					continue;
				#endif // MAKENOCLONES
				if (! LinkedNode->FlagCheck(WCS_VECTORNODELINK_FLAG_USETOMERGE))
					break;
				if (! LinkedNode->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED) && LinkedNode->MyNode->PrevNode)
					{
					if (! TestBounds || TestBounds->TestPointInBox(LinkedNode->MyNode->PrevNode))
						{
						//TestAngle = angle from CurNode to LinkedNode->MyNode->PrevNode
						TestAngle = AngleFinder.FindRelativeAngle(&TBxFrom, TBxTo.SetXY(LinkedNode->MyNode->PrevNode, CurNode));
						if (TestAngle < BestAngle)
							{
							NewCurNode = LinkedNode->MyNode;
							BestNode = LinkedNode->MyNode->PrevNode;
							BestAngle = TestAngle;
							BestPoly = LinkedNode->LinkedPolygon;
							} // if
						} // if
					} // if
				} // for
			if (BestNode)
				{
				// test to see if BestNode or any of its linked nodes are the StartNode
				if (BestNode == StartNode)
					BreakOut = true;
				else
					{
					for (LinkedNode = BestNode->LinkedNodes; LinkedNode; LinkedNode = (VectorNodeLink *)LinkedNode->NextNodeList)
						{
						if (LinkedNode->MyNode == StartNode)
							{
							BreakOut = true;
							break;
							} // if
						} // for
					} // else
				if (BreakOut)
					{
					NewCurNode->FlagSet(NewDirection);
					// Add any "right side" polygons to the inside list
					for (RightSideLoop = BestNode->RightSideList; RightSideLoop; RightSideLoop = RightSideLoop->NextPolygonList)
						{
						if (! RightSideLoop->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_RIGHTSHARE))
							{
							PolygonFound = false;
							for (PolyCheck = 0; PolyCheck < PolygonsUsed; ++PolyCheck)
								{
								if (OutputPolygons[PolyCheck] == RightSideLoop->MyPolygon)
									{
									PolygonFound = true;
									break;
									} // if
								} // for
							if (! PolygonFound)
								{
								OutputPolygons[PolygonsUsed ++] = RightSideLoop->MyPolygon;
								} // if
							} // if
						} // for
					} // if
				} // if
			else
				{
				// Bad news - something wrong
				Success = false;
				BreakOut = true;
				} // else
			} // while
		} // if
	} // if

OutputNodes[NodesUsed] = NULL;
NodeOwners[NodesUsed] = NULL;
OutputPolygons[PolygonsUsed] = NULL;

return (Success);

} // EffectEval::CreateReverseNodeLoop

/*===========================================================================*/

VectorPolygon *EffectEval::CreateNewPolygonAndList(VectorPolygonListDouble **&VPListPtr, VectorNode **InputNodes, 
	VectorPolygon **NodeOwners, unsigned long NodeListElements, VectorPolygon **InputPolygons)
{
bool Success = true;
unsigned long NodeListMarker = 0, NodeCt;
VectorPolygon *MadePoly = NULL, *PertinentPoly;
VectorPart *CurPart;
VectorNode **NewNodePtr, *NewNodes = NULL, *MadeNodes = NULL, *PreviousNode = NULL, *CurNode, *CurNewNode;
VectorNodeLink *CurLinkedNode;//, *CurCrossLinkedNode;

// create a new VectorPolygon and add it to NewVPList
// need to know the Joe and Effects to add, it could be the list from one or both the original polygons
if (*VPListPtr = new VectorPolygonListDouble())
	{
	NewNodePtr = &NewNodes;
	for (CurNode = InputNodes[NodeListMarker]; CurNode; CurNode = InputNodes[++NodeListMarker])
		{
		if (*NewNodePtr = new VectorNode(CurNode))
			{
			PreviousNode = *NewNodePtr;
			// need to copy the links and any appropriate flags
			NewNodePtr = &(*NewNodePtr)->NextNode;
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // for
	MadeNodes = NewNodes;
	if (PreviousNode)
		PreviousNode->NextNode = NewNodes;
	if (Success)
		{
		// Null the pointer to the new nodes so they don't get deleted on exit
		MadeNodes = NULL;
		// if an error during polygon creation the node list will be destroyed by the constructor
		if (MadePoly = (*VPListPtr)->MakeVectorPolygon(InputPolygons[0], NewNodes, false))
			{
			for (unsigned long PolyCt = 1; InputPolygons[PolyCt] && Success; ++PolyCt)
				{
				if (InputPolygons[PolyCt]->MyEffects)
					{
					Success = MadePoly->SupplementEffects(InputPolygons[PolyCt]);
					} // if
				} // if
			if (Success)
				{
				(*VPListPtr)->CalculateArea();
				// create links with all the nodes the original node was linked to
				// and any other nodes in the polygon list that the original's linked nodes were linked to
				NodeListMarker = 0;
				for (CurNode = InputNodes[NodeListMarker], CurPart = MadePoly->GetFirstPart(); CurNode && CurPart && Success; CurPart = CurPart->NextPart)
					{
					for (NodeCt = 0, CurNewNode = CurPart->FirstNode(); CurNode && NodeCt < CurPart->NumNodes && Success; CurNode = InputNodes[++NodeListMarker], CurNewNode = CurNewNode->NextNode, ++NodeCt)
						{
						PertinentPoly = NodeOwners[NodeListMarker];
						if (! (Success = CurNewNode->AddCrossLinks(CurNode, MadePoly, PertinentPoly)))
							break;
						#ifdef MAKENOCLONES
						if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT))
							CurNewNode->FlagSet(WCS_VECTORNODE_FLAG_INTERSECT_SHAREDVERT);
						#endif // MAKENOCLONES
						for (CurLinkedNode = CurNode->LinkedNodes; CurLinkedNode && Success; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
							{
							// create a link to the linked node
							if (CurNewNode != CurLinkedNode->MyNode)
								{
								if (! CurNewNode->AddCrossLinks(CurLinkedNode->MyNode, MadePoly, CurLinkedNode->LinkedPolygon))
									{
									Success = false;
									break;
									} // if
								/* I think this is unnecessary - GRH 10/25/06
								// see if there are any links back to either VP1 or VP2
								for (CurCrossLinkedNode = CurLinkedNode->MyNode->LinkedNodes; CurCrossLinkedNode; CurCrossLinkedNode = (VectorNodeLink *)CurCrossLinkedNode->NextNodeList)
									{
									if ((CurCrossLinkedNode->LinkedPolygon == VP1 || CurCrossLinkedNode->LinkedPolygon == VP2)
										&& (CurNewNode != CurCrossLinkedNode->MyNode))
										{
										if (! CurNewNode->AddCrossLinks(CurCrossLinkedNode->MyNode, MadePoly, CurCrossLinkedNode->LinkedPolygon))
											{
											Success = false;
											break;
											} // if
										} // if
									} // for
								*/
								} // if
							} // for
						} // for
					} // for
				} // if
			} // if
		else
			Success = false;
		} // if
	if (Success)
		VPListPtr = (VectorPolygonListDouble **)&(*VPListPtr)->NextPolygonList;
	} // if
else
	Success = false;

if (! Success)
	{
	if (*VPListPtr)
		{
		(*VPListPtr)->DeletePolygon();
		delete (*VPListPtr);
		*VPListPtr = NULL;
		} // if
	for (CurNewNode = MadeNodes; MadeNodes; CurNewNode = MadeNodes)
		{
		MadeNodes = MadeNodes->NextNode;
		delete CurNewNode;
		} // for
	MadePoly = NULL;
	} // if

return (MadePoly);

} // EffectEval::CreateNewPolygonAndList

#define WCS_VECTORPOLYGON_AREA_TOLERANCE	(1.0E-22)
/*===========================================================================*/
// Intersect all the polygons with each other to create a set that has no overlap.
// Input to this function is a list of polygons with all shared nodes cross-linked.
// All intersections have been identified and nodes added there on both intersecting segments.
bool EffectEval::MergePolygons(VectorPolygonListDouble *&ListToMerge, double AreaLimit, GeneralEffect *RequiredEffect,
	PolygonBoundingBox *TestBounds, bool SelfIntersect, bool PostProgress)
{
double AreaCreated = 0.0;
unsigned long NodeCt, NewNodeCt, NodesUsed, PolygonsUsed, PolyCt, PolyLoopCt;
VectorPolygonListDouble *CurList, *PrevList, *LastList, **VPListPtr, **VPListPtrStash;
VectorPolygon *CurPoly, *MadePoly;
VectorPart *CurPart, *TestPart, *NewPart;
VectorNode *CurNode, *CurNewNode, *SourceNode;
VectorNodeLink *CurLinkedNode;
bool Success = true, AreaBreakOut, SetToBreak, DestroyEntry, NodesTested, PolyIsPure, MakeIt, EnsureCompliance, 
	UseThisNode, MissingRequiredEffect;
PolygonBoundingBox InsideBounds, OutsideBounds;

if (TestBounds)
	{
	InsideBounds.Copy(TestBounds);
	OutsideBounds.Copy(TestBounds);
	InsideBounds.MaxX -= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	InsideBounds.MaxY -= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	InsideBounds.MinX += WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	InsideBounds.MinY += WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	OutsideBounds.MaxX += WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	OutsideBounds.MaxY += WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	OutsideBounds.MinX -= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	OutsideBounds.MinY -= WCS_VECTORPOLYGON_NODECOORD_TOLERANCE;
	} // if
	
// Need to identify shared segments and flag duplicate segments so that only one segment is active
// during merging.
// Need to identify all the polygons on the "right" side of a forward-pointing polygon segment.
// Some segments will point in both directions and have a polygon list associated with each side.

// no need to merge if only one polygon unless it is intentional
if (ListToMerge && (ListToMerge->NextPolygonList || SelfIntersect))
	{
	// Polygons need numbers so they can be referenced during the shared edge removal process
	PrepareToMerge(ListToMerge);
	DisableSharedEdges(ListToMerge, PostProgress);

	#ifdef DEBUG_POLYGONS_TO_VECTOR3
	// total debug output
	
	if (PrintToVector)
		{
		OutputVectorsToDatabase(ListToMerge, "Merge input", NULL);
		//return (false);
		} // if
		
	#endif // DEBUG_POLYGONS_TO_VECTOR3

	if (PostProgress)
		{
		if (! UpdateGauge(0))
			return (false);
		} // if
		
	// area calculation like everything else is imprecise. A small error can result in a lot of extra
	// polygon calculation trying to find the last piece of the DEM cell when in fact it is just a math error
	// that prevents the area breakout form occurring. The tolerance value was found by trial and error.
	// Too large a tolerance leaves holes in the landscape.
	if (AreaLimit > 0.0)
		AreaLimit -= AreaLimit * 1.0e-5;	//WCS_VECTORPOLYGON_AREA_TOLERANCE;
	EnsureCompliance = false;
	UseThisNode = true;
		
	// Walk the nodes of each polygon looking for nodes that have an unused forward segment
	// Walk node by node looking for unused segments that form the smallest right turn, 
	// including all linked nodes.
	// Mark segments as they are used. Use backwards segments if they are the lesser angle.
	// Loops are complete when one of the linked nodes is the start node
	// Polygons enclosed are the polygons that the nodes and linked nodes belong to if it is a forward segment
	// and the right side polygons if it is a forward segment. Backward segments do not contribute anything to
	// the enclosed polygon list.
	// Walk each part of each polygon looking for unused forward segments to start a loop

	// First allocate enough space for nodes and polygons
	if (MergeOutputNodes || (MergeOutputNodes = (VectorNode **)AppMem_Alloc(MergeNumNodesAllocated * sizeof (VectorNode *), 0)))
		{
		if (MergeNodeOwners || (MergeNodeOwners = (VectorPolygon **)AppMem_Alloc(MergeNumNodesAllocated * sizeof (VectorPolygon *), 0)))
			{
			if (! MergeOutputPolygons)
				MergeOutputPolygons = (VectorPolygon **)AppMem_Alloc(MergeNumPolysAllocated * sizeof (VectorPolygon *), 0);
			} // if
		} // if


	if (MergeOutputNodes && MergeNodeOwners && MergeOutputPolygons)
		{
		// walk to end of list and attach new items there
		for (CurList = ListToMerge; CurList->NextPolygonList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
			{
			} // for
		LastList = CurList;
		VPListPtr = (VectorPolygonListDouble **)&CurList->NextPolygonList;

		// walk through all the nodes looking for segments that are not yet used to start a new node loop
		SetToBreak = AreaBreakOut = false;
		//for (CurList = ListToMerge; CurList && Success && ! SetToBreak && ! AreaBreakOut; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: ListToMerge)
		for (PolyLoopCt = 0, CurList = ListToMerge; CurList && Success && ! SetToBreak && ! AreaBreakOut; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
			{
			if (CurList == LastList)
				SetToBreak = true;
			CurPoly = CurList->GetPolygon();
			DestroyEntry = false;
			NodesTested = false;
			MissingRequiredEffect = (RequiredEffect && ! CurPoly->TestForEffect(RequiredEffect));
			
			// The following comment proved to be wrong in cases where a polygon cutout limned the inside of
			// a circular linear terraffector. Each part needs to be tested. This change was made on the
			// 8th day of February in the year 2008 Anno Domini. Pax vobiscum. 2/8/08
			// Bogus: Only use the first part of the polygon to look for unprocessed segments.
			// Inside parts will be picked up either by intersecting them or by the process of discovering
			// interior parts below.
			//CurPart = CurPoly->GetFirstPart();
			// the following Part loop was added 2/8/08
			for (CurPart = CurPoly->GetFirstPart(); CurPart; CurPart = CurPart->NextPart)
				{
				PolyIsPure = false;
				for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes && Success && ! AreaBreakOut && ! PolyIsPure; ++NodeCt, CurNode = CurNode->NextNode)
					{
					if (TestBounds)
						{
						EnsureCompliance = false;
						UseThisNode = true;
						if (! OutsideBounds.TestPointInBox(CurNode))
							UseThisNode = false;
						else if (MissingRequiredEffect || ! InsideBounds.TestPointInBox(CurNode))
							EnsureCompliance = true;
						} // if
					// F2_NOTE: vvv CPI 10+
					if (UseThisNode && ! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_FORWARDUSED))
						{
						NodesTested = PolyIsPure = true;
						// CreateNodeLoop either makes a loop that includes all the nodes in the exact order they
						// already exist in a polygon or it pieces together a pathway through any number of nodes from any number of different
						// polygons and a new polygon needs to be made of that pathway, preserving node linkages to either already 
						// existing polygons or to newly formed polygons.
						NodesUsed = PolygonsUsed = 0;
						if (Success = CreateNodeLoop(CurPoly, CurNode, MergeOutputNodes, MergeNodeOwners,
							MergeOutputPolygons, NodesUsed, PolygonsUsed, MergeNumNodesAllocated, MergeNumPolysAllocated,
							EnsureCompliance ? &OutsideBounds: NULL, PolyIsPure))
							{
							if (! PolygonsUsed)
								MakeIt = false;
							else if (RequiredEffect)
								{
								MakeIt = false;
								for (PolyCt = 0; MergeOutputPolygons[PolyCt]; ++PolyCt)
									{
									if (MergeOutputPolygons[PolyCt]->TestForEffect(RequiredEffect))
										{
										MakeIt = true;
										break;
										} // if
									} // if
								} // if
							else
								MakeIt = true;
							if (MakeIt)
								{
								if (! PolyIsPure || NodesUsed < CurPart->NumNodes || CurPoly->ImAnOriginalPolygon)
									{
									// create a new polygon cloned from the used nodes
									// new nodes should link to all the nodes that the old nodes link to.
									// If those nodes and their polygons are later deleted, the links will be dissolved
									// along with them, causing no more damage than the allocation and deallocation
									VPListPtrStash = VPListPtr;
									if (MadePoly = CreateNewPolygonAndList(VPListPtr, MergeOutputNodes, 
										MergeNodeOwners, NodesUsed, MergeOutputPolygons))
										{
										if ((*VPListPtrStash)->DoubleVal > 0.0)
											{
											if (AreaLimit > 0.0)
												{
												AreaCreated += (*VPListPtrStash)->DoubleVal;
												if (AreaCreated >= AreaLimit)
													AreaBreakOut = true;
												} // if
											DestroyEntry = true;
											for (PolyCt = 0; MergeOutputPolygons[PolyCt] && Success; ++PolyCt)
												{
												for (TestPart = MergeOutputPolygons[PolyCt]->GetFirstPart()->NextPart; TestPart; TestPart = TestPart->NextPart)
													{
													if (MadePoly->PolygonEnclosesPart(TestPart, MergeOutputPolygons[PolyCt]))
														{
														// since there are inside polygons here the area created won't just add up
														// so prevent breaking out based on area
														AreaBreakOut = false;
														AreaLimit = 0.0;
														// add a copy of the part to the newly created polygon.
														// The part is already negative so it doesn't need to be reversed.
														if (NewPart = MadePoly->ClonePart(TestPart))
															{
															// cross link the nodes of the copy to all the linked nodes of the original
															for (NewNodeCt = 0, CurNewNode = NewPart->FirstNode(), SourceNode = TestPart->FirstNode(); NewNodeCt < NewPart->NumNodes && Success; ++NewNodeCt, CurNewNode = CurNewNode->NextNode, SourceNode = SourceNode->NextNode)
																{
																if (! (Success = CurNewNode->AddCrossLinks(SourceNode, MadePoly, MergeOutputPolygons[PolyCt])))
																	break;
																for (CurLinkedNode = SourceNode->LinkedNodes; CurLinkedNode && Success; CurLinkedNode = (VectorNodeLink *)CurLinkedNode->NextNodeList)
																	{
																	// create a link to the linked node
																	if (CurNewNode != CurLinkedNode->MyNode)
																		{
																		if (! CurNewNode->AddCrossLinks(CurLinkedNode->MyNode, MadePoly, CurLinkedNode->LinkedPolygon))
																			{
																			Success = false;
																			break;
																			} // if
																		} // if
																	} // for
																} // for
															} // if NewPart
														else
															Success = false;
														} // if
													} // for TestPart
												} // for PolyCt
											} // if positive area
										else
											{
											// negative polygon should not be kept
											VPListPtr = VPListPtrStash;
											(*VPListPtr)->DeletePolygon();
											delete (*VPListPtr);
											*VPListPtr = NULL;
											} // else
										} // if MadePoly
									else // MadePoly failed
										Success = false;
									} // if ! PolyIsPure
								else
									{
									if (AreaLimit > 0.0)
										{
										AreaCreated += CurList->DoubleVal > 0.0 ? CurList->DoubleVal: CurList->CalculateArea();
										if (AreaCreated >= AreaLimit)
											AreaBreakOut = true;
										} // if
									if (PolygonsUsed > 1)
										{
										// add the extra polygons' effects to the existing polygon
										for (PolyCt = 1; MergeOutputPolygons[PolyCt] && Success; ++PolyCt)
											{
											if (MergeOutputPolygons[PolyCt]->MyEffects)
												{
												Success = CurPoly->SupplementEffects(MergeOutputPolygons[PolyCt]);
												} // if
											} // if
										} // if
									} // else
								} // if
							} // if
						#ifdef _DEBUG
						else if (NodesUsed > 0)
							{
							char ErrorStr[256];
							sprintf(ErrorStr, "%s   %s", CurPoly->MyEffects ? CurPoly->MyEffects->MyEffect->Name: "Unknown", CurPoly->MyEffects ? CurPoly->MyEffects->MyJoe->GetBestName(): "Unknown");
							UserMessageOK("MergePolygons Failure", ErrorStr);
							OutputFailedListOfNodesToDatabase(MergeOutputNodes, NodesUsed);
							#if defined DEBUG_POLYGONS_TO_VECTOR1 || defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3 || defined DEBUG_POLYGONS_TO_VECTOR4
							OutputVectorsToDatabase(ListToMerge, "Failed merge output", NULL);
							#endif // DEBUG_POLYGONS_TO_VECTOR1, etc
							} // else
						#endif // _DEBUG
						} // if
					} // for
				} // for
			if (DestroyEntry || ! NodesTested)
				{
				// mark the polygon to be deleted later, it can't be deleted here because its
				// nodes may still be needed for other merges
				CurList->FlagSet(WCS_VECTORPOLYGONLIST_FLAG_DESTROY);
				} // if
			if (PostProgress)
				{
				if (! UpdateGauge(++PolyLoopCt))
					Success = false;
				} // if
			#if defined DEBUG_POLYGONS_TO_VECTOR1 || defined DEBUG_POLYGONS_TO_VECTOR2 || defined DEBUG_POLYGONS_TO_VECTOR3 || defined DEBUG_POLYGONS_TO_VECTOR4
			else
				++PolyLoopCt;
			#endif // DEBUG_POLYGONS_TO_VECTOR1, etc
			} // for
		if (AreaBreakOut && ! SetToBreak)
			{
			// continue where left off in last loop setting each remaining original polygon to be destroyed
			// since they apparently weren't needed to create the full area
			for (; CurList && ! SetToBreak; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (CurList == LastList)
					SetToBreak = true;
				CurList->FlagSet(WCS_VECTORPOLYGONLIST_FLAG_DESTROY);
				} // if
			} // if
			
		if (Success && ! AreaBreakOut)
			{
			SetToBreak = false;
			for (CurList = ListToMerge; CurList && Success && ! SetToBreak && ! AreaBreakOut; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
				{
				if (CurList == LastList)
					SetToBreak = true;
				CurPoly = CurList->GetPolygon();
				
				// only use the first part of the polygon to look for unprocessed segments.
				// Inside parts will be picked up either by intersecting them or by the process of discovering
				// interior parts below.
				CurPart = CurPoly->GetFirstPart();
				for (NodeCt = 0, CurNode = CurPart->FirstNode(); NodeCt < CurPart->NumNodes && Success && ! AreaBreakOut; ++NodeCt, CurNode = CurNode->NextNode)
					{
					if (TestBounds)
						{
						EnsureCompliance = false;
						UseThisNode = true;
						if (! OutsideBounds.TestPointInBox(CurNode))
							UseThisNode = false;
						else if (! InsideBounds.TestPointInBox(CurNode))
							EnsureCompliance = true;
						} // if
					// F2_NOTE: vvv CPI = 10+
					if (UseThisNode && ! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED))
						{
						// make polygon completely going backwards since all forward links are already used
						// make an included polygon list but do not include the actual polygons that the nodes are drawn from
						// but include the right side members of the nodes at the leading (backwards) end of each segment.
						// if there are no polygons included then it is the outside polygon or an open inside polygon
						// and no polygon should be made of the nodes.
						NodesUsed = PolygonsUsed = 0;
						if (CreateReverseNodeLoop(CurPoly, CurNode, MergeOutputNodes, MergeNodeOwners,
							MergeOutputPolygons, NodesUsed, PolygonsUsed, MergeNumNodesAllocated, MergeNumPolysAllocated, 
							EnsureCompliance ? &OutsideBounds: NULL))
							{
							if (! PolygonsUsed && NodesUsed && SelfIntersect)
								{
								MergeOutputPolygons[0] = CurPoly;
								PolygonsUsed = 1;
								} // if
							if (PolygonsUsed)
								{
								// create a new polygon cloned from the used nodes
								// new nodes should link to all the nodes that the old nodes link to.
								// If those nodes and their polygons are later deleted, the links will be dissolved
								// along with them, causing no more damage than the allocation and deallocation
								VPListPtrStash = VPListPtr;
								if (MadePoly = CreateNewPolygonAndList(VPListPtr, MergeOutputNodes, 
									MergeNodeOwners, NodesUsed, MergeOutputPolygons))
									{
									if ((*VPListPtrStash)->DoubleVal > 0.0)
										{
										if (AreaLimit > 0.0)
											{
											AreaCreated += (*VPListPtrStash)->DoubleVal;
											if (AreaCreated >= AreaLimit)
												AreaBreakOut = true;
											} // if
										} // if
									else
										{
										// negative polygon should not be kept
										VPListPtr = VPListPtrStash;
										(*VPListPtr)->DeletePolygon();
										delete (*VPListPtr);
										*VPListPtr = NULL;
										if (SelfIntersect)
											{
											// Weird things happen when trying to resolve self-intersection and it might
											// be that a loop can be formed with a positive area if you start on a different node.
											// So that nodes can be used again, set any that were used in this loop back to unused 
											// in the backward direction.
											for (NewNodeCt = 0; NewNodeCt < NodesUsed; ++NewNodeCt)
												{
												MergeOutputNodes[NewNodeCt]->FlagClear(WCS_VECTORNODE_FLAG_INTERSECT_BACKWARDUSED);
												} // for
											} // if
										} // else
									} // if
								} // if
							} // if
						} // if
					} // for
				} // for
			} // if
		// delete the polygons that have been replaced
		PrevList = NULL;
		SetToBreak = false;
		for (CurList = ListToMerge; CurList && ! SetToBreak; CurList = CurList ? (VectorPolygonListDouble *)CurList->NextPolygonList: ListToMerge)
			{
			#ifdef MAKENOCLONES
			// return this pointer to NULL so it doesn't confuse later mergers
			if (CurList->MyPolygon->CloneOfThis == (VectorPolygon *)~0)
				CurList->MyPolygon->SetClonePointer(NULL);
			#endif // MAKENOCLONES
			if (CurList == LastList)
				SetToBreak = true;
			if (CurList->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_DESTROY))
				{
				if (! RequiredEffect || CurList->MyPolygon->TestForEffect(RequiredEffect))
					{
					if (PrevList)
						PrevList->NextPolygonList = (VectorPolygonListDouble *)CurList->NextPolygonList;
					else
						ListToMerge = (VectorPolygonListDouble *)CurList->NextPolygonList;
					if (! CurList->MyPolygon->ImAnOriginalPolygon)
						CurList->DeletePolygon();
					else
						{
						CurList->RemoveRightSideLists();
						if (RequiredEffect)
							{
							CurList->MyPolygon->RemoveTerrainEffectLinks(RequiredEffect);
							CurList->MyPolygon->RemoveEffect(RequiredEffect);
							} // if
						} // if
					delete CurList;
					CurList = PrevList;
					} // if
				else
					{
					CurList->RemoveRightSideLists();
					#ifdef MAKENOCLONES
					CurList->FlagClear(WCS_VECTORPOLYGONLIST_FLAG_DESTROY);
					CurList->MyPolygon->RemoveTerrainEffectLinks(RequiredEffect);
					#endif // MAKENOCLONES
					} // else
				} // if
			else
				{
				CurList->RemoveRightSideLists();
				} // else
			PrevList = CurList;
			} // for
		} // if
	else
		Success = false;

	} // if

#ifdef DEBUG_POLYGONS_TO_VECTOR2
if (PrintToVector)
	{
	OutputVectorsToDatabase(ListToMerge, "Merge output", NULL);
	return (false);
	} // if
#endif // DEBUG_POLYGONS_TO_VECTOR2

return (Success);

} // EffectEval::MergePolygons

/*===========================================================================*/

RenderJoeList *EffectEval::SortByDiminishingArea(RenderJoeList *ListToSort)
{

for (RenderJoeList *CurJL = ListToSort; CurJL; CurJL = (RenderJoeList *)CurJL->Next)
	{
	if (CurJL->GetVector())
		{
		CurJL->Area = CurJL->GetVector()->ComputeAreaDegrees();
		} // if
	} // for

return (ListToSort->SortByAbsArea(WCS_JOELIST_SORTORDER_HILO));

} // EffectEval::SortByDiminishingArea

/*===========================================================================*/

VectorPolygonListDouble *EffectEval::MatchListJoe(VectorPolygonListDouble *MatchList, Joe *MatchVector)
{

for (VectorPolygonListDouble *CurPoly = MatchList; CurPoly; CurPoly = (VectorPolygonListDouble *)CurPoly->NextPolygonList)
	{
	if (CurPoly->GetFirstVector() == MatchVector)
		return (CurPoly);
	} // for

return (NULL);

} // EffectEval::MatchListJoe

/*===========================================================================*/

bool EffectEval::RemoveHoles(VectorPolygon *HoleyCow, RenderJoeList *RJLCur)
{
bool Success = true;
GeneralEffect *MatchEffect;
Joe *OutsideVector;
VectorPolygonListDouble *ListOfHoles = NULL, **CurHoleListPtr;

CurHoleListPtr = &ListOfHoles;

MatchEffect = RJLCur->GetEffect();
OutsideVector = RJLCur->GetVector();

if (LayerEntry *MultiPartLayer = RJLCur->GetVector()->GetMultiPartLayer())
	{
	// loop through layer stubs
	for (LayerStub *MultiPartStub = MultiPartLayer->FirstStub(); MultiPartStub && Success; MultiPartStub = MultiPartStub->NextObjectInLayer())
		{
		Joe *MatchVector = MultiPartStub->MyObject();
		// find the RJL that corresponds by walking the joe list
		for (RenderJoeList *MatchJL = (RenderJoeList *)RJLCur->Next; MatchJL; MatchJL = (RenderJoeList *)MatchJL->Next)
			{
			// only test negative vectors
			if (MatchJL->Area < 0.0)
				{
				// both Joe and Effect need to match
				if (MatchJL->GetVector() == MatchVector && MatchJL->GetEffect() == MatchEffect)
					{
					// if this vector is in same overall region add it to list
					if (OutsideVector->IsJoeContainedInMyGeoBounds(MatchVector))
						{
						if (*CurHoleListPtr = new VectorPolygonListDouble())
							{
							if ((*CurHoleListPtr)->MakeVectorPolygon(DBHost, MatchEffect, MatchVector))
								CurHoleListPtr = (VectorPolygonListDouble **)&(*CurHoleListPtr)->NextPolygonList;
							else
								Success = false;
							} // if
						else
							Success = false;
						} // if
					break;
					} // if
				} // if
			} // for
		} // for
	if (Success && ListOfHoles)
		{
		Success = HoleyCow->RemoveHoles(ListOfHoles);
		} // if
	if (ListOfHoles)
		{
		// if the list persisted through the hole removal process then something probably went wrong.
		// remove the list.
		for (VectorPolygonListDouble *CurHoleList = ListOfHoles; ListOfHoles; CurHoleList = ListOfHoles)
			{
			ListOfHoles = (VectorPolygonListDouble *)ListOfHoles->NextPolygonList;
			CurHoleList->DeletePolygon();
			delete CurHoleList;
			} // for
		} // if
	} // if

return (Success);

} // EffectEval::RemoveHoles

/*===========================================================================*/

bool EffectEval::NormalizeDirection(VectorPolygonListDouble *NormalizeMe)
{
bool Success = true;

// Areas are negative if counter-clockwise.
// If the area is positive then it is clockwise already
// Reverse the negative ones and change the area sign.
if (NormalizeMe->DoubleVal < 0.0)
	{
	Success = NormalizeMe->GetPolygon()->ReverseDirection();
	NormalizeMe->DoubleVal = -NormalizeMe->DoubleVal;
	} // if
	
return (Success);
	
} // EffectEval::NormalizeDirection

/*===========================================================================*/

VectorPolygonListDouble *EffectEval::CreateBoundedPolygonList(VectorPolygonListDouble *ListToTest, 
	PolygonBoundingBox *PolyBounds, bool &Success)
{
VectorPolygonListDouble *ListProgress, **NewListPtr, *NewList = NULL;
ENUM_BOUNDING_BOX_CONTAINED Result;

Success = true;

// This function makes a linked list of VectorPolygons that overlap with the bounding box provided.

// test each polygon
NewListPtr = &NewList;
for (ListProgress = ListToTest ? ListToTest: PolygonList; ListProgress; ListProgress = (VectorPolygonListDouble *)ListProgress->NextPolygonList)
	{
	Result = ListProgress->MyPolygon->TestBoxContained(PolyBounds);
	if (Result == WCS_BOUNDING_BOX_CONTAINED_INSIDE || Result == WCS_BOUNDING_BOX_CONTAINED_EDGE ||
		Result == WCS_BOUNDING_BOX_CONTAINED_ENCLOSED)
		{
		if (*NewListPtr = new VectorPolygonListDouble())
			{
			(*NewListPtr)->MyPolygon = ListProgress->MyPolygon;
			(*NewListPtr)->DoubleVal = ListProgress->DoubleVal;
			(*NewListPtr)->FlagSet(Result == WCS_BOUNDING_BOX_CONTAINED_INSIDE ? (unsigned char)WCS_VECTORPOLYGONLIST_FLAG_INSIDE:
				Result == WCS_BOUNDING_BOX_CONTAINED_EDGE ? (unsigned char)WCS_VECTORPOLYGONLIST_FLAG_EDGE:
				(unsigned char)WCS_VECTORPOLYGONLIST_FLAG_ENCLOSED);
			NewListPtr = (VectorPolygonListDouble **)&(*NewListPtr)->NextPolygonList;
			} // if
		else
			{
			Success = false;
			break;
			} // else
		} // if
	} // for

if (! Success)
	{
	VectorPolygonListDouble *Temp;
	// delete any polygons created and return NULL;
	for (Temp = NewList; Temp; Temp = NewList)
		{
		NewList = (VectorPolygonListDouble *)NewList->NextPolygonList;
		delete Temp;
		} // for
	} // if

return (NewList);

} // EffectEval::CreateBoundedPolygonList

/*===========================================================================*/

// Makes a new vector polygon surrounding the original vector or a supplied vector polygon
// Do not use this for terraffector vectors that are in point mode. They need a diferent function that
// just outlines the points.
VectorPolygonListDouble *EffectEval::MakeBoundingVectorPolygons(GeneralEffect *NewEffect, Joe *NewVector,
	VectorPolygon *PolyLiner, double MetersPerDegLat, bool &Success)
{
double CurAngle, NextAngle, TestAngle, CurMetersPerDegLon, NextMetersPerDegLon, CurInvMetersPerDegLon, 
	NextInvMetersPerDegLon, InvMetersPerDegLat, CompDistance, AngularDev;
unsigned long NodeCt, NodeCtMod, NumSegmentNodes, CurSegCt, CurrentPointIndex, NextPointIndex;
unsigned long *VectorPointIndexArray = NULL;
double *DistanceArray = NULL;
short *SegNumberArray = NULL;
char *CurNodesFlagArray = NULL, *NextNodesFlagArray = NULL, *NewNodeFlagArray;
VectorPolygon *NewPoly = NULL, *TempPolyFwd = NULL, *TempPolyReverse = NULL;
VectorNode *NodeStash, *LastNodeStash, *NewNodes[4], **CurNodesArray = NULL, **NextNodesArray = NULL, **NewNodeArray;
VectorPolygonListDouble *NewPolyList = NULL, **NewPolyListPtr;
bool ConnectBack, NewOrthogonal, ThreePointPoly, SkipIt, Outbound, InsufficientNodes = false;
VertexBase VBaseCurPt, VBaseNextPt, VBaseNextNextPt;
VectorNode TempNode;
VectorIntersecter VI;

Success = true;

NewPolyListPtr = &NewPolyList;

if ((PolyLiner && PolyLiner->TotalNumNodes < 3) || NewVector->GetNumRealPoints() < 2)
	Success = false;
	
if (Success && ! PolyLiner)
	{
	if (((NewEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR && 
		((TerraffectorEffect *)NewEffect)->Splined) || 
		(NewEffect->EffectType == WCS_EFFECTSSUBCLASS_STREAM && 
		((StreamEffect *)NewEffect)->Splined)) && NewVector->GetNumRealPoints() > 2)
		{
		AngularDev = 2.0;
		if (! (NewVector = NewVector->AddJoeSpline(DBHost, AngularDev)))
			Success = false;
		} // if
	// NumPtsIgnore = true
	if (Success && (TempPolyFwd = new VectorPolygon(DBHost, NewEffect, NewVector, true, InsufficientNodes)))
		{
		if (TempPolyFwd->TotalNumNodes == 1)
			{
			delete TempPolyFwd;
			return (MakeBoundingVectorPointPolygons(NewEffect, NewVector, MetersPerDegLat, Success));
			} // if
		if (Success = TempPolyFwd->ConvertToDefGeo())
			{
			if (TempPolyFwd->TotalNumNodes > 2)
				{
				// create a return path along the vector by first cloning the forward polygon
				if (TempPolyReverse = new VectorPolygon(TempPolyFwd))
					{
					// find last node
					assert(TempPolyReverse->TotalNumNodes != 0);
					for (NodeCt = 0, NodeStash = TempPolyReverse->PolyFirstNode(); NodeCt < TempPolyReverse->TotalNumNodes - 1; NodeStash = NodeStash->NextNode, ++NodeCt)
						{};
						
					// now we know the geographic coords of the first and last points on the vecotr - we'll need these later
					// to expedite polygon intersection
					NewVector->SetFirstLastCoords(TempPolyReverse->PolyFirstNode()->Lat, TempPolyReverse->PolyFirstNode()->Lon, NodeStash->Lat, NodeStash->Lon);
					NodeStash->NextNode = TempPolyReverse->PolySecondNode();
					// remove first node
					NodeStash = TempPolyReverse->PolyFirstNode();
					TempPolyReverse->PolySetFirstNode(TempPolyReverse->PolySecondNode());
					--TempPolyReverse->TotalNumNodes;
					--TempPolyReverse->FirstPart.NumNodes;
					delete NodeStash;
					// stash the first node
					LastNodeStash = TempPolyReverse->PolyFirstNode();
					// ReverseDirection does not require the last node to be relinked with the new first node
					if (TempPolyReverse->PolyFirstNode())
						{
						if (TempPolyReverse->ReverseDirection())
							{
							// advance the first node so that the linear direction is completely reversed
							TempPolyReverse->PolySetFirstNode(TempPolyReverse->PolySecondNode());
							// remove first node
							NodeStash = TempPolyReverse->PolyFirstNode();
							TempPolyReverse->PolySetFirstNode(TempPolyReverse->PolySecondNode());
							--TempPolyReverse->TotalNumNodes;
							--TempPolyReverse->FirstPart.NumNodes;
							delete NodeStash;

							if (TempPolyReverse->PolyFirstNode())
								{
								// walk nodes to last one of forward polygon and connect with reverse polygon
								for (NodeCt = 0, NodeStash = TempPolyFwd->PolyFirstNode(); NodeCt < TempPolyFwd->TotalNumNodes - 1; NodeStash = NodeStash->NextNode, ++NodeCt)
									{};
								NodeStash->NextNode = TempPolyReverse->PolyFirstNode();
								// link last node to first
								LastNodeStash->NextNode = TempPolyFwd->PolyFirstNode();
								// adjust number of nodes
								TempPolyFwd->TotalNumNodes += TempPolyReverse->TotalNumNodes;
								TempPolyFwd->FirstPart.NumNodes += TempPolyReverse->TotalNumNodes;
								// Null nodes and delete reverse polygon
								TempPolyReverse->PolySetFirstNode(NULL);
								TempPolyReverse->TotalNumNodes = 0;
								TempPolyReverse->FirstPart.NumNodes = 0;
								delete TempPolyReverse;
								TempPolyReverse = NULL;
								} // if
							} // if
						else
							Success = false;
						} // if
					} // if
				else
					Success = false;
				} // if
			PolyLiner = TempPolyFwd;
			} // if
		} // if
	else
		Success = false;
	} // if

if (Success)
	{
	// Build a list of vector vertex pointers and a list of vector vertex indices to use
	// in distance calculations
	// Each node in the polygon gets an entry in the arrays
	if (VectorPointIndexArray = new unsigned long[PolyLiner->TotalNumNodes])
		{
		VectorPoint *PLink;
		// fill in values
		for (PLink = NewVector->GetFirstRealPoint(), NodeCt = 0; PLink && PLink->Next && NodeCt < TempPolyFwd->TotalNumNodes; ++NodeCt, PLink = PLink->Next)
			{
			VectorPointIndexArray[NodeCt] = NodeCt;
			} // for
		for (unsigned long BackwardCt = NodeCt - 1; NodeCt < TempPolyFwd->TotalNumNodes; ++NodeCt, --BackwardCt)
			{
			VectorPointIndexArray[NodeCt] = BackwardCt;
			// can't decrement unsigned long below 0 so test here for 0 condition
			if (BackwardCt == 0)
				break;
			} // for
		} // if
	else
		Success = false;
	} // if

if (Success)
	{
	// PolyLiner must now be a supplied or constructed fact
	// One point vectors aren't covered here so return failure
	if (PolyLiner->TotalNumNodes > 1)
		{
		void *PlaceHolder = NULL;
		double SegWidth;

		// allocate enough node pointers in two arrays to handle all the tfx segments
		// GetNumSegments() includes approach slope. Add a node for 0 distance
		NumSegmentNodes = NewEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR ? 
			((TerraffectorEffect *)NewEffect)->GetNumSegments() + 1: 2;

		DistanceArray = new double[NumSegmentNodes];
		CurNodesArray = new VectorNode *[NumSegmentNodes];
		NextNodesArray = new VectorNode *[NumSegmentNodes];
		CurNodesFlagArray = new char[NumSegmentNodes];
		NextNodesFlagArray = new char[NumSegmentNodes];
		SegNumberArray = new short[NumSegmentNodes];

		if (DistanceArray && CurNodesArray && NextNodesArray && CurNodesFlagArray && NextNodesFlagArray && SegNumberArray)
			{
			DistanceArray[0] = 0.0;
			SegNumberArray[0] = 0;
			if (NewEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
				{
				for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
					{
					if (! (PlaceHolder = ((TerraffectorEffect *)NewEffect)->GetNextSegmentWidth(PlaceHolder, SegWidth)))
						{
						SegWidth = ((TerraffectorEffect *)NewEffect)->GetRadiusWidth();
						SegNumberArray[CurSegCt] = WCS_TFXDETAIL_SLOPESEGMENT;
						} // if
					else
						{
						SegNumberArray[CurSegCt] = (short)(CurSegCt - 1);
						} // else
					DistanceArray[CurSegCt] = SegWidth + DistanceArray[CurSegCt - 1];
					} // for
				} // if
			else if (NewEffect->EffectType == WCS_EFFECTSSUBCLASS_STREAM)
				{
				DistanceArray[1] = ((StreamEffect *)NewEffect)->GetAnimPtr(WCS_EFFECTS_STREAM_ANIMPAR_RADIUS)->CurValue;
				SegNumberArray[1] = 0;
				} // else
			else
				{
				DistanceArray[1] = NewEffect->UseGradient ? NewEffect->GetMaxProfileDistance(): 0.0;
				SegNumberArray[1] = 0;
				} // else

			// check and assure that there is a need for the offsetting polygons
			if (DistanceArray[1] > 0.0)
				{
				InvMetersPerDegLat = 1.0 / MetersPerDegLat;
				ConnectBack = false;
				NewOrthogonal = true;

				// Find the offsets for the nodes in PolyLiner and create new nodes at the offset points.
				// Begin by making offsets from the first node orthogonal and to the left of the first segment.
				// If there are numerous segments of a tfx cross section to be polygonized, each segment 
				// including the final tfx radius should get the same number of polygons created. Except for the innermost
				// triangle polygons in the right-veering fans, all other fan polygons will have four nodes.

				// The vertex order for rectangular polygons will be:
					// current vertex, outer projection of current, outer projection of next vertex, next vertex.
				// The vertex order for triangular polygons will be:
					// current vertex, outer projection of current, next outer projection of current vertex.
				for (NodeCt = 0, NodeStash = PolyLiner->PolyFirstNode(); NodeCt <= PolyLiner->TotalNumNodes; NodeStash = NodeStash->NextNode, ++NodeCt)
					{
					// break here if we are back to the original starting segment and we don't need to link
					// the last filler polygon back to the starting orthogonal offset
					if (NodeCt == PolyLiner->TotalNumNodes && ! ConnectBack)
						break;
					NodeCtMod = NodeCt % PolyLiner->TotalNumNodes;
					Outbound = NodeCtMod < PolyLiner->TotalNumNodes / 2;
					CurrentPointIndex = VectorPointIndexArray[NodeCtMod];
					NextPointIndex = Outbound ? VectorPointIndexArray[NodeCtMod + 1]: VectorPointIndexArray[NodeCtMod - 1];
					//CurrentPointIndex = NodeCt < PolyLiner->TotalNumNodes ? VectorPointIndexArray[NodeCt]:  VectorPointIndexArray[NodeCt - PolyLiner->TotalNumNodes];
					//NextPointIndex = NodeCt < PolyLiner->TotalNumNodes - 1 ? VectorPointIndexArray[NodeCt + 1]: VectorPointIndexArray[NodeCt - PolyLiner->TotalNumNodes + 1];

					// each node gets its own latitude-specific longitude conversion factor
					CurMetersPerDegLon = MetersPerDegLat * cos(NodeStash->Lat * PiOver180);
					if (CurMetersPerDegLon > 0.0)
						CurInvMetersPerDegLon = 1.0 / CurMetersPerDegLon;
					else
						CurInvMetersPerDegLon = 0.0;
					NextMetersPerDegLon = MetersPerDegLat * cos(NodeStash->NextNode->Lat * PiOver180);
					if (NextMetersPerDegLon > 0.0)
						NextInvMetersPerDegLon = 1.0 / NextMetersPerDegLon;
					else
						NextInvMetersPerDegLon = 0.0;

					// find the angle between the current segment and the next segment measured on the outside (left side) of the polygon.
					// To do that, set up two position vectors in cartesian coords, both pointing in the forward direction, one from the 
					// current to next node and one from next to next-next node.

					// find the angle both vectors make with north
					// subtract the first angle from the second and normalize the result to be between +/- 180

					// negative result means the vector veers leftward
					// positive means it veers rightward
					// rightward veering has an extra step of creating filler polygons
					// leftward veering polygons overlap

					VBaseCurPt.XYZ[2] = NodeStash->Lat;// * MetersPerDegLat;
					VBaseCurPt.XYZ[0] = -NodeStash->Lon;// * CurMetersPerDegLon;
					VBaseNextPt.XYZ[2] = NodeStash->NextNode->Lat;// * MetersPerDegLat;
					VBaseNextPt.XYZ[0] = -NodeStash->NextNode->Lon;// * NextMetersPerDegLon;
					VBaseNextNextPt.XYZ[2] = NodeStash->NextNode->NextNode->Lat;// * MetersPerDegLat;
					VBaseNextNextPt.XYZ[0] = -NodeStash->NextNode->NextNode->Lon;// * NextNextMetersPerDegLon;
					VBaseNextNextPt.GetPosVector(&VBaseNextPt);
					VBaseNextPt.GetPosVector(&VBaseCurPt);
					VBaseNextPt.XYZ[2] *= MetersPerDegLat;
					VBaseNextPt.XYZ[0] *= CurMetersPerDegLon;
					VBaseNextNextPt.XYZ[2] *= MetersPerDegLat;
					VBaseNextNextPt.XYZ[0] *= NextMetersPerDegLon;
					CurAngle = VBaseNextPt.FindAngleYfromZ();
					NextAngle = VBaseNextNextPt.FindAngleYfromZ();
					if (NodeStash->SamePointLatLon(NodeStash->NextNode->NextNode))
						TestAngle = 180.0;
					else if ((TestAngle = NextAngle - CurAngle) > 180.0)
						TestAngle -= 360.0;
					else if (TestAngle <= -180.0)
						TestAngle += 360.0;

					VBaseNextPt.RotateY(-90.0);
					VBaseNextPt.UnitVector();

					if (NewOrthogonal)
						{
						// create a new set of nodes at right angles to the current segment
						// pick a set of nodes to create depending on whether this set must connect back to the last set
						NewNodeArray = ConnectBack ? NextNodesArray: CurNodesArray;
						NewNodeFlagArray = ConnectBack ? NextNodesFlagArray: CurNodesFlagArray;
						if (NewNodeArray[0] = new VectorNode(NodeStash))
							{
							NewNodeFlagArray[0] = 0;
							for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
								{
								TempNode.Lat = NodeStash->Lat + VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
								if (TempNode.Lat >= 90.0)
									{
									Success = false;
									break;
									} // if illegal latitude
								TempNode.Lon = NodeStash->Lon - VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * CurInvMetersPerDegLon;
								TempNode.Elev = NodeStash->Elev;
								if (! (NewNodeArray[CurSegCt] = new VectorNode(TempNode)))
									{
									Success = false;
									break;
									} // if
								NewNodeFlagArray[CurSegCt] = 0;
								} // for
							} // if
						else
							Success = false;

						if (ConnectBack)
							{
							// make a polygon out of the vertices for each segment. Be sure to replicate nodes and only use them once
							for (CurSegCt = 1; Success && CurSegCt < NumSegmentNodes; ++CurSegCt)
								{
								SkipIt = false;
								ThreePointPoly = CurSegCt == 1 ? true: false;
								// first polygon will consist of three nodes, 
								// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
								// NextNodesArray[CurSegCt]
								// the rest four nodes:
								// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
								// NextNodesArray[CurSegCt], NextNodesArray[CurSegCt - 1]
								// set a flag to tell that a node has been used already and then it needs to be cloned
								if (! ThreePointPoly)
									{
									// if the two outer nodes are essentially the same, the new node position should be remapped
									// to the old position and this polygon skipped.
									// If only the inner nodes are essentially the same, the new inner node should be remapped to
									// the old node and it becomes a three point polygon
									if (CurNodesArray[CurSegCt - 1]->SimilarPointLatLon(NextNodesArray[CurSegCt - 1], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
										{
										NextNodesArray[CurSegCt - 1]->Lat = CurNodesArray[CurSegCt - 1]->Lat;
										NextNodesArray[CurSegCt - 1]->Lon = CurNodesArray[CurSegCt - 1]->Lon;
										ThreePointPoly = true;
										if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
											{
											NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
											NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
											SkipIt = true;
											} // if
										} // if
									} // if
								else
									{
									// if the two outer nodes are essentially the same, the new node position should be remapped
									// to the old position and this polygon skipped.
									if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
										{
										NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
										NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
										SkipIt = true;
										} // if
									} // else
								if (! SkipIt)
									{
									if (NewNodes[0] = CurNodesFlagArray[CurSegCt - 1] ? new VectorNode(CurNodesArray[CurSegCt - 1]): 
										CurNodesArray[CurSegCt - 1])
										{
										CurNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
										if (NewNodes[1] = CurNodesFlagArray[CurSegCt] ? new VectorNode(CurNodesArray[CurSegCt]): 
											CurNodesArray[CurSegCt])
											{
											CurNodesFlagArray[CurSegCt] = 1;	// indicates node was used
											if (NewNodes[2] = NextNodesFlagArray[CurSegCt] ? new VectorNode(NextNodesArray[CurSegCt]): 
												NextNodesArray[CurSegCt])
												{
												NextNodesFlagArray[CurSegCt] = 1;	// indicates node was used
												if (CurSegCt == 1 || (NewNodes[3] = NextNodesFlagArray[CurSegCt - 1] ? new VectorNode(NextNodesArray[CurSegCt - 1]): 
													NextNodesArray[CurSegCt - 1]))
													{
													if (CurSegCt > 1)
														{
														NextNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
														} // if
													if (*NewPolyListPtr = new VectorPolygonListDouble())
														{
														if (! (NewPoly = (*NewPolyListPtr)->MakeVectorPolygon(NewNodes, CurSegCt == 1 ? 3: 4, PolyLiner)))
															Success = false;
														else
															{
															if (Outbound)
																{
																if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], WCS_TFXDETAIL_FLAG_SINGLENODE | WCS_TFXDETAIL_FLAG_TRAILINGNODE))
																	Success = false;
																} // if
															else
																{
																if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], WCS_TFXDETAIL_FLAG_SINGLENODE | WCS_TFXDETAIL_FLAG_LEADINGNODE))
																	Success = false;
																} // else
															} // else
														NewPolyListPtr = (VectorPolygonListDouble **)&(*NewPolyListPtr)->NextPolygonList;
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
									} // if ! SkipIt
								} // for

							// copy the nodes over so they can be used as references
							for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
								{
								if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
									delete CurNodesArray[CurSegCt];
								CurNodesArray[CurSegCt] = NextNodesArray[CurSegCt];
								CurNodesFlagArray[CurSegCt] = NextNodesFlagArray[CurSegCt];
								NextNodesArray[CurSegCt] = NULL;
								} // for
							} // if
						} // if
						
					// break here if we are back to the original starting segment - we just needed to link
					// the last filler polygon back to the starting orthogonal offset
					if (NodeCt == PolyLiner->TotalNumNodes)
						break;

					// if the next angle is greater than 0 we'll be making pie slices to infill
					// If less than 0 there would be overlap between two sets of polygons and that causes texture
					// ambiguities so we need to chop off the polygons short of the full orthogonal shape
					if (TestAngle >= -1.0E-11)
						{
						// make a new set of nodes offsetting the next vertex at right angles
						NewNodeArray = NextNodesArray;
						NewNodeFlagArray = NextNodesFlagArray;
						if (NewNodeArray[0] = new VectorNode(NodeStash->NextNode))
							{
							NewNodeFlagArray[0] = 0;
							for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
								{
								TempNode.Lat = NodeStash->NextNode->Lat + VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
								if (TempNode.Lat >= 90.0)
									{
									Success = false;
									break;
									} // if illegal latitude
								TempNode.Lon = NodeStash->NextNode->Lon - VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * NextInvMetersPerDegLon;
								TempNode.Elev = NodeStash->NextNode->Elev;
								if (! (NewNodeArray[CurSegCt] = new VectorNode(TempNode)))
									{
									Success = false;
									break;
									} // if
								NewNodeFlagArray[CurSegCt] = 0;
								} // for
							} // if
						else
							Success = false;
						} // if
					else
						{
						// make a new set of nodes offsetting the next vertex at half the angle between the orthogonals
						// to the current segment and the next segment.
						// the angle is negative and we rotate it from the current orthogonal position.
						VBaseNextPt.RotateY(TestAngle * .5);
						VBaseNextPt.UnitVector();
						// Distances along the vector need to be compensated to keep the segment widths the same
						CompDistance = fabs(1.0 / cos((TestAngle * .5) * PiOver180));
						
						NewNodeArray = NextNodesArray;
						NewNodeFlagArray = NextNodesFlagArray;
						if (NewNodeArray[0] = new VectorNode(NodeStash->NextNode))
							{
							NewNodeFlagArray[0] = 0;
							for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
								{
								TempNode.Lat = NodeStash->NextNode->Lat + CompDistance * VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
								if (TempNode.Lat >= 90.0)
									{
									Success = false;
									break;
									} // if illegal latitude
								TempNode.Lon = NodeStash->NextNode->Lon - CompDistance * VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * NextInvMetersPerDegLon;
								TempNode.Elev = NodeStash->NextNode->Elev;
								if (! (NewNodeArray[CurSegCt] = new VectorNode(TempNode)))
									{
									Success = false;
									break;
									} // if
								NewNodeFlagArray[CurSegCt] = 0;
								} // for
							} // if
						else
							Success = false;
						} // else

					// make a polygon out of the vertices for each segment. Be sure to replicate nodes and only use them once
					for (CurSegCt = 1; Success && CurSegCt < NumSegmentNodes; ++CurSegCt)
						{
						// see if the polygon should be created by testing the dot product of the segments that are parallel to each other
						// If the dot product is negative find the intersection where the non-parallel edges cross and make a 
						// polygon out of that portion. Then break out of this loop.
						TriangleBoundingBoxVector OneSeg, OtherSeg;
						OneSeg.SetXY(NextNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt - 1]);
						OtherSeg.SetXY(NextNodesArray[CurSegCt], CurNodesArray[CurSegCt]);
						if (OneSeg.DotProductXY(&OneSeg, &OtherSeg) < 0.0)
							{
							// polygon will consist of three nodes. Two are from the existing arrays and one node at the intersection of the
							// non-parallel sides of the 4 node polygon
							if (NewNodes[0] = CurNodesFlagArray[CurSegCt - 1] ? new VectorNode(CurNodesArray[CurSegCt - 1]): 
								CurNodesArray[CurSegCt - 1])
								{
								CurNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
								if (NewNodes[2] = NextNodesFlagArray[CurSegCt - 1] ? new VectorNode(NextNodesArray[CurSegCt - 1]): 
									NextNodesArray[CurSegCt - 1])
									{
									NextNodesFlagArray[CurSegCt - 1] = 1;
									NewNodes[0]->NextNode = CurNodesArray[CurSegCt];
									NewNodes[2]->NextNode = NextNodesArray[CurSegCt];
									NewNodes[1] = VI.FindIntersectionMakeNode(NewNodes[0], NewNodes[2]);
									if (! NewNodes[1])
										{
										if (NewNodes[1] = NextNodesFlagArray[CurSegCt] ? new VectorNode(NextNodesArray[CurSegCt]): 
											NextNodesArray[CurSegCt])
											{
											NextNodesFlagArray[CurSegCt] = 1;	// indicates node was used
											} // if
										} // if 
									if (NewNodes[1])
										{
										if (*NewPolyListPtr = new VectorPolygonListDouble())
											{
											if (! (NewPoly = (*NewPolyListPtr)->MakeVectorPolygon(NewNodes, 3, PolyLiner)))
												Success = false;
											else
												{
												if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], 0))
													Success = false;
												} // if
											NewPolyListPtr = (VectorPolygonListDouble **)&(*NewPolyListPtr)->NextPolygonList;
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
							break;
							} // if
						// polygons consist of four nodes:
						// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
						// NextNodesArray[CurSegCt], NextNodesArray[CurSegCt - 1]
						// set a flag to tell that a node has been used already and then it needs to be cloned
						if (NewNodes[0] = CurNodesFlagArray[CurSegCt - 1] ? new VectorNode(CurNodesArray[CurSegCt - 1]): 
							CurNodesArray[CurSegCt - 1])
							{
							CurNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
							if (NewNodes[1] = CurNodesFlagArray[CurSegCt] ? new VectorNode(CurNodesArray[CurSegCt]): 
								CurNodesArray[CurSegCt])
								{
								CurNodesFlagArray[CurSegCt] = 1;	// indicates node was used
								if (NewNodes[2] = NextNodesFlagArray[CurSegCt] ? new VectorNode(NextNodesArray[CurSegCt]): 
									NextNodesArray[CurSegCt])
									{
									NextNodesFlagArray[CurSegCt] = 1;	// indicates node was used
									if (NewNodes[3] = NextNodesFlagArray[CurSegCt - 1] ? new VectorNode(NextNodesArray[CurSegCt - 1]): 
										NextNodesArray[CurSegCt - 1])
										{
										NextNodesFlagArray[CurSegCt - 1] = 1;
										if (*NewPolyListPtr = new VectorPolygonListDouble())
											{
											if (! (NewPoly = (*NewPolyListPtr)->MakeVectorPolygon(NewNodes, 4, PolyLiner)))
												Success = false;
											else
												{
												if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], 0))
													Success = false;
												} // if
											NewPolyListPtr = (VectorPolygonListDouble **)&(*NewPolyListPtr)->NextPolygonList;
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
						} // for

					// copy the nodes over so they can be used as references
					for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
						{
						if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
							delete CurNodesArray[CurSegCt];
						CurNodesArray[CurSegCt] = NextNodesArray[CurSegCt];
						CurNodesFlagArray[CurSegCt] = NextNodesFlagArray[CurSegCt];
						NextNodesArray[CurSegCt] = NULL;
						} // for

					// if right veering
						// make triangles and quadrilaterals to fill gap between orthogonal to
						// current segment and orthogonal to next segment.
						// Don't form the last set of polygons, they will be made for the next segment
						// unless this is the last segment.
						// Make an even number of segments so that the first half can be linked to the preceding segment 
						// and the second half can be linked to the next segment for distance-along-vector purposes
					if (TestAngle > 1.0E-11)	// might want to give it a little leeway for roundoff so we don't get extremely uselss thin slivers
						{
						int AnglesToTurn, HalfAngles;
						double AngleInterval;

						ConnectBack = NewOrthogonal = true;
						// nodes every 22.5 degrees
						if ((AnglesToTurn = int(TestAngle / 30.0)) > 0)
							{
							if (AnglesToTurn % 2)
								++AnglesToTurn;
							AngleInterval = TestAngle / AnglesToTurn;
							HalfAngles = AnglesToTurn / 2;
							// at specified angular intervals nodes are added at the PolyWidth radius from the vertex NodeStash->NextNode
							for (int AnglesTurned = 1; AnglesTurned < AnglesToTurn; ++AnglesTurned)
								{
								// already unitized
								VBaseNextPt.RotateY(AngleInterval);
								// make a new set of nodes offsetting the next vertex at right angles
								if (NextNodesArray[0] = new VectorNode(NodeStash->NextNode))
									{
									NextNodesFlagArray[0] = 0;
									for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
										{
										TempNode.Lat = NodeStash->NextNode->Lat + VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
										if (TempNode.Lat >= 90.0)
											{
											Success = false;
											break;
											} // if illegal latitude
										TempNode.Lon = NodeStash->NextNode->Lon - VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * NextInvMetersPerDegLon;
										TempNode.Elev = NodeStash->NextNode->Elev;
										if (! (NextNodesArray[CurSegCt] = new VectorNode(TempNode)))
											{
											Success = false;
											break;
											} // if
										NextNodesFlagArray[CurSegCt] = 0;
										} // for
									} // if
								else
									Success = false;

								// make a polygon out of the vertices for each segment. Be sure to replicate nodes and only use them once
								for (CurSegCt = 1; Success && CurSegCt < NumSegmentNodes; ++CurSegCt)
									{
									SkipIt = false;
									ThreePointPoly = CurSegCt == 1 ? true: false;
									// first polygon will consist of three nodes, 
									// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
									// NextNodesArray[CurSegCt]
									// the rest four nodes:
									// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
									// NextNodesArray[CurSegCt], NextNodesArray[CurSegCt - 1]
									// set a flag to tell that a node has been used already and then it needs to be cloned
									if (! ThreePointPoly)
										{
										// if the two outer nodes are essentially the same, the new node position should be remapped
										// to the old position and this polygon skipped.
										// If only the inner nodes are essentially the same, the new inner node should be remapped to
										// the old node and it becomes a three point polygon
										if (CurNodesArray[CurSegCt - 1]->SimilarPointLatLon(NextNodesArray[CurSegCt - 1], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
											{
											NextNodesArray[CurSegCt - 1]->Lat = CurNodesArray[CurSegCt - 1]->Lat;
											NextNodesArray[CurSegCt - 1]->Lon = CurNodesArray[CurSegCt - 1]->Lon;
											ThreePointPoly = true;
											if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
												{
												NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
												NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
												SkipIt = true;
												} // if
											} // if
										} // if
									else
										{
										// if the two outer nodes are essentially the same, the new node position should be remapped
										// to the old position and this polygon skipped.
										if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
											{
											NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
											NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
											SkipIt = true;
											} // if
										} // else
									if (! SkipIt)
										{
										if (NewNodes[0] = CurNodesFlagArray[CurSegCt - 1] ? new VectorNode(CurNodesArray[CurSegCt - 1]): 
											CurNodesArray[CurSegCt - 1])
											{
											CurNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
											if (NewNodes[1] = CurNodesFlagArray[CurSegCt] ? new VectorNode(CurNodesArray[CurSegCt]): 
												CurNodesArray[CurSegCt])
												{
												CurNodesFlagArray[CurSegCt] = 1;	// indicates node was used
												if (NewNodes[2] = NextNodesFlagArray[CurSegCt] ? new VectorNode(NextNodesArray[CurSegCt]): 
													NextNodesArray[CurSegCt])
													{
													NextNodesFlagArray[CurSegCt] = 1;	// indicates node was used
													if (ThreePointPoly || (NewNodes[3] = NextNodesFlagArray[CurSegCt - 1] ? new VectorNode(NextNodesArray[CurSegCt - 1]): 
														NextNodesArray[CurSegCt - 1]))
														{
														if (! ThreePointPoly)
															{
															NextNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
															} // if
														if (*NewPolyListPtr = new VectorPolygonListDouble())
															{
															if (! (NewPoly = (*NewPolyListPtr)->MakeVectorPolygon(NewNodes, ThreePointPoly ? 3: 4, PolyLiner)))
																Success = false;
															else
																{
																// set segment data for ecosystem and terraffector
																// if/else commented out 7/17/07 when  a flag value was added to TFxDetail to tell it only one node to evaluate
																//if (AnglesTurned <= HalfAngles)
																	{
																	if (Outbound)
																		{
																		if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], WCS_TFXDETAIL_FLAG_SINGLENODE | WCS_TFXDETAIL_FLAG_LEADINGNODE))
																			Success = false;
																		} // if
																	else
																		{
																		if (! NewPoly->SetTfxDetail(NewEffect, NewVector, CurrentPointIndex, SegNumberArray[CurSegCt], WCS_TFXDETAIL_FLAG_SINGLENODE | WCS_TFXDETAIL_FLAG_TRAILINGNODE))
																			Success = false;
																		} // else
																	} // if
																/*
																else
																	{
																	if (! NewPoly->SetTfxDetail(NewEffect, NewVector, NextPointIndex, SegNumberArray[CurSegCt]))
																		Success = false;
																	} // else
																*/
																} // if
															NewPolyListPtr = (VectorPolygonListDouble **)&(*NewPolyListPtr)->NextPolygonList;
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
										} // if ! SkipIt
									} // for

								// copy the nodes over so they can be used as references
								for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
									{
									if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
										delete CurNodesArray[CurSegCt];
									CurNodesArray[CurSegCt] = NextNodesArray[CurSegCt];
									CurNodesFlagArray[CurSegCt] = NextNodesFlagArray[CurSegCt];
									NextNodesArray[CurSegCt] = NULL;
									} // for
								} // for
							} // if
						} // if
					else
						{
						// this should always be true now that half angles are being turned on the inside bends.
						//ConnectBack = false;
						ConnectBack = true;
						NewOrthogonal = false;
						} // else
					} // for
				for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
					{
					if (NextNodesArray[CurSegCt] && ! NextNodesFlagArray[CurSegCt])
						delete NextNodesArray[CurSegCt];
					if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
						delete CurNodesArray[CurSegCt];
					} // for
				} // if
			} // if
		if (DistanceArray)
			delete [] DistanceArray;
		if (CurNodesArray)
			delete [] CurNodesArray;
		if (NextNodesArray)
			delete [] NextNodesArray;
		if (CurNodesFlagArray)
			delete [] CurNodesFlagArray;
		if (NextNodesFlagArray)
			delete [] NextNodesFlagArray;
		if (SegNumberArray)
			delete [] SegNumberArray;
		} // if
	else
		Success = false;
	} // if

// cleanup
if (TempPolyReverse)
	delete TempPolyReverse;
if (TempPolyFwd)
	delete TempPolyFwd;
if (VectorPointIndexArray)
	delete [] VectorPointIndexArray;

#ifdef WCS_OUTPUT_TFXDETAIL
char OutputStr[256];
VectorPolygonListDouble *CurPolyList;
EffectJoeList *CurEffect;
TfxDetail *CurData;

for (CurPolyList = NewPolyList; CurPolyList; CurPolyList = (VectorPolygonListDouble *)CurPolyList->NextPolygonList)
	{
	sprintf(OutputStr, "%p \n", CurPolyList->MyPolygon);
	OutputDebugString(OutputStr);
	for (CurEffect = CurPolyList->MyPolygon->MyEffects; CurEffect; CurEffect = CurEffect->Next)
		{
		sprintf(OutputStr, "  %s, %s\n", CurEffect->MyEffect->Name, CurEffect->MyJoe->GetBestName());
		OutputDebugString(OutputStr);
		for (CurData = CurEffect->VSData; CurData; CurData = CurData->Next)
			{
			sprintf(OutputStr, "    Index=%d, Segment=%d, Singlenode=%d, Trailing=%d, Leading=%d\n", CurData->Index, CurData->XSectSegment, CurData->FlagCheck(WCS_TFXDETAIL_FLAG_SINGLENODE), 
				CurData->FlagCheck(WCS_TFXDETAIL_FLAG_TRAILINGNODE), CurData->FlagCheck(WCS_TFXDETAIL_FLAG_LEADINGNODE));
			OutputDebugString(OutputStr);
			} // for
		} // for
	} // for
#endif // WCS_OUTPUT_TFXDETAIL
return (NewPolyList);

} // EffectEval::MakeBoundingVectorPolygons

/*===========================================================================*/

VectorPolygonListDouble *EffectEval::MakeBoundingVectorPointPolygons(GeneralEffect *NewEffect, Joe *NewVector,
	double MetersPerDegLat, bool &Success)
{
double CurMetersPerDegLon, CurInvMetersPerDegLon, InvMetersPerDegLat;
unsigned long NodeCt, NumSegmentNodes, CurSegCt;
double *DistanceArray = NULL;
short *SegNumberArray = NULL;
char *CurNodesFlagArray = NULL, *NextNodesFlagArray = NULL, *NewNodeFlagArray;
VectorPolygon *NewPoly = NULL, *TempPolyFwd = NULL;
VectorNode *CurNode, *NewNodes[4], **CurNodesArray = NULL, **NextNodesArray = NULL, **NewNodeArray;
VectorPolygonListDouble *NewPolyList = NULL, **NewPolyListPtr;
bool ThreePointPoly, SkipIt, InsufficientNodes = false;
VectorNode TempNode;
VertexBase VBaseNextPt;

Success = true;

NewPolyListPtr = &NewPolyList;

if (Success && (TempPolyFwd = new VectorPolygon(DBHost, NewEffect, NewVector, true, InsufficientNodes)))
	{
	Success = TempPolyFwd->ConvertToDefGeo();
	} // if
else
	Success = false;

if (Success)
	{
	// PolyLiner must now be a supplied or constructed fact
	// One point vectors aren't covered here so return failure
	void *PlaceHolder = NULL;
	double SegWidth;

	// allocate enough node pointers in two arrays to handle all the tfx segments
	// GetNumSegments() includes approach slope. Add a node for 0 distance
	NumSegmentNodes = NewEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR ? 
		((TerraffectorEffect *)NewEffect)->GetNumSegments() + 1: 2;

	DistanceArray = new double[NumSegmentNodes];
	CurNodesArray = new VectorNode *[NumSegmentNodes];
	NextNodesArray = new VectorNode *[NumSegmentNodes];
	CurNodesFlagArray = new char[NumSegmentNodes];
	NextNodesFlagArray = new char[NumSegmentNodes];
	SegNumberArray = new short[NumSegmentNodes];

	if (DistanceArray && CurNodesArray && NextNodesArray && CurNodesFlagArray && NextNodesFlagArray && SegNumberArray)
		{
		DistanceArray[0] = 0.0;
		SegNumberArray[0] = 0;
		if (NewEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
			{
			for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
				{
				if (! (PlaceHolder = ((TerraffectorEffect *)NewEffect)->GetNextSegmentWidth(PlaceHolder, SegWidth)))
					{
					SegWidth = ((TerraffectorEffect *)NewEffect)->GetRadiusWidth();
					SegNumberArray[CurSegCt] = WCS_TFXDETAIL_SLOPESEGMENT;
					} // if
				else
					{
					SegNumberArray[CurSegCt] = (short)(CurSegCt - 1);
					} // else
				DistanceArray[CurSegCt] = SegWidth + DistanceArray[CurSegCt - 1];
				} // for
			} // if
		else if (NewEffect->EffectType == WCS_EFFECTSSUBCLASS_STREAM)
			{
			DistanceArray[1] = ((StreamEffect *)NewEffect)->GetAnimPtr(WCS_EFFECTS_STREAM_ANIMPAR_RADIUS)->CurValue;
			SegNumberArray[1] = 0;
			} // else
		else
			{
			DistanceArray[1] = NewEffect->UseGradient ? NewEffect->GetMaxProfileDistance(): 0.0;
			SegNumberArray[1] = 0;
			} // else

		// check and assure that there is a need for the offsetting polygons
		if (DistanceArray[1] > 0.0)
			{
			InvMetersPerDegLat = 1.0 / MetersPerDegLat;

			// Find the offsets for the nodes in PolyLiner and create new nodes at the offset points.
			// Begin by making offsets from the first node orthogonal and to the left of the first segment.
			// If there are numerous segments of a tfx cross section to be polygonized, each segment 
			// including the final tfx radius should get the same number of polygons created. Except for the innermost
			// triangle polygons in the right-veering fans, all other fan polygons will have four nodes.

			// The vertex order for rectangular polygons will be:
				// current vertex, outer projection of current, outer projection of next vertex, next vertex.
			// The vertex order for triangular polygons will be:
				// current vertex, outer projection of current, next outer projection of current vertex.
			for (NodeCt = 0, CurNode = TempPolyFwd->PolyFirstNode(); NodeCt < TempPolyFwd->TotalNumNodes; CurNode = CurNode->NextNode, ++NodeCt)
				{
				// each node gets its own latitude-specific longitude conversion factor
				CurMetersPerDegLon = MetersPerDegLat * cos(CurNode->Lat * PiOver180);
				if (CurMetersPerDegLon > 0.0)
					CurInvMetersPerDegLon = 1.0 / CurMetersPerDegLon;
				else
					CurInvMetersPerDegLon = 0.0;

				VBaseNextPt.XYZ[2] = 1.0;
				VBaseNextPt.XYZ[0] = 0.0;

				// create a new set of nodes north of the current node
				// pick a set of nodes to create depending on whether this set must connect back to the last set
				NewNodeArray = CurNodesArray;
				NewNodeFlagArray = CurNodesFlagArray;
				if (NewNodeArray[0] = new VectorNode(CurNode))
					{
					NewNodeFlagArray[0] = 0;
					for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
						{
						TempNode.Lat = CurNode->Lat + VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
						if (TempNode.Lat >= 90.0)
							{
							Success = false;
							break;
							} // if illegal latitude
						TempNode.Lon = CurNode->Lon - VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * CurInvMetersPerDegLon;
						TempNode.Elev = CurNode->Elev;
						if (! (NewNodeArray[CurSegCt] = new VectorNode(TempNode)))
							{
							Success = false;
							break;
							} // if
						NewNodeFlagArray[CurSegCt] = 0;
						} // for
					} // if
				else
					Success = false;
					
				// make triangles and quadrilaterals to surround the node at a specified angle.
				// connect the last set back to the first set of nodes.
				int AnglesToTurn;
				double AngleInterval;

				// nodes every 30.0 degrees
				AnglesToTurn = 12;
				AngleInterval = 30.0;
				// at specified angular intervals nodes are added at the PolyWidth radius from the vertex NodeStash->NextNode
				for (int AnglesTurned = 1; AnglesTurned <= AnglesToTurn; ++AnglesTurned)
					{
					// already unitized
					VBaseNextPt.RotateY(AngleInterval);
					// might be some roundoff error so make sure the original north angle is restored precisely
					if (AnglesTurned == AnglesToTurn)
						{
						// back to north again
						VBaseNextPt.XYZ[0] = 0.0;
						VBaseNextPt.XYZ[2] = 1.0;
						} // if
					// make a new set of nodes offsetting the vertex at the current angle
					if (NextNodesArray[0] = new VectorNode(CurNode))
						{
						NextNodesFlagArray[0] = 0;
						for (CurSegCt = 1; CurSegCt < NumSegmentNodes; ++CurSegCt)
							{
							TempNode.Lat = CurNode->Lat + VBaseNextPt.XYZ[2] * DistanceArray[CurSegCt] * InvMetersPerDegLat;
							if (TempNode.Lat >= 90.0)
								{
								Success = false;
								break;
								} // if illegal latitude
							TempNode.Lon = CurNode->Lon - VBaseNextPt.XYZ[0] * DistanceArray[CurSegCt] * CurInvMetersPerDegLon;
							TempNode.Elev = CurNode->Elev;
							if (! (NextNodesArray[CurSegCt] = new VectorNode(TempNode)))
								{
								Success = false;
								break;
								} // if
							NextNodesFlagArray[CurSegCt] = 0;
							} // for
						} // if
					else
						Success = false;

					// make a polygon out of the vertices for each segment. Be sure to replicate nodes and only use them once
					for (CurSegCt = 1; Success && CurSegCt < NumSegmentNodes; ++CurSegCt)
						{
						SkipIt = false;
						ThreePointPoly = CurSegCt == 1 ? true: false;
						// first polygon will consist of three nodes, 
						// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
						// NextNodesArray[CurSegCt]
						// the rest four nodes:
						// CurNodesArray[CurSegCt - 1], CurNodesArray[CurSegCt], 
						// NextNodesArray[CurSegCt], NextNodesArray[CurSegCt - 1]
						// set a flag to tell that a node has been used already and then it needs to be cloned
						if (! ThreePointPoly)
							{
							// if the two outer nodes are essentially the same, the new node position should be remapped
							// to the old position and this polygon skipped.
							// If only the inner nodes are essentially the same, the new inner node should be remapped to
							// the old node and it becomes a three point polygon
							if (CurNodesArray[CurSegCt - 1]->SimilarPointLatLon(NextNodesArray[CurSegCt - 1], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
								{
								NextNodesArray[CurSegCt - 1]->Lat = CurNodesArray[CurSegCt - 1]->Lat;
								NextNodesArray[CurSegCt - 1]->Lon = CurNodesArray[CurSegCt - 1]->Lon;
								ThreePointPoly = true;
								if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
									{
									NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
									NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
									SkipIt = true;
									} // if
								} // if
							} // if
						else
							{
							// if the two outer nodes are essentially the same, the new node position should be remapped
							// to the old position and this polygon skipped.
							if (CurNodesArray[CurSegCt]->SimilarPointLatLon(NextNodesArray[CurSegCt], WCS_VECTORPOLYGON_NODECOORD_TFXTOLERANCE))
								{
								NextNodesArray[CurSegCt]->Lat = CurNodesArray[CurSegCt]->Lat;
								NextNodesArray[CurSegCt]->Lon = CurNodesArray[CurSegCt]->Lon;
								SkipIt = true;
								} // if
							} // else
						if (! SkipIt)
							{
							if (NewNodes[0] = CurNodesFlagArray[CurSegCt - 1] ? new VectorNode(CurNodesArray[CurSegCt - 1]): 
								CurNodesArray[CurSegCt - 1])
								{
								CurNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
								if (NewNodes[1] = CurNodesFlagArray[CurSegCt] ? new VectorNode(CurNodesArray[CurSegCt]): 
									CurNodesArray[CurSegCt])
									{
									CurNodesFlagArray[CurSegCt] = 1;	// indicates node was used
									if (NewNodes[2] = NextNodesFlagArray[CurSegCt] ? new VectorNode(NextNodesArray[CurSegCt]): 
										NextNodesArray[CurSegCt])
										{
										NextNodesFlagArray[CurSegCt] = 1;	// indicates node was used
										if (ThreePointPoly || (NewNodes[3] = NextNodesFlagArray[CurSegCt - 1] ? new VectorNode(NextNodesArray[CurSegCt - 1]): 
											NextNodesArray[CurSegCt - 1]))
											{
											if (! ThreePointPoly)
												{
												NextNodesFlagArray[CurSegCt - 1] = 1;	// indicates node was used
												} // if
											if (*NewPolyListPtr = new VectorPolygonListDouble())
												{
												if (! (NewPoly = (*NewPolyListPtr)->MakeVectorPolygon(NewNodes, ThreePointPoly ? 3: 4, TempPolyFwd)))
													Success = false;
												else
													{
													// set segment data for ecosystem and terraffector
													if (! NewPoly->SetTfxDetail(NewEffect, NewVector, NodeCt, SegNumberArray[CurSegCt], WCS_TFXDETAIL_FLAG_SINGLENODE | WCS_TFXDETAIL_FLAG_TRAILINGNODE))
														Success = false;
													} // if
												NewPolyListPtr = (VectorPolygonListDouble **)&(*NewPolyListPtr)->NextPolygonList;
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
							} // if ! SkipIt
						} // for

					// copy the nodes over so they can be used as references
					for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
						{
						if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
							delete CurNodesArray[CurSegCt];
						CurNodesArray[CurSegCt] = NextNodesArray[CurSegCt];
						CurNodesFlagArray[CurSegCt] = NextNodesFlagArray[CurSegCt];
						NextNodesArray[CurSegCt] = NULL;
						} // for
					} // for
				} // for
			for (CurSegCt = 0; CurSegCt < NumSegmentNodes; ++CurSegCt)
				{
				if (NextNodesArray[CurSegCt] && ! NextNodesFlagArray[CurSegCt])
					delete NextNodesArray[CurSegCt];
				if (CurNodesArray[CurSegCt] && ! CurNodesFlagArray[CurSegCt])
					delete CurNodesArray[CurSegCt];
				} // for
			} // if
		} // if
	if (DistanceArray)
		delete [] DistanceArray;
	if (CurNodesArray)
		delete [] CurNodesArray;
	if (NextNodesArray)
		delete [] NextNodesArray;
	if (CurNodesFlagArray)
		delete [] CurNodesFlagArray;
	if (NextNodesFlagArray)
		delete [] NextNodesFlagArray;
	if (SegNumberArray)
		delete [] SegNumberArray;
	} // if
else
	Success = false;

// cleanup
if (TempPolyFwd)
	delete TempPolyFwd;

return (NewPolyList);

} // EffectEval::MakeBoundingVectorPointPolygons

/*===========================================================================*/

void EffectEval::ComparePolygonsInLists(VectorPolygonListDouble *List1, VectorPolygonListDouble *List2)
{
VectorPolygonListDouble *CurList1, *CurList2;
char PtrString[128];

for (CurList1 = List1; CurList1; CurList1 = (VectorPolygonListDouble *)CurList1->NextPolygonList)
	{
	for (CurList2 = List2; CurList2; CurList2 = (VectorPolygonListDouble *)CurList2->NextPolygonList)
		{
		if (CurList2->MyPolygon == CurList1->MyPolygon)
			{
			sprintf(PtrString, "%p in both lists\n", CurList2->MyPolygon);
			OutputDebugString(PtrString);
			} // if
		} // for
	} // for
	
} // EffectEval::ComparePolygonsInLists
