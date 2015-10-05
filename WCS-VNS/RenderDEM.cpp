// RenderDEM.cpp
// Functions for rendering a terrain quad in World Construction Set.
// Functions moved from Render.cpp 10/17/99.
// Includes methods for Renderer, PolygonEdge, RenderQitem, RenderQ, BoundsPackage, DEMBoundingBox
// Created by Gary R. Huber. 
// Copyright 1999 by Questar productions. All rights reserved.

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
#include "EffectEval.h"
#include "Lists.h"
#include "QuantaAllocator.h"

#ifndef WCS_VECPOLY_EFFECTS
extern NotifyTag ThawEvent[2];
#endif // WCS_VECPOLY_EFFECTS

//#define ENABLE_RENDER_TIMER
#ifdef ENABLE_RENDER_TIMER
double RenderTimer1, RenderTimer2;
#endif // ENABLE_RENDER_TIMER

#ifdef WCS_BUILD_DEMO
unsigned long MemFuck;
#endif // WCS_BUILD_DEMO

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::RenderTerrain(long FrameNumber, long Field, double CurTime)
#else // WCS_VECPOLY_EFFECTS
int Renderer::RenderTerrain(long FrameNumber, long Field)
#endif // WCS_VECPOLY_EFFECTS
{
BoundsPackage Conditions;
RenderQItem *CurQ;
BusyWin *BWIM = NULL;
DEM *CreatedDEM;
int Success = 1, MakeFractalMap;
long StashQNum, Ct;
#ifdef WCS_VECPOLY_EFFECTS
bool LoadIt, InsufficientNodes = false, RecalcMaterialsEachFrame;
#endif // WCS_VECPOLY_EFFECTS
char FrameStr[50];

// this stuff is all needed in order to do bounds checking on RenderQ
SetBoundsPackage(&Conditions);
RecalcMaterialsEachFrame = EffectsBase->AnimateMaterials();

#ifdef ENABLE_RENDER_TIMER
RenderTimer1 = RenderTimer2 = 0;
#endif // ENABLE_RENDER_TIMER

if ((Success = DEMCue->SortList(Cam, 1, &Conditions)))	// pass 1 = turn off out-of-bounds DEMs
	{
	if (DEMCue->ActiveItems > 0)
		{

		// interface update
		if (ShadowMapInProgress)
			{
			strcpy(FrameStr, "Shadows");
			} // if
		else
			{
			if (Cam->FieldRender)
				{
				sprintf(FrameStr, "Frame %d", FrameNumber);
				if (! Field)
					strcat(FrameStr, "A");
				else
					strcat(FrameStr, "B");
				} // if 
			else
				sprintf(FrameStr, "Frame %d", FrameNumber);
			} // else

		if (IsCamView)
			{
			BWIM = new BusyWin(FrameStr, DEMCue->ActiveItems, 'BWIM', 1);
			} // if
		else
			{
			Master->FrameGaugeInit(DEMCue->ActiveItems);
			} // else

		// render the list
		for (Ct = 0, CurQ = DEMCue->GetFirst(StashQNum); Success && CurQ; Ct ++, CurQ = DEMCue->GetNext(StashQNum))
			{
			// lock 'n load DEM
			CreatedDEM = NULL;
			if (CurQ->QDEM || (CreatedDEM = CurQ->QDEM = new DEM()))
				{
				// Set current DEM in CurDEM which is Renderer scope
				CurDEM = CurQ->QDEM;

				#ifdef WCS_VECPOLY_EFFECTS
				LoadIt = CurDEM->RawMap ? false: true;
				if (! LoadIt || CurDEM->AttemptLoadDEM(CurQ->QJoe, 1, ProjectBase))
				#else // WCS_VECPOLY_EFFECTS
				if (CurDEM->AttemptLoadDEM(CurQ->QJoe, 1, ProjectBase))
				#endif // WCS_VECPOLY_EFFECTS
					{
					#ifdef WCS_VECPOLY_EFFECTS
					if (LoadIt)
						{
						DEMMemoryCurrentSize += CurDEM->CalcFullMemoryReq();
						if (DEMMemoryCurrentSize > ProjectBase->Prefs.DEMMemoryLimit * 1000000)
							{
							while (FreeSomeDEMResources() && (DEMMemoryCurrentSize > ProjectBase->Prefs.DEMMemoryLimit * 1000000))
								{
								// all the work is done in the while statement
								} // while
							} // if
						} // if
					#endif // WCS_VECPOLY_EFFECTS
					// tell AppMem the DEM size so it can warn if memory failure that DEMs may be too large
					AppMem_LogDEMSizes(CurDEM->LonEntries(), CurDEM->LatEntries());
					// see if it is a NULL DEM and skip it if it is
					if (CurDEM->GetNullReject() && CurDEM->MaxEl() < CurDEM->MinEl())
						goto MapCleanup;
					// log the DEM for debugging purposes
					if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("list_DEMs"))
						{
						char LogText[512];
						sprintf(LogText, "DEM \"%s\" rendered, frame %d", CurQ->QJoe->GetBestName(), FrameNumber);
						LocalLog->PostError(WCS_LOG_SEVERITY_DTA, LogText);
						} // if
					// transfer data to VertexDEMs
					#ifdef WCS_VECPOLY_EFFECTS
					if (! LoadIt || (Success = CurDEM->TransferToVerticesXYZ()))
					#else // WCS_VECPOLY_EFFECTS
					if (Success = CurDEM->TransferToVerticesXYZ())
					#endif // WCS_VECPOLY_EFFECTS
						{
						// compute default fractal depth
						DefaultFrd = min(DefaultTerrainPar->FractalDepth, CurQ->MaxFrd);
						// precalc ElScale multiplicative factor
						ElScaleMult = (CurDEM->pElScale / ELSCALE_METERS);
						// load fractal depth maps
						MakeFractalMap = 0;
						if (EffectsBase->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_DEPTHMAPS ||
							 EffectsBase->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_VARIABLE)
							{
							MakeFractalMap = 1;
							if (EffectsBase->TerrainParamBase.FractalMethod == WCS_FRACTALMETHOD_DEPTHMAPS)
								{
								#ifdef WCS_VECPOLY_EFFECTS
								if (LoadIt)
									{
								#endif // WCS_VECPOLY_EFFECTS
									#ifdef WCS_MULTIFRD
									if (! CurDEM->AttemptLoadFractalFile((char *)CurQ->QJoe->FileName(), DefaultTerrainPar->GetName(), ProjectBase))
									#else // WCS_MULTIFRD
									if (! CurDEM->AttemptLoadFractalFile((char *)CurQ->QJoe->FileName(), ProjectBase))
									#endif // WCS_MULTIFRD
										{
										// don't care so much if frd maps not found for shadow maps. They must be out of the camera's view
										if (! ShadowMapInProgress)
											{
											if (! FrdWarned )
												{
												// UserMessageOKCAN("Render DEM", "Error loading at least one Fractal Depth Map file!\nProceed without it?", 1)
												LocalLog->PostError(WCS_LOG_SEVERITY_ERR, "Error loading at least one Fractal Depth Map file! Variable Fractal Depth will be used when FRD map file is missing.");
												// The code below was used when this was a modal requester that could cancel rendering
												// It is preserved only for posterity.
												//Success = 0;
												//goto MapCleanup;
												} // if cancel
											FrdWarned = 1;
											} // if
										} // if load failed
									else
										MakeFractalMap = 0;
								#ifdef WCS_VECPOLY_EFFECTS
									} // if
								else
									MakeFractalMap = 0;
								#endif // WCS_VECPOLY_EFFECTS
								} // if depth maps
							if (! CurDEM->FractalMap && ! CurDEM->AllocFractalMap(TRUE))
								{
								UserMessageOK("Render DEM", "Out of memory allocating Fractal Map array!");
								Success = 0;
								goto MapCleanup;
								} // variable depth & no memory for fractal map 
							if (MakeFractalMap)
								memset(CurDEM->FractalMap, -1, CurDEM->FractalMapSize());
							} // if depth maps or variable frd 
						#ifdef WCS_VECPOLY_EFFECTS
						else if (LoadIt && CurDEM->AllocFractalMap(TRUE))
						#else // WCS_VECPOLY_EFFECTS
						else if (CurDEM->AllocFractalMap(TRUE))
						#endif // WCS_VECPOLY_EFFECTS
							{
							memset(CurDEM->FractalMap, DefaultFrd, CurDEM->FractalMapSize());
							#ifdef WCS_BUILD_VNS
							CurDEM->NullRejectSetFractalMap();
							#endif // WCS_BUILD_VNS
							} // constant depth
						#ifdef WCS_VECPOLY_EFFECTS
						else if (LoadIt)
						#else // WCS_VECPOLY_EFFECTS
						else
						#endif // WCS_VECPOLY_EFFECTS
							{
							UserMessageOK("Render DEM", "Out of memory allocating Fractal Map array!");
							Success = 0;
							goto MapCleanup;
							} // constant depth & no memory for fractal map 

						// this will be used by DEM rendering func.
						sprintf(DEMstr, "DEM %d/%d", Ct + 1, DEMCue->ActiveItems);
						if (Exporter)
							sprintf(DEMstr, "DEM %d/%d, %s", Ct + 1, DEMCue->ActiveItems, TileStr);

						#ifdef WCS_VECPOLY_EFFECTS
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
						#endif // WCS_VECPOLY_EFFECTS

						// off we go, render this puppy
						#ifdef WCS_VECPOLY_EFFECTS
						Success = RenderDEMPoly(MakeFractalMap, LoadIt, CurQ->DEMVectorPoly, CurQ->DEMPolyList, CurTime);
						#else // WCS_VECPOLY_EFFECTS
						Success = RenderDEM(MakeFractalMap);
						#endif // WCS_VECPOLY_EFFECTS
						} // if
					MapCleanup:
					// cleanup
					#ifndef WCS_VECPOLY_EFFECTS
					CurDEM->FreeRawElevs();
					#else // WCS_VECPOLY_EFFECTS
					if (RecalcMaterialsEachFrame)	// totally useless block to satisfy compiler that wants some code after the label.
						{
						CurDEM->FreeVPData(true);
						CurDEM->FreeRawElevs();
						DEMMemoryCurrentSize -= CurDEM->CalcFullMemoryReq();
						} // if
					#endif // WCS_VECPOLY_EFFECTS
					} // if loaded DEM
				else
					{
					LocalLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, (char *)CurQ->QJoe->FileName());
					} // else no file
				if (CreatedDEM)
					{
					#ifndef WCS_VECPOLY_EFFECTS
					delete CurQ->QDEM;
					CurQ->QDEM = NULL;
					#endif // WCS_VECPOLY_EFFECTS
					} // if
				CurDEM = NULL;	// set this NULL so that it isn't identified as in use by QuantaAllocator free-er.
				} // if DEM created

			// update progress
			if (IsCamView)
				{
				if (BWIM)
					{
					if (BWIM->Update(Ct + 1))
						Success = 0;
					} // if Image Busy Window open 
				} // if
			else
				{
				Master->FrameGaugeUpdate(Ct + 1);
				if (! Master->IsRunning())
					{
					Success = 0;
					} // if
				} // else
			} // for each render-enabled DEM
		} // if
	else
		{
		LocalLog->PostStockError(WCS_LOG_MSG_NULL, "Render List: No DEMs within view.");
		} // else
	} // if

