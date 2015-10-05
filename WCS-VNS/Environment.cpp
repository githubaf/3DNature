// Environment.cpp
// For managing Cmap Effects
// Built from scratch on 04/24/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "requester.h"
#include "EnvironmentEditGUI.h"
#include "AppMem.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "FeatureConfig.h"

EnvironmentEffectBase::EnvironmentEffectBase(void)
{

SetDefaults();

} // EnvironmentEffectBase::EnvironmentEffectBase

/*===========================================================================*/

void EnvironmentEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;
GeoRast = NULL;

} // EnvironmentEffectBase::SetDefaults

/*===========================================================================*/

GeoRaster *EnvironmentEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE, TotalCells, SizeMult;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
EnvironmentEffect *MyEffect;
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
Resolution = 100.0;
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
			if (MyEffect = (EnvironmentEffect *)RJL->Effect)
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
					//if (MyEffect->HiResEdge)
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
							if ((DrawRast = DrawRast->PolyRasterCopyFloodByte(MaxMem, UsedMem, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, WCS_MOSTEFFECTS_OVERLAP_ENABLED, FALSE, MinX, MinY)) == NULL)
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
						if ((DrawRast = DrawRast->PolyRasterFillByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, WCS_MOSTEFFECTS_OVERLAP_ENABLED, FALSE)) == NULL)
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
		sprintf(Name, "d:\\Frames\\EnvironmentRastTest%1d.iff", Count);
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

} // EnvironmentEffectBase::Init

/*===========================================================================*/

void EnvironmentEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // EnvironmentEffectBase::Destroy

/*===========================================================================*/

short EnvironmentEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // EnvironmentEffectBase::AreThereEdges

/*===========================================================================*/

short EnvironmentEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // EnvironmentEffectBase::AreThereGradients

/*===========================================================================*/

