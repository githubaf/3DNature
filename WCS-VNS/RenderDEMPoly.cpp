// RenderDEMPoly.cpp
// Functions for rendering terrain using VectorPolygons in World Construction Set.
// Functions moved from RenderDEM.cpp 8/15/06.
// Created by Gary R. Huber. 
// Copyright 2006 by 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "Render.h"
#include "Application.h"
#include "EffectsLib.h"
#include "Database.h"
#include "Useful.h"
#include "Log.h"
#include "Joe.h"
#include "Project.h"
#include "RenderControlGUI.h"
#include "Requester.h"
#include "Interactive.h"
#include "DEM.h"
#include "GraphData.h"
#include "AppMem.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "VectorIntersecter.h"	// for WCS_VECTORPOLYGON_NODECOORD_TOLERANCE
#include "EffectEval.h"
#include "Lists.h"
#include "QuantaAllocator.h"

#ifdef WCS_BUILD_DEMO
extern unsigned long MemFuck;
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_FRANK
#undef _OPENMP	// avoid deadlocks on Railway project
#endif // WCS_BUILD_FRANK

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

extern NotifyTag ThawEvent[2];
//char DebugStr[128];

//#define ENABLE_RENDER_TIMER
#ifdef ENABLE_RENDER_TIMER
extern double RenderTimer1, RenderTimer2;
#endif // ENABLE_RENDER_TIMER

#ifdef WCS_VECPOLY_EFFECTS
// see also build settings defines or FeatureConfig.h
// see also defined values in EffectEval.cpp
//#define DEBUG_POLYGONS_TO_VECTOR	// creates debug output from EffectEval::MergePolygons
#define MAKENOCLONES
//Resetting the water flags when a node's elevation is copied, if the node elevation changes, seems to fix
//a problem in water spikes along the edges of vector-bounded lakes that also control an area terraffector 
//that digs the lake bed. Undefine WCS_RESET_WATER_FLAGS_ON_ELEV_CHANGE to see the error at FD=0.
#define WCS_RESET_WATER_FLAGS_ON_ELEV_CHANGE

bool Renderer::SubstituteInMasterLists(VectorPolygon *FindPoly, VectorPolygonListDouble *DEMPolyList,
	VectorPolygonListDouble *SubList, VectorPolygon *OriginalPoly)
{
VectorPolygonListDouble *CurDEMPoly, *StashNextPoly, *SubListCopy;
bool Success = true;

for (CurDEMPoly = DEMPolyList; CurDEMPoly; CurDEMPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList)
	{
	if (CurDEMPoly->MyPolygon == FindPoly)
		{
		SubListCopy = SubList;
		StashNextPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList;
		if (! OriginalPoly)
			{
			CurDEMPoly->MyPolygon = SubListCopy->MyPolygon;
			CurDEMPoly->DoubleVal = SubListCopy->DoubleVal;
			CurDEMPoly->Flags = 0;
			CurDEMPoly->MyPolygon->ImAnOriginalPolygon = true;
			SubListCopy = (VectorPolygonListDouble *)SubListCopy->NextPolygonList;
			} // if
		for (; SubListCopy; SubListCopy = (VectorPolygonListDouble *)SubListCopy->NextPolygonList)
			{
			if (CurDEMPoly->NextPolygonList = new VectorPolygonListDouble())
				{
				CurDEMPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList;
				CurDEMPoly->MyPolygon = SubListCopy->MyPolygon;
				CurDEMPoly->DoubleVal = SubListCopy->DoubleVal;
				CurDEMPoly->Flags = 0;
				CurDEMPoly->MyPolygon->ImAnOriginalPolygon = true;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		CurDEMPoly->NextPolygonList = StashNextPoly;
		break;	// got our man
		} // if
	} // for

for (CurDEMPoly = EvalEffects->PolygonList; CurDEMPoly; CurDEMPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList)
	{
	if (CurDEMPoly->MyPolygon == FindPoly)
		{
		SubListCopy = SubList;
		StashNextPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList;
		if (! OriginalPoly)
			{
			CurDEMPoly->MyPolygon = SubListCopy->MyPolygon;
			CurDEMPoly->DoubleVal = SubListCopy->DoubleVal;
			CurDEMPoly->Flags = 0;
			CurDEMPoly->MyPolygon->ImAnOriginalPolygon = true;
			SubListCopy = (VectorPolygonListDouble *)SubListCopy->NextPolygonList;
			} // if
		for (; SubListCopy; SubListCopy = (VectorPolygonListDouble *)SubListCopy->NextPolygonList)
			{
			if (CurDEMPoly->NextPolygonList = new VectorPolygonListDouble())
				{
				CurDEMPoly = (VectorPolygonListDouble *)CurDEMPoly->NextPolygonList;
				CurDEMPoly->MyPolygon = SubListCopy->MyPolygon;
				CurDEMPoly->DoubleVal = SubListCopy->DoubleVal;
				CurDEMPoly->Flags = 0;
				CurDEMPoly->MyPolygon->ImAnOriginalPolygon = true;
				} // if
			else
				{
				Success = false;
				break;
				} // else
			} // for
		CurDEMPoly->NextPolygonList = StashNextPoly;
		break;	// got our man
		} // if
	} // for

return (Success);

} // Renderer::SubstituteInMasterLists

/*===========================================================================*/

int Renderer::RenderDEMPoly(int MakeFractalMap, bool DEMNeedsProcessing, VectorPolygon *DEMVecPoly, VectorPolygonListDouble *DEMPolyList, double CurTime)
{
char CompleteDEMStr[256];
int Success = 1, SetTouchedTime = 0;
unsigned long Ct, NumPolys, VertNumber, PolyNumber;
BusyWin *BWDE = NULL;
struct TerrainPolygonSort *PolyArray;
unsigned long PolyCt, PolysPerRow;
#ifdef WCS_BUILD_DEMO
unsigned char MessyRed, MessyGreen, MessyBlue, OMessyRed;
long MessyRows, FirstMessyRow, MessyRow, MessyCol, MessyRowCt, MessyZip;
time_t MessyTime;
double MessyStep;
#endif // WCS_BUILD_DEMO

// variables used to determine if vector polygon data is being or about to be rendered or if it is available to free
LastPolyNumber = CurPolyNumber = -1;

if (CurDEM)
	{
	if (DEMNeedsProcessing)
		{
		// some basic vertex manipulation - displace while in native coord sys
		// convert to default lat/lon
		if (Success)
			Success = CurDEM->TransferXYZtoLatLon();	// OMP enabled
		if (Success)
			Success = ScaleDEMElevs(CurDEM);	// OMP enabled
		// Call a method on the EffectsLib that supplies elevation alterations that may apply within the DEM.
		EffectsBase->EvalHighLowEffectExtrema(DEMPolyList, &RendData, CurDEM->RelativeEffectAdd,
			CurDEM->RelativeEffectSubtract, CurDEM->AbsoluteEffectMax, CurDEM->AbsoluteEffectMin);
		} // if
	// project these vertices for use in clipping
	// Only needed in VNS 3 because of desire to sort polygons front to back for rendering efficiency
	// There is probably a better way to accomplish the re-ordering based on projecting just the corner vertices.
	if (Success)
		Success = ProjectDEMVertices(CurDEM);	// OMP enabled

	// create fractal map if needed
	if (Success && MakeFractalMap && CurDEM->FractalMap)
		Success = CreateFractalMap(EffectsBase->TerrainParamBase.DepthMapPixSize * 2.0, NULL);	// OMP enabled
	if (Success && (MakeFractalMap || DEMNeedsProcessing) && CurDEM->FractalMap)
		Success = ApplyFractalDepthTexture(CurDEM);

	if (Success && CmapsExist)
		Success = EffectsBase->BuildDEMSpecificCmapList(CurDEM);

	#ifdef WCS_BUILD_DEMO
	// in case they've hacked the limits
	if (RendRand.GenPRN() > .75)
		{
		if (Width > 643)
			return (0);
		if (Height > 452)
			return (0);
		} // if
	#endif // WCS_BUILD_DEMO

	#ifdef WCS_BUILD_DEMO
	if (ZBuf[0] > 1.0f || ZBuf[Width * Height - 1] > 1.0f)
		{
		MemFuck += 85076000;
		MessyStep = (double)Height / Width;
		MessyRows = Height / 10;
		GetTime(MessyTime);
		RendRand.Seed64(MessyTime, MessyTime);
		OMessyRed = (unsigned char)(RendRand.GenPRN() * 255);

		for (MessyCol = 0; MessyCol < Width; MessyCol ++)
			{
			MessyRed = OMessyRed;
			MessyGreen = OMessyRed + 128;
			MessyBlue = OMessyRed + 255;
			FirstMessyRow = (long)(MessyCol * MessyStep - MessyRows / 2);
			for (MessyRow = FirstMessyRow, MessyRowCt = 0; MessyRowCt < max(MessyRows, 10) && MessyRow < Height; MessyRowCt ++, MessyRow ++)
				{
				MessyRed += 10;
				MessyGreen += 15;
				MessyBlue += 20;
				if (MessyRow < 0)
					continue;
				MessyZip = MessyRow * Width + MessyCol;
				AABuf[MessyZip] = 128;
				ZBuf[MessyZip] = 0.0f;
				Bitmap[0][MessyZip] = MessyRed;
				Bitmap[1][MessyZip] = MessyGreen;
				Bitmap[2][MessyZip] = MessyBlue;
				} // for
			} // for
		} // if
	#endif // WCS_BUILD_DEMO

	// render the DEM
	if (Success)
		{

		// at this point we can go either of two ways: render in rows and columns as done in v4 or
		// sort the polygons according to their z values for better control over order of rendering
		// to elliminate artifacts along the seam where the right half of the image meets the left half.
		// Note that we will use Q values for sorting since Z will not discriminate in true distance from camera
		bool RenderIt = false;

		// in this scheme we need to create an array of all the polygons we plan to render,
		// ie. the ones with fractal map values >= 0. The array should consist of structures that contain the
		// DEM cell number and the Q value.

		// compute the number of possible cells
		NumPolys = (CurDEM->pLatEntries - 1) * (CurDEM->pLonEntries - 1);
		// add the number of possible foliage and 3D object and fence vertices
		NumPolys += EffectsBase->CountRenderItemVertices(&RendData, DBase);

		// create the array of a size large enough to hold all the polygons in the DEM
		#ifdef WCS_BUILD_DEMO
		if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort) + MemFuck, 0))
		#else // WCS_BUILD_DEMO
		if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort), 0))
		#endif // WCS_BUILD_DEMO
			{
			unsigned long VNum[4], CellCt;

			PolyCt = PolyNumber = CellCt = VertNumber = 0;	// this will count the polygons as they are inserted into the array
			for (CurDEM->Lr = 0; CurDEM->Lr < (long)CurDEM->pLonEntries - 1; ++CurDEM->Lr, ++VertNumber)
				{
				for (CurDEM->Lc = 0; CurDEM->Lc < (long)CurDEM->pLatEntries - 1; ++CurDEM->Lc, ++VertNumber, PolyNumber += 2, ++CellCt)
					{                                         //   _
					// test two polygons at every VertNumber station !\|	VertNumber is vertex at lower left (southwest)
					if (! CurDEM->FractalMap || (CurDEM->FractalMap[PolyNumber] >= 0 ||
						CurDEM->FractalMap[PolyNumber + 1] >= 0))
						{
						PolyArray[PolyCt].PolyQ = (float)((CurDEM->Vertices[VertNumber].ScrnXYZ[2]
							+ CurDEM->Vertices[VertNumber + 1].ScrnXYZ[2]
							+ CurDEM->Vertices[VertNumber + CurDEM->pLatEntries].ScrnXYZ[2]
							+ CurDEM->Vertices[VertNumber + CurDEM->pLatEntries + 1].ScrnXYZ[2])
							* .25 - ShadowMapDistanceOffset);
						PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_DEM;
						PolyArray[PolyCt++].PolyNumber = CellCt;
						} // if
					} // for
				} // for
			EffectsBase->FillRenderPolyArray(&RendData, PolyArray, PolyCt, CurDEM);
			if (PolyCt > 0)
				{
				// sort the polygons according to increasing Q
				if (PolyCt > 1)
					{
					qsort((void *)PolyArray, (size_t)PolyCt, (size_t)(sizeof (struct TerrainPolygonSort)), CompareTerrainPolygonSort);
					} // if enough polygons to bother sorting

				// render the polygons
				if (IsCamView)
					{
					strcpy(CompleteDEMStr, DEMstr);
					if (ShadowMapInProgress)
						{
						strcat(CompleteDEMStr, " ");
						strcat(CompleteDEMStr, "Shadows");
						} // if
					BWDE = new BusyWin(CompleteDEMStr, PolyCt, 'BWDE', 0);
					} // if
				else
					{
					Master->ProcInit(PolyCt, DEMstr);
					} // else

				PolysPerRow = (CurDEM->pLatEntries - 1);
				for (Ct = 0; Ct < PolyCt; )	// Ct is incremented in busy-win update section
					{
					if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_DEM)
						{
						SetTouchedTime = 1;
						// extract the vertex numbers from the polygon number
						CurPolyNumber = PolyArray[Ct].PolyNumber;
						CurDEM->Lr = CurPolyNumber / PolysPerRow;
						CurDEM->Lc = CurPolyNumber - (CurDEM->Lr * PolysPerRow);
						VNum[0] = CurDEM->Lc + CurDEM->pLatEntries * CurDEM->Lr;
						VNum[1] = VNum[0] + 1;
						VNum[2] = VNum[0] + CurDEM->pLatEntries + 1;
						VNum[3] = VNum[0] + CurDEM->pLatEntries;

						//sprintf(DebugStr, "%d compute, %d render \n", CurPolyNumber, LastPolyNumber);
						//OutputDebugString(DebugStr);

						// override culling = false
						RenderIt = true;	// tells PrepVectorPoly to evaluate render status
						if (Success = PrepVectorPoly(VNum, CurPolyNumber, false, DEMVecPoly, DEMPolyList, CurTime, RenderIt))
							{
							//sprintf(DebugStr, "%d tuning, %d rendering \n", CurPolyNumber, LastPolyNumber);
							//OutputDebugString(DebugStr);
							if (RenderIt)
								{
								#ifndef _OPENMP
								if (! (Success = TuneVectorPoly(CurPolyNumber, PolysPerRow, DEMVecPoly, DEMPolyList, CurTime)))
									break;
								#else // _OPENMP
								#pragma omp parallel sections
									{
									#pragma omp section
										{
										#ifdef ENABLE_RENDER_TIMER
										StartHiResTimer();
										#endif // ENABLE_RENDER_TIMER
										if (! TuneVectorPoly(CurPolyNumber, PolysPerRow, DEMVecPoly, DEMPolyList, CurTime))
											Success = 0;
										#ifdef ENABLE_RENDER_TIMER
										RenderTimer1 += StopHiResTimer();
										#endif // ENABLE_RENDER_TIMER
										} // #pragma omp section
									#pragma omp section
										{
										if (LastPolyNumber >= 0)
											{
											#ifdef ENABLE_RENDER_TIMER
											StartHiResTimer();
											#endif // ENABLE_RENDER_TIMER
											if (! RenderVectorPoly(LastPolyNumber))
												Success = 0;
											#ifdef ENABLE_RENDER_TIMER
											RenderTimer2 += StopHiResTimer();
											#endif // ENABLE_RENDER_TIMER
											LastPolyNumber = -1;
											} // if
										} // #pragma omp section
									} // #pragma omp parallel sections
								//sprintf(DebugStr, "%d tuned\n", CurPolyNumber);
								//OutputDebugString(DebugStr);
								if (! Success)
									break;
								LastPolyNumber = CurPolyNumber;
								#endif // _OPENMP
								} // if
							} // if
						else
							break;
						CurPolyNumber = -1;
						} // if
					else if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_3DOBJECT)
						{
						if (! (Success = Render3DObject((Object3DVertexList *)PolyArray[Ct].PolyNumber)))
							break;
						} // else if
					else if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_FOLIAGE)
						{
						if (! (Success = RenderFoliage((FoliageVertexList *)PolyArray[Ct].PolyNumber)))
							break;
						} // else if
					else if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_FENCE)
						{
						if (! (Success = RenderFence((FenceVertexList *)PolyArray[Ct].PolyNumber)))
							break;
						} // else if
					else if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_LABEL)
						{
						if (! (Success = RenderLabel((LabelVertexList *)PolyArray[Ct].PolyNumber)))
							break;
						} // else if

					if (IsCamView)
						{
						if(BWDE->Update(++ Ct))
							{
							Success = 0;
							break;
							} // if
						} // if
					else
						{
						Master->ProcUpdate(++ Ct);
						if (! Master->IsRunning())
							{
							Success = 0;
							break;
							} // if
						} // else
					} // for
				#ifdef _OPENMP
				if (Success && LastPolyNumber >= 0)
					{
					#ifdef ENABLE_RENDER_TIMER
					StartHiResTimer();
					#endif // ENABLE_RENDER_TIMER
					Success = RenderVectorPoly(LastPolyNumber);
					#ifdef ENABLE_RENDER_TIMER
					RenderTimer2 += StopHiResTimer();
					#endif // ENABLE_RENDER_TIMER
					LastPolyNumber = -1;
					if (IsCamView)
						{
						if(BWDE->Update(++ Ct))
							{
							Success = 0;
							} // if
						} // if
					else
						{
						Master->ProcUpdate(++ Ct);
						if (! Master->IsRunning())
							{
							Success = 0;
							} // if
						} // else
					} // if
				#endif // _OPENMP
				} // if something to render
			AppMem_Free(PolyArray, NumPolys * sizeof (struct TerrainPolygonSort));
			} // if
		} // if Success
	if (SetTouchedTime)
		{
		time_t DEMTime;

		// in case they've hacked the limits
		GetTime(DEMTime);
		CurDEM->SetLastTouchedTime(DEMTime);
		} // if
	} // if CurDEM

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

#ifdef WCS_BUILD_DEMO
{ // temp scope
time_t MessyTime;

// in case they've hacked the limits
GetTime(MessyTime);
RendRand.Seed64(MessyTime, MessyTime);
if (RendRand.GenPRN() < .25)
	{
	if (Width > 641)
		{
		MemFuck += 69875342;
		return (0);
		} // if
	if (Height > 455)
		{
		MemFuck += 98543290;
		return (0);
		} // if
	} // if
} // temp scope
#endif // WCS_BUILD_DEMO

return (Success);

} // Renderer::RenderDEM

/*===========================================================================*/

#if defined DEBUG_POLYGONS_TO_VECTOR
bool PrintToVector;
#endif // DEBUG_POLYGONS_TO_VECTOR

unsigned long Renderer::FrdDataRows[10];
VectorNode *Renderer::SubNodes[129][129];