#ifdef ENABLE_RENDER_TIMER
{
char TimeStr[64];
sprintf(TimeStr, "\nTimer1 %le\n", RenderTimer1);
OutputDebugString(TimeStr);
sprintf(TimeStr, "Timer2 %le\n", RenderTimer2);
OutputDebugString(TimeStr);
}
#endif // ENABLE_RENDER_TIMER

if (Master)
	Master->FrameGaugeClear();
if (BWIM)
	delete BWIM;

return (Success);

} // Renderer::RenderTerrain

/*===========================================================================*/

int Renderer::RenderDEM(int MakeFractalMap)
{
char CompleteDEMStr[256];
int Success = 1;
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

if (CurDEM)
	{
	// some basic vertex manipulation - displace while in native coord sys
	Success = DisplaceDEMVertices(CurDEM);
	// convert to default lat/lon
	if (Success)
		Success = CurDEM->TransferXYZtoLatLon();
	if (Success)
		Success = ApplyFractalDepthTexture(CurDEM);
	if (Success)
		Success = ScaleDEMElevs(CurDEM);
	// project these vertices for use in clipping
	if (Success)
		Success = ProjectDEMVertices(CurDEM);

	// create fractal map if needed
	if (Success && MakeFractalMap && CurDEM->FractalMap)
		Success = CreateFractalMap(EffectsBase->TerrainParamBase.DepthMapPixSize * 2.0, NULL);

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

		// in this scheme we need to create an array of all the polygons we plan to render,
		// ie. the ones with fractal map values >= 0. The array should consist of structures that contain the
		// polygon number and the Q value.

		// compute the number of possible polygons
		NumPolys = (CurDEM->pLatEntries - 1) * (CurDEM->pLonEntries - 1) * 2;
		// add the number of possible foliage and 3D object and fence vertices
		NumPolys += EffectsBase->CountRenderItemVertices(&RendData, DBase);

		// create the array of a size large enough to hold all the polygons in the DEM
		#ifdef WCS_BUILD_DEMO
		if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort) + MemFuck, 0))
		#else // WCS_BUILD_DEMO
		if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort), 0))
		#endif // WCS_BUILD_DEMO
			{
			PolyCt = PolyNumber = VertNumber = 0;	// this will count the polygons as they are inserted into the array
			for (CurDEM->Lr = 0; CurDEM->Lr < (long)CurDEM->pLonEntries - 1; CurDEM->Lr ++, VertNumber ++)
				{
				for (CurDEM->Lc = 0; CurDEM->Lc < (long)CurDEM->pLatEntries - 1; CurDEM->Lc ++, VertNumber ++, PolyNumber ++)
					{                                         //   _
					// test two polygons at every VertNumber station !\|	VertNumber is vertex at lower left (southwest)
					if (! CurDEM->FractalMap || CurDEM->FractalMap[PolyNumber] >= 0)
						{
						PolyArray[PolyCt].PolyQ = (float)((CurDEM->Vertices[VertNumber].ScrnXYZ[2] + CurDEM->Vertices[VertNumber + CurDEM->pLatEntries].ScrnXYZ[2] + CurDEM->Vertices[VertNumber + 1].ScrnXYZ[2]) * .33333 - ShadowMapDistanceOffset);
						PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_DEM;
						PolyArray[PolyCt ++].PolyNumber = PolyNumber;
						} // if
					PolyNumber ++;
					if (! CurDEM->FractalMap || CurDEM->FractalMap[PolyNumber] >= 0)
						{
						PolyArray[PolyCt].PolyQ = (float)((CurDEM->Vertices[VertNumber + CurDEM->pLatEntries + 1].ScrnXYZ[2] + CurDEM->Vertices[VertNumber + 1].ScrnXYZ[2] + CurDEM->Vertices[VertNumber + CurDEM->pLatEntries].ScrnXYZ[2]) * .33333 - ShadowMapDistanceOffset);
						PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_DEM;
						PolyArray[PolyCt ++].PolyNumber = PolyNumber;
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

				PolysPerRow = (CurDEM->pLatEntries - 1) * 2;
				for (Ct = 0; Ct < PolyCt; )	// Ct is incremented in busy-win update section
					{
					if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_DEM)
						{
						// extract the vertex numbers from the polygon number
						CurDEM->Lr = PolyArray[Ct].PolyNumber / PolysPerRow;
						CurDEM->Lc = (PolyArray[Ct].PolyNumber - (CurDEM->Lr * PolysPerRow)) / 2;
						VertNumber = CurDEM->Lc + CurDEM->pLatEntries * CurDEM->Lr;
						if (PolyArray[Ct].PolyNumber % 2)	// odd polygon in 0-based system counting from SW corner
							{
							if (! (Success = PrepPoly(VertNumber + CurDEM->pLatEntries + 1, VertNumber + 1, VertNumber + CurDEM->pLatEntries, PolyArray[Ct].PolyNumber)))
								break;
							} // if
						else
							{
							if (! (Success = PrepPoly(VertNumber, VertNumber + CurDEM->pLatEntries, VertNumber + 1, PolyArray[Ct].PolyNumber)))
								break;
							} // else
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
						if (BWDE->Update(++ Ct))
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
				} // if something to render
			AppMem_Free(PolyArray, NumPolys * sizeof (struct TerrainPolygonSort));
			} // if
		} // if Success
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

int Renderer::RenderStrayObjects(void)
{
int Success = 1;
unsigned long NumPolys, PolyCt, Ct;
struct TerrainPolygonSort *PolyArray;
BusyWin *BWDE = NULL;

if (ShadowMapInProgress)
	{
	return (1);
	} // if

// add the number of possible foliage and 3D object and fence vertices
if ((NumPolys = EffectsBase->Object3DBase.VerticesToRender) > 0)
	{
	// create the array of a size large enough to hold all the 3d objects
	#ifdef WCS_BUILD_DEMO
	if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort) + MemFuck, 0))
	#else // WCS_BUILD_DEMO
	if (PolyArray = (struct TerrainPolygonSort *)AppMem_Alloc(NumPolys * sizeof (struct TerrainPolygonSort), 0))
	#endif // WCS_BUILD_DEMO
		{
		PolyCt = 0;	// this will count the polygons as they are inserted into the array
		EffectsBase->Object3DBase.FillRenderPolyArray(&RendData, PolyArray, PolyCt, NULL);
		if (PolyCt > 0)
			{
			if (IsCamView)
				{
				BWDE = new BusyWin("Stray 3D Objects", PolyCt, 'BWDE', 0);
				} // if
			else
				{
				Master->ProcInit(PolyCt, "Stray 3D Objects");
				} // else
			// sort the polygons according to increasing Q
			if (PolyCt > 1)
				{
				qsort((void *)PolyArray, (size_t)PolyCt, (size_t)(sizeof (struct TerrainPolygonSort)), CompareTerrainPolygonSort);
				} // if enough polygons to bother sorting
			for (Ct = 0; Ct < PolyCt; )	// Ct is incremented in busy-win update section
				{
				if (PolyArray[Ct].PolyType == WCS_POLYSORTTYPE_3DOBJECT)
					{
					if (! (Success = Render3DObject((Object3DVertexList *)PolyArray[Ct].PolyNumber)))
						break;
					} // else if

				if (IsCamView)
					{
					if (BWDE->Update(++ Ct))
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
			if (Master)
				Master->ProcClear();
			if (BWDE)
				delete BWDE;
			} // if at least one poly to render
		AppMem_Free(PolyArray, NumPolys * sizeof (struct TerrainPolygonSort));
		} // if
	} // if

return (Success);

} // Renderer::RenderStrayObjects

/*===========================================================================*/

int Renderer::DisplaceDEMVertices(DEM *Map)
{
int Success = 1, HorFractDisplace, Displace;
unsigned long VtxCt, RowCt, ColCt, LatSeed, LonSeed;
double OffsetLat, OffsetLon, LatRange, LonRange;
BusyWin *BWDE = NULL;
#ifdef WCS_BUILD_VNS
unsigned long TestVtx;
int i, j;
#endif // WCS_BUILD_VNS

OffsetLat = Map->pLatStep * .5;  // Optimized out division. Was / 2.0
OffsetLon = Map->pLonStep * .5;  // Optimized out division. Was / 2.0

HorFractDisplace = EffectsBase->TerrainParamBase.HorFractDisplace;

if (! HorFractDisplace)
	return (1);	// nada to do here

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, Map->pLonEntries, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Map->pLonEntries, "Displacing Vertices");
	} // else

LatRange = Map->pNorthWest.Lat - Map->pSouthEast.Lat;
LonRange = Map->pSouthEast.Lon - Map->pNorthWest.Lon;