void EnvironmentEffectBase::SetFloating(int NewFloating, Project *CurProj)
{
#ifndef WCS_VECPOLY_EFFECTS
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // EnvironmentEffectBase::SetFloating

/*===========================================================================*/

void EnvironmentEffectBase::SetFloating(int NewFloating, float NewResolution)
{
#ifndef WCS_VECPOLY_EFFECTS
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ENVIRONMENT, WCS_EFFECTSSUBCLASS_ENVIRONMENT, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // EnvironmentEffectBase::SetFloating

/*===========================================================================*/

// note that align to vector textured ecosystems only work if high-res edges are turned on
// since there is no Joe list search otherwise
// This version becomes obsolete with VNS 3 unless it is kept for the ability to render the VNS 2 way.
#ifdef WCS_VECPOLY_EFFECTS
bool EnvironmentEffectBase::Eval(RenderData *Rend, VectorNode *CurNode)
#else // WCS_VECPOLY_EFFECTS
int EnvironmentEffectBase::Eval(PolygonData *Poly)
#endif // WCS_VECPOLY_EFFECTS
{
double HalfRes, Lat, Lon, ActualSum;
unsigned long LonSeed, LatSeed;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
EnvironmentEffect *Effect;
short Priority = -1000, ValuesRead = 0, Done = 0;

HalfRes = Resolution * 0.5;
#ifdef WCS_VECPOLY_EFFECTS
Lat = CurNode->Lat;
Lon = CurNode->Lon;
LonSeed = (ULONG)((CurNode->Lon - floor(CurNode->Lon)) * ULONG_MAX);
LatSeed = (ULONG)((CurNode->Lat - floor(CurNode->Lat)) * ULONG_MAX);
#else // WCS_VECPOLY_EFFECTS
Lat = Poly->Lat;
Lon = Poly->Lon;
LonSeed = Poly->LonSeed;
LatSeed = Poly->LatSeed;
#endif // WCS_VECPOLY_EFFECTS
Rand.Seed64BitShift(LatSeed + WCS_SEEDOFFSET_ENVIRONMENT, LonSeed + WCS_SEEDOFFSET_ENVIRONMENT);
TempEnv = NULL;

// get value from first map
if (GradientsExist)
	{
	if (EdgesExist)
		{
		Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
		if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // search maps for values to do edge test on
			if (WCS_MOSTEFFECTS_OVERLAP_ENABLED)
				{ // find all containing objects until priority changes
				SumWt = ActualSum = 0.0;
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (EnvironmentEffect *)Rast->LUT[0];
							if (Effect->Priority >= Priority)
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
								} // if priority
							else
								break;
							} // if gradfill
						else
							{
							Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->Priority >= Priority)
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
								} // if priority
							else
								break;
							} // else not gradfill
						} // if a value in map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				if (Done)
					{
					#ifdef WCS_VECPOLY_EFFECTS
					CurNode->NodeData->NodeEnvironment = TempEnv;
					#else // WCS_VECPOLY_EFFECTS
					Poly->Env = TempEnv;
					#endif // WCS_VECPOLY_EFFECTS
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
							Effect = (EnvironmentEffect *)Rast->LUT[0];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
								SumWt = fabs(Wt);
								Done = 1;
								} // if contained
							} // if gradfill
						else
							{
							Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
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
									{
									#ifdef WCS_VECPOLY_EFFECTS
									CurNode->NodeData->NodeEnvironment = Effect;
									#else // WCS_VECPOLY_EFFECTS
									Poly->Env = Effect;
									#endif // WCS_VECPOLY_EFFECTS
									} // if
								else
									Done = 0;
								} // if
							else
								{
								#ifdef WCS_VECPOLY_EFFECTS
								CurNode->NodeData->NodeEnvironment = Effect;
								#else // WCS_VECPOLY_EFFECTS
								Poly->Env = Effect;
								#endif // WCS_VECPOLY_EFFECTS
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
		if (WCS_MOSTEFFECTS_OVERLAP_ENABLED)
			{ // check maps until lower priority is encountered
			SumWt = ActualSum = 0.0;
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					if (Rast->GradFill)
						{
						Effect = (EnvironmentEffect *)Rast->LUT[0];
						if (Effect->Priority >= Priority)
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
							} // if priority
						else
							break;
						} // if
					else
						{
						Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Effect->Priority >= Priority)
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
						else
							break;
						} // if
					} // if a value in map
				Rast = (GeoRaster *)Rast->Next;
				} // while
			if (Done)
				{
				#ifdef WCS_VECPOLY_EFFECTS
				CurNode->NodeData->NodeEnvironment = TempEnv;
				#else // WCS_VECPOLY_EFFECTS
				Poly->Env = TempEnv;
				#endif // WCS_VECPOLY_EFFECTS
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
						Effect = (EnvironmentEffect *)Rast->LUT[0];
						Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
						SumWt = fabs(Wt);
						} // if
					else
						{
						Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						Wt = 1.0;
						SumWt = 1.0;
						} // if
					if (SumWt < 1.0)
						{
						if (Rand.GenPRN() <= SumWt)
							{
							#ifdef WCS_VECPOLY_EFFECTS
							CurNode->NodeData->NodeEnvironment = Effect;
							#else // WCS_VECPOLY_EFFECTS
							Poly->Env = Effect;
							#endif // WCS_VECPOLY_EFFECTS
							Done = 1;
							} // if
						} // if
					else
						{
						#ifdef WCS_VECPOLY_EFFECTS
						CurNode->NodeData->NodeEnvironment = Effect;
						#else // WCS_VECPOLY_EFFECTS
						Poly->Env = Effect;
						#endif // WCS_VECPOLY_EFFECTS
						Done = 1;
						} // else
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
		if (WCS_MOSTEFFECTS_OVERLAP_ENABLED)
			{ // find all containing objects until priority changes
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->Priority >= Priority)
						{
						if (Effect->RenderJoes->SimpleContained(Lat, Lon))
							{
							Priority = Effect->Priority;
							ValuesRead ++;
							Compare(Effect, ValuesRead);
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
				#ifdef WCS_VECPOLY_EFFECTS
				CurNode->NodeData->NodeEnvironment = TempEnv;
				#else // WCS_VECPOLY_EFFECTS
				Poly->Env = TempEnv;
				#endif // WCS_VECPOLY_EFFECTS
				} // if
			} // if
		else
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->RenderJoes->SimpleContained(Lat, Lon))
						{
						#ifdef WCS_VECPOLY_EFFECTS
						CurNode->NodeData->NodeEnvironment = Effect;
						#else // WCS_VECPOLY_EFFECTS
						Poly->Env = Effect;
						#endif // WCS_VECPOLY_EFFECTS
						Done = 1;
						break;
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
	if (WCS_MOSTEFFECTS_OVERLAP_ENABLED)
		{ // check maps until lower priority is encountered
		while (Rast)
			{
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if (Effect->Priority >= Priority)
					{
					Priority = Effect->Priority;
					ValuesRead ++;
					Compare(Effect, ValuesRead);
					Done = 1;
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
			#ifdef WCS_VECPOLY_EFFECTS
			CurNode->NodeData->NodeEnvironment = TempEnv;
			#else // WCS_VECPOLY_EFFECTS
			Poly->Env = TempEnv;
			#endif // WCS_VECPOLY_EFFECTS
			} // if
		} // if overlap
	else
		{ // everything in one map - nice 'n easy
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (EnvironmentEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			#ifdef WCS_VECPOLY_EFFECTS
			CurNode->NodeData->NodeEnvironment = Effect;
			#else // WCS_VECPOLY_EFFECTS
			Poly->Env = Effect;
			#endif // WCS_VECPOLY_EFFECTS
			Done = 1;
			} // if a value in edge map
		} // else
	} // if

return (Done > 0);

} // EnvironmentEffectBase::Eval

