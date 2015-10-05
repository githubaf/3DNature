// EffectEcosystem.cpp
// For managing Ecosystem Effects
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "EcosystemEditGUI.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Random.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "Ecotype.h"
#include "Texture.h"
#include "requester.h"
#include "Render.h"
#include "AppMem.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "EffectEval.h"
#include "FeatureConfig.h"

EcosystemEffectBase::EcosystemEffectBase(void)
{

SetDefaults();

} // EcosystemEffectBase::EcosystemEffect

/*===========================================================================*/

void EcosystemEffectBase::SetDefaults(void)
{

Resolution = 90.0f;
Randomize = 0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;
DissolveRefImageHt = 480;
DissolveByImageSize = 0;

} // EcosystemEffectBase::SetDefaults

/*===========================================================================*/
#ifdef PRE_VNS2_ECO_INIT
// ifdef'ed to get it out of the way while testing new code for island effects
GeoRaster *EcosystemEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE;
UBYTE ItemNum = 2, FillVal, ReuseItem;	// item 0 = no effect, item 1 = edge
short GradDrawn, CheckItem;
EcosystemEffect *MyEffect;

CellSize = Resolution / MetersPerDegLat;

EdgesExist = AreThereEdges(EffectList);
GradientsExist = AreThereGradients(EffectList);

// sort JoeList according to whatever is the correct priority order for this effect
JL->SortPriority(OverlapOK || GradientsExist ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);

// round out to next even cell size from reference lat/lon 
CellsNS = WCS_floor((S - RefLat) / CellSize);
CellsWE = WCS_floor((E - RefLon) / CellSize);
S = RefLat + CellsNS * CellSize;
E = RefLon + CellsWE * CellSize;
CellsNS = WCS_ceil((N - RefLat) / CellSize);
CellsWE = WCS_ceil((W - RefLon) / CellSize);
N = RefLat + CellsNS * CellSize;
W = RefLon + CellsWE * CellSize;

if ((GeoRast = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256)) && (! GeoRast->ConstructError))
	{
	DrawRast = GeoRast;
	LastGradRast = GeoRast;
	while (JL)
		{
		if (JL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! JL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (MyEffect = (EcosystemEffect *)JL->Effect)
			{
			MyEffect->AddRenderJoe(JL->Me);
			DrawRast = LastGradRast;
			GradDrawn = 0;
			if (EdgesExist)
				{
				if (MyEffect->HiResEdge)
					GeoRast->PolyRasterEdgeByte(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, 1, FALSE, TRUE);
				LastDrawRast = LastGradRast;
				DrawRast = (GeoRaster *)LastGradRast->Next;
				} // if
			if (GradientsExist)
				{
				if (MyEffect->UseGradient)
					{
					while (DrawRast)
						{
						LastDrawRast = DrawRast;
						DrawRast = (GeoRaster *)DrawRast->Next;
						} // while
					JL->Me->GetTrueBounds(NWLat, NWLon, SELat, SELon);
					// round out to next even cell size from reference lat/lon 
					CellsNS = WCS_floor((SELat - RefLat) / CellSize);
					CellsWE = WCS_floor((SELon - RefLon) / CellSize);
					SELat = RefLat + CellsNS * CellSize;
					SELon = RefLon + CellsWE * CellSize;
					CellsNS = WCS_ceil((NWLat - RefLat) / CellSize);
					CellsWE = WCS_ceil((NWLon - RefLon) / CellSize);
					NWLat = RefLat + CellsNS * CellSize;
					NWLon = RefLon + CellsWE * CellSize;

					if (((LastDrawRast->Next = new GeoRaster(NWLat, SELat, SELon, NWLon, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 1)) == NULL) || LastDrawRast->Next->ConstructError)
						break;
					DrawRast = (GeoRaster *)LastDrawRast->Next;
					LastGradRast = DrawRast;
					DrawRast->LUT[0] = MyEffect;
					DrawRast->PolyRasterFillByte(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, 255, FALSE, TRUE);
					DrawRast->GradFill = TRUE;
					GradDrawn = 1;
					//DrawRast->Grad = ...
					} // if
				else
					{
					LastDrawRast = LastGradRast;
					DrawRast = (GeoRaster *)LastDrawRast->Next;
					} // else
				} // if
			if (! GradDrawn)
				{
				if (! DrawRast)
					{
					if (((LastDrawRast->Next = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256)) == NULL) || LastDrawRast->Next->ConstructError)
						break;
					DrawRast = (GeoRaster *)LastDrawRast->Next;
					} // if
				DR = DrawRast;
				FillVal = ItemNum;
				ReuseItem = 0;
				for (CheckItem = 2; CheckItem < 256 && DrawRast->LUT[CheckItem]; CheckItem ++)
					{
					if (DrawRast->LUT[CheckItem] == MyEffect)
						{
						FillVal = (unsigned char)CheckItem;
						ReuseItem = 1;
						break;
						} // if
					} // for
				if ((DrawRast = DrawRast->PolyRasterFillByte(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, OverlapOK, FALSE)) == NULL)
					break;	// returns the last attempt to allocate new map
				// do this in the fill func: DrawRast->LUT[ItemNum] = MyEffect;
				for (; DR; DR = (GeoRaster *)DR->Next)
					DR->LUT[FillVal] = MyEffect;
				if (! ReuseItem)
					ItemNum ++;
				} // else no gradients
			} // if there truly is an effect
		JL = (RenderJoeList *)JL->Next;
		} // while
	} // if

// testing - create output bitmaps
/*
if (DrawRast)
	{
	char Name[256];
	int Count = 0;
	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		sprintf(Name, "c:\\Frames\\EcoRastTest%1d.iff", Count);
		saveILBM(8, 0, &DrawRast->ByteMap, NULL, 0, 1, 1, (short)DrawRast->Cols, (short)DrawRast->Rows, Name);
		DrawRast = DrawRast->Next;
		Count ++;
		} // while
	DrawRast = DR;
	} // if
*/
return (DrawRast);	// if the last attempt to allocate failed then NULL will be returned

} // EcosystemEffectBase::Init
#endif // PRE_VNS2_ECO_INIT

/*===========================================================================*/

#ifndef WCS_BUILD_V3
// don't forget to change calling parameter RenderJoeList *&JL
GeoRaster *EcosystemEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE, TotalCells, SizeMult;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
EcosystemEffect *MyEffect;
#ifdef WCS_ISLAND_EFFECTS
GeoRaster *FloodRast;
RenderJoeList *MatchJL;
JoeList *NegJL = NULL, *NegJLCur, **NegJLPtr;
LayerStub *MultiPartStub;
LayerEntry *MultiPartLayer;
long MinX, MinY;
#endif // WCS_ISLAND_EFFECTS
short GradDrawn, CheckItem;
UBYTE ItemNum = 2, FillVal, ReuseItem;	// item 0 = no effect, item 1 = edge
#ifdef WCS_ISLAND_EFFECTS
UBYTE ByteFill = 255;
#endif // WCS_ISLAND_EFFECTS

EdgesExist = WCS_MOSTEFFECTS_HIRESEDGES_ENABLED;	//AreThereEdges(EffectList);
GradientsExist = AreThereGradients(EffectList);

#ifdef WCS_ISLAND_EFFECTS
// sort by diminishing absolute value of area
for (RJL = JL; RJL; RJL = (RenderJoeList *)RJL->Next)
	{
	if (RJL->Me)
		{
		RJL->Area = RJL->Me->ComputeAreaDegrees();
		} // if
	} // for
JL = JL->SortByAbsArea(WCS_JOELIST_SORTORDER_HILO);
#endif // WCS_ISLAND_EFFECTS

// sort JoeList according to whatever is the correct priority order for this effect
JL->SortPriority((WCS_MOSTEFFECTS_OVERLAP_ENABLED || GradientsExist) ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);

// Start with a fixed resolution and adjust it based on a target amount of memory for this effect type
Resolution = 1.0;
CellSize = Resolution / MetersPerDegLat;

// round out to next even cell size from reference lat/lon 
CellsNS = fabs(WCS_floor((S - RefLat) / CellSize)) + fabs(WCS_ceil((N - RefLat) / CellSize));
CellsWE = fabs(WCS_floor((E - RefLon) / CellSize)) + fabs(WCS_ceil((W - RefLon) / CellSize));