// When this function is called RenderIt must be pre-set to true if the actual render status is to be 
// determined and false if it is of no consequence
int Renderer::PrepVectorPoly(unsigned long *VertNum, unsigned long PolyNumber, bool OverrideCulling,
	VectorPolygon *DEMVecPoly, VectorPolygonListDouble *DEMPolyList, double CurTime, bool &RenderIt)
{
double CellArea;
int Success = 1, NodeCt, FractalLevel;
VectorPolygon *NewPoly = NULL;
VectorNode *NewNodes[4];
VectorPolygonListDouble *VPCellList = NULL, **VPCellListPtr, *CurVPList, *HeadOfEffects, *ShortPolyList, *CurPolyList;
GeneralEffect *TerrainEffect;
PolygonBoundingBox BoundLimits;
char PErr[128];

PErr[0] = 0;

#ifdef DEBUG_POLYGONS_TO_VECTOR
if (PolyNumber == 31184)
	PrintToVector = true;
else
	PrintToVector = false;
#endif // DEBUG_POLYGONS_TO_VECTOR

if (! CurDEM->VPData)
	{
	if (! CurDEM->AllocVPData())
		{
		return (0);
		} // if
	} // if
if (CurDEM->VPData[PolyNumber])	// this will prevent the fractal depth from changing during a frame or animation 
	{							// but that's probably a good thing anyway.
	// this block is hit coming from two different places. The amount of effort required before returning depends 
	// on whether it is necessary to evaluate the renderability status. If OverrideCulling is false then we do care.
	if (! OverrideCulling)
		RenderIt = GetPolygonBlockRenderStatus(CurDEM->VPData[PolyNumber]);
	return (Success);	
	} // if

RenderIt = false;
// OverrideCulling tells us to convert the DEM cell to a vector polygon whether it is visible or not
// as long as the fractal level is >= 0 since it may be visible somewhere in the animation and the vec poly
// is needed for phong shading of some other cell being rendered now
// TestVisibility argument is false if OverrideCulling is true
// if ! OverrideCulling, RenderIt will only be true if the cell proves to be visible now
FractalLevel = GetFractalLevel(VertNum, PolyNumber, RenderIt, ! OverrideCulling);

if (RenderIt || (OverrideCulling && FractalLevel >= 0))
	{
	// make four vector nodes out of the four DEM vertices
	for (NodeCt = 0; NodeCt < 4; ++NodeCt)
		{
		NewNodes[NodeCt] = NULL;
		if (Success)
			{
			if (NewNodes[NodeCt] = new VectorNode())
				{
				NewNodes[NodeCt]->Lat = CurDEM->Vertices[VertNum[NodeCt]].Lat;
				NewNodes[NodeCt]->Lon = CurDEM->Vertices[VertNum[NodeCt]].Lon;
				NewNodes[NodeCt]->Elev = CurDEM->Vertices[VertNum[NodeCt]].Elev;
				NewNodes[NodeCt]->FlagSet(
					NodeCt == 0 ? (unsigned short)WCS_VECTORNODE_FLAG_SWCORNER:
					NodeCt == 1 ? (unsigned short)WCS_VECTORNODE_FLAG_NWCORNER:
					NodeCt == 2 ? (unsigned short)WCS_VECTORNODE_FLAG_NECORNER:
					(unsigned short)WCS_VECTORNODE_FLAG_SECORNER);
				} // if
			else
				Success = 0;
			} // if
		} // for
	#ifdef DEBUG_POLYGONS_TO_VECTOR
	//sprintf(DebugStr, "%d prepping\n", PolyNumber);
	//OutputDebugString(DebugStr);
	#endif // DEBUG_POLYGONS_TO_VECTOR

	// make a vector polygon to hold the new vertices
	if (Success)
		{
		if (NewPoly = new VectorPolygon(NewNodes, 4, DEMVecPoly))
			{
			TerrainEffect = NewPoly->MyEffects->MyEffect;
			if (NewPoly->SetBoundingBox())
				{
				bool LocalSuccess = true;
				BoundLimits.Copy(NewPoly->BBox);
				ShortPolyList = DEMPolyList ? EvalEffects->CreateBoundedPolygonList(DEMPolyList, NewPoly->BBox, LocalSuccess): NULL;
				#ifdef DEBUG_POLYGONS_TO_VECTOR
				//if (PrintToVector)
				//	{
				//	EvalEffects->OutputVectorsToDatabase(DEMPolyList, "DEM's Polygons", NULL);
				//	LocalSuccess = 0;
				//	} // if
				#endif // DEBUG_POLYGONS_TO_VECTOR
				if (! LocalSuccess)
					{
					Success = 0;
					UserMessageOK("Render DEM", "Out of memory creating list of cell-bounded effects!");
					} // if
				if (Success)
					{
					// subdivide cell polygon if necessary
					// create VectorPolygonListDouble of cell polygons
					// make local copies of effect vector polygons from ShortPolyList
					// merge with effect vector polygons
					// purge list of vector polygons that do not have planet opts effect
					// render only the polygons that have the planet opts effect

					if (VPCellList = new VectorPolygonListDouble())
						{
						VPCellList->MyPolygon = NewPoly;
						// Moved area calc to after intersection with polygons since it can change if points are moved
						//CellArea = VPCellList->CalculateAreaOutsidePart();
						// NULL NewPoly so it isn't deleted independent of the VPCellList
						NewPoly = NULL;
						VPCellListPtr = (VectorPolygonListDouble **)&VPCellList->NextPolygonList;
						#ifndef MAKENOCLONES
						// clones are only relevant if clones are going to be made
						for (CurPolyList = ShortPolyList; CurPolyList; CurPolyList = (VectorPolygonListDouble *)CurPolyList->NextPolygonList)
							{
							CurPolyList->MyPolygon->SetLinkedPolygonClonesToNULL();
							} // for
						#endif // MAKENOCLONES
						for (CurPolyList = ShortPolyList; CurPolyList; CurPolyList = (VectorPolygonListDouble *)CurPolyList->NextPolygonList)
							{
							if (CurPolyList->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_ENCLOSED) && CurPolyList->MyPolygon->TrulyEncloses(VPCellList->MyPolygon))
								{
								if (! VPCellList->MyPolygon->SupplementEffects(CurPolyList->MyPolygon))
									{
									Success = 0;
									break;
									} // if
								} // if
							else if (*VPCellListPtr = new VectorPolygonListDouble())
								{
								#ifndef MAKENOCLONES
								// this will set CloneOfThis to the new polygon for the original copied polygon
								// Only applies if clones were made
								if (! (*VPCellListPtr)->MakeVectorPolygon(CurPolyList->MyPolygon, NULL, false))
									{
									Success = 0;
									break;
									} // if
								#else // MAKENOCLONES
								// if not using clones, just set a pointer to the polygon in the active list
								(*VPCellListPtr)->MyPolygon = CurPolyList->MyPolygon;
								#endif // MAKENOCLONES
								VPCellListPtr = (VectorPolygonListDouble **)&(*VPCellListPtr)->NextPolygonList;
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // for
						
						if (Success)
							{
							if (HeadOfEffects = (VectorPolygonListDouble *)VPCellList->NextPolygonList)
								{
								#ifndef MAKENOCLONES
								for (CurVPList = HeadOfEffects; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
									{
									CurVPList->MyPolygon->ReplaceNodeLinkPolygons();
									} // for
								#else // MAKENOCLONES
								// this causes all the original vector polygons to lose the "shared node" flag
								// wherever two of them are linked together. Subsequent merges lose their way
								// trying to follow loops when nodes are not marked as shared due to the function
								// DisableSharedEdges relying on that flag.
								//for (CurVPList = HeadOfEffects; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
								//	{
								//	CurVPList->MyPolygon->ClearIntersectionFlags();
								//	} // for
								#endif // MAKENOCLONES
	#ifdef DEBUG_POLYGONS_TO_VECTOR
	if (PolyNumber == 31184)
		PrintToVector = true;
	else
		PrintToVector = false;
	#endif // DEBUG_POLYGONS_TO_VECTOR
//							#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//							DevCounter[13]++;
//							#endif // FRANK or GARY
								//if (PrintToVector)
								//	{
								//	EvalEffects->OutputOneVectorToDatabase((VectorPolygonListDouble *)VPCellList->NextPolygonList, VPCellList->NextPolygonList->MyPolygon->TotalNumNodes, true);
								//	EvalEffects->OutputVectorsToDatabase(VPCellList, "Intersect terrain cell input", NULL);
								//	Success = 0;
								//	} // if
								if ((Success = EvalEffects->IntersectPolygonsWithTerrainCell(VPCellList)) && VPCellList)
									{
									#ifdef DEBUG_POLYGONS_TO_VECTOR
									//if (PrintToVector)
									//	{
									//	EvalEffects->OutputVectorsToDatabase(VPCellList, "Intersect terrain cell output", NULL);
									//	Success = 0;
									//	} // if
									#endif // DEBUG_POLYGONS_TO_VECTOR
									CellArea = VPCellList->CalculateAreaOutsidePart();
									if (Success = EvalEffects->MergePolygons(VPCellList, CellArea, TerrainEffect, &BoundLimits, false, false))
										{
										CullNonTerrainPolys(VPCellList, TerrainEffect);
										#ifdef DEBUG_POLYGONS_TO_VECTOR
										//if (PrintToVector)
										//	{
										//	EvalEffects->OutputVectorsToDatabase(VPCellList, "Non-terrain cull output", NULL);
										//	Success = 0;
										//	} // if
										#endif // DEBUG_POLYGONS_TO_VECTOR
										HeadOfEffects = (VectorPolygonListDouble *)VPCellList->NextPolygonList;
										} // if
									} // if
								} // if
							if (Success && FractalLevel > 0)
								{
	#ifdef DEBUG_POLYGONS_TO_VECTOR
	//if (PolyNumber == 145)
	//	PrintToVector = true;
	//else
	//	PrintToVector = false;
	#endif // DEBUG_POLYGONS_TO_VECTOR
								if (HeadOfEffects)
									{
									// Need to recreate and subdivide the original polygon and merge with this remaining list
									// Do not discriminate against polygons that do not have a terrain effect
									// Use the form: MergePolygons(VPCellList, NULL)
									// make four vector nodes out of the four DEM vertices
									for (NodeCt = 0; NodeCt < 4; ++NodeCt)
										{
										NewNodes[NodeCt] = NULL;
										if (Success)
											{
											if (NewNodes[NodeCt] = new VectorNode())
												{
												NewNodes[NodeCt]->Lat = CurDEM->Vertices[VertNum[NodeCt]].Lat;
												NewNodes[NodeCt]->Lon = CurDEM->Vertices[VertNum[NodeCt]].Lon;
												NewNodes[NodeCt]->Elev = CurDEM->Vertices[VertNum[NodeCt]].Elev;
												NewNodes[NodeCt]->FlagSet(
													NodeCt == 0 ? (unsigned short)WCS_VECTORNODE_FLAG_SWCORNER:
													NodeCt == 1 ? (unsigned short)WCS_VECTORNODE_FLAG_NWCORNER:
													NodeCt == 2 ? (unsigned short)WCS_VECTORNODE_FLAG_NECORNER:
													(unsigned short)WCS_VECTORNODE_FLAG_SECORNER);
												} // if
											else
												Success = 0;
											} // if
										} // for
									// make a vector polygon to hold the new vertices
									if (Success)
										{
										if (NewPoly = new VectorPolygon(NewNodes, 4, NULL))
											{
											VectorPolygonListDouble *TempCellList;
											if (TempCellList = new VectorPolygonListDouble())
												{
												TempCellList->MyPolygon = NewPoly;
												// NULL NewPoly so it isn't deleted independent of the VPCellList
												NewPoly = NULL;
												HeadOfEffects = VPCellList;
												TempCellList->NextPolygonList = VPCellList;
												VPCellList = TempCellList;
												if (Success = SubdivideTerrainPolygon(VPCellList, PolyNumber, FractalLevel))
													{
													#ifdef MAKENOCLONES
													// this causes all the original vector polygons to lose the "shared node" flag
													// wherever two of them are linked together. Subsequent merges lose their way
													// trying to follow loops when nodes are not marked as shared due to the function
													// DisableSharedEdges relying on that flag.
													//for (CurVPList = HeadOfEffects; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
													//	{
													//	CurVPList->MyPolygon->ClearIntersectionFlags();
													//	} // for
													#endif // MAKENOCLONES
													if ((Success = EvalEffects->IntersectPolygonsWithMultipleTerrainCells(VPCellList, HeadOfEffects)) && VPCellList)
														{
														#ifdef DEBUG_POLYGONS_TO_VECTOR
														//if (PrintToVector)
														//	{
														//	EvalEffects->OutputVectorsToDatabase(VPCellList, "Multi-cell intersect output", NULL);
														//	} // if
														#endif // DEBUG_POLYGONS_TO_VECTOR
														Success = EvalEffects->MergePolygons(VPCellList, CellArea, NULL, &BoundLimits, false, false);
														} // if
													// don't use this again since it might have been elliminated by the intersection function
													HeadOfEffects = NULL;
													} // if
												} // if TempCellList
											else
												Success = 0;
											} // if NewPoly
										else
											{
											for (NodeCt = 0; NodeCt < 4; ++NodeCt)
												{
												if (NewNodes[NodeCt])
													delete NewNodes[NodeCt];
												} // for
											} // else
										} // if Success
									} // if HeadOfEffects
								else
									{
									// subdivide the remaining one polygon, assigning the same effects
									// to all the subdivisions
									Success = SubdivideTerrainPolygon(VPCellList, PolyNumber, FractalLevel);
									} // else
								} // if Success && FractalLevel > 0

							#ifdef DEBUG_POLYGONS_TO_VECTOR
							//if (PolyNumber == 54716)
							//	PrintToVector = true;
							//else
							//	PrintToVector = false;
							if (PrintToVector)
								{
								//EvalEffects->OutputVectorsToDatabase(VPCellList, "VPCellList before", NULL);//, &BoundLimits);
								//EvalEffects->OutputVectorsToDatabase(DEMPolyList, "DEM's Polygons before", NULL);//&BoundLimits);
								//Success = 0;
								} // if
							#endif // DEBUG_POLYGONS_TO_VECTOR
							if (Success)
								{
								long CurVPCt;
								// all polygons should be terrain polygons now
								VectorPolygonListDouble *TriVPList, *PrevVPList = NULL, *ListEnd;
								for (CurVPCt = 0, CurVPList = VPCellList; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList, ++CurVPCt)
									{
									#ifdef MAKENOCLONES
									if (CurVPList->MyPolygon->ImAnOriginalPolygon)
										{
										VectorPolygon *StashPoly = CurVPList->MyPolygon;
										// copy the polygon and put the copy in CurVPList for triangulation
										// make sure all links are copied
										// cross-links are not made between original and copy
										// remove all links in original that aren't to original polygons
										if (! CurVPList->MakeVectorPolygon(StashPoly, NULL, true))
											{
											//sprintf(PErr, "Error copying polygon %d/%d.\nPlease report to 3D Nature. Thanks", PolyNumber, CurVPCt);
											//UserMessageOK("Polygon Error", PErr);
											Success = 0;
											break;
											} // if
										StashPoly->RemoveEffect(TerrainEffect);
										StashPoly->RemoveTerrainEffectLinks(TerrainEffect);
										} // if
									#endif // MAKENOCLONES
									if (CurVPList->MyPolygon->TotalNumNodes > 3)
										{
										bool BoolSuccess = true;
										
										#ifdef DEBUG_POLYGONS_TO_VECTOR
										//if (PrintToVector)
										//	{
										//	EvalEffects->OutputOneVectorToDatabase(CurVPList, CurVPList->MyPolygon->TotalNumNodes, true);
										//	sprintf(DebugStr, "%d/%d triangulating ", PolyNumber, CurVPCt);
										//	OutputDebugString(DebugStr);
										//	} // if
										#endif // DEBUG_POLYGONS_TO_VECTOR
										// triangulate and replace the list CurVPList with the new triangulated list
										if (TriVPList = CurVPList->Triangulate(BoolSuccess))
											{
											for (ListEnd = TriVPList; ListEnd->NextPolygonList; ListEnd = (VectorPolygonListDouble *)ListEnd->NextPolygonList)
												;	// nothing to do
											ListEnd->NextPolygonList = CurVPList->NextPolygonList;
											if (! CurVPList->MyPolygon)
												{
												if (PrevVPList)
													PrevVPList->NextPolygonList = TriVPList;
												else
													VPCellList = TriVPList;
												delete CurVPList;
												} // if
											else
												CurVPList->NextPolygonList = TriVPList;
											CurVPList = ListEnd;
											if (! (Success = BoolSuccess))
												{
												sprintf(PErr, "Error triangulating polygon %d/%d.\nPlease report to 3D Nature. Thanks", PolyNumber, CurVPCt);
												UserMessageOK("Polygon Error", PErr);
												Success = 0;
												break;
												} // if
											} // if
										else
											{
											sprintf(PErr, "Error triangulating polygon %d/%d.\nPlease report to 3D Nature. Thanks", PolyNumber, CurVPCt);
											UserMessageOK("Polygon Error", PErr);
											Success = 0;
											break;
											} // else
										#ifdef DEBUG_POLYGONS_TO_VECTOR
										//if (PrintToVector)
										//	{
										//	sprintf(DebugStr, "%d done\n", PolyNumber);
										//	OutputDebugString(DebugStr);
										//	} // if
										#endif // DEBUG_POLYGONS_TO_VECTOR
										} // if
									PrevVPList = CurVPList;
									} // for
								CurDEM->VPData[PolyNumber] = VPCellList;
								#ifdef DEBUG_POLYGONS_TO_VECTOR
								//if (PrintToVector)
									{
									//VectorPolygonListDouble *CurList;
									//char PolyNumStr[24];
									//long PolyCt;
									//for (PolyCt = 0, CurList = VPCellList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList, ++PolyCt)
									//	{
									//	CurList->GetPolygon()->SetPolyNumber(PolyCt);
									//	} // for
									//EvalEffects->ComparePolygonsInLists(VPCellList, DEMPolyList);
									//sprintf(PolyNumStr, "%d", PolyNumber);
									//EvalEffects->OutputVectorsToDatabase(VPCellList, PolyNumStr, NULL);//, &BoundLimits);
									//EvalEffects->OutputVectorsToDatabase(DEMPolyList, "DEM's Polygons after", NULL);//, &BoundLimits);
									//Success = 0;
									} // if
								#endif // DEBUG_POLYGONS_TO_VECTOR
								} // if Success
							} // if Success
						} // if VPCellList
					else
						Success = 0;
					} // if

				// Delete the original list of cell-covered polygons
				for (VectorPolygonListDouble *TempFlags = ShortPolyList; TempFlags; TempFlags = ShortPolyList)
					{
					ShortPolyList = (VectorPolygonListDouble *)ShortPolyList->NextPolygonList;
					#ifndef MAKENOCLONES
					TempFlags->MyPolygon->SetClonePointer(NULL);
					#endif // MAKENOCLONES
					delete TempFlags;
					} // for
				} // if
			// cleanup. In case of failure somewhere above NewPoly needs to be deleted separately.
			if (NewPoly)
				delete NewPoly;
			} // if
		} // if
	else
		{
		for (NodeCt = 0; NodeCt < 4; ++NodeCt)
			{
			if (NewNodes[NodeCt])
				delete NewNodes[NodeCt];
			} // for
		} // else
	} // if

#if defined DEBUG_POLYGONS_TO_VECTOR
PrintToVector = false;
#endif // DEBUG_POLYGONS_TO_VECTOR
//#ifdef _DEBUG
if (! Success && ! PErr[0])
	{
	sprintf(PErr, "Error in PrepVectorPoly polygon %d.\nPlease report to 3D Nature. Thanks", PolyNumber);
	UserMessageOK("Polygon Error", PErr);
	} // if
//#endif // _DEBUG

return (Success);

} // Renderer::PrepVectorPoly

/*===========================================================================*/

enum
	{
	WCS_ADJOINING_NORTHWEST,
	WCS_ADJOINING_NORTH,
	WCS_ADJOINING_NORTHEAST,
	WCS_ADJOINING_EAST,
	WCS_ADJOINING_SOUTHEAST,
	WCS_ADJOINING_SOUTH,
	WCS_ADJOINING_SOUTHWEST,
	WCS_ADJOINING_WEST
	}; // adjoiing cell numbers

int Renderer::TuneVectorPoly(unsigned long PolyNumber, unsigned long PolysPerRow, VectorPolygon *DEMVecPoly, VectorPolygonListDouble *DEMPolyList, 
	double CurTime)
{
double DummySumOfAllCoverages;
int Success = 1, AdjoiningPolyCt;
VectorPolygonListDouble *DataSource;
unsigned long int AdjoiningPolyNumber[8], CurLr, CurLc, VNum[4], NodeCt, PolyCt = 0;
VectorPolygonListDouble *AdjoiningPolyData[8];
VectorNode *CurNode, *CopyElevNode, *CopyNormalNode, *CopyOtherDataNode;
EffectJoeList *CurEffect;
bool RenderIt, NeedNormals, NeedOtherData;

CompleteBoundingPolygons(AdjoiningPolyData, AdjoiningPolyNumber, PolyNumber, PolysPerRow);

// prepare vector polygons for adjoining DEM cells
for (AdjoiningPolyCt = 0; Success && AdjoiningPolyCt < 8; ++AdjoiningPolyCt)
	{
	if (! AdjoiningPolyData[AdjoiningPolyCt] && AdjoiningPolyNumber[AdjoiningPolyCt] != ~0)
		{
		CurLr = AdjoiningPolyNumber[AdjoiningPolyCt] / PolysPerRow;
		CurLc = AdjoiningPolyNumber[AdjoiningPolyCt] - (CurLr * PolysPerRow);
		VNum[0] = CurLc + CurDEM->pLatEntries * CurLr;
		VNum[1] = VNum[0] + 1;
		VNum[2] = VNum[0] + CurDEM->pLatEntries + 1;
		VNum[3] = VNum[0] + CurDEM->pLatEntries;
		// override culling = true
		RenderIt = false; // tells PrepVectorPoly not to evaluate render status since we don't need it here
		Success = PrepVectorPoly(VNum, AdjoiningPolyNumber[AdjoiningPolyCt], true, DEMVecPoly, DEMPolyList, CurTime, RenderIt);
		AdjoiningPolyData[AdjoiningPolyCt] = CurDEM->VPData[AdjoiningPolyNumber[AdjoiningPolyCt]];
		} // for
	} // for

// finalize pre-rendering preparation for each vector polygon in this terrain cell
for (DataSource = CurDEM->VPData[PolyNumber]; Success && DataSource; DataSource = (VectorPolygonListDouble *)DataSource->NextPolygonList)
	{
	// triangulate the polygon - note that it may not be purely convex
	if (DataSource->MyPolygon->TotalNumNodes != 3)
		{
		if (! UserMessageYN("Render Error", "Polygon contains more or less than three vertices. Polygon skipped. Continue rendering?"))
			Success = 0;
		continue;
		} // if

	for (NodeCt = 0, CurNode = DataSource->MyPolygon->PolyFirstNode(); Success && NodeCt < DataSource->MyPolygon->TotalNumNodes; CurNode = CurNode->NextNode, ++NodeCt)
		{
		if (CurNode->NodeData || CurNode->AddNodeData())
			{
			if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) && ! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ADJOININGLINKSMADE))
				{
				Success = LinkAdjoiningPolygons(AdjoiningPolyData, AdjoiningPolyNumber, DataSource->MyPolygon, CurNode);
				} // if
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ELEVEVALUATED) ||
				! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED))
				{
				CopyElevNode = CopyNormalNode = NULL;
				for (VectorNodeLink *SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
					{
					if (SearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_ELEVEVALUATED))
						{
						CopyElevNode = SearchNodeLink->MyNode;
						if (CopyNormalNode)
							break;
						} // if
					if (SearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED))
						{
						CopyNormalNode = SearchNodeLink->MyNode;
						if (CopyElevNode)
							break;
						} // if
					} // for
				if (CopyElevNode)
					{
					double StashElev = CurNode->Elev;
					CurNode->CopyElevationData(CopyElevNode);
					#ifdef WCS_RESET_WATER_FLAGS_ON_ELEV_CHANGE
					if ( CurNode->Elev != StashElev)
						CurNode->FlagClear( WCS_VECTORNODE_FLAG_WATEREVALUATED | WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED | WCS_VECTORNODE_FLAG_BEACHEVALUATED );
					#endif // WCS_RESET_WATER_FLAGS_ON_ELEV_CHANGE
					if (TerraffectorsExist)
						{
						// elevations were not set before now so terraffectors were not evaluated for this node previously.
						// If materials already exist then this node is not going to be subject to material sharing from another node,
						// hence ecosystems and such must be evaluated. Since the elevation has been copied, the full tfx evaluation
						// is not necessary but ecosystems for terraffectors need to be evaluated.
						if (CurNode->NodeData->Materials || CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
							{
							for (CurEffect = DataSource->MyPolygon->MyEffects; CurEffect; CurEffect = CurEffect->Next)
								{
								if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
									{
									if (! EffectsBase->EvalTerraffectors(&RendData, DataSource->MyPolygon->MyEffects, CurNode, true, DummySumOfAllCoverages = 0.0))
										return (0);
									break;
									} // if
								} // for
							CurNode->FlagSet(WCS_VECTORNODE_FLAG_TFXECOEVALUATED);
							} // if
						} // if
					} // if
				if (CopyNormalNode)
					{
					CurNode->CopyNormalData(CopyNormalNode);
					} // if

				// if linked nodes have elevation data already calculated save the trouble and copy it
				// otherwise interpolate elevation, relative elevation, terraffect the elevation
				// calculate surface normal, slope, aspect, 
				// ecosystems might be different for linked nodes though since they could refer to the ecosystem of
				// an ecosystem effect in a different bounded polygon ecosystem.
				if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_ELEVEVALUATED)))
					Success = AnalyzeNodeElevation(DEMVecPoly, CurNode, DataSource->MyPolygon, false); // CellUnknown = false
				} // if
			} // if
		else
			Success = 0;
		} // for
	if (Success)
		{
		for (NodeCt = 0, CurNode = DataSource->MyPolygon->PolyFirstNode(); Success && NodeCt < DataSource->MyPolygon->TotalNumNodes; CurNode = CurNode->NextNode, ++NodeCt)
			{
			CopyOtherDataNode = CopyNormalNode = NULL;
			if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
				{
				NeedNormals = ! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED);
				NeedOtherData = (! CurNode->NodeData->Materials) && (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_NONELEVATIONITEMSREQUIRED) != WCS_VECTORNODE_FLAG_NONELEVATIONITEMSREQUIRED);
				if (NeedNormals || NeedOtherData)
					{
					for (VectorNodeLink *SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
						{
						if (NeedOtherData && (SearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_NONELEVATIONITEMSREQUIRED) ==
							WCS_VECTORNODE_FLAG_NONELEVATIONITEMSREQUIRED))
							{
							CopyOtherDataNode = SearchNodeLink->MyNode;
							if (CopyNormalNode || ! NeedNormals)
								break;
							} // if
						if (NeedNormals && SearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED))
							{
							CopyNormalNode = SearchNodeLink->MyNode;
							if (CopyOtherDataNode || ! NeedOtherData)
								break;
							} // if
						} // for
					if (CopyOtherDataNode)
						{
						CurNode->CopyNonElevationNormalData(CopyOtherDataNode);
						} // if
					if (CopyNormalNode)
						{
						CurNode->CopyNormalData(CopyNormalNode);
						} // if
					} // if
				} // if
			if (! CopyNormalNode)
				{
				if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED)))
					Success = AnalyzeNodeNormals(DEMVecPoly, CurNode, DataSource->MyPolygon, false);
				} // if
			if (! CopyOtherDataNode)
				{
				if (TerraffectorsExist)
					{
					if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_TFXECOEVALUATED)))
						{
						for (CurEffect = DataSource->MyPolygon->MyEffects; CurEffect; CurEffect = CurEffect->Next)
							{
							if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
								{
								if (! EffectsBase->EvalTerraffectors(&RendData, DataSource->MyPolygon->MyEffects, CurNode, true, DummySumOfAllCoverages = 0.0))
									return (0);
								break;
								} // if
							} // for
						} // if
					} // if
				else
					CurNode->FlagSet(WCS_VECTORNODE_FLAG_TFXECOEVALUATED);
				if (WaterExists)
					{
					if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_WATEREVALUATED)))
						Success = AnalyzeNodeWater(DEMVecPoly, CurNode, DataSource->MyPolygon, false, true);
					} // if
				else
					CurNode->FlagSet(WCS_VECTORNODE_FLAG_WATEREVALUATED);
				if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_EFFECTSEVALUATED)))
					Success = AnalyzeNodeData(DEMVecPoly, CurNode, DataSource->MyPolygon, false);
				if (WaterExists)
					{
					if (Success && (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED)))
						Success = AnalyzeNodeWaterNormals(DEMVecPoly, CurNode, DataSource->MyPolygon, false);
					} // if
				else
					CurNode->FlagSet(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED);
				} // if
			else if (! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED))	// might ahve been cleared in CopyNonElevationNormalData if water normal was 0.0
				{
				if (WaterExists)
					Success = AnalyzeNodeWaterNormals(DEMVecPoly, CurNode, DataSource->MyPolygon, false);
				else
					CurNode->FlagSet(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED);
				} // else if
			} // for
		} // if
	// render the polygon
	#ifndef _OPENMP
	if (Success)
		{
		Success = PrepPolyToRender(DataSource->MyPolygon);
		} // if
	#endif // _OPENMP
	++PolyCt;
	} // for