/*===========================================================================*/

short EnvironmentEffectBase::Compare(EnvironmentEffect *Effect, short ValuesRead)
{

if (ValuesRead <= 1 || Rand.GenPRN() <= (1.0 / ValuesRead))
	{
	TempEnv = Effect;
	return (1);
	} // if

return (0);

} // EnvironmentEffectBase::Compare

/*===========================================================================*/

short EnvironmentEffectBase::CompareWeighted(EnvironmentEffect *Effect)
{

if (Wt >= SumWt || (SumWt > 0.0 && Rand.GenPRN() <= fabs(Wt) / SumWt))
	{
	TempEnv = Effect;
	return (1);
	} // if

return (0);

} // EnvironmentEffectBase::CompareWeighted

/*===========================================================================*/
/*===========================================================================*/

EnvironmentEffect::EnvironmentEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT;
SetDefaults();

} // EnvironmentEffect::EnvironmentEffect

/*===========================================================================*/

EnvironmentEffect::EnvironmentEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT;
SetDefaults();

} // EnvironmentEffect::EnvironmentEffect

/*===========================================================================*/

EnvironmentEffect::EnvironmentEffect(RasterAnimHost *RAHost, EffectsLib *Library, EnvironmentEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT;
Prev = Library->LastEnvironment;
if (Library->LastEnvironment)
	{
	Library->LastEnvironment->Next = this;
	Library->LastEnvironment = this;
	} // if
else
	{
	Library->Environment = Library->LastEnvironment = this;
	} // else
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
	strcpy(NameBase, "Environment");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // EnvironmentEffect::EnvironmentEffect

/*===========================================================================*/

EnvironmentEffect::~EnvironmentEffect()
{
EffectList *NextEco;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->ENG && GlobalApp->GUIWins->ENG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->ENG;
		GlobalApp->GUIWins->ENG = NULL;
		} // if
	} // if

while (Ecosystems)
	{
	NextEco = Ecosystems;
	Ecosystems = Ecosystems->Next;
	delete NextEco;
	} // while

} // EnvironmentEffect::~EnvironmentEffect

/*===========================================================================*/

void EnvironmentEffect::SetDefaults(void)
{
long Ct;
double EffectDefault[WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR] = {1.0, 100.0, 0.0};
double RangeDefaults[WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR][3] = {10.0, 0.0, .01, FLT_MAX, -FLT_MAX, 10.0,	90.0, -90.0, 1.0};

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

GlobalGradientsEnabled = 0;
FoliageBlending = 1.0;
FoliageMinSize = 50;
MinPixelSize = .5;
Ecosystems = NULL;
ADProf.SetDefaults(this, (char)GetNumAnimParams());

AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALECOGRAD].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].SetMultiplier(100.0);

} // EnvironmentEffect::SetDefaults

/*===========================================================================*/