// Desired number of cells yields a WCS_MOSTEFFECTS_MAXCELLS GeoRaster file max.
// That will be one file for edges and at least one, probably two files, for rasters so a total of 
// 3 * WCS_MOSTEFFECTS_MAXCELLS bytes for a typical scenario of two slightly overlapping effect vectors.
TotalCells = CellsNS * CellsWE;
if (TotalCells > WCS_MOSTEFFECTS_MAXCELLS)
	{
	SizeMult = sqrt(TotalCells / WCS_MOSTEFFECTS_MAXCELLS);
	CellSize *= SizeMult;
	Resolution = (float)(SizeMult * Resolution);
	} // if
else if (TotalCells < WCS_MOSTEFFECTS_MINCELLS)
	{
	//SizeMult = sqrt(TotalCells / WCS_MOSTEFFECTS_MINCELLS);
	SizeMult = TotalCells / WCS_MOSTEFFECTS_MINCELLS;
	CellSize *= SizeMult;
	Resolution = (float)(SizeMult * Resolution);
	} // if
CellsNS = WCS_floor((S - RefLat) / CellSize);
CellsWE = WCS_floor((E - RefLon) / CellSize);
S = RefLat + CellsNS * CellSize;
E = RefLon + CellsWE * CellSize;
CellsNS = WCS_ceil((N - RefLat) / CellSize);
CellsWE = WCS_ceil((W - RefLon) / CellSize);
N = RefLat + CellsNS * CellSize;
W = RefLon + CellsWE * CellSize;