return (Success);

} // Renderer::TuneVectorPoly

/*===========================================================================*/

int Renderer::RenderVectorPoly(unsigned long PolyNumber)
{
int Success = 1;
VectorPolygonListDouble *DataSource;
// this can only be done if TextureData is made thread-independent
/*
#ifdef _OPENMP
VectorPolygonListDouble *SourceArray[8];
long PolysToRender, PolyToRenderNow;

DataSource = CurDEM->VPData[PolyNumber];
while (DataSource && Success)
	{
	for (PolysToRender = 0; DataSource && PolysToRender < 8; DataSource = (VectorPolygonListDouble *)DataSource->NextPolygonList, ++PolysToRender)
		{
		SourceArray[PolysToRender] = DataSource;
		} // for
	#pragma omp parallel for
	for (PolyToRenderNow = 0; PolyToRenderNow < PolysToRender; ++PolyToRenderNow)
		{
		if (! PrepPolyToRender(SourceArray[PolyToRenderNow]->MyPolygon))
			Success = 0;
		} // for
	} // while
#else // _OPENMP
*/
for (DataSource = CurDEM->VPData[PolyNumber]; Success && DataSource; DataSource = (VectorPolygonListDouble *)DataSource->NextPolygonList)
	{
	Success = PrepPolyToRender(DataSource->MyPolygon);
	} // for
//#endif // _OPENMP

return (Success);

} // Renderer::RenderVectorPoly

/*===========================================================================*/

void Renderer::CompleteBoundingPolygons(VectorPolygonListDouble **AdjoiningPolyData,
	unsigned long int *AdjoiningPolyNumber, unsigned long PolyNumber, unsigned long PolysPerRow)
{

// find the number of adjoining cells
// from RenderDEM():
// CurDEM->Lr = PolyArray[Ct].PolyNumber / PolysPerRow;
// CurDEM->Lc = PolyArray[Ct].PolyNumber - (CurDEM->Lr * PolysPerRow);
// so CurDEM->Lr is the southwest vertex row from the west
// so CurDEM->Lc is the southwest vertex column from the south
// We are looking for 8 adjoinging polygon numbers

if (CurDEM->Lr > 0)
	{
	AdjoiningPolyNumber[WCS_ADJOINING_WEST] = PolyNumber - PolysPerRow;
	AdjoiningPolyData[WCS_ADJOINING_WEST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_WEST]];
	if (CurDEM->Lc > 0)
		{
		AdjoiningPolyNumber[WCS_ADJOINING_SOUTHWEST] = PolyNumber - 1 - PolysPerRow;
		AdjoiningPolyData[WCS_ADJOINING_SOUTHWEST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_SOUTHWEST]];
		} // if
	else
		{
		// will have to get this data from the terrain manager
		AdjoiningPolyNumber[WCS_ADJOINING_SOUTHWEST] = ~0;
		AdjoiningPolyData[WCS_ADJOINING_SOUTHWEST] = NULL;
		} // else

	if (CurDEM->Lc < (long)CurDEM->pLatEntries - 2)
		{
		AdjoiningPolyNumber[WCS_ADJOINING_NORTHWEST] = PolyNumber + 1 - PolysPerRow;
		AdjoiningPolyData[WCS_ADJOINING_NORTHWEST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_NORTHWEST]];
		} // if
	else
		{
		// will have to get this data from the terrain manager
		AdjoiningPolyNumber[WCS_ADJOINING_NORTHWEST] = ~0;
		AdjoiningPolyData[WCS_ADJOINING_NORTHWEST] = NULL;
		} // else
	} // if
else
	{
	// will have to get this data from the terrain manager
	AdjoiningPolyNumber[WCS_ADJOINING_WEST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_WEST] = NULL;
	AdjoiningPolyNumber[WCS_ADJOINING_SOUTHWEST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_SOUTHWEST] = NULL;
	AdjoiningPolyNumber[WCS_ADJOINING_NORTHWEST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_NORTHWEST] = NULL;
	} // else

if (CurDEM->Lr < (long)CurDEM->pLonEntries - 2)
	{
	AdjoiningPolyNumber[WCS_ADJOINING_EAST] = PolyNumber + PolysPerRow;
	AdjoiningPolyData[WCS_ADJOINING_EAST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_EAST]];
	if (CurDEM->Lc > 0)
		{
		AdjoiningPolyNumber[WCS_ADJOINING_SOUTHEAST] = PolyNumber - 1 + PolysPerRow;
		AdjoiningPolyData[WCS_ADJOINING_SOUTHEAST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_SOUTHEAST]];
		} // if
	else
		{
		// will have to get this data from the terrain manager
		AdjoiningPolyNumber[WCS_ADJOINING_SOUTHEAST] = ~0;
		AdjoiningPolyData[WCS_ADJOINING_SOUTHEAST] = NULL;
		} // else

	if (CurDEM->Lc < (long)CurDEM->pLatEntries - 2)
		{
		AdjoiningPolyNumber[WCS_ADJOINING_NORTHEAST] = PolyNumber + 1 + PolysPerRow;
		AdjoiningPolyData[WCS_ADJOINING_NORTHEAST] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_NORTHEAST]];
		} // if
	else
		{
		// will have to get this data from the terrain manager
		AdjoiningPolyNumber[WCS_ADJOINING_NORTHEAST] = ~0;
		AdjoiningPolyData[WCS_ADJOINING_NORTHEAST] = NULL;
		} // else
	} // if
else
	{
	// will have to get this data from the terrain manager
	AdjoiningPolyNumber[WCS_ADJOINING_EAST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_EAST] = NULL;
	AdjoiningPolyNumber[WCS_ADJOINING_SOUTHEAST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_SOUTHEAST] = NULL;
	AdjoiningPolyNumber[WCS_ADJOINING_NORTHEAST] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_NORTHEAST] = NULL;
	} // else

if (CurDEM->Lc > 0)
	{
	AdjoiningPolyNumber[WCS_ADJOINING_SOUTH] = PolyNumber - 1;
	AdjoiningPolyData[WCS_ADJOINING_SOUTH] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_SOUTH]];
	} // if
else
	{
	// will have to get this data from the terrain manager
	AdjoiningPolyNumber[WCS_ADJOINING_SOUTH] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_SOUTH] = NULL;
	} // else

if (CurDEM->Lc < (long)CurDEM->pLatEntries - 2)
	{
	AdjoiningPolyNumber[WCS_ADJOINING_NORTH] = PolyNumber + 1;
	AdjoiningPolyData[WCS_ADJOINING_NORTH] = CurDEM->VPData[AdjoiningPolyNumber[WCS_ADJOINING_NORTH]];
	} // if
else
	{
	// will have to get this data from the terrain manager
	AdjoiningPolyNumber[WCS_ADJOINING_NORTH] = ~0;
	AdjoiningPolyData[WCS_ADJOINING_NORTH] = NULL;
	} // else

} // Renderer::CompleteBoundingPolygons

/*===========================================================================*/