void EnvironmentEffect::Copy(EnvironmentEffect *CopyTo, EnvironmentEffect *CopyFrom)
{
long Result = -1;
EffectList *NextEco, **ToEco;
NotifyTag Changes[2];

while (CopyTo->Ecosystems)
	{
	NextEco = CopyTo->Ecosystems;
	CopyTo->Ecosystems = CopyTo->Ecosystems->Next;
	delete NextEco;
	} // if
NextEco = CopyFrom->Ecosystems;
ToEco = &CopyTo->Ecosystems;
while (NextEco)
	{
	if (NextEco->Me)
		{
		if (*ToEco = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
				{
				(*ToEco)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEco->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextEco->Me->EffectType, NextEco->Me->Name))
					{
					Result = UserMessageCustom("Copy Environment", "How do you wish to resolve Ecosystem name collisions?\n\nLink to existing Ecosystems, replace existing Ecosystems, or create new Ecosystems?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEco->Me->EffectType, NULL, NextEco->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEco->Me);
					} // if link to existing
				else if ((*ToEco)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEco->Me->EffectType, NextEco->Me->Name))
					{
					((EcosystemEffect *)(*ToEco)->Me)->Copy((EcosystemEffect *)(*ToEco)->Me, (EcosystemEffect *)NextEco->Me);
					Changes[0] = MAKE_ID((*ToEco)->Me->GetNotifyClass(), (*ToEco)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToEco)->Me);
					} // else if found and overwrite
				else
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEco->Me->EffectType, NULL, NextEco->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToEco)->Me)
				ToEco = &(*ToEco)->Next;
			else
				{
				delete *ToEco;
				*ToEco = NULL;
				} // if
			} // if
		} // if
	NextEco = NextEco->Next;
	} // while

CopyTo->GlobalGradientsEnabled = CopyFrom->GlobalGradientsEnabled;
CopyTo->FoliageBlending = CopyFrom->FoliageBlending;
CopyTo->FoliageMinSize = CopyFrom->FoliageMinSize;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // EnvironmentEffect::Copy

/*===========================================================================*/

ULONG EnvironmentEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
short DummyShort;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
EffectList **CurEco = &Ecosystems;

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
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
					} /* switch */

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
					case WCS_EFFECTS_ENVIRONMENT_GLOBALGRADENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&GlobalGradientsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_FOLIAGEBLENDINGINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&DummyShort, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FoliageBlending = DummyShort;
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_FOLIAGEBLENDING:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageBlending, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_FOLIAGEMINSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageMinSize, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_FOLIAGEHTFACT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_GLOBALECOGRAD:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALECOGRAD].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_GLOBALREFLAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_ECONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurEco = new EffectList())
								{
								if ((*CurEco)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, MatchName))
									CurEco = &(*CurEco)->Next;
								else
									{
									delete *CurEco;
									*CurEco = NULL;
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_ENVIRONMENT_PROFILE:
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
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // EnvironmentEffect::Load

/*===========================================================================*/

unsigned long int EnvironmentEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR] = {WCS_EFFECTS_ENVIRONMENT_FOLIAGEHTFACT, 
																WCS_EFFECTS_ENVIRONMENT_GLOBALECOGRAD,
																WCS_EFFECTS_ENVIRONMENT_GLOBALREFLAT};
EffectList *CurEco;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENVIRONMENT_GLOBALGRADENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GlobalGradientsEnabled)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENVIRONMENT_FOLIAGEBLENDING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&FoliageBlending)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENVIRONMENT_FOLIAGEMINSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&FoliageMinSize)) == NULL)
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
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if anim param saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // for

CurEco = Ecosystems;
while (CurEco)
	{
	if (CurEco->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENVIRONMENT_ECONAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEco->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEco->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEco = CurEco->Next;
	} // while

ItemTag = WCS_EFFECTS_ENVIRONMENT_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

} // EnvironmentEffect::Save

/*===========================================================================*/

void EnvironmentEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->ENG)
	{
	delete GlobalApp->GUIWins->ENG;
	}
GlobalApp->GUIWins->ENG = new EnvironmentEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->ENG)
	{
	GlobalApp->GUIWins->ENG->Open(GlobalApp->MainProj);
	}

} // EnvironmentEffect::Edit

/*===========================================================================*/

char *EnvironmentEffectCritterNames[WCS_EFFECTS_ENVIRONMENT_NUMANIMPAR] = {"Foliage Height Factor (%)", "Global Ecosystem Gradient (m/deg)",
															"Gradient Reference Latitude (deg)"};

char *EnvironmentEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (EnvironmentEffectCritterNames[Ct]);
	} // for
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // EnvironmentEffect::GetCritterName

/*===========================================================================*/

short EnvironmentEffect::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		{
		return (1);
		} // if
	} // for

return (0);

} // EnvironmentEffect::AnimateShadows

/*===========================================================================*/

EffectList *EnvironmentEffect::AddEcosystem(GeneralEffect *AddMe)
{
EffectList **CurEco = &Ecosystems;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurEco)
		{
		if ((*CurEco)->Me == AddMe)
			return (NULL);
		CurEco = &(*CurEco)->Next;
		} // while
	if (*CurEco = new EffectList())
		{
		(*CurEco)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurEco);
	} // if