if ((GeoRast = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256)) && (! GeoRast->ConstructError))
	{
	DrawRast = GeoRast;
	LastGradRast = GeoRast;
	for (RJL = JL; RJL; RJL = (RenderJoeList *)RJL->Next)
		{
		// skip it if already drawn
		if (! RJL->Drawn)
			{
			if (RJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! RJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				continue;
				} // if 
			if (MyEffect = (EcosystemEffect *)RJL->Effect)
				{
				#ifdef WCS_ISLAND_EFFECTS
				// find out if there are multiparts and if this is a positive or negative area
				// build list of negative area counterparts inside this joe
				// mark negative vectors as done
				// check and see if this is a multi-part vector and if it has any negative area cousins
				if (MultiPartLayer = RJL->Me->GetMultiPartLayer())
					{
					NegJLPtr = &NegJL;
					// if this is a negative area of a multipart vector, don't draw it
					if (RJL->Area < 0.0)
						continue;
					// get first layer stub
					MultiPartStub = MultiPartLayer->FirstStub();
					for (MultiPartStub = MultiPartLayer->FirstStub(); MultiPartStub; MultiPartStub = MultiPartStub->NextObjectInLayer())
						{
						// find the RJL that corresponds by walking the joe list
						MatchJL = RJL;
						for (MatchJL = RJL; MatchJL; MatchJL = (RenderJoeList *)MatchJL->Next)
							{
							// both Joe and Effect need to match
							if (MatchJL->Me == MultiPartStub->MyObject() && MatchJL->Effect == MyEffect)
								{
								// if negative area and this vector is in same overall region draw into buffer
								if (MatchJL->Area < 0.0 && RJL->Me->IsJoeContainedInMyGeoBounds(MatchJL->Me))
									{
									MatchJL->Drawn = 1;
									if (! (*NegJLPtr = new JoeList()))
										{
										goto ErrorReturn;
										} // if
									(*NegJLPtr)->Me = MatchJL->Me;
									NegJLPtr = &(*NegJLPtr)->Next;
									} // if
								break;
								} // if
							} // for
						} // for
					} // if
				#endif // WCS_ISLAND_EFFECTS
				MyEffect->AddRenderJoe(RJL->Me);
				DrawRast = LastGradRast;
				GradDrawn = 0;
				if (EdgesExist)
					{
					if (MyEffect->HiResEdge)
						GeoRast->PolyRasterEdgeByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, 1, FALSE, TRUE);
					LastDrawRast = LastGradRast;
					DrawRast = (GeoRaster *)LastGradRast->Next;
					} // if
				if (GradientsExist)
					{
					if (MyEffect->UseGradient)
						{
						while (DrawRast)
							{
							LastDrawRast = DrawRast;
							DrawRast = (GeoRaster *)DrawRast->Next;
							} // while
						RJL->Me->GetTrueBounds(NWLat, NWLon, SELat, SELon);
						// round out to next even cell size from reference lat/lon 
						CellsNS = WCS_floor((SELat - RefLat) / CellSize);
						CellsWE = WCS_floor((SELon - RefLon) / CellSize);
						SELat = RefLat + CellsNS * CellSize;
						SELon = RefLon + CellsWE * CellSize;
						CellsNS = WCS_ceil((NWLat - RefLat) / CellSize);
						CellsWE = WCS_ceil((NWLon - RefLon) / CellSize);
						NWLat = RefLat + CellsNS * CellSize;
						NWLon = RefLon + CellsWE * CellSize;

						if (((LastDrawRast->Next = new GeoRaster(NWLat, SELat, SELon, NWLon, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 1)) == NULL) || LastDrawRast->Next->ConstructError)
							break;
						DrawRast = (GeoRaster *)LastDrawRast->Next;
						LastGradRast = DrawRast;
						DrawRast->LUT[0] = MyEffect;
						#ifdef WCS_ISLAND_EFFECTS
						if (NegJL)
							{
							// create flood map of positive areas with negative cutouts
							// transfer the floodmap to DrawRast
							if (DrawRast->AllocFloodmap(RJL->Me, ~ByteFill, MinX, MinY))
								{
								FloodRast = DrawRast;
								// draw negative vectors with color
								for (NegJLCur = NegJL; NegJLCur; NegJLCur = NegJLCur->Next)
									{
									DrawRast->PolyRasterEdgeByteFloodmap(NegJLCur->Me, ByteFill, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
									} // for
								// flood exterior region of floodmap with color
								DrawRast->SeedFill(ByteFill);
								// draw negative outlines in non color
								for (NegJLCur = NegJL; NegJLCur; NegJLCur = NegJLCur->Next)
									DrawRast->PolyRasterEdgeByteFloodmap(NegJLCur->Me, ~ByteFill, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
								// draw the real vector of interest in non-color
								DrawRast->PolyRasterEdgeByteFloodmap(RJL->Me, ~ByteFill, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
								// flood exterior region of floodmap with non-color
								DrawRast->SeedFill(~ByteFill);
								// draw the real vector of interest in color
								DrawRast->PolyRasterEdgeByteFloodmap(RJL->Me, ByteFill, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
								// copy result to main georaster, watermelon fill
								if ((DrawRast = DrawRast->PolyRasterCopyFloodByte(MaxMem, UsedMem, WCS_RASTER_EFFECT_BAND_NORMAL, ByteFill, FALSE, TRUE, MinX, MinY)) == NULL)
									goto ErrorReturn;	// returns the last attempt to allocate new map
								FloodRast->FreeFloodmap();
								while (NegJL)
									{
									NegJLCur = NegJL->Next;
									delete NegJL;
									NegJL = NegJLCur;
									} // while
								} // if
							else
								{
								goto ErrorReturn;
								} // else
							} // if
						else
						#endif // WCS_ISLAND_EFFECTS
							{
							DrawRast->PolyRasterFillByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, 255, FALSE, TRUE);
							} // else
						DrawRast->GradFill = TRUE;
						GradDrawn = 1;
						} // if
					else
						{
						LastDrawRast = LastGradRast;
						DrawRast = (GeoRaster *)LastDrawRast->Next;
						} // else
					} // if
				if (! GradDrawn)
					{
					if (! DrawRast)
						{
						if (((LastDrawRast->Next = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256)) == NULL) || LastDrawRast->Next->ConstructError)
							break;
						DrawRast = (GeoRaster *)LastDrawRast->Next;
						} // if
					DR = DrawRast;
					FillVal = ItemNum;
					ReuseItem = 0;
					for (CheckItem = 2; CheckItem < 256 && DrawRast->LUT[CheckItem]; CheckItem ++)
						{
						if (DrawRast->LUT[CheckItem] == MyEffect)
							{
							FillVal = (unsigned char)CheckItem;
							ReuseItem = 1;
							break;
							} // if
						} // for
					#ifdef WCS_ISLAND_EFFECTS
					if (NegJL)
						{
						// create flood map of positive areas with negative cutouts
						// transfer the floodmap to DrawRast
						if (DrawRast->AllocFloodmap(RJL->Me, ~ByteFill, MinX, MinY))
							{
							FloodRast = DrawRast;
							// draw negative vectors with color
							for (NegJLCur = NegJL; NegJLCur; NegJLCur = NegJLCur->Next)
								{
								DrawRast->PolyRasterEdgeByteFloodmap(NegJLCur->Me, FillVal, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
								} // for
							// flood exterior region of floodmap with color
							DrawRast->SeedFill(FillVal);
							// draw negative outlines in non color
							for (NegJLCur = NegJL; NegJLCur; NegJLCur = NegJLCur->Next)
								DrawRast->PolyRasterEdgeByteFloodmap(NegJLCur->Me, ~FillVal, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
							// draw the real vector of interest in non-color
							DrawRast->PolyRasterEdgeByteFloodmap(RJL->Me, ~FillVal, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
							// flood exterior region of floodmap with non-color
							DrawRast->SeedFill(~FillVal);
							// draw the real vector of interest in color
							DrawRast->PolyRasterEdgeByteFloodmap(RJL->Me, FillVal, MinX - 1, MinY - 1, DrawRast->xsize, DrawRast->ysize);
							// copy result to main georaster
							if ((DrawRast = DrawRast->PolyRasterCopyFloodByte(MaxMem, UsedMem, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, OverlapOK, FALSE, MinX, MinY)) == NULL)
								goto ErrorReturn;	// returns the last attempt to allocate new map
							FloodRast->FreeFloodmap();
							} // if
						while (NegJL)
							{
							NegJLCur = NegJL->Next;
							delete NegJL;
							NegJL = NegJLCur;
							} // while
						} // if
					else
					#endif // WCS_ISLAND_EFFECTS
						{
						if ((DrawRast = DrawRast->PolyRasterFillByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, OverlapOK, FALSE)) == NULL)
							break;	// returns the last attempt to allocate new map
						} // else
					// do this in the fill func: DrawRast->LUT[ItemNum] = MyEffect;
					for (; DR; DR = (GeoRaster *)DR->Next)
						DR->LUT[FillVal] = MyEffect;
					if (! ReuseItem)
						ItemNum ++;
					} // else no gradients
				} // if there truly is an effect
			RJL->Drawn = 1;
			} // if ! already drawn
		} // for
	} // if

// testing - create output bitmaps
/*
if (DrawRast)
	{
	char Name[256];
	int Count = 0;
	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		sprintf(Name, "c:\\Frames\\EcoRastTest%1d.iff", Count);
		saveILBM(8, 0, &DrawRast->ByteMap, NULL, 0, 1, 1, (short)DrawRast->Cols, (short)DrawRast->Rows, Name);
		DrawRast = DrawRast->Next;
		Count ++;
		} // while
	DrawRast = DR;
	} // if
*/
return (DrawRast);	// if the last attempt to allocate failed then NULL will be returned

#ifdef WCS_ISLAND_EFFECTS
ErrorReturn:

while (NegJL)
	{
	NegJLCur = NegJL->Next;
	delete NegJL;
	NegJL = NegJLCur;
	} // while
return (NULL);
#endif // WCS_ISLAND_EFFECTS

} // EcosystemEffectBase::Init

/*===========================================================================*/
#else // WCS_BUILD_V3
GeoRaster *EcosystemEffectBase::Init(EffectEval *EvalEffects, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, CellsNS, CellsWE, TotalCells, SizeMult;
unsigned long NumPolygons = 0;
RenderJoeList *RJL;
VectorPolygonListDouble *BuiltList = NULL, *CurList;
GeoRaster *DrawRast = NULL;
bool Success = true;

// sort by diminishing absolute value of area
for (RJL = JL; RJL; RJL = (RenderJoeList *)RJL->Next)
	{
	if (RJL->Me)
		{
		RJL->Area = RJL->Me->ComputeAreaDegrees();
		} // if
	} // for
JL = JL->SortByAbsArea(WCS_JOELIST_SORTORDER_HILO);

// sort JoeList high to low
JL->SortPriority(WCS_JOELIST_SORTORDER_HILO);

// Start with a fixed resolution and adjust it based on a target amount of memory for this effect type
Resolution = 1.0;
CellSize = Resolution / MetersPerDegLat;

// round out to next even cell size from reference lat/lon 
CellsNS = fabs(WCS_floor((S - RefLat) / CellSize)) + fabs(WCS_ceil((N - RefLat) / CellSize));
CellsWE = fabs(WCS_floor((E - RefLon) / CellSize)) + fabs(WCS_ceil((W - RefLon) / CellSize));

// Desired number of cells yields a WCS_MOSTEFFECTS_MAXCELLS GeoRaster file max.
// That will be one file for edges and at least one, probably two files, for rasters so a total of 
// 3 * WCS_MOSTEFFECTS_MAXCELLS bytes for a typical scenario of two slightly overlapping effect vectors.
TotalCells = CellsNS * CellsWE;
if (TotalCells > WCS_MOSTEFFECTS_MAXCELLS)
	{
	SizeMult = sqrt(TotalCells / WCS_MOSTEFFECTS_MAXCELLS);
	CellSize *= SizeMult;
	Resolution = (float)(SizeMult * Resolution);
	} // if
else if (TotalCells < WCS_MOSTEFFECTS_MINCELLS)
	{
	//SizeMult = sqrt(TotalCells / WCS_MOSTEFFECTS_MINCELLS);
	SizeMult = TotalCells / WCS_MOSTEFFECTS_MINCELLS;
	CellSize *= SizeMult;
	Resolution = (float)(SizeMult * Resolution);
	} // if
CellsNS = WCS_floor((S - RefLat) / CellSize);
CellsWE = WCS_floor((E - RefLon) / CellSize);
S = RefLat + CellsNS * CellSize;
E = RefLon + CellsWE * CellSize;
CellsNS = WCS_ceil((N - RefLat) / CellSize);
CellsWE = WCS_ceil((W - RefLon) / CellSize);
N = RefLat + CellsNS * CellSize;
W = RefLon + CellsWE * CellSize;

if (BuiltList = EvalEffects->BuildPolygons(JL, NumPolygons))
	{
	if ((GeoRast = new GeoRaster(N, S, E, W, CellSize, UsedMem)) && (! GeoRast->ConstructError))
		{
		DrawRast = GeoRast;
		for (CurList = BuiltList; CurList && Success; CurList = (VectorPolygonListDouble *)CurList->NextPolygonList)
			{
			Success = DrawRast->PlotPolygon(CurList->MyPolygon);
			} // for
		} // if
	} // if

return (DrawRast);	// if the attempt to allocate failed then NULL will be returned

} // EcosystemEffectBase::Init
#endif // WCS_BUILD_V3

/*===========================================================================*/

void EcosystemEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // EcosystemEffectBase::Destroy

/*===========================================================================*/

short EcosystemEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // EcosystemEffectBase::AreThereEdges

/*===========================================================================*/

short EcosystemEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // EcosystemEffectBase::AreThereGradients

/*===========================================================================*/

void EcosystemEffectBase::SetFloating(int NewFloating, Project *CurProj)
{
double CellSizeNS, CellSizeWE;
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	if (GlobalApp->AppDB->GetMinDEMCellSizeMeters(CellSizeNS, CellSizeWE, CurProj))
		{
		CellSizeNS = min(CellSizeNS, CellSizeWE);
		CellSizeNS = (int)max(CellSizeNS, 1.0);
		Resolution = (float)CellSizeNS;
		} // if
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // EcosystemEffectBase::SetFloating

/*===========================================================================*/

void EcosystemEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // EcosystemEffectBase::SetFloating

/*===========================================================================*/

// note that align to vector textured ecosystems only work if high-res edges are turned on
// since there is no Joe list search otherwise
int EcosystemEffectBase::Eval(PolygonData *Poly)
{
double HalfRes, Lat, Lon, ActualSum;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
EcosystemEffect *Effect;
short Priority = -1000, ValuesRead = 0, Done = 0;

HalfRes = Resolution * 0.5;
Lat = Poly->Lat;
Lon = Poly->Lon;
Rand.Seed64BitShift(Poly->LatSeed + WCS_SEEDOFFSET_ECOSYSTEM, Poly->LonSeed + WCS_SEEDOFFSET_ECOSYSTEM);
TempEco = NULL;

if (Randomize)
	{
	Lat += (2 * GeoRast->CellSizeNS * (.5 - Rand.GenPRN()));
	Lon += (2 * GeoRast->CellSizeEW * (.5 - Rand.GenPRN()));
	} // if

// get value from first map
if (GradientsExist)
	{
	if (EdgesExist)
		{
		Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
		if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // search maps for values to do edge test on
			if (OverlapOK)
				{ // find all containing objects until priority changes
				SumWt = ActualSum = 0.0;
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (EcosystemEffect *)Rast->LUT[0];
							if (Effect->Priority >= Priority)
								{
								if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
									Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Priority = Effect->Priority;
										Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
										SumWt = ActualSum + fabs(Wt);
										if (SumWt < 1.0)
											SumWt = 1.0;
										ValuesRead ++;
										if (CompareWeighted(Effect))
											{
											Done = 1;
											ActualSum += fabs(Wt);
											} // if
										} // if
									} // if slope within range
								} // if priority
							else
								break;
							} // if gradfill
						else
							{
							Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->Priority >= Priority)
								{
								if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
									Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Priority = Effect->Priority;
										Wt = 1.0;
										SumWt = ActualSum + 1.0;
										ValuesRead ++;
										if (CompareWeighted(Effect))
											{
											Done = 1;
											ActualSum += 1.0;
											} // if
										} // if
									} // if slope within range
								} // if priority
							else
								break;
							} // else not gradfill
						} // if a value in map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				if (Done)
					{
					Poly->Eco = TempEco;
					} // if
				} // if OverlapOK
			else
				{ // find first containing object
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (EcosystemEffect *)Rast->LUT[0];
							if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
								Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
									SumWt = fabs(Wt);
									Done = 1;
									} // if contained
								} // if slope
							} // if gradfill
						else
							{
							Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
								Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Wt = 1.0;
									SumWt = 1.0;
									Done = 1;
									} // if
								} // if
							} // if
						if (Done)
							{
							if (SumWt < 1.0)
								{
								if (Rand.GenPRN() <= SumWt)
									Poly->Eco = Effect;
								else
									Done = 0;
								} // if
							else
								{
								Poly->Eco = Effect;
								} // else
							break;
							} // if
						} // if a value in edge map
					Rast = (GeoRaster *)Rast->Next;
					} // while Rast
				} // else no overlap
			if (! Done)
				Done = -1;
			} // if a value in edge map
		} // if hires edges
	if (! Done)
		{
		Rast = FirstDataRast;
		if (OverlapOK)
			{ // check maps until lower priority is encountered
			SumWt = ActualSum = 0.0;
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					if (Rast->GradFill)
						{
						Effect = (EcosystemEffect *)Rast->LUT[0];
						if (Effect->Priority >= Priority)
							{
							if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
								Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
								{
								Priority = Effect->Priority;
								Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
								SumWt = ActualSum + fabs(Wt);
								if (SumWt < 1.0)
									SumWt = 1.0;
								ValuesRead ++;
								if (CompareWeighted(Effect))
									{
									Done = 1;
									ActualSum += fabs(Wt);
									} // if
								} // if slope
							} // if priority
						else
							break;
						} // if
					else
						{
						Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Effect->Priority >= Priority)
							{
							if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
								Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
								{
								Priority = Effect->Priority;
								Wt = 1.0;
								SumWt = ActualSum + 1.0;
								ValuesRead ++;
								if (CompareWeighted(Effect))
									{
									Done = 1;
									ActualSum += 1.0;
									} // if
								} // if
							} // if
						else
							break;
						} // if
					} // if a value in map
				Rast = (GeoRaster *)Rast->Next;
				} // while
			if (Done)
				{
				Poly->Eco = TempEco;
				} // if
			} // else if overlap
		else
			{ // search first non-gradient map
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					if (Rast->GradFill)
						{
						Effect = (EcosystemEffect *)Rast->LUT[0];
						if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
							Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
							{
							Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
							SumWt = fabs(Wt);
							Done = 1;
							} // if
						} // if
					else
						{
						Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
							Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
							{
							Wt = 1.0;
							SumWt = 1.0;
							Done = 1;
							} // if
						} // if
					if (Done)
						{
						if (SumWt < 1.0)
							{
							if (Rand.GenPRN() <= SumWt)
								Poly->Eco = Effect;
							else
								Done = 0;
							} // if
						else
							{
							Poly->Eco = Effect;
							} // else
						} // if
					break;
					} // if a value in map
				Rast = (GeoRaster *)Rast->Next;
				} // while
			} // else
		} // if ! Done
	if (! Done)
		Done = -1;
	} // if gradients