int Renderer::LinkAdjoiningPolygons(VectorPolygonListDouble **AdjoiningPolyData, 
	unsigned long int *AdjoiningPolyNumber, VectorPolygon *MyPolygon, VectorNode *CurNode)
{
int Success = 1;
unsigned long int SearchNodeCt;
VectorPolygonListDouble *SearchPolyList;
VectorNode *SearchNode;
unsigned long FlagCheck;

// link to adjoinging cells
// if edge or corner flags are set for a node, find linked node in adjoining cells
// Set a flag so that the node link can be dissolved after rendering this polygon
// flags WCS_VECTORNODE_FLAG_EASTEDGE
// flags WCS_VECTORNODE_FLAG_WESTEDGE
// flags WCS_VECTORNODE_FLAG_NORTHEDGE
// flags WCS_VECTORNODE_FLAG_SOUTHEDGE
// NW corner = WCS_VECTORNODE_FLAG_NWCORNER
// SW corner = WCS_VECTORNODE_FLAG_SWCORNER
// NE corner = WCS_VECTORNODE_FLAG_NECORNER
// SE corner = WCS_VECTORNODE_FLAG_SECORNER
// any edge or corner = WCS_VECTORNODE_FLAG_BOUNDARYNODE

if (FlagCheck = CurNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE))
	{
	if (FlagCheck == WCS_VECTORNODE_FLAG_NWCORNER)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_SWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_NORTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_NORTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_SECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_NORTHWEST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_NORTHWEST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_NECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_WEST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_WEST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		} // if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_SWCORNER)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_NWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_SOUTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_SOUTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_NECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_SOUTHWEST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_SOUTHWEST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_SECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_WEST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_WEST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_NECORNER)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_SECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_NORTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_NORTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_SWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_NORTHEAST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_NORTHEAST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_NWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_EAST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_EAST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_SECORNER)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_NECORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_SOUTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_SOUTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NECORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_NWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_SOUTHEAST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_SOUTHEAST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		// look for vertices of WCS_VECTORNODE_FLAG_SWCORNER in polygon list AdjoiningPolyData[WCS_ADJOINING_EAST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_EAST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SWCORNER)
					{
					if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
						{
						Success = 0;
						break;
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_EASTEDGE)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_WESTEDGE in polygon list AdjoiningPolyData[WCS_ADJOINING_EAST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_EAST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_WESTEDGE)
					{
					// see if lat lon are the same
					if (CurNode->SimilarPointLatLon(SearchNode, WCS_VECTORPOLYGON_NODECOORD_TOLERANCE))
					//if (CurNode->SamePointLatLon(SearchNode))
						{
						if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
							{
							Success = 0;
							break;
							} // if
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_WESTEDGE)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_EASTEDGE in polygon list AdjoiningPolyData[WCS_ADJOINING_WEST]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_WEST]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_EASTEDGE)
					{
					// see if lat lon are the same
					if (CurNode->SimilarPointLatLon(SearchNode, WCS_VECTORPOLYGON_NODECOORD_TOLERANCE))
					//if (CurNode->SamePointLatLon(SearchNode))
						{
						if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
							{
							Success = 0;
							break;
							} // if
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_NORTHEDGE)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_SOUTHEDGE in polygon list AdjoiningPolyData[WCS_ADJOINING_NORTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_NORTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_SOUTHEDGE)
					{
					// see if lat lon are the same
					if (CurNode->SimilarPointLatLon(SearchNode, WCS_VECTORPOLYGON_NODECOORD_TOLERANCE))
					//if (CurNode->SamePointLatLon(SearchNode))
						{
						if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
							{
							Success = 0;
							break;
							} // if
						} // if
					} // for
				} // for
			} // for
		} // else if
	else if (FlagCheck == WCS_VECTORNODE_FLAG_SOUTHEDGE)
		{
		// look for vertices of WCS_VECTORNODE_FLAG_NORTHEDGE in polygon list AdjoiningPolyData[WCS_ADJOINING_SOUTH]
		for (SearchPolyList = AdjoiningPolyData[WCS_ADJOINING_SOUTH]; Success && SearchPolyList; SearchPolyList = (VectorPolygonListDouble *)SearchPolyList->NextPolygonList)
			{
			for (SearchNodeCt = 0, SearchNode = SearchPolyList->MyPolygon->PolyFirstNode(); SearchNodeCt < SearchPolyList->MyPolygon->TotalNumNodes; SearchNode = SearchNode->NextNode, ++SearchNodeCt)
				{
				if (SearchNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE) == WCS_VECTORNODE_FLAG_NORTHEDGE)
					{
					// see if lat lon are the same
					if (CurNode->SimilarPointLatLon(SearchNode, WCS_VECTORPOLYGON_NODECOORD_TOLERANCE))
					//if (CurNode->SamePointLatLon(SearchNode))
						{
						if (! CurNode->AddCrossLinks(SearchNode, MyPolygon, SearchPolyList->MyPolygon))
							{
							Success = 0;
							break;
							} // if
						} // if
					} // for
				} // for
			} // for
		} // else if
	} // if

CurNode->FlagSet(WCS_VECTORNODE_FLAG_ADJOININGLINKSMADE);

return (Success);

} // Renderer::LinkAdjoiningPolygons

/*===========================================================================*/

int Renderer::AnalyzeNodeWater(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, 
	bool CellUnknown, bool AddBeachMaterials)
{
double SumOfAllCoverages = 0.0;
EffectJoeList *CurEffect;
bool EvalLakes = false, EvalStreams = false, BoolSuccess = true;

if (CurNode->NodeData || CurNode->AddNodeData())
	{
	// look for these effect types:
	// lake
	// stream
	// ecosystem
	// ground
	// snow
	for (CurEffect = VecPoly->MyEffects; CurEffect; CurEffect = CurEffect->Next)
		{
		if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LAKE)
			EvalLakes = true;
		else if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_STREAM)
			EvalStreams = true;
		} // for

	if (EvalLakes)
		BoolSuccess = EffectsBase->EvalLakes(&RendData, VecPoly->MyEffects, CurNode, SumOfAllCoverages, AddBeachMaterials);
	else
		BoolSuccess = EffectsBase->AddDefaultLake(&RendData, CurNode, SumOfAllCoverages, AddBeachMaterials);
	if (EvalStreams && BoolSuccess)
		BoolSuccess = EffectsBase->EvalStreams(&RendData, VecPoly->MyEffects, CurNode, SumOfAllCoverages, AddBeachMaterials);
		
	CurNode->FlagSet(WCS_VECTORNODE_FLAG_WATEREVALUATED);
	} // if
else
	BoolSuccess = false;

return (BoolSuccess ? 1: 0);

} // Renderer::AnalyzeNodeWater

/*===========================================================================*/

int Renderer::AnalyzeNodeData(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown)
{
double SumOfAllCoverages = 0.0, PlowedSnow = 0.0, DummySumOfAllCoverages = 0.0;
EffectJoeList *CurEffect;
MaterialList *Matty;
bool EvalEcosystems = false, EvalGrounds = false, EvalSnows = false, BoolSuccess = true;

if (CurNode->NodeData || CurNode->AddNodeData())
	{
	// look for these effect types:
	// ecosystem, ground, snow
	for (CurEffect = VecPoly->MyEffects; CurEffect; CurEffect = CurEffect->Next)
		{
		if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
			EvalEcosystems = true;
		else if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_GROUND)
			EvalGrounds = true;
		else if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_SNOW)
			EvalSnows = true;
		} // for

	// water has already been evaluated but the beach materials contribute to the overall coverage and needs
	// to be accounted for before adding more ecosystems, grounds and snow. Also terraffectors may have
	// added some ecosystems.
	for (Matty = CurNode->NodeData->Materials; Matty && BoolSuccess; Matty = Matty->NextMaterial)
		{
		// add beach materials if necessary but don't sum their coverage since they will be added to the end
		// of the material list and hence will be summed in the normal operation of this loop
		if (Matty->WaterProp && Matty->WaterProp->BeachOwner && ! CurNode->FlagCheck(WCS_VECTORNODE_FLAG_BEACHEVALUATED))
			BoolSuccess = EffectsBase->AddBeachMaterials(&RendData, CurNode, Matty, DummySumOfAllCoverages);
		if (Matty->MyMat->MaterialType == WCS_EFFECTS_MATERIALTYPE_BEACH || Matty->MyMat->MaterialType == WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)
			SumOfAllCoverages += Matty->MatCoverage * (1.0 - Matty->MyMat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
		if (Matty->MyMat->MaterialType == WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM && ((EcosystemEffect *)Matty->MyEffect)->PlowSnow)
			PlowedSnow += Matty->MatCoverage;
		} // for
	
	if (BoolSuccess)
		{
		// environment - evaluate with GeoRaster method
		// fills in CurNode->NodeData->NodeEnvironment. There can be only one environment at a node.
		// Environment has info needed for all foliage rendering so Env is needed even if a lake or stream here.
		EffectsBase->EvalEnvironment(&RendData, CurNode);
		// EvalEcosystems adds ecosystem materials to the material list but only from
		// vector-bounded ecosystems. It shouldn't be called if the coverage of ecosystems already equals 100%
		if ((EvalEcosystems || EcoRastersExist) && SumOfAllCoverages < 1.0)
			BoolSuccess = EffectsBase->EvalEcosystems(&RendData, VecPoly->MyEffects, CurNode, SumOfAllCoverages, PlowedSnow);
		if (CmapsExist && SumOfAllCoverages < 1.0 && BoolSuccess)
			BoolSuccess = EffectsBase->EvalColorMaps(&RendData, CurNode, SumOfAllCoverages, PlowedSnow);
		if (SumOfAllCoverages < 1.0 && BoolSuccess)
			BoolSuccess = CurNode->NodeData->NodeEnvironment->EvalEcosystem(&RendData, CurNode, SumOfAllCoverages, PlowedSnow);
		if (BoolSuccess)
			{
			if (EvalGrounds)
				BoolSuccess = EffectsBase->EvalGrounds(&RendData, VecPoly->MyEffects, CurNode);
			else
				BoolSuccess = EffectsBase->AddDefaultGround(&RendData, CurNode);
			} // if
		if (BoolSuccess)
			{
			if (EvalSnows)
				BoolSuccess = EffectsBase->EvalSnows(&RendData, VecPoly->MyEffects, CurNode, PlowedSnow);
			else
				BoolSuccess = EffectsBase->AddDefaultSnow(&RendData, CurNode, PlowedSnow);
			} // if
			
		CurNode->FlagSet(WCS_VECTORNODE_FLAG_EFFECTSEVALUATED);
		} // if
	} // if
else
	BoolSuccess = false;

return (BoolSuccess ? 1: 0);

} // Renderer::AnalyzeNodeData

/*===========================================================================*/

int Renderer::AnalyzeNodeElevation(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown)
{
double SumOfAllCoverages = 0.0;
unsigned long PositionFlags = 0;
CoordSys *DEMCoords;
EffectJoeList *CurEffect;
bool EvalTfx = false, EvalATfx = false, BoolSuccess = true;

if (CurNode->NodeData || CurNode->AddNodeData())
	{
	//CurNode->NodeData->LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX);
	//CurNode->NodeData->LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX);

	DEMCoords = DEMVecPoly->GetFirstVector()->GetCoordSys();

	// find elevation and relative elevation from DEM
	if (CellUnknown || ! (PositionFlags = CurNode->FlagCheck(WCS_VECTORNODE_FLAG_BOUNDARYNODE)))
		BoolSuccess = CurDEM->SplineInternalCellValue(DEMCoords, DefCoords, PositionFlags, CurNode, CellUnknown);
	else
		{
		if (PositionFlags == WCS_VECTORNODE_FLAG_EASTEDGE || PositionFlags == WCS_VECTORNODE_FLAG_WESTEDGE)
			{
			BoolSuccess = CurDEM->SplineCellEdgeEastWest(DEMCoords, DefCoords, PositionFlags, CurNode);
			} // if
		else if (PositionFlags == WCS_VECTORNODE_FLAG_NORTHEDGE || PositionFlags == WCS_VECTORNODE_FLAG_SOUTHEDGE)
			{
			BoolSuccess = CurDEM->SplineCellEdgeNorthSouth(DEMCoords, DefCoords, PositionFlags, CurNode);
			} // if
		else
			{
			CurDEM->SampleCellCorner(PositionFlags, CurNode);
			} // if
		} // if

	// vertical exaggeration and DEM scaling
	CurNode->Elev = ElevDatum + ((CurNode->Elev * ElScaleMult) - ElevDatum) * Exageration;
	
	// calculate textured displacement
	if (BoolSuccess)
		{
		if (DefaultTerrainPar->GetEnabledTexture(WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT))
			{
			double TexOpacity, Displacement, Value[3];

			Value[0] = Value[1] = Value[2] = 0.0;
			if ((TexOpacity = DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT]->Eval(Value, RendData.TransferTextureData(CurNode, NULL, NULL))) > 0.0)
				{
				// currently there is no way to get the slope at this stage of analysis
				//SlopeFactor = DefaultTerrainPar->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR].CurValue;	// temporary
				Displacement = DefaultTerrainPar->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].CurValue;	// temporary
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					Value[0] -= TexOpacity * .5;
					Displacement *= 2.0 * Value[0];
					} // if
				else
					{
					Value[0] -= .5;
					Displacement *= Value[0];
					} // else
				CurNode->NodeData->TexDisplacement = (float)Displacement;
				CurNode->Elev += CurNode->NodeData->TexDisplacement;
				} // if
			} // if
		} // if
		
	if (BoolSuccess)
		{
		if (TerraffectorsExist || RasterTAsExist)
			{
			for (CurEffect = VecPoly->MyEffects; CurEffect; CurEffect = CurEffect->Next)
				{
				if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
					EvalTfx = true;
				else if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
					EvalATfx = true;
				} // for
			if (EvalATfx)
				BoolSuccess = EffectsBase->EvalRasterTerraffectors(&RendData, VecPoly->MyEffects, CurNode);
			if (EvalTfx && BoolSuccess)
				BoolSuccess = EffectsBase->EvalTerraffectors(&RendData, VecPoly->MyEffects, CurNode, false, SumOfAllCoverages);
			} // if
		CurNode->FlagSet(WCS_VECTORNODE_FLAG_ELEVEVALUATED);
		CurNode->FlagSet(WCS_VECTORNODE_FLAG_TFXECOEVALUATED);
		} // if
	} // if
else
	BoolSuccess = 0;

return (BoolSuccess ? 1: 0);

} // Renderer::AnalyzeNodeElevation

/*===========================================================================*/

int Renderer::AnalyzeNodeNormals(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown)
{
double PolyNormal[3], NormalSum[3], PolyWt, WtSum, DummySumOfAllCoverages;
unsigned long LinkNodeCt;
int Success = 1;
VectorNodeLink *SearchNodeLink;
VectorNode *LinkedPolyNode, *CopyNode;
EffectJoeList *CurEffect;
VertexDEM Vert;

// find the surface normal at the node from the SN's of surrounding polygons
WtSum = NormalSum[2] = NormalSum[1] = NormalSum[0] = 0.0;

if (VecPoly->GetNormals(DefCoords, PlanetRad, PolyNormal, PolyWt, CurNode))
	{
	NormalSum[0] = PolyNormal[0] * PolyWt;
	NormalSum[1] = PolyNormal[1] * PolyWt;
	NormalSum[2] = PolyNormal[2] * PolyWt;
	WtSum += PolyWt;
	} // if
for (SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
	{
	// Ignore polygons with more than 3 vertices. They are not renderable polygons
	if (SearchNodeLink->LinkedPolygon->TotalNumNodes == 3)
		{
		// the linked polygon needs to have all its correct elevations first
		for (LinkedPolyNode = SearchNodeLink->LinkedPolygon->PolyFirstNode(), LinkNodeCt = 0; LinkNodeCt < SearchNodeLink->LinkedPolygon->TotalNumNodes; LinkedPolyNode = LinkedPolyNode->NextNode, ++LinkNodeCt)
			{
			if (! LinkedPolyNode->FlagCheck(WCS_VECTORNODE_FLAG_ELEVEVALUATED))
				{
				CopyNode = NULL;
				for (VectorNodeLink *TempSearchNodeLink = LinkedPolyNode->LinkedNodes; TempSearchNodeLink; TempSearchNodeLink = (VectorNodeLink *)TempSearchNodeLink->NextNodeList)
					{
					if (TempSearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_ELEVEVALUATED))
						{
						CopyNode = TempSearchNodeLink->MyNode;
						break;
						} // if
					} // for
				if (CopyNode)
					{
					LinkedPolyNode->CopyElevationData(CopyNode);
					if (TerraffectorsExist)
						{
						for (CurEffect = VecPoly->MyEffects; CurEffect; CurEffect = CurEffect->Next)
							{
							if (CurEffect->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
								{
								if (! EffectsBase->EvalTerraffectors(&RendData, VecPoly->MyEffects, CurNode, true, DummySumOfAllCoverages = 0.0))
									return (false);
								break;
								} // if
							} // for
						} // if
					} // if
				else
					{
					// CellUnknown = true
					if (! AnalyzeNodeElevation(DEMVecPoly, LinkedPolyNode, SearchNodeLink->LinkedPolygon, true))
						return (false);
					} // else
				} // if
			} // for
		if (SearchNodeLink->LinkedPolygon->GetNormals(DefCoords, PlanetRad, PolyNormal, PolyWt, SearchNodeLink->MyNode))
			{
			NormalSum[0] += PolyNormal[0] * PolyWt;
			NormalSum[1] += PolyNormal[1] * PolyWt;
			NormalSum[2] += PolyNormal[2] * PolyWt;
			WtSum += PolyWt;
			} // if
		} // if
	} // for
if (WtSum > 0.0)
	{
	// normalize
	NormalSum[0] /= WtSum;
	NormalSum[1] /= WtSum;
	NormalSum[2] /= WtSum;
	CurNode->NodeData->Normal[0] = (float)NormalSum[0];
	CurNode->NodeData->Normal[1] = (float)NormalSum[1];
	CurNode->NodeData->Normal[2] = (float)NormalSum[2];
	// vectorize the vertex location and convert to world cartesian
	Vert.Lat = CurNode->Lat;
	Vert.Lon = CurNode->Lon;
	Vert.Elev = CurNode->Elev;
	#ifdef WCS_BUILD_VNS
	if (DefCoords->DegToCart(&Vert))
	#else // WCS_BUILD_VNS
	if (Vert.DegToCart(PlanetRad))
	#endif // WCS_BUILD_VNS
		{
		// the table lookup for the slope was giving some pretty erroneous results for slopes near 0 
		// (like showing slope of 6 degrees instead of 0).
		//CurNode->NodeData->Slope = (float)(acos(VectorAngle(Vert.XYZ, NormalSum)) * PiUnder180);	// in degrees
		CurNode->NodeData->Slope = (float)(GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Vert.XYZ, NormalSum)) * PiUnder180);	// in degrees
		if (CurNode->NodeData->Slope < 0.0f)
			CurNode->NodeData->Slope = 0.0f;
		else if (CurNode->NodeData->Slope > 90.0f)
			CurNode->NodeData->Slope = 90.0f;
		Vert.XYZ[0] = NormalSum[0];
		Vert.XYZ[1] = NormalSum[1];
		Vert.XYZ[2] = NormalSum[2];
		Vert.RotateToHome();
		//CurNode->NodeData->Aspect = (float)Vert.FindAngleYfromZ();
		CurNode->NodeData->Aspect = (float)Vert.FindRoughAngleYfromZ();
		if (CurNode->NodeData->Aspect < 0.0f)
			CurNode->NodeData->Aspect += 360.0f;
		} // if
	} // if
CurNode->FlagSet(WCS_VECTORNODE_FLAG_NORMALSEVALUATED);

return (Success);

} // Renderer::AnalyzeNodeNormals

/*===========================================================================*/