// gotta increment VtxCt for each row to account for the last vertex in each row which is skippd by the for () loop
for (VtxCt = RowCt = 0; RowCt < Map->pLonEntries - 1; RowCt ++, VtxCt ++)
	{
	for (ColCt = 0; ColCt < Map->pLatEntries - 1; ColCt ++, VtxCt ++)
		{
		if (HorFractDisplace && RowCt > 0 && ColCt > 0)
			{
			Displace = 1;
			#ifdef WCS_BUILD_VNS
			if (Map->NullReject)
				{
				// test to see if adjacent cells are renderable or NULL
				// only vertices surrounded by renderable points are displaced
				for (i = -1; i < 2; i ++)
					{
					for (j = -1; j < 2; j ++)
						{
						TestVtx = VtxCt + j + i * Map->pLatEntries;
						if (Map->TestNullValue(TestVtx))
							Displace = 0;
						} // for
					} // for
				} // if
			#endif // WCS_BUILD_VNS
			if (Displace)
				{
				LonSeed = (ULONG)(((Map->Vertices[VtxCt].xyz[0] - Map->pNorthWest.Lon) / LonRange) * ULONG_MAX);
				LatSeed = (ULONG)(((Map->Vertices[VtxCt].xyz[1] - Map->pSouthEast.Lat) / LatRange) * ULONG_MAX);
				RendRand.Seed64(LatSeed, LonSeed);

				Map->Vertices[VtxCt].xyz[1] += ((RendRand.GenPRN() - .5) * OffsetLat);
				Map->Vertices[VtxCt].xyz[0] += ((RendRand.GenPRN() - .5) * OffsetLon);
				} // if
			} // if
		} // for

	if (IsCamView)	// gotta have this distinction
		{
		if (BWDE->Update(RowCt + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(RowCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::DisplaceDEMVertices

/*===========================================================================*/

int Renderer::ApplyFractalDepthTexture(DEM *Map)
{
signed char LocalFrd;
int Success = 1, FractalDepthTextureExists;
unsigned long VtxCt, RowCt, ColCt, PolyNumber;
double TexOpacity, Value[3];
BusyWin *BWDE = NULL;
VertexData TempVtx;

FractalDepthTextureExists = CurDEM->FractalMap && DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH] && DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH]->Enabled;
Value[0] = Value[1] = Value[2] = 0.0;

if (! FractalDepthTextureExists)
	return (1);	// nada to do here

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, Map->pLonEntries, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Map->pLonEntries, "Fractal Depth Texture");
	} // else


// gotta increment VtxCt for each row to account for the last vertex in each row which is skippd by the for () loop
for (VtxCt = RowCt = 0; RowCt < Map->pLonEntries - 1; RowCt ++, VtxCt ++)
	{
	for (ColCt = 0; ColCt < Map->pLatEntries - 1; ColCt ++, VtxCt ++)
		{
		// if there  is a fractal depth texture, analyze it before displacing vertices.
		if (FractalDepthTextureExists)
			{
			PolyNumber = (RowCt * (CurDEM->pLatEntries - 1) + ColCt) * 2;
			if ((LocalFrd = CurDEM->FractalMap[PolyNumber]) > 0)
				{
				//TempVtx.TexDataInitialized = 0;
				TempVtx.Lat = Map->Vertices[VtxCt].Lat;
				TempVtx.Lon = Map->Vertices[VtxCt].Lon;
				if ((TexOpacity = DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH]->Eval(Value, RendData.TransferTextureData(&TempVtx))) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						// DefaultFrd is renderer scope
						LocalFrd = (signed char)(LocalFrd * (1.0 - TexOpacity + Value[0]) + .5);	// add .5 to round result
						} // if
					else
						LocalFrd = (signed char)(LocalFrd * Value[0] + .5);	// add .5 to round result
					CurDEM->FractalMap[PolyNumber] = LocalFrd;
					} // if
				} // if
			PolyNumber ++;
			if ((LocalFrd = CurDEM->FractalMap[PolyNumber]) > 0)
				{
				//TempVtx.TexDataInitialized = 0;
				TempVtx.Lat = Map->Vertices[VtxCt].Lat;
				TempVtx.Lon = Map->Vertices[VtxCt].Lon;
				if ((TexOpacity = DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH]->Eval(Value, RendData.TransferTextureData(&TempVtx))) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						// DefaultFrd is renderer scope
						LocalFrd = (signed char)(LocalFrd * (1.0 - TexOpacity + Value[0]) + .5);	// add .5 to round result
						} // if
					else
						LocalFrd = (signed char)(LocalFrd * Value[0] + .5);	// add .5 to round result
					CurDEM->FractalMap[PolyNumber] = LocalFrd;
					} // if
				} // if
			} // if
		} // for

	if (IsCamView)	// gotta have this distinction
		{
		if (BWDE->Update(RowCt + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(RowCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::ApplyFractalDepthTexture

/*===========================================================================*/

int Renderer::ScaleDEMElevs(DEM *Map)
{
int Success = 1;
unsigned long Ct, RowCt, RowZip;
long ColCt;	// unsigned long type doesn't work with OMP parallel for loop
BusyWin *BWDE = NULL;

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, Map->pLonEntries, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Map->pLonEntries, "Scaling Elevations");
	} // else

for (RowCt = RowZip = 0; RowCt < Map->pLonEntries; ++RowCt, RowZip += Map->pLatEntries)
	{
	#pragma omp for private(Ct)
	for (ColCt = 0; ColCt < (long)Map->pLatEntries; ++ColCt)
		{
		Ct = RowZip + ColCt;
		Map->Vertices[Ct].Elev = ElevDatum + ((Map->Vertices[Ct].Elev * ElScaleMult) - ElevDatum) * Exageration;
		} // for
	// end #pragma omp for

	if (IsCamView)	// gotta have this distinction
		{
		if (BWDE->Update(RowCt + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(RowCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::ScaleDEMElevs

/*===========================================================================*/

int Renderer::ProjectDEMVertices(DEM *Map)
{
int Success = 1;
unsigned long Ct, RowCt, RowZip;
long ColCt;	// unsigned long type doesn't work with OMP parallel for loop
VertexData TempVtx;
BusyWin *BWDE = NULL;

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, Map->pLonEntries, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Map->pLonEntries, "Projecting Vertices");
	} // else

for (RowCt = RowZip = 0; RowCt < Map->pLonEntries; ++RowCt, RowZip += Map->pLatEntries)
	{
	#pragma omp for private(Ct, TempVtx)
	for (ColCt = 0; ColCt < (long)Map->pLatEntries; ++ColCt)
		{
		#ifndef WCS_VECPOLY_EFFECTS
		double NonWaterScrnXYZ[4];
		#endif // WCS_VECPOLY_EFFECTS
		Ct = RowZip + ColCt;
		#ifndef WCS_VECPOLY_EFFECTS
		TempVtx.Flags = 0;
		//TempVtx.TexDataInitialized = 0;
		TempVtx.Eco = NULL;
		TempVtx.Lake = NULL;
		TempVtx.Stream = NULL;
		TempVtx.Beach = NULL;
		TempVtx.Vector = NULL;
		TempVtx.WaterElev = -10000000.0;
		TempVtx.BeachLevel = -10000000.0;
		// RelEl may be useful for texture evaluation
		TempVtx.RelEl = Map->RelElMap[Ct];
		#endif // WCS_VECPOLY_EFFECTS
		
		// copy current vertex position to temp
		TempVtx.CopyLatLon(&Map->Vertices[Ct]);

		#ifndef WCS_VECPOLY_EFFECTS
		// evaluate elevation modifiers
		// functions are not reentrant since they use data that are members of the evaluator base classes
		if (RasterTAsExist)
			{
			#pragma omp critical (OMP_CRITICAL_EFFECTEVAL)
			EffectsBase->EvalRasterTerraffectors(&RendData, &TempVtx, 2);	// 2 = both pre-fractals and post-fractals
			} // if
		if (TerraffectorsExist)
			{
			#pragma omp critical (OMP_CRITICAL_EFFECTEVAL)
			EffectsBase->EvalTerraffectors(&RendData, &TempVtx);
			} // if
		if (StreamsExist)
			{
			#pragma omp critical (OMP_CRITICAL_EFFECTEVAL)
			EffectsBase->EvalStreams(&RendData, &TempVtx);
			} // if
		if (LakesExist)
			{
			#pragma omp critical (OMP_CRITICAL_EFFECTEVAL)
			EffectsBase->EvalLakes(&RendData, &TempVtx);
			} // if
		#endif // WCS_VECPOLY_EFFECTS

		// project vertex, use non-water elevations
		Cam->ProjectVertexDEM(DefCoords, &TempVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian

		#ifndef WCS_VECPOLY_EFFECTS
		if (TempVtx.Elev < TempVtx.WaterElev)
			{
			NonWaterScrnXYZ[0] = TempVtx.ScrnXYZ[0];
			NonWaterScrnXYZ[1] = TempVtx.ScrnXYZ[1];
			NonWaterScrnXYZ[2] = TempVtx.ScrnXYZ[2];
			NonWaterScrnXYZ[3] = TempVtx.Q;
			TempVtx.Flags |= WCS_VERTEXDATA_FLAG_WATERVERTEX;
			TempVtx.Elev = TempVtx.WaterElev;
			// project vertex, elevations may have changed so need to reconvert to cartesian
			Cam->ProjectVertexDEM(DefCoords, &TempVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 = convert to cartesian
			// choose the set of projected coordinates that most closely fit on screen
			if (TempVtx.ScrnXYZ[0] < 0.0 && NonWaterScrnXYZ[0] >= 0.0)
				{
				TempVtx.ScrnXYZ[0] = NonWaterScrnXYZ[0];
				} // if
			else if (TempVtx.ScrnXYZ[0] >= 0.0 && NonWaterScrnXYZ[0] < 0.0)
				{
				// leave it as is
				} // else if
			else if (fabs(TempVtx.ScrnXYZ[0]) > fabs(NonWaterScrnXYZ[0]))
				{
				TempVtx.ScrnXYZ[0] = NonWaterScrnXYZ[0];
				} // else if
			if (TempVtx.ScrnXYZ[1] < 0.0 && NonWaterScrnXYZ[1] >= 0.0)
				{
				TempVtx.ScrnXYZ[1] = NonWaterScrnXYZ[1];
				} // if
			else if (TempVtx.ScrnXYZ[1] >= 0.0 && NonWaterScrnXYZ[1] < 0.0)
				{
				// leave it as is
				} // else if
			else if (fabs(TempVtx.ScrnXYZ[1]) > fabs(NonWaterScrnXYZ[1]))
				{
				TempVtx.ScrnXYZ[1] = NonWaterScrnXYZ[1];
				} // else if
			if (TempVtx.ScrnXYZ[2] < NonWaterScrnXYZ[2])
				TempVtx.ScrnXYZ[2] = NonWaterScrnXYZ[2];
			if (TempVtx.Q < NonWaterScrnXYZ[3])
				TempVtx.Q = NonWaterScrnXYZ[3];
			} // if
		#endif // WCS_VECPOLY_EFFECTS


		// copy projected coords, but not actual vertex coords, they will be needed in their original state later
		Map->Vertices[Ct].CopyScrnXYZQ(&TempVtx);	// these coords are just used for screen clipping
		Map->Vertices[Ct].CopyXYZ(&TempVtx);	// these coords are used for backface culling in frd map generation
		#ifndef WCS_VECPOLY_EFFECTS
		Map->Vertices[Ct].Flags |= (TempVtx.Flags & WCS_VERTEXDATA_FLAG_WATERVERTEX);
		#endif // WCS_VECPOLY_EFFECTS
		} // for
	// end #pragma omp for

	if (IsCamView)	// gotta have this distinction
		{
		if (BWDE->Update(RowCt + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(RowCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::ProjectDEMVertices

/*===========================================================================*/

// VertexData should be preconfigured with coordinates (lat, lon, elev) 
// of vertex and with displacement applied by fractals.

int Renderer::AllocVertices(void)
{
int Success = 1;

// MaxFractalDepth is Renderer scope, initialized during renderer startup.
// It is the maximum depth used by any enabled terrain parameter effects

// allocate 3 vertices for depth 0
VertexRoot[0] = new VertexData(NULL);
VertexRoot[1] = new VertexData(NULL);
VertexRoot[2] = new VertexData(NULL);

#ifndef WCS_VECPOLY_EFFECTS
// allocate three edges for level 0
EdgeRoot[0] = new PolygonEdge(NULL);
EdgeRoot[1] = new PolygonEdge(NULL);
EdgeRoot[2] = new PolygonEdge(NULL);

// assign vertices to end points
EdgeRoot[0]->End[0] = VertexRoot[0];
EdgeRoot[0]->End[1] = VertexRoot[1];
EdgeRoot[1]->End[0] = VertexRoot[1];
EdgeRoot[1]->End[1] = VertexRoot[2];
EdgeRoot[2]->End[0] = VertexRoot[2];
EdgeRoot[2]->End[1] = VertexRoot[0];

// for each additional level, subdivide edges with a midpt vertex
if (MaxFractalDepth > 0)
	Success = SubdividePolygon(EdgeRoot, 0);
#endif // WCS_VECPOLY_EFFECTS

return (Success);

} // Renderer::AllocVertices

/*===========================================================================*/

int Renderer::SubdividePolygon(PolygonEdge *Edge[3], int Frd)
{
char EdgeCt, NextEdge, EndUsed0, EndUsed1;
PolygonEdge **NewEdgePtr, *EdgeList[3], *CrossEdge[3];

for (EdgeCt = 0; EdgeCt < 3; EdgeCt ++)
	{
	if (! Edge[EdgeCt]->Mid)
		{
		if (! Edge[EdgeCt]->Subdivide())
			return (0);
		} // if
	} // for

for (EdgeCt = 0, NextEdge = 1; EdgeCt < 3; EdgeCt ++, NextEdge ++)
	{
	if (NextEdge > 2)
		NextEdge = 0;
	if (NewEdgePtr = (! Edge[EdgeCt]->StepChild[0]) ? &Edge[EdgeCt]->StepChild[0]: (! Edge[EdgeCt]->StepChild[1]) ? &Edge[EdgeCt]->StepChild[1]: NULL)
		{
		if (*NewEdgePtr = new PolygonEdge(NULL))
			{
			(*NewEdgePtr)->End[0] = Edge[EdgeCt]->Mid;
			(*NewEdgePtr)->End[1] = Edge[NextEdge]->Mid;
			(*NewEdgePtr)->Frd = Edge[EdgeCt]->Frd + 1;
			} // if
		else
			return (0);
		} // if
	} // for

Frd ++;

// subdivide all newly created polygons if necessary
if (Frd < MaxFractalDepth)
	{
	EndUsed0 = (Edge[0]->End[0] == Edge[1]->End[0] || Edge[0]->End[0] == Edge[1]->End[1]) ? 0: 1;
	EdgeList[0] = Edge[0]->Child[EndUsed0];
	EndUsed1 = (Edge[0]->End[EndUsed0] == Edge[1]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[1]->Child[EndUsed1];
	if (Edge[0]->StepChild[0] && (Edge[0]->StepChild[0]->End[0] == Edge[1]->Mid || Edge[0]->StepChild[0]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[0]->StepChild[0];
	else if (Edge[0]->StepChild[1] && (Edge[0]->StepChild[1]->End[0] == Edge[1]->Mid || Edge[0]->StepChild[1]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[0]->StepChild[1];
	else if (Edge[1]->StepChild[0] && (Edge[1]->StepChild[0]->End[0] == Edge[0]->Mid || Edge[1]->StepChild[0]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[1]->StepChild[0];
	else if (Edge[1]->StepChild[1] && (Edge[1]->StepChild[1]->End[0] == Edge[0]->Mid || Edge[1]->StepChild[1]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[1]->StepChild[1];
	CrossEdge[0] = EdgeList[2];
	if (! SubdividePolygon(EdgeList, Frd))
		return (0);

	EndUsed0 = 1 - EndUsed1;
	EdgeList[0] = Edge[1]->Child[EndUsed0];
	EndUsed1 = (Edge[1]->End[EndUsed0] == Edge[2]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[2]->Child[EndUsed1];
	if (Edge[1]->StepChild[0] && (Edge[1]->StepChild[0]->End[0] == Edge[2]->Mid || Edge[1]->StepChild[0]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[1]->StepChild[0];
	else if (Edge[1]->StepChild[1] && (Edge[1]->StepChild[1]->End[0] == Edge[2]->Mid || Edge[1]->StepChild[1]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[1]->StepChild[1];
	else if (Edge[2]->StepChild[0] && (Edge[2]->StepChild[0]->End[0] == Edge[1]->Mid || Edge[2]->StepChild[0]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[2]->StepChild[0];
	else if (Edge[2]->StepChild[1] && (Edge[2]->StepChild[1]->End[0] == Edge[1]->Mid || Edge[2]->StepChild[1]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[2]->StepChild[1];
	CrossEdge[1] = EdgeList[2];
	if (! SubdividePolygon(EdgeList, Frd))
		return (0);

	EndUsed0 = 1 - EndUsed1;
	EdgeList[0] = Edge[2]->Child[EndUsed0];
	EndUsed1 = (Edge[2]->End[EndUsed0] == Edge[0]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[0]->Child[EndUsed1];
	if (Edge[2]->StepChild[0] && (Edge[2]->StepChild[0]->End[0] == Edge[0]->Mid || Edge[2]->StepChild[0]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[2]->StepChild[0];
	else if (Edge[2]->StepChild[1] && (Edge[2]->StepChild[1]->End[0] == Edge[0]->Mid || Edge[2]->StepChild[1]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[2]->StepChild[1];
	else if (Edge[0]->StepChild[0] && (Edge[0]->StepChild[0]->End[0] == Edge[2]->Mid || Edge[0]->StepChild[0]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[0]->StepChild[0];
	else if (Edge[0]->StepChild[1] && (Edge[0]->StepChild[1]->End[0] == Edge[2]->Mid || Edge[0]->StepChild[1]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[0]->StepChild[1];
	CrossEdge[2] = EdgeList[2];
	if (! SubdividePolygon(EdgeList, Frd))
		return (0);

	if (! SubdividePolygon(CrossEdge, Frd))
		return (0);
	} // if

return (1);

} // Renderer::SubdividePolygon

/*===========================================================================*/
/*===========================================================================*/

int Renderer::PrepPoly(unsigned long V0, unsigned long V1, unsigned long V2, unsigned long PolyNumber)
{
char Ct, PolyFrd, DepthMapFrd, PolyType, ForceRender = 0;
unsigned long PolysPerRow;

// determine fractal depth for this polygon
PolyFrd = DefaultFrd;		// DefaultFrd is Renderer scope based on default terrain param and DEM's max fractal depth

// depth maps are the least expensive way to cull polygons so do it first
if (CurDEM->FractalMap)
	{
	if ((DepthMapFrd = CurDEM->FractalMap[PolyNumber]) < 0)
		return (1);
	if (DepthMapFrd >= 10)
		{
		DepthMapFrd -= 10;
		ForceRender = 1;
		} // if
	if (DepthMapFrd < PolyFrd)
		PolyFrd = DepthMapFrd;
	} // if

// copy the vertex coords to our root VertexData array
VertexRoot[0]->CopyLatLon(&CurDEM->Vertices[V0]);
VertexRoot[1]->CopyLatLon(&CurDEM->Vertices[V1]);
VertexRoot[2]->CopyLatLon(&CurDEM->Vertices[V2]);
VertexRoot[0]->CopyScrnXYZQ(&CurDEM->Vertices[V0]);
VertexRoot[1]->CopyScrnXYZQ(&CurDEM->Vertices[V1]);
VertexRoot[2]->CopyScrnXYZQ(&CurDEM->Vertices[V2]);

// bounds testing is the second-least expensive culling method
if (! ForceRender && ! TestInBounds((VertexBase **)VertexRoot))
	return (1);

// initialize flags
VertexRoot[0]->Flags = VertexRoot[1]->Flags = VertexRoot[2]->Flags = 0;
//VertexRoot[0]->TexDataInitialized = VertexRoot[1]->TexDataInitialized = VertexRoot[2]->TexDataInitialized = 0;
// RelEl may be useful for texture evaluation
VertexRoot[0]->RelEl = CurDEM->RelElMap[V0];
VertexRoot[1]->RelEl = CurDEM->RelElMap[V1];
VertexRoot[2]->RelEl = CurDEM->RelElMap[V2];

// terrain parameter effects can raise or lower the fractal depth but do not cause culling
// <<<>>>gh analyze terrain param effects to see if fractal level needs adjusting

// analyze pre-fractal area terraffectors - can modify vertex elevation
if (RasterTAsExist)
	{
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (! (VertexRoot[Ct]->Flags & WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED))
			{
			EffectsBase->EvalRasterTerraffectors(&RendData, VertexRoot[Ct], 0);		// 0 = pre-fractals
			VertexRoot[Ct]->Flags |= WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED;
			} // if
		} // if
	} // if

// plant maximum fractal depth of surrounding polygons in edges
if (CurDEM->FractalMap)
	{
	PolysPerRow = (CurDEM->pLatEntries - 1) * 2;
	if (PolyType = (char)(PolyNumber % 2))
		{
		EdgeRoot[0]->Frd = (CurDEM->Lc < (long)CurDEM->pLatEntries - 2) ? ((CurDEM->FractalMap[PolyNumber + 1] > 0) ? CurDEM->FractalMap[PolyNumber + 1]: 0): 0;
		EdgeRoot[1]->Frd = (CurDEM->FractalMap[PolyNumber - 1] > 0) ? CurDEM->FractalMap[PolyNumber - 1]: 0;
		EdgeRoot[2]->Frd = (CurDEM->Lr < (long)CurDEM->pLonEntries - 2) ? ((CurDEM->FractalMap[PolyNumber - 1 + PolysPerRow] > 0) ? CurDEM->FractalMap[PolyNumber - 1 + PolysPerRow]: 0): 0;
		} // if
	else
		{
		EdgeRoot[0]->Frd = (CurDEM->Lc > 0) ? ((CurDEM->FractalMap[PolyNumber - 1] > 0) ? CurDEM->FractalMap[PolyNumber - 1]: 0): 0;
		EdgeRoot[1]->Frd = (CurDEM->FractalMap[PolyNumber + 1] > 0) ? CurDEM->FractalMap[PolyNumber + 1]: 0;
		EdgeRoot[2]->Frd = (CurDEM->Lr > 0) ? ((CurDEM->FractalMap[PolyNumber + 1 - PolysPerRow] > 0) ? CurDEM->FractalMap[PolyNumber + 1 - PolysPerRow]: 0): 0;
		} // else
	} // if
else
	{
	if (PolyType = (char)(PolyNumber % 2))
		{
		EdgeRoot[0]->Frd = (CurDEM->Lc < (long)CurDEM->pLatEntries - 2) ? PolyFrd: 0;
		EdgeRoot[1]->Frd = PolyFrd;
		EdgeRoot[2]->Frd = (CurDEM->Lr < (long)CurDEM->pLonEntries - 2) ? PolyFrd: 0;
		} // if
	else
		{
		EdgeRoot[0]->Frd = (CurDEM->Lc > 0) ? PolyFrd: 0;
		EdgeRoot[1]->Frd = PolyFrd;
		EdgeRoot[2]->Frd = (CurDEM->Lr > 0) ? PolyFrd: 0;
		} // else
	} // else

// setting these edge values to 0 is what triggers the displacement
EdgeRoot[0]->Displaced = EdgeRoot[1]->Displaced = EdgeRoot[2]->Displaced = 0;

// other initialization stuff
// Lat, Lon and Elev are already initialized when the vertex is copied from the VertexDEM above
// <<<>>>gh fill in relative elevation and other things that need to be interpolated
VertexRoot[0]->Eco = VertexRoot[1]->Eco = VertexRoot[2]->Eco = NULL;
VertexRoot[0]->Lake = VertexRoot[1]->Lake = VertexRoot[2]->Lake = NULL;
VertexRoot[0]->Stream = VertexRoot[1]->Stream = VertexRoot[2]->Stream = NULL;
VertexRoot[0]->Beach = VertexRoot[1]->Beach = VertexRoot[2]->Beach = NULL;
VertexRoot[0]->Vector = VertexRoot[1]->Vector = VertexRoot[2]->Vector = NULL;
VertexRoot[0]->oElev = VertexRoot[0]->Elev;
VertexRoot[1]->oElev = VertexRoot[1]->Elev;
VertexRoot[2]->oElev = VertexRoot[2]->Elev;
VertexRoot[0]->WaveHeight = VertexRoot[1]->WaveHeight = VertexRoot[2]->WaveHeight = 0.0;

// no longer necessary
// check for 3d objects and foliage effects
// check for foliage within grid cell
/*
FoliageRastNum = 0;
if (EffectsBase->AreThereFoliages())
	{
	FoliageVertexPtr = NULL;
	EffectsBase->EvalFoliageFourPoints((VertexDEM **)VertexRoot, FoliageRastNum, FoliageVertexPtr);
	} // if

// check for 3D objects within grid cell
Object3DRastNum = 0;
if (! ShadowMapInProgress && EffectsBase->AreThereObject3Ds())
	{
	Object3DVertexPtr = NULL;
	EffectsBase->EvalObject3DFourPoints((VertexDEM **)VertexRoot, Object3DRastNum, Object3DVertexPtr);
	} // if
*/
// fractalize polygon and render 
if (PolyFrd > 0)
	{
	return (FractalizePolygon(EdgeRoot, PolyFrd, 0, CurDEM));
	} // if
else
	{
	VertexRoot[0]->Map = VertexRoot[1]->Map = VertexRoot[2]->Map = CurDEM;
	// check for 3d objects and foliage effects
	// <<<>>>gh
	#ifndef WCS_VECPOLY_EFFECTS
	return (TerrainPolygonSetup(VertexRoot, 0));
	#else // WCS_VECPOLY_EFFECTS
	return (0);
	#endif // WCS_VECPOLY_EFFECTS
	} // else

} //  Renderer::PrepPoly

/*===========================================================================*/

int Renderer::TestInBounds(VertexBase *Vert[3])
{
int a, b, c, d, j, z, InFrontPts;

for (j = z = InFrontPts = 0; j < 3; j ++)
	{
	if (Vert[j]->ScrnXYZ[2] < 0.0001)
		{
		z ++;
		} // if behind viewer
	else
		InFrontPts ++;
	} // for
if (z < 3)
	{
	for (j = d = c = b = a = 0; j < 3; j ++)
		{
		if (Vert[j]->ScrnXYZ[0] < OSLowX)
			++a;
		else if (Vert[j]->ScrnXYZ[0] > OSHighX)
			++b;
		if (Vert[j]->ScrnXYZ[1] < OSLowY)
			++c;
		else if (Vert[j]->ScrnXYZ[1] > OSHighY)
			++d;
		} // for
	} // if not all corners are behind viewer 

// this caused some corner foreground polygons to be clipped inappropriately
//return (z < 3 && a < InFrontPts && b < InFrontPts && c < InFrontPts && d < InFrontPts);
return (z < 3 && a < 3 && b < 3 && c < 3 && d < 3);

} // Renderer::TestInBounds

/*===========================================================================*/

int Renderer::FractalizePolygon(PolygonEdge *Edge[3], unsigned char MaxFrd, unsigned char CurFrd, DEM *Map)
{
double ElDif, SlopeFactor, Displacement, MaxDif;
VertexData *Vtx[3];
PolygonEdge *EdgeList[3], *CrossEdge[3];
char Ct, EndUsed0, EndUsed1;

if (CurFrd < MaxFrd)
	{
	// walk the polygon edges and modify elevations of midpoints if Edge Frd is >= CurFrd
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (! Edge[Ct]->Displaced)
			{
			// propagate info down to children
			Edge[Ct]->Displaced = 1;
			if (Edge[Ct]->Child[0])
				{
				Edge[Ct]->Child[0]->Displaced = 0;
				Edge[Ct]->Child[0]->Frd = Edge[Ct]->Frd;
				} // if
			if (Edge[Ct]->Child[1])
				{
				Edge[Ct]->Child[1]->Displaced = 0;
				Edge[Ct]->Child[1]->Frd = Edge[Ct]->Frd;
				} // if
			if (Edge[Ct]->StepChild[0])
				{
				Edge[Ct]->StepChild[0]->Displaced = 0;
				Edge[Ct]->StepChild[0]->Frd = MaxFrd;
				} // if
			if (Edge[Ct]->StepChild[1])
				{
				Edge[Ct]->StepChild[1]->Displaced = 0;
				Edge[Ct]->StepChild[1]->Frd = MaxFrd;
				} // if

			// interpolate midpoint data using stored elevations which do not include post-fractal terraffectors
			Edge[Ct]->Mid->Elev = Edge[Ct]->Mid->oElev = (Edge[Ct]->End[0]->oElev + Edge[Ct]->End[1]->oElev) * 0.5;  // Optimized out division. Was / 2.0
			Edge[Ct]->Mid->Lat = (Edge[Ct]->End[0]->Lat + Edge[Ct]->End[1]->Lat) * 0.5; // Optimized out division. Was / 2.0
			Edge[Ct]->Mid->Lon = (Edge[Ct]->End[0]->Lon + Edge[Ct]->End[1]->Lon) * 0.5; // Optimized out division. Was / 2.0
			Edge[Ct]->Mid->RelEl = (Edge[Ct]->End[0]->RelEl + Edge[Ct]->End[1]->RelEl) * (float)0.5; // Optimized out division. Was / 2.0
			Edge[Ct]->Mid->Flags = 0;
			Edge[Ct]->Mid->Eco = NULL;
			Edge[Ct]->Mid->Lake = NULL;
			Edge[Ct]->Mid->Stream = NULL;
			Edge[Ct]->Mid->Beach = NULL;
			Edge[Ct]->Mid->Vector = NULL;
			Edge[Ct]->Mid->WaveHeight = 0.0;
			Edge[Ct]->Mid->Displacement = 0.0;
			Edge[Ct]->Mid->oDisplacement = 0.0;

			// Create a standard set of seed values to be used whenever referring to this vertex.
			// Different processes may seed the random # generator with these values + some process specific offset
			Edge[Ct]->Mid->LonSeed = (ULONG)((Edge[Ct]->Mid->Lon - WCS_floor(Edge[Ct]->Mid->Lon)) * ULONG_MAX);
			Edge[Ct]->Mid->LatSeed = (ULONG)((Edge[Ct]->Mid->Lat - WCS_floor(Edge[Ct]->Mid->Lat)) * ULONG_MAX);

			if (CurFrd < Edge[Ct]->Frd)
				{
				// evaluate TerrainParam effect for slope factor and displacement at midpt vertex
				SlopeFactor = DefaultTerrainPar->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR].CurValue;	// temporary
				Displacement = DefaultTerrainPar->AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].CurValue;	// temporary

				if (DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT] && DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT]->Enabled)
					{
					double TexOpacity, Value[3];

					Value[0] = Value[1] = Value[2] = 0.0;
					if ((TexOpacity = DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT]->Eval(Value, RendData.TransferTextureData(Edge[Ct]->Mid))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							Displacement *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							Displacement *= Value[0];
						} // if
					} // if
				if (DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR] && DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR]->Enabled)
					{
					double TexOpacity, Value[3];

					Value[0] = Value[1] = Value[2] = 0.0;
					if ((TexOpacity = DefaultTerrainPar->TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR]->Eval(Value, RendData.TransferTextureData(Edge[Ct]->Mid))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							SlopeFactor = 1.0 + (SlopeFactor - 1.0) * (1.0 - TexOpacity + Value[0]);
							} // if
						else
							SlopeFactor = 1.0 + (SlopeFactor - 1.0) * Value[0];
						} // if
					} // if

				// displace midpoint of edge based on displacement, elevation difference of endpoints, 
				// current frd and slope dsplacement factor
				ElDif = fabs(Edge[Ct]->End[0]->oElev - Edge[Ct]->End[1]->oElev) * INV_DISPLACE_FRACT_SLOPEFACT;
				MaxDif = 4.0 * ElDif;
				ElDif = pow(ElDif, SlopeFactor);
				if (ElDif > MaxDif)
					ElDif = MaxDif;

				RendRand.Seed64BitShift(Edge[Ct]->Mid->LatSeed, Edge[Ct]->Mid->LonSeed);

				// store total displacement in vertex for later use by effects
				Edge[Ct]->Mid->Displacement = (RendRand.GenGauss() * DEMCellSize * Displacement * PerturbTable[CurFrd]);
				if (ElDif != 0.0)
					Edge[Ct]->Mid->Displacement += (RendRand.GenGauss() * ElDif);
				Edge[Ct]->Mid->Elev += Edge[Ct]->Mid->Displacement;
				Edge[Ct]->Mid->Displacement += (Edge[Ct]->End[0]->oDisplacement + Edge[Ct]->End[1]->oDisplacement) * 0.5;  // Optimized out division. Was / 2.0
				Edge[Ct]->Mid->oElev = Edge[Ct]->Mid->Elev;
				Edge[Ct]->Mid->oDisplacement = Edge[Ct]->Mid->Displacement;
				} // if
			} // if not already displaced
		} // for

	// this wicked looking bunch of stuff processes subpolygons in such a way as to preserve
	// the clockwise or counter clockwise order of the outside edges
	EndUsed0 = (Edge[0]->End[0] == Edge[1]->End[0] || Edge[0]->End[0] == Edge[1]->End[1]) ? 0: 1;
	EdgeList[0] = Edge[0]->Child[EndUsed0];
	EndUsed1 = (Edge[0]->End[EndUsed0] == Edge[1]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[1]->Child[EndUsed1];
	if (Edge[0]->StepChild[0] && (Edge[0]->StepChild[0]->End[0] == Edge[1]->Mid || Edge[0]->StepChild[0]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[0]->StepChild[0];
	else if (Edge[0]->StepChild[1] && (Edge[0]->StepChild[1]->End[0] == Edge[1]->Mid || Edge[0]->StepChild[1]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[0]->StepChild[1];
	else if (Edge[1]->StepChild[0] && (Edge[1]->StepChild[0]->End[0] == Edge[0]->Mid || Edge[1]->StepChild[0]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[1]->StepChild[0];
	else if (Edge[1]->StepChild[1] && (Edge[1]->StepChild[1]->End[0] == Edge[0]->Mid || Edge[1]->StepChild[1]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[1]->StepChild[1];
	CrossEdge[0] = EdgeList[2];
	if (! FractalizePolygon(EdgeList, MaxFrd, CurFrd + 1, Map))
		return (0);

	EndUsed0 = 1 - EndUsed1;
	EdgeList[0] = Edge[1]->Child[EndUsed0];
	EndUsed1 = (Edge[1]->End[EndUsed0] == Edge[2]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[2]->Child[EndUsed1];
	if (Edge[1]->StepChild[0] && (Edge[1]->StepChild[0]->End[0] == Edge[2]->Mid || Edge[1]->StepChild[0]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[1]->StepChild[0];
	else if (Edge[1]->StepChild[1] && (Edge[1]->StepChild[1]->End[0] == Edge[2]->Mid || Edge[1]->StepChild[1]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[1]->StepChild[1];
	else if (Edge[2]->StepChild[0] && (Edge[2]->StepChild[0]->End[0] == Edge[1]->Mid || Edge[2]->StepChild[0]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[2]->StepChild[0];
	else if (Edge[2]->StepChild[1] && (Edge[2]->StepChild[1]->End[0] == Edge[1]->Mid || Edge[2]->StepChild[1]->End[1] == Edge[1]->Mid))
		EdgeList[2] = Edge[2]->StepChild[1];
	CrossEdge[1] = EdgeList[2];
	if (! FractalizePolygon(EdgeList, MaxFrd, CurFrd + 1, Map))
		return (0);

	EndUsed0 = 1 - EndUsed1;
	EdgeList[0] = Edge[2]->Child[EndUsed0];
	EndUsed1 = (Edge[2]->End[EndUsed0] == Edge[0]->End[0]) ? 0: 1;
	EdgeList[1] = Edge[0]->Child[EndUsed1];
	if (Edge[2]->StepChild[0] && (Edge[2]->StepChild[0]->End[0] == Edge[0]->Mid || Edge[2]->StepChild[0]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[2]->StepChild[0];
	else if (Edge[2]->StepChild[1] && (Edge[2]->StepChild[1]->End[0] == Edge[0]->Mid || Edge[2]->StepChild[1]->End[1] == Edge[0]->Mid))
		EdgeList[2] = Edge[2]->StepChild[1];
	else if (Edge[0]->StepChild[0] && (Edge[0]->StepChild[0]->End[0] == Edge[2]->Mid || Edge[0]->StepChild[0]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[0]->StepChild[0];
	else if (Edge[0]->StepChild[1] && (Edge[0]->StepChild[1]->End[0] == Edge[2]->Mid || Edge[0]->StepChild[1]->End[1] == Edge[2]->Mid))
		EdgeList[2] = Edge[0]->StepChild[1];
	CrossEdge[2] = EdgeList[2];
	if (! FractalizePolygon(EdgeList, MaxFrd, CurFrd + 1, Map))
		return (0);

	return (FractalizePolygon(CrossEdge, MaxFrd, CurFrd + 1, Map));
	} // if
else
	{
	// sort out the edge vertices into an array of vertex pointers
	// vertices will be in same order (clockwise or counter) that the edges are in
	Vtx[0] = (Edge[0]->End[1] == Edge[1]->End[0] || Edge[0]->End[1] == Edge[1]->End[1]) ? 
		Edge[0]->End[1]: Edge[0]->End[0];
	Vtx[1] = (Edge[1]->End[0] == Vtx[0]) ? Edge[1]->End[1]: Edge[1]->End[0];
	Vtx[2] = (Edge[2]->End[0] == Vtx[1]) ? Edge[2]->End[1]: Edge[2]->End[0];
	Vtx[0]->Map = Vtx[1]->Map = Vtx[2]->Map = Map;

	#ifndef WCS_VECPOLY_EFFECTS
	return (TerrainPolygonSetup(Vtx, CurFrd));
	#else // WCS_VECPOLY_EFFECTS
	return (0);
	#endif // WCS_VECPOLY_EFFECTS
	} // else

} // Renderer::FractalizePolygon

/*===========================================================================*/

void Renderer::SetBoundsPackage(BoundsPackage *Conditions)
{

Conditions->OSLowX = OSLowX;
Conditions->OSHighX = OSHighX;
Conditions->OSLowY = OSLowY;
Conditions->OSHighY = OSHighY;
Conditions->QMax = QMax;
Conditions->ZMax = MaximumZ;
Conditions->Exageration = Exageration;
Conditions->ElevDatum = ElevDatum;
Conditions->PlanetRad = PlanetRad;
Conditions->EarthLatScaleMeters = EarthLatScaleMeters;
Conditions->DefCoords = DefCoords;

} // Renderer::SetBoundsPackage

/*===========================================================================*/

bool Renderer::DEMCueGetBounds(double &LowLat, double &LowLon, double &HighLat, double &HighLon)
{

return (DEMCue ? DEMCue->GetBounds(LowLat, LowLon, HighLat, HighLon): false);

} // Renderer::DEMCueGetBounds

/*===========================================================================*/
/*===========================================================================*/

PolygonEdge::PolygonEdge(PolygonEdge *NewParent)
{

End[0] = End[1] = Mid = NULL;
Child[0] = Child[1] = NULL;
StepChild[0] = StepChild[1] = NULL;
Parent = NewParent;
Frd = Displaced = 0;

} // PolygonEdge::PolygonEdge

/*===========================================================================*/

PolygonEdge::~PolygonEdge()
{

if (Mid)
	delete Mid;
if (StepChild[0])
	delete StepChild[0];
if (StepChild[1])
	delete StepChild[1];
if (Child[0])
	delete Child[0];
if (Child[1])
	delete Child[1];

} // PolygonEdge::~PolygonEdge

/*===========================================================================*/

int PolygonEdge::Subdivide(void)
{

if (Mid = new VertexData(this))
	{
	Mid->Frd = Frd + 1;
	if (Child[0] = new PolygonEdge(this))
		{
		Child[0]->Frd = Frd + 1;
		Child[0]->End[0] = End[0];
		Child[0]->End[1] = Mid;
		if (Child[1] = new PolygonEdge(this))
			{
			Child[1]->Frd = Frd + 1;
			Child[1]->End[0] = Mid;
			Child[1]->End[1] = End[1];
			} // if
		else
			return (0);
		} // if
	else
		return (0);
	} // if
else
	return (0);

return (1);

} // PolygonEdge::Subdivide

/*===========================================================================*/
/*===========================================================================*/

RenderQItem::RenderQItem()
{

QJoe = NULL;
QJoeDEM = NULL;
QDEM = NULL;
QDist = 0.0f;
MaxFrd = RenderMe = 0;
#ifdef WCS_VECPOLY_EFFECTS
DEMPolyList = NULL;
DEMVectorPoly = NULL;
PolygonListformed = false;
#endif // WCS_VECPOLY_EFFECTS

} // RenderQItem::RenderQItem

/*===========================================================================*/

RenderQItem::~RenderQItem()
{

#ifdef WCS_VECPOLY_EFFECTS
// delete the DEM if it still exists - in VNS 2 it was deleted in RenderTerrain()
if (QDEM)
	delete QDEM;

// delete any polygons created
for (VectorPolygonListDouble *Temp = DEMPolyList; Temp; Temp = DEMPolyList)
	{
	DEMPolyList = (VectorPolygonListDouble *)DEMPolyList->NextPolygonList;
	delete Temp;
	} // for
if (DEMVectorPoly)
	delete DEMVectorPoly;
#endif // WCS_VECPOLY_EFFECTS

} // RenderQItem::~RenderQItem

/*===========================================================================*/
/*===========================================================================*/

RenderQ::RenderQ()
{

TheQ = NULL;
NumItems = ActiveItems = 0;

} // RenderQ::RenderQ

/*===========================================================================*/

RenderQ::~RenderQ()
{

if (TheQ)
	delete [] TheQ;
TheQ = NULL;

} // RenderQ::~RenderQ

/*===========================================================================*/

// FillList can be called as often as desired, 
// basically whenever it is likely that the number of elements has changed

int RenderQ::FillList(Database *DBase, int Draw0Render1)
{
Joe *CurJoe;
long Ct;

if (TheQ)
	delete [] TheQ;
TheQ = NULL;

DBase->ResetGeoClip();
NumItems = ActiveItems = 0;		// RenderQ scope

AppMem_ClearDEMSizes();

// count all pertinent DEMs
for (CurJoe = DBase->GetFirst(); CurJoe; CurJoe = DBase->GetNext(CurJoe))
	{
	if (CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ((Draw0Render1 && CurJoe->TestRenderFlags()) || (! Draw0Render1 && CurJoe->TestDrawFlags()))
		&& CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		NumItems ++;
	} // for

// allocate that number of items
if (NumItems > 0)
	{
	if (TheQ = new RenderQItem[NumItems])
		{
		// fill in all pertinent DEMs
		Ct = 0;
		for (CurJoe = DBase->GetFirst(); CurJoe; CurJoe = DBase->GetNext(CurJoe))
			{
			if (CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ((Draw0Render1 && CurJoe->TestRenderFlags()) || (! Draw0Render1 && CurJoe->TestDrawFlags()))
				&& (TheQ[Ct].QJoeDEM = (JoeDEM *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM)))
				{
				TheQ[Ct].QJoe = CurJoe;
				TheQ[Ct].MaxFrd = TheQ[Ct].QJoeDEM->MaxFract;
				Ct ++;
				} // if
			} // for
		return (1);
		} // if
	return (0);
	} // if
else
	return (1);

} // RenderQ::FillList

/*===========================================================================*/

// SortList should be called whenever it is likely that the order of rendering has changed.
// No need to call FillList first unless the number of enabled DEMs might have changed
// Camera passed must have been initialized and be capable of transforming object coords.

int RenderQ::SortList(Camera *Cam, int BoundsDiscrim, BoundsPackage *Conditions)
{
long Ct, a, b, c, d, j, q, Done;
Joe *CurJoe;
JoeDEM *CurAttrib;
DEMBoundingBox Contain;

ActiveItems = 0;

// compute Q for all items, set RenderMe flag for all visible items
for (Ct = 0; Ct < NumItems; Ct ++)
	{
	CurJoe = TheQ[Ct].QJoe;
	CurAttrib = TheQ[Ct].QJoeDEM;
	// Latitude of corner points.
	Contain.Vtx[0].Lat  /* SE */ = CurJoe->SELat;
	Contain.Vtx[1].Lat  /* SW */ = CurJoe->SELat; // SW Lat = SE Lat
	Contain.Vtx[2].Lat  /* NW */ = CurJoe->NWLat;
	Contain.Vtx[3].Lat  /* NE */ = CurJoe->NWLat; // NE Lat = NW Lat
	Contain.Vtx[4].Lat  /* SE */ = CurJoe->SELat;
	Contain.Vtx[5].Lat  /* SW */ = CurJoe->SELat; // SW Lat = SE Lat
	Contain.Vtx[6].Lat  /* NW */ = CurJoe->NWLat;
	Contain.Vtx[7].Lat  /* NE */ = CurJoe->NWLat; // NE Lat = NW Lat
	Contain.Vtx[8].Lat  /* SE */ = CurJoe->SELat;
	Contain.Vtx[9].Lat  /* SW */ = CurJoe->SELat; // SW Lat = SE Lat
	Contain.Vtx[10].Lat /* NW */ = CurJoe->NWLat;
	Contain.Vtx[11].Lat /* NE */ = CurJoe->NWLat; // NE Lat = NW Lat
	Contain.Vtx[12].Lat /* Mid */= (Contain.Vtx[0].Lat + Contain.Vtx[2].Lat) * 0.5;  // Optimized out division. Was / 2.0

	// Longitude of corner points
	Contain.Vtx[0].Lon  /* SE */ = CurJoe->SELon;
	Contain.Vtx[1].Lon  /* SW */ = CurJoe->NWLon; // NW Lon = SW Lon
	Contain.Vtx[2].Lon  /* NW */ = CurJoe->NWLon;
	Contain.Vtx[3].Lon  /* NE */ = CurJoe->SELon; // NW Lon = SE Lon
	Contain.Vtx[4].Lon  /* SE */ = CurJoe->SELon;
	Contain.Vtx[5].Lon  /* SW */ = CurJoe->NWLon; // NW Lon = SW Lon
	Contain.Vtx[6].Lon  /* NW */ = CurJoe->NWLon;
	Contain.Vtx[7].Lon  /* NE */ = CurJoe->SELon; // NW Lon = SE Lon
	Contain.Vtx[8].Lon  /* SE */ = CurJoe->SELon;
	Contain.Vtx[9].Lon  /* SW */ = CurJoe->NWLon; // NW Lon = SW Lon
	Contain.Vtx[10].Lon /* NW */ = CurJoe->NWLon;
	Contain.Vtx[11].Lon /* NE */ = CurJoe->SELon; // NW Lon = SE Lon
	Contain.Vtx[12].Lon /* Mid */= (Contain.Vtx[0].Lon + Contain.Vtx[1].Lon) * 0.5;  // Optimized out division. Was / 2.0

	// Elevations...
	// Corners 0-3 are set at MinEl
	Contain.Vtx[0].Elev = Contain.Vtx[1].Elev = Contain.Vtx[2].Elev = Contain.Vtx[3].Elev =	CurAttrib->MinEl;

	// 4-7 are set at MaxEl
	Contain.Vtx[4].Elev = Contain.Vtx[5].Elev = Contain.Vtx[6].Elev = Contain.Vtx[7].Elev = CurAttrib->MaxEl;

	// 8-11 are real elevation of corners.
	Contain.Vtx[8].Elev  /* SE */ = CurAttrib->SEAlt;
	Contain.Vtx[9].Elev  /* SW */ = CurAttrib->SWAlt;
	Contain.Vtx[10].Elev /* NW */ = CurAttrib->NWAlt;
	Contain.Vtx[11].Elev /* NE */ = CurAttrib->NEAlt;
	Contain.Vtx[12].Elev /* Mid */= (short)((CurAttrib->SEAlt + CurAttrib->SWAlt + CurAttrib->NWAlt +
									CurAttrib->NEAlt) * 0.25); // Optimized out division. Was / 4.0

	if (BoundsDiscrim)
		{
		Contain.Vtx[0].Elev -= 100;
		Contain.Vtx[1].Elev -= 100;
		Contain.Vtx[2].Elev -= 100;
		Contain.Vtx[3].Elev -= 100;
		/* no such variable in v5
		if (Contain.Vtx[4].Elev < (short)SeaLevel)
			Contain.Vtx[4].Elev = (short)SeaLevel;
		if (Contain.Vtx[5].Elev < (short)SeaLevel)
			Contain.Vtx[5].Elev = (short)SeaLevel;
		if (Contain.Vtx[6].Elev < (short)SeaLevel)
			Contain.Vtx[6].Elev = (short)SeaLevel;
		if (Contain.Vtx[7].Elev < (short)SeaLevel)
			Contain.Vtx[7].Elev = (short)SeaLevel;
		*/
		Contain.Vtx[4].Elev += 100;
		Contain.Vtx[5].Elev += 100;
		Contain.Vtx[6].Elev += 100;
		Contain.Vtx[7].Elev += 100;

		Contain.TransformToScreen(Cam, Conditions);

		TheQ[Ct].QDist = (float)((Contain.Vtx[8].Q + Contain.Vtx[9].Q + Contain.Vtx[10].Q + Contain.Vtx[11].Q + Contain.Vtx[12].Q) * 0.2);  // Optimized out division. Was / 5.0

		// first see if the camera is inside the DEM cube
		if (Cam->CamPos->Elev >= Contain.Vtx[0].Elev && Cam->CamPos->Elev <= Contain.Vtx[4].Elev &&
			Cam->CamPos->Lat >= Contain.Vtx[0].Lat && Cam->CamPos->Lat <= Contain.Vtx[2].Lat &&
			Cam->CamPos->Lon >= Contain.Vtx[0].Lon && Cam->CamPos->Lon <= Contain.Vtx[1].Lon)
			{
			// inside cube
			TheQ[Ct].RenderMe = 1; // Turn me on, she says
			ActiveItems ++;
			} // if
		else
			{
			for (j = q = 0; j < 13; j ++)
				{
				if (j == 8)
					j = 12;
				if (Contain.Vtx[j].Q < 0.0001 || (! Cam->RenderBeyondHorizon && Contain.Vtx[j].Q > Conditions->QMax) || Contain.Vtx[j].ScrnXYZ[2] > Conditions->ZMax)
					{
					q ++;
					} // if behind viewer or beyond horizon 
				} // for
			if (q < 9)
				{
				for (j = a = b = c = d = 0; j < 8; j ++)
					{
					if (Contain.Vtx[j].ScrnXYZ[0] < Conditions->OSLowX)
						a ++;
					else if (Contain.Vtx[j].ScrnXYZ[0] > Conditions->OSHighX)
						b ++;
					if (Contain.Vtx[j].ScrnXYZ[1] < Conditions->OSLowY)
						c ++;
					else if (Contain.Vtx[j].ScrnXYZ[1] > Conditions->OSHighY)
						d ++;
					} // for
				} // if not all corners are behind viewer 
			if (q == 9 || a == 8 || b == 8 || c == 8 || d == 8)
				TheQ[Ct].RenderMe = 0; // Skip this dude...
			else
				{
				TheQ[Ct].RenderMe = 1; // Turn me on, she says
				ActiveItems ++;
				} // else
			} // else
		} // if BoundsDiscrim
	} // for

// sort list by Q
Done = 0;
while (! Done)
	{
	Done = 1;
	for (Ct = 1; Ct < NumItems; Ct ++)
		{
		if (TheQ[Ct].QDist < TheQ[Ct - 1].QDist)
			{
			swmem(&TheQ[Ct], &TheQ[Ct - 1], sizeof (RenderQItem));
			Done = 0;
			} // if
		} // for
	} // while

return (1);

} // RenderQ::SortList

/*===========================================================================*/

bool RenderQ::GetBounds(double &LowLat, double &LowLon, double &HighLat, double &HighLon)
{
long Ct;
Joe *CurJoe;

LowLat = LowLon = FLT_MAX;
HighLat = HighLon = -FLT_MAX;

if (NumItems > 0)
	{
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		CurJoe = TheQ[Ct].QJoe;
		if (CurJoe->NWLon > HighLon)
			HighLon = CurJoe->NWLon;
		if (CurJoe->NWLat > HighLat)
			HighLat = CurJoe->NWLat;
		if (CurJoe->SELon < LowLon)
			LowLon = CurJoe->SELon;
		if (CurJoe->SELat < LowLat)
			LowLat = CurJoe->SELat;
		} // for
	} // if

return (NumItems > 0);

} // RenderQ::GetBounds

/*===========================================================================*/

RenderQItem *RenderQ::GetFirst(long &QNumber)
{
long Ct;

for (Ct = 0; Ct < NumItems; Ct ++)
	{
	if (TheQ[Ct].RenderMe)
		{
		QNumber = Ct;
		return (&TheQ[Ct]);
		} // if found one
	} // for

QNumber = -1;

return (NULL);

} // RenderQ::GetFirst

/*===========================================================================*/

// returns NULL if no more render-enabled items found, puts -1 in QNumber

RenderQItem *RenderQ::GetNext(long &QNumber)
{
long Ct;

for (Ct = QNumber + 1; Ct < NumItems; Ct ++)
	{
	if (TheQ[Ct].RenderMe)
		{
		QNumber = Ct;
		return (&TheQ[Ct]);
		} // if found one
	} // for

QNumber = -1;

return (NULL);

} // RenderQ::GetNext

/*===========================================================================*/
/*===========================================================================*/

BoundsPackage::BoundsPackage()
{

QMax = Exageration = ElevDatum = PlanetRad = EarthLatScaleMeters = 0.0;
ZMax = FLT_MAX;
OSLowX = OSHighX = OSLowY = OSHighY = 0;
DefCoords = NULL;

} // BoundsPackage::BoundsPackage

/*===========================================================================*/
/*===========================================================================*/

void DEMBoundingBox::TransformToScreen(Camera *Cam, BoundsPackage *Conditions)
{
long Ct;

for (Ct = 0; Ct < 13; Ct ++)
	{
	Vtx[Ct].Elev = Conditions->ElevDatum + (Vtx[Ct].Elev - Conditions->ElevDatum) * Conditions->Exageration;
	Cam->ProjectVertexDEM(Conditions->DefCoords, &Vtx[Ct], Conditions->EarthLatScaleMeters, Conditions->PlanetRad, 1);	// 1 means convert vertex to cartesian
	} // for

} // DEMBoundingBox::TransformToScreen

/*===========================================================================*/
/*===========================================================================*/

#ifndef WCS_VECPOLY_EFFECTS
int Renderer::MakeAllFractalDepthMaps(int ReportOptimum)
{
char FrameStr[64], filename[512], Str[512], TilingStash;
short RenderSegStash;
int Success = 1, JobsDone, Initialized = 0, Result;
long Ct, CurFrame, FrameInt, FirstFrame, LastFrame, StashQNum, DEMCt;
unsigned long FractalDepth[10], MaxDepth = 0, OptDepth = 0, SumPolys;
double CurTime, MaxPixelSize, LocalFrameRate, LocalProjectTime;
BusyWin *BWAN, *BWIM;
FILE *fFrd;
Joe *MyTurn;
RenderJob *CurJob;
RenderQItem *CurQ;
DEM *CreatedDEM;
BoundsPackage Conditions;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;

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
				if (GlobalApp->MainProj->Prefs.QueryConfigOptTrue("debug_file_remove"))
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
	} // for... 

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
		Initialized = 1;
		BWAN = new BusyWin ("Animation", LastFrame - FirstFrame + FrameInt, 'BWAN', 1);

		for (CurFrame = FirstFrame; Success && CurFrame <= LastFrame; CurFrame += FrameInt)
			{
			CurTime = CurFrame / LocalFrameRate;

			#ifdef WCS_RENDER_SCENARIOS
			if (CurFrame != FirstFrame && GlobalApp->AppEffects->UpdateScenarios(CurJob->Scenarios, CurTime, GlobalApp->AppDB))
				{
				Cleanup(0, 0, -1, FALSE);
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

								if (CurDEM->AttemptLoadDEM(CurQ->QJoe, 1, ProjectBase))
									{
									// transfer data to VertexDEMs
									if (Success = CurDEM->TransferToVerticesXYZ())
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

										// some init stuff
										DefaultFrd = min(DefaultTerrainPar->FractalDepth, CurQ->MaxFrd);

										// this will be used by DEM rendering func., DEMstr is Renderer scope
										sprintf(DEMstr, "DEM %d/%d", DEMCt + 1, DEMCue->ActiveItems);
										if (ShadowMapInProgress)
											strcat(DEMstr, "Shadow");

										// some basic vertex manipulation
										DisplaceDEMVertices(CurDEM);
										CurDEM->TransferXYZtoLatLon();
										ScaleDEMElevs(CurDEM);
										// project these vertices for use in clipping
										ProjectDEMVertices(CurDEM);

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
		Cleanup(0, 0, 1, FALSE);
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
			sprintf(Str, "Maximum fractal depth found was %d. Set fractal depth to %d for optimal quality.", MaxDepth, OptDepth);
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
int Success = 1;
long PolyNumber, VertNumber, MaxFract = 0;
BusyWin *BWDE = NULL;

if (IsCamView)
	{
	BWDE = new BusyWin(DEMstr, CurDEM->pLonEntries - 1, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(CurDEM->pLonEntries - 1, DEMstr);
	} // else

// note that Lr is incremented in the busy win update at the end of the loop
for (CurDEM->Lr = 0; Success && CurDEM->Lr < (long)CurDEM->pLonEntries - 1; )
	{
	VertNumber = CurDEM->Lr * CurDEM->pLatEntries;
	PolyNumber = (CurDEM->Lr * (CurDEM->pLatEntries - 1)) * 2;
	for (CurDEM->Lc = 0; CurDEM->Lc < (long)CurDEM->pLatEntries - 1; CurDEM->Lc ++, VertNumber ++)
		{
		#ifdef WCS_BUILD_VNS
		if (CurDEM->NullReject && (CurDEM->TestNullValue(VertNumber) || CurDEM->TestNullValue(VertNumber + 1) || CurDEM->TestNullValue(VertNumber + CurDEM->pLatEntries) || CurDEM->TestNullValue(VertNumber + CurDEM->pLatEntries + 1)))
			{
			PolyNumber += 2;
			} // if
		else
		#endif // WCS_BUILD_VNS
			{										//  _
			// render two polygons at every VertNumber station !\|	VertNumber is vertex at lower left (southwest)
			if ((MaxFract = FractalLevel(MaxPixelSize, VertNumber, VertNumber + CurDEM->pLatEntries, 
				VertNumber + 1)) > CurDEM->FractalMap[PolyNumber])
				{
				if (MaxFract >= 8)
					MaxFract = 7;
				CurDEM->FractalMap[PolyNumber] = (signed char)MaxFract;
				} // if
			if (FractalDepth && MaxFract >= 0)
				{
				FractalDepth[MaxFract] ++;
				} // if
			PolyNumber ++;

			if ((MaxFract = FractalLevel(MaxPixelSize, VertNumber + CurDEM->pLatEntries + 1, VertNumber + 1, 
				VertNumber + CurDEM->pLatEntries)) > CurDEM->FractalMap[PolyNumber])
				{
				if (MaxFract >= 8)
					MaxFract = 7;
				CurDEM->FractalMap[PolyNumber] = (signed char)MaxFract;
				} // if
			if (FractalDepth && MaxFract >= 0)
				{
				FractalDepth[MaxFract] ++;
				} // if
			PolyNumber ++;
			} // else
		} // for
	if (IsCamView)
		{
		if (BWDE->Update(++ CurDEM->Lr))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(++ CurDEM->Lr);
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
#endif // WCS_VECPOLY_EFFECTS

/*===========================================================================*/

long Renderer::FractalLevel(double MaxPixelSize, unsigned long V0, unsigned long V1, unsigned long V2)
{
double ViewVec[3], PolySide[2][3], Normal[3];
long dif[2][3], Fract, a, b, c, d, e, f;
long MaxFract = 0;

// copy the vertex coords to our root VertexData array
VertexRoot[0]->CopyXYZ(&CurDEM->Vertices[V0]);
VertexRoot[1]->CopyXYZ(&CurDEM->Vertices[V1]);
VertexRoot[2]->CopyXYZ(&CurDEM->Vertices[V2]);
VertexRoot[0]->CopyScrnXYZQ(&CurDEM->Vertices[V0]);
VertexRoot[1]->CopyScrnXYZQ(&CurDEM->Vertices[V1]);
VertexRoot[2]->CopyScrnXYZQ(&CurDEM->Vertices[V2]);

// bounds testing is the second-least expensive culling method
if (! TestInBounds((VertexBase **)VertexRoot))
	return (-1);

// test for backface
if (BackfaceCull)
	{
	// Dangerous to cull polygons that are partly in water. Can cause tears in terrain at water edge.
	// It is possible for a polygon to be visible but when a water vertex gets raised to 
	// the water level the polygon becomes not visible. The beach polygon and water polygon will 
	// not be rendered if culling is allowed when in fact both should be visible.
	if (! ((CurDEM->Vertices[V0].Flags & WCS_VERTEXDATA_FLAG_WATERVERTEX) || 
		(CurDEM->Vertices[V1].Flags & WCS_VERTEXDATA_FLAG_WATERVERTEX) || 
		(CurDEM->Vertices[V2].Flags & WCS_VERTEXDATA_FLAG_WATERVERTEX)))
		{
		ViewVec[0] = (VertexRoot[0]->XYZ[0] - Cam->CamPos->XYZ[0] + VertexRoot[1]->XYZ[0] - Cam->CamPos->XYZ[0] + VertexRoot[2]->XYZ[0] - Cam->CamPos->XYZ[0]);
		ViewVec[1] = (VertexRoot[0]->XYZ[1] - Cam->CamPos->XYZ[1] + VertexRoot[1]->XYZ[1] - Cam->CamPos->XYZ[1] + VertexRoot[2]->XYZ[1] - Cam->CamPos->XYZ[1]);
		ViewVec[2] = (VertexRoot[0]->XYZ[2] - Cam->CamPos->XYZ[2] + VertexRoot[1]->XYZ[2] - Cam->CamPos->XYZ[2] + VertexRoot[2]->XYZ[2] - Cam->CamPos->XYZ[2]);
		//FindPosVector(ViewVec, ViewVec, Cam->CamPos->XYZ);
		FindPosVector(PolySide[0], VertexRoot[1]->XYZ, VertexRoot[0]->XYZ);
		FindPosVector(PolySide[1], VertexRoot[2]->XYZ, VertexRoot[0]->XYZ);
		UnitVector(ViewVec);
		SurfaceNormal(Normal, PolySide[0], PolySide[1]);
		if (! SurfaceVisible(Normal, ViewVec, 0))
			{
			return (-1);
			} // if 
		} // if
	} // if

dif[0][0] = quickftol(VertexRoot[0]->ScrnXYZ[0] - VertexRoot[1]->ScrnXYZ[0]);
dif[0][1] = quickftol(VertexRoot[0]->ScrnXYZ[0] - VertexRoot[2]->ScrnXYZ[0]);
dif[0][2] = quickftol(VertexRoot[1]->ScrnXYZ[0] - VertexRoot[2]->ScrnXYZ[0]);
dif[1][0] = quickftol(VertexRoot[0]->ScrnXYZ[1] - VertexRoot[1]->ScrnXYZ[1]);
dif[1][1] = quickftol(VertexRoot[0]->ScrnXYZ[1] - VertexRoot[2]->ScrnXYZ[1]);
dif[1][2] = quickftol(VertexRoot[1]->ScrnXYZ[1] - VertexRoot[2]->ScrnXYZ[1]);

a = abs(dif[0][0]);
b = abs(dif[0][1]);
c = abs(dif[0][2]);
d = abs(dif[1][0]);
e = abs(dif[1][1]);
f = abs(dif[1][2]);
Fract = max(a, b);
Fract = max(Fract, c);
Fract = max(Fract, d);
Fract = max(Fract, e);
Fract = max(Fract, f);

while (Fract > MaxPixelSize && MaxFract < 255)
	{
	Fract >>= 1;  //lint !e704
	MaxFract ++;
	} // while 

return (MaxFract);

} // Renderer::FractalLevel

/*===========================================================================*/
/*===========================================================================*/

int CompareTerrainPolygonSort(const void *elem1, const void *elem2)
{

return (
	((struct TerrainPolygonSort *)elem1)->PolyQ > ((struct TerrainPolygonSort *)elem2)->PolyQ ? 1:
	(((struct TerrainPolygonSort *)elem1)->PolyQ < ((struct TerrainPolygonSort *)elem2)->PolyQ ? -1: 0)
	);

} // CompareTerrainPolygonSort

/*===========================================================================*/
/*===========================================================================*/