else if (EdgesExist)
	{
	Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
	if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
		{ // search maps for values to do edge test on
		if (OverlapOK)
			{ // find all containing objects until priority changes
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->Priority >= Priority)
						{
						if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
							Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
							{
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								Priority = Effect->Priority;
								ValuesRead ++;
								Compare(Effect, ValuesRead);
								Done = 1;
								} // if
							} // if
						} // if equal or higher priority
					else
						break;
					} // if a value in map
				else
					break;
				Rast = (GeoRaster *)Rast->Next;
				} // while
			if (Done)
				{
				Poly->Eco = TempEco;
				} // if
			} // if
		else
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
						Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
						{
						if (Effect->RenderJoes->SimpleContained(Lat, Lon))
							{
							Poly->Eco = Effect;
							Done = 1;
							break;
							} // if
						} // if
					} // if
				Rast = (GeoRaster *)Rast->Next;
				} // while
			} // else
		if (! Done)
			Done = -1;
		} // if a value in edge map
	} // else if hires edges
if (! Done)
	{
	Rast = FirstDataRast;
	if (OverlapOK)
		{ // check maps until lower priority is encountered
		while (Rast)
			{
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if (Effect->Priority >= Priority)
					{
					if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
						Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
						{
						Priority = Effect->Priority;
						ValuesRead ++;
						if (Compare(Effect, ValuesRead))
							Done = 1;
						} // if
					} // if equal or higher priority
				else
					break;
				} // if a value in map
			else
				break;
			Rast = (GeoRaster *)Rast->Next;
			} // while
		if (Done)
			{
			Poly->Eco = TempEco;
			} // if
		} // if overlap
	else
		{ // everything in one map - nice 'n easy
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if (Poly->Slope <= Effect->MaxSlope && Poly->Slope >= Effect->MinSlope &&
				Poly->RelEl <= Effect->MaxRelEl && Poly->RelEl >= Effect->MinRelEl)
				{
				Poly->Eco = Effect;
				Done = 1;
				} // if
			} // if a value in edge map
		} // else
	} // if

return (Done > 0);

} // EcosystemEffectBase::Eval