return (NULL);

} // EnvironmentEffect::AddEcosystem

/*===========================================================================*/

long EnvironmentEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurEco = Ecosystems;

while (CurEco)
	{
	if (CurEco->Me)
		NumImages += CurEco->Me->InitImageIDs(ImageID);
	CurEco = CurEco->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // EnvironmentEffect::InitImageIDs

/*===========================================================================*/

int EnvironmentEffect::BuildFileComponentsList(EffectList **Ecosys, EffectList **Material3Ds, EffectList **Object3Ds, 
	EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr, *CurEco = Ecosystems;

while (CurEco)
	{
	if (CurEco->Me)
		{
		ListPtr = Ecosys;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurEco->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurEco->Me;
			else
				return (0);
			} // if
		if (! ((EcosystemEffect *)CurEco->Me)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	CurEco = CurEco->Next;
	} // while

return (1);

} // EnvironmentEffect::BuildFileComponentsList

/*===========================================================================*/

int EnvironmentEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurEco = Ecosystems, *PrevEco = NULL;
NotifyTag Changes[2];

while (CurEco)
	{
	if (CurEco->Me == (GeneralEffect *)RemoveMe)
		{
		if (PrevEco)
			PrevEco->Next = CurEco->Next;
		else
			Ecosystems = CurEco->Next;

		delete CurEco;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevEco = CurEco;
	CurEco = CurEco->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // EnvironmentEffect::RemoveRAHost

/*===========================================================================*/

char EnvironmentEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	return (1);

return (0);

} // EnvironmentEffect::GetRAHostDropOK

/*===========================================================================*/

int EnvironmentEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (EnvironmentEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (EnvironmentEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddEcosystem((GeneralEffect *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
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

} // EnvironmentEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long EnvironmentEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_ENVIRONMENT | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // EnvironmentEffect::GetRAFlags

/*===========================================================================*/

RasterAnimHost *EnvironmentEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
EffectList *CurEco;
JoeList *CurJoe = Joes;

if (! Current)
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
CurEco = Ecosystems;
while (CurEco)
	{
	if (Found)
		return (CurEco->Me);
	if (Current == CurEco->Me)
		Found = 1;
	CurEco = CurEco->Next;
	} // while
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

} // EnvironmentEffect::GetRAHostChild

/*===========================================================================*/

int EnvironmentEffect::SortEcosystems(void)
{
int Success = 0, EcoNum, NumEcos = 0, Done;
NotifyTag Changes[2];
EffectList *CurEco;
EcosystemEffect **EcoEffectList;

// count ecosystems
CurEco = Ecosystems;
while (CurEco)
	{
	if (CurEco->Me)
		NumEcos ++;
	CurEco = CurEco->Next;
	} // while

// create array of eco ptrs
if (NumEcos && (EcoEffectList = (EcosystemEffect **)AppMem_Alloc(NumEcos * sizeof (EcosystemEffect *), APPMEM_CLEAR)))
	{
	CurEco = Ecosystems;
	EcoNum = 0;
	while (CurEco)
		{
		if (CurEco->Me)
			{
			EcoEffectList[EcoNum] = (EcosystemEffect *)CurEco->Me;
			EcoNum ++;
			} // if
		CurEco = CurEco->Next;
		} // while

	// sort array of ptrs
	Done = 0;
	while (! Done)
		{
		Done = 1;
		for (EcoNum = 0; EcoNum < NumEcos - 1; EcoNum ++)
			{
			if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue > EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue)
				{
				swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
				Done = 0;
				} // if 
			else if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue == EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue)
				{
				if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue < EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue)
					{
					swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
					Done = 0;
					} // if 
				else if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue == EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue)
					{
					if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue > EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue)
						{
						swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
						Done = 0;
						} // if 
					else if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue == EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].CurValue)
						{
						if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].CurValue > EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].CurValue)
							{
							swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
							Done = 0;
							} // if 
						else if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].CurValue == EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXSLOPE].CurValue)
							{
							if (EcoEffectList[EcoNum]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE].CurValue < EcoEffectList[EcoNum + 1]->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINSLOPE].CurValue)
								{
								swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
								Done = 0;
								} // if 
							} // else if 
						} // else if 
					} // else if 
				} // else if 
			} // for
		} // while

	// remove old effectlist
	while (CurEco = Ecosystems)
		{
		Ecosystems = Ecosystems->Next;
		delete CurEco;
		} // while
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	// add ecosystems
	for (EcoNum = 0; EcoNum < NumEcos; EcoNum ++)
		{
		// this will generate a notify
		AddEcosystem(EcoEffectList[EcoNum]);
		Success = 1;
		} // for

	AppMem_Free(EcoEffectList, NumEcos * sizeof (EcosystemEffect *));
	} // if