int Renderer::AnalyzeNodeWaterNormals(VectorPolygon *DEMVecPoly, VectorNode *CurNode, VectorPolygon *VecPoly, bool CellUnknown)
{
double PolyNormal[3], NormalSum[3], PolyWt, WtSum, ElevEntry1, ElevEntry2, ElevEntry3, 
	ElevEntryLink[3], LastElev = FLT_MAX;
unsigned long LinkNodeCt, AboveGroundPoints;
int Success = 1;
bool WaterMatFound = false;
VectorNodeLink *SearchNodeLink;
VectorNode *LinkedPolyNode, *AltNode2, *AltNode3;
MaterialList *Matt1, *Matt2, *Matt3, *MattLink[3];

NormalSum[2] = NormalSum[1] = NormalSum[0] = 0.0;

// analyze each water material that has a unique elevation set
for (Matt1 = CurNode->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt1;
	Matt1 = CurNode->NodeData->FindNextMaterial(Matt1, WCS_EFFECTS_MATERIALTYPE_WATER))
	{
	if (Matt1->WaterProp)
		{
		if ((ElevEntry1 = CurNode->Elev + Matt1->WaterProp->WaterDepth + Matt1->WaterProp->WaveHeight) != LastElev)
			{
			LastElev = ElevEntry1;
			Matt2 = NULL;
			
			for (SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink && ! Matt2; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
				{
				if (SearchNodeLink->MyNode->FlagCheck(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED))
					{
					// find the correct water material for this node
					for (Matt2 = SearchNodeLink->MyNode->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt2;
						Matt2 = SearchNodeLink->MyNode->NodeData->FindNextMaterial(Matt2, WCS_EFFECTS_MATERIALTYPE_WATER))
						{
						if (Matt2->WaterProp && Matt2->MyMat == Matt1->MyMat)
							{
							// it is possible to be here and not have a calculated water surface normal which makes
							// it inadvisable to just copy the normal.
							if (Matt2->WaterProp->WaterNormal[0] != 0.0)
								break;
							} // if
						} // for
					} // if
				} // for

			if (Matt2)
				{
				NormalSum[0] = Matt2->WaterProp->WaterNormal[0];
				NormalSum[1] = Matt2->WaterProp->WaterNormal[1];
				NormalSum[2] = Matt2->WaterProp->WaterNormal[2];
				Matt1->WaterProp->WaterNormal[0] = Matt2->WaterProp->WaterNormal[0];
				Matt1->WaterProp->WaterNormal[1] = Matt2->WaterProp->WaterNormal[1];
				Matt1->WaterProp->WaterNormal[2] = Matt2->WaterProp->WaterNormal[2];
				// done with this material, go on to the next
				WaterMatFound = true;
				continue;
				} // if
				
			// find out if at least one water vertex is visible above land, no need to render if it is below ground		
			AboveGroundPoints = 0;
			if (ElevEntry1 >= CurNode->Elev)
				++AboveGroundPoints;
			// if the other nodes in this polygon have not had their water bodies evaluated, do it now
			AltNode2 = CurNode->NextNode;
			AltNode3 = AltNode2->NextNode;

			// in AnalyzeNodeWater, AddBeachMaterials = false, since node slope may not yet known
			if (! AltNode2->FlagCheck(WCS_VECTORNODE_FLAG_WATEREVALUATED))
				Success = AnalyzeNodeWater(DEMVecPoly, AltNode2, VecPoly, CellUnknown, AltNode2->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED) ? true: false);
			if (Success && ! AltNode3->FlagCheck(WCS_VECTORNODE_FLAG_WATEREVALUATED))
				Success = AnalyzeNodeWater(DEMVecPoly, AltNode3, VecPoly, CellUnknown, AltNode3->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED) ? true: false);
			// find the water material in each of the other nodes
			for (Matt2 = AltNode2->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt2;
				Matt2 = AltNode2->NodeData->FindNextMaterial(Matt2, WCS_EFFECTS_MATERIALTYPE_WATER))
				{
				if (Matt2->WaterProp && Matt2->MyMat == Matt1->MyMat)
					{
					if ((ElevEntry2 = AltNode2->Elev + Matt2->WaterProp->WaterDepth + Matt2->WaterProp->WaveHeight) >= AltNode2->Elev)
						++AboveGroundPoints;
					break;
					} // if
				} // for
			for (Matt3 = AltNode3->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt3;
				Matt3 = AltNode3->NodeData->FindNextMaterial(Matt3, WCS_EFFECTS_MATERIALTYPE_WATER))
				{
				if (Matt3->WaterProp && Matt3->MyMat == Matt1->MyMat)
					{
					if ((ElevEntry3 = AltNode3->Elev + Matt3->WaterProp->WaterDepth + Matt3->WaterProp->WaveHeight) >= AltNode3->Elev)
						++AboveGroundPoints;
					break;
					} // if
				} // for
			if (! Matt2)
				{
				// find another material with the same effect and vector
				for (Matt2 = AltNode2->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt2;
					Matt2 = AltNode2->NodeData->FindNextMaterial(Matt2, WCS_EFFECTS_MATERIALTYPE_WATER))
					{
					if (Matt2->WaterProp && Matt2->MyEffect == Matt1->MyEffect && Matt2->MyVec == Matt1->MyVec)
						{
						if ((ElevEntry2 = AltNode2->Elev + Matt2->WaterProp->WaterDepth + Matt2->WaterProp->WaveHeight) >= AltNode2->Elev)
							++AboveGroundPoints;
						break;
						} // if
					} // for
				} // if
			if (! Matt2)
				{
				Matt2 = Matt1;
				ElevEntry2 = ElevEntry1;
				} // else
			if (! Matt3)
				{
				// find another material with the same effect and vector
				for (Matt3 = AltNode2->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); Matt3;
					Matt3 = AltNode2->NodeData->FindNextMaterial(Matt3, WCS_EFFECTS_MATERIALTYPE_WATER))
					{
					if (Matt3->WaterProp && Matt3->MyEffect == Matt1->MyEffect && Matt3->MyVec == Matt1->MyVec)
						{
						if ((ElevEntry3 = AltNode3->Elev + Matt3->WaterProp->WaterDepth + Matt3->WaterProp->WaveHeight) >= AltNode3->Elev)
							++AboveGroundPoints;
						break;
						} // if
					} // for
				} // if
			if (! Matt3)
				{
				Matt3 = Matt1;
				ElevEntry3 = ElevEntry1;
				} // else
			if (Success && AboveGroundPoints)
				{
				// now we know it is worth doing the rest of the labor since the water will actually be rendered
				// find the surface normal at the node from the SN's of surrounding polygons
				// Begin with the normal of this polygon
				NormalSum[2] = NormalSum[1] = NormalSum[0] = WtSum = 0.0;
				if (VecPoly->GetWaterNormals(DefCoords, PlanetRad, PolyNormal, PolyWt, CurNode, ElevEntry1, ElevEntry2, ElevEntry3))
					{
					NormalSum[0] = PolyNormal[0] * PolyWt;
					NormalSum[1] = PolyNormal[1] * PolyWt;
					NormalSum[2] = PolyNormal[2] * PolyWt;
					WtSum += PolyWt;
					} // if

				for (SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
					{
					// there are some original polygons in the mix here and we don't want to be testing them if their number of vertices exceeds 3
					// since that indicates it it not a renderable polygon
					if (SearchNodeLink->LinkedPolygon->TotalNumNodes == 3)
						{
						// the linked polygon needs to have all its correct water elevations first
						for (LinkedPolyNode = SearchNodeLink->LinkedPolygon->PolyFirstNode(), LinkNodeCt = 0; LinkNodeCt < SearchNodeLink->LinkedPolygon->TotalNumNodes; LinkedPolyNode = LinkedPolyNode->NextNode, ++LinkNodeCt)
							{
							if (! LinkedPolyNode->FlagCheck(WCS_VECTORNODE_FLAG_WATEREVALUATED))
								{
								// CellUnknown = true, AddBeachMaterials = optioned on determination of node's slope
								if (! AnalyzeNodeWater(DEMVecPoly, LinkedPolyNode, SearchNodeLink->LinkedPolygon, true, LinkedPolyNode->FlagCheck(WCS_VECTORNODE_FLAG_NORMALSEVALUATED) ? true: false))
									return (false);
								} // if
							// find the correct water material for this node
							for (MattLink[LinkNodeCt] = LinkedPolyNode->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); MattLink[LinkNodeCt];
								MattLink[LinkNodeCt] = LinkedPolyNode->NodeData->FindNextMaterial(MattLink[LinkNodeCt], WCS_EFFECTS_MATERIALTYPE_WATER))
								{
								if (MattLink[LinkNodeCt]->WaterProp && MattLink[LinkNodeCt]->MyMat == Matt1->MyMat)
									{
									break;
									} // if
								} // for
							if (! MattLink[LinkNodeCt])
								{
								// if couldn't find same material try looking for a material with same effect and vector
								for (MattLink[LinkNodeCt] = LinkedPolyNode->NodeData->FindFirstMaterial(WCS_EFFECTS_MATERIALTYPE_WATER); MattLink[LinkNodeCt];
									MattLink[LinkNodeCt] = LinkedPolyNode->NodeData->FindNextMaterial(MattLink[LinkNodeCt], WCS_EFFECTS_MATERIALTYPE_WATER))
									{
									if (MattLink[LinkNodeCt]->WaterProp && MattLink[LinkNodeCt]->MyEffect == Matt1->MyEffect && MattLink[LinkNodeCt]->MyVec == Matt1->MyVec)
										{
										break;
										} // if
									} // for
								} // if
							if (MattLink[LinkNodeCt])
								ElevEntryLink[LinkNodeCt] = LinkedPolyNode->Elev + MattLink[LinkNodeCt]->WaterProp->WaterDepth + MattLink[LinkNodeCt]->WaterProp->WaveHeight;
							} // for
						if (MattLink[0] || MattLink[1] || MattLink[2])
							{
							if (! MattLink[0])
								{
								MattLink[0] = MattLink[1] ? MattLink[1]: MattLink[2];
								ElevEntryLink[0] = MattLink[1] ? ElevEntryLink[1]: ElevEntryLink[2];
								} // if
							if (! MattLink[1])
								{
								MattLink[1] = MattLink[2] ? MattLink[2]: MattLink[0];
								ElevEntryLink[1] = MattLink[2] ? ElevEntryLink[2]: ElevEntryLink[0];
								} // if
							if (! MattLink[2])
								{
								MattLink[2] = MattLink[0] ? MattLink[0]: MattLink[1];
								ElevEntryLink[2] = MattLink[0] ? ElevEntryLink[0]: ElevEntryLink[1];
								} // if
							} // if
						if (SearchNodeLink->LinkedPolygon->GetWaterNormals(DefCoords, PlanetRad, PolyNormal, PolyWt, 
							LinkedPolyNode, ElevEntryLink[0], ElevEntryLink[1], ElevEntryLink[2]))
							{
							NormalSum[0] += PolyNormal[0] * PolyWt;
							NormalSum[1] += PolyNormal[1] * PolyWt;
							NormalSum[2] += PolyNormal[2] * PolyWt;
							WtSum += PolyWt;
							} // if
						} // if
					} // for
				if (WtSum > 0.0)
					{
					// normalize
					NormalSum[0] /= WtSum;
					NormalSum[1] /= WtSum;
					NormalSum[2] /= WtSum;
					Matt1->WaterProp->WaterNormal[0] = (float)NormalSum[0];
					Matt1->WaterProp->WaterNormal[1] = (float)NormalSum[1];
					Matt1->WaterProp->WaterNormal[2] = (float)NormalSum[2];
					WaterMatFound = true;
					} // if
				} // if
			} // if
		else
			{
			// same elevation as previous surface so copy the normals
			Matt1->WaterProp->WaterNormal[0] = (float)NormalSum[0];
			Matt1->WaterProp->WaterNormal[1] = (float)NormalSum[1];
			Matt1->WaterProp->WaterNormal[2] = (float)NormalSum[2];
			} // else
		} // if
	} // for

//if (WaterMatFound)	// why is this check here? Don't we alsways need this set if we've looked at this node?
	CurNode->FlagSet(WCS_VECTORNODE_FLAG_WATERNORMALSEVALUATED);
	
return (Success);

} // Renderer::AnalyzeNodeWaterNormals

/*===========================================================================*/

int Renderer::PrepPolyToRender(VectorPolygon *RenderMe)
{
VectorNode *TriangleNodes[3];

TriangleNodes[0] = RenderMe->PolyFirstNode();
TriangleNodes[1] = RenderMe->PolySecondNode();
TriangleNodes[2] = RenderMe->PolyThirdNode();

// this just copies the lat/lon/elev over to VertexRoot without changing it
TriangleNodes[0]->ProjToDefDeg(NULL, VertexRoot[0]);
TriangleNodes[1]->ProjToDefDeg(NULL, VertexRoot[1]);
TriangleNodes[2]->ProjToDefDeg(NULL, VertexRoot[2]);

// other initialization stuff
// Lat, Lon and Elev are already initialized when the vertex is copied from the VertexDEM above
VertexRoot[0]->RelEl = TriangleNodes[0]->NodeData->RelEl;
VertexRoot[1]->RelEl = TriangleNodes[1]->NodeData->RelEl;
VertexRoot[2]->RelEl = TriangleNodes[2]->NodeData->RelEl;

VertexRoot[0]->Map = VertexRoot[1]->Map = VertexRoot[2]->Map = CurDEM;

VertexRoot[0]->NodeData = TriangleNodes[0]->NodeData;
VertexRoot[1]->NodeData = TriangleNodes[1]->NodeData;
VertexRoot[2]->NodeData = TriangleNodes[2]->NodeData;

return (TerrainVectorPolygonSetup(RenderMe, VertexRoot));

} //  Renderer::PrepPolyToRender

/*===========================================================================*/

void Renderer::CullNonTerrainPolys(VectorPolygonListDouble *&VPCellList, GeneralEffect *TerrainEffect)
{
// cull polygons that do not have a terrain effect
VectorPolygonListDouble *CurVPList, *LastPolyList = NULL;

for (CurVPList = VPCellList; CurVPList; CurVPList = CurVPList ? (VectorPolygonListDouble *)CurVPList->NextPolygonList: VPCellList)
	{
	if (! CurVPList->MyPolygon->TestForEffect(TerrainEffect))
		{
		// remove the polygon
		if (LastPolyList)
			{
			LastPolyList->NextPolygonList = (VectorPolygonListDouble *)CurVPList->NextPolygonList;
			} // if
		else
			{
			VPCellList = (VectorPolygonListDouble *)CurVPList->NextPolygonList;
			} // else
		#ifndef MAKENOCLONES
		// delete them only if made especially for this terrain cell
		CurVPList->DeletePolygon();
		#else // MAKENOCLONES
		CurVPList->MyPolygon->RemoveTerrainEffectLinks(TerrainEffect);
		#endif // MAKENOCLONES
		delete CurVPList;
		CurVPList = LastPolyList;
		} // if
	LastPolyList = CurVPList;
	} // for

} // Renderer::CullNonTerrainPolys

/*===========================================================================*/