/*===========================================================================*/
/*
bool EcosystemEffectBase::Eval(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages)
{
double HalfRes, Lat, Lon, ActualSum, AccumulatedWt = 0.0;
unsigned long LonSeed, LatSeed;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
EcosystemEffect *Effect, *FoundEco = NULL;
Joe *FoundJoe = NULL;
bool Done = false;
short Priority = -1000, ValuesRead = 0;
TwinMaterials MatTwin;

if (GeoRast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL])
	{
	HalfRes = Resolution * 0.5;
	Lat = CurNode->Lat;
	Lon = CurNode->Lon;
							
	LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX) + WCS_SEEDOFFSET_ECOSYSTEM;
	LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX) + WCS_SEEDOFFSET_ECOSYSTEM;
	Rand.Seed64BitShift(LonSeed, LonSeed);
	TempEco = NULL;

	if (Randomize)
		{
		Lat += (2 * GeoRast->CellSizeNS * (.5 - Rand.GenPRN()));
		Lon += (2 * GeoRast->CellSizeEW * (.5 - Rand.GenPRN()));
		} // if

	// get value from first map
	if (GradientsExist)
		{
		if (EdgesExist)
			{
			Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
			if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // search maps for values to do edge test on
				if (OverlapOK)
					{ // find all containing objects until priority changes
					SumWt = ActualSum = 0.0;
					while (Rast)
						{
						if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
							{ // process data
							if (Rast->GradFill)
								{
								Effect = (EcosystemEffect *)Rast->LUT[0];
								if (Effect->Priority >= Priority)
									{
									if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
										CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
										{
										if (Effect->RenderJoes->SimpleContained(Lat, Lon))
											{
											Priority = Effect->Priority;
											Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
											SumWt = ActualSum + fabs(Wt);
											if (SumWt < 1.0)
												SumWt = 1.0;
											ValuesRead ++;
											if (CompareWeighted(Effect))
												{
												Done = true;
												ActualSum += fabs(Wt);
												} // if
											} // if
										} // if slope within range
									} // if priority
								else
									break;
								} // if gradfill
							else
								{
								Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
								if (Effect->Priority >= Priority)
									{
									if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
										CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
										{
										if (Effect->RenderJoes->SimpleContained(Lat, Lon))
											{
											Priority = Effect->Priority;
											Wt = 1.0;
											SumWt = ActualSum + 1.0;
											ValuesRead ++;
											if (CompareWeighted(Effect))
												{
												Done = true;
												ActualSum += 1.0;
												} // if
											} // if
										} // if slope within range
									} // if priority
								else
									break;
								} // else not gradfill
							} // if a value in map
						Rast = (GeoRaster *)Rast->Next;
						} // while
					if (Done)
						{
						FoundEco = TempEco;
						} // if
					} // if OverlapOK
				else
					{ // find first containing object
					while (Rast)
						{
						if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
							{ // process data
							if (Rast->GradFill)
								{
								Effect = (EcosystemEffect *)Rast->LUT[0];
								if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
									CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
										SumWt = fabs(Wt);
										Done = true;
										} // if contained
									} // if slope
								} // if gradfill
							else
								{
								Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
								if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
									CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Wt = 1.0;
										SumWt = 1.0;
										Done = true;
										} // if
									} // if
								} // if
							if (Done)
								{
								if (SumWt < 1.0)
									{
									if (Rand.GenPRN() <= SumWt)
										FoundEco = Effect;
									else
										Done = false;
									} // if
								else
									{
									FoundEco = Effect;
									} // else
								break;
								} // if
							} // if a value in edge map
						Rast = (GeoRaster *)Rast->Next;
						} // while Rast
					} // else no overlap
				if (! Done)
					Done = true;
				} // if a value in edge map
			} // if hires edges
		if (! Done)
			{
			Rast = FirstDataRast;
			if (OverlapOK)
				{ // check maps until lower priority is encountered
				SumWt = ActualSum = 0.0;
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (EcosystemEffect *)Rast->LUT[0];
							if (Effect->Priority >= Priority)
								{
								if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
									CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
									{
									Priority = Effect->Priority;
									Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
									SumWt = ActualSum + fabs(Wt);
									if (SumWt < 1.0)
										SumWt = 1.0;
									ValuesRead ++;
									if (CompareWeighted(Effect))
										{
										Done = true;
										ActualSum += fabs(Wt);
										} // if
									} // if slope
								} // if priority
							else
								break;
							} // if
						else
							{
							Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->Priority >= Priority)
								{
								if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
									CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
									{
									Priority = Effect->Priority;
									Wt = 1.0;
									SumWt = ActualSum + 1.0;
									ValuesRead ++;
									if (CompareWeighted(Effect))
										{
										Done = true;
										ActualSum += 1.0;
										} // if
									} // if
								} // if
							else
								break;
							} // if
						} // if a value in map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				if (Done)
					{
					FoundEco = TempEco;
					} // if
				} // else if overlap
			else
				{ // search first non-gradient map
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (EcosystemEffect *)Rast->LUT[0];
							if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
								CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
								{
								Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
								SumWt = fabs(Wt);
								Done = true;
								} // if
							} // if
						else
							{
							Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
								CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
								{
								Wt = 1.0;
								SumWt = 1.0;
								Done = true;
								} // if
							} // if
						if (Done)
							{
							if (SumWt < 1.0)
								{
								if (Rand.GenPRN() <= SumWt)
									FoundEco = Effect;
								else
									Done = false;
								} // if
							else
								{
								FoundEco = Effect;
								} // else
							} // if
						break;
						} // if a value in map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				} // else
			} // if ! Done
		if (! Done)
			Done = true;
		} // if gradients
	else if (EdgesExist)
		{
		Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
		if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // search maps for values to do edge test on
			if (OverlapOK)
				{ // find all containing objects until priority changes
				while (Rast)
					{
					if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
						{ // process data
						Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Effect->Priority >= Priority)
							{
							if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
								CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Priority = Effect->Priority;
									ValuesRead ++;
									Compare(Effect, ValuesRead);
									Done = true;
									} // if
								} // if
							} // if equal or higher priority
						else
							break;
						} // if a value in map
					else
						break;
					Rast = (GeoRaster *)Rast->Next;
					} // while
				if (Done)
					{
					FoundEco = TempEco;
					} // if
				} // if
			else
				{ // find first containing object
				while (Rast)
					{
					if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
						{ // do edge test
						Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
							CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
							{
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								FoundEco = Effect;
								Done = true;
								break;
								} // if
							} // if
						} // if
					Rast = (GeoRaster *)Rast->Next;
					} // while
				} // else
			if (! Done)
				Done = true;
			} // if a value in edge map
		} // else if hires edges
	if (! Done)
		{
		Rast = FirstDataRast;
		if (OverlapOK)
			{ // check maps until lower priority is encountered
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->Priority >= Priority)
						{
						if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
							CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
							{
							Priority = Effect->Priority;
							ValuesRead ++;
							if (Compare(Effect, ValuesRead))
								Done = true;
							} // if
						} // if equal or higher priority
					else
						break;
					} // if a value in map
				else
					break;
				Rast = (GeoRaster *)Rast->Next;
				} // while
			if (Done)
				{
				FoundEco = TempEco;
				} // if
			} // if overlap
		else
			{ // everything in one map - nice 'n easy
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (EcosystemEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if (CurNode->NodeData->Slope <= Effect->MaxSlope && CurNode->NodeData->Slope >= Effect->MinSlope &&
					CurNode->NodeData->RelEl <= Effect->MaxRelEl && CurNode->NodeData->RelEl >= Effect->MinRelEl)
					{
					FoundEco = Effect;
					} // if
				} // if a value in edge map
			} // else
		} // if
	} // if ByteMap[]
	
if (FoundEco)
	{
	if (FoundEco->EcoMat.GetRenderMaterial(&MatTwin, FoundEco->GetRenderMatGradientPos(Rend, FoundJoe, CurNode, NULL)))
		{
		if (MatTwin.Mat[0])
			{
			AccumulatedWt += MatTwin.Covg[0] * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
			if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], FoundEco, FoundJoe, (float)(MatTwin.Covg[0])))
				return (false);
			} // if
		if (MatTwin.Mat[1])
			{
			AccumulatedWt += MatTwin.Covg[1] * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
			if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], FoundEco, FoundJoe, (float)(MatTwin.Covg[1])))
				return (false);
			} // if
		SumOfAllCoverages += AccumulatedWt;
		} // if
	} // if

return (Done);

} // EcosystemEffectBase::Eval
*/
/*===========================================================================*/

short EcosystemEffectBase::Compare(EcosystemEffect *Effect, short ValuesRead)
{

if (ValuesRead <= 1 || Rand.GenPRN() <= (1.0 / ValuesRead))
	{
	TempEco = Effect;
	return (1);
	} // if

return (0);

} // EcosystemEffectBase::Compare

/*===========================================================================*/

short EcosystemEffectBase::CompareWeighted(EcosystemEffect *Effect)
{

if (Wt >= SumWt || (SumWt > 0.0 && Rand.GenPRN() <= fabs(Wt) / SumWt))
	{
	TempEco = Effect;
	return (1);
	} // if

return (0);

} // EcosystemEffectBase::CompareWeighted

/*===========================================================================*/