return (Success);

} // EnvironmentEffect::SortEcosystems

/*===========================================================================*/

EcosystemEffect *EnvironmentEffect::FindEco(EcosystemEffect *FindMe)
{
EffectList *CurEco = Ecosystems;

while (CurEco)
	{
	if (CurEco->Me && CurEco->Me == FindMe)
		return (FindMe);
	CurEco = CurEco->Next;
	} // while

return (NULL);

} // EnvironmentEffect::FindEco

/*===========================================================================*/

int EnvironmentEffect::GrabAllEcosystems(void)
{
int Success = 0;
EffectList *CurEco;
EcosystemEffect *CurEffect;
NotifyTag Changes[2];

while (CurEco = Ecosystems)
	{
	Ecosystems = Ecosystems->Next;
	delete CurEco;
	} // while
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

if (CurEffect = (EcosystemEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM))
	{
	while (CurEffect)
		{
		AddEcosystem(CurEffect);	// sends its own notify
		Success = 1;
		CurEffect = (EcosystemEffect *)CurEffect->Next;
		} // while
	} // if

return (Success);

} // EnvironmentEffect::GrabAllEcosystems

/*===========================================================================*/

double EnvironmentEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // EnvironmentEffect::GetMaxProfileDistance

/*===========================================================================*/

int EnvironmentEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

MinPixelSize = (double)FoliageMinSize / 100.0;
GlobalGradient = AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALECOGRAD].CurValue;
if (Rend->ExagerateElevLines)
	{
	GlobalGradient *= Rend->Exageration;
	} // if

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // EnvironmentEffect::InitFrameToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int EnvironmentEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
EnvironmentEffect *CurrentEnv = NULL;
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
						} // if material
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if material
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
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
								Success = 1;
							}
						} // if eco
					else if (! strnicmp(ReadBuf, "Environ", 8))
						{
						if (CurrentEnv = new EnvironmentEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentEnv->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Environment
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

if (Success == 1 && CurrentEnv)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM) && UserMessageYN("Load Environment", "Do you wish the loaded Environment's Ecosystem\n elevation lines to be scaled to current DEM elevations?"))
			{
			for (CurrentEco = (EcosystemEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); CurrentEco; CurrentEco = (EcosystemEffect *)CurrentEco->Next)
				{
				CurrentEco->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE) && UserMessageYN("Load Environment", "Do you wish the loaded Environment's Wave positions\n to be scaled to current DEM bounds?"))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentEnv);
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

} // EnvironmentEffect::LoadObject

/*===========================================================================*/

int EnvironmentEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *EcoList = NULL, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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

if (BuildFileComponentsList(&EcoList, &Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords)
	&& GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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

	CurEffect = EcoList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
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

					if (BytesWritten = ((EcosystemEffect *)CurEffect->Me)->Save(ffile))
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
	while (EcoList)
		{
		CurEffect = EcoList;
		EcoList = EcoList->Next;
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

// Environment
strcpy(StrBuf, "Environ");
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
			} // if Environment saved 
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

} // EnvironmentEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::EnvironmentEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
EnvironmentEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&EnvironmentBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&EnvironmentBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&EnvironmentBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new EnvironmentEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (EnvironmentEffect *)FindDuplicateByName(Current->EffectType, Current))
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

#ifndef WCS_VECPOLY_EFFECTS
if (! FloatingLoaded)
	{
	// if user has previously modified, turn floating off
	if (fabs(EnvironmentBase.Resolution - 90.0) > .0001)
		EnvironmentBase.Floating = 0;
	} // if older file
#endif // WCS_VECPOLY_EFFECTS

return (TotalRead);

} // EffectsLib::EnvironmentEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::EnvironmentEffect_Save(FILE *ffile)
{
EnvironmentEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&EnvironmentBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EnvironmentBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EnvironmentBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Environment;
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
	Current = (EnvironmentEffect *)Current->Next;
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

} // EffectsLib::EnvironmentEffect_Save()

/*===========================================================================*/