bool Renderer::TestPolygonInBounds(unsigned long *VertNum, bool &HiddenRemovalSafe)
{
VertexDEM TempVtxLow, TempVtxHigh;
int a, b, c, d, z, NodeCt;

HiddenRemovalSafe = true;

for (NodeCt = a = b = c = d = z = 0; NodeCt < 4; ++NodeCt)
	{
	TempVtxHigh.Lat = TempVtxLow.Lat = CurDEM->Vertices[VertNum[NodeCt]].Lat;
	TempVtxHigh.Lon = TempVtxLow.Lon = CurDEM->Vertices[VertNum[NodeCt]].Lon;
	TempVtxHigh.Elev = CurDEM->Vertices[VertNum[NodeCt]].Elev + CurDEM->RelativeEffectAdd;
	TempVtxLow.Elev = CurDEM->Vertices[VertNum[NodeCt]].Elev + CurDEM->RelativeEffectSubtract;
	if (TempVtxHigh.Elev < CurDEM->AbsoluteEffectMax)
		TempVtxHigh.Elev = CurDEM->AbsoluteEffectMax;
	if (TempVtxLow.Elev > CurDEM->AbsoluteEffectMin)
		TempVtxLow.Elev = CurDEM->AbsoluteEffectMin;

	// process any elevation-modifying effects at the vertex
	// these include lakes, streams, terraffectors and area terraffectors
	// First determine what effects are present at the vertex
	// Create a list of vector polygons to consider derived from the list for the current DEM
	if (TempVtxLow.Elev != TempVtxHigh.Elev)
		{
		// project both the maximum and minimum elevations
		Cam->ProjectVertexDEM(DefCoords, &TempVtxHigh, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
		Cam->ProjectVertexDEM(DefCoords, &TempVtxLow, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
		// Test the line between the projections to see if any part of the line could be in the
		// rendered image.
		if (TempVtxHigh.ScrnXYZ[2] < 0.0001 && TempVtxLow.ScrnXYZ[2] < 0.0001)
			++z;	// behind viewer
		if (TempVtxHigh.ScrnXYZ[0] < OSLowX && TempVtxLow.ScrnXYZ[0] < OSLowX)
			++a;
		else if (TempVtxHigh.ScrnXYZ[0] > OSHighX && TempVtxLow.ScrnXYZ[0] > OSHighX)
			++b;
		if (TempVtxHigh.ScrnXYZ[1] < OSLowY && TempVtxLow.ScrnXYZ[1] < OSLowY)
			++c;
		else if (TempVtxHigh.ScrnXYZ[1] > OSHighY && TempVtxLow.ScrnXYZ[1] > OSHighY)
			++d;
		HiddenRemovalSafe = false;
		} // if
	else
		{
		if (CurDEM->Vertices[VertNum[NodeCt]].ScrnXYZ[2] < 0.0001)
			++z;	// behind viewer
		if (CurDEM->Vertices[VertNum[NodeCt]].ScrnXYZ[0] < OSLowX)
			++a;
		else if (CurDEM->Vertices[VertNum[NodeCt]].ScrnXYZ[0] > OSHighX)
			++b;
		if (CurDEM->Vertices[VertNum[NodeCt]].ScrnXYZ[1] < OSLowY)
			++c;
		else if (CurDEM->Vertices[VertNum[NodeCt]].ScrnXYZ[1] > OSHighY)
			++d;
		} // else
	} // for

return (z < 4 && a < 4 && b < 4 && c < 4 && d < 4);

} // Renderer::TestPolygonInBounds

/*===========================================================================*/

bool Renderer::GetPolygonBlockRenderStatus(VectorPolygonListDouble *TestList)
{

for (VectorPolygonListDouble *CurList = TestList; CurList; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
	{
	if (GetPolygonRenderStatus(CurList->MyPolygon))
		return (true);
	} // for

return (false);

} // Renderer::GetPolygonBlockRenderStatus

/*===========================================================================*/

bool Renderer::GetPolygonRenderStatus(VectorPolygon *TestMe)
{
VertexDEM TempVtxLow, TempVtxHigh;
VectorNode *CurNode;
unsigned long a, b, c, d, z, NodeCt;
bool TestOne;

for (NodeCt = a = b = c = d = z = 0, CurNode = TestMe->PolyFirstNode(); NodeCt < TestMe->TotalNumNodes; ++NodeCt, CurNode = CurNode->NextNode)
	{
	TestOne = false;
	TempVtxHigh.Lat = TempVtxLow.Lat = CurNode->Lat;
	TempVtxHigh.Lon = TempVtxLow.Lon = CurNode->Lon;
	TempVtxHigh.Elev = CurNode->Elev + CurDEM->RelativeEffectAdd;
	TempVtxLow.Elev = CurNode->Elev + CurDEM->RelativeEffectSubtract;
	if (TempVtxHigh.Elev < CurDEM->AbsoluteEffectMax)
		TempVtxHigh.Elev = CurDEM->AbsoluteEffectMax;
	if (TempVtxLow.Elev > CurDEM->AbsoluteEffectMin)
		TempVtxLow.Elev = CurDEM->AbsoluteEffectMin;

	// process any elevation-modifying effects at the vertex
	// these include lakes, streams, terraffectors and area terraffectors
	// First determine what effects are present at the vertex
	// Create a list of vector polygons to consider derived from the list for the current DEM
	if (TempVtxLow.Elev != TempVtxHigh.Elev)
		{
		// project both the maximum and minimum elevations
		Cam->ProjectVertexDEM(DefCoords, &TempVtxHigh, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
		Cam->ProjectVertexDEM(DefCoords, &TempVtxLow, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
		} // if
	else
		{
		Cam->ProjectVertexDEM(DefCoords, &TempVtxHigh, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
		TestOne = true;
		} // else

	if (TestOne)
		{
		if (TempVtxHigh.ScrnXYZ[2] < 0.0001)
			++z;
		if (TempVtxHigh.ScrnXYZ[0] < OSLowX)
			++a;
		else if (TempVtxHigh.ScrnXYZ[0] > OSHighX)
			++b;
		if (TempVtxHigh.ScrnXYZ[1] < OSLowY)
			++c;
		else if (TempVtxHigh.ScrnXYZ[1] > OSHighY)
			++d;
		} // if
	else
		{
		// Test the line between the projections to see if any part of the line could be in the
		// rendered image.
		if (TempVtxHigh.ScrnXYZ[2] < 0.0001 && TempVtxLow.ScrnXYZ[2] < 0.0001)
			++z;
		if (TempVtxHigh.ScrnXYZ[0] < OSLowX && TempVtxLow.ScrnXYZ[0] < OSLowX)
			++a;
		else if (TempVtxHigh.ScrnXYZ[0] > OSHighX && TempVtxLow.ScrnXYZ[0] > OSHighX)
			++b;
		if (TempVtxHigh.ScrnXYZ[1] < OSLowY && TempVtxLow.ScrnXYZ[1] < OSLowY)
			++c;
		else if (TempVtxHigh.ScrnXYZ[1] > OSHighY && TempVtxLow.ScrnXYZ[1] > OSHighY)
			++d;
		} // else
	} // for

return (z < TestMe->TotalNumNodes && a < TestMe->TotalNumNodes && b < TestMe->TotalNumNodes && c < TestMe->TotalNumNodes && d < TestMe->TotalNumNodes);

} // Renderer::GetPolygonRenderStatus

/*===========================================================================*/

int Renderer::GetFractalLevel(unsigned long *VertNum, unsigned long PolyNumber, bool &RenderIt, bool TestVisibility)
{
bool Success = true, ForceRender = false, HiddenRemovalSafe;
int PolyFrd, DepthMapFrd, DepthMapFrd1, DepthMapFrd2;
unsigned long FrdPolyNumber;

// determine fractal depth for this polygon
PolyFrd = DefaultFrd;		// DefaultFrd is Renderer scope based on default terrain param and DEM's max fractal depth

// depth maps are the least expensive way to cull polygons so do it first
if (CurDEM->FractalMap)
	{
	// double the polygon number for looking up in the fractal map
	FrdPolyNumber = PolyNumber * 2;
	if ((DepthMapFrd1 = CurDEM->FractalMap[FrdPolyNumber]) >= 10)
		{
		DepthMapFrd1 -= 10;
		ForceRender = true;
		} // if
	if ((DepthMapFrd2 = CurDEM->FractalMap[FrdPolyNumber + 1]) >= 10)
		{
		DepthMapFrd2 -= 10;
		ForceRender = true;
		} // if
	DepthMapFrd = DepthMapFrd1 > DepthMapFrd2 ? DepthMapFrd1: DepthMapFrd2;
	// depth map rules
	if (DepthMapFrd < PolyFrd)
		PolyFrd = DepthMapFrd;
	} // if

// bounds testing is the second-least expensive culling method
if (TestVisibility)
	{
	if (ForceRender)
		RenderIt = true;
	else
		RenderIt = TestPolygonInBounds(VertNum, HiddenRemovalSafe);
	} // if

return (PolyFrd);

} // Renderer::GetFractalLevel

/*===========================================================================*/

bool Renderer::SubdivideTerrainPolygon(VectorPolygonListDouble *&HeadList, unsigned long PolyNumber, int FractalLevel)
{
double LatLonInterval, MultiLatLonInterval, LatFromNorth, LatFromSouth, LonFromWest, LonFromEast, M1, M2, M3, M4;
bool Success = true, RowStyle;
int FrdNorth, FrdSouth, FrdEast, FrdWest, AnotherValue, Method;
unsigned long FrdPolyNumber, DEMRow, DEMColumn; 
long LatEntriesTimes2, SubColCt, SubRowCt, NumSubRows, LastRow, NDiffRows, SDiffRows, WDiffRows, EDiffRows,
	ThisRow, NextRow;
long FirstPtThisRow, NextPtThisRow, FirstPtNextRow, NextPtNextRow;
VectorNode *NWnode, *NEnode, *SWnode, *SEnode, *FirstNode, *SecondNode, *ThirdNode;
VectorPolygonListDouble **PolygonListPtr, *EffectsHead = NULL;

// subdivide the polygon according to the value FractalLevel
// Take into account the fractal depth of surrounding polygons
FrdPolyNumber = PolyNumber * 2;
LatEntriesTimes2 = (CurDEM->LatEntries() - 1) * 2;

if (CurDEM->FractalMap)
	{
	// find row and column, rows are
	DEMColumn = PolyNumber / (CurDEM->LatEntries() - 1);
	DEMRow = PolyNumber - DEMColumn * (CurDEM->LatEntries() - 1);

	FrdNorth = FrdSouth = FrdEast = FrdWest = 0;		// in case an edge polygon
	// FrdNorth 
	// if not the topmost cell in the DEM
	if (DEMRow < CurDEM->LatEntries() - 2)
		{
		FrdNorth = CurDEM->FractalMap[FrdPolyNumber + 2];
		AnotherValue = CurDEM->FractalMap[FrdPolyNumber + 3];
		if (FrdNorth >= 10)
			FrdNorth -= 10;
		if (AnotherValue >= 10)
			AnotherValue -= 10;
		if (AnotherValue > FrdNorth)
			FrdNorth = AnotherValue;
		if (FrdNorth < 0)
			FrdNorth = 0;
		if (FrdNorth > DefaultFrd)
			FrdNorth = DefaultFrd;
		} // if
	// FrdSouth 
	// if not the bottommost cell in the DEM
	if (DEMRow > 0)
		{
		FrdSouth = CurDEM->FractalMap[FrdPolyNumber - 2];
		AnotherValue = CurDEM->FractalMap[FrdPolyNumber - 1];
		if (FrdSouth >= 10)
			FrdSouth -= 10;
		if (AnotherValue >= 10)
			AnotherValue -= 10;
		if (AnotherValue > FrdSouth)
			FrdSouth = AnotherValue;
		if (FrdSouth < 0)
			FrdSouth = 0;
		if (FrdSouth > DefaultFrd)
			FrdSouth = DefaultFrd;
		} // if
	// FrdEast 
	// if not the rightmost cell in the DEM
	if (DEMColumn < CurDEM->LonEntries() - 2)
		{
		FrdEast = CurDEM->FractalMap[FrdPolyNumber + LatEntriesTimes2];
		AnotherValue = CurDEM->FractalMap[FrdPolyNumber + LatEntriesTimes2 + 1];
		if (FrdEast >= 10)
			FrdEast -= 10;
		if (AnotherValue >= 10)
			AnotherValue -= 10;
		if (AnotherValue > FrdEast)
			FrdEast = AnotherValue;
		if (FrdEast < 0)
			FrdEast = 0;
		if (FrdEast > DefaultFrd)
			FrdEast = DefaultFrd;
		} // if
	// FrdWest
	// if not the leftmost cell in the DEM
	if (DEMColumn > 0)
		{
		FrdWest = CurDEM->FractalMap[FrdPolyNumber - LatEntriesTimes2]; 
		AnotherValue = CurDEM->FractalMap[FrdPolyNumber - LatEntriesTimes2 + 1]; 
		if (FrdWest >= 10)
			FrdWest -= 10;
		if (AnotherValue >= 10)
			AnotherValue -= 10;
		if (AnotherValue > FrdWest)
			FrdWest = AnotherValue;
		if (FrdWest < 0)
			FrdWest = 0;
		if (FrdWest > DefaultFrd)
			FrdWest = DefaultFrd;
		} // if
	} // if
else
	FrdNorth = FrdSouth = FrdEast = FrdWest = DefaultFrd;		// DefaultFrd is Renderer scope based on default terrain param and DEM's max fractal depth

// initialize FrdDataRows if not done already
if (! FrdDataRows[0])
	{
	FrdDataRows[0] = 2;
	for (int P2Ct = 1; P2Ct < 10; ++P2Ct)
		{
		FrdDataRows[P2Ct] = ((FrdDataRows[P2Ct - 1] - 1) * 2) + 1;
		} // for
	} // if

// create an array of nodes, position values will be filled in later
NumSubRows = FrdDataRows[FractalLevel];
LastRow = NumSubRows - 1;


// use original cell polygon for the four corner positions
if (! SubNodes[LastRow][0] && ! (SubNodes[LastRow][0] = new VectorNode()))	// SW
	{
	goto ReturnFailure;
	} // else
if (! SubNodes[0][0] && ! (SubNodes[0][0] = new VectorNode()))	// NW
	{
	goto ReturnFailure;
	} // else
if (! SubNodes[0][LastRow] && ! (SubNodes[0][LastRow] = new VectorNode()))	// NE
	{
	goto ReturnFailure;
	} // else
if (! SubNodes[LastRow][LastRow] && ! (SubNodes[LastRow][LastRow] = new VectorNode()))	// SE
	{
	goto ReturnFailure;
	} // else
// assign current polygon coordinates - not necessarily rectangular in shape
// corners have already had their coordinates masked
FirstNode = HeadList->MyPolygon->PolyFirstNode();
SubNodes[LastRow][0]->Lat = FirstNode->Lat;
SubNodes[LastRow][0]->Lon = FirstNode->Lon;
SubNodes[LastRow][0]->Elev = FirstNode->Elev;
SubNodes[LastRow][0]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
SubNodes[LastRow][0]->FlagSet(WCS_VECTORNODE_FLAG_SWCORNER);

SubNodes[0][0]->Lat = FirstNode->NextNode->Lat;
SubNodes[0][0]->Lon = FirstNode->NextNode->Lon;
SubNodes[0][0]->Elev = FirstNode->NextNode->Elev;
SubNodes[0][0]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
SubNodes[0][0]->FlagSet(WCS_VECTORNODE_FLAG_NWCORNER);

SubNodes[0][LastRow]->Lat = FirstNode->NextNode->NextNode->Lat;
SubNodes[0][LastRow]->Lon = FirstNode->NextNode->NextNode->Lon;
SubNodes[0][LastRow]->Elev = FirstNode->NextNode->NextNode->Elev;
SubNodes[0][LastRow]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
SubNodes[0][LastRow]->FlagSet(WCS_VECTORNODE_FLAG_NECORNER);

SubNodes[LastRow][LastRow]->Lat = FirstNode->NextNode->NextNode->NextNode->Lat;
SubNodes[LastRow][LastRow]->Lon = FirstNode->NextNode->NextNode->NextNode->Lon;
SubNodes[LastRow][LastRow]->Elev = FirstNode->NextNode->NextNode->NextNode->Elev;
SubNodes[LastRow][LastRow]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
SubNodes[LastRow][LastRow]->FlagSet(WCS_VECTORNODE_FLAG_SECORNER);

// node pointer shortcuts
NWnode = SubNodes[0][0];
NEnode = SubNodes[0][LastRow];
SWnode = SubNodes[LastRow][0];
SEnode = SubNodes[LastRow][LastRow];

NWnode->DeleteLinksNoCrossCheck();
NEnode->DeleteLinksNoCrossCheck();
SWnode->DeleteLinksNoCrossCheck();
SEnode->DeleteLinksNoCrossCheck();

// interval between rows in fractions of a cell
LatLonInterval = 1.0 / LastRow;

// north side node allocation interval depends on the adjacent cell to the north
if (FractalLevel > FrdNorth)
	NDiffRows = FrdDataRows[FractalLevel - FrdNorth] - 1;
else
	NDiffRows = 1;
MultiLatLonInterval = LatLonInterval * NDiffRows;
for (SubColCt = NDiffRows, LonFromWest = MultiLatLonInterval; SubColCt < LastRow; SubColCt += NDiffRows, LonFromWest += MultiLatLonInterval)
	{
	if (! SubNodes[0][SubColCt] && ! (SubNodes[0][SubColCt] = new VectorNode()))
		{
		goto ReturnFailure;
		} // else
	SubNodes[0][SubColCt]->DeleteLinksNoCrossCheck();
	LonFromEast = 1.0 - LonFromWest;
	if (NWnode->Lat == NEnode->Lat)
		SubNodes[0][SubColCt]->Lat = NWnode->Lat;
	else
		SubNodes[0][SubColCt]->Lat = NWnode->Lat * LonFromEast + NEnode->Lat * LonFromWest;
	SubNodes[0][SubColCt]->Lon = NWnode->Lon * LonFromEast + NEnode->Lon * LonFromWest;
	SubNodes[0][SubColCt]->Elev = NWnode->Elev * LonFromEast + NEnode->Elev * LonFromWest;
	SubNodes[0][SubColCt]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
	SubNodes[0][SubColCt]->FlagSet(WCS_VECTORNODE_FLAG_NORTHEDGE);
	} // for

// south side node allocation interval depends on the adjacent cell to the south
if (FractalLevel > FrdSouth)
	SDiffRows = FrdDataRows[FractalLevel - FrdSouth] - 1;
else
	SDiffRows = 1;
MultiLatLonInterval = LatLonInterval * SDiffRows;
for (SubColCt = SDiffRows, LonFromWest = MultiLatLonInterval; SubColCt < LastRow; SubColCt += SDiffRows, LonFromWest += MultiLatLonInterval)
	{
	if (! SubNodes[LastRow][SubColCt] && ! (SubNodes[LastRow][SubColCt] = new VectorNode()))
		{
		goto ReturnFailure;
		} // else
	SubNodes[LastRow][SubColCt]->DeleteLinksNoCrossCheck();
	LonFromEast = 1.0 - LonFromWest;
	if (SWnode->Lat == SEnode->Lat)
		SubNodes[LastRow][SubColCt]->Lat = SWnode->Lat;
	else
		SubNodes[LastRow][SubColCt]->Lat = SWnode->Lat * LonFromEast + SEnode->Lat * LonFromWest;
	SubNodes[LastRow][SubColCt]->Lon = SWnode->Lon * LonFromEast + SEnode->Lon * LonFromWest;
	SubNodes[LastRow][SubColCt]->Elev = SWnode->Elev * LonFromEast + SEnode->Elev * LonFromWest;
	SubNodes[LastRow][SubColCt]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
	SubNodes[LastRow][SubColCt]->FlagSet(WCS_VECTORNODE_FLAG_SOUTHEDGE);
	} // for

// west side node allocation interval depends on the adjacent cell to the west
if (FractalLevel > FrdWest)
	WDiffRows = FrdDataRows[FractalLevel - FrdWest] - 1;
else
	WDiffRows = 1;
MultiLatLonInterval = LatLonInterval * WDiffRows;
for (SubRowCt = WDiffRows, LatFromNorth = MultiLatLonInterval; SubRowCt < LastRow; SubRowCt += WDiffRows, LatFromNorth += MultiLatLonInterval)
	{
	if (! SubNodes[SubRowCt][0] && ! (SubNodes[SubRowCt][0] = new VectorNode()))
		{
		goto ReturnFailure;
		} // else
	SubNodes[SubRowCt][0]->DeleteLinksNoCrossCheck();
	LatFromSouth = 1.0 - LatFromNorth;
	SubNodes[SubRowCt][0]->Lat = NWnode->Lat * LatFromSouth + SWnode->Lat * LatFromNorth;
	if (NWnode->Lon == SWnode->Lon)
		SubNodes[SubRowCt][0]->Lon = NWnode->Lon;
	else
		SubNodes[SubRowCt][0]->Lon = NWnode->Lon * LatFromSouth + SWnode->Lon * LatFromNorth;
	SubNodes[SubRowCt][0]->Elev = NWnode->Elev * LatFromSouth + SWnode->Elev * LatFromNorth;
	SubNodes[SubRowCt][0]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
	SubNodes[SubRowCt][0]->FlagSet(WCS_VECTORNODE_FLAG_WESTEDGE);
	} // for

// east side node allocation interval depends on the adjacent cell to the east
if (FractalLevel > FrdEast)
	EDiffRows = FrdDataRows[FractalLevel - FrdEast] - 1;
else
	EDiffRows = 1;
MultiLatLonInterval = LatLonInterval * EDiffRows;
for (SubRowCt = EDiffRows, LatFromNorth = MultiLatLonInterval; SubRowCt < LastRow; SubRowCt += EDiffRows, LatFromNorth += MultiLatLonInterval)
	{
	if (! SubNodes[SubRowCt][LastRow] && ! (SubNodes[SubRowCt][LastRow] = new VectorNode()))
		{
		goto ReturnFailure;
		} // else
	SubNodes[SubRowCt][LastRow]->DeleteLinksNoCrossCheck();
	LatFromSouth = 1.0 - LatFromNorth;
	SubNodes[SubRowCt][LastRow]->Lat = NEnode->Lat * LatFromSouth + SEnode->Lat * LatFromNorth;
	if (NEnode->Lon == SEnode->Lon)
		SubNodes[SubRowCt][LastRow]->Lon = NEnode->Lon;
	else
		SubNodes[SubRowCt][LastRow]->Lon = NEnode->Lon * LatFromSouth + SEnode->Lon * LatFromNorth;
	SubNodes[SubRowCt][LastRow]->Elev = NEnode->Elev * LatFromSouth + SEnode->Elev * LatFromNorth;
	SubNodes[SubRowCt][LastRow]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
	SubNodes[SubRowCt][LastRow]->FlagSet(WCS_VECTORNODE_FLAG_EASTEDGE);
	} // for

// allocate all the other nodes needed
for (SubRowCt = 1, LatFromNorth = LatLonInterval; SubRowCt < LastRow; ++SubRowCt, LatFromNorth += LatLonInterval)
	{
	LatFromSouth = 1.0 - LatFromNorth;
	for (SubColCt = 1, LonFromWest = LatLonInterval; SubColCt < LastRow; ++SubColCt, LonFromWest += LatLonInterval)
		{
		if (! SubNodes[SubRowCt][SubColCt] && ! (SubNodes[SubRowCt][SubColCt] = new VectorNode()))
			{
			goto ReturnFailure;
			} // else
		SubNodes[SubRowCt][SubColCt]->DeleteLinksNoCrossCheck();
		LonFromEast = 1.0 - LonFromWest;
		M1 = LatFromSouth * LonFromEast;
		M2 = LatFromSouth * LonFromWest;
		M3 = LatFromNorth * LonFromEast;
		M4 = LatFromNorth * LonFromWest;
		if (NWnode->Lat == NEnode->Lat && SWnode->Lat == SEnode->Lat)
			SubNodes[SubRowCt][SubColCt]->Lat = NWnode->Lat * LatFromSouth + SWnode->Lat * LatFromNorth;
		else
			SubNodes[SubRowCt][SubColCt]->Lat = NWnode->Lat * M1 + NEnode->Lat * M2
				+ SWnode->Lat * M3 + SEnode->Lat * M4;
		if (NWnode->Lon == SWnode->Lon && NEnode->Lon == SEnode->Lon)
			SubNodes[SubRowCt][SubColCt]->Lon = NWnode->Lon * LonFromEast + NEnode->Lon * LonFromWest;
		else
			SubNodes[SubRowCt][SubColCt]->Lon = NWnode->Lon * M1 + NEnode->Lon * M2
				+ SWnode->Lon * M3 + SEnode->Lon * M4;
		SubNodes[SubRowCt][SubColCt]->Elev = NWnode->Elev * M1 + NEnode->Elev * M2
			+ SWnode->Elev * M3 + SEnode->Elev * M4;
		SubNodes[SubRowCt][SubColCt]->FlagClear(WCS_VECTORNODE_FLAG_BOUNDARYNODE);
		} // for
	} // for

PolygonListPtr = (VectorPolygonListDouble **)&HeadList->NextPolygonList;
EffectsHead = (VectorPolygonListDouble *)HeadList->NextPolygonList;
// create polygons from the mesh
// for the first row
ThisRow = 0;
NextRow = 1;
FirstPtThisRow = 0;
NextPtThisRow = NDiffRows;
FirstPtNextRow = 1;
NextPtNextRow = 2;
Method = 3;
while (FirstPtThisRow <= LastRow && FirstPtNextRow < LastRow && (NextPtThisRow <= LastRow || NextPtNextRow < LastRow))
	{
	if (NDiffRows == 1)
		{
		// alternate method A, A, B, B, A, A...
		if (++Method > 3)
			Method = 0;
		} // if
	else
		{
		if (NextPtThisRow - FirstPtThisRow < 2 * (NextPtNextRow - FirstPtThisRow))
			// method A
			Method = 0;
		else
			// method B
			Method = 2;
		} // else
	if (Method < 2)	// method A
		{
		// link FirstPtThisRow, NextPtThisRow, FirstPtNextRow
		FirstNode = SubNodes[ThisRow][FirstPtThisRow];
		SecondNode = SubNodes[ThisRow][NextPtThisRow];
		ThirdNode = SubNodes[NextRow][FirstPtNextRow];
		FirstPtThisRow += NDiffRows;
		NextPtThisRow += NDiffRows;
		} // if
	else	// method B
		{
		// link FirstPtThisRow, NextPtNextRow, FirstPtNextRow
		FirstNode = SubNodes[ThisRow][FirstPtThisRow];
		SecondNode = SubNodes[NextRow][NextPtNextRow];
		ThirdNode = SubNodes[NextRow][FirstPtNextRow];
		FirstPtNextRow += 1;
		NextPtNextRow += 1;
		} // else
	// make a polygon, clone the nodes
	if (*PolygonListPtr = new VectorPolygonListDouble())
		{
		if (! (*PolygonListPtr)->MakeVectorPolygon(FirstNode, SecondNode, ThirdNode, HeadList->MyPolygon, false))
			{
			goto ReturnFailure;
			} // if
		PolygonListPtr = (VectorPolygonListDouble **)&(*PolygonListPtr)->NextPolygonList;
		} // if
	else
		{
		goto ReturnFailure;
		} // else
	} // while
// for the last row
ThisRow = LastRow;
NextRow = LastRow - 1;
FirstPtThisRow = 0;
NextPtThisRow = SDiffRows;
FirstPtNextRow = 1;
NextPtNextRow = 2;
Method = 3;
while (FirstPtThisRow <= LastRow && FirstPtNextRow < LastRow && (NextPtThisRow <= LastRow || NextPtNextRow < LastRow))
	{
	if (SDiffRows == 1)
		{
		// alternate method A, A, B, B, A, A...
		if (++Method > 3)
			Method = 0;
		} // if
	else
		{
		if (NextPtThisRow - FirstPtThisRow < 2 * (NextPtNextRow - FirstPtThisRow))
			// method A
			Method = 0;
		else
			// method B
			Method = 2;
		} // else
	if (Method < 2)
		{
		// link FirstPtThisRow, FirstPtNextRow, NextPtThisRow
		FirstNode = SubNodes[ThisRow][FirstPtThisRow];
		SecondNode = SubNodes[NextRow][FirstPtNextRow];
		ThirdNode = SubNodes[ThisRow][NextPtThisRow];
		FirstPtThisRow += SDiffRows;
		NextPtThisRow += SDiffRows;
		} // if
	else
		{
		// link FirstPtThisRow, FirstPtNextRow, NextPtNextRow
		FirstNode = SubNodes[ThisRow][FirstPtThisRow];
		SecondNode = SubNodes[NextRow][FirstPtNextRow];
		ThirdNode = SubNodes[NextRow][NextPtNextRow];
		FirstPtNextRow += 1;
		NextPtNextRow += 1;
		} // else
	// make a polygon, clone the nodes
	if (*PolygonListPtr = new VectorPolygonListDouble())
		{
		if (! (*PolygonListPtr)->MakeVectorPolygon(FirstNode, SecondNode, ThirdNode, HeadList->MyPolygon, false))
			{
			goto ReturnFailure;
			} // if
		PolygonListPtr = (VectorPolygonListDouble **)&(*PolygonListPtr)->NextPolygonList;
		} // if
	else
		{
		goto ReturnFailure;
		} // else
	} // while
// for the first column
ThisRow = 0;
NextRow = 1;
FirstPtThisRow = 0;
NextPtThisRow = WDiffRows;
FirstPtNextRow = 1;
NextPtNextRow = 2;
Method = 3;
while (FirstPtThisRow <= LastRow && FirstPtNextRow < LastRow && (NextPtThisRow <= LastRow || NextPtNextRow < LastRow))
	{
	if (WDiffRows == 1)
		{
		// alternate method A, A, B, B, A, A...
		if (++Method > 3)
			Method = 0;
		} // if
	else
		{
		if (NextPtThisRow - FirstPtThisRow < 2 * (NextPtNextRow - FirstPtThisRow))
			// method A
			Method = 0;
		else
			// method B
			Method = 2;
		} // else
	if (Method < 2)
		{
		// link FirstPtThisRow, FirstPtNextRow, NextPtThisRow
		FirstNode = SubNodes[FirstPtThisRow][ThisRow];
		SecondNode = SubNodes[FirstPtNextRow][NextRow];
		ThirdNode = SubNodes[NextPtThisRow][ThisRow];
		FirstPtThisRow += WDiffRows;
		NextPtThisRow += WDiffRows;
		} // if
	else
		{
		// link FirstPtThisRow, FirstPtNextRow, NextPtNextRow
		FirstNode = SubNodes[FirstPtThisRow][ThisRow];
		SecondNode = SubNodes[FirstPtNextRow][NextRow];
		ThirdNode = SubNodes[NextPtNextRow][NextRow];
		FirstPtNextRow += 1;
		NextPtNextRow += 1;
		} // else
	// make a polygon, clone the nodes
	if (*PolygonListPtr = new VectorPolygonListDouble())
		{
		if (! (*PolygonListPtr)->MakeVectorPolygon(FirstNode, SecondNode, ThirdNode, HeadList->MyPolygon, false))
			{
			goto ReturnFailure;
			} // if
		PolygonListPtr = (VectorPolygonListDouble **)&(*PolygonListPtr)->NextPolygonList;
		} // if
	else
		{
		goto ReturnFailure;
		} // else
	} // while
// for the last column
ThisRow = LastRow;
NextRow = LastRow - 1;
FirstPtThisRow = 0;
NextPtThisRow = EDiffRows;
FirstPtNextRow = 1;
NextPtNextRow = 2;
Method = 3;
while (FirstPtThisRow <= LastRow && FirstPtNextRow < LastRow && (NextPtThisRow <= LastRow || NextPtNextRow < LastRow))
	{
	if (EDiffRows == 1)
		{
		// alternate method A, A, B, B, A, A...
		if (++Method > 3)
			Method = 0;
		} // if
	else
		{
		if (NextPtThisRow - FirstPtThisRow < 2 * (NextPtNextRow - FirstPtThisRow))
			// method A
			Method = 0;
		else
			// method B
			Method = 2;
		} // else
	if (Method < 2)
		{
		// link FirstPtThisRow, NextPtThisRow, FirstPtNextRow
		FirstNode = SubNodes[FirstPtThisRow][ThisRow];
		SecondNode = SubNodes[NextPtThisRow][ThisRow];
		ThirdNode = SubNodes[FirstPtNextRow][NextRow];
		FirstPtThisRow += EDiffRows;
		NextPtThisRow += EDiffRows;
		} // if
	else
		{
		// link FirstPtThisRow, NextPtNextRow, FirstPtNextRow
		FirstNode = SubNodes[FirstPtThisRow][ThisRow];
		SecondNode = SubNodes[NextPtNextRow][NextRow];
		ThirdNode = SubNodes[FirstPtNextRow][NextRow];
		FirstPtNextRow += 1;
		NextPtNextRow += 1;
		} // else
	// make a polygon, clone the nodes
	if (*PolygonListPtr = new VectorPolygonListDouble())
		{
		if (! (*PolygonListPtr)->MakeVectorPolygon(FirstNode, SecondNode, ThirdNode, HeadList->MyPolygon, false))
			{
			goto ReturnFailure;
			} // if
		PolygonListPtr = (VectorPolygonListDouble **)&(*PolygonListPtr)->NextPolygonList;
		} // if
	else
		{
		goto ReturnFailure;
		} // else
	} // while
// for all the rest
for (ThisRow = 1, NextRow = 2, RowStyle = true; NextRow < LastRow && Success; ++ThisRow, ++NextRow, RowStyle = ! RowStyle)
	{
	FirstPtThisRow = 1;
	NextPtThisRow = 2;
	FirstPtNextRow = 1;
	NextPtNextRow = 2;
	Method = RowStyle ? 2: 0;
	while (NextPtThisRow < LastRow || NextPtNextRow < LastRow)
		{
		// alternate method A, A, B, B, A, A...
		if (++Method > 3)
			Method = 0;
		if (Method < 2)	// method A
			{
			// link FirstPtThisRow, NextPtThisRow, FirstPtNextRow
			FirstNode = SubNodes[ThisRow][FirstPtThisRow];
			SecondNode = SubNodes[ThisRow][NextPtThisRow];
			ThirdNode = SubNodes[NextRow][FirstPtNextRow];
			++FirstPtThisRow;
			++NextPtThisRow;
			} // if
		else	// method B
			{
			// link FirstPtThisRow, NextPtNextRow, FirstPtNextRow
			FirstNode = SubNodes[ThisRow][FirstPtThisRow];
			SecondNode = SubNodes[NextRow][NextPtNextRow];
			ThirdNode = SubNodes[NextRow][FirstPtNextRow];
			++FirstPtNextRow;
			++NextPtNextRow;
			} // else
		// make a polygon, clone the nodes
		if (*PolygonListPtr = new VectorPolygonListDouble())
			{
			if (! (*PolygonListPtr)->MakeVectorPolygon(FirstNode, SecondNode, ThirdNode, HeadList->MyPolygon, false))
				{
				goto ReturnFailure;
				} // if
			PolygonListPtr = (VectorPolygonListDouble **)&(*PolygonListPtr)->NextPolygonList;
			} // if
		else
			{
			goto ReturnFailure;
			} // else
		} // while
	} // for

// restore end of list to the Effect vector polygons
*PolygonListPtr = EffectsHead;
// remove the head of the list
EffectsHead = HeadList;
HeadList = (VectorPolygonListDouble *)HeadList->NextPolygonList;
EffectsHead->DeletePolygon();
delete EffectsHead;

return (Success);

ReturnFailure:

// restore end of list to the Effect vector polygons
if (EffectsHead)
	*PolygonListPtr = EffectsHead;

return (false);

} // Renderer::SubdivideTerrainPolygon

/*===========================================================================*/

int Renderer::MakeAllFractalDepthMaps(int ReportOptimum)
{
double CurTime, MaxPixelSize, LocalFrameRate, LocalProjectTime;
BusyWin *BWAN, *BWIM;
FILE *fFrd;
Joe *MyTurn;
RenderJob *CurJob;
RenderQItem *CurQ;
DEM *CreatedDEM;
short RenderSegStash;
int Success = 1, JobsDone, Initialized = 0, Result;
long Ct, CurFrame, FrameInt, FirstFrame, LastFrame, StashQNum, DEMCt;
unsigned long FractalDepth[10], MaxDepth = 0, OptDepth = 0, SumPolys;
BoundsPackage Conditions;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;
bool LoadIt, InsufficientNodes = false;
char FrameStr[64], filename[512], Str[512], TilingStash;

for (JobsDone = 0, CurJob = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); CurJob; CurJob = (RenderJob *)CurJob->Next)
	{
	if (! CurJob->Enabled || ! CurJob->Cam || ! CurJob->Options || ! CurJob->Options->TerrainEnabled)
		continue;
	JobsDone ++;
	} // for
	
if (! JobsDone)
	{
	UserMessageOK("Fractal Depth Maps", "There are no Render Jobs currently enabled that have their options set to render terrain. Fractal Depth maps have not been created.");
	return (0);
	} // if
	
Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
GlobalApp->AppEffects->GetRAHostProperties(&Prop);
FirstFrame = (long)(Prop.KeyNodeRange[0] * GlobalApp->MainProj->Interactive->GetFrameRate());
LastFrame = (long)(Prop.KeyNodeRange[1] * GlobalApp->MainProj->Interactive->GetFrameRate() + .99);
LocalFrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
LocalProjectTime = GlobalApp->MainProj->Interactive->GetActiveTime();

MaxPixelSize = GlobalApp->AppEffects->TerrainParamBase.DepthMapPixSize * 2;

if (! LocalLog)
	LocalLog = GlobalApp->StatusLog;

// determine frame interval from user 
sprintf(Str, "%d", FirstFrame);
if (GetInputString("Enter the first frame to scan for Fractal Map generation.",
	"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", Str))
	{
	FirstFrame = atoi(Str);

	sprintf(Str, "%d", LastFrame);
	if (GetInputString("Enter the last frame to scan for Fractal Map generation.",
		"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", Str))
		{
		LastFrame = atoi(Str);

		sprintf(Str, "%d", 5);
		if (LastFrame == FirstFrame || GetInputString("Enter frame interval to scan.\nThe smaller the number the longer this process will take but the\n less likely you will encounter holes in the rendered terrain!",
			"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", Str))
			{
			FrameInt = atoi(Str);
			if (FrameInt <= 0)
				FrameInt = 1;
			} // if
		else
			return (0);
		} // if
	else
		return (0);
	} // if
else
	return (0);

// delete all the old files 
GlobalApp->AppDB->ResetGeoClip();

for (MyTurn = GlobalApp->AppDB->GetFirst(); MyTurn; MyTurn = GlobalApp->AppDB->GetNext(MyTurn))
	{
	//if (MyTurn->TestFlags(WCS_JOEFLAG_ISDEM) && MyTurn->TestRenderFlags())	// removed test to work with scenarios
	if (MyTurn->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		#ifdef WCS_MULTIFRD
		TerrainParamEffect *CurTP;
		for (CurTP = (TerrainParamEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_TERRAINPARAM); CurTP; CurTP = (TerrainParamEffect *)CurTP->Next)
			{
			if (CurTP->Enabled)
				{
		#endif // WCS_MULTIFRD
				#ifdef WCS_MULTIFRD
				sprintf(Str, "%s%s", (char *)MyTurn->FileName(), CurTP->GetName());
				MakeNonPadPath(filename, GlobalApp->MainProj->dirname, Str, ".frd");
				#else // WCS_MULTIFRD
				MakeNonPadPath(filename, GlobalApp->MainProj->dirname, (char *)MyTurn->FileName(), ".frd");
				#endif // WCS_MULTIFRD
				Result = -2;
				if ((fFrd = PROJ_fopen(filename, "rb")) != NULL)
					{
					fclose(fFrd);
					Result = PROJ_remove(filename);
					} // else open succeeds 
				else
					{
					#ifdef WCS_MULTIFRD
					MakeCompletePath(filename, GlobalApp->MainProj->dirname, Str, ".frd");
					#else // WCS_MULTIFRD
					MakeCompletePath(filename, GlobalApp->MainProj->dirname, (char *)MyTurn->FileName(), ".frd");
					#endif // WCS_MULTIFRD
					if ((fFrd = PROJ_fopen(filename, "rb")) != NULL)
						{
						fclose(fFrd);
						Result = PROJ_remove(filename);
						} // else open succeeds 
					} // else
				if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOptTrue("debug_file_remove"))
					{
					if (Result == 0)
						sprintf(Str, "DEM Debug: %s: File deleted.", filename);
					else if (Result < -1)
						sprintf(Str, "Frd Debug: %s: File not found to delete.", filename);
					else
						sprintf(Str, "Frd Debug: %s: Error %d: Unable to delete file.", filename, errno);
					LocalLog->PostError(WCS_LOG_SEVERITY_WNG, Str);
					} // if
		#ifdef WCS_MULTIFRD
				} // if
			} // for
		#endif // WCS_MULTIFRD
		} // if 
	} // for

// init fractal depth array for reporting
for (Ct = 0; Ct < 10; Ct ++)
	FractalDepth[Ct] = 0;

for (JobsDone = 0, CurJob = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); Success && CurJob; CurJob = (RenderJob *)CurJob->Next)
	{
	if (! CurJob->Enabled || ! CurJob->Cam || ! CurJob->Options || ! CurJob->Options->TerrainEnabled)
		continue;
	JobsDone ++;

	// set render segments to 1 temporarily
	if ((RenderSegStash = CurJob->Options->RenderImageSegments) > 1)
		CurJob->Options->RenderImageSegments = 1;
	if (TilingStash = CurJob->Options->TilingEnabled)
		CurJob->Options->TilingEnabled = 0;

	#ifdef WCS_RENDER_SCENARIOS
	// init enabled states at first frame to be rendered before init effects
	GlobalApp->AppEffects->InitScenarios(CurJob->Scenarios, FirstFrame / GlobalApp->MainProj->Interactive->GetFrameRate(), 
		GlobalApp->AppDB);
	#endif // WCS_RENDER_SCENARIOS

	// initalize renderer for fractal map creation
	if (InitForProcessing(CurJob, GlobalApp, GlobalApp->AppEffects, GlobalApp->AppImages, 
		GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->StatusLog, TRUE))	// true=elevation only
		{
		double invLocalFrameRate = 1.0 / LocalFrameRate;

		Initialized = 1;
		BWAN = new BusyWin ("Animation", LastFrame - FirstFrame + FrameInt, 'BWAN', 1);

		for (CurFrame = FirstFrame; Success && CurFrame <= LastFrame; CurFrame += FrameInt)
			{
			CurTime = CurFrame * invLocalFrameRate;

			#ifdef WCS_RENDER_SCENARIOS
			if (CurFrame != FirstFrame && GlobalApp->AppEffects->UpdateScenarios(CurJob->Scenarios, CurTime, GlobalApp->AppDB))
				{
				Cleanup(false, false, -1, false);
				if (! InitForProcessing(CurJob, GlobalApp, GlobalApp->AppEffects, GlobalApp->AppImages, 
					GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->StatusLog, TRUE))	// true=elevation only
					{
					Success = 0;
					break;
					} // if init error
				} // if
			#endif // WCS_RENDER_SCENARIOS

			for (PanoPanel = 0; Success && PanoPanel < CurJob->Cam->PanoPanels; PanoPanel ++)
				{
				InitFrame(CurTime, CurFrame, TRUE);		// TRUE causes time to be set
				SetBoundsPackage(&Conditions);

				if ((Success = DEMCue->SortList(Cam, 1, &Conditions)))	// pass 1 = turn off out-of-bounds DEMs
					{
					if (DEMCue->ActiveItems > 0)
						{
						if (CurJob->Cam->PanoPanels > 1)
							sprintf(FrameStr, "Frame %d/%d, Panel %d/%d", CurFrame, LastFrame, PanoPanel + 1, CurJob->Cam->PanoPanels);
						else
							sprintf(FrameStr, "Frame %d/%d", CurFrame, LastFrame);
						BWIM = new BusyWin (FrameStr, DEMCue->ActiveItems, 'BWIM', 1);

						for (DEMCt = 0, CurQ = DEMCue->GetFirst(StashQNum); Success && CurQ; CurQ = DEMCue->GetNext(StashQNum))
							{
							CreatedDEM = NULL;
							if (CurQ->QDEM || (CreatedDEM = CurQ->QDEM = new DEM()))
								{
								// Set current DEM in CurDEM which is Renderer scope
								CurDEM = CurQ->QDEM;

								LoadIt = CurDEM->RawMap ? false: true;
								if (! LoadIt || CurDEM->AttemptLoadDEM(CurQ->QJoe, 1, ProjectBase))
									{
									// see if it is a NULL DEM and skip it if it is
									if (CurDEM->GetNullReject() && CurDEM->MaxEl() < CurDEM->MinEl())
										goto MapCleanup;
									// transfer data to VertexDEMs
									if (! LoadIt || (Success = CurDEM->TransferToVerticesXYZ()))
										{
										if (LoadIt)
											{
											// load fractal depth maps
											#ifdef WCS_MULTIFRD
											if (! CurDEM->AttemptLoadFractalFile((char *)CurQ->QJoe->FileName(), DefaultTerrainPar->GetName(), ProjectBase))
											#else // WCS_MULTIFRD
											if (! CurDEM->AttemptLoadFractalFile((char *)CurQ->QJoe->FileName(), ProjectBase))
											#endif // WCS_MULTIFRD
												{
												if (! CurDEM->AllocFractalMap(TRUE))
													{
													Success = 0;
													goto MapCleanup;
													} // else no room for data
												} // if load failed
											} // if
										else if (! CurDEM->FractalMap)
											{
											// DEM already loaded but no fractal map
											if (! CurDEM->AllocFractalMap(TRUE))
												{
												Success = 0;
												goto MapCleanup;
												} // else no room for data
											} // else if
											
										// some init stuff
										DefaultFrd = min(DefaultTerrainPar->FractalDepth, CurQ->MaxFrd);

										// this will be used by DEM rendering func., DEMstr is Renderer scope
										sprintf(DEMstr, "DEM %d/%d", DEMCt + 1, DEMCue->ActiveItems);
										if (ShadowMapInProgress)
											strcat(DEMstr, "Shadow");

										// find a list of VectorPolygons that include this DEM
										if (CurQ->DEMVectorPoly || (CurQ->DEMVectorPoly = new VectorPolygon(DBase, DefaultPlanetOpt, CurQ->QJoe, false, InsufficientNodes)))
											{
											if (CurQ->DEMVectorPoly->BBox || CurQ->DEMVectorPoly->SetBoundingBox())
												{
												bool LocalSuccess = true;
												if (! CurQ->PolygonListformed)
													{
													CurQ->DEMPolyList = EvalEffects->CreateBoundedPolygonList(NULL, CurQ->DEMVectorPoly->BBox, LocalSuccess);
													CurQ->PolygonListformed = true;
													} // if
												if (! LocalSuccess)
													{
													Success = 0;
													UserMessageOK("Render DEM", "Out of memory creating list of DEM-bounded effects!");
													goto MapCleanup;
													} // if
												} // if
											} // if

										if (LoadIt)
											{
											// some basic vertex manipulation - displace while in native coord sys
											// convert to default lat/lon
											if (Success)
												Success = CurDEM->TransferXYZtoLatLon();
											if (Success)
												Success = ScaleDEMElevs(CurDEM);
											// Call a method on the EffectsLib that supplies elevation alterations that may apply within the DEM.
											EffectsBase->EvalHighLowEffectExtrema(CurQ->DEMPolyList, &RendData, CurDEM->RelativeEffectAdd,
												CurDEM->RelativeEffectSubtract, CurDEM->AbsoluteEffectMax, CurDEM->AbsoluteEffectMin);
											} // if
										// project these vertices for use in clipping
										// Only needed in VNS 3 because of desire to sort polygons front to back for rendering efficiency
										// There is probably a better way to accomplish the re-ordering based on projecting just the corner vertices.
										if (Success)
											Success = ProjectDEMVertices(CurDEM);

										// off we go, depth map this puppy
										if (Success = CreateFractalMap(MaxPixelSize, FractalDepth))
											{
											#ifdef WCS_MULTIFRD
											if (! CurDEM->AttemptSaveFractalFile((char *)CurQ->QJoe->FileName(), DefaultTerrainPar->GetName(), ProjectBase))
											#else // WCS_MULTIFRD
											if (! CurDEM->AttemptSaveFractalFile((char *)CurQ->QJoe->FileName(), ProjectBase))
											#endif // WCS_MULTIFRD
												{
												Success = 0;
												LocalLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, (char *)CurQ->QJoe->FileName());
												UserMessageOK("Fractal Depth Maps", "Error saving Fractal Depth File! Possible reasons include disk full or file permissions denied.\nOperation terminated.");
												} // if file exists 
											} // if
										} // if
									MapCleanup:
									// cleanup
									CurDEM->FreeRawElevs();
									} // if loaded DEM
								else
									{
									Success = 0;
									LocalLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, (char *)CurQ->QJoe->FileName());
									UserMessageOK("Fractal Depth Maps", "Error loading DEM file! Possible reasons include file permissions denied.\nOperation terminated.");
									} // else no file
								if (CreatedDEM)
									{
									delete CurQ->QDEM;
									CurQ->QDEM = NULL;
									} // if
								} // if CurQ->QDEM
							BWIM->Update(++ DEMCt);
							} // for DEMCt
						if (BWIM)
							delete BWIM;
						} // if DEMs > 0
					} // if list sorted OK
				} // for PanoPanel
			BWAN->Update(CurFrame - FirstFrame + FrameInt);
			// ensure that the last frame gets scanned
			if (LastFrame != CurFrame && LastFrame - CurFrame < FrameInt)
				CurFrame = LastFrame - FrameInt;
			} // for CurFrame 
		if (BWAN)
			delete BWAN;
		#ifdef WCS_RENDER_SCENARIOS
		GlobalApp->AppEffects->RestoreScenarios(CurJob->Scenarios, GlobalApp->AppDB);
		#endif // WCS_RENDER_SCENARIOS
		Cleanup(false, false, 1, false);
		GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
		} // if renderer initialized

	// set render segments to back
	if (RenderSegStash > 1)
		CurJob->Options->RenderImageSegments = RenderSegStash;
	if (TilingStash)
		CurJob->Options->TilingEnabled = TilingStash;
	} // for CurJob

if (JobsDone)
	{
	// report optimal fractal depth
	for (Ct = 0, SumPolys = 0; Ct < 8; Ct ++)
		{
		if (FractalDepth[Ct])
			{
			MaxDepth = Ct;
			SumPolys += FractalDepth[Ct];
			} // if
		} // for
	OptDepth = (unsigned long)(.98 * SumPolys);
	for (Ct = 0, SumPolys = 0; Ct < 8; Ct ++)
		{
		SumPolys += FractalDepth[Ct];
		if (SumPolys >= OptDepth)
			{
			OptDepth = Ct;
			break;
			} // if
		} // for
	if (OptDepth > MaxDepth)
		OptDepth = MaxDepth;
	if (ReportOptimum)
		{
		if (SumPolys > 0)
			{
			if (OptDepth == MaxDepth)
				sprintf(Str, "Maximum fractal depth found was %d. Fractal depth 0 will work for most projects, a higher fractal depth may be \
required for Environment-based ecosystems, terrain displacement texture, Streams, and Lakes.", MaxDepth);
			else
				sprintf(Str, "Maximum fractal depth found was %d. Fractal depth 0 will work for most projects, a higher fractal depth may be \
required for Environment-based ecosystems, terrain displacement texture, Streams, and Lakes. You might try %d in those cases.", MaxDepth, OptDepth);
			UserMessageOK("Fractal Depth Map", Str);
			} // if
		else
			{
			UserMessageOK("Fractal Depth Maps", "No terrain data was found in the image area.");
			} // else
		} // if
	else
		{
		if (SumPolys > 0)
			{
			sprintf(Str, "Fractal Depth Maps: Maximum fractal depth found was %d.", MaxDepth);
			LocalLog->PostStockError(WCS_LOG_MSG_NULL, Str);
			} // if
		else
			{
			sprintf(Str, "Fractal Depth Maps: No terrain data was found in the image area.");
			LocalLog->PostStockError(WCS_LOG_MSG_NULL, Str);
			} // else
		} // else
	} // if
else
	UserMessageOK("Create Fractal Maps", "There are no enabled Render Jobs that\n have both a Camera and Render Options.\nDepth Maps have not been created.");

if (! Success)
	GlobalApp->AppEffects->TerrainParamBase.FrdInvalid = 1;
else
	GlobalApp->AppEffects->TerrainParamBase.FrdInvalid = 0;

if (Initialized)
	{
	// restore stashed time and frame rate
	GlobalApp->MainProj->Interactive->SetActiveTimeAndRate(LocalProjectTime, LocalFrameRate);
	// reset everything in application by time
	GlobalApp->UpdateProjectByTime();
	// send notification that time has changed even though we're just resetting it.
	Changes[0] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
	Changes[1] = NULL;
	GlobalApp->MainProj->Interactive->GenerateNotify(Changes, NULL);
	} // if

return (Success);

} // Renderer::MakeAllFractalDepthMaps

/*===========================================================================*/

int Renderer::CreateFractalMap(double MaxPixelSize, unsigned long *FractalDepth)
{
BusyWin *BWDE = NULL;
long RowZip, PolyZip, RowCt, ColCt;
int Success = 1;

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, CurDEM->pLonEntries - 1, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(CurDEM->pLonEntries - 1, DEMstr);
	} // else

// note that RowCt is incremented in the busy win update at the end of the loop
for (RowCt = RowZip = PolyZip = 0; Success && RowCt < (long)CurDEM->pLonEntries - 1; RowZip += CurDEM->pLatEntries, PolyZip += (CurDEM->pLatEntries - 1))
	{
	//VertNumber = RowZip;
	//PolyNumber = PolyZip * 2;
	#pragma omp parallel for private(ColCt)
	for (ColCt = 0; ColCt < (long)CurDEM->pLatEntries - 1; ++ColCt)//, ++VertNumber, PolyNumber += 2)
		{
		long PolyNumber, VertNumber, MaxFract = 0;
		VertNumber = RowZip + ColCt;
		#ifdef WCS_BUILD_VNS
		if (CurDEM->NullReject && (CurDEM->TestNullValue(VertNumber) || CurDEM->TestNullValue(VertNumber + 1) || CurDEM->TestNullValue(VertNumber + CurDEM->pLatEntries) || CurDEM->TestNullValue(VertNumber + CurDEM->pLatEntries + 1)))
			{
			continue;
			} // if
		else
		#endif // WCS_BUILD_VNS
			{										
			PolyNumber = (PolyZip + ColCt) * 2;
			MaxFract = 0;
			// render two polygons at every VertNumber station.	VertNumber is vertex at lower left (southwest)
			if ((MaxFract = FractalLevelPoly(MaxPixelSize, VertNumber, CurDEM->pLatEntries)) > CurDEM->FractalMap[PolyNumber])
				{
				if (MaxFract >= 8)
					MaxFract = 7;
				CurDEM->FractalMap[PolyNumber] = CurDEM->FractalMap[PolyNumber + 1] = (signed char)MaxFract;
				} // if
			if (FractalDepth && MaxFract >= 0)
				{
				#pragma omp critical (OMP_CRITICAL_FRDSUMINCREMENT)
				++FractalDepth[MaxFract];
				} // if
			} // else
		} // for
	if (IsCamView)
		{
		if(BWDE->Update(++RowCt))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(++RowCt);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (BWDE)
	delete (BWDE);
if (Master)
	Master->ProcClear();

return (Success);

} // Renderer::MakeFractalMap

/*===========================================================================*/

long Renderer::FractalLevelPoly(double MaxPixelSize, unsigned long V0, unsigned long LatEntries)
{
double ViewVec[3], PolySide[2][3], Normal[3];
long MaxFract = 0;
long Fract, a, b, c, d, e, f, g, h, i, j, k, l;
unsigned long VertNumArray[4];
bool HiddenRemovalSafe;

VertNumArray[0] = V0;
VertNumArray[1] = V0 + 1;
VertNumArray[2] = V0 + LatEntries;
VertNumArray[3] = V0 + LatEntries + 1;

// bounds testing is the second-least expensive culling method
if (! TestPolygonInBounds(VertNumArray, HiddenRemovalSafe))
	return (-1);

// test for backface
if (BackfaceCull && HiddenRemovalSafe)
	{
	// test both halves of the cell for visibility. If either one is visible need to render the polygon
	ViewVec[0] = (CurDEM->Vertices[VertNumArray[0]].XYZ[0] - Cam->CamPos->XYZ[0] + CurDEM->Vertices[VertNumArray[1]].XYZ[0] - Cam->CamPos->XYZ[0] + CurDEM->Vertices[VertNumArray[3]].XYZ[0] - Cam->CamPos->XYZ[0]);
	ViewVec[1] = (CurDEM->Vertices[VertNumArray[0]].XYZ[1] - Cam->CamPos->XYZ[1] + CurDEM->Vertices[VertNumArray[1]].XYZ[1] - Cam->CamPos->XYZ[1] + CurDEM->Vertices[VertNumArray[3]].XYZ[1] - Cam->CamPos->XYZ[1]);
	ViewVec[2] = (CurDEM->Vertices[VertNumArray[0]].XYZ[2] - Cam->CamPos->XYZ[2] + CurDEM->Vertices[VertNumArray[1]].XYZ[2] - Cam->CamPos->XYZ[2] + CurDEM->Vertices[VertNumArray[3]].XYZ[2] - Cam->CamPos->XYZ[2]);
	FindPosVector(PolySide[0], CurDEM->Vertices[VertNumArray[1]].XYZ, CurDEM->Vertices[VertNumArray[0]].XYZ);
	FindPosVector(PolySide[1], CurDEM->Vertices[VertNumArray[3]].XYZ, CurDEM->Vertices[VertNumArray[0]].XYZ);
	UnitVector(ViewVec);
	SurfaceNormal(Normal, PolySide[1], PolySide[0]);
	if (! SurfaceVisible(Normal, ViewVec, 0))
		{
		ViewVec[0] = (CurDEM->Vertices[VertNumArray[0]].XYZ[0] - Cam->CamPos->XYZ[0] + CurDEM->Vertices[VertNumArray[3]].XYZ[0] - Cam->CamPos->XYZ[0] + CurDEM->Vertices[VertNumArray[2]].XYZ[0] - Cam->CamPos->XYZ[0]);
		ViewVec[1] = (CurDEM->Vertices[VertNumArray[0]].XYZ[1] - Cam->CamPos->XYZ[1] + CurDEM->Vertices[VertNumArray[3]].XYZ[1] - Cam->CamPos->XYZ[1] + CurDEM->Vertices[VertNumArray[2]].XYZ[1] - Cam->CamPos->XYZ[1]);
		ViewVec[2] = (CurDEM->Vertices[VertNumArray[0]].XYZ[2] - Cam->CamPos->XYZ[2] + CurDEM->Vertices[VertNumArray[3]].XYZ[2] - Cam->CamPos->XYZ[2] + CurDEM->Vertices[VertNumArray[2]].XYZ[2] - Cam->CamPos->XYZ[2]);
		FindPosVector(PolySide[0], CurDEM->Vertices[VertNumArray[3]].XYZ, CurDEM->Vertices[VertNumArray[0]].XYZ);
		FindPosVector(PolySide[1], CurDEM->Vertices[VertNumArray[2]].XYZ, CurDEM->Vertices[VertNumArray[0]].XYZ);
		UnitVector(ViewVec);
		SurfaceNormal(Normal, PolySide[1], PolySide[0]);
		if (! SurfaceVisible(Normal, ViewVec, 0))
			return (-1);
		} // if 
	} // if

a = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[0]);
a = abs(a);
b = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[0]);
b = abs(b);
Fract = max(a, b);
c = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[0]);
c = abs(c);
Fract = max(Fract, c);
d = quickftol(CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[0]);
d = abs(d);
Fract = max(Fract, d);
e = quickftol(CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[0]);
e = abs(e);
Fract = max(Fract, e);
f = quickftol(CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[0] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[0]);
f = abs(f);
Fract = max(Fract, f);
g = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[1]);
g = abs(g);
Fract = max(Fract, g);
h = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[1]);
h = abs(h);
Fract = max(Fract, h);
i = quickftol(CurDEM->Vertices[VertNumArray[0]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[1]);
i = abs(i);
Fract = max(Fract, i);
j = quickftol(CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[1]);
j = abs(j);
Fract = max(Fract, j);
k = quickftol(CurDEM->Vertices[VertNumArray[1]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[1]);
k = abs(k);
Fract = max(Fract, k);
l = quickftol(CurDEM->Vertices[VertNumArray[2]].ScrnXYZ[1] - CurDEM->Vertices[VertNumArray[3]].ScrnXYZ[1]);
l = abs(l);
Fract = max(Fract, l);

while (Fract > MaxPixelSize && MaxFract < 255)
	{
	Fract >>= 1;  //lint !e704
	++MaxFract;
	} // while 

return (MaxFract);

} // Renderer::FractalLevel

#endif // WCS_VECPOLY_EFFECTS