bool EcosystemEffectBase::Eval(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
{
double DistanceToVector, dX, dY, dZ, ProfWt, CurMatWt, AccumulatedWt = 0.0;
long CurCell;
EcosystemEffect *CurEffect;
Joe *CurJoe;
MaterialList *FirstMaterial = NULL;
VectorPolygonList *CurList;
bool Done = false, UseIt, Success = true;
short TopPriority = -10000;
TwinMaterials MatTwin;

if (GeoRast->PolyListBlock)
	{
	TempEco = NULL;

	if ((CurCell = GeoRast->GetCell(CurNode->Lat, CurNode->Lon)) >= 0)
		{
		// walk list of vector polygons at that cell
			
		for (CurList = GeoRast->PolyListBlock[CurCell]; CurList; CurList = CurList->NextPolygonList)
			{
			CurEffect = (EcosystemEffect *)CurList->MyPolygon->MyEffects->MyEffect;
			CurJoe = CurList->MyPolygon->MyEffects->MyJoe;
			if (CurEffect->Priority < TopPriority)
				{
				if (AccumulatedWt >= 1.0)
					{
					if (AccumulatedWt > 1.0 && FirstMaterial)
						{
						// normalize the ecosystem coverages just added so they add to 100%
						for (; FirstMaterial; FirstMaterial = FirstMaterial->NextMaterial)
							{
							FirstMaterial->MatCoverage /= (float)AccumulatedWt;
							} // for
						AccumulatedWt = 1.0;
						} // if
					SumOfAllCoverages += AccumulatedWt;
					break;
					} // if
				SumOfAllCoverages += AccumulatedWt;
				AccumulatedWt = 0.0;
				FirstMaterial = NULL;
				} // if
			if (CurNode->NodeData->Slope <= CurEffect->MaxSlope && CurNode->NodeData->Slope >= CurEffect->MinSlope &&
				CurNode->NodeData->RelEl <= CurEffect->MaxRelEl && CurNode->NodeData->RelEl >= CurEffect->MinRelEl)
				{
				UseIt = true;
				if (CurList->FlagCheck(WCS_VECTORPOLYGONLIST_FLAG_EDGE))
					{
					if ((CurList->MyPolygon->TestPointContained(CurNode, 0.0)) == WCS_TEST_POINT_CONTAINED_OUTSIDE)
						UseIt = false;
					} // if
				if (UseIt)
					{
					if (CurEffect->UseGradient)
						{
						DistanceToVector = fabs(CurJoe->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, 
							Rend->EarthLatScaleMeters, true, Rend->ElevDatum, Rend->Exageration, dX, dY, dZ));
						ProfWt = CurEffect->ADProf.GetValue(0, DistanceToVector);
						} // if
					else
						ProfWt = 1.0;
					} // if
				else
					ProfWt = -1.0;
					
				if (ProfWt >= 0.0)
					{
					if (CurEffect->EcoMat.GetRenderMaterial(&MatTwin, CurEffect->GetRenderMatGradientPos(Rend, CurJoe, CurNode, NULL)))
						{
						TopPriority = CurEffect->Priority;
						Done = true;
						if (MatTwin.Mat[0])
							{
							CurMatWt = ProfWt * MatTwin.Covg[0];
							if (CurEffect->PlowSnow)
								PlowedSnow += CurMatWt;
							AccumulatedWt += CurMatWt * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! FirstMaterial)
								{
								if (! (FirstMaterial = CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurJoe, (float)(CurMatWt))))
									{
									Success = false;
									break;
									} // if
								} // if
							else
								{
								if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurJoe, (float)(ProfWt * MatTwin.Covg[0])))
									{
									Success = false;
									break;
									} // if
								} // else
							} // if
						if (MatTwin.Mat[1])
							{
							CurMatWt = ProfWt * MatTwin.Covg[1];
							if (CurEffect->PlowSnow)
								PlowedSnow += CurMatWt;
							AccumulatedWt += CurMatWt * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, CurJoe, (float)(CurMatWt)))
								{
								Success = false;
								break;
								} // if
							} // if
						} // if
					} // if
				} // if
			} // for
		if (AccumulatedWt > 1.0 && FirstMaterial)
			{
			// normalize the ecosystem coverages just added so they add to 100%
			for (; FirstMaterial; FirstMaterial = FirstMaterial->NextMaterial)
				{
				FirstMaterial->MatCoverage /= (float)AccumulatedWt;
				} // for
			AccumulatedWt = 1.0;
			} // if
		SumOfAllCoverages += AccumulatedWt;
		} // if
	} // if in bounds
	
return (Success);

} // EcosystemEffectBase::Eval


/*===========================================================================*/

bool EcosystemEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
{
double DistanceToVector, dX, dY, dZ, ProfWt, CurMatWt, AccumulatedWt = 0.0;
long EffectCt, CurCt;
EffectJoeList *CurList;
EcosystemEffect *CurEffect;
MaterialList *FirstMaterial = NULL;
bool Done = false, Success = true;
short TopPriority = -10000;
TwinMaterials MatTwin;

// build comprehensive list of the effects to evaluate

for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
	{
	if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		EffectsLib::EffectChain[EffectCt++] = CurList;
	} // for

if (EffectsLib::EffectChain[0])
	{
	EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_ECOSYSTEM);

	for (CurCt = 0; CurCt < EffectCt; ++CurCt)
		{
		if (CurList = EffectsLib::EffectChain[CurCt])
			{
			CurEffect = (EcosystemEffect *)CurList->MyEffect;
			if (CurEffect->Priority < TopPriority)
				{
				if (AccumulatedWt >= 1.0)
					{
					if (AccumulatedWt > 1.0 && FirstMaterial)
						{
						// normalize the ecosystem coverages just added so they add to 100%
						for (; FirstMaterial; FirstMaterial = FirstMaterial->NextMaterial)
							{
							FirstMaterial->MatCoverage /= (float)AccumulatedWt;
							} // for
						AccumulatedWt = 1.0;
						} // if
					SumOfAllCoverages += AccumulatedWt;
					break;
					} // if
				SumOfAllCoverages += AccumulatedWt;
				AccumulatedWt = 0.0;
				FirstMaterial = NULL;
				} // if
			if (CurList->VSData)
				{
				for (TfxDetail *CurVS = CurList->VSData; CurVS; CurVS = CurVS->Next)
					{
					} // for
				} // if
			else
				{
				if (CurNode->NodeData->Slope <= CurEffect->MaxSlope && CurNode->NodeData->Slope >= CurEffect->MinSlope &&
					CurNode->NodeData->RelEl <= CurEffect->MaxRelEl && CurNode->NodeData->RelEl >= CurEffect->MinRelEl)
					{
					if (CurEffect->UseGradient)
						{
						DistanceToVector = fabs(CurList->MyJoe->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, 
							Rend->EarthLatScaleMeters, true, Rend->ElevDatum, Rend->Exageration, dX, dY, dZ));
						ProfWt = CurEffect->ADProf.GetValue(0, DistanceToVector);
						} // if
					else
						ProfWt = 1.0;
						
					if (CurEffect->EcoMat.GetRenderMaterial(&MatTwin, CurEffect->GetRenderMatGradientPos(Rend, CurList->MyJoe, CurNode, NULL)))
						{
						TopPriority = CurEffect->Priority;
						Done = true;
						if (MatTwin.Mat[0])
							{
							CurMatWt = ProfWt * MatTwin.Covg[0];
							if (CurEffect->PlowSnow)
								PlowedSnow += CurMatWt;
							AccumulatedWt += CurMatWt * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! FirstMaterial)
								{
								if (! (FirstMaterial = CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurList->MyJoe, (float)(CurMatWt))))
									{
									Success = false;
									break;
									}
								} // if
							else
								{
								if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurList->MyJoe, (float)(CurMatWt)))
									{
									Success = false;
									break;
									}
								} // else
							} // if
						if (MatTwin.Mat[1])
							{
							CurMatWt = ProfWt * MatTwin.Covg[1];
							if (CurEffect->PlowSnow)
								PlowedSnow += CurMatWt;
							AccumulatedWt += CurMatWt * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, CurList->MyJoe, (float)(CurMatWt)))
								{
								Success = false;
								break;
								}
							} // if
						} // if
					} // if
				} // else
			} // if
		} // for
	if (AccumulatedWt > 1.0 && FirstMaterial)
		{
		// normalize the ecosystem coverages just added so they add to 100%
		for (; FirstMaterial; FirstMaterial = FirstMaterial->NextMaterial)
			{
			FirstMaterial->MatCoverage /= (float)AccumulatedWt;
			} // for
		AccumulatedWt = 1.0;
		} // if
	SumOfAllCoverages += AccumulatedWt;
	} // if
	
return (Success);

} // EcosystemEffectBase::Eval

/*===========================================================================*/
/*===========================================================================*/

EcosystemEffect::EcosystemEffect(char NewMatType)
: GeneralEffect(NULL), EcoMat(this, 1, NewMatType)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM;
SetDefaults();

} // EcosystemEffect::EcosystemEffect

/*===========================================================================*/

EcosystemEffect::EcosystemEffect(RasterAnimHost *RAHost, char NewMatType)
: GeneralEffect(RAHost), EcoMat(this, 1, NewMatType)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM;
SetDefaults();

} // EcosystemEffect::EcosystemEffect

/*===========================================================================*/

EcosystemEffect::EcosystemEffect(RasterAnimHost *RAHost, EffectsLib *Library, EcosystemEffect *Proto, char NewMatType)
: GeneralEffect(RAHost), EcoMat(this, 1, NewMatType)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM;
if (Library)
	{
	Prev = Library->LastEcosystem;
	if (Library->LastEcosystem)
		{
		Library->LastEcosystem->Next = this;
		Library->LastEcosystem = this;
		} // if
	else
		{
		Library->Ecosystem = Library->LastEcosystem = this;
		} // else
	} // if
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "Ecosystem");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // EcosystemEffect::EcosystemEffect

/*===========================================================================*/

EcosystemEffect::~EcosystemEffect(void)
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->ECG && GlobalApp->GUIWins->ECG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->ECG;
		GlobalApp->GUIWins->ECG = NULL;
		} // if
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // EcosystemEffect::~EcosystemEffect

/*===========================================================================*/

void EcosystemEffect::SetDefaults(void)
{
long Ct;
double EffectDefault[WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR] = {100000.0, 0.0, 0.0, 0.0, 10000.0, -10000.0, 90.0, 0.0, 0.0};
double RangeDefaults[WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0, FLT_MAX, -FLT_MAX, 1.0, 
	360, -360.0, 1.0,
	FLT_MAX, -FLT_MAX, 1.0,
	FLT_MAX, -FLT_MAX, 1.0,
	FLT_MAX, -FLT_MAX, 1.0,
	FLT_MAX, 0.0, 1.0,
	FLT_MAX, 0.0, 1.0,
	1.0, 0.0, .01};

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for
CmapMatch = CmapMatchRange = 0;
Transparent = PlowSnow = 0;
for (Ct = 0; Ct < 6; Ct ++)
	{
	MatchColor[Ct] = 0;
	} // for
ADProf.SetDefaults(this, (char)GetNumAnimParams());
MaxSlope = MinSlope = MaxRelEl = MinRelEl = 0.0;

AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER].SetMultiplier(100.0);

} // EcosystemEffect::SetDefaults

/*===========================================================================*/

void EcosystemEffect::Copy(EcosystemEffect *CopyTo, EcosystemEffect *CopyFrom)
{
long Ct;

EcoMat.Copy(&CopyTo->EcoMat, &CopyFrom->EcoMat);

CopyTo->CmapMatch = CopyFrom->CmapMatch;
CopyTo->CmapMatchRange = CopyFrom->CmapMatchRange;
for (Ct = 0; Ct < 6; Ct ++)
	{
	CopyTo->MatchColor[Ct] = CopyFrom->MatchColor[Ct];
	} // for
CopyTo->Transparent = CopyFrom->Transparent;
CopyTo->PlowSnow = CopyFrom->PlowSnow;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // EcosystemEffect::Copy

/*===========================================================================*/

unsigned long EcosystemEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
char TexRootNumber = -1;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_HIRESEDGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HiResEdge, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_USEGRADIENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseGradient, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_ELEVLINE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_SKEW:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_SKEWAZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_RELEL:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MAXRELEL:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MINRELEL:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MAXSLOPE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MINSLOPE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_CMAPMATCH:
						{
						BytesRead = ReadBlock(ffile, (char *)&CmapMatch, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_CMAPMATCHRANGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CmapMatchRange, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHREDLO:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[0], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHGRNLO:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[1], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHBLULO:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[2], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHREDHI:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[3], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHGRNHI:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[4], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_MATCHBLUHI:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchColor[5], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_TRANSPARENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&Transparent, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_PLOWSNOW:
						{
						BytesRead = ReadBlock(ffile, (char *)&PlowSnow, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_ECOMATERIAL:
						{
						BytesRead = EcoMat.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_TEXMATDRIVER:
						{
						if (TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ECOSYSTEM_PROFILE:
						{
						BytesRead = ADProf.Load(ffile, Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_QUERY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Search = (SearchQuery *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName);
							} // if
						break;
						}
					#endif // WCS_BUILD_VNS
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_ECOSYSTEM_THEMEMATDRIVER:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP

					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // EcosystemEffect::Load

/*===========================================================================*/

unsigned long EcosystemEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR] = {WCS_EFFECTS_ECOSYSTEM_ELEVLINE,
																	WCS_EFFECTS_ECOSYSTEM_SKEW,
																	WCS_EFFECTS_ECOSYSTEM_SKEWAZ,
																	WCS_EFFECTS_ECOSYSTEM_RELEL,
																	WCS_EFFECTS_ECOSYSTEM_MAXRELEL,
																	WCS_EFFECTS_ECOSYSTEM_MINRELEL,
																	WCS_EFFECTS_ECOSYSTEM_MAXSLOPE,
																	WCS_EFFECTS_ECOSYSTEM_MINSLOPE,
																	WCS_EFFECTS_ECOSYSTEM_MATDRIVER};
unsigned long TextureItemTag[WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES] = {WCS_EFFECTS_ECOSYSTEM_TEXMATDRIVER};
unsigned long ThemeItemTag[WCS_EFFECTS_ECOSYSTEM_NUMTHEMES] = {WCS_EFFECTS_ECOSYSTEM_THEMEMATDRIVER};


if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_HIRESEDGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&HiResEdge)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_USEGRADIENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseGradient)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_CMAPMATCH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&CmapMatch)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_CMAPMATCHRANGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&CmapMatchRange)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHREDLO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHGRNLO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHBLULO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHREDHI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[3])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHGRNHI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[4])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_MATCHBLUHI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&MatchColor[5])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_TRANSPARENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Transparent)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ECOSYSTEM_PLOWSNOW, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&PlowSnow)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_EFFECTS_ECOSYSTEM_ECOMATERIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = EcoMat.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if anim material gradient saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block
					else
						goto WriteError;
					} // if anim param saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_EFFECTS_ECOSYSTEM_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ADProf.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if profile gradient saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

#ifdef WCS_THEMATIC_MAP
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetTheme(Ct))
		{
		if ((BytesWritten = PrepWriteBlock(ffile, ThemeItemTag[Ct], WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(GetTheme(Ct)->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)GetTheme(Ct)->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // for
#endif // WCS_THEMATIC_MAP

#ifdef WCS_BUILD_VNS
if (Search)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_QUERY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Search->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Search->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EcosystemEffect::Save

/*===========================================================================*/

void EcosystemEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->ECG)
	{
	delete GlobalApp->GUIWins->ECG;
	}
GlobalApp->GUIWins->ECG = new EcosystemEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppImages, this);
if(GlobalApp->GUIWins->ECG)
	{
	GlobalApp->GUIWins->ECG->Open(GlobalApp->MainProj);
	}

} // EcosystemEffect::Edit

/*===========================================================================*/

short EcosystemEffect::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for
if (EcoMat.AnimateEcoShadows())
	return (1);
if (TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER] && TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER]->IsAnimated())
	return (1);

return (0);

} // EcosystemEffect::AnimateShadows

/*===========================================================================*/

char *EcosystemEffectCritterNames[WCS_EFFECTS_ECOSYSTEM_NUMANIMPAR] = {"Elevation Line (m)", "Elevation Line Skew (m)", "Line Skew Azimuth (deg)", "Relative Elevation Effect",
	"Maximum Relative Elevation", "Minimum Relative Elevation", "Maximum Slope (deg)", "Minimum Slope (deg)", "Material Driver (%)"};
char *EcosystemEffectTextureNames[WCS_EFFECTS_ECOSYSTEM_NUMTEXTURES] = {"Material Driver (%)"};
char *EcosystemEffectThemeNames[WCS_EFFECTS_ECOSYSTEM_NUMTHEMES] = {"Material Driver (%)"};

char *EcosystemEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (EcosystemEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (EcosystemEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &EcoMat)
	return ("Ground Overlay and Foliage");
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // EcosystemEffect::GetCritterName

/*===========================================================================*/

char *EcosystemEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Ecosystem Texture! Remove anyway?");

} // EcosystemEffect::OKRemoveRaster

/*===========================================================================*/

int EcosystemEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{

return (EcoMat.FindnRemove3DObjects(RemoveMe));

} // EcosystemEffect::FindnRemove3DObjects

/*===========================================================================*/

int EcosystemEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (EcoMat.SetToTime(Time))
	Found = 1;

return (Found);

} // EcosystemEffect::SetToTime

/*===========================================================================*/

double EcosystemEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // EcosystemEffect::GetMaxProfileDistance

/*===========================================================================*/

int EcosystemEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for
if (! EcoMat.InitToRender(Opt, Buffers))
	return (0);

return (1);

} // EcosystemEffect::InitToRender

/*===========================================================================*/

int EcosystemEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
int Success = 1;

if (Rend->ExagerateElevLines)
	Line = (AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	Line = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue;
// modify skew value so renderer doesn't have to repeat this calculation for every polygon
Skew = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEW].CurValue / 45.0;
SkewAzimuth = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_SKEWAZ].CurValue * PiOver180;
MaxSlope = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].CurValue;
MinSlope = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE].CurValue;
MaxRelEl = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue;
MinRelEl = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue;

if (! EcoMat.InitFrameToRender(Lib, Rend))
	return (0);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // EcosystemEffect::InitFrameToRender

/*===========================================================================*/

long EcosystemEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += EcoMat.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // EcosystemEffect::InitImageIDs

/*===========================================================================*/

int EcosystemEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{

if (! EcoMat.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

return (GeneralEffect::BuildFileComponentsList(Queries, Themes, Coords));

} // EcosystemEffect::BuildFileComponentsList

/*===========================================================================*/

long EcosystemEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (EcoMat.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // EcosystemEffect::GetKeyFrameRange

/*===========================================================================*/

char EcosystemEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT
	|| DropType == WCS_EFFECTSSUBCLASS_MATERIAL
	|| DropType == WCS_RAHOST_OBJTYPE_ECOTYPE
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropType == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	return (1);
#ifdef WCS_BUILD_VNS
if (DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY && RAParent == NULL)
	return (1);
#endif // WCS_BUILD_VNS

return (0);

} // EcosystemEffect::GetRAHostDropOK

/*===========================================================================*/

int EcosystemEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (EcosystemEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (EcosystemEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT 
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
	{
	Success = EcoMat.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	Success = ADProf.ProcessRAHostDragDrop(DropSource);
	} // else if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Success = SetQuery((SearchQuery *)DropSource->DropSource);
		} // if
	} // else if
#endif // WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (((Joe *)DropSource->DropSource)->AddEffect(this, -1))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // EcosystemEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *EcosystemEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? EcosystemEffectTextureNames[TexNumber]: (char*)"");

} // EcosystemEffect::GetTextureName

/*===========================================================================*/

RootTexture *EcosystemEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // EcosystemEffect::NewRootTexture

/*===========================================================================*/

char *EcosystemEffect::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? EcosystemEffectThemeNames[ThemeNum]: (char*)"");

} // EcosystemEffect::GetThemeName

/*===========================================================================*/

int EcosystemEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (EcoMat.GetRAHostAnimated())
	return (1);

return (0);

} // EcosystemEffect::GetRAHostAnimated

/*===========================================================================*/

bool EcosystemEffect::AnimateMaterials(void)
{

if (GeneralEffect::AnimateMaterials())
	return (true);
if (EcoMat.AnimateMaterials())
	return (true);

return (false);

} // EcosystemEffect::AnimateMaterials

/*===========================================================================*/

unsigned long EcosystemEffect::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ECOSYSTEM | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // EcosystemEffect::GetRAFlags

/*===========================================================================*/

RasterAnimHost *EcosystemEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&EcoMat);
if (Current == &EcoMat)
	return (&ADProf);
if (Current == &ADProf)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (Found && GetTheme(Ct) && GetThemeUnique(Ct))
		return (GetTheme(Ct));
	if (Current == GetTheme(Ct) && GetThemeUnique(Ct))
		Found = 1;
	} // for
#ifdef WCS_BUILD_VNS
if (Found && Search)
	return (Search);
if (Search && Current == Search)
	Found = 1;
#endif // WCS_BUILD_VNS
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // EcosystemEffect::GetRAHostChild

/*===========================================================================*/

int EcosystemEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetThemeAddr(Ct))
			{
			ThemeAffil = (ThematicMap **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // EcosystemEffect::GetAffiliates

/*===========================================================================*/

void EcosystemEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double Scale;
GraphNode *CurNode;

if (OldBounds->HighElev > OldBounds->LowElev)
	Scale = (CurBounds->HighElev - CurBounds->LowElev) / (OldBounds->HighElev - OldBounds->LowElev);
else
	Scale = 1.0;

AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].SetValue(
	(AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue - OldBounds->LowElev) * Scale + CurBounds->LowElev);
if (CurNode = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].GetFirstNode(0))
	{
	CurNode->SetValue((CurNode->GetValue() - OldBounds->LowElev) * Scale + CurBounds->LowElev);
	while (CurNode = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].GetNextNode(0, CurNode))
		{
		CurNode->SetValue((CurNode->GetValue() - OldBounds->LowElev) * Scale + CurBounds->LowElev);
		} // while
	} // if

} // EcosystemEffect::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int EcosystemEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
EcosystemEffect *CurrentEco = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if Matl3D
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if Wave
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentObj->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Search", 8))
						{
						if (CurrentQuery = new SearchQuery(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentQuery->Load(ffile, Size, ByteFlip);
							}
						} // if search query
					else if (! strnicmp(ReadBuf, "ThemeMap", 8))
						{
						if (CurrentTheme = new ThematicMap(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip);
							}
						} // if thematic map
					else if (! strnicmp(ReadBuf, "Ecosys", 8))
						{
						if (CurrentEco = new EcosystemEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM))
							{
							if ((BytesRead = CurrentEco->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if eco
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentEco)
	{
	if (EffectsLib::LoadQueries && OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Ecosystem", "Do you wish the loaded Ecosystem's elevation line\n to be scaled to current DEM elevations?"))
			CurrentEco->ScaleToDEMBounds(&OldBounds, &CurBounds);
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentEco);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // EcosystemEffect::LoadObject

/*===========================================================================*/

int EcosystemEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords)
	&& BuildFileComponentsList(&Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Queries;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Search");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SearchQuery *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if SearchQuery saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	#ifdef WCS_THEMATIC_MAP
	CurEffect = Themes;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "ThemeMap");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ThematicMap *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if ThemeMap saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_THEMATIC_MAP

	CurEffect = Waves;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Wave");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((WaveEffect *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if wave saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Material3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Matl3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((MaterialEffect *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if material saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Object3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Object3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((Object3DEffect *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if 3D Object saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	while (Queries)
		{
		CurEffect = Queries;
		Queries = Queries->Next;
		delete CurEffect;
		} // while
	while (Themes)
		{
		CurEffect = Themes;
		Themes = Themes->Next;
		delete CurEffect;
		} // while
	while (Material3Ds)
		{
		CurEffect = Material3Ds;
		Material3Ds = Material3Ds->Next;
		delete CurEffect;
		} // while
	while (Object3Ds)
		{
		CurEffect = Object3Ds;
		Object3Ds = Object3Ds->Next;
		delete CurEffect;
		} // while
	while (Waves)
		{
		CurEffect = Waves;
		Waves = Waves->Next;
		delete CurEffect;
		} // while
	} // if

// Ecosystem
strcpy(StrBuf, "Ecosys");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Ecosystem saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // EcosystemEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::EcosystemEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
EcosystemEffect *Current;
int FloatingLoaded = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_RESOLUTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_RANDOMIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.Randomize, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_ECOSYSTEM_DISSOLVEBYIMAGESIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.DissolveByImageSize, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_ECOSYSTEM_DISSOLVEREFHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcosystemBase.DissolveRefImageHt, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new EcosystemEffect(NULL, this, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (EcosystemEffect *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

if (! FloatingLoaded)
	{
	// if user has previously modified, turn floating off
	if (fabs(EcosystemBase.Resolution - 90.0) > .0001)
		EcosystemBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::EcosystemEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::EcosystemEffect_Save(FILE *ffile)
{
EcosystemEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&EcosystemBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RANDOMIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EcosystemBase.Randomize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EcosystemBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EcosystemBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_ECOSYSTEM_DISSOLVEBYIMAGESIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EcosystemBase.DissolveByImageSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_ECOSYSTEM_DISSOLVEREFHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EcosystemBase.DissolveRefImageHt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Ecosystem;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if lake effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (EcosystemEffect *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // EffectsLib::EcosystemEffect_Save()

/*===========================================================================*/
